/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2011, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	wnm.c

	Abstract:
	Wireless Network Management(WNM)

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#include "rt_config.h"

static char SolicitedMulticastAddr[] = {0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 
									  0x00, 0x00, 0x00, 0x00, 0x01, 0xff};  
static char AllNodeLinkLocalMulticastAddr[] = {0xff, 0x02, 0x00, 0x00, 0x00, 0x00,
											 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
											 0x00, 0x00, 0x00, 0x01};

static char link_local[] = {0xfe, 0x80};

#define IS_UNSPECIFIED_IPV6_ADDR(_addr)	\
		(!((_addr).ipv6_addr32[0] | (_addr).ipv6_addr32[1] | (_addr).ipv6_addr32[2] | (_addr).ipv6_addr32[3]))


void PeerWNMAction(IN PRTMP_ADAPTER pAd,
				   IN MLME_QUEUE_ELEM *Elem)
{
	UCHAR Action = Elem->Msg[LENGTH_802_11+1];

	switch(Action)
	{
		default:
			DBGPRINT(RT_DEBUG_TRACE, ("Invalid action field = %d\n", Action));
			break;
	}
}

VOID WNMCtrlInit(IN PRTMP_ADAPTER pAd)
{
	PWNM_CTRL pWNMCtrl;

}


static VOID WNMCtrlRemoveAllIE(PWNM_CTRL pWNMCtrl)
{
	if (pWNMCtrl->TimeadvertisementIELen)
	{
		pWNMCtrl->TimeadvertisementIELen = 0;
		os_free_mem(NULL, pWNMCtrl->TimeadvertisementIE);
	}
	
	if (pWNMCtrl->TimezoneIELen)
	{
		pWNMCtrl->TimezoneIELen = 0;
		os_free_mem(NULL, pWNMCtrl->TimezoneIE);
	}
}


VOID WNMCtrlExit(IN PRTMP_ADAPTER pAd)
{
	PWNM_CTRL pWNMCtrl;
	UINT32 Ret;



}


