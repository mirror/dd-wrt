#include <net-snmp/net-snmp-config.h>

#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include <net-snmp/agent/null.h>

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

int
netsnmp_register_null(oid * loc, size_t loc_len)
{
    netsnmp_handler_registration *reginfo;
    reginfo = SNMP_MALLOC_TYPEDEF(netsnmp_handler_registration);
    reginfo->handlerName = strdup("");
    reginfo->rootoid = loc;
    reginfo->rootoid_len = loc_len;
    reginfo->handler =
        netsnmp_create_handler("null", netsnmp_null_handler);
    return netsnmp_register_handler(reginfo);
}

int
netsnmp_null_handler(netsnmp_mib_handler *handler,
                     netsnmp_handler_registration *reginfo,
                     netsnmp_agent_request_info *reqinfo,
                     netsnmp_request_info *requests)
{
    DEBUGMSGTL(("helper:null", "Got request\n"));

    DEBUGMSGTL(("helper:null", "  oid:"));
    DEBUGMSGOID(("helper:null", requests->requestvb->name,
                 requests->requestvb->name_length));
    DEBUGMSG(("helper:null", "\n"));

    switch (reqinfo->mode) {
    case MODE_GETNEXT:
    case MODE_GETBULK:
        return SNMP_ERR_NOERROR;

    case MODE_GET:
        netsnmp_set_all_requests_error(reqinfo, requests,
                                       SNMP_NOSUCHOBJECT);
        return SNMP_ERR_NOERROR;

    default:
        netsnmp_set_all_requests_error(reqinfo, requests,
                                       SNMP_ERR_NOSUCHNAME);
        return SNMP_ERR_NOERROR;
    }
}
