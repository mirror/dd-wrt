/*
 * linux/include/asm-arm/arch-p52/time.h
 * 2001 Mindspeed
 */

#ifndef __ASM_ARCH_TIME_H__
#define __ASM_ARCH_TIME_H__

#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/hardware.h>
#include <asm/arch/timex.h>

extern struct irqaction timer_irq;

extern unsigned long p52_gettimeoffset(void);
extern void p52_timer_interrupt(int irq, void *dev_id, struct pt_regs *regs);


void __inline__ setup_timer (void)
{

	/* disable timer interrupt (should already been disabled) */
	(*(volatile unsigned long*)P52INT_MASK) &=~(1 << P52INT_LVL_TIMER_1);

	/* Timeout 10ms */
	TIM_SET_RATE(TM_Lmt1,CLOCK_TICK_RATE);	

	//gettimeoffset = p52_gettimeoffset;	
	timer_irq.handler = p52_timer_interrupt;
	setup_arm_irq(IRQ_TIMER, &timer_irq);

}

#endif /* __ASM_ARCH_TIME_H__ */
