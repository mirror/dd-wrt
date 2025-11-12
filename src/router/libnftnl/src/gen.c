/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2012-2014 by Pablo Neira Ayuso <pablo@netfilter.org>
 */
#include "internal.h"

#include <time.h>
#include <endian.h>
#include <stdint.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>

#include <libmnl/libmnl.h>
#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nf_tables.h>

#include <libnftnl/gen.h>

struct nftnl_gen {
	uint32_t id;

	uint32_t flags;
};

EXPORT_SYMBOL(nftnl_gen_alloc);
struct nftnl_gen *nftnl_gen_alloc(void)
{
	return calloc(1, sizeof(struct nftnl_gen));
}

EXPORT_SYMBOL(nftnl_gen_free);
void nftnl_gen_free(const struct nftnl_gen *gen)
{
	xfree(gen);
}

EXPORT_SYMBOL(nftnl_gen_is_set);
bool nftnl_gen_is_set(const struct nftnl_gen *gen, uint16_t attr)
{
	return gen->flags & (1 << attr);
}

EXPORT_SYMBOL(nftnl_gen_unset);
void nftnl_gen_unset(struct nftnl_gen *gen, uint16_t attr)
{
	if (!(gen->flags & (1 << attr)))
		return;

	switch (attr) {
	case NFTNL_GEN_ID:
		break;
	}
	gen->flags &= ~(1 << attr);
}

static uint32_t nftnl_gen_validate[NFTNL_GEN_MAX + 1] = {
	[NFTNL_GEN_ID]	= sizeof(uint32_t),
};

EXPORT_SYMBOL(nftnl_gen_set_data);
int nftnl_gen_set_data(struct nftnl_gen *gen, uint16_t attr,
		       const void *data, uint32_t data_len)
{
	nftnl_assert_attr_exists(attr, NFTNL_GEN_MAX);
	nftnl_assert_validate(data, nftnl_gen_validate, attr, data_len);

	switch (attr) {
	case NFTNL_GEN_ID:
		memcpy(&gen->id, data, sizeof(gen->id));
		break;
	}
	gen->flags |= (1 << attr);
	return 0;
}

int nftnl_gen_set(struct nftnl_gen *gen, uint16_t attr, const void *data) __visible;
int nftnl_gen_set(struct nftnl_gen *gen, uint16_t attr, const void *data)
{
	return nftnl_gen_set_data(gen, attr, data, nftnl_gen_validate[attr]);
}

EXPORT_SYMBOL(nftnl_gen_set_u32);
void nftnl_gen_set_u32(struct nftnl_gen *gen, uint16_t attr, uint32_t val)
{
	nftnl_gen_set_data(gen, attr, &val, sizeof(uint32_t));
}

EXPORT_SYMBOL(nftnl_gen_get_data);
const void *nftnl_gen_get_data(const struct nftnl_gen *gen, uint16_t attr,
			       uint32_t *data_len)
{
	if (!(gen->flags & (1 << attr)))
		return NULL;

	switch(attr) {
	case NFTNL_GEN_ID:
		*data_len = sizeof(gen->id);
		return &gen->id;
	}
	return NULL;
}

EXPORT_SYMBOL(nftnl_gen_get);
const void *nftnl_gen_get(const struct nftnl_gen *gen, uint16_t attr)
{
	uint32_t data_len;
	return nftnl_gen_get_data(gen, attr, &data_len);
}

EXPORT_SYMBOL(nftnl_gen_get_u32);
uint32_t nftnl_gen_get_u32(const struct nftnl_gen *gen, uint16_t attr)
{
	const void *ret = nftnl_gen_get(gen, attr);
	return ret == NULL ? 0 : *((uint32_t *)ret);
}

static int nftnl_gen_parse_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_GEN_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFTA_GEN_ID:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

EXPORT_SYMBOL(nftnl_gen_nlmsg_parse);
int nftnl_gen_nlmsg_parse(const struct nlmsghdr *nlh, struct nftnl_gen *gen)
{
	struct nlattr *tb[NFTA_GEN_MAX + 1] = {};

	if (mnl_attr_parse(nlh, sizeof(struct nfgenmsg),
			   nftnl_gen_parse_attr_cb, tb) < 0)
		return -1;

	if (tb[NFTA_GEN_ID]) {
		gen->id = ntohl(mnl_attr_get_u32(tb[NFTA_GEN_ID]));
		gen->flags |= (1 << NFTNL_GEN_ID);
	}
	return 0;
}

static int nftnl_gen_cmd_snprintf(char *buf, size_t remain,
				  const struct nftnl_gen *gen, uint32_t cmd,
				  uint32_t type, uint32_t flags)
{
	int ret, offset = 0;

	if (type != NFTNL_OUTPUT_DEFAULT)
		return -1;

	ret = snprintf(buf, remain, "ruleset generation ID %u", gen->id);
	SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	return offset;
}

EXPORT_SYMBOL(nftnl_gen_snprintf);
int nftnl_gen_snprintf(char *buf, size_t size, const struct nftnl_gen *gen,
		       uint32_t type, uint32_t flags)
{
	if (size)
		buf[0] = '\0';

	return nftnl_gen_cmd_snprintf(buf, size, gen, nftnl_flag2cmd(flags), type,
				    flags);
}

static int nftnl_gen_do_snprintf(char *buf, size_t size, const void *gen,
				 uint32_t cmd, uint32_t type, uint32_t flags)
{
	return nftnl_gen_snprintf(buf, size, gen, type, flags);
}

EXPORT_SYMBOL(nftnl_gen_fprintf);
int nftnl_gen_fprintf(FILE *fp, const struct nftnl_gen *gen, uint32_t type,
		      uint32_t flags)
{
	return nftnl_fprintf(fp, gen, NFTNL_CMD_UNSPEC, type, flags,
			   nftnl_gen_do_snprintf);
}
