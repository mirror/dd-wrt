/******************************************************************************
 *
 * Name:	sktimer.c
 * Project:	Gigabit Ethernet Adapters, Event Scheduler Module
 * Version:	$Revision: #5 $
 * Date:	$Date: 2010/11/04 $
 * Purpose:	High level timer functions.
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
 *	$Log: sktimer.c,v $
 *	Revision 2.4  2007/07/31 13:18:42  rschmidt
 *	Used SK_BOOL for Restart flag
 *	Editorial changes
 *	
 *	Revision 2.3  2005/12/14 16:11:02  ibrueder
 *	Added copyright header flags
 *
 *	Revision 2.2  2004/05/28 13:44:39  rschmidt
 *	Changed para Time in SkHwtStart() to units of 1 uS (16 uS before).
 *	Editorial changes.
 *
 *	Revision 2.1  2003/10/27 14:16:09  amock
 *	Promoted to major revision 2.1
 *
 *	Revision 1.1  2003/10/27 14:14:18  amock
 *	Copied to yukon2 module branch
 *
 *	Revision 1.14  2003/09/16 13:46:51  rschmidt
 *	Added (C) Marvell to SysKonnectFileId
 *	Editorial changes
 *
 *	Revision 1.13  2003/05/13 18:01:01  mkarl
 *	Editorial changes.
 *
 *	Revision 1.12  1999/11/22 13:38:51  cgoos
 *	Changed license header to GPL.
 *
 *	Revision 1.11  1998/12/17 13:24:13  gklug
 *	fix: restart problem: do NOT destroy timer queue if init 1 is done
 *
 *	Revision 1.10  1998/10/15 15:11:36  gklug
 *	fix: ID_sccs to SysKonnectFileId
 *
 *	Revision 1.9  1998/09/15 15:15:04  cgoos
 *	Changed TRUE/FALSE to SK_TRUE/SK_FALSE
 *
 *	Revision 1.8  1998/09/08 08:47:55  gklug
 *	add: init level handling
 *
 *	Revision 1.7  1998/08/19 09:50:53  gklug
 *	fix: remove struct keyword from c-code (see CCC) add typedefs
 *
 *	Revision 1.6  1998/08/17 13:43:13  gklug
 *	chg: Parameter will be union of 64bit para, 2 times SK_U32 or SK_PTR
 *
 *	Revision 1.5  1998/08/14 07:09:14  gklug
 *	fix: chg pAc -> pAC
 *
 *	Revision 1.4  1998/08/07 12:53:46  gklug
 *	fix: first compiled version
 *
 *	Revision 1.3  1998/08/07 09:31:53  gklug
 *	fix: delta spelling
 *
 *	Revision 1.2  1998/08/07 09:31:02  gklug
 *	adapt functions to new c coding conventions
 *	rmv: "fast" handling
 *	chg: inserting of new timer in queue.
 *	chg: event queue generation when timer runs out
 *
 *	Revision 1.1  1998/08/05 11:27:55  gklug
 *	first version: adapted from SMT
 *
 ******************************************************************************/

/*
 *	Event queue and dispatcher
 */
#if (defined(DEBUG) || ((!defined(LINT)) && (!defined(SK_SLIM))))
static const char SysKonnectFileId[] =
	"@(#) $Id: //Release/Yukon_1G/Shared/schedule/V3/sktimer.c#5 $ (C) Marvell.";
#endif

#include "h/skdrv1st.h"		/* Driver Specific Definitions */
#include "h/skdrv2nd.h"		/* Adapter Control- and Driver specific Def. */

#ifdef __C2MAN__
/*
	Event queue management.

	General Description:

 */
intro()
{}
#endif


/* Forward declaration */
static void timer_done(SK_AC *pAC, SK_IOC IoC, SK_BOOL Restart);


/*
 * Inits the software timer
 *
 * needs to be called during Init level 1.
 */
void SkTimerInit(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
int		Level)		/* Init Level */
{
	switch (Level) {
	case SK_INIT_DATA:
		pAC->Tim.StQueue = 0;
		break;
	case SK_INIT_IO:
		SkHwtInit(pAC, IoC);
		SkTimerDone(pAC, IoC);
		break;
	default:
		break;
	}
}

/*
 * Stops a high level timer
 * - If a timer is not in the queue the function returns normally, too.
 */
void SkTimerStop(
SK_AC		*pAC,		/* Adapter Context */
SK_IOC		IoC,		/* I/O Context */
SK_TIMER	*pTimer)	/* Timer Pointer to be stopped */
{
	SK_TIMER	**ppTimPrev;
	SK_TIMER	*pTm;

	/* remove timer from queue */
	pTimer->TmActive = SK_FALSE;

	if (pAC->Tim.StQueue == pTimer && !pTimer->TmNext) {

		SkHwtStop(pAC, IoC);
	}

	for (ppTimPrev = &pAC->Tim.StQueue; (pTm = *ppTimPrev);
		ppTimPrev = &pTm->TmNext ) {

		if (pTm == pTimer) {
			/*
			 * Timer found in queue
			 * - dequeue it
			 * - correct delta of the next timer
			 */
			*ppTimPrev = pTm->TmNext;

			if (pTm->TmNext) {
				/* correct delta of next timer in queue */
				pTm->TmNext->TmDelta += pTm->TmDelta;
			}
			return;
		}
	}
}

/*
 * Start a high level software timer
 */
void SkTimerStart(
SK_AC		*pAC,		/* Adapter Context */
SK_IOC		IoC,		/* I/O Context */
SK_TIMER	*pTimer,	/* Timer Pointer to be started */
SK_U32		Time,		/* Time Value (in microsec.) */
SK_U32		Class,		/* Event Class for this timer */
SK_U32		Event,		/* Event Value for this timer */
SK_EVPARA	Para)		/* Event Parameter for this timer */
{
	SK_TIMER	**ppTimPrev;
	SK_TIMER	*pTm;
	SK_U32		Delta;

	SkTimerStop(pAC, IoC, pTimer);

	pTimer->TmClass = Class;
	pTimer->TmEvent = Event;
	pTimer->TmPara = Para;
	pTimer->TmActive = SK_TRUE;

	if (!pAC->Tim.StQueue) {
		/* first Timer to be started */
		pAC->Tim.StQueue = pTimer;
		pTimer->TmNext = 0;
		pTimer->TmDelta = Time;

		SkHwtStart(pAC, IoC, Time);

		return;
	}

	/* timer correction */
	timer_done(pAC, IoC, SK_FALSE);

	/* find position in queue */
	Delta = 0;
	for (ppTimPrev = &pAC->Tim.StQueue; (pTm = *ppTimPrev);
		ppTimPrev = &pTm->TmNext ) {

		if (Delta + pTm->TmDelta > Time) {
			/* the timer needs to be inserted here */
			break;
		}
		Delta += pTm->TmDelta;
	}

	/* insert in queue */
	*ppTimPrev = pTimer;
	pTimer->TmNext = pTm;
	pTimer->TmDelta = Time - Delta;

	if (pTm) {
		/* there is a next timer:  correct its Delta value */
		pTm->TmDelta -= pTimer->TmDelta;
	}

	/* restart with first */
	SkHwtStart(pAC, IoC, pAC->Tim.StQueue->TmDelta);
}


void SkTimerDone(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC)		/* I/O Context */
{
	timer_done(pAC, IoC, SK_TRUE);
}


static void timer_done(
SK_AC	*pAC,		/* Adapter Context */
SK_IOC	IoC,		/* I/O Context */
SK_BOOL	Restart)	/* Do we need to restart the Hardware timer ? */
{
	SK_U32		Delta;
	SK_TIMER	*pTm;
	SK_TIMER	*pTComp;	/* Timer completed now */
	SK_TIMER	**ppLast;	/* Next field of Last timer to be deq */
	int		Done = 0;

	Delta = SkHwtRead(pAC, IoC);

	ppLast = &pAC->Tim.StQueue;
	pTm = pAC->Tim.StQueue;

	while (pTm && !Done) {
		if (Delta >= pTm->TmDelta) {
			/* Timer ran out */
			pTm->TmActive = SK_FALSE;
			Delta -= pTm->TmDelta;
			ppLast = &pTm->TmNext;
			pTm = pTm->TmNext;
		}
		else {
			/* We found the first timer that did not run out */
			pTm->TmDelta -= Delta;
			Delta = 0;
			Done = 1;
		}
	}

	*ppLast = 0;
	/*
	 * pTm points to the first Timer that did not run out.
	 * StQueue points to the first Timer that run out.
	 */

	for (pTComp = pAC->Tim.StQueue; pTComp; pTComp = pTComp->TmNext) {

		SkEventQueue(pAC,pTComp->TmClass, pTComp->TmEvent, pTComp->TmPara);
	}

	/* Set head of timer queue to the first timer that did not run out */
	pAC->Tim.StQueue = pTm;

	if (Restart && pAC->Tim.StQueue) {
		/* Restart HW timer */
		SkHwtStart(pAC, IoC, pAC->Tim.StQueue->TmDelta);
	}
}

/* End of file */
