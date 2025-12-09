/*
 * Copyright (c) 2022 Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 (or any
 * later) as published by the Free Software Foundation.
 */

#include <nft.h>

#include <nftables.h>
#include <expression.h>
#include <intervals.h>
#include <rule.h>

static void set_to_range(struct expr *init);

static void setelem_expr_to_range(struct expr *expr)
{
	struct expr *key;
	mpz_t rop;

	assert(expr->etype == EXPR_SET_ELEM);

	switch (expr->key->etype) {
	case EXPR_SET_ELEM_CATCHALL:
	case EXPR_RANGE_VALUE:
		break;
	case EXPR_RANGE:
		key = constant_range_expr_alloc(&expr->location,
						expr->key->dtype,
						expr->key->byteorder,
						expr->key->len,
						expr->key->left->value,
						expr->key->right->value);
		expr_free(expr->key);
		expr->key = key;
		break;
	case EXPR_PREFIX:
		if (expr->key->prefix->etype != EXPR_VALUE)
			BUG("Prefix for unexpected type %d", expr->key->prefix->etype);

		mpz_init(rop);
		mpz_bitmask(rop, expr->key->len - expr->key->prefix_len);
		if (expr_basetype(expr)->type == TYPE_STRING)
			mpz_switch_byteorder(expr->key->prefix->value, expr->len / BITS_PER_BYTE);

		mpz_ior(rop, rop, expr->key->prefix->value);
		key = constant_range_expr_alloc(&expr->location,
						expr->key->dtype,
						expr->key->byteorder,
						expr->key->len,
						expr->key->prefix->value,
						rop);
		mpz_clear(rop);
		expr_free(expr->key);
		expr->key = key;
		break;
	case EXPR_VALUE:
		if (expr_basetype(expr)->type == TYPE_STRING)
			mpz_switch_byteorder(expr->key->value, expr->len / BITS_PER_BYTE);

		key = constant_range_expr_alloc(&expr->location,
						expr->key->dtype,
						expr->key->byteorder,
						expr->key->len,
						expr->key->value,
						expr->key->value);
		expr_free(expr->key);
		expr->key = key;
		break;
	default:
		BUG("unhandled key type %s", expr_name(expr->key));
	}
}

struct set_automerge_ctx {
	struct set	*set;
	struct expr	*init;
	struct expr	*purge;
	unsigned int	debug_mask;
};

static void purge_elem(struct set_automerge_ctx *ctx, struct expr *i)
{
	if (ctx->debug_mask & NFT_DEBUG_SEGTREE) {
		pr_gmp_debug("remove: [%Zx-%Zx]\n",
			     i->key->range.low,
			     i->key->range.high);
	}
	list_move_tail(&i->list, &expr_set(ctx->purge)->expressions);
}

static void remove_overlapping_range(struct set_automerge_ctx *ctx,
				     struct expr *prev, struct expr *i)
{
	if (i->flags & EXPR_F_KERNEL) {
		i->location = prev->location;
		purge_elem(ctx, i);
		return;
	}
	list_del(&i->list);
	expr_free(i);
	expr_set(ctx->init)->size--;
}

struct range {
	mpz_t	low;
	mpz_t	high;
};

static bool merge_ranges(struct set_automerge_ctx *ctx,
			 struct expr *prev, struct expr *i,
			 struct range *prev_range, struct range *range)
{
	if (prev->flags & EXPR_F_KERNEL) {
		prev->location = i->location;
		purge_elem(ctx, prev);
		mpz_set(i->key->range.low, prev->key->range.low);
		mpz_set(prev_range->high, range->high);
		return true;
	} else if (i->flags & EXPR_F_KERNEL) {
		i->location = prev->location;
		purge_elem(ctx, i);
		mpz_set(prev->key->range.high, i->key->range.high);
		mpz_set(prev_range->high, range->high);
	} else {
		mpz_set(prev->key->range.high, i->key->range.high);
		mpz_set(prev_range->high, range->high);
		list_del(&i->list);
		expr_free(i);
		expr_set(ctx->init)->size--;
	}
	return false;
}

static void set_sort_splice(struct expr *init, struct set *set)
{
	struct set *existing_set = set->existing_set;

	set_to_range(init);
	list_expr_sort(&expr_set(init)->expressions);

	if (!existing_set || existing_set->errors)
		return;

	if (existing_set->init) {
		set_to_range(existing_set->init);
		list_splice_sorted(&expr_set(existing_set->init)->expressions,
				   &expr_set(init)->expressions);
		init_list_head(&expr_set(existing_set->init)->expressions);
	} else {
		existing_set->init = set_expr_alloc(&internal_location, set);
	}
}

static void set_prev_elem(struct expr **prev, struct expr *i,
			  struct range *prev_range, struct range *range)
{
	*prev = i;
	mpz_set(prev_range->low, range->low);
	mpz_set(prev_range->high, range->high);
}

static void setelem_automerge(struct set_automerge_ctx *ctx)
{
	struct expr *i, *next, *prev = NULL;
	struct range range, prev_range;
	mpz_t rop;

	mpz_init(prev_range.low);
	mpz_init(prev_range.high);
	mpz_init(range.low);
	mpz_init(range.high);
	mpz_init(rop);

	list_for_each_entry_safe(i, next, &expr_set(ctx->init)->expressions, list) {
		if (expr_type_catchall(i->key))
			continue;

		range_expr_value_low(range.low, i);
		range_expr_value_high(range.high, i);

		if (!prev) {
			set_prev_elem(&prev, i, &prev_range, &range);
			continue;
		}

		if (mpz_cmp(prev_range.low, range.low) <= 0 &&
		    mpz_cmp(prev_range.high, range.high) >= 0) {
			remove_overlapping_range(ctx, prev, i);
			continue;
		} else if (mpz_cmp(range.low, prev_range.high) <= 0) {
			if (merge_ranges(ctx, prev, i, &prev_range, &range))
				prev = i;
			continue;
		} else if (ctx->set->automerge) {
			mpz_sub(rop, range.low, prev_range.high);
			/* two contiguous ranges */
			if (mpz_cmp_ui(rop, 1) == 0) {
				if (merge_ranges(ctx, prev, i, &prev_range, &range))
					prev = i;
				continue;
			}
		}

		set_prev_elem(&prev, i, &prev_range, &range);
	}

	mpz_clear(prev_range.low);
	mpz_clear(prev_range.high);
	mpz_clear(range.low);
	mpz_clear(range.high);
	mpz_clear(rop);
}

static struct expr *interval_expr_key(struct expr *i)
{
	struct expr *elem;

	switch (i->etype) {
	case EXPR_MAPPING:
		elem = i->left;
		break;
	case EXPR_SET_ELEM:
		elem = i;
		break;
	default:
		BUG("unhandled expression type %d", i->etype);
		return NULL;
	}

	return elem;
}

static void set_to_range(struct expr *init)
{
	struct expr *i, *elem;

	list_for_each_entry(i, &expr_set(init)->expressions, list) {
		elem = interval_expr_key(i);
		setelem_expr_to_range(elem);
	}
}

int set_automerge(struct list_head *msgs, struct cmd *cmd, struct set *set,
		  struct expr *init, unsigned int debug_mask)
{
	struct set *existing_set = set->existing_set;
	struct set_automerge_ctx ctx = {
		.set		= set,
		.init		= init,
		.debug_mask	= debug_mask,
	};
	struct expr *i, *next, *clone;
	struct cmd *purge_cmd;
	struct handle h = {};

	if (set->flags & NFT_SET_MAP) {
		set_to_range(init);
		list_expr_sort(&expr_set(init)->expressions);
		return 0;
	}

	set_sort_splice(init, set);

	ctx.purge = set_expr_alloc(&internal_location, set);

	setelem_automerge(&ctx);

	list_for_each_entry_safe(i, next, &expr_set(init)->expressions, list) {
		if (i->flags & EXPR_F_KERNEL) {
			list_move_tail(&i->list, &expr_set(existing_set->init)->expressions);
		} else if (existing_set) {
			if (debug_mask & NFT_DEBUG_SEGTREE) {
				pr_gmp_debug("add: [%Zx-%Zx]\n",
					     i->key->range.low, i->key->range.high);
			}
			clone = expr_clone(i);
			clone->flags |= EXPR_F_KERNEL;
			__set_expr_add(existing_set->init, clone);
		}
	}

	if (list_empty(&expr_set(ctx.purge)->expressions)) {
		expr_free(ctx.purge);
		return 0;
	}

	handle_merge(&h, &set->handle);
	purge_cmd = cmd_alloc(CMD_DELETE, CMD_OBJ_ELEMENTS, &h, &init->location, ctx.purge);
	purge_cmd->elem.set = set_get(set);
	list_add_tail(&purge_cmd->list, &cmd->list);

	return 0;
}

static void remove_elem(struct expr *prev, struct set *set, struct expr *purge)
{
	struct expr *clone;

	if (prev->flags & EXPR_F_KERNEL) {
		clone = expr_clone(prev);
		list_move_tail(&clone->list, &expr_set(purge)->expressions);
	}
}

static void __adjust_elem_left(struct set *set, struct expr *prev, struct expr *i)
{
	prev->flags &= ~EXPR_F_KERNEL;
	mpz_set(prev->key->range.low, i->key->range.high);
	mpz_add_ui(prev->key->range.low, prev->key->range.low, 1);
	list_move(&prev->list, &expr_set(set->existing_set->init)->expressions);
}

static void adjust_elem_left(struct set *set, struct expr *prev, struct expr *i,
			     struct expr *purge)
{
	prev->location = i->location;
	remove_elem(prev, set, purge);
	__adjust_elem_left(set, prev, i);

	list_del(&i->list);
	expr_free(i);
}

static void __adjust_elem_right(struct set *set, struct expr *prev, struct expr *i)
{
	prev->flags &= ~EXPR_F_KERNEL;
	mpz_set(prev->key->range.high, i->key->range.low);
	mpz_sub_ui(prev->key->range.high, prev->key->range.high, 1);
	list_move(&prev->list, &expr_set(set->existing_set->init)->expressions);
}

static void adjust_elem_right(struct set *set, struct expr *prev, struct expr *i,
			      struct expr *purge)
{
	prev->location = i->location;
	remove_elem(prev, set, purge);
	__adjust_elem_right(set, prev, i);

	list_del(&i->list);
	expr_free(i);
}

static void split_range(struct set *set, struct expr *prev, struct expr *i,
			struct expr *purge)
{
	struct expr *clone;

	prev->location = i->location;

	if (prev->flags & EXPR_F_KERNEL) {
		clone = expr_clone(prev);
		list_move_tail(&clone->list, &expr_set(purge)->expressions);
	}

	prev->flags &= ~EXPR_F_KERNEL;
	clone = expr_clone(prev);
	mpz_set(clone->key->range.low, i->key->range.high);
	mpz_add_ui(clone->key->range.low, i->key->range.high, 1);
	__set_expr_add(set->existing_set->init, clone);

	mpz_set(prev->key->range.high, i->key->range.low);
	mpz_sub_ui(prev->key->range.high, i->key->range.low, 1);
	list_move(&prev->list, &expr_set(set->existing_set->init)->expressions);

	list_del(&i->list);
	expr_free(i);
}

static int setelem_adjust(struct set *set, struct expr *purge,
			  struct range *prev_range, struct range *range,
			  struct expr *prev, struct expr *i)
{
	if (mpz_cmp(prev_range->low, range->low) == 0 &&
	    mpz_cmp(prev_range->high, range->high) > 0) {
		if (i->flags & EXPR_F_REMOVE)
			adjust_elem_left(set, prev, i, purge);
	} else if (mpz_cmp(prev_range->low, range->low) < 0 &&
		   mpz_cmp(prev_range->high, range->high) == 0) {
		if (i->flags & EXPR_F_REMOVE)
			adjust_elem_right(set, prev, i, purge);
	} else if (mpz_cmp(prev_range->low, range->low) < 0 &&
		   mpz_cmp(prev_range->high, range->high) > 0) {
		if (i->flags & EXPR_F_REMOVE)
			split_range(set, prev, i, purge);
	} else {
		return -1;
	}

	return 0;
}

static int setelem_delete(struct list_head *msgs, struct set *set,
			  struct expr *purge, struct expr *elems,
			  unsigned int debug_mask)
{
	struct expr *i, *next, *elem, *prev = NULL;
	struct range range, prev_range;
	int err = 0;
	mpz_t rop;

	mpz_init(prev_range.low);
	mpz_init(prev_range.high);
	mpz_init(range.low);
	mpz_init(range.high);
	mpz_init(rop);

	list_for_each_entry_safe(elem, next, &expr_set(elems)->expressions, list) {
		i = interval_expr_key(elem);

		if (expr_type_catchall(i->key)) {
			/* Assume max value to simplify handling. */
			mpz_bitmask(range.low, i->len);
			mpz_bitmask(range.high, i->len);
		} else {
			range_expr_value_low(range.low, i);
			range_expr_value_high(range.high, i);
		}

		if (!prev && elem->flags & EXPR_F_REMOVE) {
			expr_error(msgs, i, "element does not exist");
			err = -1;
			goto err;
		}

		if (!(elem->flags & EXPR_F_REMOVE)) {
			prev = elem;
			mpz_set(prev_range.low, range.low);
			mpz_set(prev_range.high, range.high);
			continue;
		}

		if (mpz_cmp(prev_range.low, range.low) == 0 &&
		    mpz_cmp(prev_range.high, range.high) == 0) {
			if (elem->flags & EXPR_F_REMOVE) {
				if (prev->flags & EXPR_F_KERNEL) {
					prev->location = elem->location;
					list_move_tail(&prev->list, &expr_set(purge)->expressions);
				}

				list_del(&elem->list);
				expr_free(elem);
			}
		} else if (set->automerge) {
			if (setelem_adjust(set, purge, &prev_range, &range, prev, i) < 0) {
				expr_error(msgs, i, "element does not exist");
				err = -1;
				goto err;
			}
		} else if (elem->flags & EXPR_F_REMOVE) {
			expr_error(msgs, i, "element does not exist");
			err = -1;
			goto err;
		}
		prev = NULL;
	}
err:
	mpz_clear(prev_range.low);
	mpz_clear(prev_range.high);
	mpz_clear(range.low);
	mpz_clear(range.high);
	mpz_clear(rop);

	return err;
}

static void automerge_delete(struct list_head *msgs, struct set *set,
			     struct expr *init, unsigned int debug_mask)
{
	struct set_automerge_ctx ctx = {
		.set		= set,
		.init		= init,
		.debug_mask	= debug_mask,
	};

	ctx.purge = set_expr_alloc(&internal_location, set);
	list_expr_sort(&expr_set(init)->expressions);
	setelem_automerge(&ctx);
	expr_free(ctx.purge);
}

static int __set_delete(struct list_head *msgs, struct expr *i,	struct set *set,
			struct expr *init, struct set *existing_set,
			unsigned int debug_mask)
{
	i->flags |= EXPR_F_REMOVE;
	list_move_tail(&i->list, &expr_set(existing_set->init)->expressions);
	list_expr_sort(&expr_set(existing_set->init)->expressions);

	return setelem_delete(msgs, set, init, existing_set->init, debug_mask);
}

/* detection for unexisting intervals already exists in Linux kernels >= 5.7. */
int set_delete(struct list_head *msgs, struct cmd *cmd, struct set *set,
	       struct expr *init, unsigned int debug_mask)
{
	struct set *existing_set = set->existing_set;
	struct expr *i, *next, *add, *clone;
	struct handle h = {};
	struct cmd *add_cmd;
	LIST_HEAD(del_list);
	int err;

	set_to_range(init);
	if (set->automerge)
		automerge_delete(msgs, set, init, debug_mask);

	if (existing_set->init) {
		set_to_range(existing_set->init);
	} else {
		existing_set->init = set_expr_alloc(&internal_location, set);
	}

	list_splice_init(&expr_set(init)->expressions, &del_list);

	list_for_each_entry_safe(i, next, &del_list, list) {
		err = __set_delete(msgs, i, set, init, existing_set, debug_mask);
		if (err < 0) {
			list_splice(&del_list, &expr_set(init)->expressions);
			return err;
		}
	}

	add = set_expr_alloc(&internal_location, set);
	list_for_each_entry(i, &expr_set(existing_set->init)->expressions, list) {
		if (!(i->flags & EXPR_F_KERNEL)) {
			clone = expr_clone(i);
			__set_expr_add(add, clone);
			i->flags |= EXPR_F_KERNEL;
		}
	}

	if (debug_mask & NFT_DEBUG_SEGTREE) {
		list_for_each_entry(i, &expr_set(init)->expressions, list)
			pr_gmp_debug("remove: [%Zx-%Zx]\n",
				     i->key->range.low, i->key->range.high);
		list_for_each_entry(i, &expr_set(add)->expressions, list)
			pr_gmp_debug("add: [%Zx-%Zx]\n",
				     i->key->range.low, i->key->range.high);
		list_for_each_entry(i, &expr_set(existing_set->init)->expressions, list)
			pr_gmp_debug("existing: [%Zx-%Zx]\n",
				     i->key->range.low, i->key->range.high);
	}

	if (list_empty(&expr_set(add)->expressions)) {
		expr_free(add);
		return 0;
	}

	handle_merge(&h, &cmd->handle);
	add_cmd = cmd_alloc(CMD_ADD, CMD_OBJ_ELEMENTS, &h, &cmd->location, add);
	add_cmd->elem.set = set_get(set);
	list_add(&add_cmd->list, &cmd->list);

	return 0;
}

static int setelem_overlap(struct list_head *msgs, struct set *set,
			   struct expr *init)
{
	struct expr *i, *next, *elem, *prev = NULL;
	struct range range, prev_range;
	int err = 0;
	mpz_t rop;

	mpz_init(prev_range.low);
	mpz_init(prev_range.high);
	mpz_init(range.low);
	mpz_init(range.high);
	mpz_init(rop);

	list_for_each_entry_safe(elem, next, &expr_set(init)->expressions, list) {
		i = interval_expr_key(elem);

		if (expr_type_catchall(i->key))
			continue;

		range_expr_value_low(range.low, i);
		range_expr_value_high(range.high, i);

		if (!prev) {
			prev = elem;
			mpz_set(prev_range.low, range.low);
			mpz_set(prev_range.high, range.high);
			continue;
		}

		if (mpz_cmp(prev_range.low, range.low) == 0 &&
		    mpz_cmp(prev_range.high, range.high) == 0)
			goto next;

		if (mpz_cmp(prev_range.low, range.low) <= 0 &&
		    mpz_cmp(prev_range.high, range.high) >= 0) {
			if (prev->flags & EXPR_F_KERNEL)
				expr_error(msgs, i, "interval overlaps with an existing one");
			else if (elem->flags & EXPR_F_KERNEL)
				expr_error(msgs, prev, "interval overlaps with an existing one");
			else
				expr_binary_error(msgs, i, prev,
						  "conflicting intervals specified");
			err = -1;
			goto err_out;
		} else if (mpz_cmp(range.low, prev_range.high) <= 0) {
			if (prev->flags & EXPR_F_KERNEL)
				expr_error(msgs, i, "interval overlaps with an existing one");
			else if (elem->flags & EXPR_F_KERNEL)
				expr_error(msgs, prev, "interval overlaps with an existing one");
			else
				expr_binary_error(msgs, i, prev,
						  "conflicting intervals specified");
			err = -1;
			goto err_out;
		}
next:
		prev = elem;
		mpz_set(prev_range.low, range.low);
		mpz_set(prev_range.high, range.high);
	}

err_out:
	mpz_clear(prev_range.low);
	mpz_clear(prev_range.high);
	mpz_clear(range.low);
	mpz_clear(range.high);
	mpz_clear(rop);

	return err;
}

/* overlap detection for intervals already exists in Linux kernels >= 5.7. */
int set_overlap(struct list_head *msgs, struct set *set, struct expr *init)
{
	struct set *existing_set = set->existing_set;
	struct expr *i, *n, *clone;
	int err;

	set_sort_splice(init, set);

	err = setelem_overlap(msgs, set, init);

	list_for_each_entry_safe(i, n, &expr_set(init)->expressions, list) {
		if (i->flags & EXPR_F_KERNEL)
			list_move_tail(&i->list, &expr_set(existing_set->init)->expressions);
		else if (existing_set) {
			clone = expr_clone(i);
			clone->flags |= EXPR_F_KERNEL;
			__set_expr_add(existing_set->init, clone);
		}
	}

	return err;
}

static bool segtree_needs_first_segment(const struct set *set,
					const struct expr *init, bool add)
{
	if (add && !set->root) {
		/* Add the first segment in four situations:
		 *
		 * 1) This is an anonymous set.
		 * 2) This set exists and it is empty.
		 * 3) New empty set and, separately, new elements are added.
		 * 4) This set is created with a number of initial elements.
		 */
		if ((set_is_anonymous(set->flags)) ||
		    (set->init && expr_set(set->init)->size == 0) ||
		    (set->init == NULL && init) ||
		    (set->init == init)) {
			return true;
		}
	}
	/* This is an update for a set that already contains elements, so don't
	 * add the first non-matching elements otherwise we hit EEXIST.
	 */
	return false;
}

int set_to_intervals(const struct set *set, struct expr *init, bool add)
{
	struct expr *i, *n, *prev = NULL, *elem, *root, *expr;
	LIST_HEAD(intervals);
	mpz_t p;

	list_for_each_entry_safe(i, n, &expr_set(init)->expressions, list) {
		elem = interval_expr_key(i);

		if (expr_type_catchall(elem->key))
			continue;

		if (prev)
			break;

		if (segtree_needs_first_segment(set, init, add) &&
		    mpz_cmp_ui(elem->key->range.low, 0)) {
			mpz_init2(p, set->key->len);
			mpz_set_ui(p, 0);
			expr = constant_range_expr_alloc(&internal_location,
							 set->key->dtype,
							 set->key->byteorder,
							 set->key->len, p, p);
			mpz_clear(p);

			root = set_elem_expr_alloc(&internal_location, expr);
			if (i->etype == EXPR_MAPPING) {
				root = mapping_expr_alloc(&internal_location,
							  root,
							  expr_get(i->right));
			}
			root->flags |= EXPR_F_INTERVAL_END;
			list_add(&root->list, &intervals);
			break;
		}
		prev = i;
	}

	list_splice_init(&intervals, &expr_set(init)->expressions);

	return 0;
}

/* This only works for the supported stateful statements. */
static void set_elem_stmt_clone(struct expr *dst, const struct expr *src)
{
	struct stmt *stmt, *nstmt;

	list_for_each_entry(stmt, &src->stmt_list, list) {
		nstmt = xzalloc(sizeof(*stmt));
		*nstmt = *stmt;
		list_add_tail(&nstmt->list, &dst->stmt_list);
	}
}

static void set_elem_expr_copy(struct expr *dst, const struct expr *src)
{
	if (src->comment)
		dst->comment = xstrdup(src->comment);
	if (src->timeout)
		dst->timeout = src->timeout;
	if (src->expiration)
		dst->expiration = src->expiration;

	set_elem_stmt_clone(dst, src);
}

static struct expr *setelem_key(struct expr *expr)
{
	struct expr *key;

	switch (expr->etype) {
	case EXPR_MAPPING:
		key = expr->left->key;
		break;
	case EXPR_SET_ELEM:
		key = expr->key;
		break;
	default:
		BUG("unhandled expression type %d", expr->etype);
		return NULL;
	}

	return key;
}

int setelem_to_interval(const struct set *set, struct expr *elem,
			struct expr *next_elem, struct list_head *intervals)
{
	struct expr *key, *next_key = NULL, *low, *high;
	bool adjacent = false;

	key = setelem_key(elem);
	if (expr_type_catchall(key))
		return 0;

	if (next_elem) {
		next_key = setelem_key(next_elem);
		if (expr_type_catchall(next_key))
			next_key = NULL;
	}

	if (key->etype != EXPR_RANGE_VALUE)
		BUG("key must be RANGE_VALUE, not %s", expr_name(key));

	assert(!next_key || next_key->etype == EXPR_RANGE_VALUE);

	/* skip end element for adjacents intervals in anonymous sets. */
	if (!(elem->flags & EXPR_F_INTERVAL_END) && next_key) {
		mpz_t p;

		mpz_init2(p, set->key->len);
		mpz_add_ui(p, key->range.high, 1);

		if (!mpz_cmp(p, next_key->range.low))
			adjacent = true;

		mpz_clear(p);
	}

	low = constant_expr_alloc(&key->location, set->key->dtype,
				  set->key->byteorder, set->key->len, NULL);

	mpz_set(low->value, key->range.low);
	if (set->key->byteorder == BYTEORDER_HOST_ENDIAN)
		mpz_switch_byteorder(low->value, set->key->len / BITS_PER_BYTE);

	low = set_elem_expr_alloc(&key->location, low);
	set_elem_expr_copy(low, interval_expr_key(elem));

	if (elem->etype == EXPR_MAPPING)
		low = mapping_expr_alloc(&elem->location,
					 low, expr_get(elem->right));

	list_add_tail(&low->list, intervals);

	if (adjacent)
		return 0;
	else if (!mpz_cmp_ui(key->value, 0) && elem->flags & EXPR_F_INTERVAL_END) {
		low->flags |= EXPR_F_INTERVAL_END;
		return 0;
	} else if (mpz_scan0(key->range.high, 0) == set->key->len) {
		low->flags |= EXPR_F_INTERVAL_OPEN;
		return 0;
	}

	high = constant_expr_alloc(&key->location, set->key->dtype,
				   set->key->byteorder, set->key->len,
				   NULL);
	mpz_set(high->value, key->range.high);
	mpz_add_ui(high->value, high->value, 1);
	if (set->key->byteorder == BYTEORDER_HOST_ENDIAN)
		mpz_switch_byteorder(high->value, set->key->len / BITS_PER_BYTE);

	high = set_elem_expr_alloc(&key->location, high);

	high->flags |= EXPR_F_INTERVAL_END;
	list_add_tail(&high->list, intervals);

	return 0;
}
