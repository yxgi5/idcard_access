/*
 * bsp_i2c.h
 *
 *  Created on: Jun 15, 2019
 *      Author: andy
 */

#ifndef BSP_I2C_H_
#define BSP_I2C_H_

#define I2C_WR  0   /* 写控制bit */
#define I2C_RD  1   /* 读控制bit */

#define I2C1_EN 1
#define I2C2_EN 1

#define USE_GPIO_I2C 0

#if USE_GPIO_I2C == 1

void bsp_InitI2C(void);
void i2c_Start(void);
void i2c_Stop(void);
void i2c_SendByte(uint8_t _ucByte);
uint8_t i2c_ReadByte(void);
uint8_t i2c_WaitAck(void);
void i2c_Ack(void);
void i2c_NAck(void);
uint8_t i2c_CheckDevice(uint8_t _Address);

#else
/*
 * PB6: I2C1_SCL , PB7: I2C1_SDA
 * PB10: I2C2_SCL , PB11: I2C2_SDA
 */
//#define I2Cx                            I2C1
//#define RCC_PERIPHCLK_I2Cx              RCC_PERIPHCLK_I2C1
//#define RCC_I2CxCLKSOURCE_SYSCLK        RCC_I2C1CLKSOURCE_SYSCLK
//#define I2Cx_CLK_ENABLE()               __HAL_RCC_I2C1_CLK_ENABLE()
//#define I2Cx_SDA_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
//#define I2Cx_SCL_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define I2C1_CLK_ENABLE()               __HAL_RCC_I2C1_CLK_ENABLE()
#define I2C1_SDA_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define I2C1_SCL_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define I2C1_GPIO_CLK_ENABLE()  { \
    __HAL_RCC_GPIOB_CLK_ENABLE(); \
};
//#define I2Cx_FORCE_RESET()              __HAL_RCC_I2C1_FORCE_RESET()
//#define I2Cx_RELEASE_RESET()            __HAL_RCC_I2C1_RELEASE_RESET()

/* Definition for I2C1 Pins */
#define I2C1_SCL_PIN                    GPIO_PIN_6
#define I2C1_SCL_GPIO_PORT              GPIOB
#define I2C1_SDA_PIN                    GPIO_PIN_7
#define I2C1_SDA_GPIO_PORT              GPIOB
#define I2C1_SCL_SDA_AF                 GPIO_AF1_I2C1

///* Definition for I2Cx's NVIC */
//#define I2Cx_IRQn                       I2C1_IRQn
//#define I2Cx_IRQHandler                 I2C1_IRQHandler

#define I2C2_CLK_ENABLE()               __HAL_RCC_I2C2_CLK_ENABLE()
#define I2C2_SDA_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define I2C2_SCL_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define I2C2_GPIO_CLK_ENABLE()  { \
    __HAL_RCC_GPIOB_CLK_ENABLE(); \
};

#define I2C2_SCL_PIN                    GPIO_PIN_10
#define I2C2_SCL_GPIO_PORT              GPIOB
#define I2C2_SDA_PIN                    GPIO_PIN_11
#define I2C2_SDA_GPIO_PORT              GPIOB
#define I2C2_SCL_SDA_AF                 GPIO_AF6_I2C2

/* Size of Transmission buffer */
#define TXBUFFERSIZE                      (COUNTOF(aTxBuffer) - 1)
/* Size of Reception buffer */
#define RXBUFFERSIZE                      TXBUFFERSIZE

#define COUNTOF(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))

extern I2C_HandleTypeDef I2c1Handle;
extern I2C_HandleTypeDef I2c2Handle;

#if I2C1_EN == 1
void bsp_InitI2C1(void);
#endif

#if I2C2_EN == 1
void bsp_InitI2C2(void);
#endif
uint8_t i2c_SendByte(I2C_HandleTypeDef *hi2c, uint8_t *_ucBuffer, uint16_t SlaveAddr);
uint8_t i2c_ReadByte(I2C_HandleTypeDef *hi2c, uint8_t *_ucBuffer, uint16_t SlaveAddr);
uint8_t i2c_SendBytes(I2C_HandleTypeDef *hi2c, uint8_t *_ucBuffer, uint16_t ByteCount, uint16_t SlaveAddr);
uint8_t i2c_ReadBytes(I2C_HandleTypeDef *hi2c, uint8_t *_ucBuffer, uint16_t ByteCount, uint16_t SlaveAddr);
uint8_t i2c_CheckDevice(I2C_HandleTypeDef *hi2c, uint8_t _Address);

#endif

#endif /* BSP_I2C_H_ */
