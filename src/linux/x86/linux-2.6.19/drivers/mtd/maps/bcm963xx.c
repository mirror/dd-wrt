/*
 * A simple flash mapping code for BCM963xx board flash memory
 * It is simple because it only treats all the flash memory as ROM
 * It is used with chips/map_rom.c
 *
 *  Song Wang (songw@broadcom.com)
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/io.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/autoconf.h>

#include <board.h>
#include <bcmTag.h>
#include <bcm_map_part.h>
#define  VERSION	"1.0"

extern PFILE_TAG kerSysImageTagGet(void);

static struct mtd_info *mymtd;

static map_word brcm_physmap_read16(struct map_info *map, unsigned long ofs)
{
	map_word val;
	
    /* If the requested flash address is in a memory mapped range, use
     * __raw_readw.  Otherwise, use kerSysReadFromFlash.
     */
    if(((map->map_priv_1 & ~0xfff00000) + ofs + sizeof(short)) < map->map_priv_2)
	    val.x[0] = __raw_readw(map->map_priv_1 + ofs);
    else
        kerSysReadFromFlash( &val.x[0], map->map_priv_1 + ofs, sizeof(short) );
	
	return val;
}

static void brcm_physmap_copy_from(struct map_info *map, void *to, unsigned long from, ssize_t len)
{
    /* If the requested flash address is in a memory mapped range, use
     * memcpy_fromio.  Otherwise, use kerSysReadFromFlash.
     */
    if( ((map->map_priv_1 & ~0xfff00000) + from + len) < map->map_priv_2 )
	    memcpy_fromio(to, map->map_priv_1 + from, len);
    else
        kerSysReadFromFlash( to, map->map_priv_1 + from, len );
}

static struct map_info brcm_physmap_map = {
	.name		= "Physically mapped flash",
	.bankwidth	= 2,
	.read		= brcm_physmap_read16,
	.copy_from	= brcm_physmap_copy_from
};

static int __init init_brcm_physmap(void)
{
        PFILE_TAG pTag = NULL;
        u_int32_t rootfs_addr, kernel_addr;

	printk("bcm963xx_mtd driver v%s\n", VERSION);

        /* Read the flash memory map from flash memory. */
        if (!(pTag = kerSysImageTagGet())) {
                printk("Failed to read image tag from flash\n");
                return -EIO;
        }

        rootfs_addr = (u_int32_t) simple_strtoul(pTag->rootfsAddress, NULL, 10) + BOOT_OFFSET;
        kernel_addr = (u_int32_t) simple_strtoul(pTag->kernelAddress, NULL, 10) + BOOT_OFFSET;
	
	brcm_physmap_map.size = kernel_addr - rootfs_addr;
	brcm_physmap_map.map_priv_1 = (unsigned long)rootfs_addr;

	/* Set map_priv_2 to the amount of flash memory that is memory mapped to
	 * the flash base address.  On the BCM6338, serial flash parts are only
	 * memory mapped up to 1MB even though the flash part may be bigger.
	 */
	brcm_physmap_map.map_priv_2 =(unsigned long)kerSysMemoryMappedFlashSizeGet();

	if (!brcm_physmap_map.map_priv_1) {
		printk("Wrong rootfs starting address\n");
		return -EIO;
	}
	
	if (brcm_physmap_map.size <= 0) {
		printk("Wrong rootfs size\n");
		return -EIO;
	}	
	
	mymtd = do_map_probe("map_rom", &brcm_physmap_map);
	if (mymtd) {
		mymtd->owner = THIS_MODULE;
		add_mtd_device(mymtd);

		return 0;
	}

	return -ENXIO;
}

static void __exit cleanup_brcm_physmap(void)
{
	if (mymtd) {
		del_mtd_device(mymtd);
		map_destroy(mymtd);
	}
	if (brcm_physmap_map.map_priv_1) {
		brcm_physmap_map.map_priv_1 = 0;
	}
}

module_init(init_brcm_physmap);
module_exit(cleanup_brcm_physmap);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Song Wang songw@broadcom.com");
MODULE_DESCRIPTION("Configurable MTD map driver for read-only root file system");
