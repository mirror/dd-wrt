/*
 * uclinux/include/asm-armnommu/arch-S3C3410/irq.h
 *
 * 2003 Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
 *
 */

#ifndef __ASM_ARCH_IRQ_H__
#define __ASM_ARCH_IRQ_H__

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/mach/irq.h>
#include <asm/arch/irqs.h>

extern void s3c3410_mask_irq(unsigned int irq);
extern void s3c3410_unmask_irq(unsigned int irq);
extern void s3c3410_mask_ack_irq(unsigned int irq);
extern void s3c3410_clear_pb(unsigned int irq);

static __inline__ unsigned int fixup_irq (unsigned int irq)
{
	s3c3410_clear_pb(irq);
	return (irq);
}

static __inline__ void irq_init_irq(void)
{
	int irq;
	for (irq = 0; irq < NR_IRQS; irq++) {
		irq_desc[irq].valid	= 1;
		irq_desc[irq].probe_ok	= 1;
		irq_desc[irq].mask_ack	= s3c3410_mask_ack_irq;
		irq_desc[irq].mask	= s3c3410_mask_irq;
		irq_desc[irq].unmask	= s3c3410_unmask_irq;

		/* @TODO initialize interrupt priorities */
	}


	/* mask and disable all further interrupts */
	outl(0x00000000, S3C3410X_INTMSK);

	/* set all to IRQ mode, not FIQ */
	outl(0x00000000, S3C3410X_INTMOD);

	/* Clear Intrerrupt pending register	*/
	outl(0x00000000, S3C3410X_INTPND);

	/*
	 * enable the gloabal interrupt flag, this should be
	 * safe now, all sources are masked out and acknowledged
	 */
	outb(inb(S3C3410X_SYSCON) | S3C3410X_SYSCON_GIE, S3C3410X_SYSCON);
	
}


#endif /* __ASM_ARCH_IRQ_H__ */
