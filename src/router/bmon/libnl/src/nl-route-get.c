/*
 * src/nl-route-get.c     Get Route Attributes
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

#include "utils.h"

static void print_usage(void)
{
	printf("Usage: nl-route-get <addr>\n");
	exit(1);
}

static int cb(struct nl_msg *msg, void *arg)
{
	nl_cache_parse_and_add(arg, msg);

	return 0;
}

int main(int argc, char *argv[])
{
	struct nl_handle *nlh;
	struct nl_cache *link_cache, *route_cache;
	struct nl_addr *dst;
	struct nl_dump_params params = {
		.dp_fd = stdout,
		.dp_type = NL_DUMP_BRIEF
	};
	int err = 1;

	if (argc < 2 || !strcmp(argv[1], "-h"))
		print_usage();

	if (nltool_init(argc, argv) < 0)
		goto errout;

	nlh = nl_handle_alloc_nondefault(nltool_cbset);
	if (!nlh)
		goto errout;

	if (nltool_connect(nlh, NETLINK_ROUTE) < 0)
		goto errout_free_handle;

	link_cache = nltool_alloc_link_cache(nlh);
	if (!link_cache)
		goto errout_close;

	dst = nltool_addr_parse(argv[1]);
	if (!dst)
		goto errout_link_cache;

	route_cache = nltool_alloc_route_cache(nlh);
	if (!route_cache)
		goto errout_addr_put;

	{
		struct nl_msg *m;
		struct rtmsg rmsg = {
			.rtm_family = nl_addr_get_family(dst),
			.rtm_dst_len = nl_addr_get_prefixlen(dst),
		};

		m = nlmsg_build_simple(RTM_GETROUTE, 0);
		nlmsg_append(m, &rmsg, sizeof(rmsg), 1);
		nla_put_addr(m, RTA_DST, dst);

		if ((err = nl_send_auto_complete(nlh, m)) < 0) {
			nlmsg_free(m);
			fprintf(stderr, "%s\n", nl_geterror());
			goto errout_route_cache;
		}

		nlmsg_free(m);

		nl_cb_set(nl_handle_get_cb(nlh), NL_CB_VALID, NL_CB_CUSTOM,
			  cb, route_cache);

		if (nl_recvmsgs_def(nlh) < 0) {
			fprintf(stderr, "%s\n", nl_geterror());
			goto errout_route_cache;
		}
	}

	nl_cache_dump(route_cache, &params);

	err = 0;
errout_route_cache:
	nl_cache_free(route_cache);
errout_addr_put:
	nl_addr_put(dst);
errout_link_cache:
	nl_cache_free(link_cache);
errout_close:
	nl_close(nlh);
errout_free_handle:
	nl_handle_destroy(nlh);
errout:
	return err;
}
