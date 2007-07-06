/*
 * route/filter.c         Traffic Control Filters
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
 * @defgroup filter Filters (Classifiers)
 *
 * @par Filter Identification
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
#include <netlink/netlink.h>
#include <netlink/helpers.h>
#include <netlink/route/tc.h>
#include <netlink/route/filter.h>
#include <netlink/route/link.h>
#include <netlink/route/rtattr.h>

static struct rtnl_filter_ops *filter_ops;

static struct rtnl_filter_ops * filter_lookup_ops(char *kind)
{
	struct rtnl_filter_ops *o = NULL;

	for (o = filter_ops; o; o = o->next)
		if (!strcmp(kind, o->kind))
			break;

	return o;
}

/**
 * @name Filter Module API
 * @{
 */

/**
 * Register a filter module
 * @arg ops		filter module operations
 */
void rtnl_filter_register(struct rtnl_filter_ops *ops)
{
	struct rtnl_filter_ops *o, **op;

	for (op = &filter_ops; (o = *op) != NULL; op = &o->next)
		if (!strcasecmp(ops->kind, o->kind))
			return;

	ops->next = NULL;
	*op = ops;
}

/**
 * Unregister a filter module
 * @arg ops		filter module operations
 */
void rtnl_filter_unregister(struct rtnl_filter_ops *ops)
{
	struct rtnl_filter_ops *o, **op;

	for (op = &filter_ops; (o = *op) != NULL; op = &o->next)
		if (!strcasecmp(ops->kind, o->kind))
			break;

	if (NULL == o)
		return;

	*op = ops->next;
}

/** @} */

static int filter_msg_parser(struct sockaddr_nl *who, struct nlmsghdr *n,
			     void *arg)
{
	int err;
	struct nl_parser_param *pp = arg;
	struct rtnl_filter filter = RTNL_INIT_FILTER();
	struct rtnl_filter_ops *ops;

	if (n->nlmsg_type != RTM_NEWTFILTER)
		return P_IGNORE;

	err = tca_msg_parser(n, (struct rtnl_tca *) &filter);
	if (err < 0)
		return err == -EINTR ? P_IGNORE : err;

	ops = filter_lookup_ops(filter.f_kind);

	filter.prio = TC_H_MAJ(filter.f_info) >> 16;
	filter.protocol = TC_H_MIN(filter.f_info);

	if (ops && ops->msg_parser)
		if ((err = ops->msg_parser(&filter)) < 0)
			return err;

	err = pp->pp_cb((struct nl_common *) &filter, pp);
	if (err < 0)
		return err;

	return P_ACCEPT;
}

static int filter_request_update(struct nl_cache *c, struct nl_handle *h)
{
	struct tcmsg tm = {
		.tcm_family = AF_UNSPEC,
		.tcm_ifindex = FILTER_CACHE_IFINDEX(c),
		.tcm_parent = FILTER_CACHE_PARENT(c),
	};

	return nl_request_with_data(h, RTM_GETTFILTER, NLM_F_DUMP,
		(unsigned char *) &tm, sizeof(tm));
}


static void filter_free_data(struct nl_common *a)
{
	struct rtnl_filter *f = (struct rtnl_filter *) a;
	struct rtnl_filter_ops *o = filter_lookup_ops(f->f_kind);

	tca_free_data((struct rtnl_tca *) a);

	if (o && o->free_data)
		o->free_data(f);
}

static int filter_dump_brief(struct nl_cache *cache, struct nl_common *arg,
			     FILE *fd, struct nl_dump_params *params)
{
	char p[32];
	struct rtnl_filter *f = (struct rtnl_filter *) arg;
	struct rtnl_filter_ops *ops = filter_lookup_ops(f->f_kind);

	int line = tca_dump_brief(cache, (struct rtnl_tca *) arg, "cls",
				  fd, params, 0);

	fprintf(fd, " prio %u protocol %s",
		f->prio,
		nl_ether_proto2str_r(ntohs(f->protocol), p, sizeof(p)));

	if (ops && ops->dump[NL_DUMP_BRIEF])
		line = ops->dump[NL_DUMP_BRIEF](cache, f, fd, params, line);
	fprintf(fd, "\n");

	return line;
}

static int filter_dump_full(struct nl_cache *cache, struct nl_common *arg,
			    FILE *fd, struct nl_dump_params *params)
{
	struct rtnl_filter *f = (struct rtnl_filter *) arg;
	struct rtnl_filter_ops *ops = filter_lookup_ops(f->f_kind);

	int line = filter_dump_brief(cache, arg, fd, params);

	dp_new_line(fd, params, line++);
	line = tca_dump_full(cache, (struct rtnl_tca *) arg,
			     fd, params, line);

	if (ops && ops->dump[NL_DUMP_FULL])
		line = ops->dump[NL_DUMP_FULL](cache, f, fd, params, line);

	return line;
}

static int filter_dump_with_stats(struct nl_cache *cache,
				  struct nl_common *arg,
				  FILE *fd, struct nl_dump_params *params)
{
	struct rtnl_filter *f = (struct rtnl_filter *) arg;
	struct rtnl_filter_ops *ops = filter_lookup_ops(f->f_kind);
	int line;

	f->with_stats = 1;

	line = filter_dump_full(cache, arg, fd, params);
	line = tca_dump_with_stats(cache, (struct rtnl_tca *) arg,
				   fd, params, line);

	if (ops && ops->dump[NL_DUMP_STATS])
		line = ops->dump[NL_DUMP_STATS](cache, f, fd, params, line);

	f->with_stats = 0;
	fprintf(fd, "\n");

	return line;
}

static int filter_filter(struct nl_common *o, struct nl_common *f)
{
	if (o->ce_type != RTNL_FILTER || f->ce_type != RTNL_FILTER)
		return 0;

	return tca_filter((struct rtnl_tca *) o, (struct rtnl_tca *) f);
}


static struct nl_msg * filter_build(struct rtnl_filter *f, struct nl_msg *opts,
				    int type, int flags)
{
	struct nl_msg *m;
	struct nlmsghdr n = {
		.nlmsg_type = type,
		.nlmsg_flags = flags,
	};
	int prio = f->f_mask & FILTER_HAS_PRIO ? f->prio : 0;
	int proto = f->f_mask & FILTER_HAS_PROTOCOL ? f->protocol : ETH_P_ALL;
	struct tcmsg t = {
		.tcm_family = AF_UNSPEC,
		.tcm_ifindex = f->f_ifindex,
		.tcm_handle = f->f_handle,
		.tcm_parent = f->f_parent,
		.tcm_info = TC_H_MAKE(prio << 16, htons(proto)),
	};
	m = nl_msg_build(&n);
	nl_msg_append_raw(m, &t, sizeof(t));
	nl_msg_append_tlv(m, TCA_KIND, f->f_kind, strlen(f->f_kind) + 1);

	if (opts)
		nl_msg_append_nested(m, TCA_OPTIONS, opts);

	return m;
}

/**
 * @name Filter Addition/Modification/Deletion
 * @{
 */

/**
 * Build a netlink message to add a new filter
 * @arg filter		filter to add
 * @arg opts		filter options
 *
 * Builds a new netlink message requesting an addition of a filter.
 * The netlink message header isn't fully equipped with all relevant
 * fields and must be sent out via nl_send_auto_complete() or
 * supplemented as needed. \a filter must contain the attributes of
 * the new filter set via \c rtnl_filter_set_* functions. \a opts
 * may point to the filter specific options.
 *
 * The following attributes must be set in the filter:
 *  - XXX
 *
 * @return New netlink message
 */
struct nl_msg * rtnl_filter_build_add_request(struct rtnl_filter *filter,
					      struct nl_msg *opts)
{
	return filter_build(filter, opts, RTM_NEWTFILTER, NLM_F_CREATE);
}

/**
 * Add a new filter
 * @arg handle		netlink handle
 * @arg filter		filter to add
 * @arg opts		filter options
 *
 * Builds a netlink message by calling rtnl_filter_build_add_request(),
 * sends the request to the kernel and waits for the next ACK to be
 * received and thus blocks until the request has been processed.
 *
 * The following attributes must be set in the template:
 *  - XXX
 *
 * @return 0 on sucess or a negative error if an error occured.
 */
int rtnl_filter_add(struct nl_handle *handle, struct rtnl_filter *filter,
		    struct nl_msg *opts)
{
	int err;
	struct nl_msg *m = rtnl_filter_build_add_request(filter, opts);
	
	if ((err = nl_send_auto_complete(handle, nl_msg_get(m))) < 0)
		return err;

	nl_msg_free(m);
	return nl_wait_for_ack(handle);
}

/**
 * Build a netlink message to change filter attributes
 * @arg filter		the filter to change
 * @arg opts		new filter options
 *
 * Builds a new netlink message requesting a change of a neigh
 * attributes. The netlink message header isn't fully equipped with
 * all relevant fields and must thus be sent out via nl_send_auto_complete()
 * or supplemented as needed.
 * \a neigh must point to a link handle currently configured in the kernel
 * and \a tmpl must contain the attributes to be changed set via
 * \c rtnl_neig_set_* functions.
 *
 * @return The netlink message
 * @note Not all attributes can be changed, see
 *       \ref neigh_changeable "Changeable Attributes" for a list.
 */
struct nl_msg *rtnl_filter_build_change_request(struct rtnl_filter *filter,
						struct nl_msg *opts)
{
	return filter_build(filter, opts, RTM_NEWTFILTER, NLM_F_REPLACE);
}

/**
 * Build a netlink request message to delete a filter
 * @arg filter		filter to delete
 *
 * Builds a new netlink message requesting a deletion of a filter.
 * The netlink message header isn't fully equipped with all relevant
 * fields and must thus be sent out via nl_send_auto_complete()
 * or supplemented as needed.
 *
 * @return New netlink message
 */
struct nl_msg * rtnl_filter_build_delete_request(struct rtnl_filter *filter)
{
	return filter_build(filter, NULL, RTM_DELTFILTER, 0);
}


/**
 * Delete a filter
 * @arg handle		netlink handle
 * @arg filter		filter to delete
 *
 * Builds a netlink message by calling rtnl_filter_build_delete_request(),
 * sends the request to the kernel and waits for the next ACK to be
 * received and thus blocks until the request has been processed.
 *
 * @return 0 on sucess or a negative error if an error occured.
 */
int rtnl_filter_delete(struct nl_handle *handle, struct rtnl_filter *filter)
{
	int err;
	struct nl_msg *m = rtnl_filter_build_delete_request(filter);
	
	if ((err = nl_send_auto_complete(handle, nl_msg_get(m))) < 0)
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
 * Build a filter cache including all filters attached to the
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
 *       cache after using it. (nl_cache_destroy_and_free())
 * @return The cache or NULL if an error has occured.
 */
struct nl_cache * rtnl_filter_build_cache(struct nl_handle *handle,
					 int ifindex, uint32_t parent)
{
	struct nl_cache * cache = calloc(1, sizeof(*cache));

	if (cache == NULL)
		return NULL;

	FILTER_CACHE_IFINDEX(cache) = ifindex;
	FILTER_CACHE_PARENT(cache) = parent;
	cache->c_type = RTNL_FILTER;
	cache->c_type_size = sizeof(struct rtnl_filter);
	cache->c_ops = &rtnl_filter_ops;

	if (nl_cache_update(handle, cache) < 0) {
		free(cache);
		return NULL;
	}

	return cache;
}

void rtnl_filter_dump(struct rtnl_filter *filter, FILE *fd,
		      struct nl_dump_params *params)
{
	int type = params ? params->dp_type : NL_DUMP_FULL;

	if (type < 0 || type > NL_DUMP_MAX)
		BUG();

	if (rtnl_filter_ops.co_dump[type])
		rtnl_filter_ops.co_dump[type](NULL,
					      (struct nl_common *) filter,
					      fd, params);
}

/**************************************************************************
 ** HANDLE SET API
 **************************************************************************/

void rtnl_filter_set_ifindex(struct rtnl_filter *f, int ifindex)
{
	tca_set_ifindex((struct rtnl_tca *) f, ifindex);
}

int rtnl_filter_set_ifindex_name(struct rtnl_filter *f, struct nl_cache *c,
				 const char *dev)
{
	return tca_set_ifindex_name((struct rtnl_tca *) f, c, dev);
}

void rtnl_filter_set_handle(struct rtnl_filter *f, uint32_t handle)
{
	tca_set_handle((struct rtnl_tca *) f, handle);
}

void rtnl_filter_set_parent(struct rtnl_filter *f, uint32_t parent)
{
	tca_set_parent((struct rtnl_tca *) f, parent);
}

void rtnl_filter_set_kind(struct rtnl_filter *f, const char *kind)
{
	tca_set_kind((struct rtnl_tca *) f, kind);
}

void rtnl_filter_set_prio(struct rtnl_filter *f, uint32_t prio)
{
	f->prio = prio;
	f->f_mask |= FILTER_HAS_PRIO;
}

void rtnl_filter_set_protocol(struct rtnl_filter *f, uint32_t protocol)
{
	f->protocol = protocol;
	f->f_mask |= FILTER_HAS_PROTOCOL;
}

/**************************************************************************
 ** 
 ** CACHE OPS STRUCT
 **
 **************************************************************************/

struct nl_cache_ops rtnl_filter_ops = {
	.co_request_update	= &filter_request_update,
	.co_filter		= &filter_filter,
	.co_free_data		= &filter_free_data,
	.co_msg_parser		= &filter_msg_parser,
	.co_dump[NL_DUMP_BRIEF]	= &filter_dump_brief,
	.co_dump[NL_DUMP_FULL]	= &filter_dump_full,
	.co_dump[NL_DUMP_STATS]	= &filter_dump_with_stats,
};

/** @} */
