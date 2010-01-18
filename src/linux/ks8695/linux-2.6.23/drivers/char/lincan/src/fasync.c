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
#include "../include/fasync.h"

#ifdef CAN_ENABLE_KERN_FASYNC

int can_fasync(int fd, struct file *file, int on)
{
	int retval;
	
	struct canuser_t *canuser = (struct canuser_t*)(file->private_data);
	struct canque_ends_t *qends;
	
	if(!canuser || (canuser->magic != CAN_USER_MAGIC)){
		CANMSG("can_close: bad canuser magic\n");
		return -ENODEV;
	}
	
	qends = canuser->qends;

	retval = fasync_helper(fd, file, on, &qends->endinfo.fileinfo.fasync);

	if (retval < 0)
		return retval;
	return 0;
}





#endif /*CAN_ENABLE_KERN_FASYNC*/
