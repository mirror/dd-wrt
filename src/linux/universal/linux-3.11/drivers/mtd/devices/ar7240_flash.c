/*
 * This file contains glue for Atheros ar7100 spi flash interface
 * Primitives are ar7100_spi_*
 * mtd flash implements are ar7100_flash_*
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include "../mtdcore.h"
#include <asm/delay.h>
#include <asm/io.h>
#include <linux/semaphore.h>
#include <linux/squashfs_fs.h>

#include "ar7240.h"
#include "ar7240_flash.h"

/* this is passed in as a boot parameter by bootloader */
//extern int __ath_flash_size;

/*
 * statics
 */
static void ar7240_spi_write_enable(void);
static void ar7240_spi_poll(void);
static void ar7240_spi_write_page(uint32_t addr, uint8_t * data, int len);
static void ar7240_spi_sector_erase(uint32_t addr);

#define down mutex_lock
#define up mutex_unlock
#define init_MUTEX mutex_init
#define DECLARE_MUTEX(a) struct mutex a

static DECLARE_MUTEX(ar7240_flash_sem);

/* GLOBAL FUNCTIONS */
void ar7240_flash_spi_down(void)
{
//printk(KERN_EMERG "spi down\n");
	down(&ar7240_flash_sem);
}

void ar7240_flash_spi_up(void)
{
//printk(KERN_EMERG "spi up\n");
	up(&ar7240_flash_sem);
}

EXPORT_SYMBOL(ar7240_flash_spi_down);
EXPORT_SYMBOL(ar7240_flash_spi_up);

#define AR7240_FLASH_SIZE_2MB          (2*1024*1024)
#define AR7240_FLASH_SIZE_4MB          (4*1024*1024)
#define AR7240_FLASH_SIZE_8MB          (8*1024*1024)
#define AR7240_FLASH_SIZE_16MB          (16*1024*1024)
#ifndef ST25P28
#define AR7240_FLASH_SECTOR_SIZE_64KB  (64*1024)
#else
#define AR7240_FLASH_SECTOR_SIZE_256KB  (256*1024)
#endif
#define AR7240_FLASH_SECTOR_SIZE_64KB  (64*1024)
#define AR7240_FLASH_PG_SIZE_256B       256
#define AR7240_FLASH_NAME               "ar7240-nor0"
/*
 * bank geometry
 */
typedef struct ar7240_flash_geom {
	uint32_t size;
	uint32_t sector_size;
	uint32_t nsectors;
	uint32_t pgsize;
} ar7240_flash_geom_t;

ar7240_flash_geom_t flash_geom_tbl[AR7240_FLASH_MAX_BANKS] = {
	{
#ifdef CONFIG_MTD_FLASH_16MB
	 .size = AR7240_FLASH_SIZE_16MB,
#elif CONFIG_MTD_FLASH_8MB
	 .size = AR7240_FLASH_SIZE_8MB,
#else
	 .size = AR7240_FLASH_SIZE_4MB,
#endif
	 .sector_size = AR7240_FLASH_SECTOR_SIZE_64KB,
	 .pgsize = AR7240_FLASH_PG_SIZE_256B}
};

static int ar7240_flash_probe()
{
	return 0;
}

static int zcom=0;
static int nocalibration=0;
static unsigned int zcomoffset = 0;
int guessbootsize(void *offset, unsigned int maxscan)
{
	unsigned int i,a;
	unsigned char *ofsb = (unsigned char *)offset;
	unsigned int *ofs = (unsigned int *)offset;
	maxscan -= 0x20000;
	maxscan /= 4;
	zcom=0;
	if (!strncmp((char *)(ofsb + 0x29da), "myloram.bin", 11) || !strncmp((char *)(ofsb + 0x2aba), "myloram.bin", 11)) {
		printk(KERN_EMERG "compex WP72E detected\n");
		nocalibration=1;
		return 0x30000;	// compex, lzma image
	}

	for (i = 0; i < maxscan; i += 16384) {
		if (ofs[i] == 0x6d000080) {
			printk(KERN_EMERG "redboot or compatible detected\n");
			return i * 4;	// redboot, lzma image
		}
		if (ofs[i] == 0x27051956) {
			printk(KERN_EMERG "uboot detected\n");
			return i * 4;	// uboot, lzma image
		}
		if (ofs[i] == 0x32303033) {
			printk(KERN_EMERG "WNR2000 uboot detected\n");
			return 0x50000;	// uboot, lzma image
		}
		if (ofs[i] == 0x32323030) {
			printk(KERN_EMERG "WNR2200 uboot detected\n");
			return 0x50000;	// uboot, lzma image
		}
		if (ofs[i] == 0x01000000 && ofs[i+1] == 0x44442d57) {
			printk(KERN_EMERG "tplink uboot detected\n");
			return i * 4;	// uboot, lzma image
		}
		if (ofs[i + 15] == 0x27051956) {
			printk(KERN_EMERG "WRT160NL uboot detected\n");
			return i * 4;	// uboot, lzma image
		}
		if (ofs[i] == SQUASHFS_MAGIC) {
			printk(KERN_EMERG "ZCom quirk found\n");
			zcom=1;
			for (a = i; a < maxscan; a += 16384) {
					if (ofs[a] == 0x27051956) {
					    printk(KERN_EMERG "ZCom quirk kernel offset %d\n",a*4);
					    zcomoffset = a * 4;
					}
    	
			}
			return i * 4;	// filesys starts earlier
		}
		
	}
	return -1;
}

static unsigned int guessflashsize(void *base)
{
	unsigned int size;
	unsigned int *guess = (unsigned int *)base;
	unsigned int max = 16 << 20;
//check 3 patterns since we can't write. 
	unsigned int p1 = guess[0];
	unsigned int p2 = guess[4096];
	unsigned int p3 = guess[8192];
	unsigned int c1;
	unsigned int c2;
	unsigned int c3;
	for (size = 2 << 20; size <= (max >> 1); size <<= 1) {
		unsigned int ofs = size / 4;
		c1 = guess[ofs];
		c2 = guess[ofs + 4096];
		c3 = guess[ofs + 8192];
		if (p1 == c1 && p2 == c2 && p3 == c3)	// mirror found
		{
			break;
		}
	}
	printk(KERN_EMERG "guessed flashsize = %dM\n", size >> 20);
	return size;

}

static int ar7240_flash_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	int nsect, s_curr, s_last;
	uint64_t  res;
	if (instr->addr + instr->len > mtd->size)
		return (-EINVAL);

	ar7240_flash_spi_down();

	res = instr->len;
	do_div(res, mtd->erasesize);
	nsect = res;
	if (((uint32_t)instr->len) % mtd->erasesize)
		nsect++;

	res = instr->addr;
	do_div(res,mtd->erasesize);
	s_curr = res;
	s_last = s_curr + nsect;

	do {
		ar7240_spi_sector_erase(s_curr * AR7240_SPI_SECTOR_SIZE);
	} while (++s_curr < s_last);

	ar7240_spi_done();

	ar7240_flash_spi_up();

	if (instr->callback) {
		instr->state = MTD_ERASE_DONE;
		instr->callback(instr);
	}
	return 0;
}

static int
ar7240_flash_read(struct mtd_info *mtd, loff_t from, size_t len,
		  size_t *retlen, u_char * buf)
{
	uint32_t addr = from | 0xbf000000;

//      printk(KERN_EMERG "read block %X:%X\n",from,len);
	if (!len)
		return (0);
	if (from + len > mtd->size)
		return (-EINVAL);

//      ar7240_flash_spi_down();

	memcpy(buf, (uint8_t *) (addr), len);
	*retlen = len;

//      ar7240_flash_spi_up();
//      printk(KERN_EMERG "read block %X:%X done\n",from,len);

	return 0;
}

static int
ar7240_flash_write(struct mtd_info *mtd, loff_t to, size_t len,
		   size_t *retlen, const u_char * buf)
{
	int total = 0, len_this_lp, bytes_this_page;
	uint32_t addr = 0;
	u_char *mem;
//      printk(KERN_EMERG "write block %X:%X\n",to,len);

	ar7240_flash_spi_down();

	while (total < len) {
		mem = buf + total;
		addr = to + total;
		bytes_this_page =
		    AR7240_SPI_PAGE_SIZE - (addr % AR7240_SPI_PAGE_SIZE);
		len_this_lp = min((len - total), bytes_this_page);

		ar7240_spi_write_page(addr, mem, len_this_lp);
		total += len_this_lp;
	}

	ar7240_spi_done();

	ar7240_flash_spi_up();

	*retlen = len;
	return 0;
}

static struct mtd_partition dir_parts[] = {
#ifdef CONFIG_MTD_FLASH_16MB
      {name: "RedBoot", offset: 0x30000, size:0x10000,},
	//, mask_flags: MTD_WRITEABLE, },
      {name: "linux", offset: 0x50000, size:0xf90000,},
#elif CONFIG_MTD_FLASH_8MB
      {name: "RedBoot", offset: 0x30000, size:0x10000,},
	//, mask_flags: MTD_WRITEABLE, },
      {name: "linux", offset: 0x50000, size:0x790000,},
#else
      {name: "RedBoot", offset: 0, size:0x40000,},
	//, mask_flags: MTD_WRITEABLE, },
      {name: "linux", offset: 0x40000, size:0x3a0000,},
#endif
      {name: "rootfs", offset: 0x0, size:0x2b0000,},
	//must be detected
      {name: "ddwrt", offset: 0x0, size:0x2b0000,},
	//must be detected
      {name: "nvram", offset: 0x3d0000, size:0x10000,},
      {name: "board_config", offset: 0x3f0000, size:0x10000,},
      {name: "fullflash", offset: 0x3f0000, size:0x10000,},
      {name: "fullboot", offset: 0, size:0x30000,},
      {name:NULL,},
};

struct fis_image_desc {
	unsigned char name[16];	// Null terminated name
	unsigned long flash_base;	// Address within FLASH of image
	unsigned long mem_base;	// Address in memory where it executes
	unsigned long size;	// Length of image
	unsigned long entry_point;	// Execution entry point
	unsigned long data_length;	// Length of actual data
	unsigned char _pad[256 - (16 + 7 * sizeof(unsigned long))];
	unsigned long desc_cksum;	// Checksum over image descriptor
	unsigned long file_cksum;	// Checksum over image data
};

/*
 * sets up flash_info and returns size of FLASH (bytes)
 */
static int __init ar7240_flash_init(void)
{
	int i;
	ar7240_flash_geom_t *geom;
	struct mtd_info *mtd;
	uint8_t index;
	int result = -1;
	char *buf;
	int offset = 0;
	struct squashfs_super_block *sb;
	size_t rootsize;
	size_t len;
	int fsize;
	init_MUTEX(&ar7240_flash_sem);

	ar7240_reg_wr_nf(AR7240_SPI_CLOCK, 0x43);
	buf = (char *)0xbf000000;
	fsize = guessflashsize(buf);
	for (i = 0; i < AR7240_FLASH_MAX_BANKS; i++) {

		index = ar7240_flash_probe();
		geom = &flash_geom_tbl[index];

		/* set flash size to value from bootloader if it passed valid value */
		/* otherwise use the default 4MB.                                   */
		//if (__ath_flash_size >= 4 && __ath_flash_size <= 16) 
		//    geom->size = __ath_flash_size * 1024 * 1024;

		mtd = kmalloc(sizeof(struct mtd_info), GFP_KERNEL);
		if (!mtd) {
			printk("Cant allocate mtd stuff\n");
			return -1;
		}
		memset(mtd, 0, sizeof(struct mtd_info));

		mtd->name = AR7240_FLASH_NAME;
		mtd->type = MTD_NORFLASH;
		mtd->flags = (MTD_CAP_NORFLASH | MTD_WRITEABLE);
		mtd->size = fsize;
		mtd->erasesize = geom->sector_size;
		mtd->numeraseregions = 0;
		mtd->eraseregions = NULL;
		mtd->owner = THIS_MODULE;
		mtd->_erase = ar7240_flash_erase;
		mtd->_read = ar7240_flash_read;
		mtd->_write = ar7240_flash_write;

		printk(KERN_EMERG "scanning for root partition\n");

		offset = 0;

		int guess = guessbootsize(buf, mtd->size);
		if (guess > 0) {
			printk(KERN_EMERG "guessed bootloader size = %X\n",
			       guess);
			dir_parts[0].offset = 0;
			dir_parts[0].size = guess;			
			dir_parts[7].size = guess;			
			dir_parts[1].offset = guess;
			dir_parts[1].size = 0;
		}
		while ((offset + mtd->erasesize) < mtd->size) {
//                      printk(KERN_EMERG "[0x%08X] = [0x%08X]!=[0x%08X]\n",offset,*((unsigned int *) buf),SQUASHFS_MAGIC);
			if (*((__u32 *)buf) == SQUASHFS_MAGIC) {
				printk(KERN_EMERG "\nfound squashfs at %X\n",
				       offset);
				sb = (struct squashfs_super_block *)buf;
				dir_parts[2].offset = offset;

				dir_parts[2].size = sb->bytes_used;
				len = dir_parts[2].offset + dir_parts[2].size;
				len += (mtd->erasesize - 1);
				len &= ~(mtd->erasesize - 1);
				dir_parts[2].size =
				    (len & 0x1ffffff) - dir_parts[2].offset;
				dir_parts[3].offset =
				    dir_parts[2].offset + dir_parts[2].size;

				dir_parts[5].offset = mtd->size - mtd->erasesize;	//fis config
				dir_parts[5].size = mtd->erasesize;
				#if defined(CONFIG_DIR825C1) && !defined(CONFIG_WDR4300)
				dir_parts[4].offset = dir_parts[5].offset - (mtd->erasesize*2);	//nvram
				dir_parts[4].size = mtd->erasesize;
				#else
				dir_parts[4].offset = dir_parts[5].offset - (mtd->erasesize - (nocalibration * mtd->erasesize));	//nvram
				dir_parts[4].size = mtd->erasesize;
				#endif
				dir_parts[3].size =
				    dir_parts[4].offset - dir_parts[3].offset;
				rootsize = dir_parts[4].offset - offset;	//size of rootfs aligned to nvram offset
				dir_parts[1].size =
				    (dir_parts[2].offset -
				     dir_parts[1].offset) + rootsize;
				//now scan for linux offset
				break;
			}
			offset += 4096;
			buf += 4096;
		}
		dir_parts[6].offset = 0;	// linux + nvram = phy size
		dir_parts[6].size = mtd->size;	// linux + nvram = phy size
		result = add_mtd_partitions(mtd, dir_parts, 8);
	}

	return 0;
}

static void __exit ar7240_flash_exit(void)
{
	/*
	 * nothing to do
	 */
}

/*
 * Primitives to implement flash operations
 */
static void ar7240_spi_write_enable()
{
	ar7240_reg_wr_nf(AR7240_SPI_FS, 1);
	ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_DIS);
	ar7240_spi_bit_banger(AR7240_SPI_CMD_WREN);
	ar7240_spi_go();
}

static void ar7240_spi_poll()
{
	int rd;

	do {
		ar7240_reg_wr_nf(AR7240_SPI_WRITE, AR7240_SPI_CS_DIS);
		ar7240_spi_bit_banger(AR7240_SPI_CMD_RD_STATUS);
		ar7240_spi_delay_8();
		rd = (ar7240_reg_rd(AR7240_SPI_RD_STATUS) & 1);
	} while (rd);
}

static void ar7240_spi_write_page(uint32_t addr, uint8_t * data, int len)
{
	int i;
	uint8_t ch;

	ar7240_spi_write_enable();
	ar7240_spi_bit_banger(AR7240_SPI_CMD_PAGE_PROG);
	ar7240_spi_send_addr(addr);

	for (i = 0; i < len; i++) {
		ch = *(data + i);
		ar7240_spi_bit_banger(ch);
	}

	ar7240_spi_go();
	ar7240_spi_poll();
}

static void ar7240_spi_sector_erase(uint32_t addr)
{
	ar7240_spi_write_enable();
	ar7240_spi_bit_banger(AR7240_SPI_CMD_SECTOR_ERASE);
	ar7240_spi_send_addr(addr);
	ar7240_spi_go();
//    display(0x7d);
	ar7240_spi_poll();
}

module_init(ar7240_flash_init);
module_exit(ar7240_flash_exit);
