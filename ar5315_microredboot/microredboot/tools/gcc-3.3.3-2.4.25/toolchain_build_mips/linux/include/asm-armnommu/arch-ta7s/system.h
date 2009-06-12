/*
 * linux/include/asm-armnommu/arch-ta7s/system.h
 *
 * Copyright (c) 1999 Nicolas Pitre <nico@cam.org>
 * Copyright (c) 2001 RidgeRun, Inc (http://www.ridgerun.org)
 *
 */
#include <asm/arch/arch.h>

static inline void arch_idle(void)
{
	while (!current->need_resched && !hlt_counter);
}

extern inline void arch_reset(char mode)
{
	A7_REG( SYS_RESET_CONTROL_REG ) = (1L<<SYS_RESET_BIT);
}

