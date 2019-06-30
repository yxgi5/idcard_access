/**
  ******************************************************************************
  * @file    bsp_led.h
  * @author  Andreas Zhang
  * @version V1.0
  * @date    05-June-2019
  * @brief   BSP led driver header.
  ******************************************************************************
*/

#ifndef INC_BSP_LED_H_
#define INC_BSP_LED_H_

typedef enum
{
  LED1 = 0,
  LED2 = 1,
  LED3 = 2,
  LED4 = 3
} Led_TypeDef;

/*
    4 user LEDs：(low on，high off)
    LD1     : PB12
    LD2     : PB13
    LD3     : PB14
    LD4     : PB15
*/

#define LEDn  4

#define GPIO_PORT_LED1  GPIOB
#define GPIO_PIN_LED1 GPIO_PIN_12

#define GPIO_PORT_LED2  GPIOB
#define GPIO_PIN_LED2 GPIO_PIN_13

#define GPIO_PORT_LED3  GPIOB
#define GPIO_PIN_LED3 GPIO_PIN_14

#define GPIO_PORT_LED4  GPIOB
#define GPIO_PIN_LED4 GPIO_PIN_15

#define GPIO_CLK_ENABLE_LED1()           __HAL_RCC_GPIOB_CLK_ENABLE()
#define GPIO_CLK_DISABLE_LED1()          __HAL_RCC_GPIOB_CLK_DISABLE()

#define GPIO_CLK_ENABLE_LED2()           __HAL_RCC_GPIOB_CLK_ENABLE()
#define GPIO_CLK_DISABLE_LED2()          __HAL_RCC_GPIOB_CLK_DISABLE()

#define GPIO_CLK_ENABLE_LED3()           __HAL_RCC_GPIOB_CLK_ENABLE()
#define GPIO_CLK_DISABLE_LED3()          __HAL_RCC_GPIOB_CLK_DISABLE()

#define GPIO_CLK_ENABLE_LED4()           __HAL_RCC_GPIOB_CLK_ENABLE()
#define GPIO_CLK_DISABLE_LED4()          __HAL_RCC_GPIOB_CLK_DISABLE()

/*
#define ALL_LED_GPIO_CLK_ENABLE() { \
  __HAL_RCC_GPIOB_CLK_ENABLE(); \
  __HAL_RCC_GPIOF_CLK_ENABLE(); \
  __HAL_RCC_GPIOI_CLK_ENABLE(); \
};
*/

#define ALL_LED_GPIO_CLK_ENABLE() { \
  __HAL_RCC_GPIOB_CLK_ENABLE(); \
};

#define GPIO_CLK_ENABLE_LEDx(__INDEX__) do{if((__INDEX__) == 0) GPIO_CLK_ENABLE_LED1(); else \
                                         if((__INDEX__) == 1) GPIO_CLK_ENABLE_LED2(); else \
                                         if((__INDEX__) == 2) GPIO_CLK_ENABLE_LED3(); else \
                                         if((__INDEX__) == 3) GPIO_CLK_ENABLE_LED4(); \
                                         }while(0)

#define GPIO_CLK_DISABLE_LEDx(__INDEX__) do{if((__INDEX__) == 0) GPIO_CLK_DISABLE_LED1(); else \
                                          if((__INDEX__) == 1) GPIO_CLK_DISABLE_LED2(); else \
                                          if((__INDEX__) == 2) GPIO_CLK_DISABLE_LED3(); else \
                                          if((__INDEX__) == 3) GPIO_CLK_DISABLE_LED4(); \
                                          }while(0)

void bsp_InitLed(void);
void bsp_LedOn(Led_TypeDef _no);
void bsp_LedOff(Led_TypeDef _no);
void bsp_LedToggle(Led_TypeDef _no);
uint8_t bsp_IsLedOn(Led_TypeDef _no);

#endif /* INC_BSP_LED_H_ */
