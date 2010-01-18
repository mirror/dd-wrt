/* read.c
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
#include "../include/read.h"

/* This is the 'Normal' read handler for normal transmission messages */
ssize_t can_read(struct file *file, char *buffer, size_t length, loff_t *offset)
{
	struct canuser_t *canuser = (struct canuser_t*)(file->private_data);
	struct canque_ends_t *qends;
	int bytes_to_copy;
	struct canque_edge_t *qedge;
	struct canque_slot_t *slot;
	int ret;

	if(!canuser || (canuser->magic != CAN_USER_MAGIC)){
		CANMSG("can_read: bad canuser magic\n");
		return -ENODEV;
	}

	if (length < sizeof(struct canmsg_t)) {
		DEBUGMSG("Trying to read less bytes than a CAN message, \n");
		DEBUGMSG("this will always return zero.\n");
		return 0;
	}

	qends = canuser->qends;

	ret=canque_test_outslot(qends, &qedge, &slot);
	if(ret<0){
		if (file->f_flags & O_NONBLOCK) {
			return -EAGAIN;
		}
		ret=canque_get_outslot_wait_kern(qends, &qedge, &slot);
		if(ret<0){
			if (signal_pending(current)) {
				DEBUGMSG("Rx interrupted\n");
				return -EINTR;
			}
			/*if (!can_timeout) {
				DEBUGMSG("no data received\n");
				return 0;
			}*/
			return -EIO;
		}
	}
	
	ret = copy_to_user(buffer, &slot->msg, sizeof(struct canmsg_t));
	canque_free_outslot(qends, qedge, slot);
	buffer += sizeof(struct canmsg_t);
	bytes_to_copy = length-sizeof(struct canmsg_t);
	if(ret)	return -EFAULT;
	
	while (bytes_to_copy > 0) {
		ret=canque_test_outslot(qends, &qedge, &slot);
		if(ret<0)
			break;
		ret = copy_to_user(buffer, &slot->msg, sizeof(struct canmsg_t));
		canque_free_outslot(qends, qedge, slot);
		buffer += sizeof(struct canmsg_t);
		bytes_to_copy -= sizeof(struct canmsg_t);
		if(ret)	return -EFAULT;
	}

	return length-bytes_to_copy;
}

