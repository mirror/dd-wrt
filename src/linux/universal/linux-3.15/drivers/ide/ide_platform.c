/*
 * Platform IDE driver
 *
 * Copyright (C) 2007 MontaVista Software
 *
 * Maintainer: Kumar Gala <galak@kernel.crashing.org>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/ide.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/ata_platform.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#ifdef CONFIG_MACH_KS8695_VSOPENRISC
#include <mach/vsopenrisc.h>
#include <mach/regs-mem.h>

static void vsopenrisc_set_pio(ide_hwif_t *hwif, ide_drive_t *drive)
{
	unsigned long flags, addr;

	spin_lock_irqsave(&hwif->lock, flags);

	// FIXME: we should identify the supported PIO from the devices
	// and setting the EPLD to the minimum value of them both.

	writeb(0x03,  (void __iomem *)hwif->io_ports.data_addr + 4/*IDE_FEATURE_REG*/);
	if ((drive->pio_mode - XFER_PIO_0) >= 3)
		writeb(0x0b,  (void __iomem *)hwif->io_ports.data_addr + (2*4)/*IDE_NSECTOR_REG*/);
	else
		writeb(0x08,  (void __iomem *)hwif->io_ports.data_addr + (2*4)/*IDE_NSECTOR_REG*/);
	// FIXME: didn't find a field, which shows the state of the device
	if (drive->name[2] == 'a')	// master
		writeb(0x00,  (void __iomem *)hwif->io_ports.data_addr + (6*4)/*IDE_SELECT_REG*/);
	else
		writeb(0x10,  (void __iomem *)hwif->io_ports.data_addr + (6*4)/*IDE_SELECT_REG*/);
	writeb(0xef,  (void __iomem *)hwif->io_ports.data_addr + (7*4)/*IDE_COMMAND_REG*/);

	// configure the "controller" (EPLD)
	addr = VSOPENRISC_VA_EPLD_IDE_BASE;

	if ((drive->pio_mode - XFER_PIO_0) >= 3)
		writeb(0x03, (void __iomem *)addr);
	else
		writeb(0x00, (void __iomem *)addr);

	spin_unlock_irqrestore(&hwif->lock, flags);
}

static const struct ide_port_ops vsopenrisc_port_ops = {
	.set_pio_mode		= vsopenrisc_set_pio,
};

#endif

static void plat_ide_setup_ports(struct ide_hw *hw, void __iomem *base,
				 void __iomem *ctrl,
				 struct pata_platform_info *pdata, int irq)
{
	unsigned long port = (unsigned long)base;
	int i;

	hw->io_ports.data_addr = port;

	port += (1 << pdata->ioport_shift);
	for (i = 1; i <= 7;
	     i++, port += (1 << pdata->ioport_shift))
		hw->io_ports_array[i] = port;

	hw->io_ports.ctl_addr = (unsigned long)ctrl;

	hw->irq = irq;
}

static const struct ide_port_info platform_ide_port_info = {
#ifdef CONFIG_MACH_KS8695_VSOPENRISC
	.port_ops		= &vsopenrisc_port_ops,
#endif
	.host_flags		= IDE_HFLAG_NO_DMA,
	.chipset		= ide_generic,
#ifdef CONFIG_MACH_KS8695_VSOPENRISC
	.pio_mask		= ATA_PIO4,
#endif
};

static int plat_ide_probe(struct platform_device *pdev)
{
	struct resource *res_base, *res_alt, *res_irq;
	void __iomem *base, *alt_base;
	struct pata_platform_info *pdata;
	struct ide_host *host;
	int ret = 0, mmio = 0;
	struct ide_hw hw, *hws[] = { &hw };
	struct ide_port_info d = platform_ide_port_info;

	pdata = dev_get_platdata(&pdev->dev);

	/* get a pointer to the register memory */
	res_base = platform_get_resource(pdev, IORESOURCE_IO, 0);
	res_alt = platform_get_resource(pdev, IORESOURCE_IO, 1);

	if (!res_base || !res_alt) {
		res_base = platform_get_resource(pdev, IORESOURCE_MEM, 0);
		res_alt = platform_get_resource(pdev, IORESOURCE_MEM, 1);
		if (!res_base || !res_alt) {
			ret = -ENOMEM;
			goto out;
		}
		mmio = 1;
	}

	res_irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!res_irq) {
		ret = -EINVAL;
		goto out;
	}

	if (mmio) {
		printk(KERN_EMERG "base addr %p\n",res_base->start);
		base = devm_ioremap(&pdev->dev,
			res_base->start, resource_size(res_base));
		printk(KERN_EMERG "ctrl addr %p\n",res_alt->start);
		alt_base = devm_ioremap(&pdev->dev,
			res_alt->start, resource_size(res_alt));
		printk(KERN_EMERG "map base %p\n",base);
		printk(KERN_EMERG "map ctrl %p\n",alt_base);
	} else {
		base = devm_ioport_map(&pdev->dev,
			res_base->start, resource_size(res_base));
		alt_base = devm_ioport_map(&pdev->dev,
			res_alt->start, resource_size(res_alt));

		printk(KERN_EMERG "base addr %p\n",res_base->start);
		printk(KERN_EMERG "ctrl addr %p\n",res_alt->start);
		printk(KERN_EMERG "map base %p\n",base);
		printk(KERN_EMERG "map ctrl %p\n",alt_base);
	}

	memset(&hw, 0, sizeof(hw));
	plat_ide_setup_ports(&hw, base, alt_base, pdata, res_irq->start);
	hw.dev = &pdev->dev;

	d.irq_flags = res_irq->flags & IRQF_TRIGGER_MASK;
#ifdef CONFIG_MACH_KS8695_VSOPENRISC
		d.irq_flags |= IRQF_SHARED;
#else
	if (res_irq->flags & IORESOURCE_IRQ_SHAREABLE)
		d.irq_flags |= IRQF_SHARED;
#endif
	if (mmio)
		d.host_flags |= IDE_HFLAG_MMIO;

	ret = ide_host_add(&d, hws, 1, &host);
	if (ret)
		goto out;

	platform_set_drvdata(pdev, host);

	return 0;

out:
	return ret;
}

static int plat_ide_remove(struct platform_device *pdev)
{
	struct ide_host *host = dev_get_drvdata(&pdev->dev);

	ide_host_remove(host);

	return 0;
}

static struct platform_driver platform_ide_driver = {
	.driver = {
		.name = "pata_platform",
		.owner = THIS_MODULE,
	},
	.probe = plat_ide_probe,
	.remove = plat_ide_remove,
};

static int __init platform_ide_init(void)
{
	return platform_driver_register(&platform_ide_driver);
}

static void __exit platform_ide_exit(void)
{
	platform_driver_unregister(&platform_ide_driver);
}

MODULE_DESCRIPTION("Platform IDE driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:pata_platform");

module_init(platform_ide_init);
module_exit(platform_ide_exit);
