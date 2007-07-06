/*
 * src/nl-qdisc-dump.c     Dump qdisc attributes
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
"Usage: nl-qdisc-dump <mode>\n"
"  mode := { brief | detailed | stats | xml }\n");
	exit(1);
}

int main(int argc, char *argv[])
{
	struct nl_handle *nlh;
	struct nl_cache *link_cache, *qdisc_cache;
	struct rtnl_qdisc *qdisc;
	struct nl_dump_params params = {
		.dp_fd = stdout,
		.dp_type = NL_DUMP_BRIEF
	};
	int err = 1;

	if (nltool_init(argc, argv) < 0)
		return -1;

	if (argc < 2 || !strcmp(argv[1], "-h"))
		print_usage();

	params.dp_type = nltool_parse_dumptype(argv[1]);
	if (params.dp_type < 0)
		return -1;

	nlh = nl_handle_alloc_nondefault(nltool_cbset);
	if (!nlh)
		return -1;

	qdisc = rtnl_qdisc_alloc();
	if (!qdisc)
		goto errout_no_put;

	if (nltool_connect(nlh, NETLINK_ROUTE) < 0)
		goto errout;

	link_cache = nltool_alloc_link_cache(nlh);
	if (!link_cache)
		goto errout;

	qdisc_cache = nltool_alloc_qdisc_cache(nlh);
	if (!qdisc_cache)
		goto errout_link_cache;

	nl_cache_dump_filter(qdisc_cache, &params, (struct nl_object *) qdisc);
	nl_cache_free(qdisc_cache);
	err = 0;

errout_link_cache:
	nl_cache_free(link_cache);
errout:
	rtnl_qdisc_put(qdisc);
errout_no_put:
	nl_close(nlh);
	nl_handle_destroy(nlh);
	return err;
}
