#ifndef _RTL8366RB_API_H_
#define _RTL8366RB_API_H_

#include "rtl8368s_types.h"
#include "rtl8368s_asicdrv.h"

#define ENABLE									1
#define DISABLE									0

#define RTL8366RB_PHY_CONTROL_REG               0
#define RTL8366RB_PHY_STATUS_REG                1
#define RTL8366RB_PHY_AN_ADVERTISEMENT_REG      4
#define RTL8366RB_PHY_AN_LINKPARTNER_REG        5
#define RTL8366RB_PHY_1000_BASET_CONTROL_REG    9
#define RTL8366RB_PHY_1000_BASET_STATUS_REG     10


/*Qos related configuration define*/
#define QOS_DEFAULT_TICK_PERIOD             (19-1)
#define QOS_DEFAULT_BYTE_PER_TOKEN          34
#define QOS_DEFAULT_LK_THRESHOLD            (34*3) /* Why use 0x400? */


#define QOS_DEFAULT_INGRESS_BANDWIDTH       0x3FFF /* 0x3FFF => unlimit */
#define QOS_DEFAULT_EGRESS_BANDWIDTH        0x3B9B /*( 0x3D08 + 1) * 64Kbps => 1Gbps*/
#define QOS_DEFAULT_PREIFP                  1
#define QOS_DEFAULT_PACKET_USED_PAGES_FC    0x60
#define QOS_DEFAULT_PACKET_USED_FC_EN       0
#define QOS_DEFAULT_QUEUE_BASED_FC_EN       1

#define QOS_DEFAULT_PRIORITY_SELECT_PORT    1
#define QOS_DEFAULT_PRIORITY_SELECT_1Q      2
#define QOS_DEFAULT_PRIORITY_SELECT_ACL     8
#define QOS_DEFAULT_PRIORITY_SELECT_DSCP    4

#define QOS_DEFAULT_DSCP_MAPPING_PRIORITY   0

#define QOS_DEFAULT_1Q_REMARKING_ABILITY    0
#define QOS_DEFAULT_DSCP_REMARKING_ABILITY  0
#define QOS_DEFAULT_QUEUE_GAP               20
#define QOS_DEFAULT_QUEUE_NO_MAX            6
#define QOS_DEFAULT_AVERAGE_PACKET_RATE     0x3FFF
#define QOS_DEFAULT_BURST_SIZE_IN_APR       0x3F
#define QOS_DEFAULT_PEAK_PACKET_RATE        2
#define QOS_DEFAULT_SCHEDULER_ABILITY_APR   1 	/*disable*/
#define QOS_DEFAULT_SCHEDULER_ABILITY_PPR   1	/*disable*/
#define QOS_DEFAULT_SCHEDULER_ABILITY_WFQ   1	/*disable*/

#define QOS_WEIGHT_MAX                      128

#define LED_GROUP_MAX                       3

#define ACL_DEFAULT_ABILITY                 1
#define ACL_DEFAULT_UNMATCH_PERMIT          1

#define ACL_RULE_FREE                       0
#define ACL_RULE_INAVAILABLE                1


#define RTL8366RB_REGBITLENGTH              16
#define RTL8366RB_RMAUSERDEFMAX             4
#define RTL8366RB_VLANMCIDXMAX              15
#define RTL8366RB_FIDMAX                    7
#define RTL8366RB_VIDMAX                    0xFFF
#define RTL8366RB_PRIORITYMAX               7
#define RTL8366RB_PORTMASK                  0x3F
#define RTL8366RB_SPTSTSMASK                0x03
#define RTL8366RB_L2ENTRYMAX                3
#define RTL8366RB_CAMENTRYMAX               7
#define RTL8366RB_SVLANIDXMAX               3
#define RTL8366RB_PPBIDXMAX                 3
#define RTL8366RB_LAHASHVALMAX              7
#define RTL8366RB_LAHASHSELMAX              2
#define RTL8366RB_LAPORTSMAX                4
#define RTL8366RB_DSCPMAX                   63
#define RTL8366RB_DSCPMIN                   0

#define RTL8366RB_SCPERIODMAX               3
#define RTL8366RB_SCCOUNTMAX                3

#define RTL8366RB_ACLIDXMAX                 15
#define RTL8366RB_ACLDATALENGTH             6
#define RTL8366RB_ACLDATA0WRITE             0x0901
#define RTL8366RB_ACLDATA1WRITE             0x0B01
#define RTL8366RB_ACLDATA2WRITE             0x0D01
#define RTL8366RB_ACLDATA0READ              0x0801
#define RTL8366RB_ACLDATA1READ              0x0A01
#define RTL8366RB_ACLDATA2READ              0x0C01

#define RTL8366RB_PHY_NO_MAX                4
#define RTL8366RB_PHY_PAGE_MAX              7
#define RTL8366RB_PHY_ADDR_MAX              31

#define RTL8366RB_QOS_BWCTL_DISABLE         0x3FFF
#define RTL8368RB_QOS_BWCTL_MIN             64
#define RTL8368RB_QOS_BWCTL_MAX             1048576

#define RTL8366RB_LED_GROUP_MAX             4



/* storm filtering control */
#define RTL8366RB_STORM_FILTERING_1_REG         0x02E2	
#define RTL8366RB_STORM_FILTERING_PERIOD_BIT    0
#define RTL8366RB_STORM_FILTERING_PERIOD_MSK    0x0003
#define RTL8366RB_STORM_FILTERING_COUNT_BIT     2
#define RTL8366RB_STORM_FILTERING_COUNT_MSK     0x000C
#define RTL8366RB_STORM_FILTERING_BC_BIT        8

#define RTL8366RB_STORM_FILTERING_2_REG         0x02E3	
#define RTL8366RB_STORM_FILTERING_MC_BIT        0
#define RTL8366RB_STORM_FILTERING_UNDA_BIT      8

/* ACL registers address */
#define RTL8366RB_ACL_CONTROL_REG           0x0300
#define RTL8366RB_ACL_INC_ING_BIT           12
#define RTL8366RB_ACL_RANGE_REG_BASE        0x0301

#define RTL8366RB_ACL_METER_BASE            0x0305
#define RTL8366RB_ACL_METER_RATE_MSK        0x3FFF
#define RTL8366RB_ACL_METER_TOLE_MSK        0x003F

#define RTL8366RB_ACL_DEFAULT_ABILITY       0

#define RTL8366RB_PORT_MAX                  6

typedef enum RTL8366RB_FRAMETYPE
{

    RTL8366RB_FRAME_TYPE_ALL = 0,
    RTL8366RB_FRAME_TYPE_CTAG,
    RTL8366RB_FRAME_TYPE_UNTAG,
}rtl8366rb_accept_frame_t;  

enum RTL8366RB_PHY_TESTMODE
{

    RTL8366RB_PHY_NORMAL_MODE = 0,
    RTL8366RB_PHY_TEST_MODE1,
    RTL8366RB_PHY_TEST_MODE2,
    RTL8366RB_PHY_TEST_MODE3,
    RTL8366RB_PHY_TEST_MODE4,
    RTL8366RB_PHY_TEST_MODE_MAX = RTL8366RB_PHY_TEST_MODE4,

};


typedef enum RTL8366RB_LED_MODE
{
    RTL8366RB_LED_MODE_0 = 0,
    RTL8366RB_LED_MODE_1,
    RTL8366RB_LED_MODE_2,
    RTL8366RB_LED_MODE_3,
    RTL8366RB_LED_MODE_FORCE,
    RTL8366RB_LED_MODE_MAX,
}rtl8366rb_led_mode_t;  

typedef enum RTL8366RB_LEDBLINKRATE{

	RTL8366RB_LEDBLINKRATE_28MS=0, 		
	RTL8366RB_LEDBLINKRATE_56MS,		
	RTL8366RB_LEDBLINKRATE_84MS,
	RTL8366RB_LEDBLINKRATE_111MS,
	RTL8366RB_LEDBLINKRATE_222MS,
	RTL8366RB_LEDBLINKRATE_446MS,
	RTL8366RB_LEDBLINKRATE_MAX,
}rtl8366rb_led_blinkrate_t;

typedef enum RMATRAPFRAME   rtl8366rb_rma_frame_t;
    
typedef rtl8368s_acltable   rtl8366rb_acltable_t;

typedef struct rtl8366rb_phyAbility_s
{
    uint16  AutoNegotiation:1;/*PHY register 0.12 setting for auto-negotiation process*/
    uint16  Half_10:1;      /*PHY register 4.5 setting for 10BASE-TX half duplex capable*/
    uint16  Full_10:1;      /*PHY register 4.6 setting for 10BASE-TX full duplex capable*/
    uint16  Half_100:1;     /*PHY register 4.7 setting for 100BASE-TX half duplex capable*/
    uint16  Full_100:1;     /*PHY register 4.8 setting for 100BASE-TX full duplex capable*/
    uint16  Full_1000:1;        /*PHY register 9.9 setting for 1000BASE-T full duplex capable*/
    uint16  FC:1;           /*PHY register 4.10 setting for flow control capability*/
    uint16  AsyFC:1;        /*PHY register 4.11 setting for  asymmetric flow control capability*/
} rtl8366rb_phyAbility_t;


typedef struct rtl8366rb_portMirror_s
{
    uint32 sourcePort;
    uint32 monitorPort;
    uint16 mirrorRx;
    uint16 mirrorTx;
    uint16 mirrorIso;

} rtl8366rb_portMirror_t;

typedef struct rtl8366rb_portPriority_s
{
    enum PRIORITYVALUE priPort0;
    enum PRIORITYVALUE priPort1;
    enum PRIORITYVALUE priPort2;
    enum PRIORITYVALUE priPort3;
    enum PRIORITYVALUE priPort4;
    enum PRIORITYVALUE priPort5;

} rtl8366rb_portPriority_t;

typedef struct rtl8366rb_dot1qPriority_s
{
    enum PRIORITYVALUE dot1qPri0;
    enum PRIORITYVALUE dot1qPri1;
    enum PRIORITYVALUE dot1qPri2;
    enum PRIORITYVALUE dot1qPri3;
    enum PRIORITYVALUE dot1qPri4;
    enum PRIORITYVALUE dot1qPri5;
    enum PRIORITYVALUE dot1qPri6;
    enum PRIORITYVALUE dot1qPri7;

} rtl8366rb_dot1qPriority_t;

typedef struct rtl8366rb_pri2Qid_s
{
    enum QUEUEID pri0;
    enum QUEUEID pri1;
    enum QUEUEID pri2;
    enum QUEUEID pri3;
    enum QUEUEID pri4;
    enum QUEUEID pri5;
    enum QUEUEID pri6;
    enum QUEUEID pri7;

} rtl8366rb_pri2Qid_t;


typedef struct rtl8366rb_qConfig_s
{
    enum QUEUETYPE strickWfq[6];    
    uint32 weight[6];

} rtl8366rb_qConfig_t;


typedef struct rtl8366rb_LedConfig_s
{
    uint32 ledG0Msk;
    uint32 ledG1Msk;
    uint32 ledG2Msk;
} rtl8366rb_LedConfig_t;

typedef struct rtl8366rb_rmaConfig_s
{
    enum RMAOP op;
    uint32 keepCtag;
    uint32 bypassStorm;
    uint32 priSel;
    enum PRIORITYVALUE priority;
} rtl8366rb_rmaConfig_t;

typedef struct rtl8366rb_macConfig_s
{
    enum MACLINKMODE force;
    enum PORTLINKSPEED speed;
    enum PORTLINKDUPLEXMODE duplex;
    uint32 link;
    uint32 txPause;
    uint32 rxPause;
} rtl8366rb_macConfig_t;

typedef struct 
{
    uint32 vid;
    uint32 mbrmsk;
    uint32 untagmsk;
    uint32 fid;
} rtl8366rb_vlanConfig_t;


typedef struct
{
    ipaddr_t      sip;
    ipaddr_t      dip;
    ether_addr_t  mac;
    uint16        fid;
    uint16        mbr;
    uint16        block;
    uint16        spa;
    uint16        age;
    uint16        auth;
    uint16        swst;
    uint16        ipmulti;
} rtl8366rb_l2_entry;

typedef struct
{
    uint32      port;
    uint32      bcstorm;
    uint32      mcstorm;
    uint32      undastorm;
    uint32      unmcstorm;
    uint32      rate;
    uint32      ifg;
} rtl8366rb_stormfilter_t;


typedef struct
{
    uint32 port;
    uint32 enable;
    uint32 portmask;
} rtl8366rb_isolation_t;

typedef struct rtl8366rb_interruptConfig_s
{
    uint32 polarity;
    uint32 linkChangedPorts;
    uint32 aclExceed;
    uint32 stormExceed;
} rtl8366rb_interruptConfig_t;

typedef enum RTL8366RB_MIBCOUNTER
{
	rtl8366rb_IfInOctets_cnt = 0,
	rtl8366rb_EtherStatsOctets_cnt,
	rtl8366rb_EtherStatsUnderSizePkts_cnt,
	rtl8366rb_EtherStatsFragments_cnt,
	rtl8366rb_EtherStatsPkts64Octets_cnt,
	rtl8366rb_EtherStatsPkts65to127Octets_cnt,
	rtl8366rb_EtherStatsPkts128to255Octets_cnt,
	rtl8366rb_EtherStatsPkts256to511Octets_cnt,
	rtl8366rb_EtherStatsPkts512to1023Octets_cnt,
	rtl8366rb_EtherStatsPkts1024to1518Octets_cnt,
	rtl8366rb_EtherOversizeStats_cnt,
	rtl8366rb_EtherStatsJabbers_cnt,
	rtl8366rb_IfInUcastPkts_cnt,
	rtl8366rb_EtherStatsMulticastPkts_cnt,
	rtl8366rb_EtherStatsBroadcastPkts_cnt,
	rtl8366rb_EtherStatsDropEvents_cnt,
	rtl8366rb_Dot3StatsFCSErrors_cnt,
	rtl8366rb_Dot3StatsSymbolErrors_cnt,
	rtl8366rb_Dot3InPauseFrames_cnt,
	rtl8366rb_Dot3ControlInUnknownOpcodes_cnt,
	rtl8366rb_IfOutOctets_cnt,
	rtl8366rb_Dot3StatsSingleCollisionFrames_cnt,
	rtl8366rb_Dot3StatMultipleCollisionFrames_cnt,
	rtl8366rb_Dot3sDeferredTransmissions_cnt,
	rtl8366rb_Dot3StatsLateCollisions_cnt,
	rtl8366rb_EtherStatsCollisions_cnt,
	rtl8366rb_Dot3StatsExcessiveCollisions_cnt,
	rtl8366rb_Dot3OutPauseFrames_cnt,
	rtl8366rb_Dot1dBasePortDelayExceededDiscards_cnt,
	rtl8366rb_Dot1dTpPortInDiscards_cnt,
	rtl8366rb_IfOutUcastPkts_cnt,
	rtl8366rb_IfOutMulticastPkts_cnt,
	rtl8366rb_IfOutBroadcastPkts_cnt,
	rtl8366rb_OutOampduPkts_cnt,
	rtl8366rb_InOampduPkts_cnt,
	rtl8366rb_PktgenPkts_cnt,
	/*Device only */	
	rtl8366rb_Dot1dTpLearnEntryDiscardFlag,
	RTL8366RB_MIBS_NUMBER,
}rtl8366rb_mibcounter_t;	

#endif /*#ifndef _RTL8366RB_API_H_*/

