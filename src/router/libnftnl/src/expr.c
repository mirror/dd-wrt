/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2012 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This code has been sponsored by Sophos Astaro <http://www.sophos.com>
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
#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nf_tables.h>

#include <libnftnl/expr.h>

EXPORT_SYMBOL(nftnl_expr_alloc);
struct nftnl_expr *nftnl_expr_alloc(const char *name)
{
	struct nftnl_expr *expr;
	struct expr_ops *ops;

	ops = nftnl_expr_ops_lookup(name);
	if (ops == NULL)
		return NULL;

	expr = calloc(1, sizeof(struct nftnl_expr) + ops->alloc_len);
	if (expr == NULL)
		return NULL;

	/* Manually set expression name attribute */
	expr->flags |= (1 << NFTNL_EXPR_NAME);
	expr->ops = ops;

	if (ops->init)
		ops->init(expr);

	return expr;
}

EXPORT_SYMBOL(nftnl_expr_free);
void nftnl_expr_free(const struct nftnl_expr *expr)
{
	if (expr->ops->free)
		expr->ops->free(expr);

	xfree(expr);
}

EXPORT_SYMBOL(nftnl_expr_is_set);
bool nftnl_expr_is_set(const struct nftnl_expr *expr, uint16_t type)
{
	return expr->flags & (1 << type);
}

EXPORT_SYMBOL(nftnl_expr_set);
int nftnl_expr_set(struct nftnl_expr *expr, uint16_t type,
		   const void *data, uint32_t data_len)
{
	switch(type) {
	case NFTNL_EXPR_NAME:	/* cannot be modified */
		return 0;
	default:
		if (type < NFTNL_EXPR_BASE || type > expr->ops->nftnl_max_attr)
			return -1;

		if (!expr->ops->attr_policy)
			return -1;

		if (expr->ops->attr_policy[type].maxlen &&
		    expr->ops->attr_policy[type].maxlen < data_len)
			return -1;

		if (expr->ops->set(expr, type, data, data_len) < 0)
			return -1;
	}
	expr->flags |= (1 << type);
	return 0;
}

EXPORT_SYMBOL(nftnl_expr_set_u8);
void
nftnl_expr_set_u8(struct nftnl_expr *expr, uint16_t type, uint8_t data)
{
	nftnl_expr_set(expr, type, &data, sizeof(uint8_t));
}

EXPORT_SYMBOL(nftnl_expr_set_u16);
void
nftnl_expr_set_u16(struct nftnl_expr *expr, uint16_t type, uint16_t data)
{
	nftnl_expr_set(expr, type, &data, sizeof(uint16_t));
}

EXPORT_SYMBOL(nftnl_expr_set_u32);
void
nftnl_expr_set_u32(struct nftnl_expr *expr, uint16_t type, uint32_t data)
{
	nftnl_expr_set(expr, type, &data, sizeof(uint32_t));
}

EXPORT_SYMBOL(nftnl_expr_set_u64);
void
nftnl_expr_set_u64(struct nftnl_expr *expr, uint16_t type, uint64_t data)
{
	nftnl_expr_set(expr, type, &data, sizeof(uint64_t));
}

EXPORT_SYMBOL(nftnl_expr_set_str);
int nftnl_expr_set_str(struct nftnl_expr *expr, uint16_t type, const char *str)
{
	return nftnl_expr_set(expr, type, str, strlen(str) + 1);
}

EXPORT_SYMBOL(nftnl_expr_get);
const void *nftnl_expr_get(const struct nftnl_expr *expr,
			      uint16_t type, uint32_t *data_len)
{
	const void *ret;

	if (!(expr->flags & (1 << type)))
		return NULL;

	switch(type) {
	case NFTNL_EXPR_NAME:
		*data_len = strlen(expr->ops->name) + 1;
		ret = expr->ops->name;
		break;
	default:
		ret = expr->ops->get(expr, type, data_len);
		break;
	}

	return ret;
}

EXPORT_SYMBOL(nftnl_expr_get_u8);
uint8_t nftnl_expr_get_u8(const struct nftnl_expr *expr, uint16_t type)
{
	const void *data;
	uint32_t data_len;

	data = nftnl_expr_get(expr, type, &data_len);
	if (data == NULL)
		return 0;

	if (data_len != sizeof(uint8_t))
		return 0;

	return *((uint8_t *)data);
}

EXPORT_SYMBOL(nftnl_expr_get_u16);
uint16_t nftnl_expr_get_u16(const struct nftnl_expr *expr, uint16_t type)
{
	const void *data;
	uint32_t data_len;

	data = nftnl_expr_get(expr, type, &data_len);
	if (data == NULL)
		return 0;

	if (data_len != sizeof(uint16_t))
		return 0;

	return *((uint16_t *)data);
}

EXPORT_SYMBOL(nftnl_expr_get_u32);
uint32_t nftnl_expr_get_u32(const struct nftnl_expr *expr, uint16_t type)
{
	const void *data;
	uint32_t data_len;

	data = nftnl_expr_get(expr, type, &data_len);
	if (data == NULL)
		return 0;

	if (data_len != sizeof(uint32_t))
		return 0;

	return *((uint32_t *)data);
}

EXPORT_SYMBOL(nftnl_expr_get_u64);
uint64_t nftnl_expr_get_u64(const struct nftnl_expr *expr, uint16_t type)
{
	const void *data;
	uint32_t data_len;

	data = nftnl_expr_get(expr, type, &data_len);
	if (data == NULL)
		return 0;

	if (data_len != sizeof(uint64_t))
		return 0;

	return *((uint64_t *)data);
}

EXPORT_SYMBOL(nftnl_expr_get_str);
const char *nftnl_expr_get_str(const struct nftnl_expr *expr, uint16_t type)
{
	uint32_t data_len;

	return (const char *)nftnl_expr_get(expr, type, &data_len);
}

EXPORT_SYMBOL(nftnl_expr_build_payload);
void nftnl_expr_build_payload(struct nlmsghdr *nlh, struct nftnl_expr *expr)
{
	struct nlattr *nest;

	mnl_attr_put_strz(nlh, NFTA_EXPR_NAME, expr->ops->name);

	if (!expr->ops->build)
		return;

	nest = mnl_attr_nest_start(nlh, NFTA_EXPR_DATA);
	expr->ops->build(nlh, expr);
	mnl_attr_nest_end(nlh, nest);
}

static int nftnl_rule_parse_expr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_EXPR_MAX) < 0)
		return MNL_CB_OK;

	switch (type) {
	case NFTA_EXPR_NAME:
		if (mnl_attr_validate(attr, MNL_TYPE_STRING) < 0)
			abi_breakage();
		break;
	case NFTA_EXPR_DATA:
		if (mnl_attr_validate(attr, MNL_TYPE_NESTED) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

struct nftnl_expr *nftnl_expr_parse(struct nlattr *attr)
{
	struct nlattr *tb[NFTA_EXPR_MAX+1] = {};
	struct nftnl_expr *expr;

	if (mnl_attr_parse_nested(attr, nftnl_rule_parse_expr_cb, tb) < 0)
		goto err1;

	expr = nftnl_expr_alloc(mnl_attr_get_str(tb[NFTA_EXPR_NAME]));
	if (expr == NULL)
		goto err1;

	if (tb[NFTA_EXPR_DATA] &&
	    expr->ops->parse &&
	    expr->ops->parse(expr, tb[NFTA_EXPR_DATA]) < 0)
		goto err2;

	return expr;

err2:
	xfree(expr);
err1:
	return NULL;
}

EXPORT_SYMBOL(nftnl_expr_snprintf);
int nftnl_expr_snprintf(char *buf, size_t remain, const struct nftnl_expr *expr,
			uint32_t type, uint32_t flags)
{
	int ret;
	unsigned int offset = 0;

	if (remain)
		buf[0] = '\0';

	if (!expr->ops->output || type != NFTNL_OUTPUT_DEFAULT)
		return 0;

	ret = expr->ops->output(buf + offset, remain, flags, expr);
	SNPRINTF_BUFFER_SIZE(ret, remain, offset);

	return offset;
}

static int nftnl_expr_do_snprintf(char *buf, size_t size, const void *e,
				  uint32_t cmd, uint32_t type, uint32_t flags)
{
	return nftnl_expr_snprintf(buf, size, e, type, flags);
}

EXPORT_SYMBOL(nftnl_expr_fprintf);
int nftnl_expr_fprintf(FILE *fp, const struct nftnl_expr *expr, uint32_t type,
		       uint32_t flags)
{
	return nftnl_fprintf(fp, expr, NFTNL_CMD_UNSPEC, type, flags,
			     nftnl_expr_do_snprintf);
}
