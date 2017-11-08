/*
 * BootLoader.h
 *
 *  Created on: 2017年4月19日
 *      Author: admin
 */

#ifndef BOOTLOADER_BOOTLOADER_H_
#define BOOTLOADER_BOOTLOADER_H_
#include "DSP2833x_Device.h"
#include "DSP2833x_Examples.h"
#include "data.h"
#include "delay.h"
#include "stdint.h"
#include "CANA.h"
#include "Flash.h"
#define DATA_LEN  520   //缓存数组长度
#define READ_MAX  256   //每次读取数据的最大长度,16位数据
#define APP_START_ADDR       ((uint32_t)0x310010)
#define APP_Write_START_ADDR ((uint32_t)0x310000)
#define APP_Write_END_ADDR   ((uint32_t)0x33FF80)
#define APP_INFO_ADDR   ((uint32_t)0x310000)
#define CAN_BL_APP      0xAAAAAA
#define CAN_BL_BOOT     0x555555
#define DEVICE_ADDR     0x134//设备地址
#define CMD_WIDTH       0x04
#define ADDR_WIDTH      0x0C
//----------------------以下宏定义是对芯片型号进行宏定义----------------------------
#define TMS320F28335    0x01
#define TMS230F2808     0x02
#define STM32F407IGT6   0x03
//---------------------------------------------------------------------------------
//故障信息列表
#define DEVICE_ADDR_ERROR  0xA0
#define ERASE_ERROR        0xA1
#define WRITE_ERROR        0xA2
#define READ_LEN_ERROR     0xA3
#define MSG_DATA_LEN_ERROR 0xA4
#define FILE_TYPE_ERROR    0xA5
#define CRC_ERROR          0xA6
#define FLASH_ADDR_ERROR   0xA7
#define WRITE_LEN_ERROR    0xA8
//---------------------------------------------------
#define File_None 0xF0
#define File_bin  0xF1
#define File_hex  0xF2
typedef void (*pFunction)(void);
typedef struct _Device_INFO
{
	union
	{
		unsigned short int all;
		struct
		{
			unsigned short int Device_addr:	12;
			unsigned short int reserve:	4;
		}bits;//设备地址
	}Device_addr;
	union
	{
		unsigned long int all;
		struct
		{
			unsigned long int FW_TYPE:24;//固件类型
			unsigned long int Chip_Value:8;//控制器芯片类型
		}bits;
	}FW_TYPE;
	unsigned long int FW_Version;//固件版本
}Device_INFO;
typedef struct _bootloader_data
{
	union
	{
		u32 all;
		struct
		{
			u16 cmd :CMD_WIDTH; //命令
			u16 addr :ADDR_WIDTH; //设备地址
			u16 reserve :16; //保留位
		} bit;
	} ExtId; //扩展帧ID
	unsigned char IDE;   //帧类型,可为：CAN_ID_STD(标准帧),CAN_ID_EXT(扩展帧)
	unsigned char DLC;  //数据长度，可为0到8;
	u8 data[8];
} bootloader_data;
typedef struct _Boot_CMD_LIST
{
	//Bootloader相关命令
	unsigned char Read;         //读取flash数据
	unsigned char Erase;        //擦出APP储存扇区数据
	unsigned char Write;        //以多字节形式写数据
	unsigned char Check;        //检测节点是否在线，同时返回固件信息
	unsigned char Excute;       //执行固件
	unsigned char WriteInfo;    //设置多字节写数据相关参数（写起始地址，数据量）
	unsigned char SetBaudRate;  //设置节点波特率
	//节点返回状态,关键
	unsigned char CmdFaild;     //命令执行失败
	unsigned char CmdSuccess;   //命令执行成功


} Boot_CMD_LIST;
extern Boot_CMD_LIST cmd_list;
extern bootloader_data Bootloader_data;
extern Device_INFO DEVICE_INFO;
extern Uint16 app_check[3] ;
void __disable_irq(void);
void __enable_irq(void);
void __set_PRIMASK(u8 state);

unsigned short int CRCcalc16 (unsigned char *data,unsigned short int len);
void CAN_BOOT_JumpToApplication(uint32_t Addr);
void CAN_BOOT_ExecutiveCommand(CanRxMsg *pRxMessage);
unsigned short int Check_APP(uint32_t Addr);
#endif /* BOOTLOADER_BOOTLOADER_H_ */
