/*
* Copyright c                  Realtek Semiconductor Corporation, 2007  
* All rights reserved.                                                    
* 
* Program : The internal header file of high level API
* Abstract :                                                           
* Author : Nick Wu (nickwu@realtek.com.tw)               
* $Id: rtl8366s_api.h,v 1.15 2007/10/22 08:24:23 hmchung Exp $
*/

#ifndef __RTL8366S_API_H__
#define __RTL8366S_API_H__

#include <linux/delay.h>

#include "rtl8366s_types.h"


#define ENABLE									1
#define DISABLE									0

#define PHY_CONTROL_REG						0
#define PHY_STATUS_REG							1
#define PHY_AN_ADVERTISEMENT_REG				4
#define PHY_AN_LINKPARTNER_REG				5
#define PHY_1000_BASET_CONTROL_REG			9
#define PHY_1000_BASET_STATUS_REG			10


/*Qos related configuration define*/
#define QOS_DEFAULT_TICK_PERIOD			       (19-1)
#define QOS_DEFAULT_BYTE_PER_TOKEN			34
#define QOS_DEFAULT_LK_THRESHOLD				(34*3) /* Why use 0x400? */


#define QOS_DEFAULT_INGRESS_BANDWIDTH  		0x3FFF /* 0x3FFF => unlimit */
#define QOS_DEFAULT_EGRESS_BANDWIDTH  		0x3B9B /*( 0x3D08 + 1) * 64Kbps => 1Gbps*/
#define QOS_DEFAULT_PREIFP		  				1
#define QOS_DEFAULT_PACKET_USED_PAGES_FC	0x60
#define QOS_DEFAULT_PACKET_USED_FC_EN		0
#define QOS_DEFAULT_QUEUE_BASED_FC_EN		0

#define QOS_DEFAULT_PRIORITY_SELECT_PORT		8
#define QOS_DEFAULT_PRIORITY_SELECT_1Q		0
#define QOS_DEFAULT_PRIORITY_SELECT_ACL		0
#define QOS_DEFAULT_PRIORITY_SELECT_DSCP		0

#define QOS_DEFAULT_PORT_BASED_PRIORITY		0
#define QOS_DEFAULT_DSCP_MAPPING_PRIORITY	0

#define QOS_DEFAULT_1Q_REMARKING_ABILITY	0
#define QOS_DEFAULT_DSCP_REMARKING_ABILITY	0
#define QOS_DEFAULT_QUEUE_GAP				20
#define QOS_DEFAULT_QUEUE_NO_MAX			6
#define QOS_DEFAULT_AVERAGE_PACKET_RATE		0x3FFF
#define QOS_DEFAULT_BURST_SIZE_IN_APR		0x3F
#define QOS_DEFAULT_PEAK_PACKET_RATE			2
#define QOS_DEFAULT_SCHEDULER_ABILITY_APR	0 	/*enable*/
#define QOS_DEFAULT_SCHEDULER_ABILITY_PPR	0	/*enable*/
#define QOS_DEFAULT_SCHEDULER_ABILITY_WFQ	0	/*enable*/

#define QOS_WEIGHT_MAX						128



#define LED_GROUP_MAX							3

#define ACL_DEFAULT_ABILITY					0
#define ACL_DEFAULT_UNMATCH_PERMIT			1

#define ACL_RULE_FREE							0
#define ACL_RULE_INAVAILABLE					1



enum FRAMETYPE{

	FRAME_TYPE_ALL = 0,
	FRAME_TYPE_CTAG,
	FRAME_TYPE_UNTAG,
};	

enum PHY_TESTMODE{

       PHY_NORMAL_MODE = 0,
       PHY_TEST_MODE1,
       PHY_TEST_MODE2,
       PHY_TEST_MODE3,
       PHY_TEST_MODE4,
       PHY_TEST_MODE_MAX = PHY_TEST_MODE4,

};

enum CHIP_MODEL{

       CHIP_B1 = 0,
       CHIP_B2,
 
};

enum LED_MODE{

	LED_MODE_0 = 0,
	LED_MODE_1,
	LED_MODE_2,
	LED_MODE_3,
	LED_MODE_FORCE,
	LED_MODE_MAX,

};	

enum SFC_PERIOD{

	SFC_PERIOD_450MS = 0,
	SFC_PERIOD_900MS,
	SFC_PERIOD_1800MS,
	SFC_PERIOD_3600MS,

};	

enum SFC_COUNT{

	SFC_COUNT_32PKTS = 0,
	SFC_COUNT_64PKTS,
	SFC_COUNT_128PKTS,
	SFC_COUNT_255PKTS,

};	

typedef struct phyAbility_s
{
	uint16	AutoNegotiation:1;/*PHY register 0.12 setting for auto-negotiation process*/
	uint16	Half_10:1;		/*PHY register 4.5 setting for 10BASE-TX half duplex capable*/
	uint16	Full_10:1;		/*PHY register 4.6 setting for 10BASE-TX full duplex capable*/
	uint16	Half_100:1;		/*PHY register 4.7 setting for 100BASE-TX half duplex capable*/
	uint16	Full_100:1;		/*PHY register 4.8 setting for 100BASE-TX full duplex capable*/
	uint16	Full_1000:1;		/*PHY register 9.9 setting for 1000BASE-T full duplex capable*/
	uint16	FC:1;			/*PHY register 4.10 setting for flow control capability*/
	uint16	AsyFC:1;		/*PHY register 4.11 setting for  asymmetric flow control capability*/
} phyAbility_t;


typedef struct portMirror_s
{
	enum PORTID sourcePort;
	enum PORTID monitorPort;
	uint16 mirrorRx;
	uint16 mirrorTx;
	uint16 mirrorIso;

} portMirror_t;

typedef struct portPriority_s
{
	enum PRIORITYVALUE priPort0;
	enum PRIORITYVALUE priPort1;
	enum PRIORITYVALUE priPort2;
	enum PRIORITYVALUE priPort3;
	enum PRIORITYVALUE priPort4;
	enum PRIORITYVALUE priPort5;

} portPriority_t;

typedef struct dot1qPriority_s
{
	enum PRIORITYVALUE dot1qPri0;
	enum PRIORITYVALUE dot1qPri1;
	enum PRIORITYVALUE dot1qPri2;
	enum PRIORITYVALUE dot1qPri3;
	enum PRIORITYVALUE dot1qPri4;
	enum PRIORITYVALUE dot1qPri5;
	enum PRIORITYVALUE dot1qPri6;
	enum PRIORITYVALUE dot1qPri7;

} dot1qPriority_t;

typedef struct pri2Qid_s
{
	enum QUEUEID pri0;
	enum QUEUEID pri1;
	enum QUEUEID pri2;
	enum QUEUEID pri3;
	enum QUEUEID pri4;
	enum QUEUEID pri5;
	enum QUEUEID pri6;
	enum QUEUEID pri7;

} pri2Qid_t;


typedef struct qConfig_s
{
	enum QUEUETYPE strickWfq[6];	
	uint32 weight[6];

} qConfig_t;



#endif /* __RTL8366S_API_H__ */

