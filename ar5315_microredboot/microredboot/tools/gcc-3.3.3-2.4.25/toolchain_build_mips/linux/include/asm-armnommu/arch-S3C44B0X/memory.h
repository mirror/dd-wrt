/*
 * uclinux/include/asm-armnommu/arch-S3C44B0X/memory.h
 *
 * Copyright (C) 2003 Thomas Eschenbacher <eschenbacher@sympat.de>
 *
 */

#ifndef __ASM_ARCH_MEMORY_H
#define __ASM_ARCH_MEMORY_H

#define TASK_SIZE	(0x01a00000UL)
#define TASK_SIZE_26	TASK_SIZE

#define PHYS_OFFSET     (DRAM_BASE + 2*1024*1024)
#define PAGE_OFFSET 	(PHYS_OFFSET)
#define END_MEM		(DRAM_BASE + DRAM_SIZE - 2*1024*1024)

#define __virt_to_phys__is_a_macro
#define __virt_to_phys(vpage) ((unsigned long) (vpage))
#define __phys_to_virt__is_a_macro
#define __phys_to_virt(ppage) ((void *) (ppage))

#define __virt_to_bus__is_a_macro
#define __virt_to_bus(vpage) ((unsigned long) (vpage))
#define __bus_to_virt__is_a_macro
#define __bus_to_virt(ppage) ((void *) (ppage))

#endif /* __ASM_ARCH_MEMORY_H */
