/*
 * linux/include/asm-armnommu/arch-p52/memory.h
 *
 * Copyright (c) 1999 Nicolas Pitre <nico@cam.org>
 * 2001 Mindspeed
 */

#ifndef __ASM_ARCH_MEMORY_H
#define __ASM_ARCH_MEMORY_H

#include <asm/page.h>

#define TASK_SIZE	(0x01a00000UL)
#define TASK_SIZE_26	TASK_SIZE


extern unsigned long _end_kernel;
#define PHYS_OFFSET	((unsigned long) &_end_kernel)

#define PAGE_OFFSET DRAM_BASE
#define END_MEM     DRAM_BASE + DRAM_SIZE


#define __virt_to_phys(vpage) ((unsigned long) (vpage))
#define __phys_to_virt(ppage) ((void *) (ppage))
#define __virt_to_bus(vpage) ((unsigned long) (vpage))
#define __bus_to_virt(ppage) ((void *) (ppage))

#endif
