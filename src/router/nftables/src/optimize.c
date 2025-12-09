/*
 * Copyright (c) 2021 Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 (or any
 * later) as published by the Free Software Foundation.
 */

/* Funded through the NGI0 PET Fund established by NLnet (https://nlnet.nl)
 * with support from the European Commission's Next Generation Internet
 * programme.
 */

#include <nft.h>

#include <errno.h>
#include <inttypes.h>
#include <nftables.h>
#include <parser.h>
#include <expression.h>
#include <statement.h>
#include <utils.h>
#include <erec.h>
#include <linux/netfilter.h>

#define MAX_STMTS	32

struct optimize_ctx {
	struct stmt *stmt[MAX_STMTS];
	uint32_t num_stmts;

	struct stmt ***stmt_matrix;
	struct rule **rule;
	uint32_t num_rules;
};

static bool __expr_cmp(const struct expr *expr_a, const struct expr *expr_b)
{
	if (expr_a->etype != expr_b->etype)
		return false;
	if (expr_a->len != expr_b->len)
		return false;

	switch (expr_a->etype) {
	case EXPR_PAYLOAD:
		if (expr_a->payload.base != expr_b->payload.base)
			return false;
		if (expr_a->payload.offset != expr_b->payload.offset)
			return false;
		if (expr_a->payload.desc != expr_b->payload.desc)
			return false;
		if (expr_a->payload.inner_desc != expr_b->payload.inner_desc)
			return false;
		if (expr_a->payload.tmpl != expr_b->payload.tmpl)
			return false;
		break;
	case EXPR_EXTHDR:
		if (expr_a->exthdr.desc != expr_b->exthdr.desc)
			return false;
		if (expr_a->exthdr.tmpl != expr_b->exthdr.tmpl)
			return false;
		break;
	case EXPR_META:
		if (expr_a->meta.key != expr_b->meta.key)
			return false;
		if (expr_a->meta.base != expr_b->meta.base)
			return false;
		if (expr_a->meta.inner_desc != expr_b->meta.inner_desc)
			return false;
		break;
	case EXPR_CT:
		if (expr_a->ct.key != expr_b->ct.key)
			return false;
		if (expr_a->ct.base != expr_b->ct.base)
			return false;
		if (expr_a->ct.direction != expr_b->ct.direction)
			return false;
		if (expr_a->ct.nfproto != expr_b->ct.nfproto)
			return false;
		break;
	case EXPR_RT:
		if (expr_a->rt.key != expr_b->rt.key)
			return false;
		break;
	case EXPR_SOCKET:
		if (expr_a->socket.key != expr_b->socket.key)
			return false;
		if (expr_a->socket.level != expr_b->socket.level)
			return false;
		break;
	case EXPR_OSF:
		if (expr_a->osf.ttl != expr_b->osf.ttl)
			return false;
		if (expr_a->osf.flags != expr_b->osf.flags)
			return false;
		break;
	case EXPR_XFRM:
		if (expr_a->xfrm.key != expr_b->xfrm.key)
			return false;
		if (expr_a->xfrm.direction != expr_b->xfrm.direction)
			return false;
		break;
	case EXPR_FIB:
		if (expr_a->fib.flags != expr_b->fib.flags)
			return false;
		if (expr_a->fib.result != expr_b->fib.result)
			return false;
		break;
	case EXPR_NUMGEN:
		if (expr_a->numgen.type != expr_b->numgen.type)
			return false;
		if (expr_a->numgen.mod != expr_b->numgen.mod)
			return false;
		if (expr_a->numgen.offset != expr_b->numgen.offset)
			return false;
		break;
	case EXPR_HASH:
		if (expr_a->hash.mod != expr_b->hash.mod)
			return false;
		if (expr_a->hash.seed_set != expr_b->hash.seed_set)
			return false;
		if (expr_a->hash.seed != expr_b->hash.seed)
			return false;
		if (expr_a->hash.offset != expr_b->hash.offset)
			return false;
		if (expr_a->hash.type != expr_b->hash.type)
			return false;
		break;
	case EXPR_BINOP:
		if (!__expr_cmp(expr_a->left, expr_b->left))
			return false;

		return __expr_cmp(expr_a->right, expr_b->right);
	case EXPR_SYMBOL:
		if (expr_a->symtype != expr_b->symtype)
			return false;
		if (expr_a->symtype != SYMBOL_VALUE)
			return false;

		return !strcmp(expr_a->identifier, expr_b->identifier);
	case EXPR_VALUE:
		return !mpz_cmp(expr_a->value, expr_b->value);
	default:
		return false;
	}

	return true;
}

static bool is_bitmask(const struct expr *expr)
{
	switch (expr->etype) {
	case EXPR_BINOP:
		if (expr->op == OP_OR &&
		    !is_bitmask(expr->left))
			return false;

		return is_bitmask(expr->right);
	case EXPR_VALUE:
	case EXPR_SYMBOL:
		return true;
	default:
		break;
	}

	return false;
}

static bool stmt_expr_supported(const struct expr *expr)
{
	switch (expr->right->etype) {
	case EXPR_SYMBOL:
	case EXPR_RANGE_SYMBOL:
	case EXPR_RANGE:
	case EXPR_RANGE_VALUE:
	case EXPR_PREFIX:
	case EXPR_SET:
	case EXPR_LIST:
	case EXPR_VALUE:
		return true;
	case EXPR_BINOP:
		if (is_bitmask(expr->right))
			return true;
		break;
	default:
		break;
	}

	return false;
}

static bool expr_symbol_set(const struct expr *expr)
{
	return expr->right->etype == EXPR_SYMBOL &&
	       expr->right->symtype == SYMBOL_SET;
}

static bool __stmt_type_eq(const struct stmt *stmt_a, const struct stmt *stmt_b,
			   bool fully_compare)
{
	struct expr *expr_a, *expr_b;

	if (stmt_a->type != stmt_b->type)
		return false;

	switch (stmt_a->type) {
	case STMT_EXPRESSION:
		expr_a = stmt_a->expr;
		expr_b = stmt_b->expr;

		if (expr_a->op != expr_b->op)
			return false;
		if (expr_a->op != OP_IMPLICIT && expr_a->op != OP_EQ)
			return false;

		if (fully_compare) {
			if (!stmt_expr_supported(expr_a) ||
			    !stmt_expr_supported(expr_b))
				return false;

			if (expr_symbol_set(expr_a) ||
			    expr_symbol_set(expr_b))
				return false;
		}

		return __expr_cmp(expr_a->left, expr_b->left);
	case STMT_COUNTER:
	case STMT_NOTRACK:
		break;
	case STMT_VERDICT:
		if (!fully_compare)
			break;

		expr_a = stmt_a->expr;
		expr_b = stmt_b->expr;

		if (expr_a->etype != expr_b->etype)
			return false;

		if (expr_a->etype == EXPR_MAP &&
		    expr_b->etype == EXPR_MAP)
			return __expr_cmp(expr_a->map, expr_b->map);
		break;
	case STMT_LOG:
		if (stmt_a->log.snaplen != stmt_b->log.snaplen ||
		    stmt_a->log.group != stmt_b->log.group ||
		    stmt_a->log.qthreshold != stmt_b->log.qthreshold ||
		    stmt_a->log.level != stmt_b->log.level ||
		    stmt_a->log.logflags != stmt_b->log.logflags ||
		    stmt_a->log.flags != stmt_b->log.flags)
			return false;

		if (!!stmt_a->log.prefix ^ !!stmt_b->log.prefix)
			return false;

		if (!stmt_a->log.prefix)
			return true;

		if (strcmp(stmt_a->log.prefix, stmt_b->log.prefix))
			return false;
		break;
	case STMT_REJECT:
		if (stmt_a->reject.family != stmt_b->reject.family ||
		    stmt_a->reject.type != stmt_b->reject.type ||
		    stmt_a->reject.icmp_code != stmt_b->reject.icmp_code)
			return false;

		if (!!stmt_a->reject.expr ^ !!stmt_b->reject.expr)
			return false;

		if (!stmt_a->reject.expr)
			return true;

		if (!__expr_cmp(stmt_a->reject.expr, stmt_b->reject.expr))
			return false;
		break;
	case STMT_NAT:
		if (stmt_a->nat.type != stmt_b->nat.type ||
		    stmt_a->nat.flags != stmt_b->nat.flags ||
		    stmt_a->nat.family != stmt_b->nat.family ||
		    stmt_a->nat.type_flags != stmt_b->nat.type_flags)
			return false;

		switch (stmt_a->nat.type) {
		case NFT_NAT_SNAT:
		case NFT_NAT_DNAT:
			if ((stmt_a->nat.addr &&
			     stmt_a->nat.addr->etype != EXPR_SYMBOL &&
			     stmt_a->nat.addr->etype != EXPR_RANGE) ||
			    (stmt_b->nat.addr &&
			     stmt_b->nat.addr->etype != EXPR_SYMBOL &&
			     stmt_b->nat.addr->etype != EXPR_RANGE) ||
			    (stmt_a->nat.proto &&
			     stmt_a->nat.proto->etype != EXPR_SYMBOL &&
			     stmt_a->nat.proto->etype != EXPR_RANGE) ||
			    (stmt_b->nat.proto &&
			     stmt_b->nat.proto->etype != EXPR_SYMBOL &&
			     stmt_b->nat.proto->etype != EXPR_RANGE))
				return false;
			break;
		case NFT_NAT_MASQ:
			break;
		case NFT_NAT_REDIR:
			if ((stmt_a->nat.proto &&
			     stmt_a->nat.proto->etype != EXPR_SYMBOL &&
			     stmt_a->nat.proto->etype != EXPR_RANGE) ||
			    (stmt_b->nat.proto &&
			     stmt_b->nat.proto->etype != EXPR_SYMBOL &&
			     stmt_b->nat.proto->etype != EXPR_RANGE))
				return false;

			/* it should be possible to infer implicit redirections
			 * such as:
			 *
			 *	tcp dport 1234 redirect
			 *	tcp dport 3456 redirect to :7890
			 * merge:
			 *	redirect to tcp dport map { 1234 : 1234, 3456 : 7890 }
			 *
			 * currently not implemented.
			 */
			if (fully_compare &&
			    stmt_a->nat.type == NFT_NAT_REDIR &&
			    stmt_b->nat.type == NFT_NAT_REDIR &&
			    (!!stmt_a->nat.proto ^ !!stmt_b->nat.proto))
				return false;

			break;
		default:
			assert(0);
		}

		return true;
	default:
		/* ... Merging anything else is yet unsupported. */
		return false;
	}

	return true;
}

static bool expr_verdict_eq(const struct expr *expr_a, const struct expr *expr_b)
{
	char chain_a[NFT_CHAIN_MAXNAMELEN];
	char chain_b[NFT_CHAIN_MAXNAMELEN];

	if (expr_a->verdict != expr_b->verdict)
		return false;
	if (expr_a->chain && expr_b->chain) {
		if (expr_a->chain->etype != EXPR_VALUE ||
		    expr_a->chain->etype != expr_b->chain->etype)
			return false;
		expr_chain_export(expr_a->chain, chain_a);
		expr_chain_export(expr_b->chain, chain_b);
		if (strcmp(chain_a, chain_b))
			return false;
	} else if (expr_a->chain || expr_b->chain) {
		return false;
	}

	return true;
}

static bool stmt_verdict_eq(const struct stmt *stmt_a, const struct stmt *stmt_b)
{
	struct expr *expr_a, *expr_b;

	assert (stmt_a->type == STMT_VERDICT);

	expr_a = stmt_a->expr;
	expr_b = stmt_b->expr;
	if (expr_a->etype == EXPR_VERDICT &&
	    expr_b->etype == EXPR_VERDICT)
		return expr_verdict_eq(expr_a, expr_b);

	if (expr_a->etype == EXPR_MAP &&
	    expr_b->etype == EXPR_MAP)
		return __expr_cmp(expr_a->map, expr_b->map);

	return false;
}

static bool stmt_type_find(struct optimize_ctx *ctx, const struct stmt *stmt)
{
	bool unsupported_exists = false;
	uint32_t i;

	for (i = 0; i < ctx->num_stmts; i++) {
		if (ctx->stmt[i]->type == STMT_INVALID)
			unsupported_exists = true;

		if (__stmt_type_eq(stmt, ctx->stmt[i], false))
			return true;
	}

	switch (stmt->type) {
	case STMT_EXPRESSION:
	case STMT_VERDICT:
	case STMT_COUNTER:
	case STMT_NOTRACK:
	case STMT_LOG:
	case STMT_NAT:
	case STMT_REJECT:
		break;
	default:
		/* add unsupported statement only once to statement matrix. */
		if (unsupported_exists)
			return true;
		break;
	}

	return false;
}

static int rule_collect_stmts(struct optimize_ctx *ctx, struct rule *rule)
{
	const struct stmt_ops *ops;
	struct stmt *stmt, *clone;

	list_for_each_entry(stmt, &rule->stmts, list) {
		if (stmt_type_find(ctx, stmt))
			continue;

		/* No refcounter available in statement objects, clone it to
		 * to store in the array of selectors.
		 */
		ops = stmt_ops(stmt);
		clone = stmt_alloc(&internal_location, ops);
		switch (stmt->type) {
		case STMT_EXPRESSION:
			if (stmt->expr->op != OP_IMPLICIT &&
			    stmt->expr->op != OP_EQ) {
				clone->type = STMT_INVALID;
				break;
			}
			if (stmt->expr->left->etype == EXPR_CONCAT) {
				clone->type = STMT_INVALID;
				break;
			}
			/* fall-through */
		case STMT_VERDICT:
			clone->expr = expr_get(stmt->expr);
			break;
		case STMT_COUNTER:
		case STMT_NOTRACK:
			break;
		case STMT_LOG:
			memcpy(&clone->log, &stmt->log, sizeof(clone->log));
			if (stmt->log.prefix)
				clone->log.prefix = xstrdup(stmt->log.prefix);
			break;
		case STMT_NAT:
			if ((stmt->nat.addr &&
			     (stmt->nat.addr->etype == EXPR_MAP ||
			      stmt->nat.addr->etype == EXPR_VARIABLE)) ||
			    (stmt->nat.proto &&
			     (stmt->nat.proto->etype == EXPR_MAP ||
			      stmt->nat.proto->etype == EXPR_VARIABLE))) {
				clone->type = STMT_INVALID;
				break;
			}
			clone->nat.type = stmt->nat.type;
			clone->nat.family = stmt->nat.family;
			if (stmt->nat.addr)
				clone->nat.addr = expr_clone(stmt->nat.addr);
			if (stmt->nat.proto)
				clone->nat.proto = expr_clone(stmt->nat.proto);
			clone->nat.flags = stmt->nat.flags;
			clone->nat.type_flags = stmt->nat.type_flags;
			break;
		case STMT_REJECT:
			if (stmt->reject.expr)
				clone->reject.expr = expr_get(stmt->reject.expr);
			clone->reject.type = stmt->reject.type;
			clone->reject.icmp_code = stmt->reject.icmp_code;
			clone->reject.family = stmt->reject.family;
			break;
		default:
			clone->type = STMT_INVALID;
			break;
		}

		ctx->stmt[ctx->num_stmts++] = clone;
		if (ctx->num_stmts >= MAX_STMTS)
			return -1;
	}

	return 0;
}

static int unsupported_in_stmt_matrix(const struct optimize_ctx *ctx)
{
	uint32_t i;

	for (i = 0; i < ctx->num_stmts; i++) {
		if (ctx->stmt[i]->type == STMT_INVALID)
			return i;
	}
	/* this should not happen. */
	return -1;
}

static int cmd_stmt_find_in_stmt_matrix(struct optimize_ctx *ctx, struct stmt *stmt)
{
	uint32_t i;

	for (i = 0; i < ctx->num_stmts; i++) {
		if (__stmt_type_eq(stmt, ctx->stmt[i], false))
			return i;
	}

	return -1;
}

static struct stmt unsupported_stmt = {
	.type	= STMT_INVALID,
};

static void rule_build_stmt_matrix_stmts(struct optimize_ctx *ctx,
					 struct rule *rule, uint32_t *i)
{
	struct stmt *stmt;
	int k;

	list_for_each_entry(stmt, &rule->stmts, list) {
		k = cmd_stmt_find_in_stmt_matrix(ctx, stmt);
		if (k < 0) {
			k = unsupported_in_stmt_matrix(ctx);
			assert(k >= 0);
			ctx->stmt_matrix[*i][k] = &unsupported_stmt;
			continue;
		}
		ctx->stmt_matrix[*i][k] = stmt;
	}
	ctx->rule[(*i)++] = rule;
}

static int stmt_verdict_find(const struct optimize_ctx *ctx)
{
	uint32_t i;

	for (i = 0; i < ctx->num_stmts; i++) {
		if (ctx->stmt[i]->type != STMT_VERDICT)
			continue;

		return i;
	}

	return -1;
}

struct merge {
	/* interval of rules to be merged */
	uint32_t	rule_from;
	uint32_t	num_rules;
	/* statements to be merged (index relative to statement matrix) */
	uint32_t	stmt[MAX_STMTS];
	uint32_t	num_stmts;
	/* merge has been invalidated */
	bool		skip;
};

static void merge_expr_stmts(const struct optimize_ctx *ctx,
			     uint32_t from, uint32_t to,
			     const struct merge *merge,
			     struct stmt *stmt_a)
{
	struct expr *expr_a, *expr_b, *set, *elem;
	struct stmt *stmt_b;
	uint32_t i;

	set = set_expr_alloc(&internal_location, NULL);
	expr_set(set)->set_flags |= NFT_SET_ANONYMOUS;

	expr_a = stmt_a->expr->right;
	elem = set_elem_expr_alloc(&internal_location, expr_get(expr_a));
	set_expr_add(set, elem);

	for (i = from + 1; i <= to; i++) {
		stmt_b = ctx->stmt_matrix[i][merge->stmt[0]];
		expr_b = stmt_b->expr->right;
		elem = set_elem_expr_alloc(&internal_location, expr_get(expr_b));
		set_expr_add(set, elem);
	}

	expr_free(stmt_a->expr->right);
	stmt_a->expr->right = set;
}

static void merge_vmap(const struct optimize_ctx *ctx,
		       struct stmt *stmt_a, const struct stmt *stmt_b)
{
	struct expr *mappings, *mapping, *expr;

	mappings = stmt_b->expr->mappings;
	list_for_each_entry(expr, &expr_set(mappings)->expressions, list) {
		mapping = expr_clone(expr);
		set_expr_add(stmt_a->expr->mappings, mapping);
	}
}

static void merge_verdict_stmts(const struct optimize_ctx *ctx,
				uint32_t from, uint32_t to,
				const struct merge *merge,
				struct stmt *stmt_a)
{
	struct stmt *stmt_b;
	uint32_t i;

	for (i = from + 1; i <= to; i++) {
		stmt_b = ctx->stmt_matrix[i][merge->stmt[0]];
		switch (stmt_b->type) {
		case STMT_VERDICT:
			switch (stmt_b->expr->etype) {
			case EXPR_MAP:
				merge_vmap(ctx, stmt_a, stmt_b);
				break;
			default:
				assert(0);
			}
			break;
		default:
			assert(0);
			break;
		}
	}
}

static void merge_stmts(const struct optimize_ctx *ctx,
			uint32_t from, uint32_t to, const struct merge *merge)
{
	struct stmt *stmt_a = ctx->stmt_matrix[from][merge->stmt[0]];

	switch (stmt_a->type) {
	case STMT_EXPRESSION:
		merge_expr_stmts(ctx, from, to, merge, stmt_a);
		break;
	case STMT_VERDICT:
		merge_verdict_stmts(ctx, from, to, merge, stmt_a);
		break;
	default:
		assert(0);
	}
}

static void __merge_concat(const struct optimize_ctx *ctx, uint32_t i,
			   const struct merge *merge, struct list_head *concat_list)
{
	struct expr *concat, *next, *expr, *concat_clone, *clone;
	LIST_HEAD(pending_list);
	struct stmt *stmt_a;
	uint32_t k;

	concat = concat_expr_alloc(&internal_location);
	list_add(&concat->list, concat_list);

	for (k = 0; k < merge->num_stmts; k++) {
		list_for_each_entry_safe(concat, next, concat_list, list) {
			stmt_a = ctx->stmt_matrix[i][merge->stmt[k]];
			switch (stmt_a->expr->right->etype) {
			case EXPR_SET:
				list_for_each_entry(expr, &expr_set(stmt_a->expr->right)->expressions, list) {
					concat_clone = expr_clone(concat);
					clone = expr_clone(expr->key);
					concat_expr_add(concat_clone, clone);
					list_add_tail(&concat_clone->list, &pending_list);
				}
				list_del(&concat->list);
				expr_free(concat);
				break;
			case EXPR_SYMBOL:
			case EXPR_VALUE:
			case EXPR_PREFIX:
			case EXPR_RANGE_SYMBOL:
			case EXPR_RANGE:
			case EXPR_RANGE_VALUE:
				clone = expr_clone(stmt_a->expr->right);
				concat_expr_add(concat, clone);
				break;
			case EXPR_LIST:
				list_for_each_entry(expr, &expr_list(stmt_a->expr->right)->expressions, list) {
					concat_clone = expr_clone(concat);
					clone = expr_clone(expr);
					concat_expr_add(concat_clone, clone);
					list_add_tail(&concat_clone->list, &pending_list);
				}
				list_del(&concat->list);
				expr_free(concat);
				break;
			default:
				assert(0);
				break;
			}
		}
		list_splice_init(&pending_list, concat_list);
	}
}

static void __merge_concat_stmts(const struct optimize_ctx *ctx, uint32_t i,
				 const struct merge *merge, struct expr *set)
{
	struct expr *concat, *next, *elem;
	LIST_HEAD(concat_list);

	__merge_concat(ctx, i, merge, &concat_list);

	list_for_each_entry_safe(concat, next, &concat_list, list) {
		list_del(&concat->list);
		elem = set_elem_expr_alloc(&internal_location, concat);
		set_expr_add(set, elem);
	}
}

static void merge_concat_stmts(const struct optimize_ctx *ctx,
			       uint32_t from, uint32_t to,
			       const struct merge *merge)
{
	struct stmt *stmt, *stmt_a;
	struct expr *concat, *set;
	uint32_t i, k;

	stmt = ctx->stmt_matrix[from][merge->stmt[0]];
	/* build concatenation of selectors, eg. ifname . ip daddr . tcp dport */
	concat = concat_expr_alloc(&internal_location);

	for (k = 0; k < merge->num_stmts; k++) {
		stmt_a = ctx->stmt_matrix[from][merge->stmt[k]];
		concat_expr_add(concat, expr_get(stmt_a->expr->left));
	}
	expr_free(stmt->expr->left);
	stmt->expr->left = concat;

	/* build set data contenation, eg. { eth0 . 1.1.1.1 . 22 } */
	set = set_expr_alloc(&internal_location, NULL);
	expr_set(set)->set_flags |= NFT_SET_ANONYMOUS;

	for (i = from; i <= to; i++)
		__merge_concat_stmts(ctx, i, merge, set);

	expr_free(stmt->expr->right);
	stmt->expr->right = set;

	for (k = 1; k < merge->num_stmts; k++) {
		stmt_a = ctx->stmt_matrix[from][merge->stmt[k]];
		list_del(&stmt_a->list);
		stmt_free(stmt_a);
	}
}

static void build_verdict_map(struct expr *expr, struct stmt *verdict,
			      struct expr *set, struct stmt *counter)
{
	struct expr *item, *elem, *mapping;
	struct stmt *counter_elem;

	switch (expr->etype) {
	case EXPR_LIST:
		list_for_each_entry(item, &expr_list(expr)->expressions, list) {
			elem = set_elem_expr_alloc(&internal_location, expr_get(item));
			if (counter) {
				counter_elem = counter_stmt_alloc(&counter->location);
				list_add_tail(&counter_elem->list, &elem->stmt_list);
			}

			mapping = mapping_expr_alloc(&internal_location, elem,
						     expr_get(verdict->expr));
			set_expr_add(set, mapping);
		}
		stmt_free(counter);
		break;
	case EXPR_SET:
		list_for_each_entry(item, &expr_set(expr)->expressions, list) {
			elem = set_elem_expr_alloc(&internal_location, expr_get(item->key));
			if (counter) {
				counter_elem = counter_stmt_alloc(&counter->location);
				list_add_tail(&counter_elem->list, &elem->stmt_list);
			}

			mapping = mapping_expr_alloc(&internal_location, elem,
						     expr_get(verdict->expr));
			set_expr_add(set, mapping);
		}
		stmt_free(counter);
		break;
	case EXPR_PREFIX:
	case EXPR_RANGE_SYMBOL:
	case EXPR_RANGE:
	case EXPR_RANGE_VALUE:
	case EXPR_VALUE:
	case EXPR_SYMBOL:
	case EXPR_CONCAT:
		elem = set_elem_expr_alloc(&internal_location, expr_get(expr));
		if (counter)
			list_add_tail(&counter->list, &elem->stmt_list);

		mapping = mapping_expr_alloc(&internal_location, elem,
					     expr_get(verdict->expr));
		set_expr_add(set, mapping);
		break;
	default:
		assert(0);
		break;
	}
}

static void remove_counter(const struct optimize_ctx *ctx, uint32_t from)
{
	struct stmt *stmt;
	uint32_t i;

	/* remove counter statement */
	for (i = 0; i < ctx->num_stmts; i++) {
		stmt = ctx->stmt_matrix[from][i];
		if (!stmt)
			continue;

		if (stmt->type == STMT_COUNTER) {
			list_del(&stmt->list);
			stmt_free(stmt);
		}
	}
}

static struct stmt *zap_counter(const struct optimize_ctx *ctx, uint32_t from)
{
	struct stmt *stmt;
	uint32_t i;

	/* remove counter statement */
	for (i = 0; i < ctx->num_stmts; i++) {
		stmt = ctx->stmt_matrix[from][i];
		if (!stmt)
			continue;

		if (stmt->type == STMT_COUNTER) {
			list_del(&stmt->list);
			return stmt;
		}
	}

	return NULL;
}

static void merge_stmts_vmap(const struct optimize_ctx *ctx,
			     uint32_t from, uint32_t to,
			     const struct merge *merge)
{
	struct stmt *stmt_a = ctx->stmt_matrix[from][merge->stmt[0]];
	struct stmt *stmt_b, *verdict_a, *verdict_b, *stmt;
	struct expr *expr_a, *expr_b, *expr, *left, *set;
	struct stmt *counter;
	uint32_t i;
	int k;

	k = stmt_verdict_find(ctx);
	assert(k >= 0);

	set = set_expr_alloc(&internal_location, NULL);
	expr_set(set)->set_flags |= NFT_SET_ANONYMOUS;

	expr_a = stmt_a->expr->right;
	verdict_a = ctx->stmt_matrix[from][k];
	counter = zap_counter(ctx, from);
	build_verdict_map(expr_a, verdict_a, set, counter);

	for (i = from + 1; i <= to; i++) {
		stmt_b = ctx->stmt_matrix[i][merge->stmt[0]];
		expr_b = stmt_b->expr->right;
		verdict_b = ctx->stmt_matrix[i][k];
		counter = zap_counter(ctx, i);
		build_verdict_map(expr_b, verdict_b, set, counter);
	}

	left = expr_get(stmt_a->expr->left);
	expr = map_expr_alloc(&internal_location, left, set);
	stmt = verdict_stmt_alloc(&internal_location, expr);

	list_add(&stmt->list, &stmt_a->list);
	list_del(&stmt_a->list);
	stmt_free(stmt_a);
	list_del(&verdict_a->list);
	stmt_free(verdict_a);
}

static void __merge_concat_stmts_vmap(const struct optimize_ctx *ctx,
				      uint32_t i, const struct merge *merge,
				      struct expr *set, struct stmt *verdict)
{
	struct expr *concat, *next, *elem, *mapping;
	struct stmt *counter, *counter_elem;
	LIST_HEAD(concat_list);

	counter = zap_counter(ctx, i);
	__merge_concat(ctx, i, merge, &concat_list);

	list_for_each_entry_safe(concat, next, &concat_list, list) {
		list_del(&concat->list);
		elem = set_elem_expr_alloc(&internal_location, concat);
		if (counter) {
			counter_elem = counter_stmt_alloc(&counter->location);
			list_add_tail(&counter_elem->list, &elem->stmt_list);
		}

		mapping = mapping_expr_alloc(&internal_location, elem,
					     expr_get(verdict->expr));
		set_expr_add(set, mapping);
	}
	stmt_free(counter);
}

static void merge_concat_stmts_vmap(const struct optimize_ctx *ctx,
				    uint32_t from, uint32_t to,
				    const struct merge *merge)
{
	struct stmt *orig_stmt = ctx->stmt_matrix[from][merge->stmt[0]];
	struct stmt *stmt, *stmt_a, *verdict;
	struct expr *concat_a, *expr, *set;
	uint32_t i;
	int k;

	k = stmt_verdict_find(ctx);
	assert(k >= 0);

	/* build concatenation of selectors, eg. ifname . ip daddr . tcp dport */
	concat_a = concat_expr_alloc(&internal_location);
	for (i = 0; i < merge->num_stmts; i++) {
		stmt_a = ctx->stmt_matrix[from][merge->stmt[i]];
		concat_expr_add(concat_a, expr_get(stmt_a->expr->left));
	}

	/* build set data contenation, eg. { eth0 . 1.1.1.1 . 22 : accept } */
	set = set_expr_alloc(&internal_location, NULL);
	expr_set(set)->set_flags |= NFT_SET_ANONYMOUS;

	for (i = from; i <= to; i++) {
		verdict = ctx->stmt_matrix[i][k];
		__merge_concat_stmts_vmap(ctx, i, merge, set, verdict);
	}

	expr = map_expr_alloc(&internal_location, concat_a, set);
	stmt = verdict_stmt_alloc(&internal_location, expr);

	list_add(&stmt->list, &orig_stmt->list);
	list_del(&orig_stmt->list);
	stmt_free(orig_stmt);

	for (i = 1; i < merge->num_stmts; i++) {
		stmt_a = ctx->stmt_matrix[from][merge->stmt[i]];
		list_del(&stmt_a->list);
		stmt_free(stmt_a);
	}

	verdict = ctx->stmt_matrix[from][k];
	list_del(&verdict->list);
	stmt_free(verdict);
}

static bool stmt_verdict_cmp(const struct optimize_ctx *ctx,
			     uint32_t from, uint32_t to)
{
	struct stmt *stmt_a, *stmt_b;
	uint32_t i;
	int k;

	k = stmt_verdict_find(ctx);
	if (k < 0)
		return true;

	for (i = from; i + 1 <= to; i++) {
		stmt_a = ctx->stmt_matrix[i][k];
		stmt_b = ctx->stmt_matrix[i + 1][k];
		if (!stmt_a && !stmt_b)
			continue;
		if (!stmt_a || !stmt_b)
			return false;
		if (!stmt_verdict_eq(stmt_a, stmt_b))
			return false;
	}

	return true;
}

static int stmt_nat_type(const struct optimize_ctx *ctx, int from,
			 enum nft_nat_etypes *nat_type)
{
	uint32_t j;

	for (j = 0; j < ctx->num_stmts; j++) {
		if (!ctx->stmt_matrix[from][j])
			continue;

		if (ctx->stmt_matrix[from][j]->type == STMT_NAT) {
			*nat_type = ctx->stmt_matrix[from][j]->nat.type;
			return 0;
		}
	}

	return -1;
}

static int stmt_nat_find(const struct optimize_ctx *ctx, int from)
{
	enum nft_nat_etypes nat_type;
	uint32_t i;

	if (stmt_nat_type(ctx, from, &nat_type) < 0)
		return -1;

	for (i = 0; i < ctx->num_stmts; i++) {
		if (ctx->stmt[i]->type != STMT_NAT ||
		    ctx->stmt[i]->nat.type != nat_type)
			continue;

		return i;
	}

	return -1;
}

static struct expr *stmt_nat_expr(struct stmt *nat_stmt)
{
	struct expr *nat_expr;

	assert(nat_stmt->type == STMT_NAT);

	if (nat_stmt->nat.proto) {
		if (nat_stmt->nat.addr) {
			nat_expr = concat_expr_alloc(&internal_location);
			concat_expr_add(nat_expr, expr_get(nat_stmt->nat.addr));
			concat_expr_add(nat_expr, expr_get(nat_stmt->nat.proto));
		} else {
			nat_expr = expr_get(nat_stmt->nat.proto);
		}
		expr_free(nat_stmt->nat.proto);
		nat_stmt->nat.proto = NULL;
	} else {
		nat_expr = expr_get(nat_stmt->nat.addr);
	}

	assert(nat_expr);

	return nat_expr;
}

static void merge_nat(const struct optimize_ctx *ctx,
		      uint32_t from, uint32_t to,
		      const struct merge *merge)
{
	struct expr *expr, *set, *elem, *nat_expr, *mapping, *left;
	int k, family = NFPROTO_UNSPEC;
	struct stmt *stmt, *nat_stmt;
	uint32_t i;

	k = stmt_nat_find(ctx, from);
	assert(k >= 0);

	set = set_expr_alloc(&internal_location, NULL);
	expr_set(set)->set_flags |= NFT_SET_ANONYMOUS;

	for (i = from; i <= to; i++) {
		stmt = ctx->stmt_matrix[i][merge->stmt[0]];
		expr = stmt->expr->right;

		nat_stmt = ctx->stmt_matrix[i][k];
		nat_expr = stmt_nat_expr(nat_stmt);

		elem = set_elem_expr_alloc(&internal_location, expr_get(expr));
		mapping = mapping_expr_alloc(&internal_location, elem, nat_expr);
		set_expr_add(set, mapping);
	}

	stmt = ctx->stmt_matrix[from][merge->stmt[0]];
	left = expr_get(stmt->expr->left);
	if (left->etype == EXPR_PAYLOAD) {
		if (left->payload.desc == &proto_ip)
			family = NFPROTO_IPV4;
		else if (left->payload.desc == &proto_ip6)
			family = NFPROTO_IPV6;
	}
	expr = map_expr_alloc(&internal_location, left, set);

	nat_stmt = ctx->stmt_matrix[from][k];
	if (nat_stmt->nat.family == NFPROTO_UNSPEC)
		nat_stmt->nat.family = family;

	expr_free(nat_stmt->nat.addr);
	if (nat_stmt->nat.type == NFT_NAT_REDIR)
		nat_stmt->nat.proto = expr;
	else
		nat_stmt->nat.addr = expr;

	remove_counter(ctx, from);
	list_del(&stmt->list);
	stmt_free(stmt);
}

static void merge_concat_nat(const struct optimize_ctx *ctx,
			     uint32_t from, uint32_t to,
			     const struct merge *merge)
{
	struct expr *expr, *set, *elem, *nat_expr, *mapping, *left, *concat;
	int k, family = NFPROTO_UNSPEC;
	struct stmt *stmt, *nat_stmt;
	uint32_t i, j;

	k = stmt_nat_find(ctx, from);
	assert(k >= 0);

	set = set_expr_alloc(&internal_location, NULL);
	expr_set(set)->set_flags |= NFT_SET_ANONYMOUS;

	for (i = from; i <= to; i++) {

		concat = concat_expr_alloc(&internal_location);
		for (j = 0; j < merge->num_stmts; j++) {
			stmt = ctx->stmt_matrix[i][merge->stmt[j]];
			expr = stmt->expr->right;
			concat_expr_add(concat, expr_get(expr));
		}

		nat_stmt = ctx->stmt_matrix[i][k];
		nat_expr = stmt_nat_expr(nat_stmt);

		elem = set_elem_expr_alloc(&internal_location, concat);
		mapping = mapping_expr_alloc(&internal_location, elem, nat_expr);
		set_expr_add(set, mapping);
	}

	concat = concat_expr_alloc(&internal_location);
	for (j = 0; j < merge->num_stmts; j++) {
		stmt = ctx->stmt_matrix[from][merge->stmt[j]];
		left = stmt->expr->left;
		if (left->etype == EXPR_PAYLOAD) {
			if (left->payload.desc == &proto_ip)
				family = NFPROTO_IPV4;
			else if (left->payload.desc == &proto_ip6)
				family = NFPROTO_IPV6;
		}
		concat_expr_add(concat, expr_get(left));
	}
	expr = map_expr_alloc(&internal_location, concat, set);

	nat_stmt = ctx->stmt_matrix[from][k];
	if (nat_stmt->nat.family == NFPROTO_UNSPEC)
		nat_stmt->nat.family = family;

	expr_free(nat_stmt->nat.addr);
	nat_stmt->nat.addr = expr;

	remove_counter(ctx, from);
	for (j = 0; j < merge->num_stmts; j++) {
		stmt = ctx->stmt_matrix[from][merge->stmt[j]];
		list_del(&stmt->list);
		stmt_free(stmt);
	}
}

static void rule_optimize_print(struct output_ctx *octx,
				const struct rule *rule)
{
	const struct location *loc = &rule->location;
	const struct input_descriptor *indesc = loc->indesc;
	const char *line = "";
	char buf[1024];

	switch (indesc->type) {
	case INDESC_BUFFER:
	case INDESC_CLI:
		line = indesc->data;
		*strchrnul(line, '\n') = '\0';
		break;
	case INDESC_STDIN:
		line = indesc->data;
		line += loc->line_offset;
		*strchrnul(line, '\n') = '\0';
		break;
	case INDESC_FILE:
		line = line_location(indesc, loc, buf, sizeof(buf));
		break;
	case INDESC_INTERNAL:
	case INDESC_NETLINK:
		break;
	default:
		BUG("invalid input descriptor type %u", indesc->type);
	}

	print_location(octx->error_fp, indesc, loc);
	fprintf(octx->error_fp, "%s\n", line);
}

enum {
	MERGE_BY_VERDICT,
	MERGE_BY_NAT_MAP,
	MERGE_BY_NAT,
};

static uint32_t merge_stmt_type(const struct optimize_ctx *ctx,
				uint32_t from, uint32_t to)
{
	const struct stmt *stmt;
	uint32_t i, j;

	for (i = from; i <= to; i++) {
		for (j = 0; j < ctx->num_stmts; j++) {
			stmt = ctx->stmt_matrix[i][j];
			if (!stmt)
				continue;
			if (stmt->type == STMT_NAT) {
				if ((stmt->nat.type == NFT_NAT_REDIR &&
				     !stmt->nat.proto) ||
				    stmt->nat.type == NFT_NAT_MASQ)
					return MERGE_BY_NAT;

				return MERGE_BY_NAT_MAP;
			}
		}
	}

	/* merge by verdict, even if no verdict is specified. */
	return MERGE_BY_VERDICT;
}

static void merge_rules(const struct optimize_ctx *ctx,
			uint32_t from, uint32_t to,
			const struct merge *merge,
			struct output_ctx *octx)
{
	uint32_t merge_type;
	bool same_verdict;
	uint32_t i;

	merge_type = merge_stmt_type(ctx, from, to);

	switch (merge_type) {
	case MERGE_BY_VERDICT:
		same_verdict = stmt_verdict_cmp(ctx, from, to);
		if (merge->num_stmts > 1) {
			if (same_verdict)
				merge_concat_stmts(ctx, from, to, merge);
			else
				merge_concat_stmts_vmap(ctx, from, to, merge);
		} else {
			if (same_verdict)
				merge_stmts(ctx, from, to, merge);
			else
				merge_stmts_vmap(ctx, from, to, merge);
		}
		break;
	case MERGE_BY_NAT_MAP:
		if (merge->num_stmts > 1)
			merge_concat_nat(ctx, from, to, merge);
		else
			merge_nat(ctx, from, to, merge);
		break;
	case MERGE_BY_NAT:
		if (merge->num_stmts > 1)
			merge_concat_stmts(ctx, from, to, merge);
		else
			merge_stmts(ctx, from, to, merge);
		break;
	default:
		assert(0);
	}

	if (ctx->rule[from]->comment) {
		free_const(ctx->rule[from]->comment);
		ctx->rule[from]->comment = NULL;
	}

        octx->flags |= NFT_CTX_OUTPUT_STATELESS;

	fprintf(octx->error_fp, "Merging:\n");
	rule_optimize_print(octx, ctx->rule[from]);

	for (i = from + 1; i <= to; i++) {
		rule_optimize_print(octx, ctx->rule[i]);
		list_del(&ctx->rule[i]->list);
		rule_free(ctx->rule[i]);
	}

	fprintf(octx->error_fp, "into:\n\t");
	rule_print(ctx->rule[from], octx);
	fprintf(octx->error_fp, "\n");

        octx->flags &= ~NFT_CTX_OUTPUT_STATELESS;
}

static bool stmt_type_eq(const struct stmt *stmt_a, const struct stmt *stmt_b)
{
	if (!stmt_a && !stmt_b)
		return true;
	else if (!stmt_a)
		return false;
	else if (!stmt_b)
		return false;

	return __stmt_type_eq(stmt_a, stmt_b, true);
}

static bool stmt_is_mergeable(const struct stmt *stmt)
{
	if (!stmt)
		return false;

	switch (stmt->type) {
	case STMT_VERDICT:
		if (stmt->expr->etype == EXPR_MAP)
			return true;
		break;
	case STMT_EXPRESSION:
	case STMT_NAT:
		return true;
	default:
		break;
	}

	return false;
}

static bool rules_eq(const struct optimize_ctx *ctx, int i, int j)
{
	uint32_t k, mergeable = 0;

	for (k = 0; k < ctx->num_stmts; k++) {
		if (stmt_is_mergeable(ctx->stmt_matrix[i][k]))
			mergeable++;

		if (!stmt_type_eq(ctx->stmt_matrix[i][k], ctx->stmt_matrix[j][k]))
			return false;
	}

	if (mergeable == 0)
		return false;

	return true;
}

static int chain_optimize(struct nft_ctx *nft, struct list_head *rules)
{
	struct optimize_ctx *ctx;
	uint32_t num_merges = 0;
	struct merge *merge;
	uint32_t i, j, m, k;
	struct rule *rule;
	int ret;

	ctx = xzalloc(sizeof(*ctx));

	/* Step 1: collect statements in rules */
	list_for_each_entry(rule, rules, list) {
		ret = rule_collect_stmts(ctx, rule);
		if (ret < 0)
			goto err;

		ctx->num_rules++;
	}

	ctx->rule = xzalloc(sizeof(*ctx->rule) * ctx->num_rules);
	ctx->stmt_matrix = xzalloc(sizeof(*ctx->stmt_matrix) * ctx->num_rules);
	for (i = 0; i < ctx->num_rules; i++)
		ctx->stmt_matrix[i] = xzalloc_array(MAX_STMTS,
						    sizeof(**ctx->stmt_matrix));

	merge = xzalloc(sizeof(*merge) * ctx->num_rules);

	/* Step 2: Build matrix of statements */
	i = 0;
	list_for_each_entry(rule, rules, list)
		rule_build_stmt_matrix_stmts(ctx, rule, &i);

	/* Step 3: Look for common selectors for possible rule mergers */
	for (i = 0; i < ctx->num_rules; i++) {
		for (j = i + 1; j < ctx->num_rules; j++) {
			if (!rules_eq(ctx, i, j)) {
				if (merge[num_merges].num_rules > 0)
					num_merges++;

				i = j - 1;
				break;
			}
			if (merge[num_merges].num_rules > 0) {
				merge[num_merges].num_rules++;
			} else {
				merge[num_merges].rule_from = i;
				merge[num_merges].num_rules = 2;
			}
		}
		if (j == ctx->num_rules && merge[num_merges].num_rules > 0) {
			num_merges++;
			break;
		}
	}

	/* Step 4: Invalidate merge in case of duplicated keys in set/map. */
	for (k = 0; k < num_merges; k++) {
		uint32_t r1, r2;

		i = merge[k].rule_from;

		for (r1 = i; r1 < i + merge[k].num_rules; r1++) {
			for (r2 = r1 + 1; r2 < i + merge[k].num_rules; r2++) {
				bool match_same_value = true, match_seen = false;

				for (m = 0; m < ctx->num_stmts; m++) {
					if (!ctx->stmt_matrix[r1][m])
						continue;

					switch (ctx->stmt_matrix[r1][m]->type) {
					case STMT_EXPRESSION:
						match_seen = true;
						if (!__expr_cmp(ctx->stmt_matrix[r1][m]->expr->right,
							        ctx->stmt_matrix[r2][m]->expr->right))
							match_same_value = false;
						break;
					default:
						break;
					}
				}
				if (match_seen && match_same_value)
					merge[k].skip = true;
			}
		}
	}

	/* Step 5: Infer how to merge the candidate rules */
	for (k = 0; k < num_merges; k++) {
		if (merge[k].skip)
			continue;

		i = merge[k].rule_from;

		for (m = 0; m < ctx->num_stmts; m++) {
			if (!ctx->stmt_matrix[i][m])
				continue;
			switch (ctx->stmt_matrix[i][m]->type) {
			case STMT_EXPRESSION:
				merge[k].stmt[merge[k].num_stmts++] = m;
				break;
			case STMT_VERDICT:
				if (ctx->stmt_matrix[i][m]->expr->etype == EXPR_MAP)
					merge[k].stmt[merge[k].num_stmts++] = m;
				break;
			default:
				break;
			}
		}

		j = merge[k].num_rules - 1;
		merge_rules(ctx, i, i + j, &merge[k], &nft->output);
	}
	ret = 0;
	for (i = 0; i < ctx->num_rules; i++)
		free(ctx->stmt_matrix[i]);

	free(ctx->stmt_matrix);
	free(merge);
err:
	for (i = 0; i < ctx->num_stmts; i++)
		stmt_free(ctx->stmt[i]);

	free(ctx->rule);
	free(ctx);

	return ret;
}

static int cmd_optimize(struct nft_ctx *nft, struct cmd *cmd)
{
	struct table *table;
	struct chain *chain;
	int ret = 0;

	switch (cmd->obj) {
	case CMD_OBJ_TABLE:
		table = cmd->table;
		if (!table)
			break;

		list_for_each_entry(chain, &table->chains, list) {
			if (chain->flags & CHAIN_F_HW_OFFLOAD)
				continue;

			chain_optimize(nft, &chain->rules);
		}
		break;
	default:
		break;
	}

	return ret;
}

int nft_optimize(struct nft_ctx *nft, struct list_head *cmds)
{
	struct cmd *cmd;
	int ret = 0;

	list_for_each_entry(cmd, cmds, list) {
		switch (cmd->op) {
		case CMD_ADD:
			ret = cmd_optimize(nft, cmd);
			break;
		default:
			break;
		}
	}

	return ret;
}
