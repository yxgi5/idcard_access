/*
 * bsp_motor.h
 *
 *  Created on: Jun 30, 2019
 *      Author: andy
 */

#ifndef INC_BSP_MOTOR_H_
#define INC_BSP_MOTOR_H_

typedef enum
{
  MOTOR1 = 0,
} Motor_TypeDef;

#define MOTORn  1

#define GPIO_PORT_MOTOR1  GPIOB
#define GPIO_PIN_MOTOR1 GPIO_PIN_0
#define GPIO_CLK_ENABLE_MOTOR1()           __HAL_RCC_GPIOB_CLK_ENABLE()
#define GPIO_CLK_DISABLE_MOTOR1()          __HAL_RCC_GPIOB_CLK_DISABLE()

#define ALL_LED_GPIO_CLK_ENABLE() { \
  __HAL_RCC_GPIOB_CLK_ENABLE(); \
};

#define GPIO_CLK_ENABLE_MOTORx(__INDEX__) do{if((__INDEX__) == 0) GPIO_CLK_ENABLE_MOTOR1(); \
                                         }while(0)

#define GPIO_CLK_DISABLE_MOTORx(__INDEX__) do{if((__INDEX__) == 0) GPIO_CLK_DISABLE_MOTOR1(); \
                                          }while(0)

void bsp_InitMotor(void);
void bsp_MotorOn(uint8_t _no);
void bsp_MotorOff(uint8_t _no);
uint8_t bsp_IsMotorOn(uint8_t _no);
void bsp_Motor_Run(uint32_t Run_Time);

#endif /* INC_BSP_MOTOR_H_ */
