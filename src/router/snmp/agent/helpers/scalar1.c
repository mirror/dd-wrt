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
    netsnmp_inject_handler(reginfo, netsnmp_get_scalar_handler());
    return netsnmp_register_serialize(reginfo);
}

int
netsnmp_register_read_only_scalar(netsnmp_handler_registration *reginfo)
{
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
    oid             subid;
    int             nosuch_err;

    oid             build_space[MAX_OID_LEN];

    DEBUGMSGTL(("helper:scalar", "Got request:\n"));
    namelen = SNMP_MIN(requests->requestvb->name_length,
                       reginfo->rootoid_len);
    cmp = snmp_oid_compare(requests->requestvb->name, namelen,
                           reginfo->rootoid, reginfo->rootoid_len);

    namelen = requests->requestvb->name_length;
    subid   = requests->requestvb->name[namelen-1];

    DEBUGMSGTL(("helper:scalar", "  oid:", cmp));
    DEBUGMSGOID(("helper:scalar", var->name, var->name_length));
    DEBUGMSG(("helper:scalar", "\n"));

    nosuch_err = SNMP_ERR_NOCREATION;
    switch (reqinfo->mode) {
    case MODE_GET:
        nosuch_err = SNMP_NOSUCHINSTANCE;
        /* Fall-through */

    case MODE_SET_RESERVE1:
    case MODE_SET_RESERVE2:
    case MODE_SET_ACTION:
    case MODE_SET_COMMIT:
    case MODE_SET_UNDO:
    case MODE_SET_FREE:
        if (cmp != 0) {
            netsnmp_set_request_error(reqinfo, requests,
                                      SNMP_NOSUCHOBJECT);
            return SNMP_ERR_NOERROR;
        } else {
            if (( namelen != reginfo->rootoid_len+1) ||
                ( subid   != 0 )) {
                netsnmp_set_request_error(reqinfo, requests, nosuch_err);
                return SNMP_ERR_NOERROR;
            } else {
                return netsnmp_call_next_handler(handler, reginfo, reqinfo,
                                             requests);
            }
        }
        break;

    case MODE_GETNEXT:
        if (cmp < 0 ||
           (cmp == 0 && namelen <= reginfo->rootoid_len) ||
           (cmp == 0 && namelen == reginfo->rootoid_len+1 &&
            subid == 0 && requests->inclusive)) {
            reqinfo->mode = MODE_GET;
            memcpy(build_space, reginfo->rootoid,
                   reginfo->rootoid_len * sizeof(oid));
            build_space[reginfo->rootoid_len] = 0;  /* add the instance subid */

            snmp_set_var_objid(requests->requestvb, build_space,
                               reginfo->rootoid_len+1);
            ret =
                netsnmp_call_next_handler(handler, reginfo, reqinfo,
                                          requests);
            reqinfo->mode = MODE_GETNEXT;
            return ret;
        } else {
            return SNMP_ERR_NOERROR;
        }
        break;
    }
    /*
     * got here only if illegal mode found 
     */
    return SNMP_ERR_GENERR;
}

/*
 * @} 
 */
