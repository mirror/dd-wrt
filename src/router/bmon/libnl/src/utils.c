/*
 * src/utils.c		Utilities
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

#include "utils.h"

#include <stdlib.h>

int nltool_cbset = NL_CB_VERBOSE;

int nltool_init(int argc, char *argv[])
{
	char *nlcb = getenv("NLCB");
	char *nldbg = getenv("NLDBG");
	
	if (nlcb) {
		if (!strcasecmp(nlcb, "default"))
			nltool_cbset = NL_CB_DEFAULT;
		else if (!strcasecmp(nlcb, "verbose"))
			nltool_cbset = NL_CB_VERBOSE;
		else if (!strcasecmp(nlcb, "debug"))
			nltool_cbset = NL_CB_DEBUG;
		else {
			fprintf(stderr, "Unknown value for NLCB, valid values: "
				"{default | verbose | debug}\n");
			goto errout;
		}
	}

	if (nldbg) {
		long dbg = strtol(nldbg, NULL, 0);

		if (dbg == LONG_MIN || dbg == LONG_MAX) {
			fprintf(stderr, "Invalid value for NLDBG.\n");
			goto errout;
		}

		nl_debug = dbg;
	}
	
	return 0;

errout:
	return -1;
}

int nltool_connect(struct nl_handle *nlh, int protocol)
{
	int err;

	err = nl_connect(nlh, protocol);
	if (err < 0)
		fprintf(stderr, "Unable to connect netlink socket%s\n",
			nl_geterror());

	return err;
}

struct nl_addr *nltool_addr_parse(const char *str)
{
	struct nl_addr *addr;

	addr = nl_addr_parse(str, AF_UNSPEC);
	if (!addr)
		fprintf(stderr, "Unable to parse address \"%s\": %s\n",
			str, nl_geterror());

	return addr;
}

int nltool_parse_dumptype(const char *str)
{
	if (!strcasecmp(str, "brief"))
		return NL_DUMP_BRIEF;
	else if (!strcasecmp(str, "detailed"))
		return NL_DUMP_FULL;
	else if (!strcasecmp(str, "stats"))
		return NL_DUMP_STATS;
	else if (!strcasecmp(str, "xml"))
		return NL_DUMP_XML;
	else if (!strcasecmp(str, "env"))
		return NL_DUMP_ENV;
	else {
		fprintf(stderr, "Invalid dump type \"%s\".\n", str);
		return -1;
	}
}

struct nl_cache *nltool_alloc_link_cache(struct nl_handle *nlh)
{
	struct nl_cache *cache;

	cache = rtnl_link_alloc_cache(nlh);
	if (!cache)
		fprintf(stderr, "Unable to retrieve link cache: %s\n",
			nl_geterror());
	else
		nl_cache_mngt_provide(cache);

	return cache;
}

struct nl_cache *nltool_alloc_addr_cache(struct nl_handle *nlh)
{
	struct nl_cache *cache;

	cache = rtnl_addr_alloc_cache(nlh);
	if (!cache)
		fprintf(stderr, "Unable to retrieve address cache: %s\n",
			nl_geterror());
	else
		nl_cache_mngt_provide(cache);

	return cache;
}

struct nl_cache *nltool_alloc_neigh_cache(struct nl_handle *nlh)
{
	struct nl_cache *cache;

	cache = rtnl_neigh_alloc_cache(nlh);
	if (!cache)
		fprintf(stderr, "Unable to retrieve neighbour cache: %s\n",
			nl_geterror());
	else
		nl_cache_mngt_provide(cache);

	return cache;
}

struct nl_cache *nltool_alloc_neightbl_cache(struct nl_handle *nlh)
{
	struct nl_cache *cache;

	cache = rtnl_neightbl_alloc_cache(nlh);
	if (!cache)
		fprintf(stderr, "Unable to retrieve neighbour table "
				"cache: %s\n", nl_geterror());
	else
		nl_cache_mngt_provide(cache);

	return cache;
}

struct nl_cache *nltool_alloc_route_cache(struct nl_handle *nlh)
{
	struct nl_cache *cache;

	cache = rtnl_route_alloc_cache(nlh);
	if (!cache)
		fprintf(stderr, "Unable to retrieve route cache: %s\n",
			nl_geterror());
	else
		nl_cache_mngt_provide(cache);

	return cache;
}

struct nl_cache *nltool_alloc_rule_cache(struct nl_handle *nlh)
{
	struct nl_cache *cache;

	cache = rtnl_rule_alloc_cache(nlh);
	if (!cache)
		fprintf(stderr, "Unable to retrieve rule cache: %s\n",
			nl_geterror());
	else
		nl_cache_mngt_provide(cache);

	return cache;
}

struct nl_cache *nltool_alloc_qdisc_cache(struct nl_handle *nlh)
{
	struct nl_cache *cache;

	cache = rtnl_qdisc_alloc_cache(nlh);
	if (!cache)
		fprintf(stderr, "Unable to retrieve qdisc cache: %s\n",
			nl_geterror());
	else
		nl_cache_mngt_provide(cache);

	return cache;
}
