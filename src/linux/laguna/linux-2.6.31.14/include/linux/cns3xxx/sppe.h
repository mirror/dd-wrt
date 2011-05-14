/*
 * PROJECT CODE:	CNS3XXX Smart Packet Processing Engine
 * MODULE NAME:		sppe.h
 * DESCRIPTION:		
 *
 * Change Log
 *
 * 1.0.0    25-Dec-2008
 * o  
 *
 */

#ifndef _SPPE_H_
#define _SPPE_H_

#if defined(CONFIG_CNS3XXX_SPPE)


/* PPE Table Size Def. */
#define PPE_TABLE_SIZE_2K      (0x0)
#define PPE_TABLE_SIZE_4K      (0x1)
#define PPE_TABLE_SIZE_8K      (0x2)
#define PPE_TABLE_SIZE_16K     (0x3)
#define PPE_TABLE_SIZE_32K     (0x4)
#define PPE_TABLE_SIZE_64K     (0x5)
#define PPE_TABLE_SIZE_128K    (0x6)
#define PPE_TABLE_SIZE_256K    (0x7)

#define ONE_MATCH_METHOD_SRC_IP				0
#define ONE_MATCH_METHOD_SRC_PORT			1
#define ONE_MATCH_METHOD_DEST_IP			2
#define ONE_MATCH_METHOD_DEST_PORT		3

typedef enum _sppe_cmd {
	SPPE_CMD_INIT = 0,
	SPPE_CMD_VERSION,

	SPPE_CMD_ENABLE,
	SPPE_CMD_FIREWALL,
	SPPE_CMD_RULE_CHECK,
	SPPE_CMD_GRL_CHECK,
	SPPE_CMD_FLOW_CHECK,
	SPPE_CMD_RATE_LIMIT_EN,
	SPPE_CMD_POLICE_EN,
	SPPE_CMD_RLCFG,
	SPPE_CMD_FC,			/* flow control */
	SPPE_CMD_MIRROR_TO_CPU,	
	
	SPPE_CMD_TCP_SNA_TH,
	SPPE_CMD_PRDA,
	SPPE_CMD_AGING,
	SPPE_CMD_MAX_LENGTH,

	SPPE_CMD_LANIPV4,
	SPPE_CMD_WANIPV4,

	SPPE_CMD_RULE_PPPOE_RELAY,
	SPPE_CMD_RULE_BRIDGE,
	SPPE_CMD_RULE_ACL,
	SPPE_CMD_RULE_ROUTE,
#if 0
	SPPE_CMD_RULE_VSERVER,
#else
	SPPE_CMD_RULE_SNAT,
	SPPE_CMD_RULE_DNAT,
#endif
	SPPE_CMD_RULE_GRL,

	SPPE_CMD_ARP,
	SPPE_CMD_ARL,
	SPPE_CMD_PPPOE_SID,

	SPPE_CMD_FLOW_BRIDGE_IPV4,
	SPPE_CMD_FLOW_BRIDGE_IPV6,
	SPPE_CMD_FLOW_ROUTE_IPV4,
	SPPE_CMD_FLOW_ROUTE_IPV6,
	SPPE_CMD_FLOW_NAT_IPV4,
	SPPE_CMD_FLOW_NAT_IPV6,
	//SPPE_CMD_FLOW_TWICE_NAT,
	SPPE_CMD_FLOW_MCAST_IPV4,
	SPPE_CMD_FLOW_MCAST_IPV6,
	SPPE_CMD_FLOW_BRIDGE_L2,

	SPPE_CMD_CHGDSCP,
	SPPE_CMD_CHGPRI,
	SPPE_CMD_RL_FLOW,
	SPPE_CMD_RL_RULE,

	SPPE_CMD_DEBUG,
	SPPE_CMD_REG,
	SPPE_CMD_SRAM,
	SPPE_CMD_DUMP,

	/* accounting group and drop packet count */
	SPPE_CMD_ACCOUNTING_GROUP,
	SPPE_CMD_DROP_IPCS_ERR,
	SPPE_CMD_DROP_RATE_LIMIT,
	SPPE_CMD_DROP_OTHERS,

	SPPE_CMD_PCI_FP_DEV,
	SPPE_CMD_HOOK_MODE,
	
	SPPE_CMD_RL_FLOW_EX,
	SPPE_CMD_RL_ONE_MATCH,

} SPPE_CMD;

typedef enum _sppe_op {
	SPPE_OP_GET = 0,
	SPPE_OP_SET,
	SPPE_OP_DELETE,
	SPPE_OP_DELETE_OUTDATED, /* flow only */
	SPPE_OP_UPDATE_COUNTER, /* ACL rule only */
	SPPE_OP_CLEAN,
	SPPE_OP_UNKNOWN
} SPPE_OP;

typedef enum _sppe_boolean {
    SPPE_BOOL_FALSE = 0,
    SPPE_BOOL_TRUE = 1
} SPPE_BOOL;


typedef enum _sppe_result {
	SPPE_RESULT_SUCCESS = 0,
	SPPE_RESULT_FAIL,
	SPPE_RESULT_UNSUPPORT_CMD,
	SPPE_RESULT_UNSUPPORT_OP,
	SPPE_RESULT_INVALID_INDEX,
	SPPE_RESULT_INVALID_TYPE,
	SPPE_RESULT_FLOW_NOT_FOUND,
} SPPE_RESULT;

typedef enum _sppe_prot {
	SPPE_PROT_UDP = 0,
	SPPE_PROT_TCP = 1,
	SPPE_PROT_PPTP_GRE = 2,
	SPPE_PROT_OTHERS = 3,
} SPPE_PROT;


typedef enum _sppe_l2_select {
	SPPE_L2S_ARP_TABLE = 0,
	SPPE_L2S_POLICY_ROUTE = 1,
	SPPE_L2S_IN_FLOW = 2,
	SPPE_L2S_RESERVED = 3,
} SPPE_L2_SELECT;

typedef enum _sppe_dump_type {
	SPPE_DUMP_TYPE_FLOW = 0,
	SPPE_DUMP_TYPE_ARP,
	SPPE_DUMP_TYPE_RULE
} SPPE_DUMP_TYPE;

/* Data Structure */
typedef struct _sppe_pppoe_relay {
#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int valid:1;
	unsigned int unused:31;
#else
	unsigned int unused:31;
	unsigned int valid:1;
#endif
	unsigned short lsid; /* PPPoE session ID in LAN side */
	unsigned short wsid; /* PPPoE session ID in WAN side */
	unsigned char lmac[6]; /* MAC address of PPPoE client */
	unsigned char wmac[6]; /* MAC address of PPPoE server */
} SPPE_PPPOE_RELAY;

typedef struct _sppe_bridge {
#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int valid:1;
	unsigned int wan:1;
	unsigned int ppp:1; /* enable PPPoE sessoion ID comparison*/
	unsigned int psidx:4; /* PPPoE session ID index */
	unsigned int kv:1;
	unsigned int sws:1;
	unsigned int max_len:2; /* Max. length select */
	unsigned int fp:1;	/* force VLAN priority */
	unsigned int pri:3;
	unsigned int ag:2;
	#if 0 
	unsigned int unused:15;
	#else
	unsigned int to:4;
	unsigned int from:4;
	unsigned int nosa:1;
	unsigned int noda:1;
	unsigned int unused:5;
	#endif
#else
	#if 0
	unsigned int unused:15;
	#else
	unsigned int unused:5;
	unsigned int noda:1;
	unsigned int nosa:1;
	unsigned int from:4;
	unsigned int to:4;
	#endif
	unsigned int ag:2;
	unsigned int pri:3;
	unsigned int fp:1;	/* force VLAN priority */
	unsigned int max_len:2; /* Max. length select */
	unsigned int sws:1;
	unsigned int kv:1;
	unsigned int psidx:4; /* PPPoE session ID index */
	unsigned int ppp:1; /* enable PPPoE sessoion ID comparison*/
	unsigned int wan:1;
	unsigned int valid:1;
#endif

#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int svid:12;
	unsigned int cvid:12;
	unsigned int loc:8;
#else
	unsigned int loc:8;
	unsigned int cvid:12;
	unsigned int svid:12;
#endif
			
	unsigned char smac[6]; /* source MAC address  */
	unsigned char dmac[6]; /* destination MAC address */
	unsigned int pkt_cnt;
} SPPE_BRIDGE;

typedef struct _sppe_acl {
#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int valid:1;
	unsigned int ipv6:1;
	unsigned int wan:1;
	unsigned int tcp:1;
	unsigned int udp:1;
	unsigned int to:4;
	unsigned int from:4;
	unsigned int rr:4;
	unsigned int kv:1;
	unsigned int sws:1;
	unsigned int loc:8;
	unsigned int max_len:2; /* Max. length select */
	unsigned int unused:3;
#else
	unsigned int unused:3;
	unsigned int max_len:2; /* Max. length select */
	unsigned int loc:8;
	unsigned int sws:1;
	unsigned int kv:1;
	unsigned int rr:4;
	unsigned int from:4;
	unsigned int to:4;
	unsigned int udp:1;
	unsigned int tcp:1;
	unsigned int wan:1;
	unsigned int ipv6:1;
	unsigned int valid:1;
#endif

	unsigned int sip[4];
	unsigned int dip[4];
	unsigned short sip_mask;
	unsigned short dip_mask;

	unsigned short sport_start;
	unsigned short sport_end;
	unsigned short dport_start;
	unsigned short dport_end;
	unsigned int pkt_cnt;
} SPPE_ACL;

typedef struct _sppe_route {
#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int valid:1;
	unsigned int ipv6:1;
	unsigned int wan:1;
	unsigned int rd:1; /* replace dscp */
	unsigned int dscp:6;
	unsigned int pr:1;	/* policy route */
	unsigned int prs:2;	/* policy route select */
	unsigned int kv:1;
	unsigned int sws:1;
	unsigned int max_len:2; /* Max. length select */
	unsigned int fp:1;	/* force VLAN priority */
	unsigned int pri:3;
	unsigned int pd:1;
	unsigned int pi:1;
	unsigned int psidx:4;
	unsigned int ag:2;
	unsigned int unused:3;
#else
	unsigned int unused:3;
	unsigned int ag:2;
	unsigned int psidx:4;
	unsigned int pi:1;
	unsigned int pd:1;
	unsigned int pri:3;
	unsigned int fp:1;	/* force VLAN priority */
	unsigned int max_len:2; /* Max. length select */
	unsigned int sws:1;
	unsigned int kv:1;
	unsigned int prs:2;	/* policy route select */
	unsigned int pr:1;	/* policy route */
	unsigned int dscp:6;
	unsigned int rd:1; /* replace dscp */
	unsigned int wan:1;
	unsigned int ipv6:1;
	unsigned int valid:1;
#endif	

#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int unused_1:24;
	unsigned int loc:8;
#else
	unsigned int loc:8;
	unsigned int unused_1:24;
#endif

	unsigned int dip[4];
	unsigned int sip[4];
	unsigned short dip_mask;
	unsigned short sip_mask;
	unsigned int pkt_cnt;
} SPPE_ROUTE;

#if 0
typedef struct _sppe_vserver {
	unsigned int valid:1;
	unsigned int tcp:1;
	unsigned int udp:1;
	unsigned int dscp_lan:6;
	unsigned int dscp_wan:6;
	unsigned int pri_lan:3;
	unsigned int pri_wan:3;
	unsigned int unused:11;

	unsigned int wanip;
	unsigned int lanip;
	unsigned short port_start;
	unsigned short port_end;
	unsigned int pkt_cnt;
} SPPE_VSERVER;
#else
typedef struct _sppe_snat {
#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int valid:1;
	unsigned int tcp:1;
	unsigned int udp:1;
	unsigned int rd:1;
	unsigned int dscp:6;
	unsigned int fp:1;
	unsigned int pri:3;
	unsigned int kv:1;
	unsigned int sws:1;
	unsigned int max_len:2;
	unsigned int pd:1;
	unsigned int pi:1;
	unsigned int psidx:4;
	unsigned int pr:1;	/* policy route */
	unsigned int prs:2;	/* policy route select */
	unsigned int ag:2;
	unsigned int unused:3;
#else
	unsigned int unused:3;
	unsigned int ag:2;
	unsigned int prs:2;	/* policy route select */
	unsigned int pr:1;	/* policy route */
	unsigned int psidx:4;
	unsigned int pi:1;
	unsigned int pd:1;
	unsigned int max_len:2;
	unsigned int sws:1;
	unsigned int kv:1;
	unsigned int pri:3;
	unsigned int fp:1;
	unsigned int dscp:6;
	unsigned int rd:1;
	unsigned int udp:1;
	unsigned int tcp:1;
	unsigned int valid:1;
#endif

#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int unused_1:24;
	unsigned int loc:8;
#else
	unsigned int loc:8;
	unsigned int unused_1:24;
#endif

	unsigned int wanip;
	unsigned int lanip;
	unsigned short port_start;
	unsigned short port_end;
	unsigned int pkt_cnt;
} SPPE_SNAT;

typedef struct _sppe_dnat {
#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int valid:1;
	unsigned int tcp:1;
	unsigned int udp:1;
	unsigned int rd:1;
	unsigned int dscp:6;
	unsigned int fp:1;
	unsigned int pri:3;
	unsigned int kv:1;
	unsigned int sws:1;
	unsigned int max_len:2;
	unsigned int pd:1;
	unsigned int pi:1;
	unsigned int psidx:4;
	unsigned int pr:1;	/* policy route */
	unsigned int prs:2;	/* policy route select */
	unsigned int ag:2;
	unsigned int unused:3;
#else
	unsigned int unused:3;
	unsigned int ag:2;
	unsigned int prs:2;	/* policy route select */
	unsigned int pr:1;	/* policy route */
	unsigned int psidx:4;
	unsigned int pi:1;
	unsigned int pd:1;
	unsigned int max_len:2;
	unsigned int sws:1;
	unsigned int kv:1;
	unsigned int pri:3;
	unsigned int fp:1;
	unsigned int dscp:6;
	unsigned int rd:1;
	unsigned int udp:1;
	unsigned int tcp:1;
	unsigned int valid:1;
#endif
#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int unused_1:24;
	unsigned int loc:8;
#else
	unsigned int loc:8;
	unsigned int unused_1:24;
#endif

	unsigned int wanip;
	unsigned int lanip;
	unsigned short port_start;
	unsigned short port_end;
	unsigned int pkt_cnt;
} SPPE_DNAT;
#endif
typedef struct _sppe_limit {
#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int drop_red:1;
	unsigned int pass_green:1;
	unsigned int force_color:1;
	unsigned int color_select:2;
	unsigned int time_stamp:21;
	unsigned int reserved:6;
#else
	unsigned int reserved:6;
	unsigned int time_stamp:21;
	unsigned int color_select:2;
	unsigned int force_color:1;
	unsigned int pass_green:1;
	unsigned int drop_red:1;
#endif
	unsigned short min_rate;
	unsigned short max_rate;
} SPPE_LIMIT;

typedef struct _sppe_global_rate_limit {
#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int valid:1;
	unsigned int wan:1;
	unsigned int ipv6:1;
	unsigned int tcp:1;
	unsigned int udp:1;
	unsigned int unused:17;
#else
	unsigned int unused:17;
	unsigned int udp:1;
	unsigned int tcp:1;
	unsigned int ipv6:1;
	unsigned int wan:1;
	unsigned int valid:1;
#endif
	
	unsigned int sip[4];
	unsigned int dip[4];
	unsigned short sip_mask;
	unsigned short dip_mask;
	unsigned short sport_start;
	unsigned short sport_end;
	unsigned short dport_start;
	unsigned short dport_end;
	SPPE_LIMIT limit;
} SPPE_GLOBAL_RATE_LIMIT;

/* 
 * SPPE_CMD_FLOW_BRIDGE_IPV4 
 * type = 1 , as = 3
 */
typedef struct _sppe_flow_bridge_ipv4 {
#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int fw:1;
	unsigned int s:1;
	unsigned int sws:1;
	unsigned int ag:2;
	unsigned int rl:1;
	unsigned int l4_prot:2;
	unsigned int l2s:2; /* L2 select */
	unsigned int prs:2;
	unsigned int kv:1;
	unsigned int fp:1;
	unsigned int pri:3;
	unsigned int max_len:2; /* Max. length select */
	unsigned int reserved:13;
#else
	unsigned int reserved:13;
	unsigned int max_len:2; /* Max. length select */
	unsigned int pri:3;
	unsigned int fp:1;
	unsigned int kv:1;
	unsigned int prs:2;
	unsigned int l2s:2; /* L2 select */
	unsigned int l4_prot:2;
	unsigned int rl:1;
	unsigned int ag:2;
	unsigned int sws:1;
	unsigned int s:1;
	unsigned int fw:1;
#endif
	
#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int unused:16;
	unsigned int mac4732:16;
#else
	unsigned int mac4732:16;
	unsigned int unused:16;
#endif
	
	unsigned int mac3100;
#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int lp:1;	
	unsigned int fr:1;
	unsigned int pm:4;
	unsigned int sv:1;
	unsigned int svid:12;
	unsigned int cv:1;	
	unsigned int cvid:12;
#else
	unsigned int cvid:12;
	unsigned int cv:1;	
	unsigned int svid:12;
	unsigned int sv:1;
	unsigned int pm:4;
	unsigned int fr:1;
	unsigned int lp:1;	
#endif
	unsigned int sip;
	unsigned int dip;

	union {
		struct {
			unsigned short src;
			unsigned short dst;
		} port;
		struct {
			unsigned short call_id;
		} gre;
		struct {
			unsigned char protocol;
		} others;
	} l4;

	SPPE_LIMIT limit;
	unsigned int pkt_cnt;
} SPPE_FLOW_BRIDGE_IPV4;

/* 
 * SPPE_CMD_FLOW_BRIDGE_IPV6 
 * type = 2 , as = 3
 */
typedef struct _sppe_flow_bridge_ipv6 {
#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int fw:1;
	unsigned int s:1;
	unsigned int sws:1;
	unsigned int ag:2;
	unsigned int rl:1;
	unsigned int l4_prot:2;
	unsigned int l2s:2; /* L2 select */
	unsigned int prs:2;
	unsigned int kv:1;
	unsigned int fp:1;
	unsigned int pri:3;
	unsigned int max_len:2; /* Max. length select */
	unsigned int reserved:13;
#else
	unsigned int reserved:13;
	unsigned int max_len:2; /* Max. length select */
	unsigned int pri:3;
	unsigned int fp:1;
	unsigned int kv:1;
	unsigned int prs:2;
	unsigned int l2s:2; /* L2 select */
	unsigned int l4_prot:2;
	unsigned int rl:1;
	unsigned int ag:2;
	unsigned int sws:1;
	unsigned int s:1;
	unsigned int fw:1;
#endif
	
#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int unused:16;
	unsigned int mac4732:16;
#else
	unsigned int mac4732:16;
	unsigned int unused:16;
#endif
	
	unsigned int mac3100;
#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int lp:1;	
	unsigned int fr:1;
	unsigned int pm:4;
	unsigned int sv:1;
	unsigned int svid:12;
	unsigned int cv:1;	
	unsigned int cvid:12;
#else
	unsigned int cvid:12;
	unsigned int cv:1;	
	unsigned int svid:12;
	unsigned int sv:1;
	unsigned int pm:4;
	unsigned int fr:1;
	unsigned int lp:1;	
#endif
	unsigned int sip[4];
	unsigned int dip[4];
	union {
		struct {
			unsigned short src;
			unsigned short dst;
		} port;
		struct {
			unsigned short call_id;
		} gre;
		struct {
			unsigned char protocol;
		} others;
	} l4;
	SPPE_LIMIT limit;
	unsigned int pkt_cnt;
} SPPE_FLOW_BRIDGE_IPV6;

/* 
 * SPPE_CMD_FLOW_ROUTE_IPV4
 * type = 1, as = 0
 */
typedef struct _sppe_flow_route_ipv4 {
#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int fw:1;
	unsigned int s:1;
	unsigned int sws:1;
	unsigned int ag:2;
	unsigned int rl:1;
	unsigned int l4_prot:2;
	unsigned int l2s:2; /* L2 select */
	unsigned int prs:2;
	unsigned int kv:1;
	unsigned int rd:1;
	unsigned int dscp:6;
	unsigned int fp:1;
	unsigned int pri:3;
	unsigned int max_len:2; /* Max. length select */
	unsigned int pd:1;
	unsigned int pi:1;
	unsigned int psidx:4;
#else
	unsigned int psidx:4;
	unsigned int pi:1;
	unsigned int pd:1;
	unsigned int max_len:2; /* Max. length select */
	unsigned int pri:3;
	unsigned int fp:1;
	unsigned int dscp:6;
	unsigned int rd:1;
	unsigned int kv:1;
	unsigned int prs:2;
	unsigned int l2s:2; /* L2 select */
	unsigned int l4_prot:2;
	unsigned int rl:1;
	unsigned int ag:2;
	unsigned int sws:1;
	unsigned int s:1;
	unsigned int fw:1;
#endif
	
#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int unused:16;
	unsigned int mac4732:16;
#else
	unsigned int mac4732:16;
	unsigned int unused:16;
#endif
	unsigned int mac3100;

#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int lp:1;	
	unsigned int fr:1;
	unsigned int pm:4;
	unsigned int sv:1;
	unsigned int svid:12;
	unsigned int cv:1;	
	unsigned int cvid:12;
#else
	unsigned int cvid:12;
	unsigned int cv:1;	
	unsigned int svid:12;
	unsigned int sv:1;
	unsigned int pm:4;
	unsigned int fr:1;
	unsigned int lp:1;	
#endif

	unsigned int sip;
	unsigned int dip;
	union {
		struct {
			unsigned short src;
			unsigned short dst;
		} port;
		struct {
			unsigned short call_id;
		} gre;
		struct {
			unsigned char protocol;
		} others;
	} l4;
	SPPE_LIMIT limit;
	unsigned int pkt_cnt;
} SPPE_FLOW_ROUTE_IPV4;

/* 
 * SPPE_CMD_FLOW_ROUTE_IPV6
 * type = 2, as = 0
 */
typedef struct _sppe_flow_route_ipv6 {
#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int fw:1;
	unsigned int s:1;
	unsigned int sws:1;
	unsigned int ag:2;
	unsigned int rl:1;
	unsigned int l4_prot:2;
	unsigned int l2s:2; /* L2 select */
	unsigned int prs:2;
	unsigned int kv:1;
	unsigned int rd:1;
	unsigned int dscp:6;
	unsigned int fp:1;
	unsigned int pri:3;
	unsigned int max_len:2; /* Max. length select */
	unsigned int pd:1;
	unsigned int pi:1;
	unsigned int psidx:4;
#else
	unsigned int psidx:4;
	unsigned int pi:1;
	unsigned int pd:1;
	unsigned int max_len:2; /* Max. length select */
	unsigned int pri:3;
	unsigned int fp:1;
	unsigned int dscp:6;
	unsigned int rd:1;
	unsigned int kv:1;
	unsigned int prs:2;
	unsigned int l2s:2; /* L2 select */
	unsigned int l4_prot:2;
	unsigned int rl:1;
	unsigned int ag:2;
	unsigned int sws:1;
	unsigned int s:1;
	unsigned int fw:1;
#endif
#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int unused:16;
	unsigned int mac4732:16;
#else
	unsigned int mac4732:16;
	unsigned int unused:16;
#endif	
	unsigned int mac3100;

#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int lp:1;	
	unsigned int fr:1;
	unsigned int pm:4;
	unsigned int sv:1;
	unsigned int svid:12;
	unsigned int cv:1;	
	unsigned int cvid:12;
#else
	unsigned int cvid:12;
	unsigned int cv:1;	
	unsigned int svid:12;
	unsigned int sv:1;
	unsigned int pm:4;
	unsigned int fr:1;
	unsigned int lp:1;	
#endif
	unsigned int sip[4];
	unsigned int dip[4];
	union {
		struct {
			unsigned short src;
			unsigned short dst;
		} port;
		struct {
			unsigned short call_id;
		} gre;
		struct {
			unsigned char protocol;
		} others;
	} l4;
	SPPE_LIMIT limit;
	unsigned int pkt_cnt;
} SPPE_FLOW_ROUTE_IPV6;

/*
 * SPPE_CMD_FLOW_NAT_IPV4 
 * type = 0, as = 1
 */
typedef struct _sppe_flow_nat_ipv4 {
#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int fw:1;
	unsigned int s:1;
	unsigned int sws:1;
	unsigned int ag:2;
	unsigned int rl:1;
	unsigned int l4_prot:2;
	unsigned int l2s:2; /* L2 select */
	unsigned int prs:2;
	unsigned int kv:1;
	unsigned int rd:1;
	unsigned int dscp:6;
	unsigned int fp:1;
	unsigned int pri:3;
	unsigned int max_len:2; /* Max. length select */
	unsigned int pd:1;
	unsigned int pi:1;
	unsigned int psidx:4;
#else
	unsigned int psidx:4;
	unsigned int pi:1;
	unsigned int pd:1;
	unsigned int max_len:2; /* Max. length select */
	unsigned int pri:3;
	unsigned int fp:1;
	unsigned int dscp:6;
	unsigned int rd:1;
	unsigned int kv:1;
	unsigned int prs:2;
	unsigned int l2s:2; /* L2 select */
	unsigned int l4_prot:2;
	unsigned int rl:1;
	unsigned int ag:2;
	unsigned int sws:1;
	unsigned int s:1;
	unsigned int fw:1;
#endif
#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int unused:16;
	unsigned int mac4732:16;
#else
	unsigned int mac4732:16;
	unsigned int unused:16;
#endif

	unsigned int mac3100;

#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int lp:1;	
	unsigned int fr:1;
	unsigned int pm:4;
	unsigned int sv:1;
	unsigned int svid:12;
	unsigned int cv:1;	
	unsigned int cvid:12;
#else
	unsigned int cvid:12;
	unsigned int cv:1;	
	unsigned int svid:12;
	unsigned int sv:1;
	unsigned int pm:4;
	unsigned int fr:1;
	unsigned int lp:1;	
#endif

	unsigned int sip;
	unsigned int dip;
	union {
		struct {
			unsigned short src;
			unsigned short dst;
		} port;
		struct {
			unsigned short call_id;
			unsigned short nat_call_id;
		} gre;
		struct {
			unsigned char protocol;
		} others;
	} l4;
	unsigned int nat_ip;
	unsigned short nat_port;
	SPPE_LIMIT limit;
	unsigned int pkt_cnt;
} SPPE_FLOW_NAT_IPV4;

/*
 * SPPE_CMD_FLOW_NAT_IPV6
 * type = 1, as = 1
 */
typedef struct _sppe_flow_nat_ipv6 {
#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int fw:1;
	unsigned int s:1;
	unsigned int sws:1;
	unsigned int ag:2;
	unsigned int rl:1;
	unsigned int l4_prot:2;
	unsigned int l2s:2; /* L2 select */
	unsigned int prs:2;
	unsigned int kv:1;
	unsigned int rd:1;
	unsigned int dscp:6;
	unsigned int fp:1;
	unsigned int pri:3;
	unsigned int max_len:2; /* Max. length select */
	unsigned int pd:1;
	unsigned int pi:1;
	unsigned int psidx:4;
#else
	unsigned int psidx:4;
	unsigned int pi:1;
	unsigned int pd:1;
	unsigned int max_len:2; /* Max. length select */
	unsigned int pri:3;
	unsigned int fp:1;
	unsigned int dscp:6;
	unsigned int rd:1;
	unsigned int kv:1;
	unsigned int prs:2;
	unsigned int l2s:2; /* L2 select */
	unsigned int l4_prot:2;
	unsigned int rl:1;
	unsigned int ag:2;
	unsigned int sws:1;
	unsigned int s:1;
	unsigned int fw:1;
#endif
#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int unused:16;
	unsigned int mac4732:16;
#else
	unsigned int mac4732:16;
	unsigned int unused:16;
#endif
	unsigned int mac3100;
#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int lp:1;	
	unsigned int fr:1;
	unsigned int pm:4;
	unsigned int sv:1;
	unsigned int svid:12;
	unsigned int cv:1;	
	unsigned int cvid:12;
#else
	unsigned int cvid:12;
	unsigned int cv:1;	
	unsigned int svid:12;
	unsigned int sv:1;
	unsigned int pm:4;
	unsigned int fr:1;
	unsigned int lp:1;	
#endif
	unsigned int sip[4];
	unsigned int dip[4];
	union {
		struct {
			unsigned short src;
			unsigned short dst;
		} port;
		struct {
			unsigned short call_id;
			unsigned short nat_call_id;
		} gre;
		struct {
			unsigned char protocol;
		} others;
	} l4;
	unsigned int nat_ip[4];
	unsigned short nat_port;
	SPPE_LIMIT limit;
	unsigned int pkt_cnt;
} SPPE_FLOW_NAT_IPV6;

/*
 * SPPE_CMD_FLOW_TWICE_NAT
 * type = 0, as = 2
 */
typedef struct _sppe_flow_twice_nat {
#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int fw:1;
	unsigned int s:1;
	unsigned int sws:1;
	unsigned int ag:2;
	unsigned int rl:1;
	unsigned int l4_prot:2;
	unsigned int l2s:2; /* L2 select */
	unsigned int prs:2;
	unsigned int kv:1;
	unsigned int rd:1;
	unsigned int dscp:6;
	unsigned int fp:1;
	unsigned int pri:3;
	unsigned int max_len:2; /* Max. length select */
	unsigned int psidx:4;
	unsigned int reserved:2;
#else
	unsigned int reserved:2;
	unsigned int psidx:4;
	unsigned int max_len:2; /* Max. length select */
	unsigned int pri:3;
	unsigned int fp:1;
	unsigned int dscp:6;
	unsigned int rd:1;
	unsigned int kv:1;
	unsigned int prs:2;
	unsigned int l2s:2; /* L2 select */
	unsigned int l4_prot:2;
	unsigned int rl:1;
	unsigned int ag:2;
	unsigned int sws:1;
	unsigned int s:1;
	unsigned int fw:1;
#endif
	
#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int unused:16;
	unsigned int mac4732:16;
#else
	unsigned int mac4732:16;
	unsigned int unused:16;
#endif
	unsigned int mac3100;

#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int lp:1;	
	unsigned int fr:1;
	unsigned int pm:4;
	unsigned int sv:1;
	unsigned int svid:12;
	unsigned int cv:1;	
	unsigned int cvid:12;
#else
	unsigned int cvid:12;
	unsigned int cv:1;	
	unsigned int svid:12;
	unsigned int sv:1;
	unsigned int pm:4;
	unsigned int fr:1;
	unsigned int lp:1;	
#endif
	unsigned int sip;
	unsigned int dip;
	unsigned short sport;
	unsigned short dport;
	unsigned int natsip;
	unsigned int natdip;
	unsigned short natsport;
	unsigned short natdport;
	SPPE_LIMIT limit;
	unsigned int pkt_cnt;
} SPPE_FLOW_TWICE_NAT;

/*
 * SPPE_CMD_FLOW_MULTICAST_IPV4
 * type = 0, as = 0 or 3
 */
typedef struct _sppe_flow_multicast_ipv4 {
#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int fw:1;
	unsigned int s:1;
	unsigned int sws:1;
	unsigned int ag:2;
	unsigned int rl:1;
	unsigned int l2s:2; /* L2 select */
	unsigned int prs:2;
	unsigned int kv:1;
	unsigned int rd:1;
	unsigned int dscp:6;
	unsigned int fp:1;
	unsigned int pri:3;
	unsigned int max_len:2; /* Max. length select */
	unsigned int bridge:1;
	unsigned int reserved:7;
#else
	unsigned int reserved:7;
	unsigned int bridge:1;
	unsigned int max_len:2; /* Max. length select */
	unsigned int pri:3;
	unsigned int fp:1;
	unsigned int dscp:6;
	unsigned int rd:1;
	unsigned int kv:1;
	unsigned int prs:2;
	unsigned int l2s:2; /* L2 select */
	unsigned int rl:1;
	unsigned int ag:2;
	unsigned int sws:1;
	unsigned int s:1;
	unsigned int fw:1;
#endif
#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int unused:16;
	unsigned int mac4732:16;
#else
	unsigned int mac4732:16;
	unsigned int unused:16;
#endif
	unsigned int mac3100;

#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int lp:1;	
	unsigned int fr:1;
	unsigned int pm:4;
	unsigned int sv:1;
	unsigned int svid:12;
	unsigned int cv:1;	
	unsigned int cvid:12;
#else
	unsigned int cvid:12;
	unsigned int cv:1;	
	unsigned int svid:12;
	unsigned int sv:1;
	unsigned int pm:4;
	unsigned int fr:1;
	unsigned int lp:1;	
#endif

	unsigned int sip;
	unsigned int dip;
	SPPE_LIMIT limit;
	unsigned int pkt_cnt;
} SPPE_FLOW_MCAST_IPV4;

/*
 * SPPE_CMD_FLOW_MULTICAST_IPV6
 * type = 1, as = 0 or 3
 */
typedef struct _sppe_flow_multicast_ipv6 {
#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int fw:1;
	unsigned int s:1;
	unsigned int sws:1;
	unsigned int ag:2;
	unsigned int rl:1;
	unsigned int l2s:2; /* L2 select */
	unsigned int prs:2;
	unsigned int kv:1;
	unsigned int rd:1;
	unsigned int dscp:6;
	unsigned int fp:1;
	unsigned int pri:3;
	unsigned int max_len:2; /* Max. length select */
	unsigned int bridge:1;
	unsigned int reserved:7;
#else
	unsigned int reserved:7;
	unsigned int bridge:1;
	unsigned int max_len:2; /* Max. length select */
	unsigned int pri:3;
	unsigned int fp:1;
	unsigned int dscp:6;
	unsigned int rd:1;
	unsigned int kv:1;
	unsigned int prs:2;
	unsigned int l2s:2; /* L2 select */
	unsigned int rl:1;
	unsigned int ag:2;
	unsigned int sws:1;
	unsigned int s:1;
	unsigned int fw:1;
#endif
	
#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int unused:16;
	unsigned int mac4732:16;
#else
	unsigned int mac4732:16;
	unsigned int unused:16;
#endif
	unsigned int mac3100;

#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int lp:1;	
	unsigned int fr:1;
	unsigned int pm:4;
	unsigned int sv:1;
	unsigned int svid:12;
	unsigned int cv:1;	
	unsigned int cvid:12;
#else
	unsigned int cvid:12;
	unsigned int cv:1;	
	unsigned int svid:12;
	unsigned int sv:1;
	unsigned int pm:4;
	unsigned int fr:1;
	unsigned int lp:1;	
#endif

	unsigned int sip[4];
	unsigned int dip[4];
	SPPE_LIMIT limit;
	unsigned int pkt_cnt;
} SPPE_FLOW_MCAST_IPV6;

/* 
 * SPPE_CMD_FLOW_LAYER_TWO
 * type = 2
 */
typedef struct _sppe_flow_bridge_l2 {
#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int fw:1;
	unsigned int s:1;
	unsigned int sws:1;
	unsigned int ag:2;
	unsigned int rl:1;
	unsigned int l2_prot:2;
	unsigned int kv:1;
	unsigned int fp:1;
	unsigned int pri:3;
	unsigned int psidx:4;
	unsigned int reserved:15;
#else
	unsigned int reserved:15;
	unsigned int psidx:4;
	unsigned int pri:3;
	unsigned int fp:1;
	unsigned int kv:1;
	unsigned int l2_prot:2;
	unsigned int rl:1;
	unsigned int ag:2;
	unsigned int sws:1;
	unsigned int s:1;
	unsigned int fw:1;
#endif

#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int lp:1;	
	unsigned int fr:1;
	unsigned int pm:4;
	unsigned int sv:1;
	unsigned int svid:12;
	unsigned int cv:1;	
	unsigned int cvid:12;
#else
	unsigned int cvid:12;
	unsigned int cv:1;	
	unsigned int svid:12;
	unsigned int sv:1;
	unsigned int pm:4;
	unsigned int fr:1;
	unsigned int lp:1;	
#endif

	unsigned short smac[3];
	unsigned short dmac[3];
	
	SPPE_LIMIT limit;
	unsigned int pkt_cnt;
} SPPE_FLOW_BRIDGE_L2;

typedef struct _sppe_arl {
#ifndef CONFIG_SWITCH_BIG_ENDIAN
	unsigned int vid:12;
	unsigned int pmap:5;
	unsigned int age:3;
	unsigned int mymac:1;
	unsigned int filter:1;
	unsigned int reserved:10;
#else
	unsigned int reserved:10;
	unsigned int filter:1;
	unsigned int mymac:1;
	unsigned int age:3;
	unsigned int pmap:5;
	unsigned int vid:12;
#endif
	unsigned char mac[6];
} SPPE_ARL;

typedef struct _sppe_init {
	unsigned int flow_pre_match_paddr;
	unsigned int flow_pre_match_vaddr;
	unsigned int flow_body_paddr;
	unsigned int flow_body_vaddr;
	unsigned int flow_ext_paddr;
	unsigned int flow_ext_vaddr;
	unsigned int flow_size;
	unsigned int arp_pre_match_paddr;
	unsigned int arp_pre_match_vaddr;
	unsigned int arp_body_paddr;
	unsigned int arp_body_vaddr;
	unsigned int arp_size;
	unsigned int ipv6_napt;
} SPPE_INIT;

typedef struct _sppe_param_t {
	SPPE_CMD cmd;
	SPPE_OP op;

	union {
		struct {
			unsigned char major;
			unsigned char minor;
			unsigned char very_minor;
			unsigned char pre;
		} sppe_version;

		SPPE_BOOL sppe_enable;
		unsigned int sppe_lanip; 

		struct {
			unsigned int index;
			unsigned int ip;
			unsigned int session_id;
		} sppe_wanip;

		struct {
#ifndef CONFIG_SWITCH_BIG_ENDIAN
			unsigned int index:2;
			unsigned int to:4;
			unsigned int sv:1;
			unsigned int stag_vid:12;
			unsigned int cv:1;
			unsigned int ctag_vid:12;
#else
			unsigned int ctag_vid:12;
			unsigned int cv:1;
			unsigned int stag_vid:12;
			unsigned int sv:1;
			unsigned int to:4;
			unsigned int index:2;
#endif
			unsigned char mac[6]; /* MAC address */
		} sppe_prda;

		struct {
#ifndef CONFIG_SWITCH_BIG_ENDIAN
			unsigned int interval:2;
			unsigned int mfactor:1;
			unsigned int ununsed:29;
#else
			unsigned int ununsed:29;
			unsigned int mfactor:1;
			unsigned int interval:2;
#endif
		} sppe_rlcfg;

		struct {
			unsigned int index;
			SPPE_PPPOE_RELAY rule;
		} sppe_pppoe_relay;
		
		struct {
			unsigned int index;
			SPPE_BRIDGE rule;
		} sppe_bridge;

		struct {
			unsigned int index;
			SPPE_ACL rule;
		} sppe_acl;

		struct {
			unsigned int index;
			SPPE_ROUTE rule;
		} sppe_route;
#if 0
		struct {
			unsigned int index;
			SPPE_VSERVER rule;
		} sppe_vserver;
#else	
		struct {
			unsigned int index;
			SPPE_SNAT rule;
		} sppe_snat;
		
		struct {
			unsigned int index;
			SPPE_DNAT rule;
		} sppe_dnat;
#endif
		struct {
			unsigned int index;
			SPPE_GLOBAL_RATE_LIMIT rule;
		} sppe_grl;

		struct {
			unsigned char unit;
			unsigned char arp;
			unsigned char bridge;
			unsigned char tcp;
			unsigned char udp;
			unsigned char pptp;
			unsigned char other; 
		} sppe_agingout;

		struct {
#ifndef CONFIG_SWITCH_BIG_ENDIAN
			unsigned int index:2;
			unsigned int reserved:20;
			unsigned int max:10;
#else
			unsigned int max:10;
			unsigned int reserved:20;
			unsigned int index:2;
#endif
		} sppe_max_length;

		struct {
#ifndef CONFIG_SWITCH_BIG_ENDIAN
			unsigned int v6:1;
			unsigned int s:1;
			unsigned int r:1;
			unsigned int fr:1;
			unsigned int to:4;
			unsigned int unused:24;
#else
			unsigned int unused:24;
			unsigned int to:4;
			unsigned int fr:1;
			unsigned int r:1;
			unsigned int s:1;
			unsigned int v6:1;
#endif

#ifndef CONFIG_SWITCH_BIG_ENDIAN
			unsigned int sv:1;
			unsigned int stag_vid:12;
			unsigned int cv:1;
			unsigned int ctag_vid:12;
			unsigned int unused_1:6;
#else
			unsigned int unused_1:6;
			unsigned int ctag_vid:12;
			unsigned int cv:1;
			unsigned int stag_vid:12;
			unsigned int sv:1;
#endif
			unsigned int ip[4];
			unsigned char mac[6];
		} sppe_arp;

		SPPE_ARL sppe_arl;

		struct {
			unsigned int sid;
			unsigned int index;
		} sppe_pppoe_sid;

		SPPE_FLOW_BRIDGE_IPV4		flow_bridge_ipv4;
		SPPE_FLOW_BRIDGE_IPV6		flow_bridge_ipv6;
		SPPE_FLOW_ROUTE_IPV4		flow_route_ipv4;
		SPPE_FLOW_ROUTE_IPV6		flow_route_ipv6;
		SPPE_FLOW_NAT_IPV4			flow_nat_ipv4;
		SPPE_FLOW_NAT_IPV6			flow_nat_ipv6;
		SPPE_FLOW_TWICE_NAT		 	flow_twice_nat;
		SPPE_FLOW_MCAST_IPV4		flow_mcast_ipv4;
		SPPE_FLOW_MCAST_IPV6		flow_mcast_ipv6;
		SPPE_FLOW_BRIDGE_L2			flow_bridge_l2;

		struct {
			SPPE_DUMP_TYPE type;
			unsigned short key;
			unsigned short way;
			unsigned int raw[23];
		} sppe_dump;
		
		unsigned int sppe_sna_th;

		struct {
#ifndef CONFIG_SWITCH_BIG_ENDIAN
			unsigned int enable:1;
			unsigned int lan:6;
			unsigned int wan:6;
			unsigned int reserved:19;
#else
			unsigned int reserved:19;
			unsigned int wan:6;
			unsigned int lan:6;
			unsigned int enable:1;
#endif
		} sppe_chgdscp;

		struct {
#ifndef CONFIG_SWITCH_BIG_ENDIAN
			unsigned int enable:1;
			unsigned int lan:3;
			unsigned int wan:3;
			unsigned int reserved:25;
#else
			unsigned int reserved:25;
			unsigned int wan:3;
			unsigned int lan:3;
			unsigned int enable:1;
#endif
		} sppe_chgpri;

		struct {
			int enable;
			int module;
			int level;
		} sppe_debug;

		struct {
			unsigned int offset;
			unsigned int data;
		} sppe_reg;

		struct {
			unsigned int offset;
			unsigned int data;
		} sppe_sram;

		struct {
			char enable;
			unsigned int max;	
			unsigned int min;
			char drop_red;
			char pass_green;
		} sppe_rl_flow;

		struct {
			char enable;
			unsigned int max;	
			unsigned int min;
			char drop_red;
			char pass_green;
		} sppe_rl_rule;

		struct {
			unsigned int index;
			unsigned short start;
			unsigned short end;
			SPPE_LIMIT limit;
		} sppe_bm_flow;

		struct {
			unsigned short used; 
			unsigned short fw; 
			unsigned short fp; 
			unsigned short pri; 
			unsigned short rl; 
			unsigned short protocol; 
			unsigned short cmd;
			unsigned short user_record;
			unsigned short kernel_record;
			unsigned short sport; 
			unsigned short dport; 
			unsigned short nport; 
			unsigned int sip[4];
			unsigned int dip[4];
			unsigned int nip[4];
			SPPE_LIMIT limit;
		} sppe_bm_flow_ex;

		struct {
			unsigned short used;
			unsigned short index; 
			unsigned short fp; 
			unsigned short pri; 
			unsigned short rl; 
			unsigned short match_method;
			unsigned short sport; 
			unsigned short dport; 
			unsigned int sip[4];
			unsigned int dip[4];
			SPPE_LIMIT limit;
		} sppe_bm_one_match;

		struct {	
			unsigned int index;
			unsigned int pkt_cnt;
			unsigned int byte_cnt;
		} sppe_accounting_group;

		struct {
			unsigned int pkt_cnt;
		} sppe_drop_ipcs_err; /* IP checksum error */

		struct {
			unsigned int pkt_cnt;
		} sppe_drop_rate_limit; 

		struct {
			unsigned int pkt_cnt;
		} sppe_drop_others;

		struct {
			unsigned int index;
			unsigned char name[16];
			struct net_device *dev;
			unsigned int vid;
		} sppe_pci_fp_dev;

		SPPE_INIT sppe_init;

	} data;
	
	int private_data;
} SPPE_PARAM;

extern int sppe_hook_ready;
extern int (*sppe_func_hook)(SPPE_PARAM *param);

extern int sppe_pci_fp_ready;
extern int (*sppe_pci_fp_hook)(SPPE_PARAM *param);

extern int sppe_hook_mode;

#endif /* CONFIG_CNS3XXX_SPPE */

#endif /* _SPPE_H_ */
