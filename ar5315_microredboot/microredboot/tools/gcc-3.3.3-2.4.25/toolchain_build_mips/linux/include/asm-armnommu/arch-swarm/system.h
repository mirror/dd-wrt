/*
 * system.h 
 * 
 * 9 Sep 2001 - C Hanish Menon [www.hanishkvc.com]
 *
 */ 

static inline void arch_idle(void)
{
	while (!current->need_resched && !hlt_counter);
}

extern __inline__ void arch_reset(char mode)
{
	return;
}
