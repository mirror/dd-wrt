#include <nft.h>
#include <trace.h>

#include <libnftnl/trace.h>

#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <inttypes.h>

#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nf_tables.h>
#include <linux/netfilter.h>

#include <nftables.h>
#include <mnl.h>
#include <parser.h>
#include <netlink.h>
#include <expression.h>
#include <statement.h>
#include <utils.h>

#define nft_mon_print(monh, ...) nft_print(&monh->ctx->nft->output, __VA_ARGS__)

static void trace_print_hdr(const struct nftnl_trace *nlt,
			    struct output_ctx *octx)
{
	nft_print(octx, "trace id %08x %s ",
		  nftnl_trace_get_u32(nlt, NFTNL_TRACE_ID),
		  family2str(nftnl_trace_get_u32(nlt, NFTNL_TRACE_FAMILY)));
	if (nftnl_trace_is_set(nlt, NFTNL_TRACE_TABLE))
		nft_print(octx, "%s ",
			  nftnl_trace_get_str(nlt, NFTNL_TRACE_TABLE));
	if (nftnl_trace_is_set(nlt, NFTNL_TRACE_CHAIN))
		nft_print(octx, "%s ",
			  nftnl_trace_get_str(nlt, NFTNL_TRACE_CHAIN));
}

static void trace_print_expr(const struct nftnl_trace *nlt, unsigned int attr,
			     struct expr *lhs, struct output_ctx *octx)
{
	struct expr *rhs, *rel;
	const void *data;
	uint32_t len;

	data = nftnl_trace_get_data(nlt, attr, &len);
	rhs  = constant_expr_alloc(&netlink_location,
				   lhs->dtype, lhs->byteorder,
				   len * BITS_PER_BYTE, data);
	rel  = relational_expr_alloc(&netlink_location, OP_EQ, lhs, rhs);

	expr_print(rel, octx);
	nft_print(octx, " ");
	expr_free(rel);
}

static void trace_print_verdict(const struct nftnl_trace *nlt,
				 struct output_ctx *octx)
{
	struct expr *chain_expr = NULL;
	const char *chain = NULL;
	unsigned int verdict;
	struct expr *expr;

	verdict = nftnl_trace_get_u32(nlt, NFTNL_TRACE_VERDICT);
	if (nftnl_trace_is_set(nlt, NFTNL_TRACE_JUMP_TARGET)) {
		chain = xstrdup(nftnl_trace_get_str(nlt, NFTNL_TRACE_JUMP_TARGET));
		chain_expr = constant_expr_alloc(&netlink_location,
						 &string_type,
						 BYTEORDER_HOST_ENDIAN,
						 strlen(chain) * BITS_PER_BYTE,
						 chain);
	}
	expr = verdict_expr_alloc(&netlink_location, verdict, chain_expr);

	nft_print(octx, "verdict ");
	expr_print(expr, octx);
	expr_free(expr);
}

static void trace_print_policy(const struct nftnl_trace *nlt,
			       struct output_ctx *octx)
{
	unsigned int policy;
	struct expr *expr;

	policy = nftnl_trace_get_u32(nlt, NFTNL_TRACE_POLICY);

	expr = verdict_expr_alloc(&netlink_location, policy, NULL);

	nft_print(octx, "policy ");
	expr_print(expr, octx);
	expr_free(expr);
}

static struct rule *trace_lookup_rule(const struct nftnl_trace *nlt,
				      uint64_t rule_handle,
				      struct nft_cache *cache)
{
	struct chain *chain;
	struct table *table;
	struct handle h;

	h.family = nftnl_trace_get_u32(nlt, NFTNL_TRACE_FAMILY);
	h.table.name = nftnl_trace_get_str(nlt, NFTNL_TRACE_TABLE);
	h.chain.name = nftnl_trace_get_str(nlt, NFTNL_TRACE_CHAIN);

	if (!h.table.name)
		return NULL;

	table = table_cache_find(&cache->table_cache, h.table.name, h.family);
	if (!table)
		return NULL;

	chain = chain_cache_find(table, h.chain.name);
	if (!chain)
		return NULL;

	return rule_lookup(chain, rule_handle);
}

static void trace_print_rule(const struct nftnl_trace *nlt,
			      struct output_ctx *octx, struct nft_cache *cache)
{
	uint64_t rule_handle;
	struct rule *rule;

	rule_handle = nftnl_trace_get_u64(nlt, NFTNL_TRACE_RULE_HANDLE);
	rule = trace_lookup_rule(nlt, rule_handle, cache);

	trace_print_hdr(nlt, octx);

	if (rule) {
		nft_print(octx, "rule ");
		rule_print(rule, octx);
	} else {
		nft_print(octx, "unknown rule handle %" PRIu64, rule_handle);
	}

	nft_print(octx, " (");
	trace_print_verdict(nlt, octx);
	nft_print(octx, ")\n");
}

static void trace_gen_stmts(struct list_head *stmts,
			    struct proto_ctx *ctx, struct payload_dep_ctx *pctx,
			    const struct nftnl_trace *nlt, unsigned int attr,
			    enum proto_bases base)
{
	struct list_head unordered = LIST_HEAD_INIT(unordered);
	struct list_head list;
	struct expr *rel, *lhs, *rhs, *tmp, *nexpr;
	struct stmt *stmt;
	const struct proto_desc *desc;
	const void *hdr;
	uint32_t hlen;
	unsigned int n;

	if (!nftnl_trace_is_set(nlt, attr))
		return;
	hdr = nftnl_trace_get_data(nlt, attr, &hlen);

	lhs = payload_expr_alloc(&netlink_location, NULL, 0);
	payload_init_raw(lhs, base, 0, hlen * BITS_PER_BYTE);
	rhs = constant_expr_alloc(&netlink_location,
				  &invalid_type, BYTEORDER_INVALID,
				  hlen * BITS_PER_BYTE, hdr);

restart:
	init_list_head(&list);
	payload_expr_expand(&list, lhs, ctx);
	expr_free(lhs);

	desc = NULL;
	list_for_each_entry_safe(lhs, nexpr, &list, list) {
		if (desc && desc != ctx->protocol[base].desc) {
			/* Chained protocols */
			lhs->payload.offset = 0;
			if (ctx->protocol[base].desc == NULL)
				break;
			goto restart;
		}

		tmp = constant_expr_splice(rhs, lhs->len);
		expr_set_type(tmp, lhs->dtype, lhs->byteorder);
		if (tmp->byteorder == BYTEORDER_HOST_ENDIAN)
			mpz_switch_byteorder(tmp->value, tmp->len / BITS_PER_BYTE);

		/* Skip unknown and filtered expressions */
		desc = lhs->payload.desc;
		if (lhs->dtype == &invalid_type ||
		    lhs->payload.tmpl == &proto_unknown_template ||
		    desc->checksum_key == payload_hdr_field(lhs) ||
		    desc->format.filter & (1 << payload_hdr_field(lhs))) {
			expr_free(lhs);
			expr_free(tmp);
			continue;
		}

		rel  = relational_expr_alloc(&lhs->location, OP_EQ, lhs, tmp);
		stmt = expr_stmt_alloc(&rel->location, rel);
		list_add_tail(&stmt->list, &unordered);

		desc = ctx->protocol[base].desc;
		relational_expr_pctx_update(ctx, rel);
	}

	expr_free(rhs);

	n = 0;
next:
	list_for_each_entry(stmt, &unordered, list) {
		enum proto_bases b = base;

		rel = stmt->expr;
		lhs = rel->left;

		/* Move statements to result list in defined order */
		desc = lhs->payload.desc;
		if (desc->format.order[n] &&
		    desc->format.order[n] != payload_hdr_field(lhs))
			continue;

		list_move_tail(&stmt->list, stmts);
		n++;

		if (payload_is_stacked(desc, rel))
			b--;

		/* Don't strip 'icmp type' from payload dump. */
		if (pctx->icmp_type == 0)
			payload_dependency_kill(pctx, lhs, ctx->family);
		if (lhs->flags & EXPR_F_PROTOCOL)
			payload_dependency_store(pctx, stmt, b);

		goto next;
	}
}

static struct expr *trace_alloc_list(const struct datatype *dtype,
				     enum byteorder byteorder,
				     unsigned int len, const void *data)
{
	struct expr *list_expr;
	unsigned int i;
	mpz_t value;
	uint32_t v;

	if (len != sizeof(v))
		return constant_expr_alloc(&netlink_location,
					   dtype, byteorder,
					   len * BITS_PER_BYTE, data);

	list_expr = list_expr_alloc(&netlink_location);

	mpz_init2(value, 32);
	mpz_import_data(value, data, byteorder, len);
	v = mpz_get_uint32(value);
	if (v == 0) {
		mpz_clear(value);
		expr_free(list_expr);
		return NULL;
	}

	for (i = 0; i < 32; i++) {
		uint32_t bitv = v & (1 << i);

		if (bitv == 0)
			continue;

		list_expr_add(list_expr,
			      constant_expr_alloc(&netlink_location,
						  dtype, byteorder,
						  len * BITS_PER_BYTE,
						  &bitv));
	}

	mpz_clear(value);
	return list_expr;
}

static void trace_print_ct_expr(const struct nftnl_trace *nlt, unsigned int attr,
				enum nft_ct_keys key, struct output_ctx *octx)
{
	struct expr *lhs, *rhs, *rel;
	const void *data;
	uint32_t len;

	data = nftnl_trace_get_data(nlt, attr, &len);
	lhs = ct_expr_alloc(&netlink_location, key, -1);

	switch (key) {
	case NFT_CT_STATUS:
		rhs = trace_alloc_list(lhs->dtype, lhs->byteorder, len, data);
		if (!rhs) {
			expr_free(lhs);
			return;
		}
		rel  = binop_expr_alloc(&netlink_location, OP_IMPLICIT, lhs, rhs);
		break;
	case NFT_CT_DIRECTION:
	case NFT_CT_STATE:
	case NFT_CT_ID:
		/* fallthrough */
	default:
		rhs  = constant_expr_alloc(&netlink_location,
					   lhs->dtype, lhs->byteorder,
					   len * BITS_PER_BYTE, data);
		rel  = relational_expr_alloc(&netlink_location, OP_IMPLICIT, lhs, rhs);
		break;
	}

	expr_print(rel, octx);
	nft_print(octx, " ");
	expr_free(rel);
}

static void trace_print_ct(const struct nftnl_trace *nlt,
			   struct output_ctx *octx)
{
	bool ct = nftnl_trace_is_set(nlt, NFTNL_TRACE_CT_STATE);

	if (!ct)
		return;

	trace_print_hdr(nlt, octx);

	nft_print(octx, "conntrack: ");

	if (nftnl_trace_is_set(nlt, NFTNL_TRACE_CT_DIRECTION))
		trace_print_ct_expr(nlt, NFTNL_TRACE_CT_DIRECTION,
				    NFT_CT_DIRECTION, octx);

	if (nftnl_trace_is_set(nlt, NFTNL_TRACE_CT_STATE))
		trace_print_ct_expr(nlt, NFTNL_TRACE_CT_STATE,
				    NFT_CT_STATE, octx);

	if (nftnl_trace_is_set(nlt, NFTNL_TRACE_CT_STATUS))
		trace_print_ct_expr(nlt, NFTNL_TRACE_CT_STATUS,
				    NFT_CT_STATUS, octx);

	if (nftnl_trace_is_set(nlt, NFTNL_TRACE_CT_ID))
		trace_print_ct_expr(nlt, NFTNL_TRACE_CT_ID,
				    NFT_CT_ID, octx);

	nft_print(octx, "\n");
}

static void trace_print_packet(const struct nftnl_trace *nlt,
			        struct output_ctx *octx)
{
	struct list_head stmts = LIST_HEAD_INIT(stmts);
	const struct proto_desc *ll_desc;
	struct payload_dep_ctx pctx = {};
	struct proto_ctx ctx;
	uint16_t dev_type;
	uint32_t nfproto;
	struct stmt *stmt, *next;

	trace_print_ct(nlt, octx);
	trace_print_hdr(nlt, octx);

	nft_print(octx, "packet: ");
	if (nftnl_trace_is_set(nlt, NFTNL_TRACE_IIF))
		trace_print_expr(nlt, NFTNL_TRACE_IIF,
				 meta_expr_alloc(&netlink_location,
						 NFT_META_IIF), octx);
	if (nftnl_trace_is_set(nlt, NFTNL_TRACE_OIF))
		trace_print_expr(nlt, NFTNL_TRACE_OIF,
				 meta_expr_alloc(&netlink_location,
						 NFT_META_OIF), octx);

	proto_ctx_init(&ctx, nftnl_trace_get_u32(nlt, NFTNL_TRACE_FAMILY), 0, false);
	ll_desc = ctx.protocol[PROTO_BASE_LL_HDR].desc;
	if ((ll_desc == &proto_inet || ll_desc  == &proto_netdev) &&
	    nftnl_trace_is_set(nlt, NFTNL_TRACE_NFPROTO)) {
		nfproto = nftnl_trace_get_u32(nlt, NFTNL_TRACE_NFPROTO);

		proto_ctx_update(&ctx, PROTO_BASE_LL_HDR, &netlink_location, NULL);
		proto_ctx_update(&ctx, PROTO_BASE_NETWORK_HDR, &netlink_location,
				 proto_find_upper(ll_desc, nfproto));
	}
	if (ctx.protocol[PROTO_BASE_LL_HDR].desc == NULL &&
	    nftnl_trace_is_set(nlt, NFTNL_TRACE_IIFTYPE)) {
		dev_type = nftnl_trace_get_u16(nlt, NFTNL_TRACE_IIFTYPE);
		proto_ctx_update(&ctx, PROTO_BASE_LL_HDR, &netlink_location,
				 proto_dev_desc(dev_type));
	}

	trace_gen_stmts(&stmts, &ctx, &pctx, nlt, NFTNL_TRACE_LL_HEADER,
			PROTO_BASE_LL_HDR);
	trace_gen_stmts(&stmts, &ctx, &pctx, nlt, NFTNL_TRACE_NETWORK_HEADER,
			PROTO_BASE_NETWORK_HDR);
	trace_gen_stmts(&stmts, &ctx, &pctx, nlt, NFTNL_TRACE_TRANSPORT_HEADER,
			PROTO_BASE_TRANSPORT_HDR);

	list_for_each_entry_safe(stmt, next, &stmts, list) {
		stmt_print(stmt, octx);
		nft_print(octx, " ");
		stmt_free(stmt);
	}
	nft_print(octx, "\n");
}

int netlink_events_trace_cb(const struct nlmsghdr *nlh, int type,
			    struct netlink_mon_handler *monh)
{
	struct nftnl_trace *nlt;

	assert(type == NFT_MSG_TRACE);

	nlt = nftnl_trace_alloc();
	if (!nlt)
		memory_allocation_error();

	if (nftnl_trace_nlmsg_parse(nlh, nlt) < 0)
		netlink_abi_error();

	if (nftnl_trace_is_set(nlt, NFTNL_TRACE_LL_HEADER) ||
	    nftnl_trace_is_set(nlt, NFTNL_TRACE_NETWORK_HEADER))
		trace_print_packet(nlt, &monh->ctx->nft->output);

	switch (nftnl_trace_get_u32(nlt, NFTNL_TRACE_TYPE)) {
	case NFT_TRACETYPE_RULE:
		if (nftnl_trace_is_set(nlt, NFTNL_TRACE_RULE_HANDLE))
			trace_print_rule(nlt, &monh->ctx->nft->output,
					 &monh->ctx->nft->cache);
		break;
	case NFT_TRACETYPE_POLICY:
		trace_print_hdr(nlt, &monh->ctx->nft->output);

		if (nftnl_trace_is_set(nlt, NFTNL_TRACE_POLICY)) {
			trace_print_policy(nlt, &monh->ctx->nft->output);
			nft_mon_print(monh, " ");
		}

		if (nftnl_trace_is_set(nlt, NFTNL_TRACE_MARK))
			trace_print_expr(nlt, NFTNL_TRACE_MARK,
					 meta_expr_alloc(&netlink_location,
							 NFT_META_MARK),
					 &monh->ctx->nft->output);
		nft_mon_print(monh, "\n");
		break;
	case NFT_TRACETYPE_RETURN:
		trace_print_hdr(nlt, &monh->ctx->nft->output);

		if (nftnl_trace_is_set(nlt, NFTNL_TRACE_VERDICT)) {
			trace_print_verdict(nlt, &monh->ctx->nft->output);
			nft_mon_print(monh, " ");
		}

		if (nftnl_trace_is_set(nlt, NFTNL_TRACE_MARK))
			trace_print_expr(nlt, NFTNL_TRACE_MARK,
					 meta_expr_alloc(&netlink_location,
							 NFT_META_MARK),
					 &monh->ctx->nft->output);
		nft_mon_print(monh, "\n");
		break;
	}

	nftnl_trace_free(nlt);
	return MNL_CB_OK;
}
