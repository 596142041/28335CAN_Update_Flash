/*
 * BootLoader.c
 *
 *  Created on: 2017��4��19��
 *      Author: admin
 */
#include "BootLoader.h"
typedef  void (*pFunction)(void);
bootloader_data Boot_ID_info;
u8	   data_temp[128];
Uint16 write_temp[128];
u8 can_cmd    = 0x00;//ID��bit0~bit3λΪ������
u16 can_addr  = 0x00;//ID��bit4~bit15λΪ�ڵ��ַ
u32 start_addr = 0x0000;//ÿһ�����ݵ���ʼ��ַ
u32 data_size=0;//���ݰ��Ĵ�С
u32 data_index=0;//����ָ��
u16 crc_data;
uint32_t exe_type = 0x00;
Boot_CMD_LIST cmd_list =
{
	.Erase = 0x0F,      //����APP��������
	.WriteInfo = 0x01,//���ö��ֽ�д������ز���(д��ʼ��ַ,������)
	.Write = 0x02,//�Զ��ֽ���ʽд����
	.Check = 0x03,//���ڵ��Ƿ����ߣ�ͬʱ���ع̼���Ϣ
	.SetBaudRate = 0x04,//���ýڵ㲨����
	.Excute = 0x05,//ִ�й̼�
	.CmdSuccess = 0x08,//����ִ�гɹ�
	.CmdFaild = 0x09,//����ִ��ʧ��
};
Device_INFO DEVICE_INFO =
{
.FW_TYPE = CAN_BL_BOOT,
.FW_Version = 0x0010001,
};
void __disable_irq(void)
{
	DINT;
	DRTM;
}
void __enable_irq(void)
{
	EINT;
	ERTM;
}
void __set_PRIMASK(u8 state)
{
	if(state == 1)
	{
		__disable_irq();
	}
	else if(state == 0)
	{
		__enable_irq();
	}
	else
	{
		return;
	}
}
unsigned short int CRCcalc16 (unsigned char *data,unsigned short int len)
{
	int i;
	unsigned short int crc_res =  0xFFFF;
	while(len--)
	{
		crc_res^=*data++;
		for(i = 0;i < 8;i++)
		{
			if(crc_res&0x01)
			{
				crc_res = (crc_res>>1)^0xa001;
			}
			else
			{
				crc_res = (crc_res>>1);
			}
		}
	}
	return crc_res;
}
void CAN_BOOT_JumpToApplication(uint32_t Addr)
{
	//  asm(" LB  0x310000");
   // (*((void(*)(void))(Addr)))();
	//(*(pFunction)(Addr))();
	pFunction jump;
	jump = (pFunction)(Addr);
	jump();

}
#pragma CODE_SECTION(CAN_BOOT_ExecutiveCommand,"ramfuncs");
void CAN_BOOT_ExecutiveCommand(CanRxMsg *pRxMessage)
{
	u8 i;
	CanTxMsg TxMessage;//���Ͷ�Ӧ��Ϣ
	Uint16 ret = 0x01;
	FLASH_ST Flash_Status;
	for(i = 0;i <8;i++)
	{
		TxMessage.CAN_Tx_msg_data.msg_byte.data[i] = 0x00;
	}
	TxMessage.CAN_num = CANA;
	TxMessage.DLC = 1;
	TxMessage.ExtId.bit.resved = 0x00;
	TxMessage.IDE = CAN_ID_EXT;
	TxMessage.MBox_num = 0x02;
	TxMessage.Tx_timeout_cnt = 0x00;
	TxMessage.SAE_J1939_Flag = 0;
	//��ȡ��ַ��Ϣ
	Boot_ID_info.ExtId.all = pRxMessage->ExtId;
	can_cmd = Boot_ID_info.ExtId.bit.cmd;
	can_addr = Boot_ID_info.ExtId.bit.addr;
	if((can_addr!=DEVICE_ADDR)&&(can_addr!=0))
	{
		TxMessage.ExtId.bit.ExtId = (0x0134<<CMD_WIDTH)|cmd_list.CmdFaild;
		TxMessage.DLC = 1;
		//TxMessage.Data[0] = 0x01;
		TxMessage.CAN_Tx_msg_data.msg_byte.data[0] = 0x01;
		CAN_Tx_Msg(&TxMessage);
		return;
	}
	//CMD_List.Erase������Flash�е����ݣ���Ҫ������Flash��С�洢��Data[0]��Data[3]��
	//�����������Bootloader������ʵ�֣���APP�����п��Բ���ʵ��
	//��Ҫ�ǲ���APP��������
		if(can_cmd == cmd_list.Erase)
		{
			__set_PRIMASK(1); //�ر�ȫ���ж�
			/*----------------------------------------------------------------------
			//������������
			FLASH_Unlock();
			FlashSize = (pRxMessage->Data[0]<<24)|(pRxMessage->Data[1]<<16)|(pRxMessage->Data[2]<<8)|(pRxMessage->Data[3]<<0);
			ret =  CAN_BOOT_ErasePage(APP_EXE_FLAG_START_ADDR,APP_START_ADDR+FlashSize);
			FLASH_Lock();
			---------------------------------------------------------------------------------*/
			ret = Flash_Erase(SECTORB|SECTORC|SECTORD|SECTORE|SECTORF,&Flash_Status);
			__set_PRIMASK(0); //��ȫ���ж�;
			if(can_addr != 0x00)
			{
				if(ret==STATUS_SUCCESS)//�����ɹ�
				{
					TxMessage.ExtId.bit.ExtId = (DEVICE_ADDR<<CMD_WIDTH)|cmd_list.CmdSuccess;
					TxMessage.DLC = 1;
					//TxMessage.Data[0] = 1;
					TxMessage.CAN_Tx_msg_data.msg_byte.data[0] = 1;
				}
				else//����ʧ��
				{
					TxMessage.ExtId.bit.ExtId = (DEVICE_ADDR<<CMD_WIDTH)|cmd_list.CmdFaild;
					TxMessage.DLC = 2;
					TxMessage.CAN_Tx_msg_data.msg_byte.data[0] = 2;
					TxMessage.CAN_Tx_msg_data.msg_byte.data[1] = (ret&0xFF);
				}
				CAN_Tx_Msg(&TxMessage);
			}
			return;
		}
		//CMD_List.WriteInfo������дFlash���ݵ������Ϣ������������ʼ��ַ�����ݴ�С
		//����ƫ�Ƶ�ַ�洢��Data[0]��Data[3]��,��ƫ������ʾ��ǰ���ݰ�������ļ���ʼ��ƫ����,ͬʱ����ʾд��FLASH��ƫ��
		//  ���ݴ�С�洢��Data[4]��Data[7]�У��ú���������Bootloader������ʵ�֣�APP������Բ���ʵ��
		if(can_cmd == cmd_list.WriteInfo)
		{
			__set_PRIMASK(1);
			//__disable_irq();
			start_addr = (((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[0])&0xFFFFFFFF)<<24)|\
						  (((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[1])&0x00FFFFFF)<<16)|\
						  (((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[2])&0x0000FFFF)<<8)|\
						  (((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[3])&0x000000FF)<<0);
				//start_addr   = APP_START_ADDR+addr_offset;
			data_size = (((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[4])&0xFFFFFFFF)<<24)|\
						(((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[5])&0x00FFFFFF)<<16)|\
					    (((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[6])&0x0000FFFF)<<8)|\
					    (((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[7])&0x000000FF)<<0);
			data_index   = 0;
			__set_PRIMASK(0);


			if(can_addr != 0x00)
			{
				TxMessage.ExtId.bit.ExtId = (DEVICE_ADDR<<CMD_WIDTH)|cmd_list.CmdSuccess;
				TxMessage.DLC       = 1;
				//TxMessage.Data[0] = 3;
				TxMessage.CAN_Tx_msg_data.msg_byte.data[0] = 3;
				CAN_Tx_Msg(&TxMessage);
			}
		}
		//CMD_List.Write���Ƚ����ݴ洢�ڱ��ػ������У�Ȼ��������ݵ�CRC����У����ȷ��д���ݵ�Flash��
		//ÿ��ִ�и����ݣ����ݻ������������ֽ���������pRxMessage->DLC�ֽڣ�
		//���������ﵽdata_size������2�ֽ�CRCУ���룩�ֽں�
		//�����ݽ���CRCУ�飬������У������������д��Flash��
		//�ú�����Bootloader�����б���ʵ�֣�APP������Բ���ʵ��
		if(can_cmd == cmd_list.Write)
		{
			if((data_index<data_size)&&(data_index<128))
			{
				__set_PRIMASK(1);//�ر�ȫ���ж�
				for(i=0;i<pRxMessage->DLC;i++)
				{
					data_temp[data_index++] =pRxMessage->CAN_Rx_msg_data.msg_byte.data[i];
				}
				__set_PRIMASK(0);//��ȫ���ж�
			}
			if((data_index>=data_size)||(data_index>=(128-2)))
			{
				crc_data = CRCcalc16(data_temp,data_size-2);//�Խ��յ���������CRCУ�飬��֤����������
				if(crc_data==((data_temp[data_size-2])|(data_temp[data_size-1]<<8)))
				{
					__set_PRIMASK(1);
					/*
					FLASH_Unlock();
					ret =  CAN_BOOT_ProgramDatatoFlash(start_addr,data_temp,data_size-2);
					FLASH_Lock();
					*/
					//�˴��ǽ����յ�������д��FLASH,�ؼ�֮��,��Ҫ��ϸ����
					for(i = 0;i<(data_size-2)>>1;i++)
					{
						write_temp[i] = data_temp[2*i]<<8|data_temp[2*i+1];
					}
					ret = Flash_WR(start_addr,write_temp,(data_size-2)>>1);
					__set_PRIMASK(0);
					if(can_addr != 0x00)
					{
						if(ret==STATUS_SUCCESS)//FLASHд��ɹ�,�ٴν���CRCУ��
						{
							/*
								crc_data = CRCcalc16((unsigned char*)(start_addr),data_size-2);//�ٴζ�д��Flash�е����ݽ���CRCУ�飬ȷ��д��Flash����������
								if(crc_data!=((data_temp[data_size-2])|(data_temp[data_size-1]<<8)))
								{
									TxMessage.ExtId.bit.ExtId   = (CAN_BOOT_GetAddrData()<<CMD_WIDTH)|cmd.CmdFaild;
									TxMessage.DLC     = 2;
									//TxMessage.Data[0] = 4;
									TxMessage.CAN_Tx_msg_data.msg_byte.data[0] = 4;
									TxMessage.CAN_Tx_msg_data.msg_byte.data[1] = 1;
								}
								else
								{
									TxMessage.ExtId.bit.ExtId    = (CAN_BOOT_GetAddrData()<<CMD_WIDTH)|cmd_list.CmdSuccess;
									TxMessage.DLC      = 0;
								//	TxMessage.Data[0]  = 0;
								}
								CAN_Tx_Msg(&TxMessage);
								*/
							//�˴������ٴν���CRC����,��Ϊ��д������ʱ�Ѿ����������.
							TxMessage.ExtId.bit.ExtId    = (CAN_BOOT_GetAddrData()<<CMD_WIDTH)|cmd_list.CmdSuccess;
							TxMessage.DLC      = 1;
							TxMessage.CAN_Tx_msg_data.msg_byte.data[0] = 0;
							CAN_Tx_Msg(&TxMessage);
						}
						else
						{
							//д�����
							TxMessage.ExtId.bit.ExtId   = (CAN_BOOT_GetAddrData()<<CMD_WIDTH)|cmd_list.CmdFaild;
							TxMessage.DLC     = 2;
							//TxMessage.Data[0] = 5;
							TxMessage.CAN_Tx_msg_data.msg_byte.data[0] = 4;
							TxMessage.CAN_Tx_msg_data.msg_byte.data[1] = ret&0xFF;
							CAN_Tx_Msg(&TxMessage);
						}
					}
					else
					{
						//��ַ����
						TxMessage.ExtId.bit.ExtId   = (CAN_BOOT_GetAddrData()<<CMD_WIDTH)|cmd_list.CmdFaild;
						TxMessage.DLC     = 1;
						//TxMessage.Data[0] = 6;
						//CAN_WriteData(&TxMessage);
						//TxMessage.Data[0] = 5;
						TxMessage.CAN_Tx_msg_data.msg_byte.data[0] = 5;
						TxMessage.CAN_Tx_msg_data.msg_byte.data[1] =0x01;
						CAN_Tx_Msg(&TxMessage);
					}
				}
			return;
			}
		}
		//CMD_List.Check���ڵ����߼��
		//�ڵ��յ�������󷵻ع̼��汾��Ϣ�͹̼����ͣ�
		//��������Bootloader�����APP���򶼱���ʵ��
		if(can_cmd == cmd_list.Check)//DSP����δʵ��,��ԱȽ�����ʵ��,��Ҫ��Ϊʵ��APP�ٴθ���Ӧ�ó���
		{
			if(can_addr != 0x00)
			{
				DEVICE_INFO.Device_addr.bits.Device_addr = DEVICE_ADDR;
				TxMessage.ExtId.bit.ExtId = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|cmd_list.CmdSuccess;
				TxMessage.CAN_Tx_msg_data.msg_byte.data[0] = (u8)(DEVICE_INFO.FW_Version>>24);;//���汾�ţ����ֽ�
				TxMessage.CAN_Tx_msg_data.msg_byte.data[1] = (u8)(DEVICE_INFO.FW_Version>>16);
				TxMessage.CAN_Tx_msg_data.msg_byte.data[2] = (u8)(DEVICE_INFO.FW_Version>>8);//�ΰ汾�ţ����ֽ�
				TxMessage.CAN_Tx_msg_data.msg_byte.data[3] = (u8)(DEVICE_INFO.FW_Version>>0);
				TxMessage.CAN_Tx_msg_data.msg_byte.data[4] = (u8)(DEVICE_INFO.FW_TYPE>>24);
				TxMessage.CAN_Tx_msg_data.msg_byte.data[5] = (u8)(DEVICE_INFO.FW_TYPE>>16);
				TxMessage.CAN_Tx_msg_data.msg_byte.data[6] = (u8)(DEVICE_INFO.FW_TYPE>>8);
				TxMessage.CAN_Tx_msg_data.msg_byte.data[7] = (u8)(DEVICE_INFO.FW_TYPE>>0);
				TxMessage.DLC = 8;
				CAN_Tx_Msg(&TxMessage);
			}
		}
		//CMD_List.Excute�����Ƴ�����ת��ָ����ִַ��
		//��������Bootloader��APP�����ж�����ʵ��
		if(can_cmd == cmd_list.Excute)//��������DSP���Ѿ�ʵ��
		{
			exe_type  = (((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[0])&0xFFFFFFFF)<<24)|\
						(((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[1])&0x00FFFFFF)<<16)|\
						(((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[2])&0x0000FFFF)<<8)|\
						(((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[3])&0x000000FF)<<0);
			if(exe_type == CAN_BL_APP)
			{
				if((*((uint32_t *)APP_START_ADDR)!=0xFFFFFFFF))
				{
					CAN_BOOT_JumpToApplication(APP_START_ADDR);
				}
			}
			return;
		}
		return;

}
