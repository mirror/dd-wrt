/*
 *  Template MIB group implementation - sysORTable.c
 *
 */
#include <net-snmp/net-snmp-config.h>
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <sys/types.h>
#if TIME_WITH_SYS_TIME
# ifdef WIN32
#  include <sys/timeb.h>
# else
#  include <sys/time.h>
# endif
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#if HAVE_WINSOCK_H
#include <winsock.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/agent_callbacks.h>

#include "struct.h"
#include "util_funcs.h"
#include "sysORTable.h"
#include "snmpd.h"

#ifdef USING_AGENTX_SUBAGENT_MODULE
#include "agentx/subagent.h"
#include "agentx/client.h"
#endif


static int
_register_sysOR_callback(int majorID, int minorID,
                         void *serverarg, void *clientarg);
static int
_unregister_sysOR_callback(int majorID, int minorID,
                            void *serverarg, void *clientarg);
static int
_unregister_sysOR_by_session_callback(int majorID, int minorID,
                                      void *serverarg, void *clientarg);

struct timeval  sysOR_lastchange;
static struct sysORTable *table = NULL;
static int      numEntries = 0;

/*
 * define the structure we're going to ask the agent to register our
 * information at 
 */
struct variable1 sysORTable_variables[] = {
    {SYSORTABLEID, ASN_OBJECT_ID, RONLY, var_sysORTable, 1, {2}},
    {SYSORTABLEDESCR, ASN_OCTET_STR, RONLY, var_sysORTable, 1, {3}},
    {SYSORTABLEUPTIME, ASN_TIMETICKS, RONLY, var_sysORTable, 1, {4}}
};

/*
 * Define the OID pointer to the top of the mib tree that we're
 * registering underneath 
 */
oid             sysORTable_variables_oid[] = { SNMP_OID_MIB2, 1, 9, 1 };
#ifdef USING_MIBII_SYSTEM_MIB_MODULE
extern oid      system_module_oid[];
extern int      system_module_oid_len;
extern int      system_module_count;
#endif

void
init_sysORTable(void)
{
    /*
     * register ourselves with the agent to handle our mib tree 
     */

#ifdef USING_AGENTX_SUBAGENT_MODULE
    if (netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_ROLE) == MASTER_AGENT)
        (void) register_mib_priority("mibII/sysORTable",
                                     (struct variable *)
                                     sysORTable_variables,
                                     sizeof(struct variable1),
                                     sizeof(sysORTable_variables) /
                                     sizeof(struct variable1),
                                     sysORTable_variables_oid,
                                     sizeof(sysORTable_variables_oid) /
                                     sizeof(oid), 1);
    else
#endif
        REGISTER_MIB("mibII/sysORTable", sysORTable_variables, variable1,
                     sysORTable_variables_oid);

    snmp_register_callback(SNMP_CALLBACK_APPLICATION,
                           SNMPD_CALLBACK_REQ_REG_SYSOR,
                           _register_sysOR_callback, NULL);
    snmp_register_callback(SNMP_CALLBACK_APPLICATION,
                           SNMPD_CALLBACK_REQ_UNREG_SYSOR,
                           _unregister_sysOR_callback, NULL);
    snmp_register_callback(SNMP_CALLBACK_APPLICATION,
                           SNMPD_CALLBACK_REQ_UNREG_SYSOR_SESS,
                           _unregister_sysOR_by_session_callback, NULL);

#ifdef USING_MIBII_SYSTEM_MIB_MODULE
    if (++system_module_count == 3)
        REGISTER_SYSOR_TABLE(system_module_oid, system_module_oid_len,
                             "The MIB module for SNMPv2 entities");
#endif

    gettimeofday(&sysOR_lastchange, NULL);
}

        /*********************
	 *
	 *  System specific implementation functions
	 *
	 *********************/

u_char         *
var_sysORTable(struct variable *vp,
               oid * name,
               size_t * length,
               int exact, size_t * var_len, WriteMethod ** write_method)
{
    unsigned long   i = 0;
    static unsigned long ret;
    struct sysORTable *ptr = table;

    if (header_simple_table
        (vp, name, length, exact, var_len, write_method, numEntries))
        return NULL;

    for (i = 1; ptr != NULL && i < name[*length - 1]; ptr = ptr->next, i++) {
        DEBUGMSGTL(("mibII/sysORTable", "sysORTable -- %lu != %lu\n",
                    i, name[*length - 1]));
    }
    if (ptr == NULL) {
        DEBUGMSGTL(("mibII/sysORTable", "sysORTable -- no match: %lu\n",
                    i));
        return NULL;
    }
    DEBUGMSGTL(("mibII/sysORTable", "sysORTable -- match: %lu\n", i));

    switch (vp->magic) {
    case SYSORTABLEID:
        *var_len = ptr->OR_oidlen * sizeof(ptr->OR_oid[0]);
        return (u_char *) ptr->OR_oid;

    case SYSORTABLEDESCR:
        *var_len = strlen(ptr->OR_descr);
        return (u_char *) ptr->OR_descr;

    case SYSORTABLEUPTIME:
        ret = netsnmp_timeval_uptime(&ptr->OR_uptime);
        return (u_char *) & ret;

    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_sysORTable\n",
                    vp->magic));
    }
    return NULL;
}


int
register_sysORTable_sess(oid * oidin,
                         size_t oidlen,
                         const char *descr, netsnmp_session * ss)
{
    struct sysORTable *ptr, **nptr;
    struct register_sysOR_parameters reg_sysOR_parms;

    DEBUGMSGTL(("mibII/sysORTable", "sysORTable registering: "));
    DEBUGMSGOID(("mibII/sysORTable", oidin, oidlen));
    DEBUGMSG(("mibII/sysORTable", "\n"));

    ptr = (struct sysORTable *) malloc(sizeof(struct sysORTable));
    if (ptr == NULL) {
        return SYS_ORTABLE_REGISTRATION_FAILED;
    }
    ptr->OR_descr = (char *) strdup(descr);
    if (ptr->OR_descr == NULL) {
        free(ptr);
        return SYS_ORTABLE_REGISTRATION_FAILED;
    }
    ptr->OR_oidlen = oidlen;
    ptr->OR_oid = (oid *) malloc(sizeof(oid) * oidlen);
    if (ptr->OR_oid == NULL) {
        free(ptr->OR_descr);
        free(ptr);
        return SYS_ORTABLE_REGISTRATION_FAILED;
    }
    memcpy(ptr->OR_oid, oidin, sizeof(oid) * oidlen);
    gettimeofday(&(ptr->OR_uptime), NULL);
    gettimeofday(&(sysOR_lastchange), NULL);
    ptr->OR_sess = ss;
    ptr->next = NULL;
    numEntries++;

    /* add this entry to the end of the chained list */
    nptr = &table;
    while (*nptr != NULL)
        nptr = &((*nptr)->next);
    *nptr = ptr;

    reg_sysOR_parms.name = oidin;
    reg_sysOR_parms.namelen = oidlen;
    reg_sysOR_parms.descr = descr;
    snmp_call_callbacks(SNMP_CALLBACK_APPLICATION,
                        SNMPD_CALLBACK_REG_SYSOR, &reg_sysOR_parms);

    return SYS_ORTABLE_REGISTERED_OK;
}

int
register_sysORTable(oid * oidin, size_t oidlen, const char *descr)
{
    return register_sysORTable_sess(oidin, oidlen, descr, NULL);
}



int
unregister_sysORTable_sess(oid * oidin,
                           size_t oidlen, netsnmp_session * ss)
{
    struct sysORTable *ptr, *prev = NULL, *next;
    int             found = SYS_ORTABLE_NO_SUCH_REGISTRATION;
    struct register_sysOR_parameters reg_sysOR_parms;

    DEBUGMSGTL(("mibII/sysORTable", "sysORTable unregistering: "));
    DEBUGMSGOID(("mibII/sysORTable", oidin, oidlen));
    DEBUGMSG(("mibII/sysORTable", "\n"));

    for (ptr = table; ptr; ptr = next)
    {
        next = ptr->next;
        if (ptr->OR_sess == ss &&
          (snmp_oid_compare(oidin, oidlen, ptr->OR_oid, ptr->OR_oidlen) == 0))
        {
            if (prev == NULL)
                table = ptr->next;
            else
                prev->next = ptr->next;

            free(ptr->OR_oid);
            free(ptr->OR_descr);
            free(ptr);
            numEntries--;
            gettimeofday(&(sysOR_lastchange), NULL);
            found = SYS_ORTABLE_UNREGISTERED_OK;
            break;
        } else
            prev = ptr;
    }

    reg_sysOR_parms.name = oidin;
    reg_sysOR_parms.namelen = oidlen;
    snmp_call_callbacks(SNMP_CALLBACK_APPLICATION,
                        SNMPD_CALLBACK_UNREG_SYSOR, &reg_sysOR_parms);

    return found;
}


int
unregister_sysORTable(oid * oidin, size_t oidlen)
{
    return unregister_sysORTable_sess(oidin, oidlen, NULL);
}

void
unregister_sysORTable_by_session(netsnmp_session * ss)
{
    struct sysORTable *ptr, *prev = NULL, *next;

    for (ptr = table; ptr; ptr = next)
    {
        next = ptr->next;
        if (((ss->flags & SNMP_FLAGS_SUBSESSION) && ptr->OR_sess == ss) ||
            (!(ss->flags & SNMP_FLAGS_SUBSESSION) && ptr->OR_sess &&
             ptr->OR_sess->subsession == ss)) {
            if (prev == NULL)
                table = next;
            else
                prev->next = next;
            free(ptr->OR_oid);
            free(ptr->OR_descr);
            free(ptr);
            numEntries--;
            gettimeofday(&(sysOR_lastchange), NULL);
        } else
            prev = ptr;
    }
}

static int
_register_sysOR_callback(int majorID, int minorID, void *serverarg,
                         void *clientarg)
{
    struct sysORTable *parms = (struct sysORTable *) serverarg;

    return register_sysORTable_sess(parms->OR_oid, parms->OR_oidlen,
                                    parms->OR_descr, parms->OR_sess);
}

static int
_unregister_sysOR_by_session_callback(int majorID, int minorID,
                                      void *serverarg, void *clientarg)
{
    netsnmp_session *session = (netsnmp_session *) serverarg;

    unregister_sysORTable_by_session(session);

    return 0;
}

static int
_unregister_sysOR_callback(int majorID, int minorID, void *serverarg,
                       void *clientarg)
{
    struct sysORTable *parms = (struct sysORTable *) serverarg;

    return unregister_sysORTable_sess(parms->OR_oid,
                                      parms->OR_oidlen,
                                      parms->OR_sess);
}

