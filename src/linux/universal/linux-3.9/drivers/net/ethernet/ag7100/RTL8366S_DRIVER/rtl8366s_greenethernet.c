/*
* Copyright c                  Realtek Semiconductor Corporation, 2007 
* All rights reserved.
* 
* Program : RTL8366S/SR switch high-level API
* Abstract : 
* Author : Abel Hsu (abelshie@realtek.com.tw)                 
* $Id: rtl8366s_greenethernet.c,v 1.1 2007/08/21 08:30:49 abelshie Exp $
*/
/*	@doc RTL8366S_API

	@module Rtl8366s_api.c - RTL8366S/SR switch high-level API documentation	|
	This document explains Green Ethernet API interface of the asic. 
	@normal 

	Copyright <cp>2007 Realtek<tm> Semiconductor Cooperation, All Rights Reserved.

 	@head3 List of Symbols |
 	Here is a list of all functions and variables in this module.

 	@index | RTL8366S_GREEBETHERNET
*/

#include "rtl8366s_errno.h"
#include "rtl8366s_asicdrv.h"
  
/*
@func int32 | rtl8366s_setAsicGreenFeature | Set green ethernet function.
@parm uint32 | txGreen | Green ethernet Tx function usage 1:enable, 0:disabled.
@parm uint32 | rxGreen | Green ethernet Rx function usage 1:enable, 0:disabled.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@rvalue FAILED | Invalid parameter.
@comm
	This API can set ASIC green ethernet functions' uage. When green feature function was enabled, RTL8366S/SR will dynamic estimate
connection cable length and turn on different Tx/Rx power mode with cable estimate result.
 */
int32 rtl8366s_setAsicGreenFeature(uint32 txGreen,uint32 rxGreen)
{
	int32 retVal;


	retVal = rtl8366s_setAsicRegBit(RTL8366S_GREEN_FEATURE_REG, RTL8366S_GREEN_FEATURE_TX_BIT,txGreen);
	if(SUCCESS != retVal)
		return retVal;

	retVal = rtl8366s_setAsicRegBit(RTL8366S_GREEN_FEATURE_REG, RTL8366S_GREEN_FEATURE_RX_BIT,rxGreen);
	if(SUCCESS != retVal)
		return retVal;


	return SUCCESS;
}

/*
@func int32 | rtl8366s_getAsicGreenFeature | get green ethernet function.
@parm uint32* | txGreen | Green ethernet Tx function usage 1:enable, 0:disabled.
@parm uint32* | rxGreen | Green ethernet Rx function usage 1:enable, 0:disabled.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@rvalue FAILED | Invalid parameter.
@comm
	This API can get ASIC green ethernet functions' uage. When green feature function was enabled, RTL8366S/SR will dynamic estimate
connection cable length and turn on different Tx/Rx power mode with cable estimate result.
 */
int32 rtl8366s_getAsicGreenFeature(uint32* txGreen,uint32* rxGreen)
{
	int32 retVal;
	int32 regValue;

	retVal = rtl8366s_getAsicReg(RTL8366S_GREEN_FEATURE_REG,&regValue);

	if(SUCCESS != retVal)
		return retVal;

	
	if(regValue & RTL8366S_GREEN_FEATURE_TX_MSK)
	{
		*txGreen = 1;	
	}
	else
	{
		*txGreen = 0;	
	}
	
	if(regValue & RTL8366S_GREEN_FEATURE_RX_MSK)
	{
		*rxGreen = 1;	
	}
	else
	{
		*rxGreen = 0;	
	}

	return SUCCESS;
}

/*
@func int32 | rtl8366s_setAsicPowerSaving | Set power saving function.
@parm uint32 | phyNo | PHY number (0~4).
@parm uint32 | enabled | Qos function usage 1:enable, 0:disabled.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@rvalue FAILED | Invalid parameter.
@comm
  	This API can set ASIC power saving functions' uage for dedicated PHY.When UTP is not plugged-in, this port only transmits link 
  pulse on TX and detects signal on RX. When UTP is plugged-in in power saving mode, this port should link on proper status by Nway 
  or parallel detection result.
 */
int32 rtl8366s_setAsicPowerSaving(uint32 phyNo,uint32 enabled)
{
	uint32 retVal;
	uint32 phySmiAddr;
	uint32 phyData;
	
	if(phyNo > RTL8366S_PHY_NO_MAX)
		return ERROR_PHY_INVALIDPHYNO;


	retVal = rtl8366s_setAsicReg(RTL8366S_PHY_ACCESS_CTRL_REG, RTL8366S_PHY_CTRL_READ);
	if (retVal !=  SUCCESS) 
		return retVal;

	phySmiAddr = 0x8000 | (1<<(phyNo +RTL8366S_PHY_NO_OFFSET)) | 
			((0 <<RTL8366S_PHY_PAGE_OFFSET)&RTL8366S_PHY_PAGE_MASK) | (12 &RTL8366S_PHY_REG_MASK);
	
	retVal = rtl8366s_setAsicReg(phySmiAddr, 0);
	if (retVal !=  SUCCESS) 
		return retVal;


	retVal = rtl8366s_getAsicReg(RTL8366S_PHY_ACCESS_DATA_REG,&phyData);
	if(retVal !=  SUCCESS)
		return retVal;


	retVal = rtl8366s_setAsicReg(RTL8366S_PHY_ACCESS_CTRL_REG, RTL8366S_PHY_CTRL_WRITE);
	if (retVal !=  SUCCESS) 
		return retVal;


	if(enabled)
	{
		phyData = phyData | (1<<12);
	}	
	else
	{
		phyData = phyData & (~(1<<12));
	}	
	
		
	retVal = rtl8366s_setAsicReg(phySmiAddr, phyData);
	if (retVal !=  SUCCESS) 
		return retVal;

	return SUCCESS;
}


/*
@func int32 | rtl8366s_getAsicPowerSaving | Set power saving function.
@parm uint32 | phyNo | PHY number (0~4).
@parm uint32* | enabled | Qos function usage 1:enable, 0:disabled.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@rvalue FAILED | Invalid parameter.
@comm
  	This API can get ASIC power saving functions' uage for dedicated PHY.When UTP is not plugged-in, this port only transmits link 
  pulse on TX and detects signal on RX. When UTP is plugged-in in power saving mode, this port should link on proper status by Nway 
  or parallel detection result.
 */
int32 rtl8366s_getAsicPowerSaving(uint32 phyNo,uint32* enabled)
{
	uint32 retVal;
	uint32 phySmiAddr;
	uint32 phyData;
	
	if(phyNo > RTL8366S_PHY_NO_MAX)
		return ERROR_PHY_INVALIDPHYNO;


	retVal = rtl8366s_setAsicReg(RTL8366S_PHY_ACCESS_CTRL_REG, RTL8366S_PHY_CTRL_READ);
	if (retVal !=  SUCCESS) 
		return retVal;

	phySmiAddr = 0x8000 | (1<<(phyNo +RTL8366S_PHY_NO_OFFSET)) | 
			((0 <<RTL8366S_PHY_PAGE_OFFSET)&RTL8366S_PHY_PAGE_MASK) | (12 &RTL8366S_PHY_REG_MASK);
	
	retVal = rtl8366s_setAsicReg(phySmiAddr, 0);
	if (retVal !=  SUCCESS) 
		return retVal;


	retVal = rtl8366s_getAsicReg(RTL8366S_PHY_ACCESS_DATA_REG,&phyData);
	if(retVal !=  SUCCESS)
		return retVal;



	if(phyData & (1<<12)) 
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
@func int32 | rtl8366s_setGreenEthernet | Set green ethernet function.
@parm uint32 | greenFeature | Green feature function usage 1:enable 0:disable.
@parm uint32 | powerSaving | Power saving mode 1:power saving mode 0:normal mode.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
 	The API can set Green Ethernet function to reduce power consumption. While green feature is enabled, ASIC will automatic
 detect the cable length and then select different power mode for best performance with minimums power consumption. Link down
 ports will enter power savining mode in 10 seconds after the cable disconnected if power saving function is enabled.
*/
int32 rtl8366s_setGreenEthernet(uint32 greenFeature, uint32 powerSaving)
{
	uint32 phyNo;
	uint32 regData;
	uint32 idx;
	const uint32 greenSeting[6][2] = {	{0xBE5B,0x3500},
									{0xBE5C,0xB975},
									{0xBE5D,0xB9B9},
									{0xBE77,0xA500},
									{0xBE78,0x5A78},
									{0xBE79,0x6478}};
	/*resivion*/
	if(SUCCESS != rtl8366s_getAsicReg(0x5C,&regData))
		return FAILED;

	if(0x0000 == regData)
	{	
		for(idx = 0;idx <6;idx++)
		{
			rtl8366s_setAsicReg(RTL8366S_PHY_ACCESS_CTRL_REG, RTL8366S_PHY_CTRL_WRITE);
			rtl8366s_setAsicReg(greenSeting[idx][0],greenSeting[idx][1]);
		}		
		
	}
	else
	{
		rtl8366s_setAsicReg(RTL8366S_PHY_ACCESS_CTRL_REG, RTL8366S_PHY_CTRL_WRITE);
		rtl8366s_setAsicReg(greenSeting[0][0],greenSeting[0][1]);
	}

	rtl8366s_setAsicGreenFeature(greenFeature,powerSaving);

	for(phyNo = 0; phyNo<=RTL8366S_PHY_NO_MAX;phyNo++)
	{
		rtl8366s_setAsicPowerSaving(phyNo,powerSaving);
	}

	return SUCCESS;
}


