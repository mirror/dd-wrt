/*
 * netlink/route/qdisc.h	Queueing Disciplines
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

#ifndef NETLINK_QDISC_H_
#define NETLINK_QDISC_H_

#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/route/tc.h>

/**
 * @name Available Attributes Flags
 * @ingroup qdisc
 * @anchor qdisc_avail_attrs
 * Flags used in rtnl_qdisc::q_mask to indicate which attributes are
 * present in a qdisc.
 *
 * The corresponding flag is automatically set if a attribute is set
 * via a \c rtnl_qdisc_set_* function or if the qdisc was created by
 * the internal message parser, e.g. if it comes from a cache.
 *
 * @par Example: Check if a qdisc has a parent and options attribute set
 * @code
 * if (qdisc->q_mask & (QDISC_HAS_PARENT | QDISC_HAS_OPTS))
 *         // action goes here
 * @endcode
 * @{
 */

#define QDISC_HAS_HANDLE    TCA_HAS_HANDLE	/**< Has handle */
#define QDISC_HAS_PARENT    TCA_HAS_PARENT	/**< Has parent */
#define QDISC_HAS_IFINDEX   TCA_HAS_IFINDEX	/**< Has interface index */
#define QDISC_HAS_KIND      TCA_HAS_KIND	/**< Has kind */
#define QDISC_HAS_FAMILY    TCA_HAS_FAMILY	/**< Has family */
#define QDISC_HAS_INFO      TCA_HAS_INFO	/**< Has info */
#define QDISC_HAS_OPTS      TCA_HAS_OPTS	/**< Has options */
#define QDISC_HAS_STATS     TCA_HAS_STATS	/**< Has statistics */
#define QDISC_HAS_XSTATS    TCA_HAS_XSTATS	/**< Has ext statistics */

/** @} */

/**
 * Queueing discipline
 * @ingroup qdisc
 */
struct rtnl_qdisc
{
	NL_TCA_GENERIC(q)
};

/**
 * Qdisc operations
 * @ingroup qdisc
 */
struct rtnl_qdisc_ops
{
	char qo_kind[32];
	int  (*qo_dump[NL_DUMP_MAX+1])(struct nl_cache *, struct rtnl_qdisc *,
				       FILE *, struct nl_dump_params *, int);
	struct nl_msg *(*qo_get_opts)(struct rtnl_qdisc *);
	int  (*qo_msg_parser)(struct rtnl_qdisc *);
	void (*qo_free_data)(struct rtnl_qdisc *);
	struct rtnl_qdisc_ops *qo_next;
};

#define QDISC_CACHE_IFINDEX(C) ((C)->c_iarg1)

extern struct nl_cache_ops rtnl_qdisc_ops;

/**
 * Initialize a qdisc cache structure
 * @ingroup qdisc
 * @code
 * struct nl_cache cache = RTNL_INIT_QDISC_CACHE();
 * @endcode
 */
#define RTNL_INIT_QDISC_CACHE() {              \
    .c_type = RTNL_QDISC,                      \
    .c_type_size = sizeof(struct rtnl_qdisc),  \
    .c_ops = &rtnl_qdisc_ops,                  \
}


/**
 * Initialize a qdisc structure
 * @ingroup qdisc
 * @code
 * struct rtnl_qdisc q = RTNL_INIT_QDISC();
 * @endcode
 */
#define RTNL_INIT_QDISC() {                     \
    .ce_type = RTNL_QDISC,                      \
    .ce_size = sizeof(struct rtnl_qdisc),       \
}

extern void rtnl_qdisc_register(struct rtnl_qdisc_ops *);
extern void rtnl_qdisc_unregister(struct rtnl_qdisc_ops *);

extern struct nl_cache * rtnl_qdisc_build_cache(struct nl_handle *);
extern void rtnl_qdisc_dump(struct rtnl_qdisc *, FILE *,
			    struct nl_dump_params *);

extern struct rtnl_qdisc * rtnl_qdisc_get(struct nl_cache *, int, uint32_t);
extern struct rtnl_qdisc * rtnl_qdisc_get_by_parent(struct nl_cache *,
						    int, uint32_t);
extern struct rtnl_qdisc * rtnl_qdisc_get_root(struct nl_cache *, int);
extern struct rtnl_qdisc * rtnl_qdisc_get_ingress(struct nl_cache *, int);

extern void rtnl_qdisc_foreach_child(struct rtnl_qdisc *, struct nl_cache *,
				     void (*)(struct nl_common *, void *),
				     void *);

extern void rtnl_qdisc_foreach_child_nocache(struct nl_handle *,
					     struct rtnl_qdisc *,
					     void (*)(struct nl_common *, void *),
					     void *);
extern void rtnl_qdisc_foreach_filter(struct rtnl_qdisc *, struct nl_cache *,
				      void (*)(struct nl_common *, void *),
				      void *);
extern void rtnl_qdisc_foreach_filter_nocache(struct nl_handle *,
					      struct rtnl_qdisc *,
					      void (*)(struct nl_common *, void *),
					      void *);

extern int rtnl_qdisc_add(struct nl_handle *, struct rtnl_qdisc *, int);

extern struct nl_msg *	rtnl_qdisc_build_delete_request(struct rtnl_qdisc *);
extern int		rtnl_qdisc_delete(struct nl_handle *, struct rtnl_qdisc *);
extern struct nl_msg *	rtnl_qdisc_build_delete_root_request(int);
extern int		rtnl_qdisc_delete_root(struct nl_handle *, int);
extern struct nl_msg *	rtnl_qdisc_build_delete_ingress_request(int);
extern int		rtnl_qdisc_delete_ingress(struct nl_handle *, int);

extern struct nl_msg *rtnl_qdisc_get_opts(struct rtnl_qdisc *);

extern void rtnl_qdisc_set_ifindex(struct rtnl_qdisc *, int);
extern int  rtnl_qdisc_set_ifindex_name(struct rtnl_qdisc *,
                                        struct nl_cache *,
                                        const char *dev);
extern void rtnl_qdisc_set_handle(struct rtnl_qdisc *, uint32_t);
extern void rtnl_qdisc_set_parent(struct rtnl_qdisc *, uint32_t);
extern void rtnl_qdisc_set_kind(struct rtnl_qdisc *, const char *);

#endif
