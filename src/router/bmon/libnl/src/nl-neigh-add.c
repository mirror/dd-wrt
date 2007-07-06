/*
 * src/ nl-neigh-add.c     Add a neighbour
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

	if (argc < 4 || !strcmp(argv[1], "-h")) {
		printf("Usage: nl-neigh-add <addr> <lladdr> "
		       "<ifindex> [<state>]\n");
		return 1;
	}

	nlh = nl_handle_alloc_nondefault(nltool_cbset);
	if (!nlh)
		return -1;

	neigh = rtnl_neigh_alloc();
	if (!neigh)
		goto errout;

	if (nltool_connect(nlh, NETLINK_ROUTE) < 0)
		goto errout_free;

	addr = nltool_addr_parse(argv[1]);
	if (!addr)
		goto errout_close;
	rtnl_neigh_set_dst(neigh, addr);
	nl_addr_put(addr);

	addr = nltool_addr_parse(argv[2]);
	if (!addr)
		goto errout_close;
	rtnl_neigh_set_lladdr(neigh, addr);
	nl_addr_put(addr);

	rtnl_neigh_set_ifindex(neigh, strtoul(argv[3], NULL, 0));

	if (argc > 4) {
		int state = rtnl_neigh_str2state(argv[4]);
		if (state < 0) {
			fprintf(stderr, "Unknown state \"%s\"\n", argv[4]);
			goto errout_close;
		}
		rtnl_neigh_set_state(neigh, state);
	} else
		rtnl_neigh_set_state(neigh, NUD_PERMANENT);

	if (rtnl_neigh_add(nlh, neigh, 0) < 0) {
		fprintf(stderr, "Unable to add address: %s\n", nl_geterror());
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
