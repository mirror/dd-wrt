// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2001 Momchil Velikov
 * Portions Copyright (C) 2001 Christoph Hellwig
 */
#ifndef __LIBFROG_RADIX_TREE_H__
#define __LIBFROG_RADIX_TREE_H__

#define RADIX_TREE_TAGS

struct radix_tree_root {
	unsigned int		height;
	struct radix_tree_node	*rnode;
};

#define RADIX_TREE_INIT(mask)	{					\
	.height = 0,							\
	.rnode = NULL,							\
}

#define RADIX_TREE(name, mask) \
	struct radix_tree_root name = RADIX_TREE_INIT(mask)

#define INIT_RADIX_TREE(root, mask)					\
do {									\
	(root)->height = 0;						\
	(root)->rnode = NULL;						\
} while (0)

#ifdef RADIX_TREE_TAGS
#define RADIX_TREE_MAX_TAGS 2
#endif

int radix_tree_insert(struct radix_tree_root *, unsigned long, void *);
void *radix_tree_lookup(struct radix_tree_root *, unsigned long);
void **radix_tree_lookup_slot(struct radix_tree_root *, unsigned long);
void *radix_tree_lookup_first(struct radix_tree_root *, unsigned long *);
void *radix_tree_delete(struct radix_tree_root *, unsigned long);
unsigned int
radix_tree_gang_lookup(struct radix_tree_root *root, void **results,
			unsigned long first_index, unsigned int max_items);
unsigned int
radix_tree_gang_lookup_ex(struct radix_tree_root *root, void **results,
			unsigned long first_index, unsigned long last_index,
			unsigned int max_items);

void radix_tree_init(void);

#ifdef RADIX_TREE_TAGS
void *radix_tree_tag_set(struct radix_tree_root *root,
			unsigned long index, unsigned int tag);
void *radix_tree_tag_clear(struct radix_tree_root *root,
			unsigned long index, unsigned int tag);
int radix_tree_tag_get(struct radix_tree_root *root,
			unsigned long index, unsigned int tag);
unsigned int
radix_tree_gang_lookup_tag(struct radix_tree_root *root, void **results,
			unsigned long first_index, unsigned int max_items,
			unsigned int tag);
int radix_tree_tagged(struct radix_tree_root *root, unsigned int tag);
#endif

static inline int radix_tree_preload(int gfp_mask) { return 0; }
static inline void radix_tree_preload_end(void) { }

#endif /* __LIBFROG_RADIX_TREE_H__ */
