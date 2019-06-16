/*
 * bsp_spi.h
 *
 *  Created on: Jun 16, 2019
 *      Author: andy
 */

#ifndef BSP_SPI_H_
#define BSP_SPI_H_

void bsp_InitSPIBus(void);

void bsp_spiTransfer(void);
void bsp_InitSPIParam(uint32_t _BaudRatePrescaler, uint32_t _CLKPhase, uint32_t _CLKPolarity);

void bsp_SpiBusEnter(void);
void bsp_SpiBusExit(void);
uint8_t bsp_SpiBusBusy(void);

//#define SPI_BUFFER_SIZE   (4 * 1024)
#define SPI_BUFFER_SIZE   (128)

extern uint8_t g_spiTxBuf[SPI_BUFFER_SIZE];
extern uint8_t g_spiRxBuf[SPI_BUFFER_SIZE];
extern uint32_t g_spiLen;

extern uint8_t g_spi_busy;


#endif /* BSP_SPI_H_ */
