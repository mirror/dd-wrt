/*
 * asm/arch-atmel/irq.h:
 * 2001 Erwin Authried
 */

#ifndef __ASM_ARCH_IRQ_H__
#define __ASM_ARCH_IRQ_H__


#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/mach/irq.h>
#include <asm/arch/irqs.h>


#define fixup_irq(x) (x)

extern void at91_mask_irq(unsigned int irq);
extern void at91_unmask_irq(unsigned int irq);
extern void at91_mask_ack_irq(unsigned int irq);
extern void at91_init_aic(void);

static __inline__ void irq_init_irq(void)
{
	int irq;

	at91_init_aic();

	for (irq = 0; irq < NR_IRQS; irq++) {

	        if (!VALID_IRQ(irq)) continue;

		irq_desc[irq].valid	= 1;
		irq_desc[irq].probe_ok	= 1;
		irq_desc[irq].mask_ack	= at91_mask_ack_irq;
		irq_desc[irq].mask	= at91_mask_irq;
		irq_desc[irq].unmask	= at91_unmask_irq;
	}
}



#endif /* __ASM_ARCH_IRQ_H__ */
