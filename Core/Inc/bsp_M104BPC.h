/*
 * bsp_M104BPC.h
 *
 *  Created on: Jun 30, 2019
 *      Author: andy
 */

#ifndef INC_BSP_M104BPC_H_
#define INC_BSP_M104BPC_H_

#define     COMM_MODULE_LOGIN_ON    0X00    //登记

#define   COMM_SET_MCU_IDLE     0X04  //设置模块空闲睡眠模式
#define   COMM_CONTROL_ANTENNA    0X05  //模块天线控制、自动寻卡设置

#define   COMM_SET_COM_BPS      0X15  //串口波特率设置

#define   COMM_MIFARE_SEARCH_CARD   0X20  //TYPE-A寻卡
#define   COMM_READ_BLOCK       0X21  //读块
#define   COMM_READ_SECTOR      0X22  //读扇区
#define   COMM_WRITE_BLOCK      0X23  //写块
#define   COMM_INIT_PURSE       0X24  //初始化钱包
#define   COMM_READ_PURSE       0X25  //读钱包
#define   COMM_INC_VALUE        0X26  //充值
#define   COMM_DEC_VALUE        0X27  //扣款
#define   COMM_BAK_PURSE        0X28  //备份钱包值
#define   COMM_CARD_HALT        0X29  //卡休眠
#define   COMM_LOAD_KEY_AB      0X2A  //下载密钥

#define   COMM_SET_TYPE_AB_OTHER    0X3A  //设定读卡模式TYPE A/B


#define   COMM_MIFARE1_PCD_REQUEST  0X46  //mifare one 寻卡
#define   COMM_MIFARE1_PCD_ANTICOLL 0X47  //mifare one 防冲撞
#define   COMM_MIFARE1_PCD_SELECT   0X48  //mifare one 选卡
#define   COMM_AUTHENTICATION2    0X4A  //认证2
#define   COMM_PCD_READ_BLOCK     0X4B  //无密码读块
#define   COMM_PCD_WRITE_BLOCK    0X4C  //无密码写块
 /*******************************************************************************/
#define SUCCESS 1
#define FAILURE 0
/*******************************************************************************/


extern uint8_t g_cComReceiveBuffer[43];

uint8_t ComSearchCard_Function(void); //寻卡函数
uint8_t ComAnticollCard_Function(void); //防卡冲突
uint8_t ComSelectCard_Function(void); //选卡
uint8_t ComCheckCard_Function(void);//认证
uint8_t ComReadBlock5_Function(void); //读块5
uint8_t ComWriteBlock5_Function(void);//写块5
void bsp_m104bpc_uart_callback(uint8_t i);

#endif /* INC_BSP_M104BPC_H_ */
