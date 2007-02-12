/*
 *  IP MIB group implementation - ipv6.c
 *
 */

#include <net-snmp/net-snmp-config.h>
#if defined(IFNET_NEEDS_KERNEL) && !defined(_KERNEL)
#define _KERNEL 1
#define _I_DEFINED_KERNEL
#endif
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
# if HAVE_SYS_SOCKETVAR_H
#  include <sys/socketvar.h>
# endif
#endif

#if STDC_HEADERS
#include <string.h>
#include <stdlib.h>
#else
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
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
#include <net/if.h>
#if HAVE_NET_IF_VAR_H
#include <net/if_var.h>
#endif
#if HAVE_NET_IF_DL_H
#include <net/if_dl.h>
#endif
#ifdef HAVE_NET_IF_MIB_H
#include <net/if_mib.h>
#endif
#ifdef _I_DEFINED_KERNEL
#undef _KERNEL
#endif
#include <netinet/in_systm.h>
#if HAVE_SYS_HASHING_H
#include <sys/hashing.h>
#endif
#if HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif
#if HAVE_NETINET6_IN6_VAR_H
#include <netinet6/in6_var.h>
#endif
#include <netinet/ip6.h>
#if HAVE_NETINET_IP_VAR_H
# include <netinet/ip_var.h>
#endif
#if HAVE_NETINET6_IP6_VAR_H
# include <netinet6/ip6_var.h>
#endif
#include <net/route.h>
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
# if HAVE_NETINET_IN_PCB_H
#  include <netinet/in_pcb.h>
# endif
#endif
#if HAVE_NETINET6_IN6_PCB_H
# include <netinet6/in6_pcb.h>
#endif
#if HAVE_NETINET6_TCP6_H
# define TCP6
#endif
#ifndef TCP6
# if HAVE_NETINET_TCP_H
#  include <netinet/tcp.h>
# endif
# if HAVE_NETINET_TCP_TIMER_H
#  include <netinet/tcp_timer.h>
# endif
# if HAVE_NETINET_TCP_VAR_H
#  include <netinet/tcp_var.h>
# endif
# if HAVE_NETINET_TCP_FSM_H
#  include <netinet/tcp_fsm.h>
# endif
#endif
#if HAVE_NETINET6_TCP6_H
# include <netinet6/tcp6.h>
#endif
#if HAVE_NETINET6_TCP6_TIMER_H
#include <netinet6/tcp6_timer.h>
#endif
#if HAVE_NETINET6_TCP6_VAR_H
#include <netinet6/tcp6_var.h>
#endif
#if HAVE_NETINET6_TCP6_FSM_H
#include <netinet6/tcp6_fsm.h>
#endif
#if HAVE_INET_MIB2_H
#include <inet/mib2.h>
#endif
#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#endif

#ifdef MIB_IPCOUNTER_SYMBOL
#include <sys/mib.h>
#include <netinet/mib_kern.h>
#endif                          /* MIB_IPCOUNTER_SYMBOL */

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/auto_nlist.h>

#include "kernel.h"
#include "util_funcs.h"
#include "ipv6.h"
#include "interfaces.h"

static int header_ipv6
__P((register struct variable *, oid *, size_t *, int, size_t *,
     WriteMethod **));
static int header_ipv6_scan
__P((register struct variable *, oid *, size_t *, int, size_t *,
     WriteMethod **, int, int));
static int if_initialize __P((void));
static int if_maxifindex __P((void));
static char    *if_getname __P((int));
static int if_getindex __P((const char *));

struct variable3 ipv6_variables[] = {
    {IPV6FORWARDING, ASN_INTEGER, RONLY, var_ipv6, 1, {1}},
    {IPV6DEFAULTHOPLIMIT, ASN_INTEGER, RONLY, var_ipv6, 1, {2}},
    {IPV6INTERFACES, ASN_GAUGE, RONLY, var_ipv6, 1, {3}},
    {IPV6IFTBLLASTCHG, ASN_TIMETICKS, RONLY, var_ipv6, 1, {4}},

    {IPV6IFDESCR, ASN_OCTET_STR, RONLY, var_ifv6Entry, 3, {5, 1, 2}},
    {IPV6IFLOWLAYER, ASN_OBJECT_ID, RONLY, var_ifv6Entry, 3, {5, 1, 3}},
    {IPV6IFEFFECTMTU, ASN_UNSIGNED, RONLY, var_ifv6Entry, 3, {5, 1, 4}},
    {IPV6IFREASMMAXSIZE, ASN_UNSIGNED, RONLY, var_ifv6Entry, 3, {5, 1, 5}},

    {IPV6IFTOKEN, ASN_OCTET_STR, RONLY, var_ifv6Entry, 3, {5, 1, 6}},
    {IPV6IFTOKENLEN, ASN_INTEGER, RONLY, var_ifv6Entry, 3, {5, 1, 7}},
    {IPV6IFPHYSADDRESS, ASN_OCTET_STR, RONLY, var_ifv6Entry, 3, {5, 1, 8}},
    {IPV6IFADMSTATUS, ASN_INTEGER, RONLY, var_ifv6Entry, 3, {5, 1, 9}},
    {IPV6IFOPERSTATUS, ASN_INTEGER, RONLY, var_ifv6Entry, 3, {5, 1, 10}},
    {IPV6IFLASTCHANGE, ASN_TIMETICKS, RONLY, var_ifv6Entry, 3, {5, 1, 11}},

    {IPV6IFSTATSINRCVS, ASN_COUNTER, RONLY, var_ifv6Entry, 3, {6, 1, 1}},
    {IPV6IFSTATSINHDRERRS, ASN_COUNTER, RONLY, var_ifv6Entry, 3,
     {6, 1, 2}},
    {IPV6IFSTATSTOOBIGERRS, ASN_COUNTER, RONLY, var_ifv6Entry, 3,
     {6, 1, 3}},
    {IPV6IFSTATSINNOROUTES, ASN_COUNTER, RONLY, var_ifv6Entry, 3,
     {6, 1, 4}},
    {IPV6IFSTATSINADDRERRS, ASN_COUNTER, RONLY, var_ifv6Entry, 3,
     {6, 1, 5}},
    {IPV6IFSTATSINUNKNOWPROTS, ASN_COUNTER, RONLY, var_ifv6Entry, 3,
     {6, 1, 6}},
    {IPV6IFSTATSINTRUNCATPKTS, ASN_COUNTER, RONLY, var_ifv6Entry, 3,
     {6, 1, 7}},
    {IPV6IFSTATSINDISCARDS, ASN_COUNTER, RONLY, var_ifv6Entry, 3,
     {6, 1, 8}},
    {IPV6IFSTATSINDELIVERS, ASN_COUNTER, RONLY, var_ifv6Entry, 3,
     {6, 1, 9}},
    {IPV6IFSTATSOUTFORWDATAS, ASN_COUNTER, RONLY, var_ifv6Entry, 3,
     {6, 1, 10}},
    {IPV6IFSTATSOUTREQS, ASN_COUNTER, RONLY, var_ifv6Entry, 3, {6, 1, 11}},
    {IPV6IFSTATSOUTDISCARDS, ASN_COUNTER, RONLY, var_ifv6Entry, 3,
     {6, 1, 12}},
    {IPV6IFSTATSOUTFRAGOKS, ASN_COUNTER, RONLY, var_ifv6Entry, 3,
     {6, 1, 13}},
    {IPV6IFSTATSOUTFRAGFAILS, ASN_COUNTER, RONLY, var_ifv6Entry, 3,
     {6, 1, 14}},
    {IPV6IFSTATSOUTFRAGCREATS, ASN_COUNTER, RONLY, var_ifv6Entry, 3,
     {6, 1, 15}},
    {IPV6IFSTATSOUTREASMREQS, ASN_COUNTER, RONLY, var_ifv6Entry, 3,
     {6, 1, 16}},
    {IPV6IFSTATSOUTREASMOKS, ASN_COUNTER, RONLY, var_ifv6Entry, 3,
     {6, 1, 17}},
    {IPV6IFSTATSOUTREASMFAILS, ASN_COUNTER, RONLY, var_ifv6Entry, 3,
     {6, 1, 18}},
    {IPV6IFSTATSINMCASTPKTS, ASN_COUNTER, RONLY, var_ifv6Entry, 3,
     {6, 1, 19}},
    {IPV6IFSTATSOUTMCASTPKTS, ASN_COUNTER, RONLY, var_ifv6Entry, 3,
     {6, 1, 20}},

#if 0
    {IPV6ADDRPREFIXONLINKFLG, INTEGER, RONLY, var_ipv6AddrEntry, 3,
     {7, 1, 3}},
    {IPV6ADDRPREFIXAUTONOMOUSFLAG, INTEGER, RONLY, var_ipv6AddrEntry, 3,
     {7, 1, 4}},
    {IPV6ADDRPREFIXADVPREFERLIFE, UNSIGNED32, RONLY, var_ipv6AddrEntry, 3,
     {7, 1, 5}},
    {IPV6ADDRPREFIXVALIDLIFE, UNSIGNED32, RONLY, var_ipv6AddrEntry, 3,
     {7, 1, 6}},

    {IPV6ADDRPFXLEN, INTEGER, RONLY, var_ipv6AddrEntry, 3, {8, 1, 2}},
    {IPV6ADDRTYPE, INTEGER, RONLY, var_ipv6AddrEntry, 3, {8, 1, 3}},
    {IPV6ADDRANYCASTFLAG, INTEGER, RONLY, var_ipv6AddrEntry, 3, {8, 1, 4}},
    {IPV6ADDRSTATUS, INTEGER, RONLY, var_ipv6AddrEntry, 3, {8, 1, 5}},

    {IPV6ROUTEIFINDEX, IpV6IFINDEX, RONLY, var_ipv6RouteEntry, 3,
     {11, 1, 4}},
    {IPV6ROUTENEXTHOP, IpV6ADDRESS, RONLY, var_ipv6RouteEntry, 3,
     {11, 1, 5}},
    {IPV6ROUTETYPE, INTEGER, RONLY, var_ipv6RouteEntry, 3, {11, 1, 6}},
    {IPV6ROUTEPROTOCOL, INTEGER, RONLY, var_ipv6RouteEntry, 3, {11, 1, 7}},
    {IPV6ROUTEPOLICY, UNSIGNED32, RONLY, var_ipv6RouteEntry, 3,
     {11, 1, 8}},
    {IPV6ROUTEAGE, UNSIGNED32, RONLY, var_ipv6RouteEntry, 3, {11, 1, 9}},
    {IPV6ROUTENEXTHOPRDI, UNSIGNED32, RONLY, var_ipv6RouteEntry, 3,
     {11, 1, 10}},
    {IPV6ROUTEMETRIC, UNSIGNED32, RONLY, var_ipv6RouteEntry, 3,
     {11, 1, 11}},
    {IPV6ROUTEWEIGHT, UNSIGNED32, RONLY, var_ipv6RouteEntry, 3,
     {11, 1, 12}},
    {IPV6ROUTEINFO, OBJID, RONLY, var_ipv6RouteEntry, 3, {11, 1, 13}},
    {IPV6ROUTEVALID, INTEGER, RONLY, var_ipv6RouteEntry, 3, {11, 1, 14}},

    {IPV6NETTOMEDIAPHYADDR, STRING, RONLY, var_ndpEntry, 3, {12, 1, 2}},
    {IPV6NETTOMEDIATYPE, INTEGER, RONLY, var_ndpEntry, 3, {12, 1, 3}},
    {IPV6NETTOMEDIASTATE, INTEGER, RONLY, var_ndpEntry, 3, {12, 1, 4}},
    {IPV6NETTOMEDIALASTUPDATE, TIMETICKS, RONLY, var_ndpEntry, 3,
     {12, 1, 5}},
    {IPV6NETTOMEDIAVALID, INTEGER, RONLY, var_ndpEntry, 3, {12, 1, 6}},
#endif
};
oid             ipv6_variables_oid[] = { SNMP_OID_MIB2, 55, 1 };
#if 0
config_load_mib(MIB .55 .1, 8, ipv6_variables)
    config_add_mib(IPV6 - TC)
    config_add_mib(IPV6 - MIB)
#endif
     struct variable3 ipv6icmp_variables[] = {
         {IPV6IFICMPINMSG, ASN_COUNTER, RONLY, var_icmpv6Entry, 3,
          {1, 1, 1}},
         {IPV6IFICMPINERRORS, ASN_COUNTER, RONLY, var_icmpv6Entry, 3,
          {1, 1, 2}},
         {IPV6IFICMPINDSTUNRCHS, ASN_COUNTER, RONLY, var_icmpv6Entry, 3,
          {1, 1, 3}},
         {IPV6IFICMPINADMPROHS, ASN_COUNTER, RONLY, var_icmpv6Entry, 3,
          {1, 1, 4}},
         {IPV6IFICMPINTIMEXCDS, ASN_COUNTER, RONLY, var_icmpv6Entry, 3,
          {1, 1, 5}},
         {IPV6IFICMPINPARMPROBS, ASN_COUNTER, RONLY, var_icmpv6Entry, 3,
          {1, 1, 6}},
         {IPV6IFICMPINPKTTOOBIGS, ASN_COUNTER, RONLY, var_icmpv6Entry, 3,
          {1, 1, 7}},
         {IPV6IFICMPINECHOS, ASN_COUNTER, RONLY, var_icmpv6Entry, 3,
          {1, 1, 8}},
         {IPV6IFICMPINECHOREPS, ASN_COUNTER, RONLY, var_icmpv6Entry, 3,
          {1, 1, 9}},
         {IPV6IFICMPINRTRSLICITS, ASN_COUNTER, RONLY, var_icmpv6Entry, 3,
          {1, 1, 10}},
         {IPV6IFICMPINRTRADVS, ASN_COUNTER, RONLY, var_icmpv6Entry, 3,
          {1, 1, 11}},
         {IPV6IFICMPINNBRSLICITS, ASN_COUNTER, RONLY, var_icmpv6Entry, 3,
          {1, 1, 12}},
         {IPV6IFICMPINNBRADVS, ASN_COUNTER, RONLY, var_icmpv6Entry, 3,
          {1, 1, 13}},
         {IPV6IFICMPINREDIRECTS, ASN_COUNTER, RONLY, var_icmpv6Entry, 3,
          {1, 1, 14}},
         {IPV6IFICMPINGRPMEQERYS, ASN_COUNTER, RONLY, var_icmpv6Entry, 3,
          {1, 1, 15}},
         {IPV6IFICMPINGRPMERSPS, ASN_COUNTER, RONLY, var_icmpv6Entry, 3,
          {1, 1, 16}},
         {IPV6IFICMPINGRPMEREDCS, ASN_COUNTER, RONLY, var_icmpv6Entry, 3,
          {1, 1, 17}},
         {IPV6IFICMPOUTMSG, ASN_COUNTER, RONLY, var_icmpv6Entry, 3,
          {1, 1, 18}},
         {IPV6IFICMPOUTERRORS, ASN_COUNTER, RONLY, var_icmpv6Entry, 3,
          {1, 1, 19}},
         {IPV6IFICMPOUTDSTUNRCHS, ASN_COUNTER, RONLY, var_icmpv6Entry, 3,
          {1, 1, 20}},
         {IPV6IFICMPOUTADMPROHS, ASN_COUNTER, RONLY, var_icmpv6Entry, 3,
          {1, 1, 21}},
         {IPV6IFICMPOUTTIMEXCDS, ASN_COUNTER, RONLY, var_icmpv6Entry, 3,
          {1, 1, 22}},
         {IPV6IFICMPOUTPARMPROBS, ASN_COUNTER, RONLY, var_icmpv6Entry, 3,
          {1, 1, 23}},
         {IPV6IFICMPOUTPKTTOOBIGS, ASN_COUNTER, RONLY, var_icmpv6Entry, 3,
          {1, 1, 24}},
         {IPV6IFICMPOUTECHOS, ASN_COUNTER, RONLY, var_icmpv6Entry, 3,
          {1, 1, 25}},
         {IPV6IFICMPOUTECHOREPS, ASN_COUNTER, RONLY, var_icmpv6Entry, 3,
          {1, 1, 26}},
         {IPV6IFICMPOUTRTRSLICITS, ASN_COUNTER, RONLY, var_icmpv6Entry, 3,
          {1, 1, 27}},
         {IPV6IFICMPOUTRTRADVS, ASN_COUNTER, RONLY, var_icmpv6Entry, 3,
          {1, 1, 28}},
         {IPV6IFICMPOUTNBRSLICITS, ASN_COUNTER, RONLY, var_icmpv6Entry, 3,
          {1, 1, 29}},
         {IPV6IFICMPOUTNBRADVS, ASN_COUNTER, RONLY, var_icmpv6Entry, 3,
          {1, 1, 30}},
         {IPV6IFICMPOUTREDIRECTS, ASN_COUNTER, RONLY, var_icmpv6Entry, 3,
          {1, 1, 31}},
         {IPV6IFICMPOUTGRPMEQERYS, ASN_COUNTER, RONLY, var_icmpv6Entry, 3,
          {1, 1, 32}},
         {IPV6IFICMPOUTGRPMERSPS, ASN_COUNTER, RONLY, var_icmpv6Entry, 3,
          {1, 1, 33}},
         {IPV6IFICMPOUTGRPMEREDCS, ASN_COUNTER, RONLY, var_icmpv6Entry, 3,
          {1, 1, 34}}
     };
oid             ipv6icmp_variables_oid[] = { 1, 3, 6, 1, 2, 1, 56, 1 };
#if 0
config_load_mib(MIB .56 .1, 8, ipv6icmp_variables)
    config_add_mib(IPV6 - ICMP - MIB)
#endif
     struct variable2 ipv6udp_variables[] = {
         {IPV6UDPLOCALADDRESS, ASN_OCTET_STR, RONLY, var_udp6, 2, {1, 1}},
         {IPV6UDPLOCALPORT, ASN_INTEGER, RONLY, var_udp6, 2, {1, 2}},
         {IPV6UDPIFINDEX, ASN_INTEGER, RONLY, var_udp6, 2, {1, 3}}
     };
oid             ipv6udp_variables_oid[] = { 1, 3, 6, 1, 2, 1, 7, 6 };
#if 0
config_load_mib(1.3 .6 .1 .3 .87 .1, 7, ipv6udp_variables)
    config_add_mib(IPV6 - UDP - MIB)
#endif
     struct variable2 ipv6tcp_variables[] = {
         {IPV6TCPLOCALADDR, ASN_OCTET_STR, RONLY, var_tcp6, 2, {1, 1}},
         {IPV6TCPLOCALPORT, ASN_INTEGER, RONLY, var_tcp6, 2, {1, 2}},
         {IPV6TCPREMOTEADDR, ASN_OCTET_STR, RONLY, var_tcp6, 2, {1, 3}},
         {IPV6TCPREMOTEPORT, ASN_INTEGER, RONLY, var_tcp6, 2, {1, 4}},
         {IPV6TCPIFINDEX, ASN_INTEGER, RONLY, var_tcp6, 2, {1, 5}},
         {IPV6TCPCONNSTATE, ASN_INTEGER, RONLY, var_tcp6, 2, {1, 6}},
     };
oid             ipv6tcp_variables_oid[] = { 1, 3, 6, 1, 2, 1, 6, 16 };
#if 0
config_load_mib(1.3 .6 .1 .3 .86 .1, 7, ipv6tcp_variables)
    config_add_mib(IPV6 - TCP - MIB)
#endif
     void
                     init_ipv6()
{
    /*
     * register ourselves with the agent to handle our mib tree 
     */
    REGISTER_MIB("mibII/ipv6", ipv6_variables, variable3,
                 ipv6_variables_oid);
    REGISTER_MIB("mibII/icmpv6", ipv6icmp_variables, variable3,
                 ipv6icmp_variables_oid);
    REGISTER_MIB("mibII/ipv6udp", ipv6udp_variables, variable2,
                 ipv6udp_variables_oid);
    REGISTER_MIB("mibII/ipv6tcp", ipv6tcp_variables, variable2,
                 ipv6tcp_variables_oid);
}

static int
header_ipv6(register struct variable *vp,
            /*
             * IN - pointer to variable entry that points here 
             */
            oid * name,         /* IN/OUT - input name requested, output name found */
            size_t * length,    /* IN/OUT - length of input and output oid's */
            int exact,          /* IN - TRUE if an exact match was requested */
            size_t * var_len,   /* OUT - length of variable or 0 if function returned */
            WriteMethod ** write_method)
{
    oid             newname[MAX_OID_LEN];
    int             result;

    DEBUGMSGTL(("mibII/ipv6", "header_ipv6: "));
    DEBUGMSGOID(("mibII/ipv6", name, *length));
    DEBUGMSG(("mibII/ipv6", " %d\n", exact));

    memcpy((char *) newname, (char *) vp->name,
           (int) vp->namelen * sizeof(oid));
    newname[(int) vp->namelen] = 0;
    result =
        snmp_oid_compare(name, *length, newname, (int) vp->namelen + 1);
    if ((exact && (result != 0)) || (!exact && (result >= 0)))
        return (MATCH_FAILED);
    memcpy((char *) name, (char *) newname,
           ((int) vp->namelen + 1) * sizeof(oid));
    *length = vp->namelen + 1;

    *write_method = 0;
    *var_len = sizeof(long);    /* default to 'long' results */
    return (MATCH_SUCCEEDED);
}

static int
header_ipv6_scan(register struct variable *vp,
                 /*
                  * IN - pointer to variable entry that points here 
                  */
                 oid * name,    /* IN/OUT - input name requested, output name found */
                 size_t * length,       /* IN/OUT - length of input and output oid's */
                 int exact,     /* IN - TRUE if an exact match was requested */
                 size_t * var_len,      /* OUT - length of variable or 0 if function returned */
                 WriteMethod ** write_method, int from, int to)
{
    oid             newname[MAX_OID_LEN];
    int             result;
    int             i;

    DEBUGMSGTL(("mibII/ipv6", "header_ipv6_scan: "));
    DEBUGMSGOID(("mibII/ipv6", name, *length));
    DEBUGMSG(("mibII/ipv6", " %d\n", exact));

    memcpy((char *) newname, (char *) vp->name,
           (int) vp->namelen * sizeof(oid));
    for (i = from; i <= to; i++) {
        newname[(int) vp->namelen] = i;
        result =
            snmp_oid_compare(name, *length, newname,
                             (int) vp->namelen + 1);
        if (((exact && result == 0) || (!exact && result < 0))
            && if_getname(i))
            break;
    }
    if (to < i)
        return (MATCH_FAILED);
    memcpy((char *) name, (char *) newname,
           ((int) vp->namelen + 1) * sizeof(oid));
    *length = vp->namelen + 1;
    *write_method = 0;
    *var_len = sizeof(long);    /* default to 'long' results */
    return (MATCH_SUCCEEDED);
}

static struct if_nameindex *ifnames = NULL;

#ifdef linux
static void     linux_if_freenameindex(struct if_nameindex *);
static struct if_nameindex *linux_if_nameindex(void);
#endif

static int
if_initialize(void)
{
#ifndef HAVE_IF_NAMEINDEX
    return -1;
#else
#ifndef linux
    if (ifnames)
        if_freenameindex(ifnames);
    ifnames = if_nameindex();
#else
    if (ifnames)
        linux_if_freenameindex(ifnames);
    ifnames = linux_if_nameindex();
#endif
    if (!ifnames) {
        ERROR_MSG("if_nameindex() failed");
        return -1;
    }
    return 0;
#endif
}

static int
if_maxifindex(void)
{
#ifndef HAVE_IF_NAMEINDEX
    return -1;
#else
    struct if_nameindex *p;
    int             max = 0;

    if (!ifnames) {
        if (if_initialize() < 0)
            return -1;
    }
    for (p = ifnames; p && p->if_index; p++) {
        if (max < p->if_index)
            max = p->if_index;
    }
    return max;
#endif
}

static int
if_countifindex(void)
{
#ifndef HAVE_IF_NAMEINDEX
    return -1;
#else
    struct if_nameindex *p;
    int             count = 0;

    if (!ifnames) {
        if (if_initialize() < 0)
            return -1;
    }
    for (p = ifnames; p && p->if_index; p++) {
        count++;
    }
    return count;
#endif
}

static char    *
if_getname(int idx)
{
#ifndef HAVE_IF_NAMEINDEX
    return NULL;
#else
    struct if_nameindex *p;
    int             max = -1;

    if (!ifnames) {
        if (if_initialize() < 0)
            return NULL;
    }
    for (p = ifnames; p && p->if_index; p++) {
        if (p->if_index == idx)
            return p->if_name;
    }
    return NULL;
#endif
}

static int
if_getindex(const char *name)
{
#ifndef HAVE_IF_NAMEINDEX
    return -1;
#else
    struct if_nameindex *p;

    if (!ifnames) {
        if (if_initialize() < 0)
            return -1;
    }
    for (p = ifnames; p && p->if_index; p++) {
        if (strcmp(name, p->if_name) == 0)
            return p->if_index;
    }
    return -1;
#endif
}

/*------------------------------------------------------------*/
#ifndef linux
/*
 * KAME dependent part 
 */
static int
if_getifnet(int idx, struct ifnet *result)
{
    caddr_t         q;
    struct ifnet    tmp;

    if (!auto_nlist("ifnet", (char *) &q, sizeof(q)))
        return -1;
    while (q) {
        klookup((u_long) q, (char *) &tmp, sizeof(tmp));
        if (idx == tmp.if_index) {
            memcpy(result, &tmp, sizeof(tmp));
            return 0;
        }
#if defined(__FreeBSD__) && 3 <= __FreeBSD__
        q = (caddr_t) TAILQ_NEXT(&tmp, if_link);
#else
# if defined(__NetBSD__) || defined(__OpenBSD__)
        q = (caddr_t) TAILQ_NEXT(&tmp, if_list);
# else
        q = (caddr_t) tmp.if_next;
# endif
#endif
    }
    return -1;
}

#ifdef HAVE_NET_IF_MIB_H
#if defined(HAVE_SYS_SYSCTL_H) && defined(CTL_NET)
static int
if_getifmibdata(int idx, struct ifmibdata *result)
{
    int             mib[6] = {
        CTL_NET, PF_LINK, NETLINK_GENERIC, IFMIB_IFDATA, 0, IFDATA_GENERAL
    };
    int             len;
    struct ifmibdata tmp;

    mib[4] = idx;
    len = sizeof(struct ifmibdata);
    if (sysctl(mib, sizeof(mib) / sizeof(mib[0]), &tmp, &len, 0, 0) < 0)
        return -1;
    memcpy(result, &tmp, sizeof(tmp));
    return 0;
}
#endif
#endif                          /*HAVE_NET_IF_MIB_H */

#ifdef __KAME__
#define IPV6_FORWARDING_SYMBOL	"ip6_forwarding"
#define IPV6_DEFHLIM_SYMBOL	"ip6_defhlim"
#endif

u_char         *
var_ipv6(register struct variable * vp,
         oid * name,
         size_t * length,
         int exact, size_t * var_len, WriteMethod ** write_method)
{
    int             i;

    if (header_ipv6(vp, name, length, exact, var_len, write_method)
        == MATCH_FAILED) {
        return NULL;
    }
#if defined(HAVE_SYS_SYSCTL_H) && defined(CTL_NET)
    /*
     * try with sysctl routines 
     */
    {
        int             name[] = { CTL_NET, PF_INET6, IPPROTO_IPV6, 0 };
        const char     *namestr = NULL;
        int             result;
        int             resultsiz;

        resultsiz = sizeof(result);
        switch (vp->magic) {
        case IPV6FORWARDING:
            name[3] = IPV6CTL_FORWARDING;
            namestr = "IPV6CTL_FORWARDING";
            if (sysctl
                (name, sizeof(name) / sizeof(name[0]), &result, &resultsiz,
                 0, 0) < 0) {
                DEBUGMSGTL(("mibII/ipv6",
                            "sysctl(CTL_NET, PF_INET6, IPPROTO_IPV6, %s)\n",
                            namestr));
                break;
            } else {
                if (result)
                    long_return = 1;    /* GATEWAY */
                else
                    long_return = 2;    /* HOST */
                return (u_char *) & long_return;
            }
            break;
        case IPV6DEFAULTHOPLIMIT:
            name[3] = IPV6CTL_DEFHLIM;
            namestr = "IPV6CTL_DEFHLIM";
            if (sysctl
                (name, sizeof(name) / sizeof(name[0]), &result, &resultsiz,
                 0, 0) < 0) {
                DEBUGMSGTL(("mibII/ipv6",
                            "sysctl(CTL_NET, PF_INET6, IPPROTO_IPV6, %s)\n",
                            namestr));
                break;
            } else {
                long_return = result;
                return (u_char *) & long_return;
            }
        }
    }
#endif                          /* not (HAVE_SYS_SYSCTL_H && CTL_NET) */

    /*
     * try looking into the kernel variable 
     */
    switch (vp->magic) {
#ifdef IPV6_FORWARDING_SYMBOL
    case IPV6FORWARDING:
        if (auto_nlist(IPV6_FORWARDING_SYMBOL, (char *) &i, sizeof(i))) {
            if (i)
                long_return = 1;
            /*GATEWAY*/
            else
                long_return = 2;
            /*HOST*/ return (u_char *) & long_return;
        }
        break;
#endif
#ifdef IPV6_DEFHLIM_SYMBOL
    case IPV6DEFAULTHOPLIMIT:
        if (auto_nlist(IPV6_DEFHLIM_SYMBOL, (char *) &i, sizeof(i))) {
            long_return = i;
            return (u_char *) & long_return;
        }
        break;
#endif
    case IPV6INTERFACES:
#ifdef HAVE_IF_NAMEINDEX
        /*
         * not really the right answer... we must count IPv6 capable
         * interfaces only.
         */
        long_return = if_countifindex();
        if (long_return < 0)
            break;
        return (u_char *) & long_return;
#endif
        break;
#if 0
    case IPV6IFTBLLASTCHG:
        long_return = 0;
        /*XXX*/ return (u_char *) & long_return;
#endif
    default:
        break;
    }
    ERROR_MSG("");
    return NULL;
}

u_char         *
var_ifv6Entry(register struct variable * vp,
              oid * name,
              size_t * length,
              int exact, size_t * var_len, WriteMethod ** write_method)
{
#ifndef HAVE_IF_NAMEINDEX
    return NULL;
#else
    int             interface;
    int             max;
    char           *p;

    max = if_maxifindex();
    if (max < 0)
        return NULL;

    if (header_ipv6_scan
        (vp, name, length, exact, var_len, write_method, 1, max)
        == MATCH_FAILED) {
        return NULL;
    }
    interface = name[*length - 1];
    DEBUGMSGTL(("mibII/ipv6", "interface: %d(%s)\n",
                interface, if_getname(interface)));
    if (interface > max)
        return NULL;

    switch (vp->magic) {
    case IPV6IFDESCR:
        p = if_getname(interface);
        if (p) {
            *var_len = strlen(p);
            return p;
        }
        break;
    case IPV6IFLOWLAYER:
        /*
         * should check if type, this is a hard one... 
         */
        *var_len = nullOidLen;
        return (u_char *) nullOid;
    case IPV6IFEFFECTMTU:
        {
#if defined(SIOCGIFMTU) && !defined(__OpenBSD__)
            struct ifreq    ifr;
            int             s;

            memset(&ifr, 0, sizeof(ifr));
            ifr.ifr_addr.sa_family = AF_INET6;
            strncpy(ifr.ifr_name, if_getname(interface),
                    sizeof(ifr.ifr_name));
            if ((s = socket(ifr.ifr_addr.sa_family, SOCK_DGRAM, 0)) < 0)
                break;
            if (ioctl(s, SIOCGIFMTU, (caddr_t) & ifr) < 0) {
                close(s);
                break;
            }
            long_return = ifr.ifr_mtu;
            close(s);
            return (u_char *) & long_return;
#else
            break;
#endif
        }
#if 0                           /*not yet */
    case IPV6IFREASMMAXSIZE:
        /*
         * I dunno what the spec means for this MIB 
         */
    case IPV6IFTOKEN:
    case IPV6IFTOKENLEN:
#endif
    case IPV6IFPHYSADDRESS:
        {
            struct ifnet    ifnet;
            struct ifaddr   ifaddr;
            static struct sockaddr_dl sdl;
            caddr_t         ifa;

            if (if_getifnet(interface, &ifnet) < 0)
                break;
#if defined(__FreeBSD__) && 3 <= __FreeBSD__
            ifa = (caddr_t) TAILQ_FIRST(&ifnet.if_addrhead);
#else
# if defined(__NetBSD__) || defined(__OpenBSD__)
            ifa = (caddr_t) TAILQ_FIRST(&ifnet.if_addrlist);
# else
            ifa = (caddr_t) ifnet.if_addrlist;
# endif
#endif
            while (ifa) {
                klookup((u_long) ifa, (char *) &ifaddr, sizeof(ifaddr));
                klookup((u_long) ifaddr.ifa_addr, (char *) &sdl,
                        sizeof(sdl));
                if (sdl.sdl_family == AF_LINK) {
                    if (sizeof(sdl.sdl_data) < sdl.sdl_nlen + sdl.sdl_alen) {
                        ERROR_MSG("sdl_alen too long for interface\n");
                        break;
                    }
                    *var_len = sdl.sdl_alen;
                    return (u_char *) (sdl.sdl_data + sdl.sdl_nlen);
                }
#if defined(__FreeBSD__) && 3 <= __FreeBSD__
                ifa = (caddr_t) TAILQ_NEXT(&ifaddr, ifa_link);
#else
# if defined(__NetBSD__) || defined(__OpenBSD__)
                ifa = (caddr_t) TAILQ_NEXT(&ifaddr, ifa_list);
# else
                ifa = (caddr_t) ifaddr.ifa_next;
# endif
#endif
            }

            /*
             * no physical address found 
             */
            *var_len = 0;
            return NULL;
        }
    case IPV6IFADMSTATUS:
        {
            struct ifnet    ifnet;

            if (if_getifnet(interface, &ifnet) < 0)
                break;
            long_return = (ifnet.if_flags & IFF_RUNNING) ? 1 : 2;
            return (u_char *) & long_return;
        }
    case IPV6IFOPERSTATUS:
        {
            struct ifnet    ifnet;

            if (if_getifnet(interface, &ifnet) < 0)
                break;
            long_return = (ifnet.if_flags & IFF_UP) ? 1 : 2;
            return (u_char *) & long_return;
        }
#if 0                           /*untrustable value returned... */
    case IPV6IFLASTCHANGE:
        {
            struct timeval  lastchange;
            struct timeval  now;
            int             gotanswer;

            gotanswer = 0;
            lastchange.tv_sec = lastchange.tv_usec = 0;
#ifdef HAVE_NET_IF_MIB_H
            if (!gotanswer) {
                struct ifmibdata ifmd;

                if (if_getifmibdata(interface, &ifmd) < 0);
                else {
                    lastchange = ifmd.ifmd_data.ifi_lastchange;
                    gotanswer++;
                }
            }
#endif
#ifdef STRUCT_IFNET_HAS_IF_LASTCHANGE_TV_SEC
            if (!gotanswer) {
                struct ifnet    ifnet;

                if (if_getifnet(interface, &ifnet) < 0);
                else {
                    lastchange = ifnet.if_lastchange;
                    gotanswer++;
                }
            }
#endif
            DEBUGMSGTL(("mibII/ipv6", "lastchange = { %d.%06d }\n",
                        lastchange.tv_sec, lastchange.tv_usec));
            if (lastchange.tv_sec == 0 && lastchange.tv_usec == 0)
                long_return = 0;
            else {
                gettimeofday(&now, (struct timezone *) NULL);
                long_return =
                    (u_long) ((now.tv_sec - lastchange.tv_sec) * 100);
                long_return +=
                    (u_long) ((now.tv_usec - lastchange.tv_usec) / 10000);
            }
            return (u_char *) & long_return;
        }
#endif                          /*0 */

#ifdef SIOCGIFSTAT_IN6
    case IPV6IFSTATSINRCVS:
    case IPV6IFSTATSINHDRERRS:
    case IPV6IFSTATSTOOBIGERRS:
    case IPV6IFSTATSINNOROUTES:
    case IPV6IFSTATSINADDRERRS:
    case IPV6IFSTATSINUNKNOWPROTS:
    case IPV6IFSTATSINTRUNCATPKTS:
    case IPV6IFSTATSINDISCARDS:
    case IPV6IFSTATSINDELIVERS:
    case IPV6IFSTATSOUTFORWDATAS:
    case IPV6IFSTATSOUTREQS:
    case IPV6IFSTATSOUTDISCARDS:
    case IPV6IFSTATSOUTFRAGOKS:
    case IPV6IFSTATSOUTFRAGFAILS:
    case IPV6IFSTATSOUTFRAGCREATS:
    case IPV6IFSTATSOUTREASMREQS:
    case IPV6IFSTATSOUTREASMOKS:
    case IPV6IFSTATSOUTREASMFAILS:
    case IPV6IFSTATSINMCASTPKTS:
    case IPV6IFSTATSOUTMCASTPKTS:
        {
            struct in6_ifstat *ifs6;
            struct in6_ifreq ifr;
            int             s;

            memset(&ifr, 0, sizeof(ifr));
            strncpy(ifr.ifr_name, if_getname(interface),
                    sizeof(ifr.ifr_name));
            if ((s = socket(AF_INET6, SOCK_DGRAM, 0)) < 0)
                break;
            if (ioctl(s, SIOCGIFSTAT_IN6, (caddr_t) & ifr) < 0) {
                extern int      errno;
                close(s);
                break;
            }
            close(s);
            ifs6 = &ifr.ifr_ifru.ifru_stat;
            switch (vp->magic) {
            case IPV6IFSTATSINRCVS:
                long_return = ifs6->ifs6_in_receive;
                break;
            case IPV6IFSTATSINHDRERRS:
                long_return = ifs6->ifs6_in_hdrerr;
                break;
            case IPV6IFSTATSTOOBIGERRS:
                long_return = ifs6->ifs6_in_toobig;
                break;
            case IPV6IFSTATSINNOROUTES:
                long_return = ifs6->ifs6_in_noroute;
                break;
            case IPV6IFSTATSINADDRERRS:
                long_return = ifs6->ifs6_in_addrerr;
                break;
            case IPV6IFSTATSINUNKNOWPROTS:
                long_return = ifs6->ifs6_in_protounknown;
                break;
            case IPV6IFSTATSINTRUNCATPKTS:
                long_return = ifs6->ifs6_in_truncated;
                break;
            case IPV6IFSTATSINDISCARDS:
                long_return = ifs6->ifs6_in_discard;
                break;
            case IPV6IFSTATSINDELIVERS:
                long_return = ifs6->ifs6_in_deliver;
                break;
            case IPV6IFSTATSOUTFORWDATAS:
                long_return = ifs6->ifs6_out_forward;
                break;
            case IPV6IFSTATSOUTREQS:
                long_return = ifs6->ifs6_out_request;
                break;
            case IPV6IFSTATSOUTDISCARDS:
                long_return = ifs6->ifs6_out_discard;
                break;
            case IPV6IFSTATSOUTFRAGOKS:
                long_return = ifs6->ifs6_out_fragok;
                break;
            case IPV6IFSTATSOUTFRAGFAILS:
                long_return = ifs6->ifs6_out_fragfail;
                break;
            case IPV6IFSTATSOUTFRAGCREATS:
                long_return = ifs6->ifs6_out_fragcreat;
                break;
            case IPV6IFSTATSOUTREASMREQS:
                long_return = ifs6->ifs6_reass_reqd;
                break;
            case IPV6IFSTATSOUTREASMOKS:
                long_return = ifs6->ifs6_reass_ok;
                break;
            case IPV6IFSTATSOUTREASMFAILS:
                long_return = ifs6->ifs6_reass_fail;
                break;
            case IPV6IFSTATSINMCASTPKTS:
                long_return = ifs6->ifs6_in_mcast;
                break;
            case IPV6IFSTATSOUTMCASTPKTS:
                long_return = ifs6->ifs6_out_mcast;
                break;
            default:
                return NULL;
            }
            return (u_char *) & long_return;
        }
#endif
    default:
        break;
    }
    return NULL;
#endif
}

u_char         *
var_icmpv6Entry(register struct variable * vp,
                oid * name,
                size_t * length,
                int exact, size_t * var_len, WriteMethod ** write_method)
{
#ifndef HAVE_IF_NAMEINDEX
    return NULL;
#else
    int             interface;
    int             max;
    char           *p;

    max = if_maxifindex();
    if (max < 0)
        return NULL;

    if (header_ipv6_scan
        (vp, name, length, exact, var_len, write_method, 1, max)
        == MATCH_FAILED) {
        return NULL;
    }
    interface = name[*length - 1];
    DEBUGMSGTL(("mibII/ipv6", "interface: %d(%s)\n",
                interface, if_getname(interface)));
    if (interface >= max)
        return NULL;

    switch (vp->magic) {
#ifdef SIOCGIFSTAT_ICMP6
    case IPV6IFICMPINMSG:
    case IPV6IFICMPINERRORS:
    case IPV6IFICMPINDSTUNRCHS:
    case IPV6IFICMPINADMPROHS:
    case IPV6IFICMPINTIMEXCDS:
    case IPV6IFICMPINPARMPROBS:
    case IPV6IFICMPINPKTTOOBIGS:
    case IPV6IFICMPINECHOS:
    case IPV6IFICMPINECHOREPS:
    case IPV6IFICMPINRTRSLICITS:
    case IPV6IFICMPINRTRADVS:
    case IPV6IFICMPINNBRSLICITS:
    case IPV6IFICMPINNBRADVS:
    case IPV6IFICMPINREDIRECTS:
    case IPV6IFICMPINGRPMEQERYS:
    case IPV6IFICMPINGRPMERSPS:
    case IPV6IFICMPINGRPMEREDCS:
    case IPV6IFICMPOUTMSG:
    case IPV6IFICMPOUTERRORS:
    case IPV6IFICMPOUTDSTUNRCHS:
    case IPV6IFICMPOUTADMPROHS:
    case IPV6IFICMPOUTTIMEXCDS:
    case IPV6IFICMPOUTPARMPROBS:
    case IPV6IFICMPOUTPKTTOOBIGS:
    case IPV6IFICMPOUTECHOS:
    case IPV6IFICMPOUTECHOREPS:
    case IPV6IFICMPOUTRTRSLICITS:
    case IPV6IFICMPOUTRTRADVS:
    case IPV6IFICMPOUTNBRSLICITS:
    case IPV6IFICMPOUTNBRADVS:
    case IPV6IFICMPOUTREDIRECTS:
    case IPV6IFICMPOUTGRPMEQERYS:
    case IPV6IFICMPOUTGRPMERSPS:
    case IPV6IFICMPOUTGRPMEREDCS:
        {
            struct icmp6_ifstat *ifs6;
            struct in6_ifreq ifr;
            int             s;

            memset(&ifr, 0, sizeof(ifr));
            strncpy(ifr.ifr_name, if_getname(interface),
                    sizeof(ifr.ifr_name));
            if ((s = socket(AF_INET6, SOCK_DGRAM, 0)) < 0)
                break;
            if (ioctl(s, SIOCGIFSTAT_ICMP6, (caddr_t) & ifr) < 0) {
                extern int      errno;
                close(s);
                break;
            }
            close(s);
            ifs6 = &ifr.ifr_ifru.ifru_icmp6stat;
            switch (vp->magic) {
            case IPV6IFICMPINMSG:
                long_return = ifs6->ifs6_in_msg;
                break;
            case IPV6IFICMPINERRORS:
                long_return = ifs6->ifs6_in_error;
                break;
            case IPV6IFICMPINDSTUNRCHS:
                long_return = ifs6->ifs6_in_dstunreach;
                break;
            case IPV6IFICMPINADMPROHS:
                long_return = ifs6->ifs6_in_adminprohib;
                break;
            case IPV6IFICMPINTIMEXCDS:
                long_return = ifs6->ifs6_in_timeexceed;
                break;
            case IPV6IFICMPINPARMPROBS:
                long_return = ifs6->ifs6_in_paramprob;
                break;
            case IPV6IFICMPINPKTTOOBIGS:
                long_return = ifs6->ifs6_in_pkttoobig;
                break;
            case IPV6IFICMPINECHOS:
                long_return = ifs6->ifs6_in_echo;
                break;
            case IPV6IFICMPINECHOREPS:
                long_return = ifs6->ifs6_in_echoreply;
                break;
            case IPV6IFICMPINRTRSLICITS:
                long_return = ifs6->ifs6_in_routersolicit;
                break;
            case IPV6IFICMPINRTRADVS:
                long_return = ifs6->ifs6_in_routeradvert;
                break;
            case IPV6IFICMPINNBRSLICITS:
                long_return = ifs6->ifs6_in_neighborsolicit;
                break;
            case IPV6IFICMPINNBRADVS:
                long_return = ifs6->ifs6_in_neighboradvert;
                break;
            case IPV6IFICMPINREDIRECTS:
                long_return = ifs6->ifs6_in_redirect;
                break;
            case IPV6IFICMPINGRPMEQERYS:
                long_return = ifs6->ifs6_in_mldquery;
                break;
            case IPV6IFICMPINGRPMERSPS:
                long_return = ifs6->ifs6_in_mldreport;
                break;
            case IPV6IFICMPINGRPMEREDCS:
                long_return = ifs6->ifs6_in_mlddone;
                break;
            case IPV6IFICMPOUTMSG:
                long_return = ifs6->ifs6_out_msg;
                break;
            case IPV6IFICMPOUTERRORS:
                long_return = ifs6->ifs6_out_error;
                break;
            case IPV6IFICMPOUTDSTUNRCHS:
                long_return = ifs6->ifs6_out_dstunreach;
                break;
            case IPV6IFICMPOUTADMPROHS:
                long_return = ifs6->ifs6_out_adminprohib;
                break;
            case IPV6IFICMPOUTTIMEXCDS:
                long_return = ifs6->ifs6_out_timeexceed;
                break;
            case IPV6IFICMPOUTPARMPROBS:
                long_return = ifs6->ifs6_out_paramprob;
                break;
            case IPV6IFICMPOUTPKTTOOBIGS:
                long_return = ifs6->ifs6_out_pkttoobig;
                break;
            case IPV6IFICMPOUTECHOS:
                long_return = ifs6->ifs6_out_echo;
                break;
            case IPV6IFICMPOUTECHOREPS:
                long_return = ifs6->ifs6_out_echoreply;
                break;
            case IPV6IFICMPOUTRTRSLICITS:
                long_return = ifs6->ifs6_out_routersolicit;
                break;
            case IPV6IFICMPOUTRTRADVS:
                long_return = ifs6->ifs6_out_routeradvert;
                break;
            case IPV6IFICMPOUTNBRSLICITS:
                long_return = ifs6->ifs6_out_neighborsolicit;
                break;
            case IPV6IFICMPOUTNBRADVS:
                long_return = ifs6->ifs6_out_neighboradvert;
                break;
            case IPV6IFICMPOUTREDIRECTS:
                long_return = ifs6->ifs6_out_redirect;
                break;
            case IPV6IFICMPOUTGRPMEQERYS:
                long_return = ifs6->ifs6_out_mldquery;
                break;
            case IPV6IFICMPOUTGRPMERSPS:
                long_return = ifs6->ifs6_out_mldreport;
                break;
            case IPV6IFICMPOUTGRPMEREDCS:
                long_return = ifs6->ifs6_out_mlddone;
                break;
            default:
                return NULL;
            }
            return (u_char *) & long_return;
        }
#endif
    default:
        break;
    }
    return NULL;
#endif
}

u_char         *
var_udp6(register struct variable * vp,
         oid * name,
         size_t * length,
         int exact, size_t * var_len, WriteMethod ** write_method)
{
    oid             newname[MAX_OID_LEN];
    oid             tmp1[MAX_OID_LEN], tmp2[MAX_OID_LEN];
    int             result;
    int             i, j;
    caddr_t         p;
    static struct in6pcb in6pcb, udb6;
    int             found, hitnext;
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
    char           *sysctl_buf;
    struct xinpgen *xig, *oxig;
#endif                          /* defined(__FreeBSD__) && __FreeBSD__ >= 3 */

    DEBUGMSGTL(("mibII/ipv6", "var_udp6: "));
    DEBUGMSGOID(("mibII/ipv6", name, *length));
    DEBUGMSG(("mibII/ipv6", " %d\n", exact));

#if !(defined(__FreeBSD__) && __FreeBSD__ >= 3)
    if (!auto_nlist("udb6", (char *) &udb6, sizeof(udb6)))
        return NULL;
    p = (caddr_t) udb6.in6p_next;
#else
    {
        const char     *udblist = "net.inet.udp.pcblist";
        int             len;

        if (sysctlbyname(udblist, 0, &len, 0, 0) < 0)
            return NULL;
        if ((sysctl_buf = malloc(len)) == NULL)
            return NULL;
        if (sysctlbyname(udblist, sysctl_buf, &len, 0, 0) < 0) {
            free(sysctl_buf);
            return NULL;
        }
        oxig = (struct xinpgen *) sysctl_buf;
        xig = (struct xinpgen *) ((char *) oxig + oxig->xig_len);
    }
#endif
    found = hitnext = 0;
    memcpy((char *) newname, (char *) vp->name,
           (int) vp->namelen * sizeof(oid));
    DEBUGMSGTL(("mibII/ipv6", "start: p=%x\n", p));
    while (
#if !(defined(__FreeBSD__) && __FreeBSD__ >= 3)
              p && (u_long) p != auto_nlist_value("udb6")
#else
              xig->xig_len > sizeof(struct xinpgen)
#endif
        ) {
        DEBUGMSGTL(("mibII/ipv6", "looping: p=%x\n", p));

#if !(defined(__FreeBSD__) && __FreeBSD__ >= 3)
        klookup((u_long) p, (char *) &in6pcb, sizeof(in6pcb));
#else
        in6pcb = ((struct xinpcb *) xig)->xi_inp;
#endif
        j = (int) vp->namelen;
        for (i = 0; i < sizeof(struct in6_addr); i++)
            newname[j++] = in6pcb.in6p_laddr.s6_addr[i];
        newname[j++] = ntohs(in6pcb.in6p_lport);
        if (IN6_IS_ADDR_LINKLOCAL(&in6pcb.in6p_laddr))
            newname[j++] =
                ntohs(*(u_int16_t *) & in6pcb.in6p_laddr.s6_addr[2]);
        else
            newname[j++] = 0;
        /*XXX*/
            DEBUGMSGTL(("mibII/ipv6", "var_udp6 new: %d %d ",
                        (int) vp->namelen, j));
        DEBUGMSGOID(("mibII/ipv6", newname, j));
        DEBUGMSG(("mibII/ipv6", " %d\n", exact));

        if (exact) {
            result = snmp_oid_compare(name, *length, newname, j);
            if (result == 0) {
                found++;
                break;
            }
        } else {
            /*
             * tcb entries may not be ordered as SNMP scanning code expect.
             * Therefore, we'll do a guess...
             * 1. "name" has the previously matched entry.
             *    If "name" is UDP6 MIBs, take the item that is right next
             *    to the item exactly matches to "name".
             * 2. Otherwise, it is the very first time we entered to UDP6 MIBs.
             */
            memcpy(tmp1, name, vp->namelen * sizeof(oid));
            memcpy(tmp2, newname, vp->namelen * sizeof(oid));
            tmp1[vp->namelen] = 0;
            tmp2[vp->namelen] = 0;
            if (*length == j
                && snmp_oid_compare(tmp1, vp->namelen, tmp2,
                                    vp->namelen) == 0) {
                if (!hitnext) {
                    result = snmp_oid_compare(name, *length, newname, j);
                    if (result == 0)
                        hitnext++;
                } else {
                    found++;
                    break;
                }
            } else {
                if (snmp_oid_compare(name, *length, newname, j) < 0) {
                    found++;
                    break;
                }
            }
        }
#if !(defined(__FreeBSD__) && __FreeBSD__ >= 3)
        p = (caddr_t) in6pcb.in6p_next;
#else
        xig = (struct xinpgen *) ((char *) xig + xig->xig_len);
#endif
    }
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
    free(sysctl_buf);
#endif                          /* defined(__FreeBSD__) && __FreeBSD__ >= 3 */
    if (!found)
        return NULL;
    *length = j;
    memcpy((char *) name, (char *) newname, *length * sizeof(oid));
    *write_method = 0;
    *var_len = sizeof(long);    /* default to 'long' results */


    /*
     * try looking into the kernel variable 
     */
    switch (vp->magic) {
    case IPV6UDPLOCALADDRESS:
        *var_len = sizeof(struct in6_addr);
        return in6pcb.in6p_laddr.s6_addr;
    case IPV6UDPLOCALPORT:
        long_return = ntohs(in6pcb.in6p_lport);
        return (u_char *) & long_return;
    case IPV6UDPIFINDEX:
        if (IN6_IS_ADDR_LINKLOCAL(&in6pcb.in6p_laddr))
            long_return =
                ntohs(*(u_int16_t *) & in6pcb.in6p_laddr.s6_addr[2]);
        else
            long_return = 0;
        /*XXX*/ return (u_char *) & long_return;
    default:
        break;
    }
    ERROR_MSG("");
    return NULL;
}

#ifdef TCP6
u_char         *
var_tcp6(register struct variable * vp,
         oid * name,
         size_t * length,
         int exact, size_t * var_len, WriteMethod ** write_method)
{
    oid             newname[MAX_OID_LEN];
    oid             tmp1[MAX_OID_LEN], tmp2[MAX_OID_LEN];
    int             result;
    int             i, j;
    caddr_t         p;
    static struct in6pcb in6pcb, tcb6;
    struct tcp6cb   tcp6cb;
    int             found, hitnext;
    static int      tcp6statemap[TCP6_NSTATES];
    static int      initialized = 0;
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
    char           *sysctl_buf;
    struct xinpgen *xig, *oxig;
#endif                          /* defined(__FreeBSD__) && __FreeBSD__ >= 3 */

    if (!initialized) {
        tcp6statemap[TCP6S_CLOSED] = 1;
        tcp6statemap[TCP6S_LISTEN] = 2;
        tcp6statemap[TCP6S_SYN_SENT] = 3;
        tcp6statemap[TCP6S_SYN_RECEIVED] = 4;
        tcp6statemap[TCP6S_ESTABLISHED] = 5;
        tcp6statemap[TCP6S_CLOSE_WAIT] = 8;
        tcp6statemap[TCP6S_FIN_WAIT_1] = 6;
        tcp6statemap[TCP6S_CLOSING] = 10;
        tcp6statemap[TCP6S_LAST_ACK] = 9;
        tcp6statemap[TCP6S_FIN_WAIT_2] = 7;
        tcp6statemap[TCP6S_TIME_WAIT] = 11;
        initialized++;
    }

    DEBUGMSGTL(("mibII/ipv6", "var_tcp6: "));
    DEBUGMSGOID(("mibII/ipv6", name, *length));
    DEBUGMSG(("mibII/ipv6", " %d\n", exact));

#if !(defined(__FreeBSD__) && __FreeBSD__ >= 3)
    if (!auto_nlist("tcb6", (char *) &tcb6, sizeof(tcb6)))
        return NULL;
    p = (caddr_t) tcb6.in6p_next;
#else
    {
        const char     *tcblist = "net.inet.tcp.pcblist";
        int             len;

        if (sysctlbyname(tcblist, 0, &len, 0, 0) < 0)
            return NULL;
        if ((sysctl_buf = malloc(len)) == NULL)
            return NULL;
        if (sysctlbyname(tcblist, sysctl_buf, &len, 0, 0) < 0) {
            free(sysctl_buf);
            return NULL;
        }
        oxig = (struct xinpgen *) sysctl_buf;
        xig = (struct xinpgen *) ((char *) oxig + oxig->xig_len);
    }
#endif

    found = hitnext = 0;
    memcpy((char *) newname, (char *) vp->name,
           (int) vp->namelen * sizeof(oid));
    while (
#if !(defined(__FreeBSD__) && __FreeBSD__ >= 3)
              p && (u_long) p != auto_nlist_value("tcb6")
#else
              xig->xig_len > sizeof(struct xinpgen)
#endif
        ) {
        DEBUGMSGTL(("mibII/ipv6", "looping: p=%x\n", p));

#if !(defined(__FreeBSD__) && __FreeBSD__ >= 3)
        if (klookup((u_long) p, (char *) &in6pcb, sizeof(in6pcb)) < 0) {
            DEBUGMSGTL(("mibII/ipv6", "klookup fail for in6pcb at %x\n",
                        p));
            break;
        }
#else
        in6pcb = ((struct xinpcb *) xig)->xi_inp;
#endif
        if (klookup
            ((u_long) in6pcb.in6p_ppcb, (char *) &tcp6cb, sizeof(tcp6cb))
            < 0) {
            DEBUGMSGTL(("mibII/ipv6", "klookup fail for tcp6cb at %x\n",
                        in6pcb.in6p_ppcb));
            break;
        }
        j = (int) vp->namelen;
        for (i = 0; i < sizeof(struct in6_addr); i++)
            newname[j++] = in6pcb.in6p_laddr.s6_addr[i];
        newname[j++] = ntohs(in6pcb.in6p_lport);
        for (i = 0; i < sizeof(struct in6_addr); i++)
            newname[j++] = in6pcb.in6p_faddr.s6_addr[i];
        newname[j++] = ntohs(in6pcb.in6p_fport);
        if (IN6_IS_ADDR_LINKLOCAL(&in6pcb.in6p_laddr))
            newname[j++] =
                ntohs(*(u_int16_t *) & in6pcb.in6p_laddr.s6_addr[2]);
        else
            newname[j++] = 0;
        /*XXX*/ newname[j++] = tcp6statemap[tcp6cb.t_state];

        DEBUGMSGTL(("mibII/ipv6", "var_tcp6 newname: "));
        DEBUGMSGOID(("mibII/ipv6", newname, *length));
        DEBUGMSG(("mibII/ipv6", "\nvar_tcp6 oldname: "));
        DEBUGMSGOID(("mibII/ipv6", name, *length));
        DEBUGMSG(("mibII/ipv6", " %d\n", exact));

#if 1                           /* this is very odd but sometimes happen, and cause infinite loop */
        if (ntohs(in6pcb.in6p_lport) == 0)
            goto skip;
#endif
        if (exact) {
            result = snmp_oid_compare(name, *length, newname, j);
            if (result == 0) {
                found++;
                break;
            }
        } else {
            /*
             * tcb entries may not be ordered as SNMP scanning code expect.
             * Therefore, we'll do a guess...
             * 1. "name" has the previously matched entry.
             *    If "name" is TCP6 MIBs, take the item that is right next
             *    to the item exactly matches to "name".
             * 2. Otherwise, it is the very first time we entered to TCP6 MIBs.
             */
            memcpy(tmp1, name, vp->namelen * sizeof(oid));
            memcpy(tmp2, newname, vp->namelen * sizeof(oid));
            tmp1[vp->namelen] = 0;
            tmp2[vp->namelen] = 0;
            if (*length == j
                && snmp_oid_compare(tmp1, vp->namelen, tmp2,
                                    vp->namelen) == 0) {
                if (!hitnext) {
                    result = snmp_oid_compare(name, *length, newname, j);
                    if (result == 0)
                        hitnext++;
                } else {
                    found++;
                    break;
                }
            } else {
                if (snmp_oid_compare(name, *length, newname, j) < 0) {
                    found++;
                    break;
                }
            }
        }

      skip:
#if !(defined(__FreeBSD__) && __FreeBSD__ >= 3)
        p = (caddr_t) in6pcb.in6p_next;
#else
        xig = (struct xinpgen *) ((char *) xig + xig->xig_len);
#endif
    }
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
    free(sysctl_buf);
#endif
    DEBUGMSGTL(("mibII/ipv6", "found=%d\n", found));
    if (!found)
        return NULL;
    *length = j;
    memcpy((char *) name, (char *) newname, *length * sizeof(oid));
    *write_method = 0;
    *var_len = sizeof(long);    /* default to 'long' results */

    /*
     * try looking into the kernel variable 
     */
    DEBUGMSGTL(("mibII/ipv6", "magic=%d\n", vp->magic));
    switch (vp->magic) {
    case IPV6TCPLOCALADDR:
        *var_len = sizeof(struct in6_addr);
        return (u_char *) & in6pcb.in6p_laddr.s6_addr[0];
    case IPV6TCPLOCALPORT:
        long_return = ntohs(in6pcb.in6p_lport);
        return (u_char *) & long_return;
    case IPV6TCPREMOTEADDR:
        *var_len = sizeof(struct in6_addr);
        return (u_char *) & in6pcb.in6p_faddr.s6_addr[0];
    case IPV6TCPREMOTEPORT:
        long_return = ntohs(in6pcb.in6p_fport);
        return (u_char *) & long_return;
    case IPV6TCPIFINDEX:
        if (IN6_IS_ADDR_LINKLOCAL(&in6pcb.in6p_laddr))
            long_return =
                ntohs(*(u_int16_t *) & in6pcb.in6p_laddr.s6_addr[2]);
        else
            long_return = 0;
        /*XXX*/ return (u_char *) & long_return;
    case IPV6TCPCONNSTATE:
        long_return = tcp6statemap[tcp6cb.t_state];
        return (u_char *) & long_return;
    default:
        break;
    }
    ERROR_MSG("");
    return NULL;
}
#else
u_char         *
var_tcp6(register struct variable * vp,
         oid * name,
         size_t * length,
         int exact, size_t * var_len, WriteMethod ** write_method)
{
    oid             newname[MAX_OID_LEN];
    oid             tmp1[MAX_OID_LEN], tmp2[MAX_OID_LEN];
    int             result;
    int             i, j;
    caddr_t         p;
    static struct in6pcb in6pcb, tcb6;
    struct tcpcb    tcpcb;
    int             found, hitnext;
    static int      tcpstatemap[TCP_NSTATES];
    static int      initialized = 0;
#if defined(__FreeBSD__) && __FreeBSD__ >= 3
    char           *sysctl_buf;
    struct xinpgen *xig, *oxig;
#endif                          /* defined(__FreeBSD__) && __FreeBSD__ >= 3 */

    if (!initialized) {
        tcpstatemap[TCPS_CLOSED] = 1;
        tcpstatemap[TCPS_LISTEN] = 2;
        tcpstatemap[TCPS_SYN_SENT] = 3;
        tcpstatemap[TCPS_SYN_RECEIVED] = 4;
        tcpstatemap[TCPS_ESTABLISHED] = 5;
        tcpstatemap[TCPS_CLOSE_WAIT] = 8;
        tcpstatemap[TCPS_FIN_WAIT_1] = 6;
        tcpstatemap[TCPS_CLOSING] = 10;
        tcpstatemap[TCPS_LAST_ACK] = 9;
        tcpstatemap[TCPS_FIN_WAIT_2] = 7;
        tcpstatemap[TCPS_TIME_WAIT] = 11;
        initialized++;
    }

    DEBUGMSGTL(("mibII/ipv6", "var_tcp6: "));
    DEBUGMSGOID(("mibII/ipv6", name, *length));
    DEBUGMSG(("mibII/ipv6", " %d\n", exact));

#if !(defined(__FreeBSD__) && __FreeBSD__ >= 3)
    if (!auto_nlist("tcb6", (char *) &tcb6, sizeof(tcb6)))
        return NULL;
    p = (caddr_t) tcb6.in6p_next;
#else
    {
        const char     *tcblist = "net.inet.tcp.pcblist";
        int             len;

        if (sysctlbyname(tcblist, 0, &len, 0, 0) < 0)
            return NULL;
        if ((sysctl_buf = malloc(len)) == NULL)
            return NULL;
        if (sysctlbyname(tcblist, sysctl_buf, &len, 0, 0) < 0) {
            free(sysctl_buf);
            return NULL;
        }
        oxig = (struct xinpgen *) sysctl_buf;
        xig = (struct xinpgen *) ((char *) oxig + oxig->xig_len);
    }
#endif

    found = hitnext = 0;
    memcpy((char *) newname, (char *) vp->name,
           (int) vp->namelen * sizeof(oid));
    while (
#if !(defined(__FreeBSD__) && __FreeBSD__ >= 3)
              p && (u_long) p != auto_nlist_value("tcb6")
#else
              xig->xig_len > sizeof(struct xinpgen)
#endif
        ) {
        DEBUGMSGTL(("mibII/ipv6", "looping: p=%x\n", p));

#if !(defined(__FreeBSD__) && __FreeBSD__ >= 3)
        if (klookup((u_long) p, (char *) &in6pcb, sizeof(in6pcb)) < 0)
#else
        in6pcb = ((struct xinpcb *) xig)->xi_inp;
#endif
        {
            DEBUGMSGTL(("mibII/ipv6", "klookup fail for in6pcb at %x\n",
                        p));
            break;
        }
        if (klookup
            ((u_long) in6pcb.in6p_ppcb, (char *) &tcpcb, sizeof(tcpcb))
            < 0) {
            DEBUGMSGTL(("mibII/ipv6", "klookup fail for tcpcb at %x\n",
                        in6pcb.in6p_ppcb));
            break;
        }
        j = (int) vp->namelen;
        for (i = 0; i < sizeof(struct in6_addr); i++)
            newname[j++] = in6pcb.in6p_laddr.s6_addr[i];
        newname[j++] = ntohs(in6pcb.in6p_lport);
        for (i = 0; i < sizeof(struct in6_addr); i++)
            newname[j++] = in6pcb.in6p_faddr.s6_addr[i];
        newname[j++] = ntohs(in6pcb.in6p_fport);
        if (IN6_IS_ADDR_LINKLOCAL(&in6pcb.in6p_laddr))
            newname[j++] =
                ntohs(*(u_int16_t *) & in6pcb.in6p_laddr.s6_addr[2]);
        else
            newname[j++] = 0;
        /*XXX*/ newname[j++] = tcpstatemap[tcpcb.t_state];

        DEBUGMSGTL(("mibII/ipv6", "var_tcp6 newname: "));
        DEBUGMSGOID(("mibII/ipv6", newname, *length));
        DEBUGMSG(("mibII/ipv6", "\nvar_tcp6 oldname: "));
        DEBUGMSGOID(("mibII/ipv6", name, *length));
        DEBUGMSG(("mibII/ipv6", " %d\n", exact));

#if 1                           /* this is very odd but sometimes happen, and cause infinite loop */
        if (ntohs(in6pcb.in6p_lport) == 0)
            goto skip;
#endif
        if (exact) {
            result = snmp_oid_compare(name, *length, newname, j);
            if (result == 0) {
                found++;
                break;
            }
        } else {
            /*
             * tcb entries may not be ordered as SNMP scanning code expect.
             * Therefore, we'll do a guess...
             * 1. "name" has the previously matched entry.
             *    If "name" is TCP6 MIBs, take the item that is right next
             *    to the item exactly matches to "name".
             * 2. Otherwise, it is the very first time we entered to TCP6 MIBs.
             */
            memcpy(tmp1, name, vp->namelen * sizeof(oid));
            memcpy(tmp2, newname, vp->namelen * sizeof(oid));
            tmp1[vp->namelen] = 0;
            tmp2[vp->namelen] = 0;
            if (*length == j
                && snmp_oid_compare(tmp1, vp->namelen, tmp2,
                                    vp->namelen) == 0) {
                if (!hitnext) {
                    result = snmp_oid_compare(name, *length, newname, j);
                    if (result == 0)
                        hitnext++;
                } else {
                    found++;
                    break;
                }
            } else {
                if (snmp_oid_compare(name, *length, newname, j) < 0) {
                    found++;
                    break;
                }
            }
        }

      skip:
#if !(defined(__FreeBSD__) && __FreeBSD__ >= 3)
        p = (caddr_t) in6pcb.in6p_next;
#else
        xig = (struct xinpgen *) ((char *) xig + xig->xig_len);
#endif
    }
    DEBUGMSGTL(("mibII/ipv6", "found=%d\n", found));
    if (!found)
        return NULL;
    *length = j;
    memcpy((char *) name, (char *) newname, *length * sizeof(oid));
    *write_method = 0;
    *var_len = sizeof(long);    /* default to 'long' results */

    /*
     * try looking into the kernel variable 
     */
    DEBUGMSGTL(("mibII/ipv6", "magic=%d\n", vp->magic));
    switch (vp->magic) {
    case IPV6TCPLOCALADDR:
        *var_len = sizeof(struct in6_addr);
        return (u_char *) & in6pcb.in6p_laddr.s6_addr[0];
    case IPV6TCPLOCALPORT:
        long_return = ntohs(in6pcb.in6p_lport);
        return (u_char *) & long_return;
    case IPV6TCPREMOTEADDR:
        *var_len = sizeof(struct in6_addr);
        return (u_char *) & in6pcb.in6p_faddr.s6_addr[0];
    case IPV6TCPREMOTEPORT:
        long_return = ntohs(in6pcb.in6p_fport);
        return (u_char *) & long_return;
    case IPV6TCPIFINDEX:
        if (IN6_IS_ADDR_LINKLOCAL(&in6pcb.in6p_laddr))
            long_return =
                ntohs(*(u_int16_t *) & in6pcb.in6p_laddr.s6_addr[2]);
        else
            long_return = 0;
        /*XXX*/ return (u_char *) & long_return;
    case IPV6TCPCONNSTATE:
        long_return = tcpstatemap[tcpcb.t_state];
        return (u_char *) & long_return;
    default:
        break;
    }
    ERROR_MSG("");
    return NULL;
}

#endif                          /*TCP6 */

#else                           /* !linux / linux */

/*
 * Linux dependent part 
 */
static unsigned long
linux_read_ip6_stat_ulong(const char *file)
{
    FILE           *f;
    unsigned long   value;
    f = fopen(file, "r");
    if (!f)
        return 0;
    if (fscanf(f, "%lu", &value) != 1) {
        fclose(f);
        return 0;
    }
    fclose(f);
    return value;
}

void
linux_read_ip6_stat(struct ip6_mib *ip6stat)
{
    if (!ip6stat)
        return;
    memset(ip6stat, 0, sizeof(*ip6stat));
    ip6stat->Ipv6Forwarding =
        linux_read_ip6_stat_ulong
        ("/proc/sys/net/ipv6/conf/all/forwarding");
    ip6stat->Ipv6DefaultHopLimit =
        linux_read_ip6_stat_ulong
        ("/proc/sys/net/ipv6/conf/default/hop_limit");
}

u_char         *
var_ipv6(register struct variable *vp,
         oid * name,
         size_t * length,
         int exact, size_t * var_len, WriteMethod ** write_method)
{
    int             i;
    static struct ip6_mib ip6stat;

    if (header_ipv6(vp, name, length, exact, var_len, write_method)
        == MATCH_FAILED) {
        return NULL;
    }
    linux_read_ip6_stat(&ip6stat);

    switch (vp->magic) {
    case IPV6DEFAULTHOPLIMIT:
        return (u_char *) & ip6stat.Ipv6DefaultHopLimit;
    case IPV6FORWARDING:
        return (u_char *) & ip6stat.Ipv6Forwarding;
    case IPV6INTERFACES:
#ifdef HAVE_IF_NAMEINDEX
        long_return = if_countifindex();
        if (long_return < 0)
            break;
        return (u_char *) & long_return;
#endif
        break;
    default:
        DEBUGMSGTL(("snmpd", "unknonw sub-id %d in var_ipv6\n",
                    vp->magic));
    }
    return NULL;
}

u_char         *
var_ifv6Entry(register struct variable * vp,
              oid * name,
              size_t * length,
              int exact, size_t * var_len, WriteMethod ** write_method)
{
#ifndef HAVE_IF_NAMEINDEX
    return NULL;
#else
    int             interface;
    int             max;
    char           *p;
    struct ifreq    ifr;
    int             s;

    max = if_maxifindex();
    if (max < 0)
        return NULL;

    if (header_ipv6_scan
        (vp, name, length, exact, var_len, write_method, 1, max)
        == MATCH_FAILED) {
        return NULL;
    }
    interface = name[*length - 1];
    DEBUGMSGTL(("mibII/ipv6", "interface: %d(%s)\n",
                interface, if_getname(interface)));
    if (interface > max)
        return NULL;

    switch (vp->magic) {
    case IPV6IFDESCR:
        p = if_getname(interface);
        if (p) {
            *var_len = strlen(p);
            return p;
        }
        break;
    case IPV6IFLOWLAYER:
        /*
         * should check if type, this is a hard one... 
         */
        *var_len = nullOidLen;
        return (u_char *) nullOid;
    case IPV6IFEFFECTMTU:
        {
            p = if_getname(interface);
            if (!p)
                break;
            memset(&ifr, 0, sizeof(ifr));
            ifr.ifr_addr.sa_family = AF_INET6;
            strncpy(ifr.ifr_name, p, sizeof(ifr.ifr_name));
            if ((s = socket(ifr.ifr_addr.sa_family, SOCK_DGRAM, 0)) < 0)
                break;
            if (ioctl(s, SIOCGIFMTU, (caddr_t) & ifr) < 0) {
                close(s);
                break;
            }
            long_return = ifr.ifr_mtu;
            close(s);
            return (u_char *) & long_return;
        }
    case IPV6IFPHYSADDRESS:
        {
            static struct ifreq buf;
            int             ok = 0;
            p = if_getname(interface);
            if (!p)
                break;
            memset(&ifr, 0, sizeof(ifr));
            ifr.ifr_addr.sa_family = AF_INET6;
            strncpy(ifr.ifr_name, p, sizeof(ifr.ifr_name));
            if ((s = socket(ifr.ifr_addr.sa_family, SOCK_DGRAM, 0)) < 0)
                break;
            if (ioctl(s, SIOCGIFHWADDR, &ifr) < 0) {
                memset(buf.ifr_hwaddr.sa_data, 0,
                       sizeof(buf.ifr_hwaddr.sa_data));
                *var_len = 0;
            } else {
                memcpy(buf.ifr_hwaddr.sa_data, ifr.ifr_hwaddr.sa_data, 6);
                *var_len = (buf.ifr_hwaddr.sa_data[0] |
                            buf.ifr_hwaddr.sa_data[1] |
                            buf.ifr_hwaddr.sa_data[2] |
                            buf.ifr_hwaddr.sa_data[3] |
                            buf.ifr_hwaddr.sa_data[4] |
                            buf.ifr_hwaddr.sa_data[5]) ? 6 : 0;
                ok = 1;
            }
            close(s);
            return (ok ? ((u_char *) & buf.ifr_hwaddr.sa_data) : NULL);
        }
    case IPV6IFADMSTATUS:
    case IPV6IFOPERSTATUS:
        {
            int             flag = 0;
            p = if_getname(interface);
            if (!p)
                break;
            memset(&ifr, 0, sizeof(ifr));
            ifr.ifr_addr.sa_family = AF_INET6;
            strncpy(ifr.ifr_name, p, sizeof(ifr.ifr_name));
            if ((s = socket(ifr.ifr_addr.sa_family, SOCK_DGRAM, 0)) < 0)
                break;
            if (ioctl(s, SIOCGIFFLAGS, &ifr) < 0) {
                close(s);
                break;
            }
            switch (vp->magic) {
            case IPV6IFADMSTATUS:
                flag = IFF_RUNNING;
                break;
            case IPV6IFOPERSTATUS:
                flag = IFF_UP;
                break;
            }
            long_return = (ifr.ifr_flags & flag) ? 1 : 2;
            return (u_char *) & long_return;
        }
    }
    return NULL;
#endif
}

u_char         *
var_icmpv6Entry(register struct variable * vp,
                oid * name,
                size_t * length,
                int exact, size_t * var_len, WriteMethod ** write_method)
{
    return NULL;
}

u_char         *
var_udp6(register struct variable * vp,
         oid * name,
         size_t * length,
         int exact, size_t * var_len, WriteMethod ** write_method)
{
    return NULL;
}

u_char         *
var_tcp6(register struct variable * vp,
         oid * name,
         size_t * length,
         int exact, size_t * var_len, WriteMethod ** write_method)
{
    return NULL;
}

/*
 * misc functions (against broken kernels ) 
 */
void
linux_if_freenameindex(struct if_nameindex *ifndx)
{
    int             i;
    if (!ifndx)
        return;
    for (i = 1; ifndx[i].if_index; i++) {
        free(ifndx[i].if_name);
    }
    free(ifndx);
}

#define linux_freeinternalnameindex(ifni, max)  { \
  int i; 					\
  for (i=1; i<=max; i++){			\
    if (ifni[i].if_name) free(ifni[i].if_name);	\
  }						\
  free(ifni);					\
}

#define LINUX_PROC_NET_IFINET6 "/proc/net/if_inet6"
struct if_nameindex *
linux_if_nameindex(void)
{
    FILE           *f;
    unsigned long   if_index;
    char            if_name[256];
    struct if_nameindex *ifndx = NULL, *iflist = NULL, *new;
    int             i, j;
    int             maxidx, if_count = 0;

    f = fopen(LINUX_PROC_NET_IFINET6, "r");
    if (f) {
        if_count = 0;
        maxidx = -1;
        while (!feof(f)) {
            if (fscanf(f, "%*s %lx %*x %*x %*x %s",
                       &if_index, if_name) != 2)
                continue;
            if (if_index == 0)
                continue;
            if_name[sizeof(if_name) - 1] = '\0';
            if (maxidx < 0 || maxidx < if_index) {

                new =
                    realloc(iflist,
                            (sizeof(struct if_nameindex)) * (if_index +
                                                             2));
                if (!new) {
                    linux_freeinternalnameindex(iflist, if_index);
                    if_count = 0;
                    iflist = NULL;
                    break;
                }
                iflist = new;
                for (i = maxidx + 1; i <= if_index; i++)
                    memset(&iflist[i], 0, sizeof(struct if_nameindex));
                memset(&iflist[if_index + 1], 0,
                       sizeof(struct if_nameindex));
                maxidx = if_index;
            }
            if (iflist[if_index].if_index == 0) {
                if_count++;
                iflist[if_index].if_index = if_index;
                iflist[if_index].if_name = strdup(if_name);
                if (!iflist[if_index].if_name) {
                    linux_freeinternalnameindex(iflist, if_index);
                    if_count = 0;
                    iflist = NULL;
                    break;
                }
            }
        }
        fclose(f);
        if (if_count > 0) {
            ifndx = malloc(sizeof(struct if_nameindex) * (if_count + 1));
            j = 0;
            for (i = 1; i <= maxidx; i++) {
                if (iflist[i].if_index > 0 && *iflist[i].if_name) {
                    memcpy(&ifndx[j++], &iflist[i],
                           sizeof(struct if_nameindex));
                }
            }
            ifndx[j].if_index = 0;
            ifndx[j].if_name = NULL;
        }
        free(iflist);
    }
    return (ifndx);
}

#endif                          /* linux */
