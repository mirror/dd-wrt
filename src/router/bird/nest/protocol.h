/*
 *	BIRD Internet Routing Daemon -- Protocols
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_PROTOCOL_H_
#define _BIRD_PROTOCOL_H_

#include "lib/lists.h"
#include "lib/resource.h"
#include "lib/timer.h"
#include "conf/conf.h"

struct iface;
struct ifa;
struct rtable;
struct rte;
struct neighbor;
struct rta;
struct network;
struct proto_config;
struct config;
struct proto;
struct event;
struct ea_list;
struct eattr;
struct symbol;

/*
 *	Routing Protocol
 */

struct protocol {
  node n;
  char *name;
  char *template;			/* Template for automatic generation of names */
  int name_counter;			/* Counter for automatic name generation */
  int attr_class;			/* Attribute class known to this protocol */
  int multitable;			/* Protocol handles all announce hooks itself */
  uint preference;			/* Default protocol preference */
  uint config_size;			/* Size of protocol config */

  void (*preconfig)(struct protocol *, struct config *);	/* Just before configuring */
  void (*postconfig)(struct proto_config *);			/* After configuring each instance */
  struct proto * (*init)(struct proto_config *);		/* Create new instance */
  int (*reconfigure)(struct proto *, struct proto_config *);	/* Try to reconfigure instance, returns success */
  void (*dump)(struct proto *);			/* Debugging dump */
  void (*dump_attrs)(struct rte *);		/* Dump protocol-dependent attributes */
  int (*start)(struct proto *);			/* Start the instance */
  int (*shutdown)(struct proto *);		/* Stop the instance */
  void (*cleanup)(struct proto *);		/* Called after shutdown when protocol became hungry/down */
  void (*get_status)(struct proto *, byte *buf); /* Get instance status (for `show protocols' command) */
  void (*get_route_info)(struct rte *, byte *buf, struct ea_list *attrs); /* Get route information (for `show route' command) */
  int (*get_attr)(struct eattr *, byte *buf, int buflen);	/* ASCIIfy dynamic attribute (returns GA_*) */
  void (*show_proto_info)(struct proto *);	/* Show protocol info (for `show protocols all' command) */
  void (*copy_config)(struct proto_config *, struct proto_config *);	/* Copy config from given protocol instance */
};

void protos_build(void);
void proto_build(struct protocol *);
void protos_preconfig(struct config *);
void protos_postconfig(struct config *);
void protos_commit(struct config *new, struct config *old, int force_restart, int type);
void protos_dump_all(void);

#define GA_UNKNOWN	0		/* Attribute not recognized */
#define GA_NAME		1		/* Result = name */
#define GA_FULL		2		/* Result = both name and value */

/*
 *	Known protocols
 */

extern struct protocol
  proto_device, proto_radv, proto_rip, proto_static,
  proto_ospf, proto_pipe, proto_bgp, proto_bfd, proto_babel;

/*
 *	Routing Protocol Instance
 */

struct proto_config {
  node n;
  struct config *global;		/* Global configuration data */
  struct protocol *protocol;		/* Protocol */
  struct proto *proto;			/* Instance we've created */
  char *name;
  char *dsc;
  int class;				/* SYM_PROTO or SYM_TEMPLATE */
  u32 debug, mrtdump;			/* Debugging bitfields, both use D_* constants */
  unsigned preference, disabled;	/* Generic parameters */
  int in_keep_filtered;			/* Routes rejected in import filter are kept */
  u32 router_id;			/* Protocol specific router ID */
  struct rtable_config *table;		/* Table we're attached to */
  struct filter *in_filter, *out_filter; /* Attached filters */
  struct proto_limit *rx_limit;		/* Limit for receiving routes from protocol
					   (relevant when in_keep_filtered is active) */
  struct proto_limit *in_limit;		/* Limit for importing routes from protocol */
  struct proto_limit *out_limit;	/* Limit for exporting routes to protocol */

  /* Check proto_reconfigure() and proto_copy_config() after changing struct proto_config */

  /* Protocol-specific data follow... */
};

/* Protocol statistics */
struct proto_stats {
  /* Import - from protocol to core */
  u32 imp_routes;		/* Number of routes successfully imported to the (adjacent) routing table */
  u32 filt_routes;		/* Number of routes rejected in import filter but kept in the routing table */
  u32 pref_routes;		/* Number of routes that are preferred, sum over all routing tables */
  u32 imp_updates_received;	/* Number of route updates received */
  u32 imp_updates_invalid;	/* Number of route updates rejected as invalid */
  u32 imp_updates_filtered;	/* Number of route updates rejected by filters */
  u32 imp_updates_ignored;	/* Number of route updates rejected as already in route table */
  u32 imp_updates_accepted;	/* Number of route updates accepted and imported */
  u32 imp_withdraws_received;	/* Number of route withdraws received */
  u32 imp_withdraws_invalid;	/* Number of route withdraws rejected as invalid */
  u32 imp_withdraws_ignored;	/* Number of route withdraws rejected as already not in route table */
  u32 imp_withdraws_accepted;	/* Number of route withdraws accepted and processed */

  /* Export - from core to protocol */
  u32 exp_routes;		/* Number of routes successfully exported to the protocol */
  u32 exp_updates_received;	/* Number of route updates received */
  u32 exp_updates_rejected;	/* Number of route updates rejected by protocol */
  u32 exp_updates_filtered;	/* Number of route updates rejected by filters */
  u32 exp_updates_accepted;	/* Number of route updates accepted and exported */
  u32 exp_withdraws_received;	/* Number of route withdraws received */
  u32 exp_withdraws_accepted;	/* Number of route withdraws accepted and processed */
};

struct proto {
  node n;				/* Node in *_proto_list */
  node glob_node;			/* Node in global proto_list */
  struct protocol *proto;		/* Protocol */
  struct proto_config *cf;		/* Configuration data */
  struct proto_config *cf_new;		/* Configuration we want to switch to after shutdown (NULL=delete) */
  pool *pool;				/* Pool containing local objects */
  struct event *attn;			/* "Pay attention" event */

  char *name;				/* Name of this instance (== cf->name) */
  u32 debug;				/* Debugging flags */
  u32 mrtdump;				/* MRTDump flags */
  unsigned preference;			/* Default route preference */
  byte accept_ra_types;			/* Which types of route announcements are accepted (RA_OPTIMAL or RA_ANY) */
  byte disabled;			/* Manually disabled */
  byte proto_state;			/* Protocol state machine (PS_*, see below) */
  byte core_state;			/* Core state machine (FS_*, see below) */
  byte export_state;			/* Route export state (ES_*, see below) */
  byte reconfiguring;			/* We're shutting down due to reconfiguration */
  byte refeeding;			/* We are refeeding (valid only if export_state == ES_FEEDING) */
  byte flushing;			/* Protocol is flushed in current flush loop round */
  byte gr_recovery;			/* Protocol should participate in graceful restart recovery */
  byte gr_lock;				/* Graceful restart mechanism should wait for this proto */
  byte gr_wait;				/* Route export to protocol is postponed until graceful restart */
  byte down_sched;			/* Shutdown is scheduled for later (PDS_*) */
  byte down_code;			/* Reason for shutdown (PDC_* codes) */
  byte merge_limit;			/* Maximal number of nexthops for RA_MERGED */
  u32 hash_key;				/* Random key used for hashing of neighbors */
  bird_clock_t last_state_change;	/* Time of last state transition */
  char *last_state_name_announced;	/* Last state name we've announced to the user */
  struct proto_stats stats;		/* Current protocol statistics */

  /*
   *	General protocol hooks:
   *
   *	   if_notify	Notify protocol about interface state changes.
   *	   ifa_notify	Notify protocol about interface address changes.
   *	   rt_notify	Notify protocol about routing table updates.
   *	   neigh_notify	Notify protocol about neighbor cache events.
   *	   make_tmp_attrs  Construct ea_list from private attrs stored in rte.
   *	   store_tmp_attrs Store private attrs back to the rte.
   *	   import_control  Called as the first step of the route importing process.
   *			It can construct a new rte, add private attributes and
   *			decide whether the route shall be imported: 1=yes, -1=no,
   *			0=process it through the import filter set by the user.
   *	   reload_routes   Request protocol to reload all its routes to the core
   *			(using rte_update()). Returns: 0=reload cannot be done,
   *			1= reload is scheduled and will happen (asynchronously).
   *	   feed_begin	Notify protocol about beginning of route feeding.
   *	   feed_end	Notify protocol about finish of route feeding.
   */

  void (*if_notify)(struct proto *, unsigned flags, struct iface *i);
  void (*ifa_notify)(struct proto *, unsigned flags, struct ifa *a);
  void (*rt_notify)(struct proto *, struct rtable *table, struct network *net, struct rte *new, struct rte *old, struct ea_list *attrs);
  void (*neigh_notify)(struct neighbor *neigh);
  struct ea_list *(*make_tmp_attrs)(struct rte *rt, struct linpool *pool);
  void (*store_tmp_attrs)(struct rte *rt, struct ea_list *attrs);
  int (*import_control)(struct proto *, struct rte **rt, struct ea_list **attrs, struct linpool *pool);
  int (*reload_routes)(struct proto *);
  void (*feed_begin)(struct proto *, int initial);
  void (*feed_end)(struct proto *);

  /*
   *	Routing entry hooks (called only for routes belonging to this protocol):
   *
   *	   rte_recalculate Called at the beginning of the best route selection
   *	   rte_better	Compare two rte's and decide which one is better (1=first, 0=second).
   *       rte_same	Compare two rte's and decide whether they are identical (1=yes, 0=no).
   *       rte_mergable	Compare two rte's and decide whether they could be merged (1=yes, 0=no).
   *	   rte_insert	Called whenever a rte is inserted to a routing table.
   *	   rte_remove	Called whenever a rte is removed from the routing table.
   */

  int (*rte_recalculate)(struct rtable *, struct network *, struct rte *, struct rte *, struct rte *);
  int (*rte_better)(struct rte *, struct rte *);
  int (*rte_same)(struct rte *, struct rte *);
  int (*rte_mergable)(struct rte *, struct rte *);
  void (*rte_insert)(struct network *, struct rte *);
  void (*rte_remove)(struct network *, struct rte *);

  struct rtable *table;			/* Our primary routing table */
  struct rte_src *main_source;		/* Primary route source */
  struct announce_hook *main_ahook;	/* Primary announcement hook */
  struct announce_hook *ahooks;		/* Announcement hooks for this protocol */

  struct fib_iterator *feed_iterator;	/* Routing table iterator used during protocol feeding */
  struct announce_hook *feed_ahook;	/* Announce hook we currently feed */

  /* Hic sunt protocol-specific data */
};

struct proto_spec {
  void *ptr;
  int patt;
};


#define PDS_DISABLE		1	/* Proto disable scheduled */
#define PDS_RESTART		2	/* Proto restart scheduled */

#define PDC_CF_REMOVE		0x01	/* Removed in new config */
#define PDC_CF_DISABLE		0x02	/* Disabled in new config */
#define PDC_CF_RESTART		0x03	/* Restart due to reconfiguration */
#define PDC_CMD_DISABLE		0x11	/* Result of disable command */
#define PDC_CMD_RESTART		0x12	/* Result of restart command */
#define PDC_CMD_SHUTDOWN	0x13	/* Result of global shutdown */
#define PDC_RX_LIMIT_HIT	0x21	/* Route receive limit reached */
#define PDC_IN_LIMIT_HIT	0x22	/* Route import limit reached */
#define PDC_OUT_LIMIT_HIT	0x23	/* Route export limit reached */


void *proto_new(struct proto_config *, unsigned size);
void *proto_config_new(struct protocol *, int class);
void proto_copy_config(struct proto_config *dest, struct proto_config *src);
void proto_request_feeding(struct proto *p);

static inline void
proto_copy_rest(struct proto_config *dest, struct proto_config *src, unsigned size)
{ memcpy(dest + 1, src + 1, size - sizeof(struct proto_config)); }

void graceful_restart_recovery(void);
void graceful_restart_init(void);
void graceful_restart_show_status(void);
void proto_graceful_restart_lock(struct proto *p);
void proto_graceful_restart_unlock(struct proto *p);

#define DEFAULT_GR_WAIT	240

void proto_show_limit(struct proto_limit *l, const char *dsc);
void proto_show_basic_info(struct proto *p);

void proto_cmd_show(struct proto *, uint, int);
void proto_cmd_disable(struct proto *, uint, int);
void proto_cmd_enable(struct proto *, uint, int);
void proto_cmd_restart(struct proto *, uint, int);
void proto_cmd_reload(struct proto *, uint, int);
void proto_cmd_debug(struct proto *, uint, int);
void proto_cmd_mrtdump(struct proto *, uint, int);

void proto_apply_cmd(struct proto_spec ps, void (* cmd)(struct proto *, uint, int), int restricted, uint arg);
struct proto *proto_get_named(struct symbol *, struct protocol *);

#define CMD_RELOAD	0
#define CMD_RELOAD_IN	1
#define CMD_RELOAD_OUT	2

static inline u32
proto_get_router_id(struct proto_config *pc)
{
  return pc->router_id ? pc->router_id : pc->global->router_id;
}

extern list active_proto_list;

/*
 *  Each protocol instance runs two different state machines:
 *
 *  [P] The protocol machine: (implemented inside protocol)
 *
 *		DOWN    ---->    START
 *		  ^		   |
 *		  |		   V
 *		STOP    <----     UP
 *
 *	States:	DOWN	Protocol is down and it's waiting for the core
 *			requesting protocol start.
 *		START	Protocol is waiting for connection with the rest
 *			of the network and it's not willing to accept
 *			packets. When it connects, it goes to UP state.
 *		UP	Protocol is up and running. When the network
 *			connection breaks down or the core requests
 *			protocol to be terminated, it goes to STOP state.
 *		STOP	Protocol is disconnecting from the network.
 *			After it disconnects, it returns to DOWN state.
 *
 *	In:	start()	Called in DOWN state to request protocol startup.
 *			Returns new state: either UP or START (in this
 *			case, the protocol will notify the core when it
 *			finally comes UP).
 *		stop()	Called in START, UP or STOP state to request
 *			protocol shutdown. Returns new state: either
 *			DOWN or STOP (in this case, the protocol will
 *			notify the core when it finally comes DOWN).
 *
 *	Out:	proto_notify_state() -- called by protocol instance when
 *			it does any state transition not covered by
 *			return values of start() and stop(). This includes
 *			START->UP (delayed protocol startup), UP->STOP
 *			(spontaneous shutdown) and STOP->DOWN (delayed
 *			shutdown).
 */

#define PS_DOWN 0
#define PS_START 1
#define PS_UP 2
#define PS_STOP 3

void proto_notify_state(struct proto *p, unsigned state);

/*
 *  [F] The feeder machine: (implemented in core routines)
 *
 *		HUNGRY    ---->   FEEDING
 *		 ^		     |
 *		 | 		     V
 *		FLUSHING  <----   HAPPY
 *
 *	States:	HUNGRY	Protocol either administratively down (i.e.,
 *			disabled by the user) or temporarily down
 *			(i.e., [P] is not UP)
 *		FEEDING	The protocol came up and we're feeding it
 *			initial routes. [P] is UP.
 *		HAPPY	The protocol is up and it's receiving normal
 *			routing updates. [P] is UP.
 *		FLUSHING The protocol is down and we're removing its
 *			routes from the table. [P] is STOP or DOWN.
 *
 *	Normal lifecycle of a protocol looks like:
 *
 *		HUNGRY/DOWN --> HUNGRY/START --> HUNGRY/UP -->
 *		FEEDING/UP --> HAPPY/UP --> FLUSHING/STOP|DOWN -->
 *		HUNGRY/STOP|DOWN --> HUNGRY/DOWN
 *
 *	Sometimes, protocol might switch from HAPPY/UP to FEEDING/UP
 *	if it wants to refeed the routes (for example BGP does so
 *	as a result of received ROUTE-REFRESH request).
 */

#define FS_HUNGRY	0
#define FS_FEEDING	1	/* obsolete */
#define FS_HAPPY	2
#define FS_FLUSHING	3


#define ES_DOWN		0
#define ES_FEEDING	1
#define ES_READY	2



/*
 *	Debugging flags
 */

#define D_STATES 1		/* [core] State transitions */
#define D_ROUTES 2		/* [core] Routes passed by the filters */
#define D_FILTERS 4		/* [core] Routes rejected by the filters */
#define D_IFACES 8		/* [core] Interface events */
#define D_EVENTS 16		/* Protocol events */
#define D_PACKETS 32		/* Packets sent/received */

#ifndef PARSER
#define TRACE(flags, msg, args...) \
  do { if (p->p.debug & flags) log(L_TRACE "%s: " msg, p->p.name , ## args ); } while(0)
#endif


/*
 *	MRTDump flags
 */

#define MD_STATES	1		/* Protocol state changes (BGP4MP_MESSAGE_AS4) */
#define MD_MESSAGES	2		/* Protocol packets (BGP4MP_MESSAGE_AS4) */

/*
 *	Known unique protocol instances as referenced by config routines
 */

extern struct proto_config *cf_dev_proto;


/*
 * Protocol limits
 */

#define PLD_RX		0	/* Receive limit */
#define PLD_IN		1	/* Import limit */
#define PLD_OUT		2	/* Export limit */
#define PLD_MAX		3

#define PLA_WARN	1	/* Issue log warning */
#define PLA_BLOCK	2	/* Block new routes */
#define PLA_RESTART	4	/* Force protocol restart */
#define PLA_DISABLE	5	/* Shutdown and disable protocol */

#define PLS_INITIAL	0	/* Initial limit state after protocol start */
#define PLS_ACTIVE	1	/* Limit was hit */
#define PLS_BLOCKED	2	/* Limit is active and blocking new routes */

struct proto_limit {
  u32 limit;			/* Maximum number of prefixes */
  byte action;			/* Action to take (PLA_*) */
  byte state;			/* State of limit (PLS_*) */
};

void proto_notify_limit(struct announce_hook *ah, struct proto_limit *l, int dir, u32 rt_count);
void proto_verify_limits(struct announce_hook *ah);

static inline void
proto_reset_limit(struct proto_limit *l)
{
  if (l)
    l->state = PLS_INITIAL;
}


/*
 *	Route Announcement Hook
 */

struct announce_hook {
  node n;
  struct rtable *table;
  struct proto *proto;
  struct filter *in_filter;		/* Input filter */
  struct filter *out_filter;		/* Output filter */
  struct proto_limit *rx_limit;		/* Receive limit (for in_keep_filtered) */
  struct proto_limit *in_limit;		/* Input limit */
  struct proto_limit *out_limit;	/* Output limit */
  struct proto_stats *stats;		/* Per-table protocol statistics */
  struct announce_hook *next;		/* Next hook for the same protocol */
  int in_keep_filtered;			/* Routes rejected in import filter are kept */
};

struct announce_hook *proto_add_announce_hook(struct proto *p, struct rtable *t, struct proto_stats *stats);
struct announce_hook *proto_find_announce_hook(struct proto *p, struct rtable *t);

#endif
