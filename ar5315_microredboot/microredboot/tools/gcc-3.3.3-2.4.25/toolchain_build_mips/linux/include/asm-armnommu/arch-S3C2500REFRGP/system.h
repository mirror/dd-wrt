/*
 * linux/include/asm-armnommu/arch-p52/system.h
 * 2001 Mindspeed
 */

static inline void arch_idle(void)
{
	while (!current->need_resched && !hlt_counter)
	  cpu_do_idle(IDLE_WAIT_SLOW);
}

extern inline void arch_reset(char mode)
{
  /* tbd */
}

 
