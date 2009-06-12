/*
 * linux/include/asm-armnommu/arch-ta7s/memory.h
 *
 * Copyright (c) 2003 Triscend Corp. <www.triscend.com>
 *
 * 04/09/2003  Craig Hackney	Initial.
 */

#ifndef __ASM_ARCH_MEMORY_H
#define __ASM_ARCH_MEMORY_H

/*
 * These should be the same as the definitions in asm/memory.h
 * without the '__' at the beginning.
 */
#define __virt_to_bus(x) ((unsigned long) (x))
#define __bus_to_virt(x) ((void *) (x))
#define __virt_to_phys(x) ((unsigned long) (x))
#define __phys_to_virt(x) ((void *) (x))

#define TASK_SIZE	(0x02000000UL)
#define TASK_SIZE_26	(0x01a00000UL)


#define PHYS_OFFSET	DRAM_BASE
#define PAGE_OFFSET	PHYS_OFFSET

#define MEM_SIZE	(DRAM_SIZE - PHYS_OFFSET)
#define END_MEM		(DRAM_BASE + DRAM_SIZE)

#endif
