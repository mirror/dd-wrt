/*
* Copyright c                  Realtek Semiconductor Corporation, 2008 
* All rights reserved.
* 
* Program : RTL8366RB switch high-level API
* Abstract : 
* Author : H.M Chung (hmchung@realtek.com.tw), Kobe Wu (kobe_wu@realtek.com.tw)                 
* $Id: rtl8366rb_api.c,v 1.18 2008-06-25 06:22:44 kobe_wu Exp $
*/
/*	@doc rtl8366rb_api

	@module rtl8366rb_api.c - RTL8366RB switch high-level API documentation	|
	This document explains high-level API interface of the asic. 
	@normal 

	Copyright <cp>2008 Realtek<tm> Semiconductor Cooperation, All Rights Reserved.

 	@head3 List of Symbols |
 	Here is a list of all functions and variables in this module.

 	@index | rtl8366rb_api
*/

#include "rtl8366rb_api.h"
#include "rtl8366rb_api_ext.h"
#include "rtl8368s_errno.h"
#include "rtl8368s_asicdrv.h"
#include "rtl8368s_reg.h"
  
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


static void _rtl8366rb_copy(uint8 *dest, const uint8 *src, uint32 n)
{
	uint32 i;
	
	for(i = 0; i < n; i++)
		dest[i] = src[i];
}

static int32 _rtl8366rb_cmp(const uint8 *s1, const uint8 *s2, uint32 n)
{
	uint32 i;
	
	for(i = 0; i < n; i++)
	{
		if(s1[i] != s2[i])
			return FAILED;/* not equal */
	}

	return SUCCESS;
}

static void _rtl8366rb_powof2(uint32 val, uint32 *ret)
{
	*ret = 1;
	
	for(; val > 0; val--)
		*ret = (*ret)*2;
}

static int32 _rtl8366rb_cmpAclRule( rtl8368s_acltable *aclSrc, rtl8368s_acltable *aclCmp)
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

	if (aclSrc->ac_meteridx != aclCmp->ac_meteridx ||
		aclSrc->ac_mir != aclCmp->ac_mir ||
		aclSrc->ac_mirpmsk != aclCmp->ac_mirpmsk ||
		aclSrc->ac_policing != aclCmp->ac_policing ||
		aclSrc->ac_priority != aclCmp->ac_priority ||
		aclSrc->ac_spri != aclCmp->ac_spri ||
		aclSrc->ac_svidx != aclCmp->ac_svidx ||
		aclSrc->format != aclCmp->format ||
		aclSrc->op_and != aclCmp->op_and ||
		aclSrc->op_exec != aclCmp->op_exec ||
		aclSrc->op_init != aclCmp->op_init ||
		aclSrc->op_not != aclCmp->op_not ||
		aclSrc->op_term != aclCmp->op_term ||
		aclSrc->port_en != aclCmp->port_en )
		return FAILED;

	return SUCCESS;
}


/*
@func int32 | rtl8366rb_setEthernetPHY | Set ethernet PHY registers for desired ability.
@parm uint32 | phy | PHY number (0~4).
@parm phyAbility_t | ability | Ability structure
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	If Full_1000 bit is set to 1, the AutoNegotiation will be automatic 
    set to 1. While both AutoNegotiation and Full_1000 are set to 0, the 
    PHY speed and duplex selection will be set as following 100F > 100H 
    > 10F > 10H priority sequence.
*/
int32 rtl8366rb_setEthernetPHY(uint32 phy, rtl8366rb_phyAbility_t *ptr_ability)
{
	uint32 phyData;
	
	uint32 phyEnMsk0;
	uint32 phyEnMsk4;
	uint32 phyEnMsk9;
	
	if(phy > RTL8366RB_PHY_NO_MAX)
		return FAILED;

	phyEnMsk0 = 0;
	phyEnMsk4 = 0;
	phyEnMsk9 = 0;


	if(1 == ptr_ability->Half_10)
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

	if(1 == ptr_ability->Full_10)
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

	if(1 == ptr_ability->Half_100)
	{
		//PRINT("if(1 == ptr_ability->Half_100)\n ");
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


	if(1 == ptr_ability->Full_100)
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
	
	
	if(1 == ptr_ability->Full_1000)
	{
		/*1000 BASE-T FULL duplex capable setting in reg 9.9*/
		phyEnMsk9 = phyEnMsk9 | (1<<9);

		/*Speed selection [1:0] */
		/* 11=Reserved*/
		/* 10= 1000Mpbs*/
		/* 01= 100Mpbs*/
		/* 00= 10Mpbs*/		
		phyEnMsk0 = phyEnMsk0 | (1<<6);
		phyEnMsk0 = phyEnMsk0 & (~(1<<13));
	

		/*Auto-Negotiation setting in reg 0.12*/
#if 0
		phyEnMsk0 = phyEnMsk0 | (1<<12);
#else
		if(ptr_ability->AutoNegotiation != 1)
			return FAILED;			
#endif
	}
	
	if(1 == ptr_ability->AutoNegotiation)
	{
		/*Auto-Negotiation setting in reg 0.12*/
		phyEnMsk0 = phyEnMsk0 | (1<<12);
	}

	if(1 == ptr_ability->AsyFC)
	{
		/*Asymetric flow control in reg 4.11*/
		phyEnMsk4 = phyEnMsk4 | (1<<11);
	}
	if(1 == ptr_ability->FC)
	{
		/*Flow control in reg 4.10*/
		phyEnMsk4 = phyEnMsk4 | (1<<10);
	}

	
	/*1000 BASE-T control register setting*/
	if(SUCCESS != rtl8368s_getAsicPHYRegs(phy,0,RTL8366RB_PHY_1000_BASET_CONTROL_REG,&phyData))
		return FAILED;

	phyData = (phyData & (~0x0200)) | phyEnMsk9 ;
		
	if(SUCCESS != rtl8368s_setAsicPHYRegs(phy,0,RTL8366RB_PHY_1000_BASET_CONTROL_REG,phyData))
		return FAILED;

	/*Auto-Negotiation control register setting*/
	if(SUCCESS != rtl8368s_getAsicPHYRegs(phy,0,RTL8366RB_PHY_AN_ADVERTISEMENT_REG,&phyData))
		return FAILED;

	phyData = (phyData & (~0x0DE0)) | phyEnMsk4;
		
	if(SUCCESS != rtl8368s_setAsicPHYRegs(phy,0,RTL8366RB_PHY_AN_ADVERTISEMENT_REG,phyData))
		return FAILED;


	/*Control register setting and restart auto*/
	if(SUCCESS != rtl8368s_getAsicPHYRegs(phy,0,RTL8366RB_PHY_CONTROL_REG,&phyData))
		return FAILED;


	phyData = (phyData & (~0x3140)) | phyEnMsk0;
	/*If have auto-negotiation capable, then restart auto negotiation*/
	if(1 == ptr_ability->AutoNegotiation)
	{
		phyData = phyData | (1 << 9);
	}
	
	if(SUCCESS != rtl8368s_setAsicPHYRegs(phy,0,RTL8366RB_PHY_CONTROL_REG,phyData))
		return FAILED;

	
	return SUCCESS;
}

/*
@func int32 | rtl8366rb_getEthernetPHY | Get PHY ability through PHY registers.
@parm uint32 | phy | PHY number (0~4).
@parm phyAbility_t* | ptr_ability | Ability structure
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	Get the capablity of specified PHY.
*/
int32 rtl8366rb_getEthernetPHY(uint32 phy, rtl8366rb_phyAbility_t *ptr_ability)
{
	uint32 phyData0;
	uint32 phyData4;
	uint32 phyData9;
	

	if(phy > RTL8366RB_PHY_NO_MAX)
		return FAILED;


	/*Control register setting and restart auto*/
	if(SUCCESS != rtl8368s_getAsicPHYRegs(phy,0,RTL8366RB_PHY_CONTROL_REG,&phyData0))
		return FAILED;

	/*Auto-Negotiation control register setting*/
	if(SUCCESS != rtl8368s_getAsicPHYRegs(phy,0,RTL8366RB_PHY_AN_ADVERTISEMENT_REG,&phyData4))
		return FAILED;

	/*1000 BASE-T control register setting*/
	if(SUCCESS != rtl8368s_getAsicPHYRegs(phy,0,RTL8366RB_PHY_1000_BASET_CONTROL_REG,&phyData9))
		return FAILED;

	if(phyData9 & (1<<9))
		ptr_ability->Full_1000 = 1;
	else
		ptr_ability->Full_1000 = 0;

	if(phyData4 & (1<<11))
		ptr_ability->AsyFC = 1;
	else
		ptr_ability->AsyFC = 0;

	if(phyData4 & (1<<10))
		ptr_ability->FC = 1;
	else
		ptr_ability->FC = 0;
	

	if(phyData4 & (1<<8))
		ptr_ability->Full_100= 1;
	else
		ptr_ability->Full_100= 0;
	
	if(phyData4 & (1<<7))
		ptr_ability->Half_100= 1;
	else
		ptr_ability->Half_100= 0;

	if(phyData4 & (1<<6))
		ptr_ability->Full_10= 1;
	else
		ptr_ability->Full_10= 0;
	
	if(phyData4 & (1<<5))
		ptr_ability->Half_10= 1;
	else
		ptr_ability->Half_10= 0;


	if(phyData0 & (1<<12))
		ptr_ability->AutoNegotiation= 1;
	else
		ptr_ability->AutoNegotiation= 0;

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

/*
@func int32 | rtl8366rb_setPHYTestMode | Set PHY in test mode.
@parm uint32 | phy | PHY number (0~4).
@parm uint32 | mode | PHY test mode 0:normal 1:test mode 1 2:test mode 2 3: test mode 3 4:test mode 4 5~7:reserved
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	Set PHY in test mode and only one PHY can be in test mode at the same time. 
    It means API will return FALED if other PHY is in test mode. 
*/
int32 rtl8366rb_setPHYTestMode(uint32 phy, uint32 mode)
{
	uint32 phyData;
	int16 phyIdx;

	if(phy > RTL8366RB_PHY_NO_MAX)
		return FAILED;

	if(mode > RTL8366RB_PHY_TEST_MODE_MAX)
		return FAILED;

    switch(mode)
	{
		case RTL8366RB_PHY_NORMAL_MODE:	
			rtl8368s_setAsicReg(0x8000, 0);
            rtl8368s_setAsicReg(0xBE55, 0x5500);
            rtl8368s_setAsicReg(0x8000, 0);
            rtl8368s_setAsicReg(0xBE56, 0x0004);
            rtl8368s_setAsicReg(0x8000, 0);
            rtl8368s_setAsicReg(0xBE15, 0x1007);
			break;
		case RTL8366RB_PHY_TEST_MODE1:			
			rtl8368s_setAsicReg(0x8000, 0);
            rtl8368s_setAsicReg(0xBE55, 0xAA00);
            rtl8368s_setAsicReg(0x8000, 0);
            rtl8368s_setAsicReg(0xBE56, 0x0008);
            rtl8368s_setAsicReg(0x8000, 0);
            rtl8368s_setAsicReg(0xBE15, 0x0007);
			break;			
		case RTL8366RB_PHY_TEST_MODE2:
		case RTL8366RB_PHY_TEST_MODE3:
		case RTL8366RB_PHY_TEST_MODE4:
			break;

	 	default:
			return FAILED;
	}

	if(RTL8366RB_PHY_NORMAL_MODE == mode)
	{
		if(SUCCESS != rtl8368s_getAsicPHYRegs(phy,0,RTL8366RB_PHY_1000_BASET_CONTROL_REG,&phyData))
			return FAILED;

		phyData = phyData & (~(0x7<<13));

		if(SUCCESS != rtl8368s_setAsicPHYRegs(phy,0,RTL8366RB_PHY_1000_BASET_CONTROL_REG,phyData))
			return FAILED;
		
	}	
	else
	{
		phyIdx = 0;
		while(phyIdx <= RTL8366RB_PHY_NO_MAX)
		{

			if(phyIdx != phy)
			{
				if(SUCCESS != rtl8368s_getAsicPHYRegs(phyIdx,0,RTL8366RB_PHY_1000_BASET_CONTROL_REG,&phyData))
					return FAILED;
				
				/*have other PHY in test mode*/	
				if(phyData & (0x7<<13))
				{
					return FAILED;
				}
				
			}
			phyIdx ++;
		}

		if(SUCCESS != rtl8368s_getAsicPHYRegs(phy,0,RTL8366RB_PHY_1000_BASET_CONTROL_REG,&phyData))
			return FAILED;
		
		phyData = (phyData & (~(0x7<<13))) | (mode<<13);

		if(SUCCESS != rtl8368s_setAsicPHYRegs(phy,0,RTL8366RB_PHY_1000_BASET_CONTROL_REG,phyData))
			return FAILED;
		
		
	}

	return SUCCESS;
}	

/*
@func int32 | rtl8366rb_getPHYTestMode | Get PHY in which test mode.
@parm uint32 | phy | PHY number (0~4).
@parm uint32* | ptr_mode | PHY test mode 0:normal 1:test mode 1 2:test mode 2 3: test mode 3 4:test mode 4 5~7:reserved
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	Get test mode of PHY from register setting 9.15 to 9.13.
*/
int32 rtl8366rb_getPHYTestMode(uint32 phy, uint32 *ptr_mode)
{
	uint32 phyData;

	if(phy > RTL8366RB_PHY_NO_MAX)
		return FAILED;

	if(SUCCESS != rtl8368s_getAsicPHYRegs(phy,0,RTL8366RB_PHY_1000_BASET_CONTROL_REG,&phyData))
		return FAILED;

	*ptr_mode = (phyData>>13) & 0x7;

	return SUCCESS;
}

/*
@func int32 | rtl8366rb_setPHY1000BaseTMasterSlave | Set PHY control enable MASTER/SLAVE manual configuration.
@parm uint32 | phy | PHY number (0~4).
@parm uint32 | enable | Manual configuration function 1:enable 0:disable.
@parm uint32 | masterslave | Manual config mode 1:master 0: slave
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	Set enable/disable MASTER/SLAVE manual configuration under 1000Base-T with register 9.12-9.11. If MASTER/SLAVE manual configuration is enabled with MASTER, the
	link partner must be set as SLAVE or auto negotiation will fail. 
*/
int32 rtl8366rb_setPHY1000BaseTMasterSlave(uint32 phy, uint32 enabled, uint32 masterslave)
{
	uint32 phyData;

	if(phy > RTL8366RB_PHY_NO_MAX)
		return FAILED;

	if(SUCCESS != rtl8368s_getAsicPHYRegs(phy,0,RTL8366RB_PHY_1000_BASET_CONTROL_REG,&phyData))
		return FAILED;

	phyData = (phyData & (~(0x3<<11))) | (enabled<<12) | (masterslave<<11);


	if(SUCCESS != rtl8368s_setAsicPHYRegs(phy,0,RTL8366RB_PHY_1000_BASET_CONTROL_REG,phyData))
		return FAILED;

	if(SUCCESS != rtl8368s_setAsicPHYRegs(phy,0,RTL8366RB_PHY_1000_BASET_CONTROL_REG,phyData))
		return FAILED;



	/*Restart N-way*/
	if(SUCCESS != rtl8368s_getAsicPHYRegs(phy,0,RTL8366RB_PHY_CONTROL_REG,&phyData))
		return FAILED;

	phyData = phyData | (1 << 9);
	
	if(SUCCESS != rtl8368s_setAsicPHYRegs(phy,0,RTL8366RB_PHY_CONTROL_REG,phyData))
		return FAILED;


	return SUCCESS;
}

/*
@func int32 | rtl8366rb_resetMIBs | Reset MIB with port mask setting.
@parm uint32 | portMask | Port mask (0~0x3F).
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	Reset MIB counter of ports. API will use global reset while port mask is 0x3F.
*/
int32 rtl8366rb_resetMIBs(uint32 portMask)
{
	if(portMask > RTL8366RB_PORTMASK)
		return FAILED;
	
	/*get available bits only*/
	portMask = portMask & RTL8366RB_PORTMASK;

	if(RTL8366RB_PORTMASK == portMask)
	{
		if(SUCCESS != rtl8368s_setAsicMIBsCounterReset(RTL8368S_MIB_CTRL_GLOBAL_RESET_MSK))
			return FAILED;
	}
	else 
	{
		if(SUCCESS != rtl8368s_setAsicMIBsCounterReset(portMask<<(RTL8368S_MIB_CTRL_PORT_RESET_OFF)))
			return FAILED;
	}
		
	return SUCCESS;
}

/*
@func int32 | rtl8366rb_getMIBCounter | Get MIB counter
@parm uint32 | port | Physical port number (0~5).
@num rtl8366rb_mibcounter_t | mibIdx | MIB counter index.
@parm uint64* | ptr_counter | MIB retrived counter.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	Get MIB counter by index definition as following description. 
    There are per port MIB counters from index 0 to 32 while index 33 
	is a system-wide counter.
	index	MIB counter												
	0 		IfInOctets
	1		EtherStatsOctets
	2		EtherStatsUnderSizePkts
	3		etherStatsFragments
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
	33		OutOampduPkts
	34		InOampduPkts
	35		PktgenPkts
	36		Dot1dTpLearnEntryDiscardFlag	
*/
int32 rtl8366rb_getMIBCounter(uint32 port, rtl8366rb_mibcounter_t mibIdx, uint64 *ptr_counter)
{
	if(SUCCESS != rtl8368s_getAsicMIBsCounter(port, mibIdx, ptr_counter))		
		return FAILED;

	return SUCCESS;
}

/*
@func int32 | rtl8366rb_setInterruptControl | Set gpio interrupt trigger configuration.
@parm uint32 | ptr_intcfg->polarity | I/O polarity 0:pull high 1:pull low.
@parm uint32 | ptr_intcfg->linkChangedPorts | Link changed trigger enable port mask (0-0xFF).
@parm uint32 | ptr_intcfg->aclExceed | ACL meter exceeded interrupt trigger enable.
@parm uint32 | ptr_intcfg->stormExceed | Storm filtering rate exceeded interrupt triggerenable.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	ASIC pulls gpio high/low to trigger CPU about port link up/down event.
*/
int32 rtl8366rb_setInterruptControl(rtl8366rb_interruptConfig_t *ptr_intcfg)
{
	uint32 mask;

	if(ptr_intcfg->linkChangedPorts > RTL8366RB_PORTMASK)
		return FAILED;		

	if(SUCCESS != rtl8368s_setAsicInterruptPolarity(ptr_intcfg->polarity))
		return FAILED;

	mask = (ptr_intcfg->linkChangedPorts<<RTL8368S_INTERRUPT_LINKCHANGE_OFF) & RTL8368S_INTERRUPT_LINKCHANGE_MSK;
	mask = mask | ((ptr_intcfg->aclExceed<<RTL8368S_INTERRUPT_ACLEXCEED_OFF) & RTL8368S_INTERRUPT_ACLEXCEED_MSK);
	mask = mask | ((ptr_intcfg->stormExceed<<RTL8368S_INTERRUPT_STORMEXCEED_OFF) & RTL8368S_INTERRUPT_STORMEXCEED_MSK);

    if(SUCCESS != rtl8368s_setAsicInterruptMask(mask))
		return FAILED;

	return SUCCESS;
}

/*
@func int32 | rtl8366rb_getInterruptStatus | Get interrupt event status.
@parm uint32 | ptr_intcfg->linkChangedPorts | Link changed trigger enable port mask (0-0xFF).
@parm uint32 | ptr_intcfg->aclExceed | ACL meter exceeded interrupt trigger enable.
@parm uint32 | ptr_intcfg->stormExceed | Storm filtering rate exceeded interrupt triggerenable.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	ASIC will trigger CPU again after interrupt status mask was read.
*/
int32 rtl8366rb_getInterruptStatus(rtl8366rb_interruptConfig_t *ptr_intcfg)
{
	uint32 mask;
	
	if(SUCCESS != rtl8368s_getAsicInterruptStatus(&mask))
		return FAILED;

	ptr_intcfg->linkChangedPorts = (mask & RTL8368S_INTERRUPT_LINKCHANGE_MSK) >> RTL8368S_INTERRUPT_LINKCHANGE_OFF;
	ptr_intcfg->aclExceed = (mask & RTL8368S_INTERRUPT_ACLEXCEED_MSK) >> RTL8368S_INTERRUPT_ACLEXCEED_OFF;
	ptr_intcfg->stormExceed = (mask & RTL8368S_INTERRUPT_STORMEXCEED_MSK) >> RTL8368S_INTERRUPT_STORMEXCEED_OFF;

	return SUCCESS;
}


/*
@func int32 | rtl8366rb_initQos | Configure Qos settings with queue number assigment to each port.
@parm uint32 | queueNum | Queue number of each port. it is available at 1~6.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	This API will initialize related Qos setting with queue number assigment. 
*/
int32 rtl8366rb_initQos(uint32 queueNum)
{
	int16 priority;
	uint32 qidCfg;

	enum QUEUEID qid;
	enum PORTID port;
	uint32 dscp;
	
	const enum QUEUEID qidConfig[] = {QUEUE0,QUEUE5,QUEUE1,QUEUE2,QUEUE3,QUEUE4};

	if(queueNum == 0 || queueNum > QOS_DEFAULT_QUEUE_NO_MAX)
		return FAILED;

	/*Bandwidth control, set unlimit to default*/
	/*inbound Set flow control high/low threshold, no used while bandwidth control to unlimit*/
	for(port = PORT0;port<PORT_MAX;port ++)
	{
		if(SUCCESS != rtl8368s_setAsicPortIngressBandwidth(port,QOS_DEFAULT_INGRESS_BANDWIDTH,QOS_DEFAULT_PREIFP))
			return FAILED;	
		if(SUCCESS != rtl8368s_setAsicPortEgressBandwidth(port,QOS_DEFAULT_EGRESS_BANDWIDTH,QOS_DEFAULT_PREIFP))
			return FAILED;	
	}
	
	/*disable packet used pages flow control for RX reg[0x0264]*/
	if(SUCCESS != 	rtl8368s_setAsicFcPacketUsedPagesBased(QOS_DEFAULT_PACKET_USED_PAGES_FC,QOS_DEFAULT_PACKET_USED_FC_EN))
		return FAILED;	

	/*port-based priority*/
	for(port=PORT0,priority=PRI0;port<PORT_MAX;port++,priority++)
	{
		if(SUCCESS != 	rtl8368s_setAsicPortPriority(port,priority))
			return FAILED;	
	}		

	/*dscp to priority*/
	for(dscp = RTL8368S_DSCPMIN;dscp <= RTL8368S_DSCPMAX;dscp ++)
	{
		if(SUCCESS != 	rtl8368s_setAsicDscpPriority(dscp,QOS_DEFAULT_DSCP_MAPPING_PRIORITY))
			return FAILED;			
	}		
	
	/*1Q Priority remapping to absolutely priority */
	for(priority=0;priority<=RTL8368S_PRIORITYMAX;priority ++)
	{
		if(SUCCESS != 	rtl8368s_setAsicDot1qAbsolutelyPriority(priority,g_priority1QRemapping[priority]))
			return FAILED;			
	}
	
	/*User priority to traffic classs mapping setting*/
	for(priority=0;priority<=RTL8368S_PRIORITYMAX;priority ++)
	{
		if(SUCCESS != 	rtl8368s_setAsicPriorityToQIDMappingTable(queueNum,priority,g_prioritytToQueue[queueNum-1][priority]))
			return FAILED;			
	}
	
	/*priority selection with  ACL>DSCP>802.1Q>[Port-based] */
	if(SUCCESS != 	rtl8368s_setAsicPriorityDecision(QOS_DEFAULT_PRIORITY_SELECT_PORT,QOS_DEFAULT_PRIORITY_SELECT_1Q,QOS_DEFAULT_PRIORITY_SELECT_DSCP,QOS_DEFAULT_PRIORITY_SELECT_ACL))
		return FAILED;		

	/*disable Remarking */
	if(SUCCESS != 	rtl8368s_setAsicDscpRemarkingAbility(QOS_DEFAULT_DSCP_REMARKING_ABILITY))
		return FAILED;			
	if(SUCCESS != 	rtl8368s_setAsicDot1pRemarkingAbility(QOS_DEFAULT_1Q_REMARKING_ABILITY))
		return FAILED;			

	/*Buffer management - flow control threshold setting */	
	/*1. system and share buffer based flow control threshold */
	if(SUCCESS != 	rtl8368s_setAsicFcSystemSharedBufferBased(g_thresholdShared[queueNum-1][0],g_thresholdShared[queueNum-1][1]))
		return FAILED;
	if(SUCCESS != 	rtl8368s_setAsicFcSystemBased(g_thresholdSystem[queueNum-1][0],g_thresholdSystem[queueNum-1][1]))
		return FAILED;
	if(SUCCESS != 	rtl8368s_setAsicFcSystemDrop(g_thresholdSystem[queueNum-1][2]))
		return FAILED;

	/*2. port based threshold and queue based threshold */
	for(port = PORT0;port<=PORT5;port ++)
	{
		/*port based*/
		if(SUCCESS != 	rtl8368s_setAsicFcPortBased(port,g_thresholdPort[queueNum-1][0],g_thresholdPort[queueNum-1][1]))
			return FAILED;	
		/*Queue based*/
		for(qidCfg = QUEUE0;qidCfg < QUEUE_MAX;qidCfg ++)
		{
			qid = qidConfig[qidCfg];
			/*queue based*/
			if(SUCCESS != 	rtl8368s_setAsicFcQueueDescriptorBased(port,qid,g_thresholdQueueDescriptor[queueNum-1][0]/2,g_thresholdQueueDescriptor[queueNum-1][1]/2))
				return FAILED;					
			if(SUCCESS != 	rtl8368s_setAsicFcQueuePacketBased(port,qid,g_thresholdQueuePacket[queueNum-1][0]/4,g_thresholdQueuePacket[queueNum-1][1]/4))
				return FAILED;		
			/*Enable queue-base flow control*/
			if(SUCCESS != rtl8368s_setAsicFcQueueFlowControlUsage(port,qid,QOS_DEFAULT_QUEUE_BASED_FC_EN))
				return FAILED;						
		}		
	}		

	/*queue gap */
	if(SUCCESS != 	rtl8368s_setAsicFcPerQueuePhysicalLengthGap(g_thresholdGap[queueNum-1]))
		return FAILED;
	
	/*Packet scheduling for Qos setting*/
	for(port = PORT0;port<=PORT5;port ++)
	{
		/*enable scheduler control ability APR,PPR,WFQ*/
		if(SUCCESS != 	rtl8368s_setAsicDisableSchedulerAbility(port,QOS_DEFAULT_SCHEDULER_ABILITY_APR,QOS_DEFAULT_SCHEDULER_ABILITY_PPR,QOS_DEFAULT_SCHEDULER_ABILITY_WFQ))
			return FAILED;				

		/*Queue based*/
		for(qidCfg = 0;qidCfg < QUEUE_MAX;qidCfg ++)
		{
			qid = qidConfig[qidCfg];
			/*initial weight fair queue setting*/
			if(SUCCESS != rtl8368s_setAsicQueueWeight(port,qid,WFQ_PRIO,g_weightQueue[queueNum-1][qid]))
				return FAILED;				
			/*initial leacky bucket control*/
			if(SUCCESS != rtl8368s_setAsicQueueRate(port,qid,QOS_DEFAULT_PEAK_PACKET_RATE,QOS_DEFAULT_BURST_SIZE_IN_APR,QOS_DEFAULT_AVERAGE_PACKET_RATE))
				return FAILED;				
		}		
	}		

	/*Setting F=75Mhz with (T,B) = (19,34)*/
	if(SUCCESS != rtl8368s_setAsicLBParameter(QOS_DEFAULT_BYTE_PER_TOKEN,QOS_DEFAULT_TICK_PERIOD,QOS_DEFAULT_LK_THRESHOLD))
		return FAILED;			

	/*Set each port's queue number */
	for(port = PORT0;port<=PORT5;port++)
	{
		if(SUCCESS != rtl8368s_setAsicOutputQueueNumber(port, queueNum))
			return FAILED;			
	}	

	/*Enable ASIC Qos function*/
	if(SUCCESS != rtl8368s_setAsicQosEnable(ENABLE))
		return FAILED;		

	/*Unlimit Pause Frame  */
	if(SUCCESS != rtl8368s_setAsicRegBit(RTL8368S_SWITCH_GLOBAL_CTRL_REG,RTL8368S_MAX_PAUSE_CNT_OFF,ENABLE))
		return FAILED;		

	/*Queue resetting*/
	if(SUCCESS != rtl8368s_setAsicRegBit(RTL8368S_RESET_CONTROL_REG, RTL8368S_RESET_QUEUE_OFF,ENABLE))
		return FAILED;		

	return SUCCESS;
}

/*
@func int32 | rtl8366rb_setPortPriority | Configure priority usage to each port.
@parm rtl8366rb_portPriority_t *| ptr_portpriority | Priorities assigment for each port.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	Priorityof ports assigment for queue usage and packet scheduling.
*/
int32 rtl8366rb_setPortPriority(rtl8366rb_portPriority_t *ptr_portpriority)
{
	if(SUCCESS != rtl8368s_setAsicPortPriority(0, ptr_portpriority->priPort0))
		return FAILED;
	
	if(SUCCESS != rtl8368s_setAsicPortPriority(1, ptr_portpriority->priPort1))
		return FAILED;
	
	if(SUCCESS != rtl8368s_setAsicPortPriority(2, ptr_portpriority->priPort2))
		return FAILED;
	
	if(SUCCESS != rtl8368s_setAsicPortPriority(3, ptr_portpriority->priPort3))
		return FAILED;
	
	if(SUCCESS != rtl8368s_setAsicPortPriority(4, ptr_portpriority->priPort4))
		return FAILED;
	
	if(SUCCESS != rtl8368s_setAsicPortPriority(5, ptr_portpriority->priPort5))
		return FAILED;

	return SUCCESS;
}

/*
@func int32 | rtl8366rb_set1QMappingPriority | Configure 1Q priority mapping asic internal absolute priority.  
@parm dot1qPriority_t *| ptr_1qpriority | Priorities assigment for receiving 802.1Q prioirty.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	Priorityof 802.1Q assigment for queue usage and packet scheduling.
*/
int32 rtl8366rb_set1QMappingPriority(rtl8366rb_dot1qPriority_t *ptr_1qpriority)
{
	if(SUCCESS != rtl8368s_setAsicDot1qAbsolutelyPriority(PRI0, ptr_1qpriority->dot1qPri0))
		return FAILED;
	
	if(SUCCESS != rtl8368s_setAsicDot1qAbsolutelyPriority(PRI1, ptr_1qpriority->dot1qPri1))
		return FAILED;
	
	if(SUCCESS != rtl8368s_setAsicDot1qAbsolutelyPriority(PRI2, ptr_1qpriority->dot1qPri2))
		return FAILED;
	
	if(SUCCESS != rtl8368s_setAsicDot1qAbsolutelyPriority(PRI3, ptr_1qpriority->dot1qPri3))
		return FAILED;
	
	if(SUCCESS != rtl8368s_setAsicDot1qAbsolutelyPriority(PRI4, ptr_1qpriority->dot1qPri4))
		return FAILED;
	
	if(SUCCESS != rtl8368s_setAsicDot1qAbsolutelyPriority(PRI5, ptr_1qpriority->dot1qPri5))
		return FAILED;
	
	if(SUCCESS != rtl8368s_setAsicDot1qAbsolutelyPriority(PRI6, ptr_1qpriority->dot1qPri6))
		return FAILED;
	
	if(SUCCESS != rtl8368s_setAsicDot1qAbsolutelyPriority(PRI7, ptr_1qpriority->dot1qPri7))
		return FAILED;

	return SUCCESS;
}




/*
@func int32 | rtl8366rb_setDscpToMappingPriority | Map dscp value to queue priority.
@parm uint32 | dscp | Dscp value of receiving frame (0~63)
@parm uint32 | priority | Priority mapping to dscp of receiving frame (0~7). 
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	The Differentiated Service Code Point is a selector for router's per-hop behaviours. 
    As a selector, there is no implication that a numerically greater DSCP implies a better 
    network service. As can be seen, the DSCP totally overlaps the old precedence field of 
    TOS. So if values of DSCP are carefully chosen then backward compatibility can be achieved.	
*/
int32 rtl8366rb_setDscpToMappingPriority(uint32 dscp, uint32 priority)
{
	if(SUCCESS != rtl8368s_setAsicDscpPriority(dscp,priority))
		return FAILED;

	return SUCCESS;	
}

/*
@func int32 | rtl8366rb_setPrioritySelection | Configure the priority among different priority mechanism.
@parm uint32 | portpri | Priority assign for port based selection (0~3).
@parm uint32 | dot1qpri | Priority assign for 802.1q selection (0~3).
@parm uint32 | dscppri | Priority assign for dscp selection (0~3).
@parm uint32 | aclpri | Priority assign for acl selection (0~3).
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	ASIC will follow user priority setting of mechanisms to select mapped queue 
    priority for receiving frame. If two priorities of mechanisms are the same,
	ASIC will chose the highest priority from mechanisms to assign queue priority 
    to receiving frame.
*/
int32 rtl8366rb_setPrioritySelection(uint32 portpri, uint32 dot1qpri, uint32 dscppri, uint32 aclpri)
{	
	uint32 portpri_pow;
	uint32 dot1qpri_pow;
	uint32 dscppri_pow;
	uint32 acl_pow;
	
	if(portpri > 3 || dot1qpri > 3 || dscppri > 3 || aclpri > 3)
		return FAILED;

	_rtl8366rb_powof2(portpri, &portpri_pow);
	_rtl8366rb_powof2(dot1qpri, &dot1qpri_pow);
	_rtl8366rb_powof2(dscppri, &dscppri_pow);
	_rtl8366rb_powof2(aclpri, &acl_pow);

	/*Default ACL priority selection is set to 0 for function disabled setting.*/	
	if(SUCCESS != rtl8368s_setAsicPriorityDecision(portpri_pow, dot1qpri_pow, dscppri_pow, acl_pow))
		return FAILED;

	return SUCCESS;
}




/*
@func int32 | rtl8366rb_setPriorityToQueue | Mapping internal priority to queue id.
@parm uint32 | qnum | Queue number usage.
@parm pri2Queue_t | qidMapping | Priority mapping to queue configuration.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	ASIC supports queue number setting from 1 to 6. For different queue numbers usage, 
    ASIC supports different internal available  queue IDs.
	Queue number		Available  Queue IDs
		1					0
		2					0,5
		3					0,1,5
		4					0,1,2,5
		5					0,1,2,3,5
		6					0,1,2,3,4,5	
	If software mapping priority to non-available queue IDs in less
*/
int32 rtl8366rb_setPriorityToQueue(uint32 qnum, rtl8366rb_pri2Qid_t *ptr_qidMapping)
{	

	/* Invalid input parameter */
	if ((qnum < QNUM1) || (qnum > QNUM6)) 
		return FAILED;

	if(SUCCESS != rtl8368s_setAsicPriorityToQIDMappingTable(qnum,PRI0,ptr_qidMapping->pri0))
		return FAILED;

	if(SUCCESS != rtl8368s_setAsicPriorityToQIDMappingTable(qnum,PRI1,ptr_qidMapping->pri1))
		return FAILED;

	if(SUCCESS != rtl8368s_setAsicPriorityToQIDMappingTable(qnum,PRI2,ptr_qidMapping->pri2))
		return FAILED;

	if(SUCCESS != rtl8368s_setAsicPriorityToQIDMappingTable(qnum,PRI3,ptr_qidMapping->pri3))
		return FAILED;

	if(SUCCESS != rtl8368s_setAsicPriorityToQIDMappingTable(qnum,PRI4,ptr_qidMapping->pri4))
		return FAILED;

	if(SUCCESS != rtl8368s_setAsicPriorityToQIDMappingTable(qnum,PRI5,ptr_qidMapping->pri5))
		return FAILED;

	if(SUCCESS != rtl8368s_setAsicPriorityToQIDMappingTable(qnum,PRI6,ptr_qidMapping->pri6))
		return FAILED;

	if(SUCCESS != rtl8368s_setAsicPriorityToQIDMappingTable(qnum,PRI7,ptr_qidMapping->pri7))
		return FAILED;
	
	return SUCCESS;
}


/*
@func int32 | rtl8366rb_setAsicQueueWeight | Set weight and type of queues' in dedicated port.
@parm uint32 | port | port number.
@parm qConfig_t | queueCfg | Queue type and weigth configuration.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	The API can set weight and type, strict priority or weight fair queue (WFQ) 
    for dedicated port for using queues. If queue id is not included in queue usage,
    then its type and weigth setting in dummy for setting. There are priorities as 
    queue id in strick queues. It means strick queue id 5 carrying higher prioirty than
    strick queue id 4.
 */
int32 rtl8366rb_setQueueWeight(uint32 port, rtl8366rb_qConfig_t *ptr_queueCfg )
{
	enum QUEUEID qid;

	if(port >=RTL8366RB_PORT_MAX)
		return FAILED;


	for(qid = QUEUE0;qid<=QUEUE5;qid ++)
	{

		if(ptr_queueCfg->weight[qid] == 0 || ptr_queueCfg->weight[qid] > QOS_WEIGHT_MAX)
			return FAILED;

		if(SUCCESS != rtl8368s_setAsicQueueWeight(port,qid,ptr_queueCfg->strickWfq[qid],ptr_queueCfg->weight[qid]-1))
			return FAILED;
	}

	return SUCCESS;
}

/*
@func int32 | rtl8366rb_setBandwidthControl | Set port ingress and engress bandwidth control
@parm uint32 | port | Port number to be set as CPU port (0~5).
@parm uint32 | inputBandwidth | Input bandwidth control setting (unit: 64kbps),0x3FFF:disable bandwidth control. 
@parm uint32 | OutputBandwidth | Output bandwidth control setting (unit: 64kbps),0x3FFF:disable bandwidth control. 
@rvalue ERRNO_API_INPUT | invalid input parameters.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	The port input/output rate = (bandwidth+1)*64Kbps.
*/
int32 rtl8366rb_setBandwidthControl(uint32 port, uint32 inputBandwidth, uint32 outputBandwidth)
{
	uint32 aprAblility,pprAbility,wfqAbility;

	if(port >= RTL8366RB_PORT_MAX)
		return ERRNO_API_INPUT;

	
	if(SUCCESS != rtl8368s_setAsicPortIngressBandwidth(port,inputBandwidth,QOS_DEFAULT_PREIFP))
		return FAILED;

	/*Egress Bandwidth Control should enable scheduler control WFQ ability */
	if(SUCCESS !=rtl8368s_getAsicDisableSchedulerAbility(port,&aprAblility,&pprAbility,&wfqAbility))
		return FAILED;
	/*enable ability*/
	wfqAbility = 0;
	if(SUCCESS !=rtl8368s_setAsicDisableSchedulerAbility(port,aprAblility,pprAbility,wfqAbility))
		return FAILED;
	

	if(SUCCESS !=rtl8368s_setAsicPortEgressBandwidth(port,outputBandwidth,QOS_DEFAULT_PREIFP))
		return FAILED;		

	return SUCCESS;
}

/*
@func int32 | rtl8366rb_setCPUPort | Set CPU port with/without inserting CPU tag.
@parm uint32 | port | Port number to be set as CPU port (0~6).
@parm uint32 | noTag | NOT insert Realtek proprietary tag (ethernet length/type 0x8899) to frame 1:not insert 0:insert.
@rvalue ERRNO_API_INPUT | invalid input parameters.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	The API can set CPU port and enable/disable inserting proprietary CPU tag (Length/Type 0x8899)
	to the frame that transmitting to CPU port. It also can enable/disable forwarding unknown
	destination MAC address frames to CPU port. User can reduce CPU loading by not forwarding
	unknown DA frames to CPU port.
*/
int32 rtl8366rb_setCPUPort(uint32 port, uint32 noTag)
{
	if(port >= RTL8366RB_PORT_MAX)
		return ERRNO_API_INPUT;

	/* clean CPU port first */
	if(rtl8368s_setAsicCpuPortMask(0x00) != SUCCESS)
		return FAILED;	


	if(rtl8368s_setAsicCpuPortMask(0x1<<port) != SUCCESS)
		return FAILED;		

	if(rtl8368s_setAsicCpuDisableInsTag(noTag) != SUCCESS)
		return FAILED;	

	return SUCCESS;
}

/*
@func int32 | rtl8366rb_getCPUPort | Get CPU port and its setting.
@parm uint32* | ptr_port | returned CPU port (0~7).
@parm uint32* | ptr_noTag | returned CPU port with insert tag ability 1:not insert 0:insert.
@rvalue ERRNO_API_INPUT | invalid input parameters.
@rvalue ERRNO_API_CPUNOTSET | No CPU port speicifed.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	The API can get configured CPU port and its setting.
	Return ERRNO_API_CPUNOTSET if CPU port is not speicifed.
*/
int32 rtl8366rb_getCPUPort(uint32 *ptr_port, uint32 *ptr_noTag)
{
	uint32 i;
	uint32 portmsk;	
	
	if(ptr_port == NULL || ptr_noTag == NULL)
		return ERRNO_API_INPUT;

	/* get configured CPU port */
	if(rtl8368s_getAsicCpuPortMask(&portmsk) != SUCCESS)
		return FAILED;	

	if(portmsk)
	{
		/*Return first CPU port only*/
		for(i=0;i<=PORT7;i++)
		{
			if(portmsk & (1<<i))
			{
				*ptr_port = i;
				break;
			}		
		}		
	}		
	else
		return ERRNO_API_CPUNOTSET;
		
	if(rtl8368s_getAsicCpuDisableInsTag(ptr_noTag) != SUCCESS)
		return FAILED;				
			
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
@func int32 | rtl8366rb_getVlan | Get a VLAN entry.
@parm rtl8366rb_vlanConfig_t* | ptr_vlancfg | The pointer of VLAN configuration.
@rvalue ERRNO_API_INPUT | invalid input parameters.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	The API can get the member set and untag set settings for specified vid.
*/
int32 rtl8366rb_getVlan(rtl8366rb_vlanConfig_t *ptr_vlancfg)
{
	rtl8368s_user_vlan4kentry vlan4K;
	
    if(ptr_vlancfg == NULL)
        return ERRNO_INPUT;
	
	/* vid must be 0~4095 */
	if(ptr_vlancfg->vid > RTL8366RB_VIDMAX)
		return ERRNO_VID;

	vlan4K.vid = ptr_vlancfg->vid;
	if(rtl8368s_getAsicVlan4kEntry(&vlan4K) != SUCCESS)
		return FAILED;

	ptr_vlancfg->mbrmsk   = vlan4K.member;
	ptr_vlancfg->untagmsk = vlan4K.untag;	
	ptr_vlancfg->fid      = vlan4K.fid;
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
@func int32 | rtl8366rb_getVlanPVID | Get VLAN ID(PVID) on specified port.
@parm uint32 | port | Port number (0~5).
@parm uint32* | ptr_vid | pointer to returned VLAN ID.
@parm uint32* | ptr_priority | pointer to returned 802.1p priority.
@rvalue ERRNO_API_INPUT | invalid input parameters.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	The API can get the PVID and 802.1p priority for the PVID of Port-based VLAN.
*/
int32 rtl8366rb_getVlanPVID(uint32 port, uint32 *ptr_vid, uint32 *ptr_priority)
{
	uint32 index;
	rtl8368s_vlanconfig vlanMC;	

	if(port >= RTL8366RB_PORT_MAX)
		return ERRNO_PORT_NUM;
	
	if(rtl8368s_getAsicVlanPortBasedVID(port, &index) != SUCCESS)
		return FAILED;	

	if(rtl8368s_getAsicVlanMemberConfig(index, &vlanMC) != SUCCESS)
		return FAILED;

	*ptr_vid = vlanMC.vid;
	*ptr_priority = vlanMC.priority;	
	return SUCCESS;
}

/*
@func int32 | rtl8366rb_setVlanIngressFilter | Set VLAN ingress for each port.
@parm uint32 | port | Physical port number (0~5).
@parm uint32 | enabled | VLAN ingress function 1:enable 0:disable.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	Length/type of 802.1q VLAN tag is 0x8100. While VLAN function is enabled, 
    ASIC will decide VLAN ID for each received frame and get belonged member
	ports from VLAN table. If received port is not belonged to VLAN member ports, 
    ASIC will drop received frame if VLAN ingress function is enabled.
*/
int32 rtl8366rb_setVlanIngressFilter(uint32 port,uint32 enabled)
{
	if(port >= RTL8366RB_PORT_MAX)
		return ERRNO_PORT_NUM;

	if(SUCCESS != rtl8368s_setAsicVlanIngressFiltering(port,enabled))
		return FAILED;

	return SUCCESS;
}

/*
@func int32 | rtl8366rb_setVlanAcceptFrameType | Set VLAN support frame type
@parm uint32 | port | Physical port number (0~5).
@parm rtl8366rb_accept_frame_t | type | Supported frame types 0:all 1:C-tage frame 2:un-tag frame
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	Length/type of 802.1q VLAN tag is 0x8100. While VLAN function is enabled, 
    ASIC will parse length/type of reveived frame to check if there is a existed
	C-tag to permit frame or not. This function will *NOT* work while ASIC VLAN 
    function is disabled(ASIC will accept all types of frame). Function of accept 
    un-tag frame setting is not adapted to uplink port because only S-tag 
    (the first length/type field is not C-tag) frame will be permitted at uplink port.
	Function of accept C-tag frame setting is not adapted to uplink port and CPU port 
    because frame contained S-tag(0x88a8) or CPU tag (0x8899) will also be dropped.
	User sould take care the side effect of VLAN tag filtering function in the port 
    belongs to CPU port or uplink port. 
*/
int32 rtl8366rb_setVlanAcceptFrameType(uint32 port, rtl8366rb_accept_frame_t type)
{
	if(port >= RTL8366RB_PORT_MAX)
		return ERRNO_PORT_NUM;

	switch(type)
	{
 		case RTL8366RB_FRAME_TYPE_ALL:
			if(SUCCESS != rtl8368s_setAsicVlanDropTaggedPackets(port,DISABLE))
				return FAILED;
			
			if(SUCCESS != rtl8368s_setAsicVlanAcceptTaggedOnly(port,DISABLE))
				return FAILED;
			break;
 		case RTL8366RB_FRAME_TYPE_CTAG:
			if(SUCCESS != rtl8368s_setAsicVlanDropTaggedPackets(port,DISABLE))
				return FAILED;
			
			if(SUCCESS != rtl8368s_setAsicVlanAcceptTaggedOnly(port,ENABLE))
				return FAILED;
			break;
 		case RTL8366RB_FRAME_TYPE_UNTAG:
			if(SUCCESS != rtl8368s_setAsicVlanDropTaggedPackets(port,ENABLE))
				return FAILED;
			
			if(SUCCESS != rtl8368s_setAsicVlanAcceptTaggedOnly(port,DISABLE))
				return FAILED;
			break;
		default:
			return FAILED;
	}

	return SUCCESS;
}


/*
@func int32 | rtl8366rb_setVlanKeepCtagFormat | Set VLAN keep receiving tag/untag format in ingress port.
@parm uint32 | ingressport | Ingress port number (0~5).
@uint32 | portmask | Igress port mask (0x00-0x3F).
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@rvalue ERRNO_INPUT | Invalid port number.
@comm
	Output frame from ASIC will not be tagged C-tag or untagged 
    C-tag if egress port was set enable keep-Ctag-format function. 
    Receiving frame with untag format will be output with untag 
    format, priority-tag frame will be output as priority-tag 
    frame and c-tagged frame will be output as original c-tagged 
    frame. This configuration is configured per-ingress-port basis. 
    Users can specify engress port mask for a ingress port. 
*/
int32 rtl8366rb_setVlanKeepCtagFormat(uint32 ingressport, uint32 portmask)
{
    if(SUCCESS != rtl8368s_setAsicVlanKeepCtagFormat(ingressport, portmask))
		return FAILED;

	return SUCCESS;
}



/*
@func int32 | rtl8366rb_addLUTUnicast | Set LUT unicast entry.
@parm rtl8366rb_l2_entry* | ptr_l2_entry | The pointer of L2 entry.
@rvalue ERRNO_API_INPUT | invalid input parameters.
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
int32 rtl8366rb_addLUTUnicast(rtl8366rb_l2_entry *ptr_l2_entry)
{
	uint32 i;
	rtl8368s_l2table l2Table;
		
    if(ptr_l2_entry == NULL)
        return ERRNO_INPUT;

	/* must be unicast address */
	if((ptr_l2_entry->mac.octet == NULL) || (ptr_l2_entry->mac.octet[0] & 0x1))
		return ERRNO_MAC;

	if(ptr_l2_entry->spa >= PORT_MAX)
		return ERRNO_PORT_NUM;	

	if(ptr_l2_entry->fid > RTL8366RB_FIDMAX)
		return ERRNO_FID;	

	/*
	  Scan four-ways to see if the unicast mac address already existed.
	  If the unicast mac address already existed, update the port of the entry.	  
	  Scanning priority of four way is entry 0 > entry 1 > entry 2 > entry 3.	  
	*/
	for(i = 0; i < 4; i++)
	{
		/* fill key(MAC,FID) to get L2 entry */
		_rtl8366rb_copy(l2Table.mac.octet, ptr_l2_entry->mac.octet, ETHER_ADDR_LEN);
		l2Table.fid = ptr_l2_entry->fid;
	
		if(rtl8368s_getAsicL2LookupTb(i, &l2Table) != SUCCESS)
		{					
			return FAILED;
		}	
		else if(_rtl8366rb_cmp(ptr_l2_entry->mac.octet, l2Table.mac.octet, ETHER_ADDR_LEN) == SUCCESS)
		{
			l2Table.fid = ptr_l2_entry->fid;
			l2Table.mbr = (1<<ptr_l2_entry->spa);
			l2Table.auth = FALSE;
			l2Table.swst = TRUE;
			l2Table.ipmulti = FALSE;
            l2Table.ifdel = 0;
			if(rtl8368s_setAsicL2LookupTb(i, &l2Table) != SUCCESS)
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
		_rtl8366rb_copy(l2Table.mac.octet, ptr_l2_entry->mac.octet, ETHER_ADDR_LEN);
		l2Table.fid = ptr_l2_entry->fid;			
	
		if(rtl8368s_getAsicL2LookupTb(i, &l2Table) != SUCCESS)
		{					
			return FAILED;
		}	
		else if((l2Table.swst == FALSE)&&
				 (l2Table.auth == FALSE)&&
				 (l2Table.ipmulti == FALSE))
		{
			_rtl8366rb_copy(l2Table.mac.octet, ptr_l2_entry->mac.octet, ETHER_ADDR_LEN);
			l2Table.fid = ptr_l2_entry->fid;		
			l2Table.mbr = (1<<ptr_l2_entry->spa);
			l2Table.auth = FALSE;
			l2Table.swst = TRUE;
			l2Table.ipmulti = FALSE;
            l2Table.ifdel = 0;
			if(rtl8368s_setAsicL2LookupTb(i, &l2Table) != SUCCESS)
				return FAILED;
			else
				return SUCCESS;				
		}
	}

	/* four-ways are all full */	
	return ERRNO_LUT;
}

/*
@func int32 | rtl8366rb_addLUTMulticast | Set LUT multicast entry.
@parm rtl8366rb_l2_entry* | ptr_l2_entry | The pointer of Multicast entry.
@rvalue ERRNO_API_INPUT | invalid input parameters.
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
int32 rtl8366rb_addLUTMulticast(rtl8366rb_l2_entry *ptr_l2_entry)
{
	uint32 i;
	rtl8368s_l2table l2Table;

    if(ptr_l2_entry == NULL)
        return ERRNO_INPUT;

	/* must be L2 multicast address */
	if((ptr_l2_entry->mac.octet == NULL) || !(ptr_l2_entry->mac.octet[0] & 0x1))
		return ERRNO_MAC;

	if(ptr_l2_entry->mbr > RTL8366RB_PORTMASK)
		return ERRNO_PORT_NUM;	

    if(ptr_l2_entry->fid > RTL8366RB_FIDMAX)
		return ERRNO_FID;	

	/*
	  Scan four-ways to see if the multicast mac address already existed.
	  If the multicast mac address already existed, update the port mask of the entry.
	  Scanning priority of four way is entry 0 > entry 1 > entry 2 > entry 3.	  
	*/
	for(i = 0; i < 4; i++)
	{
		/* fill key(MAC,FID) to get L2 entry */
		_rtl8366rb_copy(l2Table.mac.octet, ptr_l2_entry->mac.octet, ETHER_ADDR_LEN);
		l2Table.fid = ptr_l2_entry->fid;			
	
		if(rtl8368s_getAsicL2LookupTb(i, &l2Table) != SUCCESS)
		{					
			return FAILED;
		}	
		else if(_rtl8366rb_cmp(ptr_l2_entry->mac.octet, l2Table.mac.octet, ETHER_ADDR_LEN) == SUCCESS)
		{
			l2Table.fid = ptr_l2_entry->fid;		
			l2Table.mbr = ptr_l2_entry->mbr;
			l2Table.auth = FALSE;
			l2Table.swst = TRUE;
			l2Table.ipmulti = FALSE;
            l2Table.ifdel = 0;
			if(rtl8368s_setAsicL2LookupTb(i, &l2Table) != SUCCESS)
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
		_rtl8366rb_copy(l2Table.mac.octet, ptr_l2_entry->mac.octet, ETHER_ADDR_LEN);
		l2Table.fid = ptr_l2_entry->fid;			
	
		if(rtl8368s_getAsicL2LookupTb(i, &l2Table) != SUCCESS)
		{					
			return FAILED;
		}	
		else if((l2Table.swst == FALSE)&&
				 (l2Table.auth == FALSE)&&
				 (l2Table.ipmulti == FALSE))
		{
			_rtl8366rb_copy(l2Table.mac.octet, ptr_l2_entry->mac.octet, ETHER_ADDR_LEN);
			l2Table.fid = ptr_l2_entry->fid;		
			l2Table.mbr = ptr_l2_entry->mbr;
			l2Table.auth = FALSE;
			l2Table.swst = TRUE;
			l2Table.ipmulti = FALSE;
            l2Table.ifdel = 0;
			if(rtl8368s_setAsicL2LookupTb(i, &l2Table) != SUCCESS)
				return FAILED;
			else
				return SUCCESS;				
		}
	}

	/* four-ways are all full */	
	return ERRNO_LUT;
}

/*
@func int32 | rtl8366rb_delLUTMACAddress | Delete LUT unicast/multicast entry.
@parm rtl8366rb_l2_entry* | ptr_l2_entry | The pointer of filtering database for the mac address to be deleted.
@rvalue ERRNO_API_INPUT | invalid input parameters.
@rvalue ERRNO_API_LUTNOTEXIST | No such LUT entry.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	If the mac has existed in the LUT, it will be deleted.
	Otherwise, it will return ERRNO_API_LUTNOTEXIST.
*/
int32 rtl8366rb_delLUTMACAddress(rtl8366rb_l2_entry *ptr_l2_entry)
{
	uint32 i;
	rtl8368s_l2table l2Table;
		
    if(ptr_l2_entry == NULL)
        return ERRNO_INPUT;

    if(ptr_l2_entry->mac.octet == NULL)
		return ERRNO_MAC;

	if(ptr_l2_entry->fid > RTL8366RB_FIDMAX)
		return ERRNO_FID;

	/* Scan four-ways to find the specified entry */  
	for(i = 0; i < 4; i++)
	{
		/* fill key(MAC,FID) to get L2 entry */
		_rtl8366rb_copy(l2Table.mac.octet, ptr_l2_entry->mac.octet, ETHER_ADDR_LEN);
		l2Table.fid = ptr_l2_entry->fid;
	
		if(rtl8368s_getAsicL2LookupTb(i, &l2Table) != SUCCESS)
		{					
			return FAILED;
		}	
		else if(_rtl8366rb_cmp(ptr_l2_entry->mac.octet, l2Table.mac.octet, ETHER_ADDR_LEN) == SUCCESS &&
				 l2Table.swst == TRUE)
		{
            l2Table.ifdel = TRUE;
			if(rtl8368s_setAsicL2LookupTb(i, &l2Table) != SUCCESS)
				return FAILED;
			else
				return SUCCESS;				
		}
	}

	/* No such LUT entry */	
	return ERRNO_LUT;
}

/*
@func int32 | rtl8366rb_getLUTUnicast | Get LUT unicast entry.
@parm rtl8368s_l2_entry* | ptr_l2_entry | The return pointer of filtering database.
@rvalue ERRNO_API_INPUT | invalid input parameters.
@rvalue ERRNO_API_LUTNOTEXIST | No such LUT entry.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure.
@comm
	If the unicast mac address existed in the LUT, it will 
    return the port where the mac is learned and blocking state. 
    Otherwise, it will return a ERRNO_API_LUTNOTEXIST error.
*/
int32 rtl8366rb_getLUTUnicast(rtl8366rb_l2_entry *ptr_l2_entry)
{
	uint32 i;
	rtl8368s_l2table l2Table;
		
    if(ptr_l2_entry == NULL)
        return ERRNO_INPUT;

    if(ptr_l2_entry->mac.octet == NULL)
		return ERRNO_MAC;

	/* Scan four-ways to see if the entry existed */
	for(i = 0; i < 4; i++)
	{
		/* fill key(MAC,FID) to get L2 entry */
		_rtl8366rb_copy(l2Table.mac.octet, ptr_l2_entry->mac.octet, ETHER_ADDR_LEN);
		l2Table.fid = ptr_l2_entry->fid;
	
		if(rtl8368s_getAsicL2LookupTb(i, &l2Table) != SUCCESS)
		{					
			return FAILED;
		}	
		else if(_rtl8366rb_cmp(ptr_l2_entry->mac.octet, l2Table.mac.octet, ETHER_ADDR_LEN) == SUCCESS)
		{
            ptr_l2_entry->swst = l2Table.swst;
            if(ptr_l2_entry->swst == TRUE) /* static entry */
            {
                ptr_l2_entry->mbr = l2Table.mbr;
                ptr_l2_entry->block = l2Table.block;
            }
            else
            {
                if(l2Table.age == 0) /* Dynamic entry && age == 0 */
                    return ERRNO_API_LUTNOTEXIST;

                ptr_l2_entry->spa   = l2Table.spa;
                ptr_l2_entry->block = l2Table.block;
            }

            return SUCCESS;
		}
	}

	/* Entry not found */	
	return ERRNO_API_LUTNOTEXIST;
}

/*
@func int32 | rtl8366rb_setAgeTimerSpeed | Set LUT agging out speed
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
int32 rtl8366rb_setAgeTimerSpeed( uint32 timer, uint32 speed)
{
    return rtl8368s_setAsicAgeTimerSpeed(timer, speed);
}

/*
@func int32 | rtl8368s_getAgeTimerSpeed | Get LUT agging out speed
@parm uint32* | timer | Agging out timer 0:Has been aged out.
@parm uint32* | speed | Agging out speed 0-fastest 3-slowest.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue ERRNO_INPUT | Invalid parameter.
@comm
 	The API can get LUT agging out period for each entry. 
			Timer		1		2		3		4		5		6		7
	Speed
	0					14.3s	28.6s	42.9s	57.2s	1.19m	1.43m	1.67m
	1					28.6s	57.2s	1.43m	1.9m	2.38m	2.86m	3.34m
	2					57.2s	1.9m	2.86m	3.81m	4.77m	5.72m	6.68m
	3					1.9m	3.8m	5.72m	7.63m	9.54m	11.45m	13.36m
 	(s:Second m:Minute)
 */
int32 rtl8366rb_getAgeTimerSpeed( uint32* timer, uint32* speed)
{
    return rtl8368s_getAsicAgeTimerSpeed(timer, speed);
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
@func int32 | rtl8366rb_setLedMode | Set Led to congiuration mode
@parm rtl8366rb_led_mode_t | mode | Support mode0,mode1,mode2,mode3 and force mode only.
@parm uint32 | ledG0Msk | Turn on or turn off Leds of group 0, 1:on 0:off.
@parm uint32 | ledG1Msk | Turn on or turn off Leds of group 1, 1:on 0:off.
@parm uint32 | ledG2Msk | Turn on or turn off Leds of group 2, 1:on 0:off.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	While led mode was set to FORCE, user can force turn on or turn off dedicted leds of each port. There are three LEDs for each port for indicating information
	about dedicated port. LEDs is set to indicate different information of port in different mode.

	Mode0
		LED0-Link, Activity Indicator. Low for link established. Link/Act Blinks when the corresponding port is transmitting or receiving.
		LED1-1000Mb/s Speed Indicator. Low for 1000Mb/s.
		LED2-100Mb/s Speed Indicator. Low for 100Mb/s.

	Mode1
		LED0-Link, Activity Indicator. Low for link established. Link/Act Blinks when the corresponding port is transmitting or receiving.
		LED1-1000Mb/s Speed Indicator. Low for 1000Mb/s.
		LED2-Collision, Full duplex Indicator. Blinking when collision happens. Low for full duplex, and high for half duplex mode.

	Mode2
		LED0-Collision, Full duplex Indicator. Blinking when collision happens. Low for full duplex, and high for half duplex mode.
		LED1-1000Mb/s Speed/Activity Indicator. Low for 1000Mb/s. Blinks when the corresponding port is transmitting or receiving.
		LED2-10/100Mb/s Speed/Activity Indicator. Low for 10/100Mb/s. Blinks when the corresponding port is transmitting or receiving.

	Mode3
		LED0-10Mb/s Speed/Activity Indicator. Low for 10Mb/s. Blinks when the corresponding port is transmitting or receiving.
		LED1-1000Mb/s Speed/Activity Indicator. Low for 1000Mb/s. Blinks when the corresponding port is transmitting or receiving.
		LED2-100Mb/s Speed/Activity Indicator. Low for 100Mb/s. Blinks every the corresponding port is transmitting or receiving.

	Force: All Leds can be turned on or off by software configuration.
	
	
*/
int32 rtl8366rb_setLedMode(rtl8366rb_led_mode_t mode, rtl8366rb_LedConfig_t *ptr_ledgroup)
{
	uint16 ledGroup;
	const uint32 ledConfig[RTL8366RB_LED_MODE_MAX][RTL8366RB_LED_GROUP_MAX] = { 
					{LEDCONF_LINK_ACT,		LEDCONF_SPD1000,		LEDCONF_SPD100,			LEDCONF_SPD10},
					{LEDCONF_LINK_ACT,		LEDCONF_SPD1000,		LEDCONF_DUPCOL,		LEDCONF_SPD100},
					{LEDCONF_DUPCOL,		LEDCONF_SPD1000ACT,	LEDCONF_SPD10010ACT,	LEDCONF_LEDOFF},
					{LEDCONF_SPD10ACT,		LEDCONF_SPD1000ACT,	LEDCONF_SPD100ACT,		LEDCONF_DUPCOL},
					{LEDCONF_LEDFORCE,		LEDCONF_LEDFORCE,		LEDCONF_LEDFORCE,		LEDCONF_LEDFORCE}
					};
		

	if(mode >= RTL8366RB_LED_MODE_MAX)
		return FAILED;


	for(ledGroup= 0;ledGroup<RTL8366RB_LED_GROUP_MAX;ledGroup++)
	{
		if(SUCCESS != rtl8368s_setAsicLedIndicateInfoConfig(ledGroup,ledConfig[mode][ledGroup]))
			return FAILED;
	}

	if(mode != RTL8366RB_LED_MODE_FORCE)
		return SUCCESS;

	if(SUCCESS != rtl8368s_setAsicForceLeds(ptr_ledgroup->ledG0Msk,ptr_ledgroup->ledG1Msk,ptr_ledgroup->ledG2Msk,0))
		return FAILED;

	return SUCCESS;
}


/*
@func int32 | rtl8366rb_setLedBlinkRate | Set LED blinking rate at mode 0 to mode 3
@parm rtl8366rb_led_blinkrate_t | blinkRate | Support 6 blinking rate.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	ASIC support 6 types of LED blinking rates at 28ms, 56ms, 84ms, 111ms, 222ms and 446ms.
*/
int32 rtl8366rb_setLedBlinkRate(rtl8366rb_led_blinkrate_t blinkRate)
{
	if(blinkRate >= RTL8366RB_LEDBLINKRATE_MAX)
		return FAILED;

	if(SUCCESS != rtl8368s_setAsicLedBlinkRate(blinkRate))
		return FAILED;

	return SUCCESS;
}

/*
@func int32 | rtl8366rb_setRtctTesting | Set port RTCT cable length detection start.
@parm uint32 | operation | Operation mode
@parm enum PORTID | port | Port number to be set as CPU port (0~4).
@rvalue ERRNO_API_INVALIDPARAM | invalid input parameters.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	Detect dedicated port cable length by ASIC with desired channel. 
    After start RTCT detection testing, user must wait more tan 2 
    seconds to get detected result.
*/
int32 rtl8366rb_setRtctTesting(uint32 operation, enum PORTID port)
{
	if(port >= PORT_MAX)
		return ERRNO_INPUT;

    if(operation == 1)
    {
        rtl8368s_setAsicPHYRegs(0, 0, 21, 0x0007);
        rtl8368s_setAsicPHYRegs(1, 0, 21, 0x0007);
        rtl8368s_setAsicPHYRegs(2, 0, 21, 0x0007);
        rtl8368s_setAsicPHYRegs(3, 0, 21, 0x0007);
        rtl8368s_setAsicPHYRegs(4, 0, 21, 0x0007);
    }
		
	if(SUCCESS != rtl8368s_setAsicCableTesting(port, ENABLE))
		return FAILED;

	return SUCCESS;
}
/*
@func int32 | rtl8366rb_getRtctResult | Get port RTCT cable length detection result
@parm uint32 | operation | Operation mode
@parm enum PORTID | port | Port number to be set as CPU port (0~4).
@parm enum CHANNELID | channel | Detected channel id (0~3}.
@parm uint32* |length | Cable length. (unit meter)
@parm uint32* |impmis | Channel impedance mismatch status 1: impedance mismatch, 0: normal
@parm uint32* |openSts | Open status of previous RTCT detection result.
@parm uint32* |shortSts | Short status of previous RTCT detection result.
@rvalue ERRNO_API_INVALIDPARAM | invalid input parameters.
@rvalue ERRNO_API_RTCT_NOT_FINISHED | Still in RTCT detection state.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	After start RTCT detection testing, user must wait more than 
    2 seconds to get detected result. 
*/
int32 rtl8366rb_getRtctResult(uint32 operation, enum PORTID port,enum CHANNELID channel, uint32* length, uint32* impmis, uint32* openSts, uint32* shortSts)
{
    if(port >= PORT_MAX)
		return ERRNO_INPUT;

    if(SUCCESS != rtl8368s_getAsicRtctChannelStatus(port, channel, impmis, openSts, shortSts, length))
		return FAILED;

    if(operation == 1)
    {
        rtl8368s_setAsicPHYRegs(0, 0, 21, 0x1007);
        rtl8368s_setAsicPHYRegs(1, 0, 21, 0x1007);
        rtl8368s_setAsicPHYRegs(2, 0, 21, 0x1007);
        rtl8368s_setAsicPHYRegs(3, 0, 21, 0x1007);
        rtl8368s_setAsicPHYRegs(4, 0, 21, 0x1007);
    }

    return SUCCESS;
}

/*
@func int32 | rtl8366rb_setReservedMulticastAddress | Set reserved multicast address frame trap to CPU.
@parm rtl8366rb_rma_frame_t | rma | type of RMA for trapping frame type setting.
@parm enum RMAOP | op | RMA operation 0:forwarding 1:Trap to CPU 2:Drop 3:Forward, exclude CPU.
@parm uint32 | keepCtag | Keep CVLAN tag/untag format 1:enable 0:disable.
@parm uint32 | bypassStorm | By pass storm control calculation 1:enable 0:disable.
@parm uint32 | priSel | New CPU priority assignment 1:enable 0:disable.
@parm enum PRIORITYVALUE | priority | Priority for new CPU priority assignment. This CPU priority will not be remapped.
@rvalue ERRNO_INPUT | Invalid input parameter.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	There are 23 types of Reserved Multicast Address frame for trapping to CPU for application usage. They are as following definition.

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
	xx-xx-xx-xx-xx-xx				IGMPin PPPoE frame(Type/Length = 8864)
	xx-xx-xx-xx-xx-xx				MLD(IPv6 ICMP) in PPPoE frame(Type/Length = 8864)
	xx-xx-xx-xx-xx-xx				IGMP packet but not PPPoE frame
	xx-xx-xx-xx-xx-xx				MLD(IPv6 ICMP) but not PPPoE frame
	xx-xx-xx-xx-xx-xx				User defined address 1
	xx-xx-xx-xx-xx-xx				User defined address 2
	xx-xx-xx-xx-xx-xx				User defined address 3
	xx-xx-xx-xx-xx-xx				Unknow source address


	
*/
int32 rtl8366rb_setReservedMulticastAddress(rtl8366rb_rma_frame_t rma, rtl8366rb_rmaConfig_t *ptr_rmacfg)
{
	if(rma> RMA_MAX)
		return ERROR_RMA_INDEX;

	if(ptr_rmacfg->op >=RMAOP_MAX)
		return ERRNO_INPUT;
	
	if(ptr_rmacfg->priority >=PRI_MAX)
		return ERRNO_PRIORITY;

	if(SUCCESS != rtl8368s_setAsicRma(rma,ptr_rmacfg->op,ptr_rmacfg->keepCtag,ptr_rmacfg->bypassStorm,ptr_rmacfg->priSel,ptr_rmacfg->priority))
		return FAILED;		

	return SUCCESS;
}

/*
@func int32 | rtl8366rb_getReservedMulticastAddress | Set reserved multicast address frame trap to CPU.
@parm rtl8366rb_rma_frame_t | rma | type of RMA for trapping frame type setting.
@parm enum RMAOP* | op | RMA operation 0:forwarding 1:Trap to CPU 2:Drop 3:Forward, exclude CPU.
@parm uint32* | keepCtag | Keep CVLAN tag/untag format 1:enable 0:disable.
@parm uint32* | bypassStorm | By pass storm control calculation 1:enable 0:disable.
@parm uint32* | priSel | New CPU priority assignment 1:enable 0:disable.
@parm enum* PRIORITYVALUE | priority | Priority for new CPU priority assignment. This CPU priority will not be remapped.
@rvalue ERRNO_INPUT | Invalid input parameter.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	The API can retrieved RMA configuration. 
*/
int32 rtl8366rb_getReservedMulticastAddress(rtl8366rb_rma_frame_t rma, rtl8366rb_rmaConfig_t *ptr_rmacfg)
{
	if(rma> RMA_MAX)
		return ERRNO_INPUT;


	if(SUCCESS != rtl8368s_getAsicRma(rma,&ptr_rmacfg->op,&ptr_rmacfg->keepCtag,&ptr_rmacfg->bypassStorm,&ptr_rmacfg->priSel,&ptr_rmacfg->priority))
		return FAILED;		

	return SUCCESS;
}


/*
@func int32 | rtl8366rb_setPortMirror | Configure port mirror function.
@parm enum RTL8366RB_PORTID | rtl8366rb_portMirror_t->sourcePort | Source port.
@parm enum RTL8366RB_PORTID | rtl8366rb_portMirror_t->monitorPort | Monitor port.
@parm uint32 | rtl8366rb_portMirror_t->mirRx | Rx mirror function 1:enable 0:disable.
@parm uint32 | rtl8366rb_portMirror_t->mirTx | Tx mirror function 1:enable 0:disable. 
@parm uint32 | rtl8366rb_portMirror_t->mirIso | Mirror isolation for monitor port 1:enable 0:disable. 
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	ASIC supports one port mirror function only. For mirror function, there are one mirrored port and one monitor port. Frame from/to source port
	can be mirrored to monitor port and mirror port can be set with receiving frame from source port only. Mirror configuration is as below description.
	Field					Description
	sourcePort			Desired source port which receiving/transmitting frame to be mirrored 
	monitorPort			Monitor port which frames were mirrored to.
	mirRx				Function setting of mirroring frames received in source port.
	mirTx				Funciton setting of mirroring frames transmitted from source port.
	mirIso				Setting which permit forwarding frame from source port only.
*/
int32 rtl8366rb_setPortMirror(rtl8366rb_portMirror_t *ptr_mirrorcfg)
{
	if(SUCCESS != rtl8368s_setAsicPortMirror(ptr_mirrorcfg->sourcePort,ptr_mirrorcfg->monitorPort))
		return FAILED;		

	if(SUCCESS != rtl8368s_setAsicPortMirrorRxFunction(ptr_mirrorcfg->mirrorRx))
		return FAILED;		

	if(SUCCESS != rtl8368s_setAsicPortMirrorTxFunction(ptr_mirrorcfg->mirrorTx))
		return FAILED;		

	if(SUCCESS != rtl8368s_setAsicPortMirrorIsolation(ptr_mirrorcfg->mirrorIso))
		return FAILED;		

	return SUCCESS;

}
/*
@func int32 | rtl8366rb_initAcl | ACL function initialization.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	Set ACL function disable at each port. There are shared 32 rules and 
    12 kinds of rule format  {mac(0), ipv4(1), ipv4 icmp(2), ipv4 igmp(3), 
    ipv4 tcp(4), ipv4 udp(5), ipv6 sip(6), ipv6 dip(7), ipv6 ext(8), 
    ipv6 tcp(9), ipv6 udp(10), ipv6 icmp(11)} for ingress filtering. 
    Using ACL matched action fields combination of mirror/redirect and 
    port mask to PERMIT, DROP, REDIRECT and MIRROR packets which matching ACL rule.
*/
int32 rtl8366rb_initAcl(void)
{
	uint32 port;
	rtl8368s_acltable aclRule;
	uint32	idx;

	memset(&aclRule, 0, sizeof(rtl8368s_acltable));

	for(idx=0;idx<=RTL8366RB_ACLIDXMAX;idx++)
	{
		if(SUCCESS != rtl8368s_setAsicAclRule(idx,aclRule))
			return FAILED;			
	}


	for(port = 0;port<RTL8366RB_PORT_MAX;port ++)
	{
		/*Enable ACL function in default*/
		if(SUCCESS != rtl8368s_setAsicAcl(port,RTL8366RB_ACL_DEFAULT_ABILITY))
			return FAILED;
		/*set unmatched frame to permit*/
		if(SUCCESS != rtl8368s_setAsicAclUnmatchedPermit(port,ACL_DEFAULT_UNMATCH_PERMIT))
			return FAILED;
		/*allocated shared ACL space in default range */
		if(SUCCESS != rtl8368s_setAsicAclStartEnd(port,0,RTL8366RB_ACLIDXMAX))
			return FAILED;
	}

	return SUCCESS;
}
	
/*
@func int32 | rtl8366rb_addAclRule | Add ACL rule to dedicated port.
@parm rtl8366rb_acltable_t* | ptr_aclTable | ACL rule stucture for setting.
@rvalue ERRNO_API_INPUT | invalid input parameters.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	Adding new rule to ACL function disabled port will automatic 
    enable ACL function usage and locate new rule to available 
    rule index. If adding rule format and all rule fields (exclude 
    opeartion and action fields) are matching to existed rule, 
    then operation and action fields of existed rule will be replace 
    by new adding rule and ACL rule number of port is the same.

*/
int32 rtl8366rb_addAclRule(rtl8366rb_acltable_t *ptr_aclTable)
{
	uint32 idx;
	rtl8368s_acltable aclTemp;

	for(idx=0;idx<=RTL8366RB_ACLIDXMAX;idx++)
	{
		if(SUCCESS != rtl8368s_getAsicAclRule(idx,&aclTemp))
			return FAILED;
		/*Add ACL rule to port-disabled index*/
		if (aclTemp.port_en==0)
		{
			rtl8368s_setAsicAclRule(idx, *ptr_aclTable);
			break;
		}
		/*If ACL index is not available, return fail */
		else if (idx==RTL8366RB_ACLIDXMAX)
			return FAILED;
	}
	
	return SUCCESS;
}

/*
@func int32 | rtl8366rb_delAclRule | Delete ACL rule from dedicated port.
@parm rtl8366rb_acltable_t* | ptr_aclTable | ACL rule stucture for setting.
@rvalue ERRNO_API_INPUT | invalid input parameters.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	Only all rule fields (exculde opration and action fields) match 
    existed rule, then existed matching ACL rule will be deleted. 
    ACL function usage of port will be disabled atfer deleting last 
    ative rule in setting port.
*/
int32 rtl8366rb_delAclRule(rtl8366rb_acltable_t *ptr_aclTable)
{
	uint32 aclIdx;
	rtl8368s_acltable aclRuleCmp;

	for(aclIdx=0;aclIdx<=RTL8366RB_ACLIDXMAX;aclIdx++)
	{
		if(SUCCESS != rtl8368s_getAsicAclRule(aclIdx,&aclRuleCmp))
			return FAILED;
		if(SUCCESS == _rtl8366rb_cmpAclRule(ptr_aclTable,&aclRuleCmp))
		{
			memset(&aclRuleCmp, 0, sizeof(rtl8368s_acltable));
			if(SUCCESS != rtl8368s_setAsicAclRule(aclIdx,aclRuleCmp))	
				return FAILED;
			return SUCCESS;
		
		}
		else if(aclIdx==RTL8366RB_ACLIDXMAX) 
			return FAILED;
	}

	return SUCCESS;
}

/*
@func int32 | rtl8366rb_setPort4RGMIIControl | Set LED/Interrupt Control for P4 RGMII mode
@parm uint32 | enabled | 1:Enable LED/Interrupt for P4 RGMII, 0:Disable LED/Interrupt for P4 RGMII
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@comm
	Set LED/Interrupt Control for P4 RGMII mode
*/
int32 rtl8366rb_setPort4RGMIIControl(uint32 enabled)
{
    return rtl8368s_setAiscPort4RGMIIControl(enabled);
}

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
@func int32 | rtl8366rb_getMac5ForceLink | Set Port/MAC 5 force linking configuration.
@parm enum PORTLINKSPEED* | speed | Speed of the port 0b00-10M, 0b01-100M,0b10-1000M, 0b11 reserved.
@parm enum PORTLINKDUPLEXMODE* | duplex | Deuplex mode 0b0-half duplex 0b1-full duplex.
@parm uint32* | link | Link status 0b0-link down b1-link up.
@parm uint32* | txPause | Pause frame transmit ability of the port 0b0-inactive 0b1-active.
@parm uint32* | rxPause | Pause frame response ability of the port 0b0-inactive 0b1-active.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue FAILED | Invalid parameter.
@comm
  	This API can set Port/MAC 5 properties in force mode. 
 */
int32 rtl8366rb_getMac5ForceLink(rtl8366rb_macConfig_t *ptr_maccfg)
{
	uint32 autoNegotiation;
	if(SUCCESS != rtl8368s_getAsicPortLinkState(PORT5,&ptr_maccfg->speed,&ptr_maccfg->duplex,&ptr_maccfg->link,&ptr_maccfg->txPause,&ptr_maccfg->rxPause,&autoNegotiation))
		return FAILED;

	return SUCCESS;
}



/*
@func int32 | rtl8366rb_setStormFiltering | Set per port  storm filtering function.
@parm rtl8366rb_stormfilter_t* | ptr_stormftr | The structure of Sotrm filtering function.
@rvalue ERRNO_API_INPUT | Input NULL pointer.
@rvalue ERRNO_PORT_NUM | Invalid Port Number.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	The API can be used to enable or disable per-port storm filtering 
    control. There are three kinds of storm filtering control which 
    are broadcasting storm control, multicast storm control and unknown 
    unicast storm control. There are dedicated storm control setting 
    for each port and global storm control meter for each one reference. 
    There is still a didicated reference meter counter in each port.
*/
int32 rtl8366rb_setStormFiltering(rtl8366rb_stormfilter_t *ptr_stormftr)
{
    if(ptr_stormftr == NULL)
		return ERRNO_API_INPUT;
	
	if(ptr_stormftr->port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	if(SUCCESS != rtl8368s_setAsicStormFiltering(ptr_stormftr->port, ptr_stormftr->bcstorm, ptr_stormftr->mcstorm, ptr_stormftr->undastorm, ptr_stormftr->unmcstorm))
		return FAILED;

	if(SUCCESS != rtl8368s_setAsicStormFilteringMeter(ptr_stormftr->port, ptr_stormftr->rate, ptr_stormftr->ifg))
		return FAILED;

	return SUCCESS;
}

/*
@func int32 | rtl8366rb_getStormFiltering | Get per port storm filtering function.
@parm rtl8366rb_stormfilter_t* | ptr_stormftr | The structure of Sotrm filtering function.
@rvalue ERRNO_API_INPUT | Input NULL pointer.
@rvalue ERRNO_PORT_NUM | Invalid Port Number.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	The API can be used to get per-port storm filtering control. 
    There are four kinds of storm filtering control which are 
    broadcasting storm control, multicast storm control unknown 
    unicast storm control and unknown multicast storm control. 
    There are dedicated storm control setting for each port 
    and global storm control meter for each one reference. 
    There is still a didicated reference meter counter in each port.
*/
int32 rtl8366rb_getStormFiltering(rtl8366rb_stormfilter_t *ptr_stormftr)
{
    if(ptr_stormftr == NULL)
        return ERRNO_API_INPUT;

	if(ptr_stormftr->port >=PORT_MAX)
		return ERRNO_PORT_NUM;

	if(SUCCESS != rtl8368s_getAsicStormFiltering(ptr_stormftr->port,
                                                 &(ptr_stormftr->bcstorm),
                                                 &(ptr_stormftr->mcstorm),
                                                 &(ptr_stormftr->undastorm),
                                                 &(ptr_stormftr->unmcstorm)))
		return FAILED;

	if(SUCCESS != rtl8368s_getAsicStormFilteringMeter(ptr_stormftr->port,
                                                      &(ptr_stormftr->rate),
                                                      &(ptr_stormftr->ifg)))
		return FAILED;
		
	return SUCCESS;
}


/*
@func int32 | rtl8366rb_setPortIsolation | Set port isolation function  usage configuration.
@parm enum RTL8366RB_PORTID | ptr_isocfg->port | Egress port number (0~8).
@parm uint32 | ptr_isocfg->enable | Isolation function usage 1:enable, 0:disabled.
@parm uint32 | ptr_isocfg->portmask | Output port mask (0-8).
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMI | SMI access error.
@rvalue FAILED | Invalid parameter.
@comm
	The API can isolation source port output to output ports 
    represented by port mask. It is enabled per port. When 
	the API is used, packets from source port will not deliver 
    to other ports that are not in port mask. Other ports not 
    in port mask can still deliver packets to source port. 
    By using isolation setting, user can isolate physical ports' 
    group without VLAN configuration. This function will physically 
    seperate networking flow and isolate network within desired ports only.
 */

int32 rtl8366rb_setPortIsolation(rtl8366rb_isolation_t *ptr_isocfg)
{

	if(ptr_isocfg->port >=RTL8366RB_PORT_MAX)
		return ERRNO_INPUT;

	if(rtl8368s_setAsicPortIsolation(ptr_isocfg->port, ptr_isocfg->enable)!=SUCCESS)
		return FAILED;

	if(rtl8368s_setAsicPortIsolationConfig(ptr_isocfg->port, ptr_isocfg->portmask)!=SUCCESS)
		return FAILED;


	return SUCCESS;
}
/*
@func int32 | rtl8366rb_getPortIsolation | Get ASIC port isolation control configuration. 
@parm enum RTL8366RB_PORTID | ptr_isocfg->port | Source port number.
@parm uint32 | ptr_isocfg->enable | Isolation function usage
@parm uint32 | ptr_isocfg-> portmask | Output port mask.
@rvalue SUCCESS | Success.
@rvalue ERRNO_SMIERROR | SMI access error.
@rvalue ERRNO_INPUT | Invalid port number.
@comm
 	The API can be used to get the port isolation configuration.
*/

int32 rtl8366rb_getPortIsolation(rtl8366rb_isolation_t *ptr_isocfg)
{

	if(ptr_isocfg->port >=RTL8366RB_PORT_MAX)
		return ERRNO_INPUT;

	if(rtl8368s_getAsicPortIsolation(ptr_isocfg->port, &ptr_isocfg->enable)!=SUCCESS)
		return FAILED;

	if(rtl8368s_getAsicPortIsolationConfig(ptr_isocfg->port, &ptr_isocfg->portmask)!=SUCCESS)
		return FAILED;


	return SUCCESS;
}

/*
@func int32 | rtl8366rb_setSourceMacLearning | Set per-port source mac learning function.
@parm uint32 | port | Physical port number (0~5).
@parm uint32 | enabled | Source mac learning function 1:enable 0:disable.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	Like CPU port usage, the source mac of frame from CPU port 
    will be learned if this frame is just traped to CPU port 
    with frame modification only and re-send from CPU. This frame 
    will be learned twiced and the source port information of 
    L2 entry about this frame maybe is wrong. 
*/
int32 rtl8366rb_setSourceMacLearning(uint32 port, uint32 enabled)
{
    if(port >= RTL8366RB_PORT_MAX)
		return FAILED;

    if(SUCCESS != rtl8368s_setAsicPortLearnDisable(port, enabled?FALSE:TRUE))
		return FAILED;

	return SUCCESS;
}

/*
@func int32 | rtl8366rb_getSourceMacLearning | Get per-port source mac learning function.
@parm uint32 | port | Physical port number (0~5).
@parm uint32* | ptr_enabled | Source mac learning function 1:enable 0:disable.
@rvalue SUCCESS | Success.
@rvalue FAILED | Failure. 
@comm
	Like CPU port usage, the source mac of frame from CPU port 
    will be learned if this frame is just traped to CPU port 
    with frame modification only and re-send from CPU. This frame 
    will be learned twiced and the source port information of L2 
    entry about this frame maybe is wrong. 
*/
int32 rtl8366rb_getSourceMacLearning(uint32 port, uint32 *ptr_enabled)
{
    uint32  disabled;

	if(port >= RTL8366RB_PORT_MAX)
		return FAILED;

    if(SUCCESS != rtl8368s_getAsicPortLearnDisable(port, &disabled))
		return FAILED;

    *ptr_enabled = (disabled ? 0 : 1);
    return SUCCESS;
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


