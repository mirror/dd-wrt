/*
 * linux/include/asm-arm/arch-sa1100/time.h
 *
 * Copyright (C) 1998 Deborah Wallach.
 * Twiddles  (C) 1999 	Hugo Fiennes <hugo@empeg.com>
 * 
 * 2000/03/29 (C) Nicolas Pitre <nico@cam.org>
 *	Rewritten: big cleanup, much simpler, better HZ acuracy.
 *
 * 2001/02/20 (C) RidgeRun, Inc (http://www.ridgerun.com)
 *      Leveraged for the dsc21
 */
#ifndef __ASM_ARCH_TIME_H__
#define __ASM_ARCH_TIME_H__

#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/hardware.h>

extern struct irqaction timer_irq;

extern unsigned long dsc21_gettimeoffset(void);
extern void dsc21_timer_interrupt(int irq, void *dev_id, struct pt_regs *regs);

void __inline__ setup_timer (void)
{
	/* disable */
	outw(0, DSC21_TIMER0_MODE);
	outw(0, DSC21_TIMER1_MODE);
	outw(0, DSC21_TIMER2_MODE);
	outw(0, DSC21_TIMER3_MODE);

	/*
	 * System clock formula:
	 *         freq = clock / (div * scale)
	 * Where:
	 *         clock = 27MHz
	 *         div   = 27,000
	 *         scale = 10
	 *
	 * Which gives us 100Hz, as desired.
	 */

	/* 27MHz clock (not ARM clock) */
	outw(1, DSC21_TIMER0_SEL);
	
	/* prescale 10 */
	outw(9, DSC21_TIMER0_SCAL);
	
	/* div 27000 */
	outw(26999, DSC21_TIMER0_DIV);
	
	/* periodic */
	outw(2, DSC21_TIMER0_MODE);


	gettimeoffset = dsc21_gettimeoffset;	
	timer_irq.handler = dsc21_timer_interrupt;
	setup_arm_irq(IRQ_TIMER, &timer_irq);
    
}


#endif /* __ASM_ARCH_TIME_H__ */
