/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell 
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File in accordance with the terms and conditions of the General 
Public License Version 2, June 1991 (the "GPL License"), a copy of which is 
available along with the File in the license.txt file or by writing to the Free 
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or 
on the worldwide web at http://www.gnu.org/licenses/gpl.txt. 

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED 
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY 
DISCLAIMED.  The GPL License provides additional details about this warranty 
disclaimer.
********************************************************************************/


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/mtd/map.h>
#include <linux/mtd/mtd.h>
#include <mflash/mvMFlash.h>
#include "mflash/mvMFlashSpec.h"
#include "ctrlEnv/mvCtrlEnvLib.h"

/*#define MTD_MFLASH_DEBUG*/

#ifdef MTD_MFLASH_DEBUG
#define DB(x)	x
#else
#define DB(x)
#endif


/* Configuration options */
static struct mtd_info *mflash_probe(struct map_info *map);
static void mflash_destroy(struct mtd_info *mtd);
static int mflash_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf);
static int mflash_write(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, const u_char *buf);
static int mflash_erase(struct mtd_info *mtd, struct erase_info *instr);
static void mflash_sync(struct mtd_info *mtd);
static int mflash_suspend(struct mtd_info *mtd);
static void mflash_resume(struct mtd_info *mtd);
static int mflash_lock (struct mtd_info *mtd, loff_t ofs, size_t len);
static int mflash_unlock (struct mtd_info *mtd, loff_t ofs, size_t len);
static int mflash_block_isbad (struct mtd_info *mtd, loff_t ofs);
static int mflash_block_markbad (struct mtd_info *mtd, loff_t ofs);

static struct mtd_chip_driver mflash_chipdrv = {
	.probe		= mflash_probe,
	.destroy	= mflash_destroy,
	.name		= "mflash",
	.module		= THIS_MODULE
};


static struct mtd_info *mflash_probe(struct map_info *map)
{
	struct mtd_info *mtd = NULL;
	MV_MFLASH_INFO *mflash = NULL;
	
	DB(printk("\nINFO: enterring %s",__FUNCTION__));

	/* allocate the memory for the mtd_info */
	mtd = kmalloc(sizeof(*mtd), GFP_KERNEL);
	if(!mtd)
	{
		printk(KERN_NOTICE "\nERROR: %s - Failed to allocate memory for mtd structure",__FUNCTION__);
		return NULL;
	}

	/* allocate memory for the mflash private structure */
	mflash = kmalloc(sizeof(MV_MFLASH_INFO), GFP_KERNEL);
	if(!mflash) 
	{
		printk(KERN_NOTICE "\nERROR: %s - Failed to allocate memory for mflash structure",__FUNCTION__);
		kfree(mtd);
		return NULL;
	}
		
	/* clear both structures before usage */
	memset(mtd, 0, sizeof(*mtd));
	memset(mflash, 0, sizeof(*mflash));
	    
	DB(printk("\nINFO: %s - Base address %08x",__FUNCTION__, mflash->baseAddr));
	
    /* based on the SPI mode try to detect the Mflash device interface type */
    if (mvCtrlSpiBusModeDetect() == MV_SPI_CONN_TO_MFLASH)
        mflash->ifMode = MV_MFLASH_SPI; 
    else               
        mflash->ifMode = MV_MFLASH_PARALLEL;
	
	/* Try to detect the Marvell flash and initialize */	
	mflash->baseAddr = map->phys;
	if (mvMFlashInit(mflash) != MV_OK)
	{
		printk(KERN_NOTICE "ERROR: %s - Failed to initialize the mflash.", __FUNCTION__);
		kfree(mtd);
		kfree(mflash);
		return NULL;
	}
	
	if (mvMFlashSectorSizeSet(mflash, 0x1000 /*4K*/) != MV_OK)
	{
		printk(KERN_NOTICE "ERROR: %s - Failed to sector sector size to small.", __FUNCTION__);
		kfree(mtd);
		kfree(mflash);
		return NULL;
	}
	
	/* After success fill in the MTD structure with the appropriate info */
	mtd->erasesize = mflash->sectorSize;
	mtd->size = mflash->sectorSize * mflash->sectorNumber;
	mtd->priv = map;
	mtd->type = MTD_NORFLASH;
	mtd->erase = mflash_erase;
	mtd->read = mflash_read;
	mtd->write = mflash_write;
	mtd->sync = mflash_sync;
	mtd->suspend = mflash_suspend;
	mtd->resume = mflash_resume;	
	mtd->lock = mflash_lock;
	mtd->unlock = mflash_unlock;
	mtd->block_isbad = mflash_block_isbad;
	mtd->block_markbad = mflash_block_markbad;	
	mtd->flags = (MTD_WRITEABLE | MTD_BIT_WRITEABLE); /* just like MTD_CAP_NORFLASH */
	mtd->name = map->name;
	mtd->writesize = 1;
	
	map->fldrv = &mflash_chipdrv;
	map->fldrv_priv = mflash;
	
	/* Print some debug messages with the detected mflash info */
	DB(printk("\nINFO: %s - Detected mflash device (size %d)", __FUNCTION__, mtd->size));
	DB(printk("\n           Base Address    : 0x%08x", mflash->baseAddr));
	DB(printk("\n           Interface Mode  : %d", mflash->ifMode));
	DB(printk("\n           Sector Size     : 0x%x", mflash->sectorSize));
	DB(printk("\n           Sector Number   : %d", mflash->sectorNumber));
	DB(printk("\n           Info Region Size: 0x%x", mflash->infoSize));
	
	printk("Marvell Flash Detected @ 0x%08x, %dKB Main region (%dsec x %dKB), %dKB Information region\n",
	         mflash->baseAddr, ((mflash->sectorNumber * mflash->sectorSize)/1024),
	         mflash->sectorNumber, (mflash->sectorSize/1024), (mflash->infoSize/1024));
	
	__module_get(THIS_MODULE);
	return mtd;
}

static void mflash_destroy(struct mtd_info *mtd)
{
	struct map_info *map = mtd->priv;
	MV_MFLASH_INFO *mflash = map->fldrv_priv;

	DB(printk("\nINFO: %s called", __FUNCTION__));

	/* free memory allocated at probe for the private mflash structure */
	if (mflash)
		kfree(mflash);	
}

static int mflash_read(struct mtd_info *mtd, loff_t from, size_t len,
	size_t *retlen, u_char *buf)
{
	struct map_info *map = mtd->priv;
	MV_MFLASH_INFO *mflash = map->fldrv_priv;
	MV_U32 offset = ((MV_U32)from);
	
	*retlen = 0;

	DB(printk("\nINFO: %s  - offset %08x, len %d",__FUNCTION__, offset, (int)len));
		
	if (mvMFlashBlockRd(mflash, offset, len, buf) != MV_OK)
	{
		printk(KERN_NOTICE "\nERROR: %s - Failed to read block.", __FUNCTION__);
		return -1;
	}
	
	*retlen = len;
	
	DB(printk(" - OK"));

	return 0;	
}

static int mflash_write(struct mtd_info *mtd, loff_t to, size_t len,
	size_t *retlen, const u_char *buf)
{
	struct map_info *map = mtd->priv;
	MV_MFLASH_INFO *mflash = map->fldrv_priv;
	MV_U32 offset = ((MV_U32)to);
	
	*retlen = 0;
	
	DB(printk("\nINFO: %s - offset %08x, len %d",__FUNCTION__, offset, len));
		
	if (mvMFlashBlockWr(mflash, offset, len, (MV_U8*)buf, MV_FALSE) != MV_OK)
	{
		printk(KERN_NOTICE "\nERROR: %s - Failed to write block", __FUNCTION__);
		return -1;
	}
	
	*retlen = len;
	
	DB(printk(" - OK"));

	return 0;	

}


static int mflash_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	struct map_info *map = mtd->priv;
	MV_MFLASH_INFO *mflash = map->fldrv_priv;
	MV_U32 fsec, lsec;
	int i;

	DB(printk("\nINFO: %s - Addr %08x, len %d",__FUNCTION__, instr->addr, instr->len));
	
	if(instr->addr & (mtd->erasesize - 1))
	{
		printk(KERN_NOTICE "\nError: %s - Erase address not sector alligned",__FUNCTION__);
		return -EINVAL;
	}
	if(instr->len & (mtd->erasesize - 1))
	{
		printk(KERN_NOTICE "\nError: %s - Erase length is not sector alligned",__FUNCTION__);
		return -EINVAL;
	}
	if(instr->len + instr->addr > mtd->size)
	{
		printk(KERN_NOTICE "\nError: %s - Erase exceeded flash size",__FUNCTION__);
		return -EINVAL;
	}

	fsec = (instr->addr / mtd->erasesize);
	lsec = (fsec +(instr->len / mtd->erasesize));
	
	DB(printk("\nINFO: %s - from sector %u to %u",__FUNCTION__, fsec, lsec));
	
	for (i=fsec; i<lsec; i++)
	{
		if (mvMFlashSecErase(mflash, i) != MV_OK)
		{
			printk(KERN_NOTICE "\nError: %s - mvMFlashSecErase on sector %d",__FUNCTION__, i);
			return -1;
		}
	}
	
	instr->state = MTD_ERASE_DONE;
	mtd_erase_callback(instr);

	return 0;
}

static int mflash_lock (struct mtd_info *mtd, loff_t ofs, size_t len)
{
	struct map_info *map = mtd->priv;
	MV_MFLASH_INFO *mflash = map->fldrv_priv;
	
	DB(printk("\nINFO: %s called", __FUNCTION__));
	
	if (mvMFlashWriteProtectSet(mflash, MV_TRUE) != MV_OK)
	{
		printk(KERN_NOTICE "\nError: %s - mvmflashWpRegionSet failed",__FUNCTION__);
		return -1;
	}
	
	printk("\nNotice: Marvell flash (%s) lock per sector is not supported!\n        Locking the whole device.", mtd->name);
		
	return 0;
}

static int mflash_unlock (struct mtd_info *mtd, loff_t ofs, size_t len)
{
	struct map_info *map = mtd->priv;
	MV_MFLASH_INFO *mflash = map->fldrv_priv;

	DB(printk("\nINFO: %s called", __FUNCTION__));
	
	if (mvMFlashWriteProtectSet(mflash, MV_FALSE) != MV_OK)
	{
		printk(KERN_NOTICE "\nError: %s - mvmflashWpRegionSet failed",__FUNCTION__);
		return -1;
	}
		
	printk("\nNotice: Marvell flash (%s) unlock per sector is not supported!\n        Unlocking the whole device.", mtd->name);
	return 0;
}

static void mflash_sync(struct mtd_info *mtd)
{
	DB(printk("\nINFO: %s called - DUMMY", __FUNCTION__));
}

static int mflash_suspend(struct mtd_info *mtd)
{
	DB(printk("\nINFO: %s called - DUMMY()", __FUNCTION__));
	return 0;
}

static void mflash_resume(struct mtd_info *mtd)
{
	DB(printk("\nINFO: %s called - DUMMY", __FUNCTION__));
}

static int mflash_block_isbad (struct mtd_info *mtd, loff_t ofs)
{
	DB(printk("\nINFO: %s called - DUMMY", __FUNCTION__));
	return 0;
}

static int mflash_block_markbad (struct mtd_info *mtd, loff_t ofs)
{
	DB(printk("\nINFO: %s called - DUMMY", __FUNCTION__));
	return 0;
}

static int __init mflash_probe_init(void)
{
	DB(printk("\nINFO: %s - MTD mflash chip driver.", __FUNCTION__));

	register_mtd_chip_driver(&mflash_chipdrv);

	return 0;
}

static void __exit mflash_probe_exit(void)
{
	DB(printk(KERN_ALERT "\nINFO: %s - MTD mflash driver exit", __FUNCTION__));
	unregister_mtd_chip_driver(&mflash_chipdrv);
}

module_init(mflash_probe_init);
module_exit(mflash_probe_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("MTD chip driver for the Marvell SUNOL flash device");

