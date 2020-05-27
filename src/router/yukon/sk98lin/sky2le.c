/*****************************************************************************
 *
 *	Name:		$Id: //Release/Yukon_1G/Shared/common/V6/sky2le.c#7 $
 *	Project:	Gigabit Ethernet Adapters, Common Modules
 *	Version:	$Revision: #7 $, $Change: 4376 $
 *	Date:		$DateTime: 2010/11/10 18:08:48 $
 *	Purpose:	Functions for handling List Element Tables
 *
 *****************************************************************************/

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

/*****************************************************************************
 *
 * Description:
 *
 * This module contains the code necessary for handling List Elements.
 *
 * Supported Gigabit Ethernet Chipsets:
 *	Yukon-2 (PCI, PCI-X, PCI-Express)
 *
 * Include File Hierarchy:
 *
 *****************************************************************************/
#include "h/skdrv1st.h"
#include "h/skdrv2nd.h"

/* defines *******************************************************************/
/* typedefs ******************************************************************/
/* global variables **********************************************************/
/* local variables ***********************************************************/
/* function prototypes *******************************************************/

/*****************************************************************************
 *
 * SkGeY2InitSingleLETable() - Initialize a List Element Table
 *
 * Description:
 *	This function will initialize the selected list element table.
 *	Should be called once during DriverInit. No InitLevel required.
 *
 * Arguments:
 *	pAC			- pointer to the adapter context
 *	pLETab		- pointer to list element table structure
 *	NumLE		- number of list elements in this table
 *	pVMem		- virtual address of memory allocated for this LE table
 *	PMemLowAddr	- physical address of memory to be used for the LE table
 *	PMemHighAddr
 *
 * Returns:
 *	nothing
 */
void SkGeY2InitSingleLETable(
SK_AC	*pAC,			/* pointer to adapter context */
SK_LE_TABLE	*pLETab,	/* pointer to list element table to be initialized */
unsigned int NumLE,		/* number of list elements to be filled in tab */
void	*pVMem,			/* virtual address of memory used for list elements */
SK_U32	PMemLowAddr,	/* physical address of memory used for LE */
SK_U32	PMemHighAddr)
{
	unsigned int i;

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_INIT,
		("==> SkGeY2InitSingleLETable()\n"));

#ifdef DEBUG
	if (NumLE != 2) {	/* not table for polling unit */
		if ((NumLE % MIN_LEN_OF_LE_TAB) != 0 || NumLE > MAX_LEN_OF_LE_TAB) {
			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_ERR,
				("ERROR: Illegal number of list elements %d\n", NumLE));
		}
	}
#endif /* DEBUG */

	/* special case: unused list element table */
	if (NumLE == 0) {
		PMemLowAddr = 0;
		PMemHighAddr = 0;
		pVMem = 0;
	}

	/*
	 * in order to get the best possible performance,
	 * the macros to access list elements use & instead of %
	 * this requires the length of LE tables to be a power of 2
	 */

	/*
	 * this code guarantees that we use the next power of 2 below the
	 * value specified for NumLe - this way some LEs in the table may
	 * not be used but the macros work correctly
	 * this code does not check for bad values below 128 because in such a
	 * case we cannot do anything here
	 */

	if (NumLE != 2 && NumLE != 0) {
		/* no check for polling unit and unused sync Tx */
		i = MIN_LEN_OF_LE_TAB;
		while (NumLE > i) {
			i *= 2;
			if (i > MAX_LEN_OF_LE_TAB) {
				break;
			}
		}
		if (NumLE != i) {
			SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_ERR,
				("ERROR: Illegal number of list elements %d adjusted to %d\n",
				NumLE, (i / 2)));
			NumLE = i / 2;
		}
	}

	/* set addresses */
	pLETab->pPhyLETABLow = PMemLowAddr;
	pLETab->pPhyLETABHigh = PMemHighAddr;
	pLETab->pLETab = pVMem;

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_INIT,
		("contains %d LEs", NumLE));
	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_INIT,
		(" and starts at virt %08p and phys %08lx:%08lx\n",
		pVMem, PMemHighAddr, PMemLowAddr));

	/* initialize indexes */
	pLETab->Done = 0;
	pLETab->Put = 0;
	pLETab->HwPut = 0;
	/* initialize size */
	pLETab->Num = NumLE;

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_INIT,
		("<== SkGeY2InitSingleLETable()\n"));
}	/* SkGeY2InitSingleLETable */

/*****************************************************************************
 *
 * SkGeY2InitPrefetchUnit() - Initialize a Prefetch Unit
 *
 * Description:
 *	Calling this function requires an already configured list element table.
 *	The prefetch unit to be configured is specified in the parameter 'Queue'.
 *	The function is able to initialize the prefetch units of the following
 *	queues: Q_R1, Q_R2, Q_XS1, Q_XS2, Q_XA1, Q_XA2.
 *	The function should be called before SkGeInitPort().
 *
 * Arguments:
 *	pAC - pointer to the adapter context
 *	IoC - I/O context
 *	Queue - I/O offset of queue e.g. Q_XA1
 *	pLETab - pointer to list element table to be initialized
 *
 * Returns: N/A
 */
void SkGeY2InitPrefetchUnit(
SK_AC	*pAC,			/* pointer to adapter context */
SK_IOC	IoC,			/* I/O context */
unsigned int Queue,		/* Queue offset for finding the right registers */
SK_LE_TABLE	*pLETab)	/* pointer to list element table to be initialized */
{
	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_INIT,
		("==> SkGeY2InitPrefetchUnit()\n"));

#ifdef DEBUG
	if (Queue != Q_R1 && Queue != Q_R2 && Queue != Q_XS1 &&
		Queue != Q_XS2 && Queue != Q_XA1 && Queue != Q_XA2) {
		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_ERR,
			("ERROR: Illegal queue identifier %x\n", Queue));
	}
#endif /* DEBUG */

	/* disable the prefetch unit */
	SK_OUT8(IoC, Y2_PREF_Q_ADDR(Queue, PREF_UNIT_CTRL_REG), PREF_UNIT_RST_SET);
	SK_OUT8(IoC, Y2_PREF_Q_ADDR(Queue, PREF_UNIT_CTRL_REG), PREF_UNIT_RST_CLR);

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_INIT,
		("Base address: %08lx:%08lx\n", pLETab->pPhyLETABHigh,
		pLETab->pPhyLETABLow));

	/* Set the list base address  high part*/
	SK_OUT32(IoC, Y2_PREF_Q_ADDR(Queue, PREF_UNIT_ADDR_HI_REG),
		pLETab->pPhyLETABHigh);

	/* Set the list base address low part */
	SK_OUT32(IoC, Y2_PREF_Q_ADDR(Queue, PREF_UNIT_ADDR_LOW_REG),
		pLETab->pPhyLETABLow);

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_INIT,
		("Last index: %d\n", pLETab->Num-1));

	/* Set the list last index */
	SK_OUT16(IoC, Y2_PREF_Q_ADDR(Queue, PREF_UNIT_LAST_IDX_REG),
		(SK_U16)pLETab->Num - 1);

	/* turn on prefetch unit */
	SK_OUT8(IoC, Y2_PREF_Q_ADDR(Queue, PREF_UNIT_CTRL_REG), PREF_UNIT_OP_ON);

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_INIT,
		("<== SkGeY2InitPrefetchUnit()\n"));
}	/* SkGeY2InitPrefetchUnit */


/*****************************************************************************
 *
 * SkGeY2InitStatBmu() - Initialize the Status BMU
 *
 * Description:
 *	Calling this function requires an already configured list element table.
 *	Ensure the status BMU is only initialized once during DriverInit -
 *	InitLevel2 required.
 *
 * Arguments:
 *	pAC - pointer to the adapter context
 *	IoC - I/O context
 *	pLETab - pointer to status LE table to be initialized
 *
 * Returns: N/A
 */
void SkGeY2InitStatBmu(
SK_AC	*pAC,			/* pointer to adapter context */
SK_IOC	IoC,			/* I/O context */
SK_LE_TABLE	*pLETab)	/* pointer to status LE table */
{
#ifdef DEBUG
	SK_U32 Dummy;
#endif

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_INIT,
		("==> SkGeY2InitStatBmu()\n"));

	/* enable the status unit */
	SK_OUT8(IoC, STAT_CTRL, SC_STAT_RST_SET);
	SK_OUT8(IoC, STAT_CTRL, SC_STAT_RST_CLR);

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_INIT,
		("Base address Low: %08lX\n", pLETab->pPhyLETABLow));

	/* Set the LE base address */
	SK_OUT32(IoC, STAT_LIST_ADDR_LO, pLETab->pPhyLETABLow);

#ifdef DEBUG
	SK_IN32(IoC, STAT_LIST_ADDR_LO, &Dummy);
#endif

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_INIT,
		("Base address High: %08lX\n", pLETab->pPhyLETABHigh));

	SK_OUT32(IoC, STAT_LIST_ADDR_HI, pLETab->pPhyLETABHigh);

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_INIT,
		("Last index: %d\n", pLETab->Num - 1));

	/* Set the list last index */
	SK_OUT16(IoC, STAT_LAST_IDX, (SK_U16)pLETab->Num - 1);

#ifdef DEBUG
	SK_IN32(IoC, STAT_LAST_IDX, &Dummy);
#endif

	if (HW_FEATURE(pAC, HWF_WA_DEV_43_418)) {

		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_INIT,
			("Set Tx index threshold\n"));
		/* WA for dev. #4.3 */
		SK_OUT16(IoC, STAT_TX_IDX_TH, ST_TXTH_IDX_MASK);

		/* set Status-FIFO watermark */
		SK_OUT8(IoC, STAT_FIFO_WM, 0x21);		/* WA for dev. #4.18 */

		/* set Status-FIFO ISR watermark */
		SK_OUT8(IoC, STAT_FIFO_ISR_WM, 0x07);	/* WA for dev. #4.18 */

		/* WA for dev. #4.3 and #4.18 */
		/* set Status-FIFO Tx timer init value */
		SK_OUT32(IoC, STAT_TX_TIMER_INI, HW_MS_TO_TICKS(pAC, 10));
	}
	else {
		/*
		 * Further settings may be added if required...
		 * 1) Status-FIFO watermark (STAT_FIFO_WM, STAT_FIFO_ISR_WM)
		 * 2) Status-FIFO timer values (STAT_TX_TIMER_INI,
		 *		STAT_LEV_TIMER_INI and STAT_ISR_TIMER_INI)
		 * but tests shows that the default values give the best results,
		 * therefore the defaults are used.
		 */
#ifndef SK_SLIM
		SK_OUT16(IoC, STAT_TX_IDX_TH, (SK_U16)pAC->GIni.GITxIdxRepThres);
#else
		SK_OUT16(IoC, STAT_TX_IDX_TH, 10);
#endif
		/* set Status-FIFO watermark */
		SK_OUT8(IoC, STAT_FIFO_WM, 0x10);

		/* set Status-FIFO ISR watermark */
		SK_OUT8(IoC, STAT_FIFO_ISR_WM,
			HW_FEATURE(pAC, HWF_WA_DEV_4109) ? 0x10 : 0x04);

		/* set ISR Timer Init Value to 400 (3.2 us on Yukon-EC) */
		SK_OUT32(IoC, STAT_ISR_TIMER_INI, 0x0190);
	}

#ifdef DEBUG
	SK_IN32(IoC, STAT_ISR_TIMER_INI, &Dummy);
#endif
	/* enable the prefetch unit */
	/* operational bit not functional for Yukon-EC, but fixed in Yukon-2 */
	SK_OUT8(IoC, STAT_CTRL, SC_STAT_OP_ON);

	/* start Status-FIFO timer */
	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_INIT,
		("Start Status FIFO timer\n"));

	SK_OUT8(IoC, STAT_TX_TIMER_CTRL, TIM_START);
	SK_OUT8(IoC, STAT_LEV_TIMER_CTRL, TIM_START);
	SK_OUT8(IoC, STAT_ISR_TIMER_CTRL, TIM_START);

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_INIT,
		("<== SkGeY2InitStatBmu()\n"));
}	/* SkGeY2InitStatBmu */

#ifdef USE_POLLING_UNIT
/*****************************************************************************
 *
 * SkGeY2InitPollUnit() - Initialize the Polling Unit
 *
 * Description:
 *	This function will write the data of one polling LE table into the adapter.
 *
 * Arguments:
 *	pAC - pointer to the adapter context
 *	IoC - I/O contex.
 *	pLETab - pointer to polling LE table to be initialized
 *
 * Returns: N/A
 */
void SkGeY2InitPollUnit(
SK_AC	*pAC,			/* pointer to adapter context */
SK_IOC	IoC,			/* I/O context */
SK_LE_TABLE	*pLETab)	/* pointer to polling LE table */
{
	SK_HWLE	*pLE;

#ifdef VCPU
	int	i;

	VCPU_VARS();
#endif /* VCPU */

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_INIT,
		("==> SkGeY2InitPollUnit()\n"));

#ifdef VCPU
	for (i = 0; i < SK_MAX_MACS; i++) {
		GET_PO_LE(pLE, pLETab, i);
		VCPU_START_AND_COPY_LE();
		/* initialize polling LE but leave indexes invalid */
		POLE_SET_OPC(pLE, OP_PUTIDX | HW_OWNER);
		POLE_SET_LINK(pLE, i);
		POLE_SET_RXIDX(pLE, 0);
		POLE_SET_TXAIDX(pLE, 0);
		POLE_SET_TXSIDX(pLE, 0);
		VCPU_WRITE_LE();
		SK_DBG_DUMP_PO_LE(pLE);
	}
#endif /* VCPU */

	/* disable the polling unit */
	SK_OUT8(IoC, POLL_CTRL, PC_POLL_RST_SET);
	SK_OUT8(IoC, POLL_CTRL, PC_POLL_RST_CLR);

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_INIT,
		("Base address Low: %08lX\n", pLETab->pPhyLETABLow));

	/* Set the list base address */
	SK_OUT32(IoC, POLL_LIST_ADDR_LO, pLETab->pPhyLETABLow);

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_INIT,
		("Base address High: %08lX\n", pLETab->pPhyLETABHigh));

	SK_OUT32(IoC, POLL_LIST_ADDR_HI, pLETab->pPhyLETABHigh);

	/* we don't need to write the last index - it is hardwired to 1 */

	/* enable the prefetch unit */
	SK_OUT8(IoC, POLL_CTRL, PC_POLL_OP_ON);

	/*
	 * now we have to start the descriptor poll timer because it triggers
	 * the polling unit
	 */

	/*
	 * still playing with the value (timer runs at 125 MHz)
	 * descriptor poll timer is enabled by GeInit
	 */
	SK_OUT32(IoC, B28_DPT_INI,
		(SK_DPOLL_DEF_Y2 * (SK_U32)pAC->GIni.GIHstClkFact / 100));

	SK_OUT8(IoC, B28_DPT_CTRL, TIM_START);

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_INIT,
		("<== SkGeY2InitPollUnit()\n"));
}	/* SkGeY2InitPollUnit */
#endif /* USE_POLLING_UNIT */


/******************************************************************************
 *
 * SkGeY2SetPutIndex
 *
 * Description:
 *	This function writes the Done index of a list element table.
 *
 * Notes:
 *	Dev. Issue 4.2
 *
 * Returns: N/A
 */
void SkGeY2SetPutIndex(
SK_AC	*pAC,					/* pointer to adapter context */
SK_IOC	IoC,					/* I/O context */
unsigned int StartAddrPrefUnit,	/* start address of the prefetch unit */
SK_LE_TABLE	*pLETab)			/* list element table to work with */
{
	unsigned int Put;
	unsigned int PrefUnitPutIdxReg;
	SK_U16 EndOfListIndex;
	SK_U16 HwGetIndex;
	SK_U16 HwPutIndex;

	PrefUnitPutIdxReg = StartAddrPrefUnit + PREF_UNIT_PUT_IDX_REG;

	/* set put index we would like to write */
	Put = GET_PUT_IDX(pLETab);

	/*
	 * in this case we wrap around
	 * new put is lower than last put given to HW
	 */
	if (Put < pLETab->HwPut) {

		/* set put index = last index of list */
		EndOfListIndex = NUM_LE_IN_TABLE(pLETab) - 1;

		/* read get index of HW prefetch unit */
		SK_IN16(IoC, StartAddrPrefUnit + PREF_UNIT_GET_IDX_REG, &HwGetIndex);

		/* read put index of HW prefetch unit */
		SK_IN16(IoC, PrefUnitPutIdxReg, &HwPutIndex);

		/* prefetch unit reached end of list */
		/* prefetch unit reached first list element */
		if (HwGetIndex == 0) {
			/* restore watermark */
			SK_OUT8(IoC, StartAddrPrefUnit + PREF_UNIT_FIFO_WM_REG, 0xe0);

			/* write put index */
			SK_OUT16(IoC, PrefUnitPutIdxReg, (SK_U16)Put);

			/* remember put index written to HW */
			pLETab->HwPut = Put;
		}
		else if (HwGetIndex == EndOfListIndex) {
			/* set watermark to one list element */
			SK_OUT8(IoC, StartAddrPrefUnit + PREF_UNIT_FIFO_WM_REG, 8);

			/* set put index to first list element */
			SK_OUT16(IoC, PrefUnitPutIdxReg, 0);
		}
		/* prefetch unit did not reach end of list yet */
		/* and we did not write put index to end of list yet */
		else if (HwPutIndex != EndOfListIndex &&
				 HwGetIndex != EndOfListIndex) {
			/* write put index */
			SK_OUT16(IoC, PrefUnitPutIdxReg, EndOfListIndex);
		}
		else {
			/* do nothing */
		}
	}
	else {
#ifdef XXX	/* leads in to problems in the Windows Driver */
		if (Put != pLETab->HwPut) {
			/* write put index */
			SK_OUT16(IoC, PrefUnitPutIdxReg, (SK_U16)Put);
			/* update put index */
			UPDATE_HWPUT_IDX(pLETab);
		}
#else
		/* write put index */
		SK_OUT16(IoC, PrefUnitPutIdxReg, (SK_U16)Put);
		/* update put index */
		UPDATE_HWPUT_IDX(pLETab);
#endif
	}
}	/* SkGeY2SetPutIndex */

