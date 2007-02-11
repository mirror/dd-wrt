#include <net-snmp/net-snmp-config.h>

#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include <net-snmp/agent/cache_handler.h>

static netsnmp_cache  *cache_head = NULL;
static int             cache_outstanding_valid = 0;
static int             _cache_load( netsnmp_cache *cache );

#define CACHE_RELEASE_FREQUENCY 60      /* Check for expired caches every 60s */

void            release_cached_resources(unsigned int regNo,
                                         void *clientargs);

/** @defgroup cache_handler cache_handler
 *  Maintains a cache of data for use by lower level handlers.
 *  @ingroup utilities
 *  This helper checks to see whether the data has been loaded "recently"
 *  (according to the timeout for that particular cache) and calls the
 *  registered "load_cache" routine if necessary.
 *  The lower handlers can then work with this local cached data.
 *
 *  A timeout value of -1 will cause netsnmp_cache_check_expired() to
 *  always return true, and thus the cache will be reloaded for every
 *  request.
 *
 *  To minimze resource use by the agent, a periodic callback checks for
 *  expired caches, and will call the free_cache function for any expired
 *  cache.
 *
 *  The load_cache route should return a negative number if the cache
 *  was not successfully loaded. 0 or any positive number indicates successs.
 *
 *
 *  Several flags can be set to affect the operations on the cache.
 *
 *  If NETSNMP_CACHE_DONT_INVALIDATE_ON_SET is set, the free_cache method
 *  will not be called after a set request has processed. It is assumed that
 *  the lower mib handler using the cache has maintained cache consistency.
 *
 *  If NETSNMP_CACHE_DONT_FREE_BEFORE_LOAD is set, the free_cache method
 *  will not be called before the load_cache method is called. It is assumed
 *  that the load_cache routine will properly deal with being called with a
 *  valid cache.
 *
 *  If NETSNMP_CACHE_DONT_FREE_EXPIRED is set, the free_cache method will
 *  not be called with the cache expires. The expired flag will be set, but
 *  the valid flag will not be cleared. It is assumed that the load_cache
 *  routine will properly deal with being called with a valid cache.
 *
 *  If NETSNMP_CACHE_PRELOAD is set when a the cache handler is created,
 *  the cache load routine will be called immediately.
 *
 *  If NETSNMP_CACHE_DONT_AUTO_RELEASE is set, the periodic callback that
 *  checks for expired caches will skip the cache. The cache will only be
 *  checked for expiration when a request triggers the cache handler. This
 *  is useful if the cache has it's own periodic callback to keep the cache
 *  fresh.
 *
 *  If NETSNMP_CACHE_AUTO_RELOAD is set, a timer will be set up to reload
 *  the cache when it expires. This is useful for keeping the cache fresh,
 *  even in the absence of incoming snmp requests.
 *
 *
 *  Here are some suggestions for some common situations.
 *
 *  Cached File:
 *      If your table is based on a file that may periodically change,
 *      you can test the modification date to see if the file has
 *      changed since the last cache load. To get the cache helper to call
 *      the load function for every request, set the timeout to -1, which
 *      will cause the cache to always report that it is expired. This means
 *      that you will want to prevent the agent from flushing the cache when
 *      it has expired, and you will have to flush it manually if you
 *      detect that the file has changed. To accomplish this, set the
 *      following flags:
 *
 *          NETSNMP_CACHE_DONT_FREE_EXPIRED
 *          NETSNMP_CACHE_DONT_AUTO_RELEASE
 *
 *
 *  Constant (periodic) reload:
 *      If you want the cache kept up to date regularly, even if no requests
 *      for the table are received, you can have your cache load routine
 *      called periodically. This is very useful if you need to monitor the
 *      data for changes (eg a <i>LastChanged</i> object). You will need to
 *      prevent the agent from flushing the cache when it expires. Set the
 *      cache timeout to the frequency, in seconds, that you wish to
 *      reload your cache, and set the following flags:
 *
 *          NETSNMP_CACHE_DONT_FREE_EXPIRED
 *          NETSNMP_CACHE_DONT_AUTO_RELEASE
 *          NETSNMP_CACHE_AUTO_RELOAD
 *
 *  @{
 */

/** get cache head
 * @internal
 * unadvertised function to get cache head. You really should not
 * do this, since the internal storage mechanism might change.
 */
netsnmp_cache *
netsnmp_cache_get_head(void)
{
    return cache_head;
}

/** find existing cache
 */
netsnmp_cache *
netsnmp_cache_find_by_oid(oid * rootoid, int rootoid_len)
{
    netsnmp_cache  *cache;

    for (cache = cache_head; cache; cache = cache->next) {
        if (0 == netsnmp_oid_equals(cache->rootoid, cache->rootoid_len,
                                    rootoid, rootoid_len))
            return cache;
    }
    
    return NULL;
}

/** returns a cache
 */
netsnmp_cache *
netsnmp_cache_create(int timeout, NetsnmpCacheLoad * load_hook,
                     NetsnmpCacheFree * free_hook,
                     oid * rootoid, int rootoid_len)
{
    netsnmp_cache  *cache = NULL;

    cache = SNMP_MALLOC_TYPEDEF(netsnmp_cache);
    if (NULL == cache) {
        snmp_log(LOG_ERR,"malloc error in netsnmp_cache_create\n");
        return NULL;
    }
    cache->timeout = timeout;
    cache->load_cache = load_hook;
    cache->free_cache = free_hook;
    cache->enabled = 1;

    if(0 == cache->timeout)
        cache->timeout = netsnmp_ds_get_int(NETSNMP_DS_APPLICATION_ID,
                                            NETSNMP_DS_AGENT_CACHE_TIMEOUT);

    
    /*
     * Add the registered OID information, and tack
     * this onto the list for cache SNMP management
     *
     * Note that this list is not ordered.
     *    table_iterator rules again!
     */
    if (rootoid) {
        cache->rootoid = snmp_duplicate_objid(rootoid, rootoid_len);
        cache->rootoid_len = rootoid_len;
        cache->next = cache_head;
        if (cache_head)
            cache_head->prev = cache;
        cache_head = cache;
    }

    return cache;
}

/** callback function to call cache load function */
static void
_timer_reload(unsigned int regNo, void *clientargs)
{
    netsnmp_cache *cache = (netsnmp_cache *)clientargs;

    DEBUGMSGT(("cache_timer:start", "loading cache %p\n", cache));

    cache->expired = 1;

    _cache_load(cache);
}

/** starts the recurring cache_load callback */
unsigned int
netsnmp_cache_timer_start(netsnmp_cache *cache)
{
    if(NULL == cache)
        return 0;

    DEBUGMSGTL(( "cache_timer:start", "OID: "));
    DEBUGMSGOID(("cache_timer:start", cache->rootoid, cache->rootoid_len));
    DEBUGMSG((   "cache_timer:start", "\n"));

    if(0 != cache->timer_id) {
        snmp_log(LOG_WARNING, "cache has existing timer id.\n");
        return cache->timer_id;
    }
    
    if(! (cache->flags & NETSNMP_CACHE_AUTO_RELOAD)) {
        snmp_log(LOG_ERR,
                 "cache_timer_start called but auto_reload not set.\n");
        return 0;
    }

    cache->timer_id = snmp_alarm_register(cache->timeout, SA_REPEAT,
                                          _timer_reload, cache);
    if(0 == cache->timer_id) {
        snmp_log(LOG_ERR,"could not register alarm\n");
        return 0;
    }

    cache->flags &= ~NETSNMP_CACHE_AUTO_RELOAD;
    DEBUGMSGT(("cache_timer:start",
               "starting timer %d for cache %p\n", cache->timer_id, cache));
    return cache->timer_id;
}

/** stops the recurring cache_load callback */
void
netsnmp_cache_timer_stop(netsnmp_cache *cache)
{
    if(NULL == cache)
        return;

    if(0 == cache->timer_id) {
        snmp_log(LOG_WARNING, "cache has no timer id.\n");
        return;
    }

    DEBUGMSGT(("cache_timer:stop",
               "stopping timer %d for cache %p\n", cache->timer_id, cache));

    snmp_alarm_unregister(cache->timer_id);
    cache->flags |= NETSNMP_CACHE_AUTO_RELOAD;
}


/** returns a cache handler that can be injected into a given handler chain.  
 */
netsnmp_mib_handler *
netsnmp_cache_handler_get(netsnmp_cache* cache)
{
    netsnmp_mib_handler *ret = NULL;
    
    ret = netsnmp_create_handler("cache_handler",
                                 netsnmp_cache_helper_handler);
    if (ret) {
        ret->flags |= MIB_HANDLER_AUTO_NEXT;
        ret->myvoid = (void *) cache;
        
        if(NULL != cache) {
            if ((cache->flags & NETSNMP_CACHE_PRELOAD) && ! cache->valid) {
                /*
                 * load cache, ignore rc
                 * (failed load doesn't affect registration)
                 */
                (void)_cache_load(cache);
            }
            if (cache->flags & NETSNMP_CACHE_AUTO_RELOAD)
                netsnmp_cache_timer_start(cache);
            
        }
    }
    return ret;
}

/** returns a cache handler that can be injected into a given handler chain.  
 */
netsnmp_mib_handler *
netsnmp_get_cache_handler(int timeout, NetsnmpCacheLoad * load_hook,
                          NetsnmpCacheFree * free_hook,
                          oid * rootoid, int rootoid_len)
{
    netsnmp_mib_handler *ret = NULL;
    netsnmp_cache  *cache = NULL;

    ret = netsnmp_cache_handler_get(NULL);
    if (ret) {
        cache = netsnmp_cache_create(timeout, load_hook, free_hook,
                                     rootoid, rootoid_len);
        ret->myvoid = (void *) cache;
    }
    return ret;
}

/** functionally the same as calling netsnmp_register_handler() but also
 * injects a cache handler at the same time for you. */
int
netsnmp_cache_handler_register(netsnmp_handler_registration * reginfo,
                               netsnmp_cache* cache)
{
    netsnmp_mib_handler *handler = NULL;
    handler = netsnmp_cache_handler_get(cache);

    netsnmp_inject_handler(reginfo, handler);
    return netsnmp_register_handler(reginfo);
}

/** functionally the same as calling netsnmp_register_handler() but also
 * injects a cache handler at the same time for you. */
int
netsnmp_register_cache_handler(netsnmp_handler_registration * reginfo,
                               int timeout, NetsnmpCacheLoad * load_hook,
                               NetsnmpCacheFree * free_hook)
{
    netsnmp_mib_handler *handler = NULL;
    handler = netsnmp_get_cache_handler(timeout, load_hook, free_hook,
                                        reginfo->rootoid,
                                        reginfo->rootoid_len);

    netsnmp_inject_handler(reginfo, handler);
    return netsnmp_register_handler(reginfo);
}

NETSNMP_STATIC_INLINE char *
_build_cache_name(const char *name)
{
    char *dup = malloc(strlen(name) + strlen(CACHE_NAME) + 2);
    if (NULL == dup)
        return NULL;
    sprintf(dup, "%s:%s", CACHE_NAME, name);
    return dup;
}

/** Insert the cache information for a given request (PDU) */
void
netsnmp_cache_reqinfo_insert(netsnmp_cache* cache,
                             netsnmp_agent_request_info * reqinfo,
                             const char *name)
{
    char *cache_name = _build_cache_name(name);
    if (NULL == netsnmp_agent_get_list_data(reqinfo, cache_name)) {
        DEBUGMSGTL(("verbose:helper:cache_handler", " adding '%s' to %p\n",
                    cache_name, reqinfo));
        netsnmp_agent_add_list_data(reqinfo,
                                    netsnmp_create_data_list(cache_name,
                                                             cache, NULL));
    }
    SNMP_FREE(cache_name);
}

/** Extract the cache information for a given request (PDU) */
netsnmp_cache  *
netsnmp_cache_reqinfo_extract(netsnmp_agent_request_info * reqinfo,
                              const char *name)
{
    netsnmp_cache  *result;
    char *cache_name = _build_cache_name(name);
    result = netsnmp_agent_get_list_data(reqinfo, cache_name);
    SNMP_FREE(cache_name);
    return result;
}

/** Extract the cache information for a given request (PDU) */
netsnmp_cache  *
netsnmp_extract_cache_info(netsnmp_agent_request_info * reqinfo)
{
    return netsnmp_cache_reqinfo_extract(reqinfo, CACHE_NAME);
}


/** Check if the cache timeout has passed. Sets and return the expired flag. */
int
netsnmp_cache_check_expired(netsnmp_cache *cache)
{
    if(NULL == cache)
        return 0;
    
    if(!cache->valid || (NULL == cache->timestamp) || (-1 == cache->timeout))
        cache->expired = 1;
    else
        cache->expired = atime_ready(cache->timestamp, 1000 * cache->timeout);
    
    return cache->expired;
}

/** Reload the cache if required */
int
netsnmp_cache_check_and_reload(netsnmp_cache * cache)
{
    if (!cache) {
        DEBUGMSG(("helper:cache_handler", " no cache\n"));
        return 0;	/* ?? or -1 */
    }
    if (!cache->valid || netsnmp_cache_check_expired(cache))
        return _cache_load( cache );
    else {
        DEBUGMSG(("helper:cache_handler", " cached (%d)\n",
                  cache->timeout));
        return 0;
    }
}

/** Is the cache valid for a given request? */
int
netsnmp_cache_is_valid(netsnmp_agent_request_info * reqinfo, 
                       const char* name)
{
    netsnmp_cache  *cache = netsnmp_cache_reqinfo_extract(reqinfo, name);
    return (cache && cache->valid);
}

/** Is the cache valid for a given request?
 * for backwards compatability. netsnmp_cache_is_valid() is preferred.
 */
int
netsnmp_is_cache_valid(netsnmp_agent_request_info * reqinfo)
{
    return netsnmp_cache_is_valid(reqinfo, CACHE_NAME);
}

/** Implements the cache handler */
int
netsnmp_cache_helper_handler(netsnmp_mib_handler * handler,
                             netsnmp_handler_registration * reginfo,
                             netsnmp_agent_request_info * reqinfo,
                             netsnmp_request_info * requests)
{
    netsnmp_cache  *cache = NULL;
    netsnmp_handler_args cache_hint;

    DEBUGMSGTL(("helper:cache_handler", "Got request (%d) for %s: ",
                reqinfo->mode, reginfo->handlerName));
    DEBUGMSGOID(("helper:cache_handler", reginfo->rootoid,
                 reginfo->rootoid_len));
    DEBUGMSG(("helper:cache_handler", "\n"));

    netsnmp_assert(handler->flags & MIB_HANDLER_AUTO_NEXT);

    cache = (netsnmp_cache *) handler->myvoid;
    if (netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID,
                               NETSNMP_DS_AGENT_NO_CACHING) ||
        !cache || !cache->enabled || !cache->load_cache) {
        DEBUGMSG(("helper:cache_handler", " caching disabled or "
                  "cache not found, disabled or had no load method\n"));
        return SNMP_ERR_NOERROR;
    }

    /*
     * Make the handler-chain parameters available to
     * the cache_load hook routine.
     */
    cache_hint.handler = handler;
    cache_hint.reginfo = reginfo;
    cache_hint.reqinfo = reqinfo;
    cache_hint.requests = requests;
    cache->cache_hint = &cache_hint;

    switch (reqinfo->mode) {

    case MODE_GET:
    case MODE_GETNEXT:
    case MODE_GETBULK:
    case MODE_SET_RESERVE1: {

        /*
         * only touch cache once per pdu request, to prevent a cache
         * reload while a module is using cached data.
         *
         * XXX: this won't catch a request reloading the cache while
         * a previous (delegated) request is still using the cache.
         * maybe use a reference counter?
         */
        if (netsnmp_cache_is_valid(reqinfo, reginfo->handlerName))
            return SNMP_ERR_NOERROR;

        /*
         * call the load hook, and update the cache timestamp.
         * If it's not already there, add to reqinfo
         */
        netsnmp_cache_check_and_reload(cache);
        netsnmp_cache_reqinfo_insert(cache, reqinfo, reginfo->handlerName);
        /** next handler called automatically - 'AUTO_NEXT' */
        }
        return SNMP_ERR_NOERROR;

    case MODE_SET_RESERVE2:
    case MODE_SET_FREE:
    case MODE_SET_ACTION:
    case MODE_SET_UNDO:
        netsnmp_assert(netsnmp_cache_is_valid(reqinfo, reginfo->handlerName));
        /** next handler called automatically - 'AUTO_NEXT' */
        return SNMP_ERR_NOERROR;

        /*
         * A (successful) SET request wouldn't typically trigger a reload of
         *  the cache, but might well invalidate the current contents.
         * Only do this on the last pass through.
         */
    case MODE_SET_COMMIT:
        if (cache->valid && 
            ! (cache->flags & NETSNMP_CACHE_DONT_INVALIDATE_ON_SET) ) {
            cache->free_cache(cache, cache->magic);
            cache->valid = 0;
        }
        /** next handler called automatically - 'AUTO_NEXT' */
        return SNMP_ERR_NOERROR;

    default:
        snmp_log(LOG_WARNING, "cache_handler: Unrecognised mode (%d)\n",
                 reqinfo->mode);
        netsnmp_request_set_error_all(requests, SNMP_ERR_GENERR);
        return SNMP_ERR_GENERR;
    }
    netsnmp_request_set_error_all(requests, SNMP_ERR_GENERR);
    return SNMP_ERR_GENERR;     /* should never get here */
}

static void
_cache_free( netsnmp_cache *cache )
{
    if (NULL != cache->free_cache) {
        cache->free_cache(cache, cache->magic);
        cache->valid = 0;
    }
}

static int
_cache_load( netsnmp_cache *cache )
{
    int ret = -1;

    /*
     * If we've got a valid cache, then release it before reloading
     */
    if (cache->valid &&
        (! (cache->flags & NETSNMP_CACHE_DONT_FREE_BEFORE_LOAD)))
        _cache_free(cache);

    if ( cache->load_cache)
        ret = cache->load_cache(cache, cache->magic);
    if (ret < 0) {
        DEBUGMSG(("helper:cache_handler", " load failed (%d)\n",
                  ret));
        cache->valid = 0;
        return ret;
    }
    cache->valid = 1;
    cache->expired = 0;

    /*
     * If we didn't previously have any valid caches outstanding,
     *   then schedule a pass of the auto-release routine.
     */
    if ((!cache_outstanding_valid) &&
        (! (cache->flags & NETSNMP_CACHE_DONT_FREE_EXPIRED))) {
        snmp_alarm_register(CACHE_RELEASE_FREQUENCY,
                            0, release_cached_resources, NULL);
        cache_outstanding_valid = 1;
    }
    if (cache->timestamp)
        atime_setMarker(cache->timestamp);
    else
        cache->timestamp = atime_newMarker();
    DEBUGMSG(("helper:cache_handler", " loaded (%d)\n",
              cache->timeout));

    return ret;
}



/** run regularly to automatically release cached resources.
 * xxx - method to prevent cache from expiring while a request
 *     is being processed (e.g. delegated request). proposal:
 *     set a flag, which would be cleared when request finished
 *     (which could be acomplished by a dummy data list item in
 *     agent req info & custom free function).
 */
void
release_cached_resources(unsigned int regNo, void *clientargs)
{
    netsnmp_cache  *cache = NULL;

    cache_outstanding_valid = 0;
    DEBUGMSGTL(("helper:cache_handler", "running auto-release\n"));
    for (cache = cache_head; cache; cache = cache->next) {
        DEBUGMSGTL(("helper:cache_handler"," checking %p (flags 0x%x)\n",
                     cache, cache->flags));
        if (cache->valid &&
            ! (cache->flags & NETSNMP_CACHE_DONT_AUTO_RELEASE)) {
            DEBUGMSGTL(("helper:cache_handler","  releasing %p\n", cache));
            /*
             * Check to see if this cache has timed out.
             * If so, release the cached resources.
             * Otherwise, note that we still have at
             *   least one active cache.
             */
            if (netsnmp_cache_check_expired(cache)) {
                if(! (cache->flags & NETSNMP_CACHE_DONT_FREE_EXPIRED))
                    _cache_free(cache);
            } else {
                cache_outstanding_valid = 1;
            }
        }
    }
    /*
     * If there are any caches still valid & active,
     *   then schedule another pass.
     */
    if (cache_outstanding_valid) {
        snmp_alarm_register(CACHE_RELEASE_FREQUENCY,
                            0, release_cached_resources, NULL);
    }
}
/**  @} */

