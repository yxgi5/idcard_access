/**
  ******************************************************************************
  * @file    bsp_i2c_eeprom_24xx.h
  * @author  Andreas Zhang
  * @version V1.0
  * @date    05-June-2019
  * @brief
  ******************************************************************************
*/

#ifndef BSP_I2C_EEPROM_24XX_H_
#define BSP_I2C_EEPROM_24XX_H_

//#define AT24C02
#define AT24C128

#ifdef AT24C02
  #define EE_MODEL_NAME   "AT24C02"
  #define EE_DEV_ADDR     0xA0    /* 设备地址 */
  #define EE_PAGE_SIZE    8     /* 页面大小(字节) */
  #define EE_SIZE       256     /* 总容量(字节) */
  #define EE_ADDR_BYTES   1     /* 地址字节个数 */
#endif

#ifdef AT24C128
  #define EE_MODEL_NAME   "AT24C128"
  #define EE_DEV_ADDR     0xA0    /* 设备地址 */
  #define EE_PAGE_SIZE    64      /* 页面大小(字节) */
  #define EE_SIZE       (16*1024) /* 总容量(字节) */
  #define EE_ADDR_BYTES   2     /* 地址字节个数 */
#endif

uint8_t ee_CheckOk(void);
uint8_t ee_ReadBytes(uint8_t *_pReadBuf, uint16_t _usAddress, uint16_t _usSize);
uint8_t ee_WriteBytes(uint8_t *_pWriteBuf, uint16_t _usAddress, uint16_t _usSize);

#endif /* BSP_I2C_EEPROM_24XX_H_ */

