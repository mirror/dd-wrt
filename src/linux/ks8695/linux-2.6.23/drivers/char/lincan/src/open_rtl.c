/* open.c
 * Linux CAN-bus device driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * Rewritten for new CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

#ifdef CAN_WITH_RTL

#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/main.h"

#include <rtl_malloc.h>
#include <rtl_posixio.h>
#include "../include/can_iortl.h"

#define __NO_VERSION__
#include <linux/module.h>


static inline
int can_open_rtl_common(struct canuser_t *canuser, int open_flags)
{
	struct msgobj_t *obj=canuser->msgobj;
	struct canchip_t *chip;
	struct canque_ends_t *qends;
	struct canque_edge_t *edge;
	can_spin_irqflags_t iflags;

	if(!obj) return -ENODEV;
	
	can_msgobj_set_fl(obj,OPENED);
	
	chip=obj->hostchip;
	if (chip) {
		if (!(chip->flags & CHIP_CONFIGURED)) {
			if (chip->chipspecops->chip_config(chip))
				CANMSG("Error configuring chip.\n");
			else
				chip->flags |= CHIP_CONFIGURED; 

			if (chip->chipspecops->pre_read_config(chip,obj)<0)
				CANMSG("Error initializing chip for receiving\n");

		}
	} /* End of chip configuration */


	qends = (struct canque_ends_t *)rt_malloc(sizeof(struct canque_ends_t));
	if(qends == NULL) goto no_qends;
	canqueue_ends_init_rtl(qends);
	/* mark memory as allocated from RTL memory pool */
	qends->ends_flags|=CAN_ENDSF_MEM_RTL;
	canuser->qends = qends;
	
	can_spin_lock_irqsave(&canuser_manipulation_lock, iflags);
	list_add(&canuser->peers, &obj->obj_users);
	can_spin_unlock_irqrestore(&canuser_manipulation_lock, iflags);

	if(canqueue_connect_edge(edge=canque_new_edge_rtl(MAX_BUF_LENGTH),
		canuser->qends, obj->qends)<0) goto no_tx_qedge;

	if(canqueue_connect_edge(canuser->rx_edge0=canque_new_edge_rtl(MAX_BUF_LENGTH),
		obj->qends, canuser->qends)<0) goto no_rx_qedge;
	/*FIXME: more generic model should be used there*/
	canque_edge_decref(canuser->rx_edge0);
	canque_edge_decref(edge);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,5,50))
	MOD_INC_USE_COUNT;	/*is this enough for RT-Linux context ?*/
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

    	return -ENOMEM;
}


int can_open_rtl_posix(struct rtl_file *fptr)
{
	int ret;
	struct msgobj_t *obj;
	struct canchip_t *chip;
	struct canuser_t *canuser;
	int minor_nr = RTL_MINOR_FROM_FILEPTR(fptr);
	
	if(minor_nr>=MAX_TOT_MSGOBJS)
		return -ENODEV;

	if ( ((obj=objects_p[minor_nr]) == NULL) || 
			((chip=objects_p[minor_nr]->hostchip) == NULL) ) {
		CANMSG("There is no hardware support for the device file with minor nr.: %d\n",minor_nr);
		return -ENODEV;
	}

	atomic_inc(&obj->obj_used);
	DEBUGMSG("Device %d opened %d times.\n", minor_nr, atomic_read(&obj->obj_used));

	canuser = (struct canuser_t *)rt_malloc(sizeof(struct canuser_t));
	if(canuser == NULL){
		ret=-ENOMEM;
		goto no_canuser;
	}
	canuser->flags=CANUSER_RTL_CLIENT | CANUSER_RTL_MEM;
	canuser->userinfo.rtlinfo.file = fptr;
	canuser->msgobj = obj;
	canuser->magic = CAN_USER_MAGIC;

	/*next line would solve many problems, but RT-Linux lacks this fundamental field */
	/*fptr->private_data = canuser;*/
	/*to test code I am adding this terible hack*/
	can_set_rtl_file_private_data(fptr,canuser);

	ret=can_open_rtl_common(canuser, fptr->f_flags);
	if(ret>=0) return ret;

	rt_free(canuser);

    no_canuser:
	atomic_dec(&obj->obj_used);
	
	return ret;
}

#endif /*CAN_WITH_RTL*/
