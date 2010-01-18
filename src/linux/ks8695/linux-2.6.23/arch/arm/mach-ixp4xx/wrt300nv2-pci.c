/*
 * arch/arch/mach-ixp4xx/wrt300nv2-pci.c
 *
 * PCI setup routines for Linksys WRT300N v2
 *
 * Copyright (C) 2007 Imre Kaloz <kaloz@openwrt.org>
 *
 * based on coyote-pci.c:
 *	Copyright (C) 2002 Jungo Software Technologies.
 *	Copyright (C) 2003 MontaVista Softwrae, Inc.
 *
 * Maintainer: Imre Kaloz <kaloz@openwrt.org>
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
#include <asm/hardware.h>
#include <asm/irq.h>

#include <asm/mach/pci.h>

extern void ixp4xx_pci_preinit(void);
extern int ixp4xx_setup(int nr, struct pci_sys_data *sys);
extern struct pci_bus *ixp4xx_scan_bus(int nr, struct pci_sys_data *sys);

void __init wrt300nv2_pci_preinit(void)
{
	set_irq_type(IRQ_IXP4XX_GPIO8, IRQT_LOW);

	ixp4xx_pci_preinit();
}

static int __init wrt300nv2_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{
	if (slot == 1)
		return IRQ_IXP4XX_GPIO8;
	else return -1;
}

struct hw_pci wrt300nv2_pci __initdata = {
	.nr_controllers = 1,
	.preinit =        wrt300nv2_pci_preinit,
	.swizzle =        pci_std_swizzle,
	.setup =          ixp4xx_setup,
	.scan =           ixp4xx_scan_bus,
	.map_irq =        wrt300nv2_map_irq,
};

int __init wrt300nv2_pci_init(void)
{
	if (machine_is_wrt300nv2())
		pci_common_init(&wrt300nv2_pci);
	return 0;
}

subsys_initcall(wrt300nv2_pci_init);
