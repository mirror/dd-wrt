/*
 *  drivers/mtd/ndfc.c
 *
 *  Overview:
 *   Platform independent driver for NDFC (NanD Flash Controller)
 *   integrated into EP440 cores
 *
 *   Ported to an OF platform driver by Sean MacLennan
 *
 *   The NDFC supports multiple chips, but this driver only supports a
 *   single chip since I do not have access to any boards with
 *   multiple chips.
 *
 *  Author: Thomas Gleixner
 *
 *  Copyright 2006 IBM
 *  Copyright 2008 PIKA Technologies
 *    Sean MacLennan <smaclennan@pikatech.com>
 *
 *  This program is free software; you can redistribute	 it and/or modify it
 *  under  the terms of	 the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the	License, or (at your
 *  option) any later version.
 *
 */
#include <linux/module.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/ndfc.h>
#include <linux/slab.h>
#include <linux/mtd/mtd.h>
#include <linux/of_platform.h>
#include <asm/io.h>


struct ndfc_controller {
	struct of_device *ofdev;
	void __iomem *ndfcbase;
	struct nand_chip chip;
	int chip_select;
	struct nand_hw_control ndfc_control;
};

static struct ndfc_controller ndfc_ctrl;

extern int rb_nand_probe(struct nand_chip *nand, int booter);

static void ndfc_select_chip(struct mtd_info *mtd, int chip)
{
	uint32_t ccr;
	struct ndfc_controller *ndfc = &ndfc_ctrl;

	ccr = in_be32(ndfc->ndfcbase + NDFC_CCR);
	if (chip >= 0) {
		ccr &= ~NDFC_CCR_BS_MASK;
		ccr |= NDFC_CCR_BS(chip + ndfc->chip_select);
	} else
		ccr |= NDFC_CCR_RESET_CE;
	out_be32(ndfc->ndfcbase + NDFC_CCR, ccr);
}

static void ndfc_hwcontrol(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	struct ndfc_controller *ndfc = &ndfc_ctrl;

	if (cmd == NAND_CMD_NONE)
		return;

	if (ctrl & NAND_CLE)
		writel(cmd & 0xFF, ndfc->ndfcbase + NDFC_CMD);
	else
		writel(cmd & 0xFF, ndfc->ndfcbase + NDFC_ALE);
}

static int ndfc_ready(struct mtd_info *mtd)
{
	struct ndfc_controller *ndfc = &ndfc_ctrl;

	return in_be32(ndfc->ndfcbase + NDFC_STAT) & NDFC_STAT_IS_READY;
}

/*
 * Speedups for buffer read/write/verify
 *
 * NDFC allows 32bit read/write of data. So we can speed up the buffer
 * functions. No further checking, as nand_base will always read/write
 * page aligned.
 */
static void ndfc_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	struct ndfc_controller *ndfc = &ndfc_ctrl;
	uint32_t *p = (uint32_t *) buf;

	for(;len > 0; len -= 4)
		*p++ = in_be32(ndfc->ndfcbase + NDFC_DATA);
}

static void ndfc_write_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	struct ndfc_controller *ndfc = &ndfc_ctrl;
	uint32_t *p = (uint32_t *) buf;

	for(;len > 0; len -= 4)
		out_be32(ndfc->ndfcbase + NDFC_DATA, *p++);
}

/*
 * Initialize chip structure
 */
static int ndfc_chip_init(struct ndfc_controller *ndfc,
			  struct device_node *node)
{
	struct nand_chip *chip = &ndfc->chip;

	chip->IO_ADDR_R = ndfc->ndfcbase + NDFC_DATA;
	chip->IO_ADDR_W = ndfc->ndfcbase + NDFC_DATA;
	chip->cmd_ctrl = ndfc_hwcontrol;
	chip->dev_ready = ndfc_ready;
	chip->select_chip = ndfc_select_chip;
	chip->controller = &ndfc->ndfc_control;
	chip->read_buf = ndfc_read_buf;
	chip->write_buf = ndfc_write_buf;

	return rb_nand_probe(chip, 0);
}

static int ndfc_probe(struct of_device *ofdev,
				const struct of_device_id *match)
{
	struct ndfc_controller *ndfc = &ndfc_ctrl;
	const u32 *reg;
	u32 ccr;
	int err, len;

	spin_lock_init(&ndfc->ndfc_control.lock);
	init_waitqueue_head(&ndfc->ndfc_control.wq);
	ndfc->ofdev = ofdev;
	dev_set_drvdata(&ofdev->dev, ndfc);

	/* Read the reg property to get the chip select */
	reg = of_get_property(ofdev->dev.of_node, "reg", &len);
	if (reg == NULL || len != 12) {
		dev_err(&ofdev->dev, "unable read reg property (%d)\n", len);
		return -ENOENT;
	}
	ndfc->chip_select = reg[0];

	ndfc->ndfcbase = of_iomap(ofdev->dev.of_node, 0);
	if (!ndfc->ndfcbase) {
		dev_err(&ofdev->dev, "failed to get memory\n");
		return -EIO;
	}

	ccr = NDFC_CCR_BS(ndfc->chip_select);

	/* It is ok if ccr does not exist - just default to 0 */
	reg = of_get_property(ofdev->dev.of_node, "ccr", NULL);
	if (reg)
		ccr |= *reg;

	out_be32(ndfc->ndfcbase + NDFC_CCR, ccr);

	/* Set the bank settings if given */
	reg = of_get_property(ofdev->dev.of_node, "bank-settings", NULL);
	if (reg) {
		int offset = NDFC_BCFG0 + (ndfc->chip_select << 2);
		out_be32(ndfc->ndfcbase + offset, *reg);
	}

	err = ndfc_chip_init(ndfc, ofdev->dev.of_node);
	if (err) {
		iounmap(ndfc->ndfcbase);
		return err;
	}

	return 0;
}

static int ndfc_remove(struct of_device *ofdev)
{
	return 0;
}

static const struct of_device_id ndfc_match[] = {
	{ .compatible = "ibm,ndfc", },
	{}
};
MODULE_DEVICE_TABLE(of, ndfc_match);

static struct of_platform_driver ndfc_driver = {
	.driver = {
		.name = "ndfc",
		.owner = THIS_MODULE,
		.of_match_table = ndfc_match,
	},
	.probe = ndfc_probe,
	.remove = ndfc_remove,
};

static int __init ndfc_nand_init(void)
{
	return of_register_platform_driver(&ndfc_driver);
}

static void __exit ndfc_nand_exit(void)
{
	of_unregister_platform_driver(&ndfc_driver);
}

module_init(ndfc_nand_init);
module_exit(ndfc_nand_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Thomas Gleixner <tglx@linutronix.de>");
MODULE_DESCRIPTION("OF Platform driver for NDFC");
