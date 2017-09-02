/*
 * BootLoader.h
 *
 *  Created on: 2017��4��19��
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
#define APP_START_ADDR  ((uint32_t)0x310010)
#define APP_INFO_ADDR   ((uint32_t)0x310000)
#define CAN_BL_APP      0xAAAAAA
#define CAN_BL_BOOT     0x555555
#define DEVICE_ADDR     0x134//�豸��ַ
#define CMD_WIDTH       0x04
#define ADDR_WIDTH      0x0C
//----------------------���º궨���Ƕ�оƬ�ͺŽ��к궨��----------------------------
#define TMS320F28335    0x01
#define TMS230F2808     0x02
#define STM32F407IGT6   0x03
//---------------------------------------------------------------------------------
//������Ϣ�б�
#define DEVICE_ADDR_ERROR  0xA0
#define ERASE_ERROR        0xA1
#define WRITE_ERROR        0xA2
#define READ_LEN_ERROR     0xA3
//---------------------------------------------------
typedef struct _Device_INFO
{
	union
	{
		unsigned short int all;
		struct
		{
			unsigned short int Device_addr:	12;
			unsigned short int reserve:	4;
		}bits;//�豸��ַ
	}Device_addr;
	union
	{
		unsigned long int all;
		struct
		{
			unsigned long int FW_TYPE:24;//�̼�����
			unsigned long int Chip_Value:8;//������оƬ����
		}bits;
	}FW_TYPE;
	unsigned long int FW_Version;//�̼��汾
}Device_INFO;
typedef struct _bootloader_data
{
	union
	{
		u32 all;
		struct
		{
			u16 cmd :CMD_WIDTH; //����
			u16 addr :ADDR_WIDTH; //�豸��ַ
			u16 reserve :16; //����λ
		} bit;
	} ExtId; //��չ֡ID
	unsigned char IDE;   //֡����,��Ϊ��CAN_ID_STD(��׼֡),CAN_ID_EXT(��չ֡)
	unsigned char DLC;  //���ݳ��ȣ���Ϊ0��8;
	u8 data[8];
} bootloader_data;
typedef struct _Boot_CMD_LIST
{
	//Bootloader�������
	unsigned char Read;         //��ȡflash����
	unsigned char Erase;        //����APP������������
	unsigned char Write;        //�Զ��ֽ���ʽд����
	unsigned char Check;        //���ڵ��Ƿ����ߣ�ͬʱ���ع̼���Ϣ
	unsigned char Excute;       //ִ�й̼�
	unsigned char WriteInfo;    //���ö��ֽ�д������ز�����д��ʼ��ַ����������
	unsigned char SetBaudRate;  //���ýڵ㲨����
	//�ڵ㷵��״̬,�ؼ�
	unsigned char CmdFaild;     //����ִ��ʧ��
	unsigned char CmdSuccess;   //����ִ�гɹ�


} Boot_CMD_LIST;
extern Boot_CMD_LIST cmd_list;
extern bootloader_data Bootloader_data;
extern Device_INFO DEVICE_INFO;
void __disable_irq(void);
void __enable_irq(void);
void __set_PRIMASK(u8 state);

unsigned short int CRCcalc16 (unsigned char *data,unsigned short int len);
void CAN_BOOT_JumpToApplication(uint32_t Addr);
void CAN_BOOT_ExecutiveCommand(CanRxMsg *pRxMessage);
unsigned short int Check_APP(uint32_t Addr);
#endif /* BOOTLOADER_BOOTLOADER_H_ */
