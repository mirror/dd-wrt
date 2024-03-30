
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
#include <linux/magics.h>
#include <linux/root_dev.h>
#include <linux/vmalloc.h>
#include <linux/magic.h>
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
	FL_WRITING,
	FL_SPI
};

static struct mtd_partition dir_parts[] = {
        { name: "RedBoot", offset: 0, size: 0x30000, },//, mask_flags: MTD_WRITEABLE, },
        { name: "linux", offset: 0x30000, size: 0x390000, },
        { name: "rootfs", offset: 0x0, size: 0x2b0000,}, //must be detected
        { name: "ddwrt", offset: 0x0, size: 0x2b0000,}, //must be detected
        { name: "nvram", offset: 0x3d0000, size: 0x10000, },
        { name: "FIS directory", offset: 0x3e0000, size: 0x10000, },
        { name: "board_config", offset: 0x3f0000, size: 0x10000, },
        { name: "fullflash", offset: 0x3f0000, size: 0x10000, },
        { name: NULL, },
};



static struct spiflash_data *spidata;

extern int parse_redboot_partitions(struct mtd_info *master, struct mtd_partition **pparts);




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

#ifdef CONFIG_MTD_SPIFLASH_PP
static void sysGpioCtrlInput(int gpio_bit)
{
	unsigned int tmpVal;
	tmpVal = *(volatile int *)(0xb1000098);
	tmpVal &= ~(1<<gpio_bit);
	tmpVal |=(0<<gpio_bit);
	*(volatile int *)(0xb1000098) = tmpVal;  
}

static void sysGpioCtrlOutput(int gpio_bit)
{
	unsigned int tmpVal;
	tmpVal = *(volatile int *)(0xb1000098);
	tmpVal &= ~(1<<gpio_bit);
	tmpVal |=(1<<gpio_bit);
	*(volatile int *)(0xb1000098) = tmpVal;  
}

static void sysGpioSet(int gpio_bit, int val)
{
    unsigned int reg;

    reg = *(volatile int *)(0xb1000090);
    reg &= ~(1 << gpio_bit);
    reg |= (val&1) << gpio_bit;
    *(volatile int *)(0xb1000090) = reg;
}

#endif

static int 
spiflash_erase (struct mtd_info *mtd,struct erase_info *instr)
{
	struct opcodes *ptr_opcode;
	__u32 temp, reg;
	int finished = 0;
	unsigned int addr = instr->addr;
#ifdef CONFIG_MTD_SPIFLASH_PP
        sysGpioCtrlOutput(CONFIG_MTD_SPIFLASH_PP_GPIO); /* set GPIO_0 as output */

        sysGpioSet(CONFIG_MTD_SPIFLASH_PP_GPIO, 1);     /* drive low GPIO_0     */
#endif
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
#define FALSE 	0
#define TRUE 	1



#ifndef cpu2le32
#define cpu2le32(x)                  \
    ((((x) & 0x000000ffUL) << 24) |  \
     (((x) & 0x0000ff00UL) << 8)  |  \
     (((x) & 0x00ff0000UL) >> 8)  |  \
     (((x) & 0xff000000UL) >> 24))
#endif





static int 
spiflash_write_page (struct mtd_info *mtd,loff_t to,size_t len,size_t *retlen,const u_char *buf)
{
	int finished;
	__u32 xact_len, spi_data = 0;
	__u32 opcode, sf_opcode, reg;
	__u32 flashAddr;
       	__u32 spi_data_swapped;
	__u32  pagesToWrite, lastByteInPage, i, j;
	__u32  beginPage, beginOffset, beginRemain, endPage, endOffset;
	__u32 toggle = 0;
	__u32 totalCount = 0;
	
	
	
   	*retlen = 0;
#ifdef SPIFLASH_DEBUG
   	printk (KERN_DEBUG "%s(to = 0x%.8x, len = %d)\n",__FUNCTION__,(__u32) to,len); 
#endif	
   	/* sanity checks */
   	if (!len) return (0);
   	if (to + len > mtd->size) return (-EINVAL);
   	flashAddr = (__u32)to;
        sf_opcode = stm_opcodes[SPI_PAGE_PROGRAM].code;
	lastByteInPage= STM_PAGE_SIZE -1;
	beginPage= ((unsigned int)flashAddr) / STM_PAGE_SIZE;
	beginOffset= ((unsigned int)flashAddr) % STM_PAGE_SIZE; 
	beginRemain= STM_PAGE_SIZE - beginOffset;

	endPage= ((unsigned int)flashAddr + len - 1) / STM_PAGE_SIZE;
	endOffset= ((unsigned int)flashAddr + len - 1) % STM_PAGE_SIZE;

	pagesToWrite= endPage - beginPage + 1;
	if( (beginPage != endPage) && (endOffset != lastByteInPage) )
	{
            /* The end offset write to middle of a page */
            pagesToWrite --;
	}

	if( beginOffset | (len < STM_PAGE_SIZE) ) 
	{
               /* write from the middle of a page */
                xact_len = MIN(len, beginRemain);
                spiflash_write(mtd, (loff_t)flashAddr, xact_len, retlen, buf);
//                printk("beginOffset flashAddr = 0x%08x len =%d  bytes beginRemain  =%d bytes retlen = %d bytes\n", flashAddr, len, beginRemain, *retlen);
		flashAddr += xact_len;
		buf += xact_len;
		pagesToWrite --;
	}

        /*********** 256-byte Page program *************/        
        if( pagesToWrite )
        {
//            printk("pagesToWrite\n");
            for(i=0; i<pagesToWrite; i++)   /* write page(256 bytes) by page */
	    {               
               (void) spiflash_sendcmd(SPI_WRITE_ENABLE,0); 
		busy_wait((reg = spiflash_regread32(SPI_FLASH_CTL)) & SPI_CTL_BUSY, 0);

                spi_data = (__u32) *((__u32 *) buf);
                spi_data = cpu2le32(spi_data);
		if (!spiflash_wait_ready(FL_WRITING))
		    return -EINTR;

	        local_irq_disable();
                sysGpioCtrlOutput(CONFIG_MTD_SPIFLASH_PP_GPIO); /* set GPIO_0 as output */

                sysGpioSet(CONFIG_MTD_SPIFLASH_PP_GPIO, 0);     /* drive low GPIO_0     */

                spiflash_regwrite32(SPI_FLASH_DATA, spi_data);
                opcode = (sf_opcode & SPI_OPCODE_MASK) | ((__u32) flashAddr << 8);
                spiflash_regwrite32( SPI_FLASH_OPCODE, opcode );
                reg = (reg & ~SPI_CTL_TX_RX_CNT_MASK) | 0x8 | SPI_CTL_START;
                spiflash_regwrite32(AR531XPLUS_SPI_CTL, reg);
                buf += 4;

                for(j=0; j<31; j++)   /* 32 loops, each loop writes 4 bytes */
                {
            //      Set_Led( DIAGNOSTIC_LED, toggle);
		    if((j%3) == 0)	
                       toggle ^= 1;
                    do {
                            reg = spiflash_regread32(SPI_FLASH_CTL);
                    } while (reg & SPI_CTL_BUSY);

                    spi_data = (__u32) *((__u32 *) buf);
                    spi_data = cpu2le32(spi_data);
                    spi_data_swapped = (((spi_data >> 8) & 0xff) << 24) | (((spi_data >> 24) & 0xff) << 8) | (spi_data & 0x00ff00ff);

                    spiflash_regwrite32(SPI_FLASH_OPCODE, spi_data_swapped );

                    buf += 4;

                    spi_data = (__u32) *((__u32 *) buf);
                    spi_data = cpu2le32(spi_data);
                    spiflash_regwrite32(SPI_FLASH_DATA, spi_data );
                    reg = (reg & ~SPI_CTL_TX_RX_CNT_MASK) | 0x8 | SPI_CTL_START;
                    spiflash_regwrite32(SPI_FLASH_CTL, reg);
                    
                    buf += 4;

                }
		busy_wait((reg = spiflash_regread32(SPI_FLASH_CTL)) & SPI_CTL_BUSY, 0);

                spi_data = (__u32) *((__u32 *) buf);
                spi_data = cpu2le32(spi_data);
                spi_data_swapped = (((spi_data >> 8) & 0xff) << 24) | (((spi_data >> 24) & 0xff) << 8) | (spi_data & 0x00ff00ff);
                spiflash_regwrite32( SPI_FLASH_OPCODE, spi_data_swapped );
                reg = (reg & ~SPI_CTL_TX_RX_CNT_MASK) | 0x4 | SPI_CTL_START;
                spiflash_regwrite32(SPI_FLASH_CTL, reg);

                buf += 4;
		busy_wait((reg = spiflash_regread32(SPI_FLASH_CTL)) & SPI_CTL_BUSY, 0);

                sysGpioSet(CONFIG_MTD_SPIFLASH_PP_GPIO, 1);   /* drive high GPIO_0 */
	        local_irq_enable();
		spiflash_done();

                finished = FALSE;
                
                flashAddr += STM_PAGE_SIZE;
                *retlen += STM_PAGE_SIZE;
		do {
			udelay(10);
			reg = spiflash_sendcmd(SPI_RD_STATUS,0);
			if (!(reg & SPI_STATUS_WIP)) {
				finished = TRUE;
			}
		} while (!finished);   
	               
	    }
        }
        totalCount = *retlen;

        /************* 256-byte Page program ***************/      
        
	if( (beginPage != endPage) && (endOffset != lastByteInPage) )/* write to middle of a page */
	{
//            printk("endOffset flashAddr = 0x%08x len =%d bytes cur = 0x%08x\n", flashAddr, endOffset + 1,totalCount);		
            spiflash_write(mtd, (loff_t)flashAddr, endOffset + 1, retlen, buf); 
            totalCount += *retlen; 
            *retlen  = 0;          
	}
	*retlen = totalCount;
    return (0);
	
}


#endif

#ifdef CONFIG_MTD
static const char *part_probe_types[] = { "cmdlinepart", "RedBoot", NULL };
#endif
struct fis_image_desc {
    unsigned char name[16];      // Null terminated name
    unsigned long flash_base;    // Address within FLASH of image
    unsigned long mem_base;      // Address in memory where it executes
    unsigned long size;          // Length of image
    unsigned long entry_point;   // Execution entry point
    unsigned long data_length;   // Length of actual data
    unsigned char _pad[256-(16+7*sizeof(unsigned long))];
    unsigned long desc_cksum;    // Checksum over image descriptor
    unsigned long file_cksum;    // Checksum over image data
};

#define SCANCOUNT 5
static unsigned int redboot_offset(unsigned int highoffset,unsigned int erasesize)
{
unsigned int copy=highoffset;
int c=SCANCOUNT;
while((c--)>0)
{
highoffset-=erasesize;
unsigned char *p=(unsigned char *)highoffset;
if (!strncmp(p,"RedBoot",7))
    {
    return highoffset;
    }
}
return copy-erasesize;
}

static int spiflash_probe(struct platform_device *pdev)
{
	int offset=0;
	int ret,retlen;
   	int result = -1;
   	int index, num_parts;
	struct mtd_info *mtd;
	char *buf;
	struct fis_image_desc *fis;  
	unsigned char *p;
	struct squashfs_super_block *sb;
	size_t rootsize;
	size_t len;
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
	mtd->_write = spiflash_write_page;
#else
	mtd->_write = spiflash_write;
#endif
	mtd->owner = THIS_MODULE;
	printk(KERN_INFO "scanning for root partition\n");
	
	offset = 0;
	buf = 0xa8000000;
	int compex=0;
		if (!strncmp((char*)(buf+0x26da),"myloram.bin",11))
		    {
		    printk(KERN_INFO "Compex device detected\n");
		    dir_parts[0].size=0x20000;
		    dir_parts[0].offset=0;
		    dir_parts[7].size=mtd->size;
		    dir_parts[7].offset = 0;
		    dir_parts[6].size=mtd->erasesize;
		    dir_parts[6].offset = mtd->size-mtd->erasesize;
		    compex=1;
		    }


	while((offset+mtd->erasesize)<mtd->size)
	    {
//	    printk(KERN_INFO "[0x%08X]\n",offset);
	    if (*((__u32 *) buf) == SQUASHFS_MAGIC_SWAP) 
		{
		printk(KERN_INFO "\nfound squashfs at %X\n",offset);
		sb = (struct squashfs_super_block *) buf;
		dir_parts[2].offset = offset;
		
		dir_parts[2].size = le64_to_cpu(sb->bytes_used);
		len = dir_parts[2].offset + dir_parts[2].size;
		len +=  (mtd->erasesize - 1);
		len &= ~(mtd->erasesize - 1);
		dir_parts[2].size = (len&0xffffff) - dir_parts[2].offset;
		dir_parts[3].offset = dir_parts[2].offset + dir_parts[2].size; 
		
		dir_parts[6].offset = mtd->size-mtd->erasesize; // board config
		dir_parts[6].size = mtd->erasesize;
		dir_parts[5].offset = redboot_offset(0xa8000000+mtd->size,mtd->erasesize)-0xa8000000; //dir_parts[6].offset-mtd->erasesize; //fis config
		dir_parts[5].size = mtd->erasesize;
		dir_parts[4].offset = dir_parts[5].offset-mtd->erasesize; //nvram
		dir_parts[4].size = mtd->erasesize;
		dir_parts[3].size = dir_parts[4].offset - dir_parts[3].offset;
		rootsize = dir_parts[4].offset-offset; //size of rootfs aligned to nvram offset
		//now scan for linux offset
		if (compex)
		{
		dir_parts[1].offset=0x20000;
		dir_parts[1].size=(dir_parts[2].offset-dir_parts[1].offset)+rootsize;		
		break;
		}else{
		p=(unsigned char*)(0xa8000000+dir_parts[5].offset);
		fis = (struct fis_image_desc*)p;
		while(1)
		{
		if (fis->name[0]==0xff)
		    {
		    goto def;
		    }
		if (!strcmp(fis->name,"RedBoot"))
		    {
		    printk(KERN_INFO "found RedBoot partition at [0x%08lX]\n",fis->flash_base);
		    dir_parts[0].size=fis->size;
		    dir_parts[7].offset=0;
		    dir_parts[1].offset=fis->size;
		    }
		if (!strcmp(fis->name,"linux") || !strncmp(fis->name,"vmlinux",7) || !strcmp(fis->name,"kernel"))
		    {
		    printk(KERN_INFO "found linux partition at [0x%08lX]\n",fis->flash_base);
		    if (dir_parts[2].offset==dir_parts[1].offset) // image begins with rootfs instead of kernel
			dir_parts[1].size=fis->size+rootsize;		    
		    else
			dir_parts[1].size=(dir_parts[2].offset-dir_parts[1].offset)+rootsize;
		    dir_parts[7].size=mtd->size; // linux + nvram = phy size
		    }
		p+=sizeof(struct fis_image_desc);
		fis = (struct fis_image_desc*)p;
		}
		break;
		}
		}
	    offset+=mtd->erasesize;
	    buf+=mtd->erasesize;
	    }
	def:;
	result = add_mtd_partitions(mtd, dir_parts, 8);
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
}

module_init (spiflash_init);
module_exit (spiflash_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("OpenWrt.org, Atheros Communications Inc");
MODULE_DESCRIPTION("MTD driver for SPI Flash on Atheros SOC");

