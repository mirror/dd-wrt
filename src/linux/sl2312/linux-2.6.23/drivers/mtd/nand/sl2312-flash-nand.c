/*
 *  drivers/mtd/sl2312.c
 *
 * $Id: sl2312-flash-nand.c,v 1.5 2006/06/15 07:02:29 middle Exp $
 *
 * Copyright (C) 2001 Toshiba Corporation
 *
 * 2003 (c) MontaVista Software, Inc. This file is licensed under
 * the terms of the GNU General Public License version 2. This program
 * is licensed "as is" without any warranty of any kind, whether express
 * or implied.
 *
 */

#include <linux/slab.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <asm/hardware.h>
#include <asm/arch/sl2312.h>
#include "sl2312-flash-nand.h"


#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/mtd/compatmac.h>
#include <linux/interrupt.h>
#include <linux/bitops.h>


/*
 * NAND low-level MTD interface functions
 */
static void sl2312_nand_write_buf(struct mtd_info *mtd, const u_char *buf, int len);
static void sl2312_nand_read_buf(struct mtd_info *mtd, u_char *buf, int len);
static int sl2312_nand_verify_buf(struct mtd_info *mtd, const u_char *buf, int len);

static int sl2312_nand_read (struct mtd_info *mtd, loff_t from, size_t len, size_t * retlen, u_char * buf);
static int sl2312_nand_read_ecc (struct mtd_info *mtd, loff_t from, size_t len, size_t * retlen, u_char * buf, u_char * eccbuf, struct nand_oobinfo *oobsel);
static int sl2312_nand_read_oob (struct mtd_info *mtd, loff_t from, size_t len, size_t * retlen, u_char * buf);
static int sl2312_nand_write (struct mtd_info *mtd, loff_t to, size_t len, size_t * retlen, const u_char * buf);
static int sl2312_nand_write_ecc (struct mtd_info *mtd, loff_t to, size_t len,
			   size_t * retlen, const u_char * buf, u_char * eccbuf, struct nand_oobinfo *oobsel);
static int sl2312_nand_write_oob (struct mtd_info *mtd, loff_t to, size_t len, size_t * retlen, const u_char *buf);
static int sl2312_nand_writev (struct mtd_info *mtd, const struct kvec *vecs,
			unsigned long count, loff_t to, size_t * retlen);
static int sl2312_nand_writev_ecc (struct mtd_info *mtd, const struct kvec *vecs,
			unsigned long count, loff_t to, size_t * retlen, u_char *eccbuf, struct nand_oobinfo *oobsel);
static int sl2312_nand_erase (struct mtd_info *mtd, struct erase_info *instr, int allowbbt);
static void sl2312_nand_sync (struct mtd_info *mtd);
static int sl2312_nand_write_page (struct mtd_info *mtd, struct nand_chip *this, int page, u_char *oob_buf,  struct nand_oobinfo *oobsel);
static int sl2312_nand_block_checkbad (struct mtd_info *mtd, loff_t ofs, int getchip, int allowbbt);
static int sl2312_nand_erase_block(struct mtd_info *mtd, int page);

/*
 * MTD structure for sl2312 NDFMC
 */
static struct mtd_info *sl2312_mtd = NULL;
static int nand_page=0,nand_col=0;

/* Define default oob placement schemes for large and small page devices */
static struct nand_oobinfo nand_oob_8 = {
	.useecc = MTD_NANDECC_AUTOPLACE,
	.eccbytes = 3,
	.eccpos = {0, 1, 2},
	.oobfree = { {3, 2}, {6, 2} }
};

static struct nand_oobinfo nand_oob_16 = {
	.useecc = MTD_NANDECC_AUTOPLACE,
	.eccbytes = 6,
	.eccpos = {0, 1, 2, 3, 6, 7},
	.oobfree = { {8, 8} }
};

static struct nand_oobinfo nand_oob_64 = {
	.useecc = MTD_NANDECC_AUTOPLACE,
	.eccbytes = 24,
	.eccpos = {
		40, 41, 42, 43, 44, 45, 46, 47,
		48, 49, 50, 51, 52, 53, 54, 55,
		56, 57, 58, 59, 60, 61, 62, 63},
	.oobfree = { {2, 38} }
};


/*
 * Define partitions for flash device
 */
/* the base address of FLASH control register */
#define FLASH_CONTROL_BASE_ADDR	    (IO_ADDRESS(SL2312_FLASH_CTRL_BASE))
#define SL2312_GLOBAL_BASE_ADDR     (IO_ADDRESS(SL2312_GLOBAL_BASE))
//#define SL2312_FLASH_BASE_ADDR      (IO_ADDRESS(SL2312_FLASH_BASE))
#define SL2312_FLASH_BASE_ADDR       FLASH_VADDR(SL2312_FLASH_BASE)
static unsigned int CHIP_EN;
/* define read/write register utility */
//#define FLASH_READ_REG(offset)			(__raw_readl(offset+FLASH_CONTROL_BASE_ADDR))
//#define FLASH_WRITE_REG(offset,val) 	(__raw_writel(val,offset+FLASH_CONTROL_BASE_ADDR))
//#define FLASH_READ_DATA(offset)			(__raw_readb(offset+SL2312_FLASH_BASE_ADDR))
//#define FLASH_WRITE_DATA(offset,val) 	(__raw_writeb(val,offset+SL2312_FLASH_BASE_ADDR))

unsigned int FLASH_READ_REG(unsigned int addr)
{
    unsigned int *base;
    unsigned int data;

    base = (unsigned int *)(FLASH_CONTROL_BASE_ADDR + addr);
    data = *base;
    return (data);
}

void FLASH_WRITE_REG(unsigned int addr,unsigned int data)
{
    unsigned int *base;

    base = (unsigned int *)(FLASH_CONTROL_BASE_ADDR + addr);
    *base = data;
    return;
}

unsigned int FLASH_READ_DATA(unsigned int addr)
{
    unsigned char *base;
    unsigned int data;

    base = (unsigned char *)(SL2312_FLASH_BASE_ADDR + addr);
    data = *base;
    return (data);
}

void FLASH_WRITE_DATA(unsigned int addr,unsigned int data)
{
    unsigned char *base;

    base = (unsigned char *)(SL2312_FLASH_BASE_ADDR + addr);
    *base = data;
    return;
}

/* the offset of FLASH control register */
enum NFLASH_REGISTER {
	NFLASH_ID     			= 0x0000,
	NFLASH_STATUS 			= 0x0008,
	NFLASH_TYPE   			= 0x000c,
	NFLASH_ACCESS			= 0x0030,
	NFLASH_COUNT			= 0x0034,
	NFLASH_CMD_ADDR 		= 0x0038,
	NFLASH_ADDRESS			= 0x003C,
	NFLASH_DATA				= 0x0040,
	NFLASH_TIMING   		= 0x004C,
	NFLASH_ECC_STATUS		= 0x0050,
	NFLASH_ECC_CONTROL		= 0x0054,
	NFLASH_ECC_OOB			= 0x005c,
	NFLASH_ECC_CODE_GEN0	= 0x0060,
	NFLASH_ECC_CODE_GEN1	= 0x0064,
	NFLASH_ECC_CODE_GEN2	= 0x0068,
	NFLASH_ECC_CODE_GEN3	= 0x006C,
	NFLASH_FIFO_CONTROL		= 0x0070,
	NFLASH_FIFO_STATUS		= 0x0074,
	NFLASH_FIFO_ADDRESS		= 0x0078,
	NFLASH_FIFO_DATA		= 0x007c,
};



//#define FLASH_BASE	FLASH_CONTROL_BASE_ADDR
//#define FLASH_SIZE	0x00800000 //INTEGRATOR_FLASH_SIZE

//#define FLASH_PART_SIZE 8388608

//static unsigned int flash_indirect_access = 0;


#ifdef CONFIG_SL2312_SHARE_PIN
void sl2312flash_enable_nand_flash(void)
{
    unsigned int    reg_val;

    reg_val = readl(SL2312_GLOBAL_BASE_ADDR + 0x30);
    reg_val = reg_val & 0xfffffffb;
    writel(reg_val,SL2312_GLOBAL_BASE_ADDR + 0x30);
    return;
}

void sl2312flash_disable_nand_flash(void)
{
    unsigned int    reg_val;

    reg_val = readl(SL2312_GLOBAL_BASE_ADDR + 0x30);
    reg_val = reg_val | 0x00000004;
    writel(reg_val,SL2312_GLOBAL_BASE_ADDR + 0x30);
    return;
}
#endif

extern struct nand_oobinfo jffs2_oobinfo;
/*
 * Define partitions for flash devices
 */

static struct mtd_partition sl2312_partitions[] = {
	{ name: "RedBoot", offset: 0x00000000, size: 0x0020000, },
	{ name: "Kernel", offset: 0x00020000, size: 0x00200000, },
	{ name: "Ramdisk", offset: 0x00220000, size: 0x00280000, },
	{ name: "Application", offset: 0x004A0000, size: 0x00320000, },
	{ name: "VCTL", offset: 0x007C0000, size: 0x20000, },
	{ name: "CurConf", offset: 0x007E0000, size: 0x20000, },
	{ name: "FIS directory", offset: 0x007e0000, size: 0x00020000, }

};


/*
 *	hardware specific access to control-lines
*/
static void sl2312_hwcontrol(struct mtd_info *mtd, int cmd)
{

	return ;
}

static int sl2312_nand_scan_bbt(struct mtd_info *mtd)
{
	return 0;
}

/**
 * nand_block_isbad - [MTD Interface] Check whether the block at the given offset is bad
 * @mtd:	MTD device structure
 * @ofs:	offset relative to mtd start
 */
static int sl2312_nand_block_isbad (struct mtd_info *mtd, loff_t ofs)
{
	/* Check for invalid offset */
	if (ofs > mtd->size)
		return -EINVAL;

	return sl2312_nand_block_checkbad (mtd, ofs, 1, 0);
}

/**
 * nand_block_checkbad - [GENERIC] Check if a block is marked bad
 * @mtd:	MTD device structure
 * @ofs:	offset from device start
 * @getchip:	0, if the chip is already selected
 * @allowbbt:	1, if its allowed to access the bbt area
 *
 * Check, if the block is bad. Either by reading the bad block table or
 * calling of the scan function.
 */

static int sl2312_nand_erase_block(struct mtd_info *mtd, int page)
{
	int opcode;
	/* Send commands to erase a page */
		FLASH_WRITE_REG(NFLASH_ECC_CONTROL, 0x00000000); //set 31b = 0

		if(mtd->oobblock > 528)
		    FLASH_WRITE_REG(NFLASH_COUNT, 0x7f0fff21);  // 3 address & 2 command
		else
		    FLASH_WRITE_REG(NFLASH_COUNT, 0x7f0fff11);  // 2 address & 2 command

		FLASH_WRITE_REG(NFLASH_CMD_ADDR, 0x0000d060); // write read id command
		FLASH_WRITE_REG(NFLASH_ADDRESS, page); //write address 0x00



		/* read maker code */
		opcode = 0x80003000|DWIDTH|CHIP_EN; //set start bit & 8bits write command
		FLASH_WRITE_REG(NFLASH_ACCESS, opcode);

		while(opcode&0x80000000) //polling flash access 31b
      	{
           opcode=FLASH_READ_REG(NFLASH_ACCESS);
           //sl2312_flash_delay();
           schedule();
           //cond_resched();
      	}
}

void sl2312_flash_delay(void)
{
      int i;

      for(i=0; i<50; i++)
           i=i;
}

static int sl2312_nand_block_checkbad (struct mtd_info *mtd, loff_t ofs, int getchip, int allowbbt)
{
	struct nand_chip *this = mtd->priv;

	if (!this->bbt)
		return this->block_bad(mtd, ofs, getchip);

	/* Return info from the table */
	return nand_isbad_bbt (mtd, ofs, allowbbt);
}

/**
 * nand_block_markbad - [MTD Interface] Mark the block at the given offset as bad
 * @mtd:	MTD device structure
 * @ofs:	offset relative to mtd start
 */
static int sl2312_nand_block_markbad (struct mtd_info *mtd, loff_t ofs)
{
	struct nand_chip *this = mtd->priv;
	int ret;

        if ((ret = sl2312_nand_block_isbad(mtd, ofs))) {
        	/* If it was bad already, return success and do nothing. */
		if (ret > 0)
			return 0;
        	return ret;
        }

	return this->block_markbad(mtd, ofs);
}

/*
 *	Get chip for selected access
 */
static inline void sl2312_nand_get_chip (struct nand_chip *this, struct mtd_info *mtd, int new_state, int *erase_state)
{

	DECLARE_WAITQUEUE (wait, current);

	/*
	 * Grab the lock and see if the device is available
	 * For erasing, we keep the spinlock until the
	 * erase command is written.
	*/
retry:
	spin_lock_bh (&this->chip_lock);

	if (this->state == FL_READY) {
		this->state = new_state;
		if (new_state != FL_ERASING)
			spin_unlock_bh (&this->chip_lock);
		return;
	}

	if (this->state == FL_ERASING) {
		if (new_state != FL_ERASING) {
			this->state = new_state;
			spin_unlock_bh (&this->chip_lock);
			this->select_chip(mtd, 0);	/* select in any case */
			this->cmdfunc(mtd, NAND_CMD_RESET, -1, -1);
			return;
		}
	}

	set_current_state (TASK_UNINTERRUPTIBLE);
	add_wait_queue (&this->wq, &wait);
	spin_unlock_bh (&this->chip_lock);
	schedule ();
	remove_wait_queue (&this->wq, &wait);
	goto retry;
}

/*
*	read device ready pin
*/
static int sl2312_device_ready(struct mtd_info *mtd)
{
	int ready;

	FLASH_WRITE_REG(NFLASH_ECC_CONTROL, 0x00000000); //set 31b = 0
	FLASH_WRITE_REG(NFLASH_COUNT, 0x7f000070); //set only command no address and two data

	FLASH_WRITE_REG(NFLASH_CMD_ADDR, 0x00000070); //write read status command


	ready = 0x80002000|DWIDTH|CHIP_EN; //set start bit & 8bits read command
	FLASH_WRITE_REG(NFLASH_ACCESS, ready);

	while(ready&0x80000000) //polling flash access 31b
    {
        ready=FLASH_READ_REG(NFLASH_ACCESS);
        //sl2312_flash_delay();
		schedule();
    }
    FLASH_WRITE_REG(NFLASH_ACCESS, NFLASH_DIRECT);
      	ready=FLASH_READ_REG(NFLASH_DATA)&0xff;
	return ready;
}
void sl2312_enable_hwecc(struct mtd_info *mtd, int mode)
{
	/* reset first */
	FLASH_WRITE_REG(NFLASH_ECC_CONTROL, 0x80000001); //set 31b = 0

}


void sl2312_device_setup(void)
{

}
static u_char sl2312_nand_read_byte(struct mtd_info *mtd)
{

        unsigned int    data=0, page=0, col=0, tmp, i;

        printk ("**************************sl2312_nand_read_byte !! \n");
        //page = FLASH_READ_REG(NFLASH_ADDRESS)&0xffffff00;
        //col  = FLASH_READ_REG(NFLASH_ADDRESS)&0x000000ff;
        page = nand_page;
        col  = nand_col;
        for(i=0;i<(mtd->oobblock+mtd->oobsize);i++)
        {
        	if(i==col)
				data = FLASH_READ_DATA(page*mtd->oobblock +i);
			else
				tmp = FLASH_READ_DATA(page*mtd->oobblock +i);
        }
        return data&0xff;
}

static void sl2312_nand_write_byte(struct mtd_info *mtd, u_char byte)
{
        //struct nand_chip *this = mtd->priv;
        unsigned int    page=0, col=0, i;
        u_char *databuf,oobbuf[mtd->oobsize];
        size_t  retlen;
        retlen=0;
		printk ("********************sl2312_nand_write_byte !! \n");
		page = nand_page;
        col  = nand_col;
		databuf = kmalloc (mtd->oobsize+mtd->oobblock,GFP_KERNEL);

		if (!databuf) {
			printk ("sl2312_nand_write_byte : Unable to allocate SL2312 NAND MTD device structure.\n");

		}

		 for(i=0;i<(mtd->oobblock+mtd->oobsize);i++)
           	databuf[i] = FLASH_READ_DATA(page*mtd->oobblock +i);

        databuf[col] = byte;
        sl2312_nand_write_ecc (mtd, page, mtd->oobblock, &retlen, databuf, oobbuf, NULL);

}

static void sl2312_nand_write_buf(struct mtd_info *mtd, const u_char *buf, int len)
{
	int i, page=0,col=0;
	struct nand_chip *this = mtd->priv;
	u_char *databuf, *oobbuf;
        size_t  retlen;
        retlen=0;


		printk ("***********************sl2312_nand_write_buf !! \n");
		databuf = &(this->data_buf[0]);
		oobbuf = &(this->data_buf[mtd->oobblock]);
		for (i = 0; i < mtd->oobsize; i++)
			oobbuf[i] = 0xff;

	if(len < mtd->oobblock)
	{
		//addr = FLASH_READ_REG(NFLASH_ADDRESS);
		//page = FLASH_READ_REG(NFLASH_ADDRESS)&0xffffff00;
		//col  = FLASH_READ_REG(NFLASH_ADDRESS)&0x000000ff;
		page = nand_page;
        col  = nand_col;

		sl2312_nand_read_ecc (mtd, page, mtd->oobblock , &retlen, databuf, oobbuf, NULL);

        for(i=col;i<len;i++)
        	databuf[col+i] = buf[i];

        sl2312_nand_write_ecc (mtd, page, mtd->oobblock, &retlen, databuf, oobbuf, NULL);

	}

}

static void sl2312_nand_read_buf(struct mtd_info *mtd, u_char *buf, int len)
{
	int i, page=0,col=0,addr=0,tmp=0;
	//struct nand_chip *this = mtd->priv;
	printk ("********************sl2312_nand_read_buf !! \n");
	if(len < mtd->oobblock)
	{
		//addr = FLASH_READ_REG(NFLASH_ADDRESS);
		//page = FLASH_READ_REG(NFLASH_ADDRESS)&0xffffff00;
		//col  = FLASH_READ_REG(NFLASH_ADDRESS)&0x000000ff;
		page = nand_page;
        col  = nand_col;
		for (i=col; i<((mtd->oobblock+mtd->oobsize)-col); i++)
		{
			if(i<len)
				buf[i] = FLASH_READ_DATA(addr+i);
			else
				tmp = FLASH_READ_DATA(addr+i);
		}
	}
}

static int sl2312_nand_verify_buf(struct mtd_info *mtd, const u_char *buf, int len)
{
	int i;
	//struct nand_chip *this = mtd->priv;
	u_char *datatmp, *oobtmp;
	size_t  retlen;
	retlen=0;

	datatmp = kmalloc (mtd->oobblock,GFP_KERNEL);
	oobtmp = kmalloc (mtd->oobsize,GFP_KERNEL);

	if ((!datatmp)||(!oobtmp)) {
		printk ("sl2312_nand_verify_buf : Unable to allocate SL2312 NAND MTD device structure.\n");

	}
	//page = nand_page;
	for(i=0;i<mtd->oobblock;i++)
		datatmp[i] = FLASH_READ_DATA(nand_page*mtd->oobblock +i);
	/* read oobdata */
	for (i = 0; i <  mtd->oobsize; i++)
		oobtmp[i] = FLASH_READ_DATA(nand_page*mtd->oobblock + mtd->oobblock + i);

	if(len==mtd->oobblock)
	{
		for (i=0; i<len; i++)
		{
			if (buf[i] != datatmp[i])
			{
				kfree(datatmp);
				kfree(oobtmp);
				printk("Data verify error -> page: %x, byte: %x \n",nand_page,i);
				return i;
			}
		}
	}
	else if(len == mtd->oobsize)
	{
		for (i=0; i<len; i++)
		{
			if (buf[i] != oobtmp[i])
			{
				kfree(datatmp);
				kfree(oobtmp);
				printk("OOB verify error -> page: %x, byte: %x \n",nand_page,i);
				return i;
			}
		}
	}
	else
	{
		printk (KERN_WARNING "sl2312_nand_verify_buf : verify length not match 0x%08x\n", len);
		kfree(datatmp);
		kfree(oobtmp);
		return -1;
	}

	kfree(datatmp);
	kfree(oobtmp);
	return 0;
}

/*
 * Send command to NAND device
 */
static void sl2312_nand_command (struct mtd_info *mtd, unsigned command, int column, int page_addr)
{
	register struct nand_chip *this = mtd->priv;
	int opcode;


	/*
	 * program and erase have their own busy handlers
	 * status and sequential in needs no delay
	*/
	switch (command) {

	case NAND_CMD_PAGEPROG:
	case NAND_CMD_ERASE1:
	case NAND_CMD_ERASE2:
	case NAND_CMD_SEQIN:
	case NAND_CMD_STATUS:
	case NAND_CMD_READ0:

		/*
		 * Write out the command to the device.
		 */
		if (column != -1 || page_addr != -1) {

			/* Serially input address */
			if (column != -1)
				//FLASH_WRITE_REG(NFLASH_ADDRESS,column);
				nand_col=column;

			opcode = FLASH_READ_REG(NFLASH_ADDRESS);

			if (page_addr != -1)
				//FLASH_WRITE_REG(NFLASH_ADDRESS,opcode|(page_addr<<8));
				nand_page = page_addr;

		}
		return;

	case NAND_CMD_RESET:
		if (this->dev_ready)
			break;
		FLASH_WRITE_REG(NFLASH_ECC_CONTROL, 0x00000000); //set 31b = 0
		FLASH_WRITE_REG(NFLASH_COUNT, 0x7f0fff70); //set only command and no other data
		FLASH_WRITE_REG(NFLASH_CMD_ADDR, NAND_CMD_RESET); //write reset command

		opcode = 0x80002000|DWIDTH|CHIP_EN; //set start bit & 8bits read command
		FLASH_WRITE_REG(NFLASH_ACCESS, opcode);

		while(opcode&0x80000000) //polling flash access 31b
      	{
           opcode=FLASH_READ_REG(NFLASH_ACCESS);
           //sl2312_flash_delay();
           schedule();
      	}
		while ( !(sl2312_device_ready(mtd) & 0x40));
		{
			FLASH_WRITE_REG(NFLASH_ACCESS, NFLASH_DIRECT);
			//sl2312_flash_delay();
			schedule();
			return;
		}
	/* This applies to read commands */
	default:
		/*
		 * If we don't have access to the busy pin, we apply the given
		 * command delay
		*/
		if (!this->dev_ready) {
			udelay (this->chip_delay);
			FLASH_WRITE_REG(NFLASH_ACCESS, NFLASH_DIRECT);
			return;
		}
	}

	/* wait until command is processed */
	while (!this->dev_ready(mtd));

}
/*Add function*/
static void nand_read_id(int chip_no, unsigned char *id)
{
	unsigned int opcode, i;

	if(chip_no==0)
		CHIP_EN = NFLASH_CHIP0_EN;
	else
		CHIP_EN = NFLASH_CHIP1_EN;

	opcode = FLASH_READ_REG(NFLASH_TYPE);

	FLASH_WRITE_REG(NFLASH_ECC_CONTROL, 0x00000000); //set 31b = 0
	if((opcode&0x00000300)<=0x00000100)
	    FLASH_WRITE_REG(NFLASH_COUNT, 0x7f000100); //set only command & address and two data
	else
	    FLASH_WRITE_REG(NFLASH_COUNT, 0x7f000300); //set only command & address and 4 data

	FLASH_WRITE_REG(NFLASH_CMD_ADDR, 0x00000090); //write read id command
	FLASH_WRITE_REG(NFLASH_ADDRESS, 0x00000000); //write address 0x00

	/* read maker code */
	opcode = 0x80002000|DWIDTH|CHIP_EN;//|chip0_en; //set start bit & 8bits read command
	FLASH_WRITE_REG(NFLASH_ACCESS, opcode);
	opcode=FLASH_READ_REG(NFLASH_ACCESS);
		while(opcode&0x80000000) //polling flash access 31b
      	{
           opcode=FLASH_READ_REG(NFLASH_ACCESS);
           //sl2312_flash_delay();
           schedule();
      	}

    opcode = FLASH_READ_REG(NFLASH_DATA);
    if(DWIDTH==NFLASH_WiDTH16)
    {
      		id[0] = opcode&0xff;
      		id[1] = (opcode&0xff00)>>8;
    }
    else
    {
    	    id[0] = opcode&0xff;
    	    opcode = 0x80002000|DWIDTH|CHIP_EN;//|chip0_en; //set start bit & 8bits read command
			FLASH_WRITE_REG(NFLASH_ACCESS, opcode);
			opcode=FLASH_READ_REG(NFLASH_ACCESS);
			while(opcode&0x80000000) //polling flash access 31b
    	  	{
    	       opcode=FLASH_READ_REG(NFLASH_ACCESS);
    	       //sl2312_flash_delay();
    	       schedule();
    	  	}
    		opcode = FLASH_READ_REG(NFLASH_DATA);
      		id[1] = (opcode&0xff00)>>8;

      		opcode=FLASH_READ_REG(NFLASH_TYPE);
      		if((opcode&0x300)>0x100)
      		{
      		    for(i=0;i<2;i++)
      		    {
      				//data cycle 3 & 4 ->not use
      				opcode = 0x80002000|DWIDTH|CHIP_EN;//set start bit & 8bits read command
					FLASH_WRITE_REG(NFLASH_ACCESS, opcode);
					opcode=FLASH_READ_REG(NFLASH_ACCESS);
      				while(opcode&0x80000000) //polling flash access 31b
      				{
        			   opcode=FLASH_READ_REG(NFLASH_ACCESS);
        			   //sl2312_flash_delay();
        			   schedule();
      				}

      				opcode=FLASH_READ_REG(NFLASH_DATA);
      				id[2+i] = (opcode&(0xff0000<<i*8))>>(8*(2+i));
      		    }
      		}
    }
    FLASH_WRITE_REG(NFLASH_ACCESS, NFLASH_DIRECT);
}

/*
 * NAND erase a block
 */
static int sl2312_nand_erase (struct mtd_info *mtd, struct erase_info *instr, int allowbbt)
{
	int page, len, status, pages_per_block, ret, chipnr;
	struct nand_chip *this = mtd->priv;

	DEBUG (MTD_DEBUG_LEVEL3,
	       "nand_erase: start = 0x%08x, len = %i\n", (unsigned int) instr->addr, (unsigned int) instr->len);

	/* Start address must align on block boundary */
	if (instr->addr & ((1 << this->phys_erase_shift) - 1)) {
		DEBUG (MTD_DEBUG_LEVEL0, "nand_erase: Unaligned address\n");
		return -EINVAL;
	}

	/* Length must align on block boundary */
	if (instr->len & ((1 << this->phys_erase_shift) - 1)) {
		DEBUG (MTD_DEBUG_LEVEL0, "nand_erase: Length not block aligned\n");
		return -EINVAL;
	}

	/* Do not allow erase past end of device */
	if ((instr->len + instr->addr) > mtd->size) {
		DEBUG (MTD_DEBUG_LEVEL0, "nand_erase: Erase past end of device\n");
		return -EINVAL;
	}

	instr->fail_addr = 0xffffffff;

	/* Grab the lock and see if the device is available */
	sl2312_nand_get_chip (this, mtd, FL_ERASING, NULL);

	/* Shift to get first page */
	page = (int) (instr->addr >> this->page_shift);
	chipnr = (int) (instr->addr >> this->chip_shift);

	/* Calculate pages in each block */
	pages_per_block = 1 << (this->phys_erase_shift - this->page_shift);

	/* Select the NAND device */
	//this->select_chip(mtd, chipnr);
	this->select_chip(mtd, 0);

	/* Check the WP bit */
	/* Check, if it is write protected */
	status = sl2312_device_ready(mtd);
	if (!(status & 0x80)) {
		DEBUG (MTD_DEBUG_LEVEL0, "nand_erase: Device is write protected!!!\n");
		instr->state = MTD_ERASE_FAILED;
		goto erase_exit;
	}

	/* Loop through the pages */
	len = instr->len;

	instr->state = MTD_ERASING;

	while (len) {
		/* Check if we have a bad block, we do not erase bad blocks ! */
		if (this->block_bad(mtd, ((loff_t) page) << this->page_shift, 0)) {
			printk (KERN_WARNING "nand_erase: attempt to erase a bad block at page 0x%08x\n", page);
			//instr->state = MTD_ERASE_FAILED;
			//goto erase_exit;
		}

		/* Invalidate the page cache, if we erase the block which contains
		   the current cached page */
		if (page <= this->pagebuf && this->pagebuf < (page + pages_per_block))
			this->pagebuf = -1;
		/////////

		///* Send commands to erase a page */
		//FLASH_WRITE_REG(NFLASH_ECC_CONTROL, 0x00000000); //set 31b = 0
	    //
		//if(mtd->oobblock > 528)
		//    FLASH_WRITE_REG(NFLASH_COUNT, 0x7f0fff21);  // 3 address & 2 command
		//else
		//    FLASH_WRITE_REG(NFLASH_COUNT, 0x7f0fff11);  // 2 address & 2 command
		//
		//FLASH_WRITE_REG(NFLASH_CMD_ADDR, 0x0000d060); // write read id command
		//FLASH_WRITE_REG(NFLASH_ADDRESS, page); //write address 0x00
		//
		//
		//
		///* read maker code */
		//opcode = 0x80003000|DWIDTH|CHIP_EN; //set start bit & 8bits write command
		//FLASH_WRITE_REG(NFLASH_ACCESS, opcode);
		//
		//while(opcode&0x80000000) //polling flash access 31b
      	//{
        //   opcode=FLASH_READ_REG(NFLASH_ACCESS);
        //   //sl2312_flash_delay();
        //   schedule();
        //   //cond_resched();
      	//}
      	sl2312_nand_erase_block(mtd, page);
      	//////////////
		status = this->waitfunc (mtd, this, FL_ERASING);
		/* See if block erase succeeded */
		if (status & 0x01) {
			DEBUG (MTD_DEBUG_LEVEL0, "nand_erase: " "Failed erase, page 0x%08x\n", page);
			instr->state = MTD_ERASE_FAILED;
			instr->fail_addr = (page << this->page_shift);
			goto erase_exit;
		}

		/* Increment page address and decrement length */
		len -= (1 << this->phys_erase_shift);
		page += pages_per_block;

		/* Check, if we cross a chip boundary */
		if (len && !(page & this->pagemask)) {
			chipnr++;
			this->select_chip(mtd, 0);
			this->select_chip(mtd, 0);
		}
		//sl2312_flash_delay();
           schedule();
           //cond_resched();
	}
	instr->state = MTD_ERASE_DONE;

erase_exit:
	/* De-select the NAND device */
	this->select_chip(mtd, 0);
	spin_unlock_bh (&this->chip_lock);

	ret = instr->state == MTD_ERASE_DONE ? 0 : -EIO;;
	/* Do call back function */
	if (!ret && instr->callback)
		instr->callback (instr);

	/* The device is ready */
	spin_lock_bh (&this->chip_lock);
	this->state = FL_READY;
	spin_unlock_bh (&this->chip_lock);
	FLASH_WRITE_REG(NFLASH_ACCESS, NFLASH_DIRECT);
	/* Return more or less happy */
	return ret;
}

static void sl2312_nand_select_chip(struct mtd_info *mtd, int chip)
{
	//struct nand_chip *this = mtd->priv;

	switch(chip) {
	case -1:
		CHIP_EN = NFLASH_CHIP0_EN;
		break;
	case 0:
		CHIP_EN = NFLASH_CHIP0_EN;
		break;
	case 1:
		CHIP_EN = NFLASH_CHIP1_EN;
		break;
	default:
			CHIP_EN = NFLASH_CHIP0_EN;
			break;
	}
}

/**
 * nand_default_block_markbad - [DEFAULT] mark a block bad
 * @mtd:	MTD device structure
 * @ofs:	offset from device start
 *
 * This is the default implementation, which can be overridden by
 * a hardware specific driver.
*/
static int sl2312_nand_default_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	struct nand_chip *this = mtd->priv;
	u_char buf[2] = {0, 0};
	size_t	retlen;
	int block;

	/* Get block number */
	block = ((int) ofs) >> this->bbt_erase_shift;
	this->bbt[block >> 2] |= 0x01 << ((block & 0x03) << 1);

	/* Do we have a flash based bad block table ? */
	if (this->options & NAND_USE_FLASH_BBT)
		return nand_update_bbt (mtd, ofs);

	/* We write two bytes, so we dont have to mess with 16 bit access */
	ofs += mtd->oobsize + (this->badblockpos & ~0x01);
	return sl2312_nand_write_oob (mtd, ofs , 2, &retlen, buf);
}

/* Appropriate chip should already be selected */
static int sl2312_nand_block_bad(struct mtd_info *mtd, loff_t ofs, int getchip)//(struct mtd_info *mtd, unsigned long page, )
{
	u_char *buf, *oobbuf;
	size_t  retlen;
	unsigned long page, chipnr;
	struct nand_chip *this = mtd->priv;

	if (getchip) {
		page = (int)(ofs >> this->page_shift);
		chipnr = (int)(ofs >> this->chip_shift);

		/* Grab the lock and see if the device is available */
		sl2312_nand_get_chip (this, mtd, FL_READING, NULL);
		/* Select the NAND device */
		this->select_chip(mtd, chipnr);
	} else
		page = (int) ofs;

	buf = kmalloc (mtd->oobblock,GFP_KERNEL);
	oobbuf = kmalloc (mtd->oobsize,GFP_KERNEL);

	if ((!buf)||(!oobbuf)) {
		printk ("sl2312_nand_block_bad : Unable to allocate SL2312 NAND MTD device structure.\n");

	}

	sl2312_nand_read_ecc (mtd, page, mtd->oobblock , &retlen, buf, oobbuf, NULL);


	if(((mtd->oobblock < 528)&&(oobbuf[5] != 0xff))||((mtd->oobblock > 528)&&(oobbuf[0] != 0xff)))
	{
		kfree(buf);
		kfree(oobbuf);
		return 1;
	}

	kfree(buf);
	kfree(oobbuf);
	return 0;
}

/*
*	Use NAND read ECC
*/
static int sl2312_nand_read (struct mtd_info *mtd, loff_t from, size_t len, size_t * retlen, u_char * buf)
{
	return sl2312_nand_read_ecc (mtd, from, len, retlen, buf, NULL, NULL);
}

/*
 * NAND read with ECC
 */
static int sl2312_nand_read_ecc (struct mtd_info *mtd, loff_t from, size_t len,
			  size_t * retlen, u_char * buf, u_char * oob_buf, struct nand_oobinfo *oobsel)
{
	int j, col, page, opcode, i;
	int end=0;//, ecc=0;//, end_page=0;
	int erase_state = 0;
	int read = 0, oob = 0, ecc_failed = 0;//, ecc_status = 0
	struct nand_chip *this = mtd->priv;
	u_char *data_poi, *oob_data = oob_buf;
	//u_char ecc_calc[6];
	//u_char ecc_code[6];
	int 	eccmode;
	int	*oob_config;



	// use chip default if zero
	if (oobsel == NULL)
		oobsel = &mtd->oobinfo;

	eccmode = oobsel->useecc ? this->eccmode : NAND_ECC_NONE;
	oob_config = oobsel->eccpos;

	DEBUG (MTD_DEBUG_LEVEL3, "nand_read_ecc: from = 0x%08x, len = %i\n", (unsigned int) from, (int) len);

	/* Do not allow reads past end of device */
	if ((from + len) > mtd->size) {
		DEBUG (MTD_DEBUG_LEVEL0, "nand_read_ecc: Attempt read beyond end of device\n");
		*retlen = 0;
		return -EINVAL;
	}

	/* Grab the lock and see if the device is available */
	sl2312_nand_get_chip (this, mtd ,FL_READING, &erase_state);

	/* Select the NAND device */
	this->select_chip(mtd, 0);

	/* First we calculate the starting page */
	page = from >> this->page_shift;

	//end_page = mtd->oobblock + mtd->oobsize;
	end = mtd->oobblock;
	//ecc = mtd->eccsize;
	/* Get raw starting column */
	col = (from & (mtd->oobblock - 1));


	/* Send the read command */
	//this->cmdfunc (mtd, NAND_CMD_READ0, 0x00, page);

	/* Loop until all data read */
	FLASH_WRITE_REG(NFLASH_ACCESS, NFLASH_DIRECT);
	while (read < len) {

		//udelay(1200);
		/* If we have consequent page reads, apply delay or wait for ready/busy pin */
		if (read) {
			if (!this->dev_ready)
				udelay (this->chip_delay);
			else
				while (!this->dev_ready(mtd));
		}

		/*
		 * If the read is not page aligned, we have to read into data buffer
		 * due to ecc, else we read into return buffer direct
		 */
		if (!col && (len - read) >= end)
			data_poi = &buf[read];
		else
			data_poi = this->data_buf;

		/* get oob area, if we have no oob buffer from fs-driver */
		if (!oob_buf) {
			oob_data = &this->data_buf[end];
			oob = 0;
		}

		j = 0;
		switch (eccmode) {
			case NAND_ECC_NONE: {	/* No ECC, Read in a page */
				FLASH_WRITE_REG(NFLASH_ECC_CONTROL, 0x0); //set 31b = 0
				break;
			}

			case NAND_ECC_SOFT:	/* Software ECC 3/256: Read in a page + oob data */
				break;

			case NAND_ECC_HW3_256: /* Hardware ECC 3 byte /256 byte data: Read in first 256 byte, get ecc, */
				break;

			case NAND_ECC_HW3_512:
			case NAND_ECC_HW6_512: /* Hardware ECC 3/6 byte / 512 byte data : Read in a page  */
				FLASH_WRITE_REG(NFLASH_ECC_CONTROL, 0x80000001); //set 31b = 0
				break;

			default:
				printk (KERN_WARNING "Invalid NAND_ECC_MODE %d\n", this->eccmode);
				FLASH_WRITE_REG(NFLASH_ECC_CONTROL, 0x0);
				//BUG();
		}//end switch

			for(i=0;i<end;i++)
			{
				//udelay(7);
				data_poi[i] = FLASH_READ_DATA(page*mtd->oobblock +i);
			}
			/* read oobdata */
			for (i = 0; i <  mtd->oobsize; i++)
			{
				//udelay(7);
				oob_data[oob + i] = FLASH_READ_DATA(page*mtd->oobblock +end+i);
			}

		/* Skip ECC, if not active */
			if (eccmode == NAND_ECC_NONE)
				goto readdata;

			// compare ecc and correct data

				opcode=FLASH_READ_REG(NFLASH_ECC_STATUS);
				while(!(opcode&0x80000000)) //polling flash access 31b
      			{
        			   opcode=FLASH_READ_REG(NFLASH_ECC_STATUS);
        			   //sl2312_flash_delay();
        			   schedule();
      			}
      			for(j=0;j<(end/512);j++)
      			{//for 2k page

					opcode = 0x00000000|oob_data[mtd->oobsize-3-4*j]<<16|oob_data[mtd->oobsize-2-4*j]<<8|oob_data[mtd->oobsize-1-4*j];

					//opcode=FLASH_READ_REG(NFLASH_ECC_CODE_GEN0+(j*4));

					FLASH_WRITE_REG(NFLASH_ECC_OOB, opcode);
					opcode = 0x00000000|(j<<8); //select ECC code generation 0
					FLASH_WRITE_REG(NFLASH_ECC_CONTROL, opcode); //???

					opcode=FLASH_READ_REG(NFLASH_ECC_STATUS);
					if((opcode&0x00000003)==0x03)
					{
						printk (KERN_WARNING "\nPageRead Uncorrectable error !!\n");
						ecc_failed++;
					}
					else if((opcode&0x00000003)==0x01)
					{
						printk (KERN_WARNING "\nPageRead One bit data error !!");
						// correct data
						if((data_poi[(opcode&0xff80)>>7]>>((opcode&0x38)>>3))%1)
							data_poi[(opcode&0xff80)>>7] &= ~(1<<((opcode&0x38)>>3));
						else
							data_poi[(opcode&0xff80)>>7] |= (1<<((opcode&0x38)>>3));

					}
					else if((opcode&0x00000003)==0x02)
					{
						printk (KERN_WARNING "\nPageRead One bit ECC error !!\n");
					}
					else if((opcode&0x00000003)==0x00)
					{

					}

				}//for 2k page
readdata:
		if (col || (len - read) < end) {
			for (j = col; j < end && read < len; j++)
				buf[read++] = data_poi[j];
		} else
			read += mtd->oobblock;
		/* For subsequent reads align to page boundary. */
		col = 0;
		/* Increment page address */
		page++;
		schedule();
	}
	/* De-select the NAND device */
	//this->select_chip(mtd, -1);
	FLASH_WRITE_REG(NFLASH_ECC_CONTROL, 0x0); //set 31b = 0
	FLASH_WRITE_REG(NFLASH_ACCESS, NFLASH_INDIRECT);
	/* Wake up anyone waiting on the device */
	spin_lock_bh (&this->chip_lock);
	this->state = FL_READY;
	wake_up (&this->wq);
	spin_unlock_bh (&this->chip_lock);

	/*
	 * Return success, if no ECC failures, else -EIO
	 * fs driver will take care of that, because
	 * retlen == desired len and result == -EIO
	 */
	*retlen = read;
	return ecc_failed ? -EIO : 0;
}

/*
 * Wait for command done. This applies to erase and program only
 * Erase can take up to 400ms and program up to 20ms according to
 * general NAND and SmartMedia specs
 *
*/
static int sl2312_nand_waitfunc(struct mtd_info *mtd, struct nand_chip *this, int state)
{
	unsigned long	timeo = jiffies;
	int	status, opcode;

	if (state == FL_ERASING)
		 timeo += (HZ * 400) / 1000;
	else
		 timeo += (HZ * 20) / 1000;

	spin_lock_bh (&this->chip_lock);
	FLASH_WRITE_REG(NFLASH_ECC_CONTROL, 0x00000000); //set 31b = 0
	FLASH_WRITE_REG(NFLASH_COUNT, 0x007f000070); //set only command no address and two data

	FLASH_WRITE_REG(NFLASH_CMD_ADDR, 0x00000070); //write read status command


	opcode = 0x80002000|DWIDTH|CHIP_EN; //set start bit & 8bits read command
	FLASH_WRITE_REG(NFLASH_ACCESS, opcode);

	while(opcode&0x80000000) //polling flash access 31b
    {
        opcode=FLASH_READ_REG(NFLASH_ACCESS);
        //sl2312_flash_delay();
        schedule();
    }

	while (time_before(jiffies, timeo)) {
		/* Check, if we were interrupted */
		if (this->state != state) {
			spin_unlock_bh (&this->chip_lock);
			FLASH_WRITE_REG(NFLASH_ACCESS, NFLASH_DIRECT);
			return 0;
		}
		if (this->dev_ready) {
			if (this->dev_ready(mtd))
				break;
		}
		if (FLASH_READ_REG(NFLASH_DATA) & 0x40)
			break;

		spin_unlock_bh (&this->chip_lock);
		yield ();
		spin_lock_bh (&this->chip_lock);
	}
	status = FLASH_READ_REG(NFLASH_DATA)&0xff;
	spin_unlock_bh (&this->chip_lock);
	FLASH_WRITE_REG(NFLASH_ACCESS, NFLASH_DIRECT);
	return status;
}

static int sl2312_nand_read_oob (struct mtd_info *mtd, loff_t from, size_t len, size_t * retlen, u_char * buf)
{
	int i, col, page, j=0;
	//int erase_state = 0;
	struct nand_chip *this = mtd->priv;
	u_char *databuf, *oobbuf;

	databuf = &this->data_buf[0];
	oobbuf = &this->data_buf[mtd->oobblock];
		for (i = 0; i < mtd->oobsize; i++)
			oobbuf[i] = 0xff;

	DEBUG (MTD_DEBUG_LEVEL3, "nand_read_oob: from = 0x%08x, len = %i\n", (unsigned int) from, (int) len);

	/* Shift to get page */
	page = ((int) from) >> this->page_shift;

	/* Mask to get column */
	col = from & (mtd->oobsize-1);  //0x0f;

	/* Initialize return length value */
	*retlen = 0;
	sl2312_nand_read_ecc (mtd, page, mtd->oobblock , retlen, databuf, oobbuf, NULL);
	for(i=col,j=0;i<mtd->oobsize||i<(col+len);i++,j++)
		buf[j] = oobbuf[i];

	*retlen = j ;
	return 0;
}

#define NOTALIGNED(x) (x & (mtd->oobblock-1)) != 0
/*
*	Use NAND write ECC
*/
static int sl2312_nand_write (struct mtd_info *mtd, loff_t to, size_t len, size_t * retlen, const u_char * buf)
{
	return (sl2312_nand_write_ecc (mtd, to, len, retlen, buf, NULL, NULL));
}

/*
 * NAND write with ECC
 */
static int sl2312_nand_write_ecc (struct mtd_info *mtd, loff_t to, size_t len,
			   size_t * retlen, const u_char * buf, u_char * eccbuf, struct nand_oobinfo *oobsel)
{
	int page, ret = 0, oob = 0, written = 0;
	struct nand_chip *this = mtd->priv;

	DEBUG (MTD_DEBUG_LEVEL3, "nand_write_ecc: to = 0x%08x, len = %i\n", (unsigned int) to, (int) len);


	/* Do not allow write past end of device */
	if ((to + len) > mtd->size) {
		DEBUG (MTD_DEBUG_LEVEL0, "nand_write_ecc: Attempt to write past end of page\n");
		return -EINVAL;
	}

	/* reject writes, which are not page aligned */
	if (NOTALIGNED (to) || NOTALIGNED(len)) {
		printk (KERN_NOTICE "nand_write_ecc: Attempt to write not page aligned data\n");
		return -EINVAL;
	}

	// if oobsel is NULL, use chip defaults
	if (oobsel == NULL)
		oobsel = &mtd->oobinfo;

	/* Shift to get page */
	page = ((int) to) >> this->page_shift;

	/* Grab the lock and see if the device is available */
	sl2312_nand_get_chip (this, mtd, FL_WRITING, NULL);

	/* Select the NAND device */
	this->select_chip(mtd, 0);

	/* Check the WP bit */
	if (!(sl2312_device_ready(mtd) & 0x80)) {
		DEBUG (MTD_DEBUG_LEVEL0, "nand_write_ecc: Device is write protected!!!\n");
		ret = -EIO;
		goto out;
	}

	/* Loop until all data is written */
	while (written < len) {
		//udelay(100);
		int cnt = mtd->oobblock;
		this->data_poi = (u_char*) &buf[written];
		/* We use the same function for write and writev */
		if (eccbuf) {
			ret = sl2312_nand_write_page (mtd, this, page, &eccbuf[oob], oobsel);
			oob += mtd->oobsize;
		} else
			ret = sl2312_nand_write_page (mtd, this, page, NULL, oobsel);

		if (ret)
			goto out;

		/* Update written bytes count */
		written += cnt;
		/* Increment page address */
		page++;
	}

out:
	/* De-select the NAND device */
	//this->select_chip(mtd, -1);

	/* Wake up anyone waiting on the device */
	spin_lock_bh (&this->chip_lock);
	this->state = FL_READY;
	wake_up (&this->wq);
	spin_unlock_bh (&this->chip_lock);

	*retlen = written;
	return ret;
}

/*
 *	Nand_page_program function is used for write and writev !
 *	This function will always program a full page of data
 *	If you call it with a non page aligned buffer, you're lost :)
 */
static int sl2312_nand_write_page (struct mtd_info *mtd, struct nand_chip *this, int page, u_char *oob_buf,  struct nand_oobinfo *oobsel)
{
	int 	i, j, status, opcode;
	u_char	ecc_code[16], *oob_data;
	int	eccmode = oobsel->useecc ? this->eccmode : NAND_ECC_NONE;
	//int  	*oob_config = oobsel->eccpos;

	/* pad oob area, if we have no oob buffer from fs-driver */
	if (!oob_buf) {
		oob_data = &this->data_buf[mtd->oobblock];
		for (i = 0; i < mtd->oobsize; i++)
			oob_data[i] = 0xff;
	} else
		oob_data = oob_buf;

	/* Send command to begin auto page programming */

	memset(oob_data,0xff,mtd->oobsize);
	/* Write out complete page of data, take care of eccmode */
	switch (eccmode) {
	/* No ecc and software ecc 3/256, write all */
	case NAND_ECC_NONE:
		printk (KERN_WARNING "Writing data without ECC to NAND-FLASH is not recommended\n");
		FLASH_WRITE_REG(NFLASH_ECC_CONTROL, 0x0); //set 31b = 0
		break;
	case NAND_ECC_SOFT:
		break;

	/* Hardware ecc 3 byte / 256 data, write first half, get ecc, then second, if 512 byte pagesize */
	case NAND_ECC_HW3_256:
		break;

	/* Hardware ecc 3 byte / 512 byte data, write full page */
	case NAND_ECC_HW3_512:
		FLASH_WRITE_REG(NFLASH_ECC_CONTROL, 0x80000001); //set 31b = 0

	/* Hardware ecc 6 byte / 512 byte data, write full page */
	case NAND_ECC_HW6_512:
		break;

	default:
		printk (KERN_WARNING "Invalid NAND_ECC_MODE %d\n", this->eccmode);
		FLASH_WRITE_REG(NFLASH_ECC_CONTROL, 0x0); //set 31b = 0
		//BUG();
	}

	FLASH_WRITE_REG(NFLASH_ACCESS, NFLASH_DIRECT);

	for(i=0;i<mtd->oobblock;i++)
	{
		//udelay(5);
		FLASH_WRITE_DATA((page*mtd->oobblock)+i,this->data_poi[i]);
	}
	///////////////
	if(eccmode!=NAND_ECC_NONE)
	{
		opcode=FLASH_READ_REG(NFLASH_ECC_STATUS);
		while(!(opcode&0x80000000)) //polling flash access 31b
    	{
    	 	   opcode=FLASH_READ_REG(NFLASH_ECC_STATUS);
    	 	   //sl2312_flash_delay();
    	 	   schedule();
    	}


    	for(i=0;i<(mtd->oobblock/512);i++)
    	{
    		opcode=FLASH_READ_REG(NFLASH_ECC_CODE_GEN0+(i*4));

    		for(j=3;j>0;j--)
    		      oob_data[(mtd->oobsize-j-(i*4))] = (opcode<<((4-j)*8)) >>24;

    		for(j=0;j<4;j++)
    		{
    			ecc_code[15-i*4] = opcode;
    			ecc_code[15-i*4-1] = opcode>>8;
    			ecc_code[15-i*4-2] = opcode>>16;
    		}
    	}

    	//disable ecc
    	FLASH_WRITE_REG(NFLASH_ECC_CONTROL, 0x00000000);

    	/* Write out OOB data */
    	for(i=0;i<mtd->oobsize;i++)
    	{
    		//udelay(5);
			FLASH_WRITE_DATA((page*mtd->oobblock)+mtd->oobblock+i,oob_data[i]);
		}
    }
    else
    {
    	for(i=0;i<mtd->oobsize;i++)
    	{
    		//udelay(5);
			FLASH_WRITE_DATA((page*mtd->oobblock)+mtd->oobblock+i,0xff);
		}
    }


	/* call wait ready function */
	status = this->waitfunc (mtd, this, FL_WRITING);
	FLASH_WRITE_REG(NFLASH_ECC_CONTROL, 0x0); //set 31b = 0
	/* See if device thinks it succeeded */
	if (status & 0x01) {
		DEBUG (MTD_DEBUG_LEVEL0, "%s: " "Failed write, page 0x%08x, ", __FUNCTION__, page);
		FLASH_WRITE_REG(NFLASH_ECC_CONTROL, 0x0); //set 31b = 0
		return -EIO;
	}

#ifdef CONFIG_MTD_NAND_VERIFY_WRITE
	/*
	 * The NAND device assumes that it is always writing to
	 * a cleanly erased page. Hence, it performs its internal
	 * write verification only on bits that transitioned from
	 * 1 to 0. The device does NOT verify the whole page on a
	 * byte by byte basis. It is possible that the page was
	 * not completely erased or the page is becoming unusable
	 * due to wear. The read with ECC would catch the error
	 * later when the ECC page check fails, but we would rather
	 * catch it early in the page write stage. Better to write
	 * no data than invalid data.
	 */

	/* Send command to read back the page */
	this->cmdfunc (mtd, NAND_CMD_READ0, 0, page);
	/* Loop through and verify the data */
	if (this->verify_buf(mtd, this->data_poi, mtd->oobblock)) {
		DEBUG (MTD_DEBUG_LEVEL0, "%s: " "Failed write verify, page 0x%08x ", __FUNCTION__, page);
		return -EIO;
	}

	/* check, if we have a fs-supplied oob-buffer */
	if (oob_buf) {
		if (this->verify_buf(mtd, oob_data, mtd->oobsize)) {
			DEBUG (MTD_DEBUG_LEVEL0, "%s: " "Failed write verify, page 0x%08x ", __FUNCTION__, page);
			return -EIO;
		}
	} else {
		if (eccmode != NAND_ECC_NONE) {
			int ecc_bytes = 0;

			switch (this->eccmode) {
			case NAND_ECC_SOFT:
			case NAND_ECC_HW3_256: ecc_bytes = (mtd->oobblock == 512) ? 6 : 3; break;
			case NAND_ECC_HW3_512: ecc_bytes = 3; break;
			case NAND_ECC_HW6_512: ecc_bytes = 6; break;
			}



			for(i=0;i < (mtd->oobblock+mtd->oobsize);i++)
			{
				if(i>=mtd->oobblock)
					oob_data[i-mtd->oobblock] = FLASH_READ_DATA((page*mtd->oobblock) +i);
				else
					oob_data[0] = FLASH_READ_DATA((page*mtd->oobblock) +i);
			}

			if(this->eccmode == NAND_ECC_HW3_512)
			{
				for(i=0;i<(mtd->oobblock/512);i++)
    			{
    				for(j=0;j<3;j++)
    				{
    				    if (oob_data[mtd->oobsize-1-j-4*i] != ecc_code[15-j-4*i]) {
							DEBUG (MTD_DEBUG_LEVEL0,
							       "%s: Failed ECC write "
						       "verify, page 0x%08x, " "%6i bytes were succesful\n", __FUNCTION__, page, i);
							return -EIO;
						}
    				}
    			}
			}
		}//eccmode != NAND_ECC_NONE
	}
	/*
	 * Terminate the read command. This is faster than sending a reset command or
	 * applying a 20us delay before issuing the next programm sequence.
	 * This is not a problem for all chips, but I have found a bunch of them.
	 */
	//this->select_chip(mtd, -1);
	//this->select_chip(mtd, 0);
#endif

	return 0;
}

/*
 * NAND write with iovec
 */
static int sl2312_nand_writev (struct mtd_info *mtd, const struct kvec *vecs, unsigned long count,
		loff_t to, size_t * retlen)
{
	return (sl2312_nand_writev_ecc (mtd, vecs, count, to, retlen, NULL, 0));
}

static int sl2312_nand_writev_ecc (struct mtd_info *mtd, const struct kvec *vecs, unsigned long count,
		loff_t to, size_t * retlen, u_char *eccbuf, struct nand_oobinfo *oobsel)
{
	int i, page, len, total_len, ret = 0, written = 0;
	struct nand_chip *this = mtd->priv;

	/* Calculate total length of data */
	total_len = 0;
	for (i = 0; i < count; i++)
		total_len += (int) vecs[i].iov_len;

	DEBUG (MTD_DEBUG_LEVEL3,
	       "nand_writev: to = 0x%08x, len = %i, count = %ld\n", (unsigned int) to, (unsigned int) total_len, count);

	/* Do not allow write past end of page */
	if ((to + total_len) > mtd->size) {
		DEBUG (MTD_DEBUG_LEVEL0, "nand_writev: Attempted write past end of device\n");
		return -EINVAL;
	}

	/* reject writes, which are not page aligned */
	if (NOTALIGNED (to) || NOTALIGNED(total_len)) {
		printk (KERN_NOTICE "nand_write_ecc: Attempt to write not page aligned data\n");
		return -EINVAL;
	}

	// if oobsel is NULL, use chip defaults
	if (oobsel == NULL)
		oobsel = &mtd->oobinfo;

	/* Shift to get page */
	page = ((int) to) >> this->page_shift;

	/* Grab the lock and see if the device is available */
	sl2312_nand_get_chip (this, mtd, FL_WRITING, NULL);

	/* Select the NAND device */
	this->select_chip(mtd, 0);

	/* Check the WP bit */
	if (!(sl2312_device_ready(mtd) & 0x80)) {
		DEBUG (MTD_DEBUG_LEVEL0, "sl2312_nand_writev_ecc: Device is write protected!!!\n");
		ret = -EIO;
		goto out;
	}

	/* Loop until all iovecs' data has been written */
	len = 0;
	while (count) {
		/*
		 *  Check, if the tuple gives us not enough data for a
		 *  full page write. Then we can use the iov direct,
		 *  else we have to copy into data_buf.
		 */
		if ((vecs->iov_len - len) >= mtd->oobblock) {
			this->data_poi = (u_char *) vecs->iov_base;
			this->data_poi += len;
			len += mtd->oobblock;
			/* Check, if we have to switch to the next tuple */
			if (len >= (int) vecs->iov_len) {
				vecs++;
				len = 0;
				count--;
			}
		} else {
			/*
			 * Read data out of each tuple until we have a full page
			 * to write or we've read all the tuples.
		 	*/
			int cnt = 0;
			while ((cnt < mtd->oobblock) && count) {
				if (vecs->iov_base != NULL && vecs->iov_len) {
					this->data_buf[cnt++] = ((u_char *) vecs->iov_base)[len++];
				}
				/* Check, if we have to switch to the next tuple */
				if (len >= (int) vecs->iov_len) {
					vecs++;
					len = 0;
					count--;
				}
			}
			this->data_poi = this->data_buf;
		}

		/* We use the same function for write and writev !) */
		ret = sl2312_nand_write_page (mtd, this, page, NULL, oobsel);
		if (ret)
			goto out;

		/* Update written bytes count */
		written += mtd->oobblock;;

		/* Increment page address */
		page++;
	}

out:
	/* De-select the NAND device */
	//this->select_chip(mtd, -1);

	/* Wake up anyone waiting on the device */
	spin_lock_bh (&this->chip_lock);
	this->state = FL_READY;
	wake_up (&this->wq);
	spin_unlock_bh (&this->chip_lock);

	*retlen = written;
	return ret;
}

/*
static u_char ffchars[] = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};
*/
/*
 * NAND write out-of-band
 */
static int sl2312_nand_write_oob (struct mtd_info *mtd, loff_t to, size_t len, size_t * retlen, const u_char * buf)
{
	int column, page, status, ret = 0, j=0;
	struct nand_chip *this = mtd->priv;
	u_char *databuf, *oobbuf;


		databuf = &this->data_buf[0];
		oobbuf = &this->data_buf[mtd->oobblock];
		for (j = 0; j < mtd->oobsize; j++)
			oobbuf[j] = 0xff;
//#ifdef CONFIG_MTD_NAND_VERIFY_WRITE
//	int 	i;
//#endif

	DEBUG (MTD_DEBUG_LEVEL3, "nand_write_oob: to = 0x%08x, len = %i\n", (unsigned int) to, (int) len);

	/* Shift to get page */
	page = ((int) to) >> this->page_shift;

	/* Mask to get column */
	column = to & 0x1f;

	/* Initialize return length value */
	*retlen = 0;

	/* Do not allow write past end of page */
	if ((column + len) > mtd->oobsize) {
		DEBUG (MTD_DEBUG_LEVEL0, "nand_write_oob: Attempt to write past end of page\n");
		return -EINVAL;
	}

	/* Grab the lock and see if the device is available */
	sl2312_nand_get_chip (this, mtd, FL_WRITING, NULL);

	/* Select the NAND device */
	this->select_chip(mtd, 0);

	/* Reset the chip. Some chips (like the Toshiba TC5832DC found
	   in one of my DiskOnChip 2000 test units) will clear the whole
	   data page too if we don't do this. I have no clue why, but
	   I seem to have 'fixed' it in the doc2000 driver in
	   August 1999.  dwmw2. */
	this->cmdfunc(mtd, NAND_CMD_RESET, -1, -1);

	/* Check the WP bit */
	if (!(sl2312_device_ready(mtd) & 0x80)) {
		DEBUG (MTD_DEBUG_LEVEL0, "nand_write_oob: Device is write protected!!!\n");
		ret = -EIO;
		goto out;
	}
	/* Write out desired data */
	this->cmdfunc (mtd, NAND_CMD_SEQIN, mtd->oobblock, page);

	sl2312_nand_read_ecc (mtd, page, mtd->oobblock , retlen, databuf, oobbuf, NULL);

    for(j=column;j<(column+len);j++)
    	oobbuf[j] = buf[j-column];
    sl2312_nand_write_ecc (mtd, page, mtd->oobblock, retlen, databuf, oobbuf, NULL);

	status = this->waitfunc (mtd, this, FL_WRITING);

	/* See if device thinks it succeeded */
	if (status & 0x01) {
		DEBUG (MTD_DEBUG_LEVEL0, "nand_write_oob: " "Failed write, page 0x%08x\n", page);
		ret = -EIO;
		goto out;
	}
	/* Return happy */
	*retlen = len;


out:
	/* De-select the NAND device */
	//this->select_chip(mtd, -1);

	/* Wake up anyone waiting on the device */
	spin_lock_bh (&this->chip_lock);
	this->state = FL_READY;
	wake_up (&this->wq);
	spin_unlock_bh (&this->chip_lock);

	return ret;
}

/*
 * NAND sync
 */
static void sl2312_nand_sync (struct mtd_info *mtd)
{
	struct nand_chip *this = mtd->priv;
	DECLARE_WAITQUEUE (wait, current);

	DEBUG (MTD_DEBUG_LEVEL3, "nand_sync: called\n");

retry:
	/* Grab the spinlock */
	spin_lock_bh (&this->chip_lock);

	/* See what's going on */
	switch (this->state) {
	case FL_READY:
	case FL_SYNCING:
		this->state = FL_SYNCING;
		spin_unlock_bh (&this->chip_lock);
		break;

	default:
		/* Not an idle state */
		add_wait_queue (&this->wq, &wait);
		spin_unlock_bh (&this->chip_lock);
		schedule ();

		remove_wait_queue (&this->wq, &wait);
		goto retry;
	}

	/* Lock the device */
	spin_lock_bh (&this->chip_lock);

	/* Set the device to be ready again */
	if (this->state == FL_SYNCING) {
		this->state = FL_READY;
		wake_up (&this->wq);
	}

	/* Unlock the device */
	spin_unlock_bh (&this->chip_lock);
}


/*
 * Scan for the NAND device
 */
int sl2312_nand_scan (struct mtd_info *mtd, int maxchips)
{
	int i, j, nand_maf_id, nand_dev_id, busw;
	struct nand_chip *this = mtd->priv;
	unsigned char id[4];

	/* Get buswidth to select the correct functions*/
	busw = this->options & NAND_BUSWIDTH_16;

	/* check for proper chip_delay setup, set 20us if not */
	if (!this->chip_delay)
		this->chip_delay = 20;

	/* check, if a user supplied command function given */
	if (this->cmdfunc == NULL)
		this->cmdfunc = sl2312_nand_command;

	/* check, if a user supplied wait function given */
	if (this->waitfunc == NULL)
		this->waitfunc = sl2312_nand_waitfunc;

	if (!this->select_chip)
		this->select_chip = sl2312_nand_select_chip;
	if (!this->write_byte)
		this->write_byte = sl2312_nand_write_byte; //busw ? nand_write_byte16 : nand_write_byte;
	if (!this->read_byte)
		this->read_byte = sl2312_nand_read_byte; //busw ? nand_read_byte16 : nand_read_byte;
//	if (!this->write_word)
//		this->write_word = nand_write_word;
//	if (!this->read_word)
//		this->read_word = nand_read_word;
//	if (!this->block_bad)
		this->block_bad = sl2312_nand_block_bad; //nand_block_bad;
	if (!this->block_markbad)
		this->block_markbad = sl2312_nand_default_block_markbad;
	if (!this->write_buf)
		this->write_buf = sl2312_nand_write_buf; //busw ? nand_write_buf16 : nand_write_buf;
	if (!this->read_buf)
		this->read_buf = sl2312_nand_read_buf; //busw ? nand_read_buf16 : nand_read_buf;
	if (!this->verify_buf)
		this->verify_buf = sl2312_nand_verify_buf; //busw ? nand_verify_buf16 : nand_verify_buf;
	if (!this->scan_bbt)
		this->scan_bbt = sl2312_nand_scan_bbt;

	/* Select the device */
	this->select_chip(mtd, 0);

	/* Read manufacturer and device IDs */
	nand_read_id(0,id);

	nand_maf_id = id[0];
	nand_dev_id = id[1];

	/* Print and store flash device information */
	for (i = 0; nand_flash_ids[i].name != NULL; i++) {

		if (nand_dev_id != nand_flash_ids[i].id)
			continue;

		if (!mtd->name) mtd->name = nand_flash_ids[i].name;
		this->chipsize = nand_flash_ids[i].chipsize << 20;

		/* New devices have all the information in additional id bytes */
		if (!nand_flash_ids[i].pagesize) {
			int extid;

			/* The 4th id byte is the important one */
			extid = id[3];
			/* Calc pagesize */
			mtd->oobblock = 1024 << (extid & 0x3);
			extid >>= 2;
			/* Calc oobsize */
			mtd->oobsize = (8 << (extid & 0x03)) * (mtd->oobblock / 512);
			extid >>= 2;
			/* Calc blocksize. Blocksize is multiples of 64KiB */
			mtd->erasesize = (64 * 1024)  << (extid & 0x03);
			extid >>= 2;
			/* Get buswidth information */
			busw = (extid & 0x01) ? NAND_BUSWIDTH_16 : 0;

		} else {
			/* Old devices have this data hardcoded in the
			 * device id table */
			mtd->erasesize = nand_flash_ids[i].erasesize;
			mtd->oobblock = nand_flash_ids[i].pagesize;
			mtd->oobsize = mtd->oobblock / 32;
			busw = nand_flash_ids[i].options & NAND_BUSWIDTH_16;
		}

		/* Check, if buswidth is correct. Hardware drivers should set
		 * this correct ! */
		if (busw != (this->options & NAND_BUSWIDTH_16)) {
			printk (KERN_INFO "NAND device: Manufacturer ID:"
				" 0x%02x, Chip ID: 0x%02x (%s %s)\n", nand_maf_id, nand_dev_id,
				nand_manuf_ids[i].name , mtd->name);
			printk (KERN_WARNING
				"NAND bus width %d instead %d bit\n",
					(this->options & NAND_BUSWIDTH_16) ? 16 : 8,
					busw ? 16 : 8);
			this->select_chip(mtd, -1);
			return 1;
		}

		/* Calculate the address shift from the page size */
		this->page_shift = ffs(mtd->oobblock) - 1;
		this->bbt_erase_shift = this->phys_erase_shift = ffs(mtd->erasesize) - 1;
		this->chip_shift = ffs(this->chipsize) - 1;

		/* Set the bad block position */
		this->badblockpos = mtd->oobblock > 512 ?
			NAND_LARGE_BADBLOCK_POS : NAND_SMALL_BADBLOCK_POS;

		/* Get chip options, preserve non chip based options */
		this->options &= ~NAND_CHIPOPTIONS_MSK;
		this->options |= nand_flash_ids[i].options & NAND_CHIPOPTIONS_MSK;
		/* Set this as a default. Board drivers can override it, if neccecary */
		this->options |= NAND_NO_AUTOINCR;
		/* Check if this is a not a samsung device. Do not clear the options
		 * for chips which are not having an extended id.
		 */
		if (nand_maf_id != NAND_MFR_SAMSUNG && !nand_flash_ids[i].pagesize)
			this->options &= ~NAND_SAMSUNG_LP_OPTIONS;

		/* Check for AND chips with 4 page planes */
	//	if (this->options & NAND_4PAGE_ARRAY)
	//		this->erase_cmd = multi_erase_cmd;
	//	else
	//		this->erase_cmd = single_erase_cmd;

		/* Do not replace user supplied command function ! */
	//	if (mtd->oobblock > 512 && this->cmdfunc == nand_command)
	//		this->cmdfunc = nand_command_lp;

		/* Try to identify manufacturer */
		for (j = 0; nand_manuf_ids[j].id != 0x0; j++) {
			if (nand_manuf_ids[j].id == nand_maf_id)
				break;
		}
		printk (KERN_INFO "NAND device: Manufacturer ID:"
			" 0x%02x, Chip ID: 0x%02x (%s %s)\n", nand_maf_id, nand_dev_id,
			nand_manuf_ids[j].name , nand_flash_ids[i].name);
		break;
	}
	/////////////////////////////

	for (i=1; i < maxchips; i++) {
		this->select_chip(mtd, i);

		/* Send the command for reading device ID */
		nand_read_id(1,id);

		/* Read manufacturer and device IDs */
		if (nand_maf_id != id[0] ||
		    nand_dev_id != id[1])
			break;
	}
	if (i > 1)
		printk(KERN_INFO "%d NAND chips detected\n", i);

	/* Allocate buffers, if neccecary */
	if (!this->oob_buf) {
		size_t len;
		len = mtd->oobsize << (this->phys_erase_shift - this->page_shift);
		this->oob_buf = kmalloc (len, GFP_KERNEL);
		if (!this->oob_buf) {
			printk (KERN_ERR "nand_scan(): Cannot allocate oob_buf\n");
			return -ENOMEM;
		}
		this->options |= NAND_OOBBUF_ALLOC;
	}

	if (!this->data_buf) {
		size_t len;
		len = mtd->oobblock + mtd->oobsize;
		this->data_buf = kmalloc (len, GFP_KERNEL);
		if (!this->data_buf) {
			if (this->options & NAND_OOBBUF_ALLOC)
				kfree (this->oob_buf);
			printk (KERN_ERR "nand_scan(): Cannot allocate data_buf\n");
			return -ENOMEM;
		}
		this->options |= NAND_DATABUF_ALLOC;
	}

	/* Store the number of chips and calc total size for mtd */
	this->numchips = i;
	mtd->size = i * this->chipsize;
	/* Convert chipsize to number of pages per chip -1. */
	this->pagemask = (this->chipsize >> this->page_shift) - 1;
	/* Preset the internal oob buffer */
	memset(this->oob_buf, 0xff, mtd->oobsize << (this->phys_erase_shift - this->page_shift));

	/* If no default placement scheme is given, select an
	 * appropriate one */
	if (!this->autooob) {
		/* Select the appropriate default oob placement scheme for
		 * placement agnostic filesystems */
		switch (mtd->oobsize) {
		case 8:
			this->autooob = &nand_oob_8;
			break;
		case 16:
			this->autooob = &nand_oob_16;
			break;
		case 64:
			this->autooob = &nand_oob_64;
			break;
		default:
			printk (KERN_WARNING "No oob scheme defined for oobsize %d\n",
				mtd->oobsize);
			BUG();
		}
	}

	/* The number of bytes available for the filesystem to place fs dependend
	 * oob data */
	if (this->options & NAND_BUSWIDTH_16) {
		mtd->oobavail = mtd->oobsize - (this->autooob->eccbytes + 2);
		if (this->autooob->eccbytes & 0x01)
			mtd->oobavail--;
	} else
		mtd->oobavail = mtd->oobsize - (this->autooob->eccbytes + 1);


	/*
	 * check ECC mode, default to software
	 * if 3byte/512byte hardware ECC is selected and we have 256 byte pagesize
	 * fallback to software ECC
	*/
	this->eccsize = 256;	/* set default eccsize */
	this->eccbytes = 3;

	switch (this->eccmode) {
	case NAND_ECC_HW12_2048:
		if (mtd->oobblock < 2048) {
			printk(KERN_WARNING "2048 byte HW ECC not possible on %d byte page size, fallback to SW ECC\n",
			       mtd->oobblock);
			this->eccmode = NAND_ECC_SOFT;
			this->calculate_ecc = nand_calculate_ecc;
			this->correct_data = nand_correct_data;
		} else
			this->eccsize = 2048;
		break;

	case NAND_ECC_HW3_512:
	case NAND_ECC_HW6_512:
	case NAND_ECC_HW8_512:
		if (mtd->oobblock == 256) {
			printk (KERN_WARNING "512 byte HW ECC not possible on 256 Byte pagesize, fallback to SW ECC \n");
			this->eccmode = NAND_ECC_SOFT;
			this->calculate_ecc = nand_calculate_ecc;
			this->correct_data = nand_correct_data;
		} else
			this->eccsize = 512; /* set eccsize to 512 */
		break;

	case NAND_ECC_HW3_256:
		break;

	case NAND_ECC_NONE:
		printk (KERN_WARNING "NAND_ECC_NONE selected by board driver. This is not recommended !!\n");
		this->eccmode = NAND_ECC_NONE;
		break;

	case NAND_ECC_SOFT:
		this->calculate_ecc = nand_calculate_ecc;
		this->correct_data = nand_correct_data;
		break;

	default:
		printk (KERN_WARNING "Invalid NAND_ECC_MODE %d\n", this->eccmode);
		BUG();
	}

	/* Check hardware ecc function availability and adjust number of ecc bytes per
	 * calculation step
	*/
	switch (this->eccmode) {
	case NAND_ECC_HW12_2048:
		this->eccbytes += 4;
	case NAND_ECC_HW8_512:
		this->eccbytes += 2;
	case NAND_ECC_HW6_512:
		this->eccbytes += 3;
//	case NAND_ECC_HW3_512:
	case NAND_ECC_HW3_256:
		if (this->calculate_ecc && this->correct_data && this->enable_hwecc)
			break;
		printk (KERN_WARNING "No ECC functions supplied, Hardware ECC not possible\n");
		BUG();
	}

	mtd->eccsize = this->eccsize;

	/* Set the number of read / write steps for one page to ensure ECC generation */
	switch (this->eccmode) {
	case NAND_ECC_HW12_2048:
		this->eccsteps = mtd->oobblock / 2048;
		break;
	case NAND_ECC_HW3_512:
	case NAND_ECC_HW6_512:
	case NAND_ECC_HW8_512:
		this->eccsteps = mtd->oobblock / 512;
		break;
	case NAND_ECC_HW3_256:
	case NAND_ECC_SOFT:
		this->eccsteps = mtd->oobblock / 256;
		break;

	case NAND_ECC_NONE:
		this->eccsteps = 1;
		break;
	}

	/* Initialize state, waitqueue and spinlock */
	this->state = FL_READY;
	init_waitqueue_head (&this->wq);
	spin_lock_init (&this->chip_lock);

	/* De-select the device */
	this->select_chip(mtd, 0);

	/* Print warning message for no device */
	if (!mtd->size) {
		printk (KERN_WARNING "No NAND device found!!!\n");
		return 1;
	}

	/* Fill in remaining MTD driver data */
	mtd->type = MTD_NANDFLASH;
	mtd->flags = MTD_CAP_NANDFLASH | MTD_ECC;
	mtd->ecctype = MTD_ECC_SW;
	mtd->erase = sl2312_nand_erase;
	mtd->point = NULL;
	mtd->unpoint = NULL;
	mtd->read = sl2312_nand_read;
	mtd->write = sl2312_nand_write;
	mtd->read_ecc = sl2312_nand_read_ecc;
	mtd->write_ecc = sl2312_nand_write_ecc;
	mtd->read_oob = sl2312_nand_read_oob;
	mtd->write_oob = sl2312_nand_write_oob;
	mtd->readv = NULL;
	mtd->writev = sl2312_nand_writev;
	mtd->writev_ecc = sl2312_nand_writev_ecc;
	mtd->sync = sl2312_nand_sync;
	mtd->lock = NULL;
	mtd->unlock = NULL;
	mtd->suspend = NULL;
	mtd->resume = NULL;
	mtd->block_isbad = sl2312_nand_block_isbad;
	mtd->block_markbad = sl2312_nand_block_markbad;

	/* and make the autooob the default one */
	memcpy(&mtd->oobinfo, this->autooob, sizeof(mtd->oobinfo));

	mtd->owner = THIS_MODULE;

	/* Build bad block table */
	return this->scan_bbt (mtd);
}

/*End Add function*/

/*
 * Main initialization routine
 */
extern int nand_correct_data(struct mtd_info *mtd, u_char *dat, u_char *read_ecc, u_char *calc_ecc);

int __init sl2312_mtd_init (void)
{
	struct nand_chip *this;
	int err = 0;
	struct mtd_partition *parts;
	int nr_parts = 0;
	int ret, data, *base;

	printk("NAND MTD Driver Start Init ......\n");

    	base = (unsigned int *)(IO_ADDRESS(SL2312_GLOBAL_BASE) + 0x30);
    	data = *base;
    	data&=0xffffffeb;
    	data|=0x3; //disable p & s flash
        *base = data;

	/* Allocate memory for MTD device structure and private data */
	sl2312_mtd = kmalloc(sizeof(struct mtd_info) + sizeof(struct nand_chip), GFP_KERNEL);
	if (!sl2312_mtd) {
		printk ("Unable to allocate SL2312 NAND MTD device structure.\n");
		err = -ENOMEM;
		goto out;
	}

      //  sl2312_device_setup();

	/* io is indirect via a register so don't need to ioremap address */

	/* Get pointer to private data */
	this = (struct nand_chip *) (&sl2312_mtd[1]);

	/* Initialize structures */
	memset((char *) sl2312_mtd, 0, sizeof(struct mtd_info));
	memset((char *) this, 0, sizeof(struct nand_chip));

	/* Link the private data with the MTD structure */
	sl2312_mtd->priv = this;
	sl2312_mtd->name = "sl2312-nand";

	/* Set address of NAND IO lines */
	this->IO_ADDR_R = (void __iomem *)IO_ADDRESS((SL2312_FLASH_CTRL_BASE+NFLASH_DATA)); //(unsigned long)&(sl2312_ndfmcptr->dtr);
	this->IO_ADDR_W = (void __iomem *)IO_ADDRESS((SL2312_FLASH_CTRL_BASE+NFLASH_DATA)); //(unsigned long)&(sl2312_ndfmcptr->dtr);
	this->read_byte = sl2312_nand_read_byte;
    this->write_byte = sl2312_nand_write_byte;
    this->write_buf = sl2312_nand_write_buf;
	this->read_buf = sl2312_nand_read_buf;
	this->verify_buf = sl2312_nand_verify_buf;
	this->select_chip = sl2312_nand_select_chip;
	this->block_bad = sl2312_nand_block_bad;
	this->hwcontrol = sl2312_hwcontrol;
	this->dev_ready = sl2312_device_ready;
	this->cmdfunc = sl2312_nand_command;
	this->waitfunc = sl2312_nand_waitfunc;
	//this->calculate_ecc = sl2312_readecc;
	this->enable_hwecc = sl2312_enable_hwecc;
	this->eccmode = NAND_ECC_HW3_512;
	/*this->eccsize = 512;	*/
	/* 20 us command delay time */
	this->chip_delay = 20;

	this->correct_data = nand_correct_data;
//	this->scan_bbt = sl2312_nand_scan_bbt;

	/* Allocate memory for internal data buffer */
	this->data_buf = kmalloc (sizeof(u_char) * (sl2312_mtd->oobblock + sl2312_mtd->oobsize), GFP_KERNEL);
	if (!this->data_buf) {
		printk ("Unable to allocate NAND data buffer.\n");
		err = -ENOMEM;
		goto out_ior;
	}

	/* Scan to find existance of the device */
	if (sl2312_nand_scan(sl2312_mtd, 1)) {
		err = -ENXIO;
		goto out_ior;
	}

	/* Register the partitions */
	parts = sl2312_partitions;
	nr_parts = sizeof(sl2312_partitions)/sizeof(*parts);

	ret = add_mtd_partitions(sl2312_mtd, sl2312_partitions, nr_parts);
	/*If we got an error, free all resources.*/
	if (ret < 0) {
		del_mtd_partitions(sl2312_mtd);
		map_destroy(sl2312_mtd);
	}
	goto out;

//out_buf:
//	kfree (this->data_buf);
out_ior:
out:
	printk("NAND MTD Driver Init Success ......\n");
	return err;
}

module_init(sl2312_mtd_init);

/*
 * Clean up routine
 */
#ifdef MODULE
static void __exit sl2312_cleanup (void)
{
	struct nand_chip *this = (struct nand_chip *) &sl2312_mtd[1];

	/* Unregister partitions */
	del_mtd_partitions(sl2312_mtd);

	/* Unregister the device */
	del_mtd_device (sl2312_mtd);

	/* Free internal data buffers */
	kfree (this->data_buf);

	/* Free the MTD device structure */
	kfree (sl2312_mtd);
}
module_exit(sl2312_cleanup);
#endif

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alice Hennessy <ahennessy@mvista.com>");
MODULE_DESCRIPTION("Glue layer for SmartMediaCard on Toshiba RBsl2312");
