/*
 * mm/memleak.c
 *
 * Copyright (C) 2006 ARM Limited
 * Written by Catalin Marinas <catalin.marinas@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* #define DEBUG */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/radix-tree.h>
#include <linux/gfp.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kallsyms.h>
#include <linux/mman.h>
#include <linux/nodemask.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/vmalloc.h>
#include <linux/mutex.h>
#include <linux/cpumask.h>

#include <asm/bitops.h>
#include <asm/sections.h>
#include <asm/percpu.h>
#include <asm/processor.h>
#include <asm/thread_info.h>
#include <asm/atomic.h>

#include <linux/memleak.h>

#ifdef CONFIG_FRAME_POINTER
#define MAX_TRACE		CONFIG_DEBUG_MEMLEAK_TRACE_LENGTH
#else
#define MAX_TRACE		1
#endif

#define SCAN_BLOCK_SIZE		4096		/* maximum scan length with interrupts disabled */
#define PREINIT_POINTERS	CONFIG_DEBUG_MEMLEAK_PREINIT_POINTERS
#define BYTES_PER_WORD		sizeof(void *)

extern struct memleak_offset __memleak_offsets_start[];
extern struct memleak_offset __memleak_offsets_end[];

struct memleak_alias {
	struct hlist_node node;
	unsigned long offset;
};

struct memleak_scan_area {
	struct hlist_node node;
	unsigned long offset;
	size_t length;
};

struct memleak_pointer {
	struct list_head pointer_list;
	struct list_head gray_list;
	int use_count;
	unsigned long pointer;
	unsigned long offset;		/* padding */
	size_t size;
	int ref_count;			/* the minimum encounters of the value */
	int count;			/* the ecounters of the value */
	struct hlist_head *alias_list;
	struct hlist_head area_list;	/* areas to be scanned (or empty for all) */
	unsigned long trace[MAX_TRACE];
};

typedef enum {
	MEMLEAK_ALLOC,
	MEMLEAK_FREE,
	MEMLEAK_PADDING,
	MEMLEAK_NOT_LEAK,
	MEMLEAK_IGNORE,
	MEMLEAK_SCAN_AREA
} memleak_action_t;

struct memleak_preinit_pointer {
	memleak_action_t type;
	const void *pointer;
	unsigned long offset;
	size_t size;
	int ref_count;
};

/* Pointer colors, encoded with count and ref_count:
 *   - white - orphan block, i.e. not enough references to it (ref_count >= 1)
 *   - gray  - referred at least once and therefore non-orphan (ref_count == 0)
 *   - black - ignore; it doesn't contain references (text section) (ref_count == -1)
 */
#define COLOR_WHITE(pointer)	((pointer)->count != -1 && (pointer)->count < (pointer)->ref_count)
#define COLOR_GRAY(pointer)	((pointer)->ref_count != -1 && (pointer)->count >= (pointer)->ref_count)
#define COLOR_BLACK(pointer)	((pointer)->ref_count == -1)

/* Tree storing the pointer aliases indexed by size */
static RADIX_TREE(alias_tree, GFP_ATOMIC);
/* Tree storing all the possible pointers, indexed by the pointer value */
static RADIX_TREE(pointer_tree, GFP_ATOMIC);
/* The list of all allocated pointers */
static LIST_HEAD(pointer_list);
/* The list of the gray pointers */
static LIST_HEAD(gray_list);

static kmem_cache_t *pointer_cache;
/* Used to avoid recursive call via the kmalloc/kfree functions */
static spinlock_t memleak_lock = SPIN_LOCK_UNLOCKED;
static cpumask_t memleak_cpu_mask = CPU_MASK_NONE;
static DEFINE_MUTEX(memleak_mutex);
static atomic_t memleak_initialized = ATOMIC_INIT(0);
static int __initdata preinit_pos = 0;
static struct memleak_preinit_pointer __initdata preinit_pointers[PREINIT_POINTERS];
/* last allocated pointer (optimization); protected by memleak_lock */
static struct memleak_pointer *last_pointer = NULL;

static void dump_pointer_info(struct memleak_pointer *pointer)
{
#ifdef CONFIG_KALLSYMS
	char namebuf[KSYM_NAME_LEN + 1] = "";
	char *modname;
	unsigned long symsize;
	unsigned long offset = 0;
#endif
#ifdef DEBUG
	struct memleak_alias *alias;
	struct hlist_node *elem;
#endif
	int i;

	printk(KERN_NOTICE "kmemleak: pointer 0x%08lx:\n", pointer->pointer);
#ifdef DEBUG
	printk(KERN_NOTICE "  size = %d\n", pointer->size);
	printk(KERN_NOTICE "  ref_count = %d\n", pointer->ref_count);
	printk(KERN_NOTICE "  count = %d\n", pointer->count);
	printk(KERN_NOTICE "  aliases:\n");
	if (pointer->alias_list)
		hlist_for_each_entry(alias, elem, pointer->alias_list, node)
			printk(KERN_NOTICE "    0x%lx\n", alias->offset);
	printk(KERN_NOTICE "  trace:\n");
#endif
	for (i = 0; i < MAX_TRACE; i++) {
		unsigned long trace = pointer->trace[i];

		if (!trace)
			break;
#ifdef CONFIG_KALLSYMS
		kallsyms_lookup(trace, &symsize, &offset, &modname, namebuf);
		printk(KERN_NOTICE "    %lx: <%s>\n", trace, namebuf);
#else
		printk(KERN_NOTICE "    %lx\n", trace);
#endif
	}
}

/* Insert an element into the aliases radix tree.
 * Return 0 on success.
 */
static int insert_alias(unsigned long size, unsigned long offset)
{
	int ret = 0;
	struct hlist_head *alias_list;
	struct memleak_alias *alias;

	if (size == 0 || offset == 0 || offset >= size) {
		ret = -EINVAL;
		goto out;
	}

	offset &= ~(BYTES_PER_WORD - 1);

	alias_list = radix_tree_lookup(&alias_tree, size);
	if (alias_list) {
		struct hlist_node *elem;

		hlist_for_each_entry(alias, elem, alias_list, node) {
			if (alias->offset == offset) {
				ret = -EEXIST;
				goto out;
			}
		}
	} else {
		alias_list = kmalloc(sizeof(*alias_list), GFP_ATOMIC);
		if (!alias_list)
			panic("kmemleak: cannot allocate initial memory\n");
		INIT_HLIST_HEAD(alias_list);

		ret = radix_tree_insert(&alias_tree, size, alias_list);
		if (ret)
			panic("kmemleak: cannot insert into the aliases radix tree: %d\n", ret);
	}

	alias = kmalloc(sizeof(*alias), GFP_ATOMIC);
	if (!alias)
		panic("kmemleak: cannot allocate initial memory\n");
	INIT_HLIST_NODE(&alias->node);
	alias->offset = offset;

	hlist_add_head(&alias->node, alias_list);

 out:
	return ret;
}

/* Insert pointer aliases the from the given array */
void memleak_insert_aliases(struct memleak_offset *ml_off_start,
				  struct memleak_offset *ml_off_end)
{
	struct memleak_offset *ml_off;
	int i = 0;
	unsigned long flags;
	unsigned int cpu_id;

	pr_debug("%s(0x%p, 0x%p)\n", __FUNCTION__, ml_off_start, ml_off_end);

	spin_lock_irqsave(&memleak_lock, flags);

	/* do not track the kmemleak allocated pointers */
	cpu_id = smp_processor_id();
	if (cpu_test_and_set(cpu_id, memleak_cpu_mask))
		BUG();

	/* primary aliases - container_of(member) */
	for (ml_off = ml_off_start; ml_off < ml_off_end; ml_off++)
		if (!insert_alias(ml_off->size, ml_off->offset))
			i++;
	pr_debug("kmemleak: found %d primary alias(es)\n", i);

	/* secondary aliases - container_of(container_of(member)) */
#ifdef CONFIG_DEBUG_MEMLEAK_SECONDARY_ALIASES
	for (ml_off = ml_off_start; ml_off < ml_off_end; ml_off++) {
		struct hlist_head *alias_list;
		struct memleak_alias *alias;
		struct hlist_node *elem;

		alias_list = radix_tree_lookup(&alias_tree, ml_off->member_size);
		if (!alias_list)
			continue;

		hlist_for_each_entry(alias, elem, alias_list, node)
			if (!insert_alias(ml_off->size, ml_off->offset + alias->offset))
				i++;
	}
	pr_debug("kmemleak: found %d alias(es)\n", i);
#endif

	cpu_clear(cpu_id, memleak_cpu_mask);
	spin_unlock_irqrestore(&memleak_lock, flags);
}
EXPORT_SYMBOL_GPL(memleak_insert_aliases);

static inline struct memleak_pointer *get_cached_pointer(unsigned long ptr)
{
	if (!last_pointer || ptr != last_pointer->pointer)
		last_pointer = radix_tree_lookup(&pointer_tree, ptr);
	return last_pointer;
}

static void create_pointer_aliases(struct memleak_pointer *pointer)
{
	struct memleak_alias *alias;
	struct hlist_node *elem;
	unsigned long ptr = pointer->pointer;
	int err;

	if (pointer->offset) {
		err = radix_tree_insert(&pointer_tree, ptr + pointer->offset, pointer);
		if (err) {
			dump_stack();
			panic("kmemleak: cannot insert alias into the pointer radix tree: %d\n", err);
		}
	}

	if (pointer->alias_list) {
		hlist_for_each_entry(alias, elem, pointer->alias_list, node) {
			if (alias->offset >= pointer->size)
				BUG();

			err = radix_tree_insert(&pointer_tree, ptr
						+ pointer->offset + alias->offset,
						pointer);
			if (err) {
				dump_stack();
				panic("kmemleak: cannot insert alias into the pointer radix tree: %d\n", err);
			}
		}
	}
}

static void delete_pointer_aliases(struct memleak_pointer *pointer)
{
	struct memleak_alias *alias;
	struct hlist_node *elem;
	unsigned long ptr = pointer->pointer;

	if (pointer->offset)
		radix_tree_delete(&pointer_tree, ptr + pointer->offset);

	if (pointer->alias_list)
		hlist_for_each_entry(alias, elem, pointer->alias_list, node)
			radix_tree_delete(&pointer_tree,
					  ptr + pointer->offset + alias->offset);
}

/* no need for atomic operations since memleak_lock is held anyway */
static inline void get_pointer(struct memleak_pointer *pointer)
{
	pointer->use_count++;
}

/* called with memleak_lock held for pointer_list modification and
 * memleak_cpu_mask set to avoid entering memleak_free (and deadlock)
 */
static void __put_pointer(struct memleak_pointer *pointer)
{
	struct hlist_node *elem, *tmp;
	struct memleak_scan_area *area;

	if (--pointer->use_count > 0)
		return;

	/* free the scanning areas */
	hlist_for_each_entry_safe(area, elem, tmp, &pointer->area_list, node) {
		hlist_del(elem);
		kfree(area);
	}

	list_del(&pointer->pointer_list);
	kmem_cache_free(pointer_cache, pointer);
}

static void put_pointer(struct memleak_pointer *pointer)
{
	unsigned long flags;
	unsigned int cpu_id;

	spin_lock_irqsave(&memleak_lock, flags);
	cpu_id = smp_processor_id();
	if (cpu_test_and_set(cpu_id, memleak_cpu_mask))
		BUG();

	__put_pointer(pointer);

	cpu_clear(cpu_id, memleak_cpu_mask);
	spin_unlock_irqrestore(&memleak_lock, flags);
}

/* Insert a pointer and its aliases into the pointer radix tree */
static inline void create_pointer(unsigned long ptr, size_t size, int ref_count)
{
	struct memleak_pointer *pointer;
	int err;
#ifdef CONFIG_FRAME_POINTER
	int i;
	void *frame;
#endif

	pointer = kmem_cache_alloc(pointer_cache, SLAB_ATOMIC);
	if (!pointer)
		panic("kmemleak: cannot allocate a memleak_pointer structure\n");

	last_pointer = pointer;

	INIT_LIST_HEAD(&pointer->pointer_list);
	INIT_LIST_HEAD(&pointer->gray_list);
	INIT_HLIST_HEAD(&pointer->area_list);
	pointer->use_count = 0;
	pointer->pointer = ptr;
	pointer->offset = 0;
	pointer->size = size;
	pointer->ref_count = ref_count;
	pointer->count = -1;
	pointer->alias_list = radix_tree_lookup(&alias_tree, size);

#ifdef CONFIG_FRAME_POINTER
	frame = __builtin_frame_address(0);
	for (i = 0; i < MAX_TRACE; i++) {
		void *stack = task_stack_page(current);

		if (frame < stack || frame > stack + THREAD_SIZE - BYTES_PER_WORD) {
			pointer->trace[i] = 0;
			continue;
		}

		pointer->trace[i] = arch_call_address(frame);
		frame = arch_prev_frame(frame);
		/* we don't need the return to do_exit() */
		if (kstack_end(frame))
			pointer->trace[i] = 0;
	}
#else
	pointer->trace[0] = (unsigned long)__builtin_return_address(0);
#endif

	err = radix_tree_insert(&pointer_tree, ptr, pointer);
	if (err) {
		dump_stack();
		panic("kmemleak: cannot insert into the pointer radix tree: %d\n", err);
	}

	create_pointer_aliases(pointer);

	list_add_tail(&pointer->pointer_list, &pointer_list);
	get_pointer(pointer);
}

/* Remove a pointer and its aliases from the pointer radix tree */
static inline void delete_pointer(unsigned long ptr)
{
	struct memleak_pointer *pointer;

	pointer = radix_tree_delete(&pointer_tree, ptr);
	if (!pointer) {
		dump_stack();
		printk(KERN_WARNING "kmemleak: freeing unknown pointer value 0x%08lx\n", ptr);
		return;
	}
	if (pointer->pointer != ptr) {
		dump_stack();
		dump_pointer_info(pointer);
		panic("kmemleak: freeing pointer by alias 0x%08lx\n", ptr);
	}

	if (last_pointer && ptr == last_pointer->pointer)
		last_pointer = NULL;

#ifdef CONFIG_DEBUG_MEMLEAK_ORPHAN_FREEING
	if (COLOR_WHITE(pointer)) {
		dump_stack();
		dump_pointer_info(pointer);
		printk(KERN_WARNING "kmemleak: freeing orphan pointer 0x%08lx\n", ptr);
	}
#endif

	delete_pointer_aliases(pointer);

	pointer->pointer = 0;
	__put_pointer(pointer);
}

/* Re-create the pointer aliases according to the new size/offset
 * information */
static inline void unpad_pointer(unsigned long ptr, unsigned long offset,
				 size_t size)
{
	struct memleak_pointer *pointer;

	pointer = get_cached_pointer(ptr);
	if (!pointer) {
		dump_stack();
		panic("kmemleak: resizing unknown pointer value 0x%08lx\n", ptr);
	}
	if (pointer->pointer != ptr) {
		dump_stack();
		dump_pointer_info(pointer);
		panic("kmemleak: resizing pointer by alias 0x%08lx\n", ptr);
	}

	if (offset == pointer->offset && size == pointer->size)
		return;

	delete_pointer_aliases(pointer);

	/* we don't update the pointer->size because the real block
	 * size should be scanned. We don't worry about random data in
	 * the unused area because of the object poisoning */
	pointer->offset = offset;
	pointer->alias_list = radix_tree_lookup(&alias_tree, size);

	create_pointer_aliases(pointer);
}

/* Make a pointer permanently gray (false positive) */
static inline void make_gray_pointer(unsigned long ptr)
{
	struct memleak_pointer *pointer;

	pointer = get_cached_pointer(ptr);
	if (!pointer) {
		dump_stack();
		panic("kmemleak: graying unknown pointer value 0x%08lx\n", ptr);
	}
	if (pointer->pointer != ptr) {
		dump_stack();
		dump_pointer_info(pointer);
		panic("kmemleak: graying pointer by alias 0x%08lx\n", ptr);
	}

	pointer->ref_count = 0;
}

/* Mark the pointer as black */
static inline void make_black_pointer(unsigned long ptr)
{
	struct memleak_pointer *pointer;

	pointer = get_cached_pointer(ptr);
	if (!pointer) {
		dump_stack();
		panic("kmemleak: blacking unknown pointer value 0x%08lx\n", ptr);
	}
	if (pointer->pointer != ptr) {
		dump_stack();
		dump_pointer_info(pointer);
		panic("kmemleak: blacking pointer by alias 0x%08lx\n", ptr);
	}

	pointer->ref_count = -1;
}

/* Add a scanning area to the pointer */
static inline void add_scan_area(unsigned long ptr, unsigned long offset, size_t length)
{
	struct memleak_pointer *pointer;
	struct memleak_scan_area *area;

	pointer = get_cached_pointer(ptr);
	if (!pointer) {
		dump_stack();
		panic("kmemleak: adding scan area to unknown pointer value 0x%08lx\n", ptr);
	}
	if (pointer->pointer != ptr) {
		dump_stack();
		dump_pointer_info(pointer);
		panic("kmemleak: adding scan area to pointer by alias 0x%08lx\n", ptr);
	}
	if (offset + length > pointer->size) {
		dump_stack();
		dump_pointer_info(pointer);
		panic("kmemleak: scan area larger than block 0x%08lx\n", ptr);
	}

	area = kmalloc(sizeof(*area), GFP_ATOMIC);
	if (!area)
		panic("kmemleak: cannot allocate a scan area\n");

	INIT_HLIST_NODE(&area->node);
	area->offset = offset;
	area->length = length;

	hlist_add_head(&area->node, &pointer->area_list);
}

/* Allocation function hook */
void memleak_alloc(const void *ptr, size_t size, int ref_count)
{
	unsigned long flags;
	unsigned int cpu_id;

	if (!ptr)
		return;

	local_irq_save(flags);
	cpu_id = get_cpu();

	/* avoid recursive calls. After disabling the interrupts, the
	 * only calls to this function on the same CPU should be from
	 * kmemleak itself and we ignore them. Calls from other CPU's
	 * would wait on the spin_lock.
	 */
	if (cpu_test_and_set(cpu_id, memleak_cpu_mask))
		goto out;

	pr_debug("%s(0x%p, %d, %d)\n", __FUNCTION__, ptr, size, ref_count);

	if (!atomic_read(&memleak_initialized)) {
		/* no need for SMP locking since this block is
		 * executed before the other CPUs are started */
		struct memleak_preinit_pointer *pointer;

		BUG_ON(cpu_id != 0);

		if (preinit_pos >= PREINIT_POINTERS)
			panic("kmemleak: preinit pointers buffer overflow\n");
		pointer = &preinit_pointers[preinit_pos++];

		pointer->type = MEMLEAK_ALLOC;
		pointer->pointer = ptr;
		pointer->size = size;
		pointer->ref_count = ref_count;

		goto unmask;
	}

	spin_lock(&memleak_lock);
	create_pointer((unsigned long)ptr, size, ref_count);
	spin_unlock(&memleak_lock);

 unmask:
	cpu_clear(cpu_id, memleak_cpu_mask);
 out:
	put_cpu_no_resched();
	local_irq_restore(flags);
}
EXPORT_SYMBOL_GPL(memleak_alloc);

/* Freeing function hook */
void memleak_free(const void *ptr)
{
	unsigned long flags;
	unsigned int cpu_id;

	if (!ptr)
		return;

	local_irq_save(flags);
	cpu_id = get_cpu();

	/* avoid recursive calls. See memleak_alloc() for an explanation */
	if (cpu_test_and_set(cpu_id, memleak_cpu_mask))
		goto out;

	pr_debug("%s(0x%p)\n", __FUNCTION__, ptr);

	if (!atomic_read(&memleak_initialized)) {
		struct memleak_preinit_pointer *pointer;

		BUG_ON(cpu_id != 0);

		if (preinit_pos >= PREINIT_POINTERS)
			panic("kmemleak: preinit pointers buffer overflow\n");
		pointer = &preinit_pointers[preinit_pos++];

		pointer->type = MEMLEAK_FREE;
		pointer->pointer = ptr;

		goto unmask;
	}

	spin_lock(&memleak_lock);
	delete_pointer((unsigned long)ptr);
	spin_unlock(&memleak_lock);

 unmask:
	cpu_clear(cpu_id, memleak_cpu_mask);
 out:
	put_cpu_no_resched();
	local_irq_restore(flags);
}
EXPORT_SYMBOL_GPL(memleak_free);

/* Change the size and location information of an allocated memory
 * block (this is needed for allocations padding the object) */
void memleak_padding(const void *ptr, unsigned long offset, size_t size)
{
	unsigned long flags;
	unsigned int cpu_id;

	if (!ptr)
		return;

	local_irq_save(flags);
	cpu_id = get_cpu();

	/* avoid recursive calls. See memleak_alloc() for an explanation */
	if (cpu_test_and_set(cpu_id, memleak_cpu_mask))
		goto out;

	pr_debug("%s(0x%p, %d)\n", __FUNCTION__, ptr, size);

	if (!atomic_read(&memleak_initialized)) {
		struct memleak_preinit_pointer *pointer;

		BUG_ON(cpu_id != 0);

		if (preinit_pos >= PREINIT_POINTERS)
			panic("kmemleak: preinit pointers buffer overflow\n");
		pointer = &preinit_pointers[preinit_pos++];

		pointer->type = MEMLEAK_PADDING;
		pointer->pointer = ptr;
		pointer->offset = offset;
		pointer->size = size;

		goto unmask;
	}

	spin_lock(&memleak_lock);
	unpad_pointer((unsigned long)ptr, offset, size);
	spin_unlock(&memleak_lock);

 unmask:
	cpu_clear(cpu_id, memleak_cpu_mask);
 out:
	put_cpu_no_resched();
	local_irq_restore(flags);
}
EXPORT_SYMBOL(memleak_padding);

/* Mark a pointer as a false positive */
void memleak_not_leak(const void *ptr)
{
	unsigned long flags;
	unsigned int cpu_id;

	if (!ptr)
		return;

	local_irq_save(flags);
	cpu_id = get_cpu();

	/* avoid recursive calls. See memleak_alloc() for an explanation */
	if (cpu_test_and_set(cpu_id, memleak_cpu_mask))
		goto out;

	pr_debug("%s(0x%p)\n", __FUNCTION__, ptr);

	if (!atomic_read(&memleak_initialized)) {
		struct memleak_preinit_pointer *pointer;

		BUG_ON(cpu_id != 0);

		if (preinit_pos >= PREINIT_POINTERS)
			panic("kmemleak: preinit pointers buffer overflow\n");
		pointer = &preinit_pointers[preinit_pos++];

		pointer->type = MEMLEAK_NOT_LEAK;
		pointer->pointer = ptr;

		goto unmask;
	}

	spin_lock(&memleak_lock);
	make_gray_pointer((unsigned long)ptr);
	spin_unlock(&memleak_lock);

 unmask:
	cpu_clear(cpu_id, memleak_cpu_mask);
 out:
	put_cpu_no_resched();
	local_irq_restore(flags);
}
EXPORT_SYMBOL(memleak_not_leak);

/* Ignore this memory block */
void memleak_ignore(const void *ptr)
{
	unsigned long flags;
	unsigned int cpu_id;

	if (!ptr)
		return;

	local_irq_save(flags);
	cpu_id = get_cpu();

	/* avoid recursive calls. See memleak_alloc() for an explanation */
	if (cpu_test_and_set(cpu_id, memleak_cpu_mask))
		goto out;

	pr_debug("%s(0x%p)\n", __FUNCTION__, ptr);

	if (!atomic_read(&memleak_initialized)) {
		struct memleak_preinit_pointer *pointer;

		BUG_ON(cpu_id != 0);

		if (preinit_pos >= PREINIT_POINTERS)
			panic("kmemleak: preinit pointers buffer overflow\n");
		pointer = &preinit_pointers[preinit_pos++];

		pointer->type = MEMLEAK_IGNORE;
		pointer->pointer = ptr;

		goto unmask;
	}

	spin_lock(&memleak_lock);
	make_black_pointer((unsigned long)ptr);
	spin_unlock(&memleak_lock);

 unmask:
	cpu_clear(cpu_id, memleak_cpu_mask);
 out:
	put_cpu_no_resched();
	local_irq_restore(flags);
}
EXPORT_SYMBOL(memleak_ignore);

/* Add a scanning area to a pointer */
void memleak_scan_area(const void *ptr, unsigned long offset, size_t length)
{
	unsigned long flags;
	unsigned int cpu_id;

	if (!ptr)
		return;

	local_irq_save(flags);
	cpu_id = get_cpu();

	/* avoid recursive calls. See memleak_alloc() for an explanation */
	if (cpu_test_and_set(cpu_id, memleak_cpu_mask))
		goto out;

	pr_debug("%s(0x%p)\n", __FUNCTION__, ptr);

	if (!atomic_read(&memleak_initialized)) {
		struct memleak_preinit_pointer *pointer;

		BUG_ON(cpu_id != 0);

		if (preinit_pos >= PREINIT_POINTERS)
			panic("kmemleak: preinit pointers buffer overflow\n");
		pointer = &preinit_pointers[preinit_pos++];

		pointer->type = MEMLEAK_SCAN_AREA;
		pointer->pointer = ptr;
		pointer->offset = offset;
		pointer->size = length;

		goto unmask;
	}

	spin_lock(&memleak_lock);
	add_scan_area((unsigned long)ptr, offset, length);
	spin_unlock(&memleak_lock);

 unmask:
	cpu_clear(cpu_id, memleak_cpu_mask);
 out:
	put_cpu_no_resched();
	local_irq_restore(flags);
}
EXPORT_SYMBOL(memleak_scan_area);

/* Scan a block of memory (exclusive range) for pointers and move
 * those found to the gray list. This function is called with
 * memleak_lock held
 */
static void __scan_block(void *_start, void *_end)
{
	unsigned long *ptr;
	unsigned long *start = (unsigned long *)ALIGN((unsigned long)_start,
						      BYTES_PER_WORD);
	unsigned long *end = _end;

	for (ptr = start; ptr < end; ptr++) {
		struct memleak_pointer *pointer =
			radix_tree_lookup(&pointer_tree,
					  (*ptr) & ~(BYTES_PER_WORD - 1));
		if (!pointer)
			continue;
		if (!COLOR_WHITE(pointer))
			continue;

		pointer->count++;
		/* this can happen during the grey_list traversal */
		if (COLOR_GRAY(pointer)) {
			get_pointer(pointer);
			list_add_tail_rcu(&pointer->gray_list, &gray_list);
		}
	}
}

static void scan_block(void *start, void *end)
{
	unsigned long flags;
	void *s, *e;

	s = start;
	while (s < end) {
		e = s + SCAN_BLOCK_SIZE;

		spin_lock_irqsave(&memleak_lock, flags);
		__scan_block(s, e < end ? e : end);
		spin_unlock_irqrestore(&memleak_lock, flags);

		s = e;
	}
}

/* Scan a memory block represented by a memleak_pointer */
static inline void scan_pointer(struct memleak_pointer *pointer)
{
	struct memleak_scan_area *area;
	struct hlist_node *elem;
	unsigned long flags;

	spin_lock_irqsave(&memleak_lock, flags);

	/* freed pointer */
	if (!pointer->pointer)
		goto out;

	if (hlist_empty(&pointer->area_list))
		__scan_block((void *)pointer->pointer,
			     (void *)(pointer->pointer + pointer->size));
	else
		hlist_for_each_entry(area, elem, &pointer->area_list, node) {
			unsigned long ptr = pointer->pointer + area->offset;

			__scan_block((void *)ptr, (void *)(ptr + area->length));
		}

 out:
	spin_unlock_irqrestore(&memleak_lock, flags);
}

/* Scan the memory and print the orphan pointers */
static void memleak_scan(void)
{
	unsigned long flags;
	struct memleak_pointer *pointer, *tmp;
#ifdef CONFIG_DEBUG_MEMLEAK_TASK_STACKS
	struct task_struct *task;
#endif
	int i;

	/* initialize pointers (make them white) and build the initial
	 * gray list */
	spin_lock_irqsave(&memleak_lock, flags);
	list_for_each_entry(pointer, &pointer_list, pointer_list) {
		pointer->count = 0;
		if (COLOR_GRAY(pointer)) {
			get_pointer(pointer);
			list_add_tail(&pointer->gray_list, &gray_list);
		}
	}
	spin_unlock_irqrestore(&memleak_lock, flags);

	/* data/bss scanning */
	scan_block(_sdata, _edata);
	scan_block(__bss_start, __bss_stop);

#ifdef CONFIG_SMP
	/* per-cpu scanning */
	for (i = 0; i < NR_CPUS; i++)
		scan_block(__per_cpu_offset[i] + __per_cpu_start,
			   __per_cpu_offset[i] + __per_cpu_end);
#endif

	/* mem_map scanning */
	for_each_online_node(i) {
		struct page *page, *end;

		page = NODE_MEM_MAP(i);
		end  = page + NODE_DATA(i)->node_spanned_pages;

		scan_block(page, end);
	}

#ifdef CONFIG_DEBUG_MEMLEAK_TASK_STACKS
	read_lock(&tasklist_lock);
	for_each_process(task)
		scan_block(task_stack_page(task),
			   task_stack_page(task) + THREAD_SIZE);
	read_unlock(&tasklist_lock);
#endif

	/* gray_list scanning. RCU is needed because new elements can
	 * be added to the list during scanning */
	rcu_read_lock();
	list_for_each_entry_rcu(pointer, &gray_list, gray_list)
		scan_pointer(pointer);
	rcu_read_unlock();

	/* empty the gray list. It needs the "safe" version because
	 * put_pointer() can free the structure */
	list_for_each_entry_safe(pointer, tmp, &gray_list, gray_list) {
		list_del(&pointer->gray_list);
		put_pointer(pointer);
	}
}

#ifdef CONFIG_DEBUG_FS
static void *memleak_seq_start(struct seq_file *seq, loff_t *pos)
{
	struct memleak_pointer *pointer;
	loff_t n = *pos;
	unsigned long flags;

	mutex_lock(&memleak_mutex);

	if (!n)
		memleak_scan();

	spin_lock_irqsave(&memleak_lock, flags);

	list_for_each_entry(pointer, &pointer_list, pointer_list)
		if (!n--) {
			get_pointer(pointer);
			goto out;
		}
	pointer = NULL;

 out:
	spin_unlock_irqrestore(&memleak_lock, flags);
	return pointer;
}

static void *memleak_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
	struct list_head *n;
	struct memleak_pointer *next = NULL;
	unsigned long flags;

	++(*pos);

	spin_lock_irqsave(&memleak_lock, flags);

	n = ((struct memleak_pointer *)v)->pointer_list.next;
	if (n != &pointer_list) {
		next = list_entry(n, struct memleak_pointer, pointer_list);
		get_pointer(next);
	}

	spin_unlock_irqrestore(&memleak_lock, flags);

	put_pointer(v);
	return next;
}

static void memleak_seq_stop(struct seq_file *seq, void *v)
{
	if (v)
		put_pointer(v);
	mutex_unlock(&memleak_mutex);
}

static int memleak_seq_show(struct seq_file *seq, void *v)
{
	const struct memleak_pointer *pointer = v;
#ifdef CONFIG_KALLSYMS
	char namebuf[KSYM_NAME_LEN + 1] = "";
	char *modname;
	unsigned long symsize;
	unsigned long offset = 0;
#endif
	int i;

	if (!COLOR_WHITE(pointer))
		return 0;
	/* freed in the meantime (false positive) */
	if (!pointer->pointer)
		return 0;

	seq_printf(seq, "orphan pointer 0x%08lx (size %d):\n",
		   pointer->pointer, pointer->size);

	for (i = 0; i < MAX_TRACE; i++) {
		unsigned long trace = pointer->trace[i];
		if (!trace)
			break;

#ifdef CONFIG_KALLSYMS
		kallsyms_lookup(trace, &symsize, &offset, &modname, namebuf);
		seq_printf(seq, "  %lx: <%s>\n", trace, namebuf);
#else
		seq_printf(seq, "  %lx\n", trace);
#endif
	}

	return 0;
}

static struct seq_operations memleak_seq_ops = {
	.start = memleak_seq_start,
	.next  = memleak_seq_next,
	.stop  = memleak_seq_stop,
	.show  = memleak_seq_show,
};

static int memleak_seq_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &memleak_seq_ops);
}

static struct file_operations memleak_fops = {
	.owner	 = THIS_MODULE,
	.open    = memleak_seq_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release,
};
#endif					/* CONFIG_DEBUG_FS */

/* KMemLeak initialization. Set up the radix tree for the pointer aliases */
void __init memleak_init(void)
{
	int i = 0;
	unsigned long flags;

	pointer_cache = kmem_cache_create("pointer_cache", sizeof(struct memleak_pointer),
					  0, SLAB_PANIC, NULL, NULL);
	if (!pointer_cache)
		panic("kmemleak: cannot create the pointer cache\n");

	memleak_insert_aliases(__memleak_offsets_start, __memleak_offsets_end);

	/* no need to hold the spinlock as SMP is not initialized
	 * yet. Holding it here would lead to a deadlock */
	local_irq_save(flags);

	atomic_set(&memleak_initialized, 1);

	/* execute the buffered memleak actions */
	pr_debug("kmemleak: %d preinit actions\n", preinit_pos);
	for (i = 0; i < preinit_pos; i++) {
		struct memleak_preinit_pointer *pointer = &preinit_pointers[i];

		switch (pointer->type) {
		case MEMLEAK_ALLOC:
			memleak_alloc(pointer->pointer, pointer->size,
				      pointer->ref_count);
			break;
		case MEMLEAK_FREE:
			memleak_free(pointer->pointer);
			break;
		case MEMLEAK_PADDING:
			memleak_padding(pointer->pointer, pointer->offset,
					pointer->size);
			break;
		case MEMLEAK_NOT_LEAK:
			memleak_not_leak(pointer->pointer);
			break;
		case MEMLEAK_IGNORE:
			memleak_ignore(pointer->pointer);
			break;
		case MEMLEAK_SCAN_AREA:
			memleak_scan_area(pointer->pointer,
					  pointer->offset, pointer->size);
			break;
		default:
			BUG();
		}
	}

	local_irq_restore(flags);

	printk(KERN_INFO "Kernel memory leak detector initialized\n");
}

/* Late initialization function */
int __init memleak_late_init(void)
{
#ifdef CONFIG_DEBUG_FS
	struct dentry *dentry;

	dentry = debugfs_create_file("memleak", S_IRUGO, NULL, NULL,
				     &memleak_fops);
	if (!dentry)
		return -ENOMEM;
#endif
	pr_debug("kmemleak: late initialization completed\n");

	return 0;
}
late_initcall(memleak_late_init);
