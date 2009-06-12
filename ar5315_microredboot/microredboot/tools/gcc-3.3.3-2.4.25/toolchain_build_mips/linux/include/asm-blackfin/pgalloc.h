#ifndef _FRIONOMMU_PGALLOC_H
#define _FRIONOMMU_PGALLOC_H

/*
 * Copyright (C) 2000 Lineo, David McCullough <davidm@lineo.com>
 */

#include <asm/setup.h>
#include <asm/virtconvert.h>

/*
 * Cache handling functions
 */

#if DAVIDM
/*
 * invalidate the cache for the specified memory range.
 * It starts at the physical address specified for
 * the given number of bytes.
 */
extern void cache_clear(unsigned long paddr, int len);

/*
 * push any dirty cache in the specified memory range.
 * It starts at the physical address specified for
 * the given number of bytes.
 */
extern void cache_push(unsigned long paddr, int len);

/*
 * push and invalidate pages in the specified user virtual
 * memory range.
 */
extern void cache_push_v(unsigned long vaddr, int len);

/* cache code */
#define FLUSH_I_AND_D	(0x00000808)
#define FLUSH_I 	(0x00000008)

/* This is needed whenever the virtual mapping of the current
   process changes.  */
#endif

static inline void __flush_cache_all(void)
{
}

#define flush_cache_all()				__flush_cache_all()
#define flush_cache_mm(mm)				__flush_cache_all()
#define flush_cache_range(mm, start, end)		__flush_cache_all()
#define flush_cache_page(vma, vmaddr)			__flush_cache_all()
#define flush_page_to_ram(page)				__flush_cache_all()
#define flush_dcache_page(page)				__flush_cache_all()
#define flush_icache_range(start, end)			__flush_cache_all()
#define flush_icache_user_range(vma,pg,addr,len)	__flush_cache_all()
#define flush_icache_page(vma,pg)			__flush_cache_all()
#define flush_icache()					__flush_cache_all()

/*
 * flush all user-space atc entries.
 */
static inline void __flush_tlb(void)
{
	printk("%s,%d: called %s!\n", __FILE__,  __LINE__, __FUNCTION__);
}

static inline void __flush_tlb_one(unsigned long addr)
{
	printk("%s,%d: called %s!\n", __FILE__,  __LINE__, __FUNCTION__);
}

#define flush_tlb() __flush_tlb()

/*
 * DAVIDM - the rest of these are just so I can check where they happen
 */
/*
 * flush all atc entries (both kernel and user-space entries).
 */
static inline void flush_tlb_all(void)
{
	printk("%s,%d: DAVIDM called %s!\n", __FILE__,  __LINE__, __FUNCTION__);
}

static inline void flush_tlb_mm(struct mm_struct *mm)
{
	printk("%s,%d: DAVIDM called %s!\n", __FILE__,  __LINE__, __FUNCTION__);
}

static inline void flush_tlb_page(struct vm_area_struct *vma, unsigned long addr)
{
	printk("%s,%d: DAVIDM called %s!\n", __FILE__,  __LINE__, __FUNCTION__);
}

static inline void flush_tlb_range(struct mm_struct *mm,
				   unsigned long start, unsigned long end)
{
	printk("%s,%d: DAVIDM called %s!\n", __FILE__,  __LINE__, __FUNCTION__);
}

static inline void flush_tlb_kernel_page(unsigned long addr)
{
	printk("%s,%d: DAVIDM called %s!\n", __FILE__,  __LINE__, __FUNCTION__);
}

static inline void flush_tlb_pgtables(struct mm_struct *mm,
				      unsigned long start, unsigned long end)
{
	printk("%s,%d: DAVIDM called %s!\n", __FILE__,  __LINE__, __FUNCTION__);
}

#endif /* _FRIONOMMU_PGALLOC_H */
