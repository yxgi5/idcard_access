/*
 * bsp_i2c_oled_ssd1306.h
 *
 *  Created on: 16-June-2019
 *      Author: andy
 */

#ifndef BSP_I2C_OLED_SSD1306_H_
#define BSP_I2C_OLED_SSD1306_H_

#define OLED_ADDRESS	0x78 //通过调整0R电阻,屏可以0x78和0x7A两个地址 -- 默认0x78

#define Brightness  0xCF
#define X_WIDTH     128
#define Y_WIDTH     64

//void I2C_Configuration(void);
//void I2C_WriteByte(uint8_t addr,uint8_t data);
void WriteCmd(unsigned char I2C_Command);
void WriteDat(unsigned char I2C_Data);
void OLED_Init(void);
void OLED_SetPos(unsigned char x, unsigned char y);
void OLED_Fill(unsigned char fill_Data);
void OLED_CLS(void);
void OLED_ON(void);
void OLED_OFF(void);
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize);
void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N);
void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]);

#endif /* BSP_I2C_OLED_SSD1306_H_ */
