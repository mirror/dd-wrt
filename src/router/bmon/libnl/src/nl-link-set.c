/*
 * src/nl-link-set.c     Set link attributes
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
	printf(
	"Usage: nl-link-set <ifindex> <changes>\n"
	"  changes := [dev DEV] [mtu MTU] [txqlen TXQLEN] [weight WEIGHT] [link LINK]\n"
	"             [master MASTER] [qdisc QDISC] [addr ADDR] [broadcast BRD]\n"
	"             [{ up | down }] [{ arp | noarp }] [{ promisc | nopromisc }]\n"
	"             [{ dynamic | nodynamic }] [{ multicast | nomulticast }]\n"
	"             [{ trailers | notrailers }] [{ allmulticast | noallmulticast }]\n");
	exit(1);
}

#include "f_link.c"

int main(int argc, char *argv[])
{
	struct nl_handle *nlh;
	struct nl_cache *link_cache;
	struct rtnl_link *link, *orig;
	int err = 1, ifindex;

	if (nltool_init(argc, argv) < 0)
		return -1;

	if (argc < 2 || !strcmp(argv[1], "-h"))
		print_usage();

	nlh = nl_handle_alloc_nondefault(nltool_cbset);
	if (!nlh)
		return -1;

	link = rtnl_link_alloc();
	if (!link)
		goto errout;

	if (nltool_connect(nlh, NETLINK_ROUTE) < 0)
		goto errout_free;

	link_cache = nltool_alloc_link_cache(nlh);
	if (!link_cache)
		goto errout_close;

	ifindex = strtoul(argv[1], NULL, 0);

	if (!(orig = rtnl_link_get(link_cache, ifindex))) {
		fprintf(stderr, "Interface index %d does not exist\n", ifindex);
		goto errout_cache;
	}

	get_filter(link, argc, argv, 2, link_cache);

	if (rtnl_link_change(nlh, orig, link, 0) < 0) {
		fprintf(stderr, "Unable to change link: %s\n", nl_geterror());
		goto errout_put;
	}

	err = 0;

errout_put:
	rtnl_link_put(orig);
errout_cache:
	nl_cache_free(link_cache);
errout_close:
	nl_close(nlh);
errout_free:
	rtnl_link_put(link);
errout:
	nl_handle_destroy(nlh);
	return err;
}
