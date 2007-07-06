/*
 * lib/route/class.c            Queueing Classes
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup tc
 * @defgroup class Classes
 * @{
 */

#include <netlink-local.h>
#include <netlink-tc.h>
#include <netlink/netlink.h>
#include <netlink/route/tc.h>
#include <netlink/route/class.h>
#include <netlink/route/class-modules.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/classifier.h>
#include <netlink/utils.h>

/** @cond SKIP */
static struct nl_cache_ops rtnl_class_ops;
/** @endcond */

static struct rtnl_class_ops *class_ops_list;

static struct rtnl_class_ops *class_lookup_ops(const char *kind)
{
	struct rtnl_class_ops *ops;

	for (ops = class_ops_list; ops; ops = ops->co_next)
		if (!strcmp(kind, ops->co_kind))
			return ops;

	return NULL;
}

static inline struct rtnl_class_ops *class_ops(struct rtnl_class *class)
{
	if (!class->c_ops)
		class->c_ops = class_lookup_ops(class->c_kind);

	return class->c_ops;
}

/**
 * @name Class Module API
 * @{
 */

/**
 * Register a class module
 * @arg ops		class module operations
 */
int rtnl_class_register(struct rtnl_class_ops *ops)
{
	struct rtnl_class_ops *o, **op;

	if (!ops->co_kind[0])
		BUG();

	for (op = &class_ops_list; (o = *op) != NULL; op = &o->co_next)
		if (!strcasecmp(ops->co_kind, o->co_kind))
			return nl_errno(EEXIST);

	ops->co_next = NULL;
	*op = ops;

	return 0;
}

/**
 * Unregister a class module
 * @arg ops		class module operations
 */
int rtnl_class_unregister(struct rtnl_class_ops *ops)
{
	struct rtnl_class_ops *o, **op;

	for (op = &class_ops_list; (o = *op) != NULL; op = &o->co_next)
		if (!strcasecmp(ops->co_kind, o->co_kind))
			break;

	if (!o)
		return nl_errno(ENOENT);

	*op = ops->co_next;

	return 0;
}

/** @} */

static int class_msg_parser(struct sockaddr_nl *who, struct nlmsghdr *n,
			    void *arg)
{
	int err;
	struct nl_parser_param *pp = arg;
	struct rtnl_class *class;
	struct rtnl_class_ops *ops;

	class = rtnl_class_alloc();
	if (!class) {
		err = nl_errno(ENOMEM);
		goto errout;
	}
	class->ce_msgtype = n->nlmsg_type;

	err = tca_msg_parser(n, (struct rtnl_tca *) class);
	if (err < 0)
		goto errout_free;

	ops = class_ops(class);
	if (ops && ops->co_msg_parser) {
		err = ops->co_msg_parser(class);
		if (err < 0)
			goto errout_free;
	}

	err = pp->pp_cb((struct nl_object *) class, pp);
	if (err < 0)
		goto errout_free;

	return P_ACCEPT;

errout_free:
	rtnl_class_put(class);
errout:
	return err;
}

static int class_request_update(struct nl_cache *cache,
				struct nl_handle *handle)
{
	struct tcmsg tchdr = {
		.tcm_family = AF_UNSPEC,
		.tcm_ifindex = cache->c_iarg1,
	};

	return nl_send_simple(handle, RTM_GETTCLASS, NLM_F_DUMP, &tchdr,
			      sizeof(tchdr));
}

static void class_free_data(struct nl_object *obj)
{
	struct rtnl_class *class = (struct rtnl_class *) obj;
	struct rtnl_class_ops *ops;
	
	tca_free_data((struct rtnl_tca *) class);

	ops = class_ops(class);
	if (ops && ops->co_free_data)
		ops->co_free_data(class);
}

static int class_dump_brief(struct nl_object *obj, struct nl_dump_params *p)
{
	struct rtnl_class *class = (struct rtnl_class *) obj;
	struct rtnl_class_ops *ops;

	int line = tca_dump_brief((struct rtnl_tca *) class, "class", p, 0);

	ops = class_ops(class);
	if (ops && ops->co_dump[NL_DUMP_BRIEF])
		line = ops->co_dump[NL_DUMP_BRIEF](class, p, line);
	dp_dump(p, "\n");

	return line;
}

static int class_dump_full(struct nl_object *obj, struct nl_dump_params *p)
{
	struct rtnl_class *class = (struct rtnl_class *) obj;
	struct rtnl_class_ops *ops;
	int line;

	line = class_dump_brief(obj, p);
	line = tca_dump_full((struct rtnl_tca *) class, p, line);
	
	if (class->c_info) {
		char buf[32];
		dp_dump(p, "child-qdisc %s ",
			rtnl_tc_handle2str(class->c_info, buf, sizeof(buf)));
	}

	ops = class_ops(class);
	if (ops && ops->co_dump[NL_DUMP_FULL])
		line = ops->co_dump[NL_DUMP_FULL](class, p, line);
	else if (!class->c_info)
		dp_dump(p, "noop (no leaf qdisc)");

	dp_dump(p, "\n");

	return line;
}

static int class_dump_stats(struct nl_object *obj, struct nl_dump_params *p)
{
	struct rtnl_class *class = (struct rtnl_class *) obj;
	struct rtnl_class_ops *ops;
	int line;

	line = class_dump_full(obj, p);
	line = tca_dump_stats((struct rtnl_tca *) class, p, line);
	dp_dump(p, "\n");

	ops = class_ops(class);
	if (ops && ops->co_dump[NL_DUMP_STATS])
		line = ops->co_dump[NL_DUMP_STATS](class, p, line);

	return line;
}

static int class_filter(struct nl_object *obj, struct nl_object *filter)
{
	return tca_filter((struct rtnl_tca *) obj, (struct rtnl_tca *) filter);
}

/**
 * @name Class Addition/Modification
 * @{
 */

static struct nl_msg *class_build(struct rtnl_class *class, int type, int flags)
{
	struct rtnl_class_ops *ops;
	struct nl_msg *msg;
	int err;

	msg = tca_build_msg((struct rtnl_tca *) class, type, flags);
	if (!msg)
		goto errout;

	ops = class_ops(class);
	if (ops && ops->co_get_opts) {
		struct nl_msg *opts;
		
		opts = ops->co_get_opts(class);
		if (opts) {
			err = nla_put_nested(msg, TCA_OPTIONS, opts);
			nlmsg_free(opts);
			if (err < 0)
				goto errout;
		}
	}

	return msg;
errout:
	nlmsg_free(msg);
	return NULL;
}

/**
 * Build a netlink message to add a new class
 * @arg class		class to add 
 * @arg flags		additional netlink message flags
 *
 * Builds a new netlink message requesting an addition of a class.
 * The netlink message header isn't fully equipped with all relevant
 * fields and must be sent out via nl_send_auto_complete() or
 * supplemented as needed. 
 *
 * Common message flags
 *   - NLM_F_REPLACE - replace possibly existing classes
 *
 * @return New netlink message
 */
struct nl_msg *rtnl_class_build_add_request(struct rtnl_class *class, int flags)
{
	return class_build(class, RTM_NEWTCLASS, NLM_F_CREATE | flags);
}

/**
 * Add a new class
 * @arg handle		netlink handle
 * @arg class		class to delete
 * @arg flags		additional netlink message flags
 *
 * Builds a netlink message by calling rtnl_qdisc_build_add_request(),
 * sends the request to the kernel and waits for the next ACK to be
 * received and thus blocks until the request has been processed.
 *
 * Common message flags
 *   - NLM_F_REPLACE - replace possibly existing classes
 *
 * @return 0 on success or a negative error code
 */
int rtnl_class_add(struct nl_handle *handle, struct rtnl_class *class,
		   int flags)
{
	struct nl_msg *msg;
	int err;

	msg = rtnl_class_build_add_request(class, flags);
	if (!msg)
		return nl_errno(ENOMEM);

	err = nl_send_auto_complete(handle, msg);
	if (err < 0)
		return err;

	nlmsg_free(msg);
	return nl_wait_for_ack(handle);
}

/** @} */

/**
 * @name General
 * @{
 */

/**
 * Allocate a new class object
 * @return New class object
 */
struct rtnl_class *rtnl_class_alloc(void)
{
	return (struct rtnl_class *) nl_object_alloc_from_ops(&rtnl_class_ops);
}

/**
 * Give back reference on rclass object.
 * @arg class		Class object to be given back.
 *
 * Decrements the reference counter and frees the object if the
 * last reference has been released.
 */
void rtnl_class_put(struct rtnl_class *class)
{
	nl_object_put((struct nl_object *) class);
}

/**
 * Free class object.
 * @arg class		Class object to be freed.
 *
 * @note Always use rtnl_class_put() unless you're absolutely sure
 *       that no other user may have a reference on this object.
 */
void rtnl_class_free(struct rtnl_class *class)
{
	nl_object_free((struct nl_object *) class);
}

/**
 * Build a class cache including all classes attached to the specified interface
 * @arg handle		netlink handle
 * @arg ifindex		interface index of the link the classes are
 *                      attached to.
 *
 * Allocates a new cache, initializes it properly and updates it to
 * include all classes attached to the specified interface.
 *
 * @note The caller is responsible for destroying and freeing the
 *       cache after using it.
 * @return The cache or NULL if an error has occured.
 */
struct nl_cache * rtnl_class_alloc_cache(struct nl_handle *handle, int ifindex)
{
	struct nl_cache * cache;
	
	cache = nl_cache_alloc_from_ops(&rtnl_class_ops);
	if (!cache)
		return NULL;

	cache->c_iarg1 = ifindex;
	
	if (nl_cache_update(handle, cache) < 0) {
		nl_cache_free(cache);
		return NULL;
	}

	return cache;
}

/** @} */

/**
 * @name Leaf Qdisc Access
 * @{
 */

/**
 * Determine if the class has a leaf qdisc attached
 * @arg class		class to check
 */
int rtnl_class_has_leaf_qdisc(struct rtnl_class *class)
{
	return !!class->c_info;
}

/**
 * Lookup the leaf qdisc of a class
 * @arg class		the parent class
 * @arg cache		a qdisc cache including at laest all qdiscs of the
 *                      interface the specified class is attached to
 * @return The qdisc from the cache or NULL if the class has no leaf qdisc
 */
struct rtnl_qdisc *rtnl_class_leaf_qdisc(struct rtnl_class *class,
					 struct nl_cache *cache)
{
	struct rtnl_qdisc *leaf;

	if (!rtnl_class_has_leaf_qdisc(class))
		return NULL;

	leaf = rtnl_qdisc_get_by_parent(cache, class->c_ifindex,
					class->c_handle);
	if (!leaf || leaf->q_handle != class->c_info)
		return NULL;

	return leaf;
}

/** @} */

/**
 * @name Iterators
 * @{
 */

/**
 * Call a callback for each child class of a class
 * @arg class		the parent class
 * @arg cache		a class cache including all classes of the interface
 *                      the specified class is attached to
 * @arg cb              callback function
 * @arg arg             argument to be passed to callback function
 */
void rtnl_class_foreach_child(struct rtnl_class *class, struct nl_cache *cache,
			      void (*cb)(struct nl_object *, void *), void *arg)
{
	struct rtnl_class *filter;
	
	filter = rtnl_class_alloc();
	if (!filter)
		return;

	rtnl_class_set_parent(filter, class->c_handle);
	rtnl_class_set_ifindex(filter, class->c_ifindex);
	rtnl_class_set_kind(filter, class->c_kind);

	nl_cache_foreach_filter(cache, (struct nl_object *) filter, cb, arg);
	rtnl_class_put(filter);
}

/**
 * Call a callback for each classifier attached to the class
 * @arg class		the parent class
 * @arg cache		a filter cache including at least all the filters
 *                      attached to the specified class
 * @arg cb              callback function
 * @arg arg             argument to be passed to callback function
 */
void rtnl_class_foreach_cls(struct rtnl_class *class, struct nl_cache *cache,
			    void (*cb)(struct nl_object *, void *), void *arg)
{
	struct rtnl_cls *filter;

	filter = rtnl_cls_alloc();
	if (!filter)
		return;

	rtnl_cls_set_ifindex(filter, class->c_ifindex);
	rtnl_cls_set_parent(filter, class->c_parent);

	nl_cache_foreach_filter(cache, (struct nl_object *) filter, cb, arg);
	rtnl_cls_put(filter);
}

/** @} */

/**
 * @name Attribute Modifications
 * @{
 */

/**
 * Set the interface index of a class to the specified value
 * @arg class		class to be changed
 * @arg ifindex		new interface index
 */
void rtnl_class_set_ifindex(struct rtnl_class *class, int ifindex)
{
	tca_set_ifindex((struct rtnl_tca *) class, ifindex);
}

/**
 * Get the interface index of a class
 * @arg class		class handle
 * @return Interface index or RTNL_LINK_NOT_FOUND if not set
 */
int rtnl_class_get_ifindex(struct rtnl_class *class)
{
	return tca_get_ifindex((struct rtnl_tca *) class);
}


/**
 * Set the handle of a class to the specified value
 * @arg class		class to be changed
 * @arg handle		new handle
 */
void rtnl_class_set_handle(struct rtnl_class *class, uint32_t handle)
{
	tca_set_handle((struct rtnl_tca *) class, handle);
}

/**
 * Get the handle of a class
 * @arg class		class handle
 * @return Handle or 0 if not set
 */
uint32_t rtnl_class_get_handle(struct rtnl_class *class)
{
	return tca_get_handle((struct rtnl_tca *) class);
}

/**
 * Set the parent handle of a class to the specified value
 * @arg class		class to be changed
 * @arg parent		new parent handle
 */
void rtnl_class_set_parent(struct rtnl_class *class, uint32_t parent)
{
	tca_set_parent((struct rtnl_tca *) class, parent);
}

/**
 * Get the parent handle of a class
 * @arg class		class handle
 * @return Parent handle or 0 if not set
 */
uint32_t rtnl_class_get_parent(struct rtnl_class *class)
{
	return tca_get_parent((struct rtnl_tca *) class);
}

/**
 * Set the kind of a class to the specified value
 * @arg class		class to be changed
 * @arg name		new kind name
 */
void rtnl_class_set_kind(struct rtnl_class *class, const char *name)
{
	tca_set_kind((struct rtnl_tca *) class, name);
}

/**
 * Get the kind of a class
 * @arg class		class handle
 * @return Kind or NULL if not set
 */
char *rtnl_class_get_kind(struct rtnl_class *class)
{
	return tca_get_kind((struct rtnl_tca *) class);
}

/**
 * Get the statistic specified by the id
 * @arg class		class handle
 * @arg id		statistic id
 * @return The current counter of the specified statistic
 */
uint64_t rtnl_class_get_stat(struct rtnl_class *class,
			     enum rtnl_tc_stats_id id)
{
	return tca_get_stat((struct rtnl_tca *) class, id);
}

/** @} */

static struct nl_cache_ops rtnl_class_ops = {
	.co_name		= "route/class",
	.co_size		= sizeof(struct rtnl_class),
	.co_hdrsize		= sizeof(struct tcmsg),
	.co_msgtypes		= {
					{ RTM_NEWTCLASS, "new" },
					{ RTM_DELTCLASS, "del" },
					{ RTM_GETTCLASS, "get" },
					{ -1, NULL },
				  },
	.co_protocol		= NETLINK_ROUTE,
	.co_request_update	= &class_request_update,
	.co_msg_parser		= &class_msg_parser,
	.co_free_data         	= &class_free_data,
	.co_dump[NL_DUMP_BRIEF]	= &class_dump_brief,
	.co_dump[NL_DUMP_FULL]	= &class_dump_full,
	.co_dump[NL_DUMP_STATS]	= &class_dump_stats,
	.co_filter		= &class_filter,
};

static void __init class_init(void)
{
	nl_cache_mngt_register(&rtnl_class_ops);
}

static void __exit class_exit(void)
{
	nl_cache_mngt_unregister(&rtnl_class_ops);
}

/** @} */
