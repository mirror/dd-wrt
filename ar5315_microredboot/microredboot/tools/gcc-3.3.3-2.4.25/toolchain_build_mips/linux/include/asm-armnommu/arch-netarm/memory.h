/*
 * linux/include/asm-armnommu/arch-netarm/memory.h
 *
 * Copyright (c) 1999 Nicolas Pitre <nico@cam.org>
 * Copyright (c) 2001 RidgeRun <www.ridgerun.com>
 *
 * 02/19/2001  Gordon McNutt  Leveraged for the dsc21.
 * 06/29/2001  Rolf Peukert   A little tweak for the NetARM
 */

#ifndef __ASM_ARCH_MEMORY_H
#define __ASM_ARCH_MEMORY_H

/*
 * These are used in the fault handling code (even for nommu) and various other
 * macros. The current value comes from Steve Johnson's 2.0.38 uClinux port for
 * the dsc21 (TASK_SIZE = 0x01a00000UL).
 * --gmcnutt
 * The NetARM probably needs a value of about 32M --rp
 */
#define TASK_SIZE	(0x02000000UL)
#define TASK_SIZE_26	(0x01a00000UL)


/*
 * The start of physical memory. For the dsc21 this is the first SDRAM address.
 * This value is used in the arch-specific bootup code like setup_arch &
 * bootmem_init. By default we assume that we have one block of contiguous
 * memory (a node) that starts here and runs MEM_SIZE long (see below). By 
 * default the ARM bootup code sets MEM_SIZE to be 16MB  which is just right for
 * us (see arch/armnommu/kernel/setup.c).
 *
 * NOTE:   I don't really use the start of SDRAM. Instead I reserve some pages
 *         of SDRAM for some of the kernel sections (like .data & .bss). The
 *         linker script has been modified to mark the first unreservered page
 *         as _end_kernel (see arch/armnommu/vmlinux-armv.lds.in).
 *         
 * --gmcnutt
 */
extern unsigned long _end_kernel;
#define PHYS_OFFSET	0
//0x08000000
//#define PHYS_OFFSET	0

/*
 * PAGE_OFFSET -- the first address of the first page of memory. For archs with
 * no MMU this corresponds to the first free page in physical memory (aligned
 * on a page boundary).
 * --gmcnutt
 */
#define PAGE_OFFSET  PHYS_OFFSET

/* MEM_SIZE for the NetARM board is DRAM_SIZE (32MB) minus kernel space */
#define MEM_SIZE	(DRAM_SIZE - PHYS_OFFSET)
#define END_MEM		DRAM_BASE + DRAM_SIZE

#define __virt_to_phys(vpage) ((unsigned long) (vpage))
#define __phys_to_virt(ppage) ((void *) (ppage))
#define __virt_to_bus(vpage) ((unsigned long) (vpage))
#define __bus_to_virt(ppage) ((void *) (ppage))

#endif
