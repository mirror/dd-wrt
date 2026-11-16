/*
 * Copyright (c) 2008-2012 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Development of this code funded by Astaro AG (http://www.astaro.com/)
 */

#include <nft.h>

#include <inttypes.h>
#include <arpa/inet.h>

#include <libnftnl/udata.h>

#include <rule.h>
#include <expression.h>
#include <gmputil.h>
#include <utils.h>

static enum byteorder get_key_byteorder(const struct expr *e)
{
	enum datatypes basetype = expr_basetype(e)->type;

	switch (basetype) {
	case TYPE_INTEGER:
		/* For ranges, integers MUST be in BYTEORDER_BIG_ENDIAN.
		 * If the LHS (lookup key, e.g. 'meta mark', is host endian,
		 * a byteorder expression is injected to convert the register
		 * content before lookup.
		 */
		return BYTEORDER_BIG_ENDIAN;
	case TYPE_STRING:
		return BYTEORDER_HOST_ENDIAN;
	default:
		break;
	}

	return BYTEORDER_INVALID;
}

static void interval_expr_copy(struct expr *dst, struct expr *src)
{
	assert(dst->etype == EXPR_SET_ELEM);
	assert(src->etype == EXPR_SET_ELEM);

	if (src->comment)
		dst->comment = xstrdup(src->comment);
	if (src->timeout)
		dst->timeout = src->timeout;
	if (src->expiration)
		dst->expiration = src->expiration;

	list_splice_init(&src->stmt_list, &dst->stmt_list);
}

static void set_elem_expr_add(const struct set *set, struct expr *init,
			      mpz_t value, uint32_t flags, enum byteorder byteorder)
{
	struct expr *expr;

	expr = constant_expr_alloc(&internal_location, set->key->dtype,
				   byteorder, set->key->len, NULL);
	mpz_set(expr->value, value);
	expr = set_elem_expr_alloc(&internal_location, expr);
	expr->key->flags = flags;

	set_expr_add(init, expr);
}

struct expr *get_set_intervals(const struct set *set, const struct expr *init)
{
	enum byteorder byteorder = get_key_byteorder(set->key);
	mpz_t low, high, mask;
	struct expr *new_init;
	struct expr *i;

	mpz_init2(low, set->key->len);
	mpz_init2(high, set->key->len);
	mpz_init2(mask, set->key->len);

	new_init = set_expr_alloc(&internal_location, NULL);

	list_for_each_entry(i, &expr_set(init)->expressions, list) {
		assert(i->etype == EXPR_SET_ELEM);

		switch (i->key->etype) {
		case EXPR_VALUE:
			set_elem_expr_add(set, new_init, i->key->value,
					  i->key->flags, byteorder);
			break;
		case EXPR_CONCAT:
			set_expr_add(new_init, expr_clone(i));
			i->key->flags |= EXPR_F_INTERVAL_END;
			set_expr_add(new_init, expr_clone(i));
			break;
		case EXPR_SET_ELEM_CATCHALL:
			set_expr_add(new_init, expr_clone(i));
			break;
		case EXPR_PREFIX:
		case EXPR_RANGE:
		case EXPR_RANGE_VALUE:
		case EXPR_MAPPING:
			range_expr_value_low(low, i->key);
			set_elem_expr_add(set, new_init, low, 0, byteorder);
			range_expr_value_high(high, i->key);
			mpz_bitmask(mask, i->len);
			if (set_is_non_concat_range(set) &&
			    !mpz_cmp(mask, high))
				break;

			mpz_add_ui(high, high, 1);
			set_elem_expr_add(set, new_init, high,
					  EXPR_F_INTERVAL_END, byteorder);
			break;
		default:
			BUG("unexpected expression %s", expr_name(i->key));
			break;
		}
	}

	mpz_clear(mask);
	mpz_clear(low);
	mpz_clear(high);

	return new_init;
}

static struct expr *expr_value(struct expr *expr)
{
	assert(expr->etype == EXPR_SET_ELEM);

	if (expr->key->etype == EXPR_MAPPING)
		return expr->key->left;

	return expr->key;
}

static struct expr *get_set_interval_find(const struct set *cache_set,
					  struct expr *left,
					  struct expr *right)
{
	const struct set *set = cache_set;
	struct expr *range = NULL;
	struct expr *i, *key;
	mpz_t val;

	assert(left->etype == EXPR_SET_ELEM);
	assert(!right || right->etype == EXPR_SET_ELEM);

	mpz_init2(val, set->key->len);

	list_for_each_entry(i, &expr_set(set->init)->expressions, list) {
		assert(i->etype == EXPR_SET_ELEM);

		key = expr_value(i);
		switch (key->etype) {
		case EXPR_VALUE:
			if (expr_basetype(i->key)->type != TYPE_STRING)
				break;
			/* string type, check if its a range (wildcard). */
			/* fall-through */
		case EXPR_PREFIX:
		case EXPR_RANGE:
			range_expr_value_low(val, i->key);
			if (left && mpz_cmp(expr_value(left)->value, val))
				break;

			range_expr_value_high(val, i->key);
			if (right && mpz_cmp(expr_value(right)->value, val))
				break;

			range = expr_clone(i);
			goto out;
		default:
			break;
		}
	}
out:
	mpz_clear(val);

	return range;
}

static struct expr *__expr_to_set_elem(struct expr *low, struct expr *expr)
{
	struct expr *elem;

	assert(low->etype == EXPR_SET_ELEM);

	if (low->key->etype == EXPR_MAPPING) {
		expr = mapping_expr_alloc(&low->location, expr,
					  expr_clone(low->key->right));
	}

	elem = set_elem_expr_alloc(&low->location, expr);
	elem->key->flags |= EXPR_F_KERNEL;
	interval_expr_copy(elem, low);

	return elem;
}

static struct expr *expr_to_set_elem(struct expr *e)
{
	unsigned int len = div_round_up(e->len, BITS_PER_BYTE);
	struct expr *expr, *key;
	unsigned int str_len;
	char data[len + 1];

	assert(e->etype == EXPR_SET_ELEM);

	key = expr_value(e);
	if (expr_basetype(key)->type != TYPE_STRING)
		return expr_clone(e);

	mpz_export_data(data, key->value, BYTEORDER_BIG_ENDIAN, len);

	str_len = strnlen(data, len);
	if (str_len >= len || str_len == 0)
		return expr_clone(e);

	data[str_len] = '*';

	expr = constant_expr_alloc(&e->location, e->dtype,
				   BYTEORDER_HOST_ENDIAN,
				   (str_len + 1) * BITS_PER_BYTE, data);

	return __expr_to_set_elem(e, expr);
}

static void set_expr_add_splice(struct expr *compound, struct expr *expr, struct expr *orig)
{
	assert(expr->etype == EXPR_SET_ELEM);
	assert(orig->etype == EXPR_SET_ELEM);

	list_splice_init(&orig->stmt_list, &expr->stmt_list);
	set_expr_add(compound, expr);
}

int get_set_decompose(struct set *cache_set, struct set *set)
{
	struct expr *i, *next, *range;
	struct expr *left = NULL;
	struct expr *new_init;

	new_init = set_expr_alloc(&internal_location, set);

	list_for_each_entry_safe(i, next, &expr_set(set->init)->expressions, list) {
		assert(i->etype == EXPR_SET_ELEM);

		if (i->key->flags & EXPR_F_INTERVAL_END && left) {
			list_del(&left->list);
			list_del(&i->list);

			assert(i->key->etype == EXPR_VALUE);

			mpz_sub_ui(i->key->value, i->key->value, 1);
			range = get_set_interval_find(cache_set, left, i);
			if (!range) {
				expr_free(left);
				expr_free(i);
				expr_free(new_init);
				errno = ENOENT;
				return -1;
			}

			set_expr_add_splice(new_init, range, left);

			expr_free(left);
			expr_free(i);

			left = NULL;
		} else {
			if (left) {
				range = get_set_interval_find(cache_set,
							      left, NULL);

				if (range)
					set_expr_add_splice(new_init, range, left);
				else
					set_expr_add_splice(new_init,
							      expr_to_set_elem(left), left);
			}
			left = i;
		}
	}
	if (left) {
		range = get_set_interval_find(cache_set, left, NULL);
		if (range)
			set_expr_add_splice(new_init, range, left);
		else
			set_expr_add_splice(new_init, expr_to_set_elem(left), left);
	}

	expr_free(set->init);
	set->init = new_init;

	return 0;
}

static bool range_is_prefix(const mpz_t range)
{
	mpz_t tmp;
	bool ret;

	mpz_init_set(tmp, range);
	mpz_add_ui(tmp, tmp, 1);
	mpz_and(tmp, range, tmp);
	ret = !mpz_cmp_ui(tmp, 0);
	mpz_clear(tmp);
	return ret;
}

static int expr_value_cmp(const void *p1, const void *p2)
{
	struct expr *e1 = *(void * const *)p1;
	struct expr *e2 = *(void * const *)p2;
	struct expr *key_e1, *key_e2;
	int ret;

	assert(e1->etype == EXPR_SET_ELEM);
	assert(e2->etype == EXPR_SET_ELEM);

	key_e1 = expr_value(e1);
	if (key_e1->etype == EXPR_CONCAT)
		return -1;

	key_e2 = expr_value(e2);

	assert(key_e1->etype == EXPR_VALUE);
	assert(key_e2->etype == EXPR_VALUE);

	ret = mpz_cmp(key_e1->value, key_e2->value);
	if (ret == 0) {
		if (e1->key->flags & EXPR_F_INTERVAL_END)
			return -1;
		else if (e2->key->flags & EXPR_F_INTERVAL_END)
			return 1;
	}

	return ret;
}

/* Given start and end elements of a range, check if it can be represented as
 * a single netmask, and if so, how long, by returning zero or a positive value.
 */
static int range_mask_len(const mpz_t start, const mpz_t end, unsigned int len)
{
	mpz_t tmp_start, tmp_end;
	int ret;

	mpz_init_set(tmp_start, start);
	mpz_init_set(tmp_end, end);

	while (mpz_cmp(tmp_start, tmp_end) <= 0 &&
		!mpz_tstbit(tmp_start, 0) && mpz_tstbit(tmp_end, 0) &&
		len--) {
		mpz_fdiv_q_2exp(tmp_start, tmp_start, 1);
		mpz_fdiv_q_2exp(tmp_end, tmp_end, 1);
	}

	ret = !mpz_cmp(tmp_start, tmp_end) ? (int)len : -1;

	mpz_clear(tmp_start);
	mpz_clear(tmp_end);

	return ret;
}

/* Given a set with two elements (start and end), transform them into a
 * concatenation of ranges. That is, from a list of start expressions and a list
 * of end expressions, form a list of start - end expressions.
 */
void concat_range_aggregate(struct expr *set)
{
	struct expr *i, *start, *end, *prev = NULL, *r1, *r2, *next, *r1_next, *tmp;
	struct list_head *r2_next;
	int prefix_len, free_r1;
	mpz_t range, p;

	list_for_each_entry_safe(i, next, &expr_set(set)->expressions, list) {
		assert(i->etype == EXPR_SET_ELEM);

		if (!prev) {
			prev = i;
			continue;
		}
		start = expr_value(prev);
		end = expr_value(i);

		/* Walk over r1 (start expression) and r2 (end) in parallel,
		 * form ranges between corresponding r1 and r2 expressions,
		 * store them by replacing r2 expressions, and free r1
		 * expressions.
		 */
		r2 = list_first_entry(&expr_concat(end)->expressions,
				      struct expr, list);
		list_for_each_entry_safe(r1, r1_next,
					 &expr_concat(start)->expressions,
					 list) {
			bool string_type = false;

			mpz_init(range);
			mpz_init(p);

			r2_next = r2->list.next;
			free_r1 = 0;

			assert(r1->etype == EXPR_VALUE && r1->etype == EXPR_VALUE);

			if (!mpz_cmp(r1->value, r2->value)) {
				if (r2->dtype->basetype != NULL &&
				    r2->dtype->basetype->type == TYPE_BITMASK) {
					expr_get(r2);
					tmp = bitmask_expr_to_binops(r2);
					list_replace(&r2->list, &tmp->list);
					expr_free(r2);
				}
				free_r1 = 1;
				goto next;
			}

			if (expr_basetype(r1)->type == TYPE_STRING &&
			    expr_basetype(r2)->type == TYPE_STRING) {
				string_type = true;
				mpz_switch_byteorder(r1->value, r1->len / BITS_PER_BYTE);
				mpz_switch_byteorder(r2->value, r2->len / BITS_PER_BYTE);
			}

			mpz_sub(range, r2->value, r1->value);
			mpz_sub_ui(range, range, 1);
			mpz_and(p, r1->value, range);

			/* Check if we are forced, or if it's anyway preferable,
			 * to express the range as a wildcard string, or two points
			 * instead of a netmask.
			 */
			prefix_len = range_mask_len(r1->value, r2->value,
						    r1->len);
			if (string_type) {
				mpz_switch_byteorder(r1->value, r1->len / BITS_PER_BYTE);
				mpz_switch_byteorder(r2->value, r2->len / BITS_PER_BYTE);
			}

			if (prefix_len >= 0 &&
			    (prefix_len % BITS_PER_BYTE) == 0 &&
			    string_type) {
				unsigned int r1len = div_round_up(r1->len, BITS_PER_BYTE);
				unsigned int str_len = prefix_len / BITS_PER_BYTE;
				char data[r1len + 1];

				memset(data, 0, r1len + 1);
				mpz_export_data(data, r1->value, BYTEORDER_HOST_ENDIAN, r1len);
				data[str_len] = '*';

				tmp = constant_expr_alloc(&r1->location, r1->dtype,
							  BYTEORDER_HOST_ENDIAN,
							  (str_len + 1) * BITS_PER_BYTE, data);
				list_replace(&r2->list, &tmp->list);
				r2_next = tmp->list.next;
				expr_free(r2);
				free_r1 = 1;
				goto next;
			}

			if (prefix_len < 0 ||
			    !datatype_prefix_notation(r1->dtype)) {
				tmp = range_expr_alloc(&r1->location, r1,
						       r2);

				list_replace(&r2->list, &tmp->list);
				r2_next = tmp->list.next;
			} else {
				tmp = prefix_expr_alloc(&r1->location, r1,
							prefix_len);
				tmp->len = r2->len;

				list_replace(&r2->list, &tmp->list);
				r2_next = tmp->list.next;
				expr_free(r2);
			}

next:
			mpz_clear(p);
			mpz_clear(range);

			r2 = list_entry(r2_next, typeof(*r2), list);
			concat_expr_remove(start, r1);

			if (free_r1)
				expr_free(r1);
		}

		set_expr_remove(set, prev);
		expr_free(prev);
		prev = NULL;
	}
}

static struct expr *interval_to_prefix(struct expr *low, struct expr *i, const mpz_t range)
{
	unsigned int prefix_len;
	struct expr *prefix;
	struct expr *key;

	assert(low->etype == EXPR_SET_ELEM);
	assert(i->etype == EXPR_SET_ELEM);

	key = expr_value(i);
	prefix_len = key->len - mpz_scan0(range, 0);
	prefix = prefix_expr_alloc(&low->location,
				   expr_clone(expr_value(low)),
						   prefix_len);
	prefix->len = key->len;

	return __expr_to_set_elem(low, prefix);
}

static struct expr *interval_to_string(struct expr *low, struct expr *i, const mpz_t range)
{
	unsigned int len = div_round_up(i->len, BITS_PER_BYTE);
	unsigned int prefix_len, str_len;
	char data[len + 2];
	struct expr *expr;
	struct expr *key;

	assert(low->etype == EXPR_SET_ELEM);
	assert(i->etype == EXPR_SET_ELEM);

	key = expr_value(i);
	prefix_len = key->len - mpz_scan0(range, 0);

	if (prefix_len > i->len || prefix_len % BITS_PER_BYTE)
		return interval_to_prefix(low, i, range);

	mpz_export_data(data, expr_value(low)->value, BYTEORDER_BIG_ENDIAN, len);

	str_len = strnlen(data, len);
	if (str_len >= len || str_len == 0)
		return interval_to_prefix(low, i, range);

	data[str_len] = '*';

	expr = constant_expr_alloc(&low->location, low->dtype,
				   BYTEORDER_HOST_ENDIAN,
				   len * BITS_PER_BYTE, data);

	return __expr_to_set_elem(low, expr);
}

static struct expr *interval_to_range(struct expr *low, struct expr *i, mpz_t range)
{
	struct expr *tmp, *key;

	assert(low->etype == EXPR_SET_ELEM);
	assert(i->etype == EXPR_SET_ELEM);

	key = expr_value(low);
	tmp = constant_expr_alloc(&low->location, low->dtype,
				  low->byteorder, key->len,
				  NULL);

	mpz_add(range, range, key->value);
	mpz_set(tmp->value, range);

	tmp = range_expr_alloc(&low->location, expr_clone(key), tmp);

	return __expr_to_set_elem(low, tmp);
}

static void
add_interval(struct expr *set, struct expr *low, struct expr *i, bool closed)
{
	struct expr *expr, *key;
	mpz_t range, p;

	assert(low->etype == EXPR_SET_ELEM);
	assert(i->etype == EXPR_SET_ELEM);

	mpz_init(range);
	mpz_init(p);

	key = expr_value(low);

	assert(expr_value(i)->etype == EXPR_VALUE);
	assert(key->etype == EXPR_VALUE);

	mpz_sub(range, expr_value(i)->value, key->value);
	if (closed)
		mpz_sub_ui(range, range, 1);

	mpz_and(p, key->value, range);

	if (!mpz_cmp_ui(range, 0)) {
		if (expr_basetype(low)->type == TYPE_STRING)
			mpz_switch_byteorder(key->value,
					     key->len / BITS_PER_BYTE);

		if (key->dtype->basetype != NULL &&
		    key->dtype->basetype->type == TYPE_BITMASK) {
			if (low->key->etype == EXPR_MAPPING)
				low->key->left = bitmask_expr_to_binops(low->key->left);
			else
				low->key = bitmask_expr_to_binops(low->key);
		}
		low->key->flags |= EXPR_F_KERNEL;
		expr = expr_get(low);
	} else if (range_is_prefix(range) && !mpz_cmp_ui(p, 0)) {

		if (datatype_prefix_notation(i->dtype))
			expr = interval_to_prefix(low, i, range);
		else if (expr_basetype(i)->type == TYPE_STRING)
			expr = interval_to_string(low, i, range);
		else
			expr = interval_to_range(low, i, range);
	} else
		expr = interval_to_range(low, i, range);

	set_expr_add(set, expr);

	mpz_clear(range);
	mpz_clear(p);
}

void interval_map_decompose(struct expr *set)
{
	struct expr *i, *next, *low = NULL, *end, *catchall = NULL, *key;
	struct expr **elements, **ranges;
	unsigned int n, m, size;
	bool interval;

	if (expr_set(set)->size == 0)
		return;

	elements = xmalloc_array(expr_set(set)->size, sizeof(struct expr *));
	ranges = xmalloc_array(expr_set(set)->size * 2, sizeof(struct expr *));

	/* Sort elements */
	n = 0;
	list_for_each_entry_safe(i, next, &expr_set(set)->expressions, list) {
		assert(i->etype == EXPR_SET_ELEM);

		key = i->key;
		if (expr_type_catchall(key)) {
			list_del(&i->list);
			catchall = i;
			continue;
		}
		set_expr_remove(set, i);
		elements[n++] = i;
	}
	qsort(elements, n, sizeof(elements[0]), expr_value_cmp);
	size = n;

	/* Transform points (single values) into half-closed intervals */
	n = 0;
	interval = false;
	for (m = 0; m < size; m++) {
		i = elements[m];

		assert(i->etype == EXPR_SET_ELEM);

		if (i->key->flags & EXPR_F_INTERVAL_END)
			interval = false;
		else if (interval) {
			end = expr_clone(i);
			end->key->flags |= EXPR_F_INTERVAL_END;
			ranges[n++] = end;
		} else
			interval = true;

		ranges[n++] = i;
	}
	size = n;

	for (n = 0; n < size; n++) {
		i = ranges[n];

		assert(i->etype == EXPR_SET_ELEM);

		if (low == NULL) {
			if (i->key->flags & EXPR_F_INTERVAL_END) {
				/*
				 * End of interval mark
				 */
				expr_free(i);
				continue;
			} else {
				/*
				 * Start a new interval
				 */
				low = i;
				continue;
			}
		}

		add_interval(set, low, i, true);

		if (i->key->flags & EXPR_F_INTERVAL_END) {
			expr_free(low);
			low = NULL;
		}
		expr_free(i);
	}

	if (!low) /* no unclosed interval at end */
		goto out;

	key = expr_value(low);
	i = constant_expr_alloc(&low->location, low->dtype,
				low->byteorder, key->len, NULL);
	mpz_bitmask(i->value, i->len);
	i = set_elem_expr_alloc(&low->location, i);

	if (!mpz_cmp(i->key->value, key->value)) {
		set_expr_add(set, low);
	} else {
		add_interval(set, low, i, false);
		expr_free(low);
	}

	expr_free(i);

out:
	if (catchall) {
		catchall->key->flags |= EXPR_F_KERNEL;
		set_expr_add(set, catchall);
	}

	free(ranges);
	free(elements);
}
