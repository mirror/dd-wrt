
/*
 * usmStats.c: implements the usmStats portion of the SNMP-USER-BASED-SM-MIB 
 */

#include <net-snmp/net-snmp-config.h>

#if HAVE_WINSOCK_H
#include <winsock.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "util_funcs.h"
#include "mibII/sysORTable.h"
#include "usmStats.h"

struct variable2 usmStats_variables[] = {
    {USMSTATSUNSUPPORTEDSECLEVELS, ASN_COUNTER, RONLY, var_usmStats, 1,
     {1}},
    {USMSTATSNOTINTIMEWINDOWS, ASN_COUNTER, RONLY, var_usmStats, 1, {2}},
    {USMSTATSUNKNOWNUSERNAMES, ASN_COUNTER, RONLY, var_usmStats, 1, {3}},
    {USMSTATSUNKNOWNENGINEIDS, ASN_COUNTER, RONLY, var_usmStats, 1, {4}},
    {USMSTATSWRONGDIGESTS, ASN_COUNTER, RONLY, var_usmStats, 1, {5}},
    {USMSTATSDECRYPTIONERRORS, ASN_COUNTER, RONLY, var_usmStats, 1, {6}},
};

/*
 * now load this mib into the agents mib table 
 */
oid             usmStats_variables_oid[] = { 1, 3, 6, 1, 6, 3, 15, 1, 1 };


void
init_usmStats(void)
{
#ifdef USING_MIBII_SYSORTABLE_MODULE
    static oid      reg[] = { 1, 3, 6, 1, 6, 3, 15, 2, 1, 1 };
    register_sysORTable(reg, 10,
                        "The management information definitions for the SNMP User-based Security Model.");
#endif

    REGISTER_MIB("snmpv3/usmStats", usmStats_variables, variable2,
                 usmStats_variables_oid);
}

u_char         *
var_usmStats(struct variable *vp,
             oid * name,
             size_t * length,
             int exact, size_t * var_len, WriteMethod ** write_method)
{

    /*
     * variables we may use later 
     */
    static long     long_ret;
    int             tmagic;

    *write_method = 0;          /* assume it isnt writable for the time being */
    *var_len = sizeof(long_ret);        /* assume an integer and change later if not */

    if (header_generic(vp, name, length, exact, var_len, write_method))
        return 0;

    /*
     * this is where we do the value assignments for the mib results. 
     */
    tmagic = vp->magic;
    if ((tmagic >= 0)
        && (tmagic <= (STAT_USM_STATS_END - STAT_USM_STATS_START))) {
        long_ret = snmp_get_statistic(tmagic + STAT_USM_STATS_START);
        return (unsigned char *) &long_ret;
    }
    return 0;
}
