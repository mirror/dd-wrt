/*
 *	BIRD -- Router Advertisement
 *
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_RADV_H_
#define _BIRD_RADV_H_

#include "nest/bird.h"

#include "lib/ip.h"
#include "lib/lists.h"
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


#define ICMPV6_PROTO 58

#define ICMPV6_RS 133
#define ICMPV6_RA 134

#define MAX_INITIAL_RTR_ADVERTISEMENTS 3
#define MAX_INITIAL_RTR_ADVERT_INTERVAL 16

#define DEFAULT_MAX_RA_INT 600
#define DEFAULT_MIN_DELAY 3
#define DEFAULT_CURRENT_HOP_LIMIT 64

#define DEFAULT_VALID_LIFETIME 86400
#define DEFAULT_PREFERRED_LIFETIME 14400

#define DEFAULT_DNS_LIFETIME_MULT 3


struct radv_config
{
  struct proto_config c;
  list patt_list;		/* List of iface configs (struct radv_iface_config) */
  list pref_list;		/* Global list of prefix configs (struct radv_prefix_config) */
  list rdnss_list;		/* Global list of RDNSS configs (struct radv_rdnss_config) */
  list dnssl_list;		/* Global list of DNSSL configs (struct radv_dnssl_config) */

  ip_addr trigger_prefix;	/* Prefix of a trigger route, if defined */
  u8 trigger_pxlen;		/* Pxlen of a trigger route, if defined */
  u8 trigger_valid;		/* Whether a trigger route is defined */
  u8 propagate_routes;		/* Do we propagate more specific routes (RFC 4191)? */
  u32 max_linger_time;		/* Maximum of interface route_linger_time */
};

struct radv_iface_config
{
  struct iface_patt i;
  list pref_list;		/* Local list of prefix configs (struct radv_prefix_config) */
  list rdnss_list;		/* Local list of RDNSS configs (struct radv_rdnss_config) */
  list dnssl_list;		/* Local list of DNSSL configs (struct radv_dnssl_config) */

  u32 min_ra_int;		/* Standard options from RFC 4861 */
  u32 max_ra_int;
  u32 min_delay;

  u32 prefix_linger_time;	/* How long we advertise dead prefixes with lifetime 0 */
  u32 route_linger_time;	/* How long we advertise dead routes with lifetime 0 */

  u8 rdnss_local;		/* Global list is not used for RDNSS */
  u8 dnssl_local;		/* Global list is not used for DNSSL */

  u8 managed;			/* Standard options from RFC 4861 */
  u8 other_config;
  u32 link_mtu;
  u32 reachable_time;
  u32 retrans_timer;
  u32 current_hop_limit;
  u32 default_lifetime;
  u32 route_lifetime;		/* Lifetime for the RFC 4191 routes */
  u8 default_lifetime_sensitive; /* Whether default_lifetime depends on trigger */
  u8 route_lifetime_sensitive;	/* Whether route_lifetime depends on trigger */
  u8 default_preference;	/* Default Router Preference (RFC 4191) */
  u8 route_preference;		/* Specific Route Preference (RFC 4191) */
};

struct radv_prefix_config
{
  node n;
  ip_addr prefix;
  uint pxlen;

  u8 skip;			/* Do not include this prefix to RA */
  u8 onlink;			/* Standard options from RFC 4861 */
  u8 autonomous;
  u32 valid_lifetime;
  u32 preferred_lifetime;
  u8 valid_lifetime_sensitive;	 /* Whether valid_lifetime depends on trigger */
  u8 preferred_lifetime_sensitive; /* Whether preferred_lifetime depends on trigger */
};

struct radv_rdnss_config
{
  node n;
  u32 lifetime;			/* Valid if lifetime_mult is 0 */
  u16 lifetime_mult;		/* Lifetime specified as multiple of max_ra_int */
  ip_addr server;		/* IP address of recursive DNS server */
};

struct radv_dnssl_config
{
  node n;
  u32 lifetime;			/* Valid if lifetime_mult is 0 */
  u16 lifetime_mult;		/* Lifetime specified as multiple of max_ra_int */
  u8 dlen_first;		/* Length of first label in domain */
  u8 dlen_all;			/* Both dlen_ filled in radv_process_domain() */
  char *domain;			/* Domain for DNS search list, in processed form */
};

/*
 * One more specific route as per RFC 4191.
 *
 * Note that it does *not* contain the next hop field. The next hop is always
 * the router sending the advertisment and the more specific route only allows
 * overriding the preference of the route.
 */
struct radv_route
{
  struct fib_node n;
  u32 lifetime;			/* Lifetime from an attribute */
  u8 lifetime_set;		/* Whether lifetime is defined */
  u8 preference;		/* Preference of the route, RA_PREF_* */
  u8 preference_set;		/* Whether preference is defined */
  u8 valid;			/* Whethe route is valid or withdrawn */
  bird_clock_t changed;		/* Last time when the route changed */
};

struct radv_proto
{
  struct proto p;
  list iface_list;		/* List of active ifaces */
  u8 valid;			/* Router is valid for forwarding, used for shutdown */
  u8 active;			/* Whether radv is active w.r.t. triggers */
  u8 fib_up;			/* FIB table (routes) is initialized */
  struct fib routes;		/* FIB table of specific routes (struct radv_route) */
  bird_clock_t prune_time;	/* Next time of route table pruning */
};

struct radv_prefix		/* One prefix we advertise */
{
  node n;
  ip_addr prefix;
  u8 len;
  u8 valid;			/* Is the prefix valid? If not, we advertise it
				   with 0 lifetime, so clients stop using it */
  u8 mark;			/* A temporary mark for processing */
  bird_clock_t changed;		/* Last time when the prefix changed */
  struct radv_prefix_config *cf; /* The config tied to this prefix */
};

struct radv_iface
{
  node n;
  struct radv_proto *ra;
  struct radv_iface_config *cf;	/* Related config, must be updated in reconfigure */
  struct iface *iface;
  struct ifa *addr;		/* Link-local address of iface */
  struct pool *pool;		/* A pool for interface-specific things */
  list prefixes;		/* The prefixes we advertise (struct radv_prefix) */
  bird_clock_t prune_time;	/* Next time of prefix list pruning */
  bird_clock_t valid_time;	/* Cached packet is valid until first linger timeout */

  timer *timer;
  struct object_lock *lock;
  sock *sk;

  bird_clock_t last;		/* Time of last sending of RA */
  u16 plen;			/* Length of prepared RA in tbuf, or 0 if not valid */
  byte initial;			/* How many RAs are still to be sent as initial */
};

#define RA_EV_INIT 1		/* Switch to initial mode */
#define RA_EV_CHANGE 2		/* Change of options or prefixes */
#define RA_EV_RS 3		/* Received RS */

/* Default Router Preferences (RFC 4191) */
#define RA_PREF_LOW	0x18
#define RA_PREF_MEDIUM	0x00
#define RA_PREF_HIGH	0x08
#define RA_PREF_MASK	0x18

/* Attributes */
#define EA_RA_PREFERENCE	EA_CODE(EAP_RADV, 0)
#define EA_RA_LIFETIME		EA_CODE(EAP_RADV, 1)

#ifdef LOCAL_DEBUG
#define RADV_FORCE_DEBUG 1
#else
#define RADV_FORCE_DEBUG 0
#endif
#define RADV_TRACE(flags, msg, args...) do { if ((p->p.debug & flags) || RADV_FORCE_DEBUG) \
	log(L_TRACE "%s: " msg, p->p.name , ## args ); } while(0)


/* radv.c */
void radv_iface_notify(struct radv_iface *ifa, int event);

/* packets.c */
int radv_process_domain(struct radv_dnssl_config *cf);
void radv_send_ra(struct radv_iface *ifa);
int radv_sk_open(struct radv_iface *ifa);



#endif /* _BIRD_RADV_H_ */
