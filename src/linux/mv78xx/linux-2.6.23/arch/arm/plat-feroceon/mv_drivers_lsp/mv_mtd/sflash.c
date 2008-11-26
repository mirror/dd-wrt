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
#include "sflash/mvSFlash.h"
#include "sflash/mvSFlashSpec.h"
#include "ctrlEnv/mvCtrlEnvLib.h"

/*#define MTD_SFLASH_DEBUG*/

#ifdef MTD_SFLASH_DEBUG
#define DB(x)	x
#else
#define DB(x)
#endif


/* Configuration options */
static struct mtd_info *sflash_probe(struct map_info *map);
static void sflash_destroy(struct mtd_info *mtd);
static int sflash_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf);
static int sflash_write(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, const u_char *buf);
static int sflash_erase(struct mtd_info *mtd, struct erase_info *instr);
static void sflash_sync(struct mtd_info *mtd);
static int sflash_suspend(struct mtd_info *mtd);
static void sflash_resume(struct mtd_info *mtd);
static int sflash_lock (struct mtd_info *mtd, loff_t ofs, size_t len);
static int sflash_unlock (struct mtd_info *mtd, loff_t ofs, size_t len);
static int sflash_block_isbad (struct mtd_info *mtd, loff_t ofs);
static int sflash_block_markbad (struct mtd_info *mtd, loff_t ofs);

static struct mtd_chip_driver sflash_chipdrv = {
	.probe		= sflash_probe,
	.destroy	= sflash_destroy,
	.name		= "sflash",
	.module		= THIS_MODULE
};


static struct mtd_info *sflash_probe(struct map_info *map)
{
	struct mtd_info *mtd = NULL;
	MV_SFLASH_INFO *sflash = NULL;

#ifdef CONFIG_MV78200
	if (MV_FALSE == mvSocUnitIsMappedToThisCpu(SPI_FLASH))
	{
		printk(KERN_INFO"SPI flash is not mapped to this CPU\n");
		return -ENODEV;
	}		
#endif

	DB(printk("\nINFO: entering %s",__FUNCTION__));

	/* allocate the memory for the mtd_info */
	mtd = kmalloc(sizeof(*mtd), GFP_KERNEL);
	if(!mtd)
	{
		printk(KERN_NOTICE "\nERROR: %s - Failed to allocate memory for mtd structure",__FUNCTION__);
		return NULL;
	}

	/* allocate memory for the sflash private structure */
	sflash = kmalloc(sizeof(MV_SFLASH_INFO), GFP_KERNEL);
	if(!sflash) 
	{
		printk(KERN_NOTICE "\nERROR: %s - Failed to allocate memory for sflash structure",__FUNCTION__);
		kfree(mtd);
		return NULL;
	}
		
	/* clear both structures before usage */
	memset(mtd, 0, sizeof(*mtd));
	memset(sflash, 0, sizeof(*sflash));
	    
	DB(printk("\nINFO: %s - Base address %08x",__FUNCTION__, sflash->baseAddr));
#ifdef CONFIG_ARCH_FEROCEON_ORION	
	/* First check that SPI bus mode is configured to connect to an external SFlash */
    if (mvCtrlSpiBusModeDetect() != MV_SPI_CONN_TO_EXT_FLASH)
    {
        printk(KERN_NOTICE "\nERROR: %s - SPI interface is not routed to external SPI flash!", __FUNCTION__);
		kfree(mtd);
		kfree(sflash);
		return NULL;
    }
#endif
	/* Try to detect the flash and initialize it over SPI */	
	sflash->baseAddr         = map->phys;
    sflash->index            = MV_INVALID_DEVICE_NUMBER; /* will be detected in init */	
	if (mvSFlashInit(sflash) != MV_OK)
	{
		printk(KERN_NOTICE "ERROR: %s - Failed to initialize the SFlash.", __FUNCTION__);
		kfree(mtd);
		kfree(sflash);
		return NULL;
	}
	
	/* After success fill in the MTD structure with the appropriate info */
	mtd->erasesize = sflash->sectorSize;
	mtd->size = sflash->sectorSize * sflash->sectorNumber;
	mtd->priv = map; /*sflash;*/
	mtd->type = MTD_NORFLASH;
	mtd->erase = sflash_erase;
	mtd->read = sflash_read;
	mtd->write = sflash_write;
	mtd->sync = sflash_sync;
	mtd->suspend = sflash_suspend;
	mtd->resume = sflash_resume;	
	mtd->lock = sflash_lock;
	mtd->unlock = sflash_unlock;
	mtd->block_isbad = sflash_block_isbad;
	mtd->block_markbad = sflash_block_markbad;	
	mtd->flags = (MTD_WRITEABLE | MTD_BIT_WRITEABLE); /* just like MTD_CAP_NORFLASH */
	mtd->name = map->name;
	mtd->writesize = 1;
	
	map->fldrv = &sflash_chipdrv;
	map->fldrv_priv = sflash;
	
	/* Print some debug messages with the detected sflash info */
	DB(printk("\nINFO: %s - Detected SFlash device (size %d)", __FUNCTION__, mtd->size));
	DB(printk("\n           Base Address    : 0x%08x", sflash->baseAddr));
	DB(printk("\n           Manufacturer ID : 0x%02x", sflash->manufacturerId));
	DB(printk("\n           Device ID       : 0x%04x", sflash->deviceId));
	DB(printk("\n           Sector Size     : 0x%x", sflash->sectorSize));
	DB(printk("\n           Sector Number   : %d", sflash->sectorNumber));
	
	printk("SPI Serial flash detected @ 0x%08x, %dKB (%dsec x %dKB)\n",
	         sflash->baseAddr, ((sflash->sectorNumber * sflash->sectorSize)/1024), 
	         sflash->sectorNumber, (sflash->sectorSize/1024));
	
	__module_get(THIS_MODULE);
	return mtd;
}

static void sflash_destroy(struct mtd_info *mtd)
{
	struct map_info *map = mtd->priv;
	MV_SFLASH_INFO *sflash = map->fldrv_priv;

	DB(printk("\nINFO: %s called", __FUNCTION__));

	/* free memory allocated at probe for the private sflash structure */
	if (sflash)
		kfree(sflash);	
}

static int sflash_read(struct mtd_info *mtd, loff_t from, size_t len,
	size_t *retlen, u_char *buf)
{
	struct map_info *map = mtd->priv;
	MV_SFLASH_INFO *sflash = map->fldrv_priv;
	MV_U32 offset = ((MV_U32)from);
	
	*retlen = 0;

	DB(printk("\nINFO: %s  - offset %08x, len %d",__FUNCTION__, offset, (int)len));
		
	if (mvSFlashBlockRd(sflash, offset, buf, len) != MV_OK)
	{
		printk(KERN_NOTICE "\nERROR: %s - Failed to read block.", __FUNCTION__);
		return -1;
	}
	
	*retlen = len;
	
	DB(printk(" - OK"));

	return 0;	
}

static int sflash_write(struct mtd_info *mtd, loff_t to, size_t len,
	size_t *retlen, const u_char *buf)
{
	struct map_info *map = mtd->priv;
	MV_SFLASH_INFO *sflash = map->fldrv_priv;
/*	MV_SFLASH_INFO *sflash = mtd->priv;*/
	MV_U32 offset = ((MV_U32)to);
	
	*retlen = 0;
	
	DB(printk("\nINFO: %s - offset %08x, len %d",__FUNCTION__, offset, len));
		
	if (mvSFlashBlockWr(sflash, offset, (MV_U8*)buf, len) != MV_OK)
	{
		printk(KERN_NOTICE "\nERROR: %s - Failed to write block", __FUNCTION__);
		return -1;
	}
	
	*retlen = len;
	
	DB(printk(" - OK"));

	return 0;	

}


static int sflash_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	struct map_info *map = mtd->priv;
	MV_SFLASH_INFO *sflash = map->fldrv_priv;
/*	MV_SFLASH_INFO *sflash = mtd->priv;*/
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
	
	DB(printk("\nINFO: %s - from sector %u to %u",__FUNCTION__, fsec, 
		  lsec-1));
	
	for (i=fsec; i<lsec; i++)
	{
		if (mvSFlashSectorErase(sflash, i) != MV_OK)
		{
			printk(KERN_NOTICE "\nError: %s - mvSFlashSectorErase on sector %d",__FUNCTION__, i);
			return -1;
		}
	}
	
	instr->state = MTD_ERASE_DONE;
	mtd_erase_callback(instr);

	return 0;
}

static int sflash_lock (struct mtd_info *mtd, loff_t ofs, size_t len)
{
	struct map_info *map = mtd->priv;
	MV_SFLASH_INFO *sflash = map->fldrv_priv;
/*	MV_SFLASH_INFO *sflash = mtd->priv;*/
	
	DB(printk("\nINFO: %s called", __FUNCTION__));
	
	if (mvSFlashWpRegionSet(sflash, MV_WP_ALL) != MV_OK)
	{
		printk(KERN_NOTICE "\nError: %s - mvSFlashWpRegionSet failed",__FUNCTION__);
		return -1;
	}
	
	printk("\nNotice: Serial SPI flash (%s) lock per sector is not supported!\n        Locking the whole device.", mtd->name);
		
	return 0;
}

static int sflash_unlock (struct mtd_info *mtd, loff_t ofs, size_t len)
{
	struct map_info *map = mtd->priv;
	MV_SFLASH_INFO *sflash = map->fldrv_priv;
/*	MV_SFLASH_INFO *sflash = mtd->priv;*/

	DB(printk("\nINFO: %s called", __FUNCTION__));
	
	if (mvSFlashWpRegionSet(sflash, MV_WP_NONE) != MV_OK)
	{
		printk(KERN_NOTICE "\nError: %s - mvSFlashWpRegionSet failed",__FUNCTION__);
		return -1;
	}
		
	printk("\nNotice: Serial SPI flash (%s) unlock per sector is not supported!\n        Unlocking the whole device.", mtd->name);
	return 0;
}

static void sflash_sync(struct mtd_info *mtd)
{
	DB(printk("\nINFO: %s called - DUMMY", __FUNCTION__));
}

static int sflash_suspend(struct mtd_info *mtd)
{
	DB(printk("\nINFO: %s called - DUMMY()", __FUNCTION__));
	return 0;
}

static void sflash_resume(struct mtd_info *mtd)
{
	DB(printk("\nINFO: %s called - DUMMY", __FUNCTION__));
}

static int sflash_block_isbad (struct mtd_info *mtd, loff_t ofs)
{
	DB(printk("\nINFO: %s called - DUMMY", __FUNCTION__));
	return 0;
}

static int sflash_block_markbad (struct mtd_info *mtd, loff_t ofs)
{
	DB(printk("\nINFO: %s called - DUMMY", __FUNCTION__));
	return 0;
}

static int __init sflash_probe_init(void)
{
	DB(printk("\nINFO: %s - MTD SFlash chip driver.\n", __FUNCTION__));

	register_mtd_chip_driver(&sflash_chipdrv);

	return 0;
}

static void __exit sflash_probe_exit(void)
{
	DB(printk(KERN_ALERT "\nINFO: %s - MTD SFlash driver exit", __FUNCTION__));
	unregister_mtd_chip_driver(&sflash_chipdrv);
}

module_init(sflash_probe_init);
module_exit(sflash_probe_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("MTD chip driver for the SPI serial flash device");

