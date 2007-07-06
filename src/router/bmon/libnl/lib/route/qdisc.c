/*
 * route/qdisc.c            Queueing Disciplines
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
 * @ingroup tc
 * @defgroup qdisc Queueing Disciplines
 *
 * @par Handles
 * In general, qdiscs are identified by the major part of a traffic control
 * handle (the upper 16 bits). A few special values exist though:
 *  - \c TC_H_ROOT: root qdisc (directly attached to the device)
 *  - \c TC_H_INGRESS: ingress qdisc (directly attached to the device)
 *  - \c TC_H_UNSPEC: 
 * 
 *
 * @par Qdisc Identification
 * A qdisc is uniquely identified by the attributes listed below, whenever
 * you refer to an existing qdisc all of the attributes must be set.
 * Qdiscs from caches automatically have all required attributes set.
 * Because of the complexity we differ between the operations add, replace,
 * deletion, and get.
 * \par Qdisc Identification for Deletions
 *  - interface index (rtnl_qdisc_set_ifindex())
 *  - parent (rtnl_qdisc_set_parent())
 *  - Optional:
 *    - kind (rtnl_qdisc_set_kind())
 *    - handle (rtnl_qdisc_set_handle())
 * @par Qdisc Identification for Additions
 *  - interface index (rtnl_qdisc_set_ifindex())
 *  - parent (rtnl_qdisc_set_parent())
 *
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/helpers.h>
#include <netlink/route/rtattr.h>
#include <netlink/route/link.h>
#include <netlink/route/class.h>
#include <netlink/route/filter.h>
#include <netlink/route/tc.h>
#include <netlink/route/qdisc.h>

static struct rtnl_qdisc_ops *qdisc_ops;

static struct rtnl_qdisc_ops * qdisc_lookup_ops(char *kind)
{
	struct rtnl_qdisc_ops *o = NULL;

	for (o = qdisc_ops; o; o = o->qo_next)
		if (!strcmp(kind, o->qo_kind))
			break;

	return o;
}

/**
 * @name QDisc Module API
 * @{
 */

/**
 * Register a qdisc module
 * @arg ops		qdisc module operations
 */
void rtnl_qdisc_register(struct rtnl_qdisc_ops *ops)
{
	struct rtnl_qdisc_ops *o, **op;

	for (op = &qdisc_ops; (o = *op) != NULL; op = &o->qo_next)
		if (!strcasecmp(ops->qo_kind, o->qo_kind))
			return;

	ops->qo_next = NULL;
	*op = ops;
}

/**
 * Unregister a qdisc module
 * @arg ops		qdisc module operations
 */
void rtnl_qdisc_unregister(struct rtnl_qdisc_ops *ops)
{
	struct rtnl_qdisc_ops *o, **op;

	for (op = &qdisc_ops; (o = *op) != NULL; op = &o->qo_next)
		if (!strcasecmp(ops->qo_kind, o->qo_kind))
			break;

	if (NULL == o)
		return;

	*op = ops->qo_next;
}

/** @} */
static int qdisc_msg_parser(struct sockaddr_nl *who, struct nlmsghdr *n,
			    void *arg)
{
	int err;
	struct nl_parser_param *pp = arg;
	struct rtnl_qdisc qdisc = RTNL_INIT_QDISC();
	struct rtnl_qdisc_ops *ops;

	if (n->nlmsg_type != RTM_NEWQDISC)
		return 0; /* ignore */

	if ((err = tca_msg_parser(n, (struct rtnl_tca *) &qdisc)) < 0) {
		if (-EINTR == err)
			return 0;
		else
			return err;
	}

	ops = qdisc_lookup_ops(qdisc.q_kind);

	if (ops && ops->qo_msg_parser)
		if ((err = ops->qo_msg_parser(&qdisc)) < 0)
			return err;

	err = pp->pp_cb((struct nl_common *) &qdisc, pp);
	if (err < 0)
		return err;

	return P_ACCEPT;
}

static int qdisc_request_update(struct nl_cache *c, struct nl_handle *h)
{
	struct tcmsg tm = {
		.tcm_family = AF_UNSPEC,
		.tcm_ifindex = QDISC_CACHE_IFINDEX(c),
	};

	return nl_request_with_data(h, RTM_GETQDISC, NLM_F_DUMP,
		(unsigned char *) &tm, sizeof(tm));
}


static void qdisc_free_data(struct nl_common *a)
{
	struct rtnl_qdisc *q = (struct rtnl_qdisc *) a;
	struct rtnl_qdisc_ops *o = qdisc_lookup_ops(q->q_kind);

	tca_free_data((struct rtnl_tca *) a);

	if (o && o->qo_free_data)
		o->qo_free_data(q);
}

static int qdisc_dump_brief(struct nl_cache *cache, struct nl_common *arg,
			    FILE *fd, struct nl_dump_params *params)
{
	struct rtnl_qdisc *q = (struct rtnl_qdisc *) arg;
	struct rtnl_qdisc_ops *ops = qdisc_lookup_ops(q->q_kind);

	int line = tca_dump_brief(cache, (struct rtnl_tca *) arg, "qdisc",
				  fd, params, 0);

	if (ops && ops->qo_dump[NL_DUMP_BRIEF])
		line = ops->qo_dump[NL_DUMP_BRIEF](cache, q, fd, params, line);

	fprintf(fd, "\n");

	return line;
}

static int qdisc_dump_full(struct nl_cache *cache, struct nl_common *arg,
			   FILE *fd, struct nl_dump_params *params)
{
	struct rtnl_qdisc *qdisc = (struct rtnl_qdisc *) arg;
	struct rtnl_qdisc_ops *ops = qdisc_lookup_ops(qdisc->q_kind);

	int line = qdisc_dump_brief(cache, arg, fd, params);

	dp_new_line(fd, params, line++);
	fprintf(fd, "    refcnt %d ", qdisc->q_info);
	line = tca_dump_full(cache, (struct rtnl_tca *) qdisc, fd,
			     params, line);

	if (ops && ops->qo_dump[NL_DUMP_FULL])
		line = ops->qo_dump[NL_DUMP_FULL](cache, qdisc, fd,
						  params, line);

	fprintf(fd, "\n");
	return line;
}

static int qdisc_dump_with_stats(struct nl_cache *cache,
				 struct nl_common *arg, FILE *fd,
				 struct nl_dump_params *params)
{
	struct rtnl_qdisc *qdisc = (struct rtnl_qdisc *) arg;
	struct rtnl_qdisc_ops *ops = qdisc_lookup_ops(qdisc->q_kind);

	int line = qdisc_dump_full(cache, arg, fd, params);
	line = tca_dump_with_stats(cache, (struct rtnl_tca *) qdisc,
				   fd, params, line );
	fprintf(fd, "\n");

	if (ops && ops->qo_dump[NL_DUMP_STATS])
		line = ops->qo_dump[NL_DUMP_STATS](cache, qdisc,
						   fd, params, line);

	return line;
}

static int qdisc_filter(struct nl_common *o, struct nl_common *f)
{
	if (o->ce_type != RTNL_QDISC || f->ce_type != RTNL_QDISC)
		return 0;

	return tca_filter((struct rtnl_tca *) o, (struct rtnl_tca *) f);
}

/**
 * @name QDisc Addition/Modification
 * @{
 */

static struct nl_msg *qdisc_build(struct rtnl_qdisc *qdisc, int type, int flags)
{
	struct rtnl_qdisc_ops *ops = qdisc_lookup_ops(qdisc->q_kind);
	struct nl_msg *m;
	struct nlmsghdr n = {
		.nlmsg_type = type,
		.nlmsg_flags = flags,
	};
	struct tcmsg t = {
		.tcm_family = AF_UNSPEC,
		.tcm_ifindex = qdisc->q_ifindex,
		.tcm_handle = qdisc->q_handle,
		.tcm_parent = qdisc->q_parent,
	};
	m = nl_msg_build(&n);
	nl_msg_append_raw(m, &t, sizeof(t));
	nl_msg_append_tlv(m, TCA_KIND, qdisc->q_kind,
			  strlen(qdisc->q_kind) + 1);

	if (ops->qo_get_opts) {
		struct nl_msg *opts = ops->qo_get_opts(qdisc);
		if (opts) {
			nl_msg_append_nested(m, TCA_OPTIONS, opts);
			nl_msg_free(opts);
		}
	}

	return m;
}

/**
 * Build a netlink message to add a new qdisc
 * @arg qdisc		qdisc to add 
 * @arg replace		if non-zero replace possible existing qdisc
 *
 * Builds a new netlink message requesting an addition of a qdisc.
 * The netlink message header isn't fully equipped with all relevant
 * fields and must be sent out via nl_send_auto_complete() or
 * supplemented as needed. 
 *
 * @return New netlink message
 */
struct nl_msg * rtnl_qdisc_build_add_request(struct rtnl_qdisc *qdisc,
					     int replace)
{
	return qdisc_build(qdisc, RTM_NEWQDISC, NLM_F_CREATE |
			   (replace ? NLM_F_REPLACE : NLM_F_EXCL));
}

/**
 * Add a new qdisc
 * @arg handle		netlink handle
 * @arg qdisc		qdisc to delete
 * @arg replace		if non-zero replace possible existing qdisc
 *
 * Builds a netlink message by calling rtnl_qdisc_build_add_request(),
 * sends the request to the kernel and waits for the next ACK to be
 * received and thus blocks until the request has been processed.
 *
 * @return 0 on success or a negative error code
 */
int rtnl_qdisc_add(struct nl_handle *handle, struct rtnl_qdisc *qdisc,
		   int replace)
{
	struct nl_msg *m = rtnl_qdisc_build_add_request(qdisc, replace);
	int err;

	if ((err = nl_send_auto_complete(handle, m->nmsg)) < 0)
		return err;

	nl_msg_free(m);
	return nl_wait_for_ack(handle);
}

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
struct nl_msg * rtnl_qdisc_build_change_request(struct rtnl_qdisc *qdisc,
						struct rtnl_qdisc *new)
{
	return qdisc_build(qdisc, RTM_NEWQDISC, 0);
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
struct nl_msg * rtnl_qdisc_build_delete_request(struct rtnl_qdisc *qdisc)
{
	struct nl_msg *m;
	struct nlmsghdr n = {
		.nlmsg_type = RTM_DELQDISC,
	};
	struct tcmsg t = {
		.tcm_family = AF_UNSPEC,
		.tcm_handle = qdisc->q_handle,
		.tcm_parent = qdisc->q_parent,
		.tcm_ifindex = qdisc->q_ifindex,
	};

	m = nl_msg_build(&n);
	nl_msg_append_raw(m, &t, sizeof(t));

	return m;
}

/**
 * Delete a qdisc
 * @arg handle		netlink handle
 * @arg qdisc		qdisc to delete
 * @return 0 on success or a negative error code
 */
int rtnl_qdisc_delete(struct nl_handle *handle, struct rtnl_qdisc *qdisc)
{
	struct nl_msg *m = rtnl_qdisc_build_delete_request(qdisc);
	int err;

	if ((err = nl_send_auto_complete(handle, m->nmsg)) < 0)
		return err;

	nl_msg_free(m);
	return nl_wait_for_ack(handle);
}

/**
 * Build a netlink request message to delete the root qdisc
 * on the specified interface
 * @arg ifindex		interface index the root qdisc is attached to
 *
 * Builds a new netlink message requesting a deletion of the root
 * qdisc on the specified interface. The netlink message header
 * isn't fully equipped with all relevant fields and must thus
 * be sent out via nl_send_auto_complete() or supplemented as needed.
 *
 * @return New netlink message
 */
struct nl_msg * rtnl_qdisc_build_delete_root_request(int ifindex)
{
	struct rtnl_qdisc qdisc = {
		.q_parent = TC_H_ROOT,
		.q_ifindex = ifindex,
		.q_mask = (QDISC_HAS_PARENT | QDISC_HAS_IFINDEX)
	};

	return rtnl_qdisc_build_delete_request(&qdisc);
}

/**
 * Delete the root qdisc on the specified interface
 * @arg handle		netlink handle
 * @arg ifindex		interface index the root qdisc is attached to
 * @return 0 on success or a negative error code
 */
int rtnl_qdisc_delete_root(struct nl_handle *handle, int ifindex)
{
	struct nl_msg *m = rtnl_qdisc_build_delete_root_request(ifindex);
	int err;

	if ((err = nl_send_auto_complete(handle, m->nmsg)) < 0)
		return err;

	nl_msg_free(m);
	return nl_wait_for_ack(handle);
}

/**
 * Build a netlink request message to delete the ingress qdisc
 * on the specified interface
 * @arg ifindex		interface index the ingress qdisc is attached to
 *
 * Builds a new netlink message requesting a deletion of the ingress
 * qdisc on the specified interface. The netlink message header
 * isn't fully equipped with all relevant fields and must thus
 * be sent out via nl_send_auto_complete() or supplemented as needed.
 *
 * @return New netlink message
 */
struct nl_msg * rtnl_qdisc_build_delete_ingress_request(int ifindex)
{
	struct rtnl_qdisc qdisc = {
		.q_parent = TC_H_INGRESS,
		.q_handle = 0xFFFF0000,
		.q_ifindex = ifindex,
		.q_mask = (QDISC_HAS_PARENT|QDISC_HAS_IFINDEX|QDISC_HAS_HANDLE)
	};

	return rtnl_qdisc_build_delete_request(&qdisc);
}

/**
 * Delete the ingress qdisc on the specified interface
 * @arg handle		netlink handle
 * @arg ifindex		interface index the ingress qdisc is attached to
 * @return 0 on success or a negative error code (See rtnl_qdisc_delete())
 */
int rtnl_qdisc_delete_ingress(struct nl_handle *handle, int ifindex)
{
	struct nl_msg *m = rtnl_qdisc_build_delete_ingress_request(ifindex);
	int err;

	if ((err = nl_send_auto_complete(handle, m->nmsg)) < 0)
		return err;

	nl_msg_free(m);
	return nl_wait_for_ack(handle);
}

/** @} */

/**
 * @name General API
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
 *       cache after using it. (nl_cache_destroy_and_free())
 * @return The cache or NULL if an error has occured.
 */
struct nl_cache * rtnl_qdisc_build_cache(struct nl_handle *handle)
{
	struct nl_cache * cache = calloc(1, sizeof(*cache));

	if (cache == NULL)
		return NULL;

	cache->c_type = RTNL_QDISC;
	cache->c_type_size = sizeof(struct rtnl_qdisc);
	cache->c_ops = &rtnl_qdisc_ops;

	if (nl_cache_update(handle, cache) < 0) {
		free(cache);
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
	int i;

	if (cache->c_type != RTNL_QDISC || NULL == cache->c_cache)
		return NULL;

	for (i = 0; i < cache->c_index; i++) {
		struct rtnl_qdisc *q;
		q = (struct rtnl_qdisc *) NL_CACHE_ELEMENT_AT(cache, i);

		if (q && q->q_parent == parent && q->q_ifindex == ifindex)
			return q;
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
	int i;

	if (cache->c_type != RTNL_QDISC || NULL == cache->c_cache)
		return NULL;

	for (i = 0; i < cache->c_index; i++) {
		struct rtnl_qdisc *q;
		q = (struct rtnl_qdisc *) NL_CACHE_ELEMENT_AT(cache, i);

		if (q && q->q_handle == handle && q->q_ifindex == ifindex)
			return q;
	}

	return NULL;
}

/**
 * Return the root qdisc in the provided cache
 * @arg cache		qdisc cache
 * @arg ifindex         interface the qdisc is attached to
 * @return pointer to qdisc inside the cache or NULL if no match was found.
 */
struct rtnl_qdisc * rtnl_qdisc_get_root(struct nl_cache *cache, int ifindex)
{
	return rtnl_qdisc_get_by_parent(cache, ifindex, TC_H_ROOT);
}

/**
 * Return the ingress qdisc in the provided cache
 * @arg cache		qdisc cache
 * @arg ifindex		interface the qdisc is attached to
 * @return pointer to qdisc inside the cache or NULL if no match was found.
 */
struct rtnl_qdisc * rtnl_qdisc_get_ingress(struct nl_cache *cache,
					   int ifindex)
{
	return rtnl_qdisc_get_by_parent(cache, ifindex, TC_H_INGRESS);
}

/**
 * Dump qdisc attributes
 * @arg qdisc		qdisc to dump
 * @arg fd		file descriptor
 * @arg params		dumping parameters
 */
void rtnl_qdisc_dump(struct rtnl_qdisc *qdisc, FILE *fd,
		     struct nl_dump_params *params)
{
	int type = params ? params->dp_type : NL_DUMP_FULL;

	if (type < 0 || type > NL_DUMP_MAX)
		BUG();

	if (rtnl_qdisc_ops.co_dump[type])
		rtnl_qdisc_ops.co_dump[type](NULL, (struct nl_common *) qdisc,
					     fd, params);
}

/**
 * Return the options of a qdisc
 * @arg qdisc		qdisc carrying the optiosn
 * @return new netlink message with no header carrying the options as payload
 */
struct nl_msg *rtnl_qdisc_get_opts(struct rtnl_qdisc *qdisc)
{
	struct rtnl_qdisc_ops *ops = qdisc_lookup_ops(qdisc->q_kind);
	return ops->qo_get_opts ? ops->qo_get_opts(qdisc) : NULL;
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
			      void (*cb)(struct nl_common *, void *), void *arg)
{
	struct rtnl_class filter = RTNL_INIT_CLASS();

	rtnl_class_set_parent(&filter, qdisc->q_handle);
	rtnl_class_set_ifindex(&filter, qdisc->q_ifindex);
	rtnl_class_set_kind(&filter, qdisc->q_kind);

	nl_cache_foreach_filter(cache, (struct nl_common *) &filter, cb, arg);
}

/**
 * Call a callback for each child class of a qdisc using a own cache
 * @arg handle		netlink handle
 * @arg qdisc		the parent qdisc
 * @arg cb              callback function
 * @arg arg             argument to be passed to callback function
 */
void rtnl_qdisc_foreach_child_nocache(struct nl_handle *handle,
				      struct rtnl_qdisc *qdisc,
				      void (*cb)(struct nl_common *, void *),
				      void *arg)
{
	struct nl_cache *c = rtnl_class_build_cache(handle, qdisc->q_ifindex);

	if (c == NULL)
		return;

	rtnl_qdisc_foreach_child(qdisc, c, cb, arg);
	nl_cache_destroy_and_free(c);
}

/**
 * Call a callback for each filter attached to the qdisc
 * @arg qdisc		the parent qdisc
 * @arg cache		a filter cache including at least all the filters
 *                      attached to the specified qdisc
 * @arg cb              callback function
 * @arg arg             argument to be passed to callback function
 */
void rtnl_qdisc_foreach_filter(struct rtnl_qdisc *qdisc, struct nl_cache *cache,
			      void (*cb)(struct nl_common *, void *), void *arg)
{
	struct rtnl_filter filter = RTNL_INIT_FILTER();

	if (cache->c_type != RTNL_CLASS)
		return;

	rtnl_filter_set_ifindex(&filter, qdisc->q_ifindex);
	rtnl_filter_set_parent(&filter, qdisc->q_parent);

	nl_cache_foreach_filter(cache, (struct nl_common *) &filter, cb, arg);
}

/**
 * Call a callback for each filter of the qdisc using a own cache
 * @arg handle		netlink handle
 * @arg qdisc		the parent qdisc
 * @arg cb              callback function
 * @arg arg             argument to be passed to callback function
 */
void rtnl_qdisc_foreach_filter_nocache(struct nl_handle *handle,
				      struct rtnl_qdisc *qdisc,
				      void (*cb)(struct nl_common *, void *),
				      void *arg)
{
	struct nl_cache *c;
	c = rtnl_filter_build_cache(handle, qdisc->q_ifindex, qdisc->q_parent);

	if (c == NULL)
		return;

	rtnl_qdisc_foreach_filter(qdisc, c, cb, arg);
	nl_cache_destroy_and_free(c);
}

/** @} */

/**
 * @name Attribute Modifications
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
 * Set the interface of a qdisc via a interface name
 * @arg qdisc		qdisc to be changed
 * @arg cache		link cache to look up interface index
 * @arg name		interface name
 * @return 0 on success or a negative error code.
 */
int rtnl_qdisc_set_ifindex_name(struct rtnl_qdisc *qdisc,
				struct nl_cache *cache, const char *name)
{
	return tca_set_ifindex_name((struct rtnl_tca *) qdisc,
				    cache, name);
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
 * Set the parent of a qdisc to the specified value
 * @arg qdisc		qdisc to be changed
 * @arg parent		new parent handle
 */
void rtnl_qdisc_set_parent(struct rtnl_qdisc *qdisc, uint32_t parent)
{
	tca_set_parent((struct rtnl_tca *) qdisc, parent);
}

/**
 * Set the kind of a qdisc to the specified value
 * @arg qdisc		qdisc to be changed
 * @arg name		new kind name
 */
void rtnl_qdisc_set_kind(struct rtnl_qdisc *qdisc, const char *name)
{
	tca_set_kind((struct rtnl_tca *) qdisc, name);
}

/** @} */

struct nl_cache_ops rtnl_qdisc_ops = {
	.co_request_update	= &qdisc_request_update,
	.co_msg_parser		= &qdisc_msg_parser,
	.co_free_data		= &qdisc_free_data,
	.co_dump[NL_DUMP_BRIEF]	= &qdisc_dump_brief,
	.co_dump[NL_DUMP_FULL]	= &qdisc_dump_full,
	.co_dump[NL_DUMP_STATS]	= &qdisc_dump_with_stats,
	.co_filter		= &qdisc_filter,
};

/** @} */
