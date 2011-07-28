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
#include <asm/types.h>
#include <linux/pkt_sched.h>
#include <time.h>
#include <sys/time.h>


#include "utils.h"

int get_integer(int *val, const char *arg, int base)
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

int mask2bits(__u32 netmask)
{
	unsigned bits = 0;
	__u32 mask = ntohl(netmask);
	__u32 host = ~mask;

	/* a valid netmask must be 2^n - 1 */
	if ((host & (host + 1)) != 0)
		return -1;

	for (; mask; mask <<= 1)
		++bits;
	return bits;
}

static int get_netmask(unsigned *val, const char *arg, int base)
{
	inet_prefix addr;

	if (!get_unsigned(val, arg, base))
		return 0;

	/* try coverting dotted quad to CIDR */
	if (!get_addr_1(&addr, arg, AF_INET) && addr.family == AF_INET) {
		int b = mask2bits(addr.data[0]);
		
		if (b >= 0) {
			*val = b;
			return 0;
		}
	}

	return -1;
}

int get_unsigned(unsigned *val, const char *arg, int base)
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

int get_u64(__u64 *val, const char *arg, int base)
{
	unsigned long long res;
	char *ptr;

	if (!arg || !*arg)
		return -1;
	res = strtoull(arg, &ptr, base);
	if (!ptr || ptr == arg || *ptr || res == 0xFFFFFFFFULL)
 		return -1;
 	*val = res;
 	return 0;
}

int get_u32(__u32 *val, const char *arg, int base)
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

int get_u16(__u16 *val, const char *arg, int base)
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

int get_u8(__u8 *val, const char *arg, int base)
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

int get_s16(__s16 *val, const char *arg, int base)
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

int get_s8(__s8 *val, const char *arg, int base)
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

/* This uses a non-standard parsing (ie not inet_aton, or inet_pton)
 * because of legacy choice to parse 10.8 as 10.8.0.0 not 10.0.0.8
 */
static int get_addr_ipv4(__u8 *ap, const char *cp)
{
	int i;

	for (i = 0; i < 4; i++) {
		unsigned long n;
		char *endp;
		
		n = strtoul(cp, &endp, 0);
		if (n > 255)
			return -1;	/* bogus network value */

		if (endp == cp) /* no digits */
			return -1;

		ap[i] = n;

		if (*endp == '\0')
			break;

		if (i == 3 || *endp != '.')
			return -1; 	/* extra characters */
		cp = endp + 1;
	}

	return 1;
}

int get_addr_1(inet_prefix *addr, const char *name, int family)
{
	memset(addr, 0, sizeof(*addr));

	if (strcmp(name, "default") == 0 ||
	    strcmp(name, "all") == 0 ||
	    strcmp(name, "any") == 0) {
		if (family == AF_DECnet)
			return -1;
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

	if (get_addr_ipv4((__u8 *)addr->data, name) <= 0)
		return -1;

	addr->bytelen = 4;
	addr->bitlen = -1;
	return 0;
}

int get_prefix_1(inet_prefix *dst, char *arg, int family)
{
	int err;
	unsigned plen;
	char *slash;

	memset(dst, 0, sizeof(*dst));

	if (strcmp(arg, "default") == 0 ||
	    strcmp(arg, "any") == 0 ||
	    strcmp(arg, "all") == 0) {
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
		switch(dst->family) {
			case AF_INET6:
				dst->bitlen = 128;
				break;
			default:
			case AF_INET:
				dst->bitlen = 32;
		}
		if (slash) {
			if (get_netmask(&plen, slash+1, 0)
					|| plen > dst->bitlen) {
				err = -1;
				goto done;
			}
			dst->flags |= PREFIXLEN_SPECIFIED;
			dst->bitlen = plen;
		}
	}
done:
	if (slash)
		*slash = '/';
	return err;
}

int get_addr(inet_prefix *dst, const char *arg, int family)
{
	if (family == AF_PACKET) {
		fprintf(stderr, "Error: \"%s\" may be inet address, but it is not allowed in this context.\n", arg);
		exit(1);
	}
	if (get_addr_1(dst, arg, family)) {
		fprintf(stderr, "Error: an inet address is expected rather than \"%s\".\n", arg);
		exit(1);
	}
	return 0;
}

int get_prefix(inet_prefix *dst, char *arg, int family)
{
	if (family == AF_PACKET) {
		fprintf(stderr, "Error: \"%s\" may be inet prefix, but it is not allowed in this context.\n", arg);
		exit(1);
	}
	if (get_prefix_1(dst, arg, family)) {
		fprintf(stderr, "Error: an inet prefix is expected rather than \"%s\".\n", arg);
		exit(1);
	}
	return 0;
}

__u32 get_addr32(const char *name)
{
	inet_prefix addr;
	if (get_addr_1(&addr, name, AF_INET)) {
		fprintf(stderr, "Error: an IP address is expected rather than \"%s\"\n", name);
		exit(1);
	}
	return addr.data[0];
}

void incomplete_command(void)
{
	fprintf(stderr, "Command line is not complete. Try option \"help\"\n");
	exit(-1);
}

void missarg(const char *key)
{
	fprintf(stderr, "Error: argument \"%s\" is required\n", key);
	exit(-1);
}

void invarg(const char *msg, const char *arg)
{
	fprintf(stderr, "Error: argument \"%s\" is wrong: %s\n", arg, msg);
	exit(-1);
}

void duparg(const char *key, const char *arg)
{
	fprintf(stderr, "Error: duplicate \"%s\": \"%s\" is the second value.\n", key, arg);
	exit(-1);
}

void duparg2(const char *key, const char *arg)
{
	fprintf(stderr, "Error: either \"%s\" is duplicate, or \"%s\" is a garbage.\n", key, arg);
	exit(-1);
}

int matches(const char *cmd, const char *pattern)
{
	int len = strlen(cmd);
	if (len > strlen(pattern))
		return -1;
	return memcmp(pattern, cmd, len);
}

int inet_addr_match(const inet_prefix *a, const inet_prefix *b, int bits)
{
	const __u32 *a1 = a->data;
	const __u32 *a2 = b->data;
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

char *hexstring_n2a(const __u8 *str, int len, char *buf, int blen)
{
	char *ptr = buf;
	int i;

	for (i=0; i<len; i++) {
		if (blen < 3)
			break;
		sprintf(ptr, "%02x", str[i]);
		ptr += 2;
		blen -= 2;
		if (i != len-1 && blen > 1) {
			*ptr++ = ':';
			blen--;
		}
	}
	return buf;
}

__u8* hexstring_a2n(const char *str, __u8 *buf, int blen)
{
	int cnt = 0;

	for (;;) {
		unsigned acc;
		char ch;

		acc = 0;

		while ((ch = *str) != ':' && ch != 0) {
			if (ch >= '0' && ch <= '9')
				ch -= '0';
			else if (ch >= 'a' && ch <= 'f')
				ch -= 'a'-10;
			else if (ch >= 'A' && ch <= 'F')
				ch -= 'A'-10;
			else
				return NULL;
			acc = (acc<<4) + ch;
			str++;
		}

		if (acc > 255)
			return NULL;
		if (cnt < blen) {
			buf[cnt] = acc;
			cnt++;
		}
		if (ch == 0)
			break;
		++str;
	}
	if (cnt < blen)
		memset(buf+cnt, 0, blen-cnt);
	return buf;
}
