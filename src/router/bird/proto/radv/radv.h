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

#define AllNodes   _MI(0xFF020000, 0, 0, 1)	/* FF02::1 */
#define AllRouters _MI(0xFF020000, 0, 0, 2)	/* FF02::2 */

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
};

struct radv_iface_config
{
  struct iface_patt i;
  list pref_list;		/* Local list of prefix configs (struct radv_prefix_config) */
  list rdnss_list;		/* Local list of RDNSS configs (struct radv_rdnss_config) */
  list dnssl_list;		/* Local list of DNSSL configs (struct radv_dnssl_config) */

  u32 min_ra_int;		/* Standard options from RFC 4261 */
  u32 max_ra_int;
  u32 min_delay;

  u8 rdnss_local;		/* Global list is not used for RDNSS */
  u8 dnssl_local;		/* Global list is not used for DNSSL */

  u8 managed;			/* Standard options from RFC 4261 */
  u8 other_config;
  u32 link_mtu;
  u32 reachable_time;
  u32 retrans_timer;
  u32 current_hop_limit;
  u32 default_lifetime;
  u8 default_lifetime_sensitive; /* Whether default_lifetime depends on trigger */
};

struct radv_prefix_config
{
  node n;
  ip_addr prefix;
  int pxlen;

  u8 skip;			/* Do not include this prefix to RA */
  u8 onlink;			/* Standard options from RFC 4261 */
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


struct proto_radv
{
  struct proto p;
  list iface_list;		/* List of active ifaces */
  u8 active;			/* Whether radv is active w.r.t. triggers */
};

struct radv_iface
{
  node n;
  struct proto_radv *ra;
  struct radv_iface_config *cf;	/* Related config, must be updated in reconfigure */
  struct iface *iface;
  struct ifa *addr;		/* Link-local address of iface */

  timer *timer;
  struct object_lock *lock;
  sock *sk;

  bird_clock_t last;		/* Time of last sending of RA */
  u16 plen;			/* Length of prepared RA in tbuf, or 0 if not valid */
  byte initial;			/* List of active ifaces */
};

#define RA_EV_INIT 1		/* Switch to initial mode */
#define RA_EV_CHANGE 2		/* Change of options or prefixes */
#define RA_EV_RS 3		/* Received RS */



#ifdef LOCAL_DEBUG
#define RADV_FORCE_DEBUG 1
#else
#define RADV_FORCE_DEBUG 0
#endif
#define RADV_TRACE(flags, msg, args...) do { if ((ra->p.debug & flags) || RADV_FORCE_DEBUG) \
	log(L_TRACE "%s: " msg, ra->p.name , ## args ); } while(0)


/* radv.c */
void radv_iface_notify(struct radv_iface *ifa, int event);

/* packets.c */
int radv_process_domain(struct radv_dnssl_config *cf);
void radv_send_ra(struct radv_iface *ifa, int shutdown);
int radv_sk_open(struct radv_iface *ifa);



#endif /* _BIRD_RADV_H_ */
