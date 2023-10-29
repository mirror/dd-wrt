/* SPDX-License-Identifier: LGPL-2.1-only */
/*
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 */

#include "nl-default.h"

#include <linux/netlink.h>

#include <netlink/cli/utils.h>
#include <netlink/cli/link.h>

static void print_usage(void)
{
	printf("Usage: nl-link-name2ifindex <name>\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	struct nl_sock *sock;
	struct nl_cache *link_cache;
	uint32_t ifindex;

	if (argc < 2)
		print_usage();

	sock = nl_cli_alloc_socket();
	nl_cli_connect(sock, NETLINK_ROUTE);
	link_cache = nl_cli_link_alloc_cache(sock);

	if (!(ifindex = rtnl_link_name2i(link_cache, argv[1])))
		nl_cli_fatal(ENOENT, "Interface \"%s\" does not exist",
			     argv[1]);

	printf("%u\n", ifindex);

	return 0;
}
