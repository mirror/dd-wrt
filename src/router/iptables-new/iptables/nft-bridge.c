/*
 * (C) 2014 by Giuseppe Longo <giuseppelng@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/ether.h>
#include <inttypes.h>

#include <xtables.h>
#include <libiptc/libxtc.h>
#include <linux/netfilter/nf_tables.h>

#include <libnftnl/set.h>

#include "nft-shared.h"
#include "nft-bridge.h"
#include "nft-cache.h"
#include "nft.h"

void ebt_cs_clean(struct iptables_command_state *cs)
{
	struct ebt_match *m, *nm;

	xtables_rule_matches_free(&cs->matches);

	for (m = cs->match_list; m;) {
		if (!m->ismatch) {
			struct xtables_target *target = m->u.watcher;

			if (target->t) {
				free(target->t);
				target->t = NULL;
			}
			if (target == target->next)
				free(target);
		}

		nm = m->next;
		free(m);
		m = nm;
	}

	if (cs->target) {
		free(cs->target->t);
		cs->target->t = NULL;

		if (cs->target == cs->target->next) {
			free(cs->target);
			cs->target = NULL;
		}
	}
}

static void ebt_print_mac(const unsigned char *mac)
{
	int j;

	for (j = 0; j < ETH_ALEN; j++)
		printf("%02x%s", mac[j], (j==ETH_ALEN-1) ? "" : ":");
}

static bool mac_all_ones(const unsigned char *mac)
{
	static const char hlpmsk[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

	return memcmp(mac, hlpmsk, sizeof(hlpmsk)) == 0;
}

/* Put the mac address into 6 (ETH_ALEN) bytes returns 0 on success. */
static void ebt_print_mac_and_mask(const unsigned char *mac, const unsigned char *mask)
{

	if (!memcmp(mac, eb_mac_type_unicast, 6) &&
	    !memcmp(mask, eb_msk_type_unicast, 6))
		printf("Unicast");
	else if (!memcmp(mac, eb_mac_type_multicast, 6) &&
	         !memcmp(mask, eb_msk_type_multicast, 6))
		printf("Multicast");
	else if (!memcmp(mac, eb_mac_type_broadcast, 6) &&
	         !memcmp(mask, eb_msk_type_broadcast, 6))
		printf("Broadcast");
	else if (!memcmp(mac, eb_mac_type_bridge_group, 6) &&
	         !memcmp(mask, eb_msk_type_bridge_group, 6))
		printf("BGA");
	else {
		ebt_print_mac(mac);
		if (!mac_all_ones(mask)) {
			printf("/");
			ebt_print_mac(mask);
		}
	}
}

static void add_logical_iniface(struct nftnl_rule *r, char *iface, uint32_t op)
{
	int iface_len;

	iface_len = strlen(iface);

	add_meta(r, NFT_META_BRI_IIFNAME);
	if (iface[iface_len - 1] == '+')
		add_cmp_ptr(r, op, iface, iface_len - 1);
	else
		add_cmp_ptr(r, op, iface, iface_len + 1);
}

static void add_logical_outiface(struct nftnl_rule *r, char *iface, uint32_t op)
{
	int iface_len;

	iface_len = strlen(iface);

	add_meta(r, NFT_META_BRI_OIFNAME);
	if (iface[iface_len - 1] == '+')
		add_cmp_ptr(r, op, iface, iface_len - 1);
	else
		add_cmp_ptr(r, op, iface, iface_len + 1);
}

static int _add_action(struct nftnl_rule *r, struct iptables_command_state *cs)
{
	return add_action(r, cs, false);
}

static int nft_bridge_add(struct nft_handle *h,
			  struct nftnl_rule *r, void *data)
{
	struct iptables_command_state *cs = data;
	struct ebt_match *iter;
	struct ebt_entry *fw = &cs->eb;
	uint32_t op;

	if (fw->in[0] != '\0') {
		op = nft_invflags2cmp(fw->invflags, EBT_IIN);
		add_iniface(r, fw->in, op);
	}

	if (fw->out[0] != '\0') {
		op = nft_invflags2cmp(fw->invflags, EBT_IOUT);
		add_outiface(r, fw->out, op);
	}

	if (fw->logical_in[0] != '\0') {
		op = nft_invflags2cmp(fw->invflags, EBT_ILOGICALIN);
		add_logical_iniface(r, fw->logical_in, op);
	}

	if (fw->logical_out[0] != '\0') {
		op = nft_invflags2cmp(fw->invflags, EBT_ILOGICALOUT);
		add_logical_outiface(r, fw->logical_out, op);
	}

	if (fw->bitmask & EBT_ISOURCE) {
		op = nft_invflags2cmp(fw->invflags, EBT_ISOURCE);
		add_payload(r, offsetof(struct ethhdr, h_source), 6,
			    NFT_PAYLOAD_LL_HEADER);
		if (!mac_all_ones(fw->sourcemsk))
			add_bitwise(r, fw->sourcemsk, 6);
		add_cmp_ptr(r, op, fw->sourcemac, 6);
	}

	if (fw->bitmask & EBT_IDEST) {
		op = nft_invflags2cmp(fw->invflags, EBT_IDEST);
		add_payload(r, offsetof(struct ethhdr, h_dest), 6,
			    NFT_PAYLOAD_LL_HEADER);
		if (!mac_all_ones(fw->destmsk))
			add_bitwise(r, fw->destmsk, 6);
		add_cmp_ptr(r, op, fw->destmac, 6);
	}

	if ((fw->bitmask & EBT_NOPROTO) == 0) {
		op = nft_invflags2cmp(fw->invflags, EBT_IPROTO);
		add_payload(r, offsetof(struct ethhdr, h_proto), 2,
			    NFT_PAYLOAD_LL_HEADER);
		add_cmp_u16(r, fw->ethproto, op);
	}

	add_compat(r, fw->ethproto, fw->invflags & EBT_IPROTO);

	for (iter = cs->match_list; iter; iter = iter->next) {
		if (iter->ismatch) {
			if (add_match(h, r, iter->u.match->m))
				break;
		} else {
			if (add_target(r, iter->u.watcher->t))
				break;
		}
	}

	if (add_counters(r, cs->counters.pcnt, cs->counters.bcnt) < 0)
		return -1;

	return _add_action(r, cs);
}

static void nft_bridge_parse_meta(struct nft_xt_ctx *ctx,
				  struct nftnl_expr *e, void *data)
{
	struct iptables_command_state *cs = data;
	struct ebt_entry *fw = &cs->eb;
	uint8_t invflags = 0;
	char iifname[IFNAMSIZ] = {}, oifname[IFNAMSIZ] = {};

	parse_meta(e, ctx->meta.key, iifname, NULL, oifname, NULL, &invflags);

	switch (ctx->meta.key) {
	case NFT_META_BRI_IIFNAME:
		if (invflags & IPT_INV_VIA_IN)
			cs->eb.invflags |= EBT_ILOGICALIN;
		snprintf(fw->logical_in, sizeof(fw->logical_in), "%s", iifname);
		break;
	case NFT_META_IIFNAME:
		if (invflags & IPT_INV_VIA_IN)
			cs->eb.invflags |= EBT_IIN;
		snprintf(fw->in, sizeof(fw->in), "%s", iifname);
		break;
	case NFT_META_BRI_OIFNAME:
		if (invflags & IPT_INV_VIA_OUT)
			cs->eb.invflags |= EBT_ILOGICALOUT;
		snprintf(fw->logical_out, sizeof(fw->logical_out), "%s", oifname);
		break;
	case NFT_META_OIFNAME:
		if (invflags & IPT_INV_VIA_OUT)
			cs->eb.invflags |= EBT_IOUT;
		snprintf(fw->out, sizeof(fw->out), "%s", oifname);
		break;
	default:
		break;
	}
}

static void nft_bridge_parse_payload(struct nft_xt_ctx *ctx,
				     struct nftnl_expr *e, void *data)
{
	struct iptables_command_state *cs = data;
	struct ebt_entry *fw = &cs->eb;
	unsigned char addr[ETH_ALEN];
	unsigned short int ethproto;
	bool inv;
	int i;

	switch (ctx->payload.offset) {
	case offsetof(struct ethhdr, h_dest):
		get_cmp_data(e, addr, sizeof(addr), &inv);
		for (i = 0; i < ETH_ALEN; i++)
			fw->destmac[i] = addr[i];
		if (inv)
			fw->invflags |= EBT_IDEST;

		if (ctx->flags & NFT_XT_CTX_BITWISE) {
                        memcpy(fw->destmsk, ctx->bitwise.mask, ETH_ALEN);
                        ctx->flags &= ~NFT_XT_CTX_BITWISE;
                } else {
                        memset(&fw->destmsk, 0xff, ETH_ALEN);
                }
		fw->bitmask |= EBT_IDEST;
		break;
	case offsetof(struct ethhdr, h_source):
		get_cmp_data(e, addr, sizeof(addr), &inv);
		for (i = 0; i < ETH_ALEN; i++)
			fw->sourcemac[i] = addr[i];
		if (inv)
			fw->invflags |= EBT_ISOURCE;
		if (ctx->flags & NFT_XT_CTX_BITWISE) {
                        memcpy(fw->sourcemsk, ctx->bitwise.mask, ETH_ALEN);
                        ctx->flags &= ~NFT_XT_CTX_BITWISE;
                } else {
                        memset(&fw->sourcemsk, 0xff, ETH_ALEN);
                }
		fw->bitmask |= EBT_ISOURCE;
		break;
	case offsetof(struct ethhdr, h_proto):
		get_cmp_data(e, &ethproto, sizeof(ethproto), &inv);
		fw->ethproto = ethproto;
		if (inv)
			fw->invflags |= EBT_IPROTO;
		fw->bitmask &= ~EBT_NOPROTO;
		break;
	}
}

static void nft_bridge_parse_immediate(const char *jumpto, bool nft_goto,
				       void *data)
{
	struct iptables_command_state *cs = data;

	cs->jumpto = jumpto;
}

/* return 0 if saddr, 1 if daddr, -1 on error */
static int
lookup_check_ether_payload(uint32_t base, uint32_t offset, uint32_t len)
{
	if (base != 0 || len != ETH_ALEN)
		return -1;

	switch (offset) {
	case offsetof(struct ether_header, ether_dhost):
		return 1;
	case offsetof(struct ether_header, ether_shost):
		return 0;
	default:
		return -1;
	}
}

/* return 0 if saddr, 1 if daddr, -1 on error */
static int
lookup_check_iphdr_payload(uint32_t base, uint32_t offset, uint32_t len)
{
	if (base != 1 || len != 4)
		return -1;

	switch (offset) {
	case offsetof(struct iphdr, daddr):
		return 1;
	case offsetof(struct iphdr, saddr):
		return 0;
	default:
		return -1;
	}
}

/* Make sure previous payload expression(s) is/are consistent and extract if
 * matching on source or destination address and if matching on MAC and IP or
 * only MAC address. */
static int lookup_analyze_payloads(const struct nft_xt_ctx *ctx,
				   bool *dst, bool *ip)
{
	int val, val2 = -1;

	if (ctx->flags & NFT_XT_CTX_PREV_PAYLOAD) {
		val = lookup_check_ether_payload(ctx->prev_payload.base,
						 ctx->prev_payload.offset,
						 ctx->prev_payload.len);
		if (val < 0) {
			DEBUGP("unknown payload base/offset/len %d/%d/%d\n",
			       ctx->prev_payload.base, ctx->prev_payload.offset,
			       ctx->prev_payload.len);
			return -1;
		}
		if (!(ctx->flags & NFT_XT_CTX_PAYLOAD)) {
			DEBUGP("Previous but no current payload?\n");
			return -1;
		}
		val2 = lookup_check_iphdr_payload(ctx->payload.base,
						  ctx->payload.offset,
						  ctx->payload.len);
		if (val2 < 0) {
			DEBUGP("unknown payload base/offset/len %d/%d/%d\n",
			       ctx->payload.base, ctx->payload.offset,
			       ctx->payload.len);
			return -1;
		} else if (val != val2) {
			DEBUGP("mismatching payload match offsets\n");
			return -1;
		}
	} else if (ctx->flags & NFT_XT_CTX_PAYLOAD) {
		val = lookup_check_ether_payload(ctx->payload.base,
						 ctx->payload.offset,
						 ctx->payload.len);
		if (val < 0) {
			DEBUGP("unknown payload base/offset/len %d/%d/%d\n",
			       ctx->payload.base, ctx->payload.offset,
			       ctx->payload.len);
			return -1;
		}
	} else {
		DEBUGP("unknown LHS of lookup expression\n");
		return -1;
	}

	if (dst)
		*dst = (val == 1);
	if (ip)
		*ip = (val2 != -1);
	return 0;
}

static int set_elems_to_among_pairs(struct nft_among_pair *pairs,
				    const struct nftnl_set *s, int cnt)
{
	struct nftnl_set_elems_iter *iter = nftnl_set_elems_iter_create(s);
	struct nftnl_set_elem *elem;
	size_t tmpcnt = 0;
	const void *data;
	uint32_t datalen;
	int ret = -1;

	if (!iter) {
		fprintf(stderr, "BUG: set elems iter allocation failed\n");
		return ret;
	}

	while ((elem = nftnl_set_elems_iter_next(iter))) {
		data = nftnl_set_elem_get(elem, NFTNL_SET_ELEM_KEY, &datalen);
		if (!data) {
			fprintf(stderr, "BUG: set elem without key\n");
			goto err;
		}
		if (datalen > sizeof(*pairs)) {
			fprintf(stderr, "BUG: overlong set elem\n");
			goto err;
		}
		nft_among_insert_pair(pairs, &tmpcnt, data);
	}
	ret = 0;
err:
	nftnl_set_elems_iter_destroy(iter);
	return ret;
}

static struct nftnl_set *set_from_lookup_expr(struct nft_xt_ctx *ctx,
					      const struct nftnl_expr *e)
{
	const char *set_name = nftnl_expr_get_str(e, NFTNL_EXPR_LOOKUP_SET);
	uint32_t set_id = nftnl_expr_get_u32(e, NFTNL_EXPR_LOOKUP_SET_ID);
	struct nftnl_set_list *slist;
	struct nftnl_set *set;

	slist = nft_set_list_get(ctx->h, ctx->table, set_name);
	if (slist) {
		set = nftnl_set_list_lookup_byname(slist, set_name);
		if (set)
			return set;

		set = nft_set_batch_lookup_byid(ctx->h, set_id);
		if (set)
			return set;
	}

	return NULL;
}

static void nft_bridge_parse_lookup(struct nft_xt_ctx *ctx,
				    struct nftnl_expr *e, void *data)
{
	struct xtables_match *match = NULL;
	struct nft_among_data *among_data;
	bool is_dst, have_ip, inv;
	struct ebt_match *ematch;
	struct nftnl_set *s;
	size_t poff, size;
	uint32_t cnt;

	if (lookup_analyze_payloads(ctx, &is_dst, &have_ip))
		return;

	s = set_from_lookup_expr(ctx, e);
	if (!s)
		xtables_error(OTHER_PROBLEM,
			      "BUG: lookup expression references unknown set");

	cnt = nftnl_set_get_u32(s, NFTNL_SET_DESC_SIZE);

	for (ematch = ctx->cs->match_list; ematch; ematch = ematch->next) {
		if (!ematch->ismatch || strcmp(ematch->u.match->name, "among"))
			continue;

		match = ematch->u.match;
		among_data = (struct nft_among_data *)match->m->data;

		size = cnt + among_data->src.cnt + among_data->dst.cnt;
		size *= sizeof(struct nft_among_pair);

		size += XT_ALIGN(sizeof(struct xt_entry_match)) +
			sizeof(struct nft_among_data);

		match->m = xtables_realloc(match->m, size);
		break;
	}
	if (!match) {
		match = xtables_find_match("among", XTF_TRY_LOAD,
					   &ctx->cs->matches);

		size = cnt * sizeof(struct nft_among_pair);
		size += XT_ALIGN(sizeof(struct xt_entry_match)) +
			sizeof(struct nft_among_data);

		match->m = xtables_calloc(1, size);
		strcpy(match->m->u.user.name, match->name);
		match->m->u.user.revision = match->revision;
		xs_init_match(match);

		if (ctx->h->ops->parse_match != NULL)
			ctx->h->ops->parse_match(match, ctx->cs);
	}
	if (!match)
		return;

	match->m->u.match_size = size;

	inv = !!(nftnl_expr_get_u32(e, NFTNL_EXPR_LOOKUP_FLAGS) &
				    NFT_LOOKUP_F_INV);

	among_data = (struct nft_among_data *)match->m->data;
	poff = nft_among_prepare_data(among_data, is_dst, cnt, inv, have_ip);
	if (set_elems_to_among_pairs(among_data->pairs + poff, s, cnt))
		xtables_error(OTHER_PROBLEM,
			      "ebtables among pair parsing failed");

	ctx->flags &= ~(NFT_XT_CTX_PAYLOAD | NFT_XT_CTX_PREV_PAYLOAD);
}

static void parse_watcher(void *object, struct ebt_match **match_list,
			  bool ismatch)
{
	struct ebt_match *m;

	m = calloc(1, sizeof(struct ebt_match));
	if (m == NULL)
		xtables_error(OTHER_PROBLEM, "Can't allocate memory");

	if (ismatch)
		m->u.match = object;
	else
		m->u.watcher = object;

	m->ismatch = ismatch;
	if (*match_list == NULL)
		*match_list = m;
	else
		(*match_list)->next = m;
}

static void nft_bridge_parse_match(struct xtables_match *m, void *data)
{
	struct iptables_command_state *cs = data;

	parse_watcher(m, &cs->match_list, true);
}

static void nft_bridge_parse_target(struct xtables_target *t, void *data)
{
	struct iptables_command_state *cs = data;

	/* harcoded names :-( */
	if (strcmp(t->name, "log") == 0 ||
	    strcmp(t->name, "nflog") == 0) {
		parse_watcher(t, &cs->match_list, false);
		return;
	}

	cs->target = t;
}

static void nft_rule_to_ebtables_command_state(struct nft_handle *h,
					       const struct nftnl_rule *r,
					       struct iptables_command_state *cs)
{
	cs->eb.bitmask = EBT_NOPROTO;
	nft_rule_to_iptables_command_state(h, r, cs);
}

static void print_iface(const char *option, const char *name, bool invert)
{
	if (*name)
		printf("%s%s %s ", option, invert ? " !" : "", name);
}

static void nft_bridge_print_table_header(const char *tablename)
{
	printf("Bridge table: %s\n\n", tablename);
}

static void nft_bridge_print_header(unsigned int format, const char *chain,
				    const char *pol,
				    const struct xt_counters *counters,
				    bool basechain, uint32_t refs, uint32_t entries)
{
	printf("Bridge chain: %s, entries: %u, policy: %s\n",
	       chain, entries, pol ?: "RETURN");
}

static void print_matches_and_watchers(const struct iptables_command_state *cs,
				       unsigned int format)
{
	struct xtables_target *watcherp;
	struct xtables_match *matchp;
	struct ebt_match *m;

	for (m = cs->match_list; m; m = m->next) {
		if (m->ismatch) {
			matchp = m->u.match;
			if (matchp->print != NULL) {
				matchp->print(&cs->eb, matchp->m,
					      format & FMT_NUMERIC);
			}
		} else {
			watcherp = m->u.watcher;
			if (watcherp->print != NULL) {
				watcherp->print(&cs->eb, watcherp->t,
						format & FMT_NUMERIC);
			}
		}
	}
}

static void print_mac(char option, const unsigned char *mac,
		      const unsigned char *mask,
		      bool invert)
{
	printf("-%c ", option);
	if (invert)
		printf("! ");
	ebt_print_mac_and_mask(mac, mask);
	printf(" ");
}


static void print_protocol(uint16_t ethproto, bool invert, unsigned int bitmask)
{
	struct xt_ethertypeent *ent;

	/* Dont print anything about the protocol if no protocol was
	 * specified, obviously this means any protocol will do. */
	if (bitmask & EBT_NOPROTO)
		return;

	printf("-p ");
	if (invert)
		printf("! ");

	if (bitmask & EBT_802_3) {
		printf("length ");
		return;
	}

	ent = xtables_getethertypebynumber(ntohs(ethproto));
	if (!ent)
		printf("0x%x ", ntohs(ethproto));
	else
		printf("%s ", ent->e_name);
}

static void nft_bridge_save_rule(const void *data, unsigned int format)
{
	const struct iptables_command_state *cs = data;

	if (cs->eb.ethproto)
		print_protocol(cs->eb.ethproto, cs->eb.invflags & EBT_IPROTO,
			       cs->eb.bitmask);
	if (cs->eb.bitmask & EBT_ISOURCE)
		print_mac('s', cs->eb.sourcemac, cs->eb.sourcemsk,
		          cs->eb.invflags & EBT_ISOURCE);
	if (cs->eb.bitmask & EBT_IDEST)
		print_mac('d', cs->eb.destmac, cs->eb.destmsk,
		          cs->eb.invflags & EBT_IDEST);

	print_iface("-i", cs->eb.in, cs->eb.invflags & EBT_IIN);
	print_iface("--logical-in", cs->eb.logical_in,
		    cs->eb.invflags & EBT_ILOGICALIN);
	print_iface("-o", cs->eb.out, cs->eb.invflags & EBT_IOUT);
	print_iface("--logical-out", cs->eb.logical_out,
		    cs->eb.invflags & EBT_ILOGICALOUT);

	print_matches_and_watchers(cs, format);

	printf("-j ");

	if (cs->jumpto != NULL) {
		if (strcmp(cs->jumpto, "") != 0)
			printf("%s", cs->jumpto);
		else
			printf("CONTINUE");
	}
	if (cs->target != NULL && cs->target->print != NULL) {
		printf(" ");
		cs->target->print(&cs->fw, cs->target->t, format & FMT_NUMERIC);
	}

	if ((format & (FMT_NOCOUNTS | FMT_C_COUNTS)) == FMT_C_COUNTS) {
		if (format & FMT_EBT_SAVE)
			printf(" -c %"PRIu64" %"PRIu64"",
			       (uint64_t)cs->counters.pcnt,
			       (uint64_t)cs->counters.bcnt);
		else
			printf(" , pcnt = %"PRIu64" -- bcnt = %"PRIu64"",
			       (uint64_t)cs->counters.pcnt,
			       (uint64_t)cs->counters.bcnt);
	}

	if (!(format & FMT_NONEWLINE))
		fputc('\n', stdout);
}

static void nft_bridge_print_rule(struct nft_handle *h, struct nftnl_rule *r,
				  unsigned int num, unsigned int format)
{
	struct iptables_command_state cs = {};

	if (format & FMT_LINENUMBERS)
		printf("%d ", num);

	nft_rule_to_ebtables_command_state(h, r, &cs);
	nft_bridge_save_rule(&cs, format);
	ebt_cs_clean(&cs);
}

static void nft_bridge_save_chain(const struct nftnl_chain *c,
				  const char *policy)
{
	const char *chain = nftnl_chain_get_str(c, NFTNL_CHAIN_NAME);

	printf(":%s %s\n", chain, policy ?: "ACCEPT");
}

static bool nft_bridge_is_same(const void *data_a, const void *data_b)
{
	const struct ebt_entry *a = data_a;
	const struct ebt_entry *b = data_b;
	int i;

	if (a->ethproto != b->ethproto ||
	    /* FIXME: a->flags != b->flags || */
	    a->invflags != b->invflags) {
		DEBUGP("different proto/flags/invflags\n");
		return false;
	}

	for (i = 0; i < ETH_ALEN; i++) {
		if (a->sourcemac[i] != b->sourcemac[i]) {
			DEBUGP("different source mac %x, %x (%d)\n",
			a->sourcemac[i] & 0xff, b->sourcemac[i] & 0xff, i);
			return false;
		}

		if (a->destmac[i] != b->destmac[i]) {
			DEBUGP("different destination mac %x, %x (%d)\n",
			a->destmac[i] & 0xff, b->destmac[i] & 0xff, i);
			return false;
		}
	}

	for (i = 0; i < IFNAMSIZ; i++) {
		if (a->logical_in[i] != b->logical_in[i]) {
			DEBUGP("different logical iniface %x, %x (%d)\n",
			a->logical_in[i] & 0xff, b->logical_in[i] & 0xff, i);
			return false;
		}

		if (a->logical_out[i] != b->logical_out[i]) {
			DEBUGP("different logical outiface %x, %x (%d)\n",
			a->logical_out[i] & 0xff, b->logical_out[i] & 0xff, i);
			return false;
		}
	}

	return strcmp(a->in, b->in) == 0 && strcmp(a->out, b->out) == 0;
}

static int xlate_ebmatches(const struct iptables_command_state *cs, struct xt_xlate *xl)
{
	int ret = 1, numeric = cs->options & OPT_NUMERIC;
	struct ebt_match *m;

	for (m = cs->match_list; m; m = m->next) {
		if (m->ismatch) {
			struct xtables_match *matchp = m->u.match;
			struct xt_xlate_mt_params mt_params = {
				.ip		= (const void *)&cs->eb,
				.numeric	= numeric,
				.escape_quotes	= false,
				.match		= matchp->m,
			};

			if (!matchp->xlate)
				return 0;

			ret = matchp->xlate(xl, &mt_params);
		} else {
			struct xtables_target *watcherp = m->u.watcher;
			struct xt_xlate_tg_params wt_params = {
				.ip		= (const void *)&cs->eb,
				.numeric	= numeric,
				.escape_quotes	= false,
				.target		= watcherp->t,
			};

			if (!watcherp->xlate)
				return 0;

			ret = watcherp->xlate(xl, &wt_params);
		}

		if (!ret)
			break;
	}

	return ret;
}

static int xlate_ebaction(const struct iptables_command_state *cs, struct xt_xlate *xl)
{
	int ret = 1, numeric = cs->options & OPT_NUMERIC;

	/* If no target at all, add nothing (default to continue) */
	if (cs->target != NULL) {
		/* Standard target? */
		if (strcmp(cs->jumpto, XTC_LABEL_ACCEPT) == 0)
			xt_xlate_add(xl, " accept");
		else if (strcmp(cs->jumpto, XTC_LABEL_DROP) == 0)
			xt_xlate_add(xl, " drop");
		else if (strcmp(cs->jumpto, XTC_LABEL_RETURN) == 0)
			xt_xlate_add(xl, " return");
		else if (cs->target->xlate) {
			xt_xlate_add(xl, " ");
			struct xt_xlate_tg_params params = {
				.ip		= (const void *)&cs->eb,
				.target		= cs->target->t,
				.numeric	= numeric,
			};
			ret = cs->target->xlate(xl, &params);
		}
		else
			return 0;
	} else if (cs->jumpto == NULL) {
	} else if (strlen(cs->jumpto) > 0)
		xt_xlate_add(xl, " jump %s", cs->jumpto);

	return ret;
}

static void xlate_mac(struct xt_xlate *xl, const unsigned char *mac)
{
	int i;

	xt_xlate_add(xl, "%02x", mac[0]);

	for (i=1; i < ETH_ALEN; i++)
		xt_xlate_add(xl, ":%02x", mac[i]);
}

static void nft_bridge_xlate_mac(struct xt_xlate *xl, const char *type, bool invert,
				 const unsigned char *mac, const unsigned char *mask)
{
	char one_msk[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

	xt_xlate_add(xl, "ether %s %s", type, invert ? "!= " : "");

	xlate_mac(xl, mac);

	if (memcmp(mask, one_msk, ETH_ALEN)) {
		int i;
		xt_xlate_add(xl, " and ");

		xlate_mac(xl, mask);

		xt_xlate_add(xl, " == %02x", mac[0] & mask[0]);
		for (i=1; i < ETH_ALEN; i++)
			xt_xlate_add(xl, ":%02x", mac[i] & mask[i]);
	}

	xt_xlate_add(xl, " ");
}

static int nft_bridge_xlate(const void *data, struct xt_xlate *xl)
{
	const struct iptables_command_state *cs = data;
	int ret;

	xlate_ifname(xl, "iifname", cs->eb.in,
		     cs->eb.invflags & EBT_IIN);
	xlate_ifname(xl, "meta ibrname", cs->eb.logical_in,
		     cs->eb.invflags & EBT_ILOGICALIN);
	xlate_ifname(xl, "oifname", cs->eb.out,
		     cs->eb.invflags & EBT_IOUT);
	xlate_ifname(xl, "meta obrname", cs->eb.logical_out,
		     cs->eb.invflags & EBT_ILOGICALOUT);

	if ((cs->eb.bitmask & EBT_NOPROTO) == 0) {
		const char *implicit = NULL;

		switch (ntohs(cs->eb.ethproto)) {
		case ETH_P_IP:
			implicit = "ip";
			break;
		case ETH_P_IPV6:
			implicit = "ip6";
			break;
		case ETH_P_8021Q:
			implicit = "vlan";
			break;
		default:
			break;
		}

		if (!implicit || !xlate_find_match(cs, implicit))
			xt_xlate_add(xl, "ether type %s0x%x ",
				     cs->eb.invflags & EBT_IPROTO ? "!= " : "",
				     ntohs(cs->eb.ethproto));
	}

	if (cs->eb.bitmask & EBT_802_3)
		return 0;

	if (cs->eb.bitmask & EBT_ISOURCE)
		nft_bridge_xlate_mac(xl, "saddr", cs->eb.invflags & EBT_ISOURCE,
				     cs->eb.sourcemac, cs->eb.sourcemsk);
	if (cs->eb.bitmask & EBT_IDEST)
		nft_bridge_xlate_mac(xl, "daddr", cs->eb.invflags & EBT_IDEST,
				     cs->eb.destmac, cs->eb.destmsk);
	ret = xlate_ebmatches(cs, xl);
	if (ret == 0)
		return ret;

	/* Always add counters per rule, as in ebtables */
	xt_xlate_add(xl, "counter");
	ret = xlate_ebaction(cs, xl);

	return ret;
}

struct nft_family_ops nft_family_ops_bridge = {
	.add			= nft_bridge_add,
	.is_same		= nft_bridge_is_same,
	.print_payload		= NULL,
	.parse_meta		= nft_bridge_parse_meta,
	.parse_payload		= nft_bridge_parse_payload,
	.parse_immediate	= nft_bridge_parse_immediate,
	.parse_lookup		= nft_bridge_parse_lookup,
	.parse_match		= nft_bridge_parse_match,
	.parse_target		= nft_bridge_parse_target,
	.print_table_header	= nft_bridge_print_table_header,
	.print_header		= nft_bridge_print_header,
	.print_rule		= nft_bridge_print_rule,
	.save_rule		= nft_bridge_save_rule,
	.save_chain		= nft_bridge_save_chain,
	.post_parse		= NULL,
	.rule_to_cs		= nft_rule_to_ebtables_command_state,
	.clear_cs		= ebt_cs_clean,
	.xlate			= nft_bridge_xlate,
};
