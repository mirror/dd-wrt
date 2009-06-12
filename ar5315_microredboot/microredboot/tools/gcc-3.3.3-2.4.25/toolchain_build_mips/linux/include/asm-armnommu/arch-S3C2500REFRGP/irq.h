/*
 * irq.h 
 *
 * Copyright (C) 2002-2003 Arcturus Networks Inc.
 *                         by Oleksandr Zhadan <www.ArcturusNetworks.com>
 *
 */
#ifndef __ASM_ARCH_IRQ_H
#define __ASM_ARCH_IRQ_H

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/mach/irq.h>
#include <asm/arch/irqs.h>

#define fixup_irq(x) (x)

extern void s3c2500_mask_irq(unsigned int irq);
extern void s3c2500_unmask_irq(unsigned int irq);
extern void s3c2500_mask_ack_irq(unsigned int irq);
extern void s3c2500_mask_global(void);
extern void s3c2500_unmask_global(void);

static __inline__ void irq_init_irq(void)
{
/******************************************
 * Interrupt priority table
 ******************************************/
static unsigned char s3c2500_irq_prtable[NR_IRQS] =
	{
	SRC_IRQ_0,
	SRC_IRQ_1,
	SRC_IRQ_2,
 	SRC_IRQ_3,
 	SRC_IRQ_4,
 	SRC_IRQ_5,
 	SRC_IRQ_IOM2,
 	SRC_IRQ_IICC,
 	SRC_IRQ_HUART0_TX,
 	SRC_IRQ_HUART0_RX,
 	SRC_IRQ_HUART1_TX,
 	SRC_IRQ_HUART1_RX,
 	SRC_IRQ_CUART_TX,
 	SRC_IRQ_CUART_RX,
 	SRC_IRQ_USB,
 	SRC_IRQ_HDLC_TX0,
 	SRC_IRQ_HDLC_RX0,
 	SRC_IRQ_HDLC_TX1,
 	SRC_IRQ_HDLC_RX1,
 	SRC_IRQ_HDLC_TX2,
 	SRC_IRQ_HDLC_RX2,
 	SRC_IRQ_ETH_TX0,
 	SRC_IRQ_ETH_RX0,
 	SRC_IRQ_ETH_TX1,
 	SRC_IRQ_ETH_RX1,
 	SRC_IRQ_DES,
 	SRC_IRQ_GDMA0,
 	SRC_IRQ_GDMA1,
 	SRC_IRQ_GDMA2,
 	SRC_IRQ_GDMA3,
 	SRC_IRQ_GDMA4,
 	SRC_IRQ_GDMA5,
 	SRC_IRQ_TIMER0,
 	SRC_IRQ_TIMER1,
 	SRC_IRQ_TIMER2,
 	SRC_IRQ_TIMER3,
 	SRC_IRQ_TIMER4,
 	SRC_IRQ_TIMER5,
 	SRC_IRQ_TIMER_WD
	};
    volatile unsigned int *PtIntPr = (volatile unsigned int *)INTPRIOR0;
    unsigned int irq;
    unsigned int *tmp = (unsigned int *)s3c2500_irq_prtable;
    
    for (irq = 0; irq < NR_IRQS; irq++) {
	irq_desc[irq].valid	= 1;
	irq_desc[irq].probe_ok	= 1;
	irq_desc[irq].mask_ack	= s3c2500_mask_ack_irq;
	irq_desc[irq].mask	= s3c2500_mask_irq;
	irq_desc[irq].unmask	= s3c2500_unmask_irq;
	}
    for (irq = 0; irq < NR_IRQS; irq+=4) {
	*PtIntPr++ = *tmp++;
	}
    
    outl ( 0,          INTMOD );	/* Set internal interrupts in IRQ mode	*/
    outl ( 0,          EXTMOD );	/* Set external interrupts in IRQ mode	*/
    s3c2500_mask_global();		/* Disable all interrupt requests	*/
    outl ( 0xFFFFFFFF, INTMASK );	/* Mask all internal Intrerrupts	*/
    outl ( 0x0000007F, EXTMASK );	/* Mask all external Intrerrupts 	*/
}

#endif /* __ASM_ARCH_IRQ_H */
