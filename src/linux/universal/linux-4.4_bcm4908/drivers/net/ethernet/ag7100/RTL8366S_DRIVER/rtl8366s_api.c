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
@func int32 | rtl8366s_setEthernetPHY | Set ethernet PHY registers for desired ability.
@parm uint32 | phy | PHY number (0~4).
@parm phyAbility_t | ability | Ability structure
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	If Full_1000 bit is set to 1, the AutoNegotiation,Full_100 and Full_10 will be automatic set to 1. While both AutoNegotiation and Full_1000 are set to
	0, the PHY speed and duplex selection will be set as following 100F > 100H > 10F > 10H priority sequence.
*/
int32 rtl8366s_setEthernetPHY(uint32 phy, phyAbility_t ability)
{
	uint32 phyData;
	
	uint32 phyEnMsk0;
	uint32 phyEnMsk4;
	uint32 phyEnMsk9;
	
	if(phy > RTL8366S_PHY_NO_MAX)
		return FAILED;

	phyEnMsk0 = 0;
	phyEnMsk4 = 0;
	phyEnMsk9 = 0;


	if(1 == ability.Half_10)
	{
		/*10BASE-TX half duplex capable in reg 4.5*/
		phyEnMsk4 = phyEnMsk4 | (1<<5);

		/*Speed selection [1:0] */
		/* 11=Reserved*/
		/* 10= 1000Mpbs*/
		/* 01= 100Mpbs*/
		/* 00= 10Mpbs*/		
		phyEnMsk0 = phyEnMsk0 & (~(1<<6));
		phyEnMsk0 = phyEnMsk0 & (~(1<<13));
	}

	if(1 == ability.Full_10)
	{
		/*10BASE-TX full duplex capable in reg 4.6*/
		phyEnMsk4 = phyEnMsk4 | (1<<6);
		/*Speed selection [1:0] */
		/* 11=Reserved*/
		/* 10= 1000Mpbs*/
		/* 01= 100Mpbs*/
		/* 00= 10Mpbs*/		
		phyEnMsk0 = phyEnMsk0 & (~(1<<6));
		phyEnMsk0 = phyEnMsk0 & (~(1<<13));

		/*Full duplex mode in reg 0.8*/
		phyEnMsk0 = phyEnMsk0 | (1<<8);
		
	}

	if(1 == ability.Half_100)
	{
		/*100BASE-TX half duplex capable in reg 4.7*/
		phyEnMsk4 = phyEnMsk4 | (1<<7);
		/*Speed selection [1:0] */
		/* 11=Reserved*/
		/* 10= 1000Mpbs*/
		/* 01= 100Mpbs*/
		/* 00= 10Mpbs*/		
		phyEnMsk0 = phyEnMsk0 & (~(1<<6));
		phyEnMsk0 = phyEnMsk0 | (1<<13);
	}


	if(1 == ability.Full_100)
	{
		/*100BASE-TX full duplex capable in reg 4.8*/
		phyEnMsk4 = phyEnMsk4 | (1<<8);
		/*Speed selection [1:0] */
		/* 11=Reserved*/
		/* 10= 1000Mpbs*/
		/* 01= 100Mpbs*/
		/* 00= 10Mpbs*/		
		phyEnMsk0 = phyEnMsk0 & (~(1<<6));
		phyEnMsk0 = phyEnMsk0 | (1<<13);
		/*Full duplex mode in reg 0.8*/
		phyEnMsk0 = phyEnMsk0 | (1<<8);
	}
	
	
	if(1 == ability.Full_1000)
	{
		/*1000 BASE-T FULL duplex capable setting in reg 9.9*/
		phyEnMsk9 = phyEnMsk9 | (1<<9);
		/*100BASE-TX full duplex capable in reg 4.8*/
		phyEnMsk4 = phyEnMsk4 | (1<<8);
		/*10BASE-TX full duplex capable in reg 4.6*/
		phyEnMsk4 = phyEnMsk4 | (1<<6);

		/*Speed selection [1:0] */
		/* 11=Reserved*/
		/* 10= 1000Mpbs*/
		/* 01= 100Mpbs*/
		/* 00= 10Mpbs*/		
		phyEnMsk0 = phyEnMsk0 | (1<<6);
		phyEnMsk0 = phyEnMsk0 & (~(1<<13));
	

		/*Force Auto-Negotiation setting in reg 0.12*/
		ability.AutoNegotiation = 1;

	}
	
	if(1 == ability.AutoNegotiation)
	{
		/*Auto-Negotiation setting in reg 0.12*/
		phyEnMsk0 = phyEnMsk0 | (1<<12);
	}

	if(1 == ability.AsyFC)
	{
		/*Asymetric flow control in reg 4.11*/
		phyEnMsk4 = phyEnMsk4 | (1<<11);
	}
	if(1 == ability.FC)
	{
		/*Flow control in reg 4.10*/
		phyEnMsk4 = phyEnMsk4 | (1<<10);
	}

	
	/*1000 BASE-T control register setting*/
	if(SUCCESS != rtl8366s_getAsicPHYRegs(phy,0,PHY_1000_BASET_CONTROL_REG,&phyData))
		return FAILED;

	phyData = (phyData & (~0x0200)) | phyEnMsk9 ;
		
	if(SUCCESS != rtl8366s_setAsicPHYRegs(phy,0,PHY_1000_BASET_CONTROL_REG,phyData))
		return FAILED;

	/*Auto-Negotiation control register setting*/
	if(SUCCESS != rtl8366s_getAsicPHYRegs(phy,0,PHY_AN_ADVERTISEMENT_REG,&phyData))
		return FAILED;

	phyData = (phyData & (~0x0DE0)) | phyEnMsk4;
		
	if(SUCCESS != rtl8366s_setAsicPHYRegs(phy,0,PHY_AN_ADVERTISEMENT_REG,phyData))
		return FAILED;


	/*Control register setting and restart auto*/
	if(SUCCESS != rtl8366s_getAsicPHYRegs(phy,0,PHY_CONTROL_REG,&phyData))
		return FAILED;


	phyData = (phyData & (~0x3140)) | phyEnMsk0;
	/*If have auto-negotiation capable, then restart auto negotiation*/
	if(1 == ability.AutoNegotiation)
	{
		phyData = phyData | (1 << 9);
	}
	
	if(SUCCESS != rtl8366s_setAsicPHYRegs(phy,0,PHY_CONTROL_REG,phyData))
		return FAILED;

	
	return SUCCESS;
}

/*
@func int32 | rtl8366s_getEthernetPHY | Get PHY ability through PHY registers.
@parm uint32 | phy | PHY number (0~4).
@parm phyAbility_t* | ability | Ability structure
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	Get the capablity of specified PHY.
*/
int32 rtl8366s_getEthernetPHY(uint32 phy, phyAbility_t* ability)
{
	uint32 phyData0;
	uint32 phyData4;
	uint32 phyData9;
	

	if(phy > RTL8366S_PHY_NO_MAX)
		return FAILED;


	/*Control register setting and restart auto*/
	if(SUCCESS != rtl8366s_getAsicPHYRegs(phy,0,PHY_CONTROL_REG,&phyData0))
		return FAILED;

	/*Auto-Negotiation control register setting*/
	if(SUCCESS != rtl8366s_getAsicPHYRegs(phy,0,PHY_AN_ADVERTISEMENT_REG,&phyData4))
		return FAILED;

	/*1000 BASE-T control register setting*/
	if(SUCCESS != rtl8366s_getAsicPHYRegs(phy,0,PHY_1000_BASET_CONTROL_REG,&phyData9))
		return FAILED;

	if(phyData9 & (1<<9))
		ability->Full_1000 = 1;
	else
		ability->Full_1000 = 0;

	if(phyData4 & (1<<11))
		ability->AsyFC = 1;
	else
		ability->AsyFC = 0;

	if(phyData4 & (1<<10))
		ability->FC = 1;
	else
		ability->FC = 0;
	

	if(phyData4 & (1<<8))
		ability->Full_100= 1;
	else
		ability->Full_100= 0;
	
	if(phyData4 & (1<<7))
		ability->Half_100= 1;
	else
		ability->Half_100= 0;

	if(phyData4 & (1<<6))
		ability->Full_10= 1;
	else
		ability->Full_10= 0;
	
	if(phyData4 & (1<<5))
		ability->Half_10= 1;
	else
		ability->Half_10= 0;


	if(phyData0 & (1<<12))
		ability->AutoNegotiation= 1;
	else
		ability->AutoNegotiation= 0;

	return SUCCESS;
}


#define MAX_PHY_RETRY 3 
#define DELAY_PHY_RETRY 30 

static uint32_t rtl8366s_getAsicPHYRegsRetry( uint32_t phyNo, uint32_t page, uint32_t addr, uint32_t *data){
	int i;
	uint32_t status;
	for(i = 0; i < MAX_PHY_RETRY; i++){
		status = rtl8366s_getAsicPHYRegs(phyNo, page, addr, data);
		if( status == SUCCESS){
			break;
		}else{
			//DEBUG_MSG(("%s: Retry count = %d \n",__FUNCTION__, i));
		}
		udelay(DELAY_PHY_RETRY);
	}
	return status;
}

/*
@func int32 | rtl8366s_getPHYLinkStatus | Get ethernet PHY linking status
@parm uint32 | phy | PHY number (0~4).
@parm uint32* | linkStatus | PHY link status 1:link up 0:link down
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	Output link status of bit 2 in PHY register 1. API will return status is link up under both auto negotiation complete and link status are set to 1.  
*/
int32 rtl8366s_getPHYLinkStatus(uint32 phy, uint32 *linkStatus)
{
	uint32 phyData;

	if(phy > RTL8366S_PHY_NO_MAX)
		return FAILED;

	/*Get PHY status register*/
	if(SUCCESS != rtl8366s_getAsicPHYRegsRetry(phy,0,PHY_STATUS_REG,&phyData))
		return FAILED;

	/*check link status*/
	if(phyData & (1<<2))
	{
		*linkStatus = 1;
	}
	else
	{
		*linkStatus = 0;	
	}

	return SUCCESS;
}

/*
@func int32 | rtl8366s_setPHYTestMode | Set PHY in test mode.
@parm uint32 | phy | PHY number (0~4).
@parm uint32 | mode | PHY test mode 0:normal 1:test mode 1 2:test mode 2 3: test mode 3 4:test mode 4 5~7:reserved
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	Set PHY in test mode and only one PHY can be in test mode at the same time. It means API will return FALED if other PHY is in test mode. 
*/
int32 rtl8366s_setPHYTestMode(uint32 phy, uint32 mode)
{
	uint32 phyData, index;
	int16 phyIdx;
	const uint32 TestMode[][2] = {{0x8000, 0x8404}, {0x8014, 0xFF00}, {0x8015, 0x000C},
							     {0xFFFF, 0xABCD}};
	const uint32 NormalMode[][2] = {{0x8000, 0x8304}, {0x8014, 0x5500}, {0x8015, 0x0004},
							     {0xFFFF, 0xABCD}};

	if(phy > RTL8366S_PHY_NO_MAX)
		return FAILED;

	if(mode > PHY_TEST_MODE_MAX)
		return FAILED;

	switch(mode)
	{
		case PHY_NORMAL_MODE:	
			index = 0;
			while(NormalMode[index][0] != 0xFFFF && NormalMode[index][1] != 0xABCD)
			{	
				if(SUCCESS != rtl8366s_setAsicReg(NormalMode[index][0],NormalMode[index][1]))
					return FAILED;
				index ++;	
			}
			break;
		case PHY_TEST_MODE1:			
			index = 0;
			while(TestMode[index][0] != 0xFFFF && TestMode[index][1] != 0xABCD)
			{	
				if(SUCCESS != rtl8366s_setAsicReg(TestMode[index][0],TestMode[index][1]))
					return FAILED;			
				index ++;	
			}			
			break;			
		case PHY_TEST_MODE2:
		case PHY_TEST_MODE3:
		case PHY_TEST_MODE4:
			break;

	 	default:
			return FAILED;
			break;
	}

	if(PHY_NORMAL_MODE == mode)
	{
		if(SUCCESS != rtl8366s_getAsicPHYRegs(phy,0,PHY_1000_BASET_CONTROL_REG,&phyData))
			return FAILED;

		phyData = phyData & (~(0x7<<13));

		if(SUCCESS != rtl8366s_setAsicPHYRegs(phy,0,PHY_1000_BASET_CONTROL_REG,phyData))
			return FAILED;
		
		if(SUCCESS != rtl8366s_setAsicPHYRegs(phy,2, 29, 0xB9B9))
			return FAILED;
		
	}	
	else
	{
		phyIdx = 0;
		while(phyIdx <= RTL8366S_PHY_NO_MAX)
		{
			if(phyIdx != phy)
			{
				if(SUCCESS != rtl8366s_getAsicPHYRegs(phyIdx,0,PHY_1000_BASET_CONTROL_REG,&phyData))
					return FAILED;				
				/*have other PHY in test mode*/	
				if(phyData & (0x7<<13))
				{
					return FAILED;
				}				
			}
			phyIdx ++;
		}
		if(SUCCESS != rtl8366s_getAsicPHYRegs(phy,0,PHY_1000_BASET_CONTROL_REG,&phyData))
			return FAILED;
		
		phyData = (phyData & (~(0x7<<13))) | (mode<<13);

		if(SUCCESS != rtl8366s_setAsicPHYRegs(phy,0,PHY_1000_BASET_CONTROL_REG,phyData))
			return FAILED;
		if(SUCCESS != rtl8366s_setAsicPHYRegs(phy,2, 29, 0xEEEE))
			return FAILED;
		
	}

	return SUCCESS;
}	

/*
@func int32 | rtl8366s_getPHYTestMode | Get PHY in which test mode.
@parm uint32 | phy | PHY number (0~4).
@parm uint32* | mode | PHY test mode 0:normal 1:test mode 1 2:test mode 2 3: test mode 3 4:test mode 4 5~7:reserved
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	Get test mode of PHY from register setting 9.15 to 9.13.
*/
int32 rtl8366s_getPHYTestMode(uint32 phy, uint32* mode)
{
	uint32 phyData;

	if(phy > RTL8366S_PHY_NO_MAX)
		return FAILED;

	if(SUCCESS != rtl8366s_getAsicPHYRegs(phy,0,PHY_1000_BASET_CONTROL_REG,&phyData))
		return FAILED;

	*mode = (phyData>>13) & 0x7;

	return SUCCESS;
}

/*
@func int32 | rtl8366s_setPHY1000BaseTMasterSlave | Set PHY control enable MASTER/SLAVE manual configuration.
@parm uint32 | phy | PHY number (0~4).
@parm uint32 | enable | Manual configuration function 1:enable 0:disable.
@parm uint32 | masterslave | Manual config mode 1:master 0: slave
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	Set enable/disable MASTER/SLAVE manual configuration under 1000Base-T with register 9.12-9.11. If MASTER/SLAVE manual configuration is enabled with MASTER, the
	link partner must be set as SLAVE or auto negotiation will fail. 
*/
int32 rtl8366s_setPHY1000BaseTMasterSlave(uint32 phy, uint32 enabled, uint32 masterslave)
{
	uint32 phyData;

	if(phy > RTL8366S_PHY_NO_MAX)
		return FAILED;

	if(SUCCESS != rtl8366s_getAsicPHYRegs(phy,0,PHY_1000_BASET_CONTROL_REG,&phyData))
		return FAILED;

	phyData = (phyData & (~(0x3<<11))) | (enabled<<12) | (masterslave<<11);


	if(SUCCESS != rtl8366s_setAsicPHYRegs(phy,0,PHY_1000_BASET_CONTROL_REG,phyData))
		return FAILED;

	if(SUCCESS != rtl8366s_setAsicPHYRegs(phy,0,PHY_1000_BASET_CONTROL_REG,phyData))
		return FAILED;



	/*Restart N-way*/
	if(SUCCESS != rtl8366s_getAsicPHYRegs(phy,0,PHY_CONTROL_REG,&phyData))
		return FAILED;

	phyData = phyData | (1 << 9);
	
	if(SUCCESS != rtl8366s_setAsicPHYRegs(phy,0,PHY_CONTROL_REG,phyData))
		return FAILED;


	return SUCCESS;
}

/*
@func int32 | rtl8366s_resetMIBs | Reset MIB with port mask setting.
@parm uint32 | portMask | Port mask (0~0x3F).
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	Reset MIB counter of ports. API will use global reset while port mask is 0x3F.
*/
int32 rtl8366s_resetMIBs(uint32 portMask)
{
	if(portMask > RTL8366S_PORTMASK)
		return FAILED;
	
	/*get available bits only*/
	portMask = portMask & RTL8366S_PORTMASK;

	if(RTL8366S_PORTMASK == portMask)
	{
		if(SUCCESS != rtl8366s_setAsicMIBsCounterReset(RTL8366S_MIB_CTRL_GLOBAL_RESET_MSK))
			return FAILED;
	}
	else 
	{
		if(SUCCESS != rtl8366s_setAsicMIBsCounterReset(portMask<<(RTL8366S_MIB_CTRL_PORT_RESET_BIT)))
			return FAILED;
	}
		
	return SUCCESS;
}

/*
@func int32 | rtl8366s_getMIBCounter | Get MIB counter
@parm enum PORTID | port | Physical port number (0~5).
@num RTL8366S_MIBCOUNTER | mibIdx | MIB counter index.
@parm uint64* | counter | MIB retrived counter.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	Get MIB counter by index definition as following description. There are per port MIB counters from index 0 to 32 while index 33 
	is a system-wide counter.
	index	MIB counter												
	0 		IfInOctets
	1		EtherStatsOctets
	2		EtherStatsUnderSizePkts
	3		EtherFregament
	4		EtherStatsPkts64Octets
	5		EtherStatsPkts65to127Octets
	6		EtherStatsPkts128to255Octets
	7		EtherStatsPkts256to511Octets
	8		EtherStatsPkts512to1023Octets
	9		EtherStatsPkts1024to1518Octets
	10		EtherOversizeStats
	11		EtherStatsJabbers
	12		IfInUcastPkts
	13		EtherStatsMulticastPkts
	14		EtherStatsBroadcastPkts
	15		EtherStatsDropEvents
	16		Dot3StatsFCSErrors
	17		Dot3StatsSymbolErrors
	18		Dot3InPauseFrames
	19		Dot3ControlInUnknownOpcodes
	20		IfOutOctets
	21		Dot3StatsSingleCollisionFrames
	22		Dot3StatMultipleCollisionFrames
	23		Dot3sDeferredTransmissions
	24		Dot3StatsLateCollisions
	25		EtherStatsCollisions
	26		Dot3StatsExcessiveCollisions
	27		Dot3OutPauseFrames
	28		Dot1dBasePortDelayExceededDiscards
	29		Dot1dTpPortInDiscards
	30		IfOutUcastPkts
	31		IfOutMulticastPkts
	32		IfOutBroadcastPkts
	33		Dot1dTpLearnEntryDiscardFlag	
*/
int32 rtl8366s_getMIBCounter(enum PORTID port,enum RTL8366S_MIBCOUNTER mibIdx,uint64* counter)
{
	if(SUCCESS != rtl8366s_getAsicMIBsCounter(port,mibIdx,counter))		
		return FAILED;

	return SUCCESS;
}

/*
@func int32 | rtl8366s_setInterruptControl | Set gpio interrupt trigger configuration.
@parm uint32 | polarity | I/O polarity 0:pull high 1:pull low.
@parm uint32 | linkUpPortMask | Link up trigger enable port mask (0-0x3F).
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	ASIC pulls gpio high/low to trigger CPU about port link up event.
*/
int32 rtl8366s_setInterruptControl(uint32 polarity, uint32 linkUpPortMask)
{
	uint32 mask;

	if(linkUpPortMask > RTL8366S_PORTMASK)
		return FAILED;		

	if(SUCCESS != rtl8366s_setAsicInterruptPolarity(polarity))
		return FAILED;

	mask = linkUpPortMask & RTL8366S_PORTMASK;
	
	if(SUCCESS != rtl8366s_setAsicInterruptMask(mask))
		return FAILED;

	return SUCCESS;
}

/*
@func int32 | rtl8366s_getInterruptStatus | Get interrupt event status.
@parm uint32* | linkUpPortMask | Link up status mapped port mask. Clear by read.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	ASIC will trigger CPU again after interrupt status mask was read.
*/
int32 rtl8366s_getInterruptStatus(uint32 *linkUpPortMask)
{
	uint32 mask;
	
	if(SUCCESS != rtl8366s_getAsicInterruptStatus(&mask))
		return FAILED;


	*linkUpPortMask =  mask & RTL8366S_PORTMASK;

	return SUCCESS;
}

/*
@func int32 | rtl8366s_initQos | Configure Qos settings with queue number assigment to each port.
@parm uint32 | queueNum | Queue number of each port. it is available at 1~5.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	This API will initialize related Qos setting with queue number assigment. The Qos function enable usage must be set by other API rtl8366s_setAsicQosEnable(). 
*/
int32 rtl8366s_initQos(uint32 queueNum)
{
	int16 priority;
	uint32 qidCfg;

	enum QUEUEID qid;
	enum PORTID port;
	uint32 dscp;
	
	const enum QUEUEID qidConfig[] = {QUEUE0,QUEUE5,QUEUE1,QUEUE2,QUEUE3,QUEUE4};

	if(queueNum == 0 || queueNum >= QOS_DEFAULT_QUEUE_NO_MAX)
		return FAILED;

	/*Bandwidth control, set unlimit to default*/
	/*inbound Set flow control high/low threshold, no used while bandwidth control to unlimit*/
	for(port = PORT0;port<PORT_MAX;port ++)
	{
		if(SUCCESS != rtl8366s_setAsicPortIngressBandwidth(port,QOS_DEFAULT_INGRESS_BANDWIDTH,QOS_DEFAULT_PREIFP))
			return FAILED;	
		if(SUCCESS != rtl8366s_setAsicPortEgressBandwidth(port,QOS_DEFAULT_EGRESS_BANDWIDTH,QOS_DEFAULT_PREIFP))
			return FAILED;	
	}
	
	/*disable packet used pages flow control for RX reg[0x0264]*/
	if(SUCCESS != 	rtl8366s_setAsicPacketUsedPagesFlowControlRegister(QOS_DEFAULT_PACKET_USED_PAGES_FC,QOS_DEFAULT_PACKET_USED_FC_EN))
		return FAILED;	

	/*port-based priority to PRIORITY 0 for each port*/
	for(port=PORT0;port<PORT_MAX;port++)
	{
		if(SUCCESS != 	rtl8366s_setAsicPortPriority(port,QOS_DEFAULT_PORT_BASED_PRIORITY))
			return FAILED;	
	}		

	/*dscp to priority*/
	for(dscp = RTL8366S_DSCPMIN;dscp <= RTL8366S_DSCPMAX;dscp ++)
	{
		if(SUCCESS != 	rtl8366s_setAsicDscpPriority(dscp,QOS_DEFAULT_DSCP_MAPPING_PRIORITY))
			return FAILED;			
	}		
	
	/*1Q Priority remapping to absolutely priority */
	for(priority=0;priority<=RTL8366S_PRIORITYMAX;priority ++)
	{
		if(SUCCESS != 	rtl8366s_setAsicDot1qAbsolutelyPriority(priority,g_priority1QRemapping[priority]))
			return FAILED;			
	}
	
	/*User priority to traffic classs mapping setting*/
	for(priority=0;priority<=RTL8366S_PRIORITYMAX;priority ++)
	{
		if(SUCCESS != 	rtl8366s_setAsicPriorityToQIDMappingTable(QOS_DEFAULT_QUEUE_NO_MAX,priority,g_prioritytToQueue_patch[queueNum-1][priority]))
			return FAILED;			
	}
	
	/*priority selection with [Port-based > 802.1Q = DSCP = ACL] */
	if(SUCCESS != 	rtl8366s_setAsicPriorityDecision(QOS_DEFAULT_PRIORITY_SELECT_PORT,QOS_DEFAULT_PRIORITY_SELECT_1Q,QOS_DEFAULT_PRIORITY_SELECT_DSCP,QOS_DEFAULT_PRIORITY_SELECT_ACL))
		return FAILED;		

	/*disable Remarking */
	if(SUCCESS != 	rtl8366s_setAsicDscpRemarkingAbility(QOS_DEFAULT_DSCP_REMARKING_ABILITY))
		return FAILED;			
	if(SUCCESS != 	rtl8366s_setAsicDot1pRemarkingAbility(QOS_DEFAULT_1Q_REMARKING_ABILITY))
		return FAILED;			

	/*Buffer management - flow control threshold setting */	
	/*1. system based flow control threshold */
	if(SUCCESS != 	rtl8366s_setAsicSystemBasedFlowControlRegister(g_thresholdSystem[QOS_DEFAULT_QUEUE_NO_MAX-1][0],g_thresholdSystem[QOS_DEFAULT_QUEUE_NO_MAX-1][1],g_thresholdSystem[QOS_DEFAULT_QUEUE_NO_MAX-1][2]))
		return FAILED;

	/*2. port based threshold and queue based threshold */
	for(port = PORT0;port<=PORT5;port ++)
	{
		/*port based*/
		if(SUCCESS != 	rtl8366s_setAsicPortBasedFlowControlRegister(port,g_thresholdPort[QOS_DEFAULT_QUEUE_NO_MAX-1][0],g_thresholdPort[QOS_DEFAULT_QUEUE_NO_MAX-1][1]))
			return FAILED;	
		/*Queue based*/
		for(qidCfg = 0;qidCfg < QOS_DEFAULT_QUEUE_NO_MAX;qidCfg ++)
		{
			qid = qidConfig[qidCfg];
			/*queue based*/
			if(SUCCESS != 	rtl8366s_setAsicQueueDescriptorBasedFlowControlRegister(port,qid,g_thresholdQueueDescriptor[QOS_DEFAULT_QUEUE_NO_MAX-1][0]/2,g_thresholdQueueDescriptor[QOS_DEFAULT_QUEUE_NO_MAX-1][1]/2))
				return FAILED;					
			if(SUCCESS != 	rtl8366s_setAsicQueuePacketBasedFlowControlRegister(port,qid,g_thresholdQueuePacket[QOS_DEFAULT_QUEUE_NO_MAX-1][0]/4,g_thresholdQueuePacket[QOS_DEFAULT_QUEUE_NO_MAX-1][1]/4))
				return FAILED;		
			/*Enable queue-base flow control*/
			if(SUCCESS != rtl8366s_setAsicQueueFlowControlConfigureRegister(port,qid,QOS_DEFAULT_QUEUE_BASED_FC_EN))
				return FAILED;						
		}		
	}		

	/*queue gap */
	if(SUCCESS != 	rtl8366s_setAsicPerQueuePhysicalLengthGapRegister(g_thresholdGap[QOS_DEFAULT_QUEUE_NO_MAX-1]))
		return FAILED;
	
	/*Packet scheduling for Qos setting*/
	for(port = PORT0;port<=PORT5;port ++)
	{
		/*enable scheduler control ability APR,PPR,WFQ*/
		if(SUCCESS != 	rtl8366s_setAsicDisableSchedulerAbility(port,QOS_DEFAULT_SCHEDULER_ABILITY_APR,QOS_DEFAULT_SCHEDULER_ABILITY_PPR,QOS_DEFAULT_SCHEDULER_ABILITY_WFQ))
			return FAILED;				

		/*Queue based*/
		for(qidCfg = 0;qidCfg < QOS_DEFAULT_QUEUE_NO_MAX;qidCfg ++)
		{
			qid = qidConfig[qidCfg];
			/*initial weight fair queue setting*/
			if(SUCCESS != rtl8366s_setAsicQueueWeight(port,qid,STR_PRIO,g_weightQueue[QOS_DEFAULT_QUEUE_NO_MAX-1][qid]))
				return FAILED;				
			/*initial leacky bucket control*/
			if(SUCCESS != rtl8366s_setAsicQueueRate(port,qid,QOS_DEFAULT_PEAK_PACKET_RATE,QOS_DEFAULT_BURST_SIZE_IN_APR,QOS_DEFAULT_AVERAGE_PACKET_RATE))
				return FAILED;				
		}		
	}		

	/*Setting F=75Mhz with (T,B) = (19,34)*/
	if(SUCCESS != rtl8366s_setAsicLBParameter(QOS_DEFAULT_BYTE_PER_TOKEN,QOS_DEFAULT_TICK_PERIOD,QOS_DEFAULT_LK_THRESHOLD))
		return FAILED;			

	/*Set each port's queue number */
	for(port = PORT0;port<=PORT5;port++)
	{
		if(SUCCESS != rtl8366s_setAsicOutputQueueNumber(port, QOS_DEFAULT_QUEUE_NO_MAX))
			return FAILED;			
	}	

	/*Enable ASIC Qos function*/
	if(SUCCESS != rtl8366s_setAsicQosEnable(ENABLE))
		return FAILED;		

	/*Unlimit Pause Frame  */
	if(SUCCESS != rtl8366s_setAsicRegBit(RTL8366S_SWITCH_GLOBAL_CTRL_REG,RTL8366S_MAX_PAUSE_CNT_BIT,ENABLE))
		return FAILED;		

	/*Queue resetting*/
	if(SUCCESS != rtl8366s_setAsicRegBit(RTL8366S_RESET_CONTROL_REG, RTL8366S_RESET_QUEUE_BIT,ENABLE))
		return FAILED;		

	return SUCCESS;
}

/*
@func int32 | rtl8366s_setPortPriority | Configure priority usage to each port.
@parm portPriority_t | priority | Priorities assigment for each port.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	Priorityof ports assigment for queue usage and packet scheduling.
*/
int32 rtl8366s_setPortPriority(portPriority_t priority)
{
	if(SUCCESS != rtl8366s_setAsicPortPriority(PORT0, priority.priPort0))
		return FAILED;
	
	if(SUCCESS != rtl8366s_setAsicPortPriority(PORT1, priority.priPort1))
		return FAILED;
	
	if(SUCCESS != rtl8366s_setAsicPortPriority(PORT2, priority.priPort2))
		return FAILED;
	
	if(SUCCESS != rtl8366s_setAsicPortPriority(PORT3, priority.priPort3))
		return FAILED;
	
	if(SUCCESS != rtl8366s_setAsicPortPriority(PORT4, priority.priPort4))
		return FAILED;
	
	if(SUCCESS != rtl8366s_setAsicPortPriority(PORT5, priority.priPort5))
		return FAILED;

	return SUCCESS;
}

/*
@func int32 | rtl8366s_set1QMappingPriority | Configure 1Q priority mapping asic internal absolute priority.  
@parm dot1qPriority_t | priority | Priorities assigment for receiving 802.1Q prioirty.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	Priorityof 802.1Q assigment for queue usage and packet scheduling.
*/
int32 rtl8366s_set1QMappingPriority(dot1qPriority_t priority)
{
	if(SUCCESS != rtl8366s_setAsicDot1qAbsolutelyPriority(PRI0, priority.dot1qPri0))
		return FAILED;
	
	if(SUCCESS != rtl8366s_setAsicDot1qAbsolutelyPriority(PRI1, priority.dot1qPri0))
		return FAILED;
	
	if(SUCCESS != rtl8366s_setAsicDot1qAbsolutelyPriority(PRI2, priority.dot1qPri0))
		return FAILED;
	
	if(SUCCESS != rtl8366s_setAsicDot1qAbsolutelyPriority(PRI3, priority.dot1qPri0))
		return FAILED;
	
	if(SUCCESS != rtl8366s_setAsicDot1qAbsolutelyPriority(PRI4, priority.dot1qPri0))
		return FAILED;
	
	if(SUCCESS != rtl8366s_setAsicDot1qAbsolutelyPriority(PRI5, priority.dot1qPri0))
		return FAILED;
	
	if(SUCCESS != rtl8366s_setAsicDot1qAbsolutelyPriority(PRI6, priority.dot1qPri0))
		return FAILED;
	
	if(SUCCESS != rtl8366s_setAsicDot1qAbsolutelyPriority(PRI7, priority.dot1qPri0))
		return FAILED;

	return SUCCESS;
}




/*
@func int32 | rtl8366s_setDscpToMappingPriority | Map dscp value to queue priority.
@parm uint32 | dscp | Dscp value of receiving frame (0~63)
@parm PRIORITYVALUE | priority | Priority mapping to dscp of receiving frame (0~7). 
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	The Differentiated Service Code Point is a selector for router's per-hop behaviours. As a selector, there is no implication that a numerically 
	greater DSCP implies a better network service. As can be seen, the DSCP totally overlaps the old precedence field of TOS. So if values of 
	DSCP are carefully chosen then backward compatibility can be achieved.	
*/
int32 rtl8366s_setDscpToMappingPriority(uint32 dscp, enum PRIORITYVALUE priority)
{
	if(SUCCESS != rtl8366s_setAsicDscpPriority(dscp,priority))
		return FAILED;

	return SUCCESS;	
}

/*
@func int32 | rtl8366s_setPrioritySelection | Configure the priority among different priority mechanism.
@parm uint32 | portpri | Priority assign for port based selection (0~3).
@parm uint32 | dot1qpri | Priority assign for 802.1q selection (0~3).
@parm uint32 | dscppri | Priority assign for dscp selection (0~3).
@parm uint32 | aclpri | Priority assign for acl selection (0~3).
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	ASIC will follow user priority setting of mechanisms to select mapped queue priority for receiving frame. If two priorities of mechanisms are the same,
	ASIC will chose the highest priority from mechanisms to assign queue priority to receiving frame.
*/
int32 rtl8366s_setPrioritySelection(uint32 portpri, uint32 dot1qpri, uint32 dscppri, uint32 aclpri)
{	
	uint32 portpri_pow;
	uint32 dot1qpri_pow;
	uint32 dscppri_pow;
	uint32 acl_pow;
	
	if(portpri > 3 || dot1qpri > 3 || dscppri > 3 || aclpri > 3)
		return FAILED;

	_rtl8366s_powof2(portpri, &portpri_pow);
	_rtl8366s_powof2(dot1qpri, &dot1qpri_pow);
	_rtl8366s_powof2(dscppri, &dscppri_pow);
	_rtl8366s_powof2(aclpri, &acl_pow);

	/*Default ACL priority selection is set to 0 for function disabled setting.*/	
	if(SUCCESS != rtl8366s_setAsicPriorityDecision(portpri_pow, dot1qpri_pow, dscppri_pow, acl_pow))
		return FAILED;

	return SUCCESS;
}




/*
@func int32 | rtl8366s_setPriorityToQueue | Mapping internal priority to queue id.
@parm enum QUEUENUM | qnum | Queue number usage.
@parm pri2Queue_t | qidMapping | Priority mapping to queue configuration.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	ASIC supports queue number setting from 1 to 6. For different queue numbers usage, ASIC supports different internal available  queue IDs.
	Queue number		Available  Queue IDs
		1					0
		2					0,5
		3					0,1,5
		4					0,1,2,5
		5					0,1,2,3,5
		6					0,1,2,3,4,5	
	If software mapping priority to non-available queue IDs in less
*/
int32 rtl8366s_setPriorityToQueue(enum QUEUENUM qnum, pri2Qid_t qidMapping)
{	

	/* Invalid input parameter */
	if ((qnum < QNUM1) || (qnum > QNUM6)) 
		return FAILED;

	if(SUCCESS != rtl8366s_setAsicPriorityToQIDMappingTable(qnum,PRI0,qidMapping.pri0))
		return FAILED;

	if(SUCCESS != rtl8366s_setAsicPriorityToQIDMappingTable(qnum,PRI1,qidMapping.pri1))
		return FAILED;

	if(SUCCESS != rtl8366s_setAsicPriorityToQIDMappingTable(qnum,PRI2,qidMapping.pri2))
		return FAILED;

	if(SUCCESS != rtl8366s_setAsicPriorityToQIDMappingTable(qnum,PRI3,qidMapping.pri3))
		return FAILED;

	if(SUCCESS != rtl8366s_setAsicPriorityToQIDMappingTable(qnum,PRI4,qidMapping.pri4))
		return FAILED;

	if(SUCCESS != rtl8366s_setAsicPriorityToQIDMappingTable(qnum,PRI5,qidMapping.pri5))
		return FAILED;


	return SUCCESS;
}


/*
@func int32 | rtl8366s_setAsicQueueWeight | Set weight and type of queues' in dedicated port.
@parm qConfig_t | queueCfg | Queue type and weigth configuration.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	The API can set weight and type, strict priority or weight fair queue (WFQ) for dedicated port for using queues. If queue id is not included in queue usage,
then its type and weigth setting in dummy for setting. There are priorities as queue id in strick queues. It means strick queue id 5 carrying higher prioirty than
strick queue id 4.
 */
int32 rtl8366s_setQueueWeight(enum PORTID port, qConfig_t queueCfg )
{
	enum QUEUEID qid;

	if(port >=PORT_MAX)
		return FAILED;


	for(qid = QUEUE0;qid<=QUEUE5;qid ++)
	{

		if(queueCfg.weight[qid] == 0 || queueCfg.weight[qid] > QOS_WEIGHT_MAX)
			return FAILED;

		if(SUCCESS != rtl8366s_setAsicQueueWeight(port,qid,queueCfg.strickWfq[qid],queueCfg.weight[qid]-1))
			return FAILED;
	}

	return SUCCESS;
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

	/* Set a default VLAN with vid 1 to 4K table for all ports */
#ifdef CONFIG_BUFFALO
   	vlan4K.vid = 6;
 	vlan4K.member = 0x00; // don't forwarding during initalize.
#else
   	vlan4K.vid = 1;
 	vlan4K.member = RTL8366S_PORTMASK;
#endif // CONFIG_BUFFALO
 	vlan4K.untag = RTL8366S_PORTMASK;
 	vlan4K.fid = 0;	
	if(rtl8366s_setAsicVlan4kEntry(vlan4K) != SUCCESS)
		return FAILED;	

	/* Also set the default VLAN to 16 member configuration index 0 */
	vlanMC.priority = 0;
#ifdef CONFIG_BUFFALO
	vlanMC.vid = 6;
 	vlan4K.member = 0x00; // don't forwarding during initalize.
#else
	vlanMC.vid = 1;
 	vlan4K.member = RTL8366S_PORTMASK;
#endif // CONFIG_BUFFALO
	vlanMC.untag = RTL8366S_PORTMASK;			
	vlanMC.fid = 0;
	if(rtl8366s_setAsicVlanMemberConfig(0, &vlanMC) != SUCCESS)
		return FAILED;	

	/* Set all ports PVID to default VLAN */	
	for(i = 0; i < PORT_MAX; i++)
	{	
#ifdef CONFIG_BUFFALO
		if(rtl8366s_setAsicVlanPortBasedVID(i, RTL8366S_VLANMCIDXMAX) != SUCCESS)
#else
		if(rtl8366s_setAsicVlanPortBasedVID(i, 0) != SUCCESS)
#endif // CONFIG_BUFFALO
			return FAILED;		
	}	

	/* enable VLAN and 4K VLAN */
	if(rtl8366s_setAsicVlan(TRUE)!= SUCCESS)
		return FAILED;	
	if(rtl8366s_setAsicVlan4kTbUsage(TRUE)!= SUCCESS)
		return FAILED;
		
	return SUCCESS;
}

/*
@func int32 | rtl8366s_setVlan | Set a VLAN entry.
@parm uint32 | vid | VLAN ID to configure (0~4095).
@parm uint32 | mbrmsk | VLAN member set portmask (0~0x3F).
@parm uint32 | untagmsk | VLAN untag set portmask (0~0x3F).
@rvalue ERRNO_API_INVALIDPARAM | invalid input parameters.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	There are 4K VLAN entry supported. Member set and untag set	of 4K VLAN entry
	are all zero by default.	User could configure the member	set and untag set
	for specified vid through this API.	The portmask's bit N means port N.
	For example, mbrmask 23=0x17=010111 means port 0,1,2,4 in the member set.
*/
int32 rtl8366s_setVlan(uint32 vid, uint32 mbrmsk, uint32 untagmsk)
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

	/* update 4K table */
   	vlan4K.vid = vid;			
 	vlan4K.member = mbrmsk;
 	vlan4K.untag = untagmsk;
 	vlan4K.fid = 0;	
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
			if(rtl8366s_setAsicVlanMemberConfig(i, &vlanMC) != SUCCESS)
				return FAILED;	

			return SUCCESS;
		}	
	}
	
	return SUCCESS;
}

/*
@func int32 | rtl8366s_getVlan | Get a VLAN entry.
@parm uint32 | vid | VLAN ID to get entry (0~4095).
@parm uint32* | mbrmsk | pointer to returned member set portmask.
@parm uint32* | untagmsk | pointer to returned untag set portmask.
@rvalue ERRNO_API_INVALIDPARAM | invalid input parameters.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	The API can get the member set and untag set settings for specified vid.
*/
int32 rtl8366s_getVlan(uint32 vid, uint32 *mbrmsk, uint32 *untagmsk)
{
	rtl8366s_vlan4kentry vlan4K;
	
	/* vid must be 0~4095 */
	if(vid > RTL8366S_VIDMAX)
		return ERRNO_API_INVALIDPARAM;

	vlan4K.vid = vid;
	if(rtl8366s_getAsicVlan4kEntry(&vlan4K) != SUCCESS)
		return FAILED;

	*mbrmsk = vlan4K.member;
	*untagmsk = vlan4K.untag;	
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
@func int32 | rtl8366s_getVlanPVID | Get VLAN ID(PVID) on specified port.
@parm enum PORTID | port | Port number (0~5).
@parm uint32* | vid | pointer to returned VLAN ID.
@parm uint32* | priority | pointer to returned 802.1p priority.
@rvalue ERRNO_API_INVALIDPARAM | invalid input parameters.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	The API can get the PVID and 802.1p priority for the PVID of Port-based VLAN.
*/
int32 rtl8366s_getVlanPVID(enum PORTID port, uint32 *vid, uint32 *priority)
{
	uint32 index;
	rtl8366s_vlanconfig vlanMC;	

	if(port >= PORT_MAX)
		return ERRNO_API_INVALIDPARAM;
	
	if(rtl8366s_getAsicVlanPortBasedVID(port, &index) != SUCCESS)
		return FAILED;	

	if(rtl8366s_getAsicVlanMemberConfig(index, &vlanMC) != SUCCESS)
		return FAILED;

	*vid = vlanMC.vid;
	*priority = vlanMC.priority;	
	return SUCCESS;
}

/*
@func int32 | rtl8366s_setVlanIngressFilter | Set VLAN ingress for each port.
@parm enum PORTID | port | Physical port number (0~5).
@parm uint32 | enabled | VLAN ingress function 1:enable 0:disable.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	Length/type of 802.1q VLAN tag is 0x8100. While VLAN function is enabled, ASIC will decide VLAN ID for each received frame and get belonged member
	ports from VLAN table. If received port is not belonged to VLAN member ports, ASIC will drop received frame if VLAN ingress function is enabled.
*/
int32 rtl8366s_setVlanIngressFilter(enum PORTID port,uint32 enabled)
{
	if(port >= PORT_MAX)
		return FAILED;

	if(SUCCESS != rtl8366s_setAsicVlanIngressFiltering(port,enabled))
		return FAILED;

	return SUCCESS;
}

/*
@func int32 | rtl8366s_setVlanAcceptFrameType | Set VLAN support frame type
@parm enum PORTID | port | Physical port number (0~5).
@parm FRAMETYPE | type | Supported frame types 0:all 1:C-tage frame 2:un-tag frame
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	Length/type of 802.1q VLAN tag is 0x8100. While VLAN function is enabled, ASIC will parse length/type of reveived frame to check if there is a existed
	C-tag to permit frame or not. This function will *NOT* work while ASIC VLAN function is disabled(ASIC will accept all types of frame).
	Function of accept un-tag frame setting is not adapted to uplink port because only S-tag (the first length/type field is not C-tag) frame will be permitted at uplink port.
	Function of accept C-tag frame setting is not adapted to uplink port and CPU port because frame contained S-tag(0x88a8) or CPU tag (0x8899) will also be dropped.
	User sould take care the side effect of VLAN tag filtering function in the port belongs to CPU port or uplink port. 
*/
int32 rtl8366s_setVlanAcceptFrameType(enum PORTID port, enum FRAMETYPE type)
{
	if(port >= PORT_MAX)
		return FAILED;

	switch(type)
	{
 		case FRAME_TYPE_ALL:
			if(SUCCESS != rtl8366s_setAsicVlanDropTaggedPackets(port,DISABLE))
				return FAILED;
			
			if(SUCCESS != rtl8366s_setAsicVlanAcceptTaggedOnly(port,DISABLE))
				return FAILED;
			break;
 		case FRAME_TYPE_CTAG:
			if(SUCCESS != rtl8366s_setAsicVlanDropTaggedPackets(port,DISABLE))
				return FAILED;
			
			if(SUCCESS != rtl8366s_setAsicVlanAcceptTaggedOnly(port,ENABLE))
				return FAILED;
			break;
 		case FRAME_TYPE_UNTAG:
			if(SUCCESS != rtl8366s_setAsicVlanDropTaggedPackets(port,ENABLE))
				return FAILED;
			
			if(SUCCESS != rtl8366s_setAsicVlanAcceptTaggedOnly(port,DISABLE))
				return FAILED;
			break;
		default:
			return FAILED;
			break;
	}

	return SUCCESS;
}

/*
@func int32 | rtl8366s_addLUTUnicast | Set LUT unicast entry.
@parm uint8 | mac | 6 bytes unicast(I/G bit is 0) mac address to be written into LUT.
@parm enum PORTID | port | Port number to be forwarded to (0~5).
@rvalue ERRNO_API_INVALIDPARAM | invalid input parameters.
@rvalue ERRNO_API_LUTFULL | hashed index is full of entries.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	The LUT has a 4-way entry of an index.
	If the unicast mac address already existed	in the LUT, it will udpate the
	port of the entry. Otherwise, it will find an empty or asic auto learned
	entry to write. If all the four entries can't be replaced, that is, all
	the four entries are added by CPU, it will return a ERRNO_API_LUTFULL error.
*/
int32 rtl8366s_addLUTUnicast(uint8 *mac, enum PORTID port)
{
	uint32 i;
	rtl8366s_l2table l2Table;
		
	/* must be unicast address */
	if((mac == NULL) || (mac[0] & 0x1))
		return ERRNO_API_INVALIDPARAM;

	if(port >= PORT_MAX)
		return ERRNO_API_INVALIDPARAM;	

	/*
	  Scan four-ways to see if the unicast mac address already existed.
	  If the unicast mac address already existed, update the port of the entry.	  
	  Scanning priority of four way is entry 0 > entry 1 > entry 2 > entry 3.	  
	*/
	for(i = 0; i < 4; i++)
	{
		/* fill key(MAC,FID) to get L2 entry */
		_rtl8366s_copy(l2Table.swstatic.mac.octet, mac, ETHER_ADDR_LEN);
		l2Table.swstatic.fid = 0;
	
		if(rtl8366s_getAsicL2LookupTb(i, &l2Table, FALSE) != SUCCESS)
		{					
			return FAILED;
		}	
		else if(_rtl8366s_cmp(mac, l2Table.swstatic.mac.octet, ETHER_ADDR_LEN) == SUCCESS)
		{
			l2Table.swstatic.fid = 0;
			l2Table.swstatic.mbr = (1<<port);
			l2Table.swstatic.auth = FALSE;
			l2Table.swstatic.swst = TRUE;
			l2Table.swstatic.ipmulti = FALSE;
			if(rtl8366s_setAsicL2LookupTb(i, &l2Table, FALSE) != SUCCESS)
				return FAILED;
			else
				return SUCCESS;				
		}
	}

	/*
	  The entry was not written into LUT before, then scan four-ways again to
	  find an empty or asic auto learned entry to write.
	*/
	for(i = 0; i < 4; i++)
	{
		_rtl8366s_copy(l2Table.swstatic.mac.octet, mac, ETHER_ADDR_LEN);
		l2Table.swstatic.fid = 0;			
	
		if(rtl8366s_getAsicL2LookupTb(i, &l2Table, FALSE) != SUCCESS)
		{					
			return FAILED;
		}	
		else if((l2Table.swstatic.swst == FALSE)&&
				 (l2Table.swstatic.auth == FALSE)&&
				 (l2Table.swstatic.ipmulti == FALSE))
		{
			_rtl8366s_copy(l2Table.swstatic.mac.octet, mac, ETHER_ADDR_LEN);
			l2Table.swstatic.fid = 0;		
			l2Table.swstatic.mbr = (1<<port);
			l2Table.swstatic.auth = FALSE;
			l2Table.swstatic.swst = TRUE;
			l2Table.swstatic.ipmulti = FALSE;
			if(rtl8366s_setAsicL2LookupTb(i, &l2Table, FALSE) != SUCCESS)
				return FAILED;
			else
				return SUCCESS;				
		}
	}

	/* four-ways are all full */	
	return ERRNO_API_LUTFULL;
}

/*
@func int32 | rtl8366s_addLUTMulticast | Set LUT multicast entry.
@parm uint8 | mac | 6 bytes multicast(I/G bit isn't 0) mac address to be written into LUT.
@parm uint32 | portmask | Port mask to be forwarded to (0~0x3F).
@rvalue ERRNO_API_INVALIDPARAM | invalid input parameters.
@rvalue ERRNO_API_LUTFULL | hashed index is full of entries.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	The LUT has a 4-way entry of an index.
	If the multicast mac address already existed in the LUT, it will udpate the
	port mask of the entry. Otherwise, it will find an empty or asic auto learned
	entry to write. If all the four entries can't be replaced, that is, all
	the four entries are added by CPU, it will return a ERRNO_API_LUTFULL error.
*/
int32 rtl8366s_addLUTMulticast(uint8 *mac, uint32 portmask)
{
	uint32 i;
	rtl8366s_l2table l2Table;

	/* must be L2 multicast address */
	if((mac == NULL) || (!(mac[0] & 0x1)))
		return ERRNO_API_INVALIDPARAM;

	if(portmask > RTL8366S_PORTMASK)
		return ERRNO_API_INVALIDPARAM;	

	/*
	  Scan four-ways to see if the multicast mac address already existed.
	  If the multicast mac address already existed, update the port mask of the entry.
	  Scanning priority of four way is entry 0 > entry 1 > entry 2 > entry 3.	  
	*/
	for(i = 0; i < 4; i++)
	{
		/* fill key(MAC,FID) to get L2 entry */
		_rtl8366s_copy(l2Table.swstatic.mac.octet, mac, ETHER_ADDR_LEN);
		l2Table.swstatic.fid = 0;			
	
		if(rtl8366s_getAsicL2LookupTb(i, &l2Table, FALSE) != SUCCESS)
		{					
			return FAILED;
		}	
		else if(_rtl8366s_cmp(mac, l2Table.swstatic.mac.octet, ETHER_ADDR_LEN) == SUCCESS)
		{
			l2Table.swstatic.fid = 0;		
			l2Table.swstatic.mbr = portmask;
			l2Table.swstatic.auth = FALSE;
			l2Table.swstatic.swst = TRUE;
			l2Table.swstatic.ipmulti = FALSE;
			if(rtl8366s_setAsicL2LookupTb(i, &l2Table, FALSE) != SUCCESS)
				return FAILED;
			else
				return SUCCESS;				
		}
	}

	/*
	  The entry was not written into LUT before, then scan four-ways again to
	  find an empty or asic auto learned entry to write.
	*/
	for(i = 0; i < 4; i++)
	{
		_rtl8366s_copy(l2Table.swstatic.mac.octet, mac, ETHER_ADDR_LEN);
		l2Table.swstatic.fid = 0;			
	
		if(rtl8366s_getAsicL2LookupTb(i, &l2Table, FALSE) != SUCCESS)
		{					
			return FAILED;
		}	
		else if((l2Table.swstatic.swst == FALSE)&&
				 (l2Table.swstatic.auth == FALSE)&&
				 (l2Table.swstatic.ipmulti == FALSE))
		{
			_rtl8366s_copy(l2Table.swstatic.mac.octet, mac, ETHER_ADDR_LEN);
			l2Table.swstatic.fid = 0;		
			l2Table.swstatic.mbr = portmask;
			l2Table.swstatic.auth = FALSE;
			l2Table.swstatic.swst = TRUE;
			l2Table.swstatic.ipmulti = FALSE;
			if(rtl8366s_setAsicL2LookupTb(i, &l2Table, FALSE) != SUCCESS)
				return FAILED;
			else
				return SUCCESS;				
		}
	}

	/* four-ways are all full */	
	return ERRNO_API_LUTFULL;
}

/*
@func int32 | rtl8366s_delLUTMACAddress | Delete LUT unicast/multicast entry.
@parm uint8 | mac | 6 bytes unicast/multicast mac address to be deleted.
@rvalue ERRNO_API_INVALIDPARAM | invalid input parameters.
@rvalue ERRNO_API_LUTNOTEXIST | No such LUT entry.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	If the mac has existed in the LUT, it will be deleted.
	Otherwise, it will return ERRNO_API_LUTNOTEXIST.
*/
int32 rtl8366s_delLUTMACAddress(uint8 *mac)
{
	uint32 i;
	rtl8366s_l2table l2Table;
		
	if(mac == NULL)
		return ERRNO_API_INVALIDPARAM;

	/* Scan four-ways to find the specified entry */  
	for(i = 0; i < 4; i++)
	{
		/* fill key(MAC,FID) to get L2 entry */
		_rtl8366s_copy(l2Table.swstatic.mac.octet, mac, ETHER_ADDR_LEN);
		l2Table.swstatic.fid = 0;
	
		if(rtl8366s_getAsicL2LookupTb(i, &l2Table, FALSE) != SUCCESS)
		{					
			return FAILED;
		}	
		else if(_rtl8366s_cmp(mac, l2Table.swstatic.mac.octet, ETHER_ADDR_LEN) == SUCCESS &&
				 l2Table.swstatic.swst == TRUE)
		{
			l2Table.swstatic.fid = 0;
			l2Table.swstatic.mbr = 0;
			l2Table.swstatic.auth = FALSE;
			l2Table.swstatic.swst = FALSE;
			l2Table.swstatic.ipmulti = FALSE;
			if(rtl8366s_setAsicL2LookupTb(i, &l2Table, FALSE) != SUCCESS)
				return FAILED;
			else
				return SUCCESS;				
		}
	}

	/* No such LUT entry */	
	return ERRNO_API_LUTNOTEXIST;
}

/*
@func int32 | rtl8366s_getLUTUnicast | Get LUT unicast entry.
@parm uint8 | mac | 6 bytes unicast(I/G bit is 0) mac address to get.
@parm enum PORTID* | port | pointer to returned port number.
@rvalue ERRNO_API_INVALIDPARAM | invalid input parameters.
@rvalue ERRNO_API_LUTNOTEXIST | No such LUT entry.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	If the unicast mac address existed in the LUT, it will return the port 	where
	the mac is learned. Otherwise, it will return a ERRNO_API_LUTNOTEXIST error.
*/
int32 rtl8366s_getLUTUnicast(uint8 *mac, enum PORTID *port)
{
	uint32 i;
	uint32 j;
	rtl8366s_l2table l2Table;
		
	/* must be unicast address */
	if((mac == NULL) || (mac[0] & 0x1))
		return ERRNO_API_INVALIDPARAM;

	/* Scan four-ways to see if the entry existed */
	for(i = 0; i < 4; i++)
	{
		/* fill key(MAC,FID) to get L2 entry */
		_rtl8366s_copy(l2Table.swstatic.mac.octet, mac, ETHER_ADDR_LEN);
		l2Table.swstatic.fid = 0;
	
		if(rtl8366s_getAsicL2LookupTb(i, &l2Table, FALSE) != SUCCESS)
		{					
			return FAILED;
		}	
		else if(_rtl8366s_cmp(mac, l2Table.swstatic.mac.octet, ETHER_ADDR_LEN) == SUCCESS)
		{
			if(TRUE == l2Table.swstatic.swst)
			{
				for(j = 0; j < PORT_MAX; j++)
				{	
					if(l2Table.swstatic.mbr & (1<<j))
					{
						*port = j;
						return SUCCESS;
					}
				}	
			}
			else
			{
				*port = l2Table.autolearn.spa;
				return SUCCESS;
			}
			/* No member port, should never happen */
			return ERRNO_API_LUTNOTEXIST;
		}
	}

	/* Entry not found */	
	return ERRNO_API_LUTNOTEXIST;
}

/*
@func int32 | rtl8366s_setCPUPort | Set CPU port with/without inserting CPU tag.
@parm enum PORTID | port | Port number to be set as CPU port (0~5).
@parm uint32 | noTag | NOT insert Realtek proprietary tag (ethernet length/type 0x8899) to frame 1:not insert 0:insert.
@parm uint32 | dropUnda | NOT forward unknown DMAC frame to CPU port 1:drop 0:forward.
@rvalue ERRNO_API_INVALIDPARAM | invalid input parameters.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	The API can set CPU port and enable/disable inserting proprietary CPU tag (Length/Type 0x8899)
	to the frame that transmitting to CPU port. It also can enable/disable forwarding unknown
	destination MAC address frames to CPU port. User can reduce CPU loading by not forwarding
	unknown DA frames to CPU port.
*/
int32 rtl8366s_setCPUPort(enum PORTID port, uint32 noTag, uint32 dropUnda)
{
	uint32 i;
	
	if(port >= PORT_MAX)
		return ERRNO_API_INVALIDPARAM;

	/* clean CPU port first */
	for(i=0; i<PORT_MAX; i++)
	{
		if(rtl8366s_setAsicCpuPortMask(i, FALSE) != SUCCESS)
			return FAILED;	
	}

	if(rtl8366s_setAsicCpuPortMask(port, TRUE) != SUCCESS)
		return FAILED;		

	if(rtl8366s_setAsicCpuDisableInsTag(noTag) != SUCCESS)
		return FAILED;	

	if(rtl8366s_setAsicCpuDropUnda(dropUnda) != SUCCESS)
		return FAILED;	

	return SUCCESS;
}

/*
@func int32 | rtl8366s_getCPUPort | Get CPU port and its setting.
@parm enum PORTID* | port | returned CPU port (0~5).
@parm uint32* | noTag | returned CPU port with insert tag ability 1:not insert 0:insert.
@parm uint32* | dropUnda | returned CPU port with forward unknown DMAC frame ability 1:drop 0:forward.
@rvalue ERRNO_API_INVALIDPARAM | invalid input parameters.
@rvalue ERRNO_API_CPUNOTSET | No CPU port speicifed.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	The API can get configured CPU port and its setting.
	Return ERRNO_API_CPUNOTSET if CPU port is not speicifed.
*/
int32 rtl8366s_getCPUPort(enum PORTID *port, uint32 *noTag, uint32 *dropUnda)
{
	uint32 i;
	uint32 enable = FALSE;	
	
	if(port == NULL || noTag == NULL || dropUnda == NULL)
		return ERRNO_API_INVALIDPARAM;

	/* get configured CPU port */
	for(i=0; i<PORT_MAX; i++)
	{
		if(rtl8366s_getAsicCpuPortMask(i, &enable) != SUCCESS)
			return FAILED;	

		if(enable == TRUE)
		{
			*port = i;
			if(rtl8366s_getAsicCpuDisableInsTag(noTag) != SUCCESS)
				return FAILED;				
			if(rtl8366s_getAsicCpuDropUnda(dropUnda) != SUCCESS)
				return FAILED;				
			
			return SUCCESS;
		}
	}

	return ERRNO_API_CPUNOTSET;
}

/*
@func int32 | rtl8366s_initChip | Set chip to default configuration enviroment
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
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

	rtl8366s_setAsicMaxLengthInRx(3);
	return SUCCESS;
}

/*
@func int32 | rtl8366s_setLedMode | Set Led to congiuration mode
@parm enum LED_MODE | mode | Support mode0,mode1,mode2,mode3 and force mode only.
@parm uint32 | ledG0Msk | Turn on or turn off Leds of group 0, 1:on 0:off.
@parm uint32 | ledG1Msk | Turn on or turn off Leds of group 1, 1:on 0:off.
@parm uint32 | ledG2Msk | Turn on or turn off Leds of group 2, 1:on 0:off.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	While led mode was set to FORCE, user can force turn on or turn off dedicted leds of each port. There are three LEDs for each port for indicating information
	about dedicated port. LEDs is set to indicate different information of port in different mode.

	Mode0
		LED0-Link, Activity Indicator. Low for link established. Link/Act Blinks every 43ms when the corresponding port is transmitting or receiving.
		LED1-1000Mb/s Speed Indicator. Low for 1000Mb/s.
		LED2-100Mb/s Speed Indicator. Low for 100Mb/s.

	Mode1
		LED0-Link, Activity Indicator. Low for link established. Link/Act Blinks every 43ms when the corresponding port is transmitting or receiving.
		LED1-1000Mb/s Speed Indicator. Low for 1000Mb/s.
		LED2-Collision, Full duplex Indicator. Blinking every 43ms when collision happens. Low for full duplex, and high for half duplex mode.

	Mode2
		LED0-Collision, Full duplex Indicator. Blinking every 43ms when collision happens. Low for full duplex, and high for half duplex mode.
		LED1-1000Mb/s Speed/Activity Indicator. Low for 1000Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
		LED2-10/100Mb/s Speed/Activity Indicator. Low for 10/100Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.

	Mode3
		LED0-10Mb/s Speed/Activity Indicator. Low for 10Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
		LED1-1000Mb/s Speed/Activity Indicator. Low for 1000Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.
		LED2-100Mb/s Speed/Activity Indicator. Low for 100Mb/s. Blinks every 43ms when the corresponding port is transmitting or receiving.

	Force: All Leds can be turned on or off by software configuration.
	
	
*/
int32 rtl8366s_setLedMode(enum LED_MODE mode, uint32 ledG0Msk, uint32 ledG1Msk, uint32 ledG2Msk)
{
	uint16 ledGroup;
	const uint32 ledConfig[LED_MODE_MAX][RTL8366S_LED_GROUP_MAX] = { 
					{LEDCONF_LINK_ACT,		LEDCONF_SPD1000,		LEDCONF_SPD100,			LEDCONF_SPD10},
					{LEDCONF_LINK_ACT,		LEDCONF_SPD1000,		LEDCONF_DUPCOL,		LEDCONF_SPD100},
					{LEDCONF_DUPCOL,		LEDCONF_SPD1000ACT,	LEDCONF_SPD10010ACT,	LEDCONF_LEDOFF},
					{LEDCONF_SPD10ACT,		LEDCONF_SPD1000ACT,	LEDCONF_SPD100ACT,		LEDCONF_DUPCOL},
					{LEDCONF_LEDFORCE,		LEDCONF_LEDFORCE,		LEDCONF_LEDFORCE,		LEDCONF_LEDFORCE}
					};
		

	if(mode >= LED_MODE_MAX)
		return FAILED;


	for(ledGroup= 0;ledGroup<RTL8366S_LED_GROUP_MAX;ledGroup++)
	{
		if(SUCCESS != rtl8366s_setAsicLedIndicateInfoConfig(ledGroup,ledConfig[mode][ledGroup]))
			return FAILED;
	}

	if(mode != LED_MODE_FORCE)
		return SUCCESS;

	if(SUCCESS != rtl8366s_setAsicForceLeds(ledG0Msk,ledG1Msk,ledG2Msk,0))
		return FAILED;

	return SUCCESS;
}


/*
@func int32 | rtl8366s_setLedBlinkRate | Set LED blinking rate at mode 0 to mode 3
@parm enum RTL8366S_LEDBLINKRATE | blinkRate | Support 6 blinking rate.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	ASIC support 6 types of LED blinking rates at 43ms, 84ms, 120ms, 170ms, 340ms and 670ms.
*/
int32 rtl8366s_setLedBlinkRate(enum RTL8366S_LEDBLINKRATE blinkRate)
{
	if(blinkRate >=LEDBLINKRATE_MAX)
		return FAILED;

	if(SUCCESS != rtl8366s_setAsicLedBlinkRate(blinkRate))
		return FAILED;

	return SUCCESS;
}

/*
@func int32 | rtl8366s_setReservedMulticastAddress | Set reserved multicast address frame trap to CPU.
@parm enum RMATRAPFRAME | rma | RMA for trapping to CPU.
@parm uint32 | enabled | Qos function usage 1:enable 0:disable.
@rvalue ERRNO_INVALIDINPUT | Invalid input parameter.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	There are 9 types of Reserved Multicast Address frame for trapping to CPU for application usage. They are as following definition.
	Assignment											Address 
	Bridge Group Address									01-80-C2-00-00-00	
	IEEE Std 802.3, 1988 Edition, Full Duplex PAUSE operation		01-80-C2-00-00-01	
	IEEE Std 802.3ad Slow Protocols-Multicast address			01-80-C2-00-00-02	
	IEEE Std 802.1X PAE address							01-80-C2-00-00-03	
	All LANs Bridge Management Group Address				01-80-C2-00-00-10	
	GMRP Address											01-80-C2-00-00-20	
	GVRP address											01-80-C2-00-00-21	
	Undefined 802.1 bridge address							01-80-C2-00-00-04 ~ 01-80-C2-00-00-0F
	Undefined GARP address								01-80-C2-00-00-22 ~ 01-80-C2-00-00-2F
*/
int32 rtl8366s_setReservedMulticastAddress(enum RMATRAPFRAME rma, uint32 enabled)
{

	if(rma < RMA_BRG_GROUP || rma > RMA_UNDEF_GARP)
		return ERRNO_INVALIDINPUT;

	if(SUCCESS != rtl8366s_setAsicRma(rma,enabled))
		return FAILED;		

	return SUCCESS;
}

/*
@func int32 | rtl8366s_setStormFiltering | Set per port  storm filtering function.
@parm enum PORTID | port | Physical Port number.
@parm uint32 | bcstorm | broadcasting storm filtering 1:enable, 0:disabled.
@parm uint32 | mcstorm | multicasting storm filtering 1:enable, 0:disabled.
@parm uint32 | undastorm | unknown destination storm filtering 1:enable, 0:disabled.
@parm enum SFC_PERIOD | period | storm filtering reference time.
@parm enum SFC_COUNT | counter | storm filtering pakcet number threshold.
@rvalue ERRNO_API_INVALIDPARAM | Invalid Port Number.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	The API can be used to enable or disable per-port storm filtering control. There are three kinds of storm filtering control which are broadcasting storm control,
	multicast storm control and unknown unicast storm control. There are dedicated storm control setting for each port and global storm control meter for each one
	reference. There is still a didicated reference meter counter in each port.
*/
int32 rtl8366s_setStormFiltering(enum PORTID port, uint32 bcstorm, uint32 mcstorm, uint32 undastorm,enum SFC_PERIOD period, enum SFC_COUNT counter)
{
	/*uint32 retVal;
	uint32 bitNo;
	uint32 regData;
	uint32 regBits;*/

	if(port >=PORT_MAX)
		return ERRNO_API_INVALIDPARAM;
	
	if(period > SFC_PERIOD_3600MS)
		return ERRNO_SC_INVALIDPERIOD;

	if(counter > RTL8366S_SCCOUNTMAX)
		return SFC_COUNT_255PKTS;

	/*broadcasting storm filtering control*/
	if(SUCCESS != rtl8366s_setAsicRegBit(RTL8366S_STORM_FILTERING_1_REG,RTL8366S_STORM_FILTERING_BC_BIT + port,bcstorm))
		return FAILED;

	/*multicasting storm filtering control*/
	if(SUCCESS != rtl8366s_setAsicRegBit(RTL8366S_STORM_FILTERING_2_REG,RTL8366S_STORM_FILTERING_MC_BIT + port,mcstorm))
		return FAILED;

	/*unknown destination storm filtering control*/
	if(SUCCESS != rtl8366s_setAsicRegBit(RTL8366S_STORM_FILTERING_2_REG,RTL8366S_STORM_FILTERING_UNDA_BIT + port,undastorm))
		return FAILED;

	/*period*/
	if(SUCCESS != rtl8366s_setAsicRegBits(RTL8366S_STORM_FILTERING_1_REG,RTL8366S_STORM_FILTERING_PERIOD_MSK,period<<RTL8366S_STORM_FILTERING_PERIOD_BIT))
		return FAILED;

	/*count*/
	if(SUCCESS != rtl8366s_setAsicRegBits(RTL8366S_STORM_FILTERING_1_REG,RTL8366S_STORM_FILTERING_COUNT_MSK,counter<<RTL8366S_STORM_FILTERING_COUNT_BIT))
		return FAILED;
		

	return SUCCESS;
}

/*
@func int32 | rtl8366s_setPortMirror | Configure port mirror function.
@parm portMirror_t | portMirror | Port mirror configuration.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	ASIC supports one port mirror function only. For mirror function, there are one mirrored port and one monitor port. Frame from/to source port
	can be mirrored to monitor port and mirror port can be set with receiving frame from source port only. Mirror configuration is as below description.
	Field					Description
	sourcePort			Desired source port which receiving/transmitting frame to be mirrored 
	monitorPort			Monitor port which frames were mirrored to.
	mirrorRx				Function setting of mirroring frames received in source port.
	mirrorTx				Funciton setting of mirroring frames transmitted from source port.
	mirrorIso				Setting which permit forwarding frame from source port only.
*/
int32 rtl8366s_setPortMirror(portMirror_t portMirror)
{
	if(SUCCESS != rtl8366s_setAsicPortMirroring(portMirror.sourcePort,portMirror.monitorPort))
		return FAILED;		

	if(SUCCESS != rtl8366s_setAsicMirroredPortRxMirror(portMirror.mirrorRx))
		return FAILED;		

	if(SUCCESS != rtl8366s_setAsicMirroredPortTxMirror(portMirror.mirrorTx))
		return FAILED;		

	if(SUCCESS != rtl8366s_setAsicMinitorPortIsolation(portMirror.mirrorIso))
		return FAILED;		

	return SUCCESS;

}
/*
@func int32 | rtl8366s_initAcl | ACL function initialization.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	Set ACL function disable at each port. There are shared 32 rules and 12 kinds of rule format  {mac(0), ipv4(1), ipv4 icmp(2), ipv4 igmp(3), ipv4 tcp(4), 
	ipv4 udp(5), ipv6 sip(6), ipv6 dip(7), ipv6 ext(8), ipv6 tcp(9), ipv6 udp(10), ipv6 icmp(11)} for ingress filtering. Using ACL matched action fields combination 
	of mirror/redirect and port mask to PERMIT, DROP, REDIRECT and MIRROR packets which matching ACL rule.
*/
int32 rtl8366s_initAcl(void)
{
	enum PORTID port;
	const uint16 startEndDefault[6]= {0,6,12,18,24,30};

	for(port = PORT0;port<PORT_MAX;port ++)
	{
		/*disable ACL function in default*/
		if(SUCCESS != rtl8366s_setAsicAcl(port,ACL_DEFAULT_ABILITY))
			return FAILED;
		/*set unmatched frame to permit*/
		if(SUCCESS != rtl8366s_setAsicAclUnmatchedPermit(port,ACL_DEFAULT_UNMATCH_PERMIT))
			return FAILED;
		/*allocated shared ACL space in default range*/
		if(SUCCESS != rtl8366s_setAsicAclStartEnd(port,startEndDefault[port],startEndDefault[port]))
			return FAILED;
	}

	return SUCCESS;
}

int32 _rtl8366s_getAclUsedRuleNum(uint32 *ruleNum)
{
	enum PORTID port; 
	int16 aclUsage[RTL8366S_ACLRULENO];
	int16 aclIdx;
	uint32 aclStart;
	uint32 aclEnd;
	uint32 enabled;
	uint32 aclActiveNo;

	for(aclIdx = 0;aclIdx <=RTL8366S_ACLINDEXMAX;aclIdx ++)
		aclUsage[aclIdx] = 0;

	for(port = PORT0; port < PORT_MAX;port ++)
	{
		rtl8366s_getAsicAclStartEnd(port,&aclStart,&aclEnd);

		rtl8366s_getAsicAcl(port,&enabled);

		/*calculate ACL rule usage number at ACL function filtering enable situation*/	
		if(enabled)
		{
			aclIdx = aclStart;
			aclUsage[aclIdx] = 1;
			while(aclIdx != aclEnd)
			{	
				aclIdx ++;
				if(aclIdx >RTL8366S_ACLINDEXMAX)
					aclIdx = 0;

				aclUsage[aclIdx] = 1;
			}
		}
	}


	for(aclIdx = 0,aclActiveNo = 0;aclIdx <=RTL8366S_ACLINDEXMAX;aclIdx ++)
	{
		if(aclUsage[aclIdx])
			aclActiveNo = aclActiveNo + 1;
	}

	*ruleNum  = aclActiveNo;	

	return SUCCESS;

}

int32 _rtl8366s_getAclRuleUsage(uint32 ruleIdx, uint32* free)
{
	enum PORTID port;
	int16 aclUsage[RTL8366S_ACLRULENO];
	int16 aclIdx;
	uint32 aclStart;
	uint32 aclEnd;
	uint32 enabled;

	for(aclIdx = 0;aclIdx <=RTL8366S_ACLINDEXMAX;aclIdx ++)
		aclUsage[aclIdx] = 0;

	for(port = PORT0; port < PORT_MAX;port ++)
	{
		if(SUCCESS != rtl8366s_getAsicAclStartEnd(port,&aclStart,&aclEnd))
			return FAILED;

		if(SUCCESS != rtl8366s_getAsicAcl(port,&enabled))
			return FAILED;
		/*calculate ACL rule usage number at ACL function filtering enable situation*/	
		if(enabled)
		{

			aclIdx = aclStart;
			aclUsage[aclIdx] = 1;
			while(aclIdx != aclEnd)
			{	
				aclIdx ++;
				if(aclIdx >RTL8366S_ACLINDEXMAX)
					aclIdx = 0;

				aclUsage[aclIdx] = 1;
			}
		}
	}

	if(aclUsage[ruleIdx])
		*free = DISABLE;
	else
		*free = ENABLE;
	
	
	
	return SUCCESS;

}


int32 _rtl8366s_getAclFreeIndex(uint32* ruleIdx)
{
	enum PORTID port;
	int16 aclUsage[RTL8366S_ACLRULENO];
	int16 aclIdx;
	uint32 aclStart;
	uint32 aclEnd;
	uint32 enabled;

	for(aclIdx = 0;aclIdx <=RTL8366S_ACLINDEXMAX;aclIdx ++)
		aclUsage[aclIdx] = ACL_RULE_FREE;

	for(port = PORT0; port < PORT_MAX;port ++)
	{
		if(SUCCESS != rtl8366s_getAsicAclStartEnd(port,&aclStart,&aclEnd))
			return FAILED;

		if(SUCCESS != rtl8366s_getAsicAcl(port,&enabled))
			return FAILED;
		/*calculate ACL rule usage number at ACL function filtering enable situation*/	
		if(enabled)
		{

			aclIdx = aclStart;
			aclUsage[aclIdx] = ACL_RULE_INAVAILABLE;
			while(aclIdx != aclEnd)
			{	
				aclIdx ++;
				if(aclIdx >RTL8366S_ACLINDEXMAX)
					aclIdx = 0;

				aclUsage[aclIdx] = ACL_RULE_INAVAILABLE;
			}
		}
	}

	aclIdx = *ruleIdx;
	
	if(ACL_RULE_FREE == aclUsage[aclIdx])
	{
		return SUCCESS;
	}

	if(aclIdx == RTL8366S_ACLINDEXMAX)
		aclIdx = 0;
	else
		aclIdx = aclIdx + 1;

	/*get nearest free rule index*/
	while(aclIdx != *ruleIdx)
	{
		if(ACL_RULE_FREE == aclUsage[aclIdx])
		{
			*ruleIdx  = aclIdx;
			return SUCCESS;
		}

		if(aclIdx == RTL8366S_ACLINDEXMAX)
			aclIdx = 0;
		else
			aclIdx = aclIdx + 1;
	}
	
#if 0
	for(aclIdx = 0;aclIdx <=RTL8366S_ACLINDEXMAX;aclIdx ++)
	{
		if(0 == aclUsage[aclIdx])
		{
			*ruleIdx  = aclIdx;
			return SUCCESS;
		}
		
	}
#endif

	return FAILED;

}






int32 _rtl8366s_cmpAclRule( rtl8366s_acltable *aclSrc, rtl8366s_acltable *aclCmp)
{


	switch(aclSrc->format)
	{
	 case ACL_MAC:
		if(aclSrc->rule.mac.dmp.octet[0] != aclCmp->rule.mac.dmp.octet[0] ||
			aclSrc->rule.mac.dmp.octet[1] != aclCmp->rule.mac.dmp.octet[1] ||
			aclSrc->rule.mac.dmp.octet[2] != aclCmp->rule.mac.dmp.octet[2] ||
			aclSrc->rule.mac.dmp.octet[3] != aclCmp->rule.mac.dmp.octet[3] ||
			aclSrc->rule.mac.dmp.octet[4] != aclCmp->rule.mac.dmp.octet[4] ||
			aclSrc->rule.mac.dmp.octet[5] != aclCmp->rule.mac.dmp.octet[5])
			return FAILED;

		if(aclSrc->rule.mac.dmm.octet[0] != aclCmp->rule.mac.dmm.octet[0] ||
			aclSrc->rule.mac.dmm.octet[1] != aclCmp->rule.mac.dmm.octet[1] ||
			aclSrc->rule.mac.dmm.octet[2] != aclCmp->rule.mac.dmm.octet[2] ||
			aclSrc->rule.mac.dmm.octet[3] != aclCmp->rule.mac.dmm.octet[3] ||
			aclSrc->rule.mac.dmm.octet[4] != aclCmp->rule.mac.dmm.octet[4] ||
			aclSrc->rule.mac.dmm.octet[5] != aclCmp->rule.mac.dmm.octet[5])
			return FAILED;

		if(aclSrc->rule.mac.smp.octet[0] != aclCmp->rule.mac.smp.octet[0] ||
			aclSrc->rule.mac.smp.octet[1] != aclCmp->rule.mac.smp.octet[1] ||
			aclSrc->rule.mac.smp.octet[2] != aclCmp->rule.mac.smp.octet[2] ||
			aclSrc->rule.mac.smp.octet[3] != aclCmp->rule.mac.smp.octet[3] ||
			aclSrc->rule.mac.smp.octet[4] != aclCmp->rule.mac.smp.octet[4] ||
			aclSrc->rule.mac.smp.octet[5] != aclCmp->rule.mac.smp.octet[5])
			return FAILED;

		if(aclSrc->rule.mac.smm.octet[0] != aclCmp->rule.mac.smm.octet[0] ||
			aclSrc->rule.mac.smm.octet[1] != aclCmp->rule.mac.smm.octet[1] ||
			aclSrc->rule.mac.smm.octet[2] != aclCmp->rule.mac.smm.octet[2] ||
			aclSrc->rule.mac.smm.octet[3] != aclCmp->rule.mac.smm.octet[3] ||
			aclSrc->rule.mac.smm.octet[4] != aclCmp->rule.mac.smm.octet[4] ||
			aclSrc->rule.mac.smm.octet[5] != aclCmp->rule.mac.smm.octet[5])
			return FAILED;

		if(aclSrc->rule.mac.tlu != aclCmp->rule.mac.tlu || aclSrc->rule.mac.tll != aclCmp->rule.mac.tll )
			return FAILED;
		
		if(aclSrc->rule.mac.vtd != aclCmp->rule.mac.vtd || aclSrc->rule.mac.vtm != aclCmp->rule.mac.vtm)
			return FAILED;

		if(aclSrc->rule.mac.pu != aclCmp->rule.mac.pu || aclSrc->rule.mac.pl != aclCmp->rule.mac.pl)
			return FAILED;

		if(aclSrc->rule.mac.vidd != aclCmp->rule.mac.vidd || aclSrc->rule.mac.vidm != aclCmp->rule.mac.vidm)
			return FAILED;

		break;
	case ACL_IPV4:

		if(aclSrc->rule.ipv4.sipU != aclCmp->rule.ipv4.sipU || aclSrc->rule.ipv4.sipL != aclCmp->rule.ipv4.sipL)
			return FAILED;

		if(aclSrc->rule.ipv4.dipU != aclCmp->rule.ipv4.dipU || aclSrc->rule.ipv4.dipL != aclCmp->rule.ipv4.dipL)
			return FAILED;

		if(aclSrc->rule.ipv4.tosD != aclCmp->rule.ipv4.tosD || aclSrc->rule.ipv4.tosM != aclCmp->rule.ipv4.tosM)
			return FAILED;

		if(aclSrc->rule.ipv4.protoV != aclCmp->rule.ipv4.protoV || 
			aclSrc->rule.ipv4.proto1 != aclCmp->rule.ipv4.proto1 ||
			aclSrc->rule.ipv4.proto2 != aclCmp->rule.ipv4.proto2 ||
			aclSrc->rule.ipv4.proto3 != aclCmp->rule.ipv4.proto3 ||
			aclSrc->rule.ipv4.proto4 != aclCmp->rule.ipv4.proto4)
			return FAILED;
		
		if(aclSrc->rule.ipv4.flagD != aclCmp->rule.ipv4.flagD || aclSrc->rule.ipv4.flagM != aclCmp->rule.ipv4.flagM)
			return FAILED;

		if(aclSrc->rule.ipv4.offU != aclCmp->rule.ipv4.offU || aclSrc->rule.ipv4.offL != aclCmp->rule.ipv4.offL)
			return FAILED;
		
		break;		

	case ACL_IPV4_ICMP:
		if(aclSrc->rule.ipv4icmp.sipU != aclCmp->rule.ipv4icmp.sipU || aclSrc->rule.ipv4icmp.sipL != aclCmp->rule.ipv4icmp.sipL)
			return FAILED;

		if(aclSrc->rule.ipv4icmp.dipU != aclCmp->rule.ipv4icmp.dipU || aclSrc->rule.ipv4icmp.dipL != aclCmp->rule.ipv4icmp.dipL)
			return FAILED;

		if(aclSrc->rule.ipv4icmp.tosD != aclCmp->rule.ipv4icmp.tosD || aclSrc->rule.ipv4icmp.tosM != aclCmp->rule.ipv4icmp.tosM)
			return FAILED;

		if(aclSrc->rule.ipv4icmp.typeV != aclCmp->rule.ipv4icmp.typeV || 
			aclSrc->rule.ipv4icmp.type1 != aclCmp->rule.ipv4icmp.type1 ||
			aclSrc->rule.ipv4icmp.type2 != aclCmp->rule.ipv4icmp.type2 ||
			aclSrc->rule.ipv4icmp.type3 != aclCmp->rule.ipv4icmp.type3 ||
			aclSrc->rule.ipv4icmp.type4 != aclCmp->rule.ipv4icmp.type4)
			return FAILED;

		if(aclSrc->rule.ipv4icmp.codeV != aclCmp->rule.ipv4icmp.codeV || 
			aclSrc->rule.ipv4icmp.code1 != aclCmp->rule.ipv4icmp.code1 ||
			aclSrc->rule.ipv4icmp.code2 != aclCmp->rule.ipv4icmp.code2 ||
			aclSrc->rule.ipv4icmp.code3 != aclCmp->rule.ipv4icmp.code3 ||
			aclSrc->rule.ipv4icmp.code4 != aclCmp->rule.ipv4icmp.code4)
			return FAILED;

		break;

	case ACL_IPV4_IGMP:
		if(aclSrc->rule.ipv4igmp.sipU != aclCmp->rule.ipv4igmp.sipU || aclSrc->rule.ipv4igmp.sipL != aclCmp->rule.ipv4igmp.sipL)
			return FAILED;

		if(aclSrc->rule.ipv4igmp.dipU != aclCmp->rule.ipv4igmp.dipU || aclSrc->rule.ipv4igmp.dipL != aclCmp->rule.ipv4igmp.dipL)
			return FAILED;

		if(aclSrc->rule.ipv4igmp.tosD != aclCmp->rule.ipv4igmp.tosD || aclSrc->rule.ipv4igmp.tosM != aclCmp->rule.ipv4igmp.tosM)
			return FAILED;

		if(aclSrc->rule.ipv4igmp.typeV != aclCmp->rule.ipv4igmp.typeV || 
			aclSrc->rule.ipv4igmp.type1 != aclCmp->rule.ipv4igmp.type1 ||
			aclSrc->rule.ipv4igmp.type2 != aclCmp->rule.ipv4igmp.type2 ||
			aclSrc->rule.ipv4igmp.type3 != aclCmp->rule.ipv4igmp.type3 ||
			aclSrc->rule.ipv4igmp.type4 != aclCmp->rule.ipv4igmp.type4)
			return FAILED;

		break;
		
	case ACL_IPV4_TCP:
		if(aclSrc->rule.ipv4tcp.sipU != aclCmp->rule.ipv4tcp.sipU || aclSrc->rule.ipv4tcp.sipL != aclCmp->rule.ipv4tcp.sipL)
			return FAILED;

		if(aclSrc->rule.ipv4tcp.dipU != aclCmp->rule.ipv4tcp.dipU || aclSrc->rule.ipv4tcp.dipL != aclCmp->rule.ipv4tcp.dipL)
			return FAILED;

		if(aclSrc->rule.ipv4tcp.tosD != aclCmp->rule.ipv4tcp.tosD || aclSrc->rule.ipv4tcp.tosM != aclCmp->rule.ipv4tcp.tosM)
			return FAILED;

		if(aclSrc->rule.ipv4tcp.sPortU != aclCmp->rule.ipv4tcp.sPortU || aclSrc->rule.ipv4tcp.sPortL != aclCmp->rule.ipv4tcp.sPortL)
			return FAILED;

		if(aclSrc->rule.ipv4tcp.dPortU != aclCmp->rule.ipv4tcp.dPortU || aclSrc->rule.ipv4tcp.dPortL != aclCmp->rule.ipv4tcp.dPortL)
			return FAILED;

		if(aclSrc->rule.ipv4tcp.flagD != aclCmp->rule.ipv4tcp.flagD || aclSrc->rule.ipv4tcp.flagM != aclCmp->rule.ipv4tcp.flagM)
			return FAILED;
		
		break;
		
	case ACL_IPV4_UDP:
		if(aclSrc->rule.ipv4udp.sipU != aclCmp->rule.ipv4udp.sipU || aclSrc->rule.ipv4udp.sipL != aclCmp->rule.ipv4udp.sipL)
			return FAILED;

		if(aclSrc->rule.ipv4udp.dipU != aclCmp->rule.ipv4udp.dipU || aclSrc->rule.ipv4udp.dipL != aclCmp->rule.ipv4udp.dipL)
			return FAILED;

		if(aclSrc->rule.ipv4udp.tosD != aclCmp->rule.ipv4udp.tosD || aclSrc->rule.ipv4udp.tosM != aclCmp->rule.ipv4udp.tosM)
			return FAILED;

		if(aclSrc->rule.ipv4udp.sPortU != aclCmp->rule.ipv4udp.sPortU || aclSrc->rule.ipv4udp.sPortL != aclCmp->rule.ipv4udp.sPortL)
			return FAILED;

		if(aclSrc->rule.ipv4udp.dPortU != aclCmp->rule.ipv4udp.dPortU || aclSrc->rule.ipv4udp.dPortL != aclCmp->rule.ipv4udp.dPortL)
			return FAILED;

		break;
	case ACL_IPV6_SIP:
		if(aclSrc->rule.ipv6sip.sipU[0] != aclCmp->rule.ipv6sip.sipU[0] ||
			aclSrc->rule.ipv6sip.sipU[1] != aclCmp->rule.ipv6sip.sipU[1] ||
			aclSrc->rule.ipv6sip.sipU[2] != aclCmp->rule.ipv6sip.sipU[2] ||
			aclSrc->rule.ipv6sip.sipU[3] != aclCmp->rule.ipv6sip.sipU[3] ||
			aclSrc->rule.ipv6sip.sipU[4] != aclCmp->rule.ipv6sip.sipU[4] ||
			aclSrc->rule.ipv6sip.sipU[5] != aclCmp->rule.ipv6sip.sipU[5] ||
			aclSrc->rule.ipv6sip.sipU[6] != aclCmp->rule.ipv6sip.sipU[6] ||
			aclSrc->rule.ipv6sip.sipU[7] != aclCmp->rule.ipv6sip.sipU[7] )
			return FAILED;
			
		if(aclSrc->rule.ipv6sip.sipL[0] != aclCmp->rule.ipv6sip.sipL[0] ||
			aclSrc->rule.ipv6sip.sipL[1] != aclCmp->rule.ipv6sip.sipL[1] ||
			aclSrc->rule.ipv6sip.sipL[2] != aclCmp->rule.ipv6sip.sipL[2] ||
			aclSrc->rule.ipv6sip.sipL[3] != aclCmp->rule.ipv6sip.sipL[3] ||
			aclSrc->rule.ipv6sip.sipL[4] != aclCmp->rule.ipv6sip.sipL[4] ||
			aclSrc->rule.ipv6sip.sipL[5] != aclCmp->rule.ipv6sip.sipL[5] ||
			aclSrc->rule.ipv6sip.sipL[6] != aclCmp->rule.ipv6sip.sipL[6] ||
			aclSrc->rule.ipv6sip.sipL[7] != aclCmp->rule.ipv6sip.sipL[7] )
			return FAILED;
		
		break;
	case ACL_IPV6_DIP:
		if(aclSrc->rule.ipv6dip.dipU[0] != aclCmp->rule.ipv6dip.dipU[0] ||
			aclSrc->rule.ipv6dip.dipU[1] != aclCmp->rule.ipv6dip.dipU[1] ||
			aclSrc->rule.ipv6dip.dipU[2] != aclCmp->rule.ipv6dip.dipU[2] ||
			aclSrc->rule.ipv6dip.dipU[3] != aclCmp->rule.ipv6dip.dipU[3] ||
			aclSrc->rule.ipv6dip.dipU[4] != aclCmp->rule.ipv6dip.dipU[4] ||
			aclSrc->rule.ipv6dip.dipU[5] != aclCmp->rule.ipv6dip.dipU[5] ||
			aclSrc->rule.ipv6dip.dipU[6] != aclCmp->rule.ipv6dip.dipU[6] ||
			aclSrc->rule.ipv6dip.dipU[7] != aclCmp->rule.ipv6dip.dipU[7] )
			return FAILED;
			
		if(aclSrc->rule.ipv6dip.dipL[0] != aclCmp->rule.ipv6dip.dipL[0] ||
			aclSrc->rule.ipv6dip.dipL[1] != aclCmp->rule.ipv6dip.dipL[1] ||
			aclSrc->rule.ipv6dip.dipL[2] != aclCmp->rule.ipv6dip.dipL[2] ||
			aclSrc->rule.ipv6dip.dipL[3] != aclCmp->rule.ipv6dip.dipL[3] ||
			aclSrc->rule.ipv6dip.dipL[4] != aclCmp->rule.ipv6dip.dipL[4] ||
			aclSrc->rule.ipv6dip.dipL[5] != aclCmp->rule.ipv6dip.dipL[5] ||
			aclSrc->rule.ipv6dip.dipL[6] != aclCmp->rule.ipv6dip.dipL[6] ||
			aclSrc->rule.ipv6dip.dipL[7] != aclCmp->rule.ipv6dip.dipL[7] )
			return FAILED;

		break;
	case ACL_IPV6_EXT:
		if(aclSrc->rule.ipv6ext.tcD != aclCmp->rule.ipv6ext.tcD || aclSrc->rule.ipv6ext.tcM != aclCmp->rule.ipv6ext.tcM)
			return FAILED;

		if(aclSrc->rule.ipv6ext.nhV != aclCmp->rule.ipv6ext.nhV || 
			aclSrc->rule.ipv6ext.nhp1 != aclCmp->rule.ipv6ext.nhp1 ||
			aclSrc->rule.ipv6ext.nhp2 != aclCmp->rule.ipv6ext.nhp2 ||
			aclSrc->rule.ipv6ext.nhp3 != aclCmp->rule.ipv6ext.nhp3 ||
			aclSrc->rule.ipv6ext.nhp4 != aclCmp->rule.ipv6ext.nhp4)
			return FAILED;

		break;
	case ACL_IPV6_TCP:
		if(aclSrc->rule.ipv6tcp.tcD != aclCmp->rule.ipv6tcp.tcD || aclSrc->rule.ipv6tcp.tcM != aclCmp->rule.ipv6tcp.tcM)
			return FAILED;

		if(aclSrc->rule.ipv6tcp.sPortU != aclCmp->rule.ipv6tcp.sPortU || aclSrc->rule.ipv6tcp.sPortL != aclCmp->rule.ipv6tcp.sPortL)
			return FAILED;

		if(aclSrc->rule.ipv6tcp.dPortU != aclCmp->rule.ipv6tcp.dPortU || aclSrc->rule.ipv6tcp.dPortL != aclCmp->rule.ipv6tcp.dPortL)
			return FAILED;

		if(aclSrc->rule.ipv6tcp.flagD != aclCmp->rule.ipv6tcp.flagD || aclSrc->rule.ipv6tcp.flagM != aclCmp->rule.ipv6tcp.flagM)
			return FAILED;

		break;
	case ACL_IPV6_UDP:
		if(aclSrc->rule.ipv6udp.tcD != aclCmp->rule.ipv6udp.tcD || aclSrc->rule.ipv6udp.tcM != aclCmp->rule.ipv6udp.tcM)
			return FAILED;

		if(aclSrc->rule.ipv6udp.sPortU != aclCmp->rule.ipv6udp.sPortU || aclSrc->rule.ipv6udp.sPortL != aclCmp->rule.ipv6udp.sPortL)
			return FAILED;

		if(aclSrc->rule.ipv6udp.dPortU != aclCmp->rule.ipv6udp.dPortU || aclSrc->rule.ipv6udp.dPortL != aclCmp->rule.ipv6udp.dPortL)
			return FAILED;

		break;
	case ACL_IPV6_ICMP:
		if(aclSrc->rule.ipv6icmp.tcD != aclCmp->rule.ipv6icmp.tcD || aclSrc->rule.ipv6icmp.tcM != aclCmp->rule.ipv6icmp.tcM)
			return FAILED;

		if(aclSrc->rule.ipv6icmp.typeV != aclCmp->rule.ipv6icmp.typeV || 
			aclSrc->rule.ipv6icmp.type1 != aclCmp->rule.ipv6icmp.type1 ||
			aclSrc->rule.ipv6icmp.type2 != aclCmp->rule.ipv6icmp.type2 ||
			aclSrc->rule.ipv6icmp.type3 != aclCmp->rule.ipv6icmp.type3 ||
			aclSrc->rule.ipv6icmp.type4 != aclCmp->rule.ipv6icmp.type4)
			return FAILED;

		if(aclSrc->rule.ipv6icmp.codeV != aclCmp->rule.ipv6icmp.codeV || 
			aclSrc->rule.ipv6icmp.code1 != aclCmp->rule.ipv6icmp.code1 ||
			aclSrc->rule.ipv6icmp.code2 != aclCmp->rule.ipv6icmp.code2 ||
			aclSrc->rule.ipv6icmp.code3 != aclCmp->rule.ipv6icmp.code3 ||
			aclSrc->rule.ipv6icmp.code4 != aclCmp->rule.ipv6icmp.code4)
			return FAILED;


		break;
	default:

		return FAILED;
	}

	return SUCCESS;
}

	
int32 _rtl8366s_replaceAclRuleExisted(enum PORTID port,rtl8366s_acltable *aclTable)
{

	uint32 aclStart;
	uint32 aclEnd;
	int16 aclIdx;
	uint32 retVal;

	rtl8366s_acltable aclRuleCmp;

	rtl8366s_getAsicAclStartEnd(port,&aclStart,&aclEnd);

	/*have only one ACL rule valid at port n*/
	if(aclStart == aclEnd)
	{
		rtl8366s_getAsicAclRule(aclStart,&aclRuleCmp);

		/*different rule*/
		if(aclTable->format != aclRuleCmp.format)
			return FAILED;

		/*match all rule items*/
		if(SUCCESS == _rtl8366s_cmpAclRule(aclTable,&aclRuleCmp))
		{
			aclRuleCmp.ac_meteridx= aclTable->ac_meteridx;
			aclRuleCmp.ac_policing = aclTable->ac_policing;
			aclRuleCmp.ac_priority = aclTable->ac_priority;
			aclRuleCmp.ac_spri = aclTable->ac_spri;
			aclRuleCmp.ac_mirpmsk= aclTable->ac_mirpmsk;
			aclRuleCmp.ac_mir = aclTable->ac_mir;
			aclRuleCmp.op_term= aclTable->op_term;
			aclRuleCmp.op_exec= aclTable->op_exec;
			aclRuleCmp.op_and= aclTable->op_and;
			aclRuleCmp.op_not= aclTable->op_not;
			aclRuleCmp.op_init= aclTable->op_init;

			retVal = rtl8366s_setAsicAclRule(aclStart,&aclRuleCmp);

#if 0
			if(retVal == SUCCESS)
				return aclStart;
			else
				return FAILED;
#endif
			return retVal;
		}
#if 0


		memcpy(&aclRuleCmp,aclTable,sizeof(rtl8366s_acltable));

		rtl8366s_setAsicAclRule(aclStart,&aclRuleCmp);

		return SUCCESS;
#endif
	}
	else
	{
		aclIdx = aclStart;
		while(1)
		{
			
			rtl8366s_getAsicAclRule(aclIdx,&aclRuleCmp);
			
			if(SUCCESS == _rtl8366s_cmpAclRule(aclTable,&aclRuleCmp))	
			{
				aclRuleCmp.ac_meteridx= aclTable->ac_meteridx;
				aclRuleCmp.ac_policing = aclTable->ac_policing;
				aclRuleCmp.ac_priority = aclTable->ac_priority;
				aclRuleCmp.ac_spri = aclTable->ac_spri;
				aclRuleCmp.ac_mirpmsk= aclTable->ac_mirpmsk;
				aclRuleCmp.ac_mir = aclTable->ac_mir;
				aclRuleCmp.op_term= aclTable->op_term;
				aclRuleCmp.op_exec= aclTable->op_exec;
				aclRuleCmp.op_and= aclTable->op_and;
				aclRuleCmp.op_not= aclTable->op_not;
				aclRuleCmp.op_init= aclTable->op_init;

				retVal = rtl8366s_setAsicAclRule(aclIdx,&aclRuleCmp);

#if 0
				if(retVal == SUCCESS)
					return aclIdx;
				else
					return FAILED;
#endif
				return retVal;
			}

#if 0
			if(aclTable->format == aclRuleCmp.format)
			{
				memcpy(&aclRuleCmp,aclTable,sizeof(rtl8366s_acltable));
				rtl8366s_setAsicAclRule(aclStart,&aclRuleCmp);

				return SUCCESS;
			}
#endif
			
			if(aclIdx == aclEnd)
				break;
			
			if(aclIdx >= RTL8366S_ACLINDEXMAX)
				aclIdx = 0;
			else
				aclIdx ++;
		}	
	
	}

	return FAILED;
}


int32 _rtl8366s_checkAclNextIndex(enum PORTID port,uint32* startRule, uint32* endRule)
{
	uint32 aclFunctionality;
	uint32 ruleFree;
	uint32 idxFree;
	uint32 idxMove;
	const uint16 startEnd[6]= {0,6,12,17,22,27};

	uint32 startEndAcl[PORT_MAX][2];
	
	enum PORTID portChk;
	rtl8366s_acltable aclRuleMove;


	
	rtl8366s_getAsicAcl(port,&aclFunctionality);	

	
	if(DISABLE == aclFunctionality)
	{
		*startRule = startEnd[port];
		*endRule = startEnd[port];
	}
	else
	{
		rtl8366s_getAsicAclStartEnd(port,startRule,endRule);

		if(RTL8366S_ACLINDEXMAX == *endRule)
			*endRule = 0;
		else
			*endRule = *endRule + 1;
	}

	_rtl8366s_getAclRuleUsage(*endRule,&ruleFree);

	/*if free for use*/
	if(ENABLE ==ruleFree)
	{
		return SUCCESS;
	}
	
	if(DISABLE == aclFunctionality)
	{
		if(SUCCESS == _rtl8366s_getAclFreeIndex(&idxFree))
		{
			*startRule = idxFree;
			*endRule = idxFree;

			return SUCCESS;
		}
	}
#if 1
	else
	{
		for(portChk = PORT0;portChk<PORT_MAX;portChk ++)
		{
			rtl8366s_getAsicAclStartEnd(portChk,&startEndAcl[portChk][0],&startEndAcl[portChk][1]);

#if 0		
			PRINT("startEndAcl[%d][0]=%d, startEndAcl[%d][1]=%d\n",portChk,startEndAcl[portChk][0],portChk,startEndAcl[portChk][1]);
#endif
		}
			
		idxFree = *endRule;
		if(SUCCESS ==_rtl8366s_getAclFreeIndex(&idxFree))
		{
			while(idxFree !=  *endRule)
			{
				if(idxFree == 0)
					idxMove = RTL8366S_ACLINDEXMAX;
				else
					idxMove = idxFree -1;

				/*moving ACL */
				rtl8366s_getAsicAclRule(idxMove,&aclRuleMove);	

				rtl8366s_setAsicAclRule(idxFree,&aclRuleMove);	

				for(portChk = PORT0;portChk<PORT_MAX;portChk ++)
				{
					if(startEndAcl[portChk][0] == idxMove)
						startEndAcl[portChk][0]   = idxFree;
					if(startEndAcl[portChk][1] == idxMove)
						startEndAcl[portChk][1]   = idxFree;
				}

				idxFree = idxMove;				
			}
#if 0		
			for(portChk = PORT0;portChk<PORT_MAX;portChk ++)
			{
				PRINT("startEndAcl[%d][0]=%d, startEndAcl[%d][1]=%d\n",portChk,startEndAcl[portChk][0],portChk,startEndAcl[portChk][1]);
			}
#endif
			for(portChk = PORT0;portChk<PORT_MAX;portChk ++)
			{
				if(portChk != port)
				{
					rtl8366s_setAsicAclStartEnd(portChk,startEndAcl[portChk][0],startEndAcl[portChk][1]);
				}	
			}			

			return SUCCESS;
		}
	}
#endif

	return FAILED;

}


/*
@func int32 | rtl8366s_addAclRule | Add ACL rule to dedicated port.
@parm enum PORTID | port | Physical port number (0~5).
@parm rtl8366s_acltable* | aclTable | ACL rule stucture for setting.
@rvalue ERRNO_API_INVALIDPARAM | invalid input parameters.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	Adding new rule to ACL function disabled port will automatic enable ACL function usage and locate new rule to available rule index. If adding rule format
	and all rule fields (exclude opeartion and action fields) are matching to existed rule, then operation and action fields of existed rule will be replace by new
	adding rule and ACL rule number of port is the same.

*/
int32 rtl8366s_addAclRule(enum PORTID port,rtl8366s_acltable *aclTable)
{
	uint32 aclActiveNum;
	uint32 aclStart;
	uint32 aclEnd;
	uint32 aclFunctionality;
	uint32 retVal;

	if(port >= PORT_MAX)
		return ERRNO_API_INVALIDPARAM;


	if(SUCCESS != rtl8366s_getAsicAcl(port,&aclFunctionality))
		return SUCCESS;	


	if(ENABLE == aclFunctionality)
	{
		/*replace exist ACL rule*/
		retVal = _rtl8366s_replaceAclRuleExisted(port,aclTable);
		if(FAILED != retVal)
		{
			return retVal;
		}
	}


	_rtl8366s_getAclUsedRuleNum(&aclActiveNum);

	/*check if have free ACL rule space for adding new one to 32 ACL shared rules*/
	if(aclActiveNum >= RTL8366S_ACLRULENO)
		return FAILED;



	if(SUCCESS == _rtl8366s_checkAclNextIndex(port,&aclStart,&aclEnd))
	{
		if(SUCCESS != rtl8366s_setAsicAclRule(aclEnd,aclTable))
			return SUCCESS;	
		if(SUCCESS != rtl8366s_setAsicAclStartEnd(port,aclStart,aclEnd))
			return SUCCESS;	
		if(SUCCESS != rtl8366s_setAsicAcl(port,ENABLE))
			return SUCCESS;	

		return SUCCESS;	
	}
	
	return FAILED;
}

/*
@func int32 | rtl8366s_delAclRule | Delete ACL rule from dedicated port.
@parm enum PORTID | port | Physical port number (0~5).
@parm rtl8366s_acltable* | aclTable | ACL rule stucture for setting.
@rvalue ERRNO_API_INVALIDPARAM | invalid input parameters.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	Only all rule fields (exculde opration and action fields) match existed rule, then existed matching ACL rule will be deleted. ACL function usage
	of port will be disabled atfer deleting last ative rule in setting port.
*/
int32 rtl8366s_delAclRule(enum PORTID port,rtl8366s_acltable *aclTable)
{
	uint32 aclStart;
	uint32 aclEnd;
	int16 aclIdx;
	int16 aclPreIdx;
	rtl8366s_acltable aclRuleCmp;
	uint32 aclFunctionality;

	if(port >= PORT_MAX)
		return ERRNO_API_INVALIDPARAM;

	if(SUCCESS != rtl8366s_getAsicAcl(port,&aclFunctionality))
		return FAILED;
	
	if(DISABLE == aclFunctionality)
		return FAILED;

	if(SUCCESS != rtl8366s_getAsicAclStartEnd(port,&aclStart,&aclEnd))
		return FAILED;

	aclIdx = aclStart;
	while(1)
	{
		if(SUCCESS != rtl8366s_getAsicAclRule(aclIdx,&aclRuleCmp))
			return FAILED;

		if(aclTable->format == aclRuleCmp.format)
		{
			if(SUCCESS == _rtl8366s_cmpAclRule(aclTable,&aclRuleCmp))
			{
				break;
			}
		}
		
		/*have no existed ACL for delete*/
		if(aclIdx == aclEnd)
		{
			return FAILED;
		}	
		
		if(aclIdx >= RTL8366S_ACLINDEXMAX)
			aclIdx = 0;
		else
			aclIdx ++;
	}


	/*has one ACL rule only*/
	if(aclIdx == aclStart &&  aclIdx == aclEnd)
	{
		if(SUCCESS != rtl8366s_setAsicAcl(port,DISABLE))
			return FAILED;
	}
	else if(aclIdx == aclEnd)
	{
		if(aclIdx == 0)
			aclIdx = RTL8366S_ACLINDEXMAX;
		else
			aclIdx --;
		
		/*reset ACL rule start-end range*/
		if(SUCCESS != rtl8366s_setAsicAclStartEnd(port,aclStart,aclIdx))
			return FAILED;

	}
#if 0
	else if(aclIdx == aclStart)
	{
		if(aclIdx == RTL8366S_ACLINDEXMAX)
			aclIdx = 0;
		else
			aclIdx ++;
		
		/*reset ACL rule start-end range*/
		if(SUCCESS != rtl8366s_setAsicAclStartEnd(port,aclIdx,aclEnd))
			return FAILED;
	}
#endif	
	else
	{
		aclPreIdx = aclIdx;
		while(1)
		{
			if(aclIdx >= RTL8366S_ACLINDEXMAX)
				aclIdx = 0;
			else
				aclIdx ++;
			
			if(SUCCESS != rtl8366s_getAsicAclRule(aclIdx,&aclRuleCmp))	
				return FAILED;
			
			if(SUCCESS != rtl8366s_setAsicAclRule(aclPreIdx,&aclRuleCmp))	
				return FAILED;	

			if(aclIdx == aclEnd)
				break;

			aclPreIdx = aclIdx;			
		}

		/*reset ACL rule start-end range*/
		if(SUCCESS != rtl8366s_setAsicAclStartEnd(port,aclStart,aclPreIdx))
			return FAILED;
	}

	return SUCCESS;
}
/*
@func int32 | rtl8366s_setMac5ForceLink | Set Port/MAC 5 force linking configuration.
@parm enum PORTLINKSPEED | speed | Speed of the port 0b00-10M, 0b01-100M,0b10-1000M, 0b11 reserved.
@parm enum PORTLINKDUPLEXMODE | duplex | Deuplex mode 0b0-half duplex 0b1-full duplex.
@parm uint32 | link | Link status 0b0-link down b1-link up.
@parm uint32 | txPause | Pause frame transmit ability of the port 0b0-inactive 0b1-active.
@parm uint32 | rxPause | Pause frame response ability of the port 0b0-inactive 0b1-active.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@rvalue FAILED | Invalid parameter.
@comm
  	This API can set Port/MAC 5 properties in force mode. 
 */
int32 rtl8366s_setMac5ForceLink(enum PORTLINKSPEED speed,enum PORTLINKDUPLEXMODE duplex,uint32 link,uint32 txPause,uint32 rxPause)
{
	if(speed > SPD_1000M)
		return ERROR_MAC_INVALIDSPEED;

	if(SUCCESS != rtl8366s_setAsicMacForceLink(PORT5,1,speed,duplex,link,txPause,rxPause))
		return FAILED;

	return SUCCESS;
}
/*
@func int32 | rtl8366s_getMac5ForceLink | Set Port/MAC 5 force linking configuration.
@parm enum PORTLINKSPEED* | speed | Speed of the port 0b00-10M, 0b01-100M,0b10-1000M, 0b11 reserved.
@parm enum PORTLINKDUPLEXMODE* | duplex | Deuplex mode 0b0-half duplex 0b1-full duplex.
@parm uint32* | link | Link status 0b0-link down b1-link up.
@parm uint32* | txPause | Pause frame transmit ability of the port 0b0-inactive 0b1-active.
@parm uint32* | rxPause | Pause frame response ability of the port 0b0-inactive 0b1-active.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@rvalue FAILED | Invalid parameter.
@comm
  	This API can set Port/MAC 5 properties in force mode. 
 */
int32 rtl8366s_getMac5ForceLink(enum PORTLINKSPEED* speed,enum PORTLINKDUPLEXMODE* duplex,uint32* link,uint32* txPause,uint32* rxPause)
{
	uint32 autoNegotiation;
	if(SUCCESS != rtl8366s_getAsicPortLinkState(PORT5,speed,duplex,link,txPause,rxPause,&autoNegotiation))
		return FAILED;

	return SUCCESS;
}

/*
@func int32 | rtl8366s_setBandwidthControl | Set port ingress and engress bandwidth control
@parm enum PORTID | port | Port number to be set as CPU port (0~5).
@parm uint32 | inputBandwidth | Input bandwidth control setting (unit: 64kbps),0x3FFF:disable bandwidth control. 
@parm uint32 | OutputBandwidth | Output bandwidth control setting (unit: 64kbps),0x3FFF:disable bandwidth control. 
@rvalue ERRNO_API_INVALIDPARAM | invalid input parameters.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	The port input/output rate = (bandwidth+1)*64Kbps.
*/
int32 rtl8366s_setBandwidthControl(enum PORTID port, uint32 inputBandwidth, uint32 outputBandwidth)
{
	uint32 aprAblility,pprAbility,wfqAbility;

	if(port >= PORT_MAX)
		return ERRNO_API_INVALIDPARAM;

	
	if(SUCCESS != rtl8366s_setAsicPortIngressBandwidth(port,inputBandwidth,QOS_DEFAULT_PREIFP))
		return FAILED;

	/*enable scheduler control ability WFQ*/
	if(SUCCESS !=rtl8366s_getAsicDisableSchedulerAbility(port,&aprAblility,&pprAbility,&wfqAbility))
		return FAILED;
	/*enable ability*/
	wfqAbility = 0;
	if(SUCCESS !=rtl8366s_setAsicDisableSchedulerAbility(port,aprAblility,pprAbility,wfqAbility))
		return FAILED;
	

	if(SUCCESS !=rtl8366s_setAsicPortEgressBandwidth(port,outputBandwidth,QOS_DEFAULT_PREIFP))
		return FAILED;		

	return SUCCESS;
}

/*
@func int32 | rtl8366s_setRtctTesting | Set port RTCT cable length detection start.
@parm enum PORTID | port | Port number to be set as CPU port (0~4).
@parm enum CHANNELID | channel | Detected channel id (0~3}
@rvalue ERRNO_API_INVALIDPARAM | invalid input parameters.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	Detect dedicated port cable length by ASIC with desired channel. After start RTCT detection testing, user must wait more tan 2 seconds to get detected
	result.
*/
int32 rtl8366s_setRtctTesting(enum PORTID port, enum CHANNELID channel)
{
	if(port >= PORT_MAX)
		return ERRNO_API_INVALIDPARAM;
	
	if(channel >= CH_MAX)
		return ERRNO_API_INVALIDPARAM;

		
	if(SUCCESS != rtl8366s_setAsicPHYRegs(port,3,0,(0x8300 | (channel<<4) | channel)))
		return FAILED;

	return SUCCESS;
}

/*
@func int32 | rtl8366s_getRtctResult | Get port RTCT cable length detection result
@parm enum PORTID | port | Port number to be set as CPU port (0~4).
@parm enum CHANNELID | channel | Detected channel id (0~3}.
@parm uint32* |length | Cable length. (unit meter)
@parm uint32* |openSts | Open status of previous RTCT detection result.
@parm uint32* |shortSts | Short status of previous RTCT detection result.
@rvalue ERRNO_API_INVALIDPARAM | invalid input parameters.
@rvalue ERRNO_API_RTCT_NOT_FINISHED | Still in RTCT detection state.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	After start RTCT detection testing, user must wait more tan 2 seconds to get detected result. Only while port and channel input parameter are the same
	as previous RTCT detection testing input parameters, all returned results are correct. Otherwise, return values will be wrong.
*/
int32 rtl8366s_getRtctResult(enum PORTID port,enum CHANNELID channel,uint32* length,uint32* openSts,uint32* shortSts)
{
	uint32 regData;
	
	if(port >= PORT_MAX)
		return ERRNO_API_INVALIDPARAM;
	
	if(channel >= CH_MAX)
		return ERRNO_API_INVALIDPARAM;
		
	if(SUCCESS != rtl8366s_getAsicPHYRegs(port,3,28,&regData))
		return FAILED;

	if(!(regData & (1<<15)))
		return ERRNO_API_RTCT_NOT_FINISHED;

	/*chaneel A,C*/
	if(CH_A == channel || CH_C == channel)
	{
		if(regData & (1<<14))
			*openSts = TRUE;
		else
			*openSts = FALSE;

		if(regData & (1<<13))
			*shortSts = TRUE;
		else
			*shortSts = FALSE;
	}
	/*channel B,D*/
	else
	{
		if(regData & (1<<13))
			*openSts = TRUE;
		else
			*openSts = FALSE;

		if(regData & (1<<14))
			*shortSts = TRUE;
		else
			*shortSts = FALSE;
	}	

	if( (regData & 0x1FFF) > 60)
	{
		*length  = ((regData & 0x1FFF) -40 )/40; 
	}		
	else
	{
		*length = 0;
	}
	
	return SUCCESS;
}


/*
@func int32 | rtl8366s_setKeepCtagFormat | Keep receiving tag/untag format in egress port.. 
@parm enum PORTID | port | Egress port number (0~5).
@parm uint32* | enabled | Output format with original tag/untag format 1: enabled, 0: disabled
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@rvalue ERRNO_VLAN_INVALIDPORT | Invalid port number.
@comm
	Output frame from ASIC will not be tagged C-tag or untagged C-tag if egress port was set enable keep-Ctag-format function. Receiving frame with untag format will
	be output with untag format, priority-tag frame will be output as priority-tag frame and c-tagged frame will be output as original c-tagged frame. But if 802.1q remarking
	function was enabled in the egress port, then the original priroity of frame will be impacted.	
*/
int32 rtl8366s_setKeepCtagFormat(enum PORTID port, uint32 enabled)
{
	if(SUCCESS != rtl8366s_setAsicKeepCtagFormat(port,enabled))
		return FAILED;

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

/*
@func int32 | rtl8366s_getVlan_with_fid | Get a VLAN entry in specified filtering database.
@parm uint32 | vid | VLAN ID to get entry (0~4095).
@parm uint32* | mbrmsk | pointer to returned member set portmask.
@parm uint32* | untagmsk | pointer to returned untag set portmask.
@parm uint32* | fid | pointer to returned filtering database value.
@rvalue ERRNO_API_INVALIDPARAM | invalid input parameters.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	The API can get the member set and untag set settings for specified vid and fid.
*/
int32 rtl8366s_getVlan_with_fid(uint32 vid, uint32 *mbrmsk, uint32 *untagmsk, uint32 *fid)
{
	rtl8366s_vlan4kentry vlan4K;
	
	/* vid must be 0~4095 */
	if(vid > RTL8366S_VIDMAX)
		return ERRNO_API_INVALIDPARAM;

	vlan4K.vid = vid;
	if(rtl8366s_getAsicVlan4kEntry(&vlan4K) != SUCCESS)
		return FAILED;

	*mbrmsk = vlan4K.member;
	*untagmsk = vlan4K.untag;
	*fid = vlan4K.fid;

	return SUCCESS;
}

/*
@func int32 | rtl8366s_getVlanPVID_with_fid | Get VLAN ID(PVID) on specified port and fid.
@parm enum PORTID | port | Port number (0~5).
@parm uint32* | vid | pointer to returned VLAN ID.
@parm uint32* | priority | pointer to returned 802.1p priority.
@parm uint32* | fid | pointer to returned filtering database value.
@rvalue ERRNO_API_INVALIDPARAM | invalid input parameters.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	The API can get the PVID and 802.1p priority for the PVID of Port-based VLAN in specified filtering 
	database.
*/
int32 rtl8366s_getVlanPVID_with_fid(enum PORTID port, uint32 *vid, uint32 *priority, uint32 *fid)
{
	uint32 index;
	rtl8366s_vlanconfig vlanMC;	

	if(port >= PORT_MAX)
		return ERRNO_API_INVALIDPARAM;
	
	if(rtl8366s_getAsicVlanPortBasedVID(port, &index) != SUCCESS)
		return FAILED;	

	if(rtl8366s_getAsicVlanMemberConfig(index, &vlanMC) != SUCCESS)
		return FAILED;

	*vid = vlanMC.vid;
	*priority = vlanMC.priority;	
	*fid = vlanMC.fid;
	return SUCCESS;
}
/*
@func int32 | rtl8366s_addLUTUnicast_with_fid | Set LUT unicast entry with fid.
@parm uint8 | mac | 6 bytes unicast(I/G bit is 0) mac address to be written into LUT.
@parm enum PORTID | port | Port number to be forwarded to (0~5).
@parm uint32 | fid | filtering database value (0~7) to set.
@rvalue ERRNO_API_INVALIDPARAM | invalid input parameters.
@rvalue ERRNO_API_LUTFULL | hashed index is full of entries.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	The LUT has a 4-way entry of an index.
	If the unicast mac address already existed	in the LUT in specified filtering database, 
	it will udpate the port of the entry. Otherwise, it will find an empty or asic auto 
	learned entry to write. If all the four entries can't be replaced, that is, all
	the four entries are added by CPU, it will return a ERRNO_API_LUTFULL error.
*/
int32 rtl8366s_addLUTUnicast_with_fid(uint8 *mac, enum PORTID port, uint32 fid)
{
	uint32 i;
	rtl8366s_l2table l2Table;
		
	/* must be unicast address */
	if((mac == NULL) || (mac[0] & 0x1))
		return ERRNO_API_INVALIDPARAM;

	if(port >= PORT_MAX)
		return ERRNO_API_INVALIDPARAM;	

	if(fid > RTL8366S_FIDMAX)
		return ERRNO_API_INVALIDPARAM;	

	/*
	  Scan four-ways to see if the unicast mac address already existed.
	  If the unicast mac address already existed, update the port of the entry.	  
	  Scanning priority of four way is entry 0 > entry 1 > entry 2 > entry 3.	  
	*/
	for(i = 0; i < 4; i++)
	{
		/* fill key(MAC,FID) to get L2 entry */
		_rtl8366s_copy(l2Table.swstatic.mac.octet, mac, ETHER_ADDR_LEN);
		l2Table.swstatic.fid = fid;


		if(rtl8366s_getAsicL2LookupTb(i, &l2Table, FALSE) != SUCCESS)
		{					
			return FAILED;
		}	
		else if(_rtl8366s_cmp(mac, l2Table.swstatic.mac.octet, ETHER_ADDR_LEN) == SUCCESS)
		{
			l2Table.swstatic.fid = fid;
			l2Table.swstatic.mbr = (1<<port);
			l2Table.swstatic.auth = FALSE;
			l2Table.swstatic.swst = TRUE;
			l2Table.swstatic.ipmulti = FALSE;
			if(rtl8366s_setAsicL2LookupTb(i, &l2Table, FALSE) != SUCCESS)
				return FAILED;
			else
				return SUCCESS;				
		}
	}

	/*
	  The entry was not written into LUT before, then scan four-ways again to
	  find an empty or asic auto learned entry to write.
	*/
	for(i = 0; i < 4; i++)
	{
		_rtl8366s_copy(l2Table.swstatic.mac.octet, mac, ETHER_ADDR_LEN);
		l2Table.swstatic.fid = fid;			
	
		if(rtl8366s_getAsicL2LookupTb(i, &l2Table, FALSE) != SUCCESS)
		{					
			return FAILED;
		}	
		else if((l2Table.swstatic.swst == FALSE)&&
				 (l2Table.swstatic.auth == FALSE)&&
				 (l2Table.swstatic.ipmulti == FALSE))
		{
			_rtl8366s_copy(l2Table.swstatic.mac.octet, mac, ETHER_ADDR_LEN);
			l2Table.swstatic.fid = fid;		
			l2Table.swstatic.mbr = (1<<port);
			l2Table.swstatic.auth = FALSE;
			l2Table.swstatic.swst = TRUE;
			l2Table.swstatic.ipmulti = FALSE;
			if(rtl8366s_setAsicL2LookupTb(i, &l2Table, FALSE) != SUCCESS)
				return FAILED;
			else
				return SUCCESS;				
		}
	}

	/* four-ways are all full */	
	return ERRNO_API_LUTFULL;
}

/*
@func int32 | rtl8366s_addLUTMulticast_with_fid | Set LUT multicast entry with specified fid.
@parm uint8 | mac | 6 bytes multicast(I/G bit isn't 0) mac address to be written into LUT.
@parm uint32 | portmask | Port mask to be forwarded to (0~0x3F).
@parm uint32 | fid | filtering database value (0~7) to set.
@rvalue ERRNO_API_INVALIDPARAM | invalid input parameters.
@rvalue ERRNO_API_LUTFULL | hashed index is full of entries.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	The LUT has a 4-way entry of an index.
	If the multicast mac address already existed in the LUT in specified filtering database, 
	it will udpate the port mask of the entry. Otherwise, it will find an empty or asic
	auto learned entry to write. If all the four entries can't be replaced, that is, all
	the four entries are added by CPU, it will return a ERRNO_API_LUTFULL error.
*/
int32 rtl8366s_addLUTMulticast_with_fid(uint8 *mac, uint32 portmask, uint32 fid)
{
	uint32 i;
	rtl8366s_l2table l2Table;

	/* must be L2 multicast address */
	if((mac == NULL) || (!(mac[0] & 0x1)))
		return ERRNO_API_INVALIDPARAM;

	if(portmask > RTL8366S_PORTMASK)
		return ERRNO_API_INVALIDPARAM;	

	if(fid > RTL8366S_FIDMAX)
		return ERRNO_API_INVALIDPARAM;	

	/*
	  Scan four-ways to see if the multicast mac address already existed.
	  If the multicast mac address already existed, update the port mask of the entry.
	  Scanning priority of four way is entry 0 > entry 1 > entry 2 > entry 3.	  
	*/
	for(i = 0; i < 4; i++)
	{
		/* fill key(MAC,FID) to get L2 entry */
		_rtl8366s_copy(l2Table.swstatic.mac.octet, mac, ETHER_ADDR_LEN);
		l2Table.swstatic.fid = fid;			
	
		if(rtl8366s_getAsicL2LookupTb(i, &l2Table, FALSE) != SUCCESS)
		{					
			return FAILED;
		}	
		else if(_rtl8366s_cmp(mac, l2Table.swstatic.mac.octet, ETHER_ADDR_LEN) == SUCCESS)
		{
			l2Table.swstatic.fid = fid;		
			l2Table.swstatic.mbr = portmask;
			l2Table.swstatic.auth = FALSE;
			l2Table.swstatic.swst = TRUE;
			l2Table.swstatic.ipmulti = FALSE;
			if(rtl8366s_setAsicL2LookupTb(i, &l2Table, FALSE) != SUCCESS)
				return FAILED;
			else
				return SUCCESS;				
		}
	}

	/*
	  The entry was not written into LUT before, then scan four-ways again to
	  find an empty or asic auto learned entry to write.
	*/
	for(i = 0; i < 4; i++)
	{
		_rtl8366s_copy(l2Table.swstatic.mac.octet, mac, ETHER_ADDR_LEN);
		l2Table.swstatic.fid = fid;			
	
		if(rtl8366s_getAsicL2LookupTb(i, &l2Table, FALSE) != SUCCESS)
		{					
			return FAILED;
		}	
		else if((l2Table.swstatic.swst == FALSE)&&
				 (l2Table.swstatic.auth == FALSE)&&
				 (l2Table.swstatic.ipmulti == FALSE))
		{
			_rtl8366s_copy(l2Table.swstatic.mac.octet, mac, ETHER_ADDR_LEN);
			l2Table.swstatic.fid = fid;		
			l2Table.swstatic.mbr = portmask;
			l2Table.swstatic.auth = FALSE;
			l2Table.swstatic.swst = TRUE;
			l2Table.swstatic.ipmulti = FALSE;
			if(rtl8366s_setAsicL2LookupTb(i, &l2Table, FALSE) != SUCCESS)
				return FAILED;
			else
				return SUCCESS;				
		}
	}

	/* four-ways are all full */	
	return ERRNO_API_LUTFULL;
}

/*
@func int32 | rtl8366s_delLUTMACAddress_with_fid | Delete LUT unicast/multicast entry with specified fid.
@parm uint8 | mac | 6 bytes unicast/multicast mac address to be deleted.
@parm uint32 | fid | filtering database value (0~7) to search.
@rvalue ERRNO_API_INVALIDPARAM | invalid input parameters.
@rvalue ERRNO_API_LUTNOTEXIST | No such LUT entry.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	If the mac has existed in the LUT in specified filtering database, it will be deleted.
	Otherwise, it will return ERRNO_API_LUTNOTEXIST.
*/
int32 rtl8366s_delLUTMACAddress_with_fid(uint8 *mac, uint32 fid)
{
	uint32 i;
	rtl8366s_l2table l2Table;
		
	if(mac == NULL)
		return ERRNO_API_INVALIDPARAM;

	if(fid > RTL8366S_FIDMAX)
		return ERRNO_API_INVALIDPARAM;	

	/* Scan four-ways to find the specified entry */  
	for(i = 0; i < 4; i++)
	{
		/* fill key(MAC,FID) to get L2 entry */
		_rtl8366s_copy(l2Table.swstatic.mac.octet, mac, ETHER_ADDR_LEN);
		l2Table.swstatic.fid = fid;
	
		if(rtl8366s_getAsicL2LookupTb(i, &l2Table, FALSE) != SUCCESS)
		{					
			return FAILED;
		}	
		else if(_rtl8366s_cmp(mac, l2Table.swstatic.mac.octet, ETHER_ADDR_LEN) == SUCCESS &&
				 l2Table.swstatic.swst == TRUE)
		{
			l2Table.swstatic.fid = fid;
			l2Table.swstatic.mbr = 0;
			l2Table.swstatic.auth = FALSE;
			l2Table.swstatic.swst = FALSE;
			l2Table.swstatic.ipmulti = FALSE;
			if(rtl8366s_setAsicL2LookupTb(i, &l2Table, FALSE) != SUCCESS)
				return FAILED;
			else
				return SUCCESS;				
		}
	}

	/* No such LUT entry */	
	return ERRNO_API_LUTNOTEXIST;
}

/*
@func int32 | rtl8366s_getLUTUnicast_with_fid | Get LUT unicast entry in specified filtering database with fid.
@parm uint8 | mac | 6 bytes unicast(I/G bit is 0) mac address to get.
@parm enum PORTID* | port | pointer to returned port number.
@parm uint32 | fid | filtering database value (0~7) to get .
@rvalue ERRNO_API_INVALIDPARAM | invalid input parameters.
@rvalue ERRNO_API_LUTNOTEXIST | No such LUT entry.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	If the unicast mac address existed in the LUT in specified filtering database, it 
	will return the port where the mac is learned. Otherwise, it will return a 
	ERRNO_API_LUTNOTEXIST error.
*/
int32 rtl8366s_getLUTUnicast_with_fid(uint8 *mac, enum PORTID *port, uint32 fid)
{
	uint32 i;
	uint32 j;
	rtl8366s_l2table l2Table;
		
	/* must be unicast address */
	if((mac == NULL) || (mac[0] & 0x1))
		return ERRNO_API_INVALIDPARAM;

	
	if(fid > RTL8366S_FIDMAX)
		return ERRNO_API_INVALIDPARAM;	

	/* Scan four-ways to see if the entry existed */
	for(i = 0; i < 4; i++)
	{
		/* fill key(MAC,FID) to get L2 entry */
		_rtl8366s_copy(l2Table.swstatic.mac.octet, mac, ETHER_ADDR_LEN);
		l2Table.swstatic.fid = fid;
	
		if(rtl8366s_getAsicL2LookupTb(i, &l2Table, FALSE) != SUCCESS)
		{					
			return FAILED;
		}	
		else if(_rtl8366s_cmp(mac, l2Table.swstatic.mac.octet, ETHER_ADDR_LEN) == SUCCESS)
		{

			if(TRUE == l2Table.swstatic.swst)
			{
				for(j = 0; j < PORT_MAX; j++)
				{	
					if(l2Table.swstatic.mbr & (1<<j))
					{
						*port = j;
						return SUCCESS;
					}
				}	
			}
			else
			{
				*port = l2Table.autolearn.spa;
				return SUCCESS;
			}
			/* No member port, should never happen */
			return ERRNO_API_LUTNOTEXIST;
		}
	}

	/* Entry not found */	
	return ERRNO_API_LUTNOTEXIST;
}

/*
@func int32 | rtl8366s_setPortUsage | Set port disable or enable. 
@parm enum PORTID | port | Disable port number (0~5).
@parm uint32 | enabled | enable or disable port  1: enabled, 0: disabled
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@rvalue ERRNO_VLAN_INVALIDPORT | Invalid port number.
@comm
	This API is used to disable port or enable port. When port is enabled, the port link status is
	decided by PHY. When port is disabled, the port link status is down.
*/

int32 rtl8366s_setPortUsage(enum PORTID port, uint32 enabled)
{
	enum MACLINKMODE force;
	enum PORTLINKSPEED speed;
	enum PORTLINKDUPLEXMODE duplex;
	uint32 link,txPause, rxPause;
	
	/* Invalid input parameter */
	if(port >=PORT_MAX)
		return ERROR_MAC_INVALIDNO;

	if(enabled == 0)
	{
		/*Force Mode*/
		force = MAC_FORCE;
		speed = SPD_1000M;
		duplex = FULL_DUPLEX;
		/*Link Down*/
		link = 0; 
		txPause = 1;
		rxPause = 1;
	}
	else if(enabled == 1)
	{
		/*Normal Mode*/
		force = MAC_NORMAL;
		speed = SPD_1000M;
		duplex = FULL_DUPLEX;
		/*Link Up*/
		link = 1; 
		txPause = 1;
		rxPause = 1;
	}
	else
		return FAILED;		

	if(SUCCESS != rtl8366s_setAsicMacForceLink(port,force,speed,duplex,link,txPause,rxPause))
		return FAILED;

	return SUCCESS;
}


/*
@func int32 | rtl8366s_setLinkAmplitude100M | Add 25mv to 100M link amplitude. 
@parm enum RTL8366S_LINKAMP | amplitude | link amplitude: 
	LINKAMP_NORMAL = 0mv,
	LINKAMP_25MV = 25mv,
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@comm
	The API can be used to add 25mv to 100M link amplitude.		
*/
int32 rtl8366s_setLinkAmplitude100M(enum RTL8366S_LINKAMP amplitude)
{
	int32 retVal;
	const uint32 linkData[][2] = {{0x5500,0x004},{0x5555,0x0005}};
	

	if (amplitude > LINKAMP_MAX)
		return ERRNO_INVALIDINPUT;
	
	if(amplitude==LINKAMP_NORMAL)
	{
		retVal = rtl8366s_setAsicReg(RTL8366S_LINK_AMPLITUDE_0_REG,linkData[LINKAMP_NORMAL][0]);
		if (retVal !=  SUCCESS) 
			return retVal;	
		retVal = rtl8366s_setAsicReg(RTL8366S_LINK_AMPLITUDE_1_REG,linkData[LINKAMP_NORMAL][1]);
		if (retVal !=  SUCCESS) 
			return retVal;			
	}
	else if (amplitude==LINKAMP_25MV)
	{
		retVal = rtl8366s_setAsicReg(RTL8366S_LINK_AMPLITUDE_0_REG,linkData[LINKAMP_25MV][0]);
		if (retVal !=  SUCCESS) 
			return retVal;	
		retVal = rtl8366s_setAsicReg(RTL8366S_LINK_AMPLITUDE_1_REG,linkData[LINKAMP_25MV][1]);
		if (retVal !=  SUCCESS) 
			return retVal;
	}

	return SUCCESS;
}

/*
@func int32 | rtl8366s_getLinkAmplitude100M | Add 25mv to 100M link amplitude. 
@parm enum RTL8366S_LINKAMP* | amplitude | link amplitude: 
	LINKAMP_NORMAL = 0mv,
	LINKAMP_25MV = 25mv,
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@comm
	The API can be used to get 100M link amplitude.		
*/
int32 rtl8366s_getLinkAmplitude100M(enum RTL8366S_LINKAMP* amplitude)
{
	int32 retVal, mode;
	uint32 regData0, regData1; 
	const uint32 linkData[][2] = {{0x5500,0x004},{0x5555,0x0005}};

	retVal = rtl8366s_getAsicReg(RTL8366S_LINK_AMPLITUDE_0_REG,&regData0);
	if (retVal !=  SUCCESS) 
		return retVal;	
	retVal = rtl8366s_getAsicReg(RTL8366S_LINK_AMPLITUDE_1_REG,&regData1);
	if (retVal !=  SUCCESS) 
		return retVal;		

	for(mode=0;mode<LINKAMP_MAX;mode++)
	{
		if (regData0==linkData[mode][0]&&regData1==linkData[mode][1])
		{
			*amplitude = mode;
			return SUCCESS;
		}
	}
	return FAILED;
}


