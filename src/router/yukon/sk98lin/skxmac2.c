/******************************************************************************
 *
 * Name:	$Id: //Release/Yukon_1G/Shared/common/V6/skxmac2.c#21 $
 * Project:	Gigabit Ethernet Adapters, Common Modules
 * Version:	$Revision: #21 $, $Change: 7576 $
 * Date:	$DateTime: 2011/09/21 15:31:37 $
 * Purpose:	Contains functions to initialize the MACs and PHYs
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	LICENSE:
 *	(C)Copyright Marvell.
 *	
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *	
 *	The information in this file is provided "AS IS" without warranty.
 *	/LICENSE
 *
 ******************************************************************************/

#include "h/skdrv1st.h"
#include "h/skdrv2nd.h"

/* local variables ************************************************************/

/* function prototypes *******************************************************/
static void	SkGmInitPhyMarv(SK_AC*, SK_IOC, int, SK_BOOL);
static int	SkGmAutoNegDoneMarv(SK_AC*, SK_IOC, int);

#ifndef VCPU
/* OptimaEEE PHY AFE workaround */
typedef struct s_PhyHack {
	SK_U16	PhyReg;		/* PHY register */
	SK_U16	PhyVal;		/* Value to write */
} OPT_EEE_HACK;

static OPT_EEE_HACK OptimaEeeAfe[] = {
 { 0x156, 0x58ce },
 { 0x153, 0x99eb },
 { 0x141, 0x8064 },
/* { 0x155, 0x130b },*/
 { 0x000, 0x0000 },
 { 0x151, 0x8433 },
 { 0x14b, 0x8c44 },
 { 0x14c, 0x0f90 },
 { 0x14f, 0x39aa },
/* { 0x154, 0x2f39 },*/
 { 0x14d, 0xba33 },
 { 0x144, 0x0048 },
 { 0x152, 0x2010 },
/* { 0x158, 0x1223 },*/
 { 0x140, 0x4444 },
 { 0x154, 0x2f3b },
 { 0x158, 0xb203 },
 { 0x157, 0x2029 },
};
#endif /* !VCPU */

/******************************************************************************
 *
 *	SkGmPhyRead() - Read from GPHY register
 *
 * Description:	reads a 16-bit word from GPHY through MDIO
 *
 * Returns:
 *	0	o.k.
 *	1	error during MDIO read
 *	2	timeout
 */
int SkGmPhyRead(
SK_AC	*pAC,			/* Adapter Context */
SK_IOC	IoC,			/* I/O Context */
int		Port,			/* Port Index (MAC_1 + n) */
int		PhyReg,			/* Register Address (Offset) */
SK_U16	SK_FAR *pVal)	/* Pointer to Value */
{
	SK_GEPORT	*pPrt;
	SK_U16		Word;
	SK_U16		Ctrl;
	SK_U32		StartTime;
	SK_U32		Delta;
	SK_U32		TimeOut;
	int			Rtv;

	Rtv = 0;

	*pVal = 0xffff;

	pPrt = &pAC->GIni.GP[Port];

	/* additional check for MDC/MDIO activity */
	GM_IN16(IoC, Port, GM_SMI_CTRL, &Ctrl);

	if ((Ctrl & GM_SMI_CT_BUSY) != 0) {

		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
			("SkGmPhyRead, Port: %d, Reg: %2d, Busy (Ctrl=0x%04X)\n",
			 Port, PhyReg, Ctrl));

		return(1);
	}

	/* set PHY-Register offset and 'Read' OpCode (= 1) */
	Word = (SK_U16)(GM_SMI_CT_PHY_AD(pPrt->PhyAddr) |
		GM_SMI_CT_REG_AD(PhyReg) | GM_SMI_CT_OP_RD);

	GM_OUT16(IoC, Port, GM_SMI_CTRL, Word);

	/* additional check for MDC/MDIO activity */
	GM_IN16(IoC, Port, GM_SMI_CTRL, &Ctrl);

	if (Ctrl == 0xffff || (Ctrl & GM_SMI_CT_OP_RD) == 0) {

		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
			("SkGmPhyRead, Port: %d, Reg: %2d, Error (Ctrl=0x%04X)\n",
			 Port, PhyReg, Ctrl));

		return(1);
	}

	Word |= GM_SMI_CT_BUSY;

	/* get current value of timestamp timer */
	StartTime = SkHwGetTimeDelta(pAC, IoC, 0);

	/* set timeout to 10 ms */
	TimeOut = HW_MS_TO_TICKS(pAC, 10);

	do {	/* wait until 'Busy' is cleared and 'ReadValid' is set */
#ifdef VCPU
		VCPUwaitTime(1000);
#endif /* VCPU */

		/* get delta value of timestamp timer */
		Delta = SkHwGetTimeDelta(pAC, IoC, StartTime);

		GM_IN16(IoC, Port, GM_SMI_CTRL, &Ctrl);

		/* Error on reading SMI Control Register */
		if (Ctrl == 0xffff) {
			return(1);
		}

		/* check also if 'Busy'-bit is still set */
		if ((Delta > TimeOut) && (Ctrl & GM_SMI_CT_BUSY) != 0) {

			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("SkGmPhyRead, Port: %d, Reg: %2d, Timeout (Ctrl=0x%04X)\n",
				 Port, PhyReg, Ctrl));

			Rtv = 2;
			break;
		}

	} while ((Ctrl ^ Word) != (GM_SMI_CT_RD_VAL | GM_SMI_CT_BUSY));

	GM_IN16(IoC, Port, GM_SMI_DATA, pVal);

	/* dummy read after GM_IN16() */
	SK_IN32(IoC, B2_TI_VAL, &TimeOut);

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("SkGmPhyRead, Port: %d, Reg: %2d, Val = 0x%04X\n",
		 Port, PhyReg, *pVal));

#if defined(VCPU) || defined(SK_DIAG)
	c_print("Read 0x%04X from PHY reg %u on port %c\n",
		*pVal, PhyReg, 'A' + Port);
#endif /* VCPU || SK_DIAG */

	return(Rtv);
}	/* SkGmPhyRead */


/******************************************************************************
 *
 *	SkGmPhyWrite() - Write to GPHY register
 *
 * Description:	writes a 16-bit word to GPHY through MDIO
 *
 * Returns:
 *	0	o.k.
 *	1	error during MDIO read
 *	2	timeout
 */
int SkGmPhyWrite(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port,		/* Port Index (MAC_1 + n) */
int		PhyReg,		/* Register Address (Offset) */
SK_U16	Val)		/* Value */
{
	SK_GEPORT	*pPrt;
	SK_U16		Ctrl;
	SK_U32		StartTime;
	SK_U32		Delta;
	SK_U32		TimeOut;

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("SkGmPhyWrite, Port: %d, Reg: %2d, Val = 0x%04X\n",
		 Port, PhyReg, Val));

	pPrt = &pAC->GIni.GP[Port];

	/* write the PHY register's value */
	GM_OUT16(IoC, Port, GM_SMI_DATA, Val);

#ifdef DEBUG
	/* additional check for MDC/MDIO activity */
	GM_IN16(IoC, Port, GM_SMI_DATA, &Ctrl);

	if (Ctrl != Val) {

		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
			("PHY write impossible on Port %d (Val=0x%04X)\n", Port, Ctrl));

		return(1);
	}
#endif /* DEBUG */

	/* set PHY-Register offset and 'Write' OpCode (= 0) */
	Ctrl = (SK_U16)(GM_SMI_CT_PHY_AD(pPrt->PhyAddr) |
		GM_SMI_CT_REG_AD(PhyReg));

	GM_OUT16(IoC, Port, GM_SMI_CTRL, Ctrl);

	/* get current value of timestamp timer */
	StartTime = SkHwGetTimeDelta(pAC, IoC, 0);

	/* set timeout to 10 ms */
	TimeOut = HW_MS_TO_TICKS(pAC, 10);

	do {	/* wait until 'Busy' is cleared */
#ifdef VCPU
		VCPUwaitTime(1000);
#endif /* VCPU */

		/* get delta value of timestamp timer */
		Delta = SkHwGetTimeDelta(pAC, IoC, StartTime);

		GM_IN16(IoC, Port, GM_SMI_CTRL, &Ctrl);

		/* Error on reading SMI Control Register */
		if (Ctrl == 0xffff) {
			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("PHY write error on Port %d (Ctrl=0x%04X)\n", Port, Ctrl));
			return(1);
		}

		/* check also if 'Busy'-bit is still set */
		if ((Delta > TimeOut) && (Ctrl & GM_SMI_CT_BUSY) != 0) {

			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("PHY write timeout on Port %d (Ctrl=0x%04X)\n", Port, Ctrl));
			return(2);
		}

	} while ((Ctrl & GM_SMI_CT_BUSY) != 0);

	/* dummy read after GM_IN16() */
	SK_IN32(IoC, B2_TI_VAL, &TimeOut);

#if defined(VCPU) || defined(SK_DIAG)
	c_print("Wrote 0x%04X to PHY reg %u on port %c\n",
		Val, PhyReg, 'A' + Port);
#endif /* VCPU || SK_DIAG */

	return(0);
}	/* SkGmPhyWrite */


#ifndef SK_SLIM
/******************************************************************************
 *
 *	SkGmPtpRead() - Read from PTP (Precise Time Protocol) register
 *
 * Description:	reads a 16-bit word from PTP through AVB Command Register
 *
 * Returns:
 *	0	o.k.
 *	1	error during AVB read
 */
int SkGmPtpRead(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port,		/* AVB Port Index */
int		PtpReg,		/* Register Address (Offset) */
SK_U16	*pVal)		/* Pointer to Value */
{
	SK_U16	Ctrl;
	int		Rtv;

	Rtv = 0;

	*pVal = 0xffff;

	/* additional check for AVB activity */
	SK_IN16(IoC, PTP_AVB_COMMAND, &Ctrl);

	if ((Ctrl & PTP_AVB_COMMAND_AVBBUSY) != 0) {

		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
			("SkGmPtpRead, Reg: %2d, Busy (Ctrl=0x%04X)\n",
			 PtpReg, Ctrl));

		return(1);
	}

	Ctrl = (SK_U16)(PTP_AVB_COMMAND_AVBBUSY |
		(AVB_READ << PTP_AVB_COMMAND_AVBOP_BASE) |
		(Port << PTP_AVB_COMMAND_AVBPORT_BASE) |
		(PtpReg & PTP_AVB_COMMAND_AVBADDR_MSK));

	SK_OUT16(IoC, PTP_AVB_COMMAND, Ctrl);

	do {
		SK_IN16(IoC, PTP_AVB_COMMAND, &Ctrl);

		/* additional check for AVB activity */
		if (Ctrl == 0xffff) {

			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("SkGmPtpRead, Reg: %2d, Error (Ctrl=0x%04X)\n",
				 PtpReg, Ctrl));

			return(1);
		}

	} while ((Ctrl & PTP_AVB_COMMAND_AVBBUSY) != 0);

	SK_IN16(IoC, PTP_AVB_DATA, pVal);

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("SkGmPtpRead, Reg: %2d, Val = 0x%04X\n",
		 PtpReg, *pVal));

#if defined(VCPU) || defined(SK_DIAG)
	c_print("Read 0x%04X from PTP reg %u, Port 0x%X\n",
		*pVal, PtpReg, Port);
#endif /* VCPU || SK_DIAG */

	return(Rtv);
}	/* SkGmPtpRead */


/******************************************************************************
 *
 *	SkGmPtpReadDbl() - Read from PTP (Precise Time Protocol) register
 *
 * Description:	reads 2x 16-bit word from PTP through AVB Command Register
 *
 * Returns:
 *	0	o.k.
 *	1	error during AVB read
 */
int SkGmPtpReadDbl(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port,		/* AVB Port Index */
int		PtpReg,		/* Register Address (Offset) */
SK_U32	*pVal)		/* Pointer to Value */
{
	SK_U16	Ctrl;
	SK_U16	Word;
	int		Rtv;

	Rtv = 0;

	*pVal = 0xffff;

	/* additional check for AVB activity */
	SK_IN16(IoC, PTP_AVB_COMMAND, &Ctrl);

	if ((Ctrl & PTP_AVB_COMMAND_AVBBUSY) != 0) {

		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
			("SkGmPtpReadDbl, Reg: %2d, Busy (Ctrl=0x%04X)\n",
			 PtpReg, Ctrl));

		return(1);
	}

	Ctrl = (SK_U16)(PTP_AVB_COMMAND_AVBBUSY |
		(AVB_READ_INC << PTP_AVB_COMMAND_AVBOP_BASE) |
		(Port << PTP_AVB_COMMAND_AVBPORT_BASE) |
		(PtpReg & PTP_AVB_COMMAND_AVBADDR_MSK));

	SK_OUT16(IoC, PTP_AVB_COMMAND, Ctrl);

	do {
		SK_IN16(IoC, PTP_AVB_COMMAND, &Ctrl);

		/* additional check for AVB activity */
		if (Ctrl == 0xffff) {

			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("SkGmPtpReadDbl, Reg: %2d, Error (Ctrl=0x%04X)\n",
				 PtpReg, Ctrl));

			return(1);
		}

	} while ((Ctrl & PTP_AVB_COMMAND_AVBBUSY) != 0);

	SK_IN16(IoC, PTP_AVB_DATA, &Word);

	*pVal = Word;

	do {
		SK_IN16(IoC, PTP_AVB_COMMAND, &Ctrl);

		/* additional check for AVB activity */
		if (Ctrl == 0xffff) {

			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("SkGmPtpReadDbl, Reg: %2d, Error (Ctrl=0x%04X)\n",
				 PtpReg, Ctrl));

			return(1);
		}

	} while ((Ctrl & PTP_AVB_COMMAND_AVBBUSY) != 0);

	SK_IN16(IoC, PTP_AVB_DATA, &Word);

	*pVal |= (SK_U32)Word << 16;

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("SkGmPtpReadDbl, Reg: %2d, Val = 0x%08lX\n",
		 PtpReg, *pVal));

#if defined(VCPU) || defined(SK_DIAG)
	c_print("Read 0x%08lX from PTP reg %u, Port 0x%X\n",
		*pVal, PtpReg, Port);
#endif /* VCPU || SK_DIAG */

	return(Rtv);
}	/* SkGmPtpReadDbl */


/******************************************************************************
 *
 *	SkGmPtpWrite() - Write to PTP (Precise Time Protocol) register
 *
 * Description:	writes a 16-bit word to PTP through AVB Command Register
 *
 * Returns:
 *	0	o.k.
 *	1	error during AVB read
 */
int SkGmPtpWrite(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port,		/* AVB Port Index */
int		PtpReg,		/* Register Address (Offset) */
SK_U16	Val)		/* Value */
{
	SK_U16	Ctrl;

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("SkGmPtpWrite, Reg: %2d, Val = 0x%04X\n",
		 PtpReg, Val));

	/* additional check for AVB activity */
	SK_IN16(IoC, PTP_AVB_COMMAND, &Ctrl);

	if ((Ctrl & PTP_AVB_COMMAND_AVBBUSY) != 0) {

		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
			("SkGmPtpWrite, Reg: %2d, Busy (Ctrl=0x%04X)\n",
			 PtpReg, Ctrl));

		return(1);
	}

	SK_OUT16(IoC, PTP_AVB_DATA, Val);

	Ctrl = (SK_U16)(PTP_AVB_COMMAND_AVBBUSY |
		(AVB_WRITE << PTP_AVB_COMMAND_AVBOP_BASE) |
		(Port << PTP_AVB_COMMAND_AVBPORT_BASE) |
		(PtpReg & PTP_AVB_COMMAND_AVBADDR_MSK));

	SK_OUT16(IoC, PTP_AVB_COMMAND, Ctrl);

	do {
		SK_IN16(IoC, PTP_AVB_COMMAND, &Ctrl);

		/* additional check for AVB activity */
		if (Ctrl == 0xffff) {

			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("SkGmPtpWrite, Reg: %2d, Error (Ctrl=0x%04X)\n",
				 PtpReg, Ctrl));

			return(1);
		}

	} while ((Ctrl & PTP_AVB_COMMAND_AVBBUSY) != 0);

#if defined(VCPU) || defined(SK_DIAG)
	c_print("Wrote 0x%04X to PTP reg %u, Port 0x%X\n",
		Val, PtpReg, Port);
#endif /* VCPU || SK_DIAG */

	return(0);
}	/* SkGmPtpWrite */
#endif /* !SK_SLIM */


#ifdef SK_DIAG
/******************************************************************************
 *
 *	SkGePhyRead() - Read from PHY register
 *
 * Description:	calls a read PHY routine dep. on adapter type
 *
 * Returns:
 *	nothing
 */
void SkGePhyRead(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port,		/* Port Index (MAC_1 + n) */
int		PhyReg,		/* Register Address (Offset) */
SK_U16	*pVal)		/* Pointer to Value */
{
	pAC->GIni.GIFunc.pFnMacPhyRead(pAC, IoC, Port, PhyReg, pVal);
}	/* SkGePhyRead */


/******************************************************************************
 *
 *	SkGePhyWrite() - Write to PHY register
 *
 * Description:	calls a write PHY routine dep. on adapter type
 *
 * Returns:
 *	nothing
 */
void SkGePhyWrite(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port,		/* Port Index (MAC_1 + n) */
int		PhyReg,		/* Register Address (Offset) */
SK_U16	Val)		/* Value */
{
	pAC->GIni.GIFunc.pFnMacPhyWrite(pAC, IoC, Port, PhyReg, Val);
}	/* SkGePhyWrite */
#endif /* SK_DIAG */


/******************************************************************************
 *
 *	SkMacPromiscMode() - Enable / Disable Promiscuous Mode
 *
 * Description:
 *	enables / disables promiscuous mode by setting Receive Control Register
 *
 * Returns:
 *	nothing
 */
void SkMacPromiscMode(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC,	/* I/O Context */
int		Port,	/* Port Index (MAC_1 + n) */
SK_BOOL	Enable)	/* Enable / Disable */
{
	SK_U16	RcReg;

	GM_IN16(IoC, Port, GM_RX_CTRL, &RcReg);

	/* enable or disable unicast and multicast filtering */
	if (Enable) {
		RcReg &= ~(GM_RXCR_UCF_ENA | GM_RXCR_MCF_ENA);
	}
	else {
		RcReg |= (GM_RXCR_UCF_ENA | GM_RXCR_MCF_ENA);
	}
	/* setup Receive Control Register */
	GM_OUT16(IoC, Port, GM_RX_CTRL, RcReg);

}	/* SkMacPromiscMode*/


/******************************************************************************
 *
 *	SkMacHashing() - Enable / Disable Hashing
 *
 * Description:
 *	enables / disables hashing by setting Receive Control Register
 *
 * Returns:
 *	nothing
 */
void SkMacHashing(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC,	/* I/O Context */
int		Port,	/* Port Index (MAC_1 + n) */
SK_BOOL	Enable)	/* Enable / Disable */
{
	SK_U16	RcReg;

	GM_IN16(IoC, Port, GM_RX_CTRL, &RcReg);

	/* enable or disable multicast filtering */
	if (Enable) {
		RcReg |= GM_RXCR_MCF_ENA;
	}
	else {
		RcReg &= ~GM_RXCR_MCF_ENA;
	}
	/* setup Receive Control Register */
	GM_OUT16(IoC, Port, GM_RX_CTRL, RcReg);

}	/* SkMacHashing*/


#ifdef SK_DIAG
/******************************************************************************
 *
 *	SkGmSetRxCmd() - Modify the value of the GMAC's Rx Control Register
 *
 * Description:
 *	The features
 *	 - FCS (CRC) stripping,				SK_STRIP_FCS_ON/OFF
 *	 - don't set GMR_FS_LONG_ERR		SK_BIG_PK_OK_ON/OFF
 *		for frames > 1514 bytes
 *	- enable Rx of own packets			SK_SELF_RX_ON/OFF
 *
 *	for incoming packets may be enabled/disabled by this function.
 *	Additional modes may be added later.
 *	Multiple modes can be enabled/disabled at the same time.
 *	The new configuration is written to the Rx Command register immediately.
 *
 * Returns:
 *	nothing
 */
static void SkGmSetRxCmd(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port,		/* Port Index (MAC_1 + n) */
int		Mode)		/* Mode is SK_STRIP_FCS_ON/OFF, SK_STRIP_PAD_ON/OFF,
						SK_LENERR_OK_ON/OFF, or SK_BIG_PK_OK_ON/OFF */
{
	SK_U16	RxCmd;

	if ((Mode & (SK_STRIP_FCS_ON | SK_STRIP_FCS_OFF)) != 0) {

		GM_IN16(IoC, Port, GM_RX_CTRL, &RxCmd);

		if ((Mode & SK_STRIP_FCS_ON) != 0) {
			RxCmd |= GM_RXCR_CRC_DIS;
		}
		else {
			RxCmd &= ~GM_RXCR_CRC_DIS;
		}
		/* Write the new mode to the Rx Control register */
		GM_OUT16(IoC, Port, GM_RX_CTRL, RxCmd);
	}

	if ((Mode & (SK_BIG_PK_OK_ON | SK_BIG_PK_OK_OFF)) != 0) {

		GM_IN16(IoC, Port, GM_SERIAL_MODE, &RxCmd);

		if ((Mode & SK_BIG_PK_OK_ON) != 0) {
			RxCmd |= GM_SMOD_JUMBO_ENA;
		}
		else {
			RxCmd &= ~GM_SMOD_JUMBO_ENA;
		}
		/* Write the new mode to the Serial Mode register */
		GM_OUT16(IoC, Port, GM_SERIAL_MODE, RxCmd);
	}
}	/* SkGmSetRxCmd */


/******************************************************************************
 *
 *	SkMacSetRxCmd() - Modify the value of the MAC's Rx Control Register
 *
 * Description:	modifies the MAC's Rx Control register
 *
 * Returns:
 *	nothing
 */
void SkMacSetRxCmd(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port,		/* Port Index (MAC_1 + n) */
int		Mode)		/* Rx Mode */
{
	SkGmSetRxCmd(pAC, IoC, Port, Mode);

}	/* SkMacSetRxCmd */


/******************************************************************************
 *
 *	SkMacCrcGener() - Enable / Disable CRC Generation
 *
 * Description:	enables / disables CRC generation
 *
 * Returns:
 *	nothing
 */
void SkMacCrcGener(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC,	/* I/O Context */
int		Port,	/* Port Index (MAC_1 + n) */
SK_BOOL	Enable)	/* Enable / Disable */
{
	SK_U16	Word;

	GM_IN16(IoC, Port, GM_TX_CTRL, &Word);

	if (Enable) {
		Word &= ~GM_TXCR_CRC_DIS;
	}
	else {
		Word |= GM_TXCR_CRC_DIS;
	}
	/* setup Tx Control Register */
	GM_OUT16(IoC, Port, GM_TX_CTRL, Word);

}	/* SkMacCrcGener*/

#endif /* SK_DIAG */


/******************************************************************************
 *
 *	SkGmSoftRst() - Do a GMAC software reset
 *
 * Description:
 *	The PHY registers should not be affected during this kind of software reset.
 *
 * Returns:
 *	nothing
 */
static void SkGmSoftRst(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC,	/* I/O Context */
int		Port)	/* Port Index (MAC_1 + n) */
{
	SK_U16	EmptyHash[4] = { 0x0000, 0x0000, 0x0000, 0x0000 };
	SK_U16	RxCtrl;

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("SkGmSoftRst: Port %d\n", Port));

	/* reset the statistics module */
	(void)SkGmResetCounter(pAC, IoC, Port);

	/* disable all GMAC IRQs */
	SK_OUT8(IoC, MR_ADDR(Port, GMAC_IRQ_MSK), 0);

	if (pAC->GIni.GP[Port].PState != SK_PRT_RESET) {
		/* disable all PHY IRQs */
		SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_INT_MASK, 0);
	}

	/* clear the Hash Register */
	GM_OUTHASH(IoC, Port, GM_MC_ADDR_H1, EmptyHash);

	/* enable Unicast and Multicast filtering */
	GM_IN16(IoC, Port, GM_RX_CTRL, &RxCtrl);

	GM_OUT16(IoC, Port, GM_RX_CTRL, RxCtrl | GM_RXCR_UCF_ENA | GM_RXCR_MCF_ENA);

}	/* SkGmSoftRst */


/******************************************************************************
 *
 *	SkGmHardRst() - Do a GMAC hardware reset
 *
 * Description:
 *
 * Returns:
 *	nothing
 */
static void SkGmHardRst(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC,	/* I/O Context */
int		Port)	/* Port Index (MAC_1 + n) */
{
	SK_U32	DWord;
#if defined(QUICK_LINK) || defined(SK_DIAG)
	SK_U8	Byte;
#endif /* QUICK_LINK || SK_DIAG */

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("SkGmHardRst: Port %d\n", Port));

#if defined(VCPU) || defined(SK_DIAG)
	c_print("\nSkGmHardRst: Port %c\n", 'A' + Port);
#endif /* VCPU || SK_DIAG */

	/* WA code for COMA mode */
	if (pAC->GIni.GIYukonLite &&
		pAC->GIni.GIChipRev >= CHIP_REV_YU_LITE_A3) {

		SK_IN32(IoC, B2_GP_IO, &DWord);

		DWord |= (GP_DIR_9 | GP_IO_9);

		/* set PHY reset */
		SK_OUT32(IoC, B2_GP_IO, DWord);
	}
#if defined(QUICK_LINK) || defined(SK_DIAG)
	else if (pAC->GIni.GIChipId >= CHIP_ID_YUKON_OPT &&
			 pAC->GIni.GIChipId <= CHIP_ID_YUKON_OP_2) {

		SK_IN8(IoC, PCI_C2(pAC, IoC, PSM_CONFIG_REG4), &Byte);

		Byte &= ~(SK_U8)PSM_CONFIG_REG4_RST_PHY_LINK_DETECT;

		/* enable Config Write */
		SK_TST_MODE_ON(IoC);

		/* release reset for PHY Quick Link Detect */
		SK_OUT8(IoC, PCI_C2(pAC, IoC, PSM_CONFIG_REG4), Byte);

		/* disable Config Write */
		SK_TST_MODE_OFF(IoC);
	}
#endif /* QUICK_LINK || SK_DIAG */

	/* set GPHY Control reset */
	SK_OUT8(IoC, MR_ADDR(Port, GPHY_CTRL), (SK_U8)GPC_RST_SET);

	if (!pAC->GIni.GIAsfEnabled) {
		/* set GMAC Control reset */
		SK_OUT8(IoC, MR_ADDR(Port, GMAC_CTRL), (SK_U8)GMC_RST_SET);
	}

}	/* SkGmHardRst */


/******************************************************************************
 *
 *	SkGmClearRst() - Release the GPHY & GMAC reset
 *
 * Description:
 *
 * Returns:
 *	nothing
 */
static void SkGmClearRst(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC,	/* I/O Context */
int		Port)	/* Port Index (MAC_1 + n) */
{
	SK_U32	DWord;
	SK_U16	PhyId0;
	SK_U16	PhyId1;
	SK_U16	Word;
	SK_U8	Byte;
	int		ChipId;
	int		i;

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("SkGmClearRst: Port %d\n", Port));

	ChipId = pAC->GIni.GIChipId;

#ifndef DISABLE_YUKON_I
	/* WA code for COMA mode */
	if (pAC->GIni.GIYukonLite &&
		pAC->GIni.GIChipRev >= CHIP_REV_YU_LITE_A3) {

		SK_IN32(IoC, B2_GP_IO, &DWord);

		DWord |= GP_DIR_9;		/* set to output */
		DWord &= ~GP_IO_9;		/* clear PHY reset (active high) */

		/* clear PHY reset */
		SK_OUT32(IoC, B2_GP_IO, DWord);
	}
#endif /* !DISABLE_YUKON_I */
#ifdef VCPU
	/* set MAC Reset before PHY reset is set */
	SK_OUT8(IoC, MR_ADDR(Port, GMAC_CTRL), (SK_U8)GMC_RST_SET);
#endif /* VCPU */

	if (CHIP_ID_YUKON_2(pAC)) {

		Byte = (ChipId == CHIP_ID_YUKON_FE_P) ? (SK_U8)BIT_7S : 0;

		/* set GPHY Control reset */
		SK_OUT8(IoC, MR_ADDR(Port, GPHY_CTRL), Byte | (SK_U8)GPC_RST_SET);

		/* release GPHY Control reset */
		SK_OUT8(IoC, MR_ADDR(Port, GPHY_CTRL), Byte | (SK_U8)GPC_RST_CLR);

#ifdef DEBUG
		/* additional check for PEX */
		SK_IN8(IoC, MR_ADDR(Port, GPHY_CTRL), &Byte);

		Byte &= GPC_RST_CLR | GPC_RST_SET;

		if (pAC->GIni.GIPciBus == SK_PEX_BUS && Byte != GPC_RST_CLR) {

			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_ERR,
				("Error on PEX-bus after GPHY reset (GPHY Ctrl=0x%02X)\n",
				Byte));
		}
#endif /* DEBUG */
	}
#ifndef DISABLE_YUKON_I
	else {
		/* set HWCFG_MODE */
		DWord = GPC_INT_POL | GPC_DIS_FC | GPC_DIS_SLEEP |
			GPC_ENA_XC | GPC_ANEG_ADV_ALL_M | GPC_ENA_PAUSE |
			(pAC->GIni.GICopperType ? GPC_HWCFG_GMII_COP :
			GPC_HWCFG_GMII_FIB);

		/* set GPHY Control reset */
		SK_OUT32(IoC, MR_ADDR(Port, GPHY_CTRL), DWord | GPC_RST_SET);

		/* release GPHY Control reset */
		SK_OUT32(IoC, MR_ADDR(Port, GPHY_CTRL), DWord | GPC_RST_CLR);
	}
#endif /* !DISABLE_YUKON_I */

	/* dummy read after PHY reset */
	SK_IN32(IoC, MR_ADDR(Port, GPHY_CTRL), &DWord);

#if defined(VCPU) && !defined(HASE)
	/* wait for internal initialization of GPHY */
	VCPUprintf(0, "Waiting until PHY %d is ready to initialize\n", Port);
	VCpuWait(10000);
#endif /* VCPU */

	if (ChipId >= CHIP_ID_YUKON_OPT && pAC->GIni.GP[Port].PRevAutoNeg) {

		SK_IN32(IoC, MR_ADDR(Port, GMAC_CTRL), &DWord);

		DWord &= ~GMC_RST_SET;

		/* enable PHY Reverse Auto-Negotiation */
		DWord |= MAC_CTRL_PD_REV_ANEG_A;
	}
	else {
		DWord = 0;
	}

	/* clear GMAC Control reset */
	SK_OUT32(IoC, MR_ADDR(Port, GMAC_CTRL), DWord | GMC_RST_CLR);

#if defined(VCPU) && !defined(HASE)
	/* wait for stable GMAC clock */
	VCpuWait(9000);
#endif /* VCPU */

#ifdef SK_DIAG
	if (HW_FEATURE(pAC, HWF_WA_DEV_472) && Port == MAC_2) {

		/* clear GMAC 1 Control reset */
		SK_OUT8(IoC, MR_ADDR(MAC_1, GMAC_CTRL), (SK_U8)GMC_RST_CLR);

		do {
			/* set GMAC 2 Control reset */
			SK_OUT8(IoC, MR_ADDR(MAC_2, GMAC_CTRL), (SK_U8)GMC_RST_SET);

			/* clear GMAC 2 Control reset */
			SK_OUT8(IoC, MR_ADDR(MAC_2, GMAC_CTRL), (SK_U8)GMC_RST_CLR);

			SkGmPhyRead(pAC, IoC, MAC_2, PHY_MARV_ID0, &PhyId0);

			SkGmPhyRead(pAC, IoC, MAC_2, PHY_MARV_ID1, &PhyId1);

			SkGmPhyRead(pAC, IoC, MAC_2, PHY_MARV_INT_MASK, &Word);

		} while (Word != 0 || PhyId0 != PHY_MARV_ID0_VAL ||
				 PhyId1 != PHY_MARV_ID1_Y2);
	}
#endif /* SK_DIAG */

	if (ChipId == CHIP_ID_YUKON_FE_P) {

		for (i = 0; i < 9; ++i) {
			/* dummy read after PHY reset */
			SkGmPhyRead(pAC, IoC, Port, PHY_MARV_ID1, &Word);

			/* check VCT registers after PHY reset */
			SkGmPhyRead(pAC, IoC, Port, PHY_MARV_FE_VCT_TX, &PhyId0);

			SkGmPhyRead(pAC, IoC, Port, PHY_MARV_FE_VCT_RX, &PhyId1);

			if (Word == PHY_MARV_ID1_FEP && PhyId0 == 0 && PhyId1 == 0) {

#ifdef xDEBUG
				SK_TST_MODE_ON(IoC);
				SK_OUT8(IoC, B2_MAC_3, (SK_U8)i);
				SK_TST_MODE_OFF(IoC);
#endif
				break;
			}

			/* set GPHY Control reset */
			SK_OUT8(IoC, GPHY_CTRL, (SK_U8)(BIT_7S | GPC_RST_SET));

			/* dummy read after PHY reset */
			SK_IN32(IoC, GPHY_CTRL, &DWord);

			/* release GPHY Control reset */
			SK_OUT8(IoC, GPHY_CTRL, (SK_U8)(BIT_7S | GPC_RST_CLR));

			/* dummy read */
			SK_IN32(IoC, GPHY_CTRL, &DWord);
		}
	}

#if defined(VCPU) && !defined(HASE)
	VCpuWait(2000);

	SK_IN32(IoC, B0_ISRC, &DWord);
#endif /* VCPU */

}	/* SkGmClearRst */


/******************************************************************************
 *
 *	SkMacSoftRst() - Do a MAC software reset
 *
 * Description:	calls a MAC software reset routine
 *
 * Returns:
 *	nothing
 */
void SkMacSoftRst(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC,	/* I/O Context */
int		Port)	/* Port Index (MAC_1 + n) */
{
	/* disable receiver and transmitter */
	SkMacRxTxDisable(pAC, IoC, Port);

	SkGmSoftRst(pAC, IoC, Port);

	pAC->GIni.GP[Port].PState = SK_PRT_STOP;

}	/* SkMacSoftRst */


/******************************************************************************
 *
 *	SkMacHardRst() - Do a MAC hardware reset
 *
 * Description:	calls a MAC hardware reset routine
 *
 * Returns:
 *	nothing
 */
void SkMacHardRst(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC,	/* I/O Context */
int		Port)	/* Port Index (MAC_1 + n) */
{
	SkGmHardRst(pAC, IoC, Port);

	pAC->GIni.GP[Port].PHWLinkUp = SK_FALSE;
	pAC->GIni.GP[Port].PPhyQLink = SK_FALSE;

	pAC->GIni.GP[Port].PState = SK_PRT_RESET;

}	/* SkMacHardRst */

#ifndef SK_SLIM
/******************************************************************************
 *
 *	SkMacClearRst() - Clear the MAC reset
 *
 * Description:	calls a clear MAC reset routine
 *
 * Returns:
 *	nothing
 */
void SkMacClearRst(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC,	/* I/O Context */
int		Port)	/* Port Index (MAC_1 + n) */
{
	SkGmClearRst(pAC, IoC, Port);

}	/* SkMacClearRst */
#endif /* !SK_SLIM */

/******************************************************************************
 *
 *	SkGmInitMac() - Initialize the GMAC
 *
 * Description:
 *	Initialize the GMAC of the specified port.
 *	The GMAC must be reset or stopped before calling this function.
 *
 * Note:
 *	The GMAC's Rx and Tx state machine is still disabled when returning.
 *
 * Returns:
 *	nothing
 */
void SkGmInitMac(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port)		/* Port Index (MAC_1 + n) */
{
	SK_GEPORT	*pPrt;
	int			i;
	SK_U16		SWord;
#ifndef VCPU
	SK_BOOL		MacResetFlag;
#endif /* !VCPU */

	pPrt = &pAC->GIni.GP[Port];

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("SkGmInitMac: Port %d, PState = %d, PLinkSpeed = %d\n",
		 Port, pPrt->PState, pPrt->PLinkSpeed));

#ifndef VCPU
	/* check GMAC reset */
	SK_IN16(IoC, MR_ADDR(Port, GMAC_CTRL), &SWord);

	MacResetFlag = (SWord & GMC_RST_SET) != 0;

	if (pPrt->PState == SK_PRT_STOP) {
		/* verify that the reset bit is cleared */
		if (MacResetFlag) {
			/* PState does not match HW state */
			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("SkGmInitMac: PState does not match HW state"));
			/* Correct it */
			pPrt->PState = SK_PRT_RESET;
		}
		else {
			/* enable PHY interrupts */
			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_INT_MASK,
				(SK_U16)PHY_M_DEF_MSK);
		}
	}
#endif /* !VCPU */

	if (pPrt->PState == SK_PRT_RESET) {

		SkGmHardRst(pAC, IoC, Port);

		SkGmClearRst(pAC, IoC, Port);

#ifndef SK_SLIM
		if (HW_FEATURE(pAC, HWF_FORCE_AUTO_NEG) &&
			pPrt->PLinkModeConf < SK_LMODE_AUTOHALF) {
			/* Force Auto-Negotiation */
			pPrt->PLinkMode = (pPrt->PLinkModeConf == SK_LMODE_FULL) ?
				SK_LMODE_AUTOBOTH : SK_LMODE_AUTOHALF;

			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("SkGmInitMac: Force Auto-Negotiation, PLinkMode = %d",
				pPrt->PLinkMode));
		}
#endif /* !SK_SLIM */

		SWord = 0;

		/* speed settings */
		switch (pPrt->PLinkSpeed) {
		case SK_LSPEED_AUTO:
		case SK_LSPEED_1000MBPS:
			/* check for Gigabit adapters */
			if ((pPrt->PLinkSpeedCap & SK_LSPEED_CAP_1000MBPS) != 0) {

				SWord |= GM_GPCR_SPEED_1000;
			}
			break;
		case SK_LSPEED_100MBPS:
			SWord |= GM_GPCR_SPEED_100;
			break;
		case SK_LSPEED_10MBPS:
			break;
		}

		/* duplex settings */
		if (pPrt->PLinkMode != SK_LMODE_HALF) {
			/* set full duplex */
			SWord |= GM_GPCR_DUP_FULL;
		}

		/* flow-control settings */
		switch (pPrt->PFlowCtrlMode) {
		case SK_FLOW_MODE_NONE:
			/* disable Tx & Rx flow-control */
			SWord |= GM_GPCR_FC_TX_DIS | GM_GPCR_FC_RX_DIS | GM_GPCR_AU_FCT_DIS;
			break;
		case SK_FLOW_MODE_LOC_SEND:
			/* disable Rx flow-control */
			SWord |= GM_GPCR_FC_RX_DIS | GM_GPCR_AU_FCT_DIS;
			break;
		case SK_FLOW_MODE_SYMMETRIC:
		case SK_FLOW_MODE_SYM_OR_REM:
			/* enable Tx & Rx flow-control */
			break;
		}

		/* Auto-negotiation disabled */
		if (pPrt->PLinkMode == SK_LMODE_HALF ||
			pPrt->PLinkMode == SK_LMODE_FULL) {

			/* disable auto-update for speed, duplex and flow-control */
			SWord |= GM_GPCR_AU_ALL_DIS;
		}

		if (HW_FEATURE(pAC, HWF_PHY_SET_SLAVE_MDIX)) {
			/* disable auto-update, force 1000Mbps full duplex */
			SWord = GM_GPCR_AU_ALL_DIS | GM_GPCR_SPEED_1000 | GM_GPCR_FL_PASS |
				GM_GPCR_DUP_FULL | GM_GPCR_FC_RX_DIS | GM_GPCR_FC_TX_DIS;
		}

		/* setup General Purpose Control Register */
		GM_OUT16(IoC, Port, GM_GP_CTRL, SWord);

		/* dummy read the Interrupt Source Register */
		SK_IN16(IoC, MR_ADDR(Port, GMAC_IRQ_SRC), &SWord);

#ifndef VCPU
		SkGmInitPhyMarv(pAC, IoC, Port, SK_FALSE);
#endif /* !VCPU */
	}
	else if (pAC->GIni.GIChipId == CHIP_ID_YUKON_EX ||
			 pAC->GIni.GIChipId >= CHIP_ID_YUKON_SUPR) {

		/* Read PHY Copper Specific Control 2 */
		SkGmPhyRead(pAC, IoC, Port, PHY_MARV_EXT_CTRL_2, &SWord);

		if ((SWord & BIT_0S) != 0) {
			/* Enable PHY Transmitter (clear bit 26_0.0) */
			SWord &= ~BIT_0S;

			/* Write PHY changes (SW-reset must follow) */
			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_CTRL_2, SWord);
		}
	}

	if (HW_SUP_EEE(pAC)) {
		/*
		 * Using SK_IN16 and SK_OUT16. These I/Os are only executed on chips
		 * that allow GMAC access without additional safety measures.
		 */
		if (!HW_FEATURE(pAC, HWF_DO_EEE_ENABLE)) {
			/* disable LPI signaling */
			SK_IN16(IoC, GMA(Port, EEE_CONTROL), &SWord);

			SK_OUT16(IoC, GMA(Port, EEE_CONTROL),
				(SWord & ~EEE_CONTROL_CAPABLE) | EEE_CONTROL_RESET);

			SK_IN16(IoC, GMA(Port, EEE_CONTROL), &SWord);

			SK_OUT16(IoC, GMA(Port, EEE_CONTROL),
				SWord & ~(EEE_CONTROL_CAPABLE | EEE_CONTROL_RESET));
		}
	}

#ifndef VCPU	/* saves some time in co-sim */
	if (MacResetFlag) {
		(void)SkGmResetCounter(pAC, IoC, Port);
	}
#else	/* VCPU */
	if (pAC->GIni.GIChipId >= CHIP_ID_YUKON_PRM) {
		/*
		 * Using SK_IN16 and SK_OUT16. These I/Os are only executed on chips
		 * that allow GMAC access without additional safety measures.
		 */
		SK_IN16(IoC, GMA(Port, EEE_CONTROL), &SWord);
		SK_OUT16(IoC, GMA(Port, EEE_CONTROL), SWord | EEE_CONTROL_DEBUG_LPI);
	}
#endif	/* VCPU */

	/* setup Transmit Control Register */
#ifndef SK_SLIM
	GM_OUT16(IoC, Port, GM_TX_CTRL, (SK_U16)TX_COL_THR(pPrt->PMacColThres));
#else
	GM_OUT16(IoC, Port, GM_TX_CTRL, (SK_U16)TX_COL_THR(TX_COL_DEF));
#endif
	/* setup Receive Control Register */
	SWord = GM_RXCR_UCF_ENA | GM_RXCR_MCF_ENA | GM_RXCR_CRC_DIS;

	GM_OUT16(IoC, Port, GM_RX_CTRL, SWord);

	/* setup Transmit Flow-Control Register */
	GM_OUT16(IoC, Port, GM_TX_FLOW_CTRL, 0xffff);

	/* setup Transmit Parameter Register */
#ifdef VCPU
	GM_IN16(IoC, Port, GM_TX_PARAM, &SWord);
#endif /* VCPU */

#ifndef SK_SLIM
	SWord = (SK_U16)(TX_JAM_LEN_VAL(pPrt->PMacJamLen) |
		TX_JAM_IPG_VAL(pPrt->PMacJamIpgVal) |
		TX_IPG_JAM_DATA(pPrt->PMacJamIpgData) |
		TX_BACK_OFF_LIM(pPrt->PMacBackOffLim));
#else
	SWord = (SK_U16)(TX_JAM_LEN_VAL(TX_JAM_LEN_DEF) |
		TX_JAM_IPG_VAL(TX_JAM_IPG_DEF) |
		TX_IPG_JAM_DATA(TX_IPG_JAM_DEF) |
		TX_BACK_OFF_LIM(TX_BOF_LIM_DEF));
#endif

	GM_OUT16(IoC, Port, GM_TX_PARAM, SWord);

	/* configure the Serial Mode Register */
#ifndef SK_SLIM
	SWord = (SK_U16)(DATA_BLIND_VAL(pPrt->PMacDataBlind) |
		GM_SMOD_VLAN_ENA | IPG_DATA_VAL(pPrt->PMacIpgData_1G));

	if (pPrt->PMacLimit4) {
		/* reset of collision counter after 4 consecutive collisions */
		SWord |= GM_SMOD_LIMIT_4;
	}
#else
	SWord = (SK_U16)(DATA_BLIND_VAL(DATA_BLIND_DEF) |
		GM_SMOD_VLAN_ENA | IPG_DATA_VAL(IPG_DATA_DEF_1000));
#endif

	if (pPrt->PPortUsage == SK_JUMBO_LINK) {
		/* enable jumbo mode (Max. Frame Length = 9018) */
		SWord |= GM_SMOD_JUMBO_ENA;
	}

	if (HW_FEATURE(pAC, HWF_NEW_FLOW_CONTROL)) {
		/* enable new Flow-Control */
		SWord |= GM_NEW_FLOW_CTRL;
	}

	GM_OUT16(IoC, Port, GM_SERIAL_MODE, SWord);

	if (pAC->GIni.GIChipId >= CHIP_ID_YUKON_SUPR) {
		/* adjust Flow-Control Timeout for Gigabit clock (125 MHz) */
		GM_OUT16(IoC, Port, GM_FC_TIMEOUT, 31250);
	}

	/*
	 * configure the GMACs Station Addresses
	 * in PROM you can find our addresses at:
	 * B2_MAC_1 = xx xx xx xx xx x0 virtual address
	 * B2_MAC_2 = xx xx xx xx xx x1 is programmed to GMAC A
	 * B2_MAC_3 = xx xx xx xx xx x2 is reserved for DualPort
	 */

	for (i = 0; i < 3; i++) {
		/*
		 * The following 2 statements are together endianess independent.
		 */
		/* physical address: will be used for pause frames */
		SK_IN16(IoC, B2_MAC_2 + Port * 8 + i * 2, &SWord);

#ifdef WA_DEV_16
		/* WA for deviation #16 */
		if (pAC->GIni.GIChipId == CHIP_ID_YUKON && pAC->GIni.GIChipRev == 0) {
			/* swap the address bytes */
			SWord = ((SWord & 0xff00) >> 8)	| ((SWord & 0x00ff) << 8);

			/* write to register in reversed order */
			GM_OUT16(IoC, Port, GM_SRC_ADDR_1L + (2 - i) * 4, SWord);
		}
		else {
			GM_OUT16(IoC, Port, GM_SRC_ADDR_1L + i * 4, SWord);
		}
#else
		GM_OUT16(IoC, Port, GM_SRC_ADDR_1L + i * 4, SWord);
#endif /* WA_DEV_16 */

		/* virtual address: will be used for data */
		SK_IN16(IoC, B2_MAC_1 + Port * 8 + i * 2, &SWord);

		GM_OUT16(IoC, Port, GM_SRC_ADDR_2L + i * 4, SWord);

		/* reset Multicast filtering Hash registers 1-3 */
		GM_OUT16(IoC, Port, GM_MC_ADDR_H1 + i * 4, 0);
	}

	/* reset Multicast filtering Hash register 4 */
	GM_OUT16(IoC, Port, GM_MC_ADDR_H4, 0);

	/* enable interrupt mask for counter overflows */
	GM_OUT16(IoC, Port, GM_TX_IRQ_MSK, 0);
	GM_OUT16(IoC, Port, GM_RX_IRQ_MSK, 0);
	GM_OUT16(IoC, Port, GM_TR_IRQ_MSK, 0);

	pPrt->PState = SK_PRT_STOP;

}	/* SkGmInitMac */


#ifdef SK_PHY_LP_MODE
/******************************************************************************
 *
 *	SkGmEnterLowPowerMode()
 *
 * Description:
 *	This function sets the Marvell Alaska PHY to the low power mode
 *	given by parameter mode.
 *	The following low power modes are available:
 *
 *		- COMA Mode (Deep Sleep):
 *			The PHY cannot wake up on its own.
 *
 *		- IEEE 22.2.4.1.5 compatible power down mode
 *			The PHY cannot wake up on its own.
 *
 *		- energy detect mode
 *			The PHY can wake up on its own by detecting activity
 *			on the CAT 5 cable.
 *
 *		- energy detect plus mode
 *			The PHY can wake up on its own by detecting activity
 *			on the CAT 5 cable.
 *			Connected devices can be woken up by sending normal link
 *			pulses every second.
 *
 * Note:
 *
 * Returns:
 *		0: ok
 *		1: error
 */
int SkGmEnterLowPowerMode(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port,		/* Port Index (e.g. MAC_1) */
SK_U8	Mode)		/* low power mode */
{
	SK_U8	LastMode;
	SK_U8	Byte;
	SK_U16	Word;
	SK_U16	PhySpec;
	SK_U16	ClkDiv;
	SK_U32	DWord;
	SK_U32	PowerDownBit;
	SK_U32	ClkMask;
	int		ChipId;
	int		Ret = 0;

	if (!(CHIP_ID_YUKON_2(pAC) || (pAC->GIni.GIYukonLite &&
		pAC->GIni.GIChipRev >= CHIP_REV_YU_LITE_A3))) {

		return(1);
	}

	/* save current power mode */
	LastMode = pAC->GIni.GP[Port].PPhyPowerState;
	pAC->GIni.GP[Port].PPhyPowerState = Mode;

	ChipId = pAC->GIni.GIChipId;

	SK_DBG_MSG(pAC, SK_DBGMOD_POWM, SK_DBGCAT_CTRL,
		("SkGmEnterLowPowerMode: %u\n", Mode));

	/* release GPHY Control reset */
	SK_OUT8(IoC, MR_ADDR(Port, GPHY_CTRL), (SK_U8)GPC_RST_CLR);

	/* release GMAC reset */
	SK_OUT8(IoC, MR_ADDR(Port, GMAC_CTRL), (SK_U8)GMC_RST_CLR);

	if (ChipId == CHIP_ID_YUKON_EC_U ||
		ChipId == CHIP_ID_YUKON_EX ||
		ChipId >= CHIP_ID_YUKON_SUPR) {
		/* re-read PEX Link Control */
		SK_IN16(IoC, PCI_C2(pAC, IoC, PEX_CAP_REGS(PEX_LNK_CTRL)),
			&pAC->GIni.GIPexLinkCtrl);

		/* select page 2 to access MAC control register */
		SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, 2);

		SkGmPhyRead(pAC, IoC, Port, PHY_MARV_PHY_CTRL, &Word);
		/* allow GMII Power Down */
		Word &= ~PHY_M_MAC_GMIF_PUP;
		SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PHY_CTRL, Word);

		/* set page register back to 0 */
		SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, 0);
	}

	switch (Mode) {
	/* COMA mode (deep sleep) */
	case PHY_PM_DEEP_SLEEP:
		/* setup General Purpose Control Register */
		GM_OUT16(IoC, Port, GM_GP_CTRL, GM_GPCR_FL_PASS |
			GM_GPCR_SPEED_100 | GM_GPCR_AU_ALL_DIS);

		if (CHIP_ID_YUKON_2(pAC)) {
			/* set power down bit */
			PowerDownBit = (Port == MAC_1) ? PCI_Y2_PHY1_POWD :
				PCI_Y2_PHY2_POWD;

			if (ChipId != CHIP_ID_YUKON_EC) {

				if (ChipId == CHIP_ID_YUKON_EC_U) {

					SkGmPhyRead(pAC, IoC, Port, PHY_MARV_PHY_CTRL, &PhySpec);
					/* enable Power Down */
					PhySpec |= PHY_M_PC_POW_D_ENA;
					SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PHY_CTRL, PhySpec);
				}

				/* set IEEE compatible Power Down Mode (dev. #4.99) */
				Ret = SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_CTRL, PHY_CT_PDOWN);
			}
		}
		else {
			/* apply COMA mode workaround for Yukon-Plus */
			(void)SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PAGE_ADDR, 31);

			Ret = SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PAGE_DATA, 0xfff3);

			PowerDownBit = PCI_PHY_COMA;
		}

		/* enable Config Write */
		SK_TST_MODE_ON(IoC);

		SK_IN32(IoC, PCI_C2(pAC, IoC, PCI_OUR_REG_1), &DWord);

		/* set PHY to PowerDown/COMA Mode */
		SK_OUT32(IoC, PCI_C2(pAC, IoC, PCI_OUR_REG_1), DWord | PowerDownBit);

		/* check if this routine was called from a for() loop */
		if (CHIP_ID_YUKON_2(pAC) &&
			(pAC->GIni.GIMacsFound == 1 || Port == MAC_2)) {

			if (ChipId == CHIP_ID_YUKON_EC_U ||
				ChipId == CHIP_ID_YUKON_EX ||
				ChipId >= CHIP_ID_YUKON_FE_P) {
				/* set GPHY Control reset */
				SK_OUT8(IoC, MR_ADDR(Port, GPHY_CTRL), (SK_U8)GPC_RST_SET);

				SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
					("Set PHY into reset, Port %d\n", Port));

				/* additional power saving measurements */
				SK_IN32(IoC, PCI_C2(pAC, IoC, PCI_OUR_REG_4), &DWord);

				if (pAC->GIni.GIGotoD3Cold) {
					/* set gating core clock for LTSSM in DETECT state */
					DWord |= (P_PEX_LTSSM_STAT(P_PEX_LTSSM_DET_STAT) |
						/* enable Gate Root Core Clock */
						P_CLK_GATE_ROOT_COR_ENA);

					/* set Mask Register for Release/Gate Clock */
					SK_OUT32(IoC, PCI_C2(pAC, IoC, PCI_OUR_REG_5),
						P_REL_MAIN_PWR_AVAIL | P_GAT_MAIN_PWR_N_AVAIL);
				}
				else {
					/* set gating core clock for LTSSM in L1 state */
					DWord |= (P_PEX_LTSSM_STAT(P_PEX_LTSSM_L1_STAT) |
						/* auto clock gated scheme controlled by CLKREQ */
						P_ASPM_A1_MODE_SELECT |
						/* enable Gate Root Core Clock */
						P_CLK_GATE_ROOT_COR_ENA);

					if (HW_FEATURE(pAC, HWF_WA_DEV_4200)) {
						/* enable Clock Power Management (CLKREQ) */
						if ((pAC->GIni.GIPexLinkCtrl & PEX_LC_CLK_PM_ENA) == 0) {

							pAC->GIni.GIPexLinkCtrl |= PEX_LC_CLK_PM_ENA;

							SK_OUT16(IoC, PCI_C2(pAC, IoC, PEX_CAP_REGS(PEX_LNK_CTRL)),
								pAC->GIni.GIPexLinkCtrl);
						}
					}
					else if ((pAC->GIni.GIPexLinkCtrl & PEX_LC_CLK_PM_ENA) == 0) {
						/* Force CLKREQ Enable and CLKREQn pin low */
						DWord |= P_ASPM_FORCE_CLKREQ_PIN | P_ASPM_FORCE_CLKREQ_ENA;
					}

					if (ChipId == CHIP_ID_YUKON_FE_P) {
						/* set delay-timer value to 2 ms */
						DWord |= P_TIMER_VALUE_MSK;
					}

					/* set Mask Register for Release/Gate Clock */
					SK_OUT32(IoC, PCI_C2(pAC, IoC, PCI_OUR_REG_5),
						P_REL_PCIE_EXIT_L1_ST | P_GAT_PCIE_ENTER_L1_ST |
						P_REL_PCIE_RX_EX_IDLE | P_GAT_PCIE_RX_EL_IDLE |
						P_REL_GPHY_LINK_UP | P_GAT_GPHY_LINK_DOWN);
				}

				SK_OUT32(IoC, PCI_C2(pAC, IoC, PCI_OUR_REG_4), DWord);

				if (!pAC->GIni.GIAsfRunning) {
					/* stop the watchdog */
					SK_OUT32(IoC, CPU_WDOG, 0);

					/* put CPU into reset state */
					SK_OUT8(IoC, B28_Y2_ASF_STAT_CMD, (SK_U8)HCU_CCSR_ASF_RESET);

					if (!HW_FEATURE(pAC, HWF_WA_DEV_542)) {
						/* put CPU into halt state */
						SK_OUT8(IoC, B28_Y2_ASF_STAT_CMD, (SK_U8)HCU_CCSR_ASF_HALTED);
					}

					/* stop the watchdog */
					SK_OUT32(IoC, CPU_WDOG, 0);
				}
			}
			else {
				/* ASF system clock stopped */
				SK_OUT8(IoC, B28_Y2_ASF_STAT_CMD, (SK_U8)Y2_ASF_CLK_HALT);
			}

			if (HW_FEATURE(pAC, HWF_RED_CORE_CLK_SUP)) {
				/* divide clock by 4 only for Yukon-EC */
				ClkDiv = (ChipId == CHIP_ID_YUKON_EC) ? 1 : 0;

				/* on Yukon-2 clock select value is 31 */
				DWord = (ChipId == CHIP_ID_YUKON_XL) ?
					(Y2_CLK_DIV_VAL_2(0) | Y2_CLK_SEL_VAL_2(31)) :
					 Y2_CLK_DIV_VAL(ClkDiv);

				/* check for Yukon-2 dual port PCI-Express adapter */
				if (!(pAC->GIni.GIMacsFound == 2 &&
					  pAC->GIni.GIPciBus == SK_PEX_BUS)) {
					/* enable Core Clock Division */
					DWord |= Y2_CLK_DIV_ENA;
				}

				SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
					("Set Core Clock: 0x%08lX\n", DWord));

				/* reduce Core Clock Frequency */
				SK_OUT32(IoC, B2_Y2_CLK_CTRL, DWord);
			}

			if (HW_FEATURE(pAC, HWF_CLK_GATING_ENABLE)) {
				/* check for Yukon-2 Rev. A2 */
				if (ChipId == CHIP_ID_YUKON_XL &&
					pAC->GIni.GIChipRev > CHIP_REV_YU_XL_A1) {
					/* enable bits are inverted */
					Byte = 0;
				}
				else {
					Byte = (SK_U8)(Y2_PCI_CLK_LNK1_DIS | Y2_COR_CLK_LNK1_DIS |
						Y2_CLK_GAT_LNK1_DIS | Y2_PCI_CLK_LNK2_DIS |
						Y2_COR_CLK_LNK2_DIS | Y2_CLK_GAT_LNK2_DIS);
				}

				SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
					("Set Clock Gating: 0x%02X\n", Byte));

				/* disable MAC/PHY, PCI and Core Clock for both Links */
				SK_OUT8(IoC, B2_Y2_CLK_GATE, Byte);
			}

			if (pAC->GIni.GILevel != SK_INIT_IO &&
				pAC->GIni.GIMacsFound == 1 &&
				pAC->GIni.GIPciBus == SK_PEX_BUS) {

				if (ChipId == CHIP_ID_YUKON_EC_U ||
					ChipId == CHIP_ID_YUKON_EX ||
					ChipId >= CHIP_ID_YUKON_FE_P) {

#ifdef PCI_E_L1_STATE
					SK_IN16(IoC, PCI_C2(pAC, IoC, PCI_OUR_REG_1), &Word);
					/* force to PCIe L1 */
					Word |= (SK_U16)PCI_FORCE_PEX_L1;
					SK_OUT16(IoC, PCI_C2(pAC, IoC, PCI_OUR_REG_1), Word);
#else /* !PCI_E_L1_STATE */

#ifdef DEEP_SLEEP_D1
					/* check if ASPM L1 enabled */
					if ((pAC->GIni.GIPexLinkCtrl & PEX_LC_ASPM_LC_L1) != 0) {
						break;
					}
#else
					break;
#endif /* !DEEP_SLEEP_D1 */

#endif /* !PCI_E_L1_STATE */
				}

				SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
					("Switch to D1 state\n"));

				/* switch to D1 state */
				SK_OUT8(IoC, PCI_C2(pAC, IoC, PCI_PM_CTL_STS), PCI_PM_STATE_D1);
			}
		}

		break;

	/* IEEE 22.2.4.1.5 compatible power down mode */
	case PHY_PM_IEEE_POWER_DOWN:

		if (!CHIP_ID_YUKON_2(pAC) && !pAC->GIni.GIYukonLite) {

			Ret = SkGmPhyRead(pAC, IoC, Port, PHY_MARV_PHY_CTRL, &PhySpec);

			/* disable MAC 125 MHz clock */
			PhySpec |= PHY_M_PC_DIS_125CLK;
			PhySpec &= ~PHY_M_PC_MAC_POW_UP;

			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PHY_CTRL, PhySpec);

			/* these register changes must be followed by a software reset */
			SkGmPhyRead(pAC, IoC, Port, PHY_MARV_CTRL, &Word);

			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_CTRL, Word | PHY_CT_RESET);
		}

		/* switch IEEE compatible power down mode on */
		SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_CTRL, PHY_CT_PDOWN);

#ifdef DEBUG
		SkGmPhyRead(pAC, IoC, Port, PHY_MARV_CTRL, &Word);
#endif /* DEBUG */
		break;

	/* energy detect and energy detect plus mode */
	case PHY_PM_ENERGY_DETECT:
	case PHY_PM_ENERGY_DETECT_PLUS:

		Ret = SkGmPhyRead(pAC, IoC, Port, PHY_MARV_PHY_CTRL, &PhySpec);

		if (!CHIP_ID_YUKON_2(pAC)) {
			/* disable MAC 125 MHz clock */
			PhySpec |= PHY_M_PC_DIS_125CLK;
		}

		if (ChipId == CHIP_ID_YUKON_FE || ChipId == CHIP_ID_YUKON_FE_P) {
			/* enable Energy Detect (sense & pulse) */
			PhySpec |= PHY_M_PC_ENA_ENE_DT;
		}
		else {
			/* clear energy detect mode bits */
			PhySpec &= ~PHY_M_PC_EN_DET_MSK;

			PhySpec |= (Mode == PHY_PM_ENERGY_DETECT) ? PHY_M_PC_EN_DET :
				PHY_M_PC_EN_DET_PLUS;
		}

		SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PHY_CTRL, PhySpec);

		/* these register changes must be followed by a software reset */
		SkGmPhyRead(pAC, IoC, Port, PHY_MARV_CTRL, &Word);
		Word |= PHY_CT_RESET;
		SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_CTRL, Word);

		if (ChipId == CHIP_ID_YUKON_FE_P) {
			/* Re-enable Link Partner Next Page */
			PhySpec |= PHY_M_PC_ENA_LIP_NP;

			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PHY_CTRL, PhySpec);
		}

		if (ChipId == CHIP_ID_YUKON_EC_U ||
			ChipId == CHIP_ID_YUKON_EX ||
			ChipId >= CHIP_ID_YUKON_FE_P) {
			/* additional power saving measurements */
			SK_IN32(IoC, PCI_C2(pAC, IoC, PCI_OUR_REG_4), &DWord);

			if (pAC->GIni.GIGotoD3Cold) {
				/* set gating core clock for LTSSM in DETECT state */
				DWord |= (P_PEX_LTSSM_STAT(P_PEX_LTSSM_DET_STAT) |
					/* enable Gate Root Core Clock */
					P_CLK_GATE_ROOT_COR_ENA);

				/* set Mask Register for Release/Gate Clock */
				SK_OUT32(IoC, PCI_C2(pAC, IoC, PCI_OUR_REG_5),
					P_REL_MAIN_PWR_AVAIL | P_GAT_MAIN_PWR_N_AVAIL);
			}
			else {
				/* set gating core clock for LTSSM in L1 state */
				DWord |= (P_PEX_LTSSM_STAT(P_PEX_LTSSM_L1_STAT) |
					/* auto clock gated scheme controlled by CLKREQ */
					P_ASPM_A1_MODE_SELECT |
					/* enable Gate Root Core Clock */
					P_CLK_GATE_ROOT_COR_ENA);

				if (HW_FEATURE(pAC, HWF_WA_DEV_4200)) {
					/* enable Clock Power Management (CLKREQ) */
					if ((pAC->GIni.GIPexLinkCtrl & PEX_LC_CLK_PM_ENA) == 0) {

						pAC->GIni.GIPexLinkCtrl |= PEX_LC_CLK_PM_ENA;

						SK_OUT16(IoC, PCI_C2(pAC, IoC, PEX_CAP_REGS(PEX_LNK_CTRL)),
							pAC->GIni.GIPexLinkCtrl);
					}
				}
				else if ((pAC->GIni.GIPexLinkCtrl & PEX_LC_CLK_PM_ENA) == 0) {
					/* Force CLKREQ Enable and CLKREQn pin low */
					DWord |= P_ASPM_FORCE_CLKREQ_PIN | P_ASPM_FORCE_CLKREQ_ENA;
				}

				if (ChipId == CHIP_ID_YUKON_FE_P) {
					/* set delay-timer value to 2 ms */
					DWord |= P_TIMER_VALUE_MSK;
				}

				ClkMask = P_REL_PCIE_EXIT_L1_ST | P_GAT_PCIE_ENTER_L1_ST |
						P_REL_PCIE_RX_EX_IDLE | P_GAT_PCIE_RX_EL_IDLE |
						P_REL_GPHY_LINK_UP | P_GAT_GPHY_LINK_DOWN;

				if (pAC->GIni.GIAsfEnabled) {
					ClkMask |= P_REL_CPU_TO_SLEEP | P_GAT_CPU_TO_SLEEP;
				}

				/* set Mask Register for Release/Gate Clock */
				SK_OUT32(IoC, PCI_C2(pAC, IoC, PCI_OUR_REG_5), ClkMask);
			}

			SK_OUT32(IoC, PCI_C2(pAC, IoC, PCI_OUR_REG_4), DWord);

#ifdef PCI_E_L1_STATE
			SK_IN16(IoC, PCI_C2(pAC, IoC, PCI_OUR_REG_1), &Word);
			/* enable PCIe L1 on GPHY link down */
			Word |= (SK_U16)PCI_ENA_GPHY_LNK;
			SK_OUT16(IoC, PCI_C2(pAC, IoC, PCI_OUR_REG_1), Word);
#endif /* PCI_E_L1_STATE */
		}

		pAC->GIni.GP[Port].PState = SK_PRT_STOP;
		break;

	/* don't change current power mode */
	default:
		pAC->GIni.GP[Port].PPhyPowerState = LastMode;
		Ret = 1;
	}

	return(Ret);

}	/* SkGmEnterLowPowerMode */

/******************************************************************************
 *
 *	SkGmLeaveLowPowerMode()
 *
 * Description:
 *	Leave the current low power mode and switch to normal mode
 *
 * Note:
 *
 * Returns:
 *		0:	ok
 *		1:	error
 */
int SkGmLeaveLowPowerMode(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port)		/* Port Index (e.g. MAC_1) */
{
	SK_GEPORT	*pPrt;
	SK_U32		DWord;
	SK_U32		PowerDownBit;
	SK_U16		Word;
	SK_U8		LastMode;
	int			ChipId;
	int			Ret = 0;

	if (!(CHIP_ID_YUKON_2(pAC) || (pAC->GIni.GIYukonLite &&
		pAC->GIni.GIChipRev >= CHIP_REV_YU_LITE_A3))) {

		return(1);
	}

	pPrt = &pAC->GIni.GP[Port];

	/* save current power mode */
	LastMode = pPrt->PPhyPowerState;
	pPrt->PPhyPowerState = PHY_PM_OPERATIONAL_MODE;

	ChipId = pAC->GIni.GIChipId;

	SK_DBG_MSG(pAC, SK_DBGMOD_POWM, SK_DBGCAT_CTRL,
		("SkGmLeaveLowPowerMode: %u\n", LastMode));

	switch (LastMode) {
	/* COMA mode (deep sleep) */
	case PHY_PM_DEEP_SLEEP:

		if (ChipId == CHIP_ID_YUKON_EC_U ||
			ChipId == CHIP_ID_YUKON_EX ||
			ChipId >= CHIP_ID_YUKON_FE_P) {

#ifdef PCI_E_L1_STATE
			SkPciReadCfgWord(pAC, PCI_C1(pAC, PCI_OUR_REG_1), &Word);

			/* set the default value into bits 6 & 5 */
			Word &= ~(SK_U16)(PCI_ENA_GPHY_LNK | PCI_FORCE_PEX_L1);

			SkPciWriteCfgWord(pAC, PCI_C1(pAC, PCI_OUR_REG_1), Word);
#endif /* PCI_E_L1_STATE */

			SK_IN32(IoC, PCI_C2(pAC, IoC, PCI_OUR_REG_4), &DWord);

			DWord &= P_ASPM_CONTROL_MSK;
			/* set all bits to 0 except bits 15..12 and 8 */
			SK_OUT32(IoC, PCI_C2(pAC, IoC, PCI_OUR_REG_4), DWord);

			/* set to default value */
			SK_OUT32(IoC, PCI_C2(pAC, IoC, PCI_OUR_REG_5), 0);
		}

		SkPciReadCfgWord(pAC, PCI_PM_CTL_STS, &Word);

		if ((Word & PCI_PM_STATE_MSK) != 0) {
			/* switch to D0 state */
			SkPciWriteCfgWord(pAC, PCI_PM_CTL_STS, Word & ~PCI_PM_STATE_MSK);
		}

		/* enable Config Write */
		SK_TST_MODE_ON(IoC);

		if (CHIP_ID_YUKON_2(pAC)) {
			/* disable Core Clock Division */
			SK_OUT32(IoC, B2_Y2_CLK_CTRL, Y2_CLK_DIV_DIS);

			/* set power down bit */
			PowerDownBit = (Port == MAC_1) ? PCI_Y2_PHY1_POWD :
				PCI_Y2_PHY2_POWD;
		}
		else {
			PowerDownBit = PCI_PHY_COMA;
		}

		SK_IN32(IoC, PCI_C2(pAC, IoC, PCI_OUR_REG_1), &DWord);

		/* Release PHY from PowerDown/COMA Mode */
		SK_OUT32(IoC, PCI_C2(pAC, IoC, PCI_OUR_REG_1), DWord & ~PowerDownBit);

		/* disable Config Write */
		SK_TST_MODE_OFF(IoC);

		if (CHIP_ID_YUKON_2(pAC)) {

			if (ChipId == CHIP_ID_YUKON_FE) {
				/* release IEEE compatible Power Down Mode */
				Ret = SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_CTRL, PHY_CT_ANE);
			}
			else if (ChipId == CHIP_ID_YUKON_EC_U ||
					 ChipId == CHIP_ID_YUKON_EX ||
					 ChipId >= CHIP_ID_YUKON_FE_P) {
				/* release GPHY Control reset */
				SK_OUT8(IoC, MR_ADDR(Port, GPHY_CTRL), (SK_U8)GPC_RST_CLR);
			}
		}
		else {
			SK_IN32(IoC, B2_GP_IO, &DWord);

			/* set to output */
			DWord |= (GP_DIR_9 | GP_IO_9);

			/* set PHY reset */
			SK_OUT32(IoC, B2_GP_IO, DWord);

			DWord &= ~GP_IO_9;	/* clear PHY reset (active high) */

			/* clear PHY reset */
			SK_OUT32(IoC, B2_GP_IO, DWord);
		}

		break;

	/* IEEE 22.2.4.1.5 compatible power down mode */
	case PHY_PM_IEEE_POWER_DOWN:

		if (ChipId != CHIP_ID_YUKON_XL) {

			Ret = SkGmPhyRead(pAC, IoC, Port, PHY_MARV_PHY_CTRL, &Word);
			Word &= ~PHY_M_PC_DIS_125CLK;	/* enable MAC 125 MHz clock */
			Word |= PHY_M_PC_MAC_POW_UP;	/* set MAC power up */
			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PHY_CTRL, Word);

			/* these register changes must be followed by a software reset */
			SkGmPhyRead(pAC, IoC, Port, PHY_MARV_CTRL, &Word);
			Word |= PHY_CT_RESET;
			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_CTRL, Word);
		}

		/* switch IEEE compatible power down mode off */
		SkGmPhyRead(pAC, IoC, Port, PHY_MARV_CTRL, &Word);
		Word &= ~PHY_CT_PDOWN;
		SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_CTRL, Word);

		break;

	/* energy detect and energy detect plus mode */
	case PHY_PM_ENERGY_DETECT:
	case PHY_PM_ENERGY_DETECT_PLUS:

		if (ChipId != CHIP_ID_YUKON_XL && !pPrt->PEnDetMode) {

			Ret = SkGmPhyRead(pAC, IoC, Port, PHY_MARV_PHY_CTRL, &Word);

			if (ChipId == CHIP_ID_YUKON_FE || ChipId == CHIP_ID_YUKON_FE_P) {
				/* disable Energy Detect */
				Word &= ~PHY_M_PC_ENA_ENE_DT;
			}
			else {
				/* disable energy detect mode & enable MAC 125 MHz clock */
				Word &= ~(PHY_M_PC_EN_DET_MSK | PHY_M_PC_DIS_125CLK);
			}

			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PHY_CTRL, Word);

			/* these register changes must be followed by a software reset */
			SkGmPhyRead(pAC, IoC, Port, PHY_MARV_CTRL, &Word);
			Word |= PHY_CT_RESET;
			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_CTRL, Word);
		}
		break;

	/* don't change current power mode */
	default:
		pAC->GIni.GP[Port].PPhyPowerState = LastMode;
		Ret = 1;
	}

	return(Ret);

}	/* SkGmLeaveLowPowerMode */
#endif /* SK_PHY_LP_MODE */

/******************************************************************************
 *
 *	SkGmInitPhyMarv() - Initialize the Marvell PHY registers
 *
 * Description:	initializes all the Marvell PHY registers
 *
 * Note:
 *
 * Returns:
 *	nothing
 */
static void SkGmInitPhyMarv(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port,		/* Port Index (MAC_1 + n) */
SK_BOOL	DoLoop)		/* Should a PHY LoopBack be set-up? */
{
	SK_GEPORT	*pPrt;
	SK_BOOL		AutoNeg;
	SK_BOOL		NewPhyType;
	SK_U16		PhyCtrl;
	SK_U16		C1000BaseT;
	SK_U16		AutoNegAdv;
	SK_U8		PauseMode;
	int			ChipId;
	int			ChipRev;
#ifndef SK_SLIM
	int			MacCtrl;
	SK_U16		LoopSpeed;
#endif /* !SK_SLIM */
	SK_U16		Word;
	SK_U16		PageReg;
#ifndef VCPU
	int			i;
	SK_U16		PhySpec;
	SK_U16		ExtPhyCtrl;
	SK_U16		BlinkCtrl;
	SK_U16		LedCtrl;
	SK_U16		LedConf;
	SK_U16		LedOver;
	int			Mode;
#if !defined(SK_DIAG) && !defined(SK_SLIM)
	SK_EVPARA	Para;
#endif /* !SK_DIAG && !SK_SLIM */
#if (defined(SK_DIAG) || (defined(DEBUG) && !defined(SK_SLIM)))
	SK_U16		PhyStat;
	SK_U16		PhyStat1;
	SK_U16		PhySpecStat;
#endif /* SK_DIAG || (DEBUG && !SK_SLIM) */
#endif /* !VCPU */

	/* set Pause On */
	PauseMode = (SK_U8)GMC_PAUSE_ON;

	pPrt = &pAC->GIni.GP[Port];

	ChipId = pAC->GIni.GIChipId;

	ChipRev = pAC->GIni.GIChipRev;

	NewPhyType = HW_HAS_NEWER_PHY(pAC);

	/* Auto-negotiation ? */
	AutoNeg = pPrt->PLinkMode != SK_LMODE_HALF &&
			  pPrt->PLinkMode != SK_LMODE_FULL;

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("InitPhyMarv: Port %d, Auto-neg. %s, LMode %d, LSpeed %d, FlowC %d\n",
		 Port, AutoNeg ? "ON" : "OFF",
		 pPrt->PLinkMode, pPrt->PLinkSpeed, pPrt->PFlowCtrlMode));

#ifndef VCPU
	ExtPhyCtrl = 0;

	/* read Id from PHY */
	if (SkGmPhyRead(pAC, IoC, Port, PHY_MARV_ID1, &pPrt->PhyId1) != 0) {

#if !defined(SK_DIAG) && !defined(SK_SLIM)
		Para.Para64 = Port;
		SkEventQueue(pAC, SKGE_DRV, SK_DRV_PORT_FAIL, Para);
#endif /* !SK_DIAG && !SK_SLIM */

		return;
	}
#endif /* !VCPU */

	/* check for Gigabit adapters */
	if ((pPrt->PLinkSpeedCap & SK_LSPEED_CAP_1000MBPS) != 0) {

#ifndef SK_SLIM
		if (DoLoop) {
			/* special setup for newer PHYs */
			if (NewPhyType) {

				LoopSpeed = pPrt->PLinkSpeed;

				if (LoopSpeed == SK_LSPEED_AUTO) {
					/* force 1000 Mbps */
					LoopSpeed = SK_LSPEED_1000MBPS;
				}
				LoopSpeed += 2;

#ifdef VCPU
				if (CHIP_ID_YUKON_PRM == ChipId || CHIP_ID_YUKON_OP_2 == ChipId) {
					/*
					 * from task phy_init in sim_tasks_pcix.inc, 20100609
					 * needed for simulation with REAL_PHY
					 */
					SkGmPhyWrite(pAC, IoC, Port, 22, 0x4000);
					SkGmPhyWrite(pAC, IoC, Port, 22, 0x40fa);
					SkGmPhyWrite(pAC, IoC, Port,  0, 0x0200);
					SkGmPhyWrite(pAC, IoC, Port, 16, 0x0040);
					SkGmPhyWrite(pAC, IoC, Port,  1, 0x0010);
					if (SK_LSPEED_100MBPS == pPrt->PLinkSpeed) {
						SkGmPhyWrite(pAC, IoC, Port, 22, 0x4002);
						SkGmPhyWrite(pAC, IoC, Port, 16, 0x9000);
						SkGmPhyWrite(pAC, IoC, Port, 21, 0x0001);
					}
					else if (SK_LSPEED_10MBPS == pPrt->PLinkSpeed) {
						SkGmPhyWrite(pAC, IoC, Port, 22, 0x4002);
						SkGmPhyWrite(pAC, IoC, Port, 16, 0x9000);
						SkGmPhyWrite(pAC, IoC, Port, 21, 0x0000);
					}
					SkGmPhyWrite(pAC, IoC, Port, 22, 0x4000);
				}
#endif	/* VCPU */

				/* save page register */
				SkGmPhyRead(pAC, IoC, Port, PHY_MARV_EXT_ADR, &PageReg);

				/* select page 2 to access MAC control register */
				SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, 2);

				if (ChipId == CHIP_ID_YUKON_XL) {
					/* set PHY reg 0, page 2, field [6:4] */
					MacCtrl = PHY_MARV_CTRL;
					LoopSpeed <<= 4;
				}
				else {	/*
						 * CHIP_ID_YUKON_EC_U || CHIP_ID_YUKON_EX ||
						 * >= CHIP_ID_YUKON_SUPR
						 */
					/* set PHY reg 21, page 2, field [2:0] */
					MacCtrl = PHY_MARV_MAC_CTRL;
				}

				/* set MAC interface speed */
				SkGmPhyWrite(pAC, IoC, Port, MacCtrl, LoopSpeed);

				/* restore page register */
				SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, PageReg);

				/* disable link pulses */
				Word = PHY_M_PC_DIS_LINK_P;
			}
#ifndef VCPU
			else {
				/* set 'MAC Power up'-bit, set Manual MDI configuration */
				Word = PHY_M_PC_MAC_POW_UP;
			}

			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PHY_CTRL, Word);
#endif /* !VCPU */
		}
#ifndef VCPU
		else
#endif /* !VCPU */
#endif /* !SK_SLIM */
#ifndef VCPU
		if (!NewPhyType && AutoNeg && pPrt->PLinkSpeed == SK_LSPEED_AUTO &&
			/* check for Gigabit adapters */
			(pPrt->PLinkSpeedCap & SK_LSPEED_CAP_1000MBPS) != 0) {
			/* Read Ext. PHY Specific Control */
			SkGmPhyRead(pAC, IoC, Port, PHY_MARV_EXT_CTRL, &ExtPhyCtrl);

			ExtPhyCtrl &= ~(PHY_M_EC_M_DSC_MSK | PHY_M_EC_S_DSC_MSK |
				PHY_M_EC_MAC_S_MSK);

			ExtPhyCtrl |= PHY_M_EC_MAC_S(MAC_TX_CLK_25_MHZ);

			/* on PHY 88E1040 Rev.D0 (and newer) downshift control changed */
			if (pAC->GIni.GIYukonLite || ChipId == CHIP_ID_YUKON_EC) {
				/* set downshift counter to 3x and enable downshift */
				ExtPhyCtrl |= PHY_M_EC_DSC_2(2) | PHY_M_EC_DOWN_S_ENA;
			}
			else {
				/* set master & slave downshift counter to 1x */
				ExtPhyCtrl |= PHY_M_EC_M_DSC(0) | PHY_M_EC_S_DSC(1);
			}

			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_CTRL, ExtPhyCtrl);
			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("Set Ext. PHY Ctrl = 0x%04X\n", ExtPhyCtrl));
		}
#endif /* !VCPU */
	}

#ifndef VCPU
	if (CHIP_ID_YUKON_2(pAC)) {
		/* Read PHY Specific Control */
		SkGmPhyRead(pAC, IoC, Port, PHY_MARV_PHY_CTRL, &PhySpec);

		if (!DoLoop && pAC->GIni.GICopperType) {

			if (ChipId == CHIP_ID_YUKON_FE || ChipId == CHIP_ID_YUKON_FE_P) {
				/* enable Automatic Crossover (!!! Bits 5..4) */
				PhySpec |= (SK_U16)(PHY_M_PC_MDI_XMODE(PHY_M_PC_ENA_AUTO) >> 1);

				if (ChipId == CHIP_ID_YUKON_FE_P && ChipRev == CHIP_REV_YU_FE2_A0) {
					/* Enable Class A driver for FE+ A0 */
					SkGmPhyRead(pAC, IoC, Port, PHY_MARV_FE_SPEC_2, &Word);
					Word |= PHY_M_FESC_SEL_CL_A;
					SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_FE_SPEC_2, Word);
				}
			}
			else {
#ifndef SK_SLIM
				/* Read PHY Copper Specific Control 2 */
				SkGmPhyRead(pAC, IoC, Port, PHY_MARV_EXT_CTRL_2, &Word);

				if (HW_FEATURE(pAC, HWF_PHY_CLASS_A_100BT)) {
					/* Enable Class A driver for 100Base-T */
					Word |= BIT_12S;
				}

				if (ChipId >= CHIP_ID_YUKON_OPT && pPrt->PRevAutoNeg) {
					/* enable PHY Reverse Auto-Negotiation */
					Word |= BIT_13S;
				}

				if ((Word & (BIT_13S | BIT_12S)) != 0) {
					/* Write PHY changes (SW-reset must follow) */
					SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_CTRL_2, Word);
				}
#endif /* !SK_SLIM */

				/* set Transformer-less mode for PHY group A */
				if (HW_FEATURE(pAC, HWF_TRAFO_LESS_ENABLE)) {
					/* connect internal MDI termination to AVDD */
					SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PAGE_ADDR, 3);

					SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PAGE_DATA, 0x0002);

					/* apply fixes in PHY AFE */
					SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, 0x00ff);

					/* enable half amplitude mode */
					SkGmPhyWrite(pAC, IoC, Port, 24, 0xa104);
					SkGmPhyWrite(pAC, IoC, Port, 23, 0x2002);

					SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, 0);

					if (HW_FEATURE(pAC, HWF_PHY_SET_SLAVE_MDIX)) {
						/* set manual slave */
						pPrt->PMSMode = SK_MS_MODE_SLAVE;

						/* set manual MDIX */
						PhySpec &= ~(PHY_M_PC_EN_DET_MSK | PHY_M_PC_MDIX_MSK);

						PhySpec |= (SK_U16)PHY_M_PC_MDI_XMODE(PHY_M_PC_MAN_MDIX);

						/* force 1000Mbps full duplex */
						SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PAGE_ADDR, 7);

						SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PAGE_DATA, 0x0208);
					}
				}
				else {
					if (!pPrt->PEnDetMode) {
						/* disable Energy Detect Mode */
						PhySpec &= ~PHY_M_PC_EN_DET_MSK;
					}

					/* enable Automatic Crossover */
					PhySpec |= (SK_U16)PHY_M_PC_MDI_XMODE(PHY_M_PC_ENA_AUTO);

					/* downshift on PHY 88E1112 and 88E1149 is changed */
					if (AutoNeg && pPrt->PLinkSpeed == SK_LSPEED_AUTO &&
						NewPhyType) {
						/* set downshift counter to 3x and enable downshift */
						PhySpec &= ~PHY_M_PC_DSC_MSK;
						PhySpec |= PHY_M_PC_DSC(2) | PHY_M_PC_DOWN_S_ENA;
					}
				}
			}
		}
		/* workaround for deviation #4.88 (CRC errors) */
		else {
			/* disable Automatic Crossover */
			PhySpec &= ~PHY_M_PC_MDIX_MSK;
		}

		SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PHY_CTRL, PhySpec);
		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
			("Set PHY Spec Reg. = 0x%04X\n", PhySpec));
	}

	/* special setup for PHY 88E1112 Fiber */
	if (ChipId == CHIP_ID_YUKON_XL && !pAC->GIni.GICopperType) {
		/* select 1000BASE-X only mode in MAC Specific Ctrl Reg. */
		SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, 2);

		SkGmPhyRead(pAC, IoC, Port, PHY_MARV_PHY_CTRL, &Word);

		Word &= ~PHY_M_MAC_MD_MSK;
		Word |= PHY_M_MAC_MODE_SEL(PHY_M_MAC_MD_1000BX);

		SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PHY_CTRL, Word);

		/* select page 1 to access Fiber registers */
		SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, 1);

		if (pAC->GIni.GIPmdTyp == 'P') {
			/* for SFP-module set SIGDET polarity to low */
			SkGmPhyRead(pAC, IoC, Port, PHY_MARV_PHY_CTRL, &Word);

			Word |= PHY_M_FIB_SIGD_POL;

			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PHY_CTRL, Word);
		}
		/* !!! don't set page register back to 0 !!! */
	}
#endif /* !VCPU */

#ifdef XXX
	/* Read PHY Control */
	SkGmPhyRead(pAC, IoC, Port, PHY_MARV_CTRL, &PhyCtrl);

	if (!AutoNeg) {
		/* disable Auto-negotiation */
		PhyCtrl &= ~PHY_CT_ANE;
	}

	/* assert software reset */
	SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_CTRL, PhyCtrl | PHY_CT_RESET);
#endif /* XXX */

	PhyCtrl = 0;
	C1000BaseT = 0;
	AutoNegAdv = PHY_SEL_TYPE;

	/* manually Master/Slave ? */
	if (pPrt->PMSMode != SK_MS_MODE_AUTO) {
		/* enable Manual Master/Slave */
		C1000BaseT |= PHY_M_1000C_MSE;

		if (pPrt->PMSMode == SK_MS_MODE_MASTER) {
			C1000BaseT |= PHY_M_1000C_MSC;	/* set it to Master */
		}
	}

	/* Auto-negotiation ? */
	if (!AutoNeg) {

		if (pPrt->PLinkMode == SK_LMODE_FULL) {
			/* set Full Duplex Mode */
			PhyCtrl |= PHY_CT_DUP_MD;
		}

		/* set Master/Slave manually if not already done */
		if (pPrt->PMSMode == SK_MS_MODE_AUTO) {
			C1000BaseT |= PHY_M_1000C_MSE;	/* set it to Slave */
		}

		/* set Speed */
		switch (pPrt->PLinkSpeed) {
		case SK_LSPEED_AUTO:
		case SK_LSPEED_1000MBPS:
			PhyCtrl |= (((pPrt->PLinkSpeedCap & SK_LSPEED_CAP_1000MBPS) != 0) ?
						PHY_CT_SP1000 : PHY_CT_SP100);
			break;
		case SK_LSPEED_100MBPS:
			PhyCtrl |= PHY_CT_SP100;
			break;
		case SK_LSPEED_10MBPS:
			break;
		default:
			SK_ERR_LOG(pAC, SK_ERRCL_SW | SK_ERRCL_INIT, SKERR_HWI_E019,
				SKERR_HWI_E019MSG);
		}

		if ((pPrt->PFlowCtrlMode == SK_FLOW_STAT_NONE) ||
			/* disable Pause also for 10/100 Mbps in half duplex mode */
			(!(ChipId == CHIP_ID_YUKON_EC_U ||
			   ChipId == CHIP_ID_YUKON_EX ||
			   ChipId >= CHIP_ID_YUKON_SUPR) &&
			(pPrt->PLinkMode == SK_LMODE_HALF) &&
			 ((pPrt->PLinkSpeed == SK_LSPEED_STAT_100MBPS) ||
			  (pPrt->PLinkSpeed == SK_LSPEED_STAT_10MBPS)))) {

			/* set Pause Off */
			PauseMode = (SK_U8)GMC_PAUSE_OFF;
		}

		SK_OUT8(IoC, MR_ADDR(Port, GMAC_CTRL), PauseMode);

#ifdef XXX
		if (!DoLoop) {
			/* assert software reset */
			PhyCtrl |= PHY_CT_RESET;
		}
#endif /* XXX */
	}
	else {
		/* set Auto-negotiation advertisement */

		if (pAC->GIni.GICopperType) {
			/* set Speed capabilities */
			switch (pPrt->PLinkSpeed) {
			case SK_LSPEED_AUTO:
				/* check for Gigabit adapters */
				if ((pPrt->PLinkSpeedCap & SK_LSPEED_CAP_1000MBPS) != 0) {
					C1000BaseT |= PHY_M_1000C_AFD | PHY_M_1000C_AHD;
				}
				AutoNegAdv |= PHY_M_AN_100_FD_HD | PHY_M_AN_10_FD_HD;
				break;
			case SK_LSPEED_1000MBPS:
				/* check for Gigabit adapters */
				if ((pPrt->PLinkSpeedCap & SK_LSPEED_CAP_1000MBPS) != 0) {
					C1000BaseT |= PHY_M_1000C_AFD | PHY_M_1000C_AHD;
				}
				break;
			case SK_LSPEED_100MBPS:
				AutoNegAdv |= PHY_M_AN_100_FD_HD;

				if (!(HW_FEATURE(pAC, HWF_FORCE_AUTO_NEG) &&
						/* only in case of 100FD */
						pPrt->PLinkModeConf < SK_LMODE_AUTOHALF)) {
					/* advertise 10Base-T also */
					AutoNegAdv |= PHY_M_AN_10_FD_HD;
				}
				break;
			case SK_LSPEED_10MBPS:
				AutoNegAdv |= PHY_M_AN_10_FD_HD;
				break;
			default:
				SK_ERR_LOG(pAC, SK_ERRCL_SW | SK_ERRCL_INIT, SKERR_HWI_E019,
					SKERR_HWI_E019MSG);
			}

			/* set Full/half duplex capabilities */
			switch (pPrt->PLinkMode) {
			case SK_LMODE_AUTOHALF:
				C1000BaseT &= ~PHY_M_1000C_AFD;
				AutoNegAdv &= ~(PHY_M_AN_100_FD | PHY_M_AN_10_FD);
				break;
			case SK_LMODE_AUTOFULL:
				C1000BaseT &= ~PHY_M_1000C_AHD;
				AutoNegAdv &= ~(PHY_M_AN_100_HD | PHY_M_AN_10_HD);
				break;
			case SK_LMODE_AUTOBOTH:
				break;
			default:
				SK_ERR_LOG(pAC, SK_ERRCL_SW | SK_ERRCL_INIT, SKERR_HWI_E015,
					SKERR_HWI_E015MSG);
			}

			/* set Flow-control capabilities */
			switch (pPrt->PFlowCtrlMode) {
			case SK_FLOW_MODE_NONE:
				AutoNegAdv |= PHY_B_P_NO_PAUSE;
				break;
			case SK_FLOW_MODE_LOC_SEND:
				AutoNegAdv |= PHY_B_P_ASYM_MD;
				break;
			case SK_FLOW_MODE_SYMMETRIC:
				AutoNegAdv |= PHY_B_P_SYM_MD;
				break;
			case SK_FLOW_MODE_SYM_OR_REM:
				AutoNegAdv |= PHY_B_P_BOTH_MD;
				break;
			default:
				SK_ERR_LOG(pAC, SK_ERRCL_SW | SK_ERRCL_INIT, SKERR_HWI_E016,
					SKERR_HWI_E016MSG);
			}
		}
		else {	/* special defines for FIBER (88E1040S only) */

			/* set Full/half duplex capabilities */
			switch (pPrt->PLinkMode) {
			case SK_LMODE_AUTOHALF:
				AutoNegAdv |= PHY_M_AN_1000X_AHD;
				break;
			case SK_LMODE_AUTOFULL:
				AutoNegAdv |= PHY_M_AN_1000X_AFD;
				break;
			case SK_LMODE_AUTOBOTH:
				AutoNegAdv |= PHY_M_AN_1000X_AHD | PHY_M_AN_1000X_AFD;
				break;
			default:
				SK_ERR_LOG(pAC, SK_ERRCL_SW | SK_ERRCL_INIT, SKERR_HWI_E015,
					SKERR_HWI_E015MSG);
			}

			/* set Flow-control capabilities */
			switch (pPrt->PFlowCtrlMode) {
			case SK_FLOW_MODE_NONE:
				AutoNegAdv |= PHY_M_P_NO_PAUSE_X;
				break;
			case SK_FLOW_MODE_LOC_SEND:
				AutoNegAdv |= PHY_M_P_ASYM_MD_X;
				break;
			case SK_FLOW_MODE_SYMMETRIC:
				AutoNegAdv |= PHY_M_P_SYM_MD_X;
				break;
			case SK_FLOW_MODE_SYM_OR_REM:
				AutoNegAdv |= PHY_M_P_BOTH_MD_X;
				break;
			default:
				SK_ERR_LOG(pAC, SK_ERRCL_SW | SK_ERRCL_INIT, SKERR_HWI_E016,
					SKERR_HWI_E016MSG);
			}
		}

		if (!DoLoop) {
			/* Restart Auto-negotiation */
			PhyCtrl |= PHY_CT_ANE | PHY_CT_RE_CFG;
		}
	}

#ifdef VCPU
	if (ChipId == CHIP_ID_YUKON_PRM || ChipId == CHIP_ID_YUKON_OP_2) {
		/* email from Gu Lin 2010-05-03 */

		if (pPrt->PLinkSpeed == SK_LSPEED_1000MBPS) {
			/* Program PHY register 30 as 16'h0708 for simulation speed up */
			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PAGE_DATA, 0x0700 /* 0x0708 */);

			VCpuWait(2000);
		}
	}
	else {
		/* email from Gu Lin (08-03-2002) */

		/* Program PHY register 30 as 16'h0708 for simulation speed up */
		SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PAGE_DATA, 0x0700 /* 0x0708 */);

		VCpuWait(2000);
	}
#else /* !VCPU */
	if (ChipId != CHIP_ID_YUKON_FE && ChipId != CHIP_ID_YUKON_FE_P) {
		/* Write 1000Base-T Control Register */
		SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_1000T_CTRL, C1000BaseT);
		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
			("Set 1000B-T Ctrl  = 0x%04X\n", C1000BaseT));
	}

	/* Write AutoNeg Advertisement Register */
	SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_AUNE_ADV, AutoNegAdv);
	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("Set Auto-Neg.Adv. = 0x%04X\n", AutoNegAdv));
#endif /* !VCPU */

	/* Write to the PHY Control register with SW reset */
	SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_CTRL, PhyCtrl | PHY_CT_RESET);
	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("Set PHY Ctrl Reg. = 0x%04X\n", PhyCtrl));

#ifndef SK_SLIM
	if (DoLoop) {
		/* set the PHY Loopback bit */
		SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_CTRL, PhyCtrl | PHY_CT_LOOP);
	}
#endif /* !SK_SLIM */

#ifdef VCPU
#ifndef HASE
	VCpuWait(2000);
#endif /* HASE */
#else /* !VCPU */

	LedCtrl = PHY_M_LED_PULS_DUR(PULS_170MS);

	LedOver = 0;

	BlinkCtrl = pAC->GIni.GILedBlinkCtrl;

	if ((BlinkCtrl & SK_ACT_LED_BLINK) != 0) {

		if (ChipId == CHIP_ID_YUKON_FE) {
			/* on 88E3082 these bits are at 11..9 (shifted left) */
			LedCtrl |= PHY_M_LED_BLINK_RT(BLINK_84MS) << 1;

			Word = PHY_M_FELP_LED2_CTRL(LED_PAR_CTRL_LINK) |
				   /* change ACT LED control to LINK/ACT or blink mode */
				   PHY_M_FELP_LED1_CTRL(
					((BlinkCtrl & SK_LED_COMB_ACT_LNK) != 0) ?
					LED_PAR_CTRL_LNK_AC : LED_PAR_CTRL_ACT_BL) |
				   PHY_M_FELP_LED0_CTRL(
					/* check for LINK_LED mux */
					((BlinkCtrl & SK_LED_LINK_MUX_P60) != 0) ?
					LED_PAR_CTRL_LINK : LED_PAR_CTRL_SPEED);

			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_FE_LED_PAR, Word);
		}
		else if (ChipId == CHIP_ID_YUKON_FE_P) {
			/* Read PHY Specific Control */
			SkGmPhyRead(pAC, IoC, Port, PHY_MARV_PHY_CTRL, &PhySpec);

			/* Enable Link Partner Next Page */
			PhySpec |= PHY_M_PC_ENA_LIP_NP;

			/* disable Energy Detect and enable scrambler */
			PhySpec &= ~(PHY_M_PC_ENA_ENE_DT | PHY_M_PC_DIS_SCRAMB);

			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PHY_CTRL, PhySpec);

			/* set LED2 -> ACT, LED1 -> LINK, LED0 -> SPEED */
			Word = PHY_M_FELP_LED2_CTRL(
					((BlinkCtrl & SK_LED_COMB_ACT_LNK) != 0) ?
					LED_PAR_CTRL_LNK_AC : LED_PAR_CTRL_ACT_BL) |
				   PHY_M_FELP_LED1_CTRL(LED_PAR_CTRL_LINK) |
				   PHY_M_FELP_LED0_CTRL(
					/* check for LINK_LED mux */
					((BlinkCtrl & SK_LED_LINK_MUX_P60) != 0) ?
					LED_PAR_CTRL_LINK : LED_PAR_CTRL_SPEED);

			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_FE_LED_PAR, Word);
		}
		else if (NewPhyType) {
			/* save page register */
			SkGmPhyRead(pAC, IoC, Port, PHY_MARV_EXT_ADR, &PageReg);

			/* select page 3 to access LED control register */
			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, 3);

			/* LINK/ACT (Yukon-2 only) */
			LedConf = PHY_M_LEDC_LOS_CTRL(LC_LNK_ON_ACT_BL) |
					  PHY_M_LEDC_STA1_CTRL(LC_LINK_ON) |	/* 100 Mbps */
					  PHY_M_LEDC_STA0_CTRL(LC_LINK_ON);		/* 1000 Mbps */

			Mode = LC_LINK_ON;		/* 10 Mbps: On */

			if (ChipId == CHIP_ID_YUKON_XL) {
				/* set Polarity Control register */
				SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PHY_STAT, (SK_U16)
					(PHY_M_POLC_LS1_P_MIX(4) | PHY_M_POLC_IS0_P_MIX(4) |
					 PHY_M_POLC_LOS_CTRL(2) | PHY_M_POLC_INIT_CTRL(2) |
					 PHY_M_POLC_STA1_CTRL(2) | PHY_M_POLC_STA0_CTRL(2)));
			}
			else if (ChipId == CHIP_ID_YUKON_EC_U ||
					 ChipId == CHIP_ID_YUKON_EX ||
					 ChipId >= CHIP_ID_YUKON_SUPR) {
				/* check for LINK_LED mux */
				if ((BlinkCtrl & SK_LED_LINK_MUX_P60) != 0) {

					/* LED scheme #2 */
					SK_IN16(IoC, GPHY_CTRL, &Word);

					Word |= GPC_LED_CONF_VAL(4);

					/* set GPHY LED Config */
					SK_OUT16(IoC, GPHY_CTRL, Word);
				}
				else {
					/* check for LED config bit 8 in PCI Our4 */
					SK_IN16(IoC, PCI_C2(pAC, IoC, PCI_OUR_REG_4), &Word);

					if ((Word & P_PIN63_LINK_LED_ENA) == 0) {
						if ((BlinkCtrl & SK_LED_COMB_ACT_LNK) == 0) {
							/* LED scheme #1 */
							Mode = LC_FORCE_OFF;	/* 10 Mbps: forced Off */
						}
						/* else LED scheme #5 */

						if ((BlinkCtrl & SK_ACT_LED_NOTR_OFF) == 0) {
							/* set LED[5:4] Function Control and Polarity */
							SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_INT_STAT,
								(SK_U16)
								/* LED_ACT to Link/Act. */
								(PHY_M_LEDC_STA1_CTRL(LC_LNK_ON_ACT_BL) |
								/* LED_DUP to Duplex */
								PHY_M_LEDC_STA0_CTRL(LC_DUPLEX_ON)));
						}
					}
					/* LED scheme #3 if P_PIN63_LINK_LED_ENA is set */
				}

				/* set Blink Rate in LED Timer Control Register */
				SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_INT_MASK,
					LedCtrl | (SK_U16)PHY_M_LED_BLINK_RT(BLINK_84MS));
			}

			LedConf |= PHY_M_LEDC_INIT_CTRL(Mode);

			/* set LED Function Control register */
			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PHY_CTRL, LedConf);

#if (defined(SK_DIAG) || (defined(DEBUG) && !defined(SK_SLIM)))
			/* select page 6 to access Packet Generation register */
			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, 6);

			SkGmPhyRead(pAC, IoC, Port, PHY_MARV_PHY_CTRL, &Word);

			Word |= BIT_4S;			/* enable CRC checker */

			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PHY_CTRL, Word);

			/* enable temperature sensor */
			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_CTRL_2, BIT_5S);
#endif /* SK_DIAG || (DEBUG && !SK_SLIM) */

			/* restore page register */
			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, PageReg);
		}
		else {
			/* set Tx LED (LED_TX) to blink mode on Rx OR Tx activity */
			LedCtrl |= PHY_M_LED_BLINK_RT(BLINK_84MS) | PHY_M_LEDC_TX_CTRL;

			/* on PHY 88E1111 there is a change for LED control */
			if (ChipId == CHIP_ID_YUKON_EC &&
				(BlinkCtrl & SK_DUAL_LED_ACT_LNK) != 0) {
				/* Yukon-EC needs setting of 2 bits: 0,6=11) */
				LedCtrl |= PHY_M_LEDC_TX_C_LSB;
			}
			/* turn off the Rx LED (LED_RX) */
			LedOver |= PHY_M_LED_MO_RX(MO_LED_OFF);
		}
	}

	if ((BlinkCtrl & SK_DUP_LED_NORMAL) != 0) {
		/* disable blink mode (LED_DUPLEX) on collisions */
		LedCtrl |= PHY_M_LEDC_DP_CTRL;
	}

	if (ChipId == CHIP_ID_YUKON_EC_U || ChipId == CHIP_ID_YUKON_UL_2) {
		/* apply fixes in PHY AFE */
		SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, 0x00ff);

		/* increase differential signal amplitude in 10BASE-T */
		SkGmPhyWrite(pAC, IoC, Port, 24, 0xaa99);
		SkGmPhyWrite(pAC, IoC, Port, 23, 0x2011);

		if (ChipId == CHIP_ID_YUKON_EC_U) {
			/* fix for IEEE A/B Symmetry failure in 1000BASE-T */
			SkGmPhyWrite(pAC, IoC, Port, 24, 0xa204);
			SkGmPhyWrite(pAC, IoC, Port, 23, 0x2002);
		}

		/* set page register back to 0 */
		SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, 0);
	}
	else if (ChipId == CHIP_ID_YUKON_FE_P && ChipRev == CHIP_REV_YU_FE2_A0) {
		/* apply workaround for integrated resistors calibration */
		SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PAGE_ADDR, 17);

		SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PAGE_DATA, 0x3f60);
	}
	else if (ChipId == CHIP_ID_YUKON_OPT && ChipRev == 0) {
		/* apply fixes in PHY AFE */
		SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, 0x00ff);

		/* apply RDAC termination workaround */
		SkGmPhyWrite(pAC, IoC, Port, 24, 0x2800);
		SkGmPhyWrite(pAC, IoC, Port, 23, 0x2001);

		/* set page register back to 0 */
		SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, 0);
	}
	else if (ChipId != CHIP_ID_YUKON_EX && ChipId < CHIP_ID_YUKON_SUPR) {
		/* no effect on Yukon-XL */
		SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_LED_CTRL, LedCtrl);

#ifndef SK_SLIM
		if ((BlinkCtrl & SK_LED_LINK100_ON) != 0) {
			/* only in forced 100 Mbps mode */
			if (!AutoNeg && pPrt->PLinkSpeed == SK_LSPEED_100MBPS) {
				/* turn on 100 Mbps LED (LED_LINK100) */
				LedOver |= PHY_M_LED_MO_100(MO_LED_ON);
			}
		}
#endif /* !SK_SLIM */

		if (LedOver != 0) {
			/* set Manual LED Override */
			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_LED_OVER, LedOver);
		}
	}
	else if (ChipId == CHIP_ID_YUKON_PRM && pAC->GIni.GIChipCap == 0x7) {
		/* Start Workaround for OptimaEEE Rev.Z0 */
		SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, 0x00fb);

		SkGmPhyWrite(pAC, IoC, Port,  1, 0x4099);
		SkGmPhyWrite(pAC, IoC, Port,  3, 0x1120);
		SkGmPhyWrite(pAC, IoC, Port, 11, 0x113c);
		SkGmPhyWrite(pAC, IoC, Port, 14, 0x8100);
		SkGmPhyWrite(pAC, IoC, Port, 15, 0x112a);
		SkGmPhyWrite(pAC, IoC, Port, 17, 0x1008);

		SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, 0x00fc);

		SkGmPhyWrite(pAC, IoC, Port,  1, 0x20b0);

		SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, 0x00ff);

		for (i = 0; i < (int)(sizeof(OptimaEeeAfe) / sizeof(OPT_EEE_HACK)); i++) {
			/* apply AFE settings */
			SkGmPhyWrite(pAC, IoC, Port, 17, OptimaEeeAfe[i].PhyVal);
			SkGmPhyWrite(pAC, IoC, Port, 16, OptimaEeeAfe[i].PhyReg | BIT_13S);
		}

		/* End Workaround for OptimaEEE */
		SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, 0);

		if (HW_FEATURE(pAC, HWF_DO_EEE_ENABLE)) {
			/* Enable 10Base-Te (EEE) */
			SkGmPhyRead(pAC, IoC, Port, PHY_MARV_EXT_CTRL, &Word);
			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_CTRL,
				Word | PHY_M_10B_TE_ENABLE);
		}
	}

#ifdef SK_DIAG
	c_print("Set PHY Ctrl = 0x%04X\n", PhyCtrl);
	c_print("Set 1000 B-T = 0x%04X\n", C1000BaseT);
	c_print("Set Auto-Neg = 0x%04X\n", AutoNegAdv);
	c_print("Set PHY Spec = 0x%04X\n", PhySpec);
	c_print("Set Ext Ctrl = 0x%04X\n", ExtPhyCtrl);
#endif /* SK_DIAG */

#if (defined(SK_DIAG) || (defined(DEBUG) && !defined(SK_SLIM)))
	/* Read PHY Control */
	SkGmPhyRead(pAC, IoC, Port, PHY_MARV_CTRL, &PhyCtrl);
	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("PHY Ctrl Reg. = 0x%04X\n", PhyCtrl));

	/* Read AutoNeg Advertisement Register */
	SkGmPhyRead(pAC, IoC, Port, PHY_MARV_AUNE_ADV, &AutoNegAdv);
	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("Auto-Neg.Adv. = 0x%04X\n", AutoNegAdv));

	if (ChipId != CHIP_ID_YUKON_FE && ChipId != CHIP_ID_YUKON_FE_P) {
		/* Read 1000Base-T Control Register */
		SkGmPhyRead(pAC, IoC, Port, PHY_MARV_1000T_CTRL, &C1000BaseT);
		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
			("1000B-T Ctrl  = 0x%04X\n", C1000BaseT));

		/* Read Ext. PHY Specific Control */
		SkGmPhyRead(pAC, IoC, Port, PHY_MARV_EXT_CTRL, &ExtPhyCtrl);
		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
			("Ext. PHY Ctrl = 0x%04X\n", ExtPhyCtrl));
	}

	/* Read PHY Status */
	SkGmPhyRead(pAC, IoC, Port, PHY_MARV_STAT, &PhyStat);
	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("PHY Stat Reg. = 0x%04X\n", PhyStat));

	SkGmPhyRead(pAC, IoC, Port, PHY_MARV_STAT, &PhyStat1);
	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("PHY Stat Reg. = 0x%04X\n", PhyStat1));

	/* Read PHY Specific Status */
	SkGmPhyRead(pAC, IoC, Port, PHY_MARV_PHY_STAT, &PhySpecStat);
	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("PHY Spec Stat = 0x%04X\n", PhySpecStat));

	pAC->GIni.GIPhyInitTime = (SK_U32)SkOsGetTime(pAC);

#endif /* SK_DIAG || (DEBUG && !SK_SLIM) */

#ifdef SK_DIAG
	c_print("PHY Ctrl Reg = 0x%04X\n", PhyCtrl);
	c_print("PHY 1000 Reg = 0x%04X\n", C1000BaseT);
	c_print("PHY AnAd Reg = 0x%04X\n", AutoNegAdv);
	c_print("Ext Ctrl Reg = 0x%04X\n", ExtPhyCtrl);
	c_print("PHY Stat Reg = 0x%04X\n", PhyStat);
	c_print("PHY Stat Reg = 0x%04X\n", PhyStat1);
	c_print("PHY Spec Reg = 0x%04X\n", PhySpecStat);
#endif /* SK_DIAG */

	/* clear PHY IRQ status */
	SkGmPhyRead(pAC, IoC, Port, PHY_MARV_INT_STAT, &Word);

	/* enable PHY interrupts */
	SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_INT_MASK, (SK_U16)PHY_M_DEF_MSK);
#endif /* !VCPU */

}	/* SkGmInitPhyMarv */


/******************************************************************************
 *
 *	SkMacInitPhy() - Initialize the PHY registers
 *
 * Description:	calls the Init PHY routines
 *
 * Note:
 *
 * Returns:
 *	nothing
 */
void SkMacInitPhy(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port,		/* Port Index (MAC_1 + n) */
SK_BOOL	DoLoop)		/* Should a PHY LoopBack be set-up? */
{
	SkGmInitPhyMarv(pAC, IoC, Port, DoLoop);

}	/* SkMacInitPhy */


/******************************************************************************
 *
 *	SkGmAutoNegDoneMarv() - Auto-negotiation handling
 *
 * Description:
 *	This function handles the auto-negotiation if the Done bit is set.
 *
 * Returns:
 *	SK_AND_OK	o.k.
 *	SK_AND_DUP_CAP	Duplex capability error happened
 *	SK_AND_OTHER	Other error happened
 */
static int SkGmAutoNegDoneMarv(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port)		/* Port Index (MAC_1 + n) */
{
	SK_GEPORT	*pPrt;
	SK_U16		ResAb;		/* Resolved Ability */
	SK_U16		AuxStat;	/* Auxiliary Status */
	SK_U16		Word;
	SK_U8		PauseMode;	/* Pause Mode */
#ifndef SK_SLIM
	SK_U16		LinkPartAb;	/* Link Partner Ability */
#ifndef SK_DIAG
	SK_EVPARA	Para;
#endif /* !SK_DIAG */
#endif /* !SK_SLIM */

	/* set Pause On */
	PauseMode = (SK_U8)GMC_PAUSE_ON;

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("AutoNegDoneMarv, Port %d\n", Port));

	pPrt = &pAC->GIni.GP[Port];

#ifndef SK_SLIM
	/* Get PHY parameters */
	SkGmPhyRead(pAC, IoC, Port, PHY_MARV_AUNE_LP, &LinkPartAb);

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("Link P.Abil. = 0x%04X\n", LinkPartAb));

	if ((LinkPartAb & PHY_M_AN_RF) != 0) {
		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_ERR,
			("AutoNegFail: Remote fault bit set Port %d\n", Port));
		pPrt->PAutoNegFail = SK_TRUE;

		return(SK_AND_OTHER);
	}

	if (pAC->GIni.GICopperType) {

		if (pAC->GIni.GIChipId == CHIP_ID_YUKON_EC_U) {
			/* #979 remove workaround for 100M Link */
			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PAGE_DATA, 0);
		}

		/* Read PHY Auto-Negotiation Expansion */
		SkGmPhyRead(pAC, IoC, Port, PHY_MARV_AUNE_EXP, &Word);

		if ((Word & PHY_ANE_LP_CAP) == 0) {

			pPrt->PLipaAutoNeg = SK_LIPA_MANUAL;

#ifndef SK_DIAG
			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("Link Partner not Auto-Neg. able, AN Exp.: 0x%04X\n",
				Word));

#ifndef NDIS_MINIPORT_DRIVER
			SK_ERR_LOG(pAC, SK_ERRCL_INFO, SKERR_HWI_E025, SKERR_HWI_E025MSG);
#endif
			Para.Para64 = Port;
			SkEventQueue(pAC, SKGE_DRV, SK_DRV_LIPA_NOT_AN_ABLE, Para);
#else
			c_print("Link Partner not Auto-Neg. able, AN Exp.: 0x%04X\n",
				Word);
#endif /* !SK_DIAG */

			if (HW_FEATURE(pAC, HWF_FORCE_AUTO_NEG) &&
				pPrt->PLinkModeConf < SK_LMODE_AUTOHALF) {
				/* set used link speed */
				pPrt->PLinkSpeedUsed = pPrt->PLinkSpeed;

				/* Set Link Mode Status */
				pPrt->PLinkModeStatus = (SK_U8)
					(pPrt->PLinkModeConf == SK_LMODE_FULL) ?
					SK_LMODE_STAT_FULL : SK_LMODE_STAT_HALF;

				return(SK_AND_OK);
			}
		}
		else {
			/* check for fake auto-negotiation from link partner */
			if (HW_FEATURE(pAC, HWF_FORCE_AUTO_NEG) &&
				/* only in case of 100FD */
				pPrt->PLinkModeConf == SK_LMODE_FULL &&
				pPrt->PLinkSpeed == SK_LSPEED_100MBPS &&
				/* check if only 100HD is advertised */
				(LinkPartAb & PHY_M_AN_100_FD_HD) == PHY_M_AN_100_HD) {

				pPrt->PLipaAutoNeg = SK_LIPA_MANUAL;

				/* set used link speed */
				pPrt->PLinkSpeedUsed = (SK_U8)SK_LSPEED_STAT_100MBPS;

				/* Set Link Mode Status */
				pPrt->PLinkModeStatus = (SK_U8)SK_LMODE_STAT_FULL;

				return(SK_AND_OK);
			}

			pPrt->PLipaAutoNeg = SK_LIPA_AUTO;

			/* save Link Partner Ability */
			pPrt->PLipaAbil = LinkPartAb;
		}
	}
#endif /* !SK_SLIM */

	/* check for Gigabit adapters */
	if ((pPrt->PLinkSpeedCap & SK_LSPEED_CAP_1000MBPS) != 0) {

		SkGmPhyRead(pAC, IoC, Port, PHY_MARV_1000T_STAT, &ResAb);

#if defined(DEBUG) && !defined(SK_SLIM)
		if ((ResAb & PHY_B_1000S_LP_FD) == 0) {

			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("Link Partner is not capable of 1000FD, Phy1000BT: 0x%04X\n",
				 ResAb));
		}
#endif /* DEBUG && !SK_SLIM */

		/* Check Master/Slave resolution */
		if ((ResAb & PHY_B_1000S_MSF) != 0) {

			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_ERR,
				("Master/Slave Fault Port %d\n", Port));

			pPrt->PAutoNegFail = SK_TRUE;
			pPrt->PMSStatus = SK_MS_STAT_FAULT;

			return(SK_AND_OTHER);
		}

		pPrt->PMSStatus = ((ResAb & PHY_B_1000S_MSR) != 0) ?
			(SK_U8)SK_MS_STAT_MASTER : (SK_U8)SK_MS_STAT_SLAVE;
	}

	/* Read PHY Specific Status */
	SkGmPhyRead(pAC, IoC, Port, PHY_MARV_PHY_STAT, &AuxStat);

	/* Check Speed & Duplex resolved */
	if ((AuxStat & PHY_M_PS_SPDUP_RES) == 0) {

		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_ERR,
			("AutoNegFail: Speed & Duplex not resolved, Port %d\n", Port));

		pPrt->PAutoNegFail = SK_TRUE;
		pPrt->PLinkModeStatus = (SK_U8)SK_LMODE_STAT_UNKNOWN;

		return(SK_AND_DUP_CAP);
	}

	if (pPrt->PLipaAutoNeg == SK_LIPA_AUTO) {
		pPrt->PLinkModeStatus = (SK_U8)(((AuxStat & PHY_M_PS_FULL_DUP) != 0) ?
			SK_LMODE_STAT_AUTOFULL : SK_LMODE_STAT_AUTOHALF);
	}
	else {
		pPrt->PLinkModeStatus = (SK_U8)(((AuxStat & PHY_M_PS_FULL_DUP) != 0) ?
			SK_LMODE_STAT_FULL : SK_LMODE_STAT_HALF);
	}

	if (pAC->GIni.GIChipId == CHIP_ID_YUKON_FE ||
		pAC->GIni.GIChipId == CHIP_ID_YUKON_FE_P) {
		/* set used link speed */
		pPrt->PLinkSpeedUsed = (SK_U8)(((AuxStat & PHY_M_PS_SPEED_100) != 0) ?
			SK_LSPEED_STAT_100MBPS : SK_LSPEED_STAT_10MBPS);

		/* Yukon-FE PHYs don't report the Flow-Control resolution */
		AuxStat = 0;

		/* read MAC General Purpose Status */
		GM_IN16(IoC, Port, GM_GP_STAT, &Word);

		/* get the negotiated Flow-Control resolution from GPSR */
		if ((Word & GM_GPSR_FC_RX_DIS) == 0) {
			AuxStat |= PHY_M_PS_RX_P_EN;
		}

		if ((Word & GM_GPSR_FC_TX_DIS) == 0) {
			AuxStat |= PHY_M_PS_TX_P_EN;
		}
	}
	else {
		/* Flow-Control Timeout for Fast Ethernet clock (50 MHz) */
		Word = 12500;

		/* set used link speed */
		switch ((unsigned)(AuxStat & PHY_M_PS_SPEED_MSK)) {
		case (unsigned)PHY_M_PS_SPEED_1000:
			pPrt->PLinkSpeedUsed = (SK_U8)SK_LSPEED_STAT_1000MBPS;
			/* adjust Flow-Control Timeout for Gigabit clock (125 MHz) */
			Word = 31250;
			break;
		case PHY_M_PS_SPEED_100:
			pPrt->PLinkSpeedUsed = (SK_U8)SK_LSPEED_STAT_100MBPS;
			break;
		default:
			pPrt->PLinkSpeedUsed = (SK_U8)SK_LSPEED_STAT_10MBPS;
		}

		if (pAC->GIni.GIChipId >= CHIP_ID_YUKON_SUPR) {
			/* adjust Flow-Control Timeout */
			GM_OUT16(IoC, Port, GM_FC_TIMEOUT, Word);
		}

		if (HW_HAS_NEWER_PHY(pAC)) {
			/* Tx & Rx Pause Enabled bits are at 9..8 */
			AuxStat >>= 6;

			if (!pAC->GIni.GICopperType) {
				/* always 1000 Mbps on fiber */
				pPrt->PLinkSpeedUsed = (SK_U8)SK_LSPEED_STAT_1000MBPS;
			}
		}

		AuxStat &= PHY_M_PS_PAUSE_MSK;
	}

	/* We are using IEEE 802.3z/D5.0 Table 37-4 */
	if (AuxStat == PHY_M_PS_PAUSE_MSK) {
		/* Symmetric PAUSE */
		pPrt->PFlowCtrlStatus = SK_FLOW_STAT_SYMMETRIC;
	}
	else if (AuxStat == PHY_M_PS_RX_P_EN) {
		/* enable PAUSE receive, disable PAUSE transmit */
		pPrt->PFlowCtrlStatus = SK_FLOW_STAT_REM_SEND;
	}
	else if (AuxStat == PHY_M_PS_TX_P_EN) {
		/* disable PAUSE receive, enable PAUSE transmit */
		pPrt->PFlowCtrlStatus = SK_FLOW_STAT_LOC_SEND;
	}
	else {
		/* PAUSE mismatch -> no PAUSE */
		pPrt->PFlowCtrlStatus = SK_FLOW_STAT_NONE;
	}

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("LinkSpeedUsed = %d\n", pPrt->PLinkSpeedUsed));

	if ((pPrt->PFlowCtrlStatus == SK_FLOW_STAT_NONE) ||
		/* disable Pause also for 10/100 Mbps in half duplex mode */
		(!(pAC->GIni.GIChipId == CHIP_ID_YUKON_EC_U ||
		   pAC->GIni.GIChipId == CHIP_ID_YUKON_EX ||
		   pAC->GIni.GIChipId >= CHIP_ID_YUKON_SUPR) &&
		(pPrt->PLinkSpeedUsed < (SK_U8)SK_LSPEED_STAT_1000MBPS) &&
		 pPrt->PLinkModeStatus == (SK_U8)SK_LMODE_STAT_AUTOHALF)) {

		/* set Pause Off */
		PauseMode = (SK_U8)GMC_PAUSE_OFF;
	}

	SK_OUT8(IoC, MR_ADDR(Port, GMAC_CTRL), PauseMode);

	return(SK_AND_OK);
}	/* SkGmAutoNegDoneMarv */


/******************************************************************************
 *
 *	SkMacAutoNegDone() - Auto-negotiation handling
 *
 * Description:	calls the auto-negotiation done routine and enables Rx/Tx
 *
 * Returns:
 *	SK_AND_OK	o.k.
 *	SK_AND_DUP_CAP	Duplex capability error happened
 *	SK_AND_OTHER	Other error happened
 */
int SkMacAutoNegDone(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port)		/* Port Index (MAC_1 + n) */
{
	SK_GEPORT	*pPrt;
	int			Rtv;

	pPrt = &pAC->GIni.GP[Port];

	Rtv = SkGmAutoNegDoneMarv(pAC, IoC, Port);

	if (Rtv != SK_AND_OK) {
		return(Rtv);
	}

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("AutoNeg done Port %d\n", Port));

	/* We checked everything and may now enable the link */
	pPrt->PAutoNegFail = SK_FALSE;

	/* enable receiver/transmitter */
	SkMacRxTxEnable(pAC, IoC, Port);

	return(SK_AND_OK);
}	/* SkMacAutoNegDone */


#ifndef SK_SLIM
/******************************************************************************
 *
 *	SkGmSetRxTxEn() - Special Set Rx/Tx Enable and some features in GMAC
 *
 * Description:
 *  sets MAC LoopBack and Duplex Mode in the General Purpose Control Reg.
 *  enables Rx/Tx
 *
 * Returns: N/A
 */
static void SkGmSetRxTxEn(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port,		/* Port Index (MAC_1 + n) */
int		Para)		/* Parameter to set: MAC LoopBack, Duplex Mode */
{
	SK_U16	Ctrl;

	GM_IN16(IoC, Port, GM_GP_CTRL, &Ctrl);

	switch (Para & (SK_MAC_LOOPB_ON | SK_MAC_LOOPB_OFF)) {
	case SK_MAC_LOOPB_ON:
		Ctrl |= GM_GPCR_LOOP_ENA;
		break;
	case SK_MAC_LOOPB_OFF:
		Ctrl &= ~GM_GPCR_LOOP_ENA;
		break;
	}

	switch (Para & (SK_PHY_FULLD_ON | SK_PHY_FULLD_OFF)) {
	case SK_PHY_FULLD_ON:
		Ctrl |= GM_GPCR_DUP_FULL;
		break;
	case SK_PHY_FULLD_OFF:
		Ctrl &= ~GM_GPCR_DUP_FULL;
		break;
	}

	GM_OUT16(IoC, Port, GM_GP_CTRL, Ctrl | GM_GPCR_RX_ENA | GM_GPCR_TX_ENA);

}	/* SkGmSetRxTxEn */


/******************************************************************************
 *
 *	SkMacSetRxTxEn() - Special Set Rx/Tx Enable and parameters
 *
 * Description:	calls the Special Set Rx/Tx Enable routines
 *
 * Returns: N/A
 */
void SkMacSetRxTxEn(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port,		/* Port Index (MAC_1 + n) */
int		Para)		/* Parameter to set: MAC LoopBack, Duplex Mode */
{
	SkGmSetRxTxEn(pAC, IoC, Port, Para);

}	/* SkMacSetRxTxEn */
#endif /* !SK_SLIM */


/******************************************************************************
 *
 *	SkMacRxTxEnable() - Enable Rx/Tx activity if port is up
 *
 * Description:	enables Rx/Tx activity in the MAC
 *
 * Returns:
 *	0	o.k.
 *	!= 0	Error happened
 */
int SkMacRxTxEnable(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port)		/* Port Index (MAC_1 + n) */
{
	SK_GEPORT	*pPrt;
	SK_U16		Reg;		/* 16-bit register value */
	SK_U16		IntMask;	/* MAC interrupt mask */
#ifndef SK_SLIM
	SK_U16		SWord;
#endif /* !SK_SLIM */

	pPrt = &pAC->GIni.GP[Port];

	if (!pPrt->PHWLinkUp) {
		/* The Hardware link is NOT up */
		return(0);
	}

	if ((pPrt->PLinkMode == SK_LMODE_AUTOHALF ||
		 pPrt->PLinkMode == SK_LMODE_AUTOFULL ||
		 pPrt->PLinkMode == SK_LMODE_AUTOBOTH) &&
		 pPrt->PAutoNegFail) {
		/* Auto-negotiation is not done or failed */
		return(0);
	}

	/*
	 * Initialize the Interrupt Mask Register. Default IRQs are...
	 *	- Rx Counter Event Overflow
	 *	- Tx Counter Event Overflow
	 *	- Transmit FIFO Underrun
	 */
	IntMask = GMAC_DEF_MSK;

#if (defined(DEBUG) || defined(YUK2)) && !defined(SK_SLIM)
	/* add IRQ for Receive FIFO Overrun */
	IntMask |= GM_IS_RX_FF_OR;
#endif

	SK_OUT8(IoC, MR_ADDR(Port, GMAC_IRQ_MSK), (SK_U8)IntMask);

	/* get General Purpose Control */
	GM_IN16(IoC, Port, GM_GP_CTRL, &Reg);

	if (pPrt->PLinkModeStatus == SK_LMODE_STAT_FULL ||
		pPrt->PLinkModeStatus == SK_LMODE_STAT_AUTOFULL) {
		/* set to Full Duplex */
		Reg |= GM_GPCR_DUP_FULL;

#ifndef SK_SLIM
		if (HW_FEATURE(pAC, HWF_FORCE_AUTO_NEG) &&
			pPrt->PLinkModeConf < SK_LMODE_AUTOHALF) {
			/* disable auto-update for speed, duplex and flow-control */
			Reg |= GM_GPCR_AU_ALL_DIS;
		}
#endif /* !SK_SLIM */
	}

	/* WA for dev. #4.209 */
	if (pAC->GIni.GIChipId == CHIP_ID_YUKON_EC_U &&
		pAC->GIni.GIChipRev == CHIP_REV_YU_EC_U_A1) {
		/* enable/disable Store & Forward mode for TX */
		SK_OUT32(IoC, MR_ADDR(Port, TX_GMF_CTRL_T),
			pPrt->PLinkSpeedUsed != (SK_U8)SK_LSPEED_STAT_1000MBPS ?
			TX_STFW_ENA : TX_STFW_DIS);
	}

#ifndef SK_SLIM
	/* configure IPG according to used link speed */
	GM_IN16(IoC, Port, GM_SERIAL_MODE, &SWord);

	SWord &= ~GM_SMOD_IPG_MSK;

	SWord |= IPG_DATA_VAL((pPrt->PLinkSpeedUsed == SK_LSPEED_STAT_1000MBPS) ?
		pPrt->PMacIpgData_1G : pPrt->PMacIpgData_FE);

	GM_OUT16(IoC, Port, GM_SERIAL_MODE, SWord);
#endif /* SK_SLIM */

	/* enable Rx/Tx */
	GM_OUT16(IoC, Port, GM_GP_CTRL, Reg | GM_GPCR_RX_ENA | GM_GPCR_TX_ENA);

	pAC->GIni.GP[Port].PState = SK_PRT_RUN;

	return(0);

}	/* SkMacRxTxEnable */


/******************************************************************************
 *
 *	SkMacRxTxDisable() - Disable Receiver and Transmitter
 *
 * Description:	disables Rx/Tx in the MAC
 *
 * Returns: N/A
 */
void SkMacRxTxDisable(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port)		/* Port Index (MAC_1 + n) */
{
	SK_U16	Word;

	GM_IN16(IoC, Port, GM_GP_CTRL, &Word);

	Word &= ~(GM_GPCR_RX_ENA | GM_GPCR_TX_ENA);

	GM_OUT16(IoC, Port, GM_GP_CTRL, Word);

}	/* SkMacRxTxDisable */


/******************************************************************************
 *
 *	SkMacIrqDisable() - Disable IRQ from MAC
 *
 * Description:	sets the IRQ-mask to disable IRQ
 *
 * Returns: N/A
 */
void SkMacIrqDisable(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port)		/* Port Index (MAC_1 + n) */
{
	SK_GEPORT	*pPrt;

	pPrt = &pAC->GIni.GP[Port];

	/* disable all GMAC IRQs */
	SK_OUT8(IoC, MR_ADDR(Port, GMAC_IRQ_MSK), 0);

#ifndef VCPU
	if (pPrt->PState != SK_PRT_RESET) {
		/* disable all PHY IRQs */
		SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_INT_MASK, 0);
	}
#endif /* !VCPU */

}	/* SkMacIrqDisable */


#ifndef SK_SLIM
/******************************************************************************
 *
 *	SkMacTimeStamp() - Enable / Disable Time Stamp
 *
 * Description:	enable / disable Time Stamp generation for Rx packets
 *
 * Returns:
 *	nothing
 */
void SkMacTimeStamp(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC,	/* I/O Context */
int		Port,	/* Port Index (MAC_1 + n) */
SK_BOOL	Enable)	/* Enable / Disable */
{
	SK_U8	TimeCtrl;

	if (Enable) {
		TimeCtrl = GMT_ST_START | GMT_ST_CLR_IRQ;
	}
	else {
		TimeCtrl = GMT_ST_STOP | GMT_ST_CLR_IRQ;
	}
	/* Start/Stop Time Stamp Timer */
	SK_OUT8(IoC, GMAC_TI_ST_CTRL, TimeCtrl);

}	/* SkMacTimeStamp*/
#endif /* !SK_SLIM */

#ifdef SK_DIAG
/******************************************************************************
 *
 *	SkGmSendCont() - Enable / Disable Send Continuous Mode
 *
 * Description:	enable / disable Packet Generation on GPHY
 *
 * Returns:
 *	nothing
 */
void SkGmSendCont(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC,	/* I/O Context */
int		Port,	/* Port Index (MAC_1 + n) */
SK_BOOL	Enable)	/* Enable / Disable */
{
	SK_U16	Reg;
	SK_U16	PageReg;

	if (pAC->GIni.GIChipId == CHIP_ID_YUKON_EC) {
		/* select page 18 */
		SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PAGE_ADDR, 18);

		SkGmPhyRead(pAC, IoC, Port, PHY_MARV_PAGE_DATA, &Reg);

		Reg &= ~0x003c;			/* clear bits 5..2 */

		if (Enable) {
			/* enable packet generation, 1518 byte length */
			Reg |= (BIT_5S | BIT_3S);
		}

		SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PAGE_DATA, Reg);
	}
	else if (HW_HAS_NEWER_PHY(pAC)) {
		/* save page register */
		SkGmPhyRead(pAC, IoC, Port, PHY_MARV_EXT_ADR, &PageReg);

		/* select page 6 to access Packet Generation register */
		SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, 6);

		SkGmPhyRead(pAC, IoC, Port, PHY_MARV_PHY_CTRL, &Reg);

		Reg &= ~0x003f;			/* clear bits 5..0 */

		if (Enable) {
			/* enable packet generation, 1518 byte length */
			Reg |= (BIT_3S | BIT_1S);
		}

		SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PHY_CTRL, Reg);

		/* restore page register */
		SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, PageReg);
	}

}	/* SkGmSendCont */

/******************************************************************************
 *
 *	SkGmReadPhyTempSensor() - Read PHY's Temperature Sensor
 *
 * Description:	gets the Temperature from PHY's sensor
 *
 * Returns:
 *	0	o.k.
 *	1	PHY in reset
 */
int SkGmReadPhyTempSensor(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC,	/* I/O Context */
int		Port,	/* Port Index (MAC_1 + n) */
int		*pVal)	/* Pointer to Value */
{
	SK_U16	Word;
	SK_U16	Page;

	/* additional check for PHY reset */
	SK_IN16(IoC, MR_ADDR(Port, GPHY_CTRL), &Word);

	if ((Word & GPC_RST_CLR) == 0) {
		/* use old value */
		return(1);
	}

	/* save page register */
	SkGmPhyRead(pAC, IoC, Port, PHY_MARV_EXT_ADR, &Page);

	/* select page to access temperature sensor */
	SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, 6);

	/* read temperature */
	SkGmPhyRead(pAC, IoC, Port, PHY_MARV_EXT_CTRL_2, &Word);

	/* restore page register */
	SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, Page);

	*pVal = (Word & 0x1f) * 5;

	switch (pAC->GIni.GIChipId) {
	case CHIP_ID_YUKON_SUPR:
	case CHIP_ID_YUKON_UL_2:
		*pVal -= 25;
		break;
	case CHIP_ID_YUKON_OPT:
		*pVal -= 5;
		break;
	case CHIP_ID_YUKON_PRM:
		*pVal -= 15;
		break;
	}

	return(0);

}	/* SkGmReadPhyTempSensor */

#else /* !SK_DIAG */

/******************************************************************************
 *
 *	SkMacAutoNegLipaPhy() - Decides whether Link Partner could do auto-neg
 *
 *	This function analyses the PHY status word.
 *  If any of the Auto-negotiating bits are set, the PLipaAutoNeg variable
 *	is set true.
 */
void SkMacAutoNegLipaPhy(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port,		/* Port Index (MAC_1 + n) */
SK_U16	PhyStat)	/* PHY Status word to analyse */
{
	SK_GEPORT	*pPrt;

	pPrt = &pAC->GIni.GP[Port];

	if (pPrt->PLipaAutoNeg != SK_LIPA_AUTO &&
		(PhyStat & PHY_ST_AN_OVER) != 0) {

		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
			("AutoNegLipa: AutoNeg detected on Port %d, PhyStat = 0x%04X\n",
			Port, PhyStat));

		pPrt->PLipaAutoNeg = SK_LIPA_AUTO;
	}
}	/* SkMacAutoNegLipaPhy */


/******************************************************************************
 *
 *	SkGmIrq() - Interrupt Service Routine
 *
 * Description:	services an Interrupt Request of the GMAC
 *
 * Note:
 *
 * Returns:
 *	nothing
 */
static void SkGmIrq(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port)		/* Port Index (MAC_1 + n) */
{
	SK_U8		IStatus;	/* Interrupt status */
#if defined(SK_SLIM) || defined(SK_NO_PNMI)
	SK_U64		OverflowStatus;
#endif /* SK_SLIM || SK_NO_PNMI */
#ifndef SK_SLIM
	SK_GEPORT	*pPrt;
	SK_EVPARA	Para;

	pPrt = &pAC->GIni.GP[Port];
#endif /* !SK_SLIM */

	SK_IN8(IoC, MR_ADDR(Port, GMAC_IRQ_SRC), &IStatus);

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
		("GmacIrq Port %d Isr 0x%02X\n", Port, IStatus));

	/* Combined Tx & Rx Counter Overflow SIRQ Event */
	if (IStatus & (GM_IS_RX_CO_OV | GM_IS_TX_CO_OV)) {
		/* these IRQs will be cleared by reading GMACs register */
#if defined(SK_SLIM) || defined(SK_NO_PNMI)
		SkGmOverflowStatus(pAC, IoC, Port, (SK_U16)IStatus, &OverflowStatus);
#else /* !SK_SLIM && !SK_NO_PNMI */
		Para.Para32[0] = (SK_U32)Port;
		Para.Para32[1] = (SK_U32)IStatus;
		SkPnmiEvent(pAC, IoC, SK_PNMI_EVT_SIRQ_OVERFLOW, Para);
#endif /* !SK_SLIM && !SK_NO_PNMI */
	}

#ifndef SK_SLIM
	if (IStatus & GM_IS_RX_FF_OR) {
		/* clear GMAC Rx FIFO Overrun IRQ */
		SK_OUT8(IoC, MR_ADDR(Port, RX_GMF_CTRL_T), (SK_U8)GMF_CLI_RX_FO);

		Para.Para64 = Port;
		SkEventQueue(pAC, SKGE_DRV, SK_DRV_RX_OVERFLOW, Para);

#ifdef DEBUG
		pPrt->PRxOverCnt++;
#endif /* DEBUG */
	}
#endif /* !SK_SLIM */

	if (IStatus & GM_IS_TX_FF_UR) {
		/* clear GMAC Tx FIFO Underrun IRQ */
		SK_OUT8(IoC, MR_ADDR(Port, TX_GMF_CTRL_T), (SK_U8)GMF_CLI_TX_FU);

#ifndef SK_SLIM
		if (HW_FEATURE(pAC, HWF_WA_DEV_511)) {
			Para.Para32[0] = (SK_U32)Port;
			Para.Para32[1] = 0;
			SkEventQueue(pAC, SKGE_DRV, SK_DRV_TX_UNDERRUN, Para);
		}
		else if (pAC->GIni.GIChipId == CHIP_ID_YUKON_EC_U &&
			pPrt->PPortUsage == SK_JUMBO_LINK) {

			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_ERR,
				("%s\n", SKERR_SIRQ_E020MSG));
		}
		else {
			/* may NOT happen -> error log */
			SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E020, SKERR_SIRQ_E020MSG);
		}
#endif /* !SK_SLIM */
	}

#ifndef SK_SLIM
	if (IStatus & GM_IS_TX_COMPL) {
		/* Clear GMAC Frame Transmission Complete IRQ */
		SK_OUT8(IoC, MR_ADDR(Port, TX_GMF_CTRL_T), (SK_U8)GMF_CLI_TX_FC);
	}

	if (IStatus & GM_IS_RX_COMPL) {
		/* Clear GMAC Frame Reception Complete IRQ */
		SK_OUT8(IoC, MR_ADDR(Port, RX_GMF_CTRL_T), (SK_U8)GMF_CLI_RX_FC);
	}
#endif /* !SK_SLIM */
}	/* SkGmIrq */


/******************************************************************************
 *
 *	SkMacIrq() - Interrupt Service Routine for MAC
 *
 * Description:	calls the Interrupt Service Routine
 *
 * Returns:
 *	nothing
 */
void SkMacIrq(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port)		/* Port Index (MAC_1 + n) */
{
	/* IRQ from GMAC */
	SkGmIrq(pAC, IoC, Port);

}	/* SkMacIrq */

#endif /* !SK_DIAG */


#ifndef SK_SLIM
/******************************************************************************
 *
 *	SkGmUpdateStats() - Force the GMAC to output the current statistic
 *
 * Description:
 *	Empty function for GMAC. Statistic data is accessible in direct way.
 *
 * Returns:
 *	0:  success
 *	1:  something went wrong
 */
int SkGmUpdateStats(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
unsigned int Port)	/* Port Index (MAC_1 + n) */
{
	return(0);
}
#endif /* !SK_SLIM */


/******************************************************************************
 *
 *	SkGmMacStatistic() - Get GMAC counter value
 *
 * Description:
 *	Gets the 32bit counter value. Except for the octet counters
 *	the lower 32bit are counted in hardware and the upper 32bit
 *	must be counted in software by monitoring counter overflow interrupts.
 *
 * Returns:
 *	0:  success
 *	1:  something went wrong
 */
int SkGmMacStatistic(
SK_AC	*pAC,			/* Adapter Context */
SK_IOC	IoC,			/* I/O Context */
unsigned int Port,		/* Port Index (MAC_1 + n) */
SK_U16	StatAddr,		/* MIB counter base address */
SK_U32	SK_FAR *pVal)	/* Pointer to return statistic value */
{

	if ((StatAddr < GM_RXF_UC_OK) || (StatAddr > GM_TXE_FIFO_UR)) {

		SK_ERR_LOG(pAC, SK_ERRCL_SW, SKERR_HWI_E022, SKERR_HWI_E022MSG);

		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_ERR,
			("SkGmMacStat: wrong MIB counter 0x%04X\n", StatAddr));
		return(1);
	}

	GM_IN32(IoC, Port, StatAddr, pVal);

	return(0);
}	/* SkGmMacStatistic */


/******************************************************************************
 *
 *	SkGmResetCounter() - Clear MAC statistic counter
 *
 * Description:
 *	Force GMAC to clear its statistic counter.
 *
 * Returns:
 *	0:  success
 *	1:  something went wrong
 */
int SkGmResetCounter(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
unsigned int Port)	/* Port Index (MAC_1 + n) */
{
	SK_U16	Reg;	/* PHY Address Register */
	SK_U16	LoWord;
	SK_U16	HiWord;
	int		i;
	int		Addr;

	GM_IN16(IoC, Port, GM_PHY_ADDR, &Reg);

	/* set MIB Clear Counter Mode */
	GM_OUT16(IoC, Port, GM_PHY_ADDR, Reg | GM_PAR_MIB_CLR);

	/* read all MIB Counters with Clear Mode set */
	for (i = 0; i < GM_MIB_CNT_SIZE; i++) {

		Addr = GM_MIB_CNT_BASE + 8 * i;

		/* the reset is performed only when the lower 16 bits are read */
		GM_IN16(IoC, Port, Addr, &LoWord);

		/* check if MIB was cleared */
		GM_IN16(IoC, Port, Addr, &LoWord);
		GM_IN16(IoC, Port, Addr + 2, &HiWord);

		if (LoWord != 0 || HiWord != 0) {
			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_ERR,
				("SkGmResetCounter: MIB counter not cleared (0x%04X)\n", Addr));
		}
	}

	/* clear MIB Clear Counter Mode */
	GM_OUT16(IoC, Port, GM_PHY_ADDR, Reg);

	return(0);
}	/* SkGmResetCounter */


/******************************************************************************
 *
 *	SkGmOverflowStatus() - Gets the status of counter overflow interrupt
 *
 * Description:
 *	Checks the source causing an counter overflow interrupt. On success the
 *	resulting counter overflow status is written to <pStatus>, whereas the
 *	the following bit coding is used:
 *	63:56 - unused
 *	55:48 - TxRx interrupt register bit 7:0
 *	47:32 - Rx interrupt register
 *	31:24 - unused
 *	23:16 - TxRx interrupt register bit 15:8
 *	15: 0 - Tx interrupt register
 *
 * Returns:
 *	0:  success
 *	1:  something went wrong
 */
int SkGmOverflowStatus(
SK_AC	*pAC,				/* Adapter Context */
SK_IOC	IoC,				/* I/O Context */
unsigned int Port,			/* Port Index (MAC_1 + n) */
SK_U16	IStatus,			/* Interrupt Status from MAC */
SK_U64	SK_FAR *pStatus)	/* Pointer for return overflow status value */
{
	SK_U16	RegVal;
	SK_U64	Status;		/* Overflow status */

	Status = 0;

	if ((IStatus & GM_IS_RX_CO_OV) != 0) {
		/* this register is self-clearing after read */
		GM_IN16(IoC, Port, GM_RX_IRQ_SRC, &RegVal);

		Status |= (SK_U64)RegVal << 32;
	}

	if ((IStatus & GM_IS_TX_CO_OV) != 0) {
		/* this register is self-clearing after read */
		GM_IN16(IoC, Port, GM_TX_IRQ_SRC, &RegVal);

		Status |= (SK_U64)RegVal;
	}

	/* this register is self-clearing after read */
	GM_IN16(IoC, Port, GM_TR_IRQ_SRC, &RegVal);

	/* Rx overflow interrupt register bits (LoByte) */
	Status |= (SK_U64)((SK_U8)RegVal) << 48;
	/* Tx overflow interrupt register bits (HiByte) */
	Status |= (SK_U64)(RegVal >> 8) << 16;

	*pStatus = Status;

	return(0);
}	/* SkGmOverflowStatus */


#ifndef SK_SLIM
/******************************************************************************
 *
 *	SkGmCableDiagStatus() - Starts / Gets status of cable diagnostic test
 *
 * Description:
 *  starts the cable diagnostic test if 'StartTest' is true
 *  gets the results if 'StartTest' is true
 *
 * NOTE:	this test is meaningful only when link is down
 *
 * Returns:
 *	0:  success
 *	1:  no YUKON copper
 *	2:  test in progress
 *	3:  PHY read error
 */
int SkGmCableDiagStatus(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port,		/* Port Index (MAC_1 + n) */
SK_BOOL	StartTest)	/* flag for start / get result */
{
	SK_GEPORT	*pPrt;
	int			i;
	int			CableDiagOffs;
	int			MdiPairs;
	int			ChipId;
	int			Rtv;
	SK_BOOL		FastEthernet;
	SK_BOOL		NewPhyType;
	SK_BOOL		AdvancedVct;
	SK_U16		Ctrl;
	SK_U16		RegVal;

	pPrt = &pAC->GIni.GP[Port];

	if (pPrt->PhyType != SK_PHY_MARV_COPPER) {

		return(1);
	}

	ChipId = pAC->GIni.GIChipId;

	NewPhyType = HW_HAS_NEWER_PHY(pAC);

	AdvancedVct = ChipId == CHIP_ID_YUKON_EX || ChipId >= CHIP_ID_YUKON_SUPR;

	if (ChipId == CHIP_ID_YUKON_FE || ChipId == CHIP_ID_YUKON_FE_P) {

		CableDiagOffs = PHY_MARV_FE_VCT_TX;
		FastEthernet = SK_TRUE;
		MdiPairs = 2;
	}
	else {
		CableDiagOffs = AdvancedVct ? PHY_MARV_ADV_VCT_C :
			NewPhyType ? PHY_MARV_PHY_CTRL : PHY_MARV_CABLE_DIAG;

		FastEthernet = SK_FALSE;
		MdiPairs = 4;
	}

	if (StartTest) {
		/* set to RESET to avoid PortCheckUp */
		pPrt->PState = SK_PRT_RESET;

		/* only start the cable test */
		if (!FastEthernet) {
			/* check for PHY model 2, rev. < 4 */
			if ((((pPrt->PhyId1 & PHY_I1_MOD_NUM) >> 4) == 2) &&
				 ((pPrt->PhyId1 & PHY_I1_REV_MSK) < 4)) {
				/* apply TDR workaround */
				SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PAGE_ADDR, 30);

				SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PAGE_DATA, 0xcc00);
				SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PAGE_DATA, 0xc800);
				SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PAGE_DATA, 0xc400);
				SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PAGE_DATA, 0xc000);
				SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PAGE_DATA, 0xc100);
			}

#ifdef YUKON_DBG
			if (ChipId == CHIP_ID_YUKON_EC) {
				/* select page 1 */
				SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, 1);

				/* disable waiting period */
				SkGmPhyWrite(pAC, IoC, Port, CableDiagOffs,
					PHY_M_CABD_DIS_WAIT);
			}
#endif
			if (NewPhyType) {
				/* select page 5 */
				SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, 5);

#ifdef xYUKON_DBG
				/* disable waiting period */
				SkGmPhyWrite(pAC, IoC, Port, CableDiagOffs + 1,
					PHY_M_CABD_DIS_WAIT);
#endif
			}
			else {
				/* select page 0 for MDI[0] */
				SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, 0);
			}
		}
		else {
			RegVal = PHY_CT_RESET | PHY_CT_SP100;

			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_CTRL, RegVal);

#ifdef xYUKON_DBG
			SkGmPhyRead(pAC, IoC, Port, PHY_MARV_FE_SPEC_2, &RegVal);
			/* disable waiting period */
			RegVal |= PHY_M_FESC_DIS_WAIT;

			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_FE_SPEC_2, RegVal);
#endif
		}

		Rtv = SkGmPhyRead(pAC, IoC, Port, CableDiagOffs, &RegVal);

		if (Rtv != 0) {
			/* PHY read error */
			return(3);
		}

		/* enable Cable Diagnostic Test */
		RegVal |= PHY_M_CABD_ENA_TEST;

		if (AdvancedVct) {

			if (ChipId == CHIP_ID_YUKON_EX &&
				pAC->GIni.GIChipRev == CHIP_REV_YU_EX_A0) {
				/* apply TDR workaround */
				SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PAGE_ADDR, 13);

				SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PAGE_DATA, 0x4c32);
			}

			RegVal &= ~PHY_M_CABD_MODE_MSK;

			/* select first peak above threshold */
			RegVal |= PHY_M_CABD_TEST_MODE(1);

			/* read Transmit Pulse Reg */
			SkGmPhyRead(pAC, IoC, Port, CableDiagOffs + 5, &Ctrl);

			Ctrl &= ~PHY_M_CABD_PWID_MSK;

			/* select 1/2 pulse width (64ns) */
			Ctrl |= PHY_M_CABD_PULS_WIDT(2);

			SkGmPhyWrite(pAC, IoC, Port, CableDiagOffs + 5, Ctrl);
		}

		/* start Cable Diagnostic Test */
		SkGmPhyWrite(pAC, IoC, Port, CableDiagOffs, RegVal);

		return(0);
	}

	/* read Cable Diagnostic Reg */
	Rtv = SkGmPhyRead(pAC, IoC, Port, CableDiagOffs, &RegVal);

	if (Rtv != 0) {
		/* PHY read error */
		return(3);
	}

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("PHY Cable Diag. = 0x%04X\n", RegVal));

	if ((RegVal & PHY_M_CABD_ENA_TEST) != 0) {
		/* test is running */
		return(2);
	}

	if (AdvancedVct) {
		/* additional check */
		if ((RegVal & PHY_M_CABD_COMPLETE) == 0) {
			/* test not completed */
			return(2);
		}

		CableDiagOffs = PHY_MARV_PHY_CTRL;
	}

	/* get the test results */
	for (i = 0; i < MdiPairs; i++) {

		if (!FastEthernet && !NewPhyType) {
			/* set address to i for MDI[i] */
			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, (SK_U16)i);
		}

		/* get Cable Diagnostic values */
		SkGmPhyRead(pAC, IoC, Port, CableDiagOffs, &RegVal);

		pPrt->PMdiPairLen[i] = (SK_U8)(RegVal & PHY_M_CABD_DIST_MSK);

		pPrt->PMdiPairSts[i] = (SK_U8)(AdvancedVct ?
			(RegVal >> 8) : ((RegVal & PHY_M_CABD_STAT_MSK) >> 13));

		if (FastEthernet || NewPhyType) {
			/* get next register */
			CableDiagOffs++;
		}
	}

	return(0);
}	/* SkGmCableDiagStatus */
#endif /* !SK_SLIM */

/* End of file */

