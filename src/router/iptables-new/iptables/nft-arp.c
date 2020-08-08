/*
 * (C) 2013 by Pablo Neira Ayuso <pablo@netfilter.org>
 * (C) 2013 by Giuseppe Longo <giuseppelng@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This code has been sponsored by Sophos Astaro <http://www.sophos.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <net/if_arp.h>

#include <xtables.h>
#include <libiptc/libxtc.h>
#include <net/if_arp.h>
#include <netinet/if_ether.h>

#include <linux/netfilter_arp/arp_tables.h>
#include <linux/netfilter/nf_tables.h>

#include "nft-shared.h"
#include "nft-arp.h"
#include "nft.h"

/* a few names */
char *arp_opcodes[] =
{
	"Request",
	"Reply",
	"Request_Reverse",
	"Reply_Reverse",
	"DRARP_Request",
	"DRARP_Reply",
	"DRARP_Error",
	"InARP_Request",
	"ARP_NAK",
};

static char *
addr_to_dotted(const struct in_addr *addrp)
{
	static char buf[20];
	const unsigned char *bytep;

	bytep = (const unsigned char *) &(addrp->s_addr);
	sprintf(buf, "%d.%d.%d.%d", bytep[0], bytep[1], bytep[2], bytep[3]);
	return buf;
}

static char *
addr_to_host(const struct in_addr *addr)
{
	struct hostent *host;

	if ((host = gethostbyaddr((char *) addr,
					sizeof(struct in_addr), AF_INET)) != NULL)
		return (char *) host->h_name;

	return (char *) NULL;
}

static char *
addr_to_network(const struct in_addr *addr)
{
	struct netent *net;

	if ((net = getnetbyaddr((long) ntohl(addr->s_addr), AF_INET)) != NULL)
		return (char *) net->n_name;

	return (char *) NULL;
}

static char *
addr_to_anyname(const struct in_addr *addr)
{
	char *name;

	if ((name = addr_to_host(addr)) != NULL ||
		(name = addr_to_network(addr)) != NULL)
		return name;

	return addr_to_dotted(addr);
}

static char *
mask_to_dotted(const struct in_addr *mask)
{
	int i;
	static char buf[22];
	u_int32_t maskaddr, bits;

	maskaddr = ntohl(mask->s_addr);

	if (maskaddr == 0xFFFFFFFFL)
		/* we don't want to see "/32" */
		return "";

	i = 32;
	bits = 0xFFFFFFFEL;
	while (--i >= 0 && maskaddr != bits)
		bits <<= 1;
	if (i >= 0)
		sprintf(buf, "/%d", i);
	else
		/* mask was not a decent combination of 1's and 0's */
		snprintf(buf, sizeof(buf), "/%s", addr_to_dotted(mask));

	return buf;
}

static bool need_devaddr(struct arpt_devaddr_info *info)
{
	int i;

	for (i = 0; i < ETH_ALEN; i++) {
		if (info->addr[i] || info->mask[i])
			return true;
	}

	return false;
}

static int nft_arp_add(struct nft_handle *h, struct nftnl_rule *r, void *data)
{
	struct iptables_command_state *cs = data;
	struct arpt_entry *fw = &cs->arp;
	uint32_t op;
	int ret = 0;

	if (fw->arp.iniface[0] != '\0') {
		op = nft_invflags2cmp(fw->arp.invflags, ARPT_INV_VIA_IN);
		add_iniface(r, fw->arp.iniface, op);
	}

	if (fw->arp.outiface[0] != '\0') {
		op = nft_invflags2cmp(fw->arp.invflags, ARPT_INV_VIA_OUT);
		add_outiface(r, fw->arp.outiface, op);
	}

	if (fw->arp.arhrd != 0 ||
	    fw->arp.invflags & ARPT_INV_ARPHRD) {
		op = nft_invflags2cmp(fw->arp.invflags, ARPT_INV_ARPHRD);
		add_payload(r, offsetof(struct arphdr, ar_hrd), 2,
			    NFT_PAYLOAD_NETWORK_HEADER);
		add_cmp_u16(r, fw->arp.arhrd, op);
	}

	if (fw->arp.arpro != 0 ||
	    fw->arp.invflags & ARPT_INV_ARPPRO) {
		op = nft_invflags2cmp(fw->arp.invflags, ARPT_INV_ARPPRO);
	        add_payload(r, offsetof(struct arphdr, ar_pro), 2,
			    NFT_PAYLOAD_NETWORK_HEADER);
		add_cmp_u16(r, fw->arp.arpro, op);
	}

	if (fw->arp.arhln != 0 ||
	    fw->arp.invflags & ARPT_INV_ARPHLN) {
		op = nft_invflags2cmp(fw->arp.invflags, ARPT_INV_ARPHLN);
		add_proto(r, offsetof(struct arphdr, ar_hln), 1,
			  fw->arp.arhln, op);
	}

	add_proto(r, offsetof(struct arphdr, ar_pln), 1, 4, NFT_CMP_EQ);

	if (fw->arp.arpop != 0 ||
	    fw->arp.invflags & ARPT_INV_ARPOP) {
		op = nft_invflags2cmp(fw->arp.invflags, ARPT_INV_ARPOP);
		add_payload(r, offsetof(struct arphdr, ar_op), 2,
			    NFT_PAYLOAD_NETWORK_HEADER);
		add_cmp_u16(r, fw->arp.arpop, op);
	}

	if (need_devaddr(&fw->arp.src_devaddr)) {
		op = nft_invflags2cmp(fw->arp.invflags, ARPT_INV_SRCDEVADDR);
		add_addr(r, sizeof(struct arphdr),
			 &fw->arp.src_devaddr.addr,
			 &fw->arp.src_devaddr.mask,
			 fw->arp.arhln, op);

	}

	if (fw->arp.src.s_addr != 0 ||
	    fw->arp.smsk.s_addr != 0 ||
	    fw->arp.invflags & ARPT_INV_SRCIP) {
		op = nft_invflags2cmp(fw->arp.invflags, ARPT_INV_SRCIP);
		add_addr(r, sizeof(struct arphdr) + fw->arp.arhln,
			 &fw->arp.src.s_addr, &fw->arp.smsk.s_addr,
			 sizeof(struct in_addr), op);
	}


	if (need_devaddr(&fw->arp.tgt_devaddr)) {
		op = nft_invflags2cmp(fw->arp.invflags, ARPT_INV_TGTDEVADDR);
		add_addr(r, sizeof(struct arphdr) + fw->arp.arhln + sizeof(struct in_addr),
			 &fw->arp.tgt_devaddr.addr,
			 &fw->arp.tgt_devaddr.mask,
			 fw->arp.arhln, op);
	}

	if (fw->arp.tgt.s_addr != 0 ||
	    fw->arp.tmsk.s_addr != 0 ||
	    fw->arp.invflags & ARPT_INV_TGTIP) {
		op = nft_invflags2cmp(fw->arp.invflags, ARPT_INV_TGTIP);
		add_addr(r, sizeof(struct arphdr) + fw->arp.arhln + sizeof(struct in_addr) + fw->arp.arhln,
			 &fw->arp.tgt.s_addr, &fw->arp.tmsk.s_addr,
			 sizeof(struct in_addr), op);
	}

	/* Counters need to me added before the target, otherwise they are
	 * increased for each rule because of the way nf_tables works.
	 */
	if (add_counters(r, fw->counters.pcnt, fw->counters.bcnt) < 0)
		return -1;

	if (cs->target != NULL) {
		/* Standard target? */
		if (strcmp(cs->jumpto, XTC_LABEL_ACCEPT) == 0)
			ret = add_verdict(r, NF_ACCEPT);
		else if (strcmp(cs->jumpto, XTC_LABEL_DROP) == 0)
			ret = add_verdict(r, NF_DROP);
		else if (strcmp(cs->jumpto, XTC_LABEL_RETURN) == 0)
			ret = add_verdict(r, NFT_RETURN);
		else
			ret = add_target(r, cs->target->t);
	} else if (strlen(cs->jumpto) > 0) {
		/* No goto in arptables */
		ret = add_jumpto(r, cs->jumpto, NFT_JUMP);
	}

	return ret;
}

static uint16_t ipt_to_arpt_flags(uint8_t invflags)
{
	uint16_t result = 0;

	if (invflags & IPT_INV_VIA_IN)
		result |= ARPT_INV_VIA_IN;

	if (invflags & IPT_INV_VIA_OUT)
		result |= ARPT_INV_VIA_OUT;

	if (invflags & IPT_INV_SRCIP)
		result |= ARPT_INV_SRCIP;

	if (invflags & IPT_INV_DSTIP)
		result |= ARPT_INV_TGTIP;

	if (invflags & IPT_INV_PROTO)
		result |= ARPT_INV_ARPPRO;

	return result;
}

static void nft_arp_parse_meta(struct nft_xt_ctx *ctx, struct nftnl_expr *e,
			       void *data)
{
	struct iptables_command_state *cs = data;
	struct arpt_entry *fw = &cs->arp;
	uint8_t flags = 0;

	parse_meta(e, ctx->meta.key, fw->arp.iniface, fw->arp.iniface_mask,
		   fw->arp.outiface, fw->arp.outiface_mask,
		   &flags);

	fw->arp.invflags |= ipt_to_arpt_flags(flags);
}

static void nft_arp_parse_immediate(const char *jumpto, bool nft_goto,
				    void *data)
{
	struct iptables_command_state *cs = data;

	cs->jumpto = jumpto;
}

static void parse_mask_ipv4(struct nft_xt_ctx *ctx, struct in_addr *mask)
{
	mask->s_addr = ctx->bitwise.mask[0];
}

static bool nft_arp_parse_devaddr(struct nft_xt_ctx *ctx,
				  struct nftnl_expr *e,
				  struct arpt_devaddr_info *info)
{
	uint32_t hlen;
	bool inv;

	nftnl_expr_get(e, NFTNL_EXPR_CMP_DATA, &hlen);

	if (hlen != ETH_ALEN)
		return false;

	get_cmp_data(e, info->addr, ETH_ALEN, &inv);

	if (ctx->flags & NFT_XT_CTX_BITWISE) {
		memcpy(info->mask, ctx->bitwise.mask, ETH_ALEN);
		ctx->flags &= ~NFT_XT_CTX_BITWISE;
	} else {
		memset(info->mask, 0xff, ETH_ALEN);
	}

	return inv;
}

static void nft_arp_parse_payload(struct nft_xt_ctx *ctx,
				  struct nftnl_expr *e, void *data)
{
	struct iptables_command_state *cs = data;
	struct arpt_entry *fw = &cs->arp;
	struct in_addr addr;
	uint16_t ar_hrd, ar_pro, ar_op;
	uint8_t ar_hln;
	bool inv;

	switch (ctx->payload.offset) {
	case offsetof(struct arphdr, ar_hrd):
		get_cmp_data(e, &ar_hrd, sizeof(ar_hrd), &inv);
		fw->arp.arhrd = ar_hrd;
		fw->arp.arhrd_mask = 0xffff;
		if (inv)
			fw->arp.invflags |= ARPT_INV_ARPHRD;
		break;
	case offsetof(struct arphdr, ar_pro):
		get_cmp_data(e, &ar_pro, sizeof(ar_pro), &inv);
		fw->arp.arpro = ar_pro;
		fw->arp.arpro_mask = 0xffff;
		if (inv)
			fw->arp.invflags |= ARPT_INV_ARPPRO;
		break;
	case offsetof(struct arphdr, ar_op):
		get_cmp_data(e, &ar_op, sizeof(ar_op), &inv);
		fw->arp.arpop = ar_op;
		fw->arp.arpop_mask = 0xffff;
		if (inv)
			fw->arp.invflags |= ARPT_INV_ARPOP;
		break;
	case offsetof(struct arphdr, ar_hln):
		get_cmp_data(e, &ar_hln, sizeof(ar_hln), &inv);
		fw->arp.arhln = ar_hln;
		fw->arp.arhln_mask = 0xff;
		if (inv)
			fw->arp.invflags |= ARPT_INV_ARPOP;
		break;
	default:
		if (ctx->payload.offset == sizeof(struct arphdr)) {
			if (nft_arp_parse_devaddr(ctx, e, &fw->arp.src_devaddr))
				fw->arp.invflags |= ARPT_INV_SRCDEVADDR;
		} else if (ctx->payload.offset == sizeof(struct arphdr) +
					   fw->arp.arhln) {
			get_cmp_data(e, &addr, sizeof(addr), &inv);
			fw->arp.src.s_addr = addr.s_addr;
			if (ctx->flags & NFT_XT_CTX_BITWISE) {
				parse_mask_ipv4(ctx, &fw->arp.smsk);
				ctx->flags &= ~NFT_XT_CTX_BITWISE;
			} else {
				fw->arp.smsk.s_addr = 0xffffffff;
			}

			if (inv)
				fw->arp.invflags |= ARPT_INV_SRCIP;
		} else if (ctx->payload.offset == sizeof(struct arphdr) +
						  fw->arp.arhln +
						  sizeof(struct in_addr)) {
			if (nft_arp_parse_devaddr(ctx, e, &fw->arp.tgt_devaddr))
				fw->arp.invflags |= ARPT_INV_TGTDEVADDR;
		} else if (ctx->payload.offset == sizeof(struct arphdr) +
						  fw->arp.arhln +
						  sizeof(struct in_addr) +
						  fw->arp.arhln) {
			get_cmp_data(e, &addr, sizeof(addr), &inv);
			fw->arp.tgt.s_addr = addr.s_addr;
			if (ctx->flags & NFT_XT_CTX_BITWISE) {
				parse_mask_ipv4(ctx, &fw->arp.tmsk);
				ctx->flags &= ~NFT_XT_CTX_BITWISE;
			} else {
				fw->arp.tmsk.s_addr = 0xffffffff;
			}

			if (inv)
				fw->arp.invflags |= ARPT_INV_TGTIP;
		}
		break;
	}
}

static void nft_arp_print_header(unsigned int format, const char *chain,
				 const char *pol,
				 const struct xt_counters *counters,
				 bool basechain, uint32_t refs,
				 uint32_t entries)
{
	printf("Chain %s", chain);
	if (basechain && pol) {
		printf(" (policy %s", pol);
		if (!(format & FMT_NOCOUNTS)) {
			fputc(' ', stdout);
			xtables_print_num(counters->pcnt, (format|FMT_NOTABLE));
			fputs("packets, ", stdout);
			xtables_print_num(counters->bcnt, (format|FMT_NOTABLE));
			fputs("bytes", stdout);
		}
		printf(")\n");
	} else {
		printf(" (%u references)\n", refs);
	}
}

static void nft_arp_print_rule_details(const struct iptables_command_state *cs,
				       unsigned int format)
{
	const struct arpt_entry *fw = &cs->arp;
	char buf[BUFSIZ];
	char iface[IFNAMSIZ+2];
	const char *sep = "";
	int print_iface = 0;
	int i;

	if (strlen(cs->jumpto)) {
		printf("%s-j %s", sep, cs->jumpto);
		sep = " ";
	}

	iface[0] = '\0';

	if (fw->arp.iniface[0] != '\0') {
		strcat(iface, fw->arp.iniface);
		print_iface = 1;
	}
	else if (format & FMT_VIA) {
		print_iface = 1;
		if (format & FMT_NUMERIC) strcat(iface, "*");
		else strcat(iface, "any");
	}
	if (print_iface) {
		printf("%s%s-i %s", sep, fw->arp.invflags & ARPT_INV_VIA_IN ?
				   "! " : "", iface);
		sep = " ";
	}

	print_iface = 0;
	iface[0] = '\0';

	if (fw->arp.outiface[0] != '\0') {
		strcat(iface, fw->arp.outiface);
		print_iface = 1;
	}
	else if (format & FMT_VIA) {
		print_iface = 1;
		if (format & FMT_NUMERIC) strcat(iface, "*");
		else strcat(iface, "any");
	}
	if (print_iface) {
		printf("%s%s-o %s", sep, fw->arp.invflags & ARPT_INV_VIA_OUT ?
				   "! " : "", iface);
		sep = " ";
	}

	if (fw->arp.smsk.s_addr != 0L) {
		printf("%s%s", sep, fw->arp.invflags & ARPT_INV_SRCIP
			? "! " : "");
		if (format & FMT_NUMERIC)
			sprintf(buf, "%s", addr_to_dotted(&(fw->arp.src)));
		else
			sprintf(buf, "%s", addr_to_anyname(&(fw->arp.src)));
		strncat(buf, mask_to_dotted(&(fw->arp.smsk)),
			sizeof(buf) - strlen(buf) - 1);
		printf("-s %s", buf);
		sep = " ";
	}

	for (i = 0; i < ARPT_DEV_ADDR_LEN_MAX; i++)
		if (fw->arp.src_devaddr.mask[i] != 0)
			break;
	if (i == ARPT_DEV_ADDR_LEN_MAX)
		goto after_devsrc;
	printf("%s%s", sep, fw->arp.invflags & ARPT_INV_SRCDEVADDR
		? "! " : "");
	printf("--src-mac ");
	xtables_print_mac_and_mask((unsigned char *)fw->arp.src_devaddr.addr,
				   (unsigned char *)fw->arp.src_devaddr.mask);
	sep = " ";
after_devsrc:

	if (fw->arp.tmsk.s_addr != 0L) {
		printf("%s%s", sep, fw->arp.invflags & ARPT_INV_TGTIP
			? "! " : "");
		if (format & FMT_NUMERIC)
			sprintf(buf, "%s", addr_to_dotted(&(fw->arp.tgt)));
		else
			sprintf(buf, "%s", addr_to_anyname(&(fw->arp.tgt)));
		strncat(buf, mask_to_dotted(&(fw->arp.tmsk)),
			sizeof(buf) - strlen(buf) - 1);
		printf("-d %s", buf);
		sep = " ";
	}

	for (i = 0; i <ARPT_DEV_ADDR_LEN_MAX; i++)
		if (fw->arp.tgt_devaddr.mask[i] != 0)
			break;
	if (i == ARPT_DEV_ADDR_LEN_MAX)
		goto after_devdst;
	printf("%s%s", sep, fw->arp.invflags & ARPT_INV_TGTDEVADDR
		? "! " : "");
	printf("--dst-mac ");
	xtables_print_mac_and_mask((unsigned char *)fw->arp.tgt_devaddr.addr,
				   (unsigned char *)fw->arp.tgt_devaddr.mask);
	sep = " ";

after_devdst:

	if (fw->arp.arhln_mask != 255 || fw->arp.arhln != 6) {
		printf("%s%s", sep, fw->arp.invflags & ARPT_INV_ARPHLN
			? "! " : "");
		printf("--h-length %d", fw->arp.arhln);
		if (fw->arp.arhln_mask != 255)
			printf("/%d", fw->arp.arhln_mask);
		sep = " ";
	}

	if (fw->arp.arpop_mask != 0) {
		int tmp = ntohs(fw->arp.arpop);

		printf("%s%s", sep, fw->arp.invflags & ARPT_INV_ARPOP
			? "! " : "");
		if (tmp <= NUMOPCODES && !(format & FMT_NUMERIC))
			printf("--opcode %s", arp_opcodes[tmp-1]);
		else
			printf("--opcode %d", tmp);

		if (fw->arp.arpop_mask != 65535)
			printf("/%d", ntohs(fw->arp.arpop_mask));
		sep = " ";
	}

	if (fw->arp.arhrd_mask != 65535 || fw->arp.arhrd != htons(1)) {
		uint16_t tmp = ntohs(fw->arp.arhrd);

		printf("%s%s", sep, fw->arp.invflags & ARPT_INV_ARPHRD
			? "! " : "");
		if (tmp == 1 && !(format & FMT_NUMERIC))
			printf("--h-type %s", "Ethernet");
		else
			printf("--h-type %u", tmp);
		if (fw->arp.arhrd_mask != 65535)
			printf("/%d", ntohs(fw->arp.arhrd_mask));
		sep = " ";
	}

	if (fw->arp.arpro_mask != 0) {
		int tmp = ntohs(fw->arp.arpro);

		printf("%s%s", sep, fw->arp.invflags & ARPT_INV_ARPPRO
			? "! " : "");
		if (tmp == 0x0800 && !(format & FMT_NUMERIC))
			printf("--proto-type %s", "IPv4");
		else
			printf("--proto-type 0x%x", tmp);
		if (fw->arp.arpro_mask != 65535)
			printf("/%x", ntohs(fw->arp.arpro_mask));
		sep = " ";
	}
}

static void
nft_arp_save_rule(const void *data, unsigned int format)
{
	const struct iptables_command_state *cs = data;

	format |= FMT_NUMERIC;

	nft_arp_print_rule_details(cs, format);
	if (cs->target && cs->target->save)
		cs->target->save(&cs->fw, cs->target->t);
	printf("\n");
}

static void
nft_arp_print_rule(struct nft_handle *h, struct nftnl_rule *r,
		   unsigned int num, unsigned int format)
{
	struct iptables_command_state cs = {};

	if (format & FMT_LINENUMBERS)
		printf("%u ", num);

	nft_rule_to_iptables_command_state(h, r, &cs);

	nft_arp_print_rule_details(&cs, format);
	print_matches_and_target(&cs, format);

	if (!(format & FMT_NOCOUNTS)) {
		printf(" , pcnt=");
		xtables_print_num(cs.counters.pcnt, format | FMT_NOTABLE);
		printf("-- bcnt=");
		xtables_print_num(cs.counters.bcnt, format | FMT_NOTABLE);
	}

	if (!(format & FMT_NONEWLINE))
		fputc('\n', stdout);

	nft_clear_iptables_command_state(&cs);
}

static bool nft_arp_is_same(const void *data_a,
			    const void *data_b)
{
	const struct arpt_entry *a = data_a;
	const struct arpt_entry *b = data_b;

	if (a->arp.src.s_addr != b->arp.src.s_addr
	    || a->arp.tgt.s_addr != b->arp.tgt.s_addr
	    || a->arp.smsk.s_addr != b->arp.smsk.s_addr
	    || a->arp.tmsk.s_addr != b->arp.tmsk.s_addr
	    || a->arp.arpro != b->arp.arpro
	    || a->arp.flags != b->arp.flags
	    || a->arp.invflags != b->arp.invflags) {
		DEBUGP("different src/dst/proto/flags/invflags\n");
		return false;
	}

	return is_same_interfaces(a->arp.iniface,
				  a->arp.outiface,
				  (unsigned char *)a->arp.iniface_mask,
				  (unsigned char *)a->arp.outiface_mask,
				  b->arp.iniface,
				  b->arp.outiface,
				  (unsigned char *)b->arp.iniface_mask,
				  (unsigned char *)b->arp.outiface_mask);
}

static void nft_arp_save_chain(const struct nftnl_chain *c, const char *policy)
{
	const char *chain = nftnl_chain_get_str(c, NFTNL_CHAIN_NAME);

	printf(":%s %s\n", chain, policy ?: "-");
}

struct nft_family_ops nft_family_ops_arp = {
	.add			= nft_arp_add,
	.is_same		= nft_arp_is_same,
	.print_payload		= NULL,
	.parse_meta		= nft_arp_parse_meta,
	.parse_payload		= nft_arp_parse_payload,
	.parse_immediate	= nft_arp_parse_immediate,
	.print_header		= nft_arp_print_header,
	.print_rule		= nft_arp_print_rule,
	.save_rule		= nft_arp_save_rule,
	.save_chain		= nft_arp_save_chain,
	.post_parse		= NULL,
	.rule_to_cs		= nft_rule_to_iptables_command_state,
	.clear_cs		= nft_clear_iptables_command_state,
	.parse_target		= nft_ipv46_parse_target,
};
