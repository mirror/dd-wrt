/* $Id$ */
/*
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2002-2013 Sourcefire, Inc.
 *
 * Author(s):  Andrew R. Baker <andrewb@sourcefire.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef __IP_ADDR_SET_H__
#define __IP_ADDR_SET_H__

#include <sys/types.h>
#include "sf_types.h"

# include "ipv6_port.h"
# include "sfutil/sf_ipvar.h"



void IpAddrSetDestroy(IpAddrSet *);
struct _SnortConfig;
IpAddrSet *IpAddrSetParse(struct _SnortConfig *, char *);



#endif  /* __IP_ADDR_SET_H__ */
