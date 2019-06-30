/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "bsp.h"
extern unsigned char BMP1[];
#define  Test_Number_L_Addr 0x0000
#define  Test_Number_H_Addr 0x0010

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;

SPI_HandleTypeDef hspi1;

uint8_t Vol_Precent;
uint8_t Test_Number_L,Test_Number_H;
uint32_t Test_Nmuber;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
//void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  bsp_Init();
  uint8_t i=0;

  /* Initialize all configured peripherals */
//  MX_GPIO_Init();
//  MX_I2C1_Init();
//  MX_I2C2_Init();
//  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  OLED_Init();
  BEEP_Start(200, 200, 1);
  OLED_ShowCN(10,3,23);
  OLED_ShowCN(42,3,24);
  OLED_ShowCN(74,3,25);
  OLED_ShowCN(106,3,26);
  HAL_Delay(10000);
  OLED_CLS();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    Vol_Precent = (uint8_t)bq40z50_read_reg(CMD_RSOC);

    if(Vol_Precent>99)    //超99%，显示100%
    {
      bsp_LedOff(LED1);
      OLED_ShowCN(8,2,20);
      OLED_ShowCN(24,2,21);
      OLED_ShowCN(40,2,16);
      OLED_ShowCN(56,2,1);
      OLED_ShowCN(72,2,0);
      OLED_ShowCN(88,2,0);
      OLED_ShowCN(104,2,22);
    }
    else if(Vol_Precent<42)   //小于42%，提示充电
    {
        bsp_LedOn(LED1);
        OLED_CLS();
        OLED_ShowCN(26,2,11);
        OLED_ShowCN(58,2,12);
        OLED_ShowCN(90,2,20);
        BEEP_Start(200, 200, 5);
    }
    else
    {
      bsp_LedOff(LED1);
      OLED_ShowCN(8,2,20);
      OLED_ShowCN(24,2,21);
      OLED_ShowCN(40,2,16);
      OLED_ShowCN(56,2,10);
      OLED_ShowCN(72,2,Vol_Precent/10);
      OLED_ShowCN(88,2,Vol_Precent%10);
      OLED_ShowCN(104,2,22);
    }

    if(ComSearchCard_Function())   //寻卡
    {
      if(ComAnticollCard_Function()) //防冲突
      {
        if(ComSelectCard_Function()) //选卡
        {
          if(ComCheckCard_Function()) //密钥认证
          {
            if(ComReadBlock5_Function())//读块
            {
              if((g_cComReceiveBuffer[5]==0xff)&&(g_cComReceiveBuffer[6]==0xff))
              {
                uint8_t tmp=0;
                ee_WriteBytes(&I2c1Handle, &tmp, Test_Number_L_Addr, 1);
                ee_WriteBytes(&I2c1Handle, &tmp, Test_Number_H_Addr, 1);
              }
              Test_Number_H= Test_Number_H + g_cComReceiveBuffer[5];
              Test_Number_L= Test_Number_L + g_cComReceiveBuffer[6];
              Test_Nmuber=Test_Number_H*256+Test_Number_L;
              if(Test_Nmuber<=9999)            //若EEPROM中值加卡上值小于9999，卡清零，卡上值累加写入eeprom中
              {
                ee_WriteBytes(&I2c1Handle, &Test_Number_L, Test_Number_L_Addr, 1);
                ee_WriteBytes(&I2c1Handle, &Test_Number_H, Test_Number_H_Addr, 1);
                while(!ComWriteBlock5_Function());
              }
              else
              {
                break;
              }
            }
            else
            {
              break;
            }
          }
          else
          {
            break;
          }
        }
        else
        {
            break;
        }
      }
      else
      {
        break;
      }
    }
    ee_ReadBytes(&I2c1Handle, &Test_Number_L, Test_Number_L_Addr, 1);
    ee_ReadBytes(&I2c1Handle, &Test_Number_H, Test_Number_H_Addr, 1);
    Test_Nmuber= Test_Number_H*256+Test_Number_L;

    if(Test_Nmuber==0x0000)        //若EEPROM中值为0，提示充值
    {
      OLED_CLS();
      OLED_ShowCN(26,5,11);
      OLED_ShowCN(58,5,12);
      OLED_ShowCN(90,5,13);
      BEEP_Start(200, 200, 3);
      {
        uint8_t tmp=0;
        ee_WriteBytes(&I2c1Handle, &tmp, Test_Number_L_Addr, 1);
        ee_WriteBytes(&I2c1Handle, &tmp, Test_Number_H_Addr, 1);
      }
      while(!ComSearchCard_Function());
      while(!ComAnticollCard_Function());
      while(!ComSelectCard_Function());
      while(!ComCheckCard_Function());
      while(!ComReadBlock5_Function());
      ee_ReadBytes(&I2c1Handle, &g_cComReceiveBuffer[6], Test_Number_L_Addr, 1);
      ee_ReadBytes(&I2c1Handle, &g_cComReceiveBuffer[5], Test_Number_H_Addr, 1);
      while(!ComWriteBlock5_Function());
    }
    OLED_ShowCN(8,5,14);
    OLED_ShowCN(24,5,15);
    OLED_ShowCN(40,5,16);
    OLED_ShowCN(52,5,Test_Nmuber/1000);
    OLED_ShowCN(68,5,(Test_Nmuber%1000)/100);
    OLED_ShowCN(84,5,((Test_Nmuber%1000)%100)/10);
    OLED_ShowCN(100,5,((Test_Nmuber%1000)%100)%10);
    if((bsp_GetKey()==0x10)&&(Test_Nmuber>0))
    {
      bsp_Motor_Run(36000);
      Test_Nmuber=Test_Nmuber-1;
      Test_Number_H= Test_Nmuber/256;
      Test_Number_L= Test_Nmuber%256;
      ee_ReadBytes(&I2c1Handle, &Test_Number_L, Test_Number_L_Addr, 1);
      ee_ReadBytes(&I2c1Handle, &Test_Number_H, Test_Number_H_Addr, 1);
    }

  }
  /* USER CODE END 3 */
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00707CBB;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }
  /** Configure Analogue filter 
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }
  /** Configure Digital filter 
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x00707CBB;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }
  /** Configure Analogue filter 
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }
  /** Configure Digital filter 
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
