/*
 * cache.c		Caching Module
 *
 * Copyright (c) 2003-2004 Thomas Graf <tgraf@suug.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */

/**
 * @ingroup nl
 * @defgroup cache Cache Management
 * Module to manage generic caches.
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
#include <netlink/helpers.h>

/**
 * @name Cache Management
 * @{
 */

static int subsys_parse_cb(struct nl_common *c, struct nl_parser_param *p)
{
	return nl_cache_add((struct nl_cache *) p->pp_arg, c);
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
	struct nl_cache_ops *ops;
	struct nl_cb cb;
	struct nl_parser_param p = {
		.pp_cb = subsys_parse_cb,
		.pp_arg = cache,
	};

	NL_DBG(1, "Updating cache %p...\n", cache);

	ops = cache->c_ops;
	err = ops->co_request_update(cache, handle);
	if (err < 0)
		return err;

	nl_cache_destroy(cache);

	memcpy(&cb, &handle->h_cb, sizeof(cb));
	cb.cb_valid = ops->co_msg_parser;
	cb.cb_valid_arg = (void *) &p;

	return nl_recvmsgs(handle, &cb);
}

/**
 * Enlarge a cache by adding new elements.
 * @arg cache		cache to enlarge
 * @arg add		additional elements to be added
 *
 * Reallocates the internal cache with \c add new elements added. The
 * new elements are zeroed out. Changes the internal cache memory
 * address.
 *
 * @return New size of cache or negative error code
 */
int nl_cache_enlarge(struct nl_cache *cache, size_t add)
{
	size_t old_size = cache->c_size * cache->c_type_size;
	size_t new_size;

	NL_DBG(1, "Enlarging cache %p by %zd bytes.\n", cache, add);

	cache->c_size += add;
	new_size = cache->c_size * cache->c_type_size;
	
	cache->c_cache = realloc(cache, new_size);
	if (cache->c_cache == NULL)
		return nl_error(ENOMEM, "Out of memory");

	memset(cache->c_cache + old_size, 0, new_size - old_size);

	return cache->c_size;
}

/**
 * Return element at given index.
 * @arg cache		cache to look in
 * @arg index		index of element to be returned
 *
 * Looks up the element in the \c cache at the given \c index.
 *
 * @return Pointer to element or NULL if index is out of bounds.
 */
struct nl_common * nl_cache_get(struct nl_cache *cache, int index)
{
	if (index >= 0 && index < cache->c_index && cache->c_cache)
		return NL_CACHE_ELEMENT_AT(cache, index);
	else
		return NULL;
}

/**
 * Compare two cache elements
 * @arg cache		any cache of required type
 * @arg obj		object to check
 * @arg filter		filter object
 *
 * @return 0 if the object matches the filter or non-zero
 *           no filter procedure is available or if the
 *           filter does not match.
 */
int nl_cache_filter(struct nl_cache *cache, struct nl_common *obj,
		    struct nl_common *filter)
{
	struct nl_cache_ops *ops = cache->c_ops;

	if (ops->co_filter && !ops->co_filter(obj, filter))
		return 1;
	else
		return 0;
}

/**
 * Destroy a cache.
 * @arg cache		cache to destroy
 *
 * Removes all elements of a cache and destroys the cache by freeing all
 * internal memory allocations.
 */
void nl_cache_destroy(struct nl_cache *cache)
{
	NL_DBG(1, "Destroying cache %p...\n", cache);
	
	while (cache->c_index > 0)
		nl_cache_delete_element(cache, NL_CACHE_ELEMENT_AT(cache, 0));

	if (cache->c_cache) {
		free(cache->c_cache);
		cache->c_cache = NULL;
	}

	cache->c_size = 0;
	cache->c_index = 0;
}

/**
 * Destroy and free a cache.
 * @arg cache		cache to destroy
 *
 * Removes all elements of a cache and destroys the cache by freeing all
 * internal memory allocations. Additionally the cache itself gets
 * freed.
 *
 * @note Use this function if you are working with allocated caches.
 */
void nl_cache_destroy_and_free(struct nl_cache *cache)
{
	NL_DBG(1, "Destroying and freeing cache %p...\n", cache);
	nl_cache_destroy(cache);
	free(cache);
}

static void remove_entry(struct nl_cache *c, int index)
{
	int i;

	memmove((void *) NL_CACHE_ELEMENT_AT(c, index),
		(void *) NL_CACHE_ELEMENT_AT(c, index+1),
		(c->c_index - index - 1) * c->c_type_size);

	c->c_index--;
	memset(NL_CACHE_ELEMENT_AT(c, c->c_index), 0, c->c_type_size);

	for (i = 0; i < c->c_index; i++)
		NL_CACHE_ELEMENT_AT(c, i)->ce_index = i;
}

/**
 * Deletes an element from a cache.
 * @arg cache		cache the element is in
 * @arg obj		element (object) to remove
 *
 * Removes the element \c obj from the cache. The gets realigned afterwards.
 * The removed object's reference count is decremented anf the object is
 * freed if it hit zero.
 *
 * @return 0 or a negative error code.
 */
int nl_cache_delete_element(struct nl_cache *cache, struct nl_common *obj)
{
	if (obj->ce_refcnt <= 1) {
		if (cache->c_ops->co_free_data)
			(*cache->c_ops->co_free_data)(obj);
	} else
		obj->ce_refcnt--;
	
	remove_entry(cache, obj->ce_index);
	return 0;
}

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
int nl_cache_add(struct nl_cache *cache, struct nl_common *obj)
{
	int err, i = cache->c_index;

	if (obj->ce_size != cache->c_type_size) {
		fprintf(stderr, "size mismatch: ce_size=%d type-size=%zd\n",
		    obj->ce_size, cache->c_type_size);
		BUG();
	}

	if (cache->c_size == 0) {
		cache->c_size = 64;
		cache->c_cache = calloc(cache->c_size, cache->c_type_size);
		if (cache->c_cache == NULL)
			return nl_error(ENOMEM, "Out of memory");
	}

	while (i >= cache->c_size)
		if ((err = nl_cache_enlarge(cache, 64)) < 0)
			return err;

	obj->ce_index = i;
	obj->ce_refcnt++;
	memcpy(NL_CACHE_ELEMENT_AT(cache, i), obj, obj->ce_size);
	cache->c_index++;

	NL_DBG(1, "Added %p to cache %p.\n", obj, cache);

	return 0;
}

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
int nl_cache_parse(struct nl_cache *cache, struct nl_msg *msg)
{
	struct nl_parser_param p = {
		.pp_cb = subsys_parse_cb,
		.pp_arg = cache,
	};

	return cache->c_ops->co_msg_parser(NULL, nl_msg_get(msg), &p);
}

/**
 * Returns \b true if the cache is empty.
 * @arg cache		Cache to check
 * @return \a true if the cache is empty, otherwise \b false is returned.
 */
inline int nl_cache_is_empty(struct nl_cache *cache)
{
	return !cache->c_index;
}

/**
 * Dump all elements of a cache.
 * @arg cache		cache to dump
 * @arg fd		file descriptor to dump to
 * @arg params		dumping parameters
 *
 * Dumps all elements of the \a cache to the file descriptor \a fd.
 */
void nl_cache_dump(struct nl_cache *cache, FILE *fd, struct nl_dump_params *params)
{
	NL_DBG(1, "Dumping cache %p to fd %d.\n", cache, fileno(fd));
	nl_cache_dump_filter(cache, fd, params, NULL);
}

/**
 * Dump all elements of a cache (filtered).
 * @arg cache		cache to dump
 * @arg fd		file descriptor to dump to
 * @arg params		dumping parameters (optional)
 * @arg filter		filter object
 *
 * Dumps all elements of the \a cache to the file descriptor \a fd
 * given they match the given filter \a filter.
 */
void nl_cache_dump_filter(struct nl_cache *cache, FILE *fd,
			  struct nl_dump_params *params, struct nl_common *filter)
{
	int i, type = params ? params->dp_type : NL_DUMP_FULL;
	struct nl_cache_ops *ops = cache->c_ops;

	if (type > NL_DUMP_MAX || type < 0)
		BUG();

	if (ops->co_dump[type] == NULL)
		return;

	for (i = 0; i < cache->c_index; i++) {
		struct nl_common *obj = NL_CACHE_ELEMENT_AT(cache, i);

		if (filter && ops->co_filter && !ops->co_filter(obj, filter))
			continue;

		ops->co_dump[type](cache, obj, fd, params);
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
		      void (*cb)(struct nl_common *, void *), void *arg)
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
void nl_cache_foreach_filter(struct nl_cache *cache, struct nl_common *filter,
			     void (*cb)(struct nl_common *, void *), void *arg)
{
	int i;

	for (i = 0; i < cache->c_index; i++) {
		struct nl_cache_ops *ops = cache->c_ops;
		struct nl_common *obj = NL_CACHE_ELEMENT_AT(cache, i);

		if (filter && ops->co_filter && !ops->co_filter(obj, filter))
			continue;

		cb(obj, arg);
	}
}

/** @} */

/**
 * @name Library Internal Cache Lookups
 * @{
 */

static struct nl_cache *ca_tbl[RTNL_MAX+1];

/**
 * Provide a cache to the library.
 * @arg cache		cache to provide
 *
 * Provide a cache to the library for internal lookups. The application
 * must call this prior to calling functions which need access to caches
 * of other subsystems such as dumping routines.
 *
 * Multiple calls for a cache of the same type results in overwriting
 * of the previous entry.
 */
void nl_cache_provide(struct nl_cache *cache)
{
	if (cache->c_type > RTNL_MAX)
		BUG();

	ca_tbl[cache->c_type] = cache;
}

/**
 * Unprovide a cache to the library.
 * @arg cache		cache to unprovide
 *
 * Removes a \c cache of the list of provided caches again. You must do
 * so before destroying a cache.
 */
void nl_cache_unprovide(struct nl_cache *cache)
{
	if (cache->c_type > RTNL_MAX)
		BUG();

	ca_tbl[cache->c_type] = NULL;
}

/**
 * Lookup a cache provided by the application.
 * @arg type		type of cache
 *
 * Looks up the cache provided by the user with the given \c type.
 * Prints a verbose error message and quites in case the application
 * forgot to set it in advance. A failure of this function is a
 * programming bug and should never occur at runtime.
 *
 * @return The previously provided cache of given \c type.
 */
struct nl_cache * nl_cache_lookup(int type)
{
	if (type > RTNL_MAX)
		BUG();

	if (NULL == ca_tbl[type]) {
		fprintf(stderr, "Application BUG: Your application must " \
			"call nl_cache_provide() and provide \na valid " \
			"cache to be used for internal lookups. See the " \
			" API documentation for more details.\n");
		exit(1);
	}

	return ca_tbl[type];
}

/** @} */
/** @} */
