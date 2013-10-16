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
//#include <linux/autoconf.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/delay.h>

#include <asm/mach/pci.h>
#include <asm/irq.h>
#include <mach/hardware.h>
#include <asm/mach-types.h>

extern void ixp4xx_pci_preinit(void);
extern int ixp4xx_setup(int nr, struct pci_sys_data *sys);
extern struct pci_bus *ixp4xx_scan_bus(int nr, struct pci_sys_data *sys);
#define IRQT_LOW IRQ_TYPE_LEVEL_LOW
#define set_irq_type irq_set_irq_type

void __init wrt300nv2_pci_preinit(void)
{
	gpio_line_config(AP71_PCI_INTA_PIN,
				IXP4XX_GPIO_IN | IXP4XX_GPIO_STYLE_ACTIVE_LOW);
	gpio_line_config(AP71_PCI_INTB_PIN,
				IXP4XX_GPIO_IN | IXP4XX_GPIO_STYLE_ACTIVE_LOW);
	gpio_line_config(AP71_PCI_INTC_PIN,
				IXP4XX_GPIO_IN | IXP4XX_GPIO_STYLE_ACTIVE_LOW);

	gpio_line_isr_clear(AP71_PCI_INTA_PIN);
	gpio_line_isr_clear(AP71_PCI_INTA_PIN);
	gpio_line_isr_clear(AP71_PCI_INTA_PIN);

    /*
     * our redboot doesnt need PCI. so it doesnt reset the bus either. Do it
     * here.
     * 1. Assert the reset
     * 2. wait for 1 ms
     * 3. configure and enable clock (nothing to do...?)
     * 4. wait for 100 ms
     * 5. de-assert the reset
     */
    gpio_line_set(AP71_PCI_RESET_LINE, IXP4XX_GPIO_OUT | IXP4XX_GPIO_LOW);
    mdelay(250);
    gpio_line_set(AP71_PCI_RESET_LINE, IXP4XX_GPIO_HIGH);

	ixp4xx_pci_preinit();
}

static int __init wrt300nv2_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
	static int pci_irq_table[AP71_PCI_MAX_DEV][AP71_PCI_IRQ_LINES] = {
        {AP71_PCI_INTA_IRQ, AP71_PCI_INTC_IRQ},
        {AP71_PCI_INTB_IRQ,     -1}
	};

	int irq = -1;

	if (slot >= 1 && slot <= AP71_PCI_MAX_DEV && 
		pin >= 1 && pin <= AP71_PCI_IRQ_LINES) {
		irq = pci_irq_table[slot - 1][pin - 1];
	}
    else {
        printk("bad interrupt map req for slot %d pin %d\n", slot, pin);
    }

	return irq;
}

struct hw_pci wrt300nv2_pci __initdata = {
	.nr_controllers = 1,
	.ops = &ixp4xx_ops,
	.preinit =        wrt300nv2_pci_preinit,
	.setup =          ixp4xx_setup,
	.map_irq =        wrt300nv2_map_irq,
};

int __init wrt300nv2_pci_init(void)
{
	if (machine_is_wrt300nv2())
		pci_common_init(&wrt300nv2_pci);
	return 0;
}

subsys_initcall(wrt300nv2_pci_init);
