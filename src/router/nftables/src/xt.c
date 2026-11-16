/*
 * Copyright (c) 2013-2015 Pablo Neira Ayuso <pablo@netfilter.org>
 * Copyright (c) 2015 Arturo Borrero Gonzalez <arturo@debian.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 (or any
 * later) as published by the Free Software Foundation.
 */

#include <nft.h>

#include <time.h>
#include <net/if.h>
#include <getopt.h>
#include <ctype.h>	/* for isspace */
#include <statement.h>
#include <netlink.h>
#include <xt.h>
#include <erec.h>

#include <libmnl/libmnl.h>
#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nf_tables_compat.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv6/ip6_tables.h>
#include <linux/netfilter_arp/arp_tables.h>
#include <linux/netfilter_bridge/ebtables.h>

#ifdef HAVE_LIBXTABLES
#include <xtables.h>

static void *xt_entry_alloc(const struct xt_stmt *xt, uint32_t af);
#endif

void xt_stmt_xlate(const struct stmt *stmt, struct output_ctx *octx)
{
	static const char *typename[NFT_XT_MAX] = {
		[NFT_XT_MATCH]		= "match",
		[NFT_XT_TARGET]		= "target",
		[NFT_XT_WATCHER]	= "watcher",
	};
	int rc = 0;
#ifdef HAVE_LIBXTABLES
	struct xt_xlate *xl = xt_xlate_alloc(10240);
	struct xtables_target *tg;
	struct xt_entry_target *t;
	struct xtables_match *mt;
	struct xt_entry_match *m;
	size_t size;
	void *entry;

	xtables_set_nfproto(stmt->xt.family);
	entry = xt_entry_alloc(&stmt->xt, stmt->xt.family);

	switch (stmt->xt.type) {
	case NFT_XT_MATCH:
		mt = xtables_find_match(stmt->xt.name, XTF_TRY_LOAD, NULL);
		if (!mt) {
			fprintf(octx->error_fp,
				"# Warning: XT match %s not found\n",
				stmt->xt.name);
			break;
		}
		size = XT_ALIGN(sizeof(*m)) + stmt->xt.infolen;

		m = xzalloc(size);
		memcpy(&m->data, stmt->xt.info, stmt->xt.infolen);

		m->u.match_size = size;
		m->u.user.revision = stmt->xt.rev;

		if (mt->xlate) {
			struct xt_xlate_mt_params params = {
				.ip		= entry,
				.match		= m,
				.numeric        = 1,
			};

			rc = mt->xlate(xl, &params);
		}
		free(m);
		break;
	case NFT_XT_WATCHER:
	case NFT_XT_TARGET:
		tg = xtables_find_target(stmt->xt.name, XTF_TRY_LOAD);
		if (!tg) {
			fprintf(octx->error_fp,
				"# Warning: XT target %s not found\n",
				stmt->xt.name);
			break;
		}
		size = XT_ALIGN(sizeof(*t)) + stmt->xt.infolen;

		t = xzalloc(size);
		memcpy(&t->data, stmt->xt.info, stmt->xt.infolen);

		t->u.target_size = size;
		t->u.user.revision = stmt->xt.rev;

		strcpy(t->u.user.name, tg->name);

		if (tg->xlate) {
			struct xt_xlate_tg_params params = {
				.ip		= entry,
				.target		= t,
				.numeric        = 1,
			};

			rc = tg->xlate(xl, &params);
		}
		free(t);
		break;
	}

	if (rc == 1) {
		nft_print(octx, "%s", xt_xlate_get(xl));
		if (xt_xlate_get_comment(xl))
			nft_print(octx, "comment %s",
				  xt_xlate_get_comment(xl));
	}
	xt_xlate_free(xl);
	free(entry);
#endif
	if (!rc)
		nft_print(octx, "xt %s \"%s\"",
			  typename[stmt->xt.type], stmt->xt.name);
}

void xt_stmt_destroy(struct stmt *stmt)
{
	free_const(stmt->xt.name);
	free(stmt->xt.info);
}

#ifdef HAVE_LIBXTABLES
static void *xt_entry_alloc(const struct xt_stmt *xt, uint32_t af)
{
	union nft_entry {
		struct ipt_entry ipt;
		struct ip6t_entry ip6t;
		struct arpt_entry arpt;
		struct ebt_entry ebt;
	} *entry;

	entry = xmalloc(sizeof(union nft_entry));

	switch (af) {
	case NFPROTO_IPV4:
		entry->ipt.ip.proto = xt->proto;
		break;
	case NFPROTO_IPV6:
		entry->ip6t.ipv6.proto = xt->proto;
		break;
	case NFPROTO_BRIDGE:
		entry->ebt.ethproto = xt->proto;
		break;
	case NFPROTO_ARP:
		entry->arpt.arp.arhln_mask = 0xff;
		entry->arpt.arp.arhln = 6;
		break;
	default:
		break;
	}

	return entry;
}

static uint32_t xt_proto(const struct proto_ctx *pctx)
{
	const struct proto_desc *desc = NULL;

	if (pctx->family == NFPROTO_BRIDGE) {
		desc = pctx->protocol[PROTO_BASE_NETWORK_HDR].desc;
		if (desc == NULL)
			return 0;
		if (strcmp(desc->name, "ip") == 0)
			return __constant_htons(ETH_P_IP);
		if (strcmp(desc->name, "ip6") == 0)
			return __constant_htons(ETH_P_IPV6);
		return 0;
	}

	desc = pctx->protocol[PROTO_BASE_TRANSPORT_HDR].desc;
	if (desc == NULL)
		return 0;
	if (strcmp(desc->name, "tcp") == 0)
		return IPPROTO_TCP;
	else if (strcmp(desc->name, "udp") == 0)
		return IPPROTO_UDP;
	else if (strcmp(desc->name, "udplite") == 0)
		return IPPROTO_UDPLITE;
	else if (strcmp(desc->name, "sctp") == 0)
		return IPPROTO_SCTP;
	else if (strcmp(desc->name, "dccp") == 0)
		return IPPROTO_DCCP;
	else if (strcmp(desc->name, "esp") == 0)
		return IPPROTO_ESP;
	else if (strcmp(desc->name, "ah") == 0)
		return IPPROTO_AH;

	return 0;
}
#endif

/*
 * Delinearization
 */

void netlink_parse_match(struct netlink_parse_ctx *ctx,
			 const struct location *loc,
			 const struct nftnl_expr *nle)
{
	const char *mtinfo;
	struct stmt *stmt;
	uint32_t mt_len;

	mtinfo = nftnl_expr_get(nle, NFTNL_EXPR_MT_INFO, &mt_len);

	stmt = xt_stmt_alloc(loc);
	stmt->xt.name = strdup(nftnl_expr_get_str(nle, NFTNL_EXPR_MT_NAME));
	stmt->xt.type = NFT_XT_MATCH;
	stmt->xt.rev = nftnl_expr_get_u32(nle, NFTNL_EXPR_MT_REV);
	stmt->xt.family = ctx->table->handle.family;

	stmt->xt.infolen = mt_len;
	stmt->xt.info = xmalloc(mt_len);
	memcpy(stmt->xt.info, mtinfo, mt_len);

	ctx->table->has_xt_stmts = true;
	rule_stmt_append(ctx->rule, stmt);
}

void netlink_parse_target(struct netlink_parse_ctx *ctx,
			  const struct location *loc,
			  const struct nftnl_expr *nle)
{
	const void *tginfo;
	struct stmt *stmt;
	uint32_t tg_len;

	tginfo = nftnl_expr_get(nle, NFTNL_EXPR_TG_INFO, &tg_len);

	stmt = xt_stmt_alloc(loc);
	stmt->xt.name = strdup(nftnl_expr_get_str(nle, NFTNL_EXPR_TG_NAME));
	stmt->xt.type = NFT_XT_TARGET;
	stmt->xt.rev = nftnl_expr_get_u32(nle, NFTNL_EXPR_TG_REV);
	stmt->xt.family = ctx->table->handle.family;

	stmt->xt.infolen = tg_len;
	stmt->xt.info = xmalloc(tg_len);
	memcpy(stmt->xt.info, tginfo, tg_len);

	ctx->table->has_xt_stmts = true;
	rule_stmt_append(ctx->rule, stmt);
}

#ifdef HAVE_LIBXTABLES
static bool is_watcher(uint32_t family, struct stmt *stmt)
{
	if (family != NFPROTO_BRIDGE ||
	    stmt->xt.type != NFT_XT_TARGET)
		return false;

	/* this has to be hardcoded :-( */
	if (strcmp(stmt->xt.name, "log") == 0)
		return true;
	else if (strcmp(stmt->xt.name, "nflog") == 0)
		return true;

	return false;
}

void stmt_xt_postprocess(struct rule_pp_ctx *rctx, struct stmt *stmt,
			 struct rule *rule)
{
	struct dl_proto_ctx *dl = dl_proto_ctx(rctx);

	if (is_watcher(dl->pctx.family, stmt))
		stmt->xt.type = NFT_XT_WATCHER;

	stmt->xt.proto = xt_proto(&dl->pctx);
}

static int nft_xt_compatible_revision(const char *name, uint8_t rev, int opt)
{
	struct mnl_socket *nl;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	uint32_t portid, seq, type, family;
	struct nfgenmsg *nfg;
	int ret = 0;

	switch (opt) {
	case IPT_SO_GET_REVISION_MATCH:
		family = NFPROTO_IPV4;
		type = 0;
		break;
	case IPT_SO_GET_REVISION_TARGET:
		family = NFPROTO_IPV4;
		type = 1;
		break;
	case IP6T_SO_GET_REVISION_MATCH:
		family = NFPROTO_IPV6;
		type = 0;
		break;
	case IP6T_SO_GET_REVISION_TARGET:
		family = NFPROTO_IPV6;
		type = 1;
		break;
	default: /* No revision support, assume ok */
		return 1;
	}

	nlh = mnl_nlmsg_put_header(buf);
	nlh->nlmsg_type = (NFNL_SUBSYS_NFT_COMPAT << 8) | NFNL_MSG_COMPAT_GET;
	nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
	nlh->nlmsg_seq = seq = time(NULL);

	nfg = mnl_nlmsg_put_extra_header(nlh, sizeof(*nfg));
	nfg->nfgen_family = family;
	nfg->version = NFNETLINK_V0;
	nfg->res_id = 0;

	mnl_attr_put_strz(nlh, NFTA_COMPAT_NAME, name);
	mnl_attr_put_u32(nlh, NFTA_COMPAT_REV, htonl(rev));
	mnl_attr_put_u32(nlh, NFTA_COMPAT_TYPE, htonl(type));

	nl = mnl_socket_open(NETLINK_NETFILTER);
	if (nl == NULL)
		return 0;

	if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0)
		goto err;

	portid = mnl_socket_get_portid(nl);

	if (mnl_socket_sendto(nl, nlh, nlh->nlmsg_len) < 0)
		goto err;

	ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	if (ret == -1)
		goto err;

	ret = mnl_cb_run(buf, ret, seq, portid, NULL, NULL);
	if (ret == -1)
		goto err;

err:
	mnl_socket_close(nl);

	return ret < 0 ? 0 : 1;
}

static struct option original_opts[] = {
	{ },
};

static struct xtables_globals xt_nft_globals = {
	.program_name		= "nft",
	.program_version	= PACKAGE_VERSION,
	.orig_opts		= original_opts,
	.compat_rev		= nft_xt_compatible_revision,
};

void xt_init(void)
{
	static bool init_once;

	if (!init_once) {
		/* libxtables is full of global variables and cannot be used
		 * concurrently by multiple threads. Hence, it's fine that the
		 * "init_once" guard is not thread-safe either.
		 * Don't link against xtables if you want thread safety.
		 */
		init_once = true;

		/* Default to IPv4, but this changes in runtime */
		xtables_init_all(&xt_nft_globals, NFPROTO_IPV4);
	}
}
#endif
