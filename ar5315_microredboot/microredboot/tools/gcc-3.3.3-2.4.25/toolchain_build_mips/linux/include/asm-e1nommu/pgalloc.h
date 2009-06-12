#ifndef _HYPERSTONE_PGALLOC_H
#define _HYPERSTONE_PGALLOC_H

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
#endif  /* !DAVIDM */

#define __flush_cache_all()  do{ } while(0)/* FIXME Invalidate Caches!  */	

#define flush_cache_all()			__flush_cache_all()
#define flush_cache_mm(mm)			__flush_cache_all()
#define flush_cache_range(mm, start, end)	__flush_cache_all()
#define flush_cache_page(vma, vmaddr)		__flush_cache_all()
#define flush_page_to_ram(page)			__flush_cache_all()
#define flush_dcache_page(page)			__flush_cache_all()
#define flush_icache_range(start, end)		__flush_cache_all()
#define flush_icache_page(vma,pg)		__flush_cache_all()
#define flush_icache()				__flush_cache_all()
#define flush_icache_user_range(vma,pg,adr,len) __flush_cache_all()

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

#endif /* _HYPERSTONE_PGALLOC_H */
