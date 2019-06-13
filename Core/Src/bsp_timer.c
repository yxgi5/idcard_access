/**
  ******************************************************************************
  * @file    bsp_timer.c
  * @author  Andreas Zhang
  * @version V1.0
  * @date    05-June-2019
  * @brief   BSP soft timer driver source.
  ******************************************************************************
*/
#include "bsp.h"

#define USE_TIM2
//#define USE_TIM21
//#define USE_TIM22

#ifdef USE_TIM2
  #define TIM_HARD          TIM2
  #define RCC_TIM_HARD_CLK_ENABLE() __HAL_RCC_TIM2_CLK_ENABLE()
  #define TIM_HARD_IRQn       TIM2_IRQn
  #define TIM_HARD_IRQHandler     TIM2_IRQHandler
#endif
// TODO: USE TIM21/TIM22/TIM6

static void (*s_TIM_CallBack1)(void);
static void (*s_TIM_CallBack2)(void);
static void (*s_TIM_CallBack3)(void);
static void (*s_TIM_CallBack4)(void);

// used by bsp_DelayMS()
static volatile uint32_t s_uiDelayCount = 0;
static volatile uint8_t s_ucTimeOutFlag = 0;

static SOFT_TMR s_tTmr[TMR_COUNT] = {0};

// global running time, unit 1ms
// max 24.85 days
__IO int32_t g_iRunTime = 0;

// 1==enable, 0==disable
static __IO uint8_t g_ucEnableSystickISR = 0;

static void bsp_SoftTimerDec(SOFT_TMR *_tmr);

/**
  * @brief  config systick intterupt, initialize variables.
  * @param  None
  * @retval None
  */
void bsp_InitTimer(void)
{
  uint8_t i;

  // clear all soft cnt
  for (i = 0; i < TMR_COUNT; i++)
  {
      s_tTmr[i].Count = 0;
      s_tTmr[i].PreLoad = 0;
      s_tTmr[i].Flag = 0;
      s_tTmr[i].Mode = TMR_ONCE_MODE; // default one-shot
  }

  // config systick 1ms，enable systick intr
  // SystemCoreClock 32MHz for STM32L0XX
  //  SystemCoreClock / 1000 == 1000Hz == 1ms
  //  SystemCoreClock / 500 == 500Hz == 2ms
  //  SystemCoreClock / 2000 == 2000Hz == 500us
  SysTick_Config(SystemCoreClock / 1000);

  g_ucEnableSystickISR = 1; // 1 == enable

  bsp_InitHardTimer();
}

/**
  * @brief  SysTick_ISR
  * @param  None
  * @retval None
  */
extern void bsp_RunPer1ms(void);
extern void bsp_RunPer10ms(void);
void SysTick_ISR(void)
{
  static uint8_t s_count = 0;
  uint8_t i;

  // every 1ms. only for bsp_DelayMS
  if (s_uiDelayCount > 0)
  {
    if (--s_uiDelayCount == 0)
    {
      s_ucTimeOutFlag = 1;
    }
  }
  // every 1ms, cnt = cnt -1
  for (i = 0; i < TMR_COUNT; i++)
  {
    bsp_SoftTimerDec(&s_tTmr[i]);
  }

  g_iRunTime++;
  if (g_iRunTime == 0x7FFFFFFF) // int32_t type，max value 0x7FFFFFFF
  {
    g_iRunTime = 0;
  }

  bsp_RunPer1ms();  // every 1ms. see bsp.c

  if (++s_count >= 10)
  {
    s_count = 0;

    bsp_RunPer10ms(); // every 10 ms. see bsp.c
  }
}

/**
  * @brief  every 1ms called by SysTick_ISR
  * @param  _tmr, counter variable pointer
  * @retval None
  */
static void bsp_SoftTimerDec(SOFT_TMR *_tmr)
{
  if (_tmr->Count > 0)
  {
    /* counter decrease to 1, set flag */
    if (--_tmr->Count == 0)
    {
      _tmr->Flag = 1;

      /* reload if use AUTO_MODE */
      if(_tmr->Mode == TMR_AUTO_MODE)
      {
        _tmr->Count = _tmr->PreLoad;
      }
    }
  }
}

/**
  * @brief  delay ms, +-1ms
  * @param  n, delay n ms, n > 2
  * @retval None
  */
void bsp_DelayMS(uint32_t n)
{
  if (n == 0)
  {
    return;
  }
  else if (n == 1)
  {
    n = 2;
  }

  DISABLE_INT();

  s_uiDelayCount = n;
  s_ucTimeOutFlag = 0;

  ENABLE_INT();

  while (1)
  {
    bsp_Idle();

    // wait timeout
    if (s_ucTimeOutFlag == 1)
    {
      break;
    }
  }
}

/**
  * @brief  delay us, +-1ms
  * @param  n, delay n us, must be called after SysTick runs
  * @retval None
  */
void bsp_DelayUS(uint32_t n)
{
  uint32_t ticks;
  uint32_t told;
  uint32_t tnow;
  uint32_t tcnt = 0;
  uint32_t reload;

  reload = SysTick->LOAD;
  ticks = n * (SystemCoreClock / 1000000);

  tcnt = 0;
  told = SysTick->VAL;// time old

  while (1)
  {
      tnow = SysTick->VAL;
      if (tnow != told)
      {
          /* SYSTICK is a dec TIM */
          if (tnow < told)
          {
              tcnt += told - tnow;
          }
          /* RELOAD */
          else
          {
              tcnt += reload - tnow + told;
          }
          told = tnow;

          /* quit condition: timeout */
          if (tcnt >= ticks)
          {
            break;
          }
      }
  }
}

/**
  * @brief  run a soft timer in one-shot mode
  * @param  _id: soft timer ID [0,TMR_COUNT-1]
  *         _period: unit in ms
  * @retval None
  */
void bsp_StartTimer(uint8_t _id, uint32_t _period)
{
  if (_id >= TMR_COUNT)
  {
    BSP_Printf("Error: file %s, function %s()\r\n", __FILE__, __FUNCTION__);
    while(1); /* fautl handler */
  }

  DISABLE_INT();

  s_tTmr[_id].Count = _period;    /* timer init value */
  s_tTmr[_id].PreLoad = _period;    /* timer reload value，only for auto mode */
  s_tTmr[_id].Flag = 0;       /* timeout flag */
  s_tTmr[_id].Mode = TMR_ONCE_MODE; /* one-shot mode */

  ENABLE_INT();
}

/**
  * @brief  run a soft timer in auto mode
  * @param  _id: soft timer ID [0,TMR_COUNT-1]
  *         _period: unit in 1ms
  * @retval None
  */
void bsp_StartAutoTimer(uint8_t _id, uint32_t _period)
{
  if (_id >= TMR_COUNT)
  {
    BSP_Printf("Error: file %s, function %s()\r\n", __FILE__, __FUNCTION__);
    while(1); /* fautl handler */
  }

  DISABLE_INT();

  s_tTmr[_id].Count = _period;    /* timer init value */
  s_tTmr[_id].PreLoad = _period;    /* timer reload value，only for auto mode */
  s_tTmr[_id].Flag = 0;       /* timeout flag */
  s_tTmr[_id].Mode = TMR_AUTO_MODE; /* one-shot mode */

  ENABLE_INT();
}

/**
  * @brief  stop a soft timer
  * @param  _id: soft timer ID [0,TMR_COUNT-1]
  * @retval None
  */
void bsp_StopTimer(uint8_t _id)
{
  if (_id >= TMR_COUNT)
  {
    BSP_Printf("Error: file %s, function %s()\r\n", __FILE__, __FUNCTION__);
    while(1);  /* fautl handler */
  }

  DISABLE_INT();

  s_tTmr[_id].Count = 0;
  s_tTmr[_id].Flag = 0;
  s_tTmr[_id].Mode = TMR_ONCE_MODE;

  ENABLE_INT();
}

/**
  * @brief  check a soft timer if timeout
  * @param  _id: soft timer ID [0,TMR_COUNT-1]
  * @retval 0 not, 1 timeout
  */
uint8_t bsp_CheckTimer(uint8_t _id)
{
  if (_id >= TMR_COUNT)
  {
    return 0;
  }

  if (s_tTmr[_id].Flag == 1)
  {
    s_tTmr[_id].Flag = 0;
    return 1;
  }
  else
  {
    return 0;
  }
}

/**
  * @brief  get cpu running time, max 24.85 days
  * @param  None
  * @retval cpu running time unit in ms
  */
int32_t bsp_GetRunTime(void)
{
  int32_t runtime;

  DISABLE_INT();    /* 关中断 */

  runtime = g_iRunTime; /* 这个变量在Systick中断中被改写，因此需要关中断进行保护 */

  ENABLE_INT();     /* 开中断 */

  return runtime;
}

/**
  * @brief  get delta time between current time with a past time
  * @param  _LastTime
  * @retval unit in ms
  */
int32_t bsp_CheckRunTime(int32_t _LastTime)
{
  int32_t now_time;
  int32_t time_diff;

  DISABLE_INT();    /* 关中断 */

  now_time = g_iRunTime;  /* 这个变量在Systick中断中被改写，因此需要关中断进行保护 */

  ENABLE_INT();     /* 开中断 */

  if (now_time >= _LastTime)
  {
    time_diff = now_time - _LastTime;
  }
  else
  {
    time_diff = 0x7FFFFFFF - _LastTime + now_time;
  }

  return time_diff;
}

void SysTick_Handler(void)
{
  HAL_IncTick();

  if (g_ucEnableSystickISR == 0)
  {
    return;
  }

  SysTick_ISR();
}

#ifdef TIM_HARD

/**
  * @brief  config TIMx for us delay, TIMx will not stop
  *         TIM2 - TIM5
  *         input clk SystemCoreClock / 2 (APB1)
  * @param  None
  * @retval None
  */
void bsp_InitHardTimer(void)
{
  TIM_HandleTypeDef  TimHandle = {0};
  uint32_t usPeriod;
  uint16_t usPrescaler;
  uint32_t uiTIMxCLK;
  TIM_TypeDef* TIMx = TIM_HARD;

  RCC_TIM_HARD_CLK_ENABLE();
  /*-----------------------------------------------------------------------
  common config
  HCLK = SYSCLK / 1     (AHB1Periph)
  PCLK2 = HCLK / 2      (APB2Periph)
  PCLK1 = HCLK / 4      (APB1Periph)

  APB1 TIMxCLK = PCLK1 x 2 = SystemCoreClock / 2 = 84MHz;
  APB2 TIMxCLK = PCLK2 x 2 = SystemCoreClock = 168MHz;

  APB1(TIM2, TIM6, TIM7, LPTIM1)
  APB2(TIM21, TIM22)

----------------------------------------------------------------------- */
  if ((TIMx == TIM2) || (TIMx == TIM6))
  {
    /* APB1 = 32M */
    uiTIMxCLK = SystemCoreClock;
  }
//  else
//  {
//    /* APB2 = 32M */
//    uiTIMxCLK = SystemCoreClock / 2;
//  }

    usPrescaler = uiTIMxCLK / 1000000 - 1;  /* count 1 for 1us */

    if (TIMx == TIM2)
    {
//      usPeriod = 0xFFFFFFFF; // 32bit tim max count
      usPeriod = 0xFFFF; // 16bit tim max count
    }
    else
    {
      usPeriod = 0xFFFF; // 16bit tim max count
    }

    TimHandle.Instance = TIMx;
    TimHandle.Init.Prescaler         = usPrescaler;
    TimHandle.Init.Period            = usPeriod;
    TimHandle.Init.ClockDivision     = 0;
    TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
//    TimHandle.Init.RepetitionCounter = 0;
    TimHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

    if (HAL_TIM_Base_Init(&TimHandle) != HAL_OK)
    {
      Error_Handler(__FILE__, __LINE__);
    }

    {
      HAL_NVIC_SetPriority(TIM_HARD_IRQn, 0, 2); // lower than Uart
      HAL_NVIC_EnableIRQ(TIM_HARD_IRQn);
    }

    HAL_TIM_Base_Start(&TimHandle);
}

/**
  * @brief  TIM2 - TIM5 timing once. then run callback func. can run 4 timers at once
  *         TIM2, TIM5 32bit. TIM3, TIM4 16bit.
  * @param  _CC: CC channel [1,2,3,4]
  *         _uiTimeOut: timeout in us, 16bit timer max 65.5ms, 32bit max 4294s
  *         _pCallBack: callback fun pointer
  * @retval None
  */
void bsp_StartHardTimer(uint8_t _CC, uint32_t _uiTimeOut, void * _pCallBack)
{
  uint32_t cnt_now;
  uint32_t cnt_tar;
  TIM_TypeDef* TIMx = TIM_HARD;

  if (_uiTimeOut < 5)
  {
      ;
  }
  else
  {
      _uiTimeOut -= 5;
  }

  cnt_now = TIMx->CNT;
  cnt_tar = cnt_now + _uiTimeOut;     /* 计算捕获的计数器值 */
  if (_CC == 1)
  {
      s_TIM_CallBack1 = (void (*)(void))_pCallBack;

      TIMx->CCR1 = cnt_tar;           /* 设置捕获比较计数器CC1 */
      TIMx->SR = (uint16_t)~TIM_IT_CC1;   /* 清除CC1中断标志 */
      TIMx->DIER |= TIM_IT_CC1;     /* 使能CC1中断 */
  }
  else if (_CC == 2)
  {
  s_TIM_CallBack2 = (void (*)(void))_pCallBack;

  TIMx->CCR2 = cnt_tar;       /* 设置捕获比较计数器CC2 */
      TIMx->SR = (uint16_t)~TIM_IT_CC2; /* 清除CC2中断标志 */
  TIMx->DIER |= TIM_IT_CC2;     /* 使能CC2中断 */
  }
  else if (_CC == 3)
  {
      s_TIM_CallBack3 = (void (*)(void))_pCallBack;

  TIMx->CCR3 = cnt_tar;       /* 设置捕获比较计数器CC3 */
      TIMx->SR = (uint16_t)~TIM_IT_CC3; /* 清除CC3中断标志 */
  TIMx->DIER |= TIM_IT_CC3;     /* 使能CC3中断 */
  }
  else if (_CC == 4)
  {
      s_TIM_CallBack4 = (void (*)(void))_pCallBack;

  TIMx->CCR4 = cnt_tar;       /* 设置捕获比较计数器CC4 */
      TIMx->SR = (uint16_t)~TIM_IT_CC4; /* 清除CC4中断标志 */
  TIMx->DIER |= TIM_IT_CC4;     /* 使能CC4中断 */
  }
  else
  {
      return;
  }
}

void TIM_HARD_IRQHandler(void)
{
  uint16_t itstatus = 0x0, itenable = 0x0;
  TIM_TypeDef* TIMx = TIM_HARD;


    itstatus = TIMx->SR & TIM_IT_CC1;
  itenable = TIMx->DIER & TIM_IT_CC1;

  if ((itstatus != (uint16_t)RESET) && (itenable != (uint16_t)RESET))
  {
    TIMx->SR = (uint16_t)~TIM_IT_CC1;
    TIMx->DIER &= (uint16_t)~TIM_IT_CC1;    /* 禁能CC1中断 */

        /* 先关闭中断，再执行回调函数。因为回调函数可能需要重启定时器 */
        s_TIM_CallBack1();
    }

  itstatus = TIMx->SR & TIM_IT_CC2;
  itenable = TIMx->DIER & TIM_IT_CC2;
  if ((itstatus != (uint16_t)RESET) && (itenable != (uint16_t)RESET))
  {
    TIMx->SR = (uint16_t)~TIM_IT_CC2;
    TIMx->DIER &= (uint16_t)~TIM_IT_CC2;    /* 禁能CC2中断 */

        /* 先关闭中断，再执行回调函数。因为回调函数可能需要重启定时器 */
        s_TIM_CallBack2();
    }

  itstatus = TIMx->SR & TIM_IT_CC3;
  itenable = TIMx->DIER & TIM_IT_CC3;
  if ((itstatus != (uint16_t)RESET) && (itenable != (uint16_t)RESET))
  {
    TIMx->SR = (uint16_t)~TIM_IT_CC3;
    TIMx->DIER &= (uint16_t)~TIM_IT_CC3;    /* 禁能CC2中断 */

        /* 先关闭中断，再执行回调函数。因为回调函数可能需要重启定时器 */
        s_TIM_CallBack3();
    }

  itstatus = TIMx->SR & TIM_IT_CC4;
  itenable = TIMx->DIER & TIM_IT_CC4;
  if ((itstatus != (uint16_t)RESET) && (itenable != (uint16_t)RESET))
  {
    TIMx->SR = (uint16_t)~TIM_IT_CC4;
    TIMx->DIER &= (uint16_t)~TIM_IT_CC4;    /* 禁能CC4中断 */

        /* 先关闭中断，再执行回调函数。因为回调函数可能需要重启定时器 */
        s_TIM_CallBack4();
    }
}
#endif // #ifdef TIM_HARD
