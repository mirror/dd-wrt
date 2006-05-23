/*
 * Flash memory access on Alchemy HydrogenIII boards
 * 
 * (C) 2003 Pete Popov <ppopov@pacbell.net>
 * 
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

#define WINDOW_ADDR 0x1E000000
#define WINDOW_SIZE 0x02000000


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

static struct map_info hydrogen3_map = {
	name:		"HydrogenIII flash",
	read8: physmap_read8,
	read16: physmap_read16,
	read32: physmap_read32,
	copy_from: physmap_copy_from,
	write8: physmap_write8,
	write16: physmap_write16,
	write32: physmap_write32,
	copy_to: physmap_copy_to,
};

static unsigned char flash_buswidth = 4;

/* MTDPART_OFS_APPEND is vastly preferred to any attempt at statically lining
 * up the offsets. */
static struct mtd_partition hydrogen3_partitions[] = {
        {
                name: "User FS",
                size:   0x1c00000,
                offset: 0x0000000
        },{
                name: "yamon",
                size: 0x0100000,
                offset: MTDPART_OFS_APPEND,
                mask_flags: MTD_WRITEABLE
        },{
                name: "raw kernel",
                size: 0x02c0000,
                offset: MTDPART_OFS_APPEND
        }
};

#define NB_OF(x)  (sizeof(x)/sizeof(x[0]))

static struct mtd_partition *parsed_parts;
static struct mtd_info *mymtd;

int __init hydrogen3_mtd_init(void)
{
	struct mtd_partition *parts;
	int nb_parts = 0;
	char *part_type;
	
	/* Default flash buswidth */
	hydrogen3_map.buswidth = flash_buswidth;

	/*
	 * Static partition definition selection
	 */
	part_type = "static";
	parts = hydrogen3_partitions;
	nb_parts = NB_OF(hydrogen3_partitions);
	hydrogen3_map.size = WINDOW_SIZE;

	/*
	 * Now let's probe for the actual flash.  Do it here since
	 * specific machine settings might have been set above.
	 */
	printk(KERN_NOTICE "HydrogenIII flash: probing %d-bit flash bus\n", 
			hydrogen3_map.buswidth*8);
	hydrogen3_map.map_priv_1 = 
		(unsigned long)ioremap(WINDOW_ADDR, WINDOW_SIZE);
	mymtd = do_map_probe("cfi_probe", &hydrogen3_map);
	if (!mymtd) return -ENXIO;
	mymtd->module = THIS_MODULE;

	add_mtd_partitions(mymtd, parts, nb_parts);
	return 0;
}

static void __exit hydrogen3_mtd_cleanup(void)
{
	if (mymtd) {
		del_mtd_partitions(mymtd);
		map_destroy(mymtd);
		if (parsed_parts)
			kfree(parsed_parts);
	}
}

/*#ifndef MODULE

static int __init _bootflashonly(char *str)
{
  bootflashonly = simple_strtol(str, NULL, 0);
  return 1;
}


__setup("bootflashonly=", _bootflashonly);

#endif*/


module_init(hydrogen3_mtd_init);
module_exit(hydrogen3_mtd_cleanup);

MODULE_PARM(bootflashonly, "i");
MODULE_PARM_DESC(bootflashonly, "1=use \"boot flash only\"");
MODULE_AUTHOR("Pete Popov");
MODULE_DESCRIPTION("HydrogenIII mtd map driver");
MODULE_LICENSE("GPL");
