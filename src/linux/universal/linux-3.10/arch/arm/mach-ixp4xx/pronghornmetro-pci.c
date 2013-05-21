/*
 * arch/arch/mach-ixp4xx/pronghornmetro-pci.c
 *
 * PCI setup routines for ADI Engineering Pronghorn Metro platform
 *
 * Copyright (C) 2002 Jungo Software Technologies.
 * Copyright (C) 2003 MontaVista Softwrae, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Author: Copied from coyote-pci.c
 */

#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/irq.h>

#include <asm/mach-types.h>
#include <mach/hardware.h>
#include <asm/irq.h>

#include <asm/mach/pci.h>

#define IRQT_LOW IRQ_TYPE_LEVEL_LOW
#define set_irq_type irq_set_irq_type

extern void ixp4xx_pci_preinit(void);
extern int ixp4xx_setup(int nr, struct pci_sys_data *sys);
extern struct pci_bus *ixp4xx_scan_bus(int nr, struct pci_sys_data *sys);

void __init pronghornmetro_pci_preinit(void)
{
	set_irq_type(IRQ_PCI_SLOT0, IRQT_LOW);
	set_irq_type(IRQ_PCI_SLOT1, IRQT_LOW);
	if (machine_is_pronghorn_metro())
	{
	set_irq_type(IRQ_PCI_SLOT2, IRQT_LOW);
	set_irq_type(IRQ_PCI_SLOT3, IRQT_LOW);
	}
	ixp4xx_pci_preinit();
}

static int __init pronghornmetro_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
	if (slot == PCI_SLOT0_DEVID)
		return IRQ_PCI_SLOT0;
	else if (slot == PCI_SLOT1_DEVID)
		return IRQ_PCI_SLOT1;
	else if (slot == PCI_SLOT2_DEVID)
		return IRQ_PCI_SLOT2;
	else if (slot == PCI_SLOT3_DEVID)
		return IRQ_PCI_SLOT3;
	else return -1;
}

struct hw_pci pronghornmetro_pci __initdata = {
	.nr_controllers = 1,
	.ops = &ixp4xx_ops,
	.preinit =        pronghornmetro_pci_preinit,
	.setup =          ixp4xx_setup,
	.map_irq =        pronghornmetro_map_irq,
};

int __init pronghornmetro_pci_init(void)
{
	if (machine_is_pronghorn_metro() || machine_is_pronghorn())
		pci_common_init(&pronghornmetro_pci);
	return 0;
}

subsys_initcall(pronghornmetro_pci_init);
