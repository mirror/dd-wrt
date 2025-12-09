/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2015 Red Hat GmbH
 * Author: Florian Westphal <fw@strlen.de>
 */
#include "internal.h"

#include <time.h>
#include <endian.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>

#include <libmnl/libmnl.h>
#include <linux/netfilter.h>
#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nf_tables.h>

#include <libnftnl/trace.h>

struct nftnl_header_data {
	char *data;
	unsigned int len;
};

struct nftnl_trace {
	char *table;
	char *chain;
	char *jump_target;
	uint64_t rule_handle;
	struct nftnl_header_data ll;
	struct nftnl_header_data nh;
	struct nftnl_header_data th;
	uint32_t family;
	uint32_t type;
	uint32_t id;
	uint32_t iif;
	uint32_t oif;
	uint32_t mark;
	uint32_t verdict;
	uint32_t nfproto;
	uint32_t policy;
	uint16_t iiftype;
	uint16_t oiftype;
	struct {
		uint16_t dir;
		uint32_t id;
		uint32_t state;
		uint32_t status;
	} ct;

	uint32_t flags;
};

EXPORT_SYMBOL(nftnl_trace_alloc);
struct nftnl_trace *nftnl_trace_alloc(void)
{
	return calloc(1, sizeof(struct nftnl_trace));
}

EXPORT_SYMBOL(nftnl_trace_free);
void nftnl_trace_free(const struct nftnl_trace *t)
{
	xfree(t->chain);
	xfree(t->table);
	xfree(t->jump_target);
	xfree(t->ll.data);
	xfree(t->nh.data);
	xfree(t->th.data);
	xfree(t);
}

EXPORT_SYMBOL(nftnl_trace_is_set);
bool nftnl_trace_is_set(const struct nftnl_trace *t, uint16_t attr)
{
	return t->flags & (1 << attr);
}

static int nftnl_trace_parse_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	enum nft_trace_attributes type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_TRACE_MAX) < 0)
		return MNL_CB_OK;

	switch (type) {
	case NFTA_TRACE_UNSPEC:
	case __NFTA_TRACE_MAX:
		break;
	case NFTA_TRACE_VERDICT:
                if (mnl_attr_validate(attr, MNL_TYPE_NESTED) < 0)
			abi_breakage();
		break;
	case NFTA_TRACE_CT_DIRECTION:
		if (mnl_attr_validate(attr, MNL_TYPE_U8) < 0)
			abi_breakage();
		break;
	case NFTA_TRACE_IIFTYPE:
	case NFTA_TRACE_OIFTYPE:
		if (mnl_attr_validate(attr, MNL_TYPE_U16) < 0)
			abi_breakage();
		break;
	case NFTA_TRACE_ID:
	case NFTA_TRACE_IIF:
	case NFTA_TRACE_MARK:
	case NFTA_TRACE_OIF:
	case NFTA_TRACE_POLICY:
	case NFTA_TRACE_NFPROTO:
	case NFTA_TRACE_TYPE:
	case NFTA_TRACE_CT_ID:
	case NFTA_TRACE_CT_STATE:
	case NFTA_TRACE_CT_STATUS:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	case NFTA_TRACE_CHAIN:
	case NFTA_TRACE_TABLE:
		if (mnl_attr_validate(attr, MNL_TYPE_STRING) < 0)
			abi_breakage();
		break;
	case NFTA_TRACE_RULE_HANDLE:
		if (mnl_attr_validate(attr, MNL_TYPE_U64) < 0)
			abi_breakage();
		break;
	case NFTA_TRACE_LL_HEADER:	/* fallthrough */
	case NFTA_TRACE_NETWORK_HEADER:
	case NFTA_TRACE_TRANSPORT_HEADER:
		if (mnl_attr_get_payload_len(attr) == 0)
			abi_breakage();
		break;
	default:
		return MNL_CB_OK;
	};

	tb[type] = attr;
	return MNL_CB_OK;
}

EXPORT_SYMBOL(nftnl_trace_get_data);
const void *nftnl_trace_get_data(const struct nftnl_trace *trace,
				 uint16_t type, uint32_t *data_len)
{
	enum nftnl_trace_attr attr = type;

	if (!(trace->flags & (1 << type)))
		return NULL;

	switch (attr) {
	case NFTNL_TRACE_FAMILY:
		*data_len = sizeof(uint32_t);
		return &trace->family;
	case NFTNL_TRACE_ID:
		*data_len = sizeof(uint32_t);
		return &trace->id;
	case NFTNL_TRACE_IIF:
		*data_len = sizeof(uint32_t);
		return &trace->iif;
	case NFTNL_TRACE_OIF:
		*data_len = sizeof(uint32_t);
		return &trace->oif;
	case NFTNL_TRACE_LL_HEADER:
		*data_len = trace->ll.len;
		return trace->ll.data;
	case NFTNL_TRACE_MARK:
		*data_len = sizeof(uint32_t);
		return &trace->mark;
	case NFTNL_TRACE_NETWORK_HEADER:
		*data_len = trace->nh.len;
		return trace->nh.data;
	case NFTNL_TRACE_TYPE:
		*data_len = sizeof(uint32_t);
		return &trace->type;
	case NFTNL_TRACE_CHAIN:
		*data_len = strlen(trace->chain) + 1;
		return trace->chain;
	case NFTNL_TRACE_TABLE:
		*data_len = strlen(trace->table) + 1;
		return trace->table;
	case NFTNL_TRACE_JUMP_TARGET:
		*data_len = strlen(trace->jump_target) + 1;
		return trace->jump_target;
	case NFTNL_TRACE_TRANSPORT_HEADER:
		*data_len = trace->th.len;
		return trace->th.data;
	case NFTNL_TRACE_RULE_HANDLE:
		*data_len = sizeof(uint64_t);
		return &trace->rule_handle;
	case NFTNL_TRACE_VERDICT:
		*data_len = sizeof(uint32_t);
		return &trace->verdict;
	case NFTNL_TRACE_IIFTYPE:
		*data_len = sizeof(uint16_t);
		return &trace->iiftype;
	case NFTNL_TRACE_OIFTYPE:
		*data_len = sizeof(uint16_t);
		return &trace->oiftype;
	case NFTNL_TRACE_NFPROTO:
		*data_len = sizeof(uint32_t);
		return &trace->nfproto;
	case NFTNL_TRACE_POLICY:
		*data_len = sizeof(uint32_t);
		return &trace->policy;
	case NFTNL_TRACE_CT_DIRECTION:
		*data_len = sizeof(uint16_t);
		return &trace->ct.dir;
	case NFTNL_TRACE_CT_ID:
		*data_len = sizeof(uint32_t);
		return &trace->ct.id;
	case NFTNL_TRACE_CT_STATE:
		*data_len = sizeof(uint32_t);
		return &trace->ct.state;
	case NFTNL_TRACE_CT_STATUS:
		*data_len = sizeof(uint32_t);
		return &trace->ct.status;
	case __NFTNL_TRACE_MAX:
		break;
	}

	return NULL;
}

EXPORT_SYMBOL(nftnl_trace_get_str);
const char *nftnl_trace_get_str(const struct nftnl_trace *trace, uint16_t type)
{
	if (!nftnl_trace_is_set(trace, type))
		return NULL;

	switch (type) {
	case NFTNL_TRACE_CHAIN: return trace->chain;
	case NFTNL_TRACE_TABLE: return trace->table;
	case NFTNL_TRACE_JUMP_TARGET: return trace->jump_target;
	default: break;
	}
	return NULL;
}

EXPORT_SYMBOL(nftnl_trace_get_u16);
uint16_t nftnl_trace_get_u16(const struct nftnl_trace *trace, uint16_t type)
{
	const uint16_t *d;
	uint32_t dlen;

	d = nftnl_trace_get_data(trace, type, &dlen);
	if (d && dlen == sizeof(*d))
		return *d;

	return 0;
}

EXPORT_SYMBOL(nftnl_trace_get_u32);
uint32_t nftnl_trace_get_u32(const struct nftnl_trace *trace, uint16_t type)
{
	const uint32_t *d;
	uint32_t dlen;

	d = nftnl_trace_get_data(trace, type, &dlen);
	if (d && dlen == sizeof(*d))
		return *d;

	return 0;
}

EXPORT_SYMBOL(nftnl_trace_get_u64);
uint64_t nftnl_trace_get_u64(const struct nftnl_trace *trace, uint16_t type)
{
	const uint64_t *d;
	uint32_t dlen;

	d = nftnl_trace_get_data(trace, type, &dlen);
	if (d && dlen == sizeof(*d))
		return *d;

	return 0;
}

static bool nftnl_trace_nlmsg_parse_hdrdata(struct nlattr *attr,
					    struct nftnl_header_data *header)
{
	uint32_t len;

	if (!attr)
		return false;

	len = mnl_attr_get_payload_len(attr);

	header->data = malloc(len);
	if (header->data) {
		memcpy(header->data, mnl_attr_get_payload(attr), len);
		header->len = len;
		return true;
	}

	return false;
}

static int nftnl_trace_parse_verdict_cb(const struct nlattr *attr, void *data)
{
	int type = mnl_attr_get_type(attr);
	const struct nlattr **tb = data;

	switch (type) {
	case NFTA_VERDICT_CODE:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		tb[type] = attr;
		break;
	case NFTA_VERDICT_CHAIN:
		if (mnl_attr_validate(attr, MNL_TYPE_STRING) < 0)
			abi_breakage();
		tb[type] = attr;
		break;
	}

	return MNL_CB_OK;
}

static int nftnl_trace_parse_verdict(const struct nlattr *attr,
				     struct nftnl_trace *t)
{
	struct nlattr *tb[NFTA_VERDICT_MAX+1];

	if (mnl_attr_parse_nested(attr, nftnl_trace_parse_verdict_cb, tb) < 0)
		return -1;

	if (!tb[NFTA_VERDICT_CODE])
		abi_breakage();

	t->verdict = ntohl(mnl_attr_get_u32(tb[NFTA_VERDICT_CODE]));
	t->flags |= (1 << NFTNL_TRACE_VERDICT);

	switch (t->verdict) {
	case NFT_GOTO: /* fallthough */
	case NFT_JUMP:
		if (!tb[NFTA_VERDICT_CHAIN])
			abi_breakage();
		if (nftnl_parse_str_attr(tb[NFTA_VERDICT_CHAIN],
					 NFTNL_TRACE_JUMP_TARGET,
					 (const char **)&t->jump_target,
					 &t->flags) < 0)
			return -1;
		break;
	}
	return 0;
}

EXPORT_SYMBOL(nftnl_trace_nlmsg_parse);
int nftnl_trace_nlmsg_parse(const struct nlmsghdr *nlh, struct nftnl_trace *t)
{
	struct nfgenmsg *nfg = mnl_nlmsg_get_payload(nlh);
	struct nlattr *tb[NFTA_TRACE_MAX+1] = {};

	if (mnl_attr_parse(nlh, sizeof(*nfg), nftnl_trace_parse_attr_cb, tb) < 0)
		return -1;

	if (!tb[NFTA_TRACE_ID])
		abi_breakage();

	if (!tb[NFTA_TRACE_TYPE])
		abi_breakage();

	t->family = nfg->nfgen_family;
	t->flags |= (1 << NFTNL_TRACE_FAMILY);

	t->type = ntohl(mnl_attr_get_u32(tb[NFTA_TRACE_TYPE]));
	t->flags |= (1 << NFTNL_TRACE_TYPE);

	t->id = ntohl(mnl_attr_get_u32(tb[NFTA_TRACE_ID]));
	t->flags |= (1 << NFTNL_TRACE_ID);

	if (nftnl_parse_str_attr(tb[NFTA_TRACE_TABLE], NFTNL_TRACE_TABLE,
				 (const char **)&t->table, &t->flags) < 0)
		return -1;

	if (nftnl_parse_str_attr(tb[NFTA_TRACE_CHAIN], NFTNL_TRACE_CHAIN,
				 (const char **)&t->chain, &t->flags) < 0)
		return -1;

	if (tb[NFTA_TRACE_IIFTYPE]) {
		t->iiftype = ntohs(mnl_attr_get_u16(tb[NFTA_TRACE_IIFTYPE]));
		t->flags |= (1 << NFTNL_TRACE_IIFTYPE);
	}

	if (tb[NFTA_TRACE_IIF]) {
		t->iif = ntohl(mnl_attr_get_u32(tb[NFTA_TRACE_IIF]));
		t->flags |= (1 << NFTNL_TRACE_IIF);
	}

	if (tb[NFTA_TRACE_OIFTYPE]) {
		t->oiftype = ntohs(mnl_attr_get_u16(tb[NFTA_TRACE_OIFTYPE]));
		t->flags |= (1 << NFTNL_TRACE_OIFTYPE);
	}

	if (tb[NFTA_TRACE_OIF]) {
		t->oif = ntohl(mnl_attr_get_u32(tb[NFTA_TRACE_OIF]));
		t->flags |= (1 << NFTNL_TRACE_OIF);
	}

	if (tb[NFTA_TRACE_MARK]) {
		t->mark = ntohl(mnl_attr_get_u32(tb[NFTA_TRACE_MARK]));
		t->flags |= (1 << NFTNL_TRACE_MARK);
	}

	if (tb[NFTA_TRACE_RULE_HANDLE]) {
		t->rule_handle = be64toh(mnl_attr_get_u64(tb[NFTA_TRACE_RULE_HANDLE]));
		t->flags |= (1 << NFTNL_TRACE_RULE_HANDLE);
	}

	if (tb[NFTA_TRACE_VERDICT] &&
	    nftnl_trace_parse_verdict(tb[NFTA_TRACE_VERDICT], t) < 0)
		return -1;

	if (nftnl_trace_nlmsg_parse_hdrdata(tb[NFTA_TRACE_LL_HEADER], &t->ll))
		t->flags |= (1 << NFTNL_TRACE_LL_HEADER);

	if (nftnl_trace_nlmsg_parse_hdrdata(tb[NFTA_TRACE_NETWORK_HEADER], &t->nh))
		t->flags |= (1 << NFTNL_TRACE_NETWORK_HEADER);

	if (nftnl_trace_nlmsg_parse_hdrdata(tb[NFTA_TRACE_TRANSPORT_HEADER], &t->th))
		t->flags |= (1 << NFTNL_TRACE_TRANSPORT_HEADER);

	if (tb[NFTA_TRACE_NFPROTO]) {
		t->nfproto = ntohl(mnl_attr_get_u32(tb[NFTA_TRACE_NFPROTO]));
		t->flags |= (1 << NFTNL_TRACE_NFPROTO);
	}

	if (tb[NFTA_TRACE_POLICY]) {
		t->policy = ntohl(mnl_attr_get_u32(tb[NFTA_TRACE_POLICY]));
		t->flags |= (1 << NFTNL_TRACE_POLICY);
	}

	if (tb[NFTA_TRACE_MARK]) {
		t->mark = ntohl(mnl_attr_get_u32(tb[NFTA_TRACE_MARK]));
		t->flags |= (1 << NFTNL_TRACE_MARK);
	}

	if (tb[NFTA_TRACE_CT_DIRECTION]) {
		t->ct.dir = mnl_attr_get_u8(tb[NFTA_TRACE_CT_DIRECTION]);
		t->flags |= (1 << NFTNL_TRACE_CT_DIRECTION);
	}

	if (tb[NFTA_TRACE_CT_ID]) {
		/* NFT_CT_ID is expected to be in big endian */
		t->ct.id = mnl_attr_get_u32(tb[NFTA_TRACE_CT_ID]);
		t->flags |= (1 << NFTNL_TRACE_CT_ID);
	}

	if (tb[NFTA_TRACE_CT_STATE]) {
		t->ct.state = ntohl(mnl_attr_get_u32(tb[NFTA_TRACE_CT_STATE]));
		t->flags |= (1 << NFTNL_TRACE_CT_STATE);
	}

	if (tb[NFTA_TRACE_CT_STATUS]) {
		t->ct.status = ntohl(mnl_attr_get_u32(tb[NFTA_TRACE_CT_STATUS]));
		t->flags |= (1 << NFTNL_TRACE_CT_STATUS);
	}

	return 0;
}
