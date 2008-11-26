/******************************************************************************
 *
 * Name:	skxmac2.c
 * Project:	Gigabit Ethernet Adapters, Common Modules
 * Version:	$Revision: 2.79 $
 * Date:	$Date: 2007/07/26 11:45:58 $
 * Purpose:	Contains functions to initialize the MACs and PHYs
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	LICENSE:
 *	(C)Copyright 1998-2002 SysKonnect.
 *	(C)Copyright 2002-2007 Marvell.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *	The information in this file is provided "AS IS" without warranty.
 *	/LICENSE
 *
 ******************************************************************************/

#include "h/skdrv1st.h"
#include "h/skdrv2nd.h"

/* typedefs *******************************************************************/

/* BCOM PHY magic pattern list */
typedef struct s_PhyHack {
	int		PhyReg;		/* PHY register */
	SK_U16	PhyVal;		/* Value to write */
} BCOM_HACK;

/* local variables ************************************************************/

#if (defined(DEBUG) || ((!defined(LINT)) && (!defined(SK_SLIM))))
static const char SysKonnectFileId[] =
	"@(#) $Id: skxmac2.c,v 2.79 2007/07/26 11:45:58 malthoff Exp $ (C) Marvell.";
#endif

#ifdef GENESIS
BCOM_HACK BcomRegA1Hack[] = {
 { 0x18, 0x0c20 }, { 0x17, 0x0012 }, { 0x15, 0x1104 }, { 0x17, 0x0013 },
 { 0x15, 0x0404 }, { 0x17, 0x8006 }, { 0x15, 0x0132 }, { 0x17, 0x8006 },
 { 0x15, 0x0232 }, { 0x17, 0x800D }, { 0x15, 0x000F }, { 0x18, 0x0420 },
 { 0, 0 }
};
BCOM_HACK BcomRegC0Hack[] = {
 { 0x18, 0x0c20 }, { 0x17, 0x0012 }, { 0x15, 0x1204 }, { 0x17, 0x0013 },
 { 0x15, 0x0A04 }, { 0x18, 0x0420 },
 { 0, 0 }
};
#endif

/* function prototypes ********************************************************/
#ifdef GENESIS
static void	SkXmInitPhyXmac(SK_AC*, SK_IOC, int, SK_BOOL);
static void	SkXmInitPhyBcom(SK_AC*, SK_IOC, int, SK_BOOL);
static int	SkXmAutoNegDoneXmac(SK_AC*, SK_IOC, int);
static int	SkXmAutoNegDoneBcom(SK_AC*, SK_IOC, int);
#endif /* GENESIS */
#ifdef YUKON
static void	SkGmInitPhyMarv(SK_AC*, SK_IOC, int, SK_BOOL);
static int	SkGmAutoNegDoneMarv(SK_AC*, SK_IOC, int);
#endif /* YUKON */
#ifdef OTHER_PHY
static void	SkXmInitPhyLone(SK_AC*, SK_IOC, int, SK_BOOL);
static void	SkXmInitPhyNat (SK_AC*, SK_IOC, int, SK_BOOL);
static int	SkXmAutoNegDoneLone(SK_AC*, SK_IOC, int);
static int	SkXmAutoNegDoneNat (SK_AC*, SK_IOC, int);
#endif /* OTHER_PHY */


#ifdef GENESIS
/******************************************************************************
 *
 *	SkXmPhyRead() - Read from XMAC PHY register
 *
 * Description:	reads a 16-bit word from XMAC PHY or ext. PHY
 *
 * Returns:
 *	nothing
 */
int SkXmPhyRead(
SK_AC	*pAC,			/* Adapter Context */
SK_IOC	IoC,			/* I/O Context */
int		Port,			/* Port Index (MAC_1 + n) */
int		PhyReg,			/* Register Address (Offset) */
SK_U16	SK_FAR *pVal)	/* Pointer to Value */
{
	SK_U16		Mmu;
	SK_GEPORT	*pPrt;

	pPrt = &pAC->GIni.GP[Port];

	/* write the PHY register's address */
	XM_OUT16(IoC, Port, XM_PHY_ADDR, PhyReg | pPrt->PhyAddr);

	/* get the PHY register's value */
	XM_IN16(IoC, Port, XM_PHY_DATA, pVal);

	if (pPrt->PhyType != SK_PHY_XMAC) {
		do {
			XM_IN16(IoC, Port, XM_MMU_CMD, &Mmu);
			/* wait until 'Ready' is set */
		} while ((Mmu & XM_MMU_PHY_RDY) == 0);

		/* get the PHY register's value */
		XM_IN16(IoC, Port, XM_PHY_DATA, pVal);
	}

	return(0);
}	/* SkXmPhyRead */


/******************************************************************************
 *
 *	SkXmPhyWrite() - Write to XMAC PHY register
 *
 * Description:	writes a 16-bit word to XMAC PHY or ext. PHY
 *
 * Returns:
 *	nothing
 */
int SkXmPhyWrite(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port,		/* Port Index (MAC_1 + n) */
int		PhyReg,		/* Register Address (Offset) */
SK_U16	Val)		/* Value */
{
	SK_U16		Mmu;
	SK_GEPORT	*pPrt;

	pPrt = &pAC->GIni.GP[Port];

	if (pPrt->PhyType != SK_PHY_XMAC) {
		do {
			XM_IN16(IoC, Port, XM_MMU_CMD, &Mmu);
			/* wait until 'Busy' is cleared */
		} while ((Mmu & XM_MMU_PHY_BUSY) != 0);
	}

	/* write the PHY register's address */
	XM_OUT16(IoC, Port, XM_PHY_ADDR, PhyReg | pPrt->PhyAddr);

	/* write the PHY register's value */
	XM_OUT16(IoC, Port, XM_PHY_DATA, Val);

	if (pPrt->PhyType != SK_PHY_XMAC) {
		do {
			XM_IN16(IoC, Port, XM_MMU_CMD, &Mmu);
			/* wait until 'Busy' is cleared */
		} while ((Mmu & XM_MMU_PHY_BUSY) != 0);
	}

	return(0);
}	/* SkXmPhyWrite */
#endif /* GENESIS */


#ifdef YUKON
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
	SK_U16	Word;
	SK_U16	Ctrl;
	SK_GEPORT	*pPrt;
	SK_U32	StartTime;
	SK_U32	CurrTime;
	SK_U32	Delta;
	SK_U32	TimeOut;
	int		Rtv;

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
	SK_IN32(IoC, GMAC_TI_ST_VAL, &StartTime);

	/* set timeout to 10 ms */
	TimeOut = HW_MS_TO_TICKS(pAC, 10);

	do {	/* wait until 'Busy' is cleared and 'ReadValid' is set */
#ifdef VCPU
		VCPUwaitTime(1000);
#endif /* VCPU */

		/* get current value of timestamp timer */
		SK_IN32(IoC, GMAC_TI_ST_VAL, &CurrTime);

		if (CurrTime >= StartTime) {
			Delta = CurrTime - StartTime;
		}
		else {
			Delta = CurrTime + ~StartTime + 1;
		}

		if (Delta > TimeOut) {

			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("SkGmPhyRead, Port: %d, Reg: %2d, Timeout (Ctrl=0x%04X)\n",
				 Port, PhyReg, Ctrl));

			Rtv = 2;
			break;
		}

		GM_IN16(IoC, Port, GM_SMI_CTRL, &Ctrl);

		/* Error on reading SMI Control Register */
		if (Ctrl == 0xffff) {
			return(1);
		}

	} while ((Ctrl ^ Word) != (GM_SMI_CT_RD_VAL | GM_SMI_CT_BUSY));

	GM_IN16(IoC, Port, GM_SMI_DATA, pVal);

	/* dummy read after GM_IN16() */
	SK_IN32(IoC, GMAC_TI_ST_VAL, &CurrTime);

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("SkGmPhyRead, Port: %d, Reg: %2d, Val = 0x%04X\n",
		 Port, PhyReg, *pVal));

#if defined(VCPU) || defined(SK_DIAG)
	c_print("Read 0x%04X from PHY reg %u on port %c\n",
		*pVal, PhyReg, 'A' + Port);
#endif  /* VCPU */

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
	SK_U16	Ctrl;
	SK_GEPORT	*pPrt;
	SK_U32	StartTime;
	SK_U32	CurrTime;
	SK_U32	Delta;
	SK_U32	TimeOut;

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
	SK_IN32(IoC, GMAC_TI_ST_VAL, &StartTime);

	/* set timeout to 10 ms */
	TimeOut = HW_MS_TO_TICKS(pAC, 10);

	do {	/* wait until 'Busy' is cleared */
#ifdef VCPU
		VCPUwaitTime(1000);
#endif /* VCPU */

		/* get current value of timestamp timer */
		SK_IN32(IoC, GMAC_TI_ST_VAL, &CurrTime);

		if (CurrTime >= StartTime) {
			Delta = CurrTime - StartTime;
		}
		else {
			Delta = CurrTime + ~StartTime + 1;
		}

		if (Delta > TimeOut) {

			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("PHY write timeout on Port %d (Ctrl=0x%04X)\n", Port, Ctrl));
			return(2);
		}

		GM_IN16(IoC, Port, GM_SMI_CTRL, &Ctrl);

		/* Error on reading SMI Control Register */
		if (Ctrl == 0xffff) {
			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("PHY write error on Port %d (Ctrl=0x%04X)\n", Port, Ctrl));
			return(1);
		}

	} while ((Ctrl & GM_SMI_CT_BUSY) != 0);

	/* dummy read after GM_IN16() */
	SK_IN32(IoC, GMAC_TI_ST_VAL, &CurrTime);

#if defined(VCPU) || defined(SK_DIAG)
	c_print("Wrote 0x%04X to PHY reg %u on port %c\n",
		Val, PhyReg, 'A' + Port);
#endif /* VCPU || SK_DIAG */

	return(0);
}	/* SkGmPhyWrite */
#endif /* YUKON */


#ifdef SK_DIAG
/******************************************************************************
 *
 *	SkGePhyRead() - Read from PHY register
 *
 * Description:	calls a read PHY routine dep. on board type
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
 * Description:	calls a write PHY routine dep. on board type
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
 *	enables / disables promiscuous mode by setting Mode Register (XMAC) or
 *	Receive Control Register (GMAC) dep. on board type
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
#ifdef YUKON
	SK_U16	RcReg;
#endif
#ifdef GENESIS
	SK_U32	SK_FAR MdReg;
#endif

#ifdef GENESIS
	if (pAC->GIni.GIGenesis) {

		XM_IN32(IoC, Port, XM_MODE, &MdReg);
		/* enable or disable promiscuous mode */
		if (Enable) {
			MdReg |= XM_MD_ENA_PROM;
		}
		else {
			MdReg &= ~XM_MD_ENA_PROM;
		}
		/* setup Mode Register */
		XM_OUT32(IoC, Port, XM_MODE, MdReg);
	}
#endif /* GENESIS */

#ifdef YUKON
	if (pAC->GIni.GIYukon) {

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
	}
#endif /* YUKON */

}	/* SkMacPromiscMode*/


/******************************************************************************
 *
 *	SkMacHashing() - Enable / Disable Hashing
 *
 * Description:
 *	enables / disables hashing by setting Mode Register (XMAC) or
 *	Receive Control Register (GMAC) dep. on board type
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
#ifdef YUKON
	SK_U16	RcReg;
#endif
#ifdef GENESIS
	SK_U32	SK_FAR MdReg;
#endif

#ifdef GENESIS
	if (pAC->GIni.GIGenesis) {

		XM_IN32(IoC, Port, XM_MODE, &MdReg);
		/* enable or disable hashing */
		if (Enable) {
			MdReg |= XM_MD_ENA_HASH;
		}
		else {
			MdReg &= ~XM_MD_ENA_HASH;
		}
		/* setup Mode Register */
		XM_OUT32(IoC, Port, XM_MODE, MdReg);
	}
#endif /* GENESIS */

#ifdef YUKON
	if (pAC->GIni.GIYukon) {

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
	}
#endif /* YUKON */

}	/* SkMacHashing*/


#ifdef SK_DIAG
/******************************************************************************
 *
 *	SkXmSetRxCmd() - Modify the value of the XMAC's Rx Command Register
 *
 * Description:
 *	The features
 *	 - FCS stripping,					SK_STRIP_FCS_ON/OFF
 *	 - pad byte stripping,				SK_STRIP_PAD_ON/OFF
 *	 - don't set XMR_FS_ERR in status	SK_LENERR_OK_ON/OFF
 *	   for inrange length error frames
 *	 - don't set XMR_FS_ERR in status	SK_BIG_PK_OK_ON/OFF
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
static void SkXmSetRxCmd(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port,		/* Port Index (MAC_1 + n) */
int		Mode)		/* Mode is SK_STRIP_FCS_ON/OFF, SK_STRIP_PAD_ON/OFF,
						SK_LENERR_OK_ON/OFF, or SK_BIG_PK_OK_ON/OFF */
{
	SK_U16	OldRxCmd;
	SK_U16	RxCmd;

	XM_IN16(IoC, Port, XM_RX_CMD, &OldRxCmd);

	RxCmd = OldRxCmd;

	switch (Mode & (SK_STRIP_FCS_ON | SK_STRIP_FCS_OFF)) {
	case SK_STRIP_FCS_ON:
		RxCmd |= XM_RX_STRIP_FCS;
		break;
	case SK_STRIP_FCS_OFF:
		RxCmd &= ~XM_RX_STRIP_FCS;
		break;
	}

	switch (Mode & (SK_STRIP_PAD_ON | SK_STRIP_PAD_OFF)) {
	case SK_STRIP_PAD_ON:
		RxCmd |= XM_RX_STRIP_PAD;
		break;
	case SK_STRIP_PAD_OFF:
		RxCmd &= ~XM_RX_STRIP_PAD;
		break;
	}

	switch (Mode & (SK_LENERR_OK_ON | SK_LENERR_OK_OFF)) {
	case SK_LENERR_OK_ON:
		RxCmd |= XM_RX_LENERR_OK;
		break;
	case SK_LENERR_OK_OFF:
		RxCmd &= ~XM_RX_LENERR_OK;
		break;
	}

	switch (Mode & (SK_BIG_PK_OK_ON | SK_BIG_PK_OK_OFF)) {
	case SK_BIG_PK_OK_ON:
		RxCmd |= XM_RX_BIG_PK_OK;
		break;
	case SK_BIG_PK_OK_OFF:
		RxCmd &= ~XM_RX_BIG_PK_OK;
		break;
	}

	switch (Mode & (SK_SELF_RX_ON | SK_SELF_RX_OFF)) {
	case SK_SELF_RX_ON:
		RxCmd |= XM_RX_SELF_RX;
		break;
	case SK_SELF_RX_OFF:
		RxCmd &= ~XM_RX_SELF_RX;
		break;
	}

	/* Write the new mode to the Rx command register if required */
	if (OldRxCmd != RxCmd) {
		XM_OUT16(IoC, Port, XM_RX_CMD, RxCmd);
	}
}	/* SkXmSetRxCmd */


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
 * Description:	modifies the MAC's Rx Control reg. dep. on board type
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
	if (pAC->GIni.GIGenesis) {

		SkXmSetRxCmd(pAC, IoC, Port, Mode);
	}
	else {

		SkGmSetRxCmd(pAC, IoC, Port, Mode);
	}

}	/* SkMacSetRxCmd */


/******************************************************************************
 *
 *	SkMacCrcGener() - Enable / Disable CRC Generation
 *
 * Description:	enables / disables CRC generation dep. on board type
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

	if (pAC->GIni.GIGenesis) {

		XM_IN16(IoC, Port, XM_TX_CMD, &Word);

		if (Enable) {
			Word &= ~XM_TX_NO_CRC;
		}
		else {
			Word |= XM_TX_NO_CRC;
		}
		/* setup Tx Command Register */
		XM_OUT16(IoC, Port, XM_TX_CMD, Word);
	}
	else {

		GM_IN16(IoC, Port, GM_TX_CTRL, &Word);

		if (Enable) {
			Word &= ~GM_TXCR_CRC_DIS;
		}
		else {
			Word |= GM_TXCR_CRC_DIS;
		}
		/* setup Tx Control Register */
		GM_OUT16(IoC, Port, GM_TX_CTRL, Word);
	}

}	/* SkMacCrcGener*/

#endif /* SK_DIAG */


#ifdef GENESIS
/******************************************************************************
 *
 *	SkXmClrExactAddr() - Clear Exact Match Address Registers
 *
 * Description:
 *	All Exact Match Address registers of the XMAC 'Port' will be
 *	cleared starting with 'StartNum' up to (and including) the
 *	Exact Match address number of 'StopNum'.
 *
 * Returns:
 *	nothing
 */
void SkXmClrExactAddr(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port,		/* Port Index (MAC_1 + n) */
int		StartNum,	/* Begin with this Address Register Index (0..15) */
int		StopNum)	/* Stop after finished with this Register Idx (0..15) */
{
	int		i;
	SK_U16	ZeroAddr[3] = {0, 0, 0};

	if ((unsigned)StartNum > 15 || (unsigned)StopNum > 15 ||
		StartNum > StopNum) {

		SK_ERR_LOG(pAC, SK_ERRCL_SW, SKERR_HWI_E001, SKERR_HWI_E001MSG);
		return;
	}

	for (i = StartNum; i <= StopNum; i++) {
		XM_OUTADDR(IoC, Port, XM_EXM(i), ZeroAddr);
	}
}	/* SkXmClrExactAddr */
#endif /* GENESIS */


/******************************************************************************
 *
 *	SkMacFlushTxFifo() - Flush the MAC's transmit FIFO
 *
 * Description:
 *	Flush the transmit FIFO of the MAC specified by the index 'Port'
 *
 * Returns:
 *	nothing
 */
void SkMacFlushTxFifo(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC,	/* I/O Context */
int		Port)	/* Port Index (MAC_1 + n) */
{
#ifdef GENESIS
	SK_U32	SK_FAR MdReg;

	if (pAC->GIni.GIGenesis) {

		XM_IN32(IoC, Port, XM_MODE, &MdReg);

		XM_OUT32(IoC, Port, XM_MODE, MdReg | XM_MD_FTF);
	}
#endif /* GENESIS */

#ifdef YUKON
	if (pAC->GIni.GIYukon) {
		/* no way to flush the FIFO we have to issue a reset */
		/* TBD */
	}
#endif /* YUKON */

}	/* SkMacFlushTxFifo */


/******************************************************************************
 *
 *	SkMacFlushRxFifo() - Flush the MAC's receive FIFO
 *
 * Description:
 *	Flush the receive FIFO of the MAC specified by the index 'Port'
 *
 * Returns:
 *	nothing
 */
void SkMacFlushRxFifo(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC,	/* I/O Context */
int		Port)	/* Port Index (MAC_1 + n) */
{
#ifdef GENESIS
	SK_U32	SK_FAR MdReg;

	if (pAC->GIni.GIGenesis) {

		XM_IN32(IoC, Port, XM_MODE, &MdReg);

		XM_OUT32(IoC, Port, XM_MODE, MdReg | XM_MD_FRF);
	}
#endif /* GENESIS */

#ifdef YUKON
	if (pAC->GIni.GIYukon) {
		/* no way to flush the FIFO we have to issue a reset */
		/* TBD */
	}
#endif /* YUKON */

}	/* SkMacFlushRxFifo */


#ifdef GENESIS
/******************************************************************************
 *
 *	SkXmSoftRst() - Do a XMAC software reset
 *
 * Description:
 *	The PHY registers should not be destroyed during this
 *	kind of software reset. Therefore the XMAC Software Reset
 *	(XM_GP_RES_MAC bit in XM_GP_PORT) must not be used!
 *
 *	The software reset is done by
 *		- disabling the Rx and Tx state machine,
 *		- resetting the statistics module,
 *		- clear all other significant XMAC Mode,
 *		  Command, and Control Registers
 *		- clearing the Hash Register and the
 *		  Exact Match Address registers, and
 *		- flushing the XMAC's Rx and Tx FIFOs.
 *
 * Note:
 *	Another requirement when stopping the XMAC is to
 *	avoid sending corrupted frames on the network.
 *	Disabling the Tx state machine will NOT interrupt
 *	the currently transmitted frame. But we must take care
 *	that the Tx FIFO is cleared AFTER the current frame
 *	is complete sent to the network.
 *
 *	It takes about 12ns to send a frame with 1538 bytes.
 *	One PCI clock goes at least 15ns (66MHz). Therefore
 *	after reading XM_GP_PORT back, we are sure that the
 *	transmitter is disabled AND idle. And this means
 *	we may flush the transmit FIFO now.
 *
 * Returns:
 *	nothing
 */
static void SkXmSoftRst(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC,	/* I/O Context */
int		Port)	/* Port Index (MAC_1 + n) */
{
	SK_U16	ZeroAddr[4] = {0, 0, 0, 0};

	/* reset the statistics module */
	XM_OUT32(IoC, Port, XM_GP_PORT, XM_GP_RES_STAT);

	/* disable all XMAC IRQs */
	XM_OUT16(IoC, Port, XM_IMSK, 0xffff);

	XM_OUT32(IoC, Port, XM_MODE, 0);		/* clear Mode Reg */

	XM_OUT16(IoC, Port, XM_TX_CMD, 0);		/* reset TX CMD Reg */
	XM_OUT16(IoC, Port, XM_RX_CMD, 0);		/* reset RX CMD Reg */

	/* disable all PHY IRQs */
	switch (pAC->GIni.GP[Port].PhyType) {
	case SK_PHY_BCOM:
			SkXmPhyWrite(pAC, IoC, Port, PHY_BCOM_INT_MASK, 0xffff);
			break;
#ifdef OTHER_PHY
		case SK_PHY_LONE:
			SkXmPhyWrite(pAC, IoC, Port, PHY_LONE_INT_ENAB, 0);
			break;
		case SK_PHY_NAT:
			/* todo: National
			 SkXmPhyWrite(pAC, IoC, Port, PHY_NAT_INT_MASK, 0xffff); */
			break;
#endif /* OTHER_PHY */
	}

	/* clear the Hash Register */
	XM_OUTHASH(IoC, Port, XM_HSM, ZeroAddr);

	/* clear the Exact Match Address registers */
	SkXmClrExactAddr(pAC, IoC, Port, 0, 15);

	/* clear the Source Check Address registers */
	XM_OUTHASH(IoC, Port, XM_SRC_CHK, ZeroAddr);

}	/* SkXmSoftRst */


/******************************************************************************
 *
 *	SkXmHardRst() - Do a XMAC hardware reset
 *
 * Description:
 *	The XMAC of the specified 'Port' and all connected devices
 *	(PHY and SERDES) will receive a reset signal on its *Reset pins.
 *	External PHYs must be reset by clearing a bit in the GPIO register
 *  (Timing requirements: Broadcom: 400ns, Level One: none, National: 80ns).
 *
 * ATTENTION:
 * 	It is absolutely necessary to reset the SW_RST Bit first
 *	before calling this function.
 *
 * Returns:
 *	nothing
 */
static void SkXmHardRst(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC,	/* I/O Context */
int		Port)	/* Port Index (MAC_1 + n) */
{
	SK_U32	Reg;
	int		i;
	int		TOut;
	SK_U16	Word;

	for (i = 0; i < 4; i++) {
		/* TX_MFF_CTRL1 has 32 bits, but only the lowest 16 bits are used */
		SK_OUT16(IoC, MR_ADDR(Port, TX_MFF_CTRL1), MFF_CLR_MAC_RST);

		TOut = 0;
		do {
			if (TOut++ > 10000) {
				/*
				 * Adapter seems to be in RESET state.
				 * Registers cannot be written.
				 */
				return;
			}

			SK_OUT16(IoC, MR_ADDR(Port, TX_MFF_CTRL1), MFF_SET_MAC_RST);

			SK_IN16(IoC, MR_ADDR(Port, TX_MFF_CTRL1), &Word);

		} while ((Word & MFF_SET_MAC_RST) == 0);
	}

	/* For external PHYs there must be special handling */
	if (pAC->GIni.GP[Port].PhyType != SK_PHY_XMAC) {

		SK_IN32(IoC, B2_GP_IO, &Reg);

		if (Port == 0) {
			Reg |= GP_DIR_0;	/* set to output */
			Reg &= ~GP_IO_0;	/* set PHY reset (active low) */
		}
		else {
			Reg |= GP_DIR_2;	/* set to output */
			Reg &= ~GP_IO_2;	/* set PHY reset (active low) */
		}
		/* reset external PHY */
		SK_OUT32(IoC, B2_GP_IO, Reg);

		/* short delay */
		SK_IN32(IoC, B2_GP_IO, &Reg);
	}
}	/* SkXmHardRst */


/******************************************************************************
 *
 *	SkXmClearRst() - Release the PHY & XMAC reset
 *
 * Description:
 *
 * Returns:
 *	nothing
 */
static void SkXmClearRst(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC,	/* I/O Context */
int		Port)	/* Port Index (MAC_1 + n) */
{
	SK_U32	DWord;

	/* clear HW reset */
	SK_OUT16(IoC, MR_ADDR(Port, TX_MFF_CTRL1), MFF_CLR_MAC_RST);

	if (pAC->GIni.GP[Port].PhyType != SK_PHY_XMAC) {

		SK_IN32(IoC, B2_GP_IO, &DWord);

		if (Port == 0) {
			DWord |= (GP_DIR_0 | GP_IO_0); /* set to output */
		}
		else {
			DWord |= (GP_DIR_2 | GP_IO_2); /* set to output */
		}
		/* Clear PHY reset */
		SK_OUT32(IoC, B2_GP_IO, DWord);

		/* enable GMII interface */
		XM_OUT16(IoC, Port, XM_HW_CFG, XM_HW_GMII_MD);
	}
}	/* SkXmClearRst */
#endif /* GENESIS */


#ifdef YUKON
/******************************************************************************
 *
 *	SkGmSoftRst() - Do a GMAC software reset
 *
 * Description:
 *	The GPHY registers should not be destroyed during this
 *	kind of software reset.
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

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("SkGmHardRst: Port %d\n", Port));

	/* WA code for COMA mode */
	if (pAC->GIni.GIYukonLite &&
		pAC->GIni.GIChipRev >= CHIP_REV_YU_LITE_A3) {

		SK_IN32(IoC, B2_GP_IO, &DWord);

		DWord |= (GP_DIR_9 | GP_IO_9);

		/* set PHY reset */
		SK_OUT32(IoC, B2_GP_IO, DWord);
	}

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
#ifdef SK_DIAG
	SK_U16	PhyId0;
	SK_U16	PhyId1;
	SK_U16	Word;
#endif /* SK_DIAG */

#ifdef DEBUG
	SK_U8	Byte;
#endif /* DEBUG */

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("SkGmClearRst: Port %d\n", Port));

	/* WA code for COMA mode */
	if (pAC->GIni.GIYukonLite &&
		pAC->GIni.GIChipRev >= CHIP_REV_YU_LITE_A3) {

		SK_IN32(IoC, B2_GP_IO, &DWord);

		DWord |= GP_DIR_9;		/* set to output */
		DWord &= ~GP_IO_9;		/* clear PHY reset (active high) */

		/* clear PHY reset */
		SK_OUT32(IoC, B2_GP_IO, DWord);
	}

#ifdef VCPU
	/* set MAC Reset before PHY reset is set */
	SK_OUT8(IoC, MR_ADDR(Port, GMAC_CTRL), (SK_U8)GMC_RST_SET);
#endif /* VCPU */

	if (CHIP_ID_YUKON_2(pAC)) {
		/* set GPHY Control reset */
		SK_OUT8(IoC, MR_ADDR(Port, GPHY_CTRL), (SK_U8)GPC_RST_SET);

		/* release GPHY Control reset */
		SK_OUT8(IoC, MR_ADDR(Port, GPHY_CTRL), (SK_U8)GPC_RST_CLR);

#ifdef DEBUG
		/* additional check for PEX */
		SK_IN8(IoC, GPHY_CTRL, &Byte);

		Byte &= GPC_RST_CLR | GPC_RST_SET;

		if (pAC->GIni.GIPciBus == SK_PEX_BUS && Byte != GPC_RST_CLR) {

			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_ERR,
				("Error on PEX-bus after GPHY reset (GPHY Ctrl=0x%02X)\n",
				Byte));
		}
#endif /* DEBUG */
	}
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

#ifdef VCPU
	/* wait for internal initialization of GPHY */
	VCPUprintf(0, "Waiting until PHY %d is ready to initialize\n", Port);
	VCpuWait(10000);
#endif /* VCPU */

	/* clear GMAC Control reset */
	SK_OUT8(IoC, MR_ADDR(Port, GMAC_CTRL), (SK_U8)GMC_RST_CLR);

#ifdef VCPU
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

#ifdef VCPU
	VCpuWait(2000);

	SK_IN32(IoC, MR_ADDR(Port, GPHY_CTRL), &DWord);

	SK_IN32(IoC, B0_ISRC, &DWord);
#endif /* VCPU */

}	/* SkGmClearRst */
#endif /* YUKON */


/******************************************************************************
 *
 *	SkMacSoftRst() - Do a MAC software reset
 *
 * Description:	calls a MAC software reset routine dep. on board type
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

#ifdef GENESIS
	if (pAC->GIni.GIGenesis) {

		SkXmSoftRst(pAC, IoC, Port);
	}
#endif /* GENESIS */

#ifdef YUKON
	if (pAC->GIni.GIYukon) {

		SkGmSoftRst(pAC, IoC, Port);
	}
#endif /* YUKON */

	/* flush the MAC's Rx and Tx FIFOs */
	SkMacFlushTxFifo(pAC, IoC, Port);

	SkMacFlushRxFifo(pAC, IoC, Port);

	pAC->GIni.GP[Port].PState = SK_PRT_STOP;

}	/* SkMacSoftRst */


/******************************************************************************
 *
 *	SkMacHardRst() - Do a MAC hardware reset
 *
 * Description:	calls a MAC hardware reset routine dep. on board type
 *
 * Returns:
 *	nothing
 */
void SkMacHardRst(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC,	/* I/O Context */
int		Port)	/* Port Index (MAC_1 + n) */
{

#ifdef GENESIS
	if (pAC->GIni.GIGenesis) {

		SkXmHardRst(pAC, IoC, Port);
	}
#endif /* GENESIS */

#ifdef YUKON
	if (pAC->GIni.GIYukon) {

		SkGmHardRst(pAC, IoC, Port);
	}
#endif /* YUKON */

	pAC->GIni.GP[Port].PHWLinkUp = SK_FALSE;

	pAC->GIni.GP[Port].PState = SK_PRT_RESET;

}	/* SkMacHardRst */

#ifndef SK_SLIM
/******************************************************************************
 *
 *	SkMacClearRst() - Clear the MAC reset
 *
 * Description:	calls a clear MAC reset routine dep. on board type
 *
 * Returns:
 *	nothing
 */
void SkMacClearRst(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC,	/* I/O Context */
int		Port)	/* Port Index (MAC_1 + n) */
{

#ifdef GENESIS
	if (pAC->GIni.GIGenesis) {

		SkXmClearRst(pAC, IoC, Port);
	}
#endif /* GENESIS */

#ifdef YUKON
	if (pAC->GIni.GIYukon) {

		SkGmClearRst(pAC, IoC, Port);
	}
#endif /* YUKON */

}	/* SkMacClearRst */
#endif /* !SK_SLIM */

#ifdef GENESIS
/******************************************************************************
 *
 *	SkXmInitMac() - Initialize the XMAC II
 *
 * Description:
 *	Initialize the XMAC of the specified port.
 *	The XMAC must be reset or stopped before calling this function.
 *
 * Note:
 *	The XMAC's Rx and Tx state machine is still disabled when returning.
 *
 * Returns:
 *	nothing
 */
void SkXmInitMac(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port)		/* Port Index (MAC_1 + n) */
{
	SK_GEPORT	*pPrt;
	int			i;
	SK_U16		SWord;

	pPrt = &pAC->GIni.GP[Port];

	if (pPrt->PState == SK_PRT_STOP) {
		/* Verify that the reset bit is cleared */
		SK_IN16(IoC, MR_ADDR(Port, TX_MFF_CTRL1), &SWord);

		if ((SWord & MFF_SET_MAC_RST) != 0) {
			/* PState does not match HW state */
			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_INIT,
				("SkXmInitMac: PState does not match HW state"));
			/* Correct it */
			pPrt->PState = SK_PRT_RESET;
		}
	}

	if (pPrt->PState == SK_PRT_RESET) {

		SkXmClearRst(pAC, IoC, Port);

		if (pPrt->PhyType != SK_PHY_XMAC) {
			/* read Id from external PHY (all have the same address) */
			SkXmPhyRead(pAC, IoC, Port, PHY_XMAC_ID1, &pPrt->PhyId1);

			/*
			 * Optimize MDIO transfer by suppressing preamble.
			 * Must be done AFTER first access to BCOM chip.
			 */
			XM_IN16(IoC, Port, XM_MMU_CMD, &SWord);

			XM_OUT16(IoC, Port, XM_MMU_CMD, SWord | XM_MMU_NO_PRE);

			if (pPrt->PhyId1 == PHY_BCOM_ID1_C0) {
				/*
				 * Workaround BCOM Errata for the C0 type.
				 * Write magic patterns to reserved registers.
				 */
				i = 0;
				while (BcomRegC0Hack[i].PhyReg != 0) {
					SkXmPhyWrite(pAC, IoC, Port, BcomRegC0Hack[i].PhyReg,
						BcomRegC0Hack[i].PhyVal);
					i++;
				}
			}
			else if (pPrt->PhyId1 == PHY_BCOM_ID1_A1) {
				/*
				 * Workaround BCOM Errata for the A1 type.
				 * Write magic patterns to reserved registers.
				 */
				i = 0;
				while (BcomRegA1Hack[i].PhyReg != 0) {
					SkXmPhyWrite(pAC, IoC, Port, BcomRegA1Hack[i].PhyReg,
						BcomRegA1Hack[i].PhyVal);
					i++;
				}
			}

			/*
			 * Workaround BCOM Errata (#10523) for all BCom PHYs.
			 * Disable Power Management after reset.
			 */
			SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_AUX_CTRL, &SWord);

			SkXmPhyWrite(pAC, IoC, Port, PHY_BCOM_AUX_CTRL,
				(SK_U16)(SWord | PHY_B_AC_DIS_PM));

			/* PHY LED initialization is done in SkGeXmitLED() */
		}

		/* Dummy read the Interrupt source register */
		XM_IN16(IoC, Port, XM_ISRC, &SWord);

		/*
		 * The auto-negotiation process starts immediately after
		 * clearing the reset. The auto-negotiation process should be
		 * started by the SIRQ, therefore stop it here immediately.
		 */
		SkMacInitPhy(pAC, IoC, Port, SK_FALSE);
	}

	/*
	 * configure the XMACs Station Address
	 * B2_MAC_2 = xx xx xx xx xx x1 is programmed to XMAC A
	 * B2_MAC_3 = xx xx xx xx xx x2 is programmed to XMAC B
	 */
	for (i = 0; i < 3; i++) {
		/*
		 * The following 2 statements are together endianess
		 * independent. Remember this when changing.
		 */
		SK_IN16(IoC, (B2_MAC_2 + Port * 8 + i * 2), &SWord);

		XM_OUT16(IoC, Port, (XM_SA + i * 2), SWord);
	}

	/* Tx Inter Packet Gap (XM_TX_IPG):	use default */
	/* Tx High Water Mark (XM_TX_HI_WM):	use default */
	/* Tx Low Water Mark (XM_TX_LO_WM):	use default */
	/* Host Request Threshold (XM_HT_THR):	use default */
	/* Rx Request Threshold (XM_RX_THR):	use default */
	/* Rx Low Water Mark (XM_RX_LO_WM):	use default */

	/* configure Rx High Water Mark (XM_RX_HI_WM) */
	XM_OUT16(IoC, Port, XM_RX_HI_WM, SK_XM_RX_HI_WM);

	/* Configure Tx Request Threshold */
	SWord = SK_XM_THR_SL;				/* for single port */

	if (pAC->GIni.GIMacsFound > 1) {
		switch (pPrt->PPortUsage) {
		case SK_RED_LINK:
			SWord = SK_XM_THR_REDL;		/* redundant link */
			break;
		case SK_MUL_LINK:
			SWord = SK_XM_THR_MULL;		/* load balancing */
			break;
		case SK_JUMBO_LINK:
			SWord = SK_XM_THR_JUMBO;	/* jumbo frames */
			break;
		default:
			SK_ERR_LOG(pAC, SK_ERRCL_SW, SKERR_HWI_E014, SKERR_HWI_E014MSG);
			break;
		}
	}
	XM_OUT16(IoC, Port, XM_TX_THR, SWord);

	/* setup register defaults for the Tx Command Register */
	XM_OUT16(IoC, Port, XM_TX_CMD, XM_TX_AUTO_PAD);

	/* setup register defaults for the Rx Command Register */
	SWord = XM_RX_STRIP_FCS | XM_RX_LENERR_OK;

	if (pPrt->PPortUsage == SK_JUMBO_LINK) {
		SWord |= XM_RX_BIG_PK_OK;
	}

	if (pPrt->PLinkMode == SK_LMODE_HALF) {
		/*
		 * If in manual half duplex mode the other side might be in
		 * full duplex mode, so ignore if a carrier extension is not seen
		 * on frames received
		 */
		SWord |= XM_RX_DIS_CEXT;
	}

	XM_OUT16(IoC, Port, XM_RX_CMD, SWord);

	/*
	 * setup register defaults for the Mode Register
	 *	- Don't strip error frames to avoid Store & Forward
	 *	  on the Rx side.
	 *	- Enable 'Check Station Address' bit
	 *	- Enable 'Check Address Array' bit
	 */
	XM_OUT32(IoC, Port, XM_MODE, XM_DEF_MODE);

	/*
	 * Initialize the Receive Counter Event Mask (XM_RX_EV_MSK)
	 *	- Enable all bits excepting 'Octets Rx OK Low CntOv'
	 *	  and 'Octets Rx OK Hi Cnt Ov'.
	 */
	XM_OUT32(IoC, Port, XM_RX_EV_MSK, XMR_DEF_MSK);

	/*
	 * Initialize the Transmit Counter Event Mask (XM_TX_EV_MSK)
	 *	- Enable all bits excepting 'Octets Tx OK Low CntOv'
	 *	  and 'Octets Tx OK Hi Cnt Ov'.
	 */
	XM_OUT32(IoC, Port, XM_TX_EV_MSK, XMT_DEF_MSK);

	/*
	 * Do NOT init XMAC interrupt mask here.
	 * All interrupts remain disable until link comes up!
	 */

	/*
	 * Any additional configuration changes may be done now.
	 * The last action is to enable the Rx and Tx state machine.
	 * This should be done after the auto-negotiation process
	 * has been completed successfully.
	 */
}	/* SkXmInitMac */
#endif /* GENESIS */


#ifdef YUKON
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
	SK_U32		DWord;

	pPrt = &pAC->GIni.GP[Port];

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("SkGmInitMac: Port %d, PState = %d, PLinkSpeed = %d\n",
		 Port, pPrt->PState, pPrt->PLinkSpeed));

	if (pPrt->PState == SK_PRT_STOP) {
		/* Verify that the reset bit is cleared */
		SK_IN32(IoC, MR_ADDR(Port, GMAC_CTRL), &DWord);

		if ((DWord & GMC_RST_SET) != 0) {
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

		/* Auto-negotiation ? */
		if (pPrt->PLinkMode == SK_LMODE_HALF ||
			 pPrt->PLinkMode == SK_LMODE_FULL) {
			/* Auto-negotiation disabled */

			/* get General Purpose Control */
			GM_IN16(IoC, Port, GM_GP_CTRL, &SWord);

			/* disable auto-update for speed, duplex and flow-control */
			SWord |= GM_GPCR_AU_ALL_DIS;

			/* setup General Purpose Control Register */
			GM_OUT16(IoC, Port, GM_GP_CTRL, SWord);

			SWord = GM_GPCR_AU_ALL_DIS;
		}
		else {
			SWord = 0;
		}

		/* speed settings */
		switch (pPrt->PLinkSpeed) {
		case SK_LSPEED_AUTO:
		case SK_LSPEED_1000MBPS:
			if ((pPrt->PLinkSpeedCap & SK_LSPEED_CAP_1000MBPS) != 0) {

				SWord |= GM_GPCR_SPEED_1000 | GM_GPCR_SPEED_100;
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

		/* setup General Purpose Control Register */
		GM_OUT16(IoC, Port, GM_GP_CTRL, SWord);

		/* dummy read the Interrupt Source Register */
		SK_IN16(IoC, MR_ADDR(Port, GMAC_IRQ_SRC), &SWord);

#ifndef VCPU
		SkGmInitPhyMarv(pAC, IoC, Port, SK_FALSE);
#endif /* !VCPU */
	}

#ifndef VCPU	/* saves some time in co-sim */
	(void)SkGmResetCounter(pAC, IoC, Port);
#endif

	/* setup Transmit Control Register */
	GM_OUT16(IoC, Port, GM_TX_CTRL, (SK_U16)TX_COL_THR(pPrt->PMacColThres));

	/* setup Receive Control Register */
	SWord = GM_RXCR_UCF_ENA | GM_RXCR_MCF_ENA | GM_RXCR_CRC_DIS;

	GM_OUT16(IoC, Port, GM_RX_CTRL, SWord);

	/* setup Transmit Flow Control Register */
	GM_OUT16(IoC, Port, GM_TX_FLOW_CTRL, 0xffff);

	/* setup Transmit Parameter Register */
#ifdef VCPU
	GM_IN16(IoC, Port, GM_TX_PARAM, &SWord);
#endif /* VCPU */

	SWord = (SK_U16)(TX_JAM_LEN_VAL(pPrt->PMacJamLen) |
		TX_JAM_IPG_VAL(pPrt->PMacJamIpgVal) |
		TX_IPG_JAM_DATA(pPrt->PMacJamIpgData) |
		TX_BACK_OFF_LIM(pPrt->PMacBackOffLim));

	GM_OUT16(IoC, Port, GM_TX_PARAM, SWord);

	/* configure the Serial Mode Register */
	SWord = (SK_U16)(DATA_BLIND_VAL(pPrt->PMacDataBlind) |
		GM_SMOD_VLAN_ENA | IPG_DATA_VAL(pPrt->PMacIpgData));

	if (pPrt->PMacLimit4) {
		/* reset of collision counter after 4 consecutive collisions */
		SWord |= GM_SMOD_LIMIT_4;
	}

	if (pPrt->PPortUsage == SK_JUMBO_LINK) {
		/* enable jumbo mode (Max. Frame Length = 9018) */
		SWord |= GM_SMOD_JUMBO_ENA;
	}

	GM_OUT16(IoC, Port, GM_SERIAL_MODE, SWord);

	/*
	 * configure the GMACs Station Addresses
	 * in PROM you can find our addresses at:
	 * B2_MAC_1 = xx xx xx xx xx x0 virtual address
	 * B2_MAC_2 = xx xx xx xx xx x1 is programmed to GMAC A
	 * B2_MAC_3 = xx xx xx xx xx x2 is reserved for DualPort
	 */

	for (i = 0; i < 3; i++) {
		/*
		 * The following 2 statements are together endianess
		 * independent. Remember this when changing.
		 */
		/* physical address: will be used for pause frames */
		SK_IN16(IoC, (B2_MAC_2 + Port * 8 + i * 2), &SWord);

#ifdef WA_DEV_16
		/* WA for deviation #16 */
		if (pAC->GIni.GIChipId == CHIP_ID_YUKON && pAC->GIni.GIChipRev == 0) {
			/* swap the address bytes */
			SWord = ((SWord & 0xff00) >> 8)	| ((SWord & 0x00ff) << 8);

			/* write to register in reversed order */
			GM_OUT16(IoC, Port, (GM_SRC_ADDR_1L + (2 - i) * 4), SWord);
		}
		else {
			GM_OUT16(IoC, Port, (GM_SRC_ADDR_1L + i * 4), SWord);
		}
#else
		GM_OUT16(IoC, Port, (GM_SRC_ADDR_1L + i * 4), SWord);
#endif /* WA_DEV_16 */

		/* virtual address: will be used for data */
		SK_IN16(IoC, (B2_MAC_1 + Port * 8 + i * 2), &SWord);

		GM_OUT16(IoC, Port, (GM_SRC_ADDR_2L + i * 4), SWord);

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
#endif /* YUKON */


#ifdef GENESIS
/******************************************************************************
 *
 *	SkXmInitDupMd() - Initialize the XMACs Duplex Mode
 *
 * Description:
 *	This function initializes the XMACs Duplex Mode.
 *	It should be called after successfully finishing
 *	the Auto-negotiation Process
 *
 * Returns:
 *	nothing
 */
void SkXmInitDupMd(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port)		/* Port Index (MAC_1 + n) */
{
	switch (pAC->GIni.GP[Port].PLinkModeStatus) {
	case SK_LMODE_STAT_AUTOHALF:
	case SK_LMODE_STAT_HALF:
		/* Configuration Actions for Half Duplex Mode */
		/*
		 * XM_BURST = default value. We are probable not quick
		 * 	enough at the 'XMAC' bus to burst 8kB.
		 *	The XMAC stops bursting if no transmit frames
		 *	are available or the burst limit is exceeded.
		 */
		/* XM_TX_RT_LIM = default value (15) */
		/* XM_TX_STIME = default value (0xff = 4096 bit times) */
		break;
	case SK_LMODE_STAT_AUTOFULL:
	case SK_LMODE_STAT_FULL:
		/* Configuration Actions for Full Duplex Mode */
		/*
		 * The duplex mode is configured by the PHY,
		 * therefore it seems to be that there is nothing
		 * to do here.
		 */
		break;
	case SK_LMODE_STAT_UNKNOWN:
	default:
		SK_ERR_LOG(pAC, SK_ERRCL_SW, SKERR_HWI_E007, SKERR_HWI_E007MSG);
		break;
	}
}	/* SkXmInitDupMd */


/******************************************************************************
 *
 *	SkXmInitPauseMd() - initialize the Pause Mode to be used for this port
 *
 * Description:
 *	This function initializes the Pause Mode which should
 *	be used for this port.
 *	It should be called after successfully finishing
 *	the Auto-negotiation Process
 *
 * Returns:
 *	nothing
 */
void SkXmInitPauseMd(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port)		/* Port Index (MAC_1 + n) */
{
	SK_GEPORT	*pPrt;
	SK_U32	SK_FAR DWord;
	SK_U16		Word;

	pPrt = &pAC->GIni.GP[Port];

	XM_IN16(IoC, Port, XM_MMU_CMD, &Word);

	if (pPrt->PFlowCtrlStatus == SK_FLOW_STAT_NONE ||
		pPrt->PFlowCtrlStatus == SK_FLOW_STAT_LOC_SEND) {

		/* disable Pause Frame Reception */
		Word |= XM_MMU_IGN_PF;
	}
	else {
		/*
		 * enabling pause frame reception is required for 1000BT
		 * because the XMAC is not reset if the link is going down
		 */
		/* enable Pause Frame Reception */
		Word &= ~XM_MMU_IGN_PF;
	}

	XM_OUT16(IoC, Port, XM_MMU_CMD, Word);

	XM_IN32(IoC, Port, XM_MODE, &DWord);

	if (pPrt->PFlowCtrlStatus == SK_FLOW_STAT_SYMMETRIC ||
		pPrt->PFlowCtrlStatus == SK_FLOW_STAT_LOC_SEND) {

		/*
		 * Configure Pause Frame Generation
		 * Use internal and external Pause Frame Generation.
		 * Sending pause frames is edge triggered.
		 * Send a Pause frame with the maximum pause time if
		 * internal oder external FIFO full condition occurs.
		 * Send a zero pause time frame to re-start transmission.
		 */

		/* XM_PAUSE_DA = '010000C28001' (default) */

		/* XM_MAC_PTIME = 0xffff (maximum) */
		/* remember this value is defined in big endian (!) */
		XM_OUT16(IoC, Port, XM_MAC_PTIME, 0xffff);

		/* set Pause Mode in Mode Register */
		DWord |= XM_PAUSE_MODE;

		/* set Pause Mode in MAC Rx FIFO */
		SK_OUT16(IoC, MR_ADDR(Port, RX_MFF_CTRL1), MFF_ENA_PAUSE);
	}
	else {
		/*
		 * disable pause frame generation is required for 1000BT
		 * because the XMAC is not reset if the link is going down
		 */
		/* disable Pause Mode in Mode Register */
		DWord &= ~XM_PAUSE_MODE;

		/* disable Pause Mode in MAC Rx FIFO */
		SK_OUT16(IoC, MR_ADDR(Port, RX_MFF_CTRL1), MFF_DIS_PAUSE);
	}

	XM_OUT32(IoC, Port, XM_MODE, DWord);
}	/* SkXmInitPauseMd*/


/******************************************************************************
 *
 *	SkXmInitPhyXmac() - Initialize the XMAC PHY registers
 *
 * Description:	initializes all the XMACs PHY registers
 *
 * Note:
 *
 * Returns:
 *	nothing
 */
static void SkXmInitPhyXmac(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port,		/* Port Index (MAC_1 + n) */
SK_BOOL	DoLoop)		/* Should a PHY LoopBack be set-up? */
{
	SK_GEPORT	*pPrt;
	SK_U16		Ctrl;

	pPrt = &pAC->GIni.GP[Port];
	Ctrl = 0;

	/* Auto-negotiation ? */
	if (pPrt->PLinkMode == SK_LMODE_HALF || pPrt->PLinkMode == SK_LMODE_FULL) {
		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
			("InitPhyXmac: no auto-negotiation Port %d\n", Port));
		/* set DuplexMode in Config register */
		if (pPrt->PLinkMode == SK_LMODE_FULL) {
			Ctrl |= PHY_CT_DUP_MD;
		}

		/*
		 * Do NOT enable Auto-negotiation here. This would hold
		 * the link down because no IDLEs are transmitted
		 */
	}
	else {
		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
			("InitPhyXmac: with auto-negotiation Port %d\n", Port));
		/* set Auto-negotiation advertisement */

		/* set Full/half duplex capabilities */
		switch (pPrt->PLinkMode) {
		case SK_LMODE_AUTOHALF:
			Ctrl |= PHY_X_AN_HD;
			break;
		case SK_LMODE_AUTOFULL:
			Ctrl |= PHY_X_AN_FD;
			break;
		case SK_LMODE_AUTOBOTH:
			Ctrl |= PHY_X_AN_FD | PHY_X_AN_HD;
			break;
		default:
			SK_ERR_LOG(pAC, SK_ERRCL_SW | SK_ERRCL_INIT, SKERR_HWI_E015,
				SKERR_HWI_E015MSG);
		}

		/* set Flow-control capabilities */
		switch (pPrt->PFlowCtrlMode) {
		case SK_FLOW_MODE_NONE:
			Ctrl |= PHY_X_P_NO_PAUSE;
			break;
		case SK_FLOW_MODE_LOC_SEND:
			Ctrl |= PHY_X_P_ASYM_MD;
			break;
		case SK_FLOW_MODE_SYMMETRIC:
			Ctrl |= PHY_X_P_SYM_MD;
			break;
		case SK_FLOW_MODE_SYM_OR_REM:
			Ctrl |= PHY_X_P_BOTH_MD;
			break;
		default:
			SK_ERR_LOG(pAC, SK_ERRCL_SW | SK_ERRCL_INIT, SKERR_HWI_E016,
				SKERR_HWI_E016MSG);
		}

		/* Write AutoNeg Advertisement Register */
		SkXmPhyWrite(pAC, IoC, Port, PHY_XMAC_AUNE_ADV, Ctrl);

		/* Restart Auto-negotiation */
		Ctrl = PHY_CT_ANE | PHY_CT_RE_CFG;
	}

	if (DoLoop) {
		/* set the PHY Loopback bit, too */
		Ctrl |= PHY_CT_LOOP;
	}

	/* Write to the PHY control register */
	SkXmPhyWrite(pAC, IoC, Port, PHY_XMAC_CTRL, Ctrl);
}	/* SkXmInitPhyXmac */


/******************************************************************************
 *
 *	SkXmInitPhyBcom() - Initialize the Broadcom PHY registers
 *
 * Description:	initializes all the Broadcom PHY registers
 *
 * Note:
 *
 * Returns:
 *	nothing
 */
static void SkXmInitPhyBcom(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port,		/* Port Index (MAC_1 + n) */
SK_BOOL	DoLoop)		/* Should a PHY LoopBack be set-up? */
{
	SK_GEPORT	*pPrt;
	SK_U16		Ctrl1;
	SK_U16		Ctrl2;
	SK_U16		Ctrl3;
	SK_U16		Ctrl4;
	SK_U16		Ctrl5;

	Ctrl1 = PHY_CT_SP1000;
	Ctrl2 = 0;
	Ctrl3 = PHY_SEL_TYPE;
	Ctrl4 = PHY_B_PEC_EN_LTR;
	Ctrl5 = PHY_B_AC_TX_TST;

	pPrt = &pAC->GIni.GP[Port];

	/* manually Master/Slave ? */
	if (pPrt->PMSMode != SK_MS_MODE_AUTO) {
		Ctrl2 |= PHY_B_1000C_MSE;

		if (pPrt->PMSMode == SK_MS_MODE_MASTER) {
			Ctrl2 |= PHY_B_1000C_MSC;
		}
	}

	/* Auto-negotiation ? */
	if (pPrt->PLinkMode == SK_LMODE_HALF || pPrt->PLinkMode == SK_LMODE_FULL) {
		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
			("InitPhyBcom: no auto-negotiation Port %d\n", Port));
		/* set DuplexMode in Config register */
		if (pPrt->PLinkMode == SK_LMODE_FULL) {
			Ctrl1 |= PHY_CT_DUP_MD;
		}

		/* Determine Master/Slave manually if not already done */
		if (pPrt->PMSMode == SK_MS_MODE_AUTO) {
			Ctrl2 |= PHY_B_1000C_MSE;	/* set it to Slave */
		}

		/*
		 * Do NOT enable Auto-negotiation here. This would hold
		 * the link down because no IDLES are transmitted
		 */
	}
	else {
		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
			("InitPhyBcom: with auto-negotiation Port %d\n", Port));
		/* set Auto-negotiation advertisement */

		/*
		 * Workaround BCOM Errata #1 for the C5 type.
		 * 1000Base-T Link Acquisition Failure in Slave Mode
		 * Set Repeater/DTE bit 10 of the 1000Base-T Control Register
		 */
		Ctrl2 |= PHY_B_1000C_RD;

		 /* set Full/half duplex capabilities */
		switch (pPrt->PLinkMode) {
		case SK_LMODE_AUTOHALF:
			Ctrl2 |= PHY_B_1000C_AHD;
			break;
		case SK_LMODE_AUTOFULL:
			Ctrl2 |= PHY_B_1000C_AFD;
			break;
		case SK_LMODE_AUTOBOTH:
			Ctrl2 |= PHY_B_1000C_AFD | PHY_B_1000C_AHD;
			break;
		default:
			SK_ERR_LOG(pAC, SK_ERRCL_SW | SK_ERRCL_INIT, SKERR_HWI_E015,
				SKERR_HWI_E015MSG);
		}

		/* set Flow-control capabilities */
		switch (pPrt->PFlowCtrlMode) {
		case SK_FLOW_MODE_NONE:
			Ctrl3 |= PHY_B_P_NO_PAUSE;
			break;
		case SK_FLOW_MODE_LOC_SEND:
			Ctrl3 |= PHY_B_P_ASYM_MD;
			break;
		case SK_FLOW_MODE_SYMMETRIC:
			Ctrl3 |= PHY_B_P_SYM_MD;
			break;
		case SK_FLOW_MODE_SYM_OR_REM:
			Ctrl3 |= PHY_B_P_BOTH_MD;
			break;
		default:
			SK_ERR_LOG(pAC, SK_ERRCL_SW | SK_ERRCL_INIT, SKERR_HWI_E016,
				SKERR_HWI_E016MSG);
		}

		/* Restart Auto-negotiation */
		Ctrl1 |= PHY_CT_ANE | PHY_CT_RE_CFG;
	}

	/* Initialize LED register here? */
	/* No. Please do it in SkDgXmitLed() (if required) and swap
		init order of LEDs and XMAC. (MAl) */

	/* Write 1000Base-T Control Register */
	SkXmPhyWrite(pAC, IoC, Port, PHY_BCOM_1000T_CTRL, Ctrl2);
	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("Set 1000B-T Ctrl Reg = 0x%04X\n", Ctrl2));

	/* Write AutoNeg Advertisement Register */
	SkXmPhyWrite(pAC, IoC, Port, PHY_BCOM_AUNE_ADV, Ctrl3);
	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("Set Auto-Neg.Adv.Reg = 0x%04X\n", Ctrl3));

	if (DoLoop) {
		/* set the PHY Loopback bit, too */
		Ctrl1 |= PHY_CT_LOOP;
	}

	if (pPrt->PPortUsage == SK_JUMBO_LINK) {
		/* configure FIFO to high latency for transmission of ext. packets */
		Ctrl4 |= PHY_B_PEC_HIGH_LA;

		/* configure reception of extended packets */
		Ctrl5 |= PHY_B_AC_LONG_PACK;

		SkXmPhyWrite(pAC, IoC, Port, PHY_BCOM_AUX_CTRL, Ctrl5);
	}

	/* Configure LED Traffic Mode and Jumbo Frame usage if specified */
	SkXmPhyWrite(pAC, IoC, Port, PHY_BCOM_P_EXT_CTRL, Ctrl4);

	/* Write to the PHY control register */
	SkXmPhyWrite(pAC, IoC, Port, PHY_BCOM_CTRL, Ctrl1);
	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("PHY Control Reg = 0x%04X\n", Ctrl1));
}	/* SkXmInitPhyBcom */
#endif /* GENESIS */


#ifdef YUKON
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

	if (ChipId == CHIP_ID_YUKON_EC_U || ChipId == CHIP_ID_YUKON_EX) {
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
			/* apply COMA mode workaround for Yukon-Plus*/
			(void)SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PAGE_ADDR, 31);

			Ret = SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PAGE_DATA, 0xfff3);

			PowerDownBit = PCI_PHY_COMA;
		}

		SK_TST_MODE_ON(IoC);

		SK_IN32(IoC, PCI_C(pAC, PCI_OUR_REG_1), &DWord);

		/* set PHY to PowerDown/COMA Mode */
		SK_OUT32(IoC, PCI_C(pAC, PCI_OUR_REG_1), DWord | PowerDownBit);

		/* check if this routine was called from a for() loop */
		if (CHIP_ID_YUKON_2(pAC) &&
			(pAC->GIni.GIMacsFound == 1 || Port == MAC_2)) {

			if (ChipId == CHIP_ID_YUKON_EC_U ||
				ChipId == CHIP_ID_YUKON_EX ||
				ChipId == CHIP_ID_YUKON_FE_P) {
				/* set GPHY Control reset */
				SK_OUT8(IoC, MR_ADDR(Port, GPHY_CTRL), (SK_U8)GPC_RST_SET);

				/* additional power saving measurements */
				SK_IN32(IoC, PCI_C(pAC, PCI_OUR_REG_4), &DWord);

				if (pAC->GIni.GIGotoD3Cold) {
					/* set gating core clock for LTSSM in DETECT state */
					DWord |= (P_PEX_LTSSM_STAT(P_PEX_LTSSM_DET_STAT) |
						/* enable Gate Root Core Clock */
						P_CLK_GATE_ROOT_COR_ENA);

					/* set Mask Register for Release/Gate Clock */
					SK_OUT32(IoC, PCI_C(pAC, PCI_OUR_REG_5),
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
						SK_IN16(IoC, PCI_C(pAC, PEX_CAP_REGS(PEX_LNK_CTRL)), &Word);
						Word |= PEX_LC_CLK_PM_ENA;
						SK_OUT16(IoC, PCI_C(pAC, PEX_CAP_REGS(PEX_LNK_CTRL)), Word);
					}
					else {
						/* force CLKREQ Enable in Our4 (A1b only) */
						DWord |= P_ASPM_FORCE_CLKREQ_ENA;
					}

					/* set Mask Register for Release/Gate Clock */
					SK_OUT32(IoC, PCI_C(pAC, PCI_OUR_REG_5),
						P_REL_PCIE_EXIT_L1_ST | P_GAT_PCIE_ENTER_L1_ST |
						P_REL_PCIE_RX_EX_IDLE | P_GAT_PCIE_RX_EL_IDLE |
						P_REL_GPHY_LINK_UP | P_GAT_GPHY_LINK_DOWN);
				}

				SK_OUT32(IoC, PCI_C(pAC, PCI_OUR_REG_4), DWord);

				if (!pAC->GIni.GIAsfRunning) {
					/* put CPU into halt state */
					SK_OUT8(IoC, B28_Y2_ASF_STAT_CMD, (SK_U8)HCU_CCSR_ASF_RESET);

					SK_OUT8(IoC, B28_Y2_ASF_STAT_CMD, (SK_U8)HCU_CCSR_ASF_HALTED);
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
					("Set Core Clock: 0x%08X\n", DWord));

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
					ChipId == CHIP_ID_YUKON_FE_P) {

#ifdef PCI_E_L1_STATE
					SK_IN16(IoC, PCI_C(pAC, PCI_OUR_REG_1), &Word);
					/* force to PCIe L1 */
					Word |= (SK_U16)PCI_FORCE_PEX_L1;
					SK_OUT16(IoC, PCI_C(pAC, PCI_OUR_REG_1), Word);
#else /* !PCI_E_L1_STATE */

#ifdef DEEP_SLEEP_D1
					SK_IN16(IoC, PCI_C(pAC, PEX_CAP_REGS(PEX_LNK_CTRL)), &Word);
					/* check if ASPM L1 enabled */
					if ((Word & PEX_LC_ASPM_LC_L1) != 0) {
						break;
					}
#else
					break;
#endif /* !DEEP_SLEEP_D1 */

#endif /* !PCI_E_L1_STATE */
				}

				SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
					("Switch to D1 state\n", DWord));

				/* switch to D1 state */
				SK_OUT8(IoC, PCI_C(pAC, PCI_PM_CTL_STS), PCI_PM_STATE_D1);
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

		break;

	/* energy detect and energy detect plus mode */
	case PHY_PM_ENERGY_DETECT:
	case PHY_PM_ENERGY_DETECT_PLUS:

		Ret = SkGmPhyRead(pAC, IoC, Port, PHY_MARV_PHY_CTRL, &PhySpec);

#ifdef XXX
		/* disable Polarity Reversal */
		PhySpec |= PHY_M_PC_POL_R_DIS;
#endif /* XXX */

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
			ChipId == CHIP_ID_YUKON_FE_P) {
			/* additional power saving measurements */
			SK_IN32(IoC, PCI_C(pAC, PCI_OUR_REG_4), &DWord);

			if (pAC->GIni.GIGotoD3Cold) {
				/* set gating core clock for LTSSM in DETECT state */
				DWord |= (P_PEX_LTSSM_STAT(P_PEX_LTSSM_DET_STAT) |
					/* enable Gate Root Core Clock */
					P_CLK_GATE_ROOT_COR_ENA);

				/* set Mask Register for Release/Gate Clock */
				SK_OUT32(IoC, PCI_C(pAC, PCI_OUR_REG_5),
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
					SK_IN16(IoC, PCI_C(pAC, PEX_CAP_REGS(PEX_LNK_CTRL)), &Word);
					Word |= PEX_LC_CLK_PM_ENA;
					SK_OUT16(IoC, PCI_C(pAC, PEX_CAP_REGS(PEX_LNK_CTRL)), Word);
				}
				else {
					/* force CLKREQ Enable in Our4 (A1b only) */
					DWord |= P_ASPM_FORCE_CLKREQ_ENA;
				}

				/* set Mask Register for Release/Gate Clock */
				SK_OUT32(IoC, PCI_C(pAC, PCI_OUR_REG_5),
					P_REL_PCIE_EXIT_L1_ST | P_GAT_PCIE_ENTER_L1_ST |
					P_REL_PCIE_RX_EX_IDLE | P_GAT_PCIE_RX_EL_IDLE |
					P_REL_GPHY_LINK_UP | P_GAT_GPHY_LINK_DOWN);
			}

			SK_OUT32(IoC, PCI_C(pAC, PCI_OUR_REG_4), DWord);

#ifdef PCI_E_L1_STATE
			SK_IN16(IoC, PCI_C(pAC, PCI_OUR_REG_1), &Word);
			/* enable PCIe L1 on GPHY link down */
			Word |= (SK_U16)PCI_ENA_GPHY_LNK;
			SK_OUT16(IoC, PCI_C(pAC, PCI_OUR_REG_1), Word);
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
	SK_U32	DWord;
	SK_U32	PowerDownBit;
	SK_U16	Word;
	SK_U8	LastMode;
	int		ChipId;
	int		Ret = 0;

	if (!(CHIP_ID_YUKON_2(pAC) || (pAC->GIni.GIYukonLite &&
		pAC->GIni.GIChipRev >= CHIP_REV_YU_LITE_A3))) {

		return(1);
	}

	/* save current power mode */
	LastMode = pAC->GIni.GP[Port].PPhyPowerState;
	pAC->GIni.GP[Port].PPhyPowerState = PHY_PM_OPERATIONAL_MODE;

	ChipId = pAC->GIni.GIChipId;

	SK_DBG_MSG(pAC, SK_DBGMOD_POWM, SK_DBGCAT_CTRL,
		("SkGmLeaveLowPowerMode: %u\n", LastMode));

	switch (LastMode) {
	/* COMA mode (deep sleep) */
	case PHY_PM_DEEP_SLEEP:

		if (ChipId == CHIP_ID_YUKON_EC_U ||
			ChipId == CHIP_ID_YUKON_EX ||
			ChipId == CHIP_ID_YUKON_FE_P) {
#ifdef PCI_E_L1_STATE
			SkPciReadCfgWord(pAC, PCI_OUR_REG_1, &Word);

			/* set the default value into bits 6 & 5 */
			Word &= ~(SK_U16)(PCI_ENA_GPHY_LNK | PCI_FORCE_PEX_L1);

			SkPciWriteCfgWord(pAC, PCI_OUR_REG_1, Word);
#endif /* PCI_E_L1_STATE */

			SK_IN32(IoC, PCI_C(pAC, PCI_OUR_REG_4), &DWord);

			DWord &= P_ASPM_CONTROL_MSK;
			/* set all bits to 0 except bits 15..12 and 8 */
			SK_OUT32(IoC, PCI_C(pAC, PCI_OUR_REG_4), DWord);

			/* set to default value */
			SK_OUT32(IoC, PCI_C(pAC, PCI_OUR_REG_5), 0);
		}

		SkPciReadCfgWord(pAC, PCI_PM_CTL_STS, &Word);

		if ((Word & PCI_PM_STATE_MSK) != 0) {
			/* switch to D0 state */
			SkPciWriteCfgWord(pAC, PCI_PM_CTL_STS, Word & ~PCI_PM_STATE_MSK);
		}

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

		SK_IN32(IoC, PCI_C(pAC, PCI_OUR_REG_1), &DWord);

		/* Release PHY from PowerDown/COMA Mode */
		SK_OUT32(IoC, PCI_C(pAC, PCI_OUR_REG_1), DWord & ~PowerDownBit);

		SK_TST_MODE_OFF(IoC);

		if (CHIP_ID_YUKON_2(pAC)) {

			if (ChipId == CHIP_ID_YUKON_FE) {
				/* release IEEE compatible Power Down Mode */
				Ret = SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_CTRL, PHY_CT_ANE);
			}
			else if (ChipId == CHIP_ID_YUKON_EC_U ||
					 ChipId == CHIP_ID_YUKON_EX ||
					 ChipId == CHIP_ID_YUKON_FE_P) {
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

		if (ChipId != CHIP_ID_YUKON_XL) {

			Ret = SkGmPhyRead(pAC, IoC, Port, PHY_MARV_PHY_CTRL, &Word);

			if (ChipId == CHIP_ID_YUKON_FE || ChipId == CHIP_ID_YUKON_FE_P) {
				/* disable Energy Detect */
				Word &= ~PHY_M_PC_ENA_ENE_DT;
			}
			else {
				/* disable energy detect mode & enable MAC 125 MHz clock */
				Word &= ~(PHY_M_PC_EN_DET_MSK | PHY_M_PC_DIS_125CLK);
			}

#ifdef XXX
			/* enable Polarity Reversal */
			Word &= ~PHY_M_PC_POL_R_DIS;
#endif /* XXX */

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
#ifndef SK_SLIM
	int			MacCtrl;
	SK_U16		LoopSpeed;
#endif /* !SK_SLIM */
	SK_U16		Word;
	SK_U16		PageReg;
#ifndef VCPU
	SK_U16		PhySpec;
	SK_U16		ExtPhyCtrl;
	SK_U16		BlinkCtrl;
	SK_U16		LedCtrl;
	SK_U16		LedConf;
	SK_U16		LedOver;
	int			Mode;
#ifndef SK_DIAG
	SK_EVPARA	Para;
#endif /* !SK_DIAG */
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

#ifndef SK_DIAG
		Para.Para64 = Port;
		SkEventQueue(pAC, SKGE_DRV, SK_DRV_PORT_FAIL, Para);
#endif /* !SK_DIAG */

		return;
	}
#endif /* !VCPU */

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

				/* save page register */
				SkGmPhyRead(pAC, IoC, Port, PHY_MARV_EXT_ADR, &PageReg);

				/* select page 2 to access MAC control register */
				SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, 2);

				if (ChipId == CHIP_ID_YUKON_XL) {
					/* set PHY reg 0, page 2, field [6:4] */
					MacCtrl = PHY_MARV_CTRL;
					LoopSpeed <<= 4;
				}
				else {	/* CHIP_ID_YUKON_EC_U || CHIP_ID_YUKON_EX */
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
			else {
				/* set 'MAC Power up'-bit, set Manual MDI configuration */
				Word = PHY_M_PC_MAC_POW_UP;
			}

			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PHY_CTRL, Word);
		}
#ifndef VCPU
		else
#endif /* !VCPU */
#endif /* !SK_SLIM */
#ifndef VCPU
		if (AutoNeg && pPrt->PLinkSpeed == SK_LSPEED_AUTO && !NewPhyType &&
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

				if (ChipId == CHIP_ID_YUKON_FE_P &&
					pAC->GIni.GIChipRev == CHIP_REV_YU_FE2_A0) {
					/* Enable Class A driver for FE+ A0 */
					SkGmPhyRead(pAC, IoC, Port, PHY_MARV_FE_SPEC_2, &Word);
					Word |= PHY_M_FESC_SEL_CL_A;
					SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_FE_SPEC_2, Word);
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
	}
#endif /* !VCPU */

	/* Read PHY Control */
	SkGmPhyRead(pAC, IoC, Port, PHY_MARV_CTRL, &PhyCtrl);

	Word = PhyCtrl;

	if (!AutoNeg) {
		/* disable Auto-negotiation */
		Word &= ~PHY_CT_ANE;
	}

	/* assert software reset */
	SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_CTRL, Word | PHY_CT_RESET);

	PhyCtrl = 0 /* PHY_CT_COL_TST */;
	C1000BaseT = 0;
	AutoNegAdv = PHY_SEL_TYPE;

#ifndef SK_SLIM
	/* manually Master/Slave ? */
	if (pPrt->PMSMode != SK_MS_MODE_AUTO) {
		/* enable Manual Master/Slave */
		C1000BaseT |= PHY_M_1000C_MSE;

		if (pPrt->PMSMode == SK_MS_MODE_MASTER) {
			C1000BaseT |= PHY_M_1000C_MSC;	/* set it to Master */
		}
	}
#endif /* !SK_SLIM */

	/* Auto-negotiation ? */
	if (!AutoNeg) {

		if (pPrt->PLinkMode == SK_LMODE_FULL) {
			/* set Full Duplex Mode */
			PhyCtrl |= PHY_CT_DUP_MD;
		}

#ifndef SK_SLIM
		/* set Master/Slave manually if not already done */
		if (pPrt->PMSMode == SK_MS_MODE_AUTO) {
			C1000BaseT |= PHY_M_1000C_MSE;	/* set it to Slave */
		}
#endif /* !SK_SLIM */

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
			(!(ChipId == CHIP_ID_YUKON_EC_U || ChipId == CHIP_ID_YUKON_EX) &&
			(pPrt->PLinkMode == SK_LMODE_HALF) &&
			 ((pPrt->PLinkSpeed == SK_LSPEED_STAT_100MBPS) ||
			  (pPrt->PLinkSpeed == SK_LSPEED_STAT_10MBPS)))) {

			/* set Pause Off */
			PauseMode = (SK_U8)GMC_PAUSE_OFF;
		}

		SK_OUT8(IoC, MR_ADDR(Port, GMAC_CTRL), PauseMode);

		if (!DoLoop) {
			/* assert software reset */
			PhyCtrl |= PHY_CT_RESET;
		}
	}
	else {
		/* set Auto-negotiation advertisement */

		if (pAC->GIni.GICopperType) {
			/* set Speed capabilities */
			switch (pPrt->PLinkSpeed) {
			case SK_LSPEED_AUTO:
				if ((pPrt->PLinkSpeedCap & SK_LSPEED_CAP_1000MBPS) != 0) {
					C1000BaseT |= PHY_M_1000C_AFD;
#ifdef xSK_DIAG
					C1000BaseT |= PHY_M_1000C_AHD;
#endif /* SK_DIAG */
				}
				AutoNegAdv |= PHY_M_AN_100_FD | PHY_M_AN_100_HD |
					PHY_M_AN_10_FD | PHY_M_AN_10_HD;
				break;
			case SK_LSPEED_1000MBPS:
				if ((pPrt->PLinkSpeedCap & SK_LSPEED_CAP_1000MBPS) != 0) {
					C1000BaseT |= PHY_M_1000C_AFD;
#ifdef xSK_DIAG
					C1000BaseT |= PHY_M_1000C_AHD;
#endif /* SK_DIAG */
				}
				break;
			case SK_LSPEED_100MBPS:
				if ((pPrt->PLinkSpeedCap & SK_LSPEED_CAP_100MBPS) != 0) {
					AutoNegAdv |= PHY_M_AN_100_FD | PHY_M_AN_100_HD |
						/* advertise 10Base-T also */
						PHY_M_AN_10_FD | PHY_M_AN_10_HD;
				}
				break;
			case SK_LSPEED_10MBPS:
				if ((pPrt->PLinkSpeedCap & SK_LSPEED_CAP_10MBPS) != 0) {
					AutoNegAdv |= PHY_M_AN_10_FD | PHY_M_AN_10_HD;
				}
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
	/*
	 * E-mail from Gu Lin (08-03-2002):
	 */

	/* Program PHY register 30 as 16'h0708 for simulation speed up */
	SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PAGE_DATA, 0x0700 /* 0x0708 */);

	VCpuWait(2000);

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

#ifndef SK_SLIM
	if (DoLoop) {
		/* set the PHY Loopback bit */
		PhyCtrl |= PHY_CT_LOOP;
	}
#endif /* !SK_SLIM */

	/* Write to the PHY Control register */
	SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_CTRL, PhyCtrl);
	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("Set PHY Ctrl Reg. = 0x%04X\n", PhyCtrl));

#ifdef VCPU
	VCpuWait(2000);
#else /* !VCPU */

	LedCtrl = PHY_M_LED_PULS_DUR(PULS_170MS);

	LedOver = 0;

	BlinkCtrl = pAC->GIni.GILedBlinkCtrl;

	if ((BlinkCtrl & SK_ACT_LED_BLINK) != 0) {

		if (ChipId == CHIP_ID_YUKON_FE) {
			/* on 88E3082 these bits are at 11..9 (shifted left) */
			LedCtrl |= PHY_M_LED_BLINK_RT(BLINK_84MS) << 1;

			SkGmPhyRead(pAC, IoC, Port, PHY_MARV_FE_LED_PAR, &Word);

			/* delete LED1 (Activity) control bits */
			Word &= ~PHY_M_FELP_LED1_MSK;

			/* change ACT LED control to LINK/ACT or blink mode */
			Word |= PHY_M_FELP_LED1_CTRL(
				((BlinkCtrl & SK_LED_COMB_ACT_LNK) != 0) ?
				LED_PAR_CTRL_LNK_AC : LED_PAR_CTRL_ACT_BL);

			/* check for LINK_LED mux */
			if ((BlinkCtrl & SK_LED_LINK_MUX_P60) != 0) {
				/* delete LED0 (Speed) control bits */
				Word &= ~PHY_M_FELP_LED0_MSK;
				/* change Speed LED control to Link indication */
				Word |= PHY_M_FELP_LED0_CTRL(LED_PAR_CTRL_LINK);
			}

			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_FE_LED_PAR, Word);
		}
		else if (ChipId == CHIP_ID_YUKON_FE_P) {
			/* Enable Link Partner Next Page */
			PhySpec |= PHY_M_PC_ENA_LIP_NP;

			/* disable Energy Detect and enable scrambler */
			PhySpec &= ~(PHY_M_PC_ENA_ENE_DT | PHY_M_PC_DIS_SCRAMB);

			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PHY_CTRL, PhySpec);

			/* set LED2 -> ACT, LED1 -> LINK, LED0 -> SPEED */
			Word = PHY_M_FELP_LED2_CTRL(LED_PAR_CTRL_ACT_BL) |
				PHY_M_FELP_LED1_CTRL(LED_PAR_CTRL_LINK) |
				PHY_M_FELP_LED0_CTRL(LED_PAR_CTRL_SPEED);

			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_FE_LED_PAR, Word);
		}
		else if (NewPhyType) {
			/* save page register */
			SkGmPhyRead(pAC, IoC, Port, PHY_MARV_EXT_ADR, &PageReg);

			/* select page 3 to access LED control register */
			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, 3);

			LedConf = PHY_M_LEDC_LOS_CTRL(1) |		/* LINK/ACT (Yukon-2 only) */
					  PHY_M_LEDC_STA1_CTRL(7) |		/* 100 Mbps */
					  PHY_M_LEDC_STA0_CTRL(7);		/* 1000 Mbps */

			Mode = 7;		/* 10 Mbps: On */

			if (ChipId == CHIP_ID_YUKON_XL) {
				/* set Polarity Control register */
				SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PHY_STAT, (SK_U16)
					(PHY_M_POLC_LS1_P_MIX(4) | PHY_M_POLC_IS0_P_MIX(4) |
					 PHY_M_POLC_LOS_CTRL(2) | PHY_M_POLC_INIT_CTRL(2) |
					 PHY_M_POLC_STA1_CTRL(2) | PHY_M_POLC_STA0_CTRL(2)));
			}
			else if (ChipId == CHIP_ID_YUKON_EC_U ||
					 ChipId == CHIP_ID_YUKON_EX) {
				/* check for LINK_LED mux */
				if ((BlinkCtrl & SK_LED_LINK_MUX_P60) != 0) {

					/* LED scheme 2 */
					SK_IN16(IoC, GPHY_CTRL, &Word);

					Word |= GPC_LED_CONF_VAL(4);

					/* set GPHY LED Config */
					SK_OUT16(IoC, GPHY_CTRL, Word);
				}
				else {

					SK_IN16(IoC, PCI_C(pAC, PCI_OUR_REG_4), &Word);

					if ((Word & P_PIN63_LINK_LED_ENA) == 0) {
						/* LED scheme 1 */
						Mode = 8;	/* 10 Mbps: forced Off */

						if ((BlinkCtrl & SK_ACT_LED_NOTR_OFF) == 0) {
							/* set LED[5:4] Function Control and Polarity */
							SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_INT_STAT,
								(SK_U16)
								/* LED_ACT to Link/Act. */
								(PHY_M_LEDC_STA1_CTRL(1) |
								/* LED_DUP to Duplex */
								PHY_M_LEDC_STA0_CTRL(6)));
						}
					}
					/* LED scheme 3 if P_PIN63_LINK_LED_ENA is set */
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

	if (ChipId == CHIP_ID_YUKON_EC_U) {
		if (pAC->GIni.GIChipRev >= CHIP_REV_YU_EC_U_A1) {
			/* apply fixes in PHY AFE */
			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, 0x00ff);

			/* increase differential signal amplitude in 10BASE-T */
			SkGmPhyWrite(pAC, IoC, Port, 24, 0xaa99);
			SkGmPhyWrite(pAC, IoC, Port, 23, 0x2011);

			/* fix for IEEE A/B Symmetry failure in 1000BASE-T */
			SkGmPhyWrite(pAC, IoC, Port, 24, 0xa204);
			SkGmPhyWrite(pAC, IoC, Port, 23, 0x2002);

			/* set page register to 0 */
			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, 0);
		}
	}
	else if (ChipId == CHIP_ID_YUKON_FE_P &&
			 pAC->GIni.GIChipRev == CHIP_REV_YU_FE2_A0) {
		/* apply workaround for integrated resistors calibration */
		SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PAGE_ADDR, 17);

		SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PAGE_DATA, 0x3f60);
	}
	else if (ChipId != CHIP_ID_YUKON_EX) {
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

	/* enable PHY interrupts */
	SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_INT_MASK, (SK_U16)PHY_M_DEF_MSK);
#endif /* !VCPU */

}	/* SkGmInitPhyMarv */
#endif /* YUKON */


#ifdef OTHER_PHY
/******************************************************************************
 *
 *	SkXmInitPhyLone() - Initialize the Level One PHY registers
 *
 * Description:	initializes all the Level One PHY registers
 *
 * Note:
 *
 * Returns:
 *	nothing
 */
static void SkXmInitPhyLone(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port,		/* Port Index (MAC_1 + n) */
SK_BOOL	DoLoop)		/* Should a PHY LoopBack be set-up? */
{
	SK_GEPORT	*pPrt;
	SK_U16		Ctrl1;
	SK_U16		Ctrl2;
	SK_U16		Ctrl3;

	Ctrl1 = PHY_CT_SP1000;
	Ctrl2 = 0;
	Ctrl3 = PHY_SEL_TYPE;

	pPrt = &pAC->GIni.GP[Port];

	/* manually Master/Slave ? */
	if (pPrt->PMSMode != SK_MS_MODE_AUTO) {
		Ctrl2 |= PHY_L_1000C_MSE;

		if (pPrt->PMSMode == SK_MS_MODE_MASTER) {
			Ctrl2 |= PHY_L_1000C_MSC;
		}
	}

	/* Auto-negotiation ? */
	if (pPrt->PLinkMode == SK_LMODE_HALF || pPrt->PLinkMode == SK_LMODE_FULL) {
		/*
		 * level one spec say: "1000 Mbps: manual mode not allowed"
		 * but lets see what happens...
		 */
		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
			("InitPhyLone: no auto-negotiation Port %d\n", Port));
		/* set DuplexMode in Config register */
		if (pPrt->PLinkMode == SK_LMODE_FULL) {
			Ctrl1 |= PHY_CT_DUP_MD;
		}

		/* Determine Master/Slave manually if not already done */
		if (pPrt->PMSMode == SK_MS_MODE_AUTO) {
			Ctrl2 |= PHY_L_1000C_MSE;	/* set it to Slave */
		}
		/*
		 * Do NOT enable Auto-negotiation here. This would hold
		 * the link down because no IDLES are transmitted
		 */
	}
	else {
		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
			("InitPhyLone: with auto-negotiation Port %d\n", Port));
		/* set Auto-negotiation advertisement */

		/* set Full/half duplex capabilities */
		switch (pPrt->PLinkMode) {
		case SK_LMODE_AUTOHALF:
			Ctrl2 |= PHY_L_1000C_AHD;
			break;
		case SK_LMODE_AUTOFULL:
			Ctrl2 |= PHY_L_1000C_AFD;
			break;
		case SK_LMODE_AUTOBOTH:
			Ctrl2 |= PHY_L_1000C_AFD | PHY_L_1000C_AHD;
			break;
		default:
			SK_ERR_LOG(pAC, SK_ERRCL_SW | SK_ERRCL_INIT, SKERR_HWI_E015,
				SKERR_HWI_E015MSG);
		}

		/* set Flow-control capabilities */
		switch (pPrt->PFlowCtrlMode) {
		case SK_FLOW_MODE_NONE:
			Ctrl3 |= PHY_L_P_NO_PAUSE;
			break;
		case SK_FLOW_MODE_LOC_SEND:
			Ctrl3 |= PHY_L_P_ASYM_MD;
			break;
		case SK_FLOW_MODE_SYMMETRIC:
			Ctrl3 |= PHY_L_P_SYM_MD;
			break;
		case SK_FLOW_MODE_SYM_OR_REM:
			Ctrl3 |= PHY_L_P_BOTH_MD;
			break;
		default:
			SK_ERR_LOG(pAC, SK_ERRCL_SW | SK_ERRCL_INIT, SKERR_HWI_E016,
				SKERR_HWI_E016MSG);
		}

		/* Restart Auto-negotiation */
		Ctrl1 = PHY_CT_ANE | PHY_CT_RE_CFG;
	}

	/* Write 1000Base-T Control Register */
	SkXmPhyWrite(pAC, IoC, Port, PHY_LONE_1000T_CTRL, Ctrl2);
	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("1000B-T Ctrl Reg = 0x%04X\n", Ctrl2));

	/* Write AutoNeg Advertisement Register */
	SkXmPhyWrite(pAC, IoC, Port, PHY_LONE_AUNE_ADV, Ctrl3);
	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("Auto-Neg.Adv.Reg = 0x%04X\n", Ctrl3));

	if (DoLoop) {
		/* set the PHY Loopback bit, too */
		Ctrl1 |= PHY_CT_LOOP;
	}

	/* Write to the PHY control register */
	SkXmPhyWrite(pAC, IoC, Port, PHY_LONE_CTRL, Ctrl1);
	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("PHY Control Reg = 0x%04X\n", Ctrl1));
}	/* SkXmInitPhyLone */


/******************************************************************************
 *
 *	SkXmInitPhyNat() - Initialize the National PHY registers
 *
 * Description:	initializes all the National PHY registers
 *
 * Note:
 *
 * Returns:
 *	nothing
 */
static void SkXmInitPhyNat(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port,		/* Port Index (MAC_1 + n) */
SK_BOOL	DoLoop)		/* Should a PHY LoopBack be set-up? */
{
/* todo: National */
}	/* SkXmInitPhyNat */
#endif /* OTHER_PHY */


/******************************************************************************
 *
 *	SkMacInitPhy() - Initialize the PHY registers
 *
 * Description:	calls the Init PHY routines dep. on board type
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
	SK_GEPORT	*pPrt;

	pPrt = &pAC->GIni.GP[Port];

#ifdef GENESIS
	if (pAC->GIni.GIGenesis) {

		switch (pPrt->PhyType) {
		case SK_PHY_XMAC:
			SkXmInitPhyXmac(pAC, IoC, Port, DoLoop);
			break;
		case SK_PHY_BCOM:
			SkXmInitPhyBcom(pAC, IoC, Port, DoLoop);
			break;
#ifdef OTHER_PHY
		case SK_PHY_LONE:
			SkXmInitPhyLone(pAC, IoC, Port, DoLoop);
			break;
		case SK_PHY_NAT:
			SkXmInitPhyNat(pAC, IoC, Port, DoLoop);
			break;
#endif /* OTHER_PHY */
		}
	}
#endif /* GENESIS */

#ifdef YUKON
	if (pAC->GIni.GIYukon) {

		SkGmInitPhyMarv(pAC, IoC, Port, DoLoop);
	}
#endif /* YUKON */

}	/* SkMacInitPhy */


#ifdef GENESIS
/******************************************************************************
 *
 *	SkXmAutoNegDoneXmac() - Auto-negotiation handling
 *
 * Description:
 *	This function handles the auto-negotiation if the Done bit is set.
 *
 * Returns:
 *	SK_AND_OK	o.k.
 *	SK_AND_DUP_CAP	Duplex capability error happened
 *	SK_AND_OTHER	Other error happened
 */
static int SkXmAutoNegDoneXmac(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port)		/* Port Index (MAC_1 + n) */
{
	SK_GEPORT	*pPrt;
	SK_U16		ResAb;		/* Resolved Ability */
	SK_U16		LinkPartAb;	/* Link Partner Ability */

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("AutoNegDoneXmac, Port %d\n", Port));

	pPrt = &pAC->GIni.GP[Port];

	/* Get PHY parameters */
	SkXmPhyRead(pAC, IoC, Port, PHY_XMAC_AUNE_LP, &LinkPartAb);
	SkXmPhyRead(pAC, IoC, Port, PHY_XMAC_RES_ABI, &ResAb);

	if ((LinkPartAb & PHY_X_AN_RFB) != 0) {
		/* At least one of the remote fault bit is set */
		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_ERR,
			("AutoNegFail: Remote fault bit set Port %d\n", Port));
		pPrt->PAutoNegFail = SK_TRUE;

		return(SK_AND_OTHER);
	}

	/* Check Duplex mismatch */
	if ((ResAb & (PHY_X_RS_HD | PHY_X_RS_FD)) == PHY_X_RS_FD) {
		pPrt->PLinkModeStatus = (SK_U8)SK_LMODE_STAT_AUTOFULL;
	}
	else if ((ResAb & (PHY_X_RS_HD | PHY_X_RS_FD)) == PHY_X_RS_HD) {
		pPrt->PLinkModeStatus = (SK_U8)SK_LMODE_STAT_AUTOHALF;
	}
	else {
		/* Error */
		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_ERR,
			("AutoNegFail: Duplex mode mismatch Port %d\n", Port));
		pPrt->PAutoNegFail = SK_TRUE;

		return(SK_AND_DUP_CAP);
	}

	/* Check PAUSE mismatch */
	/* We are NOT using chapter 4.23 of the Xaqti manual */
	/* We are using IEEE 802.3z/D5.0 Table 37-4 */
	if ((pPrt->PFlowCtrlMode == SK_FLOW_MODE_SYMMETRIC ||
		 pPrt->PFlowCtrlMode == SK_FLOW_MODE_SYM_OR_REM) &&
		(LinkPartAb & PHY_X_P_SYM_MD) != 0) {
		/* Symmetric PAUSE */
		pPrt->PFlowCtrlStatus = SK_FLOW_STAT_SYMMETRIC;
	}
	else if (pPrt->PFlowCtrlMode == SK_FLOW_MODE_SYM_OR_REM &&
			 (LinkPartAb & PHY_X_RS_PAUSE) == PHY_X_P_ASYM_MD) {
		/* enable PAUSE receive, disable PAUSE transmit */
		pPrt->PFlowCtrlStatus = SK_FLOW_STAT_REM_SEND;
	}
	else if (pPrt->PFlowCtrlMode == SK_FLOW_MODE_LOC_SEND &&
			 (LinkPartAb & PHY_X_RS_PAUSE) == PHY_X_P_BOTH_MD) {
		/* disable PAUSE receive, enable PAUSE transmit */
		pPrt->PFlowCtrlStatus = SK_FLOW_STAT_LOC_SEND;
	}
	else {
		/* PAUSE mismatch -> no PAUSE */
		pPrt->PFlowCtrlStatus = SK_FLOW_STAT_NONE;
	}

	pPrt->PLinkSpeedUsed = (SK_U8)SK_LSPEED_STAT_1000MBPS;

	return(SK_AND_OK);
}	/* SkXmAutoNegDoneXmac */


/******************************************************************************
 *
 *	SkXmAutoNegDoneBcom() - Auto-negotiation handling
 *
 * Description:
 *	This function handles the auto-negotiation if the Done bit is set.
 *
 * Returns:
 *	SK_AND_OK	o.k.
 *	SK_AND_DUP_CAP	Duplex capability error happened
 *	SK_AND_OTHER	Other error happened
 */
static int SkXmAutoNegDoneBcom(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port)		/* Port Index (MAC_1 + n) */
{
	SK_GEPORT	*pPrt;
#ifdef TEST_ONLY
	SK_U16		ResAb;		/* Resolved Ability */
#endif
	SK_U16		LinkPartAb;	/* Link Partner Ability */
	SK_U16		AuxStat;	/* Auxiliary Status */

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("AutoNegDoneBcom, Port %d\n", Port));
	pPrt = &pAC->GIni.GP[Port];

	/* Get PHY parameters */
	SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_AUNE_LP, &LinkPartAb);
#ifdef TEST_ONLY
	SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_1000T_STAT, &ResAb);
#endif

	SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_AUX_STAT, &AuxStat);

	if ((LinkPartAb & PHY_B_AN_RF) != 0) {
		/* Remote fault bit is set: Error */
		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_ERR,
			("AutoNegFail: Remote fault bit set Port %d\n", Port));
		pPrt->PAutoNegFail = SK_TRUE;

		return(SK_AND_OTHER);
	}

	/* Check Duplex mismatch */
	if ((AuxStat & PHY_B_AS_AN_RES_MSK) == PHY_B_RES_1000FD) {
		pPrt->PLinkModeStatus = (SK_U8)SK_LMODE_STAT_AUTOFULL;
	}
	else if ((AuxStat & PHY_B_AS_AN_RES_MSK) == PHY_B_RES_1000HD) {
		pPrt->PLinkModeStatus = (SK_U8)SK_LMODE_STAT_AUTOHALF;
	}
	else {
		/* Error */
		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_ERR,
			("AutoNegFail: Duplex mode mismatch Port %d\n", Port));
		pPrt->PAutoNegFail = SK_TRUE;

		return(SK_AND_DUP_CAP);
	}

#ifdef TEST_ONLY
	/* Check Master/Slave resolution */
	if ((ResAb & PHY_B_1000S_MSF) != 0) {
		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_ERR,
			("Master/Slave Fault Port %d\n", Port));
		pPrt->PAutoNegFail = SK_TRUE;
		pPrt->PMSStatus = SK_MS_STAT_FAULT;
		return(SK_AND_OTHER);
	}

	pPrt->PMSStatus = ((ResAb & PHY_B_1000S_MSR) != 0) ?
		SK_MS_STAT_MASTER : SK_MS_STAT_SLAVE;
#endif

	/* Check PAUSE mismatch ??? */
	/* We are using IEEE 802.3z/D5.0 Table 37-4 */
	if ((AuxStat & PHY_B_AS_PAUSE_MSK) == PHY_B_AS_PAUSE_MSK) {
		/* Symmetric PAUSE */
		pPrt->PFlowCtrlStatus = SK_FLOW_STAT_SYMMETRIC;
	}
	else if ((AuxStat & PHY_B_AS_PAUSE_MSK) == PHY_B_AS_PRR) {
		/* enable PAUSE receive, disable PAUSE transmit */
		pPrt->PFlowCtrlStatus = SK_FLOW_STAT_REM_SEND;
	}
	else if ((AuxStat & PHY_B_AS_PAUSE_MSK) == PHY_B_AS_PRT) {
		/* disable PAUSE receive, enable PAUSE transmit */
		pPrt->PFlowCtrlStatus = SK_FLOW_STAT_LOC_SEND;
	}
	else {
		/* PAUSE mismatch -> no PAUSE */
		pPrt->PFlowCtrlStatus = SK_FLOW_STAT_NONE;
	}

	pPrt->PLinkSpeedUsed = (SK_U8)SK_LSPEED_STAT_1000MBPS;

	return(SK_AND_OK);
}	/* SkXmAutoNegDoneBcom */
#endif /* GENESIS */


#ifdef YUKON
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
		/* Read PHY Auto-Negotiation Expansion */
		SkGmPhyRead(pAC, IoC, Port, PHY_MARV_AUNE_EXP, &LinkPartAb);

		if ((LinkPartAb & PHY_ANE_LP_CAP) == 0) {

			pPrt->PLipaAutoNeg = SK_LIPA_MANUAL;

#ifndef SK_DIAG
			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("Link Partner not Auto-Neg. able, AN Exp.: 0x%04X\n",
				LinkPartAb));

#ifndef NDIS_MINIPORT_DRIVER
			SK_ERR_LOG(pAC, SK_ERRCL_INFO, SKERR_HWI_E025, SKERR_HWI_E025MSG);
#endif
			Para.Para64 = Port;
			SkEventQueue(pAC, SKGE_DRV, SK_DRV_LIPA_NOT_AN_ABLE, Para);
#else
			c_print("Link Partner not Auto-Neg. able, AN Exp.: 0x%04X\n",
				LinkPartAb);
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
			pPrt->PLipaAutoNeg = SK_LIPA_AUTO;
		}
	}
#endif /* !SK_SLIM */

	if ((pPrt->PLinkSpeedCap & SK_LSPEED_CAP_1000MBPS) != 0) {

		SkGmPhyRead(pAC, IoC, Port, PHY_MARV_1000T_STAT, &ResAb);

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
		/* set used link speed */
		switch ((unsigned)(AuxStat & PHY_M_PS_SPEED_MSK)) {
		case (unsigned)PHY_M_PS_SPEED_1000:
			pPrt->PLinkSpeedUsed = (SK_U8)SK_LSPEED_STAT_1000MBPS;
			break;
		case PHY_M_PS_SPEED_100:
			pPrt->PLinkSpeedUsed = (SK_U8)SK_LSPEED_STAT_100MBPS;
			break;
		default:
			pPrt->PLinkSpeedUsed = (SK_U8)SK_LSPEED_STAT_10MBPS;
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
		   pAC->GIni.GIChipId == CHIP_ID_YUKON_EX) &&
		(pPrt->PLinkSpeedUsed < (SK_U8)SK_LSPEED_STAT_1000MBPS) &&
		 pPrt->PLinkModeStatus == (SK_U8)SK_LMODE_STAT_AUTOHALF)) {

		/* set Pause Off */
		PauseMode = (SK_U8)GMC_PAUSE_OFF;
	}

	SK_OUT8(IoC, MR_ADDR(Port, GMAC_CTRL), PauseMode);

	return(SK_AND_OK);
}	/* SkGmAutoNegDoneMarv */
#endif /* YUKON */


#ifdef OTHER_PHY
/******************************************************************************
 *
 *	SkXmAutoNegDoneLone() - Auto-negotiation handling
 *
 * Description:
 *	This function handles the auto-negotiation if the Done bit is set.
 *
 * Returns:
 *	SK_AND_OK	o.k.
 *	SK_AND_DUP_CAP	Duplex capability error happened
 *	SK_AND_OTHER	Other error happened
 */
static int SkXmAutoNegDoneLone(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port)		/* Port Index (MAC_1 + n) */
{
	SK_GEPORT	*pPrt;
	SK_U16		ResAb;		/* Resolved Ability */
	SK_U16		LinkPartAb;	/* Link Partner Ability */
	SK_U16		QuickStat;	/* Auxiliary Status */

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("AutoNegDoneLone, Port %d\n", Port));
	pPrt = &pAC->GIni.GP[Port];

	/* Get PHY parameters */
	SkXmPhyRead(pAC, IoC, Port, PHY_LONE_AUNE_LP, &LinkPartAb);
	SkXmPhyRead(pAC, IoC, Port, PHY_LONE_1000T_STAT, &ResAb);
	SkXmPhyRead(pAC, IoC, Port, PHY_LONE_Q_STAT, &QuickStat);

	if ((LinkPartAb & PHY_L_AN_RF) != 0) {
		/* Remote fault bit is set */
		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_ERR,
			("AutoNegFail: Remote fault bit set Port %d\n", Port));
		pPrt->PAutoNegFail = SK_TRUE;

		return(SK_AND_OTHER);
	}

	/* Check Duplex mismatch */
	if ((QuickStat & PHY_L_QS_DUP_MOD) != 0) {
		pPrt->PLinkModeStatus = (SK_U8)SK_LMODE_STAT_AUTOFULL;
	}
	else {
		pPrt->PLinkModeStatus = (SK_U8)SK_LMODE_STAT_AUTOHALF;
	}

	/* Check Master/Slave resolution */
	if ((ResAb & PHY_L_1000S_MSF) != 0) {
		/* Error */
		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_ERR,
			("Master/Slave Fault Port %d\n", Port));
		pPrt->PAutoNegFail = SK_TRUE;
		pPrt->PMSStatus = SK_MS_STAT_FAULT;
		return(SK_AND_OTHER);
	}

	pPrt->PMSStatus = ((ResAb & PHY_L_1000S_MSR) != 0) ?
		(SK_U8)SK_MS_STAT_MASTER : (SK_U8)SK_MS_STAT_SLAVE;

	/* Check PAUSE mismatch */
	/* We are using IEEE 802.3z/D5.0 Table 37-4 */
	/* we must manually resolve the abilities here */
	pPrt->PFlowCtrlStatus = SK_FLOW_STAT_NONE;

	switch (pPrt->PFlowCtrlMode) {
	case SK_FLOW_MODE_NONE:
		/* default */
		break;
	case SK_FLOW_MODE_LOC_SEND:
		if ((QuickStat & (PHY_L_QS_PAUSE | PHY_L_QS_AS_PAUSE)) ==
			(PHY_L_QS_PAUSE | PHY_L_QS_AS_PAUSE)) {
			/* disable PAUSE receive, enable PAUSE transmit */
			pPrt->PFlowCtrlStatus = SK_FLOW_STAT_LOC_SEND;
		}
		break;
	case SK_FLOW_MODE_SYMMETRIC:
		if ((QuickStat & PHY_L_QS_PAUSE) != 0) {
			/* Symmetric PAUSE */
			pPrt->PFlowCtrlStatus = SK_FLOW_STAT_SYMMETRIC;
		}
		break;
	case SK_FLOW_MODE_SYM_OR_REM:
		if ((QuickStat & (PHY_L_QS_PAUSE | PHY_L_QS_AS_PAUSE)) ==
			PHY_L_QS_AS_PAUSE) {
			/* enable PAUSE receive, disable PAUSE transmit */
			pPrt->PFlowCtrlStatus = SK_FLOW_STAT_REM_SEND;
		}
		else if ((QuickStat & PHY_L_QS_PAUSE) != 0) {
			/* Symmetric PAUSE */
			pPrt->PFlowCtrlStatus = SK_FLOW_STAT_SYMMETRIC;
		}
		break;
	default:
		SK_ERR_LOG(pAC, SK_ERRCL_SW | SK_ERRCL_INIT, SKERR_HWI_E016,
			SKERR_HWI_E016MSG);
	}

	return(SK_AND_OK);
}	/* SkXmAutoNegDoneLone */


/******************************************************************************
 *
 *	SkXmAutoNegDoneNat() - Auto-negotiation handling
 *
 * Description:
 *	This function handles the auto-negotiation if the Done bit is set.
 *
 * Returns:
 *	SK_AND_OK	o.k.
 *	SK_AND_DUP_CAP	Duplex capability error happened
 *	SK_AND_OTHER	Other error happened
 */
static int SkXmAutoNegDoneNat(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port)		/* Port Index (MAC_1 + n) */
{
/* todo: National */
	return(SK_AND_OK);
}	/* SkXmAutoNegDoneNat */
#endif /* OTHER_PHY */


/******************************************************************************
 *
 *	SkMacAutoNegDone() - Auto-negotiation handling
 *
 * Description:	calls the auto-negotiation done routines dep. on board type
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
	int	Rtv;

	Rtv = SK_AND_OK;

	pPrt = &pAC->GIni.GP[Port];

#ifdef GENESIS
	if (pAC->GIni.GIGenesis) {

		switch (pPrt->PhyType) {

		case SK_PHY_XMAC:
			Rtv = SkXmAutoNegDoneXmac(pAC, IoC, Port);
			break;
		case SK_PHY_BCOM:
			Rtv = SkXmAutoNegDoneBcom(pAC, IoC, Port);
			break;
#ifdef OTHER_PHY
		case SK_PHY_LONE:
			Rtv = SkXmAutoNegDoneLone(pAC, IoC, Port);
			break;
		case SK_PHY_NAT:
			Rtv = SkXmAutoNegDoneNat(pAC, IoC, Port);
			break;
#endif /* OTHER_PHY */
		default:
			return(SK_AND_OTHER);
		}
	}
#endif /* GENESIS */

#ifdef YUKON
	if (pAC->GIni.GIYukon) {

		Rtv = SkGmAutoNegDoneMarv(pAC, IoC, Port);
	}
#endif /* YUKON */

	if (Rtv != SK_AND_OK) {
		return(Rtv);
	}

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("AutoNeg done Port %d\n", Port));

	/* We checked everything and may now enable the link */
	pPrt->PAutoNegFail = SK_FALSE;

	SkMacRxTxEnable(pAC, IoC, Port);

	return(SK_AND_OK);
}	/* SkMacAutoNegDone */


#ifndef SK_SLIM
#ifdef GENESIS
/******************************************************************************
 *
 *	SkXmSetRxTxEn() - Special Set Rx/Tx Enable and some features in XMAC
 *
 * Description:
 *  sets MAC or PHY LoopBack and Duplex Mode in the MMU Command Reg.
 *  enables Rx/Tx
 *
 * Returns: N/A
 */
static void SkXmSetRxTxEn(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port,		/* Port Index (MAC_1 + n) */
int		Para)		/* Parameter to set: MAC or PHY LoopBack, Duplex Mode */
{
	SK_U16	Word;

	XM_IN16(IoC, Port, XM_MMU_CMD, &Word);

	switch (Para & (SK_MAC_LOOPB_ON | SK_MAC_LOOPB_OFF)) {
	case SK_MAC_LOOPB_ON:
		Word |= XM_MMU_MAC_LB;
		break;
	case SK_MAC_LOOPB_OFF:
		Word &= ~XM_MMU_MAC_LB;
		break;
	}

	switch (Para & (SK_PHY_LOOPB_ON | SK_PHY_LOOPB_OFF)) {
	case SK_PHY_LOOPB_ON:
		Word |= XM_MMU_GMII_LOOP;
		break;
	case SK_PHY_LOOPB_OFF:
		Word &= ~XM_MMU_GMII_LOOP;
		break;
	}

	switch (Para & (SK_PHY_FULLD_ON | SK_PHY_FULLD_OFF)) {
	case SK_PHY_FULLD_ON:
		Word |= XM_MMU_GMII_FD;
		break;
	case SK_PHY_FULLD_OFF:
		Word &= ~XM_MMU_GMII_FD;
		break;
	}

	XM_OUT16(IoC, Port, XM_MMU_CMD, Word | XM_MMU_ENA_RX | XM_MMU_ENA_TX);

	/* dummy read to ensure writing */
	XM_IN16(IoC, Port, XM_MMU_CMD, &Word);

}	/* SkXmSetRxTxEn */
#endif /* GENESIS */


#ifdef YUKON
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
#endif /* YUKON */


/******************************************************************************
 *
 *	SkMacSetRxTxEn() - Special Set Rx/Tx Enable and parameters
 *
 * Description:	calls the Special Set Rx/Tx Enable routines dep. on board type
 *
 * Returns: N/A
 */
void SkMacSetRxTxEn(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port,		/* Port Index (MAC_1 + n) */
int		Para)
{
#ifdef GENESIS
	if (pAC->GIni.GIGenesis) {

		SkXmSetRxTxEn(pAC, IoC, Port, Para);
	}
#endif /* GENESIS */

#ifdef YUKON
	if (pAC->GIni.GIYukon) {

		SkGmSetRxTxEn(pAC, IoC, Port, Para);
	}
#endif /* YUKON */

}	/* SkMacSetRxTxEn */
#endif /* !SK_SLIM */


/******************************************************************************
 *
 *	SkMacRxTxEnable() - Enable Rx/Tx activity if port is up
 *
 * Description:	enables Rx/Tx dep. on board type
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
#ifdef GENESIS
	SK_U16		SWord;
#endif

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

#ifdef GENESIS
	if (pAC->GIni.GIGenesis) {
		/* set Duplex Mode and Pause Mode */
		SkXmInitDupMd(pAC, IoC, Port);

		SkXmInitPauseMd(pAC, IoC, Port);

		/*
		 * Initialize the Interrupt Mask Register. Default IRQs are...
		 *	- Link Asynchronous Event
		 *	- Link Partner requests config
		 *	- Auto Negotiation Done
		 *	- Rx Counter Event Overflow
		 *	- Tx Counter Event Overflow
		 *	- Transmit FIFO Underrun
		 */
		IntMask = XM_DEF_MSK;

#ifdef DEBUG
		/* add IRQ for Receive FIFO Overflow */
		IntMask &= ~XM_IS_RXF_OV;
#endif /* DEBUG */

		if (pPrt->PhyType != SK_PHY_XMAC) {
			/* disable GP0 interrupt bit */
			IntMask |= XM_IS_INP_ASS;
		}

		XM_OUT16(IoC, Port, XM_IMSK, IntMask);

		/* get MMU Command Reg. */
		XM_IN16(IoC, Port, XM_MMU_CMD, &Reg);

		if (pPrt->PhyType != SK_PHY_XMAC &&
			(pPrt->PLinkModeStatus == SK_LMODE_STAT_FULL ||
			 pPrt->PLinkModeStatus == SK_LMODE_STAT_AUTOFULL)) {
			/* set to Full Duplex */
			Reg |= XM_MMU_GMII_FD;
		}

		switch (pPrt->PhyType) {
		case SK_PHY_BCOM:
			/*
			 * Workaround BCOM Errata (#10523) for all BCom Phys
			 * Enable Power Management after link up
			 */
			SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_AUX_CTRL, &SWord);
			SkXmPhyWrite(pAC, IoC, Port, PHY_BCOM_AUX_CTRL,
				(SK_U16)(SWord & ~PHY_B_AC_DIS_PM));
			SkXmPhyWrite(pAC, IoC, Port, PHY_BCOM_INT_MASK,
				(SK_U16)PHY_B_DEF_MSK);
			break;
#ifdef OTHER_PHY
		case SK_PHY_LONE:
			SkXmPhyWrite(pAC, IoC, Port, PHY_LONE_INT_ENAB, PHY_L_DEF_MSK);
			break;
		case SK_PHY_NAT:
			/* todo National:
			SkXmPhyWrite(pAC, IoC, Port, PHY_NAT_INT_MASK, PHY_N_DEF_MSK); */
			/* no interrupts possible from National ??? */
			break;
#endif /* OTHER_PHY */
		}

		/* enable Rx/Tx */
		XM_OUT16(IoC, Port, XM_MMU_CMD, Reg | XM_MMU_ENA_RX | XM_MMU_ENA_TX);
	}
#endif /* GENESIS */

#ifdef YUKON
	if (pAC->GIni.GIYukon) {
		/*
		 * Initialize the Interrupt Mask Register. Default IRQs are...
		 *	- Rx Counter Event Overflow
		 *	- Tx Counter Event Overflow
		 *	- Transmit FIFO Underrun
		 */
		IntMask = GMAC_DEF_MSK;

#if (defined(DEBUG) || defined(YUK2)) && (!defined(SK_SLIM))
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

		/* enable Rx/Tx */
		GM_OUT16(IoC, Port, GM_GP_CTRL, Reg | GM_GPCR_RX_ENA | GM_GPCR_TX_ENA);
	}
#endif /* YUKON */

	pAC->GIni.GP[Port].PState = SK_PRT_RUN;

	return(0);

}	/* SkMacRxTxEnable */


/******************************************************************************
 *
 *	SkMacRxTxDisable() - Disable Receiver and Transmitter
 *
 * Description:	disables Rx/Tx dep. on board type
 *
 * Returns: N/A
 */
void SkMacRxTxDisable(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port)		/* Port Index (MAC_1 + n) */
{
	SK_U16	Word;

#ifdef GENESIS
	if (pAC->GIni.GIGenesis) {

		XM_IN16(IoC, Port, XM_MMU_CMD, &Word);

		Word &= ~(XM_MMU_ENA_RX | XM_MMU_ENA_TX);

		XM_OUT16(IoC, Port, XM_MMU_CMD, Word);

		/* dummy read to ensure writing */
		XM_IN16(IoC, Port, XM_MMU_CMD, &Word);
	}
#endif /* GENESIS */

#ifdef YUKON
	if (pAC->GIni.GIYukon) {

		GM_IN16(IoC, Port, GM_GP_CTRL, &Word);

		Word &= ~(GM_GPCR_RX_ENA | GM_GPCR_TX_ENA);

		GM_OUT16(IoC, Port, GM_GP_CTRL, Word);
	}
#endif /* YUKON */

}	/* SkMacRxTxDisable */


/******************************************************************************
 *
 *	SkMacIrqDisable() - Disable IRQ from MAC
 *
 * Description:	sets the IRQ-mask to disable IRQ dep. on board type
 *
 * Returns: N/A
 */
void SkMacIrqDisable(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port)		/* Port Index (MAC_1 + n) */
{
	SK_GEPORT	*pPrt;
#ifdef GENESIS
	SK_U16		Word;
#endif

	pPrt = &pAC->GIni.GP[Port];

#ifdef GENESIS
	if (pAC->GIni.GIGenesis) {

		/* disable all XMAC IRQs */
		XM_OUT16(IoC, Port, XM_IMSK, 0xffff);

		/* disable all PHY interrupts */
		switch (pPrt->PhyType) {
			case SK_PHY_BCOM:
				/* Make sure that PHY is initialized */
				if (pPrt->PState != SK_PRT_RESET) {
					/* NOT allowed if BCOM is in RESET state */
					/* Workaround BCOM Errata (#10523) all BCom */
					/* disable Power Management if link is down */
					SkXmPhyRead(pAC, IoC, Port, PHY_BCOM_AUX_CTRL, &Word);
					SkXmPhyWrite(pAC, IoC, Port, PHY_BCOM_AUX_CTRL,
						(SK_U16)(Word | PHY_B_AC_DIS_PM));
					SkXmPhyWrite(pAC, IoC, Port, PHY_BCOM_INT_MASK, 0xffff);
				}
				break;
#ifdef OTHER_PHY
			case SK_PHY_LONE:
				SkXmPhyWrite(pAC, IoC, Port, PHY_LONE_INT_ENAB, 0);
				break;
			case SK_PHY_NAT:
				/* todo: National
				SkXmPhyWrite(pAC, IoC, Port, PHY_NAT_INT_MASK, 0xffff); */
				break;
#endif /* OTHER_PHY */
		}
	}
#endif /* GENESIS */

#ifdef YUKON
	if (pAC->GIni.GIYukon) {
		/* disable all GMAC IRQs */
		SK_OUT8(IoC, MR_ADDR(Port, GMAC_IRQ_MSK), 0);

#ifndef VCPU
		if (pPrt->PState != SK_PRT_RESET) {
			/* disable all PHY IRQs */
			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_INT_MASK, 0);
		}
#endif /* !VCPU */
	}
#endif /* YUKON */

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
	SK_U32	SK_FAR MdReg;
	SK_U8	TimeCtrl;

	if (pAC->GIni.GIGenesis) {

		XM_IN32(IoC, Port, XM_MODE, &MdReg);

		if (Enable) {
			MdReg |= XM_MD_ATS;
		}
		else {
			MdReg &= ~XM_MD_ATS;
		}
		/* setup Mode Register */
		XM_OUT32(IoC, Port, XM_MODE, MdReg);
	}
	else {
		if (Enable) {
			TimeCtrl = GMT_ST_START | GMT_ST_CLR_IRQ;
		}
		else {
			TimeCtrl = GMT_ST_STOP | GMT_ST_CLR_IRQ;
		}
		/* Start/Stop Time Stamp Timer */
		SK_OUT8(IoC, GMAC_TI_ST_CTRL, TimeCtrl);
	}

}	/* SkMacTimeStamp*/
#endif /* !SK_SLIM */

#ifdef SK_DIAG
/******************************************************************************
 *
 *	SkXmSendCont() - Enable / Disable Send Continuous Mode
 *
 * Description:	enable / disable Send Continuous Mode on XMAC resp.
 *								Packet Generation on GPHY
 *
 * Returns:
 *	nothing
 */
void SkXmSendCont(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC,	/* I/O Context */
int		Port,	/* Port Index (MAC_1 + n) */
SK_BOOL	Enable)	/* Enable / Disable */
{
	SK_U16	Reg;
	SK_U16	PageReg;
	SK_U32	SK_FAR MdReg;

	if (pAC->GIni.GIGenesis) {
		XM_IN32(IoC, Port, XM_MODE, &MdReg);

		if (Enable) {
			MdReg |= XM_MD_TX_CONT;
		}
		else {
			MdReg &= ~XM_MD_TX_CONT;
		}
		/* setup Mode Register */
		XM_OUT32(IoC, Port, XM_MODE, MdReg);
	}
	else {
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

		if (HW_HAS_NEWER_PHY(pAC)) {
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
	}

}	/* SkXmSendCont */

#else /* !SK_DIAG */

#ifdef GENESIS
/******************************************************************************
 *
 *	SkXmAutoNegLipaXmac() - Decides whether Link Partner could do auto-neg
 *
 *	This function analyses the Interrupt status word. If any of the
 *	Auto-negotiating interrupt bits are set, the PLipaAutoNeg variable
 *	is set true.
 */
void SkXmAutoNegLipaXmac(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port,		/* Port Index (MAC_1 + n) */
SK_U16	IStatus)	/* Interrupt Status word to analyse */
{
	SK_GEPORT	*pPrt;

	pPrt = &pAC->GIni.GP[Port];

	if (pPrt->PLipaAutoNeg != SK_LIPA_AUTO &&
		(IStatus & (XM_IS_LIPA_RC | XM_IS_RX_PAGE | XM_IS_AND)) != 0) {

		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
			("AutoNegLipa: AutoNeg detected on Port %d, IStatus = 0x%04X\n",
			Port, IStatus));

		pPrt->PLipaAutoNeg = SK_LIPA_AUTO;
	}
}	/* SkXmAutoNegLipaXmac */
#endif /* GENESIS */


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


#ifdef GENESIS
/******************************************************************************
 *
 *	SkXmIrq() - Interrupt Service Routine
 *
 * Description:	services an Interrupt Request of the XMAC
 *
 * Note:
 *	With an external PHY, some interrupt bits are not meaningfull any more:
 *	- LinkAsyncEvent (bit #14)		XM_IS_LNK_AE
 *	- LinkPartnerReqConfig (bit #10)	XM_IS_LIPA_RC
 *	- Page Received (bit #9)		XM_IS_RX_PAGE
 *	- NextPageLoadedForXmt (bit #8)		XM_IS_TX_PAGE
 *	- AutoNegDone (bit #7)			XM_IS_AND
 *	Also probably not valid any more is the GP0 input bit:
 *	- GPRegisterBit0set			XM_IS_INP_ASS
 *
 * Returns:
 *	nothing
 */
static void SkXmIrq(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port)		/* Port Index (MAC_1 + n) */
{
	SK_GEPORT	*pPrt;
	SK_U16		IStatus;	/* Interrupt status read from the XMAC */
	SK_U16		IStatus2;
#ifdef SK_SLIM
	SK_U64		OverflowStatus;
#else
	SK_EVPARA	Para;
#endif /* SK_SLIM */

	pPrt = &pAC->GIni.GP[Port];

	XM_IN16(IoC, Port, XM_ISRC, &IStatus);

	/* Link Partner Auto-negable ? */
	if (pPrt->PhyType == SK_PHY_XMAC) {
		SkXmAutoNegLipaXmac(pAC, IoC, Port, IStatus);
	}
	else {
		/* mask bits that are not used with ext. PHY */
		IStatus &= ~(XM_IS_LNK_AE | XM_IS_LIPA_RC |
			XM_IS_RX_PAGE | XM_IS_TX_PAGE |
			XM_IS_AND | XM_IS_INP_ASS);
	}

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
		("XmacIrq Port %d Isr 0x%04X\n", Port, IStatus));

	if (!pPrt->PHWLinkUp) {
		/* Spurious XMAC interrupt */
		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
			("SkXmIrq: spurious interrupt on Port %d\n", Port));
		return;
	}

	if ((IStatus & XM_IS_INP_ASS) != 0) {
		/* Reread ISR Register if link is not in sync */
		XM_IN16(IoC, Port, XM_ISRC, &IStatus2);

		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
			("SkXmIrq: Link async. Double check Port %d 0x%04X 0x%04X\n",
			 Port, IStatus, IStatus2));
		IStatus &= ~XM_IS_INP_ASS;
		IStatus |= IStatus2;
	}

	if ((IStatus & XM_IS_LNK_AE) != 0) {
		/* not used, GP0 is used instead */
	}

	if ((IStatus & XM_IS_TX_ABORT) != 0) {
		/* not used */
	}

	if ((IStatus & XM_IS_FRC_INT) != 0) {
		/* not used, use ASIC IRQ instead if needed */
	}

	if ((IStatus & (XM_IS_INP_ASS | XM_IS_LIPA_RC | XM_IS_RX_PAGE)) != 0) {
		SkHWLinkDown(pAC, IoC, Port);

		/* Signal to RLMT */
		Para.Para32[0] = (SK_U32)Port;
		SkEventQueue(pAC, SKGE_RLMT, SK_RLMT_LINK_DOWN, Para);

		/* Start workaround Errata #2 timer */
		SkTimerStart(pAC, IoC, &pPrt->PWaTimer, SK_WA_INA_TIME,
			SKGE_HWAC, SK_HWEV_WATIM, Para);
	}

	if ((IStatus & XM_IS_RX_PAGE) != 0) {
		/* not used */
	}

	if ((IStatus & XM_IS_TX_PAGE) != 0) {
		/* not used */
	}

	if ((IStatus & XM_IS_AND) != 0) {
		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
			("SkXmIrq: AND on link that is up Port %d\n", Port));
	}

	if ((IStatus & XM_IS_TSC_OV) != 0) {
		/* not used */
	}

	/* Combined Tx & Rx Counter Overflow SIRQ Event */
	if ((IStatus & (XM_IS_RXC_OV | XM_IS_TXC_OV)) != 0) {
#ifdef SK_SLIM
		SkXmOverflowStatus(pAC, IoC, Port, IStatus, &OverflowStatus);
#else
		Para.Para32[0] = (SK_U32)Port;
		Para.Para32[1] = (SK_U32)IStatus;
		SkPnmiEvent(pAC, IoC, SK_PNMI_EVT_SIRQ_OVERFLOW, Para);
#endif /* SK_SLIM */
	}

	if ((IStatus & XM_IS_RXF_OV) != 0) {
		/* normal situation -> no effect */
#ifdef DEBUG
		pPrt->PRxOverCnt++;
#endif /* DEBUG */
	}

	if ((IStatus & XM_IS_TXF_UR) != 0) {
		/* may NOT happen -> error log */
		SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E020, SKERR_SIRQ_E020MSG);
	}

	if ((IStatus & XM_IS_TX_COMP) != 0) {
		/* not served here */
	}

	if ((IStatus & XM_IS_RX_COMP) != 0) {
		/* not served here */
	}
}	/* SkXmIrq */
#endif /* GENESIS */


#ifdef YUKON
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
	SK_GEPORT	*pPrt;
	SK_U8		IStatus;	/* Interrupt status */
#ifdef SK_SLIM
	SK_U64		OverflowStatus;
#else
	SK_EVPARA	Para;
#endif /* SK_SLIM */

	pPrt = &pAC->GIni.GP[Port];

	SK_IN8(IoC, MR_ADDR(Port, GMAC_IRQ_SRC), &IStatus);

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
		("GmacIrq Port %d Isr 0x%02X\n", Port, IStatus));

	/* Combined Tx & Rx Counter Overflow SIRQ Event */
	if (IStatus & (GM_IS_RX_CO_OV | GM_IS_TX_CO_OV)) {
		/* these IRQs will be cleared by reading GMACs register */
#ifdef SK_SLIM
		SkGmOverflowStatus(pAC, IoC, Port, (SK_U16)IStatus, &OverflowStatus);
#else
		Para.Para32[0] = (SK_U32)Port;
		Para.Para32[1] = (SK_U32)IStatus;
		SkPnmiEvent(pAC, IoC, SK_PNMI_EVT_SIRQ_OVERFLOW, Para);
#endif /* SK_SLIM */
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
		else {
			/* may NOT happen -> error log */
			SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E020, SKERR_SIRQ_E020MSG);
		}
#endif /* !SK_SLIM */
	}

	if (IStatus & GM_IS_TX_COMPL) {
		/* not served here */
	}

	if (IStatus & GM_IS_RX_COMPL) {
		/* not served here */
	}
}	/* SkGmIrq */
#endif /* YUKON */


/******************************************************************************
 *
 *	SkMacIrq() - Interrupt Service Routine for MAC
 *
 * Description:	calls the Interrupt Service Routine dep. on board type
 *
 * Returns:
 *	nothing
 */
void SkMacIrq(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port)		/* Port Index (MAC_1 + n) */
{
#ifdef GENESIS
	if (pAC->GIni.GIGenesis) {
		/* IRQ from XMAC */
		SkXmIrq(pAC, IoC, Port);
	}
#endif /* GENESIS */

#ifdef YUKON
	if (pAC->GIni.GIYukon) {
		/* IRQ from GMAC */
		SkGmIrq(pAC, IoC, Port);
	}
#endif /* YUKON */

}	/* SkMacIrq */

#endif /* !SK_DIAG */

#ifdef GENESIS
/******************************************************************************
 *
 *	SkXmUpdateStats() - Force the XMAC to output the current statistic
 *
 * Description:
 *	The XMAC holds its statistic internally. To obtain the current
 *	values a command must be sent so that the statistic data will
 *	be written to a predefined memory area on the adapter.
 *
 * Returns:
 *	0:  success
 *	1:  something went wrong
 */
int SkXmUpdateStats(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
unsigned int Port)	/* Port Index (MAC_1 + n) */
{
	SK_GEPORT	*pPrt;
	SK_U16		StatReg;
	int			WaitIndex;

	pPrt = &pAC->GIni.GP[Port];
	WaitIndex = 0;

	/* Send an update command to XMAC specified */
	XM_OUT16(IoC, Port, XM_STAT_CMD, XM_SC_SNP_TXC | XM_SC_SNP_RXC);

	/*
	 * It is an auto-clearing register. If the command bits
	 * went to zero again, the statistics are transferred.
	 * Normally the command should be executed immediately.
	 * But just to be sure we execute a loop.
	 */
	do {

		XM_IN16(IoC, Port, XM_STAT_CMD, &StatReg);

		if (++WaitIndex > 10) {

			SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_HWI_E021, SKERR_HWI_E021MSG);

			return(1);
		}
	} while ((StatReg & (XM_SC_SNP_TXC | XM_SC_SNP_RXC)) != 0);

	return(0);
}	/* SkXmUpdateStats */


/******************************************************************************
 *
 *	SkXmMacStatistic() - Get XMAC counter value
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
int SkXmMacStatistic(
SK_AC	*pAC,			/* Adapter Context */
SK_IOC	IoC,			/* I/O Context */
unsigned int Port,		/* Port Index (MAC_1 + n) */
SK_U16	StatAddr,		/* MIB counter base address */
SK_U32	SK_FAR *pVal)	/* Pointer to return statistic value */
{
	if ((StatAddr < XM_TXF_OK) || (StatAddr > XM_RXF_MAX_SZ)) {

		SK_ERR_LOG(pAC, SK_ERRCL_SW, SKERR_HWI_E022, SKERR_HWI_E022MSG);

		return(1);
	}

	XM_IN32(IoC, Port, StatAddr, pVal);

	return(0);
}	/* SkXmMacStatistic */


/******************************************************************************
 *
 *	SkXmResetCounter() - Clear MAC statistic counter
 *
 * Description:
 *	Force the XMAC to clear its statistic counter.
 *
 * Returns:
 *	0:  success
 *	1:  something went wrong
 */
int SkXmResetCounter(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
unsigned int Port)	/* Port Index (MAC_1 + n) */
{
	XM_OUT16(IoC, Port, XM_STAT_CMD, XM_SC_CLR_RXC | XM_SC_CLR_TXC);
	/* Clear two times according to XMAC Errata #3 */
	XM_OUT16(IoC, Port, XM_STAT_CMD, XM_SC_CLR_RXC | XM_SC_CLR_TXC);

	return(0);
}	/* SkXmResetCounter */


/******************************************************************************
 *
 *	SkXmOverflowStatus() - Gets the status of counter overflow interrupt
 *
 * Description:
 *	Checks the source causing an counter overflow interrupt. On success the
 *	resulting counter overflow status is written to <pStatus>, whereas the
 *	upper dword stores the XMAC ReceiveCounterEvent register and the lower
 *	dword the XMAC TransmitCounterEvent register.
 *
 * Note:
 *	For XMAC the interrupt source is a self-clearing register, so the source
 *	must be checked only once. SIRQ module does another check to be sure
 *	that no interrupt get lost during process time.
 *
 * Returns:
 *	0:  success
 *	1:  something went wrong
 */
int SkXmOverflowStatus(
SK_AC	*pAC,				/* Adapter Context */
SK_IOC	IoC,				/* I/O Context */
unsigned int Port,			/* Port Index (MAC_1 + n) */
SK_U16	IStatus,			/* Interrupt Status from MAC */
SK_U64	SK_FAR *pStatus)	/* Pointer for return overflow status value */
{
	SK_U64	Status;	/* Overflow status */
	SK_U32	SK_FAR RegVal;

	Status = 0;

	if ((IStatus & XM_IS_RXC_OV) != 0) {

		XM_IN32(IoC, Port, XM_RX_CNT_EV, &RegVal);
		Status |= (SK_U64)RegVal << 32;
	}

	if ((IStatus & XM_IS_TXC_OV) != 0) {

		XM_IN32(IoC, Port, XM_TX_CNT_EV, &RegVal);
		Status |= (SK_U64)RegVal;
	}

	*pStatus = Status;

	return(0);
}	/* SkXmOverflowStatus */
#endif /* GENESIS */


#ifdef YUKON
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

	/* dummy read after GM_IN32() */
	SK_IN16(IoC, B0_RAP, &StatAddr);

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
	SK_U16	Word;
	int		i;

	GM_IN16(IoC, Port, GM_PHY_ADDR, &Reg);

	/* set MIB Clear Counter Mode */
	GM_OUT16(IoC, Port, GM_PHY_ADDR, Reg | GM_PAR_MIB_CLR);

	/* read all MIB Counters with Clear Mode set */
	for (i = 0; i < GM_MIB_CNT_SIZE; i++) {
		/* the reset is performed only when the lower 16 bits are read */
		GM_IN16(IoC, Port, GM_MIB_CNT_BASE + 8*i, &Word);
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
#ifndef SK_SLIM
	SK_U64	Status;		/* Overflow status */

	Status = 0;
#endif /* !SK_SLIM */

	if ((IStatus & GM_IS_RX_CO_OV) != 0) {
		/* this register is self-clearing after read */
		GM_IN16(IoC, Port, GM_RX_IRQ_SRC, &RegVal);

#ifndef SK_SLIM
		Status |= (SK_U64)RegVal << 32;
#endif /* !SK_SLIM */
	}

	if ((IStatus & GM_IS_TX_CO_OV) != 0) {
		/* this register is self-clearing after read */
		GM_IN16(IoC, Port, GM_TX_IRQ_SRC, &RegVal);

#ifndef SK_SLIM
		Status |= (SK_U64)RegVal;
#endif /* !SK_SLIM */
	}

	/* this register is self-clearing after read */
	GM_IN16(IoC, Port, GM_TR_IRQ_SRC, &RegVal);

#ifndef SK_SLIM
	/* Rx overflow interrupt register bits (LoByte) */
	Status |= (SK_U64)((SK_U8)RegVal) << 48;
	/* Tx overflow interrupt register bits (HiByte) */
	Status |= (SK_U64)(RegVal >> 8) << 16;

	*pStatus = Status;
#endif /* !SK_SLIM */

	/* dummy read after GM_IN16() */
	SK_IN16(IoC, B0_RAP, &RegVal);

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
	int		i;
	int		CableDiagOffs;
	int		MdiPairs;
	int		ChipId;
	int		Rtv;
	SK_BOOL	FastEthernet;
	SK_BOOL	NewPhyType;
	SK_U16	Ctrl;
	SK_U16	RegVal;
	SK_GEPORT	*pPrt;

	pPrt = &pAC->GIni.GP[Port];

	if (pPrt->PhyType != SK_PHY_MARV_COPPER) {

		return(1);
	}

	ChipId = pAC->GIni.GIChipId;

	NewPhyType = HW_HAS_NEWER_PHY(pAC);

	if (ChipId == CHIP_ID_YUKON_FE || ChipId == CHIP_ID_YUKON_FE_P) {

		CableDiagOffs = PHY_MARV_FE_VCT_TX;
		FastEthernet = SK_TRUE;
		MdiPairs = 2;
	}
	else {
		CableDiagOffs = NewPhyType ? PHY_MARV_PHY_CTRL : PHY_MARV_CABLE_DIAG;

		if (ChipId == CHIP_ID_YUKON_EX) {

			CableDiagOffs += 7;		/* Advanced VCT Control Register */
		}

		FastEthernet = SK_FALSE;
		MdiPairs = 4;
	}

	if (StartTest) {

		/* set to RESET to avoid PortCheckUp */
		pPrt->PState = SK_PRT_RESET;

		/* only start the cable test */
		if (!FastEthernet) {

			if ((((pPrt->PhyId1 & PHY_I1_MOD_NUM) >> 4) == 2) &&
				 ((pPrt->PhyId1 & PHY_I1_REV_MSK) < 4)) {
				/* apply TDR workaround for model 2, rev. < 4 */
				SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PAGE_ADDR, 30);

				SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PAGE_DATA, 0xcc00);
				SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PAGE_DATA, 0xc800);
				SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PAGE_DATA, 0xc400);
				SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PAGE_DATA, 0xc000);
				SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PAGE_DATA, 0xc100);
			}

#ifdef YUKON_DBG
			if (pAC->GIni.GIChipId == CHIP_ID_YUKON_EC) {
				/* set address to 1 for page 1 */
				SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, 1);

				/* disable waiting period */
				SkGmPhyWrite(pAC, IoC, Port, CableDiagOffs,
					PHY_M_CABD_DIS_WAIT);
			}
#endif
			if (NewPhyType) {
				/* set address to 5 for page 5 */
				SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, 5);

#ifdef YUKON_DBG
				/* disable waiting period */
				SkGmPhyWrite(pAC, IoC, Port, CableDiagOffs + 1,
					PHY_M_CABD_DIS_WAIT);
#endif
			}
			else {
				/* set address to 0 for MDI[0] (Page 0) */
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

		/* start Cable Diagnostic Test */
		RegVal |= PHY_M_CABD_ENA_TEST;

		if (ChipId == CHIP_ID_YUKON_EX) {

			if (pAC->GIni.GIChipRev == CHIP_REV_YU_EX_A0) {
				/* apply TDR workaround */
				SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PAGE_ADDR, 13);

				SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PAGE_DATA, 0x4c32);
			}

			/* select first peak above threshold */
			RegVal |= PHY_M_CABD_TEST_MODE(1);

			/* read Transmit Pulse Reg */
			SkGmPhyRead(pAC, IoC, Port, CableDiagOffs + 5, &Ctrl);

			/* select 1/2 pulse width (64ns) */
			Ctrl |= PHY_M_CABD_PULS_WIDT(2);

			SkGmPhyWrite(pAC, IoC, Port, CableDiagOffs + 5, Ctrl);
		}

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

	if (ChipId == CHIP_ID_YUKON_EX) {
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

		pPrt->PMdiPairSts[i] = (SK_U8)((ChipId == CHIP_ID_YUKON_EX) ?
			(RegVal >> 8) : ((RegVal & PHY_M_CABD_STAT_MSK) >> 13));

		if (FastEthernet || NewPhyType) {
			/* get next register */
			CableDiagOffs++;
		}
	}

	return(0);
}	/* SkGmCableDiagStatus */
#endif /* !SK_SLIM */
#endif /* YUKON */

/* End of file */

