/**
  ******************************************************************************
  * @file    bsp_uart_fifo.h
  * @author  Andreas Zhang
  * @version V1.0
  * @date    05-June-2019
  * @brief   BSP uart driver header.
  ******************************************************************************
*/

#ifndef BSP_UART_FIFO_H_
#define BSP_UART_FIFO_H_

/*
  Uart1:
  PA9/UART1_TX
  PA10/UART1_RX
 */
#define UART1_FIFO_EN 1
#define UART2_FIFO_EN 0

///* PB2 控制RS485芯片的发送使能 */
//#define RS485_TXEN_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
//#define RS485_TXEN_GPIO_PORT              GPIOB
//#define RS485_TXEN_PIN                    GPIO_PIN_2
//
//#define RS485_RX_EN() RS485_TXEN_GPIO_PORT->BSRR = (uint32_t)RS485_TXEN_PIN << 16U
//#define RS485_TX_EN() RS485_TXEN_GPIO_PORT->BSRR = RS485_TXEN_PIN

/* 定义端口号 */
typedef enum
{
  COM1 = 0, /* USART1  PA9, PA10 */
  COM2 = 1, /* USART2, PA2, PA3 */
}COM_PORT_E;

/* 定义串口波特率和FIFO缓冲区大小，分为发送缓冲区和接收缓冲区, 支持全双工 */
#if UART1_FIFO_EN == 1
  #define UART1_BAUD      115200
  #define UART1_TX_BUF_SIZE 1*1024
  #define UART1_RX_BUF_SIZE 1*1024
#endif

#if UART2_FIFO_EN == 1
  #define UART2_BAUD      9600
  #define UART2_TX_BUF_SIZE 10
  #define UART2_RX_BUF_SIZE 2*1024
#endif

/* 串口设备结构体 */
typedef struct
{
  USART_TypeDef *uart;    /* STM32内部串口设备指针 */
  uint8_t *pTxBuf;      /* 发送缓冲区 */
  uint8_t *pRxBuf;      /* 接收缓冲区 */
  uint16_t usTxBufSize;   /* 发送缓冲区大小 */
  uint16_t usRxBufSize;   /* 接收缓冲区大小 */
  __IO uint16_t usTxWrite;  /* 发送缓冲区写指针 */
  __IO uint16_t usTxRead;   /* 发送缓冲区读指针 */
  __IO uint16_t usTxCount;  /* 等待发送的数据个数 */

  __IO uint16_t usRxWrite;  /* 接收缓冲区写指针 */
  __IO uint16_t usRxRead;   /* 接收缓冲区读指针 */
  __IO uint16_t usRxCount;  /* 还未读取的新数据个数 */

  void (*SendBefor)(void);  /* 开始发送之前的回调函数指针（主要用于RS485切换到发送模式） */
  void (*SendOver)(void);   /* 发送完毕的回调函数指针（主要用于RS485将发送模式切换为接收模式） */
  void (*ReciveNew)(uint8_t _byte); /* 串口收到数据的回调函数指针 */
  uint8_t Sending;      /* 正在发送中 */
}UART_T;

void bsp_InitUart(void);
void comSendBuf(COM_PORT_E _ucPort, uint8_t *_ucaBuf, uint16_t _usLen);
void comSendChar(COM_PORT_E _ucPort, uint8_t _ucByte);
uint8_t comGetChar(COM_PORT_E _ucPort, uint8_t *_pByte);
void comClearTxFifo(COM_PORT_E _ucPort);
void comClearRxFifo(COM_PORT_E _ucPort);
void comSetBaud(COM_PORT_E _ucPort, uint32_t _BaudRate);

void USART_SetBaudRate(USART_TypeDef* USARTx, uint32_t BaudRate);
void bsp_SetUartParam(USART_TypeDef *Instance,  uint32_t BaudRate, uint32_t Parity, uint32_t Mode);

//void RS485_SendBuf(uint8_t *_ucaBuf, uint16_t _usLen);
//void RS485_SendStr(char *_pBuf);
//void RS485_SetBaud(uint32_t _baud);
uint8_t UartTxEmpty(COM_PORT_E _ucPort);

#endif /* BSP_UART_FIFO_H_ */

