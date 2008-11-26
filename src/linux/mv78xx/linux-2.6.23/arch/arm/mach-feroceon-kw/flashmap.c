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

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>
#include <linux/version.h>
#include "mvSysHwConfig.h"
#include "boardEnv/mvBoardEnvLib.h"
#include "ctrlEnv/sys/mvCpuIf.h"

/*#define MTD_FLASH_MAP_DEBUG*/

#ifdef MTD_FLASH_MAP_DEBUG
#define DB(x)	x
#else
#define DB(x)
#endif

#define MTD_MAX_FLASH_NUMBER	4
#define MTD_DUMMY_BANK_WIDTH	2

struct maps_init_info
{
	struct map_info mapInfo;
	char ** mtdDrv;
	struct mtd_info * mtdInfo;
	char name[32];
};	

static struct maps_init_info maps[MTD_MAX_FLASH_NUMBER];
static unsigned int mapsNum = 0;

#if defined (CONFIG_MTD_CFI) || defined (CONFIG_MTD_JEDECPROBE)
static char * cfiDev = "cfi_flash";
static char * cfiMtdList[] = { "cfi_probe", "jedec_probe", "map_rom", NULL };
#endif

#ifdef CONFIG_MV_INCLUDE_SFLASH_MTD
static char * sflashDev = "spi_flash";
static char * sflashMtdList[] = {"sflash", NULL};
#endif

#ifdef CONFIG_MV_INCLUDE_MFLASH_MTD
static char * mflashDev = "marvell_flash";
static char * mflashMtdList[] = {"mflash", NULL};
#endif

#ifdef CONFIG_MTD_PARTITIONS
static struct mtd_partition *mtd_parts;
static int                   mtd_parts_nb;
static const char *part_probes[] __initdata = {"cmdlinepart", NULL};
#endif /* CONFIG_MTD_PARTITIONS */

static int flashInfoFill(void)
{
	int expectedDevs = 0;
	int devs, i;
	
	/* clear the whole array */
	memset((void*)maps, 0x0, sizeof(maps));
	
#if defined (CONFIG_MTD_CFI) || defined (CONFIG_MTD_JEDECPROBE)
	/* gather the CFI and JEDEC NOR flash devices information */
	devs = mvBoardGetDevicesNumber(BOARD_DEV_NOR_FLASH);
	
	for(i=0; i<devs; i++)
	{
		if (expectedDevs >= MTD_MAX_FLASH_NUMBER)
		{
			printk(KERN_NOTICE "\nERROR: %s - Exceeded MAX MTD flash devices number", __FUNCTION__);		
			break;	
		}
		maps[expectedDevs].mtdDrv = cfiMtdList;	
		sprintf(maps[expectedDevs].name, "%s_%d", cfiDev, i);
		maps[expectedDevs].mapInfo.name = maps[expectedDevs].name;
		maps[expectedDevs].mapInfo.phys = mvBoardGetDeviceBaseAddr(i, BOARD_DEV_NOR_FLASH);
		maps[expectedDevs].mapInfo.size = mvBoardGetDeviceWinSize(i, BOARD_DEV_NOR_FLASH);
		maps[expectedDevs].mapInfo.bankwidth = (mvBoardGetDeviceBusWidth(i, BOARD_DEV_NOR_FLASH) / 8);
		
		if ((maps[expectedDevs].mapInfo.phys != 0xFFFFFFFF) && 
		    (maps[expectedDevs].mapInfo.size != 0xFFFFFFFF))
		{
			DB(printk("\nINFO: Found %s %d - base 0x%08x, size 0x%x", maps[expectedDevs].mapInfo.name, i,
   	          (unsigned int)maps[expectedDevs].mapInfo.phys, (unsigned int)maps[expectedDevs].mapInfo.size));
   			++expectedDevs;
		}
		else
		{
			printk(KERN_NOTICE "\nERROR: %s - Failed to get Device Base address and Size (%s %d)", __FUNCTION__, maps[expectedDevs].mapInfo.name, i);
		}
	}
#endif
	
#ifdef CONFIG_MV_INCLUDE_SFLASH_MTD
	/* gather the SPI flash devices information */
	devs = mvBoardGetDevicesNumber(BOARD_DEV_SPI_FLASH);
	
	for(i=0; i<devs; i++)
	{
		if (expectedDevs >= MTD_MAX_FLASH_NUMBER)
		{
			printk(KERN_NOTICE "\nERROR: %s - Exceeded MAX MTD flash devices number", __FUNCTION__);		
			break;	
		}
		maps[expectedDevs].mtdDrv = sflashMtdList;	
		maps[expectedDevs].mapInfo.name = sflashDev;
		maps[expectedDevs].mapInfo.phys = mvBoardGetDeviceBaseAddr(i, BOARD_DEV_SPI_FLASH);
		maps[expectedDevs].mapInfo.size = mvBoardGetDeviceWinSize(i, BOARD_DEV_SPI_FLASH);
		maps[expectedDevs].mapInfo.bankwidth = MTD_DUMMY_BANK_WIDTH;
		
		if ((maps[expectedDevs].mapInfo.phys != 0xFFFFFFFF) && 
		    (maps[expectedDevs].mapInfo.size != 0xFFFFFFFF))
		{
			DB(printk("\nINFO: Found %s %d - base 0x%08x, size 0x%x", maps[expectedDevs].mapInfo.name, i,
   	          (unsigned int)maps[expectedDevs].mapInfo.phys, (unsigned int)maps[expectedDevs].mapInfo.size));
   			++expectedDevs;
		}
		else
		{
			printk(KERN_NOTICE "\nERROR: %s - Failed to get Device Base address and Size (%s %d)", __FUNCTION__, maps[expectedDevs].mapInfo.name, i);
		}
	}
#endif

#ifdef CONFIG_MV_INCLUDE_MFLASH_MTD
	/* gather the the Marvell flash devices infomarion */
	if (expectedDevs >= MTD_MAX_FLASH_NUMBER)
	{
		printk(KERN_NOTICE "\nERROR: %s - Exceeded MAX MTD flash devices number", __FUNCTION__);		
	}
	else
	{
		maps[expectedDevs].mtdDrv = mflashMtdList;	
		maps[expectedDevs].mapInfo.name = mflashDev;
		maps[expectedDevs].mapInfo.phys = mvCpuIfTargetWinBaseLowGet(DEVICE_CS1);
		maps[expectedDevs].mapInfo.size = mvCpuIfTargetWinSizeGet(DEVICE_CS1);
		maps[expectedDevs].mapInfo.bankwidth = MTD_DUMMY_BANK_WIDTH;
		
		if ((maps[expectedDevs].mapInfo.phys != 0xFFFFFFFF) && 
		    (maps[expectedDevs].mapInfo.size != 0))
		{
			DB(printk("\nINFO: Found %s - base 0x%08x, size 0x%x", maps[expectedDevs].mapInfo.name,
	          (unsigned int)maps[expectedDevs].mapInfo.phys, (unsigned int)maps[expectedDevs].mapInfo.size));
			++expectedDevs;
		}
		else
		{
			printk(KERN_NOTICE "\nERROR: %s - Failed to get Device Base address and Size (%s)", __FUNCTION__, maps[expectedDevs].mapInfo.name);
		}	
	}
#endif
	
	DB(printk("\nINFO: %s - Found %d Flash Devices", __FUNCTION__, expectedDevs));
	return expectedDevs;
}

static int flashProbe(char ** mtdDrv, struct map_info * map, struct mtd_info ** mtd)
{
	if ((mtdDrv == NULL) || (map == NULL) || (mtd == NULL))
	{
		printk(KERN_NOTICE "\nERROR: NULL pointer parameter at %s entry", __FUNCTION__);
		return -EINVAL;	
	}
	
	/* remap the physical address to a virtual address */
	map->virt = ioremap(map->phys, map->size);	
	if (!map->virt) 
	{
		printk(KERN_NOTICE "\nERROR: Failed to ioremap Flash device at physical base 0x%x.", (unsigned int)map->phys);
		return -EIO;
	}
	
	DB(printk("\nINFO: Io remapped successfully - phy addr = 0x%08x, virt addr = 0x%08x", (unsigned int)map->phys, (unsigned int)map->virt));

	simple_map_init(map);

	*mtd = NULL;
	for(; (!(*mtd) && *mtdDrv); mtdDrv++) 
	{
		DB(printk("\nINFO: Using %s to probe %s at address 0x%08x, size 0x%x, width %dm", 
                  *mtdDrv, map->name, (unsigned int)map->phys, (unsigned int)map->size, map->bankwidth));
		if ((*mtd = do_map_probe(*mtdDrv, map)))
		{
			DB(printk(" - detected OK"));
			/*map->size = (*mtd)->size;*/
			(*mtd)->owner = THIS_MODULE;

#ifdef CONFIG_MTD_PARTITIONS
			mtd_parts_nb = parse_mtd_partitions(*mtd, part_probes, &mtd_parts, 0);

			if (mtd_parts_nb > 0)
			{
				add_mtd_partitions (*mtd, mtd_parts, mtd_parts_nb);
				return 0;
			}
#endif
		
			if (add_mtd_device(*mtd) != 0)
			{
				printk(KERN_NOTICE "\nERROR: %s - Failed to add the mtd device", __FUNCTION__);
				iounmap((void *)map->virt);
				map->virt = 0;
				return -ENXIO;
			}

			return 0;
		} 
		else 
		{
			DB(printk(" - Not detected"));
		}
	}
 
	iounmap((void *)map->virt);
	map->virt = 0;
	return -ENXIO;
}
	
static int __init flash_map_init(void)
{	
	int i;
			
	mapsNum = flashInfoFill();
	DB(printk("\nINFO: flash_map_init - detected %d devices",  mapsNum));
		
	for (i=0; i<mapsNum; i++)
	{
		DB(printk("MTD: Initialize the %s device at address 0x%08x", maps[i].mapInfo.name, (unsigned int)maps[i].mapInfo.phys));
		if (flashProbe(maps[i].mtdDrv, &maps[i].mapInfo, &maps[i].mtdInfo) == 0)
		{
			DB(printk(" - OK.\n"));
		}
		else
		{
			maps[i].mtdInfo = NULL;
			DB(printk(" - FAILED!\n"));
		}
	}
	
	return 0;
}

static void __exit flash_map_exit(void)
{
	int i;
	
	for (i=0; i<mapsNum; i++)
	{
		if (maps[i].mtdInfo) 
		{
			del_mtd_device(maps[i].mtdInfo);
			map_destroy(maps[i].mtdInfo);
		}
	
		if (maps[i].mapInfo.virt) 
		{
			iounmap((void *)maps[i].mapInfo.virt);
			maps[i].mapInfo.virt = 0;
		}
	}
}

module_init(flash_map_init);
module_exit(flash_map_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MTD map driver for Marvell platforms");
