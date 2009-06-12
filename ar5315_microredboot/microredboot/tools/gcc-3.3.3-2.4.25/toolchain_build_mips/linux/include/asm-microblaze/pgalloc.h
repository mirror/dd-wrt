#ifndef _MICROBLAZE_PGALLOC_H
#define _MICROBLAZE_PGALLOC_H

/*
 * Copyright (C) 2003 John Williams <jwilliams@itee.uq.edu.au>
 * Copyright (C) 2000 Lineo, David McCullough <davidm@lineo.com>
 * Copyright (C) 2001 Lineo, Greg Ungerer <gerg@snapgear.com>
 */

#include <asm/setup.h>
#include <asm/virtconvert.h>
#include <asm/page.h>

/*
 * Cache handling functions.
 * Microblaze has a write-through data cache, meaning that the data cache
 * never needs to be flushed.  The only flushing operations that are
 * implemented are to invalidate the instruction cache.  These are called
 * after loading a user application into memory, we must invalidate the
 * instruction cache to make sure we don't fetch old, bad code.
 */

extern inline void __flush_icache_all(void)
{
#ifdef CONFIG_MICROBLAZE_ICACHE
	unsigned int i;

	__disable_icache();

	for(i=0;i<CONFIG_MICROBLAZE_ICACHE_SIZE;i+=4)
		__invalidate_icache(i+CONFIG_MICROBLAZE_ICACHE_BASE);

	__enable_icache();
#else
	do { } while(0);
#endif /* CONFIG_MICROBLAZE_ICACHE */
}

extern inline void __flush_icache_range(unsigned int start, unsigned int end)
{
#ifdef CONFIG_MICROBLAZE_ICACHE
	unsigned int i;

	__disable_icache();

	for(i=start;i<end;i+=4)
		__invalidate_icache(i);

	__enable_icache();
#else
	do { } while(0);
#endif /* CONFIG_MICROBLAZE_ICACHE */
}

#define flush_cache_all()			__flush_icache_all()
#define flush_cache_mm(mm)			do { } while(0)
#define flush_cache_range(mm, start, end)	do { } while(0)
#define flush_cache_page(vma, vmaddr)		do { } while(0)
#define flush_page_to_ram(page)			do { } while(0)
#define flush_dcache_page(page)			do { } while(0)
#define flush_icache_range(start, end)		__flush_icache_range(start,end)
#define flush_icache_user_range(vma,pg,adr,len) __flush_icache_all()
#define flush_icache_page(vma,pg)		__flush_icache_all()
#define flush_icache()				__flush_icache_all()
#define flush_cache_sigtramp(vaddr)		__flush_icache_range(vaddr,vaddr+4)

/*
 * DAVIDM - the rest of these are just so I can check where they happen
 */

/*
 * flush all user-space atc entries.
 */
static inline void __flush_tlb(void)
{
	BUG();
}

static inline void __flush_tlb_one(unsigned long addr)
{
	BUG();
}

#define flush_tlb() __flush_tlb()

/*
 * flush all atc entries (both kernel and user-space entries).
 */
static inline void flush_tlb_all(void)
{
	BUG();
}

static inline void flush_tlb_mm(struct mm_struct *mm)
{
	BUG();
}

static inline void flush_tlb_page(struct vm_area_struct *vma, unsigned long addr)
{
	BUG();
}

static inline void flush_tlb_range(struct mm_struct *mm,
				   unsigned long start, unsigned long end)
{
	BUG();
}

extern inline void flush_tlb_kernel_page(unsigned long addr)
{
	BUG();
}

extern inline void flush_tlb_pgtables(struct mm_struct *mm,
				      unsigned long start, unsigned long end)
{
	BUG();
}

#endif /* _MICROBLAZE_PGALLOC_H */
