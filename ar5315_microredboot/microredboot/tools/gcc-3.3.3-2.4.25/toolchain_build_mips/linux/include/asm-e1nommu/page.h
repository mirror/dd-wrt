#ifndef _HYPERSTONE_PAGE_H_
#define _HYPERSTONE_PAGE_H_

#include <linux/config.h>

/* PAGE_SHIFT determines the page size */

#define PAGE_SHIFT      (12)
#define PAGE_SIZE       (4096)
#define PAGE_MASK       (~(PAGE_SIZE-1))

#ifdef __KERNEL__

#include <asm/setup.h>

#define PAGE_ALIGN(addr)	(((addr)+PAGE_SIZE-1) & PAGE_MASK)

#define PAGE_OFFSET     0x00000000

#define get_user_page(vaddr)            __get_free_page(GFP_KERNEL)
#define free_user_page(page, addr)      free_page(addr)

#define clear_page(page)        memset((page), 0, PAGE_SIZE)
#define copy_page(to,from)      memcpy((to), (from), PAGE_SIZE)

#define clear_user_page(page, vaddr)    clear_page(page)
#define copy_user_page(to, from, vaddr) copy_page(to, from)

#ifdef STRICT_MM_TYPECHECKS
/*
 * These are used to make use of C type-checking..
 */
typedef struct { unsigned long pte; } pte_t;
typedef struct { unsigned long pmd[16]; } pmd_t;
typedef struct { unsigned long pgd; } pgd_t;
typedef struct { unsigned long pgprot; } pgprot_t;

#define pte_val(x)      ((x).pte)
#define pmd_val(x)      ((&x)->pmd[0])
#define pgd_val(x)      ((x).pgd)
#define pgprot_val(x)   ((x).pgprot)

#define __pte(x)        ((pte_t) { (x) } )
#define __pmd(x)        ((pmd_t) { (x) } )
#define __pgd(x)        ((pgd_t) { (x) } )
#define __pgprot(x)     ((pgprot_t) { (x) } )

#else
/*
 * .. while these make it easier on the compiler
 */
typedef unsigned long pte_t;
typedef struct { unsigned long pmd[16]; } pmd_t;
typedef unsigned long pgd_t;
typedef unsigned long pgprot_t;

#define pte_val(x)      (x)
#define pmd_val(x)      ((&x)->pmd[0])
#define pgd_val(x)      (x)
#define pgprot_val(x)   (x)

#define __pte(x)        (x)
#define __pmd(x)        ((pmd_t) { (x) } )
#define __pgd(x)        (x)
#define __pgprot(x)     (x)

#endif

#define __pa(vaddr)             virt_to_phys((void *)vaddr)
#define __va(paddr)             phys_to_virt((unsigned long)paddr)
#define MAP_NR(addr)            (((unsigned long)(addr)-PAGE_OFFSET) >> PAGE_SHIFT)
#define virt_to_page(addr)      (mem_map + (((unsigned long)(addr)-PAGE_OFFSET) >> PAGE_SHIFT))
#define VALID_PAGE(page)        ((page - mem_map) < max_mapnr)

#define BUG() do { \
        printk("kernel BUG at %s:%d!\n", __FILE__, __LINE__); \
		while(1); \
        __asm__ __volatile__(".word 0x0000"); \
} while (0)                                       

#define PAGE_BUG(page) do { \
        BUG(); \
} while (0)

#endif /* __KERNEL__ */

/* Pure 2^n version of get_order */
extern __inline__ int get_order(unsigned long size)
{
    int order;

    size = (size-1) >> (PAGE_SHIFT-1);
    order = -1;
    do {
        size >>= 1;
        order++;
    } while (size);
    return order;
}


#endif /* !_HYPERSTONE_PAGE_H_ */
