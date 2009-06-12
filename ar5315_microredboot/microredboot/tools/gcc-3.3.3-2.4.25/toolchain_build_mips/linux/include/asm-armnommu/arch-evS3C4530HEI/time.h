/*
 * include/asm/arch/time.h
 * OZH, 2001 Oleksandr Zhadan
 */

#ifndef __ASM_ARCH_TIME_H__
#define __ASM_ARCH_TIME_H__

#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/hardware.h>
#include <asm/arch/timex.h>

extern struct irqaction timer_irq;

extern unsigned long s3c4530_gettimeoffset(void);
extern void s3c4530_timer_interrupt(int irq, void *dev_id, struct pt_regs *regs);

extern __inline__ void setup_timer (void)
{
    volatile struct s3c4530_timer* tt = (struct s3c4530_timer*) (TIMER_BASE);
    unsigned long v;

    gettimeoffset     = s3c4530_gettimeoffset;
    timer_irq.handler = s3c4530_timer_interrupt;

#if	(KERNEL_TIMER==0)
	*(unsigned int *)IOPCON0 = 0x40000000;
	tt->tdr0 = (unsigned long)(ARM_CLK/HZ-1);
/*	v = tt->tmr & 0xFFFFFFF8;
	v |= 0x01; */
	tt->tmr = 0x07;
	setup_arm_irq(_TC0, &timer_irq);
	__sti();
/*	enable_irq ( _TC0 ); */
#elif	(KERNEL_TIMER==1)
	*(unsigned int *)IOPCON0 = 0x80000000;
	tt->tdr1 = (unsigned long)(ARM_CLK/HZ - 1);
	/* 0x7a11f;
	v = tt->tmr & 0xFFFFFFC7; 
	v = 0x038; */

	tt->tmr = 0x38;

	setup_arm_irq(_TC1, &timer_irq);
	enable_irq ( _TC1 );
#else
#error Weird -- KERNEL_TIMER does not seem to be defined...
#endif    
}

#endif /* __ASM_ARCH_TIME_H__ */
