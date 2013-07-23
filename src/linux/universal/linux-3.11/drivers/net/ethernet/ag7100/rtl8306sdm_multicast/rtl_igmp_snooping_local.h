/*
* Copyright c                  Realsil Semiconductor Corporation, 2006
* All rights reserved.
* 
* Program :  IGMP snooping function
* Abstract : 
* Author :qinjunjie 
* Email:qinjunjie1980@hotmail.com
*
*/

#include "rtl_igmp_glue.h"
#ifndef CYGWIN
#ifdef __linux__
#include <linux/config.h>
#endif /* __linux__ */
#endif /* CYGWIN */

 /*                                               Macro declaration                                         */


/***********************************Utilities************************************************/
#define IS_CLASSD_ADDR(ipv4addr)				((((uint32)(ipv4addr)) & 0xf0000000) == 0xe0000000)	
/***********************************************************************/



#define PORT0_MASK 0x01
#define PORT1_MASK 0x02
#define PORT2_MASK 0x04
#define PORT3_MASK 0x08
#define PORT4_MASK 0x10
#define PORT5_MASK 0x20
#define NON_PORT_MASK 0x00
#define MULTICAST_MAC_ADDR_MASK 0x007fffff

/* IGMP snooping default  parameters */
#define RTL_DefaultMaxGroupEntryCnt					      100	         /* default max group entry count **/
#define DEFAULT_GROUP_MEMBER_INTERVAL 260             /* IGMP group member interval, default is 260 seconds */
#define DEFAULT_LAST_MEMBER_QUERY_TIME  2             /* IGMP last member query time, default is 2 seconds */
#define DEFAULT_QUERIER_PRESENT_TIMEOUT 260             /* IGMP  querier present timeout, default is 260 seconds */
#define DEFAULT_DVMRP_AGING_TIME		120           
#define DEFAULT_PIM_AGING_TIME		120           
#define DEFAULT_MOSPF_AGING_TIME		120


#define IN_LOOKUP_TABLE       0x01
#define AGGREGATOR_FLAG     0x80

#define CPUTAGPROTOCOL 0x09
#define VLAN_PROTOCOL_ID 0x8100
#define IPV4_ETHER_TYPE 0x0800

#define PPPOE_ETHER_TYPE 0x8864
#define PPP_IPV4_PROTOCOL 0x0021
#define ROUTER_ALERT_OPTION 0x94040000

#define DVMRP_ADDR  0xE0000004
#define DVMRP_TYPE 0x13
#define DVMRP_PROTOCOL 3

#define PIM_ADDR 0xE000000D
#define PIM_PROTOCOL 103
#define PIM_HELLO_TYPE 0x20

#define MOSPF_ADDR1  0xE0000005
#define MOSPF_ADDR2  0xE0000006
#define MOSPF_PROTOCOL 89
#define MOSPF_HELLO_TYPE 1

#define IGMP_PROTOCOL 2
#define IGMP_ALL_HOSTS_ADDR	0xE0000001	/*general query address*/
#define IGMP_ALL_ROUTERS_ADDR	0xE0000002  /*leave report address*/
#define IGMPV3_REPORT_ADDR 0xE0000016

		/*            IGMP type              */
#define IGMP_QUERY_TYPE 0x11
#define IGMPV1_REPORT_TYPE 0x12
#define IGMPV2_REPORT_TYPE 0x16
#define IGMPV2_LEAVE_TYPE 0x17
#define IGMPV3_REPORT_TYPE 0x22

#define MODE_IS_INCLUDE			                      1
#define MODE_IS_EXCLUDE			                      2
#define CHANGE_TO_INCLUDE_MODE	                      3
#define CHANGE_TO_EXCLUDE_MODE		               4
#define ALLOW_NEW_SOURCES                                    5
#define BLOCK_OLD_SOURCES			                      6


/* CPU tag type */
#define REALTEKETHERTYPE      0x8899
 

/*********************************************
			Data Structure
 *********************************************/
struct ipv4Pkt {
	uint8       vhl;            
	uint8	typeOfService;			
	uint16	length;			/* total length */
	uint16	identification;	/* identification */
	uint16	offset;			
	uint8	ttl;				/* time to live */
	uint8	protocol;			
	uint16	checksum;			
	uint32     sourceIp;
	uint32 	destinationIp;
	uint32 	option;	
};

struct mospfHdr{
	uint8 version;
	uint8 type;
	uint16 pktLen;
	uint32 routerId;
	uint32 areaId;
	uint16 checksum;
	uint16 auType;
	uint32 authentication[2];	
};

struct mospfHello{
	struct mospfHdr hdr;
	uint32 netWorkMask;
	uint16 helloInterVal;
	uint8  options;          /*  X-X-DC-EA-N/P-MC-E-X */
	uint8 routerPriority;
	uint32 routerDeadInterval;
	uint32 designatedRouter;
	uint32 backupDesignatedRouter;
	uint32 neighbor;
};

struct pimHdr{
	uint8 verType;
	uint8 reserved;
	uint16 checksum;

};



struct igmpPkt{
	uint8 type;				      /* type*/
	uint8 maxRespTime;                /*maximum response time,unit:0.1 seconds*/
	uint16 checksum;                   
	uint32 groupAddr;
	
};

struct igmpv3Query
{
	uint8 type;                                /*query type*/
	uint8 maxRespCode;			/*maximum response code*/
	uint16 checkSum;				/*IGMP checksum*/
	uint32 groupAddr;                    /*multicast group address*/
	uint8 rsq;					/* 4bit: reserved, 1bit: suppress router-side processing, 3bit: querier's robustness variable*/
	uint8 qqic;					/* querier's query interval code */
	uint16 numOfSrc;				/* number of sources */
	uint32 srcList;				/* first entry of source list */	
};

struct groupRecord
{
	uint8	type;					/* Record Type */
	uint8	auxLen;			              /* auxiliary data length, in uints of 32 bit words*/
	uint16	numOfSrc;			       /* number of sources */
	uint32	groupAddr;			       /* group address being reported */
	uint32	srcList;				       /* first entry of source list */
	
};

struct igmpv3Report
{
	uint8	type;					/* Report Type */
	uint8  reserved1;             
	uint16 checkSum;					/*IGMP checksum*/
	uint16 reserved2;
	uint16	numOfRecords;			       /* number of Group records */
	struct groupRecord recordList;       /*first entry of group record list*/
};

struct rtl_groupEntry{
	uint32 groupAddress;                       /*D class IP multicast address*/
	struct rtl_groupEntry* next;             /*Pointer of next group entry*/
	uint16 portTimer[5];   
	uint8   fwdPortMask;                      /*the port mask of packet forwarding*/
	uint8   lookupTableFlag;		    /*to indicate whether this entry has been set into rtl8305sd lookup table*/
	
};

struct rtl_mcastRouter{	
uint8  portMask;
uint16 portTimer[6];
};


struct rtl_macFrameInfo{
	uint8  cpuTagFlag;
	uint8   cpuPriority;
	uint8   pktPortMask;
	uint8   l3Protocol;
	struct ipv4Pkt* ipBuf;
	uint32 ipHdrLen;
	uint8* l3PktBuf;
	uint16 l3PktLen;
	uint16 macFrameLen;
	uint16 vlanTagFlag;
	uint16 vlanTagID;

};


struct rtl_igmpSnoopingRouters{
 struct rtl_mcastRouter querier;
 struct rtl_mcastRouter dvmrpRouter;
 struct rtl_mcastRouter mospfRouter;
 struct rtl_mcastRouter pimDmRouter;
 		 
};

#ifdef RTL_IGMP_SNOOPING_TEST
int32  rtl_exitIgmpSnoopingV1(void);
#endif

#ifdef IGMP_DEBUG
#define DUMP_IGMP_SETTINGS				1
#define DUMP_MCAST_ROUTER_INFORMATION	2
#define DUMP_GROUP_INFORMATION			3
#define DUMP_ALL							4

void rtl_igmpDump(uint32 dumpOpration, uint32 groupAddress);
void rtl_setCpuPortNum(uint32 cpuPortNum);
void rtl_setMCastDataToCpu(uint32 toCpuFlag);
void rtl_setCpuIsIgmpRouter(uint32 cpuIsIgmpRouter);
int32 rtl_checkGroupStatus(uint32 groupAddress, uint32 *lutFlag, uint32 *aggregatorFlag);
#endif

