/*
 * arch/arm/mach-ixp4xx/ixdp425-pci.c 
 *
 * IXDP425 board-level PCI initialization
 *
 * Copyright (C) 2002 Intel Corporation.
 * Copyright (C) 2003-2004 MontaVista Software, Inc.
 *
 * Maintainer: Deepak Saxena <dsaxena@plexity.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/delay.h>

#include <asm/mach/pci.h>
#include <asm/irq.h>
#include <mach/hardware.h>
#include <asm/mach-types.h>

#define IRQT_LOW IRQ_TYPE_LEVEL_LOW
#define set_irq_type irq_set_irq_type

void __init ixdp425_pci_preinit(void)
{
	irq_set_irq_type(IRQ_IXDP425_PCI_INTA, IRQT_LOW);
	irq_set_irq_type(IRQ_IXDP425_PCI_INTB, IRQT_LOW);
	irq_set_irq_type(IRQ_IXDP425_PCI_INTC, IRQT_LOW);
	irq_set_irq_type(IRQ_IXDP425_PCI_INTD, IRQT_LOW);

	ixp4xx_pci_preinit();
}

static int __init ixdp425_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
	static int pci_irq_table[AVILA_PCI_IRQ_LINES] = {
		IRQ_IXDP425_PCI_INTA,
		IRQ_IXDP425_PCI_INTB,
		IRQ_IXDP425_PCI_INTC,
		IRQ_IXDP425_PCI_INTD
	};

	int irq = -1;

	if (slot >= 1 && slot <= AVILA_PCI_MAX_DEV && 
		pin >= 1 && pin <= AVILA_PCI_IRQ_LINES) {
		irq = pci_irq_table[(slot + pin - 2) % 4];
	}

	return irq;
}

struct hw_pci ixdp425_pci __initdata = {
	.nr_controllers = 1,
	.ops		= &ixp4xx_ops,
	.preinit	= ixdp425_pci_preinit,
	.setup		= ixp4xx_setup,
	.map_irq	= ixdp425_map_irq,
};

int __init ixdp425_pci_init(void)
{
	if (machine_is_ixdp425() || machine_is_ixcdp1100() ||
			machine_is_ixdp465() || machine_is_kixrp435() ||
			machine_is_compex())
		pci_common_init(&ixdp425_pci);
	return 0;
}

subsys_initcall(ixdp425_pci_init);

