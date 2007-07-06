/*
 * src/f_addr.c     	Address Filter
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

static void get_filter(struct rtnl_addr *addr, int argc, char **argv, int idx,
		       struct nl_cache *link_cache)
{
	struct nl_addr *a;

	while (argc > idx) {
		if (arg_match("dev")) {
			if (argc > ++idx) {
				int ifindex = rtnl_link_name2i(link_cache, argv[idx++]);
				if (ifindex == RTNL_LINK_NOT_FOUND)
					goto err_notfound;
				rtnl_addr_set_ifindex(addr, ifindex);
			}
		} else if (arg_match("family")) {
			if (argc > ++idx) {
				int family = nl_str2af(argv[idx++]);
				if (family == AF_UNSPEC)
					goto err_invaf;
				rtnl_addr_set_family(addr, family);
			}
		} else if (arg_match("label")) {
			if (argc > ++idx)
				rtnl_addr_set_label(addr, argv[idx++]);
		} else if (arg_match("scope")) {
			if (argc > ++idx) {
				int scope = rtnl_str2scope(argv[idx++]);
				if (scope < 0)
					goto err_invscope;
				rtnl_addr_set_scope(addr, scope);
			}
		} else if (arg_match("local")) {
			if (argc > ++idx) {
				a = nl_addr_parse(argv[idx++],
						  rtnl_addr_get_family(addr));
				if (!a)
					goto err_invaddr;
				rtnl_addr_set_local(addr, a);
				nl_addr_put(a);
			}
		} else if (arg_match("peer")) {
			if (argc > ++idx) {
				a = nl_addr_parse(argv[idx++],
						  rtnl_addr_get_family(addr));
				if (!a)
					goto err_invaddr;
				rtnl_addr_set_peer(addr, a);
				nl_addr_put(a);
			}
		} else if (arg_match("broadcast")) {
			if (argc > ++idx) {
				a = nl_addr_parse(argv[idx++],
						  rtnl_addr_get_family(addr));
				if (!a)
					goto err_invaddr;
				rtnl_addr_set_broadcast(addr, a);
				nl_addr_put(a);
			}
		} else if (arg_match("multicast")) {
			if (argc > ++idx) {
				a = nl_addr_parse(argv[idx++],
						  rtnl_addr_get_family(addr));
				if (!a)
					goto err_invaddr;
				rtnl_addr_set_multicast(addr, a);
				nl_addr_put(a);
			}
		} else if (arg_match("anycast")) {
			if (argc > ++idx) {
				a = nl_addr_parse(argv[idx++],
						  rtnl_addr_get_family(addr));
				if (!a)
					goto err_invaddr;
				rtnl_addr_set_anycast(addr, a);
				nl_addr_put(a);
			}
		} else {
			fprintf(stderr, "What is '%s'?\n", argv[idx]);
			exit(1);
		}
	}

	return;

err_notfound:
	fprintf(stderr, "Unknown link %s\n", argv[idx-1]);
	exit(1);
err_invscope:
	fprintf(stderr, "Invalid scope name \"%s\".\n", argv[idx-1]);
	exit(1);
err_invaf:
	fprintf(stderr, "Invalid address family \"%s\"\n", argv[idx-1]);
	exit(1);
err_invaddr:
	fprintf(stderr, "Invalid address \"%s\": %s\n", argv[idx-1], nl_geterror());
	exit(1);
}
