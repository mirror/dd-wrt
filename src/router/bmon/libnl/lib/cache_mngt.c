/*
 * lib/cache_mngt.c	Cache Management
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup utils
 * @defgroup cache_mngt Cache Management
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/utils.h>

/**
 * @name Access Functions
 * @{
 */

/**
 * Return the cache type of the cache operations
 * @arg ops		cache operations
 */
char *nl_cache_ops_get_name(struct nl_cache_ops *ops)
{
	return ops->co_name;
}

/** @} */

/**
 * @name Message Type Association
 * @{
 */

static struct nl_cache_ops *cache_ops;

/**
 * Associate a message type to a set of cache operations
 * @arg protocol		netlink protocol
 * @arg message_type		netlink message type
 *
 * Associates the specified netlink message type with
 * a registered set of cache operations.
 *
 * @return The cache operations or NULL if no association
 *         could be made.
 */
struct nl_cache_ops *nl_cache_mngt_associate(int protocol, int message_type)
{
	int i;
	struct nl_cache_ops *ops;

	for (ops = cache_ops; ops; ops = ops->co_next)
		for (i = 0; ops->co_msgtypes[i].mt_id >= 0; i++)
			if (ops->co_msgtypes[i].mt_id == message_type &&
			    ops->co_protocol == protocol)
				return ops;

	return NULL;
}

/**
 * Convert message type to character string.
 * @arg ops		Cache operations.
 * @arg msgtype		Message type.
 * @arg buf		Destination buffer.
 * @arg len		Size of destination buffer.
 *
 * Converts a message type to a character string and stores it in the
 * provided buffer.
 *
 * @return The destination buffer or the message type encoded in
 *         hexidecimal form if no match was found.
 */
char *nl_cache_mngt_type2name(struct nl_cache_ops *ops, int msgtype,
			      char *buf, size_t len)
{
	int i;

	for (i = 0; ops->co_msgtypes[i].mt_id >= 0; i++) {
		if (ops->co_msgtypes[i].mt_id == msgtype) {
			snprintf(buf, len, "%s::%s",
				 ops->co_name,
				 ops->co_msgtypes[i].mt_name);
			return buf;
		}
	}

	snprintf(buf, len, "%s->0x%x()", ops->co_name, msgtype);
	return buf;
}

/** @} */

/**
 * @name Cache Type Management
 * @{
 */

/**
 * Lookup the set cache operations of a certain cache type
 * @arg name		name of the cache type
 *
 * @return The cache operations or NULL if no operations
 *         have been registered under the specified name.
 */
struct nl_cache_ops *nl_cache_mngt_lookup(const char *name)
{
	struct nl_cache_ops *ops;

	for (ops = cache_ops; ops; ops = ops->co_next)
		if (!strcmp(ops->co_name, name))
			return ops;

	return NULL;
}

/**
 * Register a set of cache operations
 * @arg ops		cache operations
 *
 * Called by users of caches to announce the avaibility of
 * a certain cache type.
 *
 * @return 0 on success or a negative error code.
 */
int nl_cache_mngt_register(struct nl_cache_ops *ops)
{
	if (!ops->co_name)
		return nl_error(EINVAL, "No cache name specified");

	if (nl_cache_mngt_lookup(ops->co_name))
		return nl_error(EEXIST, "Cache operations already exist");
	    
	ops->co_next = cache_ops;
	cache_ops = ops;

	NL_DBG(1, "Registered cache operations %s\n", ops->co_name);

	return 0;
}

/**
 * Unregister a set of cache operations
 * @arg ops		cache operations
 *
 * Called by users of caches to announce a set of
 * cache operations is no longer available. The
 * specified cache operations must have been registered
 * previously using nl_cache_mngt_register()
 *
 * @return 0 on success or a negative error code
 */
int nl_cache_mngt_unregister(struct nl_cache_ops *ops)
{
	struct nl_cache_ops *t, **tp;

	for (tp = &cache_ops; (t=*tp) != NULL; tp = &t->co_next)
		if (t == ops)
			break;

	if (!t)
		return nl_error(ENOENT, "No such cache operations");

	NL_DBG(1, "Unregistered cache operations %s\n", ops->co_name);

	*tp = t->co_next;
	return 0;
}

/** @} */

/**
 * @name Global Cache Provisioning/Requiring
 * @{
 */

/**
 * Provide a cache for global use
 * @arg cache		cache to provide
 *
 * Offers the specified cache to be used by other modules.
 * Only one cache per type may be shared at a time,
 * a previsouly provided caches will be overwritten.
 */
void nl_cache_mngt_provide(struct nl_cache *cache)
{
	struct nl_cache_ops *ops;

	ops = nl_cache_mngt_lookup(cache->c_ops->co_name);
	if (!ops)
		BUG();
	else
		ops->co_major_cache = cache;
}

/**
 * Unprovide a cache for global use
 * @arg cache		cache to unprovide
 *
 * Cancels the offer to use a cache globally. The
 * cache will no longer be returned via lookups but
 * may still be in use.
 */
void nl_cache_mngt_unprovide(struct nl_cache *cache)
{
	struct nl_cache_ops *ops;

	ops = nl_cache_mngt_lookup(cache->c_ops->co_name);
	if (!ops)
		BUG();
	else if (ops->co_major_cache == cache)
		ops->co_major_cache = NULL;
}

/**
 * Demand the use of a global cache
 * @arg name		name of the required cache type
 *
 * Trys to find a cache of the specified type for global
 * use.
 *
 * @return A cache provided by another subsystem of the
 *         specified type marked to be available.
 */
struct nl_cache *nl_cache_mngt_require(const char *name)
{
	struct nl_cache_ops *ops;

	ops = nl_cache_mngt_lookup(name);
	if (!ops || !ops->co_major_cache) {
		fprintf(stderr, "Application BUG: Your application must "
			"call nl_cache_mngt_provide() and\nprovide a valid "
			"%s cache to be used for internal lookups.\nSee the "
			" API documentation for more details.\n", name);

		return NULL;
	}
	
	return ops->co_major_cache;
}

/** @} */

/** @} */
