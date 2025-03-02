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

#include "inet.h"

void inet_addr_set(inet_addr_t *addr, const struct in_addr *ina)
{
	struct sockaddr_in *sin = (struct sockaddr_in *)addr;

	assert(addr && ina);
	sin->sin_family = AF_INET;
#ifdef HAVE_SOCKADDR_IN_SIN_LEN
	sin->sin_len = sizeof(struct sockaddr_in);
#endif
	sin->sin_addr = *ina;
}

struct in_addr *inet_addr_get(inet_addr_t *addr)
{
	struct sockaddr_in *sin = (struct sockaddr_in *)addr;

	assert(addr);
	assert(sin->sin_family == AF_INET);

	return &sin->sin_addr;
}

#ifdef HAVE_IPV6_MULTICAST_HOST
void inet_addr6_set(inet_addr_t *addr, const struct in6_addr *ina)
{
	struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)addr;

	assert(addr && ina);
	sin6->sin6_family = AF_INET6;
#ifdef HAVE_SOCKADDR_IN_SIN_LEN
	sin6->sin6_len = sizeof(struct sockaddr_in6);
#endif
	sin6->sin6_addr = *ina;
}

struct sockaddr_in6 *inet_addr6_get(inet_addr_t *addr)
{
	struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)addr;

	assert(addr);
	assert(sin6->sin6_family == AF_INET6);

	return sin6;
}
#endif /* HAVE_IPV6_MULTICAST_HOST */

void inet_anyaddr(sa_family_t family, inet_addr_t *addr)
{
	struct sockaddr_in *sin = (struct sockaddr_in *)addr;

#ifdef HAVE_IPV6_MULTICAST_HOST
	if (family == AF_INET6) {
		struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)addr;

		sin6->sin6_family = AF_INET6;
#ifdef HAVE_SOCKADDR_IN_SIN_LEN
		sin6->sin6_len = sizeof(struct sockaddr_in6);
#endif
		memcpy(&sin6->sin6_addr, &in6addr_any, sizeof(in6addr_any));
		return;
	}
#endif

	sin->sin_family = AF_INET;
#ifdef HAVE_SOCKADDR_IN_SIN_LEN
	sin->sin_len = sizeof(struct sockaddr_in);
#endif
	sin->sin_addr.s_addr = htonl(INADDR_ANY);
}

inet_addr_t inet_netaddr(inet_addr_t *addr, int len)
{
	inet_addr_t net = *addr;
	uint32_t bits;
	int max_len;

	assert(addr);
#ifdef HAVE_IPV6_MULTICAST_HOST
	if (addr->ss_family == AF_INET6)
		max_len = 128;
	else
#endif
		max_len = 32;
	assert(len > 0 && len <= max_len);
	bits = max_len - len;

#ifdef HAVE_IPV6_MULTICAST_HOST
	if (addr->ss_family == AF_INET6) {
		struct sockaddr_in6 *sin6 = inet_addr6_get(&net);
		struct in6_addr s6 = sin6->sin6_addr;
		uint32_t pos = 3;

		while (bits >= 32) {
			s6.s6_addr32[pos--] = 0;
			bits -= 32;
		}

		s6.s6_addr32[pos] = htonl(ntohl(s6.s6_addr32[pos]) & ((0xffffffffU << bits) & 0xffffffffU));
		sin6->sin6_addr = s6;
	} else
#endif
	{
		struct in_addr *ina = inet_addr_get(&net);

		ina->s_addr = htonl(ntohl(ina->s_addr) & ((0xffffffffU << bits) & 0xffffffffU));
	}

	return net;
}

int inet_addr_cmp(inet_addr_t *a, inet_addr_t *b)
{
	if (!a || !b) {
		errno = EINVAL;
		return 1;
	}

	if (a->ss_family == AF_INET && b->ss_family == AF_INET) {
		struct sockaddr_in *sa = (struct sockaddr_in *)a;
		struct sockaddr_in *sb = (struct sockaddr_in *)b;

		return sa->sin_addr.s_addr - sb->sin_addr.s_addr;
	}

#ifdef HAVE_IPV6_MULTICAST_HOST
	if (a->ss_family == AF_INET6 && b->ss_family == AF_INET6) {
		struct sockaddr_in6 *sa = (struct sockaddr_in6 *)a;
		struct sockaddr_in6 *sb = (struct sockaddr_in6 *)b;

		return memcmp(sa, sb, sizeof(*sa));
	}
#endif

	errno = EAFNOSUPPORT;
	return 1;
}

const char *inet_addr2str(inet_addr_t *addr, char *str, size_t len)
{
	struct sockaddr_in *sin = (struct sockaddr_in *)addr;

#ifdef HAVE_IPV6_MULTICAST_HOST
	if (addr->ss_family == AF_INET6) {
		struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)addr;
		return inet_ntop(AF_INET6, &sin6->sin6_addr, str, len);
	}
#endif

	return inet_ntop(AF_INET, &sin->sin_addr, str, len);
}

int inet_str2addr(const char *str, inet_addr_t *addr)
{
	struct sockaddr_in *sin = (struct sockaddr_in *)addr;
	int rc;

	if (!str || !addr) {
		errno = EINVAL;
		return -1;
	}

#ifdef HAVE_IPV6_MULTICAST_HOST
	if (strchr(str, ':')) {
		struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)addr;

		sin6->sin6_family = AF_INET6;
#ifdef HAVE_SOCKADDR_IN_SIN_LEN
		sin6->sin6_len = sizeof(struct sockaddr_in6);
#endif
		rc = inet_pton(AF_INET6, str, &sin6->sin6_addr);
	} else
#endif
	{
		sin->sin_family = AF_INET;
#ifdef HAVE_SOCKADDR_IN_SIN_LEN
		sin->sin_len = sizeof(struct sockaddr_in);
#endif
		rc = inet_pton(AF_INET, str, &sin->sin_addr);
	}

	if (rc == 0 || rc == -1)
		return 1;

	return 0;
}

int is_multicast(inet_addr_t *addr)
{
	struct sockaddr_in *sin = (struct sockaddr_in *)addr;

#ifdef HAVE_IPV6_MULTICAST_HOST
	if (addr->ss_family == AF_INET6) {
		struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)addr;
		return IN6_IS_ADDR_MULTICAST(&sin6->sin6_addr);
	}
#endif

	return IN_MULTICAST(ntohl(sin->sin_addr.s_addr));
}

int is_anyaddr(inet_addr_t *addr)
{
	struct sockaddr_in *sin = (struct sockaddr_in *)addr;

#ifdef HAVE_IPV6_MULTICAST_HOST
	if (addr->ss_family == AF_INET6) {
		struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)addr;
		return !memcmp(&sin6->sin6_addr, &in6addr_any, sizeof(in6addr_any));
	}
#endif

	return sin->sin_addr.s_addr == htonl(INADDR_ANY);
}

int inet_iter_init(struct inet_iter *iter, inet_addr_t *addr, int len)
{
	int max_len;

	if (!iter)
		return errno = EINVAL;

#ifdef HAVE_IPV6_MULTICAST_HOST
	if (addr->ss_family == AF_INET6)
		max_len = 128;
	else
#endif
	max_len = 32;
	if (len < 0 || len > max_len) {
		iter->num = 0;
		return errno = EINVAL;
	}

	iter->orig = *addr;
	iter->len  = len;
	iter->addr = inet_netaddr(addr, len);
	iter->num  = 1 << (max_len - len);

	return 0;
}

int inet_iterator(struct inet_iter *iter, inet_addr_t *addr)
{
	if (!iter) {
		errno = EINVAL;
		return 0;
	}
	if (iter->num-- == 0)
		return 0;

	*addr = iter->addr;		/* prepared already */

#ifdef HAVE_IPV6_MULTICAST_HOST
	if (addr->ss_family == AF_INET6) {
		struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&iter->addr;
		struct in6_addr s6 = sin6->sin6_addr;
		uint32_t pos = 4;

		while (pos--) {
			uint32_t addr32 = ntohl(s6.s6_addr32[pos]);

			addr32++;
			s6.s6_addr32[pos] = htonl(addr32);
			if (addr32 > 0)
				break;
		}

		sin6->sin6_addr = s6;
	} else
#endif
	{
		struct sockaddr_in *sin = (struct sockaddr_in *)&iter->addr;
		in_addr_t ina;

		ina = ntohl(sin->sin_addr.s_addr);
		sin->sin_addr.s_addr = htonl(++ina);
	}

	return 1;
}

#ifdef _UNIT_TEST
#include <err.h>
#include <stdio.h>

int main(void)
{
	char str[INET_ADDRSTR_LEN];
	struct inet_iter iter;
	inet_addr_t addr;

	inet_anyaddr(AF_INET6, &addr);
	if (!is_anyaddr(&addr))
		err(1, "FAIL");
	puts("OK");

	inet_str2addr("192.168.1.42", &addr);
	inet_iter_init(&iter, &addr, 24);
	printf("Got num: %u\n", iter.num);
	while (inet_iterator(&iter, &addr))
		printf("%s num %u\n", inet_addr2str(&addr, str, sizeof(str)), iter.num);

	inet_str2addr("2001::1", &addr);
	inet_iter_init(&iter, &addr, 122);
	printf("Got num: %u -> initial str %s\n", iter.num, inet_addr2str(&addr, str, sizeof(str)));
	while (inet_iterator(&iter, &addr))
		printf("%s num %u\n", inet_addr2str(&addr, str, sizeof(str)), iter.num);

	inet_str2addr("192.168.1.42", &addr);
	inet_iter_init(&iter, &addr, 1);
	printf("Got num: %u\n", iter.num);
	while (inet_iterator(&iter, &addr))
		printf("%s num %u\n", inet_addr2str(&addr, str, sizeof(str)), iter.num);

	return 0;
}
#endif

/**
 * Local Variables:
 *  compile-command: "gcc -D_UNIT_TEST -I.. -I. -o unit_test inet.c && ./unit_test"
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
