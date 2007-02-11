/*
 * snmpMPDStats.c: tallies errors for SNMPv3 message processing. 
 */

#include <net-snmp/net-snmp-config.h>

#if HAVE_WINSOCK_H
#include <winsock.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "mibII/sysORTable.h"
#include "snmpMPDStats.h"
#include "util_funcs.h"


struct variable2 snmpMPDStats_variables[] = {
    {SNMPUNKNOWNSECURITYMODELS, ASN_COUNTER, RONLY, var_snmpMPDStats, 1,
     {1}},
    {SNMPINVALIDMSGS, ASN_COUNTER, RONLY, var_snmpMPDStats, 1, {2}},
    {SNMPUNKNOWNPDUHANDLERS, ASN_COUNTER, RONLY, var_snmpMPDStats, 1, {3}},
};

/*
 * now load this mib into the agents mib table 
 */
oid             snmpMPDStats_variables_oid[] =
    { 1, 3, 6, 1, 6, 3, 11, 2, 1 };

void
init_snmpMPDStats(void)
{
#ifdef USING_MIBII_SYSORTABLE_MODULE
    static oid      reg[] = { 1, 3, 6, 1, 6, 3, 11, 3, 1, 1 };
    register_sysORTable(reg, 10,
                        "The MIB for Message Processing and Dispatching.");
#endif

    REGISTER_MIB("snmpv3/snmpMPDStats", snmpMPDStats_variables, variable2,
                 snmpMPDStats_variables_oid);
}

u_char         *
var_snmpMPDStats(struct variable *vp,
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
        && (tmagic <= (STAT_MPD_STATS_END - STAT_MPD_STATS_START))) {
        long_ret = snmp_get_statistic(tmagic + STAT_MPD_STATS_START);
        return (unsigned char *) &long_ret;
    }
    return 0;
}
