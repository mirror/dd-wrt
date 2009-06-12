/*
 * linux/include/asm-armnommu/arch-swarm/irq.h
 *
 * 18 Sep 2001 - C Hanish Menon [www.hanishkvc.com]
 *
 */

#ifndef __ASM_ARCH_IRQ_H__
#define __ASM_ARCH_IRQ_H__

#include <asm/hardware.h>
#include <asm/io.h>

#define fixup_irq(x) (x)

static __inline__ void irq_init_irq(void) 
{
	outl(0x0, SWARM_INT_MASK); /* Disable all interrupts */
	outl(0x0, SWARM_INT_LEVEL); /* no FIQs only IRQs */
	//printk("DebugKVC: In irq_init_irq \n");
	return;
}

#endif

