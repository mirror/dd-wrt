/*
 * time.h 
 *
 * Copyright (C) 2002-2003 Arcturus Networks Inc.
 *                         by Oleksandr Zhadan <www.ArcturusNetworks.com>
 *
 */

#ifndef __ASM_ARCH_TIME_H
#define __ASM_ARCH_TIME_H

#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/hardware.h>
#include <asm/arch/timex.h>

extern struct irqaction timer_irq;

extern unsigned long s3c2500_gettimeoffset(void);
extern void s3c2500_timer_interrupt(int irq, void *dev_id, struct pt_regs *regs);

extern __inline__ void setup_timer (void)
{
    volatile struct s3c2500_timer* tt = (struct s3c2500_timer*) (TIMER_BASE);
    unsigned long v, tmp;

    gettimeoffset     = s3c2500_gettimeoffset;
    timer_irq.handler = s3c2500_timer_interrupt;
    
    /* I/O port is used for timer */
    tmp = *(unsigned int *)IO_Ftn_cont1 | ((0x1 << KERNEL_TIMER) << 22);
    *(unsigned int *)IO_Ftn_cont1 = tmp;

    tt->td[2*KERNEL_TIMER] = (unsigned long)(ARM_CLK/HZ);
    tmp = tt->tmr & ~(0x7 << (3*KERNEL_TIMER));
    tt->tmr = tmp | (0x1 << (3*KERNEL_TIMER));
    setup_arm_irq((KERNEL_TIMER+32), &timer_irq);
    enable_irq ( (KERNEL_TIMER+32) );
}

#endif /* __ASM_ARCH_TIME_H */
