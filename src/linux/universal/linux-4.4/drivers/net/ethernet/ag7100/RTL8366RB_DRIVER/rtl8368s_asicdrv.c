/*
* Copyright c                  Realtek Semiconductor Corporation, 2006 
* All rights reserved.
* 
* Program : RTL8366S switch low-level API
* Abstract : 
* Author :Abel Hsu(abelshie@realtek.com.tw)                
*  $Id: rtl8368s_asicdrv.c,v 1.26 2008-06-11 08:51:37 hmchung Exp $
*/
/*	@doc RTL8368S_ASICDRIVER_API

	@module Rtl8366S_AsicDrv.c - RTL8366S Switch asic driver API documentation	|
	This document explains API interface of the asic. 
	@normal 

	Copyright <cp>2007 Realtek<tm> Semiconductor Cooperation, All Rights Reserved.

 	@head3 List of Symbols |
 	Here is a list of all functions and variables in this module. 

 	@index | RTL8368S_ASICDRIVER_API
*/
#include "rtl8368s_types.h"
#include "rtl8368s_asicdrv.h"
#include "rtl8368s_errno.h"
#include "smi.h" 
#include "rtl8368s_reg.h"




/*for driver verify testing only*/
#ifdef CONFIG_RTL8368S_ASICDRV_TEST
#define RTL8368S_VIRTUAL_REG_SIZE		0x2000

uint16 Rtl8368sVirtualReg[RTL8368S_VIRTUAL_REG_SIZE];
rtl8368s_smiacltable Rtl8368sVirtualAclTable[RTL8368S_ACLRULENO];
rtl8368s_vlan4kentry Rtl8368sVirtualVlanTable[RTL8368S_VIDMAX + 1];
rtl8368s_l2smitable Rtl8368sVirtualL2Table[RTL8368S_L2ENTRYMAX + 1];
rtl8368s_camsmitable Rtl8368sVirtualCAMTable[RTL8368S_CAMENTRYMAX + 1];

uint64 Rtl8368sVirtualMIBsReadData;
uint64 Rtl8368sVirtualMIBsControlReg;

#define RTL8368S_MIB_START_ADDRESS				0x1000
#define RTL8368S_MIB_END_ADDRESS					0x1280

#define RTL8368S_MIB_CTRL_VIRTUAL_ADDR			0x4FF
#define RTL8368S_MIB_READ_VIRTUAL_ADDR			0x4F0

#define RTL8368S_PHY_VIRTUAL_ADDR				0x1000

#endif

#if defined(CONFIG_RTL865X_CLE) || defined (RTK_X86_CLE)
uint32 cleDebuggingDisplay;
#endif
/*
*	reverse MAC order: 11:22:33:44:55:66 -> 66:55:44:33:22:11
*/
static void _rtl8368s_MACReverse(ether_addr_t *mac)
{
	uint8 BYTE1;
	uint8 BYTE2;
	uint8 BYTE3;	
	uint8 BYTE4;
	uint8 BYTE5;
	uint8 BYTE6;

	BYTE1 = mac->octet[0];
	BYTE2 = mac->octet[1];
	BYTE3 = mac->octet[2];
	BYTE4 = mac->octet[3];
	BYTE5 = mac->octet[4];
	BYTE6 = mac->octet[5];			
	mac->octet[0] = BYTE6;
	mac->octet[1] = BYTE5;
	mac->octet[2] = BYTE4;
	mac->octet[3] = BYTE3;
	mac->octet[4] = BYTE2;
	mac->octet[5] = BYTE1;
}

/*
*	reverse IP order: 1.2.3.4 -> 4.3.2.1
*/
static void _rtl8368s_IpReverse(ipaddr_t *Ip)
{
	uint8 BYTE1;
	uint8 BYTE2;
	uint8 BYTE3;
	uint8 BYTE4;

	BYTE1 = (*Ip & 0xFF000000)>>24;
	BYTE2 = (*Ip & 0x00FF0000)>>16;
	BYTE3 = (*Ip & 0x0000FF00)>>8;
	BYTE4 = (*Ip & 0x000000FF);
	*Ip = BYTE1 | (BYTE2<<8) | (BYTE3<<16) | (BYTE4<<24);
}

/*=======================================================================
 * 1. Asic read/write driver through SMI
 *========================================================================*/
/*
@func int32 | rtl8368s_setAsicRegBit | Set a bit value of a specified register.
@parm uint32 | reg | Register's address.
@parm uint32 | bit | Bit location. For 16-bits register only. Maximun value is 15 for MSB location.
@parm uint32 | value | Value to set. It can be value 0 or 1.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue TBLDRV_EINVALIDINPUT | Invalid input parameter. 
@comm
	Set a bit of a specified register to 1 or 0. It is 16-bits system of RTL8366s chip.
	
*/
int32 rtl8368s_setAsicRegBit(uint32 reg, uint32 bit, uint32 value)
{

#if defined(RTK_X86_ASICDRV)
	uint32 regData;
	int32 retVal;
	
	if(bit>=RTL8368S_REGBITLENGTH)
		return ERRNO_INPUT;

	retVal = RTL_SINGLE_READ(reg, 2, &regData);
	if (retVal != TRUE) return ERRNO_SMI;

	if(0x8368 == cleDebuggingDisplay)
		PRINT("R:[0x%4.4x]=0x%4.4x\n",reg,regData);

	if (value) 
		regData = regData | (1<<bit);
	else
		regData = regData & ~(1<<bit);

	retVal = RTL_SINGLE_WRITE(reg, regData);
	if (retVal != TRUE) return ERRNO_SMI;

	if(0x8368 == cleDebuggingDisplay)
		PRINT("W:[0x%4.4x]=0x%4.4x\n",reg,regData);

	
#elif defined(CONFIG_RTL8368S_ASICDRV_TEST)

	/*MIBs emulating*/
	if(reg == RTL8368S_MIB_CTRL_REG)
	{
		reg = RTL8368S_MIB_CTRL_VIRTUAL_ADDR;
	}	
	else if(reg >=RTL8368S_PHY_ACCESS_CTRL_REG)
	{
		reg = reg -RTL8368S_PHY_ACCESS_CTRL_REG + RTL8368S_PHY_VIRTUAL_ADDR;
	}
	else if(bit>=RTL8368S_REGBITLENGTH)
		return ERRNO_INPUT;
	else if(reg >= RTL8368S_VIRTUAL_REG_SIZE)
		return ERRNO_REGUNDEFINE;

	if (value) 
	{
		Rtl8368sVirtualReg[reg] =  Rtl8368sVirtualReg[reg] | (1<<bit);

	}
	else
	{
		Rtl8368sVirtualReg[reg] =  Rtl8368sVirtualReg[reg] & (~(1<<bit));
	}
	
	if(0x8368 == cleDebuggingDisplay)
		PRINT("--[0x%4.4x]=0x%4.4x\n",reg,Rtl8368sVirtualReg[reg]);

	
#else
	uint32 regData;
	int32 retVal;
	
	if(bit>=RTL8368S_REGBITLENGTH)
		return ERRNO_INPUT;

	retVal = smi_read(reg, &regData);
	if (retVal != SUCCESS) return ERRNO_SMI;

#ifdef CONFIG_RTL865X_CLE
	if(0x8368 == cleDebuggingDisplay)
		PRINT("R:[0x%4.4x]=0x%4.4x\n",reg,regData);
#endif

	if (value) 
		regData = regData | (1<<bit);
	else
		regData = regData & (~(1<<bit));
	
	retVal = smi_write(reg, regData);
	if (retVal != SUCCESS) return ERRNO_SMI;

#ifdef CONFIG_RTL865X_CLE
	if(0x8368 == cleDebuggingDisplay)
		PRINT("W:[0x%4.4x]=0x%4.4x\n",reg,regData);
#endif

#endif
	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicRegBits | Set bits value of a specified register.
@parm uint32 | reg | Register's address.
@parm uint32 | bits | Bits mask for setting. 
@parm uint32 | value | Bits value for setting. Value of bits will be set with mapping mask bit is 1.   
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue TBLDRV_EINVALIDINPUT | Invalid input parameter. 
@comm
	Set bits of a specified register to value. Both bits and value are be treated as bit-mask.
	
*/
int32 rtl8368s_setAsicRegBits(uint32 reg, uint32 bits, uint32 value)
{
	
#if defined(RTK_X86_ASICDRV)//RTK-CNSD2-NickWu-20061222: for x86 compile

	uint32 regData;
	int32 retVal;
	
	if(bits>= (1<<RTL8368S_REGBITLENGTH) )
		return ERRNO_INPUT;

	retVal = RTL_SINGLE_READ(reg, 2, &regData);
	if (retVal != TRUE) return ERRNO_SMI;
	if(0x8368 == cleDebuggingDisplay)
		PRINT("R:[0x%4.4x]=0x%4.4x\n",reg,regData);

	regData = regData & (~bits);
	regData = regData | (value & bits);

	retVal = RTL_SINGLE_WRITE(reg, regData);
	if (retVal != TRUE) return ERRNO_SMI;
	if(0x8368 == cleDebuggingDisplay)
		PRINT("W:[0x%4.4x]=0x%4.4x\n",reg,regData);
	
#elif defined(CONFIG_RTL8368S_ASICDRV_TEST)
	uint32 regData;

	/*MIBs emulating*/
	if(reg == RTL8368S_MIB_CTRL_REG)
	{
		reg = RTL8368S_MIB_CTRL_VIRTUAL_ADDR;
	}	
	else if(reg >=RTL8368S_PHY_ACCESS_CTRL_REG)
	{
		reg = reg -RTL8368S_PHY_ACCESS_CTRL_REG + RTL8368S_PHY_VIRTUAL_ADDR;
	}
	else if(reg >= RTL8368S_VIRTUAL_REG_SIZE)
		return ERRNO_REGUNDEFINE;

	regData = Rtl8368sVirtualReg[reg] & (~bits);
	regData = regData | (value & bits);
	
	Rtl8368sVirtualReg[reg] = regData;

	if(0x8368 == cleDebuggingDisplay)
		PRINT("--[0x%4.4x]=0x%4.4x\n",reg,regData);
	
#else
	uint32 regData;
	int32 retVal;
	
	if(bits>= (1<<RTL8368S_REGBITLENGTH) )
		return ERRNO_INPUT;

	retVal = smi_read(reg, &regData);
	if (retVal != SUCCESS) return ERRNO_SMI;

#ifdef CONFIG_RTL865X_CLE
	if(0x8368 == cleDebuggingDisplay)
		PRINT("R:[0x%4.4x]=0x%4.4x\n",reg,regData);
#endif

	regData = regData & (~bits);
	regData = regData | (value & bits);

	
	retVal = smi_write(reg, regData);
	if (retVal != SUCCESS) return ERRNO_SMI;

#ifdef CONFIG_RTL865X_CLE
	if(0x8368 == cleDebuggingDisplay)
		PRINT("W:[0x%4.4x]=0x%4.4x\n",reg,regData);
#endif

#endif
	return SUCCESS;
}

/*
@func int32 | rtl8368s_getAsicReg | Get content of register.
@parm uint32 | reg | Register's address.
@parm uint32* | value | Value of register.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@comm
 	Value 0x0000 will be returned for ASIC un-mapping address.
	
*/
int32 rtl8368s_getAsicReg(uint32 reg, uint32 *value)
{
	
#if defined(RTK_X86_ASICDRV)//RTK-CNSD2-NickWu-20061222: for x86 compile

	uint32 regData;
	int32 retVal;

	retVal = RTL_SINGLE_READ(reg, 2, &regData);
	if (retVal != TRUE) return ERRNO_SMI;

	*value = regData;
	
	if(0x8368 == cleDebuggingDisplay)
		PRINT("R:[0x%4.4x]=0x%4.4x\n",reg,regData);

#elif defined(CONFIG_RTL8368S_ASICDRV_TEST)
       uint16 virtualMIBs;

	/*MIBs emulating*/
	if(reg == RTL8368S_MIB_CTRL_REG)
	{
		reg = RTL8368S_MIB_CTRL_VIRTUAL_ADDR;
	}	
	else if(reg >= RTL8368S_MIB_START_ADDRESS && reg <= RTL8368S_MIB_END_ADDRESS)
	{
		virtualMIBs = reg;
		
		reg = (reg & 0x0003) + RTL8368S_MIB_READ_VIRTUAL_ADDR;

		Rtl8368sVirtualReg[reg] = virtualMIBs;
	}
	else if(reg >=RTL8368S_PHY_ACCESS_CTRL_REG)
	{
		reg = reg -RTL8368S_PHY_ACCESS_CTRL_REG + RTL8368S_PHY_VIRTUAL_ADDR;
	}
	else if(reg >= RTL8368S_VIRTUAL_REG_SIZE)
		return ERRNO_REGUNDEFINE;

	*value = Rtl8368sVirtualReg[reg];

	if(0x8368 == cleDebuggingDisplay)
		PRINT("--[0x%4.4x]=0x%4.4x\n",reg,Rtl8368sVirtualReg[reg]);
	
#else
	uint32 regData;
	int32 retVal;

	retVal = smi_read(reg, &regData);
	if (retVal != SUCCESS) return ERRNO_SMI;

	*value = regData;

#ifdef CONFIG_RTL865X_CLE
	if(0x8368 == cleDebuggingDisplay)
		PRINT("R:[0x%4.4x]=0x%4.4x\n",reg,regData);
#endif

#endif
	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicReg | Set content of asic register.
@parm uint32 | reg | Register's address.
@parm uint32 | value | Value setting to register.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@comm
	The value will be set to ASIC mapping address only and it is always return SUCCESS while setting un-mapping address registers.
	
*/
int32 rtl8368s_setAsicReg(uint32 reg, uint32 value)
{

#if defined(RTK_X86_ASICDRV)//RTK-CNSD2-NickWu-20061222: for x86 compile

	int32 retVal;

	retVal = RTL_SINGLE_WRITE(reg, value);
	if (retVal != TRUE) return ERRNO_SMI;

	if(0x8368 == cleDebuggingDisplay)
		PRINT("W:[0x%4.4x]=0x%4.4x\n",reg,value);

#elif defined(CONFIG_RTL8368S_ASICDRV_TEST)
	/*MIBs emulating*/
	if(reg == RTL8368S_MIB_CTRL_REG)
	{
		reg = RTL8368S_MIB_CTRL_VIRTUAL_ADDR;
	}	
	else if(reg >= RTL8368S_MIB_START_ADDRESS && reg <= RTL8368S_MIB_END_ADDRESS)
	{
		reg = (reg & 0x0003) + RTL8368S_MIB_READ_VIRTUAL_ADDR;
	}
	else if(reg >=RTL8368S_PHY_ACCESS_CTRL_REG)
	{
		reg = reg -RTL8368S_PHY_ACCESS_CTRL_REG + RTL8368S_PHY_VIRTUAL_ADDR;
	}
	else if(reg >= RTL8368S_VIRTUAL_REG_SIZE)
		return ERRNO_REGUNDEFINE;

	Rtl8368sVirtualReg[reg] = value;

	if(0x8368 == cleDebuggingDisplay)
		PRINT("--[0x%4.4x]=0x%4.4x\n",reg,Rtl8368sVirtualReg[reg]);

#else
	int32 retVal;

	retVal = smi_write(reg, value);
	if (retVal != SUCCESS) return ERRNO_SMI;

#ifdef CONFIG_RTL865X_CLE
	if(0x8368 == cleDebuggingDisplay)
		PRINT("W:[0x%4.4x]=0x%4.4x\n",reg,value);
#endif

#endif
	return SUCCESS;
}

/*=======================================================================
 * 2. Port property APIs
 *========================================================================*/

/*
@func int32 | rtl8368s_getAsicPortLinkState | Get a specific port's link state.
@parm enum PORTID | port | Physical Port number.
@parm enum PORTLINKSPEED  * | speed | current link speed, 
	SPD_10M: 10Mbps, 
	SPD_100M: 100Mbps, 
	SPD_1000M: 1000Mbps.
@parm enum PORTLINKDUPLEXMODE * | duplex | current duplex status,
	FULL_DUPLEX: full duplex, 
	HALF_DUPLEX: half duplex.
@parm uint32  * | link | Link status, 1: link up, 0: link down.
@parm uint32  * | rxPause | The pause frame response ability, 1: active, 0: inactive.
@parm uint32  * | txPause | The pause frame transmit ability, 1: active, 0: inactive.
@parm uint32  * | nWay | N-way function, 1: enable, 0: disable.
@rvalue SUCCESS | Success.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can get a specific port's link state information including link up/down, 
	link speed, auto negotiation enabled/disabled, Tx in active/inactive and Rx in 
	active/inactive. 
*/
int32 rtl8368s_getAsicPortLinkState(enum PORTID port, enum PORTLINKSPEED *speed, enum PORTLINKDUPLEXMODE *duplex,uint32 *link, uint32 *txPause,uint32 *rxPause,uint32 *nWay)
{
	int32 retVal;
	uint32 RegData;
	
	if(port <PORT0 || port >=PORT_MAX)
		return ERRNO_INPUT;

	retVal = rtl8368s_getAsicReg(RTL8368S_PORT_LINK_STATUS_BASE + (port >>1),&RegData);
	if(SUCCESS != retVal)	
		return FAILED;


	if(port&0x1)
	{
		RegData = RegData >> 8;
	}

	*speed = (RegData & RTL8368S_PORT_STATUS_SPEED_MSK)>>RTL8368S_PORT_STATUS_SPEED_OFF;
	*duplex = (RegData & RTL8368S_PORT_STATUS_DUPLEX_MSK)>>RTL8368S_PORT_STATUS_DUPLEX_OFF;
	*link = (RegData & RTL8368S_PORT_STATUS_LINK_MSK)>>RTL8368S_PORT_STATUS_LINK_OFF;
	*txPause = (RegData & RTL8368S_PORT_STATUS_TXPAUSE_MSK)>>RTL8368S_PORT_STATUS_TXPAUSE_OFF;
	*rxPause = (RegData & RTL8368S_PORT_STATUS_RXPAUSE_MSK)>>RTL8368S_PORT_STATUS_RXPAUSE_OFF;
	*nWay = (RegData & RTL8368S_PORT_STATUS_AN_MSK)>>RTL8368S_PORT_STATUS_AN_OFF;
	
	return SUCCESS;
}

/*=======================================================================
 *  ACL APIs
 *========================================================================*/
/*
@func int32 | rtl8368s_setAsicAcl | Set port acl function enable/disable.
@parm enum PORTID | port | Physical port number (0~5).
@parm uint32 | enabled | 1: enabled, 0: disabled.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error. 
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	ACL function of port was enabled and ASIC will check per port ACL rule start and rule end setting. There are 32 ACL rules for shared usage
	in RTL8366s and each port has dedicated rule start/stop index (0-31).
	
*/
int32 rtl8368s_setAsicAcl(enum PORTID port, uint32 enabled)
{
	int32 retVal;
	
	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	/*bitNo = port*2+1;*/
	retVal = rtl8368s_setAsicRegBit(RTL8368S_ACL_CONTROL_REG,port,enabled);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicAcl | Get port acl function enable/disable.
@parm enum PORTID | port | Physical port number (0~5).
@parm uint32* | enabled | 1: enabled, 0: disabled.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error. 
@rvalue ERRNO_PORT_NUM | Invalid port number.
@comm
	The API can be used to get ACL function enable setting of port.
	
*/
int32 rtl8368s_getAsicAcl(enum PORTID port, uint32* enabled)
{
	int32 retVal;
	uint32 RegData;
	
	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	retVal = rtl8368s_getAsicReg(RTL8368S_ACL_CONTROL_REG,&RegData);

	if(SUCCESS != retVal)	
		return FAILED;

	if(RegData &(1<<port))
		*enabled = 1;
	else
		*enabled = 0;

	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicAclUnmatchedPermit | Set port acl function unmatched permit action.
@parm enum PORTID | port | Physical port number (0~5).
@parm uint32 | enabled | 1: Enable, 0: Disable.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error. 
@rvalue ERRNO_ACL_INVALIDPORT | Invalid Port Number.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	If ACL function of reveiving port was enabled and receiving frame was unmathed to ACL start index rule to end index rule, then ASIC 
	will following this setting to permit unmatched frame (if port unmatched permit was enabled) or drop it directly.
	
*/
int32 rtl8368s_setAsicAclUnmatchedPermit(enum PORTID port, uint32 enabled)
{
	int32 retVal;
	
	if(port <PORT0 || port >=PORT_MAX)
		return ERRNO_ACL_INVALIDPORT;

	/*bitNo = port*2+1;*/
	retVal = rtl8368s_setAsicRegBit(RTL8368S_ACL_CONTROL_REG,port+RTL8368S_ACL_UNMA_PERMIT_OFF,enabled);

	return retVal;

}

/*
@func int32 | rtl8368s_getAsicAclUnmatchedPermit | Set port acl function unmatched permit action.
@parm enum PORTID | port | Physical port number (0~5).
@parm uint32* | enabled | 1: Enable, 0: Disable.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error. 
@rvalue ERRNO_PORT_NUM | Invalid port number.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	If ACL function of reveiving port was enabled and receiving frame was unmathed to ACL start index rule to end index rule, then ASIC 
	will following this setting to permit unmatched frame (if port unmatched permit was enabled) or drop it directly.
	
*/
int32 rtl8368s_getAsicAclUnmatchedPermit(enum PORTID port, uint32* enabled)
{

	int32 retVal;
	uint32 RegData;
	
	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	retVal = rtl8368s_getAsicReg(RTL8368S_ACL_CONTROL_REG,&RegData);

	if(SUCCESS != retVal)	
		return FAILED;

	if(RegData &(1<<(port+RTL8368S_ACL_UNMA_PERMIT_OFF)))
		*enabled = 1;
	else
		*enabled = 0;

	return SUCCESS;



}

/*
@func int32 | rtl8368s_setAsicAclMeterInterFrameGap | Set ACL meter receiving frame rate calculation include inter-frame gap or not.
@parm uint32 | enabled | 1: Enabled, 0: Disabled.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error. 
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	ACL meter frame rate calculation was referenced with inter-frame gap (96 bits) and has more impact on short length burst frames. 
*/
int32 rtl8368s_setAsicAclMeterInterFrameGap(uint32 enabled)
{

	int32 retVal;
	

	retVal = rtl8368s_setAsicRegBit(RTL8368S_ACL_CONTROL_1_REG,RTL8368S_ACL_INC_ING_OFF,enabled);

	return retVal;


}

/*
@func int32 | rtl8368s_getAsicAclMeterInterFrameGap | Set ACL meter receiving frame rate calculation include inter-frame gap or not.
@parm uint32* | enabled | 1: Enabled, 0: Disabled.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error. 
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	ACL meter frame rate calculation was referenced with inter-frame gap (96 bits) and has more impact on short length burst frames. 
*/
int32 rtl8368s_getAsicAclMeterInterFrameGap(uint32* enabled)
{

	int32 retVal;
	uint32 RegData;
	

	retVal = rtl8368s_getAsicReg(RTL8368S_ACL_CONTROL_1_REG,&RegData);

	if(SUCCESS != retVal)	
		return FAILED;

	if(RegData &(1<<(RTL8368S_ACL_INC_ING_OFF)))
		*enabled = 1;
	else
		*enabled = 0;

	return SUCCESS;


}

/*
@func int32 | rtl8368s_setAsicAclStartEnd | Set port acl function ingress rule start/end  index.
@parm enum PORTID | port | Physical port number (0~5).
@parm uint32 | aclStart | Per port ACL ingress START index of 32 shared rules.
@parm uint32 | aclEnd | Per port ACL ingress  END index of 32 shared rules.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error. 
@rvalue ERRNO_ACLIDX | Invalid ACL rule index (0~31)
@rvalue ERRNO_PORT_NUM | Invalid port number.
@comm
	ASIC will check receiving frame with per port ACL start to end index rule under port ACL function was enabled. 
	If port ACL start index N is great than ACL end index M, then ASIC will check rule from start index N, N+1, N+2,
	.... 31, 0, 1, ... to end index M. ACL shared rule number is 32.
	
*/
int32 rtl8368s_setAsicAclStartEnd(enum PORTID port, uint32 aclStart, uint32 aclEnd)
{
	uint32 regData;
	int32 retVal;
		
	if(port <PORT0 || port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	if(aclStart > RTL8368S_ACLINDEXMAX || aclEnd > RTL8368S_ACLINDEXMAX)
		return ERRNO_ACLIDX;
	

	regData = ((aclStart << RTL8368S_ACL_RANGE_START_OFF) & RTL8368S_ACL_RANGE_START_MSK) | ((aclEnd << RTL8368S_ACL_RANGE_END_OFF) & RTL8368S_ACL_RANGE_END_MSK);

		
	retVal = rtl8368s_setAsicReg(RTL8368S_ACL_RANGE_REG_BASE + port,regData);		

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicAclStartEnd | Get port acl function ingress rule start/end  index.
@parm enum PORTID | port | Physical port number (0~5).
@parm uint32* | aclStart | Per port ACL ingress START index of 32 share rules.
@parm uint32* | aclEnd | Per port ACL ingress  END index of 32 share rules.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error. 
@rvalue ERRNO_PORT_NUM | Invalid port number.
@comm
	The API can be used for getting ACL ingress index range of port.
*/
int32 rtl8368s_getAsicAclStartEnd(enum PORTID port, uint32 *aclStart, uint32 *aclEnd)
{
	uint32 regData;
	int32 retVal;


	if(port <PORT0 || port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	retVal = rtl8368s_getAsicReg(RTL8368S_ACL_RANGE_REG_BASE + port,&regData);
	if(retVal != SUCCESS)
		return retVal;

	*aclStart =  (regData & RTL8368S_ACL_RANGE_START_MSK) >> RTL8368S_ACL_RANGE_START_OFF;
	
	*aclEnd =  (regData & RTL8368S_ACL_RANGE_END_MSK) >> RTL8368S_ACL_RANGE_END_OFF;

	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicAclMeter | Set ACL meter average filtering rate and accept burst size.   
@parm uint32 | meterIdx | ACL meter Index (0-15).
@parm uint32 | aclmRate | Average packets rate allowed or redirect. Average packet rate of ACL meter is (aclmRate+1)*64Kbps.
@parm uint32 | aclmTole | Number of bytes setting for permit packets to pass. Maximun number of bytes permi packets to pass is 1KB*aclmTole.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error. 
@rvalue ERRNO_ACL_INVALIDMETERIDX | Invalid acl meter index
@comm
	Per port ACL ingress checking rule number is from START to END index. If receiving frame is matched multiple ACL rules and matched ACL rules 
	were set with the same meter, ASIC will calculate receiving frame rate multiple times as matched rule number. For example, 3 ACL rules were set
	by the same ACL meter with filtering rate 600Kpbps and ASIC will allow only 200Kbps output matched ACL frame 
*/
int32 rtl8368s_setAsicAclMeter(uint32 meterIdx,uint32 aclmRate, uint32 aclmTole)
{
	int32 retVal;
	uint32 regData;
	uint32 regAddr;

	if(meterIdx > RTL8368S_ACL_METER_IDX_MAX)
		return ERRNO_ACL_INVALIDMETERIDX;


	regAddr = RTL8368S_ACL_METER_BASE + (meterIdx<<1);
	regData = aclmRate & RTL8368S_ACL_METER_RATE_MSK;

	retVal = rtl8368s_setAsicReg(regAddr,regData);		

	if(retVal !=SUCCESS)
		return retVal;
	

	regData = aclmTole & RTL8368S_ACL_METER_TOLE_MSK;

	retVal = rtl8368s_setAsicReg(regAddr+1,regData);		

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicAclMeter | Get ACL meter average filtering rate and accept burst size.   
@parm uint32 | meterIdx | ACL meter Index (0-15).
@parm uint32* | aclmRate | Average packets rate allowed or redirect. Average packet rate of ACL meter is (aclmRate+1)*64Kbps.
@parm uint32* | aclmTole | Number of bytes setting for permit packets to pass. Maximun number of bytes permi packets to pass is 1KB*aclmTole.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error. 
@rvalue ERRNO_ACL_INVALIDMETERIDX | Invalid acl meter index
@comm
	The API can be used to get ACL meter setting. The shared ACL meter number is 16.
*/
int32 rtl8368s_getAsicAclMeter(uint32 meterIdx,uint32* aclmRate, uint32* aclmTole)
{
	int32 retVal;
	uint32 regData;
	uint32 regAddr;

	if(meterIdx > RTL8368S_ACL_METER_IDX_MAX)
		return ERRNO_ACL_INVALIDMETERIDX;


	regAddr = RTL8368S_ACL_METER_BASE + (meterIdx<<1);

	/*get ACL meter RATE*/
	retVal = rtl8368s_getAsicReg(regAddr, &regData);
	if(retVal !=SUCCESS)
		return retVal;
		
	*aclmRate = regData;	

	/*get ACL meter TOLE*/
	retVal = rtl8368s_getAsicReg(regAddr+1, &regData);
	*aclmTole = regData;	

	return retVal;

}

/*
	Exchange structure type define with MMI and SMI
*/
static void _rtl8368s_aclStSmi2User( rtl8368s_acltable *aclUser, rtl8368s_smiacltable *aclSmi)
{

	switch(aclSmi->format)
	{
	 case ACL_MAC:

	 	aclUser->rule.mac.dmp.octet[0] = aclSmi->rule.smi_mac.dmp0;
	 	aclUser->rule.mac.dmp.octet[1] = aclSmi->rule.smi_mac.dmp1;
	 	aclUser->rule.mac.dmp.octet[2] = aclSmi->rule.smi_mac.dmp2;
	 	aclUser->rule.mac.dmp.octet[3] = aclSmi->rule.smi_mac.dmp3;
	 	aclUser->rule.mac.dmp.octet[4] = aclSmi->rule.smi_mac.dmp4;
	 	aclUser->rule.mac.dmp.octet[5] = aclSmi->rule.smi_mac.dmp5;

		aclUser->rule.mac.dmm.octet[0]  = aclSmi->rule.smi_mac.dmm0;
		aclUser->rule.mac.dmm.octet[1]  = aclSmi->rule.smi_mac.dmm1;
		aclUser->rule.mac.dmm.octet[2]  = aclSmi->rule.smi_mac.dmm2;
		aclUser->rule.mac.dmm.octet[3]  = aclSmi->rule.smi_mac.dmm3;
		aclUser->rule.mac.dmm.octet[4]  = aclSmi->rule.smi_mac.dmm4;
		aclUser->rule.mac.dmm.octet[5]  = aclSmi->rule.smi_mac.dmm5;

	 	aclUser->rule.mac.smp.octet[0]  = aclSmi->rule.smi_mac.smp0;
	 	aclUser->rule.mac.smp.octet[1]  = aclSmi->rule.smi_mac.smp1;
	 	aclUser->rule.mac.smp.octet[2]  = aclSmi->rule.smi_mac.smp2;
	 	aclUser->rule.mac.smp.octet[3]  = aclSmi->rule.smi_mac.smp3;
	 	aclUser->rule.mac.smp.octet[4]  = aclSmi->rule.smi_mac.smp4;
	 	aclUser->rule.mac.smp.octet[5]  = aclSmi->rule.smi_mac.smp5;

	 	aclUser->rule.mac.smm.octet[0]  = aclSmi->rule.smi_mac.smm0;
 	 	aclUser->rule.mac.smm.octet[1]  = aclSmi->rule.smi_mac.smm1;
	 	aclUser->rule.mac.smm.octet[2]  = aclSmi->rule.smi_mac.smm2;
	 	aclUser->rule.mac.smm.octet[3]  = aclSmi->rule.smi_mac.smm3;
	 	aclUser->rule.mac.smm.octet[4]  = aclSmi->rule.smi_mac.smm4;
	 	aclUser->rule.mac.smm.octet[5]  = aclSmi->rule.smi_mac.smm5;


		aclUser->rule.mac.tlu= aclSmi->rule.smi_mac.tlu;
	 	aclUser->rule.mac.tll= aclSmi->rule.smi_mac.tll;
	 	aclUser->rule.mac.vtd= aclSmi->rule.smi_mac.vtd;
	 	aclUser->rule.mac.vtm= aclSmi->rule.smi_mac.vtm;
	 	aclUser->rule.mac.pu= aclSmi->rule.smi_mac.pu;
	 	aclUser->rule.mac.pl= aclSmi->rule.smi_mac.pl;
	 	aclUser->rule.mac.vidd= aclSmi->rule.smi_mac.vidd_1 |(aclSmi->rule.smi_mac.vidd_2<<8);
	 	aclUser->rule.mac.vidm= aclSmi->rule.smi_mac.vidm;


		break;
	 case ACL_IPV4:

		aclUser->rule.ipv4.sipU = aclSmi->rule.smi_ipv4.sipU0;
		aclUser->rule.ipv4.sipU = (aclUser->rule.ipv4.sipU<<8) | aclSmi->rule.smi_ipv4.sipU1;
		aclUser->rule.ipv4.sipU = (aclUser->rule.ipv4.sipU<<8) | aclSmi->rule.smi_ipv4.sipU2;
		aclUser->rule.ipv4.sipU = (aclUser->rule.ipv4.sipU<<8) | aclSmi->rule.smi_ipv4.sipU3;
		
		aclUser->rule.ipv4.sipL = aclSmi->rule.smi_ipv4.sipL0;
		aclUser->rule.ipv4.sipL = (aclUser->rule.ipv4.sipL<<8) | aclSmi->rule.smi_ipv4.sipL1;
		aclUser->rule.ipv4.sipL = (aclUser->rule.ipv4.sipL<<8) | aclSmi->rule.smi_ipv4.sipL2;
		aclUser->rule.ipv4.sipL = (aclUser->rule.ipv4.sipL<<8) | aclSmi->rule.smi_ipv4.sipL3;

		aclUser->rule.ipv4.dipU = aclSmi->rule.smi_ipv4.dipU0;
		aclUser->rule.ipv4.dipU = (aclUser->rule.ipv4.dipU<<8) | aclSmi->rule.smi_ipv4.dipU1;
		aclUser->rule.ipv4.dipU = (aclUser->rule.ipv4.dipU<<8) | aclSmi->rule.smi_ipv4.dipU2;
		aclUser->rule.ipv4.dipU = (aclUser->rule.ipv4.dipU<<8) | aclSmi->rule.smi_ipv4.dipU3;
		
		aclUser->rule.ipv4.dipL = aclSmi->rule.smi_ipv4.dipL0;
		aclUser->rule.ipv4.dipL = (aclUser->rule.ipv4.dipL<<8) | aclSmi->rule.smi_ipv4.dipL1;
		aclUser->rule.ipv4.dipL = (aclUser->rule.ipv4.dipL<<8) | aclSmi->rule.smi_ipv4.dipL2;
		aclUser->rule.ipv4.dipL = (aclUser->rule.ipv4.dipL<<8) | aclSmi->rule.smi_ipv4.dipL3;

		aclUser->rule.ipv4.tosD= aclSmi->rule.smi_ipv4.tosD;
		aclUser->rule.ipv4.tosM= aclSmi->rule.smi_ipv4.tosM;
		aclUser->rule.ipv4.protoV= aclSmi->rule.smi_ipv4.protoV;
		aclUser->rule.ipv4.proto1= aclSmi->rule.smi_ipv4.proto1;
		aclUser->rule.ipv4.proto2= aclSmi->rule.smi_ipv4.proto2_1 | (aclSmi->rule.smi_ipv4.proto2_2 <<4);
		aclUser->rule.ipv4.proto3= aclSmi->rule.smi_ipv4.proto3;
		aclUser->rule.ipv4.proto4= aclSmi->rule.smi_ipv4.proto4_1 | (aclSmi->rule.smi_ipv4.proto4_2 <<4);
		aclUser->rule.ipv4.flagD= aclSmi->rule.smi_ipv4.flagD;
		aclUser->rule.ipv4.flagM= aclSmi->rule.smi_ipv4.flagM;
		aclUser->rule.ipv4.offU= aclSmi->rule.smi_ipv4.offU_1 | (aclSmi->rule.smi_ipv4.offU_2 << 6) ;
		aclUser->rule.ipv4.offL= aclSmi->rule.smi_ipv4.offL_1 | (aclSmi->rule.smi_ipv4.offL_2 << 9); 

		break;
	 case ACL_IPV4_ICMP:

		aclUser->rule.ipv4icmp.sipU = aclSmi->rule.smi_ipv4icmp.sipU0;
		aclUser->rule.ipv4icmp.sipU = (aclUser->rule.ipv4icmp.sipU<<8) | aclSmi->rule.smi_ipv4icmp.sipU1;
		aclUser->rule.ipv4icmp.sipU = (aclUser->rule.ipv4icmp.sipU<<8) | aclSmi->rule.smi_ipv4icmp.sipU2;
		aclUser->rule.ipv4icmp.sipU = (aclUser->rule.ipv4icmp.sipU<<8) | aclSmi->rule.smi_ipv4icmp.sipU3;
		
		aclUser->rule.ipv4icmp.sipL = aclSmi->rule.smi_ipv4icmp.sipL0;
		aclUser->rule.ipv4icmp.sipL = (aclUser->rule.ipv4icmp.sipL<<8) | aclSmi->rule.smi_ipv4icmp.sipL1;
		aclUser->rule.ipv4icmp.sipL = (aclUser->rule.ipv4icmp.sipL<<8) | aclSmi->rule.smi_ipv4icmp.sipL2;
		aclUser->rule.ipv4icmp.sipL = (aclUser->rule.ipv4icmp.sipL<<8) | aclSmi->rule.smi_ipv4icmp.sipL3;

		aclUser->rule.ipv4icmp.dipU = aclSmi->rule.smi_ipv4icmp.dipU0;
		aclUser->rule.ipv4icmp.dipU = (aclUser->rule.ipv4icmp.dipU<<8) | aclSmi->rule.smi_ipv4icmp.dipU1;
		aclUser->rule.ipv4icmp.dipU = (aclUser->rule.ipv4icmp.dipU<<8) | aclSmi->rule.smi_ipv4icmp.dipU2;
		aclUser->rule.ipv4icmp.dipU = (aclUser->rule.ipv4icmp.dipU<<8) | aclSmi->rule.smi_ipv4icmp.dipU3;
		
		aclUser->rule.ipv4icmp.dipL = aclSmi->rule.smi_ipv4icmp.dipL0;
		aclUser->rule.ipv4icmp.dipL = (aclUser->rule.ipv4icmp.dipL<<8) | aclSmi->rule.smi_ipv4icmp.dipL1;
		aclUser->rule.ipv4icmp.dipL = (aclUser->rule.ipv4icmp.dipL<<8) | aclSmi->rule.smi_ipv4icmp.dipL2;
		aclUser->rule.ipv4icmp.dipL = (aclUser->rule.ipv4icmp.dipL<<8) | aclSmi->rule.smi_ipv4icmp.dipL3;


		aclUser->rule.ipv4icmp.tosD= aclSmi->rule.smi_ipv4icmp.tosD;
		aclUser->rule.ipv4icmp.tosM= aclSmi->rule.smi_ipv4icmp.tosM;
		aclUser->rule.ipv4icmp.typeV= aclSmi->rule.smi_ipv4icmp.typeV;
		aclUser->rule.ipv4icmp.type1= aclSmi->rule.smi_ipv4icmp.type1;
		aclUser->rule.ipv4icmp.type2 = aclSmi->rule.smi_ipv4icmp.type2_1 | (aclSmi->rule.smi_ipv4icmp.type2_2 <<4);
		aclUser->rule.ipv4icmp.type3= aclSmi->rule.smi_ipv4icmp.type3;
		aclUser->rule.ipv4icmp.type4 = aclSmi->rule.smi_ipv4icmp.type4_1 | (aclSmi->rule.smi_ipv4icmp.type4_2 <<4);
		aclUser->rule.ipv4icmp.codeV= aclSmi->rule.smi_ipv4icmp.codeV;
		aclUser->rule.ipv4icmp.code1= aclSmi->rule.smi_ipv4icmp.code1;
		aclUser->rule.ipv4icmp.code2= aclSmi->rule.smi_ipv4icmp.code2;
		aclUser->rule.ipv4icmp.code3= aclSmi->rule.smi_ipv4icmp.code3;
		aclUser->rule.ipv4icmp.code4= aclSmi->rule.smi_ipv4icmp.code4;

		break;
	 case ACL_IPV4_IGMP:

		aclUser->rule.ipv4igmp.sipU = aclSmi->rule.smi_ipv4igmp.sipU0;
		aclUser->rule.ipv4igmp.sipU = (aclUser->rule.ipv4igmp.sipU<<8) | aclSmi->rule.smi_ipv4igmp.sipU1;
		aclUser->rule.ipv4igmp.sipU = (aclUser->rule.ipv4igmp.sipU<<8) | aclSmi->rule.smi_ipv4igmp.sipU2;
		aclUser->rule.ipv4igmp.sipU = (aclUser->rule.ipv4igmp.sipU<<8) | aclSmi->rule.smi_ipv4igmp.sipU3;
		
		aclUser->rule.ipv4igmp.sipL = aclSmi->rule.smi_ipv4igmp.sipL0;
		aclUser->rule.ipv4igmp.sipL = (aclUser->rule.ipv4igmp.sipL<<8) | aclSmi->rule.smi_ipv4igmp.sipL1;
		aclUser->rule.ipv4igmp.sipL = (aclUser->rule.ipv4igmp.sipL<<8) | aclSmi->rule.smi_ipv4igmp.sipL2;
		aclUser->rule.ipv4igmp.sipL = (aclUser->rule.ipv4igmp.sipL<<8) | aclSmi->rule.smi_ipv4igmp.sipL3;

		aclUser->rule.ipv4igmp.dipU = aclSmi->rule.smi_ipv4igmp.dipU0;
		aclUser->rule.ipv4igmp.dipU = (aclUser->rule.ipv4igmp.dipU<<8) | aclSmi->rule.smi_ipv4igmp.dipU1;
		aclUser->rule.ipv4igmp.dipU = (aclUser->rule.ipv4igmp.dipU<<8) | aclSmi->rule.smi_ipv4igmp.dipU2;
		aclUser->rule.ipv4igmp.dipU = (aclUser->rule.ipv4igmp.dipU<<8) | aclSmi->rule.smi_ipv4igmp.dipU3;
		
		aclUser->rule.ipv4igmp.dipL = aclSmi->rule.smi_ipv4igmp.dipL0;
		aclUser->rule.ipv4igmp.dipL = (aclUser->rule.ipv4igmp.dipL<<8) | aclSmi->rule.smi_ipv4igmp.dipL1;
		aclUser->rule.ipv4igmp.dipL = (aclUser->rule.ipv4igmp.dipL<<8) | aclSmi->rule.smi_ipv4igmp.dipL2;
		aclUser->rule.ipv4igmp.dipL = (aclUser->rule.ipv4igmp.dipL<<8) | aclSmi->rule.smi_ipv4igmp.dipL3;


		aclUser->rule.ipv4igmp.tosD= aclSmi->rule.smi_ipv4igmp.tosD;
		aclUser->rule.ipv4igmp.tosM= aclSmi->rule.smi_ipv4igmp.tosM;
		aclUser->rule.ipv4igmp.typeV= aclSmi->rule.smi_ipv4igmp.typeV;
		aclUser->rule.ipv4igmp.type1= aclSmi->rule.smi_ipv4igmp.type1;
		aclUser->rule.ipv4igmp.type2 = aclSmi->rule.smi_ipv4igmp.type2_1 | (aclSmi->rule.smi_ipv4igmp.type2_2 <<4);
		aclUser->rule.ipv4igmp.type3= aclSmi->rule.smi_ipv4igmp.type3;
		aclUser->rule.ipv4igmp.type4 = aclSmi->rule.smi_ipv4igmp.type4_1 | (aclSmi->rule.smi_ipv4igmp.type4_2 <<4);

		break;
	 case ACL_IPV4_TCP:

		aclUser->rule.ipv4tcp.sipU = aclSmi->rule.smi_ipv4tcp.sipU0;
		aclUser->rule.ipv4tcp.sipU = (aclUser->rule.ipv4tcp.sipU<<8) | aclSmi->rule.smi_ipv4tcp.sipU1;
		aclUser->rule.ipv4tcp.sipU = (aclUser->rule.ipv4tcp.sipU<<8) | aclSmi->rule.smi_ipv4tcp.sipU2;
		aclUser->rule.ipv4tcp.sipU = (aclUser->rule.ipv4tcp.sipU<<8) | aclSmi->rule.smi_ipv4tcp.sipU3;
		
		aclUser->rule.ipv4tcp.sipL = aclSmi->rule.smi_ipv4tcp.sipL0;
		aclUser->rule.ipv4tcp.sipL = (aclUser->rule.ipv4tcp.sipL<<8) | aclSmi->rule.smi_ipv4tcp.sipL1;
		aclUser->rule.ipv4tcp.sipL = (aclUser->rule.ipv4tcp.sipL<<8) | aclSmi->rule.smi_ipv4tcp.sipL2;
		aclUser->rule.ipv4tcp.sipL = (aclUser->rule.ipv4tcp.sipL<<8) | aclSmi->rule.smi_ipv4tcp.sipL3;

		aclUser->rule.ipv4tcp.dipU = aclSmi->rule.smi_ipv4tcp.dipU0;
		aclUser->rule.ipv4tcp.dipU = (aclUser->rule.ipv4tcp.dipU<<8) | aclSmi->rule.smi_ipv4tcp.dipU1;
		aclUser->rule.ipv4tcp.dipU = (aclUser->rule.ipv4tcp.dipU<<8) | aclSmi->rule.smi_ipv4tcp.dipU2;
		aclUser->rule.ipv4tcp.dipU = (aclUser->rule.ipv4tcp.dipU<<8) | aclSmi->rule.smi_ipv4tcp.dipU3;
		
		aclUser->rule.ipv4tcp.dipL = aclSmi->rule.smi_ipv4tcp.dipL0;
		aclUser->rule.ipv4tcp.dipL = (aclUser->rule.ipv4tcp.dipL<<8) | aclSmi->rule.smi_ipv4tcp.dipL1;
		aclUser->rule.ipv4tcp.dipL = (aclUser->rule.ipv4tcp.dipL<<8) | aclSmi->rule.smi_ipv4tcp.dipL2;
		aclUser->rule.ipv4tcp.dipL = (aclUser->rule.ipv4tcp.dipL<<8) | aclSmi->rule.smi_ipv4tcp.dipL3;



		aclUser->rule.ipv4tcp.tosD= aclSmi->rule.smi_ipv4tcp.tosD;
		aclUser->rule.ipv4tcp.tosM= aclSmi->rule.smi_ipv4tcp.tosM;
		aclUser->rule.ipv4tcp.sPortU= aclSmi->rule.smi_ipv4tcp.sPortU;
		aclUser->rule.ipv4tcp.sPortL= aclSmi->rule.smi_ipv4tcp.sPortL;
		aclUser->rule.ipv4tcp.dPortU= aclSmi->rule.smi_ipv4tcp.dPortU;
		aclUser->rule.ipv4tcp.dPortL= aclSmi->rule.smi_ipv4tcp.dPortL;
		aclUser->rule.ipv4tcp.flagD= aclSmi->rule.smi_ipv4tcp.flagD;
		aclUser->rule.ipv4tcp.flagM= aclSmi->rule.smi_ipv4tcp.flagM;

		break;
	 case ACL_IPV4_UDP:

		aclUser->rule.ipv4udp.sipU = aclSmi->rule.smi_ipv4udp.sipU0;
		aclUser->rule.ipv4udp.sipU = (aclUser->rule.ipv4udp.sipU<<8) | aclSmi->rule.smi_ipv4udp.sipU1;
		aclUser->rule.ipv4udp.sipU = (aclUser->rule.ipv4udp.sipU<<8) | aclSmi->rule.smi_ipv4udp.sipU2;
		aclUser->rule.ipv4udp.sipU = (aclUser->rule.ipv4udp.sipU<<8) | aclSmi->rule.smi_ipv4udp.sipU3;
		
		aclUser->rule.ipv4udp.sipL = aclSmi->rule.smi_ipv4udp.sipL0;
		aclUser->rule.ipv4udp.sipL = (aclUser->rule.ipv4udp.sipL<<8) | aclSmi->rule.smi_ipv4udp.sipL1;
		aclUser->rule.ipv4udp.sipL = (aclUser->rule.ipv4udp.sipL<<8) | aclSmi->rule.smi_ipv4udp.sipL2;
		aclUser->rule.ipv4udp.sipL = (aclUser->rule.ipv4udp.sipL<<8) | aclSmi->rule.smi_ipv4udp.sipL3;

		aclUser->rule.ipv4udp.dipU = aclSmi->rule.smi_ipv4udp.dipU0;
		aclUser->rule.ipv4udp.dipU = (aclUser->rule.ipv4udp.dipU<<8) | aclSmi->rule.smi_ipv4udp.dipU1;
		aclUser->rule.ipv4udp.dipU = (aclUser->rule.ipv4udp.dipU<<8) | aclSmi->rule.smi_ipv4udp.dipU2;
		aclUser->rule.ipv4udp.dipU = (aclUser->rule.ipv4udp.dipU<<8) | aclSmi->rule.smi_ipv4udp.dipU3;
		
		aclUser->rule.ipv4udp.dipL = aclSmi->rule.smi_ipv4udp.dipL0;
		aclUser->rule.ipv4udp.dipL = (aclUser->rule.ipv4udp.dipL<<8) | aclSmi->rule.smi_ipv4udp.dipL1;
		aclUser->rule.ipv4udp.dipL = (aclUser->rule.ipv4udp.dipL<<8) | aclSmi->rule.smi_ipv4udp.dipL2;
		aclUser->rule.ipv4udp.dipL = (aclUser->rule.ipv4udp.dipL<<8) | aclSmi->rule.smi_ipv4udp.dipL3;

		aclUser->rule.ipv4udp.tosD= aclSmi->rule.smi_ipv4udp.tosD;
		aclUser->rule.ipv4udp.tosM= aclSmi->rule.smi_ipv4udp.tosM;
		aclUser->rule.ipv4udp.sPortU= aclSmi->rule.smi_ipv4udp.sPortU;
		aclUser->rule.ipv4udp.sPortL= aclSmi->rule.smi_ipv4udp.sPortL;
		aclUser->rule.ipv4udp.dPortU= aclSmi->rule.smi_ipv4udp.dPortU;
		aclUser->rule.ipv4udp.dPortL= aclSmi->rule.smi_ipv4udp.dPortL;
		break;
	 case ACL_IPV6_SIP:
		aclUser->rule.ipv6sip.sipU[0]= aclSmi->rule.smi_ipv6sip.sipU[0];
		aclUser->rule.ipv6sip.sipU[1]= aclSmi->rule.smi_ipv6sip.sipU[1];
		aclUser->rule.ipv6sip.sipU[2]= aclSmi->rule.smi_ipv6sip.sipU[2];
		aclUser->rule.ipv6sip.sipU[3]= aclSmi->rule.smi_ipv6sip.sipU[3];
		aclUser->rule.ipv6sip.sipU[4]= aclSmi->rule.smi_ipv6sip.sipU[4];
		aclUser->rule.ipv6sip.sipU[5]= aclSmi->rule.smi_ipv6sip.sipU[5];
		aclUser->rule.ipv6sip.sipU[6]= aclSmi->rule.smi_ipv6sip.sipU[6];
		aclUser->rule.ipv6sip.sipU[7]= aclSmi->rule.smi_ipv6sip.sipU[7];
		aclUser->rule.ipv6sip.sipL[0]= aclSmi->rule.smi_ipv6sip.sipL[0];
		aclUser->rule.ipv6sip.sipL[1]= aclSmi->rule.smi_ipv6sip.sipL[1];
		aclUser->rule.ipv6sip.sipL[2]= aclSmi->rule.smi_ipv6sip.sipL[2];
		aclUser->rule.ipv6sip.sipL[3]= aclSmi->rule.smi_ipv6sip.sipL[3];
		aclUser->rule.ipv6sip.sipL[4]= aclSmi->rule.smi_ipv6sip.sipL[4];
		aclUser->rule.ipv6sip.sipL[5]= aclSmi->rule.smi_ipv6sip.sipL[5];
		aclUser->rule.ipv6sip.sipL[6]= aclSmi->rule.smi_ipv6sip.sipL[6];
		aclUser->rule.ipv6sip.sipL[7]= aclSmi->rule.smi_ipv6sip.sipL[7];
		
		break;
	 case ACL_IPV6_DIP:
		aclUser->rule.ipv6dip.dipU[0]= aclSmi->rule.smi_ipv6dip.dipU[0];
		aclUser->rule.ipv6dip.dipU[1]= aclSmi->rule.smi_ipv6dip.dipU[1];
		aclUser->rule.ipv6dip.dipU[2]= aclSmi->rule.smi_ipv6dip.dipU[2];
		aclUser->rule.ipv6dip.dipU[3]= aclSmi->rule.smi_ipv6dip.dipU[3];
		aclUser->rule.ipv6dip.dipU[4]= aclSmi->rule.smi_ipv6dip.dipU[4];
		aclUser->rule.ipv6dip.dipU[5]= aclSmi->rule.smi_ipv6dip.dipU[5];
		aclUser->rule.ipv6dip.dipU[6]= aclSmi->rule.smi_ipv6dip.dipU[6];
		aclUser->rule.ipv6dip.dipU[7]= aclSmi->rule.smi_ipv6dip.dipU[7];
		aclUser->rule.ipv6dip.dipL[0]= aclSmi->rule.smi_ipv6dip.dipL[0];
		aclUser->rule.ipv6dip.dipL[1]= aclSmi->rule.smi_ipv6dip.dipL[1];
		aclUser->rule.ipv6dip.dipL[2]= aclSmi->rule.smi_ipv6dip.dipL[2];
		aclUser->rule.ipv6dip.dipL[3]= aclSmi->rule.smi_ipv6dip.dipL[3];
		aclUser->rule.ipv6dip.dipL[4]= aclSmi->rule.smi_ipv6dip.dipL[4];
		aclUser->rule.ipv6dip.dipL[5]= aclSmi->rule.smi_ipv6dip.dipL[5];
		aclUser->rule.ipv6dip.dipL[6]= aclSmi->rule.smi_ipv6dip.dipL[6];
		aclUser->rule.ipv6dip.dipL[7]= aclSmi->rule.smi_ipv6dip.dipL[7];
		
		break;
	case ACL_IPV6_EXT:
		aclUser->rule.ipv6ext.tcD= aclSmi->rule.smi_ipv6ext.tcD;
		aclUser->rule.ipv6ext.tcM= aclSmi->rule.smi_ipv6ext.tcM;
		aclUser->rule.ipv6ext.nhV= aclSmi->rule.smi_ipv6ext.nhV;
		aclUser->rule.ipv6ext.nhp1= aclSmi->rule.smi_ipv6ext.nhp1;
		aclUser->rule.ipv6ext.nhp2= aclSmi->rule.smi_ipv6ext.nhp2_1 | (aclSmi->rule.smi_ipv6ext.nhp2_2 <<4);
		aclUser->rule.ipv6ext.nhp3= aclSmi->rule.smi_ipv6ext.nhp3;
		aclUser->rule.ipv6ext.nhp4= aclSmi->rule.smi_ipv6ext.nhp4_1 | (aclSmi->rule.smi_ipv6ext.nhp4_2 <<4);

		break;
	case ACL_IPV6_TCP:
		aclUser->rule.ipv6tcp.tcD= aclSmi->rule.smi_ipv6tcp.tcD;
		aclUser->rule.ipv6tcp.tcM= aclSmi->rule.smi_ipv6tcp.tcM;
		aclUser->rule.ipv6tcp.sPortU= aclSmi->rule.smi_ipv6tcp.sPortU;
		aclUser->rule.ipv6tcp.sPortL= aclSmi->rule.smi_ipv6tcp.sPortL;
		aclUser->rule.ipv6tcp.dPortU= aclSmi->rule.smi_ipv6tcp.dPortU;
		aclUser->rule.ipv6tcp.dPortL= aclSmi->rule.smi_ipv6tcp.dPortL;
		aclUser->rule.ipv6tcp.flagD= aclSmi->rule.smi_ipv6tcp.flagD;
		aclUser->rule.ipv6tcp.flagM= aclSmi->rule.smi_ipv6tcp.flagM;

		break;		
	case ACL_IPV6_UDP:
		aclUser->rule.ipv6udp.tcD= aclSmi->rule.smi_ipv6udp.tcD;
		aclUser->rule.ipv6udp.tcM= aclSmi->rule.smi_ipv6udp.tcM;
		aclUser->rule.ipv6udp.sPortU= aclSmi->rule.smi_ipv6udp.sPortU;
		aclUser->rule.ipv6udp.sPortL= aclSmi->rule.smi_ipv6udp.sPortL;
		aclUser->rule.ipv6udp.dPortU= aclSmi->rule.smi_ipv6udp.dPortU;
		aclUser->rule.ipv6udp.dPortL= aclSmi->rule.smi_ipv6udp.dPortL;

		break;		
	case ACL_IPV6_ICMP:
		aclUser->rule.ipv6icmp.tcD= aclSmi->rule.smi_ipv6icmp.tcD;
		aclUser->rule.ipv6icmp.tcM= aclSmi->rule.smi_ipv6icmp.tcM;
		aclUser->rule.ipv6icmp.typeV= aclSmi->rule.smi_ipv6icmp.typeV;
		aclUser->rule.ipv6icmp.type1= aclSmi->rule.smi_ipv6icmp.type1;
		aclUser->rule.ipv6icmp.type2= aclSmi->rule.smi_ipv6icmp.type2_1 | (aclSmi->rule.smi_ipv6icmp.type2_2 <<4);
		aclUser->rule.ipv6icmp.type3= aclSmi->rule.smi_ipv6icmp.type3;
		aclUser->rule.ipv6icmp.type4= aclSmi->rule.smi_ipv6icmp.type4_1 | (aclSmi->rule.smi_ipv6icmp.type4_2 <<4);
		aclUser->rule.ipv6icmp.codeV= aclSmi->rule.smi_ipv6icmp.codeV;
		aclUser->rule.ipv6icmp.code1= aclSmi->rule.smi_ipv6icmp.code1;
		aclUser->rule.ipv6icmp.code2= aclSmi->rule.smi_ipv6icmp.code2;
		aclUser->rule.ipv6icmp.code3= aclSmi->rule.smi_ipv6icmp.code3;
		aclUser->rule.ipv6icmp.code4= aclSmi->rule.smi_ipv6icmp.code4;
	
		break;
	}


	aclUser->ac_meteridx= aclSmi->ac_meteridx;
	aclUser->ac_policing = aclSmi->ac_policing;
	aclUser->ac_priority = aclSmi->ac_priority;
	aclUser->ac_spri = aclSmi->ac_spri;
	aclUser->ac_mirpmsk= aclSmi->ac_mirpmsk_1 | (aclSmi->ac_mirpmsk_2<<7);
	aclUser->ac_mir = aclSmi->ac_mir;
	aclUser->ac_svidx = aclSmi->ac_svidx;
	aclUser->ac_svlan= aclSmi->ac_svlan;

	aclUser->op_term= aclSmi->op_term;
	aclUser->op_exec= aclSmi->op_exec;
	aclUser->op_and= aclSmi->op_and;
	aclUser->op_not= aclSmi->op_not;
	aclUser->op_init= aclSmi->op_init;
	aclUser->format= aclSmi->format;
	aclUser->port_en= aclSmi->port_en_1 | (aclSmi->port_en_2<<1);
	aclUser->reserved= aclSmi->reserved;
}

/*
	Exchange structure type define with MMI and SMI
*/
static void _rtl8368s_aclStUser2Smi( rtl8368s_acltable *aclUser, rtl8368s_smiacltable *aclSmi)
{


	switch(aclUser->format)
	{
	 case ACL_MAC:

	 	aclSmi->rule.smi_mac.dmp0 = aclUser->rule.mac.dmp.octet[0] ;
	 	aclSmi->rule.smi_mac.dmp1 = aclUser->rule.mac.dmp.octet[1] ;
	 	aclSmi->rule.smi_mac.dmp2 = aclUser->rule.mac.dmp.octet[2] ;
	 	aclSmi->rule.smi_mac.dmp3 = aclUser->rule.mac.dmp.octet[3] ;
	 	aclSmi->rule.smi_mac.dmp4 = aclUser->rule.mac.dmp.octet[4] ;
	 	aclSmi->rule.smi_mac.dmp5 = aclUser->rule.mac.dmp.octet[5] ;

		aclSmi->rule.smi_mac.dmm0  = aclUser->rule.mac.dmm.octet[0] ;
		aclSmi->rule.smi_mac.dmm1  = aclUser->rule.mac.dmm.octet[1] ;
		aclSmi->rule.smi_mac.dmm2  = aclUser->rule.mac.dmm.octet[2] ;
		aclSmi->rule.smi_mac.dmm3  = aclUser->rule.mac.dmm.octet[3] ;
		aclSmi->rule.smi_mac.dmm4  = aclUser->rule.mac.dmm.octet[4] ;
		aclSmi->rule.smi_mac.dmm5  = aclUser->rule.mac.dmm.octet[5] ;

	 	aclSmi->rule.smi_mac.smp0  = aclUser->rule.mac.smp.octet[0] ;
	 	aclSmi->rule.smi_mac.smp1  = aclUser->rule.mac.smp.octet[1] ;
	 	aclSmi->rule.smi_mac.smp2  = aclUser->rule.mac.smp.octet[2] ;
	 	aclSmi->rule.smi_mac.smp3  = aclUser->rule.mac.smp.octet[3] ;
	 	aclSmi->rule.smi_mac.smp4  = aclUser->rule.mac.smp.octet[4] ;
	 	aclSmi->rule.smi_mac.smp5  = aclUser->rule.mac.smp.octet[5] ;

	 	aclSmi->rule.smi_mac.smm0  = aclUser->rule.mac.smm.octet[0] ;
	 	aclSmi->rule.smi_mac.smm1  = aclUser->rule.mac.smm.octet[1] ;
	 	aclSmi->rule.smi_mac.smm2  = aclUser->rule.mac.smm.octet[2] ;
	 	aclSmi->rule.smi_mac.smm3  = aclUser->rule.mac.smm.octet[3] ;
	 	aclSmi->rule.smi_mac.smm4  = aclUser->rule.mac.smm.octet[4] ;
	 	aclSmi->rule.smi_mac.smm5  = aclUser->rule.mac.smm.octet[5] ;

	 	aclSmi->rule.smi_mac.tlu= aclUser->rule.mac.tlu;
	 	aclSmi->rule.smi_mac.tll= aclUser->rule.mac.tll;

		aclSmi->rule.smi_mac.vtd= aclUser->rule.mac.vtd;
	 	aclSmi->rule.smi_mac.vtm= aclUser->rule.mac.vtm;

		aclSmi->rule.smi_mac.pu= aclUser->rule.mac.pu;
	 	aclSmi->rule.smi_mac.pl= aclUser->rule.mac.pl;
	 	aclSmi->rule.smi_mac.vidd_1= aclUser->rule.mac.vidd&0x00FF;
	 	aclSmi->rule.smi_mac.vidd_2= aclUser->rule.mac.vidd>>8;
	 	aclSmi->rule.smi_mac.vidm= aclUser->rule.mac.vidm;

		break;
	 case ACL_IPV4:

		aclSmi->rule.smi_ipv4.sipU0  = (aclUser->rule.ipv4.sipU & 0xFF000000) >> 24;
		aclSmi->rule.smi_ipv4.sipU1  = (aclUser->rule.ipv4.sipU & 0x00FF0000) >> 16;
		aclSmi->rule.smi_ipv4.sipU2  = (aclUser->rule.ipv4.sipU & 0x0000FF00) >> 8;
		aclSmi->rule.smi_ipv4.sipU3  = aclUser->rule.ipv4.sipU & 0x000000FF;
		
		aclSmi->rule.smi_ipv4.sipL0  = (aclUser->rule.ipv4.sipL & 0xFF000000) >> 24;
		aclSmi->rule.smi_ipv4.sipL1  = (aclUser->rule.ipv4.sipL & 0x00FF0000) >> 16;
		aclSmi->rule.smi_ipv4.sipL2  = (aclUser->rule.ipv4.sipL & 0x0000FF00) >> 8;
		aclSmi->rule.smi_ipv4.sipL3  = aclUser->rule.ipv4.sipL & 0x000000FF;

		aclSmi->rule.smi_ipv4.dipU0  = (aclUser->rule.ipv4.dipU & 0xFF000000) >> 24;
		aclSmi->rule.smi_ipv4.dipU1  = (aclUser->rule.ipv4.dipU & 0x00FF0000) >> 16;
		aclSmi->rule.smi_ipv4.dipU2  = (aclUser->rule.ipv4.dipU & 0x0000FF00) >> 8;
		aclSmi->rule.smi_ipv4.dipU3  = aclUser->rule.ipv4.dipU & 0x000000FF;
		
		aclSmi->rule.smi_ipv4.dipL0  = (aclUser->rule.ipv4.dipL & 0xFF000000) >> 24;
		aclSmi->rule.smi_ipv4.dipL1  = (aclUser->rule.ipv4.dipL & 0x00FF0000) >> 16;
		aclSmi->rule.smi_ipv4.dipL2  = (aclUser->rule.ipv4.dipL & 0x0000FF00) >> 8;
		aclSmi->rule.smi_ipv4.dipL3  = aclUser->rule.ipv4.dipL & 0x000000FF;

		aclSmi->rule.smi_ipv4.tosD= aclUser->rule.ipv4.tosD;
		aclSmi->rule.smi_ipv4.tosM= aclUser->rule.ipv4.tosM;
		aclSmi->rule.smi_ipv4.protoV= aclUser->rule.ipv4.protoV;
		aclSmi->rule.smi_ipv4.proto1= aclUser->rule.ipv4.proto1;
		aclSmi->rule.smi_ipv4.proto2_1= aclUser->rule.ipv4.proto2 & 0x000F;
		aclSmi->rule.smi_ipv4.proto2_2= aclUser->rule.ipv4.proto2>>4;
		aclSmi->rule.smi_ipv4.proto3= aclUser->rule.ipv4.proto3;
		aclSmi->rule.smi_ipv4.proto4_1= aclUser->rule.ipv4.proto4 & 0x000F;
		aclSmi->rule.smi_ipv4.proto4_2= aclUser->rule.ipv4.proto4>>4;
		aclSmi->rule.smi_ipv4.flagD= aclUser->rule.ipv4.flagD;
		aclSmi->rule.smi_ipv4.flagM= aclUser->rule.ipv4.flagM;
		aclSmi->rule.smi_ipv4.offU_1= aclUser->rule.ipv4.offU & 0x003F;
		aclSmi->rule.smi_ipv4.offU_2= aclUser->rule.ipv4.offU >>6;
		aclSmi->rule.smi_ipv4.offL_1= aclUser->rule.ipv4.offL & 0x01FF;
		aclSmi->rule.smi_ipv4.offL_2= aclUser->rule.ipv4.offL >>9;

		break;
	 case ACL_IPV4_ICMP:

		aclSmi->rule.smi_ipv4icmp.sipU0  = (aclUser->rule.ipv4icmp.sipU & 0xFF000000) >> 24;
		aclSmi->rule.smi_ipv4icmp.sipU1  = (aclUser->rule.ipv4icmp.sipU & 0x00FF0000) >> 16;
		aclSmi->rule.smi_ipv4icmp.sipU2  = (aclUser->rule.ipv4icmp.sipU & 0x0000FF00) >> 8;
		aclSmi->rule.smi_ipv4icmp.sipU3  = aclUser->rule.ipv4icmp.sipU & 0x000000FF;
		
		aclSmi->rule.smi_ipv4icmp.sipL0  = (aclUser->rule.ipv4icmp.sipL & 0xFF000000) >> 24;
		aclSmi->rule.smi_ipv4icmp.sipL1  = (aclUser->rule.ipv4icmp.sipL & 0x00FF0000) >> 16;
		aclSmi->rule.smi_ipv4icmp.sipL2  = (aclUser->rule.ipv4icmp.sipL & 0x0000FF00) >> 8;
		aclSmi->rule.smi_ipv4icmp.sipL3  = aclUser->rule.ipv4icmp.sipL & 0x000000FF;

		aclSmi->rule.smi_ipv4icmp.dipU0  = (aclUser->rule.ipv4icmp.dipU & 0xFF000000) >> 24;
		aclSmi->rule.smi_ipv4icmp.dipU1  = (aclUser->rule.ipv4icmp.dipU & 0x00FF0000) >> 16;
		aclSmi->rule.smi_ipv4icmp.dipU2  = (aclUser->rule.ipv4icmp.dipU & 0x0000FF00) >> 8;
		aclSmi->rule.smi_ipv4icmp.dipU3  = aclUser->rule.ipv4icmp.dipU & 0x000000FF;
		
		aclSmi->rule.smi_ipv4icmp.dipL0  = (aclUser->rule.ipv4icmp.dipL & 0xFF000000) >> 24;
		aclSmi->rule.smi_ipv4icmp.dipL1  = (aclUser->rule.ipv4icmp.dipL & 0x00FF0000) >> 16;
		aclSmi->rule.smi_ipv4icmp.dipL2  = (aclUser->rule.ipv4icmp.dipL & 0x0000FF00) >> 8;
		aclSmi->rule.smi_ipv4icmp.dipL3  = aclUser->rule.ipv4icmp.dipL & 0x000000FF;

		aclSmi->rule.smi_ipv4icmp.tosD= aclUser->rule.ipv4icmp.tosD;
		aclSmi->rule.smi_ipv4icmp.tosM= aclUser->rule.ipv4icmp.tosM;
		aclSmi->rule.smi_ipv4icmp.typeV= aclUser->rule.ipv4icmp.typeV;
		aclSmi->rule.smi_ipv4icmp.type1= aclUser->rule.ipv4icmp.type1;
		aclSmi->rule.smi_ipv4icmp.type2_1= aclUser->rule.ipv4icmp.type2 & 0x000F;
		aclSmi->rule.smi_ipv4icmp.type2_2= aclUser->rule.ipv4icmp.type2 >>4;
		aclSmi->rule.smi_ipv4icmp.type3= aclUser->rule.ipv4icmp.type3;
		aclSmi->rule.smi_ipv4icmp.type4_1= aclUser->rule.ipv4icmp.type4 & 0x000F;
		aclSmi->rule.smi_ipv4icmp.type4_2= aclUser->rule.ipv4icmp.type4 >>4;
		aclSmi->rule.smi_ipv4icmp.codeV= aclUser->rule.ipv4icmp.codeV;
		aclSmi->rule.smi_ipv4icmp.code1= aclUser->rule.ipv4icmp.code1;
		aclSmi->rule.smi_ipv4icmp.code2= aclUser->rule.ipv4icmp.code2;
		aclSmi->rule.smi_ipv4icmp.code3= aclUser->rule.ipv4icmp.code3;
		aclSmi->rule.smi_ipv4icmp.code4= aclUser->rule.ipv4icmp.code4;

		break;
	 case ACL_IPV4_IGMP:

		aclSmi->rule.smi_ipv4igmp.sipU0  = (aclUser->rule.ipv4igmp.sipU & 0xFF000000) >> 24;
		aclSmi->rule.smi_ipv4igmp.sipU1  = (aclUser->rule.ipv4igmp.sipU & 0x00FF0000) >> 16;
		aclSmi->rule.smi_ipv4igmp.sipU2  = (aclUser->rule.ipv4igmp.sipU & 0x0000FF00) >> 8;
		aclSmi->rule.smi_ipv4igmp.sipU3  = aclUser->rule.ipv4igmp.sipU & 0x000000FF;
		
		aclSmi->rule.smi_ipv4igmp.sipL0  = (aclUser->rule.ipv4igmp.sipL & 0xFF000000) >> 24;
		aclSmi->rule.smi_ipv4igmp.sipL1  = (aclUser->rule.ipv4igmp.sipL & 0x00FF0000) >> 16;
		aclSmi->rule.smi_ipv4igmp.sipL2  = (aclUser->rule.ipv4igmp.sipL & 0x0000FF00) >> 8;
		aclSmi->rule.smi_ipv4igmp.sipL3  = aclUser->rule.ipv4igmp.sipL & 0x000000FF;

		aclSmi->rule.smi_ipv4igmp.dipU0  = (aclUser->rule.ipv4igmp.dipU & 0xFF000000) >> 24;
		aclSmi->rule.smi_ipv4igmp.dipU1  = (aclUser->rule.ipv4igmp.dipU & 0x00FF0000) >> 16;
		aclSmi->rule.smi_ipv4igmp.dipU2  = (aclUser->rule.ipv4igmp.dipU & 0x0000FF00) >> 8;
		aclSmi->rule.smi_ipv4igmp.dipU3  = aclUser->rule.ipv4igmp.dipU & 0x000000FF;
		
		aclSmi->rule.smi_ipv4igmp.dipL0  = (aclUser->rule.ipv4igmp.dipL & 0xFF000000) >> 24;
		aclSmi->rule.smi_ipv4igmp.dipL1  = (aclUser->rule.ipv4igmp.dipL & 0x00FF0000) >> 16;
		aclSmi->rule.smi_ipv4igmp.dipL2  = (aclUser->rule.ipv4igmp.dipL & 0x0000FF00) >> 8;
		aclSmi->rule.smi_ipv4igmp.dipL3  = aclUser->rule.ipv4igmp.dipL & 0x000000FF;

		aclSmi->rule.smi_ipv4igmp.tosD= aclUser->rule.ipv4igmp.tosD;
		aclSmi->rule.smi_ipv4igmp.tosM= aclUser->rule.ipv4igmp.tosM;
		aclSmi->rule.smi_ipv4igmp.typeV= aclUser->rule.ipv4igmp.typeV;
		aclSmi->rule.smi_ipv4igmp.type1= aclUser->rule.ipv4igmp.type1;
		aclSmi->rule.smi_ipv4igmp.type2_1= aclUser->rule.ipv4igmp.type2 & 0x000F;
		aclSmi->rule.smi_ipv4igmp.type2_2= aclUser->rule.ipv4igmp.type2 >>4;
		aclSmi->rule.smi_ipv4igmp.type3= aclUser->rule.ipv4igmp.type3;
		aclSmi->rule.smi_ipv4igmp.type4_1= aclUser->rule.ipv4igmp.type4 & 0x000F;
		aclSmi->rule.smi_ipv4igmp.type4_2= aclUser->rule.ipv4igmp.type4 >>4;

		break;
	 case ACL_IPV4_TCP:

		aclSmi->rule.smi_ipv4tcp.sipU0  = (aclUser->rule.ipv4tcp.sipU & 0xFF000000) >> 24;
		aclSmi->rule.smi_ipv4tcp.sipU1  = (aclUser->rule.ipv4tcp.sipU & 0x00FF0000) >> 16;
		aclSmi->rule.smi_ipv4tcp.sipU2  = (aclUser->rule.ipv4tcp.sipU & 0x0000FF00) >> 8;
		aclSmi->rule.smi_ipv4tcp.sipU3  = aclUser->rule.ipv4tcp.sipU & 0x000000FF;
		
		aclSmi->rule.smi_ipv4tcp.sipL0  = (aclUser->rule.ipv4tcp.sipL & 0xFF000000) >> 24;
		aclSmi->rule.smi_ipv4tcp.sipL1  = (aclUser->rule.ipv4tcp.sipL & 0x00FF0000) >> 16;
		aclSmi->rule.smi_ipv4tcp.sipL2  = (aclUser->rule.ipv4tcp.sipL & 0x0000FF00) >> 8;
		aclSmi->rule.smi_ipv4tcp.sipL3  = aclUser->rule.ipv4tcp.sipL & 0x000000FF;

		aclSmi->rule.smi_ipv4tcp.dipU0  = (aclUser->rule.ipv4tcp.dipU & 0xFF000000) >> 24;
		aclSmi->rule.smi_ipv4tcp.dipU1  = (aclUser->rule.ipv4tcp.dipU & 0x00FF0000) >> 16;
		aclSmi->rule.smi_ipv4tcp.dipU2  = (aclUser->rule.ipv4tcp.dipU & 0x0000FF00) >> 8;
		aclSmi->rule.smi_ipv4tcp.dipU3  = aclUser->rule.ipv4tcp.dipU & 0x000000FF;
		
		aclSmi->rule.smi_ipv4tcp.dipL0  = (aclUser->rule.ipv4tcp.dipL & 0xFF000000) >> 24;
		aclSmi->rule.smi_ipv4tcp.dipL1  = (aclUser->rule.ipv4tcp.dipL & 0x00FF0000) >> 16;
		aclSmi->rule.smi_ipv4tcp.dipL2  = (aclUser->rule.ipv4tcp.dipL & 0x0000FF00) >> 8;
		aclSmi->rule.smi_ipv4tcp.dipL3  = aclUser->rule.ipv4tcp.dipL & 0x000000FF;

		aclSmi->rule.smi_ipv4tcp.tosD= aclUser->rule.ipv4tcp.tosD;
		aclSmi->rule.smi_ipv4tcp.tosM= aclUser->rule.ipv4tcp.tosM;
		aclSmi->rule.smi_ipv4tcp.sPortU= aclUser->rule.ipv4tcp.sPortU;
		aclSmi->rule.smi_ipv4tcp.sPortL= aclUser->rule.ipv4tcp.sPortL;
		aclSmi->rule.smi_ipv4tcp.dPortU= aclUser->rule.ipv4tcp.dPortU;
		aclSmi->rule.smi_ipv4tcp.dPortL= aclUser->rule.ipv4tcp.dPortL;
		aclSmi->rule.smi_ipv4tcp.flagD= aclUser->rule.ipv4tcp.flagD;
		aclSmi->rule.smi_ipv4tcp.flagM= aclUser->rule.ipv4tcp.flagM;

		break;
	 case ACL_IPV4_UDP:

		aclSmi->rule.smi_ipv4udp.sipU0  = (aclUser->rule.ipv4udp.sipU & 0xFF000000) >> 24;
		aclSmi->rule.smi_ipv4udp.sipU1  = (aclUser->rule.ipv4udp.sipU & 0x00FF0000) >> 16;
		aclSmi->rule.smi_ipv4udp.sipU2  = (aclUser->rule.ipv4udp.sipU & 0x0000FF00) >> 8;
		aclSmi->rule.smi_ipv4udp.sipU3  = aclUser->rule.ipv4udp.sipU & 0x000000FF;
		
		aclSmi->rule.smi_ipv4udp.sipL0  = (aclUser->rule.ipv4udp.sipL & 0xFF000000) >> 24;
		aclSmi->rule.smi_ipv4udp.sipL1  = (aclUser->rule.ipv4udp.sipL & 0x00FF0000) >> 16;
		aclSmi->rule.smi_ipv4udp.sipL2  = (aclUser->rule.ipv4udp.sipL & 0x0000FF00) >> 8;
		aclSmi->rule.smi_ipv4udp.sipL3  = aclUser->rule.ipv4udp.sipL & 0x000000FF;

		aclSmi->rule.smi_ipv4udp.dipU0  = (aclUser->rule.ipv4udp.dipU & 0xFF000000) >> 24;
		aclSmi->rule.smi_ipv4udp.dipU1  = (aclUser->rule.ipv4udp.dipU & 0x00FF0000) >> 16;
		aclSmi->rule.smi_ipv4udp.dipU2  = (aclUser->rule.ipv4udp.dipU & 0x0000FF00) >> 8;
		aclSmi->rule.smi_ipv4udp.dipU3  = aclUser->rule.ipv4udp.dipU & 0x000000FF;
		
		aclSmi->rule.smi_ipv4udp.dipL0  = (aclUser->rule.ipv4udp.dipL & 0xFF000000) >> 24;
		aclSmi->rule.smi_ipv4udp.dipL1  = (aclUser->rule.ipv4udp.dipL & 0x00FF0000) >> 16;
		aclSmi->rule.smi_ipv4udp.dipL2  = (aclUser->rule.ipv4udp.dipL & 0x0000FF00) >> 8;
		aclSmi->rule.smi_ipv4udp.dipL3  = aclUser->rule.ipv4udp.dipL & 0x000000FF;

		aclSmi->rule.smi_ipv4udp.tosD= aclUser->rule.ipv4udp.tosD;
		aclSmi->rule.smi_ipv4udp.tosM= aclUser->rule.ipv4udp.tosM;
		aclSmi->rule.smi_ipv4udp.sPortU= aclUser->rule.ipv4udp.sPortU;
		aclSmi->rule.smi_ipv4udp.sPortL= aclUser->rule.ipv4udp.sPortL;
		aclSmi->rule.smi_ipv4udp.dPortU= aclUser->rule.ipv4udp.dPortU;
		aclSmi->rule.smi_ipv4udp.dPortL= aclUser->rule.ipv4udp.dPortL;
		break;
	 case ACL_IPV6_SIP:
		aclSmi->rule.smi_ipv6sip.sipU[0]= aclUser->rule.ipv6sip.sipU[0];
		aclSmi->rule.smi_ipv6sip.sipU[1]= aclUser->rule.ipv6sip.sipU[1];
		aclSmi->rule.smi_ipv6sip.sipU[2]= aclUser->rule.ipv6sip.sipU[2];
		aclSmi->rule.smi_ipv6sip.sipU[3]= aclUser->rule.ipv6sip.sipU[3];
		aclSmi->rule.smi_ipv6sip.sipU[4]= aclUser->rule.ipv6sip.sipU[4];
		aclSmi->rule.smi_ipv6sip.sipU[5]= aclUser->rule.ipv6sip.sipU[5];
		aclSmi->rule.smi_ipv6sip.sipU[6]= aclUser->rule.ipv6sip.sipU[6];
		aclSmi->rule.smi_ipv6sip.sipU[7]= aclUser->rule.ipv6sip.sipU[7];
		aclSmi->rule.smi_ipv6sip.sipL[0]= aclUser->rule.ipv6sip.sipL[0];
		aclSmi->rule.smi_ipv6sip.sipL[1]= aclUser->rule.ipv6sip.sipL[1];
		aclSmi->rule.smi_ipv6sip.sipL[2]= aclUser->rule.ipv6sip.sipL[2];
		aclSmi->rule.smi_ipv6sip.sipL[3]= aclUser->rule.ipv6sip.sipL[3];
		aclSmi->rule.smi_ipv6sip.sipL[4]= aclUser->rule.ipv6sip.sipL[4];
		aclSmi->rule.smi_ipv6sip.sipL[5]= aclUser->rule.ipv6sip.sipL[5];
		aclSmi->rule.smi_ipv6sip.sipL[6]= aclUser->rule.ipv6sip.sipL[6];
		aclSmi->rule.smi_ipv6sip.sipL[7]= aclUser->rule.ipv6sip.sipL[7];
		
		break;
	 case ACL_IPV6_DIP:
		aclSmi->rule.smi_ipv6dip.dipU[0]= aclUser->rule.ipv6dip.dipU[0];
		aclSmi->rule.smi_ipv6dip.dipU[1]= aclUser->rule.ipv6dip.dipU[1];
		aclSmi->rule.smi_ipv6dip.dipU[2]= aclUser->rule.ipv6dip.dipU[2];
		aclSmi->rule.smi_ipv6dip.dipU[3]= aclUser->rule.ipv6dip.dipU[3];
		aclSmi->rule.smi_ipv6dip.dipU[4]= aclUser->rule.ipv6dip.dipU[4];
		aclSmi->rule.smi_ipv6dip.dipU[5]= aclUser->rule.ipv6dip.dipU[5];
		aclSmi->rule.smi_ipv6dip.dipU[6]= aclUser->rule.ipv6dip.dipU[6];
		aclSmi->rule.smi_ipv6dip.dipU[7]= aclUser->rule.ipv6dip.dipU[7];
		aclSmi->rule.smi_ipv6dip.dipL[0]= aclUser->rule.ipv6dip.dipL[0];
		aclSmi->rule.smi_ipv6dip.dipL[1]= aclUser->rule.ipv6dip.dipL[1];
		aclSmi->rule.smi_ipv6dip.dipL[2]= aclUser->rule.ipv6dip.dipL[2];
		aclSmi->rule.smi_ipv6dip.dipL[3]= aclUser->rule.ipv6dip.dipL[3];
		aclSmi->rule.smi_ipv6dip.dipL[4]= aclUser->rule.ipv6dip.dipL[4];
		aclSmi->rule.smi_ipv6dip.dipL[5]= aclUser->rule.ipv6dip.dipL[5];
		aclSmi->rule.smi_ipv6dip.dipL[6]= aclUser->rule.ipv6dip.dipL[6];
		aclSmi->rule.smi_ipv6dip.dipL[7]= aclUser->rule.ipv6dip.dipL[7];
		
		break;
	case ACL_IPV6_EXT:
		aclSmi->rule.smi_ipv6ext.tcD= aclUser->rule.ipv6ext.tcD;
		aclSmi->rule.smi_ipv6ext.tcM= aclUser->rule.ipv6ext.tcM;
		aclSmi->rule.smi_ipv6ext.nhV= aclUser->rule.ipv6ext.nhV;
		aclSmi->rule.smi_ipv6ext.nhp1= aclUser->rule.ipv6ext.nhp1;
		aclSmi->rule.smi_ipv6ext.nhp2_1= aclUser->rule.ipv6ext.nhp2 & 0x000F;
		aclSmi->rule.smi_ipv6ext.nhp2_2= aclUser->rule.ipv6ext.nhp2 >> 4;
		aclSmi->rule.smi_ipv6ext.nhp3= aclUser->rule.ipv6ext.nhp3;
		aclSmi->rule.smi_ipv6ext.nhp4_1= aclUser->rule.ipv6ext.nhp4 & 0x000F;
		aclSmi->rule.smi_ipv6ext.nhp4_2= aclUser->rule.ipv6ext.nhp4 >> 4;

		break;
	case ACL_IPV6_TCP:
		aclSmi->rule.smi_ipv6tcp.tcD= aclUser->rule.ipv6tcp.tcD;
		aclSmi->rule.smi_ipv6tcp.tcM= aclUser->rule.ipv6tcp.tcM;
		aclSmi->rule.smi_ipv6tcp.sPortU= aclUser->rule.ipv6tcp.sPortU;
		aclSmi->rule.smi_ipv6tcp.sPortL= aclUser->rule.ipv6tcp.sPortL;
		aclSmi->rule.smi_ipv6tcp.dPortU= aclUser->rule.ipv6tcp.dPortU;
		aclSmi->rule.smi_ipv6tcp.dPortL= aclUser->rule.ipv6tcp.dPortL;
		aclSmi->rule.smi_ipv6tcp.flagD= aclUser->rule.ipv6tcp.flagD;
		aclSmi->rule.smi_ipv6tcp.flagM= aclUser->rule.ipv6tcp.flagM;

		break;		
	case ACL_IPV6_UDP:
		aclSmi->rule.smi_ipv6udp.tcD= aclUser->rule.ipv6udp.tcD;
		aclSmi->rule.smi_ipv6udp.tcM= aclUser->rule.ipv6udp.tcM;
		aclSmi->rule.smi_ipv6udp.sPortU= aclUser->rule.ipv6udp.sPortU;
		aclSmi->rule.smi_ipv6udp.sPortL= aclUser->rule.ipv6udp.sPortL;
		aclSmi->rule.smi_ipv6udp.dPortU= aclUser->rule.ipv6udp.dPortU;
		aclSmi->rule.smi_ipv6udp.dPortL= aclUser->rule.ipv6udp.dPortL;

		break;		
	case ACL_IPV6_ICMP:
		aclSmi->rule.smi_ipv6icmp.tcD= aclUser->rule.ipv6icmp.tcD;
		aclSmi->rule.smi_ipv6icmp.tcM= aclUser->rule.ipv6icmp.tcM;
		aclSmi->rule.smi_ipv6icmp.typeV= aclUser->rule.ipv6icmp.typeV;
		aclSmi->rule.smi_ipv6icmp.type1= aclUser->rule.ipv6icmp.type1;
		aclSmi->rule.smi_ipv6icmp.type2_1= aclUser->rule.ipv6icmp.type2 & 0x000F;
		aclSmi->rule.smi_ipv6icmp.type2_2= aclUser->rule.ipv6icmp.type2 >>4;
		aclSmi->rule.smi_ipv6icmp.type3= aclUser->rule.ipv6icmp.type3;
		aclSmi->rule.smi_ipv6icmp.type4_1= aclUser->rule.ipv6icmp.type4 & 0x000F;
		aclSmi->rule.smi_ipv6icmp.type4_2= aclUser->rule.ipv6icmp.type4 >>4;
		aclSmi->rule.smi_ipv6icmp.codeV= aclUser->rule.ipv6icmp.codeV;
		aclSmi->rule.smi_ipv6icmp.code1= aclUser->rule.ipv6icmp.code1;
		aclSmi->rule.smi_ipv6icmp.code2= aclUser->rule.ipv6icmp.code2;
		aclSmi->rule.smi_ipv6icmp.code3= aclUser->rule.ipv6icmp.code3;
		aclSmi->rule.smi_ipv6icmp.code4= aclUser->rule.ipv6icmp.code4;
	
		break;
		
	}

	aclSmi->ac_meteridx= aclUser->ac_meteridx;
	aclSmi->ac_policing = aclUser->ac_policing;
	aclSmi->ac_priority = aclUser->ac_priority;
	aclSmi->ac_spri = aclUser->ac_spri;
	aclSmi->ac_mirpmsk_1 = aclUser->ac_mirpmsk & 0x007F;
	aclSmi->ac_mirpmsk_2 = aclUser->ac_mirpmsk>>7;
	aclSmi->ac_mir = aclUser->ac_mir;
	aclSmi->ac_svidx = aclUser->ac_svidx;
	aclSmi->ac_svlan = aclUser->ac_svlan;
	aclSmi->op_term= aclUser->op_term;
	aclSmi->op_exec= aclUser->op_exec;
	aclSmi->op_and= aclUser->op_and;
	aclSmi->op_not= aclUser->op_not;
	aclSmi->op_init= aclUser->op_init;
	aclSmi->format= aclUser->format;
	aclSmi->port_en_1 = aclUser->port_en & 0x0001;
	aclSmi->port_en_2 = aclUser->port_en>>1;
	aclSmi->reserved= aclUser->reserved;

}

/*
@func int32 | rtl8368s_setAsicAclRule | Set acl rule content.
@parm uint32 | index | ACL rule index (0-31) of 32 shared ACL rules.
@parm rtl8368s_acltable | aclTable | ACL rule stucture for setting.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error. 
@rvalue ERRNO_INPUT | Invalid input parameter.
@rvalue ERRNO_ACL_INVALIDRULEIDX | Invalid ACL rule index (0-31).
@comm
	System supported 32 shared 281-bit ACL ingress rule. Index was available at range 0-31 only. If software want to
	modify ACL rule, the ACL function should be disable at first or unspecify acl action	will be executed. 

	One ACL rule structure has four parts setting:
	Bit 0-255		Rule content for frame parsing checking.
	Bit 256-271	Matched action setting.There are meter (policing), priority (802.1q) and mirror/redirect setting.
	Bit 272-276	Operation bits. They are ingress termiation, mathced action and rule comparation status setting.
	Bit 277-280	Rule content (bit 0-255) format. System supported 12 kind of frame types.

	Because ACL data access length was 96 bits only, it must used three times 96-bits data accessing to set one ACL rule. 
	Each 96-bits data accessing needs to set one control word to give ACL accessing information.

*/
int32 rtl8368s_setAsicAclRule( uint32 index, rtl8368s_acltable aclTable)
{
	uint16 regAddr;
	uint32 i;
	uint32 retVal;
	uint32 command;
	uint16* tableAddr;
	uint32 regData;

	rtl8368s_smiacltable smiaclTable;
	
	if(index > RTL8368S_ACLINDEXMAX)
		return ERRNO_ACLIDX;

	memset(&smiaclTable,0x00,sizeof(rtl8368s_smiacltable));
	 _rtl8368s_aclStUser2Smi(&aclTable, &smiaclTable);


	tableAddr = (uint16*)&smiaclTable;

	/*write ACL data 0 for ACL table access*/
	regAddr = RTL8368S_TABLE_WRITE_BASE;

	for(i=0;i<RTL8368S_ACL_DLEN1;i++)
	{

		/*regData = MEM16(*tableAddr);*/
		regData = *tableAddr;
	
		retVal = rtl8368s_setAsicReg(regAddr,regData);
		if(retVal !=SUCCESS)
			return retVal;

		regAddr ++;
		tableAddr ++;

		
	}

	command = RTL8368S_TABLE_ACLDATA0_WRITE_CTRL | ((index <<RTL8368S_TABLE_SELECT_ENTRY_OFF)&RTL8368S_TABLE_ACL_ENTRY_MAK);
	
	retVal = rtl8368s_setAsicReg(RTL8368S_TABLE_ACCESS_CTRL_REG,command);
	if(retVal !=SUCCESS)
		return retVal;


	/*write ACL data 1 for ACL table access*/
	regAddr = RTL8368S_TABLE_WRITE_BASE;

	for(i=0;i<RTL8368S_ACL_DLEN2;i++)
	{

		/*regData = MEM16(*tableAddr);*/
		regData = *tableAddr;

		retVal = rtl8368s_setAsicReg(regAddr,regData);
		if(retVal !=SUCCESS)
			return retVal;

		regAddr ++;
		tableAddr ++;
	}
	command = RTL8368S_TABLE_ACLDATA1_WRITE_CTRL | ((index <<RTL8368S_TABLE_SELECT_ENTRY_OFF)&RTL8368S_TABLE_ACL_ENTRY_MAK);
	
	retVal = rtl8368s_setAsicReg(RTL8368S_TABLE_ACCESS_CTRL_REG,command);
	if(retVal !=SUCCESS)
		return retVal;
	
	/*write ACL data 2 for ACL table access))*/
	regAddr = RTL8368S_TABLE_WRITE_BASE;

	for(i=0;i<RTL8368S_ACL_DLEN2;i++)
	{
		/*regData = MEM16(*tableAddr);*/
		regData = *tableAddr;

		retVal = rtl8368s_setAsicReg(regAddr,regData);
		if(retVal !=SUCCESS)
			return retVal;

		regAddr ++;
		tableAddr ++;
	}
	command = RTL8368S_TABLE_ACLDATA2_WRITE_CTRL | ((index <<RTL8368S_TABLE_SELECT_ENTRY_OFF)&RTL8368S_TABLE_ACL_ENTRY_MAK);
	
	retVal = rtl8368s_setAsicReg(RTL8368S_TABLE_ACCESS_CTRL_REG,command);
	if(retVal !=SUCCESS)
		return retVal;

#ifdef CONFIG_RTL8368S_ASICDRV_TEST
	Rtl8368sVirtualAclTable[index] = smiaclTable;
#endif
	return SUCCESS;
}

/*
@func int32 | rtl8368s_getAsicAclRule | Get acl rule content.
@parm uint32 | index | ACL rule index (0-31) of 32 shared ACL rules.
@parm rtl8368s_acltable* | aclTable | ACL rule stucture address for accessing.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error. 
@rvalue ERRNO_INPUT | Invalid input parameter.
@rvalue ERRNO_ACL_INVALIDRULEIDX | Invalid ACL rule index (0-31).
@comm
	The API can be used for read ACL rule setting. System supported 32 shared 281-bit ACL ingress rule. 
	Range of index is available from 0 to 31 only. 

*/
int32 rtl8368s_getAsicAclRule( uint32 index, rtl8368s_acltable *aclTable)
{
	uint16 regAddr;
	uint32 regData;
	uint32 i;
	uint32 retVal;
	uint32 command;
	rtl8368s_smiacltable smiaclTable;
	uint16* tableAddr;


	if(index > RTL8368S_ACLINDEXMAX)
		return ERRNO_ACL_INVALIDRULEIDX;


	memset(&smiaclTable,0x00,sizeof(rtl8368s_smiacltable));

	tableAddr = (uint16*)&smiaclTable;


	command = RTL8368S_TABLE_ACLDATA0_READ_CTRL | ((index <<RTL8368S_TABLE_SELECT_ENTRY_OFF)&RTL8368S_TABLE_ACL_ENTRY_MAK);
	
	retVal = rtl8368s_setAsicReg(RTL8368S_TABLE_ACCESS_CTRL_REG,command);
	if(retVal !=SUCCESS)
		return retVal;	
	/*read ACL data 0 from ACL table access*/
	regAddr = RTL8368S_TABLE_READ_BASE;

	for(i=0;i<RTL8368S_ACL_DLEN1;i++)
	{
		retVal = rtl8368s_getAsicReg(regAddr, &regData);
		if(retVal !=SUCCESS)
			return retVal;

		/**tableAddr = MEM16(regData);*/
		*tableAddr = regData;

			
		regAddr ++;
		tableAddr ++;
	}

	command = RTL8368S_TABLE_ACLDATA1_READ_CTRL | ((index <<RTL8368S_TABLE_SELECT_ENTRY_OFF)&RTL8368S_TABLE_ACL_ENTRY_MAK);
	
	retVal = rtl8368s_setAsicReg(RTL8368S_TABLE_ACCESS_CTRL_REG,command);
	if(retVal !=SUCCESS)
		return retVal;

	/*read ACL data 0 from ACL table access*/
	regAddr = RTL8368S_TABLE_READ_BASE;

	for(i=0;i<RTL8368S_ACL_DLEN2;i++)
	{
		retVal = rtl8368s_getAsicReg(regAddr, &regData);
		if(retVal !=SUCCESS)
			return retVal;

		/**tableAddr = MEM16(regData);*/
		*tableAddr = regData;

		regAddr ++;
		tableAddr ++;
	}

	command = RTL8368S_TABLE_ACLDATA2_READ_CTRL | ((index <<RTL8368S_TABLE_SELECT_ENTRY_OFF)&RTL8368S_TABLE_ACL_ENTRY_MAK);
	
	retVal = rtl8368s_setAsicReg(RTL8368S_TABLE_ACCESS_CTRL_REG,command);
	if(retVal !=SUCCESS)
		return retVal;

	/*read ACL data 0 from ACL table access*/
	regAddr = RTL8368S_TABLE_READ_BASE;

	for(i=0;i<RTL8368S_ACL_DLEN2;i++)
	{
		retVal = rtl8368s_getAsicReg(regAddr, &regData);
		if(retVal !=SUCCESS)
			return retVal;


		/**tableAddr = MEM16(regData);*/
		*tableAddr = regData;

		regAddr ++;
		tableAddr ++;
	}

#ifdef CONFIG_RTL8368S_ASICDRV_TEST

	smiaclTable = Rtl8368sVirtualAclTable[index];


#endif

	 _rtl8368s_aclStSmi2User(aclTable, &smiaclTable);

	return SUCCESS;
}

/*=======================================================================
 *  Reserved Multicast Address APIs
 *========================================================================*/
/*
@func int32 | rtl8368s_setAsicRma | Set reserved multicast address for CPU trapping.
@parm enum RMATRAPFRAME | rma | type of RMA for trapping frame type setting.
@parm enum RMAOP | op | RMA operation 0:forwarding 1:Trap to CPU 2:Drop 3:Forward, exclude CPU.
@parm uint32 | keepCtag | Keep CVLAN tag/untag format 1:enable 0:disable.
@parm uint32 | bypassStorm | By pass storm control calculation 1:enable 0:disable.
@parm uint32 | priSel | New CPU priority assignment 1:enable 0:disable.
@parm enum PRIORITYVALUE | priority | Priority for new CPU priority assignment. This CPU priority will not be remapped.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error. 
@rvalue ERRNO_INPUT | Invalid input parameter.
@rvalue ERRNO_PRIORITY | Invalid priority. 
@comm
	System support 16 types of frame format to trap matched receiving frame to CPU by a 20-bits register. Destination MAC address of 
	receiving frame is matched RMA setting and trap function bit was set, ASIC will trap frame to CPU port with CPU port mask setting. 
	DMAC						Assignment
	01-80-C2-00-00-00			Bridge Group Address
	01-80-C2-00-00-01			IEEE Std 802.3, 1988 Edition, Full Duplex PAUSE operation
	01-80-C2-00-00-02			IEEE Std 802.3ad Slow Protocols-Multicast address
	01-80-C2-00-00-03			IEEE Std 802.1X PAE address
	01-80-C2-00-00-08			Provider Bridge Group Address
	01-80-C2-00-00-0D			Provider Bridge GVRP Address
	01-80-C2-00-00-0E			IEEE Std 802.1ab Link Layer Discovery Protocol Multicast address
	01-80-C2-00-00-10			All LANs Bridge Management Group Address
	01-80-C2-00-00-20			GMRP Address
	01-80-C2-00-00-21			GVRP address
	01-80-C2-00-00-04,05,06,07,	Undefined 802.1 bridge address
				09,0A,0B,0C,0F		
	01-80-C2-00-00-22~2F		Undefined GARP address
	xx-xx-xx-xx-xx-xx			IGMPin PPPoE frame(Type/Length = 8864)
	xx-xx-xx-xx-xx-xx			MLD(IPv6 ICMP) in PPPoE frame(Type/Length = 8864)
	xx-xx-xx-xx-xx-xx			IGMP packet but not PPPoE frame
	xx-xx-xx-xx-xx-xx			MLD(IPv6 ICMP) but not PPPoE frame
	xx-xx-xx-xx-xx-xx			User defined address 1
	xx-xx-xx-xx-xx-xx			User defined address 2
	xx-xx-xx-xx-xx-xx			User defined address 3
	xx-xx-xx-xx-xx-xx			User defined address 4	

	Four user defined DMAC RMA functions will reference correspond user defined address and mask.
	
*/
int32 rtl8368s_setAsicRma(enum RMATRAPFRAME rma, enum RMAOP op,uint32 keepCtag, uint32 bypassStorm,uint32 priSel,enum PRIORITYVALUE priority)
{
	uint32 retVal;
	uint16 regAddr;
	uint32 regData;
	uint32 regBits;

	if(rma>=RMA_MAX)
		return ERROR_RMA_INDEX;

	if(op >=RMAOP_MAX)
		return ERRNO_INPUT;
	
	if(priority >=PRI_MAX)
		return ERRNO_PRIORITY;

	if(keepCtag)
		keepCtag = 1;
	if(bypassStorm)
		bypassStorm = 1;
	if(priSel)
		priSel = 1; 

	regAddr = RTL8368S_RMA_CONTROL_REG(rma);
	regBits = RTL8368S_RMA_CONTROL_MSK(rma);

	regData = (op<<RTL8368S_RMA_OP_OFFSET(rma)) | (keepCtag<<RTL8368S_RMA_KEEP_OFFSET(rma)) | (bypassStorm<<RTL8368S_RMA_BPSTM_OFFSET(rma)) | (priSel<<RTL8368S_RMA_PRISEL_OFFSET(rma)) | (priority<<RTL8368S_RMA_NEWPRI_OFFSET(rma));

	retVal = rtl8368s_setAsicRegBits(regAddr,regBits,regData);		
	
	return retVal;	
}

/*
@func int32 | rtl8368s_getAsicRma | Set reserved multicast address for CPU trapping.
@parm enum RMATRAPFRAME | rma | type of RMA for trapping frame type setting.
@parm enum RMAOP* | op | RMA operation 0:forwarding 1:Trap to CPU 2:Drop 3:Forward, exclude CPU.
@parm uint32* | keepCtag | Keep CVLAN tag/untag format 1:enable 0:disable.
@parm uint32* | bypassStorm | By pass storm control calculation 1:enable 0:disable.
@parm uint32* | priSel | New CPU priority assignment 1:enable 0:disable.
@parm enum PRIORITYVALUE* | priority | Priority for new CPU priority assignment. This CPU priority will not be remapped.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error. 
@rvalue ERRNO_INPUT | Invalid input parameter.
@rvalue ERRNO_PRIORITY | Invalid priority. 
@comm
	The API can get  RMA setting of 20 filtering destination mac address.
	
*/
int32 rtl8368s_getAsicRma(enum RMATRAPFRAME rma, enum RMAOP* op,uint32* keepCtag, uint32* bypassStorm,uint32* priSel,enum PRIORITYVALUE* priority)
{
	uint32 retVal;
	uint32 regData;

	if(rma>=RMA_MAX)
		return ERROR_RMA_INDEX;

	retVal = rtl8368s_getAsicReg(RTL8368S_RMA_CONTROL_REG(rma),&regData);
	if(retVal != SUCCESS)
		return retVal;

	*op = (regData & RTL8368S_RMA_OP_MSK(rma))>>RTL8368S_RMA_OP_OFFSET(rma);

	*keepCtag = (regData & RTL8368S_RMA_KEEP_MSK(rma))>>RTL8368S_RMA_KEEP_OFFSET(rma);

	*bypassStorm = (regData & RTL8368S_RMA_BPSTM_MSK(rma))>>RTL8368S_RMA_BPSTM_OFFSET(rma);

	*priSel = (regData & RTL8368S_RMA_PRISEL_MSK(rma))>>RTL8368S_RMA_PRISEL_OFFSET(rma);

	*priority = (regData & RTL8368S_RMA_NEWPRI_MSK(rma))>>RTL8368S_RMA_NEWPRI_OFFSET(rma);

	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicRmaUserDefinedAddress | Set 4 user defined destination mac addresses and mask for RMA setting.
@parm uint32 | index | User define address index (0~2).
@parm ether_addr_t | mac | Userd defined mac address ( mapping to registers RADDRn_2,RADDRn_1,RADDRn_0) for RMA filtering.
@parm uint32 | mask | User defined mac address mask (RMASKn). The mask setting is used for trap receiving frame in group DMACs.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error. 
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	System supports 4 group of user defined mac addresses for trapping to CPU. Bits 11-15 of register RMA_EN are enable setting for
	trapping receiving frame with matched DMAC to CPU. If software want to trap one of the undefined 802.1 bridge address, for example 
	01-80-C2-00-00-04, and don't want to trap all of the undefined address by setting RMA_EN bit 7, then it can use one of the user defined 
	address with setting destination mac address 01-80-C2-00-00-04 and mask ff-ff-ff-ff.  
*/
int32 rtl8368s_setAsicRmaUserDefinedAddress(uint32 index, ether_addr_t mac, uint32 mask)
{
	uint32 retVal;
	uint32 regAddr;
	uint32 regData;
	uint16 *accessPtr;
	uint16 i;

	smi_ether_addr_t smimac;
	
	if(index >= RTL8368S_RMAUSERDEFMAX)
		return ERRNO_ACL_INVALIDRULEIDX;

	/* MAC order is same with LUT 
	*  11:22:33:44:55:66
	*  0x182 -> 6655, 0x183 -> 4433, 0x184 -> 2211
	*/
	smimac.mac0 = mac.octet[5];
	smimac.mac1 = mac.octet[4];
	smimac.mac2 = mac.octet[3];
	smimac.mac3 = mac.octet[2];
	smimac.mac4 = mac.octet[1];
	smimac.mac5 = mac.octet[0];

	accessPtr = (uint16*)&smimac;


	regAddr = RTL8368S_RMA_USER_DEFINED_BASE + index*4;

	for(i=0; i<3; i++)
	{
		regData = *accessPtr;
		retVal = rtl8368s_setAsicReg(regAddr,regData);

		if(retVal != SUCCESS)
			return retVal;

		accessPtr ++;
		regAddr ++;
	}	
	
	retVal = rtl8368s_setAsicReg(regAddr,mask);

	if(retVal != SUCCESS)
		return retVal;
	
	return SUCCESS;
}

/*
@func int32 | rtl8368s_getAsicRmaUserDefinedAddress | Get 4 user defined destination mac addresses and mask for RMA setting.
@parm uint32 | index | User define address index (0~2).
@parm uint32* | mac | Userd defined mac address ( mapping to registers RADDRn_2,RADDRn_1,RADDRn_0) for RMA filtering.
@parm uint32* | mask | User defined mac address mask (RMASKn). The mask setting is used for trap receiving frame in group DMACs.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error. 
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can get RMA user defined addresses setting.
*/
int32 rtl8368s_getAsicRmaUserDefinedAddress(uint32 index, ether_addr_t *mac, uint32 *mask)
{
	uint32 retVal;
	uint32 regAddr;
	uint32 regData;	
	uint16 *accessPtr;
	uint16 i;
	smi_ether_addr_t smimac;

	
	if(index >= RTL8368S_RMAUSERDEFMAX)
		return ERRNO_ACL_INVALIDRULEIDX;
#if 0
	accessPtr = (uint16*)mac;
#endif
	accessPtr = (uint16*)&smimac;

	regAddr = RTL8368S_RMA_USER_DEFINED_BASE + index*4;

	for(i=0; i<3; i++)
	{
		retVal = rtl8368s_getAsicReg(regAddr,&regData);

		if(retVal != SUCCESS)
			return retVal;

		*accessPtr = regData;
		accessPtr ++;
		regAddr ++;
	}	

	/* MAC order is same with LUT 
	*  11:22:33:44:55:66
	*  0x182 -> 6655, 0x183 -> 4433, 0x184 -> 2211
	*/
	mac->octet[0] = smimac.mac5;
	mac->octet[1] = smimac.mac4;
	mac->octet[2] = smimac.mac3;
	mac->octet[3] = smimac.mac2;
	mac->octet[4] = smimac.mac1;
	mac->octet[5] = smimac.mac0;
	
	retVal = rtl8368s_getAsicReg(regAddr,mask);

	if(retVal != SUCCESS)
		return retVal;
	
	return SUCCESS;
}

/*=======================================================================
 *  C-VLAN APIs
 *========================================================================*/

static void _rtl8368s_VlanStUser2Smi( rtl8368s_user_vlan4kentry*VlanUser, rtl8368s_vlan4kentry*VlanSmi)
{
	VlanSmi->vid		=	VlanUser->vid;
	VlanSmi->untag	=	VlanUser->untag;
	VlanSmi->member	=	VlanUser->member;
	VlanSmi->fid		=	VlanUser->fid;
}

static void _rtl8368s_VlanStSmi2User( rtl8368s_user_vlan4kentry*VlanUser, rtl8368s_vlan4kentry*VlanSmi)
{
	VlanUser->vid	=	VlanSmi->vid	;
	VlanUser->untag	=	VlanSmi->untag;
	VlanUser->member=	VlanSmi->member;
	VlanUser->fid	=	VlanSmi->fid;
}



/*
@func int32 | rtl8368s_setAsicVlan | Set VLAN enable function.
@parm uint32 | enabled | VLAN enable function usage 1: enabled, 0: disabled. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	ASIC will parse frame type/length if VLAN function usage is enabled. In 802.1q spec. ,the Type/Length of C-tag is 0x8100. System will decide
	802.1q VID of received frame from C-tag, Protocol-and-Port based VLAN and Port based VLAN. This setting will impact on VLAN ingress, VLAN egress
	and 802.1q priority selection.
	
*/
int32 rtl8368s_setAsicVlan(uint32 enabled)
{
	uint32 retVal;

	retVal = rtl8368s_setAsicRegBit(RTL8368S_SWITCH_GLOBAL_CTRL_REG,RTL8368S_EN_VLAN_OFF,enabled);
	
	return retVal;
}

/*
@func int32 | rtl8368s_getAsicVlan | Get VLAN enable function configuration.
@parm uint32* | enabled | VLAN enable function usage 1: enabled, 0: disabled. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can get usage of asic VLAN enable configuration. 
	
*/
int32 rtl8368s_getAsicVlan(uint32* enabled)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_SWITCH_GLOBAL_CTRL_REG,&regData);
	
	if(retVal !=  SUCCESS)
		return retVal;

	regData = regData & (1<<RTL8368S_EN_VLAN_OFF);

	if(regData)
		*enabled = 1;
	else
		*enabled = 0;

	return SUCCESS;	
}

/*
@func int32 | rtl8368s_setAsicVlan4kTbUsage | Set 4k VLAN table usage configuration.
@parm uint32 | enabled | 4k VLAN table usage 1: enabled, 0: disabled. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	Each VLAN entry contains member port, untag set and FID (support 8 SVL/IVL filtering database) information. While VLAN function of system 
	is enabled and 4k VLAN table is enabled, system will decide each receiving frame's VID. VLAN ingress and VLAN egress function will 
	reference member port of mapped VID entry in 4k table. Without 4k VLAN table usage, there are 16 VLAN memeber configurations to	support
	VLAN enabled reference.
	 
*/
int32 rtl8368s_setAsicVlan4kTbUsage(uint32 enabled)
{
	uint32 retVal;

	retVal = rtl8368s_setAsicRegBit(RTL8368S_SWITCH_GLOBAL_CTRL_REG,RTL8368S_EN_VLAN_4KTB_OFF,enabled);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicVlan4kTbUsage | Get 4k VLAN table usage configuration.
@parm uint32* | enabled | 4k VLAN table usage 1: enabled, 0: disabled. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can get 4K VLAN table usage.
	
*/
int32 rtl8368s_getAsicVlan4kTbUsage(uint32* enabled)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_SWITCH_GLOBAL_CTRL_REG,&regData);

	if(retVal !=  SUCCESS)
		return retVal;

	if(regData &(1<<RTL8368S_EN_VLAN_4KTB_OFF))
		*enabled = 1;
	else
		*enabled = 0;

	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicVlanMemberConfig | Set 16 VLAN member configurations.
@parm uint32 | index | VLAN member configuration index (0~15).
@parm rtl8368s_vlanconfig* | vlanmemberconf | VLAN member configuration. It contained VID, priority, member set, untag set and FID fields. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@rvalue ERRNO_VLAN_INVALIDFID | Invalid FID (0~7).
@rvalue ERRNO_VLAN_INVALIDPRIORITY | Invalid VLAN priority (0~7).
@rvalue ERRNO_VLAN_INVALIDPORTMSK | Invalid port mask (0x00~0x3F).
@rvalue ERRNO_VLAN_INVALIDVID | Invalid VID parameter (0~4095).
@rvalue ERRNO_VLAN_VIDX | Invalid VLAN member configuration index (0~15).
@comm
	VLAN ingress and egress will reference these 16 configurations while system VLAN function is enabled without 4k VLAN table usage. Port based
	, Protocol-and-Port based VLAN and 802.1x guest VLAN functions retrieved VLAN information from these 16 member configurations too. Only
	VID will be referenced while 4k VLAN table is enabled. It means that member set, untag set and FID need to be retrieved from 4k mapped VID entry.
	
*/
int32 rtl8368s_setAsicVlanMemberConfig(uint32 index,rtl8368s_vlanconfig vlanmconf )
{
	uint32 retVal;
	uint32 regAddr;
	uint32 regData;
	uint16* tableAddr;
	
	if(index > RTL8368S_VLANMCIDXMAX)
		return ERRNO_CVIDX;

	regAddr = RTL8368S_VLAN_MEMCONF_BASE + (index*3);

	tableAddr = (uint16*)&vlanmconf;
	regData = *tableAddr;

	retVal = rtl8368s_setAsicReg(regAddr,regData);
	if(retVal !=  SUCCESS)
		return retVal;

	
	regAddr = RTL8368S_VLAN_MEMCONF_BASE + 1 + (index*3);

	tableAddr ++;
	regData = *tableAddr;

	retVal = rtl8368s_setAsicReg(regAddr,regData);
	if(retVal !=  SUCCESS)
		return retVal;

	
	regAddr = RTL8368S_VLAN_MEMCONF_BASE + 2 + (index*3);

	tableAddr ++;
	regData = *tableAddr;

	retVal = rtl8368s_setAsicReg(regAddr,regData);
	if(retVal !=  SUCCESS)
		return retVal;
	
	return SUCCESS;
}

/*
@func int32 | rtl8368s_getAsicVlanMemberConfig | Get 16 VLAN member configurations.
@parm uint32 | index | VLAN member configuration index (0~15).
@parm rtl8368s_vlanconfig* | vlanmemberconf | VLAN member configuration. It contained VID, priority, member set, untag set and FID fields. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@rvalue ERRNO_VLAN_VIDX | Invalid VLAN member configuration index (0~15).
@comm
	The API can get 16 VLAN member configuration.
	
*/
int32 rtl8368s_getAsicVlanMemberConfig(uint32 index,rtl8368s_vlanconfig *vlanmconf )
{
	uint32 retVal;
	uint32 regAddr;
	uint32 regData;
	uint16* tableAddr;

	if(index > RTL8368S_VLANMCIDXMAX)
		return ERRNO_CVIDX;

	tableAddr = (uint16*)vlanmconf;
	
	regAddr = RTL8368S_VLAN_MEMCONF_BASE + (index*3);

	retVal = rtl8368s_getAsicReg(regAddr,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	*tableAddr = regData;
	tableAddr ++;
	
		
	regAddr = RTL8368S_VLAN_MEMCONF_BASE + 1 + (index*3);

	retVal = rtl8368s_getAsicReg(regAddr,&regData);
	if(retVal !=  SUCCESS)
		return retVal;
	
	*tableAddr = regData;
	tableAddr ++;

	regAddr = RTL8368S_VLAN_MEMCONF_BASE + 2 + (index*3);

	retVal = rtl8368s_getAsicReg(regAddr,&regData);
	if(retVal !=  SUCCESS)
		return retVal;
	
	*tableAddr = regData;

	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicVlan4kEntry | Set VID mapped entry to 4K VLAN table.
@parm rtl8368s_vlan4kentry* | vlan4kEntry | VLAN entry seting for 4K table. There is VID field in entry structure and  entry is directly mapping to 4K table location (1 to 1).
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@rvalue ERRNO_VLAN_INVALIDFID | Invalid FID (0~7).
@rvalue ERRNO_VLAN_INVALIDPORTMSK | Invalid port mask (0x00~0x3F).
@rvalue ERRNO_VLAN_INVALIDVID | Invalid VID parameter (0~4095).
@comm
	VID field of C-tag is 12-bits and available VID range is 0~4095. In 802.1q spec. , null VID (0x000) means tag header contain priority information
	only and VID 0xFFF is reserved for implementtation usage. But ASIC still retrieved these VID entries in 4K VLAN table if VID is decided from 16
	member configurations. It has no available VID 0x000 and 0xFFF from C-tag. ASIC will retrieve these two non-standard VIDs (0x000 and 0xFFF) from 
	member configuration indirectly referenced by Port based, Protocol-and-Port based VLAN and 802.1x functions.
	
*/
int32 rtl8368s_setAsicVlan4kEntry(rtl8368s_user_vlan4kentry vlan4kEntry )
{
	uint32 retVal;
	uint32 regData;
	uint16* tableAddr;
	uint32 i;
	rtl8368s_vlan4kentry smiVlan4kentry;

	memset(&smiVlan4kentry, 0x0, sizeof(rtl8368s_vlan4kentry));

	_rtl8368s_VlanStUser2Smi( &vlan4kEntry, &smiVlan4kentry);

	tableAddr = (uint16*)&smiVlan4kentry;

	regData = *tableAddr;
	
	retVal = rtl8368s_setAsicReg(RTL8368S_VLAN_TABLE_WRITE_BASE,regData);
	if(retVal !=  SUCCESS)
		return retVal;

	tableAddr ++;

	regData = *tableAddr;
	
	retVal = rtl8368s_setAsicReg(RTL8368S_VLAN_TABLE_WRITE_BASE+1,regData);

	if(retVal !=  SUCCESS)
		return retVal;
	
	tableAddr ++;

	regData = *tableAddr;
	
	retVal = rtl8368s_setAsicReg(RTL8368S_VLAN_TABLE_WRITE_BASE+2,regData);

	if(retVal !=  SUCCESS)
		return retVal;	
	
	/*write table access Control word*/
	retVal = rtl8368s_setAsicReg(RTL8368S_TABLE_ACCESS_CTRL_REG,RTL8368S_TABLE_VLAN_WRITE_CTRL);
	if(retVal !=  SUCCESS)
		return retVal;

	/*check ASIC command*/
	i=0;
	while(i<RTL8368S_TBL_CMD_CHECK_COUNTER)
	{
		retVal = rtl8368s_getAsicReg(RTL8368S_TABLE_ACCESS_CTRL_REG,&regData);
		if(retVal !=  SUCCESS)
			return retVal;

		if(regData &RTL8368S_TABLE_ACCESS_CMD_MSK)
		{
			i++;
			if(i==RTL8368S_TBL_CMD_CHECK_COUNTER)
				return FAILED;
		}
		else
			break;
		
	}

#ifdef CONFIG_RTL8368S_ASICDRV_TEST
	Rtl8368sVirtualVlanTable[vlan4kEntry.vid] = smiVlan4kentry;
#endif
	return SUCCESS;
}

/*
@func int32 | rtl8368s_getAsicVlan4kEntry | Get VID mapped entry to 4K VLAN table. 
@parm rtl8368s_user_vlan4kentry* | vlan4kEntry | VLAN entry seting for 4K table. There is VID field in entry structure and  entry is directly mapping to 4K table location (1 to 1).
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@rvalue ERRNO_VLAN_INVALIDVID | Invalid VID parameter (0~4095).
@comm
	The API can get entry of 4k VLAN table. Software must prepare the retrieving VID first at writing data and used control word to access desired VLAN entry.
	
*/
int32 rtl8368s_getAsicVlan4kEntry(rtl8368s_user_vlan4kentry *vlan4kEntry )
{
	uint32 retVal;
	uint32 regData;
	uint32 vid;
	uint32 i;
	uint16* tableAddr;
	rtl8368s_vlan4kentry smiVlan4kentry;

	memset(&smiVlan4kentry, 0x0, sizeof(rtl8368s_vlan4kentry));

	_rtl8368s_VlanStUser2Smi( vlan4kEntry, &smiVlan4kentry);

	vid = smiVlan4kentry.vid;
	
	if(vid > RTL8368S_VIDMAX)
		return ERRNO_VID;

	tableAddr = (uint16*)&smiVlan4kentry;


	/*write VID first*/
	regData = *tableAddr;	
	retVal = rtl8368s_setAsicReg(RTL8368S_VLAN_TABLE_WRITE_BASE,regData);

	if(retVal !=  SUCCESS)
		return retVal;

	/*write table access Control word*/
	retVal = rtl8368s_setAsicReg(RTL8368S_TABLE_ACCESS_CTRL_REG,RTL8368S_TABLE_VLAN_READ_CTRL);
	if(retVal !=  SUCCESS)
		return retVal;

	/*check ASIC command*/
	i=0;
	while(i<RTL8368S_TBL_CMD_CHECK_COUNTER)
	{
		retVal = rtl8368s_getAsicReg(RTL8368S_TABLE_ACCESS_CTRL_REG,&regData);
		if(retVal !=  SUCCESS)
			return retVal;

		if(regData &RTL8368S_TABLE_ACCESS_CMD_MSK)
		{
			i++;
			if(i==RTL8368S_TBL_CMD_CHECK_COUNTER)
				return FAILED;
		}
		else
			break;
		
	}
#ifdef CONFIG_RTL8368S_ASICDRV_TEST

	*(rtl8368s_vlan4kentry *)&Rtl8368sVirtualReg[RTL8368S_VLAN_TABLE_READ_BASE] = Rtl8368sVirtualVlanTable[vid];

#endif

	retVal = rtl8368s_getAsicReg(RTL8368S_VLAN_TABLE_READ_BASE,&regData);

	if(retVal !=  SUCCESS)
		return retVal;

	*tableAddr = regData;
	tableAddr ++;

	
	retVal = rtl8368s_getAsicReg(RTL8368S_VLAN_TABLE_READ_BASE+1,&regData);

	if(retVal !=  SUCCESS)
		return retVal;

	*tableAddr = regData;
	tableAddr ++;


	retVal = rtl8368s_getAsicReg(RTL8368S_VLAN_TABLE_READ_BASE+2,&regData);

	if(retVal !=  SUCCESS)
		return retVal;

	*tableAddr = regData;

	_rtl8368s_VlanStSmi2User( vlan4kEntry, &smiVlan4kentry);

	vlan4kEntry->vid = vid;
	
	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicVlanAcceptTaggedOnly | Set ASIC permit C-tagged frames only.
@parm enum PORTID | port | Physical port number (0~5).
@parm uint32 | enabled | ASIC permit C-tagged frame only 1: enabled, 0: disabled
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@comm
	Ingress rule for acceptable frame types control. If the input parameter is set to 'enabled', any received frames carrying no VID(i.e., Untagged 
	frames or Priority-Tagged Frames) will be dropped. If the parameter is set to 'disabled'  all incoming Priority-Tagged and Untagged frames are
	associated with a VLAN by the ingress rule on the received port. Filtering function is actived while system VLAN function is enabled.
	
*/
int32 rtl8368s_setAsicVlanAcceptTaggedOnly(enum PORTID port, uint32 enabled)
{
	int32 retVal;

	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	retVal = rtl8368s_setAsicRegBit(RTL8368S_VLAN_INGRESS_CTRL_1_REG,port,enabled);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicVlanAcceptTaggedOnly | Get ASIC permit C-tagged frames setting.
@parm enum PORTID | port | Physical port number (0~5).
@parm uint32* | enabled | ASIC permit C-tagged frame only 1: enabled, 0: disabled
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@comm
	The API can get C-tagged frame control setting.
	
*/
int32 rtl8368s_getAsicVlanAcceptTaggedOnly(enum PORTID port, uint32* enabled)
{
	int32 retVal;
	uint32 regData;
	
	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	retVal = rtl8368s_getAsicReg(RTL8368S_VLAN_INGRESS_CTRL_1_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	if(regData & (1<<port))
		*enabled = 1;
	else
		*enabled = 0;

	return SUCCESS;
}
/*
@func int32 | rtl8368s_setAsicVlanDropTaggedPackets | Set dropping C-tagged frame ability.
@parm enum PORTID | port | Physical port number (0~5).
@parm uint32 | enabled | ASIC permit  un-tagged frame only 1: enabled, 0: disabled.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@common
	Ingress rule for acceptable frame types control. If the input parameter is set to 'enabled', any received frames carrying C-tag (with VID is not 0x000) 
	will be dropped. Filtering function is actived while system VLAN function is enabled.
	
*/
int32 rtl8368s_setAsicVlanDropTaggedPackets(enum PORTID port, uint32 enabled)
{
	int32 retVal;
	uint32 bitNo;
	
	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	bitNo = port + PORT_MAX;
	retVal = rtl8368s_setAsicRegBit(RTL8368S_VLAN_INGRESS_CTRL_1_REG,bitNo,enabled);

	return retVal;
}
/*
@func int32 | rtl8368s_getAsicVlanDropTaggedPackets | Get dropping C-tagged frame ability.
@parm enum PORTID | port | Physical port number (0~5).
@parm uint32* | enabled | ASIC permit  un-tagged frame only 1: enabled, 0: disabled
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@common
	The API can get status of ingress filtering VLAN tagged packets function. 

*/
int32 rtl8368s_getAsicVlanDropTaggedPackets(enum PORTID port, uint32* enabled)
{
	int32 retVal;
	uint32 regData;
	
	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	retVal = rtl8368s_getAsicReg(RTL8368S_VLAN_INGRESS_CTRL_1_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	if(regData & (1<<(port+PORT_MAX)))
		*enabled = 1;
	else
		*enabled = 0;

	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicVlanIngressFiltering | Set VLAN ingress function. 
@parm enum PORTID | port | Physical port number (0~7).
@parm uint32 | enabled | VLAN ingress function setting 1: enabled, 0: disabled
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@common
	If enable VLAN ingress function, all frames received on a port whose VLAN classification does not include that port in its member set will be discarded. 
	Filtering function is actived while system VLAN function is enabled.

*/
int32 rtl8368s_setAsicVlanIngressFiltering(enum PORTID port, uint32 enabled)
{
	int32 retVal;

	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;
	
	retVal = rtl8368s_setAsicRegBit(RTL8368S_VLAN_INGRESS_CTRL_2_REG,port,enabled);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicVlanIngressFiltering | Get VLAN ingress function. 
@parm enum PORTID | port | Physical port number (0~5).
@parm uint32* | enabled | VLAN ingress function setting 1: enabled, 0: disabled
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@common
	The API can get VLAN ingress function enable setting.
	
*/
int32 rtl8368s_getAsicVlanIngressFiltering(enum PORTID port, uint32* enabled)
{
	int32 retVal;
	uint32 regData;
	
	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	retVal = rtl8368s_getAsicReg(RTL8368S_VLAN_INGRESS_CTRL_2_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	if(regData & (1<<port))
		*enabled = 1;
	else
		*enabled = 0;

	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicVlanIpMulticastLeaky | Set port based priority.
@parm enum PORTID | port | Physical port number (0~5).
@parm uint32 | enabled | Per-Port IP multicast VLAN leaky function setting 1: enabled, 0: disabled
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_VLAN_INVALIDPORT | Invalid port number.
@comm
	The API can set per-port IP multicast VLAN leaky while VLAN function is enable. VLAN egress function is always enabled, only
enabled IP multicast frames VLAN leaky function and IP multicast frames can be forwarded corss VLANs.
 */
int32 rtl8368s_setAsicVlanIpMulticastLeaky( enum PORTID port, uint32 enabled )
{
	int32 retVal;

	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;
	
	retVal = rtl8368s_setAsicRegBit(RTL8368S_VLAN_INGRESS_CTRL_2_REG,port+RTL8368S_VLAN_IPMUEG_OFF,enabled);

	return retVal;
}



/*
@func int32 | rtl8368s_getVlanAsicIpMulticastLeaky | Set port based priority.
@parm enum PORTID | port | Physical port number (0~5).
@parm uint32* | enabled | Per-Port IP multicast VLAN leaky function setting 1: enabled, 0: disabled
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_VLAN_INVALIDPORT | Invalid port number.
@comm
	The API can get per-port IP multicast VLAN leaky configuration. VLAN egress function is always enabled, only
enabled IP multicast frames VLAN leaky function and IP multicast frames can be forwarded corss VLANs.
 */
int32 rtl8368s_getAsicVlanIpMulticastLeaky( enum PORTID port, uint32* enabled )
{

	int32 retVal;
	uint32 regData;
	
	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	retVal = rtl8368s_getAsicReg(RTL8368S_VLAN_INGRESS_CTRL_2_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	if(regData & (1<<(port + RTL8368S_VLAN_IPMUEG_OFF)))
		*enabled = 1;
	else
		*enabled = 0;

	return SUCCESS;
}


/*
@func int32 | rtl8368s_setAsicVlanPortBasedVID | Set port based VID which is indexed to 16 VLAN member configurations.
@parm enum PORTID | port | Physical port number (0~5).
@parm uint32 | index | Index to VLAN member configuration (0~15).
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_CVIDX | Invalid VLAN member configuration index (0~15).
@rvalue ERRNO_PORT_NUM | Invalid port number.
@comm
	In port based VLAN, untagged packets recieved by port N are forwarded to a VLAN according to the setting VID of port N. Usage of VLAN 4k table is enabled
	and there are only VID and 802.1q priority retrieved from 16 member configurations . Member set, untag set and FID of port based VLAN are be retrieved from 
	4K mapped VLAN entry.
	
*/
int32 rtl8368s_setAsicVlanPortBasedVID(enum PORTID port, uint32 index)
{
	int32 retVal;
	uint32 regAddr;
	uint32 regData;
	uint32 regBits;

	/* bits mapping to port vlan control register of port n */
	const uint16 bits[8]= { 0x000F,0x00F0,0x0F00,0xF000,0x000F,0x00F0,0x0F00,0xF000 };
	/* bits offset to port vlan control register of port n */
	const uint16 bitOff[8] = { 0,4,8,12,0,4,8,12 };
	/* address offset to port vlan control register of port n */
	const uint16 addrOff[8]= { 0,0,0,0,1,1,1,1 };

	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;


	if(index > RTL8368S_VLANMCIDXMAX)
		return ERRNO_CVIDX;

	regAddr = RTL8368S_PORT_VLAN_CTRL_BASE + addrOff[port];

	regBits = bits[port];

	regData =  (index << bitOff[port]) & regBits;

	retVal = rtl8368s_setAsicRegBits(regAddr,regBits,regData);		
	
	return retVal;
}

/*
@func int32 | rtl8368s_getAsicVlanPortBasedVID | Get port based VID which is indexed to 16 VLAN member configurations.
@parm enum PORTID | port | Physical port number (0~5).
@parm uint32* | index | Index to VLAN member configuration (0~15).
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@comm
	The API can access port based VLAN index indirectly retrieving VID and priority from 16 member configuration for a specific port.
	
*/
int32 rtl8368s_getAsicVlanPortBasedVID(enum PORTID port, uint32* index)
{
	int32 retVal;
	uint32 regAddr;
	uint32 regData;

	/* bits mapping to port vlan control register of port n */
	const uint16 bits[8]= { 0x000F,0x00F0,0x0F00,0xF000,0x000F,0x00F0,0x0F00,0xF000 };
	/* bits offset to port vlan control register of port n */
	const uint16 bitOff[8] = { 0,4,8,12,0,4,8,12 };
	/* address offset to port vlan control register of port n */
	const uint16 addrOff[8]= { 0,0,0,0,1,1,1,1 };


	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;


	regAddr = RTL8368S_PORT_VLAN_CTRL_BASE + addrOff[port];

	retVal = rtl8368s_getAsicReg(regAddr,&regData);
	if(retVal != SUCCESS)
		return retVal;

	*index =  (regData & bits[port]) >> bitOff[port];
	return retVal;
}


/*
@func int32 | rtl8368s_setAsicVlanProtocolBasedGroupData | Set protocol and port based group database.
@parm uint32 | index | Index of protocol and port based database index (0~3).
@parm rtl8368s_protocolgdatacfg | pbcfg | Protocol and port based group database entry.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_VLAN_INVALIDPPBIDX | Invalid protocol base group database index (0-3).
@comm
	System supported only 4 entries and 3 types of frame format. Supported frame types are defined as Ethernet (frame type = 0b00, Ether type > 0x05FF),
	RFC 1042 (frame type = 0b01,6 bytes after Type/Length = AA-AA-03-00-00-00) and LLC other(frame type = 0b10). ASIC has available setting of each 
	frame type per port and available system setting each defined frame type. If per system frame type is set to invalid, then per port frame setting is take 
	no effect. There is contained valid bit setting in each group database.	
*/
int32 rtl8368s_setAsicVlanProtocolBasedGroupData(uint32 index, rtl8368s_protocolgdatacfg pbcfg)
{
	int32 retVal;
	uint32 regData;
	uint32 regAddr;

	if(index > RTL8368S_PPBIDXMAX)
		return ERRNO_PPBVIDX;

	regAddr = RTL8368S_PROTOCOL_GDATA_BASE + (index<<1);

	regData = (*(uint32*)&pbcfg) & 0x0000FFFF;

	retVal = rtl8368s_setAsicReg(regAddr,regData);

	if(retVal != SUCCESS)
		return retVal;

	regAddr = RTL8368S_PROTOCOL_GDATA_BASE + (index<<1) + 1;

	regData = *(uint32*)&pbcfg;

	regData = regData >> 16;

	/*regData = pbcfg.value;*/
	retVal = rtl8368s_setAsicReg(regAddr,regData);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicVlanProtocolBasedGroupData |  Get protocol and port based group database.
@parm uint32 | index | Index of protocol and port based database index (0~3).
@parm rtl8368s_protocolgdatacfg* | pbcfg | Protocol and port based group database entry.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_VLAN_INVALIDPPBIDX | Invalid protocol and port based group database index.
@comm
	The API can retrieved protocol and port based group database.	 Frame type 0b11 of database is invalid and database entry with this frame type is same as 
	setting valid bit to 0.
	
*/
int32 rtl8368s_getAsicVlanProtocolBasedGroupData(uint32 index, rtl8368s_protocolgdatacfg* pbcfg)
{
	int32 retVal;
	uint32 regData;
	uint32 regAddr;
	uint32 tmp;

	if(index > RTL8368S_PPBIDXMAX)
		return ERRNO_PPBVIDX;
	regAddr = RTL8368S_PROTOCOL_GDATA_BASE + (index<<1);

	retVal = rtl8368s_getAsicReg(regAddr,&regData);
	if(retVal !=  SUCCESS)
		return retVal;
	
	tmp = regData & 0x0000FFFF;

	regAddr = RTL8368S_PROTOCOL_GDATA_BASE + (index<<1) +1;

	retVal = rtl8368s_getAsicReg(regAddr,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	tmp = ((regData<<16)&0xFFFF0000) | tmp;

	*pbcfg = *(rtl8368s_protocolgdatacfg*)&tmp;

	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicVlanProtocolAndPortBasedCfg | Set protocol and port based VLAN configuration. 
@parm enum PORTID | port | Physical port number (0~5).
@parm uint32 | index | Index of protocol and port based database index (0~3).
@parm rtl8368s_protocolvlancfg | ppbcfg | Protocol  and port based VLAN configuration.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@rvalue ERRNO_PPBVIDX | Invalid protocol and port based group database index.
@comm
	Each port has four VLAN configurations for each protocol and port based group database. Protocol and port based VLAN configuration contained 1 valid 
	bit setting for each group database entry. There is 802.1q priority field setting for each group database entry. Different with port based VLAN information
	retrieving, ASIC decided 802.1q priority of reveiving frame from dedicated port based VLAN configuration and didn't decide from priority field of system VLAN
	16 member configurations.
	
*/
int32 rtl8368s_setAsicVlanProtocolAndPortBasedCfg(enum PORTID port, uint32 index, rtl8368s_protocolvlancfg ppbcfg)
{
	int32 retVal;
	uint32 regData;
	uint32 regAddr;
	uint32 tmp;


	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	if(index > RTL8368S_PPBIDXMAX)
		return ERRNO_PPBVIDX;
	
	regAddr = RTL8368S_PROTOCOL_VLAN_CTRL_BASE + (port<<1) + (index >>1);
	
	retVal = rtl8368s_getAsicReg(regAddr,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	if(index&0x01)
	{
		tmp =  (*(uint16*)&ppbcfg);
		tmp = tmp << 8;
		regData = (regData & 0x00FF) | (tmp & 0xFF00);
		retVal = rtl8368s_setAsicReg(regAddr,regData);
		if(retVal !=  SUCCESS)
			return retVal;
	}
	else
	{
		tmp =  (*(uint16*)&ppbcfg);
		regData = (regData & 0xFF00) | (tmp & 0x00FF);
		retVal = rtl8368s_setAsicReg(regAddr,regData);
		if(retVal !=  SUCCESS)
			return retVal;
	}
	return SUCCESS;
}

/*
@func int32 | rtl8368s_getAsicVlanProtocolAndPortBasedCfg | Get protocol and port based VLAN configuration. 
@parm enum PORTID | port | Physical port number (0~5).
@parm uint32 | index | Index of protocol and port based database index (0~3).
@parm rtl8368s_protocolvlancfg* | ppbcfg |  Protocol  and port based VLAN configuration.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@rvalue ERRNO_PPBVIDX | Invalid protocol and port based group database index.
@comm
	The API can get protocol and port based VLAN configuration. Each port supports only 4 VLAN settings mapping to each protocol and port based group database.
	
*/
int32 rtl8368s_getAsicVlanProtocolAndPortBasedCfg(enum PORTID port, uint32 index, rtl8368s_protocolvlancfg* ppbcfg)
{
	int32 retVal;
	uint32 regData;
	uint32 regAddr;

	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	if(index > RTL8368S_PPBIDXMAX)
		return ERRNO_PPBVIDX;

	regAddr = RTL8368S_PROTOCOL_VLAN_CTRL_BASE + (port<<1) + (index >>1);
	
	retVal = rtl8368s_getAsicReg(regAddr,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	if(index&0x01)
	{
		regData = (regData >> 8) & 0x00FF;
	
		*ppbcfg = *(rtl8368s_protocolvlancfg*)&regData; 
	}
	else
	{
		regData = regData & 0x00FF;
		*ppbcfg = *(rtl8368s_protocolvlancfg*)&regData; 
	}

	return SUCCESS;
}
/*
@func int32 | rtl8368s_setAsicVlanKeepCtagFormat | Set ASIC keep receiving tag/untag format in ingress port.. 
@parm enum PORTID | ingressport | Ingress port number (0~7).
@uint32 | portmask | Egress port mask (0x00-0xFF).
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@rvalue ERRNO_INPUT | Invalid port number.
@comm
	Output frame from ASIC will not be tagged C-tag or untagged C-tag if ingress port was set enable keep-Ctag-format function. 
    Receiving frame with untag format will be output with untag format, priority-tag frame will be output as priority-tag 
    frame and c-tagged frame will be output as original c-tagged frame. But if 802.1q remarking function was enabled in the 
    egress port, then the original priroity of frame will be impacted.	
*/
int32 rtl8368s_setAsicVlanKeepCtagFormat(enum PORTID ingressport, uint32 portmask)
{
	int32 retVal;
	uint32 regData; 

	if(ingressport >=PORT_MAX)
		return ERRNO_INPUT;
	if (portmask>RTL8368S_PORTMASK)
		return ERRNO_PORT_MSK;

 	retVal = rtl8368s_getAsicReg(RTL8368S_KEEP_FORMAT_BASE+(ingressport>>1), &regData);
	regData =  (regData&(RTL8368S_KEEP_FORMAT_MSK << (RTL8368S_KEEP_PORT_OFF*((ingressport+1)%2)))) | 
			((portmask&RTL8368S_KEEP_FORMAT_MSK) << (RTL8368S_KEEP_PORT_OFF*(ingressport%2))) ;
	retVal = rtl8368s_setAsicReg(RTL8368S_KEEP_FORMAT_BASE+(ingressport>>1), regData);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicVlanKeepCtagFormat | Set ASIC keep receiving tag/untag format in ingress port.. 
@parm enum PORTID | ingressport | Ingress port number (0~7).
@parm uint32* | portmask | Egress port mask with original tag/untag format 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@rvalue ERRNO_VLAN_INVALIDPORT | Invalid port number.
@comm
	The API can be used to get keep format function of receiving port.		
*/
int32 rtl8368s_getAsicVlanKeepCtagFormat(enum PORTID ingressport, uint32* portmask)
{
	int32 retVal;
	uint32 regData; 
	
	if(ingressport >=PORT_MAX)
		return ERRNO_INPUT;

	retVal = rtl8368s_getAsicReg(RTL8368S_KEEP_FORMAT_BASE+(ingressport>>1),&regData);
	if(retVal !=  SUCCESS)
		return retVal;
	
	if (ingressport%2==1)
		*portmask = (regData & 0xFF00)>>8;
	else
		*portmask = (regData & 0x00FF);

	return SUCCESS;
}
/*
@func int32 | rtl8368s_setAsicSpanningTreeStatus | Configure spanning tree state per each port.
@parm enum PORTID | port | Physical port number (0~5).
@parm uint32 | fid | FID of 8 SVL/IVL in port (0~7).
@parm enum SPTSTATE | state | Spanning tree state for FID of 8 SVL/IVL.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@rvalue ERRNO_FID | Invalid FID.
@rvalue ERRNO_STP_STATE | Invalid spanning tree state
@common
	System supports 8 SVL/IVL configuration and each port has dedicated spanning tree state setting for each FID. There are four states supported by ASIC.

	Disable state 		ASIC did not receive and transmit packets at port with disable state.
	Blocking state		ASIC will receive BPDUs without L2 auto learning and does not transmit packet out of port in blocking state.
	Learning state		ASIC will receive packets with L2 auto learning and transmit out BPDUs only.
	Forwarding state	The port will receive and transmit packets normally.
	
*/
int32 rtl8368s_setAsicSpanningTreeStatus(enum PORTID port, enum FIDVALUE fid, enum SPTSTATE state)
{
	uint32 regAddr;
	uint32 regData;
	uint32 regBits;
	int32 retVal;

	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	if(fid > RTL8368S_FIDMAX)
		return ERRNO_FID;

	if(state > FORWARDING)
		return ERRNO_STP_STATE;

	regAddr = RTL8368S_SPT_STATE_BASE + fid;
	regBits = RTL8368S_SPT_STATE_MSK << (port*RTL8368S_SPT_STATE_BITS);
	regData = (state << (port*RTL8368S_SPT_STATE_BITS)) & regBits;

	
	retVal = rtl8368s_setAsicRegBits(regAddr,regBits,regData);		

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicSpanningTreeStatus |  Get spanning tree state per each port.
@parm enum PORTID | port | Physical port number (0~5).
@parm uint32 | fid | FID of 8 SVL/IVL in port (0~7).
@parm enum SPTSTATE* | state | Spanning tree state for FID of 8 SVL/IVL.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@rvalue ERRNO_FID | Invalid FID
@common
	The API can get spanning tree state of each different FID in port.	
*/
int32 rtl8368s_getAsicSpanningTreeStatus(enum PORTID port, enum FIDVALUE fid, enum SPTSTATE* state)
{
	uint32 regAddr;
	uint32 regData;
	/*uint32 regBits;*/
	int32 retVal;

	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	if(fid > RTL8368S_FIDMAX)
		return ERRNO_FID;
	
	regAddr = RTL8368S_SPT_STATE_BASE + fid;

	retVal = rtl8368s_getAsicReg(regAddr,&regData);		
	if(retVal != SUCCESS)
		return retVal;

	*state = (regData >> (port*RTL8368S_SPT_STATE_BITS)) & RTL8368S_SPT_STATE_MSK;

	return SUCCESS;
}


/*=======================================================================
 *  L2 LUT & CAM APIs
 *========================================================================*/

/*
@func int32 | rtl8368s_setAsicL2IpMulticastLookup | Set L2 IP multicast lookup function.
@parm uint32 | enabled | L2 IP multicast checking function 1: enabled, 0: disabled. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@common
	ASIC will auto learn and write L2 look up entry. Auto learning L2 look up  table contained DMAC and source port information only. System supports L2 entry
	with IP multicast DIP/SIP to forward IP multicasting frame as user desired. If this function is enabled, then system will be looked up L2 IP multicast entry to 
	forward IP multicast frame directly without flooding. The L2 IP multicast forwarding path can be as port mask and not as same as auto learn L2 enrty with source 
	port 	information only. Both IP_MULT and Static fields of LUT must be wrote by software and these fields of auto learn entries will be 0 by ASIC.	
	
*/
int32 rtl8368s_setAsicL2IpMulticastLookup(uint32 enabled)
{
	uint32 retVal;

	retVal = rtl8368s_setAsicRegBit(RTL8368S_LUT_CONTROL_REG,RTL8368S_RMA_IPMULTICAST_LUT_OFF,enabled);
	
	return retVal;
}

/*
@func int32 | rtl8368s_getAsicL2IpMulticastLookup | Get L2 IP multicast lookup function setting.
@parm uint32* | enabled | L2 IP multicast checking function 1: enabled, 0: disabled. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@comm
	The API can get L2 table IP multicast lookup usage.
	
*/
int32 rtl8368s_getAsicL2IpMulticastLookup(uint32* enabled)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_LUT_CONTROL_REG,&regData);
	
	if(retVal !=  SUCCESS)
		return retVal;

	regData = regData & RTL8368S_RMA_IPMULTICAST_LUT_MSK;

	if(regData)
		*enabled = 1;
	else
		*enabled = 0;

	return SUCCESS;	
}

/*
@func int32 | rtl8368s_setAsicL2CamTbUsage | Configure L2 CAM table usage.
@parm uint32 | disabled | L2 CAM table usage 0: enabled, 1: disabled. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@common
	System support 8 CAM entries only. Fields of CAM entry are as same as L2 LUT except without IP_MULT field. It means that ASIC will not checking IP multicast
	frame by CAM lookup. ASIC will lookup CAM by entry location sequence (0>1>...>6>7). ASIC looks up L2 LUT and get a hit while receiving frame, then it will 
	abandon look up result from CAM. As same as L2 LUT writing rule by ASIC auto learning, ASIC will not over write CAM entry contained Auth or Static field is not 0.
	Only while 4 entries (4 way hash) in L2 LUT are all not free (Auth, Static and IP_MULTI are not all 0), then ASIC will write auto learn result to CAM.  
*/
int32 rtl8368s_setAsicL2CamTbUsage(uint32 disabled)
{
	uint32 retVal;

	retVal = rtl8368s_setAsicRegBit(RTL8368S_SWITCH_GLOBAL_CTRL_REG,RTL8368S_CAM_TBL_OFF,disabled);
	
	return retVal;
}

/*
@func int32 | rtl8368s_getAsicL2CamTbUsage | Get L2 CAM table usage status.
@parm uint32* | disabled | L2 CAM table usage 0: enabled, 1: disabled. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@comm
	The API can get L2 CAM table usage status.
	
*/
int32 rtl8368s_getAsicL2CamTbUsage(uint32* disabled)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_SWITCH_GLOBAL_CTRL_REG,&regData);
	
	if(retVal !=  SUCCESS)
		return retVal;

	regData = regData & (1<<RTL8368S_CAM_TBL_OFF);

	if(regData)
		*disabled = 1;
	else
		*disabled = 0;

	return SUCCESS;	
}

static void _rtl8368s_camStUser2Smi( rtl8368s_camtable *camUser, rtl8368s_camsmitable *camSmi)
{

	camSmi->mac0 = camUser->mac.octet[0];
	camSmi->mac1 = camUser->mac.octet[1];
	camSmi->mac2 = camUser->mac.octet[2];
	camSmi->mac3 = camUser->mac.octet[3];
	camSmi->mac4 = camUser->mac.octet[4];
	camSmi->mac5 = camUser->mac.octet[5];

	camSmi->fid = camUser->fid;
	camSmi->mbr = camUser->mbr;
	camSmi->age = camUser->age;
	camSmi->block = camUser->block;
	camSmi->auth = camUser->auth;
	camSmi->swst = camUser->swst;

	
}

static void _rtl8368s_camStSmi2User( rtl8368s_camtable *camUser, rtl8368s_camsmitable *camSmi)
{
 	camUser->mac.octet[0] = camSmi->mac0;
 	camUser->mac.octet[1] = camSmi->mac1;
 	camUser->mac.octet[2] = camSmi->mac2;
 	camUser->mac.octet[3] = camSmi->mac3;
 	camUser->mac.octet[4] = camSmi->mac4;
 	camUser->mac.octet[5] = camSmi->mac5;

	camUser->fid = camSmi->fid;
	camUser->mbr = camSmi->mbr;
	camUser->age = camSmi->age;
	camUser->block = camSmi->block;
	camUser->auth = camSmi->auth;
	camUser->swst = camSmi->swst;

}

/*
@func int32 | rtl8368s_setAsicL2CamTb | Configuration L2 CAM look up entry.
@parm uint32 | entry | Entry index of L2 CAM table.
@parm rtl8368s_camtable * | camTable | L2 CAM entry writting to table.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_CAM_INVALIDIDX | Invalid CAM index (0~7).
@common
	System support 8 CAM entries only. It supports MAC/FID format (ASIC did not parse SIP/DIP format to forward IP multicast frame) in CAM entry and there
	is no IP_MULT field as L2 LUT. 
	
*/
int32 rtl8368s_setAsicL2CamTb(uint32 entry, rtl8368s_camtable *camTable)
{
	uint32 retVal;
	uint32 regData;
	uint16* accessPtr;
	uint32 i;
	rtl8368s_camsmitable smicamTable;

	if(entry > RTL8368S_CAMENTRYMAX)
		return ERRNO_CAMIDX;

	memset(&smicamTable,0x00,sizeof(rtl8368s_camsmitable));
	
	 _rtl8368s_camStUser2Smi(camTable, &smicamTable);

	/*prepare CAM entry content first*/
	accessPtr = (uint16*)&smicamTable;

	regData = *accessPtr;

	for(i=0;i<5;i++)
	{
		retVal = rtl8368s_setAsicReg(RTL8368S_TABLE_WRITE_BASE+i,regData);
		if(retVal !=  SUCCESS)
			return retVal;

		accessPtr ++;
		regData = *accessPtr;
	}

	/*write control word for writing CAM entry*/
	regData = RTL8368S_TABLE_CAMTB_WRITE_CTRL | (entry<<3);
	retVal = rtl8368s_setAsicReg(RTL8368S_TABLE_ACCESS_CTRL_REG,regData);
	if(retVal !=  SUCCESS)
		return retVal;
	
#ifdef CONFIG_RTL8368S_ASICDRV_TEST
	memcpy((uint16*)&Rtl8368sVirtualCAMTable[entry],(uint16*)&Rtl8368sVirtualReg[RTL8368S_TABLE_WRITE_BASE],10);
#endif
	

	return SUCCESS;
}

/*
@func int32 | rtl8368s_getAsicL2CamTb | Get L2 CAM look up entry.
@parm uint32 | entry | Entry index of L2 CAM table.
@parm rtl8368s_camtable * | camTable | L2 CAM entry reading from table.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@common
	The API can read out CAM entry. 
	
*/
int32 rtl8368s_getAsicL2CamTb(uint32 entry, rtl8368s_camtable *camTable)
{
	uint32 retVal;
	uint32 regData;
	uint16 *accessPtr;
	uint32 i;
	rtl8368s_camsmitable smicamTable;

	if(entry > RTL8368S_CAMENTRYMAX)
		return ERRNO_CAMIDX;

	/*write control word to read out CAM entry*/
	regData = RTL8368S_TABLE_CAMTB_READ_CTRL | (entry<<3);
	retVal = rtl8368s_setAsicReg(RTL8368S_TABLE_ACCESS_CTRL_REG,regData);
	if(retVal !=  SUCCESS)
		return retVal;

#ifdef CONFIG_RTL8368S_ASICDRV_TEST
	memcpy((uint16*)&Rtl8368sVirtualReg[RTL8368S_TABLE_READ_BASE],(uint16*)&Rtl8368sVirtualCAMTable[entry],10);
#endif


	/*read CAM entry */
	memset(&smicamTable,0x00,sizeof(rtl8368s_camsmitable));
	accessPtr = (uint16*)&smicamTable;

	for(i=0;i<6;i++)
	{
		retVal = rtl8368s_getAsicReg(RTL8368S_TABLE_READ_BASE+i,&regData);
		if(retVal !=  SUCCESS)
			return retVal;
		*accessPtr = regData;

		accessPtr ++;
	}

	_rtl8368s_camStSmi2User(camTable, &smicamTable);

	return SUCCESS;
}

/*
	Exchange structure type define with MMI and SMI
*/
static void _rtl8368s_l2StUser2Smi( rtl8368s_l2table *l2User, rtl8368s_l2smitable *l2Smi)
{
	/*SIP/DIP hashing source*/
	if(l2User->ipmulti == 1)
	{
		l2Smi->smi_ipmul.sip0  = (l2User->sip & 0xFF000000) >> 24;
		l2Smi->smi_ipmul.sip1  = (l2User->sip & 0x00FF0000) >> 16;
		l2Smi->smi_ipmul.sip2  = (l2User->sip & 0x0000FF00) >> 8;
		l2Smi->smi_ipmul.sip3  = l2User->sip & 0x000000FF;
	
		l2Smi->smi_ipmul.dip0  = (l2User->dip & 0xFF000000) >> 24;
		l2Smi->smi_ipmul.dip1  = (l2User->dip & 0x00FF0000) >> 16;
		l2Smi->smi_ipmul.dip2  = (l2User->dip & 0x0000FF00) >> 8;
		l2Smi->smi_ipmul.dip3  = l2User->dip & 0x000000FF;

		if(TRUE !=l2User->ifdel)
		{
			l2Smi->smi_ipmul.mbr = l2User->mbr;
			l2Smi->smi_ipmul.ipmulti = l2User->ipmulti;
		}

	}	
	/*Static MAC*/
	else if (l2User->swst == 1)
	{
	 	l2Smi->smi_swstatic.mac0 = l2User->mac.octet[5];
	 	l2Smi->smi_swstatic.mac1 = l2User->mac.octet[4];
	 	l2Smi->smi_swstatic.mac2 = l2User->mac.octet[3];
	 	l2Smi->smi_swstatic.mac3 = l2User->mac.octet[2];
	 	l2Smi->smi_swstatic.mac4 = l2User->mac.octet[1];
	 	l2Smi->smi_swstatic.mac5 = l2User->mac.octet[0];

		l2Smi->smi_swstatic.fid = l2User->fid;

		if(TRUE !=l2User->ifdel)
		{
			l2Smi->smi_swstatic.mbr = l2User->mbr;
			l2Smi->smi_swstatic.block = l2User->block;
			l2Smi->smi_swstatic.auth = l2User->auth;
			l2Smi->smi_swstatic.swst = l2User->swst;
		}

	}
	/*Auto learn MAC*/
	else
	{
	 	l2Smi->smi_autolearn.mac0 = l2User->mac.octet[5];
	 	l2Smi->smi_autolearn.mac1 = l2User->mac.octet[4];
	 	l2Smi->smi_autolearn.mac2 = l2User->mac.octet[3];
	 	l2Smi->smi_autolearn.mac3 = l2User->mac.octet[2];
	 	l2Smi->smi_autolearn.mac4 = l2User->mac.octet[1];
	 	l2Smi->smi_autolearn.mac5 = l2User->mac.octet[0];

		l2Smi->smi_autolearn.fid = l2User->fid;

		if(TRUE !=l2User->ifdel)
		{
			l2Smi->smi_autolearn.spa = l2User->spa;
			l2Smi->smi_autolearn.age = l2User->age;
			l2Smi->smi_autolearn.auth = l2User->auth;
			l2Smi->smi_autolearn.swst = l2User->swst;
		}	
	}
}

/*
	Exchange structure type define with MMI and SMI
*/
static void _rtl8368s_l2StSmi2User( rtl8368s_l2table *l2User, rtl8368s_l2smitable *l2Smi)
{
	/*SIP/DIP hashing source*/
	if(l2Smi->smi_ipmul.ipmulti)
	{
		l2User->sip = l2Smi->smi_ipmul.sip0;
		l2User->sip = (l2User->sip << 8) | l2Smi->smi_ipmul.sip1;
		l2User->sip = (l2User->sip << 8) | l2Smi->smi_ipmul.sip2;
		l2User->sip = (l2User->sip << 8) | l2Smi->smi_ipmul.sip3;
		
#ifdef _LITTLE_ENDIAN
		l2User->dip = l2Smi->smi_ipmul.dip0;
#else
		l2User->dip =( l2Smi->smi_ipmul.dip0&0xF) + 0xE0;
#endif

		l2User->dip = (l2User->dip << 8) | l2Smi->smi_ipmul.dip1;
		l2User->dip = (l2User->dip << 8) | l2Smi->smi_ipmul.dip2;
#ifdef _LITTLE_ENDIAN
		l2User->dip = (l2User->dip << 8) | ((l2Smi->smi_ipmul.dip3&0xF) + 0xE0);
#else
		l2User->dip = (l2User->dip << 8) | l2Smi->smi_ipmul.dip3;
#endif
		l2User->mbr = l2Smi->smi_ipmul.mbr;
		l2User->ipmulti = l2Smi->smi_ipmul.ipmulti;

	}
	else if (l2Smi->smi_swstatic.swst)
	{
	 	l2User->mac.octet[5] = l2Smi->smi_swstatic.mac0;
	 	l2User->mac.octet[4] = l2Smi->smi_swstatic.mac1;
	 	l2User->mac.octet[3] = l2Smi->smi_swstatic.mac2;
	 	l2User->mac.octet[2] = l2Smi->smi_swstatic.mac3;
	 	l2User->mac.octet[1] = l2Smi->smi_swstatic.mac4;
	 	l2User->mac.octet[0] = l2Smi->smi_swstatic.mac5;

		l2User->fid = l2Smi->smi_swstatic.fid;
		l2User->mbr = l2Smi->smi_swstatic.mbr;
		l2User->block = l2Smi->smi_swstatic.block;
		l2User->auth = l2Smi->smi_swstatic.auth;
		l2User->swst = l2Smi->smi_swstatic.swst;
		l2User->ipmulti = l2Smi->smi_swstatic.ipmulti;
	}
	else
	{
	 	l2User->mac.octet[5] = l2Smi->smi_autolearn.mac0;
	 	l2User->mac.octet[4] = l2Smi->smi_autolearn.mac1;
	 	l2User->mac.octet[3] = l2Smi->smi_autolearn.mac2;
	 	l2User->mac.octet[2] = l2Smi->smi_autolearn.mac3;
	 	l2User->mac.octet[1] = l2Smi->smi_autolearn.mac4;
	 	l2User->mac.octet[0] = l2Smi->smi_autolearn.mac5;

		l2User->fid = l2Smi->smi_autolearn.fid;
		l2User->spa = l2Smi->smi_autolearn.spa;
		l2User->age = l2Smi->smi_autolearn.age;
		l2User->auth = l2Smi->smi_autolearn.auth;
		l2User->swst = l2Smi->smi_autolearn.swst;
		l2User->ipmulti = l2Smi->smi_autolearn.ipmulti;	
	}
}

/*
@func int32 | rtl8368s_setAsicL2LookupTb | Configure L2 LUT entry to 4-way hashing filtering database.
@parm uint32 | entry | Entry index of 4 way hashing database.
@parm rtl8368s_l2table * | l2Table | L2 table entry writing to 1K filtering database
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_LUT_INVALIDIDX | Invalid hashing entry index (0~3).
@common
	Refore writing L2 entry to 4 wa hashing database, software should read out 4 entries in hashing address of filtering database. Atfer comparing 4 entries, software can
	find out if current writting entry is existed or need to create new entry for writting. Each MAC/FID or SIP/DIP will be hashed to certainly located address and they 
	maybe be hashed to the same hashing address. If 4 entries of the same hashing address are have been written, then software must decide replace which entry to
	new LUT or write new LUT to CAM. 
	
*/
int32 rtl8368s_setAsicL2LookupTb(uint32 entry, rtl8368s_l2table *l2Table)
{
	uint32 retVal;
	uint32 regData;
	uint16 *accessPtr;
	uint32 i;
	rtl8368s_l2smitable smil2Table;

	if(entry > RTL8368S_L2ENTRYMAX)
		return ERRNO_LUTIDX;

	memset(&smil2Table,0x00,sizeof(rtl8368s_l2smitable));
	 _rtl8368s_l2StUser2Smi(l2Table, &smil2Table);

	/*
	* MAC/IP order in ACL and LUT is totally reverse 
	* user input: 11:22:33:44:55:66 LUT order: 66:55:44:33:22:11
	*	184_H | 11 |
	* 	184_L | 22 |
	*	183_H | 33 |
	* 	183_L | 44 |
	*	182_H | 55 |
	* 	182_L | 66 |
	*/
	accessPtr =  (uint16*)&smil2Table;

	regData = *accessPtr;
	for(i=0;i<5;i++)
	{
		retVal = rtl8368s_setAsicReg(RTL8368S_TABLE_WRITE_BASE+i,regData);
		if(retVal !=  SUCCESS)
			return retVal;

		accessPtr ++;
		regData = *accessPtr;
	}

	/*write control word for writing L2 entry*/
	regData = RTL8368S_TABLE_L2TB_WRITE_CTRL | (entry<<3);
	
	retVal = rtl8368s_setAsicReg(RTL8368S_TABLE_ACCESS_CTRL_REG,regData);
	if(retVal !=  SUCCESS)
		return retVal;



#ifdef CONFIG_RTL8368S_ASICDRV_TEST
	memcpy((uint16*)&Rtl8368sVirtualL2Table[entry],(uint16*)&Rtl8368sVirtualReg[RTL8368S_TABLE_WRITE_BASE],10);
#endif

	return SUCCESS;
}

/*
@func int32 | rtl8368s_getAsicL2LookupTb | Get L2 LUT entry from 4-way hashing filtering database.
@parm uint32 | entry | Entry index of 4 way hashing database.
@parm rtl8368s_l2table * | l2Table | L2 table entry need to be read from filtering database
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_LUT_INVALIDIDX | Invalid hashing entry index (0~3).
@common
	The API can read out L2 lookup entry. 
	
*/
int32 rtl8368s_getAsicL2LookupTb(uint32 entry,  rtl8368s_l2table *l2Table)
{
	uint32 retVal;
	uint32 regData;
	uint16* accessPtr;
	uint32 i;
	rtl8368s_l2smitable smil2TableW;
	rtl8368s_l2smitable smil2TableR;

	if(entry > RTL8368S_L2ENTRYMAX)
		return ERRNO_LUTIDX;

	memset(&smil2TableW,0x00,sizeof(rtl8368s_l2smitable));
	 _rtl8368s_l2StUser2Smi(l2Table, &smil2TableW);

	accessPtr =  (uint16*)&smil2TableW;
	regData = *accessPtr;

	for(i=0;i<4;i++)
	{
		retVal = rtl8368s_setAsicReg(RTL8368S_TABLE_WRITE_BASE+i,regData);
		if(retVal !=  SUCCESS)
			return retVal;

		accessPtr ++;
		regData = *accessPtr;
	}

	/*write control word to read out L2 entry*/
	regData = RTL8368S_TABLE_L2TB_READ_CTRL | (entry<<3);
	
	retVal = rtl8368s_setAsicReg(RTL8368S_TABLE_ACCESS_CTRL_REG,regData);
	if(retVal !=  SUCCESS)
		return retVal;




#ifdef CONFIG_RTL8368S_ASICDRV_TEST
	memcpy((uint16*)&Rtl8368sVirtualReg[RTL8368S_TABLE_READ_BASE],(uint16*)&Rtl8368sVirtualL2Table[entry],10);
#endif
	/*read L2 entry */


	memset(&smil2TableR,0x00,sizeof(rtl8368s_l2smitable));

	accessPtr = (uint16*)&smil2TableR;

	for(i=0;i<6;i++)
	{
		retVal = rtl8368s_getAsicReg(RTL8368S_TABLE_READ_BASE+i,&regData);
		if(retVal !=  SUCCESS)
			return retVal;

		*accessPtr = regData;

		accessPtr ++;
	}


	_rtl8368s_l2StSmi2User(l2Table, &smil2TableR);

	return SUCCESS;
}

/*=======================================================================
 *  VLAN stacking APIs
 *========================================================================*/
/*
@func int32 | rtl8368s_setAsicSvlan | Configure enable S-VLAN  function.
@parm uint32 | enabled | S-VLAN function 1: enabled, 0: disabled. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	 There are service provider VLAN functions defined in IEEE 802.1ad standard. System supports user defined type/length for parsing S-tag frame at uplink 
	 ports. Without S-VLAN function enable setting, S-tag frame from uplink ports will be treated as normal frame payload to forward. Only S-tag frame contained
	 supported S-VID will be accepted from uplink ports.
*/
int32 rtl8368s_setAsicSvlan(uint32 enabled)
{
	uint32 retVal;

	retVal = rtl8368s_setAsicRegBit(RTL8368S_SVLAN_CTRL_REG,RTL8368S_SVLAN_EN_OFF,enabled);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicSvlan | Get S-VLAN  function configuration.
@parm uint32* | enabled | S-VLAN function 1: enabled, 0: disabled. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can get configuration of SVLAN function enable setting.
	
*/
int32 rtl8368s_getAsicSvlan(uint32* enabled)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_SVLAN_CTRL_REG,&regData);

	if(retVal !=  SUCCESS)
		return retVal;

	regData = regData & RTL8368S_SVLAN_EN_MSK;

	if(regData)
		*enabled = 1;
	else
		*enabled = 0;

	return SUCCESS;
	
}

/*
@func int32 | rtl8368s_setAsicSvlanS2Cmbrsel | Configure enable S-VLAN  function.
@parm uint32 | enabled | S-VLAN S2C member selection function 1: enabled, 0: disabled. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	 There are service provider VLAN functions defined in IEEE 802.1ad standard. System supports user defined type/length for parsing S-tag frame at uplink 
	 ports. Without S-VLAN function enable setting, S-tag frame from uplink ports will be treated as normal frame payload to forward. Only S-tag frame contained
	 supported S-VID will be accepted from uplink ports.
*/
int32 rtl8368s_setAsicSvlanS2Cmbrsel(uint32 enabled)
{
	uint32 retVal;

	retVal = rtl8368s_setAsicRegBit(RTL8368S_SVLAN_S2CSEL_REG,RTL8368S_SVLAN_S2CSEL_OFF,enabled);

	return retVal;
}


/*
@func int32 | rtl8368s_getAsicSvlanS2Cmbrsel | Configure enable S-VLAN  function.
@parm uint32 | enabled | S-VLAN S2C member selection function 1: enabled, 0: disabled. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	 There are service provider VLAN functions defined in IEEE 802.1ad standard. System supports user defined type/length for parsing S-tag frame at uplink 
	 ports. Without S-VLAN function enable setting, S-tag frame from uplink ports will be treated as normal frame payload to forward. Only S-tag frame contained
	 supported S-VID will be accepted from uplink ports.
*/
int32 rtl8368s_getAsicSvlanS2Cmbrsel(uint32* enabled)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_SVLAN_S2CSEL_REG,&regData);

	if(retVal !=  SUCCESS)
		return retVal;

	regData = regData & RTL8368S_SVLAN_S2CSEL_MSK;

	if(regData)
		*enabled = 1;
	else
		*enabled = 0;

	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicSvlanPrioDecision | Configure priority decision of S-tag frame output to uplink ports.
@parm uint32 | vsPrio | Priority of S-tag decision, 0: usge C-Tag's priority as S-priority 1: use mapped priority indexed from Pn_VSIDX.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	If setting is 0 by using C-tag's priority, then two situations need to be take care. Original receiving frame is C-tag frame and S-priority will be set as
	same as C-priority in C-tag. Otherwise, S-priority of out frame to uplink ports should be user defined S-priority indirectly mapping at Pn_VSIDX.
	
*/
int32 rtl8368s_setAsicSvlanPrioDecision(uint32 vsPrio)
{
	uint32 retVal;

	retVal = rtl8368s_setAsicRegBit(RTL8368S_SVLAN_CTRL_REG,RTL8368S_SVLAN_PRIO_OFF,vsPrio);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicSvlanPrioDecision | Get priority decision of S-tag frame output to uplink ports.
@parm uint32* | vsPrio | Priority of S-tag decision, 0: usge C-Tag's priority as S-priority 1: use mapped priority indexed from Pn_VSIDX.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can get output priority decision method while sending s-tag frame to uplink port
	
*/
int32 rtl8368s_getAsicSvlanPrioDecision(uint32* vsPrio)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_SVLAN_CTRL_REG,&regData);

	if(retVal !=  SUCCESS)
		return retVal;

	regData = regData & RTL8368S_SVLAN_PRIO_MSK;

	if(regData)
		*vsPrio = 1;
	else
		*vsPrio = 0;

	return SUCCESS;
	
}

/*
@func int32 | rtl8368s_setAsicSvlanTag | Configure if output S-tag frame to uplink ports. 
@parm uint32 | enabled | Output frame to uplink ports 0: S-untag 1:S-tag 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	If register VS_STAG is set as 0, frames transmited through uplink port should be S-untagged.Otherwise, it should be S-tagged frames. Both S-VID and
	priority are defined from indirectly indexing at Pn_VSIDX at each uplink port. System supports 4 accepted S-VIDs only. In spec. 802.1ad, output frame 
	should be S-tag frame only (S-tag frames are transferred between customer bridge and priovider bridge).
*/
int32 rtl8368s_setAsicSvlanTag(uint32 enabled)
{
	uint32 retVal;

	retVal = rtl8368s_setAsicRegBit(RTL8368S_SVLAN_CTRL_REG,RTL8368S_SVLAN_STAG_OFF,enabled);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicSvlanTag | Get system output S-tag frame to uplink ports. 
@parm uint32* | enabled | Output frame to uplink ports 0: S-untag 1:S-tag 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can get setting that system outputs S-tag frame to uplink ports or not.
*/
int32 rtl8368s_getAsicSvlanTag(uint32* enabled)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_SVLAN_CTRL_REG,&regData);

	if(retVal !=  SUCCESS)
		return retVal;

	regData = regData & RTL8368S_SVLAN_STAG_MSK;

	if(regData)
		*enabled = 1;
	else
		*enabled = 0;

	return SUCCESS;	
}

/*
@func int32 | rtl8368s_setAsicSvlanUplinkPortMask | Configure uplink ports mask.
@parm uint32 | portMask | Uplink port mask setting.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	Uplink port mask is setting which ports are connected to provider switch. If ports are belong uplink ports and all frames receiving from these port must 
	contain accept SVID in S-tag field.
*/
int32 rtl8368s_setAsicSvlanUplinkPortMask(uint32 portMask)
{
	uint32 retVal;
	uint32 regData;

	if(portMask > RTL8368S_PORTMASK)
		return ERRNO_INPUT;

	regData = portMask << RTL8368S_SVLAN_PORT_OFF;

	retVal = rtl8368s_setAsicRegBits(RTL8368S_SVLAN_CTRL_REG,RTL8368S_SVLAN_PORT_MSK,regData);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicSvlanUplinkPortMask | Get uplink ports mask configuration.
@parm uint32* | portMask | Uplink port mask setting.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can get setting of  uplink port mask belong to service provider VLAN ports.
	
*/
int32 rtl8368s_getAsicSvlanUplinkPortMask(uint32* portMask)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_SVLAN_CTRL_REG,&regData);

	if(retVal !=  SUCCESS)
		return retVal;


	regData = (regData& RTL8368S_SVLAN_PORT_MSK ) >> RTL8368S_SVLAN_PORT_OFF;

	*portMask = regData;

	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicSvlanProtocolType | Configure accepted S-VLAN ether type. The default ether type of S-VLAN is 0x88a8.
@parm uint32 | protocolType | Ether type of S-tag frame parsing in uplink ports.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type 0x9100 and 0x9200 for Q-in-Q SLAN design. User can set mathced ether
	type as service provider supported protocol. 

*/
int32 rtl8368s_setAsicSvlanProtocolType(uint32 protocolType)
{
	uint32 retVal;


	retVal = rtl8368s_setAsicReg(RTL8368S_SVLAN_PROTOCOL_TYPE,protocolType);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicSvlanProtocolType | Get accepted S-VLAN ether type setting.
@parm uint32 | protocolType |  Ether type of S-tag frame parsing in uplink ports.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can get ccepted ether type of service provider VLAN tag. The default ether type of service provider VLAN in 802.1ad is 0x88a8.

*/
int32 rtl8368s_getAsicSvlanProtocolType(uint32* protocolType)
{
	uint32 retVal;
	uint32 regData;


	retVal = rtl8368s_getAsicReg(RTL8368S_SVLAN_PROTOCOL_TYPE, &regData);
	if(retVal !=  SUCCESS)
		return retVal;

	*protocolType = regData;
	
	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicSvlanConfiguration| Configure system 8 S-tag content
@parm uint32 | index | index of 8 s-tag configuration
@parm unit32 | svid | SVLAN VID
@parm enum PRIORITYVALUE | spri | Priority S-tag.
@parm unit32 | rpvid | Replace PVID of un-Ctagging frame.
@parm unit32 | cvidx | Cvidx for replace PVID
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can set system 4 accepted s-tag frame format. Only 4 SVID s-tag frame will be accpeted
	to receiving from uplink ports. Other SVID s-tag frame or s-utagged frame will be droped in uplink ports.

*/
int32 rtl8368s_setAsicSvlanConfiguration(uint32 index,uint32 svid,enum PRIORITYVALUE spri,uint32 rpvid,uint32 cvidx)
{
	uint32 retVal;
	uint16 regAddr;
	uint32 regData;
	uint32 regBits;


	if(index > RTL8368S_SVIDXMAX)
		return ERRNO_SVIDX;

	if(svid > 0xFFF)
		return ERRNO_VID;

	if(spri >= PRI_MAX)
		return ERRNO_PRIORITY;

	if(cvidx > RTL8368S_VIDXMAX)
		return ERRNO_CVIDX;

	regData = ((svid<<RTL8368S_SVLAN_SVIDX_VID_OFF)&RTL8368S_SVLAN_SVIDX_VID_MSK) |
			  ((spri<<RTL8368S_SVLAN_SVIDX_SPRI_OFF)&RTL8368S_SVLAN_SVIDX_SPRI_MSK) |
			  ((rpvid<<RTL8368S_SVLAN_SVIDX_CRPL_OFF)&RTL8368S_SVLAN_SVIDX_CRPL_MSK);

	retVal = rtl8368s_setAsicReg(RTL8368S_SVLAN_SVIDX_BASE+index,regData);
	if(SUCCESS != retVal)
		return retVal;

	if(index < 4)
	{
		regAddr = RTL8368S_SVLAN_CVIDX_BASE;
		regBits = RTL8368S_SVLAN_CVIDX_MSK<<(index*RTL8368S_SVLAN_CVIDX_BITS);
		regData = (cvidx<<(index*RTL8368S_SVLAN_CVIDX_BITS)) & regBits;
	}	
	else
	{
		regAddr = RTL8368S_SVLAN_CVIDX_BASE + 1;
		regBits = RTL8368S_SVLAN_CVIDX_MSK<<((index-4)*RTL8368S_SVLAN_CVIDX_BITS);
		regData = (cvidx<<((index-4)*RTL8368S_SVLAN_CVIDX_BITS)) & regBits;
	}	

	
	retVal = rtl8368s_setAsicRegBits(regAddr,regBits,regData);		
	
	return retVal;
	
}	

/*
@func int32 | rtl8368s_getAsicSvlanConfiguration| Configure system 8 S-tag content
@parm uint32* | index | index of 8 s-tag configuration
@parm unit32* | svid | SVLAN VID
@parm enum PRIORITYVALUE* | spri | Priority S-tag.
@parm unit32* | rpvid | Replace PVID of un-Ctagging frame.
@parm unit32* | cvidx | Cvidx for replace PVID
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can get system 4 accepted s-tag frame format. Only 4 SVID s-tag frame will be accpeted
	to receiving from uplink ports. Other SVID s-tag frame or s-utagged frame will be droped in uplink ports.

*/
int32 rtl8368s_getAsicSvlanConfiguration(uint32 index,uint32* svid,enum PRIORITYVALUE* spri,uint32* rpvid,uint32* cvidx)
{

	uint32 retVal;
	uint16 regAddr;
	uint32 regData;
	uint32 regBits;
	uint32 bitsOff;

	if(index > RTL8368S_SVIDXMAX)
		return ERRNO_SVIDX;


	retVal = rtl8368s_getAsicReg(RTL8368S_SVLAN_SVIDX_BASE+index,&regData);

	if(retVal !=  SUCCESS)
		return retVal;

	*svid = (regData & RTL8368S_SVLAN_SVIDX_VID_MSK) >> RTL8368S_SVLAN_SVIDX_VID_OFF;
	*spri = (regData & RTL8368S_SVLAN_SVIDX_SPRI_MSK) >> RTL8368S_SVLAN_SVIDX_SPRI_OFF;

	if(regData & RTL8368S_SVLAN_SVIDX_CRPL_MSK) 
		* rpvid = TRUE;
	else
		* rpvid = FALSE;

	if(index < 4)
	{
		regAddr = RTL8368S_SVLAN_CVIDX_BASE;
		regBits = RTL8368S_SVLAN_CVIDX_MSK<<(index*RTL8368S_SVLAN_CVIDX_BITS);
		bitsOff = index*RTL8368S_SVLAN_CVIDX_BITS;
	}	
	else
	{
		regAddr = RTL8368S_SVLAN_CVIDX_BASE + 1;
		regBits = RTL8368S_SVLAN_CVIDX_MSK<<((index-4)*RTL8368S_SVLAN_CVIDX_BITS);
		bitsOff = (index-4)*RTL8368S_SVLAN_CVIDX_BITS;
	}	

	retVal = rtl8368s_getAsicReg(regAddr,&regData);
	if(retVal != SUCCESS)
		return retVal;

	*cvidx =  (regData & regBits) >> bitsOff;

	return SUCCESS;
}
/*
@func int32 | rtl8368s_setAsicSvlanPortVsidx| Configure per port stag indirect index to 4 stag configuration
@parm enum PORTID | port | Physical port number.
@parm uint32 | vsidx | index of 4 s-tag configuration
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid Port Number.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can set port n S-tag format index while receiving frame from port n 
	is transmit through uplink port with s-tag field

*/
int32 rtl8368s_setAsicSvlanPortVsidx(enum PORTID port, uint32 vsidx)
{
	uint32 regData;
	uint32 regBits;
	uint32 regAddr;
	int32 retVal;

	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	if(vsidx > RTL8368S_SVIDXMAX )
		return ERRNO_SVIDX;



	if(port < PORT5)
	{
		regAddr = RTL8368S_SVLAN_PORT_CTRL_BASE;

		regBits = RTL8368S_SVLAN_VSIDX_MSK<<(RTL8368S_SVLAN_VSIDX_BITS*port);

		regData = (vsidx << (RTL8368S_SVLAN_VSIDX_BITS*port)) & regBits;
	}
	else
	{
		regAddr = RTL8368S_SVLAN_PORT_CTRL_BASE + 1;

		regBits = RTL8368S_SVLAN_VSIDX_MSK<<(RTL8368S_SVLAN_VSIDX_BITS*(port-PORT5));

		regData =(vsidx << (RTL8368S_SVLAN_VSIDX_BITS*(port-PORT5))) & regBits;
	}

	retVal = rtl8368s_setAsicRegBits(regAddr,regBits,regData);		

	return retVal;	
}

/*
@func int32 | rtl8368s_getAsicSvlanPortVsidx| get per port stag indirect index to 4 stag configuration
@parm enum PORTID | port | Physical port number.
@parm uint32* | vsidx | index of 4 s-tag configuration
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid Port Number.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can get port n S-tag format index while receiving frame from port n 
	is transmit through uplink port with s-tag field

*/
int32 rtl8368s_getAsicSvlanPortVsidx(enum PORTID port, uint32* vsidx)
{
	int32 retVal;
	uint32 regData;
	uint32 regBits;
	uint32 bitsOff;
	uint32 regAddr;

	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;


	if(port < PORT5)
	{
		regAddr = RTL8368S_SVLAN_PORT_CTRL_BASE;

		regBits = RTL8368S_SVLAN_VSIDX_MSK<<(RTL8368S_SVLAN_VSIDX_BITS*port);

		bitsOff = RTL8368S_SVLAN_VSIDX_BITS*port;
	}	
	else
	{
		regAddr = RTL8368S_SVLAN_PORT_CTRL_BASE + 1;

		regBits = RTL8368S_SVLAN_VSIDX_MSK<<(RTL8368S_SVLAN_VSIDX_BITS*(port-PORT5));

		bitsOff = RTL8368S_SVLAN_VSIDX_BITS*(port-PORT5);
	}	

	retVal = rtl8368s_getAsicReg(regAddr,&regData);
	if(retVal != SUCCESS)
		return retVal;

	*vsidx =  (regData & regBits) >> bitsOff;

	return SUCCESS;	
}


/*
@func int32 | rtl8368s_setAsicSvlanIngressUntag | Configure enable trap received un-Stag frame to CPU from unplink port/
@parm uint32 | enabled | Trap received un-Stag frame to CPU 1: enabled, 0: disabled. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	 The API can set trapping function in uplink port while receiving un-Stagging frame.
*/
int32 rtl8368s_setAsicSvlanIngressUntag(uint32 enabled)
{
	uint32 retVal;

	retVal = rtl8368s_setAsicRegBit(RTL8368S_SVLAN_CTRL_REG,RTL8368S_SVLAN_ING_UNTAG_OFF,enabled);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicSvlanIngressUntag | Configure enable trap received un-Stag frame to CPU from unplink port.
@parm uint32* | enabled | Trap received un-Stag frame to CPU 1: enabled, 0: disabled. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can get configuration of trapping function in uplink port while receiving un-Stagging frame.
	
*/
int32 rtl8368s_getAsicSvlanIngressUntag(uint32* enabled)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_SVLAN_CTRL_REG,&regData);

	if(retVal !=  SUCCESS)
		return retVal;

	regData = regData & RTL8368S_SVLAN_ING_UNTAG_MSK;

	if(regData)
		*enabled = 1;
	else
		*enabled = 0;

	return SUCCESS;
	
}


/*
@func int32 | rtl8368s_setAsicSvlanIngressUnmatch | Configure enable trap received unmatched Stag frame to CPU from unplink port.
@parm uint32 | enabled | Trap received unmatched Stag frame to CPU 1: enabled, 0: disabled. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	 The API can set trapping function in uplink port while receiving unmatched Stag frame.
*/
int32 rtl8368s_setAsicSvlanIngressUnmatch(uint32 enabled)
{
	uint32 retVal;

	retVal = rtl8368s_setAsicRegBit(RTL8368S_SVLAN_CTRL_REG,RTL8368S_SVLAN_ING_UNMAT_OFF,enabled);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicSvlanIngressUnmatch | Configure enable trap received unmatched Stag frame to CPU from unplink port.
@parm uint32* | enabled | Trap received unmatched Stag frame to CPU 1: enabled, 0: disabled. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can get configuration of trapping function in uplink port while receiving unmatched Stag frame.
	
*/
int32 rtl8368s_getAsicSvlanIngressUnmatch(uint32* enabled)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_SVLAN_CTRL_REG,&regData);

	if(retVal !=  SUCCESS)
		return retVal;

	regData = regData & RTL8368S_SVLAN_ING_UNMAT_MSK;

	if(regData)
		*enabled = 1;
	else
		*enabled = 0;

	return SUCCESS;
	
}


/*
@func int32 | rtl8368s_setAsicSvlanCpuPriority | Assign CPU priority for SVLAN trapping frame.
@parm enum PRIORITYVALUE | priority | CPU priority for trapping frame.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can assign CPU priority for unmacthed and unstag frame were trapped to CPU and this priority will be remapping after qos priority decison.
*/
int32 rtl8368s_setAsicSvlanCpuPriority(enum PRIORITYVALUE priority)
{
	uint32 retVal;
	uint32 regData;

	if(priority > PRI7)
		return ERRNO_PRIORITY; 

	regData = priority << RTL8368S_SVLAN_CPUPRI_OFF;

	retVal = rtl8368s_setAsicRegBits(RTL8368S_SVLAN_CTRL_REG,RTL8368S_SVLAN_CPUPRI_MSK,regData);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicSvlanCpuPriority | Get CPU priority for SVLAN trapping frame.
@parm enum PRIORITYVALUE* | priority | CPU priority for trapping frame.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can retrieve CPU priority for unmacthed and unstag frame were trapped to CPU
	
*/
int32 rtl8368s_getAsicSvlanCpuPriority(enum PRIORITYVALUE* priority)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_SVLAN_CTRL_REG,&regData);

	if(retVal !=  SUCCESS)
		return retVal;


	regData = (regData& RTL8368S_SVLAN_CPUPRI_MSK ) >> RTL8368S_SVLAN_CPUPRI_OFF;

	*priority = regData;

	return SUCCESS;
}




/*
@func int32 | rtl8368s_setAsicCpuPortMask| Configure cpu ports
@parm uint32 | portMask | Uplink port mask setting.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@comm
	The API can specify cpu port. All the frames send to cpu port will be
	inserted proprietary cpu tag 0x8899 if NOT disable insert CPU tag function.

*/
int32 rtl8368s_setAsicCpuPortMask(uint32 portMask)
{
	int32 retVal;


	retVal = rtl8368s_setAsicRegBits(RTL8368S_CPU_CTRL_REG,RTL8368S_CPU_PORTS_MSK,portMask<<RTL8368S_CPU_PORTS_OFF);		

	return retVal;	
}

/*
@func int32 | rtl8368s_getAsicCpuPortMask| Get cpu ports
@parm uint32* | portMask | Uplink port mask setting.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@comm
	The API can check if the specified port is a CPU port.

*/
int32 rtl8368s_getAsicCpuPortMask(uint32* portMask)
{
	uint32 regData;
	int32 retVal;

	retVal = rtl8368s_getAsicReg(RTL8368S_CPU_CTRL_REG, &regData);
	if(retVal !=  SUCCESS)
		return retVal;

	*portMask = (regData & RTL8368S_CPU_PORTS_MSK) >> RTL8368S_CPU_PORTS_OFF;

	return SUCCESS;
}
/*
@func int32 | rtl8368s_setAsicCpuDisableInsTag | Set CPU port DISABLE insert tag function
@parm uint32 | enable | 0: disable, 1: enable. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	Enable the function to NOT insert proprietary CPU tag (Length/Type 0x8899) ahead vlan tag
	to the frame that transmitted to CPU port or disable the function to insert proprietary CPU tag.
	
*/
int32 rtl8368s_setAsicCpuDisableInsTag(uint32 enable)
{
	uint32 retVal;

	retVal = rtl8368s_setAsicRegBit(RTL8368S_CPU_CTRL_REG,RTL8368S_CPU_INSTAG_OFF,enable);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicCpuDisableInsTag | Get setting of CPU port DISABLE insert tag function
@parm uint32* | enabled | 0: disable, 1: enable. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can get the setting of CPU port DISABLE insert tag function.
	
*/
int32 rtl8368s_getAsicCpuDisableInsTag(uint32* enabled)
{
	uint32 regData;
	int32 retVal;

	retVal = rtl8368s_getAsicReg(RTL8368S_CPU_CTRL_REG, &regData);
	if(retVal !=  SUCCESS)
		return retVal;

	if(regData & RTL8368S_CPU_INSTAG_MSK)
		*enabled = 1;
	else
		*enabled = 0;
	
	return SUCCESS;
}


/*
@func int32 | rtl8368s_setAsicCpuPriorityRemapping | Set CPU priority remapping parameter.
@parm enum PRIORITYVALUE | priority | Priority value.
@parm enum PRIORITYVALUE | newpriority | New priority value.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PRIORITY | Invalid priority. 
@comm
	The API can set CPU reampping source priority and new priority.
 */
int32 rtl8368s_setAsicCpuPriorityRemapping( enum PRIORITYVALUE priority, enum PRIORITYVALUE newpriority )
{
	uint32 retVal;
	uint16 regAddr;
	uint32 regData;
	uint32 regBits;
	
	/* Invalid input parameter */
	if ((priority < PRI0) || (priority > PRI7) || (newpriority < PRI0) || (newpriority > PRI7))
		return ERRNO_PRIORITY; 


	if(priority <= PRI4)
	{
		regAddr = RTL8368S_CPU_PRIORITY_BASE;
		regBits = RTL8368S_CPU_PRIORITY_MSK<<(priority*RTL8368S_CPU_PRIORITY_BITS);
		regData = (newpriority<<(priority*RTL8368S_CPU_PRIORITY_BITS)) & regBits;
	}	
	else
	{
		regAddr = RTL8368S_CPU_PRIORITY_BASE + 1;
		regBits = RTL8368S_CPU_PRIORITY_MSK<<((priority-PRI5)*RTL8368S_CPU_PRIORITY_BITS);
		regData = (newpriority<<((priority-PRI5)*RTL8368S_CPU_PRIORITY_BITS)) & regBits;
	}	

	
	retVal = rtl8368s_setAsicRegBits(regAddr,regBits,regData);		
	
	return retVal;
}

/*
@func int32 | rtl8368s_getAsicCpuPriorityRemapping | Get CPU priority remapping parameter.
@parm enum PRIORITYVALUE | priority | Priority value.
@parm enum PRIORITYVALUE* | newpriority | New priority value.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PRIORITY | Invalid priority. 
@comm
	The API can get CPU reampping source priority and new priority.
 */
int32 rtl8368s_getAsicCpuPriorityRemapping( enum PRIORITYVALUE priority, enum PRIORITYVALUE *pNewpriority )
{
	uint32 retVal;
	uint16 regAddr;
	uint32 regData;
	uint32 regBits;
	uint32 bitsOff;

	/* Invalid input parameter */
	if ((priority < PRI0) || (priority > PRI7))
		return ERRNO_PRIORITY; 

	if(priority <= PRI4)
	{
		regAddr = RTL8368S_CPU_PRIORITY_BASE;
		regBits = RTL8368S_CPU_PRIORITY_MSK<<(priority*RTL8368S_CPU_PRIORITY_BITS);
		bitsOff = priority*RTL8368S_CPU_PRIORITY_BITS;
	}	
	else
	{
		regAddr = RTL8368S_CPU_PRIORITY_BASE + 1;
		regBits = RTL8368S_CPU_PRIORITY_MSK<<((priority-PRI5)*RTL8368S_CPU_PRIORITY_BITS);
		bitsOff = (priority-PRI5)*RTL8368S_CPU_PRIORITY_BITS;
	}	

	retVal = rtl8368s_getAsicReg(regAddr,&regData);
	if(retVal != SUCCESS)
		return retVal;

	*pNewpriority =  (regData & regBits) >> bitsOff;
	return SUCCESS;

}



/*
@func int32 | rtl8368s_setAsicLinkAggregationMode | Set link aggregation mode.
@parm uint32 | mode | Link aggragation mode 1:dump 0:not dump.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@comm
	If software set link aggragation mode to dump, then ASIC will automatic set hash value mapping to link aggragation ports.  For example, aggregation port mask
	is set as 0x001011, which means that port 0,1,3 are aggregating ports, and hash value from 0 to 7 will be mapped to port (0,1,3,0,1,3,0,1). Before changing link
	aggragation mode, system should make sure that output queues in aggregated ports are empty to avoid out of order packets issuse.
*/
int32 rtl8368s_setAsicLinkAggregationMode(uint32 mode)
{

	uint32 retVal;

	retVal = rtl8368s_setAsicRegBit(RTL8368S_3AD_LINKAG_CTRL_REG,RTL8368S_3AD_LINKAG_MODE_OFF,mode);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicLinkAggregationMode | Get link aggregation mode configuration.
@parm uint32* | mode | Link aggragation mode 1:dump 0:not dump.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@comm
	The API can get Link Aggragation mode. 
*/
int32 rtl8368s_getAsicLinkAggregationMode(uint32 *mode)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_3AD_LINKAG_CTRL_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	if(regData & RTL8368S_3AD_LINKAG_MODE_MSK)
		*mode = 1;
	else
		*mode = 0;
	
	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicLinkAggregationHashAlgorithm | Configure link aggragation hashing algorithm.
@parm uint32 | algorithm | Link aggregation algorithm 0:hash(DA+SA) 1:hash(DA) 2:hash(SA).
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_LA_INVALIDHASHSEL | Invalid link aggregation hash algorithm selection.
@comm
	Either link aggragation dump mode or not dump mode, the hash algorithm will follow user setting. The hash algorithm value 00 means using hash(DA+SA)
	algorithm, value 01 means using hash(DA) algorithm and value 10 means using hash(SA) algorithm. Before changing link	aggragation algorithm, system should
	make sure that output queues in aggregated ports are empty to avoid out of order packets issuse.
*/
int32 rtl8368s_setAsicLinkAggregationHashAlgorithm(uint32 algorithm)
{
	uint32 retVal;
	uint32 regData;

	if(algorithm > RTL8368S_LAHASHSELMAX)
		return ERRNO_LA_INVALIDHASHSEL;

	regData = algorithm << RTL8368S_3AD_LINKAG_HASHSEL_OFF;

	retVal = rtl8368s_setAsicRegBits(RTL8368S_3AD_LINKAG_CTRL_REG,RTL8368S_3AD_LINKAG_HASHSEL_MSK,regData);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicLinkAggregationHashAlgorithm | Get link aggregation hashing algorithm.
@parm uint32 | algorithm | Link aggregation algorithm 0:hash(DA+SA) 1:hash(DA) 2:hash(SA).
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@comm
	The API can get link aggregation hash algorithm.

*/
int32 rtl8368s_getAsicLinkAggregationHashAlgorithm(uint32 *algorithm)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_3AD_LINKAG_CTRL_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	*algorithm = (regData&RTL8368S_3AD_LINKAG_HASHSEL_MSK)>>RTL8368S_3AD_LINKAG_HASHSEL_OFF;
	
	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicLinkAggregationPortMask | set link aggragatio port mask
@parm uint32 | portmask | support maximum 4 physicial ports
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_LA_INVALIDPORTS | invalid link aggragation port numbers.
@comm
	The API can set Link Aggragation port mask. Asic supports only 4 aggregation ports.

*/
int32 rtl8368s_setAsicLinkAggregationPortMask(uint32 portmask)
{
	uint32 retVal;
	uint32 regData;
	uint32 maskportNo;
	uint32 i;

	maskportNo = 0;
	/*checking aggregation port number*/
	/*ASIC support maximum 4 ports aggregation function*/
	for(i=0;i<PORT_MAX;i++)
	{
		if((0x01<<i) & portmask)	
			maskportNo ++;
	}

	if(maskportNo > RTL8368S_LAPORTSMAX)
		return ERRNO_LA_INVALIDPORTS;

	regData = portmask << RTL8368S_3AD_LINKAG_PORTMASK_OFF;

	regData = regData & RTL8368S_3AD_LINKAG_PORTMASK_MSK;

	retVal = rtl8368s_setAsicRegBits(RTL8368S_3AD_LINKAG_CTRL_REG,RTL8368S_3AD_LINKAG_PORTMASK_MSK,regData);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicLinkAggregationPortMask | set link aggragatio port mask
@parm uint32* | portmask | support maximum 4 physicial ports
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@comm
	The API can set Link Aggragation port mask. Asic supports only 4 aggregation ports.

*/
int32 rtl8368s_getAsicLinkAggregationPortMask(uint32* portmask)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_3AD_LINKAG_CTRL_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	*portmask = (regData&RTL8368S_3AD_LINKAG_PORTMASK_MSK)>>RTL8368S_3AD_LINKAG_PORTMASK_OFF;
	
	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicLinkAggregationHashTable | set link aggragatio hash value mapping port number
@parm uint32 | hashval | hashing value 0-7
@parm enum PORTID | port | Physical port number.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_LA_INVALIDHASHVAL | Invalid hash value. 0-7 only
@rvalue ERRNO_INPUT | Invalid Port Number.
@comm
	The API can set Link Aggragation hash value mapped to port ID. Hash value range is 0-7 and port range is 0-5.
	If mapped port ID was not set as aggragated port and ASIC will still follow the mapping result to send frame to
	port as hashing result mapping.

*/
int32 rtl8368s_setAsicLinkAggregationHashTable(uint32 hashval,enum PORTID port)
{
	uint32 retVal;
	uint32 regData;
	uint32 regAddr;
	uint32 regBits;


	/* bits mapping to hash value */
	const uint16 bits[8]= { 0x0007,0x0038,0x01C0,0x0E00,0x7000,0x0007,0x0038,0x01C0 };
	/* bits offset to hash value*/
	const uint16 bitOff[8] = { 0,3,6,9,12,0,3,6 };
	/* address offset to hash value */
	const uint16 addrOff[8]= { 0,0,0,0,0,1,1,1 };


	if(hashval > RTL8368S_LAHASHVALMAX)
		return ERRNO_LA_INVALIDHASHVAL;

	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	regAddr = RTL8368S_3AD_LINKAG_MAPPING_BASE + addrOff[hashval];

	regBits = bits[hashval];

	regData =  (port << bitOff[hashval]) & regBits;

	retVal = rtl8368s_setAsicRegBits(regAddr,regBits,regData);		
	
	return retVal;
}

/*
@func int32 | rtl8368s_getAsicLinkAggregationHashTable | get link aggragatio hash value mapping port number
@parm uint32 | hashval | hashing value 0-7
@parm enum PORTID* | port | Physical port number.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_LA_INVALIDHASHVAL | Invalid hash value. 0-7 only
@comm
	The API can get Link Aggragation hash value mapped to port ID. Hash value range is 0-7 and port range is 0-5.
	If mapped port ID was not set as aggragated port and ASIC will still follow the mapping result to send frame to
	port as hashing result mapping.

*/
int32 rtl8368s_getAsicLinkAggregationHashTable(uint32 hashval,enum PORTID* port)
{
	uint32 retVal;
	uint32 regData;
	uint32 regAddr;


	/* bits mapping to hash value */
	const uint16 bits[8]= { 0x0007,0x0038,0x01C0,0x0E00,0x7000,0x0007,0x0038,0x01C0 };
	/* bits offset to hash value*/
	const uint16 bitOff[8] = { 0,3,6,9,12,0,3,6 };
	/* address offset to hash value */
	const uint16 addrOff[8]= { 0,0,0,0,0,1,1,1 };


	if(hashval > RTL8368S_LAHASHVALMAX)
		return ERRNO_LA_INVALIDHASHVAL;



	regAddr = RTL8368S_3AD_LINKAG_MAPPING_BASE + addrOff[hashval];

	retVal = rtl8368s_getAsicReg(regAddr,&regData);
	if(retVal != SUCCESS)
		return retVal;

	*port =  (regData & bits[hashval]) >> bitOff[hashval];


	return retVal;
}

/*
@func int32 | rtl8368s_setAsicLinkAggregationFlowControl | set link aggragatio flow control
@parm uint32 | fcport | port mask for flow control of link aggragation
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@comm
	The API can set port mask for flow control in Link Aggragation

*/
int32 rtl8368s_setAsicLinkAggregationFlowControl(uint32 fcport)
{
	uint32 retVal;
	uint32 regData;

	regData = fcport & RTL8368S_3AD_LINKAG_FC_MSK;

	retVal = rtl8368s_setAsicReg(RTL8368S_3AD_LINKAG_FC_CTRL_REG,regData);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicLinkAggregationFlowControl | get link aggragatio flow control
@parm uint32* | fcport | port mask for flow control of link aggragation
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@comm
	The API can set port mask for flow control in Link Aggragation

*/
int32 rtl8368s_getAsicLinkAggregationFlowControl(uint32* fcport)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_3AD_LINKAG_FC_CTRL_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	*fcport = regData&RTL8368S_3AD_LINKAG_FC_MSK;
	
	return SUCCESS;
}

/*
@func int32 | rtl8368s_getAsicLinkAggregationQueueEmpty | get link aggragation queue empty status
@parm uint32* | qeport | port mask of queue status
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@comm
	The API can get port mask for queueing packet empty status for each port

*/
int32 rtl8368s_getAsicLinkAggregationQueueEmpty(uint32* qeport)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_3AD_LINKAG_QEMPTY_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	*qeport = regData&RTL8368S_3AD_LINKAG_QEMPTY_MSK;
	
	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicStormFiltering | Set per port  storm filtering function.
@parm enum PORTID | port | Physical Port number.
@parm uint32 | bcstorm | broadcasting storm filtering 1:enable, 0:disabled.
@parm uint32 | mcstorm | multicasting storm filtering 1:enable, 0:disabled.
@parm uint32 | undastorm | unknown destination storm filtering 1:enable, 0:disabled.
@parm uint32 | unmcstorm | unknown multicast storm filtering 1:enable, 0:disabled.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error. 
@rvalue ERRNO_PORT_NUM | Invalid Port Number.
@comm
	The API can be used to enable or disable 'Storm Filtering Control'. 
*/
int32 rtl8368s_setAsicStormFiltering(enum PORTID port, uint32 bcstorm, uint32 mcstorm, uint32 undastorm,uint32 unmcstorm)
{
	uint32 retVal;

	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	/*broadcasting storm filtering control*/
	retVal = rtl8368s_setAsicRegBit(RTL8368S_STORM_BC_CTRL_REG,port,bcstorm);
	if(retVal !=  SUCCESS)
		return retVal;

	/*multicasting storm filtering control*/
	retVal = rtl8368s_setAsicRegBit(RTL8368S_STORM_MC_CTRL_REG,port,mcstorm);
	if(retVal !=  SUCCESS)
		return retVal;

	/*unknown destination storm filtering control*/
	retVal = rtl8368s_setAsicRegBit(RTL8368S_STORM_UNDA_CTRL_REG,port,undastorm);
	if(retVal !=  SUCCESS)
		return retVal;

	/*unknown multicast storm filtering control*/
	retVal = rtl8368s_setAsicRegBit(RTL8368S_STORM_UNMC_CTRL_REG,port,unmcstorm);
	if(retVal !=  SUCCESS)
		return retVal;

	return SUCCESS;
}

/*
@func int32 | rtl8368s_getAsicStormFiltering | get per port  storm filtering function.
@parm enum PORTID | port | Physical Port number.
@parm uint32* | bcstorm | broadcasting storm filtering 1:enable, 0:disabled.
@parm uint32* | mcstorm | multicasting storm filtering 1:enable, 0:disabled.
@parm uint32* | undastorm | unknown destination storm filtering 1:enable, 0:disabled.
@parm uint32* | unmcstorm | unknown multicast storm filtering 1:enable, 0:disabled.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error. 
@rvalue ERRNO_PORT_NUM | Invalid Port Number.
@comm
	The API can be used to enable or disable 'Storm Filtering Control'. 
*/
int32 rtl8368s_getAsicStormFiltering(enum PORTID port, uint32* bcstorm, uint32* mcstorm, uint32* undastorm,uint32* unmcstorm)
{
	uint32 retVal;
	uint32 regData;

	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;


	retVal = rtl8368s_getAsicReg(RTL8368S_STORM_BC_CTRL_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	if(regData&(1<<port))
		*bcstorm = 1;
	else
		*bcstorm = 0;

	retVal = rtl8368s_getAsicReg(RTL8368S_STORM_MC_CTRL_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	if(regData&(1<<port))
		*mcstorm = 1;
	else
		*mcstorm = 0;

	retVal = rtl8368s_getAsicReg(RTL8368S_STORM_UNDA_CTRL_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	if(regData&(1<<port))
		*undastorm = 1;
	else
		*undastorm = 0;

	retVal = rtl8368s_getAsicReg(RTL8368S_STORM_UNMC_CTRL_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	if(regData&(1<<port))
		*unmcstorm = 1;
	else
		*unmcstorm = 0;

	return SUCCESS;
}
/*
@func int32 | rtl8368s_setAsicStormFiltering | Set system storm filtering meter.
@parm enum PORTID | port | Physical Port number.
@parm uint32 | rate | storm filtering rate 0~0x3FFF. (uint 64kpbs)
@parm uint32 | ifg | storm filtering rate's calculation including IPG 1:include 0:exclude 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error. 
@rvalue ERRNO_PORT_NUM | Invalid Port Number.
@comm
	The API can be used to set per system storm filtering meter
*/
int32 rtl8368s_setAsicStormFilteringMeter(enum PORTID port,uint32 rate, uint32 ifg )
{
	uint32 retVal;

	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;


	retVal = rtl8368s_setAsicReg(RTL8368S_STORM_RATE_CTRL_BASE+port,rate&RTL8368S_STORM_RATE_MSK);
	if(retVal !=  SUCCESS)
		return retVal;

	retVal = rtl8368s_setAsicRegBit(RTL8368S_STORM_MISC_CTRL_REG,RTL8368S_STORM_MISC_IFG_OFF,ifg);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicStormFiltering | get system storm filtering meter.
@parm enum PORTID | port | Physical Port number.
@parm uint32* | rate | storm filtering rate 0~0x3FFF. (uint 64kpbs)
@parm uint32* | ifg | storm filtering rate's calculation including IPG 1:include 0:exclude 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error. 
@rvalue ERRNO_PORT_NUM | Invalid Port Number.
@comm
	The API can be used to get per system storm filtering meter
*/
int32 rtl8368s_getAsicStormFilteringMeter(enum PORTID port,uint32* rate, uint32* ifg )
{
	uint32 retVal;
	uint32 regData;

	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	retVal = rtl8368s_getAsicReg(RTL8368S_STORM_RATE_CTRL_BASE+port,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	* rate = regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_STORM_MISC_CTRL_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	if(regData & RTL8368S_STORM_MISC_IFG_MSK)
	{
		*ifg = 1;
	}
	else
	{
		*ifg = 0;
	}
		
	return SUCCESS;
}

/*
@func int32 | rtl8368s_getAsicStormFilteringExceedStatus | get system storm filtering meter over status.
@parm uint32* | portmask | Port mask for storm control rate have been exceeded status.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error. 
@comm
	The API can be get storm control filtering function reach the limitation status. If the port rate limitation had been exceeded, then port
	mapped status will be set.
*/
int32 rtl8368s_getAsicStormFilteringExceedStatus(uint32* portmask)
{
	uint32 retVal;
	uint32 regData;


	retVal = rtl8368s_getAsicReg(RTL8368S_STORM_LEAKY_STATUS_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	*portmask = (regData & RTL8368S_STORM_EXCEED_MSK) >> RTL8368S_STORM_EXCEED_OFF;

	return SUCCESS;
}

/*=======================================================================
 *  Port Mirroring APIs
 *========================================================================*/
/*
@func int32 | rtl8368s_setAsicPortMirror | Configure port mirror function.
@parm enum PORTID | mirrored | Mirrored (source) port.
@parm enum PORTID | monitor | Monitor (destination) port. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid port Number.
@comm
	System supports one set of port mirror function. Mirrored port will be checked if mirror receiving frame or mirror transmitting frame to monitor port.   
*/
int32 rtl8368s_setAsicPortMirror(enum PORTID mirrored, enum PORTID monitor)
{
	uint32 retVal;
	uint32 regData;
	uint32 regBits;

	if(mirrored >=PORT_MAX)
		return ERRNO_PORT_NUM;

	if(monitor >=PORT_MAX)
		return ERRNO_PORT_NUM;

	regData = (mirrored<<RTL8368S_PORT_MIRROR_SOURCE_OFF) | (monitor<<RTL8368S_PORT_MIRROR_MINITOR_OFF);
	regBits = RTL8368S_PORT_MIRROR_SOURCE_MSK | RTL8368S_PORT_MIRROR_MINITOR_MSK;	
	
	retVal = rtl8368s_setAsicRegBits(RTL8368S_PORT_MIRROR_REG,regBits,regData);
	
	return retVal;
}

/*
@func int32 | rtl8368s_getAsicPortMirror | Get mirrored port and monitor port inforamtion.
@parm uint32 * | mirrored | The value will return mirrored port.  
@parm uint32 * | monitor | The value will return monitor port. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@comm
	The API can get the current setting mirrored port number and monitor port number 
	information.  
*/
int32 rtl8368s_getAsicPortMirror(uint32 *mirrored, uint32 *monitor)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_PORT_MIRROR_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;
	
	*mirrored = (regData&RTL8368S_PORT_MIRROR_SOURCE_MSK)>>RTL8368S_PORT_MIRROR_SOURCE_OFF;

	*monitor = (regData&RTL8368S_PORT_MIRROR_MINITOR_MSK)>>RTL8368S_PORT_MIRROR_MINITOR_OFF;
	
	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicPortMirrorRxFunction | Enable the mirror function on RX of the mirrored port. 
@parm uint32 | enabled | 1: enabled, 0: disabled. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	If the API is setted to enabled, the RX of mirrored port will be mirrorred to 
	the current monitor port.
*/
int32 rtl8368s_setAsicPortMirrorRxFunction(uint32 enabled)
{
	uint32 retVal;
	
	retVal = rtl8368s_setAsicRegBit(RTL8368S_PORT_MIRROR_REG,RTL8368S_PORT_MIRROR_RX_OFF,enabled);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicPortMirrorRxFunction | Enable the mirror function on RX of the mirrored port. 
@parm uint32* | enabled | 1: enabled, 0: disabled. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	If the API is setted to enabled, the RX of mirrored port will be mirrorred to 
	the current monitor port.
*/
int32 rtl8368s_getAsicPortMirrorRxFunction(uint32* enabled)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_PORT_MIRROR_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	if(regData & RTL8368S_PORT_MIRROR_RX_MSK)
		*enabled = 1;
	else
		*enabled = 0;
	
	return SUCCESS;
}


/*
@func int32 | rtl8368s_setAsicPortMirrorTxFunction | Enable the mirror function on TX of the mirrored port. 
@parm uint32 | enabled | 1: enabled, 0: disabled. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	If the API is setted to be enabled, the TX of mirrored port will be mirrorred to 
	the current monitor port. 
*/
int32 rtl8368s_setAsicPortMirrorTxFunction(uint32 enabled)
{
	uint32 retVal;
	
	retVal = rtl8368s_setAsicRegBit(RTL8368S_PORT_MIRROR_REG,RTL8368S_PORT_MIRROR_TX_OFF,enabled);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicPortMirrorTxFunction | Enable the mirror function on TX of the mirrored port. 
@parm uint32* | enabled | 1: enabled, 0: disabled. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	If the API is setted to be enabled, the TX of mirrored port will be mirrorred to 
	the current monitor port. 
*/
int32 rtl8368s_getAsicPortMirrorTxFunction(uint32* enabled)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_PORT_MIRROR_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	if(regData & RTL8368S_PORT_MIRROR_TX_MSK)
		*enabled = 1;
	else
		*enabled = 0;
	
	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicPortMirrorIsolation | Enable the traffic isolation on monitor port
@parm uint32 | enabled | 1: enabled, 0: disabled. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
 	If the API is setted to be enabled, the monitor port will accept frames from mirrored port only
*/
int32 rtl8368s_setAsicPortMirrorIsolation(uint32 enabled)
{
	uint32 retVal;
	
	retVal = rtl8368s_setAsicRegBit(RTL8368S_PORT_MIRROR_REG,RTL8368S_PORT_MIRROR_ISO_OFF,enabled);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicPortMirrorIsolation | Enable the traffic isolation on monitor port 
@parm uint32* | enabled | 1: enabled, 0: disabled. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can get the current mointor port isolation status.
*/
int32 rtl8368s_getAsicPortMirrorIsolation(uint32* enabled)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_PORT_MIRROR_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	if(regData & RTL8368S_PORT_MIRROR_ISO_MSK)
		*enabled = 1;
	else
		*enabled = 0;
	
	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicPortDisable | Disable physical port usage
@parm enum PORTID | port | Physical port number.
@parm uint32 | disable | disable port mask
@rvalue SUCCESS | Success.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@comm
 	This API is setted to disable physical port usage
*/
int32 rtl8368s_setAsicPortDisable(enum PORTID port,uint32 disable)
{
	uint32 retVal;

	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	retVal = rtl8368s_setAsicRegBit(RTL8368S_PORT_ENABLE_CTRL_REG,port,disable);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicPortDisable | Disable physical port usage
@parm enum PORTID | port | Physical port number.
@parm uint32* | disable | disable port mask
@rvalue SUCCESS | Success.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
 	This API is setted to disable physical port usage
*/
int32 rtl8368s_getAsicPortDisable(enum PORTID port,uint32* disable)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_PORT_ENABLE_CTRL_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	if(regData & (1<<port))
		*disable = 1;
	else
		*disable = 0;
	
	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicPortDisabel | Disable physical port usage
@parm enum PORTID | port | Physical port number.
@parm rtl8368s_portability_t | ability | ability of physical port 
@rvalue SUCCESS | Success.
@rvalue ERRNO_INPUT | Invalid input parameter.
@rvalue ERRNO_INPUT | Invalid Port Number.
@comm
 	This API is setted to ability of physical port
*/
int32 rtl8368s_setAsicPortAbility(enum PORTID port,rtl8368s_portability_t ability)
{
	uint32 retVal;
	uint32 regData;
	uint32 regBits;
	uint32 regAddr;
	/* bits mapping to port ability */
	const uint16 bits[8]= { 0x00FF,0xFF00,0x00FF,0xFF00,0x00FF,0xFF00,0x00FF,0xFF00 };
	/* bits offset to port ability*/
	const uint16 bitOff[8] = { 0,8,0,8,0,8,0,8 };
	/* address offset to port ability */
	const uint16 addrOff[8]= { 0,0,1,1,2,2,3,3 };

	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	regAddr = RTL8368S_PORT_ABILITY_BASE + addrOff[port];

	regBits = bits[port];

	regData =  *(uint16*)&ability;
	regData = (regData << bitOff[port]) & regBits;

	retVal = rtl8368s_setAsicRegBits(regAddr,regBits,regData);		
	return retVal;
}

/*
@func int32 | rtl8368s_getAsicPortDisabel | Disable physical port usage
@parm enum PORTID | port | Physical port number.
@parm rtl8368s_portability_t* | ability | ability of physical port 
@rvalue SUCCESS | Success.
@rvalue ERRNO_INPUT | Invalid input parameter.
@rvalue ERRNO_INPUT | Invalid Port Number.
@commThisIf the API is setted to ability of physical port
*/
int32 rtl8368s_getAsicPortAbility(enum PORTID port,rtl8368s_portability_t* ability)
{
	uint32 retVal;
	uint32 regData;
	/*uint32 regBits;*/
	uint32 regAddr;
	/*uint8 tempData;*/
	/* bits mapping to port ability */
	const uint16 bits[8]= { 0x00FF,0xFF00,0x00FF,0xFF00,0x00FF,0xFF00,0x00FF,0xFF00 };
	/* bits offset to port ability*/
	const uint16 bitOff[8] = { 0,8,0,8,0,8,0,8 };
	/* address offset to port ability */
	const uint16 addrOff[8]= { 0,0,1,1,2,2,3,3 };

	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;


	regAddr = RTL8368S_PORT_ABILITY_BASE + addrOff[port];

	retVal = rtl8368s_getAsicReg(regAddr,&regData);
	if(retVal != SUCCESS)
		return retVal;

	/*tempData =  (regData & bits[port]) >> bitOff[port];*/

	regData = (regData & bits[port]) >> bitOff[port];

	*ability = *(rtl8368s_portability_t*)(&regData);

	return retVal;
}

/*
@func int32 | rtl8368s_setAsicPortJamMode | Enable half duplex flow control setting
@parm uint32 | mode | 0: Back-Pressure 1: DEFER 
@rvalue SUCCESS | Success.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	If the API can be used to set half duplex flow control with setting 0: BACK-PRESSURE 1:DEFER
*/
int32 rtl8368s_setAsicPortJamMode(uint32 mode)
{
	uint32 retVal;
	
	retVal = rtl8368s_setAsicRegBit(RTL8368S_SWITCH_GLOBAL_CTRL_REG,RTL8368S_JAM_MODE_OFF,mode);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicPortJamMode | Enable half duplex flow control setting
@parm uint32* | mode | 0: Back-Pressure 1: DEFER 
@rvalue SUCCESS | Success.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	If the API can be used to get half duplex flow control mode 
*/
int32 rtl8368s_getAsicPortJamMode(uint32* mode)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_SWITCH_GLOBAL_CTRL_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	if(regData & (1<<RTL8368S_JAM_MODE_OFF))
		*mode = 1;
	else
		*mode = 0;
	
	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicMaxLengthInRx | Max receiving packet length.
@parm uint32 | maxLength | 0: 1522 bytes 1:1536 bytes 2:1552 bytes 3:16000bytes
@rvalue SUCCESS | Success.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	If the API can be used to set accepted max packet length.
*/
int32 rtl8368s_setAsicMaxLengthInRx(uint32 maxLength)
{	
	uint32 retVal;
	uint32 regData;
	uint32 regBits;


	regData = (maxLength<<RTL8368S_MAX_LENGHT_OFF);
	regBits = RTL8368S_MAX_LENGHT_MSK;	
	
	retVal = rtl8368s_setAsicRegBits(RTL8368S_SWITCH_GLOBAL_CTRL_REG,regBits,regData);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicMaxLengthInRx | Max receiving packet length.
@parm uint32* | maxLength | 0: 1522 bytes 1:1536 bytes 2:1552 bytes 3:16000bytes
@rvalue SUCCESS | Success.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	If the API can be used to get accepted max packet length.
*/
int32 rtl8368s_getAsicMaxLengthInRx(uint32* maxLength)
{
	uint32 retVal;
	uint32 regData;


	retVal = rtl8368s_getAsicReg(RTL8368S_SWITCH_GLOBAL_CTRL_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	
	*maxLength = (regData&RTL8368S_MAX_LENGHT_MSK)>>RTL8368S_MAX_LENGHT_OFF;


	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicPortLearnDisable | Disable source learning per port
@parm enum PORTID | port | Physical port number.
@parm uint32 | disable | disable port mask
@rvalue SUCCESS | Success.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
 	This API is setted to disable SA learning function per port
*/
int32 rtl8368s_setAsicPortLearnDisable(enum PORTID port,uint32 disable)
{
	uint32 retVal;

	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	retVal = rtl8368s_setAsicRegBit(RTL8368S_PORT_LEARNDIS_CTRL_REG,port,disable);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicPortLearnDisable | Disable source learning per port
@parm enum PORTID | port | Physical port number.
@parm uint32* | disable | disable port mask
@rvalue SUCCESS | Success.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
 	If the API can be used to get status of SA learning function usage of ports
*/
int32 rtl8368s_getAsicPortLearnDisable(enum PORTID port,uint32* disable)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_PORT_LEARNDIS_CTRL_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	if(regData & (1<<port))
		*disable = 1;
	else
		*disable = 0;
	
	return SUCCESS;
}

/*=======================================================================
 *  Security APIs
 *========================================================================*/
/*
@func int32 | rtl8368s_setAsicPortAge | Configure L2 LUT aging per port setting.
@parm enum PORTID | port | Physical port number.
@parm uint32 | disable | disable port mask
@rvalue SUCCESS | Success.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@comm
 	ASIC will age out L2 entry with fields Static, Auth and IP_MULT are 0. Aging out function is for new address learning LUT update because size of LUT 
 	is limited, old address information should not be kept and get more new learning SA information. Age field of L2 LUT is updated by following sequence 
 	{0b10,0b11,0b01,0b00} which means the LUT entries with age value 0b00 is free for ASIC. ASIC will use this aging sequence to decide which entry to 
 	be replace by new SA learning information. This function can be replace by setting STP state each port.
*/
int32 rtl8368s_setAsicPortAge(enum PORTID port,uint32 disable)
{
	uint32 retVal;

	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	retVal = rtl8368s_setAsicRegBit(RTL8368S_SECURITY_CTRL_REG,port+RTL8368S_DISABLE_AGE_OFF,disable);

	return retVal;

}

/*
@func int32 | rtl8368s_getAsicPortAge | Get L2 LUT aging per port setting.
@parm enum PORTID | port | Physical port number.
@parm uint32* | disable | disable port mask
@rvalue SUCCESS | Success.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@comm
 	This API can be used to get L2 LUT aging function per port. 
*/
int32 rtl8368s_getAsicPortAge(enum PORTID port,uint32* disable)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_SECURITY_CTRL_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	if(regData & (1<<(port+RTL8368S_DISABLE_AGE_OFF)))
		*disable = 1;
	else
		*disable = 0;
	
	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicFastAge | Configure L2 LUT fast aging.
@parm uint32 | enable | Fast age function setting.
@rvalue SUCCESS | Success.
@rvalue ERRNO_INVALIDINPUT | Invalid input parameter.
@comm
	Fast aging can be used to clear current LUT auto SA entries for free at short period time.
*/
int32 rtl8368s_setAsicFastAge(uint32 enable)
{
    uint32 retVal;

    retVal = rtl8368s_setAsicRegBit(RTL8368S_SPEED_CONTROL_REG, RTL8368S_SPEED_CONTROL_OFF, enable);

    return retVal;
}

/*
@func int32 | rtl8368s_getAsicFastAge | Get L2 LUT fast aging configuration.
@parm uint32* | enable | Fast age function setting
@rvalue SUCCESS | Success.
@rvalue ERRNO_INVALIDINPUT | Invalid input parameter.
@comm
	This API can be used to get L2 LUT fast aging setting.
*/
int32 rtl8368s_getAsicFastAge(uint32* enable)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_SPEED_CONTROL_REG, &regData);
	if(retVal !=  SUCCESS)
		return retVal;

	if(regData & RTL8368S_SPEED_CONTROL_OFF)
		*enable = 1;
	else
		*enable = 0;
	
	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicDropUnknownDa | drop pakcet if DA is unknow
@parm uint32 | enable | 0:disable 1:enable
@rvalue SUCCESS | Success.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	This API can be used to drop packet if DA is unknown
*/
int32 rtl8368s_setAsicDropUnknownDa(uint32 enable)
{
	uint32 retVal;
	
	retVal = rtl8368s_setAsicRegBit(RTL8368S_SECURITY_CTRL2_REG,RTL8368S_DROP_UNDA_OFF,enable);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicDropUnknownDa | drop pakcet if DA is unknow
@parm uint32* | enabled | 0:disable 1:enable
@rvalue SUCCESS | Success.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	This API can be used to drop packet if DA is unknown
*/
int32 rtl8368s_getAsicDropUnknownDa(uint32* enabled)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_SECURITY_CTRL2_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	if(regData & RTL8368S_DROP_UNDA_MSK)
		*enabled = 1;
	else
		*enabled = 0;
	
	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicDropUnknownSa | drop pakcet if SA is unknow
@parm uint32 | enable | 0:disable 1:enable
@rvalue SUCCESS | Success.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	This API can be used to drop packet if SA is unknown
*/
int32 rtl8368s_setAsicDropUnknownSa(uint32 enable)
{
	uint32 retVal;
	
	retVal = rtl8368s_setAsicRegBit(RTL8368S_SECURITY_CTRL2_REG,RTL8368S_DROP_UNSA_OFF,enable);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicDropUnknownSa | drop pakcet if SA is unknow
@parm uint32* | enabled | 0:disable 1:enable
@rvalue SUCCESS | Success.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	This API can be used to drop packet if SA is unknown
*/
int32 rtl8368s_getAsicDropUnknownSa(uint32 *enabled)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_SECURITY_CTRL2_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	if(regData & RTL8368S_DROP_UNSA_MSK)
		*enabled = 1;
	else
		*enabled = 0;
	
	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicDropUnmatchedSa | drop pakcet if SA is no from the same source port as L2 SPA
@parm uint32 | enable | 0:disable 1:enable
@rvalue SUCCESS | Success.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	This API can be used to drop packet if SA is no from the same source port as L2 SPA
*/
int32 rtl8368s_setAsicDropUnmatchedSa(uint32 enable)
{
	uint32 retVal;
	
	retVal = rtl8368s_setAsicRegBit(RTL8368S_SECURITY_CTRL2_REG,RTL8368S_DROP_UNMATCHSA_OFF,enable);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicDropUnmatchedSa | drop pakcet if SA is no from the same source port as L2 SPA
@parm uint32* | enabled | 0:disable 1:enable
@rvalue SUCCESS | Success.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	This API can be used to drop packet if SA is no from the same source port as L2 SPA
*/
int32 rtl8368s_getAsicDropUnmatchedSa(uint32* enabled)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_SECURITY_CTRL2_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	if(regData & RTL8368S_DROP_UNMATCHSA_MSK)
		*enabled = 1;
	else
		*enabled = 0;
	
	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicBlockSpa | Disable blocking frame if source port and destination port are the same.
@parm enum PORTID | port | Physical port number.
@parm uint32 | disabled | disable port mask
@rvalue SUCCESS | Success.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@comm
 	This API is setted to disable block frame if source port = destination port.
*/
int32 rtl8368s_setAsicBlockSpa(enum PORTID port,uint32 disabled)
{
	uint32 retVal;

	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	retVal = rtl8368s_setAsicRegBit(RTL8368S_SECURITY_CTRL_REG,port+RTL8368S_DISABLE_BLKSPA_OFF,disabled);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicPortLearnDisable | Disable source learning per port
@parm enum PORTID | port | Physical port number.
@parm uint32* | disabled | disable port mask
@rvalue SUCCESS | Success.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@comm
 	If the API can be used to get status of blocking disable function if Source port= Destination port.
*/
int32 rtl8368s_getAsicBlockSpa(enum PORTID port,uint32* disabled)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_SECURITY_CTRL_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	if(regData & (1<<(port+RTL8368S_DISABLE_BLKSPA_OFF)))
		*disabled = 1;
	else
		*disabled = 0;
	
	return SUCCESS;
}




/*
@func int32 | rtl8368s_setAsicInterruptPolarity | set interrupt trigger polarity
@parm uint32 | polarity | 0:pull high 1: pull low
@rvalue SUCCESS | Success.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	This API can be used to set I/O polarity while port linking status chnaged. Pull high GPIO
	while setting value is 0 and pull low while setting value 1
*/
int32 rtl8368s_setAsicInterruptPolarity(uint32 polarity)
{
	uint32 retVal;
	
	retVal = rtl8368s_setAsicRegBit(RTL8368S_INTERRUPT_CONTROL_REG,RTL8368S_INTERRUPT_POLARITY_OFF,polarity);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicInterruptPolarity | set interrupt trigger polarity
@parm uint32* | polarity | 0:pull high 1: pull low
@rvalue SUCCESS | Success.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	This API can be used to set I/O polarity while port linking status chnaged. Pull high GPIO
	while setting value is 0 and pull low while setting value 1
*/
int32 rtl8368s_getAsicInterruptPolarity(uint32* polarity)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_INTERRUPT_CONTROL_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	if(regData & RTL8368S_INTERRUPT_POLARITY_MSK)
		*polarity = 1;
	else
		*polarity = 0;
	
	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicInterruptMask | set interrupt enable mask
@parm uint32* | mask | [11:6]-enable Link Down interrupt [5:0]-enable Link Up interrupt
@rvalue SUCCESS | Success.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
 	This API can be used to set ASIC interrupt enable condition link down/up for each port.
*/
int32 rtl8368s_setAsicInterruptMask(uint32 mask)
{
	uint32 retVal;
	
    retVal = rtl8368s_setAsicRegBits(RTL8368S_INTERRUPT_MASK_REG, 
                                     (RTL8368S_INTERRUPT_LINKCHANGE_MSK | RTL8368S_INTERRUPT_ACLEXCEED_MSK | RTL8368S_INTERRUPT_STORMEXCEED_MSK),
                                     mask);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicInterruptMask | set interrupt enable mask
@parm uint32 | mask | [11:6]-enable Link Down interrupt [5:0]-enable Link Up interrupt
@rvalue SUCCESS | Success.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
 	This API can be used to set ASIC interrupt enable condition link down/up for each port.
*/
int32 rtl8368s_getAsicInterruptMask(uint32* mask)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_INTERRUPT_MASK_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	*mask = regData;
	
	return SUCCESS;
}

/*
@func int32 | rtl8368s_getAsicInterruptStatus | get interrupt status
@parm uint32 | mask | [11:6]-enable Link Down interrupt [5:0]-enable Link Up interrupt
@rvalue SUCCESS | Success.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
 	This API can be used to get ASIC interrupt status and register will be clear by READ
*/
int32 rtl8368s_getAsicInterruptStatus(uint32* mask)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_INTERRUPT_STATUS_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	*mask = regData;
	
	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicMIBsCounterReset | Set MIBs global reset or per-port reset.
@parm uint32 | mask | Port reset mask in bit[8:3] abd global reset mask in bit [2].
@rvalue SUCCESS | Success.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
 	ASIC will clear all MIBs counter by global resetting and clear counters associated with a particular port by mapped port resetting. 
*/
int32 rtl8368s_setAsicMIBsCounterReset(uint32 mask)
{
	uint32 retVal;

	mask = mask & RTL8368S_MIB_CTRL_USER_MSK;
	
	retVal = rtl8368s_setAsicReg(RTL8368S_MIB_CTRL_REG,mask);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicMIBsCounter | Get MIBs counter.
@parm enum PORTID | port | Physical port number (0~5).
@num RTL8368S_MIBCOUNTER | mibIdx | MIB counter index.
@parm uint64* | counter | MIB retrived counter.
@rvalue SUCCESS | Success.
@rvalue ERRNO_MIB_INVALIDPORT | Invalid port number
@rvalue ERRNO_MIB_INVALIDIDX | Invalid MIBs index.
@rvalue ERRNO_MIB_BUSY | MIB is busy at retrieving
@rvalue ERRNO_MIB_RESET | MIB is resetting.
@comm
 	Before MIBs counter retrieving, writting accessing address to ASIC at first and check the MIB control register status. If busy bit of MIB control is set, that
 	mean MIB counter have been waiting for preparing, then software must wait atfer this busy flag reset by ASIC. This driver did not recycle reading user desired
 	counter. Software must use driver again to get MIB counter if return value is not SUCCESS.

*/
int32 rtl8368s_getAsicMIBsCounter(enum PORTID port,enum RTL8368S_MIBCOUNTER mibIdx,uint64* counter)
{
	uint32 retVal;
	uint32 regAddr;
	uint32 regData;
	uint32 regOff;

	/* address offset to MIBs counter */
	const uint16 mibLength[RTL8368S_MIBS_NUMBER]= {4,4,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,4,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2};

	uint16 i;
	uint64 mibCounter;


	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	if(mibIdx>= RTL8368S_MIBS_NUMBER)
		return ERRNO_MIB_INVALIDIDX;

	if(mibIdx == Dot1dTpLearnEntryDiscardFlag)
	{
		regAddr = RTL8368S_MIB_DOT1DTPLEARNDISCARD;
	}
	else
	{
		i = 0;
		regOff = RTL8368S_MIB_COUTER_PORT_OFFSET*(port);

		while(i<mibIdx)
		{
			regOff += mibLength[i];
			i++;
		}		
		
		regAddr = RTL8368S_MIB_COUTER_BASE + regOff;
	}


	/*writing access counter address first*/
	/*then ASIC will prepare 64bits counter wait for being retrived*/
	regData = 0;/*writing data will be discard by ASIC*/
	retVal = rtl8368s_setAsicReg(regAddr,regData);


	/*read MIB control register*/
	retVal = rtl8368s_getAsicReg(RTL8368S_MIB_CTRL_REG,&regData);

	if(regData & RTL8368S_MIB_CTRL_BUSY_MSK)
		return ERRNO_MIB_BUSY;

	if(regData & RTL8368S_MIB_CTRL_RESET_MSK)
		return ERRNO_MIB_RESET;

	mibCounter = 0;
	regAddr = regAddr + mibLength[mibIdx]-1;
	i = mibLength[mibIdx];
	while(i)
	{
		retVal = rtl8368s_getAsicReg(regAddr,&regData);
		if(retVal != SUCCESS)
			return retVal;

		mibCounter = (mibCounter<<16) | (regData & 0xFFFF);

		regAddr --;
		i --;
		
	}
	
	*counter = mibCounter;	
	
	return SUCCESS;
}

/*
@func int32 | rtl8368s_getAsicMIBsControl | Get MIB control register.
@parm uint32* | mask | MIB control mask bit[0]-busy bit[1]-resetting bit[9:2]-port reset bit[10]-QM reset  bit[11]-global reset .
@rvalue SUCCESS | Success.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
 	Software need to check this control register atfer doing port resetting or global resetting.
*/
int32 rtl8368s_getAsicMIBsControl(uint32* mask)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_MIB_CTRL_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	*mask = regData & RTL8368S_MIB_CTRL_USER_MSK;
	
	return SUCCESS;
}

/*
@func int32  | rtl8368s_setAsic1xPBEnConfig | write 802.1x port-based port enable configuration
@parm enum PORTID | port | Physical port number.
@parm uint32 | enabled | 1: enabled, 0: disabled.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@comm
	The API can update the port-based port enable register content.
*/
int32 rtl8368s_setAsic1xPBEnConfig(enum PORTID port,uint32 enabled)
{
	int32 retVal;

	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	retVal = rtl8368s_setAsicRegBit(RTL8368S_1X_PORT_BASE_CRTL_0_REG,port+RTL8368S_1X_PORT_BASE_EN_OFF,enabled);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsic1xPBEnConfig | get 802.1x port-based port enable configuration
@parm enum PORTID | port | Physical port number.
@parm uint32* | enabled | 1: enabled, 0: disabled.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@comm
	The API can get 802.1x port-based port enable information.
	
*/
int32 rtl8368s_getAsic1xPBEnConfig(enum PORTID port,uint32 *enabled)
{
	int32 retVal;
	uint32 regData;

	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	retVal = rtl8368s_getAsicReg(RTL8368S_1X_PORT_BASE_CRTL_0_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	if(regData & (1<<(port+RTL8368S_1X_PORT_BASE_EN_OFF)))
	{
		*enabled = 1;
	}
	else
	{
		*enabled = 0;
	}

	return SUCCESS;
}

/*
@func int32  | rtl8368s_setAsic1xPBAuthConfig | write 802.1x port-based auth. port configuration
@parm enum PORTID | port | Physical port number.
@parm uint32 | auth | 1: authorised, 0: non-authorised.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@comm
	The API can update the port-based auth. port register content.
*/
int32 rtl8368s_setAsic1xPBAuthConfig(enum PORTID port,uint32 auth)
{
	int32 retVal;

	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	retVal = rtl8368s_setAsicRegBit(RTL8368S_1X_PORT_BASE_CRTL_0_REG,port+RTL8368S_1X_PORT_BASE_AUTH_OFF,auth);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsic1xPBAuthConfig | get 802.1x port-based auth. port configuration
@parm enum PORTID | port | Physical port number.
@parm uint32* | auth | 1: authorised, 0: non-authorised.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@comm
	The API can get 802.1x port-based auth. port information.
	
*/
int32 rtl8368s_getAsic1xPBAuthConfig(enum PORTID port,uint32* auth)
{
	int32 retVal;
	uint32 regData;

	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	retVal = rtl8368s_getAsicReg(RTL8368S_1X_PORT_BASE_CRTL_0_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	if(regData & (1<<(port+RTL8368S_1X_PORT_BASE_AUTH_OFF)))
	{
		*auth = 1;
	}
	else
	{
		*auth = 0;
	}

	return SUCCESS;
}

/*
@func int32  | rtl8368s_setAsic1xPBOpdirConfig | write 802.1x port-based operational direction configuration
@parm enum PORTID | port | Physical port number.
@parm uint32 | opdir | Operation direction 1: IN, 0:BOTH
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@comm
	The API can update the port-based operational direction register content.
*/
int32 rtl8368s_setAsic1xPBOpdirConfig(enum PORTID port,uint32 opdir)
{
	int32 retVal;

	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	retVal = rtl8368s_setAsicRegBit(RTL8368S_1X_PORT_BASE_CRTL_1_REG,port+RTL8368S_1X_PORT_BASE_OP_OFF,opdir);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsic1xPBOpdirConfig | get 802.1x port-based operational direction configuration
@parm enum PORTID | port | Physical port number.
@parm uint32* | opdir | Operation direction 1: IN, 0:BOTH
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@comm
	The API can get 802.1x port-based operational direction information.
	
*/
int32 rtl8368s_getAsic1xPBOpdirConfig(enum PORTID port,uint32* opdir)
{
	int32 retVal;
	uint32 regData;

	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	retVal = rtl8368s_getAsicReg(RTL8368S_1X_PORT_BASE_CRTL_1_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	if(regData & (1<<(port+RTL8368S_1X_PORT_BASE_OP_OFF)))
	{
		*opdir = 1;
	}
	else
	{
		*opdir = 0;
	}

	return SUCCESS;
}

/*
@func int32  | rtl8368s_setAsic1xMBEnConfig | write 802.1x mac-based port enable configuration
@parm enum PORTID | port | Physical port number.
@parm uint32 | enabled | 1: enabled, 0: disabled.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@comm
	The API can update the mac-based port enable register content.
*/
int32 rtl8368s_setAsic1xMBEnConfig(enum PORTID port,uint32 enabled)
{
	int32 retVal;

	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	retVal = rtl8368s_setAsicRegBit(RTL8368S_1X_MAC_BASE_CRTL_REG,port+RTL8368S_1X_MAC_BASE_EN_OFF,enabled);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsic1xMBEnConfig | get 802.1x mac-based port enable configuration
@parm enum PORTID | port | Physical port number.
@parm uint32* | enabled | 1: enabled, 0: disabled.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_REGUNDEFINE | Register undefine.
@comm
	The API can get 802.1x mac-based port enable information.
	
*/
int32 rtl8368s_getAsic1xMBEnConfig(enum PORTID port,uint32* enabled)
{
	int32 retVal;
	uint32 regData;

	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	retVal = rtl8368s_getAsicReg(RTL8368S_1X_MAC_BASE_CRTL_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	if(regData & (1<<(port+RTL8368S_1X_MAC_BASE_EN_OFF)))
	{
		*enabled = 1;
	}
	else
	{
		*enabled = 0;
	}

	return SUCCESS;
}

/*
@func int32  | rtl8368s_setAsic1xMBOpdirConfig | write 802.1x mac-based operational direction configuration
@parm uint32 | opdir | Operation direction 1: IN, 0:BOTH
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@comm
	The API can update the mac-based operational direction register content.
*/
int32 rtl8368s_setAsic1xMBOpdirConfig(uint32 opdir)
{
	uint32 retVal;
	
	retVal = rtl8368s_setAsicRegBit(RTL8368S_1X_MAC_BASE_CRTL_REG,RTL8368S_1X_MAC_BASE_OP_OFF,opdir);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsic1xMBOpdirConfig | get 802.1x mac-based operational direction configuration
@parm uint32* | opdir | Operation direction 1: IN, 0:BOTH
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@comm
	The API can get 802.1x mac-based operational direction information.
	
*/
int32 rtl8368s_getAsic1xMBOpdirConfig(uint32 *opdir)
{
	int32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_1X_MAC_BASE_CRTL_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	if(regData & (1<<RTL8368S_1X_MAC_BASE_OP_OFF))
	{
		*opdir = 1;
	}
	else
	{
		*opdir = 0;
	}
	return SUCCESS;

}

/*
@func int32  | rtl8368s_setAsic1xProcConfig | write 802.1x unauth. behavior configuration
@parm enum UNAUTH1XPRO | proc | 802.1x unauth. behavior configuration 0:drop 1:trap to CPU 2:Guest VLAN
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can update the 802.1x unauth. behavior content.
*/
int32 rtl8368s_setAsic1xProcConfig(enum UNAUTH1XPROC proc)
{
	int32 retVal;

	if(proc >= UNAUTH_MAX)
		return ERRNO_INPUT;

	retVal = rtl8368s_setAsicRegBits(RTL8368S_1X_QUEST_VLAN_CRTL_REG,RTL8368S_1X_QUEST_VLAN_PRO_MSK,proc);

	return SUCCESS;
}

/*
@func int32 | rtl8368s_getAsic1xProcConfig | get 802.1x unauth. behavior configuration
@parm enum UNAUTH1XPRO* | proc | 802.1x unauth. behavior configuration 0:drop 1:trap to CPU 2:Guest VLAN
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_REGUNDEFINE | Register undefine.
@comm
	The API can get 802.1x unauth. behavior configuration.
	
*/
int32 rtl8368s_getAsic1xProcConfig(enum UNAUTH1XPROC* proc)
{
	int32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_1X_QUEST_VLAN_CRTL_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	*proc = (regData & RTL8368S_1X_QUEST_VLAN_PRO_MSK) >> RTL8368S_1X_QUEST_VLAN_PRO_OFF;

	return SUCCESS;
}

/*
@func int32  | rtl8368s_setAsicGVIndexConfig | write 802.1x guest vlan index
@parm uint32 | index | 802.1x guest vlan index
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_CVIDX | Invalid cvid index
@comm
	The API can update the 802.1x guest vlan index content.
*/
int32 rtl8368s_setAsic1xGuestVidx(uint32 index)
{
	int32 retVal;

	if(index > RTL8368S_VLANMCIDXMAX)
		return ERRNO_CVIDX;

	retVal = rtl8368s_setAsicRegBits(RTL8368S_1X_QUEST_VLAN_CRTL_REG,RTL8368S_1X_QUEST_VIDX_MSK,index<<RTL8368S_1X_QUEST_VIDX_OFF);

	return SUCCESS;
}

/*
@func int32 | rtl8368s_getAsicGVIndexConfig | get 802.1x guest vlan index configuration
@parm uint32* | index | 802.1x guest vlan index
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@comm
	The API can get 802.1x guest vlan index configuration.
	
*/
int32 rtl8368s_getAsic1xGuestVidx(uint32 *index)
{
	int32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_1X_QUEST_VLAN_CRTL_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	*index = (regData & RTL8368S_1X_QUEST_VIDX_MSK) >> RTL8368S_1X_QUEST_VIDX_OFF;

	return SUCCESS;
}

/*
@func int32  | rtl8368s_setAsic1xGVTalkConfig | write 802.1x guest vlan talk to auth. DA
@parm uint32 | enabled | 0:disable 1:enable
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@comm
	The API can update the 802.1x guest vlan talk to auth. DA content.
*/
int32 rtl8368s_setAsic1xGuestTalkToAuth(uint32 enabled)
{
	uint32 retVal;
	
	retVal = rtl8368s_setAsicRegBit(RTL8368S_1X_QUEST_VLAN_CRTL_REG,RTL8368S_1X_QUEST_TALK_TO_AUTH_OFF,enabled);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsic1xGuestTalkToAuth | get 802.1x guest vlan talk to unauth. DA configuration
@parm uint32* | enabled | 0:disable 1:enable
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_REGUNDEFINE | Register undefine.
@comm
	The API can get 802.1x guest vlan talk to unauth. DA configuration.
	
*/
int32 rtl8368s_getAsic1xGuestTalkToAuth(uint32 *enabled)
{
	uint32 retVal;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_1X_QUEST_VLAN_CRTL_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	if(regData & RTL8368S_1X_QUEST_TALK_TO_AUTH_MSK)
		*enabled = 1;
	else
		*enabled = 0;
	
	return SUCCESS;
}

/*=======================================================================
 *PHY APIs 
 *========================================================================*/
/*
@func int32 | rtl8368s_setAsicPHYReg | Set PHY registers .
@parm uint32 | phyNo | PHY number (0~3).
@parm uint32 | page | PHY page (0~7).
@parm uint32 | addr | PHY address (0~31).
@parm uint32 | data | Writing data.
@rvalue SUCCESS | 
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8368s_setAsicPHYRegs( uint32 phyNo, uint32 page, uint32 addr, uint32 data )
{
	int32   retVal,phySmiAddr;
    uint32  dummy;

	if(phyNo > RTL8368S_PHY_NO_MAX)
		return ERROR_PHY_INVALIDPHYNO;

	if(page > RTL8368S_PHY_PAGE_MAX)
		return ERROR_PHY_INVALIDPHYPAGE;

	if(addr > RTL8368S_PHY_REG_MAX)
		return ERROR_PHY_INVALIDREG;

    /* Dummy read before write PHY register */
    retVal = rtl8368s_getAsicPHYRegs(phyNo, page, addr, &dummy);
    if (retVal !=  SUCCESS) 
		return retVal;

	retVal = rtl8368s_setAsicReg(RTL8368S_PHY_ACCESS_CTRL_REG, RTL8368S_PHY_CTRL_WRITE);
	if (retVal !=  SUCCESS) 
		return retVal;

	phySmiAddr = 0x8000 | (1<<(phyNo +RTL8368S_PHY_NO_OFFSET)) | 
			((page <<RTL8368S_PHY_PAGE_OFFSET)&RTL8368S_PHY_PAGE_MASK) | (addr &RTL8368S_PHY_REG_MASK);
	
	retVal = rtl8368s_setAsicReg(phySmiAddr, data);
	if (retVal !=  SUCCESS) 
		return retVal;


	return SUCCESS;	
}

/*
@func int32 | rtl8368s_getAsicPHYReg | get PHY registers .
@parm uint32 | phyNo | PHY number (0~3).
@parm uint32 | page | PHY page (0~7).
@parm uint32 | addr | PHY address (0~31).
@parm uint32* | data | Read data.
@rvalue SUCCESS | 
@rvalue FAILED | invalid parameter
@comm
 */
int32 rtl8368s_getAsicPHYRegs( uint32 phyNo, uint32 page, uint32 addr, uint32 *data )
{
	int32 retVal,phySmiAddr;

	if(phyNo > RTL8368S_PHY_NO_MAX)
		return ERROR_PHY_INVALIDPHYNO;

	if(page > RTL8368S_PHY_PAGE_MAX)
		return ERROR_PHY_INVALIDPHYPAGE;

	if(addr > RTL8368S_PHY_REG_MAX)
		return ERROR_PHY_INVALIDREG;

	
	retVal = rtl8368s_setAsicReg(RTL8368S_PHY_ACCESS_CTRL_REG, RTL8368S_PHY_CTRL_READ);
	if (retVal !=  SUCCESS) 
		return retVal;

	phySmiAddr = 0x8000 | (1<<(phyNo +RTL8368S_PHY_NO_OFFSET)) | 
			((page <<RTL8368S_PHY_PAGE_OFFSET)&RTL8368S_PHY_PAGE_MASK) | (addr &RTL8368S_PHY_REG_MASK);
	
	retVal = rtl8368s_setAsicReg(phySmiAddr, 0);
	if (retVal !=  SUCCESS) 
		return retVal;

	//For Verify PHY Read issue, mark this place
	retVal = rtl8368s_setAsicReg(phySmiAddr, 0);
	if (retVal !=  SUCCESS) 
		return retVal;

	retVal = rtl8368s_getAsicReg(RTL8368S_PHY_ACCESS_DATA_REG,data);
	if(retVal !=  SUCCESS)
		return retVal;

	return SUCCESS;	
}
/*
@func int32 | rtl8368s_setAsicExtPHYReg | Set external PHY registers .
@parm uint32 | phy | PHY address (0~31).
@parm uint32 | reg | PHY register (0~31).
@parm uint32 | data | Writing data.
@rvalue SUCCESS | 
@rvalue FAILED | invalid parameter
@rvalue ERROR_PHY_INVALIDPHYNO | invalid PHY address
@rvalue ERROR_PHY_INVALIDREG | invalid PHY register
@rvalue ERROR_PHY_ACCESSBUSY | external PHY access busy
@comm
 */
int32 rtl8368s_setAsicExtPHYReg(uint32 phy, uint32 reg, uint32 data )
{
	int32 retVal;
	uint32 regData;

	if(phy > RTL8368S_PHY_EXT_ADDRESS_MAX)
		return ERROR_PHY_INVALIDPHYNO;

	if(reg > RTL8368S_PHY_REG_MAX)
		return ERROR_PHY_INVALIDREG;

	/*Check external PHY access busy indicator*/
	retVal = rtl8368s_getAsicReg(RTL8368S_PHY_ACCESS_BUSY_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	if(regData & RTL8368S_PHY_EXT_BUSY_MASK)
		return ERROR_PHY_ACCESSBUSY;

	/*Write external PHY WR register*/
	retVal = rtl8368s_setAsicReg(RTL8368S_PHY_EXT_WRDATA_REG, data);
	if (retVal !=  SUCCESS) 
		return retVal;	

	/*Write PHY access comand and address & register*/
	regData = RTL8368S_PHY_EXT_COMMAND_MASK |
				((phy<<RTL8368S_PHY_EXT_ADDR_OFFSET) & RTL8368S_PHY_EXT_ADDR_MASK ) | 
				((reg<<RTL8368S_PHY_EXT_REG_OFFSET) & RTL8368S_PHY_EXT_REG_MASK );
	

	retVal = rtl8368s_setAsicReg(RTL8368S_PHY_EXT_CTRL_REG, regData);
	if (retVal !=  SUCCESS) 
		return retVal;

	return SUCCESS;	
}

/*
@func int32 | rtl8368s_getAsicExtPHYReg | get PHY registers .
@parm uint32 | phy | PHY address (0~31).
@parm uint32 | reg | PHY register (0~31).
@parm uint32* | data | Read data.
@rvalue SUCCESS | 
@rvalue FAILED | invalid parameter
@rvalue ERROR_PHY_INVALIDPHYNO | invalid PHY address
@rvalue ERROR_PHY_INVALIDREG | invalid PHY register
@rvalue ERROR_PHY_ACCESSBUSY | external PHY access busy
@comm
 */
int32 rtl8368s_getAsicExtPHYReg(uint32 phy, uint32 reg, uint32 *data )
{
	int32 retVal;
	uint32 regData,i;

	if(phy > RTL8368S_PHY_EXT_ADDRESS_MAX)
		return ERROR_PHY_INVALIDPHYNO;

	if(reg > RTL8368S_PHY_REG_MAX)
		return ERROR_PHY_INVALIDPHYPAGE;

	/*Write PHY access comand and address & register*/
	regData = RTL8368S_PHY_EXT_COMMAND_MASK |RTL8368S_PHY_EXT_RW_MASK | 
				((phy<<RTL8368S_PHY_EXT_ADDR_OFFSET) & RTL8368S_PHY_EXT_ADDR_MASK ) | 
				((reg<<RTL8368S_PHY_EXT_REG_OFFSET) & RTL8368S_PHY_EXT_REG_MASK );
	

	retVal = rtl8368s_setAsicReg(RTL8368S_PHY_EXT_CTRL_REG, regData);
	if (retVal !=  SUCCESS) 
		return retVal;

	/*check PHY BUSY checking*/
	i=0;
	while(i<RTL8368S_TBL_CMD_CHECK_COUNTER)
	{
		retVal = rtl8368s_getAsicReg(RTL8368S_PHY_ACCESS_BUSY_REG,&regData);
		if(retVal !=  SUCCESS)
			return retVal;
		/*Check external PHY access busy indicator*/
		if(regData &RTL8368S_PHY_EXT_BUSY_MASK)
		{
			i++;
			if(i==RTL8368S_TBL_CMD_CHECK_COUNTER)
				return ERROR_PHY_ACCESSBUSY;
		}
		else
			break;
	}


	/*Get extern PHY RD data*/
	retVal = rtl8368s_getAsicReg(RTL8368S_PHY_EXT_RDDATA_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;
	
	*data = regData;
		
	return SUCCESS;	
}


/*
@func int32 | rtl8368s_getAsicExtPHYReg_withPage | get PHY registers .
@parm uint32 | phy | PHY address (0~31).
@parm uint32 | page | page number(0~7).
@parm uint32 | reg | PHY register (0~31).
@parm uint32* | data | Read data.
@rvalue SUCCESS | 
@rvalue FAILED | invalid parameter
@rvalue ERROR_PHY_INVALIDPHYNO | invalid PHY address
@rvalue ERROR_PHY_INVALIDREG | invalid PHY register
@rvalue ERROR_PHY_ACCESSBUSY | external PHY access busy
@comm
 */
int32 rtl8368s_getAsicExtPHYReg_withPage(uint32 phy, uint32 page, uint32 reg, uint32 *data )
{
	int32 retVal;
	uint32 regData,i;

	if(phy > RTL8368S_PHY_EXT_ADDRESS_MAX)
		return ERROR_PHY_INVALIDPHYNO;

    if(page > RTL8368S_PHY_PAGE_MAX)
		return ERROR_PHY_INVALIDPHYPAGE;

	if(reg > RTL8368S_PHY_REG_MAX)
		return ERROR_PHY_INVALIDREG;

    /* Change page */
    retVal = rtl8368s_setAsicReg(RTL8368S_PHY_EXT_WRDATA_REG, page);
	if (retVal !=  SUCCESS) 
		return retVal;

    regData = RTL8368S_PHY_EXT_COMMAND_MASK | 
				((phy<<RTL8368S_PHY_EXT_ADDR_OFFSET) & RTL8368S_PHY_EXT_ADDR_MASK ) | 
				((RTL8368S_PHY_EXT_PAGE_REGNO<<RTL8368S_PHY_EXT_REG_OFFSET) & RTL8368S_PHY_EXT_REG_MASK );

    retVal = rtl8368s_setAsicReg(RTL8368S_PHY_EXT_CTRL_REG, regData);
	if (retVal !=  SUCCESS) 
		return retVal;

    /*check PHY BUSY checking*/
	i=0;
	while(i<RTL8368S_TBL_CMD_CHECK_COUNTER)
	{
		retVal = rtl8368s_getAsicReg(RTL8368S_PHY_ACCESS_BUSY_REG,&regData);
		if(retVal !=  SUCCESS)
			return retVal;
		/*Check external PHY access busy indicator*/
		if(regData &RTL8368S_PHY_EXT_BUSY_MASK)
		{
			i++;
			if(i==RTL8368S_TBL_CMD_CHECK_COUNTER)
				return ERROR_PHY_ACCESSBUSY;
		}
		else
			break;
	}

	/*Write PHY access comand and address & register*/
	regData = RTL8368S_PHY_EXT_COMMAND_MASK |RTL8368S_PHY_EXT_RW_MASK | 
				((phy<<RTL8368S_PHY_EXT_ADDR_OFFSET) & RTL8368S_PHY_EXT_ADDR_MASK ) | 
				((reg<<RTL8368S_PHY_EXT_REG_OFFSET) & RTL8368S_PHY_EXT_REG_MASK );
	

	retVal = rtl8368s_setAsicReg(RTL8368S_PHY_EXT_CTRL_REG, regData);
	if (retVal !=  SUCCESS) 
		return retVal;

	/*check PHY BUSY checking*/
	i=0;
	while(i<RTL8368S_TBL_CMD_CHECK_COUNTER)
	{
		retVal = rtl8368s_getAsicReg(RTL8368S_PHY_ACCESS_BUSY_REG,&regData);
		if(retVal !=  SUCCESS)
			return retVal;
		/*Check external PHY access busy indicator*/
		if(regData &RTL8368S_PHY_EXT_BUSY_MASK)
		{
			i++;
			if(i==RTL8368S_TBL_CMD_CHECK_COUNTER)
				return ERROR_PHY_ACCESSBUSY;
		}
		else
			break;
	}


	/*Get extern PHY RD data*/
	retVal = rtl8368s_getAsicReg(RTL8368S_PHY_EXT_RDDATA_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;
	
	*data = regData;
		
	return SUCCESS;	
}


/*=======================================================================
 * ASIC DRIVER API: Packet Scheduling Control Register 
 *========================================================================*/
/*
@func int32 | rtl8368s_setAsicQosEnable | Set Qos function enable usage configuration.
@parm uint32 | enabled | Qos function usage 1:enable, 0:disabled.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue FAILED | Invalid parameter.
@comm
	This API can set ASIC Qos functions' uage. It will impact on remarking, packet scheduling, priority assigment and flow control.
 */
int32 rtl8368s_setAsicQosEnable( uint32 enabled)
{
	int32 retVal;
	/* Set register value */
	retVal = rtl8368s_setAsicRegBit(RTL8368S_SWITCH_GLOBAL_CTRL_REG, RTL8368S_EN_QOS_OFF,enabled);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicQosEnable | Get Qos function enable usage configuration.
@parm uint32* | enabled | Qos function usage 1:enable, 0:disabled.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue FAILED | Invalid parameter.
@comm
	This API can get ASIC Qos enable configuration.
 */
int32 rtl8368s_getAsicQosEnable( uint32* enabled)
{
	uint32 regData;
	int32 retVal;

	retVal = rtl8368s_getAsicReg(RTL8368S_SWITCH_GLOBAL_CTRL_REG,&regData);
	if(retVal != SUCCESS)
		return retVal;

	if(regData & (1<<RTL8368S_EN_QOS_OFF))
		*enabled = 1;
	else
		*enabled = 0;

	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicLBParameter | Set Leaky Bucket Paramters.
@parm uint32 | token | Token is used for adding budget in each time slot.
@parm uint32 | tick | Tick is used for time slot size unit.
@parm uint32 | hiThreshold | Leaky bucket token high-threshold of PPR and WFQ.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue FAILED | Invalid parameter.
@comm
	The API can set leaky bucket parameters as token, tick and high-threshold of PPR and WFQ.
	Note: tick = desired value - 1.
	In 75MHz clock based, the suggesting values of (tick, token) are (19, 34), and the parameter
	'tick' need to be set 18.
 */
int32 rtl8368s_setAsicLBParameter( uint32 token, uint32 tick, uint32 hiThreshold )
{
	int32 retVal, regValue1, regValue2;

	/* Invalid input parameter */
	if ((token > (RTL8368S_SCH_TOKEN_MSK >> RTL8368S_SCH_TOKEN_OFFSET)) || 
		(tick > (RTL8368S_SCH_TICK_MSK >> RTL8368S_SCH_TICK_OFFSET)) )
		return ERRNO_INPUT;

	/* Get register value */
	retVal = rtl8368s_getAsicReg(RTL8368S_SCH_TGCR_REG, &regValue1);
	if (retVal !=  SUCCESS) 
		return retVal;

	retVal = rtl8368s_getAsicReg(RTL8368S_SCH_T2LBCR_REG, &regValue2);
	if (retVal !=  SUCCESS) 
		return retVal;

	/* Set register value */
	retVal = rtl8368s_setAsicReg(RTL8368S_SCH_TGCR_REG, (regValue1 & ~(RTL8368S_SCH_TOKEN_MSK | RTL8368S_SCH_TICK_MSK)) | (token << RTL8368S_SCH_TOKEN_OFFSET) | (tick << RTL8368S_SCH_TICK_OFFSET));
	if (retVal !=  SUCCESS) 
		return retVal;

	retVal = rtl8368s_setAsicReg(RTL8368S_SCH_T2LBCR_REG,hiThreshold);
	if (retVal !=  SUCCESS) 
		return retVal;

	return SUCCESS;
}

/*
@func int32 | rtl8368s_getAsicLBParameter | Get Leaky Bucket Paramters.
@parm uint32* | pToken | Pointer to return token.
@parm uint32* | pTick | Pointer to return tick.
@parm uint32* | pHiThreshold | Pointer to return hiThreshold.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@comm
	The API can get leaky bucket parameters as token, tick and high-threshold of PPR and WFQ.
	The real tick value = pTick + 1.
 */
int32 rtl8368s_getAsicLBParameter( uint32* pToken, uint32* pTick, uint32* pHiThreshold )
{
	int32 retVal, regValue1, regValue2;

	/* Get register value */
	retVal = rtl8368s_getAsicReg(RTL8368S_SCH_TGCR_REG, &regValue1);
	if (retVal !=  SUCCESS) 
		return retVal;
	retVal = rtl8368s_getAsicReg(RTL8368S_SCH_T2LBCR_REG, &regValue2);
	if (retVal !=  SUCCESS) 
		return retVal;

	if (pToken != NULL)
		*pToken = (regValue1 & RTL8368S_SCH_TOKEN_MSK) >> RTL8368S_SCH_TOKEN_OFFSET;
	if (pTick != NULL)
		*pTick = (regValue1 & RTL8368S_SCH_TICK_MSK) >> RTL8368S_SCH_TICK_OFFSET;
	if (pHiThreshold != NULL)
		*pHiThreshold = (regValue2 & RTL8368S_SCH_TYPE2LBTH_MSK) >> RTL8368S_SCH_TYPE2LBTH_OFFSET;
	
	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicQueueRate | Set per queue average packet rate(APR) and peak packet rate(PPR).
@parm enum PORTID | port | The port number.
@parm enum QUEUEID | queueid | The queue id wanted to set.
@parm uint32 | pprTime | PPR (in times of APR).  
@parm uint32 | aprBurstSize | Bucket Burst Size of APR (unit: 1KByte. 0 and 0x3F: reserved). 
@parm uint32 | apr | Average Packet Rate (unit: 64Kbps). 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@rvalue ERRNO_QUEUE_ID | Invalid queue id.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can set APR, PPR and burst size of APR for each queue.
	APR = (apr+1) * 64Kbps. 
	PPR = (2^pprTime)*APR for pprTime = 0~7.
	Bucket size of APR leaky bucket = aprBurstSize * 1KBytes.
	Note: The values 0 and 0x3F of aprBurstSize are reserved. 
 */
int32 rtl8368s_setAsicQueueRate( enum PORTID port, enum QUEUEID queueid, uint32 pprTime, uint32 aprBurstSize, uint32 apr )
{
	uint32 retVal;

	/* Invalid input parameter */
	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	if ((queueid < QUEUE0) || (queueid > QUEUE5))
		return ERRNO_QUEUE_ID;

	if ((pprTime > (RTL8368S_LBR_PPR_MSK >> RTL8368S_LBR_PPR_OFFSET)) || 
		(aprBurstSize > (RTL8368S_LBR_SIZE_MSK >> RTL8368S_LBR_SIZE_OFFSET)) || 
		(apr > (RTL8368S_LBR_APR_MSK >> RTL8368S_LBR_APR_OFFSET)))
		return ERRNO_INPUT;

	/* Set register value */
	retVal = rtl8368s_setAsicRegBits(RTL8368S_LBR_APR_REG(port, queueid),RTL8368S_LBR_APR_MSK,apr << RTL8368S_LBR_APR_OFFSET);
	if (retVal !=  SUCCESS) 
		return retVal;

	retVal = rtl8368s_setAsicRegBits(RTL8368S_LBR_PPR_REG(port, queueid),RTL8368S_LBR_SIZE_MSK | RTL8368S_LBR_PPR_MSK,(aprBurstSize << RTL8368S_LBR_SIZE_OFFSET) | (pprTime << RTL8368S_LBR_PPR_OFFSET));
	if (retVal !=  SUCCESS) 
		return retVal;

	return SUCCESS;
}

/*
@func int32 | rtl8368s_getAsicQueueRate | Get per queue APR and PPR parameters.
@parm enum PORTID | port | The port number.
@parm enum QUEUEID | queueid | The queue ID wanted to set.
@parm uint32* | pPprTime | Pointer to Peak Packet Rate (in times of APR). 
@parm uint32* | pAprBurstSize | Pointer to APR Burst Size (unit: 1KBytes). 
@parm uint32* | pApr | Pointer to Average Packet Rate (unit: 64Kbps).
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@rvalue ERRNO_QUEUE_ID | Invalid queue id.
@comm
	The API can get APR, PPR and burst size of APR of the specified queue. 
	APR = (pApr+1) * 64Kbps. 
	PPR = (2^pPprTime)*APR.
	Bucket size of APR leaky bucket = pAprBurstSize * 1KBytes.
 */
int32 rtl8368s_getAsicQueueRate( enum PORTID port, enum QUEUEID queueid, uint32* pPprTime, uint32* pAprBurstSize, uint32* pApr )
{
	uint32 retVal, regValue1, regValue2;

	/* Invalid input parameter */
	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	if ((queueid < QUEUE0) || (queueid > QUEUE5))
		return ERRNO_QUEUE_ID;
	
	/* Get register value */
	retVal = rtl8368s_getAsicReg(RTL8368S_LBR_APR_REG(port, queueid), &regValue1);
	if (retVal !=  SUCCESS) 
		return retVal;
	retVal = rtl8368s_getAsicReg(RTL8368S_LBR_PPR_REG(port, queueid), &regValue2);
	if (retVal !=  SUCCESS) 
		return retVal;

	if (pPprTime != NULL)
		*pPprTime = (regValue2 & RTL8368S_LBR_PPR_MSK) >> RTL8368S_LBR_PPR_OFFSET;
	if (pAprBurstSize != NULL)
		*pAprBurstSize = (regValue2 & RTL8368S_LBR_SIZE_MSK) >> RTL8368S_LBR_SIZE_OFFSET;
	if (pApr != NULL)
		*pApr = (regValue1 & RTL8368S_LBR_APR_MSK) >> RTL8368S_LBR_APR_OFFSET;
	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicDisableSchedulerAbility | Disable leaky buckets. 
@parm enum PORTID | port | The port number
@parm uint32 | aprDisable | Disable Average Packet Rate Leacky bucket. 0: Enable, 1: Disable. 
@parm uint32 | pprDisable | Disable Peak Packet Rate Leacky bucket. 0: Enable, 1: Disable. 
@parm uint32 | wfqDisable | Disable Weight Fair Queue(WFQ). 0: Enable, 1: Disable. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can be used to disable/enable APR, PPR and WFQ of a specified queue. 
 */
int32 rtl8368s_setAsicDisableSchedulerAbility( enum PORTID port, uint32 aprDisable, uint32 pprDisable, uint32 wfqDisable )
{
	uint32 retVal;
	uint16 regAddr;
	uint32 regData;
	uint32 regBits;
	uint32 schedulerData;
	/* Invalid input parameter */
	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;
	
	/* Invalid input parameter */
	if (((aprDisable != 0) && (aprDisable != 1)) || ((pprDisable != 0) && (pprDisable != 1)) ||
		((wfqDisable != 0) && (wfqDisable != 1)))
		return ERRNO_INPUT; 

	/* Adjust setting values to regValue2 */
	schedulerData = (wfqDisable << RTL8368S_SCR_WFQ_OFFSET) | (pprDisable << RTL8368S_SCR_PPR_OFFSET) | (aprDisable << RTL8368S_SCR_APR_OFFSET);

	if(port <= PORT4)
	{
		regAddr = RTL8368S_SCHEDULER_CONTROL_BASE;
		regBits = RTL8368S_SCR_SCHDIS_MSK<<(port*RTL8368S_SCR_SCHDIS_BITS);
		regData = (schedulerData<<(port*RTL8368S_SCR_SCHDIS_BITS)) & regBits;
	}	
	else
	{
		regAddr = RTL8368S_SCHEDULER_CONTROL_BASE + 1;
		regBits = RTL8368S_SCR_SCHDIS_MSK<<((port-PORT5)*RTL8368S_SCR_SCHDIS_BITS);
		regData = (schedulerData<<((port-PRI5)*RTL8368S_SCR_SCHDIS_BITS)) & regBits;
	}	

	
	retVal = rtl8368s_setAsicRegBits(regAddr,regBits,regData);		

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicDisableSchedulerAbility | Get leaky buckets ability.
@parm enum PORTID | port | The port number
@parm uint32* | pAprDisable | Average Packet Rate Leacky bucket. 0: Enable, 1: Disable. 
@parm uint32* | pPprDisable | Peak Packet Rate Leacky bucket. 0: Enable, 1: Disable. 
@parm uint32* | pWfqDisable | Weight Fair Queue(WFQ). 0: Enable, 1: Disable. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@comm
	The API can be used to get APR, PPR and WFQ ability of a specified. 
 */
int32 rtl8368s_getAsicDisableSchedulerAbility( enum PORTID port, uint32* pAprDisable, uint32* pPprDisable, uint32* pWfqDisable )
{
	uint32 retVal;
	uint16 regAddr;
	uint32 regData;
	uint32 regBits;
	uint32 bitsOff;
	uint32 schedulerData;

	/* Invalid input parameter */
	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	if(port <= PORT4)
	{
		regAddr = RTL8368S_SCHEDULER_CONTROL_BASE;
		regBits = RTL8368S_SCR_SCHDIS_MSK<<(port*RTL8368S_SCR_SCHDIS_BITS);
		bitsOff = port*3;
	}	
	else
	{
		regAddr = RTL8368S_SCHEDULER_CONTROL_BASE + 1;
		regBits = RTL8368S_SCR_SCHDIS_MSK<<((port-PORT5)*RTL8368S_SCR_SCHDIS_BITS);
		bitsOff = (port-PORT5)*RTL8368S_SCR_SCHDIS_BITS;
	}	

	retVal = rtl8368s_getAsicReg(regAddr,&regData);
	if(retVal != SUCCESS)
		return retVal;

	schedulerData =  (regData & regBits) >> bitsOff;


	if (pAprDisable != NULL)
		*pAprDisable = (schedulerData & RTL8368S_SCR_APR_MSK) >> RTL8368S_SCR_APR_OFFSET;
	if (pPprDisable != NULL)
		*pPprDisable = (schedulerData & RTL8368S_SCR_PPR_MSK) >> RTL8368S_SCR_PPR_OFFSET;
	if (pWfqDisable != NULL)
		*pWfqDisable = (schedulerData & RTL8368S_SCR_WFQ_MSK) >> RTL8368S_SCR_WFQ_OFFSET;


	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicPortIngressBandwidth | Set per-port total ingress bandwidth.
@parm enum PORTID | port | The port number.
@parm uint32 | bandwidth | The total ingress bandwidth (unit: 64Kbps), 0x3FFF:disable. 
@parm uint32 | preifg | Include preamble and IFG, 0:Exclude, 1:Include.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can set port ingress bandwidth. Port ingress bandwidth = (bandwidth+1)*64Kbps.
	To disable port ingress bandwidth control, the parameter 'bandwidth' should be set as 0x3FFF.
 */
int32 rtl8368s_setAsicPortIngressBandwidth( enum PORTID port, uint32 bandwidth, uint32 preifg)
{
	uint32 retVal, regValue;

	/* Invalid input parameter */
	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	if ((bandwidth > (RTL8368S_IB_BDTH_MSK >> RTL8368S_IB_BDTH_OFFSET)) || 
		(preifg > (RTL8368S_IB_PREIFP_MSK >> RTL8368S_IB_PREIFP_OFFSET)))
		return ERRNO_INPUT;

	/* Get register value */
	retVal = rtl8368s_getAsicReg(RTL8368S_IB_REG(port), &regValue);
	if (retVal !=  SUCCESS) 
		return retVal;

	/* Set register value */
	retVal = rtl8368s_setAsicReg(RTL8368S_IB_REG(port), (regValue & ~(RTL8368S_IB_BDTH_MSK| RTL8368S_IB_PREIFP_MSK)) | (bandwidth << RTL8368S_IB_BDTH_OFFSET) | (preifg << RTL8368S_IB_PREIFP_OFFSET));
	if (retVal !=  SUCCESS) 
		return retVal;

	return SUCCESS;
}

/*
@func int32 | rtl8368s_getAsicPortIngressBandwidth | Get per-port total ingress bandwidth.
@parm enum PORTID | port | The port number.
@parm uint32* | pBandwidth | Pointer to the returned total ingress bandwidth (unit: 64Kbps).
@parm uint32* | preifg | Include preamble and IFG, 0:Exclude, 1:Include.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@comm
	The API can get port ingress bandwidth. Port ingress bandwidth = (pBandwidth+1)*64Kbps. 
 */
int32 rtl8368s_getAsicPortIngressBandwidth( enum PORTID port, uint32* pBandwidth, uint32* pPreifg )
{
	uint32 retVal, regValue;

	/* Invalid input parameter */
	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	/* Get register value */
	retVal = rtl8368s_getAsicReg(RTL8368S_IB_REG(port), &regValue);
	if (retVal !=  SUCCESS) 
		return retVal;

	*pBandwidth = (regValue & RTL8368S_IB_BDTH_MSK) >> RTL8368S_IB_BDTH_OFFSET;
	*pPreifg = (regValue & RTL8368S_IB_PREIFP_MSK) >> RTL8368S_IB_PREIFP_OFFSET;

	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicPortEgressBandwidth | Set per-port total egress bandwidth.
@parm enum PORTID | port | The port number.
@parm uint32 | bandwidth | The total egress bandwidth (unit: 64kbps). 
@parm uint32 | preifg | Include/Exclude Preamble and IFG(20Bytes). 0:Exclude, 1:Include.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can set port egress bandwidth. Port egress bandwidth = (bandwidth+1)*64Kbps.
 */
int32 rtl8368s_setAsicPortEgressBandwidth( enum PORTID port, uint32 bandwidth, uint32 preifg )
{
	uint32 retVal, regValue;

	/* Invalid input parameter */
	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	if ((bandwidth > (RTL8368S_EB_BDTH_MSK >> RTL8368S_EB_BDTH_OFFSET)) || 
		(preifg > (RTL8368S_EBC_INC_IFG_MSK >> RTL8368S_EBC_INC_IFG_OFFSET)))
		return ERRNO_INPUT;
	
	/* Get register value */
	retVal = rtl8368s_getAsicReg(RTL8368S_EB_REG(port), &regValue);
	if (retVal !=  SUCCESS) 
		return retVal;

	retVal = rtl8368s_setAsicReg(RTL8368S_EB_REG(port), (regValue & ~(RTL8368S_EB_BDTH_MSK)) | (bandwidth << RTL8368S_EB_BDTH_OFFSET));
	if (retVal != SUCCESS)
		return retVal;

	/* Set register value */
	retVal = rtl8368s_getAsicReg(RTL8368S_EBC_INC_IFG_REG, &regValue);
	if (retVal !=  SUCCESS) 
		return retVal;

	retVal = rtl8368s_setAsicReg(RTL8368S_EBC_INC_IFG_REG, (regValue & ~(RTL8368S_EBC_INC_IFG_MSK)) | (preifg << RTL8368S_EBC_INC_IFG_OFFSET));
	if (retVal != SUCCESS)
		return retVal;

	return SUCCESS;
}

/*
@func int32 | rtl8368s_getAsicPortEgressBandwidth | Get per-port total egress bandwidth.
@parm enum PORTID | port | The port number.
@parm uint32* | pBandwidth | Pointer to the returned total egress bandwidth (unit: 64kbps).
@parm uint32* | preifg | Include/Exclude Preamble and IFG(20Bytes). 0:Exclude, 1:Include.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@comm
	The API can get port egress bandwidth. Port egress bandwidth = (pBandwidth+1)*64Kbps.
 */
int32 rtl8368s_getAsicPortEgressBandwidth( enum PORTID port, uint32* pBandwidth, uint32* preifg )
{
	uint32 retVal, regValue;

	/* Invalid input parameter */
	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	/* Get register value */
	retVal = rtl8368s_getAsicReg(RTL8368S_EB_REG(port), &regValue);
	if (retVal !=  SUCCESS) 
		return retVal;

	if (pBandwidth != NULL)
		*pBandwidth = (regValue & RTL8368S_EB_BDTH_MSK) >> RTL8368S_EB_BDTH_OFFSET;


	retVal = rtl8368s_getAsicReg(RTL8368S_EBC_INC_IFG_REG, &regValue);
	if (retVal !=  SUCCESS) 
		return retVal;

	if (preifg != NULL)
		*preifg = (regValue & RTL8368S_EBC_INC_IFG_MSK) >> RTL8368S_EBC_INC_IFG_OFFSET;
	
	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicQueueWeight | Set weight and type of a queue.
@parm enum PORTID | port | The port number.
@parm enum QUEUEID | queueid | The queue ID wanted to set.
@parm enum QUEUETYPE | queueType | The specified queue type. 0b0: Strict priority, 0b1: WFQ.
@parm uint32 | qweight | The weight value wanted to set (valid:0~127).
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@rvalue ERRNO_QUEUE_ID | Invalid queue id.
@rvalue ERRNO_QOS_INVALIDQUEUETYPE | Invalid queue type.
@rvalue ERRNO_QOS_INVALIDQUEUEWEIGHT | Invalid queue weight.
@comm
	The API can set weight and type, strict priority or weight fair queue (WFQ), of the specified 
	queue.  Parameter 'qweight' is only used in queueType = strict priority.  If queue type is 
	strict priority, the parameter 'qweight' can be ignored.  
	Weight value of WFQ = qweight+1.
 */
int32 rtl8368s_setAsicQueueWeight( enum PORTID port, enum QUEUEID queueid, enum QUEUETYPE queueType, uint32 qweight )
{
	uint32 retVal, regValue1;

	/* Invalid input parameter */
	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	if ((queueType < STR_PRIO) || (queueType > WFQ_PRIO))
		return ERRNO_QOS_INVALIDQUEUETYPE;

	if (qweight > (RTL8368S_WFQ_Q_WEIGHT_MSK >> RTL8368S_WFQ_Q_WEIGHT_OFFSET))
		return ERRNO_QOS_INVALIDQUEUEWEIGHT;

	/* Set queue type */
	retVal = rtl8368s_getAsicReg(RTL8368S_WFQ_EN_REG(port), &regValue1); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	retVal = rtl8368s_setAsicReg(RTL8368S_WFQ_EN_REG(port), (regValue1 & ~(RTL8368S_WFQ_EN_MSK(queueid,port))) | (queueType << RTL8368S_WFQ_EN_OFFSET(queueid,port))); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	/* Set queue weight */
	switch (queueid) {
		case QUEUE0:
			retVal = rtl8368s_getAsicReg(RTL8368S_WFQ_Q0_REG(port), &regValue1); 	
			if (retVal !=  SUCCESS) return retVal;			
			retVal = rtl8368s_setAsicReg(RTL8368S_WFQ_Q0_REG(port), (regValue1 & ~(RTL8368S_WFQ_Q0_WEIGHT_MSK)) | (qweight << RTL8368S_WFQ_Q0_WEIGHT_OFFSET)); 	
			break;
		case QUEUE1:
			retVal = rtl8368s_getAsicReg(RTL8368S_WFQ_Q1_REG(port), &regValue1); 	
			if (retVal !=  SUCCESS) return retVal;
			retVal = rtl8368s_setAsicReg(RTL8368S_WFQ_Q1_REG(port), (regValue1 & ~(RTL8368S_WFQ_Q1_WEIGHT_MSK)) | (qweight << RTL8368S_WFQ_Q1_WEIGHT_OFFSET)); 
			break;
		case QUEUE2:
			retVal = rtl8368s_getAsicReg(RTL8368S_WFQ_Q2_REG(port), &regValue1); 	
			if (retVal !=  SUCCESS) return retVal;
			retVal = rtl8368s_setAsicReg(RTL8368S_WFQ_Q2_REG(port), (regValue1 & ~(RTL8368S_WFQ_Q2_WEIGHT_MSK)) | (qweight << RTL8368S_WFQ_Q2_WEIGHT_OFFSET)); 	
			break;
		case QUEUE3:
			retVal = rtl8368s_getAsicReg(RTL8368S_WFQ_Q3_REG(port), &regValue1); 	
			if (retVal !=  SUCCESS) return retVal;			
			retVal = rtl8368s_setAsicReg(RTL8368S_WFQ_Q3_REG(port), (regValue1 & ~(RTL8368S_WFQ_Q3_WEIGHT_MSK)) | (qweight << RTL8368S_WFQ_Q3_WEIGHT_OFFSET)); 
			break;
		case QUEUE4:
			retVal = rtl8368s_getAsicReg(RTL8368S_WFQ_Q4_REG(port), &regValue1); 	
			if (retVal !=  SUCCESS) return retVal;
			retVal = rtl8368s_setAsicReg(RTL8368S_WFQ_Q4_REG(port), (regValue1 & ~(RTL8368S_WFQ_Q4_WEIGHT_MSK)) | (qweight << RTL8368S_WFQ_Q4_WEIGHT_OFFSET)); 	
			break;
		case QUEUE5:
			retVal = rtl8368s_getAsicReg(RTL8368S_WFQ_Q5_REG(port), &regValue1); 	
			if (retVal !=  SUCCESS) return retVal;
			retVal = rtl8368s_setAsicReg(RTL8368S_WFQ_Q5_REG(port), (regValue1 & ~(RTL8368S_WFQ_Q5_WEIGHT_MSK)) | (qweight << RTL8368S_WFQ_Q5_WEIGHT_OFFSET)); 	
			break;
		default: 
			return ERRNO_QUEUE_ID;
	}
	if (retVal !=  SUCCESS) 
		return retVal;
	return SUCCESS;
}

/*
@func int32 | rtl8368s_getAsicQueueWeight | Get weight and type of WFQ.
@parm enum PORTID | port | The port number.
@parm enum QUEUEID | queueid | The queue ID wanted to set.
@parm enum QUEUETYPE* | pQueueType | Pointer to the returned queue type.
@parm uint32* | pWeight | Pointer to the returned weight value.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@rvalue ERRNO_QUEUE_ID | Invalid queue id.
@comm
	The API can get weight and type of the specified queue. 
	The value of 'pWeight' can be ignored in queue type = strict priority. 
	Weight value of WFQ = pWeight+1.
 */
int32 rtl8368s_getAsicQueueWeight( enum PORTID port, enum QUEUEID queueid, enum QUEUETYPE *pQueueType, uint32 *pWeight )
{
	uint32 retVal, regValue1;

	/* Invalid input parameter */
	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	/* Get queue type */
	retVal = rtl8368s_getAsicReg(RTL8368S_WFQ_EN_REG(port), &regValue1); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	if (pQueueType != NULL)
		*pQueueType = (regValue1 & RTL8368S_WFQ_EN_MSK(queueid,port)) >> RTL8368S_WFQ_EN_OFFSET(queueid,port); 


	/* Get queue weight */
	switch (queueid) {
		case QUEUE0:
			retVal = rtl8368s_getAsicReg(RTL8368S_WFQ_Q0_REG(port), &regValue1); 	
			if (retVal !=  SUCCESS) return retVal;

			if (pWeight != NULL)
				*pWeight = (regValue1 & RTL8368S_WFQ_Q0_WEIGHT_MSK) >> RTL8368S_WFQ_Q0_WEIGHT_OFFSET;
			break;
		case QUEUE1:
			retVal = rtl8368s_getAsicReg(RTL8368S_WFQ_Q1_REG(port), &regValue1); 	
			if (retVal !=  SUCCESS) return retVal;			

			if (pWeight != NULL) 
				*pWeight = (regValue1 & RTL8368S_WFQ_Q1_WEIGHT_MSK) >> RTL8368S_WFQ_Q1_WEIGHT_OFFSET;
			break;
		case QUEUE2:
			retVal = rtl8368s_getAsicReg(RTL8368S_WFQ_Q2_REG(port), &regValue1); 	
			if (retVal !=  SUCCESS) return retVal;

			if (pWeight != NULL)
				*pWeight = (regValue1 & RTL8368S_WFQ_Q2_WEIGHT_MSK) >> RTL8368S_WFQ_Q2_WEIGHT_OFFSET;
			break;
		case QUEUE3:
			retVal = rtl8368s_getAsicReg(RTL8368S_WFQ_Q3_REG(port), &regValue1); 	
			if (retVal !=  SUCCESS) return retVal;			

			if (pWeight != NULL) 
				*pWeight = (regValue1 & RTL8368S_WFQ_Q3_WEIGHT_MSK) >> RTL8368S_WFQ_Q3_WEIGHT_OFFSET;
			break;
		case QUEUE4:
			retVal = rtl8368s_getAsicReg(RTL8368S_WFQ_Q4_REG(port), &regValue1); 	
			if (retVal !=  SUCCESS) return retVal;
			
			if (pWeight != NULL)
				*pWeight = (regValue1 & RTL8368S_WFQ_Q4_WEIGHT_MSK) >> RTL8368S_WFQ_Q4_WEIGHT_OFFSET;
			break;
		case QUEUE5:
			retVal = rtl8368s_getAsicReg(RTL8368S_WFQ_Q5_REG(port), &regValue1); 	
			if (retVal !=  SUCCESS) return retVal;
			
			if (pWeight != NULL)
				*pWeight = (regValue1 & RTL8368S_WFQ_Q5_WEIGHT_MSK) >> RTL8368S_WFQ_Q5_WEIGHT_OFFSET;
			break;
		default: 
			return ERRNO_QUEUE_ID;
	}
	if (retVal !=  SUCCESS) 
		return retVal;

	return SUCCESS;
}

/*=======================================================================
  * ASIC DRIVER API: Remarking Control Register 
 *========================================================================*/

/*
@func int32 | rtl8368s_setAsicDot1pRemarkingAbility | Set 802.1p remarking ability. 
@parm uint32 | enabledd | 1: enabled, 0: disabled.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can enable or disable 802.1p remarking ability for whole system. 
 */
int32 rtl8368s_setAsicDot1pRemarkingAbility(uint32 enabled)
{
	uint32 retVal, regValue;
	
	/* Invalid input parameter */
	if ((enabled != 0) && (enabled != 1))
		return ERRNO_INPUT; 

	/* Get register value*/
	retVal = rtl8368s_getAsicReg(RTL8368S_REM_REG, &regValue); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	/* Set register value */
	retVal = rtl8368s_setAsicReg(RTL8368S_REM_REG, (regValue & ~(RTL8368S_REM_1Q_MSK)) | (enabled << RTL8368S_REM_1Q_OFFSET)); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	return SUCCESS;
}

/*
@func int32 | rtl8368s_getAsicDot1pRemarkingAbility | Get 802.1p remarking ability. 
@parm uint32* | enabled | 1: enabled, 0: disabled.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@comm
	The API can get 802.1p remarking ability.
 */
int32 rtl8368s_getAsicDot1pRemarkingAbility(uint32* enabled)
{
	uint32 retVal, regValue;
	
	/* Get register value*/
	retVal = rtl8368s_getAsicReg(RTL8368S_REM_REG, &regValue); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	if (enabled != NULL)
		*enabled = (regValue & RTL8368S_REM_1Q_MSK) >> RTL8368S_REM_1Q_OFFSET;

	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicDot1pRemarkingParameter | Set 802.1p remarking parameter.
@parm enum PRIORITYVALUE | priority | Priority value.
@parm enum PRIORITYVALUE | newpriority | New priority value.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_QOS_INVALIDPRIORITY | Invalid priority. 
@comm
	The API can set 802.1p parameters source priority and new priority.
 */
int32 rtl8368s_setAsicDot1pRemarkingParameter( enum PRIORITYVALUE priority, enum PRIORITYVALUE newpriority )
{
	uint32 retVal, regValue;
	
	/* Invalid input parameter */
	if ((priority < PRI0) || (priority > PRI7) || (newpriority < PRI0) || (newpriority > PRI7))
		return ERRNO_QOS_INVALIDPRIORITY; 

	/* Get register value*/
	retVal = rtl8368s_getAsicReg(RTL8368S_D1Q_REG(priority), &regValue); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	/* Set register value*/
	retVal = rtl8368s_setAsicReg(RTL8368S_D1Q_REG(priority), (regValue & ~(RTL8368S_D1Q_MSK(priority))) | (newpriority << RTL8368S_D1Q_OFFSET(priority))); 	
	if (retVal !=  SUCCESS) 
		return retVal;
	
	return SUCCESS;
}

/*
@func int32 | rtl8368s_getAsicDot1pRemarkingParameter | Get 802.1p remarking parameter.
@parm enum PRIORITYVALUE | priority | Priority value.
@parm enum PRIORITYVALUE *| pNewpriority | It will return the new priority value of a specified priority.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_QOS_INVALIDPRIORITY | Invalid priority. 
@comm
	The API can get 802.1p remarking parameters. It would return new priority of inputed priority. 
 */
int32 rtl8368s_getAsicDot1pRemarkingParameter( enum PRIORITYVALUE priority, enum PRIORITYVALUE *pNewpriority )
{
	uint32 retVal, regValue;
	
	/* Invalid input parameter */
	if ((priority < PRI0) || (priority > PRI7))
		return ERRNO_QOS_INVALIDPRIORITY; 

	/* Get register value*/
	retVal = rtl8368s_getAsicReg(RTL8368S_D1Q_REG(priority), &regValue); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	if (pNewpriority != NULL)
		*pNewpriority = (regValue & RTL8368S_D1Q_MSK(priority)) >> RTL8368S_D1Q_OFFSET(priority);

	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicDscpRemarkingAbility | Set DSCP remarking ability.
@parm uint32* | enabled | 1: enabled, 0: disabled.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can enable or disable DSCP remarking ability for whole system. 
 */
int32 rtl8368s_setAsicDscpRemarkingAbility(uint32 enabled)
{
	uint32 retVal, regValue;
	
	/* Invalid input parameter */
	if ((enabled != 0) && (enabled != 1))
		return ERRNO_INPUT; 

	/* Get register value */
	retVal = rtl8368s_getAsicReg(RTL8368S_REM_REG, &regValue); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	/* Set register value */
	retVal = rtl8368s_setAsicReg(RTL8368S_REM_REG, (regValue & ~(RTL8368S_REM_DSCP_MSK)) | (enabled << RTL8368S_REM_DSCP_OFFSET)); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	return SUCCESS;
}

/*
@func int32 | rtl8368s_getAsicDscpRemarkingAbility | Get DSCP remarking ability.
@parm uint32* | enabled | 1: enabled, 0: disabled.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@comm
	The API can get DSCP remarking ability of whole system.
 */
int32 rtl8368s_getAsicDscpRemarkingAbility( uint32* enabled)
{
	uint32 retVal, regValue;
	
	/* Get register value */
	retVal = rtl8368s_getAsicReg(RTL8368S_REM_REG, &regValue); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	if (enabled != NULL)
		*enabled = (regValue & RTL8368S_REM_DSCP_MSK) >> RTL8368S_REM_DSCP_OFFSET;
	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicDscpRemarkingParameter | Set DSCP remarking parameter.
@parm enum PRIORITYVALUE | priority | Priority value.
@parm uint32 | newdscp | New DSCP value.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_QOS_INVALIDPRIORITY | Invalid priority. 
@rvalue ERRNO_QOS_INVALIDDSCP | Invalid DSCP value. 
@comm
	The API can set DSCP parameters source priority and new priority.
 */
int32 rtl8368s_setAsicDscpRemarkingParameter( enum PRIORITYVALUE priority, uint32 newdscp )
{
	uint32 retVal, regValue;
	
	/* Invalid input parameter */
	if ((priority < PRI0) || (priority > PRI7))
		return ERRNO_QOS_INVALIDPRIORITY; 

	if ((newdscp < 0) || (newdscp > 63))
		return ERRNO_QOS_INVALIDDSCP; 

	/* Get register value */
	retVal = rtl8368s_getAsicReg(RTL8368S_DSCP_REG(priority), &regValue); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	/* Set register value */
	retVal = rtl8368s_setAsicReg(RTL8368S_DSCP_REG(priority), (regValue & ~(RTL8368S_DSCP_MSK(priority))) | (newdscp << RTL8368S_DSCP_OFFSET(priority))); 	
	if (retVal !=  SUCCESS) 
		return retVal;
	
	return SUCCESS;
}

/*
@func int32 | rtl8368s_getAsicDscpRemarkingParameter | Get DSCP remarking parameter.
@parm enum PRIORITYVALUE | priority | Priority value.
@parm uint32* | pNewdscp |new DSCP value.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_QOS_INVALIDPRIORITY | Invalid priority.
@comm
	The API can get DSCP parameters. It would return new DSCP value of the specified priority.
 */
int32 rtl8368s_getAsicDscpRemarkingParameter( enum PRIORITYVALUE priority, uint32* pNewdscp )
{
	uint32 retVal, regValue;
	
	/* Invalid input parameter */
	if ((priority < PRI0) || (priority > PRI7))
		return ERRNO_QOS_INVALIDPRIORITY; 

	/* Get register value */
	retVal = rtl8368s_getAsicReg(RTL8368S_DSCP_REG(priority), &regValue); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	if (pNewdscp != NULL)
		*pNewdscp = (regValue & RTL8368S_DSCP_MSK(priority)) >> RTL8368S_DSCP_OFFSET(priority);

	return SUCCESS;
}

/*=======================================================================
  * ASIC DRIVER API: Priority Assignment Control Register 
 *========================================================================*/

/*
@func int32 | rtl8368s_setAsicPriorityDecision | Set priority decision table. 
@parm uint32 | portpri | Port-Based priority assignment.
@parm uint32 | dot1qpri | 1Q-Based priority assignment.
@parm uint32 | dscppri | DSCP-Based priority assignment. 
@parm uint32 | aclpri | ACL-Based priority assignment.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can set the priorities of Port-based, 1Q-based, DSCP-based and ACL-based Priority 
	Assignments	in output queue priority decision table. 
 */
int32 rtl8368s_setAsicPriorityDecision( uint32 portpri, uint32 dot1qpri, uint32 dscppri, uint32 aclpri)
{
	uint32 retVal;
	
	/* Invalid input parameter */
	if ((portpri < 0) || (portpri > 0xF) || (dot1qpri < 0) || (dot1qpri > 0xF) || 
		(dscppri < 0) || (dscppri > 0xF) || (aclpri < 0) || (aclpri > 0xF)) 
		return ERRNO_INPUT;

	/* Set register value */
	retVal = rtl8368s_setAsicReg(RTL8368S_PDCR_REG, (portpri << RTL8368S_PDCR_PBP_OFFSET) |
													(dot1qpri << RTL8368S_PDCR_1Q_OFFSET) |
													(dscppri << RTL8368S_PDCR_DSCP_OFFSET) |
													(aclpri << RTL8368S_PDCR_ACL_OFFSET)); 	
	if (retVal !=  SUCCESS) 
		return retVal;
	return SUCCESS;
}

/*
@func int32 | rtl8368s_getAsicPriorityDecision | Get priority decision table.
@parm uint32* | pPortpri | Por-Based priority assignment.
@parm uint32* | pDot1qpri | 1Q-Based priority assignment.
@parm uint32* | pDscppri | DSCP-Based priority assignment.
@parm uint32* | pAclpri | ACL-Based priority assignment.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@comm
	The API can get priorities of Port-based, 1Q-based, DSCP-based and ACL-based Priority 
	Assignments	in output queue priority decision table. 
 */
int32 rtl8368s_getAsicPriorityDecision( uint32* pPortpri, uint32* pDot1qpri, uint32* pDscppri, uint32* pAclpri)
{
	uint32 retVal, regValue;

	/* Get register value */
	retVal = rtl8368s_getAsicReg(RTL8368S_PDCR_REG, &regValue);
	if (retVal !=  SUCCESS) 
		return retVal;

	if (pPortpri != NULL)
		*pPortpri = (regValue & RTL8368S_PDCR_PBP_MSK) >> RTL8368S_PDCR_PBP_OFFSET;
	if (pDot1qpri != NULL)
		*pDot1qpri = (regValue & RTL8368S_PDCR_1Q_MSK) >> RTL8368S_PDCR_1Q_OFFSET;
	if (pDscppri != NULL)
		*pDscppri = (regValue & RTL8368S_PDCR_DSCP_MSK) >> RTL8368S_PDCR_DSCP_OFFSET;
	if (pAclpri != NULL)
		*pAclpri = (regValue & RTL8368S_PDCR_ACL_MSK) >> RTL8368S_PDCR_ACL_OFFSET;

	return SUCCESS;
}


/*
@func int32 | rtl8368s_setAsicPortPriority | Set port based priority.
@parm enum PORTID | port | The port number.
@parm enum PRIORITYVALUE | priority | Priority value.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number. 
@rvalue ERRNO_QOS_INVALIDPRIORITY | Invalid priority. 
@comm
	The API can set a 3-bit priority value of the specified port. 
 */
int32 rtl8368s_setAsicPortPriority( enum PORTID port, enum PRIORITYVALUE priority )
{
	uint32 retVal;
	uint32 regAddr;
	uint32 regData;
	uint32 regBits;

	/* Invalid input parameter */
	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM; 

	/* Invalid input parameter */
	if ((priority < PRI0) || (priority > PRI7))
		return ERRNO_PRIORITY; 


	if(port < PORT5)
	{
		regAddr = RTL8368S_PBP_BASE;

		regBits = RTL8368S_PBP_PRIORITY_MSK<<(RTL8368S_PBP_PRIORITY_BITS*port);

		regData = (priority << (RTL8368S_PBP_PRIORITY_BITS*port)) & regBits;
	}
	else
	{
		regAddr = RTL8368S_PBP_BASE + 1;
		regBits = RTL8368S_PBP_PRIORITY_MSK<<(RTL8368S_PBP_PRIORITY_BITS*(port-5));

		regData = (priority << (RTL8368S_PBP_PRIORITY_BITS*(port-5))) & regBits;
	}
		
	retVal = rtl8368s_setAsicRegBits(regAddr,regBits,regData);		

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicPortPriority | Get port based priority. 
@parm enum PORTID | port | The port number. 
@parm enum PRIORITYVALUE* | pPriority | Priority value.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number. 
@comm
	The API will return the priority value of the specified port. 
 */
int32 rtl8368s_getAsicPortPriority( enum PORTID port, enum PRIORITYVALUE *pPriority )
{

	uint32 retVal;
	uint16 regAddr;
	uint32 regData;
	uint32 regBits;
	uint32 bitsOff;

	/* Invalid input parameter */
	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	if(port < PORT5)
	{
		regAddr = RTL8368S_PBP_BASE;
		regBits = RTL8368S_PBP_PRIORITY_MSK<<(port*RTL8368S_PBP_PRIORITY_BITS);
		bitsOff = port*RTL8368S_PBP_PRIORITY_BITS;
	}	
	else
	{
		regAddr = RTL8368S_PBP_BASE + 1;
		regBits = RTL8368S_PBP_PRIORITY_MSK<<((port-PORT5)*RTL8368S_PBP_PRIORITY_BITS);
		bitsOff = (port-PORT5)*RTL8368S_PBP_PRIORITY_BITS;
	}	

	retVal = rtl8368s_getAsicReg(regAddr,&regData);
	if(retVal != SUCCESS)
		return retVal;

	*pPriority =  (regData & regBits) >> bitsOff;

	return SUCCESS;

}

/*
@func int32 | rtl8368s_setAsicDot1qAbsolutelyPriority | Set 802.1Q absolutely priority.
@parm enum PRIORITYVALUE | srcpriority | Priority value.
@parm enum PRIORITYVALUE | priority | Absolute priority value.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PRIORITY | Invalid priority.
@comm
	The API can set a 3-bit absolutely priority of the specified 802.1Q priority. 
 */
int32 rtl8368s_setAsicDot1qAbsolutelyPriority( enum PRIORITYVALUE srcpriority, enum PRIORITYVALUE priority )
{
	uint32 retVal, regValue;

	/* Invalid input parameter */
	if ((srcpriority < PRI0) || (srcpriority > PRI7) || (priority < PRI0) || (priority > PRI7)) 
		return ERRNO_PRIORITY;

	/* Get register value */
	retVal = rtl8368s_getAsicReg(RTL8368S_1QMCR_REG(srcpriority), &regValue); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	/* Set register value */
	retVal = rtl8368s_setAsicReg(RTL8368S_1QMCR_REG(srcpriority), (regValue & ~(RTL8368S_1QMCR_MSK(srcpriority))) | (priority << RTL8368S_1QMCR_OFFSET(srcpriority))); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	return SUCCESS;
}

/*
@func int32 | rtl8368s_getAsicDot1qAbsolutelyPriority | Get 802.1Q absolutely priority. 
@parm enum PRIORITYVALUE | srcpriority | Priority value. 
@parm enum PRIORITYVALUE* | pPriority | It will return the absolute priority value.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_QOS_INVALIDPRIORITY | Invalid priority. 
@comm
	The API will return the absolutely priority value of the specified 802.1Q priority. 
 */
int32 rtl8368s_getAsicDot1qAbsolutelyPriority( enum PRIORITYVALUE srcpriority, enum PRIORITYVALUE *pPriority )
{
	uint32 retVal, regValue;

	/* Invalid input parameter */
	if ((srcpriority < PRI0) || (srcpriority > PRI7)) 
		return ERRNO_PRIORITY;

	/* Get register value */
	retVal = rtl8368s_getAsicReg(RTL8368S_1QMCR_REG(srcpriority), &regValue); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	if (pPriority != NULL) 
		*pPriority = (regValue & RTL8368S_1QMCR_MSK(srcpriority)) >> RTL8368S_1QMCR_OFFSET(srcpriority);

	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicDscpPriority | Set DSCP-based priority.
@parm uint32 | dscp | DSCP value.
@parm enum PRIORITYVALUE | priority | Priority value.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_QOS_INVALIDDSCP | Invalid DSCP value. 
@rvalue ERRNO_QOS_INVALIDPRIORITY | Invalid priority. 
@comm
	The API can set a 3-bit priority of the specified DSCP value. 
 */
int32 rtl8368s_setAsicDscpPriority( uint32 dscp, enum PRIORITYVALUE priority )
{
	uint32 retVal, regValue1, regValue2, tPri1, tPri2;

	/* Invalid input parameter */
	if ((dscp < RTL8368S_DSCPMIN) || (dscp > RTL8368S_DSCPMAX)) 
		return ERRNO_QOS_INVALIDDSCP;

	if ((priority < PRI0) || (priority > PRI7)) 
		return ERRNO_QOS_INVALIDPRIORITY;

	/* Get and set register value */
	if ((0 <= (dscp % 16)) && ((dscp % 16) <= 4))
	{
		retVal = rtl8368s_getAsicReg(RTL8368S_DSCPPR_0_4_REG(dscp), &regValue1); 	
		if (retVal !=  SUCCESS) 
			return retVal;

		retVal = rtl8368s_setAsicReg(RTL8368S_DSCPPR_0_4_REG(dscp), (regValue1 & ~(RTL8368S_DSCPPR_0_4_MSK(dscp))) | (priority << RTL8368S_DSCPPR_0_4_OFFSET(dscp))); 	
		if (retVal !=  SUCCESS) 
			return retVal;
	}
	else if ((dscp % 16) == 5)
	{
		retVal = rtl8368s_getAsicReg(RTL8368S_DSCPPR_5_PRE_REG(dscp), &regValue1); 	
		if (retVal !=  SUCCESS) 
			return retVal;
		retVal = rtl8368s_getAsicReg(RTL8368S_DSCPPR_5_END_REG(dscp), &regValue2); 	
		if (retVal !=  SUCCESS) 
			return retVal;

		tPri1 = (priority & RTL8368S_DSCPPR_5_PRE_BIT_MSK) >> RTL8368S_DSCPPR_5_PRE_BIT_OFFSET;
		tPri2 = (priority & RTL8368S_DSCPPR_5_END_BIT_MSK) >> RTL8368S_DSCPPR_5_END_BIT_OFFSET;

		retVal = rtl8368s_setAsicReg(RTL8368S_DSCPPR_5_PRE_REG(dscp), (regValue1 & ~(RTL8368S_DSCPPR_5_PRE_MSK)) | (tPri1 << RTL8368S_DSCPPR_5_PRE_OFFSET)); 	
		if (retVal !=  SUCCESS) 
			return retVal;

		retVal = rtl8368s_setAsicReg(RTL8368S_DSCPPR_5_END_REG(dscp), (regValue2 & ~(RTL8368S_DSCPPR_5_END_MSK)) | (tPri2 << RTL8368S_DSCPPR_5_END_OFFSET)); 	
		if (retVal !=  SUCCESS) 
			return retVal;
	}
	else if ((6 <= (dscp % 16)) && ((dscp % 16) <= 9))
	{
		retVal = rtl8368s_getAsicReg(RTL8368S_DSCPPR_6_9_REG(dscp), &regValue1); 	
		if (retVal !=  SUCCESS) 
			return retVal;

		retVal = rtl8368s_setAsicReg(RTL8368S_DSCPPR_6_9_REG(dscp), (regValue1 & ~(RTL8368S_DSCPPR_6_9_MSK(dscp))) | (priority << RTL8368S_DSCPPR_6_9_OFFSET(dscp))); 	
		if (retVal !=  SUCCESS) 
			return retVal;
	}
	else if ((dscp % 16) == 10)
	{
		retVal = rtl8368s_getAsicReg(RTL8368S_DSCPPR_10_PRE_REG(dscp), &regValue1); 	
		if (retVal !=  SUCCESS) 
			return retVal;
		retVal = rtl8368s_getAsicReg(RTL8368S_DSCPPR_10_END_REG(dscp), &regValue2); 	
		if (retVal !=  SUCCESS) 
			return retVal;

		tPri1 = (priority & RTL8368S_DSCPPR_10_PRE_BIT_MSK) >> RTL8368S_DSCPPR_10_PRE_BIT_OFFSET;
		tPri2 = (priority & RTL8368S_DSCPPR_10_END_BIT_MSK) >> RTL8368S_DSCPPR_10_END_BIT_OFFSET;
		retVal = rtl8368s_setAsicReg(RTL8368S_DSCPPR_10_PRE_REG(dscp), (regValue1 & ~(RTL8368S_DSCPPR_10_PRE_MSK(dscp))) | (tPri1 << RTL8368S_DSCPPR_10_PRE_OFFSET(dscp))); 	
		if (retVal !=  SUCCESS) 
			return retVal;
		retVal = rtl8368s_setAsicReg(RTL8368S_DSCPPR_10_END_REG(dscp), (regValue2 & ~(RTL8368S_DSCPPR_10_END_MSK(dscp))) | (tPri2 << RTL8368S_DSCPPR_10_END_OFFSET(dscp))); 	
		if (retVal !=  SUCCESS) 
			return retVal;
	}
	else if ((11 <= (dscp % 16)) && ((dscp % 16) <= 15))
	{
		retVal = rtl8368s_getAsicReg(RTL8368S_DSCPPR_11_15_REG(dscp), &regValue1); 	
		if (retVal !=  SUCCESS) 
			return retVal;

		retVal = rtl8368s_setAsicReg(RTL8368S_DSCPPR_11_15_REG(dscp), (regValue1 & ~(RTL8368S_DSCPPR_11_15_MSK(dscp))) | (priority << RTL8368S_DSCPPR_11_15_OFFSET(dscp))); 	
		if (retVal !=  SUCCESS) 
			return retVal;
	}

	return SUCCESS;
}

/*
@func int32 | rtl8368s_getAsicDscpPriority | Get DSCP-based priority. 
@parm uint32 | dscp | DSCP value.
@parm enum PRIORITYVALUE* | pPriority | It will return the priority of the specified DSCP.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_QOS_INVALIDDSCP | Invalid DSCP value. 
@comm
	The API can get the priority of the specified DSCP value. 
 */
int32 rtl8368s_getAsicDscpPriority( uint32 dscp, enum PRIORITYVALUE *pPriority )
{
	uint32 retVal, regValue1, regValue2, tPri = 0, tPri1, tPri2;

	/* Invalid input parameter */
	if ((dscp < RTL8368S_DSCPMIN) || (dscp > RTL8368S_DSCPMAX)) 
		return ERRNO_QOS_INVALIDDSCP;

	/* Get register value */
	if ((0 <= (dscp % 16)) && ((dscp % 16) <= 4))
	{
		retVal = rtl8368s_getAsicReg(RTL8368S_DSCPPR_0_4_REG(dscp), &regValue1); 	
		if (retVal !=  SUCCESS) 
			return retVal;

		tPri = (regValue1 & RTL8368S_DSCPPR_0_4_MSK(dscp)) >> RTL8368S_DSCPPR_0_4_OFFSET(dscp);
	}
	else if ((dscp % 16) == 5)
	{
		retVal = rtl8368s_getAsicReg(RTL8368S_DSCPPR_5_PRE_REG(dscp), &regValue1); 	
		if (retVal !=  SUCCESS) 
			return retVal;
		retVal = rtl8368s_getAsicReg(RTL8368S_DSCPPR_5_END_REG(dscp), &regValue2); 	
		if (retVal !=  SUCCESS) 
			return retVal;

		tPri1 = (regValue1 & RTL8368S_DSCPPR_5_PRE_MSK) >> RTL8368S_DSCPPR_5_PRE_OFFSET;
		tPri2 = (regValue2 & RTL8368S_DSCPPR_5_END_MSK) >> RTL8368S_DSCPPR_5_END_OFFSET;
		tPri = (tPri1 << RTL8368S_DSCPPR_5_PRE_BIT_OFFSET) | (tPri2 << RTL8368S_DSCPPR_5_END_BIT_OFFSET);
	}
	else if ((6 <= (dscp % 16)) && ((dscp % 16) <= 9))
	{
		retVal = rtl8368s_getAsicReg(RTL8368S_DSCPPR_6_9_REG(dscp), &regValue1); 	
		if (retVal !=  SUCCESS) 
			return retVal;

		tPri = (regValue1 & RTL8368S_DSCPPR_6_9_MSK(dscp)) >> RTL8368S_DSCPPR_6_9_OFFSET(dscp);
	}
	else if ((dscp % 16) == 10)
	{
		retVal = rtl8368s_getAsicReg(RTL8368S_DSCPPR_10_PRE_REG(dscp), &regValue1); 	
		if (retVal !=  SUCCESS) 
			return retVal;
		retVal = rtl8368s_getAsicReg(RTL8368S_DSCPPR_10_END_REG(dscp), &regValue2); 	
		if (retVal !=  SUCCESS) 
			return retVal;

		tPri1 = (regValue1 & RTL8368S_DSCPPR_10_PRE_MSK(dscp)) >> RTL8368S_DSCPPR_10_PRE_OFFSET(dscp);
		tPri2 = (regValue2 & RTL8368S_DSCPPR_10_END_MSK(dscp)) >> RTL8368S_DSCPPR_10_END_OFFSET(dscp);
		tPri = (tPri1 << RTL8368S_DSCPPR_10_PRE_BIT_OFFSET) | (tPri2 << RTL8368S_DSCPPR_10_END_BIT_OFFSET);
	}
	else if ((11 <= (dscp % 16)) && ((dscp % 16) <= 15))
	{
		retVal = rtl8368s_getAsicReg(RTL8368S_DSCPPR_11_15_REG(dscp), &regValue1); 	
		if (retVal !=  SUCCESS) 
			return retVal;

		tPri = (regValue1 & RTL8368S_DSCPPR_11_15_MSK(dscp)) >> RTL8368S_DSCPPR_11_15_OFFSET(dscp);
	}

	if (pPriority != NULL) 
		*pPriority = tPri;	

	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicPriorityToQIDMappingTable | Set priority to QID mapping table parameters.
@parm enum QUEUENUM | qnum | The output queue number.
@parm enum PRIORITYVALUE | priority | The priority value. 
@parm enum QUEUEID | qid | Queue id.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_QUEUE_NUMBER | Invalid queue number. 
@rvalue ERRNO_QUEUE_ID | Invalid queue id. 
@rvalue ERRNO_QOS_INVALIDPRIORITY | Invalid priority.
@comm
	The API can configure priority to queue id mapping table in different queue number. 
 */
int32 rtl8368s_setAsicPriorityToQIDMappingTable( enum QUEUENUM qnum, enum PRIORITYVALUE priority, enum QUEUEID qid )
{
	uint32 retVal;
	uint32 regAddr, regData, regBits, offset;

	/* Invalid input parameter */
	if ((qnum < QNUM1) || (qnum > QNUM6)) 
		return ERRNO_QUEUE_NUMBER;

	if ((qid < QUEUE0) || (qid > QUEUE5)) 
		return ERRNO_QUEUE_ID;

	regAddr = ((qnum-1)*8+priority+1)/5 + (((qnum-1)*8+priority+1)%5==0?0:1)+ (RTL8368S_QNPQID_BASE-1);
	offset =  (((qnum-1)*8+priority)%5)*3;
	regBits = RTL8368S_QNPQID_MSK<<offset;
	regData = qid<<offset;

	retVal = rtl8368s_setAsicRegBits(regAddr,regBits,regData);
	if (retVal != SUCCESS)
		return FAILED;

	return SUCCESS;
}

/*
@func int32 | rtl8368s_getAsicPriorityToQIDMappingTable | Get priority to QID mapping table parameters. 
@parm enum QUEUENUM | qnum | The output queue number
@parm enum PRIORITYVALUE | priority | The priority value
@parm enum QUEUEID* | pQid | Queue id. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_QUEUE_NUMBER | Invalid queue number.
@rvalue ERRNO_QOS_INVALIDPRIORITY | Invalid priority.
@comm
	The API can return the mapping queue id of the specifed priority and queue number. 
 */
int32 rtl8368s_getAsicPriorityToQIDMappingTable( enum QUEUENUM qnum, enum PRIORITYVALUE priority, enum QUEUEID* pQid )
{
	uint32 retVal;
	uint32 regAddr, regData, offset;

	/* Invalid input parameter */
	if ((qnum < QNUM1) || (qnum > QNUM6))
		return ERRNO_QUEUE_NUMBER;
	if (pQid == NULL)
		return FAILED;

	regAddr = ((qnum-1)*8+priority+1)/5 + (((qnum-1)*8+priority+1)%5==0?0:1)+ (RTL8368S_QNPQID_BASE-1);
	offset =  (((qnum-1)*8+priority)%5)*3;

	retVal = rtl8368s_getAsicReg(regAddr,&regData);
	if (retVal  != SUCCESS)
		return FAILED;
	*pQid = (regData & (RTL8368S_QNPQID_MSK<<offset))>>offset;
	
	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicOutputQueueNumber | Set output queue number for each port.
@parm enum PORTID | port | The port number. 
@parm enum QUEUENUM | qnum | The output queue number.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@rvalue ERRNO_QUEUE_NUMBER | Invalid queue number.
@comm
	The API can set the output queue number of the specified port. 
 */
int32 rtl8368s_setAsicOutputQueueNumber( enum PORTID port, enum QUEUENUM qnum )
{
	uint32 retVal;
	uint32 regAddr;
	uint32 regData;
	uint32 regBits;

	/* Invalid input parameter */
	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM; 

	if ((qnum < QNUM1) || (qnum > QNUM6))
		return ERRNO_QUEUE_NUMBER; 

	if(port < PORT5)
	{
		regAddr = RTL8368S_OQN_BASE;

		regBits =RTL8368S_OQN_QNUM_MSK<<(RTL8368S_OQN_QNUM_BITS*port);

		regData = (qnum << (RTL8368S_OQN_QNUM_BITS*port)) & regBits;
	}
	else
	{
		regAddr = RTL8368S_OQN_BASE + 1;
		regBits = RTL8368S_OQN_QNUM_MSK<<(RTL8368S_OQN_QNUM_BITS*(port-5));

		regData = (qnum << (RTL8368S_OQN_QNUM_BITS*(port-5))) & regBits;
	}
		
	retVal = rtl8368s_setAsicRegBits(regAddr,regBits,regData);		

	return retVal;

}

/*
@func int32 | rtl8368s_getAsicOutputQueueNumber | Get output queue number.
@parm enum PORTID | port | The port number. 
@parm enum QUEUENUM | qnum | The output queue number. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@comm
	The API will return the output queue number of the specified port. 
 */
int32 rtl8368s_getAsicOutputQueueNumber( enum PORTID port, enum QUEUENUM *qnum )
{
	uint32 retVal;
	uint16 regAddr;
	uint32 regData;
	uint32 regBits;
	uint32 bitsOff;

	/* Invalid input parameter */
	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	if(port < PORT5)
	{
		regAddr = RTL8368S_OQN_BASE;
		regBits = RTL8368S_OQN_QNUM_MSK<<(port*RTL8368S_OQN_QNUM_BITS);
		bitsOff = port*RTL8368S_OQN_QNUM_BITS;
	}	
	else
	{
		regAddr = RTL8368S_OQN_BASE + 1;
		regBits = RTL8368S_OQN_QNUM_MSK<<((port-PORT5)*RTL8368S_OQN_QNUM_BITS);
		bitsOff = (port-PORT5)*RTL8368S_OQN_QNUM_BITS;
	}	

	retVal = rtl8368s_getAsicReg(regAddr,&regData);
	if(retVal != SUCCESS)
		return retVal;

	*qnum =  (regData & regBits) >> bitsOff;

	return SUCCESS;
}

/*=======================================================================
 * ASIC DRIVER API: FLOW CONTROL
 *========================================================================*/
/*
@func int32 | rtl8368s_setAsicFcSystemDrop | Set system-based drop parameters. 
@parm uint32 | drop | Whole system drop threshold. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can set drop threshold of whole system.
 */
int32 rtl8368s_setAsicFcSystemDrop(uint32 drop)
{
	uint32 retVal;
	/* Invalid input parameter */
	if ( drop > (RTL8368S_SBFC_RUNOUT_MSK >> RTL8368S_SBFC_RUNOUT_OFFSET) )
		return ERRNO_INPUT;

	retVal = rtl8368s_setAsicReg(RTL8368S_SBFC_RUNOUT_REG, drop); 	

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicFcSystemDrop | Set system-based drop parameters. 
@parm uint32* | drop | Whole system drop threshold. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can set drop threshold of whole system.
 */
int32 rtl8368s_getAsicFcSystemDrop(uint32* drop)
{
	uint32 regData;
	int32 retVal;
	
	retVal = rtl8368s_getAsicReg(RTL8368S_SBFC_RUNOUT_REG, &regData); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	
	*drop = regData & RTL8368S_SBFC_RUNOUT_MSK;

	return SUCCESS;

}

/*
@func int32 | rtl8368s_setAsicFcSystemSharedBufferBased | Set system-based shared buffer flow control parameters. 
@parm uint32 | sharedON | Share buffer flow control turn ON threshold. 
@parm uint32 | sharedOFF | Share buffer flow control turn OFF threshold. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can set system-based shared buffer flow control thresholds, turn ON threshold, turn OFF threshold and Drop threshold. 
	Shared buffer flow control turn ON threshold = fcON.
	Shared buffer flow control turn OFF threshold = fcOFF.	
	The maximum setting values of parameters (sharedON, sharedOFF) are (1023, 1023). 
	(unit: descriptor.  832-descriptor can be used for whole system.) 
 */
int32 rtl8368s_setAsicFcSystemSharedBufferBased(uint32 sharedON, uint32 sharedOFF)
{
	uint32 retVal, regValue;

	/* Invalid input parameter */
	if ((sharedON > (RTL8368S_SBFC_FCON_MSK >> RTL8368S_SBFC_FCON_OFFSET)) || 
		(sharedOFF > (RTL8368S_SBFC_FCOFF_MSK >> RTL8368S_SBFC_FCOFF_OFFSET)))
		return ERRNO_INPUT;

	/* shared buffer flow control turn ON */
	retVal = rtl8368s_getAsicReg(RTL8368S_SBFC_FCON_REG, &regValue); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	retVal = rtl8368s_setAsicReg(RTL8368S_SBFC_FCON_REG, (regValue & ~(RTL8368S_SBFC_FCON_MSK)) | (sharedON << RTL8368S_SBFC_FCON_OFFSET)); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	/* shared buffer flow control turn OFF */
	retVal = rtl8368s_getAsicReg(RTL8368S_SBFC_FCOFF_REG, &regValue); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	retVal = rtl8368s_setAsicReg(RTL8368S_SBFC_FCOFF_REG, (regValue & ~(RTL8368S_SBFC_FCOFF_MSK)) | (sharedOFF << RTL8368S_SBFC_FCOFF_OFFSET)); 	
	if (retVal !=  SUCCESS) 
		return retVal;


	return SUCCESS;
}

/*
@func int32 | rtl8368s_getAsicFcSystemSharedBufferBased | Get system-based shared buffer flow control parameters. 
@parm uint32* | sharedON | Share buffer flow control turn ON threshold. 
@parm uint32* | sharedOFF | Share buffer flow control turn OFF threshold. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@comm
	The API can get system-based shared buffer flow control thresholds, turn ON threshold and turn OFF threshold.
	Shared buffer flow control turn ON threshold = fcON.
	Shared buffer flow control turn OFF threshold = fcOFF.(unit: descriptor) 
 */
int32 rtl8368s_getAsicFcSystemSharedBufferBased(uint32 *sharedON, uint32 *sharedOFF)
{
	uint32 retVal, regValue;

	/* shared buffer flow control turn ON */
	retVal = rtl8368s_getAsicReg(RTL8368S_SBFC_FCON_REG, &regValue); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	if (sharedON != NULL) 
		*sharedON = (regValue & RTL8368S_SBFC_FCON_MSK) >> RTL8368S_SBFC_FCON_OFFSET;

	/* shared buffer flow control turn OFF */
	retVal = rtl8368s_getAsicReg(RTL8368S_SBFC_FCOFF_REG, &regValue); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	if (sharedOFF != NULL) 
		*sharedOFF = (regValue & RTL8368S_SBFC_FCOFF_MSK) >> RTL8368S_SBFC_FCOFF_OFFSET;

	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicFcPortBased | Set port-based flow control parameters. 
@parm enum PORTID | port | The port number. 
@parm uint32 | fcON | The flow control turn ON threshold. 
@parm uint32 | fcOFF | The flow control turn OFF threshold. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can set port-based flow control turn ON and turn OFF thresholds of the specified port.
	Port-based flow control turn ON threshold = fcON.
	Port-based flow control turn OFF threshold = fcOFF.	
	The maximum setting values of parameters (fcON, fcOFF) are (511, 511). 
	(unit: descriptor.  832-descriptor can be used for whole system.) 
 */
int32 rtl8368s_setAsicFcPortBased(enum PORTID port, uint32 fcON, uint32 fcOFF)
{
	uint32 retVal, regValue;
	
	/* Invalid input parameter */
	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	if ((fcON > (RTL8368S_PBFC_FCON_MSK >> RTL8368S_PBFC_FCON_OFFSET)) || 
		(fcOFF > (RTL8368S_PBFC_FCOFF_MSK >> RTL8368S_PBFC_FCOFF_OFFSET)))
		return ERRNO_INPUT;

	/* Port-based flow control turn ON */
	retVal = rtl8368s_getAsicReg(RTL8368S_PBFC_FCON_REG(port), &regValue); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	retVal = rtl8368s_setAsicReg(RTL8368S_PBFC_FCON_REG(port), (regValue & ~(RTL8368S_PBFC_FCON_MSK)) | (fcON << RTL8368S_PBFC_FCON_OFFSET)); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	/* Port-based flow control turn OFF */
	retVal = rtl8368s_getAsicReg(RTL8368S_PBFC_FCOFF_REG(port), &regValue); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	retVal = rtl8368s_setAsicReg(RTL8368S_PBFC_FCOFF_REG(port), (regValue & ~(RTL8368S_PBFC_FCOFF_MSK)) | (fcOFF << RTL8368S_PBFC_FCOFF_OFFSET)); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	return SUCCESS;
}

/*
@func int32 | rtl8368s_getAsicFcPortBased | Get port-based flow control parameters. 
@parm enum PORTID | port | The port number. 
@parm uint32* | fcON | The flow control turn ON threshold. 
@parm uint32* | fcOFF | The flow control turn OFF threshold. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@comm
	The API can get port-based flow control turn ON and turn OFF thresholds of the specified port. 
	Port-based flow control turn ON threshold = fcON.
	Port-based flow control turn OFF threshold = fcOFF. (unit: descriptor)
 */
int32 rtl8368s_getAsicFcPortBased(enum PORTID port, uint32 *fcON, uint32 *fcOFF)
{
	uint32 retVal, regValue;
	
	/* Invalid input parameter */
	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	/* Port-based flow control turn ON */
	retVal = rtl8368s_getAsicReg(RTL8368S_PBFC_FCON_REG(port), &regValue); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	if (fcON !=  SUCCESS) 
		*fcON = (regValue & RTL8368S_PBFC_FCON_MSK) >> RTL8368S_PBFC_FCON_OFFSET;

	/* Port-based flow control turn OFF */
	retVal = rtl8368s_getAsicReg(RTL8368S_PBFC_FCOFF_REG(port), &regValue); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	if (fcOFF !=  SUCCESS) 
		*fcOFF = (regValue & RTL8368S_PBFC_FCOFF_MSK) >> RTL8368S_PBFC_FCOFF_OFFSET;

	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicFcQueueDescriptorBased | Set queue-decriptor-based flow control parameters. 
@parm enum PORTID | port | The port number. 
@parm enum QUEUEID | queue | The queue id. 
@parm uint32 | fcON | The flow control turn ON threshold. 
@parm uint32 | fcOFF | The flow control turn OFF threshold. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@rvalue ERRNO_QUEUE_ID | Invalid queue id.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can set queue-decriptor-based flow control turn ON and turn OFF thresholds of the 
	specified queue. 
	Queue-descriptor-based flow control turn ON threshold = 2 * fcON.
	Queue-descriptor-based flow control turn OFF threshold = 2 * fcOFF.	
	The maximum setting values of parameters (fcON, fcOFF) are (127, 31). 
	(unit: descriptor.  832-descriptor can be used for whole system.) 
 */
int32 rtl8368s_setAsicFcQueueDescriptorBased(enum PORTID port, enum QUEUEID queue, uint32 fcON, uint32 fcOFF)
{
	uint32 retVal, regValue;
	
	/* Invalid input parameter */
	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	if ((fcON > (RTL8368S_QDBFC_FCON_MSK >> RTL8368S_QDBFC_FCON_OFFSET)) || 
		(fcOFF > (RTL8368S_QDBFC_FCOFF_MSK >> RTL8368S_QDBFC_FCOFF_OFFSET)))
		return ERRNO_INPUT;

	/* Get and set register value */
	switch (queue)
	{
		case QUEUE0:
			retVal = rtl8368s_getAsicReg(RTL8368S_QDBFC_QG0_REG(port), &regValue); 
			if (retVal !=  SUCCESS) 
				return retVal;

			retVal = rtl8368s_setAsicReg(RTL8368S_QDBFC_QG0_REG(port), (regValue & ~(RTL8368S_QDBFC_FCON_MSK | RTL8368S_QDBFC_FCOFF_MSK)) | (fcON << RTL8368S_QDBFC_FCON_OFFSET) | (fcOFF << RTL8368S_QDBFC_FCOFF_OFFSET)); 	
			if (retVal !=  SUCCESS) 
				return retVal;
			break;
		case QUEUE1:
		case QUEUE2:
		case QUEUE3:
		case QUEUE4:
			retVal = rtl8368s_getAsicReg(RTL8368S_QDBFC_QG1_REG(port), &regValue); 
			if (retVal !=  SUCCESS) 
				return retVal;

			retVal = rtl8368s_setAsicReg(RTL8368S_QDBFC_QG1_REG(port), (regValue & ~(RTL8368S_QDBFC_FCON_MSK | RTL8368S_QDBFC_FCOFF_MSK)) | (fcON << RTL8368S_QDBFC_FCON_OFFSET) | (fcOFF << RTL8368S_QDBFC_FCOFF_OFFSET)); 	
			if (retVal !=  SUCCESS) 
				return retVal;
			break;
		case QUEUE5:
			retVal = rtl8368s_getAsicReg(RTL8368S_QDBFC_QG2_REG(port), &regValue); 
			if (retVal !=  SUCCESS) 
				return retVal;

			retVal = rtl8368s_setAsicReg(RTL8368S_QDBFC_QG2_REG(port), (regValue & ~(RTL8368S_QDBFC_FCON_MSK | RTL8368S_QDBFC_FCOFF_MSK)) | (fcON << RTL8368S_QDBFC_FCON_OFFSET) | (fcOFF << RTL8368S_QDBFC_FCOFF_OFFSET)); 	
			if (retVal !=  SUCCESS) 
				return retVal;
			break;
		default:
			return ERRNO_QUEUE_ID;
	}

	return SUCCESS;
}

/*
@func int32 | rtl8368s_getAsicFcQueueDescriptorBased | Get queue-decriptor-based flow control parameters. 
@parm enum PORTID | port | The port number. 
@parm enum QUEUEID | queue | The queue id. 
@parm uint32* | fcON | The flow control turn ON threshold. 
@parm uint32* | fcOFF | The flow control turn OFF threshold. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@rvalue ERRNO_QUEUE_ID | Invalid queue id.
@comm
	The API can get queue-decriptor-based flow control turn ON and turn OFF thresholds of the 
	specified queue. 
	Queue-descriptor-based flow control turn ON threshold = 2 * fcON.
	Queue-descriptor-based flow control turn OFF threshold = 2 * fcOFF. (unit: descriptor) 	
 */
int32 rtl8368s_getAsicFcQueueDescriptorBased(enum PORTID port, enum QUEUEID queue, uint32 *fcON, uint32 *fcOFF)
{
	uint32 retVal, regValue;
	
	/* Invalid input parameter */
	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	/* Get register value */
	switch (queue)
	{
		case QUEUE0:
			retVal = rtl8368s_getAsicReg(RTL8368S_QDBFC_QG0_REG(port), &regValue); break;
		case QUEUE1:
		case QUEUE2:
		case QUEUE3:
		case QUEUE4:
			retVal = rtl8368s_getAsicReg(RTL8368S_QDBFC_QG1_REG(port), &regValue); break;
		case QUEUE5:
			retVal = rtl8368s_getAsicReg(RTL8368S_QDBFC_QG2_REG(port), &regValue); break;
		default:
			return ERRNO_QUEUE_ID;
	}
	if (retVal !=  SUCCESS) 
		return retVal;

	if (fcON != NULL) 
		*fcON = (regValue & RTL8368S_QDBFC_FCON_MSK) >> RTL8368S_QDBFC_FCON_OFFSET;
	if (fcOFF != NULL) 
		*fcOFF = (regValue & RTL8368S_QDBFC_FCOFF_MSK) >> RTL8368S_QDBFC_FCOFF_OFFSET;

	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicFcQueuePacketBased | Set queue-packet-based flow control parameters. 
@parm enum PORTID | port | The port number. 
@parm enum QUEUEID | queue | The queue id. 
@parm uint32 | fcON | The flow control turn ON threshold. 
@parm uint32 | fcOFF | The flow control turn OFF threshold. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@rvalue ERRNO_QUEUE_ID | Invalid queue id.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can set queue-packet-based flow control turn ON and turn OFF thresholds of the 
	specified queue. 
	Queue-packet-based flow control turn ON threshold = 4 * fcON.
	Queue-packet-based flow control turn OFF threshold = 4 * fcOFF.	
	The maximum setting values of parameters (fcON, fcOFF) are (63, 15).
	(unit: packet.  384-packet can be used for each port.) 
 */
int32 rtl8368s_setAsicFcQueuePacketBased(enum PORTID port, enum QUEUEID queue, uint32 fcON, uint32 fcOFF)
{
	uint32 retVal, regValue;
	
	/* Invalid input parameter */
	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	if ((fcON > (RTL8368S_QPBFC_FCON_MSK >> RTL8368S_QPBFC_FCON_OFFSET)) || 
		(fcOFF > (RTL8368S_QPBFC_FCOFF_MSK >> RTL8368S_QPBFC_FCOFF_OFFSET)))
		return ERRNO_INPUT;

	/* Get and set register value */
	switch (queue)
	{
		case QUEUE0:
			retVal = rtl8368s_getAsicReg(RTL8368S_QPBFC_QG0_REG(port), &regValue); 
			if (retVal !=  SUCCESS) 
				return retVal;

			retVal = rtl8368s_setAsicReg(RTL8368S_QPBFC_QG0_REG(port), (regValue & ~(RTL8368S_QPBFC_FCON_MSK | RTL8368S_QPBFC_FCOFF_MSK)) | (fcON << RTL8368S_QPBFC_FCON_OFFSET) | (fcOFF << RTL8368S_QPBFC_FCOFF_OFFSET)); 	
			if (retVal !=  SUCCESS) 
				return retVal;
			break;
		case QUEUE1:
		case QUEUE2:
		case QUEUE3:
		case QUEUE4:
			retVal = rtl8368s_getAsicReg(RTL8368S_QPBFC_QG1_REG(port), &regValue); 
			if (retVal !=  SUCCESS) 
				return retVal;

			retVal = rtl8368s_setAsicReg(RTL8368S_QPBFC_QG1_REG(port), (regValue & ~(RTL8368S_QPBFC_FCON_MSK | RTL8368S_QPBFC_FCOFF_MSK)) | (fcON << RTL8368S_QPBFC_FCON_OFFSET) | (fcOFF << RTL8368S_QPBFC_FCOFF_OFFSET)); 	
			if (retVal !=  SUCCESS) 
				return retVal;
			break;
		case QUEUE5:
			retVal = rtl8368s_getAsicReg(RTL8368S_QPBFC_QG2_REG(port), &regValue); 
			if (retVal !=  SUCCESS) 
				return retVal;

			retVal = rtl8368s_setAsicReg(RTL8368S_QPBFC_QG2_REG(port), (regValue & ~(RTL8368S_QPBFC_FCON_MSK | RTL8368S_QPBFC_FCOFF_MSK)) | (fcON << RTL8368S_QPBFC_FCON_OFFSET) | (fcOFF << RTL8368S_QPBFC_FCOFF_OFFSET)); 	
			if (retVal !=  SUCCESS) 
				return retVal;
			break;
		default:
			return ERRNO_QUEUE_ID;
	}

	return SUCCESS;
}

/*
@func int32 | rtl8368s_getAsicFcQueuePacketBased | Get queue-packet-based flow control parameters. 
@parm enum PORTID | port | The port number. 
@parm enum QUEUEID | queue | The queue id. 
@parm uint32* | fcON | The flow control turn ON threshold. 
@parm uint32* | fcOFF | The flow control turn OFF threshold. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue FAILED | Invalid parameter.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@rvalue ERRNO_QUEUE_ID | Invalid queue id.
@comm
	The API can get queue-packet-based flow control turn ON and turn OFF thresholds of the 
	specified queue. 
	Queue-packet-based flow control turn ON threshold = 4 * fcON.
	Queue-packet-based flow control turn OFF threshold = 4 * fcOFF.	
	(unit: packet) 
 */
int32 rtl8368s_getAsicFcQueuePacketBased(enum PORTID port, enum QUEUEID queue, uint32 *fcON, uint32 *fcOFF)
{
	uint32 retVal, regValue;
	
	/* Invalid input parameter */
	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	/* Get register value */
	switch (queue)
	{
		case QUEUE0:
			retVal = rtl8368s_getAsicReg(RTL8368S_QPBFC_QG0_REG(port), &regValue); break;
		case QUEUE1:
		case QUEUE2:
		case QUEUE3:
		case QUEUE4:
			retVal = rtl8368s_getAsicReg(RTL8368S_QPBFC_QG1_REG(port), &regValue); break;
		case QUEUE5:
			retVal = rtl8368s_getAsicReg(RTL8368S_QPBFC_QG2_REG(port), &regValue); break;
		default:
			return ERRNO_QUEUE_ID;
	}
	if (retVal !=  SUCCESS) 
		return retVal;

	if (fcON != NULL) 
		*fcON = (regValue & RTL8368S_QPBFC_FCON_MSK) >> RTL8368S_QPBFC_FCON_OFFSET;
	if (fcOFF != NULL) 
		*fcOFF = (regValue & RTL8368S_QPBFC_FCOFF_MSK) >> RTL8368S_QPBFC_FCOFF_OFFSET;

	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicFcPerQueuePhysicalLengthGap | Set physical size offset in queue-packet-based flow control mechanism.
@parm uint32 | gap | The offset. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can set per-queue physical size offset in queue-packet-based flow control mechanism. 
	(unit: packet)
 */
int32 rtl8368s_setAsicFcPerQueuePhysicalLengthGap(uint32 gap)
{
	uint32 retVal, regValue;

	/* Invalid input parameter */
	if (gap > (RTL8368S_PQSOCR_GAP_MSK >> RTL8368S_PQSOCR_GAP_OFFSET))
		return ERRNO_INPUT;

	/* Get register value */
	retVal = rtl8368s_getAsicReg(RTL8368S_PQSOCR_GAP_REG, &regValue); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	/* Set register value */
	retVal = rtl8368s_setAsicReg(RTL8368S_PQSOCR_GAP_REG, (regValue & ~(RTL8368S_PQSOCR_GAP_MSK)) | (gap << RTL8368S_PQSOCR_GAP_OFFSET)); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	return SUCCESS;
}

/*
@func int32 | rtl8368s_getAsicFcPerQueuePhysicalLengthGap | Get physical size offset in queue-packet-based flow control mechanism.
@parm uint32* | gap | It will return the offset value. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@comm
	The API can get per-queue physical size offset in queue-packet-based flow control mechanism. 
	(unit: packet)
 */
int32 rtl8368s_getAsicFcPerQueuePhysicalLengthGap(uint32 *gap)
{
	uint32 retVal, regValue;

	/* Get register value */
	retVal = rtl8368s_getAsicReg(RTL8368S_PQSOCR_GAP_REG, &regValue); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	if (gap != NULL) 
		*gap = (regValue & RTL8368S_PQSOCR_GAP_MSK) >> RTL8368S_PQSOCR_GAP_OFFSET;

	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicFcQueueFlowControlUsage | Set flow control ability for each queue. 
@parm enum PORTID | port | The port number. 
@parm enum QUEUEID | queue | The queue id. 
@parm uint32 | enabled | 1: enabled, 0: disabled.  
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@rvalue ERRNO_QUEUE_ID | Invalid queue id.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can enable or disable flow control ability of the specified output queue. 
 */
int32 rtl8368s_setAsicFcQueueFlowControlUsage(enum PORTID port, enum QUEUEID queue, uint32 enabled)
{
	uint32 retVal;

	/* Invalid input parameter */
	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	if ((queue < QUEUE0) || (queue > QUEUE5))
		return ERRNO_QUEUE_ID;

	if ((enabled != 0) && (enabled != 1))
		return ERRNO_INPUT; 


	retVal = rtl8368s_setAsicRegBit(RTL8368S_PBFC_ENQUE_REG(port),RTL8368S_PBFC_ENQUE_OFFSET(queue),enabled);
#if 0
	/* Get register value */
 	retVal = rtl8368s_getAsicReg(RTL8368S_PBFC_ENQUE_REG(port), &regValue); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	/* Set register value */
 	retVal = rtl8368s_setAsicReg(RTL8368S_PBFC_ENQUE_REG(port), (regValue & ~(RTL8368S_PBFC_ENQUE_MSK(queue))) | (enabled << RTL8368S_PBFC_ENQUE_OFFSET(queue))); 	
#endif
	if (retVal !=  SUCCESS) 
		return retVal;

	return SUCCESS;
}

/*
@func int32 | rtl8368s_getAsicFcQueueFlowControlUsage | Get flow control ability for each queue. 
@parm enum PORTID | port | The port number. 
@parm enum QUEUEID | queue | The queue id. 
@parm uint32* | enabled | 1: enabled, 0: disabled.  
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@rvalue ERRNO_QUEUE_ID | Invalid queue id.
@comm
	The API can get flow control ability of the specified output queue. 
 */
int32 rtl8368s_getAsicFcQueueFlowControlUsage(enum PORTID port, enum QUEUEID queue, uint32 *enabled)
{
	uint32 retVal, regValue;

	/* Invalid input parameter */
	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	if ((queue < QUEUE0) || (queue > QUEUE5))
		return ERRNO_QUEUE_ID;

	/* Get register value */
 	retVal = rtl8368s_getAsicReg(RTL8368S_PBFC_ENQUE_REG(port), &regValue); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	if (enabled != NULL) 
		*enabled = (regValue & RTL8368S_PBFC_ENQUE_MSK(queue)) >> RTL8368S_PBFC_ENQUE_OFFSET(queue);

	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicFcPacketUsedPagesBased | Set packet used pages flow control parameters.
@parm uint32 | fcON | The flow control turn ON threshold.  
@parm uint32 | enabled | 1: enabled, 0: disabled.  
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can enable or disable packet used page flow control ability and set the flow control 
	turn ON threshold.
	Packet used page flow control turn ON threshold = fcON.
	The maximum setting values of parameter fcON is 511.
	(unit: descriptor) 
 */
int32 rtl8368s_setAsicFcPacketUsedPagesBased(uint32 fcON, uint32 enabled)
{
	uint32 retVal, regValue;

	/* Invalid input parameter */
	if (fcON > (RTL8368S_PUPFCR_FCON_MSK >> RTL8368S_PUPFCR_FCON_OFFSET))
		return ERRNO_INPUT;

	if ((enabled != 0) && (enabled != 1))
		return ERRNO_INPUT; 


	/* Get register value */
	retVal = rtl8368s_getAsicReg(RTL8368S_PUPFCR_REG, &regValue); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	/* Set register value */
	retVal = rtl8368s_setAsicReg(RTL8368S_PUPFCR_REG, (regValue & ~(RTL8368S_PUPFCR_FCON_MSK | RTL8368S_PUPFCR_EN_MSK)) | (fcON << RTL8368S_PUPFCR_FCON_OFFSET) | (enabled << RTL8368S_PUPFCR_EN_OFFSET)); 	

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicFcPacketUsedPagesBased | Get packet used pages flow control parameters.
@parm uint32* | fcON | The flow control turn ON threshold.  
@parm uint32* | enabled | 1: enabled, 0: disabled.  
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@comm
	The API can get packet used page flow control ability and the flow control turn ON threshold.
	Packet used page flow control turn ON threshold = fcON.
	(unit: descriptor) 
 */
int32 rtl8368s_getAsicFcPacketUsedPagesBased(uint32 *fcON, uint32 *enabled)
{
	uint32 retVal, regValue;

	/* Get register value */
	retVal = rtl8368s_getAsicReg(RTL8368S_PUPFCR_REG, &regValue); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	if (fcON != NULL) 
		*fcON = (regValue & RTL8368S_PUPFCR_FCON_MSK) >> RTL8368S_PUPFCR_FCON_OFFSET;
	if (enabled != NULL) 
		*enabled = (regValue & RTL8368S_PUPFCR_EN_MSK) >> RTL8368S_PUPFCR_EN_OFFSET;

	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicFcSystemBased | Set system-based flow control parameters. 
@parm uint32 | systemON | Share buffer flow control turn ON threshold. 
@parm uint32 | systemOFF | Share buffer flow control turn OFF threshold. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can set system-based flow control thresholds, turn ON threshold and turn OFF threshold.
	The maximum setting values of parameters (sharedON, sharedOFF, drop) are (1023, 1023, 1023). 
	(unit: descriptor.  1000-descriptor can be used for whole system.) 
 */
int32 rtl8368s_setAsicFcSystemBased(uint32 systemON, uint32 systemOFF)
{
	uint32 retVal, regValue;

	/* Invalid input parameter */
	if ((systemON > (RTL8368S_SYSBFC_FCON_MSK >> RTL8368S_SYSBFC_FCON_OFFSET)) || 
		(systemOFF > (RTL8368S_SYSBFC_FCOFF_MSK >> RTL8368S_SYSBFC_FCOFF_OFFSET)))
		return ERRNO_INPUT;

	/* system flow control turn ON */
	retVal = rtl8368s_getAsicReg(RTL8368S_SYSBFC_FCON_REG, &regValue); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	retVal = rtl8368s_setAsicReg(RTL8368S_SYSBFC_FCON_REG, (regValue & ~(RTL8368S_SYSBFC_FCON_MSK)) | (systemON << RTL8368S_SYSBFC_FCON_OFFSET)); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	/* system flow control turn OFF */
	retVal = rtl8368s_getAsicReg(RTL8368S_SYSBFC_FCOFF_REG, &regValue); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	retVal = rtl8368s_setAsicReg(RTL8368S_SYSBFC_FCOFF_REG, (regValue & ~(RTL8368S_SYSBFC_FCOFF_MSK)) | (systemOFF << RTL8368S_SYSBFC_FCOFF_OFFSET)); 	
	if (retVal !=  SUCCESS) 
		return retVal;


	return SUCCESS;
}


/*
@func int32 | rtl8368s_getAsicFcSystemBased | Set system-based flow control parameters. 
@parm uint32* | systemON | Share buffer flow control turn ON threshold. 
@parm uint32* | systemOFF | Share buffer flow control turn OFF threshold. 
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid input parameter.
@comm
	The API can get system-based flow control thresholds, turn ON threshold and turn OFF threshold.
	The maximum setting values of parameters (sharedON, sharedOFF, drop) are (1023, 1023, 1023). 
	(unit: descriptor.  1000-descriptor can be used for whole system.) 
 */
int32 rtl8368s_getAsicFcSystemBased(uint32 *systemON, uint32 *systemOFF)
{
	uint32 retVal, regValue;

	/* shared buffer flow control turn ON */
	retVal = rtl8368s_getAsicReg(RTL8368S_SYSBFC_FCON_REG, &regValue); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	if (systemON != NULL) 
		*systemON = (regValue & RTL8368S_SYSBFC_FCON_MSK) >> RTL8368S_SYSBFC_FCON_OFFSET;

	/* shared buffer flow control turn OFF */
	retVal = rtl8368s_getAsicReg(RTL8368S_SYSBFC_FCOFF_REG, &regValue); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	if (systemOFF != NULL) 
		*systemOFF = (regValue & RTL8368S_SYSBFC_FCOFF_MSK) >> RTL8368S_SYSBFC_FCOFF_OFFSET;

	return SUCCESS;
}



/*
@func int32 | rtl8368s_setAsicLedIndicateInfoConfig | Set Leds indicated information mode
@parm uint32 | ledNo | LED group number. There are 1 to 1 led mapping to each port in each led group. 
@parm enum RTL8368S_LEDCONF | config | Support 16 types configuration.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
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
int32 rtl8368s_setAsicLedIndicateInfoConfig(uint32 ledNo, enum RTL8368S_LEDCONF config)
{
	uint32 retVal, regValue;

	if(ledNo >=RTL8368S_LED_GROUP_MAX)
		return ERRNO_INPUT;

	if(config > LEDCONF_LEDFORCE)	
		return ERRNO_INPUT;


	/* Get register value */
	retVal = rtl8368s_getAsicReg(RTL8368S_LED_INDICATED_CONF_REG, &regValue); 	
	if (retVal !=  SUCCESS) 
		return retVal;

	regValue =  (regValue & (~(0xF<<(ledNo*4)))) | (config<<(ledNo*4));

	
	retVal = rtl8368s_setAsicReg(RTL8368S_LED_INDICATED_CONF_REG, regValue); 	

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicLedIndicateInfoConfig | Get Leds indicated information mode
@parm uint32 | ledNo | LED group number. There are 1 to 1 led mapping to each port in each led group. 
@parm enum RTL8368S_LEDCONF* | config | Support 16 types configuration.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@comm
	The API can get LED indicated information configuration for each LED group.
 */
int32 rtl8368s_getAsicLedIndicateInfoConfig(uint32 ledNo, enum RTL8368S_LEDCONF* config)
{
	uint32 retVal, regValue;

	if(ledNo >=RTL8368S_LED_GROUP_MAX)
		return ERRNO_INPUT;

	/* Get register value */
	retVal = rtl8368s_getAsicReg(RTL8368S_LED_INDICATED_CONF_REG, &regValue); 	
	if (retVal !=  SUCCESS) 
		return retVal;
	
	*config = (regValue >> (ledNo*4)) & 0x000F;
		
	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicForceLeds | Turn on/off Led of dedicated port
@parm uint32 | ledG0Msk | Turn on or turn off Leds of group 0, 1:on 0:off.
@parm uint32 | ledG1Msk | Turn on or turn off Leds of group 1, 1:on 0:off.
@parm uint32 | ledG2Msk | Turn on or turn off Leds of group 2, 1:on 0:off.
@parm uint32 | ledG3Msk | Turn on or turn off Leds of group 3, 1:on 0:off.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@comm
	The API can turn on/off desired Led group of dedicated port while indicated information configuration of LED group is set to force mode.
 */
int32 rtl8368s_setAsicForceLeds(uint32 ledG0Msk, uint32 ledG1Msk, uint32 ledG2Msk, uint32 ledG3Msk)
{
	uint32 retVal, regValue;

	regValue = (ledG0Msk & RTL8368S_LED_0_FORCE_MASK) | ((ledG1Msk<<RTL8368S_LED_1_FORCE_OFF)&RTL8368S_LED_1_FORCE_MASK);

	retVal = rtl8368s_setAsicReg(RTL8368S_LED_0_1_FORCE_REG, regValue); 	
	if(retVal != SUCCESS)
		return retVal;

	regValue = (ledG2Msk & RTL8368S_LED_2_FORCE_MASK) | ((ledG3Msk<<RTL8368S_LED_3_FORCE_OFF)&RTL8368S_LED_3_FORCE_MASK);
	retVal = rtl8368s_setAsicReg(RTL8368S_LED_2_3_FORCE_REG, regValue); 	
	
	return retVal;
}

/*
@func int32 | rtl8368s_setAsicLedBlinkRate | Set led blinking rate ate mode 0 to mode 3
@parm enum RTL8368S_LEDBLINKRATE | blinkRate | Support 6 blink rates.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@comm
	The API can set LED blink rate at 43ms, 84ms, 120ms, 170ms, 340ms and 670ms.
 */
int32 rtl8368s_setAsicLedBlinkRate(enum RTL8368S_LEDBLINKRATE blinkRate)
{
	uint32 retVal;
	uint32 regData;

	if(blinkRate >=LEDBLINKRATE_MAX)
		return ERRNO_INPUT;

	regData = blinkRate << RTL8368S_LED_BLINKRATE_OFF;

	retVal = rtl8368s_setAsicRegBits(RTL8368S_LED_BLINK_REG,RTL8368S_LED_BLINKRATE_MSK,regData);

	return retVal;

}

/*
@func int32 | rtl8368s_getAsicLedBlinkRate | Get led blinking rate ate mode 0 to mode 3
@parm enum RTL8368S_LEDBLINKRATE* | blinkRate | Support 6 blink rates.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@comm
	The API can set LED blink rate at 43ms, 84ms, 120ms, 170ms, 340ms and 670ms.
 */
int32 rtl8368s_getAsicLedBlinkRate(enum RTL8368S_LEDBLINKRATE* blinkRate)
{
	uint32 retVal;
	uint32 regData;


	retVal = rtl8368s_getAsicReg(RTL8368S_LED_BLINK_REG,&regData);
	if(retVal !=  SUCCESS)
		return retVal;

	*blinkRate = (regData&RTL8368S_LED_BLINKRATE_MSK)>>RTL8368S_LED_BLINKRATE_OFF;

	
	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAiscPort4RGMIIControl | Set LED/Interrupt Control for P4 RGMII mode
@parm uint32 | enabled | 1:Enable LED/Interrupt for P4 RGMII, 0:Disable LED/Interrupt for P4 RGMII
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@comm
	Set LED/Interrupt Control for P4 RGMII mode
 */
int32 rtl8368s_setAiscPort4RGMIIControl(uint32 enabled)
{
    if((enabled != 0) && (enabled != 1))
        return FAILED;

    /* P4 RGMII LED Configuration */
    if(rtl8368s_setAsicRegBits(RTL8368S_INTERRUPT_CONTROL_REG, 
                               RTL8368S_INTERRUPT_P4_RGMII_LED_MSK, 
                               (enabled << RTL8368S_INTERRUPT_P4_RGMII_LED_OFF)) != SUCCESS)
        return FAILED;

    /* P4 RGMII Interrupt Configuration */
    if(rtl8368s_setAsicRegBits(RTL8368S_INTERRUPT_MASK_REG, 
                               RTL8368S_INTERRUPT_P4_FIBER_INT_MSK, 
                               (enabled << RTL8368S_INTERRUPT_P4_FIBER_INT_OFF) ) != SUCCESS)
        return FAILED;

    if(rtl8368s_setAsicRegBits(RTL8368S_INTERRUPT_MASK_REG, 
                               RTL8368S_INTERRUPT_P4_UTP_INT_MSK, 
                               (enabled << RTL8368S_INTERRUPT_P4_UTP_INT_OFF) ) != SUCCESS)
        return FAILED;

    return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicMacForceLink | Set mac force linking configuration.
@parm enum PORTID | port | Port/MAC number (0~5).
@parm enum MACLINKMODE | force | Mac link mode 1:force mode 0:normal
@parm enum PORTLINKSPEED | speed | Speed of the port 0b00-10M, 0b01-100M,0b10-1000M, 0b11 reserved.
@parm enum PORTLINKDUPLEXMODE | duplex | Deuplex mode 0b0-half duplex 0b1-full duplex.
@parm uint32 | link | Link status 0b0-link down b1-link up.
@parm uint32 | txPause | Pause frame transmit ability of the port 0b0-inactive 0b1-active.
@parm uint32 | rxPause | Pause frame response ability of the port 0b0-inactive 0b1-active.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue FAILED | Invalid parameter.
@comm
  	This API can set Port/MAC force mode properties. 
 */
int32 rtl8368s_setAsicMacForceLink(enum PORTID port,enum MACLINKMODE force,enum PORTLINKSPEED speed,enum PORTLINKDUPLEXMODE duplex,uint32 link,uint32 txPause,uint32 rxPause)
{
	uint32 retVal;
	uint32 macData;
	uint32 regBits;
	uint32 regAddr;
	uint32 regData;
	
	/* Invalid input parameter */
	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	/*not force mode*/
	if(MAC_NORMAL == force)
	{
		retVal = rtl8368s_getAsicReg(RTL8368S_MAC_FORCE_CTRL_REG,&regData);
		if (retVal !=  SUCCESS) 
			return retVal;
		
		regData = regData & (~(1<<port));

		retVal = rtl8368s_setAsicReg(RTL8368S_MAC_FORCE_CTRL_REG,regData);
		if (retVal !=  SUCCESS) 
			return retVal;

		return SUCCESS;
	}


	if(speed > SPD_1000M)
		return ERROR_MAC_INVALIDSPEED;

	/*prepare force status first*/
	macData = speed;

	if(duplex)
	{
		macData = macData | (duplex<<2);
	}

	if(link)
	{
		macData = macData | (link<<4);
	}

	if(txPause)
	{
		macData = macData | (txPause<<5);
	}
	
	if(rxPause)
	{
		macData = macData | (rxPause<<6);
	}
	
	regBits = 0xFF << (8*(port&0x01));
	macData = macData <<(8*(port&0x01));
	
	/* Set register value */
	regAddr = RTL8368S_PORT_ABILITY_BASE + (port>>1);


	retVal= rtl8368s_setAsicRegBits(regAddr,regBits,macData);
	if (retVal !=  SUCCESS) 
		return retVal;


	/* Set register value */
	retVal = rtl8368s_getAsicReg(RTL8368S_MAC_FORCE_CTRL_REG,&regData);
	if (retVal !=  SUCCESS) 
		return retVal;

	regData = regData | (1<<port);

	retVal = rtl8368s_setAsicReg(RTL8368S_MAC_FORCE_CTRL_REG,regData);
	if (retVal !=  SUCCESS) 
		return retVal;

	return SUCCESS;
}


/*
@func int32 | rtl8368s_setAsicCableTesting | Set port cable testing.
@parm enum PORTID | port | Physical port number (0~4).
@parm uint32 | enabled | Per-Port cable testing function 1: enabled, 0: disabled
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid port number.
@comm
	The API can start realtek cable testing function.
 */
int32 rtl8368s_setAsicCableTesting( enum PORTID port, uint32 enabled)
{

	int32 retVal;

	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;
	
	retVal = rtl8368s_setAsicRegBit(RTL8368S_CABLE_TESTING_CTRL_REG, (RTL8368S_CABLE_TESTING_START_OFF + port), enabled);

	return retVal;
}


/*
@func int32 | rtl8368s_getAsicRtctChannelStatus | Get channel cable testing result of each port.
@parm enum PORTID | port | Physical port number (0~4).
@parm enum CHANNELID | channel | Channel id of each port (0~3).
@parm uint32 | *impmis | Channel impedance mismatch status 1: impedance mismatch, 0: normal
@parm uint32 | *open | Channel open status 1: open, 0: normal
@parm uint32 | *short | Channel short status  1: short, 0: normal
@parm uint32 | *length | Cable length of channel (unit: 2.5cm).
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid port number.
@comm
	The API can Get realtek cable testing function result.
 */
int32 rtl8368s_getAsicRtctChannelStatus(enum PORTID port, enum CHANNELID channel,uint32* impmis,uint32* open_state,uint32* short_state,uint32* length)
{
	int32 retVal;
	uint32 regData;
	
	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;
	if(channel >=CH_MAX)
		return ERRNO_INPUT;

    /* Status checking, if RTCT function is not finished, return FAILED. */
    retVal = rtl8368s_getAsicReg(RTL8368S_CABLE_TESTING_ACT_REG, &regData);
	if(retVal !=  SUCCESS)
		return retVal;

    if(regData & (0x1 << (RTL8368S_CABLE_TESTING_ACT_OFF + port)))
        return FAILED;

    /* RTCT function finished, Start to get information */
    retVal = rtl8368s_getAsicReg((RTL8368S_CABLE_TESTING_INFO_BASE + (port * 5)), &regData);
	if(retVal !=  SUCCESS)
		return retVal;

    /* Mismatch */
    if(regData & RTL8368S_CABLE_TESTING_CH_MIS_MSK)
		*impmis = 1;
	else
		*impmis = 0;

    /* Open */
    if(regData & RTL8368S_CABLE_TESTING_CH_OPEN_MSK)
		*open_state = 1;
	else
		*open_state = 0;

    /* Short */
    if(regData & RTL8368S_CABLE_TESTING_CH_SHORT_MSK)
		*short_state = 1;
	else
		*short_state = 0;

    /* Length */
    retVal = rtl8368s_getAsicReg((RTL8368S_CABLE_TESTING_INFO_BASE + (port * 5) + channel + 1), &regData);
	if(retVal !=  SUCCESS)
		return retVal;
	
	*length  = regData;
    return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicAgeTimerSpeed | Set LUT agging out speed
@parm uint32 | timer | Agging out timer 0:Has been aged out.
@parm uint32 | speed | Agging out speed 0-fastest 3-slowest.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid parameter.
@comm
 	The API can set LUT agging out period for each entry. Differet {timer, speed} parameter will make asic do agging out task
 	at different period time. Following description is times period for different timer,speed combination setting.
			Timer		1		2		3		4		5		6		7
	Speed
	0					14.3s	28.6s	42.9s	57.2s	1.19m	1.43m	1.67m
	1					28.6s	57.2s	1.43m	1.9m	2.38m	2.86m	3.34m
	2					57.2s	1.9m	2.86m	3.81m	4.77m	5.72m	6.68m
	3					1.9m	3.8m	5.72m	7.63m	9.54m	11.45m	13.36m
 	(s:Second m:Minute)
 */
int32 rtl8368s_setAsicAgeTimerSpeed( uint32 timer, uint32 speed)
{
	uint32 retVal;
	uint32 regData;
	uint32 regBits;

	if(timer>RTL8368S_AGE_TIMER_MAX)
		return ERRNO_INPUT;

	if(speed >RTL8368S_AGE_SPEED_MAX)
		return ERRNO_INPUT;
	
	regBits = RTL8368S_AGE_TIMER_MSK | RTL8368S_AGE_SPEED_MSK;

	regData = ((timer<<RTL8368S_AGE_TIMER_OFF) & RTL8368S_AGE_TIMER_MSK) | ((speed<<RTL8368S_AGE_SPEED_OFF) & RTL8368S_AGE_SPEED_MSK);

	retVal = rtl8368s_setAsicRegBits(RTL8368S_SECURITY_CTRL2_REG,regBits,regData);		
	
	return retVal;	
}

/*
@func int32 | rtl8368s_getAsicAgeTimerSpeed | Set LUT agging out speed
@parm uint32* | timer | Agging out timer 0:Has been aged out.
@parm uint32* | speed | Agging out speed 0-fastest 3-slowest.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid parameter.
@comm
 	The API can set LUT agging out period for each entry. Differet {timer, speed} parameter will make asic do agging out task
 	at different period time. Following description is times period for different timer,speed combination setting.
			Timer		1		2		3		4		5		6		7
	Speed
	0					14.3s	28.6s	42.9s	57.2s	1.19m	1.43m	1.67m
	1					28.6s	57.2s	1.43m	1.9m	2.38m	2.86m	3.34m
	2					57.2s	1.9m	2.86m	3.81m	4.77m	5.72m	6.68m
	3					1.9m	3.8m	5.72m	7.63m	9.54m	11.45m	13.36m
 	(s:Second m:Minute)
 */
int32 rtl8368s_getAsicAgeTimerSpeed( uint32* timer, uint32* speed)
{
	uint32 regData;
	int32 retVal;

	retVal = rtl8368s_getAsicReg(RTL8368S_SECURITY_CTRL2_REG,&regData);
	if(retVal != SUCCESS)
		return retVal;

	*timer =  (regData & RTL8368S_AGE_TIMER_MSK) >> RTL8368S_AGE_TIMER_OFF;
	
	*speed =  (regData & RTL8368S_AGE_SPEED_MSK) >> RTL8368S_AGE_SPEED_OFF;

	return SUCCESS;

}


/*
@func int32 | rtl8368s_setAsicPortIsolation | Set port isolation function enable usage configuration.
@parm enum PORTID | port | Egress port number (0~8).
@parm uint32 | enabled | Isolation function usage 1:enable, 0:disabled.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue FAILED | Invalid parameter.
@comm
	This API can set ASIC port isolation functions' uage. It is enabled per port. By using isolation setting, 
	user can isolate physical ports' group without VLAN configuration. This function will physically seperate networking 
	flow and isolate network within desired ports only.
 */

int32 rtl8368s_setAsicPortIsolation(enum PORTID port, uint32 enable)
{
	int32 retVal;

	if(port >=PORT_MAX)
		return ERRNO_INPUT;
	retVal = rtl8368s_setAsicRegBit(RTL8368S_PORT_ISOLATION_BASE+(port),RTL8368S_PORT_ISOLATION_ENABLE_OFF, enable);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicPortIsolation | Get port isolation function enable usage configuration.
@parm enum PORTID | port | Egress port number (0~8).
@parm uint32* | enabled | Isolation function usage 1:enable, 0:disabled.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue FAILED | Invalid parameter.
@comm
	This API can get ASIC port isolation functions' uage. By using isolation setting, user can isolate physical
	ports' group without VLAN configuration. This function will physically seperate networking flow and isolate network 
	within desired ports only.
 */

int32 rtl8368s_getAsicPortIsolation( enum PORTID port, uint32* enabled)
{
	uint32 regData;
	int32 retVal;

	if(port >=PORT_MAX)
		return ERRNO_INPUT;

	retVal = rtl8368s_getAsicReg(RTL8368S_PORT_ISOLATION_BASE+(port),&regData);
	if(retVal != SUCCESS)
		return retVal;

	if(regData & RTL8368S_PORT_ISOLATION_ENABLE_MSK)
		*enabled = 1;
	else
		*enabled = 0;

	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicPortIsolationConfig | Set ASIC port isolation control configuration. 
@parm enum PORTID | port | Source port number (0~8).
@uint32 | portmask | Output port mask (0-8).
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@rvalue ERRNO_INPUT | Invalid port number.
@comm
	The API can isolation source port output to output ports represented by port mask. When the API is used,
	packets from source port will not deliver to other ports that are not in port mask. Other ports not in port mask 
	can still deliver packets to source port. By using isolation setting, user can isolate physical ports' group without
	VLAN configuration. This function will physically seperate networking flow and isolate network within desired ports only.
*/
int32 rtl8368s_setAsicPortIsolationConfig(enum PORTID port, uint32 portmask)
{

	int32 retVal;
	uint32 regData;

	if(port >=PORT_MAX)
		return ERRNO_INPUT;
 
 	retVal = rtl8368s_getAsicReg(RTL8368S_PORT_ISOLATION_BASE+(port),&regData);
	regData = (regData & RTL8368S_PORT_ISOLATION_ENABLE_MSK)|((portmask<<RTL8368S_PORT_ISOLATION_PORTS_OFF)&RTL8368S_PORT_ISOLATION_PORTS_MSK);
	retVal = rtl8368s_setAsicReg(RTL8368S_PORT_ISOLATION_BASE+(port),regData);

	return retVal;
}
/*
@func int32 | rtl8368s_getAsicPortIsolationConfig | Get ASIC port isolation control configuration. 
@parm enum PORTID | port | Source port number (0~8).
@uint32* | portmask | Output port mask (0-8).
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@rvalue ERRNO_INPUT | Invalid port number.
@comm
 	The API can be used to get the port isolation configuration.
*/

int32 rtl8368s_getAsicPortIsolationConfig(enum PORTID port, uint32* portmask)
{

	int32 retVal;
	uint32 regData;

	if(port >=PORT_MAX)
		return ERRNO_INPUT;
 
	retVal = rtl8368s_getAsicReg(RTL8368S_PORT_ISOLATION_BASE+port, &regData);

	*portmask = (regData & RTL8368S_PORT_ISOLATION_PORTS_MSK) >>RTL8368S_PORT_ISOLATION_PORTS_OFF;

	return retVal;
}

/*
@func int32 | rtl8368s_setAsicSpecialCongestModeConfig | Set ASIC special congest mode configuration. 
@parm enum PORTID | port | port number (0~8).
@uint32 | sustain | sustain timer (0-15).
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@rvalue ERRNO_INPUT | Invalid port number.
@comm
	The API can set ASIC special congest mode per port. It is a timer that if the source port was blocked by 
	pause frame and get into congestion state, the port will return normal state after sustain time.
*/

int32 rtl8368s_setAsicSpecialCongestModeConfig(enum PORTID port, uint32 sustain)
{

	int32 retVal;
	uint32 regData;

	if(port >=PORT_MAX)
		return ERRNO_INPUT;

	 retVal = rtl8368s_getAsicReg(RTL8368S_CONGEST_MODE_CONTROL_BASE+(port*RTL8368S_CONGEST_MODE_CONTROL_OFF),&regData);
	 
	regData = (regData&0xFFF0)|((sustain<<RTL8368S_CONGEST_MODE_SUSTAIN_OFF)&RTL8368S_CONGEST_MODE_SUSTAIN_MSK);
	
	retVal = rtl8368s_setAsicReg(RTL8368S_CONGEST_MODE_CONTROL_BASE+(port*RTL8368S_CONGEST_MODE_CONTROL_OFF),regData);

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicSpecialCongestModeConfig | Get ASIC special congest mode configuration. 
@parm enum PORTID | port | port number (0~8).
@uint32* | sustain | sustain timer (0-15).
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@rvalue ERRNO_INPUT | Invalid port number.
@comm
	The API can get ASIC special congest mode setup per port.
*/
int32 rtl8368s_getAsicSpecialCongestModeConfig(enum PORTID port, uint32* sustain)
{

	int32 retVal;
	uint32 regData;

	if(port >=PORT_MAX)
		return ERRNO_INPUT;
 
	retVal = rtl8368s_getAsicReg(RTL8368S_CONGEST_MODE_CONTROL_BASE+(port*RTL8368S_CONGEST_MODE_CONTROL_OFF),&regData);

	*sustain = (regData&RTL8368S_CONGEST_MODE_SUSTAIN_MSK)>>RTL8368S_CONGEST_MODE_SUSTAIN_OFF;

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicSpecialCongestModeTimer | Get ASIC special congest mode timer. 
@parm enum PORTID | port | port number (0~8).
@uint32* | timer | time (0-15).
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@rvalue ERRNO_INPUT | Invalid port number.
@comm
	The API can get ASIC special congest mode time from congestion state to normal state now per port.
*/
int32 rtl8368s_getAsicSpecialCongestModeTimer(enum PORTID port, uint32* timer)
{

	int32 retVal;
	uint32 regData;

	if(port >=PORT_MAX)
		return ERRNO_INPUT;
 
	retVal = rtl8368s_getAsicReg(RTL8368S_CONGEST_MODE_CONTROL_BASE+(port*RTL8368S_CONGEST_MODE_CONTROL_OFF),&regData);

	*timer = (regData&RTL8368S_CONGEST_MODE_TIMER_MSK)>>RTL8368S_CONGEST_MODE_TIMER_OFF;

	return retVal;
}

/*
@func int32 | rtl8368s_setAsicOamParser | Set OAM parser state
@parm enum PORTID | port | Physical port number (0~7).
@parm enum OAMPARACT | parser | Per-Port OAM parser state
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@comm
	The API can set per-port OAM parser state. Using OAM parser and multiplex configuration to implement 
	OAM loopback testing.
 */
int32 rtl8368s_setAsicOamParser(enum PORTID port, enum OAMPARACT parser)
{
	int32 retVal;
	uint32 regData;
	uint32 regBits;

	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	if(parser > OAM_PARFWDCPU)
		return ERRNO_OAM_PARACT;
	
	regBits = 0x3 << (2*port);
	regData = parser <<(2*port);

	retVal = rtl8368s_setAsicRegBits(RTL8368S_OAM_PARSER_REG,regBits,regData);		

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicOamParser | Set OAM parser state
@parm enum PORTID | port | Physical port number (0~7).
@parm enum OAMPARACT* | parser | Per-Port OAM parser state
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@comm
	The API can get per-port OAM parser state. Using OAM parser and multiplex configuration to implement 
	OAM loopback testing.
 */
int32 rtl8368s_getAsicOamParser(enum PORTID port, enum OAMPARACT* parser)
{
	int32 retVal;
	uint32 regData;

	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	retVal = rtl8368s_getAsicReg(RTL8368S_OAM_PARSER_REG,&regData);
	if(retVal != SUCCESS)
		return retVal;

	*parser =  (regData & (0x3 << (2*port))) >> (2*port);
	
	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicOamMultiplexer | Set OAM multiplexer state
@parm enum PORTID | port | Physical port number (0~7).
@parm enum OAMMULACT | multiplexer | Per-Port OAM multiplexer state
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@comm
	The API can set per-port OAM multiplexer state. Using OAM parser and multiplex configuration to implement 
	OAM loopback testing.
 */
int32 rtl8368s_setAsicOamMultiplexer(enum PORTID port, enum OAMMULACT multiplexer)
{
	int32 retVal;
	uint32 regData;
	uint32 regBits;

	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	if(multiplexer > OAM_MULCPU)
		return ERRNO_OAM_MULACT;
	
	regBits = 0x3 << (2*port);
	regData = multiplexer <<(2*port);

	retVal = rtl8368s_setAsicRegBits(RTL8368S_OAM_MULTIPLEXER_REG,regBits,regData);		

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicOamMultiplexer | Get OAM multiplexer state
@parm enum PORTID | port | Physical port number (0~7).
@parm enum OAMMULACT* | multiplexer | Per-Port OAM multiplexer state
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_PORT_NUM | Invalid port number.
@comm
	The API can get per-port OAM multiplexer state. Using OAM parser and multiplex configuration to implement 
	OAM loopback testing.
 */
int32 rtl8368s_getAsicOamMultiplexer(enum PORTID port, enum OAMMULACT* multiplexer)
{
	int32 retVal;
	uint32 regData;

	if(port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	retVal = rtl8368s_getAsicReg(RTL8368S_OAM_MULTIPLEXER_REG,&regData);
	if(retVal != SUCCESS)
		return retVal;

	*multiplexer =  (regData & (0x3 << (2*port))) >> (2*port);
	
	return SUCCESS;
}

/*
@func int32 | rtl8368s_setAsicMacAddress | Set Mac Address of Switch
@parm ether_addr_t | macAsic | MAC
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@comm
	The API can set MAC address of switch. The MAC address will be attached to SA of PAUSE frame.
 */
int32 rtl8368s_setAsicMacAddress(ether_addr_t macAsic)
{
	int32 retVal;
	uint32 regData;
	uint16 *accessPtr;
	uint32 i;


	accessPtr =  (uint16*)&macAsic;

	regData = *accessPtr;
	for(i=0;i<=2;i++)
	{
		retVal = rtl8368s_setAsicReg(RTL8368S_SWITCH_MAC_BASE+i,regData);
		if(retVal !=  SUCCESS)
			return retVal;

		accessPtr ++;
		regData = *accessPtr;
	}

	return retVal;
}

/*
@func int32 | rtl8368s_getAsicMacAddress | Set Mac Address of Switch
@parm ether_addr_t* | macAsic | MAC
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@comm
	The API can get MAC address of switch. The MAC address will be attached to SA of PAUSE frame.
 */
int32 rtl8368s_getAsicMacAddress(ether_addr_t* macAsic)
{
	int32 retVal;
	uint32 regData;
	uint16 *accessPtr;
	uint32 i;


	accessPtr =  (uint16*)macAsic;

	for(i=0;i<=2;i++)
	{
		retVal = rtl8368s_getAsicReg(RTL8368S_SWITCH_MAC_BASE+i,&regData);
		if(retVal !=  SUCCESS)
			return retVal;

		*accessPtr = regData;

		accessPtr ++;
	}

	return retVal;
}


/*
@func int32 | rtl8368s_setAsicGreenFeature | Set green ethernet function.
@parm uint32 | enable | Green ethernet function usage 1:enable, 0:disabled.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue FAILED | Invalid parameter.
@comm
	This API can set ASIC green ethernet functions' uage. When green feature function was enabled, RTL8366S/SR will dynamic estimate
connection cable length and turn on different Tx/Rx power mode with cable estimate result.
 */
int32 rtl8368s_setAsicGreenFeature(uint32 enable)
{
	uint32 regData;

	if(enable>1)
		return FAILED;

	if(SUCCESS != rtl8368s_getAsicReg(RTL8368S_REVISION_ID_REG,&regData))
		return FAILED;

	if (enable==1)
	{
		if (regData==0x0001)
		{
			if(SUCCESS != rtl8368s_setAsicReg(RTL8368S_GREEN_FEATURE_REG, 0x0007))
				return FAILED;
		}
		else
		{
			if(SUCCESS != rtl8368s_setAsicReg(RTL8368S_GREEN_FEATURE_REG, 0x0003))
				return FAILED;
		}
	}
	else
	{
		if(SUCCESS != rtl8368s_setAsicReg(RTL8368S_GREEN_FEATURE_REG, 0))
			return FAILED;
	}

	return SUCCESS;
}

/*
@func int32 | rtl8368s_getAsicGreenFeature | get green ethernet function.
@parm uint32* | enable | Green ethernet function usage 1:enable, 0:disabled.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue FAILED | Invalid parameter.
@comm
	This API can get ASIC green ethernet functions' uage. When green feature function was enabled, RTL8366S/SR will dynamic estimate
connection cable length and turn on different Tx/Rx power mode with cable estimate result.
 */
int32 rtl8368s_getAsicGreenFeature(uint32* enable)
{
	int32 retVal;
	int32 regValue;
	uint32 regData;

	retVal = rtl8368s_getAsicReg(RTL8368S_GREEN_FEATURE_REG,&regValue);

	if(SUCCESS != retVal)
		return retVal;
	
	if(SUCCESS != rtl8368s_getAsicReg(RTL8368S_REVISION_ID_REG,&regData))
		return FAILED;

	
	if (regData==0x0001)
	{
		if((regValue & RTL8368S_GREEN_FEATURE_MSK)==RTL8368S_GREEN_FEATURE_MSK)
		{
			*enable = 1;	
		}
		else
		{
			*enable = 0;	
		}
	}
	else
	{
		if((regValue & RTL8368S_GREEN_FEATURE_MSK)==0x0003)
		{
			*enable = 1;	
		}
		else
		{
			*enable = 0;	
		}
	}
	return SUCCESS;
}


/*
@func int32 | rtl8368s_setAsicPowerSaving | Set power saving function.
@parm uint32 | phyNo | PHY number (0~3).
@parm uint32 | enabled | Qos function usage 1:enable, 0:disabled.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue FAILED | Invalid parameter.
@comm
  	This API can set ASIC power saving functions' uage for dedicated PHY.When UTP is not plugged-in, this port only transmits link 
  pulse on TX and detects signal on RX. When UTP is plugged-in in power saving mode, this port should link on proper status by Nway 
  or parallel detection result.
 */
int32 rtl8368s_setAsicPowerSaving(uint32 phyNo,uint32 enabled)
{
	uint32 retVal;
	uint32 phySmiAddr;
	uint32 phyData;
	
	if(phyNo > RTL8368S_PHY_NO_MAX)
		return ERROR_PHY_INVALIDPHYNO;


	retVal = rtl8368s_setAsicReg(RTL8368S_PHY_ACCESS_CTRL_REG, RTL8368S_PHY_CTRL_READ);
	if (retVal !=  SUCCESS) 
		return retVal;

	phySmiAddr = 0x8000 | (1<<(phyNo +RTL8368S_PHY_NO_OFFSET)) | 
			((RTL8368S_POWER_SAVING_PAGE<<RTL8368S_PHY_PAGE_OFFSET)&RTL8368S_PHY_PAGE_MASK) | 
			(RTL8368S_POWER_SAVING_REG&RTL8368S_PHY_REG_MASK);
	
	retVal = rtl8368s_setAsicReg(phySmiAddr, 0);
	if (retVal !=  SUCCESS) 
		return retVal;


	retVal = rtl8368s_getAsicReg(RTL8368S_PHY_ACCESS_DATA_REG,&phyData);
	if(retVal !=  SUCCESS)
		return retVal;


	retVal = rtl8368s_setAsicReg(RTL8368S_PHY_ACCESS_CTRL_REG, RTL8368S_PHY_CTRL_WRITE);
	if (retVal !=  SUCCESS) 
		return retVal;


	if(enabled)
	{
		phyData = phyData | RTL8368S_POWER_SAVING_BIT_MSK;
	}	
	else
	{
		phyData = phyData & (~RTL8368S_POWER_SAVING_BIT_MSK);
	}	
	
		
	retVal = rtl8368s_setAsicReg(phySmiAddr, phyData);
	if (retVal !=  SUCCESS) 
		return retVal;

	return SUCCESS;

}


/*
@func int32 | rtl8368s_getAsicPowerSaving | Set power saving function.
@parm uint32 | phyNo | PHY number (0~3).
@parm uint32* | enabled | Qos function usage 1:enable, 0:disabled.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue FAILED | Invalid parameter.
@comm
  	This API can get ASIC power saving functions' uage for dedicated PHY.When UTP is not plugged-in, this port only transmits link 
  pulse on TX and detects signal on RX. When UTP is plugged-in in power saving mode, this port should link on proper status by Nway 
  or parallel detection result.
 */
int32 rtl8368s_getAsicPowerSaving(uint32 phyNo,uint32* enabled)
{
	uint32 retVal;
	uint32 phySmiAddr;
	uint32 phyData;
	
	if(phyNo > RTL8368S_PHY_NO_MAX)
		return ERROR_PHY_INVALIDPHYNO;


	retVal = rtl8368s_setAsicReg(RTL8368S_PHY_ACCESS_CTRL_REG, RTL8368S_PHY_CTRL_READ);
	if (retVal !=  SUCCESS) 
		return retVal;

	phySmiAddr = 0x8000 | (1<<(phyNo +RTL8368S_PHY_NO_OFFSET)) | 
			((RTL8368S_POWER_SAVING_PAGE<<RTL8368S_PHY_PAGE_OFFSET)&RTL8368S_PHY_PAGE_MASK) | 
			(RTL8368S_POWER_SAVING_REG&RTL8368S_PHY_REG_MASK);
	
	retVal = rtl8368s_setAsicReg(phySmiAddr, 0);
	if (retVal !=  SUCCESS) 
		return retVal;


	retVal = rtl8368s_getAsicReg(RTL8368S_PHY_ACCESS_DATA_REG,&phyData);
	if(retVal !=  SUCCESS)
		return retVal;



	if(phyData & RTL8368S_POWER_SAVING_BIT_MSK) 
	{
		*enabled = 1;
	}	
	else
	{
		*enabled = 0;
	}	
	
	return SUCCESS;
}



