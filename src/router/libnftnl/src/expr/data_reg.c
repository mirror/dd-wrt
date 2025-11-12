/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2012 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This code has been sponsored by Sophos Astaro <http://www.sophos.com>
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>

#include <libmnl/libmnl.h>
#include <linux/netfilter.h>
#include <linux/netfilter/nf_tables.h>
#include <libnftnl/expr.h>
#include <libnftnl/rule.h>
#include "internal.h"

static int
nftnl_data_reg_value_snprintf_default(char *buf, size_t remain,
				      const union nftnl_data_reg *reg,
				      uint32_t flags)
{
	const char *pfx = flags & DATA_F_NOPFX ? "" : "0x";
	int offset = 0, ret, i;



	for (i = 0; i < div_round_up(reg->len, sizeof(uint32_t)); i++) {
		ret = snprintf(buf + offset, remain,
			       "%s%.8x ", pfx, reg->val[i]);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	}

	return offset;
}

static int
nftnl_data_reg_verdict_snprintf_def(char *buf, size_t size,
				    const union nftnl_data_reg *reg,
				    uint32_t flags)
{
	int remain = size, offset = 0, ret = 0;

	ret = snprintf(buf, size, "%s ", nftnl_verdict2str(reg->verdict));
	SNPRINTF_BUFFER_SIZE(ret, remain, offset);

	if (reg->chain != NULL) {
		ret = snprintf(buf + offset, remain, "-> %s ", reg->chain);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	}

	return offset;
}

int nftnl_data_reg_snprintf(char *buf, size_t size,
			    const union nftnl_data_reg *reg,
			    uint32_t flags, int reg_type)
{
	switch(reg_type) {
	case DATA_VALUE:
		return nftnl_data_reg_value_snprintf_default(buf, size,
							     reg, flags);
	case DATA_VERDICT:
	case DATA_CHAIN:
		return nftnl_data_reg_verdict_snprintf_def(buf, size,
							   reg, flags);
	default:
		return -1;
	}
}

static int nftnl_data_parse_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_DATA_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFTA_DATA_VALUE:
		if (mnl_attr_validate(attr, MNL_TYPE_BINARY) < 0)
			abi_breakage();
		break;
	case NFTA_DATA_VERDICT:
		if (mnl_attr_validate(attr, MNL_TYPE_NESTED) < 0)
			abi_breakage();
		break;
	}
	tb[type] = attr;
	return MNL_CB_OK;
}

static int nftnl_verdict_parse_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_VERDICT_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFTA_VERDICT_CODE:
	case NFTA_VERDICT_CHAIN_ID:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	case NFTA_VERDICT_CHAIN:
		if (mnl_attr_validate(attr, MNL_TYPE_STRING) < 0)
			abi_breakage();
		break;
	}
	tb[type] = attr;
	return MNL_CB_OK;
}

static int
nftnl_parse_verdict(union nftnl_data_reg *data, const struct nlattr *attr, int *type)
{
	struct nlattr *tb[NFTA_VERDICT_MAX+1];

	if (mnl_attr_parse_nested(attr, nftnl_verdict_parse_cb, tb) < 0)
		return -1;

	if (!tb[NFTA_VERDICT_CODE])
		return -1;

	data->verdict = ntohl(mnl_attr_get_u32(tb[NFTA_VERDICT_CODE]));

	switch(data->verdict) {
	case NF_ACCEPT:
	case NF_DROP:
	case NF_QUEUE:
	case NFT_CONTINUE:
	case NFT_BREAK:
	case NFT_RETURN:
		if (type)
			*type = DATA_VERDICT;
		data->len = sizeof(data->verdict);
		break;
	case NFT_JUMP:
	case NFT_GOTO:
		if (!tb[NFTA_VERDICT_CHAIN])
			return -1;

		data->chain = strdup(mnl_attr_get_str(tb[NFTA_VERDICT_CHAIN]));
		if (!data->chain)
			return -1;

		if (type)
			*type = DATA_CHAIN;
		break;
	default:
		return -1;
	}

	return 0;
}

static int
__nftnl_parse_data(union nftnl_data_reg *data, const struct nlattr *attr)
{
	void *orig = mnl_attr_get_payload(attr);
	uint32_t data_len = mnl_attr_get_payload_len(attr);

	if (data_len == 0)
		return -1;

	if (data_len > sizeof(data->val))
		return -1;

	memcpy(data->val, orig, data_len);
	data->len = data_len;

	return 0;
}

int nftnl_parse_data(union nftnl_data_reg *data, struct nlattr *attr, int *type)
{
	struct nlattr *tb[NFTA_DATA_MAX+1] = {};
	int ret = 0;

	if (mnl_attr_parse_nested(attr, nftnl_data_parse_cb, tb) < 0)
		return -1;

	if (tb[NFTA_DATA_VALUE]) {
		if (type)
			*type = DATA_VALUE;

		ret = __nftnl_parse_data(data, tb[NFTA_DATA_VALUE]);
		if (ret < 0)
			return ret;
	}
	if (tb[NFTA_DATA_VERDICT])
		ret = nftnl_parse_verdict(data, tb[NFTA_DATA_VERDICT], type);

	return ret;
}

int nftnl_data_cpy(union nftnl_data_reg *dreg, const void *src, uint32_t len)
{
	int ret = 0;

	if (len > sizeof(dreg->val)) {
		len = sizeof(dreg->val);
		ret = -1;
	}

	memcpy(dreg->val, src, len);
	dreg->len = len;
	return ret;
}
