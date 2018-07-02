/*
 *  AR71xx SoC routines
 *
 *  Copyright (C) 2007 Atheros 
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */
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
//#include <asm/gdb-stub.h>

#include "ar7100.h"
#include <asm/irq_cpu.h>

/*
 * dummy irqaction, so that interrupt controller cascading can work. Basically
 * when one IC is connected to another, this will be used to enable to Parent
 * IC's irq line to which the child IC is connected
 */
static struct irqaction cascade = {
	.handler = no_action,
	.name = "cascade",
};

static void ar7100_dispatch_misc_intr(void);
static void ar7100_dispatch_pci_intr(void);
static void ar7100_dispatch_gpio_intr(void);
static void ar7100_misc_irq_init(int irq_base);
extern asmlinkage void ar7100_interrupt_receive(void);

void __init arch_init_irq(void)
{
	/*
	 * initialize our interrupt controllers
	 */
	//mips_cpu_irq_init(AR7100_CPU_IRQ_BASE);
#if MIPS_CPU_IRQ_BASE
#warning MIPS_CPU_IRQ_BASE is not 0
#endif
	mips_cpu_irq_init();

	ar7100_misc_irq_init(AR7100_MISC_IRQ_BASE);
	cp0_perfcount_irq = AR7100_MISC_IRQ_PERF_COUNTER;
//      ar7100_gpio_irq_init(AR7100_GPIO_IRQ_BASE);
#ifdef CONFIG_PCI
	ar7100_pci_irq_init(AR7100_PCI_IRQ_BASE);
#endif

	/*
	 * enable cascades
	 */
	setup_irq(AR7100_CPU_IRQ_MISC, &cascade);
	setup_irq(AR7100_MISC_IRQ_GPIO, &cascade);
#ifdef CONFIG_PCI
	setup_irq(AR7100_CPU_IRQ_PCI, &cascade);
#endif
	printk(KERN_EMERG "irq init done\n");
//      set_c0_status(ST0_IM);
}

static void ar7100_dispatch_misc_intr(void)
{
	int pending;

	pending = ar7100_reg_rd(AR7100_MISC_INT_STATUS) & ar7100_reg_rd(AR7100_MISC_INT_MASK);
	if (pending & MIMR_UART) {
//              printk(KERN_EMERG "dispatch uart\n");
		do_IRQ(AR7100_MISC_IRQ_UART);
	} else if (pending & MIMR_DMA) {
//              printk(KERN_EMERG "dispatch dma\n");

		do_IRQ(AR7100_MISC_IRQ_DMA);
	}

	else if (pending & MIMR_PERF_COUNTER) {
//              printk(KERN_EMERG "dispatch perf counter\n");

		do_IRQ(AR7100_MISC_IRQ_PERF_COUNTER);
	}

	else if (pending & MIMR_TIMER) {
//              printk(KERN_EMERG "dispatch timer\n");
		do_IRQ(AR7100_MISC_IRQ_TIMER);
	}

	else if (pending & MIMR_OHCI_USB) {
//              printk(KERN_EMERG "dispatch ohci\n");
		do_IRQ(AR7100_MISC_IRQ_USB_OHCI);
	}

	else if (pending & MIMR_ERROR) {
//              printk(KERN_EMERG "dispatch error\n");
		do_IRQ(AR7100_MISC_IRQ_ERROR);
	}

	else if (pending & MIMR_GPIO) {
//              printk(KERN_EMERG "dispatch gpio\n");
		ar7100_dispatch_gpio_intr();
	}

	else if (pending & MIMR_WATCHDOG) {
//              printk(KERN_EMERG "dispatch watchdog\n");
		do_IRQ(AR7100_MISC_IRQ_WATCHDOG);
	} else
		spurious_interrupt();
}

#ifndef CONFIG_AR9100
static void ar7100_dispatch_pci_intr(void)
{
	int pending;

	pending = ar7100_reg_rd(AR7100_PCI_INT_STATUS) & ar7100_reg_rd(AR7100_PCI_INT_MASK);

	if (pending & PISR_DEV0)
		do_IRQ(AR7100_PCI_IRQ_DEV0);

	else if (pending & PISR_DEV1)
		do_IRQ(AR7100_PCI_IRQ_DEV1);

	else if (pending & PISR_DEV2)
		do_IRQ(AR7100_PCI_IRQ_DEV2);
}
#endif

static void ar7100_dispatch_gpio_intr(void)
{
	int pending, i;

	pending = ar7100_reg_rd(AR7100_GPIO_INT_PENDING) & ar7100_reg_rd(AR7100_GPIO_INT_MASK);

	for (i = 0; i < AR7100_GPIO_COUNT; i++) {
		if (pending & (1 << i))
			do_IRQ(AR7100_GPIO_IRQn(i));
	}
}

/*
 * Dispatch interrupts. 
 * XXX: This currently does not prioritize except in calling order. Eventually
 * there should perhaps be a static map which defines, the IPs to be masked for
 * a given IP.
 */
asmlinkage void plat_irq_dispatch(void)
{
	int pending = read_c0_status() & read_c0_cause() & ST0_IM;

	if (pending & CAUSEF_IP7)
		do_IRQ(AR7100_CPU_IRQ_TIMER);
#ifndef CONFIG_AR9100
	else if (pending & CAUSEF_IP2)
		ar7100_dispatch_pci_intr();
#else
	else if (pending & CAUSEF_IP2)
		do_IRQ(AR7100_CPU_IRQ_WMAC);
#endif

	else if (pending & CAUSEF_IP4)
		do_IRQ(AR7100_CPU_IRQ_GE0);

	else if (pending & CAUSEF_IP5)
		do_IRQ(AR7100_CPU_IRQ_GE1);

	else if (pending & CAUSEF_IP3)
		do_IRQ(AR7100_CPU_IRQ_USB);

	else if (pending & CAUSEF_IP6)
		ar7100_dispatch_misc_intr();

	/*
	 * Some PCI devices are write to clear. These writes are posted and might
	 * require a flush (r8169.c e.g.). Its unclear what will have more 
	 * performance impact - flush after every interrupt or taking a few
	 * "spurious" interrupts. For now, its the latter.
	 */
	else
		printk("spurious IRQ pending: 0x%x\n", pending);
}

static void ar7100_misc_irq_enable(struct irq_data *irq)
{
	ar7100_reg_rmw_set(AR7100_MISC_INT_MASK, (1 << (irq->irq - AR7100_MISC_IRQ_BASE)));
}

static void ar7100_misc_irq_disable(struct irq_data *irq)
{
	ar7100_reg_rmw_clear(AR7100_MISC_INT_MASK, (1 << (irq->irq - AR7100_MISC_IRQ_BASE)));
}

static struct irq_chip ar7100_misc_irq_chip = {
	.name = "AR7100 MISC",
	.irq_unmask = ar7100_misc_irq_enable,
	.irq_mask = ar7100_misc_irq_disable,
};

/*
 * Determine interrupt source among interrupts that use IP6
 */
static void ar7100_misc_irq_init(int irq_base)
{
	int i;
	ar7100_misc_irq_chip.irq_mask_ack = ar7100_misc_irq_disable;

	for (i = irq_base; i < irq_base + AR7100_MISC_IRQ_COUNT; i++) {
		irq_set_chip_and_handler(i, &ar7100_misc_irq_chip, handle_level_irq);
	}
}
