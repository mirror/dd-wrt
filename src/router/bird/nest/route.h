/*
 *	BIRD Internet Routing Daemon -- Routing Table
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_ROUTE_H_
#define _BIRD_ROUTE_H_

#include "lib/lists.h"
#include "lib/resource.h"
#include "lib/timer.h"

struct protocol;
struct proto;
struct symbol;
struct filter;
struct cli;

/*
 *	Generic data structure for storing network prefixes. Also used
 *	for the master routing table. Currently implemented as a hash
 *	table.
 *
 *	Available operations:
 *		- insertion of new entry
 *		- deletion of entry
 *		- searching for entry by network prefix
 *		- asynchronous retrieval of fib contents
 */

struct fib_node {
  struct fib_node *next;		/* Next in hash chain */
  struct fib_iterator *readers;		/* List of readers of this node */
  byte pxlen;
  byte flags;				/* User-defined */
  byte x0, x1;				/* User-defined */
  ip_addr prefix;			/* In host order */
};

struct fib_iterator {			/* See lib/slists.h for an explanation */
  struct fib_iterator *prev, *next;	/* Must be synced with struct fib_node! */
  byte efef;				/* 0xff to distinguish between iterator and node */
  byte pad[3];
  struct fib_node *node;		/* Or NULL if freshly merged */
  unsigned int hash;
};

typedef void (*fib_init_func)(struct fib_node *);

struct fib {
  pool *fib_pool;			/* Pool holding all our data */
  slab *fib_slab;			/* Slab holding all fib nodes */
  struct fib_node **hash_table;		/* Node hash table */
  unsigned int hash_size;		/* Number of hash table entries (a power of two) */
  unsigned int hash_order;		/* Binary logarithm of hash_size */
  unsigned int hash_shift;		/* 16 - hash_log */
  unsigned int entries;			/* Number of entries */
  unsigned int entries_min, entries_max;/* Entry count limits (else start rehashing) */
  fib_init_func init;			/* Constructor */
};

void fib_init(struct fib *, pool *, unsigned node_size, unsigned hash_order, fib_init_func init);
void *fib_find(struct fib *, ip_addr *, int);	/* Find or return NULL if doesn't exist */
void *fib_get(struct fib *, ip_addr *, int); 	/* Find or create new if nonexistent */
void *fib_route(struct fib *, ip_addr, int);	/* Longest-match routing lookup */
void fib_delete(struct fib *, void *);	/* Remove fib entry */
void fib_free(struct fib *);		/* Destroy the fib */
void fib_check(struct fib *);		/* Consistency check for debugging */

void fit_init(struct fib_iterator *, struct fib *); /* Internal functions, don't call */
struct fib_node *fit_get(struct fib *, struct fib_iterator *);
void fit_put(struct fib_iterator *, struct fib_node *);

#define FIB_WALK(fib, z) do {					\
	struct fib_node *z, **ff = (fib)->hash_table;		\
	unsigned int count = (fib)->hash_size;			\
	while (count--)						\
	  for(z = *ff++; z; z=z->next)

#define FIB_WALK_END } while (0)

#define FIB_ITERATE_INIT(it, fib) fit_init(it, fib)

#define FIB_ITERATE_START(fib, it, z) do {			\
	struct fib_node *z = fit_get(fib, it);			\
	unsigned int count = (fib)->hash_size;			\
	unsigned int hpos = (it)->hash;				\
	for(;;) {						\
	  if (!z)						\
            {							\
	       if (++hpos >= count)				\
		 break;						\
	       z = (fib)->hash_table[hpos];			\
	       continue;					\
	    }

#define FIB_ITERATE_END(z) z = z->next; } } while(0)

#define FIB_ITERATE_PUT(it, z) fit_put(it, z)

/*
 *	Master Routing Tables. Generally speaking, each of them contains a FIB
 *	with each entry pointing to a list of route entries representing routes
 *	to given network (with the selected one at the head).
 *
 *	Each of the RTE's contains variable data (the preference and protocol-dependent
 *	metrics) and a pointer to a route attribute block common for many routes).
 *
 *	It's guaranteed that there is at most one RTE for every (prefix,proto) pair.
 */

struct rtable_config {
  node n;
  char *name;
  struct rtable *table;
  struct proto_config *krt_attached;	/* Kernel syncer attached to this table */
  int gc_max_ops;			/* Maximum number of operations before GC is run */
  int gc_min_time;			/* Minimum time between two consecutive GC runs */
};

typedef struct rtable {
  node n;				/* Node in list of all tables */
  struct fib fib;
  char *name;				/* Name of this table */
  list hooks;				/* List of announcement hooks */
  int pipe_busy;			/* Pipe loop detection */
  int use_count;			/* Number of protocols using this table */
  struct rtable_config *config;		/* Configuration of this table */
  struct config *deleted;		/* Table doesn't exist in current configuration,
					 * delete as soon as use_count becomes 0 and remove
					 * obstacle from this routing table.
					 */
  struct event *gc_event;		/* Garbage collector event */
  int gc_counter;			/* Number of operations since last GC */
  bird_clock_t gc_time;			/* Time of last GC */
} rtable;

typedef struct network {
  struct fib_node n;			/* FIB flags reserved for kernel syncer */
  struct rte *routes;			/* Available routes for this network */
} net;

typedef struct rte {
  struct rte *next;
  net *net;				/* Network this RTE belongs to */
  struct proto *sender;			/* Protocol instance that sent the route to the routing table */
  struct rta *attrs;			/* Attributes of this route */
  byte flags;				/* Flags (REF_...) */
  byte pflags;				/* Protocol-specific flags */
  word pref;				/* Route preference */
  bird_clock_t lastmod;			/* Last modified */
  union {				/* Protocol-dependent data (metrics etc.) */
#ifdef CONFIG_RIP
    struct {
      node garbage;			/* List for garbage collection */
      byte metric;			/* RIP metric */
      u16 tag;				/* External route tag */
      struct rip_entry *entry;
    } rip;
#endif
#ifdef CONFIG_OSPF
    struct {
      u32 metric1, metric2;		/* OSPF Type 1 and Type 2 metrics */
      u32 tag;				/* External route tag */
    } ospf;
#endif
    struct {				/* Routes generated by krt sync (both temporary and inherited ones) */
      s8 src;				/* Alleged route source (see krt.h) */
      u8 proto;				/* Kernel source protocol ID */
      u8 type;				/* Kernel route type */
      u8 seen;				/* Seen during last scan */
      u32 metric;			/* Kernel metric */
    } krt;
  } u;
} rte;

#define REF_COW 1			/* Copy this rte on write */

/* Types of route announcement, also used as flags */
#define RA_OPTIMAL 1			/* Announcement of optimal route change */
#define RA_ANY 2			/* Announcement of any route change */

struct config;

void rt_init(void);
void rt_preconfig(struct config *);
void rt_commit(struct config *new, struct config *old);
void rt_lock_table(rtable *);
void rt_unlock_table(rtable *);
void rt_setup(pool *, rtable *, char *, struct rtable_config *);
static inline net *net_find(rtable *tab, ip_addr addr, unsigned len) { return (net *) fib_find(&tab->fib, &addr, len); }
static inline net *net_get(rtable *tab, ip_addr addr, unsigned len) { return (net *) fib_get(&tab->fib, &addr, len); }
rte *rte_find(net *net, struct proto *p);
rte *rte_get_temp(struct rta *);
void rte_update(rtable *tab, net *net, struct proto *p, struct proto *src, rte *new);
void rte_discard(rtable *tab, rte *old);
void rte_dump(rte *);
void rte_free(rte *);
rte *rte_do_cow(rte *);
static inline rte * rte_cow(rte *r) { return (r->flags & REF_COW) ? rte_do_cow(r) : r; }
void rt_dump(rtable *);
void rt_dump_all(void);
int rt_feed_baby(struct proto *p);
void rt_feed_baby_abort(struct proto *p);
void rt_prune(rtable *tab);
void rt_prune_all(void);
struct rtable_config *rt_new_table(struct symbol *s);

struct rt_show_data {
  ip_addr prefix;
  unsigned pxlen;
  rtable *table;
  struct filter *filter;
  int verbose;
  struct fib_iterator fit;
  struct proto *show_protocol;
  struct proto *export_protocol;
  int export_mode, primary_only;
  struct config *running_on_config;
  int net_counter, rt_counter, show_counter;
  int stats, show_for;
};
void rt_show(struct rt_show_data *);

/*
 *	Route Attributes
 *
 *	Beware: All standard BGP attributes must be represented here instead
 *	of making them local to the route. This is needed to ensure proper
 *	construction of BGP route attribute lists.
 */

typedef struct rta {
  struct rta *next, **pprev;		/* Hash chain */
  struct proto *proto;			/* Protocol instance that originally created the route */
  unsigned uc;				/* Use count */
  byte source;				/* Route source (RTS_...) */
  byte scope;				/* Route scope (SCOPE_... -- see ip.h) */
  byte cast;				/* Casting type (RTC_...) */
  byte dest;				/* Route destination type (RTD_...) */
  byte flags;				/* Route flags (RTF_...), now unused */
  byte aflags;				/* Attribute cache flags (RTAF_...) */
  u16 hash_key;				/* Hash over important fields */
  ip_addr gw;				/* Next hop */
  ip_addr from;				/* Advertising router */
  struct iface *iface;			/* Outgoing interface */
  struct ea_list *eattrs;		/* Extended Attribute chain */
} rta;

#define RTS_DUMMY 0			/* Dummy route to be removed soon */
#define RTS_STATIC 1			/* Normal static route */
#define RTS_INHERIT 2			/* Route inherited from kernel */
#define RTS_DEVICE 3			/* Device route */
#define RTS_STATIC_DEVICE 4		/* Static device route */
#define RTS_REDIRECT 5			/* Learned via redirect */
#define RTS_RIP 6			/* RIP route */
#define RTS_OSPF 7			/* OSPF route */
#define RTS_OSPF_IA 8			/* OSPF inter-area route */
#define RTS_OSPF_EXT1 9			/* OSPF external route type 1 */
#define RTS_OSPF_EXT2 10		/* OSPF external route type 2 */
#define RTS_BGP 11			/* BGP route */
#define RTS_PIPE 12			/* Inter-table wormhole */

#define RTC_UNICAST 0
#define RTC_BROADCAST 1
#define RTC_MULTICAST 2
#define RTC_ANYCAST 3			/* IPv6 Anycast */

#define RTD_ROUTER 0			/* Next hop is neighbor router */
#define RTD_DEVICE 1			/* Points to device */
#define RTD_BLACKHOLE 2			/* Silently drop packets */
#define RTD_UNREACHABLE 3		/* Reject as unreachable */
#define RTD_PROHIBIT 4			/* Administratively prohibited */
#define RTD_NONE 5			/* Invalid RTD */

#define RTAF_CACHED 1			/* This is a cached rta */

/*
 *	Extended Route Attributes
 */

typedef struct eattr {
  word id;				/* EA_CODE(EAP_..., protocol-dependent ID) */
  byte flags;				/* Protocol-dependent flags */
  byte type;				/* Attribute type and several flags (EAF_...) */
  union {
    u32 data;
    struct adata *ptr;			/* Attribute data elsewhere */
  } u;
} eattr;

#define EAP_GENERIC 0			/* Generic attributes */
#define EAP_BGP 1			/* BGP attributes */
#define EAP_RIP 2			/* RIP */
#define EAP_OSPF 3			/* OSPF */
#define EAP_MAX 4

#define EA_CODE(proto,id) (((proto) << 8) | (id))
#define EA_PROTO(ea) ((ea) >> 8)
#define EA_ID(ea) ((ea) & 0xff)

#define EA_CODE_MASK 0xffff
#define EA_ALLOW_UNDEF 0x10000		/* ea_find: allow EAF_TYPE_UNDEF */

#define EAF_TYPE_MASK 0x0f		/* Mask with this to get type */
#define EAF_TYPE_INT 0x01		/* 32-bit signed integer number */
#define EAF_TYPE_OPAQUE 0x02		/* Opaque byte string (not filterable) */
#define EAF_TYPE_IP_ADDRESS 0x04	/* IP address */
#define EAF_TYPE_ROUTER_ID 0x05		/* Router ID (IPv4 address) */
#define EAF_TYPE_AS_PATH 0x06		/* BGP AS path (encoding per RFC 1771:4.3) */
#define EAF_TYPE_INT_SET 0x0a		/* Set of u32's (e.g., a community list) */
#define EAF_TYPE_UNDEF 0x0f		/* `force undefined' entry */
#define EAF_EMBEDDED 0x01		/* Data stored in eattr.u.data (part of type spec) */
#define EAF_VAR_LENGTH 0x02		/* Attribute length is variable (part of type spec) */
#define EAF_ORIGINATED 0x40		/* The attribute has originated locally */
#define EAF_TEMP 0x80			/* A temporary attribute (the one stored in the tmp attr list) */

struct adata {
  unsigned int length;			/* Length of data */
  byte data[0];
};

typedef struct ea_list {
  struct ea_list *next;			/* In case we have an override list */
  byte flags;				/* Flags: EALF_... */
  byte rfu;
  word count;				/* Number of attributes */
  eattr attrs[0];			/* Attribute definitions themselves */
} ea_list;

#define EALF_SORTED 1			/* Attributes are sorted by code */
#define EALF_BISECT 2			/* Use interval bisection for searching */
#define EALF_CACHED 4			/* Attributes belonging to cached rta */

eattr *ea_find(ea_list *, unsigned ea);
int ea_get_int(ea_list *, unsigned ea, int def);
void ea_dump(ea_list *);
void ea_sort(ea_list *);		/* Sort entries in all sub-lists */
unsigned ea_scan(ea_list *);		/* How many bytes do we need for merged ea_list */
void ea_merge(ea_list *from, ea_list *to); /* Merge sub-lists to allocated buffer */
int ea_same(ea_list *x, ea_list *y);	/* Test whether two ea_lists are identical */
unsigned int ea_hash(ea_list *e);	/* Calculate 16-bit hash value */
void ea_format(eattr *e, byte *buf);
#define EA_FORMAT_BUF_SIZE 256
ea_list *ea_append(ea_list *to, ea_list *what);

void rta_init(void);
rta *rta_lookup(rta *);			/* Get rta equivalent to this one, uc++ */
static inline rta *rta_clone(rta *r) { r->uc++; return r; }
void rta__free(rta *r);
static inline void rta_free(rta *r) { if (r && !--r->uc) rta__free(r); }
void rta_dump(rta *);
void rta_dump_all(void);
void rta_show(struct cli *, rta *, ea_list *);

extern struct protocol *attr_class_to_protocol[EAP_MAX];

/*
 *	Default protocol preferences
 */

#define DEF_PREF_DIRECT	    	240	/* Directly connected */
#define DEF_PREF_STATIC		200	/* Static route */
#define DEF_PREF_OSPF		150	/* OSPF intra-area, inter-area and type 1 external routes */
#define DEF_PREF_RIP		120	/* RIP */
#define DEF_PREF_BGP		100	/* BGP */
#define DEF_PREF_PIPE		70	/* Routes piped from other tables */
#define DEF_PREF_INHERITED	10	/* Routes inherited from other routing daemons */

#endif
