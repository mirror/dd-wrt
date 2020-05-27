/******************************************************************************
 *
 * Name:	skqueue.h
 * Project:	Gigabit Ethernet Adapters, Event Scheduler Module
 * Version:	$Revision: #5 $
 * Date:	$Date: 2010/11/04 $
 * Purpose:	Defines for the Event queue
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
 *	$Log: skqueue.h,v $
 *	Revision 2.6  2010/07/07 15:50:36  malthoff
 *	Added macros to lock the scheduler functions.
 *	
 *	Revision 2.5  2007/07/31 13:06:54  rschmidt
 *	Changed defines for error numbers and messages
 *	Editorial changes
 *	
 *	Revision 2.4  2005/12/14 16:11:03  ibrueder
 *	Added copyright header flags
 *
 *	Revision 2.3  2004/05/14 13:39:15  malthoff
 *	Added some debug messages
 *
 *	Revision 2.2  2004/02/26 16:03:04  mkunz
 *	Changes for ASF
 *
 *	Revision 2.1  2003/10/27 14:16:09  amock
 *	Promoted to major revision 2.1
 *
 *	Revision 1.1  2003/10/27 14:14:18  amock
 *	Copied to yukon2 module branch
 *
 *	Revision 1.16  2003/09/16 12:50:32  rschmidt
 *	Editorial changes
 *
 *	Revision 1.15  2003/05/13 17:54:57  mkarl
 *	Editorial changes.
 *
 *	Revision 1.14  2002/03/15 10:52:13  mkunz
 *	Added event classes for link aggregation
 *
 *	Revision 1.13  1999/11/22 13:59:05  cgoos
 *	Changed license header to GPL.
 *
 *	Revision 1.12  1998/09/08 08:48:01  gklug
 *	add: init level handling
 *
 *	Revision 1.11  1998/09/03 14:15:11  gklug
 *	add: CSUM and HWAC Eventclass and function.
 *	fix: pParaPtr according to CCC
 *
 *	Revision 1.10  1998/08/20 12:43:03  gklug
 *	add: typedef SK_QUEUE
 *
 *	Revision 1.9  1998/08/19 09:50:59  gklug
 *	fix: remove struct keyword from C-code (see CCC) add typedefs
 *
 *	Revision 1.8  1998/08/18 07:00:01  gklug
 *	fix: SK_PTR not defined use void * instead.
 *
 *	Revision 1.7  1998/08/17 13:43:19  gklug
 *	chg: Parameter will be union of 64bit para, 2 times SK_U32 or SK_PTR
 *
 *	Revision 1.6  1998/08/14 07:09:30  gklug
 *	fix: chg pAc -> pAC
 *
 *	Revision 1.5  1998/08/11 14:26:44  gklug
 *	chg: Event Dispatcher returns now int.
 *
 *	Revision 1.4  1998/08/11 12:15:21  gklug
 *	add: Error numbers of skqueue module
 *
 *	Revision 1.3  1998/08/07 12:54:23  gklug
 *	fix: first compiled version
 *
 *	Revision 1.2  1998/08/07 09:34:00  gklug
 *	adapt structure defs to CCC
 *	add: prototypes for functions
 *
 *	Revision 1.1  1998/07/30 14:52:12  gklug
 *	Initial version.
 *	Defines Event Classes, Event structs and queue management variables.
 *
 ******************************************************************************/

/*
 * SKQUEUE.H	contains all defines and types for the event queue
 */

#ifndef _SKQUEUE_H_
#define _SKQUEUE_H_

/*
 * define the event classes to be served
 */
#define SKGE_DRV	1	/* Driver Event Class */
#define SKGE_RLMT	2	/* RLMT Event Class */
#define SKGE_I2C	3	/* I2C Event Class */
#define SKGE_PNMI	4	/* PNMI Event Class */
#define SKGE_CSUM	5	/* Checksum Event Class */
#define SKGE_HWAC	6	/* Hardware Access Event Class */

#define SKGE_SWT	9	/* Software Timer Event Class */
#define SKGE_LACP	10	/* LACP Aggregation Event Class */
#define SKGE_RSF	11	/* RSF Aggregation Event Class */
#define SKGE_MARKER	12	/* MARKER Aggregation Event Class */
#define SKGE_FD		13	/* FD Distributor Event Class */
#ifdef SK_ASF
#define SKGE_ASF	14	/* ASF Event Class */
#endif

/*
 * define event queue as circular buffer
 */
#define SK_MAX_EVENT	64

/*
 * Parameter union for the Para stuff
 */
typedef	union u_EvPara {
	void	*pParaPtr;	/* Parameter Pointer */
	SK_U64	Para64;		/* Parameter 64bit version */
	SK_U32	Para32[2];	/* Parameter Array of 32bit parameters */
} SK_EVPARA;

/*
 * Event Queue
 *	skqueue.c
 * events are class/value pairs
 *	class	is addressee, e.g. RLMT, PNMI etc.
 *	value	is command, e.g. line state change, ring op change etc.
 */
typedef	struct s_EventElem {
	SK_U32		Class;			/* Event class */
	SK_U32		Event;			/* Event value */
	SK_EVPARA	Para;			/* Event parameter */
} SK_EVENTELEM;

typedef	struct s_Queue {
	SK_EVENTELEM	EvQueue[SK_MAX_EVENT];
	SK_EVENTELEM	*EvPut;
	SK_EVENTELEM	*EvGet;
} SK_QUEUE;

extern void SkEventInit(SK_AC *, SK_IOC, int Level);
extern void SkEventQueue(SK_AC *, SK_U32 Class, SK_U32 Event, SK_EVPARA Para);
extern int  SkEventDispatcher(SK_AC *, SK_IOC);

/* Define Error Numbers and messages */
#define SKERR_Q_E001	(SK_ERRBASE_QUEUE+1)
#define SKERR_Q_E001MSG	"Event queue overflow"
#define SKERR_Q_E002	(SKERR_Q_E001+1)
#define SKERR_Q_E002MSG	"Undefined event class"
#define SKERR_Q_E003	(SKERR_Q_E002+1)
#define SKERR_Q_E003MSG	"Event queued in init level 0"
#define SKERR_Q_E004	(SKERR_Q_E003+1)
#define SKERR_Q_E004MSG	"Error reported from event function (queue blocked)"
#define SKERR_Q_E005	(SKERR_Q_E004+1)
#define SKERR_Q_E005MSG	"Event scheduler called in init level 0 or 1"

/* define dummies for event mutex and lock */
#if !defined(SK_EVENT_MUTEX_INITIALZE)
#define SK_EVENT_MUTEX_INITIALZE(x)
#endif

#if !defined(SK_EVENT_MUTEX_ACQUIRE)
#define SK_EVENT_MUTEX_ACQUIRE(x) 		SK_TRUE
#endif

#if !defined(SK_EVENT_MUTEX_RELEASE)
#define SK_EVENT_MUTEX_RELEASE(x)
#endif

#if !defined(SK_EVENT_LOCK_ACQUIRE)
#define SK_EVENT_LOCK_ACQUIRE(x)
#endif

#if !defined(SK_EVENT_LOCK_RELEASE)
#define SK_EVENT_LOCK_RELEASE(x)
#endif


#endif /* _SKQUEUE_H_ */

