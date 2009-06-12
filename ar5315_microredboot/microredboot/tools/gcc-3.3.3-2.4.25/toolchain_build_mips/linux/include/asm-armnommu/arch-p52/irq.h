/*
 * asm/arch-p52/irq.h:
 * 2001 Mindspeed
 */

#ifndef __ASM_ARCH_IRQ_H__
#define __ASM_ARCH_IRQ_H__


#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/mach/irq.h>
#include <asm/arch/irqs.h>


#define fixup_irq(x) (x)

extern void p52_mask_irq(unsigned int irq);
extern void p52_unmask_irq(unsigned int irq);
extern void p52_mask_ack_irq(unsigned int irq);

extern struct irqdesc irq_desc[];

static __inline__ void irq_init_irq(void)
{
	int irq;

	/* disable all int sources */
	(*(unsigned long*)P52INT_MASK)=0x00000000;

	for (irq = 0; irq < NR_IRQS; irq++) {
		irq_desc[irq].valid	= 1;
		irq_desc[irq].probe_ok	= 1;
		irq_desc[irq].mask_ack	= p52_mask_ack_irq;
		irq_desc[irq].mask	= p52_mask_irq;
		irq_desc[irq].unmask	= p52_unmask_irq;
	}
}

#endif /* __ASM_ARCH_IRQ_H__ */
