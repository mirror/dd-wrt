/* SPDX-License-Identifier: LGPL-2.1-only */
/*
 * Copyright (c) 2010-2011 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup cli
 * @defgroup cli_class Traffic Classes
 * @{
 */

#include "nl-default.h"

#include <netlink/cli/utils.h>
#include <netlink/cli/class.h>

struct rtnl_class *nl_cli_class_alloc(void)
{
	struct rtnl_class *class;

	if (!(class = rtnl_class_alloc()))
		nl_cli_fatal(ENOMEM, "Unable to allocate class object");

	return class;
}

struct nl_cache *nl_cli_class_alloc_cache(struct nl_sock *sock, int ifindex)
{
	struct nl_cache *cache;
	int err;

	if ((err = rtnl_class_alloc_cache(sock, ifindex, &cache)) < 0)
		nl_cli_fatal(err, "Unable to allocate class cache: %s",
			     nl_geterror(err));

	nl_cache_mngt_provide(cache);

	return cache;
}

/** @} */
