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

/** @defgroup watcher watcher
 *  Watch a specified variable and process it as an instance or scalar object
 *  @ingroup leaf
 *  @{
 */
netsnmp_mib_handler *
netsnmp_get_watcher_handler(void)
{
    netsnmp_mib_handler *ret = NULL;
    
    ret = netsnmp_create_handler("watcher",
                                 netsnmp_watcher_helper_handler);
    if (ret) {
        ret->flags |= MIB_HANDLER_AUTO_NEXT;
    }
    return ret;
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

    /* next handler called automatically - 'AUTO_NEXT' */
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
    netsnmp_mib_handler *ret = NULL;
    
    ret = netsnmp_create_handler("watcher",
                                 netsnmp_watched_timestamp_handler);
    if (ret) {
        ret->flags |= MIB_HANDLER_AUTO_NEXT;
    }
    return ret;
}

int
netsnmp_watched_timestamp_register(netsnmp_mib_handler *whandler,
                                   netsnmp_handler_registration *reginfo,
                                   marker_t timestamp)
{
    whandler->myvoid = (void *)timestamp;
    netsnmp_inject_handler(reginfo, whandler);
    return netsnmp_register_scalar(reginfo);   /* XXX - or instance? */
}

int
netsnmp_register_watched_timestamp(netsnmp_handler_registration *reginfo,
                                   marker_t timestamp)
{
    netsnmp_mib_handler *whandler;

    whandler         = netsnmp_get_watched_timestamp_handler();

    return netsnmp_watched_timestamp_register(whandler, reginfo, timestamp);
}


int
netsnmp_watched_timestamp_handler(netsnmp_mib_handler *handler,
                               netsnmp_handler_registration *reginfo,
                               netsnmp_agent_request_info *reqinfo,
                               netsnmp_request_info *requests)
{
    marker_t timestamp = (marker_t) handler->myvoid;
    long     uptime;
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
        if (handler->flags & NETSNMP_WATCHER_DIRECT)
            uptime = * (long*)timestamp;
        else
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
    case MODE_SET_RESERVE1:
        netsnmp_set_request_error(reqinfo, requests,
                                  SNMP_ERR_NOTWRITABLE);
        return SNMP_ERR_NOTWRITABLE;
    }

    /* next handler called automatically - 'AUTO_NEXT' */
    return SNMP_ERR_NOERROR;
}

    /***************************
     *
     * Another specialised form of the above,
     *   implementing a 'TestAndIncr' spinlock
     *
     ***************************/

netsnmp_mib_handler *
netsnmp_get_watched_spinlock_handler(void)
{
    netsnmp_mib_handler *ret = NULL;
    
    ret = netsnmp_create_handler("watcher",
                                 netsnmp_watched_spinlock_handler);
    if (ret) {
        ret->flags |= MIB_HANDLER_AUTO_NEXT;
    }
    return ret;
}

int
netsnmp_register_watched_spinlock(netsnmp_handler_registration *reginfo,
                                   int *spinlock)
{
    netsnmp_mib_handler  *whandler;
    netsnmp_watcher_info *winfo;

    whandler         = netsnmp_get_watched_spinlock_handler();
    whandler->myvoid = (void *)spinlock;
    winfo            = netsnmp_create_watcher_info((void *)spinlock,
		           sizeof(int), ASN_INTEGER, WATCHER_FIXED_SIZE);
    netsnmp_inject_handler(reginfo, whandler);
    return netsnmp_register_watched_scalar(reginfo, winfo);
}


int
netsnmp_watched_spinlock_handler(netsnmp_mib_handler *handler,
                               netsnmp_handler_registration *reginfo,
                               netsnmp_agent_request_info *reqinfo,
                               netsnmp_request_info *requests)
{
    int     *spinlock = (int *) handler->myvoid;
    netsnmp_request_info *request;
    int      cmp;

    DEBUGMSGTL(("helper:watcher:spinlock",
                               "Got request:  %d\n", reqinfo->mode));
    cmp = snmp_oid_compare(requests->requestvb->name,
                           requests->requestvb->name_length,
                           reginfo->rootoid, reginfo->rootoid_len);

    DEBUGMSGTL(( "helper:watcher:spinlock", "  oid:", cmp));
    DEBUGMSGOID(("helper:watcher:spinlock", requests->requestvb->name,
                                   requests->requestvb->name_length));
    DEBUGMSG((   "helper:watcher:spinlock", "\n"));



    switch (reqinfo->mode) {
        /*
         * Ensure the assigned value matches the current one
         */
    case MODE_SET_RESERVE1:
        for (request=requests; request; request=request->next) {
            if (*request->requestvb->val.integer != *spinlock) {
                netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_WRONGVALUE);
                return SNMP_ERR_WRONGVALUE;

            }
        }
        break;

        /*
         * Everything else worked, so increment the spinlock
         */
    case MODE_SET_COMMIT:
	(*spinlock)++;
	break;
    }

    /* next handler called automatically - 'AUTO_NEXT' */
    return SNMP_ERR_NOERROR;
}

    /***************************
     *
     *   Convenience registration routines - modelled on
     *   the equivalent netsnmp_register_*_instance() calls
     *
     ***************************/

int
netsnmp_register_ulong_scalar(const char *name,
                              oid * reg_oid, size_t reg_oid_len,
                              u_long * it,
                              Netsnmp_Node_Handler * subhandler)
{
    return netsnmp_register_watched_scalar(
               netsnmp_create_handler_registration(
                   name, subhandler,
                   reg_oid, reg_oid_len,
                   HANDLER_CAN_RWRITE ),
               netsnmp_create_watcher_info(
                   (void *)it, sizeof( u_long ),
                   ASN_UNSIGNED, WATCHER_FIXED_SIZE ));
}

int
netsnmp_register_read_only_ulong_scalar(const char *name,
                              oid * reg_oid, size_t reg_oid_len,
                              u_long * it,
                              Netsnmp_Node_Handler * subhandler)
{
    return netsnmp_register_watched_scalar(
               netsnmp_create_handler_registration(
                   name, subhandler,
                   reg_oid, reg_oid_len,
                   HANDLER_CAN_RONLY ),
               netsnmp_create_watcher_info(
                   (void *)it, sizeof( u_long ),
                   ASN_UNSIGNED, WATCHER_FIXED_SIZE ));
}

int
netsnmp_register_long_scalar(const char *name,
                              oid * reg_oid, size_t reg_oid_len,
                              long * it,
                              Netsnmp_Node_Handler * subhandler)
{
    return netsnmp_register_watched_scalar(
               netsnmp_create_handler_registration(
                   name, subhandler,
                   reg_oid, reg_oid_len,
                   HANDLER_CAN_RWRITE ),
               netsnmp_create_watcher_info(
                   (void *)it, sizeof( long ),
                   ASN_INTEGER, WATCHER_FIXED_SIZE ));
}

int
netsnmp_register_read_only_long_scalar(const char *name,
                              oid * reg_oid, size_t reg_oid_len,
                              long * it,
                              Netsnmp_Node_Handler * subhandler)
{
    return netsnmp_register_watched_scalar(
               netsnmp_create_handler_registration(
                   name, subhandler,
                   reg_oid, reg_oid_len,
                   HANDLER_CAN_RONLY ),
               netsnmp_create_watcher_info(
                   (void *)it, sizeof( long ),
                   ASN_INTEGER, WATCHER_FIXED_SIZE ));
}


int
netsnmp_register_int_scalar(const char *name,
                              oid * reg_oid, size_t reg_oid_len,
                              int * it,
                              Netsnmp_Node_Handler * subhandler)
{
    return netsnmp_register_watched_scalar(
               netsnmp_create_handler_registration(
                   name, subhandler,
                   reg_oid, reg_oid_len,
                   HANDLER_CAN_RWRITE ),
               netsnmp_create_watcher_info(
                   (void *)it, sizeof( int ),
                   ASN_INTEGER, WATCHER_FIXED_SIZE ));
}

int
netsnmp_register_read_only_int_scalar(const char *name,
                              oid * reg_oid, size_t reg_oid_len,
                              int * it,
                              Netsnmp_Node_Handler * subhandler)
{
    return netsnmp_register_watched_scalar(
               netsnmp_create_handler_registration(
                   name, subhandler,
                   reg_oid, reg_oid_len,
                   HANDLER_CAN_RONLY ),
               netsnmp_create_watcher_info(
                   (void *)it, sizeof( int ),
                   ASN_INTEGER, WATCHER_FIXED_SIZE ));
}


int
netsnmp_register_read_only_counter32_scalar(const char *name,
                              oid * reg_oid, size_t reg_oid_len,
                              u_long * it,
                              Netsnmp_Node_Handler * subhandler)
{
    return netsnmp_register_watched_scalar(
               netsnmp_create_handler_registration(
                   name, subhandler,
                   reg_oid, reg_oid_len,
                   HANDLER_CAN_RONLY ),
               netsnmp_create_watcher_info(
                   (void *)it, sizeof( u_long ),
                   ASN_COUNTER, WATCHER_FIXED_SIZE ));
}
/**  @} */

