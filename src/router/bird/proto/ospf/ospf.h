/*
 *	BIRD -- OSPF
 *
 *	(c) 1999--2005 Ondrej Filip <feela@network.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_OSPF_H_
#define _BIRD_OSPF_H_

#define MAXNETS 10
#define OSPF_MAX_PKT_SIZE 65535
/*
 * RFC 2328 says, maximum packet size is 65535 (IP packet size
 * limit). Really a bit less for OSPF, because this contains also IP
 * header. This could be too much for small systems, so I normally
 * allocate 2*mtu (i found one cisco sending packets mtu+16). OSPF
 * packets are almost always sent small enough to not be fragmented.
 */

#ifdef LOCAL_DEBUG
#define OSPF_FORCE_DEBUG 1
#else
#define OSPF_FORCE_DEBUG 0
#endif
#define OSPF_TRACE(flags, msg, args...) do { if ((p->debug & flags) || OSPF_FORCE_DEBUG) \
  log(L_TRACE "%s: " msg, p->name , ## args ); } while(0)

#define OSPF_PACKET(dumpfn, buffer, msg, args...) \
do { if ((p->debug & D_PACKETS) || OSPF_FORCE_DEBUG) \
{ log(L_TRACE "%s: " msg, p->name, ## args ); dumpfn(p, buffer); } } while(0)


#include "nest/bird.h"

#include "lib/checksum.h"
#include "lib/ip.h"
#include "lib/lists.h"
#include "lib/slists.h"
#include "lib/socket.h"
#include "lib/timer.h"
#include "lib/resource.h"
#include "nest/protocol.h"
#include "nest/iface.h"
#include "nest/route.h"
#include "nest/cli.h"
#include "nest/locks.h"
#include "conf/conf.h"
#include "lib/string.h"

#define OSPF_PROTO 89

#ifndef IPV6
#define OSPFv2 1
#define OSPF_VERSION 2
#define OSPF_VLINK_MTU 576			/* RFC 2328 A.1 */
#define AllSPFRouters ipa_from_u32(0xe0000005)	/* 224.0.0.5 */
#define AllDRouters ipa_from_u32(0xe0000006)	/* 224.0.0.6 */
#else
#define OSPFv3 1
#define OSPF_VERSION 3
#define OSPF_VLINK_MTU 1280			/* RFC 5340 A.1 */
#define AllSPFRouters _MI(0xFF020000, 0, 0, 5)	/* FF02::5 */
#define AllDRouters   _MI(0xFF020000, 0, 0, 6)	/* FF02::6 */
#endif


#define LSREFRESHTIME 1800	/* 30 minutes */
#define MINLSINTERVAL 5
#define MINLSARRIVAL 1
#define LSINFINITY 0xffffff

#define DEFAULT_OSPFTICK 1
#define DEFAULT_RFC1583 0	/* compatibility with rfc1583 */
#define DEFAULT_STUB_COST 1000
#define DEFAULT_ECMP_LIMIT 16
#define DEFAULT_TRANSINT 40


struct ospf_config
{
  struct proto_config c;
  unsigned tick;
  byte rfc1583;
  byte stub_router;
  byte abr;
  int ecmp;
  list area_list;		/* list of struct ospf_area_config */
  list vlink_list;		/* list of struct ospf_iface_patt */
};

struct nbma_node
{
  node n;
  ip_addr ip;
  byte eligible;
  byte found; 
};

struct area_net_config
{
  node n;
  struct prefix px;
  int hidden;
  u32 tag;
};

struct area_net
{
  struct fib_node fn;
  int hidden;
  int active;
  u32 metric;
  u32 tag;
};

struct ospf_stubnet_config
{
  node n;
  struct prefix px;
  int hidden, summary;
  u32 cost;
};

struct ospf_area_config
{
  node n;
  u32 areaid;
  u32 default_cost;		/* Cost of default route for stub areas */
  u8 type;			/* Area type (standard, stub, NSSA), represented
				   by option flags (OPT_E, OPT_N) */
  u8 summary;			/* Import summaries to this stub/NSSA area, valid for ABR */
  u8 default_nssa;		/* Generate default NSSA route for NSSA+summary area */
  u8 translator;		/* Translator role, for NSSA ABR */
  u32 transint;			/* Translator stability interval */
  list patt_list;
  list net_list;	      	/* List of aggregate networks for that area */
  list enet_list;	      	/* List of aggregate external (NSSA) networks */
  list stubnet_list;		/* List of stub networks added to Router LSA */
};


/* Option flags */

#define OPT_E	0x02
#define OPT_N	0x08
#define OPT_DC	0x20

#ifdef OSPFv2
#define OPT_P	0x08		/* flags P and N share position, see NSSA RFC */
#define OPT_EA	0x10

/* VEB flags are are stored independently in 'u16 options' */
#define OPT_RT_B  (0x01 << 8)
#define OPT_RT_E  (0x02 << 8)
#define OPT_RT_V  (0x04 << 8)
#define OPT_RT_NT (0x10 << 8)
#endif

#ifdef OSPFv3
#define OPT_V6	0x01
#define OPT_R	0x10

/* VEB flags are are stored together with options in 'u32 options' */
#define OPT_RT_B  (0x01 << 24)
#define OPT_RT_E  (0x02 << 24)
#define OPT_RT_V  (0x04 << 24)
#define OPT_RT_NT (0x10 << 24)

#define OPT_PX_NU 0x01
#define OPT_PX_LA 0x02
#define OPT_PX_P  0x08
#define OPT_PX_DN 0x10
#endif


struct ospf_iface
{
  node n;
  struct iface *iface;		/* Nest's iface, non-NULL (unless type OSPF_IT_VLINK) */
  struct ifa *addr;		/* IP prefix associated with that OSPF iface */
  struct ospf_area *oa;
  struct ospf_iface_patt *cf;
  pool *pool;
  sock *sk;			/* IP socket (for DD ...) */
  list neigh_list;		/* List of neigbours */
  u32 cost;			/* Cost of iface */
  u32 waitint;			/* number of sec before changing state from wait */
  u32 rxmtint;			/* number of seconds between LSA retransmissions */
  u32 pollint;			/* Poll interval */
  u32 deadint;			/* after "deadint" missing hellos is router dead */
  u32 iface_id;			/* Interface ID (iface->index or new value for vlinks) */
  u32 vid;			/* ID of peer of virtual link */
  ip_addr vip;			/* IP of peer of virtual link */
  struct ospf_iface *vifa;	/* OSPF iface which the vlink goes through */
  struct ospf_area *voa;	/* OSPF area which the vlink goes through */
  u16 inftransdelay;		/* The estimated number of seconds it takes to
				   transmit a Link State Update Packet over this
				   interface.  LSAs contained in the update */
  u16 helloint;			/* number of seconds between hello sending */

#ifdef OSPFv2
  list *passwords;
  u16 autype;
  u32 csn;                      /* Last used crypt seq number */
  bird_clock_t csn_use;         /* Last time when packet with that CSN was sent */
#endif

  ip_addr all_routers;		/*  */
  ip_addr drip;			/* Designated router */
  ip_addr bdrip;		/* Backup DR */
  u32 drid;
  u32 bdrid;
  s16 rt_pos_beg;		/* Position of iface in Router-LSA, begin, inclusive */
  s16 rt_pos_end;		/* Position of iface in Router-LSA, end, exclusive */

#ifdef OSPFv3
  s16 px_pos_beg;		/* Position of iface in Rt Prefix-LSA, begin, inclusive */
  s16 px_pos_end;		/* Position of iface in Rt Prefix-LSA, end, exclusive */

  u32 dr_iface_id;		/* if drid is valid, this is iface_id of DR (for connecting network) */
  u8 instance_id;		/* Used to differentiate between more OSPF
				   instances on one interface */
#endif

  u8 type;			/* OSPF view of type */
#define OSPF_IT_BCAST 0
#define OSPF_IT_NBMA 1
#define OSPF_IT_PTP 2
#define OSPF_IT_PTMP 3
#define OSPF_IT_VLINK 4
#define OSPF_IT_UNDEF 5
  u8 strictnbma;		/* Can I talk with unknown neighbors? */
  u8 stub;			/* Inactive interface */
  u8 state;			/* Interface state machine */
#define OSPF_IS_DOWN 0		/* Not working */
#define OSPF_IS_LOOP 1		/* Iface with no link */
#define OSPF_IS_WAITING 2	/* Waiting for Wait timer */
#define OSPF_IS_PTP 3		/* PTP operational */
#define OSPF_IS_DROTHER 4	/* I'm on BCAST or NBMA and I'm not DR */
#define OSPF_IS_BACKUP 5	/* I'm BDR */
#define OSPF_IS_DR 6		/* I'm DR */
  timer *wait_timer;		/* WAIT timer */
  timer *hello_timer;		/* HELLOINT timer */
  timer *poll_timer;		/* Poll Interval - for NBMA */
/* Default values for interface parameters */
#define COST_D 10
#define RXMTINT_D 5
#define INFTRANSDELAY_D 1
#define PRIORITY_D 1
#define HELLOINT_D 10
#define POLLINT_D 20
#define DEADC_D 4
#define WAIT_DMH 4		
  /* Value of Wait timer - not found it in RFC * - using 4*HELLO */

  struct top_hash_entry *net_lsa;	/* Originated network LSA */
  int orignet;				/* Schedule network LSA origination */
#ifdef OSPFv3
  int origlink;				/* Schedule link LSA origination */
  struct top_hash_entry *link_lsa;	/* Originated link LSA */
  struct top_hash_entry *pxn_lsa;	/* Originated prefix LSA */
#endif
  int fadj;				/* Number of full adjacent neigh */
  list nbma_list;
  u8 priority;			/* A router priority for DR election */
  u8 ioprob;
#define OSPF_I_OK 0		/* Everything OK */
#define OSPF_I_SK 1		/* Socket open failed */
#define OSPF_I_LL 2		/* Missing link-local address (OSPFv3) */
  u8 sk_dr; 			/* Socket is a member of DRouters group */
  u8 marked;			/* Used in OSPF reconfigure */
  u16 rxbuf;			/* Buffer size */
  u8 check_link;		/* Whether iface link change is used */
  u8 ecmp_weight;		/* Weight used for ECMP */
  u8 ptp_netmask;		/* Send real netmask for P2P */
  u8 check_ttl;			/* Check incoming packets for TTL 255 */
};

struct ospf_md5
{
  u16 zero;
  u8 keyid;
  u8 len;
  u32 csn;
};

union ospf_auth
{
  u8 password[8];
  struct ospf_md5 md5;
};


/* Packet types */
#define HELLO_P 1		/* Hello */
#define DBDES_P 2		/* Database description */
#define LSREQ_P 3		/* Link state request */
#define LSUPD_P 4		/* Link state update */
#define LSACK_P 5		/* Link state acknowledgement */

/* Area IDs */
#define BACKBONE 0


struct immsb
{
#ifdef CPU_BIG_ENDIAN
  u8 padding:5;
  u8 i:1;
  u8 m:1;
  u8 ms:1;
#else
  u8 ms:1;
  u8 m:1;
  u8 i:1;
  u8 padding:5;
#endif
};

union imms
{
  u8 byte;
  struct immsb bit;
};
#define DBDES_MS 1
#define DBDES_M 2
#define DBDES_I 4


#ifdef OSPFv2

struct ospf_packet
{
  u8 version;
  u8 type;
  u16 length;
  u32 routerid;
  u32 areaid;
  u16 checksum;
  u16 autype;
  union ospf_auth u;
};


#else /* OSPFv3 packet descriptions */

struct ospf_packet
{
  u8 version;
  u8 type;
  u16 length;
  u32 routerid;
  u32 areaid;
  u16 checksum;
  u8 instance_id;
  u8 zero;
};


#endif




struct ospf_lsa_header
{
  u16 age;			/* LS Age */
#define LSA_MAXAGE 3600		/* 1 hour */
#define LSA_CHECKAGE 300	/* 5 minutes */
#define LSA_MAXAGEDIFF 900	/* 15 minutes */

#ifdef OSPFv2
  u8 options;
  u8 type;

#define LSA_T_RT	1
#define LSA_T_NET	2
#define LSA_T_SUM_NET	3
#define LSA_T_SUM_RT	4
#define LSA_T_EXT	5
#define LSA_T_NSSA	7

#define LSA_SCOPE_AREA	0x2000
#define LSA_SCOPE_AS	0x4000

#define LSA_SCOPE(lsa)	(((lsa)->type == LSA_T_EXT) ? LSA_SCOPE_AS : LSA_SCOPE_AREA)

#else /* OSPFv3 */
  u16 type;

#define LSA_T_RT	0x2001
#define LSA_T_NET	0x2002
#define LSA_T_SUM_NET	0x2003
#define LSA_T_SUM_RT	0x2004
#define LSA_T_EXT	0x4005
#define LSA_T_NSSA	0x2007
#define LSA_T_LINK	0x0008
#define LSA_T_PREFIX	0x2009

#define LSA_UBIT	0x8000

#define LSA_SCOPE_LINK	0x0000
#define LSA_SCOPE_AREA	0x2000
#define LSA_SCOPE_AS	0x4000
#define LSA_SCOPE_RES	0x6000
#define LSA_SCOPE_MASK	0x6000

#define LSA_SCOPE(lsa)	((lsa)->type & LSA_SCOPE_MASK)
#endif

  u32 id;
  u32 rt;			/* Advertising router */
  s32 sn;			/* LS Sequence number */
#define LSA_INITSEQNO ((s32) 0x80000001)
#define LSA_MAXSEQNO ((s32) 0x7fffffff)
  u16 checksum;
  u16 length;
};


#define LSART_PTP 1
#define LSART_NET 2
#define LSART_STUB 3
#define LSART_VLNK 4


#ifdef OSPFv2

struct ospf_lsa_rt
{
#ifdef CPU_BIG_ENDIAN
  u16 options;	/* VEB flags only */
  u16 links;
#else
  u16 links;
  u16 options;	/* VEB flags only */
#endif
};

struct ospf_lsa_rt_link
{
  u32 id;
  u32 data;
#ifdef CPU_BIG_ENDIAN
  u8 type;
  u8 padding;
  u16 metric;
#else
  u16 metric;
  u8 padding;
  u8 type;
#endif
};

struct ospf_lsa_net
{
  ip_addr netmask;
  u32 routers[];
};

struct ospf_lsa_sum
{
  ip_addr netmask;
  u32 metric;
};

struct ospf_lsa_ext
{
  ip_addr netmask;
  u32 metric;
  ip_addr fwaddr;
  u32 tag;
};

#define LSA_SUM_TOS  0xFF000000
#define LSA_EXT_TOS  0x7F000000
#define LSA_EXT_EBIT 0x80000000

/* Endianity swap for lsa->type */
#define ntoht(x) x
#define htont(x) x


#else  /* OSPFv3 */

struct ospf_lsa_rt
{
  u32 options;
};

struct ospf_lsa_rt_link
{
#ifdef CPU_BIG_ENDIAN
  u8 type;
  u8 padding;
  u16 metric;
#else
  u16 metric;
  u8 padding;
  u8 type;
#endif
  u32 lif;	/* Local interface ID */
  u32 nif;	/* Neighbor interface ID */
  u32 id;	/* Neighbor router ID */
};

struct ospf_lsa_net
{
  u32 options;
  u32 routers[];
};

struct ospf_lsa_sum_net
{
  u32 metric;
  u32 prefix[];
};

struct ospf_lsa_sum_rt
{
  u32 options;
  u32 metric;
  u32 drid;
};

struct ospf_lsa_ext
{
  u32 metric;
  u32 rest[];
};

struct ospf_lsa_link
{
  u32 options;
  ip_addr lladdr;
  u32 pxcount;
  u32 rest[];
};

struct ospf_lsa_prefix
{
#ifdef CPU_BIG_ENDIAN
  u16 pxcount;
  u16 ref_type;
#else
  u16 ref_type;
  u16 pxcount;
#endif
  u32 ref_id;
  u32 ref_rt;
  u32 rest[];
};

#define LSA_EXT_EBIT 0x4000000
#define LSA_EXT_FBIT 0x2000000
#define LSA_EXT_TBIT 0x1000000

/* Endianity swap for lsa->type */
#define ntoht(x) ntohs(x)
#define htont(x) htons(x)

#endif

#define METRIC_MASK  0x00FFFFFF
#define OPTIONS_MASK 0x00FFFFFF

static inline unsigned
lsa_rt_count(struct ospf_lsa_header *lsa)
{
  return (lsa->length - sizeof(struct ospf_lsa_header) - sizeof(struct ospf_lsa_rt))
    / sizeof(struct ospf_lsa_rt_link);
}

static inline unsigned
lsa_net_count(struct ospf_lsa_header *lsa)
{
  return (lsa->length - sizeof(struct ospf_lsa_header) - sizeof(struct ospf_lsa_net))
    / sizeof(u32);
}


#ifdef OSPFv3

#define IPV6_PREFIX_SPACE(x) ((((x) + 63) / 32) * 4)
#define IPV6_PREFIX_WORDS(x) (((x) + 63) / 32)

static inline u32 *
lsa_get_ipv6_prefix(u32 *buf, ip_addr *addr, int *pxlen, u8 *pxopts, u16 *rest)
{
  u8 pxl = (*buf >> 24);
  *pxopts = (*buf >> 16);
  *rest = *buf;
  *pxlen = pxl;
  buf++;

  *addr = IPA_NONE;

  if (pxl > 0)
    _I0(*addr) = *buf++;
  if (pxl > 32)
    _I1(*addr) = *buf++;
  if (pxl > 64)
    _I2(*addr) = *buf++;
  if (pxl > 96)
    _I3(*addr) = *buf++;

  /* Clean up remaining bits */
  if (pxl < 128)
    addr->addr[pxl / 32] &= u32_mkmask(pxl % 32);

  return buf;
}

static inline u32 *
lsa_get_ipv6_addr(u32 *buf, ip_addr *addr)
{
  *addr = *(ip_addr *) buf;
  return buf + 4;
}

static inline u32 *
put_ipv6_prefix(u32 *buf, ip_addr addr, u8 pxlen, u8 pxopts, u16 lh)
{
  *buf++ = ((pxlen << 24) | (pxopts << 16) | lh);

  if (pxlen > 0)
    *buf++ = _I0(addr);
  if (pxlen > 32)
    *buf++ = _I1(addr);
  if (pxlen > 64)
    *buf++ = _I2(addr);
  if (pxlen > 96)
    *buf++ = _I3(addr);
  return buf;
}

static inline u32 *
put_ipv6_addr(u32 *buf, ip_addr addr)
{
  *(ip_addr *) buf = addr;
  return buf + 4;
}

#endif



struct ospf_lsreq_header
{
  u32 type;
  u32 id;
  u32 rt;			/* Advertising router */
};

struct l_lsr_head
{
  node n;
  struct ospf_lsreq_header lsh;
};


struct ospf_neighbor
{
  node n;
  pool *pool;
  struct ospf_iface *ifa;
  u8 state;
#define NEIGHBOR_DOWN 0
#define NEIGHBOR_ATTEMPT 1
#define NEIGHBOR_INIT 2
#define NEIGHBOR_2WAY 3
#define NEIGHBOR_EXSTART 4
#define NEIGHBOR_EXCHANGE 5
#define NEIGHBOR_LOADING 6
#define NEIGHBOR_FULL 7
  timer *inactim;		/* Inactivity timer */
  union imms imms;		/* I, M, Master/slave received */
  u32 dds;			/* DD Sequence number being sent */
  u32 ddr;			/* last Dat Des packet received */
  union imms myimms;		/* I, M Master/slave */
  u32 rid;			/* Router ID */
  ip_addr ip;			/* IP of it's interface */
  u8 priority;			/* Priority */
  u8 adj;			/* built adjacency? */
  u32 options;			/* Options received */

  /* dr and bdr store IP address in OSPFv2 and router ID in OSPFv3,
     we use the same type to simplify handling */
  u32 dr;			/* Neigbour's idea of DR */
  u32 bdr;			/* Neigbour's idea of BDR */

#ifdef OSPFv3
  u32 iface_id;			/* ID of Neighbour's iface connected to common network */
#endif

  siterator dbsi;		/* Database summary list iterator */
  slist lsrql;			/* Link state request */
  struct top_graph *lsrqh;	/* LSA graph */
  siterator lsrqi;
  slist lsrtl;			/* Link state retransmission list */
  siterator lsrti;
  struct top_graph *lsrth;
  void *ldbdes;			/* Last database description packet */
  timer *rxmt_timer;		/* RXMT timer */
  list ackl[2];
#define ACKL_DIRECT 0
#define ACKL_DELAY 1
  timer *ackd_timer;		/* Delayed ack timer */
  u32 csn;                      /* Last received crypt seq number (for MD5) */
};

/* Definitions for interface state machine */
#define ISM_UP 0		/* Interface Up */
#define ISM_WAITF 1		/* Wait timer fired */
#define ISM_BACKS 2		/* Backup seen */
#define ISM_NEICH 3		/* Neighbor change */
#define ISM_LOOP 4		/* Link down */
#define ISM_UNLOOP 5		/* Link up */
#define ISM_DOWN 6		/* Interface down */

/* Definitions for neighbor state machine */
#define INM_HELLOREC 0		/* Hello Received */
#define INM_START 1		/* Neighbor start - for NBMA */
#define INM_2WAYREC 2		/* 2-Way received */
#define INM_NEGDONE 3		/* Negotiation done */
#define INM_EXDONE 4		/* Exchange done */
#define INM_BADLSREQ 5		/* Bad LS Request */
#define INM_LOADDONE 6		/* Load done */
#define INM_ADJOK 7		/* AdjOK? */
#define INM_SEQMIS 8		/* Sequence number mismatch */
#define INM_1WAYREC 9		/* 1-Way */
#define INM_KILLNBR 10		/* Kill Neighbor */
#define INM_INACTTIM 11		/* Inactivity timer */
#define INM_LLDOWN 12		/* Line down */

struct ospf_area
{
  node n;
  u32 areaid;
  struct ospf_area_config *ac;	/* Related area config */
  struct top_hash_entry *rt;	/* My own router LSA */
  struct top_hash_entry *pxr_lsa; /* Originated prefix LSA */
  list cand;			/* List of candidates for RT calc. */
  struct fib net_fib;		/* Networks to advertise or not */
  struct fib enet_fib;		/* External networks for NSSAs */
  u32 options;			/* Optional features */
  byte origrt;			/* Rt lsa origination scheduled? */
  byte trcap;			/* Transit capability? */
  byte marked;			/* Used in OSPF reconfigure */
  byte translate;		/* Translator state (TRANS_*), for NSSA ABR  */
  timer *translator_timer;	/* For NSSA translator switch */
  struct proto_ospf *po;
  struct fib rtr;		/* Routing tables for routers */
};

#define TRANS_OFF	0
#define TRANS_ON	1
#define TRANS_WAIT	2	/* Waiting before the end of translation */

struct proto_ospf
{
  struct proto proto;
  timer *disp_timer;		/* OSPF proto dispatcher */
  unsigned tick;
  struct top_graph *gr;		/* LSA graph */
  slist lsal;			/* List of all LSA's */
  int calcrt;			/* Routing table calculation scheduled?
				   0=no, 1=normal, 2=forced reload */
  list iface_list;		/* Interfaces we really use */
  list area_list;
  int areano;			/* Number of area I belong to */
  struct fib rtf;		/* Routing table */
  byte rfc1583;			/* RFC1583 compatibility */
  byte stub_router;		/* Do not forward transit traffic */
  byte ebit;			/* Did I originate any ext lsa? */
  byte ecmp;			/* Maximal number of nexthops in ECMP route, or 0 */
  struct ospf_area *backbone;	/* If exists */
  void *lsab;			/* LSA buffer used when originating router LSAs */
  int lsab_size, lsab_used;
  linpool *nhpool;		/* Linpool used for next hops computed in SPF */
  u32 router_id;
  u32 last_vlink_id;		/* Interface IDs for vlinks (starts at 0x80000000) */
};

struct ospf_iface_patt
{
  struct iface_patt i;
  u32 type;
  u32 stub;
  u32 cost;
  u32 helloint;
  u32 rxmtint;
  u32 pollint;
  u32 waitint;
  u32 deadc;
  u32 deadint;
  u32 inftransdelay;
  list nbma_list;
  u32 priority;
  u32 voa;
  u32 vid;
  int tx_tos;
  int tx_priority;
  u16 rxbuf;
#define OSPF_RXBUF_NORMAL 0
#define OSPF_RXBUF_LARGE 1
#define OSPF_RXBUF_MINSIZE 256	/* Minimal allowed size */
  u16 autype;			/* Not really used in OSPFv3 */
#define OSPF_AUTH_NONE 0
#define OSPF_AUTH_SIMPLE 1
#define OSPF_AUTH_CRYPT 2
#define OSPF_AUTH_CRYPT_SIZE 16
  u8 strictnbma;
  u8 check_link;
  u8 ecmp_weight;
  u8 real_bcast;		/* Not really used in OSPFv3 */
  u8 ptp_netmask;		/* bool + 2 for unspecified */
  u8 ttl_security;		/* bool + 2 for TX only */

#ifdef OSPFv2
  list *passwords;
#endif

#ifdef OSPFv3
  u8 instance_id;
#endif
};

int ospf_import_control(struct proto *p, rte **new, ea_list **attrs,
			struct linpool *pool);
struct ea_list *ospf_make_tmp_attrs(struct rte *rt, struct linpool *pool);
void ospf_store_tmp_attrs(struct rte *rt, struct ea_list *attrs);
void schedule_rt_lsa(struct ospf_area *oa);
void schedule_rtcalc(struct proto_ospf *po);
void schedule_net_lsa(struct ospf_iface *ifa);

struct ospf_area *ospf_find_area(struct proto_ospf *po, u32 aid);
static inline struct ospf_area *ospf_main_area(struct proto_ospf *po)
{ return (po->areano == 1) ? HEAD(po->area_list) : po->backbone; }

static inline int oa_is_stub(struct ospf_area *oa)
{ return (oa->options & (OPT_E | OPT_N)) == 0; }
static inline int oa_is_ext(struct ospf_area *oa)
{ return oa->options & OPT_E; }
static inline int oa_is_nssa(struct ospf_area *oa)
{ return oa->options & OPT_N; }


#ifdef OSPFv3
void schedule_link_lsa(struct ospf_iface *ifa);
#else
static inline void schedule_link_lsa(struct ospf_iface *ifa UNUSED) {}
#endif

void ospf_sh_neigh(struct proto *p, char *iff);
void ospf_sh(struct proto *p);
void ospf_sh_iface(struct proto *p, char *iff);
void ospf_sh_state(struct proto *p, int verbose, int reachable);

#define SH_ROUTER_SELF 0xffffffff

struct lsadb_show_data {
  struct symbol *name;	/* Protocol to request data from */
  u16 type;		/* LSA Type, 0 -> all */
  u16 scope;		/* Scope, 0 -> all, hack to handle link scope as 1 */
  u32 area;		/* Specified for area scope */
  u32 lsid;		/* LSA ID, 0 -> all */
  u32 router;		/* Advertising router, 0 -> all */
};

void ospf_sh_lsadb(struct lsadb_show_data *ld);


#define EA_OSPF_METRIC1	EA_CODE(EAP_OSPF, 0)
#define EA_OSPF_METRIC2	EA_CODE(EAP_OSPF, 1)
#define EA_OSPF_TAG	EA_CODE(EAP_OSPF, 2)
#define EA_OSPF_ROUTER_ID EA_CODE(EAP_OSPF, 3)

#include "proto/ospf/rt.h"
#include "proto/ospf/hello.h"
#include "proto/ospf/packet.h"
#include "proto/ospf/iface.h"
#include "proto/ospf/neighbor.h"
#include "proto/ospf/topology.h"
#include "proto/ospf/dbdes.h"
#include "proto/ospf/lsreq.h"
#include "proto/ospf/lsupd.h"
#include "proto/ospf/lsack.h"
#include "proto/ospf/lsalib.h"

#endif /* _BIRD_OSPF_H_ */
