/*
 * include/asm-microblaze/page.h -- VM ops
 *
 *  Copyright (C) 2003        John Williams <jwilliams@itee.uq.edu.au>
 *  Copyright (C) 2001, 2002  NEC Corporation
 *  Copyright (C) 2001, 2002  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file COPYING in the main directory of this
 * archive for more details.
 *
 * Written by Miles Bader <miles@gnu.org>
 * Microblaze port by John Williams
 */

#ifndef __MICROBLAZE_PAGE_H__
#define __MICROBLAZE_PAGE_H__

#include <asm/machdep.h>


#define PAGE_SHIFT	12
#define PAGE_SIZE       (1UL << PAGE_SHIFT)
#define PAGE_MASK       (~(PAGE_SIZE-1))

/*
 * PAGE_OFFSET -- the first address of the first page of memory. For archs with
 * no MMU this corresponds to the first free page in physical memory (aligned
 * on a page boundary).
 */
#ifndef PAGE_OFFSET
#define PAGE_OFFSET  0x0000000
#endif

/*
 * MAP_NR -- given an address, calculate the index of the page struct which
 * points to the address's page.
 */
#define MAP_NR(addr) (((unsigned long)(addr) - PAGE_OFFSET) >> PAGE_SHIFT)

#ifdef __KERNEL__
#ifndef __ASSEMBLY__

#define STRICT_MM_TYPECHECKS

#define clear_page(page)		memset ((void *)(page), 0, PAGE_SIZE)
extern void copy_page(void *to, void *from);

#define clear_user_page(page, vaddr)	clear_page (page)
#define copy_user_page(to, from, vaddr)	copy_page (to, from)

#ifdef STRICT_MM_TYPECHECKS
/*
 * These are used to make use of C type-checking..
 */

typedef struct { unsigned long pte; } pte_t;
typedef struct { unsigned long pmd; } pmd_t;
typedef struct { unsigned long pgd; } pgd_t;
typedef struct { unsigned long pgprot; } pgprot_t;

#define pte_val(x)      ((x).pte)
#define pmd_val(x)      ((x).pmd)
#define pgd_val(x)      ((x).pgd)
#define pgprot_val(x)   ((x).pgprot)

#define __pte(x)        ((pte_t) { (x) } )
#define __pmd(x)        ((pmd_t) { (x) } )
#define __pgd(x)        ((pgd_t) { (x) } )
#define __pgprot(x)     ((pgprot_t) { (x) } )

#else /* !STRICT_MM_TYPECHECKS */
/*
 * .. while these make it easier on the compiler
 */

typedef unsigned long pte_t;
typedef unsigned long pmd_t;
typedef unsigned long pgd_t;
typedef unsigned long pgprot_t;

#define pte_val(x)      (x)
#define pmd_val(x)      (x)
#define pgd_val(x)      (x)
#define pgprot_val(x)   (x)

#define __pte(x)        (x)
#define __pmd(x)        (x)
#define __pgd(x)        (x)
#define __pgprot(x)     (x)

#endif /* STRICT_MM_TYPECHECKS */

#endif /* !__ASSEMBLY__ */

/* to align the pointer to the (next) page boundary */
#define PAGE_ALIGN(addr)	(((addr) + PAGE_SIZE - 1) & PAGE_MASK)

#ifndef __ASSEMBLY__

extern void __bug(const char *file, int line, void *data);

#define BUG()		__bug(__FILE__, __LINE__, NULL)
#define PAGE_BUG(page)	__bug(__FILE__, __LINE__, page)

/* Pure 2^n version of get_order */
extern __inline__ int get_order (unsigned long size)
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

#endif /* !__ASSEMBLY__ */

#define __pa(x)			__virt_to_phys ((unsigned long)(x))
#define __va(x)			((void *)__phys_to_virt ((unsigned long)(x)))

/* No current microblaze processor has virtual memory.  */
#define __virt_to_phys(addr)	(addr)
#define __phys_to_virt(addr)	(addr)

#define virt_to_page(kaddr) 	(mem_map +  MAP_NR (kaddr))
#define VALID_PAGE(page) 	((page - mem_map) < max_mapnr)

#endif /* KERNEL */

#ifdef CONFIG_CONTIGUOUS_PAGE_ALLOC
#define WANT_PAGE_VIRTUAL 1 /* page alloc 2 relies on this */
#endif

#endif /* __MICROBLAZE_PAGE_H__ */
