/*
 * Flash memory access on 4G Systems MTX-1 board
 * 
 * (C) 2003 Pete Popov <ppopov@mvista.com>
 *	    Bruno Randolf <bruno.randolf@4g-systems.de>
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>

#include <asm/io.h>
#include <asm/au1000.h>

#ifdef 	DEBUG_RW
#define	DBG(x...)	printk(x)
#else
#define	DBG(x...)	
#endif

#ifdef CONFIG_MIPS_MTX1
#define WINDOW_ADDR 0x1E000000
#define WINDOW_SIZE 0x2000000
#endif

__u8 physmap_read8(struct map_info *map, unsigned long ofs)
{
	__u8 ret;
	ret = __raw_readb(map->map_priv_1 + ofs);
	DBG("read8 from %x, %x\n", (unsigned)(map->map_priv_1 + ofs), ret);
	return ret;
}

__u16 physmap_read16(struct map_info *map, unsigned long ofs)
{
	__u16 ret;
	ret = __raw_readw(map->map_priv_1 + ofs);
	DBG("read16 from %x, %x\n", (unsigned)(map->map_priv_1 + ofs), ret);
	return ret;
}

__u32 physmap_read32(struct map_info *map, unsigned long ofs)
{
	__u32 ret;
	ret = __raw_readl(map->map_priv_1 + ofs);
	DBG("read32 from %x, %x\n", (unsigned)(map->map_priv_1 + ofs), ret);
	return ret;
}

void physmap_copy_from(struct map_info *map, void *to, unsigned long from, ssize_t len)
{
	DBG("physmap_copy from %x to %x\n", (unsigned)from, (unsigned)to);
	memcpy_fromio(to, map->map_priv_1 + from, len);
}

void physmap_write8(struct map_info *map, __u8 d, unsigned long adr)
{
	DBG("write8 at %x, %x\n", (unsigned)(map->map_priv_1 + adr), d);
	__raw_writeb(d, map->map_priv_1 + adr);
	mb();
}

void physmap_write16(struct map_info *map, __u16 d, unsigned long adr)
{
	DBG("write16 at %x, %x\n", (unsigned)(map->map_priv_1 + adr), d);
	__raw_writew(d, map->map_priv_1 + adr);
	mb();
}

void physmap_write32(struct map_info *map, __u32 d, unsigned long adr)
{
	DBG("write32 at %x, %x\n", (unsigned)(map->map_priv_1 + adr), d);
	__raw_writel(d, map->map_priv_1 + adr);
	mb();
}

void physmap_copy_to(struct map_info *map, unsigned long to, const void *from, ssize_t len)
{
	DBG("physmap_copy_to %x from %x\n", (unsigned)to, (unsigned)from);
	memcpy_toio(map->map_priv_1 + to, from, len);
}



static struct map_info mtx1_map = {
	name:		"MTX-1 flash",
	read8: physmap_read8,
	read16: physmap_read16,
	read32: physmap_read32,
	copy_from: physmap_copy_from,
	write8: physmap_write8,
	write16: physmap_write16,
	write32: physmap_write32,
	copy_to: physmap_copy_to,
};


static unsigned long flash_size = 0x01000000;
static unsigned char flash_buswidth = 4;
static struct mtd_partition mtx1_partitions[] = {
        {
                name: "user fs",
                size: 0x1c00000,
                offset: 0,
        },{
                name: "yamon",
                size: 0x0100000,
                offset: MTDPART_OFS_APPEND,
                mask_flags: MTD_WRITEABLE
        },{
                name: "raw kernel",
                size: 0x02c0000,
                offset: MTDPART_OFS_APPEND,
        },{
                name: "yamon env vars",
                size: 0x0040000,
                offset: MTDPART_OFS_APPEND,
                mask_flags: MTD_WRITEABLE
        }
};


#define NB_OF(x)  (sizeof(x)/sizeof(x[0]))

static struct mtd_partition *parsed_parts;
static struct mtd_info *mymtd;

int __init mtx1_mtd_init(void)
{
	struct mtd_partition *parts;
	int nb_parts = 0;
	char *part_type;
	
	/* Default flash buswidth */
	mtx1_map.buswidth = flash_buswidth;

	/*
	 * Static partition definition selection
	 */
	part_type = "static";
	parts = mtx1_partitions;
	nb_parts = NB_OF(mtx1_partitions);
	mtx1_map.size = flash_size;

	/*
	 * Now let's probe for the actual flash.  Do it here since
	 * specific machine settings might have been set above.
	 */
	printk(KERN_NOTICE "MTX-1 flash: probing %d-bit flash bus\n",
			mtx1_map.buswidth*8);
	mtx1_map.map_priv_1 =
		(unsigned long)ioremap(WINDOW_ADDR, WINDOW_SIZE);
	mymtd = do_map_probe("cfi_probe", &mtx1_map);
	if (!mymtd) return -ENXIO;
	mymtd->module = THIS_MODULE;

	add_mtd_partitions(mymtd, parts, nb_parts);
	return 0;
}

static void __exit mtx1_mtd_cleanup(void)
{
	if (mymtd) {
		del_mtd_partitions(mymtd);
		map_destroy(mymtd);
		if (parsed_parts)
			kfree(parsed_parts);
	}
}

module_init(mtx1_mtd_init);
module_exit(mtx1_mtd_cleanup);

MODULE_AUTHOR("Pete Popov");
MODULE_DESCRIPTION("MTX-1 CFI map driver");
MODULE_LICENSE("GPL");
