#include "mod/common/rtrie.h"

#include <linux/rcupdate.h>

#include "common/types.h"
#include "mod/common/log.h"
#include "mod/common/rcu.h"
#include "mod/common/wkmalloc.h"


#define deref_reader(node) \
	rcu_dereference_bh(node)
#define deref_updater(trie, node) \
	rcu_dereference_protected(node, lockdep_is_held(trie->lock))
#define deref_both(trie, node) \
	rcu_dereference_bh_check(node, lockdep_is_held(trie->lock))

/**
 * Returns the number of bytes you need to be able to store @bits bits.
 *
 * Basically `ceiling(bits / 8)`.
 */
static __u8 bits_to_bytes(__u8 bits)
{
	return (bits != 0u) ? (((bits - 1u) >> 3) + 1u) : 0u;
}

static struct rtrie_node *create_inode(struct rtrie_key *key,
		struct rtrie_node *left_child,
		struct rtrie_node *right_child)
{
	struct rtrie_node *inode;
	__u8 key_len;

	key_len = bits_to_bytes(key->len);
	inode = __wkmalloc("Rtrie node", sizeof(*inode) + key_len, GFP_ATOMIC);
	if (!inode)
		return NULL;

	RCU_INIT_POINTER(inode->left, left_child);
	RCU_INIT_POINTER(inode->right, right_child);
	inode->parent = NULL;
	inode->color = COLOR_BLACK;
	INIT_LIST_HEAD(&inode->list_hook);
	inode->key.bytes = (__u8 *) (inode + 1);
	inode->key.len = key->len;
	memcpy(inode->key.bytes, key->bytes, key_len);

	return inode;
}

static struct rtrie_node *create_leaf(void *content, size_t content_len,
		size_t key_offset, __u8 key_len)
{
	struct rtrie_node *leaf;

	leaf = __wkmalloc("Rtrie node", sizeof(*leaf) + content_len, GFP_ATOMIC);
	if (!leaf)
		return NULL;

	RCU_INIT_POINTER(leaf->left, NULL);
	RCU_INIT_POINTER(leaf->right, NULL);
	leaf->parent = NULL;
	leaf->color = COLOR_WHITE;
	INIT_LIST_HEAD(&leaf->list_hook);
	leaf->key.bytes = ((__u8 *) (leaf + 1)) + key_offset;
	leaf->key.len = key_len;
	memcpy(leaf + 1, content, content_len);

	return leaf;
}

/**
 * Zero-based, left to right. Eg. get_bit(0b00100000, 2) returns 1.
 */
static unsigned int get_bit(__u8 byte, unsigned int pos)
{
	return (byte >> (7u - pos)) & 1u;
}

static unsigned int __key_match(struct rtrie_key *key1, struct rtrie_key *key2,
		unsigned int bits)
{
	unsigned int result = 0;
	unsigned int y, i; /* b[y]te counter, b[i]t counter. */
	unsigned int bytes;
	unsigned int bit1, bit2;

	bytes = bits >> 3; /* ">> 3" = "/ 8" */
	bits &= 7; /* "& 7" = "% 8" */

	for (y = 0; y < bytes; y++) {
		if (key1->bytes[y] != key2->bytes[y]) {
			bits = 8;
			break;
		}
		result += 8;
	}

	for (i = 0; i < bits; i++) {
		bit1 = get_bit(key1->bytes[y], i);
		bit2 = get_bit(key2->bytes[y], i);

		if (bit1 != bit2)
			break;

		result++;
	}

	return result;
}

/**
 * match - Returns the number of prefix bits @key1 and @key2 have in common.
 */
static unsigned int key_match(struct rtrie_key *key1, struct rtrie_key *key2)
{
	return __key_match(key1, key2, min(key1->len, key2->len));
}

/**
 * Returns true if @key1 is a prefix of @key2, false otherwise.
 *
 * The name can go both ways so it might be confusing. Think of it like this:
 * If @key1 is 2001:db8::/32 and @key2 is 2001:db8:1::/64, then @key1 contains
 * @key2.
 */
static bool key_contains(struct rtrie_key *key1, struct rtrie_key *key2)
{
	return (key2->len >= key1->len)
			? (__key_match(key1, key2, key1->len) == key1->len)
			: false;
}

static bool key_equals(struct rtrie_key *key1, struct rtrie_key *key2)
{
	return (key1->len == key2->len)
			? (__key_match(key1, key2, key1->len) == key1->len)
			: false;
}

void rtrie_init(struct rtrie *trie, size_t size, struct mutex *lock)
{
	trie->root = NULL;
	INIT_LIST_HEAD(&trie->list);
	trie->value_size = size;
	trie->lock = lock;
}

void rtrie_clean(struct rtrie *trie)
{
	struct rtrie_node *node;
	struct rtrie_node *tmp_node;

	/* rtrie_print("Destroying trie", trie); */
	list_for_each_entry_safe(node, tmp_node, &trie->list, list_hook) {
		list_del(&node->list_hook);
		__wkfree("Rtrie node", node);
	}

	/* rtrie_print("Trie after", trie); */
}

/**
 * find_longest_common_prefix - Returns the node from @trie which best matches
 * @key.
 *
 * If you're a reader, you need to "lock" RCU reads before calling.
 *
 * @force_white: if true, the resulting node will be the best white match.
 *	If false, the result will be the best match, regardless of color.
 */
static struct rtrie_node *find_longest_common_prefix(struct rtrie *trie,
		struct rtrie_key *key, bool force_white)
{
	struct rtrie_node *node;
	struct rtrie_node *child;
	struct rtrie_node *last_white;

	node = deref_both(trie, trie->root);
	if (!node || !key_contains(&node->key, key))
		return NULL;

	last_white = NULL;
	do {
		if (node->color == COLOR_WHITE)
			last_white = node;

		child = deref_both(trie, node->left);
		if (child && key_contains(&child->key, key)) {
			node = child;
			continue;
		}

		child = deref_both(trie, node->right);
		if (child && key_contains(&child->key, key)) {
			node = child;
			continue;
		}

		return force_white ? last_white : node;
	} while (true);

	return NULL; /* <-- Shuts up Eclipse. */
}

/**
 * This must only be called by updater code.
 */
static struct rtrie_node __rcu **get_parent_ptr(struct rtrie *trie,
		struct rtrie_node *node)
{
	struct rtrie_node *parent = node->parent;

	if (!parent)
		return &trie->root;

	return (deref_updater(trie, parent->left) == node)
			? &parent->left
			: &parent->right;
}

static void swap_nodes(struct rtrie *trie, struct rtrie_node *old,
		struct rtrie_node *new)
{
	struct rtrie_node __rcu **parent_ptr;

	new->left = old->left;
	new->right = old->right;
	new->parent = old->parent;

	parent_ptr = get_parent_ptr(trie, old);
	rcu_assign_pointer(*parent_ptr, new);

	list_add(&new->list_hook, &trie->list);
	list_del(&old->list_hook);

	__wkfree("Rtrie node", old);
}

static int add_to_root(struct rtrie *trie, struct rtrie_node *new)
{
	struct rtrie_node *root = deref_updater(trie, trie->root);
	struct rtrie_node *inode;
	struct rtrie_key key;

	if (!root) {
		rcu_assign_pointer(trie->root, new);
		list_add(&new->list_hook, &trie->list);
		return 0;
	}

	if (key_contains(&new->key, &root->key)) {
		RCU_INIT_POINTER(new->left, root);
		root->parent = new;
		list_add(&new->list_hook, &trie->list);
		rcu_assign_pointer(trie->root, new);
		return 0;
	}

	key.bytes = new->key.bytes;
	key.len = key_match(&root->key, &new->key);

	inode = create_inode(&key, root, new);
	if (!inode)
		return -ENOMEM;

	root->parent = inode;
	new->parent = inode;
	list_add(&inode->list_hook, &trie->list);
	list_add(&new->list_hook, &trie->list);

	rcu_assign_pointer(trie->root, inode);
	return 0;
}

static int add_full_collision(struct rtrie *trie, struct rtrie_node *parent,
		struct rtrie_node *new, bool synchronize)
{
	/*
	 * We're adding new to
	 *
	 * parent
	 *    |
	 *    +---- child1
	 *    |
	 *    +---- child2
	 *
	 * We need to turn it into this:
	 *
	 * parent
	 *    |
	 *    +---- smallest_prefix_node
	 *    |
	 *    +---- inode
	 *             |
	 *             +---- higher_prefix1
	 *             |
	 *             +---- higher_prefix2
	 *
	 * { smallest_prefix_node, higher_prefix1, higher_prefix2 } is some
	 * combination from { child1, child2, new }.
	 */
	struct rtrie_node *left = deref_updater(trie, parent->left);
	struct rtrie_node *right = deref_updater(trie, parent->right);

	struct rtrie_node *smallest_prefix;
	struct rtrie_node *higher_prefix1;
	struct rtrie_node *higher_prefix2;
	struct rtrie_node *inode;
	struct rtrie_key inode_prefix;

	unsigned int match_lr = key_match(&left->key, &right->key);
	unsigned int match_ln = key_match(&left->key, &new->key);
	unsigned int match_rn = key_match(&right->key, &new->key);

	if (match_lr > match_ln && match_lr > match_rn) {
		smallest_prefix = new;
		higher_prefix1 = left;
		higher_prefix2 = right;
		inode_prefix.len = match_lr;
	} else if (match_ln > match_lr && match_ln > match_rn) {
		smallest_prefix = right;
		higher_prefix1 = new;
		higher_prefix2 = left;
		inode_prefix.len = match_ln;
	} else if (match_rn > match_lr && match_rn > match_ln) {
		smallest_prefix = left;
		higher_prefix1 = new;
		higher_prefix2 = right;
		inode_prefix.len = match_rn;
	} else {
		WARN(true, "Inconsistent bwrtrie! (%u %u %u)",
				match_lr, match_ln, match_rn);
		return -EINVAL;
	}
	inode_prefix.bytes = higher_prefix1->key.bytes;

	inode = create_inode(&inode_prefix, higher_prefix1, higher_prefix2);
	if (!inode)
		return -ENOMEM;

	rcu_assign_pointer(parent->left, NULL);
	rcu_assign_pointer(parent->right, NULL);
	if (synchronize)
		synchronize_rcu_bh();
	rcu_assign_pointer(parent->left, smallest_prefix);
	rcu_assign_pointer(parent->right, inode);

	smallest_prefix->parent = parent;
	inode->parent = parent;
	higher_prefix1->parent = inode;
	higher_prefix2->parent = inode;
	list_add(&inode->list_hook, &trie->list);
	list_add(&new->list_hook, &trie->list);

	return 0;
}

int rtrie_add(struct rtrie *trie, void *value, size_t key_offset, __u8 key_len,
		bool synchronize)
{
	struct rtrie_node *new;
	struct rtrie_node *parent;
	struct rtrie_node *left, *right;
	bool contains_left;
	bool contains_right;

	new = create_leaf(value, trie->value_size, key_offset, key_len);
	if (!new)
		return -ENOMEM;

	parent = find_longest_common_prefix(trie, &new->key, false);
	if (!parent)
		return add_to_root(trie, new);

	if (key_equals(&parent->key, &new->key)) {
		if (parent->color == COLOR_BLACK) {
			swap_nodes(trie, parent, new);
			return 0;
		}
		__wkfree("Rtrie node", new);
		return -EEXIST;
	}

	if (!parent->left) {
		rcu_assign_pointer(parent->left, new);
		goto simple_success;
	}
	if (!parent->right) {
		rcu_assign_pointer(parent->right, new);
		goto simple_success;
	}

	left = deref_updater(trie, parent->left);
	right = deref_updater(trie, parent->right);
	contains_left = key_contains(&new->key, &left->key);
	contains_right = key_contains(&new->key, &right->key);

	if (contains_left && contains_right) {
		if (parent->color == COLOR_BLACK) {
			swap_nodes(trie, parent, new);
			return 0;
		}

		RCU_INIT_POINTER(new->left, left);
		RCU_INIT_POINTER(new->right, right);
		rcu_assign_pointer(parent->left, NULL);
		rcu_assign_pointer(parent->right, NULL);
		if (synchronize)
			synchronize_rcu_bh();
		rcu_assign_pointer(parent->right, new);

		left->parent = new;
		right->parent = new;
		goto simple_success;
	}

	if (contains_left) {
		RCU_INIT_POINTER(new->left, left);
		rcu_assign_pointer(parent->left, new);

		left->parent = new;
		goto simple_success;
	}

	if (contains_right) {
		RCU_INIT_POINTER(new->right, right);
		rcu_assign_pointer(parent->right, new);

		right->parent = new;
		goto simple_success;
	}

	return add_full_collision(trie, parent, new, synchronize);

simple_success:
	new->parent = parent;
	list_add(&new->list_hook, &trie->list);
	return 0;
}

/**
 * rtrie_find - Finds the node keyed @key, and copies its value to @result.
 */
int rtrie_find(struct rtrie *trie, struct rtrie_key *key, void *result)
{
	struct rtrie_node *node;

	rcu_read_lock_bh();

	node = find_longest_common_prefix(trie, key, true);
	if (!node) {
		rcu_read_unlock_bh();
		return -ESRCH;
	}

	memcpy(result, node + 1, trie->value_size);
	rcu_read_unlock_bh();
	return 0;
}

bool rtrie_contains(struct rtrie *trie, struct rtrie_key *key)
{
	bool result;

	rcu_read_lock_bh();
	result = !!find_longest_common_prefix(trie, key, true);
	rcu_read_unlock_bh();

	return result;
}

bool rtrie_is_empty(struct rtrie *trie)
{
	bool result;

	rcu_read_lock_bh();
	result = !!deref_reader(trie->root);
	rcu_read_unlock_bh();

	return result;
}

int rtrie_rm(struct rtrie *trie, struct rtrie_key *key, bool synchronize)
{
	struct rtrie_node *node;
	struct rtrie_node *new;
	struct rtrie_node *parent;
	struct rtrie_node __rcu **parent_ptr;

	node = find_longest_common_prefix(trie, key, true);
	if (!node || !key_equals(&node->key, key))
		return -ESRCH;

	if (node->left && node->right) {
		new = create_inode(&node->key,
				deref_updater(trie, node->left),
				deref_updater(trie, node->right));
		if (!new)
			return -ENOMEM;

		parent = node->parent;
		parent_ptr = get_parent_ptr(trie, node);

		rcu_assign_pointer(*parent_ptr, new);
		if (synchronize)
			synchronize_rcu_bh();

		deref_updater(trie, new->left)->parent = new;
		deref_updater(trie, new->right)->parent = new;
		list_add(&new->list_hook, &trie->list);
		list_del(&node->list_hook);
		__wkfree("Rtrie node", node);
		return 0;
	}

	/* Keep pruning unnecessary nodes. */
	do {
		/*
		 * At this point, node cannot have two children,
		 * so it's going down.
		 */
		parent = node->parent;
		parent_ptr = get_parent_ptr(trie, node);

		if (node->left) {
			rcu_assign_pointer(*parent_ptr, node->left);
			if (synchronize)
				synchronize_rcu_bh();
			deref_updater(trie, node->left)->parent = parent;
			list_del(&node->list_hook);
			__wkfree("Rtrie node", node);
			return 0;
		}

		if (node->right) {
			rcu_assign_pointer(*parent_ptr, node->right);
			if (synchronize)
				synchronize_rcu_bh();
			deref_updater(trie, node->right)->parent = parent;
			list_del(&node->list_hook);
			__wkfree("Rtrie node", node);
			return 0;
		}

		rcu_assign_pointer(*parent_ptr, NULL);
		if (synchronize)
			synchronize_rcu_bh();
		list_del(&node->list_hook);
		__wkfree("Rtrie node", node);

		node = parent;
	} while (node && node->color == COLOR_BLACK);

	return 0;
}

void rtrie_flush(struct rtrie *trie)
{
	struct rtrie_node *node;
	struct rtrie_node *tmp_node;
	struct list_head tmp_list;

	/* rtrie_print("Flushing trie", trie); */

	if (!deref_updater(trie, trie->root))
		return;

	rcu_assign_pointer(trie->root, NULL);
	list_replace_init(&trie->list, &tmp_list);

	synchronize_rcu_bh();

	list_for_each_entry_safe(node, tmp_node, &tmp_list, list_hook) {
		list_del(&node->list_hook);
		__wkfree("Rtrie node", node);
	}
}

/**
 * TODO (performance) find offset using a normal trie find.
 */
int rtrie_foreach(struct rtrie *trie,
		rtrie_foreach_cb cb, void *arg,
		struct rtrie_key *offset)
{
	struct rtrie_node *node;
	int error;

	if (list_empty(&trie->list))
		return 0;

	list_for_each_entry(node, &trie->list, list_hook) {
		if (offset) {
			if (key_equals(offset, &node->key))
				offset = NULL;
		} else if (node->color == COLOR_WHITE) {
			error = cb(node + 1, arg);
			if (error)
				return error;
		}
	}

	return offset ? -ESRCH : 0;
}

static char *color2str(enum rtrie_color color)
{
	switch (color) {
	case COLOR_WHITE:
		return "w";
	case COLOR_BLACK:
		return "b";
	}
	return "u";
}

static void print_node(struct rtrie_node *node, unsigned int level)
{
	unsigned int i, j;
	unsigned int remainder;

	if (!node)
		return;

	for (i = 0; i < level; i++)
		printk("| ");

	printk("(%s) ", color2str(node->color));

	for (i = 0; i < (node->key.len >> 3); i++)
		printk("%02x", node->key.bytes[i]);
	remainder = node->key.len & 7u;
	if (remainder) {
		printk(" ");
		for (j = 0; j < remainder; j++)
			printk("%u", (node->key.bytes[i] >> (7u - j)) & 1u);
	}

	printk(" (/%u)", node->key.len);
	printk(" %p %p %p", &node->list_hook, node->list_hook.prev,
			node->list_hook.next);
	printk("\n");

	print_node(deref_reader(node->left), level + 1);
	print_node(deref_reader(node->right), level + 1);
}

/**
 * rtrie_print - print the trie in dmesg.
 *
 * This function is recursive. Do not use in production code.
 */
void rtrie_print(char const *prefix, struct rtrie *trie)
{
	struct rtrie_node *root;

	printk(KERN_DEBUG "%s:\n", prefix);
	printk("-----------------------\n");

	printk("root: %p %p %p\n", &trie->list, trie->list.prev, trie->list.next);

	rcu_read_lock_bh();

	root = deref_reader(trie->root);
	if (root) {
		print_node(root, 0);
	} else {
		printk("  (empty)\n");
	}

	rcu_read_unlock_bh();

	printk("-----------------------\n");
}
