/*
 * linux/include/asm-armnommu/arch-c5471/system.h
 */

static inline void arch_idle(void)
{
  while (!current->need_resched && !hlt_counter);
}

extern inline void arch_reset(char mode)
{
}

