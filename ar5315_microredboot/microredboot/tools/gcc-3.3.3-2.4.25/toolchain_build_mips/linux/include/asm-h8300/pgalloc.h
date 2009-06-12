#ifndef _H8300_PGALLOC_H
#define _H8300_PGALLOC_H

/*
 * Copyright (C) 2000 Lineo, David McCullough <davidm@lineo.com>
 * Copyright (C) 2001 Lineo, Greg Ungerer <gerg@snapgear.com>
 */

#include <asm/setup.h>
#include <asm/virtconvert.h>

#define flush_cache_all()                       do { } while(0)
#define flush_cache_mm(mm)                      do { } while(0)
#define flush_cache_range(mm, start, end)       do { } while(0)
#define flush_cache_page(vma, vmaddr)           do { } while(0)
#define flush_page_to_ram(page)                 do { } while(0)
#define flush_dcache_page(page)                 do { } while(0)
#define flush_icache_range(start, end)          do { } while(0)
#define flush_icache_user_range(vma,pg,adr,len) do { } while(0)
#define flush_icache_page(vma,pg)               do { } while(0)
#define flush_icache()                          do { } while(0)

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

#endif /* _H8300_PGALLOC_H */
