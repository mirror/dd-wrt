/*
* Copyright c                  Realsil Semiconductor Corporation, 2006
* All rights reserved.
* 
* Program :  multicast snooping function
* Abstract : 
* Author :qinjunjie 
* Email:qinjunjie1980@hotmail.com
*
*/
#include "rtl_multicast_types.h"

#ifndef RTL_MULTICAST_H
#define RTL_MULTICAST_H


#define DROP_PACKET 		0
#define FORWARD_PACKET 	1
#define ERROR_NO_SPACE 	-1

/* multicast configuration*/
struct rtl_multicastConfig
{
	uint32 enableSourceList;
	uint32 maxGroupNum;
	uint32 maxSourceNum;
	uint32 hashTableSize;
	
	uint32 groupMemberAgingTime;                
	uint32 lastMemberAgingTime;			
	uint32 querierPresentInterval;                   
	
	uint32 dvmrpRouterAgingTime;			
	uint32 mospfRouterAgingTime;                     
	uint32 pimRouterAgingTime;     

};

 struct rtl_mCastTimerParameters
 {

	uint32 groupMemberAgingTime;              
	uint32 lastMemberAgingTime;			   
	uint32 querierPresentInterval;                   
	
	uint32 dvmrpRouterAgingTime;				  /*DVMRP multicast router aging time*/
	uint32 mospfRouterAgingTime;                           /*MOSPF multicast router aging time*/
	uint32 pimRouterAgingTime;                          /*PIM-DM multicast router aging time*/
	
};

extern uint8 rtl_gatewayMac[4][6];
extern uint32 rtl_multicastStatus;

/******************************************************
	Function called in the system initialization 
******************************************************/

int32 rtl_initMulticastSnooping(struct rtl_multicastConfig multicastConfiguration, uint32 currentSystemTime, uint32 cpuAsRouter);
void rtl_setMulticastParameters(struct rtl_mCastTimerParameters mCastTimerParameters, uint8* gatewayMac, uint32 gatewayIpv4Addr,uint32 *gatewayIpv6Addr);
void rtl_multicastSnoopingReceive(uint8 * macFrame,  uint32 removeCpuTag, uint8 ** newMacFrame);
int32 rtl_maintainMulticastSnoopingTimerList(uint32 currentSystemTime);
int32 rtl_multicastSnoopingSend(uint8* macFrame, uint32 priorFreeBytes, uint32 posteriorFreeBytes,  uint8 ** newMacFrame );
int32 rtl_disableMulticastSnooping(void);

#endif /* RTL8305SD_MULTICAST_H */
