/*
 * netlink/cache.h		Caching Module
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_CACHE_H_
#define NETLINK_CACHE_H_

#include <netlink/netlink.h>
#include <netlink/msg.h>
#include <netlink/utils.h>
#include <netlink/object.h>

struct nl_cache;
struct nl_cache_ops;

/* Access Functions */
extern int			nl_cache_nitems(struct nl_cache *);
extern int			nl_cache_nitems_filter(struct nl_cache *,
						       struct nl_object *);
extern struct nl_cache_ops *	nl_cache_get_ops(struct nl_cache *);
extern struct nl_object *	nl_cache_get_first(struct nl_cache *);
extern struct nl_object *	nl_cache_get_last(struct nl_cache *);
extern struct nl_object *	nl_cache_get_next(struct nl_object *);
extern struct nl_object *	nl_cache_get_prev(struct nl_object *);

/* Cache creation/deletion */
extern struct nl_cache *	nl_cache_alloc(void);
extern struct nl_cache *	nl_cache_alloc_from_ops(struct nl_cache_ops *);
extern struct nl_cache *	nl_cache_alloc_name(const char *);
extern void			nl_cache_clear(struct nl_cache *);
extern void			nl_cache_free(struct nl_cache *);

/* Cache modification */
extern int			nl_cache_add(struct nl_cache *,
					     struct nl_object *);
extern int			nl_cache_parse_and_add(struct nl_cache *,
						       struct nl_msg *);
extern void			nl_cache_delete(struct nl_cache *,
						struct nl_object *);
extern int			nl_cache_update(struct nl_handle *,
						struct nl_cache *);
extern int			nl_cache_pickup(struct nl_handle *,
						struct nl_cache *);

/* General */
extern int			nl_cache_is_empty(struct nl_cache *);

/* Dumping */
extern void			nl_cache_dump(struct nl_cache *,
					      struct nl_dump_params *);
extern void			nl_cache_dump_filter(struct nl_cache *,
						     struct nl_dump_params *,
						     struct nl_object *);

/* Iterators */
extern void			nl_cache_foreach(struct nl_cache *,
						 void (*cb)(struct nl_object *,
							    void *),
						 void *arg);
extern void			nl_cache_foreach_filter(struct nl_cache *,
							struct nl_object *,
							void (*cb)(struct
								   nl_object *,
								   void *),
							void *arg);

/* --- cache management --- */

/* Access Functions */
extern char *			nl_cache_ops_get_name(struct nl_cache_ops *);

/* Message type association */
extern struct nl_cache_ops *	nl_cache_mngt_associate(int, int);
extern char *			nl_cache_mngt_type2name(struct nl_cache_ops *,
							int, char *, size_t);

/* Cache type management */
extern struct nl_cache_ops *	nl_cache_mngt_lookup(const char *);
extern int			nl_cache_mngt_register(struct nl_cache_ops *);
extern int			nl_cache_mngt_unregister(struct nl_cache_ops *);

/* Global cache provisioning/requiring */
extern void			nl_cache_mngt_provide(struct nl_cache *);
extern void			nl_cache_mngt_unprovide(struct nl_cache *);
extern struct nl_cache *	nl_cache_mngt_require(const char *);

#endif
