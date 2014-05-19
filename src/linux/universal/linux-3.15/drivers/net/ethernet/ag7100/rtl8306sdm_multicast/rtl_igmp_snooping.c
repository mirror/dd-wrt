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

/*	@doc RTL8306_Igmp_Snooping_API

	@module rtl_igmp_snooping.c - RTL8306 Igmp Snooping API documentation	|
	This document explains the API interface of the IGMP snooping module.
	@normal Jun-Jie Qin (qjj_qin@realsil.com.cn) <date>

	Copyright <cp>2006 Realsil<tm> Semiconductor Cooperation, All Rights Reserved.

 	@head3 List of Symbols |
 	Here is a list of all functions and variables in this module.

 	@index | RTL8306_Igmp_Snooping_API
*/

#include "rtl_igmp_snooping.h"
#include "rtl_igmp_snooping_local.h"
#include "rtl_igmp_glue.h"

#include "Rtl8306_AsicDrv.h"
#include "Rtl8306_Driver_s.h"

static uint32 rtl_sysUpSeconds;          /*the current system  time*/

/******************************************************
	                        System settings
******************************************************/
static uint8 rtl_cpuPortMask;
static uint32 rtl_mCastDataToCpu;
static uint32 rtl_delPortMaskRevLeave;
static uint16 rtl_etherType;
static uint8 rtl_gatewayMac[6];
static uint32 rtl_gatewayIp;
static struct rtl_igmpSnoopingParameter rtl_multicastParas;  /*IGMP snooping parameters */
static struct rtl_igmpSnoopingRouters rtl_multicastRouters;

#if defined RTL8306_TBLBAK && defined RTL_IGMP_SNOOPING_TEST
rtl8306_ConfigBakPara_t rtl8306_TblBak;
#endif



/******************************************************
	                       Global System Resources Declaration
******************************************************/
static uint32 rtl_totalMaxGroupCnt;    /*maximum total group entry count,  default is 100*/
static struct rtl_groupEntry* rtl_groupEntryPool=NULL;
void *rtl_memoryPool=NULL;

/*hash table definition*/
static struct rtl_groupEntry ** rtl_fwdHashTable=NULL;
static uint32 rtl_fwdHashTableSize=0;
static uint32 rtl_fwdHashMask=0;

static uint32 rtl_portNumToTimerIndex[6];
static uint8 rtl_timerIndexToPortmask[5];
static uint32 rtl_igmpStatus=DISABLE;


/*********************************************
			Internal function Declaration
  *********************************************/


/**************************
	Resource Managment
**************************/
static  struct rtl_groupEntry* rtl_initGroupEntryPool(uint32 poolSize);
static  struct rtl_groupEntry* rtl_allocateGroupEntry(void);
static  void rtl_freeGroupEntry(struct rtl_groupEntry* groupEntryPtr) ;



/**********************************Structure Maintenance*************************/
/* Group Entry Management*/
static struct rtl_groupEntry* rtl_searchGroupEntry(uint32 multicastAddr, struct rtl_groupEntry* groupListHead);
static void rtl_linkGroupEntry(struct rtl_groupEntry* entryNode , uint32 hashIndex);
static void rtl_unlinkGroupEntry(struct rtl_groupEntry* entryNode, uint32 hashIndex);
static void rtl_clearGroupEntry(struct rtl_groupEntry* groupEntryPtr);

static uint32 rtl_initTimerMappingTable(uint32  cpuPortNum);

static uint8 rtl_mapPortMaskToPortNum(uint8 pormask);
static uint8 rtl_mapPortNumToPortMask(uint8 portNum);

static uint32 rtl_mapPortMaskToTimerIndex(uint8 portMask);

static int32 rtl_mapMuticastIPToMAC(uint32 ipAddr, uint8* macAddr );
static int32 rtl_checkMCastAddrMapping(uint32 ipAddr, uint8* macAddr);

static uint16 rtl_checksum(uint8 *packetBuf, uint32 packetLen);
static int32 rtl_checkPortMask(uint8 pktPortMask);
static uint8 rtl_checkGroupTimer(struct rtl_groupEntry * groupEntry ,uint16 passedSeconds);
static void rtl_checkMulticastRouterTimer(struct rtl_igmpSnoopingRouters * multicastRouterEntry, uint16 passedSeconds);
static uint8  rtl_getMulticastRouterPortMask(void);

static  void rtl_config8306(uint32 cpuPortNum, uint32 realtekEtherType);
static void rtl_checkAggregator(uint32 ipAddr,  uint8 *portMask, uint8* lookupTableFlag);
static void rtl_setAggregator(uint32 ipAddr,  int32 flag);
static int32 rtl_set8306LookupTable(struct rtl_groupEntry *groupEntry);
static void rtl_updateAllGroupEntry(void);

/*hash table operation*/
static struct rtl_groupEntry ** rtl_initFwdHashTable(uint32 hashTableSize);


/************************************Pkt Process**********************************/
/*MAC frame analyze function*/
static void  rtl_parseMacFrame(uint8* macFrame, struct rtl_macFrameInfo* macInfo);

/***********************igmp process function****************************************/
/*multicast router querier election*/
static void rtl_snoopQuerier(uint8 packetPortMask);
/*Process Query Packet*/
static void rtl_processGeneralQuery(uint8 packetPortMask);// process general query report
static void rtl_processGroupSpecificQuery(uint8 packetPortMask, uint32 groupAddress); //process group-specific query

/*Process Report Packet*/
static  void rtl_processJoinReport(uint8 packetPortMask,uint32 groupAddress); // process join report packet 
static void rtl_processLeaveReport(uint8 packetPortMask, uint32 groupAddress );// process leave report packet 
/*******************different protocol process function**********************************/
static uint8 rtl_processIgmp(uint8* pktBuf,uint32 pktLen, uint8 pktPortMask);
static uint8 rtl_processDvmrp(uint8* pktBuf, uint32 pktLen, uint8 pktPortMask);
static void rtl_processMospf(uint8* pktBuf, uint32 pktLen,  uint8 pktPortMask);
static void rtl_processPimDm(uint8* pktBuf, uint32 pktLen, uint8 pktPortMask);


/********************************Timer Expired Callback Function*********************/
static void  rtl_groupExpired(struct rtl_groupEntry* expiriedEntry, uint32 hashIndex);


#ifdef RTL8306_TBLBAK
static uint8 rtl8306_processSpanTree(uint8 forwardMask);
#endif




/************************************************
			Implementation
  ************************************************/
  
/**************************
	Initialize
**************************/

static  struct rtl_groupEntry* rtl_initGroupEntryPool(uint32 poolSize)
{
	
	uint32 idx=0;
	struct rtl_groupEntry *poolHead=NULL;
	struct rtl_groupEntry *entryPtr=NULL;
	rtl_igmpGlueMutexLock();	/* Lock resource */
	if (poolSize == 0)
	{
		goto out;
	}

	/* Allocate memory */
	poolHead = (struct rtl_groupEntry *)rtl_igmpGlueMalloc(sizeof(struct rtl_groupEntry) * rtl_totalMaxGroupCnt);

	if (poolHead != NULL)
	{
		memset(poolHead, 0,  (poolSize  * sizeof(struct rtl_groupEntry)));
		entryPtr = poolHead;

		/* link the whole group entry pool */
		for (idx = 0 ; idx < poolSize ; idx++, entryPtr++)
		{	
			if (idx == (poolSize - 1))
			{
				entryPtr->next = NULL;
			}
			else
			{
				entryPtr->next = entryPtr + 1;
			}
			
		}
	}
	
out:
	rtl_igmpGlueMutexUnlock();	/* UnLock resource */
	return poolHead;
	
}



/**************************
	Resource Managment
**************************/

// allocate a group entry pool from the group entry pool
static  struct rtl_groupEntry* rtl_allocateGroupEntry(void)
{
	struct rtl_groupEntry *ret = NULL;

	rtl_igmpGlueMutexLock();	
		if (rtl_groupEntryPool!=NULL)
		{
			ret = rtl_groupEntryPool;
			rtl_groupEntryPool = rtl_groupEntryPool->next;
			memset(ret, 0, sizeof(struct rtl_groupEntry));
		}
		
	rtl_igmpGlueMutexUnlock();	
	
	return ret;
}

// free a group entry and link it back to the group entry pool, default is link to the pool head
static  void rtl_freeGroupEntry(struct rtl_groupEntry* groupEntryPtr) 
{
	if (!groupEntryPtr)
	{
		return;
	}
		
	rtl_igmpGlueMutexLock();	
		groupEntryPtr->next = rtl_groupEntryPool;
		rtl_groupEntryPool=groupEntryPtr;	
	rtl_igmpGlueMutexUnlock();	
}




/*********************************************
			Group list operation
 *********************************************/

/*       find a group address in a group list    */

struct rtl_groupEntry* rtl_searchGroupEntry(uint32 multicastAddr, struct rtl_groupEntry* groupListHead)
{

	struct rtl_groupEntry* groupPtr = groupListHead;
	while (groupPtr!=NULL)
	{	
		if (groupPtr->groupAddress== multicastAddr)
		{	
			return groupPtr;
		}
		else
		{
			groupPtr = groupPtr->next;
		}
	}

	return NULL;
}



/* link group Entry in the front of a group list */
static void  rtl_linkGroupEntry(struct rtl_groupEntry* entryNode ,uint32 hashIndex)
{
	rtl_igmpGlueMutexLock();//Lock resource
	if(NULL==entryNode)
	{
		return;
	}
	else
	{
		entryNode->next = rtl_fwdHashTable[hashIndex];
		rtl_fwdHashTable[hashIndex]=entryNode;
		
	}
	rtl_igmpGlueMutexUnlock();//UnLock resource

}


/* unlink a group entry from group list */
static void rtl_unlinkGroupEntry(struct rtl_groupEntry* entryNode,uint32 hashIndex)
{


	struct rtl_groupEntry* groupPtr = rtl_fwdHashTable[hashIndex];
	struct rtl_groupEntry* previousEntry=NULL;

	if(NULL==entryNode)
	{
		return;
	}
	else
	{
		rtl_igmpGlueMutexLock();  /* lock resource*/
		while (groupPtr)  /*find previous entry*/
		{
			if(groupPtr==entryNode)
			{
				break;
			}
			else
			{
				previousEntry=groupPtr;
				groupPtr = groupPtr->next;
			}
		}

		/* unlink entry node*/
		if(NULL==previousEntry)
		{
			rtl_fwdHashTable[hashIndex]=entryNode->next;
		}
		else
		{
			previousEntry->next=entryNode->next;
			entryNode->next=NULL;
		}
		
		rtl_igmpGlueMutexUnlock();//UnLock resource
	}
}


/* clear the content of group entry */
static void rtl_clearGroupEntry(struct rtl_groupEntry* groupEntryPtr)
{
	rtl_igmpGlueMutexLock();//Lock resource
	if (NULL!=groupEntryPtr)
	{
		groupEntryPtr->groupAddress=0x00000000;
		groupEntryPtr->next=NULL;
		groupEntryPtr->portTimer[0]=0;
		groupEntryPtr->portTimer[1]=0;
		groupEntryPtr->portTimer[2]=0;
		groupEntryPtr->portTimer[3]=0;
		groupEntryPtr->portTimer[4]=0;
		groupEntryPtr->portTimer[5]=0;
		groupEntryPtr->fwdPortMask=0x00;
		groupEntryPtr->lookupTableFlag=0;
		
	}
	rtl_igmpGlueMutexUnlock();//UnLock resource
}


static uint32 rtl_initTimerMappingTable(uint32  cpuPortNum)
{
	uint32 i=0;
	uint32 j=0;
	uint8 portMask=1;
	if(cpuPortNum>=6)
	{
		return FAILED;
	}
	else
	{
		/*intialize rtl_portNumToTimerIndex table according to cpuPortNum*/
		for(i=0,j=0; i<6; i++)
		{
			if(i==cpuPortNum)
			{
				rtl_portNumToTimerIndex[i]=0xff;
			}
			else
			{
				rtl_portNumToTimerIndex[i]=j;
				j++;
			}
		}

		/*intialize rtl_timerIndexToPortmask table according to cpuPortNum*/
		for(i=0,j=0,portMask=1; i<6; i++)
		{
			if(i!=cpuPortNum)
			{
				rtl_timerIndexToPortmask[j]=portMask;
				j++;
			}
			
			portMask=portMask<<1;
		}
		return SUCCESS;	
		
	}
}

static uint8 rtl_mapPortMaskToPortNum(uint8 portMask)
{	
	switch(portMask)
	{
		
		case PORT0_MASK: return 0;
				
		case PORT1_MASK: return 1;

		case PORT2_MASK: return 2;

		case PORT3_MASK: return 3;

		case PORT4_MASK: return 4;

		case PORT5_MASK: return 5;

		default:return 255;
		
	}

}


static uint8 rtl_mapPortNumToPortMask(uint8 portNum)
{	
	switch(portNum)
	{
	
		case 0: return PORT0_MASK;
				
		case 1: return PORT1_MASK;

		case 2: return PORT2_MASK;

		case 3: return PORT3_MASK;

		case 4: return PORT4_MASK;
				
		case 5: return PORT5_MASK;

		default:return NON_PORT_MASK;
		
	}

}


static uint32 rtl_mapPortMaskToTimerIndex(uint8 portMask)
{
	switch(portMask)
	{
		case PORT0_MASK: return rtl_portNumToTimerIndex[0];
		case PORT1_MASK: return rtl_portNumToTimerIndex[1];
		case PORT2_MASK: return rtl_portNumToTimerIndex[2];
		case PORT3_MASK: return rtl_portNumToTimerIndex[3];
		case PORT4_MASK: return rtl_portNumToTimerIndex[4];
		case PORT5_MASK: return rtl_portNumToTimerIndex[5];
		default: return 0xff;
	}

}


static int32 rtl_mapMuticastIPToMAC(uint32 ipAddr, uint8* macAddr )
{
	if(IS_CLASSD_ADDR(ipAddr))
	{
		*macAddr=0x01;
		*(macAddr+1)=0x00;
		*(macAddr+2)=0x5e;
		*(macAddr+3)=(ipAddr&0x007f0000)>>16;
		*(macAddr+4)=(ipAddr&0x0000ff00)>>8;
		*(macAddr+5)=ipAddr&0x000000ff;
		return SUCCESS;
	}
	else
	{
		return FAILED;
	}
	
}

static int32 rtl_checkMCastAddrMapping(uint32 ipAddr, uint8* macAddr)
{
	if(macAddr[0]!=0x01)
	{
		return FALSE;
	}

	if((macAddr[3]&0x7f)!=(uint8)((ipAddr&0x007f0000)>>16))
	{
		return FALSE;
	}
	
	if(macAddr[4]!=(uint8)((ipAddr&0x0000ff00)>>8))
	{
		return FALSE;
	}

	if(macAddr[5]!=(uint8)(ipAddr&0x000000ff))
	{
		return FALSE;
	}

	return TRUE;
}

static int32 rtl_compareMacAddr(uint8* macAddr1, uint8* macAddr2)
{
	int i;
	for(i=0; i<6; i++)
	{
		if(macAddr1[i]!=macAddr2[i])
		{
			return FALSE;
		}
	}
	return TRUE;
}

static uint16 rtl_checksum(uint8 *packetBuf, uint32 packetLen)
{
	/*note: the first bytes of  packetBuf should be two bytes aligned*/
	uint32  checksum=0;
	uint32 count=packetLen;
	uint16   *ptr= (uint16 *) (packetBuf);	
	
	 while(count>1)
	 {
		  checksum+= ntohs(*ptr);
		  ptr++;
		  count -= 2;
	 }
	 
	if(count>0)
	{
		checksum+= *(packetBuf+packetLen-1)<<8; /*the last odd byte is treated as bit 15~8 of unsigned short*/
	}

	/* Roll over carry bits */
	checksum = (checksum >> 16) + (checksum & 0xffff);
	checksum += (checksum >> 16);

	/* Return checksum */
	return ((uint16) ~ checksum);

}

static int32 rtl_checkPortMask(uint8 pktPortMask)
{
	int32 i=0;
	uint8 portMaskn=PORT0_MASK;
	uint8 count=0;
	for(i=0; i<6; i++)
	{
		if(portMaskn&pktPortMask)
		{
			count++;
		}
		portMaskn=portMaskn<<1;
	}
	
	if(count==1)
	{
		return GOOD;
	}
	else
	{
		return WRONG;
	}
}



static uint8 rtl_checkGroupTimer(struct rtl_groupEntry * groupEntry ,uint16 passedSeconds)
{
	uint8 expiredPortMask=0;
	uint32 i=0;
	for(i=0; i<5; i++)
	{
		if(groupEntry->portTimer[i]>0)
		{	
			if(groupEntry->portTimer[i]>passedSeconds)
			{	
				groupEntry->portTimer[i]-=passedSeconds;
			}
			else
			{
				groupEntry->fwdPortMask&=(~rtl_timerIndexToPortmask[i]);
				expiredPortMask|=rtl_timerIndexToPortmask[i]; /*set expired portmask*/
				groupEntry->portTimer[i]=0;  /*reset timer value */
			}	
			
		}
	}

	return expiredPortMask;
	
	
}


static void rtl_checkMulticastRouterTimer(struct rtl_igmpSnoopingRouters * multicastRouterEntry, uint16 passedSeconds)
{
	uint32 portIndex=0;
	uint8 portMaskn=0;

	portMaskn=PORT0_MASK;
	for(portIndex=0; portIndex<6; portIndex++)
	{
		if(multicastRouterEntry->querier.portTimer[portIndex]>0)
		{
			if(multicastRouterEntry->querier.portTimer[portIndex]>passedSeconds)
			{	
				multicastRouterEntry->querier.portTimer[portIndex]-=passedSeconds;
			}
			else
			{
				multicastRouterEntry->querier.portTimer[portIndex]=0; /*clear timer value and corresponding port mask*/
				multicastRouterEntry->querier.portMask&=(~portMaskn);
			}	

		}
		
		if(multicastRouterEntry->dvmrpRouter.portTimer[portIndex]>0)      /*check whether DVMRP router has expired*/
		{
			if(multicastRouterEntry->dvmrpRouter.portTimer[portIndex]>passedSeconds)
			{	
				multicastRouterEntry->dvmrpRouter.portTimer[portIndex]-=passedSeconds;
			}
			else
			{
				multicastRouterEntry->dvmrpRouter.portTimer[portIndex]=0; /*clear timer value and corresponding port mask*/
				multicastRouterEntry->dvmrpRouter.portMask&=(~portMaskn);
			}	

		}

		if(multicastRouterEntry->mospfRouter.portTimer[portIndex]>0)  /*check whether MOSPF router has expired*/
		{
			if(multicastRouterEntry->mospfRouter.portTimer[portIndex]>passedSeconds)
			{	
				multicastRouterEntry->mospfRouter.portTimer[portIndex]-=passedSeconds;
			}
			else
			{
				multicastRouterEntry->mospfRouter.portTimer[portIndex]=0; /*clear timer value and corresponding port mask*/
				multicastRouterEntry->mospfRouter.portMask&=(~portMaskn);
			}		
		}


		if(multicastRouterEntry->pimDmRouter.portTimer[portIndex]>0)    /*check whether PIM-DM router has expired*/
		{
			if(multicastRouterEntry->pimDmRouter.portTimer[portIndex]>passedSeconds)
			{	
				multicastRouterEntry->pimDmRouter.portTimer[portIndex]-=passedSeconds;
			}
			else
			{
				multicastRouterEntry->pimDmRouter.portTimer[portIndex]=0; /*clear timer value and corresponding port mask*/
				multicastRouterEntry->pimDmRouter.portMask&=(~portMaskn);
			}	
		}
		
		portMaskn=portMaskn<<1;  /*shift to next port mask*/
		
	}
	
}

static void rtl_config8306(uint32 cpuPortNum, uint32 realtekEtherType)
{
#ifndef RTL_IGMP_SNOOPING_TEST

	rtl8306_setAsicPhyReg(4, 24, RTL8306_REGPAGE3, realtekEtherType);
	rtl8306_setAsicCPUPort(cpuPortNum,TRUE) ;
	rtl8306_setAsicMulticastVlan(TRUE);
	rtl8306_setAsicStormFilterEnable(RTL8306_MULTICASTPKT, FALSE);
	rtl8306_setAsicIGMPMLDSnooping(RTL8306_IGMP, TRUE);
     
      /*disable CPU learning*/
	rtl8306_setAsicPortLearningAbility(cpuPortNum, FALSE);

	

#endif
}

static void rtl_checkAggregator(uint32 ipAddr,  uint8 *portMask, uint8 *lookupTableFlag)
{	
	uint32 hashIndex=ipAddr&rtl_fwdHashMask; 
	struct rtl_groupEntry *entryPtr=rtl_fwdHashTable[hashIndex];
	uint32 lowIpAddr=ipAddr&MULTICAST_MAC_ADDR_MASK;
	uint32 count=0;
	*portMask=0;
	*lookupTableFlag=0;
	while(entryPtr!=NULL)
	{
		if((entryPtr->groupAddress&MULTICAST_MAC_ADDR_MASK)==lowIpAddr)
		{
			*portMask|=entryPtr->fwdPortMask;
			*lookupTableFlag|=(entryPtr->lookupTableFlag&IN_LOOKUP_TABLE);
			if(entryPtr->fwdPortMask!=0)/*exclude zero port mask entry*/
			{
				count=count+1;
			}
		}
		
	
		entryPtr=entryPtr->next;
	}

     /*check whether need  change to non-aggregator status*/
	entryPtr=rtl_fwdHashTable[hashIndex];
	while(entryPtr!=NULL)
	{
		if((entryPtr->groupAddress&MULTICAST_MAC_ADDR_MASK)==lowIpAddr)
		{
			if(count>1)
			{	
				entryPtr->lookupTableFlag|=AGGREGATOR_FLAG;
			}
			else
			{
				entryPtr->lookupTableFlag&=(~AGGREGATOR_FLAG);
			}
		}
		entryPtr=entryPtr->next;
	}
	
	
	
}

static void rtl_setAggregator(uint32 ipAddr, int32 flag)
{	
	
	uint32 hashIndex=ipAddr&rtl_fwdHashMask; 
	struct rtl_groupEntry*entryPtr=rtl_fwdHashTable[hashIndex];
	uint32 lowIpAddr=ipAddr&MULTICAST_MAC_ADDR_MASK;
	while(entryPtr!=NULL)
	{
		if((entryPtr->groupAddress&MULTICAST_MAC_ADDR_MASK)==lowIpAddr)
		{
			if((flag==TRUE)&& (entryPtr->fwdPortMask!=0))
			{	
				entryPtr->lookupTableFlag|=IN_LOOKUP_TABLE;
			}
			else
			{
				entryPtr->lookupTableFlag&=(~IN_LOOKUP_TABLE);
			}
		}
		entryPtr=entryPtr->next;
	}


}




static int32 rtl_set8306LookupTable(struct rtl_groupEntry *groupEntry)
{ 
	uint8 multicastMacAddress[6];
	uint8 aggregatorPortMask=0;
	uint8 aggregatorLookupTableFlag=0;
	uint32 fwdPortMask=0;
	uint32 entryaddr=0;
	uint8 multicastRouterPortMask=rtl_getMulticastRouterPortMask();
	rtl_mapMuticastIPToMAC(groupEntry->groupAddress,multicastMacAddress);
	if((groupEntry->lookupTableFlag&AGGREGATOR_FLAG)!=0)
	{
		/*get aggreagtor count and aggregator forward port mask*/
		rtl_checkAggregator(groupEntry->groupAddress, &aggregatorPortMask,&aggregatorLookupTableFlag);
		if(rtl_mCastDataToCpu==FALSE)
		{
			fwdPortMask=(uint32)((aggregatorPortMask|multicastRouterPortMask)&(~rtl_cpuPortMask));
		}
		else
		{
			fwdPortMask=(uint32)(aggregatorPortMask|multicastRouterPortMask|rtl_cpuPortMask);
		}
		
		if(aggregatorPortMask!=0) /*need  modify aggregator multicast entry*/
		{
			if((aggregatorLookupTableFlag&IN_LOOKUP_TABLE)!=0) /*means in the lookup table*/
			{
#ifdef IGMP_DEBUG
				rtl_igmpGluePrintf("step 1\n");
				rtl_igmpGluePrintf("set multicast MacAddress %x:%x:%x:%x:%x:%x to lookup table\n",multicastMacAddress[0],multicastMacAddress[1],multicastMacAddress[2],multicastMacAddress[3],multicastMacAddress[4],multicastMacAddress[5]);
#endif				
				if(rtl8306_addMuticastMacAddress(multicastMacAddress, 1, fwdPortMask, &entryaddr)==SUCCESS) /*modify existing entry*/
				{
					rtl_setAggregator(groupEntry->groupAddress, TRUE);

				}
				else
				{
#ifdef IGMP_DEBUG
					rtl_igmpGluePrintf("hareware failure\n");
#endif
					return FAILED;
				}
			}
			else /*means not in the lookup table*/
			{
#ifdef IGMP_DEBUG
				rtl_igmpGluePrintf("step 2\n");
				rtl_igmpGluePrintf("set multicast MacAddress %x:%x:%x:%x:%x:%x to lookup table\n",multicastMacAddress[0],multicastMacAddress[1],multicastMacAddress[2],multicastMacAddress[3],multicastMacAddress[4],multicastMacAddress[5]);
#endif
				if(rtl8306_addMuticastMacAddress(multicastMacAddress, 1, fwdPortMask, &entryaddr)==SUCCESS)  /*always try to write lookup table*/
				{
					rtl_setAggregator(groupEntry->groupAddress, TRUE); /* update aggragator entries' flag*/

				}
				else   /* maybe lookup table collision, or run out of multicast entry*/
				{
					rtl_setAggregator(groupEntry->groupAddress, FALSE);
					return FAILED;
				}
			}

			
		}
		else /*means all aggregator entries expired at the same time*/
		{
			if((aggregatorLookupTableFlag&IN_LOOKUP_TABLE)!=0)
			{
#ifdef IGMP_DEBUG
				rtl_igmpGluePrintf("step 3\n");
				rtl_igmpGluePrintf("delete multicast MacAddress %x:%x:%x:%x:%x:%x from lookup table\n",multicastMacAddress[0],multicastMacAddress[1],multicastMacAddress[2],multicastMacAddress[3],multicastMacAddress[4],multicastMacAddress[5]);
#endif
				if(rtl8306_deleteMacAddress(multicastMacAddress, &entryaddr)==SUCCESS) 
				{
					rtl_setAggregator(groupEntry->groupAddress, FALSE);  /* update aggragator entries' flag*/
				}
				else
				{
#ifdef IGMP_DEBUG
						rtl_igmpGluePrintf("hareware failure\n");
#endif
						return FAILED;
				}
			}
			
		}
	

	}
	else /*here means non-aggregator entry*/
	{
		if(groupEntry->fwdPortMask!=0) /*modify look up table*/
		{	
			if(rtl_mCastDataToCpu==FALSE)
			{
				fwdPortMask=(uint32)((groupEntry->fwdPortMask|multicastRouterPortMask)&(~rtl_cpuPortMask));
			}
			else
			{
				fwdPortMask=(uint32)(groupEntry->fwdPortMask|multicastRouterPortMask|rtl_cpuPortMask);
			}
			
			/*forward port mask is the combination of member ports,multicast router ports and cpu port*/
			if((groupEntry->lookupTableFlag&IN_LOOKUP_TABLE)!=0)
			{	
#ifdef IGMP_DEBUG
				rtl_igmpGluePrintf("step 4\n");
				rtl_igmpGluePrintf("set multicast MacAddress %x:%x:%x:%x:%x:%x to lookup table\n",multicastMacAddress[0],multicastMacAddress[1],multicastMacAddress[2],multicastMacAddress[3],multicastMacAddress[4],multicastMacAddress[5]);
#endif
				if(rtl8306_addMuticastMacAddress(multicastMacAddress, 1, fwdPortMask, &entryaddr)!=SUCCESS)
				{
#ifdef IGMP_DEBUG
					rtl_igmpGluePrintf("hareware failure\n");
#endif
					return FAILED;
				}
				
			}
			else  /*means not in the look up table, alwarys try to write lookup table*/
			{
#ifdef IGMP_DEBUG
				rtl_igmpGluePrintf("step 5\n");
				rtl_igmpGluePrintf("set multicast MacAddress %x:%x:%x:%x:%x:%x to lookup table\n",multicastMacAddress[0],multicastMacAddress[1],multicastMacAddress[2],multicastMacAddress[3],multicastMacAddress[4],multicastMacAddress[5]);
#endif
				if(rtl8306_addMuticastMacAddress(multicastMacAddress, 1, fwdPortMask, &entryaddr)==SUCCESS)
				{
					groupEntry->lookupTableFlag|=IN_LOOKUP_TABLE;
				}
				else /* maybe lookup table collision, or run out of multicast entry*/
				{
					return FAILED;
				}
			}
		}
		else
		{
			if(groupEntry->lookupTableFlag==IN_LOOKUP_TABLE)
			{	
#ifdef IGMP_DEBUG
				rtl_igmpGluePrintf("step 6\n");
				rtl_igmpGluePrintf("delete multicast MacAddress %x:%x:%x:%x:%x:%x from lookup table\n",multicastMacAddress[0],multicastMacAddress[1],multicastMacAddress[2],multicastMacAddress[3],multicastMacAddress[4],multicastMacAddress[5]);
#endif
				if(rtl8306_deleteMacAddress(multicastMacAddress, &entryaddr)==SUCCESS)
				{
					groupEntry->lookupTableFlag&=(~IN_LOOKUP_TABLE);
				}
				else
				{
#ifdef IGMP_DEBUG
					rtl_igmpGluePrintf("hareware failure\n");
#endif
					return FAILED;
				}
			}
			
		}
		
	}

	
	
	return SUCCESS;
	

}
static void rtl_updateAllGroupEntry(void)
{
	uint32 i=0;
	struct rtl_groupEntry* groupEntryPtr=NULL;

	for(i=0; i<rtl_fwdHashTableSize; i++)
	{
		
		groupEntryPtr=rtl_fwdHashTable[i];
		while(groupEntryPtr!=NULL)
		{	
			if((groupEntryPtr->lookupTableFlag&IN_LOOKUP_TABLE)!=0)
			{
#ifdef IGMP_DEBUG
				rtl_igmpGluePrintf("update all group entry\n");
#endif
				rtl_set8306LookupTable(groupEntryPtr);
			}
			groupEntryPtr=groupEntryPtr->next;
		}
	}
	
}


static struct rtl_groupEntry ** rtl_initFwdHashTable(uint32 hashTableSize)
{
	uint32 i=0;
	struct rtl_groupEntry **hashTablePtr=NULL;
	rtl_igmpGlueMutexLock();	/* Lock resource */

	/* Allocate memory */
	hashTablePtr = (struct rtl_groupEntry **)rtl_igmpGlueMalloc(4 * hashTableSize);

	if (hashTablePtr != NULL)
	{
		for (i = 0 ; i < hashTableSize ; i++)
		{	
			*(hashTablePtr+i)=NULL;
		}
	
	}

	rtl_igmpGlueMutexUnlock();	/* UnLock resource */
	return hashTablePtr;

}



/**************************
	Utility
**************************/

static void  rtl_parseMacFrame(uint8* macFrame, struct rtl_macFrameInfo* macInfo) 
{
	
//MAC Frame :DA(6 bytes)+SA(6 bytes)+ CPU tag(4 bytes) + VlAN tag(Optional, 4 bytes)
//                   +Type(IPv4:0x0800, IPV6:0x86DD, PPPOE:0x8864, 2 bytes )+Data(46~1500 bytes)+CRC(4 bytes)
//CPU tag: Realtek Ethertype==0x8899(2 bytes)+protocol==0x9(4 MSB)+priority(2 bits)+reserved(4 bits)+portmask(6 LSB)
	uint8 *ptr=macFrame;
	struct igmpPkt *igmpBuf=NULL;
	/* initialize */
	macInfo->cpuTagFlag=0;
	macInfo->cpuPriority=0;
	macInfo->pktPortMask=0;
	macInfo->ipBuf=NULL;
	macInfo->ipHdrLen=0;
	macInfo->l3PktBuf=NULL;
	macInfo->l3PktLen=0;
	macInfo->l3Protocol=0;
	macInfo->macFrameLen=0;
	macInfo->vlanTagID=0xff;
	macInfo->vlanTagFlag=UNTAGGED;       //?????
	uint8 portNum=0xff;
	uint8 portIndex=0xff;


	/*check the presence of CPU tag*/
	ptr=ptr+12;
	if((*((uint16 *)ptr)==htons(rtl_etherType)) && (((*(ptr+2)&0xf0)>>4)==CPUTAGPROTOCOL))
	{
		macInfo->cpuTagFlag=1;
		macInfo->cpuPriority=(*(ptr+2)&0x0c)>>2;
		macInfo->pktPortMask=*(ptr+3)&0x3f;	
		portNum=rtl_mapPortMaskToPortNum(macInfo->pktPortMask);
		ptr=ptr+4;
	}
	else
	{
		macInfo->pktPortMask=NON_PORT_MASK;
	}

	/*check the presence of VLAN tag*/	
	#ifdef RTL8306_TBLBAK
	if(*(int16 *)(ptr)==(int16)htons(VLAN_PROTOCOL_ID))
	{
              if((macInfo->vlanTagID=(ntohs(*(int16 *)(ptr+2))&0x0fff))!=0)
	       {
	             macInfo->vlanTagFlag=VLANTAGGED;
		}
		else
		{
		      macInfo->vlanTagFlag=PRITAGGED;
		}
		ptr=ptr+4;
		
	}
	else
	{
              if(portNum!=0xff)
              {
              portIndex=rtl8306_TblBak.vlanPvidIdx[portNum];
	       macInfo->vlanTagID= rtl8306_TblBak.vlanTable[portIndex].vid;
		}
		else
		{
              macInfo->vlanTagID=0x00;
		}
	       macInfo->vlanTagFlag=UNTAGGED;
	}
	#else
	if(*(int16 *)(ptr)==(int16)htons(VLAN_PROTOCOL_ID))
	{
		ptr=ptr+4;
	}
	#endif


	/*check the presence of PPPOE header*/	
	/*ignore packet with pppoe header*/
	if(*(int16 *)(ptr)==(int16)htons(PPPOE_ETHER_TYPE))
	{
		return;
	}
	
	/*check the presence of ipv4 type*/
	
	if((*(int16 *)(ptr)==(int16)htons(PPP_IPV4_PROTOCOL)) ||(*(int16 *)(ptr)==(int16)htons(IPV4_ETHER_TYPE)))
	{
		ptr=ptr+2;
		macInfo->ipBuf=(struct ipv4Pkt*)(ptr);
		
	}
	else
	{
		return;
	}

	macInfo->ipHdrLen=(((uint32)(macInfo->ipBuf->vhl&0x0f))<<2);
	macInfo->l3PktLen=ntohs(macInfo->ipBuf->length)-macInfo->ipHdrLen;
	ptr=ptr+macInfo->ipHdrLen;
	macInfo->l3PktBuf=ptr;
	macInfo->macFrameLen=(uint16)((ptr-macFrame)+macInfo->l3PktLen);
	
	
/*distinguish different IGMP packet:
                                                    ip_header_length      destination_ip      igmp_packet_length   igmp_type   group_address         	
IGMPv1_general_query:                       >=20                   224.0.0.1                       8                    0x11                 0
IGMPv2_general_query:                       >=24                   224.0.0.1                       8                    0x11                 0                     
IGMPv2_group_specific_query:		  >=24                   224.0.0.1                       8                    0x11               !=0  
IGMPv3 _query:                                   >=24                   224.0.0.1                  >=12                  0x11         according_to_different_situation 

IGMPv1_join:                                       >=20           actual_multicast_address         8                    0x12           actual_multicast_address
IGMPv2_join:                                       >=24           actual_multicast_address         8                    0x16           actual_multicast_address
IGMPv2_leave:                                     >=24           actual_multicast_address         8                    0x17           actual_multicast_address
IGMPv3_report:                                    >=24           actual_multicast_address    >=12                0x22              actual_multicast_address*/

	/* parse IGMP type and version*/	
	if(macInfo->ipBuf->protocol==IGMP_PROTOCOL)
	{	
		igmpBuf=(struct igmpPkt *)ptr;
		/*check DVMRP*/
		if((igmpBuf->type==DVMRP_TYPE) && (macInfo->ipBuf->destinationIp==htonl(DVMRP_ADDR)) )
		{
			
			macInfo->l3Protocol=DVMRP_PROTOCOL;
		}
		else
		{	
			/*means unicast*/
			if((macFrame[0]&0x01)==0)
			{
				      if((rtl_compareMacAddr(macFrame, rtl_gatewayMac)==TRUE) &&\
					(macInfo->ipBuf->destinationIp==htonl(rtl_gatewayIp)))
				       {
						macInfo->l3Protocol=IGMP_PROTOCOL;
					}
				
			}
			else /*means multicast*/
			{	
				if(rtl_checkMCastAddrMapping(ntohl(macInfo->ipBuf->destinationIp),macFrame)==TRUE)
				{
					macInfo->l3Protocol=IGMP_PROTOCOL;
				}
			}
			
			
		}
		
	}

	if(macInfo->ipBuf->protocol==MOSPF_PROTOCOL && (macInfo->ipBuf->destinationIp==htonl(MOSPF_ADDR1)))
	{
		macInfo->l3Protocol=MOSPF_PROTOCOL;
	}

	if(macInfo->ipBuf->protocol==PIM_PROTOCOL && (macInfo->ipBuf->destinationIp==htonl(PIM_ADDR)))
	{
		macInfo->l3Protocol=PIM_PROTOCOL;
	}

	return;
	
}


static uint8  rtl_getMulticastRouterPortMask(void)
{
	return (rtl_multicastRouters.querier.portMask|rtl_multicastRouters.dvmrpRouter.portMask|rtl_multicastRouters.mospfRouter.portMask|rtl_multicastRouters.pimDmRouter.portMask);
	
}

/***************************   Process IGMP packet    ****************************/
static void rtl_snoopQuerier(uint8 packetPortMask)
{
	uint8 oldPortMask=rtl_getMulticastRouterPortMask();
	uint8 newPortMask=0;
	uint32 portNum=rtl_mapPortMaskToPortNum(packetPortMask);

	rtl_multicastRouters.querier.portMask|=packetPortMask;/*update querier port mask*/
	rtl_multicastRouters.querier.portTimer[portNum]=rtl_multicastParas.querierPresentInterval;/*update timer value*/
	newPortMask=rtl_getMulticastRouterPortMask();
	if(oldPortMask!=newPortMask)
	{
#ifdef IGMP_DEBUG
	rtl_igmpGluePrintf("querier change, update all multicast entry\n");
#endif
		rtl_updateAllGroupEntry();
	}
	
}


static void rtl_processGeneralQuery(uint8 packetPortMask)
{
	/*querier timer update*/
	rtl_snoopQuerier(packetPortMask);
	
}

static void rtl_processGroupSpecificQuery(uint8 packetPortMask, uint32 groupAddress )
{	

	struct rtl_groupEntry* groupEntry=NULL;
	uint32 timerIndex=0;
	uint32 hashIndex=0; 
	
	/*querier timer update*/
	rtl_snoopQuerier(packetPortMask);

	/*update group member timer*/
	hashIndex=groupAddress&rtl_fwdHashMask;
	if(rtl_fwdHashTable[hashIndex]!=NULL)
	{
		groupEntry=rtl_searchGroupEntry(groupAddress, rtl_fwdHashTable[hashIndex]);
		if(groupEntry!=NULL)
		{
			for(timerIndex=0; timerIndex<5; timerIndex++)
			{
				if((groupEntry->portTimer[timerIndex]!=0) && (groupEntry->portTimer[timerIndex]>rtl_multicastParas.lastMemberAgingTime))
				{
#ifdef IGMP_DEBUG
		rtl_igmpGluePrintf("here to low down group timer\n");
#endif

					groupEntry->portTimer[timerIndex]=rtl_multicastParas.lastMemberAgingTime;
				}			

			}

		}
	}
	

}


/*                                           Process Report Packet                                                       */

/*Process IGMPv1/v2  join report*/
static  void  rtl_processJoinReport(uint8 packetPortMask,uint32 groupAddress)
{	
	uint32 hashIndex=0; 
	struct rtl_groupEntry* groupEntry=NULL;
	struct rtl_groupEntry* newGroupEntry=NULL;
	uint8 oldPortMask=0;
	uint8 newPortMask=0;
	uint8 aggregatorPortMask=0;
	uint8 aggregatorLookupTableFlag=0;
	uint32 timerIndex=rtl_mapPortMaskToTimerIndex(packetPortMask);
#ifdef IGMP_DEBUG
		rtl_igmpGluePrintf("here to process join packet\n");
#endif
#ifdef IGMP_DEBUG
		rtl_igmpGluePrintf("the join group address is %d.%d.%d.%d\n",((groupAddress&0xff000000)>>24),((groupAddress&0x00ff0000)>>16),((groupAddress&0x0000ff00)>>8),(groupAddress&0x000000ff));
#endif
	if(timerIndex>=5)
	{
#ifdef IGMP_DEBUG
		rtl_igmpGluePrintf("wrong port mask!\n");
#endif
		return;
	}
	


	/*update group member timer*/
	hashIndex=groupAddress&rtl_fwdHashMask;
	if(rtl_fwdHashTable[hashIndex]==NULL)   /*means new group address, create new hash entry*/
	{
		newGroupEntry=rtl_allocateGroupEntry();
		if(newGroupEntry==NULL)
		{
#ifdef IGMP_DEBUG
			rtl_igmpGluePrintf("run out of group entry!\n");
#endif
			return;
		}
	
	}
	else
	{
		groupEntry=rtl_searchGroupEntry(groupAddress, rtl_fwdHashTable[hashIndex]);
		if(groupEntry==NULL)   /*means new group address, create new group entry*/
		{
			newGroupEntry=rtl_allocateGroupEntry();
			if(newGroupEntry==NULL)
			{
#ifdef IGMP_DEBUG
				rtl_igmpGluePrintf("run out of group entry!\n");
#endif
				return;
			}
			
		}
		else
		{   
			oldPortMask=groupEntry->fwdPortMask;
			groupEntry->fwdPortMask|=packetPortMask; /*update port mask, maybe new port mask still keep the same as old port mask*/
			groupEntry->portTimer[timerIndex]=rtl_multicastParas.groupMemberAgingTime;/*update timer*/
			newPortMask=groupEntry->fwdPortMask;
			
		}

	}

	if(newGroupEntry!=NULL)  
	{	
		/*set new multicast entry*/
		newGroupEntry->fwdPortMask=packetPortMask;
		newGroupEntry->groupAddress=groupAddress;
		newGroupEntry->portTimer[timerIndex]=rtl_multicastParas.groupMemberAgingTime;
		newGroupEntry->lookupTableFlag=0;
		
		/*must first link into hash table, then call the function of set rtl8306sdm, because of aggregator checking*/
		rtl_linkGroupEntry(newGroupEntry, hashIndex);
		rtl_checkAggregator(groupAddress,&aggregatorPortMask,&aggregatorLookupTableFlag);
#ifdef IGMP_DEBUG
		rtl_igmpGluePrintf("here to add new entry to lookup table\n");
#endif
		rtl_set8306LookupTable(newGroupEntry);  /*always try to add new group entry into lookup table*/

#ifdef RTL_CPU_HW_FWD
		if(((newGroupEntry->lookupTableFlag & IN_LOOKUP_TABLE)==0)||((newGroupEntry->lookupTableFlag & AGGREGATOR_FLAG)!=0))
		{
			uint32 hashIndex=newGroupEntry->groupAddress&rtl_fwdHashMask; 
			struct rtl_groupEntry *entryPtr=rtl_fwdHashTable[hashIndex];
			while(entryPtr!=NULL)
			{
				if(((entryPtr->lookupTableFlag & IN_LOOKUP_TABLE)==0)||((entryPtr->lookupTableFlag & AGGREGATOR_FLAG)!=0))
				{
					rtl_igmpDisCpuHwFwd(entryPtr->groupAddress);
				}
				entryPtr=entryPtr->next;
			}
		}
#endif

	}

	/*always try to add new portmask and rewrite those that were not in lookup table*/
	if((groupEntry!=NULL) && ((oldPortMask!=newPortMask) ||((groupEntry->lookupTableFlag&IN_LOOKUP_TABLE)==0)))
	{
#ifdef IGMP_DEBUG
		rtl_igmpGluePrintf("here to set new port mask\n");
#endif
		rtl_set8306LookupTable(groupEntry);  
	}
	
#ifdef RTL_CPU_HW_FWD
	if(groupEntry!=NULL)
	{
		if(((groupEntry->lookupTableFlag & IN_LOOKUP_TABLE)==0)||((groupEntry->lookupTableFlag & AGGREGATOR_FLAG)!=0))
		{
			uint32 hashIndex=groupEntry->groupAddress&rtl_fwdHashMask; 
			struct rtl_groupEntry *entryPtr=rtl_fwdHashTable[hashIndex];
			while(entryPtr!=NULL)
			{
				if(((entryPtr->lookupTableFlag & IN_LOOKUP_TABLE)==0)||((entryPtr->lookupTableFlag & AGGREGATOR_FLAG)!=0))
				{
					rtl_igmpDisCpuHwFwd(entryPtr->groupAddress);
				}
				entryPtr=entryPtr->next;
			}

		}
	}
#endif


}

static void rtl_processLeaveReport(uint8 packetPortMask, uint32 groupAddress )
{	
#ifdef IGMP_DEBUG
		rtl_igmpGluePrintf("here to process leave report\n");
#endif
	struct rtl_groupEntry* groupEntry=NULL;
	uint32 timerIndex=0;
	uint32 hashIndex=0; 
	
	hashIndex=groupAddress&rtl_fwdHashMask;
	if(rtl_fwdHashTable[hashIndex]!=NULL)
	{
		groupEntry=rtl_searchGroupEntry(groupAddress, rtl_fwdHashTable[hashIndex]);
		if(groupEntry!=NULL)
		{
			timerIndex=rtl_mapPortMaskToTimerIndex(packetPortMask);
			if((groupEntry->portTimer[timerIndex]!=0) && (groupEntry->portTimer[timerIndex]>rtl_multicastParas.lastMemberAgingTime))
			{
				groupEntry->portTimer[timerIndex]=rtl_multicastParas.lastMemberAgingTime;
				/*if last member aging time is 0,delete the entry.*/
				if(rtl_delPortMaskRevLeave==TRUE)
				{
					groupEntry->fwdPortMask&=(~packetPortMask);
					rtl_set8306LookupTable(groupEntry);
						if(groupEntry->fwdPortMask==0) /*means all timer have been expired*/
						{
							/* delete this multicast entry*/
							rtl_unlinkGroupEntry(groupEntry, hashIndex);
							rtl_clearGroupEntry(groupEntry);
							rtl_freeGroupEntry(groupEntry);
						}

				}
					
			}			

		}
	}
	

}

static uint8 rtl_processIgmp(uint8* pktBuf, uint32 pktLen, uint8 pktPortMask)
{
	uint32 groupAddr=0;
	uint8 suppressFlag=0;
	uint16 numOfRecords=0;
	
	uint8 *groupRecordPtr=NULL;
	uint8 igmpv3RecordType=0;
	uint16 numOfSrc=0;
	
	uint8 multicastRouterPortMask=rtl_getMulticastRouterPortMask();
	uint16 i=0;
	if(rtl_checksum(pktBuf, pktLen)!=0)// checksum error
	{
		return 0x00;
	}

	switch(pktBuf[0])
	{
		case IGMP_QUERY_TYPE:
		{
		groupAddr=ntohl(((struct igmpPkt *)pktBuf)->groupAddr);
		if(groupAddr==0) /*general query*/
		{
#ifdef IGMP_DEBUG
	rtl_igmpGluePrintf("here to process igmp general query\n");
#endif
			rtl_processGeneralQuery( pktPortMask);
		}
		else 
		{
			if(pktLen<12) /*igmpv2 group-specific query*/
			{
#ifdef IGMP_DEBUG
	rtl_igmpGluePrintf("here to process igmp group specific query\n");
#endif
				rtl_processGroupSpecificQuery(pktPortMask,groupAddr);
			}
			else /*igmpv3 group-specific query or group-source-specific query*/
			{
				suppressFlag=(((struct igmpv3Query *)pktBuf)->rsq) & 0x08;
				numOfSrc=ntohs(((struct igmpv3Query *)pktBuf)->numOfSrc);
				if((suppressFlag==0) && (numOfSrc==0)) /*group specific query*/
				{
#ifdef IGMP_DEBUG
	rtl_igmpGluePrintf("here to process igmpv3 group-specific query\n");
#endif
					rtl_processGroupSpecificQuery(pktPortMask,groupAddr);
				}
			}
		}
		return ((~(pktPortMask|rtl_cpuPortMask))&0x3f);

		}
	

		case IGMPV1_REPORT_TYPE:
		case IGMPV2_REPORT_TYPE:
		{
			groupAddr=ntohl(((struct igmpPkt *)pktBuf)->groupAddr);
			rtl_processJoinReport(pktPortMask,groupAddr);
			return (multicastRouterPortMask&(~pktPortMask)&0x3f);

		}
		

		case IGMPV2_LEAVE_TYPE:
		{
			groupAddr=ntohl(((struct igmpPkt *)pktBuf)->groupAddr);
			rtl_processLeaveReport(pktPortMask,groupAddr);
			return (multicastRouterPortMask&(~pktPortMask)&0x3f);

		}
		

		case IGMPV3_REPORT_TYPE:
		{
			numOfRecords=ntohs(((struct igmpv3Report *)pktBuf)->numOfRecords);
			groupRecordPtr=(uint8 *)(&(((struct igmpv3Report *)pktBuf)->recordList));
			for(i=0; i<numOfRecords; i++)
			{
				igmpv3RecordType=((struct groupRecord *)groupRecordPtr)->type;
				groupAddr=ntohl(((struct groupRecord *)groupRecordPtr)->groupAddr);
				numOfSrc=ntohs(((struct groupRecord *)groupRecordPtr)->numOfSrc);
				switch(igmpv3RecordType)
				{
					case MODE_IS_INCLUDE:
						if(numOfSrc!=0)
						{
							rtl_processJoinReport(pktPortMask,groupAddr);
						}
					break;
					
					case MODE_IS_EXCLUDE:
						rtl_processJoinReport(pktPortMask,groupAddr);
					break;
					
					case CHANGE_TO_INCLUDE_MODE:
						if(numOfSrc!=0)
						{
							rtl_processJoinReport(pktPortMask,groupAddr);
						}
						else
						{
							
								rtl_processLeaveReport(pktPortMask,groupAddr);
							
						}
					break;
					
					case CHANGE_TO_EXCLUDE_MODE:
						rtl_processJoinReport(pktPortMask,groupAddr);
					break;
					
					case ALLOW_NEW_SOURCES:
						if(numOfSrc!=0)
						{
							rtl_processJoinReport(pktPortMask,groupAddr);
						}
					break;
					
					case BLOCK_OLD_SOURCES:
					break;
					
					default:break;
					
				}

				/*shift pointer to another group record*/
				groupRecordPtr=groupRecordPtr+8+numOfSrc*4+(((struct groupRecord *)(groupRecordPtr))->auxLen)*4;
			}
			return (multicastRouterPortMask&(~pktPortMask)&0x3f);

				
		}	
		

		default:
				{

	return ((~(pktPortMask|rtl_cpuPortMask))&0x3f);

		}
	}
	
				
}

static uint8 rtl_processDvmrp(uint8* pktBuf, uint32 pktLen, uint8 pktPortMask)
{
	uint8 portIndex=0;
	uint8 oldPortMask=rtl_getMulticastRouterPortMask();
	uint8 newPortMask=0;
	if(rtl_checksum(pktBuf, pktLen)!=0) /*checksum error*/
	{
#ifdef IGMP_DEBUG
		rtl_igmpGluePrintf("dvmrp packet checksum error!\n");
#endif
		return FAILED;
	}
	portIndex=rtl_mapPortMaskToPortNum(pktPortMask);
	rtl_multicastRouters.dvmrpRouter.portTimer[portIndex]=rtl_multicastParas.dvmrpRouterAgingTime; /*update timer*/

	if((pktPortMask!=0) && ((rtl_multicastRouters.dvmrpRouter.portMask&pktPortMask)!=pktPortMask)) /*means new multicast router*/
	{
		rtl_multicastRouters.dvmrpRouter.portMask|=pktPortMask; /*update dvmrp port mask*/
		
		newPortMask=rtl_getMulticastRouterPortMask();
		if(oldPortMask!=newPortMask)
		{
			rtl_updateAllGroupEntry();
		}
	}
	
	return ((~(pktPortMask|rtl_cpuPortMask))&0x3f); 
}

static void rtl_processMospf(uint8* pktBuf, uint32 pktLen,  uint8 pktPortMask)
{ 
	uint8 portIndex=0;
	struct mospfHdr* mospfHeader=(struct mospfHdr*)pktBuf;
	struct mospfHello* helloPkt=(struct mospfHello*)pktBuf;
	uint8 oldPortMask=rtl_getMulticastRouterPortMask();
	uint8 newPortMask=0;
	if(rtl_checksum(pktBuf, pktLen)!=0) /*checksum error*/
	{
#ifdef IGMP_DEBUG
		rtl_igmpGluePrintf("mospf packet checksum error!\n");
#endif
		return;
	}
	/*mospf is built based on ospfv2*/
	if((mospfHeader->version==2) && (mospfHeader->type==MOSPF_HELLO_TYPE))
	{
	    if((helloPkt->options & 0x04)!=0)
	    {
		portIndex=rtl_mapPortMaskToPortNum(pktPortMask);
		rtl_multicastRouters.mospfRouter.portTimer[portIndex]=rtl_multicastParas.mospfRouterAgingTime; /*update timer*/

		if((pktPortMask!=0)&& ((rtl_multicastRouters.mospfRouter.portMask&pktPortMask)!=pktPortMask) ) /*means new multicast router*/
		{
			rtl_multicastRouters.mospfRouter.portMask|=pktPortMask;
			
			newPortMask=rtl_getMulticastRouterPortMask();
			if(oldPortMask!=newPortMask)
			{
				rtl_updateAllGroupEntry();
			}
		}
	    }
	}
}

static void rtl_processPimDm(uint8* pktBuf, uint32 pktLen, uint8 pktPortMask)
{
	uint8 portIndex=0;
	struct pimHdr *pimHeader=(struct pimHdr* )pktBuf;
	uint8 oldPortMask=rtl_getMulticastRouterPortMask();
	uint8 newPortMask=0;
	if(rtl_checksum(pktBuf, pktLen)!=0) // chechsum error
	{
#ifdef IGMP_DEBUG
		rtl_igmpGluePrintf("pim packet checksum error!\n");
#endif
		return;
	}
	if(pimHeader->verType ==PIM_HELLO_TYPE)
	{
	
		portIndex=rtl_mapPortMaskToPortNum(pktPortMask);
		rtl_multicastRouters.pimDmRouter.portTimer[portIndex]=rtl_multicastParas.pimDmRouterAgingTime; /*update timer*/

		if((pktPortMask!=0)&& ((rtl_multicastRouters.pimDmRouter.portMask&pktPortMask)!=pktPortMask) )/*means new multicast router*/
		{
			rtl_multicastRouters.pimDmRouter.portMask|=pktPortMask;
			
			newPortMask=rtl_getMulticastRouterPortMask();
			if(oldPortMask!=newPortMask)
			{
				rtl_updateAllGroupEntry();
			}
		}
	}

}


/*********************************************************************
                                         time out  callback funtions

**********************************************************************/

/*Group entry expired callback function*/
static void rtl_groupExpired(struct rtl_groupEntry* expiredEntry, uint32 hashIndex)
{
#ifdef IGMP_DEBUG
	rtl_igmpGluePrintf("group entry expired\n");
#endif
#if 0
	/*expired entry's port mask has been cleared when checked the port timers*/
	rtl_set8306LookupTable(expiredEntry);  /*set rtl8306sdmlookup table*/
	if(expiredEntry->fwdPortMask==0) /*means all timer have been expired*/
	{
		/* delete this multicast entry*/
		rtl_unlinkGroupEntry(expiredEntry, hashIndex);
		rtl_clearGroupEntry(expiredEntry);
		rtl_freeGroupEntry(expiredEntry);
	}

#endif           
	uint8   preLUTFlag=expiredEntry->lookupTableFlag;	
	uint32 hashStart=expiredEntry->groupAddress & rtl_fwdHashMask & 0xfffffffc; 
	uint32 hashEnd=hashStart+3; /*because rtl8306sdm 4-way look up table architecture*/
	struct rtl_groupEntry*entryPtr=NULL;
	uint32 i=0;
	
	/*expired entry's port mask has been cleared when checked the port timers*/
	rtl_set8306LookupTable(expiredEntry);  /*set rtl8306sdm lookup table*/
	/*select another entry to write in the same 4-way position*/
	if((preLUTFlag & AGGREGATOR_FLAG)==0)
	{

		if(((preLUTFlag & IN_LOOKUP_TABLE)!=0) && ((expiredEntry->lookupTableFlag & IN_LOOKUP_TABLE)==0))
		{
			for(i=hashStart; i<=hashEnd; i++)
			{

				entryPtr=rtl_fwdHashTable[i];
				while(entryPtr!=NULL)
				{

					if(((entryPtr->lookupTableFlag & IN_LOOKUP_TABLE)==0) && (entryPtr!=expiredEntry))
					{
						if(rtl_set8306LookupTable(entryPtr)==SUCCESS)
						{
							goto endLoop;
						}
					}
					entryPtr=entryPtr->next;
				}
			}

		}
	}

endLoop:
	if(expiredEntry->fwdPortMask==0) /*means all timer have been expired*/
	{
		/* delete this multicast entry*/
		rtl_unlinkGroupEntry(expiredEntry, hashIndex);
		rtl_clearGroupEntry(expiredEntry);
		rtl_freeGroupEntry(expiredEntry);
	}
}


#ifdef RTL8306_TBLBAK
static uint8 rtl8306_processSpanTree(uint8 forwardMask){
       uint32 i;
	uint8 enablePortMask=0;

	for(i=0;i<6;i++)
	{
             if(rtl8306_TblBak.dot1DportCtl[i]==RTL8306_SPAN_FORWARD)
             {
                   enablePortMask|=(0x1<<i);
	      }
	}
	return (forwardMask&enablePortMask);
}
#endif



/*********************************************
				External Function
  *********************************************/

//External called function by high level program

/*
@func int32	| rtl_initIgmpSnoopingV1	|   IGMP snooping initialization.
@parm uint32	| mCastDataToCpu		| TRUE to enable multicast data forwarding to cpu, FALSE to disable it.
@parm uint32	| delPortMaskRevLeave		| TRUE to delete group member port which receiving igmp leave message, FALSE to lower group member port timer 
@parm uint32	| maxGroupNum		| Specifies maximum group number to be supported in Igmp snooping module, default is 100.
@parm uint32	| hashTableSize		| Specifies hash table size, default is 32.
@parm uint32	| currentSystemTime		| Specifies the current system time for IGMP snooping timer list initialization(unit: seconds).
@rvalue SUCCESS| Igmp snooping  initialization success
@rvalue FAILED | Igmp snooping  initialization fail
@comm 
 Initialize function, config RTL8306 and allocate memory pool for driver.

Note: If <p realtekEtherType> or <p maxGroupNum> or <p hashTableSize> has been set to 0, it means not to change this value.
*/

int32 rtl_initIgmpSnoopingV1(uint32 mCastDataToCpu, uint32 delPortMaskRevLeave,  uint32 maxGroupNum, uint32 hashTableSize, uint32 currentSystemTime)
{
       uint32 cpuPortNum;
	 /*enable insert CPU tag*/
	uint32 enTag;   
	int32 i=0;
	int32 j=0;

	/* initialize IGMP timer*/
	uint32 maxHashTableSize=1024;

	if(rtl_igmpStatus==DISABLE)
	{
	 rtl_sysUpSeconds=currentSystemTime;       
	/*set rtl8306sdm*/
	/*set group number of group entry pool*/ 
	if(maxGroupNum==0)
	{
		rtl_totalMaxGroupCnt=RTL_DefaultMaxGroupEntryCnt;
	}	
	else
	{
		rtl_totalMaxGroupCnt=maxGroupNum;
	}

	/*specify the  realtekEtherType of the cpu tag*/ 
	
		rtl_etherType=REALTEKETHERTYPE ;
		

       /*specify the cpuPortMask*/
	rtl8306_getAsicCPUPort(&cpuPortNum, &enTag);
       
	
	if( cpuPortNum<6 )
	{
		
		rtl_cpuPortMask=rtl_mapPortNumToPortMask((uint8)cpuPortNum);

	}
	else
	{
		return FAILED;

	}
	
	rtl_mCastDataToCpu=mCastDataToCpu;
	rtl_initTimerMappingTable(cpuPortNum);

	 /*set igmp protocol parameter, use the default value*/
	rtl_multicastParas.groupMemberAgingTime=DEFAULT_GROUP_MEMBER_INTERVAL;
	rtl_multicastParas.lastMemberAgingTime=DEFAULT_LAST_MEMBER_QUERY_TIME;
	rtl_multicastParas.querierPresentInterval=DEFAULT_QUERIER_PRESENT_TIMEOUT;
	rtl_multicastParas.dvmrpRouterAgingTime=DEFAULT_DVMRP_AGING_TIME;
	rtl_multicastParas.mospfRouterAgingTime=DEFAULT_MOSPF_AGING_TIME;
	rtl_multicastParas.pimDmRouterAgingTime=DEFAULT_PIM_AGING_TIME;

	/*initialize multicast Routers information*/

	rtl_multicastRouters.querier.portMask=0x00;

	rtl_multicastRouters.dvmrpRouter.portMask=0x00;
	rtl_multicastRouters.pimDmRouter.portMask=0x00;
	rtl_multicastRouters.mospfRouter.portMask=0x00;
	
	for(i=0; i<6; i++)
	{
		rtl_multicastRouters.querier.portTimer[i]=0;
		rtl_multicastRouters.dvmrpRouter.portTimer[i]=0;
		rtl_multicastRouters.pimDmRouter.portTimer[i]=0;
		rtl_multicastRouters.mospfRouter.portTimer[i]=0;
	}

	
		for(j=0;j<6;j++)
		{
			rtl_gatewayMac[j]=0;
		}
		
		
		rtl_gatewayIp=0;
	
	
      /* set hash table size and hash mask*/
	  if(hashTableSize==0)
	  {
		rtl_fwdHashTableSize=32;   /*default hash table size*/
	  }
	  else
	  {
	  
		  for(i=0;i<11;i++)
		  {
			if(hashTableSize>=maxHashTableSize)
			{
				rtl_fwdHashTableSize=maxHashTableSize;
			
				break;
			}
		 	maxHashTableSize=maxHashTableSize>>1;
			
		  }
	  }

	rtl_fwdHashMask=rtl_fwdHashTableSize-1;
	
	/*initialize hash table*/
	rtl_fwdHashTable=rtl_initFwdHashTable(rtl_fwdHashTableSize);
	if(rtl_fwdHashTable==NULL)
	{
		return FAILED;
	}
	

	/*initialize group entry pool*/
	rtl_groupEntryPool=rtl_initGroupEntryPool(rtl_totalMaxGroupCnt); 
	rtl_memoryPool=(void*)rtl_groupEntryPool;
	if(rtl_groupEntryPool==NULL)
	{
			return FAILED;
	}

	rtl_delPortMaskRevLeave=delPortMaskRevLeave;
     /*at last, config switch*/
	rtl_config8306(cpuPortNum, rtl_etherType);

      /*set igmp status is enable*/
	  rtl_igmpStatus=ENABLE;
	  rtl_igmpGluePrintf("IGMP is enable\n");
         return SUCCESS;

	}
	else
	{
         rtl_igmpGluePrintf("IGMP is already initialized\n");
         return FAILED;
	}
	
		

}

/*
@func int32	| rtl_setIgmpSnoopingV1Parameter	|   Config IGMP snooping parameters.
@parm struct rtl_igmpSnoopingParameter	| igmpSnoopingParas	| Specifies IGMP snooping time parameters.
@parm uint8*	| gatewayMac		| Specifies gateway mac address
@parm uint32	| gatewayIp		| Specifies gateway ip address
@rvalue SUCCESS	|Always return SUCCESS.
@comm 
 Config or change igmp snooping parameters.
 
Note:(1)if the <p gatewayMac> or <p gatewayIp> has been set, the unicast igmp packet whose destination MAC equals to <p gatewayMac>  and destination IP equals to<p gatewayIp>,will be accepted too
	 (2)If the member of <p igmpSnoopingParas>  has been set to 0, it means not to change its value.
*/

/*
@struct rtl_igmpSnoopingParameter	|Specifies IGMP snooping time parameters.
@field uint16 | groupMemberAgingTime	| Specifies IGMP group member aging time. Default is 260(unit:seconds).
@field uint16 | lastMemberAgingTime	| Specifies IGMP last member aging time. Default is 2(unit:seconds).
@field uint16 | querierPresentInterval	| Specifies IGMP querier present interval. Default is 260(unit:seconds).
@field uint16 | dvmrpRouterAgingTime	| Specifies DVMRP router aging time. Default is 120(unit:seconds).
@field uint16 | mospfRouterAgingTime	| Specifies MOSPF router aging time. Default is 120(unit:seconds).
@field uint16 | pimDmRouterAgingTime	| Specifies PIM-DM router aging time. Default is 120(unit:seconds).
*/

int32 rtl_setIgmpSnoopingV1Parameter(struct rtl_igmpSnoopingParameter igmpSnoopingParas, uint8* gatewayMac, uint32 gatewayIp)
{
	uint8 cpuPortNum;
	uint32 entryaddr=0;
	
	if(igmpSnoopingParas.groupMemberAgingTime>0)
	{
		rtl_multicastParas.groupMemberAgingTime= igmpSnoopingParas.groupMemberAgingTime;
	}
	
	if(igmpSnoopingParas.lastMemberAgingTime>0)
	{
		rtl_multicastParas.lastMemberAgingTime= igmpSnoopingParas.lastMemberAgingTime;
	}

	if(igmpSnoopingParas.querierPresentInterval>0)
	{
		rtl_multicastParas.querierPresentInterval=igmpSnoopingParas.querierPresentInterval;
	}

	if(igmpSnoopingParas.dvmrpRouterAgingTime>0)
	{
		rtl_multicastParas.dvmrpRouterAgingTime=igmpSnoopingParas.dvmrpRouterAgingTime;
	}

	if(igmpSnoopingParas.mospfRouterAgingTime>0)
	{
		rtl_multicastParas.mospfRouterAgingTime=igmpSnoopingParas.mospfRouterAgingTime;
	}
	

	if(igmpSnoopingParas.pimDmRouterAgingTime>0)
	{
		rtl_multicastParas.pimDmRouterAgingTime=igmpSnoopingParas.pimDmRouterAgingTime;
	}


	if(gatewayMac!=NULL)
	{
			if((rtl_gatewayMac[0]|rtl_gatewayMac[1]|rtl_gatewayMac[2]|rtl_gatewayMac[3]|rtl_gatewayMac[4]|rtl_gatewayMac[5])==0)
			{
				rtl8306_deleteMacAddress(rtl_gatewayMac, &entryaddr);

			}
		
		rtl_gatewayMac[0]=gatewayMac[0];
		rtl_gatewayMac[1]=gatewayMac[1];
		rtl_gatewayMac[2]=gatewayMac[2];
		rtl_gatewayMac[3]=gatewayMac[3];
		rtl_gatewayMac[4]=gatewayMac[4];
		rtl_gatewayMac[5]=gatewayMac[5];
		rtl_igmpGluePrintf("set gateway mac : %x %x %x %x %x %x\n",rtl_gatewayMac[0],rtl_gatewayMac[1],rtl_gatewayMac[2],rtl_gatewayMac[3],rtl_gatewayMac[4],rtl_gatewayMac[5]);

		      /*set CPU mac to LUT*/
		cpuPortNum=rtl_mapPortMaskToPortNum(rtl_cpuPortMask);
		#ifndef RTL_IGMP_SNOOPING_TEST
       	if(rtl8306_addLUTUnicastMacAddress(rtl_gatewayMac, 0x02, TRUE, TRUE, cpuPortNum)==FAILED)
		{
              	   rtl_igmpGluePrintf("set CPU mac error!\n");
		}
		#endif

		
	}

			rtl_gatewayIp=gatewayIp;
	
	return SUCCESS;
	
}

/*
@func int32	| rtl_maintainIgmpSnoopingV1TimerList	|   IGMP snooping timer list maintenance function.
@parm  uint32	| currentSystemTime	|The current system time (unit: seconds).
@rvalue SUCCESS	|Always return SUCCESS.
@comm 
 This function should be called once a second to maintain IGMP timer list.
*/

int32 rtl_maintainIgmpSnoopingV1TimerList(uint32 currentSystemTime )
{
	/* maintain current time */
	 uint32 i=0;
	 struct rtl_groupEntry* groupEntryPtr=NULL;
	 struct rtl_groupEntry* nextEntry=NULL;
	 uint8 expiredPortMask=0;
	 uint8 oldPortMask=rtl_getMulticastRouterPortMask();
	 uint8 newPortMask=0;

	 uint16 passedSeconds=(uint16)(currentSystemTime-rtl_sysUpSeconds);

        if(rtl_igmpStatus==ENABLE)
        {
	/*maintain group entry list timer */
	for(i=0; i<rtl_fwdHashTableSize; i++)
	{
		  /*scan the hash table*/
		groupEntryPtr=rtl_fwdHashTable[i];
		while(groupEntryPtr)              /*traverse each group list*/
		{	
			nextEntry=groupEntryPtr->next; 
			expiredPortMask=rtl_checkGroupTimer(groupEntryPtr, passedSeconds);
			if(expiredPortMask!=0) /*inidicate this group entry  expired*/
			{
				rtl_groupExpired(groupEntryPtr, i); /*call back function of group entry expired*/
			}
			groupEntryPtr=nextEntry;/*because group entry expired callback function will clear groupEntryPtr's content */
		}
	}

	/*maintain multicast router position timer */
	rtl_checkMulticastRouterTimer(&rtl_multicastRouters, passedSeconds);
	
	newPortMask=rtl_getMulticastRouterPortMask();
	if(oldPortMask!=newPortMask)
	{
		rtl_updateAllGroupEntry();
	}

	rtl_sysUpSeconds=currentSystemTime;
}
	return SUCCESS;
}

/*
@func void	| rtl_igmpSnoopingV1Receive	| IGMP packet snooping function.
@parm uint8*	| macFrame	| Pointer of MAC frame.
@parm uint32	| removeCpuTag	| TRUE to  remove cpu tag, if available. Fasle to keep cpu tag untouched.
@parm uint8 **| newMacFrame	| if removeCpuTag has been set to true,it indicate the new new mac frame position after removing cpu tag.
@rvalue void|void
@comm 
	CPU should have the ability to receive/transmit mac frame with cpu tag attached.
*/

void rtl_igmpSnoopingV1Receive(uint8 * macFrame,  uint32 removeCpuTag, uint8 ** newMacFrame)
{

	struct rtl_macFrameInfo macFrameInfo;
	uint8 fwdPortMask=0; 	
	int32 i=0;
       uint8 portNum;
	uint8 portIndex;
	uint8 vlanEntryExist=FALSE;
	
	*newMacFrame=macFrame;
	rtl_parseMacFrame(macFrame,&macFrameInfo);

	if(rtl_igmpStatus==ENABLE)
	{
	       if(macFrameInfo.ipBuf==NULL)
	       {
		      goto out;

	       }
	

	if(rtl_checkPortMask(macFrameInfo.pktPortMask)==WRONG)
	{
#ifdef IGMP_DEBUG	
	rtl_igmpGluePrintf("error! packet port mask is wrong\n");
#endif
		goto out;
	}
	

	switch(macFrameInfo.l3Protocol)
	{

		case IGMP_PROTOCOL:
			fwdPortMask=rtl_processIgmp(macFrameInfo.l3PktBuf, macFrameInfo.l3PktLen, macFrameInfo.pktPortMask);
		break;


		case DVMRP_PROTOCOL:
			fwdPortMask=rtl_processDvmrp(macFrameInfo.l3PktBuf, macFrameInfo.l3PktLen, macFrameInfo.pktPortMask);
		break;

		case MOSPF_PROTOCOL:
			rtl_processMospf(macFrameInfo.l3PktBuf, macFrameInfo.l3PktLen, macFrameInfo.pktPortMask);
		break;
			
		case PIM_PROTOCOL:
			rtl_processPimDm(macFrameInfo.l3PktBuf, macFrameInfo.l3PktLen, macFrameInfo.pktPortMask);
		break;

		default: break;
	}


	
#ifndef RTL_IGMP_SNOOPING_TEST	
	if(fwdPortMask!=0 )
	{
	#ifdef RTL8306_TBLBAK
            fwdPortMask=rtl8306_processSpanTree(fwdPortMask);
	if(rtl8306_TblBak.vlanConfig.enVlan==TRUE)   /*vlan function is enable*/
              {
           
                    if((rtl8306_TblBak.vlanConfig.enTagAware==FALSE) ||macFrameInfo.vlanTagFlag!=VLANTAGGED)  /*enable port-base vlan, untagged pkt use port-base vlan*/
		      {
                          portNum=rtl_mapPortMaskToPortNum(macFrameInfo.pktPortMask);
			     portIndex=rtl8306_TblBak.vlanPvidIdx[portNum];
			     fwdPortMask&=rtl8306_TblBak.vlanTable[portIndex].memberPortMask;
				 
		       }
			else  /*enable 802.1q tag aware vlan only when it is a tagged pkt*/
			{
                           for(i=0;i<16;i++)
			     {
                                 if(rtl8306_TblBak.vlanTable[i].vid==macFrameInfo.vlanTagID)
                                 	{
                                        fwdPortMask&=rtl8306_TblBak.vlanTable[i].memberPortMask;
						vlanEntryExist=TRUE;

					     break;
					}
			     }

				if(vlanEntryExist==FALSE)
				{
					fwdPortMask=0;
				}

			}
              }
	#endif

		macFrame[15]=fwdPortMask;/*set forward packet mask*/
		rtl_igmpGlueNicSend(macFrame,(uint32)macFrameInfo.macFrameLen, MII_PORT_MASK);
	}
	
#else
	macFrame[15]=fwdPortMask;/*set forward packet mask*/
	return;
#endif

   }
else
   {
        #ifdef RTL_IGMP_SNOOPING_TEST 
		rtl_igmpGluePrintf("now disable the igmp modle\n");
	#endif
   }

out: 
	if((removeCpuTag==TRUE) && (macFrameInfo.cpuTagFlag!=0))
	{
		for(i=11; i>=0; i--)
		{
			macFrame[i+4]=macFrame[i];
		}
		*newMacFrame= macFrame+4;
	}

	

	
}

/*
@func int32	| rtl_igmpSnoopingV1Send	|   IGMP snooping packet forwarding function
@parm uint8*	| macFrame	| The pointer of MAC frame to be transmitted.
@parm uint32	| priorFreeBytes		| Free space ahead MAC Frame.
@parm uint32	| posteriorFreeSpace	| Free space after MAC Frame.
@parm uint8 **	| newMacFrame	| New pointer of MAC frame to be forwarded(maybe insert cpu tag or not).
@rvalue SUCCESS	| This packet can be handled by igmp snooping module
@rvalue FAILED	|Not enough space to insert cpu tag, or pppoe header present, or ip header not found
@comm 
This function is called when forward a multicast MAC frame. Since there is an aggregator issues when multicast IP maps to MAC address, besides, RTL8305SD may also run out of multicast entry. CPU has to solve these problems, and give appropriate direction when forward a multicast data.
*/

int32 rtl_igmpSnoopingV1Send(uint8* macFrame, uint32 priorFreeBytes,  uint32 posteriorFreeSpace, uint8 ** newMacFrame )
{
	/*only the multicast address, which can be found in the  forward list but not set in the look up table, will be insert cpu tag*/
	struct rtl_groupEntry * groupEntry=NULL;
	uint32 hashIndex=0;
	int32 i=0;
	uint32 pktLen=0;
	uint32 destIp=0;
	uint8 *tempPtr1=NULL;
	uint8 *tempPtr2=NULL;
	uint8 multicastRouterPortMask=rtl_getMulticastRouterPortMask();
	struct rtl_macFrameInfo macInfo;
	struct ipv4Pkt *ipBuf=NULL;
	rtl_parseMacFrame(macFrame,&macInfo);
	*newMacFrame=macFrame;

       if(rtl_igmpStatus==ENABLE)
       {
	if(macInfo.ipBuf==NULL)
	{
		return FAILED;
	}
	ipBuf=(struct ipv4Pkt *)(macInfo.ipBuf);
	destIp=ntohl(ipBuf->destinationIp);
	if(IS_CLASSD_ADDR(destIp) && (macInfo.cpuTagFlag==0))          /*check whether destination ip is a multicast address*/
	{
		hashIndex=destIp&rtl_fwdHashMask;
		groupEntry=rtl_searchGroupEntry(destIp, rtl_fwdHashTable[hashIndex]); /*search multicast list to find whether destination IP registered*/ 
		if(groupEntry!=NULL)
		{
			 if(((groupEntry->lookupTableFlag & IN_LOOKUP_TABLE) !=0)  && ((groupEntry->lookupTableFlag & AGGREGATOR_FLAG) ==0))
			 {
			 	/*means in look up table and non-aggregator entry */
				return SUCCESS;
			 }
			 
			if((priorFreeBytes<4) && (posteriorFreeSpace<4))  /*cpu tag already available*/
			{
				return FAILED;
			}
			
			if(priorFreeBytes>=4) /*move forwardly*/
			{
				tempPtr1=macFrame-4;
				for(i=0; i<12; i++)  /*move DA,SA forwardly*/
				{
					tempPtr1[i]=macFrame[i];
				}

				*newMacFrame=tempPtr1;
			}
			else  /*posteriorFreeSpace>=4bytes, move backforwardly*/
			{
				/*move until one bytes before cpu tag*/
				pktLen=ntohs(ipBuf->length);
				tempPtr1=(uint8*)(macInfo.ipBuf)+pktLen-1;
				tempPtr2=macFrame+11;						
				while(tempPtr1!=tempPtr2)
				{
					*(tempPtr1+4)=*tempPtr1;
					 tempPtr1--;
				}
				*newMacFrame=macFrame;
			}

			/*insert cpu tag, add multicast router port and clear cpu port*/
			*(*newMacFrame+12)=(uint8)((rtl_etherType&0xff00)>>8);
			*(*newMacFrame+13)=(uint8)(rtl_etherType&0x00ff);
			*(*newMacFrame+14)=CPUTAGPROTOCOL<<4;
			*(*newMacFrame+15)=(groupEntry->fwdPortMask|multicastRouterPortMask)&(~rtl_cpuPortMask);
								
		
		}
		

	}
}

	return SUCCESS;

}
/*
@func int32	| rtl_checkGroupStatus	|  To check if a group address has been set into lookup table, or an aggregator address.
@parm uint32 	| groupAddress	| Specifies the group address to be checked.
@parm  uint32 *	| lutFlag		| TRUE: if <p groupAddress> has been set into RTL8306sdm lookup table.
@parm uint32 *	| aggregatorFlag	| TRUE: if <p groupAddress> is an aggregator address.
@rvalue SUCCESS	| <p groupAddress> has been found.
@rvalue FAILED	| <p groupAddress> is not found.
@comm 
This function is enable when define the RTL_CPU_HW_FWD macro.It's used to determine whether a group address can be set into CPU acceleration hardware 
*/
#ifdef RTL_CPU_HW_FWD
int32 rtl_checkGroupStatus(uint32 groupAddress, uint32 *lutFlag, uint32 *aggregatorFlag)
{
	struct rtl_groupEntry* groupEntry=NULL;
	uint32 hashIndex=0; 

	hashIndex=groupAddress&rtl_fwdHashMask;
	if(rtl_fwdHashTable[hashIndex]==NULL)
	{
		return FAILED;
	}
	else
	{
		groupEntry=rtl_searchGroupEntry(groupAddress, rtl_fwdHashTable[hashIndex]);
		if(groupEntry==NULL)
		{
			return FAILED;
		}
		else
		{
			if((groupEntry->lookupTableFlag & IN_LOOKUP_TABLE)!=0)
			{
				*lutFlag=TRUE;
			}
			else
			{
				*lutFlag=FALSE;
			}

			if((groupEntry->lookupTableFlag & AGGREGATOR_FLAG)!=0)
			{
				*aggregatorFlag=TRUE;
			}
			else
			{
				*aggregatorFlag=FALSE;
			}

			return SUCCESS;
		}
	}

	
}
#endif


/*
@func int32	| rtl_disableIgmpSnoopingV1	|   IGMP snooping invalidation function
@parm void	| void
@rvalue SUCCESS	| IGMP snooping module is disabled.
@rvalue FAILED	| IGMP snooping invalidation function is failed.
@comm 
Disable IGMP snooping module.
*/
int32 rtl_disableIgmpSnoopingV1(void)
{
       uint32 i=0;
	struct rtl_groupEntry* groupEntryPtr=NULL;
	uint32 entryaddr=0;
	uint8 multicastMacAddress[6];
	
       if(rtl_igmpStatus==ENABLE)
       {
	       rtl_sysUpSeconds=0;         
	       rtl_cpuPortMask=0x00;
	       rtl_mCastDataToCpu=TRUE;

              #ifndef RTL_IGMP_SNOOPING_TEST	
	        rtl8306_setAsicIGMPMLDSnooping(RTL8306_IGMP, FALSE);
	       #endif
	
	       for(i=0; i<5; i++)
	      {
		       rtl_timerIndexToPortmask[i]=0x00;

	       }

	       for(i=0; i<6; i++)
	       {
		      rtl_portNumToTimerIndex[i]=0xff;
	       }
	
	       /*first delete multicast entry in lookuptable*/
	       for(i=0;i<rtl_fwdHashTableSize;i++)
	      {
                  groupEntryPtr=rtl_fwdHashTable[i];
				
		    while(groupEntryPtr!=NULL)
		   {
		  	     rtl_mapMuticastIPToMAC(groupEntryPtr->groupAddress,multicastMacAddress);
                          if((groupEntryPtr->lookupTableFlag&IN_LOOKUP_TABLE)!=0)
                          {
					  if(rtl8306_deleteMacAddress(multicastMacAddress, &entryaddr)==SUCCESS)
					  {
                                           if((groupEntryPtr->lookupTableFlag&AGGREGATOR_FLAG)!=0)
                                              {
                                                   rtl_setAggregator(groupEntryPtr->groupAddress, FALSE);
						    }
											  
			                         		rtl_unlinkGroupEntry(groupEntryPtr, i);
								rtl_clearGroupEntry(groupEntryPtr);
								rtl_freeGroupEntry(groupEntryPtr);
     

					  }
					  else
					  {
                                              rtl_igmpGluePrintf("hardware is failure\n");
						    return FAILED;
					  }
                          }
			     else
			     {
			                         		rtl_unlinkGroupEntry(groupEntryPtr, i);
								rtl_clearGroupEntry(groupEntryPtr);
								rtl_freeGroupEntry(groupEntryPtr);
					
			     }

			     groupEntryPtr=rtl_fwdHashTable[i];
		   }
	       }

	       rtl_igmpGlueFree(rtl_fwdHashTable);
	       rtl_fwdHashTable=NULL;
	       rtl_fwdHashTableSize=0;
	       rtl_fwdHashMask=0;

	       i=0;
	       groupEntryPtr=rtl_groupEntryPool;
	       while(groupEntryPtr!=NULL)
	       {
		       rtl_groupEntryPool=rtl_groupEntryPool->next;
		       memset(groupEntryPtr,0,sizeof(struct rtl_groupEntry));
		       i++;
	 	       groupEntryPtr=rtl_groupEntryPool;
	        }

	       if(i!=rtl_totalMaxGroupCnt)
	       {
		       return FAILED;
	        }
	       rtl_totalMaxGroupCnt=0;

	       rtl_igmpGlueFree(rtl_memoryPool);
		rtl_memoryPool=NULL;
		rtl_groupEntryPool=NULL;

	       memset(&rtl_multicastParas,0,sizeof(struct rtl_igmpSnoopingParameter));


	       rtl_multicastRouters.querier.portMask=0x00;
     	       rtl_multicastRouters.dvmrpRouter.portMask=0x00;
	       rtl_multicastRouters.pimDmRouter.portMask=0x00;
	       rtl_multicastRouters.mospfRouter.portMask=0x00;
	
	      for(i=0; i<6; i++)
	     {
		    rtl_multicastRouters.querier.portTimer[i]=0x00;
		    rtl_multicastRouters.dvmrpRouter.portTimer[i]=0;
		    rtl_multicastRouters.pimDmRouter.portTimer[i]=0;
		    rtl_multicastRouters.mospfRouter.portTimer[i]=0;
	      }


		

			if((rtl_gatewayMac[0]|rtl_gatewayMac[1]|rtl_gatewayMac[2]|rtl_gatewayMac[3]|rtl_gatewayMac[4]|rtl_gatewayMac[5])!=0)
			{
			rtl8306_deleteMacAddress(rtl_gatewayMac, &entryaddr);
			 }
              	 for(i=0; i<6; i++)
	     		{
		   		 rtl_gatewayMac[i]=0;
	      		}
				 rtl_gatewayIp=0;
				
		
		
	#ifndef RTL_IGMP_SNOOPING_TEST
      /*enable CPU learning*/
	rtl8306_setAsicPortLearningAbility(rtl_mapPortMaskToPortNum(rtl_cpuPortMask), TRUE);
	#endif
	    

             rtl_igmpStatus=DISABLE;
       }
	   else
	  {
		rtl_igmpGluePrintf("IGMP is disabled \n");
	   }
		
	return SUCCESS;
}


#ifdef RTL_IGMP_SNOOPING_TEST
int32 rtl_exitIgmpSnoopingV1(void)
{
	uint32 i=0;
	struct rtl_groupEntry* groupEntryPtr=NULL;

	rtl_sysUpSeconds=0;         
	rtl_etherType=0x0000;
	rtl_cpuPortMask=0x00;
	rtl_mCastDataToCpu=TRUE;
	for(i=0; i<5; i++)
	{
		rtl_timerIndexToPortmask[i]=0x00;

	}

	for(i=0; i<6; i++)
	{
		rtl_portNumToTimerIndex[i]=0xff;
	}


	for(i=0; i<rtl_fwdHashTableSize; i++)
	{
		
		groupEntryPtr=rtl_fwdHashTable[i];
		while(groupEntryPtr!=NULL)
		{
			rtl_fwdHashTable[i]=rtl_fwdHashTable[i]->next;
			memset(groupEntryPtr,0,sizeof(struct rtl_groupEntry));
			rtl_freeGroupEntry(groupEntryPtr);

			groupEntryPtr=rtl_fwdHashTable[i];
		}
	}

	rtl_igmpGlueFree(rtl_fwdHashTable);
	rtl_fwdHashTable=NULL;
	rtl_fwdHashTableSize=0;
	rtl_fwdHashMask=0;

	i=0;
	groupEntryPtr=rtl_groupEntryPool;
	while(groupEntryPtr!=NULL)
	{
		rtl_groupEntryPool=rtl_groupEntryPool->next;
		memset(groupEntryPtr,0,sizeof(struct rtl_groupEntry));
		i++;
		groupEntryPtr=rtl_groupEntryPool;
	}

	if(i!=rtl_totalMaxGroupCnt)
	{
		return FAILED;
	}
	rtl_totalMaxGroupCnt=0;

	rtl_igmpGlueFree(rtl_memoryPool);

	memset(&rtl_multicastParas,0,sizeof(struct rtl_igmpSnoopingParameter));


	rtl_multicastRouters.querier.portMask=0x00;

	rtl_multicastRouters.dvmrpRouter.portMask=0x00;
	rtl_multicastRouters.pimDmRouter.portMask=0x00;
	rtl_multicastRouters.mospfRouter.portMask=0x00;
	
	for(i=0; i<6; i++)
	{
		rtl_multicastRouters.querier.portTimer[i]=0x00;
		rtl_multicastRouters.dvmrpRouter.portTimer[i]=0;
		rtl_multicastRouters.pimDmRouter.portTimer[i]=0;
		rtl_multicastRouters.mospfRouter.portTimer[i]=0;
	}

	return SUCCESS;
}

int32 rtl8306_getAsicCPUPort(uint32 *port, uint32 *entag)
{
         *port=cpuPortNumer;
	  return SUCCESS;
}
#endif


#ifdef IGMP_DEBUG
void rtl_igmpDump(uint32 dumpOpration, uint32 groupAddress)
{
	uint32 i=0;
	uint32 portIndex=0;
	
	uint32 hashIndex=0;
	uint32 timerIndex=0;
	struct rtl_groupEntry* groupEntry=NULL;
	
	uint8 multicastRouterPortMask=rtl_getMulticastRouterPortMask();
	uint8 aggregatorPortMask=0;
	uint8 aggregatorLookupTableFlag=0;
	uint8 lookupTablePortMask=0;
	switch(dumpOpration)
	{
		/*****************************************************************************/
		case DUMP_IGMP_SETTINGS:  	
		rtl_igmpGluePrintf("\n");
		rtl_igmpGluePrintf("the current system time is %d\n",rtl_sysUpSeconds);
		rtl_igmpGluePrintf("the cpu port mask is 0x%x\n",rtl_cpuPortMask);
		rtl_igmpGluePrintf("the realtek ether-type is 0x%x\n",rtl_etherType);
		
		rtl_igmpGluePrintf("the gateway mac address is %x:%x:%x:%x:%x:%x\n",rtl_gatewayMac[0],rtl_gatewayMac[1],rtl_gatewayMac[2],rtl_gatewayMac[3],rtl_gatewayMac[4],rtl_gatewayMac[5]);
		rtl_igmpGluePrintf("the gateway ip address is %d.%d.%d.%d\n", ((rtl_gatewayIp&0xff000000)>>24),((rtl_gatewayIp&0x00ff0000)>>16),((rtl_gatewayIp&0x0000ff00)>>8),(rtl_gatewayIp&0x000000ff));

		

		rtl_igmpGluePrintf("the group member aging time is %d seconds\n", rtl_multicastParas.groupMemberAgingTime);
		rtl_igmpGluePrintf("the last group member aging time is %d seconds\n", rtl_multicastParas.lastMemberAgingTime);
		rtl_igmpGluePrintf("the querier Present Interval is %d seconds\n", rtl_multicastParas.querierPresentInterval);
		
		rtl_igmpGluePrintf("the dvmrp multicast router aging time is %d seconds\n", rtl_multicastParas.dvmrpRouterAgingTime);
		rtl_igmpGluePrintf("the mospf multicast router aging time is %d seconds\n", rtl_multicastParas.mospfRouterAgingTime);
		rtl_igmpGluePrintf("the pim multicast router aging time is %d seconds\n", rtl_multicastParas.pimDmRouterAgingTime);

		if(rtl_mCastDataToCpu==TRUE)
		{
			rtl_igmpGluePrintf("enable multicast data  forward to cpu port\n");
		}
		else
		{
			rtl_igmpGluePrintf("disable multicast data forward to cpu port\n");
		}
		break;

		
		/*****************************************************************************/
		case DUMP_MCAST_ROUTER_INFORMATION:  

		rtl_igmpGluePrintf("the  querier router port mask is 0x%x\n",rtl_multicastRouters.querier.portMask);
		rtl_igmpGluePrintf("the  dvmrp router port mask is 0x%x\n",rtl_multicastRouters.dvmrpRouter.portMask);
		rtl_igmpGluePrintf("the  mospf router port mask is 0x%x\n",rtl_multicastRouters.mospfRouter.portMask);
		rtl_igmpGluePrintf("the  pim router port mask is 0x%x\n",rtl_multicastRouters.pimDmRouter.portMask);

		for(portIndex=0; portIndex<6; portIndex++)
		{
			if(rtl_multicastRouters.querier.portTimer[portIndex]>0)
			{
				rtl_igmpGluePrintf("the  port%d  querier timer is %d seconds\n",portIndex,rtl_multicastRouters.querier.portTimer[portIndex]);
			}
			
			if(rtl_multicastRouters.dvmrpRouter.portTimer[portIndex]>0)      /*check whether DVMRP router has expired*/
			{
				rtl_igmpGluePrintf("the  port%d  dvmrp router  timer is %d seconds\n",portIndex,rtl_multicastRouters.dvmrpRouter.portTimer[portIndex]);
			}

			if(rtl_multicastRouters.mospfRouter.portTimer[portIndex]>0)  /*check whether MOSPF router has expired*/
			{
				rtl_igmpGluePrintf("the  port%d  mospf router timer is %d seconds\n",portIndex,rtl_multicastRouters.mospfRouter.portTimer[portIndex]);
			}


			if(rtl_multicastRouters.pimDmRouter.portTimer[portIndex]>0)    /*check whether PIM-DM router has expired*/
			{
				rtl_igmpGluePrintf("the  port%d  pim-dm router timer is %d seconds\n",portIndex,rtl_multicastRouters.pimDmRouter.portTimer[portIndex]);
			}
			
			
		}
		
		break;
		
		/*****************************************************************************/
		case DUMP_GROUP_INFORMATION: 
		 if(IS_CLASSD_ADDR(groupAddress))
		 {
		 	hashIndex=groupAddress&rtl_fwdHashMask;
			groupEntry=rtl_searchGroupEntry(groupAddress, rtl_fwdHashTable[hashIndex]); 
			if(groupEntry==NULL)
			{
				rtl_igmpGluePrintf("%d.%d.%d.%d multcast address not found\n",((groupAddress&0xff000000)>>24),((groupAddress&0x00ff0000)>>16),((groupAddress&0x0000ff00)>>8),(groupAddress&0x000000ff));
			}
			else
			{	
				rtl_igmpGluePrintf("%d.%d.%d.%d member port mask is 0x%x\n",((groupAddress&0xff000000)>>24),((groupAddress&0x00ff0000)>>16),((groupAddress&0x0000ff00)>>8),(groupAddress&0x000000ff),groupEntry->fwdPortMask);
				for(portIndex=0; portIndex<6; portIndex++)
				{
					timerIndex=rtl_portNumToTimerIndex[portIndex];
					if(timerIndex<=5)
					{
						if(groupEntry->portTimer[timerIndex]>0)
						{
							rtl_igmpGluePrintf("%d.%d.%d.%d  port%d timer is %d seconds\n",((groupAddress&0xff000000)>>24),((groupAddress&0x00ff0000)>>16),((groupAddress&0x0000ff00)>>8),(groupAddress&0x000000ff),i,groupEntry->portTimer[timerIndex]);
						}
						else
						{
							rtl_igmpGluePrintf("%d.%d.%d.%d  port%d timer is 0 seconds\n",((groupAddress&0xff000000)>>24),((groupAddress&0x00ff0000)>>16),((groupAddress&0x0000ff00)>>8),(groupAddress&0x000000ff),portIndex);
						}
					}
					else
					{
						rtl_igmpGluePrintf("%d.%d.%d.%d  port%d timer is 0 seconds\n",((groupAddress&0xff000000)>>24),((groupAddress&0x00ff0000)>>16),((groupAddress&0x0000ff00)>>8),(groupAddress&0x000000ff),portIndex);
					}
				}
				
				if((groupEntry->lookupTableFlag & IN_LOOKUP_TABLE)!=0)
				{
					rtl_igmpGluePrintf("%d.%d.%d.%d  has been set into look up table\n",((groupAddress&0xff000000)>>24),((groupAddress&0x00ff0000)>>16),((groupAddress&0x0000ff00)>>8),(groupAddress&0x000000ff));
					if((groupEntry->lookupTableFlag&AGGREGATOR_FLAG)!=0)
					{
						rtl_igmpGluePrintf("this group address is an aggregator multicast address\n");	
						rtl_checkAggregator(groupAddress, &aggregatorPortMask, &aggregatorLookupTableFlag);
						if(rtl_mCastDataToCpu==FALSE)
						{
							lookupTablePortMask=(uint32)((aggregatorPortMask|multicastRouterPortMask)&(~rtl_cpuPortMask));
						}
						else
						{
							lookupTablePortMask=(uint32)(aggregatorPortMask|multicastRouterPortMask|rtl_cpuPortMask);
						}
						rtl_igmpGluePrintf("its look up table port mask is 0x%x\n",lookupTablePortMask);	
					}
					else
					{
						if(rtl_mCastDataToCpu==FALSE)
						{
							lookupTablePortMask=(uint32)((groupEntry->fwdPortMask|multicastRouterPortMask)&(~rtl_cpuPortMask));
						}
						else
						{
							lookupTablePortMask=(uint32)(groupEntry->fwdPortMask|multicastRouterPortMask|rtl_cpuPortMask);
						}
						rtl_igmpGluePrintf("its look up table port mask is 0x%x\n",lookupTablePortMask);	
					}
				}
				else
				{
					rtl_igmpGluePrintf("%d.%d.%d.%d  isn't in look up table\n",((groupAddress&0xff000000)>>24),((groupAddress&0x00ff0000)>>16),((groupAddress&0x0000ff00)>>8),(groupAddress&0x000000ff));
				}
				
			}

		 }
		 else
		 {
			rtl_igmpGluePrintf("%d.%d.%d.%d is not a multcast address",((groupAddress&0xff000000)>>24),((groupAddress&0x00ff0000)>>16),((groupAddress&0x0000ff00)>>8),(groupAddress&0x000000ff));
		 }

		break;

		
		/*****************************************************************************/
		case DUMP_ALL:

			for(i=0; i<rtl_fwdHashTableSize; i++)
			{
				groupEntry=rtl_fwdHashTable[i];
				while(groupEntry!=NULL)
				{
					rtl_igmpGluePrintf("multcast address:%d.%d.%d.%d \n",((groupEntry->groupAddress&0xff000000)>>24),((groupEntry->groupAddress&0x00ff0000)>>16),((groupEntry->groupAddress&0x0000ff00)>>8),(groupEntry->groupAddress&0x000000ff));
					rtl_igmpGluePrintf("its member port mask is 0x%x\n",groupEntry->fwdPortMask);
					for(portIndex=0; portIndex<6;portIndex++)
					{
						timerIndex=rtl_portNumToTimerIndex[portIndex];
						if(timerIndex<=5)
						{
							if(groupEntry->portTimer[timerIndex]>0)
							{
								rtl_igmpGluePrintf("port%d timer is %d seconds\n",portIndex,groupEntry->portTimer[timerIndex]);
							}
							else
							{
								rtl_igmpGluePrintf("port%d timer is 0 seconds\n",portIndex);
							}
						}
						else
						{
							rtl_igmpGluePrintf("port%d timer is 0 seconds\n",portIndex);
						}
					}
					
					if((groupEntry->lookupTableFlag & IN_LOOKUP_TABLE)!=0)
					{
						rtl_igmpGluePrintf("it has been set into look up table\n");
						if((groupEntry->lookupTableFlag&AGGREGATOR_FLAG)!=0)
						{
							rtl_igmpGluePrintf("it's an aggregator multicast address\n");	
							rtl_checkAggregator(groupEntry->groupAddress, &aggregatorPortMask, &aggregatorLookupTableFlag);
							if(rtl_mCastDataToCpu==FALSE)
							{
								lookupTablePortMask=(uint32)((aggregatorPortMask|multicastRouterPortMask)&(~rtl_cpuPortMask));
							}
							else
							{
								lookupTablePortMask=(uint32)(aggregatorPortMask|multicastRouterPortMask|rtl_cpuPortMask);
							}
							rtl_igmpGluePrintf("its look up table port mask is 0x%x\n\n",lookupTablePortMask);	
						}
						else
						{
							if(rtl_mCastDataToCpu==FALSE)
							{
								lookupTablePortMask=(uint32)((groupEntry->fwdPortMask|multicastRouterPortMask)&(~rtl_cpuPortMask));
							}
							else
							{
								lookupTablePortMask=(uint32)(groupEntry->fwdPortMask|multicastRouterPortMask|rtl_cpuPortMask);
							}
							rtl_igmpGluePrintf("its look up table port mask is 0x%x\n\n",lookupTablePortMask);	
						}
					}
					else
					{
						rtl_igmpGluePrintf("it is not in look up table\n\n");
					}
					
					groupEntry=groupEntry->next;
				}
			}
			
			break;
		
		/*****************************************************************************/
		default:
			break;

		}
		
}

void rtl_setCpuPortNum(uint32 cpuPortNum)
{
	uint32 i=0;
	struct rtl_groupEntry* groupEntry=NULL;

	if(cpuPortNum>6)
	{
		return;
	}
	
	rtl_cpuPortMask=rtl_mapPortNumToPortMask(cpuPortNum);
	for(i=0; i<rtl_fwdHashTableSize; i++)
	{
		groupEntry=rtl_fwdHashTable[i];
		while(groupEntry!=NULL)
		{
			if((groupEntry->lookupTableFlag & IN_LOOKUP_TABLE)!=0)
			{
				rtl_set8306LookupTable(groupEntry);
			}
			
			groupEntry=groupEntry->next;
		}
	}
	
}

void rtl_setMCastDataToCpu(uint32 toCpuFlag)
{
	uint32 i=0;
	struct rtl_groupEntry* groupEntry=NULL;

	if((toCpuFlag!=0) && (toCpuFlag!=1))
	{
		return;
	}

	if(toCpuFlag==rtl_mCastDataToCpu)
	{
		return;
	}
	else
	{
		rtl_mCastDataToCpu=toCpuFlag;
		for(i=0; i<rtl_fwdHashTableSize; i++)
		{
			groupEntry=rtl_fwdHashTable[i];
			while(groupEntry!=NULL)
			{
				if((groupEntry->lookupTableFlag & IN_LOOKUP_TABLE)!=0)
				{
					rtl_set8306LookupTable(groupEntry);
				}
				groupEntry=groupEntry->next;
			}
		}
	}

}

void rtl_setCpuIsIgmpRouter(uint32 igmpProxyAvailable)
{
	if((igmpProxyAvailable==FALSE) || (igmpProxyAvailable==TRUE))
	{
		rtl_delPortMaskRevLeave=igmpProxyAvailable;
	}
}

#endif


