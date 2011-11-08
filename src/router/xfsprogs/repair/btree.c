/*
 * Copyright (c) 2007, Silicon Graphics, Inc. Barry Naujok <bnaujok@sgi.com>
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

#include <libxfs.h>
#include "btree.h"

/*
 * Maximum number of keys per node.  Must be greater than 2 for the code
 * to work.
 */
#define BTREE_KEY_MAX		7
#define BTREE_KEY_MIN		(BTREE_KEY_MAX / 2)

#define BTREE_PTR_MAX		(BTREE_KEY_MAX + 1)

struct btree_node {
	unsigned long		num_keys;
	unsigned long		keys[BTREE_KEY_MAX];
	struct btree_node	*ptrs[BTREE_PTR_MAX];
};

struct btree_cursor {
	struct btree_node	*node;
	int			index;
};

struct btree_root {
	struct btree_node	*root_node;
	struct btree_cursor	*cursor;	/* track path to end leaf */
	int			height;
	/* lookup cache */
	int			keys_valid;	/* set if the cache is valid */
	unsigned long		cur_key;
	unsigned long		next_key;
	void			*next_value;
	unsigned long		prev_key;
	void			*prev_value;
#ifdef BTREE_STATS
	struct btree_stats {
		unsigned long	num_items;
		unsigned long	max_items;
		int		alloced;
		int		cache_hits;
		int		cache_misses;
		int		lookup;
		int		find;
		int		key_update;
		int		value_update;
		int		insert;
		int		delete;
		int		inc_height;
		int		dec_height;
		int		shift_prev;
		int		shift_next;
		int		split;
		int		merge_prev;
		int		merge_next;
		int		balance_prev;
		int		balance_next;
	} stats;
#endif
};


static struct btree_node *
btree_node_alloc(void)
{
	return calloc(1, sizeof(struct btree_node));
}

static void
btree_node_free(
	struct btree_node 	*node)
{
	free(node);
}

static void
btree_free_nodes(
	struct btree_node	*node,
	int			level)
{
	int			i;

	if (level)
		for (i = 0; i <= node->num_keys; i++)
			btree_free_nodes(node->ptrs[i], level - 1);
	btree_node_free(node);
}

static void
__btree_init(
	struct btree_root	*root)
{
	memset(root, 0, sizeof(struct btree_root));
	root->height = 1;
	root->cursor = calloc(1, sizeof(struct btree_cursor));
	root->root_node = btree_node_alloc();
	ASSERT(root->root_node);
#ifdef BTREE_STATS
	root->stats.max_items = 1;
	root->stats.alloced += 1;
#endif
}

static void
__btree_free(
	struct btree_root	*root)
{
	btree_free_nodes(root->root_node, root->height - 1);
	free(root->cursor);
	root->height = 0;
	root->cursor = NULL;
	root->root_node = NULL;
}

void
btree_init(
	struct btree_root	**root)
{
	*root = calloc(1, sizeof(struct btree_root));
	__btree_init(*root);
}

void
btree_clear(
	struct btree_root	*root)
{
	__btree_free(root);
	__btree_init(root);
}

void
btree_destroy(
	struct btree_root	*root)
{
	__btree_free(root);
	free(root);
}

int
btree_is_empty(
	struct btree_root	*root)
{
	return root->root_node->num_keys == 0;
}

static inline void
btree_invalidate_cursor(
	struct btree_root	*root)
{
	root->cursor[0].node = NULL;
	root->keys_valid = 0;
}

static inline unsigned long
btree_key_of_cursor(
	struct btree_cursor	*cursor,
	int			height)
{
	while (cursor->node->num_keys == cursor->index && --height > 0)
		cursor++;
	return cursor->node->keys[cursor->index];
}

static void *
btree_get_prev(
	struct btree_root	*root,
	unsigned long		*key)
{
	struct btree_cursor	*cur = root->cursor;
	int			level = 0;
	struct btree_node	*node;

	if (cur->index > 0) {
		if (key)
			*key = cur->node->keys[cur->index - 1];
		return cur->node->ptrs[cur->index - 1];
	}

	/* else need to go up and back down the tree to find the previous */
	do {
		if (cur->index)
			break;
		cur++;
	} while (++level < root->height);

	if (level == root->height)
		return NULL;

	/* the key is in the current level */
	if (key)
		*key = cur->node->keys[cur->index - 1];

	/* descend back down the right side to get the pointer */
	node = cur->node->ptrs[cur->index - 1];
	while (level--)
		node = node->ptrs[node->num_keys];
	return node;
}

static void *
btree_get_next(
	struct btree_root	*root,
	unsigned long		*key)
{
	struct btree_cursor	*cur = root->cursor;
	int			level = 0;
	struct btree_node	*node;

	while (cur->index == cur->node->num_keys) {
		if (++level == root->height)
			return NULL;
		cur++;
	}
	if (level == 0) {
		if (key) {
			cur->index++;
			*key = btree_key_of_cursor(cur, root->height);
			cur->index--;
		}
		return cur->node->ptrs[cur->index + 1];
	}

	node = cur->node->ptrs[cur->index + 1];
	while (--level > 0)
		node = node->ptrs[0];
	if (key)
		*key = node->keys[0];
	return node->ptrs[0];
}

/*
 * Lookup/Search functions
 */

static int
btree_do_search(
	struct btree_root	*root,
	unsigned long		key)
{
	unsigned long		k = 0;
	struct btree_cursor	*cur = root->cursor + root->height;
	struct btree_node	*node = root->root_node;
	int			height = root->height;
	int			key_found = 0;
	int			i;

	while (--height >= 0) {
		cur--;
		for (i = 0; i < node->num_keys; i++)
			if (node->keys[i] >= key) {
				k = node->keys[i];
				key_found = 1;
				break;
			}
		cur->node = node;
		cur->index = i;
		node = node->ptrs[i];
	}
	root->keys_valid = key_found;
	if (!key_found)
		return 0;

	root->cur_key = k;
	root->next_value = NULL;	/* do on-demand next value lookup */
	root->prev_value = btree_get_prev(root, &root->prev_key);
	return 1;
}

static int
btree_search(
	struct btree_root	*root,
	unsigned long		key)
{
	if (root->keys_valid && key <= root->cur_key &&
				(!root->prev_value || key > root->prev_key)) {
#ifdef BTREE_STATS
		root->stats.cache_hits++;
#endif
		return 1;
	}
#ifdef BTREE_STATS
	root->stats.cache_misses++;
#endif
	return btree_do_search(root, key);
}

void *
btree_find(
	struct btree_root	*root,
	unsigned long		key,
	unsigned long		*actual_key)
{
#ifdef BTREE_STATS
	root->stats.find += 1;
#endif
	if (!btree_search(root, key))
		return NULL;

	if (actual_key)
		*actual_key = root->cur_key;
	return root->cursor->node->ptrs[root->cursor->index];
}

void *
btree_lookup(
	struct btree_root	*root,
	unsigned long		key)
{
#ifdef BTREE_STATS
	root->stats.lookup += 1;
#endif
	if (!btree_search(root, key) || root->cur_key != key)
		return NULL;
	return root->cursor->node->ptrs[root->cursor->index];
}

void *
btree_peek_prev(
	struct btree_root	*root,
	unsigned long		*key)
{
	if (!root->keys_valid)
		return NULL;
	if (key)
		*key = root->prev_key;
	return root->prev_value;
}

void *
btree_peek_next(
	struct btree_root	*root,
	unsigned long		*key)
{
	if (!root->keys_valid)
		return NULL;
	if (!root->next_value)
		root->next_value = btree_get_next(root, &root->next_key);
	if (key)
		*key = root->next_key;
	return root->next_value;
}

static void *
btree_move_cursor_to_next(
	struct btree_root	*root,
	unsigned long		*key)
{
	struct btree_cursor	*cur = root->cursor;
	int			level = 0;

	while (cur->index == cur->node->num_keys) {
		if (++level == root->height)
			return NULL;
		cur++;
	}
	cur->index++;
	if (level == 0) {
		if (key)
			*key = btree_key_of_cursor(cur, root->height);
		return cur->node->ptrs[cur->index];
	}

	while (--level >= 0) {
		root->cursor[level].node = cur->node->ptrs[cur->index];
		root->cursor[level].index = 0;
		cur--;
	}
	if (key)
		*key = cur->node->keys[0];
	return cur->node->ptrs[0];
}

void *
btree_lookup_next(
	struct btree_root	*root,
	unsigned long		*key)
{
	void			*value;

	if (!root->keys_valid)
		return NULL;

	root->prev_key = root->cur_key;
	root->prev_value = root->cursor->node->ptrs[root->cursor->index];

	value = btree_move_cursor_to_next(root, &root->cur_key);
	if (!value) {
		btree_invalidate_cursor(root);
		return NULL;
	}
	root->next_value = NULL;	/* on-demand next value fetch */
	if (key)
		*key = root->cur_key;
	return value;
}

static void *
btree_move_cursor_to_prev(
	struct btree_root	*root,
	unsigned long		*key)
{
	struct btree_cursor	*cur = root->cursor;
	int			level = 0;

	while (cur->index == 0) {
		if (++level == root->height)
			return NULL;
		cur++;
	}
	cur->index--;
	if (key)	/* the key is in the current level */
		*key = cur->node->keys[cur->index];
	while (level > 0) {
		level--;
		root->cursor[level].node = cur->node->ptrs[cur->index];
		root->cursor[level].index = root->cursor[level].node->num_keys;
		cur--;
	}
	return cur->node->ptrs[cur->index];
}

void *
btree_lookup_prev(
	struct btree_root	*root,
	unsigned long		*key)
{
	void			*value;

	if (!root->keys_valid)
		return NULL;

	value = btree_move_cursor_to_prev(root, &root->cur_key);
	if (!value)
		return NULL;
	root->prev_value = btree_get_prev(root, &root->prev_key);
	root->next_value = NULL;	/* on-demand next value fetch */
	if (key)
		*key = root->cur_key;
	return value;
}

void *
btree_uncached_lookup(
	struct btree_root	*root,
	unsigned long		key)
{
	/* cursor-less (ie. uncached) lookup */
	int			height = root->height - 1;
	struct btree_node	*node = root->root_node;
	int			i;
	int			key_found = 0;

	while (height >= 0) {
		for (i = 0; i < node->num_keys; i++)
			if (node->keys[i] >= key) {
				key_found = node->keys[i] == key;
				break;
			}
		node = node->ptrs[i];
		height--;
	}
	return key_found ? node : NULL;
}

/* Update functions */

static inline void
btree_update_node_key(
	struct btree_root	*root,
	struct btree_cursor	*cursor,
	int			level,
	unsigned long		new_key)
{
	int			i;

#ifdef BTREE_STATS
	root->stats.key_update += 1;
#endif

	cursor += level;
	for (i = level; i < root->height; i++) {
		if (cursor->index < cursor->node->num_keys) {
			cursor->node->keys[cursor->index] = new_key;
			break;
		}
		cursor++;
	}
}

int
btree_update_key(
	struct btree_root	*root,
	unsigned long		old_key,
	unsigned long		new_key)
{
	if (!btree_search(root, old_key) || root->cur_key != old_key)
		return ENOENT;

	if (root->next_value && new_key >= root->next_key)
		return EINVAL;

	if (root->prev_value && new_key <= root->prev_key)
		return EINVAL;

	btree_update_node_key(root, root->cursor, 0, new_key);
	root->cur_key = new_key;

	return 0;
}

int
btree_update_value(
	struct btree_root	*root,
	unsigned long		key,
	void			*new_value)
{
	if (!new_value)
		return EINVAL;

	if (!btree_search(root, key) || root->cur_key != key)
		return ENOENT;

#ifdef BTREE_STATS
	root->stats.value_update += 1;
#endif
	root->cursor->node->ptrs[root->cursor->index] = new_value;

	return 0;
}

/*
 * Cursor modification functions - used for inserting and deleting
 */

static struct btree_cursor *
btree_copy_cursor_prev(
	struct btree_root	*root,
	struct btree_cursor	*dest_cursor,
	int			level)
{
	struct btree_cursor	*src_cur = root->cursor + level;
	struct btree_cursor	*dst_cur;
	int			l = level;
	int			i;

	if (level >= root->height)
		return NULL;

	while (src_cur->index == 0) {
		if (++l >= root->height)
			return NULL;
		src_cur++;
	}
	for (i = l; i < root->height; i++)
		dest_cursor[i] = *src_cur++;

	dst_cur = dest_cursor + l;
	dst_cur->index--;
	while (l-- >= level) {
		dest_cursor[l].node = dst_cur->node->ptrs[dst_cur->index];
		dest_cursor[l].index = dest_cursor[l].node->num_keys;
		dst_cur--;
	}
	return dest_cursor;
}

static struct btree_cursor *
btree_copy_cursor_next(
	struct btree_root	*root,
	struct btree_cursor	*dest_cursor,
	int			level)
{
	struct btree_cursor	*src_cur = root->cursor + level;
	struct btree_cursor	*dst_cur;
	int			l = level;
	int			i;

	if (level >= root->height)
		return NULL;

	while (src_cur->index == src_cur->node->num_keys) {
		if (++l >= root->height)
			return NULL;
		src_cur++;
	}
	for (i = l; i < root->height; i++)
		dest_cursor[i] = *src_cur++;

	dst_cur = dest_cursor + l;
	dst_cur->index++;
	while (l-- >= level) {
		dest_cursor[l].node = dst_cur->node->ptrs[dst_cur->index];
		dest_cursor[l].index = 0;
		dst_cur--;
	}
	return dest_cursor;
}

/*
 * Shift functions
 *
 * Tries to move items in the current leaf to its sibling if it has space.
 * Used in both insert and delete functions.
 * Returns the number of items shifted.
 */

static int
btree_shift_to_prev(
	struct btree_root	*root,
	int			level,
	struct btree_cursor	*prev_cursor,
	int			num_children)
{
	struct btree_node	*node;
	struct btree_node	*prev_node;
	int			num_remain;	/* # of keys left in "node" */
	unsigned long		key;
	int			i;

	if (!prev_cursor || !num_children)
		return 0;

	prev_node = prev_cursor[level].node;
	node = root->cursor[level].node;

	ASSERT(num_children > 0 && num_children <= node->num_keys + 1);

	if ((prev_node->num_keys + num_children) > BTREE_KEY_MAX)
		return 0;

#ifdef BTREE_STATS
	root->stats.shift_prev += 1;
#endif

	num_remain = node->num_keys - num_children;
	ASSERT(num_remain == -1 || num_remain >= BTREE_KEY_MIN);

	/* shift parent keys around */
	level++;
	if (num_remain > 0)
		key = node->keys[num_children - 1];
	else
		key = btree_key_of_cursor(root->cursor + level,
						root->height - level);
	while (prev_cursor[level].index == prev_cursor[level].node->num_keys) {
		level++;
		ASSERT(level < root->height);
	}
	prev_node->keys[prev_node->num_keys] =
			prev_cursor[level].node->keys[prev_cursor[level].index];
	prev_cursor[level].node->keys[prev_cursor[level].index] = key;

	/* copy pointers and keys to the end of the prev node */
	for (i = 0; i < num_children - 1; i++) {
		prev_node->keys[prev_node->num_keys + 1 + i] = node->keys[i];
		prev_node->ptrs[prev_node->num_keys + 1 + i] = node->ptrs[i];
	}
	prev_node->ptrs[prev_node->num_keys + 1 + i] = node->ptrs[i];
	prev_node->num_keys += num_children;

	/* move remaining pointers/keys to start of node */
	if (num_remain >= 0) {
		for (i = 0; i < num_remain; i++) {
			node->keys[i] = node->keys[num_children + i];
			node->ptrs[i] = node->ptrs[num_children + i];
		}
		node->ptrs[i] = node->ptrs[num_children + i];
		node->num_keys = num_remain;
	} else
		node->num_keys = 0;

	return num_children;
}

static int
btree_shift_to_next(
	struct btree_root	*root,
	int			level,
	struct btree_cursor	*next_cursor,
	int			num_children)
{
	struct btree_node	*node;
	struct btree_node	*next_node;
	int			num_remain;	/* # of children left in node */
	int			i;

	if (!next_cursor || !num_children)
		return 0;

	node = root->cursor[level].node;
	next_node = next_cursor[level].node;

	ASSERT(num_children > 0 && num_children <= node->num_keys + 1);

	if ((next_node->num_keys + num_children) > BTREE_KEY_MAX)
		return 0;

	num_remain = node->num_keys + 1 - num_children;
	ASSERT(num_remain == 0 || num_remain > BTREE_KEY_MIN);

#ifdef BTREE_STATS
	root->stats.shift_next += 1;
#endif

	/* make space for "num_children" items at beginning of next-leaf */
	i = next_node->num_keys;
	next_node->ptrs[num_children + i] = next_node->ptrs[i];
	while (--i >= 0) {
		next_node->keys[num_children + i] = next_node->keys[i];
		next_node->ptrs[num_children + i] = next_node->ptrs[i];
	}

	/* update keys in parent and next node from parent */
	do {
		level++;
		ASSERT(level < root->height);
	} while (root->cursor[level].index ==
		 root->cursor[level].node->num_keys);

	next_node->keys[num_children - 1] =
		root->cursor[level].node->keys[root->cursor[level].index];
	root->cursor[level].node->keys[root->cursor[level].index] =
		node->keys[node->num_keys - num_children];

	/* copy last "num_children" items from node into start of next-node */
	for (i = 0; i < num_children - 1; i++) {
		next_node->keys[i] = node->keys[num_remain + i];
		next_node->ptrs[i] = node->ptrs[num_remain + i];
	}
	next_node->ptrs[i] = node->ptrs[num_remain + i];
	next_node->num_keys += num_children;

	if (num_remain > 0)
		node->num_keys -= num_children;
	else
		node->num_keys = 0;

	return num_children;
}

/*
 * Insertion functions
 */

static struct btree_node *
btree_increase_height(
	struct btree_root	*root)
{
	struct btree_node	*new_root;
	struct btree_cursor	*new_cursor;

	new_cursor = realloc(root->cursor, (root->height + 1) *
				sizeof(struct btree_cursor));
	if (!new_cursor)
		return NULL;
	root->cursor = new_cursor;

	new_root = btree_node_alloc();
	if (!new_root)
		return NULL;

#ifdef BTREE_STATS
	root->stats.alloced += 1;
	root->stats.inc_height += 1;
	root->stats.max_items *= BTREE_PTR_MAX;
#endif

	new_root->ptrs[0] = root->root_node;
	root->root_node = new_root;

	root->cursor[root->height].node = new_root;
	root->cursor[root->height].index = 0;

	root->height++;

	return new_root;
}

static int
btree_insert_item(
	struct btree_root	*root,
	int			level,
	unsigned long		key,
	void			*value);


static struct btree_node *
btree_split(
	struct btree_root	*root,
	int			level,
	unsigned long		key,
	int			*index)
{
	struct btree_node	*node = root->cursor[level].node;
	struct btree_node	*new_node;
	int			i;

	new_node = btree_node_alloc();
	if (!new_node)
		return NULL;

	if (btree_insert_item(root, level + 1, node->keys[BTREE_KEY_MIN],
							new_node) != 0) {
		btree_node_free(new_node);
		return NULL;
	}

#ifdef BTREE_STATS
	root->stats.alloced += 1;
	root->stats.split += 1;
#endif

	for (i = 0; i < BTREE_KEY_MAX - BTREE_KEY_MIN - 1; i++) {
		new_node->keys[i] = node->keys[BTREE_KEY_MIN + 1 + i];
		new_node->ptrs[i] = node->ptrs[BTREE_KEY_MIN + 1 + i];
	}
	new_node->ptrs[i] = node->ptrs[BTREE_KEY_MIN + 1 + i];
	new_node->num_keys = BTREE_KEY_MAX - BTREE_KEY_MIN - 1;

	node->num_keys = BTREE_KEY_MIN;
	if (key < node->keys[BTREE_KEY_MIN])
		return node;	/* index doesn't change */

	/* insertion point is in new node... */
	*index -= BTREE_KEY_MIN + 1;
	return new_node;
}

static int
btree_insert_shift_to_prev(
	struct btree_root	*root,
	int			level,
	int			*index)
{
	struct btree_cursor	tmp_cursor[root->height];
	int			n;

	if (*index <= 0)
		return -1;

	if (!btree_copy_cursor_prev(root, tmp_cursor, level + 1))
		return -1;

	n = MIN(*index, (BTREE_PTR_MAX - tmp_cursor[level].node->num_keys) / 2);
	if (!n || !btree_shift_to_prev(root, level, tmp_cursor, n))
		return -1;

	*index -= n;
	return 0;
}

static int
btree_insert_shift_to_next(
	struct btree_root	*root,
	int			level,
	int			*index)
{
	struct btree_cursor	tmp_cursor[root->height];
	int			n;

	if (*index >= BTREE_KEY_MAX)
		return -1;

	if (!btree_copy_cursor_next(root, tmp_cursor, level + 1))
		return -1;

	n = MIN(BTREE_KEY_MAX - *index,
		(BTREE_PTR_MAX - tmp_cursor[level].node->num_keys) / 2);
	if (!n || !btree_shift_to_next(root, level, tmp_cursor, n))
		return -1;
	return 0;
}

static int
btree_insert_item(
	struct btree_root	*root,
	int			level,
	unsigned long		key,
	void			*value)
{
	struct btree_node	*node = root->cursor[level].node;
	int			index = root->cursor[level].index;
	int			i;

	if (node->num_keys == BTREE_KEY_MAX) {
		if (btree_insert_shift_to_prev(root, level, &index) == 0)
			goto insert;
		if (btree_insert_shift_to_next(root, level, &index) == 0)
			goto insert;
		if (level == root->height - 1) {
			if (!btree_increase_height(root))
				return ENOMEM;
		}
		node = btree_split(root, level, key, &index);
		if (!node)
			return ENOMEM;
	}
insert:
	ASSERT(index <= node->num_keys);

	i = node->num_keys;
	node->ptrs[i + 1] = node->ptrs[i];
	while (--i >= index) {
		node->keys[i + 1] = node->keys[i];
		node->ptrs[i + 1] = node->ptrs[i];
	}

	node->num_keys++;
	node->keys[index] = key;

	if (level == 0)
		node->ptrs[index] = value;
	else
		node->ptrs[index + 1] = value;

	return 0;
}



int
btree_insert(
	struct btree_root	*root,
	unsigned long		key,
	void			*value)
{
	int			result;

	if (!value)
		return EINVAL;

	if (btree_search(root, key) && root->cur_key == key)
		return EEXIST;

#ifdef BTREE_STATS
	root->stats.insert += 1;
	root->stats.num_items += 1;
#endif

	result = btree_insert_item(root, 0, key, value);

	btree_invalidate_cursor(root);

	return result;
}


/*
 * Deletion functions
 *
 * Rather more complicated as deletions has 4 ways to go once a node
 * ends up with less than the minimum number of keys:
 *   - move remainder to previous node
 *   - move remainder to next node
 *       (both will involve a parent deletion which may recurse)
 *   - balance by moving some items from previous node
 *   - balance by moving some items from next node
 */

static void
btree_decrease_height(
	struct btree_root	*root)
{
	struct btree_node	*old_root = root->root_node;

	ASSERT(old_root->num_keys == 0);

#ifdef BTREE_STATS
	root->stats.alloced -= 1;
	root->stats.dec_height += 1;
	root->stats.max_items /= BTREE_PTR_MAX;
#endif
	root->root_node = old_root->ptrs[0];
	btree_node_free(old_root);
	root->height--;
}

static int
btree_merge_with_prev(
	struct btree_root	*root,
	int			level,
	struct btree_cursor	*prev_cursor)
{
	if (!prev_cursor)
		return 0;

	if (!btree_shift_to_prev(root, level, prev_cursor,
					root->cursor[level].node->num_keys + 1))
		return 0;

#ifdef BTREE_STATS
	root->stats.merge_prev += 1;
#endif
	return 1;
}

static int
btree_merge_with_next(
	struct btree_root	*root,
	int			level,
	struct btree_cursor	*next_cursor)
{
	if (!next_cursor)
		return 0;

	if (!btree_shift_to_next(root, level, next_cursor,
					root->cursor[level].node->num_keys + 1))
		return 0;

#ifdef BTREE_STATS
	root->stats.merge_next += 1;
#endif
	return 1;
}

static int
btree_balance_with_prev(
	struct btree_root	*root,
	int			level,
	struct btree_cursor	*prev_cursor)
{
	struct btree_cursor	*root_cursor = root->cursor;

	if (!prev_cursor)
		return 0;
	ASSERT(prev_cursor[level].node->num_keys > BTREE_KEY_MIN);

#ifdef BTREE_STATS
	root->stats.balance_prev += 1;
#endif
	/*
	 * Move some nodes from the prev node into the current node.
	 * As the shift operation is a right shift and is relative to
	 * the root cursor, make the root cursor the prev cursor and
	 * pass in the root cursor as the next cursor.
	 */

	root->cursor = prev_cursor;
	if (!btree_shift_to_next(root, level, root_cursor,
		(prev_cursor[level].node->num_keys + 1 - BTREE_KEY_MIN) / 2))
			abort();
	root->cursor = root_cursor;

	return 1;
}

static int
btree_balance_with_next(
	struct btree_root	*root,
	int			level,
	struct btree_cursor	*next_cursor)
{
	struct btree_cursor	*root_cursor = root->cursor;

	if (!next_cursor)
		return 0;
	assert(next_cursor[level].node->num_keys > BTREE_KEY_MIN);

#ifdef btree_stats
	root->stats.balance_next += 1;
#endif
	/*
	 * move some nodes from the next node into the current node.
	 * as the shift operation is a left shift and is relative to
	 * the root cursor, make the root cursor the next cursor and
	 * pass in the root cursor as the prev cursor.
	 */

	root->cursor = next_cursor;
	if (!btree_shift_to_prev(root, level, root_cursor,
		(next_cursor[level].node->num_keys + 1 - BTREE_KEY_MIN) / 2))
			abort();
	root->cursor = root_cursor;

	return 1;

}

static void
btree_delete_key(
	struct btree_root	*root,
	int			level);

/*
 * btree_delete_node:
 *
 * Return 0 if it's done or 1 if the next level needs to be collapsed
 */
static void
btree_delete_node(
	struct btree_root	*root,
	int			level)
{
	struct btree_cursor	prev_cursor[root->height];
	struct btree_cursor	next_cursor[root->height];
	struct btree_cursor	*pc;
	struct btree_cursor	*nc;

	/*
	 * the node has underflowed, grab or merge keys/items from a
	 * neighbouring node.
	 */

	if (level == root->height - 1) {
		if (level > 0 && root->root_node->num_keys == 0)
			btree_decrease_height(root);
		return;
	}

	pc = btree_copy_cursor_prev(root, prev_cursor, level + 1);
	if (!btree_merge_with_prev(root, level, pc)) {
		nc = btree_copy_cursor_next(root, next_cursor, level + 1);
		if (!btree_merge_with_next(root, level, nc)) {
			/* merging failed, try redistrubution */
			if (!btree_balance_with_prev(root, level, pc) &&
			    !btree_balance_with_next(root, level, nc))
				abort();
			return;	/* when balancing, then the node isn't freed */
		}
	}

#ifdef BTREE_STATS
	root->stats.alloced -= 1;
#endif
	btree_node_free(root->cursor[level].node);

	btree_delete_key(root, level + 1);
}

static void
btree_delete_key(
	struct btree_root	*root,
	int			level)
{
	struct btree_node	*node = root->cursor[level].node;
	int			index = root->cursor[level].index;

	node->num_keys--;
	if (index <= node->num_keys) {
		/*
		 * if not deleting the last item, shift higher items down
		 * to cover the item being deleted
		 */
		while (index < node->num_keys) {
			node->keys[index] = node->keys[index + 1];
			node->ptrs[index] = node->ptrs[index + 1];
			index++;
		}
		node->ptrs[index] = node->ptrs[index + 1];
	} else {
		/*
		 * else update the associated parent key as the last key
		 * in the leaf has changed
		 */
		btree_update_node_key(root, root->cursor, level + 1,
						node->keys[node->num_keys]);
	}
	/*
	 * if node underflows, either merge with sibling or rebalance
	 * with sibling.
	 */
	if (node->num_keys < BTREE_KEY_MIN)
		btree_delete_node(root, level);
}

void *
btree_delete(
	struct btree_root	*root,
	unsigned long		key)
{
	void			*value;

	value = btree_lookup(root, key);
	if (!value)
		return NULL;

#ifdef BTREE_STATS
	root->stats.delete += 1;
	root->stats.num_items -= 1;
#endif

	btree_delete_key(root, 0);

	btree_invalidate_cursor(root);

	return value;
}

#ifdef BTREE_STATS
void
btree_print_stats(
	struct btree_root	*root,
	FILE			*f)
{
	unsigned long		max_items = root->stats.max_items *
						(root->root_node->num_keys + 1);

	fprintf(f, "\tnum_items = %lu, max_items = %lu (%lu%%)\n",
			root->stats.num_items, max_items,
			root->stats.num_items * 100 / max_items);
	fprintf(f, "\talloced = %d nodes, %lu bytes, %lu bytes per item\n",
			root->stats.alloced,
			root->stats.alloced * sizeof(struct btree_node),
			root->stats.alloced * sizeof(struct btree_node) /
							root->stats.num_items);
	fprintf(f, "\tlookup = %d\n", root->stats.lookup);
	fprintf(f, "\tfind = %d\n", root->stats.find);
	fprintf(f, "\tcache_hits = %d\n", root->stats.cache_hits);
	fprintf(f, "\tcache_misses = %d\n", root->stats.cache_misses);
	fprintf(f, "\tkey_update = %d\n", root->stats.key_update);
	fprintf(f, "\tvalue_update = %d\n", root->stats.value_update);
	fprintf(f, "\tinsert = %d\n", root->stats.insert);
	fprintf(f, "\tshift_prev = %d\n", root->stats.shift_prev);
	fprintf(f, "\tshift_next = %d\n", root->stats.shift_next);
	fprintf(f, "\tsplit = %d\n", root->stats.split);
	fprintf(f, "\tinc_height = %d\n", root->stats.inc_height);
	fprintf(f, "\tdelete = %d\n", root->stats.delete);
	fprintf(f, "\tmerge_prev = %d\n", root->stats.merge_prev);
	fprintf(f, "\tmerge_next = %d\n", root->stats.merge_next);
	fprintf(f, "\tbalance_prev = %d\n", root->stats.balance_prev);
	fprintf(f, "\tbalance_next = %d\n", root->stats.balance_next);
	fprintf(f, "\tdec_height = %d\n", root->stats.dec_height);
}
#endif
