/*
 * CANA.c
 *
 *  Created on: 2017��4��16��
 *      Author: admin
 */
#include "CANA.h"
CanTxMsg can_tx_msg;
CanRxMsg can_rx_msg =
{
	.rx_update = NON_CHANGE,
	.CAN_num = CANA,
	.DLC = 8,
	.ExtId = 0x00,
	.IDE = CAN_ID_EXT,
};
void CAN_GPIO_Config(CAN_Num CAN)
{

	if (CAN == CANA)
	{
		EALLOW;
		GpioCtrlRegs.GPAPUD.bit.GPIO18 = 0;	// Enable pull-up for GPIO18 (CANRXA)
		GpioCtrlRegs.GPAPUD.bit.GPIO19 = 0;	// Enable pull-up for GPIO19 (CANTXA)
		GpioCtrlRegs.GPAQSEL2.bit.GPIO18 = 3; // Asynch qual for GPIO18 (CANRXA)
		GpioCtrlRegs.GPAMUX2.bit.GPIO18 = 3; // Configure GPIO18 for CANRXA operation
		GpioCtrlRegs.GPAMUX2.bit.GPIO19 = 3; // Configure GPIO19 for CANTXA operation
		EDIS;
	}
	else if (CAN == CANB)
	{
		EALLOW;
		GpioCtrlRegs.GPAPUD.bit.GPIO12 = 0; // Enable pull-up for GPIO12 (CANTXB)
		GpioCtrlRegs.GPAPUD.bit.GPIO13 = 0; // Enable pull-up for GPIO13 (CANRXB)
		GpioCtrlRegs.GPAQSEL1.bit.GPIO13 = 3; // Asynch qual for GPIO13 (CANRXB)
		GpioCtrlRegs.GPAMUX1.bit.GPIO12 = 2; // Configure GPIO12 for CANTXB operation
		GpioCtrlRegs.GPAMUX1.bit.GPIO13 = 2; // Configure GPIO13 for CANRXB operation
		EDIS;
	}
	else
	{
		return;
	}

}
void CAN_Config(CAN_Num CAN)
{
	struct ECAN_REGS ECan_Reg_Shadow;
	//-------------------------
	volatile struct ECAN_REGS *ECanReg;
	volatile struct ECAN_MBOXES *ECanMboxe;
	if (CAN == CANA)
	{
		EALLOW;
		SysCtrlRegs.PCLKCR0.bit.ECANAENCLK = 1;
		EDIS;
		ECanReg = &ECanaRegs;
		ECanMboxe = &ECanaMboxes;
	}
	else if (CAN == CANB)
	{
		EALLOW;
		SysCtrlRegs.PCLKCR0.bit.ECANBENCLK = 1;
		EDIS;
		ECanReg = &ECanbRegs;
		ECanMboxe = &ECanbMboxes;
	}
	else
	{
		return;
	}

	EALLOW;		// EALLOW enables access to protected bits
	/* Configure eCAN RX and TX pins for CAN operation using eCAN regs*/
	ECan_Reg_Shadow.CANTIOC.all = ECanReg->CANTIOC.all;
	ECan_Reg_Shadow.CANTIOC.bit.TXFUNC = 1;
	ECanReg->CANTIOC.all = ECan_Reg_Shadow.CANTIOC.all;
	ECan_Reg_Shadow.CANRIOC.all = ECanReg->CANRIOC.all;
	ECan_Reg_Shadow.CANRIOC.bit.RXFUNC = 1;
	ECanReg->CANRIOC.all = ECan_Reg_Shadow.CANRIOC.all;
	/* Configure eCAN for HECC mode - (reqd to access mailboxes 16 thru 31) */
	// HECC mode also enables time-stamping feature
	//ѡ��CAN����ģʽ������eCANģʽ,32�����䶼������
	ECan_Reg_Shadow.CANMC.all = ECanReg->CANMC.all;
	ECan_Reg_Shadow.CANMC.bit.SCB = 1;
	ECanReg->CANMC.all = ECan_Reg_Shadow.CANMC.all;

	/********************************************************************************
	 * Initialize all bits of 'Master Control Field' to zero
	 * Some bits of MSGCTRL register come up in an unknown state. For proper operation,
 	 * all bits (including reserved bits) of MSGCTRL must be initialized to zero
 	 *********************************************************************************/
	ECanMboxe->MBOX0.MSGCTRL.all = 0x00000000;
	ECanMboxe->MBOX1.MSGCTRL.all = 0x00000000;
	ECanMboxe->MBOX2.MSGCTRL.all = 0x00000000;
	ECanMboxe->MBOX3.MSGCTRL.all = 0x00000000;
	ECanMboxe->MBOX4.MSGCTRL.all = 0x00000000;
	ECanMboxe->MBOX5.MSGCTRL.all = 0x00000000;
	ECanMboxe->MBOX6.MSGCTRL.all = 0x00000000;
	ECanMboxe->MBOX7.MSGCTRL.all = 0x00000000;
	ECanMboxe->MBOX8.MSGCTRL.all = 0x00000000;
	ECanMboxe->MBOX9.MSGCTRL.all = 0x00000000;
	ECanMboxe->MBOX10.MSGCTRL.all = 0x00000000;
	ECanMboxe->MBOX11.MSGCTRL.all = 0x00000000;
	ECanMboxe->MBOX12.MSGCTRL.all = 0x00000000;
	ECanMboxe->MBOX13.MSGCTRL.all = 0x00000000;
	ECanMboxe->MBOX14.MSGCTRL.all = 0x00000000;
	ECanMboxe->MBOX15.MSGCTRL.all = 0x00000000;
	ECanMboxe->MBOX16.MSGCTRL.all = 0x00000000;
	ECanMboxe->MBOX17.MSGCTRL.all = 0x00000000;
	ECanMboxe->MBOX18.MSGCTRL.all = 0x00000000;
	ECanMboxe->MBOX19.MSGCTRL.all = 0x00000000;
	ECanMboxe->MBOX20.MSGCTRL.all = 0x00000000;
	ECanMboxe->MBOX21.MSGCTRL.all = 0x00000000;
	ECanMboxe->MBOX22.MSGCTRL.all = 0x00000000;
	ECanMboxe->MBOX23.MSGCTRL.all = 0x00000000;
	ECanMboxe->MBOX24.MSGCTRL.all = 0x00000000;
	ECanMboxe->MBOX25.MSGCTRL.all = 0x00000000;
	ECanMboxe->MBOX26.MSGCTRL.all = 0x00000000;
	ECanMboxe->MBOX27.MSGCTRL.all = 0x00000000;
	ECanMboxe->MBOX28.MSGCTRL.all = 0x00000000;
	ECanMboxe->MBOX29.MSGCTRL.all = 0x00000000;
	ECanMboxe->MBOX30.MSGCTRL.all = 0x00000000;
	ECanMboxe->MBOX31.MSGCTRL.all = 0x00000000;

// TAn, RMPn, GIFn bits are all zero upon reset and are cleared again
//	as a matter of precaution.
	/* Clear all TAn bits */
	ECanReg->CANTA.all = 0xFFFFFFFF;
	/* Clear all RMPn bits */
	ECanReg->CANRMP.all = 0xFFFFFFFF;
	/* Clear all interrupt flag bits */
	ECanReg->CANGIF0.all = 0xFFFFFFFF;
	ECanReg->CANGIF1.all = 0xFFFFFFFF;

	/* Configure bit timing parameters for eCANA*/
	//��ʼ����CAN���߲�����
	ECan_Reg_Shadow.CANMC.all = ECanReg->CANMC.all;
	ECan_Reg_Shadow.CANMC.bit.CCR = 1;            // Set CCR = 1
	ECanReg->CANMC.all = ECan_Reg_Shadow.CANMC.all;
	ECan_Reg_Shadow.CANES.all = ECanReg->CANES.all;
	do
	{
		ECan_Reg_Shadow.CANES.all = ECanReg->CANES.all;
	}
	while (ECan_Reg_Shadow.CANES.bit.CCE != 1);  // Wait for CCE bit to be set..
	ECan_Reg_Shadow.CANBTC.all = 0;
#if (CPU_FRQ_150MHZ)
	/********************************************************
	 *CANͨѶ����������Ϊ500kbps
	 *******************************************************/
	ECan_Reg_Shadow.CANBTC.bit.BRPREG = 9;
	ECan_Reg_Shadow.CANBTC.bit.TSEG2REG = 2;
	ECan_Reg_Shadow.CANBTC.bit.TSEG1REG = 10;
#endif
	ECan_Reg_Shadow.CANBTC.bit.SAM = 1;
	ECanReg->CANBTC.all = ECan_Reg_Shadow.CANBTC.all;
	ECan_Reg_Shadow.CANMC.all = ECanReg->CANMC.all;
	ECan_Reg_Shadow.CANMC.bit.CCR = 0;            // Set CCR = 0
	ECanReg->CANMC.all = ECan_Reg_Shadow.CANMC.all;
	ECan_Reg_Shadow.CANES.all = ECanReg->CANES.all;
	do
	{
		ECan_Reg_Shadow.CANES.all = ECanReg->CANES.all;
	}
	while (ECan_Reg_Shadow.CANES.bit.CCE != 0); // Wait for CCE bit to be  cleared..
	/* Disable all Mailboxes  */
	ECanReg->CANME.all = 0;		// Required before writing the MSGIDs
	EDIS;

}
void CAN_Tx_Msg(CanTxMsg *can_tx_msg)  //������Ϣ
{
	Uint32 mbox_enable_temp  = 0x0000;
	Uint32 mbox_disable_temp = 0x0000;
	Uint32 mbox_dir_temp     = 0x0000;
	mbox_enable_temp = 1<<(can_tx_msg->MBox_num);
	mbox_disable_temp = ~(1<<(can_tx_msg->MBox_num));
	mbox_dir_temp = ~(1<<(can_tx_msg->MBox_num));
	volatile struct ECAN_REGS ECan_Reg_Shadow;
	volatile struct ECAN_REGS *ECanReg;
	volatile struct MBOX *Mailbox;
	if(can_tx_msg->CAN_num == CANA)
	{
		ECanReg = &ECanaRegs;
		Mailbox = &ECanaMboxes.MBOX0+can_tx_msg->MBox_num;
	}
	else if (can_tx_msg->CAN_num == CANB)
	{
		ECanReg = &ECanbRegs;
		Mailbox = &ECanbMboxes.MBOX0+can_tx_msg->MBox_num;
	}
	else
	{
		return;
	}
	ECan_Reg_Shadow.CANME.all  = ECanReg->CANME.all;
	ECan_Reg_Shadow.CANME.all &= mbox_disable_temp;
	ECanReg->CANME.all    = ECan_Reg_Shadow.CANME.all;
	if(can_tx_msg->IDE == CAN_ID_STD)
	{

		Mailbox->MSGID.all = can_tx_msg->StdId.bit.StdId; //standard identifier
		Mailbox->MSGID.bit.IDE = can_tx_msg->IDE;
	}
	else if(can_tx_msg->IDE == CAN_ID_EXT)
	{
		if(can_tx_msg->SAE_J1939_Flag == 0)
		{
			Mailbox->MSGID.all = can_tx_msg->ExtId.bit.ExtId; //extended identifier.
			Mailbox->MSGID.bit.IDE = can_tx_msg->IDE;
		}
		else
		{
			Mailbox->MSGID.all = can_tx_msg->SAE_J1939_ID.id; //extended identifier.
			Mailbox->MSGID.bit.IDE = can_tx_msg->IDE;
		}
	}
   ECan_Reg_Shadow.CANMD.all = ECanReg->CANMD.all;
   ECan_Reg_Shadow.CANMD.all &=mbox_dir_temp;//�������乤������,0��ʾ���乤���ڷ���,1��ʾ�����ڽ���
   ECanReg->CANMD.all = ECan_Reg_Shadow.CANMD.all;
   ECan_Reg_Shadow.CANME.all = ECanReg->CANME.all;
   ECan_Reg_Shadow.CANME.all |= mbox_enable_temp;//ʹ������
   ECanReg->CANME.all = ECan_Reg_Shadow.CANME.all;
   Mailbox->MSGCTRL.bit.DLC = can_tx_msg->DLC;//���ݳ���
   Mailbox->MDL.byte.BYTE0 = can_tx_msg->CAN_Tx_msg_data.msg_Byte.byte0;
   Mailbox->MDL.byte.BYTE1 = can_tx_msg->CAN_Tx_msg_data.msg_Byte.byte1;
   Mailbox->MDL.byte.BYTE2 = can_tx_msg->CAN_Tx_msg_data.msg_Byte.byte2;
   Mailbox->MDL.byte.BYTE3 = can_tx_msg->CAN_Tx_msg_data.msg_Byte.byte3;
   Mailbox->MDH.byte.BYTE4 = can_tx_msg->CAN_Tx_msg_data.msg_Byte.byte4;
   Mailbox->MDH.byte.BYTE5 = can_tx_msg->CAN_Tx_msg_data.msg_Byte.byte5;
   Mailbox->MDH.byte.BYTE6 = can_tx_msg->CAN_Tx_msg_data.msg_Byte.byte6;
   Mailbox->MDH.byte.BYTE7 = can_tx_msg->CAN_Tx_msg_data.msg_Byte.byte7;
   ECan_Reg_Shadow.CANTRS.all = 0;
   ECan_Reg_Shadow.CANTRS.all |= mbox_enable_temp;             // Set TRS for mailbox under test
   ECanReg->CANTRS.all = ECan_Reg_Shadow.CANTRS.all;
   do
	{
	ECan_Reg_Shadow.CANTA.all = ECanReg->CANTA.all;

	} while(((ECan_Reg_Shadow.CANTA.all&mbox_enable_temp) == 0 ));
   ECan_Reg_Shadow.CANTA.all = 0;
   ECan_Reg_Shadow.CANTA.all = mbox_enable_temp;
   ECanReg->CANTA.all = ECan_Reg_Shadow.CANTA.all;
}
void CAN_Rx_Msg(CanRxMsg *can_rx_msg)  //������Ϣ
{
	//	struct ECAN_REGS ECan_Reg_Shadow;
		volatile struct MBOX *Mailbox;
		if(can_rx_msg->CAN_num == CANA)
		{
			Mailbox = &ECanaMboxes.MBOX0+can_rx_msg->MBox_num;
		}
		else if (can_rx_msg->CAN_num == CANB)
		{
			Mailbox =&ECanbMboxes.MBOX0+can_rx_msg->MBox_num;
		}
		else
		{
			return;
		}
		can_rx_msg->DLC = Mailbox->MSGCTRL.bit.DLC;
		can_rx_msg->IDE = Mailbox->MSGID.bit.IDE;
		if(can_rx_msg->IDE == CAN_ID_EXT)
		{
			can_rx_msg->ExtId = Mailbox->MSGID.all&0x1FFFFFFF;
			can_rx_msg->SAE_J1939_ID.id = can_rx_msg->ExtId;
		}
		else if(can_rx_msg->IDE == CAN_ID_STD)
		{
			can_rx_msg->StdId = Mailbox->MSGID.bit.STDMSGID;
		}
		can_rx_msg->CAN_Rx_msg_data.msg_Byte.byte0 = Mailbox->MDL.byte.BYTE0;
		can_rx_msg->CAN_Rx_msg_data.msg_Byte.byte1 = Mailbox->MDL.byte.BYTE1;
		can_rx_msg->CAN_Rx_msg_data.msg_Byte.byte2 = Mailbox->MDL.byte.BYTE2;
		can_rx_msg->CAN_Rx_msg_data.msg_Byte.byte3 = Mailbox->MDL.byte.BYTE3;
		can_rx_msg->CAN_Rx_msg_data.msg_Byte.byte4 = Mailbox->MDH.byte.BYTE4;
		can_rx_msg->CAN_Rx_msg_data.msg_Byte.byte5 = Mailbox->MDH.byte.BYTE5;
		can_rx_msg->CAN_Rx_msg_data.msg_Byte.byte6 = Mailbox->MDH.byte.BYTE6;
 		can_rx_msg->CAN_Rx_msg_data.msg_Byte.byte7 = Mailbox->MDH.byte.BYTE7;
}
#if USE_CANA
/*
 * ������31����Ϊ��������,
 */
static void CANA_RX_Config(void)
{
		struct ECAN_REGS ECan_Reg_Shadow;
		ECan_Reg_Shadow.CANME.all = ECanaRegs.CANME.all;
		ECan_Reg_Shadow.CANME.bit.ME31 = 0;//��ʹ������31
		ECanaRegs.CANME.all = ECan_Reg_Shadow.CANME.all;
		/*----------���´��������ý����������ش���------------*/
		//����31�������
		ECanaMboxes.MBOX31.MSGCTRL.bit.DLC = 8;//�������ݳ��ȣ�Ӧ����û�����;
		ECanaMboxes.MBOX31.MSGID.all = 0x1340;//���ý�����Ϣ����ЧID
		ECanaMboxes.MBOX31.MSGID.bit.AME =1;//����ʹ��λ,
		ECanaMboxes.MBOX31.MSGID.bit.IDE = CAN_ID_EXT;
		/*
			LAM[28:0]
			��Щλ����һ��������Ϣ�������ʶ��λ�����Ρ�
			1 ��Խ��ܵ��ı�ʶ������Ӧλ�� ����һ�� 0 �� 1�� �޹أ� ��
			0 ���յ��ı�ʶ��λֵ������ MSGID �Ĵ�������Ӧ��ʶ��λ��ƥ�䡣
		*/

		ECanaLAMRegs.LAM31.all = 0x000000F;//
		 /*
			LAMI ���ؽ������α�ʶ����չλ
			1 ���Խ��ձ�׼����չ֡�� ����չ֡������£� ��ʶ�������� 29 λ���洢�������У� ���ؽ������μ�
			���������� 29 λ��������ʹ�á� ��һ����׼֡������£� ֻ�б�ʶ����ͷ 11 ��λ�� 28 �� 18 λ��
			�ͱ��ؽ������α�ʹ�á�
			0 �洢�������еı�ʶ����չλ��������Щ��ϢӦ�ñ����յ�
		*/
		ECanaLAMRegs.LAM31.bit.LAMI = 1;
		ECanaRegs.CANRMP.all      = 0xFFFFFFFF;
		ECan_Reg_Shadow.CANMD.all     = ECanaRegs.CANMD.all;
		//ECan_Reg_Shadow.CANMD.bit.MD1 = 1;
		ECan_Reg_Shadow.CANMD.bit.MD31 = 1;
		ECanaRegs.CANMD.all       = ECan_Reg_Shadow.CANMD.all;

		ECan_Reg_Shadow.CANME.all       = ECanaRegs.CANME.all;
		//ECan_Reg_Shadow.CANME.bit.ME1   = 1;//ʹ������1
		ECan_Reg_Shadow.CANME.bit.ME31  = 1;//ʹ������1
		ECanaRegs.CANME.all         = ECan_Reg_Shadow.CANME.all;
}
#endif
#if USE_CANB
static void CANB_RX_Config(void)
{
	struct ECAN_REGS ECan_Reg_Shadow;
	ECan_Reg_Shadow.CANME.all = ECanbRegs.CANME.all;
	ECan_Reg_Shadow.CANME.bit.ME1 = 0;//��ʹ������1
	ECan_Reg_Shadow.CANME.bit.ME31 = 0;//��ʹ������31
	ECanbRegs.CANME.all = ECan_Reg_Shadow.CANME.all;
	/*----------���´��������ý����������ش���------------*/
	//����1�������
	ECanbMboxes.MBOX1.MSGCTRL.bit.DLC = 8;//�������ݳ��ȣ�Ӧ����û�����;
	ECanbMboxes.MBOX1.MSGID.all = 0x0123C;//���ý�����Ϣ����ЧID
	ECanbMboxes.MBOX1.MSGID.bit.AME =1;//����ʹ��λ,
	ECanbMboxes.MBOX1.MSGID.bit.IDE = CAN_ID_EXT;
	/*
		LAM[28:0]
		��Щλ����һ��������Ϣ�������ʶ��λ�����Ρ�
		1 ��Խ��ܵ��ı�ʶ������Ӧλ�� ����һ�� 0 �� 1�� �޹أ� ��
		0 ���յ��ı�ʶ��λֵ������ MSGID �Ĵ�������Ӧ��ʶ��λ��ƥ�䡣
	*/

	ECanbLAMRegs.LAM1.all = 0x000000F;//
	 /*
		LAMI ���ؽ������α�ʶ����չλ
		1 ���Խ��ձ�׼����չ֡�� ����չ֡������£� ��ʶ�������� 29 λ���洢�������У� ���ؽ������μ�
		���������� 29 λ��������ʹ�á� ��һ����׼֡������£� ֻ�б�ʶ����ͷ 11 ��λ�� 28 �� 18 λ��
		�ͱ��ؽ������α�ʹ�á�
		0 �洢�������еı�ʶ����չλ��������Щ��ϢӦ�ñ����յ�
	*/
	ECanbLAMRegs.LAM1.bit.LAMI = 1;
	//����31�������
	ECanbMboxes.MBOX31.MSGCTRL.bit.DLC = 8;//�������ݳ��ȣ�Ӧ����û�����;
	ECanbMboxes.MBOX31.MSGID.all = 0x07909ABC;//���ý�����Ϣ����ЧID
	ECanbMboxes.MBOX31.MSGID.bit.AME =1;//����ʹ��λ,
	ECanbMboxes.MBOX31.MSGID.bit.IDE = CAN_ID_EXT;
	/*
		LAM[28:0]
		��Щλ����һ��������Ϣ�������ʶ��λ�����Ρ�
		1 ��Խ��ܵ��ı�ʶ������Ӧλ�� ����һ�� 0 �� 1�� �޹أ� ��
		0 ���յ��ı�ʶ��λֵ������ MSGID �Ĵ�������Ӧ��ʶ��λ��ƥ�䡣
	*/

	ECanbLAMRegs.LAM31.all = 0x0000000;//
	 /*
		LAMI ���ؽ������α�ʶ����չλ
		1 ���Խ��ձ�׼����չ֡�� ����չ֡������£� ��ʶ�������� 29 λ���洢�������У� ���ؽ������μ�
		���������� 29 λ��������ʹ�á� ��һ����׼֡������£� ֻ�б�ʶ����ͷ 11 ��λ�� 28 �� 18 λ��
		�ͱ��ؽ������α�ʹ�á�
		0 �洢�������еı�ʶ����չλ��������Щ��ϢӦ�ñ����յ�
	*/
	ECanbLAMRegs.LAM31.bit.LAMI = 1;
	ECanbRegs.CANRMP.all      = 0xFFFFFFFF;
	ECan_Reg_Shadow.CANMD.all     = ECanbRegs.CANMD.all;
	ECan_Reg_Shadow.CANMD.bit.MD1 = 1;
	ECan_Reg_Shadow.CANMD.bit.MD31 = 1;
	ECanbRegs.CANMD.all       = ECan_Reg_Shadow.CANMD.all;

	ECan_Reg_Shadow.CANME.all       = ECanbRegs.CANME.all;
	ECan_Reg_Shadow.CANME.bit.ME1   = 1;//ʹ������1
	ECan_Reg_Shadow.CANME.bit.ME31  = 1;//ʹ������1
	ECanbRegs.CANME.all         = ECan_Reg_Shadow.CANME.all;
}
#endif
void CAN_Rx_Config(void)
{
#if USE_CANA
	CANA_RX_Config();
#endif
/*---------------------------------*/
#if USE_CANB
	CANB_RX_Config();
#endif
}
void CAN_Rx_IT_Concig(void)
{
	EALLOW;
	ECanaRegs.CANMIM.bit.MIM31 = 1;//ʹ���ж�����31���ж�;
	ECanaRegs.CANMIL.bit.MIL31 = 1;//���ж�31�������ж�1;
	ECanaRegs.CANGIM.bit.I1EN = 1;//ʹ���ж�1;
	EDIS;
}
__interrupt void Ecana_isr1(void)
{
	if(ECanaRegs.CANGIF1.bit.GMIF1 == 1)
	{
		if(ECanaRegs.CANRMP.bit.RMP31 == 1)
		{
			can_rx_msg.rx_update = UPDATE;
			can_rx_msg.MBox_num = ECanaRegs.CANGIF1.bit.MIV1;
			can_rx_msg.DLC = ECanaMboxes.MBOX31.MSGCTRL.bit.DLC;
			can_rx_msg.IDE = ECanaMboxes.MBOX31.MSGID.bit.IDE;
			if(can_rx_msg.IDE == CAN_ID_EXT)
			{
				can_rx_msg.ExtId = ECanaMboxes.MBOX31.MSGID.all&0x1FFFFFFF;
				can_rx_msg.SAE_J1939_ID.id = can_rx_msg.ExtId;
			}
			else if(can_rx_msg.IDE == CAN_ID_STD)
			{
				can_rx_msg.StdId = ECanaMboxes.MBOX31.MSGID.bit.STDMSGID;
			}
		   can_rx_msg.CAN_Rx_msg_data.msg_Byte.byte0 = ECanaMboxes.MBOX31.MDL.byte.BYTE0;
		   can_rx_msg.CAN_Rx_msg_data.msg_Byte.byte1 = ECanaMboxes.MBOX31.MDL.byte.BYTE1;
		   can_rx_msg.CAN_Rx_msg_data.msg_Byte.byte2 = ECanaMboxes.MBOX31.MDL.byte.BYTE2;
		   can_rx_msg.CAN_Rx_msg_data.msg_Byte.byte3 = ECanaMboxes.MBOX31.MDL.byte.BYTE3;
		   can_rx_msg.CAN_Rx_msg_data.msg_Byte.byte4 = ECanaMboxes.MBOX31.MDH.byte.BYTE4;
		   can_rx_msg.CAN_Rx_msg_data.msg_Byte.byte5 = ECanaMboxes.MBOX31.MDH.byte.BYTE5;
		   can_rx_msg.CAN_Rx_msg_data.msg_Byte.byte6 = ECanaMboxes.MBOX31.MDH.byte.BYTE6;
		   can_rx_msg.CAN_Rx_msg_data.msg_Byte.byte7 = ECanaMboxes.MBOX31.MDH.byte.BYTE7;
		   ECanaRegs.CANRMP.bit.RMP31 = 1;
		}
	}
	PieCtrlRegs.PIEACK.bit.ACK9 = 1;


}
