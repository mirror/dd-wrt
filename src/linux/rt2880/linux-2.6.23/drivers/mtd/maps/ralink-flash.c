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

#include <asm/io.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/concat.h>
#include <linux/mtd/partitions.h>
#include <linux/vmalloc.h>
#include <linux/squashfs_fs.h>


#ifndef CONFIG_RT2880_FLASH_32M 
#define WINDOW_ADDR		CONFIG_MTD_PHYSMAP_START
#define WINDOW_SIZE		CONFIG_MTD_PHYSMAP_LEN
#define NUM_FLASH_BANKS		1
#else
#define WINDOW_ADDR_0		CONFIG_MTD_PHYSMAP_START
#define WINDOW_ADDR_1		0xBB000000
#define WINDOW_SIZE		(CONFIG_MTD_PHYSMAP_LEN / 2)
#define NUM_FLASH_BANKS		2
#endif

#define BUSWIDTH		CONFIG_MTD_PHYSMAP_BUSWIDTH


static struct mtd_info *ralink_mtd[NUM_FLASH_BANKS];
#ifndef CONFIG_RT2880_FLASH_32M 
static struct map_info ralink_map[] = {
	{
	.name = "Ralink SoC physically mapped flash",
	.bankwidth = BUSWIDTH,
	.size = WINDOW_SIZE,
	.phys = WINDOW_ADDR
	}
};
#else
static struct mtd_info *merged_mtd;
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
        }, {
                name:           "linux", /* mtdblock3 */
                size:           CONFIG_MTD_KERNEL_PART_SIZ,
                offset:         0x50000,
        }, {
                name:           "rootfs", /* mtdblock4 */
                size:           MTDPART_SIZ_FULL,
                offset:         MTDPART_OFS_APPEND,
        }, {
                name:           "nvram", /* mtdblock4 */
                size:           0x10000,
                offset:         0x3f0000,
        }};
#endif

static int ralink_lock(struct mtd_info *mtd, loff_t ofs, size_t len)
{
	return 0;
}

static int ralink_unlock(struct mtd_info *mtd, loff_t ofs, size_t len)
{
	return 0;
}

int __init rt2880_mtd_init(void)
{
	int ret = -ENXIO;
	int i, found = 0;

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
			ralink_mtd[i]->lock = ralink_lock;
			ralink_mtd[i]->unlock = ralink_unlock;
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
		char *buf = vmalloc(4096);
		int offset = 0;
			    while((offset+ralink_mtd[0]->erasesize)<ralink_mtd[0]->size)
			    {
			    int retlen;
			    ralink_mtd[0]->read(ralink_mtd[0],offset,4096, &retlen, buf);
//			    printk(KERN_EMERG "%X: %c %c %c %c\n",offset,buf[0],buf[1],buf[2],buf[3]);
			    if (*((__u32 *) buf) == SQUASHFS_MAGIC)
				    {
				    	printk(KERN_EMERG "\nfound squashfs at %X\n",offset);
					rt2880_partitions[3].size=((ralink_mtd[0]->size-0x10000)-0x50000);					
					rt2880_partitions[4].offset=offset;					
					rt2880_partitions[4].size = rt2880_partitions[3].size-(offset-0x50000);					
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

#ifdef CONFIG_RT2880_FLASH_32M 
	if (merged_mtd) {
		del_mtd_device(merged_mtd);
		mtd_concat_destroy(merged_mtd);
	}
#endif
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
