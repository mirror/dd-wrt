/*
 * Copyright 2004 PMC-Sierra Inc.
 * Author: Manish Lachwani (lachwani@pmc-sierra.com)
 *
 * arch/mips/pmc-sierra/big_sur/big_sur_irq.c
 *     Interrupt routines for Xilinx system controller.  Interrupt numbers 
 *     are assigned from BIG_SUR_IRQ_BASE to BIG_SUR_IRQ_BASE + 10
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <asm/ptrace.h>
#include <linux/config.h>
#include <linux/sched.h>
#include <linux/kernel_stat.h>
#include <asm/io.h>
#include <asm/irq.h>

#include "xilinx_irq.h"

#define	BIG_SUR_IRQ_BASE	16

/* mask off an interrupt -- 1 is enable, 0 is disable */
static inline void mask_big_sur_irq(unsigned int irq)
{
	unsigned long reg_data;
	
	reg_data = BIG_SUR_READ(BIG_SUR_INTERRUPT_MASK_1);
	reg_data &= ~(1 << (irq - BIG_SUR_IRQ_BASE));
	BIG_SUR_WRITE(BIG_SUR_INTERRUPT_MASK_1, reg_data);
}

/* unmask an interrupt -- 1 is enable, 0 is disable */
static inline void unmask_big_sur_irq(unsigned int irq)
{
	unsigned long reg_data;

	reg_data = BIG_SUR_READ(BIG_SUR_INTERRUPT_MASK_1);
	reg_data |= (1 << (irq - BIG_SUR_IRQ_BASE));
	BIG_SUR_WRITE(BIG_SUR_INTERRUPT_MASK_1, reg_data);
}

/* Enable IRQ on the Xilinx FPGA */
static void enable_big_sur_irq(unsigned int irq)
{
	unmask_big_sur_irq(irq);
}

/* Initialize IRQ on the Xilinx FPGA */
static unsigned int startup_big_sur_irq(unsigned int irq)
{
	unmask_big_sur_irq(irq);
	return 0;
}

/* Disable the IRQ on the Xilinx FPGA */
static void disable_big_sur_irq(unsigned int irq)
{
	mask_big_sur_irq(irq);
}

/* Mask and ack an IRQ on the Xilinx FPGA */
static void mask_and_ack_big_sur_irq(unsigned int irq)
{
	mask_big_sur_irq(irq);
}

/* End IRQ processing on the Xilinx FPGA */
static void end_big_sur_irq(unsigned int irq)
{
	if (!(irq_desc[irq].status & (IRQ_DISABLED|IRQ_INPROGRESS)))
		unmask_big_sur_irq(irq);
}

/*
 * Main interrupt handler for the Xilinx FPGA on the 
 * Big Sur board. These interrupts could be coming
 * from the IDE, PCI, UART etc.
 */
void big_sur_irq_handler(struct pt_regs *regs)
{
	unsigned long reg_data;

	reg_data = BIG_SUR_READ(BIG_SUR_INTERRUPT_STATUS_1);
	
	/* Now check for the UART 1 interrupts */
	if (reg_data & 0x38)
		do_IRQ(BIG_SUR_UART1_IRQ + BIG_SUR_IRQ_BASE, regs);

	/* Now check for the UART 2 interrupts */
	if (reg_data & 0x1c0)
		do_IRQ(BIG_SUR_UART2_IRQ + BIG_SUR_IRQ_BASE, regs);

	/* Now check for the Timer interrupt */
	if (reg_data & 0x800)
		do_IRQ(BIG_SUR_TIMER_IRQ + BIG_SUR_IRQ_BASE, regs);

	/* Now check for the PCI interrupt, INTA */
	if (reg_data & 0x2)
		do_IRQ(BIG_SUR_PCI_IRQ + BIG_SUR_IRQ_BASE, regs);

	/* Now check for the IDE interrupts */
	if (reg_data & 0x1)
		do_IRQ(BIG_SUR_IDE_IRQ + BIG_SUR_IRQ_BASE, regs);
}

#define	shutdown_big_sur_irq	disable_big_sur_irq

struct hw_interrupt_type big_sur_irq_type = {
	"BIG-SUR",
	startup_big_sur_irq,
	shutdown_big_sur_irq,
	enable_big_sur_irq,
	disable_big_sur_irq,
	mask_and_ack_big_sur_irq,
	end_big_sur_irq,
	NULL
};

void big_sur_irq_init(void)
{
	int i;

	/* Reset irq handlers pointers to NULL */
	for (i = BIG_SUR_IRQ_BASE; i < (BIG_SUR_IRQ_BASE + 10); i++) {
		irq_desc[i].status = IRQ_DISABLED;
		irq_desc[i].action = 0;
		irq_desc[i].depth = 2;
		irq_desc[i].handler = &big_sur_irq_type;
	}
}

