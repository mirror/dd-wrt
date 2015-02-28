/*
 *	BIRD Internet Routing Daemon -- Network Interfaces
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_IFACE_H_
#define _BIRD_IFACE_H_

#include "lib/lists.h"

extern list iface_list;

struct proto;
struct pool;

struct ifa {				/* Interface address */
  node n;
  struct iface *iface;			/* Interface this address belongs to */
  ip_addr ip;				/* IP address of this host */
  ip_addr prefix;			/* Network prefix */
  unsigned pxlen;			/* Prefix length */
  ip_addr brd;				/* Broadcast address */
  ip_addr opposite;			/* Opposite end of a point-to-point link */
  unsigned scope;			/* Interface address scope */
  unsigned flags;			/* Analogous to iface->flags */
};

struct iface {
  node n;
  char name[16];
  unsigned flags;
  unsigned mtu;
  unsigned index;			/* OS-dependent interface index */
  list addrs;				/* Addresses assigned to this interface */
  struct ifa *addr;			/* Primary address */
  list neighbors;			/* All neighbors on this interface */
};

#define IF_UP 1				/* IF_ADMIN_UP and IP address known */
#define IF_MULTIACCESS 2
#define IF_BROADCAST 4
#define IF_MULTICAST 8
#define IF_SHUTDOWN 0x10		/* Interface disappeared */
#define IF_LOOPBACK 0x20
#define IF_IGNORE 0x40			/* Not to be used by routing protocols (loopbacks etc.) */
#define IF_ADMIN_UP 0x80		/* Administrative up (e.g. IFF_UP in Linux) */
#define IF_LINK_UP 0x100		/* Link available (e.g. IFF_LOWER_UP in Linux) */

#define IA_PRIMARY 0x10000		/* This address is primary */
#define IA_SECONDARY 0x20000		/* This address has been reported as secondary by the kernel */
#define IA_PEER 0x40000			/* A peer/ptp address */
#define IA_HOST 0x80000			/* A host/loopback address */
#define IA_FLAGS 0xff0000

/*
 * There are three kinds of addresses in BIRD:
 *  - Standard (prefix-based) addresses, these may define ifa.opposite (for /30 or /31).
 *  - Peer/ptp addresses, without common prefix for ifa.ip and ifa.opposite.
 *    ifa.opposite is defined and ifa.prefix/pxlen == ifa.opposite/32 (for simplicity).
 *  - Host addresses, with ifa.prefix/pxlen == ifa.ip/32 (or /128).
 *    May be considered a special case of standard addresses.
 *
 * Peer addresses (AFAIK) do not exist in IPv6. Linux also supports generalized peer
 * addresses (with pxlen < 32 and ifa.ip outside prefix), we do not support that.
 */


#define IF_JUST_CREATED 0x10000000	/* Send creation event as soon as possible */
#define IF_TMP_DOWN 0x20000000		/* Temporary shutdown due to interface reconfiguration */
#define IF_UPDATED 0x40000000		/* Touched in last scan */

/* Interface change events */

#define IF_CHANGE_UP 1
#define IF_CHANGE_DOWN 2
#define IF_CHANGE_MTU 4
#define IF_CHANGE_CREATE 8		/* Seen this interface for the first time */
#define IF_CHANGE_LINK 0x10
#define IF_CHANGE_TOO_MUCH 0x40000000	/* Used internally */

void if_init(void);
void if_dump(struct iface *);
void if_dump_all(void);
void ifa_dump(struct ifa *);
void if_show(void);
void if_show_summary(void);
struct iface *if_update(struct iface *);
void if_delete(struct iface *old);
struct ifa *ifa_update(struct ifa *);
void ifa_delete(struct ifa *);
void if_start_update(void);
void if_end_partial_update(struct iface *);
void if_end_update(void);
void if_flush_ifaces(struct proto *p);
void if_feed_baby(struct proto *);
struct iface *if_find_by_index(unsigned);
struct iface *if_find_by_name(char *);
struct iface *if_get_by_name(char *);
void ifa_recalc_all_primary_addresses(void);


/* The Neighbor Cache */

typedef struct neighbor {
  node n;				/* Node in global neighbor list */
  node if_n;				/* Node in per-interface neighbor list */
  ip_addr addr;				/* Address of the neighbor */
  struct ifa *ifa;			/* Ifa on related iface */
  struct iface *iface;			/* Interface it's connected to */
  struct proto *proto;			/* Protocol this belongs to */
  void *data;				/* Protocol-specific data */
  unsigned aux;				/* Protocol-specific data */
  unsigned flags;
  int scope;				/* Address scope, -1 for unreachable sticky neighbors,
					   SCOPE_HOST when it's our own address */
} neighbor;

#define NEF_STICKY 1
#define NEF_ONLINK 2
#define NEF_BIND 4			/* Used internally for neighbors bound to an iface */

neighbor *neigh_find(struct proto *, ip_addr *, unsigned flags);
neighbor *neigh_find2(struct proto *p, ip_addr *a, struct iface *ifa, unsigned flags);

static inline int neigh_connected_to(struct proto *p, ip_addr *a, struct iface *i)
{
  neighbor *n = neigh_find(p, a, 0);
  return n && n->iface == i;
}

void neigh_dump(neighbor *);
void neigh_dump_all(void);
void neigh_prune(void);
void neigh_if_up(struct iface *);
void neigh_if_down(struct iface *);
void neigh_if_link(struct iface *);
void neigh_ifa_update(struct ifa *);
void neigh_init(struct pool *);

/*
 *	Interface Pattern Lists
 */

struct iface_patt_node {
  node n;
  int positive;
  byte *pattern;
  ip_addr prefix;
  int pxlen;
};

struct iface_patt {
  node n;
  list ipn_list;			/* A list of struct iface_patt_node */

  /* Protocol-specific data follow after this structure */
};

int iface_patt_match(struct iface_patt *ifp, struct iface *i, struct ifa *a);
struct iface_patt *iface_patt_find(list *l, struct iface *i, struct ifa *a);
int iface_patts_equal(list *, list *, int (*)(struct iface_patt *, struct iface_patt *));


u32 if_choose_router_id(struct iface_patt *mask, u32 old_id);

#endif
