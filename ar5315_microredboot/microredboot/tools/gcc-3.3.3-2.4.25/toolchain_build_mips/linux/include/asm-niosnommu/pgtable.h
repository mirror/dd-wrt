#ifndef _NIOS_PGTABLE_H
#define _NIOS_PGTABLE_H


//vic - this bit copied from m68knommu version
#include <linux/config.h>
#include <asm/setup.h>

#define pgd_present(pgd)     (1)       /* pages are always present on NO_MM */
#define kern_addr_valid(addr) (1)

#define PAGE_NONE		__pgprot(0)    /* these mean nothing to NO_MM */
#define PAGE_SHARED		__pgprot(0)    /* these mean nothing to NO_MM */
#define PAGE_COPY		__pgprot(0)    /* these mean nothing to NO_MM */
#define PAGE_READONLY	__pgprot(0)    /* these mean nothing to NO_MM */
#define PAGE_KERNEL		__pgprot(0)    /* these mean nothing to NO_MM */
//vic - this bit copied from m68knommu version


extern void paging_init(void);

/*
 * No page table caches to initialise
 */
#define pgtable_cache_init()   do { } while (0)

extern inline void flush_cache_mm(struct mm_struct *mm)
{
}

extern inline void flush_cache_range(struct mm_struct *mm,
				     unsigned long start,
				     unsigned long end)
{
}

/* Push the page at kernel virtual address and clear the icache */
extern inline void flush_page_to_ram (unsigned long address)
{
}

/* Push n pages at kernel virtual address and clear the icache */
extern inline void flush_pages_to_ram (unsigned long address, int n)
{
}


#endif /* _NIOS_PGTABLE_H */
