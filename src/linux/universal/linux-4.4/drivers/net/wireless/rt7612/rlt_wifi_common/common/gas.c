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
	gas.c

	Abstract:
	generic advertisement service(GAS)

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#include "rt_config.h"


enum DOT11U_ADVERTISMENT_PROTOCOL_ID dot11GASAdvertisementID[] =
{
	ACCESS_NETWORK_QUERY_PROTOCOL,
};

enum GAS_STATE GASPeerCurrentState(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	PGAS_CTRL pGASCtrl;
	PGAS_PEER_ENTRY GASPeerEntry;
	PGAS_EVENT_DATA Event = (PGAS_EVENT_DATA)Elem->Msg;


#ifdef CONFIG_STA_SUPPORT
	pGASCtrl = &pAd->StaCfg.GASCtrl;
#endif /* CONFIG_STA_SUPPORT */	

	RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
	DlListForEach(GASPeerEntry, &pGASCtrl->GASPeerList, GAS_PEER_ENTRY, List)
	{
		if (MAC_ADDR_EQUAL(GASPeerEntry->PeerMACAddr, Event->PeerMACAddr))
		{
			
			RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
			return GASPeerEntry->CurrentState;
		}
	}
	RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);

	return GAS_UNKNOWN;
}


VOID GASSetPeerCurrentState(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem,
	IN enum GAS_STATE State)
{

	PGAS_CTRL pGASCtrl;
	PGAS_PEER_ENTRY GASPeerEntry;
	PGAS_EVENT_DATA Event = (PGAS_EVENT_DATA)Elem->Msg;


#ifdef CONFIG_STA_SUPPORT
	pGASCtrl = &pAd->StaCfg.GASCtrl;
#endif /* CONFIG_STA_SUPPORT */	

	RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
	DlListForEach(GASPeerEntry, &pGASCtrl->GASPeerList, GAS_PEER_ENTRY, List)
	{
		if (MAC_ADDR_EQUAL(GASPeerEntry->PeerMACAddr, Event->PeerMACAddr))
		{
			GASPeerEntry->CurrentState = State;
			break;
		}

	}
		RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
}



static VOID GASCtrlInit(IN PRTMP_ADAPTER pAd)
{
	PGAS_CTRL pGASCtrl;

#ifdef CONFIG_STA_SUPPORT
	pGASCtrl = &pAd->StaCfg.GASCtrl;
	NdisZeroMemory(pGASCtrl, sizeof(*pGASCtrl));
	NdisAllocateSpinLock(pAd, &pGASCtrl->GASPeerListLock);
	DlListInit(&pGASCtrl->GASPeerList);	
#endif

}

VOID GASCtrlExit(IN PRTMP_ADAPTER pAd)
{
	PGAS_CTRL pGASCtrl;
	GAS_PEER_ENTRY *GASPeerEntry, *GASPeerEntryTmp;
	GAS_QUERY_RSP_FRAGMENT *GASQueryRspFrag, *GASQueryRspFragTmp;
	BOOLEAN Cancelled;

#ifdef CONFIG_STA_SUPPORT
	pGASCtrl = &pAd->StaCfg.GASCtrl;

	RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
	/* Remove all GAS peer entry */
	DlListForEachSafe(GASPeerEntry, GASPeerEntryTmp, 
						&pGASCtrl->GASPeerList, GAS_PEER_ENTRY, List)
	{
		
		DlListDel(&GASPeerEntry->List);
		
		DlListForEachSafe(GASQueryRspFrag, GASQueryRspFragTmp, 
			&GASPeerEntry->GASQueryRspFragList, GAS_QUERY_RSP_FRAGMENT, List)
		{
			DlListDel(&GASQueryRspFrag->List);
			os_free_mem(NULL, GASQueryRspFrag->FragQueryRsp);
			os_free_mem(NULL, GASQueryRspFrag);
		}

		DlListInit(&GASPeerEntry->GASQueryRspFragList);
		
		if (GASPeerEntry->GASResponseTimerRunning)
		{
			RTMPCancelTimer(&GASPeerEntry->GASResponseTimer, &Cancelled);
			GASPeerEntry->GASResponseTimerRunning = FALSE;
		}

		if (GASPeerEntry->GASCBDelayTimerRunning)
		{
			RTMPCancelTimer(&GASPeerEntry->GASCBDelayTimer, &Cancelled);
			GASPeerEntry->GASCBDelayTimerRunning = FALSE;
		}
		
		RTMPReleaseTimer(&GASPeerEntry->GASResponseTimer, &Cancelled);
		RTMPReleaseTimer(&GASPeerEntry->GASCBDelayTimer, &Cancelled);
		os_free_mem(NULL, GASPeerEntry);

	}
	
	DlListInit(&pGASCtrl->GASPeerList);
	RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);
	NdisFreeSpinLock(&pGASCtrl->GASPeerListLock);
#endif

}


VOID GASStateMachineInit(
			IN PRTMP_ADAPTER pAd, 
			IN STATE_MACHINE *S, 
			OUT STATE_MACHINE_FUNC	Trans[])
{
	DBGPRINT(RT_DEBUG_TRACE, ("%s\n", __FUNCTION__));

	GASCtrlInit(pAd);

	StateMachineInit(S,	(STATE_MACHINE_FUNC*)Trans, MAX_GAS_STATE, MAX_GAS_MSG, (STATE_MACHINE_FUNC)Drop, GAS_UNKNOWN, GAS_MACHINE_BASE);

}
