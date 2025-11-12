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
	case NFTNL_OBJ_TUNNEL_VXLAN_GBP:
		memcpy(&tun->u.tun_vxlan.gbp, data, data_len);
		break;
	case NFTNL_OBJ_TUNNEL_ERSPAN_VERSION:
		memcpy(&tun->u.tun_erspan.version, data, data_len);
		break;
	case NFTNL_OBJ_TUNNEL_ERSPAN_V1_INDEX:
		memcpy(&tun->u.tun_erspan.u.v1_index, data, data_len);
		break;
	case NFTNL_OBJ_TUNNEL_ERSPAN_V2_HWID:
		memcpy(&tun->u.tun_erspan.u.v2.hwid, data, data_len);
		break;
	case NFTNL_OBJ_TUNNEL_ERSPAN_V2_DIR:
		memcpy(&tun->u.tun_erspan.u.v2.dir, data, data_len);
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
	case NFTNL_OBJ_TUNNEL_VXLAN_GBP:
		*data_len = sizeof(tun->u.tun_vxlan.gbp);
		return &tun->u.tun_vxlan.gbp;
	case NFTNL_OBJ_TUNNEL_ERSPAN_VERSION:
		*data_len = sizeof(tun->u.tun_erspan.version);
		return &tun->u.tun_erspan.version;
	case NFTNL_OBJ_TUNNEL_ERSPAN_V1_INDEX:
		*data_len = sizeof(tun->u.tun_erspan.u.v1_index);
		return &tun->u.tun_erspan.u.v1_index;
	case NFTNL_OBJ_TUNNEL_ERSPAN_V2_HWID:
		*data_len = sizeof(tun->u.tun_erspan.u.v2.hwid);
		return &tun->u.tun_erspan.u.v2.hwid;
	case NFTNL_OBJ_TUNNEL_ERSPAN_V2_DIR:
		*data_len = sizeof(tun->u.tun_erspan.u.v2.dir);
		return &tun->u.tun_erspan.u.v2.dir;
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

static void
nftnl_obj_tunnel_build(struct nlmsghdr *nlh, const struct nftnl_obj *e)
{
	struct nftnl_obj_tunnel *tun = nftnl_obj_data(e);
	struct nlattr *nest, *nest_inner;

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
	if (e->flags & (1 << NFTNL_OBJ_TUNNEL_VXLAN_GBP)) {
		nest = mnl_attr_nest_start(nlh, NFTA_TUNNEL_KEY_OPTS);
		nest_inner = mnl_attr_nest_start(nlh, NFTA_TUNNEL_KEY_OPTS_VXLAN);
		mnl_attr_put_u32(nlh, NFTA_TUNNEL_KEY_VXLAN_GBP,
				 htonl(tun->u.tun_vxlan.gbp));
		mnl_attr_nest_end(nlh, nest_inner);
		mnl_attr_nest_end(nlh, nest);
	}
	if (e->flags & (1 << NFTNL_OBJ_TUNNEL_ERSPAN_VERSION) &&
	    (e->flags & (1 << NFTNL_OBJ_TUNNEL_ERSPAN_V1_INDEX) ||
	     (e->flags & (1 << NFTNL_OBJ_TUNNEL_ERSPAN_V2_HWID) &&
	      e->flags & (1u << NFTNL_OBJ_TUNNEL_ERSPAN_V2_DIR)))) {
		nest = mnl_attr_nest_start(nlh, NFTA_TUNNEL_KEY_OPTS);
		nest_inner = mnl_attr_nest_start(nlh, NFTA_TUNNEL_KEY_OPTS_ERSPAN);
		mnl_attr_put_u32(nlh, NFTA_TUNNEL_KEY_ERSPAN_VERSION,
				 htonl(tun->u.tun_erspan.version));
		if (e->flags & (1 << NFTNL_OBJ_TUNNEL_ERSPAN_V1_INDEX))
			mnl_attr_put_u32(nlh, NFTA_TUNNEL_KEY_ERSPAN_V1_INDEX,
					 htonl(tun->u.tun_erspan.u.v1_index));
		if (e->flags & (1 << NFTNL_OBJ_TUNNEL_ERSPAN_V2_HWID))
			mnl_attr_put_u8(nlh, NFTA_TUNNEL_KEY_ERSPAN_V2_HWID,
					tun->u.tun_erspan.u.v2.hwid);
		if (e->flags & (1u << NFTNL_OBJ_TUNNEL_ERSPAN_V2_DIR))
			mnl_attr_put_u8(nlh, NFTA_TUNNEL_KEY_ERSPAN_V2_DIR,
					tun->u.tun_erspan.u.v2.dir);
		mnl_attr_nest_end(nlh, nest_inner);
		mnl_attr_nest_end(nlh, nest);
	}
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

static int
nftnl_obj_tunnel_parse_vxlan(struct nftnl_obj *e, struct nlattr *attr,
			     struct nftnl_obj_tunnel *tun)
{
	struct nlattr *tb[NFTA_TUNNEL_KEY_VXLAN_MAX + 1] = {};

	if (mnl_attr_parse_nested(attr, nftnl_obj_tunnel_vxlan_cb, tb) < 0)
		return -1;

	if (tb[NFTA_TUNNEL_KEY_VXLAN_GBP]) {
		tun->u.tun_vxlan.gbp =
			ntohl(mnl_attr_get_u32(tb[NFTA_TUNNEL_KEY_VXLAN_GBP]));
		e->flags |= (1 << NFTNL_OBJ_TUNNEL_VXLAN_GBP);
	}

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
nftnl_obj_tunnel_parse_erspan(struct nftnl_obj *e, struct nlattr *attr,
			      struct nftnl_obj_tunnel *tun)
{
	struct nlattr *tb[NFTA_TUNNEL_KEY_ERSPAN_MAX + 1] = {};

	if (mnl_attr_parse_nested(attr, nftnl_obj_tunnel_erspan_cb, tb) < 0)
		return -1;

	if (tb[NFTA_TUNNEL_KEY_ERSPAN_VERSION]) {
		tun->u.tun_erspan.version =
			ntohl(mnl_attr_get_u32(tb[NFTA_TUNNEL_KEY_ERSPAN_VERSION]));
		e->flags |= (1 << NFTNL_OBJ_TUNNEL_ERSPAN_VERSION);
	}
	if (tb[NFTA_TUNNEL_KEY_ERSPAN_V1_INDEX]) {
		tun->u.tun_erspan.u.v1_index =
			ntohl(mnl_attr_get_u32(tb[NFTA_TUNNEL_KEY_ERSPAN_V1_INDEX]));
		e->flags |= (1 << NFTNL_OBJ_TUNNEL_ERSPAN_V1_INDEX);
	}
	if (tb[NFTA_TUNNEL_KEY_ERSPAN_V2_HWID]) {
		tun->u.tun_erspan.u.v2.hwid =
			mnl_attr_get_u8(tb[NFTA_TUNNEL_KEY_ERSPAN_V2_HWID]);
		e->flags |= (1 << NFTNL_OBJ_TUNNEL_ERSPAN_V2_HWID);
	}
	if (tb[NFTA_TUNNEL_KEY_ERSPAN_V2_DIR]) {
		tun->u.tun_erspan.u.v2.dir =
			mnl_attr_get_u8(tb[NFTA_TUNNEL_KEY_ERSPAN_V2_DIR]);
		e->flags |= (1u << NFTNL_OBJ_TUNNEL_ERSPAN_V2_DIR);
	}

	return 0;
}

static int nftnl_obj_tunnel_opts_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_TUNNEL_KEY_OPTS_MAX) < 0)
		return MNL_CB_OK;

	switch (type) {
	case NFTA_TUNNEL_KEY_OPTS_VXLAN:
	case NFTA_TUNNEL_KEY_OPTS_ERSPAN:
		if (mnl_attr_validate(attr, MNL_TYPE_NESTED) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static int
nftnl_obj_tunnel_parse_opts(struct nftnl_obj *e, struct nlattr *attr,
			    struct nftnl_obj_tunnel *tun)
{
	struct nlattr *tb[NFTA_TUNNEL_KEY_OPTS_MAX + 1] = {};
	int err = 0;

	if (mnl_attr_parse_nested(attr, nftnl_obj_tunnel_opts_cb, tb) < 0)
		return -1;

	if (tb[NFTA_TUNNEL_KEY_OPTS_VXLAN]) {
		err = nftnl_obj_tunnel_parse_vxlan(e, tb[NFTA_TUNNEL_KEY_OPTS_VXLAN],
						   tun);
	} else if (tb[NFTA_TUNNEL_KEY_OPTS_ERSPAN]) {
		err = nftnl_obj_tunnel_parse_erspan(e, tb[NFTA_TUNNEL_KEY_OPTS_ERSPAN],
						    tun);
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
	[NFTNL_OBJ_TUNNEL_VXLAN_GBP]	= { .maxlen = sizeof(uint32_t) },
	[NFTNL_OBJ_TUNNEL_ERSPAN_VERSION] = { .maxlen = sizeof(uint32_t) },
	[NFTNL_OBJ_TUNNEL_ERSPAN_V1_INDEX] = { .maxlen = sizeof(uint32_t) },
	[NFTNL_OBJ_TUNNEL_ERSPAN_V2_HWID] = { .maxlen = sizeof(uint8_t) },
	[NFTNL_OBJ_TUNNEL_ERSPAN_V2_DIR] = { .maxlen = sizeof(uint8_t) },
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
