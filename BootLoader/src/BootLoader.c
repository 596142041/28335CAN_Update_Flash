/*
 * BootLoader.c
 *
 *  Created on: 2017��4��19��
 *      Author: admin
 */
#include "BootLoader.h"
#define DATA_LEN  520
#define READ_MAX  256
typedef  void (*pFunction)(void);
bootloader_data Boot_ID_info;
u8	   data_temp[DATA_LEN*2];
Uint16 write_temp[DATA_LEN];
Uint16 read_temp[READ_MAX];
u8 can_cmd        = (u8 )0x00;//ID��bit0~bit3λΪ������
u32 read_addr     = (u32)0x00;//��ȡ������ʼ��ַ
u32 read_len      = (u32)0x00;//��ȡ���ݳ���
u16 crc_data      = (u16)0x00;
u16 can_addr      = (u16)0x00;;//ID��bit4~bit15λΪ�ڵ��ַ
u32 start_addr    = (u32)0x00;//ÿһ�����ݵ���ʼ��ַ
u32 data_size     = (u32)0x00;//���ݰ��Ĵ�С
u32 data_index    = (u32)0x00;//����ָ��
u32 FlashSize     = (u32)0x00;
uint32_t exe_type = (u32)0x00;
Boot_CMD_LIST cmd_list =
{
	.Read        = 0x0A, //��ȡflash����
	.Erase       = 0x00, //����APP��������
	.Write       = 0x02, //�Զ��ֽ���ʽд����
	.Check       = 0x03, //���ڵ��Ƿ����ߣ�ͬʱ���ع̼���Ϣ
	.Excute      = 0x05, //ִ�й̼�
	.CmdFaild    = 0x09, //����ִ��ʧ��
	.WriteInfo   = 0x01, //���ö��ֽ�д������ز���(д��ʼ��ַ,������)
	.CmdSuccess  = 0x08, //����ִ�гɹ�
	.SetBaudRate = 0x04, //���ýڵ㲨����
};
Device_INFO DEVICE_INFO =
{
 .Device_addr.bits.reserve      = 0x00,
 .FW_Version                    = 0x0010101,
 .FW_TYPE.bits.FW_TYPE          = CAN_BL_BOOT,
 .FW_TYPE.bits.Chip_Value       = TMS320F28335,
 .Device_addr.bits.Device_addr  = DEVICE_ADDR,
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
	//  asm(" LB  0x310000"); //�ʼ���õ��ǻ����ת��ʽ,ֻ����תΪ�̶��ĵ�ַ
   // (*((void(*)(void))(Addr)))();  //�����ο�STM32����ת��ʽ
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
	TxMessage.CAN_num          = CANA;
	TxMessage.DLC              = 1;
	TxMessage.ExtId.bit.resved = 0x00;
	TxMessage.IDE              = CAN_ID_EXT;
	TxMessage.MBox_num         = 0x02;
	TxMessage.Tx_timeout_cnt   = 0x00;
	TxMessage.SAE_J1939_Flag   = 0;
	//��ȡ��ַ��Ϣ
	Boot_ID_info.ExtId.all     = pRxMessage->ExtId;
	can_cmd = Boot_ID_info.ExtId.bit.cmd;
	can_addr = Boot_ID_info.ExtId.bit.addr;
	if((can_addr!=DEVICE_ADDR)&&(can_addr!=0))
	{
		TxMessage.ExtId.bit.ExtId = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|cmd_list.CmdFaild;
		TxMessage.DLC = 1;
		TxMessage.CAN_Tx_msg_data.msg_byte.data[0] = DEVICE_ADDR_ERROR;
		CAN_Tx_Msg(&TxMessage);
		return;
	}
	//CMD_List.Erase������Flash�е����ݣ���Ҫ������Flash��С�洢��Data[0]��Data[3]��
	//�����������Bootloader������ʵ�֣���APP�����п��Բ���ʵ��
	//��Ҫ�ǲ���APP��������
		if(can_cmd == cmd_list.Erase)
		{
			__set_PRIMASK(1); //�ر�ȫ���ж�
			FlashSize    = (((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[0])&0xFFFFFFFF)<<0x18)|\
						   (((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[1])&0x00FFFFFF)<<0x10)|\
						   (((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[2])&0x0000FFFF)<<0x08)|\
						   (((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[3])&0x000000FF)<<0x00);
			FlashSize    = FlashSize>>1;
			u8 SECT_num  = 0;
			u32 SEC_temp = 0x00;
			SECT_num     = FlashSize/0x8000;
			if(FlashSize%0x8000 != 0)
			{
				SECT_num = SECT_num+1;
			}
			else
			{
				SECT_num = SECT_num;
			}
			if(SECT_num >5 )
			{
				SECT_num = 5;
			}
			switch (SECT_num)
			{
				case 1:
					SEC_temp = SECTORF;
					break;
				case 2:
					SEC_temp = SECTORF|SECTORE;
					break;
				case 3:
					SEC_temp = SECTORF|SECTORE|SECTORD;
					break;
				case 4:
					SEC_temp = SECTORF|SECTORE|SECTORD|SECTORC;
					break;
				case 5:
					SEC_temp = SECTORF|SECTORE|SECTORD|SECTORC|SECTORB;
					break;
				default:
					break;
			}
			ret = Flash_Erase(SEC_temp,&Flash_Status);
			__set_PRIMASK(0); //��ȫ���ж�;
			if(can_addr != 0x00)
			{
				if(ret==STATUS_SUCCESS)//�����ɹ�
				{
					TxMessage.ExtId.bit.ExtId                  = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|cmd_list.CmdSuccess;
					TxMessage.DLC                              = 1;
					TxMessage.CAN_Tx_msg_data.msg_byte.data[0] = cmd_list.Erase;;
				}
				else//����ʧ��
				{
					TxMessage.ExtId.bit.ExtId                  = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|cmd_list.CmdFaild;
					TxMessage.DLC                              = 3;
					TxMessage.CAN_Tx_msg_data.msg_byte.data[0] = cmd_list.Erase;
					TxMessage.CAN_Tx_msg_data.msg_byte.data[1] = ERASE_ERROR;
					TxMessage.CAN_Tx_msg_data.msg_byte.data[2] = (ret&0xFF);
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
			data_index = 0;
			start_addr = (((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[0])&0xFFFFFFFF)<<0x18)|\
						 (((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[1])&0x00FFFFFF)<<0x10)|\
						 (((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[2])&0x0000FFFF)<<0x08)|\
						 (((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[3])&0x000000FF)<<0x00);
			data_size  = (((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[4])&0xFFFFFFFF)<<0x18)|\
						 (((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[5])&0x00FFFFFF)<<0x10)|\
					     (((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[6])&0x0000FFFF)<<0x08)|\
					     (((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[7])&0x000000FF)<<0x00);
			__set_PRIMASK(0);
			if(can_addr != 0x00)
			{
				TxMessage.ExtId.bit.ExtId                  = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|cmd_list.CmdSuccess;
				TxMessage.DLC                              = 1;
				TxMessage.CAN_Tx_msg_data.msg_byte.data[0] = cmd_list.WriteInfo;
			}
			else
			{
				TxMessage.ExtId.bit.ExtId                  = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|cmd_list.CmdSuccess;
				TxMessage.DLC                              = 2;
				TxMessage.CAN_Tx_msg_data.msg_byte.data[0] = cmd_list.WriteInfo;
				TxMessage.CAN_Tx_msg_data.msg_byte.data[1] = DEVICE_ADDR_ERROR;
			}
			CAN_Tx_Msg(&TxMessage);
			return;
		}
		//CMD_List.Write���Ƚ����ݴ洢�ڱ��ػ������У�Ȼ��������ݵ�CRC����У����ȷ��д���ݵ�Flash��
		//ÿ��ִ�и����ݣ����ݻ������������ֽ���������pRxMessage->DLC�ֽڣ�
		//���������ﵽdata_size������2�ֽ�CRCУ���룩�ֽں�
		//�����ݽ���CRCУ�飬������У������������д��Flash��
		//�ú�����Bootloader�����б���ʵ�֣�APP������Բ���ʵ��
		if(can_cmd == cmd_list.Write)
		{
			if((data_index<data_size)&&(data_index<DATA_LEN*2))
			{
				for(i=0;i<pRxMessage->DLC;i++)
				{
					data_temp[data_index++] =pRxMessage->CAN_Rx_msg_data.msg_byte.data[i];
				}
			}
			if((data_index>=data_size)||(data_index>=(DATA_LEN*2-2)))
			{
				crc_data = CRCcalc16(data_temp,data_size-2);//�Խ��յ���������CRCУ�飬��֤����������
				if(crc_data==((data_temp[data_size-2])|(data_temp[data_size-1]<<8)))
				{
					__set_PRIMASK(1);
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
							TxMessage.ExtId.bit.ExtId                  = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|cmd_list.CmdSuccess;
							TxMessage.DLC                              = 0x01;
							TxMessage.CAN_Tx_msg_data.msg_byte.data[0] = cmd_list.Write;
						}
						else
						{
							//д�����
							TxMessage.ExtId.bit.ExtId                  = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|cmd_list.CmdFaild;
							TxMessage.DLC                              = 0x03;
							TxMessage.CAN_Tx_msg_data.msg_byte.data[0] = cmd_list.Write;
							TxMessage.CAN_Tx_msg_data.msg_byte.data[1] = WRITE_ERROR;
							TxMessage.CAN_Tx_msg_data.msg_byte.data[2] = ret&0xFF;
						}
					}
					else
					{
						//��ַ����
						TxMessage.ExtId.bit.ExtId                      = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|cmd_list.CmdFaild;
						TxMessage.DLC                                  = 0x02;
						TxMessage.CAN_Tx_msg_data.msg_byte.data[0]     = cmd_list.Write;
						TxMessage.CAN_Tx_msg_data.msg_byte.data[1]     = DEVICE_ADDR_ERROR;
					}
					CAN_Tx_Msg(&TxMessage);
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
				TxMessage.DLC                              = 0x08;
				TxMessage.ExtId.bit.ExtId                  = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|cmd_list.CmdSuccess;
				TxMessage.CAN_Tx_msg_data.msg_byte.data[0] = (u8)(DEVICE_INFO.FW_Version>>24);//���汾�ţ����ֽ�
				TxMessage.CAN_Tx_msg_data.msg_byte.data[1] = (u8)(DEVICE_INFO.FW_Version>>16);
				TxMessage.CAN_Tx_msg_data.msg_byte.data[2] = (u8)(DEVICE_INFO.FW_Version>>8);//�ΰ汾�ţ����ֽ�
				TxMessage.CAN_Tx_msg_data.msg_byte.data[3] = (u8)(DEVICE_INFO.FW_Version>>0);
				TxMessage.CAN_Tx_msg_data.msg_byte.data[4] = (u8)(DEVICE_INFO.FW_TYPE.bits.FW_TYPE>>16);
				TxMessage.CAN_Tx_msg_data.msg_byte.data[5] = (u8)(DEVICE_INFO.FW_TYPE.bits.FW_TYPE>>8);
				TxMessage.CAN_Tx_msg_data.msg_byte.data[6] = (u8)(DEVICE_INFO.FW_TYPE.bits.FW_TYPE>>0);
				TxMessage.CAN_Tx_msg_data.msg_byte.data[7] = (u8)(DEVICE_INFO.FW_TYPE.bits.Chip_Value>>0);

			}
			else
			{
				//��ַ����
				TxMessage.ExtId.bit.ExtId                      = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|cmd_list.CmdFaild;
				TxMessage.DLC                                  = 0x02;
				TxMessage.CAN_Tx_msg_data.msg_byte.data[0]     = cmd_list.Check;
				TxMessage.CAN_Tx_msg_data.msg_byte.data[1]     = DEVICE_ADDR_ERROR;
			}
			CAN_Tx_Msg(&TxMessage);
			return;
		}
		//cmd_list.read,��ȡflash����,
		//�����������ڶ�ȡ�ڲ��洢������
		//��������Bootloader��APP�����й�����ʵ��
		if(can_cmd == cmd_list.Read)
		{
			if(pRxMessage->DLC != 8)
			{
				TxMessage.DLC                              = 0x02;
				TxMessage.ExtId.bit.ExtId                  = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|cmd_list.CmdFaild;
				TxMessage.CAN_Tx_msg_data.msg_byte.data[0] = (u8)cmd_list.Read;//���汾�ţ����ֽ�
				TxMessage.CAN_Tx_msg_data.msg_byte.data[1] = (u8)MSG_DATA_LEN_ERROR;
				CAN_Tx_Msg(&TxMessage);
				return;
			}
			read_addr =  (((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[0])&0xFFFFFFFF)<<0x18)|\
						 (((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[1])&0x00FFFFFF)<<0x10)|\
						 (((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[2])&0x0000FFFF)<<0x08)|\
						 (((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[3])&0x000000FF)<<0x00);
			read_len  =  (((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[4])&0xFFFFFFFF)<<0x18)|\
						 (((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[5])&0x00FFFFFF)<<0x10)|\
					     (((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[6])&0x0000FFFF)<<0x08)|\
					     (((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[7])&0x000000FF)<<0x00);
			if(read_len > READ_MAX)
			{
				TxMessage.DLC                              = 0x02;
				TxMessage.ExtId.bit.ExtId                  = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|cmd_list.CmdFaild;
				TxMessage.CAN_Tx_msg_data.msg_byte.data[0] = (u8)cmd_list.Read;//���汾�ţ����ֽ�
				TxMessage.CAN_Tx_msg_data.msg_byte.data[1] = (u8)READ_LEN_ERROR;
				CAN_Tx_Msg(&TxMessage);
				return;
			}
			u16 read_len_temp  = 0;
			if(read_len%2 == 0)//��Ϊÿ��ֻ�ܶ�ȡN����
			{
				read_len_temp = read_len;
			}
			else
			{
				read_len_temp = read_len+1;
			}
			read_len_temp = read_len_temp>>1;
			__set_PRIMASK(1);
			Flash_RD(read_addr,read_temp,read_len_temp);
			__set_PRIMASK(0);
			for(i = 0;i <read_len_temp;i++)
			{
				data_temp[i*2+0] = (read_temp[i]>>8)&0xFF;
				data_temp[i*2+1] = read_temp[i]&0xFF;
			}
			data_index = 0;
			i = 0;
			while(data_index < read_len)
			{
				int temp;
				temp = read_len - data_index;
				if (temp >= 8)
				{
					TxMessage.DLC             = 0x08;
					TxMessage.ExtId.bit.ExtId = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|cmd_list.CmdSuccess;
					for (i = 0; i < TxMessage.DLC; i++)
					{
						TxMessage.CAN_Tx_msg_data.msg_byte.data[i] = data_temp[data_index];
						data_index++;
					}

				}
				else
				{
					TxMessage.DLC             = temp;
					TxMessage.ExtId.bit.ExtId = (DEVICE_INFO.Device_addr.bits.Device_addr<<CMD_WIDTH)|cmd_list.CmdSuccess;
					for (i = 0; i < TxMessage.DLC; i++)
					{
						TxMessage.CAN_Tx_msg_data.msg_byte.data[i] = data_temp[data_index];
						data_index++;
					}
				}
				CAN_Tx_Msg(&TxMessage);
			}
			data_index = 0;
			return;
		}
		//CMD_List.Excute�����Ƴ�����ת��ָ����ִַ��
		//��������Bootloader��APP�����ж�����ʵ��
		if(can_cmd == cmd_list.Excute)//��������DSP���Ѿ�ʵ��
		{
			exe_type  = (((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[0])&0xFFFFFFFF)<<0x10)|\
						(((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[1])&0x00FFFFFF)<<0x08)|\
						(((u32)(pRxMessage->CAN_Rx_msg_data.msg_byte.data[2])&0x0000FFFF)<<0x00);
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
unsigned short int Check_APP(uint32_t Addr)
{
	return 0;
}
