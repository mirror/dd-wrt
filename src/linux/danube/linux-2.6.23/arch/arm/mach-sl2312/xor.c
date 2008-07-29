/*
 * arch/arm/mach-sl2312/xor.c
 *
 * Support functions for the Gemini Soc. This is
 * a HW XOR unit that is specifically designed for use with RAID5
 * applications.  This driver provides an interface that is used by
 * the Linux RAID stack.
 *
 * Original Author: Jason Lee<jason@storlink.com.tw>
 *
 * Contributors:Sanders<sanders@storlink.com.tw>
 				Jason Lee<jason@storlink.com.tw>
 *
 *
 * Maintainer: Jason Lee<jason@storlink.com.tw>
 *
 * Copyright (C) 2005 Storlink Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *
 * History:	(06/25/2005, DJ) Initial Creation
 *
 *	Versing 1.0.0	Initial version
 */

#include <linux/types.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/list.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/mm.h>
#include <asm/irq.h>
#include <asm/delay.h>
#include <asm/uaccess.h>
#include <asm/cacheflush.h>
#include <asm/hardware.h>
#include <asm/arch/xor.h>
#include <asm/pci.h>
#include <linux/version.h>

/*
 * pick up local definitions
 */
#define XOR_SW_FILL_IN
#include "hw_xor.h"


//#define XOR_DEBUG
//#define XOR_TEST	1
#ifdef XOR_TEST
#define TEST_ITERATION 1000
#define SPIN_WAIT	1
#endif
#ifdef XOR_DEBUG
#define DPRINTK(s, args...) printk("Gemini XOR: " s "\n", ## args)
#define DENTER() DPRINTK("Entered...\n");
#define DEXIT() DPRINTK("Exited...\n");
#else
#define DPRINTK(s, args...)
#define DENTER()
#define DEXIT()
#endif

//#define SPIN_WAIT

/* globals */
static RAID_T tp;
static RAID_TXDMA_CTRL_T 	txdma_ctrl;
RAID_RXDMA_CTRL_T			rxdma_ctrl;

//#ifndef SPIN_WAIT
static spinlock_t	    raid_lock;
//#endif

static unsigned int     tx_desc_virtual_base;
static unsigned int     rx_desc_virtual_base;
RAID_DESCRIPTOR_T	*tx_desc_ptr;
RAID_DESCRIPTOR_T	*rx_desc_ptr;

/* static prototypes */
#define DMA_MALLOC(size,handle)		pci_alloc_consistent(NULL,size,handle)
#define DMA_MFREE(mem,size,handle)	pci_free_consistent(NULL,size,mem,handle)

static int gemini_xor_init_desc(void);

static unsigned int raid_read_reg(unsigned int offset)
{
    unsigned int    reg_val;

    reg_val = readl(RAID_BASE_ADDR + offset);
	return (reg_val);
}

static void raid_write_reg(unsigned int offset,unsigned int data,unsigned int bit_mask)
{
	unsigned int reg_val;
    unsigned int *addr;

	reg_val = ( raid_read_reg(offset) & (~bit_mask) ) | (data & bit_mask);
	addr = (unsigned int *)(RAID_BASE_ADDR + offset);
    writel(reg_val,addr);
	return;
}

#ifndef SPIN_WAIT
__inline__ void xor_queue_descriptor(void)
{
	unsigned int flags,status=1;

	DPRINTK("Going to sleep");

	while(status){
		yield();
		//schedule();
		spin_lock_irqsave(&raid_lock,flags);
		status = tp.busy;
		spin_unlock_irqrestore(&raid_lock, flags);
	}
//	tp.status = COMPLETE;
	DPRINTK("woken up!");

}
#endif

#ifdef SPIN_WAIT
static void gemini_xor_isr(int d_n)
#else
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,4,28)
static void gemini_xor_isr(int irq, void *dev_id, struct pt_regs *regs)
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
static irqreturn_t gemini_xor_isr(int irq, void *dev_instance, struct pt_regs *regs)
#endif
#endif
{

	unsigned int err;
	RAID_DMA_STATUS_T 	dma_status;
//	RAID_DESCRIPTOR_T *rdesc,*tdesc;
//	unsigned int *paddr;

	dma_status.bits32 = raid_read_reg(RAID_DMA_STATUS);
#ifdef SPIN_WAIT
	while( (dma_status.bits32& (1<<31) ) ==0 ){
		udelay(1);
		dma_status.bits32 = raid_read_reg(RAID_DMA_STATUS);
	}

/*	tdesc = tp.tx_first_desc;
	rdesc = tp.rx_first_desc;
	for(d_n;d_n>0;d_n--){
		if( tdesc->func_ctrl.bits.own == DMA ){
			paddr = tdesc;
			printk("error tx desc:0x%x\n",*paddr++);
			printk("error tx desc:0x%x\n",*paddr++);
			printk("error tx desc:0s%x\n",*paddr++);
			printk("error tx desc:0x%x\n",*paddr);
			while(1);
		}
		tdesc = (RAID_DESCRIPTOR_T *)((tdesc->next_desc_addr.bits32 & 0xfffffff0)+tx_desc_virtual_base);
	}

	if( rdesc->func_ctrl.bits.own == DMA ){
		paddr = rdesc;
		printk("error rx desc:0x%x\n",*paddr++);
		printk("error rx desc:0x%x\n",*paddr++);
		printk("error rx desc:0s%x\n",*paddr++);
		printk("error rx desc:0x%x\n",*paddr);
		while(1);
	}
*/
#endif

	if(dma_status.bits32 & ((1<<31)|(1<<26))){
		// if no bug , we can turn off rx finish interrupt
		dma_status.bits32 = raid_read_reg(RAID_DMA_STATUS);
		err = raid_read_reg(RAID_DMA_DEVICE_ID);
		tp.busy = 0;

		if(err&0x00FF0000){
			tp.status = ERROR;
			printk("XOR:<HW>%s error code %x\n",(err&0x00F00000)?"tx":"rx",err);

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,4,28)
			return ;
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#ifndef SPIN_WAIT
			return IRQ_RETVAL(IRQ_HANDLED);
#endif
#endif
		}
		// 16~19 rx error code
		// 20~23 tx error codd

		dma_status.bits.tsFinishI = 1;
		dma_status.bits.rsFinishI = 1;
		raid_write_reg(RAID_DMA_STATUS, dma_status.bits32,0x84000000);	// clear INT

//		printk("xor %d\n",d_n);
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,4,28)
			return ;
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#ifndef SPIN_WAIT
			return IRQ_RETVAL(IRQ_HANDLED);
#endif
#endif
	}

	#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,4,28)
		return ;
	#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
	#ifndef SPIN_WAIT
		printk("XOR: DMA status register(0x%8x)\n",dma_status.bits32);
		return IRQ_RETVAL(IRQ_HANDLED);
	#endif
	#endif
}

void
xor_gemini_2(unsigned long bytes, unsigned long *p1, unsigned long *p2)
{
	int status=0;
	unsigned int flags;

	if(bytes > (1<<(SRAM_PAR_SIZE+11))){
		printk("XOR: out of SRAM partition!![0x%x]\n",(unsigned int)bytes);
	}

	spin_lock_irqsave(&raid_lock,flags);
	while(tp.status != COMPLETE){
		spin_unlock_irqrestore(&raid_lock, flags);
		//printk("XOR yield2\n");
#ifdef XOR_SW_FILL_IN
		xor_arm4regs_2(bytes,p1,p2);
		return ;
#else
		yield();
#endif
	}
	spin_unlock_irqrestore(&raid_lock, flags);
	tp.status = RUNNING;

	// flush the cache to memory before H/W XOR touches them
	consistent_sync(p1, bytes, DMA_BIDIRECTIONAL);
	consistent_sync(p2, bytes, DMA_TO_DEVICE);


	tp.tx_desc = tp.tx_first_desc;
	tp.rx_desc = tp.rx_first_desc;
	if((tp.tx_desc->func_ctrl.bits.own == CPU)/*&&(tp.rx_desc->func_ctrl.bits.own == DMA)*/){
		// prepare tx descript
		raid_write_reg(RAID_FCHDMA_CURR_DESC,(unsigned int)tp.tx_desc-tx_desc_virtual_base,0xffffffff);
		tp.tx_desc->buf_addr = (unsigned int)__pa(p1);		// physical address
    	tp.tx_desc->func_ctrl.bits.buffer_size = bytes;    		/* total frame byte count */
//    	tp.tx_desc->flg_status.bits_cmd_status.bcc = 2;			// first descript
//    	tp.tx_desc->flg_status.bits_cmd_status.mode = 0;		// only support XOR command
		tp.tx_desc->flg_status.bits32 = 0x00020000;
    	tp.tx_desc->next_desc_addr.bits.sof_eof = 0x03;          /*only one descriptor*/
    	tp.tx_desc->func_ctrl.bits.own = DMA;	        		/* set owner bit */
    	tp.tx_cur_desc = (RAID_DESCRIPTOR_T *)((tp.tx_desc->next_desc_addr.bits32 & 0xfffffff0)+tx_desc_virtual_base);
		wmb();
    	tp.tx_desc = tp.tx_cur_desc;
    	tp.tx_desc->buf_addr = (unsigned int)__pa(p2);		// pysical address
    	tp.tx_desc->func_ctrl.bits.buffer_size = bytes;    		/* total frame byte count */
//    	tp.tx_desc->flg_status.bits_cmd_status.bcc = 1;			// last descript
//    	tp.tx_desc->flg_status.bits_cmd_status.mode = 0;		// only support XOR command
		tp.tx_desc->flg_status.bits32 = 0x00010000;
    	tp.tx_desc->func_ctrl.bits.own = DMA;	        		/* set owner bit */
    	tp.tx_desc->next_desc_addr.bits32 = 0x0000000b;// end of descript
    	tp.tx_cur_desc = (RAID_DESCRIPTOR_T *)((tp.tx_desc->next_desc_addr.bits32 & 0xfffffff0)+tx_desc_virtual_base);							// keep last descript

    	wmb();
    	// prepare rx descript
    	raid_write_reg(RAID_STRDMA_CURR_DESC,(unsigned int)tp.rx_desc-rx_desc_virtual_base,0xFFFFFFFf);
    	tp.rx_desc->buf_addr = (unsigned int)__pa(p1);
    	tp.rx_desc->func_ctrl.bits.buffer_size = bytes;    		/* total frame byte count */
    	tp.rx_desc->flg_status.bits32 = 0;				// link data from XOR
//    	tp.rx_cur_desc->next_desc_addr.bits.sof_eof = 0x03;          /*only one descriptor*/
    	tp.rx_desc->func_ctrl.bits.own = DMA;	        		/* set owner bit */
    	tp.rx_desc->next_desc_addr.bits32 = 0x0000000b;// end of descript

	}
	else{
	 	/* no free tx descriptor */
	 	printk("XOR:no free tx descript");
		return ;
	}

	// change status
//	tp.status = RUNNING;
	status = tp.busy = 1;

	// start tx DMA
	rxdma_ctrl.bits.rd_start = 1;
	// start rx DMA
	txdma_ctrl.bits.td_start = 1;

	raid_write_reg(RAID_FCHDMA_CTRL, txdma_ctrl.bits32,0x80000000);
	raid_write_reg(RAID_STRDMA_CTRL, rxdma_ctrl.bits32,0x80000000);

#ifdef SPIN_WAIT
	gemini_xor_isr(2);
#else
	xor_queue_descriptor();
#endif

	tp.tx_desc->next_desc_addr.bits32 = ((unsigned long)tp.tx_first_desc - tx_desc_virtual_base + sizeof(RAID_DESCRIPTOR_T)*2) ;
	tp.status = COMPLETE;
//	tp.rx_desc->next_desc_addr.bits32 = ((unsigned long)tp.rx_first_desc - tx_desc_virtual_base + sizeof(RAID_DESCRIPTOR_T)*1) ;
//	tp.rx_desc = tp.rx_first_desc ;
//	tp.rx_desc->func_ctrl.bits.own = DMA;

}

void
xor_gemini_3(unsigned long bytes, unsigned long *p1, unsigned long *p2,
		unsigned long *p3)
{
	int status=0;
	unsigned int flags;

	if(bytes > (1<<(SRAM_PAR_SIZE+11))){
		printk("XOR: out of SRAM partition!![0x%x]\n",(unsigned int)bytes);
	}

	spin_lock_irqsave(&raid_lock,flags);
	if(tp.status != COMPLETE){
		spin_unlock_irqrestore(&raid_lock, flags);
		//printk("XOR yield3\n");
#ifdef XOR_SW_FILL_IN
		xor_arm4regs_3(bytes,p1,p2,p3);
		return;
#else
		yield();
#endif
	}
	spin_unlock_irqrestore(&raid_lock, flags);
	tp.status = RUNNING;

	// flush the cache to memory before H/W XOR touches them
	consistent_sync(p1, bytes, DMA_BIDIRECTIONAL);
	consistent_sync(p2, bytes, DMA_TO_DEVICE);
	consistent_sync(p3, bytes, DMA_TO_DEVICE);

	tp.tx_desc = tp.tx_first_desc;
	tp.rx_desc = tp.rx_first_desc;
	if((tp.tx_desc->func_ctrl.bits.own == CPU)/*&&(tp.rx_desc->func_ctrl.bits.own == DMA)*/){
		// prepare tx descript
		raid_write_reg(RAID_FCHDMA_CURR_DESC,(unsigned int)tp.tx_desc-tx_desc_virtual_base,0xffffffff);
		tp.tx_desc->buf_addr = (unsigned int)__pa(p1);		// physical address
    	tp.tx_desc->func_ctrl.bits.buffer_size = bytes;    		/* total frame byte count */
//    	tp.tx_desc->flg_status.bits_cmd_status.bcc = 2;			// first descript
//    	tp.tx_desc->flg_status.bits_cmd_status.mode = 0;		// only support XOR command
		tp.tx_desc->flg_status.bits32 = 0x00020000;
    	tp.tx_desc->next_desc_addr.bits.sof_eof = 0x03;          /*only one descriptor*/
    	tp.tx_desc->func_ctrl.bits.own = DMA;	        		/* set owner bit */
    	tp.tx_cur_desc = (RAID_DESCRIPTOR_T *)((tp.tx_desc->next_desc_addr.bits32 & 0xfffffff0)+tx_desc_virtual_base);

    	tp.tx_desc = tp.tx_cur_desc;
    	tp.tx_desc->buf_addr = (unsigned int)__pa(p2);		// pysical address
    	tp.tx_desc->func_ctrl.bits.buffer_size = bytes;    		/* total frame byte count */
//    	tp.tx_desc->flg_status.bits_cmd_status.bcc = 0;			// first descript
//    	tp.tx_desc->flg_status.bits_cmd_status.mode = 0;		// only support XOR command
		tp.tx_desc->flg_status.bits32 = 0x0000000;
    	tp.tx_desc->next_desc_addr.bits.sof_eof = 0x03;          /*only one descriptor*/
    	tp.tx_desc->func_ctrl.bits.own = DMA;	        		/* set owner bit */
    	tp.tx_cur_desc = (RAID_DESCRIPTOR_T *)((tp.tx_desc->next_desc_addr.bits32 & 0xfffffff0)+tx_desc_virtual_base);

    	tp.tx_desc = tp.tx_cur_desc;
    	tp.tx_desc->buf_addr = (unsigned int)__pa(p3);		// pysical address
    	tp.tx_desc->func_ctrl.bits.buffer_size = bytes;    		/* total frame byte count */
//    	tp.tx_desc->flg_status.bits_cmd_status.bcc = 1;			// last descript
//    	tp.tx_desc->flg_status.bits_cmd_status.mode = 0;		// only support XOR command
		tp.tx_desc->flg_status.bits32 = 0x00010000;
    	tp.tx_desc->func_ctrl.bits.own = DMA;	        		/* set owner bit */
    	tp.tx_desc->next_desc_addr.bits32 = 0x0000000b;// end of descript
    	tp.tx_cur_desc = (RAID_DESCRIPTOR_T *)((tp.tx_desc->next_desc_addr.bits32 & 0xfffffff0)+tx_desc_virtual_base);							// keep last descript

    	// prepare rx descript
    	raid_write_reg(RAID_STRDMA_CURR_DESC,(unsigned int)tp.rx_desc-rx_desc_virtual_base,0xFFFFFFFf);
    	tp.rx_desc->buf_addr = (unsigned int)__pa(p1);
    	tp.rx_desc->func_ctrl.bits.buffer_size = bytes;    		/* total frame byte count */
    	tp.rx_desc->flg_status.bits32 = 0;				// link data from XOR
//    	tp.rx_cur_desc->next_desc_addr.bits.sof_eof = 0x03;          /*only one descriptor*/
    	tp.rx_desc->func_ctrl.bits.own = DMA;	        		/* set owner bit */
    	tp.rx_desc->next_desc_addr.bits32 = 0x0000000b;// end of descript

	}
	else{
	 	/* no free tx descriptor */
	 	printk("XOR:no free tx descript \n");
		return ;
	}

	// change status
//	tp.status = RUNNING;
	status = tp.busy = 1;

	// start tx DMA
	rxdma_ctrl.bits.rd_start = 1;
	// start rx DMA
	txdma_ctrl.bits.td_start = 1;
	wmb();
	raid_write_reg(RAID_FCHDMA_CTRL, txdma_ctrl.bits32,0x80000000);
	raid_write_reg(RAID_STRDMA_CTRL, rxdma_ctrl.bits32,0x80000000);

#ifdef SPIN_WAIT
	gemini_xor_isr(3);
#else
	xor_queue_descriptor();
#endif
	tp.tx_desc->next_desc_addr.bits32 = ((unsigned long)tp.tx_first_desc - tx_desc_virtual_base + sizeof(RAID_DESCRIPTOR_T)*3) | 0x0B;
	tp.status = COMPLETE;
//	tp.rx_desc->next_desc_addr.bits32 = ((unsigned long)tp.rx_first_desc - tx_desc_virtual_base + sizeof(RAID_DESCRIPTOR_T)*1) | 0x0B;
	//tp.rx_desc = tp.rx_first_desc ;
//	tp.rx_desc->func_ctrl.bits.own = DMA;

}

void
xor_gemini_4(unsigned long bytes, unsigned long *p1, unsigned long *p2,
		unsigned long *p3, unsigned long *p4)
{
	int status=0;
	unsigned int flags;

	if(bytes > (1<<(SRAM_PAR_SIZE+11))){
		printk("XOR: out of SRAM partition!![0x%x]\n",(unsigned int)bytes);
	}

	spin_lock_irqsave(&raid_lock,flags);
	if(tp.status != COMPLETE){
		spin_unlock_irqrestore(&raid_lock, flags);
		//printk("S\n");
#ifdef XOR_SW_FILL_IN
		xor_arm4regs_4(bytes,p1,p2,p3,p4);
		return;
#else
		msleep(1);
		yield();
#endif
	}
	spin_unlock_irqrestore(&raid_lock, flags);

	tp.status = RUNNING;

	// flush the cache to memory before H/W XOR touches them
	consistent_sync(p1, bytes, DMA_BIDIRECTIONAL);
	consistent_sync(p2, bytes, DMA_TO_DEVICE);
	consistent_sync(p3, bytes, DMA_TO_DEVICE);
	consistent_sync(p4, bytes, DMA_TO_DEVICE);

	tp.tx_desc = tp.tx_first_desc;
	tp.rx_desc = tp.rx_first_desc;
	if((tp.tx_desc->func_ctrl.bits.own == CPU)/*&&(tp.rx_desc->func_ctrl.bits.own == DMA)*/){
		// prepare tx descript
		raid_write_reg(RAID_FCHDMA_CURR_DESC,(unsigned int)tp.tx_desc-tx_desc_virtual_base,0xffffffff);
		tp.tx_desc->buf_addr = (unsigned int)__pa(p1);		// physical address
    	tp.tx_desc->func_ctrl.bits.buffer_size = bytes;    		/* total frame byte count */
//    	tp.tx_desc->flg_status.bits_cmd_status.bcc = 2;			// first descript
//    	tp.tx_desc->flg_status.bits_cmd_status.mode = 0;		// only support XOR command
		tp.tx_desc->flg_status.bits32 = 0x00020000;
    	tp.tx_desc->next_desc_addr.bits.sof_eof = 0x03;          /*only one descriptor*/
    	tp.tx_desc->func_ctrl.bits.own = DMA;	        		/* set owner bit */
    	tp.tx_cur_desc = (RAID_DESCRIPTOR_T *)((tp.tx_desc->next_desc_addr.bits32 & 0xfffffff0)+tx_desc_virtual_base);

    	tp.tx_desc = tp.tx_cur_desc;
    	tp.tx_cur_desc->buf_addr = (unsigned int)__pa(p2);		// pysical address
    	tp.tx_desc->func_ctrl.bits.buffer_size = bytes;    		/* total frame byte count */
//    	tp.tx_desc->flg_status.bits_cmd_status.bcc = 0;			// first descript
//    	tp.tx_desc->flg_status.bits_cmd_status.mode = 0;		// only support XOR command
		tp.tx_desc->flg_status.bits32 = 0x00000000;
    	tp.tx_cur_desc->next_desc_addr.bits.sof_eof = 0x03;          /*only one descriptor*/
    	tp.tx_desc->func_ctrl.bits.own = DMA;	        		/* set owner bit */
    	tp.tx_cur_desc = (RAID_DESCRIPTOR_T *)((tp.tx_desc->next_desc_addr.bits32 & 0xfffffff0)+tx_desc_virtual_base);

    	tp.tx_desc = tp.tx_cur_desc;
    	tp.tx_desc->buf_addr = (unsigned int)__pa(p3);		// pysical address
    	tp.tx_desc->func_ctrl.bits.buffer_size = bytes;    		/* total frame byte count */
//    	tp.tx_desc->flg_status.bits_cmd_status.bcc = 0;			// first descript
//    	tp.tx_desc->flg_status.bits_cmd_status.mode = 0;		// only support XOR command
		tp.tx_desc->flg_status.bits32 = 0x00000000;
    	tp.tx_desc->next_desc_addr.bits.sof_eof = 0x03;          /*only one descriptor*/
    	tp.tx_desc->func_ctrl.bits.own = DMA;	        		/* set owner bit */
    	tp.tx_cur_desc = (RAID_DESCRIPTOR_T *)((tp.tx_desc->next_desc_addr.bits32 & 0xfffffff0)+tx_desc_virtual_base);


    	tp.tx_desc = tp.tx_cur_desc;
    	tp.tx_desc->buf_addr = (unsigned int)__pa(p4);		// pysical address
    	tp.tx_desc->func_ctrl.bits.buffer_size = bytes;    		/* total frame byte count */
//    	tp.tx_desc->flg_status.bits_cmd_status.bcc = 1;			// last descript
//    	tp.tx_desc->flg_status.bits_cmd_status.mode = 0;		// only support XOR command
		tp.tx_desc->flg_status.bits32 = 0x00010000;
//    	tp.tx_cur_desc->next_desc_addr.bits.sof_eof = 0x03;          /*only one descriptor*/
    	tp.tx_desc->func_ctrl.bits.own = DMA;	        		/* set owner bit */
    	tp.tx_desc->next_desc_addr.bits32 = 0x0000000b;// end of descript
    	tp.tx_cur_desc = (RAID_DESCRIPTOR_T *)((tp.tx_desc->next_desc_addr.bits32 & 0xfffffff0)+tx_desc_virtual_base);							// keep last descript

    	// prepare rx descript
    	raid_write_reg(RAID_STRDMA_CURR_DESC,(unsigned int)tp.rx_desc-rx_desc_virtual_base,0xFFFFFFFF);
    	tp.rx_desc->buf_addr = (unsigned int)__pa(p1);
    	tp.rx_desc->func_ctrl.bits.buffer_size = bytes;    		/* total frame byte count */
    	tp.rx_desc->flg_status.bits32 = 0;				// link data from XOR
//    	tp.rx_cur_desc->next_desc_addr.bits.sof_eof = 0x03;          /*only one descriptor*/
    	tp.rx_desc->func_ctrl.bits.own = DMA;	        		/* set owner bit */
    	tp.rx_desc->next_desc_addr.bits32 = 0x0000000b;// end of descript

	}
	else{
	 	/* no free tx descriptor */
	 	printk("XOR:no free tx descript");
		return ;
	}

	// change status
//	tp.status = RUNNING;
	status = tp.busy = 1;

	// start tx DMA
	rxdma_ctrl.bits.rd_start = 1;
	// start rx DMA
	txdma_ctrl.bits.td_start = 1;
	wmb();
	raid_write_reg(RAID_FCHDMA_CTRL, txdma_ctrl.bits32,0x80000000);
	raid_write_reg(RAID_STRDMA_CTRL, rxdma_ctrl.bits32,0x80000000);

#ifdef SPIN_WAIT
	gemini_xor_isr(4);
#else
	xor_queue_descriptor();
#endif

	tp.tx_desc->next_desc_addr.bits32 = ((unsigned long)tp.tx_first_desc - tx_desc_virtual_base + sizeof(RAID_DESCRIPTOR_T)*4) | 0x0B;
	tp.status = COMPLETE;
//	tp.rx_desc->next_desc_addr.bits32 = ((unsigned long)tp.rx_first_desc - tx_desc_virtual_base + sizeof(RAID_DESCRIPTOR_T)*1) | 0x0B;
	//tp.rx_desc = tp.rx_first_desc ;
//	tp.rx_desc->func_ctrl.bits.own = DMA;

}

void
xor_gemini_5(unsigned long bytes, unsigned long *p1, unsigned long *p2,
		unsigned long *p3, unsigned long *p4, unsigned long *p5)
{

	int status=0;
	unsigned int flags;


	if(bytes > (1<<(SRAM_PAR_SIZE+11))){
		printk("XOR: out of SRAM partition!![0x%x]\n",(unsigned int)bytes);
	}

	spin_lock_irqsave(&raid_lock,flags);
	while(tp.status != COMPLETE){
		spin_unlock_irqrestore(&raid_lock, flags);
		//printk("XOR yield5\n");
#ifdef XOR_SW_FILL_IN
		xor_arm4regs_5(bytes,p1,p2,p3,p4,p5);
		return;
#else
		msleep(1);
		yield();
#endif
	}
	spin_unlock_irqrestore(&raid_lock, flags);
	tp.status = RUNNING;

	// flush the cache to memory before H/W XOR touches them
	consistent_sync(p1, bytes, DMA_BIDIRECTIONAL);
	consistent_sync(p2, bytes, DMA_TO_DEVICE);
	consistent_sync(p3, bytes, DMA_TO_DEVICE);
	consistent_sync(p4, bytes, DMA_TO_DEVICE);
	consistent_sync(p5, bytes, DMA_TO_DEVICE);

	tp.tx_desc = tp.tx_first_desc;
	tp.rx_desc = tp.rx_first_desc;
	if((tp.tx_desc->func_ctrl.bits.own == CPU)/*&&(tp.rx_desc->func_ctrl.bits.own == DMA)*/){
		// prepare tx descript
		raid_write_reg(RAID_FCHDMA_CURR_DESC,(unsigned int)tp.tx_desc-tx_desc_virtual_base,0xffffffff);
		tp.tx_desc->buf_addr = (unsigned int)__pa(p1);		// physical address
    	tp.tx_desc->func_ctrl.bits.buffer_size = bytes;    		/* total frame byte count */
//    	tp.tx_desc->flg_status.bits_cmd_status.bcc = 2;			// first descript
//    	tp.tx_desc->flg_status.bits_cmd_status.mode = 0;		// only support XOR command
		tp.tx_desc->flg_status.bits32 = 0x00020000;
    	tp.tx_desc->next_desc_addr.bits.sof_eof = 0x03;          /*only one descriptor*/
    	wmb();
    	tp.tx_desc->func_ctrl.bits.own = DMA;	        		/* set owner bit */
    	tp.tx_cur_desc = (RAID_DESCRIPTOR_T *)((tp.tx_desc->next_desc_addr.bits32 & 0xfffffff0)+tx_desc_virtual_base);

    	tp.tx_desc = tp.tx_cur_desc;
    	tp.tx_desc->buf_addr = (unsigned int)__pa(p2);		// pysical address
    	tp.tx_desc->func_ctrl.bits.buffer_size = bytes;    		/* total frame byte count */
//    	tp.tx_desc->flg_status.bits_cmd_status.bcc = 0;			// first descript
//    	tp.tx_desc->flg_status.bits_cmd_status.mode = 0;		// only support XOR command
		tp.tx_desc->flg_status.bits32 = 0x00000000;
    	tp.tx_desc->next_desc_addr.bits.sof_eof = 0x03;          /*only one descriptor*/
    	wmb();
    	tp.tx_desc->func_ctrl.bits.own = DMA;	        		/* set owner bit */
    	tp.tx_cur_desc = (RAID_DESCRIPTOR_T *)((tp.tx_desc->next_desc_addr.bits32 & 0xfffffff0)+tx_desc_virtual_base);

    	tp.tx_desc = tp.tx_cur_desc;
    	tp.tx_desc->buf_addr = (unsigned int)__pa(p3);		// pysical address
    	tp.tx_desc->func_ctrl.bits.buffer_size = bytes;    		/* total frame byte count */
//    	tp.tx_desc->flg_status.bits_cmd_status.bcc = 0;			// first descript
//    	tp.tx_desc->flg_status.bits_cmd_status.mode = 0;		// only support XOR command
		tp.tx_desc->flg_status.bits32 = 0x00000000;
    	tp.tx_desc->next_desc_addr.bits.sof_eof = 0x03;          /*only one descriptor*/
    	wmb();
    	tp.tx_desc->func_ctrl.bits.own = DMA;	        		/* set owner bit */
    	tp.tx_cur_desc = (RAID_DESCRIPTOR_T *)((tp.tx_desc->next_desc_addr.bits32 & 0xfffffff0)+tx_desc_virtual_base);

		tp.tx_desc = tp.tx_cur_desc;
    	tp.tx_desc->buf_addr = (unsigned int)__pa(p4);		// pysical address
    	tp.tx_desc->func_ctrl.bits.buffer_size = bytes;    		/* total frame byte count */
//    	tp.tx_desc->flg_status.bits_cmd_status.bcc = 0;			// first descript
//    	tp.tx_desc->flg_status.bits_cmd_status.mode = 0;		// only support XOR command
		tp.tx_desc->flg_status.bits32 = 0x00000000;
    	tp.tx_desc->next_desc_addr.bits.sof_eof = 0x03;          /*only one descriptor*/
    	wmb();
    	tp.tx_desc->func_ctrl.bits.own = DMA;	        		/* set owner bit */
    	tp.tx_cur_desc = (RAID_DESCRIPTOR_T *)((tp.tx_desc->next_desc_addr.bits32 & 0xfffffff0)+tx_desc_virtual_base);


    	tp.tx_desc = tp.tx_cur_desc;
    	tp.tx_desc->buf_addr = (unsigned int)__pa(p5);		// pysical address
    	tp.tx_desc->func_ctrl.bits.buffer_size = bytes;    		/* total frame byte count */
//    	tp.tx_desc->flg_status.bits_cmd_status.bcc = 1;			// last descript
//    	tp.tx_desc->flg_status.bits_cmd_status.mode = 0;		// only support XOR command
//    	tp.tx_cur_desc->next_desc_addr.bits.sof_eof = 0x03;          /*only one descriptor*/
		tp.tx_desc->flg_status.bits32 = 0x00010000;
    	tp.tx_desc->func_ctrl.bits.own = DMA;	        		/* set owner bit */
    	tp.tx_desc->next_desc_addr.bits32 = 0x0000000b;// end of descript
    	tp.tx_cur_desc = (RAID_DESCRIPTOR_T *)((tp.tx_desc->next_desc_addr.bits32 & 0xfffffff0)+tx_desc_virtual_base);
    	tp.tx_finished_desc = tp.tx_desc;								// keep last descript

    	// prepare rx descript
    	raid_write_reg(RAID_STRDMA_CURR_DESC,(unsigned int)tp.rx_desc-rx_desc_virtual_base,0xFFFFFFFF);
    	tp.rx_desc->buf_addr = (unsigned int)__pa(p1);
    	tp.rx_desc->func_ctrl.bits.buffer_size = bytes;    		/* total frame byte count */
    	tp.rx_desc->flg_status.bits32 = 0;				// link data from XOR
//    	tp.rx_cur_desc->next_desc_addr.bits.sof_eof = 0x03;          /*only one descriptor*/
    	tp.rx_desc->func_ctrl.bits.own = DMA;	        		/* set owner bit */
    	tp.rx_desc->next_desc_addr.bits32 = 0x0000000b;// end of descript

	}
	else{
	 	/* no free tx descriptor */
	 	printk("XOR:no free tx descript");
		return ;
	}

	// change status
//	tp.status = RUNNING;
	status = tp.busy = 1;

	// start tx DMA
	rxdma_ctrl.bits.rd_start = 1;
	// start rx DMA
	txdma_ctrl.bits.td_start = 1;
	wmb();
	raid_write_reg(RAID_FCHDMA_CTRL, txdma_ctrl.bits32,0x80000000);
	raid_write_reg(RAID_STRDMA_CTRL, rxdma_ctrl.bits32,0x80000000);

#ifdef SPIN_WAIT
	gemini_xor_isr(5);
#else
	xor_queue_descriptor();
#endif

	tp.tx_desc->next_desc_addr.bits32 = ((unsigned long)tp.tx_first_desc - tx_desc_virtual_base + sizeof(RAID_DESCRIPTOR_T)*5) | 0x0B;
	tp.status = COMPLETE;
//	tp.rx_desc->next_desc_addr.bits32 = ((unsigned long)tp.rx_first_desc - tx_desc_virtual_base + sizeof(RAID_DESCRIPTOR_T)*1) | 0x0B;
	//tp.rx_desc = tp.rx_first_desc ;
//	tp.rx_desc->func_ctrl.bits.own = DMA;

}

#ifdef XOR_TEST
void
raid_memset(unsigned int *p1, unsigned int pattern, unsigned int bytes)
{
	int status=0,i;

	if(bytes > (1<<(SRAM_PAR_SIZE+11))){
		printk("XOR: out of SRAM partition!![0x%x]\n",(unsigned int)bytes);
	}

	*p1 = pattern;

	// flush the cache to memory before H/W XOR touches them
	consistent_sync(p1, bytes, DMA_BIDIRECTIONAL);

	while(tp.status != COMPLETE){
		DPRINTK("XOR yield\n");
		//schedule();
		yield();
	}
	tp.status = RUNNING;

	tp.tx_desc = tp.tx_first_desc;
	tp.rx_desc = tp.rx_first_desc;
	if((tp.tx_desc->func_ctrl.bits.own == CPU)/*&&(tp.rx_desc->func_ctrl.bits.own == DMA)*/){
		// prepare tx descript
		raid_write_reg(RAID_FCHDMA_CURR_DESC,(unsigned int)tp.tx_desc-tx_desc_virtual_base,0xFFFFFFFF);
		tp.tx_desc->buf_addr = (unsigned int)__pa(p1);		// physical address
    	tp.tx_desc->func_ctrl.bits.buffer_size = 4;    		/* total frame byte count */
    	tp.tx_desc->flg_status.bits_cmd_status.bcc = bytes;			// bytes to fill
    	tp.tx_desc->flg_status.bits_cmd_status.mode = CMD_FILL;		// only support memory FILL command
    	tp.tx_desc->next_desc_addr.bits.sof_eof = 0x03;          /*only one descriptor*/
    	tp.tx_desc->func_ctrl.bits.own = DMA;	        		/* set owner bit */
    	tp.tx_desc->next_desc_addr.bits32 = 0x0000000b;
//    	tp.tx_cur_desc = (RAID_DESCRIPTOR_T *)((tp.tx_desc->next_desc_addr.bits32 & 0xFFFFFFF0)+tx_desc_virtual_base);

    	// prepare rx descript
    	raid_write_reg(RAID_STRDMA_CURR_DESC,(unsigned int)tp.rx_desc-rx_desc_virtual_base,0xFFFFFFFF);
    	tp.rx_desc->buf_addr = (unsigned int)__pa(p1);
    	tp.rx_desc->func_ctrl.bits.buffer_size = bytes;    		/* total frame byte count */
    	tp.rx_desc->flg_status.bits32 = 0;				// link data from XOR
    	tp.rx_cur_desc->next_desc_addr.bits.sof_eof = 0x03;          /*only one descriptor*/
    	tp.rx_desc->func_ctrl.bits.own = DMA;	        		/* set owner bit */
//    	tp.rx_cur_desc = (RAID_DESCRIPTOR_T *)((tp.rx_cur_desc->next_desc_addr.bits32 & 0xfffffff0)+rx_desc_virtual_base);
    	tp.rx_desc->next_desc_addr.bits32 = 0x0000000b;// end of descript
    	tp.rx_finished_desc = tp.rx_desc;

	}
	else{
	 	/* no free tx descriptor */
	 	printk("XOR:no free tx descript");
		return ;
	}

	// change status
	//tp.status = RUNNING;
	status = tp.busy = 1;

	// start tx DMA
	rxdma_ctrl.bits.rd_start = 1;
	// start rx DMA
	txdma_ctrl.bits.td_start = 1;

	raid_write_reg(RAID_FCHDMA_CTRL, txdma_ctrl.bits32,0x80000000);
	raid_write_reg(RAID_STRDMA_CTRL, rxdma_ctrl.bits32,0x80000000);

#ifdef SPIN_WAIT
	gemini_xor_isr(2);
#else
	xor_queue_descriptor();
#endif

	for(i=1; i<(bytes/sizeof(int)); i++) {
		if(p1[0]!=p1[i]){
			printk("pattern set error!\n");
			while(1);
		}
	}

	tp.tx_desc->next_desc_addr.bits32 = ((unsigned long)tp.tx_first_desc - tx_desc_virtual_base + sizeof(RAID_DESCRIPTOR_T)*1) ;
	tp.status = COMPLETE;
//	tp.rx_desc->next_desc_addr.bits32 = ((unsigned long)tp.rx_first_desc - tx_desc_virtual_base + sizeof(RAID_DESCRIPTOR_T)*1) ;
	//tp.rx_desc = tp.rx_first_desc ;
//	tp.rx_desc->func_ctrl.bits.own = DMA;

}
#endif

void
raid_memcpy(unsigned int *to, unsigned int *from, unsigned int bytes)
{
	int status=0,i;

	if(bytes > (1<<(SRAM_PAR_SIZE+11))){
		printk("XOR: out of SRAM partition!![0x%x]\n",(unsigned int)bytes);
	}

	// flush the cache to memory before H/W XOR touches them
	consistent_sync(to, bytes, DMA_BIDIRECTIONAL);
	consistent_sync(from,bytes, DMA_TO_DEVICE);

	while(tp.status != COMPLETE){
		DPRINTK("XOR yield\n");
		//schedule();
		yield();
	}
	tp.status = RUNNING;

	tp.tx_desc = tp.tx_first_desc;
	tp.rx_desc = tp.rx_first_desc;
	if((tp.tx_desc->func_ctrl.bits.own == CPU)/*&&(tp.rx_desc->func_ctrl.bits.own == DMA)*/){
		// prepare tx descript
		raid_write_reg(RAID_FCHDMA_CURR_DESC,(unsigned int)tp.tx_desc-tx_desc_virtual_base,0xFFFFFFFF);
		tp.tx_desc->buf_addr = (unsigned int)__pa(from);		// physical address
    	tp.tx_desc->func_ctrl.bits.buffer_size = bytes;    		/* total frame byte count */
    	tp.tx_desc->flg_status.bits32 = CMD_CPY;		// only support memory FILL command
    	tp.tx_desc->next_desc_addr.bits.sof_eof = 0x03;          /*only one descriptor*/
    	tp.tx_desc->func_ctrl.bits.own = DMA;	        		/* set owner bit */
    	tp.tx_desc->next_desc_addr.bits32 = 0x0000000b;
//    	tp.tx_cur_desc = (RAID_DESCRIPTOR_T *)((tp.tx_desc->next_desc_addr.bits32 & 0xFFFFFFF0)+tx_desc_virtual_base);

    	// prepare rx descript
    	raid_write_reg(RAID_STRDMA_CURR_DESC,(unsigned int)tp.rx_desc-rx_desc_virtual_base,0xFFFFFFFF);
    	tp.rx_desc->buf_addr = (unsigned int)__pa(to);
    	tp.rx_desc->func_ctrl.bits.buffer_size = bytes;    		/* total frame byte count */
    	tp.rx_desc->flg_status.bits32 = 0;				// link data from XOR
    	tp.rx_cur_desc->next_desc_addr.bits.sof_eof = 0x03;          /*only one descriptor*/
    	tp.rx_desc->func_ctrl.bits.own = DMA;	        		/* set owner bit */
//    	tp.rx_cur_desc = (RAID_DESCRIPTOR_T *)((tp.rx_cur_desc->next_desc_addr.bits32 & 0xfffffff0)+rx_desc_virtual_base);
    	tp.rx_desc->next_desc_addr.bits32 = 0x0000000b;// end of descript

	}
	else{
	 	/* no free tx descriptor */
	 	printk("XOR:no free tx descript");
		return ;
	}

	// change status
	//tp.status = RUNNING;
	status = tp.busy = 1;

	// start tx DMA
	rxdma_ctrl.bits.rd_start = 1;
	// start rx DMA
	txdma_ctrl.bits.td_start = 1;

	raid_write_reg(RAID_FCHDMA_CTRL, txdma_ctrl.bits32,0x80000000);
	raid_write_reg(RAID_STRDMA_CTRL, rxdma_ctrl.bits32,0x80000000);

#ifdef SPIN_WAIT
	gemini_xor_isr(2);
#else
	xor_queue_descriptor();
#endif

#ifdef XOR_TEST
	for(i=1; i<(bytes/sizeof(int)); i++) {
		if(to[i]!=from[i]){
			printk("pattern check error!\n");
			printk("offset=0x%x p1=%x p2=%x\n",i*4,to[i],from[i]);
			while(1);
		}
	}
#endif

	tp.tx_desc->next_desc_addr.bits32 = ((unsigned long)tp.tx_first_desc - tx_desc_virtual_base + sizeof(RAID_DESCRIPTOR_T)*1) ;
	tp.status = COMPLETE;
//	tp.rx_desc->next_desc_addr.bits32 = ((unsigned long)tp.rx_first_desc - tx_desc_virtual_base + sizeof(RAID_DESCRIPTOR_T)*1) ;
	//tp.rx_desc = tp.rx_first_desc ;
//	tp.rx_desc->func_ctrl.bits.own = DMA;

}
EXPORT_SYMBOL(raid_memcpy);

#ifdef XOR_TEST
int
raid_memchk(unsigned int *p1, unsigned int pattern, unsigned int bytes)
{
	int status=0;
	RAID_DMA_STATUS_T 	dma_status;

	if(bytes > (1<<(SRAM_PAR_SIZE+11))){
		printk("XOR: out of SRAM partition!![0x%x]\n",(unsigned int)bytes);
	}

	status = ((pattern&0xFFFF)%bytes )/4;
	p1[status] = pattern;

	while(tp.status != COMPLETE){
		DPRINTK("XOR yield\n");
		//schedule();
		yield();
	}
	tp.status = RUNNING;

	// flush the cache to memory before H/W XOR touches them
	consistent_sync(p1, bytes, DMA_BIDIRECTIONAL);

	tp.tx_desc = tp.tx_first_desc;
	if((tp.tx_desc->func_ctrl.bits.own == CPU)/*&&(tp.rx_desc->func_ctrl.bits.own == DMA)*/){
		// prepare tx descript
		raid_write_reg(RAID_FCHDMA_CURR_DESC,(unsigned int)tp.tx_desc-tx_desc_virtual_base,0xFFFFFFFF);
		tp.tx_desc->buf_addr = (unsigned int)__pa(p1);		// physical address
    	tp.tx_desc->func_ctrl.bits.raid_ctrl_status = 0;
    	tp.tx_desc->func_ctrl.bits.buffer_size = bytes ;    		/* total frame byte count */
    	tp.tx_desc->flg_status.bits32 = CMD_CHK;		// only support memory FILL command
    	tp.tx_desc->next_desc_addr.bits.sof_eof = 0x03;          /*only one descriptor*/
    	tp.tx_desc->func_ctrl.bits.own = DMA;	        		/* set owner bit */
    	tp.tx_desc->next_desc_addr.bits32 = 0x0000000b;
//    	tp.tx_cur_desc = (RAID_DESCRIPTOR_T *)((tp.tx_desc->next_desc_addr.bits32 & 0xFFFFFFF0)+tx_desc_virtual_base);

	}
	else{
	 	/* no free tx descriptor */
	 	printk("XOR:no free tx descript");
		return -1;
	}

	// change status
	//tp.status = RUNNING;
	status = tp.busy = 1;

	// start tx DMA
	txdma_ctrl.bits.td_start = 1;

	raid_write_reg(RAID_FCHDMA_CTRL, txdma_ctrl.bits32,0x80000000);
//	raid_write_reg(RAID_STRDMA_CTRL, rxdma_ctrl.bits32,0x80000000);

#ifdef SPIN_WAIT
	gemini_xor_isr(2);
#else
	xor_queue_descriptor();
#endif

//	dma_status.bits32 = raid_read_reg(RAID_DMA_STATUS);
//	if (dma_status.bits32 & (1<<15))	{

	if((tp.tx_first_desc->func_ctrl.bits.raid_ctrl_status & 0x2)) {
		status = 1;
//		raid_write_reg(RAID_DMA_STATUS,0x00008000,0x00080000);
	}
	else{
		status = 0;
	}

	tp.tx_desc->next_desc_addr.bits32 = ((unsigned long)tp.tx_first_desc - tx_desc_virtual_base + sizeof(RAID_DESCRIPTOR_T)*1) ;
	tp.status = COMPLETE;
//	tp.rx_desc->func_ctrl.bits.own = DMA;
	return status ;
}
#endif

int __init gemini_xor_init(void)
{
	unsigned int res;
	unsigned int *paddr1,*paddr2,*paddr3,i;
	unsigned volatile char  *charact;
	unsigned volatile short *two_char;
	unsigned volatile int   *four_char;

	// init descript
	res = gemini_xor_init_desc();
	if(res) {
		printk("Init RAID Descript Fail!!\n");
		return -res;
	}

	tp.device_name = "Gemini XOR Acceleration";

	// request irq
#ifndef SPIN_WAIT
	res = request_irq(IRQ_RAID, gemini_xor_isr, SA_INTERRUPT, tp.device_name, NULL);
#endif
	if(res){
		printk(KERN_ERR "%s: unable to request IRQ %d for "
		       "HW XOR %d\n", tp.device_name, IRQ_RAID, res);
		return -EBUSY;
	}

#ifdef XOR_TEST

RETEST:
	paddr1 = kmalloc(0x1000,GFP_KERNEL);
	paddr2 = kmalloc(0x1000,GFP_KERNEL);
	paddr3 = kmalloc(0x1000,GFP_KERNEL);
	for(i=0;i<TEST_ITERATION;i++) {
		printk("XOR test round %d\n",i);
		for(res=0;res<(0x1000)/sizeof(int);res++){		// prepare data pattern
		 	paddr1[res]= readl(0xf62000ac);
		 	paddr2[res]= readl(0xf62000ac);
		}
		for(res=0;res<0x1000/sizeof(int);res++){		// calculate xor by software
			paddr3[res] = paddr1[res]^paddr2[res];
		}
		xor_gemini_2(0x1000,paddr1,paddr2);				// calculate xor by hw
		for(res=0;res<0x1000/sizeof(int);res++){		// check error
			if(paddr1[res]!=paddr3[res]){
				printk("XOR ERROR\n");
				printk("[%d][0x%x]=0x%x should be %x\n",res,&paddr1[res],paddr1[res],paddr3[res]);
				while(1);
			}
		}
	}
	kfree(paddr1);
	kfree(paddr2);
	kfree(paddr3);


	// memcpy test
	paddr1 = kmalloc(0x4000,GFP_KERNEL);
	for(i=0;i<TEST_ITERATION;i++) {
		for(res=0;res<(0x4000)/sizeof(int);res++)
		 	paddr1[res]= readl(0xf62000ac);

		printk("MEMCOPY round %d\n",i);
		paddr2 = kmalloc(0x4000,GFP_KERNEL);
		raid_memcpy(paddr2,paddr1,0x4000);
		kfree(paddr2);
	}
	kfree(paddr1);

	// memset test
	for(i=0;i<TEST_ITERATION;i++) {
		raid_memset(paddr1,0xFFFFFFFF,0x4000);
		res = readl(0xf62000ac);
		printk("MEMFILL fill 0x%x round %d\n",res,i);
		paddr1 = kmalloc(0x4000,GFP_KERNEL);
		raid_memset(paddr1,res,0x4000);
		raid_memset(paddr1,0x0,0x4000);
		kfree(paddr1);
	}

	paddr1 = kmalloc(0x4000,GFP_KERNEL);
	for(i=0;i<TEST_ITERATION;i++){
		raid_memset(paddr1, i,0x4000);
		printk("Pattern check same ? ");
		res = raid_memchk(paddr1, i,0x4000);
		printk("%s\n",res?"Fail":"OK");
		if(res)	while(1);

		printk("Pattern check diff ? ");
		res = raid_memchk(paddr1,readl(0xf62000ac),0x4000);
		printk("%s\n",res?"OK":"Fail");
		if(!res)	while(1);
	}
	kfree(paddr1);

	// SRAM test
	raid_write_reg(RAID_PCR, 0,0x00000003);
	for(i=0;i<TEST_ITERATION;i++) {
		printk("SRAM test %d\n",i);
		charact = 0xF7000000;
		two_char = 0xF7000000;
		four_char = 0xF7000000;
		for(res=0;res<(16*1024)/sizeof(char);res++) {		// 8-bit access
			*charact++ = (unsigned char)res;
		}
		charact = 0xF7000000;
		for(res=0;res<(16*1024)/sizeof(char);res++) {
			if(*charact++ != (unsigned char)res){
				printk("SRAM data error(8)\n");
				while(1);
			}
		}

		for(res=0;res<(16*1024)/sizeof(short);res++) {		// 16-bit access
			*two_char++ = (unsigned short)res;
		}
		two_char = 0xF7000000;
		for(res=0;res<(16*1024)/sizeof(short);res++) {
			if(*two_char++ != (unsigned short)res){
				printk("SRAM data error(16)\n");
				while(1);
			}
		}

		for(res=0;res<(16*1024)/sizeof(int);res++) {		// 32-bit access
			*four_char++ = (unsigned int)res;
		}
		four_char = 0xF7000000;
		for(res=0;res<(16*1024)/sizeof(int);res++) {
			if(*four_char++ != (unsigned int)res){
				printk("SRAM data error(32)\n");
				while(1);
			}
		}
	}
	raid_write_reg(RAID_PCR, SRAM_PAR_SIZE,0x00000003);

#endif
	return 0;
}

void __exit gemini_xor_exit(void)
{
	DMA_MFREE(tp.tx_desc, TX_DESC_NUM*sizeof(RAID_DESCRIPTOR_T),(unsigned int)tp.tx_desc_dma);
	DMA_MFREE(tp.rx_desc, RX_DESC_NUM*sizeof(RAID_DESCRIPTOR_T),(unsigned int)tp.rx_desc_dma);
	free_irq(IRQ_RAID, NULL);
}


static int gemini_xor_init_desc(void)
{
	unsigned int i;
	dma_addr_t          tx_first_desc_dma;
	dma_addr_t          rx_first_desc_dma;
	RAID_DMA_STATUS_T 	dma_status;

	printk("Initial RAID Descripter...\n");

	tp.tx_desc = (RAID_DESCRIPTOR_T*)DMA_MALLOC(TX_DESC_NUM*sizeof(RAID_DESCRIPTOR_T),(dma_addr_t *)&tp.tx_desc_dma);
    tx_desc_virtual_base = (unsigned int)tp.tx_desc - (unsigned int)tp.tx_desc_dma;
    memset(tp.tx_desc,0x00,TX_DESC_NUM*sizeof(RAID_DESCRIPTOR_T));

	tp.rx_desc = (RAID_DESCRIPTOR_T*)DMA_MALLOC(RX_DESC_NUM*sizeof(RAID_DESCRIPTOR_T),(dma_addr_t *)&tp.rx_desc_dma);
    rx_desc_virtual_base = (unsigned int)tp.rx_desc - (unsigned int)tp.rx_desc_dma;
    memset(tp.rx_desc,0x00,RX_DESC_NUM*sizeof(RAID_DESCRIPTOR_T));
	printk("XOR:tx_desc = %08x\n",(unsigned int)tp.tx_desc);
    printk("XOR:rx_desc = %08x\n",(unsigned int)tp.rx_desc);
	printk("XOR:tx_desc_dma = %08x\n",(unsigned int)tp.tx_desc_dma);
	printk("XOR:rx_desc_dma = %08x\n",(unsigned int)tp.rx_desc_dma);

	if ((tp.tx_desc == NULL) || (tp.rx_desc == NULL)) {
		if (tp.tx_desc)
			DMA_MFREE(tp.tx_desc, TX_DESC_NUM*sizeof(RAID_DESCRIPTOR_T),(dma_addr_t)tp.tx_desc_dma);
		if (tp.rx_desc)
			DMA_MFREE(tp.rx_desc, RX_DESC_NUM*sizeof(RAID_DESCRIPTOR_T),(dma_addr_t)tp.rx_desc_dma);
		return -ENOMEM;
	}

	tp.tx_cur_desc = tp.tx_desc;  /* virtual address */
	tp.tx_finished_desc = tp.tx_desc; /* virtual address */
	tx_first_desc_dma = (dma_addr_t)tp.tx_desc_dma; /* physical address */
	for (i = 1; i < TX_DESC_NUM; i++) {
		tp.tx_desc->func_ctrl.bits.own = CPU;
		tp.tx_desc->func_ctrl.bits.buffer_size = 0;
		tp.tx_desc_dma = tp.tx_desc_dma + sizeof(RAID_DESCRIPTOR_T);
//		tp.tx_desc->next_desc_addr.bits32 = (unsigned int)tp.tx_desc_dma | 0x0B;
		tp.tx_desc->next_desc_addr.bits32 = ((unsigned int)tx_first_desc_dma | 0x0B) + i*0x10;
		tp.tx_desc = &tp.tx_desc[1];
	}
	tp.tx_desc->func_ctrl.bits.own = DMA;
	tp.tx_desc->next_desc_addr.bits32 = (unsigned int)tx_first_desc_dma|0x0b;
	tp.tx_desc = tp.tx_cur_desc;
	tp.tx_desc_dma = (unsigned int*)tx_first_desc_dma;
	tp.tx_first_desc = tp.tx_desc ;

	tp.rx_cur_desc = tp.rx_desc;  /* virtual address */
	tp.rx_finished_desc = tp.rx_desc; /* virtual address */
	rx_first_desc_dma = (dma_addr_t)tp.rx_desc_dma; /* physical address */
	for (i = 1; i < RX_DESC_NUM; i++) {
		tp.rx_desc->func_ctrl.bits.own = DMA;
		tp.rx_desc->func_ctrl.bits.buffer_size = 0;
		tp.rx_desc_dma = tp.rx_desc_dma + sizeof(RAID_DESCRIPTOR_T);
//		tp.rx_desc->next_desc_addr.bits32 = (unsigned int)tp.rx_desc_dma | 0x0B;
		tp.rx_desc->next_desc_addr.bits32 = ((unsigned int)rx_first_desc_dma | 0x0B) + i*0x10;
		tp.rx_desc = &tp.rx_desc[1];
	}
	tp.rx_desc->func_ctrl.bits.own = DMA;
	tp.rx_desc->next_desc_addr.bits32 = rx_first_desc_dma|0x0b;
	tp.rx_desc = tp.rx_cur_desc;
	tp.rx_desc_dma = (unsigned int*)rx_first_desc_dma;
	tp.rx_first_desc = tp.rx_desc ;
	tp.busy = 0;
	tp.status = COMPLETE;

	// Partition SRAM size
	raid_write_reg(RAID_PCR, SRAM_PAR_SIZE,0x00000003);

	// config tx DMA controler
	txdma_ctrl.bits32 = 0;
	txdma_ctrl.bits.td_start = 0;
	txdma_ctrl.bits.td_continue = 1;
	txdma_ctrl.bits.td_chain_mode = 1;
	txdma_ctrl.bits.td_prot = 0;
	txdma_ctrl.bits.td_burst_size = 1;
	txdma_ctrl.bits.td_bus = 3;
	txdma_ctrl.bits.td_endian = 0;
	txdma_ctrl.bits.td_finish_en = 1;
	txdma_ctrl.bits.td_fail_en = 1;
	txdma_ctrl.bits.td_perr_en = 1;
	txdma_ctrl.bits.td_eod_en = 0;	// enable tx descript
	txdma_ctrl.bits.td_eof_en = 0;
	raid_write_reg(RAID_FCHDMA_CTRL, txdma_ctrl.bits32,0xFFFFFFFF);

	// config rx DMA controler
	rxdma_ctrl.bits32 = 0;
	rxdma_ctrl.bits.rd_start = 0;
	rxdma_ctrl.bits.rd_continue = 1;
	rxdma_ctrl.bits.rd_chain_mode = 1;
	rxdma_ctrl.bits.rd_prot = 0;
	rxdma_ctrl.bits.rd_burst_size = 1;
	rxdma_ctrl.bits.rd_bus = 3;
	rxdma_ctrl.bits.rd_endian = 0;
	rxdma_ctrl.bits.rd_finish_en = 0;
	rxdma_ctrl.bits.rd_fail_en = 1;
	rxdma_ctrl.bits.rd_perr_en = 1;
	rxdma_ctrl.bits.rd_eod_en = 0;
	rxdma_ctrl.bits.rd_eof_en = 0;
	raid_write_reg(RAID_STRDMA_CTRL, rxdma_ctrl.bits32,0xFFFFFFFF);

	// enable interrupt
	dma_status.bits32 = 3;	// enable RpInt
	raid_write_reg(RAID_DMA_STATUS, dma_status.bits32,0xFFFFFFFF);

	return 0;
}

module_init(gemini_xor_init);
module_exit(gemini_xor_exit);

