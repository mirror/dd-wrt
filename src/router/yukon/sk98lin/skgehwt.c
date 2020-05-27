/******************************************************************************
 *
 * Name:	skgehwt.c
 * Project:	Gigabit Ethernet Adapters, Event Scheduler Module
 * Version:	$Revision: #5 $
 * Date:	$Date: 2010/11/04 $
 * Purpose:	Hardware Timer
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

/******************************************************************************
 *
 * History:
 *
 *	$Log: skgehwt.c,v $
 *	Revision 2.4  2007/07/31 13:22:14  rschmidt
 *	Editorial changes.
 *	
 *	Revision 2.3  2005/12/14 16:11:02  ibrueder
 *	Added copyright header flags.
 *
 *	Revision 2.2  2004/05/28 13:39:04  rschmidt
 *	Corrected define SK_HWT_MAX, all HW-timer events were reduced to 1 sec.
 *	Changed para Time in SkHwtStart() to units of  1 uS (16 uS before).
 *	Adapted for Yukon-2 timer IRQ in SkHwtRead().
 *	Editorial changes.
 *
 *	Revision 2.1  2003/10/27 14:16:09  amock
 *	Promoted to major revision 2.1.
 *
 *	Revision 1.1  2003/10/27 14:14:18  amock
 *	Copied to yukon2 module branch.
 *
 *	Revision 1.15  2003/09/16 13:41:23  rschmidt
 *	Added (C) Marvell to SysKonnectFileId
 *	Editorial changes.
 *
 *	Revision 1.14  2003/05/13 18:01:58  mkarl
 *	Editorial changes.
 *
 *	Revision 1.13  1999/11/22 13:31:12  cgoos
 *	Changed license header to GPL.
 *
 *	Revision 1.12  1998/10/15 15:11:34  gklug
 *	fix: ID_sccs to SysKonnectFileId
 *
 *	Revision 1.11  1998/10/08 15:27:51  gklug
 *	chg: correction factor is host clock dependent
 *
 *	Revision 1.10  1998/09/15 14:18:31  cgoos
 *	Changed more BOOLEANs to SK_xxx
 *
 *	Revision 1.9  1998/09/15 14:16:06  cgoos
 *	Changed line 107: FALSE to SK_FALSE
 *
 *	Revision 1.8  1998/08/24 13:04:44  gklug
 *	fix: typo
 *
 *	Revision 1.7  1998/08/19 09:50:49  gklug
 *	fix: remove struct keyword from c-code (see CCC) add typedefs
 *
 *	Revision 1.6  1998/08/17 09:59:02  gklug
 *	fix: typos
 *
 *	Revision 1.5  1998/08/14 07:09:10  gklug
 *	fix: chg pAc -> pAC
 *
 *	Revision 1.4  1998/08/10 14:14:52  gklug
 *	rmv: unnecessary SK_ADDR macro
 *
 *	Revision 1.3  1998/08/07 12:53:44  gklug
 *	fix: first compiled version
 *
 *	Revision 1.2  1998/08/07 09:19:29  gklug
 *	adapt functions to the C coding conventions
 *	rmv unnecessary functions.
 *
 *	Revision 1.1  1998/08/05 11:28:36  gklug
 *	first version: adapted from SMT/FDDI
 *
 ******************************************************************************/

/*
 *	Event queue and dispatcher
 */
#if (defined(DEBUG) || ((!defined(LINT)) && (!defined(SK_SLIM))))
static const char SysKonnectFileId[] =
	"@(#) $Id: //Release/Yukon_1G/Shared/schedule/V3/skgehwt.c#5 $ (C) Marvell.";
#endif

#include "h/skdrv1st.h"		/* Driver Specific Definitions */
#include "h/skdrv2nd.h"		/* Adapter Control- and Driver specific Def. */

#ifdef __C2MAN__
/*
 *   Hardware Timer function queue management.
 */
intro()
{}
#endif

/*
 * Prototypes of local functions.
 */
#define SK_HWT_MAX	65000UL * 160		/* ca. 10 sec. */

/* correction factor (62.5/100 = 5/8) */
#define SK_HWT_FAC	(5 * (SK_U32)pAC->GIni.GIHstClkFact / 8)

/*
 * Initialize hardware timer.
 *
 * Must be called during init level 1.
 */
void SkHwtInit(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC)	/* I/O Context */
{
	pAC->Hwt.TStart = 0 ;
	pAC->Hwt.TStop = 0;
	pAC->Hwt.TActive = SK_FALSE;

	SkHwtStop(pAC, IoC);
}

/*
 *
 * Start hardware timer (clock ticks are HW-dependent).
 *
 */
void SkHwtStart(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC,	/* I/O Context */
SK_U32	Time)	/* Time in usec to load the timer */
{
	if (Time > SK_HWT_MAX) {
		Time = SK_HWT_MAX;
	}

	pAC->Hwt.TStart = Time;
	pAC->Hwt.TStop = 0L;

	if (!Time) {
		Time = 1L;
	}

	SK_OUT32(IoC, B2_TI_INI, Time * SK_HWT_FAC);

	SK_OUT16(IoC, B2_TI_CTRL, TIM_START);	/* Start timer */

	pAC->Hwt.TActive = SK_TRUE;
}

/*
 * Stop hardware timer.
 * and clear the timer IRQ
 */
void SkHwtStop(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC)	/* I/O Context */
{
	SK_OUT16(IoC, B2_TI_CTRL, TIM_STOP);

	SK_OUT16(IoC, B2_TI_CTRL, TIM_CLR_IRQ);

	pAC->Hwt.TActive = SK_FALSE;
}

/*
 *	Stop hardware timer and read time elapsed since last start.
 *
 * returns
 *	The elapsed time since last start in units of 1 us.
 *
 */
SK_U32 SkHwtRead(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC)	/* I/O Context */
{
	SK_U32	TRead;
	SK_U32	IStatus;
	SK_U32	TimerInt;

	TimerInt = CHIP_ID_YUKON_2(pAC) ? Y2_IS_TIMINT : IS_TIMINT;

	if (pAC->Hwt.TActive) {

		SkHwtStop(pAC, IoC);

		SK_IN32(IoC, B2_TI_VAL, &TRead);

		TRead /= SK_HWT_FAC;

		SK_IN32(IoC, B0_ISRC, &IStatus);

		/* Check if timer expired (or wrapped around) */
		if ((TRead > pAC->Hwt.TStart) || ((IStatus & TimerInt) != 0)) {

			SkHwtStop(pAC, IoC);

			pAC->Hwt.TStop = pAC->Hwt.TStart;
		}
		else {

			pAC->Hwt.TStop = pAC->Hwt.TStart - TRead;
		}
	}
	return(pAC->Hwt.TStop);
}

/*
 * interrupt source= timer
 */
void SkHwtIsr(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC)	/* I/O Context */
{
	SkHwtStop(pAC, IoC);

	pAC->Hwt.TStop = pAC->Hwt.TStart;

	SkTimerDone(pAC, IoC);
}

/* End of file */
