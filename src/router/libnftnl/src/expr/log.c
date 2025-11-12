/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2012-2013 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This code has been sponsored by Sophos Astaro <http://www.sophos.com>
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <linux/netfilter/nf_tables.h>
#include <linux/netfilter/nf_log.h>

#include "internal.h"
#include <libmnl/libmnl.h>
#include <libnftnl/expr.h>
#include <libnftnl/rule.h>

struct nftnl_expr_log {
	uint32_t		snaplen;
	uint16_t		group;
	uint16_t		qthreshold;
	uint32_t		level;
	uint32_t		flags;
	const char		*prefix;
};

static int nftnl_expr_log_set(struct nftnl_expr *e, uint16_t type,
				 const void *data, uint32_t data_len)
{
	struct nftnl_expr_log *log = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_LOG_PREFIX:
		if (log->flags & (1 << NFTNL_EXPR_LOG_PREFIX))
			xfree(log->prefix);

		log->prefix = strdup(data);
		if (!log->prefix)
			return -1;
		break;
	case NFTNL_EXPR_LOG_GROUP:
		memcpy(&log->group, data, data_len);
		break;
	case NFTNL_EXPR_LOG_SNAPLEN:
		memcpy(&log->snaplen, data, data_len);
		break;
	case NFTNL_EXPR_LOG_QTHRESHOLD:
		memcpy(&log->qthreshold, data, data_len);
		break;
	case NFTNL_EXPR_LOG_LEVEL:
		memcpy(&log->level, data, data_len);
		break;
	case NFTNL_EXPR_LOG_FLAGS:
		memcpy(&log->flags, data, data_len);
		break;
	}
	return 0;
}

static const void *
nftnl_expr_log_get(const struct nftnl_expr *e, uint16_t type,
		      uint32_t *data_len)
{
	struct nftnl_expr_log *log = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_LOG_PREFIX:
		*data_len = strlen(log->prefix)+1;
		return log->prefix;
	case NFTNL_EXPR_LOG_GROUP:
		*data_len = sizeof(log->group);
		return &log->group;
	case NFTNL_EXPR_LOG_SNAPLEN:
		*data_len = sizeof(log->snaplen);
		return &log->snaplen;
	case NFTNL_EXPR_LOG_QTHRESHOLD:
		*data_len = sizeof(log->qthreshold);
		return &log->qthreshold;
	case NFTNL_EXPR_LOG_LEVEL:
		*data_len = sizeof(log->level);
		return &log->level;
	case NFTNL_EXPR_LOG_FLAGS:
		*data_len = sizeof(log->flags);
		return &log->flags;
	}
	return NULL;
}

static int nftnl_expr_log_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_LOG_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFTA_LOG_PREFIX:
		if (mnl_attr_validate(attr, MNL_TYPE_STRING) < 0)
			abi_breakage();
		break;
	case NFTA_LOG_GROUP:
	case NFTA_LOG_QTHRESHOLD:
		if (mnl_attr_validate(attr, MNL_TYPE_U16) < 0)
			abi_breakage();
		break;
	case NFTA_LOG_SNAPLEN:
	case NFTA_LOG_LEVEL:
	case NFTA_LOG_FLAGS:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static void
nftnl_expr_log_build(struct nlmsghdr *nlh, const struct nftnl_expr *e)
{
	struct nftnl_expr_log *log = nftnl_expr_data(e);

	if (e->flags & (1 << NFTNL_EXPR_LOG_PREFIX))
		mnl_attr_put_strz(nlh, NFTA_LOG_PREFIX, log->prefix);
	if (e->flags & (1 << NFTNL_EXPR_LOG_GROUP))
		mnl_attr_put_u16(nlh, NFTA_LOG_GROUP, htons(log->group));
	if (e->flags & (1 << NFTNL_EXPR_LOG_SNAPLEN))
		mnl_attr_put_u32(nlh, NFTA_LOG_SNAPLEN, htonl(log->snaplen));
	if (e->flags & (1 << NFTNL_EXPR_LOG_QTHRESHOLD))
		mnl_attr_put_u16(nlh, NFTA_LOG_QTHRESHOLD, htons(log->qthreshold));
	if (e->flags & (1 << NFTNL_EXPR_LOG_LEVEL))
		mnl_attr_put_u32(nlh, NFTA_LOG_LEVEL, htonl(log->level));
	if (e->flags & (1 << NFTNL_EXPR_LOG_FLAGS))
		mnl_attr_put_u32(nlh, NFTA_LOG_FLAGS, htonl(log->flags));
}

static int
nftnl_expr_log_parse(struct nftnl_expr *e, struct nlattr *attr)
{
	struct nftnl_expr_log *log = nftnl_expr_data(e);
	struct nlattr *tb[NFTA_LOG_MAX+1] = {};

	if (mnl_attr_parse_nested(attr, nftnl_expr_log_cb, tb) < 0)
		return -1;

	if (tb[NFTA_LOG_PREFIX]) {
		if (log->prefix)
			xfree(log->prefix);

		log->prefix = strdup(mnl_attr_get_str(tb[NFTA_LOG_PREFIX]));
		if (!log->prefix)
			return -1;
		e->flags |= (1 << NFTNL_EXPR_LOG_PREFIX);
	}
	if (tb[NFTA_LOG_GROUP]) {
		log->group = ntohs(mnl_attr_get_u16(tb[NFTA_LOG_GROUP]));
		e->flags |= (1 << NFTNL_EXPR_LOG_GROUP);
	}
	if (tb[NFTA_LOG_SNAPLEN]) {
		log->snaplen = ntohl(mnl_attr_get_u32(tb[NFTA_LOG_SNAPLEN]));
		e->flags |= (1 << NFTNL_EXPR_LOG_SNAPLEN);
	}
	if (tb[NFTA_LOG_QTHRESHOLD]) {
		log->qthreshold = ntohs(mnl_attr_get_u16(tb[NFTA_LOG_QTHRESHOLD]));
		e->flags |= (1 << NFTNL_EXPR_LOG_QTHRESHOLD);
	}
	if (tb[NFTA_LOG_LEVEL]) {
		log->level = ntohl(mnl_attr_get_u32(tb[NFTA_LOG_LEVEL]));
		e->flags |= (1 << NFTNL_EXPR_LOG_LEVEL);
	}
	if (tb[NFTA_LOG_FLAGS]) {
		log->flags = ntohl(mnl_attr_get_u32(tb[NFTA_LOG_FLAGS]));
		e->flags |= (1 << NFTNL_EXPR_LOG_FLAGS);
	}

	return 0;
}

static int
nftnl_expr_log_snprintf(char *buf, size_t remain,
			uint32_t flags, const struct nftnl_expr *e)
{
	struct nftnl_expr_log *log = nftnl_expr_data(e);
	int ret, offset = 0;

	if (e->flags & (1 << NFTNL_EXPR_LOG_PREFIX)) {
		ret = snprintf(buf, remain, "prefix %s ", log->prefix);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	}

	if (e->flags & (1 << NFTNL_EXPR_LOG_GROUP)) {
		ret = snprintf(buf + offset, remain,
			       "group %u snaplen %u qthreshold %u ",
			       log->group, log->snaplen, log->qthreshold);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	} else {
		if (e->flags & (1 << NFTNL_EXPR_LOG_LEVEL)) {
			ret = snprintf(buf + offset, remain, "level %u ",
				       log->level);
			SNPRINTF_BUFFER_SIZE(ret, remain, offset);
		}
		if (e->flags & (1 << NFTNL_EXPR_LOG_FLAGS)) {
			if (log->flags & NF_LOG_TCPSEQ) {
				ret = snprintf(buf + offset, remain, "tcpseq ");
				SNPRINTF_BUFFER_SIZE(ret, remain, offset);
			}
			if (log->flags & NF_LOG_TCPOPT) {
				ret = snprintf(buf + offset, remain, "tcpopt ");
				SNPRINTF_BUFFER_SIZE(ret, remain, offset);
			}
			if (log->flags & NF_LOG_IPOPT) {
				ret = snprintf(buf + offset, remain, "ipopt ");
				SNPRINTF_BUFFER_SIZE(ret, remain, offset);
			}
			if (log->flags & NF_LOG_UID) {
				ret = snprintf(buf + offset, remain, "uid ");
				SNPRINTF_BUFFER_SIZE(ret, remain, offset);
			}
			if (log->flags & NF_LOG_MACDECODE) {
				ret = snprintf(buf + offset, remain,
					       "macdecode ");
				SNPRINTF_BUFFER_SIZE(ret, remain, offset);
			}
		}
	}

	return offset;
}

static void nftnl_expr_log_free(const struct nftnl_expr *e)
{
	struct nftnl_expr_log *log = nftnl_expr_data(e);

	xfree(log->prefix);
}

static struct attr_policy log_attr_policy[__NFTNL_EXPR_LOG_MAX] = {
	[NFTNL_EXPR_LOG_PREFIX]     = { .maxlen = NF_LOG_PREFIXLEN },
	[NFTNL_EXPR_LOG_GROUP]      = { .maxlen = sizeof(uint16_t) },
	[NFTNL_EXPR_LOG_SNAPLEN]    = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_LOG_QTHRESHOLD] = { .maxlen = sizeof(uint16_t) },
	[NFTNL_EXPR_LOG_LEVEL]      = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_LOG_FLAGS]      = { .maxlen = sizeof(uint32_t) },
};

struct expr_ops expr_ops_log = {
	.name		= "log",
	.alloc_len	= sizeof(struct nftnl_expr_log),
	.nftnl_max_attr	= __NFTNL_EXPR_LOG_MAX - 1,
	.attr_policy	= log_attr_policy,
	.free		= nftnl_expr_log_free,
	.set		= nftnl_expr_log_set,
	.get		= nftnl_expr_log_get,
	.parse		= nftnl_expr_log_parse,
	.build		= nftnl_expr_log_build,
	.output		= nftnl_expr_log_snprintf,
};
