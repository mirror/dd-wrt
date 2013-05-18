#ifndef __HEADERS_H__
#define __HEADERS_H__

#ifndef __need_sig_atomic_t
#define __need_sig_atomic_t 1
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef  HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef  HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef  HAVE_SYS_FILE_H
#include <sys/file.h>
#endif

#ifdef  HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef  HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

#ifdef  HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#ifdef  HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_NET_IF_H
#include <net/if.h>
#endif

#ifdef HAVE_NET_ROUTE_H
#include <net/route.h>
#endif

#ifdef HAVE_IFADDRS_H
#include <ifaddrs.h>
#endif

#ifdef HAVE_NET_ETHERNET_H
#include <net/ethernet.h>
#endif
#ifdef HAVE_NETINET_IN_SYSTM_H
#include <netinet/in_systm.h>
#endif

#ifdef  HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifdef  HAVE_NETINET_IP_H
#include <netinet/ip.h>
#endif

#ifdef  HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif

#ifdef  HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#ifdef HAVE_NETINET_IF_ETHER_H
#include <netinet/if_ether.h>
#endif

#ifdef  HAVE_NETDB_H
#include <netdb.h>
#endif

#ifdef  HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef  HAVE_LIBGEN_H
#include <libgen.h>   /* for dirname(3) */
#else
#define dirname(foo)  (foo) /* Fine approximation. */
#endif

#ifdef  HAVE_SYSEXITS_H
#include <sysexits.h>
#endif

#ifdef  HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef  HAVE_PTHREAD_H
#include <pthread.h>
#endif

#ifdef  HAVE_PATHS_H
#include <paths.h>
#endif

#include <poll.h>
#include <signal.h>
#include <fnmatch.h>

#include <assert.h>

#ifndef IPPROTO_SCTP
#define IPPROTO_SCTP  132 /* Stream Control Transmission Protocol */
#endif  /* IPPROTO_SCTP */

#ifndef _PATH_DEVNULL
#define _PATH_DEVNULL "/dev/null"
#endif  /* _PATH_DEVNULL */

#ifndef ETHER_HDR_LEN
#define ETHER_ADDR_LEN 6
#define ETHER_TYPE_LEN 2
#define ETHER_HDR_LEN (ETHER_ADDR_LEN*2+ETHER_TYPE_LEN)
#endif  /* ETHER_HDR_LEN */

#ifdef  HAVE_ASM_TYPES_H
#include <asm/types.h>
#endif

#ifdef  HAVE_LINUX_SOCKET_H
#include <linux/socket.h>
#endif

#ifdef  HAVE_LINUX_NETLINK_H
#include <linux/netlink.h>
#endif

#ifdef  HAVE_LINUX_NETFILTER_H
#include <linux/netfilter.h>
#endif

#if defined(HAVE_IFADDRS_H) && defined(HAVE_GETIFADDRS)
#define IPCAD_IFLIST_USE_GETIFADDRS
#endif

#include <pcap.h>

#ifndef HAVE_SOCKLEN_T
typedef size_t  socklen_t;
#endif

#ifndef HAVE_STRTOULL
#ifdef  HAVE_STRTOUQ
#define strtoull(a,b,c) strtouq(a,b,c)
#else /* HAVE_STRTOUQ */
#warning Neither strtoull() nor strtouq() functions are present!
#warning Byte count type width limited to "long" width.
#define strtoull(a,b,c) strtoul(a,b,c)
#endif  /* HAVE_STRTOUQ */
#endif  /* HAVE_SRTOULL */
#ifndef HAVE_INET_ATON
#ifdef  HAVE_INET_PTON
#define inet_aton(a,b)  inet_pton(AF_INET, a, b)
#else /* HAVE_INET_PTON */
#error  Neither inet_aton() nor inet_pton() functions are present!
#endif  /* HAVE_INET_PTON */
#endif  /* HAVE_INET_ATON */

#ifndef offsetof
#define offsetof(T, m)  (((void *)&(((T *)0)->m)) - (void *)0)
#endif  /* offsetof */

#ifndef HAVE_PTHREAD_CANCEL
#define pthread_cancel(foo)   ((int)0)
#define pthread_setcancelstate(foo,bar) ((void)0)
#endif  /* HAVE_PTHREAD_CANCEL */

#ifndef MIN
#define MIN(a,b)  ((a)<(b)?(a):(b))
#endif  /* MIN */

#endif  /* __HEADERS_H__ */
