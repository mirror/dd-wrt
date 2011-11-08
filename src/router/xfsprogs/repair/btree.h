/*
 * Copyright (c) 2007 Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _BTREE_H
#define _BTREE_H


struct btree_root;

void
btree_init(
	struct btree_root	**root);

void
btree_destroy(
	struct btree_root	*root);

int
btree_is_empty(
	struct btree_root	*root);

void *
btree_lookup(
	struct btree_root	*root,
	unsigned long		key);

void *
btree_find(
	struct btree_root	*root,
	unsigned long		key,
	unsigned long		*actual_key);

void *
btree_peek_prev(
	struct btree_root	*root,
	unsigned long		*key);

void *
btree_peek_next(
	struct btree_root	*root,
	unsigned long		*key);

void *
btree_lookup_next(
	struct btree_root	*root,
	unsigned long		*key);

void *
btree_lookup_prev(
	struct btree_root	*root,
	unsigned long		*key);

int
btree_insert(
	struct btree_root	*root,
	unsigned long		key,
	void			*value);

void *
btree_delete(
	struct btree_root	*root,
	unsigned long		key);

int
btree_update_key(
	struct btree_root	*root,
	unsigned long		old_key,
	unsigned long		new_key);

int
btree_update_value(
	struct btree_root	*root,
	unsigned long		key,
	void 			*new_value);

void
btree_clear(
	struct btree_root	*root);

#ifdef BTREE_STATS
void
btree_print_stats(
	struct btree_root	*root,
	FILE			*f);
#endif

#endif /* _BTREE_H */
