/**  
 *  This file implements the snmpSetSerialNo TestAndIncr counter
 */

/*
 * start be including the appropriate header files 
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "setSerialNo.h"

/*
 * Then, we declare the variables we want to be accessed 
 */
static long     setserialno = 0;        /* default value */

/*
 * our initialization routine, automatically called by the agent 
 */
/*
 * (to get called, the function name must match init_FILENAME() 
 */
void
init_setSerialNo(void)
{
    oid             my_registration_oid[] =
        { 1, 3, 6, 1, 6, 3, 1, 1, 6, 1, 0 };

    /*
     * a debugging statement.  Run the agent with -Dscalar_int to see
     * the output of this debugging statement. 
     */
    DEBUGMSGTL(("snmpSetSerialNo",
                "Initalizing SnmpSetSerialNo to %d\n", setserialno));

    netsnmp_register_long_instance("snmpSetSerialNo",
                                   my_registration_oid,
                                   OID_LENGTH(my_registration_oid),
                                   &setserialno,
                                   netsnmp_setserialno_handler);

    DEBUGMSGTL(("scalar_int", "Done initalizing example scalar int\n"));
}

/*
 * additional support for the given node.  Everything else (value
 * retrival, undo support, etc) is handled by the instance helper for
 * us. 
 */
int
netsnmp_setserialno_handler(netsnmp_mib_handler *handler,
                            netsnmp_handler_registration *reginfo,
                            netsnmp_agent_request_info *reqinfo,
                            netsnmp_request_info *requests)
{
    switch (reqinfo->mode) {
    case MODE_SET_RESERVE1:
        /*
         * the set value must be exactly the same as the current value 
         */
        if (*(requests->requestvb->val.integer) != setserialno)
            netsnmp_set_request_error(reqinfo, requests,
                                      SNMP_ERR_WRONGVALUE);
        break;

    case MODE_SET_ACTION:
        /*
         * we actually increment the value once when it's set. 
         */
        setserialno++;
        break;

    }
    return SNMP_ERR_NOERROR;
}
