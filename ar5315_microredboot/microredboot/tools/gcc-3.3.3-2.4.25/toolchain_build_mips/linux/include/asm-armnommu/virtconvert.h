#ifndef __M68KNOMMU_VIRT_CONVERT__
#define __M68KNOMMU_VIRT_CONVERT__

/*
 * Macros used for converting between virtual and physical mappings.
 */

#ifdef __KERNEL__

#define mm_vtop(vaddr)		((unsigned long) vaddr)

#endif
#endif
