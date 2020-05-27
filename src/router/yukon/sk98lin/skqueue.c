/******************************************************************************
 *
 * Name:	skqueue.c
 * Project:	Gigabit Ethernet Adapters, Event Scheduler Module
 * Version:	$Revision: #5 $
 * Date:	$Date: 2010/11/04 $
 * Purpose:	Management of an event queue.
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
 *	$Log: skqueue.c,v $
 *	Revision 2.10  2010/07/07 15:49:22  malthoff
 *	Added macros to activate log capability.
 *	
 *	Revision 2.9  2010/03/09 11:24:02  rschmidt
 *	Added use of define SK_NO_PNMI for VxWorks.
 *	Editorial changes.
 *	
 *	Revision 2.8  2009/10/27 14:47:52  tschilli
 *	Added define SK_NO_RLMT to allow compilation without RLMT.
 *
 *	Revision 2.7  2009/02/17 11:12:37  rschmidt
 *	Fixed PREfast compiler warnings.
 *	Editorial changes.
 *
 *	Revision 2.6  2007/07/17 15:31:56  rschmidt
 *	Replaced debug output PRINTF with SK_DBG_MSG.
 *	Changed return handling in SkEventQueue().
 *	Editorial changes.
 *
 *	Revision 2.5  2005/12/14 16:11:02  ibrueder
 *	Added copyright header flags.
 *
 *	Revision 2.4  2005/01/24 16:02:57  omater
 *	First working Miniport with LBFO (LACP) support.
 *
 *	Revision 2.3  2004/05/14 13:28:18  malthoff
 *	Reworked the function headers.
 *	Added comments and descriptions.
 *	Added Init Level Check for each function.
 *	Addd Debug Prints.
 *
 *	Revision 2.2  2004/02/26 16:02:58  mkunz
 *	Changes for ASF.
 *
 *	Revision 2.1  2003/10/27 14:16:09  amock
 *	Promoted to major revision 2.1
 *
 *	Revision 1.1  2003/10/27 14:14:18  amock
 *	Copied to yukon2 module branch
 *
 *	Revision 1.20  2003/09/16 13:44:00  rschmidt
 *	Added (C) Marvell to SysKonnectFileId.
 *	Editorial changes.
 *
 *	Revision 1.19  2003/05/13 18:00:07  mkarl
 *	Removed calls to RLMT, TWSI, and PNMI for SLIM driver (SK_SLIM).
 *	Editorial changes.
 *
 *	Revision 1.18  2002/05/07 14:11:11  rwahl
 *	Fixed Watcom Precompiler error.
 *
 *	Revision 1.17  2002/03/25 10:06:41  mkunz
 *	SkIgnoreEvent deleted
 *
 *	Revision 1.16  2002/03/15 10:51:59  mkunz
 *	Added event classes for link aggregation
 *
 *	Revision 1.15  1999/11/22 13:36:29  cgoos
 *	Changed license header to GPL.
 *
 *	Revision 1.14  1998/10/15 15:11:35  gklug
 *	fix: ID_sccs to SysKonnectFileId
 *
 *	Revision 1.13  1998/09/08 08:47:52  gklug
 *	add: Init Level handling
 *
 *	Revision 1.12  1998/09/08 07:43:20  gklug
 *	fix: SIRQ Event function name
 *
 *	Revision 1.11  1998/09/08 05:54:34  gklug
 *	chg: define SK_CSUM is replaced by SK_USE_CSUM
 *
 *	Revision 1.10  1998/09/03 14:14:49  gklug
 *	add: CSUM and HWAC Eventclass and function.
 *
 *	Revision 1.9  1998/08/19 09:50:50  gklug
 *	fix: remove struct keyword from C-code (see CCC) add typedefs
 *
 *	Revision 1.8  1998/08/17 13:43:11  gklug
 *	chg: Parameter will be union of 64bit para, 2 times SK_U32 or SK_PTR
 *
 *	Revision 1.7  1998/08/14 07:09:11  gklug
 *	fix: chg pAc -> pAC
 *
 *	Revision 1.6  1998/08/11 12:13:14  gklug
 *	add: return code feature of Event service routines
 *	add: correct Error log calls
 *
 *	Revision 1.5  1998/08/07 12:53:45  gklug
 *	fix: first compiled version
 *
 *	Revision 1.4  1998/08/07 09:20:48  gklug
 *	adapt functions to C coding conventions.
 *
 *	Revision 1.3  1998/08/05 11:29:32  gklug
 *	rmv: Timer event entry. Timer will queue event directly
 *
 *	Revision 1.2  1998/07/31 11:22:40  gklug
 *	Initial version
 *
 *	Revision 1.1  1998/07/30 15:14:01  gklug
 *	Initial version. Adapted from SMT
 *
 ******************************************************************************/

/*
 *	Event queue and dispatcher
 */
#if (defined(DEBUG) || ((!defined(LINT)) && (!defined(SK_SLIM))))
static const char SysKonnectFileId[] =
	"@(#) $Id: //Release/Yukon_1G/Shared/schedule/V3/skqueue.c#5 $ (C) Marvell.";
#endif

#include "h/skdrv1st.h"		/* Driver Specific Definitions */
#include "h/skqueue.h"		/* Queue Definitions */
#include "h/skdrv2nd.h"		/* Adapter Control- and Driver specific Def. */

/******************************************************************************
 *
 *	SkEventInit() - init event queue management
 *
 * Description:
 *	This function initializes event queue management.
 *	It must be called during Init Level 0.
 *
 * Returns:
 *	nothing
 */
void	SkEventInit(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	Ioc,	/* I/O Context */
int		Level)	/* Init Level */
{
	switch (Level) {
	case SK_INIT_DATA:
		pAC->Event.EvPut = pAC->Event.EvGet = pAC->Event.EvQueue;
		break;
	default:
		break;
	}
}

/******************************************************************************
 *
 *	SkEventQueue() - add event to queue
 *
 * Description:
 *	This function adds an event to the event queue.
 *	At least Init Level 1 is required to queue events,
 *	but will be scheduled at Init Level 2.
 *
 * returns:
 *	nothing
 */
void	SkEventQueue(
SK_AC		*pAC,	/* Adapter Context */
SK_U32		Class,	/* Event Class */
SK_U32		Event,	/* Event to be queued */
SK_EVPARA	Para)	/* Event Parameter */
{

	if (pAC->GIni.GILevel == SK_INIT_DATA) {
		SK_ERR_LOG(pAC, SK_ERRCL_NORES, SKERR_Q_E003, SKERR_Q_E003MSG);
		return;
	}

	SK_EVENT_LOCK_ACQUIRE(pAC);

	if (pAC->Event.EvPut < &pAC->Event.EvQueue[SK_MAX_EVENT]) {
		pAC->Event.EvPut->Class = Class;
		pAC->Event.EvPut->Event = Event;
		pAC->Event.EvPut->Para = Para;

		if (++pAC->Event.EvPut == &pAC->Event.EvQueue[SK_MAX_EVENT]) {
			pAC->Event.EvPut = pAC->Event.EvQueue;
		}
	}

	SK_EVENT_LOCK_RELEASE(pAC);

	if (pAC->Event.EvPut == pAC->Event.EvGet) {
		SK_ERR_LOG(pAC, SK_ERRCL_NORES, SKERR_Q_E001, SKERR_Q_E001MSG);
	}
}

/******************************************************************************
 *
 *	SkEventDispatcher() - Event Dispatcher
 *
 * Description:
 *	The event dispatcher performs the following operations:
 *		while event queue is not empty
 *			- get event from queue
 *			- send event to state machine
 *
 * CAUTION:
 *	The event functions MUST report an error if performing a reinitialization
 *	of the event queue, e.g. performing Init Level 0..2 while in dispatcher call!
 *	ANY OTHER return value delays scheduling the other events in the queue.
 *	In this case the event blocks the queue until the error condition is cleared!
 *
 * Returns:
 *	The return value error reported by individual event function
 */
int	SkEventDispatcher(
SK_AC	*pAC,	/* Adapter Context */
SK_IOC	IoC)	/* I/O Context */
{
	SK_EVENTELEM	*pEv;	/* pointer into queue */
	SK_U32			Class;
	int			Rtv;

	if (pAC->GIni.GILevel != SK_INIT_RUN) {
		SK_ERR_LOG(pAC, SK_ERRCL_NORES, SKERR_Q_E005, SKERR_Q_E005MSG);
	}

	pEv = pAC->Event.EvGet;

	SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_QUEUE,
		("Dispatch get 0x%x, put 0x%x\n", (SK_U32)pEv, (SK_U32)pAC->Event.EvPut));

	if (!SK_EVENT_MUTEX_ACQUIRE(pAC)) {
		/* another thread is already dispatching events */
		return 0;
	}

	Rtv = 0;
	while (pEv != pAC->Event.EvPut) {
		SK_DBG_MSG(pAC, SK_DBGMOD_HWM, SK_DBGCAT_QUEUE,
			("Dispatch Class %d, Event %d\n", pEv->Class, pEv->Event));

		switch (Class = pEv->Class) {
#ifndef SK_USE_LAC_EV
#ifndef SK_SLIM
#ifndef SK_NO_RLMT
		case SKGE_RLMT:		/* RLMT Event */
			Rtv = SkRlmtEvent(pAC, IoC, pEv->Event, pEv->Para);
			break;
#endif /* !SK_NO_RLMT */
		case SKGE_I2C:		/* I2C Event */
			Rtv = SkI2cEvent(pAC, IoC, pEv->Event, pEv->Para);
			break;
#ifndef SK_NO_PNMI
		case SKGE_PNMI:		/* PNMI Event */
			Rtv = SkPnmiEvent(pAC, IoC, pEv->Event, pEv->Para);
			break;
#endif /* !SK_NO_PNMI */
#endif	/* not SK_SLIM */
#endif	/* not SK_USE_LAC_EV */
		case SKGE_DRV:		/* Driver Event */
			Rtv = SkDrvEvent(pAC, IoC, pEv->Event, pEv->Para);
			break;
#ifndef SK_USE_SW_TIMER
		case SKGE_HWAC:
			Rtv = SkGeSirqEvent(pAC, IoC, pEv->Event, pEv->Para);
			break;
#else /* !SK_USE_SW_TIMER */
		case SKGE_SWT :
			Rtv = SkSwtEvent(pAC, IoC, pEv->Event, pEv->Para);
			break;
#endif /* !SK_USE_SW_TIMER */
#if defined(SK_USE_LAC_EV) || defined(SK_LBFO)
		case SKGE_LACP :
			Rtv = SkLacpEvent(pAC, IoC, pEv->Event, pEv->Para);
			break;
		case SKGE_RSF :
			Rtv = SkRsfEvent(pAC, IoC, pEv->Event, pEv->Para);
			break;
		case SKGE_MARKER :
			Rtv = SkMarkerEvent(pAC, IoC, pEv->Event, pEv->Para);
			break;
		case SKGE_FD :
			Rtv = SkFdEvent(pAC, IoC, pEv->Event, pEv->Para);
			break;
#endif /* SK_USE_LAC_EV */
#ifdef SK_ASF
		case SKGE_ASF :
			Rtv = SkAsfEvent(pAC,IoC,pEv->Event,pEv->Para);
			break ;
#endif
#ifdef	SK_USE_CSUM
		case SKGE_CSUM :
			Rtv = SkCsEvent(pAC, IoC, pEv->Event, pEv->Para);
			break;
#endif	/* SK_USE_CSUM */
		default :
			SK_ERR_LOG(pAC, SK_ERRCL_SW, SKERR_Q_E002, SKERR_Q_E002MSG);
			Rtv = 0;
		}

		if (Rtv != 0) {
			/*
			 * Special Case: See CAUTION statement above.
			 * We assume the event queue is reset.
			 */
			if (pAC->Event.EvGet != pAC->Event.EvQueue &&
				pAC->Event.EvGet != pEv) {
				/*
				 * Create an error log entry if the event queue isn't reset.
				 * In this case it may be blocked.
				 */
				SK_ERR_LOG(pAC, SK_ERRCL_SW, SKERR_Q_E004, SKERR_Q_E004MSG);
			}

			break;
		}

		if (++pEv == &pAC->Event.EvQueue[SK_MAX_EVENT]) {

			pEv = pAC->Event.EvQueue;
		}

		/* Renew get: it is used in queue_events to detect overruns */
		pAC->Event.EvGet = pEv;
	}

	SK_EVENT_MUTEX_RELEASE(pAC);
	return(Rtv);
}

/* End of file */
