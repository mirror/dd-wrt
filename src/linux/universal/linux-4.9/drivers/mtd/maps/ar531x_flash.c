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
#define WINDOW_SIZE CONFIG_MTD_AR531X_SIZE


/* These ought to be somewhere common... */

#define AR531X_FLASHCTL	0x18400000

#define FLASHCTL_MW	0x30000000      /* Memory width */
#define FLASHCTL_MWx8	0x00000000      /* Memory width x8 */
#define FLASHCTL_MWx16	0x10000000      /* Memory width x16 */
#define FLASHCTL_MWx32	0x20000000      /* Memory width x32 (not supported) */

#define sysRegRead(phys) __raw_readl(KSEG1ADDR(phys))


static struct mtd_info *mymtd;

static const char *probes []={"cmdlinepart",NULL};

struct map_info ar531x_map = {
	name: "ar531x",
	size: WINDOW_SIZE
};

#ifdef CONFIG_MTD
static struct mtd_partition *mtd_parts = 0;
static int                   mtd_parts_nb = 0;

struct img_info {
	uint32_t lenght;
	uint32_t CRC;
	};

static struct mtd_partition ar531x_partitions[] = {
        {
                name:           "bdata",
                size:           0x10000,		/* 64KB */
                offset:         0,
        }, {
                name:           "boot",
                size:           0x40000,		/* 256KB */
                offset:         MTDPART_OFS_APPEND,
        }, {
                name:           "linux",
                size:           0x0,	
                offset:         MTDPART_OFS_APPEND,
        }, {
                name:           "rootfs",
                size:           0x0,	
                offset:         MTDPART_OFS_APPEND,
        }, {
                name:           "nvram",
                size:           0x10000,	
                offset:         0x3e0000,
        }, {
                name:           "board_config",
                size:           0x10000,		/* 64KB */
                offset:         0x3f0000,
        }

};

#define NUM_PARTITIONS	(sizeof(ar531x_partitions)/sizeof(struct mtd_partition))
#endif



int __init init_ar531x(void)
{
	static const char *rom_probe_types[] = { "cfi_probe", "jedec_probe", "map_rom", 0 };
	const char **type;
	unsigned int flashctl;
	void *buf;
	size_t	retlen;
	int ret;
	struct img_info *image_info;
		

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

//	printk(KERN_NOTICE "ar531x flash buswidth detected as %d\n",
//	       ar531x_map.bankwidth);

	/*
	 * Must make sure that WINDOW_ADDR is in kseg1 (uncached)
	 */
	ar531x_map.map_priv_1 = (WINDOW_ADDR | KSEG1);

	ar531x_map.phys = WINDOW_ADDR;
	ar531x_map.virt = ioremap(WINDOW_ADDR,WINDOW_SIZE);
	
	if (!ar531x_map.virt) {
	    printk("Failed to ioremap\n");
	    return -EIO;
	}

	simple_map_init(&ar531x_map);

	mymtd = 0;
	type = rom_probe_types;
	for(; !mymtd && *type; type++) {
		mymtd = do_map_probe(*type, &ar531x_map);
	}
	if (mymtd) {
		mymtd->owner = THIS_MODULE;
		add_mtd_device(mymtd);
       		printk(KERN_NOTICE "AR531x Flash device initialized: size 0x%x at 0x%x bankwidth 0x%x\n",
	    	    mymtd->size, WINDOW_ADDR, ar531x_map.bankwidth);

#ifdef CONFIG_MTD
		mtd_parts_nb = parse_mtd_partitions(mymtd, probes, &mtd_parts,0);
		if (mtd_parts_nb > 0)
		{
			printk(KERN_NOTICE 
			       "Using command line partition definition\n");
			add_mtd_partitions (mymtd, mtd_parts, mtd_parts_nb);
		}

		else {
		
/*	Pokus o identifikaci openwrt firmware a jeho rozlozeni  */

		    buf=vmalloc(mymtd->erasesize);
			if (buf) {
			    ret=mtd_read(mymtd,0,mymtd->erasesize, &retlen, buf);
			    if (ret) 
				{
				vfree(buf);
				goto out;
				}
			    if (retlen != mymtd->erasesize) 
				{
				vfree(buf);
				goto out;
				}
	
			    if (strstr(buf+0x10,"CA804.SOB") || strstr(buf+0x10,"CE801.SOB") || strstr(buf+0x10,"OVISCA401") || strstr(buf+0x10,"OVISCE401") || strstr(buf+0x10,"RCAAO1") || strstr((char*)(0xbfc00010),"RDAT81.SOB")) {
				image_info = buf+0x56;
			    ar531x_partitions[2].size = mymtd->size - 0x70000;	/* Velikost kernelu */
			    int offset = 0x0;
			    vfree(buf);
			    char *buf = 0xbfc00000;
			    while((offset+mymtd->erasesize)<mymtd->size)
			    {
			    if (*((__u32 *) buf) == SQUASHFS_MAGIC_SWAP)
				    {
				    	printk(KERN_INFO "\nfound squashfs at %X\n",offset);
					ar531x_partitions[3].offset=offset;					
					ar531x_partitions[3].size = ar531x_partitions[2].size-(offset-0x50000);					
					ar531x_partitions[5].offset=mymtd->size-mymtd->erasesize; 
					ar531x_partitions[4].offset=mymtd->size-(mymtd->erasesize*2); 
					break;
				    } 
			    offset+=mymtd->erasesize;
			    buf+=mymtd->erasesize;
			    }
				if (NUM_PARTITIONS != 0) 
				{
				    printk(KERN_NOTICE "Using ar531x DD-WRT partition definition\n");
				    add_mtd_partitions (mymtd, ar531x_partitions, NUM_PARTITIONS);
				}

	    		    }
out:;	
			}
		}
#endif

		return 0;
	}
	iounmap ((void *)ar531x_map.virt);
	return -ENXIO;
}

static void __exit cleanup_ar531x(void)
{
	if (mymtd) {
		del_mtd_device(mymtd);
		map_destroy(mymtd);
	}
	if (ar531x_map.virt) {
	    iounmap ((void *)ar531x_map.virt);
	    ar531x_map.virt=0;
	}
}

module_init(init_ar531x);
module_exit(cleanup_ar531x);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tomas Dlabac");
MODULE_DESCRIPTION("Atheros AR531x MTD map driver");
