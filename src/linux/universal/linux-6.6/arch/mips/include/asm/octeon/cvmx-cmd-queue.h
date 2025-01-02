/***********************license start***************
 * Copyright (c) 2003-2014  Cavium Inc. (support@cavium.com). All rights
 * reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.

 *   * Neither the name of Cavium Inc. nor the names of
 *     its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written
 *     permission.

 * This Software, including technical data, may be subject to U.S. export  control
 * laws, including the U.S. Export Administration Act and its  associated
 * regulations, and may be subject to export or import  regulations in other
 * countries.

 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND CAVIUM INC. MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY REPRESENTATION OR
 * DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT DEFECTS, AND CAVIUM
 * SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES OF TITLE,
 * MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF
 * VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. THE ENTIRE  RISK ARISING OUT OF USE OR
 * PERFORMANCE OF THE SOFTWARE LIES WITH YOU.
 ***********************license end**************************************/

/**
 * @file
 *
 * Support functions for managing command queues used for
 * various hardware blocks.
 *
 * The common command queue infrastructure abstracts out the
 * software necessary for adding to Octeon's chained queue
 * structures. These structures are used for commands to the
 * PKO, ZIP, DFA, RAID, HNA, and DMA engine blocks. Although each
 * hardware unit takes commands and CSRs of different types,
 * they all use basic linked command buffers to store the
 * pending request. In general, users of the CVMX API don't
 * call cvmx-cmd-queue functions directly. Instead the hardware
 * unit specific wrapper should be used. The wrappers perform
 * unit specific validation and CSR writes to submit the
 * commands.
 *
 * Even though most software will never directly interact with
 * cvmx-cmd-queue, knowledge of its internal workings can help
 * in diagnosing performance problems and help with debugging.
 *
 * Command queue pointers are stored in a global named block
 * called "cvmx_cmd_queues". Except for the PKO queues, each
 * hardware queue is stored in its own cache line to reduce SMP
 * contention on spin locks. The PKO queues are stored such that
 * every 16th queue is next to each other in memory. This scheme
 * allows for queues being in separate cache lines when there
 * are low number of queues per port. With 16 queues per port,
 * the first queue for each port is in the same cache area. The
 * second queues for each port are in another area, etc. This
 * allows software to implement very efficient lockless PKO with
 * 16 queues per port using a minimum of cache lines per core.
 * All queues for a given core will be isolated in the same
 * cache area.
 *
 * In addition to the memory pointer layout, cvmx-cmd-queue
 * provides an optimized fair ll/sc locking mechanism for the
 * queues. The lock uses a "ticket / now serving" model to
 * maintain fair order on contended locks. In addition, it uses
 * predicted locking time to limit cache contention. When a core
 * know it must wait in line for a lock, it spins on the
 * internal cycle counter to completely eliminate any causes of
 * bus traffic.
 *
 * <hr> $Revision: 137666 $ <hr>
 */

#ifndef __CVMX_CMD_QUEUE_H__
#define __CVMX_CMD_QUEUE_H__

#include "cvmx-atomic.h"

#ifdef	__cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

/**
 * By default we disable the max depth support. Most programs
 * don't use it and it slows down the command queue processing
 * significantly.
 */
#ifndef CVMX_CMD_QUEUE_ENABLE_MAX_DEPTH
#define CVMX_CMD_QUEUE_ENABLE_MAX_DEPTH 0
#endif

#define	NUM_ELEMENTS(arr) (sizeof(arr)/sizeof((arr)[0]))

/**
 * Enumeration representing all hardware blocks that use command
 * queues. Each hardware block has up to 65536 sub identifiers for
 * multiple command queues. Not all chips support all hardware
 * units.
 */
typedef enum {
	CVMX_CMD_QUEUE_PKO_BASE = 0x00000,
#define CVMX_CMD_QUEUE_PKO(queue) ((cvmx_cmd_queue_id_t)(CVMX_CMD_QUEUE_PKO_BASE + (0xffff&(queue))))
	CVMX_CMD_QUEUE_ZIP = 0x10000,
#define CVMX_CMD_QUEUE_ZIP_QUE(queue) ((cvmx_cmd_queue_id_t)(CVMX_CMD_QUEUE_ZIP + (0xffff&(queue))))
	CVMX_CMD_QUEUE_DFA = 0x20000,
	CVMX_CMD_QUEUE_RAID = 0x30000,
	CVMX_CMD_QUEUE_DMA_BASE = 0x40000,
#define CVMX_CMD_QUEUE_DMA(queue) ((cvmx_cmd_queue_id_t)(CVMX_CMD_QUEUE_DMA_BASE + (0xffff&(queue))))
	CVMX_CMD_QUEUE_BCH = 0x50000,
#define CVMX_CMD_QUEUE_BCH(queue) ((cvmx_cmd_queue_id_t)(CVMX_CMD_QUEUE_BCH + (0xffff&(queue))))
	CVMX_CMD_QUEUE_HNA = 0x60000,
	CVMX_CMD_QUEUE_END = 0x70000,
} cvmx_cmd_queue_id_t;

#define CVMX_CMD_QUEUE_ZIP3_QUE(node,queue) \
  ((cvmx_cmd_queue_id_t)((node) << 24 | CVMX_CMD_QUEUE_ZIP | (0xffff&(queue))))

/**
 * Command write operations can fail if the command queue needs
 * a new buffer and the associated FPA pool is empty. It can also
 * fail if the number of queued command words reaches the maximum
 * set at initialization.
 */
typedef enum {
	CVMX_CMD_QUEUE_SUCCESS = 0,
	CVMX_CMD_QUEUE_NO_MEMORY = -1,
	CVMX_CMD_QUEUE_FULL = -2,
	CVMX_CMD_QUEUE_INVALID_PARAM = -3,
	CVMX_CMD_QUEUE_ALREADY_SETUP = -4,
} cvmx_cmd_queue_result_t;

typedef struct {
#ifdef __BIG_ENDIAN_BITFIELD
	/* First 64-bit word: */
	uint64_t fpa_pool:16;		/**< FPA1:POOL/FPA3:GAURA for buffers */
	uint64_t base_paddr:48;		/**< command buffer physical address */
	/* Second 64-bit word: */
	int32_t index;			/**< Number of cmd words in buffer */
	uint16_t max_depth;		/**< Maximum outstanding blocks */
	uint16_t pool_size_m1;		/**< FPA buffer size in dwords - 1 */
#else
	/* First 64-bit word: */
	uint64_t base_paddr:48;
	uint64_t fpa_pool:16;
	/* Second 64-bit word: */
	uint16_t pool_size_m1;
	uint16_t max_depth;
	int32_t index;
#endif
} __cvmx_cmd_queue_state_t;

/**
 * command-queue locking uses a fair ticket spinlock algo,
 * with 64-bit tickets for endianness-neutrality and
 * counter overflow protection.
 * Lock is free when both counters are of equal value.
 */
typedef struct {
	uint64_t ticket;
	uint64_t now_serving;
} __cvmx_cmd_queue_lock_t;

/**
 * @INTERNAL
 * This structure contains the global state of all command queues.
 * It is stored in a bootmem named block and shared by all
 * applications running on Octeon. Tickets are stored in a different
 * cache line that queue information to reduce the contention on the
 * ll/sc used to get a ticket. If this is not the case, the update
 * of queue state causes the ll/sc to fail quite often.
 */
typedef struct {
	__cvmx_cmd_queue_lock_t lock[(CVMX_CMD_QUEUE_END >> 16) * 256];
	__cvmx_cmd_queue_state_t state[(CVMX_CMD_QUEUE_END >> 16) * 256];
} __cvmx_cmd_queue_all_state_t;

extern CVMX_SHARED __cvmx_cmd_queue_all_state_t *
__cvmx_cmd_queue_state_ptrs[CVMX_MAX_NODES];

/**
 * @INTERNAL
 * Internal function to handle the corner cases
 * of adding command words to a queue when the current
 * block is getting full.
 */
extern cvmx_cmd_queue_result_t
		__cvmx_cmd_queue_write_raw(cvmx_cmd_queue_id_t queue_id,
			__cvmx_cmd_queue_state_t *qptr,
			int cmd_count, const uint64_t *cmds);


/**
 * Initialize a command queue for use. The initial FPA buffer is
 * allocated and the hardware unit is configured to point to the
 * new command queue.
 *
 * @param queue_id  Hardware command queue to initialize.
 * @param max_depth Maximum outstanding commands that can be queued.
 * @param fpa_pool  FPA pool the command queues should come from.
 * @param pool_size Size of each buffer in the FPA pool (bytes)
 *
 * @return CVMX_CMD_QUEUE_SUCCESS or a failure code
 */
cvmx_cmd_queue_result_t cvmx_cmd_queue_initialize(cvmx_cmd_queue_id_t queue_id,
						  int max_depth, int fpa_pool,
						  int pool_size);

/**
 * Shutdown a queue a free it's command buffers to the FPA. The
 * hardware connected to the queue must be stopped before this
 * function is called.
 *
 * @param queue_id Queue to shutdown
 *
 * @return CVMX_CMD_QUEUE_SUCCESS or a failure code
 */
cvmx_cmd_queue_result_t cvmx_cmd_queue_shutdown(cvmx_cmd_queue_id_t queue_id);

/**
 * Return the number of command words pending in the queue. This
 * function may be relatively slow for some hardware units.
 *
 * @param queue_id Hardware command queue to query
 *
 * @return Number of outstanding commands
 */
int cvmx_cmd_queue_length(cvmx_cmd_queue_id_t queue_id);

/**
 * Return the command buffer to be written to. The purpose of this
 * function is to allow CVMX routine access to the low level buffer
 * for initial hardware setup. User applications should not call this
 * function directly.
 *
 * @param queue_id Command queue to query
 *
 * @return Command buffer or NULL on failure
 */
void *cvmx_cmd_queue_buffer(cvmx_cmd_queue_id_t queue_id);

/**
 * @INTERNAL
 * Retreive or allocate command queue state named block
 */
extern cvmx_cmd_queue_result_t __cvmx_cmd_queue_init_state_ptr(unsigned node);

/**
 * @INTERNAL
 * Get the index into the state arrays for the supplied queue id.
 *
 * @param queue_id Queue ID to get an index for
 *
 * @return Index into the state arrays
 */
static inline unsigned __cvmx_cmd_queue_get_index(cvmx_cmd_queue_id_t queue_id)
{
	/* Warning: This code currently only works with devices that have 256
	 * queues or less.  Devices with more than 16 queues are laid out in
	 * memory to allow cores quick access to every 16th queue. This reduces
	 * cache thrashing when you are running 16 queues per port to support
	 * lockless operation
	 */
	unsigned unit = (queue_id >> 16) & 0xff;
	unsigned q = (queue_id >> 4) & 0xf;
	unsigned core = queue_id & 0xf;
	return (unit << 8) | (core << 4) | q;
}

static inline int __cvmx_cmd_queue_get_node(cvmx_cmd_queue_id_t queue_id)
{
	unsigned node = queue_id >> 24;
	return node;
}

/**
 * @INTERNAL
 * Lock the supplied queue so nobody else is updating it at the same
 * time as us.
 *
 * @param queue_id Queue ID to lock
 * 
 */
static inline void __cvmx_cmd_queue_lock(cvmx_cmd_queue_id_t queue_id)
{
#ifndef __U_BOOT__
	__cvmx_cmd_queue_lock_t *lock_ptr;
	unsigned node;
	uint64_t tmp;
	uint64_t my_ticket;

	tmp = __cvmx_cmd_queue_get_index(queue_id);
	node = __cvmx_cmd_queue_get_node(queue_id);
	lock_ptr = &__cvmx_cmd_queue_state_ptrs[node]->lock[tmp];

	asm volatile (".set push\n"
		      ".set noreorder\n"
		      /* Atomic incremebt of 'ticket' with LL/SC */
		      "1:\n"
		      "lld     %[my_ticket], %[ticket_ptr]\n"
		      "daddiu  %[ticket], %[my_ticket], 1\n"
		      "scd     %[ticket], %[ticket_ptr]\n"
		      "beqz    %[ticket], 1b\n"
		      " lld    %[ticket], %[now_serving]\n"
		      "2:\n"
		      /* Wait until 'now_serving == ticket' with LL/PAUSE */
		      "beq    %[ticket], %[my_ticket], 3f\n"
		      " nop\n pause\n"	/* PAUSE is not allowed in delay slot */
		      "b      2b\n"	/* check now_serving again */
		      " lld    %[ticket], %[now_serving]\n"
		      "3:\n"
		      ".set pop\n"
                      : [ticket_ptr] "=m"(lock_ptr->ticket),
                      [now_serving] "=m"(lock_ptr->now_serving),
                      [ticket] "=&r"(tmp),
                      [my_ticket] "=&r"(my_ticket)
	    );
#endif
}

/**
 * @INTERNAL
 * Unlock the queue, flushing all writes.
 *
 * @param queue_id Queue ID to lock
 * 
 */
static inline void __cvmx_cmd_queue_unlock(cvmx_cmd_queue_id_t queue_id)
{
#ifndef __U_BOOT__
	__cvmx_cmd_queue_lock_t *lock_ptr;
	uint64_t *ns_ptr;
	uint64_t ns;
	unsigned node;
	int tmp;

	/* Order queue manipulation with respect to the unlock.  */
	CVMX_SYNCWS;

	tmp = __cvmx_cmd_queue_get_index(queue_id);
	node = __cvmx_cmd_queue_get_node(queue_id);

	lock_ptr = &__cvmx_cmd_queue_state_ptrs[node]->lock[tmp];
	ns_ptr = &lock_ptr->now_serving;

	/* Incremebt 'now_sercving to allow next contender in */
	ns = (*ns_ptr) + 1;
	(*ns_ptr) = ns;
#endif
	CVMX_SYNCWS;		/* nudge out the unlock. */
}

/**
 * @INTERNAL
 * Initialize a command-queue lock to "unlocked" state.
 */
static inline void __cvmx_cmd_queue_lock_init(cvmx_cmd_queue_id_t queue_id)
{
	unsigned index = __cvmx_cmd_queue_get_index(queue_id);
	unsigned node = __cvmx_cmd_queue_get_node(queue_id);

	__cvmx_cmd_queue_state_ptrs[node]->lock[index] =
			(__cvmx_cmd_queue_lock_t){0, 0};
	CVMX_SYNCWS;
}

/**
 * @INTERNAL
 * Get the queue state structure for the given queue id
 *
 * @param queue_id Queue id to get
 *
 * @return Queue structure or NULL on failure
 */
static inline __cvmx_cmd_queue_state_t *
__cvmx_cmd_queue_get_state(cvmx_cmd_queue_id_t queue_id)
{
	unsigned index;
	unsigned node;
	__cvmx_cmd_queue_state_t *qptr;

	if (CVMX_ENABLE_PARAMETER_CHECKING) {
		if (cvmx_unlikely(queue_id >= CVMX_CMD_QUEUE_END))
			return NULL;
		if (cvmx_unlikely((queue_id & 0xffff) >= 256))
			return NULL;
	}
	node = __cvmx_cmd_queue_get_node(queue_id);
	index = __cvmx_cmd_queue_get_index(queue_id);

	if (cvmx_unlikely(__cvmx_cmd_queue_state_ptrs[node] == NULL))
		__cvmx_cmd_queue_init_state_ptr(node);

	qptr = &__cvmx_cmd_queue_state_ptrs[node]->state[index];
	return qptr;
}

/**
 * Write an arbitrary number of command words to a command queue.
 * This is a generic function; the fixed number of command word
 * functions yield higher performance.
 *
 * @param queue_id  Hardware command queue to write to
 * @param use_locking
 *                  Use internal locking to ensure exclusive access for queue
 *                  updates. If you don't use this locking you must ensure
 *                  exclusivity some other way. Locking is strongly recommended.
 * @param cmd_count Number of command words to write
 * @param cmds      Array of commands to write
 *
 * @return CVMX_CMD_QUEUE_SUCCESS or a failure code
 */
static inline cvmx_cmd_queue_result_t
cvmx_cmd_queue_write(cvmx_cmd_queue_id_t queue_id, bool use_locking,
		     int cmd_count, const uint64_t *cmds)
{
	cvmx_cmd_queue_result_t ret = CVMX_CMD_QUEUE_SUCCESS;
	uint64_t *cmd_ptr;

	__cvmx_cmd_queue_state_t *qptr = __cvmx_cmd_queue_get_state(queue_id);

	if (CVMX_ENABLE_PARAMETER_CHECKING) {
		if (cvmx_unlikely(qptr == NULL))
			return CVMX_CMD_QUEUE_INVALID_PARAM;
		if (cvmx_unlikely((cmd_count < 1) || (cmd_count > 32)))
			return CVMX_CMD_QUEUE_INVALID_PARAM;
		if (cvmx_unlikely(cmds == NULL))
			return CVMX_CMD_QUEUE_INVALID_PARAM;
	}

	/* Make sure nobody else is updating the same queue */
	if (cvmx_likely(use_locking))
		__cvmx_cmd_queue_lock(queue_id);

	/* Most of the time there is lots of free words in current block */
	if (cvmx_unlikely(
	    (qptr->index + cmd_count) >= qptr->pool_size_m1)) {
		/* The rare case when nearing end of block */
		ret = __cvmx_cmd_queue_write_raw(queue_id, qptr,
			cmd_count, cmds);
	} else {
		cmd_ptr = (uint64_t *)cvmx_phys_to_ptr((uint64_t) qptr->base_paddr);
		/* Loop easy for compiler to unroll for the likely case */
		while (cmd_count > 0) {
			cmd_ptr[ qptr->index ++ ] = *cmds++;
			cmd_count --;
		}
	}

	/* All updates are complete. Release the lock and return */
	if (cvmx_likely(use_locking))
		__cvmx_cmd_queue_unlock(queue_id);
	else
		CVMX_SYNCWS;

	return ret;
}

/**
 * Simple function to write two command words to a command queue.
 *
 * @param queue_id Hardware command queue to write to
 * @param use_locking
 *                 Use internal locking to ensure exclusive access for queue
 *                 updates. If you don't use this locking you must ensure
 *                 exclusivity some other way. Locking is strongly recommended.
 * @param cmd1     Command
 * @param cmd2     Command
 *
 * @return CVMX_CMD_QUEUE_SUCCESS or a failure code
 */
static inline cvmx_cmd_queue_result_t
cvmx_cmd_queue_write2(cvmx_cmd_queue_id_t queue_id, bool use_locking,
		      uint64_t cmd1, uint64_t cmd2)
{
	cvmx_cmd_queue_result_t ret = CVMX_CMD_QUEUE_SUCCESS;
	uint64_t *cmd_ptr;

	__cvmx_cmd_queue_state_t *qptr = __cvmx_cmd_queue_get_state(queue_id);

	if (CVMX_ENABLE_PARAMETER_CHECKING) {
		if (cvmx_unlikely(qptr == NULL))
			return CVMX_CMD_QUEUE_INVALID_PARAM;
	}

	/* Make sure nobody else is updating the same queue */
	if (cvmx_likely(use_locking))
		__cvmx_cmd_queue_lock(queue_id);

	if (cvmx_unlikely((qptr->index + 2) >= qptr->pool_size_m1)) {
		/* The rare case when nearing end of block */
		uint64_t cmds[2];
		cmds[0] = cmd1;
		cmds[1] = cmd2;
		ret = __cvmx_cmd_queue_write_raw(queue_id, qptr, 2, cmds);
	} else {
		/* Likely case to work fast */
		cmd_ptr = (uint64_t *)cvmx_phys_to_ptr((uint64_t) qptr->base_paddr);
		cmd_ptr += qptr->index;
		qptr->index += 2;
		cmd_ptr[0] = cmd1;
		cmd_ptr[1] = cmd2;
	}

	/* All updates are complete. Release the lock and return */
	if (cvmx_likely(use_locking))
		__cvmx_cmd_queue_unlock(queue_id);
	else
		CVMX_SYNCWS;

	return ret;
}

/**
 * Simple function to write three command words to a command queue.
 *
 * @param queue_id Hardware command queue to write to
 * @param use_locking
 *                 Use internal locking to ensure exclusive access for queue
 *                 updates. If you don't use this locking you must ensure
 *                 exclusivity some other way. Locking is strongly recommended.
 * @param cmd1     Command
 * @param cmd2     Command
 * @param cmd3     Command
 *
 * @return CVMX_CMD_QUEUE_SUCCESS or a failure code
 */
static inline cvmx_cmd_queue_result_t
cvmx_cmd_queue_write3(cvmx_cmd_queue_id_t queue_id, bool use_locking,
		      uint64_t cmd1, uint64_t cmd2, uint64_t cmd3)
{
	cvmx_cmd_queue_result_t ret = CVMX_CMD_QUEUE_SUCCESS;
	uint64_t *cmd_ptr;

	__cvmx_cmd_queue_state_t *qptr = __cvmx_cmd_queue_get_state(queue_id);

	if (CVMX_ENABLE_PARAMETER_CHECKING) {
		if (cvmx_unlikely(qptr == NULL))
			return CVMX_CMD_QUEUE_INVALID_PARAM;
	}

	/* Make sure nobody else is updating the same queue */
	if (cvmx_likely(use_locking))
		__cvmx_cmd_queue_lock(queue_id);

	if (cvmx_unlikely((qptr->index + 3) >= qptr->pool_size_m1)) {
	/* Most of the time there is lots of free words in current block */
		uint64_t cmds[3];
		cmds[0] = cmd1;
		cmds[1] = cmd2;
		cmds[2] = cmd3;
		ret = __cvmx_cmd_queue_write_raw(queue_id, qptr, 3, cmds);
	} else {
		cmd_ptr = (uint64_t *)cvmx_phys_to_ptr((uint64_t) qptr->base_paddr);
		cmd_ptr += qptr->index;
		qptr->index += 3;
		cmd_ptr[0] = cmd1;
		cmd_ptr[1] = cmd2;
		cmd_ptr[2] = cmd3;
	}

	/* All updates are complete. Release the lock and return */
	if (cvmx_likely(use_locking))
		__cvmx_cmd_queue_unlock(queue_id);
	else
		CVMX_SYNCWS;

	return ret;
}

#ifdef	__cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif

#endif /* __CVMX_CMD_QUEUE_H__ */
