/* SPDX-License-Identifier: LGPL-2.1-only */
/*
 * Copyright (c) 2022 Stanislav Zaikin <zstaseg@gmail.com>
 */

/**
 * @ingroup cli
 * @defgroup cli_nh nhs
 *
 * @{
 */

#include "nl-default.h"

#include <linux/if.h>

#include <netlink/cli/utils.h>
#include <netlink/cli/nh.h>
#include <netlink/route/nh.h>

struct rtnl_nh *nl_cli_nh_alloc(void)
{
	struct rtnl_nh *nh;

	nh = rtnl_nh_alloc();
	if (!nh)
		nl_cli_fatal(ENOMEM, "Unable to allocate nh object");

	return nh;
}

struct nl_cache *nl_cli_nh_alloc_cache_family_flags(struct nl_sock *sock,
						    int family,
						    unsigned int flags)
{
	struct nl_cache *cache;
	int err;

	if ((err = rtnl_nh_alloc_cache(sock, family, &cache)) < 0)
		nl_cli_fatal(err, "Unable to allocate nh cache: %s",
			     nl_geterror(err));

	nl_cache_mngt_provide(cache);

	return cache;
}

struct nl_cache *nl_cli_nh_alloc_cache_family(struct nl_sock *sock, int family)
{
	return nl_cli_nh_alloc_cache_family_flags(sock, family, 0);
}

struct nl_cache *nl_cli_nh_alloc_cache(struct nl_sock *sock)
{
	return nl_cli_nh_alloc_cache_family(sock, AF_UNSPEC);
}

struct nl_cache *nl_cli_nh_alloc_cache_flags(struct nl_sock *sock,
					     unsigned int flags)
{
	return nl_cli_nh_alloc_cache_family_flags(sock, AF_UNSPEC, flags);
}

/** @} */
