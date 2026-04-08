// SPDX-License-Identifier: GPL-2.0
#include <linux/mtd/spi-nor.h>

#include "core.h"

static const struct flash_info xtx_parts[] = {
	/* XTX Technology (Shenzhen) Limited */
	{
		.id = SNOR_ID(0x0B, 0x40, 0x18),
		.name = "xt25f128b",
		.size = SZ_16M,
		.no_sfdp_flags = SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ,
	},
};

const struct spi_nor_manufacturer spi_nor_xtx = {
	.name = "xtx",
	.parts = xtx_parts,
	.nparts = ARRAY_SIZE(xtx_parts),
};
