/*
 * src/nl-tctree-dump.c		Dump Traffic Control Tree
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

#include "utils.h"
#include <linux/pkt_sched.h>

static struct nl_handle *nl_handle;
static struct nl_cache *qdisc_cache, *class_cache;
static struct nl_dump_params dump_params = {
	.dp_type = NL_DUMP_FULL,
};

static int ifindex;
static void print_qdisc(struct nl_object *, void *);

static void print_class(struct nl_object *obj, void *arg)
{
	struct rtnl_qdisc *leaf;
	struct rtnl_class *class = (struct rtnl_class *) obj;
	struct nl_cache *cls_cache;
	uint32_t parent = rtnl_class_get_handle(class);

	dump_params.dp_prefix = (int) arg;
	nl_object_dump(obj, &dump_params);

	leaf = rtnl_class_leaf_qdisc(class, qdisc_cache);
	if (leaf)
		print_qdisc((struct nl_object *) leaf, arg + 2);

	rtnl_class_foreach_child(class, class_cache, &print_class, arg + 2);

	cls_cache = rtnl_cls_alloc_cache(nl_handle, ifindex, parent);
	if (!cls_cache)
		return;

	dump_params.dp_prefix = (int) arg + 2;
	nl_cache_dump(cls_cache, &dump_params);
	nl_cache_free(cls_cache);
}

static void print_qdisc(struct nl_object *obj, void *arg)
{
	struct rtnl_qdisc *qdisc = (struct rtnl_qdisc *) obj;
	struct nl_cache *cls_cache;
	uint32_t parent = rtnl_qdisc_get_handle(qdisc);

	dump_params.dp_prefix = (int) arg;
	nl_object_dump(obj, &dump_params);

	rtnl_qdisc_foreach_child(qdisc, class_cache, &print_class, arg + 2);

	cls_cache = rtnl_cls_alloc_cache(nl_handle, ifindex, parent);
	if (!cls_cache)
		return;

	dump_params.dp_prefix = (int) arg + 2;
	nl_cache_dump(cls_cache, &dump_params);
	nl_cache_free(cls_cache);
}

static void print_link(struct nl_object *obj, void *arg)
{
	struct rtnl_link *link = (struct rtnl_link *) obj;
	struct rtnl_qdisc *qdisc;

	ifindex = rtnl_link_get_ifindex(link);
	dump_params.dp_prefix = 0;
	nl_object_dump(obj, &dump_params);

	class_cache = rtnl_class_alloc_cache(nl_handle, ifindex);
	if (!class_cache)
		return;

	qdisc = rtnl_qdisc_get_by_parent(qdisc_cache, ifindex, TC_H_ROOT);
	if (qdisc) {
		print_qdisc((struct nl_object *) qdisc, (void *) 2);
		rtnl_qdisc_put(qdisc);
	}

	qdisc = rtnl_qdisc_get_by_parent(qdisc_cache, ifindex, 0);
	if (qdisc) {
		print_qdisc((struct nl_object *) qdisc, (void *) 2);
		rtnl_qdisc_put(qdisc);
	}

	qdisc = rtnl_qdisc_get_by_parent(qdisc_cache, ifindex, TC_H_INGRESS);
	if (qdisc) {
		print_qdisc((struct nl_object *) qdisc, (void *) 2);
		rtnl_qdisc_put(qdisc);
	}

	nl_cache_free(class_cache);
}

int main(int argc, char *argv[])
{
	struct nl_cache *link_cache;

	if (nltool_init(argc, argv) < 0)
		return -1;

	dump_params.dp_fd = stdout;

	if (argc > 1) {
		if (!strcasecmp(argv[1], "brief"))
			dump_params.dp_type = NL_DUMP_BRIEF;
		else if (!strcasecmp(argv[1], "full"))
			dump_params.dp_type = NL_DUMP_FULL;
		else if (!strcasecmp(argv[1], "stats"))
			dump_params.dp_type = NL_DUMP_STATS;
	}

	nl_handle = nl_handle_alloc_nondefault(nltool_cbset);
	if (!nl_handle)
		return 1;

	if (nltool_connect(nl_handle, NETLINK_ROUTE) < 0)
		return 1;

	link_cache = nltool_alloc_link_cache(nl_handle);
	if (!link_cache)
		return 1;

	qdisc_cache = nltool_alloc_qdisc_cache(nl_handle);
	if (!qdisc_cache)
		return 1;

	nl_cache_foreach(link_cache, &print_link, NULL);

	nl_cache_free(qdisc_cache);
	nl_cache_free(link_cache);

	nl_close(nl_handle);
	nl_handle_destroy(nl_handle);
	return 0;
}
