/*
 * BCM47XX NAND flash driver
 *
 * Copyright (C) 2012 Rafał Miłecki <zajec5@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include "bcm47xxnflash.h"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/bcma/bcma.h>
#include <bcmnvram.h>
#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmdevs.h>
#include <bcmnvram.h>
#include <siutils.h>
#include <hndpci.h>
#include <pcicfg.h>
#include <hndsoc.h>
#define	NFLASH_SUPPORT
#include <sbchipc.h>
#include <nflash.h>

#ifdef CONFIG_MTD
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>
#include "../../mtdcore.h"
#include <linux/mtd/partitions.h>
extern int boot_flags(void);
extern struct mtd_partition * init_brcmnand_mtd_partitions(struct mtd_info *mtd, size_t size);
extern void add_netgear_boarddata(void);
extern void add_netgear_boarddata_sflash(void);
extern void add_cfenvram(void);

#endif

#ifdef CONFIG_MTD
extern struct mtd_partition *
init_nflash_mtd_partitions(struct mtd_info *mtd, size_t size);

struct mtd_partition *nflash_parts;
#endif

MODULE_DESCRIPTION("NAND flash driver for BCMA bus");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rafał Miłecki");

static const char *probes[] = { "bcm47xxpart", NULL };

struct mtd_partition brcmnand_parts[3] = {{0}};

struct mtd_partition *init_brcmnand_mtd_partitions(struct mtd_info *mtd, size_t size)
{
	int bootflags = boot_flags();
	int j = 0;
	int offset = 0;
	int offlinux = 0;
	int offddwrt = 0;
	
	if (nvram_match("boardnum", "3500L") && nvram_match("boardtype", "0x052b")){
		offlinux = 0x300000;
		offddwrt = 0x2000000;
	}
	
	if ((bootflags & FLASH_KERNEL_NFLASH) == FLASH_KERNEL_NFLASH) {
		brcmnand_parts[j].name = "linux";
		if ((bootflags & FLASH_BOOT_NFLASH) == FLASH_BOOT_NFLASH) {
			brcmnand_parts[j].offset = NFL_BOOT_SIZE + offlinux;
			brcmnand_parts[j++].size = NFL_BOOT_OS_SIZE - NFL_BOOT_SIZE - offlinux;
		} else {
			brcmnand_parts[j].offset = 0;
			brcmnand_parts[j++].size = NFL_BOOT_OS_SIZE;
		}			
		offset += NFL_BOOT_OS_SIZE + offddwrt;
		size -= NFL_BOOT_OS_SIZE + offddwrt;
	}

	size -= NFL_BBT_SIZE;
	if (size <= 0) {
		printk(KERN_ERR "%s: nand flash size is too small\n", __func__);
		return NULL;
	}

	brcmnand_parts[j].name = "ddwrt";
	brcmnand_parts[j].offset = offset;
	brcmnand_parts[j++].size = size;

	return brcmnand_parts;
}

static int bcm47xxnflash_probe(struct platform_device *pdev)
{
	struct bcma_nflash *nflash = dev_get_platdata(&pdev->dev);
	struct bcm47xxnflash *b47n;
	int err = 0;
#ifdef CONFIG_MTD
	struct mtd_partition *parts;
	int i, ret;
#endif

	b47n = devm_kzalloc(&pdev->dev, sizeof(*b47n), GFP_KERNEL);
	if (!b47n)
		return -ENOMEM;

	b47n->nand_chip.priv = b47n;
	b47n->mtd.dev.parent = &pdev->dev;
	b47n->mtd.priv = &b47n->nand_chip; /* Required */
	b47n->cc = container_of(nflash, struct bcma_drv_cc, nflash);

	if (b47n->cc->core->bus->chipinfo.id == BCMA_CHIP_ID_BCM4706) {
		err = bcm47xxnflash_ops_bcm4706_init(b47n);
	} else {
		pr_err("Device not supported\n");
		err = -ENOTSUPP;
	}
	if (err) {
		pr_err("Initialization failed: %d\n", err);
		return err;
	}

	err = mtd_device_parse_register(&b47n->mtd, probes, NULL, NULL, 0);
	if (err) {
		pr_err("Failed to register MTD device: %d\n", err);
		return err;
	}

#ifdef CONFIG_MTD
	parts = init_nflash_mtd_partitions(&b47n->mtd, b47n->mtd.size);
	if (!parts)
		goto fail;

	for (i = 0; parts[i].name; i++)
		;
		
	if (!i){
		parts[i].name = "nand";
		parts[i].offset = 0;
		parts[i].size = b47n->mtd.size;
		i++;
	}

	ret = add_mtd_partitions(&b47n->mtd, parts, i);
	if (ret) {
		printk(KERN_ERR "nflash: add_mtd failed\n");
		goto fail;
	}
	nflash_parts = parts;
#ifdef CONFIG_MIPS
	printk(KERN_INFO "add netgear boarddata for pflash if exists\n");
	add_netgear_boarddata();
	printk(KERN_INFO "add netgear boarddata for sflash if exists\n");
	add_netgear_boarddata_sflash();
	add_cfenvram();
#endif
#endif
#ifdef CONFIG_MTD
	parts = init_brcmnand_mtd_partitions(&b47n->mtd, b47n->mtd.size);
	if (!parts)
		goto fail;
	for (i = 0; parts[i].name; i++);
	ret = add_mtd_partitions(&b47n->mtd, parts, i);
	if (ret) {
		printk(KERN_ERR "brcmnand: add_mtd failed\n");
		goto fail;
	}
#endif
    fail:;
	return 0;
}

static int bcm47xxnflash_remove(struct platform_device *pdev)
{
	struct bcma_nflash *nflash = dev_get_platdata(&pdev->dev);

	if (nflash->mtd)
		mtd_device_unregister(nflash->mtd);

	return 0;
}

static struct platform_driver bcm47xxnflash_driver = {
	.probe	= bcm47xxnflash_probe,
	.remove = bcm47xxnflash_remove,
	.driver = {
		.name = "bcma_nflash",
	},
};

module_platform_driver(bcm47xxnflash_driver);
