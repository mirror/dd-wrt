#include "smi.h"
#include "rtl8368s_errno.h"
#include "rtl8368s_reg.h"
#include "rtl8366rb_api.h"
#include "rtl8368s_asicdrv.h"
#include "rtl8366rb_api_compat.h"


/*************************************************************************
 * derive from rtl8368s_asicdrv.c
 *************************************************************************/
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

/* ZJin 091116 turn on/off SYS, WLAN, USB, QSS led when 5 enet led inited. */
static void toggleGpioLeds(int on)
{
#define	MY_WRITE(y, z)	((*((volatile unsigned int*)(y))) = z)
#define	MY_READ(y)		(*((volatile unsigned int*)(y)))
	if (1 == on)
	{
		MY_WRITE(0xb8040000, MY_READ(0xb8040000) | (0x200));	/* wlan out put enable */
	    MY_WRITE(0xb8040008, MY_READ(0xb8040008) | (0x20));		/* qss */
		MY_WRITE(0xb8040008, MY_READ(0xb8040008) & (~0x206));	/* sys,usb */
	}
	else
	{
		MY_WRITE(0xb8040008, MY_READ(0xb8040008) & (~0x20));	/* qss */
		MY_WRITE(0xb8040008, MY_READ(0xb8040008) | (0x206));	/* sys,usb */
	}
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

/************************************************************************
 * derive from rtl8366rb_api.c 
 ************************************************************************/

#define DELAY_800MS_FOR_CHIP_STATABLE() {  }


/*
gap: 48  drop: 800
<Share buffer> OFF: 389      ON: 390
<Port  based>  OFF: 510      ON: 511
<Q-Desc based> OFF:  28      ON:  40
<Q-pkt based>  OFF:  40      ON:  48
*/
/* QoS parameter & threshold */
const uint16 g_thresholdGap[6]= { 72,72,72,72,72,72 };
const uint16 g_thresholdSystem[6][3]= { {562,526,1000},{562,526,1000},{562,526,1000},{562,526,1000},{562,526,1000},{562,526,1000} };
const uint16 g_thresholdShared[6][2]= { {384,360},{384,360},{384,360},{384,360},{384,360},{384,360}};
const uint16 g_thresholdPort[6][2]= { {288,276},{288,276},{288,276},{288,276},{288,276},{288,276}};
const uint16 g_thresholdQueueDescriptor[6][2]= {{72,64}, {72,64}, {72,64}, {72,64}, {72,64}, {72,64}};
const uint16 g_thresholdQueuePacket[6][2]= { {24,20},{24,20},{24,20},{24,20},{24,20},{24,20} };

const uint16 g_prioritytToQueue[6][8]= { 
	{QUEUE0,QUEUE0,QUEUE0,QUEUE0,QUEUE0,QUEUE0,QUEUE0,QUEUE0}, 
	{QUEUE0,QUEUE0,QUEUE0,QUEUE0,QUEUE5,QUEUE5,QUEUE5,QUEUE5}, 
	{QUEUE0,QUEUE0,QUEUE0,QUEUE0,QUEUE1,QUEUE1,QUEUE5,QUEUE5}, 
	{QUEUE0,QUEUE0,QUEUE1,QUEUE1,QUEUE2,QUEUE2,QUEUE5,QUEUE5},
	{QUEUE0,QUEUE0,QUEUE1,QUEUE1,QUEUE2,QUEUE3,QUEUE5,QUEUE5},
	{QUEUE0,QUEUE0,QUEUE1,QUEUE1,QUEUE2,QUEUE3,QUEUE4,QUEUE5}
};

const uint16 g_weightQueue[6][6] = { {0,0,0,0,0,0}, {0,0,0,0,0,1}, {0,1,0,0,0,3}, {0,1,3,0,0,7}, {0,1,3,7,0,15}, {0,1,3,7,15,31} };
const uint16 g_priority1QRemapping[8] = {1,0,2,3,4,5,6,7 };

/*
@func int32 | rtl8366rb_setMac5ForceLink | Set Port/MAC 5 force linking configuration.
@parm enum PORTLINKSPEED | speed | Speed of the port 0b00-10M, 0b01-100M,0b10-1000M, 0b11 reserved.
@parm enum PORTLINKDUPLEXMODE | duplex | Deuplex mode 0b0-half duplex 0b1-full duplex.
@parm uint32 | link | Link status 0b0-link down b1-link up.
@parm uint32 | txPause | Pause frame transmit ability of the port 0b0-inactive 0b1-active.
@parm uint32 | rxPause | Pause frame response ability of the port 0b0-inactive 0b1-active.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue FAILED | Invalid parameter.
@comm
  	This API can set Port/MAC 5 properties in force mode. 
 */
int32 rtl8366rb_setMac5ForceLink(rtl8366rb_macConfig_t *ptr_maccfg)
{
	if(ptr_maccfg->speed > SPD_1000M)
		return ERROR_MAC_INVALIDSPEED;

	if(SUCCESS != rtl8368s_setAsicMacForceLink(PORT5,1,ptr_maccfg->speed,ptr_maccfg->duplex,ptr_maccfg->link,ptr_maccfg->txPause,ptr_maccfg->rxPause))
		return FAILED;

	return SUCCESS;
}

/*
@func int32 | rtl8366rb_initChip | Set chip to default configuration enviroment
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	The API can set chip registers to default configuration for different release chip model.
*/
int32 rtl8366rb_initChip(void)
{
	uint32 index;
	uint32 ledGroup;
	enum RTL8368S_LEDCONF ledCfg[RTL8366RB_LED_GROUP_MAX];
	uint32 verid;
	
	
	const uint32 chipDefault[][2] = {{0x000B, 0x0001},{0x03A6, 0x0100},{0x3A7, 0x0001},{0x02D1, 0x3FFF},
								{0x02D2, 0x3FFF},{0x02D3, 0x3FFF},{0x02D4, 0x3FFF},{0x02D5, 0x3FFF},
								{0x02D6, 0x3FFF},{0x02D7, 0x3FFF},{0x02D8, 0x3FFF},{0x022B, 0x0688},								
								{0x022C, 0x0FAC},{0x03D0, 0x4688},{0x03D1, 0x01F5},{0x0000, 0x0830},
								{0x02F9, 0x0200},{0x02F7, 0x7FFF},{0x02F8, 0x03FF},{0x0080, 0x03E8},
								{0x0081, 0x00CE},{0x0082, 0x00DA},{0x0083, 0x0230},{0xBE0F, 0x2000},
								{0x0231, 0x422A},{0x0232, 0x422A},{0x0233, 0x422A},{0x0234, 0x422A},
								{0x0235, 0x422A},{0x0236, 0x422A},{0x0237, 0x422A},{0x0238, 0x422A},
								{0x0239, 0x422A},{0x023A, 0x422A},{0x023B, 0x422A},{0x023C, 0x422A},
								{0x023D, 0x422A},{0x023E, 0x422A},{0x023F, 0x422A},{0x0240, 0x422A},
								{0x0241, 0x422A},{0x0242, 0x422A},{0x0243, 0x422A},{0x0244, 0x422A},
								{0x0245, 0x422A},{0x0246, 0x422A},{0x0247, 0x422A},{0x0248, 0x422A},
								{0x0249, 0x0146},{0x024A, 0x0146},{0x024B, 0x0146},{0xBE03, 0xC961},
								{0x024D, 0x0146},{0x024E, 0x0146},{0x024F, 0x0146},{0x0250, 0x0146},
								{0xBE64, 0x0226},{0x0252, 0x0146},{0x0253, 0x0146},{0x024C, 0x0146},
								{0x0251, 0x0146},{0x0254, 0x0146},{0xBE62, 0x3FD0},{0x0084, 0x0320},
								{0x0255, 0x0146},{0x0256, 0x0146},{0x0257, 0x0146},{0x0258, 0x0146},	
								{0x0259, 0x0146},{0x025A, 0x0146},{0x025B, 0x0146},{0x025C, 0x0146},	
								{0x025D, 0x0146},{0x025E, 0x0146},{0x025F, 0x0146},{0x0260, 0x0146},		
								{0x0261, 0xA23F},{0x0262, 0x0294},{0x0263, 0xA23F},{0x0264, 0x0294},	
								{0x0265, 0xA23F},{0x0266, 0x0294},{0x0267, 0xA23F},{0x0268, 0x0294},	
								{0x0269, 0xA23F},{0x026A, 0x0294},{0x026B, 0xA23F},{0x026C, 0x0294},	
								{0x026D, 0xA23F},{0x026E, 0x0294},{0x026F, 0xA23F},{0x0270, 0x0294},	
								{0x02F5, 0x0048},{0xBE09, 0x0E00},{0xBE1E, 0x0FA0},{0xBE14, 0x8448},
								{0xBE15, 0x1007},{0xBE4A, 0xA284},{0xC454, 0x3F0B},{0xC474, 0x3F0B},
								{0xBE48, 0x3672},{0xBE4B, 0x17A7},{0xBE4C, 0x0B15},{0xBE52, 0x0EDD},
								{0xBE49, 0x8C00},{0xBE5B, 0x785C},{0xBE5C, 0x785C},{0xBE5D, 0x785C},
								{0xBE61, 0x368A},{0xBE63, 0x9B84},{0xC456, 0xCC13},{0xC476, 0xCC13},
								{0xBE65, 0x307D},{0xBE6D, 0x0005},{0xBE6E, 0xE120},{0xBE2E, 0x7BAF},		
								{0xFFFF, 0xABCD}};

    const uint32 chipDefault2[][2] = {{0x0000, 0x0830},{0x0400, 0x8130},{0x000A, 0x83ED},{0x0431, 0x5432},
								   {0x0F51, 0x0017},{0x02F5, 0x0048},{0x02FA, 0xFFDF},{0x02FB, 0xFFE0},
                                   {0xC456, 0x0C14},{0xC476, 0x0C14},{0xC454, 0x3F8B},{0xC474, 0x3F8B},
                                   {0xC450, 0x2071},{0xC470, 0x2071},{0xC451, 0x226B},{0xC471, 0x226B},
                                   {0xC452, 0xA293},{0xC472, 0xA293},{0xC44C, 0x1585},{0xC44C, 0x1185},
                                   {0xC44C, 0x1585},{0xC46C, 0x1585},{0xC46C, 0x1185},{0xC46C, 0x1585},
                                   {0xC44C, 0x0185},{0xC44C, 0x0181},{0xC44C, 0x0185},{0xC46C, 0x0185},
                                   {0xC46C, 0x0181},{0xC46C, 0x0185},{0xBE24, 0xB000},{0xBE23, 0xFF51},
                                   {0xBE22, 0xDF20},{0xBE21, 0x0140},{0xBE20, 0x00BB},{0xBE24, 0xB800},
                                   {0xBE24, 0x0000},{0xBE24, 0x7000},{0xBE23, 0xFF51},{0xBE22, 0xDF60},
                                   {0xBE21, 0x0140},{0xBE20, 0x0077},{0xBE24, 0x7800},{0xBE24, 0x0000},
                                   {0xBE2E, 0x7BA7},{0xBE36, 0x1000},{0xBE37, 0x1000},{0x8000, 0x0001},
                                   {0xBE69, 0xD50F},{0x8000, 0x0000},{0xBE69, 0xD50F},{0xBE6B, 0x0320},
                                   {0xBE77, 0x2800},{0xBE78, 0x3C3C},{0xBE79, 0x3C3C},{0xBE6E, 0xE120},
                                   {0x8000, 0x0001},{0xBE10, 0x8140},{0x8000, 0x0000},{0xBE10, 0x8140},
                                   {0xBE15, 0x1007},{0xBE14, 0x0448},{0xBE1E, 0x00A0},{0xBE10, 0x8160},
                                   {0xBE10, 0x8140},{0xBE00, 0x1340},{0x0450, 0x0000},{0x0401, 0x0000},
                                   {0xFFFF, 0xABCD}};

	toggleGpioLeds(1);
                                   
	for(ledGroup= 0;ledGroup<RTL8366RB_LED_GROUP_MAX;ledGroup++)
	{
		if(SUCCESS != rtl8368s_getAsicLedIndicateInfoConfig(ledGroup,&ledCfg[ledGroup]))
			return FAILED;

		if(SUCCESS != rtl8368s_setAsicLedIndicateInfoConfig(ledGroup,LEDCONF_LEDFORCE))
			return FAILED;
	}

	if(SUCCESS != rtl8368s_setAsicForceLeds(0x3F,0x3F,0x3F,0x3F))
		return FAILED;

	if(SUCCESS != rtl8368s_getAsicReg(RTL8368S_REVISION_ID_REG,&verid))
			return FAILED;
	index = 0;

    switch (verid)
    {
        case 0x0000:
            while(chipDefault[index][0] != 0xFFFF && chipDefault[index][1] != 0xABCD)
    		{	
    			/*PHY registers setting*/	
    			if(0xBE00 == (chipDefault[index][0] & 0xBE00))
    			{
    				if(SUCCESS != rtl8368s_setAsicReg(RTL8368S_PHY_ACCESS_CTRL_REG, RTL8368S_PHY_CTRL_WRITE))
    					return FAILED;
    
    			}
    
    			if(SUCCESS != rtl8368s_setAsicReg(chipDefault[index][0],chipDefault[index][1]))
    				return FAILED;
    				
    			index ++;	
    		}
            break;

        case 0x0001:
        case 0x0002:
        case 0x0003:
        default:
            while(chipDefault2[index][0] != 0xFFFF && chipDefault2[index][1] != 0xABCD)
    		{	
    			/*PHY registers setting*/	
    			if(0xBE00 == (chipDefault2[index][0] & 0xBE00))
    			{
    				if(SUCCESS != rtl8368s_setAsicReg(RTL8368S_PHY_ACCESS_CTRL_REG, RTL8368S_PHY_CTRL_WRITE))
    					return FAILED;
    
    			}
    
    			if(SUCCESS != rtl8368s_setAsicReg(chipDefault2[index][0],chipDefault2[index][1]))
    				return FAILED;
    				
    			index ++;
				/* make the GPIO LEDS be turned off at the same time with enet leds. */
				if (index == 55)
				{
					toggleGpioLeds(0);
				}
    		}
            break;
    }

	DELAY_800MS_FOR_CHIP_STATABLE();


	for(ledGroup= 0;ledGroup<RTL8366RB_LED_GROUP_MAX;ledGroup++)
	{
		if(SUCCESS != rtl8368s_setAsicLedIndicateInfoConfig(ledGroup,ledCfg[ledGroup]))
			return FAILED;
			
	}

	return SUCCESS;
}

/*
@func int32 | rtl8366rb_initVlan | Initialize VLAN.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	VLAN is disabled by default. User has to call this API to enable VLAN before
	using it. And It will set a default VLAN(vid 1) including all ports and set 
	all ports PVID to the default VLAN.
*/
int32 rtl8366rb_initVlan(void)
{
	uint32 i;
	rtl8368s_user_vlan4kentry vlan4K;
	rtl8368s_vlanconfig vlanMC;
	
	/* clear 16 VLAN member configuration */
	for(i = 0; i <= RTL8366RB_VLANMCIDXMAX; i++)
	{	
		vlanMC.vid = 0;
		vlanMC.priority = 0;
		vlanMC.member = 0;		
		vlanMC.untag = 0;			
		vlanMC.fid = 0;
		if(rtl8368s_setAsicVlanMemberConfig(i, vlanMC) != SUCCESS)
			return FAILED;	
	}

	/* Set a default VLAN with vid 1 to 4K table for all ports */
   	vlan4K.vid = 1;
 	vlan4K.member = RTL8366RB_PORTMASK;
 	vlan4K.untag = RTL8366RB_PORTMASK;
 	vlan4K.fid = 0;	
	if(rtl8368s_setAsicVlan4kEntry(vlan4K) != SUCCESS)
		return FAILED;	

	/* Also set the default VLAN to 16 member configuration index 0 */
	vlanMC.vid = 1;
	vlanMC.priority = 0;
	vlanMC.member = RTL8366RB_PORTMASK;		
	vlanMC.untag = RTL8366RB_PORTMASK;			
	vlanMC.fid = 0;
	if(rtl8368s_setAsicVlanMemberConfig(0, vlanMC) != SUCCESS)
		return FAILED;	

	/* Set all ports PVID to default VLAN */	
	for(i = 0; i < PORT_MAX; i++)
	{	
		if(rtl8368s_setAsicVlanPortBasedVID(i, 0) != SUCCESS)
			return FAILED;		
	}	

	/* enable VLAN and 4K VLAN */
	if(rtl8368s_setAsicVlan(TRUE)!= SUCCESS)
		return FAILED;	
	if(rtl8368s_setAsicVlan4kTbUsage(TRUE)!= SUCCESS)
		return FAILED;
		
	return SUCCESS;
}

/*
@func int32 | rtl8366rb_setVlan | Set a VLAN entry.
@parm rtl8366rb_vlanConfig_t* | ptr_vlancfg | The pointer of VLAN configuration.
@rvalue ERRNO_API_INPUT | invalid input parameters.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	There are 4K VLAN entry supported. Member set and untag set	of 4K VLAN entry
	are all zero by default. User could configure the member set and untag set
	for specified vid through this API.	The portmask's bit N means port N.
	For example, mbrmask 23=0x17=010111 means port 0,1,2,4 in the member set.
*/
int32 rtl8366rb_setVlan(rtl8366rb_vlanConfig_t *ptr_vlancfg)
{
	uint32 i;
	rtl8368s_user_vlan4kentry vlan4K;
	rtl8368s_vlanconfig vlanMC;	
	
    if(ptr_vlancfg == NULL)
        return ERRNO_INPUT;

	/* vid must be 0~4095 */
	if(ptr_vlancfg->vid > RTL8366RB_VIDMAX)
		return ERRNO_VID;

	if(ptr_vlancfg->mbrmsk > RTL8366RB_PORTMASK)
		return ERRNO_MBR;

	if(ptr_vlancfg->untagmsk > RTL8366RB_PORTMASK)
		return ERRNO_UTAG;

		/* fid must be 0~7 */
	if(ptr_vlancfg->fid > RTL8366RB_FIDMAX)
		return ERRNO_FID;

	/* update 4K table */
   	vlan4K.vid    = ptr_vlancfg->vid;			
 	vlan4K.member = ptr_vlancfg->mbrmsk;
 	vlan4K.untag  = ptr_vlancfg->untagmsk;
 	vlan4K.fid    = ptr_vlancfg->fid;	
	if(rtl8368s_setAsicVlan4kEntry(vlan4K) != SUCCESS)
		return FAILED;
	
	/* 
		Since VLAN entry would be copied from 4K to 16 member configuration while
		setting Port-based VLAN. So also update the entry in 16 member configuration
		if it existed.
	*/
	for(i = 0; i <= RTL8366RB_VLANMCIDXMAX; i++)
	{	
		if(rtl8368s_getAsicVlanMemberConfig(i, &vlanMC) != SUCCESS)
			return FAILED;	

		if(ptr_vlancfg->vid == vlanMC.vid)
		{
			vlanMC.member = ptr_vlancfg->mbrmsk;		
			vlanMC.untag  = ptr_vlancfg->untagmsk;			
			vlanMC.fid    = ptr_vlancfg->fid;
			if(rtl8368s_setAsicVlanMemberConfig(i, vlanMC) != SUCCESS)
				return FAILED;	

			return SUCCESS;
		}	
	}
	
	return SUCCESS;
}

/*
@func int32 | rtl8366rb_setVlanPVID | Set port to specified VLAN ID(PVID).
@parm uint32 | port | Port number (0~5).
@parm uint32 | vid | Specified VLAN ID (0~4095).
@parm uint32 | priority | 802.1p priority for the PVID (0~7).
@rvalue ERRNO_API_INPUT | invalid input parameters.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	The API is used for Port-based VLAN. The untagged frame received from the
	port will be classified to the specified VLAN and assigned to the specified priority.
	Make sure you have configured the VLAN by 'rtl8366rb_setVlan' before using the API.
*/
int32 rtl8366rb_setVlanPVID(uint32 port, uint32 vid, uint32 priority)
{
	uint32 i;
	uint32 j;
	uint32 index;	
	uint8  bUsed;	
	rtl8368s_user_vlan4kentry vlan4K;
	rtl8368s_vlanconfig vlanMC;	

	if(port >= PORT_MAX)
		return ERRNO_PORT_NUM;
	
	/* vid must be 0~4095 */
	if(vid > RTL8366RB_VIDMAX)
		return ERRNO_VID;

	/* priority must be 0~7 */
	if(priority > RTL8366RB_PRIORITYMAX)
		return ERRNO_PRIORITY;
	
	/* 
		Search 16 member configuration to see if the entry already existed.
		If existed, update the priority and assign the index to the port.
	*/
	for(i = 0; i <= RTL8366RB_VLANMCIDXMAX; i++)
	{	
		if(rtl8368s_getAsicVlanMemberConfig(i, &vlanMC) != SUCCESS)
			return FAILED;	

		if(vid == vlanMC.vid)
		{
			vlanMC.priority = priority;		
			if(rtl8368s_setAsicVlanMemberConfig(i, vlanMC) != SUCCESS)
				return FAILED;	
		
			if(rtl8368s_setAsicVlanPortBasedVID(port, i) != SUCCESS)
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
	for(i = 0; i <= RTL8366RB_VLANMCIDXMAX; i++)
	{	
		if(rtl8368s_getAsicVlanMemberConfig(i, &vlanMC) != SUCCESS)
			return FAILED;	

		if(vlanMC.vid == 0 && vlanMC.member == 0)
		{
			vlan4K.vid = vid;
			if(rtl8368s_getAsicVlan4kEntry(&vlan4K) != SUCCESS)
				return FAILED;

			vlanMC.vid = vid;
			vlanMC.priority = priority;
			vlanMC.member = vlan4K.member;		
			vlanMC.untag = vlan4K.untag;			
			vlanMC.fid = vlan4K.fid;			
			if(rtl8368s_setAsicVlanMemberConfig(i, vlanMC) != SUCCESS)
				return FAILED;	

			if(rtl8368s_setAsicVlanPortBasedVID(port, i) != SUCCESS)
				return FAILED;	

			return SUCCESS;			
		}	
	}	

	/* 16 member configuration is full, found a unused entry to replace */
	for(i = 0; i <= RTL8366RB_VLANMCIDXMAX; i++)
	{	
		bUsed = FALSE;	

		for(j = 0; j < PORT_MAX; j++)
		{	
			if(rtl8368s_getAsicVlanPortBasedVID(j, &index) != SUCCESS)
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
			if(rtl8368s_getAsicVlan4kEntry(&vlan4K) != SUCCESS)
				return FAILED;

			vlanMC.vid = vid;
			vlanMC.priority = priority;
			vlanMC.member = vlan4K.member;		
			vlanMC.untag = vlan4K.untag;			
			vlanMC.fid = vlan4K.fid;
			vlanMC.stag_idx = 0;
			vlanMC.stag_mbr = 0;			
			if(rtl8368s_setAsicVlanMemberConfig(i, vlanMC) != SUCCESS)
				return FAILED;	

			if(rtl8368s_setAsicVlanPortBasedVID(port, i) != SUCCESS)
				return FAILED;	

			return SUCCESS;			
		}
	}	
	
	return FAILED;
}

/*
@func int32 | rtl8366rb_setGreenEthernet | Set green ethernet function.
@parm uint32 | greenFeature | Green feature function usage 1:enable 0:disable.
@parm uint32 | powerSaving | Power saving mode 1:power saving mode 0:normal mode.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
 	The API can set Green Ethernet function to reduce power consumption. 
    While green feature is enabled, ASIC will automatic detect the cable 
    length and then select different power mode for best performance with 
    minimums power consumption. Link down ports will enter power savining 
    mode in 10 seconds after the cable disconnected if power saving function 
    is enabled.
*/
int32 rtl8366rb_setGreenEthernet(uint32 greenFeature, uint32 powerSaving)
{
	uint32 phyNo;
	uint32 regData;
	uint32 idx;
	const uint32 greenSeting[][2] = {	{0xBE78,0x323C},	{0xBE77,0x5000}, {0xBE2E,0x7BA7},
									{0xBE59,0x3459},	{0xBE5A,0x745A},	{0xBE5B,0x785C},
									{0xBE5C,0x785C}, {0xBE6E,0xE120},	{0xBE79,0x323C},	
									{0xFFFF,0xABCD}
								};

	idx = 0;	
	while(greenSeting[idx][0] != 0xFFFF && greenSeting[idx][1] != 0xABCD)
	{
		if(SUCCESS != rtl8368s_getAsicReg(RTL8368S_PHY_ACCESS_BUSY_REG, &regData))
				return FAILED;
		if((regData&RTL8368S_PHY_INT_BUSY_MASK)==0)
		{
			/*PHY registers setting*/	
			if(SUCCESS != rtl8368s_setAsicReg(RTL8368S_PHY_ACCESS_CTRL_REG, RTL8368S_PHY_CTRL_WRITE))
				return FAILED;
			if(SUCCESS != rtl8368s_setAsicReg(greenSeting[idx][0],greenSeting[idx][1]))
				return FAILED;
			idx ++;
		}
	}	
		
	if(SUCCESS !=rtl8368s_setAsicGreenFeature(greenFeature))
		return FAILED;

	for(phyNo = 0; phyNo<=RTL8366RB_PHY_NO_MAX;phyNo++)
	{
		if(SUCCESS !=rtl8368s_setAsicPowerSaving(phyNo,powerSaving))
			return FAILED;	
	}

	return SUCCESS;
}


/*
@func int32 | rtl8366rb_getPHYLinkStatus | Get ethernet PHY linking status
@parm uint32 | phy | PHY number (0~4).
@parm uint32* | ptr_linkStatus | PHY link status 1:link up 0:link down
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	Output link status of bit 2 in PHY register 1. API will return 
    status is link up under both auto negotiation complete and link 
    status are set to 1.  
*/
int32 rtl8366rb_getPHYLinkStatus(uint32 phy, uint32 *ptr_linkStatus)
{
	uint32 phyData;

	if(phy > RTL8366RB_PHY_NO_MAX)
		return FAILED;

	/*Get PHY status register*/
	if(SUCCESS != rtl8368s_getAsicPHYRegs(phy,0,RTL8366RB_PHY_STATUS_REG,&phyData))
		return FAILED;

	/*check link status*/
	if(phyData & (1<<2))
	{
		*ptr_linkStatus = 1;
	}
	else
	{
		*ptr_linkStatus = 0;	
	}

	return SUCCESS;
}


