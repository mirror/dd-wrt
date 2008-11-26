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
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

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
********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File under the following licensing terms. 
Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    *   Redistributions of source code must retain the above copyright notice,
	    this list of conditions and the following disclaimer. 

    *   Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution. 

    *   Neither the name of Marvell nor the names of its contributors may be 
        used to endorse or promote products derived from this software without 
        specific prior written permission. 
    
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

******************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/telephony.h>
#include <linux/phonedev.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include "proslic.h"

#include "dbg-trace.h"
extern MV_U8 currRxSampleGet(MV_U8 ch);
extern MV_U8 currTxSampleGet(MV_U8 ch);

#ifdef CONFIG_MV_GTW_QOS
extern int mv_gtw_qos_tos_enable(void);
extern int mv_gtw_qos_tos_disable(void);
#endif

#ifdef MV_88F5181L
#include "mvTdm.h"
#define MP_IRQ	13
#else
#include "mvTdmFpga.h"
#define MP_IRQ	(32+3)
#endif


/****************
* DEBUG CONTROL *
****************/
/* Extra IOCTLs for debug (used by mv_voip_tool) */
#define MV_IOCTL
#ifdef MV_IOCTL
#define PHONE_MV_READ_SLIC_REG	_IOWR ('q', 0xB0, unsigned char)
#define PHONE_MV_WRITE_SLIC_REG	_IOW ('q', 0xB1, unsigned int)
#define PHONE_MV_READ_REG	_IOWR ('q', 0xB2, unsigned int)
#define PHONE_MV_WRITE_REG	_IOW ('q', 0xB3, unsigned int)
#define PHONE_MV_SPI_TEST	_IO ('q', 0xB4)
#endif

/***********************
* PHONE DRIVER'S STUFF *
***********************/
typedef struct {
	struct phone_device p;
	unsigned int ch_info;
	unsigned char ch;
	unsigned int irq;
	int readers, writers;
        wait_queue_head_t poll_q;
} MV_PHONE;


static MV_PHONE mv_phone[MV_TDM_MAX_CHANNELS];
#define get_mp(ch) (&mv_phone[ch])

/* Forward declarations */
static int __init mp_init(void);
static void __exit mp_exit(void);
static int mp_open(struct phone_device *pdev, struct file *file_p);
static int mp_close(struct inode *inode, struct file *file_p);
static ssize_t mp_read(struct file * file_p, char __user *buf, size_t length, loff_t * ppos);
static ssize_t mp_write(struct file *file_p, const char __user *buf, size_t count, loff_t * ppos);
static unsigned int mp_poll(struct file *file_p, poll_table * wait);
static int mp_ioctl(struct inode *inode, struct file *file_p, unsigned int cmd, unsigned long arg);
static irqreturn_t mp_int_handler(int irq, void *dev_id, struct pt_regs *regs);
#ifdef MV_IOCTL
extern MV_STATUS mvTdmSpiRead(MV_U8 addr, MV_U8 *data);
extern MV_STATUS mvTdmSpiWrite(MV_U8 addr, MV_U8 data);
extern MV_STATUS mvTdmChSpiTest(MV_U32 loop);
#endif

/* Module stuff */
module_init(mp_init);
module_exit(mp_exit);
MODULE_DESCRIPTION("Marvell Telephony Device Driver - www.marvell.com");
MODULE_AUTHOR("Tzachi Perelstein <tzachi@marvell.com>");
MODULE_LICENSE("GPL");

/* Driver's operations */
static struct file_operations mp_fops =
{
        .owner          = THIS_MODULE,
        .read           = mp_read,
        .write          = mp_write,
        .poll           = mp_poll,
        .ioctl          = mp_ioctl,
        .release        = mp_close,
};

static int __init mp_init(void)
{
	MV_PHONE *mp;
	int i;

	printk("Marvell Telephony Driver:\n");

	TRC_INIT(currRxSampleGet, currTxSampleGet, 0, 0);
	TRC_REC("->%s\n",__FUNCTION__);

	/* General TDM and SLIC init */
	TRC_REC("tdm init\n");
	mvTdmInit();
	mvTdmShowProperties();

	/* per channel init */
	for(i=0; i<MV_TDM_MAX_CHANNELS; i++) {
		printk("Initializing channel %d\n",i);
		TRC_REC("ch%d init\n",i);
		mp = get_mp(i);
		mp->p.board = mp->ch = i;
		mp->p.f_op = &mp_fops;
		mp->p.open = mp_open;
		mp->irq = MP_IRQ;
        	init_waitqueue_head(&mp->poll_q);
		if(mvTdmChInit(&mp->p, i, &(mp->ch_info)) == MV_OK) {
			/*mvTdmChShowProperties(mp->ch_info);*/
			phone_register_device(&mp->p, PHONE_UNIT_ANY);
			printk("phone%d registered\n",i);
		}
		else {
			printk("%s: error, failed to init ch%d\n",__FUNCTION__,i);
		}
	}

	TRC_REC("<-%s\n",__FUNCTION__);
	return 0;
}

static void __exit mp_exit(void)
{
	MV_PHONE *mp;
	int i;

	TRC_REC("->%s\n",__FUNCTION__);

	for(i=0; i<MV_TDM_MAX_CHANNELS; i++) {
		TRC_REC("ch%d remove\n",i);
		mp = get_mp(i);
		mvTdmChRemove(mp->ch_info);
		mp->irq = 0;
		mp->p.f_op = NULL;
		mp->p.open = NULL;
		mp->p.board = 0;
		phone_unregister_device(&mp->p);
		printk("phone%d removed\n",i);
	}

	TRC_REC("<-%s\n",__FUNCTION__);
	TRC_OUTPUT();
	TRC_RELEASE();
}

static int mp_open(struct phone_device *p, struct file *file_p)
{
	MV_PHONE *mp = get_mp(p->board);
	file_p->private_data = mp;

	printk("Opening phone channel %d - \n",mp->ch);
	TRC_REC("->%s ch%d\n",__FUNCTION__,mp->ch);

        if (file_p->f_mode & FMODE_READ) {
		if(!mp->readers) {
	                mp->readers++;
        	}
		else {
			printk("device is busy (read)\n");
                	return -EBUSY;
		}
        }

	if (file_p->f_mode & FMODE_WRITE) {
		if(!mp->writers) {
			mp->writers++;
		} 
		else {
			if (file_p->f_mode & FMODE_READ) {
				mp->readers--;
			}
			printk("device is busy (write)\n");
			return -EBUSY;
		}
	}

	if(request_irq(mp->irq, mp_int_handler,	SA_SHIRQ | SA_INTERRUPT, "MP", mp ) ) {
		printk("Failed to connect IRQ %d\n", mp->irq);
		mp->irq = 0;
		return -EIO;
	}

	mvTdmChStart(mp->ch_info);

	TRC_REC("<-%s\n",__FUNCTION__);
	return 0;
}

static int mp_close(struct inode *inode, struct file *file_p)
{
	MV_PHONE *mp = file_p->private_data;

	printk("Closing Marvell phone%d device\n",mp->ch);
	TRC_REC("->%s ch%d\n",__FUNCTION__,mp->ch);

	mvTdmChStop(mp->ch_info);

	free_irq(mp->irq, mp);

	if (file_p->f_mode & FMODE_READ)
		mp->readers--;
	if (file_p->f_mode & FMODE_WRITE)
		mp->writers--;

	file_p->private_data = NULL;

	TRC_REC("<-%s\n",__FUNCTION__);
	TRC_OUTPUT();
	return 0;
}

static ssize_t mp_read(struct file * file_p, char __user *buf, size_t length, loff_t * ppos)
{
	MV_PHONE * mp = file_p->private_data;
	MV_STATUS status;
	unsigned long count = 0;
	unsigned int err = 0;

	TRC_REC("->%s ch%d\n",__FUNCTION__,mp->ch);

	count = min(length, (size_t)MV_TDM_BUFF_SIZE);
	status = mvTdmChRx(mp->ch_info, buf, count);

	if(status == MV_NOT_READY) { 
		TRC_REC("read not ready, try again\n");
		err = -EAGAIN;
	}
	else if(status == MV_FAIL) {
		TRC_REC("copy to user failed\n");
		printk("%s: copy to user failed\n", __FUNCTION__);
		err = -EFAULT;
	}
	else {
		TRC_REC("copy to user %dB ok\n",count);
		err = 0;
	}

	TRC_REC("<-%s\n",__FUNCTION__);
	return (err)?err:count;
}

static ssize_t mp_write(struct file *file_p, const char __user *buf, size_t length, loff_t * ppos)
{
	MV_PHONE *mp = file_p->private_data;
	MV_STATUS status;
	unsigned long count = 0;
	int err = 0;

	TRC_REC("->%s ch%d\n",__FUNCTION__,mp->ch);

	count = min(length, (size_t)MV_TDM_BUFF_SIZE);
	status = mvTdmChTx(mp->ch_info, (MV_U8 *)buf, count);

	if(status == MV_NOT_READY) { 
		TRC_REC("write not ready, try again\n");
		err = -EAGAIN;
	}
	else if(status == MV_FAIL) {
		TRC_REC("copy from user failed\n");
		printk("%s: copy from user failed\n",__FUNCTION__);
		err = -EFAULT;
	}
	else {
		TRC_REC("copy from user %dB ok\n",count);
		err = 0;
	}

	TRC_REC("<-%s\n",__FUNCTION__);
	return (err)?err:count;
}

static unsigned int mp_poll(struct file *file_p, poll_table * wait)
{
	MV_PHONE *mp = file_p->private_data;
	unsigned int mask = 0;

	TRC_REC("->%s ch%d\n",__FUNCTION__,mp->ch);

	poll_wait(file_p, &(mp->poll_q), wait);

	if(mvTdmChRxReady(mp->ch_info)) {
		mask |= POLLIN | POLLRDNORM;	/* readable */
		TRC_REC("poll can read\n");
	}
	if(mvTdmChTxReady(mp->ch_info)) {
		mask |= POLLOUT | POLLWRNORM;	/* writable */
		TRC_REC("poll can write\n");
	}
	if(mvTdmChExceptionReady(mp->ch_info)) {
		mask |= POLLPRI;		/* events */
		TRC_REC("poll can get event\n");
	}

	TRC_REC("<-%s\n",__FUNCTION__);
	return mask;
}

static int mp_ioctl(struct inode *inode, struct file *file_p, unsigned int cmd, unsigned long arg)
{
	MV_PHONE *mp = get_mp(iminor(inode) & 0xf);
	int retval = 0;

	TRC_REC("->%s ch%d\n",__FUNCTION__,mp->ch);

	/* check ioctls only root can use */
	if (!capable(CAP_SYS_ADMIN)) {
		return -EPERM;
	}

	disable_irq(mp->irq);

#ifdef CONFIG_MV_GTW_QOS
	if(cmd == PHONE_REC_START || cmd == PHONE_PLAY_START) {
		mv_gtw_qos_tos_enable();
	}
	if(cmd == PHONE_REC_STOP || cmd == PHONE_PLAY_STOP) {
		mv_gtw_qos_tos_disable();
	}
#endif

	switch (cmd) {
#ifdef MV_88F5181L
		case PHONE_REC_START:
			TRC_REC("PHONE_REC_START\n");
			mvTdmChRxEnable(mp->ch_info);
			break;
		case PHONE_REC_STOP:
			TRC_REC("PHONE_REC_STOP\n");
			mvTdmChRxDisable(mp->ch_info);
			break;
		case PHONE_PLAY_START:
			TRC_REC("PHONE_PLAY_START\n");
			mvTdmChTxEnable(mp->ch_info);
			break;
		case PHONE_PLAY_STOP:
			TRC_REC("PHONE_PLAY_STOP\n");
			mvTdmChTxDisable(mp->ch_info);
			break;
#else
		case PHONE_REC_START:
		case PHONE_PLAY_START:
			TRC_REC("PHONE_REC/PLAY_START\n");
			mvTdmChEnable(mp->ch_info);
			break;
		case PHONE_REC_STOP:
		case PHONE_PLAY_STOP:
			TRC_REC("PHONE_REC/PLAY_STOP\n");
			mvTdmChDisable(mp->ch_info);
			break;
#endif
		case PHONE_DIALTONE:
			TRC_REC("PHONE_DIALTONE\n");
			mvTdmChDialTone(mp->ch_info);
			break;
		case PHONE_BUSY:
			TRC_REC("PHONE_BUSY\n");
			mvTdmChBusyTone(mp->ch_info);
			break;
		case PHONE_CPT_STOP:
			TRC_REC("PHONE_CPT_STOP\n");
			mvTdmChStopTone(mp->ch_info);
			break;
	 	case PHONE_RING_START:
			TRC_REC("PHONE_RING_START\n");
			mvTdmChStartRing(mp->ch_info);
			break;
		case PHONE_RING_STOP:
			TRC_REC("PHONE_RING_STOP\n");
			mvTdmChStopRing(mp->ch_info);
			break;
		case PHONE_EXCEPTION:
		{
			MV_U8 offhook;
			union telephony_exception ex;
			TRC_REC("PHONE_EXCEPTION\n");
			ex.bytes = 0;
			mvTdmChEventGet(mp->ch_info, &offhook);
			if(offhook) {
				TRC_REC("off hook\n");
				ex.bits.hookstate = 1;
			}
			else {
				TRC_REC("on hook\n");
			}
			retval = ex.bytes;
		}
			break;
		case PHONE_RINGBACK:
			TRC_REC("PHONE_RINGBACK\n");
			mvTdmChRingBackTone(mp->ch_info);
			break;
		case PHONE_MV_READ_SLIC_REG:
#ifdef MV_IOCTL
			mvTdmSpiRead(arg, (MV_U8*)&retval);
#else
			printk("Driver is not configured to support this IOCTL\n");
			retval = -1;
#endif
			break;
		case PHONE_MV_WRITE_SLIC_REG:
#ifdef MV_IOCTL
			mvTdmSpiWrite((arg>>16)&0xff,arg&0xff);
#else
			printk("Driver is not configured to support this IOCTL\n");
			retval = -1;
#endif
			break;
		case PHONE_MV_READ_REG:
#ifdef MV_IOCTL
			retval = *((unsigned int*)(0xf1000000|arg));
#else
			printk("Driver is not configured to support this IOCTL\n");
			retval = -1;
#endif
			break;
		case PHONE_MV_WRITE_REG:
#ifdef MV_IOCTL
			printk("not implemented yet\n");
#else
			printk("Driver is not configured to support this IOCTL\n");
			retval = -1;
#endif
			break;
		case PHONE_MV_SPI_TEST:
#ifdef MV_IOCTL
			mvTdmChSpiTest(10000);
#else
			printk("Driver is not configured to support this IOCTL\n");
			retval = -1;
#endif
			break;
		default:
			printk("%s: Unsupported IOCTL\n",__FUNCTION__);
	}

	TRC_REC("<-%s\n",__FUNCTION__);
	enable_irq(mp->irq);
	return retval;
}


/* 
** Two types of interrupts exists: 
** (1) Slic async event (e.g. on/off-hook). 
** (2) TDM sync 10ms event (data is ready for read+write)
*/
static irqreturn_t mp_int_handler(int irq, void *dev_id, struct pt_regs *regs)
{
	MV_PHONE *mp = dev_id;
	irqreturn_t ret;

	TRC_REC("->->->%s\n",__FUNCTION__);

	if(mvTdmIsr() == MV_OK) {
		TRC_REC("wake up poll\n");
		wake_up_interruptible(&mp->poll_q);
		ret = IRQ_HANDLED;
	}
	else {
		TRC_REC("bogus isr\n");
		ret = IRQ_NONE;
	}

	TRC_REC("<-%s\n",__FUNCTION__);
	return ret;
}
