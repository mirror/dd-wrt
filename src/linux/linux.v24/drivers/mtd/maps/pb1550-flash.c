/*
 * Flash memory access on Alchemy Pb1550 board
 * 
 * (C) 2004 Embedded Edge, LLC, based on pb1550-flash.c:
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
#include <asm/pb1550.h>

#ifdef 	DEBUG_RW
#define	DBG(x...)	printk(x)
#else
#define	DBG(x...)	
#endif

static unsigned long window_addr;
static unsigned long window_size;

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

static struct map_info pb1550_map = {
	name:		"Pb1550 flash",
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

/* 
 * Support only 64MB NOR Flash parts
 */

#ifdef PB1550_BOTH_BANKS
/* both banks will be used. Combine the first bank and the first 
 * part of the second bank together into a single jffs/jffs2
 * partition.
 */
static struct mtd_partition pb1550_partitions[] = {
	/* assume boot[2:0]:swap is '0000' or '1000', which translates to:
	 * 1C00 0000 1FFF FFFF CE0 64MB Boot NOR Flash
	 * 1800 0000 1BFF FFFF CE0 64MB Param NOR Flash
	 */
        {
                name: "User FS",
                size:   (0x1FC00000 - 0x18000000),
                offset: 0x0000000
        },{
                name: "yamon",
                size: 0x0100000,
		offset:	MTDPART_OFS_APPEND,
                mask_flags: MTD_WRITEABLE
        },{
                name: "raw kernel",
		size: (0x300000 - 0x40000), /* last 256KB is yamon env */
		offset:	MTDPART_OFS_APPEND,
        }
};
#elif defined(PB1550_BOOT_ONLY)
static struct mtd_partition pb1550_partitions[] = {
	/* assume boot[2:0]:swap is '0000' or '1000', which translates to:
	 * 1C00 0000 1FFF FFFF CE0 64MB Boot NOR Flash
	 */
        {
                name: "User FS",
                size:   0x03c00000,
                offset: 0x0000000
        },{
                name: "yamon",
                size: 0x0100000,
		offset:	MTDPART_OFS_APPEND,
                mask_flags: MTD_WRITEABLE
        },{
                name: "raw kernel",
		size: (0x300000-0x40000), /* last 256KB is yamon env */
		offset:	MTDPART_OFS_APPEND,
        }
};
#elif defined(PB1550_USER_ONLY)
static struct mtd_partition pb1550_partitions[] = {
	/* assume boot[2:0]:swap is '0000' or '1000', which translates to:
	 * 1800 0000 1BFF FFFF CE0 64MB Param NOR Flash
	 */
        {
                name: "User FS",
                size:  (0x4000000 - 0x200000), /* reserve 2MB for raw kernel */
                offset: 0x0000000
        },{
                name: "raw kernel",
		size: MTDPART_SIZ_FULL,
		offset:	MTDPART_OFS_APPEND,
        }
};
#else
#error MTD_PB1550 define combo error /* should never happen */
#endif

#define NB_OF(x)  (sizeof(x)/sizeof(x[0]))

static struct mtd_partition *parsed_parts;
static struct mtd_info *mymtd;

/*
 * Probe the flash density and setup window address and size
 * based on user CONFIG options. There are times when we don't
 * want the MTD driver to be probing the boot or user flash,
 * so having the option to enable only one bank is important.
 */
int setup_flash_params()
{
	u16 boot_swapboot;
	boot_swapboot = (au_readl(MEM_STSTAT) & (0x7<<1)) | 
		((bcsr->status >> 6)  & 0x1);
	printk("Pb1550 MTD: boot:swap %d\n", boot_swapboot);

	switch (boot_swapboot) {
		case 0: /* 512Mbit devices, both enabled */
		case 1: 
		case 8:
		case 9: 
#if defined(PB1550_BOTH_BANKS)
			window_addr = 0x18000000;
			window_size = 0x8000000; 
#elif defined(PB1550_BOOT_ONLY)
			window_addr = 0x1C000000;
			window_size = 0x4000000; 
#else /* USER ONLY */
			window_addr = 0x1E000000;
			window_size = 0x1000000; 
#endif
			break;
		case 0xC:
		case 0xD:
		case 0xE:
		case 0xF: 
			/* 64 MB Boot NOR Flash is disabled */
			/* and the start address is moved to 0x0C00000 */
			window_addr = 0x0C000000;
			window_size = 0x4000000; 
		default:
			printk("Pb1550 MTD: unsupported boot:swap setting\n");
			return 1;
	}
	return 0;
}

int __init pb1550_mtd_init(void)
{
	struct mtd_partition *parts;
	int nb_parts = 0;
	char *part_type;
	
	/* Default flash buswidth */
	pb1550_map.buswidth = flash_buswidth;

	if (setup_flash_params()) 
		return -ENXIO;

	/*
	 * Static partition definition selection
	 */
	part_type = "static";
	parts = pb1550_partitions;
	nb_parts = NB_OF(pb1550_partitions);
	pb1550_map.size = window_size;

	/*
	 * Now let's probe for the actual flash.  Do it here since
	 * specific machine settings might have been set above.
	 */
	printk(KERN_NOTICE "Pb1550 flash: probing %d-bit flash bus\n", 
			pb1550_map.buswidth*8);
	pb1550_map.map_priv_1 = 
		(unsigned long)ioremap(window_addr, window_size);
	mymtd = do_map_probe("cfi_probe", &pb1550_map);
	if (!mymtd) return -ENXIO;
	mymtd->module = THIS_MODULE;

	add_mtd_partitions(mymtd, parts, nb_parts);
	return 0;
}

static void __exit pb1550_mtd_cleanup(void)
{
	if (mymtd) {
		del_mtd_partitions(mymtd);
		map_destroy(mymtd);
		if (parsed_parts)
			kfree(parsed_parts);
	}
}

module_init(pb1550_mtd_init);
module_exit(pb1550_mtd_cleanup);

MODULE_AUTHOR("Embedded Edge, LLC");
MODULE_DESCRIPTION("Pb1550 mtd map driver");
MODULE_LICENSE("GPL");
