/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2003 Atheros Communications, Inc.,  All Rights Reserved.
 * Copyright (C) 2006 FON Technology, SL.
 * Copyright (C) 2006 Imre Kaloz <kaloz@openwrt.org>
 * Copyright (C) 2006 Felix Fietkau <nbd@openwrt.org>
 */

/*
 * Interrupt support for AR531X WiSOC.
 */

#include <linux/autoconf.h>
#include <linux/init.h>
#include <linux/kernel_stat.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/pm.h>
#include <linux/delay.h>
#include <linux/reboot.h>

#include <asm/irq.h>
#include <asm/mipsregs.h>
#include <asm/gdb-stub.h>

#include "ar531xlnx.h"
#include <asm/irq_cpu.h>

extern int setup_irq(unsigned int irq, struct irqaction *irqaction);

static void ar531x_misc_intr_enable(unsigned int irq);
static void ar531x_misc_intr_disable(unsigned int irq);


/* Turn on the specified AR531X_MISC_IRQ interrupt */
static unsigned int
ar531x_misc_intr_startup(unsigned int irq)
{
	ar531x_misc_intr_enable(irq);
	return 0;
}

/* Turn off the specified AR531X_MISC_IRQ interrupt */
static void
ar531x_misc_intr_shutdown(unsigned int irq)
{
	ar531x_misc_intr_disable(irq);
}

/* Enable the specified AR531X_MISC_IRQ interrupt */
static void
ar531x_misc_intr_enable(unsigned int irq)
{
	unsigned int imr;

	imr = sysRegRead(AR5315_IMR);
	switch(irq)
	{
	   case AR531X_MISC_IRQ_TIMER:
	     imr |= IMR_TIMER;
	     break;

	   case AR531X_MISC_IRQ_AHB_PROC:
	     imr |= IMR_AHB;
	     break;

	   case AR531X_MISC_IRQ_AHB_DMA:
	     imr |= 0/* ?? */;
	     break;

	   case	AR531X_MISC_IRQ_GPIO:
	     imr |= IMR_GPIO;
	     break;

	   case AR531X_MISC_IRQ_UART0:
	     imr |= IMR_UART0;
	     break;


	   case	AR531X_MISC_IRQ_WATCHDOG:
	     imr |= IMR_WD;
	     break;

	   case AR531X_MISC_IRQ_LOCAL:
	     imr |= 0/* ?? */;
	     break;

	}
	sysRegWrite(AR5315_IMR, imr);
	imr=sysRegRead(AR5315_IMR); /* flush write buffer */
	//printk("enable Interrupt irq 0x%x imr 0x%x \n",irq,imr);

}

/* Disable the specified AR531X_MISC_IRQ interrupt */
static void
ar531x_misc_intr_disable(unsigned int irq)
{
	unsigned int imr;

	imr = sysRegRead(AR5315_IMR);
	switch(irq)
	{
	   case AR531X_MISC_IRQ_TIMER:
	     imr &= (~IMR_TIMER);
	     break;

	   case AR531X_MISC_IRQ_AHB_PROC:
	     imr &= (~IMR_AHB);
	     break;

	   case AR531X_MISC_IRQ_AHB_DMA:
	     imr &= 0/* ?? */;
	     break;

	   case	AR531X_MISC_IRQ_GPIO:
	     imr &= ~IMR_GPIO;
	     break;

	   case AR531X_MISC_IRQ_UART0:
	     imr &= (~IMR_UART0);
	     break;

	   case	AR531X_MISC_IRQ_WATCHDOG:
	     imr &= (~IMR_WD);
	     break;

	   case AR531X_MISC_IRQ_LOCAL:
	     imr &= ~0/* ?? */;
	     break;

	}
	sysRegWrite(AR5315_IMR, imr);
	sysRegRead(AR5315_IMR); /* flush write buffer */
}

static void
ar531x_misc_intr_ack(unsigned int irq)
{
	ar531x_misc_intr_disable(irq);
}

static void
ar531x_misc_intr_end(unsigned int irq)
{
	if (!(irq_desc[irq].status & (IRQ_DISABLED | IRQ_INPROGRESS)))
		ar531x_misc_intr_enable(irq);
}

struct irq_chip ar531x_misc_intr_controller = {
	.typename	= "AR531X MISC",
	.startup	= ar531x_misc_intr_startup,
	.shutdown	= ar531x_misc_intr_shutdown,
	.enable		= ar531x_misc_intr_enable,
	.disable	= ar531x_misc_intr_disable,
	.ack		= ar531x_misc_intr_ack,
	.end		= ar531x_misc_intr_end,
};

/*
 * Determine interrupt source among interrupts that use IP6
 */
void
ar531x_misc_intr_init(int irq_base)
{
	int i;

	for (i = irq_base; i < irq_base + AR531X_MISC_IRQ_COUNT; i++) {
		irq_desc[i].status = IRQ_DISABLED;
		irq_desc[i].action = NULL;
		irq_desc[i].depth = 1;
		irq_desc[i].chip = &ar531x_misc_intr_controller;
	}
}

/* ARGSUSED */
irqreturn_t
spurious_irq_handler(int cpl, void *dev_id)
{
    /* 
    printk("spurious_irq_handler: %d  cause=0x%8.8x  status=0x%8.8x\n",
	   cpl, cause_intrs, status_intrs); 
    */
	return IRQ_NONE;
}

/* ARGSUSED */
irqreturn_t
spurious_misc_handler(int cpl, void *dev_id)
{
    /*
    printk("spurious_misc_handler: 0x%x isr=0x%8.8x imr=0x%8.8x\n",
	   cpl, ar531x_isr, ar531x_imr);
    */
	return IRQ_NONE;
}

irqreturn_t
ar531x_ahb_proc_handler(int cpl, void *dev_id)
{
    u32 procAddr = -1;
    u32 proc1 = -1;
    u32 dmaAddr = -1;
    u32 dma1 = -1;
    sysRegWrite(AR5315_AHB_ERR0,AHB_ERROR_DET);
    sysRegRead(AR5315_AHB_ERR1);

    printk("AHB interrupt: PROCADDR=0x%8.8x  PROC1=0x%8.8x  DMAADDR=0x%8.8x  DMA1=0x%8.8x\n",
	procAddr, proc1, dmaAddr, dma1);
	
    machine_restart("AHB error"); /* Catastrophic failure */
    return IRQ_HANDLED;
}

static struct irqaction cascade  = {
	.handler	= no_action,
	.flags		= SA_INTERRUPT,
	.name		= "cascade",
};

static struct irqaction spurious_irq  = {
	.handler	= spurious_irq_handler,
	.flags		= SA_INTERRUPT,
	.name		= "spurious_irq",
};

static struct irqaction spurious_misc  = {
	.handler	= spurious_misc_handler,
	.flags		= SA_INTERRUPT,
	.name		= "spurious_misc",
};

static struct irqaction ar531x_ahb_proc_interrupt  = {
	.handler	= ar531x_ahb_proc_handler,
	.flags		= SA_INTERRUPT,
	.name		= "ar531x_ahb_proc_interrupt",
};

/*
 * Called when an interrupt is received, this function
 * determines exactly which interrupt it was, and it
 * invokes the appropriate handler.
 *
 * Implicitly, we also define interrupt priority by
 * choosing which to dispatch first.
 */
asmlinkage void plat_irq_dispatch(void)
{
	int pending = read_c0_status() & read_c0_cause();

	if (pending & CAUSEF_IP3)
		do_IRQ(AR531X_IRQ_WLAN0_INTRS);
	else if (pending & CAUSEF_IP4)
		do_IRQ(AR531X_IRQ_ENET0_INTRS);
	else if (pending & CAUSEF_IP2) {
		unsigned int ar531x_misc_intrs = sysRegRead(AR5315_ISR) & sysRegRead(AR5315_IMR);

	    if (ar531x_misc_intrs & ISR_TIMER)
			do_IRQ(AR531X_MISC_IRQ_TIMER);
		else if (ar531x_misc_intrs & ISR_AHB)
			do_IRQ(AR531X_MISC_IRQ_AHB_PROC);
		else if (ar531x_misc_intrs & ISR_GPIO) {
#if 0
			int i;
			u32 gpioIntPending;
			
			gpioIntPending = sysRegRead(AR5315_GPIO_DI) & gpioIntMask;
			for (i=0; i<AR531X_GPIO_IRQ_COUNT; i++) {
				if (gpioIntPending & (1 << i))
					do_IRQ(AR531X_GPIO_IRQ_BASE+i);
			}
#endif
			sysRegWrite(AR5315_ISR, sysRegRead(AR5315_IMR) | ~ISR_GPIO);
		} else if (ar531x_misc_intrs & ISR_UART0)
			do_IRQ(AR531X_MISC_IRQ_UART0);
		else if (ar531x_misc_intrs & ISR_WD)
			do_IRQ(AR531X_MISC_IRQ_WATCHDOG);
		else
			do_IRQ(AR531X_MISC_IRQ_NONE);
	} else if (pending & CAUSEF_IP7)
		do_IRQ(AR531X_IRQ_CPU_CLOCK);
	else
		do_IRQ(AR531X_IRQ_NONE);
}

void __init arch_init_irq(void)
{
	clear_c0_status(ST0_IM);
	mips_cpu_irq_init(0);

	/* Initialize interrupt controllers */
	ar531x_misc_intr_init(AR531X_MISC_IRQ_BASE);
#if 0
	ar531x_gpio_intr_init(AR531X_GPIO_IRQ_BASE);
#endif
	setup_irq(AR531X_IRQ_MISC_INTRS, &cascade);
	/*
	 * AR531X_IRQ_CPU_CLOCK is setup by ar531x_timer_setup.
	 */

	/* Default "spurious interrupt" handlers */
	setup_irq(AR531X_IRQ_NONE, &spurious_irq);
	setup_irq(AR531X_MISC_IRQ_NONE, &spurious_misc);
	setup_irq(AR531X_MISC_IRQ_AHB_PROC, &ar531x_ahb_proc_interrupt);
	setup_irq(AR531X_MISC_IRQ_GPIO, &cascade);
}
