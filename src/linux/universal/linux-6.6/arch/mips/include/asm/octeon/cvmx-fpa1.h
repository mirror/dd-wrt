/***********************license start***************
 * Copyright (c) 2014  Cavium Inc. (support@cavium.com). All rights
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
 * Interface to the hardware Free Pool Allocator on Octeon chips.
 * These are the legacy models, i.e. prior to CN78XX/CN76XX.
 *
 * <hr>$Revision: 120123 $<hr>
 *
 */


#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <asm/octeon/cvmx-scratch.h>
#include <asm/octeon/cvmx-fpa-defs.h>
#else
#include "cvmx-scratch.h"
#include "cvmx-fpa-defs.h"
#endif

#ifndef __CVMX_FPA1_HW_H__
#define __CVMX_FPA1_HW_H__

#ifdef	__cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

/* Legacy pool range is 0..7 and 8 on CN68XX */
typedef	int cvmx_fpa1_pool_t;
#define CVMX_FPA1_NUM_POOLS      8
#define CVMX_FPA1_INVALID_POOL ((cvmx_fpa1_pool_t)-1)
#define	CVMX_FPA1_NAME_SIZE	16

/**
 * Structure describing the data format used for stores to the FPA.
 */
typedef union {
	uint64_t u64;
	struct {
		CVMX_BITFIELD_FIELD(uint64_t scraddr:8,	/**
					 * the (64-bit word) location in
					 * scratchpad to write to (if len != 0)
					 */
		CVMX_BITFIELD_FIELD(uint64_t len:8,		/**
					 * the number of words in the response
					 * (0 => no response)
					 */
		CVMX_BITFIELD_FIELD(uint64_t did:8,		/**
					 * the ID of the device on the
					 * non-coherent bus
					 */
		CVMX_BITFIELD_FIELD(uint64_t addr:40,	/**
					 * the address that will appear in the
					 * first tick on the NCB bus
					 */
		))));
	} s;
} cvmx_fpa1_iobdma_data_t;

/*
 * Allocate or reserve the specified fpa pool.
 *
 * @param pool	  FPA pool to allocate/reserve. If -1 it
 *                finds an empty pool to allocate.
 * @return        Alloctaed pool number or CVMX_FPA1_POOL_INVALID
 *                if fails to allocate the pool
 */
cvmx_fpa1_pool_t cvmx_fpa1_reserve_pool(cvmx_fpa1_pool_t pool);

/**
 * Free the specified fpa pool.
 * @param pool	   Pool to free
 * @return         0 for success -1 failure
 */
int cvmx_fpa1_release_pool(cvmx_fpa1_pool_t pool);

static inline void cvmx_fpa1_free(void *ptr, cvmx_fpa1_pool_t pool,
	uint64_t num_cache_lines)
{
	cvmx_addr_t newptr;

	newptr.u64 = cvmx_ptr_to_phys(ptr);
	newptr.sfilldidspace.didspace =
		CVMX_ADDR_DIDSPACE(CVMX_FULL_DID(CVMX_OCT_DID_FPA, pool));
	/* Make sure that any previous writes to memory go out before we free
	 * this buffer.  This also serves as a barrier to prevent GCC from
	 * reordering operations to after the free.
	 */
	CVMX_SYNCWS;
	/* value written is number of cache lines not written back */
	cvmx_write_io(newptr.u64, num_cache_lines);
}

static inline void cvmx_fpa1_free_nosync(void *ptr, cvmx_fpa1_pool_t pool,
					unsigned num_cache_lines)
{
	cvmx_addr_t newptr;
	newptr.u64 = cvmx_ptr_to_phys(ptr);
	newptr.sfilldidspace.didspace =
		CVMX_ADDR_DIDSPACE(CVMX_FULL_DID(CVMX_OCT_DID_FPA, pool));
	/* Prevent GCC from reordering around free */
	asm volatile ("" : : : "memory");
	/* value written is number of cache lines not written back */
	cvmx_write_io(newptr.u64, num_cache_lines);
}

/**
 * Enable the FPA for use. Must be performed after any CSR
 * configuration but before any other FPA functions.
 */
static inline void cvmx_fpa1_enable(void)
{
	cvmx_fpa_ctl_status_t status;

	extern int cvmx_fpa1_init_pool;
	/* clean up leftover buffers from u-boot */
	/* cvmx_fpa1_enable() is called several times, but this loop executes */
	/* fully only once; then cvmx_fpa1_init_pool > CVMX_FPA1_NUM_POOLS */
	for (; cvmx_fpa1_init_pool < CVMX_FPA1_NUM_POOLS; cvmx_fpa1_init_pool++) {
		uint64_t available = cvmx_read_csr(CVMX_FPA_QUEX_AVAILABLE(cvmx_fpa1_init_pool));

		if (available) {
			printk(KERN_DEBUG
				"FPA: purging %llu blocks from pool %d\n",
				available, cvmx_fpa1_init_pool);
			for (; available; available--) {
				uint64_t address;

				cvmx_wait(50);
				address = cvmx_read_csr(CVMX_ADDR_DID(
					CVMX_FULL_DID(CVMX_OCT_DID_FPA, cvmx_fpa1_init_pool)));
				if (!address)
					break;
			}
		}
	}

	status.u64 = cvmx_read_csr(CVMX_FPA_CTL_STATUS);
	if (status.s.enb) {
		/*
		 * CN68XXP1 should not reset the FPA (doing so may break
		 * the SSO, so we may end up enabling it more than once.
		 * Just return and don't spew messages.
		 */
		return;
	}

	status.u64 = 0;
	status.s.enb = 1;
	cvmx_write_csr(CVMX_FPA_CTL_STATUS, status.u64);
}

/**
 * Reset FPA to disable. Make sure buffers from all FPA pools are freed
 * before disabling FPA.
 */
static inline void cvmx_fpa1_disable(void)
{
	cvmx_fpa_ctl_status_t status;

	if (OCTEON_IS_MODEL(OCTEON_CN68XX_PASS1))
		return;

	status.u64 = cvmx_read_csr(CVMX_FPA_CTL_STATUS);
	status.s.reset = 1;
	cvmx_write_csr(CVMX_FPA_CTL_STATUS, status.u64);
}


static inline void *cvmx_fpa1_alloc(cvmx_fpa1_pool_t pool)
{
	uint64_t address;

	for (;;) {
		address = cvmx_read_csr(CVMX_ADDR_DID(CVMX_FULL_DID(CVMX_OCT_DID_FPA,
					pool)));
		if (cvmx_likely(address)) {
			return cvmx_phys_to_ptr(address);
		} else {
			if (cvmx_read_csr(CVMX_FPA_QUEX_AVAILABLE(pool)) > 0)
				cvmx_wait(50);
			else
				return NULL;
		}
	}
}

/**
 * Asynchronously get a new block from the FPA
 * @INTERNAL
 *
 * The result of cvmx_fpa_async_alloc() may be retrieved using
 * cvmx_fpa_async_alloc_finish().
 *
 * @param scr_addr Local scratch address to put response in.  This is a byte
 *		   address but must be 8 byte aligned.
 * @param pool      Pool to get the block from
 */
static inline void
cvmx_fpa1_async_alloc(uint64_t scr_addr, cvmx_fpa1_pool_t pool)
{
	cvmx_fpa1_iobdma_data_t data;

	/* Hardware only uses 64 bit aligned locations, so convert from byte
	 * address to 64-bit index
	 */
	data.u64 = 0ull;
	data.s.scraddr = scr_addr >> 3;
	data.s.len = 1;
	data.s.did = CVMX_FULL_DID(CVMX_OCT_DID_FPA, pool);
	data.s.addr = 0;

	cvmx_scratch_write64(scr_addr, 0ull);
	CVMX_SYNCW;
	cvmx_send_single(data.u64);
}

/**
 * Retrieve the result of cvmx_fpa_async_alloc
 * @INTERNAL
 *
 * @param scr_addr The Local scratch address.  Must be the same value
 * passed to cvmx_fpa_async_alloc().
 *
 * @param pool Pool the block came from.  Must be the same value
 * passed to cvmx_fpa_async_alloc.
 *
 * @return Pointer to the block or NULL on failure
 */
static inline void *
cvmx_fpa1_async_alloc_finish(uint64_t scr_addr, cvmx_fpa1_pool_t pool)
{
	uint64_t address;

	CVMX_SYNCIOBDMA;

	address = cvmx_scratch_read64(scr_addr);
	if (cvmx_likely(address))
		return cvmx_phys_to_ptr(address);
	else
		return cvmx_fpa1_alloc(pool);
}


static inline uint64_t cvmx_fpa1_get_available(cvmx_fpa1_pool_t pool)
{
	return cvmx_read_csr(CVMX_FPA_QUEX_AVAILABLE(pool));
}

#ifdef	__cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif

#endif /* __CVMX_FPA1_HW_H__ */
