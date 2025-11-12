#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <errno.h>
#include <inttypes.h>

#include <linux/netfilter/nf_tables.h>

#include "internal.h"
#include <libmnl/libmnl.h>
#include <libnftnl/object.h>

#include "obj.h"

static int nftnl_obj_synproxy_set(struct nftnl_obj *e, uint16_t type,
				  const void *data, uint32_t data_len)
{
	struct nftnl_obj_synproxy *synproxy = nftnl_obj_data(e);

	switch (type) {
	case NFTNL_OBJ_SYNPROXY_MSS:
		memcpy(&synproxy->mss, data, data_len);
		break;
	case NFTNL_OBJ_SYNPROXY_WSCALE:
		memcpy(&synproxy->wscale, data, data_len);
		break;
	case NFTNL_OBJ_SYNPROXY_FLAGS:
		memcpy(&synproxy->flags, data, data_len);
		break;
	}
	return 0;
}

static const void *nftnl_obj_synproxy_get(const struct nftnl_obj *e,
					  uint16_t type, uint32_t *data_len)
{
	struct nftnl_obj_synproxy *synproxy = nftnl_obj_data(e);

	switch (type) {
	case NFTNL_OBJ_SYNPROXY_MSS:
		*data_len = sizeof(synproxy->mss);
		return &synproxy->mss;
	case NFTNL_OBJ_SYNPROXY_WSCALE:
		*data_len = sizeof(synproxy->wscale);
		return &synproxy->wscale;
	case NFTNL_OBJ_SYNPROXY_FLAGS:
		*data_len = sizeof(synproxy->flags);
		return &synproxy->flags;
	}
	return NULL;
}

static int nftnl_obj_synproxy_cb(const struct nlattr *attr, void *data)
{
	int type = mnl_attr_get_type(attr);
	const struct nlattr **tb = data;

	if (mnl_attr_type_valid(attr, NFTA_SYNPROXY_MAX) < 0)
		return MNL_CB_OK;

	switch (type) {
	case NFTA_SYNPROXY_MSS:
		if (mnl_attr_validate(attr, MNL_TYPE_U16) < 0)
			abi_breakage();
		break;
	case NFTA_SYNPROXY_WSCALE:
		if (mnl_attr_validate(attr, MNL_TYPE_U8) < 0)
			abi_breakage();
		break;
	case NFTA_SYNPROXY_FLAGS:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static void nftnl_obj_synproxy_build(struct nlmsghdr *nlh,
				     const struct nftnl_obj *e)
{
	struct nftnl_obj_synproxy *synproxy = nftnl_obj_data(e);

	if (e->flags & (1 << NFTNL_OBJ_SYNPROXY_MSS))
		mnl_attr_put_u16(nlh, NFTA_SYNPROXY_MSS, htons(synproxy->mss));
	if (e->flags & (1 << NFTNL_OBJ_SYNPROXY_WSCALE))
		mnl_attr_put_u8(nlh, NFTA_SYNPROXY_WSCALE, synproxy->wscale);
	if (e->flags & (1 << NFTNL_OBJ_SYNPROXY_FLAGS))
		mnl_attr_put_u32(nlh, NFTA_SYNPROXY_FLAGS,
				 htonl(synproxy->flags));
}

static int nftnl_obj_synproxy_parse(struct nftnl_obj *e, struct nlattr *attr)
{
	struct nftnl_obj_synproxy *synproxy = nftnl_obj_data(e);
	struct nlattr *tb[NFTA_SYNPROXY_MAX + 1] = {};

	if (mnl_attr_parse_nested(attr, nftnl_obj_synproxy_cb, tb) < 0)
		return -1;

	if (tb[NFTA_SYNPROXY_MSS]) {
		synproxy->mss = ntohs(mnl_attr_get_u16(tb[NFTA_SYNPROXY_MSS]));
		e->flags |= (1 << NFTNL_OBJ_SYNPROXY_MSS);
	}
	if (tb[NFTA_SYNPROXY_WSCALE]) {
		synproxy->wscale = mnl_attr_get_u8(tb[NFTA_SYNPROXY_WSCALE]);
		e->flags |= (1 << NFTNL_OBJ_SYNPROXY_WSCALE);
	}
	if (tb[NFTA_SYNPROXY_FLAGS]) {
		synproxy->flags = ntohl(mnl_attr_get_u32(tb[NFTA_SYNPROXY_FLAGS]));
		e->flags |= (1 << NFTNL_OBJ_SYNPROXY_FLAGS);
	}

	return 0;
}

static int nftnl_obj_synproxy_snprintf(char *buf, size_t len,
				    uint32_t flags,
				    const struct nftnl_obj *e)
{
	struct nftnl_obj_synproxy *synproxy = nftnl_obj_data(e);
        int ret, offset = 0;

        if (e->flags & (1 << NFTNL_OBJ_SYNPROXY_MSS) &&
            e->flags & (1 << NFTNL_OBJ_SYNPROXY_WSCALE)) {
                ret = snprintf(buf, len, "mss %u wscale %u ", synproxy->mss,
                               synproxy->wscale);
                SNPRINTF_BUFFER_SIZE(ret, len, offset);
        }

        return offset;
}

static struct attr_policy obj_synproxy_attr_policy[__NFTNL_OBJ_SYNPROXY_MAX] = {
	[NFTNL_OBJ_SYNPROXY_MSS]	= { .maxlen = sizeof(uint16_t) },
	[NFTNL_OBJ_SYNPROXY_WSCALE]	= { .maxlen = sizeof(uint8_t) },
	[NFTNL_OBJ_SYNPROXY_FLAGS]	= { .maxlen = sizeof(uint32_t) },
};

struct obj_ops obj_ops_synproxy = {
	.name		= "synproxy",
	.type		= NFT_OBJECT_SYNPROXY,
	.alloc_len	= sizeof(struct nftnl_obj_synproxy),
	.nftnl_max_attr	= __NFTNL_OBJ_SYNPROXY_MAX - 1,
	.attr_policy	= obj_synproxy_attr_policy,
	.set		= nftnl_obj_synproxy_set,
	.get		= nftnl_obj_synproxy_get,
	.parse		= nftnl_obj_synproxy_parse,
	.build		= nftnl_obj_synproxy_build,
	.output		= nftnl_obj_synproxy_snprintf,
};
