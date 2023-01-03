/*
 *  Copyright (C) Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/kmod.h>
#include <linux/root_dev.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#include <linux/byteorder/generic.h>
#include <linux/squashfs_fs.h>

#define PFX	"trxsplit: "

#define TRX_MAGIC	0x30524448	/* "HDR0" */
#define TRX_VERSION	1
#define TRX_MAX_LEN	0x3A0000
#define TRX_NO_HEADER	0x1	/* do not write TRX header */
#define TRX_GZ_FILES	0x2     /* contains individual gzip files */
#define TRX_MAX_OFFSET	3
#define TRX_MIN_KERNEL_SIZE	(256 * 1024)

struct trx_header {
	u32 magic;	/* "HDR0" */
	u32 len;	/* Length of file including header */
	u32 crc32;	/* 32-bit CRC from flag_version to end of file */
	u32 flag_version; /* 0:15 flags, 16:31 version */
	u32 offsets[TRX_MAX_OFFSET]; /* Offsets of partitions */
};

#define TRX_ALIGN	0x1000

static int trx_nr_parts;
static unsigned long trx_offset;
static struct mtd_info *trx_mtd;
static struct mtd_partition trx_parts[TRX_MAX_OFFSET];
static struct trx_header trx_hdr;
static struct mtd_partition boot;

static int trxsplit_refresh_partitions(struct mtd_info *mtd);

static int trxsplit_checktrx(struct mtd_info *mtd, unsigned long offset)
{
	size_t retlen;
	int err;

	err = mtd_read(mtd, offset, sizeof(trx_hdr), &retlen, (void *)&trx_hdr);
	if (err) {
		printk(KERN_ALERT PFX "unable to read from '%s'\n", mtd->name);
		goto err_out;
	}

	if (retlen != sizeof(trx_hdr)) {
		printk(KERN_ALERT PFX "reading failed on '%s'\n", mtd->name);
		goto err_out;
	}

	trx_hdr.magic = le32_to_cpu(trx_hdr.magic);
	trx_hdr.len = le32_to_cpu(trx_hdr.len);
	trx_hdr.crc32 = le32_to_cpu(trx_hdr.crc32);
	trx_hdr.flag_version = le32_to_cpu(trx_hdr.flag_version);
	trx_hdr.offsets[0] = le32_to_cpu(trx_hdr.offsets[0]);
	trx_hdr.offsets[1] = le32_to_cpu(trx_hdr.offsets[1]);
	trx_hdr.offsets[2] = le32_to_cpu(trx_hdr.offsets[2]);

	/* sanity checks */
	if (trx_hdr.magic != TRX_MAGIC)
		goto err_out;

	if (trx_hdr.len > mtd->size - offset)
		goto err_out;

	/* TODO: add crc32 checking too? */

	return 0;

err_out:
	return -1;
}

static void trxsplit_findtrx(struct mtd_info *mtd)
{
	unsigned long offset;
	int err;

	printk(KERN_INFO PFX "searching TRX header in '%s'\n", mtd->name);

	err = 0;
	for (offset = 0; offset < mtd->size; offset ++) {
		err = trxsplit_checktrx(mtd, offset);
		if (err == 0)
			break;
	}

	if (err)
		return;

	printk(KERN_INFO PFX "TRX header found at 0x%lX\n", offset);

	trx_mtd = mtd;
	trx_offset = offset;
}


static void trxsplit_create_partitions(struct mtd_info *mtd)
{
	struct mtd_partition *part = trx_parts;
	unsigned char buffer[32];
	int err;
	int i;
	int retlen;
	trx_parts[0].offset = 0x20000;
	trx_parts[0].size = (mtd->size-mtd->erasesize) - trx_parts[0].offset;
	int offset = 0;

			    char *buf = &buffer[0];
			    while((offset+mtd->erasesize)<mtd->size)
			    {
			    mtd_read(mtd, offset,32, &retlen, buf);

//			    printk(KERN_INFO "scanning squashfs at %X\n",offset);
			    if (*((__u32 *) buf) == SQUASHFS_MAGIC)
				    {
				    	printk(KERN_INFO "\nfound squashfs at %X\n",offset);
					trx_parts[1].offset=offset;					
					break;
				    } 
//			    offset+=mtd->erasesize;
//			    offset++;
			    offset+=4096; //scan in smaller blocks
			    }
	trx_parts[1].size = (mtd->size-mtd->erasesize) - trx_parts[1].offset;
	i = 0;


	part = &trx_parts[i];
	part->name = "linux";
	i++;

	part = &trx_parts[i];
	part->name = "rootfs";
	i++;
	

//	for (i=0;i<3;i++)
//	    printk(KERN_INFO "partition %s: offset %08X, size %08X\n",trx_parts[i].name,trx_parts[i].offset,trx_parts[i].size);
	
	/*
	detect OSBridge 
	*/
	mtd_read(mtd, 0xff90-2,32, &retlen, (void *)&trx_hdr);
	int bootmul=2;
	if (strncmp(buf,"OSBRiDGE 5XLi",13)==0)
	    {
	    printk(KERN_INFO "found osbridge 5XLi");
	    bootmul=1;
	    trx_parts[0].offset = 0x20000;
	    trx_parts[0].size = (mtd->size-mtd->size-mtd->erasesize) - trx_parts[0].offset;
	    }

	part = &trx_parts[i];
	part->name = "nvram";
	part->offset = mtd->size-mtd->erasesize;
	part->size = mtd->erasesize;
	
	trx_nr_parts++;


	boot.name="boot";
	boot.offset=0;
#ifdef ADM5120_MACH_WP54
	boot.size=0x10000;
#else
	boot.size=mtd->erasesize*bootmul;
#endif
	err = mtd_device_register(mtd, &boot, 1);
//	for (i=0;i<3;i++)
//	    printk(KERN_INFO "partition %s: offset %08X, size %08X\n",trx_parts[i].name,trx_parts[i].offset,trx_parts[i].size);
	
	err = mtd_device_register(mtd, trx_parts, 3);
	if (err) {
		printk(KERN_ALERT PFX "adding TRX partitions failed\n");
		return;
	}

	mtd->refresh_device = trxsplit_refresh_partitions;
}

static int trxsplit_refresh_partitions(struct mtd_info *mtd)
{
	printk(KERN_INFO PFX "refreshing TRX partitions in '%s' (%d,%d)\n",
		mtd->name, MTD_BLOCK_MAJOR, mtd->index);

	/* remove old partitions */
	mtd_device_unregister(mtd);

	trxsplit_findtrx(mtd);
	if (!trx_mtd)
		goto err;

	trxsplit_create_partitions(trx_mtd);
	return 1;

err:
	return 0;
}

static void __init trxsplit_add_mtd(struct mtd_info *mtd)
{
	if (mtd->type != MTD_NORFLASH) {
		printk(KERN_INFO PFX "'%s' is not a NOR flash, skipped\n",
				mtd->name);
		return;
	}

	if (!trx_mtd)
		trxsplit_findtrx(mtd);
}

static void __init trxsplit_remove_mtd(struct mtd_info *mtd)
{
	/* nothing to do */
}

static struct mtd_notifier trxsplit_notifier __initdata = {
	.add	= trxsplit_add_mtd,
	.remove	= trxsplit_remove_mtd,
};

static void __init trxsplit_scan(void)
{
	register_mtd_user(&trxsplit_notifier);
	unregister_mtd_user(&trxsplit_notifier);
}

static int __init trxsplit_init(void)
{
	trxsplit_scan();

	if (trx_mtd) {
		printk(KERN_INFO PFX "creating TRX partitions in '%s' "
			"(%d,%d)\n", trx_mtd->name, MTD_BLOCK_MAJOR,
			trx_mtd->index);
		trxsplit_create_partitions(trx_mtd);
	}

	return 0;
}

late_initcall(trxsplit_init);
