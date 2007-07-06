/*
 * src/nl-monitor.c     Monitor events
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

#include "utils.h"
#include <netlink/route/link.h>
#include <netlink/route/addr.h>

static void obj_input(struct nl_object *obj, void *arg)
{
	struct nl_dump_params dp = {
		.dp_type = NL_DUMP_STATS,
		.dp_fd = stdout,
		.dp_dump_msgtype = 1,
	};

	nl_object_dump(obj, &dp);
}

static int event_input(struct nl_msg *msg, void *arg)
{
	if (nl_msg_parse(msg, &obj_input, NULL) < 0)
		fprintf(stderr, "<<EVENT>> Unknown message type\n");

	/* Exit nl_recvmsgs_def() and return to the main select() */
	return NL_EXIT;
}

int main(int argc, char *argv[])
{
	struct nl_handle *nlh;
	struct nl_cache *link_cache;
	int err = 1;
	int i, idx;

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
		{ RTNLGRP_FIB_MAGIC, "fib-magic" },
		{ RTNLGRP_NONE, NULL }
	};

	if (nltool_init(argc, argv) < 0)
		return -1;

	nlh = nl_handle_alloc_nondefault(nltool_cbset);
	if (nlh == NULL)
		return -1;

	nl_disable_sequence_check(nlh);

	nl_cb_set(nl_handle_get_cb(nlh), NL_CB_VALID, NL_CB_CUSTOM,
		  event_input, NULL);

	if (argc > 1 && !strcasecmp(argv[1], "-h")) {
		printf("Usage: nl-monitor [<groups>]\n");

		printf("Known groups:");
		for (i = 0; known_groups[i].gr_id != RTNLGRP_NONE; i++)
			printf(" %s", known_groups[i].gr_name);
		printf("\n");
		return 2;
	}

	nl_join_groups(nlh, RTMGRP_LINK);

	if (nl_connect(nlh, NETLINK_ROUTE) < 0) {
		fprintf(stderr, "%s\n", nl_geterror());
		goto errout;
	}

	for (idx = 1; argc > idx; idx++) {
		for (i = 0; known_groups[i].gr_id != RTNLGRP_NONE; i++) {
			if (!strcmp(argv[idx], known_groups[i].gr_name)) {

				if (nl_join_group(nlh, known_groups[i].gr_id) < 0) {
					fprintf(stderr, "%s: %s\n", argv[idx], nl_geterror());
					goto errout;
				}

				break;
			}
		}
		if (known_groups[i].gr_id == RTNLGRP_NONE)
			fprintf(stderr, "Warning: Unknown group: %s\n", argv[idx]);
	}

	if ((link_cache = rtnl_link_alloc_cache(nlh)) == NULL) {
		fprintf(stderr, "%s\n", nl_geterror());
		goto errout_close;
	}

	nl_cache_mngt_provide(link_cache);

	while (1) {
		fd_set rfds;
		int fd, retval;

		fd = nl_handle_get_fd(nlh);

		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		/* wait for an incoming message on the netlink socket */
		retval = select(fd+1, &rfds, NULL, NULL, NULL);

		if (retval) {
			/* FD_ISSET(fd, &rfds) will be true */
			nl_recvmsgs_def(nlh);
		}
	}

	nl_cache_free(link_cache);
errout_close:
	nl_close(nlh);
errout:
	return err;
}
