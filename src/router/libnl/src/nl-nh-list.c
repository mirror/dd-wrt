/* SPDX-License-Identifier: LGPL-2.1-only */
/*
 * Copyright (c) 2022 Stanislav Zaikin <zstaseg@gmail.com>
 */

#include "nl-default.h"

#include <linux/netlink.h>

#include <netlink/cli/utils.h>
#include <netlink/cli/nh.h>
#include <netlink/route/nh.h>

static void print_usage(void)
{
	printf("Usage: nl-nh-list [OPTIONS]... \n"
	       "\n"
	       "OPTIONS\n"
	       "     --details             Show detailed information of each link\n"
	       " -h, --help                Show this help text.\n"
	       " -v, --version             Show versioning information.\n"
	       "\n"
	       " -n, --name=NAME	    Name of link\n"
	       " -i, --index               Interface index (unique identifier)\n"
	       "     --family=NAME         Link address family\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	struct nl_sock *sock;
	struct nl_cache *link_cache;
	struct nl_dump_params params = {
		.dp_type = NL_DUMP_LINE,
		.dp_fd = stdout,
	};

	sock = nl_cli_alloc_socket();
	nl_cli_connect(sock, NETLINK_ROUTE);

	for (;;) {
		int c, optidx = 0;
		enum {
			ARG_FAMILY = 257,
			ARG_DETAILS,
		};
		static struct option long_opts[] = { { "details", 0, 0,
						       ARG_DETAILS },
						     { "help", 0, 0, 'h' },
						     { "version", 0, 0, 'v' },
						     { "name", 1, 0, 'n' },
						     { 0, 0, 0, 0 } };

		c = getopt_long(argc, argv, "hvn:i:", long_opts, &optidx);
		if (c == -1)
			break;

		switch (c) {
		case ARG_DETAILS:
			params.dp_type = NL_DUMP_DETAILS;
			break;
		case 'h':
			print_usage();
			break;
		case 'v':
			nl_cli_print_version();
			break;
		}
	}

	link_cache = nl_cli_nh_alloc_cache(sock);

	nl_cache_dump(link_cache, &params);

	return 0;
}
