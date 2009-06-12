/*
 * linux/include/asm-armnommu/arch-c5471/memory.h
 */

#ifndef __ASM_ARCH_MEMORY_H
#define __ASM_ARCH_MEMORY_H

#include <linux/autoconf.h>

#define __virt_to_bus(x) ((unsigned long) (x))
#define __bus_to_virt(x) ((void *) (x))
#define __virt_to_phys(x) ((unsigned long) (x))
#define __phys_to_virt(x) ((void *) (x))

#define END_MEM		(DRAM_BASE + DRAM_SIZE)

/* These are used in the fault handling code (even for nommu) and various other
 * macros.
 */

#define TASK_SIZE	(0x01a00000UL)
#define TASK_SIZE_26	TASK_SIZE

/* The alignment requirement of the memory management system
 * this is ~(1 << (PAGE_OFFSET+(MAX_ORDER-1)))-1.
 */

#define PHYS_OFFSET_MASK 0x001fffff

/* The start of physical memory.  This is the first SDRAM address.
 * This value is used in the arch-specific bootup code like setup_arch &
 * bootmem_init.
 *
 * NOTE the alignment requirement of the memory management system
 * this is given by PHYS_OFFSET_MASK.
 *
 * Boot from flash -  kernel .text and .rodata are in rom/flash and only
 * .data lies in SDRAM.  So the mem_map tables can begin near the
 * beginning of physical SDRAM.  The value _end_kernel provides
 * SDRAM address after the memory used by the kernel .data section.
 * There can still be a significant loss of memory due to the alignment
 * requirement.
 */

/*
  extern unsigned long _end_kernel;
#define PHYS_OFFSET \
  ((((unsigned long)&_end_kernel) + PHYS_OFFSET_MASK) & ~PHYS_OFFSET_MASK)
*/


#define PHYS_OFFSET \
  ((((unsigned long)DRAM_BASE) + PHYS_OFFSET_MASK) & ~PHYS_OFFSET_MASK)


#define UNALIGNED_PHYS_OFFSET ((unsigned long)&_end_kernel)

#define LOST_MEMORY_SIZE (PHYS_OFFSET - UNALIGNED_PHYS_OFFSET)

/* PAGE_OFFSET -- the first address of the first page of memory. For archs with
 * no MMU this corresponds to the first free page in physical memory (aligned
 * on a page boundary).
 */

#define PAGE_OFFSET PHYS_OFFSET

#endif
