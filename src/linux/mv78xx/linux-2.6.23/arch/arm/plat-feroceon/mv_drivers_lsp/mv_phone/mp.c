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
#include "gpp/mvGpp.h"
#include "mp.h"

extern MV_U8 currRxSampleGet(MV_U8 ch);
extern MV_U8 currTxSampleGet(MV_U8 ch);
extern u32 mvVoiceIfType;


#ifdef CONFIG_MV_GTW_QOS
extern int mv_gtw_qos_tos_enable(void);
extern int mv_gtw_qos_tos_disable(void);
#endif


/***********************
* PHONE DRIVER'S STUFF *
***********************/
typedef struct {
	struct phone_device p;
	unsigned int ch_info;
	unsigned int slic_dev;
	unsigned int daa_dev;
	unsigned char ch;
	unsigned int irq;
	unsigned char exception;
	int readers, writers;
	wait_queue_head_t poll_q;
	wait_queue_head_t read_q;
	wait_queue_head_t write_q;
        MV_VOICE_IF_TYPE chType;
        int cs;  /* for daisy chain */
	unsigned char rxActive, txActive, started;
} MV_PHONE;

static MV_PHONE mv_phone[MV_TDM_MAX_CHANNELS];
static char *cmdline = "dev0:fxs,dev1:fxs";
static int work_mode = DAISY_CHAIN_MODE;
static int interrupt_mode = INTERRUPT_TO_TDM;
static struct class *phone_class;
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
static irqreturn_t mp_int_handler(int irq, void *dev_id);
static void mp_spi_write(unsigned char addr, unsigned char data, MV_PHONE *mp);
static void mp_spi_read(unsigned char addr, unsigned char *data, MV_PHONE *mp);
static int slicRingBackTone(unsigned int slicDev);
static int slicEventGet(unsigned char *offhook, MV_PHONE *mp);
static int slicBusyTone(unsigned int slicDev);
static void slicSetLineFeedControl(unsigned char val, unsigned int slicDev);
static void slicReversDcPolarity(unsigned int slicDev);
static int slicSpiTest(unsigned int loop, MV_PHONE *mp);
static int fxdevInterruptBits(unsigned char ch, MV_PHONE *mp);
static int mp_get_int(unsigned int* work_done);
static int mp_cmdline_config(char *s);
static int mp_parse_config(int ifNum);
static int mp_check_config(void);






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



static int mp_parse_config(int ifNum)
{
	char type[4];
	int size = 3, i = 0, offset;
	offset = BASE_ENV_PARAM_OFF + (DEV_ENV_PARAM_OFF * ifNum); /* start of requested type in cmdline */

	if(ifNum < MV_TDM_MAX_CHANNELS)
	{
		while(i < size)
			type[i++] = cmdline[offset++];
		type[i] = '\0';
		if(!strcmp(type,"fxs"))
			return MV_FXS;
		else if(!strcmp(type,"fxo"))
			return MV_FXO;
	}

	return MV_ERROR;                                                                                                                                                                                                                                  }

static int mp_cmdline_config(char *s)
{
	cmdline = s;
	return 1;
}

__setup("mv_phone_config=", mp_cmdline_config);


static int mp_check_config(void)
{

#if defined(CONFIG_ARCH_FEROCEON_ORION)
	int boardId;

	boardId = mvBoardIdGet();

	 /* print board detection info */
 	   switch(boardId) {
		case RD_88F5181L_VOIP_GE:
			printk("Detected RD_88F5181L_VOIP_GE\n");
			work_mode = DAISY_CHAIN_MODE;
			interrupt_mode = INTERRUPT_TO_TDM;
               
			break;
		case RD_88F5181L_VOIP_FE:
			printk("Detected RD_88F5181L_VOIP_FE\n");
			work_mode = DAISY_CHAIN_MODE;
			interrupt_mode = INTERRUPT_TO_TDM;
			break;	
		case DB_88F5181L_DDR2_2XTDM:
			printk("Detected DB_88F5181L_DDR2_2XTDM\n");
			work_mode = DUAL_CHIP_SELECT_MODE;
			interrupt_mode = INTERRUPT_TO_MPP;
               
		    	break;

		case RD_88F5181L_VOIP_FXO_GE: 
		    	printk("Detected RD_88F5181L_VOIP_FXO_GE\n");
			work_mode = DUAL_CHIP_SELECT_MODE;
			interrupt_mode = INTERRUPT_TO_TDM;
				
               /* set life line control mpp */
				/*mvGppValueSet(0, MV_GPP11, MV_GPP11);*/
					
		    	break;
		default:
		    	printk("Error, platform does not support telephony\n");
		    	return MV_ERROR;
  	  }
#elif defined(CONFIG_ARCH_FEROCEON_KW)
			work_mode = DUAL_CHIP_SELECT_MODE;
			interrupt_mode = INTERRUPT_TO_TDM;
			mvBoardTdmLifeLineSet();
#endif
		return MV_OK;

	
}

static int __init mp_init(void)
{
	MV_PHONE *mp;
	int i, retval;
	u32 mpp0;
int err;
static int unit=0;
char dev_name[10];

	printk("Marvell Telephony Driver:\n");

	if (MV_FALSE == mvCtrlPwrClckGet(TDM_UNIT_ID, 0)) 
	{
		printk("\nWarning Tdm is Powered Off\n");
		return 0;
	}

	/* check system configuration */
	if(MV_ERROR == mp_check_config())
		return MV_ERROR; 

	TRC_INIT(currRxSampleGet, currTxSampleGet, 0, 0);
	TRC_REC("->%s\n",__FUNCTION__);

	/* General TDM and SLIC init */
	TRC_REC("tdm init\n");
	mvTdmInit(work_mode, interrupt_mode);
	mvTdmShowProperties();
	
    phone_class = class_create(THIS_MODULE, "telephony");
    if (IS_ERR(phone_class))
    {
        err = PTR_ERR(phone_class);
        return err;
    }
	
	/* per channel init */
	for(i=0; i<MV_TDM_MAX_CHANNELS; i++)
	{
		printk("Initializing channel %d\n",i);
		TRC_REC("ch%d init\n",i);
		mp = get_mp(i);
		mp->chType = mp_parse_config(i);
		if(mp->chType == MV_ERROR)
		{
			printk("%s: error, unknown device type (check u-boot config)\n",__FUNCTION__);
			return MV_ERROR;
		}
#ifdef CONFIG_ARCH_FEROCEON_ORION
		if((mvBoardIdGet() == RD_88F5181L_VOIP_FXO_GE) && (i != 0)) {
			mpp0 = MV_REG_READ(MPP_CONTROL_REG0);
					
			
			if(mp->chType == MV_FXO)
			{ 
	
				mvGppValueSet(0, MV_GPP3, MV_GPP3); /* reset daa */	
				mvGppValueSet(0, MV_GPP3, 0);

			}
			else {
				mvGppValueSet(0, MV_GPP3, 0); /* reset slic */
			   	mvGppValueSet(0, MV_GPP3, MV_GPP3);
			}
		}
#endif
        	printk("Unit Type: %s\n",mp->chType == MV_FXO ? "FXO":"FXS");                
		mp->p.board = mp->ch = i;
		mp->p.f_op = &mp_fops;
		mp->p.open = mp_open;
	

		if(interrupt_mode == INTERRUPT_TO_MPP)
		{
			mp->irq = mvBoardSlicGpioPinGet(i) + IRQ_GPP_START;	
		}
		else
			mp->irq = MP_IRQ; 

		init_waitqueue_head(&mp->poll_q);
		init_waitqueue_head(&mp->read_q);
		init_waitqueue_head(&mp->write_q);
                   
		if(mvTdmChInit(&mp->p, i, &(mp->ch_info), mp->chType) == MV_OK)
		{
		 mp->rxActive = 0;
		 mp->txActive = 0;
		 mp->started =  0;
		}
		else
		{
            	 printk("mvTdmChInit() failed !\n");
		 return -EIO;
		}
			
		if(mp->chType == MV_FXS)
			retval = initSlic(&(mp->slic_dev), mp->ch, work_mode, interrupt_mode);
		else
			retval = initDaa(&(mp->daa_dev), mp->ch, work_mode, interrupt_mode);
		if(!retval)
		{
			phone_register_device(&mp->p, PHONE_UNIT_ANY);
			printk("phone%d registered\n",i);
		}
		else
			printk("%s: Unable to initialize %s-%d\n",__FUNCTION__, mp->chType == 0 ? "FXO":"FXS", i);

        sprintf(dev_name, "phone%d", unit);
        device_create(phone_class, NULL, MKDEV(PHONE_MAJOR, unit), dev_name);
        unit++;
	}

	if (request_irq(MP_IRQ, mp_int_handler, IRQF_SHARED | IRQF_DISABLED, "mv-phone", &mv_phone[0])) {
		printk("Failed to connect IRQ %d\n", MP_IRQ);
		mp->irq = 0;
		return -EIO;
	}		

	if(interrupt_mode == INTERRUPT_TO_MPP)
	{
		mp = get_mp(0);
        	if (request_irq(mp->irq, mp_int_handler,  IRQF_DISABLED, "mv-voice0", (&mv_phone[0])+1)) {
			printk("Failed to connect IRQ %d\n", mp->irq);
			mp->irq = 0;
			return -EIO;
		}
		mp = get_mp(1);
        	if (request_irq(mp->irq, mp_int_handler,  IRQF_DISABLED, "mv-voice1", (&mv_phone[0])+2)) {
			printk("Failed to connect IRQ %d\n", mp->irq);
			mp->irq = 0;
			return -EIO;
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

	free_irq(MP_IRQ, &mv_phone[0]);
	TRC_REC("free irq %d\n",MP_IRQ);

	for(i=0; i<MV_TDM_MAX_CHANNELS; i++) {
		TRC_REC("ch%d remove\n",i);
		mp = get_mp(i);
		mvTdmChRemove(mp->ch_info);

	if(interrupt_mode == INTERRUPT_TO_MPP) {
		free_irq(mp->irq, (&mv_phone[0]) + i + 1);
		TRC_REC("free irq %d\n",mp->irq);

	}
		if(mp->chType == MV_FXS)
			slicFreeChannel(mp->slic_dev);
		else
			daaFreeChannel(mp->daa_dev);
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
	int i, first = 1;
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
	
	/* enable FXO/FXS interrupts */	
	if(mp->chType == MV_FXS)
		enableSlicInterrupts(mp->slic_dev);
	else
		enableDaaInterrupts(mp->daa_dev);

	for (i = 0; i < MV_TDM_MAX_CHANNELS; i++)
	{
		if (mp->started)
		{
			first = 0;
			break;
		}
	}
	if (first)
	{
		MV_REG_WRITE(INT_STATUS_REG,0); 	
		MV_REG_WRITE(INT_STATUS_MASK_REG,TDM_INT_SLIC); 
	}

	mp->started = 1;

	
	if(mvTdmChStart(mp->ch_info) == MV_OK)
	  mp->rxActive = mp->txActive = 0;
	else
	  printk("mvTdmChStart() failed !\n");

	TRC_REC("<-%s\n",__FUNCTION__);
	return 0;
}

static int mp_close(struct inode *inode, struct file *file_p)
{
	MV_PHONE *mp = file_p->private_data;
	int i, stillRunning = 0;
    	

	printk("Closing Marvell phone%d device\n",mp->ch);
	TRC_REC("->%s ch%d\n",__FUNCTION__,mp->ch);

	/* Disable Rx/Tx */
	if(mp->rxActive)
		mp->rxActive = 0;
	
	if(mp->txActive)
		mp->txActive = 0;
	
	/* Clear cause and disable all interrupts */
	mp->started = 0;
	TRC_REC("disable all ch%d interrupts\n",mp->ch);
	for (i = 0; i < MV_TDM_MAX_CHANNELS; i++)
	{
		if (mp->started)	
		{
			stillRunning = 1;
			break;
		}
	}	
	if (!stillRunning)
	{			
		MV_REG_WRITE(INT_STATUS_MASK_REG,0);
		MV_REG_WRITE(INT_STATUS_REG,0);
	}
	

	if(mvTdmChStop(mp->ch_info) == MV_OK)
          mp->rxActive = mp->txActive = 0;
	else
	  printk("mvTdmChStop() failed !\n");


	/* disable FXO/FXS interrupts */	
	if(mp->chType == MV_FXS)
		disableSlicInterrupts(mp->slic_dev);
	else
		disableDaaInterrupts(mp->daa_dev);
 

	wake_up_interruptible(&mp->poll_q);
	wake_up_interruptible(&mp->read_q);
	wake_up_interruptible(&mp->write_q);

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
	
	MV_STATUS status;
	unsigned long count = 0;
	unsigned int err = 0;
        MV_PHONE *mp = file_p->private_data;

	TRC_REC("->%s ch%d\n",__FUNCTION__,mp->ch);


	count = min(length, (size_t)MV_TDM_BUFF_SIZE);
	do
	{	
		if(mp->rxActive)
		   status = mvTdmChRx(mp->ch_info, buf, count);
		else
		{
		   TRC_REC("rx is not active!\n");
		   status = MV_NOT_READY;
		}

		if (status == MV_NOT_READY) 
		{ 
			if (file_p->f_flags & O_NONBLOCK) 
			{		
				TRC_REC("read not ready, try again\n");
				err = -EAGAIN;
				break;
			}
			else
			{
				
				if(!mp->rxActive)
				{
					mp->rxActive = 1;			
					interruptible_sleep_on(&mp->read_q);
					if (signal_pending(current)) 
					{
						return -EINTR;
					}
				 } 
				 else
				 {				
					err = -EFAULT;
					break;
				 }
				
			}
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
	} while (status == MV_NOT_READY);

	TRC_REC("<-%s\n",__FUNCTION__);
	return (err)?err:count;
}

static ssize_t mp_write(struct file *file_p, const char __user *buf, size_t length, loff_t * ppos)
{
	
	MV_STATUS status;
	unsigned long count = 0;
	int err = 0;
        MV_PHONE *mp = file_p->private_data;

	TRC_REC("->%s ch%d\n",__FUNCTION__,mp->ch);
	

	count = min(length, (size_t)MV_TDM_BUFF_SIZE);
	do
	{
		
		if(mp->txActive)
			status = mvTdmChTx(mp->ch_info, (MV_U8 *)buf, count);
		else
		{
			TRC_REC("tx is not active!\n");
			status = MV_NOT_READY;
		}

		if (status == MV_NOT_READY) 
		{ 
			if (file_p->f_flags & O_NONBLOCK) 
			{				
				TRC_REC("write not ready, try again\n");
				err = -EAGAIN;
				break;
			}
			else
			{
				if(!mp->txActive)
				{
					mp->txActive = 1;			
					interruptible_sleep_on(&mp->write_q);
					if (signal_pending(current)) 
					{
						return -EINTR;
					}
				}
				else
				{				
					err = -EFAULT;
					break;
				}
			}
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
	} while (status == MV_NOT_READY);
	TRC_REC("<-%s\n",__FUNCTION__);
	return (err)?err:count;
}

static unsigned int mp_poll(struct file *file_p, poll_table * wait)
{
	
	unsigned int mask = 0;
        MV_PHONE *mp = file_p->private_data;

	TRC_REC("->%s ch%d\n",__FUNCTION__,mp->ch);

	poll_wait(file_p, &(mp->poll_q), wait);
	
        if(mp->rxActive)
	{
	 if(mvTdmChRxReady(mp->ch_info)) {
		mask |= POLLIN | POLLRDNORM;	/* readable */
		TRC_REC("poll can read\n");
	 }
	}
	if(mp->txActive)
	{
	  if(mvTdmChTxReady(mp->ch_info)) {
		mask |= POLLOUT | POLLWRNORM;	/* writable */
		TRC_REC("poll can write\n");
	  }
	}
	  if(mp->exception == EXCEPTION_ON) {
		mask |= POLLPRI;		/* events */
		TRC_REC("poll can get event\n");
	  }



	TRC_REC("<-%s\n",__FUNCTION__);
	return mask;
}

static int mp_ioctl(struct inode *inode, struct file *file_p, unsigned int cmd, unsigned long arg)
{
	
	int retval = 0;
        MV_PHONE *mp = get_mp(iminor(inode) & 0xf);

	TRC_REC("->%s ch%d\n",__FUNCTION__,mp->ch);

	/* check ioctls only root can use */
	if (!capable(CAP_SYS_ADMIN)) {
		return -EPERM;
	}

	disable_irq(MP_IRQ);

	if(interrupt_mode == INTERRUPT_TO_MPP) 
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
		case PHONE_REC_START:
			TRC_REC("PHONE_REC_START\n");
			if(!mp->rxActive)
		        {
			   if(mvTdmChRxEnable(mp->ch_info) ==MV_OK)
				mp->rxActive = 1;
			   else TRC_REC("failed\n");
			}
			TRC_REC("rec already active\n");
			break;
		case PHONE_REC_STOP:
			TRC_REC("PHONE_REC_STOP\n");			
			if(mp->rxActive)
				mp->rxActive = 0;
			wake_up_interruptible(&mp->read_q);
			break;
		case PHONE_PLAY_START:
			TRC_REC("PHONE_PLAY_START\n");
			if(!mp->txActive)
		        {
				if(mvTdmChTxEnable(mp->ch_info) == MV_OK)
					mp->txActive = 1;
				else TRC_REC("failed\n");
			}
			TRC_REC("play already active\n");
			break;
		case PHONE_PLAY_STOP:
			TRC_REC("PHONE_PLAY_STOP\n");
			if(mp->txActive)
				mp->txActive = 0;
			wake_up_interruptible(&mp->write_q);
			break;
		case PHONE_EXCEPTION:
		{
			union telephony_exception ex;
			TRC_REC("PHONE_EXCEPTION\n");
			ex.bytes = 0;
			if(mp->chType == MV_FXS)
			{
				MV_U8 offhook;
				slicEventGet(&offhook, mp);
				if(offhook) {
					TRC_REC("off hook\n");				
					ex.bits.hookstate = 1;
				}
				else 
					TRC_REC("on hook\n");				
			}
			else
			{
				MV_U8 eventType;
				daaEventTypeGet(&eventType, mp->daa_dev);
				mp->exception = EXCEPTION_OFF;
				if(eventType & 0x80)
				{
					TRC_REC("Ring Detected\n");				
					ex.bits.pstn_ring = 1;
				}
				if(eventType & 0x1)
				{
					TRC_REC("Reverse Polarity Detected\n");				
					ex.bits.reverse_polarity = 1;

				}
				if(eventType & 0x8)
				{
					TRC_REC("Drop Out Detected\n");				
					ex.bits.drop_out = 1;

				}
	
			}
			retval = ex.bytes;
		}
			break;
		case PHONE_PSTN_SET_STATE:
			TRC_REC("PHONE_PSTN_SET_STATE\n");
			setPstnState(arg, mp->daa_dev);
			break;
		case PHONE_SET_CID_STATE:
			TRC_REC("PHONE_SET_CID_STATE\n");
			setDaaCidState(arg, mp->daa_dev);
			break;
		case PHONE_SET_REVERSE_POLARITY:
			TRC_REC("PHONE_SET_REVERSE_POLARITY\n");
			enableDaaReveresePolarity(arg, mp->daa_dev);
			break;
		case PHONE_SET_DIGITAL_HYBRID:
			TRC_REC("PHONE_SET_DIGITAL_HYBRID\n");
			setDaaDigitalHybrid((unsigned int)arg, mp->daa_dev);
			break;
		case PHONE_GET_LINE_VOLTAGE:
			TRC_REC("PHONE_GET_LINE_VOLTAGE\n");
			retval = daaGetLineVoltage(mp->daa_dev);
			break;
		case PHONE_DIALTONE:
			TRC_REC("PHONE_DIALTONE\n");
			dialTone(mp->slic_dev);
			break;
		case PHONE_BUSY:
			TRC_REC("PHONE_BUSY\n");
			slicBusyTone(mp->slic_dev);
			break;
		case PHONE_CPT_STOP:
			TRC_REC("PHONE_CPT_STOP\n");
			stopTone(mp->slic_dev);
			break;
	 	case PHONE_RING_START:
			TRC_REC("PHONE_RING_START\n");
			activateRinging(mp->slic_dev);
			break;
		case PHONE_RING_STOP:
			TRC_REC("PHONE_RING_STOP\n");
			stopRinging(mp->slic_dev);
			break;
		case PHONE_RING_CADENCE:
			TRC_REC("PHONE_RING_CADENCE\n");
			slicRingCadence(arg, mp->slic_dev);
			break;			
		case PHONE_RINGBACK:
			TRC_REC("PHONE_RINGBACK\n");
			slicRingBackTone(mp->slic_dev);
			break;
		case PHONE_PSTN_REVERSE_POLARITY:	/* Timor */
			TRC_REC("PHONE_PSTN_REVERSE_POLARITY\n");
			slicReversDcPolarity(mp->slic_dev);
			break;
		case PHONE_PSTN_SEND_NTT_CRA:		/* Timor */
			TRC_REC("PHONE_PSTN_SEND_NTT_CRA\n");
			slicSendNTTCRATone(mp->slic_dev);
			break;

        	case PHONE_SET_LINE_FEED_CONTROL:               /* Timor */
            		TRC_REC("PHONE_SET_LINE_FEED_CONTROL\n");
            		slicSetLineFeedControl((unsigned char)arg, mp->slic_dev);
            		break;

		case PHONE_MV_READ_SLIC_REG:
#ifdef MV_IOCTL
	
             		mp_spi_read(arg, (MV_U8*)&retval, mp);
#else
			printk("Driver is not configured to support this IOCTL\n");
			retval = -1;
#endif
			break;
		case PHONE_MV_WRITE_SLIC_REG:
#ifdef MV_IOCTL
			
            		mp_spi_write((arg>>16)&0xff,arg&0xff, mp);
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
			slicSpiTest(10000, mp);
#else
			printk("Driver is not configured to support this IOCTL\n");
			retval = -1;
#endif
			break;
		default:
			printk("mv_phone %s: unsupported IOCTL\n",__FUNCTION__);
	}

	TRC_REC("<-%s\n",__FUNCTION__);
	enable_irq(MP_IRQ);

	if(interrupt_mode == INTERRUPT_TO_MPP)
		enable_irq(mp->irq);


	return retval;
}


/* 
** Two types of interrupts exists: 
** (1) FXS/FXO async event (e.g. on/off-hook). 
** (2) TDM sync 10ms event (data is ready for read+write)
*/
static irqreturn_t mp_int_handler(int irq, void *dev_id)
{	
	MV_U32 work_done = 0;
	MV_U8 i;
	MV_U8 wake_up_poll_q;

	TRC_REC("->->->%s\n",__FUNCTION__);
	if (!mp_get_int(&work_done)) 
	{		
		for (i = 0; i < MV_TDM_MAX_CHANNELS; i++)
		{								
			wake_up_poll_q = 0;
			if (work_done & (MV_TDM_SLIC_INTR << MV_CHANNEL_INTR(i)))
			{
				TRC_REC("wake up for ex\n");
				wake_up_poll_q = 1;
			}
			if (work_done & (MV_TDM_READ_INTR << MV_CHANNEL_INTR(i)))
			{
				wake_up_poll_q = 1;
				TRC_REC("wake up for rd\n");
				wake_up_interruptible(&mv_phone[i].read_q);
			}
			if (work_done & (MV_TDM_WRITE_INTR << MV_CHANNEL_INTR(i)))
			{		
				wake_up_poll_q = 1;
				TRC_REC("wake up for wr\n");
				wake_up_interruptible(&mv_phone[i].write_q);
			}
			if (wake_up_poll_q) {
				TRC_REC("wake up poll\n");
				wake_up_interruptible(&mv_phone[i].poll_q);	
			}
		}
	}
	TRC_REC("<-%s\n",__FUNCTION__);
	return IRQ_HANDLED;
}

static void mp_spi_write(unsigned char addr, unsigned char data, MV_PHONE *mp)
{
	if(mp->chType == MV_FXS)
		writeDirectReg(mp->slic_dev, addr, data);
	else
		writeDaaDirectReg(mp->daa_dev, addr, data);


}

static void mp_spi_read(unsigned char addr, unsigned char *data, MV_PHONE *mp)
{
	if(mp->chType == MV_FXS)
		*data = readDirectReg(mp->slic_dev, addr);
	else	
		*data = readDaaDirectReg(mp->daa_dev, addr);


}

static int slicEventGet(unsigned char *offhook, MV_PHONE *mp)
{
	
	MV_U8 tmp;

	TRC_REC("->%s ch%d\n",__FUNCTION__,mp->ch);

  
    	mp_spi_read(68, &tmp, mp);

	/* hook state (0-on, 1-off) */
	*offhook = !(tmp & 4);

	if(*offhook)
		TRC_REC("off-hook\n");		
	else		
		TRC_REC("on-hook\n");	
	mp->exception = EXCEPTION_OFF;
	TRC_REC("<-%s\n",__FUNCTION__);
	return 0;
}

static int slicBusyTone(unsigned int slicDev)
{
   
#if 1
	busyTone(slicDev);
#else
	busyJapanTone(slicDev);
#endif
	return 0;
}


static int slicRingBackTone(unsigned int slicDev)
{
#if 1
	ringBackTone(slicDev);
#else
	ringBackJapanTone(slicDev);
#endif
	return 0;
}

static void slicSetLineFeedControl(unsigned char val, unsigned int slicDev)    /* Timor */
{
    writeDirectReg(slicDev, LINE_STATE, val);
	return ;
}

static void slicReversDcPolarity(unsigned int slicDev)         /* Timor */
{ 
    MV_U8 data;
    /* read value of On-Hook Line Voltage register */
    data = readDirectReg(slicDev, OFF_HOOK_V);
    /* toggle the VSGN bit - VTIPVRING polarity */
    writeDirectReg(slicDev, OFF_HOOK_V, data^0x40);
    
    return;
}

static int slicSpiTest(unsigned int loop, MV_PHONE *mp)
{
	
	unsigned char  w,r;
	int i,ch;

	for(ch=0; ch<MV_TDM_MAX_CHANNELS; ch++)
	{

	    /*mvTdmSetCurrentUnit(mp->cs);*/
		printk("SPI channel %d Write/Read test %d loops... ",mp->ch,loop);

		for(i=0;i<loop;i++) 
		{
			w = (i & 0x7f);
			r = 0;
			mp_spi_write(2, w, mp);
			mp_spi_read(2, &r, mp);
			if(r!=w) 
			{
				printk("SPI: Wrote %x, Read = %x ",w,r);
				break;
			}
		}

		if(i != loop)
		{
			printk("failed\n");
			return 1;
		}

		printk("ok\n");
	}
	return 0;
}


static int mp_get_int(unsigned int *work_done)
{
	MV_PHONE *mp;	
	unsigned int cause, gppDataIn = 0, isFxInterrupt = 0;
	unsigned char ch;
	
	/* Read and clear cause */
	cause = MV_REG_READ(INT_STATUS_REG);
	MV_REG_WRITE(INT_STATUS_REG, ~cause);

	if(interrupt_mode == INTERRUPT_TO_MPP) {
		gppDataIn = MV_REG_READ(GPP_DATA_IN_REG(0));
		TRC_REC("->%s gppcause=0x%x\n",__FUNCTION__, gppDataIn);
	}
	else
		TRC_REC("->%s tdmcause=0x%x mask=0x%x\n",__FUNCTION__, cause, MV_REG_READ(INT_STATUS_MASK_REG));



	/* Refer only to unmasked bits */
	cause &= (MV_REG_READ(INT_STATUS_MASK_REG));
	isFxInterrupt = ((interrupt_mode == INTERRUPT_TO_MPP) ?  (gppDataIn & (BIT8 | BIT9)) : (cause & SLIC_INT_BIT)) ;

	if(isFxInterrupt) 	
	{
		for (ch=0; ch<MV_TDM_MAX_CHANNELS; ch++)
		{
 			mp = get_mp(ch);
			
			if (fxdevInterruptBits(ch, mp)) { 
				mp->exception = EXCEPTION_ON;				
				TRC_REC("ch%d voice unit interrupt\n",ch);				
				*work_done |= MV_TDM_SLIC_INTR << MV_CHANNEL_INTR(ch);			
			}			
		}		
	}

	if(cause & DMA_ABORT_BIT)
	{
		mvOsPrintf("%s: DMA data abort. Address: 0x%08x, Info: 0x%08x\n",
		__FUNCTION__, MV_REG_READ(DMA_ABORT_ADDR_REG), MV_REG_READ(DMA_ABORT_INFO_REG));
		*work_done |= MV_TDM_DMA_ABORT_INTR;
	}

	for(ch=0; ch<MV_TDM_MAX_CHANNELS; ch++)
	{
		
		mp = get_mp(ch);
	
		if (cause & TDM_INT_RX(ch))
		{
			if (cause & RX_BIT(ch)) 
			{
				/* Give next buff to TDM and set curr buff as full */				
			
				if (mp->rxActive)
                           		mvTdmChRxLow(mp->ch_info);	
						
			}
			if (cause & RX_OVERFLOW_BIT(ch))
			{
				/*Channel Rx goes to idle*/	
				if (likely(!mp->rxActive))
				{
					MV_REG_WRITE(INT_STATUS_MASK_REG, MV_REG_READ(INT_STATUS_MASK_REG) & (~(TDM_INT_RX(mp->ch)))); 
					TRC_REC("ch %d: Rx stopped.\n",mp->ch);
				}
				else
				{
					mvOsPrintf("ch %d: Rx overflow.\n",mp->ch);
				}
			}
			*work_done |= MV_TDM_READ_INTR << MV_CHANNEL_INTR(ch);			
		}

		if (cause & TDM_INT_TX(ch))
		{
			/* Give next buff to TDM and set curr buff as empty */
			if (cause & TX_BIT(ch))
			{
			     if (mp->txActive)			
				mvTdmChTxLow(mp->ch_info); 
				 					
			}
			if(cause & TX_UNDERFLOW_BIT(ch))
			{
				/*Channel Tx goes to idle*/	
				if (likely(!mp->txActive))
				{
					MV_REG_WRITE(INT_STATUS_MASK_REG, MV_REG_READ(INT_STATUS_MASK_REG) & (~(TDM_INT_TX(mp->ch)))); 
					TRC_REC("ch %d: Tx stopped.\n",mp->ch);
				}
				else
				{
					mvOsPrintf("ch %d: Tx underflow.\n",mp->ch);
				}
			}
			*work_done |= MV_TDM_WRITE_INTR << MV_CHANNEL_INTR(ch);			
		}
	}
	TRC_REC("<-%s\n",__FUNCTION__);
	return( (*work_done) ? 0 : 1 );
}


static int fxdevInterruptBits(unsigned char ch, MV_PHONE *mp)
{
	MV_BOOL retVal; 
	TRC_REC("<-%s: %s\n",__FUNCTION__, mp->chType == MV_FXS ? "FXS":"FXO");

	if(mp->chType == MV_FXS)
		retVal = checkSlicInterrupts(mp->slic_dev);  
	else
		retVal = checkDaaInterrupts(mp->daa_dev);
     
     
    return retVal;

}


