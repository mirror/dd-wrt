/*
 * class.h             libnl class module
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

#ifndef NETLINK_CLASS_H_
#define NETLINK_CLASS_H_

#include <netlink/netlink.h>
#include <netlink/route/tc.h>
#include <netlink/route/qdisc.h>

/**
 * @name Available Attributes Flags
 * @ingroup class
 * @anchor class_avail_attrs
 * Flags used in rtnl_class::c_mask to indicate which attributes are
 * present in a class.
 *
 * The corresponding flag is automatically set if a attribute is set
 * via a \c rtnl_class_set_* function or if the class was created by
 * the internal message parser, e.g. if it comes from a cache.
 *
 * @par Example: Check if a class has a parent and options attribute set
 * @code
 * if (class->c_mask & (CLASS_HAS_PARENT | CLASSHAS_OPTS))
 *         // action goes here
 * @endcode
 * @{
 */

#define CLASS_HAS_HANDLE    TCA_HAS_HANDLE	/**< Has handle */
#define CLASS_HAS_PARENT    TCA_HAS_PARENT	/**< Has parent */
#define CLASS_HAS_IFINDEX   TCA_HAS_IFINDEX	/**< Has interface index */
#define CLASS_HAS_KIND      TCA_HAS_KIND	/**< Has kind */
#define CLASS_HAS_FAMILY    TCA_HAS_FAMILY	/**< Has family */
#define CLASS_HAS_INFO      TCA_HAS_INFO	/**< Has info */
#define CLASS_HAS_OPTS      TCA_HAS_OPTS	/**< Has options */
#define CLASS_HAS_STATS     TCA_HAS_STATS	/**< Has statistics */
#define CLASS_HAS_XSTATS    TCA_HAS_XSTATS	/**< Has ext statistics */

/** @} */

/**
 * Class
 * @ingroup class
 */
struct rtnl_class
{
	NL_TCA_GENERIC(c)
};

/**
 * Class module operations
 * @ingroup class
 */
struct rtnl_class_ops
{
	char co_kind[32];
	int (*co_dump[NL_DUMP_MAX+1])(struct nl_cache *, struct rtnl_class *,
				      FILE *, struct nl_dump_params *, int);
	struct nl_msg *(*co_get_opts)(struct rtnl_class *);
	int  (*co_msg_parser)(struct rtnl_class *);
	void (*co_free_data)(struct rtnl_class *);
	struct rtnl_class_ops *co_next;
};

#define CLASS_CACHE_IFINDEX(C) ((C)->c_iarg1)

extern struct nl_cache_ops rtnl_class_ops;

/**
 * Initialize a class cache structure
 * @code
 * struct nl_cache cache = RTNL_INIT_CLASS_CACHE();
 * @endcode
 */
#define RTNL_INIT_CLASS_CACHE() {              \
    .c_type = RTNL_CLASS,                      \
    .c_type_size = sizeof(struct rtnl_class),  \
    .c_ops = &rtnl_class_ops,                  \
}

/**
 * Initialize a class structure
 * @code
 * struct rtnl_class c = RTNL_INIT_CLASS();
 * @endcode
 */
#define RTNL_INIT_CLASS() {                     \
    .ce_type = RTNL_CLASS,                      \
    .ce_size = sizeof(struct rtnl_class),       \
}

extern void rtnl_class_register(struct rtnl_class_ops *);
extern void rtnl_class_unregister(struct rtnl_class_ops *);

extern struct nl_cache * rtnl_class_build_cache(struct nl_handle *, int);
extern int rtnl_class_has_qdisc_attached(struct rtnl_class *);
extern struct rtnl_qdisc *rtnl_class_leaf_qdisc(struct rtnl_class *, struct nl_cache *);
extern void rtnl_class_dump(struct rtnl_class *, FILE *,
			    struct nl_dump_params *);
extern void rtnl_class_foreach_child(struct rtnl_class *, struct nl_cache *,
				     void (*cb)(struct nl_common *, void *), void *);
extern void rtnl_class_foreach_child_nocache(struct nl_handle *,
					     struct rtnl_class *,
					     void (*)(struct nl_common *, void *),
					     void *);
extern void rtnl_class_foreach_filter(struct rtnl_class *, struct nl_cache *,
				      void (*)(struct nl_common *, void *),
				      void *);
extern void rtnl_class_foreach_filter_nocache(struct nl_handle *,
					      struct rtnl_class *,
					      void (*)(struct nl_common *, void *),
					      void *);


extern void rtnl_class_set_ifindex(struct rtnl_class *, int);
extern int  rtnl_class_set_ifindex_name(struct rtnl_class *,
                                        struct nl_cache *,
                                        const char *dev);
extern void rtnl_class_set_handle(struct rtnl_class *, uint32_t);
extern void rtnl_class_set_parent(struct rtnl_class *, uint32_t);
extern void rtnl_class_set_kind(struct rtnl_class *, const char *);

#endif
