/* can_quertl.c - CAN message queues functions for the RT-Linux
 * Linux CAN-bus device driver.
 * New CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

#ifdef CAN_WITH_RTL

#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/can_queue.h"

#include <rtl_malloc.h>

/* 
 * Modifies Tx message processing 
 *  0 .. local message processing disabled
 *  1 .. local messages disabled by default but can be enabled by canque_set_filt
 *  2 .. local messages enabled by default, can be disabled by canque_set_filt
 */
extern int processlocal;


#define CANQUE_PENDOPS_LIMIT 15
#define CANQUE_PENDOPS_MASK  ((1<<CANQUE_PENDOPS_LIMIT)-1)

struct list_head canque_pending_edges_list;
can_spinlock_t canque_pending_edges_lock;
unsigned long canqueue_rtl2lin_pend;

int canqueue_rtl_irq = 0;

void
canqueue_rtl2lin_handler(int irq, void *ignore, struct pt_regs *ignoreregs)
{
	can_spin_irqflags_t flags;
	struct canque_edge_t *qedge;
	unsigned pending_inops;
	unsigned pending_outops;
	int i;
	
	can_spin_lock_irqsave (&canque_pending_edges_lock, flags);

	while(!list_empty(&canque_pending_edges_list)){
		qedge=list_entry(canque_pending_edges_list.next,struct canque_edge_t,pending_peers);
		list_del(&qedge->pending_peers);
		canque_fifo_clear_fl(&qedge->fifo, NOTIFYPEND);
		pending_inops=qedge->pending_inops;
		qedge->pending_inops=0;
		pending_outops=qedge->pending_outops;
		qedge->pending_outops=0;
		can_spin_unlock_irqrestore (&canque_pending_edges_lock, flags);

		if(pending_outops & ~CANQUE_PENDOPS_MASK){
			pending_outops &= CANQUE_PENDOPS_MASK;
			canque_notify_outends(qedge,CANQUEUE_NOTIFY_ERROR | qedge->fifo.error_code);
		}
		for(i=0;pending_outops;i++,pending_outops>>=1){
			if(pending_outops&1)
				canque_notify_outends(qedge,i);
		}
		if(pending_inops & ~CANQUE_PENDOPS_MASK){
			pending_inops &= CANQUE_PENDOPS_MASK;
			canque_notify_inends(qedge,CANQUEUE_NOTIFY_ERROR | qedge->fifo.error_code);
		}
		for(i=0;pending_inops;i++,pending_inops>>=1){
			if(pending_inops&1)
				canque_notify_inends(qedge,i);
		}
		
		canque_edge_decref(qedge);
		can_spin_lock_irqsave (&canque_pending_edges_lock, flags);
	}

	can_spin_unlock_irqrestore (&canque_pending_edges_lock, flags);

	if(test_and_clear_bit(CAN_RTL2LIN_PEND_DEAD_b,&canqueue_rtl2lin_pend))
		tasklet_schedule(&canque_dead_tl);

	return;
}


/**
 * canqueue_rtl2lin_check_and_pend - postpones edge notification if called from RT-Linux 
 * @qends: notification target ends
 * @qedge: edge delivering notification
 * @what:  notification type
 *
 * Return Value: if called from Linux context, returns 0 and lefts notification processing
 *		on caller responsibility. If called from RT-Linux contexts, schedules postponed
 *		event delivery and returns 1
 */
int canqueue_rtl2lin_check_and_pend(struct canque_ends_t *qends,
			 struct canque_edge_t *qedge, int what)
{
	can_spin_irqflags_t flags;

	if(rtl_rt_system_is_idle()) return 0;

	can_spin_lock_irqsave (&canque_pending_edges_lock, flags);
	
	if(what>CANQUE_PENDOPS_LIMIT) what=CANQUE_PENDOPS_LIMIT;

	if(qends == qedge->inends) {
		set_bit(what,&qedge->pending_inops);
	} else if(qends == qedge->outends) {
		set_bit(what,&qedge->pending_outops);
	}

	if(!canque_fifo_test_and_set_fl(&qedge->fifo, NOTIFYPEND)){
		canque_edge_incref(qedge);
		list_add_tail(&qedge->pending_peers,&canque_pending_edges_list);
		rtl_global_pend_irq (canqueue_rtl_irq);
	}

	can_spin_unlock_irqrestore (&canque_pending_edges_lock, flags);
	
	return 1;

}


/**
 * canque_get_inslot4id_wait_rtl - find or wait for best outgoing edge and slot for given ID
 * @qends: ends structure belonging to calling communication object
 * @qedgep: place to store pointer to found edge
 * @slotp: place to store pointer to  allocated slot
 * @cmd: command type for slot
 * @id: communication ID of message to send into edge
 * @prio: optional priority of message
 *
 * Same as canque_get_inslot4id(), except, that it waits for free slot
 * in case, that queue is full. Function is specific for Linux userspace clients.
 * Return Value: If there is no usable edge negative value is returned.
 */
int canque_get_inslot4id_wait_rtl(struct canque_ends_t *qends,
	struct canque_edge_t **qedgep, struct canque_slot_t **slotp,
	int cmd, unsigned long id, int prio)
{
	rtl_irqstate_t flags;
	int ret;
	unsigned old_age;
	rtl_sigset_t sigset;
	
	old_age=atomic_read(&qends->endinfo.rtlinfo.rtl_writeq_age);
	while((ret=canque_get_inslot4id(qends,qedgep,slotp,cmd,id,prio))==-1){
		rtl_sigemptyset(&sigset);
		rtl_spin_lock_irqsave(&qends->endinfo.rtlinfo.rtl_lock, flags);
		if(old_age == atomic_read(&qends->endinfo.rtlinfo.rtl_writeq_age))
			sigset=rtl_wait_sleep(&qends->endinfo.rtlinfo.rtl_writeq, &qends->endinfo.rtlinfo.rtl_lock);
		rtl_spin_unlock_irqrestore(&qends->endinfo.rtlinfo.rtl_lock, flags);
		if(RTL_SIGINTR(&sigset))
			return -1;
		old_age=atomic_read(&qends->endinfo.rtlinfo.rtl_writeq_age);
	}
	
	return ret;
}


/**
 * canque_get_outslot_wait_rtl - receive or wait for ready slot for given ends
 * @qends: ends structure belonging to calling communication object
 * @qedgep: place to store pointer to found edge
 * @slotp: place to store pointer to received slot
 *
 * The same as canque_test_outslot(), except it waits in the case, that there is
 * no ready slot for given ends. Function is specific for Linux userspace clients.
 * Return Value: Negative value informs, that there is no ready output
 *	slot for given ends. Positive value is equal to the command
 *	slot has been allocated by the input side.
 */
int canque_get_outslot_wait_rtl(struct canque_ends_t *qends,
	struct canque_edge_t **qedgep, struct canque_slot_t **slotp)
{
	rtl_irqstate_t flags;
	int ret;
	unsigned old_age;
	rtl_sigset_t sigset;
	
	old_age=atomic_read(&qends->endinfo.rtlinfo.rtl_readq_age);
	while((ret=canque_test_outslot(qends,qedgep,slotp))==-1){
		rtl_sigemptyset(&sigset);
		rtl_spin_lock_irqsave(&qends->endinfo.rtlinfo.rtl_lock, flags);
		if(old_age == atomic_read(&qends->endinfo.rtlinfo.rtl_readq_age))
			sigset=rtl_wait_sleep(&qends->endinfo.rtlinfo.rtl_readq, &qends->endinfo.rtlinfo.rtl_lock);
		rtl_spin_unlock_irqrestore(&qends->endinfo.rtlinfo.rtl_lock, flags);
		if(RTL_SIGINTR(&sigset))
			return -1;
		old_age=atomic_read(&qends->endinfo.rtlinfo.rtl_readq_age);
	}
	return ret;
}


/**
 * canque_sync_wait_rtl - wait for all slots processing
 * @qends: ends structure belonging to calling communication object
 * @qedge: pointer to edge
 *
 * Functions waits for ends transition into empty state.
 * Return Value: Positive value indicates, that edge empty state has been reached.
 *	Negative or zero value informs about interrupted wait or other problem.
 */
int canque_sync_wait_rtl(struct canque_ends_t *qends, struct canque_edge_t *qedge)
{
	rtl_irqstate_t flags;
	int ret;
	unsigned old_age;
	rtl_sigset_t sigset;
	
	old_age=atomic_read(&qends->endinfo.rtlinfo.rtl_emptyq_age);
	while(!(ret=canque_fifo_test_fl(&qedge->fifo,EMPTY)?1:0)){
		rtl_sigemptyset(&sigset);
		rtl_spin_lock_irqsave(&qends->endinfo.rtlinfo.rtl_lock, flags);
		if(old_age == atomic_read(&qends->endinfo.rtlinfo.rtl_emptyq_age))
			sigset=rtl_wait_sleep(&qends->endinfo.rtlinfo.rtl_emptyq, &qends->endinfo.rtlinfo.rtl_lock);
		rtl_spin_unlock_irqrestore(&qends->endinfo.rtlinfo.rtl_lock, flags);
		if(RTL_SIGINTR(&sigset))
			return -1;
		old_age=atomic_read(&qends->endinfo.rtlinfo.rtl_emptyq_age);
	}
	
	return ret;
}


/**
 * canque_fifo_init_rtl - initialize one CAN FIFO
 * @fifo: pointer to the FIFO structure
 * @slotsnr: number of requested slots
 *
 * Return Value: The negative value indicates, that there is no memory
 *	to allocate space for the requested number of the slots.
 */
int canque_fifo_init_rtl(struct canque_fifo_t *fifo, int slotsnr)
{
	int size;
	if(!slotsnr) slotsnr=MAX_BUF_LENGTH;
	size=sizeof(struct canque_slot_t)*slotsnr;
	fifo->entry=rt_malloc(size);
	if(!fifo->entry) return -1;
	fifo->slotsnr=slotsnr;
	return canque_fifo_init_slots(fifo);
}


/**
 * canque_fifo_done_rtl - frees slots allocated for CAN FIFO
 * @fifo: pointer to the FIFO structure
 */
int canque_fifo_done_rtl(struct canque_fifo_t *fifo)
{
	if(fifo->entry)
		rt_free(fifo->entry);
	fifo->entry=NULL;
	return 1;
}

void canque_dispose_edge_rtl(struct canque_edge_t *qedge)
{
	canque_fifo_done_rtl(&qedge->fifo);
	rt_free(qedge);
}

/**
 * canque_new_edge_rtl - allocate new edge structure in the RT-Linux context
 * @slotsnr: required number of slots in the newly allocated edge structure
 *
 * Return Value: Returns pointer to allocated slot structure or %NULL if
 *	there is not enough memory to process operation.
 */
struct canque_edge_t *canque_new_edge_rtl(int slotsnr)
{
	struct canque_edge_t *qedge;
	qedge = (struct canque_edge_t *)rt_malloc(sizeof(struct canque_edge_t));
	if(qedge == NULL) return NULL;

	memset(qedge,0,sizeof(struct canque_edge_t));
	can_spin_lock_init(&qedge->fifo.fifo_lock);
	canque_fifo_set_fl(&qedge->fifo,RTL_MEM);
	if(canque_fifo_init_rtl(&qedge->fifo, slotsnr)<0){
		rt_free(qedge);
		return NULL;
	}
	atomic_set(&qedge->edge_used,1);
	qedge->filtid = 0;
	qedge->filtmask = canque_filtid2internal(0l, (processlocal<2)? MSG_LOCAL:0);
	qedge->edge_prio = 0;
    #if defined(CAN_DEBUG) && 0
	/* not exactly clean, but enough for debugging */
	atomic_inc(&edge_num_cnt);
	qedge->edge_num=atomic_read(&edge_num_cnt);
    #endif /* CAN_DEBUG */
	return qedge;
}

void canque_ends_free_rtl(struct canque_ends_t *qends)
{
	rt_free(qends);
}


/**
 * canqueue_notify_rtl - notification callback handler for Linux userspace clients
 * @qends: pointer to the callback side ends structure
 * @qedge: edge which invoked notification 
 * @what: notification type
 */
void canqueue_notify_rtl(struct canque_ends_t *qends, struct canque_edge_t *qedge, int what)
{
	rtl_irqstate_t flags;
	
	switch(what){
		case CANQUEUE_NOTIFY_EMPTY:
			rtl_spin_lock_irqsave(&qends->endinfo.rtlinfo.rtl_lock, flags);
			atomic_inc(&qends->endinfo.rtlinfo.rtl_emptyq_age);
			rtl_wait_wakeup(&qends->endinfo.rtlinfo.rtl_emptyq);
			rtl_spin_unlock_irqrestore(&qends->endinfo.rtlinfo.rtl_lock, flags);
			if(canque_fifo_test_and_clear_fl(&qedge->fifo, FREEONEMPTY))
				canque_edge_decref(qedge);
			break;
		case CANQUEUE_NOTIFY_SPACE:
			rtl_spin_lock_irqsave(&qends->endinfo.rtlinfo.rtl_lock, flags);
			atomic_inc(&qends->endinfo.rtlinfo.rtl_writeq_age);
			rtl_wait_wakeup(&qends->endinfo.rtlinfo.rtl_writeq);
			rtl_spin_unlock_irqrestore(&qends->endinfo.rtlinfo.rtl_lock, flags);
			break;
		case CANQUEUE_NOTIFY_PROC:
			rtl_spin_lock_irqsave(&qends->endinfo.rtlinfo.rtl_lock, flags);
			atomic_inc(&qends->endinfo.rtlinfo.rtl_readq_age);
			rtl_wait_wakeup(&qends->endinfo.rtlinfo.rtl_readq);
			rtl_spin_unlock_irqrestore(&qends->endinfo.rtlinfo.rtl_lock, flags);
			break;
		case CANQUEUE_NOTIFY_NOUSR:
			rtl_spin_lock_irqsave(&qends->endinfo.rtlinfo.rtl_lock, flags);

			atomic_inc(&qends->endinfo.rtlinfo.rtl_readq_age);
			rtl_wait_wakeup(&qends->endinfo.rtlinfo.rtl_readq);

			atomic_inc(&qends->endinfo.rtlinfo.rtl_writeq_age);
			rtl_wait_wakeup(&qends->endinfo.rtlinfo.rtl_writeq);

			atomic_inc(&qends->endinfo.rtlinfo.rtl_emptyq_age);
			rtl_wait_wakeup(&qends->endinfo.rtlinfo.rtl_emptyq);

			rtl_spin_unlock_irqrestore(&qends->endinfo.rtlinfo.rtl_lock, flags);
			break;
		case CANQUEUE_NOTIFY_DEAD_WANTED:
		case CANQUEUE_NOTIFY_DEAD:
			if(canque_fifo_test_and_clear_fl(&qedge->fifo, READY))
				canque_edge_decref(qedge);
			break;
		case CANQUEUE_NOTIFY_ATTACH:
			break;
	}
}


/**
 * canqueue_ends_init_rtl - RT-Linux clients specific ends initialization
 * @qends: pointer to the callback side ends structure
 */
int canqueue_ends_init_rtl(struct canque_ends_t *qends)
{
	canqueue_ends_init_gen(qends);
	qends->context=NULL;
    	rtl_spin_lock_init(&(qends->endinfo.rtlinfo.rtl_lock));
	rtl_wait_init(&(qends->endinfo.rtlinfo.rtl_readq));
	rtl_wait_init(&(qends->endinfo.rtlinfo.rtl_writeq));
	rtl_wait_init(&(qends->endinfo.rtlinfo.rtl_emptyq));
	
	qends->notify=canqueue_notify_rtl;
	qends->endinfo.rtlinfo.pend_flags=0;
	return 0;
}

/**
 * canqueue_ends_dispose_rtl - finalizing of the ends structure for Linux kernel clients
 * @qends: pointer to ends structure
 * @sync: flag indicating, that user wants to wait for processing of all remaining
 *	messages
 *
 * Return Value: Function should be designed such way to not fail.
 */
int canqueue_ends_dispose_rtl(struct canque_ends_t *qends, int sync)
{
	rtl_irqstate_t flags;
	int delayed;

	canqueue_block_inlist(qends);
	canqueue_block_outlist(qends);

	/*Wait for sending of all pending messages in the output FIFOs*/
	/*if(sync)
		canqueue_ends_sync_all_rtl(qends);*/
	
	/* Finish or kill all outgoing edges listed in inends */
	delayed=canqueue_ends_kill_inlist(qends, 1);
	/* Kill all incoming edges listed in outends */
	delayed|=canqueue_ends_kill_outlist(qends);

	rtl_spin_lock_irqsave(&qends->endinfo.rtlinfo.rtl_lock, flags);
	rtl_wait_wakeup(&qends->endinfo.rtlinfo.rtl_readq);
	rtl_wait_wakeup(&qends->endinfo.rtlinfo.rtl_writeq);
	rtl_wait_wakeup(&qends->endinfo.rtlinfo.rtl_emptyq);
	rtl_spin_unlock_irqrestore(&qends->endinfo.rtlinfo.rtl_lock, flags);

	if(delayed || !(qends->ends_flags&CAN_ENDSF_MEM_RTL)){
		canqueue_ends_dispose_postpone(qends);

		return 1;
	}

	canque_ends_free_rtl(qends);
	return 0;
}



/**
 * canqueue_rtl_initialize - initialization of global RT-Linux specific features
 */
void canqueue_rtl_initialize(void)
{
	INIT_LIST_HEAD(&canque_pending_edges_list);
	can_spin_lock_init(&canque_pending_edges_lock);

	canqueue_rtl_irq = rtl_get_soft_irq (canqueue_rtl2lin_handler, "rtl_canqueue_irq");
}


/**
 * canqueue_rtl_done - finalization of glopal RT-Linux specific features
 */
void canqueue_rtl_done(void)
{
	rtl_free_soft_irq (canqueue_rtl_irq);

}


#endif /*CAN_WITH_RTL*/
