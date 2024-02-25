/*
 *  Atheros AR71xx PCI setup code
 *
 *  Copyright (C) 2008-2009 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  Parts of this file are based on Atheros' 2.6.15 BSP
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/kernel.h>

#include <asm/traps.h>
#include <linux/init.h>
#include <linux/export.h>
#include <linux/pci.h>
#include <linux/resource.h>
#include <linux/platform_device.h>

#include <asm/mach-ar71xx/ar71xx.h>
#include <asm/mach-ar71xx/pci.h>
#ifndef CONFIG_MACH_HORNET
static unsigned ar71xx_pci_nr_irqs __initdata;
static const struct ar71xx_pci_irq *ar71xx_pci_irq_map __initdata;


int (*ar71xx_pci_plat_dev_init) (struct pci_dev * dev);

#ifdef CONFIG_MACH_AR7100
static int ar71xx_be_handler(struct pt_regs *regs, int is_fixup)
{
	int err = 0;

	err = ar71xx_pci_be_handler(is_fixup);

	return (is_fixup && !err) ? MIPS_BE_FIXUP : MIPS_BE_FATAL;
}
#endif
int pcibios_plat_dev_init(struct pci_dev *dev)
{
	if (ar71xx_pci_plat_dev_init)
		return ar71xx_pci_plat_dev_init(dev);

	return 0;
}

static const struct ar71xx_pci_irq ar724x_pci_irq_map[] __initconst = {
	{
		.slot	= 0,
		.pin	= 1,
		.irq	= AR71XX_PCI_IRQ(0),
	}
};

static const struct ar71xx_pci_irq qca953x_pci_irq_map[] __initconst = {
	{
		.bus	= 0,
		.slot	= 0,
		.pin	= 1,
		.irq	= AR71XX_PCI_IRQ(0),	
	},
};

static const struct ar71xx_pci_irq qca955x_pci_irq_map[] __initconst = {
	{
		.bus	= 0,
		.slot	= 0,
		.pin	= 1,
		.irq	= AR71XX_PCI_IRQ(0),
	},
	{
		.bus	= 1,
		.slot	= 0,
		.pin	= 1,
		.irq	= AR71XX_PCI_IRQ(1),
	},
};


int __init pcibios_map_irq(const struct pci_dev *dev, uint8_t slot, uint8_t pin)
{
	int irq = -1;
	int i;

	if (ar71xx_pci_nr_irqs == 0 ||
	    ar71xx_pci_irq_map == NULL) {
		if (soc_is_ar724x() ||
			   soc_is_ar9342() ||
			   soc_is_ar9344()) {
			ar71xx_pci_irq_map = ar724x_pci_irq_map;
			ar71xx_pci_nr_irqs = ARRAY_SIZE(ar724x_pci_irq_map);
		} else if (soc_is_qca953x()) {
			ar71xx_pci_irq_map = qca953x_pci_irq_map;
			ar71xx_pci_nr_irqs = ARRAY_SIZE(qca953x_pci_irq_map);
		} else if (soc_is_qca955x()) {
			ar71xx_pci_irq_map = qca955x_pci_irq_map;
			ar71xx_pci_nr_irqs = ARRAY_SIZE(qca955x_pci_irq_map);
		} else if (soc_is_qca956x()) {
			ar71xx_pci_irq_map = qca955x_pci_irq_map;
			ar71xx_pci_nr_irqs = ARRAY_SIZE(qca955x_pci_irq_map);
		} else {
			printk(KERN_INFO "pci %s: invalid irq map\n",
				pci_name((struct pci_dev *) dev));
			return irq;
		}
	}

	for (i = 0; i < ar71xx_pci_nr_irqs; i++) {
		const struct ar71xx_pci_irq *entry;

		entry = &ar71xx_pci_irq_map[i];
		if (entry->bus == dev->bus->number &&
		    entry->slot == slot &&
		    entry->pin == pin) {
			irq = entry->irq;
			break;
		}
	}

	if (irq < 0)
		printk(KERN_INFO "pci %s: no irq found for pin %u\n",
			pci_name((struct pci_dev *) dev), pin);
	else
		printk(KERN_INFO "pci %s: using irq %d for pin %u\n",
			pci_name((struct pci_dev *) dev), irq, pin);

	return irq;
}

static struct platform_device *
ar71xx_register_pci_ar724x(int id,
			  unsigned long cfg_base,
			  unsigned long ctrl_base,
			  unsigned long crp_base,
			  unsigned long mem_base,
			  unsigned long mem_size,
			  unsigned long io_base,
			  int irq)
{
	struct platform_device *pdev;
	struct resource res[6];

	memset(res, 0, sizeof(res));

	res[0].name = "cfg_base";
	res[0].flags = IORESOURCE_MEM;
	res[0].start = cfg_base;
	res[0].end = cfg_base + AR724X_PCI_CFG_SIZE - 1;

	res[1].name = "ctrl_base";
	res[1].flags = IORESOURCE_MEM;
	res[1].start = ctrl_base;
	res[1].end = ctrl_base + AR724X_PCI_CTRL_SIZE - 1;

	res[2].flags = IORESOURCE_IRQ;
	res[2].start = irq;
	res[2].end = irq;

	res[3].name = "mem_base";
	res[3].flags = IORESOURCE_MEM;
	res[3].start = mem_base;
	res[3].end = mem_base + mem_size - 1;

	res[4].name = "io_base";
	res[4].flags = IORESOURCE_IO;
	res[4].start = io_base;
	res[4].end = io_base;

	res[5].name = "crp_base";
	res[5].flags = IORESOURCE_MEM;
	res[5].start = crp_base;
	res[5].end = crp_base + AR724X_PCI_CRP_SIZE - 1;

	pdev = platform_device_register_simple("ar724x-pci", id,
					       res, ARRAY_SIZE(res));
	return pdev;
}


int __init ar71xx_pci_init(void)
{
	u32 t;
	int ret = 0;
	struct platform_device *pdev = NULL;

	switch (ar71xx_soc) {
	
	case AR71XX_SOC_AR7240:
	case AR71XX_SOC_AR7241:
	case AR71XX_SOC_AR7242:
		pdev = ar71xx_register_pci_ar724x(-1,
						 AR724X_PCI_CFG_BASE,
						 AR724X_PCI_CTRL_BASE,
						 AR724X_PCI_CRP_BASE,
						 AR71XX_PCI_MEM_BASE,
						 AR71XX_PCI_MEM_SIZE,
						 0,
						 AR71XX_CPU_IRQ_IP2);
		break;

	case AR71XX_SOC_AR9342:
	case AR71XX_SOC_AR9344:
		t = ar71xx_reset_rr(AR934X_RESET_REG_BOOTSTRAP);
		if (t & AR934X_BOOTSTRAP_PCIE_RC) {
		pdev = ar71xx_register_pci_ar724x(-1,
						 AR724X_PCI_CFG_BASE,
						 AR724X_PCI_CTRL_BASE,
						 AR724X_PCI_CRP_BASE,
						 AR71XX_PCI_MEM_BASE,
						 AR71XX_PCI_MEM_SIZE,
						 0,
						 AR934X_IP2_IRQ_PCIE);
			break;
		} else
			printk("no pci device found\n");
		return 0;
		break;
	case AR71XX_SOC_QCA9533:
		pdev = ar71xx_register_pci_ar724x(0,
						 QCA953X_PCI_CFG_BASE0,
						 QCA953X_PCI_CTRL_BASE0,
						 QCA953X_PCI_CRP_BASE0,
						 QCA953X_PCI_MEM_BASE0,
						 QCA953X_PCI_MEM_SIZE,
						 0,
						 AR934X_IP2_IRQ(0));
		break;
	case AR71XX_SOC_QCA9556:
	case AR71XX_SOC_QCA9558:
		pdev = ar71xx_register_pci_ar724x(0,
						 QCA955X_PCI_CFG_BASE0,
						 QCA955X_PCI_CTRL_BASE0,
						 QCA955X_PCI_CRP_BASE0,
						 QCA955X_PCI_MEM_BASE0,
						 QCA955X_PCI_MEM_SIZE,
						 0,
						 AR934X_IP2_IRQ(0));

		pdev = ar71xx_register_pci_ar724x(1,
						 QCA955X_PCI_CFG_BASE1,
						 QCA955X_PCI_CTRL_BASE1,
						 QCA955X_PCI_CRP_BASE1,
						 QCA955X_PCI_MEM_BASE1,
						 QCA955X_PCI_MEM_SIZE,
						 1,
						 AR934X_IP3_IRQ(2));
		/* fall through */
		break;
	case AR71XX_SOC_QCA9563:
	case AR71XX_SOC_TP9343:
	case AR71XX_SOC_QCN550X:
		pdev = ar71xx_register_pci_ar724x(0,
						 QCA956X_PCI_CFG_BASE1,
						 QCA956X_PCI_CTRL_BASE1,
						 QCA956X_PCI_CRP_BASE1,
						 QCA956X_PCI_MEM_BASE1,
						 QCA956X_PCI_MEM_SIZE,
						 1,
						 AR934X_IP3_IRQ(2));

		/* fall through */
		break;
	default:
		return 0;
	}

	return ret;
}
#endif
