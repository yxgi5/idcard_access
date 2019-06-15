/**
  ******************************************************************************
  * @file    bsp_tim_pwm.h
  * @author  Andreas Zhang
  * @version V1.0
  * @date    05-June-2019
  * @brief   BSP TIM driver header.
  ******************************************************************************
*/

#ifndef BSP_TIM_PWM_H_
#define BSP_TIM_PWM_H_

void bsp_SetTIMOutPWM(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, TIM_TypeDef* TIMx, uint8_t _ucChannel,
   uint32_t _ulFreq, uint32_t _ulDutyCycle);

//void bsp_SetTIMOutPWM_N(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, TIM_TypeDef* TIMx, uint8_t _ucChannel,
//   uint32_t _ulFreq, uint32_t _ulDutyCycle);

void bsp_SetTIMforInt(TIM_TypeDef* TIMx, uint32_t _ulFreq, uint8_t _PreemptionPriority, uint8_t _SubPriority);

#endif /* BSP_TIM_PWM_H_ */
