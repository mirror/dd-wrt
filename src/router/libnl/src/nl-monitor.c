/* SPDX-License-Identifier: LGPL-2.1-only */
/*
 * Copyright (c) 2003-2009 Thomas Graf <tgraf@suug.ch>
 */

#include "nl-default.h"

#include <linux/rtnetlink.h>

#include <netlink/cli/utils.h>
#include <netlink/cli/link.h>
#include <netlink/cli/mdb.h>

static const struct {
	enum rtnetlink_groups gr_id;
	const char* gr_name;
} known_groups[] = {
	{ RTNLGRP_LINK, "link" },
	{ RTNLGRP_NOTIFY, "notify" },
	{ RTNLGRP_NEIGH, "neigh" },
	{ RTNLGRP_TC, "tc" },
	{ RTNLGRP_IPV4_IFADDR, "ipv4-ifaddr" },
	{ RTNLGRP_IPV4_MROUTE, "ipv4-mroute" },
	{ RTNLGRP_IPV4_ROUTE, "ipv4-route" },
	{ RTNLGRP_IPV6_IFADDR, "ipv6-ifaddr" },
	{ RTNLGRP_IPV6_MROUTE, "ipv6-mroute" },
	{ RTNLGRP_IPV6_ROUTE, "ipv6-route" },
	{ RTNLGRP_IPV6_IFINFO, "ipv6-ifinfo" },
	{ RTNLGRP_DECnet_IFADDR, "decnet-ifaddr" },
	{ RTNLGRP_DECnet_ROUTE, "decnet-route" },
	{ RTNLGRP_IPV6_PREFIX, "ipv6-prefix" },
	{ RTNLGRP_IPV4_NETCONF, "ipv4-netconf" },
	{ RTNLGRP_IPV6_NETCONF, "ipv6-netconf" },
	{ RTNLGRP_MPLS_NETCONF, "mpls-netconf" },
	{ RTNLGRP_MDB, "mdb" },
	{ RTNLGRP_NONE, NULL }
};

static void obj_input(struct nl_object *obj, void *arg)
{
	nl_object_dump(obj, arg);
}

static int event_input(struct nl_msg *msg, void *arg)
{
	if (nl_msg_parse(msg, &obj_input, arg) < 0)
		fprintf(stderr, "<<EVENT>> Unknown message type\n");

	/* Exit nl_recvmsgs_def() and return to the main select() */
	return NL_STOP;
}

static void print_usage(void)
{
	int i;

        printf(
	"Usage: nl-monitor [OPTION] [<groups>]\n"
	"\n"
	"Options\n"
	" -d, --debug=LEVEL     Set libnl debug level { 0 - 7 }\n"
	" -f, --format=TYPE     Output format { brief | details | stats }\n"
	" -h, --help            Show this help.\n"
	"\n"
        );
	printf("Known groups:");
	for (i = 0; known_groups[i].gr_id != RTNLGRP_NONE; i++)
		printf(" %s", known_groups[i].gr_name);
	printf("\n");
        exit(0);
}

int main(int argc, char *argv[])
{
	struct nl_dump_params dp = {
		.dp_type = NL_DUMP_STATS,
		.dp_fd = stdout,
		.dp_dump_msgtype = 1,
	};

	struct nl_sock *sock;
	int err = 1;
	int i, idx;

	sock = nl_cli_alloc_socket();
	nl_socket_disable_seq_check(sock);
	nl_socket_modify_cb(sock, NL_CB_VALID, NL_CB_CUSTOM, event_input, &dp);

	for (;;) {
		int c, optidx = 0;
		static struct option long_opts[] = {
			{ "debug",  1, 0, 'd' },
			{ "format", 1, 0, 'f' },
			{ "help",   0, 0, 'h' },
			{ 0, 0, 0, 0 }
		};

		c = getopt_long(argc, argv, "d:f:h", long_opts, &optidx);
		if (c == -1)
                        break;

                switch (c) {
		case 'd':
			nl_debug = atoi(optarg);
			break;
                case 'f':
			dp.dp_type = nl_cli_parse_dumptype(optarg);
			break;
		default:
			print_usage();
			break;
		}
	}

	nl_cli_connect(sock, NETLINK_ROUTE);

	for (idx = optind; argc > idx; idx++) {
		for (i = 0; known_groups[i].gr_id != RTNLGRP_NONE; i++) {
			if (!strcmp(argv[idx], known_groups[i].gr_name)) {

				if ((err = nl_socket_add_membership(sock, known_groups[i].gr_id)) < 0) {
					nl_cli_fatal(err, "%s: %s\n", argv[idx],
						     nl_geterror(err));
				}

				break;
			}
		}
		if (known_groups[i].gr_id == RTNLGRP_NONE)
			fprintf(stderr, "Warning: Unknown group: %s\n", argv[idx]);
	}

	nl_cli_link_alloc_cache(sock);

	while (1) {
		fd_set rfds;
		int fd, retval;

		fd = nl_socket_get_fd(sock);

		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		/* wait for an incoming message on the netlink socket */
		retval = select(fd+1, &rfds, NULL, NULL, NULL);

		if (retval) {
			/* FD_ISSET(fd, &rfds) will be true */
			nl_recvmsgs_default(sock);
		}
	}

	return 0;
}
