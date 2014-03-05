/*
* Copyright c                  Realsil Semiconductor Corporation, 2006
* All rights reserved.
* 
* Program :  IGMP snooping function
* Abstract : 
* Author :qinjunjie 
* Email:qinjunjie1980@hotmail.com
*
*/

#include "rtl_igmp_glue.h"

#ifndef RTL8306_TBLBAK
#define RTL8306_TBLBAK
#endif

/* IGMP parameters; */
 struct rtl_igmpSnoopingParameter{
 	
	uint16 groupMemberAgingTime;                  /* expired time of group membership, default: 260 seconds*/
	uint16 lastMemberAgingTime;			     /* IGMPv2 fast leave latency*/
	
	uint16 querierPresentInterval;                   
	
	uint16 dvmrpRouterAgingTime;				  /*DVMRP multicast router aging time*/
	uint16 mospfRouterAgingTime;                           /*MOSPF multicast router aging time*/
	uint16 pimDmRouterAgingTime;                          /*PIM-DM multicast router aging time*/
	
};


/******************************************************
	Function called in the system initialization 
******************************************************/

int32 rtl_initIgmpSnoopingV1( uint32 mCastDataToCpu, uint32 igmpProxyAvailable,  uint32 maxGroupNum, uint32 hashTableSize, uint32 currentSystemTime);
int32 rtl_setIgmpSnoopingV1Parameter(struct rtl_igmpSnoopingParameter igmpSnoopingParameters, uint8* gatewayMac, uint32 gatewayIp);
void rtl_igmpSnoopingV1Receive(uint8 * macFrame,  uint32 removeCpuTag, uint8 ** newMacFrame);
int32 rtl_maintainIgmpSnoopingV1TimerList(uint32 currentSystemTime);
int32 rtl_igmpSnoopingV1Send(uint8* macFrame, uint32 priorFreeBytes, uint32 posteriorFreeBytes,  uint8 ** newMacFrame );
int32 rtl_disableIgmpSnoopingV1(void);
#ifdef RTL_CPU_HW_FWD
int32 rtl_checkGroupStatus(uint32 groupAddress, uint32 *lutFlag, uint32 *aggregatorFlag);
#endif
