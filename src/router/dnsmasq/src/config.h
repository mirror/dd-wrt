/* dnsmasq is Copyright (c) 2000-2006 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/

#define VERSION "2.33"

#define FTABSIZ 150 /* max number of outstanding requests (default) */
#define MAX_PROCS 20 /* max no children for TCP requests */
#define CHILD_LIFETIME 150 /* secs 'till terminated (RFC1035 suggests > 120s) */
#define EDNS_PKTSZ 1280 /* default max EDNS.0 UDP packet from RFC2671 */
#define TIMEOUT 20 /* drop UDP queries after TIMEOUT seconds */
#define LEASE_RETRY 60 /* on error, retry writing leasefile after LEASE_RETRY seconds */
#define LOGRATE 120 /* log table overflows every LOGRATE seconds */
#define CACHESIZ 150 /* default cache size */
#define MAXLEASES 150 /* maximum number of DHCP leases */
#define PING_WAIT 3 /* wait for ping address-in-use test */
#define PING_CACHE_TIME 30 /* Ping test assumed to be valid this long. */
#define DECLINE_BACKOFF 600 /* disable DECLINEd static addresses for this long */
#define DHCP_PACKET_MAX 16384 /* hard limit on DHCP packet size */
#define SMALLDNAME 40 /* most domain names are smaller than this */
#define HOSTSFILE "/etc/hosts"
#define ETHERSFILE "/etc/ethers"
#ifdef __uClinux__
#  define RESOLVFILE "/etc/config/resolv.conf"
#else
#  define RESOLVFILE "/etc/resolv.conf"
#endif
#define RUNFILE "/var/run/dnsmasq.pid"
#if defined(__FreeBSD__) || defined (__OpenBSD__)
#   define LEASEFILE "/var/db/dnsmasq.leases"
#else
#   define LEASEFILE "/var/lib/misc/dnsmasq.leases"
#endif
#if defined(__FreeBSD__)
#   define CONFFILE "/usr/local/etc/dnsmasq.conf"
#else
#   define CONFFILE "/etc/dnsmasq.conf"
#endif
#define DEFLEASE 3600 /* default lease time, 1 hour */
#define CHUSER "nobody"
#define CHGRP "dip"
#define DHCP_SERVER_PORT 67
#define DHCP_CLIENT_PORT 68

/* DBUS interface specifics */
#define DNSMASQ_SERVICE "uk.org.thekelleys.dnsmasq"
#define DNSMASQ_PATH "/uk/org/thekelleys/dnsmasq"

/* A small collection of RR-types which are missing on some platforms */

#ifndef T_SRV
#  define T_SRV 33
#endif

#ifndef T_OPT
#  define T_OPT 41
#endif

/* Get linux C library versions. */
#if defined(__linux__) && !defined(__UCLIBC__) && !defined(__uClinux__)
/*#  include <libio.h> */
#  include <features.h> 
#endif


/* Follows system specific switches. If you run on a 
   new system, you may want to edit these. 
   May replace this with Autoconf one day. 

HAVE_LINUX_NETWORK
   define this to do networking the Linux way. When it's defined, the code will
   use IP_PKTINFO, Linux capabilities and the RTnetlink system. If it's not defined,
   a few facilities will be lost, namely support for multiple addresses on an interface,
   DNS query retransmission, and (on some systems) wildcard interface binding.

HAVE_BROKEN_RTC
   define this on embedded systems which don't have an RTC
   which keeps time over reboots. Causes dnsmasq to use uptime
   for timing, and keep lease lengths rather than expiry times
   in its leases file. This also make dnsmasq "flash disk friendly".
   Normally, dnsmasq tries very hard to keep the on-disk leases file
   up-to-date: rewriting it after every renewal.  When HAVE_BROKEN_RTC 
   is in effect, the lease file is only written when a new lease is 
   created, or an old one destroyed. (Because those are the only times 
   it changes.) This vastly reduces the number of file writes, and makes
   it viable to keep the lease file on a flash filesystem.
   NOTE: when enabling or disabling this, be sure to delete any old
   leases file, otherwise dnsmasq may get very confused.

HAVE_ISC_READER 
   define this to include the old ISC dhcpcd integration. Note that you cannot
   set both HAVE_ISC_READER and HAVE_BROKEN_RTC.

HAVE_GETOPT_LONG
   define this if you have GNU libc or GNU getopt. 

HAVE_ARC4RANDOM
   define this if you have arc4random() to get better security from DNS spoofs
   by using really random ids (OpenBSD) 

HAVE_RANDOM
   define this if you have the 4.2BSD random() function (and its
   associated srandom() function), which is at least as good as (if not
   better than) the rand() function.

HAVE_DEV_RANDOM
   define this if you have the /dev/random device, which gives truly
   random numbers but may run out of random numbers.

HAVE_DEV_URANDOM
   define this if you have the /dev/urandom device, which gives
   semi-random numbers when it runs out of truly random numbers.

HAVE_SOCKADDR_SA_LEN
   define this if struct sockaddr has sa_len field (*BSD) 

HAVE_DBUS
   Define this if you want to link against libdbus, and have dnsmasq
   define some methods to allow (re)configuration of the upstream DNS 
   servers via DBus.

NOTES:
   For Linux you should define 
      HAVE_LINUX_NETWORK
      HAVE_GETOPT_LONG
      HAVE_RANDOM
      HAVE_DEV_RANDOM
      HAVE_DEV_URANDOM
  you should NOT define 
      HAVE_ARC4RANDOM
      HAVE_SOCKADDR_SA_LEN

   For *BSD systems you should define 
     HAVE_SOCKADDR_SA_LEN
     HAVE_RANDOM
   you should NOT define  
     HAVE_LINUX_NETWORK 
   and you MAY define  
     HAVE_ARC4RANDOM - OpenBSD and FreeBSD and NetBSD version 2.0 or later
     HAVE_DEV_URANDOM - OpenBSD and FreeBSD and NetBSD
     HAVE_DEV_RANDOM - FreeBSD  and NetBSD 
                       (OpenBSD with hardware random number generator)
     HAVE_GETOPT_LONG - NetBSD, later FreeBSD 
                       (FreeBSD and OpenBSD only if you link GNU getopt) 

*/

/* platform independent options- uncomment to enable */
/* #define HAVE_BROKEN_RTC */
/* #define HAVE_ISC_READER */
/* #define HAVE_DBUS */

#if defined(HAVE_BROKEN_RTC) && defined(HAVE_ISC_READER)
#  error HAVE_ISC_READER is not compatible with HAVE_BROKEN_RTC
#endif

/* platform dependent options. */

/* Must preceed __linux__ since uClinux defines __linux__ too. */
#if defined(__uClinux__)
#define HAVE_LINUX_NETWORK
#define HAVE_GETOPT_LONG
#undef HAVE_ARC4RANDOM
#define HAVE_RANDOM
#define HAVE_DEV_URANDOM
#define HAVE_DEV_RANDOM
#undef HAVE_SOCKADDR_SA_LEN
/* Never use fork() on uClinux. Note that this is subtly different from the
   --keep-in-foreground option, since it also  suppresses forking new 
   processes for TCP connections. It's intended for use on MMU-less kernels. */
#define NO_FORK

#elif defined(__UCLIBC__)
#define HAVE_LINUX_NETWORK
#if defined(__UCLIBC_HAS_GNU_GETOPT__) || \
   ((__UCLIBC_MAJOR__==0) && (__UCLIBC_MINOR__==9) && (__UCLIBC_SUBLEVEL__<21))
#    define HAVE_GETOPT_LONG
#else
#    undef HAVE_GETOPT_LONG
#endif
#undef HAVE_ARC4RANDOM
#define HAVE_RANDOM
#define HAVE_DEV_URANDOM
#define HAVE_DEV_RANDOM
#undef HAVE_SOCKADDR_SA_LEN
#if !defined(__ARCH_HAS_MMU__) && !defined(__UCLIBC_HAS_MMU__)
#  define NO_FORK
#endif
#if defined(__UCLIBC_HAS_IPV6__)
#  ifndef IPV6_V6ONLY
#    define IPV6_V6ONLY 26
#  endif
#endif

/* This is for glibc 2.x */
#elif defined(__linux__)
#define HAVE_LINUX_NETWORK
#define HAVE_GETOPT_LONG
#undef HAVE_ARC4RANDOM
#define HAVE_RANDOM
#define HAVE_DEV_URANDOM
#define HAVE_DEV_RANDOM
#undef HAVE_SOCKADDR_SA_LEN
/* glibc < 2.2  has broken Sockaddr_in6 so we have to use our own. */
/* glibc < 2.2 doesn't define in_addr_t */
#if defined(__GLIBC__) && (__GLIBC__ == 2) && \
    defined(__GLIBC_MINOR__) && (__GLIBC_MINOR__ < 2)
typedef unsigned long in_addr_t; 
#   define HAVE_BROKEN_SOCKADDR_IN6
#endif

#elif defined(__FreeBSD__) || defined(__OpenBSD__)
#undef HAVE_LINUX_NETWORK
/* Later verions of FreeBSD have getopt_long() */
#if defined(optional_argument) && defined(required_argument)
#   define HAVE_GETOPT_LONG
#else
#   undef HAVE_GETOPT_LONG
#endif
#define HAVE_ARC4RANDOM
#define HAVE_RANDOM
#define HAVE_DEV_URANDOM
#define HAVE_SOCKADDR_SA_LEN

#elif defined(__APPLE__)
#undef HAVE_LINUX_NETWORK
#undef HAVE_GETOPT_LONG
#define HAVE_ARC4RANDOM
#define HAVE_RANDOM
#define HAVE_DEV_URANDOM
#define HAVE_SOCKADDR_SA_LEN
/* Define before sys/socket.h is included so we get socklen_t */
#define _BSD_SOCKLEN_T_
/* This is not defined in Mac OS X arpa/nameserv.h */
#define IN6ADDRSZ 16
 
#elif defined(__NetBSD__)
#undef HAVE_LINUX_NETWORK
#define HAVE_GETOPT_LONG
#undef HAVE_ARC4RANDOM
#define HAVE_RANDOM
#define HAVE_DEV_URANDOM
#define HAVE_DEV_RANDOM
#define HAVE_SOCKADDR_SA_LEN
 
#endif
/* Decide if we're going to support IPv6 */
/* We assume that systems which don't have IPv6
   headers don't have ntop and pton either */

#if defined(INET6_ADDRSTRLEN) && defined(IPV6_V6ONLY) && !defined(NO_IPV6)
#  define HAVE_IPV6
#  define ADDRSTRLEN INET6_ADDRSTRLEN
#  if defined(SOL_IPV6)
#    define IPV6_LEVEL SOL_IPV6
#  else
#    define IPV6_LEVEL IPPROTO_IPV6
#  endif
#elif defined(INET_ADDRSTRLEN)
#  undef HAVE_IPV6
#  define ADDRSTRLEN INET_ADDRSTRLEN
#else
#  undef HAVE_IPV6
#  define ADDRSTRLEN 16 /* 4*3 + 3 dots + NULL */
#endif

