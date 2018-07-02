/*
* Copyright c                  Realtek Semiconductor Corporation, 2007  
* All rights reserved.                                                    
* 
* Program : The external header file of high level API
* Abstract :                                                           
* Author : Nick Wu (nickwu@realtek.com.tw)               
* $Id: rtl8366rb_api_ext.h,v 1.9 2008-06-25 06:22:50 kobe_wu Exp $
*/

#ifndef __RTL8366RB_API_EXT_H__
#define __RTL8366RB_API_EXT_H__

#include "rtl8366rb_api.h"


/* PHY */
extern int32 rtl8366rb_setEthernetPHY(uint32 phy, rtl8366rb_phyAbility_t *ptr_ability);
extern int32 rtl8366rb_getEthernetPHY(uint32 phy, rtl8366rb_phyAbility_t *ptr_ability);
extern int32 rtl8366rb_getPHYLinkStatus(uint32 phy, uint32 *ptr_linkStatus);
extern int32 rtl8366rb_setPHYTestMode(uint32 phy, uint32 mode);
extern int32 rtl8366rb_getPHYTestMode(uint32 phy, uint32 *ptr_mode);
extern int32 rtl8366rb_setPHY1000BaseTMasterSlave(uint32 phy, uint32 enabled, uint32 masterslave);

/* MIB Counter */
extern int32 rtl8366rb_resetMIBs(uint32 portMask);
extern int32 rtl8366rb_getMIBCounter(uint32 port, rtl8366rb_mibcounter_t mibIdx,uint64 *ptr_counter);

/* Interrupt */
extern int32 rtl8366rb_setInterruptControl(rtl8366rb_interruptConfig_t *ptr_intcfg);
extern int32 rtl8366rb_getInterruptStatus(rtl8366rb_interruptConfig_t *ptr_intcfg);

/* QoS */
extern int32 rtl8366rb_initQos(uint32 queueNum);
extern int32 rtl8366rb_setPortPriority(rtl8366rb_portPriority_t *ptr_portpriority);
extern int32 rtl8366rb_set1QMappingPriority(rtl8366rb_dot1qPriority_t *ptr_1qpriority);
extern int32 rtl8366rb_setDscpToMappingPriority(uint32 dscp, uint32 priority);
extern int32 rtl8366rb_setPrioritySelection(uint32 portpri, uint32 dot1qpri, uint32 dscppri, uint32 aclpri);
extern int32 rtl8366rb_setPriorityToQueue(uint32 qnum, rtl8366rb_pri2Qid_t *ptr_qidMapping);
extern int32 rtl8366rb_setQueueWeight(uint32 port, rtl8366rb_qConfig_t *ptr_queueCfg );
extern int32 rtl8366rb_setBandwidthControl(uint32 port, uint32 inputBandwidth, uint32 outputBandwidth);

/* CPU */
extern int32 rtl8366rb_setCPUPort(uint32 port, uint32 noTag);
extern int32 rtl8366rb_getCPUPort(uint32 *ptr_port, uint32 *ptr_noTag);

/* VLAN */
extern int32 rtl8366rb_initVlan(void);
extern int32 rtl8366rb_setVlan(rtl8366rb_vlanConfig_t *ptr_vlancfg);
extern int32 rtl8366rb_getVlan(rtl8366rb_vlanConfig_t *ptr_vlancfg);
extern int32 rtl8366rb_setVlanPVID(uint32 port, uint32 vid, uint32 priority);
extern int32 rtl8366rb_getVlanPVID(uint32 port, uint32 *ptr_vid, uint32 *ptr_priority);
extern int32 rtl8366rb_setVlanIngressFilter(uint32 port,uint32 enabled);
extern int32 rtl8366rb_setVlanAcceptFrameType(uint32 port, rtl8366rb_accept_frame_t type);
extern int32 rtl8366rb_setVlanKeepCtagFormat(uint32 ingressport, uint32 portmask);

/* LUT */
extern int32 rtl8366rb_addLUTUnicast(rtl8366rb_l2_entry *ptr_l2_entry);
extern int32 rtl8366rb_addLUTMulticast(rtl8366rb_l2_entry *ptr_l2_entry);
extern int32 rtl8366rb_delLUTMACAddress(rtl8366rb_l2_entry *ptr_l2_entry);
extern int32 rtl8366rb_getLUTUnicast(rtl8366rb_l2_entry *ptr_l2_entry);
extern int32 rtl8366rb_setAgeTimerSpeed( uint32 timer, uint32 speed);
extern int32 rtl8366rb_getAgeTimerSpeed( uint32* timer, uint32* speed);

/* Init */
extern int32 rtl8366rb_initChip(void);

/* LED */
extern int32 rtl8366rb_setLedMode(rtl8366rb_led_mode_t mode, rtl8366rb_LedConfig_t *ptr_ledgroup);
extern int32 rtl8366rb_setLedBlinkRate(rtl8366rb_led_blinkrate_t blinkRate);

/* RTCT */
extern int32 rtl8366rb_setRtctTesting(uint32 operation, enum PORTID port);
extern int32 rtl8366rb_getRtctResult(uint32 operation, enum PORTID port,enum CHANNELID channel, uint32* length, uint32* impmis, uint32* openSts, uint32* shortSts);

/* RMA */
extern int32 rtl8366rb_setReservedMulticastAddress(rtl8366rb_rma_frame_t rma, rtl8366rb_rmaConfig_t *ptr_rmacfg);
extern int32 rtl8366rb_getReservedMulticastAddress(rtl8366rb_rma_frame_t rma, rtl8366rb_rmaConfig_t *ptr_rmacfg);

/* Mirror */
extern int32 rtl8366rb_setPortMirror(rtl8366rb_portMirror_t *ptr_mirrorcfg);

/* ACL */
extern int32 rtl8366rb_initAcl(void);
extern int32 rtl8366rb_addAclRule(rtl8366rb_acltable_t *ptr_aclTable);
extern int32 rtl8366rb_delAclRule(rtl8366rb_acltable_t *ptr_aclTable);

/* P4 RGMII */
extern int32 rtl8366rb_setPort4RGMIIControl(uint32 enabled);

/* MAC */
extern int32 rtl8366rb_setMac5ForceLink(rtl8366rb_macConfig_t *ptr_maccfg);
extern int32 rtl8366rb_getMac5ForceLink(rtl8366rb_macConfig_t *ptr_maccfg);

/* Storm Filtering */
extern int32 rtl8366rb_setStormFiltering(rtl8366rb_stormfilter_t *ptr_stormftr);
extern int32 rtl8366rb_getStormFiltering(rtl8366rb_stormfilter_t *ptr_stormftr);

/* Port Isolation */
extern int32 rtl8366rb_setPortIsolation(rtl8366rb_isolation_t *ptr_isocfg);
extern int32 rtl8366rb_getPortIsolation(rtl8366rb_isolation_t *ptr_isocfg);

/* Learning */
extern int32 rtl8366rb_setSourceMacLearning(uint32 port, uint32 enabled);
extern int32 rtl8366rb_getSourceMacLearning(uint32 port, uint32 *ptr_enabled);

/* Green Feature */
extern int32 rtl8366rb_setGreenEthernet(uint32 greenFeature, uint32 powerSaving);

#endif /* __RTL8366RB_API_EXT_H__ */

