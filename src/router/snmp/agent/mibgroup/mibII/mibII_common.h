#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#include <sys/types.h>

#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_SYS_SYSMP_H
#include <sys/sysmp.h>
#endif
#ifdef HAVE_SYS_TCPIPSTATS_H
#include <sys/tcpipstats.h>
#endif
#if defined(NETSNMP_IFNET_NEEDS_KERNEL) && !defined(_KERNEL)
#define _KERNEL 1
#define _I_DEFINED_KERNEL
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NET_IF_H
#ifdef _I_DEFINED_KERNEL
#undef _KERNEL
#endif
#include <net/if.h>
#ifdef _I_DEFINED_KERNEL
#define _KERNEL 1
#endif
#endif
#ifdef HAVE_NET_IF_VAR_H
#include <net/if_var.h>
#endif
#ifdef _I_DEFINED_KERNEL
#undef _KERNEL
#endif

#ifdef HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif
#ifdef HAVE_SYS_STREAM_H
#include <sys/stream.h>
#endif
#ifdef HAVE_NET_ROUTE_H
#include <net/route.h>
#endif
#ifdef HAVE_NETINET_IN_SYSTM_H
#include <netinet/in_systm.h>
#endif
#ifdef HAVE_NETINET_IP_H
#include <netinet/ip.h>
#endif
#ifdef HAVE_SYS_QUEUE_H
#include <sys/queue.h>
#endif
/* IRIX 6.5 build breaks on sys/socketvar.h because _KMEMUSER brings in 
   sys/pda.h which doesn't compile */
#ifndef irix6
#ifdef HAVE_SYS_SOCKETVAR_H
#include <sys/socketvar.h>
#endif
#endif  /* irix6 */
#ifdef HAVE_NETINET_IP_VAR_H
#include <netinet/ip_var.h>
#endif
#ifdef NETSNMP_ENABLE_IPV6
#ifdef HAVE_NETNETSNMP_ENABLE_IPV6_IP6_VAR_H
#include <netinet6/ip6_var.h>
#endif
#endif
#ifdef HAVE_NETINET_IN_PCB_H
#include <netinet/in_pcb.h>
#endif
#ifdef HAVE_INET_MIB2_H
#include <inet/mib2.h>
#endif
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#ifdef solaris2
#include "kernel_sunos5.h"
#else
#include "kernel.h"
#endif
#ifdef linux
#include "kernel_linux.h"
#endif
#ifdef NETBSD_STATS_VIA_SYSCTL
#include "kernel_netbsd.h"
#endif
#if defined(NETSNMP_CAN_USE_SYSCTL) /* && defined(ICMPCTL_STATS) */
#include "kernel_sysctl.h"
#endif

	/* or MIB_xxxCOUNTER_SYMBOL || hpux11 */
#ifdef hpux
#include <sys/mib.h>
#include <netinet/mib_kern.h>
#endif

#ifdef cygwin
#include <windows.h>
#endif
