/*
 * u32.h         libnl u32 classifier
 *
 * $Id: u32.h 28 2004-10-18 22:16:23Z tgr $
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

#ifndef NETLINK_U32_H_
#define NETLINK_U32_H_

#include <netlink/netlink.h>
#include <netlink/filter.h>
#include <netlink/cache.h>

#define U32_HAS_DIVISOR      0x001
#define U32_HAS_HASH         0x002
#define U32_HAS_CLASSID      0x004
#define U32_HAS_LINK         0x008
#define U32_HAS_PCNT         0x010
#define U32_HAS_SELECTOR     0x020
#define U32_HAS_ACTION       0x040
#define U32_HAS_POLICE       0x080
#define U32_HAS_INDEV        0x100

struct rtnl_u32
{
	uint32_t         divisor;
	uint32_t         hash;
	uint32_t         classid;
	uint32_t         link;
	struct nl_data   pcnt;
	struct nl_data   selector;
	struct nl_data   act;
	struct nl_data   police;
	char             indev[IFNAMSIZ];
	int              mask;
};

#endif
