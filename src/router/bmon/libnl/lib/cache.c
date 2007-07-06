/*
 * lib/cache.c		Caching Module
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
 * @defgroup cache Caching
 *
 * @code
 *   Cache Management             |    | Type Specific Cache Operations
 *                                      
 *                                |    | +----------------+ +------------+
 *                                       | request update | | msg_parser |
 *                                |    | +----------------+ +------------+
 *                                     +- - - - -^- - - - - - - -^- -|- - - -
 *    nl_cache_update:            |              |               |   |
 *          1) --------- co_request_update ------+               |   |
 *                                |                              |   |
 *          2) destroy old cache     +----------- pp_cb ---------|---+
 *                                |  |                           |
 *          3) ---------- nl_recvmsgs ----------+   +- cb_valid -+
 *             +--------------+   |  |          |   |
 *             | nl_cache_add |<-----+   + - - -v- -|- - - - - - - - - - -
 *             +--------------+   |      | +-------------+
 *                                         | nl_recvmsgs |
 *                                |      | +-----|-^-----+
 *                                           +---v-|---+
 *                                |      |   | nl_recv |
 *                                           +---------+
 *                                |      |                 Core Netlink
 * @endcode
 * 
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/object.h>
#include <netlink/utils.h>

static inline char *nl_cache_name(struct nl_cache *cache)
{
	return cache->c_ops ? cache->c_ops->co_name : "unknown";
}

/**
 * @name Access Functions
 * @{
 */

/**
 * Return the number of items in the cache
 * @arg cache		cache handle
 */
int nl_cache_nitems(struct nl_cache *cache)
{
	return cache->c_nitems;
}

/**
 * Return the number of items matching a filter in the cache
 * @arg cache		Cache object.
 * @arg filter		Filter object.
 */
int nl_cache_nitems_filter(struct nl_cache *cache, struct nl_object *filter)
{
	struct nl_cache_ops *ops = cache->c_ops;
	struct nl_object *obj;
	int nitems = 0;
	
	nl_list_for_each_entry(obj, &cache->c_items, ce_list) {
		if (filter && ops->co_filter && !ops->co_filter(obj, filter))
			continue;

		nitems++;
	}

	return nitems;
}

/**
 * Returns \b true if the cache is empty.
 * @arg cache		Cache to check
 * @return \a true if the cache is empty, otherwise \b false is returned.
 */
int nl_cache_is_empty(struct nl_cache *cache)
{
	return nl_list_empty(&cache->c_items);
}

/**
 * Return the operations set of the cache
 * @arg cache		cache handle
 */
struct nl_cache_ops *nl_cache_get_ops(struct nl_cache *cache)
{
	return cache->c_ops;
}

/**
 * Return the first element in the cache
 * @arg cache		cache handle
 */
struct nl_object *nl_cache_get_first(struct nl_cache *cache)
{
	if (nl_list_empty(&cache->c_items))
		return NULL;

	return nl_list_entry(cache->c_items.next,
			     struct nl_object, ce_list);
}

/**
 * Return the last element in the cache
 * @arg cache		cache handle
 */
struct nl_object *nl_cache_get_last(struct nl_cache *cache)
{
	if (nl_list_empty(&cache->c_items))
		return NULL;

	return nl_list_entry(cache->c_items.prev,
			     struct nl_object, ce_list);
}

/**
 * Return the next element in the cache
 * @arg obj		current object
 */
struct nl_object *nl_cache_get_next(struct nl_object *obj)
{
	if (nl_list_at_tail(obj, &obj->ce_cache->c_items, ce_list))
		return NULL;
	else
		return nl_list_entry(obj->ce_list.next,
				     struct nl_object, ce_list);
}

/**
 * Return the previous element in the cache
 * @arg obj		current object
 */
struct nl_object *nl_cache_get_prev(struct nl_object *obj)
{
	if (nl_list_at_head(obj, &obj->ce_cache->c_items, ce_list))
		return NULL;
	else
		return nl_list_entry(obj->ce_list.prev,
				     struct nl_object, ce_list);
}

/** @} */

/**
 * @name Cache Creation/Deletion
 * @{
 */

/**
 * Allocate an empty cache of no certain type
 * 
 * @return A newly allocated and initialized cache.
 */
struct nl_cache *nl_cache_alloc(void)
{
	struct nl_cache *cache;

	cache = calloc(1, sizeof(*cache));
	if (!cache) {
		nl_errno(ENOMEM);
		return NULL;
	}

	nl_init_list_head(&cache->c_items);
	NL_DBG(2, "Allocated cache %p <%s>.\n", cache, nl_cache_name(cache));

	return cache;
}

/**
 * Allocate an empty cache based on cache operations
 * @arg ops		cache operations to base the cache on
 * @return A newly allocated and initialized cache.
 */
struct nl_cache *nl_cache_alloc_from_ops(struct nl_cache_ops *ops)
{
	struct nl_cache *new;

	new = nl_cache_alloc();
	if (!new)
		return NULL;

	new->c_ops = ops;

	return new;
}

/**
 * Allocate an empty cache based on type name
 * @arg kind		Name of cache type
 * @return A newly allocated and initialized cache.
 */
struct nl_cache *nl_cache_alloc_name(const char *kind)
{
	struct nl_cache_ops *ops;

	ops = nl_cache_mngt_lookup(kind);
	if (!ops) {
		nl_error(ENOENT, "Unable to lookup cache \"%s\"", kind);
		return NULL;
	}

	return nl_cache_alloc_from_ops(ops);
}

/**
 * Clear a cache.
 * @arg cache		cache to clear
 *
 * Removes all elements of a cache.
 */
void nl_cache_clear(struct nl_cache *cache)
{
	struct nl_object *obj, *tmp;

	NL_DBG(1, "Clearing cache %p <%s>...\n", cache, nl_cache_name(cache));

	nl_list_for_each_entry_safe(obj, tmp, &cache->c_items, ce_list)
		nl_cache_delete(cache, obj);
}

/**
 * Free a cache.
 * @arg cache		Cache to free.
 *
 * Removes all elements of a cache and frees all memory.
 *
 * @note Use this function if you are working with allocated caches.
 */
void nl_cache_free(struct nl_cache *cache)
{
	nl_cache_clear(cache);
	NL_DBG(1, "Freeing cache %p <%s>...\n", cache, nl_cache_name(cache));
	free(cache);
}

/** @} */

/**
 * @name Cache Modifications
 * @{
 */

/**
 * Add an element to the cache.
 * @arg cache		cache to add a element to
 * @arg obj		Common obj to be added to the cache
 *
 * Adds the object \c obj to the tail of the cache \c cache and. The
 * cache is enlarged as needed.
 *
 * @return 0 or a negative error code.
 */
int nl_cache_add(struct nl_cache *cache, struct nl_object *obj)
{
	struct nl_object *new;

	if (nl_object_shared(obj)) {
		new = nl_object_clone(obj);
		if (!new)
			return nl_errno(ENOMEM);

		nl_object_put(obj);
	} else
		new = obj;

	new->ce_cache = cache;

	nl_list_add_tail(&new->ce_list, &cache->c_items);
	cache->c_nitems++;

	NL_DBG(1, "Added %p to cache %p <%s>.\n",
	       new, cache, nl_cache_name(cache));

	return 0;
}

static int subsys_parse_cb(struct nl_object *c, struct nl_parser_param *p)
{
	return nl_cache_add((struct nl_cache *) p->pp_arg, c);
}

/** @cond SKIP */
int nl_cache_parse(struct nl_cache_ops *ops, struct sockaddr_nl *who,
		   struct nlmsghdr *nlh, struct nl_parser_param *params)
{
	int i, len, err, hdrsize;

	hdrsize = ops->co_hdrsize;
	len = nlh->nlmsg_len - nlmsg_msg_size(hdrsize);
	if (len < 0) {
		err = nl_error(EINVAL, "netlink message too short to "
				       "of kind %s", ops->co_name);
		goto errout;
	}

	for (i = 0; ops->co_msgtypes[i].mt_id >= 0; i++)
		if (ops->co_msgtypes[i].mt_id == nlh->nlmsg_type)
			return ops->co_msg_parser(who, nlh, params);

	err = nl_error(EINVAL, "Unsupported netlink message type %d",
		       nlh->nlmsg_type);
errout:
	return err;
}
/** @endcond */

/**
 * Parse a netlink message and add it to the cache.
 * @arg cache		cache to add element to
 * @arg msg		netlink message
 *
 * Parses a netlink message by calling the cache specific message parser
 * and adds the new element to the cache.
 *
 * @return 0 or a negative error code.
 */
int nl_cache_parse_and_add(struct nl_cache *cache, struct nl_msg *msg)
{
	struct nl_parser_param p = {
		.pp_cb = subsys_parse_cb,
		.pp_arg = cache,
	};

	return nl_cache_parse(cache->c_ops, NULL, nlmsg_hdr(msg), &p);
}

/**
 * Delete an element from a cache.
 * @arg cache		cache to delete the element from
 * @arg obj		Object to delete
 *
 * Deletes the object \c obj from the cache \c cache.
 */
void nl_cache_delete(struct nl_cache *cache, struct nl_object *obj)
{
	if (obj->ce_cache != cache)
		BUG();

	nl_list_del(&obj->ce_list);
	obj->ce_cache = NULL;
	nl_object_put(obj);
	cache->c_nitems--;

	NL_DBG(1, "Deleted %p from cache %p <%s>.\n",
	       obj, cache, nl_cache_name(cache));
}

/** @cond SKIP */
struct update_xdata {
	struct nl_cache_ops *ops;
	struct nl_parser_param *params;
};
/** @endcond */

static int update_msg_parser(struct nl_msg *msg, void *arg)
{
	struct update_xdata *x = arg;
	
	return nl_cache_parse(x->ops, &msg->nm_src, msg->nm_nlh, x->params);
}

/**
 * Pickup a netlink dump response and put it into a cache.
 * @arg handle		Netlink handle.
 * @arg cache		Cache to put items into.
 *
 * Waits for netlink messages to arrive, parses them and puts them into
 * the specified cache.
 *
 * @return 0 on success or a negative error code.
 */
int nl_cache_pickup(struct nl_handle *handle, struct nl_cache *cache)
{
	int err;
	struct nl_cache_ops *ops = cache->c_ops;
	struct nl_cb *cb;
	struct nl_parser_param p = {
		.pp_cb = subsys_parse_cb,
		.pp_arg = cache,
	};
	struct update_xdata x = {
		.ops = ops,
		.params = &p,
	};

	NL_DBG(1, "Filling cache %p <%s>...\n", cache, nl_cache_name(cache));

	cb = nl_cb_clone(nl_handle_get_cb(handle));
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, update_msg_parser, &x);

	err = nl_recvmsgs(handle, cb);
	if (err < 0)
		NL_DBG(2, "While picking up for %p <%s>, recvmsgs() returned " \
		       "%d: %s", cache, nl_cache_name(cache),
		       err, nl_geterror());

	nl_cb_destroy(cb);

	return err;
}

/**
 * Update (synchronize) a local cache with the kernel.
 * @arg handle		netlink handle
 * @arg cache		cache to update
 *
 * Updates the local cache \c cache with the state in the kernel. During
 * this process the cache gets emptied and refilled with the new content
 * received from the kernel.
 *
 * @return 0 or a negative error code.
 */
int nl_cache_update(struct nl_handle *handle, struct nl_cache *cache)
{
	int err;
	struct nl_cache_ops *ops = cache->c_ops;

	err = ops->co_request_update(cache, handle);
	if (err < 0)
		return err;

	NL_DBG(2, "Upading cache %p <%s>, request sent, waiting for dump...\n",
	       cache, nl_cache_name(cache));
	nl_cache_clear(cache);

	return nl_cache_pickup(handle, cache);
}

/** @} */

/**
 * @name Dumping
 * @{
 */

/**
 * Dump all elements of a cache.
 * @arg cache		cache to dump
 * @arg params		dumping parameters
 *
 * Dumps all elements of the \a cache to the file descriptor \a fd.
 */
void nl_cache_dump(struct nl_cache *cache, struct nl_dump_params *params)
{
	NL_DBG(1, "Dumping cache %p <%s>\n", cache, nl_cache_name(cache));
	nl_cache_dump_filter(cache, params, NULL);
}

/**
 * Dump all elements of a cache (filtered).
 * @arg cache		cache to dump
 * @arg params		dumping parameters (optional)
 * @arg filter		filter object
 *
 * Dumps all elements of the \a cache to the file descriptor \a fd
 * given they match the given filter \a filter.
 */
void nl_cache_dump_filter(struct nl_cache *cache,
			  struct nl_dump_params *params,
			  struct nl_object *filter)
{
	int type = params ? params->dp_type : NL_DUMP_FULL;
	struct nl_cache_ops *ops = cache->c_ops;
	struct nl_object *obj;

	if (type > NL_DUMP_MAX || type < 0)
		BUG();

	if (!ops->co_dump[type])
		return;

	nl_list_for_each_entry(obj, &cache->c_items, ce_list) {
		if (filter && obj->ce_ops != filter->ce_ops)
			continue;
		
		if (filter && ops->co_filter && !ops->co_filter(obj, filter))
			continue;

		dump_from_ops(obj, params);
	}
}

/** @} */

/**
 * @name Iterators
 * @{
 */

/**
 * Call a callback on each element of the cache.
 * @arg cache		cache to iterate on
 * @arg cb		callback function
 * @arg arg		argument passed to callback function
 *
 * Calls a callback function \a cb on each element of the \a cache.
 * The argument \a arg is passed on the callback function.
 */
void nl_cache_foreach(struct nl_cache *cache,
		      void (*cb)(struct nl_object *, void *), void *arg)
{
	nl_cache_foreach_filter(cache, NULL, cb, arg);
}

/**
 * Call a callback on each element of the cache (filtered).
 * @arg cache		cache to iterate on
 * @arg filter		filter object
 * @arg cb		callback function
 * @arg arg		argument passed to callback function
 *
 * Calls a callback function \a cb on each element of the \a cache
 * that matches the \a filter. The argument \a arg is passed on
 * to the callback function.
 */
void nl_cache_foreach_filter(struct nl_cache *cache, struct nl_object *filter,
			     void (*cb)(struct nl_object *, void *), void *arg)
{
	struct nl_object *obj, *tmp;
	struct nl_cache_ops *ops = cache->c_ops;

	nl_list_for_each_entry_safe(obj, tmp, &cache->c_items, ce_list) {
		if (filter && ops->co_filter && !ops->co_filter(obj, filter))
			continue;

		cb(obj, arg);
	}
}

/** @} */

/** @} */
