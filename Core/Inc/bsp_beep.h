/**
  ******************************************************************************
  * @file    bsp_beep.h
  * @author  Andreas Zhang
  * @version V1.0
  * @date    05-June-2019
  * @brief   BSP beep driver header.
  ******************************************************************************
*/

#ifndef BSP_BEEP_H_
#define BSP_BEEP_H_

typedef struct _BEEP_T
{
  uint8_t ucEnalbe;
  uint8_t ucState;
  uint16_t usBeepTime;
  uint16_t usStopTime;
  uint16_t usCycle;
  uint16_t usCount;
  uint16_t usCycleCount;
  uint8_t ucMute;   /* 1表示静音 */
}BEEP_T;

/* 供外部调用的函数声明 */
void BEEP_InitHard(void);
void BEEP_Start(uint16_t _usBeepTime, uint16_t _usStopTime, uint16_t _usCycle);
void BEEP_Stop(void);
void BEEP_KeyTone(void);
void BEEP_Pro(void);

void BEEP_Pause(void);
void BEEP_Resume(void);

#endif /* BSP_BEEP_H_ */
