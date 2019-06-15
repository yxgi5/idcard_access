/**
  ******************************************************************************
  * @file    bsp_tim_pwm.c
  * @author  Andreas Zhang
  * @version V1.0
  * @date    05-June-2019
  * @brief   BSP TIM driver source.
  ******************************************************************************
*/

#include "bsp.h"

/*
  可以输出到GPIO的TIM通道:

  TIM2_CH1, PA0
  TIM2_CH2, PA1
  TIM2_CH3, PA2
  TIM2_CH4, PA3

  TIM21_CH1, PB13 或 PA2
  TIM21_CH2, PB14 或 PA3

  TIM22_CH1, PA6
  TIM22_CH2, PA7

  APB1 定时器有 TIM2, TIM6
  APB2 定时器有 TIM21, TIM22
*/

/*
*********************************************************************************************************
* 函 数 名: bsp_RCC_GPIO_Enable
* 功能说明: 使能GPIO时钟
* 形    参: GPIOx GPIOA - GPIOK
* 返 回 值: 无
*********************************************************************************************************
*/
void bsp_RCC_GPIO_Enable(GPIO_TypeDef* GPIOx)
{
  if (GPIOx == GPIOA) __HAL_RCC_GPIOA_CLK_ENABLE();
  else if (GPIOx == GPIOB) __HAL_RCC_GPIOB_CLK_ENABLE();
  else if (GPIOx == GPIOC) __HAL_RCC_GPIOC_CLK_ENABLE();
  else if (GPIOx == GPIOD) __HAL_RCC_GPIOD_CLK_ENABLE();
  else if (GPIOx == GPIOH) __HAL_RCC_GPIOH_CLK_ENABLE();
}

/*
*********************************************************************************************************
* 函 数 名: bsp_RCC_TIM_Enable
* 功能说明: 使能TIM RCC 时钟
* 形    参: TIMx TIM1 - TIM17
* 返 回 值: 无
*********************************************************************************************************
*/
void bsp_RCC_TIM_Enable(TIM_TypeDef* TIMx)
{
  if (TIMx == TIM2) __HAL_RCC_TIM2_CLK_ENABLE();
//  else if (TIMx == TIM6) __HAL_RCC_TIM6_CLK_ENABLE();
  else if (TIMx == TIM21) __HAL_RCC_TIM21_CLK_ENABLE();
  else if (TIMx == TIM22) __HAL_RCC_TIM22_CLK_ENABLE();
  else
  {
    Error_Handler(__FILE__, __LINE__);
  }
}

/*
*********************************************************************************************************
* 函 数 名: bsp_RCC_TIM_Disable
* 功能说明: 关闭TIM RCC 时钟
* 形    参: TIMx TIM1 - TIM17
* 返 回 值: TIM外设时钟名
*********************************************************************************************************
*/
void bsp_RCC_TIM_Disable(TIM_TypeDef* TIMx)
{
  /*
    APB1 定时器有 TIM2, TIM6
    APB2 定时器有 TIM21, TIM22
  */
  if (TIMx == TIM2) __HAL_RCC_TIM2_CLK_DISABLE();
//  else if (TIMx == TIM6) __HAL_RCC_TIM6_CLK_DISABLE();
  else if (TIMx == TIM21) __HAL_RCC_TIM21_CLK_DISABLE();
  else if (TIMx == TIM22) __HAL_RCC_TIM22_CLK_DISABLE();
  else
  {
    Error_Handler(__FILE__, __LINE__);
  }
}

/*
*********************************************************************************************************
* 函 数 名: bsp_GetAFofTIM
* 功能说明: 根据TIM 得到AF寄存器配置
* 形    参: TIMx: TIM2 TIM21 TIM22
* 返 回 值: AF寄存器配置
*********************************************************************************************************
*/
uint8_t bsp_GetAFofTIM(TIM_TypeDef* TIMx)
{
  uint8_t ret = 0;

  if (TIMx == TIM2) ret = GPIO_AF5_TIM2;        // TODO: depends on pin definition
  else if (TIMx == TIM21) ret = GPIO_AF5_TIM21; // TODO: depends on pin definition
  else if (TIMx == TIM22) ret = GPIO_AF4_TIM22; // TODO: depends on pin definition
  else
  {
    Error_Handler(__FILE__, __LINE__);
  }

  return ret;
}

/*
*********************************************************************************************************
* 函 数 名: bsp_ConfigTimGpio
* 功能说明: 配置GPIO和TIM时钟， GPIO连接到TIM输出通道
* 形    参: GPIOx : GPIOA - GPIOD, GPIOH
*       GPIO_PinX : GPIO_PIN_0 - GPIO__PIN_15
*       TIMx : TIM2 TIM21 TIM22
* 返 回 值: 无
*********************************************************************************************************
*/
void bsp_ConfigTimGpio(GPIO_TypeDef* GPIOx, uint16_t GPIO_PinX, TIM_TypeDef* TIMx)
{
  GPIO_InitTypeDef   GPIO_InitStruct;

  /* 使能GPIO时钟 */
  bsp_RCC_GPIO_Enable(GPIOx);

    /* 使能TIM时钟 */
  bsp_RCC_TIM_Enable(TIMx);

  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = bsp_GetAFofTIM(TIMx);
  GPIO_InitStruct.Pin = GPIO_PinX;
  HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

/*
*********************************************************************************************************
* 函 数 名: bsp_ConfigGpioOut
* 功能说明: 配置GPIO为推挽输出。主要用于PWM输出，占空比为0和100的情况。
* 形    参:  GPIOx : GPIOA - GPIOD, GPIOH
*       GPIO_PinX : GPIO_PIN_0 - GPIO__PIN_15
* 返 回 值: 无
*********************************************************************************************************
*/
void bsp_ConfigGpioOut(GPIO_TypeDef* GPIOx, uint16_t GPIO_PinX)
{
  GPIO_InitTypeDef   GPIO_InitStruct;

  bsp_RCC_GPIO_Enable(GPIOx);   /* 使能GPIO时钟 */

  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Pin = GPIO_PinX;
  HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

/*
*********************************************************************************************************
* 函 数 名: bsp_SetTIMOutPWM
* 功能说明: 设置引脚输出的PWM信号的频率和占空比.  当频率为0，并且占空为0时，关闭定时器，GPIO输出0；
*       当频率为0，占空比为100%时，GPIO输出1.
* 形    参: GPIOx : GPIOA - GPIOD, GPIOH
*       GPIO_Pin : GPIO_PIN_0 - GPIO__PIN_15
*       TIMx : TIM2 TIM21 TIM22
*             _ucChannel：使用的定时器通道，范围1 - 4
*       _ulFreq : PWM信号频率，单位Hz (实际测试，可以输出100MHz），0 表示禁止输出
*       _ulDutyCycle : PWM信号占空比，单位: 万分之一。如5000，表示50.00%的占空比
* 返 回 值: 无
*********************************************************************************************************
*/
void bsp_SetTIMOutPWM(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, TIM_TypeDef* TIMx, uint8_t _ucChannel,
   uint32_t _ulFreq, uint32_t _ulDutyCycle)
{
  TIM_HandleTypeDef  TimHandle = {0};
  TIM_OC_InitTypeDef sConfig = {0};
  uint16_t usPeriod;
  uint16_t usPrescaler;
  uint32_t pulse;
  uint32_t uiTIMxCLK;
  const uint16_t TimChannel[4+1] = {0, TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3, TIM_CHANNEL_4};

  if (_ucChannel > 6)
  {
    Error_Handler(__FILE__, __LINE__);
  }

  if (_ulDutyCycle == 0)
  {
    //bsp_RCC_TIM_Disable(TIMx);    /* 关闭TIM时钟, 可能影响其他通道 */
    bsp_ConfigGpioOut(GPIOx, GPIO_Pin); /* 配置GPIO为推挽输出 */
    GPIOx->BSRR = (uint32_t)GPIO_Pin << 16U;    /* PWM = 0 */
    return;
  }
  else if (_ulDutyCycle == 10000)
  {
    //bsp_RCC_TIM_Disable(TIMx);    /* 关闭TIM时钟, 可能影响其他通道 */
    bsp_ConfigGpioOut(GPIOx, GPIO_Pin); /* 配置GPIO为推挽输出 */
    GPIOx->BSRR = GPIO_Pin;    /* PWM = 1*/
    return;
  }

  /* 下面是PWM输出 */

  bsp_ConfigTimGpio(GPIOx, GPIO_Pin, TIMx); /* 使能GPIO和TIM时钟，并连接TIM通道到GPIO */

  /*-----------------------------------------------------------------------
    see bsp.c 文件中 void SystemClock_Config(void) 函数对时钟的配置:
  ----------------------------------------------------------------------- */
  if ((TIMx == TIM21) || (TIMx == TIM22))
  {
    /* APB2 时钟 */
    uiTIMxCLK = SystemCoreClock;
  }
  else
  {
    /* APB1 时钟 */
    uiTIMxCLK = SystemCoreClock;
  }

  if (_ulFreq < 100)
  {
    usPrescaler = 10000 - 1;          /* 分频比 = 10000 */
    usPeriod =  (uiTIMxCLK / 10000) / _ulFreq  - 1;   /* 自动重装的值 */
  }
  else if (_ulFreq < 3000)
  {
    usPrescaler = 100 - 1;          /* 分频比 = 100 */
    usPeriod =  (uiTIMxCLK / 100) / _ulFreq  - 1;   /* 自动重装的值 */
  }
  else  /* 大于4K的频率，无需分频 */
  {
    usPrescaler = 0;          /* 分频比 = 1 */
    usPeriod = uiTIMxCLK / _ulFreq - 1; /* 自动重装的值 */
  }
  pulse = (_ulDutyCycle * usPeriod) / 10000;


  HAL_TIM_PWM_DeInit(&TimHandle);

  /*  PWM频率 = TIMxCLK / usPrescaler + 1）/usPeriod + 1）*/
  TimHandle.Instance = TIMx;
  TimHandle.Init.Prescaler         = usPrescaler;
  TimHandle.Init.Period            = usPeriod;
  TimHandle.Init.ClockDivision     = 0;
  TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
//  TimHandle.Init.RepetitionCounter = 0;
  TimHandle.Init.AutoReloadPreload = 0;
  if (HAL_TIM_PWM_Init(&TimHandle) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }

  /* 配置定时器PWM输出通道 */
  sConfig.OCMode       = TIM_OCMODE_PWM1;
  sConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;
  sConfig.OCFastMode   = TIM_OCFAST_DISABLE;
//  sConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
//  sConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;
//  sConfig.OCIdleState  = TIM_OCIDLESTATE_RESET;

  /* 占空比 */
  sConfig.Pulse = pulse;
  if (HAL_TIM_PWM_ConfigChannel(&TimHandle, &sConfig, TimChannel[_ucChannel]) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }

  /* 启动PWM输出 */
  if (HAL_TIM_PWM_Start(&TimHandle, TimChannel[_ucChannel]) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }
}


/*
*********************************************************************************************************
* 函 数 名: bsp_SetTIMforInt
* 功能说明: 配置TIM和NVIC，用于简单的定时中断，开启定时中断。
*         注意中断服务程序需要由用户应用程序实现。
* 形    参: TIMx : 定时器
*       _ulFreq : 定时频率 （Hz）。 0 表示关闭。
*       _PreemptionPriority : 抢占优先级
*       _SubPriority : 子优先级
* 返 回 值: 无
*********************************************************************************************************
*/
/*
TIM定时中断服务程序范例，必须清中断标志
void TIM6_IRQn(void)
{
  if((TIM6->SR & TIM_FLAG_UPDATE) != RESET)
  {
    TIM6->SR = ~ TIM_FLAG_UPDATE;
    //添加用户代码
  }
}
*/
void bsp_SetTIMforInt(TIM_TypeDef* TIMx, uint32_t _ulFreq, uint8_t _PreemptionPriority, uint8_t _SubPriority)
{
  TIM_HandleTypeDef   TimHandle = {0};
  uint16_t usPeriod;
  uint16_t usPrescaler;
  uint32_t uiTIMxCLK;

  /* 使能TIM时钟 */
  bsp_RCC_TIM_Enable(TIMx);

    /*-----------------------------------------------------------------------
    bsp.c 文件中 void SystemClock_Config(void) 函数对时钟的配置
  ----------------------------------------------------------------------- */
  if ((TIMx == TIM21) || (TIMx == TIM22))
  {
    /* APB2 定时器时钟 */
    uiTIMxCLK = SystemCoreClock;
  }
  else
  {
    /* APB1 定时器 */
    uiTIMxCLK = SystemCoreClock;
  }

  if (_ulFreq < 100)
  {
    usPrescaler = 10000 - 1;          /* 分频比 = 10000 */
    usPeriod =  (uiTIMxCLK / 10000) / _ulFreq  - 1;   /* 自动重装的值 */
  }
  else if (_ulFreq < 3000)
  {
    usPrescaler = 100 - 1;          /* 分频比 = 100 */
    usPeriod =  (uiTIMxCLK / 100) / _ulFreq  - 1;   /* 自动重装的值 */
  }
  else  /* 大于4K的频率，无需分频 */
  {
    usPrescaler = 0;          /* 分频比 = 1 */
    usPeriod = uiTIMxCLK / _ulFreq - 1; /* 自动重装的值 */
  }

  /*
       定时器中断更新周期 = TIMxCLK / usPrescaler + 1）/usPeriod + 1）
  */
  TimHandle.Instance = TIMx;
  TimHandle.Init.Prescaler         = usPrescaler;
  TimHandle.Init.Period            = usPeriod;
  TimHandle.Init.ClockDivision     = 0;
  TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
//  TimHandle.Init.RepetitionCounter = 0;
  TimHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&TimHandle) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }

  /* 使能定时器中断  */
  __HAL_TIM_ENABLE_IT(&TimHandle, TIM_IT_UPDATE);


  /* 配置TIM定时更新中断 (Update) */
  {
        uint8_t irq = 0;  /* 中断号, 定义在 stm32h7xx.h */

        if (TIMx == TIM2) irq = TIM2_IRQn;
        else if (TIMx == TIM6) irq = TIM6_IRQn;
        else if (TIMx == TIM21) irq = TIM21_IRQn;
        else if (TIMx == TIM22) irq = TIM22_IRQn;
        else
        {
            Error_Handler(__FILE__, __LINE__);
        }
        HAL_NVIC_SetPriority((IRQn_Type)irq, _PreemptionPriority, _SubPriority);
        HAL_NVIC_EnableIRQ((IRQn_Type)irq);
  }

  HAL_TIM_Base_Start(&TimHandle);
}
