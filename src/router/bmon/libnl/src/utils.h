/*
 * src/utils.h		Utilities
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

#ifndef __SRC_UTILS_H_
#define __SRC_UTILS_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <errno.h>
#include <stdint.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/addr.h>
#include <netlink/route/rtnl.h>
#include <netlink/route/link.h>
#include <netlink/route/addr.h>
#include <netlink/route/neighbour.h>
#include <netlink/route/neightbl.h>
#include <netlink/route/route.h>
#include <netlink/route/rule.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/class.h>
#include <netlink/route/classifier.h>
#include <netlink/fib_lookup/lookup.h>
#include <netlink/fib_lookup/request.h>

extern int nltool_init(int argc, char *argv[]);
extern int nltool_connect(struct nl_handle *nlh, int protocol);
extern struct nl_addr *nltool_addr_parse(const char *str);
extern int nltool_parse_dumptype(const char *str);

extern struct nl_cache *nltool_alloc_link_cache(struct nl_handle *nlh);
extern struct nl_cache *nltool_alloc_addr_cache(struct nl_handle *nlh);
extern struct nl_cache *nltool_alloc_neigh_cache(struct nl_handle *nlh);
extern struct nl_cache *nltool_alloc_neightbl_cache(struct nl_handle *nlh);
extern struct nl_cache *nltool_alloc_route_cache(struct nl_handle *nlh);
extern struct nl_cache *nltool_alloc_rule_cache(struct nl_handle *nlh);
extern struct nl_cache *nltool_alloc_qdisc_cache(struct nl_handle *nlh);

extern int nltool_cbset;

#define arg_match(str) !strcasecmp(argv[idx], str)

#endif
