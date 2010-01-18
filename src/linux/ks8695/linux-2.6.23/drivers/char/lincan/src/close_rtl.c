/* close.c
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
int can_release_rtl_common(struct canuser_t *canuser, int file_flags)
{
	struct canque_ends_t *qends;
	struct msgobj_t *obj;
	can_spin_irqflags_t iflags;

	obj = canuser->msgobj;
	qends = canuser->qends;

	can_spin_lock_irqsave(&canuser_manipulation_lock, iflags);
	list_del(&canuser->peers);
	can_spin_unlock_irqrestore(&canuser_manipulation_lock, iflags);
	canuser->qends = NULL;
	canqueue_ends_dispose_rtl(qends, file_flags & O_SYNC);

	can_spin_lock_irqsave(&canuser_manipulation_lock, iflags);
	if(atomic_dec_and_test(&obj->obj_used)){
		can_msgobj_clear_fl(obj,OPENED);
	};
	can_spin_unlock_irqrestore(&canuser_manipulation_lock, iflags);

    #if (LINUX_VERSION_CODE < KERNEL_VERSION(2,5,50))
	MOD_DEC_USE_COUNT;
    #endif

	return 0;
}


int can_release_rtl_posix(struct rtl_file *fptr)
{
	struct canuser_t *canuser =
		(struct canuser_t *)can_get_rtl_file_private_data(fptr);
	int ret;
	
	if(!canuser || (canuser->magic != CAN_USER_MAGIC)){
		CANMSG("can_release_rtl_posix: bad canuser magic\n");
		return -ENODEV;
	}

	ret=can_release_rtl_common(canuser, fptr->f_flags);

	rt_free(canuser);
	
	return ret;
}

#endif /*CAN_WITH_RTL*/

