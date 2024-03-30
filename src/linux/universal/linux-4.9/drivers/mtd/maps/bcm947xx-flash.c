/*
 * Flash mapping for BCM947XX boards
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: bcm947xx-flash.c,v 1.5 2008/03/25 01:27:49 Exp $
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/io.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>
#include "../mtdcore.h"
#include <linux/magic.h>
#include <linux/jffs2.h>
#include <linux/crc32.h>
#include <linux/vmalloc.h>

#include <typedefs.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <siutils.h>
#include <trxhdr.h>

/* Global SB handle */
extern void *bcm947xx_sih;
extern spinlock_t bcm947xx_sih_lock;

/* Convenience */
#define sih bcm947xx_sih
#define sih_lock bcm947xx_sih_lock

#define WINDOW_ADDR 0x1fc00000
#define WINDOW_SIZE 0x400000
#define BUSWIDTH 2

/* e.g., flash=2M or flash=4M */
static int flash = 0;
module_param(flash, int, 0);
static int __init bcm947xx_setup(char *str)
{
	flash = memparse(str, &str);
	return 1;
}

__setup("flash=", bcm947xx_setup);

static struct mtd_info *bcm947xx_mtd;

#if LINUX_VERSION_CODE < 0x20212 && defined(MODULE)
#define init_bcm947xx_map init_module
#define cleanup_bcm947xx_map cleanup_module
#endif

#define ROUTER_NETGEAR_WGR614L           1
#define ROUTER_NETGEAR_WNR834B           2
#define ROUTER_NETGEAR_WNDR3300          3
#define ROUTER_NETGEAR_WNR3500L          4
#define ROUTER_NETGEAR_WNR2000V2         5
#define ROUTER_NETGEAR_WNDR3400          6
#define ROUTER_NETGEAR_WNDR4000          7
#define ROUTER_NETGEAR_WNDR4500          8
#define ROUTER_BELKIN_F5D8235V3          9
#define ROUTER_BELKIN_F7D3301_3302_4302  10
#define ROUTER_UBNT_UNIFIAC              11
/* Belkin series */
#define TRX_MAGIC_F7D3301              0x20100322	/* Belkin Share Max; router's birthday ? */
#define TRX_MAGIC_F7D3302              0x20090928	/* Belkin Share; router's birthday ? */
#define TRX_MAGIC_F7D4302              0x20091006	/* Belkin Play; router's birthday ? */
#define TRX_MAGIC_F5D8235V3            0x00017116	/* Belkin F5D8235-4v3 */
#define TRX_MAGIC_QA                   0x12345678	/* cfe: It's QA firmware */

/* Netgear wgr614 */
#define WGR614_CHECKSUM_BLOCK_START    0x003A0000
#define WGR614_CHECKSUM_OFF            0x003AFFF8
#define WGR614_FAKE_LEN                0x00000004	//we fake checksum only over 4 bytes (HDR0)
#define WGR614_FAKE_CHK                0x02C0010E

static int get_router(void)
{
	uint boardnum = bcm_strtoul(nvram_safe_get("boardnum"), NULL, 0);

	if ((boardnum == 8 || boardnum == 01)
	    && nvram_match("boardtype", "0x0472")
	    && nvram_match("cardbus", "1")) {
		return ROUTER_NETGEAR_WNR834B;	//Netgear WNR834B, Netgear WNR834Bv2
	}

	if (boardnum == 01 && nvram_match("boardtype", "0x0472")
	    && nvram_match("boardrev", "0x23")) {
		return ROUTER_NETGEAR_WNDR3300;	//Netgear WNDR-3300    
	}

	if ((boardnum == 83258 || boardnum == 01)	//or 001 or 0x01
	    && (nvram_match("boardtype", "0x048e") || nvram_match("boardtype", "0x48E"))
	    && (nvram_match("boardrev", "0x11") || nvram_match("boardrev", "0x10"))
	    && (nvram_match("boardflags", "0x750") || nvram_match("boardflags", "0x0750"))
	    && nvram_match("sdram_init", "0x000A")) {
		return ROUTER_NETGEAR_WGR614L;	//Netgear WGR614v8/L/WW 16MB ram, cfe v1.3 or v1.5
	}

	if ((boardnum == 1 || boardnum == 3500)
	    && nvram_match("boardtype", "0x04CF")
	    && (nvram_match("boardrev", "0x1213") || nvram_match("boardrev", "02"))) {
		return ROUTER_NETGEAR_WNR3500L;	//Netgear WNR3500v2/U/L
	}

	if (boardnum == 1 && nvram_match("boardtype", "0xE4CD")
	    && nvram_match("boardrev", "0x1700")) {
		return ROUTER_NETGEAR_WNR2000V2;	//Netgear WNR2000v2   
	}

	if (boardnum == 01 && nvram_match("boardtype", "0xb4cf")
	    && nvram_match("boardrev", "0x1100")) {
		return ROUTER_NETGEAR_WNDR3400;	//Netgear WNDR3400     
	}

	if (boardnum == 01 && nvram_match("boardtype", "0xF52C")
	    && nvram_match("boardrev", "0x1101")) {
		return ROUTER_NETGEAR_WNDR4000;	//Netgear WNDR4000     
	}

	if (nvram_match("boardtype", "0xa4cf")
	    && nvram_match("boardrev", "0x1100")) {
		return ROUTER_BELKIN_F5D8235V3;	//F5D8235v3
	}

	if (nvram_match("boardtype", "0xa4cf")
	    && nvram_match("boardrev", "0x1102")) {
		return ROUTER_BELKIN_F7D3301_3302_4302;	//Belkin F7D3301v1 /F7D3302v1 / F7D4302v1
	}

	if (boardnum == 4536 
	    && nvram_match("boardtype", "0xf52e")
	    && nvram_match("boardrev", "0x1102")) {
		printk(KERN_INFO "found WNDR4500 / R6300\n");
		return ROUTER_NETGEAR_WNDR4500;	//Netgear WNDR4500                 
	}

	if (nvram_match("boardtype","0x0617") &&
	   nvram_match("boardrev","0x1103"))
	 {
	    printk(KERN_INFO "found Ubiquiti UnifiAP AC");
	    return ROUTER_UBNT_UNIFIAC;
	 }

	return 0;
}

static void bcm47xx_map_copy_from(struct map_info *map, void *to, unsigned long from, ssize_t len)
{
	if (len == 1) {
		memcpy_fromio(to, map->virt + from, len);
	} else {
		int i;
		u16 *dest = (u16 *)to;
		u16 *src = (u16 *)(map->virt + from);
		for (i = 0; i < (len / 2); i++) {
			dest[i] = src[i];
		}
		if (len & 1)
			*((u8 *)dest + len - 1) = src[i] & 0xff;
	}
}

struct map_info bcm947xx_map = {
      name:"Physically mapped flash",
      size:WINDOW_SIZE,
      bankwidth:BUSWIDTH,
      phys:WINDOW_ADDR,
};

#ifdef CONFIG_MTD

static struct mtd_partition bcm947xx_parts[] = {
      {name: "cfe", offset: 0, size:0,},
      {name: "linux", offset: 0, size:0,},
      {name: "rootfs", offset: 0, size:0,},
      {name: "nvram", offset: 0, size:0,},
      {name: "ddwrt", offset: 0, size:0,},
      {name: "board_data", offset: 0, size:0,},
      {name:NULL,},
};

struct mtd_partition cfe_nvrampart = {
      name:NULL,
      offset:0,
      size:0
};

struct mtd_partition cfe_boardpart = {
      name:NULL,
      offset:0,
      size:0
};

extern int cfenvram;

static struct mtd_partition nflash_parts[] = {
      {name: "cfe", offset: 0, size:0,},
      {name: "nvram", offset: 0, size:0,},
      {name:NULL,},
};

void add_netgear_boarddata(void)
{
	if (bcm947xx_mtd && cfe_boardpart.name)
		add_mtd_partitions(bcm947xx_mtd, &cfe_boardpart, 1);
}

#define NVRAM_MAGIC		0x48534C46	/* 'FLSH' */
static int onlyboot = 0;
static int __init find_cfe_size(struct mtd_info *mtd, size_t size)
{
	struct trx_header *trx;
	static unsigned char buf[513];
	int off;
	size_t len;
	int blocksize;

	trx = (struct trx_header *)buf;
	printk(KERN_INFO "try to find cfe size up to %d\n", size);
	if (mtd == NULL) {
		printk(KERN_INFO "mtd is NULL\n");
		return -1;
	}
	blocksize = mtd->erasesize;
	if (blocksize < 0x10000)
		blocksize = 0x10000;
//      printk(KERN_INFO "blocksize is %d\n",blocksize);
	for (off = 0x10000; off < size; off += 1024) {
		memset(buf, 0xe5, sizeof(buf));
//              printk(KERN_INFO "scan at 0x%08x\n",off);
		/*
		 * Read into buffer 
		 */
		if (mtd_read(mtd, off, sizeof(buf), &len, buf) || len != sizeof(buf))
			continue;

		switch (le32_to_cpu(trx->magic)) {
			/* found a TRX header */
		case TRX_MAGIC:
			goto found;
			break;
			/* found a Belkin TRX header */
		case TRX_MAGIC_F7D3301:
		case TRX_MAGIC_F7D3302:
		case TRX_MAGIC_F7D4302:
		case TRX_MAGIC_F5D8235V3:
		case TRX_MAGIC_QA:
			if (get_router() == ROUTER_BELKIN_F7D3301_3302_4302 || get_router() == ROUTER_BELKIN_F5D8235V3) {
				printk(KERN_INFO "Found Belkin TRX magic\n");
				goto found;
			}
			break;
		case NVRAM_MAGIC:
			printk(KERN_NOTICE "no filesys. assume nflash devices\n");
			onlyboot = 1;
			off = size;
			goto found;
			break;

		}

	}

	printk(KERN_INFO "%s: Couldn't find bootloader size\n", mtd->name);
	return -1;

found:
	printk(KERN_INFO "bootloader size: %d\n", off);
	printk(KERN_INFO "nvram size: %d\n", NVRAM_SPACE);
	return off;

}

/*
 * Copied from mtdblock.c
 *
 * Cache stuff...
 * 
 * Since typical flash erasable sectors are much larger than what Linux's
 * buffer cache can handle, we must implement read-modify-write on flash
 * sectors for each block write requests.  To avoid over-erasing flash sectors
 * and to speed things up, we locally cache a whole flash sector while it is
 * being written to until a different sector is required.
 */

static void erase_callback(struct erase_info *done)
{
	wait_queue_head_t *wait_q = (wait_queue_head_t *) done->priv;
	wake_up(wait_q);
}

static int erase_write(struct mtd_info *mtd, unsigned long pos, int len, const char *buf)
{
	struct erase_info erase;
	DECLARE_WAITQUEUE(wait, current);
	wait_queue_head_t wait_q;
	size_t retlen;
	int ret;

	/*
	 * First, let's erase the flash block.
	 */

	init_waitqueue_head(&wait_q);
	erase.mtd = mtd;
	erase.callback = erase_callback;
	erase.addr = pos;
	erase.len = len;
	erase.priv = (u_long) & wait_q;

	set_current_state(TASK_INTERRUPTIBLE);
	add_wait_queue(&wait_q, &wait);

	ret = mtd_erase(mtd, &erase);
	if (ret) {
		set_current_state(TASK_RUNNING);
		remove_wait_queue(&wait_q, &wait);
		printk(KERN_WARNING "erase of region [0x%lx, 0x%x] " "on \"%s\" failed\n", pos, len, mtd->name);
		return ret;
	}

	schedule();		/* Wait for erase to finish. */
	remove_wait_queue(&wait_q, &wait);

	/*
	 * Next, writhe data to flash.
	 */

	ret = mtd_write(mtd, pos, len, &retlen, buf);
	if (ret)
		return ret;
	if (retlen != len)
		return -EIO;
	return 0;
}

extern int __init root_dev_setup(char *line);

static int __init find_root(struct mtd_info *mtd, size_t size, struct mtd_partition *part)
{
	struct trx_header trx, *trx2;
	unsigned char buf[512], *block;
	int off, blocksize;
	u32 i, crc = ~0;
	size_t len;
	struct squashfs_super_block *sb = (struct squashfs_super_block *)buf;

	blocksize = mtd->erasesize;
	if (blocksize < 0x10000)
		blocksize = 0x10000;

	for (off = 0; off < size; off += 64 * 1024) {
		memset(&trx, 0xe5, sizeof(trx));
//              printk(KERN_INFO "scan root at 0x%08x\n",off);

		/*
		 * Read into buffer 
		 */
		if (mtd_read(mtd, off, sizeof(trx), &len, (char *)&trx) || len != sizeof(trx))
			continue;

		switch (le32_to_cpu(trx.magic)) {
			/* found a TRX header */
		case TRX_MAGIC:
			goto found;
			break;
			/* found a Belkin TRX header */
		case TRX_MAGIC_F7D3301:
		case TRX_MAGIC_F7D3302:
		case TRX_MAGIC_F7D4302:
		case TRX_MAGIC_F5D8235V3:
		case TRX_MAGIC_QA:
			if (get_router() == ROUTER_BELKIN_F7D3301_3302_4302 || get_router() == ROUTER_BELKIN_F5D8235V3) {
				printk(KERN_INFO "Found Belkin TRX magic\n");
				goto found;
			}
			break;
		}
	}

	printk(KERN_INFO "%s: Couldn't find root filesystem\n", mtd->name);
	return -1;

found:
	part->offset = le32_to_cpu(trx.offsets[2]) ? : le32_to_cpu(trx.offsets[1]);
	part->size = le32_to_cpu(trx.len);

	part->size -= part->offset;
	part->offset += off;

	if (part->size == 0)
		return 0;

	if (mtd_read(mtd, part->offset, sizeof(buf), &len, buf) || len != sizeof(buf))
		return 0;

	if (*((__u32 *)buf) == SQUASHFS_MAGIC) {
		printk(KERN_INFO "%s: Filesystem type: squashfs, size=0x%x\n", mtd->name, (u32)le64_to_cpu(sb->bytes_used));

		/* Update the squashfs partition size based on the superblock info */
		part->size = le64_to_cpu(sb->bytes_used);
		//part->size = part->size + 1024; /* uncomment for belkin v2000 ! */
		len = part->offset + part->size;
		len += (mtd->erasesize - 1);
		len &= ~(mtd->erasesize - 1);
		part->size = len - part->offset;
		printk(KERN_INFO "partition size = %d\n", part->size);
	} else if (*((__u16 *)buf) == JFFS2_MAGIC_BITMASK) {
		printk(KERN_INFO "%s: Filesystem type: jffs2\n", mtd->name);

		/* Move the squashfs outside of the trx */
		part->size = 0;
	} else {
		printk(KERN_INFO "%s: Filesystem type: unknown\n", mtd->name);
		return 0;
	}

	if (trx.len != part->offset + part->size - off) {
		/* Update the trx offsets and length */
		trx.len = part->offset + part->size - off;
//              printk(KERN_INFO "update crc32\n");
		/* Update the trx crc32 */
		for (i = (u32)&(((struct trx_header *)NULL)->flag_version); i <= trx.len; i += sizeof(buf)) {
//                      printk(KERN_INFO "read from %d\n",off + i);
			if (mtd_read(mtd, off + i, sizeof(buf), &len, buf) || len != sizeof(buf))
				return 0;
			crc = crc32_le(crc, buf, min(sizeof(buf), trx.len - i));
		}
		trx.crc32 = crc;

//                      printk(KERN_INFO "malloc\n",off + i);
		/* read first eraseblock from the trx */
		trx2 = block = vmalloc(mtd->erasesize);
		if (mtd_read(mtd, off, mtd->erasesize, &len, block) || len != mtd->erasesize) {
			printk(KERN_INFO "Error accessing the first trx eraseblock\n");
			vfree(block);
			return 0;
		}

		printk(KERN_INFO "Updating TRX offsets and length:\n");
		printk(KERN_INFO "old trx = [0x%08x, 0x%08x, 0x%08x], len=0x%08x crc32=0x%08x\n", trx2->offsets[0], trx2->offsets[1], trx2->offsets[2], trx2->len, trx2->crc32);
		printk(KERN_INFO "new trx = [0x%08x, 0x%08x, 0x%08x], len=0x%08x crc32=0x%08x\n", trx.offsets[0], trx.offsets[1], trx.offsets[2], trx.len, trx.crc32);

		/* Write updated trx header to the flash */
		memcpy(block, &trx, sizeof(trx));
		mtd_unlock(mtd, off, mtd->erasesize);
		erase_write(mtd, off, mtd->erasesize, block);
		mtd_sync(mtd);
		vfree(block);
		printk(KERN_INFO "Done\n");

		/* Write fake Netgear checksum to the flash */
		if (get_router() == ROUTER_NETGEAR_WGR614L) {
			/*
			 * Read into buffer 
			 */
			block = vmalloc(mtd->erasesize);
			if (mtd_read(mtd, WGR614_CHECKSUM_BLOCK_START, mtd->erasesize, &len, block) || len != mtd->erasesize) {
				printk(KERN_INFO "Error accessing the WGR614 checksum eraseblock\n");
				vfree(block);
			} else {
				char imageInfo[8];
				u32 fake_len = le32_to_cpu(WGR614_FAKE_LEN);
				u32 fake_chk = le32_to_cpu(WGR614_FAKE_CHK);
				memcpy(&imageInfo[0], (char *)&fake_len, 4);
				memcpy(&imageInfo[4], (char *)&fake_chk, 4);
				char *tmp;
				tmp = block + ((WGR614_CHECKSUM_OFF - WGR614_CHECKSUM_BLOCK_START) % mtd->erasesize);
				memcpy(tmp, imageInfo, sizeof(imageInfo));
				mtd_unlock(mtd, WGR614_CHECKSUM_BLOCK_START, mtd->erasesize);
				erase_write(mtd, WGR614_CHECKSUM_BLOCK_START, mtd->erasesize, block);
				mtd_sync(mtd);
				vfree(block);
				printk(KERN_INFO "Done fixing WGR614 checksum\n");
			}
		}

	}

	return part->size;
}

struct mtd_partition *__init init_mtd_partitions(struct mtd_info *mtd, size_t size)
{
	int cfe_size;

	int board_data_size = 0;	// e.g Netgear 0x003e0000-0x003f0000 : "board_data", we exclude this part from our mapping
	int jffs_exclude_size = 0;	// to prevent overwriting len/checksum on e.g. Netgear WGR614v8/L/WW

	switch (get_router()) {
	case ROUTER_UBNT_UNIFIAC:
		board_data_size = 0x10000;	//Netgear: checksum is @ 0x003AFFF8 for 4M flash
		jffs_exclude_size = 0x0;	//or checksum is @ 0x007AFFF8 for 8M flash
		break;
	case ROUTER_NETGEAR_WGR614L:
	case ROUTER_NETGEAR_WNR834B:
	case ROUTER_NETGEAR_WNDR3300:
	case ROUTER_NETGEAR_WNR3500L:
		board_data_size = 4 * 0x10000;	//Netgear: checksum is @ 0x003AFFF8 for 4M flash
		jffs_exclude_size = 0x10000;	//or checksum is @ 0x007AFFF8 for 8M flash
		break;
	case ROUTER_NETGEAR_WNR2000V2:
		board_data_size = 0x10000;
		break;
	case ROUTER_NETGEAR_WNDR3400:	//Netgear: checksum is @ 0x0070FFF8 @ 8M flash, but can be overwritten
	case ROUTER_NETGEAR_WNDR4000:	//Netgear: checksum is @ 0x0073FFF8 @ 8M flash, but can be overwritten
	case ROUTER_NETGEAR_WNDR4500:	//Netgear: checksum is @ 0x0073FFF8 @ 8M flash, but can be overwritten
		board_data_size = 0x10000;
		break;
	}

	if ((cfe_size = find_cfe_size(mtd, size)) < 0)
		return NULL;
	if (onlyboot) {
		nflash_parts[0].offset = 0;
		nflash_parts[0].size = cfe_size;
		if (cfenvram) {
			nflash_parts[1].offset = size - mtd->erasesize * 3;
			nflash_parts[1].size = mtd->erasesize;
			cfe_nvrampart.name = "nvram_cfe";
			cfe_nvrampart.offset = size - ROUNDUP(NVRAM_SPACE, mtd->erasesize);
			cfe_nvrampart.size = ROUNDUP(NVRAM_SPACE, mtd->erasesize);
		} else {
#ifdef NVRAM_SPACE_256
		if (nvram_match("boardnum","${serno}") && nvram_match("boardtype","0xC617") && nvram_match("boardrev","0x1103")) {
			printk(KERN_INFO "Linksys NVRAM Hack\n");
			nflash_parts[1].offset = size - ROUNDUP(NVRAM_SPACE_256, mtd->erasesize);
			nflash_parts[1].size = ROUNDUP(NVRAM_SPACE_256, mtd->erasesize);

//			cfe_nvrampart.name = "nvram_cfe";
//			cfe_nvrampart.offset = size - ROUNDUP(NVRAM_SPACE_256, mtd->erasesize);
//			cfe_nvrampart.size = ROUNDUP(NVRAM_SPACE_256, mtd->erasesize);
		}else
#endif
		{
			nflash_parts[1].offset = size - ROUNDUP(NVRAM_SPACE, mtd->erasesize);
			nflash_parts[1].size = ROUNDUP(NVRAM_SPACE, mtd->erasesize);
		}
		}
		if (board_data_size) {
			cfe_boardpart.name = "board_data";
			cfe_boardpart.size = board_data_size;
			if (cfe_nvrampart.name)
				cfe_boardpart.offset = cfe_nvrampart.offset - board_data_size;
			else
				cfe_boardpart.offset = nflash_parts[1].offset - board_data_size;
		}
		nflash_parts[2].size = mtd->erasesize;
		return nflash_parts;
	}
	// in case of nor flash, this small hack is required
	root_dev_setup("1f02");
	/* boot loader */
	bcm947xx_parts[0].offset = 0;
	bcm947xx_parts[0].size = cfe_size;

	/* nvram */
	if (cfe_size != 384 * 1024) {
		if (cfenvram) {
			bcm947xx_parts[3].offset = (size - mtd->erasesize * 2) - board_data_size;
			bcm947xx_parts[3].size = mtd->erasesize;
			cfe_nvrampart.name = "nvram_cfe";
			cfe_nvrampart.offset = size - ROUNDUP(NVRAM_SPACE, mtd->erasesize);
			cfe_nvrampart.size = ROUNDUP(NVRAM_SPACE, mtd->erasesize);
		} else {
			bcm947xx_parts[3].offset = size - ROUNDUP(NVRAM_SPACE, mtd->erasesize);
			bcm947xx_parts[3].size = ROUNDUP(NVRAM_SPACE, mtd->erasesize);
		}
	} else {
		/* nvram (old 128kb config partition on netgear wgt634u) */
		bcm947xx_parts[3].offset = bcm947xx_parts[0].size;
		bcm947xx_parts[3].size = ROUNDUP(NVRAM_SPACE, mtd->erasesize);
	}

	if (board_data_size != 0) {
		bcm947xx_parts[5].size = board_data_size;
		if (cfe_nvrampart.name)
			bcm947xx_parts[5].offset = cfe_nvrampart.offset - board_data_size;
		else
			bcm947xx_parts[5].offset = bcm947xx_parts[3].offset - board_data_size;
	} else {
		bcm947xx_parts[5].name = NULL;
	}

	if (cfenvram)
		board_data_size = 0;

	/* linux (kernel and rootfs) */
	if (cfe_size != 384 * 1024) {
		bcm947xx_parts[1].offset = bcm947xx_parts[0].size;
		bcm947xx_parts[1].size = (bcm947xx_parts[3].offset - bcm947xx_parts[1].offset) - board_data_size;
	} else {
		/* do not count the elf loader, which is on one block */
		bcm947xx_parts[1].offset = bcm947xx_parts[0].size + bcm947xx_parts[3].size + mtd->erasesize;
		bcm947xx_parts[1].size = (((size - bcm947xx_parts[0].size) - (2 * bcm947xx_parts[3].size)) - mtd->erasesize) - board_data_size;
	}

	/* find and size rootfs */
	if (find_root(mtd, size, &bcm947xx_parts[2]) == 0) {
		/* entirely jffs2 */
		bcm947xx_parts[4].name = NULL;
		bcm947xx_parts[2].size = (size - bcm947xx_parts[2].offset) - bcm947xx_parts[3].size;
	} else {
		/* legacy setup */
		/* calculate leftover flash, and assign it to the jffs2 partition */
		if (cfe_size != 384 * 1024) {
			bcm947xx_parts[4].offset = bcm947xx_parts[2].offset + bcm947xx_parts[2].size;
			if ((((unsigned int)bcm947xx_parts[4].offset) % mtd->erasesize) > 0) {
				bcm947xx_parts[4].offset += mtd->erasesize - (((unsigned int)bcm947xx_parts[4].offset) % mtd->erasesize);
			}
			bcm947xx_parts[4].size = ((bcm947xx_parts[3].offset - bcm947xx_parts[4].offset) - board_data_size) - jffs_exclude_size;
		} else {
			bcm947xx_parts[4].offset = bcm947xx_parts[2].offset + bcm947xx_parts[2].size;
			if ((((unsigned int)bcm947xx_parts[4].offset) % mtd->erasesize) > 0) {
				bcm947xx_parts[4].offset += mtd->erasesize - (((unsigned int)bcm947xx_parts[4].offset) % mtd->erasesize);
			}
			bcm947xx_parts[4].size = (((size - bcm947xx_parts[3].size) - bcm947xx_parts[4].offset) - board_data_size) - jffs_exclude_size;
		}
		/* do not make zero size jffs2 partition  */
		if (bcm947xx_parts[4].size < mtd->erasesize) {
			bcm947xx_parts[4].name = NULL;
		}
	}

	return bcm947xx_parts;
}

#endif

static int __init init_bcm947xx_map(void)
{
	ulong flags;
	uint coreidx;
	chipcregs_t *cc;
	uint32 fltype;
	uint window_addr = 0, window_size = 0;
	size_t size;
	int ret = 0;
#ifdef CONFIG_MTD
	struct mtd_partition *parts;
	int i;
#endif

	spin_lock_irqsave(&sih_lock, flags);
	coreidx = si_coreidx(sih);

	/* Check strapping option if chipcommon exists */
	if ((cc = si_setcore(sih, CC_CORE_ID, 0))) {
		fltype = readl(&cc->capabilities) & CC_CAP_FLASH_MASK;
		if (fltype == PFLASH) {
			bcm947xx_map.map_priv_2 = 1;
			window_addr = 0x1c000000;
			bcm947xx_map.size = window_size = 32 * 1024 * 1024;
			if ((readl(&cc->flash_config) & CC_CFG_DS) == 0)
				bcm947xx_map.bankwidth = 1;
		}
	} else {
		fltype = PFLASH;
		bcm947xx_map.map_priv_2 = 0;
		window_addr = WINDOW_ADDR;
		bcm947xx_map.size = window_size = WINDOW_SIZE;
	}

	si_setcoreidx(sih, coreidx);
	spin_unlock_irqrestore(&sih_lock, flags);

	if (fltype != PFLASH) {
		printk(KERN_ERR "pflash: found no supported devices\n");
		ret = -ENODEV;
		goto fail;
	}

	bcm947xx_map.virt = ioremap(window_addr, window_size);
	if (bcm947xx_map.virt == NULL) {
		printk(KERN_ERR "pflash: ioremap failed\n");
		ret = -EIO;
		goto fail;
	}

	if ((bcm947xx_mtd = do_map_probe("cfi_probe", &bcm947xx_map)) == NULL) {
		printk(KERN_ERR "pflash: cfi_probe failed\n");
		ret = -ENXIO;
		goto fail;
	}

	bcm947xx_mtd->owner = THIS_MODULE;

	/* override copy_from routine */
//      bcm947xx_map.copy_from = bcm47xx_map_copy_from;

	/* Allow size override for testing */
	size = flash ? : bcm947xx_mtd->size;

	printk(KERN_NOTICE "Flash device: 0x%x at 0x%x\n", size, window_addr);

#ifdef CONFIG_MTD
	parts = init_mtd_partitions(bcm947xx_mtd, size);
	for (i = 0; parts[i].name; i++) ;
	ret = add_mtd_partitions(bcm947xx_mtd, parts, i);
	if (ret) {
		printk(KERN_ERR "pflash: add_mtd_partitions failed\n");
		goto fail;
	}
	if (cfe_nvrampart.name)
		add_mtd_partitions(bcm947xx_mtd, &cfe_nvrampart, 1);

#endif

	return 0;

fail:
	if (bcm947xx_mtd)
		map_destroy(bcm947xx_mtd);
	if (bcm947xx_map.map_priv_1)
		iounmap((void *)bcm947xx_map.map_priv_1);
	bcm947xx_map.map_priv_1 = 0;
	return ret;
}

static void __exit cleanup_bcm947xx_map(void)
{
#ifdef CONFIG_MTD
	del_mtd_partitions(bcm947xx_mtd);
#endif
	map_destroy(bcm947xx_mtd);
	iounmap((void *)bcm947xx_map.map_priv_1);
	bcm947xx_map.map_priv_1 = 0;
}

module_init(init_bcm947xx_map);
module_exit(cleanup_bcm947xx_map);
