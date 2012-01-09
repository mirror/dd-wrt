/*
* Copyright c                  Realtek Semiconductor Corporation, 2006 
* All rights reserved.
* 
* Program : RTL8306 switch high-level API for RTL8306S/RTL8306SD/RTL8306SDM
* Abstract : 
* Author : Robin Zheng-bei Xing(robin_xing@realsil.com.cn)  
*		:Ralph Jianwei Yu(Ralph_yu@realsil.com.cn)
*  $Id: Rtl8306_Driver_s.c,v 1.2 2007/08/03 03:59:52 michael Exp $
*/
/*	@doc RTL8306_DRIVER_s_API

	@module Rtl8306_Driver_s.c - RTL8306 Switch driver API documentation	|
	This document explains API interface for RTL8306S/RTL8306SD/RTL8306SDM . 
	@normal 

	Copyright <cp>2006 Realtek<tm> Semiconductor Cooperation, All Rights Reserved.

 	@head3 List of Symbols |
 	Here is a list of all functions and variables in this module.

 	@index | RTL8306_DRIVER_s_API
*/

#include "Rtl8306_types.h"
#include "Rtl8306_AsicDrv.h"
#include "Rtl8306_Driver_s.h"

#ifdef RTL8306_TBLBAK        
    rtl8306_ConfigBakPara_t rtl8306_TblBak;
    
#endif 

    
/*
@func int32 | rtl8306_setEthernetPHY | Configure PHY setting.
@parm uint32 | phy | Specify the phy to configure (0~6).
@parm uint32 | autoNegotiation | Specify whether enable auto-negotiation.
@parm uint32 | advCapability | When auto-negotiation is enabled, specify the advertised capability.
@parm uint32 | speed | When auto-negotiation is disabled, specify the force mode speed.
@parm uint32 | fullDuplex | When auto-negotiatoin is disabled, specify the force mode duplex mode.
@rvalue SUCCESS 
@rvalue FAILED
@comm
phy 0 ~4 correspond port 0~ 4, phy 5 correspond port 4 mac, and phy 6 correspond port5, port 4 mac and port 5 is mii interface
When auto-negotiation is enabled, the advertisement capability is used to handshaking with link partner.
Wehn auto-negotiation is disabled, the phy is configured into force mode and the speed and duplex mode setting is based on speed and fullDuplex setting.
AdverCapability should be ranged between RTL8306_ETHER_AUTO_100FULL and RTL8306_ETHER_AUTO_10HALF.
Speed should be either RTL8306_ETHER_SPEED_100 or RTL8306_ETHER_SPEED_10.

*/

int32 rtl8306_setEthernetPHY(uint32 phy, uint32 autoNegotiation, uint32 advCapability, uint32 speed, uint32 fullDuplex) {

    /*phy 5, 6 are mii interface*/
    if ((phy == 5) ||(phy == 6))
        autoNegotiation = FALSE;
    if (rtl8306_setAsicEthernetPHY(phy, autoNegotiation, advCapability, speed, fullDuplex) == FAILED)
        return FAILED;
    return SUCCESS;
}


/*
@func int32 | rtl8306_getEthernetPHY | Get PHY setting.
@parm uint32 | phy | Specify the phy to get setting
@parm uint32 * | autoNegotiation | Specify whether auto-negotiation is enabled.
@parm uint32 * | advCapability | When auto-negotiation is enabled, get the advertised capability.
@parm uint32 * | speed | When auto-negotiation is disabled, get the force mode speed.
@parm uint32 * | fullDuplex | When auto-negotiatoin is disabled, get the force mode duplex mode.
@rvalue SUCCESS 
@rvalue FAILED
@comm
Either auto-negotiation advertised capability or force mode speed/duplex setting is valid.
If auto-negotiation is enabled, the advertisement capability is valid.
If auto-negotiation is diabled, the speed and duplex mode setting are valid.
If any of the pointer is NULL, the function will return FAILED.
*/

int32 rtl8306_getEthernetPHY(uint32 phy, uint32 *autoNegotiation, uint32 *advCapability, uint32 *speed, uint32 *fullDuplex) {

    if (rtl8306_getAsicEthernetPHY(phy, autoNegotiation,advCapability, speed, fullDuplex) == FAILED)
        return FAILED;
    return SUCCESS;
}


/*
@func int32 | rtl8306_getPHYLinkStatus | Get PHY Link Status.
@parm uint32 | phy | Specify the phy to get setting
@parm uint32 * | linkUp | Describe whether link status is up or not.
@rvalue SUCCESS 
@rvalue FAILED
@comm
	Read the link status of PHY register 1. Latched on 0 until read this once again.
*/
int32 rtl8306_getPHYLinkStatus(uint32 phy, uint32 *linkUp) {

    if (rtl8306_getAsicPHYLinkStatus(phy, linkUp) == FAILED)
        return FAILED;
    return SUCCESS;
}

/*
@func  int32 | rtl8306_setPort5LinkStatus | Specify port 5 link up / link down.
@parm uint32 | enabled | Ture or False.
@rvalue SUCCESS 
@rvalue FAILED
@comm
Port 5 should be manully enable / disable.
*/
int32 rtl8306_setPort5LinkStatus(uint32 enabled) {
    if (rtl8306_setAsicPort5LinkStatus(enabled) == FAILED)
        return FAILED;
    return SUCCESS;
}


/*
@func int32 | rtl8306_initVlan | Initialize Vlan before use vlan
@rvalue SUCCESS 
@rvalue FAILED
@comm
	Because the vlan is disabled as the default configuration, user must call the function before using vlan.
	It will clear vlan table and set a default vlan(vid 1), which all ports are in the vlan,
	and set all ports vid to the default vlan.
*/
int32 rtl8306_initVlan(void)
{
	uint32 i;


	/*clear vlan table*/
	for(i = 0;i < 16; i++)
		rtl8306_setAsicVlan(i, 0, 0);

	/*set switch default configuration */
	rtl8306_setVlanTagAware(TRUE);	/*enable tag aware*/
	rtl8306_setIngressFilter(FALSE);		/*disable ingress filter*/
	rtl8306_setVlanTagOnly(FALSE);		/*disable vlan tag only*/

	/*add a default vlan which contains all ports*/
	rtl8306_addVlan(1);
	for(i = 0; i < 6; i++)
		rtl8306_addVlanPortMember(1, i);
	/*set all ports' vid to vlan 1*/
	for(i = 0; i < 6; i++)
		rtl8306_setPvid(i, 1);

	/*set vlan enabled*/
	rtl8306_setAsicVlanEnable(TRUE);
	return SUCCESS;
}

/*
@func int32 | rtl8306_addVlan | Add a Vlan by vid
@parm uint32 | vid | Specify the VLAN ID
@rvalue SUCCESS 
@rvalue FAILED
@rvalue RTL8306_VLAN_VIDEXISTED
@rvalue RTL8306_VLAN_FULL
@comm
	Totally there are 16 Vlans could be added. Vid should be 1~4094.
	User could not add a vid that has existed in the vlan,or it will return RTL8306_VLAN_VIDEXISTED error.
	If the vlan table is full, user can not add a vlan any more. It will return  RTL8306_VLAN_FULL. 
	User can delete a vlan using rtl8306_delVlan and then add a vlan again.
	Notice: the function does not set vlan port member, so remember call rtl8306_addVlanPortMember to set port members
*/
int32 rtl8306_addVlan(uint32 vid)
{
	uint32 vidData,memData,index;
	uint32 fullflag;
	int32 i;
	/*check vid*/
	if ( vid < 1 || vid > 4094 )
		return FAILED;

	/*check if vid exists and check if vlan is full*/
	fullflag = TRUE;
	index = 16;
	for(i = 15;i >= 0; i--)
	{
		rtl8306_getAsicVlan(i, &vidData, &memData);
		if (vidData == 0)	/*has empty entry*/
		{
			index = i;	/*set available Vlan Entry index*/
			fullflag = FALSE;
			continue;
		}
		if (vidData == vid)
			return RTL8306_VLAN_VIDEXISTED;
	}
	if (fullflag == TRUE)
		return RTL8306_VLAN_FULL;

	/*check over and add Vlan*/
	rtl8306_setAsicVlan(index, vid, 0);
	return SUCCESS;
	
}

/*
@func int32 | rtl8306_delVlan | delete a Vlan by vid
@parm uint32 | vid | Specify the VLAN ID
@rvalue SUCCESS 
@rvalue FAILED
@rvalue RTL8306_VLAN_VIDNOTEXIST
@comm
	 Vid should be 1~4094.
	 If the vid does not exist, it will return RTL8306_VLAN_VIDNOTEXIST.
*/
int32 rtl8306_delVlan(uint32 vid)
{
	uint32 vidData,memData;
	uint32 i,index;
	/*check vid*/
	if ( vid < 1 || vid > 4094 )
		return FAILED;
	
	/*search the vid*/
	index = 16;	/*init a invalid value*/
	for(i = 0; i  < 16; i++)
	{
		rtl8306_getAsicVlan(i, &vidData, &memData);
		if ( vidData == vid)
		{
			index = i;
			break;
		}			
	}
	
	if (index == 16)	/*vid not exists*/
		return RTL8306_VLAN_VIDNOTEXISTS;

	/*delete the vid*/
	rtl8306_setAsicVlan(index,0,0);
	return SUCCESS;
	
}

/*
@func int32 | rtl8306_addVlanPortMember | Add a port into an existed vlan
@parm uint32 | vid | Specify the VLAN ID
@parm uint32 | port | Port number to be added into the vlan
@rvalue SUCCESS 
@rvalue FAILED
@rvalue RTL8306_VLAN_VIDNOTEXIST
@comm
	 Make sure the vid has existed. Otherwise the Vlan must be added using rtl8306_addVlan() first.
	 Vid should be 1~4094. Port should be 0~5.
*/
int32 rtl8306_addVlanPortMember(uint32 vid, uint32 port)
{
	uint32 vidData,memData;
	uint32 i,index;

	/*check vid*/
	if ( vid < 1 || vid > 4094 )
		return FAILED;
	/*check port number*/
	if (port > 5)
		return FAILED;

	/*search the vid*/
	index = 16;	/*init a invalid value*/
	for(i = 0; i  < 16; i++)
	{
		rtl8306_getAsicVlan(i, &vidData, &memData);
		if ( vidData == vid)
		{
			index = i;
			break;
		}			
	}
	
	if (index == 16)	/*vid not exists*/
		return RTL8306_VLAN_VIDNOTEXISTS;

	/*add the port number to the Vlan*/
	memData |= 1 << port;
	rtl8306_setAsicVlan(index, vid, memData);
    
	return SUCCESS;
}

/*
@func int32 | rtl8306_delVlanPortMember | delete a port in an existed vlan
@parm uint32 | vid | Specify the VLAN ID
@parm uint32 | port | Port number to be deleted in the vlan
@rvalue SUCCESS 
@rvalue FAILED
@rvalue RTL8306_VLAN_INVALIDPORT
@rvalue RTL8306_VLAN_VIDNOTEXIST
@comm
	 The function does not check if the port was in the vlan member set.
	 But it guarantees the port will not in the vlan member set when this function is called successfully.
	 Vid should be 1~4094. Port should be 0~5.
*/
int32 rtl8306_delVlanPortMember(uint32 vid, uint32 port)
{
	uint32 vidData,memData;
	uint32 i,index;

	/*check vid*/
	if ( vid < 1 || vid > 4094 )
		return FAILED;
	/*check port number*/
	if (port > 5)
		return FAILED;

	/*search the vid*/
	index = 16;	/*init a invalid value*/
	for(i = 0; i  < 16; i++)
	{
		rtl8306_getAsicVlan(i, &vidData, &memData);
		if ( vidData == vid)
		{
			index = i;
			break;
		}			
	}
	
	if (index == 16)	/*vid not exists*/
		return RTL8306_VLAN_VIDNOTEXISTS;

	/*delete the port number in the Vlan*/
	memData &= ~(1 << port); /*set the port member maskbit to 0*/
	rtl8306_setAsicVlan(index, vid, memData);
    
	return SUCCESS;
	
}

/*
@func int32 | rtl8306_getVlanPortMember | get a vlan's portmask
@parm uint32 | vid | Specify the VLAN ID
@parm uint32* | portmask | a point to get portmask
@rvalue SUCCESS 
@rvalue FAILED
@rvalue RTL8306_VLAN_VIDNOTEXIST
@comm
	 Vid should be 1~4094.
	 If the vid does not exist, it will return RTL8306_VLAN_VIDNOTEXIST.
	 The portmask bit0 means port0.(portmake 23 -- 010111 means port 0,1,2,4 in the member set)
*/
int32 rtl8306_getVlanPortMember(uint32 vid, uint32 *portmask)
{
	uint32 vidData,memData;
	uint32 i,index;

	/*check vid*/
	if ( vid < 1 || vid > 4094 )
		return FAILED;
	/*check portmask*/
	if (portmask == NULL)
		return FAILED;

	/*search the vid*/
	index = 16;	/*init a invalid value*/
	for(i = 0; i  < 16; i++)
	{
		rtl8306_getAsicVlan(i, &vidData, &memData);
		if ( vidData == vid)
		{
			index = i;
			break;
		}			
	}
	
	if (index == 16)	/*vid not exists*/
		return RTL8306_VLAN_VIDNOTEXISTS;

	/*get the portmask of the Vlan*/
	rtl8306_getAsicVlan(index, &vid, portmask);
	return SUCCESS;
}

/*
@func int32 | rtl8306_setPvid | set a given port's pvid
@parm uint32 | port | The given port.
@parm uint32 | vid |  vid
@rvalue SUCCESS 
@rvalue FAILED
@rvalue RTL8306_VLAN_VIDNOTEXISTS
@comm
	 Vid should be 1~4094. Port should be 0~5.
	 Make sure the vid has existed in the vlan table, or it will return a RTL8306_VLAN_VIDNOTEXISTS error.
*/
int32 rtl8306_setPvid(uint32 port, uint32 vid);

int32 rtl8306_setPvid(uint32 port, uint32 vid)
{
	uint32 vidData,memData;
	uint32 i,index;
	
	/*check vid*/
	if ( vid > 4094 || vid < 1)
		return FAILED;
	
	/*check port number*/
	if (port > 5)
		return FAILED;

	/*For init only*/
	if ( vid == 0)
		rtl8306_setAsicPortVlanIndex(port, 0);

	/*search the vid*/
	index = 16;	/*init a invalid value*/
	for(i = 0; i  < 16; i++)
	{
		rtl8306_getAsicVlan(i, &vidData, &memData);
		if ( vidData == vid)
		{
			index = i;
			break;
		}			
	}
	
	if (index == 16)	/*vid not exists*/
		return RTL8306_VLAN_VIDNOTEXISTS;

	/*set pvid*/
	rtl8306_setAsicPortVlanIndex(port,index);
    
	
	return SUCCESS;
}

/*
@func int32 | rtl8306_getPvid | get a given port's pvid
@parm uint32 | port | The given port.
@parm uint32* | vid | a point to fetch the port's pvid
@rvalue SUCCESS 
@rvalue FAILED
@comm
	 Port should be 0~5. Return value by the vid point. 
*/
int32 rtl8306_getPvid(uint32 port, uint32 *vid)
{
	uint32 index,memData;

	/*check port number*/
	if (port > 5)
		return FAILED;

	/*check vid*/
	if (vid == NULL)
		return FAILED;
		
	/*get the pvid*/
	rtl8306_getAsicPortVlanIndex(port ,&index);
	rtl8306_getAsicVlan(index,vid,&memData);

	return SUCCESS;
}

/*
@func int32 | rtl8306_setIngressFilter | enable or disable the vlan ingress filter
@parm uint32 | enabled | TRUE or FALSE
@rvalue SUCCESS 
@rvalue FAILED
@comm
	If enable the filter(set TRUE),the switch will drop the received frames if the ingress port is 
	not included in the matched VLAN member set. 
	The default set is FALSE.
*/
int32 rtl8306_setIngressFilter(uint32 enabled)
{
	if ( !(enabled == TRUE || enabled == FALSE) )
		return FAILED;
	rtl8306_setAsicVlanIngressFilter(enabled);
	return SUCCESS;
}

/*
@func int32 | rtl8306_setVlanTagOnly | set the switch is vlan taged only or not
@parm uint32 | enabled | TRUE or FALSE
@rvalue SUCCESS 
@rvalue FAILED
@comm
  	If enable the filter(set TRUE),the switch will only accept tagged frames 
  	and will drop untagged frames.
  	The default set is FALSE.
*/
int32 rtl8306_setVlanTagOnly(uint32 enabled)
{
	if ( !(enabled == TRUE || enabled == FALSE) )
		return FAILED;
	rtl8306_setAsicVlanTaggedOnly(enabled);
	return SUCCESS;
}

/*
@func int32 | rtl8306_setVlanTagAware | set the switch is vlan taged aware or not
@parm uint32 | enabled | TRUE or FALSE
@rvalue SUCCESS 
@rvalue FAILED
@comm
  	If enable the filter(set TRUE),the switch will check the tagged VID 
  	and then performs VLAN mapping, but still use port-based VLAN mapping 
  	for priority-tagged and untagged frames.
  	The default set is TRUE in rtl8306_initVlan.
*/
int32 rtl8306_setVlanTagAware(uint32 enabled)
{
	if ( !(enabled == TRUE || enabled == FALSE) )
		return FAILED;
	rtl8306_setAsicVlanTagAware(enabled);
	return SUCCESS;
}



/*
@func int32 | rtl8306_init | init the asic
@rvalue SUCCESS 
@rvalue FAILED
*/
int32 rtl8306_init(void) {

    asicVersionPara_t AsicVer;

#ifdef   RTL8306_TBLBAK
        uint32 cnt;
        /*Vlan default value*/
        rtl8306_TblBak.vlanConfig.enVlan = FALSE;
        rtl8306_TblBak.vlanConfig.enArpVlan = FALSE;
        rtl8306_TblBak.vlanConfig.enLeakVlan = FALSE;
        rtl8306_TblBak.vlanConfig.enVlanTagOnly = FALSE;
        rtl8306_TblBak.vlanConfig.enIngress =  FALSE;
        rtl8306_TblBak.vlanConfig.enTagAware = FALSE;
        rtl8306_TblBak.vlanConfig.enIPMleaky = FALSE;
        rtl8306_TblBak.vlanConfig.enMirLeaky = FALSE;
        for (cnt = 0; cnt < 6; cnt++) {
            rtl8306_TblBak.vlanConfig_perport[cnt].vlantagInserRm = RTL8306_VLAN_UNDOTAG;
            rtl8306_TblBak.vlanConfig_perport[cnt].en1PRemark = FALSE;
            rtl8306_TblBak.vlanConfig_perport[cnt].enNulPvidRep =  FALSE;
        }
        for (cnt = 0; cnt < 16; cnt++) {
            rtl8306_TblBak.vlanTable[cnt].vid = cnt;
            if ((cnt % 5) == 4 ) {            
                rtl8306_TblBak.vlanTable[cnt].memberPortMask = 0x1F;                
            } 
            else  {
                rtl8306_TblBak.vlanTable[cnt].memberPortMask = (0x1<<4) | (0x1 << (cnt % 5));
            }
        }
        for (cnt = 0; cnt < 6; cnt++) {

            rtl8306_TblBak.vlanPvidIdx[cnt] = (uint8)cnt;
            rtl8306_TblBak.dot1DportCtl[cnt] = RTL8306_SPAN_FORWARD;
        }
        rtl8306_TblBak.En1PremarkPortMask = 0;
        rtl8306_TblBak.dot1PremarkCtl[0] = 0x3;
        rtl8306_TblBak.dot1PremarkCtl[1] = 0x4;
        rtl8306_TblBak.dot1PremarkCtl[2] = 0x5;
        rtl8306_TblBak.dot1PremarkCtl[3] = 0x6;
        
        for (cnt = 0; cnt < RTL8306_ACL_ENTRYNUM; cnt++) {
            rtl8306_TblBak.aclTbl[cnt].phy_port = RTL8306_ACL_INVALID;
            rtl8306_TblBak.aclTbl[cnt].proto = RTL8306_ACL_ETHER;
            rtl8306_TblBak.aclTbl[cnt].data = 0;
            rtl8306_TblBak.aclTbl[cnt].action = RTL8306_ACT_PERMIT;
            rtl8306_TblBak.aclTbl[cnt].pri = RTL8306_PRIO0;            
        }
        rtl8306_TblBak.mir.mirPort = 0x7;
        rtl8306_TblBak.mir.mirRxPortMask = 0;
        rtl8306_TblBak.mir.mirTxPortMask = 0;
        rtl8306_TblBak.mir.enMirself = FALSE;
        rtl8306_TblBak.mir.enMirMac = FALSE;
        rtl8306_TblBak.mir.mir_mac[0] = 0x0;
        rtl8306_TblBak.mir.mir_mac[1] = 0x0;
        rtl8306_TblBak.mir.mir_mac[2] = 0x0;
        rtl8306_TblBak.mir.mir_mac[3] = 0x0;
        rtl8306_TblBak.mir.mir_mac[4] = 0x0;
        rtl8306_TblBak.mir.mir_mac[5] = 0x0;        

#endif

    /*Fix EQC problem in Version B of RTL8306 series*/

    rtl8306_getAsicVersionInfo(&AsicVer);
    if ((AsicVer.chipid == RTL8306_CHIPID) && 
        (AsicVer.vernum == RTL8306_VERNUM) && 
        (AsicVer.revision == 0x0)  )
    {
        rtl8306_setAsicPhyReg(2, 26, 0, 0x0056);
    }
    
    rtl8306_setAsicCPUPort(RTL8306_NOCPUPORT, FALSE);
    rtl8306_setAsicStormFilterEnable(RTL8306_BROADCASTPKT, TRUE);
    
    return SUCCESS;
} 


/*
@func int32 | rtl8306_initQos | Init qos configuration
@parm uint32 | qnum | Sepcify Tx queue number (4~1)for each port
@comm
when qnum = 1, only queue 0 is enabled;when qnum = 2, queue 0,1 are enabled;
when qnum = 3, queue 0,1,2 are enabled; when qnum =4, all four queues are enabled;
4 queues are recomended to use full qos function, While RTL8306S  could support 2 queues
at most.
*/
int32 rtl8306_initQos(uint32 qnum) {
    uint32 port;
    uint32 prio;
    uint32 vendorID, regVal;

        
    
   rtl8306_getVendorID(&vendorID);
   /*for RTL8306S*/
   if(RTL8306S(vendorID))  {


        switch (qnum) {
        case 1:
            /*for one queue, use default value*/
            rtl8306_getAsicPhyReg(1, 20, 0, &regVal);
            regVal = (regVal & 0xFC00) | 68;
            rtl8306_setAsicPhyReg(1, 20, 0, regVal);
            break;
        case 2:
            /*for two queue it should change shareband*/
            rtl8306_getAsicPhyReg(1, 20, 0, &regVal);
            regVal = (regVal & 0xFC00) | 100;
            rtl8306_setAsicPhyReg(1, 20, 0, regVal);
            break;
        default:
            return FAILED;
        }
        
        return SUCCESS;
   }
   
   /* for RTL8306SD/RTL8306SDM */
   
    /*Set queue number*/
   if ( rtl8306_setAsicQosPortQueueNum(qnum) == FAILED)
        return FAILED;
   
    /*set set 1 flow control is for cpu port, set 0 is for other ports, default all ports use set 0*/

   if ((qnum == 4) || (qnum == 3))  {

        /*set 0 flow control*/
        rtl8306_setAsicQosQueueFlowControlThr(0, RTL8306_FCO_QLEN, RTL8306_FCON, 0, 7, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(0, RTL8306_FCO_QLEN, RTL8306_FCOFF, 0,2, TRUE);		
        rtl8306_setAsicQosQueueFlowControlThr(0, RTL8306_FCO_DSC, RTL8306_FCON, 0, 20, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(0, RTL8306_FCO_DSC, RTL8306_FCOFF,0 , 13, TRUE);	

        rtl8306_setAsicQosQueueFlowControlThr(1, RTL8306_FCO_QLEN, RTL8306_FCON, 0, 7, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(1, RTL8306_FCO_QLEN, RTL8306_FCOFF, 0,2, TRUE);		
        rtl8306_setAsicQosQueueFlowControlThr(1, RTL8306_FCO_DSC, RTL8306_FCON, 0, 20, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(1, RTL8306_FCO_DSC, RTL8306_FCOFF,0 , 13, TRUE);	

        rtl8306_setAsicQosQueueFlowControlThr(2, RTL8306_FCO_QLEN, RTL8306_FCON, 0, 7, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(2, RTL8306_FCO_QLEN, RTL8306_FCOFF,0, 2, TRUE);		
        rtl8306_setAsicQosQueueFlowControlThr(2, RTL8306_FCO_DSC, RTL8306_FCON, 0, 20, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(2, RTL8306_FCO_DSC, RTL8306_FCOFF,0 , 13, TRUE);	

        rtl8306_setAsicQosQueueFlowControlThr(3, RTL8306_FCO_QLEN, RTL8306_FCON, 0, 7, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(3, RTL8306_FCO_QLEN, RTL8306_FCOFF,0, 2, TRUE);		
        rtl8306_setAsicQosQueueFlowControlThr(3, RTL8306_FCO_DSC, RTL8306_FCON, 0, 20, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(3, RTL8306_FCO_DSC, RTL8306_FCOFF,0 , 13, TRUE);	

        /*set 1 flow control*/
        rtl8306_setAsicQosQueueFlowControlThr(0, RTL8306_FCO_QLEN, RTL8306_FCON, 1, 7, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(0, RTL8306_FCO_QLEN, RTL8306_FCOFF,1, 2, TRUE);		
        rtl8306_setAsicQosQueueFlowControlThr(0, RTL8306_FCO_DSC, RTL8306_FCON, 1, 40, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(0, RTL8306_FCO_DSC, RTL8306_FCOFF,1 , 13, TRUE);	

        rtl8306_setAsicQosQueueFlowControlThr(1, RTL8306_FCO_QLEN, RTL8306_FCON, 1, 7, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(1, RTL8306_FCO_QLEN, RTL8306_FCOFF, 1, 2, TRUE);		
        rtl8306_setAsicQosQueueFlowControlThr(1, RTL8306_FCO_DSC, RTL8306_FCON, 1, 40, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(1, RTL8306_FCO_DSC, RTL8306_FCOFF, 1 , 13, TRUE);	

        rtl8306_setAsicQosQueueFlowControlThr(2, RTL8306_FCO_QLEN, RTL8306_FCON, 1, 7, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(2, RTL8306_FCO_QLEN, RTL8306_FCOFF,1, 2, TRUE);		
        rtl8306_setAsicQosQueueFlowControlThr(2, RTL8306_FCO_DSC, RTL8306_FCON, 1, 63, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(2, RTL8306_FCO_DSC, RTL8306_FCOFF,1 , 13, TRUE);	

        rtl8306_setAsicQosQueueFlowControlThr(3, RTL8306_FCO_QLEN, RTL8306_FCON, 1, 7, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(3, RTL8306_FCO_QLEN, RTL8306_FCOFF,1, 2, TRUE);		
        rtl8306_setAsicQosQueueFlowControlThr(3, RTL8306_FCO_DSC, RTL8306_FCON, 1, 63, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(3, RTL8306_FCO_DSC, RTL8306_FCOFF,1 , 13, TRUE);	

        

    }

    if ( qnum == 2) {

        /*set 0 flow control*/
        rtl8306_setAsicQosQueueFlowControlThr(0, RTL8306_FCO_QLEN, RTL8306_FCON, 0, 24, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(0, RTL8306_FCO_QLEN, RTL8306_FCOFF,0, 15, TRUE);		
        rtl8306_setAsicQosQueueFlowControlThr(0, RTL8306_FCO_DSC, RTL8306_FCON, 0,  26, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(0, RTL8306_FCO_DSC, RTL8306_FCOFF,0 , 13, TRUE);	

        rtl8306_setAsicQosQueueFlowControlThr(1, RTL8306_FCO_QLEN, RTL8306_FCON, 0, 24, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(1, RTL8306_FCO_QLEN, RTL8306_FCOFF, 0, 15, TRUE);		
        rtl8306_setAsicQosQueueFlowControlThr(1, RTL8306_FCO_DSC, RTL8306_FCON, 0, 26, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(1, RTL8306_FCO_DSC, RTL8306_FCOFF,0 , 13, TRUE);	

        rtl8306_setAsicQosQueueFlowControlThr(2, RTL8306_FCO_QLEN, RTL8306_FCON, 0, 7, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(2, RTL8306_FCO_QLEN, RTL8306_FCOFF,0, 2, TRUE);		
        rtl8306_setAsicQosQueueFlowControlThr(2, RTL8306_FCO_DSC, RTL8306_FCON, 0, 13, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(2, RTL8306_FCO_DSC, RTL8306_FCOFF,0 , 6, TRUE);	

        rtl8306_setAsicQosQueueFlowControlThr(3, RTL8306_FCO_QLEN, RTL8306_FCON, 0, 7, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(3, RTL8306_FCO_QLEN, RTL8306_FCOFF,0, 2, TRUE);		
        rtl8306_setAsicQosQueueFlowControlThr(3, RTL8306_FCO_DSC, RTL8306_FCON, 0, 13, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(3, RTL8306_FCO_DSC, RTL8306_FCOFF,0 , 6, TRUE);	

        /*set 1 flow control*/
        rtl8306_setAsicQosQueueFlowControlThr(0, RTL8306_FCO_QLEN, RTL8306_FCON, 1, 24, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(0, RTL8306_FCO_QLEN, RTL8306_FCOFF,1, 15, TRUE);		
        rtl8306_setAsicQosQueueFlowControlThr(0, RTL8306_FCO_DSC, RTL8306_FCON, 1, 26, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(0, RTL8306_FCO_DSC, RTL8306_FCOFF,1 , 13, TRUE);	

        rtl8306_setAsicQosQueueFlowControlThr(1, RTL8306_FCO_QLEN, RTL8306_FCON, 1, 24, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(1, RTL8306_FCO_QLEN, RTL8306_FCOFF, 1, 15, TRUE);		
        rtl8306_setAsicQosQueueFlowControlThr(1, RTL8306_FCO_DSC, RTL8306_FCON, 1, 26, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(1, RTL8306_FCO_DSC, RTL8306_FCOFF, 1 , 13, TRUE);	

        rtl8306_setAsicQosQueueFlowControlThr(2, RTL8306_FCO_QLEN, RTL8306_FCON, 1, 7, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(2, RTL8306_FCO_QLEN, RTL8306_FCOFF,1, 2, TRUE);		
        rtl8306_setAsicQosQueueFlowControlThr(2, RTL8306_FCO_DSC, RTL8306_FCON, 1, 13, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(2, RTL8306_FCO_DSC, RTL8306_FCOFF,1 , 6, TRUE);	

        rtl8306_setAsicQosQueueFlowControlThr(3, RTL8306_FCO_QLEN, RTL8306_FCON, 1, 7, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(3, RTL8306_FCO_QLEN, RTL8306_FCOFF,1, 2, TRUE);		
        rtl8306_setAsicQosQueueFlowControlThr(3, RTL8306_FCO_DSC, RTL8306_FCON, 1,  13, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(3, RTL8306_FCO_DSC, RTL8306_FCOFF,1 , 6, TRUE);	


    }   
    if (qnum == 1 ) {

        /*set 0 flow control*/
        rtl8306_setAsicQosQueueFlowControlThr(0, RTL8306_FCO_QLEN, RTL8306_FCON, 0, 24, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(0, RTL8306_FCO_QLEN, RTL8306_FCOFF,0, 15, TRUE);		
        rtl8306_setAsicQosQueueFlowControlThr(0, RTL8306_FCO_DSC, RTL8306_FCON, 0,  26, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(0, RTL8306_FCO_DSC, RTL8306_FCOFF,0 , 13, TRUE);	

        rtl8306_setAsicQosQueueFlowControlThr(1, RTL8306_FCO_QLEN, RTL8306_FCON, 0, 24, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(1, RTL8306_FCO_QLEN, RTL8306_FCOFF, 0, 15, TRUE);		
        rtl8306_setAsicQosQueueFlowControlThr(1, RTL8306_FCO_DSC, RTL8306_FCON, 0, 13, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(1, RTL8306_FCO_DSC, RTL8306_FCOFF,0 , 6, TRUE);	

        rtl8306_setAsicQosQueueFlowControlThr(2, RTL8306_FCO_QLEN, RTL8306_FCON, 0, 7, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(2, RTL8306_FCO_QLEN, RTL8306_FCOFF,0, 2, TRUE);		
        rtl8306_setAsicQosQueueFlowControlThr(2, RTL8306_FCO_DSC, RTL8306_FCON, 0, 13, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(2, RTL8306_FCO_DSC, RTL8306_FCOFF,0 , 6, TRUE);	

        rtl8306_setAsicQosQueueFlowControlThr(3, RTL8306_FCO_QLEN, RTL8306_FCON, 0, 7, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(3, RTL8306_FCO_QLEN, RTL8306_FCOFF,0, 2, TRUE);		
        rtl8306_setAsicQosQueueFlowControlThr(3, RTL8306_FCO_DSC, RTL8306_FCON, 0, 13, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(3, RTL8306_FCO_DSC, RTL8306_FCOFF,0 , 6, TRUE);	

        /*set 1 flow control*/
        rtl8306_setAsicQosQueueFlowControlThr(0, RTL8306_FCO_QLEN, RTL8306_FCON, 1, 24, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(0, RTL8306_FCO_QLEN, RTL8306_FCOFF,1, 15, TRUE);		
        rtl8306_setAsicQosQueueFlowControlThr(0, RTL8306_FCO_DSC, RTL8306_FCON, 1, 26, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(0, RTL8306_FCO_DSC, RTL8306_FCOFF,1 , 13, TRUE);	

        rtl8306_setAsicQosQueueFlowControlThr(1, RTL8306_FCO_QLEN, RTL8306_FCON, 1, 24, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(1, RTL8306_FCO_QLEN, RTL8306_FCOFF, 1, 15, TRUE);		
        rtl8306_setAsicQosQueueFlowControlThr(1, RTL8306_FCO_DSC, RTL8306_FCON, 1, 13, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(1, RTL8306_FCO_DSC, RTL8306_FCOFF, 1 , 6, TRUE);	

        rtl8306_setAsicQosQueueFlowControlThr(2, RTL8306_FCO_QLEN, RTL8306_FCON, 1, 7, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(2, RTL8306_FCO_QLEN, RTL8306_FCOFF,1, 2, TRUE);		
        rtl8306_setAsicQosQueueFlowControlThr(2, RTL8306_FCO_DSC, RTL8306_FCON, 1, 13, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(2, RTL8306_FCO_DSC, RTL8306_FCOFF,1 , 6, TRUE);	

        rtl8306_setAsicQosQueueFlowControlThr(3, RTL8306_FCO_QLEN, RTL8306_FCON, 1, 7, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(3, RTL8306_FCO_QLEN, RTL8306_FCOFF,1, 2, TRUE);		
        rtl8306_setAsicQosQueueFlowControlThr(3, RTL8306_FCO_DSC, RTL8306_FCON, 1,  13, TRUE);
        rtl8306_setAsicQosQueueFlowControlThr(3, RTL8306_FCO_DSC, RTL8306_FCOFF,1 , 6, TRUE);	


    }       
    for (port = 0; port < 6; port ++) {
            rtl8306_setAsicQosPortFlowControlMode(port, 0);
            rtl8306_setAsicQosPortFlowControlThr(port, 26, 13, RTL8306_PORT_TX);                            
    }

    /*set schedule parameter for qnum = 1,2 , set 1 for cpu port, set 0 for other ports*/
    if ((qnum == 4) ||(qnum == 3) ) {
        if (rtl8306_setAsicQosTxQueueStrictPriority(RTL8306_QUEUE3, 1, FALSE) == FAILED)
            return FAILED;
        if (rtl8306_setAsicQosTxQueueStrictPriority(RTL8306_QUEUE2, 1, FALSE) == FAILED)
            return FAILED;
        if (rtl8306_setAsicQosTxQueueWeight(RTL8306_QUEUE3, 16, 1) == FAILED)
            return FAILED;
        if (rtl8306_setAsicQosTxQueueWeight(RTL8306_QUEUE2, 4, 1) == FAILED)
            return FAILED;
        if (rtl8306_setAsicQosTxQueueWeight(RTL8306_QUEUE1, 2, 1) == FAILED)
            return FAILED;
        if (rtl8306_setAsicQosTxQueueWeight(RTL8306_QUEUE0, 1, 1) == FAILED)
            return FAILED; 
        if ( rtl8306_setAsicQosTxQueueLeakyBucket(RTL8306_QUEUE3, 1, 48,  1526) == FAILED)
            return FAILED;        
        if ( rtl8306_setAsicQosTxQueueLeakyBucket(RTL8306_QUEUE2, 1, 48,  1526) == FAILED)
            return FAILED;
        
        if (rtl8306_setAsicQosTxQueueStrictPriority(RTL8306_QUEUE3, 0, FALSE) == FAILED)
            return FAILED;
        if (rtl8306_setAsicQosTxQueueStrictPriority(RTL8306_QUEUE2, 0, FALSE) == FAILED)
            return FAILED;
        if (rtl8306_setAsicQosTxQueueWeight(RTL8306_QUEUE3, 1, 0) == FAILED)
            return FAILED;
        if (rtl8306_setAsicQosTxQueueWeight(RTL8306_QUEUE2, 1, 0) == FAILED)
            return FAILED;
        if (rtl8306_setAsicQosTxQueueWeight(RTL8306_QUEUE1, 1, 0) == FAILED)
            return FAILED;
        if (rtl8306_setAsicQosTxQueueWeight(RTL8306_QUEUE0, 1, 0) == FAILED)
            return FAILED;        
        if ( rtl8306_setAsicQosTxQueueLeakyBucket(RTL8306_QUEUE3, 0, 48,  1526) == FAILED)
            return FAILED;        
        if ( rtl8306_setAsicQosTxQueueLeakyBucket(RTL8306_QUEUE2, 0, 48,  1526) == FAILED)
            return FAILED;
    }        
        
    /*set schedule parameter for qnum = 1,2 ,set 1 for cpu port, set 0 for other ports*/
    if ((qnum == 1) ||(qnum == 2) ) {
        if (rtl8306_setAsicQosTxQueueStrictPriority(RTL8306_QUEUE3, 1, FALSE) == FAILED)
            return FAILED;
        if (rtl8306_setAsicQosTxQueueStrictPriority(RTL8306_QUEUE2, 1, FALSE) == FAILED)
            return FAILED;
        if (rtl8306_setAsicQosTxQueueWeight(RTL8306_QUEUE3, 1, 1) == FAILED)
            return FAILED;
        if (rtl8306_setAsicQosTxQueueWeight(RTL8306_QUEUE2, 1, 1) == FAILED)
            return FAILED;
        if (rtl8306_setAsicQosTxQueueWeight(RTL8306_QUEUE1, 16, 1) == FAILED)
            return FAILED;
        if (rtl8306_setAsicQosTxQueueWeight(RTL8306_QUEUE0, 1, 1) == FAILED)
            return FAILED; 
        if ( rtl8306_setAsicQosTxQueueLeakyBucket(RTL8306_QUEUE3, 1, 48,  1526) == FAILED)
            return FAILED;        
        if ( rtl8306_setAsicQosTxQueueLeakyBucket(RTL8306_QUEUE2, 1, 48,  1526) == FAILED)
            return FAILED;
        
        if (rtl8306_setAsicQosTxQueueStrictPriority(RTL8306_QUEUE3, 0, FALSE) == FAILED)
            return FAILED;
        if (rtl8306_setAsicQosTxQueueStrictPriority(RTL8306_QUEUE2, 0, FALSE) == FAILED)
            return FAILED;
        if (rtl8306_setAsicQosTxQueueWeight(RTL8306_QUEUE3, 1, 0) == FAILED)
            return FAILED;
        if (rtl8306_setAsicQosTxQueueWeight(RTL8306_QUEUE2, 1, 0) == FAILED)
            return FAILED;
        if (rtl8306_setAsicQosTxQueueWeight(RTL8306_QUEUE1, 1, 0) == FAILED)
            return FAILED;
        if (rtl8306_setAsicQosTxQueueWeight(RTL8306_QUEUE0, 1, 0) == FAILED)
            return FAILED;        
        if ( rtl8306_setAsicQosTxQueueLeakyBucket(RTL8306_QUEUE3, 0, 48,  1526) == FAILED)
            return FAILED;        
        if ( rtl8306_setAsicQosTxQueueLeakyBucket(RTL8306_QUEUE2, 0, 48,  1526) == FAILED)
            return FAILED;
    }        




    /*default : all ports select schedule set 0*/
      for (port = 0; port < 6; port ++) {
            rtl8306_setAsicQosPortScheduleMode(port, 0, 0);
      }
    /*set default QID mapping */
    switch (qnum) {
    case 1:
        for (prio = 0; prio < 4; prio ++) {
            rtl8306_setAsicQosPrioritytoQIDMapping(prio, 0);
        }
        break;
     case 2:
            rtl8306_setAsicQosPrioritytoQIDMapping(0, 0);
            rtl8306_setAsicQosPrioritytoQIDMapping(1, 0);
            rtl8306_setAsicQosPrioritytoQIDMapping(2, 1);
            rtl8306_setAsicQosPrioritytoQIDMapping(3, 1);
        break;
     case 3:
            rtl8306_setAsicQosPrioritytoQIDMapping(0, 1);
            rtl8306_setAsicQosPrioritytoQIDMapping(1, 0);
            rtl8306_setAsicQosPrioritytoQIDMapping(2, 1);
            rtl8306_setAsicQosPrioritytoQIDMapping(3, 2);            
        break;
     case 4:
            rtl8306_setAsicQosPrioritytoQIDMapping(0, 1);
            rtl8306_setAsicQosPrioritytoQIDMapping(1, 0);
            rtl8306_setAsicQosPrioritytoQIDMapping(2, 2);
            rtl8306_setAsicQosPrioritytoQIDMapping(3, 3);            
        break;
      default:
        return FAILED;                 
    }

    /*default disable all priority source*/
    for (port = 0; port < 6; port ++ ) {
        rtl8306_setAsicQosPriorityEnable(port, RTL8306_DSCP_PRIO, FALSE);           
        rtl8306_setAsicQosPriorityEnable(port, RTL8306_1QBP_PRIO, FALSE); 
        rtl8306_setAsicQosPriorityEnable(port, RTL8306_PBP_PRIO, FALSE);
        rtl8306_setAsicQosPriorityEnable(port, RTL8306_CPUTAG_PRIO, FALSE);
    }
    rtl8306_setAsicQosIPAddress(RTL8306_IPADD_A, 0, 0, FALSE);
    rtl8306_setAsicQosIPAddress(RTL8306_IPADD_B, 0, 0, FALSE);
    rtl8306_setAsicQosIPAddressPriority(RTL8306_PRIO3);
    /*set default priority arbitration*/
    rtl8306_setAsicQosPktPriorityAssign(RTL8306_ACL_PRIO, 1);
    rtl8306_setAsicQosPktPriorityAssign(RTL8306_DSCP_PRIO, 1);
    rtl8306_setAsicQosPktPriorityAssign(RTL8306_1QBP_PRIO, 1); 
    rtl8306_setAsicQosPktPriorityAssign(RTL8306_PBP_PRIO, 1);  
    /*disable all bandwidth control*/
    for (port = 0; port < 6; port ++) {
        rtl8306_setAsicQosPortRate(port, 0x5F6, RTL8306_PORT_RX, TRUE);
        rtl8306_setAsicQosPortRate(port, 0x5F6, RTL8306_PORT_TX, TRUE);        
        rtl8306_setAsicQosPortRate(port, 0x5F6, RTL8306_PORT_RX, FALSE);
        rtl8306_setAsicQosPortRate(port, 0x5F6, RTL8306_PORT_TX, FALSE);
    }
            
    return SUCCESS;    
}

