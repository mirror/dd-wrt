/* Generic kernel multicast routing API for Linux and *BSD */
#ifndef SMCROUTE_MROUTE_H_
#define SMCROUTE_MROUTE_H_

#include "config.h"
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>		/* Defines u_char, needed by netinet/in.h */
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#ifdef HAVE_NETINET_IN_VAR_H
#include <netinet/in_var.h>
#endif
#include <netinet/ip.h>
#include "queue.h"		/* Needed by netinet/ip_mroute.h on FreeBSD */

#ifdef HAVE_LINUX_MROUTE_H
#define _LINUX_IN_H             /* For Linux <= 2.6.25 */
#include <linux/types.h>
#include <linux/mroute.h>
#endif

#ifdef HAVE_LINUX_MROUTE6_H
#include <linux/mroute6.h>
#endif

#ifdef HAVE_LINUX_FILTER_H
#include <linux/filter.h>
#endif

#ifdef HAVE_NET_ROUTE_H
#include <net/route.h>
#endif

#ifdef HAVE_NETINET_IP_MROUTE_H
#define _KERNEL
#include <netinet/ip_mroute.h>
#undef _KERNEL
#else
# ifdef __APPLE__
#  include "ip_mroute.h"
# endif
#endif

#ifdef HAVE_NETINET6_IP6_MROUTE_H
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#include <netinet6/ip6_mroute.h>
#endif

#ifndef IN6_IS_ADDR_MULTICAST
#define IN6_IS_ADDR_MULTICAST(a) (((__const uint8_t *) (a))[0] == 0xff)
#endif

#include "inet.h"

/*
 * IPv4 multicast route
 */
#ifndef MAXVIFS
#define MAXVIFS 32
#endif

/*
 * IPv6 multicast route
 */
#ifdef HAVE_IPV6_MULTICAST_ROUTING
# ifndef MAXMIFS
#  define MAXMIFS MAXVIFS
# endif

/* Allocate data types to the max, on FreeBSD MAXMIFS > MAXVIFS */
# if MAXMIFS > MAXVIFS
#  define MAX_MC_VIFS MAXMIFS
# else
#  define MAX_MC_VIFS MAXVIFS
# endif
#else
#define MAX_MC_VIFS MAXVIFS
#endif

/* We're on a system w/o IPv6 routing, define to avoid other ifdefs */
#ifndef mifi_t
typedef unsigned short mifi_t;
#endif

struct mroute {
	TAILQ_ENTRY(mroute) link;
	int            unused;

	inet_addr_t    source;		/* originating host, may be inet_anyaddr() */
	short	       src_len;		/* source prefix len, or 0:disabled */

	inet_addr_t    group;		/* multicast group */
	short	       len;		/* prefix len, or 0:disabled */

	vifi_t         inbound;		/* incoming VIF	   */
	uint8_t	       ttl[MAX_MC_VIFS];/* outgoing VIFs   */


	unsigned long  valid_pkt;	/* packet counter at last mroute4_dyn_expire() */
	time_t	       last_use;	/* timestamp of last forwarded packet */
};

int  mroute_init       (int do_vifs, int table_id, int cache_tmo);
void mroute_exit       (void);

int  mroute_add_vif    (char *ifname, uint8_t mrdisc, uint8_t threshold);
int  mroute_del_vif    (char *ifname);

void mroute_expire     (int max_idle);

int  mroute_add_route  (struct mroute *mroute);
int  mroute_del_route  (struct mroute *mroute);

void mroute_reload_beg (void);
void mroute_reload_end (int do_vifs);

int  mroute_show       (int sd, int detail);

#endif /* SMCROUTE_MROUTE_H_ */
