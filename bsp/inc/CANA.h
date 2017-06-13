/*
 * CANA.h
 *
 *  Created on: 2017��4��16��
 *      Author: admin
 */

#ifndef BSP_INC_CANA_H_
#define BSP_INC_CANA_H_
#include "DSP2833x_Device.h"
#include "DSP2833x_Examples.h"
#include "data.h"
#include "delay.h"
#include "stdint.h"
#define USE_CANA  1
#define USE_CANB  0
#define CAN_Id_Standard   0//��ʾ��׼֡
#define CAN_Id_Extended   1//��ʾ��չ֡
#define CAN_ID_STD      CAN_Id_Standard
#define CAN_ID_EXT      CAN_Id_Extended
#define TXCOUNT 100
typedef enum
{
	Null = 0, CANA = 1, CANB = 2,
} CAN_Num;
typedef enum
{
	NON_CHANGE = 0, UPDATE = 1,
} UPDATE_State;
typedef struct
{
	unsigned short int SA :8;
	unsigned short int PS :8;
	unsigned short int PF :8;
	unsigned short int DP :1;
	unsigned short int R :1;
	unsigned short int Priority :3;
	unsigned short int resved :3;

} SAE__ID;
typedef union
{
	unsigned long int id;
	SAE__ID SAE_id;
} SAE_ID;
typedef struct
{
	unsigned char data[8];
} CAN_MSG_byte;
typedef struct
{
	unsigned char byte0;
	unsigned char byte1;
	unsigned char byte2;
	unsigned char byte3;
	unsigned char byte4;
	unsigned char byte5;
	unsigned char byte6;
	unsigned char byte7;
} CAN_MSG_BYTE;
typedef struct
{
	unsigned long int MSG_MD_L;
	unsigned long int MSG_MD_H;
} CAN_MSG_DWORD;
typedef union
{
	CAN_MSG_byte msg_byte;
	CAN_MSG_BYTE msg_Byte;
} CAN_MSG_DATA;
typedef struct
{
	union
	{
		unsigned short int all;
		struct
		{
			unsigned short int StdId :11;
			unsigned short int resved :5;
		} bit;
	} StdId; //��׼֡ID
	union
	{
		unsigned long int all;
		struct
		{
			unsigned long int ExtId :29;
			unsigned long int resved :3;
		} bit;
	} ExtId; //��չ֡ID
	unsigned char SAE_J1939_Flag; //��ʾ�Ƿ�ʹ��SAE J1939Э��
	SAE_ID SAE_J1939_ID;
	unsigned char IDE;   //֡����,��Ϊ��CAN_ID_STD(��׼֡),CAN_ID_EXT(��չ֡)
	unsigned char DLC;  //���ݳ��ȣ���Ϊ0��8;
	unsigned char MBox_num;  //������,0-31
	unsigned short int Tx_timeout_cnt;
	CAN_Num CAN_num;
	UPDATE_State tx_update;
	CAN_MSG_DATA CAN_Tx_msg_data; /*!< ֡��Ϣ����,��8�ֽ� */

} CanTxMsg;
typedef struct
{
	unsigned short int StdId;  //��׼֡ID,ֵΪ0x000��0x7FFF;
	unsigned long int ExtId; //��չ֡ID,ֵΪ0��0x1FFFFFFF
	unsigned char SAE_J1939_Flag; //��ʾ�Ƿ�ʹ��SAE J1939Э��
	SAE_ID SAE_J1939_ID;
	unsigned char IDE;   //֡����,��Ϊ��CAN_ID_STD(��׼֡),CAN_ID_EXT(��չ֡)
	unsigned char DLC;  //���ݳ��ȣ���Ϊ0��8;
	unsigned char MBox_num;  //��������������
	unsigned short int Rx_timeout_cnt;
	CAN_MSG_DATA CAN_Rx_msg_data; /*!< ֡��Ϣ����,��8�ֽ� */
	CAN_Num CAN_num;
	UPDATE_State rx_update;
} CanRxMsg;


void CAN_GPIO_Config(CAN_Num CAN);
void CAN_Config(CAN_Num CAN);
void CAN_Tx_Msg(CanTxMsg *can_tx_msg);  //������Ϣ
void CAN_Rx_Msg(CanRxMsg *can_rx_msg);  //������Ϣ
#if USE_CANA
static void CANA_RX_Config(void);
#endif
#if USE_CANB
static void CANB_RX_Config(void);
#endif
void CAN_Rx_Config(void);
void CAN_Rx_IT_Concig(void);
__interrupt void Ecana_isr1(void);
extern CanTxMsg can_tx_msg;
extern CanRxMsg can_rx_msg;
#endif /* BSP_INC_CANA_H_ */
