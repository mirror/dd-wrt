/*
 * arch/arm/mach-ixp4xx/mi424wr-pci.c
 *
 * Actiontec MI424WR board-level PCI initialization
 *
 * Copyright (C) 2008 Jose Vasconcellos
 *
 * Maintainer: Jose Vasconcellos <jvasco@verizon.net>
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

#include <asm/mach-types.h>
#include <asm/mach/pci.h>

/* PCI controller GPIO to IRQ pin mappings
 * This information was obtained from Actiontec's GPL release.
 *
 *		INTA		INTB
 * SLOT 13	8		6
 * SLOT 14	7		8
 * SLOT 15	6		7
 */

void __init mi424wr_pci_preinit(void)
{
	irq_set_irq_type(IRQ_IXP4XX_GPIO6, IRQ_TYPE_LEVEL_LOW);
	irq_set_irq_type(IRQ_IXP4XX_GPIO7, IRQ_TYPE_LEVEL_LOW);
	irq_set_irq_type(IRQ_IXP4XX_GPIO8, IRQ_TYPE_LEVEL_LOW);

	ixp4xx_pci_preinit();
}

static int __init mi424wr_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
	if (slot == 13)
		return IRQ_IXP4XX_GPIO8;
	if (slot == 14)
		return IRQ_IXP4XX_GPIO7;
	if (slot == 15)
		return IRQ_IXP4XX_GPIO6;

	return -1;
}

struct hw_pci mi424wr_pci __initdata = {
	.nr_controllers = 1,
	.ops = &ixp4xx_ops,
	.preinit	= mi424wr_pci_preinit,
	.setup		= ixp4xx_setup,
	.map_irq	= mi424wr_map_irq,
};

int __init mi424wr_pci_init(void)
{
	if (machine_is_mi424wr())
		pci_common_init(&mi424wr_pci);
	return 0;
}

subsys_initcall(mi424wr_pci_init);

