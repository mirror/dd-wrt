/* read.c
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

#include <rtl_posixio.h>
#include "../include/can_iortl.h"

/* This is the 'Normal' read handler for normal transmission messages */
static inline 
ssize_t can_std_read_rtl(struct canque_ends_t *qends, int nonblock_fl, 
			char *buffer, size_t length)
{
	int ret;
	struct canmsg_t *msg_buff = (struct canmsg_t *)buffer;
	int bytes_to_copy;
	struct canque_edge_t *qedge;
	struct canque_slot_t *slot;
	
	ret=canque_test_outslot(qends, &qedge, &slot);
	if(ret<0){
		if (nonblock_fl) {
			return -EAGAIN;
		}
		ret=canque_get_outslot_wait_rtl(qends, &qedge, &slot);
		if(ret<0){
			if (ret==-1) {
				DEBUGMSG("Rx interrupted\n");
				return -EINTR;
			}

			return -EIO;
		}
	}
	*(msg_buff++)=slot->msg;
	canque_free_outslot(qends, qedge, slot);
	bytes_to_copy = length-sizeof(struct canmsg_t);
	
	while (bytes_to_copy > 0) {
		ret=canque_test_outslot(qends, &qedge, &slot);
		if(ret<0)
			break;
		*(msg_buff++)=slot->msg;
		canque_free_outslot(qends, qedge, slot);
		bytes_to_copy -= sizeof(struct canmsg_t);
	}

	return length-bytes_to_copy;
}


ssize_t can_read_rtl_posix(struct rtl_file *fptr, char *buffer,
				size_t length, loff_t *ppos)
{
	struct canuser_t *canuser = 
		(struct canuser_t *)can_get_rtl_file_private_data(fptr);
	struct canque_ends_t *qends;
	int      ret;

	if(!canuser || (canuser->magic != CAN_USER_MAGIC)){
		CANMSG("can_read_rtl_posix: bad canuser magic\n");
		return -ENODEV;
	}

	if (length < sizeof(struct canmsg_t)) {
		DEBUGMSG("Trying to read less bytes than a CAN message, \n");
		DEBUGMSG("this will always return zero.\n");
		return 0;
	}

	qends = canuser->qends;

	/*if (((struct canmsg_t *)buffer)->flags & MSG_RTR)
		ret = can_rtr_read(chip, obj, buffer);
	else*/
		ret = can_std_read_rtl(qends, fptr->f_flags & O_NONBLOCK,
			buffer, length);

	return ret;
}

#endif /*CAN_WITH_RTL*/

