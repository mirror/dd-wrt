/*
 * lib/addr.c		Abstract Address
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

/**
 * @ingroup nl
 * @defgroup addr Abstract Address
 * Modules to manage addresses of any kind in a generic way.
 *
 * @par Example 1: Transform a character string into an address
 * @code
 * struct nl_addr a;
 * nl_str2addr("::1", &a, AF_UNSPEC);
 * printf("Address family: %s\n", nl_af2str(a.a_family));
 * nl_str2addr("11:22:33:44:55:66", &a, AF_UNSPEC);
 * printf("Address family: %s\n", nl_af2str(a.a_family));
 * @endcode
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/helpers.h>
#include <netlink/addr.h>
#include <linux/socket.h>

/**
 * @name Address Translations
 * @{
 */

/* All this DECnet stuff is stolen from iproute2, thanks to whoever wrote
 * this, probably Alexey. */
static inline uint16_t dn_ntohs(uint16_t addr)
{
	union {
		uint8_t byte[2];
		uint16_t word;
	} u;

	u.word = addr;
	return ((uint16_t)u.byte[0]) | (((uint16_t)u.byte[1]) << 8);
}

static inline int do_digit(char *str, uint16_t *addr, uint16_t scale,
			   size_t *pos, size_t len, int *started)
{
	uint16_t tmp = *addr / scale;

	if (*pos == len)
		return 1;

	if (((tmp) > 0) || *started || (scale == 1)) {
		*str = tmp + '0';
		*started = 1;
		(*pos)++;
		*addr -= (tmp * scale);
	}

	return 0;
}


static const char *dnet_ntop(char *addrbuf, size_t addrlen, char *str, size_t len)
{
	uint16_t addr = dn_ntohs(*(uint16_t *)addrbuf);
	uint16_t area = addr >> 10;
	size_t pos = 0;
	int started = 0;

	if (addrlen != 2)
		return NULL;

	addr &= 0x03ff;

	if (len == 0)
		return str;

	if (do_digit(str + pos, &area, 10, &pos, len, &started))
		return str;

	if (do_digit(str + pos, &area, 1, &pos, len, &started))
		return str;

	if (pos == len)
		return str;

	*(str + pos) = '.';
	pos++;
	started = 0;

	if (do_digit(str + pos, &addr, 1000, &pos, len, &started))
		return str;

	if (do_digit(str + pos, &addr, 100, &pos, len, &started))
		return str;

	if (do_digit(str + pos, &addr, 10, &pos, len, &started))
		return str;

	if (do_digit(str + pos, &addr, 1, &pos, len, &started))
		return str;

	if (pos == len)
		return str;

	*(str + pos) = 0;

	return str;
}

static int dnet_num(const char *src, uint16_t * dst)
{
	int rv = 0;
	int tmp;
	*dst = 0;

	while ((tmp = *src++) != 0) {
		tmp -= '0';
		if ((tmp < 0) || (tmp > 9))
			return rv;

		rv++;
		(*dst) *= 10;
		(*dst) += tmp;
	}

	return rv;
}

static inline int dnet_pton(const char *src, char *addrbuf)
{
	uint16_t area = 0;
	uint16_t node = 0;
	int pos;

	pos = dnet_num(src, &area);
	if ((pos == 0) || (area > 63) ||
	    ((*(src + pos) != '.') && (*(src + pos) != ',')))
		return -EINVAL;

	pos = dnet_num(src + pos + 1, &node);
	if ((pos == 0) || (node > 1023))
		return -EINVAL;

	*(uint16_t *)addrbuf = dn_ntohs((area << 10) | node);

	return 1;
}


/**
 * nl_addr2str_r - Converts a network address into a character string
 *
 * @arg addr   network address
 * @arg buf    destination buffer
 * @arg len    length of destination buffer
 *
 * Converts the given network address into a character string. The family is
 * taken into account if available. In case of AF_UNSPEC or a unknown address
 * family a generic HH:HH:... format is used.
 *
 * The prefix is appended in the form of /prefix.
 *
 * @return The destination buffer \a buf
 */
char * nl_addr2str_r(struct nl_addr *addr, char *buf, size_t len)
{
	int i;
	char s[4] = {0};

	if (0 == addr->a_len) {
		snprintf(buf, len, "none");
		return buf;
	}

	switch (addr->a_family) {
		case AF_INET:
			inet_ntop(AF_INET, addr->a_addr, buf, len);
			break;

		case AF_INET6:
			inet_ntop(AF_INET6, addr->a_addr, buf, len);
			break;

		case AF_DECnet:
			dnet_ntop(addr->a_addr, addr->a_len, buf, len);
			break;

		case AF_LLC:
		default:
			snprintf(buf, len, "%02x", addr->a_addr[0]);
			for (i = 1; i < addr->a_len; i++) {
				snprintf(s, sizeof(s), ":%02x", addr->a_addr[i]);
				strncat(buf, s, len - strlen(buf) - 1);
			}
			break;
	}

	if (addr->a_prefix && addr->a_prefix != (8 * addr->a_len)) {
		char prefix[16];
		snprintf(prefix, sizeof(prefix), "/%u", addr->a_prefix);
		strncat(buf, prefix, len - strlen(buf) - 1);
	}

	return buf;
}

/**
 * Transform a abstract address into a character string
 * @arg addr		abstract address
 *
 * Transforms an abstract address into a character string and stores it in
 * a static buffer.
 *
 * @return A static buffer
 * @attention This function is NOT thread safe.
 */
char * nl_addr2str(struct nl_addr *addr)
{
	static char buf[INET6_ADDRSTRLEN+5];
	memset(buf, 0, sizeof(buf));
	return nl_addr2str_r(addr, buf, sizeof(buf));
}

/**
 * Transforms a character string into a abstract address
 * @arg str		address in form of a character strring
 * @arg addr		destination address buffer
 * @arg hint		address family hint
 *
 * Regognizes the following address formats:
 *@code
 *  Format                      Len                Family
 *  ----------------------------------------------------------------
 *  IPv6 address format         16                 AF_INET6
 *  ddd.ddd.ddd.ddd             4                  AF_INET
 *  HH:HH:HH:HH:HH:HH           6                  AF_LLC
 *  AA{.|,}NNNN                 2                  AF_DECnet
 *  HH:HH:HH:...                variable           AF_UNSPEC
 *
 *  Special values:
 *   {default|all|any}: All bits set to 0, length based on hint or
 *                      AF_INET if no hint is given.
 * @endcode
 *
 * The prefix length may be appened at the end prefixed with a
 * slash, e.g. 10.0.0.0/8.
 *
 * @return 0 on sucess or a negative error code
 */
int nl_str2addr(const char *str, struct nl_addr *addr, int hint)
{
	int err = 0;
	char *prefix;

	prefix = strchr(str, '/');
	if (prefix)
		*prefix = '\0';

	if (!strcasecmp(str, "none")) {
		memset(addr->a_addr, 0, sizeof(addr->a_addr));
		addr->a_family = hint;
		addr->a_len = 0;
		goto prefix;
	}

	if (!strcasecmp(str, "default") ||
	    !strcasecmp(str, "all") ||
	    !strcasecmp(str, "any")) {
		switch (hint) {
			case AF_INET:
			case AF_UNSPEC:
				/* Kind of a hack, we assume that if there is
				 * no hint given the user wants to have a IPv4
				 * address given back. */
				addr->a_family = AF_INET;
				addr->a_len = 4;
				break;

			case AF_INET6:
				addr->a_family = AF_INET6;
				addr->a_len = 16;
				break;

			case AF_LLC:
				addr->a_family = AF_LLC;
				addr->a_len = 6;
				break;

			default:
				err = nl_error(EINVAL, "Unsuported address" \
				    "family for default address");
				goto errout;
		}

		memset(addr->a_addr, 0, sizeof(addr->a_addr));
		goto prefix;
	}

	if (hint == AF_INET || hint == AF_UNSPEC) {
		if (inet_pton(AF_INET, str, addr->a_addr) > 0) {
			addr->a_family = AF_INET;
			addr->a_len = 4;
			goto prefix;
		}
		if (hint == AF_INET) {
			err = nl_error(EINVAL, "Invalid IPv4 address");
			goto errout;
		}
	}

	if (hint == AF_INET6 || hint == AF_UNSPEC) {
		if (inet_pton(AF_INET6, str, addr->a_addr) > 0) {
			addr->a_family = AF_INET6;
			addr->a_len = 16;
			goto prefix;
		}
		if (hint == AF_INET6) {
			err = nl_error(EINVAL, "Invalid IPv6 address");
			goto errout;
		}
	}

	if ((hint == AF_LLC || hint == AF_UNSPEC) && strchr(str, ':')) {
		unsigned int a, b, c, d, e, f;

		if (sscanf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
		    &a, &b, &c, &d, &e, &f) == 6) {
			addr->a_family = AF_LLC;
			addr->a_len = 6;
			addr->a_addr[0] = (unsigned char) a;
			addr->a_addr[1] = (unsigned char) b;
			addr->a_addr[2] = (unsigned char) c;
			addr->a_addr[3] = (unsigned char) d;
			addr->a_addr[4] = (unsigned char) e;
			addr->a_addr[5] = (unsigned char) f;
			goto prefix;
		}

		if (hint == AF_LLC) {
			err = nl_error(EINVAL, "Invalid link layer address");
			goto errout;
		}
	}

	if ((hint == AF_DECnet || hint == AF_UNSPEC) &&
	    (strchr(str, '.') || strchr(str, ','))) {
		if (dnet_pton(str, addr->a_addr) > 0) {
			addr->a_family = AF_DECnet;
			addr->a_len = 2;
			goto prefix;
		}
		if (hint == AF_DECnet) {
			err = nl_error(EINVAL, "Invalid DECnet address");
			goto errout;
		}
	}

	if (hint == AF_UNSPEC && strchr(str, ':')) {
		int i = 0;
		char *p;
		for (;;) {
			long l = strtol(str, &p, 16);

			if (str == p || l > 0xff || i >= NL_ADDR_MAX)
				return -EINVAL;

			addr->a_addr[i++] = (unsigned char) l;
			if (*p == '\0')
				break;
			str = ++p;
		}

		addr->a_len = i;
		addr->a_family = AF_UNSPEC;
		goto prefix;
	}

	err = nl_error(EINVAL, "Invalid address");
	goto errout;

prefix:
	if (prefix) {
		char *p;
		long pl = strtol(++prefix, &p, 0);
		if (p == prefix)
			return -EINVAL;
		addr->a_prefix = pl;
	}
errout:
	if (prefix)
		*prefix = '/';
	
	return err;
}

/** @} */

/**
 * Build a new address
 * @arg buf		address buffer containing the address
 * @arg len		length of address in bytes
 * @arg addr		destination address buffer
 */
void nl_build_addr(void *buf, size_t len, struct nl_addr *addr)
{
	
	addr->a_len = len;
	if (addr->a_len > sizeof(addr->a_addr))
		addr->a_len = sizeof(addr->a_addr);

	memcpy(addr->a_addr, buf, addr->a_len);
}


/**
 * Compares two netlink addreses
 * @arg a		left address
 * @arg b		right address
 * @return 0 if the addresses are equal or a->a_len - b->a_len.
 */
int nl_addrcmp(struct nl_addr *a, struct nl_addr *b)
{
	int d = a->a_len - b->a_len;

	printf("Comparing %d %d\n", a->a_len, b->a_len);

	if (a->a_len && d == 0)
		return memcmp(a->a_addr, b->a_addr, a->a_len);

	return d;
}

/**
 * Check if a address is of a certain type
 * @arg addr		address character string
 * @arg family		desired address family
 *
 * @return 1 if the address is of the desired type, otherwise 0 is returned.
 */
int nl_addr_valid(char *addr, int family)
{
	int ret;
	char buf[32];

	switch (family) {
		case AF_INET:
		case AF_INET6:
			ret = inet_pton(family, addr, buf);
			if (ret <= 0)
				return 0;
			break;

		case AF_DECnet:
			ret = dnet_pton(addr, buf);
			if (ret <= 0)
				return 0;
			break;

		case AF_LLC:
			if (sscanf(addr, "%*02x:%*02x:%*02x:%*02x:%*02x:%*02x") != 6)
				return 0;
			break;
	}

	return 1;
}

/**
 * Guess the family of a address
 * @arg addr		address
 * @return The family or AF_UNSPEC if guessing didn't work.
 */
int nl_addr_guess_family(struct nl_addr *addr)
{
	switch (addr->a_len) {
		case 4:
			return AF_INET;
		case 6:
			return AF_LLC;
		case 16:
			return AF_INET6;
		default:
			return AF_UNSPEC;
	}
}
	

/**
 * @name Address Family Transformations
 * @{
 */

static struct trans_tbl afs[] = {
	__ADD(AF_UNSPEC,unspec)
	__ADD(AF_UNIX,unix)
	__ADD(AF_LOCAL,local)
	__ADD(AF_INET,inet)
	__ADD(AF_AX25,ax25)
	__ADD(AF_IPX,ipx)
	__ADD(AF_APPLETALK,appletalk)
	__ADD(AF_NETROM,netrom)
	__ADD(AF_BRIDGE,bridge)
	__ADD(AF_ATMPVC,atmpvc)
	__ADD(AF_X25,x25)
	__ADD(AF_INET6,inet6)
	__ADD(AF_ROSE,rose)
	__ADD(AF_DECnet,decnet)
	__ADD(AF_NETBEUI,netbeui)
	__ADD(AF_SECURITY,security)
	__ADD(AF_KEY,key)
	__ADD(AF_NETLINK,netlink)
	__ADD(AF_ROUTE,route)
	__ADD(AF_PACKET,packet)
	__ADD(AF_ASH,ash)
	__ADD(AF_ECONET,econet)
	__ADD(AF_ATMSVC,atmsvc)
	__ADD(AF_SNA,sna)
	__ADD(AF_IRDA,irda)
	__ADD(AF_PPPOX,pppox)
	__ADD(AF_WANPIPE,wanpipe)
	__ADD(AF_LLC,llc)
	__ADD(AF_BLUETOOTH,bluetooth)
};


/**
 * Transform a address family into a character string (Reentrant).
 * @arg family	address family
 * @arg buf		destination buffer
 * @arg len		buffer length
 *
 * Transforms a address family into a character string and stores it in the
 * provided buffer.
 *
 * @return The destination buffer or the type encoded in hex if no match was found.
 */
char * nl_af2str_r(char family, char *buf, size_t len)
{
	return __type2str_r(family, buf, len, afs, ARRAY_SIZE(afs));
}

/**
 * Transform a address family into a character string
 * @arg family	address family
 *
 * Transforms a address family into a character string and stores it in
 * a static buffer.
 *
 * @return A static buffer or the type encoded in hex if no match was found.
 * @attention This funnction is NOT thread safe.
 */
char * nl_af2str(char family)
{
	static char buf[32];
	memset(buf, 0, sizeof(buf));
	return __type2str_r(family, buf, sizeof(buf), afs, ARRAY_SIZE(afs));
}

/**
 * Transform a character string into a address family.
 * @arg name		address family name
 *
 * Transform the provided character string specifying a address family
 * into the corresponding numeric value.
 *
 * @return Address family as number or \c AF_UNSPEC.
 */
char nl_str2af(const char *name)
{
	char fam = __str2type(name, afs, ARRAY_SIZE(afs));
	return fam >= 0 ? fam : AF_UNSPEC;
}

/** @} */
/** @} */
