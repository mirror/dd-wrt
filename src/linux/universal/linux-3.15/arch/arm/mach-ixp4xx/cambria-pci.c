/*
 * arch/arm/mach-ixp4xx/cambria-pci.c 
 *
 * Cambria board-level PCI initialization
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
void __init cambria_pci_preinit(void)
{
	set_irq_type(IRQ_CAMBRIA_PCI_INTA, IRQT_LOW);
	set_irq_type(IRQ_CAMBRIA_PCI_INTB, IRQT_LOW);
	set_irq_type(IRQ_CAMBRIA_PCI_INTC, IRQT_LOW);
	set_irq_type(IRQ_CAMBRIA_PCI_INTD, IRQT_LOW);

	ixp4xx_pci_preinit();
}

static int __init cambria_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
	if (slot == 1)
		return IRQ_CAMBRIA_PCI_INTA;
	else if (slot == 2)
		return IRQ_CAMBRIA_PCI_INTB;
	else if (slot == 3)
		return IRQ_CAMBRIA_PCI_INTC;
	else if (slot == 4)
		return IRQ_CAMBRIA_PCI_INTD;
	else return -1;
}

struct hw_pci cambria_pci __initdata = {
	.nr_controllers = 1,
	.ops = &ixp4xx_ops,
	.preinit	= cambria_pci_preinit,
	.setup		= ixp4xx_setup,
	.map_irq	= cambria_map_irq,
};

int __init cambria_pci_init(void)
{
	if (machine_is_cambria())
		pci_common_init(&cambria_pci);
	return 0;
}

subsys_initcall(cambria_pci_init);

