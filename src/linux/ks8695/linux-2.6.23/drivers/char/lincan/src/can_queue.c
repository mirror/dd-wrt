/* can_queue.c - CAN message queues
 * Linux CAN-bus device driver.
 * New CAN queues by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

#include "../include/can.h"
#include "../include/can_sysdep.h"
#include "../include/can_queue.h"

/* 
 * Modifies Tx message processing 
 *  0 .. local message processing disabled
 *  1 .. local messages disabled by default but can be enabled by canque_set_filt
 *  2 .. local messages enabled by default, can be disabled by canque_set_filt
 */
extern int processlocal;

atomic_t edge_num_cnt;

//#define CAN_DEBUG
#undef CAN_DEBUG

#ifdef CAN_DEBUG
	#define DEBUGQUE(fmt,args...) can_printk(KERN_ERR "can_queue (debug): " fmt,\
	##args)

#else
	#define DEBUGQUE(fmt,args...)
#endif

#define CANQUE_ROUNDROB 1


/**
 * canque_fifo_flush_slots - free all ready slots from the FIFO
 * @fifo: pointer to the FIFO structure
 *
 * The caller should be prepared to handle situations, when some
 * slots are held by input or output side slots processing.
 * These slots cannot be flushed or their processing interrupted.
 *
 * Return Value: The nonzero value indicates, that queue has not been
 *	empty before the function call.
 */
int canque_fifo_flush_slots(struct canque_fifo_t *fifo)
{
	int ret;
	can_spin_irqflags_t flags;
	struct canque_slot_t *slot;
	can_spin_lock_irqsave(&fifo->fifo_lock, flags);
	slot=fifo->head;
	if(slot){
		*fifo->tail=fifo->flist;
		fifo->flist=slot;
		fifo->head=NULL;
		fifo->tail=&fifo->head;
	}
	canque_fifo_clear_fl(fifo,FULL);
	ret=canque_fifo_test_and_set_fl(fifo,EMPTY)?0:1;
	can_spin_unlock_irqrestore(&fifo->fifo_lock, flags);
	return ret;
}


/**
 * canque_fifo_init_slots - initializes slot chain of one CAN FIFO
 * @fifo: pointer to the FIFO structure
 *
 * Return Value: The negative value indicates, that there is no memory
 *	to allocate space for the requested number of the slots.
 */
int canque_fifo_init_slots(struct canque_fifo_t *fifo)
{
	struct canque_slot_t *slot;
	int slotsnr=fifo->slotsnr;
	if(!fifo->entry || !slotsnr) return -1;
	slot=fifo->entry;
	fifo->flist=slot;
	while(--slotsnr){
		slot->next=slot+1;
		slot++;
	}
	slot->next=NULL;
	fifo->head=NULL;
	fifo->tail=&fifo->head;
	canque_fifo_set_fl(fifo,EMPTY);
	return 1;
}

/* atomic_dec_and_test(&qedge->edge_used);
 void atomic_inc(&qedge->edge_used);
 list_add_tail(struct list_head *new, struct list_head *head)
 list_for_each(edge,qends->inlist);
 list_entry(ptr, type, member)
*/

void __canque_edge_decref(struct canque_edge_t *edge)
{
	__canque_edge_decref_body(edge);
}

/**
 * canque_get_inslot - finds one outgoing edge and allocates slot from it
 * @qends: ends structure belonging to calling communication object
 * @qedgep: place to store pointer to found edge
 * @slotp: place to store pointer to  allocated slot
 * @cmd: command type for slot
 *
 * Function looks for the first non-blocked outgoing edge in @qends structure
 * and tries to allocate slot from it.
 * Return Value: If there is no usable edge or there is no free slot in edge
 *	negative value is returned.
 */
int canque_get_inslot(struct canque_ends_t *qends,
	struct canque_edge_t **qedgep, struct canque_slot_t **slotp, int cmd)
{
	int ret=-2;
	struct canque_edge_t *edge;
	
	edge=canque_first_inedge(qends);
	if(edge){
		if(!canque_fifo_test_fl(&edge->fifo,BLOCK)){
			ret=canque_fifo_get_inslot(&edge->fifo, slotp, cmd);
			if(ret>0){
				*qedgep=edge;
				DEBUGQUE("canque_get_inslot cmd=%d found edge %d\n",cmd,edge->edge_num);
				return ret;

			}
		}
		canque_edge_decref(edge);
	}
	*qedgep=NULL;
	DEBUGQUE("canque_get_inslot cmd=%d failed\n",cmd);
	return ret;
}

/**
 * canque_get_inslot4id - finds best outgoing edge and slot for given ID
 * @qends: ends structure belonging to calling communication object
 * @qedgep: place to store pointer to found edge
 * @slotp: place to store pointer to  allocated slot
 * @cmd: command type for slot
 * @id: communication ID of message to send into edge
 * @prio: optional priority of message
 *
 * Function looks for the non-blocked outgoing edge accepting messages
 * with given ID. If edge is found, slot is allocated from that edge.
 * The edges with non-zero mask are preferred over edges open to all messages.
 * If more edges with mask accepts given message ID, the edge with
 * highest priority below or equal to required priority is selected.
 * Return Value: If there is no usable edge or there is no free slot in edge
 *	negative value is returned.
 */
int canque_get_inslot4id(struct canque_ends_t *qends,
	struct canque_edge_t **qedgep, struct canque_slot_t **slotp,
	int cmd, unsigned long id, int prio)
{
	int ret=-2;
	struct canque_edge_t *edge, *bestedge=NULL;
	
	canque_for_each_inedge(qends, edge){
		if(canque_fifo_test_fl(&edge->fifo,BLOCK))
			continue;
		if((id^edge->filtid)&edge->filtmask)
			continue;
		if(bestedge){
			if(bestedge->filtmask){
				if (!edge->filtmask) continue;
			} else {
				if(edge->filtmask){
					canque_edge_decref(bestedge);
					bestedge=edge;
					canque_edge_incref(bestedge);
					continue;
				}
			}
			if(bestedge->edge_prio<edge->edge_prio){
				if(edge->edge_prio>prio) continue;
			} else {
				if(bestedge->edge_prio<=prio) continue;
			}
			canque_edge_decref(bestedge);
		}
		bestedge=edge;
		canque_edge_incref(bestedge);
	}
	if((edge=bestedge)!=NULL){
		ret=canque_fifo_get_inslot(&edge->fifo, slotp, cmd);
		if(ret>0){
			*qedgep=edge;
			DEBUGQUE("canque_get_inslot4id cmd=%d id=%ld prio=%d found edge %d\n",cmd,id,prio,edge->edge_num);
			return ret;
		}
		canque_edge_decref(bestedge);
	}
	*qedgep=NULL;
	DEBUGQUE("canque_get_inslot4id cmd=%d id=%ld prio=%d failed\n",cmd,id,prio);
	return ret;
}


/**
 * canque_put_inslot - schedules filled slot for processing
 * @qends: ends structure belonging to calling communication object
 * @qedge: edge slot belong to
 * @slot: pointer to the prepared slot
 *
 * Puts slot previously acquired by canque_get_inslot() or canque_get_inslot4id()
 * function call into FIFO queue and activates edge processing if needed.
 * Return Value: Positive value informs, that activation of output end
 *	has been necessary
 */
int canque_put_inslot(struct canque_ends_t *qends,
	struct canque_edge_t *qedge, struct canque_slot_t *slot)
{
	int ret;
	ret=canque_fifo_put_inslot(&qedge->fifo,slot);
	if(ret) {
		canque_activate_edge(qends,qedge);
		canque_notify_outends(qedge,CANQUEUE_NOTIFY_PROC);
	}
	canque_edge_decref(qedge);
	DEBUGQUE("canque_put_inslot for edge %d returned %d\n",qedge->edge_num,ret);
	return ret;
}

/**
 * canque_abort_inslot - aborts preparation of the message in the slot
 * @qends: ends structure belonging to calling communication object
 * @qedge: edge slot belong to
 * @slot: pointer to the previously allocated slot
 *
 * Frees slot previously acquired by canque_get_inslot() or canque_get_inslot4id()
 * function call. Used when message copying into slot fails.
 * Return Value: Positive value informs, that queue full state has been negated.
 */
int canque_abort_inslot(struct canque_ends_t *qends,
	struct canque_edge_t *qedge, struct canque_slot_t *slot)
{
	int ret;
	ret=canque_fifo_abort_inslot(&qedge->fifo,slot);
	if(ret) {
		canque_notify_outends(qedge,CANQUEUE_NOTIFY_SPACE);
	}
	canque_edge_decref(qedge);
	DEBUGQUE("canque_abort_inslot for edge %d returned %d\n",qedge->edge_num,ret);
	return ret;
}

/**
 * canque_filter_msg2edges - sends message into all edges which accept its ID
 * @qends: ends structure belonging to calling communication object
 * @msg: pointer to CAN message
 *
 * Sends message to all outgoing edges connected to the given ends, which accepts
 * message communication ID.
 * Return Value: Returns number of edges message has been send to
 */
int canque_filter_msg2edges(struct canque_ends_t *qends, struct canmsg_t *msg)
{
	int destnr=0;
	int ret;
	unsigned long msgid;
	struct canque_edge_t *edge;
	struct canque_slot_t *slot;
	
	DEBUGQUE("canque_filter_msg2edges for msg ID 0x%08lx and flags 0x%02x\n",
			msg->id, msg->flags);
	msgid = canque_filtid2internal(msg->id, msg->flags);

	canque_for_each_inedge(qends, edge) {
		if(canque_fifo_test_fl(&edge->fifo,BLOCK))
			continue;
		if((msgid^edge->filtid)&edge->filtmask)
			continue;
		ret=canque_fifo_get_inslot(&edge->fifo, &slot, 0);
		if(ret>0){
			slot->msg=*msg;
			destnr++;
			ret=canque_fifo_put_inslot(&edge->fifo,slot);
			if(ret) {
				canque_activate_edge(qends,edge);
				canque_notify_outends(edge,CANQUEUE_NOTIFY_PROC);
			}

		}
	}
	DEBUGQUE("canque_filter_msg2edges sent msg ID %ld to %d edges\n",msg->id,destnr);
	return destnr;
}

/**
 * canque_test_outslot - test and retrieve ready slot for given ends
 * @qends: ends structure belonging to calling communication object
 * @qedgep: place to store pointer to found edge
 * @slotp: place to store pointer to received slot
 *
 * Function takes highest priority active incoming edge and retrieves
 * oldest ready slot from it.
 * Return Value: Negative value informs, that there is no ready output
 *	slot for given ends. Positive value is equal to the command
 *	slot has been allocated by the input side.
 */
int canque_test_outslot(struct canque_ends_t *qends,
	struct canque_edge_t **qedgep, struct canque_slot_t **slotp)
{
	can_spin_irqflags_t flags;
	int prio;
	struct canque_edge_t *edge;
	int ret;
	
	can_spin_lock_irqsave(&qends->ends_lock, flags);
	for(prio=CANQUEUE_PRIO_NR;--prio>=0;){
		while(!list_empty(&qends->active[prio])){
			edge=list_entry(qends->active[prio].next,struct canque_edge_t,activepeers);
			if(!canque_fifo_test_fl(&edge->fifo,DEAD)) {
				/* The first test on unlocked FIFO */
				if(canque_fifo_test_fl(&edge->fifo,EMPTY)) {
					can_spin_lock(&edge->fifo.fifo_lock);
					/* Test has to be repeated to ensure that EMPTY
					   state has not been nagated when locking FIFO */
					if(canque_fifo_test_fl(&edge->fifo,EMPTY)) {
						canque_fifo_set_fl(&edge->fifo,INACTIVE);
						list_del(&edge->activepeers);
						list_add(&edge->activepeers,&qends->idle);
						can_spin_unlock(&edge->fifo.fifo_lock);
						continue;
					}
					can_spin_unlock(&edge->fifo.fifo_lock);
				}
				canque_edge_incref(edge);
				can_spin_unlock_irqrestore(&qends->ends_lock, flags);
				*qedgep=edge;
				DEBUGQUE("canque_test_outslot found edge %d\n",edge->edge_num);
				ret=canque_fifo_test_outslot(&edge->fifo, slotp);
				if(ret>=0)
					return ret;

				canque_edge_decref(edge);
				can_spin_lock_irqsave(&qends->ends_lock, flags);
			} else {
				can_spin_lock(&edge->fifo.fifo_lock);
				canque_fifo_set_fl(&edge->fifo,INACTIVE);
				list_del(&edge->activepeers);
				list_add(&edge->activepeers,&qends->idle);
				can_spin_unlock(&edge->fifo.fifo_lock);
			}
		}
	}
	can_spin_unlock_irqrestore(&qends->ends_lock, flags);
	*qedgep=NULL;
	DEBUGQUE("canque_test_outslot no ready slot\n");
	return -1;
}

/**
 * canque_free_outslot - frees processed output slot
 * @qends: ends structure belonging to calling communication object
 * @qedge: edge slot belong to
 * @slot: pointer to the processed slot
 *
 * Function releases processed slot previously acquired by canque_test_outslot()
 * function call.
 * Return Value: Return value informs if input side has been notified
 *	to know about change of edge state
 */
int canque_free_outslot(struct canque_ends_t *qends,
	struct canque_edge_t *qedge, struct canque_slot_t *slot)
{
	int ret;
	can_spin_irqflags_t flags;
	ret=canque_fifo_free_outslot(&qedge->fifo, slot);
	if(ret&CAN_FIFOF_EMPTY){
	  	canque_notify_inends(qedge,CANQUEUE_NOTIFY_EMPTY);
	}
	if(ret&CAN_FIFOF_FULL)
	  	canque_notify_inends(qedge,CANQUEUE_NOTIFY_SPACE);
	can_spin_lock_irqsave(&qends->ends_lock, flags);
	if((ret&CAN_FIFOF_EMPTY) || CANQUE_ROUNDROB ){
		can_spin_lock(&qedge->fifo.fifo_lock);
		if(canque_fifo_test_fl(&qedge->fifo,EMPTY)){
			canque_fifo_set_fl(&qedge->fifo,INACTIVE);
			list_del(&qedge->activepeers);
			list_add(&qedge->activepeers,&qends->idle);
		} else{
			list_del(&qedge->activepeers);
			list_add_tail(&qedge->activepeers,&qends->active[qedge->edge_prio]);
		}
		can_spin_unlock(&qedge->fifo.fifo_lock);
	}
	can_spin_unlock_irqrestore(&qends->ends_lock, flags);
	canque_edge_decref(qedge);
	DEBUGQUE("canque_free_outslot for edge %d returned %d\n",qedge->edge_num,ret);
	return ret;
}

/**
 * canque_again_outslot - reschedule output slot to process it again later
 * @qends: ends structure belonging to calling communication object
 * @qedge: edge slot belong to
 * @slot: pointer to the slot for re-processing
 *
 * Function reschedules slot previously acquired by canque_test_outslot()
 * function call for second time processing.
 * Return Value: Function cannot fail.
 */
int canque_again_outslot(struct canque_ends_t *qends,
	struct canque_edge_t *qedge, struct canque_slot_t *slot)
{
	int ret;
	ret=canque_fifo_again_outslot(&qedge->fifo, slot);
	canque_edge_decref(qedge);
	DEBUGQUE("canque_again_outslot for edge %d returned %d\n",qedge->edge_num,ret);
	return ret;
}

/**
 * canque_set_filt - sets filter for specified edge
 * @qedge: pointer to the edge
 * @filtid: ID to set for the edge
 * @filtmask: mask used for ID match check
 * @filtflags: required filer flags
 *
 * Return Value: Negative value is returned if edge is in the process of delete.
 */
int canque_set_filt(struct canque_edge_t *qedge,
	unsigned long filtid, unsigned long filtmask, int filtflags)
{
	int ret;
	can_spin_irqflags_t flags;

	can_spin_lock_irqsave(&qedge->fifo.fifo_lock,flags);
	
	if(!(filtflags&MSG_PROCESSLOCAL) && (processlocal<2))
		filtflags |= MSG_LOCAL_MASK;
	
	qedge->filtid=canque_filtid2internal(filtid, filtflags);
	qedge->filtmask=canque_filtid2internal(filtmask, filtflags>>MSG_FILT_MASK_SHIFT);
	
	if(canque_fifo_test_fl(&qedge->fifo,DEAD)) ret=-1;
	else ret=canque_fifo_test_and_set_fl(&qedge->fifo,BLOCK)?1:0;

	can_spin_unlock_irqrestore(&qedge->fifo.fifo_lock,flags);
	if(ret>=0){
		canque_notify_bothends(qedge,CANQUEUE_NOTIFY_FILTCH);
	}
	can_spin_lock_irqsave(&qedge->fifo.fifo_lock,flags);
	if(!ret) canque_fifo_clear_fl(&qedge->fifo,BLOCK);
	can_spin_unlock_irqrestore(&qedge->fifo.fifo_lock,flags);
	
	DEBUGQUE("canque_set_filt for edge %d, ID %ld, mask %ld, flags %d returned %d\n",
	          qedge->edge_num,filtid,filtmask,filtflags,ret);
	return ret;
}

/**
 * canque_flush - fluesh all ready slots in the edge
 * @qedge: pointer to the edge
 *
 * Tries to flush all allocated slots from the edge, but there could
 * exist some slots associated to edge which are processed by input
 * or output side and cannot be flushed at this moment.
 * Return Value: The nonzero value indicates, that queue has not been
 *	empty before the function call.
 */
int canque_flush(struct canque_edge_t *qedge)
{
	int ret;
	can_spin_irqflags_t flags;

	ret=canque_fifo_flush_slots(&qedge->fifo);
	if(ret){
		canque_notify_inends(qedge,CANQUEUE_NOTIFY_EMPTY);
		canque_notify_inends(qedge,CANQUEUE_NOTIFY_SPACE);
		can_spin_lock_irqsave(&qedge->outends->ends_lock, flags);
		can_spin_lock(&qedge->fifo.fifo_lock);
		if(canque_fifo_test_fl(&qedge->fifo,EMPTY)){
			list_del(&qedge->activepeers);
			list_add(&qedge->activepeers,&qedge->outends->idle);
		}
		can_spin_unlock(&qedge->fifo.fifo_lock);
		can_spin_unlock_irqrestore(&qedge->outends->ends_lock, flags);
	}
	DEBUGQUE("canque_flush for edge %d returned %d\n",qedge->edge_num,ret);
	return ret;
}

/**
 * canqueue_ends_init_gen - subsystem independent routine to initialize ends state
 * @qends: pointer to the ends structure
 *
 * Return Value: Cannot fail.
 */
int canqueue_ends_init_gen(struct canque_ends_t *qends)
{
	int i;
	qends->ends_flags=0;
	for(i=CANQUEUE_PRIO_NR;--i>=0;){
		INIT_LIST_HEAD(&qends->active[i]);
	}
	INIT_LIST_HEAD(&qends->idle);
	INIT_LIST_HEAD(&qends->inlist);
	INIT_LIST_HEAD(&qends->outlist);
	can_spin_lock_init(&qends->ends_lock);
	return 0;
}


/**
 * canqueue_connect_edge - connect edge between two communication entities
 * @qedge: pointer to edge
 * @inends: pointer to ends the input of the edge should be connected to
 * @outends: pointer to ends the output of the edge should be connected to
 *
 * Return Value: Negative value informs about failed operation.
 */
int canqueue_connect_edge(struct canque_edge_t *qedge, struct canque_ends_t *inends, struct canque_ends_t *outends)
{
	can_spin_irqflags_t flags;
	if(qedge == NULL) return -1;
	DEBUGQUE("canqueue_connect_edge %d\n",qedge->edge_num);
	canque_edge_incref(qedge);
	flags=canque_edge_lock_both_ends(inends, outends);
	can_spin_lock(&qedge->fifo.fifo_lock);
	qedge->inends=inends;
	list_add(&qedge->inpeers,&inends->inlist);
	qedge->outends=outends;
	list_add(&qedge->outpeers,&outends->outlist);
	list_add(&qedge->activepeers,&outends->idle);
	can_spin_unlock(&qedge->fifo.fifo_lock);
	canque_edge_unlock_both_ends(inends, outends, flags);
	canque_notify_bothends(qedge, CANQUEUE_NOTIFY_ATTACH);

	if(canque_fifo_test_and_set_fl(&qedge->fifo, READY))
		canque_edge_decref(qedge);
	return 0;
}

/**
 * canqueue_disconnect_edge - disconnect edge from communicating entities
 * @qedge: pointer to edge
 *
 * Return Value: Negative value means, that edge is used by somebody
 *	other and cannot be disconnected. Operation has to be delayed.
 */
int canqueue_disconnect_edge(struct canque_edge_t *qedge)
{
	int ret;
	can_spin_irqflags_t flags;
	struct canque_ends_t *inends, *outends;

	inends=qedge->inends;
	outends=qedge->outends;

	if(inends && outends) {
		flags=canque_edge_lock_both_ends(inends, outends);
	} else {
		DEBUGQUE("canqueue_disconnect_edge called with not fully connected edge");
		if(inends) can_spin_lock_irqsave(&inends->ends_lock,flags);
		if(outends) can_spin_lock(&outends->ends_lock);
		flags=0;
	}
	
	can_spin_lock(&qedge->fifo.fifo_lock);
	if(atomic_read(&qedge->edge_used)==0) {
		if(qedge->outends){
			list_del(&qedge->activepeers);
			mb(); /* memory barrier for list_empty use in canque_dead_func */
			list_del(&qedge->outpeers);
			qedge->outends=NULL;
		}
		if(qedge->inends){
			list_del(&qedge->inpeers);
			qedge->inends=NULL;
		}
		ret=1;
	} else ret=-1;
	can_spin_unlock(&qedge->fifo.fifo_lock);

	if(inends && outends) {
		canque_edge_unlock_both_ends(inends, outends, flags);
	} else {
		if(outends) can_spin_unlock(&outends->ends_lock);
		if(inends) can_spin_unlock_irqrestore(&inends->ends_lock,flags);
	}

	DEBUGQUE("canqueue_disconnect_edge %d returned %d\n",qedge->edge_num,ret);
	return ret;
}


/**
 * canqueue_block_inlist - block slot allocation of all outgoing edges of specified ends  
 * @qends: pointer to ends structure
 */
void canqueue_block_inlist(struct canque_ends_t *qends)
{
	struct canque_edge_t *edge;

        canque_for_each_inedge(qends, edge) {
		canque_fifo_set_fl(&edge->fifo,BLOCK);
	}
}


/**
 * canqueue_block_outlist - block slot allocation of all incoming edges of specified ends  
 * @qends: pointer to ends structure
 */
void canqueue_block_outlist(struct canque_ends_t *qends)
{
	struct canque_edge_t *edge;

        canque_for_each_outedge(qends, edge) {
		canque_fifo_set_fl(&edge->fifo,BLOCK);
	}
}


/**
 * canqueue_ends_kill_inlist - sends request to die to all outgoing edges
 * @qends: pointer to ends structure
 * @send_rest: select, whether already allocated slots should be processed
 *	by FIFO output side
 *
 * Return Value: Non-zero value means, that not all edges could be immediately
 *	disconnected and that ends structure memory release has to be delayed
 */
int canqueue_ends_kill_inlist(struct canque_ends_t *qends, int send_rest)
{
	struct canque_edge_t *edge;
	
	canque_for_each_inedge(qends, edge){
		canque_notify_bothends(edge, CANQUEUE_NOTIFY_DEAD_WANTED);
		if(send_rest){
			canque_edge_incref(edge);
			if(!canque_fifo_test_and_set_fl(&edge->fifo, FREEONEMPTY)){
				if(!canque_fifo_test_fl(&edge->fifo, EMPTY))
					continue;
				if(!canque_fifo_test_and_clear_fl(&edge->fifo, FREEONEMPTY))
					continue;
			}
			canque_edge_decref(edge);
		}
	}
	return list_empty(&qends->inlist)?0:1;
}


/**
 * canqueue_ends_kill_outlist - sends request to die to all incoming edges
 * @qends: pointer to ends structure
 *
 * Return Value: Non-zero value means, that not all edges could be immediately
 *	disconnected and that ends structure memory release has to be delayed
 */
int canqueue_ends_kill_outlist(struct canque_ends_t *qends)
{
	struct canque_edge_t *edge;
	
	canque_for_each_outedge(qends, edge){
		canque_notify_bothends(edge, CANQUEUE_NOTIFY_DEAD_WANTED);
	}
	return list_empty(&qends->outlist)?0:1;
}


/**
 * canqueue_ends_filt_conjuction - computes conjunction of incoming edges filters filters
 * @qends: pointer to ends structure
 * @filt: pointer the filter structure filled by computed filters conjunction
 *
 * Return Value: Number of incoming edges
 */
int canqueue_ends_filt_conjuction(struct canque_ends_t *qends, struct canfilt_t *filt)
{
	struct canque_edge_t *edge;
	int cnt=0;
	unsigned long filtid=0;
	unsigned long filtmask=~0;
	unsigned long local_only=canque_filtid2internal(0,MSG_LOCAL);

	canque_for_each_inedge(qends, edge){
		/* skip edges processing only local messages */
		if(edge->filtid & edge->filtmask & local_only)
			continue;

		if(!cnt++)
			filtid = edge->filtid;
		else
			filtmask &= ~(filtid ^ edge->filtid);
	
		filtmask &= edge->filtmask;
	}
	
	filt->id = filtid & MSG_ID_MASK;
	filt->mask = filtmask & MSG_ID_MASK;
	filtid >>= 28;
	filtmask >>= 28;
	filt->flags = filtid & MSG_EXT;
	if(filtmask & (MSG_EXT))
		filt->flags |= MSG_EXT_MASK;
	if(filtid & (MSG_RTR<<1))
		filt->flags |= MSG_RTR<<1;
	if(filtmask & (MSG_RTR<<1))
		filt->flags |= MSG_RTR_MASK;
	return cnt;
}


/**
 * canqueue_ends_flush_inlist - flushes all messages in incoming edges
 * @qends: pointer to ends structure
 *
 * Return Value: Negative value informs about unsuccessful result
 */
int canqueue_ends_flush_inlist(struct canque_ends_t *qends)
{
	struct canque_edge_t *edge;
	
	canque_for_each_inedge(qends, edge){
		canque_flush(edge);
	}
	return 0;
}


/**
 * canqueue_ends_flush_outlist - flushes all messages in outgoing edges
 * @qends: pointer to ends structure
 *
 * Return Value: Negative value informs about unsuccessful result
 */
int canqueue_ends_flush_outlist(struct canque_ends_t *qends)
{
	struct canque_edge_t *edge;
	
	canque_for_each_outedge(qends, edge){
		canque_flush(edge);
	}
	return 0;
}




