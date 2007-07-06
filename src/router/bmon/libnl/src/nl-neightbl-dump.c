/*
 * src/nl-neightbl-dump.c     Dump neighbour tables
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
	"Usage: nl-neightbl-dump <mode> [<filter>]\n"
	"  mode := { brief | detailed | stats | xml }\n"
	"  filter :=\n");
	exit(1);
}

int main(int argc, char *argv[])
{
	int err = -1;
	struct nl_handle *nlh;
	struct nl_cache *ntc, *lc;
	struct nl_dump_params params = {
		.dp_fd = stdout,
		.dp_type = NL_DUMP_BRIEF,
	};

	if (argc < 2)
		print_usage();

	if (nltool_init(argc, argv) < 0)
		return -1;

	nlh = nl_handle_alloc_nondefault(nltool_cbset);
	if (!nlh)
		return -1;

	if (nltool_connect(nlh, NETLINK_ROUTE) < 0)
		goto errout;

	ntc = nltool_alloc_neightbl_cache(nlh);
	if (!ntc)
		goto errout_close;

	lc = nltool_alloc_link_cache(nlh);
	if (!lc)
		goto errout_ntbl_cache;

	params.dp_type = nltool_parse_dumptype(argv[1]);
	if (params.dp_type < 0)
		goto errout_link_cache;

	nl_cache_dump(ntc, &params);
	err = 0;

errout_link_cache:
	nl_cache_free(lc);
errout_ntbl_cache:
	nl_cache_free(ntc);
errout_close:
	nl_close(nlh);
errout:
	nl_handle_destroy(nlh);
	return err;
}
