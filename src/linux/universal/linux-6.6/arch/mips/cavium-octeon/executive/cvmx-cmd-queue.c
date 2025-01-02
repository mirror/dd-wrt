/***********************license start***************
 * Copyright (c) 2003-2013  Cavium Inc. (support@cavium.com). All rights
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
 * <hr>$Revision: 112009 $<hr>
 */
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <linux/export.h>
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-bootmem.h>
#include <asm/octeon/cvmx-npei-defs.h>
#include <asm/octeon/cvmx-pexp-defs.h>
#include <asm/octeon/cvmx-dpi-defs.h>
#include <asm/octeon/cvmx-pko-defs.h>
#include <asm/octeon/cvmx-fpa.h>
#include <asm/octeon/cvmx-cmd-queue.h>
#else
#include "cvmx.h"
#include "cvmx-bootmem.h"
#include "cvmx-fpa.h"
#include "cvmx-cmd-queue.h"
#endif

/**
 * This application uses this pointer to access the global queue
 * state. It points to a bootmem named block.
 */
CVMX_SHARED __cvmx_cmd_queue_all_state_t *__cvmx_cmd_queue_state_ptrs[CVMX_MAX_NODES];
EXPORT_SYMBOL(__cvmx_cmd_queue_state_ptrs);

/**
 * @INTERNAL
 * Initialize the Global queue state pointer.
 *
 * @return CVMX_CMD_QUEUE_SUCCESS or a failure code
 */
cvmx_cmd_queue_result_t __cvmx_cmd_queue_init_state_ptr(unsigned node)
{
	char *alloc_name = "cvmx_cmd_queues\0\0";
	char s[4] = "_0";
	const cvmx_bootmem_named_block_desc_t *block_desc = NULL;
	unsigned size;
	uint64_t paddr_min = 0, paddr_max = 0;
	void *ptr;

#if defined(CONFIG_CAVIUM_RESERVE32) && CONFIG_CAVIUM_RESERVE32
#endif

	if (cvmx_likely(__cvmx_cmd_queue_state_ptrs[node]))
		return CVMX_CMD_QUEUE_SUCCESS;

	/* Add node# to block name */
	if (node > 0) {
		s[1] += node;
		strcat(alloc_name, s);
	}

	/* Find the named block in case it has been created already */
	block_desc = cvmx_bootmem_find_named_block(alloc_name);
	if (block_desc) {
		__cvmx_cmd_queue_state_ptrs[node] =
			cvmx_phys_to_ptr(block_desc->base_addr);
		return CVMX_CMD_QUEUE_SUCCESS;
	}

	size = sizeof(*__cvmx_cmd_queue_state_ptrs[node]);

	/* Rest f the code is to allocate a new named block */
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#if defined(CONFIG_CAVIUM_RESERVE32) && CONFIG_CAVIUM_RESERVE32
	{
		/* Special address range for SE-UM apps in 32-bit mode */
		extern uint64_t octeon_reserve32_memory;
		if (octeon_reserve32_memory) {
			paddr_min = octeon_reserve32_memory;
			paddr_max = octeon_reserve32_memory +
				(CONFIG_CAVIUM_RESERVE32 << 20) - 1;
		}
	}
#endif
#endif

	/* Atomically allocate named block once, and zero it by default */
	ptr = cvmx_bootmem_alloc_named_range_once(size, paddr_min, paddr_max, 
			128, alloc_name, NULL);

	if (ptr != NULL) {
		__cvmx_cmd_queue_state_ptrs[node] = ptr;
	} else {
		cvmx_dprintf("ERROR: %s: Unable to get named block %s.\n", 
			__func__, alloc_name);
		return CVMX_CMD_QUEUE_NO_MEMORY;
	}
	return CVMX_CMD_QUEUE_SUCCESS;
}
EXPORT_SYMBOL(__cvmx_cmd_queue_init_state_ptr);

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
						  int pool_size)
{
	__cvmx_cmd_queue_state_t *qstate;
	cvmx_cmd_queue_result_t result;
	unsigned node;
	unsigned index;
	int fpa_pool_min, fpa_pool_max;

	node = __cvmx_cmd_queue_get_node(queue_id);

	index = __cvmx_cmd_queue_get_index(queue_id);
	if (index >= NUM_ELEMENTS(__cvmx_cmd_queue_state_ptrs[node]->state)){
		cvmx_printf("ERROR: %s: queue %#x out of range\n",
			__func__, queue_id);
		return CVMX_CMD_QUEUE_INVALID_PARAM;
	}

	result = __cvmx_cmd_queue_init_state_ptr(node);
	if (result != CVMX_CMD_QUEUE_SUCCESS)
		return result;

	qstate = __cvmx_cmd_queue_get_state(queue_id);
	if (qstate == NULL)
		return CVMX_CMD_QUEUE_INVALID_PARAM;

	/*
	 * We artificially limit max_depth to 1<<20 words. It is an
	 * arbitrary limit.
	 */
	if (CVMX_CMD_QUEUE_ENABLE_MAX_DEPTH) {
		if ((max_depth < 0) || (max_depth > 1 << 20))
			return CVMX_CMD_QUEUE_INVALID_PARAM;
	} else if (max_depth != 0)
		return CVMX_CMD_QUEUE_INVALID_PARAM;

	/* CVMX_FPA_NUM_POOLS maps to cvmx_fpa3_num_auras for FPA3 */
	fpa_pool_min = node << 10;
	fpa_pool_max = fpa_pool_min + CVMX_FPA_NUM_POOLS;

	if ((fpa_pool < fpa_pool_min) || (fpa_pool >= fpa_pool_max))
		return CVMX_CMD_QUEUE_INVALID_PARAM;

	if ((pool_size < 128) || (pool_size > (1<<17)))
		return CVMX_CMD_QUEUE_INVALID_PARAM;

	if (pool_size & 3)
		cvmx_dprintf("WARNING: %s: pool_size %d not multiple of 8\n",
			__func__, pool_size);

	/* See if someone else has already initialized the queue */
	if (qstate->base_paddr) {
		int depth;
		static const char emsg[] = /* Common error message part */
			"Queue already initialized with different ";

		depth = (max_depth + qstate->pool_size_m1 - 1) /
			qstate->pool_size_m1;
		if (depth != qstate->max_depth) {
			depth = qstate->max_depth * qstate->pool_size_m1;
			cvmx_dprintf("ERROR: %s: %s max_depth (%d).\n",
				__func__, emsg, depth);
			return CVMX_CMD_QUEUE_INVALID_PARAM;
		}
		if (fpa_pool != qstate->fpa_pool) {
			cvmx_dprintf("ERROR: %s: %s FPA pool (%u).\n",
				__func__, emsg, qstate->fpa_pool);
			return CVMX_CMD_QUEUE_INVALID_PARAM;
		}
		if ((pool_size >> 3) - 1 != qstate->pool_size_m1) {
			cvmx_dprintf("ERROR: %s: %s FPA pool size (%u).\n",
				__func__, emsg,
				(qstate->pool_size_m1 + 1) << 3);
			return CVMX_CMD_QUEUE_INVALID_PARAM;
		}
		return CVMX_CMD_QUEUE_ALREADY_SETUP;
	} else {
		union cvmx_fpa_ctl_status status;
		void *buffer;

		if (!(octeon_has_feature(OCTEON_FEATURE_FPA3))) {
			status.u64 = cvmx_read_csr(CVMX_FPA_CTL_STATUS);
			if (!status.s.enb) {
				cvmx_dprintf("ERROR: %s: FPA is not enabled.\n",
					__func__);
				return CVMX_CMD_QUEUE_NO_MEMORY;
			}
		}
		buffer = cvmx_fpa_alloc(fpa_pool);
		if (buffer == NULL) {
			cvmx_dprintf("ERROR: %s: allocating first buffer.\n",
				__func__);
			return CVMX_CMD_QUEUE_NO_MEMORY;
		}

		index = (pool_size >> 3) - 1;
		qstate->pool_size_m1 = index;
		qstate->max_depth = (max_depth + index -1) / index;
		qstate->index = 0;
		qstate->fpa_pool = fpa_pool;
		qstate->base_paddr = cvmx_ptr_to_phys(buffer);

		/* Initialize lock */
		__cvmx_cmd_queue_lock_init(queue_id);
		return CVMX_CMD_QUEUE_SUCCESS;
	}
}

EXPORT_SYMBOL(cvmx_cmd_queue_initialize);

/**
 * Shutdown a queue a free it's command buffers to the FPA. The
 * hardware connected to the queue must be stopped before this
 * function is called.
 *
 * @param queue_id Queue to shutdown
 *
 * @return CVMX_CMD_QUEUE_SUCCESS or a failure code
 */
cvmx_cmd_queue_result_t cvmx_cmd_queue_shutdown(cvmx_cmd_queue_id_t queue_id)
{
	__cvmx_cmd_queue_state_t *qptr = __cvmx_cmd_queue_get_state(queue_id);

	/* FIXME: This will not complain if the queue was never initialized */
	if (qptr == NULL) {
		cvmx_dprintf("ERROR: cvmx_cmd_queue_shutdown: Unable to get queue information.\n");
		return CVMX_CMD_QUEUE_INVALID_PARAM;
	}

	if (cvmx_cmd_queue_length(queue_id) > 0) {
		cvmx_dprintf("ERROR: cvmx_cmd_queue_shutdown: Queue still has data in it.\n");
		return CVMX_CMD_QUEUE_FULL;
	}

	__cvmx_cmd_queue_lock(queue_id);
	if (qptr->base_paddr) {
		cvmx_fpa_free(cvmx_phys_to_ptr(
				       (uint64_t) qptr->base_paddr),
				       qptr->fpa_pool, 0);
		qptr->base_paddr = 0;
	}
	__cvmx_cmd_queue_unlock(queue_id);

	return CVMX_CMD_QUEUE_SUCCESS;
}
EXPORT_SYMBOL(cvmx_cmd_queue_shutdown);

/**
 * Return the number of command words pending in the queue. This
 * function may be relatively slow for some hardware units.
 *
 * @param queue_id Hardware command queue to query
 *
 * @return Number of outstanding commands
 */
int cvmx_cmd_queue_length(cvmx_cmd_queue_id_t queue_id)
{
	if (CVMX_ENABLE_PARAMETER_CHECKING) {
		if (__cvmx_cmd_queue_get_state(queue_id) == NULL)
			return CVMX_CMD_QUEUE_INVALID_PARAM;
	}

	/*
	 * The cast is here so gcc with check that all values in the
	 * cvmx_cmd_queue_id_t enumeration are here.
	 */
	switch ((cvmx_cmd_queue_id_t) (queue_id & 0xff0000)) {
	case CVMX_CMD_QUEUE_PKO_BASE:
		/*
		 * Really need atomic lock on
		 * CVMX_PKO_REG_READ_IDX. Right now we are normally
		 * called with the queue lock, so that is a SLIGHT
		 * amount of protection.
		 */
		cvmx_write_csr(CVMX_PKO_REG_READ_IDX, queue_id & 0xffff);
		if (OCTEON_IS_MODEL(OCTEON_CN3XXX)) {
			union cvmx_pko_mem_debug9 debug9;
			debug9.u64 = cvmx_read_csr(CVMX_PKO_MEM_DEBUG9);
			return debug9.cn38xx.doorbell;
		} else {
			union cvmx_pko_mem_debug8 debug8;
			debug8.u64 = cvmx_read_csr(CVMX_PKO_MEM_DEBUG8);
			if (octeon_has_feature(OCTEON_FEATURE_PKND))
				return debug8.cn68xx.doorbell;
			else
				return debug8.cn58xx.doorbell;
		}
	case CVMX_CMD_QUEUE_ZIP:
	case CVMX_CMD_QUEUE_DFA:
	case CVMX_CMD_QUEUE_HNA:
	case CVMX_CMD_QUEUE_RAID:
		/* Still need to implement other lengths */
		return 0;
	case CVMX_CMD_QUEUE_DMA_BASE:
		if (octeon_has_feature(OCTEON_FEATURE_NPEI)) {
			union cvmx_npei_dmax_counts dmax_counts;
			dmax_counts.u64 = cvmx_read_csr(CVMX_PEXP_NPEI_DMAX_COUNTS(queue_id & 0x7));
			return dmax_counts.s.dbell;
		} else {
			union cvmx_dpi_dmax_counts dmax_counts;
			dmax_counts.u64 = cvmx_read_csr(CVMX_DPI_DMAX_COUNTS(queue_id & 0x7));
			return dmax_counts.s.dbell;
		}
	case CVMX_CMD_QUEUE_BCH:
		/* Not available */
		return 0;
	case CVMX_CMD_QUEUE_END:
		return CVMX_CMD_QUEUE_INVALID_PARAM;
	}
	return CVMX_CMD_QUEUE_INVALID_PARAM;
}

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
void *cvmx_cmd_queue_buffer(cvmx_cmd_queue_id_t queue_id)
{
	__cvmx_cmd_queue_state_t *qptr = __cvmx_cmd_queue_get_state(queue_id);
	if (qptr && qptr->base_paddr)
		return cvmx_phys_to_ptr((uint64_t) qptr->base_paddr);
	else
		return NULL;
}
EXPORT_SYMBOL(cvmx_cmd_queue_buffer);

static uint64_t *__cvmx_cmd_queue_add_blk(__cvmx_cmd_queue_state_t *qptr)
{
	uint64_t *cmd_ptr;
	uint64_t *new_buffer;
	uint64_t new_paddr;

	/* Get base vaddr of current (full) block */
	cmd_ptr = cvmx_phys_to_ptr((uint64_t) qptr->base_paddr);

	/* Allocate a new block from the per-queue pool */
	new_buffer = cvmx_fpa_alloc(qptr->fpa_pool);

	/* Check for allocation failure */
	if (cvmx_unlikely(new_buffer == NULL))
		return NULL;

	/* Zero out the new block link pointer,
	 * in case this block will be filled to the rim
	 */
	new_buffer[ qptr->pool_size_m1 ] = ~0ull;

	/* Get physical address of the new buffer */
	new_paddr = cvmx_ptr_to_phys(new_buffer);

	/* Store the physical link address at the end of current full block */
	cmd_ptr[ qptr->pool_size_m1] = new_paddr;

	/* Store the physical address in the queue state structure */
	qptr->base_paddr = new_paddr;
	qptr->index = 0;

	/* Return the virtual base of the new block */
	return new_buffer;
}

/**
 * @INTERNAL
 * Add command words into a queue, handles all the corener cases
 * where only some of the words might fit into the current block,
 * and a new block may need to be allocated.
 * Locking and argument checks are done in the front-end in-line
 * functions that call this one for the rare corner cases.
 */
cvmx_cmd_queue_result_t
__cvmx_cmd_queue_write_raw(cvmx_cmd_queue_id_t queue_id,
	__cvmx_cmd_queue_state_t *qptr,
	int cmd_count, const uint64_t *cmds)
{
	uint64_t *cmd_ptr;
	unsigned index;

	cmd_ptr = cvmx_phys_to_ptr((uint64_t) qptr->base_paddr);
	index = qptr->index;

	/* Enforce queue depth limit, if enabled, once per block */
	if (CVMX_CMD_QUEUE_ENABLE_MAX_DEPTH &&
	    cvmx_unlikely(qptr->max_depth)) {
		unsigned depth = cvmx_cmd_queue_length(queue_id);
		depth /= qptr->pool_size_m1;

		if (cvmx_unlikely(depth > qptr->max_depth)) {
			return CVMX_CMD_QUEUE_FULL;
		}
	}

	/*
	 * If the block allocation fails, even the words that we wrote
	 * to the current block will not count because the 'index' will
	 * not be comitted.
	 * The loop is run 'count + 1' times to take care of the tail
	 * case, where the buffer is full to the rim, so the link
	 * pointer must be filled with a valid address.
	 */
	while (cmd_count >= 0) {
		if (index >= qptr->pool_size_m1) {
			/* Block is full, get another one and proceed */
			cmd_ptr = __cvmx_cmd_queue_add_blk(qptr);

			/* Baul on allocation error w/o comitting anything */
			if (cvmx_unlikely(cmd_ptr == NULL))
				return CVMX_CMD_QUEUE_NO_MEMORY;

			/* Reset index for start of new block */
			index = 0;
		}
		/* Exit Loop on 'count + 1' iterations */
		if (cmd_count <= 0)
			break;
		/* Store commands into queue block while there is space */
		cmd_ptr[ index ++ ] = *cmds++;
		cmd_count --;
	} /* while cmd_count */

	/* Commit added words if all is well */
	qptr->index = index;

	return CVMX_CMD_QUEUE_SUCCESS;
}
EXPORT_SYMBOL(__cvmx_cmd_queue_write_raw);
