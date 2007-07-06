/*
 * netlink/object.c	Generic Cacheable Object
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_OBJECT_H_
#define NETLINK_OBJECT_H_

#include <netlink/netlink.h>
#include <netlink/utils.h>

struct nl_cache;
struct nl_object;
struct nl_cache_ops;

/* General */
extern struct nl_object *	nl_object_alloc(size_t size);
extern struct nl_object *	nl_object_alloc_from_ops(struct nl_cache_ops *);
extern struct nl_object *	nl_object_alloc_name(const char *);
extern void			nl_object_free(struct nl_object *);
extern struct nl_object *	nl_object_clone(struct nl_object *obj);
extern void			nl_object_get(struct nl_object *);
extern void			nl_object_put(struct nl_object *);
extern int			nl_object_shared(struct nl_object *);
extern void			nl_object_dump(struct nl_object *,
					       struct nl_dump_params *);
extern int			nl_object_match(struct nl_object *,
						struct nl_object *);

/* Access Functions */
extern int			nl_object_get_refcnt(struct nl_object *);
extern struct nl_cache_ops *	nl_object_get_ops(struct nl_object *);
extern struct nl_cache *	nl_object_get_cache(struct nl_object *);
extern inline void *		nl_object_priv(struct nl_object *);

#endif
