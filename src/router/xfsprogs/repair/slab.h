// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#ifndef SLAB_H_
#define SLAB_H_

struct xfs_slab;
struct xfs_slab_cursor;

extern int init_slab(struct xfs_slab **, size_t);
extern void free_slab(struct xfs_slab **);

extern int slab_add(struct xfs_slab *, void *);
extern void qsort_slab(struct xfs_slab *, int (*)(const void *, const void *));
extern size_t slab_count(struct xfs_slab *);

extern int init_slab_cursor(struct xfs_slab *,
	int (*)(const void *, const void *), struct xfs_slab_cursor **);
extern void free_slab_cursor(struct xfs_slab_cursor **);

extern void *peek_slab_cursor(struct xfs_slab_cursor *);
extern void advance_slab_cursor(struct xfs_slab_cursor *);
extern void *pop_slab_cursor(struct xfs_slab_cursor *);

struct xfs_bag;

extern int init_bag(struct xfs_bag **);
extern void free_bag(struct xfs_bag **);
extern int bag_add(struct xfs_bag *, void *);
extern int bag_remove(struct xfs_bag *, size_t);
extern size_t bag_count(struct xfs_bag *);
extern void *bag_item(struct xfs_bag *, size_t);

#define foreach_bag_ptr(bag, idx, ptr) \
	for ((idx) = 0, (ptr) = bag_item((bag), (idx)); \
	     (idx) < bag_count(bag); \
	     (idx)++, (ptr) = bag_item((bag), (idx)))

#define foreach_bag_ptr_reverse(bag, idx, ptr) \
	for ((idx) = bag_count(bag) - 1, (ptr) = bag_item((bag), (idx)); \
	     (ptr) != NULL; \
	     (idx)--, (ptr) = bag_item((bag), (idx)))

#endif /* SLAB_H_ */
