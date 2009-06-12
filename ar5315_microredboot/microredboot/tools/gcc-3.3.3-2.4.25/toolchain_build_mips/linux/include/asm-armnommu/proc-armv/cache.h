/*
 *  linux/include/asm-arm/proc-armv/cache.h
 *
 *  Copyright (C) 1999-2001 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <asm/mman.h>

/*
 * Cache handling for 32-bit ARM processors.
 *
 * Note that on ARM, we have a more accurate specification than that
 * Linux's "flush".  We therefore do not use "flush" here, but instead
 * use:
 *
 * clean:      the act of pushing dirty cache entries out to memory.
 * invalidate: the act of discarding data held within the cache,
 *             whether it is dirty or not.
 */

/*
 * Generic I + D cache
 */
#define flush_cache_all()						\
	do {								\
		cpu_cache_clean_invalidate_all();			\
	} while (0)

/* This is always called for current->mm */
#define flush_cache_mm(_mm)						\
	do {								\
		if ((_mm) == current->active_mm)			\
			cpu_cache_clean_invalidate_all();		\
	} while (0)

#define flush_cache_range(_mm,_start,_end)				\
	do {								\
		if ((_mm) == current->mm)				\
			cpu_cache_clean_invalidate_range((_start), (_end), 1); \
	} while (0)

#define flush_cache_page(_vma,_vmaddr)					\
	do {								\
		if (1 /* DAVIDM (_vma)->vm_mm == current->mm */) {			\
			cpu_cache_clean_invalidate_range((_vmaddr),	\
				(_vmaddr) + PAGE_SIZE,			\
				((_vma)->vm_flags & VM_EXEC));		\
		} \
	} while (0)

/*
 * This flushes back any buffered write data.  We have to clean the entries
 * in the cache for this page.  This does not invalidate either I or D caches.
 */
static __inline__ void flush_page_to_ram(struct page *page)
{
	cpu_flush_ram_page(page_address(page));
}

/*
 * D cache only
 */

#define invalidate_dcache_range(_s,_e)	cpu_dcache_invalidate_range((_s),(_e))
#define clean_dcache_range(_s,_e)	cpu_dcache_clean_range((_s),(_e))
#define flush_dcache_range(_s,_e)	cpu_cache_clean_invalidate_range((_s),(_e),0)

/*
 * FIXME: We currently clean the dcache for this page.  Should we
 * also invalidate the Dcache?  And what about the Icache? -- rmk
 */
#define flush_dcache_page(page)		cpu_dcache_clean_page(page_address(page))

#define clean_dcache_entry(_s)		cpu_dcache_clean_entry((unsigned long)(_s))

/*
 * I cache only
 */
#define flush_icache_range(_s,_e)					\
	do {								\
		cpu_icache_invalidate_range((_s), (_e));		\
	} while (0)

/* DAVIDM - this could be better */
#define flush_icache_user_range(vma,pg,adr,len) flush_cache_all()

#define flush_icache_page(vma,pg)					\
	do {								\
		if ((vma)->vm_flags & PROT_EXEC)			\
			cpu_icache_invalidate_page(page_address(pg));	\
	} while (0)

/*
 * Old ARM MEMC stuff.  This supports the reversed mapping handling that
 * we have on the older 26-bit machines.  We don't have a MEMC chip, so...
 */
#define memc_update_all()		do { } while (0)
#define memc_update_mm(mm)		do { } while (0)
#define memc_update_addr(mm,pte,log)	do { } while (0)
#define memc_clear(mm,physaddr)		do { } while (0)

/*
 * TLB flushing.
 *
 *  - flush_tlb_all()			flushes all processes TLBs
 *  - flush_tlb_mm(mm)			flushes the specified mm context TLB's
 *  - flush_tlb_page(vma, vmaddr)	flushes TLB for specified page
 *  - flush_tlb_range(mm, start, end)	flushes TLB for specified range of pages
 *
 * We drain the write buffer in here to ensure that the page tables in ram
 * are really up to date.  It is more efficient to do this here...
 */

/*
 * Notes:
 *  current->active_mm is the currently active memory description.
 *  current->mm == NULL iff we are lazy.
 */
#define flush_tlb_all()							\
	do {								\
		cpu_tlb_invalidate_all();				\
	} while (0)

/*
 * Flush all user virtual address space translations described by `_mm'.
 *
 * Currently, this is always called for current->mm, which should be
 * the same as current->active_mm.  This is currently not be called for
 * the lazy TLB case.
 */
#define flush_tlb_mm(_mm)						\
	do {								\
		if ((_mm) == current->active_mm)			\
			cpu_tlb_invalidate_all();			\
	} while (0)

/*
 * Flush the specified range of user virtual address space translations.
 *
 * _mm may not be current->active_mm, but may not be NULL.
 */
#define flush_tlb_range(_mm,_start,_end)				\
	do {								\
		if ((_mm) == current->active_mm)			\
			cpu_tlb_invalidate_range((_start), (_end));	\
	} while (0)

/*
 * Flush the specified user virtual address space translation.
 */
#define flush_tlb_page(_vma,_page)					\
	do {								\
		if (1 /* DAVIDM (_vma)->vm_mm == current->active_mm */)		\
			cpu_tlb_invalidate_page((_page),		\
				 ((_vma)->vm_flags & VM_EXEC));		\
	} while (0)

/*
 * 32-bit ARM Processors don't have any MMU cache
 */
#define update_mmu_cache(vma,address,pte) do { } while (0)

