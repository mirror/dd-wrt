/******************************************************************************
 *
 * Name:        skdim.c
 * Project:     GEnesis, PCI Gigabit Ethernet Adapter
 * Purpose:     All functions regardig interrupt moderation
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	    (C)Copyright 1998-2002 SysKonnect GmbH.
 *	    (C)Copyright 2002-2011 Marvell.
 *
 *	    Driver for Marvell Yukon/2 chipset and SysKonnect Gigabit Ethernet
 *      Server Adapters.
 *
 *	    Address all question to: support@marvell.com
 *
 *      LICENSE:
 *      (C)Copyright Marvell.
 *      
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *      
 *      The information in this file is provided "AS IS" without warranty.
 *      /LICENSE
 *
 *****************************************************************************/

#include "h/skdrv1st.h"
#include "h/skdrv2nd.h"

/******************************************************************************
 *
 * Local Function Prototypes
 *
 *****************************************************************************/

static SK_U64 getIsrCalls(SK_AC *pAC);
static SK_BOOL isIntModEnabled(SK_AC *pAC);
static void setCurrIntCtr(SK_AC *pAC);
static void enableIntMod(SK_AC *pAC);
static void disableIntMod(SK_AC *pAC);

#define M_DIMINFO pAC->DynIrqModInfo

/******************************************************************************
 *
 * Global Functions
 *
 *****************************************************************************/

/*****************************************************************************
 *
 * 	SkDimModerate - Moderates the IRQs depending on the current needs
 *
 * Description:
 *	Moderation of IRQs depends on the number of occurred IRQs with
 *	respect to the previous moderation cycle.
 *
 * Returns:	N/A
 *
 */
void SkDimModerate(
SK_AC *pAC)  /* pointer to adapter control context */
{
	SK_U64  IsrCalls = getIsrCalls(pAC);

	SK_DBG_MSG(pAC,SK_DBGMOD_DRV,SK_DBGCAT_DRV_MSG,("==> SkDimModerate\n"));

	if (M_DIMINFO.IntModTypeSelect == C_INT_MOD_DYNAMIC) {
		if (isIntModEnabled(pAC)) {
			if (IsrCalls < M_DIMINFO.MaxModIntsPerSecLowerLimit) {
				disableIntMod(pAC);
			}
		} else {
			if (IsrCalls > M_DIMINFO.MaxModIntsPerSecUpperLimit) {
				enableIntMod(pAC);
			}
		}
	}
	setCurrIntCtr(pAC);

	SK_DBG_MSG(pAC,SK_DBGMOD_DRV,SK_DBGCAT_DRV_MSG,("<== SkDimModerate\n"));
}

/*****************************************************************************
 *
 * 	SkDimStartModerationTimer - Starts the moderation timer
 *
 * Description:
 *	Dynamic interrupt moderation is regularly checked using the
 *	so-called moderation timer. This timer is started with this function.
 *
 * Returns:	N/A
 */
void SkDimStartModerationTimer(
SK_AC *pAC) /* pointer to adapter control context */
{
	SK_EVPARA   EventParam;   /* Event struct for timer event */

	SK_DBG_MSG(pAC,SK_DBGMOD_DRV,SK_DBGCAT_DRV_MSG,
			("==> SkDimStartModerationTimer\n"));

	if (M_DIMINFO.IntModTypeSelect == C_INT_MOD_DYNAMIC) {
		SK_MEMSET((char *) &EventParam, 0, sizeof(EventParam));
		EventParam.Para32[0] = SK_DRV_MODERATION_TIMER;
		SkTimerStart(pAC, pAC->IoBase,
			&pAC->DynIrqModInfo.ModTimer,
			pAC->DynIrqModInfo.DynIrqModSampleInterval * 1000000,
			SKGE_DRV, SK_DRV_TIMER, EventParam);
	}

	SK_DBG_MSG(pAC,SK_DBGMOD_DRV,SK_DBGCAT_DRV_MSG,
			("<== SkDimStartModerationTimer\n"));
}

/*****************************************************************************
 *
 * 	SkDimEnableModerationIfNeeded - Enables or disables any moderationtype
 *
 * Description:
 *	This function effectively initializes the IRQ moderation of a network
 *	adapter. Depending on the configuration, this might be either static
 *	or dynamic. If no moderation is configured, this function will do
 *	nothing.
 *
 * Returns:	N/A
 */
void SkDimEnableModerationIfNeeded(
SK_AC *pAC)  /* pointer to adapter control context */
{
	SK_DBG_MSG(pAC,SK_DBGMOD_DRV,SK_DBGCAT_DRV_MSG,
			("==> SkDimEnableModerationIfNeeded\n"));

	if (M_DIMINFO.IntModTypeSelect != C_INT_MOD_NONE) {
		if (M_DIMINFO.IntModTypeSelect == C_INT_MOD_STATIC) {
			enableIntMod(pAC);
		} else { /* must be C_INT_MOD_DYNAMIC */
			SkDimStartModerationTimer(pAC);
		}
	}

	SK_DBG_MSG(pAC,SK_DBGMOD_DRV,SK_DBGCAT_DRV_MSG,
			("<== SkDimEnableModerationIfNeeded\n"));
}

/*****************************************************************************
 *
 * 	SkDimDisableModeration - disables moderation if it is enabled
 *
 * Description:
 *	Disabling of the moderation requires that is enabled already.
 *
 * Returns:	N/A
 */
void SkDimDisableModeration(
SK_AC  *pAC,                /* pointer to adapter control context */
int     CurrentModeration)  /* type of current moderation         */
{
	SK_DBG_MSG(pAC,SK_DBGMOD_DRV,SK_DBGCAT_DRV_MSG,
			("==> SkDimDisableModeration\n"));

	if (M_DIMINFO.IntModTypeSelect != C_INT_MOD_NONE) {
		if (CurrentModeration == C_INT_MOD_STATIC) {
			disableIntMod(pAC);
		} else { /* must be C_INT_MOD_DYNAMIC */
			SkTimerStop(pAC, pAC->IoBase, &M_DIMINFO.ModTimer);
			disableIntMod(pAC);
		}
	}

	SK_DBG_MSG(pAC,SK_DBGMOD_DRV,SK_DBGCAT_DRV_MSG,
			("<== SkDimDisableModeration\n"));
}

/******************************************************************************
 *
 * Local Functions
 *
 *****************************************************************************/

/*****************************************************************************
 *
 * 	getIsrCalls - evaluate the number of IRQs handled in mod interval
 *
 * Description:
 *	Depending on the selected moderation mask, this function will return
 *	the number of interrupts handled in the previous moderation interval.
 *	This evaluated number is based on the current number of interrupts
 *	stored in PNMI-context and the previous stored interrupts.
 *
 * Returns:
 *	the number of IRQs handled
 */
static SK_U64 getIsrCalls(
SK_AC *pAC)  /* pointer to adapter control context */
{
	SK_U64   RxPort0IntDiff = 0, RxPort1IntDiff = 0;
	SK_U64   TxPort0IntDiff = 0, TxPort1IntDiff = 0;
	SK_U64   StatusPort0IntDiff = 0, StatusPort1IntDiff = 0;

        SK_DBG_MSG(pAC,SK_DBGMOD_DRV,SK_DBGCAT_DRV_MSG,("==>getIsrCalls\n"));

	if (!CHIP_ID_YUKON_2(pAC)) {
		if ((M_DIMINFO.MaskIrqModeration == IRQ_MASK_TX_ONLY) ||
		    (M_DIMINFO.MaskIrqModeration == IRQ_MASK_SP_TX)) {
			if (pAC->GIni.GIMacsFound == 2) {
				TxPort1IntDiff =
					pAC->Pnmi.Port[1].TxIntrCts -
					M_DIMINFO.PrevPort1TxIntrCts;
			}
			TxPort0IntDiff = pAC->Pnmi.Port[0].TxIntrCts -
					M_DIMINFO.PrevPort0TxIntrCts;
		} else if ((M_DIMINFO.MaskIrqModeration == IRQ_MASK_RX_ONLY) ||
		           (M_DIMINFO.MaskIrqModeration == IRQ_MASK_SP_RX)) {
			if (pAC->GIni.GIMacsFound == 2) {
				RxPort1IntDiff =
					pAC->Pnmi.Port[1].RxIntrCts -
					M_DIMINFO.PrevPort1RxIntrCts;
			}
			RxPort0IntDiff = pAC->Pnmi.Port[0].RxIntrCts -
					M_DIMINFO.PrevPort0RxIntrCts;
		} else {
			if (pAC->GIni.GIMacsFound == 2) {
				RxPort1IntDiff =
					pAC->Pnmi.Port[1].RxIntrCts -
					M_DIMINFO.PrevPort1RxIntrCts;
				TxPort1IntDiff =
					pAC->Pnmi.Port[1].TxIntrCts -
					M_DIMINFO.PrevPort1TxIntrCts;
			}
			RxPort0IntDiff = pAC->Pnmi.Port[0].RxIntrCts -
					M_DIMINFO.PrevPort0RxIntrCts;
			TxPort0IntDiff = pAC->Pnmi.Port[0].TxIntrCts -
					M_DIMINFO.PrevPort0TxIntrCts;
		}
        	SK_DBG_MSG(pAC,SK_DBGMOD_DRV,SK_DBGCAT_DRV_MSG,
				("==>getIsrCalls (!CHIP_ID_YUKON_2)\n"));
		return (RxPort0IntDiff + RxPort1IntDiff +
		        TxPort0IntDiff + TxPort1IntDiff);
	}

	/*
	** We have a Yukon2 compliant chipset if we come up to here
	**
	if (pAC->GIni.GIMacsFound == 2) {
		StatusPort1IntDiff = pAC->Pnmi.Port[1].StatusLeIntrCts -
					M_DIMINFO.PrevPort1StatusIntrCts;
	}
	StatusPort0IntDiff = pAC->Pnmi.Port[0].StatusLeIntrCts -
				M_DIMINFO.PrevPort0StatusIntrCts;
	*/
        SK_DBG_MSG(pAC,SK_DBGMOD_DRV,SK_DBGCAT_DRV_MSG,
			("==>getIsrCalls (CHIP_ID_YUKON_2)\n"));
	return (StatusPort0IntDiff + StatusPort1IntDiff);
}

/*****************************************************************************
 *
 * 	setCurrIntCtr - stores the current number of interrupts
 *
 * Description:
 *	Stores the current number of occurred interrupts in the adapter
 *	context. This is needed to evaluate the  umber of interrupts within
 *	the moderation interval.
 *
 * Returns:	N/A
 *
 */
static void setCurrIntCtr(
SK_AC *pAC)  /* pointer to adapter control context */
{
        SK_DBG_MSG(pAC,SK_DBGMOD_DRV,SK_DBGCAT_DRV_MSG,("==>setCurrIntCtr\n"));

	if (!CHIP_ID_YUKON_2(pAC)) {
		if (pAC->GIni.GIMacsFound == 2) {
			M_DIMINFO.PrevPort1RxIntrCts = pAC->Pnmi.Port[1].RxIntrCts;
			M_DIMINFO.PrevPort1TxIntrCts = pAC->Pnmi.Port[1].TxIntrCts;
		}
		M_DIMINFO.PrevPort0RxIntrCts = pAC->Pnmi.Port[0].RxIntrCts;
		M_DIMINFO.PrevPort0TxIntrCts = pAC->Pnmi.Port[0].TxIntrCts;
        	SK_DBG_MSG(pAC,SK_DBGMOD_DRV,SK_DBGCAT_DRV_MSG,
				("<== setCurrIntCtr (!CHIP_ID_YUKON_2)\n"));
		return;
	}

	/*
	** We have a Yukon2 compliant chipset if we come up to here
	**
	if (pAC->GIni.GIMacsFound == 2) {
		M_DIMINFO.PrevPort1StatusIntrCts = pAC->Pnmi.Port[1].StatusLeIntrCts;
	}
	M_DIMINFO.PrevPort0StatusIntrCts = pAC->Pnmi.Port[0].StatusLeIntrCts;
	*/
        SK_DBG_MSG(pAC,SK_DBGMOD_DRV,SK_DBGCAT_DRV_MSG,
			("<== setCurrIntCtr (CHIP_ID_YUKON_2)\n"));
}

/*****************************************************************************
 *
 * 	isIntModEnabled - returns the current state of interrupt moderation
 *
 * Description:
 *	This function retrieves the current value of the interrupt moderation
 *	command register. Its content determines whether any moderation is
 *	running or not.
 *
 * Returns:
 *	SK_TRUE : IRQ moderation is currently active
 *	SK_FALSE: No IRQ moderation is active
 */
static SK_BOOL isIntModEnabled(
SK_AC *pAC)  /* pointer to adapter control context */
{
	unsigned long CtrCmd;

        SK_DBG_MSG(pAC,SK_DBGMOD_DRV,SK_DBGCAT_DRV_MSG,("==>isIntModEnabled\n"));

	SK_IN32(pAC->IoBase, B2_IRQM_CTRL, &CtrCmd);
	if ((CtrCmd & TIM_START) == TIM_START) {
        	SK_DBG_MSG(pAC,SK_DBGMOD_DRV,SK_DBGCAT_DRV_MSG,
			("<== isIntModEnabled (SK_TRUE)\n"));
		return SK_TRUE;
	}
        SK_DBG_MSG(pAC,SK_DBGMOD_DRV,SK_DBGCAT_DRV_MSG,
			("<== isIntModEnabled (SK_FALSE)\n"));
	return SK_FALSE;
}

/*****************************************************************************
 *
 * 	enableIntMod - enables the interrupt moderation
 *
 * Description:
 *	Enabling the interrupt moderation is done by putting the desired
 *	moderation interval in the B2_IRQM_INI register, specifying the
 *	desired maks in the B2_IRQM_MSK register and finally starting the
 *	IRQ moderation timer using the B2_IRQM_CTRL register.
 *
 * Returns:	N/A
 *
 */
static void enableIntMod(
SK_AC *pAC)  /* pointer to adapter control context */
{
	unsigned long ModBase;

        SK_DBG_MSG(pAC,SK_DBGMOD_DRV,SK_DBGCAT_DRV_MSG,("==> enableIntMod\n"));
	ModBase = ((62500L/100) * pAC->GIni.GIHstClkFact * 1000) / M_DIMINFO.MaxModIntsPerSec;

	SK_OUT32(pAC->IoBase, B2_IRQM_INI, ModBase);
	SK_OUT32(pAC->IoBase, B2_IRQM_MSK, M_DIMINFO.MaskIrqModeration);
	SK_OUT32(pAC->IoBase, B2_IRQM_CTRL, TIM_START);

        SK_DBG_MSG(pAC,SK_DBGMOD_DRV,SK_DBGCAT_DRV_MSG,("<== enableIntMod\n"));
}

/*****************************************************************************
 *
 * 	disableIntMod - disables the interrupt moderation
 *
 * Description:
 *	Disabling the interrupt moderation is done by stopping the
 *	IRQ moderation timer using the B2_IRQM_CTRL register.
 *
 * Returns:	N/A
 *
 */
static void disableIntMod(
SK_AC *pAC)  /* pointer to adapter control context */
{
        SK_DBG_MSG(pAC,SK_DBGMOD_DRV,SK_DBGCAT_DRV_MSG,("==> disableIntMod\n"));

	SK_OUT32(pAC->IoBase, B2_IRQM_CTRL, TIM_STOP);

        SK_DBG_MSG(pAC,SK_DBGMOD_DRV,SK_DBGCAT_DRV_MSG,("<== disableIntMod\n"));
}

/*******************************************************************************
 *
 * End of file
 *
 ******************************************************************************/
