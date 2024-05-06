#ifndef SRC_MOD_COMMON_RTRIE_H_
#define SRC_MOD_COMMON_RTRIE_H_

/**
 * @file
 * A Radix Trie.
 *
 * Why don't we use the kernel's radix trie instead?
 * Because it's only good for keys long-sized; we need 128-bit keys.
 */

#include <linux/types.h>
#include <linux/mutex.h>

struct rtrie_key {
	__u8 *bytes;
	/* In bits; not bytes. */
	__u8 len;
};

enum rtrie_color {
	COLOR_BLACK,
	COLOR_WHITE,
};

/**
 * Some fields here are RCU-friendly and others aren't.
 *
 * RCU-friendly fields can be dereferenced in RCU-protected areas happily,
 * and of course MUST NOT BE EDITED while the node is in the trie.
 *
 * RCU-unfriendly fields must not be touched outside the domain of the trie's
 * lock.
 */
struct rtrie_node {
	/** RCU-friendly. */
	struct rtrie_node __rcu *left;
	/** RCU-friendly. */
	struct rtrie_node __rcu *right;
	/** NOT RCU-friendly. */
	struct rtrie_node *parent;

	/**
	 * RCU-friendly.
	 *
	 * If you want to assign a different value here, consider:
	 *
	 * - Black nodes cannot be changed into white nodes since white nodes
	 *   are supposed to contain a value (black nodes only need a key).
	 * - White nodes can be changed into black nodes as long as they're
	 *   not attached to tries.
	 */
	enum rtrie_color color;
	/** RCU-friendly. */
	struct rtrie_key key;

	/**
	 * This is used to foreach all the nodes (whether black or white).
	 *
	 * NOT RCU-friendly.
	 */
	struct list_head list_hook;

	/* The value hangs off end. RCU-friendly. */
};

struct rtrie {
	/** The tree. */
	struct rtrie_node __rcu *root;
	/** @root's nodes chained to ease foreaching. */
	struct list_head list;
	/** Size of the values being stored (in bytes). */
	size_t value_size;

	/**
	 * Notice that this is a pointer.
	 * Locking is actually the caller's responsibility; the only reason why
	 * the trie keeps track of it is for the sake of RCU validation.
	 */
	struct mutex *lock;
};

void rtrie_init(struct rtrie *trie, size_t size, struct mutex *lock);
void rtrie_clean(struct rtrie *trie);

/* Safe-to-use-during-packet-translation functions */

int rtrie_find(struct rtrie *trie, struct rtrie_key *key, void *result);
bool rtrie_contains(struct rtrie *trie, struct rtrie_key *key);
bool rtrie_is_empty(struct rtrie *trie);
void rtrie_print(char const *prefix, struct rtrie *trie);

/* Lock-before-using functions. */

/*
 * The "synchronize" flag controls whether RCU synchronization is performed.
 * By default, send true. If you absolutely know for sure that there are no
 * readers, send false.
 * This flag exists because synchronize_rcu() has shown to be cripplingly slow
 * in some systems: https://github.com/NICMx/Jool/issues/363
 */

int rtrie_add(struct rtrie *trie, void *value, size_t key_offset, __u8 key_len,
		bool synchronize);
int rtrie_rm(struct rtrie *trie, struct rtrie_key *key, bool synchronize);
void rtrie_flush(struct rtrie *trie);

typedef int (*rtrie_foreach_cb)(void const *, void *);
int rtrie_foreach(struct rtrie *trie,
		rtrie_foreach_cb cb, void *arg,
		struct rtrie_key *offset);

#endif /* SRC_MOD_COMMON_RTRIE_H_ */
