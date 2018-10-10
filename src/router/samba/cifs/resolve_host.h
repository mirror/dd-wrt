/*
 * resolving DNS hostname routine
 *
 * Copyright (C) 2010 Jeff Layton (jlayton@samba.org)
 * Copyright (C) 2010 Igor Druzhinin (jaxbrigs@gmail.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _RESOLVE_HOST_H_
#define _RESOLVE_HOST_H_

#include <arpa/inet.h>

/* currently maximum length of IPv6 address string */
#define MAX_ADDRESS_LEN INET6_ADDRSTRLEN

/* limit list of addresses to 16 max-size addrs */
#define MAX_ADDR_LIST_LEN ((MAX_ADDRESS_LEN + 1) * 16)

extern int resolve_host(const char *host, char *addrstr);

#endif /* _RESOLVE_HOST_H_ */
