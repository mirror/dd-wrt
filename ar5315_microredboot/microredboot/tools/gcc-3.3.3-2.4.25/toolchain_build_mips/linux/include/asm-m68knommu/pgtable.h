#ifndef _M68KNOMMU_PGTABLE_H
#define _M68KNOMMU_PGTABLE_H

#include <linux/config.h>
#include <asm/setup.h>

#define pgd_present(pgd)     (1)       /* pages are always present on NO_MM */
#define kern_addr_valid(addr) (1)

#define PAGE_NONE		__pgprot(0)    /* these mean nothing to NO_MM */
#define PAGE_SHARED		__pgprot(0)    /* these mean nothing to NO_MM */
#define PAGE_COPY		__pgprot(0)    /* these mean nothing to NO_MM */
#define PAGE_READONLY	__pgprot(0)    /* these mean nothing to NO_MM */
#define PAGE_KERNEL		__pgprot(0)    /* these mean nothing to NO_MM */

extern void paging_init(void);

/*
 * No page table caches to initialise
 */
#define pgtable_cache_init()   do { } while (0)

#endif /* _M68KNOMMU_PGTABLE_H */
