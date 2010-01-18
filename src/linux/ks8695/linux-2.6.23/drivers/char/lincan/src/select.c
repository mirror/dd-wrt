/* select.c
 * Header file for the Linux CAN-bus driver.
 * Written by Arnaud Westenberg email:arnaud@wanadoo.nl
 * Added by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/main.h"

#include <linux/poll.h>
#include "../include/select.h"

unsigned int can_poll(struct file *file, poll_table *wait)
{
	struct canuser_t *canuser = (struct canuser_t*)(file->private_data);
	struct canque_ends_t *qends;
	struct msgobj_t *obj;
        unsigned int mask = 0;
	struct canque_edge_t *edge;
	int full=0;
	int i;

	if(!canuser || (canuser->magic != CAN_USER_MAGIC)){
		CANMSG("can_close: bad canuser magic\n");
		return -ENODEV;
	}
	
	obj = canuser->msgobj;
	qends = canuser->qends;

        if (file->f_mode & FMODE_READ) {
                poll_wait(file, &qends->endinfo.fileinfo.readq, wait);
		for(i=CANQUEUE_PRIO_NR;--i>=0;) {
			if(!list_empty(&qends->active[i]))
				mask |= POLLIN | POLLRDNORM;
		}
        }

        if ((file->f_mode & FMODE_WRITE) && !(file->f_flags & O_SYNC)) {
                poll_wait(file, &qends->endinfo.fileinfo.writeq, wait);

		canque_for_each_inedge(qends, edge) {
			if(canque_fifo_test_fl(&edge->fifo,FULL))
				full=1;
		}

		if(!full)
			mask |= POLLOUT | POLLWRNORM;
	}

        if ((file->f_mode & FMODE_WRITE) && (file->f_flags & O_SYNC)) {
                poll_wait(file, &qends->endinfo.fileinfo.emptyq, wait);

		canque_for_each_inedge(qends, edge) {
			if(!canque_fifo_test_fl(&edge->fifo,EMPTY))
				full=1;
		}

		if(!full)
			mask |= POLLOUT | POLLWRNORM;
	}
	return mask;
}
