/*
 * linux/include/asm-armnommu/arch-ta7s/irq.h
 *
 * Copyright (C) 2000 Triscend Corporation
 *
 * Modifications:
 *  05-Nov-200	GS	Created.
 */

#include	<asm/arch/hardware.h>
#include	<asm/arch/irqs.h>

#define fixup_irq(x) (x)

#define BUILD_IRQ(s,n,m)

extern void IRQ_interrupt(void);
extern void timer_IRQ_interrupt(void);
extern void fast_IRQ_interrupt(void);
extern void bad_IRQ_interrupt(void);
extern void probe_IRQ_interrupt(void);

#define IRQ_interrupt0 IRQ_interrupt
#define IRQ_interrupt1 IRQ_interrupt
#define IRQ_interrupt2 IRQ_interrupt
#define IRQ_interrupt3 timer_IRQ_interrupt
#define IRQ_interrupt4 IRQ_interrupt
#define IRQ_interrupt5 IRQ_interrupt
#define IRQ_interrupt6 IRQ_interrupt
#define IRQ_interrupt7 IRQ_interrupt
#define IRQ_interrupt8 IRQ_interrupt
#define IRQ_interrupt9 IRQ_interrupt
#define IRQ_interrupt10 IRQ_interrupt
#define IRQ_interrupt11 IRQ_interrupt
#define IRQ_interrupt12 IRQ_interrupt
#define IRQ_interrupt13 IRQ_interrupt
#define IRQ_interrupt14 IRQ_interrupt
#define IRQ_interrupt15 IRQ_interrupt

#define IRQ_INTERRUPT(n)	IRQ_interrupt##n
#define FAST_INTERRUPT(n)	fast_IRQ_interrupt
#define BAD_INTERRUPT(n)	bad_IRQ_interrupt
#define PROBE_INTERRUPT(n)	probe_IRQ_interrupt

static __inline__ void mask_irq(unsigned int irq)
{
	volatile unsigned long int *mask = 
		(volatile unsigned long *)(INT_IRQ_ENABLE_CLEAR_REG);
	
	if (irq < NR_IRQS) *mask = ( 1 << irq );
}

static __inline__ void unmask_irq(unsigned int irq)
{
	volatile unsigned long int *set = 
		(volatile unsigned long *)(INT_IRQ_ENABLE_REG);
	
	if (irq < NR_IRQS) *set = ( 1 << irq );
}
 
static __inline__ unsigned long get_enabled_irqs(void)
{
	volatile unsigned long int *ie = 
		(volatile unsigned long *)(INT_IRQ_ENABLE_REG);
	
	return *ie ;
}

static __inline__ void mask_ack_irq(unsigned int irq)
{
	mask_irq(irq);
}

static __inline__ void irq_init_irq(void)
{
	unsigned long flags;
	int irq;
	volatile unsigned long int *mask = 
		(volatile unsigned long *)(INT_IRQ_ENABLE_CLEAR_REG);

	save_flags_cli (flags);	/* clear all interrupt enables */
	*mask = 0x0ffffffe;

    	for (irq = 0; irq < NR_IRQS; irq++) {
		irq_desc[irq].valid	= 1;
		irq_desc[irq].probe_ok	= 1;
		irq_desc[irq].mask_ack	= mask_ack_irq;
		irq_desc[irq].mask	= mask_irq;
		irq_desc[irq].unmask	= unmask_irq;
	}
    
	restore_flags (flags);
}
