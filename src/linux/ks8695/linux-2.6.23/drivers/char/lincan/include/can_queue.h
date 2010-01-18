/* can_queue.h - CAN queues and message passing infrastructure 
 * Linux CAN-bus device driver.
 * Written by Pavel Pisa - OCERA team member
 * email:pisa@cmp.felk.cvut.cz
 * This software is released under the GPL-License.
 * Version lincan-0.3  17 Jun 2004
 */

#ifndef _CAN_QUEUE_H
#define _CAN_QUEUE_H

#include "./canmsg.h"
#include "./constants.h"
#include "./can_sysdep.h"

/**
 * struct canque_slot_t - one CAN message slot in the CAN FIFO queue 
 * @next: pointer to the next/younger slot
 * @slot_flags: space for flags and optional command describing action
 *	associated with slot data
 * @msg: space for one CAN message
 *
 * This structure is used to store CAN messages in the CAN FIFO queue.
 */
 struct canque_slot_t {
	struct canque_slot_t *next;
	unsigned long slot_flags;
	struct canmsg_t msg;
};

#define CAN_SLOTF_CMD	0x00ff	/*  */

/**
 * struct canque_fifo_t - CAN FIFO queue representation
 * @fifo_flags: this field holds global flags describing state of the FIFO.
 *	%CAN_FIFOF_ERROR is set when some error condition occurs.
 *	%CAN_FIFOF_ERR2BLOCK defines, that error should lead to the FIFO block state.
 *	%CAN_FIFOF_BLOCK state blocks insertion of the next messages. 
 *	%CAN_FIFOF_OVERRUN attempt to acquire new slot, when FIFO is full. 
 *	%CAN_FIFOF_FULL indicates FIFO full state. 
 *	%CAN_FIFOF_EMPTY indicates no allocated slot in the FIFO.
 *	%CAN_FIFOF_DEAD condition indication. Used when FIFO is beeing destroyed.
 * @error_code: futher description of error condition
 * @head: pointer to the FIFO head, oldest slot
 * @tail: pointer to the location, where pointer to newly inserted slot
 *	should be added
 * @flist: pointer to list of the free slots associated with queue
 * @entry: pointer to the memory allocated for the list slots.
 * @fifo_lock: the lock to ensure atomicity of slot manipulation operations.
 * @slotsnr:  number of allocated slots
 *
 * This structure represents CAN FIFO queue. It is implemented as 
 * a single linked list of slots prepared for processing. The empty slots
 * are stored in single linked list (@flist).
 */
struct canque_fifo_t {
	unsigned long fifo_flags;
	unsigned long error_code;
	struct canque_slot_t *head;	/* points to the oldest entry */
	struct canque_slot_t **tail;	/* points to NULL pointer for chaining */
	struct canque_slot_t *flist;	/* points the first entry in the free list */
	struct canque_slot_t *entry;	/* points to first allocated entry */
	can_spinlock_t fifo_lock;	/* can_spin_lock_irqsave / can_spin_unlock_irqrestore */
	int    slotsnr;
};

#define CAN_FIFOF_DESTROY_b	15
#define CAN_FIFOF_ERROR_b	14
#define CAN_FIFOF_ERR2BLOCK_b	13
#define CAN_FIFOF_BLOCK_b	12
#define CAN_FIFOF_OVERRUN_b	11
#define CAN_FIFOF_FULL_b	10
#define CAN_FIFOF_EMPTY_b	9
#define CAN_FIFOF_DEAD_b	8
#define CAN_FIFOF_INACTIVE_b	7
#define CAN_FIFOF_FREEONEMPTY_b	6
#define CAN_FIFOF_READY_b	5
#define CAN_FIFOF_NOTIFYPEND_b	4
#define CAN_FIFOF_RTL_MEM_b	3

#define CAN_FIFOF_DESTROY	(1<<CAN_FIFOF_DESTROY_b)
#define CAN_FIFOF_ERROR		(1<<CAN_FIFOF_ERROR_b)
#define CAN_FIFOF_ERR2BLOCK	(1<<CAN_FIFOF_ERR2BLOCK_b)
#define CAN_FIFOF_BLOCK		(1<<CAN_FIFOF_BLOCK_b)
#define CAN_FIFOF_OVERRUN	(1<<CAN_FIFOF_OVERRUN_b)
#define CAN_FIFOF_FULL		(1<<CAN_FIFOF_FULL_b)
#define CAN_FIFOF_EMPTY		(1<<CAN_FIFOF_EMPTY_b)
#define CAN_FIFOF_DEAD		(1<<CAN_FIFOF_DEAD_b)
#define CAN_FIFOF_INACTIVE	(1<<CAN_FIFOF_INACTIVE_b)
#define CAN_FIFOF_FREEONEMPTY	(1<<CAN_FIFOF_FREEONEMPTY_b)
#define CAN_FIFOF_READY		(1<<CAN_FIFOF_READY_b)
#define CAN_FIFOF_NOTIFYPEND    (1<<CAN_FIFOF_NOTIFYPEND_b)
#define CAN_FIFOF_RTL_MEM       (1<<CAN_FIFOF_RTL_MEM_b)

#define canque_fifo_test_fl(fifo,fifo_fl) \
  test_bit(CAN_FIFOF_##fifo_fl##_b,&(fifo)->fifo_flags)
#define canque_fifo_set_fl(fifo,fifo_fl) \
  set_bit(CAN_FIFOF_##fifo_fl##_b,&(fifo)->fifo_flags)
#define canque_fifo_clear_fl(fifo,fifo_fl) \
  clear_bit(CAN_FIFOF_##fifo_fl##_b,&(fifo)->fifo_flags)
#define canque_fifo_test_and_set_fl(fifo,fifo_fl) \
  test_and_set_bit(CAN_FIFOF_##fifo_fl##_b,&(fifo)->fifo_flags)
#define canque_fifo_test_and_clear_fl(fifo,fifo_fl) \
  test_and_clear_bit(CAN_FIFOF_##fifo_fl##_b,&(fifo)->fifo_flags)


/**
 * canque_fifo_get_inslot - allocate slot for the input of one CAN message 
 * @fifo: pointer to the FIFO structure
 * @slotp: pointer to location to store pointer to the allocated slot.
 * @cmd: optional command associated with allocated slot.
 *
 * Return Value: The function returns negative value if there is no
 *	free slot in the FIFO queue.
 */
static inline
int canque_fifo_get_inslot(struct canque_fifo_t *fifo, struct canque_slot_t **slotp, int cmd)
{
	can_spin_irqflags_t flags;
	struct canque_slot_t *slot;
	can_spin_lock_irqsave(&fifo->fifo_lock, flags);
	/* get the first free slot slot from flist */
	if(!(slot=fifo->flist)) {
		canque_fifo_set_fl(fifo,OVERRUN);
		canque_fifo_set_fl(fifo,FULL);
		can_spin_unlock_irqrestore(&fifo->fifo_lock, flags);
		*slotp=NULL;
		return -1;
	}
	/* adjust free slot list */
	if(!(fifo->flist=slot->next))
		canque_fifo_set_fl(fifo,FULL);
	can_spin_unlock_irqrestore(&fifo->fifo_lock, flags);
	*slotp=slot;
	slot->slot_flags=cmd&CAN_SLOTF_CMD;
	return 1;
}

/**
 * canque_fifo_put_inslot - releases slot to further processing
 * @fifo: pointer to the FIFO structure
 * @slot: pointer to the slot previously acquired by canque_fifo_get_inslot().
 *
 * Return Value: The nonzero return value indicates, that the queue was empty
 *	before call to the function. The caller should wake-up output side of the queue.
 */
static inline
int canque_fifo_put_inslot(struct canque_fifo_t *fifo, struct canque_slot_t *slot)
{
	int ret;
	can_spin_irqflags_t flags;
	slot->next=NULL;
	can_spin_lock_irqsave(&fifo->fifo_lock, flags);
	if(*fifo->tail) can_printk(KERN_CRIT "canque_fifo_put_inslot: fifo->tail != NULL\n");
	*fifo->tail=slot;
	fifo->tail=&slot->next;
	ret=0;
	if(canque_fifo_test_and_clear_fl(fifo,EMPTY))
	  ret=CAN_FIFOF_EMPTY;	/* Fifo has been empty before put */
	if(canque_fifo_test_and_clear_fl(fifo,INACTIVE))
	  ret=CAN_FIFOF_INACTIVE; /* Fifo has been empty before put */
	can_spin_unlock_irqrestore(&fifo->fifo_lock, flags);
	return ret;
}

/**
 * canque_fifo_abort_inslot - release and abort slot
 * @fifo: pointer to the FIFO structure
 * @slot: pointer to the slot previously acquired by canque_fifo_get_inslot().
 *
 * Return Value: The nonzero value indicates, that fifo was full
 */
static inline
int canque_fifo_abort_inslot(struct canque_fifo_t *fifo, struct canque_slot_t *slot)
{
	int ret=0;
	can_spin_irqflags_t flags;
	can_spin_lock_irqsave(&fifo->fifo_lock, flags);
	slot->next=fifo->flist;
	fifo->flist=slot;
	if(canque_fifo_test_and_clear_fl(fifo,FULL))
		ret=CAN_FIFOF_FULL;
	can_spin_unlock_irqrestore(&fifo->fifo_lock, flags);
	return ret;
}

/**
 * canque_fifo_test_outslot - test and get ready slot from the FIFO
 * @fifo: pointer to the FIFO structure
 * @slotp: pointer to location to store pointer to the oldest slot from the FIFO.
 *
 * Return Value: The negative value indicates, that queue is empty.
 *	The positive or zero value represents command stored into slot by
 *	the call to the function canque_fifo_get_inslot().
 *	The successfully acquired FIFO output slot has to be released by
 *	the call canque_fifo_free_outslot() or canque_fifo_again_outslot().
 */
static inline
int canque_fifo_test_outslot(struct canque_fifo_t *fifo, struct canque_slot_t **slotp)
{
	can_spin_irqflags_t flags;
	int cmd;
	struct canque_slot_t *slot;
	can_spin_lock_irqsave(&fifo->fifo_lock, flags);
	if(!(slot=fifo->head)){;
		canque_fifo_set_fl(fifo,EMPTY);
		can_spin_unlock_irqrestore(&fifo->fifo_lock, flags);
		*slotp=NULL;
		return -1;
	}
	if(!(fifo->head=slot->next))
		fifo->tail=&fifo->head;
	can_spin_unlock_irqrestore(&fifo->fifo_lock, flags);

	*slotp=slot;
	cmd=slot->slot_flags;
	return cmd&CAN_SLOTF_CMD;
}


/**
 * canque_fifo_free_outslot - free processed FIFO slot
 * @fifo: pointer to the FIFO structure
 * @slot: pointer to the slot previously acquired by canque_fifo_test_outslot().
 *
 * Return Value: The returned value informs about FIFO state change.
 *	The mask %CAN_FIFOF_FULL indicates, that the FIFO was full before
 *	the function call. The mask %CAN_FIFOF_EMPTY informs, that last ready slot
 *	has been processed.
 */
static inline
int canque_fifo_free_outslot(struct canque_fifo_t *fifo, struct canque_slot_t *slot)
{
	int ret=0;
	can_spin_irqflags_t flags;
	can_spin_lock_irqsave(&fifo->fifo_lock, flags);
	slot->next=fifo->flist;
	fifo->flist=slot;
	if(canque_fifo_test_and_clear_fl(fifo,FULL))
		ret=CAN_FIFOF_FULL;
	if(!(fifo->head)){
		canque_fifo_set_fl(fifo,EMPTY);
		ret|=CAN_FIFOF_EMPTY;
	}
	can_spin_unlock_irqrestore(&fifo->fifo_lock, flags);
	return ret;
}

/**
 * canque_fifo_again_outslot - interrupt and postpone processing of the slot
 * @fifo: pointer to the FIFO structure
 * @slot: pointer to the slot previously acquired by canque_fifo_test_outslot().
 *
 * Return Value: The function cannot fail..
 */
static inline
int canque_fifo_again_outslot(struct canque_fifo_t *fifo, struct canque_slot_t *slot)
{
	can_spin_irqflags_t flags;
	can_spin_lock_irqsave(&fifo->fifo_lock, flags);
	if(!(slot->next=fifo->head))
		fifo->tail=&slot->next;
	fifo->head=slot;
	can_spin_unlock_irqrestore(&fifo->fifo_lock, flags);
	return 1;
}

int canque_fifo_flush_slots(struct canque_fifo_t *fifo);

int canque_fifo_init_slots(struct canque_fifo_t *fifo);

#define CANQUEUE_PRIO_NR  3

/* Forward declarations for external types */
struct msgobj_t;
struct canchip_t;

/**
 * struct canque_edge_t - CAN message delivery subsystem graph edge
 * @fifo: place where primitive @struct canque_fifo_t FIFO is located.
 * @filtid: the possible CAN message identifiers filter.
 * @filtmask: the filter mask, the comparison considers only
 *	@filtid bits corresponding to set bits in the @filtmask field.
 * @inpeers: the lists of all peers FIFOs connected by their
 *	input side (@inends) to the same terminal (@struct canque_ends_t).
 * @outpeers: the lists of all peers FIFOs connected by their
 *	output side (@outends) to the same terminal (@struct canque_ends_t).
 * @activepeers: the lists of peers FIFOs connected by their
 *	output side (@outends) to the same terminal (@struct canque_ends_t)
 *	with same priority and active state.
 * @inends: the pointer to the FIFO input side terminal (@struct canque_ends_t).
 * @outends: the pointer to the FIFO output side terminal (@struct canque_ends_t).
 * @edge_used: the atomic usage counter, mainly used for safe destruction of the edge.
 * @edge_prio: the assigned queue priority from the range 0 to %CANQUEUE_PRIO_NR-1
 * @edge_num: edge sequential number intended for debugging purposes only
 * @pending_peers: edges with pending delayed events (RTL->Linux calls)
 * @pending_inops: bitmask of pending operations
 * @pending_outops: bitmask of pending operations
 *
 * This structure represents one direction connection from messages source 
 * (@inends) to message consumer (@outends) fifo ends hub. The edge contains
 * &struct canque_fifo_t for message fifo implementation.
 */
struct canque_edge_t {
	struct canque_fifo_t fifo;
	unsigned long filtid;
	unsigned long filtmask;
	struct list_head inpeers;
	struct list_head outpeers;
	struct list_head activepeers;
	struct canque_ends_t *inends;
	struct canque_ends_t *outends;
	atomic_t edge_used;
	int edge_prio;
	int edge_num;
    #ifdef CAN_WITH_RTL
	struct list_head pending_peers;
	unsigned long pending_inops;
	unsigned long pending_outops;
    #endif /*CAN_WITH_RTL*/
};

/**
 * struct canque_ends_t - CAN message delivery subsystem graph vertex (FIFO ends)
 * @ends_flags: this field holds flags describing state of the ENDS structure.
 * @active: the array of the lists of active edges directed to the ends structure
 *	with ready messages. The array is indexed by the edges priorities. 
 * @idle: the list of the edges directed to the ends structure with empty FIFOs.
 * @inlist: the list of outgoing edges input sides.
 * @outlist: the list of all incoming edges output sides. Each of there edges
 *	is listed on one of @active or @idle lists.
 * @ends_lock: the lock synchronizing operations between threads accessing
 *	same ends structure.
 * @notify: pointer to notify procedure. The next state changes are notified.
 *	%CANQUEUE_NOTIFY_EMPTY (out->in call) - all slots are processed by FIFO out side. 
 *	%CANQUEUE_NOTIFY_SPACE (out->in call) - full state negated => there is space for new message.
 *	%CANQUEUE_NOTIFY_PROC  (in->out call) - empty state negated => out side is requested to process slots.
 *	%CANQUEUE_NOTIFY_NOUSR (both) - notify, that the last user has released the edge usage
 *		called with some lock to prevent edge disappear.
 *	%CANQUEUE_NOTIFY_DEAD  (both) - edge is in progress of deletion.
 *	%CANQUEUE_NOTIFY_ATACH (both) - new edge has been attached to end.
 *	%CANQUEUE_NOTIFY_FILTCH (out->in call) - edge filter rules changed
 *	%CANQUEUE_NOTIFY_ERROR  (out->in call) - error in messages processing.
 * @context: space to store ends user specific information
 * @endinfo: space to store some other ends usage specific informations
 *	mainly for waking-up by the notify calls.
 * @dead_peers: used to chain ends wanting for postponed destruction
 *
 * Structure represents place to connect edges to for CAN communication entity.
 * The zero, one or more incoming and outgoing edges can be connected to
 * this structure.
 */
struct canque_ends_t {
	unsigned long ends_flags;
	struct list_head active[CANQUEUE_PRIO_NR];
	struct list_head idle;
	struct list_head inlist;
	struct list_head outlist;
	can_spinlock_t ends_lock;	/* can_spin_lock_irqsave / can_spin_unlock_irqrestore */
	void (*notify)(struct canque_ends_t *qends, struct canque_edge_t *qedge, int what);
	void *context;
	union {
		struct {
			wait_queue_head_t readq;
			wait_queue_head_t writeq;
			wait_queue_head_t emptyq;
		    #ifdef CAN_ENABLE_KERN_FASYNC
			struct fasync_struct *fasync;
		    #endif /*CAN_ENABLE_KERN_FASYNC*/
		} fileinfo;
	    #ifdef CAN_WITH_RTL
		struct {
			rtl_spinlock_t rtl_lock;
			rtl_wait_t rtl_readq;
			atomic_t   rtl_readq_age;
			rtl_wait_t rtl_writeq;
			atomic_t   rtl_writeq_age;
			rtl_wait_t rtl_emptyq;
			atomic_t   rtl_emptyq_age;
			unsigned long pend_flags;
		} rtlinfo;
	    #endif /*CAN_WITH_RTL*/
		struct {
			struct msgobj_t *msgobj;
			struct canchip_t *chip;
		    #ifndef CAN_WITH_RTL
			wait_queue_head_t daemonq;
		    #else /*CAN_WITH_RTL*/
			pthread_t worker_thread;
		    #endif /*CAN_WITH_RTL*/
		} chipinfo;
	} endinfo;
	struct list_head dead_peers;
};

#define CANQUEUE_NOTIFY_EMPTY  1 /* out -> in - all slots are processed by FIFO out side */
#define CANQUEUE_NOTIFY_SPACE  2 /* out -> in - full state negated => there is space for new message */
#define CANQUEUE_NOTIFY_PROC   3 /* in -> out - empty state negated => out side is requested to process slots */
#define CANQUEUE_NOTIFY_NOUSR  4 /* called with some lock to prevent edge disappear */
#define CANQUEUE_NOTIFY_DEAD   5 /*  */
#define CANQUEUE_NOTIFY_DEAD_WANTED 6 /*  */
#define CANQUEUE_NOTIFY_ATTACH 7 /*  */
#define CANQUEUE_NOTIFY_FILTCH 8 /* filter changed */
#define CANQUEUE_NOTIFY_ERROR      0x10000 /* error notifiers */
#define CANQUEUE_NOTIFY_ERRTX_PREP 0x11001 /* tx preparation error */
#define CANQUEUE_NOTIFY_ERRTX_SEND 0x11002 /* tx send error */
#define CANQUEUE_NOTIFY_ERRTX_BUS  0x11003 /* tx bus error */

#define CAN_ENDSF_DEAD	  (1<<0)
#define CAN_ENDSF_MEM_RTL (1<<1)

/**
 * canque_notify_inends - request to send notification to the input ends
 * @qedge: pointer to the edge structure
 * @what: notification type
 */
static inline
void canque_notify_inends(struct canque_edge_t *qedge, int what)
{
	if(qedge->inends)
		if(qedge->inends->notify)
			qedge->inends->notify(qedge->inends,qedge,what);
}

/**
 * canque_notify_outends - request to send notification to the output ends
 * @qedge: pointer to the edge structure
 * @what: notification type
 */
static inline
void canque_notify_outends(struct canque_edge_t *qedge, int what)
{
	if(qedge->outends)
		if(qedge->outends->notify)
			qedge->outends->notify(qedge->outends,qedge,what);
}

/**
 * canque_notify_bothends - request to send notification to the both ends
 * @qedge: pointer to the edge structure
 * @what: notification type
 */
static inline
void canque_notify_bothends(struct canque_edge_t *qedge, int what)
{
	canque_notify_inends(qedge, what);
	canque_notify_outends(qedge, what);
}

/**
 * canque_activate_edge - mark output end of the edge as active
 * @qedge: pointer to the edge structure
 * @inends: input side of the edge
 *
 * Function call moves output side of the edge from idle onto active edges
 * list. This function has to be called with edge reference count held.
 * that is same as for most of other edge functions.
 */
static inline
void canque_activate_edge(struct canque_ends_t *inends, struct canque_edge_t *qedge)
{
	can_spin_irqflags_t flags;
	struct canque_ends_t *outends;
	if(qedge->edge_prio>=CANQUEUE_PRIO_NR)
		qedge->edge_prio=CANQUEUE_PRIO_NR-1;
	if((outends=qedge->outends)){
		can_spin_lock_irqsave(&outends->ends_lock, flags);
		can_spin_lock(&qedge->fifo.fifo_lock);
		if(!canque_fifo_test_fl(&qedge->fifo,EMPTY)){
			list_del(&qedge->activepeers);
			list_add_tail(&qedge->activepeers,&outends->active[qedge->edge_prio]);
		}
		can_spin_unlock(&qedge->fifo.fifo_lock);
		can_spin_unlock_irqrestore(&outends->ends_lock, flags);
	}
}

/**
 * canque_filtid2internal - converts message ID and filter flags into internal format
 * @id: CAN message 11 or 29 bit identifier
 * @filtflags: CAN message flags
 *
 * This function maps message ID and %MSG_RTR, %MSG_EXT and %MSG_LOCAL into one 32 bit number
 */
static inline
unsigned int canque_filtid2internal(unsigned long id, int filtflags)
{
	filtflags &= MSG_RTR|MSG_EXT|MSG_LOCAL;
	filtflags += filtflags&MSG_RTR;
	return (id&MSG_ID_MASK) | (filtflags<<28);
}

int canque_get_inslot(struct canque_ends_t *qends,
	struct canque_edge_t **qedgep, struct canque_slot_t **slotp, int cmd);
	
int canque_get_inslot4id(struct canque_ends_t *qends,
	struct canque_edge_t **qedgep, struct canque_slot_t **slotp,
	int cmd, unsigned long id, int prio);
	
int canque_put_inslot(struct canque_ends_t *qends,
	struct canque_edge_t *qedge, struct canque_slot_t *slot);

int canque_abort_inslot(struct canque_ends_t *qends,
	struct canque_edge_t *qedge, struct canque_slot_t *slot);

int canque_filter_msg2edges(struct canque_ends_t *qends, struct canmsg_t *msg);

int canque_test_outslot(struct canque_ends_t *qends,
	struct canque_edge_t **qedgep, struct canque_slot_t **slotp);

int canque_free_outslot(struct canque_ends_t *qends,
	struct canque_edge_t *qedge, struct canque_slot_t *slot);

int canque_again_outslot(struct canque_ends_t *qends,
	struct canque_edge_t *qedge, struct canque_slot_t *slot);

int canque_set_filt(struct canque_edge_t *qedge,
	unsigned long filtid, unsigned long filtmask, int flags);
	
int canque_flush(struct canque_edge_t *qedge);

int canqueue_disconnect_edge(struct canque_edge_t *qedge);

int canqueue_connect_edge(struct canque_edge_t *qedge, struct canque_ends_t *inends, struct canque_ends_t *outends);

int canqueue_ends_init_gen(struct canque_ends_t *qends);

void canqueue_block_inlist(struct canque_ends_t *qends);

void canqueue_block_outlist(struct canque_ends_t *qends);

int canqueue_ends_kill_inlist(struct canque_ends_t *qends, int send_rest);

int canqueue_ends_kill_outlist(struct canque_ends_t *qends);

int canqueue_ends_filt_conjuction(struct canque_ends_t *qends, struct canfilt_t *filt);

int canqueue_ends_flush_inlist(struct canque_ends_t *qends);

int canqueue_ends_flush_outlist(struct canque_ends_t *qends);

/* edge reference and traversal functions */

void canque_edge_do_dead(struct canque_edge_t *edge);

/**
 * canque_edge_incref - increments edge reference count
 * @edge: pointer to the edge structure
 */
static inline
void canque_edge_incref(struct canque_edge_t *edge)
{
	atomic_inc(&edge->edge_used);
}

static inline
can_spin_irqflags_t canque_edge_lock_both_ends(struct canque_ends_t *inends, struct canque_ends_t *outends)
{
	can_spin_irqflags_t  flags;
	if(inends<outends) {
		can_spin_lock_irqsave(&inends->ends_lock, flags);
		can_spin_lock(&outends->ends_lock);
	}else{
		can_spin_lock_irqsave(&outends->ends_lock, flags);
		if(outends!=inends) can_spin_lock(&inends->ends_lock);
	}
	return flags; 	
}

static inline
void canque_edge_unlock_both_ends(struct canque_ends_t *inends, struct canque_ends_t *outends, can_spin_irqflags_t flags)
{
	if(outends!=inends) can_spin_unlock(&outends->ends_lock);
	can_spin_unlock_irqrestore(&inends->ends_lock, flags);
}

/* Non-inlined version of edge reference decrement */
void __canque_edge_decref(struct canque_edge_t *edge);

static inline
void __canque_edge_decref_body(struct canque_edge_t *edge)
{
	can_spin_irqflags_t flags;
	int dead_fl=0;
	struct canque_ends_t *inends=edge->inends;
	struct canque_ends_t *outends=edge->outends;
	
	flags=canque_edge_lock_both_ends(inends, outends);
	if(atomic_dec_and_test(&edge->edge_used)) {
		dead_fl=!canque_fifo_test_and_set_fl(&edge->fifo,DEAD);
		/* Because of former evolution of edge references 
		   management notify of CANQUEUE_NOTIFY_NOUSR could
		   be moved to canque_edge_do_dead :-) */
	}
	canque_edge_unlock_both_ends(inends, outends, flags);
	if(dead_fl) canque_edge_do_dead(edge);
}

#ifndef CAN_HAVE_ARCH_CMPXCHG
/**
 * canque_edge_decref - decrements edge reference count
 * @edge: pointer to the edge structure
 *
 * This function has to be called without lock held for both ends of edge.
 * If reference count drops to 0, function canque_edge_do_dead()
 * is called.
 */
static inline
void canque_edge_decref(struct canque_edge_t *edge)
{
	__canque_edge_decref_body(edge);
}
#else
static inline
void canque_edge_decref(struct canque_edge_t *edge)
{
	int x, y;
	
        x = atomic_read(&edge->edge_used);
        do{
		if(x<=1)
			return __canque_edge_decref(edge);
		y=x;
		/* This code strongly depends on the definition of atomic_t !!!! */
		/* x=cmpxchg(&edge->edge_used, x, x-1); */
		/* Next alternative could be more portable */
		x=__cmpxchg(&edge->edge_used, x, x-1, sizeof(atomic_t));
		/* If even this does not help, comment out CAN_HAVE_ARCH_CMPXCHG in can_sysdep.h */
	} while(x!=y);
}
#endif

static inline
struct canque_edge_t *canque_first_inedge(struct canque_ends_t *qends)
{
	can_spin_irqflags_t flags;
	struct list_head *entry;
	struct canque_edge_t *edge;
	
	can_spin_lock_irqsave(&qends->ends_lock, flags);
	entry=qends->inlist.next;
    skip_dead:
	if(entry != &qends->inlist) {
		edge=list_entry(entry,struct canque_edge_t,inpeers);
		if(canque_fifo_test_fl(&edge->fifo,DEAD)) {
			entry=entry->next;
			goto skip_dead;
		}
		canque_edge_incref(edge);
	} else {
		edge=NULL;
	}
	can_spin_unlock_irqrestore(&qends->ends_lock, flags);
	return edge;
}


static inline
struct canque_edge_t *canque_next_inedge(struct canque_ends_t *qends, struct canque_edge_t *edge)
{
	can_spin_irqflags_t flags;
	struct list_head *entry;
	struct canque_edge_t *next;
	
	can_spin_lock_irqsave(&qends->ends_lock, flags);
	entry=edge->inpeers.next;
    skip_dead:
	if(entry != &qends->inlist) {
		next=list_entry(entry,struct canque_edge_t,inpeers);
		if(canque_fifo_test_fl(&edge->fifo,DEAD)) {
			entry=entry->next;
			goto skip_dead;
		}
		canque_edge_incref(next);
	} else {
		next=NULL;
	}
	can_spin_unlock_irqrestore(&qends->ends_lock, flags);
	canque_edge_decref(edge);
	return next;
}

#define canque_for_each_inedge(qends, edge) \
	    for(edge=canque_first_inedge(qends);edge;edge=canque_next_inedge(qends, edge))

static inline
struct canque_edge_t *canque_first_outedge(struct canque_ends_t *qends)
{
	can_spin_irqflags_t flags;
	struct list_head *entry;
	struct canque_edge_t *edge;
	
	can_spin_lock_irqsave(&qends->ends_lock, flags);
	entry=qends->outlist.next;
    skip_dead:
	if(entry != &qends->outlist) {
		edge=list_entry(entry,struct canque_edge_t,outpeers);
		if(canque_fifo_test_fl(&edge->fifo,DEAD)) {
			entry=entry->next;
			goto skip_dead;
		}
		canque_edge_incref(edge);
	} else {
		edge=NULL;
	}
	can_spin_unlock_irqrestore(&qends->ends_lock, flags);
	return edge;
}


static inline
struct canque_edge_t *canque_next_outedge(struct canque_ends_t *qends, struct canque_edge_t *edge)
{
	can_spin_irqflags_t flags;
	struct list_head *entry;
	struct canque_edge_t *next;
	
	can_spin_lock_irqsave(&qends->ends_lock, flags);
	entry=edge->outpeers.next;
    skip_dead:
	if(entry != &qends->outlist) {
		next=list_entry(entry,struct canque_edge_t,outpeers);
		if(canque_fifo_test_fl(&edge->fifo,DEAD)) {
			entry=entry->next;
			goto skip_dead;
		}
		canque_edge_incref(next);
	} else {
		next=NULL;
	}
	can_spin_unlock_irqrestore(&qends->ends_lock, flags);
	canque_edge_decref(edge);
	return next;
}

#define canque_for_each_outedge(qends, edge) \
	    for(edge=canque_first_outedge(qends);edge;edge=canque_next_outedge(qends, edge))

/* Linux kernel specific functions */

int canque_fifo_init_kern(struct canque_fifo_t *fifo, int slotsnr);

int canque_fifo_done_kern(struct canque_fifo_t *fifo);

struct canque_edge_t *canque_new_edge_kern(int slotsnr);

int canque_get_inslot4id_wait_kern(struct canque_ends_t *qends,
	struct canque_edge_t **qedgep, struct canque_slot_t **slotp,
	int cmd, unsigned long id, int prio);

int canque_get_outslot_wait_kern(struct canque_ends_t *qends,
	struct canque_edge_t **qedgep, struct canque_slot_t **slotp);

int canque_sync_wait_kern(struct canque_ends_t *qends, struct canque_edge_t *qedge);

int canqueue_ends_init_kern(struct canque_ends_t *qends);

int canqueue_ends_dispose_kern(struct canque_ends_t *qends, int sync);

void canqueue_ends_dispose_postpone(struct canque_ends_t *qends);

void canqueue_kern_initialize(void);

#ifdef CAN_WITH_RTL

extern struct tasklet_struct canque_dead_tl;	/*publication required only for RTL*/

/* RT-Linux specific functions and variables */

extern int canqueue_rtl_irq;

extern unsigned long canqueue_rtl2lin_pend;

#define CAN_RTL2LIN_PEND_DEAD_b 0

void canqueue_rtl_initialize(void);
void canqueue_rtl_done(void);

int canqueue_rtl2lin_check_and_pend(struct canque_ends_t *qends,
			 struct canque_edge_t *qedge, int what);

struct canque_edge_t *canque_new_edge_rtl(int slotsnr);

void canque_dispose_edge_rtl(struct canque_edge_t *qedge);

int canque_get_inslot4id_wait_rtl(struct canque_ends_t *qends,
	struct canque_edge_t **qedgep, struct canque_slot_t **slotp,
	int cmd, unsigned long id, int prio);

int canque_get_outslot_wait_rtl(struct canque_ends_t *qends,
	struct canque_edge_t **qedgep, struct canque_slot_t **slotp);

int canque_sync_wait_rtl(struct canque_ends_t *qends, struct canque_edge_t *qedge);

void canque_ends_free_rtl(struct canque_ends_t *qends);

int canqueue_ends_init_rtl(struct canque_ends_t *qends);

int canqueue_ends_dispose_rtl(struct canque_ends_t *qends, int sync);

#else /*CAN_WITH_RTL*/

static inline int canqueue_rtl2lin_check_and_pend(struct canque_ends_t *qends,
			struct canque_edge_t *qedge, int what) { return 0; }

#endif /*CAN_WITH_RTL*/


#endif /*_CAN_QUEUE_H*/
