/*
 * Copyright (c) 2018 Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <nft.h>

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <pwd.h>
#include <grp.h>
#include <arpa/inet.h>
#include <linux/netfilter.h>
#include <linux/pkt_sched.h>
#include <linux/if_packet.h>

#include <nftables.h>
#include <expression.h>
#include <datatype.h>
#include <tunnel.h>
#include <gmputil.h>
#include <utils.h>
#include <erec.h>

const struct tunnel_template tunnel_templates[] = {
	[NFT_TUNNEL_PATH]	= META_TEMPLATE("path", &boolean_type,
						BITS_PER_BYTE, BYTEORDER_HOST_ENDIAN),
	[NFT_TUNNEL_ID]		= META_TEMPLATE("id",  &integer_type,
						4 * 8, BYTEORDER_HOST_ENDIAN),
};

struct error_record *tunnel_key_parse(const struct location *loc,
				      const char *str,
				      unsigned int *value)
{
	unsigned int i;

	for (i = 0; i < array_size(tunnel_templates); i++) {
		if (!tunnel_templates[i].token ||
		    strcmp(tunnel_templates[i].token, str))
			continue;

		*value = i;
		return NULL;
	}

	return error(loc, "syntax error, unexpected %s", str);
}

static void tunnel_expr_print(const struct expr *expr, struct output_ctx *octx)
{
	uint32_t key = expr->tunnel.key;
	const char *token = "unknown";

	if (key < array_size(tunnel_templates))
		token = tunnel_templates[key].token;

	nft_print(octx, "tunnel %s", token);
}

static bool tunnel_expr_cmp(const struct expr *e1, const struct expr *e2)
{
	return e1->tunnel.key == e2->tunnel.key;
}

static void tunnel_expr_clone(struct expr *new, const struct expr *expr)
{
	new->tunnel.key = expr->tunnel.key;
}

const struct expr_ops tunnel_expr_ops = {
	.type		= EXPR_TUNNEL,
	.name		= "tunnel",
	.print		= tunnel_expr_print,
	.json		= tunnel_expr_json,
	.cmp		= tunnel_expr_cmp,
	.clone		= tunnel_expr_clone,
};

struct expr *tunnel_expr_alloc(const struct location *loc,
			       enum nft_tunnel_keys key)
{
	const struct tunnel_template *tmpl = &tunnel_templates[key];
	struct expr *expr;

	expr = expr_alloc(loc, EXPR_TUNNEL, tmpl->dtype, tmpl->byteorder,
			  tmpl->len);
	expr->tunnel.key = key;

	return expr;
}
