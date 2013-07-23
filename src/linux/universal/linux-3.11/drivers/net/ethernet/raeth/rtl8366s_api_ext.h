/*
* Copyright c                  Realtek Semiconductor Corporation, 2007  
* All rights reserved.                                                    
* 
* Program : The external header file of high level API
* Abstract :                                                           
* Author : Nick Wu (nickwu@realtek.com.tw)               
* $Id: rtl8366s_api_ext.h,v 1.16 2007/10/31 06:15:57 hmchung Exp $
*/

#ifndef __RTL8366S_API_EXT_H__
#define __RTL8366S_API_EXT_H__

#include "rtl8366s_api.h"



/* PHY setting */
extern int32 rtl8366s_setEthernetPHY(uint32 phy, phyAbility_t ability);
extern int32 rtl8366s_getEthernetPHY(uint32 phy, phyAbility_t *ability);
extern int32 rtl8366s_getPHYLinkStatus(uint32 phy, uint32 *linkStatus);
extern int32 rtl8366s_setPHYTestMode(uint32 phy, uint32 mode);
extern int32 rtl8366s_getPHYTestMode(uint32 phy, uint32 *mode);
extern int32 rtl8366s_setPHY1000BaseTMasterSlave(uint32 phy, uint32 enabled, uint32 masterslave);

/* QoS */
extern int32 rtl8366s_initQos(uint32 queueNum);
extern int32 rtl8366s_initQos_p(uint32 queueNum);


extern int32 rtl8366s_setDscpToMappingPriority(uint32 dscp, enum PRIORITYVALUE priority);
extern int32 rtl8366s_setPortPriority(portPriority_t priority);
extern int32 rtl8366s_setPriorityAssigment(uint32 portpri, uint32 dot1qpri, uint32 dscppri);
extern int32 rtl8366s_setBandwidthControl(enum PORTID port, uint32 inputBandwidth, uint32 OutputBandwidth);


/* MIB */
extern int32 rtl8366s_resetMIBs(uint32 portMask);
extern int32 rtl8366s_getMIBCounter(enum PORTID port,enum RTL8366S_MIBCOUNTER mibIdx,uint64* counter);

/* Interrupt */
extern int32 rtl8366s_setInterruptControl(uint32 polarity, uint32 linkUpPortMask);
extern int32 rtl8366s_getInterruptStatus(uint32 *linkUpPortMask);

/* VLAN */
extern int32 rtl8366s_initVlan(void);
extern int32 rtl8366s_setVlan(uint32 vid, uint32 mbrmsk, uint32 untagmsk);
extern int32 rtl8366s_getVlan(uint32 vid, uint32 *mbrmsk, uint32 *untagmsk);
extern int32 rtl8366s_setVlanPVID(enum PORTID port, uint32 vid, uint32 priority);
extern int32 rtl8366s_getVlanPVID(enum PORTID port, uint32 *vid, uint32 *priority);
extern int32 rtl8366s_setVlanIngressFilter(enum PORTID port, uint32 enabled);
extern int32 rtl8366s_setVlanAcceptFrameType(enum PORTID port, enum FRAMETYPE type);

/* LUT */
extern int32 rtl8366s_addLUTUnicast(uint8 *mac, enum PORTID port);
extern int32 rtl8366s_addLUTMulticast(uint8 *mac, uint32 portmask);
extern int32 rtl8366s_delLUTMACAddress(uint8 *mac);
extern int32 rtl8366s_getLUTUnicast(uint8 *mac, enum PORTID *port);
extern int32 rtl8366s_addLUTIpMulticast(uint32 sip, uint32 dip, uint32 portmask);
extern int32 rtl8366s_delLUTIpMulticast(uint32 sip, uint32 dip);
extern int32 rtl8366s_setSourceMacLearning(enum PORTID port,uint32 enabled);
extern int32 rtl8366s_getSourceMacLearning(enum PORTID port,uint32* enabled);


/* CPU Port */
extern int32 rtl8366s_setCPUPort(enum PORTID port, uint32 noTag, uint32 dropUnda);
extern int32 rtl8366s_getCPUPort(enum PORTID *port, uint32 *noTag, uint32 *dropUnda);

/*misc*/
extern int32 rtl8366s_initChip(void);


/*LED*/
extern int32 rtl8366s_setLedMode(enum LED_MODE mode, uint32 ledG0Msk, uint32 ledG1Msk, uint32 ledG2Msk);
extern int32 rtl8366s_setLedBlinkRate(enum RTL8366S_LEDBLINKRATE blinkRate);
extern int32 rtl8366s_setRtctTesting(enum PORTID port, enum CHANNELID channel);
extern int32 rtl8366s_getRtctResult(enum PORTID port,enum CHANNELID channel,uint32* length,uint32* openSts,uint32* shortSts);

/*Reserved Multicast Address APIs*/
extern int32 rtl8366s_setReservedMulticastAddress(enum RMATRAPFRAME rma, uint32 enabled);

/*storm filtering control*/
extern  int32 rtl8366s_setStormFiltering(enum PORTID port, uint32 bcstorm, uint32 mcstorm, uint32 undastorm,enum SFC_PERIOD period, enum SFC_COUNT counter);

/*Port mirror APIs*/
extern int32 rtl8366s_setPortMirror(portMirror_t portMirror);

/*ACL APIs*/
extern int32 rtl8366s_initAcl(void);
extern int32 rtl8366s_addAclRule(enum PORTID port,rtl8366s_acltable *aclTable);
extern int32 rtl8366s_delAclRule(enum PORTID port,rtl8366s_acltable *aclTable);

/*Green ethernet*/
extern int32 rtl8366s_setGreenEthernet(uint32 greenFeature, uint32 powerSaving);

/*MAC*/
extern int32 rtl8366s_setMac5ForceLink(enum PORTLINKSPEED speed,enum PORTLINKDUPLEXMODE duplex,uint32 link,uint32 txPause,uint32 rxPause);
extern int32 rtl8366s_getMac5ForceLink(enum PORTLINKSPEED* speed,enum PORTLINKDUPLEXMODE* duplex,uint32* link,uint32* txPause,uint32* rxPause);

extern int32 rtl8366s_setPortUsage(enum PORTID port, uint32 enabled);

extern int32 rtl8366s_setLinkAmplitude100M(enum RTL8366S_LINKAMP amplitude);
extern int32 rtl8366s_getLinkAmplitude100M(enum RTL8366S_LINKAMP* amplitude);

#endif /* __RTL8366S_API_EXT_H__ */

