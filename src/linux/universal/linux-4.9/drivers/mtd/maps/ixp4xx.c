/*
 * $Id: ixp4xx.c,v 1.13 2005/11/16 16:23:21 dvrabel Exp $
 *
 * drivers/mtd/maps/ixp4xx.c
 *
 * MTD Map file for IXP4XX based systems. Please do not make per-board
 * changes in here. If your board needs special setup, do it in your
 * platform level code in arch/arm/mach-ixp4xx/board-setup.c
 *
 * Original Author: Intel Corporation
 * Maintainer: Deepak Saxena <dsaxena@mvista.com>
 *
 * Copyright (C) 2002 Intel Corporation
 * Copyright (C) 2003-2004 MontaVista Software, Inc.
 *
 */

#include <linux/err.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/ioport.h>
#include <linux/device.h>
#include <linux/platform_device.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>
#include "../mtdcore.h"
#include <linux/vmalloc.h>
#include <linux/magic.h>

#include <asm/io.h>
#include <asm/mach/flash.h>

#include <linux/reboot.h>

/*
 * Read/write a 16 bit word from flash address 'addr'.
 *
 * When the cpu is in little-endian mode it swizzles the address lines
 * ('address coherency') so we need to undo the swizzling to ensure commands
 * and the like end up on the correct flash address.
 *
 * To further complicate matters, due to the way the expansion bus controller
 * handles 32 bit reads, the byte stream ABCD is stored on the flash as:
 *     D15    D0
 *     +---+---+
 *     | A | B | 0
 *     +---+---+
 *     | C | D | 2
 *     +---+---+
 * This means that on LE systems each 16 bit word must be swapped. Note that
 * this requires CONFIG_MTD_CFI_BE_BYTE_SWAP to be enabled to 'unswap' the CFI
 * data and other flash commands which are always in D7-D0.
 */
#ifndef __ARMEB__
#ifndef CONFIG_MTD_CFI_BE_BYTE_SWAP
#error CONFIG_MTD_CFI_BE_BYTE_SWAP required
#endif

static inline u16 flash_read16(void __iomem * addr)
{
	return be16_to_cpu(__raw_readw((void __iomem *)((unsigned long)addr ^ 0x2)));
}

static inline void flash_write16(u16 d, void __iomem * addr)
{
	__raw_writew(cpu_to_be16(d), (void __iomem *)((unsigned long)addr ^ 0x2));
}

#define	BYTE0(h)	((h) & 0xFF)
#define	BYTE1(h)	(((h) >> 8) & 0xFF)

#else

static inline u16 flash_read16(const void __iomem * addr)
{
	return __raw_readw(addr);
}

static inline void flash_write16(u16 d, void __iomem * addr)
{
	__raw_writew(d, addr);
}

#define	BYTE0(h)	(((h) >> 8) & 0xFF)
#define	BYTE1(h)	((h) & 0xFF)
#endif

static map_word ixp4xx_read16(struct map_info *map, unsigned long ofs)
{
	map_word val;
	val.x[0] = flash_read16(map->virt + ofs);
	return val;
}

/*
 * The IXP4xx expansion bus only allows 16-bit wide acceses
 * when attached to a 16-bit wide device (such as the 28F128J3A),
 * so we can't just memcpy_fromio().
 */
static void ixp4xx_copy_from(struct map_info *map, void *to, unsigned long from, ssize_t len)
{
	u8 *dest = (u8 *)to;
	void __iomem *src = map->virt + from;

	if (len <= 0)
		return;

	if (from & 1) {
		*dest++ = BYTE1(flash_read16(src));
		src++;
		--len;
	}

	while (len >= 2) {
		u16 data = flash_read16(src);
		*dest++ = BYTE0(data);
		*dest++ = BYTE1(data);
		src += 2;
		len -= 2;
	}

	if (len > 0)
		*dest++ = BYTE0(flash_read16(src));
}

/*
 * Unaligned writes are ignored, causing the 8-bit
 * probe to fail and proceed to the 16-bit probe (which succeeds).
 */
static void ixp4xx_probe_write16(struct map_info *map, map_word d, unsigned long adr)
{
	if (!(adr & 1))
		flash_write16(d.x[0], map->virt + adr);
}

/*
 * Fast write16 function without the probing check above
 */
static void ixp4xx_write16(struct map_info *map, map_word d, unsigned long adr)
{
	flash_write16(d.x[0], map->virt + adr);
}

struct ixp4xx_flash_info {
	struct mtd_info *mtd;
	struct map_info map;
	struct mtd_partition *partitions;
	struct resource *res;
};

static const char *const probes[] = { "RedBoot", "cmdlinepart", NULL };

struct fis_image_desc {
	unsigned char name[16];	// Null terminated name
	unsigned long flash_base;	// Address within FLASH of image
	unsigned long mem_base;	// Address in memory where it executes
	unsigned long size;	// Length of image
	unsigned long entry_point;	// Execution entry point
	unsigned long data_length;	// Length of actual data
	unsigned char _pad[256 - (16 + 7 * sizeof(unsigned long))];
	unsigned long desc_cksum;	// Checksum over image descriptor
	unsigned long file_cksum;	// Checksum over image data
};
#define BOOT 0
#define LINUX 0x1
#define ROOTFS 0x2
#define DDWRT 0x3
#define MAMPF 0x4
#define NVRAM 0x5
#define FIS 0x6
#define DIR 0x7

static struct mtd_partition dir_parts[] = {
      { name: "RedBoot", offset: 0, size:0x30000, },
				//, mask_flags: MTD_WRITEABLE, },
      { name: "linux", offset: 0x30000, size:0x390000, },
      { name: "rootfs", offset: 0x0, size:0x2b0000, },
				//must be detected
      { name: "ddwrt", offset: 0x0, size:0x2b0000, },
				//must be detected
      { name: "mampf", offset: 0x3d0000, size:0x10000, },
      { name: "nvram", offset: 0x3d0000, size:0x10000, },
      { name: "FIS directory", offset: 0x3e0000, size:0x10000, },
      { name: "RedBoot config", offset: 0x3e0000, size:0x10000, },
      { name: "fullflash", offset: 0x3e0000, size:0x10000, },
      { name:NULL, },
};

/*static struct mtd_partition ixp_parts[] = {
        { name: "boot", offset: 0, size: 0x80000, },//, mask_flags: MTD_WRITEABLE, },
        { name: "linux", offset: 0x80000, size: 0x390000, },
        { name: "rootfs", offset: 0x140000, size: 0x2b0000,},
        { name: "mampf", offset: 0x140000, size: 0x20000,},
        { name: "nvram", offset: 0x3f0000, size: 0x20000, },
        { name: NULL, },
};
*/
static int ixp4xx_flash_remove(struct platform_device *dev)
{
	struct flash_platform_data *plat = dev_get_platdata(&dev->dev);
	struct ixp4xx_flash_info *info = platform_get_drvdata(dev);

	if (!info)
		return 0;

	if (info->mtd) {
		del_mtd_partitions(info->mtd);
		map_destroy(info->mtd);
	}
	kfree(info->partitions);

	if (plat->exit)
		plat->exit();

	return 0;
}

/*static struct mtd_partition ap71_parts[] = {
        { name: "RedBoot", offset: 0, size: 0x80000, },//, mask_flags: MTD_WRITEABLE, },
        { name: "linux", offset: 0x80000, size: 0xd20000, },
        { name: "ramdisk", offset: 0x280000, size: 0xd00000,},
        { name: "mampf", offset: 0xf80000, size: 0x20000, },
        { name: "nvram", offset: 0xfa0000, size: 0x20000, },
        { name: "RedBoot config", offset: 0xfc0000, size: 0x01000, },
        { name: "FIS directory", offset: 0xfe0000, size: 0x20000, },
        { name: NULL, },
};*/

#define AP71_MTDP_NUM 7

static int ixp4xx_flash_probe(struct platform_device *dev)
{
	struct flash_platform_data *plat = dev_get_platdata(&dev->dev);
	struct ixp4xx_flash_info *info;
	int result = -1;
	struct mtd_info *mtd;
	int err = -1;
	int offset = 0;
	char *buf;
	unsigned char *p;
	struct fis_image_desc *fis;

	if (!plat)
		return -ENODEV;

	if (plat->init) {
		err = plat->init();
		if (err)
			return err;
	}

	info = devm_kzalloc(&dev->dev, sizeof(struct ixp4xx_flash_info), GFP_KERNEL);
	if (!info) {
		err = -ENOMEM;
		goto Error;
	}
	memset(info, 0, sizeof(struct ixp4xx_flash_info));

	platform_set_drvdata(dev, info);

	/*
	 * Tell the MTD layer we're not 1:1 mapped so that it does
	 * not attempt to do a direct access on us.
	 */
	info->map.phys = NO_XIP;
	info->map.size = dev->resource->end - dev->resource->start + 1;

	/*
	 * We only support 16-bit accesses for now. If and when
	 * any board use 8-bit access, we'll fixup the driver to
	 * handle that.
	 */
	info->map.bankwidth = 2;
	info->map.name = dev_name(&dev->dev);
	info->map.read = ixp4xx_read16;
	info->map.write = ixp4xx_probe_write16;
	info->map.copy_from = ixp4xx_copy_from;
	printk(KERN_ERR "try to reserve 0x%08X with size of 0x%08X\n", dev->resource->start, dev->resource->end - dev->resource->start + 1);
	info->map.virt = devm_ioremap_resource(&dev->dev, dev->resource);
	if (IS_ERR(info->map.virt)) {
		err = PTR_ERR(info->map.virt);
	}

	info->mtd = do_map_probe(plat->map_name, &info->map);
	if (!info->mtd) {
		printk(KERN_ERR "IXP4XXFlash: map_probe failed\n");
		err = -ENXIO;
		goto Error;
	}
	info->mtd->dev.parent = &dev->dev;

	/* Use the fast version */
	info->map.write = ixp4xx_write16;
	printk(KERN_INFO "scanning for root partition\n");
	mtd = info->mtd;
	offset = 0;
	buf = info->map.virt;
	int foundconfig = 0;
	int foundfis = 0;
	int filesyssize = 0;
	int tmplen;
	int erasesize = mtd->erasesize;
#ifdef CONFIG_TONZE
	erasesize = 0x20000;
#endif
#ifdef CONFIG_NOP8670
	erasesize = 0x20000;
#endif
	while ((offset + erasesize) < mtd->size) {
		printk(KERN_INFO "[0x%08X]\r", offset);
		if (*((__u32 *)buf) == SQUASHFS_MAGIC_SWAP || *((__u16 *)buf) == 0x1985) {
			struct squashfs_super_block *sb = (struct squashfs_super_block *)buf;
			if (*((__u16 *)buf) != 0x1985) {
				filesyssize = le64_to_cpu(sb->bytes_used);
				tmplen = offset + filesyssize;
				tmplen += (erasesize - 1);
				tmplen &= ~(erasesize - 1);
				filesyssize = tmplen - offset;
			}

			printk(KERN_INFO "\nfound squashfs/jffs2 at %X\n", offset);
			dir_parts[ROOTFS].offset = offset;
			//detect now compex board
			//printk(KERN_INFO "id = %s\n",(char*)(info->map.virt+0x23d6));
			if (!strncmp((char *)(info->map.virt + 0x23d6), "myloram.bin", 11)) {
				printk(KERN_INFO "Compex WP188 detected!\n");
				dir_parts[BOOT].size = 0x40000;
				dir_parts[BOOT].offset = 0;
				dir_parts[DIR].size = 0x1000;
				dir_parts[DIR].offset = mtd->size - 0x1000;
				dir_parts[FIS].size = erasesize;
				dir_parts[FIS].offset = mtd->size - erasesize;

				long highest = dir_parts[FIS].offset;
#ifdef CONFIG_TONZE
				highest &= ~(erasesize - 1);
#endif
				dir_parts[ROOTFS].size = (highest - (erasesize * 2)) - dir_parts[ROOTFS].offset;
				dir_parts[MAMPF].offset = highest - erasesize * 2;
				dir_parts[MAMPF].size = erasesize;
				dir_parts[NVRAM].offset = highest - erasesize;
				dir_parts[NVRAM].size = erasesize;
				dir_parts[LINUX].offset = 0x40000;
				dir_parts[LINUX].size = dir_parts[ROOTFS].offset - dir_parts[LINUX].offset + dir_parts[ROOTFS].size;
				goto def;
			}

			//now scan for linux offset
#ifdef CONFIG_NOP8670
			p = (unsigned char *)(info->map.virt + mtd->size - erasesize);
#elif CONFIG_TONZE
			p = (unsigned char *)(info->map.virt + mtd->size - 0x8000);
#else
			p = (unsigned char *)(info->map.virt + mtd->size - erasesize);
#endif
			fis = (struct fis_image_desc *)p;
			printk(KERN_INFO "scan redboot from %p\n", fis);
			while (1) {
				if (fis->name[BOOT] == 0xff) {
					goto def;
				}
				if (!strncmp(fis->name, "RedBoot", 7) && strncmp(fis->name, "RedBoot config", 14)) {
					printk(KERN_INFO "found RedBoot partition at [0x%08lX]\n", fis->flash_base);
					dir_parts[BOOT].size = fis->size;
				}
				if (!strncmp(fis->name, "RedBoot config", 14)) {
					printk(KERN_INFO "found RedBoot config partition at [0x%08lX]\n", fis->flash_base);
					dir_parts[DIR].size = mtd->erasesize;
					dir_parts[DIR].offset = fis->flash_base & (mtd->size - 1);
#ifdef CONFIG_TONZE
					dir_parts[DIR].offset &= ~(erasesize - 1);
#endif
					if (foundfis) {
						long highest = dir_parts[NVRAM].offset;
						if (dir_parts[FIS].offset < highest)
							highest = dir_parts[FIS].offset;
#ifdef CONFIG_TONZE
						highest &= ~(erasesize - 1);
#endif
						dir_parts[ROOTFS].size = (highest - (erasesize * 2)) - dir_parts[ROOTFS].offset;
						dir_parts[MAMPF].offset = highest - erasesize * 2;
						dir_parts[MAMPF].size = erasesize;
						dir_parts[NVRAM].offset = highest - erasesize;
						dir_parts[NVRAM].size = erasesize;
					}
					foundconfig = 1;
				}
				if (!strncmp(fis->name, "linux", 5) || !strncmp(fis->name, "vmlinux", 7) || !strncmp(fis->name, "kernel", 6)) {
					printk(KERN_INFO "found linux partition at [0x%08lX]\n", fis->flash_base);
					dir_parts[LINUX].offset = fis->flash_base & (mtd->size - 1);
					dir_parts[LINUX].size = dir_parts[ROOTFS].offset - dir_parts[LINUX].offset + dir_parts[ROOTFS].size;
				}
				if (!strncmp(fis->name, "FIS directory", 13)) {
					printk(KERN_INFO "found config partition at [0x%08lX]\n", fis->flash_base);
					dir_parts[FIS].offset = (fis->flash_base & (mtd->size - 1));
					dir_parts[FIS].size = mtd->erasesize;
#ifdef CONFIG_TONZE
					dir_parts[FIS].offset &= ~(erasesize - 1);
#endif
					if (foundconfig) {
						long highest = dir_parts[FIS].offset;
						if (dir_parts[DIR].offset < highest)
							highest = dir_parts[DIR].offset;
#ifdef CONFIG_TONZE
						highest &= ~(erasesize - 1);
#endif
						dir_parts[ROOTFS].size = (highest - (erasesize * 2)) - dir_parts[ROOTFS].offset;
						dir_parts[MAMPF].offset = highest - erasesize * 2;
						dir_parts[MAMPF].size = erasesize;
						dir_parts[NVRAM].offset = highest - erasesize;
						dir_parts[NVRAM].size = erasesize;
					}
					foundfis = 1;
				}
				p += sizeof(struct fis_image_desc);
				fis = (struct fis_image_desc *)p;
			}
			break;
		}
#ifdef CONFIG_TONZE
		offset += 0x1000;
		buf += 0x1000;
#else
		offset += erasesize;
		buf += erasesize;
#endif
	}
      def:;
	info->partitions = dir_parts;
	if (filesyssize) {
		dir_parts[ROOTFS].size = filesyssize;
	}
	dir_parts[DDWRT].offset = dir_parts[ROOTFS].offset + dir_parts[ROOTFS].size;
	dir_parts[DDWRT].size = dir_parts[MAMPF].offset - dir_parts[DDWRT].offset;

	dir_parts[8].offset = 0;
	dir_parts[8].size = mtd->size;

	err = add_mtd_partitions(mtd, dir_parts, 9);

/*#ifndef CONFIG_NOP8670
	err = parse_mtd_partitions(info->mtd, probes, &info->partitions, dev->resource->start);
#else
	info->partitions=ap71_parts;
	err=AP71_MTDP_NUM;    
#endif
	if (err > 0) {
		err = add_mtd_partitions(info->mtd, info->partitions, err);
		if(err)
			printk(KERN_ERR "Could not parse partitions\n");
	}
*/
	if (err)
		goto Error;

	return 0;

Error:
	ixp4xx_flash_remove(dev);
	return err;
}

static struct platform_driver ixp4xx_flash_driver = {
	.probe = ixp4xx_flash_probe,
	.remove = ixp4xx_flash_remove,
	.driver = {
		   .name = "IXP4XX-Flash",
		    },
};

module_platform_driver(ixp4xx_flash_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MTD map driver for Intel IXP4xx systems");
MODULE_AUTHOR("Deepak Saxena");
