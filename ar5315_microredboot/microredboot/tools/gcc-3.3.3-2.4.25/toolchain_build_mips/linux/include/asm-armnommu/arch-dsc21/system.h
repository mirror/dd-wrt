/*
 * linux/include/asm-armnommu/arch-dsc21/system.h
 *
 * Copyright (c) 1999 Nicolas Pitre <nico@cam.org>
 * Copyright (c) 2001 RidgeRun, Inc (http://www.ridgerun.org)
 *
 */

static inline void arch_idle(void)
{
	while (!current->need_resched && !hlt_counter);
}

extern inline void arch_reset(char mode)
{
	/* REVISIT --gmcnutt */
}

