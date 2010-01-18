/* devcommon.c - common device code
 * Linux CAN-bus device driver.
 * New CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/can_queue.h"
#include "../include/main.h"
#include "../include/devcommon.h"

#ifdef CAN_WITH_RTL
static inline
void canqueue_wake_chip_worker(struct canque_ends_t *qends, struct canchip_t *chip, struct msgobj_t *obj)
{
	if(qends->endinfo.chipinfo.worker_thread){
		can_msgobj_set_fl(obj,WORKER_WAKE);
		pthread_kill(qends->endinfo.chipinfo.worker_thread,RTL_SIGNAL_WAKEUP);
		rtl_schedule();
	} else {
		set_bit(MSGOBJ_TX_REQUEST_b,&chip->pend_flags);
		if(chip->worker_thread) {
			set_bit(MSGOBJ_WORKER_WAKE_b,&chip->pend_flags);
			pthread_kill(chip->worker_thread,RTL_SIGNAL_WAKEUP);
			rtl_schedule();
		}
	}
}

#endif /*CAN_WITH_RTL*/


/**
 * canqueue_notify_chip - notification callback handler for CAN chips ends of queues
 * @qends: pointer to the callback side ends structure
 * @qedge: edge which invoked notification 
 * @what: notification type
 *
 * This function has to deal with more possible cases. It can be called from
 * the kernel or interrupt context for Linux only compilation of driver.
 * The function can be called from kernel context or RT-Linux thread context
 * for mixed mode Linux/RT-Linux compilation.
 */
void canqueue_notify_chip(struct canque_ends_t *qends, struct canque_edge_t *qedge, int what)
{
	struct canchip_t *chip=qends->endinfo.chipinfo.chip;
	struct msgobj_t *obj=qends->endinfo.chipinfo.msgobj;

	DEBUGMSG("canqueue_notify_chip for edge %d and event %d\n",qedge->edge_num,what);
	switch(what){
		/*case CANQUEUE_NOTIFY_EMPTY:*/
		/*case CANQUEUE_NOTIFY_SPACE:*/
		/*case CANQUEUE_NOTIFY_NOUSR:
			wake_up(&qends->endinfo.chipinfo.daemonq);
			break;*/
		case CANQUEUE_NOTIFY_PROC:
		    #ifndef CAN_WITH_RTL
			/*wake_up(&qends->endinfo.chipinfo.daemonq);*/
			chip->chipspecops->wakeup_tx(chip, obj);
		    #else /*CAN_WITH_RTL*/
			can_msgobj_set_fl(obj,TX_REQUEST);
			canqueue_wake_chip_worker(qends, chip, obj);
		    #endif /*CAN_WITH_RTL*/
			break;
		case CANQUEUE_NOTIFY_DEAD_WANTED:
		case CANQUEUE_NOTIFY_DEAD:
			if(canque_fifo_test_and_clear_fl(&qedge->fifo, READY))
				canque_edge_decref(qedge);
			break;
		case CANQUEUE_NOTIFY_ATTACH:
			break;
		case CANQUEUE_NOTIFY_FILTCH:
			if(!chip->chipspecops->filtch_rq)
				break;
		    #ifndef CAN_WITH_RTL
			chip->chipspecops->filtch_rq(chip, obj);
		    #else /*CAN_WITH_RTL*/
			can_msgobj_set_fl(obj,FILTCH_REQUEST);
			canqueue_wake_chip_worker(qends, chip, obj);
		    #endif /*CAN_WITH_RTL*/
			
			break;
	}
}


/**
 * canqueue_ends_init_chip - CAN chip specific ends initialization
 * @qends: pointer to the ends structure
 * @chip: pointer to the corresponding CAN chip structure
 * @obj: pointer to the corresponding message object structure
 */
int canqueue_ends_init_chip(struct canque_ends_t *qends, struct canchip_t *chip, struct msgobj_t *obj)
{
	int ret;
	ret=canqueue_ends_init_gen(qends);
	if(ret<0) return ret;
	
	qends->context=NULL;
    #ifndef CAN_WITH_RTL
	init_waitqueue_head(&qends->endinfo.chipinfo.daemonq);
    #endif /*CAN_WITH_RTL*/
	qends->endinfo.chipinfo.chip=chip;
	qends->endinfo.chipinfo.msgobj=obj;
	qends->notify=canqueue_notify_chip;

	DEBUGMSG("canqueue_ends_init_chip\n");
	return 0;
}


/**
 * canqueue_ends_done_chip - finalizing of the ends structure for CAN chips
 * @qends: pointer to ends structure
 *
 * Return Value: Function should be designed such way to not fail.
 */
int canqueue_ends_done_chip(struct canque_ends_t *qends)
{
	int delayed;
	
	/* Finish or kill all outgoing edges listed in inends */
	delayed=canqueue_ends_kill_inlist(qends, 1);
	/* Kill all incoming edges listed in outends */
	delayed|=canqueue_ends_kill_outlist(qends);

	return delayed;
}
