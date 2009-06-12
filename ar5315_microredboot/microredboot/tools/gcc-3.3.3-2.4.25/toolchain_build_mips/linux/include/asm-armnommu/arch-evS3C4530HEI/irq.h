/*
 * include/asm/arch/irq.h
 * OZH, 2001 Oleksandr Zhadan
 */

#ifndef __ASM_ARCH_IRQ_H__
#define __ASM_ARCH_IRQ_H__


#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/mach/irq.h>
#include <asm/arch/irqs.h>

// #define fixup_irq(x) (x)

extern void s3c4530_mask_irq(unsigned int irq);
extern void s3c4530_unmask_irq(unsigned int irq);
extern void s3c4530_mask_ack_irq(unsigned int irq);
extern void s3c4530_clear_pb(unsigned int irq);


static __inline__ unsigned int fixup_irq (unsigned int irq)
{
s3c4530_clear_pb(irq);
return (irq);
}

static __inline__ void irq_init_irq(void)
{
    volatile unsigned char *PtIntPr = (volatile unsigned char *)INTPRI0;
    int irq;
/* Current IRQ number by priority */
    static unsigned char s3c4530_irq_prtable[32] = {
    _IRQ0,			/* Low */
    _IRQ1,
    _IRQ2,
    _IRQ3,
    _URTTx0,
    _URTRx0,
    _URTTx1,
    _URTRx1,
    _GDMA0,
    _GDMA1,
    _TC0,
    _TC1,
    _HDLCATx,
    _HDLCARx,
    _HDLCBTx,
    _HDLCBRx,
    _BDMATx,
    _BDMARx,
    _MACTx,
    _MACRx,
    _I2C,			/* High */
    0,0,0,0,0,0,0,0,0,0,0
    };
    
    
    for (irq = 0; irq < NR_IRQS; irq++) {
	irq_desc[irq].valid	= 1;
	irq_desc[irq].probe_ok	= 1;
	irq_desc[irq].mask_ack	= s3c4530_mask_ack_irq;
	irq_desc[irq].mask	= s3c4530_mask_irq;
	irq_desc[irq].unmask	= s3c4530_unmask_irq;
	
	*PtIntPr++ = s3c4530_irq_prtable[irq]; 		/* Set up priority 	*/
	}	
    outl ( 0,        INTMOD );		/* Set all interrupts as IRQ mode	*/
    outl ( 0x1FFFFF, INTPND );		/* Clear Intrerrupt pending register	*/
    outl ( 0x1FFFFF, INTMSK );		/* Disable all Intrerrupts		*/
}


#endif /* __ASM_ARCH_IRQ_H__ */
