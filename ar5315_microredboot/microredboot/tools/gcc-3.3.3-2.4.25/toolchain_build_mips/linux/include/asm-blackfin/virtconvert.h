#ifndef __FRIONOMMU_VIRT_CONVERT__
#define __FRIONOMMU_VIRT_CONVERT__

/*
 * Macros used for converting between virtual and physical mappings.
 */

#ifdef __KERNEL__

#include <linux/config.h>
#include <asm/setup.h>
#include <asm/page.h>

#define mm_vtop(vaddr)		((unsigned long) vaddr)
#define mm_ptov(vaddr)		((unsigned long) vaddr)
#define phys_to_virt(vaddr)	((unsigned long) vaddr)
#define virt_to_phys(vaddr)	((unsigned long) vaddr)

#define virt_to_bus virt_to_phys
#define bus_to_virt phys_to_virt

#define __page_address(page) 	(PAGE_OFFSET + (((page) - mem_map) << PAGE_SHIFT))
#define page_to_phys(page)	virt_to_phys((void *)__page_address(page))

#endif /* __KERNEL__ */
#endif /* __FRIONOMMU_VIRT_CONVERT__ */
