/*
 *  linux/include/asm-arm/mmu_context.h
 *
 *  Copyright (C) 1996 Russell King.
 *  Copyright (C) 2001 RidgRun Inc (www.ridgerun.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Changelog:
 *   27-06-1996	RMK	Created
 *   20-02-2001 GJM     Gutted for uClinux
 */
#ifndef __ASM_ARM_MMU_CONTEXT_H
#define __ASM_ARM_MMU_CONTEXT_H

#define destroy_context(mm)		do { } while(0)
#define init_new_context(tsk,mm)	0
#define switch_mm(prev,next,tsk,cpu)
#define activate_mm(prev,next)
#define enter_lazy_tlb(mm,tsk,cpu)

#endif
