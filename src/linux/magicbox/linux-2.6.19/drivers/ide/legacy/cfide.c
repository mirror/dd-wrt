/*
 * Compact Flash Memory Mapped True IDE mode driver
 *
 * Maintainer: Kumar Gala <galak@kernel.crashing.org>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <linux/config.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/ide.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <asm/io.h>
#include <asm/byteorder.h>

static void __iomem *cfide_mapbase;
static void __iomem *cfide_alt_mapbase;

/* xxx: use standard outsw, insw when byte lanes swapped */
static void cfide_outsw(unsigned long port, void *addr, u32 count)
{
	_outsw((volatile u16 __iomem *)port, addr, count);
}

static void cfide_insw(unsigned long port, void *addr, u32 count)
{
	_insw((volatile u16 __iomem *)port, addr, count);
}

static void cfide_outl(u32 addr, unsigned long port)
{
	panic("outl unsupported");
}

static void cfide_outsl(unsigned long port, void *addr, u32 count)
{
	panic("outsl unsupported");
}

static u32 cfide_inl(unsigned long port)
{
	panic("inl unsupported");
	return 0;
}

static void cfide_insl(unsigned long port, void *addr, u32 count)
{
	panic("insl unsupported");
}

static ide_hwif_t *cfide_locate_hwif(void __iomem * base, void __iomem * ctrl,
				     unsigned int sz, int irq)
{
	unsigned long port = (unsigned long)base;
	ide_hwif_t *hwif;
	int index, i;

	for (index = 0; index < MAX_HWIFS; ++index) {
		hwif = ide_hwifs + index;
		if (hwif->io_ports[IDE_DATA_OFFSET] == port)
			goto found;
	}

	for (index = 0; index < MAX_HWIFS; ++index) {
		hwif = ide_hwifs + index;
		if (hwif->io_ports[IDE_DATA_OFFSET] == 0)
			goto found;
	}

	return NULL;

 found:

	/* xxx: fix when byte lanes are swapped */
	hwif->hw.io_ports[IDE_DATA_OFFSET] = port;
	port += 3;
	for (i = IDE_ERROR_OFFSET; i <= IDE_STATUS_OFFSET; i++, port += sz)
		hwif->hw.io_ports[i] = port;

	hwif->hw.io_ports[IDE_CONTROL_OFFSET] = (unsigned long)ctrl + 13;

	memcpy(hwif->io_ports, hwif->hw.io_ports, sizeof(hwif->hw.io_ports));
	hwif->hw.irq = hwif->irq = irq;

	hwif->hw.dma = NO_DMA;
	hwif->hw.chipset = ide_unknown;

	hwif->mmio = 2;
	default_hwif_mmiops(hwif);
	hwif->OUTL = cfide_outl;
	hwif->OUTSW = cfide_outsw;
	hwif->OUTSL = cfide_outsl;
	hwif->INL = cfide_inl;
	hwif->INSW = cfide_insw;
	hwif->INSL = cfide_insl;

	return hwif;
}

static int __devinit cfide_lbus_probe(struct platform_device *dev)
{
	struct resource *res_base, *res_alt, *res_irq;
	ide_hwif_t *hwif;
	int ret = 0;

	/* get a pointer to the register memory */
	res_base = platform_get_resource(dev, IORESOURCE_MEM, 0);
	res_alt = platform_get_resource(dev, IORESOURCE_MEM, 1);
	res_irq = platform_get_resource(dev, IORESOURCE_IRQ, 0);

	if ((!res_base) || (!res_alt) || (!res_irq)) {
		ret = -ENODEV;
		goto out;
	}

	if (!request_mem_region
	    (res_base->start, res_base->end - res_base->start + 1, dev->name)) {
		pr_debug("%s: request_mem_region of base failed\n", dev->name);
		ret = -EBUSY;
		goto out;
	}

	if (!request_mem_region
	    (res_alt->start, res_alt->end - res_alt->start + 1, dev->name)) {
		pr_debug("%s: request_mem_region of alt failed\n", dev->name);
		ret = -EBUSY;
		goto out;
	}

	cfide_mapbase =
	    ioremap(res_base->start, res_base->end - res_base->start + 1);
	if (!cfide_mapbase) {
		ret = -ENOMEM;
		goto out;
	}

	cfide_alt_mapbase =
	    ioremap(res_alt->start, res_alt->end - res_alt->start + 1);

	if (!cfide_alt_mapbase) {
		ret = -ENOMEM;
		goto unmap_base;
	}

	hwif = cfide_locate_hwif(cfide_mapbase, cfide_alt_mapbase, 2,
			      res_irq->start);

	if (!hwif) {
		ret = -ENODEV;
		goto unmap_alt;
	}

	hwif->gendev.parent = &dev->dev;
	hwif->noprobe = 0;

	probe_hwif_init(hwif);

	platform_set_drvdata(dev, hwif);

	return 0;

 unmap_alt:
	iounmap(cfide_alt_mapbase);
 unmap_base:
	iounmap(cfide_mapbase);
 out:
	return ret;
}

static int __devexit cfide_lbus_remove(struct platform_device *dev)
{
	ide_hwif_t *hwif = platform_get_drvdata(dev);
	struct resource *res_base, *res_alt;

	/* get a pointer to the register memory */
	res_base = platform_get_resource(dev, IORESOURCE_MEM, 0);
	res_alt = platform_get_resource(dev, IORESOURCE_MEM, 1);

	release_mem_region(res_base->start, res_base->end - res_base->start + 1);
	release_mem_region(res_alt->start, res_alt->end - res_alt->start + 1);

	platform_set_drvdata(dev, NULL);

	/* there must be a better way */
	ide_unregister(hwif - ide_hwifs);

	iounmap(cfide_mapbase);
	iounmap(cfide_alt_mapbase);

	return 0;
}

static struct platform_driver cfide_driver = {
	.probe = cfide_lbus_probe,
	.remove = __devexit_p(cfide_lbus_remove),
	.driver = {
		   .name = "mmio-cfide",
		   },
};

static int __init cfide_lbus_init(void)
{
	return platform_driver_register(&cfide_driver);
}

static void __exit cfide_lbus_exit(void)
{
	platform_driver_unregister(&cfide_driver);
}

MODULE_DESCRIPTION("MMIO based True IDE Mode Compact Flash Driver");
MODULE_LICENSE("GPL");

module_init(cfide_lbus_init);
module_exit(cfide_lbus_exit);
