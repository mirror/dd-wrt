/*
 *  linux/include/asm-armnommu/memory.h
 *
 *  Copyright (C) 2000 Russell King
 *  Copyright (C) 2001 RidgeRun, Inc (http://www.ridgerun.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Note: this file should not be included by non-asm/.h files
 *
 *  Modifications:
 *  02/20/01  Gordon McNutt  Leveraged for uClinux-2.4.x
 */
#ifndef __ASM_ARM_MEMORY_H
#define __ASM_ARM_MEMORY_H

#include <asm/arch/memory.h>

/*
 * Virtual, bus and physical addresses are all the same when there's no MMU.
 * --gmcnutt
 */
#define virt_to_bus(x) ((unsigned long) (x))
#define bus_to_virt(x) ((void *) (x))
#define virt_to_phys(x) ((unsigned long) (x))
#define phys_to_virt(x) ((void *) (x))

#define __page_address(page)	(PAGE_OFFSET + (((page) - mem_map) << PAGE_SHIFT))
#define page_to_phys(page)	virt_to_phys((void *)__page_address(page))

#if 0
/*
 * For some reason other asm/.h files refer to these instead of the more 
 * public macros above.
 * --gmcnutt
 */
#define __virt_to_bus(x) ((unsigned long) (x))
#define __bus_to_virt(x) ((void *) (x))
#define __virt_to_phys(x) ((unsigned long) (x))
#define __phys_to_virt(x) ((void *) (x))
#endif

/*
 * arch/armnommu/kernel/armksyms.c needs these.
 * --gmcnutt
 */
#define __virt_to_phys__is_a_macro
#define __phys_to_virt__is_a_macro
#define __virt_to_bus__is_a_macro
#define __bus_to_virt__is_a_macro

#endif
