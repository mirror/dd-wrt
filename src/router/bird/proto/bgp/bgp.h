/*
 *	BIRD -- The Border Gateway Protocol
 *
 *	(c) 2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_BGP_H_
#define _BIRD_BGP_H_

#include <stdint.h>
#include "nest/route.h"

struct linpool;
struct eattr;

struct bgp_config {
  struct proto_config c;
  u32 local_as, remote_as;
  ip_addr remote_ip;
  ip_addr source_addr;			/* Source address to use */
  struct iface *iface;			/* Interface for link-local addresses */
  int multihop;				/* Number of hops if multihop */
  int ttl_security;			/* Enable TTL security [RFC5082] */
  int next_hop_self;			/* Always set next hop to local IP address */
  int next_hop_keep;			/* Do not touch next hop attribute */
  int missing_lladdr;			/* What we will do when we don' know link-local addr, see MLL_* */
  int gw_mode;				/* How we compute route gateway from next_hop attr, see GW_* */
  int compare_path_lengths;		/* Use path lengths when selecting best route */
  int med_metric;			/* Compare MULTI_EXIT_DISC even between routes from differen ASes */
  int igp_metric;			/* Use IGP metrics when selecting best route */
  int prefer_older;			/* Prefer older routes according to RFC 5004 */
  int deterministic_med;		/* Use more complicated algo to have strict RFC 4271 MED comparison */
  u32 default_local_pref;		/* Default value for LOCAL_PREF attribute */
  u32 default_med;			/* Default value for MULTI_EXIT_DISC attribute */
  int capabilities;			/* Enable capability handshake [RFC3392] */
  int enable_refresh;			/* Enable local support for route refresh [RFC2918] */
  int enable_as4;			/* Enable local support for 4B AS numbers [RFC4893] */
  u32 rr_cluster_id;			/* Route reflector cluster ID, if different from local ID */
  int rr_client;			/* Whether neighbor is RR client of me */
  int rs_client;			/* Whether neighbor is RS client of me */
  int advertise_ipv4;			/* Whether we should add IPv4 capability advertisement to OPEN message */
  int passive;				/* Do not initiate outgoing connection */
  int interpret_communities;		/* Hardwired handling of well-known communities */
  int secondary;			/* Accept also non-best routes (i.e. RA_ACCEPTED) */
  unsigned connect_retry_time;
  unsigned hold_time, initial_hold_time;
  unsigned keepalive_time;
  unsigned start_delay_time;		/* Minimum delay between connects */
  unsigned error_amnesia_time;		/* Errors are forgotten after */
  unsigned error_delay_time_min;	/* Time to wait after an error is detected */
  unsigned error_delay_time_max;
  unsigned disable_after_error;		/* Disable the protocol when error is detected */
  char *password;			/* Password used for MD5 authentication */
  struct rtable_config *igp_table;	/* Table used for recursive next hop lookups */
};

#define MLL_SELF 1
#define MLL_DROP 2
#define MLL_IGNORE 3

#define GW_DIRECT 1
#define GW_RECURSIVE 2

struct bgp_conn {
  struct bgp_proto *bgp;
  struct birdsock *sk;
  unsigned int state;			/* State of connection state machine */
  struct timer *connect_retry_timer;
  struct timer *hold_timer;
  struct timer *keepalive_timer;
  struct event *tx_ev;
  int packets_to_send;			/* Bitmap of packet types to be sent */
  int notify_code, notify_subcode, notify_size;
  byte *notify_data;
  u32 advertised_as;			/* Temporary value for AS number received */
  int start_state;			/* protocol start_state snapshot when connection established */
  int want_as4_support;			/* Connection tries to establish AS4 session */
  int peer_as4_support;			/* Peer supports 4B AS numbers [RFC4893] */
  int peer_refresh_support;		/* Peer supports route refresh [RFC2918] */
  unsigned hold_time, keepalive_time;	/* Times calculated from my and neighbor's requirements */
};

struct bgp_proto {
  struct proto p;
  struct bgp_config *cf;		/* Shortcut to BGP configuration */
  u32 local_as, remote_as;
  int start_state;			/* Substates that partitions BS_START */
  int is_internal;			/* Internal BGP connection (local_as == remote_as) */
  int as4_session;			/* Session uses 4B AS numbers in AS_PATH (both sides support it) */
  u32 local_id;				/* BGP identifier of this router */
  u32 remote_id;			/* BGP identifier of the neighbor */
  u32 rr_cluster_id;			/* Route reflector cluster ID */
  int rr_client;			/* Whether neighbor is RR client of me */
  int rs_client;			/* Whether neighbor is RS client of me */
  struct bgp_conn *conn;		/* Connection we have established */
  struct bgp_conn outgoing_conn;	/* Outgoing connection we're working with */
  struct bgp_conn incoming_conn;	/* Incoming connection we have neither accepted nor rejected yet */
  struct object_lock *lock;		/* Lock for neighbor connection */
  struct neighbor *neigh;		/* Neighbor entry corresponding to remote ip, NULL if multihop */
  ip_addr source_addr;			/* Local address used as an advertised next hop */
  rtable *igp_table;			/* Table used for recursive next hop lookups */
  struct event *event;			/* Event for respawning and shutting process */
  struct timer *startup_timer;		/* Timer used to delay protocol startup due to previous errors (startup_delay) */
  struct bgp_bucket **bucket_hash;	/* Hash table of attribute buckets */
  unsigned int hash_size, hash_count, hash_limit;
  struct fib prefix_fib;		/* Prefixes to be sent */
  list bucket_queue;			/* Queue of buckets to send */
  struct bgp_bucket *withdraw_bucket;	/* Withdrawn routes */
  unsigned startup_delay;		/* Time to delay protocol startup by due to errors */
  bird_clock_t last_proto_error;	/* Time of last error that leads to protocol stop */
  u8 last_error_class; 			/* Error class of last error */
  u32 last_error_code;			/* Error code of last error. BGP protocol errors
					   are encoded as (bgp_err_code << 16 | bgp_err_subcode) */
#ifdef IPV6
  byte *mp_reach_start, *mp_unreach_start; /* Multiprotocol BGP attribute notes */
  unsigned mp_reach_len, mp_unreach_len;
  ip_addr local_link;			/* Link-level version of source_addr */
#endif
};

struct bgp_prefix {
  struct fib_node n;			/* Node in prefix fib */
  node bucket_node;			/* Node in per-bucket list */
};

struct bgp_bucket {
  node send_node;			/* Node in send queue */
  struct bgp_bucket *hash_next, *hash_prev;	/* Node in bucket hash table */
  unsigned hash;			/* Hash over extended attributes */
  list prefixes;			/* Prefixes in this buckets */
  ea_list eattrs[0];			/* Per-bucket extended attributes */
};

#define BGP_PORT		179
#define BGP_VERSION		4
#define BGP_HEADER_LENGTH	19
#define BGP_MAX_PACKET_LENGTH	4096
#define BGP_RX_BUFFER_SIZE	4096
#define BGP_TX_BUFFER_SIZE	BGP_MAX_PACKET_LENGTH

extern struct linpool *bgp_linpool;


void bgp_start_timer(struct timer *t, int value);
void bgp_check_config(struct bgp_config *c);
void bgp_error(struct bgp_conn *c, unsigned code, unsigned subcode, byte *data, int len);
void bgp_close_conn(struct bgp_conn *c);
void bgp_update_startup_delay(struct bgp_proto *p);
void bgp_conn_enter_openconfirm_state(struct bgp_conn *conn);
void bgp_conn_enter_established_state(struct bgp_conn *conn);
void bgp_conn_enter_close_state(struct bgp_conn *conn);
void bgp_conn_enter_idle_state(struct bgp_conn *conn);
void bgp_store_error(struct bgp_proto *p, struct bgp_conn *c, u8 class, u32 code);
void bgp_stop(struct bgp_proto *p, unsigned subcode);



#ifdef LOCAL_DEBUG
#define BGP_FORCE_DEBUG 1
#else
#define BGP_FORCE_DEBUG 0
#endif
#define BGP_TRACE(flags, msg, args...) do { if ((p->p.debug & flags) || BGP_FORCE_DEBUG) \
	log(L_TRACE "%s: " msg, p->p.name , ## args ); } while(0)

#define BGP_TRACE_RL(rl, flags, msg, args...) do { if ((p->p.debug & flags) || BGP_FORCE_DEBUG) \
	log_rl(rl, L_TRACE "%s: " msg, p->p.name , ## args ); } while(0)


/* attrs.c */

/* Hack: although BA_NEXT_HOP attribute has type EAF_TYPE_IP_ADDRESS, in IPv6
 * we store two addesses in it - a global address and a link local address.
 */
#ifdef IPV6
#define NEXT_HOP_LENGTH (2*sizeof(ip_addr))
static inline void set_next_hop(byte *b, ip_addr addr) { ((ip_addr *) b)[0] = addr; ((ip_addr *) b)[1] = IPA_NONE; }
#else
#define NEXT_HOP_LENGTH sizeof(ip_addr)
static inline void set_next_hop(byte *b, ip_addr addr) { ((ip_addr *) b)[0] = addr; }
#endif

void bgp_attach_attr(struct ea_list **to, struct linpool *pool, unsigned attr, uintptr_t val);
byte *bgp_attach_attr_wa(struct ea_list **to, struct linpool *pool, unsigned attr, unsigned len);
struct rta *bgp_decode_attrs(struct bgp_conn *conn, byte *a, unsigned int len, struct linpool *pool, int mandatory);
int bgp_get_attr(struct eattr *e, byte *buf, int buflen);
int bgp_rte_better(struct rte *, struct rte *);
int bgp_rte_recalculate(rtable *table, net *net, rte *new, rte *old, rte *old_best);
void bgp_rt_notify(struct proto *P, rtable *tbl UNUSED, net *n, rte *new, rte *old UNUSED, ea_list *attrs);
int bgp_import_control(struct proto *, struct rte **, struct ea_list **, struct linpool *);
void bgp_attr_init(struct bgp_proto *);
unsigned int bgp_encode_attrs(struct bgp_proto *p, byte *w, ea_list *attrs, int remains);
void bgp_free_bucket(struct bgp_proto *p, struct bgp_bucket *buck);
void bgp_get_route_info(struct rte *, byte *buf, struct ea_list *attrs);

inline static void bgp_attach_attr_ip(struct ea_list **to, struct linpool *pool, unsigned attr, ip_addr a)
{ *(ip_addr *) bgp_attach_attr_wa(to, pool, attr, sizeof(ip_addr)) = a; }

/* packets.c */

void mrt_dump_bgp_state_change(struct bgp_conn *conn, unsigned old, unsigned new);
void bgp_schedule_packet(struct bgp_conn *conn, int type);
void bgp_kick_tx(void *vconn);
void bgp_tx(struct birdsock *sk);
int bgp_rx(struct birdsock *sk, int size);
const char * bgp_error_dsc(unsigned code, unsigned subcode);
void bgp_log_error(struct bgp_proto *p, u8 class, char *msg, unsigned code, unsigned subcode, byte *data, unsigned len);

/* Packet types */

#define PKT_OPEN		0x01
#define PKT_UPDATE		0x02
#define PKT_NOTIFICATION	0x03
#define PKT_KEEPALIVE		0x04
#define PKT_ROUTE_REFRESH	0x05
#define PKT_SCHEDULE_CLOSE	0x1f	/* Used internally to schedule socket close */

/* Attributes */

#define BAF_OPTIONAL		0x80
#define BAF_TRANSITIVE		0x40
#define BAF_PARTIAL		0x20
#define BAF_EXT_LEN		0x10

#define BA_ORIGIN		0x01	/* [RFC1771] */		/* WM */
#define BA_AS_PATH		0x02				/* WM */
#define BA_NEXT_HOP		0x03				/* WM */
#define BA_MULTI_EXIT_DISC	0x04				/* ON */
#define BA_LOCAL_PREF		0x05				/* WD */
#define BA_ATOMIC_AGGR		0x06				/* WD */
#define BA_AGGREGATOR		0x07				/* OT */
#define BA_COMMUNITY		0x08	/* [RFC1997] */		/* OT */
#define BA_ORIGINATOR_ID	0x09	/* [RFC1966] */		/* ON */
#define BA_CLUSTER_LIST		0x0a				/* ON */
/* We don't support these: */
#define BA_DPA			0x0b	/* ??? */
#define BA_ADVERTISER		0x0c	/* [RFC1863] */
#define BA_RCID_PATH		0x0d
#define BA_MP_REACH_NLRI	0x0e	/* [RFC2283] */
#define BA_MP_UNREACH_NLRI	0x0f
#define BA_EXT_COMMUNITY	0x10	/* [RFC4360] */
#define BA_AS4_PATH             0x11    /* [RFC4893] */
#define BA_AS4_AGGREGATOR       0x12

/* BGP connection states */

#define BS_IDLE			0
#define BS_CONNECT		1	/* Attempting to connect */
#define BS_ACTIVE		2	/* Waiting for connection retry & listening */
#define BS_OPENSENT		3
#define BS_OPENCONFIRM		4
#define BS_ESTABLISHED		5
#define BS_CLOSE		6	/* Used during transition to BS_IDLE */

#define BS_MAX			7

/* BGP start states
 * 
 * Used in PS_START for fine-grained specification of starting state.
 *
 * When BGP protocol is started by core, it goes to BSS_PREPARE. When BGP protocol
 * done what is neccessary to start itself (like acquiring the lock), it goes to BSS_CONNECT.
 * When some connection attempt failed because of option or capability error, it goes to
 * BSS_CONNECT_NOCAP.
 */

#define BSS_PREPARE		0	/* Used before ordinary BGP started, i. e. waiting for lock */
#define BSS_DELAY		1	/* Startup delay due to previous errors */
#define BSS_CONNECT		2	/* Ordinary BGP connecting */
#define BSS_CONNECT_NOCAP	3	/* Legacy BGP connecting (without capabilities) */

/* Error classes */

#define BE_NONE			0
#define BE_MISC			1	/* Miscellaneous error */
#define BE_SOCKET		2	/* Socket error */
#define BE_BGP_RX		3	/* BGP protocol error notification received */
#define BE_BGP_TX		4	/* BGP protocol error notification sent */
#define BE_AUTO_DOWN		5	/* Automatic shutdown */
#define BE_MAN_DOWN		6	/* Manual shutdown */

/* Misc error codes */

#define BEM_NEIGHBOR_LOST	1
#define BEM_INVALID_NEXT_HOP	2
#define BEM_INVALID_MD5		3	/* MD5 authentication kernel request failed (possibly not supported) */
#define BEM_NO_SOCKET		4

/* Automatic shutdown error codes */

#define BEA_ROUTE_LIMIT_EXCEEDED 1

/* Well-known communities */

#define BGP_COMM_NO_EXPORT		0xffffff01	/* Don't export outside local AS / confed. */
#define BGP_COMM_NO_ADVERTISE		0xffffff02	/* Don't export at all */
#define BGP_COMM_NO_EXPORT_SUBCONFED	0xffffff03	/* NO_EXPORT even in local confederation */

/* Origins */

#define ORIGIN_IGP		0
#define ORIGIN_EGP		1
#define ORIGIN_INCOMPLETE	2

/* Address families */

#define BGP_AF_IPV4		1
#define BGP_AF_IPV6		2

#ifdef IPV6
#define BGP_AF BGP_AF_IPV6
#else
#define BGP_AF BGP_AF_IPV4
#endif

#endif
