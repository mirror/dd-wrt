/* Housekeeping IPv4/IPv6 wrapper functions
 *
 * Copyright (C) 2017-2021  Joachim Wiberg <troglobit@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef SMCROUTE_INET_H_
#define SMCROUTE_INET_H_

#include "config.h"

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>		/* inet_ntop() */
#include <netinet/in.h>
#include <sys/socket.h>

#ifdef  HAVE_IPV6_MULTICAST_HOST
#define INET_ADDRSTR_LEN  INET6_ADDRSTRLEN
#else
#define INET_ADDRSTR_LEN  INET_ADDRSTRLEN
#endif
typedef struct sockaddr_storage inet_addr_t;

#ifndef s6_addr32
# if   defined(__FreeBSD__)
#  define s6_addr32 __u6_addr.__u6_addr32
# elif defined(__linux__)
#  define s6_addr32 __in6_u.__u6_addr32
# else
#  error "IPv6 s6_addr32 is not defined, unknown operating system, build --without-ipv6"
# endif
#endif

struct inet_iter {
	inet_addr_t orig;
	int         len;

	inet_addr_t addr;
	uint32_t    num;
};

void                 inet_addr_set  (inet_addr_t *addr, const struct in_addr *ina);
struct in_addr      *inet_addr_get  (inet_addr_t *addr);

#ifdef HAVE_IPV6_MULTICAST_HOST
void                 inet_addr6_set (inet_addr_t *addr, const struct in6_addr *ina);
struct sockaddr_in6 *inet_addr6_get (inet_addr_t *addr);
#endif

void                 inet_anyaddr   (sa_family_t family, inet_addr_t *addr);
inet_addr_t          inet_netaddr   (inet_addr_t *addr, int len);

int                  inet_addr_cmp  (inet_addr_t *a, inet_addr_t *b);

const char          *inet_addr2str  (inet_addr_t *addr, char *str, size_t len);
int                  inet_str2addr  (const char *str, inet_addr_t *addr);

int                  is_multicast   (inet_addr_t *addr);
int                  is_anyaddr     (inet_addr_t *addr);

int                  inet_iter_init (struct inet_iter *iter, inet_addr_t *addr, int len);
int                  inet_iterator  (struct inet_iter *iter, inet_addr_t *addr);

static inline int    inet_max_len   (inet_addr_t *addr)
{
#ifdef HAVE_IPV6_MULTICAST_HOST
	if (addr->ss_family == AF_INET6)
		return 128;
#endif
	return 32;
}

#endif /* SMCROUTE_INET_H_ */
