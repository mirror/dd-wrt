/* close.c
 * Linux CAN-bus device driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/main.h"
#include "../include/close.h"
#include "../include/setup.h"
#include "../include/fasync.h"
#include "../include/sja1000p.h"

#define __NO_VERSION__
#include <linux/module.h>
int can_close(struct inode *inode, struct file *file)
{
	struct canuser_t *canuser = (struct canuser_t*)(file->private_data);
	struct canque_ends_t *qends;
	struct msgobj_t *obj;
	can_spin_irqflags_t iflags;
	int status, wait_loops, i = 0;
	
	if(!canuser || (canuser->magic != CAN_USER_MAGIC)){
		CANMSG("can_close: bad canuser magic\n");
		return -ENODEV;
	}
	
	obj = canuser->msgobj;
	qends = canuser->qends;
	
    #ifdef CAN_ENABLE_KERN_FASYNC

	can_fasync(-1, file, 0);

    #endif /*CAN_ENABLE_KERN_FASYNC*/

	can_spin_lock_irqsave(&canuser_manipulation_lock, iflags);
	list_del(&canuser->peers);
	can_spin_unlock_irqrestore(&canuser_manipulation_lock, iflags);
	canuser->qends = NULL;
	canqueue_ends_dispose_kern(qends, file->f_flags & O_SYNC);
	

	kfree(canuser);

	can_spin_lock_irqsave(&canuser_manipulation_lock, iflags);
	if(atomic_dec_and_test(&obj->obj_used)){
		can_msgobj_clear_fl(obj,OPENED);
	};
	can_spin_unlock_irqrestore(&canuser_manipulation_lock, iflags);



	while(1)
	{
		i = 0;
		wait_loops = CAN_MAX_TRANSMIT_TIMEOUT;
		 
		/* wait until transmit buffer is released */
		while ( !((status=can_read_reg(obj->hostchip, SJASR)) & sjaSR_TBS) && 
							i++<wait_loops) 
		{
			udelay(CAN_WAIT_FOR_TBS);
		}

		/* timeout sending frame */
		if(!(status & sjaSR_TBS))
		{
			/* abort transmission */
			can_write_reg(obj->hostchip, sjaCMR_AT, SJACMR);
			goto flush_queues;
		}

		/* bus error */
		if ((can_read_reg(obj->hostchip, SJASR) & sjaSR_BS)) 
		{
			goto flush_queues;
		}
	
		/* get next frame */
		if(canque_test_outslot(obj->qends, &obj->tx_qedge, &obj->tx_slot) < 0)
		{
			goto flush_queues;
		}

		/* fill transmit buffer */
		if (obj->hostchip->chipspecops->pre_write_config(obj->hostchip, obj, &obj->tx_slot->msg)) {
			obj->ret = -1;
			canque_notify_inends(obj->tx_qedge, CANQUEUE_NOTIFY_ERRTX_PREP);
			canque_free_outslot(obj->qends, obj->tx_qedge, obj->tx_slot);
			obj->tx_slot=NULL;
			goto flush_queues;
		}

		/*send the frame */
		if (obj->hostchip->chipspecops->send_msg(obj->hostchip, obj, &obj->tx_slot->msg)) {
			obj->ret = -1;
			canque_notify_inends(obj->tx_qedge, CANQUEUE_NOTIFY_ERRTX_SEND);
			canque_free_outslot(obj->qends, obj->tx_qedge, obj->tx_slot);
			obj->tx_slot=NULL;
			goto flush_queues;
		}
	}

flush_queues:
	/* disable interrupts */
	obj->hostchip->chipspecops->enable_configuration(obj->hostchip);
	can_write_reg(obj->hostchip,sjaDISABLE_INTERRUPTS, SJAIER);
	obj->hostchip->chipspecops->disable_configuration(obj->hostchip);

	if(canqueue_ends_flush_outlist(qends) < 0)
		printk("lincan: error flushing outlists\n");
	if(canqueue_ends_flush_inlist(qends) < 0)
		printk("lincan: error flushing inlists\n");

	/* clear canStatus */
	obj->hostchip->canStatus = 0x0;
#ifdef CAN_USE_LEDS
	obj->hostchip->canStatusLED = 0x0;
	can_write_reg(obj->hostchip, obj->hostchip->canStatusLED, LED_REGISTER);
#endif

	/* force the chip to reconfigured by open */
	obj->hostchip->flags &= ~CHIP_CONFIGURED;
	
    #if (LINUX_VERSION_CODE < KERNEL_VERSION(2,5,50))
	MOD_DEC_USE_COUNT;
    #endif

#if defined(CONFIG_ARCH_KS8695_VSOPENRISC)
	/* restore old EPLD value */
	obj->hostchip->write_register(obj->hostchip->old_epld_val, obj->hostchip->epld_addr);

	clear_dev_open_status(obj->minor, DRV_IN_USE_TTY);
#endif
	return 0;
}
