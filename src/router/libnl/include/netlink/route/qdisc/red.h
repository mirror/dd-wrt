/*
 * netlink/route/sch/red.h	RED Qdisc
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_RED_H_
#define NETLINK_RED_H_

#include <netlink/netlink.h>

extern  void rtnl_red_set_limit(struct rtnl_qdisc *qdisc, int limit);
extern int rtnl_red_get_limit(struct rtnl_qdisc *qdisc);

#endif
