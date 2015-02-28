/*
 * Structures for RIP protocol
 *
   FIXME: in V6, they insert additional entry whenever next hop differs. Such entry is identified by 0xff in metric.
 */

#include "nest/route.h"
#include "nest/password.h"
#include "nest/locks.h"

#define EA_RIP_TAG	EA_CODE(EAP_RIP, 0)
#define EA_RIP_METRIC	EA_CODE(EAP_RIP, 1)

#define PACKET_MAX	25
#define PACKET_MD5_MAX	18	/* FIXME */


#define RIP_V1		1
#define RIP_V2		2
#define RIP_NG		1	/* A new version numbering */

#ifndef IPV6
#define RIP_PORT	520	/* RIP for IPv4 */
#else
#define RIP_PORT	521	/* RIPng */
#endif

struct rip_connection {
  node n;

  int num;
  struct proto *proto;
  ip_addr addr;
  sock *send;
  struct rip_interface *rif;
  struct fib_iterator iter;

  ip_addr daddr;
  int dport;
  int done;
};

struct rip_packet_heading {		/* 4 bytes */
  u8 command;
#define RIPCMD_REQUEST          1       /* want info */
#define RIPCMD_RESPONSE         2       /* responding to request */
#define RIPCMD_TRACEON          3       /* turn tracing on */
#define RIPCMD_TRACEOFF         4       /* turn it off */
#define RIPCMD_MAX              5
  u8 version;
#define RIP_V1			1
#define RIP_V2			2
#define RIP_NG 			1	/* this is verion 1 of RIPng */
  u16 unused;
};

#ifndef IPV6
struct rip_block {	/* 20 bytes */
  u16 family;	/* 0xffff on first message means this is authentication */
  u16 tag;
  ip_addr network;
  ip_addr netmask;
  ip_addr nexthop;
  u32 metric;
};
#else
struct rip_block { /* IPv6 version!, 20 bytes, too */
  ip_addr network;
  u16 tag;
  u8 pxlen;
  u8 metric;
};
#endif

struct rip_block_auth { /* 20 bytes */
  u16 mustbeFFFF;
  u16 authtype;
  u16 packetlen;
  u8 keyid;
  u8 authlen;
  u32 seq;
  u32 zero0;
  u32 zero1;
};

struct rip_md5_tail {	/* 20 bytes */
  u16 mustbeFFFF;
  u16 mustbe0001;
  char md5[16];
};

struct rip_entry {
  struct fib_node n;

  ip_addr whotoldme;
  ip_addr nexthop;
  int metric;
  u16 tag;

  bird_clock_t updated, changed;
  int flags;
};

struct rip_packet {
  struct rip_packet_heading heading;
  struct rip_block block[PACKET_MAX];
};

struct rip_interface {
  node n;
  struct proto *proto;
  struct iface *iface;
  sock *sock;
  struct rip_connection *busy;
  int metric;			/* You don't want to put struct rip_patt *patt here -- think about reconfigure */
  int mode;
  int check_ttl;		/* Check incoming packets for TTL 255 */
  int triggered;
  struct object_lock *lock;
  int multicast;
};

struct rip_patt {
  struct iface_patt i;

  int metric;		/* If you add entries here, don't forget to modify patt_compare! */
  int mode;
#define IM_BROADCAST 2
#define IM_QUIET 4
#define IM_NOLISTEN 8
#define IM_VERSION1 16
  int tx_tos;
  int tx_priority;
  int ttl_security;	/* bool + 2 for TX only (send, but do not check on RX) */
};

struct rip_proto_config {
  struct proto_config c;
  list iface_list;	/* Patterns configured -- keep it first; see rip_reconfigure why */
  list *passwords;	/* Passwords, keep second */

  int infinity;		/* User configurable data; must be comparable with memcmp */
  int port;
  int period;
  int garbage_time;
  int timeout_time;

  int authtype;
#define AT_NONE 0
#define AT_PLAINTEXT 2
#define AT_MD5 3
  int honor;
#define HO_NEVER 0
#define HO_NEIGHBOR 1
#define HO_ALWAYS 2
};

struct rip_proto {
  struct proto inherited;
  timer *timer;
  list connections;
  struct fib rtable;
  list garbage;
  list interfaces;	/* Interfaces we really know about */
#ifdef LOCAL_DEBUG
  int magic;
#endif
  int tx_count;		/* Do one regular update once in a while */
  int rnd_count;	/* Randomize sending time */
};

#ifdef LOCAL_DEBUG
#define RIP_MAGIC 81861253
#define CHK_MAGIC do { if (P->magic != RIP_MAGIC) bug( "Not enough magic" ); } while (0)
#else
#define CHK_MAGIC do { } while (0)
#endif


void rip_init_config(struct rip_proto_config *c);

/* Authentication functions */

int rip_incoming_authentication( struct proto *p, struct rip_block_auth *block, struct rip_packet *packet, int num, ip_addr whotoldme );
int rip_outgoing_authentication( struct proto *p, struct rip_block_auth *block, struct rip_packet *packet, int num );
