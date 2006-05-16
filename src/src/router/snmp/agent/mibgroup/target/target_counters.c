#include <net-snmp/net-snmp-config.h>

#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "target_counters.h"
#include <net-snmp/agent/instance.h>

static oid      unavailable_context_oid[] =
    { 1, 3, 6, 1, 6, 3, 12, 1, 4, 0 };
static oid      unknown_context_oid[] = { 1, 3, 6, 1, 6, 3, 12, 1, 5, 0 };

void
init_target_counters(void)
{
    DEBUGMSGTL(("target_counters", "initializing\n"));

    /*
     * unknown contexts
     */

    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("myInstance",
                                         get_unknown_context_count,
                                         unknown_context_oid,
                                         sizeof(unknown_context_oid) /
                                         sizeof(oid), HANDLER_CAN_RONLY));

    /*
     * unavailable available
     */

    netsnmp_register_read_only_instance(netsnmp_create_handler_registration
                                        ("myInstance",
                                         get_unavailable_context_count,
                                         unavailable_context_oid,
                                         sizeof(unavailable_context_oid) /
                                         sizeof(oid), HANDLER_CAN_RONLY));

}

int
get_unknown_context_count(netsnmp_mib_handler *handler,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info *reqinfo,
                          netsnmp_request_info *requests)
{
    /*
     * we're only called for GETs of the right node, so this is easy: 
     */

    u_long          long_ret =
        snmp_get_statistic(STAT_SNMPUNKNOWNCONTEXTS);
    snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
                             (u_char *) & long_ret, sizeof(long_ret));
    return SNMP_ERR_NOERROR;
}


int
get_unavailable_context_count(netsnmp_mib_handler *handler,
                              netsnmp_handler_registration *reginfo,
                              netsnmp_agent_request_info *reqinfo,
                              netsnmp_request_info *requests)
{
    /*
     * we're only called for GETs of the right node, so this is easy: 
     */

    u_long          long_ret =
        snmp_get_statistic(STAT_SNMPUNAVAILABLECONTEXTS);
    snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER,
                             (u_char *) & long_ret, sizeof(long_ret));
    return SNMP_ERR_NOERROR;
}
