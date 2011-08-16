/****************************************************************************
 *
 * Copyright (C) 2003-2011 Sourcefire, Inc.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 ****************************************************************************/
 
/**
 * @file   util_net.h
 * @author Chris Green <cmg@sourcefire.com>
 * @date   Fri Jun 27 10:20:31 2003
 * 
 * @brief  simple network related functions
 * 
 * Put your simple network related functions here
 */

#ifndef _UTIL_NET_H
#define _UTIL_NET_H

#include "sf_types.h"
#include "ipv6_port.h"

#ifndef SUP_IP6
char *inet_ntoax(const struct in_addr);
#else
char *inet_ntoax(sfip_t *);
#endif

char * mktcpflag_str(int flags);

#endif /* _UTIL_NET_H */
