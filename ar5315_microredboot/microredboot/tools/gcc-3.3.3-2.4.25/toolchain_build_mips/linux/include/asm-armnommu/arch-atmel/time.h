/*
 * linux/include/asm-armnommu/arch-atmel/time.h
 *
 * Copyright (C) 2001/02 Erwin Authried <eauth@softsys.co.at>
 */

#if (KERNEL_TIMER==0)
#   define KERNEL_TIMER_IRQ_NUM IRQ_TC0
#elif (KERNEL_TIMER==1)
#   define KERNEL_TIMER_IRQ_NUM IRQ_TC1
#elif (KERNEL_TIMER==2)
#   define KERNEL_TIMER_IRQ_NUM IRQ_TC2
#else
#error Wierd -- KERNEL_TIMER is not defined or something....
#endif

static unsigned long atmel_gettimeoffset (void)
{
	volatile struct at91_timers* tt = (struct at91_timers*) (AT91_TC_BASE);
	volatile struct at91_timer_channel* tc = &tt->chans[KERNEL_TIMER].ch;
	return tc->cv * (1000*1000)/(ARM_CLK/128);
}

static void atmel_timer_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	volatile struct at91_timers* tt = (struct at91_timers*) (AT91_TC_BASE);
	volatile struct at91_timer_channel* tc = &tt->chans[KERNEL_TIMER].ch;
	unsigned long v = tc->sr;
        do_timer(regs);
        do_profile(regs);
}

extern inline void setup_timer (void)
{
        register volatile struct at91_timers* tt = (struct at91_timers*) (AT91_TC_BASE);
        register volatile struct at91_timer_channel* tc = &tt->chans[KERNEL_TIMER].ch;
        unsigned long v;

	/* enable Kernel timer */
	HW_AT91_TIMER_INIT(KERNEL_TIMER)

        /* No SYNC */
        tt->bcr = 0;
        /* program NO signal on XC1 */
        v = tt->bmr;
	v &= ~TCNXCNS(KERNEL_TIMER,3);
	v |= TCNXCNS(KERNEL_TIMER,1);
        tt->bmr = v;

        tc->ccr = 2;  /* disable the channel */

        /* select ACLK/128 as inupt frequency for TC1 and enable CPCTRG */
        tc->cmr = 3 | (1 << 14);

        tc->idr = ~0ul;  /* disable all interrupt */
        tc->rc = ((ARM_CLK/128)/HZ - 1);   /* load the count limit into the CR register */
        tc->ier = TC_CPCS;  /* enable CPCS interrupt */

        /* enable the channel */
        tc->ccr = TC_SWTRG|TC_CLKEN;

        gettimeoffset = atmel_gettimeoffset;
        timer_irq.handler = atmel_timer_interrupt;

        setup_arm_irq(KERNEL_TIMER_IRQ_NUM, &timer_irq);
}
