/*******************************************************************************
 *
 *  arch/arm/mm/cache-l2cc.c - L2 cache controller support
 *
 *  Copyright (c) 2008 Cavium Networks 
 * 
 *  This file is free software; you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License, Version 2, as 
 *  published by the Free Software Foundation. 
 *
 *  This file is distributed in the hope that it will be useful, 
 *  but AS-IS and WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, TITLE, or 
 *  NONINFRINGEMENT.  See the GNU General Public License for more details. 
 *
 *  You should have received a copy of the GNU General Public License 
 *  along with this file; if not, write to the Free Software 
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA or 
 *  visit http://www.gnu.org/licenses/. 
 *
 *  This file may also be available under a different license from Cavium. 
 *  Contact Cavium Networks for more information
 *
 ******************************************************************************/

#include <linux/init.h>
#include <linux/spinlock.h>

#include <asm/cacheflush.h>
#include <asm/io.h>
#include <asm/hardware/cache-l2cc.h>

#define CACHE_LINE_SIZE		32

static void __iomem *cns3xxx_l2_base;
static DEFINE_SPINLOCK(cns3xxx_l2_lock);

static inline void cache_wait(void __iomem *reg, unsigned long mask)
{
#ifndef CONFIG_L2CC_NO_WAIT
	/* wait for the operation to complete */
	while (readl(reg) & mask);
#endif
}

static inline void sync_writel(unsigned long val, unsigned long reg,
			       unsigned long complete_mask)
{
	unsigned long flags;

	spin_lock_irqsave(&cns3xxx_l2_lock, flags);
	writel(val, cns3xxx_l2_base + reg);
	/* wait for the operation to complete */
	while (readl(cns3xxx_l2_base + reg) & complete_mask)
		;
	spin_unlock_irqrestore(&cns3xxx_l2_lock, flags);
}

static inline void cache_sync(void)
{
	sync_writel(0, L2CC_CACHE_SYNC, 1);
}

static inline void cns3xxx_l2_inv_all(void)
{
	/* invalidate all ways */
	sync_writel(0xffff, L2CC_INV_WAY, 0xffff);
	cache_sync();
}

static void cns3xxx_l2_inv_range(unsigned long start, unsigned long end)
{
	unsigned long addr;

	if (start & (CACHE_LINE_SIZE - 1)) {
		start &= ~(CACHE_LINE_SIZE - 1);
		writel(start, cns3xxx_l2_base + L2CC_CLEAN_INV_LINE_PA);
		start += CACHE_LINE_SIZE;
	}

	if (end & (CACHE_LINE_SIZE - 1)) {
		end &= ~(CACHE_LINE_SIZE - 1);
		writel(end, cns3xxx_l2_base + L2CC_CLEAN_INV_LINE_PA);
	}

	for (addr = start; addr < end; addr += CACHE_LINE_SIZE)
		writel(addr, cns3xxx_l2_base + L2CC_INV_LINE_PA);

	cache_sync();
}

static void cns3xxx_l2_clean_range(unsigned long start, unsigned long end)
{
	unsigned long addr;

	start &= ~(CACHE_LINE_SIZE - 1);
	for (addr = start; addr < end; addr += CACHE_LINE_SIZE)
		writel(addr, cns3xxx_l2_base + L2CC_CLEAN_LINE_PA);

	cache_wait(cns3xxx_l2_base + L2CC_CLEAN_LINE_PA, 1);
	cache_sync();
}

static void cns3xxx_l2_flush_range(unsigned long start, unsigned long end)
{
	unsigned long addr;

	start &= ~(CACHE_LINE_SIZE - 1);
	for (addr = start; addr < end; addr += CACHE_LINE_SIZE)
		writel(addr, cns3xxx_l2_base + L2CC_CLEAN_INV_LINE_PA);

	cache_wait(cns3xxx_l2_base + L2CC_CLEAN_INV_LINE_PA, 1);
	cache_sync();
}

static void cns3xxx_l2_cache_sync(void)
{
	unsigned long flags;

	spin_lock_irqsave(&cns3xxx_l2_lock, flags);
	cache_sync();
	spin_unlock_irqrestore(&cns3xxx_l2_lock, flags);
}

static void cns3xxx_l2_disable(void)
{
	unsigned long flags;

	spin_lock_irqsave(&cns3xxx_l2_lock, flags);
	writel(0, cns3xxx_l2_base + L2CC_CTRL);
	spin_unlock_irqrestore(&cns3xxx_l2_lock, flags);
}

void __init l2cc_init(void __iomem *base)
{
	__u32 aux, prefetch, tag, data;

	printk(KERN_INFO "Initializing CNS3XXX L2 cache controller... ");

	cns3xxx_l2_base = base;

	/* disable L2CC */
	writel(0, cns3xxx_l2_base + L2CC_CTRL);

	/*
	 * Auxiliary control register 
	 *
	 * bit[22]	- shared attribute internally ignored
	 * bit[21]	- parity enabled
	 * bit[20]	- 
	 * bit[19:17]	- 32kB way size 
	 * bit[16]	- way associative
	 * bit[12]	- exclusive cache disabled
	 *
	 */
	aux = readl(cns3xxx_l2_base + L2CC_AUX_CTRL);
	aux &= 0xfe000fff;
#ifdef CONFIG_CACHE_L2_I_PREFETCH
	aux |= 0x20000000;	/* bit[29]: Instruction prefetching enable, bit[29]: Data prefetching enable */
#endif
#ifdef CONFIG_CACHE_L2_D_PREFETCH
	aux |= 0x10000000;	/* bit[28]: Instruction prefetching enable, bit[28]: Data prefetching enable */
#endif
	aux |= 0x00540000;	/* ...010..., 32KB, 8-way, Parity Disable*/
	writel(aux, cns3xxx_l2_base + L2CC_AUX_CTRL);

	prefetch = readl(cns3xxx_l2_base + 0xF60);
	prefetch |= 0x00000008; /* prefetch offset, bit[4..0] */
#ifdef CONFIG_CACHE_L2_I_PREFETCH
	prefetch |= 0x20000000;
#endif
#ifdef CONFIG_CACHE_L2_D_PREFETCH
	prefetch |= 0x10000000;
#endif
	writel(prefetch, cns3xxx_l2_base + 0xF60);

	/* Tag RAM Control register
	 * 
	 * bit[10:8]	- 1 cycle of write accesses latency
	 * bit[6:4]	- 1 cycle of read accesses latency
	 * bit[3:0]	- 1 cycle of setup latency
	 *
	 * 1 cycle of latency for setup, read and write accesses
	 */
	tag = readl(cns3xxx_l2_base + L2CC_TAG_RAM_LATENCY_CTRL);
	tag &= 0xfffff888;
	tag |= 0x00000000;
	writel(tag, cns3xxx_l2_base + L2CC_TAG_RAM_LATENCY_CTRL);

	/* Data RAM Control register
	 *
	 * bit[10:8]	- 1 cycles of write accesses latency
	 * bit[6:4]	- 1 cycles of read accesses latency
	 * bit[3:0]	- 1 cycle of setup latency
	 *
	 * 1 cycle of setup latency, 2 cycles of read and write accesses latency
	 */
	data = readl(cns3xxx_l2_base + L2CC_DATA_RAM_LATENCY_CTRL);
	data &= 0xfffff888;
	data |= 0x00000000;
	writel(data, cns3xxx_l2_base + L2CC_DATA_RAM_LATENCY_CTRL);

	cns3xxx_l2_inv_all();

	/* lockdown required ways for different effective size of the L2 cache */
#ifdef CONFIG_CACHE_L2CC_32KB
        /* 32KB, lock way7..1 */
        writel(0xfe, cns3xxx_l2_base + L2CC_LOCKDOWN_0_WAY_D);
        writel(0xfe, cns3xxx_l2_base + L2CC_LOCKDOWN_0_WAY_I);
        printk(KERN_INFO "CNS3XXX L2 cache lock down : way7..1\n");
#elif defined(CONFIG_CACHE_L2CC_64KB)
        /* 64KB, lock way7..2 */
        writel(0xfc, cns3xxx_l2_base + L2CC_LOCKDOWN_0_WAY_D);
        writel(0xfc, cns3xxx_l2_base + L2CC_LOCKDOWN_0_WAY_I);
        printk(KERN_INFO "CNS3XXX L2 cache lock down : way7..2\n");
#elif defined(CONFIG_CACHE_L2CC_96KB)
        /* 96KB, lock way7..3 */
        writel(0xf8, cns3xxx_l2_base + L2CC_LOCKDOWN_0_WAY_D);
        writel(0xf8, cns3xxx_l2_base + L2CC_LOCKDOWN_0_WAY_I);
        printk(KERN_INFO "CNS3XXX L2 cache lock down : way7..3\n");
#elif defined(CONFIG_CACHE_L2CC_128KB)
        /* 128KB, lock way7..4 */
        writel(0xf0, cns3xxx_l2_base + L2CC_LOCKDOWN_0_WAY_D);
        writel(0xf0, cns3xxx_l2_base + L2CC_LOCKDOWN_0_WAY_I);
        printk(KERN_INFO "CNS3XXX L2 cache lock down : way7..4\n");
#endif

	/* enable L2CC */
	writel(1, cns3xxx_l2_base + L2CC_CTRL);

	outer_cache.inv_range = cns3xxx_l2_inv_range;
	outer_cache.clean_range = cns3xxx_l2_clean_range;
	outer_cache.flush_range = cns3xxx_l2_flush_range;
	outer_cache.sync = cns3xxx_l2_cache_sync;
	outer_cache.flush_all = l2x0_flush_all;
	outer_cache.inv_all = cns3xxx_l2_inv_all;
	outer_cache.disable = cns3xxx_l2_disable;
	outer_cache.set_debug = l2x0_set_debug;

	printk("done.\n");
}
