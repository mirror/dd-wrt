/*
 * linux/include/asm-armnommu/arch-ta7s/blkmem.h
 *
 * Copyright (c) 2003 Triscend Corp. <www.triscend.com>
 *
 * 04/09/2003  Craig Hackney	Initial.
 *
 * Contains configuration settings for the blkmem driver.
 */
#ifndef __ASM_ARCH_BLKMEM_H
#define __ASM_ARCH_BLKMEM_H

#include <asm/arch/triscend_a7.h>

#define CAT_ROMARRAY

extern char __bss_start[];

#define FIXUP_ARENAS \
	arena[0].address = ((unsigned long)__bss_start)+EXTERNAL_FLASH_BASE;

#endif
