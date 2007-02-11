/* Portions of this file are subject to the following copyright(s).  See
 * the Net-SNMP's COPYING file for more details and other copyrights
 * that may apply:
 */
/*
 * Portions of this file are copyrighted by:
 * Copyright © 2003 Sun Microsystems, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */
#include <net-snmp/net-snmp-config.h>

#include <sys/types.h>

#if HAVE_STRING_H
#include <string.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include <net-snmp/agent/bulk_to_next.h>


static netsnmp_mib_handler *_clone_handler(netsnmp_mib_handler *it);

/***********************************************************************/
/*
 * New Handler based API 
 */
/***********************************************************************/
/** @defgroup handler Net-SNMP Agent handler and extensibility API
 *  @ingroup agent
 *
 *  The basic theory goes something like this: In the past, with the
 *  original mib module api (which derived from the original CMU SNMP
 *  code) the underlying mib modules were passed very little
 *  information (only the truly most basic information about a
 *  request).  This worked well at the time but in todays world of
 *  subagents, device instrumentation, low resource consumption, etc,
 *  it just isn't flexible enough.  "handlers" are here to fix all that.
 *
 *  With the rewrite of the agent internals for the net-snmp 5.0
 *  release, we introduce a modular calling scheme that allows agent
 *  modules to be written in a very flexible manner, and more
 *  importantly allows reuse of code in a decent way (and without the
 *  memory and speed overheads of OO languages like C++).
 *
 *  Functionally, the notion of what a handler does is the same as the
 *  older api: A handler is @link netsnmp_create_handler() created@endlink and
 *  then @link netsnmp_register_handler() registered@endlink with the main
 *  agent at a given OID in the OID tree and gets called any time a
 *  request is made that it should respond to.  You probably should
 *  use one of the convenience helpers instead of doing anything else
 *  yourself though:
 *
 *  Most importantly, though, is that the handlers are built on the
 *  notion of modularity and reuse.  Specifically, rather than do all
 *  the really hard work (like parsing table indexes out of an
 *  incoming oid request) in each module, the API is designed to make
 *  it easy to write "helper" handlers that merely process some aspect
 *  of the request before passing it along to the final handler that
 *  returns the real answer.  Most people will want to make use of the
 *  @link instance instance@endlink, @link table table@endlink, @link
 *  table_iterator table_iterator@endlink, @link table_data
 *  table_data@endlink, or @link table_dataset table_dataset@endlink
 *  helpers to make their life easier.  These "helpers" interpert
 *  important aspects of the request and pass them on to you.
 *
 *  For instance, the @link table table@endlink helper is designed to
 *  hand you a list of extracted index values from an incoming
 *  request.  THe @link table_iterator table_iterator@endlink helper
 *  is built on top of the table helper, and is designed to help you
 *  iterate through data stored elsewhere (like in a kernel) that is
 *  not in OID lexographical order (ie, don't write your own index/oid
 *  sorting routine, use this helper instead).  The beauty of the
 *  @link table_iterator table_iterator helper@endlink, as well as the @link
 *  instance instance@endlink helper is that they take care of the complex
 *  GETNEXT processing entirely for you and hand you everything you
 *  need to merely return the data as if it was a GET request.  Much
 *  less code and hair pulling.  I've pulled all my hair out to help
 *  you so that only one of us has to be bald.
 *
 * @{
 */

/** creates a netsnmp_mib_handler structure given a name and a access method.
 *  The returned handler should then be @link netsnmp_register_handler()
 *  registered.@endlink
 *
 *  @param name is the handler name and is copied then assigned to
 *              netsnmp_mib_handler->handler_name
 *
 *  @param handler_access_method is a function pointer used as the access
 *	   method for this handler registration instance for whatever required
 *         needs.
 *
 *  @return a pointer to a populated netsnmp_mib_handler struct to be
 *          registered
 *
 *  @see netsnmp_create_handler_registration()
 *  @see netsnmp_register_handler()
 */
netsnmp_mib_handler *
netsnmp_create_handler(const char *name,
                       Netsnmp_Node_Handler * handler_access_method)
{
    netsnmp_mib_handler *ret = SNMP_MALLOC_TYPEDEF(netsnmp_mib_handler);
    if (ret) {
        ret->access_method = handler_access_method;
        if (NULL != name) {
            ret->handler_name = strdup(name);
            if (NULL == ret->handler_name)
                SNMP_FREE(ret);
        }
    }
    return ret;
}

/** creates a handler registration structure given a name, a
 *  access_method function, a registration location oid and the modes
 *  the handler supports. If modes == 0, then modes will automatically
 *  be set to the default value of only HANDLER_CAN_DEFAULT, which is
 *  by default read-only GET and GETNEXT requests. A hander which supports
 *  sets but not row creation should set us a mode of HANDLER_CAN_SET_ONLY.
 *  @note This ends up calling netsnmp_create_handler(name, handler_access_method)
 *  @param name is the handler name and is copied then assigned to
 *              netsnmp_handler_registration->handlerName.
 *
 *  @param handler is a function pointer used as the access
 *	method for this handler registration instance for whatever required
 *	needs.
 *
 *  @param reg_oid is the registration location oid.
 *
 *  @param reg_oid_len is the length of reg_oid, can use the macro,
 *         OID_LENGTH
 *
 *  @param modes is used to configure read/write access.  If modes == 0, 
 *	then modes will automatically be set to the default 
 *	value of only HANDLER_CAN_DEFAULT, which is by default read-only GET 
 *	and GETNEXT requests.  The other two mode options are read only, 
 *	HANDLER_CAN_RONLY, and read/write, HANDLER_CAN_RWRITE.
 *
 *		- HANDLER_CAN_GETANDGETNEXT
 *		- HANDLER_CAN_SET
 *		- HANDLER_CAN_GETBULK      
 *
 *		- HANDLER_CAN_RONLY   (HANDLER_CAN_GETANDGETNEXT)
 *		- HANDLER_CAN_RWRITE  (HANDLER_CAN_GETANDGETNEXT | 
 *			HANDLER_CAN_SET)
 *		- HANDLER_CAN_DEFAULT HANDLER_CAN_RONLY
 *
 *  @return Returns a pointer to a netsnmp_handler_registration struct.
 *          NULL is returned only when memory could not be allocated for the 
 *          netsnmp_handler_registration struct.
 *
 *
 *  @see netsnmp_create_handler()
 *  @see netsnmp_register_handler()
 */
netsnmp_handler_registration *
netsnmp_handler_registration_create(const char *name,
                                    netsnmp_mib_handler *handler,
                                    oid * reg_oid, size_t reg_oid_len,
                                    int modes)
{
    netsnmp_handler_registration *the_reg;
    the_reg = SNMP_MALLOC_TYPEDEF(netsnmp_handler_registration);
    if (!the_reg)
        return NULL;

    if (modes)
        the_reg->modes = modes;
    else
        the_reg->modes = HANDLER_CAN_DEFAULT;

    the_reg->handler = handler;
    the_reg->priority = DEFAULT_MIB_PRIORITY;
    if (name)
        the_reg->handlerName = strdup(name);
    memdup((u_char **) & the_reg->rootoid, (const u_char *) reg_oid,
           reg_oid_len * sizeof(oid));
    the_reg->rootoid_len = reg_oid_len;
    return the_reg;
}

netsnmp_handler_registration *
netsnmp_create_handler_registration(const char *name,
                                    Netsnmp_Node_Handler *
                                    handler_access_method, oid * reg_oid,
                                    size_t reg_oid_len, int modes)
{
    return
        netsnmp_handler_registration_create(name,
                                            netsnmp_create_handler(name, handler_access_method),
                                            reg_oid, reg_oid_len, modes);
}

/** register a handler, as defined by the netsnmp_handler_registration pointer. */
int
netsnmp_register_handler(netsnmp_handler_registration *reginfo)
{
    netsnmp_mib_handler *handler;
    int flags = 0;

    if (reginfo == NULL) {
        snmp_log(LOG_ERR, "netsnmp_register_handler() called illegally\n");
        netsnmp_assert(reginfo != NULL);
        return SNMP_ERR_GENERR;
    }

    DEBUGIF("handler::register") {
        DEBUGMSGTL(("handler::register", "Registering %s (", reginfo->handlerName));
        for (handler = reginfo->handler; handler; handler = handler->next) {
            DEBUGMSG(("handler::register", "::%s", handler->handler_name));
        }

        DEBUGMSG(("handler::register", ") at "));
        if (reginfo->rootoid && reginfo->range_subid) {
            DEBUGMSGOIDRANGE(("handler::register", reginfo->rootoid,
                              reginfo->rootoid_len, reginfo->range_subid,
                              reginfo->range_ubound));
        } else if (reginfo->rootoid) {
            DEBUGMSGOID(("handler::register", reginfo->rootoid,
                         reginfo->rootoid_len));
        } else {
            DEBUGMSG(("handler::register", "[null]"));
        }
        DEBUGMSG(("handler::register", "\n"));
    }

    /*
     * don't let them register for absolutely nothing.  Probably a mistake 
     */
    if (0 == reginfo->modes) {
        reginfo->modes = HANDLER_CAN_DEFAULT;
        snmp_log(LOG_WARNING, "no registration modes specified for %s. "
                 "Defaulting to 0x%x\n", reginfo->handlerName, reginfo->modes);
    }

    /*
     * for handlers that can't GETBULK, force a conversion handler on them 
     */
    if (!(reginfo->modes & HANDLER_CAN_GETBULK)) {
        netsnmp_inject_handler(reginfo,
                               netsnmp_get_bulk_to_next_handler());
    }

    for (handler = reginfo->handler; handler; handler = handler->next) {
        if (handler->flags & MIB_HANDLER_INSTANCE)
            flags = FULLY_QUALIFIED_INSTANCE;
    }

    return netsnmp_register_mib(reginfo->handlerName,
                                NULL, 0, 0,
                                reginfo->rootoid, reginfo->rootoid_len,
                                reginfo->priority,
                                reginfo->range_subid,
                                reginfo->range_ubound, NULL,
                                reginfo->contextName, reginfo->timeout, flags,
                                reginfo, 1);
}

/** unregister a handler, as defined by the netsnmp_handler_registration pointer. */
int
netsnmp_unregister_handler(netsnmp_handler_registration *reginfo)
{
    return unregister_mib_context(reginfo->rootoid, reginfo->rootoid_len,
                                  reginfo->priority,
                                  reginfo->range_subid, reginfo->range_ubound,
                                  reginfo->contextName);
}

/** register a handler, as defined by the netsnmp_handler_registration pointer. */
int
netsnmp_register_handler_nocallback(netsnmp_handler_registration *reginfo)
{
    netsnmp_mib_handler *handler;
    if (reginfo == NULL) {
        snmp_log(LOG_ERR, "netsnmp_register_handler_nocallback() called illegally\n");
        netsnmp_assert(reginfo != NULL);
        return SNMP_ERR_GENERR;
    }
    DEBUGIF("handler::register") {
        DEBUGMSGTL(("handler::register",
                    "Registering (with no callback) "));
        for (handler = reginfo->handler; handler; handler = handler->next) {
            DEBUGMSG(("handler::register", "::%s", handler->handler_name));
        }

        DEBUGMSG(("handler::register", " at "));
        if (reginfo->rootoid && reginfo->range_subid) {
            DEBUGMSGOIDRANGE(("handler::register", reginfo->rootoid,
                              reginfo->rootoid_len, reginfo->range_subid,
                              reginfo->range_ubound));
        } else if (reginfo->rootoid) {
            DEBUGMSGOID(("handler::register", reginfo->rootoid,
                         reginfo->rootoid_len));
        } else {
            DEBUGMSG(("handler::register", "[null]"));
        }
        DEBUGMSG(("handler::register", "\n"));
    }

    /*
     * don't let them register for absolutely nothing.  Probably a mistake 
     */
    if (0 == reginfo->modes) {
        reginfo->modes = HANDLER_CAN_DEFAULT;
    }

    return netsnmp_register_mib(reginfo->handler->handler_name,
                                NULL, 0, 0,
                                reginfo->rootoid, reginfo->rootoid_len,
                                reginfo->priority,
                                reginfo->range_subid,
                                reginfo->range_ubound, NULL,
                                reginfo->contextName, reginfo->timeout, 0,
                                reginfo, 0);
}

/** inject a new handler into the calling chain of the handlers
   definedy by the netsnmp_handler_registration pointer.  The new
   handler is injected after the before_what handler, or if NULL at
   the top of the list and hence will be the new handler to be called
   first.*/
int
netsnmp_inject_handler_before(netsnmp_handler_registration *reginfo,
                              netsnmp_mib_handler *handler,
                              const char *before_what)
{
    netsnmp_mib_handler *handler2 = handler;

    if (handler == NULL || reginfo == NULL) {
        snmp_log(LOG_ERR, "netsnmp_inject_handler() called illegally\n");
        netsnmp_assert(reginfo != NULL);
        netsnmp_assert(handler != NULL);
        return SNMP_ERR_GENERR;
    }
    while (handler2->next) {
        handler2 = handler2->next;  /* Find the end of a handler sub-chain */
    }
    if (reginfo->handler == NULL) {
        DEBUGMSGTL(("handler:inject", "injecting %s\n", handler->handler_name));
    }
    else {
        DEBUGMSGTL(("handler:inject", "injecting %s before %s\n",
                    handler->handler_name, reginfo->handler->handler_name));
    }
    if (before_what) {
        netsnmp_mib_handler *nexth, *prevh = NULL;
        if (reginfo->handler == NULL) {
            snmp_log(LOG_ERR, "no handler to inject before\n");
            return SNMP_ERR_GENERR;
        }
        for(nexth = reginfo->handler; nexth;
            prevh = nexth, nexth = nexth->next) {
            if (strcmp(nexth->handler_name, before_what) == 0)
                break;
        }
        if (!nexth)
            return SNMP_ERR_GENERR;
        if (prevh) {
            /* after prevh and before nexth */
            prevh->next = handler;
            handler2->next = nexth;
            handler->prev = prevh;
            nexth->prev = handler2;
            return SNMPERR_SUCCESS;
        }
        /* else we're first, which is what we do next anyway so fall through */
    }
    handler2->next = reginfo->handler;
    if (reginfo->handler)
        reginfo->handler->prev = handler2;
    reginfo->handler = handler;
    return SNMPERR_SUCCESS;
}

/** inject a new handler into the calling chain of the handlers
   definedy by the netsnmp_handler_registration pointer.  The new handler is
   injected at the top of the list and hence will be the new handler
   to be called first.*/
int
netsnmp_inject_handler(netsnmp_handler_registration *reginfo,
                       netsnmp_mib_handler *handler)
{
    return netsnmp_inject_handler_before(reginfo, handler, NULL);
}

/** calls a handler with with appropriate NULL checking of arguments, etc. */
NETSNMP_INLINE int
netsnmp_call_handler(netsnmp_mib_handler *next_handler,
                     netsnmp_handler_registration *reginfo,
                     netsnmp_agent_request_info *reqinfo,
                     netsnmp_request_info *requests)
{
    Netsnmp_Node_Handler *nh;
    int             ret;

    if (next_handler == NULL || reginfo == NULL || reqinfo == NULL ||
        requests == NULL) {
        snmp_log(LOG_ERR, "netsnmp_call_handler() called illegally\n");
        netsnmp_assert(next_handler != NULL);
        netsnmp_assert(reqinfo != NULL);
        netsnmp_assert(reginfo != NULL);
        netsnmp_assert(requests != NULL);
        return SNMP_ERR_GENERR;
    }

    do {
    nh = next_handler->access_method;
    if (!nh) {
        if (next_handler->next) {
            snmp_log(LOG_ERR, "no access method specified in handler %s.",
                     next_handler->handler_name);
            return SNMP_ERR_GENERR;
        }
        /*
         * The final handler registration in the chain may well not need
         * to include a handler routine, if the processing of this object
         * is handled completely by the agent toolkit helpers.
         */
        return SNMP_ERR_NOERROR;
    }

    DEBUGMSGTL(("handler:calling", "calling handler %s for mode %s\n",
                next_handler->handler_name,
                se_find_label_in_slist("agent_mode", reqinfo->mode)));

    /*
     * XXX: define acceptable return statuses 
     */
    ret = (*nh) (next_handler, reginfo, reqinfo, requests);

    DEBUGMSGTL(("handler:returned", "handler %s returned %d\n",
                next_handler->handler_name, ret));

    if (! (next_handler->flags & MIB_HANDLER_AUTO_NEXT))
        break;

    /*
     * did handler signal that it didn't want auto next this time around?
     */
    if(next_handler->flags & MIB_HANDLER_AUTO_NEXT_OVERRIDE_ONCE) {
        next_handler->flags &= ~MIB_HANDLER_AUTO_NEXT_OVERRIDE_ONCE;
        break;
    }

    next_handler = next_handler->next;

    } while(next_handler);

    return ret;
}

/** @internal
 *  calls all the handlers for a given mode.
 */
int
netsnmp_call_handlers(netsnmp_handler_registration *reginfo,
                      netsnmp_agent_request_info *reqinfo,
                      netsnmp_request_info *requests)
{
    netsnmp_request_info *request;
    int             status;

    if (reginfo == NULL || reqinfo == NULL || requests == NULL) {
        snmp_log(LOG_ERR, "netsnmp_call_handlers() called illegally\n");
        netsnmp_assert(reqinfo != NULL);
        netsnmp_assert(reginfo != NULL);
        netsnmp_assert(requests != NULL);
        return SNMP_ERR_GENERR;
    }

    if (reginfo->handler == NULL) {
        snmp_log(LOG_ERR, "no handler specified.");
        return SNMP_ERR_GENERR;
    }

    switch (reqinfo->mode) {
    case MODE_GETBULK:
    case MODE_GET:
    case MODE_GETNEXT:
        if (!(reginfo->modes & HANDLER_CAN_GETANDGETNEXT))
            return SNMP_ERR_NOERROR;    /* legal */
        break;

    case MODE_SET_RESERVE1:
    case MODE_SET_RESERVE2:
    case MODE_SET_ACTION:
    case MODE_SET_COMMIT:
    case MODE_SET_FREE:
    case MODE_SET_UNDO:
        if (!(reginfo->modes & HANDLER_CAN_SET)) {
            for (; requests; requests = requests->next) {
                netsnmp_set_request_error(reqinfo, requests,
                                          SNMP_ERR_NOTWRITABLE);
            }
            return SNMP_ERR_NOERROR;
        }
        break;

    default:
        snmp_log(LOG_ERR, "unknown mode in netsnmp_call_handlers! bug!\n");
        return SNMP_ERR_GENERR;
    }
    DEBUGMSGTL(("handler:calling", "main handler %s\n",
                reginfo->handler->handler_name));

    for (request = requests ; request; request = request->next) {
        request->processed = 0;
    }

    status = netsnmp_call_handler(reginfo->handler, reginfo, reqinfo, requests);

    return status;
}

/** calls the next handler in the chain after the current one with
   with appropriate NULL checking, etc. */
NETSNMP_INLINE int
netsnmp_call_next_handler(netsnmp_mib_handler *current,
                          netsnmp_handler_registration *reginfo,
                          netsnmp_agent_request_info *reqinfo,
                          netsnmp_request_info *requests)
{

    if (current == NULL || reginfo == NULL || reqinfo == NULL ||
        requests == NULL) {
        snmp_log(LOG_ERR, "netsnmp_call_next_handler() called illegally\n");
        netsnmp_assert(current != NULL);
        netsnmp_assert(reginfo != NULL);
        netsnmp_assert(reqinfo != NULL);
        netsnmp_assert(requests != NULL);
        return SNMP_ERR_GENERR;
    }

    return netsnmp_call_handler(current->next, reginfo, reqinfo, requests);
}

/** calls the next handler in the chain after the current one with
   with appropriate NULL checking, etc. */
NETSNMP_INLINE int
netsnmp_call_next_handler_one_request(netsnmp_mib_handler *current,
                                      netsnmp_handler_registration *reginfo,
                                      netsnmp_agent_request_info *reqinfo,
                                      netsnmp_request_info *requests)
{
    netsnmp_request_info *request;
    int ret;
    
    if (!requests) {
        snmp_log(LOG_ERR, "netsnmp_call_next_handler_ONE_REQUEST() called illegally\n");
        netsnmp_assert(requests != NULL);
        return SNMP_ERR_GENERR;
    }

    request = requests->next;
    requests->next = NULL;
    ret = netsnmp_call_handler(current->next, reginfo, reqinfo, requests);
    requests->next = request;
    return ret;
}

/** free's the resourceses associated with a given handler */
void
netsnmp_handler_free(netsnmp_mib_handler *handler)
{
    if (handler != NULL) {
        if (handler->next != NULL) {
            /** make sure we aren't pointing to ourselves.  */
            netsnmp_assert(handler != handler->next); /* bugs caught: 1 */
            netsnmp_handler_free(handler->next);
            handler->next = NULL;
        }
        /** XXX : segv here at shutdown if SHUTDOWN_AGENT_CLEANLY
         *  defined. About 30 functions down the stack, starting
         *  in clear_context() -> clear_subtree()
         */
        SNMP_FREE(handler->handler_name);
        SNMP_FREE(handler);
    }
}

/** dulpicates a handler and all subsequent handlers
 * see also _clone_handler
 */
netsnmp_mib_handler *
netsnmp_handler_dup(netsnmp_mib_handler *handler)
{
    netsnmp_mib_handler *h = NULL;

    if (handler == NULL) {
        return NULL;
    }

    h = _clone_handler(handler);

    if (h != NULL) {
        h->myvoid = handler->myvoid;

        if (handler->next != NULL) {
            h->next = netsnmp_handler_dup(handler->next);
            if (h->next == NULL) {
                netsnmp_handler_free(h);
                return NULL;
            }
            h->next->prev = h;
        }
        h->prev = NULL;
        return h;
    }
    return NULL;
}

/** free the resources associated with a handler registration object */
void
netsnmp_handler_registration_free(netsnmp_handler_registration *reginfo)
{
    if (reginfo != NULL) {
        netsnmp_handler_free(reginfo->handler);
        SNMP_FREE(reginfo->handlerName);
        SNMP_FREE(reginfo->contextName);
        SNMP_FREE(reginfo->rootoid);
        reginfo->rootoid_len = 0;
        SNMP_FREE(reginfo);
    }
}

/** duplicates the handler registration object */
netsnmp_handler_registration *
netsnmp_handler_registration_dup(netsnmp_handler_registration *reginfo)
{
    netsnmp_handler_registration *r = NULL;

    if (reginfo == NULL) {
        return NULL;
    }


    r = (netsnmp_handler_registration *) calloc(1,
                                                sizeof
                                                (netsnmp_handler_registration));

    if (r != NULL) {
        r->modes = reginfo->modes;
        r->priority = reginfo->priority;
        r->range_subid = reginfo->range_subid;
        r->timeout = reginfo->timeout;
        r->range_ubound = reginfo->range_ubound;
        r->rootoid_len = reginfo->rootoid_len;

        if (reginfo->handlerName != NULL) {
            r->handlerName = strdup(reginfo->handlerName);
            if (r->handlerName == NULL) {
                netsnmp_handler_registration_free(r);
                return NULL;
            }
        }

        if (reginfo->contextName != NULL) {
            r->contextName = strdup(reginfo->contextName);
            if (r->contextName == NULL) {
                netsnmp_handler_registration_free(r);
                return NULL;
            }
        }

        if (reginfo->rootoid != NULL) {
            memdup((u_char **) & (r->rootoid),
                   (const u_char *) reginfo->rootoid,
                   reginfo->rootoid_len * sizeof(oid));
            if (r->rootoid == NULL) {
                netsnmp_handler_registration_free(r);
                return NULL;
            }
        }

        r->handler = netsnmp_handler_dup(reginfo->handler);
        if (r->handler == NULL) {
            netsnmp_handler_registration_free(r);
            return NULL;
        }
        return r;
    }

    return NULL;
}

/** creates a cache of information which can be saved for future
   reference.  Use netsnmp_handler_check_cache() later to make sure it's still
   valid before referencing it in the future. */
NETSNMP_INLINE netsnmp_delegated_cache *
netsnmp_create_delegated_cache(netsnmp_mib_handler *handler,
                               netsnmp_handler_registration *reginfo,
                               netsnmp_agent_request_info *reqinfo,
                               netsnmp_request_info *requests,
                               void *localinfo)
{
    netsnmp_delegated_cache *ret;

    ret = SNMP_MALLOC_TYPEDEF(netsnmp_delegated_cache);
    if (ret) {
        ret->transaction_id = reqinfo->asp->pdu->transid;
        ret->handler = handler;
        ret->reginfo = reginfo;
        ret->reqinfo = reqinfo;
        ret->requests = requests;
        ret->localinfo = localinfo;
    }
    return ret;
}

/** check's a given cache and returns it if it is still valid (ie, the
   agent still considers it to be an outstanding request.  Returns
   NULL if it's no longer valid. */
NETSNMP_INLINE netsnmp_delegated_cache *
netsnmp_handler_check_cache(netsnmp_delegated_cache *dcache)
{
    if (!dcache)
        return dcache;

    if (netsnmp_check_transaction_id(dcache->transaction_id) ==
        SNMPERR_SUCCESS)
        return dcache;

    return NULL;
}

/** frees a cache once you're finished using it */
NETSNMP_INLINE void
netsnmp_free_delegated_cache(netsnmp_delegated_cache *dcache)
{
    /*
     * right now, no extra data is there that needs to be freed 
     */
    if (dcache)
        SNMP_FREE(dcache);

    return;
}


/** marks a list of requests as delegated (or not if isdelegaded = 0) */
void
netsnmp_handler_mark_requests_as_delegated(netsnmp_request_info *requests,
                                           int isdelegated)
{
    while (requests) {
        requests->delegated = isdelegated;
        requests = requests->next;
    }
}

/** add data to a request that can be extracted later by submodules
 *
 * @param request the netsnmp request info structure
 *
 * @param node this is the data to be added to the linked list
 *             request->parent_data
 *
 * @return void
 *
 */
NETSNMP_INLINE void
netsnmp_request_add_list_data(netsnmp_request_info *request,
                              netsnmp_data_list *node)
{
    if (request) {
        if (request->parent_data)
            netsnmp_add_list_data(&request->parent_data, node);
        else
            request->parent_data = node;
    }
}

/** remove data from a request
 *
 * @param request the netsnmp request info structure
 *
 * @param name this is the name of the previously added data
 *
 * @return 0 on successful find-and-delete, 1 otherwise.
 *
 */
NETSNMP_INLINE int
netsnmp_request_remove_list_data(netsnmp_request_info *request,
                                 const char *name)
{
    if ((NULL == request) || (NULL ==request->parent_data))
        return 1;

    return netsnmp_remove_list_node(&request->parent_data, name);
}

/** extract data from a request that was added previously by a parent module
 *
 * @param request the netsnmp request info function
 *
 * @param name used to compare against the request->parent_data->name value,
 *             if a match is found request->parent_data->data is returned
 *
 * @return a void pointer(request->parent_data->data), otherwise NULL is
 *         returned if request is NULL or request->parent_data is NULL or
 *         request->parent_data object is not found.
 */
NETSNMP_INLINE void    *
netsnmp_request_get_list_data(netsnmp_request_info *request,
                              const char *name)
{
    if (request)
        return netsnmp_get_list_data(request->parent_data, name);
    return NULL;
}

/** Free the extra data stored in a request */
NETSNMP_INLINE void
netsnmp_free_request_data_set(netsnmp_request_info *request)
{
    if (request)
        netsnmp_free_list_data(request->parent_data);
}

/** Free the extra data stored in a bunch of requests (all data in the chain) */
NETSNMP_INLINE void
netsnmp_free_request_data_sets(netsnmp_request_info *request)
{
    if (request && request->parent_data) {
        netsnmp_free_all_list_data(request->parent_data);
        request->parent_data = NULL;
    }
}

/** Returns a handler from a chain based on the name */
netsnmp_mib_handler *
netsnmp_find_handler_by_name(netsnmp_handler_registration *reginfo,
                             const char *name)
{
    netsnmp_mib_handler *it;
    for (it = reginfo->handler; it; it = it->next) {
        if (strcmp(it->handler_name, name) == 0) {
            return it;
        }
    }
    return NULL;
}

/** Returns a handler's void * pointer from a chain based on the name.
 This probably shouldn't be used by the general public as the void *
 data may change as a handler evolves.  Handlers should really
 advertise some function for you to use instead. */
void           *
netsnmp_find_handler_data_by_name(netsnmp_handler_registration *reginfo,
                                  const char *name)
{
    netsnmp_mib_handler *it = netsnmp_find_handler_by_name(reginfo, name);
    if (it)
        return it->myvoid;
    return NULL;
}

/** clones a mib handler (name, flags and access methods only; not myvoid)
 * see also netsnmp_handler_dup
 */
static netsnmp_mib_handler *
_clone_handler(netsnmp_mib_handler *it)
{
    netsnmp_mib_handler *dup;

    if(NULL == it)
        return NULL;

    dup = netsnmp_create_handler(it->handler_name, it->access_method);
    if(NULL != dup)
        dup->flags = it->flags;

    return dup;
}

static netsnmp_data_list *handler_reg = NULL;

void
handler_free_callback(void *free)
{
    netsnmp_handler_free((netsnmp_mib_handler *)free);
}

/** registers a given handler by name so that it can be found easily later.
 */
void
netsnmp_register_handler_by_name(const char *name,
                                 netsnmp_mib_handler *handler)
{
    netsnmp_add_list_data(&handler_reg,
                          netsnmp_create_data_list(name, (void *) handler,
                                                   handler_free_callback));
    DEBUGMSGTL(("handler_registry", "registering helper %s\n", name));
}

/** clears the entire handler-registration list
 */
void
netsnmp_clear_handler_list(void)
{
    DEBUGMSGTL(("agent_handler", "netsnmp_clear_handler_list() called\n"));
    netsnmp_free_all_list_data(handler_reg);
    handler_reg = NULL;
}

/** @internal
 *  injects a handler into a subtree, peers and children when a given
 *  subtrees name matches a passed in name.
 */
void
netsnmp_inject_handler_into_subtree(netsnmp_subtree *tp, const char *name,
                                    netsnmp_mib_handler *handler,
                                    const char *before_what)
{
    netsnmp_subtree *tptr;
    netsnmp_mib_handler *mh;

    for (tptr = tp; tptr != NULL; tptr = tptr->next) {
        /*  if (tptr->children) { 
              netsnmp_inject_handler_into_subtree(tptr->children,name,handler);
	    }   */
        if (strcmp(tptr->label_a, name) == 0) {
            DEBUGMSGTL(("injectHandler", "injecting handler %s into %s\n",
                        handler->handler_name, tptr->label_a));
            netsnmp_inject_handler_before(tptr->reginfo, _clone_handler(handler),
                                          before_what);
        } else if (tptr->reginfo != NULL &&
		   tptr->reginfo->handlerName != NULL &&
                   strcmp(tptr->reginfo->handlerName, name) == 0) {
            DEBUGMSGTL(("injectHandler", "injecting handler into %s/%s\n",
                        tptr->label_a, tptr->reginfo->handlerName));
            netsnmp_inject_handler_before(tptr->reginfo, _clone_handler(handler),
                                          before_what);
        } else {
            for (mh = tptr->reginfo->handler; mh != NULL; mh = mh->next) {
                if (mh->handler_name && strcmp(mh->handler_name, name) == 0) {
                    DEBUGMSGTL(("injectHandler", "injecting handler into %s\n",
                                tptr->label_a));
                    netsnmp_inject_handler_before(tptr->reginfo,
                                                  _clone_handler(handler),
                                                  before_what);
                    break;
                } else {
                    DEBUGMSGTL(("yyyinjectHandler",
                                "not injecting handler into %s\n",
                                mh->handler_name));
                }
            }
        }
    }
}

static int      doneit = 0;
/** @internal
 *  parses the "injectHandler" token line.
 */
void
parse_injectHandler_conf(const char *token, char *cptr)
{
    char            handler_to_insert[256], reg_name[256];
    subtree_context_cache *stc;
    netsnmp_mib_handler *handler;

    /*
     * XXXWWW: ensure instead that handler isn't inserted twice 
     */
    if (doneit)                 /* we only do this once without restart the agent */
        return;

    cptr = copy_nword(cptr, handler_to_insert, sizeof(handler_to_insert));
    handler = netsnmp_get_list_data(handler_reg, handler_to_insert);
    if (!handler) {
        config_perror("no such \"%s\" handler registered.");
        return;
    }

    if (!cptr) {
        config_perror("no INTONAME specified.  Can't do insertion.");
        return;
    }
    cptr = copy_nword(cptr, reg_name, sizeof(reg_name));

    for (stc = get_top_context_cache(); stc; stc = stc->next) {
        DEBUGMSGTL(("injectHandler", "Checking context tree %s (before=%s)\n",
                    stc->context_name, (cptr)?cptr:"null"));
        netsnmp_inject_handler_into_subtree(stc->first_subtree, reg_name,
                                            handler, cptr);
    }
}

/** @internal
 *  callback to ensure injectHandler parser doesn't do things twice
 *  @todo replace this with a method to check the handler chain instead.
 */
static int
handler_mark_doneit(int majorID, int minorID,
                    void *serverarg, void *clientarg)
{
    doneit = 1;
    return 0;
}

/** @internal
 *  register's the injectHandle parser token.
 */
void
netsnmp_init_handler_conf(void)
{
    snmpd_register_config_handler("injectHandler",
                                  parse_injectHandler_conf,
                                  NULL, "injectHandler NAME INTONAME [BEFORE_OTHER_NAME]");
    snmp_register_callback(SNMP_CALLBACK_LIBRARY,
                           SNMP_CALLBACK_POST_READ_CONFIG,
                           handler_mark_doneit, NULL);

    se_add_pair_to_slist("agent_mode", strdup("GET"), MODE_GET);
    se_add_pair_to_slist("agent_mode", strdup("GETNEXT"), MODE_GETNEXT);
    se_add_pair_to_slist("agent_mode", strdup("GETBULK"), MODE_GETBULK);
    se_add_pair_to_slist("agent_mode", strdup("SET_BEGIN"),
                         MODE_SET_BEGIN);
    se_add_pair_to_slist("agent_mode", strdup("SET_RESERVE1"),
                         MODE_SET_RESERVE1);
    se_add_pair_to_slist("agent_mode", strdup("SET_RESERVE2"),
                         MODE_SET_RESERVE2);
    se_add_pair_to_slist("agent_mode", strdup("SET_ACTION"),
                         MODE_SET_ACTION);
    se_add_pair_to_slist("agent_mode", strdup("SET_COMMIT"),
                         MODE_SET_COMMIT);
    se_add_pair_to_slist("agent_mode", strdup("SET_FREE"), MODE_SET_FREE);
    se_add_pair_to_slist("agent_mode", strdup("SET_UNDO"), MODE_SET_UNDO);

    se_add_pair_to_slist("babystep_mode", strdup("pre-request"),
                         MODE_BSTEP_PRE_REQUEST);
    se_add_pair_to_slist("babystep_mode", strdup("object_lookup"),
                         MODE_BSTEP_OBJECT_LOOKUP);
    se_add_pair_to_slist("babystep_mode", strdup("check_value"),
                         MODE_BSTEP_CHECK_VALUE);
    se_add_pair_to_slist("babystep_mode", strdup("row_create"),
                         MODE_BSTEP_ROW_CREATE);
    se_add_pair_to_slist("babystep_mode", strdup("undo_setup"),
                         MODE_BSTEP_UNDO_SETUP);
    se_add_pair_to_slist("babystep_mode", strdup("set_value"),
                         MODE_BSTEP_SET_VALUE);
    se_add_pair_to_slist("babystep_mode", strdup("check_consistency"),
                         MODE_BSTEP_CHECK_CONSISTENCY);
    se_add_pair_to_slist("babystep_mode", strdup("undo_set"),
                         MODE_BSTEP_UNDO_SET);
    se_add_pair_to_slist("babystep_mode", strdup("commit"),
                         MODE_BSTEP_COMMIT);
    se_add_pair_to_slist("babystep_mode", strdup("undo_commit"),
                         MODE_BSTEP_UNDO_COMMIT);
    se_add_pair_to_slist("babystep_mode", strdup("irreversible_commit"),
                         MODE_BSTEP_IRREVERSIBLE_COMMIT);
    se_add_pair_to_slist("babystep_mode", strdup("undo_cleanup"),
                         MODE_BSTEP_UNDO_CLEANUP);
    se_add_pair_to_slist("babystep_mode", strdup("post_request"),
                         MODE_BSTEP_POST_REQUEST);
    se_add_pair_to_slist("babystep_mode", strdup("original"), 0xffff);

    /*
     * xxx-rks: hmmm.. will this work for modes which are or'd together?
     *          I'm betting not...
     */
    se_add_pair_to_slist("handler_can_mode", strdup("GET/GETNEXT"),
                         HANDLER_CAN_GETANDGETNEXT);
    se_add_pair_to_slist("handler_can_mode", strdup("SET"),
                         HANDLER_CAN_SET);
    se_add_pair_to_slist("handler_can_mode", strdup("GETBULK"),
                         HANDLER_CAN_GETBULK);
    se_add_pair_to_slist("handler_can_mode", strdup("BABY_STEP"),
                         HANDLER_CAN_BABY_STEP);
}

/** @} */
