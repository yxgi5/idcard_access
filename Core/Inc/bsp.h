/**
  ******************************************************************************
  * @file    bsp.h
  * @author  Andreas Zhang
  * @version V1.0
  * @date    05-June-2019
  * @brief   BSP header all in one.
  ******************************************************************************
*/

#ifndef INC_BSP_H_
#define INC_BSP_H_

// define bsp version
#define __STM32L051_BSP_VERSION   "1.0"

//#define CPU_IDLE()    bsp_Idle()

// set the Priority Mask Register
#define ENABLE_INT()  __set_PRIMASK(0)
#define DISABLE_INT() __set_PRIMASK(1)

#define BSP_Printf    printf
//#define BSP_Printf(...)

#include "stm32l0xx_hal.h"
#include "stm32l0xx.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef TRUE
  #define TRUE  1
#endif

#ifndef FALSE
  #define FALSE 0
#endif

#define ERROR_HANDLER()   Error_Handler(__FILE__, __LINE__);

#include "bsp_uart_fifo.h"
#include "bsp_led.h"
#include "bsp_timer.h"
#include "bsp_key.h"
//#include "bsp_exti.h"
//#include "bsp_modbus.h"
//#include "bsp_user_lib.h"

void bsp_Idle(void);
void bsp_Init(void);
//static void NVIC_Configuration(void);

#endif /* INC_BSP_H_ */
