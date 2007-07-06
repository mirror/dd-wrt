/*
 * lib/route/classifier.c       Classifier
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
 * @defgroup cls Classifiers
 *
 * @par Classifier Identification
 * - protocol
 * - priority
 * - parent
 * - interface
 * - kind
 * - handle
 * 
 * @{
 */

#include <netlink-local.h>
#include <netlink-tc.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/route/tc.h>
#include <netlink/route/classifier.h>
#include <netlink/route/classifier-modules.h>
#include <netlink/route/link.h>

/** @cond SKIP */
#define CLS_ATTR_PRIO		(TCA_ATTR_MAX << 1)
#define CLS_ATTR_PROTOCOL	(TCA_ATTR_MAX << 2)

static struct nl_cache_ops rtnl_cls_ops;
/** @endcond */

static struct rtnl_cls_ops *cls_ops_list;

static struct rtnl_cls_ops * cls_lookup_ops(char *kind)
{
	struct rtnl_cls_ops *ops;

	for (ops = cls_ops_list; ops; ops = ops->co_next)
		if (!strcmp(kind, ops->co_kind))
			return ops;

	return NULL;
}

static inline struct rtnl_cls_ops *cls_ops(struct rtnl_cls *cls)
{
	if (!cls->c_ops)
		cls->c_ops = cls_lookup_ops(cls->c_kind);

	return cls->c_ops;
}

/**
 * @name Classifier Module API
 * @{
 */

/**
 * Register a classifier module
 * @arg ops		classifier module operations
 */
int rtnl_cls_register(struct rtnl_cls_ops *ops)
{
	struct rtnl_cls_ops *o, **op;

	if (!ops->co_kind)
		BUG();

	for (op = &cls_ops_list; (o = *op) != NULL; op = &o->co_next)
		if (!strcasecmp(ops->co_kind, o->co_kind))
			return nl_errno(EEXIST);

	ops->co_next = NULL;
	*op = ops;

	return 0;
}

/**
 * Unregister a classifier module
 * @arg ops		classifier module operations
 */
int rtnl_cls_unregister(struct rtnl_cls_ops *ops)
{
	struct rtnl_cls_ops *o, **op;

	for (op = &cls_ops_list; (o = *op) != NULL; op = &o->co_next)
		if (!strcasecmp(ops->co_kind, o->co_kind))
			break;

	if (!o)
		return nl_errno(ENOENT);

	*op = ops->co_next;

	return 0;
}

/** @} */

static int cls_msg_parser(struct sockaddr_nl *who, struct nlmsghdr *nlh,
			  void *arg)
{
	int err;
	struct nl_parser_param *pp = arg;
	struct rtnl_cls *cls;
	struct rtnl_cls_ops *ops;

	cls = rtnl_cls_alloc();
	if (!cls) {
		err = nl_errno(ENOMEM);
		goto errout;
	}
	cls->ce_msgtype = nlh->nlmsg_type;

	err = tca_msg_parser(nlh, (struct rtnl_tca *) cls);
	if (err < 0)
		goto errout_free;

	cls->c_prio = TC_H_MAJ(cls->c_info) >> 16;
	cls->c_protocol = ntohs(TC_H_MIN(cls->c_info));

	ops = cls_ops(cls);
	if (ops && ops->co_msg_parser) {
		err = ops->co_msg_parser(cls);
		if (err < 0)
			goto errout_free;
	}

	err = pp->pp_cb((struct nl_object *) cls, pp);
	if (err < 0)
		goto errout_free;

	return P_ACCEPT;

errout_free:
	rtnl_cls_put(cls);
errout:
	return err;
}

static int cls_request_update(struct nl_cache *cache, struct nl_handle *handle)
{
	struct tcmsg tchdr = {
		.tcm_family = AF_UNSPEC,
		.tcm_ifindex = cache->c_iarg1,
		.tcm_parent = cache->c_iarg2,
	};

	return nl_send_simple(handle, RTM_GETTFILTER, NLM_F_DUMP, &tchdr,
			      sizeof(tchdr));
}


static void cls_free_data(struct nl_object *obj)
{
	struct rtnl_cls *cls = (struct rtnl_cls *) obj;
	struct rtnl_cls_ops *ops;
	
	tca_free_data((struct rtnl_tca *) cls);

	ops = cls_ops(cls);
	if (ops && ops->co_free_data)
		ops->co_free_data(cls);
}

static int cls_dump_brief(struct nl_object *obj, struct nl_dump_params *p)
{
	char buf[32];
	struct rtnl_cls *cls = (struct rtnl_cls *) obj;
	struct rtnl_cls_ops *ops;
	int line;

	line = tca_dump_brief((struct rtnl_tca *) cls, "cls", p, 0);

	dp_dump(p, " prio %u protocol %s", cls->c_prio,
		nl_ether_proto2str(cls->c_protocol, buf, sizeof(buf)));

	ops = cls_ops(cls);
	if (ops && ops->co_dump[NL_DUMP_BRIEF])
		line = ops->co_dump[NL_DUMP_BRIEF](cls, p, line);
	dp_dump(p, "\n");

	return line;
}

static int cls_dump_full(struct nl_object *obj, struct nl_dump_params *p)
{
	struct rtnl_cls *cls = (struct rtnl_cls *) obj;
	struct rtnl_cls_ops *ops;
	int line;

	line = cls_dump_brief(obj, p);
	line = tca_dump_full((struct rtnl_tca *) cls, p, line);

	ops = cls_ops(cls);
	if (ops && ops->co_dump[NL_DUMP_FULL])
		line = ops->co_dump[NL_DUMP_FULL](cls, p, line);
	else
		dp_dump(p, "no options\n");

	return line;
}

static int cls_dump_stats(struct nl_object *obj, struct nl_dump_params *p)
{
	struct rtnl_cls *cls = (struct rtnl_cls *) obj;
	struct rtnl_cls_ops *ops;
	int line;

	line = cls_dump_full(obj, p);
	line = tca_dump_stats((struct rtnl_tca *) cls, p, line);
	dp_dump(p, "\n");

	ops = cls_ops(cls);
	if (ops && ops->co_dump[NL_DUMP_STATS])
		line = ops->co_dump[NL_DUMP_STATS](cls, p, line);

	return line;
}

static int cls_filter(struct nl_object *obj, struct nl_object *filter)
{
	return tca_filter((struct rtnl_tca *) obj, (struct rtnl_tca *) filter);
}

static struct nl_msg *cls_build(struct rtnl_cls *cls, int type, int flags)
{
	struct nl_msg *msg;
	struct rtnl_cls_ops *ops;
	int err, prio, proto;
	struct tcmsg *tchdr;

	msg = tca_build_msg((struct rtnl_tca *) cls, type, flags);
	if (!msg)
		goto errout;

	tchdr = nlmsg_data(nlmsg_hdr(msg));
	prio = cls->c_mask & CLS_ATTR_PRIO ? cls->c_prio : 0;
	proto = cls->c_mask & CLS_ATTR_PROTOCOL ? cls->c_protocol : ETH_P_ALL;
	tchdr->tcm_info = TC_H_MAKE(prio << 16, htons(proto)),

	ops = cls_ops(cls);
	if (ops && ops->co_get_opts) {
		struct nl_msg *opts;
		
		opts = ops->co_get_opts(cls);
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
 * @name Classifier Addition/Modification/Deletion
 * @{
 */

/**
 * Build a netlink message to add a new classifier
 * @arg cls		classifier to add
 * @arg flags		additional netlink message flags
 *
 * Builds a new netlink message requesting an addition of a classifier
 * The netlink message header isn't fully equipped with all relevant
 * fields and must be sent out via nl_send_auto_complete() or
 * supplemented as needed. \a classifier must contain the attributes of
 * the new classifier set via \c rtnl_cls_set_* functions. \a opts
 * may point to the clsasifier specific options.
 *
 * @return New netlink message
 */
struct nl_msg * rtnl_cls_build_add_request(struct rtnl_cls *cls, int flags)
{
	return cls_build(cls, RTM_NEWTFILTER, NLM_F_CREATE | flags);
}

/**
 * Add a new classifier
 * @arg handle		netlink handle
 * @arg cls 		classifier to add
 * @arg flags		additional netlink message flags
 *
 * Builds a netlink message by calling rtnl_cls_build_add_request(),
 * sends the request to the kernel and waits for the next ACK to be
 * received and thus blocks until the request has been processed.
 *
 * @return 0 on sucess or a negative error if an error occured.
 */
int rtnl_cls_add(struct nl_handle *handle, struct rtnl_cls *cls, int flags)
{
	int err;
	struct nl_msg *msg;
	
	msg = rtnl_cls_build_add_request(cls, flags);
	if (!msg)
		return nl_errno(ENOMEM);
	
	err = nl_send_auto_complete(handle, msg);
	if (err < 0)
		return err;

	nlmsg_free(msg);
	return nl_wait_for_ack(handle);
}

/**
 * Build a netlink message to change classifier attributes
 * @arg cls		classifier to change
 * @arg flags		additional netlink message flags
 *
 * Builds a new netlink message requesting a change of a neigh
 * attributes. The netlink message header isn't fully equipped with
 * all relevant fields and must thus be sent out via nl_send_auto_complete()
 * or supplemented as needed.
 *
 * @return The netlink message
 */
struct nl_msg *rtnl_cls_build_change_request(struct rtnl_cls *cls, int flags)
{
	return cls_build(cls, RTM_NEWTFILTER, NLM_F_REPLACE | flags);
}

/**
 * Change a classifier
 * @arg handle		netlink handle
 * @arg cls		classifier to change
 * @arg flags		additional netlink message flags
 *
 * Builds a netlink message by calling rtnl_cls_build_change_request(),
 * sends the request to the kernel and waits for the next ACK to be
 * received and thus blocks until the request has been processed.
 *
 * @return 0 on sucess or a negative error if an error occured.
 */
int rtnl_cls_change(struct nl_handle *handle, struct rtnl_cls *cls,
		    int flags)
{
	int err;
	struct nl_msg *msg;
	
	msg = rtnl_cls_build_change_request(cls, flags);
	if (!msg)
		return nl_errno(ENOMEM);
	
	err = nl_send_auto_complete(handle, msg);
	if (err < 0)
		return err;

	nlmsg_free(msg);
	return nl_wait_for_ack(handle);
}

/**
 * Build a netlink request message to delete a classifier
 * @arg cls		classifier to delete
 * @arg flags		additional netlink message flags
 *
 * Builds a new netlink message requesting a deletion of a classifier.
 * The netlink message header isn't fully equipped with all relevant
 * fields and must thus be sent out via nl_send_auto_complete()
 * or supplemented as needed.
 *
 * @return New netlink message
 */
struct nl_msg *rtnl_cls_build_delete_request(struct rtnl_cls *cls, int flags)
{
	return cls_build(cls, RTM_DELTFILTER, flags);
}


/**
 * Delete a classifier
 * @arg handle		netlink handle
 * @arg cls		classifier to delete
 * @arg flags		additional netlink message flags
 *
 * Builds a netlink message by calling rtnl_cls_build_delete_request(),
 * sends the request to the kernel and waits for the next ACK to be
 * received and thus blocks until the request has been processed.
 *
 * @return 0 on sucess or a negative error if an error occured.
 */
int rtnl_cls_delete(struct nl_handle *handle, struct rtnl_cls *cls, int flags)
{
	int err;
	struct nl_msg *msg;
	
	msg = rtnl_cls_build_delete_request(cls, flags);
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
 * @name General API
 * @{
 */

/**
 * Allocate a new classifier object
 * @return New classifier object
 */
struct rtnl_cls *rtnl_cls_alloc(void)
{
	return (struct rtnl_cls *) nl_object_alloc_from_ops(&rtnl_cls_ops);
}

/**
 * Give back reference on classifier object.
 * @arg cls		Classifier object to be given back.
 *
 * Decrements the reference counter and frees the object if the
 * last reference has been released.
 */
void rtnl_cls_put(struct rtnl_cls *cls)
{
	nl_object_put((struct nl_object *) cls);
}

/**
 * Free classifier object.
 * @arg cls		Classifier object to be freed.
 *
 * @note Always use rtnl_cls_put() unless you're absolutely sure
 *       that no other user may have a reference on this object.
 */
void rtnl_cls_free(struct rtnl_cls *cls)
{
	nl_object_free((struct nl_object *) cls);
}

/**
 * Build a classifier cache including all classifiers attached to the
 * specified class/qdisc on eht specified interface.
 * @arg handle		netlink handle
 * @arg ifindex		interface index of the link the classes are
 *                      attached to.
 * @arg parent          parent qdisc/class
 *
 * Allocates a new cache, initializes it properly and updates it to
 * include all classes attached to the specified interface.
 *
 * @note The caller is responsible for destroying and freeing the
 *       cache after using it.
 * @return The cache or NULL if an error has occured.
 */
struct nl_cache *rtnl_cls_alloc_cache(struct nl_handle *handle,
				      int ifindex, uint32_t parent)
{
	struct nl_cache * cache;
	
	cache = nl_cache_alloc_from_ops(&rtnl_cls_ops);
	if (cache == NULL)
		return NULL;

	cache->c_iarg1 = ifindex;
	cache->c_iarg2 = parent;
	
	if (nl_cache_update(handle, cache) < 0) {
		nl_cache_free(cache);
		return NULL;
	}

	return cache;
}

void rtnl_cls_set_ifindex(struct rtnl_cls *f, int ifindex)
{
	tca_set_ifindex((struct rtnl_tca *) f, ifindex);
}

void rtnl_cls_set_handle(struct rtnl_cls *f, uint32_t handle)
{
	tca_set_handle((struct rtnl_tca *) f, handle);
}

void rtnl_cls_set_parent(struct rtnl_cls *f, uint32_t parent)
{
	tca_set_parent((struct rtnl_tca *) f, parent);
}

void rtnl_cls_set_kind(struct rtnl_cls *f, const char *kind)
{
	tca_set_kind((struct rtnl_tca *) f, kind);
}

/**
 * Set prioroty of a classifier
 * @arg cls		classifier to change
 * @arg prio		new priority
 */
void rtnl_cls_set_prio(struct rtnl_cls *cls, int prio)
{
	cls->c_prio = prio;
	cls->c_mask |= CLS_ATTR_PRIO;
}

/**
 * Get priority of a classifier
 * @arg cls		classifier
 */
int rtnl_cls_get_prio(struct rtnl_cls *cls)
{
	if (cls->c_mask & CLS_ATTR_PRIO)
		return cls->c_prio;
	else
		return 0;
}

/**
 * Set protocol of a classifier
 * @arg cls		classifier to change
 * @arg protocol	protocol identifier (ETH_P_xxx) in host byte-order
 */
void rtnl_cls_set_protocol(struct rtnl_cls *cls, int protocol)
{
	cls->c_protocol = protocol;
	cls->c_mask |= CLS_ATTR_PROTOCOL;
}

/**
 * Get protocol of a classifier
 * @arg cls		classifier
 */
int rtnl_cls_get_protocol(struct rtnl_cls *cls)
{
	if (cls->c_mask & CLS_ATTR_PROTOCOL)
		return cls->c_protocol;
	else
		return 0;
}

/** @} */

static struct nl_cache_ops rtnl_cls_ops = {
	.co_name		= "route/cls",
	.co_size		= sizeof(struct rtnl_cls),
	.co_hdrsize		= sizeof(struct tcmsg),
	.co_msgtypes		= {
					{ RTM_NEWTFILTER, "new" },
					{ RTM_DELTFILTER, "delete" },
					{ RTM_GETTFILTER, "get" },
					{ -1, NULL },
				  },
	.co_protocol		= NETLINK_ROUTE,
	.co_request_update	= cls_request_update,
	.co_filter		= cls_filter,
	.co_free_data		= cls_free_data,
	.co_msg_parser		= cls_msg_parser,
	.co_dump[NL_DUMP_BRIEF]	= cls_dump_brief,
	.co_dump[NL_DUMP_FULL]	= cls_dump_full,
	.co_dump[NL_DUMP_STATS]	= cls_dump_stats,
};

static void __init cls_init(void)
{
	nl_cache_mngt_register(&rtnl_cls_ops);
}

static void __exit cls_exit(void)
{
	nl_cache_mngt_unregister(&rtnl_cls_ops);
}

/** @} */
