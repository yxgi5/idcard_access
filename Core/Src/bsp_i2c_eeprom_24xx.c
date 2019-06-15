/**
  ******************************************************************************
  * @file    bsp_i2c_eeprom_24xx.c
  * @author  Andreas Zhang
  * @version V1.0
  * @date    05-June-2019
  * @brief
  ******************************************************************************
*/

/*
  应用说明：访问串行EEPROM前，请先调用一次 bsp_InitI2C()函数配置好I2C相关的GPIO.
*/

#include "bsp.h"

#if USE_GPIO_I2C == 1
/*
*********************************************************************************************************
* 函 数 名: ee_CheckOk
* 功能说明: 判断串行EERPOM是否正常
* 形    参:  无
* 返 回 值: 1 表示正常， 0 表示不正常
*********************************************************************************************************
*/
uint8_t ee_CheckOk(void)
{

  if (i2c_CheckDevice(EE_DEV_ADDR) == 0)
  {
    return 1;
  }
  else
  {
    /* 失败后，切记发送I2C总线停止信号 */
    i2c_Stop();
    return 0;
  }
}


/*
*********************************************************************************************************
* 函 数 名: ee_ReadBytes
* 功能说明: 从串行EEPROM指定地址处开始读取若干数据
* 形    参:  _usAddress : 起始地址
*      _usSize : 数据长度，单位为字节
*      _pReadBuf : 存放读到的数据的缓冲区指针
* 返 回 值: 0 表示失败，1表示成功
*********************************************************************************************************
*/
uint8_t ee_ReadBytes(uint8_t *_pReadBuf, uint16_t _usAddress, uint16_t _usSize)
{
  uint16_t i;

  /* 采用串行EEPROM随即读取指令序列，连续读取若干字节 */

  /* 第1步：发起I2C总线启动信号 */
  i2c_Start();

  /* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
  i2c_SendByte(EE_DEV_ADDR | I2C_WR); /* 此处是写指令 */

  /* 第3步：发送ACK */
  if (i2c_WaitAck() != 0)
  {
    goto cmd_fail;  /* EEPROM器件无应答 */
  }

  /* 第4步：发送字节地址，24C02只有256字节，因此1个字节就够了，如果是24C04以上，那么此处需要连发多个地址 */
  if (EE_ADDR_BYTES == 1)
  {
    i2c_SendByte((uint8_t)_usAddress);
    if (i2c_WaitAck() != 0)
    {
      goto cmd_fail;  /* EEPROM器件无应答 */
    }
  }
  else
  {
    i2c_SendByte(_usAddress >> 8);
    if (i2c_WaitAck() != 0)
    {
      goto cmd_fail;  /* EEPROM器件无应答 */
    }

    i2c_SendByte(_usAddress);
    if (i2c_WaitAck() != 0)
    {
      goto cmd_fail;  /* EEPROM器件无应答 */
    }
  }

  /* 第6步：重新启动I2C总线。下面开始读取数据 */
  i2c_Start();

  /* 第7步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
  i2c_SendByte(EE_DEV_ADDR | I2C_RD); /* 此处是读指令 */

  /* 第8步：发送ACK */
  if (i2c_WaitAck() != 0)
  {
    goto cmd_fail;  /* EEPROM器件无应答 */
  }

  /* 第9步：循环读取数据 */
  for (i = 0; i < _usSize; i++)
  {
    _pReadBuf[i] = i2c_ReadByte();  /* 读1个字节 */

    /* 每读完1个字节后，需要发送Ack， 最后一个字节不需要Ack，发Nack */
    if (i != _usSize - 1)
    {
      i2c_Ack();  /* 中间字节读完后，CPU产生ACK信号(驱动SDA = 0) */
    }
    else
    {
      i2c_NAck(); /* 最后1个字节读完后，CPU产生NACK信号(驱动SDA = 1) */
    }
  }
  /* 发送I2C总线停止信号 */
  i2c_Stop();
  return 1; /* 执行成功 */

cmd_fail: /* 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备 */
  /* 发送I2C总线停止信号 */
  i2c_Stop();
  return 0;
}

/*
*********************************************************************************************************
* 函 数 名: ee_WriteBytes
* 功能说明: 向串行EEPROM指定地址写入若干数据，采用页写操作提高写入效率
* 形    参:  _usAddress : 起始地址
*      _usSize : 数据长度，单位为字节
*      _pWriteBuf : 存放读到的数据的缓冲区指针
* 返 回 值: 0 表示失败，1表示成功
*********************************************************************************************************
*/
uint8_t ee_WriteBytes(uint8_t *_pWriteBuf, uint16_t _usAddress, uint16_t _usSize)
{
  uint16_t i,m;
  uint16_t usAddr;

  /*
    写串行EEPROM不像读操作可以连续读取很多字节，每次写操作只能在同一个page。
    对于24xx02，page size = 8
    简单的处理方法为：按字节写操作模式，每写1个字节，都发送地址
    为了提高连续写的效率: 本函数采用page wirte操作。
  */

  usAddr = _usAddress;
  for (i = 0; i < _usSize; i++)
  {
    /* 当发送第1个字节或是页面首地址时，需要重新发起启动信号和地址 */
    if ((i == 0) || (usAddr & (EE_PAGE_SIZE - 1)) == 0)
    {
      /*　第０步：发停止信号，启动内部写操作　*/
      i2c_Stop();

      /* 通过检查器件应答的方式，判断内部写操作是否完成, 一般小于 10ms
        CLK频率为200KHz时，查询次数为30次左右
      */
      for (m = 0; m < 1000; m++)
      {
        /* 第1步：发起I2C总线启动信号 */
        i2c_Start();

        /* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
        i2c_SendByte(EE_DEV_ADDR | I2C_WR); /* 此处是写指令 */

        /* 第3步：发送一个时钟，判断器件是否正确应答 */
        if (i2c_WaitAck() == 0)
        {
          break;
        }
      }
      if (m  == 1000)
      {
        goto cmd_fail;  /* EEPROM器件写超时 */
      }

      /* 第4步：发送字节地址，24C02只有256字节，因此1个字节就够了，如果是24C04以上，那么此处需要连发多个地址 */
      if (EE_ADDR_BYTES == 1)
      {
        i2c_SendByte((uint8_t)usAddr);
        if (i2c_WaitAck() != 0)
        {
          goto cmd_fail;  /* EEPROM器件无应答 */
        }
      }
      else
      {
        i2c_SendByte(usAddr >> 8);
        if (i2c_WaitAck() != 0)
        {
          goto cmd_fail;  /* EEPROM器件无应答 */
        }

        i2c_SendByte(usAddr);
        if (i2c_WaitAck() != 0)
        {
          goto cmd_fail;  /* EEPROM器件无应答 */
        }
      }
    }

    /* 第6步：开始写入数据 */
    i2c_SendByte(_pWriteBuf[i]);

    /* 第7步：发送ACK */
    if (i2c_WaitAck() != 0)
    {
      goto cmd_fail;  /* EEPROM器件无应答 */
    }

    usAddr++; /* 地址增1 */
  }

  /* 命令执行成功，发送I2C总线停止信号 */
  i2c_Stop();

  /* 通过检查器件应答的方式，判断内部写操作是否完成, 一般小于 10ms
    CLK频率为200KHz时，查询次数为30次左右
  */
  for (m = 0; m < 1000; m++)
  {
    /* 第1步：发起I2C总线启动信号 */
    i2c_Start();

    /* 第2步：发起控制字节，高7bit是地址，bit0是读写控制位，0表示写，1表示读 */
    #if EE_ADDR_A8 == 1
      i2c_SendByte(EE_DEV_ADDR | I2C_WR | ((_usAddress >> 7) & 0x0E));  /* 此处是写指令 */
    #else
      i2c_SendByte(EE_DEV_ADDR | I2C_WR); /* 此处是写指令 */
    #endif

    /* 第3步：发送一个时钟，判断器件是否正确应答 */
    if (i2c_WaitAck() == 0)
    {
      break;
    }
  }
  if (m  == 1000)
  {
    goto cmd_fail;  /* EEPROM器件写超时 */
  }

  /* 命令执行成功，发送I2C总线停止信号 */
  i2c_Stop();
  return 1;

cmd_fail: /* 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备 */
  /* 发送I2C总线停止信号 */
  i2c_Stop();
  return 0;
}

#else

uint8_t ee_CheckOk(void)
{

}

uint8_t ee_ReadBytes(uint8_t *_pReadBuf, uint16_t _usAddress, uint16_t _usSize)
{
  do
  {
    if(HAL_I2C_Master_Receive_IT(&I2cHandle, (uint16_t)_usAddress, (uint8_t *)_pReadBuf, _usSize) != HAL_OK)
    {
      /* Error_Handler() function is called when error occurs. */
        Error_Handler(__FILE__, __LINE__);
    }

    /*##-5- Wait for the end of the transfer #################################*/
    /*  Before starting a new communication transfer, you need to check the current
        state of the peripheral; if it�s busy you need to wait for the end of current
        transfer before starting a new one.
        For simplicity reasons, this example is just waiting till the end of the
        transfer, but application may perform other tasks while transfer operation
        is ongoing. */
    while (HAL_I2C_GetState(&I2cHandle) != HAL_I2C_STATE_READY)
    {
    }

    /* When Acknowledge failure occurs (Slave don't acknowledge it's address)
       Master restarts communication */
  }
  while(HAL_I2C_GetError(&I2cHandle) == HAL_I2C_ERROR_AF);

  return 0;
}

uint8_t ee_WriteBytes(uint8_t *_pWriteBuf, uint16_t _usAddress, uint16_t _usSize)
{
  do
  {
    if(HAL_I2C_Master_Receive_IT(&I2cHandle, (uint16_t)EE_DEV_ADDR, (uint8_t *)_pWriteBuf, _usSize) != HAL_OK)
    {
      /* Error_Handler() function is called when error occurs. */
      Error_Handler(__FILE__, __LINE__);
    }

    /*##-5- Wait for the end of the transfer #################################*/
    /*  Before starting a new communication transfer, you need to check the current
        state of the peripheral; if it�s busy you need to wait for the end of current
        transfer before starting a new one.
        For simplicity reasons, this example is just waiting till the end of the
        transfer, but application may perform other tasks while transfer operation
        is ongoing. */
    while (HAL_I2C_GetState(&I2cHandle) != HAL_I2C_STATE_READY)
    {
    }

    /* When Acknowledge failure occurs (Slave don't acknowledge it's address)
       Master restarts communication */
  }
  while(HAL_I2C_GetError(&I2cHandle) == HAL_I2C_ERROR_AF);

  return 0;
}

#endif