/* SPDX-License-Identifier: LGPL-2.1-only */
/*
 * Copyright (c) 2003-2009 Thomas Graf <tgraf@suug.ch>
 */

#include "nl-default.h"

#include <linux/netlink.h>

#include <netlink/cli/utils.h>
#include <netlink/cli/link.h>

static void print_usage(void)
{
	printf("Usage: nl-link-ifindex2name <ifindex>\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	struct nl_sock *sock;
	struct nl_cache *link_cache;
	char name[IFNAMSIZ];
	uint32_t ifindex;

	if (argc < 2)
		print_usage();

	sock = nl_cli_alloc_socket();
	nl_cli_connect(sock, NETLINK_ROUTE);
	link_cache = nl_cli_link_alloc_cache(sock);

	ifindex = nl_cli_parse_u32(argv[1]);

	if (!rtnl_link_i2name(link_cache, ifindex, name, sizeof(name)))
		nl_cli_fatal(ENOENT, "Interface index %d does not exist",
			     ifindex);

	printf("%s\n", name);

	return 0;
}
