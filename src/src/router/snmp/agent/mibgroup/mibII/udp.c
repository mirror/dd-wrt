/*
 *  UDP MIB group implementation - udp.c
 *
 */

#include <net-snmp/net-snmp-config.h>

#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#include <sys/types.h>
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif

#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_SYS_SYSMP_H
#include <sys/sysmp.h>
#endif
#if HAVE_SYS_TCPIPSTATS_H
#include <sys/tcpipstats.h>
#endif
#if defined(IFNET_NEEDS_KERNEL) && !defined(_KERNEL)
#define _KERNEL 1
#define _I_DEFINED_KERNEL
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_NET_IF_H
#include <net/if.h>
#endif
#if HAVE_NET_IF_VAR_H
#include <net/if_var.h>
#endif
#ifdef _I_DEFINED_KERNEL
#undef _KERNEL
#endif

#if HAVE_SYS_STREAM_H
#include <sys/stream.h>
#endif
#if HAVE_NET_ROUTE_H
#include <net/route.h>
#endif
#if HAVE_NETINET_IN_SYSTM_H
#include <netinet/in_systm.h>
#endif
#if HAVE_NETINET_IP_H
#include <netinet/ip.h>
#endif
#if HAVE_SYS_QUEUE_H
#include <sys/queue.h>
#endif
#if HAVE_SYS_SOCKETVAR_H
#include <sys/socketvar.h>
#endif
#if HAVE_NETINET_IP_VAR_H
#include <netinet/ip_var.h>
#endif
#ifdef INET6
#if HAVE_NETINET6_IP6_VAR_H
#include <netinet6/ip6_var.h>
#endif
#endif
#if HAVE_NETINET_IN_PCB_H
#include <netinet/in_pcb.h>
#endif
#ifdef HAVE_NETINET_UDP_H
#include <netinet/udp.h>
#endif
#if HAVE_NETINET_UDP_VAR_H
#include <netinet/udp_var.h>
#endif
#if HAVE_INET_MIB2_H
#include <inet/mib2.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/auto_nlist.h>

#include "util_funcs.h"

#ifdef solaris2
#include "kernel_sunos5.h"
#else
#include "kernel.h"
#endif

#ifdef linux
#include "kernel_linux.h"
#endif

#ifdef cygwin
#define WIN32
#include <windows.h>
#endif

#ifdef hpux
#include <sys/mib.h>
#include <netinet/mib_kern.h>
#endif                          /* hpux */

#ifdef linux
#include "tcp.h"
#endif
#include "udp.h"
#include "udpTable.h"
#include "sysORTable.h"

#ifdef CAN_USE_SYSCTL
#include <sys/sysctl.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#ifndef MIB_STATS_CACHE_TIMEOUT
#define MIB_STATS_CACHE_TIMEOUT	5
#endif
#ifndef UDP_STATS_CACHE_TIMEOUT
#define UDP_STATS_CACHE_TIMEOUT	MIB_STATS_CACHE_TIMEOUT
#endif
marker_t        udp_stats_cache_marker = NULL;

        /*********************
	 *
	 *  Kernel & interface information,
	 *   and internal forward declarations
	 *
	 *********************/


        /*********************
	 *
	 *  Initialisation & common implementation functions
	 *
	 *********************/

struct variable3 udp_variables[] = {
    {UDPINDATAGRAMS, ASN_COUNTER, RONLY, var_udp, 1, {1}},
    {UDPNOPORTS, ASN_COUNTER, RONLY, var_udp, 1, {2}},
    {UDPINERRORS, ASN_COUNTER, RONLY, var_udp, 1, {3}},
    {UDPOUTDATAGRAMS, ASN_COUNTER, RONLY, var_udp, 1, {4}},
    {UDPLOCALADDRESS, ASN_IPADDRESS, RONLY, var_udpEntry, 3, {5, 1, 1}},
    {UDPLOCALPORT, ASN_INTEGER, RONLY, var_udpEntry, 3, {5, 1, 2}}
};

/*
 * Define the OID pointer to the top of the mib tree that we're
 * registering underneath, and the OID for the MIB module 
 */
oid             udp_variables_oid[] = { SNMP_OID_MIB2, 7 };
oid             udp_module_oid[] = { SNMP_OID_MIB2, 50 };

void
init_udp(void)
{

    /*
     * register ourselves with the agent to handle our mib tree 
     */
    REGISTER_MIB("mibII/udp", udp_variables, variable3, udp_variables_oid);
    REGISTER_SYSOR_ENTRY(udp_module_oid,
                         "The MIB module for managing UDP implementations");

#ifdef UDPSTAT_SYMBOL
    auto_nlist(UDPSTAT_SYMBOL, 0, 0);
#endif
#ifdef UDB_SYMBOL
    auto_nlist(UDB_SYMBOL, 0, 0);
#endif
#ifdef solaris2
    init_kernel_sunos5();
#endif
}


        /*********************
	 *
	 *  System specific implementation functions
	 *
	 *********************/

#ifdef linux
#define UDP_STAT_STRUCTURE	struct udp_mib
#define USES_SNMP_DESIGNED_UDPSTAT
#undef UDPSTAT_SYMBOL
#endif

#ifdef solaris2
#define UDP_STAT_STRUCTURE	mib2_udp_t
#define USES_SNMP_DESIGNED_UDPSTAT
#endif

#ifdef hpux11
#define UDP_STAT_STRUCTURE	int
#endif

#ifdef WIN32
#include <iphlpapi.h>
#define UDP_STAT_STRUCTURE MIB_UDPSTATS
#endif

#ifdef HAVE_SYS_TCPIPSTATS_H
#define UDP_STAT_STRUCTURE	struct kna
#define USES_TRADITIONAL_UDPSTAT
#endif


#if !defined(UDP_STAT_STRUCTURE)
#define UDP_STAT_STRUCTURE	struct udpstat
#define USES_TRADITIONAL_UDPSTAT
#endif

long            read_udp_stat(UDP_STAT_STRUCTURE *, int);

u_char         *
var_udp(struct variable *vp,
        oid * name,
        size_t * length,
        int exact, size_t * var_len, WriteMethod ** write_method)
{
    static UDP_STAT_STRUCTURE udpstat;
    static long     ret_value;

    if (header_generic(vp, name, length, exact, var_len, write_method) ==
        MATCH_FAILED)
        return NULL;

    ret_value = read_udp_stat(&udpstat, vp->magic);
    if (ret_value < 0)
        return NULL;

#ifdef HAVE_SYS_TCPIPSTATS_H
    /*
     * This actually reads statistics for *all* the groups together,
     * so we need to isolate the UDP-specific bits.  
     */
#define udpstat          udpstat.udpstat
#endif

    /*
     *        Get the UDP statistics from the kernel...
     */


    switch (vp->magic) {
#ifdef USES_SNMP_DESIGNED_UDPSTAT
    case UDPINDATAGRAMS:
        return (u_char *) & udpstat.udpInDatagrams;
    case UDPNOPORTS:
#ifdef solaris2
        /*
         * Solaris keeps the count of UDP No Port errors in
         * the IP Mib structure, so this value is returned
         * via the return value of the read_udp_stats routine 
         */
        return (u_char *) & ret_value;
#else
        return (u_char *) & udpstat.udpNoPorts;
#endif
    case UDPOUTDATAGRAMS:
        return (u_char *) & udpstat.udpOutDatagrams;
    case UDPINERRORS:
        return (u_char *) & udpstat.udpInErrors;
#endif                          /* SNMP_DESIGNED_UDPSTAT */


#ifdef USES_TRADITIONAL_UDPSTAT
    case UDPINDATAGRAMS:
#if STRUCT_UDPSTAT_HAS_UDPS_IPACKETS
        return (u_char *) & udpstat.udps_ipackets;
#else
        return NULL;
#endif
    case UDPNOPORTS:
#if STRUCT_UDPSTAT_HAS_UDPS_NOPORT
        return (u_char *) & udpstat.udps_noport;
#else
        return NULL;
#endif
    case UDPOUTDATAGRAMS:
#if STRUCT_UDPSTAT_HAS_UDPS_OPACKETS
        return (u_char *) & udpstat.udps_opackets;
#else
        return NULL;
#endif
    case UDPINERRORS:
        long_return = udpstat.udps_hdrops + udpstat.udps_badsum +
#ifdef STRUCT_UDPSTAT_HAS_UDPS_DISCARD
            udpstat.udps_discard +
#endif
#ifdef STRUCT_UDPSTAT_HAS_UDPS_FULLSOCK
            udpstat.udps_fullsock +
#endif
            udpstat.udps_badlen;
        return (u_char *) & long_return;

#endif                          /* USES_TRADITIONAL_UDPSTAT */
#ifdef WIN32
    case UDPINDATAGRAMS:
        return (u_char *) & udpstat.dwInDatagrams;
    case UDPNOPORTS:
        return (u_char *) & udpstat.dwNoPorts;
    case UDPOUTDATAGRAMS:
        return (u_char *) & udpstat.dwOutDatagrams;
    case UDPINERRORS:
        return (u_char *) & udpstat.dwInErrors;
#endif                          /* WIN32 */

    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_udp\n", vp->magic));
    }
    return NULL;

#ifdef HAVE_SYS_TCPIPSTATS_H
#undef udpstat
#endif
}

        /*********************
	 *
	 *  Internal implementation functions
	 *
	 *********************/

long
read_udp_stat(UDP_STAT_STRUCTURE * udpstat, int magic)
{
    long            ret_value = -1;
#if (defined(CAN_USE_SYSCTL) && defined(UDPCTL_STATS))
    static int      sname[4] =
        { CTL_NET, PF_INET, IPPROTO_UDP, UDPCTL_STATS };
    size_t          len = sizeof(*udpstat);
#endif
#ifdef solaris2
    static mib2_ip_t ipstat;
#endif

#ifdef hpux11
    int             fd;
    struct nmparms  p;
    unsigned int    ulen;

    if ((fd = open_mib("/dev/ip", O_RDONLY, 0, NM_ASYNC_OFF)) < 0)
        return (-1);            /* error */

    switch (magic) {
    case UDPINDATAGRAMS:
        p.objid = ID_udpInDatagrams;
        break;
    case UDPNOPORTS:
        p.objid = ID_udpNoPorts;
        break;
    case UDPOUTDATAGRAMS:
        p.objid = ID_udpOutDatagrams;
        break;
    case UDPINERRORS:
        p.objid = ID_udpInErrors;
        break;
    default:
        *udpstat = 0;
        close_mib(fd);
        return (0);
    }

    p.buffer = (void *) udpstat;
    ulen = sizeof(UDP_STAT_STRUCTURE);
    p.len = &ulen;
    ret_value = get_mib_info(fd, &p);
    close_mib(fd);

    return (ret_value);         /* 0: ok, < 0: error */
#else                           /* hpux11 */

    if (udp_stats_cache_marker &&
        (!atime_ready
         (udp_stats_cache_marker, UDP_STATS_CACHE_TIMEOUT * 1000)))
#ifdef solaris2
        return (magic == UDPNOPORTS ? ipstat.udpNoPorts : 0);
#else
        return 0;
#endif

    if (udp_stats_cache_marker)
        atime_setMarker(udp_stats_cache_marker);
    else
        udp_stats_cache_marker = atime_newMarker();

#ifdef linux
    ret_value = linux_read_udp_stat(udpstat);
#endif

#ifdef WIN32
    ret_value = GetUdpStatistics(udpstat);
#endif

#ifdef solaris2
    if (magic == UDPNOPORTS) {
        if (getMibstat
            (MIB_IP, &ipstat, sizeof(mib2_ip_t), GET_FIRST,
             &Get_everything, NULL) < 0)
            ret_value = -1;
        else
            ret_value = ipstat.udpNoPorts;
    } else
        ret_value = getMibstat(MIB_UDP, udpstat, sizeof(mib2_udp_t),
                               GET_FIRST, &Get_everything, NULL);
#endif

#ifdef HAVE_SYS_TCPIPSTATS_H
    ret_value = sysmp(MP_SAGET, MPSA_TCPIPSTATS, udpstat, sizeof *udpstat);
#endif

#if defined(CAN_USE_SYSCTL) && defined(UDPCTL_STATS)
    ret_value = sysctl(sname, 4, udpstat, &len, 0, 0);
#endif

#ifdef UDPSTAT_SYMBOL
    if (auto_nlist(UDPSTAT_SYMBOL, (char *) udpstat, sizeof(*udpstat)))
        ret_value = 0;
#endif

    if (ret_value == -1) {
        free(udp_stats_cache_marker);
        udp_stats_cache_marker = NULL;
    }
    return ret_value;
#endif                          /* hpux11 */
}
