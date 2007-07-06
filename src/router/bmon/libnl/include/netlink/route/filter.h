/*
 * filter.h          libnl filter.h module
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

#ifndef NETLINK_FILTER_H_
#define NETLINK_FILTER_H_

#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/route/tc.h>
#include <netlink/helpers.h>

/**
 * @name Available Attributes Flags
 * @ingroup filter
 * @anchor filter_avail_attrs
 * Flags used in rtnl_filter::f_mask to indicate which attributes are
 * present in a filter.
 *
 * The corresponding flag is automatically set if a attribute is set
 * via a \c rtnl_filter_set_* function or if the class was created by
 * the internal message parser, e.g. if it comes from a cache.
 *
 * @par Example: Check if a filter has a parent and options attribute set
 * @code
 * if (filter->f_mask & (FILTER_HAS_PARENT | FILTER_HAS_OPTS))
 *         // action goes here
 * @endcode
 * @{
 */

#define FILTER_HAS_HANDLE    TCA_HAS_HANDLE	/**< Has handle */
#define FILTER_HAS_PARENT    TCA_HAS_PARENT	/**< Has parent */
#define FILTER_HAS_IFINDEX   TCA_HAS_IFINDEX	/**< Has interface index */
#define FILTER_HAS_KIND      TCA_HAS_KIND	/**< Has kind */
#define FILTER_HAS_FAMILY    TCA_HAS_FAMILY	/**< Has family */
#define FILTER_HAS_INFO      TCA_HAS_INFO	/**< Has info */
#define FILTER_HAS_OPTS      TCA_HAS_OPTS	/**< Has options */
#define FILTER_HAS_STATS     TCA_HAS_STATS	/**< Has statistics */
#define FILTER_HAS_XSTATS    TCA_HAS_XSTATS	/**< Has ext statistics */
#define FILTER_HAS_PRIO      0x200		/**< Has prio */
#define FILTER_HAS_PROTOCOL  0x400		/**< Has protocol */

/** @} */

/**
 * Filter
 * @ingroup filter
 */
struct rtnl_filter
{
	NL_TCA_GENERIC(f)
	uint32_t             prio;
	uint32_t             protocol;

	/* private */
	int                  with_stats;
};

struct rtnl_filter_ops
{
	char kind[32];
	int (*dump[NL_DUMP_MAX+1])(struct nl_cache *, struct rtnl_filter *,
				    FILE *, struct nl_dump_params *, int);
	int (*msg_parser)(struct rtnl_filter *);
	void (*free_data)(struct rtnl_filter *);
	struct rtnl_filter_ops *next;
};

#define FILTER_CACHE_IFINDEX(C) ((C)->c_iarg1)
#define FILTER_CACHE_PARENT(C) ((C)->c_iarg2)

extern struct nl_cache_ops rtnl_filter_ops;

/**
 * Initialize a filter cache structure
 * @ingroup filter
 * @code
 * struct nl_cache cache = RTNL_INIT_FILTER_CACHE();
 * @endcode
 */
#define RTNL_INIT_FILTER_CACHE() {             \
    .c_type = RTNL_FILTER,                     \
    .c_type_size = sizeof(struct rtnl_filter), \
    .c_ops = &rtnl_filter_ops,                 \
}

/**
 * Initialize a filter structure
 * @ingroup filter
 * @code
 * struct rtnl_filter f = RTNL_INIT_FILTER();
 * @endcode
 */
#define RTNL_INIT_FILTER() {                     \
    .ce_type = RTNL_FILTER,                      \
    .ce_size = sizeof(struct rtnl_filter),       \
}

extern void rtnl_filter_register(struct rtnl_filter_ops *);
extern void rtnl_filter_unregister(struct rtnl_filter_ops *);

extern int  rtnl_filter_add(struct nl_handle *, struct rtnl_filter *,
                            struct nl_msg *);

extern struct nl_cache *rtnl_filter_build_cache(struct nl_handle *,
						int, uint32_t);
extern void rtnl_filter_dump(struct rtnl_filter *, FILE *,
			     struct nl_dump_params *);

extern struct nl_msg *rtnl_filter_build_add_request(struct rtnl_filter *,
                                                    struct nl_msg *opts);
extern struct nl_msg *rtnl_filter_build_change_request(struct rtnl_filter *,
                                                       struct nl_msg *opts);
extern struct nl_msg *rtnl_filter_build_delete_request(struct rtnl_filter *);
extern int  rtnl_filter_delete(struct nl_handle *, struct rtnl_filter *);

extern void rtnl_filter_set_ifindex(struct rtnl_filter *, int);
extern int  rtnl_filter_set_ifindex_name(struct rtnl_filter *,
                                         struct nl_cache *, const char *);
extern void rtnl_filter_set_handle(struct rtnl_filter *, uint32_t);
extern void rtnl_filter_set_parent(struct rtnl_filter *, uint32_t);
extern void rtnl_filter_set_kind(struct rtnl_filter *, const char *);
extern void rtnl_filter_set_prio(struct rtnl_filter *, uint32_t);
extern void rtnl_filter_set_protocol(struct rtnl_filter *, uint32_t);

#endif
