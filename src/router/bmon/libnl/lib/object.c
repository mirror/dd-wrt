/*
 * lib/object.c		Generic Cacheable Object
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup cache
 * @defgroup object Cacheable Object
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/object.h>
#include <netlink/utils.h>

static inline struct nl_cache_ops *obj_ops(struct nl_object *obj)
{
	if (!obj->ce_ops)
		BUG();

	return obj->ce_ops;
}

/**
 * @name Object Creation/Deletion
 * @{
 */

/**
 * Allocate a cacheable object
 * @arg size		size of object
 * @return The new object or NULL.
 */
struct nl_object *nl_object_alloc(size_t size)
{
	struct nl_object *new;

	if (size < sizeof(*new))
		BUG();

	new = calloc(1, size);
	if (!new) {
		nl_errno(ENOMEM);
		return NULL;
	}

	new->ce_refcnt = 1;
	nl_init_list_head(&new->ce_list);

	return new;
}

/**
 * Allocate a new object of kind specified by the operations handle
 * @arg ops		cache operations handle
 * @return The new object or NULL
 */
struct nl_object *nl_object_alloc_from_ops(struct nl_cache_ops *ops)
{
	struct nl_object *new;

	new = nl_object_alloc(ops->co_size);
	if (new) {
		new->ce_ops = ops;
		if (ops->co_constructor)
			ops->co_constructor(new);
	}

	return new;
}

/**
 * Allocate a new object of kind specified by the name
 * @arg kind		name of object type
 * @return The new object or nULL
 */
struct nl_object *nl_object_alloc_name(const char *kind)
{
	struct nl_cache_ops *ops;

	ops = nl_cache_mngt_lookup(kind);
	if (!ops) {
		nl_error(ENOENT, "Unable to lookup cache kind \"%s\"", kind);
		return NULL;
	}

	return nl_object_alloc_from_ops(ops);
}

struct nl_derived_object {
	NLHDR_COMMON
	char data;
};

/**
 * Allocate a new object and copy all data from an existing object
 * @arg obj		object to inherite data from
 * @return The new object or NULL.
 */
struct nl_object *nl_object_clone(struct nl_object *obj)
{
	struct nl_object *new;
	struct nl_cache_ops *ops = obj_ops(obj);
	int doff = offsetof(struct nl_derived_object, data);
	int size;

	new = nl_object_alloc(ops->co_size);
	if (!new)
		return NULL;

	size = ops->co_size - doff;
	if (size < 0)
		BUG();

	new->ce_cache = obj->ce_cache;
	new->ce_ops = obj->ce_ops;
	new->ce_msgtype = obj->ce_msgtype;
	if (ops->co_free_data) {
		new->ce_dataref = obj;
		nl_object_get(obj);
	}

	if (size)
		memcpy((void *)new + doff, (void *)obj + doff, size);

	return new;
}

/**
 * Free a cacheable object
 * @arg obj		object to free
 *
 * @return 0 or a negative error code.
 */
void nl_object_free(struct nl_object *obj)
{
	struct nl_cache_ops *ops = obj_ops(obj);

	if (obj->ce_refcnt > 0)
		NL_DBG(1, "Warning: Freeing object in used...\n");

	if (obj->ce_dataref)
		nl_object_put(obj->ce_dataref);
	else if (ops->co_free_data)
		ops->co_free_data(obj);

	free(obj);
}

/** @} */

/**
 * @name Reference Management
 * @{
 */

/**
 * Acquire a reference on a object
 * @arg obj		object to acquire reference from
 */
void nl_object_get(struct nl_object *obj)
{
	obj->ce_refcnt++;
}

/**
 * Release a reference from an object
 * @arg obj		object to release reference from
 */
void nl_object_put(struct nl_object *obj)
{
	if (!obj)
		return;

	obj->ce_refcnt--;

	if (obj->ce_refcnt < 0)
		BUG();

	if (obj->ce_refcnt <= 0)
		nl_object_free(obj);
}

/**
 * Check whether this object is used by multiple users
 * @arg obj		object to check
 * @return true or false
 */
int nl_object_shared(struct nl_object *obj)
{
	return obj->ce_refcnt > 1;
}

/** @} */

/**
 * @name Utillities
 * @{
 */

/**
 * Dump this object according to the specified parameters
 * @arg obj		object to dump
 * @arg params		dumping parameters
 */
void nl_object_dump(struct nl_object *obj, struct nl_dump_params *params)
{
	dump_from_ops(obj, params);
}

/**
 * Match a filter against an object
 * @arg obj		object to check
 * @arg filter		filter object
 *
 * @return 0 if the object matches the filter or non-zero
 *           if no filter procedure is available or if the
 *           filter does not match.
 */
int nl_object_match(struct nl_object *obj, struct nl_object *filter)
{
	struct nl_cache_ops *ops = obj->ce_ops;

	if (ops == filter->ce_ops &&
	    ops->co_filter && !ops->co_filter(obj, filter))
		return 1;
	else
		return 0;
}

/** @} */

/**
 * @name Access Functions
 * @{
 */

/**
 * Get reference count of object
 * @arg obj		object handle
 */
int nl_object_get_refcnt(struct nl_object *obj)
{
	return obj->ce_refcnt;
}

/**
 * Get cache operations of object
 * @arg obj		object handle
 */
struct nl_cache_ops *nl_object_get_ops(struct nl_object *obj)
{
	return obj->ce_ops;
}

/**
 * Get cache this object is in
 * @arg obj		object handle
 * @return cache handle or NULL if object is not associated to a cache
 */
struct nl_cache *nl_object_get_cache(struct nl_object *obj)
{
	return obj->ce_cache;
}

/**
 * Get the private data of object
 * @arg obj		object handle
 */
inline void *nl_object_priv(struct nl_object *obj)
{
	return obj;
}

/** @} */

/** @} */
