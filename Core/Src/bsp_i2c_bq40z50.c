/*
 * bsp_i2c_bq40z50.c
 *
 *  Created on: 2019年6月20日
 *      Author: andy
 */

#include "bsp.h"


uint16_t bq40z50_read_reg(uint8_t reg)
{
	uint8_t bqRxBuffer[2];
	uint16_t ret;

	i2c2_SendByte(&reg, ADDR_BQ40Z50R1);
	i2c2_ReadBytes(bqRxBuffer, 2, ADDR_BQ40Z50R1);

	ret  = (uint16_t)(bqRxBuffer[1] << 8);
	ret |= bqRxBuffer[0];

	return ret;
}
