#ifndef _FRIONOMMU_PGTABLE_H
#define _FRIONOMMU_PGTABLE_H

#include <linux/config.h>
#include <asm/setup.h>

/* XXX: defined in <linux/mm.h>
#define page_address(page)   ({ if (!(page)->virtual) BUG(); (page)->virtual;})
*/
/* XXX: defined in <asm/virtconvert.h>
#define __page_address(page) (PAGE_OFFSET + (((page) - mem_map) << PAGE_SHIFT))
*/
#define kern_addr_valid(addr) (1)

#define PAGE_NONE		__pgprot(0)    /* these mean nothing to NO_MM */
#define PAGE_SHARED		__pgprot(0)    /* these mean nothing to NO_MM */
#define PAGE_COPY		__pgprot(0)    /* these mean nothing to NO_MM */
#define PAGE_READONLY		__pgprot(0)    /* these mean nothing to NO_MM */
#define PAGE_KERNEL		__pgprot(0)    /* these mean nothing to NO_MM */

extern void paging_init(void);

/*
 * No page table caches to initialise.
 */
#define pgtable_cache_init()	do {} while (0)

#endif /* _FRIONOMMU_PGTABLE_H */
