/*
 * lib/route/qdisc.c            Queueing Disciplines
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
 * @defgroup qdisc Queueing Disciplines
 *
 * @par Qdisc Handles
 * In general, qdiscs are identified by the major part of a traffic control
 * handle (the upper 16 bits). A few special values exist though:
 *  - \c TC_H_ROOT: root qdisc (directly attached to the device)
 *  - \c TC_H_INGRESS: ingress qdisc (directly attached to the device)
 *  - \c TC_H_UNSPEC: unspecified qdisc (no reference)
 *
 * @par 1) Adding a Qdisc
 * @code
 * // Allocate a new empty qdisc to be filled out
 * struct rtnl_qdisc *qdisc = rtnl_qdisc_alloc();
 *
 * // ... specify the kind of the Qdisc
 * rtnl_qdisc_set_kind(qdisc, "pfifo");
 *
 * // Specify the device the qdisc should be attached to
 * rtnl_qdisc_set_ifindex(qdisc, ifindex);
 *
 * // ... specify the parent qdisc
 * rtnl_qdisc_set_parent(qdisc, TC_H_ROOT);
 *
 * // Specifying the handle is not required but makes reidentifying easier
 * // and may help to avoid adding a qdisc twice.
 * rtnl_qdisc_set_handle(qdisc, 0x000A0000);
 *
 * // Now on to specify the qdisc specific options, see the relevant qdisc
 * // modules for documentation, in this example we set the upper limit of
 * // the packet fifo qdisc to 64
 * rtnl_qdisc_fifo_set_limit(qdisc, 64);
 *
 * rtnl_qdisc_add(handle, qdisc, NLM_R_REPLACE);
 *
 * // Free up the memory
 * rtnl_qdisc_put(qdisc);
 * @endcode
 *
 * @par 2) Deleting a Qdisc
 * @code
 * // Allocate a new empty qdisc to be filled out with the parameters
 * // specifying the qdisc to be deleted. Alternatively a fully equiped
 * // Qdisc object from a cache can be used.
 * struct rtnl_qdisc *qdisc = rtnl_qdisc_alloc();
 *
 * // The interface index of the device the qdisc is on and the parent handle
 * // are the least required fields to be filled out.
 * // Note: Specify TC_H_ROOT or TC_H_INGRESS as parent handle to delete the
 * //       root respectively root ingress qdisc.
 * rtnl_qdisc_set_ifindex(qdisc, ifindex);
 * rtnl_qdisc_set_parent(qdisc, parent_handle);
 *
 * // If required for identification, the handle can be specified as well.
 * rtnl_qdisc_set_handle(qdisc, qdisc_handle);
 *
 * // Not required but maybe helpful as sanity check, the kind of the qdisc
 * // can be specified to avoid mistakes.
 * rtnl_qdisc_set_kind(qdisc, "pfifo");
 *
 * // Finally delete the qdisc with rtnl_qdisc_delete(), alternatively
 * // rtnl_qdisc_build_delete_request() can be invoked to generate an
 * // appropritate netlink message to send out.
 * rtnl_qdisc_delete(handle, qdisc);
 *
 * // Free up the memory
 * rtnl_qdisc_put(qdisc);
 * @endcode
 *
 * @{
 */

#include <netlink-local.h>
#include <netlink-tc.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/route/link.h>
#include <netlink/route/tc.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/class.h>
#include <netlink/route/classifier.h>
#include <netlink/route/qdisc-modules.h>

/** @cond SKIP */
static struct nl_cache_ops rtnl_qdisc_ops;
/** @endcond */

static struct rtnl_qdisc_ops *qdisc_ops_list;

static struct rtnl_qdisc_ops *qdisc_lookup_ops(const char *kind)
{
	struct rtnl_qdisc_ops *ops;

	for (ops = qdisc_ops_list; ops; ops = ops->qo_next)
		if (!strcmp(kind, ops->qo_kind))
			return ops;

	return NULL;
}

static inline struct rtnl_qdisc_ops *qdisc_ops(struct rtnl_qdisc *qdisc)
{
	if (!qdisc->q_ops)
		qdisc->q_ops = qdisc_lookup_ops(qdisc->q_kind);

	return qdisc->q_ops;
}

/**
 * @name QDisc Module API
 * @{
 */

/**
 * Register a qdisc module
 * @arg ops		qdisc module operations
 */
int rtnl_qdisc_register(struct rtnl_qdisc_ops *ops)
{
	struct rtnl_qdisc_ops *o, **op;

	if (!ops->qo_kind[0])
		BUG();

	for (op = &qdisc_ops_list; (o = *op) != NULL; op = &o->qo_next)
		if (!strcasecmp(ops->qo_kind, o->qo_kind))
			return nl_errno(EEXIST);

	ops->qo_next = NULL;
	*op = ops;

	return 0;
}

/**
 * Unregister a qdisc module
 * @arg ops		qdisc module operations
 */
int rtnl_qdisc_unregister(struct rtnl_qdisc_ops *ops)
{
	struct rtnl_qdisc_ops *o, **op;

	for (op = &qdisc_ops_list; (o = *op) != NULL; op = &o->qo_next)
		if (!strcasecmp(ops->qo_kind, o->qo_kind))
			break;

	if (!o)
		return nl_errno(ENOENT);

	*op = ops->qo_next;

	return 0;
}

/** @} */

static int qdisc_msg_parser(struct sockaddr_nl *who, struct nlmsghdr *n,
			    void *arg)
{
	int err = -ENOMEM;
	struct nl_parser_param *pp = arg;
	struct rtnl_qdisc *qdisc;
	struct rtnl_qdisc_ops *ops;

	qdisc = rtnl_qdisc_alloc();
	if (!qdisc) {
		err = nl_errno(ENOMEM);
		goto errout;
	}

	qdisc->ce_msgtype = n->nlmsg_type;

	err = tca_msg_parser(n, (struct rtnl_tca *) qdisc);
	if (err < 0)
		goto errout_free;

	ops = qdisc_ops(qdisc);
	if (ops && ops->qo_msg_parser) {
		err = ops->qo_msg_parser(qdisc);
		if (err < 0)
			goto errout_free;
	}

	err = pp->pp_cb((struct nl_object *) qdisc, pp);
	if (err < 0)
		goto errout_free;

	return P_ACCEPT;

errout_free:
	rtnl_qdisc_put(qdisc);
errout:
	return err;
}

static int qdisc_request_update(struct nl_cache *c, struct nl_handle *h)
{
	struct tcmsg tchdr = {
		.tcm_family = AF_UNSPEC,
		.tcm_ifindex = c->c_iarg1,
	};

	return nl_send_simple(h, RTM_GETQDISC, NLM_F_DUMP, &tchdr,
			      sizeof(tchdr));
}

static void qdisc_free_data(struct nl_object *obj)
{
	struct rtnl_qdisc *qdisc = (struct rtnl_qdisc *) obj;
	struct rtnl_qdisc_ops *ops;

	tca_free_data((struct rtnl_tca *) qdisc);

	ops = qdisc_ops(qdisc);
	if (ops && ops->qo_free_data)
		ops->qo_free_data(qdisc);
}

static int qdisc_dump_brief(struct nl_object *obj, struct nl_dump_params *p)
{
	struct rtnl_qdisc *qdisc = (struct rtnl_qdisc *) obj;
	struct rtnl_qdisc_ops *ops;

	int line = tca_dump_brief((struct rtnl_tca *) qdisc, "qdisc", p, 0);

	ops = qdisc_ops(qdisc);
	if (ops && ops->qo_dump[NL_DUMP_BRIEF])
		line = ops->qo_dump[NL_DUMP_BRIEF](qdisc, p, line);

	dp_dump(p, "\n");

	return line;
}

static int qdisc_dump_full(struct nl_object *arg, struct nl_dump_params *p)
{
	struct rtnl_qdisc *qdisc = (struct rtnl_qdisc *) arg;
	struct rtnl_qdisc_ops *ops;

	int line = qdisc_dump_brief(arg, p);

	line = tca_dump_full((struct rtnl_tca *) qdisc, p, line);
	dp_dump(p, "refcnt %u ", qdisc->q_info);

	ops = qdisc_ops(qdisc);
	if (ops && ops->qo_dump[NL_DUMP_FULL])
		line = ops->qo_dump[NL_DUMP_FULL](qdisc, p, line);

	dp_dump(p, "\n");
	return line;
}

static int qdisc_dump_stats(struct nl_object *arg, struct nl_dump_params *p)
{
	struct rtnl_qdisc *qdisc = (struct rtnl_qdisc *) arg;
	struct rtnl_qdisc_ops *ops;

	int line = qdisc_dump_full(arg, p);
	line = tca_dump_stats((struct rtnl_tca *) qdisc, p, line );
	dp_dump(p, "\n");

	ops = qdisc_ops(qdisc);
	if (ops && ops->qo_dump[NL_DUMP_STATS])
		line = ops->qo_dump[NL_DUMP_STATS](qdisc, p, line);

	return line;
}

static int qdisc_filter(struct nl_object *obj, struct nl_object *filter)
{
	return tca_filter((struct rtnl_tca *) obj, (struct rtnl_tca *) filter);
}

/**
 * @name QDisc Addition
 * @{
 */

static struct nl_msg *qdisc_build(struct rtnl_qdisc *qdisc, int type, int flags)
{
	struct rtnl_qdisc_ops *ops;
	struct nl_msg *msg;
	int err;

	msg = tca_build_msg((struct rtnl_tca *) qdisc, type, flags);
	if (!msg)
		goto errout;

	ops = qdisc_ops(qdisc);
	if (ops && ops->qo_get_opts) {
		struct nl_msg *opts;
		
		opts = ops->qo_get_opts(qdisc);
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
 * Build a netlink message to add a new qdisc
 * @arg qdisc		qdisc to add 
 * @arg flags		additional netlink message flags
 *
 * Builds a new netlink message requesting an addition of a qdisc.
 * The netlink message header isn't fully equipped with all relevant
 * fields and must be sent out via nl_send_auto_complete() or
 * supplemented as needed. 
 *
 * Common message flags used:
 *  - NLM_F_REPLACE - replace a potential existing qdisc
 *
 * @return New netlink message
 */
struct nl_msg *rtnl_qdisc_build_add_request(struct rtnl_qdisc *qdisc,
					    int flags)
{
	struct nl_msg *msg;

	msg = qdisc_build(qdisc, RTM_NEWQDISC, NLM_F_CREATE | flags);
	if (!msg)
		nl_errno(ENOMEM);

	return msg;
}

/**
 * Add a new qdisc
 * @arg handle		netlink handle
 * @arg qdisc		qdisc to delete
 * @arg flags		additional netlink message flags
 *
 * Builds a netlink message by calling rtnl_qdisc_build_add_request(),
 * sends the request to the kernel and waits for the ACK to be
 * received and thus blocks until the request has been processed.
 *
 * Common message flags used:
 *  - NLM_F_REPLACE - replace a potential existing qdisc
 *
 * @return 0 on success or a negative error code
 */
int rtnl_qdisc_add(struct nl_handle *handle, struct rtnl_qdisc *qdisc,
		   int flags)
{
	struct nl_msg *msg;
	int err;

	msg = rtnl_qdisc_build_add_request(qdisc, flags);
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
 * @name QDisc Modification
 * @{
 */

/**
 * Build a netlink message to change attributes of a existing qdisc
 * @arg qdisc		qdisc to change
 * @arg new		new qdisc attributes
 *
 * Builds a new netlink message requesting an change of qdisc
 * attributes. The netlink message header isn't fully equipped
 * with all relevant fields and must be sent out via
 * nl_send_auto_complete() or supplemented as needed. 
 *
 * @return New netlink message
 */
struct nl_msg *rtnl_qdisc_build_change_request(struct rtnl_qdisc *qdisc,
					       struct rtnl_qdisc *new)
{
	return qdisc_build(qdisc, RTM_NEWQDISC, NLM_F_REPLACE);
}

/**
 * Change attributes of a qdisc
 * @arg handle		netlink handle
 * @arg qdisc		qdisc to change
 * @arg new		new qdisc attributes
 *
 * Builds a netlink message by calling rtnl_qdisc_build_change_request(),
 * sends the request to the kernel and waits for the ACK to be
 * received and thus blocks until the request has been processed.
 *
 * @return 0 on success or a negative error code
 */
int rtnl_qdisc_change(struct nl_handle *handle, struct rtnl_qdisc *qdisc,
		      struct rtnl_qdisc *new)
{
	struct nl_msg *msg;
	int err;

	msg = rtnl_qdisc_build_change_request(qdisc, new);
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
 * @name QDisc Deletion
 * @{
 */

/**
 * Build a netlink request message to delete a qdisc
 * @arg qdisc		qdisc to delete
 *
 * Builds a new netlink message requesting a deletion of a qdisc.
 * The netlink message header isn't fully equipped with all relevant
 * fields and must thus be sent out via nl_send_auto_complete()
 * or supplemented as needed.
 *
 * @return New netlink message
 */
struct nl_msg *rtnl_qdisc_build_delete_request(struct rtnl_qdisc *qdisc)
{
	struct nl_msg *msg;
	struct tcmsg tchdr;
	int required = TCA_ATTR_IFINDEX | TCA_ATTR_PARENT;

	if ((qdisc->q_mask & required) != required)
		BUG();

	msg = nlmsg_build_simple(RTM_DELQDISC, 0);
	if (!msg)
		return NULL;

	tchdr.tcm_family = AF_UNSPEC,
	tchdr.tcm_handle = qdisc->q_handle,
	tchdr.tcm_parent = qdisc->q_parent,
	tchdr.tcm_ifindex = qdisc->q_ifindex,
	nlmsg_append(msg, &tchdr, sizeof(tchdr), 1);

	return msg;
}

/**
 * Delete a qdisc
 * @arg handle		netlink handle
 * @arg qdisc		qdisc to delete
 *
 * Builds a netlink message by calling rtnl_qdisc_build_delete_request(),
 * sends the request to the kernel and waits for the ACK to be
 * received and thus blocks until the request has been processed.
 *
 * @return 0 on success or a negative error code
 */
int rtnl_qdisc_delete(struct nl_handle *handle, struct rtnl_qdisc *qdisc)
{
	struct nl_msg *msg;
	int err;

	msg = rtnl_qdisc_build_delete_request(qdisc);
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
 * Allocate a new qdisc object
 * @return New qdisc object
 */
struct rtnl_qdisc *rtnl_qdisc_alloc(void)
{
	return (struct rtnl_qdisc *) nl_object_alloc_from_ops(&rtnl_qdisc_ops);
}

/**
 * Give back reference on rqdisc object.
 * @arg qdisc		Qdisc object to be given back.
 *
 * Decrements the reference counter and frees the object if the
 * last reference has been released.
 */
void rtnl_qdisc_put(struct rtnl_qdisc *qdisc)
{
	nl_object_put((struct nl_object *) qdisc);
}
/**
 * Free qdisc object.
 * @arg qdisc		Qdisc object to be freed.
 *
 * @note Always use rtnl_qdisc_put() unless you're absolutely sure
 *       that no other user may have a reference on this object.
 */
void rtnl_qdisc_free(struct rtnl_qdisc *qdisc)
{
	nl_object_free((struct nl_object *) qdisc);
}

/** @} */

/**
 * @name Qdisc Cache Management
 * @{
 */

/**
 * Build a qdisc cache including all qdiscs currently configured in
 * the kernel
 * @arg handle		netlink handle
 *
 * Allocates a new cache, initializes it properly and updates it to
 * include all qdiscs currently configured in the kernel.
 *
 * @note The caller is responsible for destroying and freeing the
 *       cache after using it.
 * @return The cache or NULL if an error has occured.
 */
struct nl_cache * rtnl_qdisc_alloc_cache(struct nl_handle *handle)
{
	struct nl_cache * cache;
	
	cache = nl_cache_alloc_from_ops(&rtnl_qdisc_ops);
	if (cache == NULL)
		return NULL;

	if (nl_cache_update(handle, cache) < 0) {
		nl_cache_free(cache);
		return NULL;
	}

	return cache;
}

/**
 * Look up qdisc by its parent in the provided cache
 * @arg cache		qdisc cache
 * @arg ifindex		interface the qdisc is attached to
 * @arg parent		parent handle
 * @return pointer to qdisc inside the cache or NULL if no match was found.
 */
struct rtnl_qdisc * rtnl_qdisc_get_by_parent(struct nl_cache *cache,
					     int ifindex, uint32_t parent)
{
	struct rtnl_qdisc *q;

	if (cache->c_ops != &rtnl_qdisc_ops)
		return NULL;

	nl_list_for_each_entry(q, &cache->c_items, ce_list) {
		if (q->q_parent == parent && q->q_ifindex == ifindex) {
			nl_object_get((struct nl_object *) q);
			return q;
		}
	}

	return NULL;
}

/**
 * Look up qdisc by its handle in the provided cache
 * @arg cache		qdisc cache
 * @arg ifindex		interface the qdisc is attached to
 * @arg handle		qdisc handle
 * @return pointer to qdisc inside the cache or NULL if no match was found.
 */
struct rtnl_qdisc * rtnl_qdisc_get(struct nl_cache *cache,
				   int ifindex, uint32_t handle)
{
	struct rtnl_qdisc *q;

	if (cache->c_ops != &rtnl_qdisc_ops)
		return NULL;

	nl_list_for_each_entry(q, &cache->c_items, ce_list) {
		if (q->q_handle == handle && q->q_ifindex == ifindex) {
			nl_object_get((struct nl_object *) q);
			return q;
		}
	}

	return NULL;
}

/** @} */

/**
 * @name Qdisc Specific Options
 * @{
 */

/**
 * Return qdisc specific options for use in TCA_OPTIONS
 * @arg qdisc		qdisc carrying the optiosn
 * 
 * @return new headerless netlink message carrying the options as payload
 */
struct nl_msg *rtnl_qdisc_get_opts(struct rtnl_qdisc *qdisc)
{
	struct rtnl_qdisc_ops *ops;

	ops = qdisc_ops(qdisc);
	if (ops && ops->qo_get_opts)
		return ops->qo_get_opts(qdisc);

	return NULL;
}

/** @} */

/**
 * @name Iterators
 * @{
 */

/**
 * Call a callback for each child class of a qdisc
 * @arg qdisc		the parent qdisc
 * @arg cache		a class cache including all classes of the interface
 *                      the specified qdisc is attached to
 * @arg cb              callback function
 * @arg arg             argument to be passed to callback function
 */
void rtnl_qdisc_foreach_child(struct rtnl_qdisc *qdisc, struct nl_cache *cache,
			      void (*cb)(struct nl_object *, void *), void *arg)
{
	struct rtnl_class *filter;
	
	filter = rtnl_class_alloc();
	if (!filter)
		return;

	rtnl_class_set_parent(filter, qdisc->q_handle);
	rtnl_class_set_ifindex(filter, qdisc->q_ifindex);
	rtnl_class_set_kind(filter, qdisc->q_kind);

	nl_cache_foreach_filter(cache, (struct nl_object *) filter, cb, arg);

	rtnl_class_put(filter);
}

/**
 * Call a callback for each filter attached to the qdisc
 * @arg qdisc		the parent qdisc
 * @arg cache		a filter cache including at least all the filters
 *                      attached to the specified qdisc
 * @arg cb              callback function
 * @arg arg             argument to be passed to callback function
 */
void rtnl_qdisc_foreach_cls(struct rtnl_qdisc *qdisc, struct nl_cache *cache,
			    void (*cb)(struct nl_object *, void *), void *arg)
{
	struct rtnl_cls *filter;

	filter = rtnl_cls_alloc();
	if (!filter)
		return;

	rtnl_cls_set_ifindex(filter, qdisc->q_ifindex);
	rtnl_cls_set_parent(filter, qdisc->q_parent);

	nl_cache_foreach_filter(cache, (struct nl_object *) filter, cb, arg);
	rtnl_cls_put(filter);
}

/** @} */

/**
 * @name Attribute Modification
 * @{
 */

/**
 * Set the interface index of a qdisc to the specified value
 * @arg qdisc		qdisc to be changed
 * @arg ifindex		new interface index
 */
void rtnl_qdisc_set_ifindex(struct rtnl_qdisc *qdisc, int ifindex)
{
	tca_set_ifindex((struct rtnl_tca *) qdisc, ifindex);
}

/**
 * Get the interface index of a qdisc
 * @arg qdisc		qdisc handle
 * @return Interface index or RTNL_LINK_NOT_FOUND if not set
 */
int rtnl_qdisc_get_ifindex(struct rtnl_qdisc *qdisc)
{
	return tca_get_ifindex((struct rtnl_tca *) qdisc);
}

/**
 * Set the handle of a qdisc to the specified value
 * @arg qdisc		qdisc to be changed
 * @arg handle		new handle
 */
void rtnl_qdisc_set_handle(struct rtnl_qdisc *qdisc, uint32_t handle)
{
	tca_set_handle((struct rtnl_tca *) qdisc, handle);
}

/**
 * Get the handle of a qdisc
 * @arg qdisc		qdisc handle
 * @return Handle or 0 if not set
 */
uint32_t rtnl_qdisc_get_handle(struct rtnl_qdisc *qdisc)
{
	return tca_get_handle((struct rtnl_tca *) qdisc);
}

/**
 * Set the parent handle of a qdisc to the specified value
 * @arg qdisc		qdisc to be changed
 * @arg parent		new parent handle
 */
void rtnl_qdisc_set_parent(struct rtnl_qdisc *qdisc, uint32_t parent)
{
	tca_set_parent((struct rtnl_tca *) qdisc, parent);
}

/**
 * Get the parent handle of a qdisc
 * @arg qdisc		qdisc handle
 * @return Parent handle or 0 if not set
 */
uint32_t rtnl_qdisc_get_parent(struct rtnl_qdisc *qdisc)
{
	return tca_get_parent((struct rtnl_tca *) qdisc);
}

/**
 * Set the kind of a qdisc to the specified value
 * @arg qdisc		qdisc to be changed
 * @arg name		new kind name
 */
void rtnl_qdisc_set_kind(struct rtnl_qdisc *qdisc, const char *name)
{
	tca_set_kind((struct rtnl_tca *) qdisc, name);
	qdisc->q_ops = qdisc_lookup_ops(name);
}

/**
 * Get the kind of a qdisc
 * @arg qdisc		qdisc handle
 * @return Kind or NULL if not set
 */
char *rtnl_qdisc_get_kind(struct rtnl_qdisc *qdisc)
{
	return tca_get_kind((struct rtnl_tca *) qdisc);
}

/**
 * Get the statistic specified by the id
 * @arg qdisc		qdisc handle
 * @arg id		statistic id
 * @return The current counter of the specified statistic
 */
uint64_t rtnl_qdisc_get_stat(struct rtnl_qdisc *qdisc,
			     enum rtnl_tc_stats_id id)
{
	return tca_get_stat((struct rtnl_tca *) qdisc, id);
}

/** @} */

static struct nl_cache_ops rtnl_qdisc_ops = {
	.co_name		= "route/qdisc",
	.co_size		= sizeof(struct rtnl_qdisc),
	.co_hdrsize		= sizeof(struct tcmsg),
	.co_msgtypes		= {
					{ RTM_NEWQDISC, "new" },
					{ RTM_DELQDISC, "delete" },
					{ RTM_GETQDISC, "get" },
					{ -1, NULL },
				  },
	.co_protocol		= NETLINK_ROUTE,
	.co_request_update	= qdisc_request_update,
	.co_msg_parser		= qdisc_msg_parser,
	.co_free_data		= qdisc_free_data,
	.co_dump[NL_DUMP_BRIEF]	= qdisc_dump_brief,
	.co_dump[NL_DUMP_FULL]	= qdisc_dump_full,
	.co_dump[NL_DUMP_STATS]	= qdisc_dump_stats,
	.co_filter		= qdisc_filter,
};

static void __init qdisc_init(void)
{
	nl_cache_mngt_register(&rtnl_qdisc_ops);
}

static void __exit qdisc_exit(void)
{
	nl_cache_mngt_unregister(&rtnl_qdisc_ops);
}

/** @} */
