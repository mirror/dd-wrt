/*
 * Payload expression and related functions.
 *
 * Copyright (c) 2008 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Development of this code funded by Astaro AG (http://www.astaro.com/)
 */

#include <nft.h>

#include <stddef.h>
#include <stdio.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <linux/netfilter.h>
#include <linux/if_ether.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>

#include <rule.h>
#include <expression.h>
#include <statement.h>
#include <payload.h>
#include <gmputil.h>
#include <utils.h>
#include <json.h>

bool payload_is_known(const struct expr *expr)
{
	const struct proto_hdr_template *tmpl;
	const struct proto_desc *desc;

	desc = expr->payload.desc;
	tmpl = expr->payload.tmpl;

	return desc && tmpl && desc != &proto_unknown &&
	       tmpl != &proto_unknown_template;
}

static void payload_expr_print(const struct expr *expr, struct output_ctx *octx)
{
	const struct proto_desc *desc;
	const struct proto_hdr_template *tmpl;

	if (expr->payload.inner_desc &&
	    expr->payload.inner_desc != expr->payload.desc)
		nft_print(octx, "%s ", expr->payload.inner_desc->name);

	desc = expr->payload.desc;
	tmpl = expr->payload.tmpl;
	if (payload_is_known(expr))
		nft_print(octx, "%s %s", desc->name, tmpl->token);
	else
		nft_print(octx, "@%s,%u,%u",
			  proto_base_tokens[expr->payload.base],
			  expr->payload.offset, expr->len);
}

bool payload_expr_cmp(const struct expr *e1, const struct expr *e2)
{
	return e1->payload.inner_desc == e2->payload.inner_desc &&
	       e1->payload.desc   == e2->payload.desc &&
	       e1->payload.tmpl   == e2->payload.tmpl &&
	       e1->payload.base   == e2->payload.base &&
	       e1->payload.offset == e2->payload.offset;
}

static void payload_expr_clone(struct expr *new, const struct expr *expr)
{
	new->payload.inner_desc   = expr->payload.inner_desc;
	new->payload.desc   = expr->payload.desc;
	new->payload.tmpl   = expr->payload.tmpl;
	new->payload.base   = expr->payload.base;
	new->payload.offset = expr->payload.offset;
}

/**
 * payload_expr_pctx_update - update protocol context based on payload match
 *
 * @ctx:	protocol context
 * @expr:	relational payload expression
 *
 * Update protocol context for relational payload expressions.
 */
static void payload_expr_pctx_update(struct proto_ctx *ctx,
				     const struct location *loc,
				     const struct expr *left,
				     const struct expr *right)
{
	const struct proto_desc *base, *desc;
	unsigned int proto = 0;

	/* Export the data in the correct byte order */
	assert(right->len / BITS_PER_BYTE <= sizeof(proto));
	mpz_export_data(constant_data_ptr(proto, right->len), right->value,
			right->byteorder, right->len / BITS_PER_BYTE);

	base = ctx->protocol[left->payload.base].desc;
	desc = proto_find_upper(base, proto);

	if (!desc) {
		if (base == &proto_icmp) {
			/* proto 0 is ECHOREPLY, just pretend its ECHO.
			 * Not doing this would need an additional marker
			 * bit to tell when icmp.type was set.
			 */
			ctx->th_dep.icmp.type = proto ? proto : ICMP_ECHO;
		} else if (base == &proto_icmp6) {
			if (proto == ICMP6_ECHO_REPLY)
				proto = ICMP6_ECHO_REQUEST;
			ctx->th_dep.icmp.type = proto;
		}
		return;
	}

	assert(desc->base <= PROTO_BASE_MAX);
	if (desc->base == base->base) {
		if (!left->payload.is_raw) {
			if (desc->base == PROTO_BASE_LL_HDR &&
			    ctx->stacked_ll_count < PROTO_CTX_NUM_PROTOS) {
				assert(base->length > 0);
				ctx->stacked_ll[ctx->stacked_ll_count] = base;
				ctx->stacked_ll_count++;
			}
		}
	}
	proto_ctx_update(ctx, desc->base, loc, desc);
}

#define NFTNL_UDATA_SET_KEY_PAYLOAD_DESC 0
#define NFTNL_UDATA_SET_KEY_PAYLOAD_TYPE 1
#define NFTNL_UDATA_SET_KEY_PAYLOAD_BASE 2
#define NFTNL_UDATA_SET_KEY_PAYLOAD_OFFSET 3
#define NFTNL_UDATA_SET_KEY_PAYLOAD_LEN 4
#define NFTNL_UDATA_SET_KEY_PAYLOAD_INNER_DESC 5
#define NFTNL_UDATA_SET_KEY_PAYLOAD_MAX 6

static unsigned int expr_payload_type(const struct proto_desc *desc,
				      const struct proto_hdr_template *tmpl)
{
	if (desc->id == PROTO_DESC_UNKNOWN)
		return 0;

	return (unsigned int)(tmpl - &desc->templates[0]);
}

static int payload_expr_build_udata(struct nftnl_udata_buf *udbuf,
				    const struct expr *expr)
{
	const struct proto_hdr_template *tmpl = expr->payload.tmpl;
	const struct proto_desc *desc = expr->payload.desc;
	unsigned int type = expr_payload_type(desc, tmpl);

	nftnl_udata_put_u32(udbuf, NFTNL_UDATA_SET_KEY_PAYLOAD_DESC, desc->id);
	nftnl_udata_put_u32(udbuf, NFTNL_UDATA_SET_KEY_PAYLOAD_TYPE, type);

	if (desc->id == 0) {
		nftnl_udata_put_u32(udbuf, NFTNL_UDATA_SET_KEY_PAYLOAD_BASE,
				    expr->payload.base);
		nftnl_udata_put_u32(udbuf, NFTNL_UDATA_SET_KEY_PAYLOAD_OFFSET,
				    expr->payload.offset);
	}
	if (expr->dtype->type == TYPE_INTEGER)
		nftnl_udata_put_u32(udbuf, NFTNL_UDATA_SET_KEY_PAYLOAD_LEN, expr->len);

	if (expr->payload.inner_desc) {
		nftnl_udata_put_u32(udbuf, NFTNL_UDATA_SET_KEY_PAYLOAD_INNER_DESC,
				    expr->payload.inner_desc->id);
	}

	return 0;
}

const struct proto_desc *find_proto_desc(const struct nftnl_udata *ud)
{
	return proto_find_desc(nftnl_udata_get_u32(ud));
}

static int payload_parse_udata(const struct nftnl_udata *attr, void *data)
{
	const struct nftnl_udata **ud = data;
	uint8_t type = nftnl_udata_type(attr);
	uint8_t len = nftnl_udata_len(attr);

	switch (type) {
	case NFTNL_UDATA_SET_KEY_PAYLOAD_DESC:
	case NFTNL_UDATA_SET_KEY_PAYLOAD_TYPE:
	case NFTNL_UDATA_SET_KEY_PAYLOAD_BASE:
	case NFTNL_UDATA_SET_KEY_PAYLOAD_OFFSET:
	case NFTNL_UDATA_SET_KEY_PAYLOAD_LEN:
	case NFTNL_UDATA_SET_KEY_PAYLOAD_INNER_DESC:
		if (len != sizeof(uint32_t))
			return -1;
		break;
	default:
		return 0;
	}

	ud[type] = attr;
	return 0;
}

static struct expr *payload_expr_parse_udata(const struct nftnl_udata *attr)
{
	const struct nftnl_udata *ud[NFTNL_UDATA_SET_KEY_PAYLOAD_MAX + 1] = {};
	unsigned int type, base, offset, len = 0;
	const struct proto_desc *desc;
	bool is_raw = false;
	struct expr *expr;
	int err;

	err = nftnl_udata_parse(nftnl_udata_get(attr), nftnl_udata_len(attr),
				payload_parse_udata, ud);
	if (err < 0)
		return NULL;

	if (!ud[NFTNL_UDATA_SET_KEY_PAYLOAD_DESC] ||
	    !ud[NFTNL_UDATA_SET_KEY_PAYLOAD_TYPE])
		return NULL;

	desc = find_proto_desc(ud[NFTNL_UDATA_SET_KEY_PAYLOAD_DESC]);
	if (!desc) {
		if (!ud[NFTNL_UDATA_SET_KEY_PAYLOAD_BASE] ||
		    !ud[NFTNL_UDATA_SET_KEY_PAYLOAD_OFFSET])
			return NULL;

		base = nftnl_udata_get_u32(ud[NFTNL_UDATA_SET_KEY_PAYLOAD_BASE]);
		offset = nftnl_udata_get_u32(ud[NFTNL_UDATA_SET_KEY_PAYLOAD_OFFSET]);
		is_raw = true;
	}

	type = nftnl_udata_get_u32(ud[NFTNL_UDATA_SET_KEY_PAYLOAD_TYPE]);
	if (ud[NFTNL_UDATA_SET_KEY_PAYLOAD_LEN])
		len = nftnl_udata_get_u32(ud[NFTNL_UDATA_SET_KEY_PAYLOAD_LEN]);

	expr = payload_expr_alloc(&internal_location, desc, type);

	if (len)
		expr->len = len;

	if (is_raw) {
		struct datatype *dtype;

		expr->payload.base = base;
		expr->payload.offset = offset;
		expr->payload.is_raw = true;
		expr->len = len;
		dtype = datatype_clone(&xinteger_type);
		dtype->size = len;
		dtype->byteorder = BYTEORDER_BIG_ENDIAN;
		__datatype_set(expr, dtype);
	}

	if (ud[NFTNL_UDATA_SET_KEY_PAYLOAD_INNER_DESC]) {
		desc = find_proto_desc(ud[NFTNL_UDATA_SET_KEY_PAYLOAD_INNER_DESC]);
		expr->payload.inner_desc = desc;
	}

	return expr;
}

const struct expr_ops payload_expr_ops = {
	.type		= EXPR_PAYLOAD,
	.name		= "payload",
	.print		= payload_expr_print,
	.json		= payload_expr_json,
	.cmp		= payload_expr_cmp,
	.clone		= payload_expr_clone,
	.pctx_update	= payload_expr_pctx_update,
	.build_udata	= payload_expr_build_udata,
	.parse_udata	= payload_expr_parse_udata,
};

/*
 * We normally use 'meta l4proto' to fetch the last l4 header of the
 * ipv6 extension header chain so we will also match
 * tcp after a fragmentation header, for instance.
 * For consistency we also use meta l4proto for ipv4.
 *
 * If user specifically asks for nexthdr x, don't add another (useless)
 * meta dependency.
 */
static bool proto_key_is_protocol(const struct proto_desc *desc, unsigned int type)
{
	if (type == desc->protocol_key)
		return true;

	if (desc == &proto_ip6 && type == IP6HDR_NEXTHDR)
		return true;
	if (desc == &proto_ip && type == IPHDR_PROTOCOL)
		return true;

	return false;
}

struct expr *payload_expr_alloc(const struct location *loc,
				const struct proto_desc *desc,
				unsigned int type)
{
	const struct proto_hdr_template *tmpl;
	enum proto_bases base;
	struct expr *expr;
	unsigned int flags = 0;

	if (desc != NULL) {
		tmpl = &desc->templates[type];
		base = desc->base;
		if (proto_key_is_protocol(desc, type))
			flags = EXPR_F_PROTOCOL;
	} else {
		tmpl = &proto_unknown_template;
		base = PROTO_BASE_INVALID;
		desc = &proto_unknown;
	}

	expr = expr_alloc(loc, EXPR_PAYLOAD, tmpl->dtype,
			  tmpl->byteorder, tmpl->len);
	expr->flags |= flags;

	expr->payload.desc   = desc;
	expr->payload.tmpl   = tmpl;
	expr->payload.base   = base;
	expr->payload.offset = tmpl->offset;

	return expr;
}

void payload_init_raw(struct expr *expr, enum proto_bases base,
		      unsigned int offset, unsigned int len)
{
	enum th_hdr_fields thf;

	expr->payload.base	= base;
	expr->payload.offset	= offset;
	expr->len		= len;
	expr->dtype		= &xinteger_type;

	if (base != PROTO_BASE_TRANSPORT_HDR)
		return;
	if (len != 16)
		return;

	switch (offset) {
	case 0:
		thf = THDR_SPORT;
		/* fall through */
	case 16:
		if (offset == 16)
			thf = THDR_DPORT;
		expr->payload.tmpl = &proto_th.templates[thf];
		expr->payload.desc = &proto_th;
		expr->dtype = &inet_service_type;
		break;
	default:
		break;
	}
}

unsigned int payload_hdr_field(const struct expr *expr)
{
	return expr->payload.tmpl - expr->payload.desc->templates;
}

static void payload_stmt_print(const struct stmt *stmt, struct output_ctx *octx)
{
	expr_print(stmt->payload.expr, octx);
	nft_print(octx, " set ");
	expr_print(stmt->payload.val, octx);
}

static void payload_stmt_destroy(struct stmt *stmt)
{
	expr_free(stmt->payload.expr);
	expr_free(stmt->payload.val);
}

const struct stmt_ops payload_stmt_ops = {
	.type		= STMT_PAYLOAD,
	.name		= "payload",
	.print		= payload_stmt_print,
	.json		= payload_stmt_json,
	.destroy	= payload_stmt_destroy,
};

struct stmt *payload_stmt_alloc(const struct location *loc,
				struct expr *expr, struct expr *val)
{
	struct stmt *stmt;

	stmt = stmt_alloc(loc, &payload_stmt_ops);
	stmt->payload.expr = expr;
	stmt->payload.val  = val;
	return stmt;
}

static int payload_add_dependency(struct eval_ctx *ctx,
				  const struct proto_desc *desc,
				  const struct proto_desc *upper,
				  const struct expr *expr,
				  struct stmt **res)
{
	const struct proto_hdr_template *tmpl;
	struct expr *dep, *left, *right;
	struct proto_ctx *pctx;
	struct stmt *stmt;
	int protocol;

	protocol = proto_find_num(desc, upper);
	if (protocol < 0)
		return expr_error(ctx->msgs, expr,
				  "conflicting protocols specified: %s vs. %s",
				  desc->name, upper->name);

	tmpl = &desc->templates[desc->protocol_key];
	if (tmpl->meta_key)
		left = meta_expr_alloc(&expr->location, tmpl->meta_key);
	else
		left = payload_expr_alloc(&expr->location, desc, desc->protocol_key);

	right = constant_expr_alloc(&expr->location, tmpl->dtype,
				    tmpl->dtype->byteorder, tmpl->len,
				    constant_data_ptr(protocol, tmpl->len));

	dep = relational_expr_alloc(&expr->location, OP_EQ, left, right);

	stmt = expr_stmt_alloc(&dep->location, dep);
	if (stmt_dependency_evaluate(ctx, stmt) < 0)
		return -1;

	if (ctx->inner_desc) {
		if (tmpl->meta_key)
			left->meta.inner_desc = ctx->inner_desc;
		else
			left->payload.inner_desc = ctx->inner_desc;
	}

	pctx = eval_proto_ctx(ctx);
	relational_expr_pctx_update(pctx, dep);
	*res = stmt;
	return 0;
}

static const struct proto_desc *
payload_get_get_ll_hdr(const struct proto_ctx *pctx)
{
	switch (pctx->family) {
	case NFPROTO_INET:
		return &proto_inet;
	case NFPROTO_BRIDGE:
		return &proto_eth;
	case NFPROTO_NETDEV:
		return &proto_netdev;
	default:
		break;
	}

	return NULL;
}

static const struct proto_desc *
payload_gen_special_dependency(struct eval_ctx *ctx, const struct expr *expr)
{
	struct proto_ctx *pctx = eval_proto_ctx(ctx);

	switch (expr->payload.base) {
	case PROTO_BASE_LL_HDR:
		return payload_get_get_ll_hdr(pctx);
	case PROTO_BASE_TRANSPORT_HDR:
		if (expr->payload.desc == &proto_icmp ||
		    expr->payload.desc == &proto_icmp6 ||
		    expr->payload.desc == &proto_igmp) {
			const struct proto_desc *desc, *desc_upper;
			struct stmt *nstmt;

			desc = pctx->protocol[PROTO_BASE_LL_HDR].desc;
			if (!desc) {
				desc = payload_get_get_ll_hdr(pctx);
				if (!desc)
					break;
			}

			/* this tunnel protocol does not encapsulate an inner
			 * link layer, use proto_netdev which relies on
			 * NFT_META_PROTOCOL for dependencies.
			 */
			if (expr->payload.inner_desc &&
			    !(expr->payload.inner_desc->inner.flags & NFT_INNER_LL))
				desc = &proto_netdev;

			desc_upper = &proto_ip6;
			if (expr->payload.desc == &proto_icmp ||
			    expr->payload.desc == &proto_igmp)
				desc_upper = &proto_ip;

			if (payload_add_dependency(ctx, desc, desc_upper,
						   expr, &nstmt) < 0)
				return NULL;

			list_add_tail(&nstmt->list, &ctx->stmt->list);
			return desc_upper;
		}
		return &proto_inet_service;
	default:
		break;
	}
	return NULL;
}

/**
 * payload_gen_dependency - generate match expression on payload dependency
 *
 * @ctx:	evaluation context
 * @expr:	payload expression
 * @res:	dependency expression
 *
 * Generate matches on protocol dependencies. There are two different kinds
 * of dependencies:
 *
 * - A payload expression for a base above the hook base requires a match
 *   on the protocol value in the lower layer header.
 *
 * - A payload expression for a base below the hook base is invalid in the
 *   output path since the lower layer header does not exist when the packet
 *   is classified. In the input path a payload expressions for a base exactly
 *   one below the hook base is valid. In this case a match on the device type
 *   is required to verify that we're dealing with the expected protocol.
 *
 *   Note: since it is unknown to userspace which hooks a chain is called from,
 *   it is not explicitly verified. The NFT_META_IIFTYPE match will only match
 *   in the input path though.
 */
int payload_gen_dependency(struct eval_ctx *ctx, const struct expr *expr,
			   struct stmt **res)
{
	const struct hook_proto_desc *h;
	const struct proto_desc *desc;
	struct proto_ctx *pctx;
	struct stmt *stmt;
	uint16_t type;

	pctx = eval_proto_ctx(ctx);
	h = &hook_proto_desc[pctx->family];
	if (expr->payload.base < h->base) {
		if (expr->payload.base < h->base - 1)
			return expr_error(ctx->msgs, expr,
					  "payload base is invalid for this "
					  "family");

		if (proto_dev_type(expr->payload.desc, &type) < 0)
			return expr_error(ctx->msgs, expr,
					  "protocol specification is invalid "
					  "for this family");

		stmt = meta_stmt_meta_iiftype(&expr->location, type);
		if (stmt_dependency_evaluate(ctx, stmt) < 0)
			return -1;

		*res = stmt;

		return 0;
	}

	desc = pctx->protocol[expr->payload.base - 1].desc;
	/* Special case for mixed IPv4/IPv6 and bridge tables */
	if (desc == NULL)
		desc = payload_gen_special_dependency(ctx, expr);

	if (desc == NULL)
		return expr_error(ctx->msgs, expr,
				  "ambiguous payload specification: "
				  "no %s protocol specified",
				  proto_base_names[expr->payload.base - 1]);

	if (pctx->family == NFPROTO_BRIDGE && desc == &proto_eth) {
		/* prefer netdev proto, which adds dependencies based
		 * on skb->protocol.
		 *
		 * This has the advantage that we will also match
		 * vlan encapsulated traffic.
		 *
		 * eth_hdr(skb)->type would not match, as nft_payload
		 * will pretend vlan tag was not offloaded, i.e.
		 * type is ETH_P_8021Q in such a case, but skb->protocol
		 * would still match the l3 header type.
		 */
		if (expr->payload.desc == &proto_ip ||
		    expr->payload.desc == &proto_ip6)
			desc = &proto_netdev;
	}

	return payload_add_dependency(ctx, desc, expr->payload.desc, expr, res);
}

int exthdr_gen_dependency(struct eval_ctx *ctx, const struct expr *expr,
			  const struct proto_desc *dependency,
			  enum proto_bases pb, struct stmt **res)
{
	const struct proto_desc *desc;
	struct proto_ctx *pctx;

	pctx = eval_proto_ctx(ctx);
	desc = pctx->protocol[pb].desc;
	if (desc == NULL) {
		if (expr->exthdr.op == NFT_EXTHDR_OP_TCPOPT) {
			switch (pctx->family) {
			case NFPROTO_NETDEV:
			case NFPROTO_BRIDGE:
			case NFPROTO_INET:
				desc = &proto_inet_service;
				goto found;
			default:
				break;
			}
		}

		return expr_error(ctx->msgs, expr,
				  "Cannot generate dependency: "
				  "no %s protocol specified",
				  proto_base_names[pb]);
	}

 found:
	return payload_add_dependency(ctx, desc, dependency, expr, res);
}

/**
 * payload_is_stacked - return whether a payload protocol match defines a stacked
 * 			protocol on the same layer
 *
 * @desc: current protocol description on this layer
 * @expr: payload match
 */
bool payload_is_stacked(const struct proto_desc *desc, const struct expr *expr)
{
	const struct proto_desc *next;

	if (expr->left->etype != EXPR_PAYLOAD ||
	    !(expr->left->flags & EXPR_F_PROTOCOL) ||
	    expr->op != OP_EQ)
		return false;

	next = proto_find_upper(desc, mpz_get_be16(expr->right->value));
	return next && next->base == desc->base;
}

void payload_dependency_reset(struct payload_dep_ctx *ctx)
{
	memset(ctx, 0, sizeof(*ctx));
}

static bool payload_dependency_store_icmp_type(struct payload_dep_ctx *ctx,
					       const struct stmt *stmt)
{
	struct expr *dep = stmt->expr;
	const struct proto_desc *desc;
	const struct expr *right;
	uint8_t type;

	if (dep->left->etype != EXPR_PAYLOAD)
		return false;

	right = dep->right;
	if (right->etype != EXPR_VALUE || right->len != BITS_PER_BYTE)
		return false;

	desc = dep->left->payload.desc;
	if (desc == &proto_icmp) {
		type = mpz_get_uint8(right->value);

		if (type == ICMP_ECHOREPLY)
			type = ICMP_ECHO;

		ctx->icmp_type = type;

		return type == ICMP_ECHO;
	} else if (desc == &proto_icmp6) {
		type = mpz_get_uint8(right->value);

		ctx->icmp_type = type;
		return type == ICMP6_ECHO_REQUEST || type == ICMP6_ECHO_REPLY;
	}

	return false;
}

/**
 * payload_dependency_store - store a possibly redundant protocol match
 *
 * @ctx: payload dependency context
 * @stmt: payload match
 * @base: base of payload match
 */
void payload_dependency_store(struct payload_dep_ctx *ctx,
			      struct stmt *stmt, enum proto_bases base)
{
	bool ignore_dep = payload_dependency_store_icmp_type(ctx, stmt);

	if (ignore_dep)
		return;

	ctx->pdeps[base + 1] = stmt;
}

/**
 * payload_dependency_exists - there is a payload dependency in place
 * @ctx: payload dependency context
 * @base: payload protocol base
 *
 * Check if we have seen a protocol key payload expression for this base, we can
 * usually remove it if we can infer it from another payload expression in the
 * upper base.
 */
bool payload_dependency_exists(const struct payload_dep_ctx *ctx,
			       enum proto_bases base)
{
	if (ctx->pdeps[base])
		return true;

	return	base == PROTO_BASE_TRANSPORT_HDR &&
		ctx->pdeps[PROTO_BASE_INNER_HDR];
}

/**
 * payload_dependency_get - return a payload dependency if available
 * @ctx: payload dependency context
 * @base: payload protocol base
 *
 * If we have seen a protocol key payload expression for this base, we return
 * it.
 */
struct expr *payload_dependency_get(struct payload_dep_ctx *ctx,
				    enum proto_bases base)
{
	if (ctx->pdeps[base])
		return ctx->pdeps[base]->expr;

	if (base == PROTO_BASE_TRANSPORT_HDR &&
	    ctx->pdeps[PROTO_BASE_INNER_HDR])
		return ctx->pdeps[PROTO_BASE_INNER_HDR]->expr;

	return NULL;
}

static void __payload_dependency_release(struct payload_dep_ctx *ctx,
					 enum proto_bases base)
{
	list_del(&ctx->pdeps[base]->list);
	stmt_free(ctx->pdeps[base]);

	if (ctx->pdeps[base] == ctx->prev)
		ctx->prev = NULL;
	ctx->pdeps[base] = NULL;
}

void payload_dependency_release(struct payload_dep_ctx *ctx,
				enum proto_bases base)
{
	if (ctx->pdeps[base])
		__payload_dependency_release(ctx, base);
	else if (base == PROTO_BASE_TRANSPORT_HDR &&
		 ctx->pdeps[PROTO_BASE_INNER_HDR])
		__payload_dependency_release(ctx, PROTO_BASE_INNER_HDR);
}

static uint8_t icmp_dep_to_type(enum icmp_hdr_field_type t)
{
	switch (t) {
	case PROTO_ICMP_ANY:
		BUG("Invalid map for simple dependency");
	case PROTO_ICMP_ECHO: return ICMP_ECHO;
	case PROTO_ICMP6_ECHO: return ICMP6_ECHO_REQUEST;
	case PROTO_ICMP_MTU: return ICMP_DEST_UNREACH;
	case PROTO_ICMP_ADDRESS: return ICMP_REDIRECT;
	case PROTO_ICMP6_MTU: return ICMP6_PACKET_TOO_BIG;
	case PROTO_ICMP6_MGMQ: return MLD_LISTENER_QUERY;
	case PROTO_ICMP6_PPTR: return ICMP6_PARAM_PROB;
	case PROTO_ICMP6_REDIRECT: return ND_REDIRECT;
	case PROTO_ICMP6_ADDRESS: return ND_NEIGHBOR_SOLICIT;
	}

	BUG("Missing icmp type mapping");
}

static bool icmp_dep_type_match(enum icmp_hdr_field_type t, uint8_t type)
{
	switch (t) {
	case PROTO_ICMP_ECHO:
		return type == ICMP_ECHO || type == ICMP_ECHOREPLY;
	case PROTO_ICMP6_ECHO:
		return type == ICMP6_ECHO_REQUEST || type == ICMP6_ECHO_REPLY;
	case PROTO_ICMP6_ADDRESS:
		return type == ND_NEIGHBOR_SOLICIT ||
		       type == ND_NEIGHBOR_ADVERT ||
		       type == ND_REDIRECT ||
		       type == MLD_LISTENER_QUERY ||
		       type == MLD_LISTENER_REPORT ||
		       type == MLD_LISTENER_REDUCTION;
	case PROTO_ICMP_ADDRESS:
	case PROTO_ICMP_MTU:
	case PROTO_ICMP6_MTU:
	case PROTO_ICMP6_MGMQ:
	case PROTO_ICMP6_PPTR:
	case PROTO_ICMP6_REDIRECT:
		return icmp_dep_to_type(t) == type;
	case PROTO_ICMP_ANY:
		return true;
	}
	BUG("Missing icmp type mapping");
}

static bool payload_may_dependency_kill_icmp(struct payload_dep_ctx *ctx, const struct expr *expr)
{
	const struct expr *dep = payload_dependency_get(ctx, expr->payload.base);
	enum icmp_hdr_field_type icmp_dep;

	icmp_dep = expr->payload.tmpl->icmp_dep;
	if (icmp_dep == PROTO_ICMP_ANY)
		return false;

	if (dep->left->payload.desc != expr->payload.desc)
		return false;

	if (expr->payload.tmpl->icmp_dep == PROTO_ICMP_ECHO ||
	    expr->payload.tmpl->icmp_dep == PROTO_ICMP6_ECHO ||
	    expr->payload.tmpl->icmp_dep == PROTO_ICMP6_ADDRESS)
		return false;

	return ctx->icmp_type == icmp_dep_to_type(icmp_dep);
}

static bool payload_may_dependency_kill_ll(struct payload_dep_ctx *ctx, const struct expr *expr)
{
	const struct expr *dep = payload_dependency_get(ctx, expr->payload.base);

	/* Never remove a 'vlan type 0x...' expression, they are never added
	 * implicitly
	 */
	if (dep->left->payload.desc == &proto_vlan)
		return false;

	/* 'vlan id 2' implies 'ether type 8021Q'. If a different protocol is
	 * tested, this is not a redundant expression.
	 */
	if (dep->left->payload.desc == &proto_eth &&
	    dep->right->etype == EXPR_VALUE && dep->right->len == 16)
		return mpz_get_uint16(dep->right->value) == ETH_P_8021Q;

	return true;
}

static bool payload_may_dependency_kill(struct payload_dep_ctx *ctx,
					unsigned int family, struct expr *expr)
{
	struct expr *dep = payload_dependency_get(ctx, expr->payload.base);

	/* Protocol key payload expression at network base such as 'ip6 nexthdr'
	 * need to be left in place since it implicitly restricts matching to
	 * IPv6 for the bridge, inet and netdev families.
	 */
	switch (family) {
	case NFPROTO_BRIDGE:
	case NFPROTO_NETDEV:
	case NFPROTO_INET:
		if (dep->left->etype == EXPR_PAYLOAD &&
		    dep->left->payload.base == PROTO_BASE_NETWORK_HDR &&
		    (dep->left->payload.desc == &proto_ip ||
		     dep->left->payload.desc == &proto_ip6) &&
		    expr->payload.base == PROTO_BASE_TRANSPORT_HDR)
			return false;
		/* Do not kill
		 *  ether type vlan and vlan type ip and ip protocol icmp
		 * into
		 *  ip protocol icmp
		 * as this lacks ether type vlan.
		 * More generally speaking, do not kill protocol type
		 * for stacked protocols if we only have protcol type matches.
		 */
		if (dep->left->etype == EXPR_PAYLOAD && dep->op == OP_EQ &&
		    expr->payload.base == dep->left->payload.base) {
			if (expr->flags & EXPR_F_PROTOCOL)
				return false;

			if (expr->payload.base == PROTO_BASE_LL_HDR)
				return payload_may_dependency_kill_ll(ctx, expr);
		}

		break;
	}

	if (expr->payload.base != PROTO_BASE_TRANSPORT_HDR)
		return true;

	if (expr->payload.desc == &proto_th) {
		/* &proto_th could mean any of udp, tcp, dccp, ... so we
		 * cannot remove the dependency.
		 *
		 * Also prefer raw payload @th syntax, there is no
		 * 'source/destination port' protocol here.
		 */
		expr->payload.desc = &proto_unknown;
		expr->dtype = &xinteger_type;
		return false;
	}

	if (dep->left->etype != EXPR_PAYLOAD ||
	    dep->left->payload.base != PROTO_BASE_TRANSPORT_HDR)
		return true;

	if (dep->left->payload.desc == &proto_icmp)
		return payload_may_dependency_kill_icmp(ctx, expr);

	if (dep->left->payload.desc == &proto_icmp6)
		return payload_may_dependency_kill_icmp(ctx, expr);

	return true;
}

/**
 * payload_dependency_kill - kill a redundant payload dependency
 *
 * @ctx: payload dependency context
 * @expr: higher layer payload expression
 *
 * Kill a redundant payload expression if a higher layer payload expression
 * implies its existence. Skip this if the dependency is a network payload and
 * we are in bridge, netdev and inet families.
 */
void payload_dependency_kill(struct payload_dep_ctx *ctx, struct expr *expr,
			     unsigned int family)
{
	if (expr->payload.desc != &proto_unknown &&
	    payload_dependency_exists(ctx, expr->payload.base) &&
	    payload_may_dependency_kill(ctx, family, expr))
		payload_dependency_release(ctx, expr->payload.base);
}

void exthdr_dependency_kill(struct payload_dep_ctx *ctx, struct expr *expr,
			    unsigned int family)
{
	switch (expr->exthdr.op) {
	case NFT_EXTHDR_OP_TCPOPT:
		if (payload_dependency_exists(ctx, PROTO_BASE_TRANSPORT_HDR))
			payload_dependency_release(ctx, PROTO_BASE_TRANSPORT_HDR);
		break;
	case NFT_EXTHDR_OP_IPV6:
		if (payload_dependency_exists(ctx, PROTO_BASE_NETWORK_HDR))
			payload_dependency_release(ctx, PROTO_BASE_NETWORK_HDR);
		break;
	case NFT_EXTHDR_OP_IPV4:
		if (payload_dependency_exists(ctx, PROTO_BASE_NETWORK_HDR))
			payload_dependency_release(ctx, PROTO_BASE_NETWORK_HDR);
		break;
	default:
		break;
	}
}

static const struct proto_desc *get_stacked_desc(const struct proto_ctx *ctx,
						 const struct proto_desc *top,
						 const struct expr *e,
						 unsigned int *skip)
{
	unsigned int i, total, payload_offset = e->payload.offset;

	assert(e->etype == EXPR_PAYLOAD);

	if (e->payload.base != PROTO_BASE_LL_HDR ||
	    payload_offset < top->length) {
		*skip = 0;
		return top;
	}

	for (i = 0, total = 0; i < ctx->stacked_ll_count; i++) {
		const struct proto_desc *stacked;

		stacked = ctx->stacked_ll[i];
		if (payload_offset < stacked->length) {
			*skip = total;
			return stacked;
		}

		payload_offset -= stacked->length;
		total += stacked->length;
	}

	*skip = total;
	return top;
}

/**
 * payload_expr_complete - fill in type information of a raw payload expr
 *
 * @expr:	the payload expression
 * @ctx:	protocol context
 *
 * Complete the type of a raw payload expression based on the context. If
 * insufficient information is available the expression remains unchanged.
 */
void payload_expr_complete(struct expr *expr, const struct proto_ctx *ctx)
{
	unsigned int payload_offset = expr->payload.offset;
	const struct proto_desc *desc;
	const struct proto_hdr_template *tmpl;
	unsigned int i, total;

	assert(expr->etype == EXPR_PAYLOAD);

	desc = ctx->protocol[expr->payload.base].desc;
	if (desc == NULL || desc == &proto_inet)
		return;
	assert(desc->base == expr->payload.base);

	desc = get_stacked_desc(ctx, desc, expr, &total);
	payload_offset -= total;

	for (i = 0; i < array_size(desc->templates); i++) {
		tmpl = &desc->templates[i];
		if (tmpl->offset != payload_offset ||
		    tmpl->len    != expr->len)
			continue;

		if (tmpl->meta_key && i == 0)
			continue;

		if (tmpl->icmp_dep && ctx->th_dep.icmp.type &&
		    !icmp_dep_type_match(tmpl->icmp_dep,
					 ctx->th_dep.icmp.type))
			continue;

		expr->dtype	   = tmpl->dtype;
		expr->payload.desc = desc;
		expr->byteorder = tmpl->byteorder;
		expr->payload.tmpl = tmpl;
		return;
	}
}

static unsigned int mask_to_offset(const struct expr *mask)
{
	return mask ? mpz_scan1(mask->value, 0) : 0;
}

static unsigned int mask_length(const struct expr *mask)
{
	unsigned long off;

        off = mask_to_offset(mask);

	return mpz_scan0(mask->value, off + 1);
}

/**
 * payload_expr_trim - trim payload expression according to mask
 *
 * @expr:	the payload expression
 * @mask:	mask to use when searching templates
 * @ctx:	protocol context
 * @shift:	shift adjustment to fix up RHS value
 *
 * Walk the template list and determine if a match can be found without
 * using the provided mask.
 *
 * If the mask has to be used, trim the payload expression length accordingly,
 * adjust the payload offset and return true to let the caller know that the
 * mask can be removed. This function also returns the shift for the right hand
 * constant side of the expression.
 */
bool payload_expr_trim(struct expr *expr, struct expr *mask,
		       const struct proto_ctx *ctx, unsigned int *shift)
{
	unsigned int payload_offset = expr->payload.offset;
	unsigned int mask_offset = mask_to_offset(mask);
	unsigned int mask_len = mask_length(mask);
	const struct proto_hdr_template *tmpl;
	unsigned int payload_len = expr->len;
	const struct proto_desc *desc;
	unsigned int off, i, len = 0;
	unsigned int total;

	assert(expr->etype == EXPR_PAYLOAD);

	desc = ctx->protocol[expr->payload.base].desc;
	if (desc == NULL)
		return false;

	assert(desc->base == expr->payload.base);

	desc = get_stacked_desc(ctx, desc, expr, &total);
	payload_offset -= total;

	off = round_up(mask->len, BITS_PER_BYTE) - mask_len;
	payload_offset += off;

	for (i = 1; i < array_size(desc->templates); i++) {
		tmpl = &desc->templates[i];
		if (tmpl->offset != payload_offset)
			continue;

		if (tmpl->len > payload_len)
			return false;

		payload_len -= tmpl->len;
		payload_offset += tmpl->len;
		len += tmpl->len;
		if (payload_len == 0)
			return false;

		if (mask_offset + len == mask_len) {
			expr->payload.offset += off;
			expr->len = len;
			*shift = mask_offset;
			return true;
		}
	}

	return false;
}

/**
 * payload_expr_trim_force - adjust payload len/offset according to mask
 *
 * @expr:	the payload expression
 * @mask:	mask to use when searching templates
 * @shift:	shift adjustment to fix up RHS value
 *
 * Force-trim an unknown payload expression according to mask.
 *
 * This is only useful for unkown payload expressions that need
 * to be printed in raw syntax (@base,offset,length).  The kernel
 * can only deal with byte-divisible offsets/length, e.g. @th,16,8.
 * In such case we might be able to get rid of the mask.
 * @base,offset,length & MASK OPERATOR VALUE then becomes
 * @base,offset,length VALUE, where at least one of offset increases
 * and length decreases.
 *
 * This function also returns the shift for the right hand
 * constant side of the expression.
 *
 * @return: true if @expr was adjusted and mask can be discarded.
 */
bool payload_expr_trim_force(struct expr *expr, struct expr *mask, unsigned int *shift)
{
	unsigned int payload_offset = expr->payload.offset;
	unsigned int mask_len = mask_length(mask);
	unsigned int off, real_len;

	if (payload_is_known(expr) || expr->len <= mask_len)
		return false;

	/* This provides the payload offset to use.
	 * mask->len is the total length of the mask, e.g. 16.
	 * mask_len holds the last bit number that will be zeroed,
	 */
	off = round_up(mask->len, BITS_PER_BYTE) - mask_len;
	payload_offset += off;

	/* kernel only allows offsets <= 255 */
	if (round_up(payload_offset, BITS_PER_BYTE) > 255)
		return false;

	real_len = mpz_popcount(mask->value);
	if (real_len > expr->len)
		return false;

	expr->payload.offset = payload_offset;
	expr->len = real_len;

	*shift = mask_to_offset(mask);
	return true;
}

/**
 * stmt_payload_expr_trim - adjust payload len/offset according to mask
 *
 * @stmt:	the payload statement
 * @pctx:	protocol context
 *
 * Infer offset to header field from mask, walk the template list to determine
 * if offset falls within a matching header field.
 *
 * Trim the payload expression length accordingly, adjust the payload offset
 * and return true if payload statement expressions has been updated.
 *
 * @return: true if @stmt was adjusted.
 */
bool stmt_payload_expr_trim(struct stmt *stmt, const struct proto_ctx *pctx)
{
	struct expr *expr = stmt->payload.val;
	const struct proto_hdr_template *tmpl;
	const struct proto_desc *desc;
	struct expr *payload, *mask;
	uint32_t offset, i, shift;
	unsigned int mask_offset;
	mpz_t bitmask, tmp, tmp2;
	unsigned long n;

	assert(stmt->type == STMT_PAYLOAD);
	assert(expr->etype == EXPR_BINOP);

	payload = expr->left;
	mask = expr->right;

	if (payload->etype != EXPR_PAYLOAD ||
	    mask->etype != EXPR_VALUE)
		return false;

	if (payload_is_known(payload) ||
	    !pctx->protocol[payload->payload.base].desc ||
	    payload->len % (2 * BITS_PER_BYTE) != 0)
		return false;

	switch (expr->op) {
	case OP_AND:
		/* infer offset from first 0 in mask */
		n = mpz_scan0(mask->value, 0);
		if (n == ULONG_MAX)
			return false;

		mask_offset = payload->len - n;
		break;
	case OP_OR:
	case OP_XOR:
		/* infer offset from first 1 in mask */
		n = mpz_scan1(mask->value, 0);
		if (n == ULONG_MAX)
			return false;

		mask_offset = payload->len - n;
		break;
	default:
		return false;
	}

	offset = payload->payload.offset + mask_offset;

	desc = pctx->protocol[payload->payload.base].desc;
	for (i = 1; i < array_size(desc->templates); i++) {
		tmpl = &desc->templates[i];

		if (tmpl->len == 0)
			return false;

		/* Is this inferred offset within this header field? */
		if (tmpl->offset + tmpl->len >= offset) {
			/* Infer shift to reach this header field. */
			if ((tmpl->offset % (2 * BITS_PER_BYTE)) < 8) {
				shift = BITS_PER_BYTE - (tmpl->offset % BITS_PER_BYTE + tmpl->len);
				shift += BITS_PER_BYTE;
			} else {
				shift = (2 * BITS_PER_BYTE) - (tmpl->offset % (2 * BITS_PER_BYTE) + tmpl->len);
			}

			/* Build bitmask to fetch this header field. */
			mpz_init2(bitmask, payload->len);
			mpz_bitmask(bitmask, tmpl->len);
			if (shift)
				mpz_lshift_ui(bitmask, shift);

			/* Check if mask expression falls within this header
			 * bitfield, if the mask expression is over this header
			 * field, then skip this delinearization, this could be
			 * a raw expression.
			 */
			switch (expr->op) {
			case OP_AND:
				/* Inverted bitmask to fetch untouched bits. */
				mpz_init_bitmask(tmp, payload->len);
				mpz_xor(tmp, bitmask, tmp);

				/* Get untouched bits out of the header field. */
				mpz_init2(tmp2, payload->len);
				mpz_and(tmp2, mask->value, tmp);

				/* Modified any bits out of the header field? */
				if (mpz_cmp(tmp, tmp2) != 0) {
					mpz_clear(tmp);
					mpz_clear(tmp2);
					mpz_clear(bitmask);
					return false;
				}
				mpz_clear(tmp2);
				break;
			case OP_OR:
			case OP_XOR:
				mpz_init2(tmp, payload->len);

				/* Get modified bits in header field. */
				mpz_and(tmp, mask->value, bitmask);

				/* Modified any bits out of the header field? */
				if (mpz_cmp(tmp, mask->value) != 0) {
					mpz_clear(tmp);
					mpz_clear(bitmask);
					return false;
				}
				break;
			default:
				assert(0);
				break;
			}
			mpz_clear(tmp);

			/* Clear unrelated bits for this header field. Shrink
			 * to "real size". Shift bits when needed.
			 */
			mpz_and(mask->value, bitmask, mask->value);
			mpz_clear(bitmask);

			mask->len -= (tmpl->offset - payload->payload.offset);
			if (shift) {
				mask->len -= shift;
				mpz_rshift_ui(mask->value, shift);
			}
			payload->payload.offset = tmpl->offset;
			payload->len = tmpl->len;

			expr_free(stmt->payload.expr);
			stmt->payload.expr = expr_get(payload);

			if (expr->op == OP_AND) {
				/* Reduce 'expr AND 0x0', otherwise listing
				 * shows:
				 *
				 *	ip dscp set ip dscp & 0x0
				 *
				 * instead of the more compact:
				 *
				 *	ip dscp set 0x0
				 */
				if (mpz_cmp_ui(mask->value, 0) == 0) {
					expr = stmt->payload.val;
					stmt->payload.val = expr_get(mask);
					expr_free(expr);
				}
			}
			return true;
		}
	}

	return false;
}

/**
 * payload_expr_expand - expand raw merged adjacent payload expressions into its
 * 			 original components
 *
 * @list:	list to append expanded payload expressions to
 * @expr:	the payload expression to expand
 * @ctx:	protocol context
 *
 * Expand a merged adjacent payload expression into its original components
 * by splitting elements off the beginning matching a payload template.
 *
 * Note: this requires all payload templates to be specified in ascending
 * 	 offset order.
 */
void payload_expr_expand(struct list_head *list, struct expr *expr,
			 const struct proto_ctx *ctx)
{
	unsigned int payload_offset = expr->payload.offset;
	const struct proto_hdr_template *tmpl;
	const struct proto_desc *desc;
	unsigned int i, total;
	struct expr *new;

	assert(expr->etype == EXPR_PAYLOAD);

	desc = ctx->protocol[expr->payload.base].desc;
	if (desc == NULL || desc == &proto_unknown)
		goto raw;

	assert(desc->base == expr->payload.base);

	desc = get_stacked_desc(ctx, desc, expr, &total);
	payload_offset -= total;

	for (i = 1; i < array_size(desc->templates); i++) {
		tmpl = &desc->templates[i];

		if (tmpl->len == 0)
			break;

		if (tmpl->offset != payload_offset)
			continue;

		if (tmpl->icmp_dep && ctx->th_dep.icmp.type &&
		    !icmp_dep_type_match(tmpl->icmp_dep,
					 ctx->th_dep.icmp.type))
			continue;

		if (tmpl->len <= expr->len) {
			new = payload_expr_alloc(&expr->location, desc, i);
			list_add_tail(&new->list, list);
			expr->len	     -= tmpl->len;
			expr->payload.offset += tmpl->len;
			payload_offset       += tmpl->len;
			if (expr->len == 0)
				return;
		} else if (expr->len > 0) {
			new = payload_expr_alloc(&expr->location, desc, i);
			new->len = expr->len;
			list_add_tail(&new->list, list);
			return;
		} else
			break;
	}
raw:
	new = payload_expr_alloc(&expr->location, NULL, 0);
	payload_init_raw(new, expr->payload.base, payload_offset,
			 expr->len);

	if (expr->payload.inner_desc)
		new->dtype = &integer_type;

	list_add_tail(&new->list, list);
}

static bool payload_is_adjacent(const struct expr *e1, const struct expr *e2)
{
	if (e1->payload.base		 == e2->payload.base &&
	    e1->payload.offset + e1->len == e2->payload.offset)
		return true;
	return false;
}

/**
 * payload_can_merge - return whether two payload expressions can be merged
 *
 * @e1:		first payload expression
 * @e2:		second payload expression
 */
bool payload_can_merge(const struct expr *e1, const struct expr *e2)
{
	unsigned int total;

	if (e1->payload.inner_desc != e2->payload.inner_desc)
		return false;

	if (!payload_is_adjacent(e1, e2))
		return false;

	if (e1->payload.offset % BITS_PER_BYTE || e1->len % BITS_PER_BYTE ||
	    e2->payload.offset % BITS_PER_BYTE || e2->len % BITS_PER_BYTE)
		return false;

	total = e1->len + e2->len;
	if (total < e1->len || total > (NFT_REG_SIZE * BITS_PER_BYTE))
		return false;

	/* could return true after this, the expressions are mergeable.
	 *
	 * However, there are some caveats.
	 *
	 * Loading anything <= sizeof(u32) with base >= network header
	 * is fast, because its handled directly from eval loop in the
	 * kernel.
	 *
	 * We thus restrict merging a bit more.
	 */

	/* can still be handled by fastpath after merge */
	if (total <= NFT_REG32_SIZE * BITS_PER_BYTE)
		return true;

	/* Linklayer base is not handled in fastpath, merge */
	if (e1->payload.base == PROTO_BASE_LL_HDR)
		return true;

	/* Also merge if at least one expression is already
	 * above REG32 size, in this case merging is faster.
	 */
	if (e1->len > (NFT_REG32_SIZE * BITS_PER_BYTE) ||
	    e2->len > (NFT_REG32_SIZE * BITS_PER_BYTE))
		return true;

	return false;
}

/**
 * payload_expr_join - join two adjacent payload expressions
 *
 * @e1:		first payload expression
 * @e2:		second payload expression
 */
struct expr *payload_expr_join(const struct expr *e1, const struct expr *e2)
{
	struct expr *expr;

	assert(payload_is_adjacent(e1, e2));

	expr = payload_expr_alloc(&internal_location, NULL, 0);
	expr->payload.base   = e1->payload.base;
	expr->payload.offset = e1->payload.offset;
	expr->len	     = e1->len + e2->len;
	expr->payload.inner_desc = e1->payload.inner_desc;

	return expr;
}

static struct stmt *
__payload_gen_icmp_simple_dependency(struct eval_ctx *ctx, const struct expr *expr,
				     const struct datatype *icmp_type,
				     const struct proto_desc *desc,
				     uint8_t type)
{
	struct expr *left, *right, *dep;

	left = payload_expr_alloc(&expr->location, desc, desc->protocol_key);
	right = constant_expr_alloc(&expr->location, icmp_type,
				    BYTEORDER_BIG_ENDIAN, BITS_PER_BYTE,
				    constant_data_ptr(type, BITS_PER_BYTE));

	dep = relational_expr_alloc(&expr->location, OP_EQ, left, right);
	return expr_stmt_alloc(&dep->location, dep);
}

static struct stmt *
__payload_gen_icmp_echo_dependency(struct eval_ctx *ctx, const struct expr *expr,
				   uint8_t echo, uint8_t reply,
				   const struct datatype *icmp_type,
				   const struct proto_desc *desc)
{
	struct expr *left, *right, *dep, *set;

	left = payload_expr_alloc(&expr->location, desc, desc->protocol_key);

	set = set_expr_alloc(&expr->location, NULL);

	right = constant_expr_alloc(&expr->location, icmp_type,
				    BYTEORDER_BIG_ENDIAN, BITS_PER_BYTE,
				    constant_data_ptr(echo, BITS_PER_BYTE));
	right = set_elem_expr_alloc(&expr->location, right);
	set_expr_add(set, right);

	right = constant_expr_alloc(&expr->location, icmp_type,
				    BYTEORDER_BIG_ENDIAN, BITS_PER_BYTE,
				    constant_data_ptr(reply, BITS_PER_BYTE));
	right = set_elem_expr_alloc(&expr->location, right);
	set_expr_add(set, right);

	dep = relational_expr_alloc(&expr->location, OP_IMPLICIT, left, set);
	return expr_stmt_alloc(&dep->location, dep);
}

static struct stmt *
__payload_gen_icmp6_addr_dependency(struct eval_ctx *ctx, const struct expr *expr,
				    const struct proto_desc *desc)
{
	static const uint8_t icmp_addr_types[] = {
		MLD_LISTENER_QUERY,
		MLD_LISTENER_REPORT,
		MLD_LISTENER_REDUCTION,
		ND_NEIGHBOR_SOLICIT,
		ND_NEIGHBOR_ADVERT,
		ND_REDIRECT
	};
	struct expr *left, *right, *dep, *set;
	size_t i;

	left = payload_expr_alloc(&expr->location, desc, desc->protocol_key);

	set = set_expr_alloc(&expr->location, NULL);

	for (i = 0; i < array_size(icmp_addr_types); ++i) {
		right = constant_expr_alloc(&expr->location, &icmp6_type_type,
					    BYTEORDER_BIG_ENDIAN, BITS_PER_BYTE,
					    constant_data_ptr(icmp_addr_types[i],
							      BITS_PER_BYTE));
		right = set_elem_expr_alloc(&expr->location, right);
		set_expr_add(set, right);
	}

	dep = relational_expr_alloc(&expr->location, OP_IMPLICIT, left, set);
	return expr_stmt_alloc(&dep->location, dep);
}

int payload_gen_icmp_dependency(struct eval_ctx *ctx, const struct expr *expr,
				struct stmt **res)
{
	struct proto_ctx *pctx = eval_proto_ctx(ctx);
	const struct proto_hdr_template *tmpl;
	const struct proto_desc *desc;
	struct stmt *stmt = NULL;
	uint8_t type;

	assert(expr->etype == EXPR_PAYLOAD);

	tmpl = expr->payload.tmpl;
	desc = expr->payload.desc;

	switch (tmpl->icmp_dep) {
	case PROTO_ICMP_ANY:
		BUG("No dependency needed");
		break;
	case PROTO_ICMP_ECHO:
		/* do not test ICMP_ECHOREPLY here: its 0 */
		if (pctx->th_dep.icmp.type == ICMP_ECHO)
			goto done;

		type = ICMP_ECHO;
		if (pctx->th_dep.icmp.type)
			goto bad_proto;

		stmt = __payload_gen_icmp_echo_dependency(ctx, expr,
							  ICMP_ECHO, ICMP_ECHOREPLY,
							  &icmp_type_type,
							  desc);
		break;
	case PROTO_ICMP_MTU:
	case PROTO_ICMP_ADDRESS:
		type = icmp_dep_to_type(tmpl->icmp_dep);
		if (pctx->th_dep.icmp.type == type)
			goto done;
		if (pctx->th_dep.icmp.type)
			goto bad_proto;
		stmt = __payload_gen_icmp_simple_dependency(ctx, expr,
							    &icmp_type_type,
							    desc, type);
		break;
	case PROTO_ICMP6_ECHO:
		if (pctx->th_dep.icmp.type == ICMP6_ECHO_REQUEST ||
		    pctx->th_dep.icmp.type == ICMP6_ECHO_REPLY)
			goto done;

		type = ICMP6_ECHO_REQUEST;
		if (pctx->th_dep.icmp.type)
			goto bad_proto;

		stmt = __payload_gen_icmp_echo_dependency(ctx, expr,
							  ICMP6_ECHO_REQUEST,
							  ICMP6_ECHO_REPLY,
							  &icmp6_type_type,
							  desc);
		break;
	case PROTO_ICMP6_ADDRESS:
		if (icmp_dep_type_match(PROTO_ICMP6_ADDRESS,
					pctx->th_dep.icmp.type))
			goto done;
		type = ND_NEIGHBOR_SOLICIT;
		if (pctx->th_dep.icmp.type)
			goto bad_proto;
		stmt = __payload_gen_icmp6_addr_dependency(ctx, expr, desc);
		break;
	case PROTO_ICMP6_REDIRECT:
	case PROTO_ICMP6_MTU:
	case PROTO_ICMP6_MGMQ:
	case PROTO_ICMP6_PPTR:
		type = icmp_dep_to_type(tmpl->icmp_dep);
		if (pctx->th_dep.icmp.type == type)
			goto done;
		if (pctx->th_dep.icmp.type)
			goto bad_proto;
		stmt = __payload_gen_icmp_simple_dependency(ctx, expr,
							    &icmp6_type_type,
							    desc, type);
		break;
		break;
	default:
		BUG("Unhandled icmp dependency code");
	}

	pctx->th_dep.icmp.type = type;

	if (stmt_dependency_evaluate(ctx, stmt) < 0)
		return -1;
done:
	*res = stmt;
	return 0;

bad_proto:
	return expr_error(ctx->msgs, expr, "incompatible icmp match: rule has %d, need %u",
			  pctx->th_dep.icmp.type, type);
}

int payload_gen_inner_dependency(struct eval_ctx *ctx, const struct expr *expr,
				 struct stmt **res)
{
	struct proto_ctx *pctx = eval_proto_ctx(ctx);
	const struct proto_hdr_template *tmpl;
	const struct proto_desc *desc, *inner_desc;
	struct expr *left, *right, *dep;
	struct stmt *stmt = NULL;
	int protocol;

	assert(expr->etype == EXPR_PAYLOAD);

	inner_desc = expr->payload.inner_desc;
	desc = pctx->protocol[inner_desc->base - 1].desc;
	if (desc == NULL)
		desc = &proto_ip;

	tmpl = &inner_desc->templates[0];
	assert(tmpl);

	protocol = proto_find_num(desc, inner_desc);
	if (protocol < 0)
                return expr_error(ctx->msgs, expr,
                                  "conflicting protocols specified: %s vs. %s",
                                  desc->name, inner_desc->name);

	left = meta_expr_alloc(&expr->location, tmpl->meta_key);

	right = constant_expr_alloc(&expr->location, tmpl->dtype,
				    tmpl->dtype->byteorder, tmpl->len,
				    constant_data_ptr(protocol, tmpl->len));

	dep = relational_expr_alloc(&expr->location, OP_EQ, left, right);
	stmt = expr_stmt_alloc(&dep->location, dep);

	*res = stmt;
	return 0;
}
