/*
 * src/nl-neigh-delete.c     Delete a neighbour
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
	struct rtnl_neigh *neigh;
	struct nl_addr *addr;
	int err = 1;

	if (nltool_init(argc, argv) < 0)
		return -1;

	if (argc < 3 || !strcmp(argv[1], "-h")) {
		printf("Usage: nl-neigh-delete <addr> <ifindex>\n");
		return 2;
	}

	nlh = nl_handle_alloc_nondefault(nltool_cbset);

	neigh = rtnl_neigh_alloc();
	if (neigh == NULL)
		goto errout;

	if (nl_connect(nlh, NETLINK_ROUTE) < 0) {
		fprintf(stderr, "%s\n", nl_geterror());
		goto errout_free;
	}

	addr = nl_addr_parse(argv[1], AF_UNSPEC);
	if (addr == NULL) {
		fprintf(stderr, "Invalid address \"%s\"\n", argv[1]);
		goto errout_close;
	}
	rtnl_neigh_set_dst(neigh, addr);
	nl_addr_put(addr);

	rtnl_neigh_set_ifindex(neigh, strtoul(argv[2], NULL, 0));

	if (rtnl_neigh_delete(nlh, neigh, 0) < 0) {
		fprintf(stderr, "%s\n", nl_geterror());
		goto errout_close;
	}

	err = 0;

errout_close:
	nl_close(nlh);
errout_free:
	rtnl_neigh_put(neigh);
errout:
	nl_handle_destroy(nlh);
	return err;
}
