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
	hotspot.c

	Abstract:
	hotspot2.0 features

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#include "rt_config.h"

extern UCHAR BROADCAST_ADDR[MAC_ADDR_LEN];

void wext_hotspot_onoff_event(PNET_DEV net_dev, int onoff)
{
	struct hs_onoff *hotspot_onoff;
	u16 buflen = 0;
	char *buf;

	buflen = sizeof(*hotspot_onoff);

	os_alloc_mem(NULL, (UCHAR **)&buf, buflen);
	NdisZeroMemory(buf, buflen);

	hotspot_onoff = (struct hs_onoff *)buf;
	hotspot_onoff->ifindex = RtmpOsGetNetIfIndex(net_dev);
	hotspot_onoff->hs_onoff = onoff;
	
	RtmpOSWrielessEventSend(net_dev, RT_WLAN_EVENT_CUSTOM, 
					OID_802_11_HS_ONOFF, NULL, (PUCHAR)buf, buflen);
	os_free_mem(NULL, buf);
}


void HotspotOnOffEvent(PNET_DEV net_dev, int onoff)
{

	wext_hotspot_onoff_event(net_dev, onoff);
}


static void wext_hotspot_ap_reload_event(PNET_DEV net_dev)
{
	struct hs_onoff *hotspot_onoff;
	u16 buflen = 0;
	char *buf;
	
	buflen = sizeof(*hotspot_onoff);
	os_alloc_mem(NULL, (UCHAR **)&buf, buflen);
	NdisZeroMemory(buf, buflen);

	hotspot_onoff = (struct hs_onoff *)buf;
	hotspot_onoff->ifindex = RtmpOsGetNetIfIndex(net_dev);
	
	RtmpOSWrielessEventSend(net_dev, RT_WLAN_EVENT_CUSTOM, 
					OID_802_11_HS_AP_RELOAD, NULL, (PUCHAR)buf, buflen);
	os_free_mem(NULL, buf);
}


void HotspotAPReload(PNET_DEV net_dev)
{
	wext_hotspot_ap_reload_event(net_dev);
}



VOID HSCtrlRemoveAllIE(PHOTSPOT_CTRL pHSCtrl)
{

	/* Remove all IE from daemon */
	if (pHSCtrl->P2PIELen)
	{
		pHSCtrl->P2PIELen = 0;
		os_free_mem(NULL, pHSCtrl->P2PIE);
	}

	if (pHSCtrl->HSIndicationIELen)
	{
		pHSCtrl->HSIndicationIELen = 0;
		os_free_mem(NULL, pHSCtrl->HSIndicationIE);
	}

	if (pHSCtrl->InterWorkingIELen)
	{
		pHSCtrl->InterWorkingIELen = 0;
		os_free_mem(NULL, pHSCtrl->InterWorkingIE);
	}

	if (pHSCtrl->AdvertisementProtoIELen)
	{
		pHSCtrl->AdvertisementProtoIELen = 0;
		os_free_mem(NULL, pHSCtrl->AdvertisementProtoIE);
	}

	if (pHSCtrl->QosMapSetIELen)
	{
		pHSCtrl->AdvertisementProtoIELen = 0;
		os_free_mem(NULL, pHSCtrl->QosMapSetIE);
	}

	if (pHSCtrl->RoamingConsortiumIELen)
	{
		pHSCtrl->RoamingConsortiumIELen = 0;
		os_free_mem(NULL, pHSCtrl->RoamingConsortiumIE);
	}
}



INT Set_HotSpot_OnOff(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 OnOff,
	IN UINT8 EventTrigger,
	IN UINT8 EventType)
{
	UCHAR *Buf;
	HSCTRL_EVENT_DATA *Event;
	UINT32 Len = 0;

	os_alloc_mem(NULL, (UCHAR **)&Buf, sizeof(*Event));

	if (!Buf)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s Not available memory\n", __FUNCTION__));
		return FALSE;
	}

	NdisZeroMemory(Buf, sizeof(*Event));
		
	Event = (HSCTRL_EVENT_DATA *)Buf;
#ifdef CONFIG_STA_SUPPORT
	Event->ControlIndex = 0;
#endif /*CONFIG_STA_SUPPORT */
	Len += 1;

	Event->EventTrigger = EventTrigger;
	Len += 1;

	Event->EventType = EventType;
	Len += 1;

	if (OnOff)
		MlmeEnqueue(pAd, HSCTRL_STATE_MACHINE, HSCTRL_ON, Len, Buf, 0);
	else
		MlmeEnqueue(pAd, HSCTRL_STATE_MACHINE, HSCTRL_OFF, Len, Buf, 0);
	
	os_free_mem(NULL, Buf);
	
	return TRUE;
}


enum HSCTRL_STATE HSCtrlCurrentState(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	PHOTSPOT_CTRL pHSCtrl;


#ifdef CONFIG_STA_SUPPORT
	pHSCtrl = &pAd->StaCfg.HotSpotCtrl;
#endif /* CONFIG_STA_SUPPORT */

	return pHSCtrl->HSCtrlState;
}


VOID HSCtrlSetCurrentState(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem,
	IN enum HSCTRL_STATE State)
{

	PHOTSPOT_CTRL pHSCtrl;


#ifdef CONFIG_STA_SUPPORT
	pHSCtrl = &pAd->StaCfg.HotSpotCtrl;
#endif /* CONFIG_STA_SUPPORT */	

	pHSCtrl->HSCtrlState = State;
}


static VOID HSCtrlOn(
    IN PRTMP_ADAPTER    pAd, 
    IN MLME_QUEUE_ELEM  *Elem)
{
	PHOTSPOT_CTRL pHSCtrl;
	PGAS_CTRL pGASCtrl;
	PNET_DEV NetDev;
	HSCTRL_EVENT_DATA *Event = (HSCTRL_EVENT_DATA *)Elem->Msg;

	printk("%s\n", __FUNCTION__);

#ifdef CONFIG_STA_SUPPORT
	NetDev = pAd->net_dev;
	pHSCtrl = &pAd->StaCfg.HotSpotCtrl;
	pGASCtrl = &pAd->StaCfg.GASCtrl;
#endif /* CONFIG_STA_SUPPORT */


	RTMP_SEM_LOCK(&pGASCtrl->GASPeerListLock);
	DlListInit(&pGASCtrl->GASPeerList);
	RTMP_SEM_UNLOCK(&pGASCtrl->GASPeerListLock);

	pHSCtrl->HotSpotEnable = 1;
	pHSCtrl->HSDaemonReady = 1;

	HSCtrlSetCurrentState(pAd, Elem, HSCTRL_IDLE);

	/* Send indication to daemon */
	if (Event->EventTrigger) {
		switch (Event->EventType) {
		case HS_ON_OFF_BASE: 
			HotspotOnOffEvent(NetDev, 1);
			break;
		case HS_AP_RELOAD:
			HotspotAPReload(NetDev); 
			break;
		default:
			DBGPRINT(RT_DEBUG_ERROR, ("%s: Unknow event type(%d)\n", __FUNCTION__, Event->EventType));
			break;
		}
	}
}


static VOID HSCtrlInit(
	IN PRTMP_ADAPTER pAd)
{
	PHOTSPOT_CTRL pHSCtrl;

#ifdef CONFIG_STA_SUPPORT
	pHSCtrl = &pAd->StaCfg.HotSpotCtrl;
	NdisZeroMemory(pHSCtrl, sizeof(*pHSCtrl));
	pHSCtrl->HotSpotEnable = 0;
	pHSCtrl->HSCtrlState = HSCTRL_IDLE;
#endif /* CONFIG_STA_SUPPORT */

}


VOID HSCtrlExit(
	IN PRTMP_ADAPTER pAd)
{
	PHOTSPOT_CTRL pHSCtrl;

#ifdef CONFIG_STA_SUPPORT
	pHSCtrl = &pAd->StaCfg.HotSpotCtrl;
	
	/* Remove all IE */
	HSCtrlRemoveAllIE(pHSCtrl);
#endif /* CONFIG_STA_SUPPORT */
	
}


VOID HSCtrlHalt(
	IN PRTMP_ADAPTER pAd)
{
	
	PHOTSPOT_CTRL pHSCtrl;

#ifdef CONFIG_STA_SUPPORT
	pHSCtrl = &pAd->StaCfg.HotSpotCtrl;
	pHSCtrl->HotSpotEnable = 0;
#endif /* CONFIG_STA_SUPPORT */

}

static VOID HSCtrlOff(
    IN PRTMP_ADAPTER    pAd, 
    IN MLME_QUEUE_ELEM  *Elem)
{
	PHOTSPOT_CTRL pHSCtrl;
	PGAS_CTRL pGASCtrl;
	PNET_DEV NetDev;
	HSCTRL_EVENT_DATA *Event = (HSCTRL_EVENT_DATA *)Elem->Msg;

	printk("%s\n", __FUNCTION__);


 	pHSCtrl->HotSpotEnable = 0;
	pHSCtrl->HSDaemonReady = 0;
	

	HSCtrlSetCurrentState(pAd, Elem, HSCTRL_IDLE);
	
	if (Event->EventTrigger) {
		switch (Event->EventType) {
		case HS_ON_OFF_BASE: 
			HotspotOnOffEvent(NetDev, 0);
			break;
		case HS_AP_RELOAD:
			HotspotAPReload(NetDev); 
			break;
		default:
			DBGPRINT(RT_DEBUG_ERROR, ("%s: Unknow event type(%d)\n", __FUNCTION__, Event->EventType));
			break;
		}
	}
}

BOOLEAN HotSpotEnable(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem,
	IN INT Type)
{
	PHOTSPOT_CTRL pHSCtrl = NULL;



#ifdef CONFIG_STA_SUPPORT
	pHSCtrl = &pAd->StaCfg.HotSpotCtrl;
#endif /* CONFIG_STA_SUPPORT */

	return pHSCtrl->HotSpotEnable;
}


VOID HSCtrlStateMachineInit(
	IN	PRTMP_ADAPTER		pAd, 
	IN	STATE_MACHINE		*S, 
	OUT	STATE_MACHINE_FUNC	Trans[])
{

	DBGPRINT(RT_DEBUG_TRACE, ("%s\n", __FUNCTION__));
	
	HSCtrlInit(pAd);

	StateMachineInit(S,	(STATE_MACHINE_FUNC*)Trans, MAX_HSCTRL_STATE, MAX_HSCTRL_MSG, (STATE_MACHINE_FUNC)Drop, HSCTRL_IDLE, HSCTRL_MACHINE_BASE);

	StateMachineSetAction(S, HSCTRL_IDLE, HSCTRL_ON, (STATE_MACHINE_FUNC)HSCtrlOn);
	StateMachineSetAction(S, HSCTRL_IDLE, HSCTRL_OFF, (STATE_MACHINE_FUNC)HSCtrlOff);
}
