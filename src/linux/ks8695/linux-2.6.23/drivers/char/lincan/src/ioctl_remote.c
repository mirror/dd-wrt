/* ioctl_remote.c
 * Linux CAN-bus device driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Nov 2004
 */

#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/main.h"
#include "../include/read.h"
#include "../include/ioctl.h"

/* This is the 'RTR' read handler for remote transmission request messages */
int can_ioctl_remote_read(struct canuser_t *canuser, struct canmsg_t *rtr_msg,
                          unsigned long rtr_id, int options)
{
	can_spin_irqflags_t flags;
	struct rtr_id *rtr_current, *new_rtr_entry;
	struct msgobj_t *obj;
	struct canchip_t *chip;
	/*struct canque_ends_t *qends;*/
	
	DEBUGMSG("Remote transmission request\n");

	/*qends = canuser->qends;*/

	/* Initialize hardware pointers */
	obj = canuser->msgobj;
	if (obj == NULL) {
		CANMSG("Could not assign buffer structure\n");
		return -ENODEV;
	}

	if ( (chip = obj->hostchip) == NULL) {
		CANMSG("Device is not correctly configured,\n");
		CANMSG("please reload the driver.\n");
		return -ENODEV;
	}

	can_spin_lock_irqsave(&hardware_p->rtr_lock, flags);
	if (hardware_p->rtr_queue == NULL) { //No remote messages pending
		new_rtr_entry=(struct rtr_id *)kmalloc(sizeof(struct rtr_id),GFP_ATOMIC);
		if (new_rtr_entry == NULL) {
			can_spin_unlock_irqrestore(&hardware_p->rtr_lock, 
								flags);
			return -ENOMEM;
		}
		hardware_p->rtr_queue=new_rtr_entry;
	}
	else {
		rtr_current=hardware_p->rtr_queue;
		while (rtr_current->next != NULL)
			rtr_current=rtr_current->next;
		new_rtr_entry=(struct rtr_id *)kmalloc(sizeof(struct rtr_id),GFP_ATOMIC);
		rtr_current->next=new_rtr_entry;
	}
	init_waitqueue_head(&new_rtr_entry->rtr_wq);
	new_rtr_entry->id = rtr_id;
	new_rtr_entry->rtr_message = rtr_msg;
	new_rtr_entry->next=NULL;

	can_spin_unlock_irqrestore(&hardware_p->rtr_lock, flags);

	/* Send remote transmission request */
	chip->chipspecops->remote_request(chip,obj);
	obj->ret = 0;
	interruptible_sleep_on(&new_rtr_entry->rtr_wq);

	can_spin_lock_irqsave(&hardware_p->rtr_lock, flags);
	if (hardware_p->rtr_queue == new_rtr_entry) {
		if (new_rtr_entry->next != NULL) 
			hardware_p->rtr_queue=new_rtr_entry->next;
		else
			hardware_p->rtr_queue=NULL;
	}
	else {
		rtr_current=hardware_p->rtr_queue;
		while (rtr_current->next != new_rtr_entry)
			rtr_current=rtr_current->next;
		if (new_rtr_entry->next != NULL)
			rtr_current->next=new_rtr_entry->next;
		else
			rtr_current->next=NULL;
	}
	can_spin_unlock_irqrestore(&hardware_p->rtr_lock, flags);
	kfree(new_rtr_entry);

	return obj->ret;
}

