/*
 * bsp_i2c.c
 *
 *  Created on: Jun 15, 2019
 *      Author: andy
 */

#include "bsp.h"

/*
 * PB6: I2C1_SCL , PB7: I2C1_SDA
 * PB10: I2C2_SCL , PB11: I2C2_SDA
 *
 */

#if USE_GPIO_I2C == 1
/* 定义I2C总线连接的GPIO端口, 只需要修改下面4行代码即可任意改变SCL和SDA的引脚 */
#define I2C_SCL_GPIO  GPIOB     /* 连接到SCL时钟线的GPIO */
#define I2C_SDA_GPIO  GPIOB     /* 连接到SDA数据线的GPIO */

#define I2C_SCL_PIN   GPIO_PIN_10     /* 连接到SCL时钟线的GPIO */
#define I2C_SDA_PIN   GPIO_PIN_11      /* 连接到SDA数据线的GPIO */

#define ALL_I2C_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()

/* 定义读写SCL和SDA的宏 */
#define I2C_SCL_1()  I2C_SCL_GPIO->BSRR = I2C_SCL_PIN        /* SCL = 1 */
#define I2C_SCL_0()  I2C_SCL_GPIO->BSRR = (uint32_t)I2C_SCL_PIN << 16U        /* SCL = 0 */

#define I2C_SDA_1()  I2C_SDA_GPIO->BSRR = I2C_SDA_PIN        /* SDA = 1 */
#define I2C_SDA_0()  I2C_SDA_GPIO->BSRR = (uint32_t)I2C_SDA_PIN << 16U        /* SDA = 0 */

#define I2C_SDA_READ()  ((I2C_SDA_GPIO->IDR & I2C_SDA_PIN) != 0)  /* 读SDA口线状态 */
#define I2C_SCL_READ()  ((I2C_SCL_GPIO->IDR & I2C_SCL_PIN) != 0)  /* 读SCL口线状态 */

/*
*********************************************************************************************************
* 函 数 名: bsp_InitI2C
* 功能说明: 配置I2C总线的GPIO，采用模拟IO的方式实现
* 形    参:  无
* 返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitI2C(void)
{
  GPIO_InitTypeDef gpio_init;

  /* 第1步：打开GPIO时钟 */
  ALL_I2C_GPIO_CLK_ENABLE();

  gpio_init.Mode = GPIO_MODE_OUTPUT_OD; /* 设置开漏输出 */
  gpio_init.Pull = GPIO_NOPULL;     /* 上下拉电阻不使能 */
  gpio_init.Speed = GPIO_SPEED_FREQ_LOW;  // GPIO_SPEED_FREQ_HIGH;  /* GPIO速度等级 */

  gpio_init.Pin = I2C_SCL_PIN;
  HAL_GPIO_Init(I2C_SCL_GPIO, &gpio_init);

  gpio_init.Pin = I2C_SDA_PIN;
  HAL_GPIO_Init(I2C_SDA_GPIO, &gpio_init);

  /* 给一个停止信号, 复位I2C总线上的所有设备到待机模式 */
  i2c_Stop();
}

/*
*********************************************************************************************************
* 函 数 名: i2c_Delay
* 功能说明: I2C总线位延迟，最快400KHz
* 形    参:  无
* 返 回 值: 无
*********************************************************************************************************
*/
static void i2c_Delay(void)
{
  bsp_DelayUS(2);
}

/*
*********************************************************************************************************
* 函 数 名: i2c_Start
* 功能说明: CPU发起I2C总线启动信号
* 形    参:  无
* 返 回 值: 无
*********************************************************************************************************
*/
void i2c_Start(void)
{
  /* 当SCL高电平时，SDA出现一个下跳沿表示I2C总线启动信号 */
  I2C_SDA_1();
  I2C_SCL_1();
  i2c_Delay();
  I2C_SDA_0();
  i2c_Delay();

  I2C_SCL_0();
  i2c_Delay();
}

/*
*********************************************************************************************************
* 函 数 名: i2c_Start
* 功能说明: CPU发起I2C总线停止信号
* 形    参:  无
* 返 回 值: 无
*********************************************************************************************************
*/
void i2c_Stop(void)
{
  /* 当SCL高电平时，SDA出现一个上跳沿表示I2C总线停止信号 */
  I2C_SDA_0();
  i2c_Delay();
  I2C_SCL_1();
  i2c_Delay();
  I2C_SDA_1();
  i2c_Delay();
}

/*
*********************************************************************************************************
* 函 数 名: i2c_SendByte
* 功能说明: CPU向I2C总线设备发送8bit数据
* 形    参:  _ucByte ： 等待发送的字节
* 返 回 值: 无
*********************************************************************************************************
*/
void i2c_SendByte(uint8_t _ucByte)
{
  uint8_t i;

  /* 先发送字节的高位bit7 */
  for (i = 0; i < 8; i++)
  {
    if (_ucByte & 0x80)
    {
      I2C_SDA_1();
    }
    else
    {
      I2C_SDA_0();
    }
    i2c_Delay();
    I2C_SCL_1();
    i2c_Delay();
    I2C_SCL_0();
    I2C_SCL_0();  /* 延迟几十ns */
    if (i == 7)
    {
       I2C_SDA_1(); // 释放总线
    }
    _ucByte <<= 1;  /* 左移一个bit */
  }
}

/*
*********************************************************************************************************
* 函 数 名: i2c_ReadByte
* 功能说明: CPU从I2C总线设备读取8bit数据
* 形    参:  无
* 返 回 值: 读到的数据
*********************************************************************************************************
*/
uint8_t i2c_ReadByte(void)
{
  uint8_t i;
  uint8_t value;

  /* 读到第1个bit为数据的bit7 */
  value = 0;
  for (i = 0; i < 8; i++)
  {
    value <<= 1;
    I2C_SCL_1();
    i2c_Delay();
    if (I2C_SDA_READ())
    {
      value++;
    }
    I2C_SCL_0();
    i2c_Delay();
  }
  return value;
}

/*
*********************************************************************************************************
* 函 数 名: i2c_WaitAck
* 功能说明: CPU产生一个时钟，并读取器件的ACK应答信号
* 形    参:  无
* 返 回 值: 返回0表示正确应答，1表示无器件响应
*********************************************************************************************************
*/
uint8_t i2c_WaitAck(void)
{
  uint8_t re;

  I2C_SDA_1();  /* CPU释放SDA总线 */
  i2c_Delay();
  I2C_SCL_1();  /* CPU驱动SCL = 1, 此时器件会返回ACK应答 */
  i2c_Delay();
  if (I2C_SDA_READ()) /* CPU读取SDA口线状态 */
  {
    re = 1;
  }
  else
  {
    re = 0;
  }
  I2C_SCL_0();
  i2c_Delay();
  return re;
}

/*
*********************************************************************************************************
* 函 数 名: i2c_Ack
* 功能说明: CPU产生一个ACK信号
* 形    参:  无
* 返 回 值: 无
*********************************************************************************************************
*/
void i2c_Ack(void)
{
  I2C_SDA_0();  /* CPU驱动SDA = 0 */
  i2c_Delay();
  I2C_SCL_1();  /* CPU产生1个时钟 */
  i2c_Delay();
  I2C_SCL_0();
  i2c_Delay();
  I2C_SDA_1();  /* CPU释放SDA总线 */

  i2c_Delay();
}

/*
*********************************************************************************************************
* 函 数 名: i2c_NAck
* 功能说明: CPU产生1个NACK信号
* 形    参:  无
* 返 回 值: 无
*********************************************************************************************************
*/
void i2c_NAck(void)
{
  I2C_SDA_1();  /* CPU驱动SDA = 1 */
  i2c_Delay();
  I2C_SCL_1();  /* CPU产生1个时钟 */
  i2c_Delay();
  I2C_SCL_0();
  i2c_Delay();
}

/*
*********************************************************************************************************
* 函 数 名: i2c_CheckDevice
* 功能说明: 检测I2C总线设备，CPU向发送设备地址，然后读取设备应答来判断该设备是否存在
* 形    参:  _Address：设备的I2C总线地址
* 返 回 值: 返回值 0 表示正确， 返回1表示未探测到
*********************************************************************************************************
*/
uint8_t i2c_CheckDevice(uint8_t _Address)
{
  uint8_t ucAck;

  if (I2C_SDA_READ() && I2C_SCL_READ())
  {
    i2c_Start();    /* 发送启动信号 */

    /* 发送设备地址+读写控制bit（0 = w， 1 = r) bit7 先传 */
    i2c_SendByte(_Address | I2C_WR);
    ucAck = i2c_WaitAck();  /* 检测设备的ACK应答 */

    i2c_Stop();     /* 发送停止信号 */

    return ucAck;
  }
  return 1; /* I2C总线异常 */
}

#else


I2C_HandleTypeDef I2cHandle;
#define I2C_ADDRESS        0x30F
/* I2C TIMING Register define when I2C clock source is SYSCLK */
/* I2C TIMING is calculated in case of the I2C Clock source is the SYSCLK = 32 MHz */
//#define I2C_TIMING    0x10A13E56 /* 100 kHz with analog Filter ON, Rise Time 400ns, Fall Time 100ns */
#define I2C_TIMING      0x00B1112E /* 400 kHz with analog Filter ON, Rise Time 250ns, Fall Time 100ns */
/*
*********************************************************************************************************
* 函 数 名: Configure the I2C peripheral
* 功能说明: 配置I2C总线
* 形    参:  无
* 返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitI2C(void)
{
  I2cHandle.Instance             = I2Cx;
  I2cHandle.Init.Timing          = I2C_TIMING;
  I2cHandle.Init.OwnAddress1     = I2C_ADDRESS;
  I2cHandle.Init.AddressingMode  = I2C_ADDRESSINGMODE_10BIT;
  I2cHandle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  I2cHandle.Init.OwnAddress2     = 0xFF;
  I2cHandle.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  I2cHandle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  I2cHandle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;

  if(HAL_I2C_Init(&I2cHandle) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler(__FILE__, __LINE__);
  }

  /* Enable the Analog I2C Filter */
  HAL_I2CEx_ConfigAnalogFilter(&I2cHandle,I2C_ANALOGFILTER_ENABLE);
}

#endif
