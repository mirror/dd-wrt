/**********************************************************************
 * Author: Cavium, Inc.
 *
 * Contact: support@cavium.com
 * This file is part of the OCTEON SDK
 *
 * Copyright (c) 2003-2012 Cavium, Inc.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, Version 2, as
 * published by the Free Software Foundation.
 *
 * This file is distributed in the hope that it will be useful, but
 * AS-IS and WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, TITLE, or
 * NONINFRINGEMENT.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this file; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 * or visit http://www.gnu.org/licenses/.
 *
 * This file may also be available under a different license from Cavium.
 * Contact Cavium, Inc. for more information
 **********************************************************************/
#include <linux/netdevice.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>

#include <asm/octeon/octeon.h>
#include <asm/octeon/cvmx-fpa1.h>

#include "ethernet-defines.h"
#include "octeon-ethernet.h"

struct fpa_pool {
	int pool;
	int users;
	int size;
	char kmem_name[20];
	struct kmem_cache *kmem;
	int (*fill)(struct fpa_pool *p, int num);
	int (*empty)(struct fpa_pool *p, int num);
};

static DEFINE_MUTEX(cvm_oct_pools_lock);
static struct fpa_pool cvm_oct_pools[CVMX_FPA1_NUM_POOLS] = {
	[0 ... CVMX_FPA1_NUM_POOLS - 1] = {-1,},
};

/**
 * cvm_oct_fill_hw_skbuff - fill the supplied hardware pool with skbuffs
 * @pool:     Pool to allocate an skbuff for
 * @size:     Size of the buffer needed for the pool
 * @elements: Number of buffers to allocate
 *
 * Returns the actual number of buffers allocated.
 */
static int cvm_oct_fill_hw_skbuff(struct fpa_pool *pool, int elements)
{
	int freed = elements;
	int size = pool->size;
	int pool_num = pool->pool;
	while (freed) {
		int extra_reserve;
		u8 *desired_data;
		struct sk_buff *skb = alloc_skb(size + CVM_OCT_SKB_TO_FPA_PADDING,
						GFP_ATOMIC);
		if (skb == NULL) {
			pr_err("Failed to allocate skb for hardware pool %d\n",
			       pool_num);
			break;
		}
		desired_data = cvm_oct_get_fpa_head(skb);
		extra_reserve = desired_data - skb->data;
		skb_reserve(skb, extra_reserve);
		*(struct sk_buff **)(skb->data - sizeof(void *)) = skb;
		cvmx_fpa1_free(skb->data, pool_num, DONT_WRITEBACK(size / 128));
		freed--;
	}
	return elements - freed;
}

/**
 * cvm_oct_free_hw_skbuff- free hardware pool skbuffs
 * @pool:     Pool to allocate an skbuff for
 * @size:     Size of the buffer needed for the pool
 * @elements: Number of buffers to allocate
 */
static int cvm_oct_free_hw_skbuff(struct fpa_pool *pool, int elements)
{
	struct sk_buff *skb;
	char *memory;
	int pool_num = pool->pool;

	while (elements) {
		memory = cvmx_fpa1_alloc(pool_num);
		if (!memory)
			break;
		skb = *cvm_oct_packet_to_skb(memory);
		elements--;
		dev_kfree_skb(skb);
	}

	if (elements > 0)
		pr_err("Freeing of pool %u is missing %d skbuffs\n",
		       pool_num, elements);

	return elements;
}

/**
 * cvm_oct_fill_hw_memory - fill a hardware pool with memory.
 * @pool:     Pool to populate
 * @size:     Size of each buffer in the pool
 * @elements: Number of buffers to allocate
 *
 * Returns the actual number of buffers allocated.
 */
static int cvm_oct_fill_hw_kmem(struct fpa_pool *pool, int elements)
{
	char *memory;
	int freed = elements;

	while (freed) {
		memory = kmem_cache_alloc(pool->kmem, GFP_KERNEL);
		if (unlikely(memory == NULL)) {
			pr_err("Unable to allocate %u bytes for FPA pool %d\n",
			       elements * pool->size, pool->pool);
			break;
		}
		cvmx_fpa1_free(memory, pool->pool, 0);
		freed--;
	}
	return elements - freed;
}

/**
 * cvm_oct_free_hw_memory - Free memory allocated by cvm_oct_fill_hw_memory
 * @pool:     FPA pool to free
 * @size:     Size of each buffer in the pool
 * @elements: Number of buffers that should be in the pool
 */
static int cvm_oct_free_hw_kmem(struct fpa_pool *pool, int elements)
{
	char *fpa;
	while (elements) {
		fpa = cvmx_fpa1_alloc(pool->pool);
		if (!fpa)
			break;
		elements--;
		kmem_cache_free(pool->kmem, fpa);
	}

	if (elements > 0)
		pr_err("Warning: Freeing of pool %u is missing %d buffers\n",
		       pool->pool, elements);
	return elements;
}

int cvm_oct_mem_fill_fpa(int pool, int elements)
{
	struct fpa_pool *p;

	if (pool < 0 || pool >= ARRAY_SIZE(cvm_oct_pools))
		return -EINVAL;

	p = cvm_oct_pools + pool;

	return p->fill(p, elements);
}
EXPORT_SYMBOL(cvm_oct_mem_fill_fpa);

int cvm_oct_mem_empty_fpa(int pool, int elements)
{
	struct fpa_pool *p;

	if (pool < 0 || pool >= ARRAY_SIZE(cvm_oct_pools))
		return -EINVAL;

	p = cvm_oct_pools + pool;
	if (p->empty)
		return p->empty(p, elements);

	return 0;
}
EXPORT_SYMBOL(cvm_oct_mem_empty_fpa);

void cvm_oct_mem_cleanup(void)
{
	int i;

	mutex_lock(&cvm_oct_pools_lock);

	for (i = 0; i < ARRAY_SIZE(cvm_oct_pools); i++)
		if (cvm_oct_pools[i].kmem)
			kmem_cache_shrink(cvm_oct_pools[i].kmem);
	mutex_unlock(&cvm_oct_pools_lock);
}
EXPORT_SYMBOL(cvm_oct_mem_cleanup);

/**
 * cvm_oct_alloc_fpa_pool() - Allocate an FPA pool of the given size
 * @pool:  Requested pool number (-1 for don't care).
 * @size:  The size of the pool elements.
 *
 * Returns the pool number or a negative number on error.
 */
int cvm_oct_alloc_fpa_pool(int pool, int size)
{
	int i;
	int ret = 0;

	if (pool >= (int)ARRAY_SIZE(cvm_oct_pools) || size < 128)
		return -EINVAL;

	BUG_ON(octeon_has_feature(OCTEON_FEATURE_FPA3));

	mutex_lock(&cvm_oct_pools_lock);

	if (pool >= 0) {
		if (cvm_oct_pools[pool].pool != -1) {
			if (cvm_oct_pools[pool].size == size) {
				/* Already allocated */
				cvm_oct_pools[pool].users++;
				ret = pool;
				goto out;
			} else {
				/* conflict */
				ret = -EINVAL;
				goto out;
			}
		}
		/* reserve/alloc fpa pool */
		pool = cvmx_fpa1_reserve_pool(pool);
		if (pool < 0) {
			ret = -EINVAL;
			goto out;
		}
	} else {
		/* Find an established pool */
		for (i = 0; i < ARRAY_SIZE(cvm_oct_pools); i++)
			if (cvm_oct_pools[i].pool >= 0 &&
			    cvm_oct_pools[i].size == size) {
				cvm_oct_pools[i].users++;
				ret = i;
				goto out;
			}

		/* Alloc fpa pool */
		pool = cvmx_fpa1_reserve_pool(pool);
		if (pool < 0) {
			/* No empties. */
			ret = -EINVAL;
			goto out;
		}
	}

	/* Setup the pool */
	cvm_oct_pools[pool].pool = pool;
	cvm_oct_pools[pool].users++;
	cvm_oct_pools[pool].size = size;
	if (USE_SKBUFFS_IN_HW && pool == 0) {
		/* Special packet pool */
		cvm_oct_pools[pool].fill = cvm_oct_fill_hw_skbuff;
		cvm_oct_pools[pool].empty = cvm_oct_free_hw_skbuff;
	} else {
		snprintf(cvm_oct_pools[pool].kmem_name,
			 sizeof(cvm_oct_pools[pool].kmem_name),
			 "oct-fpa-%d", size);
		cvm_oct_pools[pool].fill = cvm_oct_fill_hw_kmem;
		cvm_oct_pools[pool].empty = cvm_oct_free_hw_kmem;
		cvm_oct_pools[pool].kmem =
			kmem_cache_create(cvm_oct_pools[pool].kmem_name,
					  size, 128, 0, NULL);
		if (!cvm_oct_pools[pool].kmem) {
			ret = -ENOMEM;
			cvm_oct_pools[pool].pool = -1;
			cvmx_fpa1_release_pool(pool);
			goto out;
		}
	}
	ret = pool;
out:
	mutex_unlock(&cvm_oct_pools_lock);
	return ret;
}
EXPORT_SYMBOL(cvm_oct_alloc_fpa_pool);

/**
 * cvm_oct_release_fpa_pool() - Releases an FPA pool
 * @pool:  Pool number.
 *
 * This undoes the action of cvm_oct_alloc_fpa_pool().
 *
 * Returns zero on success.
 */
int cvm_oct_release_fpa_pool(int pool)
{
	int ret = -EINVAL;

	if (pool < 0 || pool >= (int)ARRAY_SIZE(cvm_oct_pools))
		return ret;

	mutex_lock(&cvm_oct_pools_lock);

	if (cvm_oct_pools[pool].users <= 0) {
		pr_err("Error: Unbalanced FPA pool allocation\n");
		goto out;
	}
	cvm_oct_pools[pool].users--;

	if (cvm_oct_pools[pool].users == 0)
		cvmx_fpa1_release_pool(pool);

	ret = 0;
out:
	mutex_unlock(&cvm_oct_pools_lock);
	return ret;
}
EXPORT_SYMBOL(cvm_oct_release_fpa_pool);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Cavium, Inc. Ethernet/FPA memory allocator.");
