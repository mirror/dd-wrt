/*
* Copyright c                  Realtek Semiconductor Corporation, 2006 
* All rights reserved.
* 
* Program : RTL8306 switch high-level API for RTL8306SD/RTL8306SDM
* Abstract : 
* Author : Robin Zheng-bei Xing(robin_xing@realsil.com.cn)                
*  $Id: Rtl8306_Driver_sd.c,v 1.2 2007/08/03 03:59:52 michael Exp $
*/
/*	@doc RTL8306_DRIVER_sd_API

	@module Rtl8306_Driver_sd.c - RTL8306 Switch driver API documentation	|
	This document explains API interface for RTL8306SD/RTL8306SDM . 
	@normal 

	Copyright <cp>2006 Realtek<tm> Semiconductor Cooperation, All Rights Reserved.

 	@head3 List of Symbols |
 	Here is a list of all functions and variables in this module.

 	@index | RTL8306_DRIVER_sd_API
*/

#include "Rtl8306_types.h"
#include "Rtl8306_AsicDrv.h"
#include "Rtl8306_Driver_s.h"
#include "Rtl8306_Driver_sd.h"


/*
@func int32 | rtl8306_addLUTUnicastMacAddress | Set LUT unicast entry
@parm uint8* | macAddress | Specify the unicast Mac address(6 bytes) to be written into LUT
@parm uint32 | age | Specify age time. 
@parm uint32 | isStatic | TRUE(static entry), FALSE(dynamic entry)
@parm uint32 | isAuth |  Whether the mac address is authorized by IEEE 802.1x
@parm uint32 | port | Specify the port number to be forwarded to
@rvalue SUCCESS 
@rvalue RTL8306_LUT_FULL 
@rvalue FAILED
@comm	
	Age time has 4 value :RTL8306_LUT_AGEOUT, RTL8306_LUT_AGE100(100s), 
		RTL8306_LUT_AGE200(200s), RTL8306_LUT_AGE300(300s)
	The lut has a 4-way entry of an index. If the macAddress has existed in the lut, it will update the entry,
	or the function will find an empty entry to put it.
	When the index is full, it will find a dynamic & unauth unicast macAddress entry to replace with it. 
	If all the four entries can not be replaced, it will return a  RTL8306_LUT_FULL error.
	Notice: As the 8306s does not support 802.1x, the isAuth parameter has no effect on that chip.

*/
int32 rtl8306_addLUTUnicastMacAddress(uint8 *macAddress, uint32 age, uint32 isStatic, uint32 isAuth, uint32 port)
{
	int32 i;
	uint8 macAddr[6];
	uint32 index,entryaddr;
	uint32 isStatic1,isAuth1,vendorID,age1,port1;

	/*check for macAddress. must be unicast address*/
	if( (macAddr == NULL) || (macAddr[0] & 0x1))
		return FAILED;

	/*check port*/
	if (port > 5)
		return FAILED;

	/*check chip version. If it's 8306s, ignore the isAuth parameter,set to FALSE*/
	rtl8306_getVendorID(&vendorID);
	if(RTL8306S(vendorID))
		isAuth = FALSE;
	
	/*Get index of MAC in lookup table*/
	index = ((macAddress[4] & 0x7) << 6) |((macAddress[5] & 0xFC) >>2) ;

	/*
	 First scan four-ways, if the unicast entry has existed, only update the entry, that could 
	 prevent two same Mac in four-ways; if the mac was not written into entry before, then scan 
	 four-ways again, to Find an dynamic & unauthorized unicast entry which is auto learned, then  
	 replace it with the multicast Mac addr. scanning sequence is from entry 3 to entry 0, because priority
	   of four way is entry 3 > entry 2 > entry 1 > entry 0

	*/
	for (i = 3; i >= 0; i--) {
		entryaddr = (index << 2) | i;
		if (rtl8306_getAsicLUTUnicastEntry(macAddr, entryaddr, &age1, &isStatic1, &isAuth1, &port1) != SUCCESS) {					
			return FAILED;
		}	
		else if ((macAddress[0] == macAddr[0]) && (macAddress[1] == macAddr[1]) && 
			 (macAddress[2] == macAddr[2]) && (macAddress[3] == macAddr[3]) &&
			 (macAddress[4] == macAddr[4]) && (macAddress[5] == macAddr[5])) {			
			rtl8306_setAsicLUTUnicastEntry(macAddress, i , age, isStatic,isAuth,port);
			return SUCCESS;
		}
	}
	for (i = 3; i >= 0; i--) {
		entryaddr = (index << 2) | i ;
		if (rtl8306_getAsicLUTUnicastEntry(macAddr, entryaddr, &age1, &isStatic1, &isAuth1, &port1) != SUCCESS) {					
			return FAILED;
		}	
		else if (((macAddr[0] & 0x1) == 0) && (isStatic1 == FALSE) && (isAuth1 == FALSE))  {			
			rtl8306_setAsicLUTUnicastEntry(macAddress, i , age, isStatic,isAuth,port);
			return SUCCESS;
		}
	}

	/* four way are all full, return RTL8306_LUT_FULL*/
	return RTL8306_LUT_FULL;
	
}


/*
@func int32 | rtl8306_delLUTMacAddress | Delete the specified Mac address
@parm uint8* | macAddr | the Mac address(unicast or multicast) to be deleted
@rvalue SUCCESS 
@rvalue RTL8306_LUT_NOTEXIST
@rvalue FAILED
@comm
	Use this function to delete a Mac address. If the Mac has existed in the LUT, it will be deleted,
	or it will return RTL8306_LUT_NOTEXIST	
*/
int32 rtl8306_delLUTMacAddress(uint8 *macAddress)
{
	uint32 entry;

	/*check macAddress*/
	if(macAddress == NULL)
		return FAILED;

	/*delete mac*/
	return rtl8306_deleteMacAddress(macAddress, &entry);
}
/*
@func int32 | rtl8306_addLUTMulticastMacAddress | Add an multicast mac address
@parm uint8* | macAddress | specify macAddress 
@parm uint32 | portMask | specify port mask
@rvalue SUCCESS 
@rvalue FAILED
@rvalue RTL8306_LUT_FULL 
@comm
add an multicast entry, if the mac address has written into LUT, function return value is SUCCESS,  
if  4-way entries are all written by cpu, this mac address could not written into LUT and the function 
return value is  RTL8306_LUT_FULL, but if the Mac address has exist, the port mask will be updated. 
When function return value is RTL8306_LUT_FULL, you can delete one of them 
and rewrite the multicast address 

*/
int32 rtl8306_addLUTMulticastMacAddress(uint8 *macAddress, uint32 portMask)
{
	uint32 entry;
	/*check macAddress*/
	if(macAddress == NULL)
		return FAILED;
	return rtl8306_addMuticastMacAddress(macAddress,TRUE,portMask,&entry);
}



/*
@func int32 | rtl8306_setCPUPort | Specify Asic CPU port.
@parm uint32 | port | Specify the port.
@parm uint32 | enTag | CPU tag insert or not.
@parm rtl8306_qosQueueRatePara_t* | querate | specify cpu port queue 3 , queue 2 rate and burst size, NULL means no rate limit queue
@struct rtl8306_qosQueueRatePara_t | This structure describe cpu port queue 3, queue 2 rate and burst size
@field uint32 | q3_n64Kbps | (1 ~1526), queue 3 rate will be q3_n64Kbps*64Kbps
@field uint32 | q3_burstsize | (1~48), queue 3 burst size will be q3_burstsize*1KB
@field uint32 | q2_n64Kbps | (1 ~1526), queue 2 rate will be q3_n64Kbps*64Kbps
@field uint32 | q2_burstsize | (1~48), queue 2 burst size will be q3_burstsize*1KB
@rvalue SUCCESS 
@rvalue FAILED
@comm
If the port is specified RTL8306_NOCPUPORT, it means that no port is assigned as cpu port, 
packet sent to cpu port could be insterted CPU tag or not, that decided by enTag.
Queue 3 and queue 2 of cpu port could be limited average rate, queue  rate could be limited 
to times of 64Kbps, the maxmum rate is 64Kbps*1526 = 100Mbps,  the two queues also could
configure burst size to adjust traffic, burstsize uinit is 1KB, so the maxmum burst size is 48KB.
*/

int32 rtl8306_setCPUPort(uint32 port, uint32 enTag, rtl8306_qosQueueRatePara_t* querate) {
    uint32 phyport;
    
    if (rtl8306_setAsicCPUPort(port, enTag) == FAILED)
        return FAILED;


        for (phyport = 0; phyport < 6; phyport ++) {
            if (phyport == port) {
                rtl8306_setAsicQosPortFlowControlMode(phyport, 1);
                rtl8306_setAsicQosPortFlowControlThr(phyport, 127, 63, RTL8306_PORT_TX);                            
            } else {
                rtl8306_setAsicQosPortFlowControlMode(phyport, 0);
                rtl8306_setAsicQosPortFlowControlThr(phyport, 26, 13, RTL8306_PORT_TX);                            
            }
        }
        
     /*set schedule mode*/


      if (querate != NULL) {
            if ( rtl8306_setAsicQosTxQueueLeakyBucket(RTL8306_QUEUE3, 1, querate->q3_burstsize, querate->q3_n64Kbps) == FAILED)
                   return FAILED;        
            if ( rtl8306_setAsicQosTxQueueLeakyBucket(RTL8306_QUEUE2, 1, querate->q2_burstsize, querate->q2_n64Kbps) == FAILED)
                   return FAILED;
      }
            
        for (phyport = 0; phyport < 6; phyport ++ ) {
            if (phyport == port) {
                if (querate != NULL )
                    rtl8306_setAsicQosPortScheduleMode(phyport, 1, 0xC);
                else 
                    /*default q3, q2 no rate limit*/
                    rtl8306_setAsicQosPortScheduleMode(phyport, 1, 0);
            }
            else
                rtl8306_setAsicQosPortScheduleMode(phyport, 0, 0);

        }

    return SUCCESS;

    
}



/*
@func int32 | rtl8306_setPrioritytoQIDMapping | Set priority to Queue ID mapping
@parm uint32 | priority | priority vale (0 ~ 3)
@parm uint32 | qid | Queue id (0~3)
@rvalue SUCCESS 
@rvalue FAILED
@comm
Packets could be classified into specified queue through their priority. 
we can use this function to set pkt priority with queue id mapping.
*/
int32 rtl8306_setPrioritytoQIDMapping(uint32 priority, uint32 qid) {

    if (rtl8306_setAsicQosPrioritytoQIDMapping(priority, qid) == FAILED)
        return FAILED;

    return SUCCESS;
}

/*
@func int32 | rtl8306_getPrioritytoQIDMapping | Get pkt priority and qid mapping
@parm uint32 | priority | Packet priority
@parm uint32* | qid | queue id
@rvalue SUCCESS 
@rvalue FAILED
@comm
*/
int32 rtl8306_getPrioritytoQIDMapping(uint32 priority, uint32 *qid) {

   if (rtl8306_getAsicQosPrioritytoQIDMapping(priority, qid) == FAILED)
        return FAILED;
   return SUCCESS;
}



/*
@func int32 | rtl8306_setPrioritySourceArbit | Set priority source arbitration level
@parm rtl8306_qosPriorityArbitPara_t | priArbit
@struct rtl8306_qosPriorityArbitPara_t | The structure describe levels of 4 kinds of priority 
@field uint32 | aclbasedLevel | ACL-Based priority's level
@field uint32 | dscpbasedLevel | DSCP-Based priority's level
@field uint32 | dot1qbasedLevel | 1Q-based priority's level
@field uint32 | portbasedLevel | Port-based priority's level
@rvalue SUCCESS
@rvalue FAILED
@comm
RTL8306sd/sdm could recognize 6 types of priority source at most, and a packet properly has all of them, 
among them, there are 4 type priorities could be set priority level, they are ACL-based  priority,
DSCP-based priority, 1Q-based priority,Port-based priority, each one could be set level from 0 to 4, 
arbitration module will decide their sequece to take, the highest level priority will be adopted at first, 
then  priority type of the sencond highest level. priority with level 0 will not be recognized any more. 

*/
int32 rtl8306_setPrioritySourceArbit(rtl8306_qosPriorityArbitPara_t priArbit) {

    if (rtl8306_setAsicQosPktPriorityAssign(RTL8306_ACL_PRIO, priArbit.aclbasedLevel) == FAILED)
        return FAILED;    
    if (rtl8306_setAsicQosPktPriorityAssign(RTL8306_DSCP_PRIO, priArbit.dscpbasedLevel) == FAILED)
        return FAILED;
    if (rtl8306_setAsicQosPktPriorityAssign(RTL8306_1QBP_PRIO, priArbit.dot1qbasedLevel) == FAILED)
        return FAILED;    
    if (rtl8306_setAsicQosPktPriorityAssign(RTL8306_PBP_PRIO, priArbit.portbasedLevel) == FAILED)
        return FAILED;
    
    return SUCCESS;
}

/*
@func int32 | rtl8306_getPrioritySourceArbit | Set priority source arbitration 
@parm rtl8306_qosPriorityArbitPara_t* | priArbit
@comm
RTL8306sd/sdm could recognize 6 types of  priority source at most, and a packet properly has all of them, 
among them, there are 4 type priorities could be set priority level, they are ACL-based  priority,
DSCP-based priority, 1Q-based priority,Port-based priority, each one could be set level from 0 to 4, 
arbitration module will decide their sequece to take, the highest level priority will be adopted at first, 
then  priority type of the sencond highest level. priority with level 0 will not be recognized as long. 
*/

int32 rtl8306_getPrioritySourceArbit(rtl8306_qosPriorityArbitPara_t *priArbit) {

    if (rtl8306_getAsicQosPktPriorityAssign(RTL8306_ACL_PRIO, &priArbit->aclbasedLevel) == FAILED)
        return FAILED;

    if (rtl8306_getAsicQosPktPriorityAssign(RTL8306_DSCP_PRIO, &priArbit->dscpbasedLevel) == FAILED)
        return FAILED;

    if (rtl8306_getAsicQosPktPriorityAssign(RTL8306_1QBP_PRIO, &priArbit->dot1qbasedLevel) == FAILED)
        return FAILED;
    
    if (rtl8306_getAsicQosPktPriorityAssign(RTL8306_PBP_PRIO, &priArbit->portbasedLevel) == FAILED)
        return FAILED;
    
    return SUCCESS;
}


/*
@func int32 | rtl8306_setProritySourceEnable | Per-port enable/disable some priority type
@parm uint32 | port | Specify physical port (0~5)
@parm rtl8306_qosPriorityEnablePara_t | enPri |  per-port configured priority 
@struct rtl8306_qosPriorityEnablePara_t | enPri | This structure describe priorities enable/disable 
@field uint32 | en_dscp | DSCP-based priority enable/disable
@field uint32 | en_1qBP | 1Q-based priority enable/disable
@field uint32 | en_pbp | Port-based priority enable/disable
@field uint32 | en_cputag |cpu tag priority enable/disable
@rvalue SUCCESS
@rvalue FAILED
@comm
There are 4 types of  priority which could be per-port enable/disable, they are DSCP-based priority,
1Q-based, Port-based, cpu tag priority. When the priority of the specified port is disabled, the priority
will not recognized for packets received from that port.
*/
int32 rtl8306_setPrioritySourceEnable(uint32 port, rtl8306_qosPriorityEnablePara_t enPri ) {

    if (rtl8306_setAsicQosPriorityEnable(port, RTL8306_DSCP_PRIO, enPri.en_dscp) == FAILED)
        return FAILED;
    if (rtl8306_setAsicQosPriorityEnable(port, RTL8306_1QBP_PRIO, enPri.en_1qBP) == FAILED)
        return FAILED;
    if (rtl8306_setAsicQosPriorityEnable(port, RTL8306_PBP_PRIO, enPri.en_pbp) == FAILED)
        return FAILED;
    if (rtl8306_setAsicQosPriorityEnable(port, RTL8306_CPUTAG_PRIO, enPri.en_cputag) == FAILED)
        return FAILED;
    
    return SUCCESS;
}


/*
@func int32 | rtl8306_getPrioritySourceEnable | Get some priority type at specified port enabled or disabled
@parm uint32 | port | Specify physical port (0~5)
@parm rtl8306_qosPriorityEnablePara_t* | enPri |  per-port configured priority 
@rvalue SUCCESS
@rvalue FAILED
@comm
There are 4 types of  priority which could be per-port enable/disable, they are DSCP-based priority,
1Q-based, Port-based, cpu tag priority. When the priority of the specified port is disabled, the priority
will not recognized for packets received from that port.
*/

int32 rtl8306_getPrioritySourceEnable(uint32 port, rtl8306_qosPriorityEnablePara_t* enPri ) {

    if (rtl8306_getAsicQosPriorityEnable(port, RTL8306_DSCP_PRIO, &enPri->en_dscp) == FAILED)
        return FAILED;
    if (rtl8306_getAsicQosPriorityEnable(port, RTL8306_1QBP_PRIO, &enPri->en_1qBP) == FAILED)
        return FAILED;
    if (rtl8306_getAsicQosPriorityEnable(port, RTL8306_PBP_PRIO, &enPri->en_pbp) == FAILED)
        return FAILED;
    if (rtl8306_getAsicQosPriorityEnable(port, RTL8306_CPUTAG_PRIO, &enPri->en_cputag) == FAILED)
        return FAILED;
    return SUCCESS;
}

/*
@func int32 | rtl8306_setPrioritySourcePriority | Set priority source value
@parm uint32 | pri_type | Specify priority source type
@parm uint32 | pri_obj | Specify priority object
@parm uint32 | pri_val | Priority value:RTL8306_PRIO0~RTL8306_PRIO3
@rvalue | SUCCESS
@rvalue | FAILED
@comm
This function could set port-based priority for port n(RTL8306_PRI_PORTBASE), 
802.1Q based default priority for port n(RTL8306_PRI_1QDEFAULT),
802.1Q tag 3 bit priority to 2 bit priority mapping(RTL8306_PRI_1QTAG), 
DSCP based prioriy (RTL8306_PRI_DSCP)<nl>
for RTL8306_PRI_PORTBASE, pri_obj is port number 0 ~5;<nl>
for RTL8306_PRI_1QDEFAULT, pri_obj is also port number;<nl>
for RTL8306_PRI_1QTAG, pri_obj is RTL8306_1QTAG_PRIO0 ~ RTL8306_1QTAG_PRIO7, 
because 1Q tag priority is 3 bit value, so it should be mapped to 2 bit priority value at 
first;<nl>
for RTL8306_PRI_DSCP, pri_obj could be following DSCP priority type<nl>
	RTL8306_DSCP_EF<nl>			
	RTL8306_DSCP_AFL1<nl>		
	RTL8306_DSCP_AFM1<nl>		
	RTL8306_DSCP_AFH1<nl>		
	RTL8306_DSCP_AFL2<nl>			
	RTL8306_DSCP_AFM2<nl>			
	RTL8306_DSCP_AFH2<nl>				
	RTL8306_DSCP_AFL3<nl>					
	RTL8306_DSCP_AFM3<nl>		
	RTL8306_DSCP_AFH3<nl>		
	RTL8306_DSCP_AFL4<nl>			
	RTL8306_DSCP_AFM4<nl>				
	RTL8306_DSCP_AFH4<nl>		
	RTL8306_DSCP_NC<nl>			
	RTL8306_DSCP_BF<nl>			


*/
int32 rtl8306_setPrioritySourcePriority(uint32 pri_type, uint32 pri_obj, uint32 pri_val) {

    switch (pri_type) {
        
    case RTL8306_PRI_PORTBASE:  
        if (rtl8306_setAsicQosPortBasedPriority(pri_obj, pri_val) == FAILED)
            return FAILED;
        break;
    case RTL8306_PRI_1QDEFAULT:
        if (rtl8306_setAsicQos1QBasedPriority(pri_obj, pri_val) == FAILED)
            return FAILED;
        break;
    case RTL8306_PRI_1QTAG:
        if (rtl8306_setAsicQos1QtagPriorityto2bitPriority(pri_obj, pri_val) == FAILED)
            return FAILED;
        break;
    case RTL8306_PRI_DSCP:
        if (rtl8306_setAsicQosDSCPBasedPriority(pri_obj, pri_val) == FAILED)
            return FAILED;
        break;
     default:
        return FAILED;

    }    
    
    return SUCCESS;
}

/*
@func int32 | rtl8306_getPrioritySourcePriority | Get priority source priority value
@parm uint32 | pri_type | Specify priority source type
@parm uint32 | pri_obj | Specify priority object
@parm uint32* | pri_val | Priority value:RTL8306_PRIO0~RTL8306_PRIO3
@rvalue | SUCCESS
@rvalue | FAILED
@comm
This function could set port-based priority for port n(RTL8306_PRI_PORTBASE), 
802.1Q based default priority for port n(RTL8306_PRI_1QDEFAULT),
802.1Q tag 3 bit priority to 2 bit priority mapping(RTL8306_PRI_1QTAG), 
and DSCP based prioriy(RTL8306_PRI_DSCP).<nl>
for RTL8306_PRI_PORTBASE, pri_obj is port number 0 ~5;<nl>
for RTL8306_PRI_1QDEFAULT, pri_obj is also port number;<nl>
for RTL8306_PRI_1QTAG, pri_obj is RTL8306_1QTAG_PRIO0 ~ RTL8306_1QTAG_PRIO7, 
because 1Q tag priority is 3 bit value, so it should be mapped to 2 bit priority value at 
first;<nl>
for RTL8306_PRI_DSCP, pri_obj could be 16 kinds of DSCP priority type<nl>
	RTL8306_DSCP_EF<nl>			
	RTL8306_DSCP_AFL1<nl>		
	RTL8306_DSCP_AFM1<nl>		
	RTL8306_DSCP_AFH1<nl>		
	RTL8306_DSCP_AFL2<nl>			
	RTL8306_DSCP_AFM2<nl>			
	RTL8306_DSCP_AFH2<nl>				
	RTL8306_DSCP_AFL3<nl>					
	RTL8306_DSCP_AFM3<nl>		
	RTL8306_DSCP_AFH3<nl>		
	RTL8306_DSCP_AFL4<nl>			
	RTL8306_DSCP_AFM4<nl>				
	RTL8306_DSCP_AFH4<nl>		
	RTL8306_DSCP_NC<nl>			
	RTL8306_DSCP_BF<nl>			
*/


int32 rtl8306_getPrioritySourcePriority(uint32 pri_type, uint32 pri_obj, uint32 *pri_val) {

    switch (pri_type) {
        
    case RTL8306_PRI_PORTBASE:  
        if (rtl8306_getAsicQosPortBasedPriority(pri_obj, pri_val) == FAILED)
            return FAILED;
        break;
    case RTL8306_PRI_1QDEFAULT:
        if (rtl8306_getAsicQos1QBasedPriority(pri_obj, pri_val) == FAILED)
            return FAILED;
        break;
    case RTL8306_PRI_1QTAG:
        if (rtl8306_getAsicQos1QtagPriorityto2bitPriority(pri_obj, pri_val) == FAILED)
            return FAILED;
        break;
    case RTL8306_PRI_DSCP:
        if (rtl8306_getAsicQosDSCPBasedPriority(pri_obj, pri_val) == FAILED)
            return FAILED;
        break;
     default:
        return FAILED;

    }    
    
    return SUCCESS;

} 



/*
@func int32 | rtl8306_setIPAddressPriority | Set IP address priority
@parm rtl8306_qosIPadrressPara_t | ippri | ip address priority parameter
@struct rtl8306_qosIPadrressPara_t | This structure describes ip address priority parameter
@field uint32 | ip1 | The first IP
@field uint32 | ip1mask | IP mask of the first IP
@field uint32 | ip1 | The second IP
@field uint32 | ip1mask | IP mask of the second IP
@field uint32 | prio | IP address priority value:RTL8306_PRIO0~RTL8306_PRIO3
@rvalue SUCCESS
@rvalue FAILED
@comm
you can specify total two ip subnets for IP address priority, 
if ip address and ipmask of packet matches any one of it, 
the packet IP address priority will be assigned prio value.
if you do not use ip address priortiy, please set them all zero. 
*/

int32  rtl8306_setIPAddressPriority(rtl8306_qosIPadrressPara_t ippri) {

   if (ippri.ip1) {
        if (rtl8306_setAsicQosIPAddress(RTL8306_IPADD_A, ippri.ip1, ippri.ip1mask, TRUE) == FAILED)
            return FAILED;
   } else {
        if (rtl8306_setAsicQosIPAddress(RTL8306_IPADD_A, ippri.ip1, ippri.ip1mask, FALSE) == FAILED)
            return FAILED;
   }

   if (ippri.ip2) {
        if (rtl8306_setAsicQosIPAddress(RTL8306_IPADD_B, ippri.ip2, ippri.ip2mask, TRUE) == FAILED)
            return FAILED;
   } else {
        if (rtl8306_setAsicQosIPAddress(RTL8306_IPADD_B, ippri.ip2, ippri.ip2mask, FALSE) == FAILED)
            return FAILED;
   }

  if (rtl8306_setAsicQosIPAddressPriority(ippri.prio) == FAILED)
    return FAILED;
      
    return SUCCESS;    
}

/*
@func int32 | rtl8306_setPortRate | Set port bandwidth control
@parm uint32 | port | Specify port number (0~5)
@parm uint32 | isOutPut | TRUE means output rate control, FALSE means input rate control
@parm uint32 | n64Kbps | (0~1526), port rate will be n64Kbps*64Kbps
@parm uint32 | enabled | enable bandwidth control
@rvalue SUCCESS 
@rvalue FAILED
@comm
For each port, both input and output rate could be limitted to a specified speed ,
rate control unit is 64Kbps. Output rate control  could be enabled/disabled 
per port, but input rate enable/disable is for all port.
*/

int32 rtl8306_setPortRate(uint32 port, uint32 isOutPut, uint32 n64Kbps,  uint32 enabled) {

        if (rtl8306_setAsicQosPortRate(port, n64Kbps, isOutPut, enabled) == FAILED)
            return FAILED;
      
    return SUCCESS;
}


/*
@func int32 |rtl8306_getPortRate | Get specified port rate control configuration
@parm uint32* | port | port number 
@parm uint32 | isOutput | Output or input bandwidth control
@parm uint32* | n64Kbps | the rate
@parm uint32* | enabled | enabled or disabled bandwidth control
@rvalue SUCCESS 
@rvalue FAILED
@comm
*/

int32 rtl8306_getPortRate(uint32 port, uint32 isOutPut, uint32 *n64Kbps, uint32 *enabled) {


    if (rtl8306_getAsicQosPortRate(port, n64Kbps, isOutPut, enabled) == FAILED)
        return FAILED;                
    return SUCCESS;
}



/*
@func int32 | rtl8306_getMIBInfo | Get MIB counter 
@parm uint32 | port | Specify port number (0 ~ 5)
@parm uint32 | counter | Specify counter type
@parm uint32* | value | Counter value
@rvalue SUCCESS 
@rvalue FAILED
@comm
There are five MIB counter for each port, they are:<nl>  
RTL8306_MIB_CNT1 - TX packet count<nl>
RTL8306_MIB_CNT2 - RX packet count<nl>
RTL8306_MIB_CNT3 - RX Drop packet count<nl>
RTL8306_MIB_CNT4 - RX CRC error Count<nl>
RTL8306_MIB_CNT5 - RX Fragment Count<nl>
*/
int32 rtl8306_getMIBInfo(uint32 port, uint32 counter, uint32 *value)  {

    if (rtl8306_getAsicMibCounter(port, counter, value) == FAILED)
        return FAILED;
    return SUCCESS;
}


/*
@func int32 | rtl8306_resetStartMIB | Reset/Start Mib counter
@parm uint32 | port | Specify port number ( 0 ~ 5)
@parm uint32 | operation | operation type: reset or start
@rvalue SUCCESS 
@rvalue FAILED
@comm
Mib counters of each port has two operation :<nl>
RTL8306_MIB_RESET - stop counting and clear Mib counter to 0<nl>
RTL8306_MIB_START - Start counting<nl>
*/
int32 rtl8306_resetStartMIB(uint32 port, uint32 operation) {

    if (rtl8306_setAsicMibCounterReset(port, operation) == FAILED)
        return FAILED;
    return SUCCESS;    
}



/*
@func int32 | rtl8306_setMirror | Set mirror ability
@parm rtl8306_mirrorPara_t | mir | mirror parameter
@struct rtl8306_mirrorPara_t | This structure describes mirror parameter
@field uint32 | mirport | Specify mirror port number
@field uint32 | rxport  | Specify Rx mirror port mask
@field uint32 | txport | Specify Tx mirror port mask
@field uint8 | macAddr[6] | Specify Mac address
@field uint32 | enMirMac | Enable mirror packet by DA/SA 
@rvalue SUCCESS
@rvalue FAILED
@comm
mirport could be physical port 0 ~5, 7 means that no port has 
mirror ability. rxport and txport is 6 bit value, each bit corresponds 
one port, if one bit is set 1, it means that all Rx or Tx packet of that port
will mirrored to mirror port. mirror port could also mirror packet by SA or DA, 
you could set one Mac address.
*/
int32 rtl8306_setMirror(rtl8306_mirrorPara_t mir) {

    /*Enable mirror leaky*/
    if (mir.mirport != 7)
        rtl8306_setAsicMirrorVlan(TRUE);
    if (rtl8306_setAsicMirrorPort(mir.mirport, mir.rxport, mir.txport, TRUE) == FAILED)
        return FAILED;

    if (rtl8306_setAsicMirrorMacAddress(mir.macAddr, mir.enMirMac) == FAILED)
        return FAILED;
    return SUCCESS;
    
}



/*
@func rtl8306_getMirror | Get mirror configuration
@parm rtl8306_mirrorPara_t* | mir
@rvalue SUCCESS
@rvalue FAILED
*/
int32 rtl8306_getMirror(rtl8306_mirrorPara_t *mir) {
    uint32 enFilter;

    if (rtl8306_getAsicMirrorPort(&mir->mirport, &mir->rxport, &mir->txport, &enFilter) == FAILED)
        return FAILED;
    if (rtl8306_getAsicMirrorMacAddress(mir->macAddr, &mir->enMirMac) == FAILED)
        return FAILED;

    return SUCCESS;

}

/*
@func int32 | rtl8306_setSpanningTreePortState | Set IEEE 802.1d spanning tree port state
@parm uint32 | port | Specify port number (0 ~ 5)
@parm uint32 | state | Specify port state
@rvalue SUCCESS 
@rvalue FAILED
@comm
There are 4 port state:<nl> 
	RTL8306_SPAN_DISABLE  - Disable state<nl>
	RTL8306_SPAN_BLOCK    - Blocking state<nl>   
	RTL8306_SPAN_LEARN    - Learning state<nl>
	RTL8306_SPAN_FORWARD	- Forwarding state<nl>	
*/

int32 rtl8306_setSpanningTreePortState(uint32 port, uint32 state) {

    if (rtl8306_setAsic1dPortState(port, state) == FAILED)
        return FAILED;
    return SUCCESS;
}

/*
@func int32 | rtl8306_getSpanningTreePortState | Get spanning tree port state
@parm uint32 | port | Specify port number (0 ~ 5)
@parm uint32* | state | port state
@rvalue SUCCESS 
@rvalue FAILED
@comm
*/
int32 rtl8306_getSpanningTreePortState(uint32 port, uint32 * state) {

    if (rtl8306_getAsic1dPortState(port, state) == FAILED)
        return FAILED;
    return SUCCESS;
}

/*
@func int32 | rtl8306_setStormFilter | Configure storm filter
@parm rtl8306_stormPara_t | storm | storm filter parameter
@struct rtl8306_stormPara_t | This structure describes storm filter parameter
@field  uint32 | enBroadStmfil | enable/disable broadcast storm filter
@field  uint32 | enUDAStmfil   | enable/disable unkown DA storm filter;
@rvalue SUCCESS
@rvalue FAILED
@comm
*/
int32 rtl8306_setStormFilter(rtl8306_stormPara_t storm) {

    if (rtl8306_setAsicStormFilterEnable(RTL8306_BROADCASTPKT, storm.enBroadStmfil) == FAILED)
        return FAILED;    
    if (rtl8306_setAsicStormFilterEnable(RTL8306_UDAPKT, storm.enUDAStmfil) == FAILED)
        return FAILED;

    return SUCCESS;
}


/*
@func int32 | rtl8306_setDot1xPortBased | Set IEEE802.1x port-based access control
@parm uint32 | port | Specify port number (0 ~ 5)
@parm uint32 | enabled | enable port-based access control
@parm uint32 | isAuth | Authorized or unauthorized state 
@parm uint32 | direction | set IEEE802.1x port-based control direction
@rvalue SUCCESS 
@rvalue FAILED
@comm
There are 2 IEEE802.1x port state:<nl>
	RTL8306_PORT_AUTH - authorized<nl>
	RTL8306_PORT_UNAUTH - unauthorized<nl> 
There are also 2 802.1x port-based control direction:<nl>
	RTL8306_PORT_BOTHDIR - if port-base access control is enabled, 
							forbid forwarding this port's traffic to unauthorized port <nl>
	RTL8306_PORT_INDIR - if port-base access control is enabled, permit forwarding this port's traffic to unauthorized port<nl>
*/

int32 rtl8306_setDot1xPortBased(uint32 port, uint32 enabled, uint32 isAuth, uint32 direction) {

    if (rtl8306_setAsic1xPortBased(port,  enabled,  isAuth,  direction) == FAILED)
        return FAILED;
    return SUCCESS;

}



/*
@func int32 | rtl8306_setDot1xMacBased | Set IEEE802.1x Mac-based access control
@parm uint32 | port | Specify port number (0 ~ 5)
@parm uint32 | enabled | Enable the port Mac-based access control ability
@parm uint32 | direction | IEEE802.1x mac-based access control direction
@rvalue SUCCESS 
@rvalue FAILED
@comm
There are also two mac-based control directions which are not per 
port but global configurtion:<nl> 
	RTL8306_MAC_BOTHDIR - if Mac-based access control is enabled, packet with 
	                         unauthorized DA will be dropped.<nl> 
	RTL8306_MAC_INDIR   - if Mac-based access control is enabled, packet with 
	                        unauthorized DA will pass mac-based access control igress rule.<nl>
*/

int32 rtl8306_setDot1xMacBased(uint32 port, uint32 enabled, uint32 direction) {
   if (rtl8306_setAsic1xMacBased(port, enabled, direction) == FAILED)
        return FAILED;
   return SUCCESS;

}


/*
@func int32 | rtl8306_enableInterrupt | Set asic interrupt
@parm uint32 | enInt | Enable interrupt cpu 
@parm uint32 | intmask | interrupt event  mask
@rvalue SUCCESS 
@rvalue FAILED
@comm
  enInt is global setting and  intmask  has 8 bits totally, each bit 
  represents one interrupt event, bit0 ~bit4 represent port 0 ~ port 4 
  link change, bit 5 represents port 4 MAC link change, bit 6 represents 
  port 5 link change, bit 7 represents storm filter interrupt, write 1 to the 
  bit to enable the interrupt and 0 will disable the interrupt. 
*/


int32 rtl8306_enableInterrupt(uint32 enInt, uint32 intmask) {

    if (rtl8306_setAsicInterrupt(enInt, intmask) == FAILED)
        return FAILED;
    return SUCCESS;

}


/*
@func int32 | rtl8306_getInterruptEvent | Get interrupt event
@parm uint32* | intmask | interrupt flag mask
@rvalue SUCCESS 
@rvalue FAILED
@comm
intmask  has 8 bits totally, each bit represents one interrupt event, 
bit0 ~bit4 represent port 0 ~ port 4 link change, bit 5 represents
port 4 MAC link change, bit 6 represents port 5 link change, bit 7 
represents storm filter interrupt,  1 means the interrupt happens 
and 0 means not. 
*/
int32 rtl8306_getInterruptEvent(uint32 *intmask) {

    if (rtl8306_getAsicInterruptFlag(intmask) == FAILED)
        return FAILED;
    return SUCCESS;
}


/*
@func int32 | rtl8306_setLedStatus | Set asic Led status
@parm uint32 | port | Specify port number (0 ~4)
@parm uint32 | group | Specify LED group 
@parm uint32 | status | Led status: TRUE - on, FALSE - off
@parm uint32 | enCPUCtl | Enable cpu control
@rvalue SUCCESS 
@rvalue FAILED
@comm
In all of 6 ports, only port5 has no led , the other has
4 Led to specify its status, each led belongs to 1 groups , 
total 20 Leds  are divided into 4 groups: RTL8306_LED_GROUPA, 
RTL8306_LED_GROUPB, RTL8306_LED_GROUPC, RTL8306_LED_GROUPD.
each group could be set  controlled by cpu or switch, In normal 
appliction, they are controlled by switch and display port's duplex,
speed , act information. If you want to give special application to them, 
please use cpu to control them  on or off. Be carefull that if a led
is set controlled by cpu or switch, the others of the same group also
has the property.

*/

int32 rtl8306_setLedStatus(uint32 port, uint32 group, uint32 status, uint32 enCPUCtl) {

    if (rtl8306_setAsicLedStatus(port, group, status,  enCPUCtl) == FAILED)
        return FAILED;
    return SUCCESS;

}


/*
@func int32 | rtl8306_getLedStatus | Get asic led status
@parm uint32 | port | Specify port number (0 ~ 4)
@parm uint32 | group | Specify group 
@parm uint32* | status | Led status
@parm uint32* | enCPUCtl | Whether controlled by cpu
@rvalue SUCCESS 
@rvalue FAILED
@comm
Only when led is controlled by cpu, the read back led status is same as 
led real status, else the led status could not be read. 
*/

int32 rtl8306_getLedStatus(uint32 port, uint32 group, uint32 *status, uint32 *enCPUCtl) {

    if(rtl8306_getAsicLedStatus(port, group, status, enCPUCtl) == FAILED)
        return FAILED;
    return SUCCESS;

}


