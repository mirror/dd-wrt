
/*
 * lib/pathnames.h    This file contains the definitions of the path 
 *                      names used by the NET-LIB.
 *
 * NET-LIB      
 *
 * Version:     lib/pathnames.h 1.37 (1997-08-23)
 *
 * Author:      Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
 */

/* pathnames of the procfs files used by NET. */
#define _PATH_PROCNET_IGMP		"/proc/net/igmp"
#define _PATH_PROCNET_IGMP6		"/proc/net/igmp6"
#define _PATH_PROCNET_TCP		"/proc/net/tcp"
#define _PATH_PROCNET_TCP6		"/proc/net/tcp6"
#define _PATH_PROCNET_UDP		"/proc/net/udp"
#define _PATH_PROCNET_UDP6		"/proc/net/udp6"
#define _PATH_PROCNET_RAW		"/proc/net/raw"
#define _PATH_PROCNET_RAW6		"/proc/net/raw6"
#define _PATH_PROCNET_UNIX		"/proc/net/unix"
#define _PATH_PROCNET_ROUTE		"/proc/net/route"
#define _PATH_PROCNET_ROUTE6		"/proc/net/ipv6_route"
#define _PATH_PROCNET_RTCACHE		"/proc/net/rt_cache"
#define _PATH_PROCNET_AX25_ROUTE	"/proc/net/ax25_route"
#define _PATH_PROCNET_NR		"/proc/net/nr"
#define _PATH_PROCNET_NR_NEIGH		"/proc/net/nr_neigh"
#define _PATH_PROCNET_NR_NODES		"/proc/net/nr_nodes"
#define _PATH_PROCNET_ARP		"/proc/net/arp"
#define _PATH_PROCNET_AX25		"/proc/net/ax25"
#define _PATH_PROCNET_IPX		"/proc/net/ipx"
#define _PATH_PROCNET_IPX_ROUTE		"/proc/net/ipx_route"
#define _PATH_PROCNET_ATALK		"/proc/net/appletalk"
#define _PATH_PROCNET_IP_BLK		"/proc/net/ip_block"
#define _PATH_PROCNET_IP_FWD		"/proc/net/ip_forward"
#define _PATH_PROCNET_IP_ACC		"/proc/net/ip_acct"
#define _PATH_PROCNET_IP_MASQ		"/proc/net/ip_masquerade"
#define _PATH_PROCNET_NDISC		"/proc/net/ndisc"
#define _PATH_PROCNET_IFINET6		"/proc/net/if_inet6"
#define _PATH_PROCNET_DEV		"/proc/net/dev"
#define _PATH_PROCNET_RARP		"/proc/net/rarp"
#define _PATH_ETHERS			"/etc/ethers"
#define _PATH_PROCNET_ROSE_ROUTE	"/proc/net/rose_routes"
#define _PATH_PROCNET_X25              "/proc/net/x25"
#define _PATH_PROCNET_X25_ROUTE                "/proc/net/x25_routes"
#define _PATH_PROCNET_DEV_MCAST		"/proc/net/dev_mcast"

/* pathname for the netlink device */
#define _PATH_DEV_ROUTE	"/dev/route"

/* End of pathnames.h */
