// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#include "xfs.h"
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include "platform_defs.h"
#include "avl64.h"
#include "list.h"
#include "bitmap.h"

/*
 * Space Efficient Bitmap
 *
 * Implements a space-efficient bitmap.  We use an AVL tree to manage
 * extent records that tell us which ranges are set; the bitmap key is
 * an arbitrary uint64_t.  The usual bitmap operations (set, clear,
 * test, test and set) are supported, plus we can iterate set ranges.
 */

#define avl_for_each_range_safe(pos, n, l, first, last) \
	for (pos = (first), n = pos->avl_nextino, l = (last)->avl_nextino; \
			pos != (l); \
			pos = n, n = pos ? pos->avl_nextino : NULL)

#define avl_for_each_safe(tree, pos, n) \
	for (pos = (tree)->avl_firstino, n = pos ? pos->avl_nextino : NULL; \
			pos != NULL; \
			pos = n, n = pos ? pos->avl_nextino : NULL)

#define avl_for_each(tree, pos) \
	for (pos = (tree)->avl_firstino; pos != NULL; pos = pos->avl_nextino)

struct bitmap_node {
	struct avl64node	btn_node;
	uint64_t		btn_start;
	uint64_t		btn_length;
};

static uint64_t
extent_start(
	struct avl64node	*node)
{
	struct bitmap_node	*btn;

	btn = container_of(node, struct bitmap_node, btn_node);
	return btn->btn_start;
}

static uint64_t
extent_end(
	struct avl64node	*node)
{
	struct bitmap_node	*btn;

	btn = container_of(node, struct bitmap_node, btn_node);
	return btn->btn_start + btn->btn_length;
}

static struct avl64ops bitmap_ops = {
	extent_start,
	extent_end,
};

/* Initialize a bitmap. */
int
bitmap_alloc(
	struct bitmap		**bmapp)
{
	struct bitmap		*bmap;
	int			ret;

	bmap = calloc(1, sizeof(struct bitmap));
	if (!bmap)
		return -errno;
	bmap->bt_tree = malloc(sizeof(struct avl64tree_desc));
	if (!bmap->bt_tree) {
		ret = -errno;
		goto out;
	}

	ret = -pthread_mutex_init(&bmap->bt_lock, NULL);
	if (ret)
		goto out_tree;

	avl64_init_tree(bmap->bt_tree, &bitmap_ops);
	*bmapp = bmap;

	return 0;
out_tree:
	free(bmap->bt_tree);
out:
	free(bmap);
	return ret;
}

/* Free a bitmap. */
void
bitmap_free(
	struct bitmap		**bmapp)
{
	struct bitmap		*bmap;
	struct avl64node	*node;
	struct avl64node	*n;
	struct bitmap_node	*ext;

	bmap = *bmapp;
	avl_for_each_safe(bmap->bt_tree, node, n) {
		ext = container_of(node, struct bitmap_node, btn_node);
		free(ext);
	}
	free(bmap->bt_tree);
	free(bmap);
	*bmapp = NULL;
}

/* Create a new bitmap extent node. */
static struct bitmap_node *
bitmap_node_init(
	uint64_t		start,
	uint64_t		len)
{
	struct bitmap_node	*ext;

	ext = malloc(sizeof(struct bitmap_node));
	if (!ext)
		return NULL;

	ext->btn_node.avl_nextino = NULL;
	ext->btn_start = start;
	ext->btn_length = len;

	return ext;
}

/* Create a new bitmap node and insert it. */
static inline int
__bitmap_insert(
	struct bitmap		*bmap,
	uint64_t		start,
	uint64_t		length)
{
	struct bitmap_node	*ext;
	struct avl64node	*node;

	ext = bitmap_node_init(start, length);
	if (!ext)
		return -errno;

	node = avl64_insert(bmap->bt_tree, &ext->btn_node);
	if (node == NULL) {
		free(ext);
		return -EEXIST;
	}

	return 0;
}

/* Set a region of bits (locked). */
static int
__bitmap_set(
	struct bitmap		*bmap,
	uint64_t		start,
	uint64_t		length)
{
	struct avl64node	*firstn;
	struct avl64node	*lastn;
	struct avl64node	*pos;
	struct avl64node	*n;
	struct avl64node	*l;
	struct bitmap_node	*ext;
	uint64_t		new_start;
	uint64_t		new_length;

	/* Find any existing nodes adjacent or within that range. */
	avl64_findranges(bmap->bt_tree, start - 1, start + length + 1,
			&firstn, &lastn);

	/* Nothing, just insert a new extent. */
	if (firstn == NULL && lastn == NULL)
		return __bitmap_insert(bmap, start, length);

	assert(firstn != NULL && lastn != NULL);
	new_start = start;
	new_length = length;

	avl_for_each_range_safe(pos, n, l, firstn, lastn) {
		ext = container_of(pos, struct bitmap_node, btn_node);

		/* Bail if the new extent is contained within an old one. */
		if (ext->btn_start <= start &&
		    ext->btn_start + ext->btn_length >= start + length)
			return 0;

		/* Check for overlapping and adjacent extents. */
		if (ext->btn_start + ext->btn_length >= start ||
		    ext->btn_start <= start + length) {
			if (ext->btn_start < start) {
				new_start = ext->btn_start;
				new_length += ext->btn_length;
			}

			if (ext->btn_start + ext->btn_length >
			    new_start + new_length)
				new_length = ext->btn_start + ext->btn_length -
						new_start;

			avl64_delete(bmap->bt_tree, pos);
			free(ext);
		}
	}

	return __bitmap_insert(bmap, new_start, new_length);
}

/* Set a region of bits. */
int
bitmap_set(
	struct bitmap		*bmap,
	uint64_t		start,
	uint64_t		length)
{
	int			res;

	pthread_mutex_lock(&bmap->bt_lock);
	res = __bitmap_set(bmap, start, length);
	pthread_mutex_unlock(&bmap->bt_lock);

	return res;
}

#if 0	/* Unused, provided for completeness. */
/* Clear a region of bits. */
int
bitmap_clear(
	struct bitmap		*bmap,
	uint64_t		start,
	uint64_t		len)
{
	struct avl64node	*firstn;
	struct avl64node	*lastn;
	struct avl64node	*pos;
	struct avl64node	*n;
	struct avl64node	*l;
	struct bitmap_node	*ext;
	uint64_t		new_start;
	uint64_t		new_length;
	struct avl64node	*node;
	int			stat;

	pthread_mutex_lock(&bmap->bt_lock);
	/* Find any existing nodes over that range. */
	avl64_findranges(bmap->bt_tree, start, start + len, &firstn, &lastn);

	/* Nothing, we're done. */
	if (firstn == NULL && lastn == NULL) {
		pthread_mutex_unlock(&bmap->bt_lock);
		return 0;
	}

	assert(firstn != NULL && lastn != NULL);

	/* Delete or truncate everything in sight. */
	avl_for_each_range_safe(pos, n, l, firstn, lastn) {
		ext = container_of(pos, struct bitmap_node, btn_node);

		stat = 0;
		if (ext->btn_start < start)
			stat |= 1;
		if (ext->btn_start + ext->btn_length > start + len)
			stat |= 2;
		switch (stat) {
		case 0:
			/* Extent totally within range; delete. */
			avl64_delete(bmap->bt_tree, pos);
			free(ext);
			break;
		case 1:
			/* Extent is left-adjacent; truncate. */
			ext->btn_length = start - ext->btn_start;
			break;
		case 2:
			/* Extent is right-adjacent; move it. */
			ext->btn_length = ext->btn_start + ext->btn_length -
					(start + len);
			ext->btn_start = start + len;
			break;
		case 3:
			/* Extent overlaps both ends. */
			ext->btn_length = start - ext->btn_start;
			new_start = start + len;
			new_length = ext->btn_start + ext->btn_length -
					new_start;

			ext = bitmap_node_init(new_start, new_length);
			if (!ext) {
				ret = -errno;
				goto out;
			}

			node = avl64_insert(bmap->bt_tree, &ext->btn_node);
			if (node == NULL) {
				ret = -EEXIST;
				goto out;
			}
			break;
		}
	}

out:
	pthread_mutex_unlock(&bmap->bt_lock);
	return ret;
}
#endif

/* Iterate the set regions of this bitmap. */
int
bitmap_iterate(
	struct bitmap		*bmap,
	int			(*fn)(uint64_t, uint64_t, void *),
	void			*arg)
{
	struct avl64node	*node;
	struct bitmap_node	*ext;
	int			error = 0;

	pthread_mutex_lock(&bmap->bt_lock);
	avl_for_each(bmap->bt_tree, node) {
		ext = container_of(node, struct bitmap_node, btn_node);
		error = fn(ext->btn_start, ext->btn_length, arg);
		if (error)
			break;
	}
	pthread_mutex_unlock(&bmap->bt_lock);

	return error;
}

/* Iterate the set regions of part of this bitmap. */
int
bitmap_iterate_range(
	struct bitmap		*bmap,
	uint64_t		start,
	uint64_t		length,
	int			(*fn)(uint64_t, uint64_t, void *),
	void			*arg)
{
	struct avl64node	*firstn;
	struct avl64node	*lastn;
	struct avl64node	*pos;
	struct avl64node	*n;
	struct avl64node	*l;
	struct bitmap_node	*ext;
	int			ret = 0;

	pthread_mutex_lock(&bmap->bt_lock);

	avl64_findranges(bmap->bt_tree, start, start + length, &firstn,
			&lastn);

	if (firstn == NULL && lastn == NULL)
		goto out;

	avl_for_each_range_safe(pos, n, l, firstn, lastn) {
		ext = container_of(pos, struct bitmap_node, btn_node);
		ret = fn(ext->btn_start, ext->btn_length, arg);
		if (ret)
			break;
	}

out:
	pthread_mutex_unlock(&bmap->bt_lock);
	return ret;
}

/* Do any bitmap extents overlap the given one?  (locked) */
static bool
__bitmap_test(
	struct bitmap		*bmap,
	uint64_t		start,
	uint64_t		len)
{
	struct avl64node	*firstn;
	struct avl64node	*lastn;

	/* Find any existing nodes over that range. */
	avl64_findranges(bmap->bt_tree, start, start + len, &firstn, &lastn);

	return firstn != NULL && lastn != NULL;
}

/* Is any part of this range set? */
bool
bitmap_test(
	struct bitmap		*bmap,
	uint64_t		start,
	uint64_t		len)
{
	bool			res;

	pthread_mutex_lock(&bmap->bt_lock);
	res = __bitmap_test(bmap, start, len);
	pthread_mutex_unlock(&bmap->bt_lock);

	return res;
}

/* Are none of the bits set? */
bool
bitmap_empty(
	struct bitmap		*bmap)
{
	return bmap->bt_tree->avl_firstino == NULL;
}

#ifdef DEBUG
static int
bitmap_dump_fn(
	uint64_t		startblock,
	uint64_t		blockcount,
	void			*arg)
{
	printf("%"PRIu64":%"PRIu64"\n", startblock, blockcount);
	return 0;
}

/* Dump bitmap. */
void
bitmap_dump(
	struct bitmap		*bmap)
{
	printf("BITMAP DUMP %p\n", bmap);
	bitmap_iterate(bmap, bitmap_dump_fn, NULL);
	printf("BITMAP DUMP DONE\n");
}
#endif
