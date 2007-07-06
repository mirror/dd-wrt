/*
 * src/nl-addr-delete.c     Delete addresses
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
	struct rtnl_addr *addr;
	struct nl_addr *local;
	int err = 1;

	if (argc < 3 || !strcmp(argv[1], "-h")) {
		printf("Usage: nl-addr-delete <addr> <ifindex>\n");
		goto errout;
	}

	if (nltool_init(argc, argv) < 0)
		goto errout;

	nlh = nl_handle_alloc_nondefault(nltool_cbset);
	if (!nlh)
		goto errout;

	addr = rtnl_addr_alloc();
	if (!addr)
		goto errout_free_handle;

	if (nltool_connect(nlh, NETLINK_ROUTE) < 0)
		goto errout_free_addr;

	local = nltool_addr_parse(argv[1]);
	if (!local)
		goto errout_close;

	if (rtnl_addr_set_local(addr, local) < 0) {
		fprintf(stderr, "Unable to set local address: %s\n",
			nl_geterror());
		goto errout_addr_put;
	}

	rtnl_addr_set_ifindex(addr, strtoul(argv[2], NULL, 0));

	if (rtnl_addr_delete(nlh, addr, 0) < 0) {
		fprintf(stderr, "Unable to delete address: %s\n",
			nl_geterror());
		goto errout_addr_put;
	}

	err = 0;

errout_addr_put:
	nl_addr_put(local);
errout_close:
	nl_close(nlh);
errout_free_addr:
	rtnl_addr_put(addr);
errout_free_handle:
	nl_handle_destroy(nlh);
errout:
	return err;
}
