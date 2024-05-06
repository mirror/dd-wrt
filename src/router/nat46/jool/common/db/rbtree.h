#ifndef SRC_MOD_COMMON_RBTREE_H_
#define SRC_MOD_COMMON_RBTREE_H_

/**
 * @file
 * This is just some convenience additions to the kernel's Red-Black Tree
 * implementation.
 * I'm sorry it's a macro maze, but the alternative is a lot of redundant code.
 */

#include <linux/rbtree.h>

/**
 * rbtree_find - Stock search on a Red-Black tree.
 *
 * If you want to read a cleaner version of it, see
 * https://www.kernel.org/doc/Documentation/rbtree.txt
 *
 * @expected is a key (has the key's type).
 * @compare_fn takes a @type and the key's type as arguments.
 */
#define rbtree_find(expected, root, compare_fn, type, hook_name) \
	({ \
		type *result = NULL; \
		struct rb_node *node; \
		\
		node = (root)->rb_node; \
		while (node) { \
			type *entry = rb_entry(node, type, hook_name); \
			int comparison = compare_fn(entry, expected); \
			\
			if (comparison < 0) { \
				node = node->rb_right; \
			} else if (comparison > 0) { \
				node = node->rb_left; \
			} else { \
				result = entry; \
				break; \
			} \
		} \
		\
		result; \
	})

/**
 * rbtree_add - Add a node to a Red-Black tree.
 *
 * Returns NULL on success. If there was a collision, it returns the in-tree
 * entry that caused it. There are no other possible outcomes.
 *
 * If you want to read a cleaner version of it, see
 * https://www.kernel.org/doc/Documentation/rbtree.txt
 */
#define rbtree_add(entry, key, root, compare_fn, type, hook_name) \
	({ \
		struct rb_node **new = &((root)->rb_node), *parent = NULL; \
		type *collision = NULL; \
		\
		/* Figure out where to put new node */ \
		while (*new) { \
			type *this = rb_entry(*new, type, hook_name); \
			int result = compare_fn(this, key); \
			\
			parent = *new; \
			if (result < 0) { \
				new = &((*new)->rb_right); \
			} else if (result > 0) { \
				new = &((*new)->rb_left); \
			} else { \
				collision = this; \
				break; \
			} \
		} \
		\
		/* Add new node and rebalance tree. */ \
		if (!collision) { \
			rb_link_node(&(entry)->hook_name, parent, new); \
			rb_insert_color(&(entry)->hook_name, root); \
		} \
		\
		collision; \
	})

/**
 * A "tree slot" is a temporal memorization of where in a red-black tree might
 * an node be added.
 *
 * This:
 *
 *	node = tree_find(a);
 *	if (!node) {
 *		error = do_other_things();
 *		if (error)
 *			return error;
 *		tree_add(a);
 *	}
 *
 * Is needlessly slow because, since an add implies a find, there are two
 * identical tree searches where one would have sufficed.
 *
 * Unfortunately, Jool is full of do_other_things() constructs so a simultaneous
 * find and/or add function is rarely useful.
 *
 * Tree slots can be used instead. The slot is found and initialized during the
 * find, and the add only commits it. If there is a reason to cancel the add,
 * the slot structure is simply abandoned. Their fairly small size makes them
 * natural stack citizens.
 *
 *	struct tree_slot slot;
 *
 *	node = tree_find(a, &slot);
 *	if (!node) {
 *		error = do_other_things();
 *		if (error)
 *			return error;
 *		tree_add(&slot);
 *	}
 *
 * You need to make sure the tree does not change while you hold an initialized
 * slot you're planning to commit.
 */
struct tree_slot {
	struct rb_root *tree;
	struct rb_node *entry;
	struct rb_node *parent;
	struct rb_node **rb_link;
};

/** Prepares @slot for tree traversal. */
void treeslot_init(struct tree_slot *slot,
		struct rb_root *tree,
		struct rb_node *entry);
/** Adds @slot's node to the tree. Also rebalances while it's at it. */
void treeslot_commit(struct tree_slot *slot);

/**
 * rbtree_find_node - Similar to rbtree_find(), except if it doesn't find the
 * node it returns the slot where it'd be placed so you can insert something in
 * there.
 *
 * @expected has to be a rb_node,
 * @compare_cb takes rb_nodes as arguments.
 */
#define rbtree_find_slot(expected, root, compare_cb, slot) \
	({ \
		struct rb_node *collision = NULL; \
		struct rb_node *node; \
		\
		treeslot_init(slot, root, expected); \
		node = (root)->rb_node; \
		\
		while (node) { \
			int comparison = compare_cb(node, expected); \
			(slot)->parent = node; \
			if (comparison < 0) { \
				(slot)->rb_link = &node->rb_right; \
				node = node->rb_right; \
			} else if (comparison > 0) { \
				(slot)->rb_link = &node->rb_left; \
				node = node->rb_left; \
			} else { \
				collision = node; \
				node = NULL; \
			} \
		} \
		\
		collision; \
	})

/**
 * rbtree_find_node - Similar to rbtree_find_slot(), except it doesn't rely
 * on a struct tree_slot.
 *
 * TODO (fine) seriously. The point of this module is to prevent duplicate code.
 */
#define rbtree_find_node(expected, root, compare_cb, type, hook_name, parent, \
		node) \
	({ \
		node = &((root)->rb_node); \
		parent = NULL; \
		\
		/* Figure out where to put new node */ \
		while (*node) { \
			type *entry = rb_entry(*node, type, hook_name); \
			int comparison = compare_cb(entry, expected); \
			\
			parent = *node; \
			if (comparison < 0) { \
				node = &((*node)->rb_right); \
			} else if (comparison > 0) { \
				node = &((*node)->rb_left); \
			} else { \
				break; \
			} \
		} \
	})

#define rbtree_foreach rbtree_postorder_for_each_entry_safe

#endif /* SRC_MOD_COMMON_RBTREE_H_ */
