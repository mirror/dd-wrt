/*
 * utils.c
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 *
 *
 * Changes:
 *
 * Rani Assaf <rani@magic.metawire.com> 980929:	resolve addresses
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <resolv.h>

#include "intl.h"
#include "util-ank.h"

#ifndef AF_INET6
#define AF_INET6	10
#endif

int scan_number(char *arg, unsigned *val)
{
	unsigned long res;
	char *ptr;

	if (!arg || !*arg)
		return -1;
	res = strtoul(arg, &ptr, 0);
	if (!ptr || ptr == arg || *ptr || res > UINT_MAX)
		return -1;
	*val = res;
	return 0;
}

int get_integer(int *val, char *arg, int base)
{
	long res;
	char *ptr;

	if (!arg || !*arg)
		return -1;
	res = strtol(arg, &ptr, base);
	if (!ptr || ptr == arg || *ptr || res > INT_MAX || res < INT_MIN)
		return -1;
	*val = res;
	return 0;
}

int get_unsigned(unsigned *val, char *arg, int base)
{
	unsigned long res;
	char *ptr;

	if (!arg || !*arg)
		return -1;
	res = strtoul(arg, &ptr, base);
	if (!ptr || ptr == arg || *ptr || res > UINT_MAX)
		return -1;
	*val = res;
	return 0;
}

int get_u32(__u32 *val, char *arg, int base)
{
	unsigned long res;
	char *ptr;

	if (!arg || !*arg)
		return -1;
	res = strtoul(arg, &ptr, base);
	if (!ptr || ptr == arg || *ptr || res > 0xFFFFFFFFUL)
		return -1;
	*val = res;
	return 0;
}

int get_u16(__u16 *val, char *arg, int base)
{
	unsigned long res;
	char *ptr;

	if (!arg || !*arg)
		return -1;
	res = strtoul(arg, &ptr, base);
	if (!ptr || ptr == arg || *ptr || res > 0xFFFF)
		return -1;
	*val = res;
	return 0;
}

int get_u8(__u8 *val, char *arg, int base)
{
	unsigned long res;
	char *ptr;

	if (!arg || !*arg)
		return -1;
	res = strtoul(arg, &ptr, base);
	if (!ptr || ptr == arg || *ptr || res > 0xFF)
		return -1;
	*val = res;
	return 0;
}

int get_s16(__s16 *val, char *arg, int base)
{
	long res;
	char *ptr;

	if (!arg || !*arg)
		return -1;
	res = strtol(arg, &ptr, base);
	if (!ptr || ptr == arg || *ptr || res > 0x7FFF || res < -0x8000)
		return -1;
	*val = res;
	return 0;
}

int get_s8(__s8 *val, char *arg, int base)
{
	long res;
	char *ptr;

	if (!arg || !*arg)
		return -1;
	res = strtol(arg, &ptr, base);
	if (!ptr || ptr == arg || *ptr || res > 0x7F || res < -0x80)
		return -1;
	*val = res;
	return 0;
}

int get_addr_1(inet_prefix *addr, char *name, int family)
{
	char *cp;
	unsigned char *ap = (unsigned char*)addr->data;
	int i;

	memset(addr, 0, sizeof(*addr));

	if (strcmp(name, "default") == 0 || strcmp(name, "any") == 0) {
		addr->family = family;
		addr->bytelen = (family == AF_INET6 ? 16 : 4);
		addr->bitlen = -1;
		return 0;
	}

	if (strchr(name, ':')) {
		addr->family = AF_INET6;
		if (family != AF_UNSPEC && family != AF_INET6)
			return -1;
		if (inet_pton(AF_INET6, name, addr->data) <= 0)
			return -1;
		addr->bytelen = 16;
		addr->bitlen = -1;
		return 0;
	}

	addr->family = AF_INET;
	if (family != AF_UNSPEC && family != AF_INET)
		return -1;
	addr->bytelen = 4;
	addr->bitlen = -1;
	for (cp=name, i=0; *cp; cp++) {
		if (*cp <= '9' && *cp >= '0') {
			ap[i] = 10*ap[i] + (*cp-'0');
			continue;
		}
		if (*cp == '.' && ++i <= 3)
			continue;
		return -1;
	}
	return 0;
}

int get_prefix_1(inet_prefix *dst, char *arg, int family)
{
	int err;
	unsigned plen;
	char *slash;

	memset(dst, 0, sizeof(*dst));

	if (strcmp(arg, "default") == 0 || strcmp(arg, "any") == 0) {
		dst->family = family;
		dst->bytelen = 0;
		dst->bitlen = 0;
		return 0;
	}

	slash = strchr(arg, '/');
	if (slash)
		*slash = 0;
	err = get_addr_1(dst, arg, family);
	if (err == 0) {
		dst->bitlen = (dst->family == AF_INET6 ? 128 : 32);
		if (slash) {
			if (scan_number(slash+1, &plen) || plen > dst->bitlen) {
				err = -1;
				goto done;
			}
			dst->bitlen = plen;
		}
	}
done:
	if (slash)
		*slash = '/';
	return err;
}

int get_addr(inet_prefix *dst, char *arg, int family)
{
	if (get_addr_1(dst, arg, family)) {
		fprintf(stderr, _("ip: %s is invalid inet address\n"), arg);
		exit(1);
	}
	return 0;
}

int get_prefix(inet_prefix *dst, char *arg, int family)
{
	if (get_prefix_1(dst, arg, family)) {
		fprintf(stderr, _("ip: %s is invalid inet prefix\n"), arg);
		exit(1);
	}
	return 0;
}

__u32 get_addr32(char *name)
{
	inet_prefix addr;
	if (get_addr_1(&addr, name, AF_INET)) {
		fprintf(stderr, _("ip: %s is invalid IPv4 address\n"), name);
		exit(1);
	}
	return addr.data[0];
}

void invarg(char *msg)
{
	fprintf(stderr, _("ip: argument is wrong: %s\n"), msg);
	exit(1);
}

int matches(char *cmd, char *pattern)
{
	int len = strlen(cmd);
	if (len > strlen(pattern))
		return -1;
	return memcmp(pattern, cmd, len);
}

int inet_addr_match(inet_prefix *a, inet_prefix *b, int bits)
{
	__u32 *a1 = a->data;
	__u32 *a2 = b->data;
	int words = bits >> 0x05;

	bits &= 0x1f;

	if (words)
		if (memcmp(a1, a2, words << 2))
			return -1;

	if (bits) {
		__u32 w1, w2;
		__u32 mask;

		w1 = a1[words];
		w2 = a2[words];

		mask = htonl((0xffffffff) << (0x20 - bits));

		if ((w1 ^ w2) & mask)
			return 1;
	}

	return 0;
}

const char *format_host(int af, void *addr, __u8 *abuf, int alen)
{
#ifdef RESOLVE_HOSTNAMES
	if (resolve_hosts) {
		int addrlen = 0;
		struct hostent *h_ent;
		switch (af) {
		case AF_INET:
			addrlen = 4;
			break;
		case AF_INET6:
			addrlen = 16;
			break;
		}
		if (addrlen &&
		    (h_ent = gethostbyaddr(addr, addrlen, af)) != NULL) {
			snprintf(abuf, alen-1, "%s", h_ent->h_name);
			return abuf;
		}
	}
#endif
	return inet_ntop(af, addr, abuf, alen);
}
