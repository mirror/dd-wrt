/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2018 by Pablo Neira Ayuso <pablo@netfilter.org>
 */

#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <errno.h>
#include <inttypes.h>

#include <linux/netfilter/nf_tables.h>

#include <libmnl/libmnl.h>
#include <libnftnl/object.h>

#include "internal.h"
#include "obj.h"

static int
nftnl_obj_tunnel_set(struct nftnl_obj *e, uint16_t type,
		     const void *data, uint32_t data_len)
{
	struct nftnl_obj_tunnel *tun = nftnl_obj_data(e);

	switch (type) {
	case NFTNL_OBJ_TUNNEL_ID:
		memcpy(&tun->id, data, data_len);
		break;
	case NFTNL_OBJ_TUNNEL_IPV4_SRC:
		memcpy(&tun->src_v4, data, data_len);
		break;
	case NFTNL_OBJ_TUNNEL_IPV4_DST:
		memcpy(&tun->dst_v4, data, data_len);
		break;
	case NFTNL_OBJ_TUNNEL_IPV6_SRC:
		memcpy(&tun->src_v6, data, data_len);
		break;
	case NFTNL_OBJ_TUNNEL_IPV6_DST:
		memcpy(&tun->dst_v6, data, data_len);
		break;
	case NFTNL_OBJ_TUNNEL_IPV6_FLOWLABEL:
		memcpy(&tun->flowlabel, data, data_len);
		break;
	case NFTNL_OBJ_TUNNEL_SPORT:
		memcpy(&tun->sport, data, data_len);
		break;
	case NFTNL_OBJ_TUNNEL_DPORT:
		memcpy(&tun->dport, data, data_len);
		break;
	case NFTNL_OBJ_TUNNEL_FLAGS:
		memcpy(&tun->tun_flags, data, data_len);
		break;
	case NFTNL_OBJ_TUNNEL_TOS:
		memcpy(&tun->tun_tos, data, data_len);
		break;
	case NFTNL_OBJ_TUNNEL_TTL:
		memcpy(&tun->tun_ttl, data, data_len);
		break;
	case NFTNL_OBJ_TUNNEL_OPTS:
		memcpy(&tun->tun_opts, data, data_len);
		break;
	}
	return 0;
}

static const void *
nftnl_obj_tunnel_get(const struct nftnl_obj *e, uint16_t type,
		     uint32_t *data_len)
{
	struct nftnl_obj_tunnel *tun = nftnl_obj_data(e);

	switch (type) {
	case NFTNL_OBJ_TUNNEL_ID:
		*data_len = sizeof(tun->id);
		return &tun->id;
	case NFTNL_OBJ_TUNNEL_IPV4_SRC:
		*data_len = sizeof(tun->src_v4);
		return &tun->src_v4;
	case NFTNL_OBJ_TUNNEL_IPV4_DST:
		*data_len = sizeof(tun->dst_v4);
		return &tun->dst_v4;
	case NFTNL_OBJ_TUNNEL_IPV6_SRC:
		*data_len = sizeof(tun->src_v6);
		return &tun->src_v6;
	case NFTNL_OBJ_TUNNEL_IPV6_DST:
		*data_len = sizeof(tun->dst_v6);
		return &tun->dst_v6;
	case NFTNL_OBJ_TUNNEL_IPV6_FLOWLABEL:
		*data_len = sizeof(tun->flowlabel);
		return &tun->flowlabel;
	case NFTNL_OBJ_TUNNEL_SPORT:
		*data_len = sizeof(tun->sport);
		return &tun->sport;
	case NFTNL_OBJ_TUNNEL_DPORT:
		*data_len = sizeof(tun->dport);
		return &tun->dport;
	case NFTNL_OBJ_TUNNEL_FLAGS:
		*data_len = sizeof(tun->tun_flags);
		return &tun->tun_flags;
	case NFTNL_OBJ_TUNNEL_TOS:
		*data_len = sizeof(tun->tun_tos);
		return &tun->tun_tos;
	case NFTNL_OBJ_TUNNEL_TTL:
		*data_len = sizeof(tun->tun_ttl);
		return &tun->tun_ttl;
	case NFTNL_OBJ_TUNNEL_OPTS:
		*data_len = sizeof(tun->tun_opts);
		return &tun->tun_opts;
	}
	return NULL;
}

static int nftnl_obj_tunnel_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_TUNNEL_KEY_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFTA_TUNNEL_KEY_ID:
	case NFTA_TUNNEL_KEY_FLAGS:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	case NFTA_TUNNEL_KEY_IP:
	case NFTA_TUNNEL_KEY_IP6:
	case NFTA_TUNNEL_KEY_OPTS:
		if (mnl_attr_validate(attr, MNL_TYPE_NESTED) < 0)
			abi_breakage();
		break;
	case NFTA_TUNNEL_KEY_SPORT:
	case NFTA_TUNNEL_KEY_DPORT:
		if (mnl_attr_validate(attr, MNL_TYPE_U16) < 0)
			abi_breakage();
		break;
	case NFTA_TUNNEL_KEY_TOS:
	case NFTA_TUNNEL_KEY_TTL:
		if (mnl_attr_validate(attr, MNL_TYPE_U8) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static void nftnl_tunnel_opts_build(struct nlmsghdr *nlh,
				    struct nftnl_tunnel_opts *opts);

static void
nftnl_obj_tunnel_build(struct nlmsghdr *nlh, const struct nftnl_obj *e)
{
	struct nftnl_obj_tunnel *tun = nftnl_obj_data(e);
	struct nlattr *nest;

	if (e->flags & (1 << NFTNL_OBJ_TUNNEL_ID))
		mnl_attr_put_u32(nlh, NFTA_TUNNEL_KEY_ID, htonl(tun->id));
	if (e->flags & (1 << NFTNL_OBJ_TUNNEL_IPV4_SRC) ||
	    e->flags & (1 << NFTNL_OBJ_TUNNEL_IPV4_DST)) {
		nest = mnl_attr_nest_start(nlh, NFTA_TUNNEL_KEY_IP);
		if (e->flags & (1 << NFTNL_OBJ_TUNNEL_IPV4_SRC))
			mnl_attr_put_u32(nlh, NFTA_TUNNEL_KEY_IP_SRC, tun->src_v4);
		if (e->flags & (1 << NFTNL_OBJ_TUNNEL_IPV4_DST))
			mnl_attr_put_u32(nlh, NFTA_TUNNEL_KEY_IP_DST, tun->dst_v4);
		mnl_attr_nest_end(nlh, nest);
	}
	if (e->flags & (1 << NFTNL_OBJ_TUNNEL_IPV6_SRC) ||
	    e->flags & (1 << NFTNL_OBJ_TUNNEL_IPV6_DST)) {
		nest = mnl_attr_nest_start(nlh, NFTA_TUNNEL_KEY_IP6);
		if (e->flags & (1 << NFTNL_OBJ_TUNNEL_IPV6_SRC))
			mnl_attr_put(nlh, NFTA_TUNNEL_KEY_IP6_SRC,
				     sizeof(tun->src_v6), &tun->src_v6);
		if (e->flags & (1 << NFTNL_OBJ_TUNNEL_IPV6_DST))
			mnl_attr_put(nlh, NFTA_TUNNEL_KEY_IP6_DST,
				     sizeof(tun->dst_v6), &tun->dst_v6);
		if (e->flags & (1 << NFTNL_OBJ_TUNNEL_IPV6_FLOWLABEL))
			mnl_attr_put_u32(nlh, NFTA_TUNNEL_KEY_IP6_FLOWLABEL,
					 htonl(tun->flowlabel));
		mnl_attr_nest_end(nlh, nest);
	}
	if (e->flags & (1 << NFTNL_OBJ_TUNNEL_SPORT))
		mnl_attr_put_u16(nlh, NFTA_TUNNEL_KEY_SPORT, htons(tun->sport));
	if (e->flags & (1 << NFTNL_OBJ_TUNNEL_DPORT))
		mnl_attr_put_u16(nlh, NFTA_TUNNEL_KEY_DPORT, htons(tun->dport));
	if (e->flags & (1 << NFTNL_OBJ_TUNNEL_TOS))
		mnl_attr_put_u8(nlh, NFTA_TUNNEL_KEY_TOS, tun->tun_tos);
	if (e->flags & (1 << NFTNL_OBJ_TUNNEL_TTL))
		mnl_attr_put_u8(nlh, NFTA_TUNNEL_KEY_TTL, tun->tun_ttl);
	if (e->flags & (1 << NFTNL_OBJ_TUNNEL_FLAGS))
		mnl_attr_put_u32(nlh, NFTA_TUNNEL_KEY_FLAGS, htonl(tun->tun_flags));
	if (e->flags & (1 << NFTNL_OBJ_TUNNEL_OPTS))
		nftnl_tunnel_opts_build(nlh, tun->tun_opts);
}

static int nftnl_obj_tunnel_ip_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_TUNNEL_KEY_MAX) < 0)
		return MNL_CB_OK;

	switch (type) {
	case NFTA_TUNNEL_KEY_IP_SRC:
	case NFTA_TUNNEL_KEY_IP_DST:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static int nftnl_obj_tunnel_parse_ip(struct nftnl_obj *e, struct nlattr *attr,
				     struct nftnl_obj_tunnel *tun)
{
	struct nlattr *tb[NFTA_TUNNEL_KEY_IP_MAX + 1] = {};

	if (mnl_attr_parse_nested(attr, nftnl_obj_tunnel_ip_cb, tb) < 0)
		return -1;

	if (tb[NFTA_TUNNEL_KEY_IP_SRC]) {
		tun->src_v4 = mnl_attr_get_u32(tb[NFTA_TUNNEL_KEY_IP_SRC]);
		e->flags |= (1 << NFTNL_OBJ_TUNNEL_IPV4_SRC);
	}
	if (tb[NFTA_TUNNEL_KEY_IP_DST]) {
		tun->dst_v4 = mnl_attr_get_u32(tb[NFTA_TUNNEL_KEY_IP_DST]);
		e->flags |= (1 << NFTNL_OBJ_TUNNEL_IPV4_DST);
	}

	return 0;
}

static int nftnl_obj_tunnel_ip6_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_TUNNEL_KEY_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFTA_TUNNEL_KEY_IP6_SRC:
	case NFTA_TUNNEL_KEY_IP6_DST:
		if (mnl_attr_validate(attr, MNL_TYPE_BINARY) < 0)
			abi_breakage();
		break;
	case NFTA_TUNNEL_KEY_IP6_FLOWLABEL:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static int nftnl_obj_tunnel_parse_ip6(struct nftnl_obj *e, struct nlattr *attr,
				      struct nftnl_obj_tunnel *tun)
{
	struct nlattr *tb[NFTA_TUNNEL_KEY_IP6_MAX + 1] = {};

	if (mnl_attr_parse_nested(attr, nftnl_obj_tunnel_ip6_cb, tb) < 0)
		return -1;

	if (tb[NFTA_TUNNEL_KEY_IP6_SRC]) {
		memcpy(&tun->src_v6,
		       mnl_attr_get_payload(tb[NFTA_TUNNEL_KEY_IP6_SRC]),
		       sizeof(struct in6_addr));
		e->flags |= (1 << NFTNL_OBJ_TUNNEL_IPV6_SRC);
	}
	if (tb[NFTA_TUNNEL_KEY_IP6_DST]) {
		memcpy(&tun->dst_v6,
		       mnl_attr_get_payload(tb[NFTA_TUNNEL_KEY_IP6_DST]),
		       sizeof(struct in6_addr));
		e->flags |= (1 << NFTNL_OBJ_TUNNEL_IPV6_DST);
	}
	if (tb[NFTA_TUNNEL_KEY_IP6_FLOWLABEL]) {
		tun->flowlabel =
			ntohl(mnl_attr_get_u32(tb[NFTA_TUNNEL_KEY_IP6_FLOWLABEL]));
		e->flags |= (1 << NFTNL_OBJ_TUNNEL_IPV6_FLOWLABEL);
	}

	return 0;
}

struct nftnl_tunnel_opt {
	struct list_head			list;

	enum nftnl_tunnel_type			type;
	uint32_t				flags;

	union {
		struct {
			uint32_t		gbp;
		} vxlan;
		struct {
			uint32_t		version;
			struct {
				uint32_t	index;
			} v1;
			struct {
				uint8_t		hwid;
				uint8_t		dir;
			} v2;
		} erspan;
		struct {
			uint16_t	geneve_class;
			uint8_t		type;
			uint8_t		data[NFTNL_TUNNEL_GENEVE_DATA_MAXLEN];
			uint32_t	data_len;
		} geneve;
	};
};

struct nftnl_tunnel_opts {
	enum nftnl_tunnel_type			type;
	uint32_t				num;
	struct list_head			opts_list;
};

EXPORT_SYMBOL(nftnl_tunnel_opt_get_data);
const void *nftnl_tunnel_opt_get_data(const struct nftnl_tunnel_opt *ne,
				      uint16_t attr, uint32_t *data_len)
{
	if (!(ne->flags & (1 << attr)))
		return NULL;

	switch (ne->type) {
	case NFTNL_TUNNEL_TYPE_ERSPAN:
		switch (attr) {
		case NFTNL_TUNNEL_ERSPAN_VERSION:
			*data_len = sizeof(uint32_t);
			return &ne->erspan.version;
		case NFTNL_TUNNEL_ERSPAN_V1_INDEX:
			*data_len = sizeof(uint32_t);
			return &ne->erspan.v1.index;
		case NFTNL_TUNNEL_ERSPAN_V2_HWID:
			*data_len = sizeof(uint8_t);
			return &ne->erspan.v2.hwid;
		case NFTNL_TUNNEL_ERSPAN_V2_DIR:
			*data_len = sizeof(uint8_t);
			return &ne->erspan.v2.dir;
		}
		break;
	case NFTNL_TUNNEL_TYPE_VXLAN:
		switch (attr) {
		case NFTNL_TUNNEL_VXLAN_GBP:
			*data_len = sizeof(uint32_t);
			return &ne->vxlan.gbp;
		}
		break;
	case NFTNL_TUNNEL_TYPE_GENEVE:
		switch (attr) {
		case NFTNL_TUNNEL_GENEVE_CLASS:
			*data_len = sizeof(uint16_t);
			return &ne->geneve.geneve_class;
		case NFTNL_TUNNEL_GENEVE_TYPE:
			*data_len = sizeof(uint8_t);
			return &ne->geneve.type;
		case NFTNL_TUNNEL_GENEVE_DATA:
			*data_len = ne->geneve.data_len;
			return &ne->geneve.data;
		}
		break;
	}

	errno = EOPNOTSUPP;
	return NULL;
}

EXPORT_SYMBOL(nftnl_tunnel_opt_get);
const void *
nftnl_tunnel_opt_get(const struct nftnl_tunnel_opt *ne, uint16_t attr)
{
	uint32_t data_len;
	return nftnl_tunnel_opt_get_data(ne, attr, &data_len);
}

EXPORT_SYMBOL(nftnl_tunnel_opt_get_u8);
uint8_t nftnl_tunnel_opt_get_u8(const struct nftnl_tunnel_opt *ne, uint16_t attr)
{
	const void *ret = nftnl_tunnel_opt_get(ne, attr);
	return ret == NULL ? 0 : *((uint8_t *)ret);
}

EXPORT_SYMBOL(nftnl_tunnel_opt_get_u16);
uint16_t nftnl_tunnel_opt_get_u16(const struct nftnl_tunnel_opt *ne, uint16_t attr)
{
	const void *ret = nftnl_tunnel_opt_get(ne, attr);
	return ret == NULL ? 0 : *((uint16_t *)ret);
}

EXPORT_SYMBOL(nftnl_tunnel_opt_get_u32);
uint32_t nftnl_tunnel_opt_get_u32(const struct nftnl_tunnel_opt *ne, uint16_t attr)
{
	const void *ret = nftnl_tunnel_opt_get(ne, attr);
	return ret == NULL ? 0 : *((uint32_t *)ret);
}

EXPORT_SYMBOL(nftnl_tunnel_opt_get_type);
enum nftnl_tunnel_type nftnl_tunnel_opt_get_type(const struct nftnl_tunnel_opt *ne)
{
	return ne->type;
}

EXPORT_SYMBOL(nftnl_tunnel_opt_get_flags);
uint32_t nftnl_tunnel_opt_get_flags(const struct nftnl_tunnel_opt *ne)
{
	return ne->flags;
}

EXPORT_SYMBOL(nftnl_obj_tunnel_opts_foreach);
int nftnl_obj_tunnel_opts_foreach(const struct nftnl_obj *ne,
                              int (*cb)(struct nftnl_tunnel_opt *opt, void *data),
                              void *data)
{
	struct nftnl_obj_tunnel *tun = nftnl_obj_data(ne);
        struct nftnl_tunnel_opt *cur, *tmp;
        int ret;

        list_for_each_entry_safe(cur, tmp, &tun->tun_opts->opts_list, list) {
                ret = cb(cur, data);
                if (ret < 0)
                        return ret;
        }

        return 0;
}

static int nftnl_obj_tunnel_vxlan_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_TUNNEL_KEY_VXLAN_MAX) < 0)
		return MNL_CB_OK;

	switch (type) {
	case NFTA_TUNNEL_KEY_VXLAN_GBP:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

struct nftnl_tunnel_opt *nftnl_tunnel_opt_alloc(enum nftnl_tunnel_type type);

static int
nftnl_obj_tunnel_parse_vxlan(struct nftnl_tunnel_opts *opts, struct nlattr *attr)
{
	struct nlattr *tb[NFTA_TUNNEL_KEY_VXLAN_MAX + 1] = {};
	struct nftnl_tunnel_opt *opt;

	if (mnl_attr_parse_nested(attr, nftnl_obj_tunnel_vxlan_cb, tb) < 0)
		return -1;

	opt = nftnl_tunnel_opt_alloc(NFTNL_TUNNEL_TYPE_VXLAN);
	if (!opt)
		return -1;

	if (tb[NFTA_TUNNEL_KEY_VXLAN_GBP]) {
		opt->vxlan.gbp =
			ntohl(mnl_attr_get_u32(tb[NFTA_TUNNEL_KEY_VXLAN_GBP]));
		opt->flags |= (1 << NFTNL_TUNNEL_VXLAN_GBP);
	}

	list_add_tail(&opt->list, &opts->opts_list);

	return 0;
}

static int nftnl_obj_tunnel_erspan_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_TUNNEL_KEY_ERSPAN_MAX) < 0)
		return MNL_CB_OK;

	switch (type) {
	case NFTA_TUNNEL_KEY_ERSPAN_VERSION:
	case NFTA_TUNNEL_KEY_ERSPAN_V1_INDEX:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	case NFTA_TUNNEL_KEY_ERSPAN_V2_HWID:
	case NFTA_TUNNEL_KEY_ERSPAN_V2_DIR:
		if (mnl_attr_validate(attr, MNL_TYPE_U8) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static int
nftnl_obj_tunnel_parse_erspan(struct nftnl_tunnel_opts *opts, struct nlattr *attr)
{
	struct nlattr *tb[NFTA_TUNNEL_KEY_ERSPAN_MAX + 1] = {};
	struct nftnl_tunnel_opt *opt;

	if (mnl_attr_parse_nested(attr, nftnl_obj_tunnel_erspan_cb, tb) < 0)
		return -1;

	opt = nftnl_tunnel_opt_alloc(NFTNL_TUNNEL_TYPE_ERSPAN);
	if (!opt)
		return -1;

	if (tb[NFTA_TUNNEL_KEY_ERSPAN_VERSION]) {
		opt->erspan.version =
			ntohl(mnl_attr_get_u32(tb[NFTA_TUNNEL_KEY_ERSPAN_VERSION]));
		opt->flags |= (1 << NFTNL_TUNNEL_ERSPAN_VERSION);
	}
	if (tb[NFTA_TUNNEL_KEY_ERSPAN_V1_INDEX]) {
		opt->erspan.v1.index =
			ntohl(mnl_attr_get_u32(tb[NFTA_TUNNEL_KEY_ERSPAN_V1_INDEX]));
		opt->flags |= (1 << NFTNL_TUNNEL_ERSPAN_V1_INDEX);
	}
	if (tb[NFTA_TUNNEL_KEY_ERSPAN_V2_HWID]) {
		opt->erspan.v2.hwid =
			mnl_attr_get_u8(tb[NFTA_TUNNEL_KEY_ERSPAN_V2_HWID]);
		opt->flags |= (1 << NFTNL_TUNNEL_ERSPAN_V2_HWID);
	}
	if (tb[NFTA_TUNNEL_KEY_ERSPAN_V2_DIR]) {
		opt->erspan.v2.dir =
			mnl_attr_get_u8(tb[NFTA_TUNNEL_KEY_ERSPAN_V2_DIR]);
		opt->flags |= (1 << NFTNL_TUNNEL_ERSPAN_V2_DIR);
	}

	list_add_tail(&opt->list, &opts->opts_list);

	return 0;
}

static int nftnl_obj_tunnel_geneve_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_TUNNEL_KEY_GENEVE_MAX) < 0)
		return MNL_CB_OK;

	switch (type) {
	case NFTA_TUNNEL_KEY_GENEVE_CLASS:
		if (mnl_attr_validate(attr, MNL_TYPE_U16) < 0)
			abi_breakage();
		break;
	case NFTA_TUNNEL_KEY_GENEVE_TYPE:
		if (mnl_attr_validate(attr, MNL_TYPE_U8) < 0)
			abi_breakage();
		break;
	case NFTA_TUNNEL_KEY_GENEVE_DATA:
		if (mnl_attr_validate(attr, MNL_TYPE_BINARY) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static int
nftnl_obj_tunnel_parse_geneve(struct nftnl_tunnel_opts *opts, struct nlattr *attr)
{
	struct nlattr *tb[NFTA_TUNNEL_KEY_GENEVE_MAX + 1] = {};
	struct nftnl_tunnel_opt *opt;

	if (mnl_attr_parse_nested(attr, nftnl_obj_tunnel_geneve_cb, tb) < 0)
		return -1;

	opt = nftnl_tunnel_opt_alloc(NFTNL_TUNNEL_TYPE_GENEVE);
	if (!opt)
		return -1;

	if (tb[NFTA_TUNNEL_KEY_GENEVE_CLASS]) {
		opt->geneve.geneve_class =
			ntohs(mnl_attr_get_u16(tb[NFTA_TUNNEL_KEY_GENEVE_CLASS]));
		opt->flags |= (1 << NFTNL_TUNNEL_GENEVE_CLASS);
	}

	if (tb[NFTA_TUNNEL_KEY_GENEVE_TYPE]) {
		opt->geneve.type =
			mnl_attr_get_u8(tb[NFTA_TUNNEL_KEY_GENEVE_TYPE]);
		opt->flags |= (1 << NFTNL_TUNNEL_GENEVE_TYPE);
	}

	if (tb[NFTA_TUNNEL_KEY_GENEVE_DATA]) {
		uint32_t len = mnl_attr_get_payload_len(tb[NFTA_TUNNEL_KEY_GENEVE_DATA]);

		memcpy(opt->geneve.data,
		       mnl_attr_get_payload(tb[NFTA_TUNNEL_KEY_GENEVE_DATA]),
		       len);
		opt->geneve.data_len = len;
		opt->flags |= (1 << NFTNL_TUNNEL_GENEVE_DATA);
	}

	list_add_tail(&opt->list, &opts->opts_list);

	return 0;
}

struct nftnl_tunnel_opts *nftnl_tunnel_opts_alloc(enum nftnl_tunnel_type type);

static int
nftnl_obj_tunnel_parse_opts(struct nftnl_obj *e, struct nlattr *nest,
			    struct nftnl_obj_tunnel *tun)
{
	struct nlattr *attr;
	struct nftnl_tunnel_opts *opts = NULL;
	int err = 0;

	mnl_attr_for_each_nested(attr, nest) {
		if (mnl_attr_validate(attr, MNL_TYPE_NESTED) < 0)
			abi_breakage();

		switch(mnl_attr_get_type(attr)) {
			case NFTA_TUNNEL_KEY_OPTS_VXLAN:
				opts = nftnl_tunnel_opts_alloc(NFTNL_TUNNEL_TYPE_VXLAN);
				if (!opts)
					return -1;

				err = nftnl_obj_tunnel_parse_vxlan(opts, attr);
			break;
			case NFTA_TUNNEL_KEY_OPTS_ERSPAN:
				opts = nftnl_tunnel_opts_alloc(NFTNL_TUNNEL_TYPE_ERSPAN);
				if (!opts)
					return -1;

				err = nftnl_obj_tunnel_parse_erspan(opts, attr);
			break;
			case NFTA_TUNNEL_KEY_OPTS_GENEVE:
				if (!opts)
					opts = nftnl_tunnel_opts_alloc(NFTNL_TUNNEL_TYPE_GENEVE);

				if (!opts)
					return -1;

				err = nftnl_obj_tunnel_parse_geneve(opts, attr);
			break;
		}
	}

	if (opts) {
		tun->tun_opts = opts;
		e->flags |= (1 << NFTNL_OBJ_TUNNEL_OPTS);
	}

	return err;
}

static int
nftnl_obj_tunnel_parse(struct nftnl_obj *e, struct nlattr *attr)
{
	struct nftnl_obj_tunnel *tun = nftnl_obj_data(e);
	struct nlattr *tb[NFTA_TUNNEL_KEY_MAX + 1] = {};
	int err;

	if (mnl_attr_parse_nested(attr, nftnl_obj_tunnel_cb, tb) < 0)
		return -1;

	if (tb[NFTA_TUNNEL_KEY_ID]) {
		tun->id = ntohl(mnl_attr_get_u32(tb[NFTA_TUNNEL_KEY_ID]));
		e->flags |= (1 << NFTNL_OBJ_TUNNEL_ID);
	}
	if (tb[NFTA_TUNNEL_KEY_IP]) {
		err = nftnl_obj_tunnel_parse_ip(e, tb[NFTA_TUNNEL_KEY_IP], tun);
		if (err < 0)
			return err;
	} else if (tb[NFTA_TUNNEL_KEY_IP6]) {
		err = nftnl_obj_tunnel_parse_ip6(e, tb[NFTA_TUNNEL_KEY_IP6], tun);
		if (err < 0)
			return err;
	}

	if (tb[NFTA_TUNNEL_KEY_SPORT]) {
		tun->sport = ntohs(mnl_attr_get_u16(tb[NFTA_TUNNEL_KEY_SPORT]));
		e->flags |= (1 << NFTNL_OBJ_TUNNEL_SPORT);
	}
	if (tb[NFTA_TUNNEL_KEY_DPORT]) {
		tun->dport = ntohs(mnl_attr_get_u16(tb[NFTA_TUNNEL_KEY_DPORT]));
		e->flags |= (1 << NFTNL_OBJ_TUNNEL_DPORT);
	}
	if (tb[NFTA_TUNNEL_KEY_TOS]) {
		tun->tun_tos = mnl_attr_get_u8(tb[NFTA_TUNNEL_KEY_TOS]);
		e->flags |= (1 << NFTNL_OBJ_TUNNEL_TOS);
	}
	if (tb[NFTA_TUNNEL_KEY_TTL]) {
		tun->tun_ttl = mnl_attr_get_u8(tb[NFTA_TUNNEL_KEY_TTL]);
		e->flags |= (1 << NFTNL_OBJ_TUNNEL_TTL);
	}
	if (tb[NFTA_TUNNEL_KEY_FLAGS]) {
		tun->tun_flags = mnl_attr_get_u8(tb[NFTA_TUNNEL_KEY_FLAGS]);
		e->flags |= (1 << NFTNL_OBJ_TUNNEL_FLAGS);
	}
	if (tb[NFTA_TUNNEL_KEY_OPTS]) {
		err = nftnl_obj_tunnel_parse_opts(e, tb[NFTA_TUNNEL_KEY_OPTS], tun);
		if (err < 0)
			return err;
	}

	return 0;
}

static int nftnl_obj_tunnel_snprintf(char *buf, size_t len,
				     uint32_t flags, const struct nftnl_obj *e)
{
	struct nftnl_obj_tunnel *tun = nftnl_obj_data(e);

	return snprintf(buf, len, "id %u ", tun->id);
}

EXPORT_SYMBOL(nftnl_tunnel_opts_alloc);
struct nftnl_tunnel_opts *nftnl_tunnel_opts_alloc(enum nftnl_tunnel_type type)
{
	struct nftnl_tunnel_opts *opts;

	switch (type) {
	case NFTNL_TUNNEL_TYPE_VXLAN:
	case NFTNL_TUNNEL_TYPE_ERSPAN:
	case NFTNL_TUNNEL_TYPE_GENEVE:
		break;
	default:
		errno = EOPNOTSUPP;
		return NULL;
	}

	opts = calloc(1, sizeof(struct nftnl_tunnel_opts));
	if (!opts)
		return NULL;

	opts->type = type;
	INIT_LIST_HEAD(&opts->opts_list);

	return opts;
}

EXPORT_SYMBOL(nftnl_tunnel_opts_add);
int nftnl_tunnel_opts_add(struct nftnl_tunnel_opts *opts,
			  struct nftnl_tunnel_opt *opt)
{
	if (opt->type != opts->type) {
		errno = EOPNOTSUPP;
		return -1;
	}

	switch (opts->type) {
	case NFTNL_TUNNEL_TYPE_VXLAN:
	case NFTNL_TUNNEL_TYPE_ERSPAN:
		if (opts->num > 0) {
			errno = EEXIST;
			return -1;
		}
		break;
	case NFTNL_TUNNEL_TYPE_GENEVE:
		break;
	default:
		errno = EOPNOTSUPP;
		return -1;
	}

	list_add_tail(&opt->list, &opts->opts_list);

	return 0;
}

EXPORT_SYMBOL(nftnl_tunnel_opts_free);
void nftnl_tunnel_opts_free(struct nftnl_tunnel_opts *opts)
{
	struct nftnl_tunnel_opt *opt, *next;

	list_for_each_entry_safe(opt, next, &opts->opts_list, list) {
		switch(opts->type) {
		case NFTNL_TUNNEL_TYPE_VXLAN:
		case NFTNL_TUNNEL_TYPE_ERSPAN:
		case NFTNL_TUNNEL_TYPE_GENEVE:
			list_del(&opt->list);
			xfree(opt);
			break;
		}
	}
}

EXPORT_SYMBOL(nftnl_tunnel_opt_alloc);
struct nftnl_tunnel_opt *nftnl_tunnel_opt_alloc(enum nftnl_tunnel_type type)
{
	struct nftnl_tunnel_opt *opt;

	switch (type) {
	case NFTNL_TUNNEL_TYPE_VXLAN:
	case NFTNL_TUNNEL_TYPE_ERSPAN:
	case NFTNL_TUNNEL_TYPE_GENEVE:
		break;
	default:
		errno = EOPNOTSUPP;
		return NULL;
	}

	opt = calloc(1, sizeof(struct nftnl_tunnel_opt));
	if (!opt)
		return NULL;

	opt->type = type;

	return opt;
}

static int nftnl_tunnel_opt_vxlan_set(struct nftnl_tunnel_opt *opt, uint16_t type,
				      const void *data, uint32_t data_len)
{
	switch (type) {
	case NFTNL_TUNNEL_VXLAN_GBP:
		memcpy(&opt->vxlan.gbp, data, data_len);
		break;
	default:
		errno = EOPNOTSUPP;
		return -1;
	}

	opt->flags |= (1 << type);

	return 0;
}

static int nftnl_tunnel_opt_erspan_set(struct nftnl_tunnel_opt *opt, uint16_t type,
				       const void *data, uint32_t data_len)
{
	switch (type) {
	case NFTNL_TUNNEL_ERSPAN_VERSION:
		memcpy(&opt->erspan.version, data, data_len);
		break;
	case NFTNL_TUNNEL_ERSPAN_V1_INDEX:
		memcpy(&opt->erspan.v1.index, data, data_len);
		break;
	case NFTNL_TUNNEL_ERSPAN_V2_HWID:
		memcpy(&opt->erspan.v2.hwid, data, data_len);
		break;
	case NFTNL_TUNNEL_ERSPAN_V2_DIR:
		memcpy(&opt->erspan.v2.dir, data, data_len);
		break;
	default:
		errno = EOPNOTSUPP;
		return -1;
	}

	opt->flags |= (1 << type);

	return 0;
}

static int nftnl_tunnel_opt_geneve_set(struct nftnl_tunnel_opt *opt, uint16_t type,
				       const void *data, uint32_t data_len)
{
	switch(type) {
	case NFTNL_TUNNEL_GENEVE_CLASS:
		memcpy(&opt->geneve.geneve_class, data, data_len);
		break;
	case NFTNL_TUNNEL_GENEVE_TYPE:
		memcpy(&opt->geneve.type, data, data_len);
		break;
	case NFTNL_TUNNEL_GENEVE_DATA:
		if (data_len > NFTNL_TUNNEL_GENEVE_DATA_MAXLEN) {
			errno = EINVAL;
			return -1;
		}
		memcpy(&opt->geneve.data, data, data_len);
		opt->geneve.data_len = data_len;
		break;
	default:
		errno = EOPNOTSUPP;
		return -1;
	}

	opt->flags |= (1 << type);

	return 0;
}

EXPORT_SYMBOL(nftnl_tunnel_opt_set);
int nftnl_tunnel_opt_set(struct nftnl_tunnel_opt *opt, uint16_t type,
			 const void *data, uint32_t data_len)
{
	switch (opt->type) {
	case NFTNL_TUNNEL_TYPE_VXLAN:
		return nftnl_tunnel_opt_vxlan_set(opt, type, data, data_len);
	case NFTNL_TUNNEL_TYPE_ERSPAN:
		return nftnl_tunnel_opt_erspan_set(opt, type, data, data_len);
	case NFTNL_TUNNEL_TYPE_GENEVE:
		return nftnl_tunnel_opt_geneve_set(opt, type, data, data_len);
	default:
		errno = EOPNOTSUPP;
		return -1;
	}

	return 0;
}

static void nftnl_tunnel_opt_build_vxlan(struct nlmsghdr *nlh,
					 const struct nftnl_tunnel_opt *opt)
{
	struct nlattr *nest_inner;

	if (opt->flags & (1 << NFTNL_TUNNEL_VXLAN_GBP)) {
		nest_inner = mnl_attr_nest_start(nlh, NFTA_TUNNEL_KEY_OPTS_VXLAN);
		mnl_attr_put_u32(nlh, NFTA_TUNNEL_KEY_VXLAN_GBP,
				 htonl(opt->vxlan.gbp));
		mnl_attr_nest_end(nlh, nest_inner);
	}
}

static void nftnl_tunnel_opt_build_erspan(struct nlmsghdr *nlh,
					  const struct nftnl_tunnel_opt *opt)
{
	struct nlattr *nest_inner;

	if (opt->flags & (1 << NFTNL_TUNNEL_ERSPAN_VERSION) &&
	    (opt->flags & (1 << NFTNL_TUNNEL_ERSPAN_V1_INDEX) ||
	     (opt->flags & (1 << NFTNL_TUNNEL_ERSPAN_V2_HWID) &&
	      opt->flags & (1 << NFTNL_TUNNEL_ERSPAN_V2_DIR)))) {
		nest_inner = mnl_attr_nest_start(nlh, NFTA_TUNNEL_KEY_OPTS_ERSPAN);
		mnl_attr_put_u32(nlh, NFTA_TUNNEL_KEY_ERSPAN_VERSION,
				 htonl(opt->erspan.version));
		if (opt->flags & (1 << NFTNL_TUNNEL_ERSPAN_V1_INDEX))
			mnl_attr_put_u32(nlh, NFTA_TUNNEL_KEY_ERSPAN_V1_INDEX,
					 htonl(opt->erspan.v1.index));
		if (opt->flags & (1 << NFTNL_TUNNEL_ERSPAN_V2_HWID))
			mnl_attr_put_u8(nlh, NFTA_TUNNEL_KEY_ERSPAN_V2_HWID,
					opt->erspan.v2.hwid);
		if (opt->flags & (1 << NFTNL_TUNNEL_ERSPAN_V2_DIR))
			mnl_attr_put_u8(nlh, NFTA_TUNNEL_KEY_ERSPAN_V2_DIR,
					opt->erspan.v2.dir);
		mnl_attr_nest_end(nlh, nest_inner);
	}
}

static void nftnl_tunnel_opt_build_geneve(struct nlmsghdr *nlh,
					  const struct nftnl_tunnel_opt *opt)
{
	struct nlattr *nest_inner;

	if (opt->flags & (1 << NFTNL_TUNNEL_GENEVE_CLASS) &&
	    opt->flags & (1 << NFTNL_TUNNEL_GENEVE_TYPE) &&
	    opt->flags & (1 << NFTNL_TUNNEL_GENEVE_DATA)) {
		nest_inner = mnl_attr_nest_start(nlh, NFTA_TUNNEL_KEY_OPTS_GENEVE);
		mnl_attr_put_u16(nlh, NFTA_TUNNEL_KEY_GENEVE_CLASS,
				 htons(opt->geneve.geneve_class));
		mnl_attr_put_u8(nlh, NFTA_TUNNEL_KEY_GENEVE_TYPE,
				opt->geneve.type);
		mnl_attr_put(nlh, NFTA_TUNNEL_KEY_GENEVE_DATA,
			     opt->geneve.data_len,
			     opt->geneve.data);
		mnl_attr_nest_end(nlh, nest_inner);
	}
}

void nftnl_tunnel_opts_build(struct nlmsghdr *nlh,
			     struct nftnl_tunnel_opts *opts)
{
	const struct nftnl_tunnel_opt *opt;
	struct nlattr *nest;

	nest = mnl_attr_nest_start(nlh, NFTA_TUNNEL_KEY_OPTS);

	list_for_each_entry(opt, &opts->opts_list, list) {
		switch (opts->type) {
		case NFTNL_TUNNEL_TYPE_VXLAN:
			nftnl_tunnel_opt_build_vxlan(nlh, opt);
			break;
		case NFTNL_TUNNEL_TYPE_ERSPAN:
			nftnl_tunnel_opt_build_erspan(nlh, opt);
			break;
		case NFTNL_TUNNEL_TYPE_GENEVE:
			nftnl_tunnel_opt_build_geneve(nlh, opt);
			break;
		}
	}
	mnl_attr_nest_end(nlh, nest);
}

static struct attr_policy obj_tunnel_attr_policy[__NFTNL_OBJ_TUNNEL_MAX] = {
	[NFTNL_OBJ_TUNNEL_ID]		= { .maxlen = sizeof(uint32_t) },
	[NFTNL_OBJ_TUNNEL_IPV4_SRC]	= { .maxlen = sizeof(uint32_t) },
	[NFTNL_OBJ_TUNNEL_IPV4_DST]	= { .maxlen = sizeof(uint32_t) },
	[NFTNL_OBJ_TUNNEL_IPV6_SRC]	= { .maxlen = sizeof(struct in6_addr) },
	[NFTNL_OBJ_TUNNEL_IPV6_DST]	= { .maxlen = sizeof(struct in6_addr) },
	[NFTNL_OBJ_TUNNEL_IPV6_FLOWLABEL] = { .maxlen = sizeof(uint32_t) },
	[NFTNL_OBJ_TUNNEL_SPORT]	= { .maxlen = sizeof(uint16_t) },
	[NFTNL_OBJ_TUNNEL_DPORT]	= { .maxlen = sizeof(uint16_t) },
	[NFTNL_OBJ_TUNNEL_FLAGS]	= { .maxlen = sizeof(uint32_t) },
	[NFTNL_OBJ_TUNNEL_TOS]		= { .maxlen = sizeof(uint8_t) },
	[NFTNL_OBJ_TUNNEL_TTL]		= { .maxlen = sizeof(uint8_t) },
	[NFTNL_OBJ_TUNNEL_OPTS]		= { .maxlen = sizeof(struct nftnl_tunnel_opts *) },
};

struct obj_ops obj_ops_tunnel = {
	.name		= "tunnel",
	.type		= NFT_OBJECT_TUNNEL,
	.alloc_len	= sizeof(struct nftnl_obj_tunnel),
	.nftnl_max_attr	= __NFTNL_OBJ_TUNNEL_MAX - 1,
	.attr_policy	= obj_tunnel_attr_policy,
	.set		= nftnl_obj_tunnel_set,
	.get		= nftnl_obj_tunnel_get,
	.parse		= nftnl_obj_tunnel_parse,
	.build		= nftnl_obj_tunnel_build,
	.output		= nftnl_obj_tunnel_snprintf,
};
