/*
* Copyright c                  Realtek Semiconductor Corporation, 2006 
* All rights reserved.
* 
* Program : RTL8366S/SR switch high-level API
* Abstract : 
* Author : Nick Wu(nickwu@realtek.com.tw)                
* $Id: rtl8366s_api.c,v 1.51 2007/11/05 05:05:03 hmchung Exp $
*/
/*	@doc RTL8366S_API

	@module Rtl8366s_api.c - RTL8366S/SR switch high-level API documentation	|
	This document explains high-level API interface of the asic. 
	@normal 

	Copyright <cp>2007 Realtek<tm> Semiconductor Cooperation, All Rights Reserved.

 	@head3 List of Symbols |
 	Here is a list of all functions and variables in this module.

 	@index | RTL8366S_API
*/

#include "rtl8366s_errno.h"
#include "rtl8366s_asicdrv.h"
#include "rtl8366s_api.h"
  
#define DELAY_800MS_FOR_CHIP_STATABLE() {  }
/*
Based on 20070907_8366s_yjlou_6Q.txt
gap: 32		drop: 800
<Share buffer> OFF: 206      ON: 218
<Port  based>  OFF: 510      ON: 511
<Q-Desc based> OFF:  62      ON:  74
<Q-pkt based>  OFF:  28      ON:  32
*/
/* QoS parameter & threshold */
const uint16 g_thresholdGap[6]= { 32,32,32,32,32,32 };
const uint16 g_thresholdSystem[6][3]= { {218,206,800},{218,206,800},{218,206,800},{218,206,800},{218,206,800},{218,206,800} };
const uint16 g_thresholdPort[6][2]= { {511,510},{511,510},{511,510},{511,510},{511,510},{511,510} };
const uint16 g_thresholdQueueDescriptor[6][2]= { {74,62},{74,62},{74,62},{74,62},{74,62},{74,62} };
const uint16 g_thresholdQueuePacket[6][2]= { {32,28},{32,28},{32,28},{32,28},{32,28},{32,28} };

const uint16 g_prioritytToQueue[6][8]= { 
	{QUEUE0,QUEUE0,QUEUE0,QUEUE0,QUEUE0,QUEUE0,QUEUE0,QUEUE0}, 
	{QUEUE0,QUEUE0,QUEUE0,QUEUE0,QUEUE5,QUEUE5,QUEUE5,QUEUE5}, 
	{QUEUE0,QUEUE0,QUEUE0,QUEUE0,QUEUE1,QUEUE1,QUEUE5,QUEUE5}, 
	{QUEUE0,QUEUE0,QUEUE1,QUEUE1,QUEUE2,QUEUE2,QUEUE5,QUEUE5},
	{QUEUE0,QUEUE0,QUEUE1,QUEUE1,QUEUE2,QUEUE3,QUEUE5,QUEUE5},
	{QUEUE0,QUEUE0,QUEUE1,QUEUE1,QUEUE2,QUEUE3,QUEUE4,QUEUE5}
};

const uint16 g_prioritytToQueue_patch[5][8]= { 
	{QUEUE1,QUEUE1,QUEUE1,QUEUE1,QUEUE1,QUEUE1,QUEUE1,QUEUE1}, 
	{QUEUE1,QUEUE1,QUEUE1,QUEUE1,QUEUE5,QUEUE5,QUEUE5,QUEUE5}, 
	{QUEUE1,QUEUE1,QUEUE1,QUEUE1,QUEUE2,QUEUE2,QUEUE5,QUEUE5}, 
	{QUEUE1,QUEUE1,QUEUE2,QUEUE2,QUEUE3,QUEUE3,QUEUE5,QUEUE5},
	{QUEUE1,QUEUE1,QUEUE2,QUEUE2,QUEUE3,QUEUE4,QUEUE5,QUEUE5},
};


const uint16 g_weightQueue[6][6] = { {0,0,0,0,0,0}, {0,0,0,0,0,1}, {0,1,0,0,0,2}, {0,1,2,0,0,3}, {0,1,2,3,0,4}, {0,1,2,3,4,5} };
const uint16 g_priority1QRemapping[8] = {1,0,2,3,4,5,6,7 };


void _rtl8366s_copy(uint8 *dest, const uint8 *src, uint32 n)
{
	uint32 i;
	
	for(i = 0; i < n; i++)
		dest[i] = src[i];
}

int32 _rtl8366s_cmp(const uint8 *s1, const uint8 *s2, uint32 n)
{
	uint32 i;
	
	for(i = 0; i < n; i++)
	{
		if(s1[i] != s2[i])
			return FAILED;/* not equal */
	}

	return SUCCESS;
}

void _rtl8366s_powof2(uint32 val, uint32 *ret)
{
	*ret = 1;
	
	for(; val > 0; val--)
		*ret = (*ret)*2;
}

/*
@func int32 | rtl8366s_initVlan | Initialize VLAN.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	VLAN is disabled by default. User has to call this API to enable VLAN before
	using it. And It will set a default VLAN(vid 1) including all ports and set 
	all ports PVID to the default VLAN.
*/
int32 rtl8366s_initVlan(void)
{
	uint32 i;
	rtl8366s_vlan4kentry vlan4K;
	rtl8366s_vlanconfig vlanMC;
#if 1	
	/* clear 16 VLAN member configuration */
	for(i = 0; i <= RTL8366S_VLANMCIDXMAX; i++)
	{	
		vlanMC.vid = 0;
		vlanMC.priority = 0;
		vlanMC.member = 0;		
		vlanMC.untag = 0;			
		vlanMC.fid = 0;
		if(rtl8366s_setAsicVlanMemberConfig(i, &vlanMC) != SUCCESS)
			return FAILED;	
	}

#endif
#if 1
	/* Set a default VLAN with vid 1 to 4K table for all ports */
   	vlan4K.vid = 1;
 	vlan4K.member = RTL8366S_PORTMASK;
 	vlan4K.untag = RTL8366S_PORTMASK;
 	vlan4K.fid = 0;	
	if(rtl8366s_setAsicVlan4kEntry(vlan4K) != SUCCESS)
		return FAILED;	

	/* Also set the default VLAN to 16 member configuration index 0 */
	vlanMC.vid = 1;
	vlanMC.priority = 0;
	vlanMC.member = RTL8366S_PORTMASK;		
	vlanMC.untag = RTL8366S_PORTMASK;			
	vlanMC.fid = 0;
	if(rtl8366s_setAsicVlanMemberConfig(0, &vlanMC) != SUCCESS)
		return FAILED;	

	/* Set all ports PVID to default VLAN */	
	for(i = 0; i < PORT_MAX; i++)
	{	
		if(rtl8366s_setAsicVlanPortBasedVID(i, 0) != SUCCESS)
			return FAILED;		
	}	

	/* enable VLAN and 4K VLAN */
	if(rtl8366s_setAsicVlan(TRUE)!= SUCCESS)
		return FAILED;	
	if(rtl8366s_setAsicVlan4kTbUsage(TRUE)!= SUCCESS)
		return FAILED;
#endif	
	return SUCCESS;
}


/*
@func int32 | rtl8366s_setVlanPVID | Set port to specified VLAN ID(PVID).
@parm enum PORTID | port | Port number (0~5).
@parm uint32 | vid | Specified VLAN ID (0~4095).
@parm uint32 | priority | 802.1p priority for the PVID (0~7).
@rvalue ERRNO_API_INVALIDPARAM | invalid input parameters.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	The API is used for Port-based VLAN. The untagged frame received from the
	port will be classified to the specified VLAN and assigned to the specified priority.
	Make sure you have configured the VLAN by 'rtl8366s_setVlan' before using the API.
*/
int32 rtl8366s_setVlanPVID(enum PORTID port, uint32 vid, uint32 priority)
{
	uint32 i;
	uint32 j;
	uint32 index;	
	uint8  bUsed;	
	rtl8366s_vlan4kentry vlan4K;
	rtl8366s_vlanconfig vlanMC;	

	if(port >= PORT_MAX)
		return ERRNO_API_INVALIDPARAM;
	
	/* vid must be 0~4095 */
	if(vid > RTL8366S_VIDMAX)
		return ERRNO_API_INVALIDPARAM;

	/* priority must be 0~7 */
	if(priority > RTL8366S_PRIORITYMAX)
		return ERRNO_API_INVALIDPARAM;
	
	/* 
		Search 16 member configuration to see if the entry already existed.
		If existed, update the priority and assign the index to the port.
	*/
	for(i = 0; i <= RTL8366S_VLANMCIDXMAX; i++)
	{	
		if(rtl8366s_getAsicVlanMemberConfig(i, &vlanMC) != SUCCESS)
			return FAILED;	

		if(vid == vlanMC.vid)
		{
			vlanMC.priority = priority;		
			if(rtl8366s_setAsicVlanMemberConfig(i, &vlanMC) != SUCCESS)
				return FAILED;	
		
			if(rtl8366s_setAsicVlanPortBasedVID(port, i) != SUCCESS)
				return FAILED;	

			return SUCCESS;
		}	
	}

	/*
		vid doesn't exist in 16 member configuration. Find an empty entry in 
		16 member configuration, then copy entry from 4K. If 16 member configuration
		are all full, then find an entry which not used by Port-based VLAN and 
		then replace it with 4K. Finally, assign the index to the port.
	*/
	for(i = 0; i <= RTL8366S_VLANMCIDXMAX; i++)
	{	
		if(rtl8366s_getAsicVlanMemberConfig(i, &vlanMC) != SUCCESS)
			return FAILED;	

		if(vlanMC.vid == 0 && vlanMC.member == 0)
		{
			vlan4K.vid = vid;
			if(rtl8366s_getAsicVlan4kEntry(&vlan4K) != SUCCESS)
				return FAILED;

			vlanMC.vid = vid;
			vlanMC.priority = priority;
			vlanMC.member = vlan4K.member;		
			vlanMC.untag = vlan4K.untag;			
			vlanMC.fid = vlan4K.fid;
			if(rtl8366s_setAsicVlanMemberConfig(i, &vlanMC) != SUCCESS)
				return FAILED;	

			if(rtl8366s_setAsicVlanPortBasedVID(port, i) != SUCCESS)
				return FAILED;	

			return SUCCESS;			
		}	
	}	

	/* 16 member configuration is full, found a unused entry to replace */
	for(i = 0; i <= RTL8366S_VLANMCIDXMAX; i++)
	{	
		bUsed = FALSE;	

		for(j = 0; j < PORT_MAX; j++)
		{	
			if(rtl8366s_getAsicVlanPortBasedVID(j, &index) != SUCCESS)
				return FAILED;	

			if(i == index)/*index i is in use by port j*/
			{
				bUsed = TRUE;
				break;
			}	
		}

		if(bUsed == FALSE)/*found a unused index, replace it*/
		{
			vlan4K.vid = vid;
			if(rtl8366s_getAsicVlan4kEntry(&vlan4K) != SUCCESS)
				return FAILED;

			vlanMC.vid = vid;
			vlanMC.priority = priority;
			vlanMC.member = vlan4K.member;		
			vlanMC.untag = vlan4K.untag;			
			vlanMC.fid = vlan4K.fid;
			if(rtl8366s_setAsicVlanMemberConfig(i, &vlanMC) != SUCCESS)
				return FAILED;	

			if(rtl8366s_setAsicVlanPortBasedVID(port, i) != SUCCESS)
				return FAILED;	

			return SUCCESS;			
		}
	}	
	
	return FAILED;
}

/*
@func int32 | rtl8366s_initChip | Set chip to default configuration enviroment
@rvalue SUCCESS | Success.
@comm
	The API can set chip registers to default configuration for different release chip model.
*/
int32 rtl8366s_initChip(void)
{
	uint32 index;
	uint32 regData;
	uint32 ledGroup;
	enum RTL8366S_LEDCONF ledCfg[RTL8366S_LED_GROUP_MAX];
	
	const uint32 chipB[][2] = {{0x0000,	0x0038},{0x8100,	0x1B37},{0xBE2E,	0x7B9F},{0xBE2B,	0xA4C8},
							{0xBE74,	0xAD14},{0xBE2C,	0xDC00},{0xBE69,	0xD20F},{0xBE3B,	0xB414},
							{0xBE24,	0x0000},{0xBE23,	0x00A1},{0xBE22,	0x0008},{0xBE21,	0x0120},
							{0xBE20,	0x1000},{0xBE24,	0x0800},{0xBE24,	0x0000},{0xBE24,	0xF000},
							{0xBE23,	0xDF01},{0xBE22,	0xDF20},{0xBE21,	0x101A},{0xBE20,	0xA0FF},
							{0xBE24,	0xF800},{0xBE24,	0xF000},{0x0242,	0x02BF},{0x0245,	0x02BF},
							{0x0248,	0x02BF},{0x024B,	0x02BF},{0x024E,	0x02BF},{0x0251,	0x02BF},
							{0x0230,	0x0A32},{0x0233,	0x0A32},{0x0236,	0x0A32},{0x0239,	0x0A32},
							{0x023C,	0x0A32},{0x023F,	0x0A32},{0x0254,	0x0A3F},{0x0255,	0x0064},
							{0x0256,	0x0A3F},{0x0257,	0x0064},{0x0258,	0x0A3F},{0x0259,	0x0064},
							{0x025A,	0x0A3F},{0x025B,	0x0064},{0x025C,	0x0A3F},{0x025D,	0x0064},
							{0x025E,	0x0A3F},{0x025F,	0x0064},{0x0260,	0x0178},{0x0261,	0x01F4},
							{0x0262,	0x0320},{0x0263,	0x0014},{0x021D,	0x9249},{0x021E,	0x0000},
							{0x0100,	0x0004},{0xBE4A,	0xA0B4},{0xBE40,	0x9C00},{0xBE41,	0x501D},
							{0xBE48,	0x3602},{0xBE47,	0x8051},{0xBE4C,	0x6465},{0x8000,	0x1F00},
							{0x8001,	0x000C},{0x8008,	0x0000},{0x8007,	0x0000},{0x800C,	0x00A5},
							{0x8101,	0x02BC},{0xBE53,	0x0005},{0x8E45,	0xAFE8},{0x8013,	0x0005},
							{0xBE4B,	0x6700},{0x800B,	0x7000},{0xBE09,	0x0E00},
							{0xFFFF, 0xABCD}};
	
	const uint32 chipDefault[][2] = {{0x0242, 0x02BF},{0x0245, 0x02BF},{0x0248, 0x02BF},{0x024B, 0x02BF},
								{0x024E, 0x02BF},{0x0251, 0x02BF},
								{0x0254, 0x0A3F},{0x0256, 0x0A3F},{0x0258, 0x0A3F},{0x025A, 0x0A3F},
								{0x025C, 0x0A3F},{0x025E, 0x0A3F},
								{0x0263, 0x007C},{0x0100,	0x0004},									
								{0xBE5B, 0x3500},{0x800E, 0x200F},{0xBE1D, 0x0F00},{0x8001, 0x5011},
								{0x800A, 0xA2F4},{0x800B, 0x17A3},{0xBE4B, 0x17A3},{0xBE41, 0x5011},
								{0xBE17, 0x2100},{0x8000, 0x8304},{0xBE40, 0x8304},{0xBE4A, 0xA2F4},
								{0x800C, 0xA8D5},{0x8014, 0x5500},{0x8015, 0x0004},{0xBE4C, 0xA8D5},
								{0xBE59, 0x0008},{0xBE09,	0x0E00},{0xBE36, 0x1036},{0xBE37, 0x1036},
								{0x800D, 0x00FF},{0xBE4D, 0x00FF},
								{0xFFFF, 0xABCD}};
	for(ledGroup= 0;ledGroup<RTL8366S_LED_GROUP_MAX;ledGroup++)
	{
		if(SUCCESS != rtl8366s_getAsicLedIndicateInfoConfig(ledGroup,&ledCfg[ledGroup]))
			return FAILED;

		if(SUCCESS != rtl8366s_setAsicLedIndicateInfoConfig(ledGroup,LEDCONF_LEDFORCE))
			return FAILED;
	}

	if(SUCCESS != rtl8366s_setAsicForceLeds(0x3F,0x3F,0x3F,0x3F))
		return FAILED;

	/*resivion*/
	if(SUCCESS != rtl8366s_getAsicReg(0x5C,&regData))
		return FAILED;

	index = 0;
	switch(regData)
	{
 	 case 0x0000:	
		
		while(chipB[index][0] != 0xFFFF && chipB[index][1] != 0xABCD)
		{	
			/*PHY registers setting*/	
			if(0xBE00 == (chipB[index][0] & 0xBE00))
			{
				if(SUCCESS != rtl8366s_setAsicReg(RTL8366S_PHY_ACCESS_CTRL_REG, RTL8366S_PHY_CTRL_WRITE))
					return FAILED;
			}

			if(SUCCESS != rtl8366s_setAsicReg(chipB[index][0],chipB[index][1]))
				return FAILED;
			
			index ++;	
		}			
		break;
 	 case 0x6027:	
		while(chipDefault[index][0] != 0xFFFF && chipDefault[index][1] != 0xABCD)
		{	
			/*PHY registers setting*/	
			if(0xBE00 == (chipDefault[index][0] & 0xBE00))
			{
				if(SUCCESS != rtl8366s_setAsicReg(RTL8366S_PHY_ACCESS_CTRL_REG, RTL8366S_PHY_CTRL_WRITE))
					return FAILED;

			}

			if(SUCCESS != rtl8366s_setAsicReg(chipDefault[index][0],chipDefault[index][1]))
				return FAILED;
			
			index ++;	
		}			
		break;
	 default:
		return FAILED;
		break;
	}

	DELAY_800MS_FOR_CHIP_STATABLE();


	for(ledGroup= 0;ledGroup<RTL8366S_LED_GROUP_MAX;ledGroup++)
	{
		if(SUCCESS != rtl8366s_setAsicLedIndicateInfoConfig(ledGroup,ledCfg[ledGroup]))
			return FAILED;
			
	}

	return SUCCESS;
}


/*
@func int32 | rtl8366s_setVlan_with_fid | Set a VLAN entry with specified filtering database.
@parm uint32 | vid | VLAN ID to configure (0~4095).
@parm uint32 | mbrmsk | VLAN member set portmask (0~0x3F).
@parm uint32 | untagmsk | VLAN untag set portmask (0~0x3F).
@parm uint32 | fid | set VLAN filtering database (0~0x7).
@rvalue ERRNO_API_INVALIDPARAM | invalid input parameters.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	There are 4K VLAN entry supported. Member set and untag set	of 4K VLAN entry
	are all zero by default.	User could configure the member	set and untag set
	for specified vid and fid through this API. The portmask's bit N means port N.
	For example, mbrmask 23=0x17=010111 means port 0,1,2,4 in the member set.
*/
int32 rtl8366s_setVlan_with_fid(uint32 vid, uint32 mbrmsk, uint32 untagmsk, uint32 fid)
{
	uint32 i;
	rtl8366s_vlan4kentry vlan4K;
	rtl8366s_vlanconfig vlanMC;	
	
	/* vid must be 0~4095 */
	if(vid > RTL8366S_VIDMAX)
		return ERRNO_API_INVALIDPARAM;

	if(mbrmsk > RTL8366S_PORTMASK)
		return ERRNO_API_INVALIDPARAM;

	if(untagmsk > RTL8366S_PORTMASK)
		return ERRNO_API_INVALIDPARAM;

	/* fid must be 0~7 */
	if(fid > RTL8366S_FIDMAX)
		return ERRNO_API_INVALIDPARAM;	

	/* update 4K table */
   	vlan4K.vid = vid;			
 	vlan4K.member = mbrmsk;
 	vlan4K.untag = untagmsk;
 	vlan4K.fid = fid;	
	
	if(rtl8366s_setAsicVlan4kEntry(vlan4K) != SUCCESS)
		return FAILED;
	
	/* 
		Since VLAN entry would be copied from 4K to 16 member configuration while
		setting Port-based VLAN. So also update the entry in 16 member configuration
		if it existed.
	*/
	for(i = 0; i <= RTL8366S_VLANMCIDXMAX; i++)
	{	
		if(rtl8366s_getAsicVlanMemberConfig(i, &vlanMC) != SUCCESS)
			return FAILED;	

		if(vid == vlanMC.vid)
		{
			vlanMC.member = mbrmsk;		
			vlanMC.untag = untagmsk;	
			vlanMC.fid = fid;
			if(rtl8366s_setAsicVlanMemberConfig(i, &vlanMC) != SUCCESS)
				return FAILED;	

			return SUCCESS;
		}	
	}
	
	return SUCCESS;
}

