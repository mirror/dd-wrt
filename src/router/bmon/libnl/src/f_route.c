/*
 * src/f_route.c	Routes Filter
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

static void get_filter(struct rtnl_route *r, int ac, char **av, int idx,
		       struct nl_cache *cache, struct nl_cache *link_cache)
{
	while (ac > idx) {
		if (!strcasecmp(av[idx], "src")) {
			if (ac > ++idx) {
				struct nl_addr *a = nl_addr_parse(av[idx++], AF_UNSPEC);
				if (!a)
					goto err;
				rtnl_route_set_pref_src(r, a);
				nl_addr_put(a);
			}
		} else if (!strcasecmp(av[idx], "via")) {
			if (ac > ++idx) {
				struct nl_addr *a = nl_addr_parse(av[idx++], AF_UNSPEC);
				if (!a)
					goto err;
				rtnl_route_set_gateway(r, a);
				nl_addr_put(a);
			}
		} else if (!strcasecmp(av[idx], "from")) {
			if (ac > ++idx) {
				struct nl_addr *a = nl_addr_parse(av[idx++], AF_UNSPEC);
				if (!a)
					goto err;
				rtnl_route_set_src(r, a);
				nl_addr_put(a);
			}
		} else if (!strcasecmp(av[idx], "tos")) {
			if (ac > ++idx)
				rtnl_route_set_tos(r, strtoul(av[idx++], NULL, 0));
		} else if (!strcasecmp(av[idx], "prio")) {
			if (ac > ++idx)
				rtnl_route_set_prio(r, strtoul(av[idx++], NULL, 0));
		} else if (!strcasecmp(av[idx], "scope")) {
			if (ac > ++idx)
				rtnl_route_set_prio(r, rtnl_str2scope(av[idx++]));
		} else if (!strcasecmp(av[idx], "dev")) {
			if (ac > ++idx) {
				int ifindex = rtnl_link_name2i(link_cache, av[idx++]);
				if (ifindex == RTNL_LINK_NOT_FOUND)
					goto err_notfound;
				rtnl_route_set_oif(r, ifindex);
			}
		} else if (!strcasecmp(av[idx], "table")) {
			if (ac > ++idx)
				rtnl_route_set_table(r, strtoul(av[idx++], NULL, 0));
		} else {
			fprintf(stderr, "What is '%s'?\n", av[idx]);
			exit(1);
		}
	}

	return;

err_notfound:
	fprintf(stderr, "Unable to find device \"%s\"\n", av[idx-1]);
	exit(1);
err:
	fprintf(stderr, "%s\n", nl_geterror());
	exit(1);
}
