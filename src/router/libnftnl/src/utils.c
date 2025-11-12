/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2012-2013 by Pablo Neira Ayuso <pablo@netfilter.org>
 * (C) 2013 by Arturo Borrero Gonzalez <arturo@debian.org>
 */

#include <internal.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <errno.h>
#include <inttypes.h>

#include <libnftnl/common.h>

#include <linux/netfilter.h>
#include <linux/netfilter/nf_tables.h>

static const char *const nftnl_family_str[NFPROTO_NUMPROTO] = {
	[NFPROTO_INET]		= "inet",
	[NFPROTO_IPV4]		= "ip",
	[NFPROTO_ARP]		= "arp",
	[NFPROTO_NETDEV]	= "netdev",
	[NFPROTO_BRIDGE]	= "bridge",
	[NFPROTO_IPV6]		= "ip6",
};

const char *nftnl_family2str(uint32_t family)
{
	if (family >= NFPROTO_NUMPROTO || !nftnl_family_str[family])
		return "unknown";

	return nftnl_family_str[family];
}

const char *nftnl_verdict2str(uint32_t verdict)
{
	switch (verdict) {
	case NF_ACCEPT:
		return "accept";
	case NF_DROP:
		return "drop";
	case NF_STOLEN:
		return "stolen";
	case NF_QUEUE:
		return "queue";
	case NF_REPEAT:
		return "repeat";
	case NF_STOP:
		return "stop";
	case NFT_RETURN:
		return "return";
	case NFT_JUMP:
		return "jump";
	case NFT_GOTO:
		return "goto";
	case NFT_CONTINUE:
		return "continue";
	case NFT_BREAK:
		return "break";
	default:
		return "unknown";
	}
}

enum nftnl_cmd_type nftnl_flag2cmd(uint32_t flags)
{
	if (flags & NFTNL_OF_EVENT_NEW)
		return NFTNL_CMD_ADD;
	else if (flags & NFTNL_OF_EVENT_DEL)
		return NFTNL_CMD_DELETE;

	return NFTNL_CMD_UNSPEC;
}

int nftnl_fprintf(FILE *fp, const void *obj, uint32_t cmd, uint32_t type,
		  uint32_t flags,
		  int (*snprintf_cb)(char *buf, size_t bufsiz, const void *obj,
				     uint32_t cmd, uint32_t type,
				     uint32_t flags))
{
	char _buf[NFTNL_SNPRINTF_BUFSIZ];
	char *buf = _buf;
	size_t bufsiz = sizeof(_buf);
	int ret;

	ret = snprintf_cb(buf, bufsiz, obj, cmd, type, flags);
	if (ret <= 0)
		goto out;

	if (ret >= NFTNL_SNPRINTF_BUFSIZ) {
		bufsiz = ret + 1;

		buf = malloc(bufsiz);
		if (buf == NULL)
			return -1;

		ret = snprintf_cb(buf, bufsiz, obj, cmd, type, flags);
		if (ret <= 0)
			goto out;
	}

	ret = fprintf(fp, "%s", buf);

out:
	if (buf != _buf)
		xfree(buf);

	return ret;
}

void __nftnl_assert_attr_exists(uint16_t attr, uint16_t attr_max,
				const char *filename, int line)
{
	fprintf(stderr, "libnftnl: attribute %d > %d (maximum) assertion failed in %s:%d\n",
		attr, attr_max, filename, line);
	exit(EXIT_FAILURE);
}

void __nftnl_assert_fail(uint16_t attr, const char *filename, int line)
{
	fprintf(stderr, "libnftnl: attribute %d assertion failed in %s:%d\n",
		attr, filename, line);
	exit(EXIT_FAILURE);
}

void __noreturn __abi_breakage(const char *file, int line, const char *reason)
{
       fprintf(stderr, "nf_tables kernel ABI is broken, contact your vendor.\n"
		       "%s:%d reason: %s\n", file, line, reason);
       exit(EXIT_FAILURE);
}

int nftnl_set_str_attr(const char **dptr, uint32_t *flags,
		       uint16_t attr, const void *data, uint32_t data_len)
{
	if (*flags & (1 << attr))
		xfree(*dptr);

	*dptr = strndup(data, data_len);
	if (!*dptr)
		return -1;

	*flags |= (1 << attr);
	return 0;
}
