/*
 * src/nl-neigh-dump.c     Dump neighbour attributes
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

#include "utils.h"

static void print_usage(void)
{
	printf(
	"Usage: nl-neigh-dump <mode> [<filter>]\n"
	"  mode := { brief | detailed | stats | xml }\n"
	"  filter := [dev DEV] [dst ADDR] [lladdr ADDR]\n");
	exit(1);
}

#include "f_neigh.c"

int main(int argc, char *argv[])
{
	struct nl_handle *nlh;
	struct nl_cache *link_cache, *neigh_cache;
	struct rtnl_neigh *neigh;
	struct nl_dump_params params = {
		.dp_fd = stdout,
		.dp_type = NL_DUMP_BRIEF
	};
	int err = 1;

	if (nltool_init(argc, argv) < 0)
		return -1;

	if (argc < 2 || !strcmp(argv[1], "-h"))
		print_usage();
	
	nlh = nl_handle_alloc_nondefault(nltool_cbset);
	if (!nlh)
		return -1;

	neigh = rtnl_neigh_alloc();
	if (neigh == NULL)
		goto errout;

	if (nltool_connect(nlh, NETLINK_ROUTE) < 0)
		goto errout_free;

	link_cache = nltool_alloc_link_cache(nlh);
	if (!link_cache)
		goto errout_close;

	neigh_cache = nltool_alloc_neigh_cache(nlh);
	if (!neigh_cache)
		goto errout_link_cache;

	params.dp_type = nltool_parse_dumptype(argv[1]);
	if (params.dp_type < 0)
		goto errout_neigh_cache;

	get_filter(neigh, argc, argv, 2, neigh_cache);

	nl_cache_dump_filter(neigh_cache, &params, (struct nl_object *) neigh);

	err = 0;

errout_neigh_cache:
	nl_cache_free(neigh_cache);
errout_link_cache:
	nl_cache_free(link_cache);
errout_close:
	nl_close(nlh);
errout_free:
	rtnl_neigh_put(neigh);
errout:
	nl_handle_destroy(nlh);
	return err;
}
