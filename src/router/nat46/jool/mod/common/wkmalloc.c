/**
 * This is exactly the same as kmemleak.
 * https://www.kernel.org/doc/Documentation/kmemleak.txt
 * I didn't want to recompile the kernel yet again so I made this.
 * It's nowhere near as good but it took much less time. Sorry.
 *
 * Include in the compilation by adding JKMEMLEAK to CFLAGS.
 *
 * 	make CFLAGS_MODULE=-DJKMEMLEAK
 *
 * Will print a report when you modprobe -r.
 */

#ifdef JKMEMLEAK

#include "mod/common/wkmalloc.h"

#include "mod/common/log.h"
#include "mod/common/db/rbtree.h"

struct kmalloc_entry {
	unsigned int count;
	struct rb_node hook;
};

static DEFINE_SPINLOCK(lock);
static struct rb_root tree = RB_ROOT;

static struct kmalloc_entry *kmn_entry(struct rb_node *hook)
{
	return rb_entry(hook, struct kmalloc_entry, hook);
}

static char *kmn_name(struct kmalloc_entry *entry)
{
	return (char *)(entry + 1);
}

static int cmp_node(struct rb_node *n1, struct rb_node *n2)
{
	return strcmp(kmn_name(kmn_entry(n1)), kmn_name(kmn_entry(n2)));
}

void wkmalloc_add(const char *name)
{
	struct tree_slot slot;
	struct kmalloc_entry *entry;
	struct rb_node *collision;

	entry = kmalloc(sizeof(struct kmalloc_entry) + strlen(name) + 1,
			GFP_ATOMIC);
	if (WARN(!entry, "could not allocate a tracker."))
		return;

	entry->count = 1;
	strcpy(kmn_name(entry), name);

	spin_lock_bh(&lock);

	treeslot_init(&slot, &tree, &entry->hook);
	collision = rbtree_find_slot(&entry->hook, &tree, cmp_node, &slot);
	if (collision) {
		kfree(entry);
		entry = kmn_entry(collision);
		entry->count++;
	} else {
		treeslot_commit(&slot);
	}

	spin_unlock_bh(&lock);
}

static int cmp_kmn(struct kmalloc_entry *entry, const char *str)
{
	return strcmp(kmn_name(entry), str);
}

void wkmalloc_rm(const char *name, void *obj)
{
	struct kmalloc_entry *entry;

	if (!obj)
		return;

	spin_lock_bh(&lock);

	entry = rbtree_find(name, &tree, cmp_kmn, struct kmalloc_entry, hook);
	if (!WARN(!entry, "Freeing out-of-tree object '%s'.", name)) {
		if (!WARN(entry->count == 0, "Freeing unallocated object '%s'.", name))
			entry->count--;
	}

	spin_unlock_bh(&lock);
}

void wkmalloc_print_leaks(void)
{
	struct rb_node *node;
	struct kmalloc_entry *entry;
	unsigned int leaks = 0;

	log_info("Memory leaks:");
	for (node = rb_first(&tree); node; node = rb_next(node)) {
		entry = kmn_entry(node);
		if (entry->count > 0) {
			pr_err("- %s: %d\n", kmn_name(entry), entry->count);
			leaks++;
		}
	}

	if (!WARN(leaks, "Total leaks: %u", leaks))
		log_info("None.");
}

void wkmalloc_teardown(void)
{
	struct kmalloc_entry *entry, *tmp;
	rbtree_foreach(entry, tmp, &tree, hook)
		kfree(entry);
	tree.rb_node = NULL;
}

#endif /* JKMEMLEAK */
