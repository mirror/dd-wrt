/*
 * cbq.h          libnl cbq module
 *
 * $Id: cbq.h 28 2004-10-18 22:16:23Z tgr $
 *
 * Copyright (c) 2003-2004 Thomas Graf <tgraf@suug.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */

#ifndef NETLINK_CBQ_H_
#define NETLINK_CBQ_H_

#include <netlink/netlink.h>
#include <netlink/qdisc.h>
#include <netlink/cache.h>

struct rtnl_cbq
{
	struct tc_cbq_lssopt    cbq_lss;
	struct tc_ratespec      cbq_rate;
	struct tc_cbq_wrropt    cbq_wrr;
	struct tc_cbq_ovl       cbq_ovl;
	struct tc_cbq_fopt      cbq_fopt;
	struct tc_cbq_police    cbq_police;
};

extern char * nl_ovl_strategy2str_r(int, char *, size_t);
extern char * nl_ovl_strategy2str(int);
extern int    nl_str2ovl_strategy(const char *);


#endif
