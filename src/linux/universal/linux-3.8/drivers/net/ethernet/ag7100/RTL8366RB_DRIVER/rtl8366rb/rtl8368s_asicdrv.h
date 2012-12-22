#ifndef _RTL8368S_ASICDRV_H_
#define _RTL8368S_ASICDRV_H_

#include "rtl8368s_types.h"

#define RTL8368S_REGBITLENGTH					16
#define RTL8368S_ACLRULENO						32
#define RTL8368S_ACLINDEXMAX					(RTL8368S_ACLRULENO-1)
#define RTL8368S_RMAUSERDEFMAX				3
#define RTL8368S_VLANMCIDXMAX					15
#define RTL8368S_FIDMAX							7
#define RTL8368S_FIDMASK						0xFF
#define RTL8368S_VIDMAX						0xFFF
#define RTL8368S_PRIORITYMAX					7
#define RTL8368S_PORTMASK						0xFF
#define RTL8368S_L2ENTRYMAX					3
#define RTL8368S_CAMENTRYMAX					7
#define RTL8368S_SVIDXMAX						7
#define RTL8368S_PPBIDXMAX						3
#define RTL8368S_LAHASHVALMAX					7
#define RTL8368S_LAHASHSELMAX					2
#define RTL8368S_LAPORTSMAX					4
#define RTL8368S_DSCPMAX						63
#define RTL8368S_DSCPMIN						0
#define RTL8368S_VIDXMAX						15

#define RTL8368S_SCPERIODMAX					3
#define RTL8368S_SCCOUNTMAX					3

#define RTL8368S_ACL_METER_IDX_MAX			15
#define RTL8368S_ACL_DLEN1						7
#define RTL8368S_ACL_DLEN2					6


#define RTL8368S_PHY_NO_MAX					(5-1)
#define RTL8368S_PHY_PAGE_MAX					7	
#define RTL8368S_PHY_REG_MAX					31	

#define RTL8368S_PHY_EXT_ADDRESS_MAX			31	


#define RTL8368S_QOS_BWCTL_DISABLE			0x3FFF
#define RTL8368S_QOS_BWCTL_MIN				64
#define RTL8368S_QOS_BWCTL_MAX				1048576

#define RTL8368S_LED_GROUP_MAX				4	

#define RTL8368S_AGE_SPEED_MAX				3	
#define RTL8368S_AGE_TIMER_MAX				7	

#define RTL8368S_TBL_CMD_CHECK_COUNTER		1000

/* enum for port ID */
enum PORTID
{
	PORT0 =  0,
	PORT1,
	PORT2,
	PORT3,
	PORT4,
	PORT5,
	PORT6,	
	PORT7,	
	PORT_MAX
};

/* enum for port ability speed */
enum PORTABILITYSPEED
{
	SPD_10M_H = 1,
	SPD_10M_F,
	SPD_100M_H,
	SPD_100M_F,
	SPD_1000M_F
};

/* enum for port current link speed */
enum PORTLINKSPEED
{
	SPD_10M = 0,
	SPD_100M,
	SPD_1000M
};

/* enum for mac link mode */
enum MACLINKMODE
{
	MAC_NORMAL = 0,
	MAC_FORCE,
};

/* enum for port current link duplex mode */
enum PORTLINKDUPLEXMODE
{
	HALF_DUPLEX = 0,
	FULL_DUPLEX
};

/* spanning tree state */
enum SPTSTATE
{
	DISABLED = 0,
	BLOCKING,
	LEARNING,
	FORWARDING
};

enum ACLFORMAT
{
	ACL_MAC = 0,
	ACL_IPV4,
	ACL_IPV4_ICMP,
	ACL_IPV4_IGMP,
	ACL_IPV4_TCP,
	ACL_IPV4_UDP,
	ACL_IPV6_SIP,	
	ACL_IPV6_DIP,	
	ACL_IPV6_EXT,	
	ACL_IPV6_TCP,	
	ACL_IPV6_UDP,	
	ACL_IPV6_ICMP,	
	ACL_DUMB = 15,
};

/* enum for output queue number */
enum QUEUENUM
{
	QNUM1 = 1,
	QNUM2,
	QNUM3,
	QNUM4,
	QNUM5,
	QNUM6,
};

/* enum for queue ID */
enum QUEUEID
{
	QUEUE0 = 0,
	QUEUE1,
	QUEUE2,
	QUEUE3,
	QUEUE4,
	QUEUE5,
	QUEUE_MAX,
};

/* enum for queue type */
enum QUEUETYPE
{
	STR_PRIO = 0,
	WFQ_PRIO,
};

/* enum for priority value type */
enum PRIORITYVALUE
{
	PRI0 = 0,
	PRI1,
	PRI2,
	PRI3,
	PRI4,
	PRI5,
	PRI6,
	PRI7,
	PRI_MAX,
};

/* enum for RMA trapping frame type */
enum RMATRAPFRAME
{
	RMA_BRG_GROUP = 0,
	RMA_FD_PAUSE,
	RMA_SP_MCAST,
	RMA_1X_PAE,
	RMA_PROVIDER_BRIDGE_GROUP_ADDRESS,
	RMA_PROVIDER_BRIDGE_GVRP_ADDRESS,
	RMA_8021AB,
	RMA_BRG_MNGEMENT,
	RMA_GMRP,
	RMA_GVRP,
	RMA_UNDEF_BRG,
	RMA_UNDEF_GARP,
	RMA_IGMP_PPPOE,
	RMA_MLD_PPPOE,
	RMA_IGMP,
	RMA_MLD,
	RMA_UNKOWNIPV6MC,
	RMA_UNKOWNIPV4MC,
	RMA_UNKOWNL2MC,
	RMA_USER_DEF1,	
	RMA_USER_DEF2,	
	RMA_USER_DEF3,	
	RMA_UNKNOW_SA,	
	RMA_MAX,
};
enum RMAOP
{
	RMAOP_FORWARD = 0,
	RMAOP_TRAP_TO_CPU,
	RMAOP_DROP,
	RMAOP_FORWARD_EXCLUDE_CPU,
	RMAOP_MAX,
};

enum CHANNELID
{
	CH_A =  0,
	CH_B,
	CH_C,
	CH_D,
	CH_MAX
};

/* enum for priority value type */
enum FIDVALUE
{
	FID0 = 0,
	FID1,
	FID2,
	FID3,
	FID4,
	FID5,
	FID6,
	FID7,
	FID_MAX,
};


enum UNAUTH1XPROC
{
	UNAUTH_DROP = 0,
	UNAUTH_TRAP,
	UNAUTH_GUESTVLAN,
	UNAUTH_MAX,
};


enum OAMPARACT
{
	OAM_PARFWD = 0,
	OAM_PARLB,	
	OAM_PARDISCARD,
	OAM_PARFWDCPU,
};

enum OAMMULACT
{	
	OAM_MULFWD = 0,
	OAM_MULDISCARD,
	OAM_MULCPU,
};

typedef struct   smi_ether_addr_s{

#ifdef _LITTLE_ENDIAN
	uint16	mac0:8;
	uint16	mac1:8;
	uint16	mac2:8;
	uint16	mac3:8;
	uint16	mac4:8;
	uint16	mac5:8;
#else
	uint16	mac1:8;
	uint16	mac0:8;
	uint16	mac3:8;
	uint16	mac2:8;
	uint16	mac5:8;
	uint16	mac4:8;
#endif	
}smi_ether_addr_t;



typedef struct  VLANCONFIG{

#ifdef _LITTLE_ENDIAN
	uint16 	vid:12;
 	uint16 	priority:3;
 	uint16 	reserved1:1;

	uint16 	member:8;
 	uint16 	untag:8;
	
 	uint16 	fid:3;
	uint16 	reserved2:2;
	uint16	stag_idx:3;
	uint16	stag_mbr:8;

#else
  	uint16 	reserved2:1;
	uint16 	priority:3;
	uint16 	vid:12;

 	uint16 	untag:8;
 	uint16 	member:8;

	uint16	stag_mbr:8;
	uint16	stag_idx:3;
	uint16 	reserved1:2;
 	uint16 	fid:3;	

#endif
}rtl8368s_vlanconfig;


typedef struct  VLANTABLE{

#ifdef _LITTLE_ENDIAN
	uint16 	vid:12;
 	uint16 	reserved1:4;
	
 	uint16 	member:8;
 	uint16 	untag:8;
	
 	uint16 	fid:3;
	uint16 	reserved2:13;

#else
 	uint16 	reserved1:4;
	uint16 	vid:12;
	
 	uint16 	untag:8;
 	uint16 	member:8;
	
	uint16 	reserved2:13;
 	uint16 	fid:3;
	
#endif
}rtl8368s_vlan4kentry;

typedef struct  USER_VLANTABLE{

	uint16 	vid:12;
 	uint16 	member:8;
 	uint16 	untag:8;
 	uint16 	fid:3;

}rtl8368s_user_vlan4kentry;


struct smi_l2tb_ipmulticast_st{
	
#ifdef _LITTLE_ENDIAN
	uint16 	sip0:8;
	uint16 	sip1:8;
	uint16 	sip2:8;
	uint16 	sip3:8;
	uint16 	dip0:8;
	uint16 	dip1:8;
	uint16 	dip2:8;
	uint16 	dip3:8;

	uint16 	 mbr:8;
	uint16 	 reserved1:7;
	uint16 	 ipmulti:1;

	uint16 	 reserved2;

#else
	uint16 	sip2:8;
	uint16 	sip3:8;
	uint16 	sip0:8;
	uint16 	sip1:8;
	uint16 	dip2:8;
	uint16 	dip3:8;
	uint16 	dip0:8;
	uint16 	dip1:8;

	uint16 	 ipmulti:1;
	uint16 	 reserved1:7;
	uint16 	 mbr:8;

	uint16 	 reserved2;
#endif
	
};


struct smi_l2tb_macstatic_st{

#ifdef _LITTLE_ENDIAN

	uint16	mac0:8;
	uint16	mac1:8;
	uint16	mac2:8;
	uint16	mac3:8;
	uint16	mac4:8;
	uint16	mac5:8;

	uint16 	fid:3;
	uint16 	reserved1:13;

	uint16 	mbr:8;
	uint16 	reserved2:4;
	uint16 	block:1;
	uint16 	auth:1;
	uint16 	swst:1;
	uint16 	ipmulti:1;

	uint16 	reserved3;
#else
	uint16	mac1:8;
	uint16	mac0:8;
	uint16	mac3:8;
	uint16	mac2:8;
	uint16	mac5:8;
	uint16	mac4:8;

	uint16 	reserved1:13;
	uint16 	fid:3;

	uint16 	ipmulti:1;
	uint16 	swst:1;
	uint16 	auth:1;
	uint16	block:1;
	uint16 	reserved2:4;
	uint16 	mbr:8;

	uint16 	reserved3;
#endif	
};

struct smi_l2tb_maclearn_st{

#ifdef _LITTLE_ENDIAN

	uint16	mac0:8;
	uint16	mac1:8;
	uint16	mac2:8;
	uint16	mac3:8;
	uint16	mac4:8;
	uint16	mac5:8;

	uint16 	fid:3;
	uint16 	reserved1:13;

	uint16 	spa:3;
	uint16 	age:3;
	uint16 	reserved2:7;
	uint16 	auth:1;
	uint16 	swst:1;
	uint16 	ipmulti:1;
	
	uint16 	reserved3;
#else
	uint16	mac1:8;
	uint16	mac0:8;
	uint16	mac3:8;
	uint16	mac2:8;
	uint16	mac5:8;
	uint16	mac4:8;

	uint16 	reserved1:13;
	uint16 	fid:3;
	
	uint16 	ipmulti:1;
	uint16 	swst:1;
	uint16 	auth:1;
	uint16 	reserved2:7;
	uint16 	age:3;
	uint16 	spa:3;

	
	uint16 	reserved3;


#endif
};

struct l2tb_ipmulticast_st{
	
	ipaddr_t sip;
	ipaddr_t dip;

	uint16 	 mbr:8;
	uint16 	 reserved1:7;
	uint16 	 ipmulti:1;

	uint16 	 reserved2;
};

struct l2tb_macstatic_st{

	ether_addr_t 	mac;

	uint16 	fid:3;
	uint16 	reserved1:13;

	uint16 	mbr:8;
	uint16 	reserved2:4;
	uint16	block:1;
	uint16 	auth:1;
	uint16 	swst:1;
	uint16 	ipmulti:1;

	uint16 	reserved3;	
};

struct l2tb_maclearn_st{

	ether_addr_t 	mac;

	uint16 	fid:3;
	uint16 	reserved1:13;

	uint16 	spa:3;
	uint16 	age:3;
	uint16 	reserved2:7;
	uint16 	auth:1;
	uint16 	swst:1;
	uint16 	ipmulti:1;
	
	uint16 	reserved3;
};


typedef 	union L2SMITABLE{

	struct smi_l2tb_ipmulticast_st	smi_ipmul;
	struct smi_l2tb_macstatic_st	smi_swstatic;
	struct smi_l2tb_maclearn_st		smi_autolearn;
		
}rtl8368s_l2smitable;

typedef 	struct L2TABLE{
	
	ipaddr_t sip;
	ipaddr_t dip;
	ether_addr_t 	mac;
	uint16	fid:3;	
	uint16	mbr:8;
	uint16	reserve1:5;
	uint16	block:1;
	uint16 	spa:3;
	uint16 	age:3;
	uint16 	auth:1;
	uint16 	swst:1;
	uint16 	ipmulti:1;
	uint16	ifdel:1;
		
}rtl8368s_l2table;

typedef 	struct CAMTABLE{

	ether_addr_t 	mac;
	
	uint16 	fid:3;
	uint16 	mbr:8;
	uint16 	age:3;
	uint16	block:1;
	uint16 	auth:1;
	uint16 	swst:1;
	
}rtl8368s_camtable;


typedef 	struct CAMSMITABLE{

#ifdef _LITTLE_ENDIAN
	uint16	mac0:8;
	uint16	mac1:8;
	uint16	mac2:8;
	uint16	mac3:8;
	uint16	mac4:8;
	uint16	mac5:8;
	
	uint16 	fid:3;
	uint16 	reserved1:13;

	uint16 	mbr:8;
	uint16 	age:3;
	uint16 	reserved2:1;
	uint16	block:1;
	uint16 	auth:1;
	uint16 	swst:1;
	uint16 	reserved3:1;

#else
	uint16	mac1:8;
	uint16	mac0:8;
	uint16	mac3:8;
	uint16	mac2:8;
	uint16	mac5:8;
	uint16	mac4:8;

	uint16 	reserved1:13;
	uint16 	fid:3;

	uint16 	reserved3:1;
	uint16 	swst:1;
	uint16 	auth:1;
	uint16	block:1;
	uint16 	reserved2:1;
	uint16 	age:3;
	uint16 	mbr:8;

#endif	
}rtl8368s_camsmitable;


struct smi_mac_st{

#ifdef _LITTLE_ENDIAN

	uint16	dmp0:8;
	uint16	dmp1:8;
	uint16	dmp2:8;
	uint16	dmp3:8;
	uint16	dmp4:8;
	uint16	dmp5:8;

	uint16	dmm0:8;
	uint16	dmm1:8;
	uint16	dmm2:8;
	uint16	dmm3:8;
	uint16	dmm4:8;
	uint16	dmm5:8;

	uint16	smp0:8;
	uint16	smp1:8;
	uint16	smp2:8;
	uint16	smp3:8;
	uint16	smp4:8;
	uint16	smp5:8;

	uint16	smm0:8;
	uint16	smm1:8;
	uint16	smm2:8;
	uint16	smm3:8;
	uint16	smm4:8;
	uint16	smm5:8;

	uint16 	tlu;
	uint16 	tll;

	uint16	vtd:1;
	uint16	vtm:1;
	uint16	pu:3;
	uint16	pl:3;
	uint16	vidd_1:8;
	
	uint16	vidd_2:4;
	uint16	vidm:12;
#else
	uint16	dmp1:8;
	uint16	dmp0:8;
	uint16	dmp3:8;
	uint16	dmp2:8;
	uint16	dmp5:8;
	uint16	dmp4:8;

	uint16	dmm1:8;
	uint16	dmm0:8;
	uint16	dmm3:8;
	uint16	dmm2:8;
	uint16	dmm5:8;
	uint16	dmm4:8;

	uint16	smp1:8;
	uint16	smp0:8;
	uint16	smp3:8;
	uint16	smp2:8;
	uint16	smp5:8;
	uint16	smp4:8;

	uint16	smm1:8;
	uint16	smm0:8;
	uint16	smm3:8;
	uint16	smm2:8;
	uint16	smm5:8;
	uint16	smm4:8;
	
	uint16 	tlu;
	uint16 	tll;

	uint16	vidd_1:8;
	uint16	pl:3;
	uint16	pu:3;
	uint16	vtm:1;
	uint16	vtd:1;
	
	uint16	vidm:12;
	uint16	vidd_2:4;

#endif	
};	

struct mac_st{

	ether_addr_t 	dmp;
	ether_addr_t 	dmm;
	ether_addr_t 	smp;
	ether_addr_t 	smm;
	uint16 	tlu;
	uint16 	tll;
	uint16	vtd:1;
	uint16	vtm:1;
	uint16	pu:3;
	uint16	pl:3;
	uint16	vidd:12;
	uint16	vidm:12;

};	

struct smi_ipv4_st{
#ifdef _LITTLE_ENDIAN
	uint16 	sipU0:8;
	uint16 	sipU1:8;
	uint16 	sipU2:8;
	uint16 	sipU3:8;

	uint16 	sipL0:8;
	uint16 	sipL1:8;
	uint16 	sipL2:8;
	uint16 	sipL3:8;

	uint16 	dipU0:8;
	uint16 	dipU1:8;
	uint16 	dipU2:8;
	uint16 	dipU3:8;

	uint16 	dipL0:8;
	uint16 	dipL1:8;
	uint16 	dipL2:8;
	uint16 	dipL3:8;

	uint16	tosD:8;
	uint16	tosM:8;
	
	uint16	protoV:4;
	uint16	proto1:8;
	uint16	proto2_1:4;
	
	uint16	proto2_2:4;
	uint16	proto3:8;
	uint16	proto4_1:4;
	
	uint16	proto4_2:4;
	uint16	flagD:3;
	uint16	flagM:3;
	uint16	offU_1:6;

	uint16	offU_2:7;
	uint16	offL_1:9;
	
	uint16	offL_2:4;
	uint16	reserved:12;
#else
	uint16 	sipU2:8;
	uint16 	sipU3:8;
	uint16 	sipU0:8;
	uint16 	sipU1:8;

	uint16 	sipL2:8;
	uint16 	sipL3:8;
	uint16 	sipL0:8;
	uint16 	sipL1:8;

	uint16 	dipU2:8;
	uint16 	dipU3:8;
	uint16 	dipU0:8;
	uint16 	dipU1:8;

	uint16 	dipL2:8;
	uint16 	dipL3:8;
	uint16 	dipL0:8;
	uint16 	dipL1:8;

	uint16	tosM:8;
	uint16	tosD:8;
	
	uint16	proto2_1:4;
	uint16	proto1:8;
	uint16	protoV:4;
	
	uint16	proto4_1:4;
	uint16	proto3:8;
	uint16	proto2_2:4;
	
	uint16	offU_1:6;
	uint16	flagM:3;
	uint16	flagD:3;
	uint16	proto4_2:4;

	uint16	offL_1:9;
	uint16	offU_2:7;
	
	uint16	reserved:12;
	uint16	offL_2:4;
#endif
};

struct ipv4_st{

	ipaddr_t 	sipU;
	ipaddr_t 	sipL;
	ipaddr_t 	dipU;
	ipaddr_t 	dipL;
	uint16	tosD:8;
	uint16	tosM:8;
	uint16	protoV:4;
	uint16	proto1:8;
	uint16	proto2:8;
	uint16	proto3:8;
	uint16	proto4:8;
	uint16	flagD:3;
	uint16	flagM:3;
	uint16	offU:13;
	uint16	offL:13;
};


struct smi_ipv4_icmp_st{

#ifdef _LITTLE_ENDIAN
	uint16 	sipU0:8;
	uint16 	sipU1:8;
	uint16 	sipU2:8;
	uint16 	sipU3:8;

	uint16 	sipL0:8;
	uint16 	sipL1:8;
	uint16 	sipL2:8;
	uint16 	sipL3:8;

	uint16 	dipU0:8;
	uint16 	dipU1:8;
	uint16 	dipU2:8;
	uint16 	dipU3:8;

	uint16 	dipL0:8;
	uint16 	dipL1:8;
	uint16 	dipL2:8;
	uint16 	dipL3:8;

	uint16	tosD:8;
	uint16	tosM:8;
	
	uint16	typeV:4;
	uint16	type1:8;
	uint16	type2_1:4;
	
	uint16	type2_2:4;
	uint16	type3:8;
	uint16	type4_1:4;
	
	uint16	type4_2:4;
	uint16	codeV:4;
	uint16	code1:8;

	uint16	code2:8;
	uint16	code3:8;

	uint16	code4:8;
	uint16	reserved:8;
#else
	uint16 	sipU2:8;
	uint16 	sipU3:8;
	uint16 	sipU0:8;
	uint16 	sipU1:8;

	uint16 	sipL2:8;
	uint16 	sipL3:8;
	uint16 	sipL0:8;
	uint16 	sipL1:8;

	uint16 	dipU2:8;
	uint16 	dipU3:8;
	uint16 	dipU0:8;
	uint16 	dipU1:8;

	uint16 	dipL2:8;
	uint16 	dipL3:8;
	uint16 	dipL0:8;
	uint16 	dipL1:8;

	uint16	tosM:8;
	uint16	tosD:8;

	uint16	type2_1:4;	
	uint16	type1:8;
	uint16	typeV:4;
	
	uint16	type4_1:4;
	uint16	type3:8;
	uint16	type2_2:4;
	
	uint16	code1:8;
	uint16	codeV:4;
	uint16	type4_2:4;

	uint16	code3:8;
	uint16	code2:8;

	uint16	reserved:8;
	uint16	code4:8;
#endif
};

struct ipv4_icmp_st{

	ipaddr_t 	sipU;
	ipaddr_t 	sipL;
	ipaddr_t 	dipU;
	ipaddr_t 	dipL;
	uint16	tosD:8;
	uint16	tosM:8;
	uint16	typeV:4;
	uint16	type1:8;
	uint16	type2:8;
	uint16	type3:8;
	uint16	type4:8;
	uint16	codeV:4;
	uint16	code1:8;
	uint16	code2:8;
	uint16	code3:8;
	uint16	code4:8;
};



struct smi_ipv4_igmp_st{

#ifdef _LITTLE_ENDIAN
	uint16 	sipU0:8;
	uint16 	sipU1:8;
	uint16 	sipU2:8;
	uint16 	sipU3:8;

	uint16 	sipL0:8;
	uint16 	sipL1:8;
	uint16 	sipL2:8;
	uint16 	sipL3:8;

	uint16 	dipU0:8;
	uint16 	dipU1:8;
	uint16 	dipU2:8;
	uint16 	dipU3:8;

	uint16 	dipL0:8;
	uint16 	dipL1:8;
	uint16 	dipL2:8;
	uint16 	dipL3:8;

	uint16	tosD:8;
	uint16	tosM:8;
	
	uint16	typeV:4;
	uint16	type1:8;
	uint16	type2_1:4;
	
	uint16	type2_2:4;
	uint16	type3:8;
	uint16	type4_1:4;

	uint16	type4_2:4;
	uint16	reserved:12;
#else
	uint16 	sipU2:8;
	uint16 	sipU3:8;
	uint16 	sipU0:8;
	uint16 	sipU1:8;

	uint16 	sipL2:8;
	uint16 	sipL3:8;
	uint16 	sipL0:8;
	uint16 	sipL1:8;

	uint16 	dipU2:8;
	uint16 	dipU3:8;
	uint16 	dipU0:8;
	uint16 	dipU1:8;

	uint16 	dipL2:8;
	uint16 	dipL3:8;
	uint16 	dipL0:8;
	uint16 	dipL1:8;

	uint16	tosM:8;
	uint16	tosD:8;
	
	uint16	type2_1:4;
	uint16	type1:8;
	uint16	typeV:4;
	
	uint16	type4_1:4;
	uint16	type3:8;
	uint16	type2_2:4;

	uint16	reserved:12;
	uint16	type4_2:4;

#endif
};


struct ipv4_igmp_st{

	ipaddr_t 	sipU;
	ipaddr_t 	sipL;
	ipaddr_t 	dipU;
	ipaddr_t 	dipL;
	uint16	tosD:8;
	uint16	tosM:8;
	uint16	typeV:4;
	uint16	type1:8;
	uint16	type2:8;
	uint16	type3:8;
	uint16	type4:8;
};



struct smi_ipv4_tcp_st{

#ifdef _LITTLE_ENDIAN
	uint16 	sipU0:8;
	uint16 	sipU1:8;
	uint16 	sipU2:8;
	uint16 	sipU3:8;

	uint16 	sipL0:8;
	uint16 	sipL1:8;
	uint16 	sipL2:8;
	uint16 	sipL3:8;

	uint16 	dipU0:8;
	uint16 	dipU1:8;
	uint16 	dipU2:8;
	uint16 	dipU3:8;

	uint16 	dipL0:8;
	uint16 	dipL1:8;
	uint16 	dipL2:8;
	uint16 	dipL3:8;

	uint16	tosD:8;
	uint16	tosM:8;

	uint16	sPortU;
	uint16	sPortL;
	uint16	dPortU;
	uint16	dPortL;
	
	uint16	flagD:6;
	uint16	flagM:6;
	uint16	reserved:4;
#else
	uint16 	sipU2:8;
	uint16 	sipU3:8;
	uint16 	sipU0:8;
	uint16 	sipU1:8;

	uint16 	sipL2:8;
	uint16 	sipL3:8;
	uint16 	sipL0:8;
	uint16 	sipL1:8;

	uint16 	dipU2:8;
	uint16 	dipU3:8;
	uint16 	dipU0:8;
	uint16 	dipU1:8;

	uint16 	dipL2:8;
	uint16 	dipL3:8;
	uint16 	dipL0:8;
	uint16 	dipL1:8;

	uint16	tosM:8;
	uint16	tosD:8;

	uint16	sPortU;
	uint16	sPortL;
	uint16	dPortU;
	uint16	dPortL;
	
	uint16	reserved:4;
	uint16	flagM:6;
	uint16	flagD:6;
#endif
};

struct ipv4_tcp_st{

	ipaddr_t 	sipU;
	ipaddr_t 	sipL;
	ipaddr_t 	dipU;
	ipaddr_t 	dipL;
	uint16	tosD:8;
	uint16	tosM:8;
	uint16	sPortU;
	uint16	sPortL;
	uint16	dPortU;
	uint16	dPortL;
	uint16	flagD:6;
	uint16	flagM:6;
};



struct smi_ipv4_udp_st{

#ifdef _LITTLE_ENDIAN
	uint16 	sipU0:8;
	uint16 	sipU1:8;
	uint16 	sipU2:8;
	uint16 	sipU3:8;

	uint16 	sipL0:8;
	uint16 	sipL1:8;
	uint16 	sipL2:8;
	uint16 	sipL3:8;

	uint16 	dipU0:8;
	uint16 	dipU1:8;
	uint16 	dipU2:8;
	uint16 	dipU3:8;

	uint16 	dipL0:8;
	uint16 	dipL1:8;
	uint16 	dipL2:8;
	uint16 	dipL3:8;

	uint16	tosD:8;
	uint16	tosM:8;

	uint16	sPortU;
	uint16	sPortL;
	uint16	dPortU;
	uint16	dPortL;
#else
	uint16 	sipU2:8;
	uint16 	sipU3:8;
	uint16 	sipU0:8;
	uint16 	sipU1:8;

	uint16 	sipL2:8;
	uint16 	sipL3:8;
	uint16 	sipL0:8;
	uint16 	sipL1:8;

	uint16 	dipU2:8;
	uint16 	dipU3:8;
	uint16 	dipU0:8;
	uint16 	dipU1:8;

	uint16 	dipL2:8;
	uint16 	dipL3:8;
	uint16 	dipL0:8;
	uint16 	dipL1:8;

	uint16	tosM:8;
	uint16	tosD:8;

	uint16	sPortU;
	uint16	sPortL;
	uint16	dPortU;
	uint16	dPortL;

#endif
};

struct ipv4_udp_st{

	ipaddr_t 	sipU;
	ipaddr_t 	sipL;
	ipaddr_t 	dipU;
	ipaddr_t 	dipL;

	uint16	tosD:8;
	uint16	tosM:8;

	uint16	sPortU;
	uint16	sPortL;
	uint16	dPortU;
	uint16	dPortL;
};



struct smi_ipv6_sip_st{

	uint16 	sipU[8];
	uint16 	sipL[8];
};
struct ipv6_sip_st{

	uint16 	sipU[8];
	uint16 	sipL[8];
};

struct smi_ipv6_dip_st{

	uint16 	dipU[8];
	uint16 	dipL[8];
};
struct ipv6_dip_st{

	uint16 	dipU[8];
	uint16 	dipL[8];
};

struct smi_ipv6_ext_st{

#ifdef _LITTLE_ENDIAN
	uint16	tcD:8;
	uint16	tcM:8;
	
	uint16	nhV:4;
	uint16	nhp1:8;
	uint16	nhp2_1:4;
	
	uint16	nhp2_2:4;
	uint16	nhp3:8;
	uint16	nhp4_1:4;

	uint16	nhp4_2:4;
	uint16	reserved:12;
#else
	uint16	tcM:8;
	uint16	tcD:8;
	
	uint16	nhp2_1:4;
	uint16	nhp1:8;
	uint16	nhV:4;
	
	uint16	nhp4_1:4;
	uint16	nhp3:8;
	uint16	nhp2_2:4;

	uint16	reserved:12;
	uint16	nhp4_2:4;
	
#endif
	
};

struct ipv6_ext_st{

	uint16	tcD:8;
	uint16	tcM:8;
	
	uint16	nhV:4;
	uint16	nhp1:8;
	uint16	nhp2:8;
	uint16	nhp3:8;
	uint16	nhp4:8;
	uint16	reserved:12;
	
};



struct smi_ipv6_tcp_st{

#ifdef _LITTLE_ENDIAN
	uint16	tcD:8;
	uint16	tcM:8;

	uint16	sPortU;
	uint16	sPortL;
	uint16	dPortU;
	uint16	dPortL;
	
	uint16	flagD:6;
	uint16	flagM:6;
	uint16	reserved:4;

#else
	uint16	tcM:8;
	uint16	tcD:8;

	uint16	sPortU;
	uint16	sPortL;
	uint16	dPortU;
	uint16	dPortL;
	
	uint16	reserved:4;
	uint16	flagM:6;
	uint16	flagD:6;

#endif
};

struct ipv6_tcp_st{

	uint16	tcD:8;
	uint16	tcM:8;

	uint16	sPortU;
	uint16	sPortL;
	uint16	dPortU;
	uint16	dPortL;
	
	uint16	flagD:6;
	uint16	flagM:6;
};
struct smi_ipv6_udp_st{

#ifdef _LITTLE_ENDIAN
	uint16	tcD:8;
	uint16	tcM:8;

	uint16	sPortU;
	uint16	sPortL;
	uint16	dPortU;
	uint16	dPortL;
#else
	uint16	tcM:8;
	uint16	tcD:8;

	uint16	sPortU;
	uint16	sPortL;
	uint16	dPortU;
	uint16	dPortL;


#endif	
};
struct ipv6_udp_st{

	uint16	tcD:8;
	uint16	tcM:8;

	uint16	sPortU;
	uint16	sPortL;
	uint16	dPortU;
	uint16	dPortL;
};

struct smi_ipv6_icmp_st{

#ifdef _LITTLE_ENDIAN
	uint16	tcD:8;
	uint16	tcM:8;
	
	uint16	typeV:4;
	uint16	type1:8;
	uint16	type2_1:4;

	uint16	type2_2:4;
	uint16	type3:8;
	uint16	type4_1:4;

	uint16	type4_2:4;
	uint16	codeV:4;
	uint16	code1:8;

	uint16	code2:8;
	uint16	code3:8;
	
	uint16	code4:8;
	uint16	reserved:8;
#else
	uint16	tcM:8;
	uint16	tcD:8;
	
	uint16	type2_1:4;
	uint16	type1:8;
	uint16	typeV:4;

	uint16	type4_1:4;
	uint16	type3:8;
	uint16	type2_2:4;

	uint16	code1:8;
	uint16	codeV:4;
	uint16	type4_2:4;

	uint16	code3:8;
	uint16	code2:8;
	
	uint16	reserved:8;
	uint16	code4:8;
#endif
};


struct ipv6_icmp_st{

	uint16	tcD:8;
	uint16	tcM:8;
	
	uint16	typeV:4;
	uint16	type1:8;
	uint16	type2:8;
	uint16	type3:8;
	uint16	type4:8;
	uint16	codeV:4;
	uint16	code1:8;
	uint16	code2:8;
	uint16	code3:8;
	uint16	code4:8;
};



typedef union{

	struct smi_mac_st			smi_mac;	
	struct smi_ipv4_st			smi_ipv4;
	struct smi_ipv4_icmp_st	smi_ipv4icmp;
	struct smi_ipv4_igmp_st	smi_ipv4igmp;
	struct smi_ipv4_tcp_st		smi_ipv4tcp;
	struct smi_ipv4_udp_st		smi_ipv4udp;

	struct smi_ipv6_sip_st		smi_ipv6sip;
	struct smi_ipv6_dip_st		smi_ipv6dip;
	struct smi_ipv6_ext_st		smi_ipv6ext;
	struct smi_ipv6_tcp_st		smi_ipv6tcp;
	struct smi_ipv6_udp_st		smi_ipv6udp;
	struct smi_ipv6_icmp_st	smi_ipv6icmp;
	
}rtl8368s_smiaclrule;	




typedef struct  SMI_ACLTABLE{
	
#ifdef _LITTLE_ENDIAN

	rtl8368s_smiaclrule rule;

	uint16 ac_meteridx:4;
	uint16 ac_policing:1;
	uint16 ac_priority:3;
	uint16 ac_spri:1;
	uint16 ac_mirpmsk_1:7;
	
	uint16 ac_mirpmsk_2:1;
	uint16 ac_mir:1;
	uint16 ac_svidx:3;
	uint16 ac_svlan:1;
	uint16 op_term:1;
	uint16 op_exec:1;
	uint16 op_and:1;
	uint16 op_not:1;
	uint16 op_init:1;
	uint16 format:4;
	uint16 port_en_1:1;
	
	uint16 port_en_2:7;
	uint16 reserved:9;
#else
	rtl8368s_smiaclrule rule;

	uint16 ac_mirpmsk_1:7;
	uint16 ac_spri:1;
	uint16 ac_priority:3;
	uint16 ac_policing:1;
	uint16 ac_meteridx:4;

	uint16 port_en_1:1;
	uint16 format:4;
	uint16 op_init:1;
	uint16 op_not:1;
	uint16 op_and:1;
	uint16 op_exec:1;
	uint16 op_term:1;
	uint16 ac_svlan:1;
	uint16 ac_svidx:3;
	uint16 ac_mir:1;
	uint16 ac_mirpmsk_2:1;

	uint16 reserved:9;
	uint16 port_en_2:7;
#endif

}rtl8368s_smiacltable;



typedef union{

	struct mac_st				mac;	
	struct ipv4_st				ipv4;
	struct ipv4_icmp_st		ipv4icmp;
	struct ipv4_igmp_st		ipv4igmp;
	struct ipv4_tcp_st			ipv4tcp;
	struct ipv4_udp_st			ipv4udp;

	struct ipv6_sip_st			ipv6sip;
	struct ipv6_dip_st			ipv6dip;
	struct ipv6_ext_st			ipv6ext;
	struct ipv6_tcp_st			ipv6tcp;
	struct ipv6_udp_st			ipv6udp;
	struct ipv6_icmp_st		ipv6icmp;

}rtl8368s_aclrule;	


typedef struct  ACLTABLE{
	
	rtl8368s_aclrule rule;

	uint16 ac_meteridx:4;
	uint16 ac_policing:1;
	uint16 ac_priority:3;
	uint16 ac_spri:1;
	uint16 ac_mirpmsk:8;
	uint16 ac_mir:1;
	uint16 ac_svidx:3;
	uint16 ac_svlan:1;
	uint16 op_term:1;
	uint16 op_exec:1;
	uint16 op_and:1;
	uint16 op_not:1;
	uint16 op_init:1;
	uint16 format:4;
	uint16 port_en:8;
	uint16 reserved:9;

}rtl8368s_acltable;


typedef struct  PROTOCOLGDATACONFIG{

#ifdef _LITTLE_ENDIAN
	uint16 frameType:2;
	uint16 valid:1;
	uint16 reserved:13;
#else
	uint16 reserved:13;
	uint16 valid:1;
	uint16 frameType:2;
#endif
	uint16 value;

}rtl8368s_protocolgdatacfg;


typedef struct  PROTOCLVLANCONFIG{

#ifdef _LITTLE_ENDIAN
	uint16 vidx:4;
	uint16 prio:3;
	uint16 valid:1;
	uint16 reserved:8;
#else
	uint16 reserved:8;
	uint16 valid:1;
	uint16 prio:3;
	uint16 vidx:4;
#endif
	
}rtl8368s_protocolvlancfg;

typedef struct  rtl8368s_portability_s{
#ifdef _LITTLE_ENDIAN
	uint8 AN:1;
	uint8 H10:1;
	uint8 F10:1;
	uint8 H100:1;
	uint8 F100:1;
	uint8 F1000:1;
	uint8 FC:1;
	uint8 AsyFC:1;
#else
	uint8 AsyFC:1;
	uint8 FC:1;
	uint8 F1000:1;
	uint8 F100:1;
	uint8 H100:1;
	uint8 F10:1;
	uint8 H10:1;
	uint8 AN:1;
#endif
}rtl8368s_portability_t;


enum RTL8368S_MIBCOUNTER{

	IfInOctets = 0,
	EtherStatsOctets,
	EtherStatsUnderSizePkts,
	EtherStatsFragments,
	EtherStatsPkts64Octets,
	EtherStatsPkts65to127Octets,
	EtherStatsPkts128to255Octets,
	EtherStatsPkts256to511Octets,
	EtherStatsPkts512to1023Octets,
	EtherStatsPkts1024to1518Octets,
	EtherOversizeStats,
	EtherStatsJabbers,
	IfInUcastPkts,
	EtherStatsMulticastPkts,
	EtherStatsBroadcastPkts,
	EtherStatsDropEvents,
	Dot3StatsFCSErrors,
	Dot3StatsSymbolErrors,
	Dot3InPauseFrames,
	Dot3ControlInUnknownOpcodes,
	IfOutOctets,
	Dot3StatsSingleCollisionFrames,
	Dot3StatMultipleCollisionFrames,
	Dot3sDeferredTransmissions,
	Dot3StatsLateCollisions,
	EtherStatsCollisions,
	Dot3StatsExcessiveCollisions,
	Dot3OutPauseFrames,
	Dot1dBasePortDelayExceededDiscards,
	Dot1dTpPortInDiscards,
	IfOutUcastPkts,
	IfOutMulticastPkts,
	IfOutBroadcastPkts,
	OutOampduPkts,
	InOampduPkts,
	PktgenPkts,
	/*Device only */	
	Dot1dTpLearnEntryDiscardFlag,
	RTL8368S_MIBS_NUMBER,
	
};	

enum RTL8368S_LEDCONF{

	LEDCONF_LEDOFF=0, 		
	LEDCONF_DUPCOL,		
	LEDCONF_LINK_ACT,		
	LEDCONF_SPD1000,		
	LEDCONF_SPD100,		
	LEDCONF_SPD10,			
	LEDCONF_SPD1000ACT,	
	LEDCONF_SPD100ACT,	
	LEDCONF_SPD10ACT,		
	LEDCONF_SPD10010ACT,  
	LEDCONF_FIBER,			
	LEDCONF_FAULT,			
	LEDCONF_LINKRX,		
	LEDCONF_LINKTX,		
	LEDCONF_MASTER,		
	LEDCONF_LEDFORCE,		
};


enum RTL8368S_LEDBLINKRATE{

	LEDBLINKRATE_28MS=0, 		
	LEDBLINKRATE_56MS,		
	LEDBLINKRATE_84MS,
	LEDBLINKRATE_111MS,
	LEDBLINKRATE_222MS,
	LEDBLINKRATE_446MS,
	LEDBLINKRATE_MAX,

};




int32 rtl8368s_setAsicRegBit(uint32 reg, uint32 bit, uint32 value);
int32 rtl8368s_setAsicRegBits(uint32 reg, uint32 bits, uint32 value);

int32 rtl8368s_getAsicReg(uint32 reg, uint32 *val);
int32 rtl8368s_setAsicReg(uint32 reg, uint32 value);

int32 rtl8368s_setAsicAcl(enum PORTID port, uint32 enabled);
int32 rtl8368s_getAsicAcl(enum PORTID port, uint32* enabled);
int32 rtl8368s_setAsicAclUnmatchedPermit(enum PORTID port, uint32 enabled);
int32 rtl8368s_getAsicAclUnmatchedPermit(enum PORTID port, uint32* enabled);
int32 rtl8368s_setAsicAclMeterInterFrameGap(uint32 enabled);
int32 rtl8368s_getAsicAclMeterInterFrameGap(uint32* enabled);

int32 rtl8368s_setAsicAclMeter(uint32 meterIdx,uint32 aclmRate, uint32 aclmTole);
int32 rtl8368s_getAsicAclMeter(uint32 meterIdx,uint32* aclmRate, uint32* aclmTole);


int32 rtl8368s_setAsicAclStartEnd(enum PORTID port, uint32 aclStart, uint32 aclEnd);
int32 rtl8368s_getAsicAclStartEnd(enum PORTID port, uint32 *aclStart, uint32 *aclEnd);
int32 rtl8368s_setAsicAclRule( uint32 index, rtl8368s_acltable aclTable);
int32 rtl8368s_getAsicAclRule( uint32 index, rtl8368s_acltable *aclTable);
int32 rtl8368s_setAsicSmiAclTable( uint32 index, rtl8368s_smiacltable *smiaclTable);

int32 rtl8368s_setAsicRma(enum RMATRAPFRAME rma, enum RMAOP op,uint32 keepCtag, uint32 bypassStorm,uint32 priSel,enum PRIORITYVALUE priority);
int32 rtl8368s_getAsicRma(enum RMATRAPFRAME rma, enum RMAOP* op,uint32* keepCtag, uint32* bypassStorm,uint32* priSel,enum PRIORITYVALUE* priority);
int32 rtl8368s_setAsicRmaUserDefinedAddress(uint32 index, ether_addr_t mac, uint32 mask);
int32 rtl8368s_getAsicRmaUserDefinedAddress(uint32 index, ether_addr_t *mac, uint32 *mask);


int32 rtl8368s_setAsicVlan4kTbUsage(uint32 enabled);
int32 rtl8368s_getAsicVlan4kTbUsage(uint32* enabled);

int32 rtl8368s_setAsicVlanMemberConfig(uint32 index,rtl8368s_vlanconfig vlanmconf );
int32 rtl8368s_getAsicVlanMemberConfig(uint32 index,rtl8368s_vlanconfig *vlanmconf );
int32 rtl8368s_setAsicVlan(uint32 enabled);
int32 rtl8368s_getAsicVlan(uint32* enabled);
int32 rtl8368s_setAsicVlan4kEntry(rtl8368s_user_vlan4kentry vlan4kEntry);
int32 rtl8368s_getAsicVlan4kEntry(rtl8368s_user_vlan4kentry *vlan4kEntry);
int32 rtl8368s_setAsicSpanningTreeStatus(enum PORTID port, uint32 fid, enum SPTSTATE state);
int32 rtl8368s_getAsicSpanningTreeStatus(enum PORTID port, uint32 fid, enum SPTSTATE* state);
int32 rtl8368s_setAsicVlanAcceptTaggedOnly(enum PORTID port, uint32 enabled);
int32 rtl8368s_getAsicVlanAcceptTaggedOnly(enum PORTID port, uint32* enabled);
int32 rtl8368s_setAsicVlanDropTaggedPackets(enum PORTID port, uint32 enabled);
int32 rtl8368s_getAsicVlanDropTaggedPackets(enum PORTID port, uint32* enabled);
int32 rtl8368s_setAsicVlanIngressFiltering(enum PORTID port, uint32 enabled);
int32 rtl8368s_getAsicVlanIngressFiltering(enum PORTID port, uint32* enabled);
int32 rtl8368s_setAsicVlanIpMulticastLeaky( enum PORTID port, uint32 enabled );
int32 rtl8368s_getAsicVlanIpMulticastLeaky( enum PORTID port, uint32* enabled );
int32 rtl8368s_setAsicVlanPortBasedVID(enum PORTID port, uint32 index);
int32 rtl8368s_getAsicVlanPortBasedVID(enum PORTID port, uint32* index);
int32 rtl8368s_setAsicVlanProtocolBasedGroupData(uint32 index, rtl8368s_protocolgdatacfg pbcfg);
int32 rtl8368s_getAsicVlanProtocolBasedGroupData(uint32 index, rtl8368s_protocolgdatacfg* pbcfg);
int32 rtl8368s_setAsicVlanProtocolAndPortBasedCfg(enum PORTID port, uint32 index, rtl8368s_protocolvlancfg ppbcfg);
int32 rtl8368s_getAsicVlanProtocolAndPortBasedCfg(enum PORTID port, uint32 index, rtl8368s_protocolvlancfg* ppbcfg);


int32 rtl8368s_setAsicL2LookupTb(uint32 entry, rtl8368s_l2table *l2Table);
int32 rtl8368s_getAsicL2LookupTb(uint32 entry, rtl8368s_l2table *l2Table);
int32 rtl8368s_setAsicL2CamTb(uint32 entry, rtl8368s_camtable *camTable);
int32 rtl8368s_getAsicL2CamTb(uint32 entry, rtl8368s_camtable *camTable);
int32 rtl8368s_setAsicL2IpMulticastLookup(uint32 enabled);
int32 rtl8368s_getAsicL2IpMulticastLookup(uint32* enabled);
int32 rtl8368s_setAsicL2CamTbUsage(uint32 disabled);
int32 rtl8368s_getAsicL2CamTbUsage(uint32 *disabled);


int32 rtl8368s_setAsicSvlanProtocolType(uint32 protocolType);
int32 rtl8368s_getAsicSvlanProtocolType(uint32* protocolType);
int32 rtl8368s_setAsicSvlanUplinkPortMask(uint32 portMask);
int32 rtl8368s_getAsicSvlanUplinkPortMask(uint32* portMask);
int32 rtl8368s_setAsicSvlanTag(uint32 enabled);
int32 rtl8368s_getAsicSvlanTag(uint32* enabled);
int32 rtl8368s_setAsicSvlan(uint32 enabled);
int32 rtl8368s_getAsicSvlan(uint32* enabled);
int32 rtl8368s_setAsicSvlanS2Cmbrsel(uint32 enabled);
int32 rtl8368s_getAsicSvlanS2Cmbrsel(uint32* enabled);
int32 rtl8368s_setAsicSvlanPrioDecision(uint32 vsPrio);
int32 rtl8368s_getAsicSvlanPrioDecision(uint32* vsPrio);
int32 rtl8368s_setAsicSvlanPortVsidx(enum PORTID port, uint32 vsidx);
int32 rtl8368s_getAsicSvlanPortVsidx(enum PORTID port, uint32* vsidx);
int32 rtl8368s_setAsicSvlanIngressUntag(uint32 enabled);
int32 rtl8368s_getAsicSvlanIngressUntag(uint32* enabled);
int32 rtl8368s_setAsicSvlanIngressUnmatch(uint32 enabled);
int32 rtl8368s_getAsicSvlanIngressUnmatch(uint32* enabled);
int32 rtl8368s_setAsicSvlanCpuPriority(enum PRIORITYVALUE priority);
int32 rtl8368s_getAsicSvlanCpuPriority(enum PRIORITYVALUE* priority);
int32 rtl8368s_setAsicSvlanConfiguration(uint32 index,uint32 svid,enum PRIORITYVALUE spri,uint32 rpvid,uint32 cvidx);
int32 rtl8368s_getAsicSvlanConfiguration(uint32 index,uint32* svid,enum PRIORITYVALUE* spri,uint32* rpvid,uint32* cvidx);




int32 rtl8368s_setAsicCpuPortMask(uint32 portMask);
int32 rtl8368s_getAsicCpuPortMask(uint32* portMask);
int32 rtl8368s_setAsicCpuDisableInsTag(uint32 enabled);
int32 rtl8368s_getAsicCpuDisableInsTag(uint32* enabled);
int32 rtl8368s_setAsicCpuPriorityRemapping(enum PRIORITYVALUE priority, enum PRIORITYVALUE newpriority);
int32 rtl8368s_getAsicCpuPriorityRemapping(enum PRIORITYVALUE priority, enum PRIORITYVALUE *pNewpriority);


int32 rtl8368s_setAsicLinkAggregationMode(uint32 mode);
int32 rtl8368s_getAsicLinkAggregationMode(uint32 *mode);
int32 rtl8368s_setAsicLinkAggregationHashAlgorithm(uint32 algorithm);
int32 rtl8368s_getAsicLinkAggregationHashAlgorithm(uint32 *algorithm);
int32 rtl8368s_setAsicLinkAggregationPortMask(uint32 portmask);
int32 rtl8368s_getAsicLinkAggregationPortMask(uint32* portmask);
int32 rtl8368s_setAsicLinkAggregationHashTable(uint32 hashval,enum PORTID port);
int32 rtl8368s_getAsicLinkAggregationHashTable(uint32 hashval,enum PORTID* port);
int32 rtl8368s_setAsicLinkAggregationFlowControl(uint32 fcport);
int32 rtl8368s_getAsicLinkAggregationFlowControl(uint32* fcport);
int32 rtl8368s_getAsicLinkAggregationQueueEmpty(uint32* qeport);

int32 rtl8368s_setAsicStormFiltering(enum PORTID port, uint32 bcstorm, uint32 mcstorm, uint32 undastorm,uint32 unmcstorm);
int32 rtl8368s_getAsicStormFiltering(enum PORTID port, uint32* bcstorm, uint32* mcstorm, uint32* undastorm,uint32* unmcstorm);
int32 rtl8368s_setAsicStormFilteringMeter(enum PORTID port,uint32 rate, uint32 ifg );
int32 rtl8368s_getAsicStormFilteringMeter(enum PORTID port,uint32* rate, uint32* ifg );
int32 rtl8368s_getAsicStormFilteringExceedStatus(uint32* portmask);

int32 rtl8368s_setAsicPortMirror(enum PORTID mirrored, enum PORTID monitor);
int32 rtl8368s_getAsicPortMirror(uint32 *mirrored, uint32 *monitor);
int32 rtl8368s_setAsicPortMirrorRxFunction(uint32 enabled);
int32 rtl8368s_getAsicPortMirrorRxFunction(uint32* enabled);
int32 rtl8368s_setAsicPortMirrorTxFunction(uint32 enabled);
int32 rtl8368s_getAsicPortMirrorTxFunction(uint32* enabled);
int32 rtl8368s_setAsicPortMirrorIsolation(uint32 enabled);
int32 rtl8368s_getAsicPortMirrorIsolation(uint32* enabled);

int32 rtl8368s_setAsicPortDisable(enum PORTID port,uint32 disable);
int32 rtl8368s_getAsicPortDisable(enum PORTID port,uint32* disable);
int32 rtl8368s_setAsicPortAbility(enum PORTID port,rtl8368s_portability_t ability);
int32 rtl8368s_getAsicPortAbility(enum PORTID port,rtl8368s_portability_t* ability);
int32 rtl8368s_setAsicPortJamMode(uint32 mode);
int32 rtl8368s_getAsicPortJamMode(uint32* mode);
int32 rtl8368s_setAsicMaxLengthInRx(uint32 maxLength);
int32 rtl8368s_getAsicMaxLengthInRx(uint32* maxLength);
int32 rtl8368s_setAsicPortLearnDisable(enum PORTID port,uint32 disabled);
int32 rtl8368s_getAsicPortLearnDisable(enum PORTID port,uint32* disabled);
int32 rtl8368s_setAsicPortAge(enum PORTID port,uint32 disabled);
int32 rtl8368s_getAsicPortAge(enum PORTID port,uint32* disabled);
int32 rtl8368s_setAsicFastAge(uint32 enable);
int32 rtl8368s_getAsicFastAge(uint32* enable);
int32 rtl8368s_setAsicDropUnknownDa(uint32 enabled);
int32 rtl8368s_getAsicDropUnknownDa(uint32* enabled);
int32 rtl8368s_setAsicDropUnknownSa(uint32 enabled);
int32 rtl8368s_getAsicDropUnknownSa(uint32 *enabled);
int32 rtl8368s_setAsicDropUnmatchedSa(uint32 enabled);
int32 rtl8368s_getAsicDropUnmatchedSa(uint32* enabled);
int32 rtl8368s_setAsicBlockSpa(enum PORTID port,uint32 disabled);
int32 rtl8368s_getAsicBlockSpa(enum PORTID port,uint32* disabled);


int32 rtl8368s_setAsicInterruptPolarity(uint32 polarity);
int32 rtl8368s_getAsicInterruptPolarity(uint32* polarity);
int32 rtl8368s_setAsicInterruptMask(uint32 mask);
int32 rtl8368s_getAsicInterruptMask(uint32* mask);
int32 rtl8368s_getAsicInterruptStatus(uint32* mask);

int32 rtl8368s_setAsicMIBsCounterReset(uint32 mask);
int32 rtl8368s_getAsicMIBsCounter(enum PORTID port,enum RTL8368S_MIBCOUNTER mibIdx,uint64* counter);
int32 rtl8368s_getAsicMIBsControl(uint32* mask);

int32 rtl8368s_setAsicPHYRegs( uint32 phyNo, uint32 page, uint32 addr, uint32 data);
int32 rtl8368s_getAsicPHYRegs( uint32 phyNo, uint32 page, uint32 addr, uint32 *data);
int32 rtl8368s_setAsicExtPHYReg(uint32 phy, uint32 reg, uint32 data );
int32 rtl8368s_getAsicExtPHYReg(uint32 phy, uint32 reg, uint32 *data );
int32 rtl8368s_getAsicExtPHYReg_withPage(uint32 phy, uint32 page, uint32 reg, uint32 *data );


int32 rtl8368s_setAsicQosEnable( uint32 enabled);
int32 rtl8368s_getAsicQosEnable( uint32* enabled);

int32 rtl8368s_setAsicLBParameter( uint32 token, uint32 tick, uint32 hiThreshold );
int32 rtl8368s_getAsicLBParameter( uint32* pToken, uint32* pTick, uint32* pHiThreshold );
int32 rtl8368s_setAsicQueueRate( enum PORTID port, enum QUEUEID queueid, uint32 pprTime, uint32 aprBurstSize, uint32 apr );
int32 rtl8368s_getAsicQueueRate( enum PORTID port, enum QUEUEID queueid, uint32* pPprTime, uint32* pAprBurstSize, uint32* pApr );

int32 rtl8368s_setAsicDisableSchedulerAbility( enum PORTID port, uint32 aprDisable, uint32 pprDisable, uint32 wfqDisable );
int32 rtl8368s_getAsicDisableSchedulerAbility( enum PORTID port, uint32* pAprDisable, uint32* pPprDisable, uint32* pWfqDisable);
int32 rtl8368s_setAsicPortIngressBandwidth( enum PORTID port, uint32 bandwidth, uint32 preifg);
int32 rtl8368s_getAsicPortIngressBandwidth( enum PORTID port, uint32* pBandwidth, uint32* pPreifg );
int32 rtl8368s_setAsicPortEgressBandwidth( enum PORTID port, uint32 bandwidth, uint32 preifg );
int32 rtl8368s_getAsicPortEgressBandwidth( enum PORTID port, uint32* pBandwidth, uint32* preifg );
int32 rtl8368s_setAsicQueueWeight( enum PORTID port, enum QUEUEID queueid, enum QUEUETYPE queueType, uint32 weight );
int32 rtl8368s_getAsicQueueWeight( enum PORTID port, enum QUEUEID queueid, enum QUEUETYPE *pQueueType, uint32 *pWeight );
int32 rtl8368s_setAsicOutputQueueNumber( enum PORTID port, enum QUEUENUM qnum );
int32 rtl8368s_getAsicOutputQueueNumber( enum PORTID port, enum QUEUENUM *qnum );

int32 rtl8368s_setAsicDot1pRemarkingAbility(uint32 enabled);
int32 rtl8368s_getAsicDot1pRemarkingAbility(uint32* enabled);
int32 rtl8368s_setAsicDot1pRemarkingParameter( enum PRIORITYVALUE priority, enum PRIORITYVALUE newpriority );
int32 rtl8368s_getAsicDot1pRemarkingParameter( enum PRIORITYVALUE priority, enum PRIORITYVALUE *pNewpriority );
int32 rtl8368s_setAsicDscpRemarkingAbility(uint32 enabled);
int32 rtl8368s_getAsicDscpRemarkingAbility( uint32* enabled);
int32 rtl8368s_setAsicDscpRemarkingParameter( enum PRIORITYVALUE priority, uint32 newdscp );
int32 rtl8368s_getAsicDscpRemarkingParameter( enum PRIORITYVALUE priority, uint32* pNewdscp );

int32 rtl8368s_setAsicPriorityDecision( uint32 portpri, uint32 dot1qpri, uint32 dscppri, uint32 aclpri);
int32 rtl8368s_getAsicPriorityDecision( uint32* pPortpri, uint32* pDot1qpri, uint32* pDscppri, uint32* pAclpri);
int32 rtl8368s_setAsicPortPriority( enum PORTID port, enum PRIORITYVALUE priority );
int32 rtl8368s_getAsicPortPriority( enum PORTID port, enum PRIORITYVALUE *pPriority );
int32 rtl8368s_setAsicDot1qAbsolutelyPriority( enum PRIORITYVALUE srcpriority, enum PRIORITYVALUE priority );
int32 rtl8368s_getAsicDot1qAbsolutelyPriority( enum PRIORITYVALUE srcpriority, enum PRIORITYVALUE *pPriority );
int32 rtl8368s_setAsicDscpPriority( uint32 dscp, enum PRIORITYVALUE priority );
int32 rtl8368s_getAsicDscpPriority( uint32 dscp, enum PRIORITYVALUE *pPriority );
int32 rtl8368s_setAsicPriorityToQIDMappingTable( enum QUEUENUM qnum, enum PRIORITYVALUE priority, enum QUEUEID qid );
int32 rtl8368s_getAsicPriorityToQIDMappingTable( enum QUEUENUM qnum, enum PRIORITYVALUE priority, enum QUEUEID* pQid );

int32 rtl8368s_setAsicFcSystemDrop(uint32 drop);
int32 rtl8368s_getAsicFcSystemDrop(uint32* drop);
int32 rtl8368s_setAsicFcSystemSharedBufferBased(uint32 sharedON, uint32 sharedOFF);
int32 rtl8368s_getAsicFcSystemSharedBufferBased(uint32 *sharedON, uint32 *sharedOFF);
int32 rtl8368s_setAsicFcPortBased(enum PORTID port, uint32 fcON, uint32 fcOFF);
int32 rtl8368s_getAsicFcPortBased(enum PORTID port, uint32 *fcON, uint32 *fcOFF);
int32 rtl8368s_setAsicFcQueueDescriptorBased(enum PORTID port, enum QUEUEID queue, uint32 fcON, uint32 fcOFF);
int32 rtl8368s_getAsicFcQueueDescriptorBased(enum PORTID port, enum QUEUEID queue, uint32 *fcON, uint32 *fcOFF);
int32 rtl8368s_setAsicFcQueuePacketBased(enum PORTID port, enum QUEUEID queue, uint32 fcON, uint32 fcOFF);
int32 rtl8368s_getAsicFcQueuePacketBased(enum PORTID port, enum QUEUEID queue, uint32 *fcON, uint32 *fcOFF);
int32 rtl8368s_setAsicFcPerQueuePhysicalLengthGap(uint32 gap);
int32 rtl8368s_getAsicFcPerQueuePhysicalLengthGap(uint32 *gap);
int32 rtl8368s_setAsicFcQueueFlowControlUsage(enum PORTID port, enum QUEUEID queue, uint32 enabled);
int32 rtl8368s_getAsicFcQueueFlowControlUsage(enum PORTID port, enum QUEUEID queue, uint32 *enabled);
int32 rtl8368s_setAsicFcPacketUsedPagesBased(uint32 fcON, uint32 enabled);
int32 rtl8368s_getAsicFcPacketUsedPagesBased(uint32 *fcON, uint32 *enabled);
int32 rtl8368s_setAsicFcSystemBased(uint32 systemON, uint32 systemOFF);
int32 rtl8368s_getAsicFcSystemBased(uint32 *systemON, uint32 *systemOFF);


int32 rtl8368s_setAsic1xPBEnConfig(enum PORTID port,uint32 enabled);
int32 rtl8368s_getAsic1xPBEnConfig(enum PORTID port,uint32 *enabled);
int32 rtl8368s_setAsic1xPBAuthConfig(enum PORTID port,uint32 auth);
int32 rtl8368s_getAsic1xPBAuthConfig(enum PORTID port,uint32* auth);
int32 rtl8368s_setAsic1xPBOpdirConfig(enum PORTID port,uint32 opdir);
int32 rtl8368s_getAsic1xPBOpdirConfig(enum PORTID port,uint32* opdir);
int32 rtl8368s_setAsic1xMBEnConfig(enum PORTID port,uint32 enabled);
int32 rtl8368s_getAsic1xMBEnConfig(enum PORTID port,uint32* enabled);
int32 rtl8368s_setAsic1xMBOpdirConfig(uint32 opdir);
int32 rtl8368s_getAsic1xMBOpdirConfig(uint32 *opdir);
int32 rtl8368s_setAsic1xProcConfig(enum UNAUTH1XPROC proc);
int32 rtl8368s_getAsic1xProcConfig(enum UNAUTH1XPROC* proc);
int32 rtl8368s_setAsic1xGuestVidx(uint32 index);
int32 rtl8368s_getAsic1xGuestVidx(uint32 *index);
int32 rtl8368s_setAsic1xGuestTalkToAuth(uint32 enabled);
int32 rtl8368s_getAsic1xGuestTalkToAuth(uint32 *enabled);

int32 rtl8368s_setAsicLedIndicateInfoConfig(uint32 ledNo, enum RTL8368S_LEDCONF config);
int32 rtl8368s_getAsicLedIndicateInfoConfig(uint32 ledNo, enum RTL8368S_LEDCONF* config);
int32 rtl8368s_setAsicForceLeds(uint32 ledG0Msk, uint32 ledG1Msk, uint32 ledG2Msk, uint32 ledG3Msk);
int32 rtl8368s_setAsicLedBlinkRate(enum RTL8368S_LEDBLINKRATE blinkRate);
int32 rtl8368s_getAsicLedBlinkRate(enum RTL8368S_LEDBLINKRATE* blinkRate);

int32 rtl8368s_setAiscPort4RGMIIControl(uint32 enabled);

int32 rtl8368s_setAsicMacForceLink(enum PORTID port,enum MACLINKMODE force,enum PORTLINKSPEED speed,enum PORTLINKDUPLEXMODE duplex,uint32 link,uint32 txPause,uint32 rxPause);
int32 rtl8368s_getAsicPortLinkState(enum PORTID port, enum PORTLINKSPEED *speed, enum PORTLINKDUPLEXMODE *duplex,uint32 *link,uint32 *txPause,uint32 *rxPause, uint32 *nWay);


int32 rtl8368s_setVlanAsicIpMulticastLeaky(enum PORTID port, uint32 enabled );
int32 rtl8368s_getVlanAsicIpMulticastLeaky(enum PORTID port, uint32* enabled );

int32 rtl8368s_setAsicCableTesting( enum PORTID port, uint32 enabled);
int32 rtl8368s_getAsicRtctChannelStatus(enum PORTID port, enum CHANNELID channel,uint32* impmis,uint32* open_state,uint32* short_state,uint32* length);

int32 rtl8368s_setAsicAgeTimerSpeed( uint32 timer, uint32 speed);
int32 rtl8368s_getAsicAgeTimerSpeed( uint32* timer, uint32* speed);

int32 rtl8368s_setAsicVlanKeepCtagFormat(enum PORTID ingressport, uint32 portmask);
int32 rtl8368s_getAsicVlanKeepCtagFormat(enum PORTID ingressport, uint32* portmask);

int32 rtl8368s_setAsicPortIsolation(enum PORTID port, uint32 enable);
int32 rtl8368s_getAsicPortIsolation( enum PORTID port, uint32* enabled);

int32 rtl8368s_setAsicPortIsolationConfig(enum PORTID port, uint32 portmask);
int32 rtl8368s_getAsicPortIsolationConfig(enum PORTID port, uint32* portmask);

int32 rtl8368s_setAsicSpecialCongestModeConfig(enum PORTID port, uint32 sustain);
int32 rtl8368s_getAsicSpecialCongestModeConfig(enum PORTID port, uint32* sustain);
int32 rtl8368s_getAsicSpecialCongestModeTimer(enum PORTID port, uint32* timer);

int32 rtl8368s_setAsicOamParser(enum PORTID port, enum OAMPARACT parser);
int32 rtl8368s_getAsicOamParser(enum PORTID port, enum OAMPARACT* parser);
int32 rtl8368s_setAsicOamMultiplexer(enum PORTID port, enum OAMMULACT multiplexer);
int32 rtl8368s_getAsicOamMultiplexer(enum PORTID port, enum OAMMULACT* multiplexer);

int32 rtl8368s_setAsicMacAddress(ether_addr_t macAsic);
int32 rtl8368s_getAsicMacAddress(ether_addr_t* macAsic);

int32 rtl8368s_setAsicGreenFeature(uint32 enable);
int32 rtl8368s_getAsicGreenFeature(uint32* enable);
int32 rtl8368s_setAsicPowerSaving(uint32 phyNo,uint32 enabled);
int32 rtl8368s_getAsicPowerSaving(uint32 phyNo,uint32* enabled);	


#endif /*#ifndef _RTL8368S_ASICDRV_H_*/

