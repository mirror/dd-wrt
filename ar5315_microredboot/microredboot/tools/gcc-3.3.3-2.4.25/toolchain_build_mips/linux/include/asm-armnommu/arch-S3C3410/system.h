/*
 * uclinux/include/asm-armnommu/arch-S3C3410/system.h
 *
 * (just a placeholder, not needed in our machine)
 *
 * 2003 Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
 *
 */

#ifndef __ASM_ARCH_SYSTEM_H_
#define __ASM_ARCH_SYSTEM_H_

static inline void arch_idle(void)
{
	while (!current->need_resched && !hlt_counter)
		cpu_do_idle(IDLE_WAIT_SLOW);
}

extern inline void arch_reset(char mode)
{
	/* @todo: reset the hardware in some way */
}

#endif /* __ASM_ARCH_SYSTEM_H_ */
