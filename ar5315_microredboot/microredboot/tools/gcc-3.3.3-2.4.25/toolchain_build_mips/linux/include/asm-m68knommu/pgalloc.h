#ifndef _M68KNOMMU_PGALLOC_H
#define _M68KNOMMU_PGALLOC_H

/*
 * Copyright (C) 2000 Lineo, David McCullough <davidm@lineo.com>
 * Copyright (C) 2001 Lineo, Greg Ungerer <gerg@snapgear.com>
 */

#include <asm/setup.h>
#include <asm/virtconvert.h>

/*
 * Cache handling functions
 */

extern inline void __flush_cache_all(void)
{
#ifdef CONFIG_M5407
	/*
	 *	Use cpushl to push and invalidate all cache lines.
	 *	Gas doesn't seem to know how to generate the ColdFire
	 *	cpushl instruction... Oh well, bit stuff it for now.
	 */
	__asm__ __volatile__ (
		"nop\n\t"
		"clrl	%%d0\n\t"
		"1:\n\t"
		"movel	%%d0,%%a0\n\t"
		"2:\n\t"
		".word	0xf468\n\t"
		"addl	#0x10,%%a0\n\t"
		"cmpl	#0x00000800,%%a0\n\t"
		"blt	2b\n\t"
		"addql	#1,%%d0\n\t"
		"cmpil	#4,%%d0\n\t"
		"bne	1b\n\t"
		"movel	#0xb6088500,%%d0\n\t"
		"movec	%%d0,%%CACR\n\t"
		: : : "d0", "a0" );
#endif /* CONFIG_M5407 */
}

#define flush_cache_all()			__flush_cache_all()
#define flush_cache_mm(mm)			__flush_cache_all()
#define flush_cache_range(mm, start, end)	__flush_cache_all()
#define flush_cache_page(vma, vmaddr)		__flush_cache_all()
#define flush_page_to_ram(page)			__flush_cache_all()
#define flush_dcache_page(page)			__flush_cache_all()
#define flush_icache_range(start, end)		__flush_cache_all()
#define flush_icache_user_range(vma,pg,adr,len) __flush_cache_all()
#define flush_icache_page(vma,pg)		__flush_cache_all()
#define flush_icache()				__flush_cache_all()

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

#endif /* _M68KNOMMU_PGALLOC_H */
