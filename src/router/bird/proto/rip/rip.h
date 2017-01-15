/*
 *	BIRD -- Routing Information Protocol (RIP)
 *
 *	(c) 1998--1999 Pavel Machek <pavel@ucw.cz>
 *	(c) 2004--2013 Ondrej Filip <feela@network.cz>
 *	(c) 2009--2015 Ondrej Zajicek <santiago@crfreenet.org>
 *	(c) 2009--2015 CZ.NIC z.s.p.o.
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_RIP_H_
#define _BIRD_RIP_H_

#include "nest/bird.h"
#include "nest/cli.h"
#include "nest/iface.h"
#include "nest/protocol.h"
#include "nest/route.h"
#include "nest/password.h"
#include "nest/locks.h"
#include "nest/bfd.h"
#include "lib/lists.h"
#include "lib/resource.h"
#include "lib/socket.h"
#include "lib/string.h"
#include "lib/timer.h"


#ifdef IPV6
#define RIP_IS_V2 0
#else
#define RIP_IS_V2 1
#endif

#define RIP_V1			1
#define RIP_V2			2

#define RIP_PORT		520	/* RIP for IPv4 */
#define RIP_NG_PORT		521	/* RIPng */

#define RIP_MAX_PKT_LENGTH	532	/* 512 + IP4_HEADER_LENGTH */
#define RIP_AUTH_TAIL_LENGTH	4	/* Without auth_data */

#define RIP_DEFAULT_ECMP_LIMIT	16
#define RIP_DEFAULT_INFINITY	16
#define RIP_DEFAULT_UPDATE_TIME	30
#define RIP_DEFAULT_TIMEOUT_TIME 180
#define RIP_DEFAULT_GARBAGE_TIME 120


struct rip_config
{
  struct proto_config c;
  list patt_list;			/* List of iface configs (struct rip_iface_config) */

  u8 rip2;				/* RIPv2 (IPv4) or RIPng (IPv6) */
  u8 ecmp;				/* Maximum number of nexthops in ECMP route, or 0 */
  u8 infinity;				/* Maximum metric value, representing infinity */

  u32 min_timeout_time;			/* Minimum of interface timeout_time */
  u32 max_garbage_time;			/* Maximum of interface garbage_time */
};

struct rip_iface_config
{
  struct iface_patt i;
  ip_addr address;			/* Configured dst address */
  u16 port;				/* Src+dst port */
  u8 metric;				/* Incoming metric */
  u8 mode;				/* Interface mode (RIP_IM_*) */
  u8 passive;				/* Passive iface - no packets are sent */
  u8 version;				/* RIP version used for outgoing packets */
  u8 version_only;	/* FIXXX */
  u8 split_horizon;			/* Split horizon is used in route updates */
  u8 poison_reverse;			/* Poisoned reverse is used in route updates */
  u8 check_zero;			/* Validation of RIPv1 reserved fields */
  u8 ecmp_weight;			/* Weight for ECMP routes*/
  u8 auth_type;				/* Authentication type (RIP_AUTH_*) */
  u8 ttl_security;			/* bool + 2 for TX only (send, but do not check on RX) */
  u8 check_link;			/* Whether iface link change is used */
  u8 bfd;				/* Use BFD on iface */
  u16 rx_buffer;			/* RX buffer size, 0 for MTU */
  u16 tx_length;			/* TX packet length limit (including headers), 0 for MTU */
  int tx_tos;
  int tx_priority;
  u32 update_time;			/* Periodic update interval */
  u32 timeout_time;			/* Route expiration timeout */
  u32 garbage_time;			/* Unreachable entry GC timeout */
  list *passwords;			/* Passwords for authentication */
};

struct rip_proto
{
  struct proto p;
  struct fib rtable;			/* Internal routing table */
  list iface_list;			/* List of interfaces (struct rip_iface) */
  slab *rte_slab;			/* Slab for internal routes (struct rip_rte) */
  timer *timer;				/* Main protocol timer */

  u8 ecmp;				/* Maximum number of nexthops in ECMP route, or 0 */
  u8 infinity;				/* Maximum metric value, representing infinity */
  u8 triggered;				/* Logical AND of interface want_triggered values */
  u8 rt_reload;				/* Route reload is scheduled */

  struct tbf log_pkt_tbf;		/* TBF for packet messages */
  struct tbf log_rte_tbf;		/* TBF for RTE messages */
};

struct rip_iface
{
  node n;
  struct rip_proto *rip;
  struct iface *iface;			/* Underyling core interface */
  struct rip_iface_config *cf;		/* Related config, must be updated in reconfigure */
  struct object_lock *lock;		/* Interface lock */
  timer *timer;				/* Interface timer */
  sock *sk;				/* UDP socket */

  u8 up;				/* Interface is active */
  u8 csn_ready;				/* Nonzero CSN can be used */
  u16 tx_plen;				/* Max TX packet data length */
  u32 csn;				/* Last used crypto sequence number */
  ip_addr addr;				/* Destination multicast/broadcast address */
  list neigh_list;			/* List of iface neighbors (struct rip_neighbor) */

  /* Update scheduling */
  bird_clock_t next_regular;		/* Next time when regular update should be called */
  bird_clock_t next_triggered;		/* Next time when triggerd update may be called */
  bird_clock_t want_triggered;		/* Nonzero if triggered update is scheduled */

  /* Active update */
  int tx_active;			/* Update session is active */
  ip_addr tx_addr;			/* Update session destination address */
  bird_clock_t tx_changed;		/* Minimal changed time for triggered update */
  struct fib_iterator tx_fit;		/* FIB iterator in RIP routing table (p.rtable) */
};

struct rip_neighbor
{
  node n;
  struct rip_iface *ifa;		/* Associated interface, may be NULL if stale */
  struct neighbor *nbr;			/* Associaded core neighbor, may be NULL if stale */
  struct bfd_request *bfd_req;		/* BFD request, if BFD is used */
  bird_clock_t last_seen;		/* Time of last received and accepted message */
  u32 uc;				/* Use count, number of routes linking the neighbor */
  u32 csn;				/* Last received crypto sequence number */
};

struct rip_entry
{
  struct fib_node n;
  struct rip_rte *routes;		/* List of incoming routes */

  u8 valid;				/* Entry validity state (RIP_ENTRY_*) */
  u8 metric;				/* Outgoing route metric */
  u16 tag;				/* Outgoing route tag */
  struct iface *from;			/* Outgoing route from, NULL if from  proto */
  struct iface *iface;			/* Outgoing route iface (for next hop) */
  ip_addr next_hop;			/* Outgoing route next hop */

  bird_clock_t changed;			/* Last time when the outgoing route metric changed */
};

struct rip_rte
{
  struct rip_rte *next;

  struct rip_neighbor *from;		/* Advertising router */
  ip_addr next_hop;			/* Route next hop (iface is from->nbr->iface) */
  u16 metric;				/* Route metric (after increase) */
  u16 tag;				/* Route tag */

  bird_clock_t expires;			/* Time of route expiration */
};


#define RIP_AUTH_NONE		0
#define RIP_AUTH_PLAIN		2
#define RIP_AUTH_CRYPTO		3

#define RIP_IM_MULTICAST	1
#define RIP_IM_BROADCAST	2

#define RIP_ENTRY_DUMMY		0	/* Only used to store list of incoming routes */
#define RIP_ENTRY_VALID		1	/* Valid outgoing route */
#define RIP_ENTRY_STALE		2	/* Stale outgoing route, waiting for GC */

#define EA_RIP_METRIC		EA_CODE(EAP_RIP, 0)
#define EA_RIP_TAG		EA_CODE(EAP_RIP, 1)

#define rip_is_v2(X) RIP_IS_V2
#define rip_is_ng(X) (!RIP_IS_V2)

/*
static inline int rip_is_v2(struct rip_proto *p)
{ return p->rip2; }

static inline int rip_is_ng(struct rip_proto *p)
{ return ! p->rip2; }
*/

static inline void
rip_reset_tx_session(struct rip_proto *p, struct rip_iface *ifa)
{
  if (ifa->tx_active)
  {
    FIB_ITERATE_UNLINK(&ifa->tx_fit, &p->rtable);
    ifa->tx_active = 0;
  }
}

/* rip.c */
void rip_update_rte(struct rip_proto *p, ip_addr *prefix, int pxlen, struct rip_rte *new);
void rip_withdraw_rte(struct rip_proto *p, ip_addr *prefix, int pxlen, struct rip_neighbor *from);
struct rip_neighbor * rip_get_neighbor(struct rip_proto *p, ip_addr *a, struct rip_iface *ifa);
void rip_update_bfd(struct rip_proto *p, struct rip_neighbor *n);
void rip_show_interfaces(struct proto *P, char *iff);
void rip_show_neighbors(struct proto *P, char *iff);

/* packets.c */
void rip_send_request(struct rip_proto *p, struct rip_iface *ifa);
void rip_send_table(struct rip_proto *p, struct rip_iface *ifa, ip_addr addr, bird_clock_t changed);
int rip_open_socket(struct rip_iface *ifa);


#endif
