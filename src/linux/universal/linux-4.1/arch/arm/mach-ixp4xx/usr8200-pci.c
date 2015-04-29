/*
 * arch/arch/mach-ixp4xx/usr8200-pci.c
 *
 * PCI setup routines for USRobotics USR8200
 *
 * Copyright (C) 2008 Peter Denison <openwrt@marshadder.org>
 *
 * based on pronghorn-pci.c
 * 	Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 * based on coyote-pci.c:
 *	Copyright (C) 2002 Jungo Software Technologies.
 *	Copyright (C) 2003 MontaVista Softwrae, Inc.
 *
 * Maintainer: Peter Denison <openwrt@marshadder.org>
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

#define IRQT_LOW IRQ_TYPE_LEVEL_LOW
#define set_irq_type irq_set_irq_type

void __init usr8200_pci_preinit(void)
{
	set_irq_type(IRQ_IXP4XX_GPIO7, IRQ_TYPE_LEVEL_LOW);
	set_irq_type(IRQ_IXP4XX_GPIO8, IRQ_TYPE_LEVEL_LOW);
	set_irq_type(IRQ_IXP4XX_GPIO9, IRQ_TYPE_LEVEL_LOW);
	set_irq_type(IRQ_IXP4XX_GPIO10, IRQ_TYPE_LEVEL_LOW);
	set_irq_type(IRQ_IXP4XX_GPIO11, IRQ_TYPE_LEVEL_LOW);

	ixp4xx_pci_preinit();
}

static int __init usr8200_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
	if (slot == 14)
		return IRQ_IXP4XX_GPIO7;
	else if (slot == 15)
		return IRQ_IXP4XX_GPIO8;
	else if (slot == 16) {
		if (pin == 1)
			return IRQ_IXP4XX_GPIO11;
		else if (pin == 2)
			return IRQ_IXP4XX_GPIO10;
		else if (pin == 3)
			return IRQ_IXP4XX_GPIO9;
		else
			return -1;
	} else
		return -1;
}

struct hw_pci usr8200_pci __initdata = {
	.nr_controllers	= 1,
	.ops = &ixp4xx_ops,
	.preinit	= usr8200_pci_preinit,
	.setup		= ixp4xx_setup,
	.map_irq	= usr8200_map_irq,
};

int __init usr8200_pci_init(void)
{
	if (machine_is_usr8200())
		pci_common_init(&usr8200_pci);
	return 0;
}

subsys_initcall(usr8200_pci_init);
