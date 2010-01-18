/* can_quekern.c - CAN message queues functions for the Linux kernel
 * Linux CAN-bus device driver.
 * New CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/can_queue.h"

//#define CAN_DEBUG

extern atomic_t edge_num_cnt;

#ifdef CAN_DEBUG
	#define DEBUGQUE(fmt,args...) can_printk(KERN_ERR "can_quekern (debug): " fmt,\
	##args)

#else
	#define DEBUGQUE(fmt,args...)
#endif

#define ERRMSGQUE(fmt,args...) can_printk(KERN_ERR "can_quekern: " fmt,\
	##args)


/* 
 * Modifies Tx message processing 
 *  0 .. local message processing disabled
 *  1 .. local messages disabled by default but can be enabled by canque_set_filt
 *  2 .. local messages enabled by default, can be disabled by canque_set_filt
 */
extern int processlocal;

void canque_dead_func(unsigned long data);

/* Support for dead ends structures left after client close */
can_spinlock_t canque_dead_func_lock;
LIST_HEAD(canque_dead_ends);
/* retrieved by list_entry(canque_dead_ends.next,struct canque_ends_t,dead_peers) */
LIST_HEAD(canque_dead_edges);
/* retrieved by list_entry(canque_dead_edges.next,struct canque_edge_t,inpeers) */
DECLARE_TASKLET(canque_dead_tl, canque_dead_func, 0);
/* activated by tasklet_schedule(&canque_dead_tl) */


static inline
struct canque_edge_t *canque_dead_edges_cut_first(void)
{
	can_spin_irqflags_t flags;
	struct canque_edge_t *edge;
	can_spin_lock_irqsave(&canque_dead_func_lock, flags);
	if(list_empty(&canque_dead_edges))
		edge=NULL;
	else{
		edge=list_entry(canque_dead_edges.next,struct canque_edge_t,inpeers);
		list_del(&edge->inpeers);
	}
	can_spin_unlock_irqrestore(&canque_dead_func_lock, flags);
	return edge;
}

void canque_dead_func(unsigned long data)
{
	can_spin_irqflags_t flags;
	struct canque_edge_t *qedge;
	struct canque_ends_t *qends;
	struct list_head *entry;

	while((qedge=canque_dead_edges_cut_first())){
		DEBUGQUE("edge %d disposed\n",qedge->edge_num);
	    #ifdef CAN_WITH_RTL
		if(canque_fifo_test_fl(&qedge->fifo,RTL_MEM)){
			canque_dispose_edge_rtl(qedge);
			continue;
		}
	    #endif /*CAN_WITH_RTL*/
		canque_fifo_done_kern(&qedge->fifo);
		kfree(qedge);
	}
	
	can_spin_lock_irqsave(&canque_dead_func_lock, flags);
	entry=canque_dead_ends.next;
	can_spin_unlock_irqrestore(&canque_dead_func_lock,flags);
	/* lock can be released there, because only one instance of canque_dead_tl
	   can run at once and all other functions add ends only to head */
	while(entry!=&canque_dead_ends){
		qends=list_entry(entry,struct canque_ends_t,dead_peers);
		entry=entry->next;
		if(!list_empty(&qends->inlist))
			continue;
		if(!list_empty(&qends->outlist))
			continue;
		can_spin_lock_irqsave(&canque_dead_func_lock, flags);
		list_del(&qends->dead_peers);
		can_spin_unlock_irqrestore(&canque_dead_func_lock,flags);
		DEBUGQUE("ends structure disposed\n");
	    #ifdef CAN_WITH_RTL
		if(qends->ends_flags&CAN_ENDSF_MEM_RTL){
			canque_ends_free_rtl(qends);
			continue;
		}
	    #endif /*CAN_WITH_RTL*/
		kfree(qends);
	}

}

static inline void canque_dead_tasklet_schedule(void)
{
    #ifdef CAN_WITH_RTL
	if(!rtl_rt_system_is_idle()){
		set_bit(CAN_RTL2LIN_PEND_DEAD_b,&canqueue_rtl2lin_pend);
		rtl_global_pend_irq (canqueue_rtl_irq);
		return;
	}
    #endif /*CAN_WITH_RTL*/

	tasklet_schedule(&canque_dead_tl);
}


void canque_edge_do_dead(struct canque_edge_t *edge)
{
	can_spin_irqflags_t flags;
	
	canque_notify_bothends(edge,CANQUEUE_NOTIFY_NOUSR);
    #ifdef CAN_WITH_RTL
	/* The problem of the above call is, that in RT-Linux to Linux notify
	   case is edge scheduled for delayed notify delivery, this needs
	   to be reflected there */
	if(atomic_read(&edge->edge_used)>0){
		can_spin_lock_irqsave(&edge->inends->ends_lock, flags);
		can_spin_lock(&edge->outends->ends_lock);
		if(atomic_read(&edge->edge_used)>0){
			/* left edge to live for a while, banshee comes again in a while */
			canque_fifo_clear_fl(&edge->fifo,DEAD);
			can_spin_unlock(&edge->outends->ends_lock);
			can_spin_unlock_irqrestore(&edge->inends->ends_lock, flags);
			can_printk(KERN_ERR "can_quertl (debug): canque_edge_do_dead postponed\n");
			return;
		}
		can_spin_unlock(&edge->outends->ends_lock);
		can_spin_unlock_irqrestore(&edge->inends->ends_lock, flags);
	}
    #endif /*CAN_WITH_RTL*/
	
	if(canqueue_disconnect_edge(edge)<0){
		ERRMSGQUE("canque_edge_do_dead: canqueue_disconnect_edge failed !!!\n");
		return;
	}

	can_spin_lock_irqsave(&canque_dead_func_lock, flags);
	list_add(&edge->inpeers,&canque_dead_edges);
	can_spin_unlock_irqrestore(&canque_dead_func_lock, flags);
	canque_dead_tasklet_schedule();
}



/*if(qends->ends_flags & CAN_ENDSF_DEAD){
	can_spin_lock_irqsave(&canque_dead_func_lock, flags);
	list_del(&qends->dead_peers);
	list_add(&qends->dead_peers,&canque_dead_ends);
	can_spin_unlock_irqrestore(&canque_dead_func_lock, flags);
	tasklet_schedule(&canque_dead_tl);
}*/


/**
 * canqueue_notify_kern - notification callback handler for Linux userspace clients
 * @qends: pointer to the callback side ends structure
 * @qedge: edge which invoked notification 
 * @what: notification type
 *
 * The notification event is handled directly by call of this function except case,
 * when called from RT-Linux context in mixed mode Linux/RT-Linux compilation.
 * It is not possible to directly call Linux kernel synchronization primitives
 * in such case. The notification request is postponed and signaled by @pending_inops flags
 * by call canqueue_rtl2lin_check_and_pend() function. 
 * The edge reference count is increased until until all pending notifications are processed.
 */
void canqueue_notify_kern(struct canque_ends_t *qends, struct canque_edge_t *qedge, int what)
{
	DEBUGQUE("canqueue_notify_kern for edge %d, use %d and event %d\n",
			qedge->edge_num,(int)atomic_read(&qedge->edge_used),what);

	/* delay event delivery for RT-Linux -> kernel notifications */
	if(canqueue_rtl2lin_check_and_pend(qends,qedge,what)){
		DEBUGQUE("canqueue_notify_kern postponed\n");
		return;
	}
	
	switch(what){
		case CANQUEUE_NOTIFY_EMPTY:
			wake_up(&qends->endinfo.fileinfo.emptyq);
			if(canque_fifo_test_and_clear_fl(&qedge->fifo, FREEONEMPTY))
				canque_edge_decref(qedge);
			break;
		case CANQUEUE_NOTIFY_SPACE:
			wake_up(&qends->endinfo.fileinfo.writeq);
		    #ifdef CAN_ENABLE_KERN_FASYNC
			/* Asynchronous I/O processing */
			kill_fasync(&qends->endinfo.fileinfo.fasync, SIGIO, POLL_OUT); 
		    #endif /*CAN_ENABLE_KERN_FASYNC*/
			break;
		case CANQUEUE_NOTIFY_PROC:
			wake_up(&qends->endinfo.fileinfo.readq);
		    #ifdef CAN_ENABLE_KERN_FASYNC
			/* Asynchronous I/O processing */
			kill_fasync(&qends->endinfo.fileinfo.fasync, SIGIO, POLL_IN); 
		    #endif /*CAN_ENABLE_KERN_FASYNC*/
			break;
		case CANQUEUE_NOTIFY_NOUSR:
			wake_up(&qends->endinfo.fileinfo.readq);
			wake_up(&qends->endinfo.fileinfo.writeq);
			wake_up(&qends->endinfo.fileinfo.emptyq);
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
 * canqueue_ends_init_kern - Linux userspace clients specific ends initialization
 * @qends: pointer to the callback side ends structure
 */
int canqueue_ends_init_kern(struct canque_ends_t *qends)
{
	canqueue_ends_init_gen(qends);
	qends->context=NULL;
	init_waitqueue_head(&qends->endinfo.fileinfo.readq);
	init_waitqueue_head(&qends->endinfo.fileinfo.writeq);
	init_waitqueue_head(&qends->endinfo.fileinfo.emptyq);
    #ifdef CAN_ENABLE_KERN_FASYNC
	qends->endinfo.fileinfo.fasync=NULL;
    #endif /*CAN_ENABLE_KERN_FASYNC*/
	
	qends->notify=canqueue_notify_kern;
	DEBUGQUE("canqueue_ends_init_kern\n");
	return 0;
}


/**
 * canque_get_inslot4id_wait_kern - find or wait for best outgoing edge and slot for given ID
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
int canque_get_inslot4id_wait_kern(struct canque_ends_t *qends,
	struct canque_edge_t **qedgep, struct canque_slot_t **slotp,
	int cmd, unsigned long id, int prio)
{
	int ret=-1;
	DEBUGQUE("canque_get_inslot4id_wait_kern for cmd %d, id %ld, prio %d\n",cmd,id,prio);
	wait_event_interruptible((qends->endinfo.fileinfo.writeq), 
		(ret=canque_get_inslot4id(qends,qedgep,slotp,cmd,id,prio))!=-1);
	return ret;
}

/**
 * canque_get_outslot_wait_kern - receive or wait for ready slot for given ends
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
int canque_get_outslot_wait_kern(struct canque_ends_t *qends,
	struct canque_edge_t **qedgep, struct canque_slot_t **slotp)
{
	int ret=-1;
	DEBUGQUE("canque_get_outslot_wait_kern\n");
	wait_event_interruptible((qends->endinfo.fileinfo.readq), 
		(ret=canque_test_outslot(qends,qedgep,slotp))!=-1);
	return ret;
}

/**
 * canque_sync_wait_kern - wait for all slots processing
 * @qends: ends structure belonging to calling communication object
 * @qedge: pointer to edge
 *
 * Functions waits for ends transition into empty state.
 * Return Value: Positive value indicates, that edge empty state has been reached.
 *	Negative or zero value informs about interrupted wait or other problem.
 */
int canque_sync_wait_kern(struct canque_ends_t *qends, struct canque_edge_t *qedge)
{
	int ret=-1;
	DEBUGQUE("canque_sync_wait_kern\n");
	wait_event_interruptible((qends->endinfo.fileinfo.emptyq), 
		(ret=canque_fifo_test_fl(&qedge->fifo,EMPTY)?1:0));
	return ret;
}


/**
 * canque_fifo_init_kern - initialize one CAN FIFO
 * @fifo: pointer to the FIFO structure
 * @slotsnr: number of requested slots
 *
 * Return Value: The negative value indicates, that there is no memory
 *	to allocate space for the requested number of the slots.
 */
int canque_fifo_init_kern(struct canque_fifo_t *fifo, int slotsnr)
{
	int size;
	if(!slotsnr) slotsnr=MAX_BUF_LENGTH;
	size=sizeof(struct canque_slot_t)*slotsnr;
	fifo->entry=kmalloc(size,GFP_KERNEL);
	if(!fifo->entry) return -1;
	fifo->slotsnr=slotsnr;
	return canque_fifo_init_slots(fifo);
}

/**
 * canque_fifo_done_kern - frees slots allocated for CAN FIFO
 * @fifo: pointer to the FIFO structure
 */
int canque_fifo_done_kern(struct canque_fifo_t *fifo)
{
	if(fifo->entry)
		kfree(fifo->entry);
	fifo->entry=NULL;
	return 1;
}


/**
 * canque_new_edge_kern - allocate new edge structure in the Linux kernel context
 * @slotsnr: required number of slots in the newly allocated edge structure
 *
 * Return Value: Returns pointer to allocated slot structure or %NULL if
 *	there is not enough memory to process operation.
 */
struct canque_edge_t *canque_new_edge_kern(int slotsnr)
{
	struct canque_edge_t *qedge;
	qedge = (struct canque_edge_t *)kmalloc(sizeof(struct canque_edge_t), GFP_KERNEL);
	if(qedge == NULL) return NULL;

	memset(qedge,0,sizeof(struct canque_edge_t));
	can_spin_lock_init(&qedge->fifo.fifo_lock);
	if(canque_fifo_init_kern(&qedge->fifo, slotsnr)<0){
		kfree(qedge);
		DEBUGQUE("canque_new_edge_kern failed\n");
		return NULL;
	}
	atomic_set(&qedge->edge_used,1);
	qedge->filtid = 0;
	qedge->filtmask = canque_filtid2internal(0l, (processlocal<2)? MSG_LOCAL:0);
	qedge->edge_prio = 0;
    #ifdef CAN_DEBUG
	/* not exactly clean, but enough for debugging */
	atomic_inc(&edge_num_cnt);
	qedge->edge_num=atomic_read(&edge_num_cnt);
    #endif /* CAN_DEBUG */
	DEBUGQUE("canque_new_edge_kern %d\n",qedge->edge_num);
	return qedge;
}

#ifdef USE_SYNC_DISCONNECT_EDGE_KERN

/*not included in doc
 * canqueue_disconnect_edge_kern - disconnect edge from communicating entities with wait
 * @qends: ends structure belonging to calling communication object
 * @qedge: pointer to edge
 *
 * Same as canqueue_disconnect_edge(), but tries to wait for state with zero
 * use counter.
 * Return Value: Negative value means, that edge is used and cannot
 *	be disconnected yet. Operation has to be delayed.
 */
int canqueue_disconnect_edge_kern(struct canque_ends_t *qends, struct canque_edge_t *qedge)
{
	canque_fifo_set_fl(&qedge->fifo,BLOCK);
	DEBUGQUE("canqueue_disconnect_edge_kern %d called\n",qedge->edge_num);
	if(!canque_fifo_test_and_set_fl(&qedge->fifo,DEAD)){
		canque_notify_bothends(qedge, CANQUEUE_NOTIFY_DEAD);
		
		if(atomic_read(&qedge->edge_used)>0)
			atomic_dec(&qedge->edge_used);

		DEBUGQUE("canqueue_disconnect_edge_kern %d waiting\n",qedge->edge_num);
		wait_event((qends->endinfo.fileinfo.emptyq), 
			(canqueue_disconnect_edge(qedge)>=0));

		/*set_current_state(TASK_UNINTERRUPTIBLE);*/
		/*schedule_timeout(HZ);*/
		return 0;
	} else {
		DEBUGQUE("canqueue_disconnect_edge_kern cannot set DEAD\n");
		return -1;
	}
}


int canqueue_disconnect_list_kern(struct canque_ends_t *qends, struct list_head *list)
{
	struct canque_edge_t *edge;
	can_spin_irqflags_t flags;
	for(;;){
		can_spin_lock_irqsave(&qends->ends_lock,flags);
		if(list_empty(list)){
			can_spin_unlock_irqrestore(&qends->ends_lock,flags);
			return 0;
		}
		if(list == &qends->inlist)
			edge=list_entry(list->next,struct canque_edge_t,inpeers);
		else
			edge=list_entry(list->next,struct canque_edge_t,outpeers);
		atomic_inc(&edge->edge_used);
		can_spin_unlock_irqrestore(&qends->ends_lock,flags);
		if(canqueue_disconnect_edge_kern(qends, edge)>=0) {
			/* Free edge memory */
			canque_fifo_done_kern(&edge->fifo);
			kfree(edge);
		}else{
			canque_notify_bothends(edge, CANQUEUE_NOTIFY_DEAD_WANTED);
			canque_edge_decref(edge);
			DEBUGQUE("canqueue_disconnect_list_kern in troubles\n");
			DEBUGQUE("the edge %d has usage count %d and flags %ld\n",edge->edge_num,atomic_read(&edge->edge_used),edge->fifo.fifo_flags);
			return -1;
		}
	}
}

#endif /*USE_SYNC_DISCONNECT_EDGE_KERN*/


int canqueue_ends_sync_all_kern(struct canque_ends_t *qends)
{
	struct canque_edge_t *qedge;
	
	canque_for_each_inedge(qends, qedge){
		DEBUGQUE("canque_sync_wait_kern called for edge %d\n",qedge->edge_num);
		canque_sync_wait_kern(qends, qedge);
	}
	return 0;
}


void canqueue_ends_dispose_postpone(struct canque_ends_t *qends)
{
	can_spin_irqflags_t flags;

	can_spin_lock_irqsave(&canque_dead_func_lock, flags);
	qends->ends_flags |= CAN_ENDSF_DEAD;
	list_add(&qends->dead_peers,&canque_dead_ends);
	can_spin_unlock_irqrestore(&canque_dead_func_lock, flags);
	canque_dead_tasklet_schedule();
}


/**
 * canqueue_ends_dispose_kern - finalizing of the ends structure for Linux kernel clients
 * @qends: pointer to ends structure
 * @sync: flag indicating, that user wants to wait for processing of all remaining
 *	messages
 *
 * Return Value: Function should be designed such way to not fail.
 */
int canqueue_ends_dispose_kern(struct canque_ends_t *qends, int sync)
{
	int delayed;

	DEBUGQUE("canqueue_ends_dispose_kern\n");
	canqueue_block_inlist(qends);
	canqueue_block_outlist(qends);

	/*Wait for sending of all pending messages in the output FIFOs*/
	if(sync)
		canqueue_ends_sync_all_kern(qends);
	
	/* Finish or kill all outgoing edges listed in inends */
	delayed=canqueue_ends_kill_inlist(qends, 1);
	/* Kill all incoming edges listed in outends */
	delayed|=canqueue_ends_kill_outlist(qends);

	wake_up(&qends->endinfo.fileinfo.readq);
	wake_up(&qends->endinfo.fileinfo.writeq);
	wake_up(&qends->endinfo.fileinfo.emptyq);

	if(delayed){
		canqueue_ends_dispose_postpone(qends);

		DEBUGQUE("canqueue_ends_dispose_kern delayed\n");
		return 1;
	}

	kfree(qends);
	DEBUGQUE("canqueue_ends_dispose_kern finished\n");
	return 0;
}

void canqueue_kern_initialize()
{
	can_spin_lock_init(&canque_dead_func_lock);
}
