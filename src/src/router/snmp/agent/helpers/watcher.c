#include <net-snmp/net-snmp-config.h>

#include <stdlib.h>
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include <net-snmp/agent/watcher.h>
#include <net-snmp/agent/instance.h>
#include <net-snmp/agent/scalar.h>

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif


/** @defgroup watcher watcher: watch a specified variable and process
 *   it as an instance or scalar object
 *  @ingroup handler
 *  @{
 */
netsnmp_mib_handler *
netsnmp_get_watcher_handler(void)
{
    return netsnmp_create_handler("watcher",
                                  netsnmp_watcher_helper_handler);
}

netsnmp_watcher_info *
netsnmp_create_watcher_info(void *data, size_t size, u_char type, int flags)
{
    netsnmp_watcher_info *winfo = SNMP_MALLOC_TYPEDEF(netsnmp_watcher_info);

    winfo->data      = data;
    winfo->data_size = size;
    winfo->max_size  = size;	/* Probably wrong for non-fixed size data */
    winfo->type      = type;
    if (flags)
        winfo->flags = flags;
    else
        winfo->flags = WATCHER_FIXED_SIZE;

    return winfo;
}

int
netsnmp_register_watched_instance(netsnmp_handler_registration *reginfo,
                                  netsnmp_watcher_info         *watchinfo)
{
    netsnmp_mib_handler *whandler;

    whandler         = netsnmp_get_watcher_handler();
    whandler->myvoid = (void *)watchinfo;

    netsnmp_inject_handler(reginfo, whandler);
    return netsnmp_register_instance(reginfo);
}

int
netsnmp_register_watched_scalar(netsnmp_handler_registration *reginfo,
                                  netsnmp_watcher_info         *watchinfo)
{
    netsnmp_mib_handler *whandler;

    whandler         = netsnmp_get_watcher_handler();
    whandler->myvoid = (void *)watchinfo;

    netsnmp_inject_handler(reginfo, whandler);
    return netsnmp_register_scalar(reginfo);
}



int
netsnmp_watcher_helper_handler(netsnmp_mib_handler *handler,
                               netsnmp_handler_registration *reginfo,
                               netsnmp_agent_request_info *reqinfo,
                               netsnmp_request_info *requests)
{
    netsnmp_watcher_info *winfo = (netsnmp_watcher_info *) handler->myvoid;
    u_char              *old_data;
    int                  cmp;

    DEBUGMSGTL(("helper:watcher", "Got request:  %d\n", reqinfo->mode));
    cmp = snmp_oid_compare(requests->requestvb->name,
                           requests->requestvb->name_length,
                           reginfo->rootoid, reginfo->rootoid_len);

    DEBUGMSGTL(( "helper:watcher", "  oid:", cmp));
    DEBUGMSGOID(("helper:watcher", requests->requestvb->name,
                                   requests->requestvb->name_length));
    DEBUGMSG((   "helper:watcher", "\n"));



    switch (reqinfo->mode) {
        /*
         * data requests 
         */
    case MODE_GET:
        snmp_set_var_typed_value(requests->requestvb,
                                 winfo->type,
                                 winfo->data,
                                 winfo->data_size);
        break;

        /*
         * SET requests.  Should only get here if registered RWRITE 
         */
    case MODE_SET_RESERVE1:
        if (requests->requestvb->type != winfo->type)
            netsnmp_set_request_error(reqinfo, requests,
                                      SNMP_ERR_WRONGTYPE);

        if (((winfo->flags & WATCHER_MAX_SIZE) &&
               requests->requestvb->val_len >  winfo->max_size) ||
            ((winfo->flags & WATCHER_FIXED_SIZE) &&
               requests->requestvb->val_len != winfo->data_size))
             netsnmp_set_request_error(reqinfo, requests,
                                      SNMP_ERR_WRONGLENGTH);
        break;

    case MODE_SET_RESERVE2:
        /*
         * store old info for undo later 
         */
        memdup(&old_data, (u_char *) winfo->data, winfo->data_size);
        if (old_data == NULL) {
            netsnmp_set_request_error(reqinfo, requests,
                                      SNMP_ERR_RESOURCEUNAVAILABLE);
            return SNMP_ERR_NOERROR;
        }
        netsnmp_request_add_list_data(requests,
                                      netsnmp_create_data_list
                                      ("watcher", old_data, free));
        break;

    case MODE_SET_FREE:
        /*
         * nothing to do 
         */
        break;

    case MODE_SET_ACTION:
        /*
         * update current 
         */
        memcpy(winfo->data, (void *)requests->requestvb->val.string,
                                    requests->requestvb->val_len);
        break;

    case MODE_SET_UNDO:
        memcpy(winfo->data,
               netsnmp_request_get_list_data(requests, "watcher"),
               winfo->data_size);
        break;

    case MODE_SET_COMMIT:
        winfo->data_size = requests->requestvb->val_len;
        break;

    }
    if (handler->next && handler->next->access_method)
        return netsnmp_call_next_handler(handler, reginfo, reqinfo,
                                         requests);
    return SNMP_ERR_NOERROR;
}


    /***************************
     *
     * A specialised form of the above, reporting
     *   the sysUpTime indicated by a given timestamp
     *
     ***************************/

netsnmp_mib_handler *
netsnmp_get_watched_timestamp_handler(void)
{
    return netsnmp_create_handler("watcher",
                                  netsnmp_watched_timestamp_handler);
}

int
netsnmp_register_watched_timestamp(netsnmp_handler_registration *reginfo,
                                   marker_t *timestamp)
{
    netsnmp_mib_handler *whandler;

    whandler         = netsnmp_get_watched_timestamp_handler();
    whandler->myvoid = (void *)timestamp;
    netsnmp_inject_handler(reginfo, whandler);
    return netsnmp_register_scalar(reginfo);   /* XXX - or instance? */
}


int
netsnmp_watched_timestamp_handler(netsnmp_mib_handler *handler,
                               netsnmp_handler_registration *reginfo,
                               netsnmp_agent_request_info *reqinfo,
                               netsnmp_request_info *requests)
{
    marker_t timestamp = (marker_t) handler->myvoid;
    int      uptime;
    int      cmp;

    DEBUGMSGTL(("helper:watcher:timestamp",
                               "Got request:  %d\n", reqinfo->mode));
    cmp = snmp_oid_compare(requests->requestvb->name,
                           requests->requestvb->name_length,
                           reginfo->rootoid, reginfo->rootoid_len);

    DEBUGMSGTL(( "helper:watcher:timestamp", "  oid:", cmp));
    DEBUGMSGOID(("helper:watcher:timestamp", requests->requestvb->name,
                                   requests->requestvb->name_length));
    DEBUGMSG((   "helper:watcher:timestamp", "\n"));



    switch (reqinfo->mode) {
        /*
         * data requests 
         */
    case MODE_GET:
        uptime = netsnmp_marker_uptime( timestamp );
        snmp_set_var_typed_value(requests->requestvb,
                                 ASN_TIMETICKS,
                                 (u_char *) &uptime,
                                 sizeof(uptime));
        break;

        /*
         * Timestamps are inherently Read-Only,
         *  so don't need to support SET requests.
         */
    }
    if (handler->next && handler->next->access_method)
        return netsnmp_call_next_handler(handler, reginfo, reqinfo,
                                         requests);
    return SNMP_ERR_NOERROR;
}
