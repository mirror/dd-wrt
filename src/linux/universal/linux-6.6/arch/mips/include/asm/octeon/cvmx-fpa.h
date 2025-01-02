/***********************license start***************
 * Copyright (c) 2003-2010  Cavium Inc. (support@cavium.com). All rights
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
 * Interface to the hardware Free Pool Allocator.
 *
 * <hr>$Revision: 120123 $<hr>
 *
 */

#ifndef __CVMX_FPA_H__
#define __CVMX_FPA_H__

#include "cvmx-scratch.h"
#include "cvmx.h"

#include "cvmx-fpa-defs.h"
#include "cvmx-fpa1.h"
#include "cvmx-fpa3.h"

#define CVMX_FPA_MIN_BLOCK_SIZE 128
#define CVMX_FPA_ALIGNMENT      128
#define CVMX_FPA_POOL_NAME_LEN  16

/* On CN78XX in backward-compatible mode, pool is mapped to AURA */
#define CVMX_FPA_NUM_POOLS (octeon_has_feature(OCTEON_FEATURE_FPA3) ? \
			cvmx_fpa3_num_auras() : CVMX_FPA1_NUM_POOLS)

#ifdef	__cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

/**
 * Structure to store FPA pool configuration parameters.
 */
struct cvmx_fpa_pool_config {
	int64_t pool_num;
	uint64_t buffer_size;
	uint64_t buffer_count;
};
typedef struct cvmx_fpa_pool_config cvmx_fpa_pool_config_t;

/**
 * Return the name of the pool
 *
 * @param pool_num   Pool to get the name of
 * @return The name
 */
const char *cvmx_fpa_get_name(int pool_num);

/**
 * Initialize FPA per node
 */
int cvmx_fpa_global_init_node(int node);

/**
 * Enable the FPA
 */
static inline void cvmx_fpa_enable(void)
{
	if (!octeon_has_feature(OCTEON_FEATURE_FPA3))
		cvmx_fpa1_enable();
	else
		cvmx_fpa_global_init_node(cvmx_get_node_num());
}

/**
 * Disable the FPA
 */
static inline void cvmx_fpa_disable(void)
{
	if (!octeon_has_feature(OCTEON_FEATURE_FPA3))
		cvmx_fpa1_disable();
	/* FPA3 does not have a disable funcion */
}

/**
 * @INTERNAL
 * @deprecated OBSOLETE
 *
 * Kept for transition assistance only
 */
static inline void cvmx_fpa_global_initialize(void)
{
	cvmx_fpa_global_init_node(cvmx_get_node_num());
}


/**
 * @INTERNAL
 *
 * Convert FPA1 style POOL into FPA3 AURA in
 * backward compatibility mode.
 */
static inline cvmx_fpa3_gaura_t
cvmx_fpa1_pool_to_fpa3_aura(cvmx_fpa1_pool_t pool)
{
	if ((octeon_has_feature(OCTEON_FEATURE_FPA3))) {
		unsigned node = cvmx_get_node_num();
		cvmx_fpa3_gaura_t aura = __cvmx_fpa3_gaura(node, pool);
		return aura;
	}
	return CVMX_FPA3_INVALID_GAURA;
}

/**
 * Get a new block from the FPA
 *
 * @param pool   Pool to get the block from
 * @return Pointer to the block or NULL on failure
 */
static inline void *cvmx_fpa_alloc(uint64_t pool)
{
	/* FPA3 is handled differently */
	if ((octeon_has_feature(OCTEON_FEATURE_FPA3))) {
		return cvmx_fpa3_alloc(
			cvmx_fpa1_pool_to_fpa3_aura(pool));
	} else
		return cvmx_fpa1_alloc(pool);
}

/**
 * Asynchronously get a new block from the FPA
 *
 * The result of cvmx_fpa_async_alloc() may be retrieved using
 * cvmx_fpa_async_alloc_finish().
 *
 * @param scr_addr Local scratch address to put response in.  This is a byte
 *		   address but must be 8 byte aligned.
 * @param pool      Pool to get the block from
 */
static inline void cvmx_fpa_async_alloc(uint64_t scr_addr, uint64_t pool)
{
	if ((octeon_has_feature(OCTEON_FEATURE_FPA3))) {
		return cvmx_fpa3_async_alloc(scr_addr,
			cvmx_fpa1_pool_to_fpa3_aura(pool));
	} else
		return cvmx_fpa1_async_alloc(scr_addr, pool);
}

/**
 * Retrieve the result of cvmx_fpa_async_alloc
 *
 * @param scr_addr The Local scratch address.  Must be the same value
 * passed to cvmx_fpa_async_alloc().
 *
 * @param pool Pool the block came from.  Must be the same value
 * passed to cvmx_fpa_async_alloc.
 *
 * @return Pointer to the block or NULL on failure
 */
static inline void *cvmx_fpa_async_alloc_finish(uint64_t scr_addr, uint64_t pool)
{
	if ((octeon_has_feature(OCTEON_FEATURE_FPA3)))
		return cvmx_fpa3_async_alloc_finish(
			scr_addr, cvmx_fpa1_pool_to_fpa3_aura(pool));
	else
		return cvmx_fpa1_async_alloc_finish(
			scr_addr, pool);
}

/**
 * Free a block allocated with a FPA pool.
 * Does NOT provide memory ordering in cases where the memory block was
 * modified by the core.
 *
 * @param ptr    Block to free
 * @param pool   Pool to put it in
 * @param num_cache_lines
 *               Cache lines to invalidate
 */
static inline void cvmx_fpa_free_nosync(void *ptr, uint64_t pool,
					uint64_t num_cache_lines)
{
	/* FPA3 is handled differently */
	if ((octeon_has_feature(OCTEON_FEATURE_FPA3)))
		cvmx_fpa3_free_nosync(ptr, cvmx_fpa1_pool_to_fpa3_aura(pool),
			num_cache_lines);
	else
		cvmx_fpa1_free_nosync(ptr, pool, num_cache_lines);
}

/**
 * Free a block allocated with a FPA pool.  Provides required memory
 * ordering in cases where memory block was modified by core.
 *
 * @param ptr    Block to free
 * @param pool   Pool to put it in
 * @param num_cache_lines
 *               Cache lines to invalidate
 */
static inline void cvmx_fpa_free(void *ptr, uint64_t pool,
				 uint64_t num_cache_lines)
{
	if ((octeon_has_feature(OCTEON_FEATURE_FPA3)))
		cvmx_fpa3_free(ptr, cvmx_fpa1_pool_to_fpa3_aura(pool),
			num_cache_lines);
	else
		cvmx_fpa1_free(ptr, pool, num_cache_lines);
}


/**
 * Setup a FPA pool to control a new block of memory.
 * This can only be called once per pool. Make sure proper
 * locking enforces this.
 *
 * @param pool       Pool to initialize
 * @param name       Constant character string to name this pool.
 *                   String is not copied.
 * @param buffer     Pointer to the block of memory to use. This must be
 *                   accessable by all processors and external hardware.
 * @param block_size Size for each block controlled by the FPA
 * @param num_blocks Number of blocks
 *
 * @return the pool number on Success,
 *         -1 on failure
 */
extern int cvmx_fpa_setup_pool(int pool, const char *name, void *buffer,
			       uint64_t block_size, uint64_t num_blocks);

extern int cvmx_fpa_shutdown_pool(int pool);

/**
 * Gets the block size of buffer in specified pool
 * @param pool	 Pool to get the block size from
 * @return       Size of buffer in specified pool
 */
extern unsigned cvmx_fpa_get_block_size(int pool);

extern int cvmx_fpa_is_pool_available(int pool_num);
extern uint64_t cvmx_fpa_get_pool_owner(int pool_num);
extern int cvmx_fpa_get_max_pools(void);
extern int cvmx_fpa_get_current_count(int pool_num);
extern int cvmx_fpa_validate_pool(int pool);


#ifdef	__cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif

#endif /*  __CVM_FPA_H__ */
