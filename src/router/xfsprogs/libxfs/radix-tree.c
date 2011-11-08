/*
 * Copyright (C) 2001 Momchil Velikov
 * Portions Copyright (C) 2001 Christoph Hellwig
 * Copyright (C) 2005 SGI, Christoph Lameter <clameter@sgi.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <libxfs.h>
#include "radix-tree.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

#define RADIX_TREE_MAP_SHIFT	6
#define RADIX_TREE_MAP_SIZE	(1UL << RADIX_TREE_MAP_SHIFT)
#define RADIX_TREE_MAP_MASK	(RADIX_TREE_MAP_SIZE-1)

#ifdef RADIX_TREE_TAGS
#define RADIX_TREE_TAG_LONGS	\
	((RADIX_TREE_MAP_SIZE + BITS_PER_LONG - 1) / BITS_PER_LONG)
#endif

struct radix_tree_node {
	unsigned int	count;
	void		*slots[RADIX_TREE_MAP_SIZE];
#ifdef RADIX_TREE_TAGS
	unsigned long	tags[RADIX_TREE_MAX_TAGS][RADIX_TREE_TAG_LONGS];
#endif
};

struct radix_tree_path {
	struct radix_tree_node *node;
	int offset;
};

#define RADIX_TREE_INDEX_BITS  (8 /* CHAR_BIT */ * sizeof(unsigned long))
#define RADIX_TREE_MAX_PATH (RADIX_TREE_INDEX_BITS/RADIX_TREE_MAP_SHIFT + 2)

static unsigned long height_to_maxindex[RADIX_TREE_MAX_PATH];

/*
 * Radix tree node cache.
 */

#define radix_tree_node_alloc(r) 	((struct radix_tree_node *) \
		calloc(1, sizeof(struct radix_tree_node)))
#define radix_tree_node_free(n) 	free(n)

#ifdef RADIX_TREE_TAGS

static inline void tag_set(struct radix_tree_node *node, unsigned int tag,
		int offset)
{
	*((__uint32_t *)node->tags[tag] + (offset >> 5)) |= (1 << (offset & 31));
}

static inline void tag_clear(struct radix_tree_node *node, unsigned int tag,
		int offset)
{
	__uint32_t 	*p = (__uint32_t*)node->tags[tag] + (offset >> 5);
	__uint32_t 	m = 1 << (offset & 31);
	*p &= ~m;
}

static inline int tag_get(struct radix_tree_node *node, unsigned int tag,
		int offset)
{
	return 1 & (((const __uint32_t *)node->tags[tag])[offset >> 5] >> (offset & 31));
}

/*
 * Returns 1 if any slot in the node has this tag set.
 * Otherwise returns 0.
 */
static inline int any_tag_set(struct radix_tree_node *node, unsigned int tag)
{
	int idx;
	for (idx = 0; idx < RADIX_TREE_TAG_LONGS; idx++) {
		if (node->tags[tag][idx])
			return 1;
	}
	return 0;
}

#endif

/*
 *	Return the maximum key which can be store into a
 *	radix tree with height HEIGHT.
 */
static inline unsigned long radix_tree_maxindex(unsigned int height)
{
	return height_to_maxindex[height];
}

/*
 *	Extend a radix tree so it can store key @index.
 */
static int radix_tree_extend(struct radix_tree_root *root, unsigned long index)
{
	struct radix_tree_node *node;
	unsigned int height;
#ifdef RADIX_TREE_TAGS
	char tags[RADIX_TREE_MAX_TAGS];
	int tag;
#endif

	/* Figure out what the height should be.  */
	height = root->height + 1;
	while (index > radix_tree_maxindex(height))
		height++;

	if (root->rnode == NULL) {
		root->height = height;
		goto out;
	}

#ifdef RADIX_TREE_TAGS
	/*
	 * Prepare the tag status of the top-level node for propagation
	 * into the newly-pushed top-level node(s)
	 */
	for (tag = 0; tag < RADIX_TREE_MAX_TAGS; tag++) {
		tags[tag] = 0;
		if (any_tag_set(root->rnode, tag))
			tags[tag] = 1;
	}
#endif
	do {
		if (!(node = radix_tree_node_alloc(root)))
			return -ENOMEM;

		/* Increase the height.  */
		node->slots[0] = root->rnode;

#ifdef RADIX_TREE_TAGS
		/* Propagate the aggregated tag info into the new root */
		for (tag = 0; tag < RADIX_TREE_MAX_TAGS; tag++) {
			if (tags[tag])
				tag_set(node, tag, 0);
		}
#endif
		node->count = 1;
		root->rnode = node;
		root->height++;
	} while (height > root->height);
out:
	return 0;
}

/**
 *	radix_tree_insert    -    insert into a radix tree
 *	@root:		radix tree root
 *	@index:		index key
 *	@item:		item to insert
 *
 *	Insert an item into the radix tree at position @index.
 */
int radix_tree_insert(struct radix_tree_root *root,
			unsigned long index, void *item)
{
	struct radix_tree_node *node = NULL, *slot;
	unsigned int height, shift;
	int offset;
	int error;

	/* Make sure the tree is high enough.  */
	if ((!index && !root->rnode) ||
			index > radix_tree_maxindex(root->height)) {
		error = radix_tree_extend(root, index);
		if (error)
			return error;
	}

	slot = root->rnode;
	height = root->height;
	shift = (height-1) * RADIX_TREE_MAP_SHIFT;

	offset = 0;			/* uninitialised var warning */
	do {
		if (slot == NULL) {
			/* Have to add a child node.  */
			if (!(slot = radix_tree_node_alloc(root)))
				return -ENOMEM;
			if (node) {
				node->slots[offset] = slot;
				node->count++;
			} else
				root->rnode = slot;
		}

		/* Go a level down */
		offset = (index >> shift) & RADIX_TREE_MAP_MASK;
		node = slot;
		slot = node->slots[offset];
		shift -= RADIX_TREE_MAP_SHIFT;
		height--;
	} while (height > 0);

	if (slot != NULL)
		return -EEXIST;

	ASSERT(node);
	node->count++;
	node->slots[offset] = item;
#ifdef RADIX_TREE_TAGS
	ASSERT(!tag_get(node, 0, offset));
	ASSERT(!tag_get(node, 1, offset));
#endif
	return 0;
}

static inline void **__lookup_slot(struct radix_tree_root *root,
				   unsigned long index)
{
	unsigned int height, shift;
	struct radix_tree_node **slot;

	height = root->height;
	if (index > radix_tree_maxindex(height))
		return NULL;

	shift = (height-1) * RADIX_TREE_MAP_SHIFT;
	slot = &root->rnode;

	while (height > 0) {
		if (*slot == NULL)
			return NULL;

		slot = (struct radix_tree_node **)
			((*slot)->slots +
				((index >> shift) & RADIX_TREE_MAP_MASK));
		shift -= RADIX_TREE_MAP_SHIFT;
		height--;
	}

	return (void **)slot;
}

/**
 *	radix_tree_lookup_slot    -    lookup a slot in a radix tree
 *	@root:		radix tree root
 *	@index:		index key
 *
 *	Lookup the slot corresponding to the position @index in the radix tree
 *	@root. This is useful for update-if-exists operations.
 */
void **radix_tree_lookup_slot(struct radix_tree_root *root, unsigned long index)
{
	return __lookup_slot(root, index);
}

/**
 *	radix_tree_lookup    -    perform lookup operation on a radix tree
 *	@root:		radix tree root
 *	@index:		index key
 *
 *	Lookup the item at the position @index in the radix tree @root.
 */
void *radix_tree_lookup(struct radix_tree_root *root, unsigned long index)
{
	void **slot;

	slot = __lookup_slot(root, index);
	return slot != NULL ? *slot : NULL;
}

/**
 *	raid_tree_first_key - find the first index key in the radix tree
 *	@root:		radix tree root
 *	@index:		where the first index will be placed
 *
 *	Returns the first entry and index key in the radix tree @root.
 */
void *radix_tree_lookup_first(struct radix_tree_root *root, unsigned long *index)
{
	unsigned int height, shift;
	struct radix_tree_node *slot;
	unsigned long i;

	height = root->height;
	*index = 0;
	if (height == 0)
		return NULL;

	shift = (height-1) * RADIX_TREE_MAP_SHIFT;
	slot = root->rnode;

	for (; height > 1; height--) {
		for (i = 0; i < RADIX_TREE_MAP_SIZE; i++) {
			if (slot->slots[i] != NULL)
				break;
		}
		ASSERT(i < RADIX_TREE_MAP_SIZE);

		*index |= (i << shift);
		shift -= RADIX_TREE_MAP_SHIFT;
		slot = slot->slots[i];
	}
	for (i = 0; i < RADIX_TREE_MAP_SIZE; i++) {
		if (slot->slots[i] != NULL) {
			*index |= i;
			return slot->slots[i];
		}
	}
	return NULL;
}

#ifdef RADIX_TREE_TAGS

/**
 *	radix_tree_tag_set - set a tag on a radix tree node
 *	@root:		radix tree root
 *	@index:		index key
 *	@tag: 		tag index
 *
 *	Set the search tag (which must be < RADIX_TREE_MAX_TAGS)
 *	corresponding to @index in the radix tree.  From
 *	the root all the way down to the leaf node.
 *
 *	Returns the address of the tagged item.   Setting a tag on a not-present
 *	item is a bug.
 */
void *radix_tree_tag_set(struct radix_tree_root *root,
			unsigned long index, unsigned int tag)
{
	unsigned int height, shift;
	struct radix_tree_node *slot;

	height = root->height;
	if (index > radix_tree_maxindex(height))
		return NULL;

	shift = (height - 1) * RADIX_TREE_MAP_SHIFT;
	slot = root->rnode;

	while (height > 0) {
		int offset;

		offset = (index >> shift) & RADIX_TREE_MAP_MASK;
		if (!tag_get(slot, tag, offset))
			tag_set(slot, tag, offset);
		slot = slot->slots[offset];
		ASSERT(slot != NULL);
		shift -= RADIX_TREE_MAP_SHIFT;
		height--;
	}

	return slot;
}

/**
 *	radix_tree_tag_clear - clear a tag on a radix tree node
 *	@root:		radix tree root
 *	@index:		index key
 *	@tag: 		tag index
 *
 *	Clear the search tag (which must be < RADIX_TREE_MAX_TAGS)
 *	corresponding to @index in the radix tree.  If
 *	this causes the leaf node to have no tags set then clear the tag in the
 *	next-to-leaf node, etc.
 *
 *	Returns the address of the tagged item on success, else NULL.  ie:
 *	has the same return value and semantics as radix_tree_lookup().
 */
void *radix_tree_tag_clear(struct radix_tree_root *root,
			unsigned long index, unsigned int tag)
{
	struct radix_tree_path path[RADIX_TREE_MAX_PATH + 1], *pathp = path;
	struct radix_tree_node *slot;
	unsigned int height, shift;
	void *ret = NULL;

	height = root->height;
	if (index > radix_tree_maxindex(height))
		goto out;

	shift = (height - 1) * RADIX_TREE_MAP_SHIFT;
	pathp->node = NULL;
	slot = root->rnode;

	while (height > 0) {
		int offset;

		if (slot == NULL)
			goto out;

		offset = (index >> shift) & RADIX_TREE_MAP_MASK;
		pathp[1].offset = offset;
		pathp[1].node = slot;
		slot = slot->slots[offset];
		pathp++;
		shift -= RADIX_TREE_MAP_SHIFT;
		height--;
	}

	ret = slot;
	if (ret == NULL)
		goto out;

	do {
		if (!tag_get(pathp->node, tag, pathp->offset))
			goto out;
		tag_clear(pathp->node, tag, pathp->offset);
		if (any_tag_set(pathp->node, tag))
			goto out;
		pathp--;
	} while (pathp->node);
out:
	return ret;
}

#endif

static unsigned int
__lookup(struct radix_tree_root *root, void **results, unsigned long index,
	unsigned int max_items, unsigned long *next_index)
{
	unsigned int nr_found = 0;
	unsigned int shift, height;
	struct radix_tree_node *slot;
	unsigned long i;

	height = root->height;
	if (height == 0)
		goto out;

	shift = (height-1) * RADIX_TREE_MAP_SHIFT;
	slot = root->rnode;

	for ( ; height > 1; height--) {

		for (i = (index >> shift) & RADIX_TREE_MAP_MASK ;
				i < RADIX_TREE_MAP_SIZE; i++) {
			if (slot->slots[i] != NULL)
				break;
			index &= ~((1UL << shift) - 1);
			index += 1UL << shift;
			if (index == 0)
				goto out;	/* 32-bit wraparound */
		}
		if (i == RADIX_TREE_MAP_SIZE)
			goto out;

		shift -= RADIX_TREE_MAP_SHIFT;
		slot = slot->slots[i];
	}

	/* Bottom level: grab some items */
	for (i = index & RADIX_TREE_MAP_MASK; i < RADIX_TREE_MAP_SIZE; i++) {
		index++;
		if (slot->slots[i]) {
			results[nr_found++] = slot->slots[i];
			if (nr_found == max_items)
				goto out;
		}
	}
out:
	*next_index = index;
	return nr_found;
}

/**
 *	radix_tree_gang_lookup - perform multiple lookup on a radix tree
 *	@root:		radix tree root
 *	@results:	where the results of the lookup are placed
 *	@first_index:	start the lookup from this key
 *	@max_items:	place up to this many items at *results
 *
 *	Performs an index-ascending scan of the tree for present items.  Places
 *	them at *@results and returns the number of items which were placed at
 *	*@results.
 *
 *	The implementation is naive.
 */
unsigned int
radix_tree_gang_lookup(struct radix_tree_root *root, void **results,
			unsigned long first_index, unsigned int max_items)
{
	const unsigned long max_index = radix_tree_maxindex(root->height);
	unsigned long cur_index = first_index;
	unsigned int ret = 0;

	while (ret < max_items) {
		unsigned int nr_found;
		unsigned long next_index;	/* Index of next search */

		if (cur_index > max_index)
			break;
		nr_found = __lookup(root, results + ret, cur_index,
					max_items - ret, &next_index);
		ret += nr_found;
		if (next_index == 0)
			break;
		cur_index = next_index;
	}
	return ret;
}

/**
 *	radix_tree_gang_lookup_ex - perform multiple lookup on a radix tree
 *	@root:		radix tree root
 *	@results:	where the results of the lookup are placed
 *	@first_index:	start the lookup from this key
 *	@last_index:	don't lookup past this key
 *	@max_items:	place up to this many items at *results
 *
 *	Performs an index-ascending scan of the tree for present items starting
 *	@first_index until @last_index up to as many as @max_items.  Places
 *	them at *@results and returns the number of items which were placed
 *	at *@results.
 *
 *	The implementation is naive.
 */
unsigned int
radix_tree_gang_lookup_ex(struct radix_tree_root *root, void **results,
			unsigned long first_index, unsigned long last_index,
			unsigned int max_items)
{
	const unsigned long max_index = radix_tree_maxindex(root->height);
	unsigned long cur_index = first_index;
	unsigned int ret = 0;

	while (ret < max_items && cur_index < last_index) {
		unsigned int nr_found;
		unsigned long next_index;	/* Index of next search */

		if (cur_index > max_index)
			break;
		nr_found = __lookup(root, results + ret, cur_index,
					max_items - ret, &next_index);
		ret += nr_found;
		if (next_index == 0)
			break;
		cur_index = next_index;
	}
	return ret;
}

#ifdef RADIX_TREE_TAGS

static unsigned int
__lookup_tag(struct radix_tree_root *root, void **results, unsigned long index,
	unsigned int max_items, unsigned long *next_index, unsigned int tag)
{
	unsigned int nr_found = 0;
	unsigned int shift;
	unsigned int height = root->height;
	struct radix_tree_node *slot;

	shift = (height - 1) * RADIX_TREE_MAP_SHIFT;
	slot = root->rnode;

	while (height > 0) {
		unsigned long i = (index >> shift) & RADIX_TREE_MAP_MASK;

		for ( ; i < RADIX_TREE_MAP_SIZE; i++) {
			if (tag_get(slot, tag, i)) {
				ASSERT(slot->slots[i] != NULL);
				break;
			}
			index &= ~((1UL << shift) - 1);
			index += 1UL << shift;
			if (index == 0)
				goto out;	/* 32-bit wraparound */
		}
		if (i == RADIX_TREE_MAP_SIZE)
			goto out;
		height--;
		if (height == 0) {	/* Bottom level: grab some items */
			unsigned long j = index & RADIX_TREE_MAP_MASK;

			for ( ; j < RADIX_TREE_MAP_SIZE; j++) {
				index++;
				if (tag_get(slot, tag, j)) {
					ASSERT(slot->slots[j] != NULL);
					results[nr_found++] = slot->slots[j];
					if (nr_found == max_items)
						goto out;
				}
			}
		}
		shift -= RADIX_TREE_MAP_SHIFT;
		slot = slot->slots[i];
	}
out:
	*next_index = index;
	return nr_found;
}

/**
 *	radix_tree_gang_lookup_tag - perform multiple lookup on a radix tree
 *	                             based on a tag
 *	@root:		radix tree root
 *	@results:	where the results of the lookup are placed
 *	@first_index:	start the lookup from this key
 *	@max_items:	place up to this many items at *results
 *	@tag:		the tag index (< RADIX_TREE_MAX_TAGS)
 *
 *	Performs an index-ascending scan of the tree for present items which
 *	have the tag indexed by @tag set.  Places the items at *@results and
 *	returns the number of items which were placed at *@results.
 */
unsigned int
radix_tree_gang_lookup_tag(struct radix_tree_root *root, void **results,
		unsigned long first_index, unsigned int max_items,
		unsigned int tag)
{
	const unsigned long max_index = radix_tree_maxindex(root->height);
	unsigned long cur_index = first_index;
	unsigned int ret = 0;

	while (ret < max_items) {
		unsigned int nr_found;
		unsigned long next_index;	/* Index of next search */

		if (cur_index > max_index)
			break;
		nr_found = __lookup_tag(root, results + ret, cur_index,
					max_items - ret, &next_index, tag);
		ret += nr_found;
		if (next_index == 0)
			break;
		cur_index = next_index;
	}
	return ret;
}

#endif

/**
 *	radix_tree_shrink    -    shrink height of a radix tree to minimal
 *	@root		radix tree root
 */
static inline void radix_tree_shrink(struct radix_tree_root *root)
{
	/* try to shrink tree height */
	while (root->height > 1 &&
			root->rnode->count == 1 &&
			root->rnode->slots[0]) {
		struct radix_tree_node *to_free = root->rnode;

		root->rnode = to_free->slots[0];
		root->height--;
		/* must only free zeroed nodes into the slab */
#ifdef RADIX_TREE_TAGS
		tag_clear(to_free, 0, 0);
		tag_clear(to_free, 1, 0);
#endif
		to_free->slots[0] = NULL;
		to_free->count = 0;
		radix_tree_node_free(to_free);
	}
}

/**
 *	radix_tree_delete    -    delete an item from a radix tree
 *	@root:		radix tree root
 *	@index:		index key
 *
 *	Remove the item at @index from the radix tree rooted at @root.
 *
 *	Returns the address of the deleted item, or NULL if it was not present.
 */
void *radix_tree_delete(struct radix_tree_root *root, unsigned long index)
{
	struct radix_tree_path path[RADIX_TREE_MAX_PATH + 1], *pathp = path;
	struct radix_tree_path *orig_pathp;
	struct radix_tree_node *slot;
	unsigned int height, shift;
	void *ret = NULL;
#ifdef RADIX_TREE_TAGS
	char tags[RADIX_TREE_MAX_TAGS];
	int nr_cleared_tags;
	int tag;
#endif
	int offset;

	height = root->height;
	if (index > radix_tree_maxindex(height))
		goto out;

	shift = (height - 1) * RADIX_TREE_MAP_SHIFT;
	pathp->node = NULL;
	slot = root->rnode;

	for ( ; height > 0; height--) {
		if (slot == NULL)
			goto out;

		pathp++;
		offset = (index >> shift) & RADIX_TREE_MAP_MASK;
		pathp->offset = offset;
		pathp->node = slot;
		slot = slot->slots[offset];
		shift -= RADIX_TREE_MAP_SHIFT;
	}

	ret = slot;
	if (ret == NULL)
		goto out;

	orig_pathp = pathp;

#ifdef RADIX_TREE_TAGS
	/*
	 * Clear all tags associated with the just-deleted item
	 */
	nr_cleared_tags = 0;
	for (tag = 0; tag < RADIX_TREE_MAX_TAGS; tag++) {
		tags[tag] = 1;
		if (tag_get(pathp->node, tag, pathp->offset)) {
			tag_clear(pathp->node, tag, pathp->offset);
			if (!any_tag_set(pathp->node, tag)) {
				tags[tag] = 0;
				nr_cleared_tags++;
			}
		}
	}

	for (pathp--; nr_cleared_tags && pathp->node; pathp--) {
		for (tag = 0; tag < RADIX_TREE_MAX_TAGS; tag++) {
			if (tags[tag])
				continue;

			tag_clear(pathp->node, tag, pathp->offset);
			if (any_tag_set(pathp->node, tag)) {
				tags[tag] = 1;
				nr_cleared_tags--;
			}
		}
	}
#endif
	/* Now free the nodes we do not need anymore */
	for (pathp = orig_pathp; pathp->node; pathp--) {
		pathp->node->slots[pathp->offset] = NULL;
		pathp->node->count--;

		if (pathp->node->count) {
			if (pathp->node == root->rnode)
				radix_tree_shrink(root);
			goto out;
		}

		/* Node with zero slots in use so free it */
		radix_tree_node_free(pathp->node);
	}
	root->rnode = NULL;
	root->height = 0;
out:
	return ret;
}

#ifdef RADIX_TREE_TAGS
/**
 *	radix_tree_tagged - test whether any items in the tree are tagged
 *	@root:		radix tree root
 *	@tag:		tag to test
 */
int radix_tree_tagged(struct radix_tree_root *root, unsigned int tag)
{
  	struct radix_tree_node *rnode;
  	rnode = root->rnode;
  	if (!rnode)
  		return 0;
	return any_tag_set(rnode, tag);
}
#endif

static unsigned long __maxindex(unsigned int height)
{
	unsigned int tmp = height * RADIX_TREE_MAP_SHIFT;
	unsigned long index = (~0UL >> (RADIX_TREE_INDEX_BITS - tmp - 1)) >> 1;

	if (tmp >= RADIX_TREE_INDEX_BITS)
		index = ~0UL;
	return index;
}

static void radix_tree_init_maxindex(void)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(height_to_maxindex); i++)
		height_to_maxindex[i] = __maxindex(i);
}

void radix_tree_init(void)
{
	radix_tree_init_maxindex();
}
