/*
 *  ICMP MIB group implementation - icmp.c
 *
 */

#include <net-snmp/net-snmp-config.h>
#include "mibII_common.h"

#if HAVE_NETINET_IP_ICMP_H
#include <netinet/ip_icmp.h>
#endif
#if HAVE_NETINET_ICMP_VAR_H
#include <netinet/icmp_var.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/auto_nlist.h>

#include <net-snmp/agent/cache_handler.h>
#include <net-snmp/agent/scalar_group.h>

#include "util_funcs.h"
#include "icmp.h"
#include "sysORTable.h"

#ifndef MIB_STATS_CACHE_TIMEOUT
#define MIB_STATS_CACHE_TIMEOUT	5
#endif
#ifndef ICMP_STATS_CACHE_TIMEOUT
#define ICMP_STATS_CACHE_TIMEOUT	MIB_STATS_CACHE_TIMEOUT
#endif

#if defined(HAVE_LIBPERFSTAT_H) && (defined(aix4) || defined(aix5)) && !defined(FIRST_PROTOCOL)
#include <libperfstat.h>
#ifdef FIRST_PROTOCOL
perfstat_protocol_t ps_proto;
perfstat_id_t ps_name;
#define _USE_PERFSTAT_PROTOCOL 1
#endif
#endif

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



/*
 * Define the OID pointer to the top of the mib tree that we're
 * registering underneath 
 */
oid             icmp_oid[] = { SNMP_OID_MIB2, 5 };
#ifdef USING_MIBII_IP_MODULE
extern oid      ip_module_oid[];
extern int      ip_module_oid_len;
extern int      ip_module_count;
#endif

void
init_icmp(void)
{
    netsnmp_handler_registration *reginfo;

    /*
     * register ourselves with the agent as a group of scalars...
     */
    DEBUGMSGTL(("mibII/icmp", "Initialising ICMP group\n"));
    reginfo = netsnmp_create_handler_registration("icmp", icmp_handler,
		    icmp_oid, OID_LENGTH(icmp_oid), HANDLER_CAN_RONLY);
    netsnmp_register_scalar_group(reginfo, ICMPINMSGS, ICMPOUTADDRMASKREPS);
    /*
     * .... with a local cache
     *    (except for HP-UX 11, which extracts objects individually)
     */
#ifndef hpux11
    netsnmp_inject_handler( reginfo,
		    netsnmp_get_cache_handler(ICMP_STATS_CACHE_TIMEOUT,
			   		icmp_load, icmp_free,
					icmp_oid, OID_LENGTH(icmp_oid)));
#endif

#ifdef USING_MIBII_IP_MODULE
    if (++ip_module_count == 2)
        REGISTER_SYSOR_TABLE(ip_module_oid, ip_module_oid_len,
                             "The MIB module for managing IP and ICMP implementations");
#endif

#if !defined(_USE_PERFSTAT_PROTOCOL)
#ifdef ICMPSTAT_SYMBOL
    auto_nlist(ICMPSTAT_SYMBOL, 0, 0);
#endif
#ifdef solaris2
    init_kernel_sunos5();
#endif
#endif
}


        /*********************
	 *
	 *  System specific data formats
	 *
	 *********************/

#ifdef hpux11
#define ICMP_STAT_STRUCTURE	int
#endif

#ifdef linux
#define ICMP_STAT_STRUCTURE	struct icmp_mib
#define USES_SNMP_DESIGNED_ICMPSTAT
#undef ICMPSTAT_SYMBOL
#endif

#ifdef solaris2
#define ICMP_STAT_STRUCTURE	mib2_icmp_t
#define USES_SNMP_DESIGNED_ICMPSTAT
#endif

#if defined (WIN32) || defined (cygwin)
#include <iphlpapi.h>
#define ICMP_STAT_STRUCTURE MIB_ICMP
#endif

/* ?? #if (defined(NETSNMP_CAN_USE_SYSCTL) && defined(ICMPCTL_STATS)) ?? */

#ifdef HAVE_SYS_ICMPIPSTATS_H
/* or #ifdef		HAVE_SYS_TCPIPSTATS_H  ??? */
#define ICMP_STAT_STRUCTURE	struct kna
#define USES_TRADITIONAL_ICMPSTAT
#endif

#if !defined(ICMP_STAT_STRUCTURE)
#define ICMP_STAT_STRUCTURE	struct icmpstat
#define USES_TRADITIONAL_ICMPSTAT
#endif

ICMP_STAT_STRUCTURE icmpstat;


        /*********************
	 *
	 *  System independent handler
	 *       (mostly!)
	 *
	 *********************/

int
icmp_handler(netsnmp_mib_handler          *handler,
             netsnmp_handler_registration *reginfo,
             netsnmp_agent_request_info   *reqinfo,
             netsnmp_request_info         *requests)
{
    netsnmp_request_info  *request;
    netsnmp_variable_list *requestvb;
    long     ret_value;
    oid      subid;
#ifdef USES_TRADITIONAL_ICMPSTAT
    int      i;
#endif

    /*
     * The cached data should already have been loaded by the
     *    cache handler, higher up the handler chain.
     * But just to be safe, check this and load it manually if necessary
     */
#if defined(_USE_PERFSTAT_PROTOCOL)
    icmp_load(NULL, NULL);
#elif !defined(hpux11)
    if (!netsnmp_cache_is_valid(reqinfo, reginfo->handlerName)) {
        netsnmp_assert("cache" == "valid"); /* always false */
        icmp_load( NULL, NULL );	/* XXX - check for failure */
    }
#endif


    /*
     * 
     *
     */
    DEBUGMSGTL(("mibII/icmp", "Handler - mode %s\n",
                    se_find_label_in_slist("agent_mode", reqinfo->mode)));
    switch (reqinfo->mode) {
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            requestvb = request->requestvb;
            subid = requestvb->name[OID_LENGTH(icmp_oid)];  /* XXX */
            DEBUGMSGTL(( "mibII/icmp", "oid: "));
            DEBUGMSGOID(("mibII/icmp", requestvb->name,
                                       requestvb->name_length));
            DEBUGMSG((   "mibII/icmp", "\n"));

            switch (subid) {
#ifdef USES_SNMP_DESIGNED_ICMPSTAT
    case ICMPINMSGS:
        ret_value = icmpstat.icmpInMsgs;
        break;
    case ICMPINERRORS:
        ret_value = icmpstat.icmpInErrors;
        break;
    case ICMPINDESTUNREACHS:
        ret_value = icmpstat.icmpInDestUnreachs;
        break;
    case ICMPINTIMEEXCDS:
        ret_value = icmpstat.icmpInTimeExcds;
        break;
    case ICMPINPARMPROBS:
        ret_value = icmpstat.icmpInParmProbs;
        break;
    case ICMPINSRCQUENCHS:
        ret_value = icmpstat.icmpInSrcQuenchs;
        break;
    case ICMPINREDIRECTS:
        ret_value = icmpstat.icmpInRedirects;
        break;
    case ICMPINECHOS:
        ret_value = icmpstat.icmpInEchos;
        break;
    case ICMPINECHOREPS:
        ret_value = icmpstat.icmpInEchoReps;
        break;
    case ICMPINTIMESTAMPS:
        ret_value = icmpstat.icmpInTimestamps;
        break;
    case ICMPINTIMESTAMPREPS:
        ret_value = icmpstat.icmpInTimestampReps;
        break;
    case ICMPINADDRMASKS:
        ret_value = icmpstat.icmpInAddrMasks;
        break;
    case ICMPINADDRMASKREPS:
        ret_value = icmpstat.icmpInAddrMaskReps;
        break;
    case ICMPOUTMSGS:
        ret_value = icmpstat.icmpOutMsgs;
        break;
    case ICMPOUTERRORS:
        ret_value = icmpstat.icmpOutErrors;
        break;
    case ICMPOUTDESTUNREACHS:
        ret_value = icmpstat.icmpOutDestUnreachs;
        break;
    case ICMPOUTTIMEEXCDS:
        ret_value = icmpstat.icmpOutTimeExcds;
        break;
    case ICMPOUTPARMPROBS:
        ret_value = icmpstat.icmpOutParmProbs;
        break;
    case ICMPOUTSRCQUENCHS:
        ret_value = icmpstat.icmpOutSrcQuenchs;
        break;
    case ICMPOUTREDIRECTS:
        ret_value = icmpstat.icmpOutRedirects;
        break;
    case ICMPOUTECHOS:
        ret_value = icmpstat.icmpOutEchos;
        break;
    case ICMPOUTECHOREPS:
        ret_value = icmpstat.icmpOutEchoReps;
        break;
    case ICMPOUTTIMESTAMPS:
        ret_value = icmpstat.icmpOutTimestamps;
        break;
    case ICMPOUTTIMESTAMPREPS:
        ret_value = icmpstat.icmpOutTimestampReps;
        break;
    case ICMPOUTADDRMASKS:
        ret_value = icmpstat.icmpOutAddrMasks;
        break;
    case ICMPOUTADDRMASKREPS:
        ret_value = icmpstat.icmpOutAddrMaskReps;
        break;
#elif defined(USES_TRADITIONAL_ICMPSTAT) && !defined(_USE_PERFSTAT_PROTOCOL)
    case ICMPINMSGS:
        ret_value = icmpstat.icps_badcode +
            icmpstat.icps_tooshort +
            icmpstat.icps_checksum + icmpstat.icps_badlen;
        for (i = 0; i <= ICMP_MAXTYPE; i++)
            ret_value += icmpstat.icps_inhist[i];
        break;
    case ICMPINERRORS:
        ret_value = icmpstat.icps_badcode +
            icmpstat.icps_tooshort +
            icmpstat.icps_checksum + icmpstat.icps_badlen;
        break;
    case ICMPINDESTUNREACHS:
        ret_value = icmpstat.icps_inhist[ICMP_UNREACH];
        break;
    case ICMPINTIMEEXCDS:
        ret_value = icmpstat.icps_inhist[ICMP_TIMXCEED];
        break;
    case ICMPINPARMPROBS:
        ret_value = icmpstat.icps_inhist[ICMP_PARAMPROB];
        break;
    case ICMPINSRCQUENCHS:
        ret_value = icmpstat.icps_inhist[ICMP_SOURCEQUENCH];
        break;
    case ICMPINREDIRECTS:
        ret_value = icmpstat.icps_inhist[ICMP_REDIRECT];
        break;
    case ICMPINECHOS:
        ret_value = icmpstat.icps_inhist[ICMP_ECHO];
        break;
    case ICMPINECHOREPS:
        ret_value = icmpstat.icps_inhist[ICMP_ECHOREPLY];
        break;
    case ICMPINTIMESTAMPS:
        ret_value = icmpstat.icps_inhist[ICMP_TSTAMP];
        break;
    case ICMPINTIMESTAMPREPS:
        ret_value = icmpstat.icps_inhist[ICMP_TSTAMPREPLY];
        break;
    case ICMPINADDRMASKS:
        ret_value = icmpstat.icps_inhist[ICMP_MASKREQ];
        break;
    case ICMPINADDRMASKREPS:
        ret_value = icmpstat.icps_inhist[ICMP_MASKREPLY];
        break;
    case ICMPOUTMSGS:
        ret_value = icmpstat.icps_oldshort + icmpstat.icps_oldicmp;
        for (i = 0; i <= ICMP_MAXTYPE; i++)
            ret_value += icmpstat.icps_outhist[i];
        break;
    case ICMPOUTERRORS:
        ret_value = icmpstat.icps_oldshort + icmpstat.icps_oldicmp;
        break;
    case ICMPOUTDESTUNREACHS:
        ret_value = icmpstat.icps_outhist[ICMP_UNREACH];
        break;
    case ICMPOUTTIMEEXCDS:
        ret_value = icmpstat.icps_outhist[ICMP_TIMXCEED];
        break;
    case ICMPOUTPARMPROBS:
        ret_value = icmpstat.icps_outhist[ICMP_PARAMPROB];
        break;
    case ICMPOUTSRCQUENCHS:
        ret_value = icmpstat.icps_outhist[ICMP_SOURCEQUENCH];
        break;
    case ICMPOUTREDIRECTS:
        ret_value = icmpstat.icps_outhist[ICMP_REDIRECT];
        break;
    case ICMPOUTECHOS:
        ret_value = icmpstat.icps_outhist[ICMP_ECHO];
        break;
    case ICMPOUTECHOREPS:
        ret_value = icmpstat.icps_outhist[ICMP_ECHOREPLY];
        break;
    case ICMPOUTTIMESTAMPS:
        ret_value = icmpstat.icps_outhist[ICMP_TSTAMP];
        break;
    case ICMPOUTTIMESTAMPREPS:
        ret_value = icmpstat.icps_outhist[ICMP_TSTAMPREPLY];
        break;
    case ICMPOUTADDRMASKS:
        ret_value = icmpstat.icps_outhist[ICMP_MASKREQ];
        break;
    case ICMPOUTADDRMASKREPS:
        ret_value = icmpstat.icps_outhist[ICMP_MASKREPLY];
        break;
#elif defined(hpux11)
    case ICMPINMSGS:
    case ICMPINERRORS:
    case ICMPINDESTUNREACHS:
    case ICMPINTIMEEXCDS:
    case ICMPINPARMPROBS:
    case ICMPINSRCQUENCHS:
    case ICMPINREDIRECTS:
    case ICMPINECHOS:
    case ICMPINECHOREPS:
    case ICMPINTIMESTAMPS:
    case ICMPINTIMESTAMPREPS:
    case ICMPINADDRMASKS:
    case ICMPINADDRMASKREPS:
    case ICMPOUTMSGS:
    case ICMPOUTERRORS:
    case ICMPOUTDESTUNREACHS:
    case ICMPOUTTIMEEXCDS:
    case ICMPOUTPARMPROBS:
    case ICMPOUTSRCQUENCHS:
    case ICMPOUTREDIRECTS:
    case ICMPOUTECHOS:
    case ICMPOUTECHOREPS:
    case ICMPOUTTIMESTAMPS:
    case ICMPOUTTIMESTAMPREPS:
    case ICMPOUTADDRMASKS:
    case ICMPOUTADDRMASKREPS:
	/*
	 * This is a bit of a hack, to shoehorn the HP-UX 11
	 * single-object retrieval approach into the caching
	 * architecture.
	 */
	if (icmp_load(NULL, (void*)subid) == -1 ) {
            netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHOBJECT);
            continue;
	}
        ret_value = icmpstat;
        break;
#elif defined (WIN32) || defined (cygwin)
    case ICMPINMSGS:
        ret_value = icmpstat.stats.icmpInStats.dwMsgs;
        break;
    case ICMPINERRORS:
        ret_value = icmpstat.stats.icmpInStats.dwErrors;
        break;
    case ICMPINDESTUNREACHS:
        ret_value = icmpstat.stats.icmpInStats.dwDestUnreachs;
        break;
    case ICMPINTIMEEXCDS:
        ret_value = icmpstat.stats.icmpInStats.dwTimeExcds;
        break;
    case ICMPINPARMPROBS:
        ret_value = icmpstat.stats.icmpInStats.dwParmProbs;
        break;
    case ICMPINSRCQUENCHS:
        ret_value = icmpstat.stats.icmpInStats.dwSrcQuenchs;
        break;
    case ICMPINREDIRECTS:
        ret_value = icmpstat.stats.icmpInStats.dwRedirects;
        break;
    case ICMPINECHOS:
        ret_value = icmpstat.stats.icmpInStats.dwEchos;
        break;
    case ICMPINECHOREPS:
        ret_value = icmpstat.stats.icmpInStats.dwEchoReps;
        break;
    case ICMPINTIMESTAMPS:
        ret_value = icmpstat.stats.icmpInStats.dwTimestamps;
        break;
    case ICMPINTIMESTAMPREPS:
        ret_value = icmpstat.stats.icmpInStats.dwTimestampReps;
        break;
    case ICMPINADDRMASKS:
        ret_value = icmpstat.stats.icmpInStats.dwAddrMasks;
        break;
    case ICMPINADDRMASKREPS:
        ret_value = icmpstat.stats.icmpInStats.dwAddrMaskReps;
        break;
    case ICMPOUTMSGS:
        ret_value = icmpstat.stats.icmpOutStats.dwMsgs;
        break;
    case ICMPOUTERRORS:
        ret_value = icmpstat.stats.icmpOutStats.dwErrors;
        break;
    case ICMPOUTDESTUNREACHS:
        ret_value = icmpstat.stats.icmpOutStats.dwDestUnreachs;
        break;
    case ICMPOUTTIMEEXCDS:
        ret_value = icmpstat.stats.icmpOutStats.dwTimeExcds;
        break;
    case ICMPOUTPARMPROBS:
        ret_value = icmpstat.stats.icmpOutStats.dwParmProbs;
        break;
    case ICMPOUTSRCQUENCHS:
        ret_value = icmpstat.stats.icmpOutStats.dwSrcQuenchs;
        break;
    case ICMPOUTREDIRECTS:
        ret_value = icmpstat.stats.icmpOutStats.dwRedirects;
        break;
    case ICMPOUTECHOS:
        ret_value = icmpstat.stats.icmpOutStats.dwEchos;
        break;
    case ICMPOUTECHOREPS:
        ret_value = icmpstat.stats.icmpOutStats.dwEchoReps;
        break;
    case ICMPOUTTIMESTAMPS:
        ret_value = icmpstat.stats.icmpOutStats.dwTimestamps;
        break;
    case ICMPOUTTIMESTAMPREPS:
        ret_value = icmpstat.stats.icmpOutStats.dwTimestampReps;
        break;
    case ICMPOUTADDRMASKS:
        ret_value = icmpstat.stats.icmpOutStats.dwAddrMasks;
        break;
    case ICMPOUTADDRMASKREPS:
        ret_value = icmpstat.stats.icmpOutStats.dwAddrMaskReps;
        break;
#elif defined(_USE_PERFSTAT_PROTOCOL)
    case ICMPINMSGS:
        ret_value = ps_proto.u.icmp.received;
        break;
    case ICMPINERRORS:
        ret_value = ps_proto.u.icmp.errors;
        break;
    case ICMPINDESTUNREACHS:
    case ICMPINTIMEEXCDS:
    case ICMPINPARMPROBS:
    case ICMPINSRCQUENCHS:
    case ICMPINREDIRECTS:
    case ICMPINECHOS:
    case ICMPINECHOREPS:
    case ICMPINTIMESTAMPS:
    case ICMPINTIMESTAMPREPS:
    case ICMPINADDRMASKS:
    case ICMPINADDRMASKREPS:
        ret_value = 0;
        break;
    case ICMPOUTMSGS:
        ret_value = ps_proto.u.icmp.sent;
        break;
    case ICMPOUTERRORS:
        ret_value = ps_proto.u.icmp.errors;
        break;
    case ICMPOUTDESTUNREACHS:
    case ICMPOUTTIMEEXCDS:
    case ICMPOUTPARMPROBS:
    case ICMPOUTSRCQUENCHS:
    case ICMPOUTREDIRECTS:
    case ICMPOUTECHOS:
    case ICMPOUTECHOREPS:
    case ICMPOUTTIMESTAMPS:
    case ICMPOUTTIMESTAMPREPS:
    case ICMPOUTADDRMASKS:
    case ICMPOUTADDRMASKREPS:
        ret_value = 0;
        break;
#endif                          /* USES_SNMP_DESIGNED_ICMPSTAT */
	    }
	    snmp_set_var_typed_value(request->requestvb, ASN_COUNTER,
			             (u_char *)&ret_value, sizeof(ret_value));
	}
        break;

    case MODE_GETNEXT:
    case MODE_GETBULK:
    case MODE_SET_RESERVE1:
    case MODE_SET_RESERVE2:
    case MODE_SET_ACTION:
    case MODE_SET_COMMIT:
    case MODE_SET_FREE:
    case MODE_SET_UNDO:
        snmp_log(LOG_WARNING, "mibII/icmp: Unsupported mode (%d)\n",
                               reqinfo->mode);
        break;
    default:
        snmp_log(LOG_WARNING, "mibII/icmp: Unrecognised mode (%d)\n",
                               reqinfo->mode);
        break;
    }

    return SNMP_ERR_NOERROR;
}


        /*********************
	 *
	 *  Internal implementation functions
	 *
	 *********************/

#ifdef hpux11
int
icmp_load(netsnmp_cache *cache, void *vmagic)
{
    int             fd;
    struct nmparms  p;
    unsigned int    ulen;
    int             ret;
    int             magic = (int) vmagic;

    if ((fd = open_mib("/dev/ip", O_RDONLY, 0, NM_ASYNC_OFF)) < 0) {
        DEBUGMSGTL(("mibII/icmp", "Failed to load ICMP object %d (hpux11)\n", magic));
        return (-1);            /* error */
    }

    switch (magic) {
    case ICMPINMSGS:
        p.objid = ID_icmpInMsgs;
        break;
    case ICMPINERRORS:
        p.objid = ID_icmpInErrors;
        break;
    case ICMPINDESTUNREACHS:
        p.objid = ID_icmpInDestUnreachs;
        break;
    case ICMPINTIMEEXCDS:
        p.objid = ID_icmpInTimeExcds;
        break;
    case ICMPINPARMPROBS:
        p.objid = ID_icmpInParmProbs;
        break;
    case ICMPINSRCQUENCHS:
        p.objid = ID_icmpInSrcQuenchs;
        break;
    case ICMPINREDIRECTS:
        p.objid = ID_icmpInRedirects;
        break;
    case ICMPINECHOS:
        p.objid = ID_icmpInEchos;
        break;
    case ICMPINECHOREPS:
        p.objid = ID_icmpInEchoReps;
        break;
    case ICMPINTIMESTAMPS:
        p.objid = ID_icmpInTimestamps;
        break;
    case ICMPINTIMESTAMPREPS:
        p.objid = ID_icmpInTimestampReps;
        break;
    case ICMPINADDRMASKS:
        p.objid = ID_icmpInAddrMasks;
        break;
    case ICMPINADDRMASKREPS:
        p.objid = ID_icmpInAddrMaskReps;
        break;
    case ICMPOUTMSGS:
        p.objid = ID_icmpOutMsgs;
        break;
    case ICMPOUTERRORS:
        p.objid = ID_icmpOutErrors;
        break;
    case ICMPOUTDESTUNREACHS:
        p.objid = ID_icmpOutDestUnreachs;
        break;
    case ICMPOUTTIMEEXCDS:
        p.objid = ID_icmpOutTimeExcds;
        break;
    case ICMPOUTPARMPROBS:
        p.objid = ID_icmpOutParmProbs;
        break;
    case ICMPOUTSRCQUENCHS:
        p.objid = ID_icmpOutSrcQuenchs;
        break;
    case ICMPOUTREDIRECTS:
        p.objid = ID_icmpOutRedirects;
        break;
    case ICMPOUTECHOS:
        p.objid = ID_icmpOutEchos;
        break;
    case ICMPOUTECHOREPS:
        p.objid = ID_icmpOutEchoReps;
        break;
    case ICMPOUTTIMESTAMPS:
        p.objid = ID_icmpOutTimestamps;
        break;
    case ICMPOUTTIMESTAMPREPS:
        p.objid = ID_icmpOutTimestampReps;
        break;
    case ICMPOUTADDRMASKS:
        p.objid = ID_icmpOutAddrMasks;
        break;
    case ICMPOUTADDRMASKREPS:
        p.objid = ID_icmpOutAddrMaskReps;
        break;
    default:
        icmpstat = 0;
        close_mib(fd);
        return (0);
    }

    p.buffer = (void *)&icmpstat;
    ulen = sizeof(ICMP_STAT_STRUCTURE);
    p.len = &ulen;
    ret = get_mib_info(fd, &p);
    close_mib(fd);

    DEBUGMSGTL(("mibII/icmp", "%s ICMP object %d (hpux11)\n",
               (ret < 0 ? "Failed to load" : "Loaded"),  magic));
    return (ret);               /* 0: ok, < 0: error */
}
#elif defined(linux)
int
icmp_load(netsnmp_cache *cache, void *vmagic)
{
    long            ret_value = -1;

    ret_value = linux_read_icmp_stat(&icmpstat);

    if ( ret_value < 0 ) {
        DEBUGMSGTL(("mibII/icmp", "Failed to load ICMP Group (linux)\n"));
    } else {
        DEBUGMSGTL(("mibII/icmp", "Loaded ICMP Group (linux)\n"));
    }
    return ret_value;
}
#elif defined(solaris2)
int
icmp_load(netsnmp_cache *cache, void *vmagic)
{
    long            ret_value = -1;

    ret_value =
        getMibstat(MIB_ICMP, &icmpstat, sizeof(mib2_icmp_t), GET_FIRST,
                   &Get_everything, NULL);

    if ( ret_value < 0 ) {
        DEBUGMSGTL(("mibII/icmp", "Failed to load ICMP Group (solaris)\n"));
    } else {
        DEBUGMSGTL(("mibII/icmp", "Loaded ICMP Group (solaris)\n"));
    }
    return ret_value;
}
#elif defined (WIN32) || defined (cygwin)
int
icmp_load(netsnmp_cache *cache, void *vmagic)
{
    long            ret_value = -1;

    ret_value = GetIcmpStatistics(&icmpstat);

    if ( ret_value < 0 ) {
        DEBUGMSGTL(("mibII/icmp", "Failed to load ICMP Group (win32)\n"));
    } else {
        DEBUGMSGTL(("mibII/icmp", "Loaded ICMP Group (win32)\n"));
    }
    return ret_value;
}
#elif defined(_USE_PERFSTAT_PROTOCOL)
int
icmp_load(netsnmp_cache *cache, void *vmagic)
{
    long            ret_value = -1;

    strcpy(ps_name.name, "icmp");
    ret_value = perfstat_protocol(&ps_name, &ps_proto, sizeof(ps_proto), 1);

    if ( ret_value < 0 ) {
        DEBUGMSGTL(("mibII/icmp", "Failed to load ICMP Group (AIX)\n"));
    } else {
        ret_value = 0;
        DEBUGMSGTL(("mibII/icmp", "Loaded ICMP Group (AIX)\n"));
    }
    return ret_value;
}
#elif defined(NETSNMP_CAN_USE_SYSCTL) && defined(ICMPCTL_STATS)
int
icmp_load(netsnmp_cache *cache, void *vmagic)
{
    long            ret_value = -1;
    static int      sname[4] =
        { CTL_NET, PF_INET, IPPROTO_ICMP, ICMPCTL_STATS };
    size_t          len = sizeof(icmpstat);

    ret_value = sysctl(sname, 4, &icmpstat, &len, 0, 0);

    if ( ret_value < 0 ) {
        DEBUGMSGTL(("mibII/icmp", "Failed to load ICMP Group (sysctl)\n"));
    } else {
        DEBUGMSGTL(("mibII/icmp", "Loaded ICMP Group (sysctl)\n"));
    }
    return ret_value;
}
#elif defined(HAVE_SYS_TCPIPSTATS_H)
int
icmp_load(netsnmp_cache *cache, void *vmagic)
{
    long            ret_value = -1;

    ret_value =
        sysmp(MP_SAGET, MPSA_TCPIPSTATS, &icmpstat, sizeof icmpstat);

    if ( ret_value < 0 ) {
        DEBUGMSGTL(("mibII/icmp", "Failed to load ICMP Group (tcpipstats)\n"));
    } else {
        DEBUGMSGTL(("mibII/icmp", "Loaded ICMP Group (tcpipstats)\n"));
    }
    return ret_value;
}
#elif defined(ICMPSTAT_SYMBOL)
int
icmp_load(netsnmp_cache *cache, void *vmagic)
{
    long            ret_value = -1;

    if (auto_nlist(ICMPSTAT_SYMBOL, (char *)&icmpstat, sizeof(icmpstat)))
        ret_value = 0;

    if ( ret_value < 0 ) {
        DEBUGMSGTL(("mibII/icmp", "Failed to load ICMP Group (icmpstat)\n"));
    } else {
        DEBUGMSGTL(("mibII/icmp", "Loaded ICMP Group (icmpstat)\n"));
    }
    return ret_value;
}
#else		/* ICMPSTAT_SYMBOL */
int
icmp_load(netsnmp_cache *cache, void *vmagic)
{
    long            ret_value = -1;

    DEBUGMSGTL(("mibII/icmp", "Failed to load ICMP Group (null)\n"));
    return ret_value;
}
#endif		/* hpux11 */

void
icmp_free(netsnmp_cache *cache, void *magic)
{
#if defined(_USE_PERFSTAT_PROTOCOL)
    memset(&ps_proto, 0, sizeof(ps_proto));
#else
    memset(&icmpstat, 0, sizeof(icmpstat));
#endif
}
