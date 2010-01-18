/* write.c
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
#include "../include/write.h"

ssize_t can_write(struct file *file, const char *buffer, size_t length, loff_t *offset)
{
	struct canuser_t *canuser = (struct canuser_t*)(file->private_data);
	struct msgobj_t *obj;
	struct canmsg_t msg_buff;
	struct canque_ends_t *qends;
	struct canque_edge_t *qedge;
	struct canque_slot_t *slot;
	int ret;
	unsigned bytes_to_copy;

	if(!canuser || (canuser->magic != CAN_USER_MAGIC)){
		CANMSG("can_write: bad canuser magic\n");
		return -ENODEV;
	}

	if (length < sizeof(struct canmsg_t)) {
		DEBUGMSG("Trying to write less bytes than a CAN message,\n");
		DEBUGMSG("this will always return 0 !\n");
		return 0;
	}
	if (length > INT_MAX) {
		CANMSG("Trying to write more than is supported.\n");
		return -1;
	}
	if (length % sizeof(struct canmsg_t)) {
		CANMSG("The number of bytes requested to be written is not a multiple of\n");
		CANMSG("'sizeof(struct canmsg_t)', currently this is not allowed.\n");
		return -1;
	} 

	/* Initialize hardware pointers */
	obj = canuser->msgobj;
	if (obj == NULL) {
		CANMSG("Could not assign buffer structure\n");
		return -1;
	}
	qends = canuser->qends;


	/* Prepare first message */
	ret = copy_from_user(&msg_buff, buffer, sizeof(struct canmsg_t));
	if(ret)	return -EFAULT;

	/* Automatic selection of extended format if ID>2047 */
	if (msg_buff.id & ~0x7ffl & MSG_ID_MASK ) msg_buff.flags |= MSG_EXT;
	/* has been dependent on "extended" option */

	/* If the output buffer is full, return immediately in case O_NONBLOCK
	 * has been specified or loop until space becomes available.
	 */
	if ((ret=canque_get_inslot4id(qends, &qedge, &slot, 
			0, msg_buff.id, 0))<0){
		DEBUGMSG("Buffer is full\n");

		if(ret < -1)
			return -EIO;

		if (file->f_flags & O_NONBLOCK)
			return -EAGAIN;

		ret=canque_get_inslot4id_wait_kern(qends, &qedge, &slot, 
						0, msg_buff.id, 0);
		if(ret<0) {
			if (signal_pending(current))
				return -EINTR;
			/*if (!can_timeout)*/
			return -EIO;
		}
	}
	slot->msg=msg_buff;
	canque_put_inslot(qends, qedge, slot);
	buffer += sizeof(struct canmsg_t);
	bytes_to_copy = length-sizeof(struct canmsg_t);

	/* 
	 * Try to send more messages
	 */
	while (bytes_to_copy >= sizeof(struct canmsg_t)) {
		bytes_to_copy -= sizeof(struct canmsg_t);
		/* Prepare first message */
		ret = copy_from_user(&msg_buff, buffer, sizeof(struct canmsg_t));
		if(ret)	return -EFAULT;

		/* Automatic selection of extended format if ID>2047 */
		if (msg_buff.id & ~0x7ffl & MSG_ID_MASK ) msg_buff.flags |= MSG_EXT;
		/* has been dependent on "extended" option */

		/* Get slot */
		if(canque_get_inslot4id(qends, &qedge, &slot, 
			0, msg_buff.id, 0) < 0) break;
		slot->msg=msg_buff;
		canque_put_inslot(qends, qedge, slot);
		buffer += sizeof(struct canmsg_t);
	}

        if(file->f_flags & O_SYNC) {
		canque_sync_wait_kern(qends, qedge);
	}
	return length-bytes_to_copy;
}	
