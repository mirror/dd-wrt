/*
 * asm/arch-samsung/irq.h:
 * 2001 Mac Wang <mac@os.nctu.edu.tw>
 */

#ifndef __ASM_ARCH_IRQ_H__
#define __ASM_ARCH_IRQ_H__

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/mach/irq.h>
#include <asm/arch/irqs.h>

#define fixup_irq(x) (x)

extern void s3c4510b_mask_irq(unsigned int irq);
extern void s3c4510b_unmask_irq(unsigned int irq);
extern void s3c4510b_mask_ack_irq(unsigned int irq);
extern void s3c4510b_int_init(void);

static __inline__ void irq_init_irq(void)
{
	unsigned long flags;
	int irq;

	save_flags_cli(flags);
	s3c4510b_int_init();
	restore_flags(flags);

	for (irq = 0; irq < NR_IRQS; irq++) {
		irq_desc[irq].valid	= 1;
		irq_desc[irq].probe_ok	= 1;
		irq_desc[irq].mask_ack	= s3c4510b_mask_ack_irq;
		irq_desc[irq].mask	= s3c4510b_mask_irq;
		irq_desc[irq].unmask	= s3c4510b_unmask_irq;
	}
}
#endif /* __ASM_ARCH_IRQ_H__ */
