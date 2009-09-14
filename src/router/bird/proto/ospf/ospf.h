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
#define OSPF_VLINK_MTU 576	/* RFC2328 - A.1 */
#define OSPF_MAX_PKT_SIZE 65536
			/*
                         * RFC 2328 says, maximum packet size is 65535
			 * This could be too much for small systems, so I
			 * normally allocate 2*mtu - (I found one cisco
			 * sending packets mtu+16)
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
#define OSPF_VERSION 2
#define AllSPFRouters ipa_from_u32(0xe0000005)	/* 224.0.0.5 */
#define AllDRouters ipa_from_u32(0xe0000006)	/* 224.0.0.6 */
#define DEFAULTDES ipa_from_u32(0)
#else
#error OSPF for IPv6 is not implemented (mail to Feela <feela@network.cz>)
#endif


#define LSREFRESHTIME 1800	/* 30 minutes */
#define MINLSINTERVAL 5
#define MINLSARRIVAL 1
#define LSINFINITY 0xffffff

#define DEFAULT_OSPFTICK 1
#define DEFAULT_RFC1583 0	/* compatibility with rfc1583 */
#define DEFAULT_STUB_COST 1000


struct ospf_config
{
  struct proto_config c;
  unsigned tick;
  int rfc1583;
  list area_list;
};

struct nbma_node
{
  node n;
  ip_addr ip;
  int eligible;
};

struct area_net_config
{
  node n;
  struct prefix px;
  int hidden;
};

struct area_net
{
  struct fib_node fn;
  int hidden;
  int active;
  u32 metric;
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
  int stub;
  list patt_list;
  list vlink_list;
  list net_list;
  list stubnet_list;
};

struct obits
{
#ifdef CPU_BIG_ENDIAN
  u8 unused2:2;
  u8 dc:1;
  u8 ea:1;
  u8 np:1;
  u8 mc:1;
  u8 e:1;
  u8 unused1:1;
#else
  u8 unused1:1;
  u8 e:1;
  u8 mc:1;
  u8 np:1;
  u8 ea:1;
  u8 dc:1;
  u8 unused2:2;
#endif
};

union options
{
  u8 byte;
  struct obits bit;
};


struct ospf_iface
{
  node n;
  struct iface *iface;		/* Nest's iface */
  struct ospf_area *oa;
  struct object_lock *lock;
  sock *hello_sk;		/* Hello socket */
  sock *dr_sk;			/* For states DR or BACKUP */
  sock *ip_sk;			/* IP socket (for DD ...) */
  list neigh_list;		/* List of neigbours */
  u32 cost;			/* Cost of iface */
  u32 waitint;			/* number of sec before changing state from wait */
  u32 rxmtint;			/* number of seconds between LSA retransmissions */
  u32 pollint;			/* Poll interval */
  u32 dead;			/* after "deadint" missing hellos is router dead */
  u32 vid;			/* Id of peer of virtual link */
  ip_addr vip;			/* IP of peer of virtual link */
  struct ospf_area *voa;	/* Area wich the vlink goes through */
  u16 inftransdelay;		/* The estimated number of seconds it takes to
				   transmit a Link State Update Packet over this
				   interface.  LSAs contained in the update */
  u16 autype;
  u16 helloint;			/* number of seconds between hello sending */
  list *passwords;
  u32 csn;                      /* Last used crypt seq number */
  bird_clock_t csn_use;         /* Last time when packet with that CSN was sent */
  ip_addr drip;			/* Designated router */
  u32 drid;
  ip_addr bdrip;		/* Backup DR */
  u32 bdrid;
  u8 type;			/* OSPF view of type */
#define OSPF_IT_BCAST 0
#define OSPF_IT_NBMA 1
#define OSPF_IT_PTP 2
#define OSPF_IT_VLINK 3
#define OSPF_IT_UNDEF 4
  u8 strictnbma;		/* Can I talk with unknown neighbors? */
  u8 stub;			/* Inactive interface */
#define OSPF_I_OK 0		/* Everything OK */
#define OSPF_I_MC 1		/* I didn't open MC socket */
#define OSPF_I_IP 2		/* I didn't open IP socet */
  u8 state;			/* Interface state machine */
#define OSPF_IS_DOWN 0		/* Not working */
#define OSPF_IS_LOOP 1		/* Should never happen */
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
#define WAIT_DMH 4		/* Value of Wait timer - not found it in RFC 
				 * - using 4*HELLO
				 */
  struct top_hash_entry *nlsa;	/* Originated net lsa */
  int orignet;			/* Schedule network LSA origination */
  int fadj;			/* Number of full adjacent neigh */
  list nbma_list;
  u8 priority;			/* A router priority for DR election */
  u8 ioprob;
  u32 rxbuf;
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

struct ospf_packet
{
  u8 version;
  u8 type;
#define HELLO_P 1		/* Hello */
#define DBDES_P 2		/* Database description */
#define LSREQ_P 3		/* Link state request */
#define LSUPD_P 4		/* Link state update */
#define LSACK_P 5		/* Link state acknowledgement */
  u16 length;
  u32 routerid;
  u32 areaid;
#define BACKBONE 0
  u16 checksum;
  u16 autype;
  union ospf_auth u;
};

struct ospf_hello_packet
{
  struct ospf_packet ospf_packet;
  ip_addr netmask;
  u16 helloint;
  u8 options;
  u8 priority;
  u32 deadint;
  ip_addr dr;
  ip_addr bdr;
};

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

struct ospf_dbdes_packet
{
  struct ospf_packet ospf_packet;
  u16 iface_mtu;
  u8 options;
  union imms imms;		/* I, M, MS bits */
#define DBDES_MS 1
#define DBDES_M 2
#define DBDES_I 4
  u32 ddseq;
};


struct ospf_lsa_header
{
  u16 age;			/* LS Age */
#define LSA_MAXAGE 3600		/* 1 hour */
#define LSA_CHECKAGE 300	/* 5 minutes */
#define LSA_MAXAGEDIFF 900	/* 15 minutes */
  u8 options;
  u8 type;
  u32 id;
#define LSA_T_RT 1
#define LSA_T_NET 2
#define LSA_T_SUM_NET 3
#define LSA_T_SUM_RT 4
#define LSA_T_EXT 5
  u32 rt;			/* Advertising router */
  s32 sn;			/* LS Sequence number */
#define LSA_INITSEQNO 0x80000001
#define LSA_MAXSEQNO 0x7fffffff
  u16 checksum;
  u16 length;
};

struct vebb
{
#ifdef CPU_BIG_ENDIAN
  u8 padding:5;
  u8 v:1;
  u8 e:1;
  u8 b:1;
#else
  u8 b:1;
  u8 e:1;
  u8 v:1;
  u8 padding:5;
#endif
};

union veb
{
  u8 byte;
  struct vebb bit;
};

struct ospf_lsa_rt
{
  union veb veb;
  u8 padding;
  u16 links;
};

struct ospf_lsa_rt_link
{
  u32 id;
  u32 data;
  u8 type;
#define LSART_PTP 1
#define LSART_NET 2
#define LSART_STUB 3
#define LSART_VLNK 4
  u8 notos;
  u16 metric;
};

struct ospf_lsa_rt_link_tos
{				/* Actually we ignore TOS. This is useless */
  u8 tos;
  u8 padding;
  u16 metric;
};

struct ospf_lsa_net
{
  ip_addr netmask;
};

struct ospf_lsa_sum
{
  ip_addr netmask;
};


struct ospf_lsa_ext
{
  ip_addr netmask;
};

struct ospf_lsa_ext_etos 
{
#ifdef CPU_BIG_ENDIAN
  u8 ebit:1;
  u8 tos:7;
  u8 padding1;
  u16 padding2;
#else
  u16 padding2;
  u8 padding1;
  u8 tos:7;
  u8 ebit:1;
#endif
};

#define METRIC_MASK 0x00FFFFFF
struct ospf_lsa_sum_tos 
{
#ifdef CPU_BIG_ENDIAN
  u8 tos;
  u8 padding1;
  u16 padding2;
#else
  u16 padding2;
  u8 padding1;
  u8 tos;
#endif
};

union ospf_lsa_sum_tm
{
  struct ospf_lsa_sum_tos tos;
  u32 metric;
};

union ospf_lsa_ext_etm
{
  struct ospf_lsa_ext_etos etos;
  u32 metric;
};

struct ospf_lsa_ext_tos
{
  union ospf_lsa_ext_etm etm;
  ip_addr fwaddr;
  u32 tag;
};

struct ospf_lsreq_packet
{
  struct ospf_packet ospf_packet;
};

struct ospf_lsreq_header
{
  u16 padd1;
  u8 padd2;
  u8 type;
  u32 id;
  u32 rt;			/* Advertising router */
};

struct l_lsr_head
{
  node n;
  struct ospf_lsreq_header lsh;
};

struct ospf_lsupd_packet
{
  struct ospf_packet ospf_packet;
  u32 lsano;			/* Number of LSA's */
};

struct ospf_lsack_packet
{
  struct ospf_packet ospf_packet;
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
  u8 options;			/* Options received */
  ip_addr dr;			/* Neigbour's idea of DR */
  ip_addr bdr;			/* Neigbour's idea of BDR */
  u8 adj;			/* built adjacency? */
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
#define ISM_LOOP 4		/* Loop indicated */
#define ISM_UNLOOP 5		/* Unloop indicated */
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
  int origrt;			/* Rt lsa origination scheduled? */
  struct top_hash_entry *rt;	/* My own router LSA */
  list cand;			/* List of candidates for RT calc. */
  struct fib net_fib;		/* Networks to advertise or not */
  int stub;
  int trcap;			/* Transit capability? */
  struct proto_ospf *po;
  struct fib rtr;		/* Routing tables for routers */
  union options opt;            /* RFC2328 - A.2 */
};

struct proto_ospf
{
  struct proto proto;
  timer *disp_timer;		/* OSPF proto dispatcher */
  unsigned tick;
  struct top_graph *gr;		/* LSA graph */
  slist lsal;			/* List of all LSA's */
  int calcrt;			/* Routing table calculation scheduled? */
  int cleanup;                  /* Should I cleanup after RT calculation? */
  list iface_list;		/* Interfaces we really use */
  list area_list;
  int areano;			/* Number of area I belong to */
  struct fib rtf;		/* Routing table */
  int rfc1583;			/* RFC1583 compatibility */
  int ebit;			/* Did I originate any ext lsa? */
  struct ospf_area *backbone;	/* If exists */
  void *lsab;			/* LSA buffer used when originating router LSAs */
  int lsab_size, lsab_used;
};

struct ospf_iface_patt
{
  struct iface_patt i;
  u32 cost;
  u32 helloint;
  u32 rxmtint;
  u32 pollint;
  u32 inftransdelay;
  u32 priority;
  u32 waitint;
  u32 deadc;
  u32 dead;
  u32 type;
  u32 autype;
  u32 strictnbma;
  u32 stub;
  u32 vid;
#define OSPF_AUTH_NONE 0
#define OSPF_AUTH_SIMPLE 1
#define OSPF_AUTH_CRYPT 2
#define OSPF_AUTH_CRYPT_SIZE 16
  u32 rxbuf;
#define OSPF_RXBUF_NORMAL 0
#define OSPF_RXBUF_LARGE 1
#define OSPF_RXBUF_MINSIZE 256	/* Minimal allowed size */
  list *passwords;
  list nbma_list;
};

int ospf_import_control(struct proto *p, rte **new, ea_list **attrs,
			struct linpool *pool);
struct ea_list *ospf_make_tmp_attrs(struct rte *rt, struct linpool *pool);
void ospf_store_tmp_attrs(struct rte *rt, struct ea_list *attrs);
void schedule_rt_lsa(struct ospf_area *oa);
void schedule_rtcalc(struct proto_ospf *po);
void schedule_net_lsa(struct ospf_iface *ifa);
void ospf_sh_neigh(struct proto *p, char *iff);
void ospf_sh(struct proto *p);
void ospf_sh_iface(struct proto *p, char *iff);
void ospf_sh_state(struct proto *p, int verbose);


#define EA_OSPF_METRIC1	EA_CODE(EAP_OSPF, 0)
#define EA_OSPF_METRIC2	EA_CODE(EAP_OSPF, 1)
#define EA_OSPF_TAG	EA_CODE(EAP_OSPF, 2)

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
