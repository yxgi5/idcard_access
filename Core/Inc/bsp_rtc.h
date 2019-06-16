/*
 * bsp_rtc.h
 *
 *  Created on: Jun 16, 2019
 *      Author: andy
 */

#ifndef BSP_RTC_H_
#define BSP_RTC_H_

#define RTC_CLOCK_SOURCE_LSI
//#define RTC_CLOCK_SOURCE_LSE

#ifdef RTC_CLOCK_SOURCE_LSI
  #define RTC_ASYNCH_PREDIV    0x7C
  #define RTC_SYNCH_PREDIV     0x0127
#endif

#ifdef RTC_CLOCK_SOURCE_LSE
  #define RTC_ASYNCH_PREDIV  0x7F
  #define RTC_SYNCH_PREDIV   0x00FF
#endif

/* RTC handler declaration */
extern RTC_HandleTypeDef RtcHandle;

void bsp_rtc_init(void);
void RTC_TimeStampConfig(void);
void RTC_CalendarShow(void);


#endif /* BSP_RTC_H_ */
