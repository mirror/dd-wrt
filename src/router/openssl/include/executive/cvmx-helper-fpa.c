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
 * Helper functions for FPA setup.
 *
 * <hr>$Revision: 75216 $<hr>
 */
#include "cvmx.h"
#include "cvmx-bootmem.h"
#include "cvmx-fpa.h"
#include "cvmx-helper-fpa.h"
#include "cvmx-ipd.h"
#include "cvmx-dfa.h"
#include "cvmx-zip.h"
#include "cvmx-tim.h"
#include "cvmx-pko.h"

int fpa_helper_debug = 0;

/**
 * @INTERNAL
 * Setup a FPA3 pool and aura to control a new block of memory. This
 * function is called by legacy code.
 *
 * @param pool       Pool to initialize
 * @param name       Pool name
 * @param buffers    Pointer to block of memory to use for the aura buffers
 * @param block_size Size of the aura buffers
 * @param num_blocks Number of aura buffers
 *
 * @return 0 on Success, -1 on failure
 */
int __cvmx_fpa3_setup_pool(uint64_t pool, const char *name, void *buffers,
			   uint64_t block_size, uint64_t num_blocks)
{
	int	aura;
	int	pool_num = (int)pool;

	/* We hardcode the aura to have the same value as the pool */
	aura = pool_num;

	/* Allocate the pool from the global resources */
	if (cvmx_fpa_allocate_fpa_pools(0, &pool_num, 1) < 0)
		return -1;

	/* Allocate the aura from the global resources */
	if (cvmx_fpa_allocate_auras(0, &aura, 1) < 0)
		return -1;

	/* Initialize the buffer pointers */
	if (cvmx_fpa_pool_stack_init(0, pool_num, name, 0, (int)num_blocks,
				     FPA_NATURAL_ALIGNMENT, (int)block_size))
		return -1;

	/* Associate the aura to the pool */
	if (cvmx_fpa_assign_aura(0, aura, pool_num))
		return -1;

	/* Configure the aura and allocate the buffers */
	if (cvmx_fpa_aura_init(0, aura, name, 0, buffers, (int)num_blocks, 0))
		return -1;

	return 0;
}

/**
 * @INTERNAL
 * Allocate memory for and initialize a single FPA pool.
 *
 * @param pool    Pool to initialize
 * @param buffer_size  Size of buffers to allocate in bytes
 * @param buffers Number of buffers to put in the pool. Zero is allowed
 * @param name    String name of the pool for debugging purposes
 * @return Zero on success, non-zero on failure
 */
int __cvmx_helper_initialize_fpa_pool(int pool, uint64_t buffer_size,
				      uint64_t buffers, const char *name)
{
	uint64_t current_num;
	void *memory;
	uint64_t align = CVMX_CACHE_LINE_SIZE;

	/* Align the allocation so that power of 2 size buffers are naturally aligned */
	while (align < buffer_size)
		align = align << 1;

	if (buffers == 0)
		return 0;

	current_num = cvmx_read_csr(CVMX_FPA_QUEX_AVAILABLE(pool));
	if (current_num) {
		cvmx_dprintf("Fpa pool %d(%s) already has %llu buffers. "
			     "Skipping setup.\n",
			     pool, name, (unsigned long long)current_num);
		return 0;
	}
	if (fpa_helper_debug)
		cvmx_dprintf("pool=%20s pool=%02d bufsize=%04d buffers=%04d \n",
			     name, pool, (int)buffer_size, (int)buffers);
	if(cvmx_fpa_alloc_pool(pool) != pool) {
		cvmx_dprintf("\nINFO: pool %d is already in use", pool);
		return 0;
	}
	memory = cvmx_bootmem_alloc(buffer_size * buffers, align);
	if (memory == NULL) {
		cvmx_dprintf("Out of memory initializing fpa pool %d(%s).\n", pool, name);
		return -1;
	}
	cvmx_fpa_setup_pool(pool, name, memory, buffer_size, buffers);
	return 0;
}

/**
 * @INTERNAL
 * Allocate memory and initialize the FPA pools using memory
 * from cvmx-bootmem. Specifying zero for the number of
 * buffers will cause that FPA pool to not be setup. This is
 * useful if you aren't using some of the hardware and want
 * to save memory. Use cvmx_helper_initialize_fpa instead of
 * this function directly.
 *
 * @param pip_pool Should always be CVMX_FPA_PACKET_POOL
 * @param pip_size Should always be CVMX_FPA_PACKET_POOL_SIZE
 * @param pip_buffers
 *                 Number of packet buffers.
 * @param wqe_pool Should always be CVMX_FPA_WQE_POOL
 * @param wqe_size Should always be CVMX_FPA_WQE_POOL_SIZE
 * @param wqe_entries
 *                 Number of work queue entries
 * @param pko_pool Should always be CVMX_FPA_OUTPUT_BUFFER_POOL
 * @param pko_size Should always be CVMX_FPA_OUTPUT_BUFFER_POOL_SIZE
 * @param pko_buffers
 *                 PKO Command buffers. You should at minimum have two per
 *                 each PKO queue.
 * @param tim_pool Should always be CVMX_FPA_TIMER_POOL
 * @param tim_size Should always be CVMX_FPA_TIMER_POOL_SIZE
 * @param tim_buffers
 *                 TIM ring buffer command queues. At least two per timer bucket
 *                 is recommended.
 * @param dfa_pool Should always be CVMX_FPA_DFA_POOL
 * @param dfa_size Should always be CVMX_FPA_DFA_POOL_SIZE
 * @param dfa_buffers
 *                 DFA command buffer. A relatively small (32 for example)
 *                 number should work.
 * @return Zero on success, non-zero if out of memory
 */
static int
__cvmx_helper_initialize_fpa(int pip_pool, int pip_size, int pip_buffers,
			     int wqe_pool, int wqe_size, int wqe_entries,
			     int pko_pool, int pko_size, int pko_buffers,
			     int tim_pool, int tim_size, int tim_buffers,
			     int dfa_pool, int dfa_size, int dfa_buffers)
{
	int status;
#define __init_fpa_pool(pool, size, buffers, str)                              \
	if (pool >= 0) {                                                       \
		status = __cvmx_helper_initialize_fpa_pool(pool, size, buffers,\
							   str);	       \
		if (status)						       \
			return status;                                         \
	}

	cvmx_fpa_enable();

	if ((pip_buffers > 0) && (pip_buffers <= 64))
		cvmx_dprintf("Warning: %d packet buffers may not be enough for"
			     "hardware prefetch. 65 or more is recommended.\n",
			     pip_buffers);

	__init_fpa_pool(pip_pool, pip_size, pip_buffers, "Packet Buffers");
	__init_fpa_pool(wqe_pool, wqe_size, wqe_entries, "Work Q Entries");
	__init_fpa_pool(pko_pool, pko_size, pko_buffers, "PKO Cmd Buffers");
	__init_fpa_pool(tim_pool, tim_size, tim_buffers, "TIM Cmd Buffers");
	__init_fpa_pool(dfa_pool, dfa_size, dfa_buffers, "DFA Cmd Buffers");

	return 0;
}

/**
 * Allocate memory and initialize the FPA pools using memory
 * from cvmx-bootmem. Sizes of each element in the pools is
 * controlled by the cvmx-config.h header file. Specifying
 * zero for any parameter will cause that FPA pool to not be
 * setup. This is useful if you aren't using some of the
 * hardware and want to save memory.
 *
 * @param packet_buffers
 *               Number of packet buffers to allocate
 * @param work_queue_entries
 *               Number of work queue entries
 * @param pko_buffers
 *               PKO Command buffers. You should at minimum have two per
 *               each PKO queue.
 * @param tim_buffers
 *               TIM ring buffer command queues. At least two per timer bucket
 *               is recommended.
 * @param dfa_buffers
 *               DFA command buffer. A relatively small (32 for example)
 *               number should work.
 * @return Zero on success, non-zero if out of memory
 */
int cvmx_helper_initialize_fpa(int packet_buffers, int work_queue_entries,
			       int pko_buffers, int tim_buffers, int dfa_buffers)
{
	int packet_pool = (int)cvmx_fpa_get_packet_pool();
	int wqe_pool = (int)cvmx_fpa_get_wqe_pool();
	int outputbuffer_pool = (int)cvmx_fpa_get_pko_pool();
	int timer_pool = (int)cvmx_fpa_get_timer_pool();
	int dfa_pool = (int)cvmx_fpa_get_dfa_pool();
	int rv;


	rv = __cvmx_helper_initialize_fpa(packet_pool,
					  cvmx_fpa_get_packet_pool_block_size(),
					  packet_buffers,
					  wqe_pool,
					  cvmx_fpa_get_wqe_pool_block_size(),
					  work_queue_entries,
					  outputbuffer_pool,
					  cvmx_fpa_get_pko_pool_block_size(),
					  pko_buffers,
					  timer_pool,
					  cvmx_fpa_get_timer_pool_block_size(),
					  tim_buffers,
					  dfa_pool,
					  cvmx_fpa_get_dfa_pool_block_size(),
					  dfa_buffers
 					);

	return rv;
}
