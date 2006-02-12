#include <net-snmp/net-snmp-config.h>

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include <net-snmp/agent/mode_end_call.h>

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

/** @defgroup mode_end_call mode_end_call: at the end of a series of requests, call another handler hook.
 *  Handlers that want to loop through a series of requests and then
 *  receive a callback at the end of a particular MODE can use this
 *  helper to make this possible.  For most modules, this is not
 *  needed as the handler itself could perform a for() loop around the
 *  request list and then perform its actions afterwards.  However, if
 *  something like the serialize helper is in use this isn't possible
 *  because not all the requests for a given handler are being passed
 *  downward in a single group.  Thus, this helper *must* be added
 *  above other helpers like the serialize helper to be useful.
 *
 *  Multiple mode specific handlers can be registered and will be
 *  called in the order they were regestered in.  Callbacks regesterd
 *  with a mode of NETSNMP_MODE_END_ALL_MODES will be called for all
 *  modes.
 * 
 *  @ingroup handler
 *  @{
 */

/** returns a mode_end_call handler that can be injected into a given
 *  handler chain.
 * @param endlist The callback list for the handler to make use of.
 * @return An injectable Net-SNMP handler.
 */
netsnmp_mib_handler *
netsnmp_get_mode_end_call_handler(netsnmp_mode_handler_list *endlist)
{
    netsnmp_mib_handler *me =
        netsnmp_create_handler("mode_end_call",
                               netsnmp_mode_end_call_helper);

    if (!me)
        return NULL;

    me->myvoid = endlist;
    return me;
}

/** adds a mode specific callback to the callback list.
 * @param endinfo the information structure for the mode_end_call helper.  Can be NULL to create a new list.
 * @param mode the mode to be called upon.  A mode of NETSNMP_MODE_END_ALL_MODES = all modes.
 * @param callbackh the netsnmp_mib_handler callback to call.
 * @return the new registration information list upon success.
 */
netsnmp_mode_handler_list *
netsnmp_mode_end_call_add_mode_callback(netsnmp_mode_handler_list *endlist,
                                        int mode,
                                        netsnmp_mib_handler *callbackh) {
    netsnmp_mode_handler_list *ptr, *ptr2;
    ptr = SNMP_MALLOC_TYPEDEF(netsnmp_mode_handler_list);
    if (!ptr)
        return NULL;
    
    ptr->mode = mode;
    ptr->callback_handler = callbackh;
    ptr->next = endlist;

    if (!endlist)
        return ptr;

    /* get to end */
    for(ptr2 = endlist; ptr2->next != NULL; ptr2 = ptr2->next);

    ptr2->next = ptr;
    return endlist;
}
    
/** @internal Implements the mode_end_call handler */
int
netsnmp_mode_end_call_helper(netsnmp_mib_handler *handler,
                            netsnmp_handler_registration *reginfo,
                            netsnmp_agent_request_info *reqinfo,
                            netsnmp_request_info *requests)
{

    int             ret;
    int             ret2 = SNMP_ERR_NOERROR;
    netsnmp_mode_handler_list *ptr;

    /* always call the real handlers first */
    ret = netsnmp_call_next_handler(handler, reginfo, reqinfo,
                                    requests);

    /* then call the callback handlers */
    for(ptr = handler->myvoid; ptr; ptr = ptr->next) {
        if (ptr->mode == NETSNMP_MODE_END_ALL_MODES ||
            reqinfo->mode == ptr->mode) {
            ret2 = netsnmp_call_handler(ptr->callback_handler, reginfo,
                                             reqinfo, requests);
            if (ret != SNMP_ERR_NOERROR)
                ret = ret2;
        }
    }
    
    return ret2;
}
