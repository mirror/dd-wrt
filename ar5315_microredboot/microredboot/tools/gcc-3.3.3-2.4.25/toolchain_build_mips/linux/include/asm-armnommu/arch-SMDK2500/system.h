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
	if (mode == 's') {
		/* Jump into ROM at address 0 */
		cpu_reset(0x80000000);
	} /* tbd */
	printk(KERN_ERR "Reboot mode '%c' not implemented\n", mode);
}

 
