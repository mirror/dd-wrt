/*
 * linux/include/asm-arm/arch-swarm/time.h
 *
 * Copyright (C) 1998 Deborah Wallach.
 * Twiddles  (C) 1999 	Hugo Fiennes <hugo@empeg.com>
 * 
 * 2000/03/29 (C) Nicolas Pitre <nico@cam.org>
 *	Rewritten: big cleanup, much simpler, better HZ acuracy.
 *
 * 2001/02/20 (C) RidgeRun, Inc (http://www.ridgerun.com)
 *      Leveraged for the dsc21
 * 09 Sep 2001 - C Hanish Menon [www.hanishkvc.com]
 *   - Leveraged for SWARM
 *
 */
#ifndef __ASM_ARCH_TIME_H__
#define __ASM_ARCH_TIME_H__

#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/hardware.h>

static void timer_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
  int  TimerCnt, IntPending;
	
	printk("DebugKVC: In timer_interrupt \n");
	do_timer(regs);
	do_set_rtc();
	do_profile(regs);

	TimerCnt = inl(SWARM_TIMER_CNT);
	TimerCnt += SWARM_TIMER_SYS_COUNT;
printk("%s(%d): outl(%x,%x)\n", "time.h", __LINE__, TimerCnt, SWARM_TIMER_MATCH0);
	outl(TimerCnt, SWARM_TIMER_MATCH0);
printk("%s(%d): inl(%x)=%x\n", "time.h", __LINE__, SWARM_INT_IRQ_STATUS, inl(SWARM_INT_IRQ_STATUS));
	IntPending = inl(SWARM_INT_IRQ_STATUS);
printk("%s(%d): outl(%x,%x)\n", "time.h", __LINE__, IntPending & ~(0x1 << SWARM_IRQ_TIMER0), SWARM_INT_IRQ_STATUS);
	outl(IntPending & ~(0x1 << SWARM_IRQ_TIMER0), SWARM_INT_IRQ_STATUS); /* clear System timer interrupt */
	printk("DebugKVC: Leaving timer_interrupt \n");
}

/*
 * Set up timer interrupt.
 */
extern __inline__ void setup_timer(void)
{
  int  TimerCnt, IntMask;

	//printk("DebugKVC: In setup_timer \n");
	timer_irq.handler = timer_interrupt;
	setup_arm_irq(IRQ_TIMER, &timer_irq);
	TimerCnt = inl(SWARM_TIMER_CNT);
	TimerCnt += SWARM_TIMER_SYS_COUNT;
	outl(TimerCnt, SWARM_TIMER_MATCH0);
	outl(0x1, SWARM_TIMER_INTEN); /* enable interrupt mode */
	IntMask = inl(SWARM_INT_MASK);
	outl(IntMask | (0x1 << SWARM_IRQ_TIMER0), SWARM_INT_MASK); /* enable System timer interrupt */
}


#endif /* __ASM_ARCH_TIME_H__ */
