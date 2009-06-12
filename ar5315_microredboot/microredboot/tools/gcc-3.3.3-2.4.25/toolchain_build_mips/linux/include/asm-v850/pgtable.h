#ifndef __V850_PGTABLE_H__
#define __V850_PGTABLE_H__

#include <linux/config.h>

#define pgd_present(pgd)     (1)       /* pages are always present on NO_MM */
#define kern_addr_valid(addr) (1)

/* These mean nothing to !CONFIG_MMU.  */
#define PAGE_NONE		__pgprot(0)
#define PAGE_SHARED		__pgprot(0)
#define PAGE_COPY		__pgprot(0)
#define PAGE_READONLY		__pgprot(0)
#define PAGE_KERNEL		__pgprot(0)

extern void paging_init(void);

#define pgtable_cache_init()   ((void)0)

#endif /* __V850_PGTABLE_H__ */
