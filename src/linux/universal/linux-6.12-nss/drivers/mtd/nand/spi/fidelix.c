// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Davide Fioravanti <pantanastyle@gmail.com>
 */

#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/mtd/spinand.h>

#define SPINAND_MFR_FIDELIX		0xE5
#define FIDELIX_ECCSR_MASK		0x0F

static SPINAND_OP_VARIANTS(read_cache_variants,
		SPINAND_PAGE_READ_FROM_CACHE_X4_OP(0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_OP(true, 0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_OP(false, 0, 1, NULL, 0));

static SPINAND_OP_VARIANTS(write_cache_variants,
		SPINAND_PROG_LOAD_X4(true, 0, NULL, 0),
		SPINAND_PROG_LOAD(true, 0, NULL, 0));

static SPINAND_OP_VARIANTS(update_cache_variants,
		SPINAND_PROG_LOAD_X4(true, 0, NULL, 0),
		SPINAND_PROG_LOAD(true, 0, NULL, 0));

static int fm35x1ga_ooblayout_ecc(struct mtd_info *mtd, int section,
				  struct mtd_oob_region *region)
{
	if (section > 3)
		return -ERANGE;

	region->offset = (16 * section) + 8;
	region->length = 8;

	return 0;
}

static int fm35x1ga_ooblayout_free(struct mtd_info *mtd, int section,
				   struct mtd_oob_region *region)
{
	if (section > 3)
		return -ERANGE;

	region->offset = (16 * section) + 2;
	region->length = 6;

	return 0;
}

static const struct mtd_ooblayout_ops fm35x1ga_ooblayout = {
	.ecc = fm35x1ga_ooblayout_ecc,
	.free = fm35x1ga_ooblayout_free,
};

static const struct spinand_info fidelix_spinand_table[] = {
	SPINAND_INFO("FM35X1GA",
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_DUMMY, 0x71),
		     NAND_MEMORG(1, 2048, 64, 64, 1024, 20, 1, 1, 1),
		     NAND_ECCREQ(4, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     SPINAND_HAS_QE_BIT,
		     SPINAND_ECCINFO(&fm35x1ga_ooblayout, NULL)),
};

static const struct spinand_manufacturer_ops fidelix_spinand_manuf_ops = {
};

const struct spinand_manufacturer fidelix_spinand_manufacturer = {
	.id = SPINAND_MFR_FIDELIX,
	.name = "Fidelix",
	.chips = fidelix_spinand_table,
	.nchips = ARRAY_SIZE(fidelix_spinand_table),
	.ops = &fidelix_spinand_manuf_ops,
};
