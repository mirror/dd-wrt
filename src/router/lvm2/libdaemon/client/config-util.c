/*
 * Copyright (C) 2011-2012 Red Hat, Inc.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "tools/tool.h"

#include "daemon-io.h"
#include "device_mapper/misc/dm-logging.h"

#include <math.h>  /* fabs() */
#include <float.h> /* DBL_EPSILON */

int buffer_append_vf(struct buffer *buf, va_list ap)
{
	char *append;
	char *next;
	int keylen;
	int64_t value;
	char *string;
	char *block;

	while ((next = va_arg(ap, char *))) {
		append = NULL;
		if (!strchr(next, '=')) {
			log_error(INTERNAL_ERROR "Bad format string at '%s'", next);
			goto fail;
		}
		keylen = strchr(next, '=') - next;
		if (strstr(next, "%d")) {
                        /* Use of plain %d is prohibited, use  FMTd64 */
			log_error(INTERNAL_ERROR "Do not use  %%d and use correct 64bit form");
			goto fail;
		}
		if (strstr(next, FMTd64)) {
			value = va_arg(ap, int64_t);
			if (dm_asprintf(&append, "%.*s= %" PRId64 "\n", keylen, next, value) < 0)
				goto fail;
		} else if (strstr(next, "%s")) {
			string = va_arg(ap, char *);
			if (dm_asprintf(&append, "%.*s= \"%s\"\n", keylen, next, string) < 0)
				goto fail;
		} else if (strstr(next, "%b")) {
			if (!(block = va_arg(ap, char *)))
				continue;
			if (dm_asprintf(&append, "%.*s%s", keylen, next, block) < 0)
				goto fail;
		} else if (dm_asprintf(&append, "%s", next) < 0)
			goto fail;

		if (!append ||
		    !buffer_append(buf, append))
			goto fail;

		free(append);
	}

	return 1;
fail:
	free(append);
	return 0;
}

int buffer_append_f(struct buffer *buf, ...)
{
	int res;
	va_list ap;

	va_start(ap, buf);
	res = buffer_append_vf(buf, ap);
	va_end(ap);

	return res;
}

int set_flag(struct dm_config_tree *cft, struct dm_config_node *parent,
	     const char *field, const char *flag, int want)
{
	struct dm_config_value *value = NULL, *pred = NULL;
	struct dm_config_node *node = dm_config_find_node(parent->child, field);
	struct dm_config_value *new;

	if (node)
		value = node->v;

	while (value && value->type != DM_CFG_EMPTY_ARRAY && strcmp(value->v.str, flag)) {
		pred = value;
		value = value->next;
	}

	if (value && want)
		return 1;

	if (!value && !want)
		return 1;

	if (value && !want) {
		if (pred) {
			pred->next = value->next;
		} else if (value == node->v && value->next) {
			node->v = value->next;
		} else {
			node->v->type = DM_CFG_EMPTY_ARRAY;
		}
	}

	if (!value && want) {
		if (!node) {
			if (!(node = dm_config_create_node(cft, field)))
				return 0;
			node->sib = parent->child;
			if (!(node->v = dm_config_create_value(cft)))
				return 0;
			node->v->type = DM_CFG_EMPTY_ARRAY;
			node->parent = parent;
			parent->child = node;
		}
		if (!(new = dm_config_create_value(cft))) {
			/* FIXME error reporting */
			return 0;
		}
		new->type = DM_CFG_STRING;
		new->v.str = flag;
		new->next = node->v;
		node->v = new;
	}

	return 1;
}

void chain_node(struct dm_config_node *cn,
                struct dm_config_node *parent,
                struct dm_config_node *pre_sib)
{
	cn->parent = parent;
	cn->sib = NULL;

	if (parent && parent->child && !pre_sib) { /* find the last one */
		pre_sib = parent->child;
		while (pre_sib && pre_sib->sib)
			pre_sib = pre_sib->sib;
	}

	if (parent && !parent->child)
		parent->child = cn;
	if (pre_sib) {
		cn->sib = pre_sib->sib;
		pre_sib->sib = cn;
	}

}

struct dm_config_tree *config_tree_from_string_without_dup_node_check(const char *config_settings)
{
	struct dm_config_tree *cft;

	if (!(cft = dm_config_create()))
		return_NULL;

	if (!dm_config_parse_without_dup_node_check(cft, config_settings, config_settings + strlen(config_settings))) {
		dm_config_destroy(cft);
		return_NULL;
	}

	return cft;
}

struct dm_config_node *make_config_node(struct dm_config_tree *cft,
					const char *key,
					struct dm_config_node *parent,
					struct dm_config_node *pre_sib)
{
	struct dm_config_node *cn;

	if (!(cn = dm_config_create_node(cft, key)))
		return NULL;

	cn->v = NULL;
	cn->child = NULL;

	chain_node(cn, parent, pre_sib);

	return cn;
}

struct dm_config_node *make_text_node(struct dm_config_tree *cft,
				      const char *key,
				      const char *value,
				      struct dm_config_node *parent,
				      struct dm_config_node *pre_sib)
{
	struct dm_config_node *cn;

	if (!(cn = make_config_node(cft, key, parent, pre_sib)) ||
	    !(cn->v = dm_config_create_value(cft)))
		return NULL;

	cn->v->type = DM_CFG_STRING;
	cn->v->v.str = value;
	return cn;
}

struct dm_config_node *make_int_node(struct dm_config_tree *cft,
				     const char *key,
				     int64_t value,
				     struct dm_config_node *parent,
				     struct dm_config_node *pre_sib)
{
	struct dm_config_node *cn;

	if (!(cn = make_config_node(cft, key, parent, pre_sib)) ||
	    !(cn->v = dm_config_create_value(cft)))
		return NULL;

	cn->v->type = DM_CFG_INT;
	cn->v->v.i = value;
	return cn;
}

/*
 * FIXME: return 1 even if VA list is empty and return the
 *        dm_config_node* result as output parameter
 */
struct dm_config_node *config_make_nodes_v(struct dm_config_tree *cft,
					   struct dm_config_node *parent,
					   struct dm_config_node *pre_sib,
					   va_list ap)
{
	const char *next;
	struct dm_config_node *first = NULL;
	struct dm_config_node *cn;
	const char *fmt;
	char *key;

	while ((next = va_arg(ap, char *))) {
		cn = NULL;
		fmt = strchr(next, '=');

		if (!fmt) {
			log_error(INTERNAL_ERROR "Bad format string '%s'", fmt);
			return NULL;
		}

		if (!(key = dm_pool_strdup(cft->mem, next))) {
			log_error("Failed to duplicate node key.");
			return NULL;
		}

		key[fmt - next] = '\0';
		fmt += 2;

		if (!strcmp(fmt, FMTd64)) {
			int64_t value = va_arg(ap, int64_t);
			if (!(cn = make_int_node(cft, key, value, parent, pre_sib)))
				return 0;
		} else if (!strcmp(fmt, "%s")) {
			char *value = va_arg(ap, char *);
			if (!(cn = make_text_node(cft, key, value, parent, pre_sib)))
				return 0;
		} else if (!strcmp(fmt, "%t")) {
			struct dm_config_tree *tree = va_arg(ap, struct dm_config_tree *);
			cn = dm_config_clone_node(cft, tree->root, 1);
			if (!cn)
				return 0;
			cn->key = key;
			chain_node(cn, parent, pre_sib);
		} else {
			log_error(INTERNAL_ERROR "Bad format string '%s'", fmt);
			return NULL;
		}
		if (!first)
			first = cn;
		if (cn)
			pre_sib = cn;
	}

	return first;
}

struct dm_config_node *config_make_nodes(struct dm_config_tree *cft,
					 struct dm_config_node *parent,
					 struct dm_config_node *pre_sib,
					 ...)
{
	struct dm_config_node *res;
	va_list ap;

	va_start(ap, pre_sib);
	res = config_make_nodes_v(cft, parent, pre_sib, ap);
	va_end(ap);

	return res;
}

/* Test if the doubles are close enough to be considered equal */
static int _close_enough(double d1, double d2)
{
	return fabs(d1 - d2) < DBL_EPSILON;
}

int compare_value(struct dm_config_value *a, struct dm_config_value *b)
{
	int r = 0;

	if (a->type > b->type)
		return 1;
	if (a->type < b->type)
		return -1;

	switch (a->type) {
	case DM_CFG_STRING: r = strcmp(a->v.str, b->v.str); break;
	case DM_CFG_FLOAT: r = _close_enough(a->v.f, b->v.f) ? 0 : (a->v.f > b->v.f) ? 1 : -1; break;
	case DM_CFG_INT: r = (a->v.i == b->v.i) ? 0 : (a->v.i > b->v.i) ? 1 : -1; break;
	case DM_CFG_EMPTY_ARRAY: return 0;
	}

	if (r == 0 && a->next && b->next)
		r = compare_value(a->next, b->next);
	return r;
}

int compare_config(struct dm_config_node *a, struct dm_config_node *b)
{
	int result = 0;
	if (a->v && b->v)
		result = compare_value(a->v, b->v);
	if (a->v && !b->v)
		result = 1;
	if (!a->v && b->v)
		result = -1;
	if (a->child && b->child)
		result = compare_config(a->child, b->child);

	if (result) {
		// DEBUGLOG("config inequality at %s / %s", a->key, b->key);
		return result;
	}

	if (a->sib && b->sib)
		result = compare_config(a->sib, b->sib);
	if (a->sib && !b->sib)
		result = 1;
	if (!a->sib && b->sib)
		result = -1;

	return result;
}

int buffer_realloc(struct buffer *buf, int needed)
{
	char *new;
	int alloc = buf->allocated;
	if (alloc < needed)
		alloc = needed;

	buf->allocated += alloc;
	new = realloc(buf->mem, buf->allocated);
	if (new)
		buf->mem = new;
	else { /* utter failure */
		free(buf->mem);
		buf->mem = 0;
		buf->allocated = buf->used = 0;
		return 0;
	}
	return 1;
}

int buffer_append(struct buffer *buf, const char *string)
{
	int len = strlen(string);

	if ((!buf->mem || (buf->allocated - buf->used <= len)) &&
	    !buffer_realloc(buf, len + 1))
		return 0;

	strcpy(buf->mem + buf->used, string);
	buf->used += len;
	return 1;
}

int buffer_line(const char *line, void *baton)
{
	struct buffer *buf = baton;
	if (!buffer_append(buf, line))
		return 0;
	if (!buffer_append(buf, "\n"))
		return 0;
	return 1;
}

void buffer_destroy(struct buffer *buf)
{
	free(buf->mem);
	buffer_init(buf);
}

void buffer_init(struct buffer *buf)
{
	buf->allocated = buf->used = 0;
	buf->mem = 0;
}
