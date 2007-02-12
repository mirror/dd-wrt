/*
 *  IP MIB group implementation - ip.c
 *
 */

#include <net-snmp/net-snmp-config.h>

#if defined(IFNET_NEEDS_KERNEL) && !defined(_KERNEL)
#define _KERNEL 1
#define _I_DEFINED_KERNEL
#endif
#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif
#if HAVE_SYS_SYSCTL_H
#ifdef _I_DEFINED_KERNEL
#undef _KERNEL
#endif
#include <sys/sysctl.h>
#ifdef _I_DEFINED_KERNEL
#define _KERNEL 1
#endif
#endif
#if HAVE_SYS_SYSMP_H
#include <sys/sysmp.h>
#endif
#if HAVE_SYS_TCPIPSTATS_H
#include <sys/tcpipstats.h>
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
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_NETINET_IN_SYSTM_H
#include <netinet/in_systm.h>
#endif
#if HAVE_SYS_HASHING_H
#include <sys/hashing.h>
#endif
#if HAVE_NETINET_IN_VAR_H
#include <netinet/in_var.h>
#endif
#if HAVE_NETINET_IP_H
#include <netinet/ip.h>
#endif
#if HAVE_NETINET_IP_VAR_H
#include <netinet/ip_var.h>
#endif
#if HAVE_INET_MIB2_H
#include <inet/mib2.h>
#endif
#if HAVE_SYS_STREAM_H
#include <sys/stream.h>
#endif
#if HAVE_NET_ROUTE_H
#include <net/route.h>
#endif
#if HAVE_SYSLOG_H
#include <syslog.h>
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

#if defined(MIB_IPCOUNTER_SYMBOL) || defined(hpux11)
#include <sys/mib.h>
#include <netinet/mib_kern.h>
#endif                          /* MIB_IPCOUNTER_SYMBOL || hpux11 */

#include "util_funcs.h"
#include "ip.h"
#include "ipAddr.h"
#include "interfaces.h"
#include "sysORTable.h"

#ifdef cygwin
#define WIN32
#include <windows.h>
#endif

#ifndef MIB_STATS_CACHE_TIMEOUT
#define MIB_STATS_CACHE_TIMEOUT	5
#endif
#ifndef IP_STATS_CACHE_TIMEOUT
#define IP_STATS_CACHE_TIMEOUT	MIB_STATS_CACHE_TIMEOUT
#endif
marker_t        ip_stats_cache_marker = NULL;

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

extern void     init_routes(void);


/*
 * define the structure we're going to ask the agent to register our
 * information at 
 */
struct variable3 ip_variables[] = {
#ifdef WIN32
    {IPFORWARDING, ASN_INTEGER, RWRITE, var_ip, 1, {1}},
    {IPDEFAULTTTL, ASN_INTEGER, RWRITE, var_ip, 1, {2}},
#else
    {IPFORWARDING, ASN_INTEGER, RONLY, var_ip, 1, {1}},
    {IPDEFAULTTTL, ASN_INTEGER, RONLY, var_ip, 1, {2}},
#endif
#ifndef sunV3
    {IPINRECEIVES, ASN_COUNTER, RONLY, var_ip, 1, {3}},
#endif
    {IPINHDRERRORS, ASN_COUNTER, RONLY, var_ip, 1, {4}},
#ifndef sunV3
    {IPINADDRERRORS, ASN_COUNTER, RONLY, var_ip, 1, {5}},
    {IPFORWDATAGRAMS, ASN_COUNTER, RONLY, var_ip, 1, {6}},
#endif
    {IPINUNKNOWNPROTOS, ASN_COUNTER, RONLY, var_ip, 1, {7}},
#ifndef sunV3
    {IPINDISCARDS, ASN_COUNTER, RONLY, var_ip, 1, {8}},
    {IPINDELIVERS, ASN_COUNTER, RONLY, var_ip, 1, {9}},
#endif
    {IPOUTREQUESTS, ASN_COUNTER, RONLY, var_ip, 1, {10}},
    {IPOUTDISCARDS, ASN_COUNTER, RONLY, var_ip, 1, {11}},
    {IPOUTNOROUTES, ASN_COUNTER, RONLY, var_ip, 1, {12}},
    {IPREASMTIMEOUT, ASN_INTEGER, RONLY, var_ip, 1, {13}},
#ifndef sunV3
    {IPREASMREQDS, ASN_COUNTER, RONLY, var_ip, 1, {14}},
    {IPREASMOKS, ASN_COUNTER, RONLY, var_ip, 1, {15}},
    {IPREASMFAILS, ASN_COUNTER, RONLY, var_ip, 1, {16}},
#endif
    {IPFRAGOKS, ASN_COUNTER, RONLY, var_ip, 1, {17}},
    {IPFRAGFAILS, ASN_COUNTER, RONLY, var_ip, 1, {18}},
    {IPFRAGCREATES, ASN_COUNTER, RONLY, var_ip, 1, {19}},
    {IPADADDR, ASN_IPADDRESS, RONLY, var_ipAddrEntry, 3, {20, 1, 1}},
    {IPADIFINDEX, ASN_INTEGER, RONLY, var_ipAddrEntry, 3, {20, 1, 2}},
#ifndef sunV3
    {IPADNETMASK, ASN_IPADDRESS, RONLY, var_ipAddrEntry, 3, {20, 1, 3}},
#endif
    {IPADBCASTADDR, ASN_INTEGER, RONLY, var_ipAddrEntry, 3, {20, 1, 4}},
    {IPADREASMMAX, ASN_INTEGER, RONLY, var_ipAddrEntry, 3, {20, 1, 5}},
    {IPROUTEDEST, ASN_IPADDRESS, RWRITE, var_ipRouteEntry, 3, {21, 1, 1}},
    {IPROUTEIFINDEX, ASN_INTEGER, RWRITE, var_ipRouteEntry, 3, {21, 1, 2}},
    {IPROUTEMETRIC1, ASN_INTEGER, RWRITE, var_ipRouteEntry, 3, {21, 1, 3}},
    {IPROUTEMETRIC2, ASN_INTEGER, RWRITE, var_ipRouteEntry, 3, {21, 1, 4}},
    {IPROUTEMETRIC3, ASN_INTEGER, RWRITE, var_ipRouteEntry, 3, {21, 1, 5}},
    {IPROUTEMETRIC4, ASN_INTEGER, RWRITE, var_ipRouteEntry, 3, {21, 1, 6}},
    {IPROUTENEXTHOP, ASN_IPADDRESS, RWRITE, var_ipRouteEntry, 3,
     {21, 1, 7}},
    {IPROUTETYPE, ASN_INTEGER, RWRITE, var_ipRouteEntry, 3, {21, 1, 8}},
    {IPROUTEPROTO, ASN_INTEGER, RONLY, var_ipRouteEntry, 3, {21, 1, 9}},
    {IPROUTEAGE, ASN_INTEGER, RWRITE, var_ipRouteEntry, 3, {21, 1, 10}},
    {IPROUTEMASK, ASN_IPADDRESS, RWRITE, var_ipRouteEntry, 3, {21, 1, 11}},
    {IPROUTEMETRIC5, ASN_INTEGER, RWRITE, var_ipRouteEntry, 3,
     {21, 1, 12}},
    {IPROUTEINFO, ASN_OBJECT_ID, RONLY, var_ipRouteEntry, 3, {21, 1, 13}},
#ifdef USING_MIBII_AT_MODULE
#ifdef WIN32
    {IPMEDIAIFINDEX, ASN_INTEGER, RWRITE, var_atEntry, 3, {22, 1, 1}},
    {IPMEDIAPHYSADDRESS, ASN_OCTET_STR, RWRITE, var_atEntry, 3,
     {22, 1, 2}},
    {IPMEDIANETADDRESS, ASN_IPADDRESS, RWRITE, var_atEntry, 3, {22, 1, 3}},
    {IPMEDIATYPE, ASN_INTEGER, RWRITE, var_atEntry, 3, {22, 1, 4}},
#else
    {IPMEDIAIFINDEX, ASN_INTEGER, RONLY, var_atEntry, 3, {22, 1, 1}},
    {IPMEDIAPHYSADDRESS, ASN_OCTET_STR, RONLY, var_atEntry, 3, {22, 1, 2}},
    {IPMEDIANETADDRESS, ASN_IPADDRESS, RONLY, var_atEntry, 3, {22, 1, 3}},
    {IPMEDIATYPE, ASN_INTEGER, RONLY, var_atEntry, 3, {22, 1, 4}},
#endif
#endif
    {IPROUTEDISCARDS, ASN_COUNTER, RONLY, var_ip, 1, {23}}
};

/*
 * Define the OID pointer to the top of the mib tree that we're
 * registering underneath, and the OID of the MIB module 
 */
oid             ip_variables_oid[] = { SNMP_OID_MIB2, 4 };
oid             ip_module_oid[] = { SNMP_OID_MIB2, 4 };
oid             ip_module_oid_len = sizeof(ip_module_oid) / sizeof(oid);
int             ip_module_count = 0;    /* Need to liaise with icmp.c */

void
init_ip(void)
{
    /*
     * register ourselves with the agent to handle our mib tree 
     */
    REGISTER_MIB("mibII/ip", ip_variables, variable3, ip_variables_oid);
    if (++ip_module_count == 2)
        REGISTER_SYSOR_ENTRY(ip_module_oid,
                             "The MIB module for managing IP and ICMP implementations");


    /*
     * for speed optimization, we call this now to do the lookup 
     */
#ifdef IPSTAT_SYMBOL
    auto_nlist(IPSTAT_SYMBOL, 0, 0);
#endif
#ifdef IP_FORWARDING_SYMBOL
    auto_nlist(IP_FORWARDING_SYMBOL, 0, 0);
#endif
#ifdef TCP_TTL_SYMBOL
    auto_nlist(TCP_TTL_SYMBOL, 0, 0);
#endif
#ifdef MIB_IPCOUNTER_SYMBOL
    auto_nlist(MIB_IPCOUNTER_SYMBOL, 0, 0);
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
#define IP_STAT_STRUCTURE	struct ip_mib
#define	USES_SNMP_DESIGNED_IPSTAT
#endif

#ifdef solaris2
#define IP_STAT_STRUCTURE	mib2_ip_t
#define	USES_SNMP_DESIGNED_IPSTAT
#endif

#ifdef hpux11
#define IP_STAT_STRUCTURE	int
#endif

#ifdef HAVE_SYS_TCPIPSTATS_H
#define IP_STAT_STRUCTURE	struct kna
#define	USES_TRADITIONAL_IPSTAT
#endif

#ifdef WIN32
#include <iphlpapi.h>
#define IP_STAT_STRUCTURE MIB_IPSTATS
WriteMethod     writeIpStats;
long            ipForwarding;
long            oldipForwarding;
long            ipTTL, oldipTTL;
#endif                          /* WIN32 */

#if !defined(IP_STAT_STRUCTURE)
#define IP_STAT_STRUCTURE	struct ipstat
#define	USES_TRADITIONAL_IPSTAT
#endif

long            read_ip_stat(IP_STAT_STRUCTURE *, int);

u_char         *
var_ip(struct variable *vp,
       oid * name,
       size_t * length,
       int exact, size_t * var_len, WriteMethod ** write_method)
{
    static IP_STAT_STRUCTURE ipstat;
    static long     ret_value;

    if (header_generic(vp, name, length, exact, var_len, write_method) ==
        MATCH_FAILED)
        return NULL;

    ret_value = read_ip_stat(&ipstat, vp->magic);
    if (ret_value < 0)
        return NULL;

#ifdef HAVE_SYS_TCPIPSTATS_H
    /*
     * This actually reads statistics for *all* the groups together,
     * so we need to isolate the IP-specific bits.  
     */
#define	ipstat		ipstat.ipstat
#endif

    switch (vp->magic) {
#ifdef USES_SNMP_DESIGNED_IPSTAT
    case IPFORWARDING:
        return (u_char *) & ipstat.ipForwarding;
    case IPDEFAULTTTL:
        return (u_char *) & ipstat.ipDefaultTTL;
    case IPINRECEIVES:
        return (u_char *) & ipstat.ipInReceives;
    case IPINHDRERRORS:
        return (u_char *) & ipstat.ipInHdrErrors;
    case IPINADDRERRORS:
        return (u_char *) & ipstat.ipInAddrErrors;
    case IPFORWDATAGRAMS:
        return (u_char *) & ipstat.ipForwDatagrams;
    case IPINUNKNOWNPROTOS:
        return (u_char *) & ipstat.ipInUnknownProtos;
    case IPINDISCARDS:
        return (u_char *) & ipstat.ipInDiscards;
    case IPINDELIVERS:
        return (u_char *) & ipstat.ipInDelivers;
    case IPOUTREQUESTS:
        return (u_char *) & ipstat.ipOutRequests;
    case IPOUTDISCARDS:
        return (u_char *) & ipstat.ipOutDiscards;
    case IPOUTNOROUTES:
        return (u_char *) & ipstat.ipOutNoRoutes;
    case IPREASMTIMEOUT:
        return (u_char *) & ipstat.ipReasmTimeout;
    case IPREASMREQDS:
        return (u_char *) & ipstat.ipReasmReqds;
    case IPREASMOKS:
        return (u_char *) & ipstat.ipReasmOKs;
    case IPREASMFAILS:
        return (u_char *) & ipstat.ipReasmFails;
    case IPFRAGOKS:
        return (u_char *) & ipstat.ipFragOKs;
    case IPFRAGFAILS:
        return (u_char *) & ipstat.ipFragFails;
    case IPFRAGCREATES:
        return (u_char *) & ipstat.ipFragCreates;
    case IPROUTEDISCARDS:
        return (u_char *) & ipstat.ipRoutingDiscards;
#endif



#ifdef USES_TRADITIONAL_IPSTAT
    case IPFORWARDING:
        return (u_char *) & ret_value;
    case IPDEFAULTTTL:
        return (u_char *) & ret_value;
    case IPINRECEIVES:
        long_return = ipstat.ips_total;
        return (u_char *) & long_return;
    case IPINHDRERRORS:
        long_return = ipstat.ips_badsum
            + ipstat.ips_tooshort
            + ipstat.ips_toosmall + ipstat.ips_badhlen + ipstat.ips_badlen;
        return (u_char *) & long_return;
    case IPINADDRERRORS:
        long_return = ipstat.ips_cantforward;
        return (u_char *) & long_return;
    case IPFORWDATAGRAMS:
        long_return = ipstat.ips_forward;
        return (u_char *) & long_return;
    case IPINUNKNOWNPROTOS:
#if STRUCT_IPSTAT_HAS_IPS_NOPROTO
        long_return = ipstat.ips_noproto;
        return (u_char *) & long_return;
#else
        return NULL;
#endif
    case IPINDISCARDS:
#if STRUCT_IPSTAT_HAS_IPS_FRAGDROPPED
        long_return = ipstat.ips_fragdropped;   /* ?? */
        return (u_char *) & long_return;
#else
        return NULL;
#endif
    case IPINDELIVERS:
#if STRUCT_IPSTAT_HAS_IPS_DELIVERED
        long_return = ipstat.ips_delivered;
        return (u_char *) & long_return;
#else
        return NULL;
#endif
    case IPOUTREQUESTS:
#if STRUCT_IPSTAT_HAS_IPS_LOCALOUT
        long_return = ipstat.ips_localout;
        return (u_char *) & long_return;
#else
        return NULL;
#endif
    case IPOUTDISCARDS:
#if STRUCT_IPSTAT_HAS_IPS_ODROPPED
        long_return = ipstat.ips_odropped;
        return (u_char *) & long_return;
#else
        return NULL;
#endif
    case IPOUTNOROUTES:
        /*
         * XXX: how to calculate this (counts dropped routes, not packets)?
         * ipstat.ips_cantforward isn't right, as it counts packets.
         * ipstat.ips_noroute is also incorrect.
         */
        return NULL;
    case IPREASMTIMEOUT:
        long_return = IPFRAGTTL;
        return (u_char *) & long_return;
    case IPREASMREQDS:
        long_return = ipstat.ips_fragments;
        return (u_char *) & long_return;
    case IPREASMOKS:
#if STRUCT_IPSTAT_HAS_IPS_REASSEMBLED
        long_return = ipstat.ips_reassembled;
        return (u_char *) & long_return;
#else
        return NULL;
#endif
    case IPREASMFAILS:
        long_return = ipstat.ips_fragdropped + ipstat.ips_fragtimeout;
        return (u_char *) & long_return;
    case IPFRAGOKS:            /* XXX */
        long_return = ipstat.ips_fragments
            - (ipstat.ips_fragdropped + ipstat.ips_fragtimeout);
        return (u_char *) & long_return;
    case IPFRAGFAILS:
#if STRUCT_IPSTAT_HAS_IPS_CANTFRAG
        long_return = ipstat.ips_cantfrag;
        return (u_char *) & long_return;
#else
        return NULL;
#endif
    case IPFRAGCREATES:
#if STRUCT_IPSTAT_HAS_IPS_OFRAGMENTS
        long_return = ipstat.ips_ofragments;
        return (u_char *) & long_return;
#else
        return NULL;
#endif
    case IPROUTEDISCARDS:
#if STRUCT_IPSTAT_HAS_IPS_NOROUTE
        long_return = ipstat.ips_noroute;
        return (u_char *) & long_return;
#else
        return NULL;
#endif

#endif                          /* USE_TRADITIONAL_IPSTAT */
#ifdef WIN32
    case IPFORWARDING:
        *write_method = writeIpStats;
        ipForwarding = ipstat.dwForwarding;
        return (u_char *) & ipstat.dwForwarding;
    case IPDEFAULTTTL:
        *write_method = writeIpStats;
        ipTTL = ipstat.dwDefaultTTL;
        return (u_char *) & ipstat.dwDefaultTTL;
    case IPINRECEIVES:
        return (u_char *) & ipstat.dwInReceives;
    case IPINHDRERRORS:
        return (u_char *) & ipstat.dwInHdrErrors;
    case IPINADDRERRORS:
        return (u_char *) & ipstat.dwInAddrErrors;
    case IPFORWDATAGRAMS:
        return (u_char *) & ipstat.dwForwDatagrams;
    case IPINUNKNOWNPROTOS:
        return (u_char *) & ipstat.dwInUnknownProtos;
    case IPINDISCARDS:
        return (u_char *) & ipstat.dwInDiscards;
    case IPINDELIVERS:
        return (u_char *) & ipstat.dwInDelivers;
    case IPOUTREQUESTS:
        return (u_char *) & ipstat.dwOutRequests;
    case IPOUTDISCARDS:
        return (u_char *) & ipstat.dwOutDiscards;
    case IPOUTNOROUTES:
        return (u_char *) & ipstat.dwOutNoRoutes;
    case IPREASMTIMEOUT:
        return (u_char *) & ipstat.dwReasmTimeout;
    case IPREASMREQDS:
        return (u_char *) & ipstat.dwReasmReqds;
    case IPREASMOKS:
        return (u_char *) & ipstat.dwReasmOks;
    case IPREASMFAILS:
        return (u_char *) & ipstat.dwReasmFails;
    case IPFRAGOKS:
        return (u_char *) & ipstat.dwFragOks;
    case IPFRAGFAILS:
        return (u_char *) & ipstat.dwFragFails;
    case IPFRAGCREATES:
        return (u_char *) & ipstat.dwFragCreates;
    case IPROUTEDISCARDS:
        return (u_char *) & ipstat.dwRoutingDiscards;
#endif                          /* WIN32 */

    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_ip\n", vp->magic));
    }
    return NULL;

#ifdef HAVE_SYS_TCPIPSTATS_H
#undef	ipstat
#endif
}



        /*********************
	 *
	 *  Internal implementation functions
	 *
	 *********************/

long
read_ip_stat(IP_STAT_STRUCTURE * ipstat, int magic)
{
    long            ret_value = 0;
#if (defined(CAN_USE_SYSCTL) && defined(IPCTL_STATS))
    int             i;
#endif
#if !(defined (linux) || defined(solaris2))
    static int      ttl, forward;
#endif
#ifdef hpux11
    int             fd;
    struct nmparms  p;
    unsigned int    ulen;
#endif

#if (defined(CAN_USE_SYSCTL) && defined(IPCTL_STATS))
    static int      sname[4] = { CTL_NET, PF_INET, IPPROTO_IP, 0 };
    size_t          len;
#endif

#ifdef hpux11
    if ((fd = open_mib("/dev/ip", O_RDONLY, 0, NM_ASYNC_OFF)) < 0)
        return (-1);            /* error */

    switch (magic) {
    case IPFORWARDING:
        p.objid = ID_ipForwarding;
        break;
    case IPDEFAULTTTL:
        p.objid = ID_ipDefaultTTL;
        break;
    case IPINRECEIVES:
        p.objid = ID_ipInReceives;
        break;
    case IPINHDRERRORS:
        p.objid = ID_ipInHdrErrors;
        break;
    case IPINADDRERRORS:
        p.objid = ID_ipInAddrErrors;
        break;
    case IPFORWDATAGRAMS:
        p.objid = ID_ipForwDatagrams;
        break;
    case IPINUNKNOWNPROTOS:
        p.objid = ID_ipInUnknownProtos;
        break;
    case IPINDISCARDS:
        p.objid = ID_ipInDiscards;
        break;
    case IPINDELIVERS:
        p.objid = ID_ipInDelivers;
        break;
    case IPOUTREQUESTS:
        p.objid = ID_ipOutRequests;
        break;
    case IPOUTDISCARDS:
        p.objid = ID_ipOutDiscards;
        break;
    case IPOUTNOROUTES:
        p.objid = ID_ipOutNoRoutes;
        break;
    case IPREASMTIMEOUT:
        p.objid = ID_ipReasmTimeout;
        break;
    case IPREASMREQDS:
        p.objid = ID_ipReasmReqds;
        break;
    case IPREASMOKS:
        p.objid = ID_ipReasmOKs;
        break;
    case IPREASMFAILS:
        p.objid = ID_ipReasmFails;
        break;
    case IPFRAGOKS:
        p.objid = ID_ipFragOKs;
        break;
    case IPFRAGFAILS:
        p.objid = ID_ipFragFails;
        break;
    case IPFRAGCREATES:
        p.objid = ID_ipFragCreates;
        break;
    case IPROUTEDISCARDS:
        p.objid = ID_ipRoutingDiscards;
        break;
    default:
        *ipstat = 0;
        close_mib(fd);
        return (0);
    }

    p.buffer = (void *) ipstat;
    ulen = sizeof(IP_STAT_STRUCTURE);
    p.len = &ulen;
    ret_value = get_mib_info(fd, &p);
    close_mib(fd);

    return (ret_value);         /* 0: ok, < 0: error */
#else                           /* hpux11 */


    if (ip_stats_cache_marker &&
        (!atime_ready
         (ip_stats_cache_marker, IP_STATS_CACHE_TIMEOUT * 1000)))
#if !(defined(linux) || defined(solaris2))
        return ((magic == IPFORWARDING ? forward :
                 (magic == IPDEFAULTTTL ? ttl : 0)));
#else
        return 0;
#endif

    if (ip_stats_cache_marker)
        atime_setMarker(ip_stats_cache_marker);
    else
        ip_stats_cache_marker = atime_newMarker();


#ifdef linux
    ret_value = linux_read_ip_stat(ipstat);
#endif

#ifdef solaris2
    ret_value =
        getMibstat(MIB_IP, ipstat, sizeof(mib2_ip_t), GET_FIRST,
                   &Get_everything, NULL);
#endif

#ifdef WIN32
    ret_value = GetIpStatistics(ipstat);
#endif

#if !(defined(linux) || defined(solaris2) || defined(WIN32))
    if (magic == IPFORWARDING) {

#if defined(CAN_USE_SYSCTL) && defined(IPCTL_STATS)
        len = sizeof i;
        sname[3] = IPCTL_FORWARDING;
        if (sysctl(sname, 4, &i, &len, 0, 0) < 0)
            forward = -1;
        else
            forward = (i ? 1    /* GATEWAY */
                       : 2 /* HOST    */ );
#else
        if (!auto_nlist
            (IP_FORWARDING_SYMBOL, (char *) &ret_value, sizeof(ret_value)))
            forward = -1;
        else
            forward = (ret_value ? 1    /* GATEWAY */
                       : 2 /* HOST    */ );
#endif
        if (forward == -1) {
            free(ip_stats_cache_marker);
            ip_stats_cache_marker = NULL;
        }
        return forward;
    }

    if (magic == IPDEFAULTTTL) {

#if (defined(CAN_USE_SYSCTL) && defined(IPCTL_STATS))
        len = sizeof i;
        sname[3] = IPCTL_DEFTTL;
        if (sysctl(sname, 4, &i, &len, 0, 0) < 0)
            ttl = -1;
        else
            ttl = i;
#else
        if (!auto_nlist
            (TCP_TTL_SYMBOL, (char *) &ret_value, sizeof(ret_value)))
            ttl = -1;
        else
            ttl = ret_value;
#endif
        if (ttl == -1) {
            free(ip_stats_cache_marker);
            ip_stats_cache_marker = NULL;
        }
        return ttl;
    }

#ifdef HAVE_SYS_TCPIPSTATS_H
    ret_value = sysmp(MP_SAGET, MPSA_TCPIPSTATS, ipstat, sizeof *ipstat);
#endif

#if (defined(CAN_USE_SYSCTL) && defined(IPCTL_STATS))
    len = sizeof *ipstat;
    sname[3] = IPCTL_STATS;
    ret_value = sysctl(sname, 4, ipstat, &len, 0, 0);
#endif
#ifdef IPSTAT_SYMBOL
    if (auto_nlist(IPSTAT_SYMBOL, (char *) ipstat, sizeof(*ipstat)))
        ret_value = 0;
#endif
#endif                          /* !(defined(linux) || defined(solaris2)) */

    if (ret_value == -1) {
        free(ip_stats_cache_marker);
        ip_stats_cache_marker = NULL;
    }
    return ret_value;
#endif                          /* hpux11 */
}

#ifdef WIN32
int
writeIpStats(int action,
             u_char * var_val,
             u_char var_val_type,
             size_t var_val_len,
             u_char * statP, oid * name, size_t name_len)
{
    long           *buf, *oldbuf;
    MIB_IPSTATS     ipStats;
    int             var;
    int             retval = SNMP_ERR_NOERROR;
    /*
     * #define for ip scalar objects are 1 less than corresponding sub-id in MIB
     * * i.e. IPFORWARDING defined as 0, but ipForwarding registered as 1
     */
    var = name[7] - 1;
    switch (var) {
    case IPFORWARDING:
        buf = &ipForwarding;
        oldbuf = &oldipForwarding;
        break;
    case IPDEFAULTTTL:
        buf = &ipTTL;
        oldbuf = &oldipTTL;
        break;
    default:
        return SNMP_ERR_NOTWRITABLE;
    }

    switch (action) {
    case RESERVE1:             /* Check values for acceptability */
        if (var_val_type != ASN_INTEGER) {
            snmp_log(LOG_ERR, "not integer\n");
            return SNMP_ERR_WRONGTYPE;
        }
        if (var_val_len > sizeof(int)) {
            snmp_log(LOG_ERR, "bad length\n");
            return SNMP_ERR_WRONGLENGTH;
        }
        switch (var) {
        case IPFORWARDING:
            if (((int) *var_val < 1) || ((int) *var_val > 2)) {
                snmp_log(LOG_ERR, "not supported ip forwarding : %d\n",
                         *var_val);
                return SNMP_ERR_WRONGVALUE;
            }
            break;
        case IPDEFAULTTTL:
            if ((int) *var_val < 0) {
                snmp_log(LOG_ERR, "not supported ip Default : %d\n",
                         (int) *var_val);
                return SNMP_ERR_WRONGVALUE;
            }
        }
        break;

    case RESERVE2:             /* Allocate memory and similar resources */
        break;

    case ACTION:
        /*
         * Save the old value, in case of UNDO 
         */

        *oldbuf = *buf;
        *buf = (int) *var_val;
        break;

    case UNDO:                 /* Reverse the SET action and free resources */
        *buf = *oldbuf;
        break;

    case COMMIT:               /* Confirm the SET, performing any irreversible actions,
                                 * and free resources */
        switch (var) {
        case IPFORWARDING:
            /*
             * Currently windows supports only ON->OFF 
             */
            ipStats.dwForwarding = *buf;
            ipStats.dwDefaultTTL = MIB_USE_CURRENT_TTL;
            if (SetIpStatistics(&ipStats) != NO_ERROR) {
                retval = SNMP_ERR_COMMITFAILED;
                snmp_log(LOG_ERR,
                         "Can't set ipForwarding, supports only enable->disable \n");
            }
            break;
        case IPDEFAULTTTL:
            if (SetIpTTL((UINT) * buf) != NO_ERROR) {
                retval = SNMP_ERR_COMMITFAILED;
                snmp_log(LOG_ERR, "Can't set ipDefaultTTL\n");
            }
            break;
        }

    case FREE:                 /* Free any resources allocated */
        break;
    }
    return retval;
}                               /* end of writeIpStats */
#endif                          /* WIN32 */
