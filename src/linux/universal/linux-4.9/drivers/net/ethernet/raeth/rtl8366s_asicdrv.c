/*
* Copyright c                  Realtek Semiconductor Corporation, 2006 
* All rights reserved.
* 
* Program : RTL8366S switch low-level API
* Abstract : 
* Author :Abel Hsu(abelshie@realtek.com.tw)                
*  $Id: rtl8366s_asicdrv.c,v 1.22 2007/10/16 02:56:50 abelshie Exp $
*/
/*	@doc RTL8366S_ASICDRIVER_API

	@module Rtl8366S_AsicDrv.c - RTL8366S Switch asic driver API documentation	|
	This document explains API interface of the asic. 
	@normal 

	Copyright <cp>2007 Realtek<tm> Semiconductor Cooperation, All Rights Reserved.

 	@head3 List of Symbols |
 	Here is a list of all functions and variables in this module. 

 	@index | RTL8366S_ASICDRIVER_API
*/
#include "rtl8366s_types.h"
#include "rtl8366s_asicdrv.h"
#include "rtl8366s_errno.h"
#include "smi_rtl.h" 


/*for driver verify testing only*/
#ifdef CONFIG_RTL8366S_ASICDRV_TEST
#define RTL8366S_VIRTUAL_REG_SIZE		0x500
uint16 Rtl8366sVirtualReg[RTL8366S_VIRTUAL_REG_SIZE];
rtl8366s_smiacltable Rtl8366sVirtualAclTable[RTL8366S_ACLRULENO];
rtl8366s_vlan4kentry Rtl8366sVirtualVlanTable[RTL8366S_VIDMAX + 1];
rtl8366s_l2table Rtl8366sVirtualL2Table[RTL8366S_L2ENTRYMAX + 1]; 

uint64 rtl8366sVirtualMIBsReadData;
uint64 rtl8366sVirtualMIBsControlReg;

#define RTL8366S_MIB_START_ADDRESS				0x1000
#define RTL8366S_MIB_END_ADDRESS				0x11B3

#define RTL8366S_MIB_CTRL_VIRTUAL_ADDR			0x4FF
#define RTL8366S_MIB_READ_VIRTUAL_ADDR			0x4F0
#endif


/*=======================================================================
 * 1. Asic read/write driver through SMI
 *========================================================================*/
/*
@func int32 | rtl8366s_setAsicRegBit | Set a bit value of a specified register.
@parm uint32 | reg | Register's address.
@parm uint32 | bit | Bit location. For 16-bits register only. Maximun value is 15 for MSB location.
@parm uint32 | value | Value to set. It can be value 0 or 1.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@rvalue TBLDRV_EINVALIDINPUT | Invalid input parameter. 
@comm
	Set a bit of a specified register to 1 or 0. It is 16-bits system of RTL8366s chip.
	
*/
int32 rtl8366s_setAsicRegBit(uint32 reg, uint32 bit, uint32 value)
{

#if defined(RTK_X86_ASICDRV)//RTK-CNSD2-NickWu-20061222: for x86 compile

	uint32 regData;
	int32 retVal;
	
	if(bit>=RTL8366S_REGBITLENGTH)
		return ERRNO_INVALIDINPUT;

	retVal = RTL_SINGLE_READ(reg, 2, &regData);
	if (retVal != TRUE) return ERRNO_SMIERROR;
#ifdef RTK_DEBUG
	PRINT("get reg=0x%4.4x data=0x%4.4x\n",reg,regData);
#endif

	if (value) 
		regData = regData | (1<<bit);
	else
		regData = regData & ~(1<<bit);

	retVal = RTL_SINGLE_WRITE(reg, regData);
	if (retVal != TRUE) return ERRNO_SMIERROR;

#ifdef RTK_DEBUG
	PRINT("set reg=0x%4.4x data=0x%4.4x\n",reg,regData);
#endif

	
#elif defined(CONFIG_RTL8366S_ASICDRV_TEST)

	/*MIBs emulating*/
	if(reg == RTL8366S_MIB_CTRL_REG)
	{
		reg = RTL8366S_MIB_CTRL_VIRTUAL_ADDR;
	}	
	else if(bit>=RTL8366S_REGBITLENGTH)
		return ERRNO_INVALIDINPUT;
	else if(reg >= RTL8366S_VIRTUAL_REG_SIZE)
		return ERRNO_REGUNDEFINE;

	if (value) 
	{
		Rtl8366sVirtualReg[reg] =  Rtl8366sVirtualReg[reg] | (1<<bit);

	}
	else
	{
		Rtl8366sVirtualReg[reg] =  Rtl8366sVirtualReg[reg] & (~(1<<bit));
	}
	
#else
	uint32 regData;
	int32 retVal;
	
	if(bit>=RTL8366S_REGBITLENGTH)
		return ERRNO_INVALIDINPUT;

	retVal = smi_read(reg, &regData);
	if (retVal != SUCCESS) return ERRNO_SMIERROR;

	if (value) 
		regData = regData | (1<<bit);
	else
		regData = regData & (~(1<<bit));
	
	retVal = smi_write(reg, regData);
	if (retVal != SUCCESS) return ERRNO_SMIERROR;
#endif
	return SUCCESS;
}

/*
@func int32 | rtl8366s_setAsicRegBits | Set bits value of a specified register.
@parm uint32 | reg | Register's address.
@parm uint32 | bits | Bits mask for setting. 
@parm uint32 | value | Bits value for setting. Value of bits will be set with mapping mask bit is 1.   
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@rvalue TBLDRV_EINVALIDINPUT | Invalid input parameter. 
@comm
	Set bits of a specified register to value. Both bits and value are be treated as bit-mask.
	
*/
int32 rtl8366s_setAsicRegBits(uint32 reg, uint32 bits, uint32 value)
{
	
#if defined(RTK_X86_ASICDRV)//RTK-CNSD2-NickWu-20061222: for x86 compile

	uint32 regData;
	int32 retVal;
	
	if(bits>= (1<<RTL8366S_REGBITLENGTH) )
		return ERRNO_INVALIDINPUT;

	retVal = RTL_SINGLE_READ(reg, 2, &regData);
	if (retVal != TRUE) return ERRNO_SMIERROR;
#ifdef RTK_DEBUG
	PRINT("get reg=0x%4.4x data=0x%4.4x\n",reg,regData);
#endif

	regData = regData & (~bits);
	regData = regData | (value & bits);

	retVal = RTL_SINGLE_WRITE(reg, regData);
	if (retVal != TRUE) return ERRNO_SMIERROR;
#ifdef RTK_DEBUG
	PRINT("set reg=0x%4.4x data=0x%4.4x\n",reg,regData);
#endif
	
#elif defined(CONFIG_RTL8366S_ASICDRV_TEST)
	uint32 regData;

	/*MIBs emulating*/
	if(reg == RTL8366S_MIB_CTRL_REG)
	{
		reg = RTL8366S_MIB_CTRL_VIRTUAL_ADDR;
	}	
	else if(reg >= RTL8366S_VIRTUAL_REG_SIZE)
		return ERRNO_REGUNDEFINE;

	regData = Rtl8366sVirtualReg[reg] & (~bits);
	regData = regData | (value & bits);
	
	Rtl8366sVirtualReg[reg] = regData;
	
#else
	uint32 regData;
	int32 retVal;
	
	if(bits>= (1<<RTL8366S_REGBITLENGTH) )
		return ERRNO_INVALIDINPUT;

	retVal = smi_read(reg, &regData);
	if (retVal != SUCCESS) return ERRNO_SMIERROR;

	regData = regData & (~bits);
	regData = regData | (value & bits);

	
	retVal = smi_write(reg, regData);
	if (retVal != SUCCESS) return ERRNO_SMIERROR;
#endif
	return SUCCESS;
}

/*
@func int32 | rtl8366s_getAsicReg | Get content of register.
@parm uint32 | reg | Register's address.
@parm uint32* | value | Value of register.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@comm
 	Value 0x0000 will be returned for ASIC un-mapping address.
	
*/
int32 rtl8366s_getAsicReg(uint32 reg, uint32 *value)
{
	
#if defined(RTK_X86_ASICDRV)//RTK-CNSD2-NickWu-20061222: for x86 compile

	uint32 regData;
	int32 retVal;

	retVal = RTL_SINGLE_READ(reg, 2, &regData);
	if (retVal != TRUE) return ERRNO_SMIERROR;

	*value = regData;
	
#ifdef RTK_DEBUG
	PRINT("get reg=0x%4.4x data=0x%4.4x\n",reg,regData);
#endif

#elif defined(CONFIG_RTL8366S_ASICDRV_TEST)
       uint16 virtualMIBs;

	/*MIBs emulating*/
	if(reg == RTL8366S_MIB_CTRL_REG)
	{
		reg = RTL8366S_MIB_CTRL_VIRTUAL_ADDR;
	}	
	else if(reg >= RTL8366S_MIB_START_ADDRESS && reg <= RTL8366S_MIB_END_ADDRESS)
	{
		virtualMIBs = reg;
		
		reg = (reg & 0x0003) + RTL8366S_MIB_READ_VIRTUAL_ADDR;

		Rtl8366sVirtualReg[reg] = virtualMIBs;
	}
	else if(reg >= RTL8366S_VIRTUAL_REG_SIZE)
		return ERRNO_REGUNDEFINE;

	*value = Rtl8366sVirtualReg[reg];
#else
	uint32 regData;
	int32 retVal;

	retVal = smi_read(reg, &regData);
	if (retVal != SUCCESS) return ERRNO_SMIERROR;
	*value = regData;

#endif
	return SUCCESS;
}

/*
@func int32 | rtl8366s_setAsicReg | Set content of asic register.
@parm uint32 | reg | Register's address.
@parm uint32 | value | Value setting to register.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@comm
	The value will be set to ASIC mapping address only and it is always return SUCCESS while setting un-mapping address registers.
	
*/
int32 rtl8366s_setAsicReg(uint32 reg, uint32 value)
{

#if defined(RTK_X86_ASICDRV)//RTK-CNSD2-NickWu-20061222: for x86 compile

	int32 retVal;

	retVal = RTL_SINGLE_WRITE(reg, value);
	if (retVal != TRUE) return ERRNO_SMIERROR;
#ifdef RTK_DEBUG
	PRINT("set reg=0x%4.4x data=0x%4.4x\n",reg,value);
#endif

#elif defined(CONFIG_RTL8366S_ASICDRV_TEST)
	/*MIBs emulating*/
	if(reg == RTL8366S_MIB_CTRL_REG)
	{
		reg = RTL8366S_MIB_CTRL_VIRTUAL_ADDR;
	}	
	else if(reg >= RTL8366S_MIB_START_ADDRESS && reg <= RTL8366S_MIB_END_ADDRESS)
	{
		reg = (reg & 0x0003) + RTL8366S_MIB_READ_VIRTUAL_ADDR;
	}
	else if(reg >= RTL8366S_VIRTUAL_REG_SIZE)
		return ERRNO_REGUNDEFINE;

	Rtl8366sVirtualReg[reg] = value;
#else
	int32 retVal;

	retVal = smi_write(reg, value);
	if (retVal != SUCCESS) return ERRNO_SMIERROR;

#endif
	return SUCCESS;
}

/*=======================================================================
 * 2. Port property APIs
 *========================================================================*/


/*=======================================================================
 *  ACL APIs
 *========================================================================*/


/*=======================================================================
 *  Reserved Multicast Address APIs
 *========================================================================*/

/*=======================================================================
 *  C-VLAN APIs
 *========================================================================*/
/*
@func int32 | rtl8366s_setAsicVlan | Set VLAN enable function.
@parm uint32 | enabled | VLAN enable function usage 1: enabled, 0: disabled. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@rvalue ERRNO_INVALIDINPUT | Invalid input parameter.
@comm
	ASIC will parse frame type/length if VLAN function usage is enabled. In 802.1q spec. ,the Type/Length of C-tag is 0x8100. System will decide
	802.1q VID of received frame from C-tag, Protocol-and-Port based VLAN and Port based VLAN. This setting will impact on VLAN ingress, VLAN egress
	and 802.1q priority selection.
	
*/
int32 rtl8366s_setAsicVlan(uint32 enabled)
{
	uint32 retVal;

	retVal = rtl8366s_setAsicRegBit(RTL8366S_SWITCH_GLOBAL_CTRL_REG,RTL8366S_EN_VLAN_BIT,enabled);
	
	return retVal;
}

/*
@func int32 | rtl8366s_getAsicVlan | Get VLAN enable function configuration.
@parm uint32* | enabled | VLAN enable function usage 1: enabled, 0: disabled. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@rvalue ERRNO_INVALIDINPUT | Invalid input parameter.
@comm
	The API can get usage of asic VLAN enable configuration. 
	
*/
int32 rtl8366s_getAsicVlan(uint32* enabled)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8366s_getAsicReg(RTL8366S_SWITCH_GLOBAL_CTRL_REG,&regData);
	
	if(retVal !=  SUCCESS)
		return retVal;

	regData = regData & (1<<RTL8366S_EN_VLAN_BIT);

	if(regData)
		*enabled = 1;
	else
		*enabled = 0;

	return SUCCESS;	
}

/*
@func int32 | rtl8366s_setAsicVlan4kTbUsage | Set 4k VLAN table usage configuration.
@parm uint32 | enabled | 4k VLAN table usage 1: enabled, 0: disabled. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@rvalue ERRNO_INVALIDINPUT | Invalid input parameter.
@comm
	Each VLAN entry contains member port, untag set and FID (support 8 SVL/IVL filtering database) information. While VLAN function of system 
	is enabled and 4k VLAN table is enabled, system will decide each receiving frame's VID. VLAN ingress and VLAN egress function will 
	reference member port of mapped VID entry in 4k table. Without 4k VLAN table usage, there are 16 VLAN memeber configurations to	support
	VLAN enabled reference.
	 
*/
int32 rtl8366s_setAsicVlan4kTbUsage(uint32 enabled)
{
	uint32 retVal;

	retVal = rtl8366s_setAsicRegBit(RTL8366S_VLAN_TB_CTRL_REG,RTL8366S_VLAN_TB_BIT,enabled);

	return retVal;
}

/*
@func int32 | rtl8366s_getAsicVlan4kTbUsage | Get 4k VLAN table usage configuration.
@parm uint32* | enabled | 4k VLAN table usage 1: enabled, 0: disabled. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@rvalue ERRNO_INVALIDINPUT | Invalid input parameter.
@comm
	The API can get 4K VLAN table usage.
	
*/
int32 rtl8366s_getAsicVlan4kTbUsage(uint32* enabled)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8366s_getAsicReg(RTL8366S_VLAN_TB_CTRL_REG,&regData);

	if(retVal !=  SUCCESS)
		return retVal;

	regData = regData & RTL8366S_VLAN_TB_MSK;

	if(regData)
		*enabled = 1;
	else
		*enabled = 0;

	return SUCCESS;
}

/*
@func int32 | rtl8366s_setAsicVlanMemberConfig | Set 16 VLAN member configurations.
@parm uint32 | index | VLAN member configuration index (0~15).
@parm rtl8366s_vlanconfig* | vlanmemberconf | VLAN member configuration. It contained VID, priority, member set, untag set and FID fields. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@rvalue ERRNO_INVALIDINPUT | Invalid input parameter.
@rvalue ERRNO_VLAN_INVALIDFID | Invalid FID (0~7).
@rvalue ERRNO_VLAN_INVALIDPRIORITY | Invalid VLAN priority (0~7).
@rvalue ERRNO_VLAN_INVALIDPORTMSK | Invalid port mask (0x00~0x3F).
@rvalue ERRNO_VLAN_INVALIDVID | Invalid VID parameter (0~4095).
@rvalue ERRNO_VLAN_INVALIDMBRCONFIDX | Invalid VLAN member configuration index (0~15).
@comm
	VLAN ingress and egress will reference these 16 configurations while system VLAN function is enabled without 4k VLAN table usage. Port based
	, Protocol-and-Port based VLAN and 802.1x guest VLAN functions retrieved VLAN information from these 16 member configurations too. Only
	VID will be referenced while 4k VLAN table is enabled. It means that member set, untag set and FID need to be retrieved from 4k mapped VID entry.
	
*/
int32 rtl8366s_setAsicVlanMemberConfig(uint32 index,rtl8366s_vlanconfig *vlanmconf )
{
	uint32 retVal;
	uint32 regAddr;
	uint32 regData;
	uint16* tableAddr;
	
	if(index > RTL8366S_VLANMCIDXMAX)
		return ERRNO_VLAN_INVALIDMBRCONFIDX;

	if(vlanmconf->vid > RTL8366S_VIDMAX)
		return ERRNO_VLAN_INVALIDVID;

	if(vlanmconf->priority> RTL8366S_PRIORITYMAX)
		return ERRNO_VLAN_INVALIDPRIORITY;

	if(vlanmconf->member > RTL8366S_PORTMASK)
		return ERRNO_VLAN_INVALIDPORTMSK;

	if(vlanmconf->untag> RTL8366S_PORTMASK)
		return ERRNO_VLAN_INVALIDPORTMSK;

	if(vlanmconf->fid > RTL8366S_FIDMAX)
		return ERRNO_VLAN_INVALIDFID;

	regAddr = RTL8366S_VLAN_MEMCONF_BASE + (index<<1);


	tableAddr = (uint16*)vlanmconf;
	regData = *tableAddr;

	retVal = rtl8366s_setAsicReg(regAddr,regData);
	if(retVal !=  SUCCESS)
		return retVal;

	
	regAddr = RTL8366S_VLAN_MEMCONF_BASE + 1 + (index<<1);

	tableAddr ++;
	regData = *tableAddr;

	retVal = rtl8366s_setAsicReg(regAddr,regData);
	if(retVal !=  SUCCESS)
		return retVal;
	
	return SUCCESS;
}

/*
@func int32 | rtl8366s_getAsicVlanMemberConfig | Get 16 VLAN member configurations.
@parm uint32 | index | VLAN member configuration index (0~15).
@parm rtl8366s_vlanconfig* | vlanmemberconf | VLAN member configuration. It contained VID, priority, member set, untag set and FID fields. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@rvalue ERRNO_INVALIDINPUT | Invalid input parameter.
@rvalue ERRNO_VLAN_INVALIDMBRCONFIDX | Invalid VLAN member configuration index (0~15).
@comm
	The API can get 16 VLAN member configuration.
	
*/
int32 rtl8366s_getAsicVlanMemberConfig(uint32 index,rtl8366s_vlanconfig *vlanmconf )
{
	uint32 retVal;
	uint32 regAddr;
	uint32 regData;
	uint16* tableAddr;

	if(index > RTL8366S_VLANMCIDXMAX)
		return ERRNO_VLAN_INVALIDMBRCONFIDX;

	tableAddr = (uint16*)vlanmconf;
	
	regAddr = RTL8366S_VLAN_MEMCONF_BASE + (index<<1);

	retVal = rtl8366s_getAsicReg(regAddr,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	*tableAddr = regData;
	tableAddr ++;
	
		
	regAddr = RTL8366S_VLAN_MEMCONF_BASE + 1 + (index<<1);

	retVal = rtl8366s_getAsicReg(regAddr,&regData);
	if(retVal !=  SUCCESS)
		return retVal;
	
	*tableAddr = regData;

	return SUCCESS;
}

/*
@func int32 | rtl8366s_setAsicVlan4kEntry | Set VID mapped entry to 4K VLAN table.
@parm rtl8366s_vlan4kentry* | vlan4kEntry | VLAN entry seting for 4K table. There is VID field in entry structure and  entry is directly mapping to 4K table location (1 to 1).
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@rvalue ERRNO_INVALIDINPUT | Invalid input parameter.
@rvalue ERRNO_VLAN_INVALIDFID | Invalid FID (0~7).
@rvalue ERRNO_VLAN_INVALIDPORTMSK | Invalid port mask (0x00~0x3F).
@rvalue ERRNO_VLAN_INVALIDVID | Invalid VID parameter (0~4095).
@comm
	VID field of C-tag is 12-bits and available VID range is 0~4095. In 802.1q spec. , null VID (0x000) means tag header contain priority information
	only and VID 0xFFF is reserved for implementtation usage. But ASIC still retrieved these VID entries in 4K VLAN table if VID is decided from 16
	member configurations. It has no available VID 0x000 and 0xFFF from C-tag. ASIC will retrieve these two non-standard VIDs (0x000 and 0xFFF) from 
	member configuration indirectly referenced by Port based, Protocol-and-Port based VLAN and 802.1x functions.
	
*/
int32 rtl8366s_setAsicVlan4kEntry(rtl8366s_vlan4kentry vlan4kEntry )
{
	uint32 retVal;
	uint32 regData;
	uint16* tableAddr;

	if(vlan4kEntry.vid > RTL8366S_VIDMAX)
		return ERRNO_VLAN_INVALIDVID;

	if(vlan4kEntry.member > RTL8366S_PORTMASK)
		return ERRNO_VLAN_INVALIDPORTMSK;

	if(vlan4kEntry.untag> RTL8366S_PORTMASK)
		return ERRNO_VLAN_INVALIDPORTMSK;

	if(vlan4kEntry.fid > RTL8366S_FIDMAX)
		return ERRNO_VLAN_INVALIDFID;

	tableAddr = (uint16*)&vlan4kEntry;

	regData = *tableAddr;
	
	retVal = rtl8366s_setAsicReg(RTL8366S_VLAN_TABLE_WRITE_BASE,regData);
	if(retVal !=  SUCCESS)
		return retVal;

	tableAddr ++;

	regData = *tableAddr;
	
	retVal = rtl8366s_setAsicReg(RTL8366S_VLAN_TABLE_WRITE_BASE+1,regData);

	if(retVal !=  SUCCESS)
		return retVal;
	
	/*write table access Control word*/
	retVal = rtl8366s_setAsicReg(RTL8366S_TABLE_ACCESS_CTRL_REG,RTL8366S_TABLE_VLAN_WRITE_CTRL);
	if(retVal !=  SUCCESS)
		return retVal;

#ifdef CONFIG_RTL8366S_ASICDRV_TEST
	Rtl8366sVirtualVlanTable[vlan4kEntry.vid] = vlan4kEntry;
#endif
	return SUCCESS;
}

/*
@func int32 | rtl8366s_getAsicVlan4kEntry | Get VID mapped entry to 4K VLAN table. 
@parm rtl8366s_vlan4kentry* | vlan4kEntry | VLAN entry seting for 4K table. There is VID field in entry structure and  entry is directly mapping to 4K table location (1 to 1).
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@rvalue ERRNO_INVALIDINPUT | Invalid input parameter.
@rvalue ERRNO_VLAN_INVALIDVID | Invalid VID parameter (0~4095).
@comm
	The API can get entry of 4k VLAN table. Software must prepare the retrieving VID first at writing data and used control word to access desired VLAN entry.
	
*/
int32 rtl8366s_getAsicVlan4kEntry(rtl8366s_vlan4kentry *vlan4kEntry )
{
	uint32 retVal;
	uint32 regData;
	uint32 vid;
	uint16* tableAddr;

	vid = vlan4kEntry->vid;
	
	if(vid > RTL8366S_VIDMAX)
		return ERRNO_VLAN_INVALIDVID;

	tableAddr = (uint16*)vlan4kEntry;


	/*write VID first*/
	regData = *tableAddr;	
	retVal = rtl8366s_setAsicReg(RTL8366S_VLAN_TABLE_WRITE_BASE,regData);

	if(retVal !=  SUCCESS)
		return retVal;

	/*write table access Control word*/
	retVal = rtl8366s_setAsicReg(RTL8366S_TABLE_ACCESS_CTRL_REG,RTL8366S_TABLE_VLAN_READ_CTRL);
	if(retVal !=  SUCCESS)
		return retVal;

#ifdef CONFIG_RTL8366S_ASICDRV_TEST

	*(rtl8366s_vlan4kentry *)&Rtl8366sVirtualReg[RTL8366S_VLAN_TABLE_READ_BASE] = Rtl8366sVirtualVlanTable[vid];

#endif

	retVal = rtl8366s_getAsicReg(RTL8366S_VLAN_TABLE_READ_BASE,&regData);

	if(retVal !=  SUCCESS)
		return retVal;

	*tableAddr = regData;
	tableAddr ++;

	
	retVal = rtl8366s_getAsicReg(RTL8366S_VLAN_TABLE_READ_BASE+1,&regData);

	if(retVal !=  SUCCESS)
		return retVal;

	*tableAddr = regData;

	vlan4kEntry->vid = vid;
	
	return SUCCESS;
}


/*
@func int32 | rtl8366s_setAsicVlanPortBasedVID | Set port based VID which is indexed to 16 VLAN member configurations.
@parm enum PORTID | port | Physical port number (0~5).
@parm uint32 | index | Index to VLAN member configuration (0~15).
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@rvalue ERRNO_VLAN_INVALIDMBRCONFIDX | Invalid VLAN member configuration index (0~15).
@rvalue ERRNO_VLAN_INVALIDPORT | Invalid port number.
@comm
	In port based VLAN, untagged packets recieved by port N are forwarded to a VLAN according to the setting VID of port N. Usage of VLAN 4k table is enabled
	and there are only VID and 802.1q priority retrieved from 16 member configurations . Member set, untag set and FID of port based VLAN are be retrieved from 
	4K mapped VLAN entry.
	
*/
int32 rtl8366s_setAsicVlanPortBasedVID(enum PORTID port, uint32 index)
{
	int32 retVal;
	uint32 regAddr;
	uint32 regData;
	uint32 regBits;

	/* bits mapping to port vlan control register of port n */
	const uint16 bits[6]= { 0x000F,0x00F0,0x0F00,0xF000,0x000F,0x00F0 };
	/* bits offset to port vlan control register of port n */
	const uint16 bitOff[6] = { 0,4,8,12,0,4 };
	/* address offset to port vlan control register of port n */
	const uint16 addrOff[6]= { 0,0,0,0,1,1 };

	if(port >=PORT_MAX)
		return ERRNO_VLAN_INVALIDPORT;


	if(index > RTL8366S_VLANMCIDXMAX)
		return ERRNO_VLAN_INVALIDMBRCONFIDX;

	regAddr = RTL8366S_PORT_VLAN_CTRL_BASE + addrOff[port];

	regBits = bits[port];

	regData =  (index << bitOff[port]) & regBits;

	retVal = rtl8366s_setAsicRegBits(regAddr,regBits,regData);		
	
	return retVal;
}

/*
@func int32 | rtl8366s_getAsicVlanPortBasedVID | Get port based VID which is indexed to 16 VLAN member configurations.
@parm enum PORTID | port | Physical port number (0~5).
@parm uint32* | index | Index to VLAN member configuration (0~15).
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@rvalue ERRNO_VLAN_INVALIDPORT | Invalid port number.
@comm
	The API can access port based VLAN index indirectly retrieving VID and priority from 16 member configuration for a specific port.
	
*/
int32 rtl8366s_getAsicVlanPortBasedVID(enum PORTID port, uint32* index)
{
	int32 retVal;
	uint32 regAddr;
	uint32 regData;

	/* bits mapping to port vlan control register of port n */
	const uint16 bits[6]= { 0x000F,0x00F0,0x0F00,0xF000,0x000F,0x00F0 };
	/* bits offset to port vlan control register of port n */
	const uint16 bitOff[6] = { 0,4,8,12,0,4 };
	/* address offset to port vlan control register of port n */
	const uint16 addrOff[6]= { 0,0,0,0,1,1 };


	if(port >=PORT_MAX)
		return ERRNO_VLAN_INVALIDPORT;


	regAddr = RTL8366S_PORT_VLAN_CTRL_BASE + addrOff[port];

	retVal = rtl8366s_getAsicReg(regAddr,&regData);
	if(retVal != SUCCESS)
		return retVal;

	*index =  (regData & bits[port]) >> bitOff[port];
	return retVal;
}


/*=======================================================================
 *  L2 LUT & CAM APIs
 *========================================================================*/


/*=======================================================================
 *  VLAN stacking APIs
 *========================================================================*/

/*=======================================================================
 *  Port Mirroring APIs
 *========================================================================*/


/*=======================================================================
 *  Security APIs
 *========================================================================*/

/*=======================================================================
 *PHY APIs 
 *========================================================================*/

/*=======================================================================
 * ASIC DRIVER API: Packet Scheduling Control Register 
 *========================================================================*/

/*=======================================================================
  * ASIC DRIVER API: Remarking Control Register 
 *========================================================================*/

/*=======================================================================
  * ASIC DRIVER API: Priority Assignment Control Register 
 *========================================================================*/


/*
@func int32 | rtl8366s_setAsicLedIndicateInfoConfig | Set Leds indicated information mode
@parm uint32 | ledNo | LED group number. There are 1 to 1 led mapping to each port in each led group. 
@parm enum RTL8366S_LEDCONF | config | Support 16 types configuration.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@comm
	The API can set LED indicated information configuration for each LED group with 1 to 1 led mapping to each port.
	Definition		LED Statuses			Description
	0000		LED_Off				LED pin Tri-State.
	0001		Dup/Col				Collision, Full duplex Indicator. Blinking every 43ms when collision happens. Low for full duplex, and high for half duplex mode.
	0010		Link/Act				Link, Activity Indicator. Low for link established. Link/Act Blinks every 43ms when the corresponding port is transmitting or receiving.
	0011		Spd1000				1000Mb/s Speed Indicator. Low for 1000Mb/s.
	0100		Spd100				100Mb/s Speed Indicator. Low for 100Mb/s.
	0101		Spd10				10Mb/s Speed Indicator. Low for 10Mb/s.
	0110		Spd1000/Act			1000Mb/s Speed/Activity Indicator. Low for 1000Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
	0111		Spd100/Act			100Mb/s Speed/Activity Indicator. Low for 100Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
	1000		Spd10/Act			10Mb/s Speed/Activity Indicator. Low for 10Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
	1001		Spd100 (10)/Act		10/100Mb/s Speed/Activity Indicator. Low for 10/100Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
	1010		Fiber				Fiber link Indicator. Low for Fiber.
	1011		Fault	Auto-negotiation 	Fault Indicator. Low for Fault.
	1100		Link/Rx				Link, Activity Indicator. Low for link established. Link/Rx Blinks every 43ms when the corresponding port is transmitting.
	1101		Link/Tx				Link, Activity Indicator. Low for link established. Link/Tx Blinks every 43ms when the corresponding port is receiving.
	1110		Master				Link on Master Indicator. Low for link Master established.
	1111		LED_Force			Force LED output, LED output value reference 
 */
int32 rtl8366s_setAsicLedIndicateInfoConfig(uint32 ledNo, enum RTL8366S_LEDCONF config)
{
	uint32 retVal, regValue;

	if(ledNo >=RTL8366S_LED_GROUP_MAX)
		return ERRNO_INVALIDINPUT;

	if(config > LEDCONF_LEDFORCE)	
		return ERRNO_INVALIDINPUT;


	/* Get register value */
	retVal = rtl8366s_getAsicReg(RTL8366S_LED_INDICATED_CONF_REG, &regValue); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	regValue =  (regValue & (~(0xF<<(ledNo*4)))) | (config<<(ledNo*4));

	
	retVal = rtl8366s_setAsicReg(RTL8366S_LED_INDICATED_CONF_REG, regValue); 	

	return retVal;
}

/*
@func int32 | rtl8366s_getAsicLedIndicateInfoConfig | Get Leds indicated information mode
@parm uint32 | ledNo | LED group number. There are 1 to 1 led mapping to each port in each led group.
@parm enum RTL8366S_LEDCONF* | config | Support 16 types configuration.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@comm
        The API can get LED indicated information configuration for each LED group.
 */
int32 rtl8366s_getAsicLedIndicateInfoConfig(uint32 ledNo, enum RTL8366S_LEDCONF* config)
{
        uint32 retVal, regValue;

        if(ledNo >=RTL8366S_LED_GROUP_MAX)
                return ERRNO_INVALIDINPUT;

        /* Get register value */
        retVal = rtl8366s_getAsicReg(RTL8366S_LED_INDICATED_CONF_REG, &regValue);
        if (retVal !=  SUCCESS)
                return retVal;

        *config = (regValue >> (ledNo*4)) & 0x000F;

        return SUCCESS;
}


/*
@func int32 | rtl8366s_setAsicForceLeds | Turn on/off Led of dedicated port
@parm uint32 | ledG0Msk | Turn on or turn off Leds of group 0, 1:on 0:off.
@parm uint32 | ledG1Msk | Turn on or turn off Leds of group 1, 1:on 0:off.
@parm uint32 | ledG2Msk | Turn on or turn off Leds of group 2, 1:on 0:off.
@parm uint32 | ledG3Msk | Turn on or turn off Leds of group 3, 1:on 0:off.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@comm
	The API can turn on/off desired Led group of dedicated port while indicated information configuration of LED group is set to force mode.
 */
int32 rtl8366s_setAsicForceLeds(uint32 ledG0Msk, uint32 ledG1Msk, uint32 ledG2Msk, uint32 ledG3Msk)
{
	uint32 retVal, regValue;

	regValue = (ledG0Msk & 0x3F) | (ledG1Msk&0x3F) << 6;

	retVal = rtl8366s_setAsicReg(RTL8366S_LED_0_1_FORCE_REG, regValue); 	
	if(retVal != SUCCESS)
		return retVal;

	regValue = (ledG2Msk & 0x3F) | (ledG3Msk&0x3F) << 6;
	retVal = rtl8366s_setAsicReg(RTL8366S_LED_2_3_FORCE_REG, regValue); 	
	
	return retVal;
}

