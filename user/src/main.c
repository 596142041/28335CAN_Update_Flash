/*
 * main.c
 *
 *  Created on: 2017��4��16��
 *      Author: admin
 */
/**
 *�����˼·��:
 * ��һ��:����FLASH������д��Ͳ���;
 * �ڶ���:��֤CAN���߽�������;
 * ������:����ǰ��Ĳ����������ۺ�
 * ��һ����֤FLASH���ܺ�����������;
 * ��Ҫ����������ܺ���:��ĳ����ַд��ʹ�ĳ����ַ�����ĺ���
 ***************************************/
#include "main.h"
#include "BootLoader.h"
#include "LED.h"
int main(void)
{
	u8 i = 0;
	DINT;
	DRTM;
	InitSysCtrl();
	CAN_GPIO_Config(CANA);
	LED_GPIO_Config();
	
	CsmUnlock();
	InitPieCtrl();
	IER = 0x0000;	// Disable CPU interrupts and clear all CPU interrupt flags:
	IFR = 0x0000;
	InitPieVectTable();
	EALLOW;
	PieVectTable.TINT0 = &cpu_timer0_isr;
	PieVectTable.ECAN1INTA = &Ecana_isr1;
	EDIS;
	MemCopy(&RamfuncsLoadStart, &RamfuncsLoadEnd, &RamfuncsRunStart);
	MemCopy(&Flash28_API_LoadStart, &Flash28_API_LoadEnd,&Flash28_API_RunStart);
	InitFlash();
	FlashAPI_Init();
	CAN_Config(CANA);
	CAN_Rx_Config();
	CAN_Rx_IT_Concig();
	//����LEDָʾ��
	LED_Timer_Config();
	//------------------------------------
	//�����ж�
	PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
	PieCtrlRegs.PIEIER9.bit.INTx6 = 1; //CANA���ж�1
	IER |= M_INT9;
	IER |= M_INT1;
	__enable_irq();
	for(i = 0; i< 8;i++)
	{
		can_tx_msg.CAN_Tx_msg_data.msg_byte.data[i] =i;
	}
	can_tx_msg.CAN_num  = CANA;
	can_tx_msg.DLC = 5;
	can_tx_msg.ExtId.bit.ExtId = 0x1134522;
	can_tx_msg.ExtId.bit.resved = 0x00;
	can_tx_msg.IDE = CAN_ID_EXT;
	can_tx_msg.MBox_num = 0x01;
	can_tx_msg.SAE_J1939_Flag = 0;
	can_tx_msg.StdId.all = 0x00;
	can_tx_msg.Tx_timeout_cnt = 0x00;
	while (1)
	{
		if(updata_info.time_out_flag == 0)
		{
			if(can_rx_msg.rx_update == UPDATE)
			{
				if(CpuTimer0Regs.TCR.bit.TSS == 0)
				{
					CpuTimer0Regs.TCR.bit.TSS = 1;
				}
				updata_info.time_out_flag = 0;
				can_rx_msg.rx_update = NON_CHANGE;
				CAN_BOOT_ExecutiveCommand(&can_rx_msg);
				 GpioDataRegs.GPBTOGGLE.bit.GPIO34 = 1;
			}
		}
		else
		{
			CAN_BOOT_JumpToApplication(APP_START_ADDR);
		}

	}
}
