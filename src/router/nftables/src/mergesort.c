/*
 * Copyright (c) 2017 Elise Lennion <elise.lennion@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 (or any
 * later) as published by the Free Software Foundation.
 */

#include <nft.h>

#include <expression.h>
#include <gmputil.h>
#include <list.h>

static void concat_expr_msort_value(const struct expr *expr, mpz_t value)
{
	unsigned int len = 0, ilen;
	const struct expr *i;
	char data[512];

	list_for_each_entry(i, &expr_concat(expr)->expressions, list) {
		ilen = div_round_up(i->len, BITS_PER_BYTE);
		mpz_export_data(data + len, i->value, i->byteorder, ilen);
		len += ilen;
	}

	mpz_import_data(value, data, BYTEORDER_HOST_ENDIAN, len);
}

static mpz_srcptr expr_msort_value(const struct expr *expr, mpz_t value)
{
	switch (expr->etype) {
	case EXPR_SET_ELEM:
		return expr_msort_value(expr->key, value);
	case EXPR_BINOP:
	case EXPR_MAPPING:
	case EXPR_RANGE:
		return expr_msort_value(expr->left, value);
	case EXPR_VALUE:
		return expr->value;
	case EXPR_RANGE_VALUE:
		return expr->range.low;
	case EXPR_CONCAT:
		concat_expr_msort_value(expr, value);
		break;
	case EXPR_SET_ELEM_CATCHALL:
		/* max value to ensure listing shows it in the last position */
		mpz_bitmask(value, expr->len);
		break;
	default:
		BUG("Unknown expression %s", expr_name(expr));
	}
	return value;
}

static int expr_msort_cmp(const struct expr *e1, const struct expr *e2)
{
	mpz_srcptr value1;
	mpz_srcptr value2;
	mpz_t value1_tmp;
	mpz_t value2_tmp;
	int ret;

	mpz_init(value1_tmp);
	mpz_init(value2_tmp);
	value1 = expr_msort_value(e1, value1_tmp);
	value2 = expr_msort_value(e2, value2_tmp);
	ret = mpz_cmp(value1, value2);
	mpz_clear(value1_tmp);
	mpz_clear(value2_tmp);

	return ret;
}

void list_splice_sorted(struct list_head *list, struct list_head *head)
{
	struct list_head *h = head->next;
	struct list_head *l = list->next;

	while (l != list) {
		if (h == head ||
		    expr_msort_cmp(list_entry(l, typeof(struct expr), list),
				   list_entry(h, typeof(struct expr), list)) <= 0) {
			l = l->next;
			list_add_tail(l->prev, h);
			continue;
		}

		h = h->next;
	}
}

static void list_cut_middle(struct list_head *list, struct list_head *head)
{
	struct list_head *s = head->next;
	struct list_head *e = head->prev;

	while (e != s) {
		e = e->prev;

		if (e != s)
			s = s->next;
	}

	__list_cut_position(list, head, s);
}

void list_expr_sort(struct list_head *head)
{
	struct list_head *list;
	LIST_HEAD(temp);

	list = &temp;

	if (list_empty(head) || list_is_singular(head))
		return;

	list_cut_middle(list, head);

	list_expr_sort(head);
	list_expr_sort(list);

	list_splice_sorted(list, head);
}
