/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2006-2007 Tomas Dlabac <tomas@dlabac.net>
*/

/*
 *
 *  Normal mappings of chips in physical memory
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <linux/magic.h>

#include <linux/mtd/partitions.h>
#define MTDID "ar531x"
#include "../mtdcore.h"

#define WINDOW_ADDR 0xbe000000
#define WINDOW_SIZE 0x00800000

/* These ought to be somewhere common... */

#define AR531X_FLASHCTL	0x18400000

#define FLASHCTL_MW	0x30000000	/* Memory width */
#define FLASHCTL_MWx8	0x00000000	/* Memory width x8 */
#define FLASHCTL_MWx16	0x10000000	/* Memory width x16 */
#define FLASHCTL_MWx32	0x20000000	/* Memory width x32 (not supported) */

#define sysRegRead(phys) __raw_readl(KSEG1ADDR(phys))

static struct mtd_info *mymtd;

static const char *probes[] = { "cmdlinepart", NULL };

struct map_info ar531x_map = {
      name:"ar531x",
      size:WINDOW_SIZE
};

static struct mtd_partition *mtd_parts = 0;
static int mtd_parts_nb = 0;

struct img_info {
	uint32_t lenght;
	uint32_t CRC;
};

static struct mtd_partition dir_parts[] = {
	{
	      name:"RedBoot",
	      size:0x30000,	/* 64KB */
	      offset:0,
	 }, {
	      name:"linux",
	      size:0x7A0000,
	      offset:0x30000,
	     }, {
	      name:"rootfs",
	      size:0x0,
	      offset:MTDPART_OFS_APPEND,
		 }, {
	      name: "ddwrt",
	      size: 0x0,
	      offset:MTDPART_OFS_APPEND,
		     }, {
	      name:	 "nvram",
	      size:	 0x10000,
	      offset:	 0x7D0000,
			 }, {
	      name:	     "FIS directory",
	      size:	     0x10000,
	      offset:	     0x7E0000,
			     }, {
	      name:		 "board_config",
	      size:		 0x10000,	/* 64KB */
	      offset:		 0x7F0000,
				 }
	, {
	      name:"fullflash",
	      size:0x800000,	/* 64KB */
	      offset:0x000000,
	   }
};

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

#define NUM_PARTITIONS	(sizeof(dir_parts)/sizeof(struct mtd_partition))

int __init init_ar531x(void)
{
	static const char *rom_probe_types[] =
	    { "cfi_probe", "jedec_probe", "map_rom", 0 };
	const char **type;
	unsigned int flashctl;
	void *buf;
	size_t retlen;
	int ret;
	unsigned char *p;
	struct fis_image_desc *fis;
	struct img_info *image_info;
	struct squashfs_super_block *sb;
	int len;
	size_t rootsize;

	/* This is nasty, but needed as the new AR2312-01 parts only
	 * have an 8 bit bus to the flash; others have 16
	 */

	flashctl = sysRegRead(AR531X_FLASHCTL) & FLASHCTL_MW;

	if (flashctl == FLASHCTL_MWx8)
		ar531x_map.bankwidth = 1;
	else if (flashctl == FLASHCTL_MWx16)
		ar531x_map.bankwidth = 2;
	else {
		printk(KERN_ERR "ar531x illegal flash buswidth (%#x)\n",
		       flashctl);
		return -ENXIO;
	}

//      printk(KERN_NOTICE "ar531x flash buswidth detected as %d\n",
//             ar531x_map.bankwidth);

	/*
	 * Must make sure that WINDOW_ADDR is in kseg1 (uncached)
	 */
	ar531x_map.map_priv_1 = (WINDOW_ADDR | KSEG1);

	ar531x_map.phys = WINDOW_ADDR;
	ar531x_map.virt = ioremap(WINDOW_ADDR, WINDOW_SIZE);

	if (!ar531x_map.virt) {
		printk("Failed to ioremap\n");
		return -EIO;
	}

	simple_map_init(&ar531x_map);

	mymtd = 0;
	type = rom_probe_types;
	for (; !mymtd && *type; type++) {
		mymtd = do_map_probe(*type, &ar531x_map);
	}
	if (mymtd) {
		mymtd->owner = THIS_MODULE;
//              add_mtd_device(mymtd);
		printk(KERN_NOTICE
		       "AR531x Flash device initialized: size 0x%x at 0x%x bankwidth 0x%x\n",
		       mymtd->size, WINDOW_ADDR, ar531x_map.bankwidth);

#ifdef CONFIG_MTD
		 {
			int offset = 0x0;
			unsigned char *buf = (unsigned char *)0xbe000000;
			while ((offset + mymtd->erasesize) < mymtd->size) {
				if (*((__u32 *)buf) == SQUASHFS_MAGIC_SWAP) {
					printk(KERN_INFO
					       "\nfound squashfs at %X\n",
					       offset);
					sb = (struct squashfs_super_block *)buf;

					dir_parts[2].offset = offset;
					len = le64_to_cpu(sb->bytes_used);
					len += (mymtd->erasesize - 1);
					len &= ~(mymtd->erasesize - 1);
					dir_parts[2].size = len;
					dir_parts[3].offset =
					    offset + dir_parts[2].size;
					dir_parts[3].size =
					    dir_parts[1].size -
					    (dir_parts[3].offset -
					     0x30000);
					dir_parts[6].offset =
					    mymtd->size - mymtd->erasesize;
					dir_parts[5].offset =
					    mymtd->size -
					    (mymtd->erasesize * 2);
					dir_parts[4].offset =
					    mymtd->size -
					    (mymtd->erasesize * 3);
					rootsize = dir_parts[4].offset - offset;	//size of rootfs aligned to nvram offset

					p = (unsigned char *)(0xbe000000 +
							      dir_parts[5].
							      offset);
					fis = (struct fis_image_desc *)p;
					while (1) {
						if (fis->name[0] == 0xff) {
							goto def;
						}
						if (!strcmp
						    (fis->name, "RedBoot")) {
							printk(KERN_INFO
							       "found RedBoot partition at [0x%08lX]\n",
							       fis->flash_base);
							dir_parts[0].size =
							    fis->size;
							dir_parts[7].offset = 0;
						}
						if (!strcmp(fis->name, "linux")
						    || !strncmp(fis->name,
								"vmlinux", 7)
						    || !strcmp(fis->name,
							       "kernel")) {
							printk(KERN_INFO
							       "found linux partition at [0x%08lX]\n",
							       fis->flash_base);
							dir_parts[1].offset =
							    fis->
							    flash_base & (mymtd->
									  size -
									  1);
							dir_parts[1].size =
							    (dir_parts[2].
							     offset -
							     dir_parts[1].
							     offset) + rootsize;
							dir_parts[7].size = mymtd->size;	// linux + nvram = phy size
						}
						p += sizeof(struct
							    fis_image_desc);
						fis =
						    (struct fis_image_desc *)p;
					}
					break;
				}
				offset += mymtd->erasesize;
				buf += mymtd->erasesize;
			}
			def:;
			if (NUM_PARTITIONS != 0) {
				printk(KERN_NOTICE
				       "Using ar531x DD-WRT partition definition\n");
				add_mtd_partitions(mymtd, dir_parts,
						   NUM_PARTITIONS);
			}

		}
#endif

		return 0;
	}
	iounmap((void *)ar531x_map.virt);
	return -ENXIO;
}

static void __exit cleanup_ar531x(void)
{
	if (mymtd) {
		del_mtd_device(mymtd);
		map_destroy(mymtd);
	}
	if (ar531x_map.virt) {
		iounmap((void *)ar531x_map.virt);
		ar531x_map.virt = 0;
	}
}

module_init(init_ar531x);
module_exit(cleanup_ar531x);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tomas Dlabac");
MODULE_DESCRIPTION("Atheros AR531x MTD map driver");
