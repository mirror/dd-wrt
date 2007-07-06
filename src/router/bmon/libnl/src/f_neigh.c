/*
 * src/f_neigh.c	Neighbour Filter
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

static void get_filter(struct rtnl_neigh *n, int ac, char **av, int idx,
		       struct nl_cache *cache)
{
	struct nl_cache *lc = nl_cache_mngt_require("route/link");
	
	while (ac > idx) {
		if (!strcasecmp(av[idx], "dev")) {
			if (ac > ++idx) {
				int ifindex = rtnl_link_name2i(lc, av[idx++]);
				if (ifindex == RTNL_LINK_NOT_FOUND)
					goto err_notfound;
				rtnl_neigh_set_ifindex(n, ifindex);
			}
		} else if (!strcasecmp(av[idx], "dst")) {
			if (ac > ++idx) {
				struct nl_addr *a = nl_addr_parse(av[idx++], AF_UNSPEC);
				if (a == NULL)
					goto err;
				rtnl_neigh_set_dst(n, a);
				nl_addr_put(a);
			}
		} else if (!strcasecmp(av[idx], "lladdr")) {
			if (ac > ++idx) {
				struct nl_addr *a = nl_addr_parse(av[idx++], AF_UNSPEC);
				if (a == NULL)
					goto err;
				rtnl_neigh_set_lladdr(n, a);
				nl_addr_put(a);
			}
		}
	}

	return;
err_notfound:
	fprintf(stderr, "Unable to find interface %s\n", av[idx-1]);
	exit(1);
err:
	fprintf(stderr, "%s\n", nl_geterror());
	exit(1);
}
