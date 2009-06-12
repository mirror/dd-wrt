/*
 * linux/include/asm-arm/arch-a5k/mmu.h
 *
 * Copyright (c) 1996 Russell King.
 *
 * Changelog:
 *  22-11-1996	RMK	Created
 */
#ifndef __ASM_ARCH_MMU_H
#define __ASM_ARCH_MMU_H

#define __virt_to_phys__is_a_macro
#define __virt_to_phys(vpage) vpage
#define __phys_to_virt__is_a_macro
#define __phys_to_virt(ppage) ppage

#define __virt_to_bus__is_a_macro
#define __virt_to_bus(vpage) vpage
#define __bus_to_virt__is_a_macro
#define __bus_to_virt(ppage) ppage

#define __flush_entry_to_
#endif
