/******************************************************************************
 *
 * Name:	skhwt.h
 * Project:	Gigabit Ethernet Adapters, Event Scheduler Module
 * Version:	$Revision: #5 $
 * Date:	$Date: 2010/11/04 $
 * Purpose:	Defines for the hardware timer functions
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
 *	$Log: skgehwt.h,v $
 *	Revision 2.3  2007/07/31 12:58:09  rschmidt
 *	Used SK_BOOL for HWT flag
 *	Editorial changes
 *	
 *	Revision 2.2  2005/12/14 16:11:03  ibrueder
 *	Added copyright header flags
 *
 *	Revision 2.1  2003/10/27 14:16:09  amock
 *	Promoted to major revision 2.1
 *
 *	Revision 1.1  2003/10/27 14:14:18  amock
 *	Copied to yukon2 module branch
 *
 *	Revision 1.7  2003/09/16 12:55:08  rschmidt
 *	Editorial changes
 *
 *	Revision 1.6  2003/05/13 17:57:48  mkarl
 *	Editorial changes.
 *
 *	Revision 1.5  1999/11/22 13:54:24  cgoos
 *	Changed license header to GPL.
 *
 *	Revision 1.4  1998/08/19 09:50:58  gklug
 *	fix: remove struct keyword from C-code (see CCC) add typedefs
 *
 *	Revision 1.3  1998/08/14 07:09:29  gklug
 *	fix: chg pAc -> pAC
 *
 *	Revision 1.2  1998/08/07 12:54:21  gklug
 *	fix: first compiled version
 *
 *	Revision 1.1  1998/08/07 09:32:58  gklug
 *	first version
 *
 ******************************************************************************/

/*
 * SKGEHWT.H	contains all defines and types for the timer functions
 */

#ifndef _SKGEHWT_H_
#define _SKGEHWT_H_

/*
 * SK Hardware Timer
 * - needed wherever the HWT module is used
 * - use in Adapters context name pAC->Hwt
 */
typedef	struct s_Hwt {
	SK_U32		TStart;		/* HWT start */
	SK_U32		TStop;		/* HWT stop */
	SK_BOOL		TActive;	/* HWT: flag : active/inactive */
} SK_HWT;

extern void SkHwtInit(SK_AC *pAC, SK_IOC IoC);
extern void SkHwtStart(SK_AC *pAC, SK_IOC IoC, SK_U32 Time);
extern void SkHwtStop(SK_AC *pAC, SK_IOC IoC);
extern SK_U32 SkHwtRead(SK_AC *pAC, SK_IOC IoC);
extern void SkHwtIsr(SK_AC *pAC, SK_IOC IoC);
#endif /* _SKGEHWT_H_ */

