// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#ifndef __LIBFROG_BITMAP_H__
#define __LIBFROG_BITMAP_H__

struct bitmap {
	pthread_mutex_t		bt_lock;
	struct avl64tree_desc	*bt_tree;
};

int bitmap_alloc(struct bitmap **bmap);
void bitmap_free(struct bitmap **bmap);
int bitmap_set(struct bitmap *bmap, uint64_t start, uint64_t length);
int bitmap_iterate(struct bitmap *bmap, int (*fn)(uint64_t, uint64_t, void *),
		void *arg);
int bitmap_iterate_range(struct bitmap *bmap, uint64_t start, uint64_t length,
		int (*fn)(uint64_t, uint64_t, void *), void *arg);
bool bitmap_test(struct bitmap *bmap, uint64_t start,
		uint64_t len);
bool bitmap_empty(struct bitmap *bmap);
void bitmap_dump(struct bitmap *bmap);

#endif /* __LIBFROG_BITMAP_H__ */
