/*
 * bsp_motor.c
 *
 *  Created on: Jun 30, 2019
 *      Author: andy
 */

#include "bsp.h"

static void BSP_MOTOR_Init(Motor_TypeDef Motor);

static GPIO_TypeDef* GPIO_PORT[MOTORn] = {GPIO_PORT_MOTOR1};
static const uint16_t GPIO_PIN[MOTORn] = {GPIO_PIN_MOTOR1};

void bsp_InitMotor(void)
{
  BSP_MOTOR_Init(MOTOR1);
}

/**
  * @brief  Configures MOTOR GPIO.
  * @param  Led: Specifies the Led to be configured.
  *   This parameter can be one of following parameters:
  *     @arg MOTOR1
  */
void BSP_MOTOR_Init(Motor_TypeDef Motor)
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  /* Enable the GPIO Clock */
  GPIO_CLK_ENABLE_MOTORx(Motor);

  /* Configure the GPIO_LED pin */
  GPIO_InitStruct.Pin = GPIO_PIN[Motor];
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;

  bsp_MotorOff(Motor);
  HAL_GPIO_Init(GPIO_PORT[Motor], &GPIO_InitStruct);

  HAL_GPIO_WritePin(GPIO_PORT[Motor], GPIO_PIN[Motor], GPIO_PIN_SET);
}


void bsp_MotorOn(Motor_TypeDef _no)
{
//  _no--;

  if (_no == 0)
  {
    GPIO_PORT_MOTOR1->BSRR = (uint32_t)GPIO_PIN_MOTOR1 << 16U;
  }
}

void bsp_MotorOff(Motor_TypeDef _no)
{
  //  _no--;

  if (_no == 0)
  {
      GPIO_PORT_MOTOR1->BSRR = GPIO_PIN_MOTOR1;
  }
}

uint8_t bsp_IsMotorOn(Motor_TypeDef _no)
{
  if (_no == 0)
  {
    if ((GPIO_PORT_MOTOR1->ODR & GPIO_PIN_MOTOR1) == 0)
    {
      return 1;
    }
    return 0;
  }

  return 0;
}

void bsp_Motor_Run(uint32_t Run_Time)
{
  bsp_MotorOn(MOTOR1);
  bsp_LedOn(LED2);
  HAL_Delay(Run_Time);
  bsp_MotorOff(MOTOR1);
  bsp_LedOff(LED2);
}

