/*
 * Copyright (c) 2013-2024 Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 (or any
 * later) as published by the Free Software Foundation.
 */

#include <nft.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <utils.h>

#include "list.h"
#include "parser.h"
#include "erec.h"

struct str_buf {
	uint8_t		*str;
	uint32_t	len;
	uint32_t	size;
};

#define STR_BUF_LEN	128

static struct str_buf *str_buf_alloc(void)
{
	struct str_buf *buf;

	buf = xzalloc(sizeof(*buf));
	buf->str = xzalloc_array(1, STR_BUF_LEN);
	buf->size = STR_BUF_LEN;

	return buf;
}

static int str_buf_add(struct str_buf *buf, const char *str, uint32_t len)
{
	uint8_t *tmp;

	if (len + buf->len > buf->size) {
		buf->size = (len + buf->len) * 2;
		tmp = xrealloc(buf->str, buf->size);
		buf->str = tmp;
	}

	memcpy(&buf->str[buf->len], str, len);
	buf->len += len;

	return 0;
}

struct str_chunk {
	struct list_head	list;
	char			*str;
	uint32_t		len;
	bool			is_sym;
};

static void add_str_chunk(const char *x, int from, int to, struct list_head *list, bool is_sym)
{
	struct str_chunk *chunk;
	int len = to - from;

	chunk = xzalloc_array(1, sizeof(*chunk));
	chunk->str = xzalloc_array(1, len + 1);
	chunk->is_sym = is_sym;
	chunk->len = len;
	memcpy(chunk->str, &x[from], len);

	list_add_tail(&chunk->list, list);
}

static void free_str_chunk(struct str_chunk *chunk)
{
	free(chunk->str);
	free(chunk);
}

const char *str_preprocess(struct parser_state *state, struct location *loc,
			   struct scope *scope, const char *x,
			   struct error_record **erec)
{
	struct str_chunk *chunk, *next;
	struct str_buf *buf;
	const char *str;
	int i, j, start;
	LIST_HEAD(list);

	start = 0;
	i = 0;
	while (1) {
		if (x[i] == '\0') {
			i++;
			break;
		}

		if (x[i] != '$') {
			i++;
			continue;
		}

		if (isdigit(x[++i]))
			continue;

		j = i;
		while (1) {
			if (isalpha(x[i]) ||
			    isdigit(x[i]) ||
			    x[i] == '_') {
				i++;
				continue;
			}
			break;
		}
		add_str_chunk(x, start, j-1, &list, false);
		add_str_chunk(x, j, i, &list, true);
		start = i;
	}
	if (start != i)
		add_str_chunk(x, start, i, &list, false);

	buf = str_buf_alloc();

	list_for_each_entry_safe(chunk, next, &list, list) {
		if (chunk->is_sym) {
			struct symbol *sym;

			sym = symbol_lookup(scope, chunk->str);
			if (!sym) {
				sym = symbol_lookup_fuzzy(scope, chunk->str);
				if (sym) {
					*erec = error(loc, "unknown identifier '%s'; "
							   "did you mean identifier '%s'?",
					              chunk->str, sym->identifier);
				} else {
					*erec = error(loc, "unknown identifier '%s'",
						      chunk->str);
				}
				goto err;
			}
			str_buf_add(buf, sym->expr->identifier,
				    strlen(sym->expr->identifier));
		} else {
			str_buf_add(buf, chunk->str, chunk->len);
		}
		list_del(&chunk->list);
		free_str_chunk(chunk);
	}

	str = (char *)buf->str;

	free(buf);

	return (char *)str;
err:
	list_for_each_entry_safe(chunk, next, &list, list) {
		list_del(&chunk->list);
		free_str_chunk(chunk);
	}
	free(buf->str);
	free(buf);

	return NULL;
}
