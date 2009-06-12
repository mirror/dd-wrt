/*
 * linux/include/asm/arch-samsung/time.h
 * 2001 Mac Wang <mac@os.nctu.edu.tw> 
 */

#ifndef __ASM_ARCH_TIME_H__
#define __ASM_ARCH_TIME_H__

#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/hardware.h>
#include <asm/arch/timex.h>

extern struct irqaction timer_irq;

extern unsigned long samsung_gettimeoffset(void);
extern void samsung_timer_interrupt(int irq, void *dev_id, struct pt_regs *regs);


void __inline__ setup_timer (void)
{
	CSR_WRITE(TCNT0, 0);
	CSR_WRITE(TDATA0, CLOCK_TICK_RATE);
	CSR_WRITE(TMOD, TMOD_TIMER0_VAL);

	CLEAR_PEND_INT(IRQ_TIMER);
	INT_ENABLE(IRQ_TIMER);
	
	timer_irq.handler = samsung_timer_interrupt;
	setup_arm_irq(IRQ_TIMER, &timer_irq);
}

#endif /* __ASM_ARCH_TIME_H__ */
