/******************************************************************************
 *
 * Name:	$Id: //Release/Yukon_1G/Shared/common/V6/skgesirq.c#12 $
 * Project:	Gigabit Ethernet Adapters, Common Modules
 * Version:	$Revision: #12 $, $Change: 5903 $
 * Date:	$DateTime: 2011/03/29 15:10:31 $
 * Purpose:	Special IRQ module
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

/*
 *	Special Interrupt handler
 *
 *	The following abstract should show how this module is included
 *	in the driver path:
 *
 *	In the ISR of the driver the bits for frame transmission complete and
 *	for receive complete are checked and handled by the driver itself.
 *	The bits of the slow path mask are checked after that and then the
 *	entry into the so-called "slow path" is prepared. It is an implementors
 *	decision whether this is executed directly or just scheduled by
 *	disabling the mask. In the interrupt service routine some events may be
 *	generated, so it would be a good idea to call the EventDispatcher
 *	right after this ISR.
 *
 *	The Interrupt source register of the adapter is NOT read by this module.
 *	SO if the drivers implementor needs a while loop around the
 *	slow data paths interrupt bits, he needs to call the SkGeSirqIsr() for
 *	each loop entered.
 *
 *	However, the MAC Interrupt status registers are read in a while loop.
 *
 */

#include "h/skdrv1st.h"		/* Driver Specific Definitions */
#ifndef SK_SLIM
#ifndef SK_NO_PNMI
#include "h/skgepnmi.h"		/* PNMI Definitions */
#endif /* !SK_NO_PNMI */
#ifndef SK_NO_RLMT
#include "h/skrlmt.h"		/* RLMT Definitions */
#endif /* !SK_NO_RLMT */
#endif
#include "h/skdrv2nd.h"		/* Adapter Control and Driver specific Def. */

/* local variables ************************************************************/

/* local function prototypes */
static int	SkGePortCheckUpGmac(SK_AC *, SK_IOC, int, SK_BOOL);
static void	SkPhyIsrGmac(SK_AC *, SK_IOC, int, SK_U16);


#ifndef SK_SLIM
/******************************************************************************
 *
 *	SkHWInitDefSense() - Default Autosensing mode initialization
 *
 * Description: sets the PLinkMode for HWInit
 *
 * Returns: N/A
 */
static void SkHWInitDefSense(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC,	/* I/O context */
int		Port)	/* Port Index (MAC_1 + n) */
{
	SK_GEPORT	*pPrt;		/* GIni Port struct pointer */

	pPrt = &pAC->GIni.GP[Port];

	if (pPrt->PLinkModeConf != (SK_U8)SK_LMODE_AUTOSENSE) {
		pPrt->PLinkMode = pPrt->PLinkModeConf;
		return;
	}

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
		("AutoSensing: First mode %d on Port %d\n",
		(int)SK_LMODE_AUTOFULL, Port));

	pPrt->PLinkMode = (SK_U8)SK_LMODE_AUTOFULL;

	return;
}	/* SkHWInitDefSense */
#endif


/******************************************************************************
 *
 *	SkHWLinkDown() - Link Down handling
 *
 * Description: handles the hardware link down signal
 *
 * Returns: N/A
 */
void SkHWLinkDown(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O context */
int		Port)		/* Port Index (MAC_1 + n) */
{
	SK_GEPORT	*pPrt;		/* GIni Port struct pointer */

	pPrt = &pAC->GIni.GP[Port];

	/* Disable all MAC interrupts */
	SkMacIrqDisable(pAC, IoC, Port);

	/* Disable Receiver and Transmitter */
	SkMacRxTxDisable(pAC, IoC, Port);

#ifndef SK_SLIM
	/* Init default sense mode */
	SkHWInitDefSense(pAC, IoC, Port);
#endif

	if (!pPrt->PHWLinkUp) {
		return;
	}

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("Link down Port %d\n", Port));

	/* Set Link to DOWN */
	pPrt->PHWLinkUp = SK_FALSE;
	pPrt->PPhyQLink = SK_FALSE;

#ifndef SK_SLIM
	/* Reset Port stati */
	pPrt->PLinkModeStatus = (SK_U8)SK_LMODE_STAT_UNKNOWN;
	pPrt->PFlowCtrlStatus = (SK_U8)SK_FLOW_STAT_NONE;
	pPrt->PLinkSpeedUsed = (SK_U8)SK_LSPEED_STAT_INDETERMINATED;
#endif /* !SK_SLIM */

	if (!pAC->GIni.GIDontInitPhy) {
		/* Re-init PHY */
		SkMacInitPhy(pAC, IoC, Port, SK_FALSE);
	}

	/* Do NOT signal to RLMT */

	/* Do NOT start the timer here */
}	/* SkHWLinkDown */


/******************************************************************************
 *
 *	SkHWLinkUp() - Link Up handling
 *
 * Description: handles the hardware link up signal
 *
 * Returns: N/A
 */
void SkHWLinkUp(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC,	/* I/O context */
int		Port)	/* Port Index (MAC_1 + n) */
{
	SK_GEPORT	*pPrt;		/* GIni Port struct pointer */

	pPrt = &pAC->GIni.GP[Port];

	if (pPrt->PHWLinkUp) {
		/* We do NOT need to proceed on active link */
		return;
	}

	pPrt->PHWLinkUp = SK_TRUE;

#ifndef SK_SLIM
	pPrt->PAutoNegFail = SK_FALSE;
	pPrt->PLinkModeStatus = (SK_U8)SK_LMODE_STAT_UNKNOWN;

	if (pPrt->PLinkMode != (SK_U8)SK_LMODE_AUTOHALF &&
		pPrt->PLinkMode != (SK_U8)SK_LMODE_AUTOFULL &&
		pPrt->PLinkMode != (SK_U8)SK_LMODE_AUTOBOTH) {
		/* Link is up and no Auto-negotiation should be done */

		/* Link speed should be the configured one */
		switch (pPrt->PLinkSpeed) {
		case SK_LSPEED_AUTO:
			/* default is 1000 Mbps */
		case SK_LSPEED_1000MBPS:
			pPrt->PLinkSpeedUsed = (SK_U8)
				((pPrt->PLinkSpeedCap & SK_LSPEED_CAP_1000MBPS) != 0) ?
				 SK_LSPEED_STAT_1000MBPS : SK_LSPEED_STAT_100MBPS;
			break;
		case SK_LSPEED_100MBPS:
			pPrt->PLinkSpeedUsed = (SK_U8)SK_LSPEED_STAT_100MBPS;
			break;
		case SK_LSPEED_10MBPS:
			pPrt->PLinkSpeedUsed = (SK_U8)SK_LSPEED_STAT_10MBPS;
			break;
		}

		/* Set Link Mode Status */
		pPrt->PLinkModeStatus = (SK_U8)((pPrt->PLinkMode == SK_LMODE_FULL) ?
			 SK_LMODE_STAT_FULL : SK_LMODE_STAT_HALF);

		/* Set Flow Control Status using Flow Control Mode */
		switch (pPrt->PFlowCtrlMode) {

		case SK_FLOW_MODE_NONE:
			pPrt->PFlowCtrlStatus = SK_FLOW_STAT_NONE;
			break;

		case SK_FLOW_MODE_LOC_SEND:
			pPrt->PFlowCtrlStatus = SK_FLOW_STAT_LOC_SEND;
			break;

		case SK_FLOW_MODE_SYMMETRIC:
			pPrt->PFlowCtrlStatus = SK_FLOW_STAT_SYMMETRIC;
			break;

		case SK_FLOW_MODE_SYM_OR_REM:
			pPrt->PFlowCtrlStatus = SK_FLOW_STAT_REM_SEND;
			break;

		default:
			pPrt->PFlowCtrlStatus = SK_FLOW_STAT_INDETERMINATED;
			break;
		}
#endif /* !SK_SLIM */

		/* enable Rx/Tx */
		(void)SkMacRxTxEnable(pAC, IoC, Port);
#ifndef SK_SLIM
	}
#endif /* !SK_SLIM */
}	/* SkHWLinkUp */


#ifndef DISABLE_YUKON_I
/******************************************************************************
 *
 *	SkMacParity() - MAC parity workaround
 *
 * Description: handles MAC parity errors correctly
 *
 * Returns: N/A
 */
static void SkMacParity(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC,	/* I/O context */
int		Port)	/* Port Index (MAC_1 + n) */
{
	SK_EVPARA	Para;
	SK_GEPORT	*pPrt;		/* GIni Port struct pointer */
	SK_U32		TxMax;		/* Tx Max Size Counter */

	TxMax = 0;

	pPrt = &pAC->GIni.GP[Port];

	/* clear IRQ Tx Parity Error */
	/* HW-Bug #8: cleared by GMF_CLI_TX_FC instead of GMF_CLI_TX_PE */
	SK_OUT8(IoC, MR_ADDR(Port, TX_GMF_CTRL_T), (SK_U8)
		((pAC->GIni.GIChipId == CHIP_ID_YUKON &&
		 pAC->GIni.GIChipRev == 0) ? GMF_CLI_TX_FC : GMF_CLI_TX_PE));

	if (pPrt->PCheckPar) {

		if (Port == MAC_1) {
			SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E016, SKERR_SIRQ_E016MSG);
		}
		else {
			SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E017, SKERR_SIRQ_E017MSG);
		}
		Para.Para64 = Port;
		SkEventQueue(pAC, SKGE_DRV, SK_DRV_PORT_FAIL, Para);

		Para.Para32[0] = Port;
		SkEventQueue(pAC, SKGE_RLMT, SK_RLMT_LINK_DOWN, Para);

		return;
	}

	/* Check whether frames with a size of 1k were sent */
	(void)SkGmMacStatistic(pAC, IoC, Port, GM_TXF_1518B, &TxMax);

	if (TxMax > 0) {
		/* From now on check the parity */
		pPrt->PCheckPar = SK_TRUE;
	}
}	/* SkMacParity */

/******************************************************************************
 *
 *	SkGeYuHwErr() - Hardware Error service routine (Genesis and Yukon)
 *
 * Description: handles all HW Error interrupts
 *
 * Returns: N/A
 */
static void SkGeYuHwErr(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O context */
SK_U32	HwStatus)	/* Interrupt status word */
{
	SK_EVPARA	Para;
	SK_U16		Word;

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
		("HW-Error Status: 0x%08lX\n", HwStatus));

	if ((HwStatus & (IS_IRQ_MST_ERR | IS_IRQ_STAT)) != 0) {
		/* PCI Errors occurred */
		if ((HwStatus & IS_IRQ_STAT) != 0) {
			SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E013, SKERR_SIRQ_E013MSG);
		}
		else {
			SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E012, SKERR_SIRQ_E012MSG);
		}

		/* Reset all bits in the PCI STATUS register */
		SK_IN16(IoC, PCI_C2(pAC, IoC, PCI_STATUS), &Word);

		SK_TST_MODE_ON(IoC);
		SK_OUT16(IoC, PCI_C2(pAC, IoC, PCI_STATUS), (SK_U16)(Word | PCI_ERRBITS));
		SK_TST_MODE_OFF(IoC);

		Para.Para64 = 0;
		SkEventQueue(pAC, SKGE_DRV, SK_DRV_ADAP_FAIL, Para);
	}

	/* This is necessary only for Rx timing measurements */
	if ((HwStatus & IS_IRQ_TIST_OV) != 0) {
		/* increment Time Stamp Timer counter (high) */
#ifndef SK_SLIM
		pAC->GIni.GITimeStampCnt++;
#endif
		/* clear Time Stamp Timer IRQ */
		SK_OUT8(IoC, GMAC_TI_ST_CTRL, (SK_U8)GMT_ST_CLR_IRQ);
	}

	if ((HwStatus & IS_IRQ_SENSOR) != 0) {
		/* no sensors on 32-bit Yukon */
		if (pAC->GIni.GIYukon32Bit) {
			/* disable HW Error IRQ */
			pAC->GIni.GIValIrqMask &= ~IS_HW_ERR;
		}
	}

	if ((HwStatus & IS_RAM_RD_PAR) != 0) {

		SK_OUT16(IoC, B3_RI_CTRL, RI_CLR_RD_PERR);

		SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E014, SKERR_SIRQ_E014MSG);
		Para.Para64 = 0;
		SkEventQueue(pAC, SKGE_DRV, SK_DRV_ADAP_FAIL, Para);
	}

	if ((HwStatus & IS_RAM_WR_PAR) != 0) {

		SK_OUT16(IoC, B3_RI_CTRL, RI_CLR_WR_PERR);

		SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E015, SKERR_SIRQ_E015MSG);
		Para.Para64 = 0;
		SkEventQueue(pAC, SKGE_DRV, SK_DRV_ADAP_FAIL, Para);
	}

	if ((HwStatus & IS_M1_PAR_ERR) != 0) {
		SkMacParity(pAC, IoC, MAC_1);
	}

	if ((HwStatus & IS_M2_PAR_ERR) != 0) {
		SkMacParity(pAC, IoC, MAC_2);
	}

	if ((HwStatus & IS_R1_PAR_ERR) != 0) {
		/* Clear IRQ */
		SK_OUT32(IoC, B0_R1_CSR, CSR_IRQ_CL_P);

		SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E018, SKERR_SIRQ_E018MSG);
		Para.Para64 = MAC_1;
		SkEventQueue(pAC, SKGE_DRV, SK_DRV_PORT_FAIL, Para);

		Para.Para32[0] = MAC_1;
		SkEventQueue(pAC, SKGE_RLMT, SK_RLMT_LINK_DOWN, Para);
	}

	if ((HwStatus & IS_R2_PAR_ERR) != 0) {
		/* Clear IRQ */
		SK_OUT32(IoC, B0_R2_CSR, CSR_IRQ_CL_P);

		SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E019, SKERR_SIRQ_E019MSG);
		Para.Para64 = MAC_2;
		SkEventQueue(pAC, SKGE_DRV, SK_DRV_PORT_FAIL, Para);

		Para.Para32[0] = MAC_2;
		SkEventQueue(pAC, SKGE_RLMT, SK_RLMT_LINK_DOWN, Para);
	}
}	/* SkGeYuHwErr */
#endif /* !DISABLE_YUKON_I */

#ifdef YUK2
/******************************************************************************
 *
 *	SkYuk2HwPortErr() - Service HW Errors for specified port (Yukon-2 only)
 *
 * Description: handles the HW Error interrupts for a specific port.
 *
 * Returns: N/A
 */
static void SkYuk2HwPortErr(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
SK_U32	HwStatus,	/* Interrupt status word */
int		Port)		/* Port Index (MAC_1 + n) */
{
	SK_EVPARA	Para;
	int			Queue;

	if (Port == MAC_2) {
		HwStatus >>= 8;
	}

	if ((HwStatus & Y2_HWE_L1_MASK) == 0) {
		return;
	}

	if ((HwStatus & Y2_IS_PAR_RD1) != 0) {
		/* Clear IRQ */
		SK_OUT16(IoC, SELECT_RAM_BUFFER(Port, B3_RI_CTRL), RI_CLR_RD_PERR);

		if (Port == MAC_1) {
			SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E028, SKERR_SIRQ_E028MSG);
		}
		else {
			SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E030, SKERR_SIRQ_E030MSG);
		}
	}

	if ((HwStatus & Y2_IS_PAR_WR1) != 0) {
		/* Clear IRQ */
		SK_OUT16(IoC, SELECT_RAM_BUFFER(Port, B3_RI_CTRL), RI_CLR_WR_PERR);

		if (Port == MAC_1) {
			SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E029, SKERR_SIRQ_E029MSG);
		}
		else {
			SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E031, SKERR_SIRQ_E031MSG);
		}
	}

	if ((HwStatus & Y2_IS_PAR_MAC1) != 0) {
		/* Clear IRQ */
		SK_OUT8(IoC, MR_ADDR(Port, TX_GMF_CTRL_T), GMF_CLI_TX_PE);

		if (Port == MAC_1) {
			SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E016, SKERR_SIRQ_E016MSG);
		}
		else {
			SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E017, SKERR_SIRQ_E017MSG);
		}
	}

	if ((HwStatus & Y2_IS_PAR_RX1) != 0) {
		if (Port == MAC_1) {
			Queue = Q_R1;
			SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E018, SKERR_SIRQ_E018MSG);
		}
		else {
			Queue = Q_R2;
			SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E019, SKERR_SIRQ_E019MSG);
		}
		/* Clear IRQ */
		SK_OUT32(IoC, Q_ADDR(Queue, Q_CSR), BMU_CLR_IRQ_PAR);
	}

	if ((HwStatus & Y2_IS_TCP_TXS1) != 0) {
		if (Port == MAC_1) {
			Queue = Q_XS1;
			SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E033, SKERR_SIRQ_E033MSG);
		}
		else {
			Queue = Q_XS2;
			SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E035, SKERR_SIRQ_E035MSG);
		}
		/* Clear IRQ */
		SK_OUT32(IoC, Q_ADDR(Queue, Q_CSR), BMU_CLR_IRQ_TCP);
	}

	if ((HwStatus & Y2_IS_TCP_TXA1) != 0) {
		if (Port == MAC_1) {
			Queue = Q_XA1;
			SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E032, SKERR_SIRQ_E032MSG);
		}
		else {
			Queue = Q_XA2;
			SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E034, SKERR_SIRQ_E034MSG);
		}
		/* Clear IRQ */
		SK_OUT32(IoC, Q_ADDR(Queue, Q_CSR), BMU_CLR_IRQ_TCP);
	}

	Para.Para64 = Port;
	SkEventQueue(pAC, SKGE_DRV, SK_DRV_PORT_FAIL, Para);

	Para.Para32[0] = Port;
	SkEventQueue(pAC, SKGE_RLMT, SK_RLMT_LINK_DOWN, Para);

}	/* SkYuk2HwPortErr */

/******************************************************************************
 *
 *	SkYuk2HwErr() - Hardware Error service routine (Yukon-2 only)
 *
 * Description: handles all HW Error interrupts
 *
 * Returns: N/A
 */
static void SkYuk2HwErr(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
SK_U32	HwStatus)	/* Interrupt status word */
{
	SK_EVPARA	Para;
	SK_U16		Word;
	SK_U32		DWord;
	SK_U32		TlpHead[4];
	int			i;

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
		("HW-Error Status: 0x%08lX\n", HwStatus));

	/* This is necessary only for Rx timing measurements */
	if ((HwStatus & Y2_IS_TIST_OV) != 0) {
#ifndef SK_SLIM
		/* increment Time Stamp Timer counter (high) */
		pAC->GIni.GITimeStampCnt++;
#endif
		/* clear Time Stamp Timer IRQ */
		SK_OUT8(IoC, GMAC_TI_ST_CTRL, (SK_U8)GMT_ST_CLR_IRQ);
	}

	/* Evaluate Y2_IS_PCI_NEXP before Y2_IS_MST_ERR or Y2_IS_IRQ_STAT */
	if ((HwStatus & Y2_IS_PCI_NEXP) != 0) {
		/*
		 * This error is also mapped either to Master Abort (Y2_IS_MST_ERR)
		 * or Target Abort (Y2_IS_IRQ_STAT) bit and can only be cleared there.
		 * Therefore handle this event just by printing an error log entry.
		 */
		SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E027, SKERR_SIRQ_E027MSG);
	}

	if ((HwStatus & (Y2_IS_MST_ERR | Y2_IS_IRQ_STAT)) != 0) {
		/* PCI Errors occurred */
		if ((HwStatus & Y2_IS_IRQ_STAT) != 0) {
			SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E013, SKERR_SIRQ_E013MSG);
		}
		else {
			SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E012, SKERR_SIRQ_E012MSG);
		}

		/* Reset all bits in the PCI STATUS register */
		SK_IN16(IoC, PCI_C2(pAC, IoC, PCI_STATUS), &Word);

		SK_TST_MODE_ON(IoC);
		SK_OUT16(IoC, PCI_C2(pAC, IoC, PCI_STATUS), (SK_U16)(Word | PCI_ERRBITS));
		SK_TST_MODE_OFF(IoC);

		Para.Para64 = 0;
		SkEventQueue(pAC, SKGE_DRV, SK_DRV_ADAP_FAIL, Para);
	}

	/* check for PCI-Express Uncorrectable Error*/
	if ((HwStatus & Y2_IS_PCI_EXP) != 0) {
		/*
		 * On PCI-Express bus bridges are called root complexes (RC).
		 * PCI-Express errors are recognized by the root complex too,
		 * which requests the system to handle the problem. After error
		 * occurence it may be that no access to the adapter may be performed
		 * any longer.
		 */

		/* Get uncorrectable error status */
		SK_IN32(IoC, PCI_C2(pAC, IoC, PEX_UNC_ERR_STAT), &DWord);

		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
			("PEX Uncorr.Error Status: 0x%08lX\n", DWord));

		/* check for Flow Control Protocol Error (FCPE) */
		if (DWord == PEX_FLOW_CTRL_P &&
			/* check if ASPM L1 enabled */
			(pAC->GIni.GIPexLinkCtrl & PEX_LC_ASPM_LC_L1) != 0) {

			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
				("Flow Control Protocol Error (FCPE)\n"));
		}
		else if (DWord != PEX_UNSUP_REQ) {
			/* ignore Unsupported Request Errors */
			SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E026, SKERR_SIRQ_E026MSG);

			if ((DWord & (PEX_FATAL_ERRORS | PEX_POIS_TLP)) != 0) {
				/*
				 * Stop only, if the uncorrectable error is fatal or
				 * Poisoned TLP occurred
				 */
				SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ, ("Header Log:"));

				for (i = 0; i < 4; i++) {
					/* get TLP Header from Log Registers */
					SK_IN32(IoC, PCI_C2(pAC, IoC, PEX_HEADER_LOG + i*4), &TlpHead[i]);

					SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
						(" 0x%08lX", TlpHead[i]));
				}
				SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ, ("\n"));

				/* check for vendor defined broadcast message */
				if (TlpHead[0] == 0x73004001 && (SK_U8)TlpHead[1] == 0x7f) {

					SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
						("Vendor defined broadcast message\n"));
				}
				else {
					Para.Para64 = 0;
					SkEventQueue(pAC, SKGE_DRV, SK_DRV_ADAP_FAIL, Para);

					pAC->GIni.GIValHwIrqMask &= ~Y2_IS_PCI_EXP;
					/* Rewrite HW IRQ mask */
					SK_OUT32(IoC, B0_HWE_IMSK, pAC->GIni.GIValHwIrqMask);
				}
			}
		}

		/* clear any PEX errors */
		SK_TST_MODE_ON(IoC);
		SK_OUT32(IoC, PCI_C2(pAC, IoC, PEX_UNC_ERR_STAT), ALL_32_BITS);
		SK_TST_MODE_OFF(IoC);

		SK_IN32(IoC, PCI_C2(pAC, IoC, PEX_UNC_ERR_STAT), &DWord);

		if ((DWord & PEX_RX_OV) != 0) {
			/* Dev #4.205 occurred */
			pAC->GIni.GIValHwIrqMask &= ~Y2_IS_PCI_EXP;

			pAC->GIni.GIValIrqMask &= ~Y2_IS_HW_ERR;
		}
	}

	for (i = 0; i < pAC->GIni.GIMacsFound; i++) {

		SkYuk2HwPortErr(pAC, IoC, HwStatus, i);
	}

}	/* SkYuk2HwErr */
#endif /* YUK2 */

/******************************************************************************
 *
 *	SkGeSirqIsr() - Wrapper for Special Interrupt Service Routine
 *
 * Description: calls the preselected special ISR (slow path)
 *
 * Returns: N/A
 */
void SkGeSirqIsr(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O context */
SK_U32	Istatus)	/* Interrupt status word */
{
	pAC->GIni.GIFunc.pSkGeSirqIsr(pAC, IoC, Istatus);
}
#ifndef DISABLE_YUKON_I
/******************************************************************************
 *
 *	SkGeYuSirqIsr() - Special Interrupt Service Routine
 *
 * Description: handles all non data transfer specific interrupts (slow path)
 *
 * Returns: N/A
 */
void SkGeYuSirqIsr(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
SK_U32	Istatus)	/* Interrupt status word */
{
	SK_EVPARA	Para;
	SK_U32		RegVal32;	/* Read register value */
	SK_GEPORT	*pPrt;		/* GIni Port struct pointer */
	SK_U16		PhyInt;
	int			i;
	int			Rtv;

	if (((Istatus & IS_HW_ERR) & pAC->GIni.GIValIrqMask) != 0) {
		/* read the HW Error Interrupt source */
		SK_IN32(IoC, B0_HWE_ISRC, &RegVal32);

		SkGeYuHwErr(pAC, IoC, RegVal32);
	}

	/*
	 * Packet Timeout interrupts
	 */
	/* Check whether MACs are correctly initialized */
	if (((Istatus & (IS_PA_TO_RX1 | IS_PA_TO_TX1)) != 0) &&
		pAC->GIni.GP[MAC_1].PState == SK_PRT_RESET) {
		/* MAC 1 was not initialized but Packet timeout occurred */
		SK_ERR_LOG(pAC, SK_ERRCL_SW | SK_ERRCL_INIT, SKERR_SIRQ_E004,
			SKERR_SIRQ_E004MSG);
	}

	if (((Istatus & (IS_PA_TO_RX2 | IS_PA_TO_TX2)) != 0) &&
		pAC->GIni.GP[MAC_2].PState == SK_PRT_RESET) {
		/* MAC 2 was not initialized but Packet timeout occurred */
		SK_ERR_LOG(pAC, SK_ERRCL_SW | SK_ERRCL_INIT, SKERR_SIRQ_E005,
			SKERR_SIRQ_E005MSG);
	}

	if ((Istatus & IS_PA_TO_RX1) != 0) {
		/* Means network is filling us up */
		SK_ERR_LOG(pAC, SK_ERRCL_HW | SK_ERRCL_INIT, SKERR_SIRQ_E002,
			SKERR_SIRQ_E002MSG);
		SK_OUT16(IoC, B3_PA_CTRL, PA_CLR_TO_RX1);
	}

	if ((Istatus & IS_PA_TO_RX2) != 0) {
		/* Means network is filling us up */
		SK_ERR_LOG(pAC, SK_ERRCL_HW | SK_ERRCL_INIT, SKERR_SIRQ_E003,
			SKERR_SIRQ_E003MSG);
		SK_OUT16(IoC, B3_PA_CTRL, PA_CLR_TO_RX2);
	}

	if ((Istatus & IS_PA_TO_TX1) != 0) {

		pPrt = &pAC->GIni.GP[MAC_1];

		/* May be a normal situation in a server with a slow network */
		SK_OUT16(IoC, B3_PA_CTRL, PA_CLR_TO_TX1);
	}

	if ((Istatus & IS_PA_TO_TX2) != 0) {

		pPrt = &pAC->GIni.GP[MAC_2];

		/* May be a normal situation in a server with a slow network */
		SK_OUT16(IoC, B3_PA_CTRL, PA_CLR_TO_TX2);
	}

	/* Check interrupts of the particular queues */
	if ((Istatus & IS_R1_C) != 0) {
		/* Clear IRQ */
		SK_OUT32(IoC, B0_R1_CSR, CSR_IRQ_CL_C);

		SK_ERR_LOG(pAC, SK_ERRCL_SW | SK_ERRCL_INIT, SKERR_SIRQ_E006,
			SKERR_SIRQ_E006MSG);
		Para.Para64 = MAC_1;
		SkEventQueue(pAC, SKGE_DRV, SK_DRV_PORT_FAIL, Para);
		Para.Para32[0] = MAC_1;
		SkEventQueue(pAC, SKGE_RLMT, SK_RLMT_LINK_DOWN, Para);
	}

	if ((Istatus & IS_R2_C) != 0) {
		/* Clear IRQ */
		SK_OUT32(IoC, B0_R2_CSR, CSR_IRQ_CL_C);

		SK_ERR_LOG(pAC, SK_ERRCL_SW | SK_ERRCL_INIT, SKERR_SIRQ_E007,
			SKERR_SIRQ_E007MSG);
		Para.Para64 = MAC_2;
		SkEventQueue(pAC, SKGE_DRV, SK_DRV_PORT_FAIL, Para);
		Para.Para32[0] = MAC_2;
		SkEventQueue(pAC, SKGE_RLMT, SK_RLMT_LINK_DOWN, Para);
	}

	if ((Istatus & IS_XS1_C) != 0) {
		/* Clear IRQ */
		SK_OUT32(IoC, B0_XS1_CSR, CSR_IRQ_CL_C);

		SK_ERR_LOG(pAC, SK_ERRCL_SW | SK_ERRCL_INIT, SKERR_SIRQ_E008,
			SKERR_SIRQ_E008MSG);
		Para.Para64 = MAC_1;
		SkEventQueue(pAC, SKGE_DRV, SK_DRV_PORT_FAIL, Para);
		Para.Para32[0] = MAC_1;
		SkEventQueue(pAC, SKGE_RLMT, SK_RLMT_LINK_DOWN, Para);
	}

	if ((Istatus & IS_XA1_C) != 0) {
		/* Clear IRQ */
		SK_OUT32(IoC, B0_XA1_CSR, CSR_IRQ_CL_C);

		SK_ERR_LOG(pAC, SK_ERRCL_SW | SK_ERRCL_INIT, SKERR_SIRQ_E009,
			SKERR_SIRQ_E009MSG);
		Para.Para64 = MAC_1;
		SkEventQueue(pAC, SKGE_DRV, SK_DRV_PORT_FAIL, Para);
		Para.Para32[0] = MAC_1;
		SkEventQueue(pAC, SKGE_RLMT, SK_RLMT_LINK_DOWN, Para);
	}

	if ((Istatus & IS_XS2_C) != 0) {
		/* Clear IRQ */
		SK_OUT32(IoC, B0_XS2_CSR, CSR_IRQ_CL_C);

		SK_ERR_LOG(pAC, SK_ERRCL_SW | SK_ERRCL_INIT, SKERR_SIRQ_E010,
			SKERR_SIRQ_E010MSG);
		Para.Para64 = MAC_2;
		SkEventQueue(pAC, SKGE_DRV, SK_DRV_PORT_FAIL, Para);
		Para.Para32[0] = MAC_2;
		SkEventQueue(pAC, SKGE_RLMT, SK_RLMT_LINK_DOWN, Para);
	}

	if ((Istatus & IS_XA2_C) != 0) {
		/* Clear IRQ */
		SK_OUT32(IoC, B0_XA2_CSR, CSR_IRQ_CL_C);

		SK_ERR_LOG(pAC, SK_ERRCL_SW | SK_ERRCL_INIT, SKERR_SIRQ_E011,
			SKERR_SIRQ_E011MSG);
		Para.Para64 = MAC_2;
		SkEventQueue(pAC, SKGE_DRV, SK_DRV_PORT_FAIL, Para);
		Para.Para32[0] = MAC_2;
		SkEventQueue(pAC, SKGE_RLMT, SK_RLMT_LINK_DOWN, Para);
	}

	/* External reg interrupt */
	if ((Istatus & IS_EXT_REG) != 0) {
		/* Test IRQs from PHY */
		for (i = 0; i < pAC->GIni.GIMacsFound; i++) {

			pPrt = &pAC->GIni.GP[i];

			if (pPrt->PState == SK_PRT_RESET) {
				continue;
			}

			/* Read PHY Interrupt Status */
			Rtv = SkGmPhyRead(pAC, IoC, i, PHY_MARV_INT_STAT, &PhyInt);

			if (Rtv == 0 && (PhyInt & PHY_M_DEF_MSK) != 0) {
				SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
					("Port %d PHY Int: 0x%04X\n", i, PhyInt));

				SkPhyIsrGmac(pAC, IoC, i, PhyInt);
			}
		}
	}

	/* TWSI Ready interrupt */
	if ((Istatus & IS_I2C_READY) != 0) {
#ifdef SK_SLIM
		/* clear TWSI IRQ */
		SK_OUT32(IoC, B2_I2C_IRQ, I2C_CLR_IRQ);
#else /* SK_SLIM */
		SkI2cIsr(pAC, IoC);
#endif /* !SK_SLIM */
	}

	/* SW forced interrupt */
	if ((Istatus & IS_IRQ_SW) != 0) {
		/* clear the software IRQ */
		SK_OUT8(IoC, B0_CTST, CS_CL_SW_IRQ);
	}

	if ((Istatus & IS_LNK_SYNC_M1) != 0) {
		/*
		 * We do NOT need the Link Sync interrupt, because it shows
		 * only a link going down.
		 */
		/* clear interrupt */
		SK_OUT8(IoC, MR_ADDR(MAC_1, LNK_SYNC_CTRL), LNK_CLR_IRQ);
	}

	/* Check MAC after link sync counter */
	if ((Istatus & IS_MAC1) != 0) {
		/* IRQ from MAC 1 */
		SkMacIrq(pAC, IoC, MAC_1);
	}

	if ((Istatus & IS_LNK_SYNC_M2) != 0) {
		/*
		 * We do NOT need the Link Sync interrupt, because it shows
		 * us only a link going down.
		 */
		/* clear interrupt */
		SK_OUT8(IoC, MR_ADDR(MAC_2, LNK_SYNC_CTRL), LNK_CLR_IRQ);
	}

	/* Check MAC after link sync counter */
	if ((Istatus & IS_MAC2) != 0) {
		/* IRQ from MAC 2 */
		SkMacIrq(pAC, IoC, MAC_2);
	}

	/* Timer interrupt (served last) */
	if ((Istatus & IS_TIMINT) != 0) {
		/* check for HW Errors */
		if (((Istatus & IS_HW_ERR) & ~pAC->GIni.GIValIrqMask) != 0) {
			/* read the HW Error Interrupt source */
			SK_IN32(IoC, B0_HWE_ISRC, &RegVal32);

			SkGeYuHwErr(pAC, IoC, RegVal32);
		}

		SkHwtIsr(pAC, IoC);
	}

}	/* SkGeYuSirqIsr */
#endif /* !DISABLE_YUKON_I */
#ifdef YUK2
/******************************************************************************
 *
 *	SkYuk2PortSirq() - Service HW Errors for specified port (Yukon-2 only)
 *
 * Description: handles the HW Error interrupts for a specific port.
 *
 * Returns: N/A
 */
static void SkYuk2PortSirq(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
SK_U32	IStatus,	/* Interrupt status word */
int		Port)		/* Port Index (MAC_1 + n) */
{
	SK_EVPARA	Para;
	int			Queue;
	int			Rtv;
	SK_U16		PhyInt;
#ifdef DEBUG
	SK_U32		Time;
#endif /* DEBUG */

	if (Port == MAC_2) {
		IStatus >>= 8;
	}

	/* Interrupt from PHY */
	if ((IStatus & Y2_IS_IRQ_PHY1) != 0 &&
		pAC->GIni.GP[Port].PState != SK_PRT_RESET) {
		/* Read PHY Interrupt Status */
		Rtv = SkGmPhyRead(pAC, IoC, Port, PHY_MARV_INT_STAT, &PhyInt);

		if (Rtv == 0 && (PhyInt & PHY_M_DEF_MSK) != 0) {
			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
				("Port %d PHY Int: 0x%04X\n", Port, PhyInt));

			SkPhyIsrGmac(pAC, IoC, Port, PhyInt);
		}
	}

	if ((IStatus & Y2_IS_PHY_QLNK & pAC->GIni.GIValIrqMask) != 0) {

#if defined(DEBUG) && !defined(SK_SLIM)
		Time = (SK_U32)SkOsGetTime(pAC) - pAC->GIni.GIPhyInitTime;

		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
			("PHY Quick Link IRQ (%lu tics, %u.%u sec)\n", Time,
			 (unsigned)(Time / SK_TICKS_PER_SEC),
			 (unsigned)((Time % SK_TICKS_PER_SEC) * 10) / SK_TICKS_PER_SEC));
#endif /* DEBUG && !SK_SLIM */

		pAC->GIni.GP[Port].PPhyQLink = SK_TRUE;

		/* disable PHY Quick Link */
		pAC->GIni.GIValIrqMask &= ~Y2_IS_PHY_QLNK;

		SK_OUT32(IoC, B0_IMSK, pAC->GIni.GIValIrqMask);

		/* reset PHY Link Detect */
		SK_IN16(IoC, PCI_C(pAC, PSM_CONFIG_REG4), &PhyInt);

		SK_TST_MODE_ON(IoC);
		SK_OUT16(IoC, PCI_C(pAC, PSM_CONFIG_REG4), PhyInt | BIT_0S);
		SK_TST_MODE_OFF(IoC);

		Para.Para32[0] = Port;
		SkEventQueue(pAC, SKGE_DRV, SK_DRV_LINK_UP, Para);
	}

	/* Interrupt from MAC */
	if ((IStatus & Y2_IS_IRQ_MAC1) != 0) {
		SkMacIrq(pAC, IoC, Port);
	}

	if ((IStatus & (Y2_IS_CHK_RX1 | Y2_IS_CHK_TXS1 | Y2_IS_CHK_TXA1)) != 0) {
		if ((IStatus & Y2_IS_CHK_RX1) != 0) {
			if (Port == MAC_1) {
				Queue = Q_R1;
				SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E006,
					SKERR_SIRQ_E006MSG);
			}
			else {
				Queue = Q_R2;
				SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E007,
					SKERR_SIRQ_E007MSG);
			}
			/* Clear IRQ */
			SK_OUT32(IoC, Q_ADDR(Queue, Q_CSR), BMU_CLR_IRQ_CHK);
		}

		if ((IStatus & Y2_IS_CHK_TXS1) != 0) {
			if (Port == MAC_1) {
				Queue = Q_XS1;
				SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E008,
					SKERR_SIRQ_E008MSG);
			}
			else {
				Queue = Q_XS2;
				SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E010,
					SKERR_SIRQ_E010MSG);
			}
			/* Clear IRQ */
			SK_OUT32(IoC, Q_ADDR(Queue, Q_CSR), BMU_CLR_IRQ_CHK);
		}

		if ((IStatus & Y2_IS_CHK_TXA1) != 0) {
			if (Port == MAC_1) {
				Queue = Q_XA1;
				SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E009,
					SKERR_SIRQ_E009MSG);
			}
			else {
				Queue = Q_XA2;
				SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E011,
					SKERR_SIRQ_E011MSG);
			}
			/* Clear IRQ */
			SK_OUT32(IoC, Q_ADDR(Queue, Q_CSR), BMU_CLR_IRQ_CHK);
		}

		Para.Para64 = Port;
		SkEventQueue(pAC, SKGE_DRV, SK_DRV_PORT_FAIL, Para);

		Para.Para32[0] = Port;
		SkEventQueue(pAC, SKGE_RLMT, SK_RLMT_LINK_DOWN, Para);
	}
}	/* SkYuk2PortSirq */
#endif /* YUK2 */

/******************************************************************************
 *
 *	SkYuk2SirqIsr() - Special Interrupt Service Routine (Yukon-2 only)
 *
 * Description: handles all non data transfer specific interrupts (slow path)
 *
 * Returns: N/A
 */
void SkYuk2SirqIsr(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
SK_U32	Istatus)	/* Interrupt status word */
{
#ifdef YUK2
	SK_GEPORT	*pPrt;		/* GIni Port struct pointer */
	SK_EVPARA	Para;
	SK_U8		Byte;
	SK_U16		Word;
	SK_U32		DWord;
	SK_U32		RegVal32;	/* Read register value */

	pPrt = &pAC->GIni.GP[MAC_1];

	/* HW Error indicated ? */
	if (((Istatus & Y2_IS_HW_ERR) & pAC->GIni.GIValIrqMask) != 0) {
		/* read the HW Error Interrupt source */
		SK_IN32(IoC, B0_HWE_ISRC, &RegVal32);

		SkYuk2HwErr(pAC, IoC, RegVal32);
	}

	/* Interrupt from ASF Subsystem */
	if ((Istatus & Y2_IS_ASF) != 0) {

		if (pAC->GIni.GIChipId == CHIP_ID_YUKON_EX ||
			pAC->GIni.GIChipId == CHIP_ID_YUKON_SUPR) {
			/* clear IRQ */
			SK_IN32(IoC, HCU_CCSR, &DWord);

			SK_OUT32(IoC, HCU_CCSR, DWord | HCU_CCSR_CLR_IRQ_HOST);

#ifdef USE_HCI2HOST_INT
			Para.Para64 = 0;
			SkEventQueue(pAC, SKGE_ASF, 3, Para);
#endif
		}
		else { /* Yukon-2, -EC */
			/* clear IRQ */
			/* later on clearing should be done in ASF ISR handler */
			SK_IN8(IoC, B28_Y2_ASF_STAT_CMD, &Byte);

			SK_OUT8(IoC, B28_Y2_ASF_STAT_CMD, Byte | (SK_U8)Y2_ASF_CLR_HSTI);
			/* Call IRQ handler in ASF Module */
			/* TBD */
		}
	}

	/* check IRQ from polling unit */
	if ((Istatus & Y2_IS_POLL_CHK) != 0) {
		/* Clear IRQ Check */
		SK_OUT8(IoC, POLL_CTRL, PC_CLR_IRQ_CHK);

		SK_ERR_LOG(pAC, SK_ERRCL_SW | SK_ERRCL_INIT, SKERR_SIRQ_E036,
			SKERR_SIRQ_E036MSG);
		Para.Para64 = 0;
		SkEventQueue(pAC, SKGE_DRV, SK_DRV_ADAP_FAIL, Para);
	}

	/* TWSI Ready interrupt */
	if ((Istatus & Y2_IS_TWSI_RDY) != 0) {

		/* clear TWSI IRQ */
		SK_OUT32(IoC, B2_I2C_IRQ, I2C_CLR_IRQ);

		/* TWSI IRQ is Pause Packet IRQ */
		if (HW_FEATURE(pAC, HWF_NEW_FLOW_CONTROL) &&
			pPrt->PLinkSpeedUsed == (SK_U8)SK_LSPEED_STAT_1000MBPS) {

			/* get number of Xmitted Pause Packets */
			(void)SkGmMacStatistic(pAC, IoC, MAC_1, GM_TXF_MPAUSE, &DWord);

			Word = (SK_U16)(DWord - pPrt->PPauseTxCnt);

			/* save the value */
			pPrt->PPauseTxCnt = DWord;

#ifdef DEBUG
			if (Word != 1) {
				SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
					("Pause Packets ON & OFF, %u\n", Word));
			}
#endif /* DEBUG */

			if ((Word % 2) == 1) {
				/* toggle Pause Packet (On/Off) */
				pPrt->PPauseFlag ^= SK_TRUE;
			}

			/* get Rx GMAC FIFO Write Level */
			SK_IN16(IoC, RX_GMF_WLEV, &Word);

			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
				("Pause Packet IRQ, Level: 0x%03X, Cnt = %u, Pause %s\n",
				 Word, DWord, pPrt->PPauseFlag ? "ON" : "off"));

			if (pPrt->PPauseFlag) {
				/* check if a stopped 30ms timer IRQ is pending */
				if ((Istatus & Y2_IS_TIMINT) != 0) {
					/* clear timer IRQ */
					SK_OUT8(IoC, B2_TI_CTRL, TIM_CLR_IRQ);

					/* to avoid handling this timer IRQ */
					Istatus &= ~Y2_IS_TIMINT;
				}

				if (Word > (SK_U16)SK_ECU_LLPP) {
					/* start a 30 ms timer */
					Para.Para32[0] = 0;
					SkTimerStart(pAC, IoC, &pPrt->PPauseTimer,
						SK_PAUSE_PKT_TIME, SKGE_HWAC, SK_HWEV_PAUSE_PACKET,
						Para);

					SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
						("start timer\n"));
				}
			}
			else if (pPrt->PPauseTimer.TmActive) {
				/* stop the 30 ms timer */
				SkTimerStop(pAC, IoC, &pPrt->PPauseTimer);

				SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
					("stop timer\n"));
			}
		}
#ifndef SK_SLIM
		else {
			SkI2cIsr(pAC, IoC);
		}
#endif
	}

	/* SW forced interrupt */
	if ((Istatus & Y2_IS_IRQ_SW) != 0) {
		/* clear the software IRQ */
		SK_OUT8(IoC, B0_CTST, CS_CL_SW_IRQ);
	}

	if ((Istatus & (Y2_IS_L1_MASK | Y2_IS_PHY_QLNK)) != 0) {
		/* IRQ from 1st port */
		SkYuk2PortSirq(pAC, IoC, Istatus, MAC_1);
	}

	if ((Istatus & Y2_IS_L2_MASK & pAC->GIni.GIValIrqMask) != 0) {
		/* IRQ from 2nd port */
		if (pAC->GIni.GIMacsFound == 1) {
			SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E037, SKERR_SIRQ_E037MSG);
			Para.Para64 = 0;
			SkEventQueue(pAC, SKGE_DRV, SK_DRV_ADAP_FAIL, Para);

			return;
		}

		SkYuk2PortSirq(pAC, IoC, Istatus, MAC_2);
	}

	/* Timer interrupt (served last) */
	if ((Istatus & Y2_IS_TIMINT) != 0) {

		if (((Istatus & Y2_IS_HW_ERR) & ~pAC->GIni.GIValIrqMask) != 0) {
			/* read the HW Error Interrupt source */
			SK_IN32(IoC, B0_HWE_ISRC, &RegVal32);

			/* otherwise we would generate error log entries periodically */
			RegVal32 &= pAC->GIni.GIValHwIrqMask;

			if (RegVal32 != 0) {
				SkYuk2HwErr(pAC, IoC, RegVal32);
			}
		}

		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
			("Timer Int: 0x%08lX\n", Istatus));

		SkHwtIsr(pAC, IoC);
	}
#endif /* YUK2 */

}	/* SkYuk2SirqIsr */


/******************************************************************************
 *
 * SkGePortCheckUp() - Check if the link is up
 *
 * return:
 *	0	o.k. nothing needed
 *	1	Restart needed on this port
 *	2	Link came up
 */
static int SkGePortCheckUp(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port)		/* Port Index (MAC_1 + n) */
{
	SK_GEPORT	*pPrt;		/* GIni Port struct pointer */
	SK_BOOL		AutoNeg;	/* Is Auto-negotiation used ? */
	int			Rtv;		/* Return value */

	pPrt = &pAC->GIni.GP[Port];

	AutoNeg = pPrt->PLinkMode != SK_LMODE_HALF &&
			  pPrt->PLinkMode != SK_LMODE_FULL;

	Rtv = SkGePortCheckUpGmac(pAC, IoC, Port, AutoNeg);

	return(Rtv);
}	/* SkGePortCheckUp */


/******************************************************************************
 *
 * SkGePortCheckUpGmac() - Check if the link is up on Marvell PHY
 *
 * return:
 *	0	o.k. nothing needed
 *	1	Restart needed on this port
 *	2	Link came up
 */
static int SkGePortCheckUpGmac(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Port,		/* Port Index (MAC_1 + n) */
SK_BOOL	AutoNeg)	/* Is Auto-negotiation used ? */
{
	SK_GEPORT	*pPrt;		/* GIni Port struct pointer */
	int			Done;
	int			ChipId;
	SK_U16		PhyStat;	/* PHY Status */
	SK_U16		PhySpecStat;/* PHY Specific Status */
	SK_U16		ResAb;		/* Master/Slave resolution */
	SK_U16		Word;		/* I/O helper */
	SK_U32		Val32;

#ifndef SK_SLIM
	SK_EVPARA	Para;
	int			i;
	SK_U16		DspOffs;	/* DSP Register Offset */
	SK_U16		DspCtrl;	/* DSP Control Value */
	SK_U16		DspIncr;	/* Increment to access MDI pair */
#ifdef DEBUG
	SK_U16		LinkPartAb;	/* Link Partner Ability */
#endif /* DEBUG */
#endif /* !SK_SLIM */

#ifdef xDEBUG
	SK_U64	StartTime;
#endif /* DEBUG */

	pPrt = &pAC->GIni.GP[Port];

	ChipId = pAC->GIni.GIChipId;

	if (pPrt->PHWLinkUp) {
		/* check for PHY status only in newer chips */
		if (ChipId >= CHIP_ID_YUKON_FE_P) {
			/* check for link through GPHY Control */
			SK_IN32(IoC, MR_ADDR(Port, GPHY_CTRL), &Val32);

			if ((Val32 & GPC_PHY_LINK_UP) == 0) {
				/* Link is down */
				return(SK_HW_PS_RESTART);
			}
		}

		/* don't poll PHY register */
		return(SK_HW_PS_NONE);
	}

	/* Read PHY Status */
	SkGmPhyRead(pAC, IoC, Port, PHY_MARV_STAT, &PhyStat);

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("CheckUp Port %d, PhyStat: 0x%04X\n", Port, PhyStat));

	SkMacAutoNegLipaPhy(pAC, IoC, Port, PhyStat);

	if ((pPrt->PLinkSpeedCap & SK_LSPEED_CAP_1000MBPS) != 0) {

		SkGmPhyRead(pAC, IoC, Port, PHY_MARV_1000T_STAT, &ResAb);

		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
			("Phy1000BT: 0x%04X, ", ResAb));

		if ((ResAb & PHY_B_1000S_MSF) != 0) {

			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_ERR,
				("Master/Slave Fault, ResAb: 0x%04X\n", ResAb));

			pPrt->PAutoNegFail = SK_TRUE;
			pPrt->PMSStatus = SK_MS_STAT_FAULT;

			return(SK_HW_PS_RESTART);
		}
	}

	/* Read PHY Specific Status */
	SkGmPhyRead(pAC, IoC, Port, PHY_MARV_PHY_STAT, &PhySpecStat);

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("PhySpecStat: 0x%04X\n", PhySpecStat));

#if defined(DEBUG) && !defined(SK_SLIM)
	/* Read PHY Auto-Negotiation Expansion */
	SkGmPhyRead(pAC, IoC, Port, PHY_MARV_AUNE_EXP, &LinkPartAb);

	if ((LinkPartAb & PHY_ANE_RX_PG) != 0 ||
		(PhySpecStat & PHY_M_PS_PAGE_REC) != 0) {
		/* Read PHY Next Page Link Partner */
		SkGmPhyRead(pAC, IoC, Port, PHY_MARV_NEPG_LP, &Word);

		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
			("Page received, NextPage: 0x%04X\n", Word));
	}
#endif /* DEBUG && !SK_SLIM */

	if ((PhySpecStat & PHY_M_PS_LINK_UP) == 0) {
		/* Link down */
		return(SK_HW_PS_NONE);
	}

	if (pAC->GIni.GICopperType) {

#if defined(DEBUG) && !defined(SK_SLIM)
		if ((LinkPartAb & PHY_ANE_LP_CAP) == 0) {

			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("Link Partner is not AN able, AN Exp.: 0x%04X\n", Word));
		}
		else if ((pPrt->PLinkSpeedCap & SK_LSPEED_CAP_1000MBPS) != 0 &&
				 (ResAb & PHY_B_1000S_LP_FD) == 0) {

			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("Link Partner is not capable of 1000FD\n"));
		}

		if ((LinkPartAb & PHY_ANE_PAR_DF) != 0) {

			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("Parallel Detection Fault, AN Exp.: 0x%04X\n", Word));
		}
#endif /* DEBUG && !SK_SLIM */

		/* check only Gigabit adapters */
		if ((pPrt->PLinkSpeedCap & SK_LSPEED_CAP_1000MBPS) != 0) {
			/* check for 2 pair downshift */
			if ((PhySpecStat & PHY_M_PS_DOWNS_STAT) != 0) {
#ifndef SK_SLIM
				/* Downshift detected */
				Para.Para64 = Port;
				SkEventQueue(pAC, SKGE_DRV, SK_DRV_DOWNSHIFT_DET, Para);

				SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
					("Downshift detected, PhySpecStat: 0x%04X\n", PhySpecStat));

				SK_ERR_LOG(pAC, SK_ERRCL_INFO, SKERR_SIRQ_E025,
					SKERR_SIRQ_E025MSG);
#endif /* !SK_SLIM */

				if (ChipId == CHIP_ID_YUKON_PRM) {
					/* read General Purpose Status */
					GM_IN16(IoC, Port, GM_GP_STAT, &Word);

					if ((Word & GM_GPSR_GIG_SPEED) != 0) {
						SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
							("MAC speed mismatch, GPSR=0x%04X\n", Word));

						/* read General Purpose Control */
						GM_IN16(IoC, Port, GM_GP_CTRL, &Word);

						Word &= ~GM_GPCR_GIGS_ENA;
						Word |= GM_GPCR_SPEED_100 | GM_GPCR_AU_SPD_DIS;

						/* change General Purpose Control */
						GM_OUT16(IoC, Port, GM_GP_CTRL, Word);
					}
				}
			}

			pPrt->PMSStatus = ((ResAb & PHY_B_1000S_MSR) != 0) ?
				SK_MS_STAT_MASTER : SK_MS_STAT_SLAVE;
		}

#ifndef SK_SLIM
		if ((PhySpecStat & PHY_M_PS_MDI_X_STAT) != 0) {
			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("MDI Xover detected, PhyStat: 0x%04X\n", PhySpecStat));
		}

		if (HW_HAS_NEWER_PHY(pAC)) {
			/* on PHY 88E1112 cable length is in Reg. 26, Page 5 */
			if (ChipId == CHIP_ID_YUKON_XL) {
#ifdef XXX
				/* select page 5 to access VCT DSP distance register */
				SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, 5);

				/* get VCT DSP distance */
				SkGmPhyRead(pAC, IoC, Port, PHY_MARV_EXT_CTRL_2, &PhySpecStat);

				SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, 0);

				pPrt->PCableLen = (SK_U8)(PhySpecStat & PHY_M_EC2_FO_AM_MSK);
#endif /* XXX */
				DspOffs = 29;
				DspCtrl = 0x8754;
				DspIncr = 0x1000;
			}
			else {
				if (ChipId <= CHIP_ID_YUKON_UL_2) {
					/* PHY 88E1149R */
					DspOffs = 19;
					DspCtrl = 0x1018;
				}
				else {
					/* PHY 88E1240/88E1340 */
					DspOffs = 16;
					DspCtrl = 0x1118;
				}
				DspIncr = 0x0001;

				/* select page 0xff to access VCT DSP distance register */
				SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, 0x00ff);
			}

			for (Word = 0, i = 0; i < 4; i++) {
				/* select register for MDI pair 0..3 */
				SkGmPhyWrite(pAC, IoC, Port, DspOffs, DspCtrl + i * DspIncr);

				/* get length for MDI pair 0..3 */
				SkGmPhyRead(pAC, IoC, Port, DspOffs + 2, &PhySpecStat);

				Word += PhySpecStat;
			}

			pPrt->PCableLen = Word / 4;	/* calculate the average */

			/* set page register back to 0 */
			SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_EXT_ADR, 0);
		}
		else {	/* CHIP_ID_YUKON || CHIP_ID_YUKON_LITE || CHIP_ID_YUKON_EC */
			/* on these PHYs the rough estimated cable length is in Reg. 17 */
			pPrt->PCableLen = (SK_U8)((PhySpecStat & PHY_M_PS_CABLE_MSK) >> 7);
		}
#endif /* !SK_SLIM */
	}

#ifdef xDEBUG
	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("LinkUp Port %d, PHY register dump: 0x%04X\n", Port));

	for (i = 0; i < PHY_MARV_PAGE_ADDR; i++) {
		/* dump all PHY register */
		SkGmPhyRead(pAC, IoC, Port, i, &Word);
	}

	StartTime = SkOsGetTime(pAC);

	while (SkOsGetTime(pAC) - StartTime < SK_TICKS_PER_SEC / 2) {
		/* dummy PHY read */
		SK_IN16(IoC, PCI_C2(pAC, IoC, PCI_STATUS), &Word);
	}
#endif /* DEBUG */

	if (AutoNeg) {
		/* Auto-Negotiation Complete ? */
		if ((PhyStat & PHY_ST_AN_OVER) != 0) {

			SkHWLinkUp(pAC, IoC, Port);

			Done = SkMacAutoNegDone(pAC, IoC, Port);

			return((Done != SK_AND_OK) ? SK_HW_PS_RESTART : SK_HW_PS_LINK);
		}

		return(SK_HW_PS_NONE);
	}

	/* No Auto-Negotiation */
#if defined(DEBUG) && !defined(SK_SLIM)
	if (pPrt->PLipaAutoNeg == (SK_U8)SK_LIPA_AUTO) {
		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_ERR,
			("ERROR: LiPa auto detected on port %d\n", Port));
	}
#endif /* DEBUG && !SK_SLIM */

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
		("Link sync, Port %d\n", Port));

	SkHWLinkUp(pAC, IoC, Port);

	return(SK_HW_PS_LINK);
}	/* SkGePortCheckUpGmac */


/******************************************************************************
 *
 *	SkGeSirqEvent() - Event Service Routine
 *
 * Description:
 *
 * Notes:
 */
int SkGeSirqEvent(
SK_AC		*pAC,		/* Adapter Context */
SK_IOC		IoC,		/* I/O Context */
SK_U32		Event,		/* Module specific Event */
SK_EVPARA	Para)		/* Event specific Parameter */
{
	SK_GEPORT	*pPrt;		/* GIni Port struct pointer */
	int			Port;
	SK_U16		Word;
	SK_U32		Val32;
	int			PortStat;
	SK_BOOL		LinkUpFlag;
#ifndef SK_SLIM
	SK_U8		Val8;
#endif

	Port = (int)Para.Para32[0];
	pPrt = &pAC->GIni.GP[Port];

	LinkUpFlag = SK_FALSE;

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_IRQ,
		("SIRQ HW Event: %u\n", Event));

	switch (Event) {
	case SK_HWEV_WATIM:
		if (pPrt->PState == SK_PRT_RESET) {

			PortStat = SK_HW_PS_NONE;
		}
		else {
			/* Check whether port came up */
			PortStat = SkGePortCheckUp(pAC, IoC, Port);
		}

		switch (PortStat) {
		case SK_HW_PS_RESTART:
			if (pPrt->PHWLinkUp) {
				/* Set Link to down */
				SkHWLinkDown(pAC, IoC, Port);

				/*
				 * Signal directly to RLMT to ensure correct
				 * sequence of SWITCH and RESET event.
				 */
				SkRlmtEvent(pAC, IoC, SK_RLMT_LINK_DOWN, Para);
			}

			/* Restart needed */
			SkEventQueue(pAC, SKGE_DRV, SK_DRV_PORT_RESET, Para);
			break;

		case SK_HW_PS_LINK:
			/* Signal to RLMT */
			SkEventQueue(pAC, SKGE_RLMT, SK_RLMT_LINK_UP, Para);
			break;
		}

		/* Start again the check timer */
		if (pPrt->PHWLinkUp) {

			Val32 = SK_WA_ACT_TIME;

			if (pAC->GIni.GIChipId == CHIP_ID_YUKON_PRM) {
				/* reduce check timer */
				Val32 /= 2;
			}
		}
		else {
			Val32 = SK_WA_INA_TIME * 5;
		}
		/* Start workaround Errata #2 timer */
		SkTimerStart(pAC, IoC, &pPrt->PWaTimer, Val32,
			SKGE_HWAC, SK_HWEV_WATIM, Para);

		break;

	case SK_HWEV_PORT_START:

#ifndef SK_SLIM
		if (pAC->GIni.GIDontInitPhy) {

#ifdef XXX
			PortStat = SkGePortCheckUp(pAC, IoC, Port);

			if (PortStat == SK_HW_PS_LINK) {
				/* Link is already up */
				LinkUpFlag = SK_TRUE;
			}
#endif /* XXX */

			/* check for link through GPHY Control */
			SK_IN32(IoC, MR_ADDR(Port, GPHY_CTRL), &Val32);

			if ((Val32 & GPC_PHY_LINK_UP) != 0) {
				/* Link is already up */
				LinkUpFlag = SK_TRUE;
			}
			else {
				/* reset this flag to allow PHY init */
				pAC->GIni.GIDontInitPhy = SK_FALSE;
			}

			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("SK_HWEV_PORT_START: Link is %s\n",
				 LinkUpFlag ? "Up" : "Down"));
		}
#endif /* !SK_SLIM */

		if (!LinkUpFlag) {
#ifndef SK_SLIM
			if (pPrt->PHWLinkUp) {
				/*
				 * Signal directly to RLMT to ensure correct
				 * sequence of SWITCH and RESET event.
				 */
				SkRlmtEvent(pAC, IoC, SK_RLMT_LINK_DOWN, Para);
			}
#endif /* !SK_SLIM */

			SkHWLinkDown(pAC, IoC, Port);
		}

		/* Schedule Port RESET */
		SkEventQueue(pAC, SKGE_DRV, SK_DRV_PORT_RESET, Para);

		/* Start workaround Errata #2 timer */
		SkTimerStart(pAC, IoC, &pPrt->PWaTimer, SK_WA_INA_TIME,
			SKGE_HWAC, SK_HWEV_WATIM, Para);

		break;

	case SK_HWEV_PORT_STOP:
#ifndef SK_SLIM
		if (pPrt->PHWLinkUp) {
			/*
			 * Signal directly to RLMT to ensure correct
			 * sequence of SWITCH and RESET event.
			 */
			SkRlmtEvent(pAC, IoC, SK_RLMT_LINK_DOWN, Para);
		}
#endif /* !SK_SLIM */

		/* Stop Workaround Timer */
		SkTimerStop(pAC, IoC, &pPrt->PWaTimer);

		SkHWLinkDown(pAC, IoC, Port);
		break;

#ifndef SK_SLIM
	case SK_HWEV_UPDATE_STAT:
		/* We do NOT need to update any statistics */
		break;

	case SK_HWEV_CLEAR_STAT:
		/* We do NOT need to clear any statistics */
		break;

	case SK_HWEV_SET_LMODE:
		Val8 = (SK_U8)Para.Para32[1];

		if (pPrt->PLinkModeConf != Val8) {
			/* Set New link mode */
			pPrt->PLinkModeConf = Val8;
			/* Restart Port */
			SkEventQueue(pAC, SKGE_HWAC, SK_HWEV_PORT_STOP, Para);
			SkEventQueue(pAC, SKGE_HWAC, SK_HWEV_PORT_START, Para);
		}
		break;

	case SK_HWEV_SET_FLOWMODE:
		Val8 = (SK_U8)Para.Para32[1];

		if (pPrt->PFlowCtrlMode != Val8) {
			/* Set New Flow Control mode */
			pPrt->PFlowCtrlMode = Val8;
			/* Restart Port */
			SkEventQueue(pAC, SKGE_HWAC, SK_HWEV_PORT_STOP, Para);
			SkEventQueue(pAC, SKGE_HWAC, SK_HWEV_PORT_START, Para);
		}
		break;

	case SK_HWEV_SET_ROLE:
		/* not possible for fiber */
		if (!pAC->GIni.GICopperType) {
			break;
		}

		Val8 = (SK_U8)Para.Para32[1];

		if (pPrt->PMSMode != Val8) {
			/* Set New Role (Master/Slave) mode */
			pPrt->PMSMode = Val8;
			/* Restart Port */
			SkEventQueue(pAC, SKGE_HWAC, SK_HWEV_PORT_STOP, Para);
			SkEventQueue(pAC, SKGE_HWAC, SK_HWEV_PORT_START, Para);
		}
		break;

	case SK_HWEV_SET_SPEED:
		if (pPrt->PhyType != SK_PHY_MARV_COPPER) {
			break;
		}

		Val8 = (SK_U8)Para.Para32[1];

		if (pPrt->PLinkSpeed != Val8) {
			/* Set New Speed parameter */
			pPrt->PLinkSpeed = Val8;
			/* Restart Port */
			SkEventQueue(pAC, SKGE_HWAC, SK_HWEV_PORT_STOP, Para);
			SkEventQueue(pAC, SKGE_HWAC, SK_HWEV_PORT_START, Para);
		}
		break;
#endif /* !SK_SLIM */

	case SK_HWEV_PAUSE_PACKET:
		/* get Rx GMAC FIFO Write Level */
		SK_IN16(IoC, RX_GMF_WLEV, &Word);

		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
			("Rx GMAC FIFO Level: 0x%03X\n", Word));

		if (Word > (SK_U16)SK_ECU_LLPP) {
			/* to signal that a Pause ON Packet was sent */
			pPrt->PPauseFlag = SK_FALSE;

			/* send one Pause Packet */
			GM_IN16(IoC, Port, GM_GP_CTRL, &Word);

			/* this will generate also a Pause Packet IRQ */
			GM_OUT16(IoC, Port, GM_GP_CTRL, Word | BIT_15S);

			/* clear bit 15 to be able to re-send one Pause Packet */
			GM_OUT16(IoC, Port, GM_GP_CTRL, Word);

			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("Send a Pause Packet\n"));
		}

		break;

	default:
		SK_ERR_LOG(pAC, SK_ERRCL_SW, SKERR_SIRQ_E001, SKERR_SIRQ_E001MSG);
		break;
	}

	return(0);
}	/* SkGeSirqEvent */


/******************************************************************************
 *
 *	SkPhyIsrGmac() - PHY interrupt service routine
 *
 * Description: handles all interrupts from Marvell PHY
 *
 * Returns: N/A
 */
static void SkPhyIsrGmac(
SK_AC		*pAC,		/* Adapter Context */
SK_IOC		IoC,		/* I/O Context */
int			Port,		/* Port Index (MAC_1 + n) */
SK_U16		IStatus)	/* Interrupt Status */
{
	SK_GEPORT	*pPrt;	/* GIni Port struct pointer */
	SK_EVPARA	Para;
#ifdef xDEBUG
	int		i;
	SK_U16		Word;
	SK_U32		DWord;
#endif /* DEBUG */

	pPrt = &pAC->GIni.GP[Port];

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
		("Port %d PHY IRQ, PhyIsrc: 0x%04X\n", Port, IStatus));

	if ((IStatus & PHY_M_IS_LST_CHANGE) != 0) {

		Para.Para32[0] = (SK_U32)Port;

		if (pPrt->PHWLinkUp) {

#ifdef xDEBUG
			/* read the GMAC Interrupt source register */
			SK_IN16(IoC, MR_ADDR(Port, GMAC_IRQ_SRC), &Word);

			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("GmacIrqSrc: 0x%02X\n", Word));

			SkGmPhyRead(pAC, IoC, Port, PHY_MARV_PHY_STAT, &Word);

			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("Link Status changed, PhySpecStat: 0x%04X, ", Word));

			if (Word != 0 && (IStatus & PHY_M_IS_SYMB_ERROR) != 0) {
				/* read all MIB Counters */
				for (i = 0; i < GM_MIB_CNT_SIZE; i++) {
					GM_IN32(IoC, Port, GM_MIB_CNT_BASE + 8*i, &DWord);

					SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
						("MIB Addr 0x%04X: 0x%08lX\n",
						BASE_GMAC_1 + GM_MIB_CNT_BASE + 8*i, DWord));
				}
			}
#endif /* DEBUG */

			SkHWLinkDown(pAC, IoC, Port);

#ifdef XXX
			SkGmPhyRead(pAC, IoC, Port, PHY_MARV_AUNE_ADV, &Word);

			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("AutoNeg.Adv: 0x%04X\n", Word));

			/* Set Auto-negotiation advertisement */
			if (pAC->GIni.GIChipId != CHIP_ID_YUKON_FE &&
				pPrt->PFlowCtrlMode == SK_FLOW_MODE_SYM_OR_REM) {
				/* restore Asymmetric Pause bit */
				SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_AUNE_ADV,
					(SK_U16)(Word | PHY_M_AN_ASP));
			}
#endif /* XXX */

			/* Signal to RLMT */
			SkEventQueue(pAC, SKGE_RLMT, SK_RLMT_LINK_DOWN, Para);
		}
		else {
			if ((IStatus & PHY_M_IS_AN_COMPL) != 0) {
				SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
					("Auto-Negotiation completed\n"));
			}

			if ((IStatus & PHY_M_IS_LSP_CHANGE) != 0) {
				SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
					("Link Speed changed\n"));
			}

			/* trigger SkGePortCheckUp */
			SkEventQueue(pAC, SKGE_HWAC, SK_HWEV_WATIM, Para);
		}
	}

	if ((IStatus & PHY_M_IS_AN_ERROR) != 0) {
		/* the copper PHY makes 1 retry */
		if (pAC->GIni.GICopperType) {
			/* not logged as error, it might be the first attempt */
			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_CTRL,
				("Auto-Negotiation Error\n"));

#ifndef SK_SLIM
			/*
			 * #979 Workaround for magnetics with incorrect inductivity.
			 *
			 * The issue happens with 100Mbps and auto-negotiation only.
			 * The specificity of the issue is that a 100Mbit link cannot be
			 * established although speed and duplex mode can be exchanged during
			 * auto-negotiation process.
			 * The workaround forces a good link. It is only executed if an
			 * auto-negotiation error occurs and next pages have already been
			 * exchanged. If a workaround is executed only a 100Mbps HD mode is
			 * establised.
			 */
			if (pAC->GIni.GIChipId == CHIP_ID_YUKON_EC_U &&
				(IStatus & PHY_M_IS_AN_PR) != 0) {
				/* apply workaround for 100M Link */
				SkGmPhyWrite(pAC, IoC, Port, PHY_MARV_PAGE_DATA, 0x0180);
			}
#endif /* !SK_SLIM */
		}
		else {
			/* Auto-Negotiation Error */
			SK_ERR_LOG(pAC, SK_ERRCL_INFO, SKERR_SIRQ_E023, SKERR_SIRQ_E023MSG);
		}
	}

	if ((IStatus & PHY_M_IS_FIFO_ERROR) != 0) {
		/* FIFO Overflow/Underrun Error */
		SK_ERR_LOG(pAC, SK_ERRCL_HW, SKERR_SIRQ_E024, SKERR_SIRQ_E024MSG);
	}

}	/* SkPhyIsrGmac */

/* End of File */

