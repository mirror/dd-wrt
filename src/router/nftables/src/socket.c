/*
 * Socket expression/statement related definition and types.
 *
 * Copyright (c) 2018 Máté Eckl <ecklm94@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 (or any
 * later) as published by the Free Software Foundation.
 */

#include <nft.h>

#include <nftables.h>
#include <expression.h>
#include <socket.h>
#include <json.h>

const struct socket_template socket_templates[] = {
	[NFT_SOCKET_TRANSPARENT] = {
		.token		= "transparent",
		.dtype		= &integer_type,
		.len		= BITS_PER_BYTE,
		.byteorder	= BYTEORDER_HOST_ENDIAN,
	},
	[NFT_SOCKET_MARK] = {
		.token		= "mark",
		.dtype		= &mark_type,
		.len		= 4 * BITS_PER_BYTE,
		.byteorder	= BYTEORDER_HOST_ENDIAN,
	},
	[NFT_SOCKET_WILDCARD] = {
		.token		= "wildcard",
		.dtype		= &integer_type,
		.len		= BITS_PER_BYTE,
		.byteorder	= BYTEORDER_HOST_ENDIAN,
	},
	[NFT_SOCKET_CGROUPV2] = {
		.token		= "cgroupv2",
		.dtype		= &cgroupv2_type,
		.len		= 8 * BITS_PER_BYTE,
		.byteorder	= BYTEORDER_HOST_ENDIAN,
	},
};

static void socket_expr_print(const struct expr *expr, struct output_ctx *octx)
{
	nft_print(octx, "socket %s", socket_templates[expr->socket.key].token);
	if (expr->socket.key == NFT_SOCKET_CGROUPV2)
		nft_print(octx, " level %u", expr->socket.level);
}

static bool socket_expr_cmp(const struct expr *e1, const struct expr *e2)
{
	return e1->socket.key == e2->socket.key &&
	       e1->socket.level == e2->socket.level;
}

static void socket_expr_clone(struct expr *new, const struct expr *expr)
{
	new->socket.key = expr->socket.key;
	new->socket.level = expr->socket.level;
}

#define NFTNL_UDATA_SOCKET_KEY 0
#define NFTNL_UDATA_SOCKET_MAX 1

static int socket_expr_build_udata(struct nftnl_udata_buf *udbuf,
				 const struct expr *expr)
{
	nftnl_udata_put_u32(udbuf, NFTNL_UDATA_SOCKET_KEY, expr->socket.key);

	return 0;
}

static int socket_parse_udata(const struct nftnl_udata *attr, void *data)
{
	const struct nftnl_udata **ud = data;
	uint8_t type = nftnl_udata_type(attr);
	uint8_t len = nftnl_udata_len(attr);

	switch (type) {
	case NFTNL_UDATA_SOCKET_KEY:
		if (len != sizeof(uint32_t))
			return -1;
		break;
	default:
		return 0;
	}

	ud[type] = attr;
	return 0;
}

static struct expr *socket_expr_parse_udata(const struct nftnl_udata *attr)
{
	const struct nftnl_udata *ud[NFTNL_UDATA_SOCKET_MAX + 1] = {};
	uint32_t key;
	int err;

	err = nftnl_udata_parse(nftnl_udata_get(attr), nftnl_udata_len(attr),
				socket_parse_udata, ud);
	if (err < 0)
		return NULL;

	if (!ud[NFTNL_UDATA_SOCKET_KEY])
		return NULL;

	key = nftnl_udata_get_u32(ud[NFTNL_UDATA_SOCKET_KEY]);

	return socket_expr_alloc(&internal_location, key, 0);
}

const struct expr_ops socket_expr_ops = {
	.type		= EXPR_SOCKET,
	.name		= "socket",
	.print		= socket_expr_print,
	.json		= socket_expr_json,
	.cmp		= socket_expr_cmp,
	.clone		= socket_expr_clone,
	.build_udata	= socket_expr_build_udata,
	.parse_udata	= socket_expr_parse_udata,
};

struct expr *socket_expr_alloc(const struct location *loc,
			       enum nft_socket_keys key, uint32_t level)
{
	const struct socket_template *tmpl = &socket_templates[key];
	struct expr *expr;

	expr = expr_alloc(loc, EXPR_SOCKET, tmpl->dtype,
			  tmpl->byteorder, tmpl->len);
	expr->socket.key = key;
	expr->socket.level = level;

	return expr;
}
