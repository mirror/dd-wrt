/* open.c
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
#include "../include/open.h"
#include "../include/setup.h"

#define __NO_VERSION__
#include <linux/module.h>

int can_open(struct inode *inode, struct file *file)
{
	struct msgobj_t *obj;
	struct canchip_t *chip;
	struct canuser_t *canuser;
	struct canque_ends_t *qends;
	struct canque_edge_t *edge;
	can_spin_irqflags_t iflags;
	char openflag;		// Martin Petera: Object already opened

	if ( ((obj=objects_p[MINOR_NR]) == NULL) || 
			((chip=objects_p[MINOR_NR]->hostchip) == NULL) ) {
		CANMSG("There is no hardware support for the device file with minor nr.: %d\n",MINOR_NR);
		return -ENODEV;
	}
	
#if defined(CONFIG_ARCH_KS8695_VSOPENRISC)
	if(check_and_set_dev_open_status(obj->minor, DRV_IN_USE_TTY) != 0)
		return -EBUSY;
	/* save old EPLD value */
	chip->old_epld_val = chip->read_register(chip->epld_addr);

	/* turn CAN drivers on */
	chip->write_register(EPLD_CAN, chip->epld_addr);
#endif

	atomic_inc(&obj->obj_used);
	DEBUGMSG("Device %d opened %d times.\n", MINOR_NR, atomic_read(&obj->obj_used));
	openflag = can_msgobj_test_fl(obj,OPENED);	// Martin Petera: store previous status
	can_msgobj_set_fl(obj,OPENED);

	if (chip->flags & CHIP_CONFIGURED) 
	{
		DEBUGMSG("Device is already configured.\n");
	}
	else {
		if (chip->chipspecops->chip_config(chip))
			CANMSG("Error configuring chip.\n");
		else
			chip->flags |= CHIP_CONFIGURED; 
	} /* End of chip configuration */


	/* Martin Petera: Fix for HCAN2
	 * pre_read was called only once -> Opening second MSG object from userspace
	 * didn't call function to configure MSG object for receive.
	 * FIX: Call pre_read once for each MSG object
	 **/
	if (!openflag) {
		if (chip->chipspecops->pre_read_config(chip,obj)<0)
			CANMSG("Error initializing chip for receiving\n");
	}

	/* clear CAN status */
	chip->canStatus = 0x0;

#ifdef CAN_USE_LEDS
	chip->canStatusLED = 0x0;
	can_write_reg(chip, chip->canStatusLED, LED_REGISTER);
#endif

	canuser = (struct canuser_t *)kmalloc(sizeof(struct canuser_t), GFP_KERNEL);
	if(canuser == NULL) goto no_canuser;
	canuser->flags=0;
	canuser->userinfo.fileinfo.file = file;
	canuser->msgobj = obj;
	canuser->magic = CAN_USER_MAGIC;
	file->private_data = canuser;

	qends = (struct canque_ends_t *)kmalloc(sizeof(struct canque_ends_t), GFP_KERNEL);
	if(qends == NULL) goto no_qends;
	canqueue_ends_init_kern(qends);
	canuser->qends = qends;
	
	/*required to synchronize with RT-Linux context*/
	can_spin_lock_irqsave(&canuser_manipulation_lock, iflags);
	list_add(&canuser->peers, &obj->obj_users);
	can_spin_unlock_irqrestore(&canuser_manipulation_lock, iflags);
	
#ifdef CAN_USE_LEDS
	printk(KERN_DEBUG "lincan: use LEDs\n");
#else
	printk(KERN_DEBUG "lincan: don't use LEDs\n");
#endif

	printk(KERN_DEBUG "lincan: buffer length: %d\n", MAX_BUF_LENGTH);

	if(canqueue_connect_edge(edge=canque_new_edge_kern(MAX_BUF_LENGTH),
		canuser->qends, obj->qends)<0) goto no_tx_qedge;

	if(canqueue_connect_edge(canuser->rx_edge0=canque_new_edge_kern(MAX_BUF_LENGTH),
		obj->qends, canuser->qends)<0) goto no_rx_qedge;
	/*FIXME: more generic model should be used there*/
	canque_edge_decref(canuser->rx_edge0);
	canque_edge_decref(edge);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,5,50))
	MOD_INC_USE_COUNT;
#endif	

	return 0;
	
    no_rx_qedge:
	canque_notify_bothends(edge, CANQUEUE_NOTIFY_DEAD_WANTED);
	canque_edge_decref(edge);
    no_tx_qedge:
	list_del(&canuser->peers);
	canuser->qends = NULL;
	canqueue_ends_dispose_kern(qends, 1);

    no_qends:
	kfree(canuser);

    no_canuser:
	atomic_dec(&obj->obj_used);

#if defined(CONFIG_ARCH_KS8695_VSOPENRISC)
	/* restore old EPLD value */
	obj->hostchip->write_register(obj->hostchip->old_epld_val, obj->hostchip->epld_addr);

	clear_dev_open_status(obj->minor, DRV_IN_USE_TTY);
#endif
    	return -ENOMEM;
}
