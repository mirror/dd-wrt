#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <linux/netfilter/nf_tables.h>

#include "internal.h"
#include <libmnl/libmnl.h>
#include <libnftnl/expr.h>
#include <libnftnl/rule.h>

#define OSF_GENRE_SIZE	32

struct nftnl_expr_osf {
	enum nft_registers	dreg;
	uint8_t			ttl;
	uint32_t		flags;
};

static int nftnl_expr_osf_set(struct nftnl_expr *e, uint16_t type,
			      const void *data, uint32_t data_len)
{
	struct nftnl_expr_osf *osf = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_OSF_DREG:
		memcpy(&osf->dreg, data, data_len);
		break;
	case NFTNL_EXPR_OSF_TTL:
		memcpy(&osf->ttl, data, data_len);
		break;
	case NFTNL_EXPR_OSF_FLAGS:
		memcpy(&osf->flags, data, data_len);
		break;
	}
	return 0;
}

static const void *
nftnl_expr_osf_get(const struct nftnl_expr *e, uint16_t type,
		   uint32_t *data_len)
{
	struct nftnl_expr_osf *osf = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_OSF_DREG:
		*data_len = sizeof(osf->dreg);
		return &osf->dreg;
	case NFTNL_EXPR_OSF_TTL:
		*data_len = sizeof(osf->ttl);
		return &osf->ttl;
	case NFTNL_EXPR_OSF_FLAGS:
		*data_len = sizeof(osf->flags);
		return &osf->flags;
	}
	return NULL;
}

static int nftnl_expr_osf_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_OSF_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFTNL_EXPR_OSF_DREG:
	case NFTNL_EXPR_OSF_FLAGS:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;

	case NFTNL_EXPR_OSF_TTL:
		if (mnl_attr_validate(attr, MNL_TYPE_U8) < 0)
			abi_breakage();
		break;

	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static void
nftnl_expr_osf_build(struct nlmsghdr *nlh, const struct nftnl_expr *e)
{
	struct nftnl_expr_osf *osf = nftnl_expr_data(e);

	if (e->flags & (1 << NFTNL_EXPR_OSF_DREG))
		mnl_attr_put_u32(nlh, NFTA_OSF_DREG, htonl(osf->dreg));
	if (e->flags & (1 << NFTNL_EXPR_OSF_TTL))
		mnl_attr_put_u8(nlh, NFTA_OSF_TTL, osf->ttl);
	if (e->flags & (1 << NFTNL_EXPR_OSF_FLAGS))
		if (osf->flags)
			mnl_attr_put_u32(nlh, NFTA_OSF_FLAGS, htonl(osf->flags));
}

static int
nftnl_expr_osf_parse(struct nftnl_expr *e, struct nlattr *attr)
{
	struct nftnl_expr_osf *osf = nftnl_expr_data(e);
	struct nlattr *tb[NFTA_OSF_MAX + 1] = {};

	if (mnl_attr_parse_nested(attr, nftnl_expr_osf_cb, tb) < 0)
		return -1;

	if (tb[NFTA_OSF_DREG]) {
		osf->dreg = ntohl(mnl_attr_get_u32(tb[NFTA_OSF_DREG]));
		e->flags |= (1 << NFTNL_EXPR_OSF_DREG);
	}

	if (tb[NFTA_OSF_TTL]) {
		osf->ttl = mnl_attr_get_u8(tb[NFTA_OSF_TTL]);
		e->flags |= (1 << NFTNL_EXPR_OSF_TTL);
	}

	if (tb[NFTA_OSF_FLAGS]) {
		osf->flags = ntohl(mnl_attr_get_u32(tb[NFTA_OSF_FLAGS]));
		e->flags |= (1 << NFTNL_EXPR_OSF_FLAGS);
	}

	return 0;
}

static int
nftnl_expr_osf_snprintf(char *buf, size_t len,
			uint32_t flags, const struct nftnl_expr *e)
{
	struct nftnl_expr_osf *osf = nftnl_expr_data(e);
	int ret, offset = 0;

	if (e->flags & (1 << NFTNL_EXPR_OSF_DREG)) {
		ret = snprintf(buf, len, "dreg %u ", osf->dreg);
		SNPRINTF_BUFFER_SIZE(ret, len, offset);
	}

	return offset;
}

static struct attr_policy osf_attr_policy[__NFTNL_EXPR_OSF_MAX] = {
	[NFTNL_EXPR_OSF_DREG]  = { .maxlen = sizeof(uint32_t) },
	[NFTNL_EXPR_OSF_TTL]   = { .maxlen = sizeof(uint8_t) },
	[NFTNL_EXPR_OSF_FLAGS] = { .maxlen = sizeof(uint32_t) },
};

struct expr_ops expr_ops_osf = {
	.name		= "osf",
	.alloc_len	= sizeof(struct nftnl_expr_osf),
	.nftnl_max_attr	= __NFTNL_EXPR_OSF_MAX - 1,
	.attr_policy	= osf_attr_policy,
	.set		= nftnl_expr_osf_set,
	.get		= nftnl_expr_osf_get,
	.parse		= nftnl_expr_osf_parse,
	.build		= nftnl_expr_osf_build,
	.output		= nftnl_expr_osf_snprintf,
};
