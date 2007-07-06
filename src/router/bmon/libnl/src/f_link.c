/*
 * src/f_link.c		Link Filter
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

#include <linux/if.h>

static void get_filter(struct rtnl_link *l, int ac, char **av, int idx,
		       struct nl_cache *cache)
{
	while (ac > idx) {
		if (!strcasecmp(av[idx], "dev")) {
			if (ac > ++idx) {
				int ifindex = rtnl_link_name2i(cache, av[idx++]);
				if (ifindex == RTNL_LINK_NOT_FOUND)
					goto err_notfound;
				rtnl_link_set_ifindex(l, ifindex);
			}
		} else if (!strcasecmp(av[idx], "mtu")) {
			if (ac > ++idx)
				rtnl_link_set_mtu(l, strtoul(av[idx++], NULL, 0));
		} else if (!strcasecmp(av[idx], "txqlen")) {
			if (ac > ++idx)
				rtnl_link_set_txqlen(l, strtoul(av[idx++], NULL, 0));
		} else if (!strcasecmp(av[idx], "weight")) {
			if (ac > ++idx)
				rtnl_link_set_weight(l, strtoul(av[idx++], NULL, 0));
		} else if (!strcasecmp(av[idx], "link")) {
			if (ac > ++idx) {
				int ifindex = rtnl_link_name2i(cache, av[idx++]);
				if (ifindex == RTNL_LINK_NOT_FOUND)
					goto err_notfound;
				rtnl_link_set_link(l, ifindex);
			}
		} else if (!strcasecmp(av[idx], "master")) {
			if (ac > ++idx) {
				int ifindex = rtnl_link_name2i(cache, av[idx++]);
				if (ifindex == RTNL_LINK_NOT_FOUND)
					goto err_notfound;
				rtnl_link_set_master(l, ifindex);
			}
		} else if (!strcasecmp(av[idx], "qdisc")) {
			if (ac > ++idx)
				rtnl_link_set_qdisc(l, av[idx++]);
		} else if (!strcasecmp(av[idx], "name")) {
			if (ac > ++idx)
				rtnl_link_set_name(l, av[idx++]);
		} else if (!strcasecmp(av[idx], "addr")) {
			if (ac > ++idx) {
				struct nl_addr *a = nl_addr_parse(av[idx++], AF_UNSPEC);
				if (a == NULL)
					goto err;
				rtnl_link_set_addr(l, a);
				nl_addr_put(a);
			}
		} else if (!strcasecmp(av[idx], "broadcast")) {
			if (ac > ++idx) {
				struct nl_addr *a = nl_addr_parse(av[idx++], AF_UNSPEC);
				if (a == NULL)
					goto err;
				rtnl_link_set_broadcast(l, a);
				nl_addr_put(a);
			}
		}
#define MFLAG(STR, FLAG) \
	else if (!strcasecmp(av[idx], STR)) { \
		rtnl_link_set_flags(l, FLAG); idx++; }
#define MNOFLAG(STR, FLAG) \
	else if (!strcasecmp(av[idx], STR)) { \
		rtnl_link_unset_flags(l, FLAG); idx++; }

		MFLAG("up", IFF_UP)
		MNOFLAG("down", IFF_UP)
		MFLAG("noarp", IFF_NOARP)
		MNOFLAG("arp", IFF_NOARP)
		MFLAG("promisc", IFF_PROMISC)
		MNOFLAG("nopromisc", IFF_PROMISC)
		MFLAG("dynamic", IFF_DYNAMIC)
		MNOFLAG("nodynamic", IFF_DYNAMIC)
		MFLAG("multicast", IFF_MULTICAST)
		MNOFLAG("nomulticast", IFF_MULTICAST)
		MFLAG("allmulticast", IFF_ALLMULTI)
		MNOFLAG("noallmulticast", IFF_ALLMULTI)
#undef MFLAG
#undef MNOFLAG
		else {
			fprintf(stderr, "What is '%s'?\n", av[idx]);
			exit(1);
		}
	}

	return;

err_notfound:
	fprintf(stderr, "Unknown link %s\n", av[idx-1]);
	exit(1);
err:
	fprintf(stderr, "%s\n", nl_geterror());
	exit(1);
}
