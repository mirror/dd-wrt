/*
 * Copyright (C) 2003-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2012 Red Hat, Inc. All rights reserved.
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

#include "lib/misc/lib.h"
#include "lib/datastruct/str_list.h"

struct dm_list *str_list_create(struct dm_pool *mem)
{
	struct dm_list *sl;

	if (!(sl = dm_pool_alloc(mem, sizeof(struct dm_list)))) {
		log_errno(ENOMEM, "str_list allocation failed");
		return NULL;
	}

	dm_list_init(sl);

	return sl;
}

static int _str_list_add_no_dup_check(struct dm_pool *mem, struct dm_list *sll, const char *str, int as_first)
{
	struct dm_str_list *sln;

	if (!str)
		return_0;

	if (!(sln = dm_pool_alloc(mem, sizeof(*sln))))
		return_0;

	sln->str = str;
	if (as_first)
		dm_list_add_h(sll, &sln->list);
	else
		dm_list_add(sll, &sln->list);

	return 1;
}

int str_list_add_no_dup_check(struct dm_pool *mem, struct dm_list *sll, const char *str)
{
	return _str_list_add_no_dup_check(mem, sll, str, 0);
}

int str_list_add_h_no_dup_check(struct dm_pool *mem, struct dm_list *sll, const char *str)
{
	return _str_list_add_no_dup_check(mem, sll, str, 1);
}

int str_list_add(struct dm_pool *mem, struct dm_list *sll, const char *str)
{
	if (!str)
		return_0;

	/* Already in list? */
	if (str_list_match_item(sll, str))
		return 1;

	return str_list_add_no_dup_check(mem, sll, str);
}

/* Add contents of sll2 to sll */
int str_list_add_list(struct dm_pool *mem, struct dm_list *sll, struct dm_list *sll2)
{
	struct dm_str_list *sl;

	if (!sll2)
		return_0;

	dm_list_iterate_items(sl, sll2)
		if (!str_list_add(mem, sll, sl->str))
			return_0;

	return 1;
}

void str_list_del(struct dm_list *sll, const char *str)
{
	struct dm_list *slh, *slht;

	dm_list_iterate_safe(slh, slht, sll)
		if (!strcmp(str, dm_list_item(slh, struct dm_str_list)->str))
			dm_list_del(slh);
}

void str_list_wipe(struct dm_list *sll)
{
	struct dm_list *slh, *slht;

	dm_list_iterate_safe(slh, slht, sll)
		dm_list_del(slh);
}

int str_list_dup(struct dm_pool *mem, struct dm_list *sllnew,
		 const struct dm_list *sllold)
{
	struct dm_str_list *sl;

	dm_list_init(sllnew);

	dm_list_iterate_items(sl, sllold) {
		if (!str_list_add(mem, sllnew, dm_pool_strdup(mem, sl->str)))
			return_0;
	}

	return 1;
}

/*
 * Is item on list?
 */
int str_list_match_item(const struct dm_list *sll, const char *str)
{
	struct dm_str_list *sl;

	dm_list_iterate_items(sl, sll)
		if (!strcmp(str, sl->str))
			return 1;

	return 0;
}

/*
 * Is at least one item on both lists?
 * If tag_matched is non-NULL, it is set to the tag that matched.
 */
int str_list_match_list(const struct dm_list *sll, const struct dm_list *sll2, const char **tag_matched)
{
	struct dm_str_list *sl;

	dm_list_iterate_items(sl, sll)
		if (str_list_match_item(sll2, sl->str)) {
			if (tag_matched)
				*tag_matched = sl->str;
			return 1;
		}

	return 0;
}

/*
 * Do both lists contain the same set of items?
 */
int str_list_lists_equal(const struct dm_list *sll, const struct dm_list *sll2)
{
	struct dm_str_list *sl;

	if (dm_list_size(sll) != dm_list_size(sll2))
		return 0;

	dm_list_iterate_items(sl, sll)
		if (!str_list_match_item(sll2, sl->str))
			return 0;

	return 1;
}

char *str_list_to_str(struct dm_pool *mem, const struct dm_list *list,
		      const char *delim)
{
	size_t delim_len = strlen(delim);
	unsigned list_size = dm_list_size(list);
	struct dm_str_list *sl;
	char *str, *p;
	size_t len = 0;
	unsigned i = 0;

	dm_list_iterate_items(sl, list)
		len += strlen(sl->str);
	if (list_size > 1)
		len += ((list_size - 1) * delim_len);

	str = dm_pool_alloc(mem, len+1);
	if (!str) {
		log_error("str_list_to_str: string allocation failed.");
		return NULL;
	}
	str[len] = '\0';
	p = str;

	dm_list_iterate_items(sl, list) {
		len = strlen(sl->str);
		memcpy(p, sl->str, len);
		p += len;

		if (++i != list_size) {
			memcpy(p, delim, delim_len);
			p += delim_len;
		}
	}

	return str;
}

struct dm_list *str_to_str_list(struct dm_pool *mem, const char *str,
				const char *delim, int ignore_multiple_delim)
{
	size_t delim_len = strlen(delim);
	struct dm_list *list;
	const char *p1, *p2, *next;
	char *str_item;
	size_t len;

	if (!(list = str_list_create(mem))) {
		log_error("str_to_str_list: string list allocation failed.");
		return NULL;
	}

	p1 = p2 = str;
	while (*p1) {
		if (!(p2 = strstr(p1, delim)))
			next = p2 = str + strlen(str);
		else
			next = p2 + delim_len;

		len = p2 - p1;
		str_item = dm_pool_alloc(mem, len+1);
		if (!str_item) {
			log_error("str_to_str_list: string list item allocation failed.");
			goto bad;
		}
		memcpy(str_item, p1, len);
		str_item[len] = '\0';

		if (!str_list_add_no_dup_check(mem, list, str_item))
			goto_bad;

		if (ignore_multiple_delim) {
			while (!strncmp(next, delim, delim_len))
				next += delim_len;
		}

		p1 = next;
	}

	return list;
bad:
	dm_pool_free(mem, list);

	return NULL;
}
