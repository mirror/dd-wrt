/*
 * magicmap.c: Copyleft 2005  Karol Lewandowski
 *
 * Mapping for MagicBox flash.
 * Based on walnut.c.
 *
 * Heikki Lindholm <holindho@infradead.org>
 *
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>
#include <linux/config.h>
#include <asm/io.h>

static struct mtd_info *flash;

static struct map_info magic_map = {
	.name =		"Magically mapped flash",
	.phys =         0xffc00000,
	.size =		0x400000,
	.bankwidth =	2,
};

static struct mtd_partition magic_partitions[] = {
	{
		.name =   "kernel",
		.offset = 0x0,
		.size =   0x0e0000,
	},
	{
		.name =   "ramdisk",
		.offset = 0x0e0000,
		.size =   0x2c0000,
	},
	{
		.name =   "persistent",
		.offset = 0x3a0000,
		.size =   0x020000,
	},
	{
		.name =   "bootloader",
		.offset = 0x3c0000,
		.size =   0x040000,
		.mask_flags = MTD_WRITEABLE,
	},
};

int __init init_magic(void)
{
	magic_map.virt =
		(void __iomem *)ioremap(magic_map.phys, magic_map.size);

	if (!magic_map.virt) {
		printk("Failed to ioremap flash.\n");
		return -EIO;
	}

	simple_map_init(&magic_map);

	flash = do_map_probe("cfi_probe", &magic_map);
	if (flash) {
		flash->owner = THIS_MODULE;
		add_mtd_partitions(flash, magic_partitions,
					ARRAY_SIZE(magic_partitions));
	} else {
		printk("map probe failed for flash\n");
		return -ENXIO;
	}

	return 0;
}

static void __exit cleanup_magic(void)
{
	if (flash) {
		del_mtd_partitions(flash);
		map_destroy(flash);
	}

	if (magic_map.virt) {
		iounmap((void *)magic_map.virt);
		magic_map.virt = NULL;
	}
}

module_init(init_magic);
module_exit(cleanup_magic);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Karol Lewandowski");
MODULE_DESCRIPTION("MTD map and partitions for IBM 405EP MagicBox boards");
