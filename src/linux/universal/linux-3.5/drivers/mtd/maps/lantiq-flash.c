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
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/cfi.h>
#include <linux/platform_device.h>
#include <linux/squashfs_fs.h>
#include <linux/mtd/physmap.h>

#include "../mtdcore.h"

#include <lantiq_soc.h>
#include <lantiq_platform.h>

/*
 * The NOR flash is connected to the same external bus unit (EBU) as PCI.
 * To make PCI work we need to enable the endianness swapping for the address
 * written to the EBU. This endianness swapping works for PCI correctly but
 * fails for attached NOR devices. To workaround this we need to use a complex
 * map. The workaround involves swapping all addresses whilst probing the chip.
 * Once probing is complete we stop swapping the addresses but swizzle the
 * unlock addresses to ensure that access to the NOR device works correctly.
 */

static struct mtd_partition ifxmips_partitions[] = {
	{
		.name = "uboot",
		.offset = 0x00000000,
		.size = 0x00020000,
	},
	{
		.name = "uboot-env",
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
	{
		.name = "ddwrt",
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
	printk("uImage size %lX\n",temp + 0x40);
	return temp + 0x40;
}

static int
detect_squashfs_partition(struct map_info *map, unsigned long offset)
{
	unsigned long temp;
	map->copy_from(map, &temp, offset, 4);
	printk(KERN_INFO "Magic = %08lX\n", temp);
	return le32_to_cpu(temp) == SQUASHFS_MAGIC_SWAP || le32_to_cpu(temp) == SQUASHFS_MAGIC;
}

#ifdef CONFIG_AR9
#define RESERVE 6
#else
#define RESERVE 3
#endif

enum {
	LTQ_NOR_PROBING,
	LTQ_NOR_NORMAL
};

struct ltq_mtd {
	struct resource *res;
	struct mtd_info *mtd;
	struct map_info *map;
};

static char ltq_map_name[] = "ltq_nor";
static const char *ltq_probe_types[] __devinitconst = { "cmdlinepart", NULL };

static map_word
ltq_read16(struct map_info *map, unsigned long adr)
{
	unsigned long flags;
	map_word temp;

	if (map->map_priv_1 == LTQ_NOR_PROBING)
		adr ^= 2;
	spin_lock_irqsave(&ebu_lock, flags);
	temp.x[0] = *(u16 *)(map->virt + adr);
	spin_unlock_irqrestore(&ebu_lock, flags);
	return temp;
}

static void
ltq_write16(struct map_info *map, map_word d, unsigned long adr)
{
	unsigned long flags;

	if (map->map_priv_1 == LTQ_NOR_PROBING)
		adr ^= 2;
	spin_lock_irqsave(&ebu_lock, flags);
	*(u16 *)(map->virt + adr) = d.x[0];
	spin_unlock_irqrestore(&ebu_lock, flags);
}

/*
 * The following 2 functions copy data between iomem and a cached memory
 * section. As memcpy() makes use of pre-fetching we cannot use it here.
 * The normal alternative of using memcpy_{to,from}io also makes use of
 * memcpy() on MIPS so it is not applicable either. We are therefore stuck
 * with having to use our own loop.
 */
static void
ltq_copy_from(struct map_info *map, void *to,
	unsigned long from, ssize_t len)
{
	unsigned char *f = (unsigned char *)map->virt + from;
	unsigned char *t = (unsigned char *)to;
	unsigned long flags;

	spin_lock_irqsave(&ebu_lock, flags);
	while (len--)
		*t++ = *f++;
	spin_unlock_irqrestore(&ebu_lock, flags);
}

static void
ltq_copy_to(struct map_info *map, unsigned long to,
	const void *from, ssize_t len)
{
	unsigned char *f = (unsigned char *)from;
	unsigned char *t = (unsigned char *)map->virt + to;
	unsigned long flags;

	spin_lock_irqsave(&ebu_lock, flags);
	while (len--)
		*t++ = *f++;
	spin_unlock_irqrestore(&ebu_lock, flags);
}


static int __init
ltq_mtd_probe(struct platform_device *pdev)
{
	struct physmap_flash_data *ltq_mtd_data = dev_get_platdata(&pdev->dev);
	struct ltq_mtd *ltq_mtd;
	struct mtd_partition *parts = NULL;
	struct resource *res;
	int nr_parts = 0;
	unsigned long uimage_size;
	struct cfi_private *cfi;
	int err,i;
	int kernel_part = 2, rootfs_part = 3;
	int num_parts = ARRAY_SIZE(ifxmips_partitions);

	ltq_mtd = kzalloc(sizeof(struct ltq_mtd), GFP_KERNEL);
	platform_set_drvdata(pdev, ltq_mtd);

	ltq_mtd->res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!ltq_mtd->res) {
		dev_err(&pdev->dev, "failed to get memory resource");
		err = -ENOENT;
		goto err_out;
	}

	res = devm_request_mem_region(&pdev->dev, ltq_mtd->res->start,
		resource_size(ltq_mtd->res), dev_name(&pdev->dev));
	if (!ltq_mtd->res) {
		dev_err(&pdev->dev, "failed to request mem resource");
		err = -EBUSY;
		goto err_out;
	}

	ltq_mtd->map = kzalloc(sizeof(struct map_info), GFP_KERNEL);
	ltq_mtd->map->phys = res->start;
	ltq_mtd->map->size = resource_size(res);
	ltq_mtd->map->virt = devm_ioremap_nocache(&pdev->dev,
				ltq_mtd->map->phys, ltq_mtd->map->size);
	if (!ltq_mtd->map->virt) {
		dev_err(&pdev->dev, "failed to ioremap!\n");
		err = -ENOMEM;
		goto err_free;
	}

	ltq_mtd->map->name = ltq_map_name;
	ltq_mtd->map->bankwidth = 2;
	ltq_mtd->map->read = ltq_read16;
	ltq_mtd->map->write = ltq_write16;
	ltq_mtd->map->copy_from = ltq_copy_from;
	ltq_mtd->map->copy_to = ltq_copy_to;

	ltq_mtd->map->map_priv_1 = LTQ_NOR_PROBING;
	ltq_mtd->mtd = do_map_probe("cfi_probe", ltq_mtd->map);
	ltq_mtd->map->map_priv_1 = LTQ_NOR_NORMAL;

	if (!ltq_mtd->mtd) {
		dev_err(&pdev->dev, "probing failed\n");
		err = -ENXIO;
		goto err_free;
	}

	ltq_mtd->mtd->owner = THIS_MODULE;

	cfi = ltq_mtd->map->fldrv_priv;
	cfi->addr_unlock1 ^= 1;
	cfi->addr_unlock2 ^= 1;

	nr_parts = parse_mtd_partitions(ltq_mtd->mtd, ltq_probe_types, &parts, 0);
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
		ifxmips_partitions[1].size = ltq_mtd->mtd->erasesize;
		ifxmips_partitions[2].offset = ifxmips_partitions[1].offset + ltq_mtd->mtd->erasesize;
		parts = &ifxmips_partitions[0];
	}

	/* dynamic size detection only if rootfs-part follows kernel-part */
	if (kernel_part+1 == rootfs_part) {
		for (i=0;i<0xa0000;i+=0x10000)
		{
		uimage_size = find_uImage_size(ltq_mtd->map,i);
		if (uimage_size>0)
		    break;
		}
		parts[0].offset=0;
		parts[0].size=i-ltq_mtd->mtd->erasesize;
		parts[1].offset=parts[0].size;
		parts[1].size=ltq_mtd->mtd->erasesize;
		parts[kernel_part].offset=i;
		uimage_size &= ~(4096 -1);
		uimage_size += 4096;
		
		if (detect_squashfs_partition(ltq_mtd->map,parts[kernel_part].offset + uimage_size)) {
			printk(KERN_INFO "ltq_mtd: found a squashfs following the uImage\n");
		}
		
		parts[kernel_part].size = uimage_size;
		parts[rootfs_part].offset = parts[kernel_part].offset + parts[kernel_part].size;
		struct squashfs_super_block sb;
		ltq_mtd->map->copy_from(ltq_mtd->map, &sb, parts[rootfs_part].offset, sizeof(struct squashfs_super_block));
		int jffsoffset = le64_to_cpu(sb.bytes_used);
		jffsoffset += parts[rootfs_part].offset;
		jffsoffset += (ltq_mtd->mtd->erasesize - 1);
		jffsoffset &= ~(ltq_mtd->mtd->erasesize - 1);
		parts[rootfs_part].size = jffsoffset - parts[rootfs_part].offset;
		parts[7].offset = jffsoffset;
		parts[7].size = (ltq_mtd->mtd->size - (ltq_mtd->mtd->erasesize*RESERVE)) - jffsoffset;
		ifxmips_meta_partition.offset = parts[kernel_part].offset;
		ifxmips_meta_partition.size = parts[kernel_part].size + parts[rootfs_part].size + parts[7].size;
	}

	if (err <= 0) {
//			parts[3].size -= (ltq_mtd->erasesize*3);
			parts[4].offset = ltq_mtd->mtd->size - (ltq_mtd->mtd->erasesize*RESERVE);
			parts[4].size = ltq_mtd->mtd->erasesize;
			parts[5].offset = ltq_mtd->mtd->size - (ltq_mtd->mtd->erasesize);
			parts[5].size = ltq_mtd->mtd->erasesize;
			parts[6].offset = 0;
			parts[6].size = ltq_mtd->mtd->size;
//			ifxmips_meta_partition.size -= lq_mtd->erasesize;
	}

	add_mtd_partitions(ltq_mtd->mtd, parts, num_parts);
	add_mtd_partitions(ltq_mtd->mtd, &ifxmips_meta_partition, 1);

	return 0;

err_destroy:
	map_destroy(ltq_mtd->mtd);
err_free:
	kfree(ltq_mtd->map);
err_out:
	kfree(ltq_mtd);
	return err;
}

static int __devexit
ltq_mtd_remove(struct platform_device *pdev)
{
	struct ltq_mtd *ltq_mtd = platform_get_drvdata(pdev);

	if (ltq_mtd) {
		if (ltq_mtd->mtd) {
			mtd_device_unregister(ltq_mtd->mtd);
			map_destroy(ltq_mtd->mtd);
		}
		kfree(ltq_mtd->map);
		kfree(ltq_mtd);
	}
	return 0;
}

int ra_mtd_read(int num,int from, int len, u_char *buf)
{
	int ret;
	size_t rdlen;
	struct mtd_info *mtd;
	printk(KERN_INFO "read ralink eeprom from %X with len %X to %p (device %d)\n",from,len,buf,num);
	mtd = get_mtd_device(NULL, num);

	ret = mtd_read(mtd, from, len, &rdlen, buf);
	if (rdlen != len)
		printk(KERN_EMERG "warning: ra_mtd_read: rdlen is not equal to len\n");

	put_mtd_device(mtd);
	return ret;
}
EXPORT_SYMBOL(ra_mtd_read);


static struct platform_driver ltq_mtd_driver = {
	.remove = __devexit_p(ltq_mtd_remove),
	.driver = {
		.name = "ltq_nor",
		.owner = THIS_MODULE,
	},
};

static int __init
init_ltq_mtd(void)
{
	int ret = platform_driver_probe(&ltq_mtd_driver, ltq_mtd_probe);

	if (ret)
		pr_err("ltq_nor: error registering platform driver");
	return ret;
}

static void __exit
exit_ltq_mtd(void)
{
	platform_driver_unregister(&ltq_mtd_driver);
}

module_init(init_ltq_mtd);
module_exit(exit_ltq_mtd);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("John Crispin <blogic@openwrt.org>");
MODULE_DESCRIPTION("Lantiq SoC NOR");
