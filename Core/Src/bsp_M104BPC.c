/*
 * bsp_M104BPC.c
 *
 *  Created on: Jun 30, 2019
 *      Author: andy
 */


#include "bsp.h"

uint8_t g_cComReceiveBuffer[43];
uint8_t g_cPackageStarted;
uint8_t g_cReceiveCounter;
uint8_t g_cCheckSum;

uint8_t RX_Data;

uint8_t g_bReceiveOK;
uint8_t g_b0x10Received;
uint8_t g_bCard;
uint8_t busy;


uint8_t COMM_MIFARE1_PCD_REQUEST_[]
= {0X00,0X00,
   0x04,
   COMM_MIFARE1_PCD_REQUEST,
   0x52};
uint8_t COMM_MIFARE1_PCD_ANTICOLL_[]
= {0X00,0X00,
   0x04,
   COMM_MIFARE1_PCD_ANTICOLL,
   0x04};
uint8_t COMM_MIFARE1_PCD_SELECT_[]
= {0X00,0X00,
   0x07,
   COMM_MIFARE1_PCD_SELECT,
   0x00,0x00,0x00,0x00};
uint8_t COMM_AUTHENTICATION2_[]
= {0X00,0X00,
   0x0B,
   COMM_AUTHENTICATION2,
   0x60,0x01,0xff,0xff,0xff,0xff,0xff,0xff};
uint8_t COMM_PCD_READ_BLOCK_[]
= {0X00,0X00,
   0x04,
   COMM_PCD_READ_BLOCK,
   0x01};
uint8_t COMM_PCD_WRITE_BLOCK_[]
= {0X00,0X00,
   0x14,
   COMM_PCD_WRITE_BLOCK,
   0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

void UartSend(unsigned char * cpBUFFER)  //发送字符串
{
  unsigned char i;
  g_bReceiveOK = 0;

  g_cCheckSum = 0;

  comSendChar(COM1, 0x02);

  for (i = 0; i < (cpBUFFER[2] + 1); i++)
  {
    if(( cpBUFFER[i] == 0x02 ) || ( cpBUFFER[i] == 0x03 ) || ( cpBUFFER[i] == 0x10))
    {
      comSendChar(COM1, 0x10);
    }
    comSendChar(COM1, cpBUFFER[i]);

    g_cCheckSum += cpBUFFER[i];
  }
  if (( g_cCheckSum == 0x02 ) || ( g_cCheckSum == 0x03 ) || ( g_cCheckSum == 0x10))
  {
    comSendChar(COM1, 0x10);
  }
  comSendChar(COM1, g_cCheckSum);

  comSendChar(COM1, 0x03);
  g_cReceiveCounter = 0;
}

/*****************************************************************************
*功能：接收上位机数据
*****************************************************************************/
void bsp_m104bpc_uart_callback(uint8_t i)
{
    if (!g_bReceiveOK)    //接收该包数据
    {
      if ((0x02 == i) && (0 == g_b0x10Received))
      {
        g_cPackageStarted = 1;
        g_cReceiveCounter = 0;
        g_cCheckSum = 0;
      }
      else if (( 0x03 == i) && (0 == g_b0x10Received) && (g_cPackageStarted))   //package end
      {       //check package
        g_cPackageStarted = 0;
        if(g_cReceiveCounter < sizeof(g_cComReceiveBuffer) - 2)
        {
          g_bReceiveOK = 1;
          g_cPackageStarted = 0;
          g_cReceiveCounter = 0;
        }
      }
      else if (( 0x10 == i ) && (0 == g_b0x10Received))
      {
        g_b0x10Received = 1;
      }
      else if (g_cPackageStarted)
      {
        g_b0x10Received = 0;

        if (g_cReceiveCounter < sizeof(g_cComReceiveBuffer) - 2)
        {
          if (g_cReceiveCounter != 0)
          {
            g_cCheckSum += g_cComReceiveBuffer[g_cReceiveCounter-1];
          }
          g_cComReceiveBuffer[g_cReceiveCounter++] = i;
        }
        else
        {
          g_cPackageStarted     = 0;    //标准串口接收包起始标志
          g_cReceiveCounter   = 0;
        }
      }
      else
      {
         g_b0x10Received = 0;   //没收到02头时，若收到10就永远不再接受
      }
    }
}

uint8_t ComSearchCard_Function(void)
{
    unsigned int cCnt;
    UartSend(COMM_MIFARE1_PCD_REQUEST_);
    for (cCnt=65000; ((cCnt > 0) && !g_bReceiveOK)>0; cCnt--)
    {
    }
    if((g_bReceiveOK)&&(g_cComReceiveBuffer[5]==0x04))
    {
      return SUCCESS;
    }
    else if (((g_cComReceiveBuffer[4])!=0) || (0 == cCnt))
    {
      return FAILURE;
    }
    else
    {
        return FAILURE;
    }
}

uint8_t ComAnticollCard_Function(void)
{
    unsigned int cCnt;
    UartSend(COMM_MIFARE1_PCD_ANTICOLL_);
    for (cCnt=65000; ((cCnt > 0) && !g_bReceiveOK)>0; cCnt--)
    {
    }
    if((g_bReceiveOK)&&(g_cComReceiveBuffer[2]==0x07))
    {
      return SUCCESS;
    }
    else if (((g_cComReceiveBuffer[4])!=0) || (0 == cCnt))
    {
      return FAILURE;
    }
    else
    {
        return FAILURE;
    }
}

uint8_t ComSelectCard_Function(void)
{
    unsigned int cCnt;
    UartSend(COMM_MIFARE1_PCD_SELECT_);
    for (cCnt=65000; ((cCnt > 0) && !g_bReceiveOK)>0; cCnt--)
    {
    }
    if((g_bReceiveOK)&&(g_cComReceiveBuffer[5]==0x08))
    {
      return SUCCESS;
    }
    else if (((g_cComReceiveBuffer[4])!=0) || (0 == cCnt))
    {
      return FAILURE;
    }
    else
    {
        return FAILURE;
    }
}

uint8_t ComCheckCard_Function(void)
{
    unsigned int cCnt;
    UartSend(COMM_AUTHENTICATION2_);
    for (cCnt=65000; ((cCnt > 0) && !g_bReceiveOK)>0; cCnt--)
    {
    }
    if(g_bReceiveOK)
    {
      return SUCCESS;
    }
    else if (((g_cComReceiveBuffer[4])!=0) || (0 == cCnt))
    {
      return FAILURE;
    }
    else
    {
        return FAILURE;
    }
}


uint8_t ComReadBlock5_Function(void)
{
    unsigned int cCnt;
    UartSend(COMM_PCD_READ_BLOCK_);
    for (cCnt=65000; ((cCnt > 0) && !g_bReceiveOK)>0; cCnt--)
    {
    }
    if((g_bReceiveOK)&&(g_cComReceiveBuffer[2]==0x13))
    {
      return SUCCESS;
    }
    else if (((g_cComReceiveBuffer[4])!=0) || (0 == cCnt))
    {
      return FAILURE;
    }
    else
    {
        return FAILURE;
    }
}

uint8_t ComWriteBlock5_Function(void)
{
    unsigned int cCnt;
    UartSend(COMM_PCD_WRITE_BLOCK_);
    for (cCnt=65000; ((cCnt > 0) && !g_bReceiveOK)>0; cCnt--)
    {
    }
    if(g_bReceiveOK)
    {
      return SUCCESS;
    }
    else if (((g_cComReceiveBuffer[4])!=0) || (0 == cCnt))
    {
      return FAILURE;
    }
    else
    {
        return FAILURE;
    }
}

