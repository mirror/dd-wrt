/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * A skip list is a probabilistic alternative to balanced trees. Unlike the
 * red-black tree, it does not require rebalancing.
 *
 * This implementation uses only unidirectional next pointers and is optimized
 * for use in a priority queue where elements are mostly deleted from the front
 * of the queue.
 *
 * When storing up to 2^n elements in a n-level skiplist. lookup and deletion
 * for the first element happens in O(1) time, other than that, insertion and
 * deletion takes O(log n) time, assuming that the number of elements for an
 * n-level list does not exceed 2^n.
 *
 * Usage:
 * DECLARE_SKIPLIST_TYPE(foo, 5) will define the data types for a 5-level list:
 * struct foo_list: the list data type
 * struct foo_node: the node data for an element in the list
 *
 * DECLARE_SKIPLIST_IMPL(foo, foo_cmp_fn)
 *
 * Adds the skip list implementation. It depends on a provided function:
 * int foo_cmp_fn(struct foo_list *list, struct foo_node *n1, struct foo_node *n2)
 * This compares two elements given by their node pointers, returning values <0
 * if n1 is less than n2, =0 and >0 for equal or bigger than respectively.
 *
 * This macro implements the following functions:
 *
 * void foo_list_init(struct foo_list *list)
 *	initializes the skip list
 *
 * void foo_node_init(struct foo_node *node)
 *	initializes a node. must be called before adding the node to the list
 *
 * struct foo_node *foo_node_next(struct foo_node *node)
 *	gets the node directly after the provided node, or NULL if it was the last
 *	element in the list.
 *
 * bool foo_is_queued(struct foo_node *node)
 *	returns true if the node is on a list
 *
 * struct foo_node *foo_dequeue(struct foo_list *list)
 *	deletes and returns the first element of the list (or returns NULL if empty)
 *
 * struct foo_node *foo_peek(struct foo_list *list)
 *	returns the first element of the list
 *
 * void foo_insert(struct foo_list *list, struct foo_node *node)
 *	inserts the node into the list. the node must be initialized and not on a
 *	list already.
 *
 * void foo_delete(struct foo_list *list, struct foo_node *node)
 *	deletes the node from the list, or does nothing if it's not on the list
 */
#ifndef __SKIPLIST_H
#define __SKIPLIST_H

#include <linux/bits.h>
#include <linux/bug.h>
#include <linux/random.h>

#define SKIPLIST_POISON ((void *)1)

#define DECLARE_SKIPLIST_TYPE(name, levels)				\
struct name##_node {							\
	struct name##_node *next[levels];				\
};									\
struct name##_list {							\
	struct name##_node head;					\
	unsigned int max_level;						\
	unsigned int count;						\
};

#define DECLARE_SKIPLIST_IMPL(name, cmp_fn)				\
static inline void							\
name##_list_init(struct name##_list *list)				\
{									\
	memset(list, 0, sizeof(*list));					\
}									\
static inline void							\
name##_node_init(struct name##_node *node)				\
{									\
	node->next[0] = SKIPLIST_POISON;				\
}									\
static inline struct name##_node *					\
name##_node_next(struct name##_node *node)				\
{									\
	return node->next[0];						\
}									\
static inline bool							\
name##_is_queued(struct name##_node *node)				\
{									\
	return node->next[0] != SKIPLIST_POISON;			\
}									\
static inline int							\
__skiplist_##name##_cmp_impl(void *head, void *n1, void *n2)		\
{									\
	return cmp_fn(head, n1, n2);					\
}									\
static inline void							\
__##name##_delete(struct name##_list *list)				\
{									\
	list->count--;							\
	while (list->max_level &&					\
	       !list->head.next[list->max_level])			\
		list->max_level--;					\
}									\
static inline struct name##_node *					\
name##_dequeue(struct name##_list *list)				\
{									\
	struct name##_node *ret;					\
	unsigned int max_level = ARRAY_SIZE(list->head.next) - 1;	\
	ret = (void *)__skiplist_dequeue((void **)&list->head,		\
					 max_level);			\
	if (!ret)							\
		return NULL;						\
	__##name##_delete(list);					\
	return ret;							\
}									\
static inline struct name##_node *					\
name##_peek(struct name##_list *list)					\
{									\
	return list->head.next[0];					\
}									\
static inline void							\
name##_insert(struct name##_list *list, struct name##_node *node)	\
{									\
	int level = __skiplist_level(ARRAY_SIZE(list->head.next) - 1,	\
				     list->count, get_random_u32());	\
	level = min_t(int, level, list->max_level + 1);			\
	__skiplist_insert((void *)&list->head, (void *)node, level,	\
			  __skiplist_##name##_cmp_impl);		\
	if (level > list->max_level)					\
		list->max_level = level;				\
	list->count++;							\
}									\
static inline void							\
name##_delete(struct name##_list *list, struct name##_node *node)	\
{									\
	if (node->next[0] == SKIPLIST_POISON)				\
	    return;							\
	__skiplist_delete((void *)&list->head, (void *)node,		\
			  ARRAY_SIZE(list->head.next) - 1,		\
			  __skiplist_##name##_cmp_impl);		\
	__##name##_delete(list);					\
}


typedef int (*__skiplist_cmp_t)(void *head, void *n1, void *n2);

#define __skiplist_cmp(cmp, head, cur, node)				\
	({								\
		int cmp_val = cmp(head, cur, node);			\
		if (!cmp_val)						\
			cmp_val = (unsigned long)(cur) -		\
				  (unsigned long)(node);		\
		cmp_val;						\
	})

static inline void *
__skiplist_dequeue(void **list, int max_level)
{
	void **node = list[0];
	unsigned int i;

	if (!node)
		return NULL;

	list[0] = node[0];
	for (i = 1; i <= max_level; i++) {
		if (list[i] != node)
			break;

		list[i] = node[i];
	}
	node[0] = SKIPLIST_POISON;

	return node;
}

static inline void
__skiplist_insert(void **list, void **node, int level, __skiplist_cmp_t cmp)
{
	void **head = list;

	if (WARN(node[0] != SKIPLIST_POISON, "Insert on already inserted or uninitialized node"))
	    return;
	for (; level >= 0; level--) {
		while (list[level] &&
		       __skiplist_cmp(cmp, head, list[level], node) < 0)
			list = list[level];

		node[level] = list[level];
		list[level] = node;
	}
}


static inline void
__skiplist_delete(void **list, void **node, int max_level, __skiplist_cmp_t cmp)
{
	void *head = list;
	int i;

	for (i = max_level; i >= 0; i--) {
		while (list[i] && list[i] != node &&
		       __skiplist_cmp(cmp, head, list[i], node) <= 0)
			list = list[i];

		if (list[i] != node)
			continue;

		list[i] = node[i];
	}
	node[0] = SKIPLIST_POISON;
}

static inline unsigned int
__skiplist_level(unsigned int max_level, unsigned int count, unsigned int seed)
{
	unsigned int level = 0;

	if (max_level >= 16 && !(seed & GENMASK(15, 0))) {
		level += 16;
		seed >>= 16;
	}

	if (max_level >= 8 && !(seed & GENMASK(7, 0))) {
		level += 8;
		seed >>= 8;
	}

	if (max_level >= 4 && !(seed & GENMASK(3, 0))) {
		level += 4;
		seed >>= 4;
	}

	if (!(seed & GENMASK(1, 0))) {
		level += 2;
		seed >>= 2;
	}

	if (!(seed & BIT(0)))
		level++;

	return min(level, max_level);
}

#endif
