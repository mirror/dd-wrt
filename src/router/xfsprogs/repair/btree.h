// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2007 Silicon Graphics, Inc.
 * All Rights Reserved.
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
