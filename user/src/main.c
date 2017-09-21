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
	DINT;
	DRTM;
	InitSysCtrl();
	CAN_GPIO_Config(CANA);
	LED_GPIO_Config();
	CsmUnlock();
	InitPieCtrl();
	IER = 0x0000;
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
	/*
	FLASH_ST Flash_status;
	Uint16 status = 0x0001;
	//status = Flash_Verify((Uint16*)APP_INFO_ADDR,app_check,3,&Flash_status);
	if(status == STATUS_SUCCESS)
	{
		CAN_BOOT_JumpToApplication(APP_START_ADDR);
	}
	*/
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
