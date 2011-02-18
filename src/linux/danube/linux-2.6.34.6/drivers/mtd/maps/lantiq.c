/*
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 *  Copyright (C) 2004 Liu Peng Infineon IFAP DC COM CPE
 *  Copyright (C) 2010 John Crispin <blogic@openwrt.org>
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/init.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/cfi.h>
#include <linux/magic.h>
#include <linux/platform_device.h>
#include <linux/squashfs_fs.h>
#include <linux/mtd/physmap.h>

#include <lantiq.h>
#include <lantiq_platform.h>

#ifdef CONFIG_SOC_LANTIQ_XWAY
#include <xway.h>
#endif

static map_word
lq_read16(struct map_info *map, unsigned long adr)
{
	unsigned long flags;
	map_word temp;
	spin_lock_irqsave(&ebu_lock, flags);
	adr ^= 2;
	temp.x[0] = *((__u16 *)(map->virt + adr));
	spin_unlock_irqrestore(&ebu_lock, flags);
	return temp;
}

static void
lq_write16(struct map_info *map, map_word d, unsigned long adr)
{
	unsigned long flags;
	spin_lock_irqsave(&ebu_lock, flags);
	adr ^= 2;
	*((__u16 *)(map->virt + adr)) = d.x[0];
	spin_unlock_irqrestore(&ebu_lock, flags);
}

void
lq_copy_from(struct map_info *map, void *to,
	unsigned long from, ssize_t len)
{
	unsigned char *p;
	unsigned char *to_8;
	unsigned long flags;
	spin_lock_irqsave(&ebu_lock, flags);
	from = (unsigned long)(from + map->virt);
	p = (unsigned char *) from;
	to_8 = (unsigned char *) to;
	while (len--)
		*to_8++ = *p++;
	spin_unlock_irqrestore(&ebu_lock, flags);
}

void
lq_copy_to(struct map_info *map, unsigned long to,
	const void *from, ssize_t len)
{
	unsigned char *p =  (unsigned char *)from;
	unsigned char *to_8;
	unsigned long flags;
	spin_lock_irqsave(&ebu_lock, flags);
	to += (unsigned long) map->virt;
	to_8 = (unsigned char *)to;
	while (len--)
		*p++ = *to_8++;
	spin_unlock_irqrestore(&ebu_lock, flags);
}

static struct mtd_partition ifxmips_partitions[] = {
	{
		.name = "uboot",
		.offset = 0x00000000,
		.size = 0x00020000,
	},
	{
		.name = "uboot_env",
		.offset = 0x00020000,
		.size = 0x0,
	},
	{
		.name = "kernel",
		.offset = 0x0,
		.size = 0x0,
	},
	{
		.name = "rootfs",
		.offset = 0x0,
		.size = 0x0,
	},
	{
		.name = "nvram",
		.offset = 0x0,
		.size = 0x0,
	},
	{
		.name = "board_config",
		.offset = 0x0,
		.size = 0x0,
	},
	{
		.name = "fullflash",
		.offset = 0x0,
		.size = 0x0,
	},
};

static struct mtd_partition ifxmips_meta_partition = {
	.name = "linux",
	.offset = 0x00030000,
	.size = 0x0,
};


static unsigned long
find_uImage_size(struct map_info *map, unsigned long offset)
{
#define UBOOT_MAGIC	0x56190527
	unsigned long magic;
	unsigned long temp;
	map->copy_from(map, &magic, offset, 4);
	if (le32_to_cpu(magic) != UBOOT_MAGIC)
		return 0;
	map->copy_from(map, &temp, offset + 12, 4);
	printk("uImage size %X\n",temp + 0x40);
	return temp + 0x40;
}

static int
detect_squashfs_partition(struct map_info *map, unsigned long offset)
{
	unsigned long temp;
	map->copy_from(map, &temp, offset, 4);
	return le32_to_cpu(temp) == SQUASHFS_MAGIC;
}


static const char *part_probe_types[] = { "cmdlinepart", NULL };




static struct map_info lq_map = {
	.name = "lq_nor",
	.bankwidth = 2,
	.read = lq_read16,
	.write = lq_write16,
	.copy_from = lq_copy_from,
	.copy_to = lq_copy_to,
};



static int
lq_mtd_probe(struct platform_device *pdev)
{
	struct physmap_flash_data *lq_mtd_data =
		(struct physmap_flash_data*) dev_get_platdata(&pdev->dev);
	struct mtd_info *lq_mtd = NULL;
	struct mtd_partition *parts = NULL;
	struct resource *res = 0;
	int nr_parts = 0;
	unsigned long uimage_size;
	int err, i;
	int kernel_part = 2, rootfs_part = 3;
	int num_parts = ARRAY_SIZE(ifxmips_partitions);

#ifdef CONFIG_SOC_LANTIQ_XWAY
	lq_w32(lq_r32(LQ_EBU_BUSCON0) & ~EBU_WRDIS, LQ_EBU_BUSCON0);
#endif

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(!res)
	{
		dev_err(&pdev->dev, "failed to get memory resource");
		return -ENOENT;
	}
	res = request_mem_region(res->start, resource_size(res),
		dev_name(&pdev->dev));
	if(!res)
	{
		dev_err(&pdev->dev, "failed to request mem resource");
		return -EBUSY;
	}

	lq_map.phys = res->start;
	lq_map.size = resource_size(res);
	lq_map.virt = ioremap_nocache(lq_map.phys, lq_map.size);

	if (!lq_map.virt ) {
		dev_err(&pdev->dev, "failed to ioremap!\n");
		return -EIO;
	}

	lq_mtd = (struct mtd_info *) do_map_probe("cfi_probe", &lq_map);
	if (!lq_mtd) {
		iounmap(lq_map.virt);
		dev_err(&pdev->dev, "probing failed\n");
		return -ENXIO;
	}

	lq_mtd->owner = THIS_MODULE;

	nr_parts = parse_mtd_partitions(lq_mtd, part_probe_types, &parts, 0);
	printk(KERN_EMERG "parse_mtd returns %d\n",nr_parts);
	if (nr_parts > 0) {
		printk(KERN_INFO "ifxmips_mtd: found %d partitions from cmdline\n", err);
		num_parts = err;
		kernel_part = 0;
		rootfs_part = 0;
		for (i = 0; i < num_parts; i++) {
			if (strcmp(parts[i].name, "kernel") == 0)
				kernel_part = i;
			if (strcmp(parts[i].name, "rootfs") == 0)
				rootfs_part = i;
		}
	} else {
		/* if the flash is 64k sectors, the kernel will reside at 0xb0030000
		   if the flash is 128k sectors, the kernel will reside at 0xb0040000 */
		ifxmips_partitions[1].size = lq_mtd->erasesize;
		ifxmips_partitions[2].offset = ifxmips_partitions[1].offset + lq_mtd->erasesize;
		parts = &ifxmips_partitions[0];
	}

	/* dynamic size detection only if rootfs-part follows kernel-part */
	if (kernel_part+1 == rootfs_part) {
		for (i=0;i<0xa0000;i+=0x10000)
		{
		uimage_size = find_uImage_size(&lq_map,i);
		if (uimage_size>0)
		    break;
		}
		parts[0].offset=0;
		parts[0].size=i-lq_mtd->erasesize;
		parts[1].offset=parts[0].size;
		parts[1].size=lq_mtd->erasesize;
		parts[kernel_part].offset=i;
		uimage_size &= ~(4096 -1);
		uimage_size += 4096;
		
		if (detect_squashfs_partition(&lq_map,parts[kernel_part].offset + uimage_size)) {
			printk(KERN_INFO "lq_mtd: found a squashfs following the uImage\n");
		}
		
		parts[kernel_part].size = uimage_size;
		parts[rootfs_part].offset = parts[kernel_part].offset + parts[kernel_part].size;
		parts[rootfs_part].size = ((lq_mtd->size >> 20) * 1024 * 1024) - parts[rootfs_part].offset;

		ifxmips_meta_partition.offset = parts[kernel_part].offset;
		ifxmips_meta_partition.size = parts[kernel_part].size + parts[rootfs_part].size;
	}

	if (err <= 0) {
			parts[3].size -= (lq_mtd->erasesize*3);
			parts[4].offset = lq_mtd->size - (lq_mtd->erasesize*3);
			parts[4].size = lq_mtd->erasesize;
			parts[5].offset = lq_mtd->size - (lq_mtd->erasesize);
			parts[5].size = lq_mtd->erasesize;
			parts[6].offset = 0;
			parts[6].size = lq_mtd->size;
			ifxmips_meta_partition.size -= lq_mtd->erasesize;
	}

	add_mtd_partitions(lq_mtd, parts, num_parts);
	add_mtd_partitions(lq_mtd, &ifxmips_meta_partition, 1);

	printk(KERN_INFO "lq_mtd: added %s flash with %dMB\n",
		lq_map.name, ((int)lq_mtd->size) >> 20);
	return 0;
}

static struct platform_driver lq_mtd_driver = {
	.probe = lq_mtd_probe,
	.driver = {
		.name = "lq_nor",
		.owner = THIS_MODULE,
	},
};

int __init
init_lq_mtd(void)
{
	int ret = platform_driver_register(&lq_mtd_driver);
	if (ret)
		printk(KERN_INFO "lq_nor: error registering platfom driver");
	return ret;
}

module_init(init_lq_mtd);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("John Crispin <blogic@openwrt.org>");
MODULE_DESCRIPTION("Lantiq SoC NOR");
