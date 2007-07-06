/*
 * src/nl-link-ifindex2name.c     Transform a interface index to its name
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

#include "utils.h"

int main(int argc, char **argv)
{
	struct nl_handle *nlh;
	struct nl_cache *link_cache;
	int err = -1, ifindex;
	char dst[32] = {0};
	const char *name;

	if (nltool_init(argc, argv) < 0)
		return -1;

	if (argc < 2 || !strcmp(argv[1], "-h")) {
		fprintf(stderr, "Usage: nl-link-ifindex2name <ifindex>\n");
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

	ifindex = strtoul(argv[1], NULL, 0);

	if (!(name = rtnl_link_i2name(link_cache, ifindex, dst, sizeof(dst))))
		fprintf(stderr, "Interface index %d does not exist\n", ifindex);
	else
		printf("%s\n", name);

	nl_cache_free(link_cache);
	err = 0;
errout:
	nl_close(nlh);
	nl_handle_destroy(nlh);

	return err;
}
