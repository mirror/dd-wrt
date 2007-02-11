/*
 *  SNMPv1 MIB group implementation - snmp.c
 *
 */

#include <net-snmp/net-snmp-config.h>
#include <sys/types.h>
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#if HAVE_WINSOCK_H
#include <winsock.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "util_funcs.h"
#include "snmp_mib.h"
#include "sysORTable.h"


        /*********************
	 *
	 *  Kernel & interface information,
	 *   and internal forward declarations
	 *
	 *********************/

extern int      snmp_enableauthentraps;
extern int      snmp_enableauthentrapsset;
int             old_snmp_enableauthentraps;

/*********************
 *
 *  Initialisation & common implementation functions
 *
 *********************/

/*
 * define the structure we're going to ask the agent to register our
 * information at 
 */
struct variable1 snmp_variables[] = {
    {SNMPINPKTS, ASN_COUNTER, RONLY, var_snmp, 1, {1}},
    {SNMPOUTPKTS, ASN_COUNTER, RONLY, var_snmp, 1, {2}},
    {SNMPINBADVERSIONS, ASN_COUNTER, RONLY, var_snmp, 1, {3}},
#if !defined(NETSNMP_DISABLE_SNMPV1) || !defined(NETSNMP_DISABLE_SNMPV2C)
    {SNMPINBADCOMMUNITYNAMES, ASN_COUNTER, RONLY, var_snmp, 1, {4}},
    {SNMPINBADCOMMUNITYUSES, ASN_COUNTER, RONLY, var_snmp, 1, {5}},
#endif /* support for community based SNMP */
    {SNMPINASNPARSEERRORS, ASN_COUNTER, RONLY, var_snmp, 1, {6}},
    {SNMPINTOOBIGS, ASN_COUNTER, RONLY, var_snmp, 1, {8}},
    {SNMPINNOSUCHNAMES, ASN_COUNTER, RONLY, var_snmp, 1, {9}},
    {SNMPINBADVALUES, ASN_COUNTER, RONLY, var_snmp, 1, {10}},
    {SNMPINREADONLYS, ASN_COUNTER, RONLY, var_snmp, 1, {11}},
    {SNMPINGENERRS, ASN_COUNTER, RONLY, var_snmp, 1, {12}},
    {SNMPINTOTALREQVARS, ASN_COUNTER, RONLY, var_snmp, 1, {13}},
    {SNMPINTOTALSETVARS, ASN_COUNTER, RONLY, var_snmp, 1, {14}},
    {SNMPINGETREQUESTS, ASN_COUNTER, RONLY, var_snmp, 1, {15}},
    {SNMPINGETNEXTS, ASN_COUNTER, RONLY, var_snmp, 1, {16}},
    {SNMPINSETREQUESTS, ASN_COUNTER, RONLY, var_snmp, 1, {17}},
    {SNMPINGETRESPONSES, ASN_COUNTER, RONLY, var_snmp, 1, {18}},
    {SNMPINTRAPS, ASN_COUNTER, RONLY, var_snmp, 1, {19}},
    {SNMPOUTTOOBIGS, ASN_COUNTER, RONLY, var_snmp, 1, {20}},
    {SNMPOUTNOSUCHNAMES, ASN_COUNTER, RONLY, var_snmp, 1, {21}},
    {SNMPOUTBADVALUES, ASN_COUNTER, RONLY, var_snmp, 1, {22}},
    {SNMPOUTGENERRS, ASN_COUNTER, RONLY, var_snmp, 1, {24}},
    {SNMPOUTGETREQUESTS, ASN_COUNTER, RONLY, var_snmp, 1, {25}},
    {SNMPOUTGETNEXTS, ASN_COUNTER, RONLY, var_snmp, 1, {26}},
    {SNMPOUTSETREQUESTS, ASN_COUNTER, RONLY, var_snmp, 1, {27}},
    {SNMPOUTGETRESPONSES, ASN_COUNTER, RONLY, var_snmp, 1, {28}},
    {SNMPOUTTRAPS, ASN_COUNTER, RONLY, var_snmp, 1, {29}},
    {SNMPENABLEAUTHENTRAPS, ASN_INTEGER, RWRITE, var_snmp, 1, {30}},
    {SNMPSILENTDROPS, ASN_COUNTER, RONLY, var_snmp, 1, {31}},
    {SNMPPROXYDROPS, ASN_COUNTER, RONLY, var_snmp, 1, {32}}
};

/*
 * Define the OID pointer to the top of the mib tree that we're
 * registering underneath 
 */
oid             snmp_variables_oid[] = { SNMP_OID_MIB2, 11 };
#ifdef USING_MIBII_SYSTEM_MIB_MODULE
extern oid      system_module_oid[];
extern int      system_module_oid_len;
extern int      system_module_count;
#endif

static int
snmp_enableauthentraps_store(int a, int b, void *c, void *d)
{
    char            line[SNMP_MAXBUF_SMALL];

    if (snmp_enableauthentrapsset > 0) {
        snprintf(line, SNMP_MAXBUF_SMALL, "pauthtrapenable %d",
                 snmp_enableauthentraps);
        snmpd_store_config(line);
    }
    return 0;
}

void
init_snmp_mib(void)
{
    /*
     * register ourselves with the agent to handle our mib tree 
     */
    REGISTER_MIB("mibII/snmp", snmp_variables, variable1,
                 snmp_variables_oid);

#ifdef USING_MIBII_SYSTEM_MIB_MODULE
    if (++system_module_count == 3)
        REGISTER_SYSOR_TABLE(system_module_oid, system_module_oid_len,
                             "The MIB module for SNMPv2 entities");
#endif
    snmp_register_callback(SNMP_CALLBACK_LIBRARY, SNMP_CALLBACK_STORE_DATA,
                           snmp_enableauthentraps_store, NULL);
}

/*
 * header_snmp(...
 * Arguments:
 * vp     IN      - pointer to variable entry that points here
 * name    IN/OUT  - IN/name requested, OUT/name found
 * length  IN/OUT  - length of IN/OUT oid's 
 * exact   IN      - TRUE if an exact match was requested
 * var_len OUT     - length of variable or 0 if function returned
 * write_method
 * 
 */

        /*********************
	 *
	 *  System specific implementation functions
	 *	(actually common!)
	 *
	 *********************/


u_char         *
var_snmp(struct variable *vp,
         oid * name,
         size_t * length,
         int exact, size_t * var_len, WriteMethod ** write_method)
{
    static long     long_ret;

    *write_method = 0;          /* assume it isnt writable for the time being */
    *var_len = sizeof(long_ret);        /* assume an integer and change later if not */

    if (header_generic(vp, name, length, exact, var_len, write_method)
        == MATCH_FAILED)
        return NULL;

    /*
     * this is where we do the value assignments for the mib results. 
     */
    if (vp->magic == SNMPENABLEAUTHENTRAPS) {
        *write_method = write_snmp;
        long_return = snmp_enableauthentraps;
        return (u_char *) & long_return;
    } else if ((vp->magic >= 1)
               && (vp->magic <=
                   (STAT_SNMP_STATS_END - STAT_SNMP_STATS_START + 1))) {
        long_ret =
            snmp_get_statistic(vp->magic + STAT_SNMP_STATS_START - 1);
        return (unsigned char *) &long_ret;
    }
    return NULL;
}

/*
 * only for snmpEnableAuthenTraps:
 */

int
write_snmp(int action,
           u_char * var_val,
           u_char var_val_type,
           size_t var_val_len, u_char * statP, oid * name, size_t name_len)
{
    long            intval = 0;

    switch (action) {
    case RESERVE1:             /* Check values for acceptability */
        if (var_val_type != ASN_INTEGER) {
            DEBUGMSGTL(("mibII/snmp_mib", "%x not integer type",
                        var_val_type));
            return SNMP_ERR_WRONGTYPE;
        }

        intval = *((long *) var_val);
        if (intval != 1 && intval != 2) {
            DEBUGMSGTL(("mibII/snmp_mib", "not valid %x\n", intval));
            return SNMP_ERR_WRONGVALUE;
        }
        if (snmp_enableauthentrapsset < 0) {
            /*
             * The object is set in a read-only configuration file.  
             */
            return SNMP_ERR_NOTWRITABLE;
        }
        break;

    case RESERVE2:             /* Allocate memory and similar resources */

        /*
         * Using static variables, so nothing needs to be done 
         */
        break;

    case ACTION:               /* Perform the SET action (if reversible) */

        /*
         * Save the old value, in case of UNDO 
         */
        intval = *((long *) var_val);
        old_snmp_enableauthentraps = snmp_enableauthentraps;
        snmp_enableauthentraps = intval;
        break;

    case UNDO:                 /* Reverse the SET action and free resources */

        snmp_enableauthentraps = old_snmp_enableauthentraps;
        break;

    case COMMIT:
        snmp_enableauthentrapsset = 1;
        snmp_save_persistent(netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_APPTYPE));
        (void) snmp_call_callbacks(SNMP_CALLBACK_LIBRARY,
                                   SNMP_CALLBACK_STORE_DATA, NULL);
        snmp_clean_persistent(netsnmp_ds_get_string
                              (NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_APPTYPE));
        break;

    case FREE:                 /* Free any resources allocated */
        break;
    }
    return SNMP_ERR_NOERROR;
}

/*********************
 *
 *  Internal implementation functions
 *
 *********************/
