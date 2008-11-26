/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.


********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the General
Public License Version 2, June 1991 (the "GPL License"), a copy of which is
available along with the File in the license.txt file or by writing to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or
on the worldwide web at http://www.gnu.org/licenses/gpl.txt.

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY
DISCLAIMED.  The GPL License provides additional details about this warranty
disclaimer.
*******************************************************************************/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/cdev.h>

#include "ctrlEnv/mvCtrlEnvLib.h"
#include "ctrlEnv/sys/mvSysTs.h"
#include "ts/mvTsu.h"
#include "mv_tsu_ioctl.h"
#include "mv_tsu.h"

/*
 * Local Macros and Definiions.
 */
#undef TSU_DEBUG
//#define TSU_DEBUG
#define TSU_DBG_OFF     0x00
#define TSU_DBG_INIT    0x01
#define TSU_DBG_OPEN    0x02
#define TSU_DBG_RELEASE 0x04
#define TSU_DBG_READ 	0x08
#define TSU_DBG_WRITE 	0x10
#define TSU_DBG_IOCTL 	0x20
#define TSU_DBG_PROC    0x40
#define TSU_DBG_INT     0x80
#define TSU_DBG_ALL     0xFFFFFFFF

#ifdef TSU_DEBUG
static u32 mvtsu_dbg = TSU_DBG_READ;
#define TSU_DPRINT(f, x) if((mvtsu_dbg & (f)) == (f)) printk x
#else
#define TSU_DPRINT(f, x)
#endif
#define TSU_ENTER(f, fname)	TSU_DPRINT(f, ("Entering " fname "()\n"))
#define TSU_LEAVE(f, fname)	TSU_DPRINT(f, ("Leaving " fname "()\n"))

#define TSU_UNCACHED_DATA_BUFFERS

#define TSU_DATA_BUFF_HW_2_SW(buf)      (((MV_U32)(buf)) & ~(TSU_DMA_ALIGN - 1))

#define TSU_IS_DEC_DIGIT(c)	(((c) >= '0') && ((c) <= '9'))

#define TSU_DEV_NAME "tsu"
#define TSU_NUM_DEVICES	2
#ifdef CONFIG_TSU_SERIAL_IF
#define DEF_TSU_MODE TSU_MODE_SERIAL
#else
#define DEF_TSU_MODE TSU_MODE_PARALLEL
#endif

#if defined(CONFIG_TSU_CORE_CLK_71MHZ)
#define DEF_TSU_CORE_CLOCK TSU_CORE_CLK_71_MHZ
#elif defined(CONFIG_TSU_CORE_CLK_83MHZ)
#define DEF_TSU_CORE_CLOCK TSU_CORE_CLK_83_MHZ
#elif defined(CONFIG_TSU_CORE_CLK_91MHZ)
#define DEF_TSU_CORE_CLOCK TSU_CORE_CLK_91_MHZ
#elif defined(CONFIG_TSU_CORE_CLK_100MHZ)
#define DEF_TSU_CORE_CLOCK TSU_CORE_CLK_100_MHZ
#endif

#define TSU_MIN_READ_SIZE	8

/*
 * Local data-structures.
 */
struct mvtsu_dev {
	u8 port;
	struct cdev cdev;
	MV_TSU_BUFF_INFO buff_info;
	MV_TSU_PORT_DIRECTION port_dir;
	u32 valid_data_size;	/* Size of data in HW buffers.		*/
	u32 rd_rw_data_size;	/* Size of buffer for read / write.	*/
	u8 *data_buff;
	u32 *stat_buff;
	u32 stat_buff_size;
	u32 data_offs;
	u32 buff_handle;
	u8 read_all_at_once;
	u32 clockrate;
	u32 rd_wr_timeout;
	spinlock_t lock;
	struct tsu_stat int_stat;
};


/*
 * Local control variables.
 */
static MV_TSU_PORTS_MODE cfg_tsu_mode;
static MV_TSU_CORE_CLOCK cfg_core_clk;
static int cfg_pkt_size;
static struct mvtsu_dev mvtsu_devs[TSU_NUM_DEVICES];
static dev_t mvtsu_device;
char *mvtsu_cmdline;

int mvtsu_cmdline_config(char *s);
static int mvtsu_parse_cmdline(void);
__setup("mv_tsu_config=", mvtsu_cmdline_config);

#ifdef CONFIG_MV_TSU_PROC
static struct proc_dir_entry *mvtsu_proc_entry;

static int mvtsu_proc_init(void);
#endif /* CONFIG_MV_TSU_PROC */

void mvtsu_dump_regs(int port)
{
	int i;

	for(i = 0x0; i <= 0x68; i+=4) {
		printk("Reg 0x%08x --> 0x%08x.\n",TSU_REG_BASE(port) + i,
		       MV_REG_READ(TSU_REG_BASE(port) + i));
	}
	return;
}


static void mvtsu_rd_wr_timeout_calc(struct mvtsu_dev *dev)
{
	dev->rd_wr_timeout = 0;
	if(dev->clockrate >= 1000) {
		/* How much time (in milisec) is needed for a single 	*/
		/* block of data.					*/
		printk("dev->valid_data_size - %d, dev->clockrate - %d.\n",
		       dev->valid_data_size,dev->clockrate);
		dev->rd_wr_timeout = ((dev->valid_data_size * 8) /
				      (dev->clockrate / 1000));
//		dev->rd_wr_timeout = ((dev->valid_data_size * 8) /
//				      (dev->clockrate / 1000) + 100);
	}
//	if(!dev->rd_wr_timeout)
//		dev->rd_wr_timeout = 500; /* 1/2 sec */
	/* Double the timeout to be on the safe side.	*/
	dev->rd_wr_timeout *= 2;
	printk("dev->rd_wr_timeout = %d.\n",dev->rd_wr_timeout);
	return;
}


static void mvtsu_set_defaults(struct mvtsu_dev *dev)
{
	TSU_ENTER(TSU_DBG_INIT, "mvtsu_set_defaults");

	dev->buff_info.aggrMode = TSU_DFLT_AGGR_MODE;
	dev->buff_info.aggrMode2TmstmpOff = TSU_DFLT_TMSTMP_OFFS;
	dev->buff_info.aggrNumPackets = TSU_DFLT_AGGR_PCKT_NUM;
	dev->buff_info.numTsDesc = TSU_DFLT_TS_DESC_NUM;
	dev->buff_info.numDoneQEntry = TSU_DFLT_TS_DONEQ_NUM;
	dev->buff_info.tsDataBuff = NULL;
	dev->buff_info.tsDoneBuff = NULL;
	dev->buff_info.dataBlockSize = 0;

	dev->read_all_at_once = TSU_DFLT_DATA_READ_AT_ONCE;
	dev->clockrate = TSU_DFLT_CLOCK_RATE;

	TSU_LEAVE(TSU_DBG_INIT, "mvtsu_set_defaults");
	return;
}

static irqreturn_t mvtsu_interrupt_handler(int irq , void *arg)
{
	struct mvtsu_dev* dev = (struct mvtsu_dev*)arg;
	u32 cause;

	TSU_ENTER(TSU_DBG_INT, "mvtsu_interrupt_handler");

	cause = MV_REG_READ(MV_TSU_INTERRUPT_SRC_REG(dev->port));
	TSU_DPRINT(TSU_DBG_INT, ("\tPort %d, Cause = 0x%08x.\n",dev->port,cause));

	if(cause & TSU_INT_TS_IF_ERROR)
		dev->int_stat.ts_if_err++;
	if(cause & TSU_INT_FIFO_OVFL_ERROR)
		dev->int_stat.fifo_ovfl++;
	if(cause & TSU_INT_TS_CONN_ERROR)
		dev->int_stat.ts_conn_err++;
	if(cause & TSU_INT_CLOCK_SYNC_EXP)
		dev->int_stat.clk_sync_exp++;
        
	TSU_LEAVE(TSU_DBG_INT, "mvtsu_interrupt_handler");
	return IRQ_HANDLED;
}

int g_tx_cnt = 0;
int mvtsu_open (struct inode *inode, struct file *filp)
{
	struct mvtsu_dev *dev;
	MV_TSU_PORT_CONFIG port_cfg;
	MV_TSU_BUFF_INFO *binfo = NULL;
	MV_STATUS status;
	int result = 0;
	int stat_size = 0;
	int data_size = 0;
	int data_buff_offs;

	TSU_ENTER(TSU_DBG_OPEN, "mvtsu_open");

	if(MV_FALSE == mvCtrlPwrClckGet(TS_UNIT_ID, 0)) {
		printk("Transport Stream interface is powered off.\n");
		return 0;
	}

	g_tx_cnt = 0;
	/*  Find the device structure.	*/
	dev = container_of(inode->i_cdev, struct mvtsu_dev, cdev);

    	/* Determine the port direction according to the read / write flag.*/
	if ((filp->f_mode & (FMODE_WRITE | FMODE_READ)) == FMODE_WRITE) {
		port_cfg.portDir = TSU_PORT_OUTPUT;
	} else if ((filp->f_mode & (FMODE_WRITE | FMODE_READ)) == FMODE_READ) {
		port_cfg.portDir = TSU_PORT_INPUT;
	} else {
		result = -EINVAL;
		goto fail_init;
	}

	/* Reset the port.		*/
	mvTsuPortReset(dev->port);
#if 0
#warning "TEMPORARY: Configure TSU to loopback mode.\n"
	MV_REG_WRITE(0xB803C,0x2);
	MV_REG_WRITE(0xB883C,0x2);
#endif /* 0 */
	/* Initialize the port.		*/
	port_cfg.pktSize = cfg_pkt_size;
       	status = mvTsuPortInit(dev->port,&port_cfg);
	if(status != MV_OK) {
		result = -EINVAL;
		goto fail_init;
	}

	TSU_DPRINT(TSU_DBG_OPEN, ("\tTSU unit initialized successfully.\n"));

	/* Initialize the port buffers.	*/
	binfo = &dev->buff_info;
	switch(binfo->aggrMode)
	{
	case (MV_TSU_AGGR_MODE_DISABLED):
		binfo->dataBlockSize = port_cfg.pktSize;
		dev->valid_data_size = port_cfg.pktSize;
		dev->rd_rw_data_size = binfo->dataBlockSize + TSU_DONE_STATUS_ENTRY_SIZE;
		binfo->aggrNumPackets = 1;
		break;
	case (MV_TSU_AGGR_MODE_1):
		binfo->dataBlockSize = port_cfg.pktSize * binfo->aggrNumPackets;
		if(port_cfg.portDir == TSU_PORT_OUTPUT) {
			binfo->dataBlockSize += MV_MAX(TSU_DMA_ALIGN,TSU_MODE1_OUT_TMS_SIZE);
			dev->rd_rw_data_size = binfo->dataBlockSize;
		}
		else {
			dev->rd_rw_data_size = binfo->dataBlockSize + 
				(binfo->aggrNumPackets * TSU_DONE_STATUS_ENTRY_SIZE);
		}
		dev->valid_data_size = port_cfg.pktSize * binfo->aggrNumPackets;
		break;
	case (MV_TSU_AGGR_MODE_2):
		binfo->aggrMode2TmstmpOff = TSU_DMA_ALIGN - 
			(port_cfg.pktSize & (TSU_DMA_ALIGN - 1)); 
		binfo->dataBlockSize =
			(binfo->aggrNumPackets * 
			 (port_cfg.pktSize + binfo->aggrMode2TmstmpOff) + 
			 TSU_DMA_ALIGN);
		dev->valid_data_size = (binfo->aggrNumPackets * 
					(port_cfg.pktSize + binfo->aggrMode2TmstmpOff));
		dev->rd_rw_data_size = dev->valid_data_size;
	default:
		break;
	}

	dev->port_dir = port_cfg.portDir;

	/* Align the data block size to a cache line.	*/
	binfo->dataBlockSize = MV_ALIGN_UP(binfo->dataBlockSize,32);
	data_size = binfo->dataBlockSize * binfo->numTsDesc;
	printk("aggrMode2TmstmpOff = %d, dataBlockSize = %d\n",
	       binfo->aggrMode2TmstmpOff,binfo->dataBlockSize);
#ifdef TSU_UNCACHED_DATA_BUFFERS
	binfo->tsDataBuff = 
		mvOsIoUncachedMalloc(NULL,data_size,(MV_ULONG*)(&binfo->tsDataBuffPhys),
				     NULL);
#else 
	binfo->tsDataBuff = 
		mvOsIoCachedMalloc(NULL,data_size,(MV_ULONG*)(&binfo->tsDataBuffPhys),
				   NULL);
#endif /* TSU_UNCACHED_DATA_BUFFERS */
	if(binfo->tsDataBuff == NULL) {
		result = -ENOMEM;
		goto fail_init;
	}
	memset(binfo->tsDataBuff,0x88,data_size);
#ifndef TSU_UNCACHED_DATA_BUFFERS
	mvOsCacheClear(NULL,(MV_U32*)TSU_DATA_BUFF_HW_2_SW(binfo->tsDataBuff),
		       data_size);
#endif /* TSU_UNCACHED_DATA_BUFFERS */

	/* Align tsDataBuff according to the HW limitation.	*/
	if(binfo->aggrMode == MV_TSU_AGGR_MODE_2) {
		data_buff_offs = port_cfg.pktSize & (TSU_DMA_ALIGN - 1);
	}
	else if((binfo->aggrMode == MV_TSU_AGGR_MODE_1) &&
		(port_cfg.portDir == TSU_PORT_OUTPUT)) {
        	data_buff_offs = TSU_DMA_ALIGN - TSU_MODE1_OUT_TMS_SIZE;
	}
	else {
		data_buff_offs = 0;
	}
	printk("data_buff_offs = %d.\n",data_buff_offs);

	binfo->tsDataBuff = (MV_U32*)((MV_U32)binfo->tsDataBuff + data_buff_offs);
	binfo->tsDataBuffPhys += data_buff_offs;

	TSU_DPRINT(TSU_DBG_OPEN, ("\tTSU Data buffer allocated successfully "
				  "(%p, %d).\n",binfo->tsDataBuff, data_size));
	/* Allocate memory for done queue.	*/
	stat_size = TSU_DONE_STATUS_ENTRY_SIZE * binfo->numDoneQEntry;
	dev->stat_buff_size = stat_size;
	binfo->tsDoneBuff =
		mvOsIoUncachedMalloc(NULL,stat_size,
				     (MV_ULONG*)(&binfo->tsDoneBuffPhys),NULL);
	if(binfo->tsDoneBuff == NULL) {
		result = -ENOMEM;
		goto fail_init;
	}

	TSU_DPRINT(TSU_DBG_OPEN, ("\tTSU Done buffer allocated successfully"
				  "(%p, %d).\n",binfo->tsDoneBuff, stat_size));

	status = mvTsuBuffersInit(dev->port,&dev->buff_info);
	if(status != MV_OK) {
		TSU_DPRINT(TSU_DBG_OPEN, ("\tmvTsuBuffersInit() Failed (%d).",
					  status));
		result = -EINVAL;
		goto fail_init;
	}
	TSU_DPRINT(TSU_DBG_OPEN, ("\tHAL Buffers initialized successfully.\n"));

	mvtsu_rd_wr_timeout_calc(dev);

	/* Register IRQ.	*/
	MV_REG_WRITE(MV_TSU_INTERRUPT_MASK_REG(dev->port),TSU_DFLT_INT_MASK);
	if(request_irq(IRQ_TS_INT(dev->port),mvtsu_interrupt_handler,
		       IRQF_DISABLED | IRQF_SAMPLE_RANDOM,"tsu",dev)) {
		printk(KERN_ERR "Cannot assign irq%d to TSU port%d\n",
		       IRQ_TS_INT(dev->port), dev->port);
		goto fail_init;
	}
	TSU_DPRINT(TSU_DBG_OPEN, ("\tTSU interrupt registered at IRQ %d.\n",
				  IRQ_TS_INT(dev->port)));

	if(port_cfg.portDir == TSU_PORT_INPUT) {
		/* Enable Rx timestamp.	*/
		mvTsuRxTimestampCntEn(dev->port,MV_TRUE);
		mvTsuDmaWatermarkSet(dev->port,0x8);
	}

	/* Make the private_data hold the pointer to the device data.	*/
	filp->private_data = dev;

	TSU_LEAVE(TSU_DBG_OPEN, "mvtsu_open");
	mvtsu_dump_regs(dev->port);
	return 0;

fail_init:
	if(binfo != NULL) {
		if(binfo->tsDataBuff != NULL)
#ifdef TSU_UNCACHED_DATA_BUFFERS
			mvOsIoUncachedFree(
				NULL,data_size,
				TSU_DATA_BUFF_HW_2_SW(binfo->tsDataBuffPhys),
				(MV_U32*)TSU_DATA_BUFF_HW_2_SW(binfo->tsDataBuff),0);
#else
		mvOsIoCachedFree(
			NULL,data_size,
			TSU_DATA_BUFF_HW_2_SW(binfo->tsDataBuffPhys),
			(MV_U32*)TSU_DATA_BUFF_HW_2_SW(binfo->tsDataBuff),0);
#endif /* TSU_UNCACHED_DATA_BUFFERS */
		if(binfo->tsDoneBuff != NULL)
			mvOsIoUncachedFree(NULL,stat_size,binfo->tsDoneBuffPhys,
					   binfo->tsDoneBuff,0);
		binfo->tsDataBuff = NULL;
		binfo->tsDoneBuff = NULL;
	}
	TSU_LEAVE(TSU_DBG_OPEN, "mvtsu_open");
	return result;
}


int mvtsu_release (struct inode *inode, struct file *filp)
{
	struct mvtsu_dev *dev = (struct mvtsu_dev*)filp->private_data;
	MV_TSU_BUFF_INFO *binfo;
	int size;

	TSU_ENTER(TSU_DBG_RELEASE, "mvtsu_release");

	free_irq(IRQ_TS_INT(dev->port),dev);

	if(dev->port_dir == TSU_PORT_INPUT) {
		/* Stop Rx timestamp.	*/
		mvTsuRxTimestampCntEn(dev->port,MV_FALSE);
	}

	/* Shutdown the port.		*/
	mvTsuPortShutdown(dev->port);

	/* Clear interrupt mask.	*/
	MV_REG_WRITE(MV_TSU_INTERRUPT_MASK_REG(dev->port),0);

	/* Free previously allocated buffers.	*/
	binfo = &dev->buff_info;
	if(binfo->tsDataBuff != NULL) {
		size = binfo->dataBlockSize * binfo->numTsDesc;
#ifdef TSU_UNCACHED_DATA_BUFFERS
		mvOsIoUncachedFree(NULL,size,
				   TSU_DATA_BUFF_HW_2_SW(binfo->tsDataBuffPhys),
				   (MV_U32*)TSU_DATA_BUFF_HW_2_SW(binfo->tsDataBuff),0);
#else
		mvOsIoCachedFree(NULL,size,
				 TSU_DATA_BUFF_HW_2_SW(binfo->tsDataBuffPhys),
				 (MV_U32*)TSU_DATA_BUFF_HW_2_SW(binfo->tsDataBuff),0);
#endif /* TSU_UNCACHED_DATA_BUFFERS */
	}
	if(binfo->tsDoneBuff != NULL) {
		mvOsIoUncachedFree(NULL,dev->stat_buff_size,binfo->tsDoneBuffPhys,
				   binfo->tsDoneBuff,0);
	}
	binfo->tsDataBuff = NULL;
	binfo->tsDoneBuff = NULL;

	mvtsu_set_defaults(dev);

	TSU_LEAVE(TSU_DBG_RELEASE, "mvtsu_release");
	return 0;
}


/*
 * Helper function for retrying read buffer requests.
 * Assume that the device spinlock is held.
 */
static int mvtsu_next_rx_buff_get(struct file *filp, u32 **data_buff,
				  u32 **stat_buff, u32 *buff_handle,
				  unsigned long *flags)
{
	struct mvtsu_dev *dev = (struct mvtsu_dev*)filp->private_data;
	int timeout = 0;
	int cnt = 0;
	MV_STATUS status;

	timeout = (dev->rd_wr_timeout == 0) ? 2000000 : (dev->rd_wr_timeout + 100);

	while(cnt < timeout) {
		status = mvTsuRxNextBuffGet(dev->port,data_buff,stat_buff,
					    buff_handle);
		if(status != MV_OK) {
			if(status != MV_NO_MORE)
				return -EIO;
		}
		else {
			break;
		}
		if (filp->f_flags & O_NONBLOCK)
			break;

		spin_unlock_irqrestore(&(dev->lock), *flags);
		if(dev->rd_wr_timeout) {
			msleep_interruptible(100);
			cnt += 100;
		} else {
			udelay(10);
			cnt += 10;
		}
		spin_lock_irqsave(&(dev->lock), *flags);
	}
	if(cnt >= timeout) {
		printk(KERN_INFO "TSU: Read timeout.\n");
		return -EAGAIN;
	}
	return 0;
}


/*
 * Helper function for copying timestamp info to user buffer.
 */
inline static void mvtsu_rx_tmsstmp_copy(struct mvtsu_dev *dev, u32 *stat_buff,
					 char *out_buff, int *buf_offs,
					 int *buf_size)
{
	MV_U32 size;
	MV_U32 all;
	MV_U32 avail;

	all = TSU_DONE_STATUS_ENTRY_SIZE * dev->buff_info.aggrNumPackets;
	/* Calculate avilable data at end of buffer.	*/
	avail = dev->stat_buff_size - 
		((MV_U32)stat_buff - (MV_U32)dev->buff_info.tsDoneBuff);
	if(avail < all)
		size = avail;
	else
		size = all;

	if(copy_to_user(out_buff,stat_buff,size))
		panic("TSU: copy_to_user failed.");
	avail = all - size;
	if(avail != 0) {
		if(copy_to_user(out_buff + size,dev->buff_info.tsDoneBuff,avail))
			panic("TSU: copy_to_user failed.");
	}

	*buf_offs += all;
	*buf_size -= all;
        return;
}

/*
 * TSU data read
 */
ssize_t mvtsu_read(struct file *filp, char __user *buf, size_t count,
		   loff_t *f_pos)
{
	struct mvtsu_dev *dev = filp->private_data;
	MV_U8 port = dev->port;
	MV_U8 *data_buff;
	MV_U32 size;
	int status;
	int buf_offs = 0;
	unsigned long flags;

	TSU_ENTER(TSU_DBG_READ, "mvtsu_read");
	spin_lock_irqsave(&(dev->lock), flags);

	if((dev->buff_info.aggrMode != MV_TSU_AGGR_MODE_2) &&
	   (count < (TSU_DONE_STATUS_ENTRY_SIZE * dev->buff_info.aggrNumPackets))) {
		printk(KERN_ERR "TSU: Read operations must be at least of size "
				"%d",
		       TSU_DONE_STATUS_ENTRY_SIZE * dev->buff_info.aggrNumPackets);
		status = -EINVAL;
		goto no_data;
	}

	if(dev->data_buff == NULL) {
		TSU_DPRINT(TSU_DBG_READ, ("\tGet new data buffer..."));
		status = mvtsu_next_rx_buff_get(filp,(MV_U32**)(&dev->data_buff),
						&dev->stat_buff,
						&dev->buff_handle,&flags);
		if(status) {
			TSU_DPRINT(TSU_DBG_READ, ("FAILED.\n"));
			goto no_data;
		}

		TSU_DPRINT(TSU_DBG_READ, ("OK.\n"));
		if(dev->buff_info.aggrMode != MV_TSU_AGGR_MODE_2)
			/* Copy the timestamp before the packet's data.	*/
			mvtsu_rx_tmsstmp_copy(dev,dev->stat_buff,buf,&buf_offs,
					      &count);
		dev->data_offs = 0;
	}
	data_buff = dev->data_buff;
	size = dev->rd_rw_data_size - dev->data_offs;

	if(dev->read_all_at_once && (size > count)) {
		status = -EINVAL;
		printk(KERN_INFO "TSU: Read buffer too small, "
				 "(Read All At Once mode)");
		goto no_data;
	}

        /* Valid data in data_buff.	*/
	if(size < count)
		count = size;
	TSU_DPRINT(TSU_DBG_READ, ("\tCopy %d Bytes (%d, %p, %d).\n", count,
				  buf_offs,data_buff,dev->data_offs));
	if(copy_to_user(buf + buf_offs,data_buff + dev->data_offs,count))
		panic("TSU: copy_to_user failed.");

	dev->data_offs += count;
	*f_pos += (count + buf_offs);

	if(dev->data_offs == dev->valid_data_size) {
		TSU_DPRINT(TSU_DBG_READ, ("\tFree RX buffer.\n"));
		memset(dev->data_buff,0x88,dev->buff_info.dataBlockSize);
#ifndef TSU_UNCACHED_DATA_BUFFERS
		mvOsCacheClear(NULL,
			       (MV_U32*)TSU_DATA_BUFF_HW_2_SW(dev->data_buff),
			       dev->buff_info.dataBlockSize);
#endif /* TSU_UNCACHED_DATA_BUFFERS */
		status = mvTsuRxBuffFree(port,(MV_U32*)dev->data_buff,
					 dev->stat_buff,dev->buff_handle);
		if(status != MV_OK) {
			panic("TSU: Error in state machine, mvTsuRxBuffFree() "
			      "Failed.");
		}
		dev->data_buff = NULL;
		dev->stat_buff = NULL;
	}

	spin_unlock_irqrestore(&(dev->lock), flags);
        TSU_LEAVE(TSU_DBG_READ, "mvtsu_read");
	return count + buf_offs;

no_data:
	spin_unlock_irqrestore(&(dev->lock), flags);
	printk(KERN_DEBUG "TSU: Failed to read.\n");
        TSU_LEAVE(TSU_DBG_READ, "mvtsu_read");
	return status;
}


/*
 * Helper function for retrying write buffer requests.
  * Assume that the device spinlock is held.
 */
static int mvtsu_next_tx_buff_get(struct file *filp, u32 **data_buff,
				  u32 *buff_handle, unsigned long *flags)
{
	struct mvtsu_dev *dev = (struct mvtsu_dev*)filp->private_data;
	int timeout = 0;
	int cnt = 0;
	MV_STATUS status;

	timeout = (dev->rd_wr_timeout == 0) ? 2000000 : (dev->rd_wr_timeout + 100);

	while(cnt < timeout) {
		status = mvTsuTxNextBuffGet(dev->port,data_buff,buff_handle);
		if(status != MV_OK) {
			if(status != MV_NO_MORE)
				return -EIO;
		}
		else {
			break;
		}

		if (filp->f_flags & O_NONBLOCK)
			break;
		spin_unlock_irqrestore(&(dev->lock), *flags);
		if(dev->rd_wr_timeout) {
			msleep_interruptible(100);
			cnt += 100;
		} else {
			udelay(10);
			cnt += 10;
		}
		spin_lock_irqsave(&(dev->lock), *flags);

		cnt += 1;
	}
	if(cnt >= timeout) {
		printk(KERN_INFO "TSU: Write timeout.\n");
		return -EAGAIN;
	}
	return 0;
}


/*
 * TSU data write
 */
ssize_t mvtsu_write (struct file *filp, const char __user *buf, size_t count,
		     loff_t *f_pos)
{
	struct mvtsu_dev *dev = filp->private_data;
	MV_U8 port = dev->port;
	int status;
	int buf_offs = 0;
	MV_BOOL tsErr = MV_FALSE;
	MV_U32 tms = 0;
	MV_U32 data;
	unsigned long flags;
	size_t orig_cnt = count;

	TSU_ENTER(TSU_DBG_WRITE, "mvtsu_write");
	spin_lock_irqsave(&(dev->lock), flags);

	if(count < dev->rd_rw_data_size) {
		printk(KERN_ERR "TSU: Write operations must hold at least a "
				"single data block of data(%d , %d)",
		       dev->rd_rw_data_size, count);
		status = -EINVAL;
		goto no_tx;
	}

	TSU_DPRINT(TSU_DBG_WRITE, ("\tGet new data buffer..."));
	status = mvtsu_next_tx_buff_get(filp,(MV_U32**)(&dev->data_buff),
					&dev->buff_handle,&flags);
	if(status) {
		TSU_DPRINT(TSU_DBG_WRITE, ("FAILED.\n"));
		goto no_tx;
	}

	TSU_DPRINT(TSU_DBG_WRITE, ("OK.\n"));

	if(dev->buff_info.aggrMode == MV_TSU_AGGR_MODE_DISABLED) {
		TSU_DPRINT(TSU_DBG_WRITE, ("\tGet timestamp info.\n"));
                /* Copy the timestamp from the beginning of data buffer. */
		if(copy_from_user(&data,buf,TSU_DONE_STATUS_ENTRY_SIZE)) 
			panic("TSU: copy_to_user failed.");

		tsErr = TSU_STATUS_ERROR_GET(data);
		tms = TSU_STATUS_TMSSTMP_GET(data);
		buf_offs = TSU_DONE_STATUS_ENTRY_SIZE;
		count -= TSU_DONE_STATUS_ENTRY_SIZE;
		TSU_DPRINT(TSU_DBG_WRITE, ("\tTimestamp = %d.\n",tms));
	}

	/* Valid data in data_buff.	*/
	if(dev->valid_data_size < count)
		count = dev->valid_data_size;

	TSU_DPRINT(TSU_DBG_WRITE, ("\tCopy %d Bytes.\n", count));
	if(g_tx_cnt < 65536)
		if(copy_from_user(dev->data_buff, buf + buf_offs, count))
			panic("TSU Write: copy_to_user failed.");
	*f_pos += orig_cnt;

	TSU_DPRINT(TSU_DBG_WRITE, ("\tFree TX buffer.\n"));
#ifndef TSU_UNCACHED_DATA_BUFFERS
	mvOsCacheFlush(NULL,(MV_U32*)TSU_DATA_BUFF_HW_2_SW(dev->data_buff),
		       dev->buff_info.dataBlockSize);
#endif /* TSU_UNCACHED_DATA_BUFFERS */

	status = mvTsuTxBuffPut(port,(MV_U32*)dev->data_buff,tms,tsErr,
				dev->buff_handle);
	g_tx_cnt++;
	if(status != MV_OK)
		panic("TSU: Error in state machine, mvTsuTxBuffPut() Failed.");

	spin_unlock_irqrestore(&(dev->lock), flags);
	TSU_LEAVE(TSU_DBG_WRITE, "mvtsu_write");
	return orig_cnt;

no_tx:
	spin_unlock_irqrestore(&(dev->lock), flags);
	printk(KERN_DEBUG "TSU: Failed to write.\n");
	TSU_LEAVE(TSU_DBG_WRITE, "mvtsu_write");
	return status;
}

/*
 * TSU ioctl()
 */
int mvtsu_ioctl (struct inode *inode, struct file *filp, unsigned int cmd,
		 unsigned long arg)
{
	struct mvtsu_dev *dev = filp->private_data;
	int ret = 0;
	u32 val;
	unsigned long flags;
	MV_STATUS status = MV_OK;
	struct tsu_tmstmp_info tms_info;
	struct tsu_buff_info buf_info;

	TSU_ENTER(TSU_DBG_IOCTL, "mvtsu_ioctl"); 
	TSU_DPRINT(TSU_DBG_IOCTL, ("\targ = 0x%08x.\n",(unsigned int)arg));

	spin_lock_irqsave(&(dev->lock), flags);
	
	switch(cmd) {
	case MVTSU_IOCFREQSET:
		get_user(val,(u32 __user *)arg);
		TSU_DPRINT(TSU_DBG_IOCTL, ("\tFrequency set to %d.\n",val));
		if(dev->port_dir == TSU_PORT_OUTPUT)
			status = mvTsuTxClockFreqSet(dev->port,val,MV_FALSE);
		if(status == MV_OK) {
			dev->clockrate = val;
			mvtsu_rd_wr_timeout_calc(dev);
		} else {
			ret = -EINVAL;
		}
		break;
	case MVTSU_IOCTXTMSSET:
		if(copy_from_user(&tms_info,(struct tsu_tmstmp_info*)arg,
				  sizeof(tms_info)))
			panic("TSU IOCTL: copy_from_user failed.\n");
		TSU_DPRINT(TSU_DBG_IOCTL, ("\tTx timestamp set to (%d,%d).\n",
					   tms_info.timestamp,
					   tms_info.enable_tms));
		status = mvTsuTxInitTimeStampSet(
			dev->port, (tms_info.enable_tms ? MV_TRUE : MV_FALSE),
			tms_info.timestamp);
		if(status != MV_OK)
			ret = -EINVAL;
		break;
	case MVTSU_IOCTXDONE:
		TSU_DPRINT(TSU_DBG_IOCTL, ("\tTx Done.\n"));
		if(mvTsuTxDone(dev->port) != MV_OK)
			ret = -EINVAL;
		break;
	case MVTSU_IOCRDPKTATONCE:
		TSU_DPRINT(TSU_DBG_IOCTL, ("\tRx Read Packet At Once.\n"));
		get_user(val,(u32 __user *)arg);
                dev->read_all_at_once = val;
		break;
	case MVTSU_IOCBUFFPARAMGET:
		TSU_DPRINT(TSU_DBG_IOCTL, ("\tGet Buffer Params.\n"));
		switch(dev->buff_info.aggrMode) {
		case MV_TSU_AGGR_MODE_DISABLED:
			buf_info.aggr_mode = aggrModeDisabled;
			break;
		case MV_TSU_AGGR_MODE_1:
			buf_info.aggr_mode = aggrMode1;
			break;
		case MV_TSU_AGGR_MODE_2:
			buf_info.aggr_mode = aggrMode2;
			break;
		default:
			ret = -EINVAL;
		}
		buf_info.aggr_mode2_tmstmp_off = 
			dev->buff_info.aggrMode2TmstmpOff;
		buf_info.aggr_num_packets = 
			dev->buff_info.aggrNumPackets;
		buf_info.num_done_q_entries = 
			dev->buff_info.numDoneQEntry;
		buf_info.num_ts_desc = 
			dev->buff_info.numTsDesc;
		buf_info.pkt_size = cfg_pkt_size;

		if(copy_to_user((struct tsu_buff_info*)arg,&buf_info,
				sizeof(buf_info)))
			panic("TSU IOCTL: copy_to_user failed.\n");
		break;
	case MVTSU_IOCGETSTAT:
		TSU_DPRINT(TSU_DBG_IOCTL, ("\tGet Statistics.\n"));
		if(copy_to_user((struct tsu_stat*)arg,&dev->int_stat,
				sizeof(struct tsu_stat)))
			panic("TSU IOCTL: copy_to_user failed.\n");
		break;
	case MVTSU_IOCCLEARSTAT:
		TSU_DPRINT(TSU_DBG_IOCTL, ("\tClear Statistics.\n"));
		memset(&(dev->int_stat),0,sizeof(dev->int_stat));
		break;
	default:
		TSU_DPRINT(TSU_DBG_IOCTL, ("\tInvalid request.\n"));
		ret = -EINVAL;
	}

	spin_unlock_irqrestore(&(dev->lock), flags);

	TSU_LEAVE(TSU_DBG_IOCTL, "mvtsu_ioctl"); 
	return ret;
}


#ifdef CONFIG_MV_TSU_PROC

/* 	
	Parameter		Port	Value		Possible Values

	sync_detect		x	x		int
	sync_loss		x	x		int
	aggr_mode		x	x		(mode1 / mode2 / dis)
	aggr_mode2_off		x	x		int
	aggr_num_pckts		x	x		int
	num_desc		x	x		int
	num_done_queue		x	x		int
	sync_sig		x	x		(dis / high / low)
	valid_sig		x	x		(dis / high / low)
	error_sig		x	x		(dis / high / low)
	data_edge		x	x		(fall / rise)
	data_order		x	x		(msb / lsb)
	sync_act		x	x		(1 / 8)
	clk_mode		x	x		(cont / gap)
*/

#define TSU_AGGR_MODE_2_STR(mode)					\
	((mode == MV_TSU_AGGR_MODE_DISABLED) ? "dis" : 			\
	 ((mode == MV_TSU_AGGR_MODE_1) ? "mode1" : "mode2"))

#define TSU_SIGNAL_MODE_2_STR(mode)					\
	((mode == TSU_SIGNAL_DIS) ? "dis" : 				\
	 ((mode == TSU_SIGNAL_EN_ACT_LOW) ? "low" : "high"))

#define TSU_STR_2_SIGNAL_MODE(mode,str)					\
	{								\
		if(!strncmp((str),"dis",3))				\
			mode = TSU_SIGNAL_DIS;				\
		else if(!strncmp((str),"low",3))			\
			mode = TSU_SIGNAL_EN_ACT_LOW;			\
		else if(!strncmp((str),"high",4))			\
			mode = TSU_SIGNAL_EN_ACT_HIGH;			\
		else							\
			mode = TSU_SIGNAL_KEEP_DEF;			\
	}



int mvtsu_proc_write(struct file *file, const char *buffer,unsigned long count,
		     void *data)
{
	MV_TSU_SIGNAL_CONFIG	signal_cfg;
	MV_U32 flags;
	MV_U8 sync_detect;
	MV_U8 sync_loss;
	MV_BOOL write_signal = MV_FALSE;
        MV_BOOL write_sync = MV_FALSE;
	MV_U8 port;
	int len = 0;
	int tmp;
	char *str;
	struct mvtsu_dev *dev;

	TSU_ENTER(TSU_DBG_PROC, "mvtsu_proc_write");

	len = sscanf(buffer,"%d ",&tmp);

	if(tmp >= MV_TSU_NUM_PORTS)
		return count;
	port = (MV_U8)tmp;
        len++; /* Skip over the space.	*/

	signal_cfg.tsDataEdge = TSU_SIGNAL_EDGE_FALL;
	signal_cfg.tsError = TSU_SIGNAL_KEEP_DEF;
	signal_cfg.tsSync = TSU_SIGNAL_KEEP_DEF;
	signal_cfg.tsValid = TSU_SIGNAL_KEEP_DEF;
	flags = 0;

	if(mvTsuRxSyncDetectionGet(port,&sync_detect,&sync_loss) != MV_OK)
		return count;
	dev = &mvtsu_devs[port];

	TSU_DPRINT(TSU_DBG_PROC, ("\tbuffer = %s.\n",buffer+len));
	str = "sync_detect ";
	if(!strncmp(buffer+len, str,strlen(str))) {
		len += strlen(str);
		len += sscanf(buffer+len,"%d",&tmp);
		sync_detect = (MV_U8)tmp;
		TSU_DPRINT(TSU_DBG_PROC, ("\tsync_detect = %d.\n",sync_detect));
		write_sync = MV_TRUE;
		goto done;
	} 
	str = "sync_loss ";
	if(!strncmp(buffer+len, str,strlen(str))) {
		len += strlen(str);
		len += sscanf(buffer+len,"%d",&tmp);
		sync_loss = (MV_U8)tmp;
		TSU_DPRINT(TSU_DBG_PROC, ("\tsync_loss = %d.\n",sync_loss));
		write_sync = MV_TRUE;
		goto done;
	}

	str = "aggr_mode ";
	if(!strncmp(buffer+len, str,strlen(str))) {
		len += strlen(str);
		str = "dis";
		if(!strncmp(buffer+len, str,strlen(str))) {
			len += strlen(str);
			dev->buff_info.aggrMode = MV_TSU_AGGR_MODE_DISABLED;
			goto done;
		}
		str = "mode1";
		if(!strncmp(buffer+len, str,strlen(str))) {
			len += strlen(str);
			dev->buff_info.aggrMode = MV_TSU_AGGR_MODE_1;
			goto done;
		}
		str = "mode2";
		if(!strncmp(buffer+len, str,strlen(str))) {
			len += strlen(str);
			dev->buff_info.aggrMode = MV_TSU_AGGR_MODE_2;
			goto done;
		}
		goto done;
	}

        str = "aggr_mode2_off ";
	if(!strncmp(buffer+len, str,strlen(str))) {
		len += strlen(str);
		len += sscanf(buffer+len,"%d",&tmp);
		dev->buff_info.aggrMode2TmstmpOff = tmp;
		goto done;
	}

	str = "aggr_num_pckts ";
	if(!strncmp(buffer+len, str,strlen(str))) {
		len += strlen(str);
		len += sscanf(buffer+len,"%d",&tmp);
		dev->buff_info.aggrNumPackets = tmp;
		goto done;
	}

	str = "num_desc ";
	if(!strncmp(buffer+len, str,strlen(str))) {
		len += strlen(str);
		len += sscanf(buffer+len,"%d",&tmp);
		dev->buff_info.numTsDesc = tmp;
		goto done;
	}

	str = "num_done_queue ";
	if(!strncmp(buffer+len, str,strlen(str))) {
		len += strlen(str);
		len += sscanf(buffer+len,"%d",&tmp);
		dev->buff_info.numDoneQEntry = tmp;
		goto done;
	}

	str = "sync_sig ";
	if(!strncmp(buffer+len, str,strlen(str))) {
		len += strlen(str);
		TSU_STR_2_SIGNAL_MODE(signal_cfg.tsSync,buffer+len);
                write_signal = MV_TRUE;
		goto done;
	}

	str = "valid_sig ";
	if(!strncmp(buffer+len, str,strlen(str))) {
		len += strlen(str);
		TSU_STR_2_SIGNAL_MODE(signal_cfg.tsValid,buffer+len);
		write_signal = MV_TRUE;
		goto done;
	}

	str = "error_sig ";
	if(!strncmp(buffer+len, str,strlen(str))) {
		len += strlen(str);
		TSU_STR_2_SIGNAL_MODE(signal_cfg.tsError,buffer+len);
		write_signal = MV_TRUE;
		goto done;
	}

	str = "data_edge ";
	if(!strncmp(buffer+len, str,strlen(str))) {
		len += strlen(str);
		str = "fall";
		if(!strncmp(buffer+len, str,strlen(str))) {
			len += strlen(str);
			signal_cfg.tsDataEdge = TSU_SIGNAL_EDGE_FALL;
			write_signal = MV_TRUE;
			goto done;
		}
		str = "rise";
		if(!strncmp(buffer+len, str,strlen(str))) {
			len += strlen(str);
			signal_cfg.tsDataEdge = TSU_SIGNAL_EDGE_RISE;
			write_signal = MV_TRUE;
			goto done;
		}
		goto done;
	}

	str = "data_order ";
	if(!strncmp(buffer+len, str,strlen(str))) {
		len += strlen(str);
		str = "msb";
		if(!strncmp(buffer+len, str,strlen(str))) {
			len += strlen(str);
			flags |= MV_TSU_SER_DATA_ORDER_MSB;
			write_signal = MV_TRUE;
			goto done;
		}
		str = "lsb";
		if(!strncmp(buffer+len, str,strlen(str))) {
			len += strlen(str);
			flags |= MV_TSU_SER_DATA_ORDER_LSB;
			write_signal = MV_TRUE;
			goto done;
		}
		goto done;
	}

	str = "sync_act ";
	if(!strncmp(buffer+len, str,strlen(str))) {
		len += strlen(str);
		len += sscanf(buffer+len,"%d",&tmp);
		write_signal = MV_TRUE;
		if(tmp == 1)
			flags |= MV_TSU_SER_SYNC_ACT_1_BIT;
		else if(tmp == 8)
			flags |= MV_TSU_SER_SYNC_ACT_8_BIT;
		else
			write_signal = MV_FALSE;
		goto done;
	}

	str = "clk_mode ";
	if(!strncmp(buffer+len, str,strlen(str))) {
		len += strlen(str);
		str = "cont";
		if(!strncmp(buffer+len, str,strlen(str))) {
			len += strlen(str);
			flags |= MV_TSU_SER_TX_CLK_MODE_CONT;
			write_signal = MV_TRUE;
			goto done;
		}
		str = "gap";
		if(!strncmp(buffer+len, str,strlen(str))) {
			len += strlen(str);
			flags |= MV_TSU_SER_TX_CLK_MODE_GAPPED;
			write_signal = MV_TRUE;
			goto done;
		}
		goto done;
	}

done:
	if(write_signal == MV_TRUE)
		mvTsuPortSignalCfgSet(port,&signal_cfg,flags);
	if(write_sync == MV_TRUE)
		mvTsuRxSyncDetectionSet(port,sync_detect,sync_loss);

	TSU_LEAVE(TSU_DBG_PROC, "mvtsu_proc_write");
	return count;
}


int mvtsu_proc_read_port(int port, char *buf)
{
	MV_U8	sync_detect, sync_loss;
	MV_TSU_SIGNAL_CONFIG signal_cfg;
	MV_U32 flags;
	int len=0;

	if(mvTsuRxSyncDetectionGet(port,&sync_detect,&sync_loss) != MV_OK)
		return -1;
	len += sprintf(buf,"\tsync_detect\t\t%d\t%d\tint\n",port,sync_detect);
	len += sprintf(buf+len,"\tsync_loss\t\t%d\t%d\tint\n",port,sync_loss);

	len += sprintf(buf+len,"\taggr_mode\t\t%d\t%s\t(mode1 / mode2 / dis)\n",
		       port,
		       TSU_AGGR_MODE_2_STR(mvtsu_devs[port].buff_info.aggrMode));
	len += sprintf(buf+len,"\taggr_mode2_off\t\t%d\t%d\tint\n",
		       port,mvtsu_devs[port].buff_info.aggrMode2TmstmpOff);
	len += sprintf(buf+len,"\taggr_num_pckts\t\t%d\t%d\tint\n",
		       port,mvtsu_devs[port].buff_info.aggrNumPackets);
	len += sprintf(buf+len,"\tnum_desc\t\t%d\t%d\tint\n",
		       port,mvtsu_devs[port].buff_info.numTsDesc);
	len += sprintf(buf+len,"\tnum_done_queue\t\t%d\t%d\tint\n",
		       port,mvtsu_devs[port].buff_info.numDoneQEntry);

	if(mvTsuPortSignalCfgGet(port,&signal_cfg,&flags) != MV_OK)
		return -1;
	len += sprintf(buf+len,"\tsync_sig\t\t%d\t%s\t(dis / high / low)\n",
		       port,TSU_SIGNAL_MODE_2_STR(signal_cfg.tsSync));
	len += sprintf(buf+len,"\tvalid_sig\t\t%d\t%s\t(dis / high / low)\n",
		       port,TSU_SIGNAL_MODE_2_STR(signal_cfg.tsValid));
	len += sprintf(buf+len,"\terror_sig\t\t%d\t%s\t(dis / high / low)\n",
		       port,TSU_SIGNAL_MODE_2_STR(signal_cfg.tsError));
	len += sprintf(buf+len,"\tdata_edge\t\t%d\t%s\t(fall / rise)\n",
		       port,(signal_cfg.tsDataEdge == TSU_SIGNAL_EDGE_FALL) ?
		       "fall" : "rise");

	if(cfg_tsu_mode == TSU_MODE_SERIAL) {
		len += sprintf(buf+len,"\tdata_order\t\t%d\t%s\t(msb / lsb)\n",
			       port,(flags & MV_TSU_SER_DATA_ORDER_MSB) ? "msb" : "lsb");
		len += sprintf(buf+len,"\tsync_act\t\t%d\t%d\t(1 / 8)\n",
			       port,(flags & MV_TSU_SER_SYNC_ACT_1_BIT) ? 1 : 8);
		len += sprintf(buf+len,"\tclk_mode\t\t%d\t%s\t(cont / gap)\n",
			       port,
			       (flags & MV_TSU_SER_TX_CLK_MODE_CONT) ? "cont" : "gap");
	}
	return len;
}


int mvtsu_proc_read(char* page, char** start, off_t off, int count,int* eof,
		    void* data)
{
	int len = 0;
	int i;
	int res;

	if(off > 0)
		return 0;
	len += sprintf(page,"\tParameter\t\tPort\tValue\tPossible Values\n\n");

	for(i = 0; i < MV_TSU_NUM_PORTS; i++) {
                res = mvtsu_proc_read_port(i,page+len);
		if(res < 0) {
			len = -1;
			break;
		}
		len += res;
	}
	if(len != -1)
		len += sprintf(page+len,"\nConfig String: <port> <attr> <val>\n");

	return len;
}


/*
 * Create TSU proc entry.
 */
static int mvtsu_proc_init(void)
{
	mvtsu_proc_entry = create_proc_entry("tsu", 0666, &proc_root);
	mvtsu_proc_entry->read_proc = mvtsu_proc_read;
	mvtsu_proc_entry->write_proc = mvtsu_proc_write;
	mvtsu_proc_entry->owner = THIS_MODULE;
	return 0;
}

#endif /* CONFIG_MV_TSU_PROC */


/*
 * Parse the TSU command line.
 * The command line looks as follows:
 * mv_tsu_config=<interface mode>,<packet size>,<core clock in MHz>
 * e.g. serial,188,73
 */
int mvtsu_cmdline_config(char *s)
{
    mvtsu_cmdline = s;
    return 1;
}

static int mvtsu_parse_core_clk(char **cmdline)
{
	char *str = *cmdline;
	int num = 0;

	while(*str != '\0') {
		if(!TSU_IS_DEC_DIGIT(*str))
			return -1;
		num = (num * 10) + (*str - '0');
		str++;
	}

	switch (num) {
	case(71):
		cfg_core_clk = TSU_CORE_CLK_71_MHZ;
                break;
	case(83):
		cfg_core_clk = TSU_CORE_CLK_83_MHZ;
		break;
	case(91):
		cfg_core_clk = TSU_CORE_CLK_91_MHZ;
		break;
	case(100):
		cfg_core_clk = TSU_CORE_CLK_100_MHZ;
		break;
	default:
		return -1;
	}

	*cmdline = str;
	return 0;
}


static int mvtsu_parse_pkt_size(char **cmdline)
{
	char *str = *cmdline;
	int num = 0;

	while((*str != '\0') && (*str != ',')) {
		if(!TSU_IS_DEC_DIGIT(*str))
			return -1;
		num = (num * 10) + (*str - '0');
		str++;
	}
	if((*str == ',') && (num != 0)) {
		cfg_pkt_size = num;
		*cmdline = str;
		return 0;
	}
	return -1;
}


static int mvtsu_parse_mode(char **cmdline)
{
	char *str;

	str = "serial";
	if(!strncmp(*cmdline,str,strlen(str))) {
		cfg_tsu_mode = TSU_MODE_SERIAL;
		*cmdline += strlen(str);
	} else {
                str = "parallel";
		if(!strncmp(*cmdline,str,strlen(str))) {
			cfg_tsu_mode = TSU_MODE_PARALLEL;
			*cmdline += strlen(str);
			return 0;
		}
	}
	return 0;
}


static int mvtsu_parse_cmdline(void)
{
	char *cmdline;

	if(mvtsu_cmdline) {
		TSU_DPRINT(TSU_DBG_INIT, ("TSU command line: %s.\n",mvtsu_cmdline));
		cmdline = mvtsu_cmdline;
		if(mvtsu_parse_mode(&cmdline) != 0) {
			printk(KERN_ERR "TSU: Bad interface mode option in command line, using default.\n");
			goto set_default;
		}
                if(cmdline[0] != ',') {
			printk(KERN_ERR "TSU: Bad command line format (Expected ',' found %c).\n",
			       cmdline[0]);
		}
		cmdline++;
		if(mvtsu_parse_pkt_size(&cmdline) != 0) {
			printk(KERN_ERR "TSU: Bad packet size option in command line, using default.\n");
			goto set_default;
		}
		if(cmdline[0] != ',') {
			printk(KERN_ERR "TSU: Bad command line format (Expected ',' found %c).\n",
			       cmdline[0]);
		}
		cmdline++;
		if(mvtsu_parse_core_clk(&cmdline) != 0) {
			printk(KERN_ERR "TSU: Bad core-clock option in command line "
					"(Expected 71 / 83 / 91 / 100), using default.\n");
			goto set_default;
		}
		goto success;
	} else {
		printk(KERN_INFO "TSU: No command line parameters, using default.\n");
	}

set_default:
	cfg_tsu_mode = DEF_TSU_MODE;
	cfg_core_clk = DEF_TSU_CORE_CLOCK;
	cfg_pkt_size = CONFIG_MV_TSU_PKT_SIZE;
success:
	return 0;
}


struct file_operations mvtsu_fops = {
	.owner =     THIS_MODULE,
	.read =	     mvtsu_read,
	.write =     mvtsu_write,
	.ioctl =     mvtsu_ioctl,
	.open =	     mvtsu_open,
	.release =   mvtsu_release,
};

/*
 * Initialize the TSU driver
 */
int mvtsu_init(void)
{
	int result, i;
	dev_t dev;
	MV_STATUS status;

        TSU_ENTER(TSU_DBG_INIT, "mvtsu_init");

#if 0
#warning "TEMPORARY: Configure MPPs for TSU.\n"
	{
	MV_U32 reg;
	reg = MV_REG_READ(0x10008);
	reg &= 0xFFFF;
	reg |= 0x11110000;
	MV_REG_WRITE(0x10008,reg);
	reg = 0x11111111;
	MV_REG_WRITE(0x1000C,reg);
	reg = MV_REG_READ(0x10010);
	reg &= ~0xF;
	reg |= 0x1;
	MV_REG_WRITE(0x10010,reg);
	}
#else
	/* Check unit power mode.		*/
	if(mvCtrlPwrClckGet(TS_UNIT_ID,0) == MV_FALSE) {
		printk("Warning: TS unit is powered off.\n");
		TSU_LEAVE(TSU_DBG_INIT, "mvtsu_init");
		return 0;
	}
#endif
	/* Parse command line parameters.	*/
	mvtsu_parse_cmdline();
#if 0
	result = alloc_chrdev_region(&dev, 0, TSU_NUM_DEVICES, TSU_DEV_NAME);
	mvtsu_device = MAJOR(dev);
#else
	dev = MKDEV(MV_TSU_MAJOR, 0);
	result = register_chrdev_region(dev, TSU_NUM_DEVICES, TSU_DEV_NAME);
#endif /* 0 */
	if (result < 0) {
		printk("Failed to register char device (%d,%d)\n",result, TSU_NUM_DEVICES);
		TSU_LEAVE(TSU_DBG_INIT, "mvtsu_init");
		return result;
	}

	/* Perform unit initialization.	*/
	result = mvTsuInit(cfg_core_clk, cfg_tsu_mode ,NULL);
	if(result != MV_OK) {
		goto fail_init;
		result = -EINVAL;
	}
	TSU_DPRINT(TSU_DBG_INIT, ("\tTSU unit initialized successfully.\n"));

	/* Initialize TSU windows.	*/
	status = mvTsuWinInit();
	if(status != MV_OK) {
		goto fail_init;
		result = -EINVAL;
	}
	TSU_DPRINT(TSU_DBG_INIT, ("\tTSU windows initialized successfully.\n"));

	/* Create the char device.	*/
	for (i = 0; i < TSU_NUM_DEVICES; i++) {
		mvtsu_set_defaults(&mvtsu_devs[i]);
		cdev_init(&mvtsu_devs[i].cdev, &mvtsu_fops);
		mvtsu_devs[i].cdev.owner = THIS_MODULE;
		mvtsu_devs[i].cdev.ops = &mvtsu_fops;
		mvtsu_devs[i].port = i;
		dev = MKDEV(MV_TSU_MAJOR, i);
		result = cdev_add (&mvtsu_devs[i].cdev, dev, 1);
		if (result) {
			printk(KERN_ERR "Error %d adding tsu%d", result, i);
			goto fail_add;
		}
		spin_lock_init(&mvtsu_devs[i].lock);
		TSU_DPRINT(TSU_DBG_INIT, ("\tChar device %d initialized.\n",i));
	}

#ifdef CONFIG_MV_TSU_PROC
	TSU_DPRINT(TSU_DBG_INIT, ("\tCreating Proc entry.\n"));
	mvtsu_proc_init();
#endif /* CONFIG_MV_TSU_PROC */

	printk("Transport Stream interface registered.\n");
	printk("  o %s Mode.\n",
	       (cfg_tsu_mode == TSU_MODE_PARALLEL) ? "Paralle" : "Serial");
	printk("  o Core-Clock - ");
	switch (cfg_core_clk) {
	case (TSU_CORE_CLK_83_MHZ):
		printk("83");
		break;
	case (TSU_CORE_CLK_71_MHZ):
		printk("71");
		break;
	case (TSU_CORE_CLK_91_MHZ):
		printk("91");
		break;
	case (TSU_CORE_CLK_100_MHZ):
		printk("100");
		break;
	}
	printk(" MHz.\n");
	printk("  o Packet Size - %d Bytes.\n",cfg_pkt_size);
	TSU_LEAVE(TSU_DBG_INIT, "mvtsu_init");
	return 0;

fail_add:
	while(i > 0) {
		cdev_del(&mvtsu_devs[i-1].cdev);
		i--;
	}

fail_init:
	dev = MKDEV(MV_TSU_MAJOR, 0);
	unregister_chrdev_region(dev, TSU_NUM_DEVICES);

	TSU_LEAVE(TSU_DBG_INIT, "mvtsu_init");
	return result;
}


/*
 * Cleanup the TSU driver.
 */
void mvtsu_cleanup(void)
{
	int i;

	TSU_ENTER(TSU_DBG_INIT, "mvtsu_cleanup");
	for(i = 0; i < TSU_NUM_DEVICES; i++) {
		cdev_del(&mvtsu_devs[i].cdev);
	}

	unregister_chrdev_region(mvtsu_device, TSU_NUM_DEVICES);

	mvTsuShutdown();

	TSU_LEAVE(TSU_DBG_INIT, "mvtsu_cleanup");
}

module_init(mvtsu_init);
module_exit(mvtsu_cleanup);
MODULE_LICENSE("GPL");

