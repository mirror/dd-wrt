/*
 * (C) 2012-2014 by Pablo Neira Ayuso <pablo@netfilter.org>
 * (C) 2013 by Tomasz Bursztyka <tomasz.bursztyka@linux.intel.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This code has been sponsored by Sophos Astaro <http://www.sophos.com>
 */

#include <string.h>
#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netdb.h>

#include <xtables.h>

#include <linux/netfilter/nf_tables.h>

#include "nft.h"
#include "nft-shared.h"

static int nft_ipv4_add(struct nft_handle *h, struct nftnl_rule *r, void *data)
{
	struct iptables_command_state *cs = data;
	struct xtables_rule_match *matchp;
	uint32_t op;
	int ret;

	if (cs->fw.ip.iniface[0] != '\0') {
		op = nft_invflags2cmp(cs->fw.ip.invflags, IPT_INV_VIA_IN);
		add_iniface(r, cs->fw.ip.iniface, op);
	}

	if (cs->fw.ip.outiface[0] != '\0') {
		op = nft_invflags2cmp(cs->fw.ip.invflags, IPT_INV_VIA_OUT);
		add_outiface(r, cs->fw.ip.outiface, op);
	}

	if (cs->fw.ip.proto != 0) {
		op = nft_invflags2cmp(cs->fw.ip.invflags, XT_INV_PROTO);
		add_l4proto(r, cs->fw.ip.proto, op);
	}

	if (cs->fw.ip.src.s_addr || cs->fw.ip.smsk.s_addr || cs->fw.ip.invflags & IPT_INV_SRCIP) {
		op = nft_invflags2cmp(cs->fw.ip.invflags, IPT_INV_SRCIP);
		add_addr(r, offsetof(struct iphdr, saddr),
			 &cs->fw.ip.src.s_addr, &cs->fw.ip.smsk.s_addr,
			 sizeof(struct in_addr), op);
	}
	if (cs->fw.ip.dst.s_addr || cs->fw.ip.dmsk.s_addr || cs->fw.ip.invflags & IPT_INV_DSTIP) {
		op = nft_invflags2cmp(cs->fw.ip.invflags, IPT_INV_DSTIP);
		add_addr(r, offsetof(struct iphdr, daddr),
			 &cs->fw.ip.dst.s_addr, &cs->fw.ip.dmsk.s_addr,
			 sizeof(struct in_addr), op);
	}
	if (cs->fw.ip.flags & IPT_F_FRAG) {
		add_payload(r, offsetof(struct iphdr, frag_off), 2,
			    NFT_PAYLOAD_NETWORK_HEADER);
		/* get the 13 bits that contain the fragment offset */
		add_bitwise_u16(r, htons(0x1fff), 0);

		/* if offset is non-zero, this is a fragment */
		op = NFT_CMP_NEQ;
		if (cs->fw.ip.invflags & IPT_INV_FRAG)
			op = NFT_CMP_EQ;

		add_cmp_u16(r, 0, op);
	}

	add_compat(r, cs->fw.ip.proto, cs->fw.ip.invflags & XT_INV_PROTO);

	for (matchp = cs->matches; matchp; matchp = matchp->next) {
		ret = add_match(h, r, matchp->match->m);
		if (ret < 0)
			return ret;
	}

	/* Counters need to me added before the target, otherwise they are
	 * increased for each rule because of the way nf_tables works.
	 */
	if (add_counters(r, cs->counters.pcnt, cs->counters.bcnt) < 0)
		return -1;

	return add_action(r, cs, !!(cs->fw.ip.flags & IPT_F_GOTO));
}

static bool nft_ipv4_is_same(const void *data_a,
			     const void *data_b)
{
	const struct iptables_command_state *a = data_a;
	const struct iptables_command_state *b = data_b;

	if (a->fw.ip.src.s_addr != b->fw.ip.src.s_addr
	    || a->fw.ip.dst.s_addr != b->fw.ip.dst.s_addr
	    || a->fw.ip.smsk.s_addr != b->fw.ip.smsk.s_addr
	    || a->fw.ip.dmsk.s_addr != b->fw.ip.dmsk.s_addr
	    || a->fw.ip.proto != b->fw.ip.proto
	    || a->fw.ip.flags != b->fw.ip.flags
	    || a->fw.ip.invflags != b->fw.ip.invflags) {
		DEBUGP("different src/dst/proto/flags/invflags\n");
		return false;
	}

	return is_same_interfaces(a->fw.ip.iniface, a->fw.ip.outiface,
				  a->fw.ip.iniface_mask, a->fw.ip.outiface_mask,
				  b->fw.ip.iniface, b->fw.ip.outiface,
				  b->fw.ip.iniface_mask, b->fw.ip.outiface_mask);
}

static void get_frag(struct nft_xt_ctx *ctx, struct nftnl_expr *e, bool *inv)
{
	uint8_t op;

	/* we assume correct mask and xor */
	if (!(ctx->flags & NFT_XT_CTX_BITWISE))
		return;

	/* we assume correct data */
	op = nftnl_expr_get_u32(e, NFTNL_EXPR_CMP_OP);
	if (op == NFT_CMP_EQ)
		*inv = true;
	else
		*inv = false;

	ctx->flags &= ~NFT_XT_CTX_BITWISE;
}

static const char *mask_to_str(uint32_t mask)
{
	static char mask_str[sizeof("255.255.255.255")];
	uint32_t bits, hmask = ntohl(mask);
	struct in_addr mask_addr = {
		.s_addr = mask,
	};
	int i;

	if (mask == 0xFFFFFFFFU) {
		sprintf(mask_str, "32");
		return mask_str;
	}

	i    = 32;
	bits = 0xFFFFFFFEU;
	while (--i >= 0 && hmask != bits)
		bits <<= 1;
	if (i >= 0)
		sprintf(mask_str, "%u", i);
	else
		sprintf(mask_str, "%s", inet_ntoa(mask_addr));

	return mask_str;
}

static void nft_ipv4_parse_meta(struct nft_xt_ctx *ctx, struct nftnl_expr *e,
				void *data)
{
	struct iptables_command_state *cs = data;

	switch (ctx->meta.key) {
	case NFT_META_L4PROTO:
		cs->fw.ip.proto = nftnl_expr_get_u8(e, NFTNL_EXPR_CMP_DATA);
		if (nftnl_expr_get_u32(e, NFTNL_EXPR_CMP_OP) == NFT_CMP_NEQ)
			cs->fw.ip.invflags |= XT_INV_PROTO;
		return;
	default:
		break;
	}

	parse_meta(e, ctx->meta.key, cs->fw.ip.iniface, cs->fw.ip.iniface_mask,
		   cs->fw.ip.outiface, cs->fw.ip.outiface_mask,
		   &cs->fw.ip.invflags);
}

static void parse_mask_ipv4(struct nft_xt_ctx *ctx, struct in_addr *mask)
{
	mask->s_addr = ctx->bitwise.mask[0];
}

static void nft_ipv4_parse_payload(struct nft_xt_ctx *ctx,
				   struct nftnl_expr *e, void *data)
{
	struct iptables_command_state *cs = data;
	struct in_addr addr;
	uint8_t proto;
	bool inv;

	switch(ctx->payload.offset) {
	case offsetof(struct iphdr, saddr):
		get_cmp_data(e, &addr, sizeof(addr), &inv);
		cs->fw.ip.src.s_addr = addr.s_addr;
		if (ctx->flags & NFT_XT_CTX_BITWISE) {
			parse_mask_ipv4(ctx, &cs->fw.ip.smsk);
			ctx->flags &= ~NFT_XT_CTX_BITWISE;
		} else {
			cs->fw.ip.smsk.s_addr = 0xffffffff;
		}

		if (inv)
			cs->fw.ip.invflags |= IPT_INV_SRCIP;
		break;
	case offsetof(struct iphdr, daddr):
		get_cmp_data(e, &addr, sizeof(addr), &inv);
		cs->fw.ip.dst.s_addr = addr.s_addr;
		if (ctx->flags & NFT_XT_CTX_BITWISE) {
			parse_mask_ipv4(ctx, &cs->fw.ip.dmsk);
			ctx->flags &= ~NFT_XT_CTX_BITWISE;
		} else {
			cs->fw.ip.dmsk.s_addr = 0xffffffff;
		}

		if (inv)
			cs->fw.ip.invflags |= IPT_INV_DSTIP;
		break;
	case offsetof(struct iphdr, protocol):
		get_cmp_data(e, &proto, sizeof(proto), &inv);
		cs->fw.ip.proto = proto;
		if (inv)
			cs->fw.ip.invflags |= IPT_INV_PROTO;
		break;
	case offsetof(struct iphdr, frag_off):
		cs->fw.ip.flags |= IPT_F_FRAG;
		inv = false;
		get_frag(ctx, e, &inv);
		if (inv)
			cs->fw.ip.invflags |= IPT_INV_FRAG;
		break;
	default:
		DEBUGP("unknown payload offset %d\n", ctx->payload.offset);
		break;
	}
}

static void nft_ipv4_parse_immediate(const char *jumpto, bool nft_goto,
				     void *data)
{
	struct iptables_command_state *cs = data;

	cs->jumpto = jumpto;

	if (nft_goto)
		cs->fw.ip.flags |= IPT_F_GOTO;
}

static void print_fragment(unsigned int flags, unsigned int invflags,
			   unsigned int format)
{
	if (!(format & FMT_OPTIONS))
		return;

	if (format & FMT_NOTABLE)
		fputs("opt ", stdout);
	fputc(invflags & IPT_INV_FRAG ? '!' : '-', stdout);
	fputc(flags & IPT_F_FRAG ? 'f' : '-', stdout);
	fputc(' ', stdout);
}

static void nft_ipv4_print_rule(struct nft_handle *h, struct nftnl_rule *r,
				unsigned int num, unsigned int format)
{
	struct iptables_command_state cs = {};

	nft_rule_to_iptables_command_state(h, r, &cs);

	print_rule_details(&cs, cs.jumpto, cs.fw.ip.flags,
			   cs.fw.ip.invflags, cs.fw.ip.proto, num, format);
	print_fragment(cs.fw.ip.flags, cs.fw.ip.invflags, format);
	print_ifaces(cs.fw.ip.iniface, cs.fw.ip.outiface, cs.fw.ip.invflags,
		     format);
	print_ipv4_addresses(&cs.fw, format);

	if (format & FMT_NOTABLE)
		fputs("  ", stdout);

#ifdef IPT_F_GOTO
	if (cs.fw.ip.flags & IPT_F_GOTO)
		printf("[goto] ");
#endif

	print_matches_and_target(&cs, format);

	if (!(format & FMT_NONEWLINE))
		fputc('\n', stdout);

	nft_clear_iptables_command_state(&cs);
}

static void save_ipv4_addr(char letter, const struct in_addr *addr,
			   uint32_t mask, int invert)
{
	if (!mask && !invert && !addr->s_addr)
		return;

	printf("%s-%c %s/%s ", invert ? "! " : "", letter, inet_ntoa(*addr),
	       mask_to_str(mask));
}

static void nft_ipv4_save_rule(const void *data, unsigned int format)
{
	const struct iptables_command_state *cs = data;

	save_ipv4_addr('s', &cs->fw.ip.src, cs->fw.ip.smsk.s_addr,
		       cs->fw.ip.invflags & IPT_INV_SRCIP);
	save_ipv4_addr('d', &cs->fw.ip.dst, cs->fw.ip.dmsk.s_addr,
		       cs->fw.ip.invflags & IPT_INV_DSTIP);

	save_rule_details(cs, cs->fw.ip.invflags, cs->fw.ip.proto,
			  cs->fw.ip.iniface, cs->fw.ip.iniface_mask,
			  cs->fw.ip.outiface, cs->fw.ip.outiface_mask);

	if (cs->fw.ip.flags & IPT_F_FRAG) {
		if (cs->fw.ip.invflags & IPT_INV_FRAG)
			printf("! ");
		printf("-f ");
	}

	save_matches_and_target(cs, cs->fw.ip.flags & IPT_F_GOTO,
				&cs->fw, format);
}

static void nft_ipv4_proto_parse(struct iptables_command_state *cs,
				 struct xtables_args *args)
{
	cs->fw.ip.proto = args->proto;
	cs->fw.ip.invflags = args->invflags;
}

static void nft_ipv4_post_parse(int command,
				struct iptables_command_state *cs,
				struct xtables_args *args)
{
	cs->fw.ip.flags = args->flags;
	/* We already set invflags in proto_parse, but we need to refresh it
	 * to include new parsed options.
	 */
	cs->fw.ip.invflags = args->invflags;

	strncpy(cs->fw.ip.iniface, args->iniface, IFNAMSIZ);
	memcpy(cs->fw.ip.iniface_mask,
	       args->iniface_mask, IFNAMSIZ*sizeof(unsigned char));

	strncpy(cs->fw.ip.outiface, args->outiface, IFNAMSIZ);
	memcpy(cs->fw.ip.outiface_mask,
	       args->outiface_mask, IFNAMSIZ*sizeof(unsigned char));

	if (args->goto_set)
		cs->fw.ip.flags |= IPT_F_GOTO;

	cs->counters.pcnt = args->pcnt_cnt;
	cs->counters.bcnt = args->bcnt_cnt;

	if (command & (CMD_REPLACE | CMD_INSERT |
			CMD_DELETE | CMD_APPEND | CMD_CHECK)) {
		if (!(cs->options & OPT_DESTINATION))
			args->dhostnetworkmask = "0.0.0.0/0";
		if (!(cs->options & OPT_SOURCE))
			args->shostnetworkmask = "0.0.0.0/0";
	}

	if (args->shostnetworkmask)
		xtables_ipparse_multiple(args->shostnetworkmask,
					 &args->s.addr.v4, &args->s.mask.v4,
					 &args->s.naddrs);
	if (args->dhostnetworkmask)
		xtables_ipparse_multiple(args->dhostnetworkmask,
					 &args->d.addr.v4, &args->d.mask.v4,
					 &args->d.naddrs);

	if ((args->s.naddrs > 1 || args->d.naddrs > 1) &&
	    (cs->fw.ip.invflags & (IPT_INV_SRCIP | IPT_INV_DSTIP)))
		xtables_error(PARAMETER_PROBLEM,
			      "! not allowed with multiple"
			      " source or destination IP addresses");
}

static int nft_ipv4_xlate(const void *data, struct xt_xlate *xl)
{
	const struct iptables_command_state *cs = data;
	const char *comment;
	int ret;

	xlate_ifname(xl, "iifname", cs->fw.ip.iniface,
		     cs->fw.ip.invflags & IPT_INV_VIA_IN);
	xlate_ifname(xl, "oifname", cs->fw.ip.outiface,
		     cs->fw.ip.invflags & IPT_INV_VIA_OUT);

	if (cs->fw.ip.flags & IPT_F_FRAG) {
		xt_xlate_add(xl, "ip frag-off & 0x1fff %s%x ",
			   cs->fw.ip.invflags & IPT_INV_FRAG? "" : "!= ", 0);
	}

	if (cs->fw.ip.proto != 0) {
		const struct protoent *pent =
			getprotobynumber(cs->fw.ip.proto);
		char protonum[sizeof("65535")];
		const char *name = protonum;

		snprintf(protonum, sizeof(protonum), "%u",
			 cs->fw.ip.proto);

		if (!pent || !xlate_find_match(cs, pent->p_name)) {
			if (pent)
				name = pent->p_name;
			xt_xlate_add(xl, "ip protocol %s%s ",
				   cs->fw.ip.invflags & IPT_INV_PROTO ?
					"!= " : "", name);
		}
	}

	if (cs->fw.ip.src.s_addr != 0) {
		xt_xlate_add(xl, "ip saddr %s%s%s ",
			   cs->fw.ip.invflags & IPT_INV_SRCIP ? "!= " : "",
			   inet_ntoa(cs->fw.ip.src),
			   xtables_ipmask_to_numeric(&cs->fw.ip.smsk));
	}
	if (cs->fw.ip.dst.s_addr != 0) {
		xt_xlate_add(xl, "ip daddr %s%s%s ",
			   cs->fw.ip.invflags & IPT_INV_DSTIP ? "!= " : "",
			   inet_ntoa(cs->fw.ip.dst),
			   xtables_ipmask_to_numeric(&cs->fw.ip.dmsk));
	}

	ret = xlate_matches(cs, xl);
	if (!ret)
		return ret;

	/* Always add counters per rule, as in iptables */
	xt_xlate_add(xl, "counter");
	ret = xlate_action(cs, !!(cs->fw.ip.flags & IPT_F_GOTO), xl);

	comment = xt_xlate_get_comment(xl);
	if (comment)
		xt_xlate_add(xl, " comment %s", comment);

	return ret;
}

struct nft_family_ops nft_family_ops_ipv4 = {
	.add			= nft_ipv4_add,
	.is_same		= nft_ipv4_is_same,
	.parse_meta		= nft_ipv4_parse_meta,
	.parse_payload		= nft_ipv4_parse_payload,
	.parse_immediate	= nft_ipv4_parse_immediate,
	.print_header		= print_header,
	.print_rule		= nft_ipv4_print_rule,
	.save_rule		= nft_ipv4_save_rule,
	.save_chain		= nft_ipv46_save_chain,
	.proto_parse		= nft_ipv4_proto_parse,
	.post_parse		= nft_ipv4_post_parse,
	.parse_target		= nft_ipv46_parse_target,
	.rule_to_cs		= nft_rule_to_iptables_command_state,
	.clear_cs		= nft_clear_iptables_command_state,
	.xlate			= nft_ipv4_xlate,
};
