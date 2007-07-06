/*
 * netlink/route/sch/htb.h	HTB Qdisc
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 * Copyright (c) 2005 Petr Gotthard <petr.gotthard@siemens.com>
 * Copyright (c) 2005 Siemens AG Oesterreich
 */

#ifndef NETLINK_HTB_H_
#define NETLINK_HTB_H_

#include <netlink/netlink.h>
#include <netlink/route/tc.h>

extern void rtnl_htb_set_rate2quantum(struct rtnl_qdisc *, uint32_t);
extern void rtnl_htb_set_defcls(struct rtnl_qdisc *, uint32_t);

extern void rtnl_htb_set_prio(struct rtnl_class *, uint32_t);
extern void rtnl_htb_set_mtu(struct rtnl_class *, uint32_t);
extern void rtnl_htb_set_rate(struct rtnl_class *, uint32_t);
extern void rtnl_htb_set_ceil(struct rtnl_class *, uint32_t);

#endif
