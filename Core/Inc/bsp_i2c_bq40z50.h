/*
 * bsp_i2c_bq40z50.h
 *
 *  Created on: 2019年6月20日
 *      Author: andy
 */

//I2C2_TIMING 速率为10K-100K，不可过低或者过高

#ifndef BSP_I2C_BQ40Z50_H_
#define BSP_I2C_BQ40Z50_H_

#define CMD_TEMPERATURE      0x08
#define CMD_VOLTAGE          0x09
#define CMD_AVERAGE_CURRENT  0x0B
#define CMD_RSOC             0x0D    //RelativeStateOfCharge
#define REMAINING_CAPACITY   0x0F
#define FULL_CHARGE_CAPACITY 0x10
#define STATE_OF_HEALTH      0x4F

#define ADDR_BQ40Z50R1       0x16  // default address

uint16_t bq40z50_read_reg(uint8_t reg);

#endif /* BSP_I2C_BQ40Z50_H_ */
