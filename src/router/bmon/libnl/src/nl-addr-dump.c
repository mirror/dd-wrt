/*
 * src/nl-addr-dump.c     Dump address attributes
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
	"Usage: nl-addr-dump <mode> [<filter>]\n"
	"  mode := { brief | detailed | stats | xml }\n"
	"  filter := [dev DEV] [label LABEL] [family FAMILY] [scope SCOPE]\n"
	"            [local ADDR] [peer ADDR] [broadcast ADDR] [anycast ADDR]\n"
	"            [multicast ADDR]\n");
	exit(1);
}

#include "f_addr.c"

int main(int argc, char *argv[])
{
	struct nl_handle *nlh;
	struct nl_cache *link_cache, *addr_cache;
	struct rtnl_addr *addr;
	struct nl_dump_params params = {
		.dp_fd = stdout,
		.dp_type = NL_DUMP_BRIEF
	};
	int err = 1;

	if (nltool_init(argc, argv) < 0)
		return -1;

	nlh = nl_handle_alloc_nondefault(nltool_cbset);
	if (!nlh)
		return -1;

	addr = rtnl_addr_alloc();
	if (!addr)
		goto errout;

	if (argc < 2 || !strcmp(argv[1], "-h"))
		print_usage();

	if (nltool_connect(nlh, NETLINK_ROUTE) < 0)
		goto errout_free;

	link_cache = nltool_alloc_link_cache(nlh);
	if (!link_cache)
		goto errout_close;

	addr_cache = nltool_alloc_addr_cache(nlh);
	if (!addr_cache)
		goto errout_link_cache;

	params.dp_type = nltool_parse_dumptype(argv[1]);
	if (params.dp_type < 0)
		goto errout_addr_cache;
	get_filter(addr, argc, argv, 2, link_cache);

	nl_cache_dump_filter(addr_cache, &params, (struct nl_object *) addr);

	err = 0;

errout_addr_cache:
	nl_cache_free(addr_cache);
errout_link_cache:
	nl_cache_free(link_cache);
errout_close:
	nl_close(nlh);
errout_free:
	rtnl_addr_put(addr);
errout:
	return err;
}
