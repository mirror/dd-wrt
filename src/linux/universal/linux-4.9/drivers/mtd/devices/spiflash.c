
/*
 * MTD driver for the SPI Flash Memory support.
 *
 * Copyright (c) 2005-2006 Atheros Communications Inc.
 * Copyright (C) 2006-2007 FON Technology, SL.
 * Copyright (C) 2006-2007 Imre Kaloz <kaloz@openwrt.org>
 * Copyright (C) 2006-2007 Felix Fietkau <nbd@openwrt.org>
 * Copyright (C) 2008 Sebastian Gottschall <s.gottschall@newmedia-net.de>
 *
 * This code is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

/*===========================================================================
** !!!!  VERY IMPORTANT NOTICE !!!!  FLASH DATA STORED IN LITTLE ENDIAN FORMAT
**
** This module contains the Serial Flash access routines for the Atheros SOC.
** The Atheros SOC integrates a SPI flash controller that is used to access
** serial flash parts. The SPI flash controller executes in "Little Endian"
** mode. THEREFORE, all WRITES and READS from the MIPS CPU must be
** BYTESWAPPED! The SPI Flash controller hardware by default performs READ
** ONLY byteswapping when accessed via the SPI Flash Alias memory region
** (Physical Address 0x0800_0000 - 0x0fff_ffff). The data stored in the
** flash sectors is stored in "Little Endian" format.
**
** The spiflash_write() routine performs byteswapping on all write
** operations.
**===========================================================================*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/magic.h>
#include <linux/root_dev.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <asm/delay.h>
#include <asm/io.h>
#include "spiflash.h"
#include "../mtdcore.h"

#ifndef __BIG_ENDIAN
#error This driver currently only works with big endian CPU.
#endif

#define MAX_PARTS 32

#define SPIFLASH "spiflash: "

#define MIN(a,b)        ((a) < (b) ? (a) : (b))

#define busy_wait(condition, wait) \
	do { \
		while (condition) { \
			spin_unlock_bh(&spidata->mutex); \
			if (wait > 1) \
				msleep(wait); \
			else if ((wait == 1) && need_resched()) \
				schedule(); \
			else \
				udelay(1); \
			spin_lock_bh(&spidata->mutex); \
		} \
	} while (0)
		

static __u32 spiflash_regread32(int reg);
static void spiflash_regwrite32(int reg, __u32 data);
static __u32 spiflash_sendcmd (int op, u32 addr);

int __init spiflash_init (void);
void __exit spiflash_exit (void);
static int spiflash_probe_chip (void);
static int spiflash_erase (struct mtd_info *mtd,struct erase_info *instr);
static int spiflash_read (struct mtd_info *mtd, loff_t from,size_t len,size_t *retlen,u_char *buf);
static int spiflash_write (struct mtd_info *mtd,loff_t to,size_t len,size_t *retlen,const u_char *buf);

/* Flash configuration table */
struct flashconfig {
    __u32 byte_cnt;
    __u32 sector_cnt;
    __u32 sector_size;
    __u32 cs_addrmask;
} flashconfig_tbl[MAX_FLASH] =
    {
        { 0, 0, 0, 0},
        { STM_1MB_BYTE_COUNT, STM_1MB_SECTOR_COUNT, STM_1MB_SECTOR_SIZE, 0x0},
        { STM_2MB_BYTE_COUNT, STM_2MB_SECTOR_COUNT, STM_2MB_SECTOR_SIZE, 0x0},
        { STM_4MB_BYTE_COUNT, STM_4MB_SECTOR_COUNT, STM_4MB_SECTOR_SIZE, 0x0},
        { STM_8MB_BYTE_COUNT, STM_8MB_SECTOR_COUNT, STM_8MB_SECTOR_SIZE, 0x0},
        { STM_16MB_BYTE_COUNT, STM_16MB_SECTOR_COUNT, STM_16MB_SECTOR_SIZE, 0x0}
    };

/* Mapping of generic opcodes to STM serial flash opcodes */
#define SPI_WRITE_ENABLE    0
#define SPI_WRITE_DISABLE   1
#define SPI_RD_STATUS       2
#define SPI_WR_STATUS       3
#define SPI_RD_DATA         4
#define SPI_FAST_RD_DATA    5
#define SPI_PAGE_PROGRAM    6
#define SPI_SECTOR_ERASE    7
#define SPI_BULK_ERASE      8
#define SPI_DEEP_PWRDOWN    9
#define SPI_RD_SIG          10
#define SPI_MAX_OPCODES     11

struct opcodes {
    __u16 code;
    __s8 tx_cnt;
    __s8 rx_cnt;
} stm_opcodes[] = {
        {STM_OP_WR_ENABLE, 1, 0},
        {STM_OP_WR_DISABLE, 1, 0},
        {STM_OP_RD_STATUS, 1, 1},
        {STM_OP_WR_STATUS, 1, 0},
        {STM_OP_RD_DATA, 4, 4},
        {STM_OP_FAST_RD_DATA, 5, 0},
        {STM_OP_PAGE_PGRM, 8, 0},
        {STM_OP_SECTOR_ERASE, 4, 0},
        {STM_OP_BULK_ERASE, 1, 0},
        {STM_OP_DEEP_PWRDOWN, 1, 0},
        {STM_OP_RD_SIG, 4, 1},
};

/* Driver private data structure */
struct spiflash_data {
	struct 	mtd_info       *mtd;	
	struct 	mtd_partition  *parsed_parts;     /* parsed partitions */
	void 	*readaddr; /* memory mapped data for read  */
	void 	*mmraddr;  /* memory mapped register space */
	wait_queue_head_t wq;
	spinlock_t mutex;
	int state;
};
enum {
	FL_READY,
	FL_READING,
	FL_ERASING,
	FL_WRITING
};

static struct spiflash_data *spidata;

extern int parse_redboot_partitions(struct mtd_info *master, struct mtd_partition **pparts);

#ifdef CONFIG_MTD_SPIFLASH_PP
/*
 * With AR2317, WRG-G19, we add the external circuit to implement page
 * programming. The GPIO 0 is used to control the chip select of the SPI
 * interface. The chip select is low active.
 *
 *								david_hsieh@alphanetworks.com
 */

/* The following part is cut from arch/mips/ar531x/ar531x.h */

#include <asm/addrspace.h>

#define AR5315_DSLBASE          0xB1000000      /* RESET CONTROL MMR */

/* GPIO */
#define AR5315_GPIO_DI          (AR5315_DSLBASE + 0x0088)
#define AR5315_GPIO_DO          (AR5315_DSLBASE + 0x0090)
#define AR5315_GPIO_CR          (AR5315_DSLBASE + 0x0098)
#define AR5315_GPIO_INT         (AR5315_DSLBASE + 0x00a0)

/* Chip Select GPIO for Page Programming */
#ifndef CONFIG_MTD_SPIFLASH_PP_GPIO
#define CONFIG_MTD_SPIFLASH_PP_GPIO 0
#endif
#define SPI_CS_BIT_MASK (1 << CONFIG_MTD_SPIFLASH_PP_GPIO)

typedef unsigned int AR531X_REG;
#define sysRegRead(phys)		(*(volatile AR531X_REG *)KSEG1ADDR(phys))
#define sysRegWrite(phys, val)	((*(volatile AR531X_REG *)KSEG1ADDR(phys)) = (val))

static atomic_t spiflash_cs = ATOMIC_INIT(0);

static inline void chip_select(int value)
{
	__u32 reg;

	/* Set GPIO 0 as output. */
	reg = sysRegRead(AR5315_GPIO_CR);
	reg |= SPI_CS_BIT_MASK;
	sysRegWrite(AR5315_GPIO_CR, reg);

	/* Set GPIO 0 data. */
	reg = sysRegRead(AR5315_GPIO_DO);
	if (value) reg |= SPI_CS_BIT_MASK;
	else reg &= ~SPI_CS_BIT_MASK;
	sysRegWrite(AR5315_GPIO_DO, reg);
}

#define SET_SPI_ACTIVITY()						\
{												\
}

#define CLEAR_SPI_ACTIVITY()					\
{												\
	chip_select(1);								\
}

#else

#define SET_SPI_ACTIVITY()
#define CLEAR_SPI_ACTIVITY()

#endif



/***************************************************************************************************/

static __u32
spiflash_regread32(int reg)
{
	volatile __u32 *data = (__u32 *)(spidata->mmraddr + reg);

	return (*data);
}

static void 
spiflash_regwrite32(int reg, __u32 data)
{
	volatile __u32 *addr = (__u32 *)(spidata->mmraddr + reg);

	*addr = data;
	return;
}


static __u32 
spiflash_sendcmd (int op, u32 addr)
{
	 u32 reg;
	 u32 mask;
	struct opcodes *ptr_opcode;

	ptr_opcode = &stm_opcodes[op];
	busy_wait((reg = spiflash_regread32(SPI_FLASH_CTL)) & SPI_CTL_BUSY, 0);
	spiflash_regwrite32(SPI_FLASH_OPCODE, ((u32) ptr_opcode->code) | (addr << 8));

	reg = (reg & ~SPI_CTL_TX_RX_CNT_MASK) | ptr_opcode->tx_cnt |
        	(ptr_opcode->rx_cnt << 4) | SPI_CTL_START;

	spiflash_regwrite32(SPI_FLASH_CTL, reg);

	busy_wait(spiflash_regread32(SPI_FLASH_CTL) & SPI_CTL_BUSY, 0);
 
	if (!ptr_opcode->rx_cnt)
		return 0;

	reg = (__u32) spiflash_regread32(SPI_FLASH_DATA);

	switch (ptr_opcode->rx_cnt) {
	case 1:
			mask = 0x000000ff;
			break;
	case 2:
			mask = 0x0000ffff;
			break;
	case 3:
			mask = 0x00ffffff;
			break;
	default:
			mask = 0xffffffff;
			break;
	}
	reg &= mask;

	return reg;
}



/* Probe SPI flash device
 * Function returns 0 for failure.
 * and flashconfig_tbl array index for success.
 */
static int 
spiflash_probe_chip (void)
{
	__u32 sig;
   	int flash_size;
	
   	/* Read the signature on the flash device */
	spin_lock_bh(&spidata->mutex);
   	sig = spiflash_sendcmd(SPI_RD_SIG, 0);
	spin_unlock_bh(&spidata->mutex);

   	switch (sig) {
   	case STM_8MBIT_SIGNATURE:
            	flash_size = FLASH_1MB;
        	break;
        case STM_16MBIT_SIGNATURE:
            	flash_size = FLASH_2MB;
            	break;
        case STM_32MBIT_SIGNATURE:
            	flash_size = FLASH_4MB;
            	break;
        case STM_64MBIT_SIGNATURE:
            	flash_size = FLASH_8MB;
            	break;
        case STM_128MBIT_SIGNATURE:
            	flash_size = FLASH_16MB;
            	break;
        default:
	    	printk (KERN_WARNING SPIFLASH "Read of flash device signature failed!\n");
            	return (0);
   	}

   	return (flash_size);
}


/* wait until the flash chip is ready and grab a lock */
static int spiflash_wait_ready(int state)
{
	DECLARE_WAITQUEUE(wait, current);

retry:
	spin_lock_bh(&spidata->mutex);
	if (spidata->state != FL_READY) {
		set_current_state(TASK_UNINTERRUPTIBLE);
		add_wait_queue(&spidata->wq, &wait);
		spin_unlock_bh(&spidata->mutex);
		schedule();
		remove_wait_queue(&spidata->wq, &wait);
		
		if(signal_pending(current))
			return 0;

	
		goto retry;
	}
	spidata->state = state;

	return 1;
}

static inline void spiflash_done(void)
{
	spidata->state = FL_READY;
	spin_unlock_bh(&spidata->mutex);
	wake_up(&spidata->wq);
}

static int 
spiflash_erase (struct mtd_info *mtd,struct erase_info *instr)
{
	struct opcodes *ptr_opcode;
	__u32 temp, reg;
	int finished = 0;
	unsigned int addr = instr->addr;

#ifdef SPIFLASH_DEBUG
   	printk (KERN_DEBUG "%s(addr = 0x%.8x, len = %d)\n",__FUNCTION__,instr->addr,instr->len);
#endif

   	/* sanity checks */
   	if (instr->addr + instr->len > mtd->size) return (-EINVAL);
	if (!spiflash_wait_ready(FL_ERASING))
		return -EINTR;
for (addr=instr->addr;addr<instr->addr+instr->len;addr+=mtd->erasesize)
{

	ptr_opcode = &stm_opcodes[SPI_SECTOR_ERASE];

	temp = ((__u32)addr << 8) | (__u32)(ptr_opcode->code);
	spiflash_sendcmd(SPI_WRITE_ENABLE,0);
	busy_wait((reg = spiflash_regread32(SPI_FLASH_CTL)) & SPI_CTL_BUSY, 0);

	spiflash_regwrite32(SPI_FLASH_OPCODE, temp);

	reg = (reg & ~SPI_CTL_TX_RX_CNT_MASK) | ptr_opcode->tx_cnt | SPI_CTL_START;
	spiflash_regwrite32(SPI_FLASH_CTL, reg);

	busy_wait(spiflash_sendcmd(SPI_RD_STATUS, 0) & SPI_STATUS_WIP, 20);
}
	spiflash_done();

   	instr->state = MTD_ERASE_DONE;
   	if (instr->callback) instr->callback (instr);
#ifdef SPIFLASH_DEBUG
   	printk (KERN_DEBUG "%s return\n",__FUNCTION__);
#endif
   	return (0);
}

static int 
spiflash_read (struct mtd_info *mtd, loff_t from,size_t len,size_t *retlen,u_char *buf)
{
	u8 *read_addr;
	
   	/* sanity checks */
   	if (!len) return (0);
   	if (from + len > mtd->size) return (-EINVAL);
	
   	/* we always read len bytes */
   	*retlen = len;

	if (!spiflash_wait_ready(FL_READING))
		return -EINTR;
	read_addr = (u8 *)(spidata->readaddr + from);
	memcpy(buf, read_addr, len);
	spiflash_done();

   	return 0;
}

static int 
spiflash_write (struct mtd_info *mtd,loff_t to,size_t len,size_t *retlen,const u_char *buf)
{
	u32 opcode, bytes_left;

   	*retlen = 0;

   	/* sanity checks */
   	if (!len) return (0);
   	if (to + len > mtd->size) return (-EINVAL);
	
	opcode = stm_opcodes[SPI_PAGE_PROGRAM].code;
	bytes_left = len;
	
	do {
		u32 xact_len, reg, page_offset, spi_data = 0;

		xact_len = MIN(bytes_left, sizeof(__u32));

		/* 32-bit writes cannot span across a page boundary
		 * (256 bytes). This types of writes require two page
		 * program operations to handle it correctly. The STM part
		 * will write the overflow data to the beginning of the
		 * current page as opposed to the subsequent page.
		 */
		page_offset = (to & (STM_PAGE_SIZE - 1)) + xact_len;

		if (page_offset > STM_PAGE_SIZE) {
			xact_len -= (page_offset - STM_PAGE_SIZE);
		}

		if (!spiflash_wait_ready(FL_WRITING))
			return -EINTR;

		spiflash_sendcmd(SPI_WRITE_ENABLE, 0);
		switch (xact_len) {
			case 1:
			 	spi_data = (u32) ((u8) *buf);
				break;
			case 2:
				spi_data = (buf[1] << 8) | buf[0];
				break;
			case 3:
				spi_data = (buf[2] << 16) | (buf[1] << 8) | buf[0];
				break;
			case 4:
				spi_data = (buf[3] << 24) | (buf[2] << 16) | 
							(buf[1] << 8) | buf[0];
				break;
			default:
				spi_data = 0;
				break;
		}

		spiflash_regwrite32(SPI_FLASH_DATA, spi_data);
		opcode = (opcode & SPI_OPCODE_MASK) | ((__u32)to << 8);
		spiflash_regwrite32(SPI_FLASH_OPCODE, opcode);

		reg = spiflash_regread32(SPI_FLASH_CTL);
		reg = (reg & ~SPI_CTL_TX_RX_CNT_MASK) | (xact_len + 4) | SPI_CTL_START;
		spiflash_regwrite32(SPI_FLASH_CTL, reg);

		/* give the chip some time before we start busy waiting */
		spin_unlock_bh(&spidata->mutex);
		schedule();
		spin_lock_bh(&spidata->mutex);

		busy_wait(spiflash_sendcmd(SPI_RD_STATUS, 0) & SPI_STATUS_WIP, 0);
		spiflash_done();

		bytes_left -= xact_len;
		to += xact_len;
		buf += xact_len;

   		*retlen += xact_len;
	} while (bytes_left != 0);

   	return 0;
}

#ifdef CONFIG_MTD_SPIFLASH_PP

static void page_write(loff_t to, const u_char * buf)
{
	__u32	reg, spi_data, opcode;
	int		i;


	/* We are going to write flash now, do write enable first. */
	spiflash_sendcmd(SPI_WRITE_ENABLE, 0);

	/* we are not really waiting for CPU spiflash activity, just need the value of the register. */
	busy_wait((reg = spiflash_regread32(SPI_FLASH_CTL)) & SPI_CTL_BUSY, 0);

	/* Prepare SPI opcode, data and control register values. */
	opcode   = (stm_opcodes[SPI_PAGE_PROGRAM].code & SPI_OPCODE_MASK) | ((__u32)to << 8);
	spi_data = (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0]; buf += 4;
	reg      = (reg & ~SPI_CTL_TX_RX_CNT_MASK) | 0x8 | SPI_CTL_START;

	/* wait and mark our activity */
	if (!spiflash_wait_ready(FL_WRITING))
		return -EINTR;
	SET_SPI_ACTIVITY();
	chip_select(0);

	/* Send out the the first 4 bytes. */
	spiflash_regwrite32(SPI_FLASH_DATA, spi_data);
	spiflash_regwrite32(SPI_FLASH_OPCODE, opcode);
	spiflash_regwrite32(SPI_FLASH_CTL, reg);

	/* 31 loops, each loop send 8 bytes */
	for (i=0; i<31; i++)
	{
		busy_wait((reg = spiflash_regread32(SPI_FLASH_CTL)) & SPI_CTL_BUSY, 0);

		/*
		 * The sample code from the application node is:
		 *
		 *	spi_data = (UINT32)*((UINT32 *)buf);
		 *	spi_data = cpi2le32(spi_data);
		 *	spi_data_swapped =
		 *			(((spi_data>>8) & 0xff) << 24) |
		 *			(((spi_data>>24)& 0xff) << 8) |
		 *			(spi_data & 0x00ff00ff);
		 */
		opcode   = (buf[3] <<  8) | (buf[2] << 16) | (buf[1] << 24) | buf[0]; buf += 4;
		spi_data = (buf[3] << 24) | (buf[2] << 16) | (buf[1] <<  8) | buf[0]; buf += 4;
		reg      = (reg & ~SPI_CTL_TX_RX_CNT_MASK) | 0x8 | SPI_CTL_START;

		spiflash_regwrite32(SPI_FLASH_DATA, spi_data);
		spiflash_regwrite32(SPI_FLASH_OPCODE, opcode);
		spiflash_regwrite32(SPI_FLASH_CTL, reg);
	}

	/* send out the last 4 bytes */
	busy_wait((reg = spiflash_regread32(SPI_FLASH_CTL)) & SPI_CTL_BUSY, 0);

	opcode   = (buf[3] <<  8) | (buf[2] << 16) | (buf[1] << 24) | buf[0]; buf += 4;
	reg      = (reg & ~SPI_CTL_TX_RX_CNT_MASK) | 0x4 | SPI_CTL_START;

	spiflash_regwrite32(SPI_FLASH_OPCODE, opcode);
	spiflash_regwrite32(SPI_FLASH_CTL, reg);

	busy_wait((reg = spiflash_regread32(SPI_FLASH_CTL)) & SPI_CTL_BUSY, 0);

	/* Deactive chip select */
	chip_select(1);
	/* clean our activity */
	CLEAR_SPI_ACTIVITY();
	

	busy_wait(spiflash_sendcmd(SPI_RD_STATUS, 0) & SPI_STATUS_WIP, 20);
	spiflash_done();
	return;
}

/* 
 * Do page programming test.
 * The 'block' should be erase already.
 * We try to use page programming mode to write flash,
 * and erase this block again before return.
 */
static int test_page_programming(struct mtd_info * mtd, loff_t block)
{
	unsigned char	buffer[256];
	unsigned char *	flash;
	struct opcodes *ptr_opcode;
	__u32			opcode, reg;
	int				i;


	/* write the flash with known pattern */
	for (i=0; i<256; i++) buffer[i] = (unsigned char)i;
	page_write(block, buffer);
	if (!spiflash_wait_ready(FL_WRITING))
		return -EINTR;

	/* wait and mark our activity */
	SET_SPI_ACTIVITY();
	
	/* read it back and check pattern */
	flash = (unsigned char *)(spidata->readaddr + block);
	printk(KERN_INFO "%s(): checking @ 0x%.8x ...\n",__FUNCTION__,(__u32)flash);
	for (i = 0; i < 8; i++)
	{
		if (flash[i*4] != (unsigned char)(i*4))
		{
			printk(KERN_INFO "unexpected value @ %d: 0x%02x !!\n", i*4, flash[i*4]);
			break;
		}
	}

	/* clean our activity */
	CLEAR_SPI_ACTIVITY();
	udelay(10);
	
	/* erase this block before return */
	printk(KERN_INFO "%s(): erasing block 0x%.8x ...\n",__FUNCTION__,(__u32)block);

	/* we are going to erase sector, do write enable first */
	spiflash_sendcmd(SPI_WRITE_ENABLE, 0);

	/* wait and mark our activity */
	SET_SPI_ACTIVITY();

	/* we are not really waiting for CPU spiflash activity, just need the value of the register. */
	busy_wait((reg = spiflash_regread32(SPI_FLASH_CTL)) & SPI_CTL_BUSY, 0);

	/* send sector erase op. */
	ptr_opcode = &stm_opcodes[SPI_SECTOR_ERASE];
	opcode = ((__u32)ptr_opcode->code) | ((__u32)block << 8);
	spiflash_regwrite32(SPI_FLASH_OPCODE, opcode);
	reg = (reg & ~SPI_CTL_TX_RX_CNT_MASK) | ptr_opcode->tx_cnt | SPI_CTL_START;
	spiflash_regwrite32(SPI_FLASH_CTL, reg);

	/* wait for CPU spiflash activity */
	busy_wait((reg = spiflash_regread32(SPI_FLASH_CTL)) & SPI_CTL_BUSY, 0);
	/* clean our activity */
	CLEAR_SPI_ACTIVITY();
	udelay(10);

	busy_wait(spiflash_sendcmd(SPI_RD_STATUS, 0) & SPI_STATUS_WIP, 20);
	spiflash_done();
	printk("SPI flash write test done (%d)!, page programming is %s!\n", i, i<8 ? "disabled":"enabled");
	return (i<8 ? 1:0);
}

static int pp_mode = -1;
static int pp_enable = 1;

/* implementation for spiflash page programing. */
static int spiflash_page_write(struct mtd_info * mtd,
		loff_t to, size_t len, size_t * retlen, const u_char * buf)
{
	size_t bytes_left = len;
	size_t xact_len;
	size_t written;
	size_t offset;


	/* If we already test page programming and failed,
	 * fall back to spiflash_write() directly. */
	if (pp_mode > 0) return spiflash_write(mtd, to, len, retlen, buf);

	*retlen = 0;
	if (to + len > mtd->size) return (-EINVAL);

	while (bytes_left > 0)
	{
		offset = to % STM_PAGE_SIZE;
		xact_len = MIN(bytes_left, STM_PAGE_SIZE - offset);
		if (offset > 0 || xact_len < STM_PAGE_SIZE)
		{
			spiflash_write(mtd, to, xact_len, &written, buf);
		}
		else
		{
			/* test page program mode, if we did not test it before. */
			if (pp_mode < 0) pp_mode = test_page_programming(mtd, to);

			if (pp_enable && (pp_mode == 0)) page_write(to, buf);
			else spiflash_write(mtd, to, xact_len, &written, buf);
		}
		to += xact_len;
		bytes_left -= xact_len;
		buf += xact_len;
		*retlen += xact_len;
	}

	return 0;
}

static int __my_atoi(const char * buf)
{
	int ret = 0;
	while (*buf)
	{
		if (*buf >= '0' && *buf <= '9') ret += (int)(*buf - '0');
		buf++;
	}
	return ret;
}

static int proc_read_pp_enable(char * buf, char ** start, off_t offset,
		int len, int * eof, void * data)
{
	char * p = buf;
	p += sprintf(p, "%d\n", pp_enable);
	*eof = 1;
	return p - buf;
}

static int proc_write_pp_enable(struct file * file, const char * buf,
		unsigned long count, void * data)
{
	pp_enable = __my_atoi(buf);
	printk("spiflash: %s page programming!\n", pp_enable ? "enable" : "disable");
	if (pp_mode >= 0) printk("spiflash: H/W is %scapable of doing page programming!\n", pp_mode ? "not " : "");
	return count;
}

static struct proc_dir_entry * root = NULL;
static struct proc_dir_entry * pp_enable_entry = NULL;

static int register_spi_proc(void)
{
	root = proc_mkdir("spiflash", NULL);
	if (root == NULL)
	{
		printk("spiflash: fail to create /proc/spiflash !!\n");
		return -1;
	}
	pp_enable_entry = create_proc_entry("pp_enable", 0644, root);
	if (pp_enable_entry == NULL)
	{
		printk("spiflash: fail to create /proc/spiflash/pp_enable !!\n");
		remove_proc_entry("spiflash", root);
		root = NULL;
		return -1;
	}
	pp_enable_entry->data = 0;
	pp_enable_entry->read_proc = proc_read_pp_enable;
	pp_enable_entry->write_proc = proc_write_pp_enable;
	pp_enable_entry->owner = THIS_MODULE;
	printk("spiflash: /proc/spiflash/pp_enable created !!\n");
	return 0;
}

static void remove_spi_proc(void)
{
	if (pp_enable_entry) remove_proc_entry("pp_enable", root);
	if (root) remove_proc_entry("spiflash", root);
	pp_enable_entry = NULL;
	root = NULL;
}

#endif

#ifdef CONFIG_MTD
static const char *part_probe_types[] = { "cmdlinepart", "RedBoot", NULL };
#endif


static int spiflash_probe(struct platform_device *pdev)
{
   	int result = -1;
   	int index, num_parts;
	struct mtd_info *mtd;

	spidata->mmraddr = ioremap_nocache(SPI_FLASH_MMR, SPI_FLASH_MMR_SIZE);
	spin_lock_init(&spidata->mutex);
	init_waitqueue_head(&spidata->wq);
	spidata->state = FL_READY;
	
	if (!spidata->mmraddr) {
  		printk (KERN_WARNING SPIFLASH "Failed to map flash device\n");
		kfree(spidata);
		spidata = NULL;
	}

   	mtd = kzalloc(sizeof(struct mtd_info), GFP_KERNEL);
   	if (!mtd) {
		kfree(spidata);
		return -ENXIO;
	}
	
   	if (!(index = spiflash_probe_chip())) {
    	printk (KERN_WARNING SPIFLASH "Found no serial flash device\n");
		goto error;
   	}

	spidata->readaddr = ioremap_nocache(SPI_FLASH_READ, flashconfig_tbl[index].byte_cnt);
	if (!spidata->readaddr) {
		printk (KERN_WARNING SPIFLASH "Failed to map flash device\n");
		goto error;
	}

   	mtd->name = "spiflash";
   	mtd->type = MTD_NORFLASH;
   	mtd->flags = (MTD_CAP_NORFLASH|MTD_WRITEABLE);
   	mtd->size = flashconfig_tbl[index].byte_cnt;
   	mtd->erasesize = flashconfig_tbl[index].sector_size;
	mtd->writesize = 1;
   	mtd->numeraseregions = 0;
   	mtd->eraseregions = NULL;
   	mtd->_erase = spiflash_erase;
   	mtd->_read = spiflash_read;
#ifdef CONFIG_MTD_SPIFLASH_PP
	mtd->_write = spiflash_page_write;
#else
	mtd->_write = spiflash_write;
#endif
	mtd->owner = THIS_MODULE;

   	/* parse redboot partitions */
	num_parts = parse_mtd_partitions(mtd, part_probe_types, &spidata->parsed_parts, 0);
	if (!num_parts)
		goto error;

	result = add_mtd_partitions(mtd, spidata->parsed_parts, num_parts);

#ifdef CONFIG_MTD_SPIFLASH_PP
		register_spi_proc();
#endif


	spidata->mtd = mtd;
	
   	return (result);
	
error:
	kfree(mtd);
	kfree(spidata);
	return -ENXIO;
}

static int spiflash_remove (struct platform_device *pdev)
{
	del_mtd_partitions (spidata->mtd);
	kfree(spidata->mtd);
	return 0;
}

struct platform_driver spiflash_driver = {
	.driver.name = "spiflash",
	.probe = spiflash_probe,
	.remove = spiflash_remove,
};

int __init 
spiflash_init (void)
{
   	spidata = kmalloc(sizeof(struct spiflash_data), GFP_KERNEL);
  	if (!spidata)
		return (-ENXIO);

	spin_lock_init(&spidata->mutex);
	platform_driver_register(&spiflash_driver);

	return 0;
}

void __exit 
spiflash_exit (void)
{
	kfree(spidata);
#ifdef CONFIG_MTD_SPIFLASH_PP
	remove_spi_proc();
#endif
}

module_init (spiflash_init);
module_exit (spiflash_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("OpenWrt.org, Atheros Communications Inc");
MODULE_DESCRIPTION("MTD driver for SPI Flash on Atheros SOC");

