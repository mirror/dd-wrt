#include <net-snmp/net-snmp-config.h>

#include <stdlib.h>
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include <net-snmp/agent/scalar.h>
#include <net-snmp/agent/instance.h>
#include <net-snmp/agent/serialize.h>
#include <net-snmp/agent/read_only.h>

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

/** @defgroup scalar scalar: process scalars easily.
 *  @ingroup handler
 *  @{
 */
netsnmp_mib_handler *
netsnmp_get_scalar_handler(void)
{
    return netsnmp_create_handler("scalar",
                                  netsnmp_scalar_helper_handler);
}

int
netsnmp_register_scalar(netsnmp_handler_registration *reginfo)
{
    /*
     * Extend the registered OID with space for the instance subid
     * (but don't extend the length just yet!)
     */
    reginfo->rootoid = realloc(reginfo->rootoid,
                              (reginfo->rootoid_len+1) * sizeof(oid) );
    reginfo->rootoid[ reginfo->rootoid_len ] = 0;

    netsnmp_inject_handler(reginfo, netsnmp_get_instance_handler());
    netsnmp_inject_handler(reginfo, netsnmp_get_scalar_handler());
    return netsnmp_register_serialize(reginfo);
}

int
netsnmp_register_read_only_scalar(netsnmp_handler_registration *reginfo)
{
    /*
     * Extend the registered OID with space for the instance subid
     * (but don't extend the length just yet!)
     */
    reginfo->rootoid = realloc(reginfo->rootoid,
                              (reginfo->rootoid_len+1) * sizeof(oid) );
    reginfo->rootoid[ reginfo->rootoid_len ] = 0;

    netsnmp_inject_handler(reginfo, netsnmp_get_instance_handler());
    netsnmp_inject_handler(reginfo, netsnmp_get_scalar_handler());
    netsnmp_inject_handler(reginfo, netsnmp_get_read_only_handler());
    return netsnmp_register_serialize(reginfo);
}



int
netsnmp_scalar_helper_handler(netsnmp_mib_handler *handler,
                                netsnmp_handler_registration *reginfo,
                                netsnmp_agent_request_info *reqinfo,
                                netsnmp_request_info *requests)
{

    netsnmp_variable_list *var = requests->requestvb;

    int             ret, cmp;
    int             namelen;

    DEBUGMSGTL(("helper:scalar", "Got request:\n"));
    namelen = SNMP_MIN(requests->requestvb->name_length,
                       reginfo->rootoid_len);
    cmp = snmp_oid_compare(requests->requestvb->name, namelen,
                           reginfo->rootoid, reginfo->rootoid_len);

    DEBUGMSGTL(("helper:scalar", "  oid:", cmp));
    DEBUGMSGOID(("helper:scalar", var->name, var->name_length));
    DEBUGMSG(("helper:scalar", "\n"));

    switch (reqinfo->mode) {
    case MODE_GET:
        if (cmp != 0) {
            netsnmp_set_request_error(reqinfo, requests,
                                      SNMP_NOSUCHOBJECT);
            return SNMP_ERR_NOERROR;
        } else {
            reginfo->rootoid_len++;
            ret = netsnmp_call_next_handler(handler, reginfo, reqinfo,
                                             requests);
            reginfo->rootoid_len--;
            return ret;
        }
        break;

    case MODE_SET_RESERVE1:
    case MODE_SET_RESERVE2:
    case MODE_SET_ACTION:
    case MODE_SET_COMMIT:
    case MODE_SET_UNDO:
    case MODE_SET_FREE:
        if (cmp != 0) {
            netsnmp_set_request_error(reqinfo, requests,
                                      SNMP_ERR_NOCREATION);
            return SNMP_ERR_NOERROR;
        } else {
            reginfo->rootoid_len++;
            ret = netsnmp_call_next_handler(handler, reginfo, reqinfo,
                                             requests);
            reginfo->rootoid_len--;
            return ret;
        }
        break;

    case MODE_GETNEXT:
        reginfo->rootoid_len++;
        ret = netsnmp_call_next_handler(handler, reginfo, reqinfo, requests);
        reginfo->rootoid_len--;
        return ret;
    }
    /*
     * got here only if illegal mode found 
     */
    return SNMP_ERR_GENERR;
}

/*
 * @} 
 */
