/*
 *  linux/include/asm-arm/proc-armo/cache.h
 *
 *  Copyright (C) 1999-2000 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Cache handling for 26-bit ARM processors.
 */
#define flush_cache_all()			do { } while (0)
#define flush_cache_mm(mm)			do { } while (0)
#define flush_cache_range(mm,start,end)		do { } while (0)
#define flush_cache_page(vma,vmaddr)		do { } while (0)
#define flush_page_to_ram(page)			do { } while (0)

#define invalidate_dcache_range(start,end)	do { } while (0)
#define clean_dcache_range(start,end)		do { } while (0)
#define flush_dcache_range(start,end)		do { } while (0)
#define flush_dcache_page(page)			do { } while (0)
#define clean_dcache_entry(_s)      do { } while (0)
#define clean_cache_entry(_start)		do { } while (0)

#define flush_icache_range(start,end)		do { } while (0)
#define flush_icache_page(vma,page)		do { } while (0)
#define flush_icache_user_range(vma,pg,adr,len)		do { } while (0)


/* DAG: ARM3 will flush cache on MEMC updates anyway? so don't bother */
#define clean_cache_area(_start,_size) do { } while (0)

/*
 * TLB flushing:
 *
 *  - flush_tlb_all() flushes all processes TLBs
 *  - flush_tlb_mm(mm) flushes the specified mm context TLB's
 *  - flush_tlb_page(vma, vmaddr) flushes one page
 *  - flush_tlb_range(mm, start, end) flushes a range of pages
 */
#define flush_tlb_all()				memc_update_all()
#define flush_tlb_mm(mm)			do { } while (0)
#define flush_tlb_range(mm, start, end)		do { (void)(start); (void)(end); } while (0)
#define flush_tlb_page(vma, vmaddr)		do { } while (0)

/*
 * The following handle the weird MEMC chip
 */
extern __inline__ void memc_update_all(void)
{
	struct task_struct *p;

	cpu_memc_update_all(init_mm.pgd);
	for_each_task(p) {
		if (!p->mm)
			continue;
		cpu_memc_update_all(p->mm->pgd);
	}
	processor._set_pgd(current->active_mm->pgd);
}

extern __inline__ void memc_update_mm(struct mm_struct *mm)
{
	cpu_memc_update_all(mm->pgd);

	if (mm == current->active_mm)
		processor._set_pgd(mm->pgd);
}

extern __inline__ void
memc_update_addr(struct mm_struct *mm, pte_t pte, unsigned long vaddr)
{
	cpu_memc_update_entry(mm->pgd, pte_val(pte), vaddr);

	if (mm == current->active_mm)
		processor._set_pgd(mm->pgd);
}

extern __inline__ void
memc_clear(struct mm_struct *mm, struct page *page)
{
	cpu_memc_update_entry(mm->pgd, (unsigned long) page_address(page), 0);

	if (mm == current->active_mm)
		processor._set_pgd(mm->pgd);
}
