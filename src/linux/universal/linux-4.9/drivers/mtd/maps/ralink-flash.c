/* ralink-flash.c
 *
 * Steven Liu <steven_liu@ralinktech.com.tw>:
 *   - initial approach
 *
 * Winfred Lu <winfred_lu@ralinktech.com.tw>:
 *   - 32MB flash support for RT3052
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>

#include <asm/io.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/concat.h>
#include <linux/mtd/partitions.h>
#include <linux/vmalloc.h>
#include <linux/magic.h>
#include <linux/err.h>
#include <asm/rt2880/rt_mmap.h>
#include "../mtdcore.h"



#ifdef CONFIG_RT2880_FLASH_8M
        /* marklin 20080605 : return read mode for ST */
extern void Flash_SetModeRead(void);
#endif

#ifndef CONFIG_RT2880_FLASH_32M 
#define WINDOW_ADDR		CONFIG_MTD_PHYSMAP_START
#define WINDOW_SIZE		CONFIG_MTD_PHYSMAP_LEN
#else
#define WINDOW_ADDR_0		CONFIG_MTD_PHYSMAP_START
#define WINDOW_ADDR_1		0xBB000000
#define WINDOW_SIZE		(CONFIG_MTD_PHYSMAP_LEN / 2)
#endif

#define BUSWIDTH		CONFIG_MTD_PHYSMAP_BUSWIDTH
static struct mtd_info *merged_mtd=NULL;
static struct mtd_info *concat_mtd=NULL;


#ifndef CONFIG_RT2880_FLASH_32M 

#if defined(CONFIG_RT2880_ASIC) && defined(CONFIG_RT2880_FLASH_8M)
#define NUM_FLASH_BANKS 1

static struct map_info ralink_map[] = {
	{
	.name = "Ralink SoC physically mapped flash bank 1",
	.bankwidth = BUSWIDTH,
	.size = CONFIG_MTD_PHYSMAP_LEN,
	.phys = CONFIG_MTD_PHYSMAP_START
	},
};

#else
#define NUM_FLASH_BANKS 1

static struct map_info ralink_map[] = {
	{
	.name = "Ralink SoC physically mapped flash",
	.bankwidth = BUSWIDTH,
	.size = CONFIG_MTD_PHYSMAP_LEN,
	.phys = CONFIG_MTD_PHYSMAP_START
	},
};
#endif


#else
#define NUM_FLASH_BANKS 2
static struct map_info ralink_map[] = {
	{
	.name = "Ralink SoC physically mapped flash bank 0",
	.bankwidth = BUSWIDTH,
	.size = WINDOW_SIZE,
	.phys = WINDOW_ADDR_0
	},
	{
	.name = "Ralink SoC physically mapped flash bank 1",
	.bankwidth = BUSWIDTH,
	.size = WINDOW_SIZE,
	.phys = WINDOW_ADDR_1
	}
};
#endif
static struct mtd_info *ralink_mtd[NUM_FLASH_BANKS];

#if defined (CONFIG_RT2880_FLASH_32M) && defined (CONFIG_RALINK_RT3052_MP2)
static struct mtd_partition rt2880_partitions[] = {
        {
                name:           "Bootloader",  /* mtdblock0 */
                size:           0x40000,  /* 192K */
                offset:         0,
        }, {
#ifdef CONFIG_RT2880_ROOTFS_IN_FLASH
                name:           "Kernel", /* mtdblock3 */
                size:           CONFIG_MTD_KERNEL_PART_SIZ,
                offset:         MTDPART_OFS_APPEND,
        }, {
                name:           "RootFS", /* mtdblock4 */
                size:           (0x2000000 - 0x80000 - CONFIG_MTD_KERNEL_PART_SIZ),
                offset:         MTDPART_OFS_APPEND,
        }, {
#else //CONFIG_RT2880_ROOTFS_IN_RAM
                name:           "Kernel", /* mtdblock3 */
                size:           (0x2000000 - 0x80000),
                offset:         MTDPART_OFS_APPEND,
        }, {
#endif
                name:           "Config", /* mtdblock1 */
                size:           0x20000,  /* 128K */
                offset:         MTDPART_OFS_APPEND,
                mask_flags:     MTD_WRITEABLE  /* force read-only */
        }, {
                name:           "Factory", /* mtdblock2 */
                size:           0x20000,  /* 128K */
                offset:         MTDPART_OFS_APPEND,
                mask_flags:     MTD_WRITEABLE  /* force read-only */
        }
};
#elif CONFIG_RT2880_FLASH_8M 
static struct mtd_partition rt2880_partitions[] = {
        {
                name:           "uboot",  /* mtdblock0 */
                size:           0x30000,  /* 192K */
                offset:         0x400000,
        }, {
                name:           "uboot-config", /* mtdblock1 */
                size:           0x10000,  /* 64K */
                offset:         MTDPART_OFS_APPEND,
                mask_flags:     0x400000  /* force read-only */
        }, {
                name:           "factory-defaults", /* mtdblock2 */
                size:           0x10000,  /* 64K */
                offset:         MTDPART_OFS_APPEND,
                mask_flags:     0x400000  /* force read-only */
        }, {
                name:           "linux", /* mtdblock3 */
                size:           CONFIG_MTD_KERNEL_PART_SIZ,
                offset:         0x450000,
        }, {
                name:           "rootfs", /* mtdblock4 */
                size:           MTDPART_SIZ_FULL,
                offset:         MTDPART_OFS_APPEND,
        }, {
                name:           "nvram", /* mtdblock4 */
                size:           0x10000,
                offset:         0x7e0000,
        }};


#else //not 32M flash
static struct mtd_partition rt2880_partitions[] = {
        {
                name:           "uboot",  /* mtdblock0 */
                size:           0x30000,  /* 192K */
                offset:         0,
        }, {
                name:           "uboot-config", /* mtdblock1 */
                size:           0x10000,  /* 64K */
                offset:         MTDPART_OFS_APPEND,
                mask_flags:     0  /* force read-only */
        }, {
                name:           "factory-defaults", /* mtdblock2 */
                size:           0x10000,  /* 64K */
                offset:         MTDPART_OFS_APPEND,
                mask_flags:     0  /* force read-only */
        }
#ifdef CONFIG_MTD_AR670W
, {
                name:           "linux", /* mtdblock3 */
                size:           CONFIG_MTD_KERNEL_PART_SIZ,
                offset:         0x40000,
        }, 
#else
, {
                name:           "linux", /* mtdblock3 */
                size:           CONFIG_MTD_KERNEL_PART_SIZ,
                offset:         0x50000,
        }, 
#endif
    	    {
                name:           "rootfs", /* mtdblock4 */
                size:           MTDPART_SIZ_FULL,
                offset:         MTDPART_OFS_APPEND,
        }, {
                name:           "nvram", /* mtdblock4 */
                size:           0x10000,
                offset:         0x3f0000,
        }};
#endif


static int ralink_lock(struct mtd_info *mtd, loff_t ofs, uint64_t len)
{
	return 0;
}

static int ralink_unlock(struct mtd_info *mtd, loff_t ofs, uint64_t len)
{
	return 0;
}
#define BOOT_FROM_NOR	0
#define BOOT_FROM_NAND	2
#define BOOT_FROM_SPI	3


int ra_check_flash_type(void)
{

    uint8_t Id[10];
    int syscfg=0;
    int boot_from=0;
    int chip_mode=0;

    memset(Id, 0, sizeof(Id));
    strncpy(Id, (char *)RALINK_SYSCTL_BASE, 6);
    syscfg = (*((int *)(RALINK_SYSCTL_BASE + 0x10)));
    printk(KERN_INFO "system type: %s\n",Id);
    if((strcmp(Id,"RT3052")==0) || (strcmp(Id, "RT3350")==0)) {
	boot_from = (syscfg >> 16) & 0x3; 
	switch(boot_from)
	{
	case 0:
	case 1:
	    boot_from=BOOT_FROM_NOR;
	    break;
	case 2:
	    boot_from=BOOT_FROM_NAND;
	    break;
	case 3:
	    boot_from=BOOT_FROM_SPI;
	    break;
	}
    }else if(strcmp(Id,"RT3883")==0) {
	boot_from = (syscfg >> 4) & 0x3; 
	switch(boot_from)
	{
	case 0:
	case 1:
	    boot_from=BOOT_FROM_NOR;
	    break;
	case 2:
	case 3:
	    chip_mode = syscfg & 0xF;
	    if((chip_mode==0) || (chip_mode==7)) {
		boot_from = BOOT_FROM_SPI;
	    }else if(chip_mode==8) {
		boot_from = BOOT_FROM_NAND;
	    }else {
		printk("unknow chip_mode=%d\n",chip_mode);
	    }
	    break;
	}
    }else if(strcmp(Id,"RT3352")==0) {
	boot_from = BOOT_FROM_SPI;
    }else if(strcmp(Id,"RT5350")==0) {
	boot_from = BOOT_FROM_SPI;
    }else if(strcmp(Id,"RT2880")==0) {
	boot_from = BOOT_FROM_NOR;
    } else {
	printk("%s: %s is not supported\n",__FUNCTION__, Id);
    }
    printk(KERN_INFO "boot type: %d\n",boot_from);
    return boot_from;


}


int __init rt2880_mtd_init(void)
{
	int ret = -ENXIO;
	int i, found = 0;

	if(ra_check_flash_type()!=BOOT_FROM_NOR) { /* NOR */
	    return 0;
	}

	for (i = 0; i < NUM_FLASH_BANKS; i++) {
		printk(KERN_NOTICE "ralink flash device: 0x%x at 0x%x\n",
				ralink_map[i].size, ralink_map[i].phys);

		ralink_map[i].virt = ioremap(ralink_map[i].phys, ralink_map[i].size);
		if (!ralink_map[i].virt) {
			printk("Failed to ioremap\n");
			return -EIO;
		}
		simple_map_init(&ralink_map[i]);
		ralink_mtd[i] = do_map_probe("cfi_probe", &ralink_map[i]);
		if (ralink_mtd[i]) {
			ralink_mtd[i]->owner = THIS_MODULE;
			ralink_mtd[i]->_lock = ralink_lock;
			ralink_mtd[i]->_unlock = ralink_unlock;
			++found;
		}
		else
			iounmap(ralink_map[i].virt);
	}
	if (found == NUM_FLASH_BANKS) {
#ifdef CONFIG_RT2880_FLASH_32M 
		merged_mtd = mtd_concat_create(ralink_mtd, NUM_FLASH_BANKS,
				"Ralink Merged Flash");
		ret = add_mtd_partitions(merged_mtd, rt2880_partitions,
				ARRAY_SIZE(rt2880_partitions));
#else
#ifdef CONFIG_MTD_ESR6650 
int nvramsize = ralink_mtd[0]->erasesize*2;
		int offset = 0;
#elif CONFIG_EAP9550
int nvramsize = ralink_mtd[0]->erasesize*2;
		int offset = 0;
#elif CONFIG_RT2880_FLASH_8M 
/* marklin 20080605 : return read mode for ST */
Flash_SetModeRead();
int nvramsize = ralink_mtd[0]->erasesize*2;
		int offset = 0x400000;
#else
int nvramsize = ralink_mtd[0]->erasesize;
		int offset = 0;
#endif
		char *buf = vmalloc(4096);
			    while((offset+ralink_mtd[0]->erasesize)<ralink_mtd[0]->size)
			    {
			    int retlen;
			    mtd_read(ralink_mtd[0],offset,4, &retlen, buf);
//			    printk(KERN_INFO "%X: %c %c %c %c\n",offset,buf[0],buf[1],buf[2],buf[3]);
			    if (*((__u32 *) buf) == SQUASHFS_MAGIC)
				    {
				    	printk(KERN_INFO "\nfound squashfs at %X\n",offset);
#ifdef CONFIG_RT2880_FLASH_8M 
					rt2880_partitions[3].size=(((ralink_mtd[0]->size)-nvramsize)-0x450000);					
					rt2880_partitions[4].offset=offset;					
					rt2880_partitions[4].size = rt2880_partitions[3].size-(offset-0x450000);					
#else
#ifdef CONFIG_MTD_AR670W
					rt2880_partitions[3].size=(((ralink_mtd[0]->size)-nvramsize)-0x40000);					
					rt2880_partitions[4].offset=offset;					
					rt2880_partitions[4].size = rt2880_partitions[3].size-(offset-0x40000);					
#else
					rt2880_partitions[3].size=(((ralink_mtd[0]->size)-nvramsize)-0x50000);					
					rt2880_partitions[4].offset=offset;					
					rt2880_partitions[4].size = rt2880_partitions[3].size-(offset-0x50000);					
#endif
#endif
					rt2880_partitions[5].offset=ralink_mtd[0]->size-nvramsize;					
					rt2880_partitions[5].size = ralink_mtd[0]->erasesize;					
					break;
				    } 
			    offset+=4096;
			    }
		vfree(buf);
		ret = add_mtd_partitions(ralink_mtd[0], rt2880_partitions,ARRAY_SIZE(rt2880_partitions));
#endif
		if (ret) {
			for (i = 0; i < NUM_FLASH_BANKS; i++)
				iounmap(ralink_map[i].virt);
			return ret;
		}
	}
	else {
		printk("Error: %d flash device was found\n", found);
		return -ENXIO;
	}
	return 0;
}

static void __exit rt2880_mtd_cleanup(void)
{
	int i;

	if (merged_mtd) {
		del_mtd_device(merged_mtd);
		mtd_concat_destroy(merged_mtd);
	}
	for (i = 0; i < NUM_FLASH_BANKS; i++) {
		if (ralink_mtd[i])
			map_destroy(ralink_mtd[i]);
		if (ralink_map[i].virt) {
			iounmap(ralink_map[i].virt);
			ralink_map[i].virt = NULL;
		}
	}
}

module_init(rt2880_mtd_init);
module_exit(rt2880_mtd_cleanup);

MODULE_AUTHOR("Steven Liu <steven_liu@ralinktech.com.tw>");
MODULE_DESCRIPTION("Ralink APSoC Flash Map");
MODULE_LICENSE("GPL");


/*
 * Flash API: ra_mtd_read, ra_mtd_write
 * Arguments:
 *   - num: specific the mtd number
 *   - to/from: the offset to read from or written to
 *   - len: length
 *   - buf: data to be read/written
 * Returns:
 *   - return -errno if failed
 *   - return the number of bytes read/written if successed
 */
int ra_mtd_write(int num, loff_t to, size_t len, const u_char *buf)
{
	int ret = -1;
	size_t rdlen, wrlen;
	struct mtd_info *mtd;
	struct erase_info ei;
	u_char *bak = NULL;
//	printk(KERN_INFO "writing to partition %d, offset %d, len %d\n",num,to,len);
#ifdef CONFIG_RT2880_FLASH_8M
        /* marklin 20080605 : return read mode for ST */
        Flash_SetModeRead();
#endif

	mtd = get_mtd_device(NULL, num);
	if (IS_ERR(mtd))
		return (int)mtd;
	if (len > mtd->erasesize) {
		put_mtd_device(mtd);
		return -E2BIG;
	}

	bak = kmalloc(mtd->erasesize, GFP_KERNEL);
	if (bak == NULL) {
		put_mtd_device(mtd);
		return -ENOMEM;
	}

	ret = mtd_read(mtd, 0, mtd->erasesize, &rdlen, bak);
	if (ret != 0) {
		put_mtd_device(mtd);
		kfree(bak);
		return ret;
	}
	if (rdlen != mtd->erasesize)
		printk(KERN_INFO "warning: ra_mtd_write: rdlen is not equal to erasesize\n");

	memcpy(bak + to, buf, len);

	ei.mtd = mtd;
	ei.callback = NULL;
	ei.addr = 0;
	ei.len = mtd->erasesize;
	ei.priv = 0;
	ret = mtd_erase(mtd, &ei);
	if (ret != 0) {
		put_mtd_device(mtd);
		kfree(bak);
		return ret;
	}

	ret = mtd_write(mtd, 0, mtd->erasesize, &wrlen, bak);

	put_mtd_device(mtd);
	kfree(bak);
#ifdef CONFIG_RT2880_FLASH_8M
        /* marklin 20080605 : return read mode for ST */
        Flash_SetModeRead();
#endif
	return ret;
}

int ra_mtd_read(int num, loff_t from, size_t len, u_char *buf)
{
	int ret;
	size_t rdlen;
	struct mtd_info *mtd;
#ifdef CONFIG_RT2880_FLASH_8M
        /* marklin 20080605 : return read mode for ST */
        Flash_SetModeRead();
#endif
//	printk(KERN_INFO "reading from partition %d, offset %d, len %d\n",num,from,len);
	mtd = get_mtd_device(NULL, num);
	if (IS_ERR(mtd))
		return (int)mtd;

	ret = mtd_read(mtd, from, len, &rdlen, buf);
	if (rdlen != len)
		printk(KERN_INFO "warning: ra_mtd_read: rdlen is not equal to len\n");

	put_mtd_device(mtd);
	return ret;
}

EXPORT_SYMBOL(ra_mtd_write);
EXPORT_SYMBOL(ra_mtd_read);
