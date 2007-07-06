/*
 * src/nl-link-name2ifindex.c     Transform a interface name to its index
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

#include "utils.h"

int main(int argc, char *argv[])
{
	struct nl_handle *nlh;
	struct nl_cache *link_cache;
	int err = -1, ifindex;

	if (nltool_init(argc, argv) < 0)
		return -1;

	if (argc < 2 || !strcmp(argv[1], "-h")) {
		printf("Usage: nl-link-name2ifindex <name>\n");
		return -1;
	}

	nlh = nl_handle_alloc_nondefault(nltool_cbset);
	if (!nlh)
		return -1;

	if (nltool_connect(nlh, NETLINK_ROUTE) < 0)
		goto errout;

	link_cache = nltool_alloc_link_cache(nlh);
	if (!link_cache)
		goto errout;

	if ((ifindex = rtnl_link_name2i(link_cache, argv[1])) == RTNL_LINK_NOT_FOUND)
		fprintf(stderr, "Interface %s does not exist\n", argv[1]);
	else
		printf("%d\n", ifindex);

	nl_cache_free(link_cache);
	err = 0;
errout:
	nl_close(nlh);
	nl_handle_destroy(nlh);

	return err;
}
