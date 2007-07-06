/*
 * src/nl-route-dump.c     Dump route attributes
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
	"Usage: nl-route-dump <mode> [<filter>]\n"
	"  mode := { brief | detailed | stats | xml }\n");
	exit(1);
}

#include "f_route.c"

int main(int argc, char *argv[])
{
	struct nl_handle *nlh;
	struct nl_cache *link_cache, *route_cache;
	struct rtnl_route *route;
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
		goto errout;

	route = rtnl_route_alloc();
	if (!route)
		goto errout;

	if (nltool_connect(nlh, NETLINK_ROUTE) < 0)
		goto errout_free;

	link_cache = nltool_alloc_link_cache(nlh);
	if (!link_cache)
		goto errout_close;

	route_cache = nltool_alloc_route_cache(nlh);
	if (!route_cache)
		goto errout_link_cache;

	params.dp_type = nltool_parse_dumptype(argv[1]);
	if (params.dp_type < 0)
		goto errout_route_cache;

	get_filter(route, argc, argv, 2, route_cache, link_cache);

	nl_cache_dump_filter(route_cache, &params, (struct nl_object *) route);

	err = 0;

errout_route_cache:
	nl_cache_free(route_cache);
errout_link_cache:
	nl_cache_free(link_cache);
errout_close:
	nl_close(nlh);
errout_free:
	rtnl_route_put(route);
errout:
	nl_handle_destroy(nlh);
	return err;
}
