#include "internal.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h> /* for memcpy */
#include <arpa/inet.h>
#include <errno.h>
#include <libmnl/libmnl.h>
#include <linux/netfilter/nf_tables.h>
#include <libnftnl/rule.h>
#include <libnftnl/expr.h>

struct nftnl_expr_flow {
	char			*table_name;
};

static int nftnl_expr_flow_set(struct nftnl_expr *e, uint16_t type,
			       const void *data, uint32_t data_len)
{
	struct nftnl_expr_flow *flow = nftnl_expr_data(e);

	switch (type) {
	case NFTNL_EXPR_FLOW_TABLE_NAME:
		flow->table_name = strdup((const char *)data);
		if (!flow->table_name)
			return -1;
		break;
	}
	return 0;
}

static const void *nftnl_expr_flow_get(const struct nftnl_expr *e,
				       uint16_t type, uint32_t *data_len)
{
	struct nftnl_expr_flow *flow = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_FLOW_TABLE_NAME:
		*data_len = strlen(flow->table_name) + 1;
		return flow->table_name;
	}
	return NULL;
}

static int nftnl_expr_flow_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_FLOW_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFTA_FLOW_TABLE_NAME:
		if (mnl_attr_validate(attr, MNL_TYPE_STRING) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static void nftnl_expr_flow_build(struct nlmsghdr *nlh,
				  const struct nftnl_expr *e)
{
	struct nftnl_expr_flow *flow = nftnl_expr_data(e);

	if (e->flags & (1 << NFTNL_EXPR_FLOW_TABLE_NAME))
		mnl_attr_put_strz(nlh, NFTA_FLOW_TABLE_NAME, flow->table_name);
}

static int nftnl_expr_flow_parse(struct nftnl_expr *e, struct nlattr *attr)
{
	struct nftnl_expr_flow *flow = nftnl_expr_data(e);
	struct nlattr *tb[NFTA_FLOW_MAX+1] = {};
	int ret = 0;

	if (mnl_attr_parse_nested(attr, nftnl_expr_flow_cb, tb) < 0)
		return -1;

	if (tb[NFTA_FLOW_TABLE_NAME]) {
		flow->table_name =
			strdup(mnl_attr_get_str(tb[NFTA_FLOW_TABLE_NAME]));
		if (!flow->table_name)
			return -1;
		e->flags |= (1 << NFTNL_EXPR_FLOW_TABLE_NAME);
	}

	return ret;
}

static int nftnl_expr_flow_snprintf(char *buf, size_t remain,
				    uint32_t flags, const struct nftnl_expr *e)
{
	struct nftnl_expr_flow *l = nftnl_expr_data(e);
	int offset = 0, ret;

	ret = snprintf(buf, remain, "flowtable %s ", l->table_name);
	SNPRINTF_BUFFER_SIZE(ret, remain, offset);

	return offset;
}

static void nftnl_expr_flow_free(const struct nftnl_expr *e)
{
	struct nftnl_expr_flow *flow = nftnl_expr_data(e);

	xfree(flow->table_name);
}

static struct attr_policy flow_offload_attr_policy[__NFTNL_EXPR_FLOW_MAX] = {
	[NFTNL_EXPR_FLOW_TABLE_NAME] = { .maxlen = NFT_NAME_MAXLEN },
};

struct expr_ops expr_ops_flow = {
	.name		= "flow_offload",
	.alloc_len	= sizeof(struct nftnl_expr_flow),
	.nftnl_max_attr	= __NFTNL_EXPR_FLOW_MAX - 1,
	.attr_policy	= flow_offload_attr_policy,
	.free		= nftnl_expr_flow_free,
	.set		= nftnl_expr_flow_set,
	.get		= nftnl_expr_flow_get,
	.parse		= nftnl_expr_flow_parse,
	.build		= nftnl_expr_flow_build,
	.output		= nftnl_expr_flow_snprintf,
};
