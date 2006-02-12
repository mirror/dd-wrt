
/*
 *  TCP MIB group implementation - tcp.c
 *
 */

#include <net-snmp/net-snmp-config.h>

#if HAVE_STRING_H
#include <string.h>
#endif
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#if HAVE_SYS_PROTOSW_H
#include <sys/protosw.h>
#endif

#if HAVE_SYS_SYSMP_H
#include <sys/sysmp.h>
#endif
#if defined(IFNET_NEEDS_KERNEL) && !defined(_KERNEL)
#define _KERNEL 1
#define _I_DEFINED_KERNEL
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
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
#if HAVE_NETINET_IP_VAR_H
#include <netinet/ip_var.h>
#endif
#ifdef INET6
#if HAVE_NETINET6_IP6_VAR_H
#include <netinet6/ip6_var.h>
#endif
#endif
#if HAVE_SYS_SOCKETVAR_H
#include <sys/socketvar.h>
#endif
#if HAVE_NETINET_IN_PCB_H
#include <netinet/in_pcb.h>
#endif
#if HAVE_INET_MIB2_H
#include <inet/mib2.h>
#endif

#if HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#if defined(osf4) || defined(aix4) || defined(hpux10)
/*
 * these are undefed to remove a stupid warning on osf compilers
 * because they get redefined with a slightly different notation of the
 * same value.  -- Wes 
 */
#undef TCP_NODELAY
#undef TCP_MAXSEG
#endif
#if HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif
#if HAVE_NETINET_TCPIP_H
#include <netinet/tcpip.h>
#endif
#if HAVE_NETINET_TCP_TIMER_H
#include <netinet/tcp_timer.h>
#endif
#if HAVE_NETINET_TCP_VAR_H
#include <netinet/tcp_var.h>
#endif
#if HAVE_NETINET_TCP_FSM_H
#include <netinet/tcp_fsm.h>
#endif
#if HAVE_SYS_TCPIPSTATS_H
#include <sys/tcpipstats.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/auto_nlist.h>

#ifdef solaris2
#include "kernel_sunos5.h"
#else
#include "kernel.h"
#endif
#ifdef linux
#include "kernel_linux.h"
#endif
#include "util_funcs.h"

#ifdef hpux
#include <sys/mib.h>
#include <netinet/mib_kern.h>
#endif                          /* hpux */

#ifdef cygwin
#define WIN32
#include <windows.h>
#endif

#include "tcp.h"
#include "tcpTable.h"
#include "sysORTable.h"

#ifndef MIB_STATS_CACHE_TIMEOUT
#define MIB_STATS_CACHE_TIMEOUT	5
#endif
#ifndef TCP_STATS_CACHE_TIMEOUT
#define TCP_STATS_CACHE_TIMEOUT	MIB_STATS_CACHE_TIMEOUT
#endif
marker_t        tcp_stats_cache_marker = NULL;

        /*********************
	 *
	 *  Kernel & interface information,
	 *   and internal forward declarations
	 *
	 *********************/

#ifdef freebsd4
static unsigned int hz;
#endif

        /*********************
	 *
	 *  Initialisation & common implementation functions
	 *
	 *********************/

struct variable3 tcp_variables[] = {
    {TCPRTOALGORITHM, ASN_INTEGER, RONLY, var_tcp, 1, {1}},
    {TCPRTOMIN, ASN_INTEGER, RONLY, var_tcp, 1, {2}},
#ifndef sunV3
    {TCPRTOMAX, ASN_INTEGER, RONLY, var_tcp, 1, {3}},
#endif
    {TCPMAXCONN, ASN_INTEGER, RONLY, var_tcp, 1, {4}},
#ifndef sunV3
    {TCPACTIVEOPENS, ASN_COUNTER, RONLY, var_tcp, 1, {5}},
    {TCPPASSIVEOPENS, ASN_COUNTER, RONLY, var_tcp, 1, {6}},
    {TCPATTEMPTFAILS, ASN_COUNTER, RONLY, var_tcp, 1, {7}},
    {TCPESTABRESETS, ASN_COUNTER, RONLY, var_tcp, 1, {8}},
#endif
    {TCPCURRESTAB, ASN_GAUGE, RONLY, var_tcp, 1, {9}},
#ifndef sunV3
    {TCPINSEGS, ASN_COUNTER, RONLY, var_tcp, 1, {10}},
    {TCPOUTSEGS, ASN_COUNTER, RONLY, var_tcp, 1, {11}},
    {TCPRETRANSSEGS, ASN_COUNTER, RONLY, var_tcp, 1, {12}},
#endif
#ifdef WIN32
    {TCPCONNSTATE, ASN_INTEGER, RWRITE, var_tcpEntry, 3, {13, 1, 1}},
#else
    {TCPCONNSTATE, ASN_INTEGER, RONLY, var_tcpEntry, 3, {13, 1, 1}},
#endif
    {TCPCONNLOCALADDRESS, ASN_IPADDRESS, RONLY, var_tcpEntry, 3,
     {13, 1, 2}},
    {TCPCONNLOCALPORT, ASN_INTEGER, RONLY, var_tcpEntry, 3, {13, 1, 3}},
    {TCPCONNREMADDRESS, ASN_IPADDRESS, RONLY, var_tcpEntry, 3, {13, 1, 4}},
    {TCPCONNREMPORT, ASN_INTEGER, RONLY, var_tcpEntry, 3, {13, 1, 5}},
    {TCPINERRS, ASN_COUNTER, RONLY, var_tcp, 1, {14}},
    {TCPOUTRSTS, ASN_COUNTER, RONLY, var_tcp, 1, {15}}
};

/*
 * Define the OID pointer to the top of the mib tree that we're
 * registering underneath, and the OID for the MIB module 
 */
oid             tcp_variables_oid[] = { SNMP_OID_MIB2, 6 };
oid             tcp_module_oid[] = { SNMP_OID_MIB2, 49 };

void
init_tcp(void)
{
    /*
     * register ourselves with the agent to handle our mib tree 
     */
    REGISTER_MIB("mibII/tcp", tcp_variables, variable3,
                 tcp_variables_oid);
    REGISTER_SYSOR_ENTRY(tcp_module_oid,
                         "The MIB module for managing TCP implementations");

#ifdef TCPSTAT_SYMBOL
    auto_nlist(TCPSTAT_SYMBOL, 0, 0);
#endif
#ifdef TCP_SYMBOL
    auto_nlist(TCP_SYMBOL, 0, 0);
#endif
#if freebsd4
    hz = sysconf(_SC_CLK_TCK);  /* get ticks/s from system */
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
#define TCP_STAT_STRUCTURE	struct tcp_mib
#define USES_SNMP_DESIGNED_TCPSTAT
#undef TCPSTAT_SYMBOL
#endif

#ifdef solaris2
#define TCP_STAT_STRUCTURE	mib2_tcp_t
#define USES_SNMP_DESIGNED_TCPSTAT
#endif

#ifdef hpux11
#define TCP_STAT_STRUCTURE	int
#endif

#ifdef WIN32
#include <iphlpapi.h>
#define TCP_STAT_STRUCTURE     MIB_TCPSTATS
#endif

#ifdef HAVE_SYS_TCPIPSTATS_H
#define TCP_STAT_STRUCTURE	struct kna
#define USES_TRADITIONAL_TCPSTAT
#endif

#if !defined(TCP_STAT_STRUCTURE)
#define TCP_STAT_STRUCTURE	struct tcpstat
#define USES_TRADITIONAL_TCPSTAT
#endif

long            read_tcp_stat(TCP_STAT_STRUCTURE *, int);


u_char         *
var_tcp(struct variable *vp,
        oid * name,
        size_t * length,
        int exact, size_t * var_len, WriteMethod ** write_method)
{
    static TCP_STAT_STRUCTURE tcpstat;
    static long     ret_value;
#ifdef TCPTV_NEEDS_HZ
    /*
     * I don't know of any such system now, but maybe they'll figure
     * it out some day.
     */
    int             hz = 1000;
#endif

    if (header_generic(vp, name, length, exact, var_len, write_method) ==
        MATCH_FAILED)
        return NULL;

    ret_value = read_tcp_stat(&tcpstat, vp->magic);
    if (ret_value < 0)
        return NULL;

#ifdef HAVE_SYS_TCPIPSTATS_H
    /*
     * This actually reads statistics for *all* the groups together,
     * so we need to isolate the TCP-specific bits.  
     */
#define tcpstat          tcpstat.tcpstat
#endif

    switch (vp->magic) {
#ifdef USES_SNMP_DESIGNED_TCPSTAT
    case TCPRTOALGORITHM:
        return (u_char *) & tcpstat.tcpRtoAlgorithm;
    case TCPRTOMIN:
        return (u_char *) & tcpstat.tcpRtoMin;
    case TCPRTOMAX:
        return (u_char *) & tcpstat.tcpRtoMax;
    case TCPMAXCONN:
        return (u_char *) & tcpstat.tcpMaxConn;
    case TCPACTIVEOPENS:
        return (u_char *) & tcpstat.tcpActiveOpens;
    case TCPPASSIVEOPENS:
        return (u_char *) & tcpstat.tcpPassiveOpens;
    case TCPATTEMPTFAILS:
        return (u_char *) & tcpstat.tcpAttemptFails;
    case TCPESTABRESETS:
        return (u_char *) & tcpstat.tcpEstabResets;
    case TCPCURRESTAB:
        return (u_char *) & tcpstat.tcpCurrEstab;
    case TCPINSEGS:
        return (u_char *) & tcpstat.tcpInSegs;
    case TCPOUTSEGS:
        return (u_char *) & tcpstat.tcpOutSegs;
    case TCPRETRANSSEGS:
        return (u_char *) & tcpstat.tcpRetransSegs;
    case TCPINERRS:
#ifdef solaris2
        return (u_char *) & ret_value;
#elif defined(linux)
        if (tcpstat.tcpInErrsValid)
            return (u_char *) & tcpstat.tcpInErrs;
        return NULL;
#else
        return NULL;
#endif
    case TCPOUTRSTS:
#ifdef linux
        if (tcpstat.tcpOutRstsValid)
            return (u_char *) & tcpstat.tcpOutRsts;
        return NULL;
#else
        return NULL;
#endif
#endif

#ifdef USES_TRADITIONAL_TCPSTAT
    case TCPRTOALGORITHM:      /* Assume Van Jacobsen's algorithm */
        long_return = 4;
        return (u_char *) & long_return;
    case TCPRTOMIN:
#ifdef TCPTV_NEEDS_HZ
        long_return = TCPTV_MIN;
#else
        long_return = TCPTV_MIN / PR_SLOWHZ * 1000;
#endif
        return (u_char *) & long_return;
    case TCPRTOMAX:
#ifdef TCPTV_NEEDS_HZ
        long_return = TCPTV_REXMTMAX;
#else
        long_return = TCPTV_REXMTMAX / PR_SLOWHZ * 1000;
#endif
        return (u_char *) & long_return;
    case TCPMAXCONN:
        return NULL;
    case TCPACTIVEOPENS:
        return (u_char *) & tcpstat.tcps_connattempt;
    case TCPPASSIVEOPENS:
        return (u_char *) & tcpstat.tcps_accepts;
        /*
         * NB:  tcps_drops is actually the sum of the two MIB
         *      counters tcpAttemptFails and tcpEstabResets.
         */
    case TCPATTEMPTFAILS:
        return (u_char *) & tcpstat.tcps_conndrops;
    case TCPESTABRESETS:
        return (u_char *) & tcpstat.tcps_drops;
    case TCPCURRESTAB:
        long_return = TCP_Count_Connections();
        return (u_char *) & long_return;
    case TCPINSEGS:
        return (u_char *) & tcpstat.tcps_rcvtotal;
    case TCPOUTSEGS:
        /*
         * RFC 1213 defines this as the number of segments sent
         * "excluding those containing only retransmitted octets"
         */
        long_return = tcpstat.tcps_sndtotal - tcpstat.tcps_sndrexmitpack;
        return (u_char *) & long_return;
    case TCPRETRANSSEGS:
        return (u_char *) & tcpstat.tcps_sndrexmitpack;
    case TCPINERRS:
        long_return = tcpstat.tcps_rcvbadsum + tcpstat.tcps_rcvbadoff
#ifdef STRUCT_TCPSTAT_HAS_TCPS_RCVMEMDROP
            + tcpstat.tcps_rcvmemdrop
#endif
            + tcpstat.tcps_rcvshort;
        return (u_char *) & long_return;
    case TCPOUTRSTS:
        long_return = tcpstat.tcps_sndctrl - tcpstat.tcps_closed;
        return (u_char *) & long_return;
#endif
#ifdef WIN32
    case TCPRTOALGORITHM:
        return (u_char *) & tcpstat.dwRtoAlgorithm;
    case TCPRTOMIN:
        return (u_char *) & tcpstat.dwRtoMin;
    case TCPRTOMAX:
        return (u_char *) & tcpstat.dwRtoMax;
    case TCPMAXCONN:
        return (u_char *) & tcpstat.dwMaxConn;
    case TCPACTIVEOPENS:
        return (u_char *) & tcpstat.dwActiveOpens;
    case TCPPASSIVEOPENS:
        return (u_char *) & tcpstat.dwPassiveOpens;
    case TCPATTEMPTFAILS:
        return (u_char *) & tcpstat.dwAttemptFails;
    case TCPESTABRESETS:
        return (u_char *) & tcpstat.dwEstabResets;
    case TCPCURRESTAB:
        return (u_char *) & tcpstat.dwCurrEstab;
    case TCPINSEGS:
        return (u_char *) & tcpstat.dwInSegs;
    case TCPOUTSEGS:
        return (u_char *) & tcpstat.dwOutSegs;
    case TCPRETRANSSEGS:
        return (u_char *) & tcpstat.dwRetransSegs;
    case TCPINERRS:
        return (u_char *) & tcpstat.dwInErrs;
    case TCPOUTRSTS:
        return (u_char *) & tcpstat.dwOutRsts;
#endif
    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_tcp\n", vp->magic));
    }
    return NULL;

#ifdef HAVE_SYS_TCPIPSTATS_H
#undef tcpstat
#endif
}


        /*********************
	 *
	 *  Internal implementation functions
	 *
	 *********************/

long
read_tcp_stat(TCP_STAT_STRUCTURE * tcpstat, int magic)
{
    long            ret_value = -1;
#if (defined(CAN_USE_SYSCTL) && defined(TCPCTL_STATS))
    static int      sname[4] =
        { CTL_NET, PF_INET, IPPROTO_TCP, TCPCTL_STATS };
    size_t          len = sizeof(*tcpstat);
#endif
#ifdef solaris2
    static mib2_ip_t ipstat;
#endif
#ifdef hpux11
    int             fd;
    struct nmparms  p;
    unsigned int    ulen;
    int             ret;

    if ((fd = open_mib("/dev/ip", O_RDONLY, 0, NM_ASYNC_OFF)) < 0)
        return (-1);            /* error */

    switch (magic) {
    case TCPRTOALGORITHM:
        p.objid = ID_tcpRtoAlgorithm;
        break;
    case TCPRTOMIN:
        p.objid = ID_tcpRtoMin;
        break;
    case TCPRTOMAX:
        p.objid = ID_tcpRtoMax;
        break;
    case TCPMAXCONN:
        p.objid = ID_tcpMaxConn;
        break;
    case TCPACTIVEOPENS:
        p.objid = ID_tcpActiveOpens;
        break;
    case TCPPASSIVEOPENS:
        p.objid = ID_tcpPassiveOpens;
        break;
    case TCPATTEMPTFAILS:
        p.objid = ID_tcpAttemptFails;
        break;
    case TCPESTABRESETS:
        p.objid = ID_tcpEstabResets;
        break;
    case TCPCURRESTAB:
        p.objid = ID_tcpCurrEstab;
        break;
    case TCPINSEGS:
        p.objid = ID_tcpInSegs;
        break;
    case TCPOUTSEGS:
        p.objid = ID_tcpOutSegs;
        break;
    case TCPRETRANSSEGS:
        p.objid = ID_tcpRetransSegs;
        break;
    case TCPINERRS:
        p.objid = ID_tcpInErrs;
        break;
    case TCPOUTRSTS:
        p.objid = ID_tcpOutRsts;
        break;
    default:
        *tcpstat = 0;
        close_mib(fd);
        return (0);
    }

    p.buffer = (void *) tcpstat;
    ulen = sizeof(TCP_STAT_STRUCTURE);
    p.len = &ulen;
    ret_value = get_mib_info(fd, &p);
    close_mib(fd);

    return (ret_value);         /* 0: ok, < 0: error */
#else                           /* hpux11 */

    if (tcp_stats_cache_marker &&
        (!atime_ready
         (tcp_stats_cache_marker, TCP_STATS_CACHE_TIMEOUT * 1000)))
#ifdef solaris2
        return (magic == TCPINERRS ? ipstat.tcpInErrs : 0);
#else
        return 0;
#endif

    if (tcp_stats_cache_marker)
        atime_setMarker(tcp_stats_cache_marker);
    else
        tcp_stats_cache_marker = atime_newMarker();

#ifdef linux
    ret_value = linux_read_tcp_stat(tcpstat);
#endif

#ifdef solaris2
    if (magic == TCPINERRS) {
        if (getMibstat
            (MIB_IP, &ipstat, sizeof(mib2_ip_t), GET_FIRST,
             &Get_everything, NULL) < 0)
            ret_value = -1;
        else
            ret_value = ipstat.tcpInErrs;
    } else
        ret_value = getMibstat(MIB_TCP, tcpstat, sizeof(mib2_tcp_t),
                               GET_FIRST, &Get_everything, NULL);
#endif

#ifdef WIN32
    ret_value = GetTcpStatistics(tcpstat);
#endif

#ifdef HAVE_SYS_TCPIPSTATS_H
    ret_value = sysmp(MP_SAGET, MPSA_TCPIPSTATS, tcpstat, sizeof *tcpstat);
#endif

#if defined(CAN_USE_SYSCTL) && defined(TCPCTL_STATS)
    ret_value = sysctl(sname, 4, tcpstat, &len, 0, 0);
#endif

#ifdef TCPSTAT_SYMBOL
    if (auto_nlist(TCPSTAT_SYMBOL, (char *) tcpstat, sizeof(*tcpstat)))
        ret_value = 0;
#endif

    if (ret_value == -1) {
        free(tcp_stats_cache_marker);
        tcp_stats_cache_marker = NULL;
    }
    return ret_value;
#endif                          /* hpux11 */
}
