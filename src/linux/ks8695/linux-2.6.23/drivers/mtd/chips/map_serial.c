/*
 * Common code to handle map devices which are simple ROM
 * (C) 2000 Red Hat. GPL'd.
 * $Id: map_serial.c,v 1.3 2006/06/05 02:34:54 middle Exp $
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <asm/io.h>

#include <asm/byteorder.h>
#include <linux/errno.h>
#include <linux/slab.h>

#include <asm/hardware.h>
#include <linux/mtd/map.h>
#include <linux/mtd/mtd.h>
#include <linux/init.h> //add
#include <asm/arch/sl2312.h>
#include <asm/arch/flash.h>

static int mapserial_erase(struct mtd_info *mtd, struct erase_info *instr);
static int mapserial_read (struct mtd_info *, loff_t, size_t, size_t *, u_char *);
static int mapserial_write (struct mtd_info *, loff_t, size_t, size_t *, const u_char *);
static void mapserial_nop (struct mtd_info *);
struct mtd_info *map_serial_probe(struct map_info *map);

extern int m25p80_sector_erase(__u32 address, __u32 schip_en);

static struct mtd_chip_driver mapserial_chipdrv = {
	probe: map_serial_probe,
	name: "map_serial",
	module: THIS_MODULE
};

struct mtd_info *map_serial_probe(struct map_info *map)
{
	struct mtd_info *mtd;

	mtd = kmalloc(sizeof(*mtd), GFP_KERNEL);
	if (!mtd)
		return NULL;

	memset(mtd, 0, sizeof(*mtd));

	map->fldrv = &mapserial_chipdrv;
	mtd->priv = map;
	mtd->name = map->name;
	mtd->type = MTD_OTHER;
	mtd->erase = mapserial_erase;
	mtd->size = map->size;
	mtd->read = mapserial_read;
	mtd->write = mapserial_write;
	mtd->sync = mapserial_nop;
	mtd->flags = (MTD_WRITEABLE|MTD_ERASEABLE);
//	mtd->erasesize = 512; // page size;
#ifdef CONFIG_MTD_SL2312_SERIAL_ST
	mtd->erasesize = M25P80_SECTOR_SIZE; // block size;
#else
	mtd->erasesize = 0x1000; // block size;
#endif

	__module_get(THIS_MODULE);
	//MOD_INC_USE_COUNT;
	return mtd;
}

#define	FLASH_ACCESS_OFFSET	        		0x00000010
#define	FLASH_ADDRESS_OFFSET            		0x00000014
#define	FLASH_WRITE_DATA_OFFSET         		0x00000018
#define	FLASH_READ_DATA_OFFSET          		0x00000018

static __u32 readflash_ctrl_reg(__u32 ofs)
{
    __u32 *base;

    base = (__u32 *)IO_ADDRESS((SL2312_FLASH_CTRL_BASE + ofs));
    return __raw_readl(base);
}

static void writeflash_ctrl_reg(__u32 data, __u32 ofs)
{
    __u32 *base;

    base = (__u32 *)IO_ADDRESS((SL2312_FLASH_CTRL_BASE + ofs));
    __raw_writel(data, base);
}

static int mapserial_erase_block(struct map_info *map,unsigned int block)
{

	__u32 address;
#ifdef CONFIG_MTD_SL2312_SERIAL_ST

	if(!m25p80_sector_erase(block, 0))
		return (MTD_ERASE_DONE);
#else
      __u32 opcode;
      __u32 count=0;
//      __u8  status;

 //     printk("mapserial_erase_block : erase block %d \n",block);
//      opcode = 0x80000000 | FLASH_ACCESS_ACTION_SHIFT_ADDRESS | cmd;
      opcode = 0x80000000 | 0x0200 | 0x50;
      address = (block << 13);
      writeflash_ctrl_reg(address,FLASH_ADDRESS_OFFSET);
      writeflash_ctrl_reg(opcode,FLASH_ACCESS_OFFSET);
      opcode=readflash_ctrl_reg(FLASH_ACCESS_OFFSET);
      while(opcode&0x80000000)
      {
          opcode = readflash_ctrl_reg(FLASH_ACCESS_OFFSET);
          count++;
          if (count > 10000)
          {
            return (MTD_ERASE_FAILED);
          }
      }
      return (MTD_ERASE_DONE);
#endif
}

static int mapserial_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	struct map_info *map = (struct map_info *)mtd->priv;
    unsigned int    addr;
    int             len;
    unsigned int    block;
    unsigned int    ret=0;

	addr = instr->addr;
	len = instr->len;
    while (len > 0)
    {
        block = addr / mtd->erasesize;
#ifdef CONFIG_MTD_SL2312_SERIAL_ST
        ret = mapserial_erase_block(map,addr);
#else
		ret = mapserial_erase_block(map,block);
#endif
        addr = addr + mtd->erasesize;
        len = len - mtd->erasesize;
    }
    return (ret);
}

static int mapserial_read (struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf)
{
	struct map_info *map = (struct map_info *)mtd->priv;
//        printk("mapserial_read : \n");
	map->copy_from(map, buf, from, len);
	*retlen = len;
	return 0;
}

static void mapserial_nop(struct mtd_info *mtd)
{
	/* Nothing to see here */
}

static int mapserial_write (struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf)
{
	struct map_info *map = (struct map_info *)mtd->priv;
//	printk("mapserial_write : buf %x to %x len %x \n",(int)buf, (int)to, (int)len);
	//map->copy_to(map, buf, to, len);
	map->copy_to(map, to, buf, len);
	*retlen = len;
	return 0;
}

int __init map_serial_init(void)
{
	register_mtd_chip_driver(&mapserial_chipdrv);
	return 0;
}

static void __exit map_serial_exit(void)
{
	unregister_mtd_chip_driver(&mapserial_chipdrv);
}

module_init(map_serial_init);
module_exit(map_serial_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("David Woodhouse <dwmw2@infradead.org>");
MODULE_DESCRIPTION("MTD chip driver for ROM chips");
