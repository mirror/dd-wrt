/*
 * class.c              rtnetlink class
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
 * @defgroup class Classes
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/route/tc.h>
#include <netlink/route/class.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/filter.h>
#include <netlink/helpers.h>

static struct rtnl_class_ops *class_ops;

static struct rtnl_class_ops * class_lookup_ops(char *kind)
{
	struct rtnl_class_ops *o = NULL;

	for (o = class_ops; o; o = o->co_next)
		if (!strcmp(kind, o->co_kind))
			break;

	return o;
}

/**
 * @name Class Module API
 * @{
 */

/**
 * Register a class module
 * @arg ops		class module operations
 */
void rtnl_class_register(struct rtnl_class_ops *ops)
{
	struct rtnl_class_ops *o, **op;

	for (op = &class_ops; (o = *op) != NULL; op = &o->co_next)
		if (!strcasecmp(ops->co_kind, o->co_kind))
			return;

	ops->co_next = NULL;
	*op = ops;
}

/**
 * Unregister a class module
 * @arg ops		class module operations
 */
void rtnl_class_unregister(struct rtnl_class_ops *ops)
{
	struct rtnl_class_ops *o, **op;

	for (op = &class_ops; (o = *op) != NULL; op = &o->co_next)
		if (!strcasecmp(ops->co_kind, o->co_kind))
			break;

	if (NULL == o)
		return;

	*op = ops->co_next;
}

/** @} */

static int class_msg_parser(struct sockaddr_nl *who, struct nlmsghdr *n,
			    void *arg)
{
	int err;
	struct nl_parser_param *pp = arg;
	struct rtnl_class class = RTNL_INIT_CLASS();
	struct rtnl_class_ops *ops;

	if (n->nlmsg_type != RTM_NEWTCLASS)
		return P_IGNORE;

	err = tca_msg_parser(n, (struct rtnl_tca *) &class);
	if (err < 0)
		return err == -EINTR ? P_IGNORE : err;

	ops = class_lookup_ops(class.c_kind);

	if (ops && ops->co_msg_parser) {
		err = ops->co_msg_parser(&class);
		if (err < 0)
			return err;
	}

	err = pp->pp_cb((struct nl_common *) &class, pp);
	if (err < 0)
		return err;

	return P_ACCEPT;
}

static int class_request_update(struct nl_cache *c, struct nl_handle *h)
{
	struct tcmsg tm = {
		.tcm_family = AF_UNSPEC,
		.tcm_ifindex = CLASS_CACHE_IFINDEX(c),
	};

	return nl_request_with_data(h, RTM_GETTCLASS, NLM_F_DUMP,
		(unsigned char *) &tm, sizeof(tm));
}

static void class_free_data(struct nl_common *a)
{
	struct rtnl_class *c = (struct rtnl_class *) a;
	struct rtnl_class_ops *o = class_lookup_ops(c->c_kind);

	tca_free_data((struct rtnl_tca *) a);

	if (o && o->co_free_data)
		o->co_free_data(c);
}

static int class_dump_brief(struct nl_cache *cache, struct nl_common *arg,
			    FILE *fd, struct nl_dump_params *params)
{
	struct rtnl_class *cl = (struct rtnl_class *) arg;
	struct rtnl_class_ops *ops = class_lookup_ops(cl->c_kind);

	int line = tca_dump_brief(cache, (struct rtnl_tca *) cl, "class",
				  fd, params, 0);

	if (ops && ops->co_dump[NL_DUMP_BRIEF])
		line = ops->co_dump[NL_DUMP_BRIEF](cache, cl, fd, params, line);
	fprintf(fd, "\n");

	return line;
}

static int class_dump_full(struct nl_cache *cache, struct nl_common *a,
			   FILE *fd, struct nl_dump_params *params)
{
	struct rtnl_class *cl = (struct rtnl_class *) a;
	struct rtnl_class_ops *ops = class_lookup_ops(cl->c_kind);

	int line = class_dump_brief(cache, a, fd, params);

	dp_new_line(fd, params, line++);
	line = tca_dump_full(cache, (struct rtnl_tca *) a, fd, params, line);
	
	if (cl->c_info) {
		char s[32];
		fprintf(fd, "child-qdisc %s ",
			nl_handle2str_r(cl->c_info, s, sizeof(s)));
	}

	if (ops && ops->co_dump[NL_DUMP_FULL])
		line = ops->co_dump[NL_DUMP_FULL](cache, cl, fd, params, line);
	else if (!cl->c_info)
		fprintf(fd, "nop");
	
	fprintf(fd, "\n");

	return line;
}

static int class_dump_with_stats(struct nl_cache *cache, struct nl_common *a,
				  FILE *fd, struct nl_dump_params *params)
{
	struct rtnl_class *cl = (struct rtnl_class *) a;
	struct rtnl_class_ops *ops = class_lookup_ops(cl->c_kind);

	int line = class_dump_full(cache, a, fd, params);
	line = tca_dump_with_stats(cache, (struct rtnl_tca *) a,
				   fd, params, line);
	fprintf(fd, "\n");
	if (ops && ops->co_dump[NL_DUMP_STATS])
		line = ops->co_dump[NL_DUMP_STATS](cache, cl, fd, params, line);

	return line;
}

static int class_filter(struct nl_common *o, struct nl_common *f)
{
	if (o->ce_type != RTNL_CLASS || f->ce_type != RTNL_CLASS)
		return 0;

	return tca_filter((struct rtnl_tca *) o, (struct rtnl_tca *) f);
}

/**
 * @name General API
 * @{
 */

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
 *       cache after using it. (nl_cache_destroy_and_free())
 * @return The cache or NULL if an error has occured.
 */
struct nl_cache * rtnl_class_build_cache(struct nl_handle *handle,
					 int ifindex)
{
	struct nl_cache * cache = calloc(1, sizeof(*cache));

	if (cache == NULL)
		return NULL;

	CLASS_CACHE_IFINDEX(cache) = ifindex;
	cache->c_type = RTNL_CLASS;
	cache->c_type_size = sizeof(struct rtnl_class);
	cache->c_ops = &rtnl_class_ops;

	if (nl_cache_update(handle, cache) < 0) {
		free(cache);
		return NULL;
	}

	return cache;
}

/**
 * Determine if the class has a leaf qdisc attached
 * @arg class		class to check
 */
int rtnl_class_has_qdisc_attached(struct rtnl_class *class)
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

	if (!rtnl_class_has_qdisc_attached(class))
		return NULL;

	leaf = rtnl_qdisc_get_by_parent(cache, class->c_ifindex, class->c_handle);
	if (leaf && leaf->q_handle == class->c_info)
		return leaf;

	return NULL;
}


/**
 * Dump class attributes
 * @arg class		class to dump
 * @arg fd		file descriptor
 * @arg params		dumping parameters
 */
void rtnl_class_dump(struct rtnl_class *class, FILE *fd,
		     struct nl_dump_params *params)
{
	int type = params ? params->dp_type : NL_DUMP_FULL;

	if (type < 0 || type > NL_DUMP_MAX)
		BUG();

	if (rtnl_class_ops.co_dump[type])
		rtnl_class_ops.co_dump[type](NULL, (struct nl_common *) class,
					     fd, params);
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
			      void (*cb)(struct nl_common *, void *), void *arg)
{
	struct rtnl_class filter = RTNL_INIT_CLASS();

	rtnl_class_set_parent(&filter, class->c_handle);
	rtnl_class_set_ifindex(&filter, class->c_ifindex);
	rtnl_class_set_kind(&filter, class->c_kind);

	nl_cache_foreach_filter(cache, (struct nl_common *) &filter, cb, arg);
}

/**
 * Call a callback for each child class of a class using a own cache
 * @arg handle		netlink handle
 * @arg class		the parent class
 * @arg cb              callback function
 * @arg arg             argument to be passed to callback function
 */
void rtnl_class_foreach_child_nocache(struct nl_handle *handle,
				      struct rtnl_class *class,
				      void (*cb)(struct nl_common *, void *),
				      void *arg)
{
	struct nl_cache *c = rtnl_class_build_cache(handle, class->c_ifindex);

	if (c == NULL)
		return;

	rtnl_class_foreach_child(class, c, cb, arg);
	nl_cache_destroy_and_free(c);
}

/**
 * Call a callback for each filter attached to the class
 * @arg class		the parent class
 * @arg cache		a filter cache including at least all the filters
 *                      attached to the specified class
 * @arg cb              callback function
 * @arg arg             argument to be passed to callback function
 */
void rtnl_class_foreach_filter(struct rtnl_class *class, struct nl_cache *cache,
			      void (*cb)(struct nl_common *, void *), void *arg)
{
	struct rtnl_filter filter = RTNL_INIT_FILTER();

	if (cache->c_type != RTNL_CLASS)
		return;

	rtnl_filter_set_ifindex(&filter, class->c_ifindex);
	rtnl_filter_set_parent(&filter, class->c_parent);

	nl_cache_foreach_filter(cache, (struct nl_common *) &filter, cb, arg);
}

/**
 * Call a callback for each filter of the class using a own cache
 * @arg handle		netlink handle
 * @arg class		the parent class
 * @arg cb              callback function
 * @arg arg             argument to be passed to callback function
 */
void rtnl_class_foreach_filter_nocache(struct nl_handle *handle,
				      struct rtnl_class *class,
				      void (*cb)(struct nl_common *, void *),
				      void *arg)
{
	struct nl_cache *c;
	c = rtnl_filter_build_cache(handle, class->c_ifindex, class->c_parent);

	if (c == NULL)
		return;

	rtnl_class_foreach_filter(class, c, cb, arg);
	nl_cache_destroy_and_free(c);
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
 * Set the interface of a class via a interface name
 * @arg class		class to be changed
 * @arg cache		link cache to look up interface index
 * @arg name		interface name
 * @return 0 on success or a negative error code.
 */
int rtnl_class_set_ifindex_name(struct rtnl_class *class,
				struct nl_cache *cache, const char *name)
{
	return tca_set_ifindex_name((struct rtnl_tca *) class,
				    cache, name);
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
 * Set the parent of a class to the specified value
 * @arg class		class to be changed
 * @arg parent		new parent handle
 */
void rtnl_class_set_parent(struct rtnl_class *class, uint32_t parent)
{
	tca_set_parent((struct rtnl_tca *) class, parent);
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

/** @} */

struct nl_cache_ops rtnl_class_ops = {
	.co_request_update	= &class_request_update,
	.co_msg_parser		= &class_msg_parser,
	.co_free_data         	= &class_free_data,
	.co_dump[NL_DUMP_BRIEF]	= &class_dump_brief,
	.co_dump[NL_DUMP_FULL]	= &class_dump_full,
	.co_dump[NL_DUMP_STATS]	= &class_dump_with_stats,
	.co_filter		= &class_filter,
};

/** @} */
