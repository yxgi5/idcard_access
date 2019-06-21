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


#if I2C1_EN == 1
/* 定义I2C总线连接的GPIO端口, 只需要修改下面4行代码即可任意改变SCL和SDA的引脚 */
#define I2C1_SCL_GPIO  GPIOB     /* 连接到SCL时钟线的GPIO */
#define I2C1_SDA_GPIO  GPIOB     /* 连接到SDA数据线的GPIO */

#define I2C1_SCL_PIN   GPIO_PIN_6     /* 连接到SCL时钟线的GPIO */
#define I2C1_SDA_PIN   GPIO_PIN_7      /* 连接到SDA数据线的GPIO */

#define ALL_I2C1_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()

/* 定义读写SCL和SDA的宏 */
#define I2C1_SCL_1()  I2C1_SCL_GPIO->BSRR = I2C1_SCL_PIN        /* SCL = 1 */
#define I2C1_SCL_0()  I2C1_SCL_GPIO->BSRR = (uint32_t)I2C1_SCL_PIN << 16U        /* SCL = 0 */

#define I2C1_SDA_1()  I2C1_SDA_GPIO->BSRR = I2C1_SDA_PIN        /* SDA = 1 */
#define I2C1_SDA_0()  I2C1_SDA_GPIO->BSRR = (uint32_t)I2C1_SDA_PIN << 16U        /* SDA = 0 */

#define I2C1_SDA_READ()  ((I2C1_SDA_GPIO->IDR & I2C1_SDA_PIN) != 0)  /* 读SDA口线状态 */
#define I2C1_SCL_READ()  ((I2C1_SCL_GPIO->IDR & I2C1_SCL_PIN) != 0)  /* 读SCL口线状态 */

/*
*********************************************************************************************************
* 函 数 名: bsp_InitI2C
* 功能说明: 配置I2C总线的GPIO，采用模拟IO的方式实现
* 形    参:  无
* 返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitI2C1(void)
{
  GPIO_InitTypeDef gpio_init;

  /* 第1步：打开GPIO时钟 */
  ALL_I2C1_GPIO_CLK_ENABLE();

  gpio_init.Mode = GPIO_MODE_OUTPUT_OD; /* 设置开漏输出 */
  gpio_init.Pull = GPIO_NOPULL;     /* 上下拉电阻不使能 */
  gpio_init.Speed = GPIO_SPEED_FREQ_LOW;  // GPIO_SPEED_FREQ_HIGH;  /* GPIO速度等级 */

  gpio_init.Pin = I2C1_SCL_PIN;
  HAL_GPIO_Init(I2C1_SCL_GPIO, &gpio_init);

  gpio_init.Pin = I2C1_SDA_PIN;
  HAL_GPIO_Init(I2C1_SDA_GPIO, &gpio_init);

  /* 给一个停止信号, 复位I2C总线上的所有设备到待机模式 */
  i2c_Stop();
}

/*
*********************************************************************************************************
* 函 数 名: i2c_Start
* 功能说明: CPU发起I2C总线启动信号
* 形    参:  无
* 返 回 值: 无
*********************************************************************************************************
*/
void i2c1_Start(void)
{
  /* 当SCL高电平时，SDA出现一个下跳沿表示I2C总线启动信号 */
  I2C1_SDA_1();
  I2C1_SCL_1();
  i2c_Delay();
  I2C1_SDA_0();
  i2c_Delay();

  I2C1_SCL_0();
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
void i2c1_Stop(void)
{
  /* 当SCL高电平时，SDA出现一个上跳沿表示I2C总线停止信号 */
  I2C1_SDA_0();
  i2c_Delay();
  I2C1_SCL_1();
  i2c_Delay();
  I2C1_SDA_1();
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
void i2c1_SendByte(uint8_t _ucByte)
{
  uint8_t i;

  /* 先发送字节的高位bit7 */
  for (i = 0; i < 8; i++)
  {
    if (_ucByte & 0x80)
    {
      I2C1_SDA_1();
    }
    else
    {
      I2C1_SDA_0();
    }
    i2c_Delay();
    I2C1_SCL_1();
    i2c_Delay();
    I2C1_SCL_0();
    I2C1_SCL_0();  /* 延迟几十ns */
    if (i == 7)
    {
       I2C1_SDA_1(); // 释放总线
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
uint8_t i2c1_ReadByte(void)
{
  uint8_t i;
  uint8_t value;

  /* 读到第1个bit为数据的bit7 */
  value = 0;
  for (i = 0; i < 8; i++)
  {
    value <<= 1;
    I2C1_SCL_1();
    i2c_Delay();
    if (I2C1_SDA_READ())
    {
      value++;
    }
    I2C1_SCL_0();
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
uint8_t i2c1_WaitAck(void)
{
  uint8_t re;

  I2C1_SDA_1();  /* CPU释放SDA总线 */
  i2c_Delay();
  I2C1_SCL_1();  /* CPU驱动SCL = 1, 此时器件会返回ACK应答 */
  i2c_Delay();
  if (I2C1_SDA_READ()) /* CPU读取SDA口线状态 */
  {
    re = 1;
  }
  else
  {
    re = 0;
  }
  I2C1_SCL_0();
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
void i2c1_Ack(void)
{
  I2C1_SDA_0();  /* CPU驱动SDA = 0 */
  i2c_Delay();
  I2C1_SCL_1();  /* CPU产生1个时钟 */
  i2c_Delay();
  I2C1_SCL_0();
  i2c_Delay();
  I2C1_SDA_1();  /* CPU释放SDA总线 */

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
void i2c1_NAck(void)
{
  I2C1_SDA_1();  /* CPU驱动SDA = 1 */
  i2c_Delay();
  I2C1_SCL_1();  /* CPU产生1个时钟 */
  i2c_Delay();
  I2C1_SCL_0();
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
uint8_t i2c1_CheckDevice(uint8_t _Address)
{
  uint8_t ucAck;

  if (I2C1_SDA_READ() && I2C1_SCL_READ())
  {
    i2c1_Start();    /* 发送启动信号 */

    /* 发送设备地址+读写控制bit（0 = w， 1 = r) bit7 先传 */
    i2c1_SendByte(_Address | I2C_WR);
    ucAck = i2c1_WaitAck();  /* 检测设备的ACK应答 */

    i2c1_Stop();     /* 发送停止信号 */

    return ucAck;
  }
  return 1; /* I2C总线异常 */
}
#endif // I2C1_EN

#if I2C2_EN == 1
/* 定义I2C总线连接的GPIO端口, 只需要修改下面4行代码即可任意改变SCL和SDA的引脚 */
#define I2C2_SCL_GPIO  GPIOB     /* 连接到SCL时钟线的GPIO */
#define I2C2_SDA_GPIO  GPIOB     /* 连接到SDA数据线的GPIO */

#define I2C2_SCL_PIN   GPIO_PIN_10     /* 连接到SCL时钟线的GPIO */
#define I2C2_SDA_PIN   GPIO_PIN_11      /* 连接到SDA数据线的GPIO */

#define ALL_I2C2_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()

/* 定义读写SCL和SDA的宏 */
#define I2C2_SCL_1()  I2C2_SCL_GPIO->BSRR = I2C2_SCL_PIN        /* SCL = 1 */
#define I2C2_SCL_0()  I2C2_SCL_GPIO->BSRR = (uint32_t)I2C2_SCL_PIN << 16U        /* SCL = 0 */

#define I2C2_SDA_1()  I2C2_SDA_GPIO->BSRR = I2C2_SDA_PIN        /* SDA = 1 */
#define I2C2_SDA_0()  I2C2_SDA_GPIO->BSRR = (uint32_t)I2C2_SDA_PIN << 16U        /* SDA = 0 */

#define I2C2_SDA_READ()  ((I2C2_SDA_GPIO->IDR & I2C2_SDA_PIN) != 0)  /* 读SDA口线状态 */
#define I2C2_SCL_READ()  ((I2C2_SCL_GPIO->IDR & I2C2_SCL_PIN) != 0)  /* 读SCL口线状态 */

/*
*********************************************************************************************************
* 函 数 名: bsp_InitI2C
* 功能说明: 配置I2C总线的GPIO，采用模拟IO的方式实现
* 形    参:  无
* 返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitI2C2(void)
{
  GPIO_InitTypeDef gpio_init;

  /* 第1步：打开GPIO时钟 */
  ALL_I2C2_GPIO_CLK_ENABLE();

  gpio_init.Mode = GPIO_MODE_OUTPUT_OD; /* 设置开漏输出 */
  gpio_init.Pull = GPIO_NOPULL;     /* 上下拉电阻不使能 */
  gpio_init.Speed = GPIO_SPEED_FREQ_LOW;  // GPIO_SPEED_FREQ_HIGH;  /* GPIO速度等级 */

  gpio_init.Pin = I2C2_SCL_PIN;
  HAL_GPIO_Init(I2C2_SCL_GPIO, &gpio_init);

  gpio_init.Pin = I2C2_SDA_PIN;
  HAL_GPIO_Init(I2C2_SDA_GPIO, &gpio_init);

  /* 给一个停止信号, 复位I2C总线上的所有设备到待机模式 */
  i2c_Stop();
}

/*
*********************************************************************************************************
* 函 数 名: i2c_Start
* 功能说明: CPU发起I2C总线启动信号
* 形    参:  无
* 返 回 值: 无
*********************************************************************************************************
*/
void i2c2_Start(void)
{
  /* 当SCL高电平时，SDA出现一个下跳沿表示I2C总线启动信号 */
  I2C2_SDA_1();
  I2C2_SCL_1();
  i2c_Delay();
  I2C2_SDA_0();
  i2c_Delay();

  I2C2_SCL_0();
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
void i2c2_Stop(void)
{
  /* 当SCL高电平时，SDA出现一个上跳沿表示I2C总线停止信号 */
  I2C2_SDA_0();
  i2c_Delay();
  I2C2_SCL_1();
  i2c_Delay();
  I2C2_SDA_1();
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
void i2c2_SendByte(uint8_t _ucByte)
{
  uint8_t i;

  /* 先发送字节的高位bit7 */
  for (i = 0; i < 8; i++)
  {
    if (_ucByte & 0x80)
    {
      I2C2_SDA_1();
    }
    else
    {
      I2C2_SDA_0();
    }
    i2c_Delay();
    I2C2_SCL_1();
    i2c_Delay();
    I2C2_SCL_0();
    I2C2_SCL_0();  /* 延迟几十ns */
    if (i == 7)
    {
       I2C2_SDA_1(); // 释放总线
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
uint8_t i2c2_ReadByte(void)
{
  uint8_t i;
  uint8_t value;

  /* 读到第1个bit为数据的bit7 */
  value = 0;
  for (i = 0; i < 8; i++)
  {
    value <<= 1;
    I2C2_SCL_1();
    i2c_Delay();
    if (I2C2_SDA_READ())
    {
      value++;
    }
    I2C2_SCL_0();
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
uint8_t i2c2_WaitAck(void)
{
  uint8_t re;

  I2C2_SDA_1();  /* CPU释放SDA总线 */
  i2c_Delay();
  I2C2_SCL_1();  /* CPU驱动SCL = 1, 此时器件会返回ACK应答 */
  i2c_Delay();
  if (I2C2_SDA_READ()) /* CPU读取SDA口线状态 */
  {
    re = 1;
  }
  else
  {
    re = 0;
  }
  I2C2_SCL_0();
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
void i2c2_Ack(void)
{
  I2C2_SDA_0();  /* CPU驱动SDA = 0 */
  i2c_Delay();
  I2C2_SCL_1();  /* CPU产生1个时钟 */
  i2c_Delay();
  I2C2_SCL_0();
  i2c_Delay();
  I2C2_SDA_1();  /* CPU释放SDA总线 */

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
void i2c2_NAck(void)
{
  I2C2_SDA_1();  /* CPU驱动SDA = 1 */
  i2c_Delay();
  I2C2_SCL_1();  /* CPU产生1个时钟 */
  i2c_Delay();
  I2C2_SCL_0();
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
uint8_t i2c2_CheckDevice(uint8_t _Address)
{
  uint8_t ucAck;

  if (I2C2_SDA_READ() && I2C2_SCL_READ())
  {
    i2c2_Start();    /* 发送启动信号 */

    /* 发送设备地址+读写控制bit（0 = w， 1 = r) bit7 先传 */
    i2c2_SendByte(_Address | I2C_WR);
    ucAck = i2c2_WaitAck();  /* 检测设备的ACK应答 */

    i2c2_Stop();     /* 发送停止信号 */

    return ucAck;
  }
  return 1; /* I2C总线异常 */
}
#endif // I2C2_EN

#else


I2C_HandleTypeDef I2c1Handle;
I2C_HandleTypeDef I2c2Handle;

#define I2C1_ADDRESS        0x1E
#define I2C2_ADDRESS        0x2E
/* I2C TIMING Register define when I2C clock source is SYSCLK */
/* I2C TIMING is calculated in case of the I2C Clock source is the SYSCLK = 32 MHz */
#define I2C1_TIMING    0x10A13E56 /* 100 kHz with analog Filter ON, Rise Time 400ns, Fall Time 100ns */
//#define I2C1_TIMING      0x00B1112E /* 400 kHz with analog Filter ON, Rise Time 250ns, Fall Time 100ns */
#define I2C2_TIMING    0x10A13E56 /* 100 kHz with analog Filter ON, Rise Time 400ns, Fall Time 100ns */
//#define I2C2_TIMING      0x00B1112E /* 400 kHz with analog Filter ON, Rise Time 250ns, Fall Time 100ns */

/*
*********************************************************************************************************
* 函 数 名: Configure the I2C peripheral
* 功能说明: 配置I2C总线
* 形    参:  无
* 返 回 值: 无
*********************************************************************************************************
*/
//void bsp_InitI2C(void)
//{
////  GPIO_InitTypeDef  GPIO_InitStruct;
////  I2C_TypeDef  I2C_InitStruct;
//
//#if I2C1_EN == 1
////  /* 使能 GPIO SCL/SDA 时钟 */
////  I2C1_GPIO_CLK_ENABLE();
////
////  GPIO_InitStruct.Pin = I2C1_SCL_PIN | I2C1_SDA_PIN;
////  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
////  GPIO_InitStruct.Pull = GPIO_PULLUP;
////  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
////  GPIO_InitStruct.Alternate = I2C1_SCL_SDA_AF;
////
//////  GPIO_InitStruct.Pin = I2C1_SCL_PIN;
////  HAL_GPIO_Init(I2C1_SCL_GPIO_PORT, &GPIO_InitStruct);  // I2C1_SDA in the same port
//////  GPIO_InitStruct.Pin = I2C1_SDA_PIN;
//////  HAL_GPIO_Init(I2C1_SDA_GPIO_PORT, &GPIO_InitStruct);
////
////  /* 使能 I2Cx 时钟 */
////  I2C1_CLK_ENABLE();
////
////  CLEAR_BIT(I2C1->CR1, I2C_CR1_PE);
////  I2C1->TIMINGR = I2C1_TIMING & TIMING_CLEAR_MASK;
////  I2C1->OAR1 &= ~I2C_OAR1_OA1EN;
//
//  I2c1Handle.Instance             = I2C1;
//  I2c1Handle.Init.Timing          = I2C1_TIMING;
//  I2c1Handle.Init.OwnAddress1     = I2C1_ADDRESS;
//  I2c1Handle.Init.AddressingMode  = I2C_ADDRESSINGMODE_10BIT;
//  I2c1Handle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
//  I2c1Handle.Init.OwnAddress2     = 0xFF;
//  I2c1Handle.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
//  I2c1Handle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
//  I2c1Handle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;
//
//  if(HAL_I2C_Init(&I2c1Handle) != HAL_OK)
//  {
//    /* Initialization Error */
//    Error_Handler(__FILE__, __LINE__);
//  }
//
//  /*
//   * Enable Analogue filter
//   */
//  if (HAL_I2CEx_ConfigAnalogFilter(&I2c1Handle, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
//  {
//    _Error_Handler(__FILE__, __LINE__);
//  }
//
//  /*
//   * Enable Digital filter
//   */
//  if (HAL_I2CEx_ConfigDigitalFilter(&I2c1Handle, 0) != HAL_OK)
//  {
//    _Error_Handler(__FILE__, __LINE__);
//  }
//
//#endif // I2C1_EN
//
//#if I2C2_EN == 1
////  I2C2_GPIO_CLK_ENABLE();
////  GPIO_InitStruct.Pin = I2C2_SCL_PIN | I2C2_SDA_PIN;
////  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
////  GPIO_InitStruct.Pull = GPIO_PULLUP;
////  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
////  GPIO_InitStruct.Alternate = I2C2_SCL_SDA_AF;
////  HAL_GPIO_Init(I2C2_SCL_GPIO_PORT, &GPIO_InitStruct);
//////  HAL_GPIO_Init(I2C2_SDA_GPIO_PORT, &GPIO_InitStruct);
////
////  I2C2_CLK_ENABLE();
////
////  CLEAR_BIT(I2C2->CR1, I2C_CR1_PE);
////  I2C2->TIMINGR = I2C2_TIMING & TIMING_CLEAR_MASK;
//
//  I2c2Handle.Instance             = I2C2;
//  I2c2Handle.Init.Timing          = I2C2_TIMING;
//  I2c2Handle.Init.OwnAddress1     = I2C2_ADDRESS;
//  I2c2Handle.Init.AddressingMode  = I2C_ADDRESSINGMODE_10BIT;
//  I2c2Handle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
//  I2c2Handle.Init.OwnAddress2     = 0xFF;
//  I2c2Handle.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
//  I2c2Handle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
//  I2c2Handle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;
//
//  if(HAL_I2C_Init(&I2c2Handle) != HAL_OK)
//  {
//    /* Initialization Error */
//    Error_Handler(__FILE__, __LINE__);
//  }
//
//  /*
//   * Enable Analogue filter
//   */
//  if (HAL_I2CEx_ConfigAnalogFilter(&I2c2Handle, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
//  {
//    _Error_Handler(__FILE__, __LINE__);
//  }
//
//  /*
//   * Enable Digital filter
//   */
//  if (HAL_I2CEx_ConfigDigitalFilter(&I2c2Handle, 0) != HAL_OK)
//  {
//    _Error_Handler(__FILE__, __LINE__);
//  }
//
//#endif // I2C2_EN
//
//}

#if I2C1_EN == 1
void bsp_InitI2C1(void)
{
  I2c1Handle.Instance             = I2C1;
  I2c1Handle.Init.Timing          = I2C1_TIMING;
  I2c1Handle.Init.OwnAddress1     = I2C1_ADDRESS;
  I2c1Handle.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
  I2c1Handle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  I2c1Handle.Init.OwnAddress2     = 0xFF;
  I2c1Handle.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  I2c1Handle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  I2c1Handle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;

  if(HAL_I2C_Init(&I2c1Handle) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler(__FILE__, __LINE__);
  }

  /*
   * Enable Analogue filter
   */
  if (HAL_I2CEx_ConfigAnalogFilter(&I2c1Handle, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }

//  /*
//   * Enable Digital filter
//   */
//  if (HAL_I2CEx_ConfigDigitalFilter(&I2c1Handle, 0) != HAL_OK)
//  {
//    Error_Handler(__FILE__, __LINE__);
//  }
}

#endif // I2C1_EN

#if I2C2_EN == 1
void bsp_InitI2C2(void)
{
  I2c2Handle.Instance             = I2C2;
  I2c2Handle.Init.Timing          = I2C2_TIMING;
  I2c2Handle.Init.OwnAddress1     = I2C2_ADDRESS;
  I2c2Handle.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
  I2c2Handle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  I2c2Handle.Init.OwnAddress2     = 0xFF;
  I2c2Handle.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  I2c2Handle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  I2c2Handle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;

  if(HAL_I2C_Init(&I2c2Handle) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler(__FILE__, __LINE__);
  }

  /*
   * Enable Analogue filter
   */
  if (HAL_I2CEx_ConfigAnalogFilter(&I2c2Handle, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }

//  /*
//   * Enable Digital filter
//   */
//  if (HAL_I2CEx_ConfigDigitalFilter(&I2c2Handle, 0) != HAL_OK)
//  {
//    Error_Handler(__FILE__, __LINE__);
//  }
}
#endif // I2C2_EN

uint8_t i2c_SendByte(I2C_HandleTypeDef *hi2c, uint8_t *_ucBuffer, uint16_t SlaveAddr)
{
  do
  {
    if(HAL_I2C_Master_Transmit_IT(hi2c, SlaveAddr, _ucBuffer, 1) != HAL_OK)
    {
      /* Error_Handler() function is called when error occurs. */
      Error_Handler(__FILE__, __LINE__);
      return 1;
    }

    /*##-5- Wait for the end of the transfer #################################*/
    /*  Before starting a new communication transfer, you need to check the current
        state of the peripheral; if it�s busy you need to wait for the end of current
        transfer before starting a new one.
        For simplicity reasons, this example is just waiting till the end of the
        transfer, but application may perform other tasks while transfer operation
        is ongoing. */
    while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY)
    {
    }

    /* When Acknowledge failure occurs (Slave don't acknowledge it's address)
       Master restarts communication */
  }
  while(HAL_I2C_GetError(hi2c) == HAL_I2C_ERROR_AF);

  return 0;
}

uint8_t i2c_ReadByte(I2C_HandleTypeDef *hi2c, uint8_t *_ucBuffer, uint16_t SlaveAddr)
{
  do
  {
    if(HAL_I2C_Master_Receive_IT(hi2c, SlaveAddr, _ucBuffer, 1) != HAL_OK)
    {
      /* Error_Handler() function is called when error occurs. */
      Error_Handler(__FILE__, __LINE__);
      return 1;
    }

    /*##-5- Wait for the end of the transfer #################################*/
    /*  Before starting a new communication transfer, you need to check the current
        state of the peripheral; if it�s busy you need to wait for the end of current
        transfer before starting a new one.
        For simplicity reasons, this example is just waiting till the end of the
        transfer, but application may perform other tasks while transfer operation
        is ongoing. */
    while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY)
    {
    }

    /* When Acknowledge failure occurs (Slave don't acknowledge it's address)
       Master restarts communication */
  }
  while(HAL_I2C_GetError(hi2c) == HAL_I2C_ERROR_AF);

  return 0;
}

uint8_t i2c_SendBytes(I2C_HandleTypeDef *hi2c, uint8_t *_ucBuffer, uint16_t ByteCount, uint16_t SlaveAddr)
{
  do
  {
    if(HAL_I2C_Master_Transmit_IT(hi2c, SlaveAddr, _ucBuffer, ByteCount) != HAL_OK)
//	if(HAL_I2C_Mem_Write_IT(hi2c, DevAddress, MemAddress, MemAddSize, pData, Size)!= HAL_OK)
    {
      /* Error_Handler() function is called when error occurs. */
      Error_Handler(__FILE__, __LINE__);
      return 1;
    }

    /*##-5- Wait for the end of the transfer #################################*/
    /*  Before starting a new communication transfer, you need to check the current
        state of the peripheral; if it�s busy you need to wait for the end of current
        transfer before starting a new one.
        For simplicity reasons, this example is just waiting till the end of the
        transfer, but application may perform other tasks while transfer operation
        is ongoing. */
    while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY)
    {
    }

    /* When Acknowledge failure occurs (Slave don't acknowledge it's address)
       Master restarts communication */
  }
  while(HAL_I2C_GetError(hi2c) == HAL_I2C_ERROR_AF);

  return 0;
}

uint8_t i2c_ReadBytes(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size)
{

	do
	{
	  if(HAL_I2C_Mem_Read_IT(hi2c, DevAddress, MemAddress, MemAddSize, pData, Size)!= HAL_OK)
	  {
		/* Error_Handler() function is called when error occurs. */
		Error_Handler(__FILE__, __LINE__);
		return 1;
	  }

	  /*  Before starting a new communication transfer, you need to check the current
		  state of the peripheral; if it抯 busy you need to wait for the end of current
		  transfer before starting a new one.
		  For simplicity reasons, this example is just waiting till the end of the
		  transfer, but application may perform other tasks while transfer operation
		  is ongoing. */
	  while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY)
	  {
	  }

	  /* When Acknowledge failure occurs (Slave don't acknowledge it's address)
		 Master restarts communication */
	}
	while(HAL_I2C_GetError(hi2c) == HAL_I2C_ERROR_AF);

	return 0;
}

uint8_t i2c_CheckDevice(I2C_HandleTypeDef *hi2c, uint8_t _Address)
{
  uint8_t tmp,ret;
  ret=i2c_ReadByte(hi2c, &tmp,_Address);

  return ret; // 1 为异常
}

#endif // USE_GPIO_I2C
