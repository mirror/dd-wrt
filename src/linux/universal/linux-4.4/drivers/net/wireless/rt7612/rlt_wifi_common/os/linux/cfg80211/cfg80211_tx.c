/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2013, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************
 
	Abstract:

	All related CFG80211 P2P function body.

	History:

***************************************************************************/

#define RTMP_MODULE_OS

#ifdef RT_CFG80211_SUPPORT

#include "rt_config.h"

VOID CFG80211_SwitchTxChannel(RTMP_ADAPTER *pAd, ULONG Data)
{
	//UCHAR lock_channel = CFG80211_getCenCh(pAd, Data);
	UCHAR lock_channel = Data;
	if (pAd->LatchRfRegs.Channel != lock_channel)
	{
		AsicSwitchChannel(pAd, lock_channel, FALSE);
		AsicLockChannel(pAd, lock_channel);
		
		DBGPRINT(RT_DEBUG_INFO, ("Off-Channel Send Packet: From(%d)-To(%d)\n", 
									pAd->LatchRfRegs.Channel, lock_channel));
	}
	else
		DBGPRINT(RT_DEBUG_INFO, ("Off-Channel Channel Equal: %d\n", pAd->LatchRfRegs.Channel));

}


static
PCFG80211_TX_PACKET CFG80211_TxMgmtFrameSearch(RTMP_ADAPTER *pAd, USHORT Sequence)
{
	PLIST_HEADER  pPacketList = &pAd->cfg80211_ctrl.cfg80211TxPacketList;
	PCFG80211_TX_PACKET pTxPkt = NULL;
	PLIST_ENTRY pListEntry = NULL;

	DBGPRINT(RT_DEBUG_ERROR, ("CFG_TX_STATUS: Search %d\n", Sequence));
	pListEntry = pPacketList->pHead;
	pTxPkt = (PCFG80211_TX_PACKET)pListEntry;

	while (pTxPkt != NULL)
	{
		if (pTxPkt->TxStatusSeq == Sequence)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("CFG_TX_STATUS: got %d\n", Sequence));
			return pTxPkt;
		}	
		
		pListEntry = pListEntry->pNext;
		pTxPkt = (PCFG80211_TX_PACKET)pListEntry;
	}	

}

INT CFG80211_SendMgmtFrame(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data)
{
	if (pData != NULL) 
	{
		{		
			PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;

			pCfg80211_ctrl->TxStatusInUsed = TRUE;
			pCfg80211_ctrl->TxStatusSeq = pAd->Sequence;

			if (pCfg80211_ctrl->pTxStatusBuf != NULL)
			{
				os_free_mem(NULL, pCfg80211_ctrl->pTxStatusBuf);
				pCfg80211_ctrl->pTxStatusBuf = NULL;
			}

			os_alloc_mem(NULL, (UCHAR **)&pCfg80211_ctrl->pTxStatusBuf, Data);
			if (pCfg80211_ctrl->pTxStatusBuf != NULL)
			{
				NdisCopyMemory(pCfg80211_ctrl->pTxStatusBuf, pData, Data);
				pCfg80211_ctrl->TxStatusBufLen = Data;
			}
			else
			{
				pCfg80211_ctrl->TxStatusBufLen = 0;
				DBGPRINT(RT_DEBUG_ERROR, ("CFG_TX_STATUS: MEM ALLOC ERROR\n"));
				return NDIS_STATUS_FAILURE;
			}
			CFG80211_CheckActionFrameType(pAd, "TX", pData, Data);


			MiniportMMRequest(pAd, 0, pData, Data);
		}
	}

}

VOID CFG80211_SendMgmtFrameDone(RTMP_ADAPTER *pAd, USHORT Sequence)
{
//RTMP_USB_SUPPORT/RTMP_PCI_SUPPORT
	PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;

	if (pCfg80211_ctrl->TxStatusInUsed && pCfg80211_ctrl->pTxStatusBuf 
		/*&& (pAd->TxStatusSeq == pHeader->Sequence)*/)
	{
		DBGPRINT(RT_DEBUG_INFO, ("CFG_TX_STATUS: REAL send %d\n", Sequence));
		
		CFG80211OS_TxStatus(CFG80211_GetEventDevice(pAd), 5678, 
							pCfg80211_ctrl->pTxStatusBuf, pCfg80211_ctrl->TxStatusBufLen, 
							TRUE);
		pCfg80211_ctrl->TxStatusSeq = 0;
		pCfg80211_ctrl->TxStatusInUsed = FALSE;
	} 


}

#endif /* RT_CFG80211_SUPPORT */
