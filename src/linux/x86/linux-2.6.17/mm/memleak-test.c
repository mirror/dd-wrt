/*
 * mm/memleak-test.c
 *
 * Copyright (C) 2006 ARM Limited
 * Written by Catalin Marinas <catalin.marinas@arm.com>
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

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/list.h>

#include <linux/memleak.h>

struct test_node {
	long header[25];
	struct list_head list;
	long footer[25];
};

static LIST_HEAD(test_list);

/* Some very simple testing. This function needs to be extended for
 * proper testing */
static int __init memleak_test_init(void)
{
	struct test_node *elem;
	int i;

	printk(KERN_INFO "KMemLeak testing\n");

	/* make some orphan pointers */
	kmalloc(32, GFP_KERNEL);
	kmalloc(32, GFP_KERNEL);
#ifndef CONFIG_MODULES
	kmem_cache_alloc(files_cachep, GFP_KERNEL);
	kmem_cache_alloc(files_cachep, GFP_KERNEL);
#endif
	vmalloc(64);
	vmalloc(64);

	/* add elements to a list. They should only appear as orphan
	 * after the module is removed */
	for (i = 0; i < 10; i++) {
		elem = kmalloc(sizeof(*elem), GFP_KERNEL);
		if (!elem)
			return -ENOMEM;
		memset(elem, 0, sizeof(*elem));
		INIT_LIST_HEAD(&elem->list);

		list_add_tail(&elem->list, &test_list);
	}

	return 0;
}
module_init(memleak_test_init);

static void __exit memleak_test_exit(void)
{
	struct test_node *elem, *tmp;

	/* remove the list elements without actually freeing the memory */
	list_for_each_entry_safe(elem, tmp, &test_list, list)
		list_del(&elem->list);
}
module_exit(memleak_test_exit);

MODULE_LICENSE("GPL");
