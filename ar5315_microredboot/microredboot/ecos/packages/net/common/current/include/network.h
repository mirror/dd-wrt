
//==========================================================================
//
//      include/network.h
//
//      Misc network support
//
//==========================================================================
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD or other sources,
// and are covered by the appropriate copyright disclaimers included herein.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas, andrew.lunn@ascom.ch
// Date:         2000-01-10
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#ifndef _NETWORK_H_
#define _NETWORK_H_

#define NO_LIBKERN_INLINE  // Avoid kernel inline functions

#include <pkgconf/system.h>
#include <pkgconf/net.h>
#include <pkgconf/io_eth_drivers.h>
#undef _KERNEL
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/errno.h>

#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <net/route.h>

#include <cyg/infra/diag.h>
#include <cyg/kernel/kapi.h>

#include <netdb.h>
#include <bootp.h>

#ifdef CYGHWR_NET_DRIVER_ETH0
extern struct bootp eth0_bootp_data;
extern cyg_bool_t   eth0_up;
extern const char  *eth0_name;
#endif
#ifdef CYGHWR_NET_DRIVER_ETH1
extern struct bootp eth1_bootp_data;
extern cyg_bool_t   eth1_up;
extern const char  *eth1_name;
#endif

__externC void init_all_network_interfaces(void);

__externC void     cyg_route_reinit(void);
__externC void     perror(const char *) __THROW;
__externC int      close(int);
__externC ssize_t  read(int, void *, size_t);
__externC ssize_t  write(int, const void *, size_t);
__externC int      select(int, fd_set *, fd_set *, fd_set *, struct timeval *tv);

// This API is for our own automated network tests.
// It's not at all supported.
#define CYG_NET_GET_MEM_STATS_MISC  0 // Misc mpool
#define CYG_NET_GET_MEM_STATS_MBUFS 1 // Mbufs pool
#define CYG_NET_GET_MEM_STATS_CLUST 2 // Clust pool
int cyg_net_get_mem_stats( int which, cyg_mempool_info *p );

#ifdef CYGPKG_NET_INET6
#ifdef CYGOPT_NET_IPV6_ROUTING_THREAD 
__externC void ipv6_start_routing_thread(void);
__externC int cyg_net_get_ipv6_advrouter(struct sockaddr_in6 * addr);
#endif
#endif

#endif // _NETWORK_H_
