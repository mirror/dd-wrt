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
 * Interface to the hardware Input Packet Data unit.
 *
 * <hr>$Revision: 116854 $<hr>
 */

#ifndef __CVMX_IPD_H__
#define __CVMX_IPD_H__

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-fpa.h>
#include <asm/octeon/cvmx-ipd-defs.h>
#include <asm/octeon/cvmx-pki.h>
#else
#include "cvmx-pki.h"
#endif

#ifdef	__cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

/* CSR typedefs have been moved to cvmx-ipd-defs.h */

typedef cvmx_ipd_1st_mbuff_skip_t cvmx_ipd_mbuff_not_first_skip_t;
typedef cvmx_ipd_1st_next_ptr_back_t cvmx_ipd_second_next_ptr_back_t;

typedef struct cvmx_ipd_tag_fields
{
	uint64_t ipv6_src_ip:1;
	uint64_t ipv6_dst_ip:1;
	uint64_t ipv6_src_port:1;
	uint64_t ipv6_dst_port:1;
	uint64_t ipv6_next_header:1;
	uint64_t ipv4_src_ip:1;
	uint64_t ipv4_dst_ip:1;
	uint64_t ipv4_src_port:1;
	uint64_t ipv4_dst_port:1;
	uint64_t ipv4_protocol:1;
	uint64_t input_port:1;
} cvmx_ipd_tag_fields_t;

typedef struct cvmx_pip_port_config
{
	uint64_t parse_mode;
	uint64_t tag_type;
	uint64_t tag_mode;
	cvmx_ipd_tag_fields_t tag_fields;
}cvmx_pip_port_config_t;

typedef struct cvmx_ipd_config_struct
{
	uint64_t first_mbuf_skip;
	uint64_t not_first_mbuf_skip;
	uint64_t ipd_enable;
	uint64_t enable_len_M8_fix;
	uint64_t cache_mode;
	cvmx_fpa_pool_config_t packet_pool;
	cvmx_fpa_pool_config_t wqe_pool;
	cvmx_pip_port_config_t port_config;
}cvmx_ipd_config_t;

extern CVMX_SHARED cvmx_ipd_config_t cvmx_ipd_cfg;
/**
 * Gets the fpa pool number of packet pool
 */
static inline int64_t cvmx_fpa_get_packet_pool(void)
{
	return (cvmx_ipd_cfg.packet_pool.pool_num);
}

/**
 * Gets the buffer size of packet pool buffer
 */
static inline uint64_t cvmx_fpa_get_packet_pool_block_size(void)
{
	return (cvmx_ipd_cfg.packet_pool.buffer_size);
}

/**
 * Gets the buffer count of packet pool
 */
static inline uint64_t cvmx_fpa_get_packet_pool_buffer_count(void)
{
	return (cvmx_ipd_cfg.packet_pool.buffer_count);
}

/**
 * Gets the fpa pool number of wqe pool
 */
static inline int64_t cvmx_fpa_get_wqe_pool(void)
{
	return (cvmx_ipd_cfg.wqe_pool.pool_num);
}

/**
 * Gets the buffer size of wqe pool buffer
 */
static inline uint64_t cvmx_fpa_get_wqe_pool_block_size(void)
{
	return (cvmx_ipd_cfg.wqe_pool.buffer_size);
}

/**
 * Gets the buffer count of wqe pool
 */
static inline uint64_t cvmx_fpa_get_wqe_pool_buffer_count(void)
{
	return (cvmx_ipd_cfg.wqe_pool.buffer_count);
}

/**
 * Sets the ipd related configuration in internal structure which is then used
 * for seting IPD hardware block
 */
int cvmx_ipd_set_config(cvmx_ipd_config_t ipd_config);

/**
 * Gets the ipd related configuration from internal structure.
 */
void cvmx_ipd_get_config(cvmx_ipd_config_t* ipd_config);

/**
 * Sets the internal FPA pool data structure for packet buffer pool.
 * @param pool	fpa pool number yo use
 * @param buffer_size	buffer size of pool
 * @param buffer_count	number of buufers to allocate to pool
 */
void cvmx_ipd_set_packet_pool_config(int64_t pool, uint64_t buffer_size,
				       uint64_t buffer_count);

/**
 * Sets the internal FPA pool data structure for wqe pool.
 * @param pool	fpa pool number yo use
 * @param buffer_size	buffer size of pool
 * @param buffer_count	number of buufers to allocate to pool
 */
void cvmx_ipd_set_wqe_pool_config(int64_t pool, uint64_t buffer_size,
				       uint64_t buffer_count);

/**
 * Gets the FPA packet buffer pool parameters.
 */
static inline void cvmx_fpa_get_packet_pool_config(int64_t *pool,
						uint64_t *buffer_size, uint64_t *buffer_count)
{
	if (pool != NULL)
		*pool = cvmx_ipd_cfg.packet_pool.pool_num;
	if (buffer_size != NULL)
		*buffer_size = cvmx_ipd_cfg.packet_pool.buffer_size;
	if (buffer_count != NULL)
		*buffer_count = cvmx_ipd_cfg.packet_pool.buffer_count;
}

/**
 * Sets the FPA packet buffer pool parameters.
 */
static inline void cvmx_fpa_set_packet_pool_config(int64_t pool,
						uint64_t buffer_size, uint64_t buffer_count)
{
	cvmx_ipd_set_packet_pool_config(pool, buffer_size, buffer_count);
}

/**
 * Gets the FPA WQE pool parameters.
 */
static inline void cvmx_fpa_get_wqe_pool_config(int64_t *pool,
						uint64_t *buffer_size, uint64_t *buffer_count)
{
	if (pool != NULL)
		*pool = cvmx_ipd_cfg.wqe_pool.pool_num;
	if (buffer_size != NULL)
		*buffer_size = cvmx_ipd_cfg.wqe_pool.buffer_size;
	if (buffer_count != NULL)
		*buffer_count = cvmx_ipd_cfg.wqe_pool.buffer_count;
}

/**
 * Sets the FPA WQE pool parameters.
 */
static inline void cvmx_fpa_set_wqe_pool_config(int64_t pool,
						uint64_t buffer_size, uint64_t buffer_count)
{
	cvmx_ipd_set_wqe_pool_config(pool, buffer_size, buffer_count);
}

/**
 * Configure IPD
 *
 * @param mbuff_size Packets buffer size in 8 byte words
 * @param first_mbuff_skip
 *                   Number of 8 byte words to skip in the first buffer
 * @param not_first_mbuff_skip
 *                   Number of 8 byte words to skip in each following buffer
 * @param first_back Must be same as first_mbuff_skip / 128
 * @param second_back
 *                   Must be same as not_first_mbuff_skip / 128
 * @param wqe_fpa_pool
 *                   FPA pool to get work entries from
 * @param cache_mode
 * @param back_pres_enable_flag
 *                   Enable or disable port back pressure at a global level.
 *                   This should always be 1 as more accurate control can be
 *                   found in IPD_PORTX_BP_PAGE_CNT[BP_ENB].
 */
void cvmx_ipd_config(uint64_t mbuff_size, uint64_t first_mbuff_skip,
		     uint64_t not_first_mbuff_skip, uint64_t first_back,
		     uint64_t second_back, uint64_t wqe_fpa_pool,
		     cvmx_ipd_mode_t cache_mode, uint64_t back_pres_enable_flag);
/**
 * Enable IPD
 */
void cvmx_ipd_enable(void);

/**
 * Disable IPD
 */
void cvmx_ipd_disable(void);

void __cvmx_ipd_free_ptr(void);

void cvmx_ipd_set_packet_pool_buffer_count(uint64_t buffer_count);
void cvmx_ipd_set_wqe_pool_buffer_count(uint64_t buffer_count);

/**
 * Setup Random Early Drop on a specific input queue
 *
 * @param queue  Input queue to setup RED on (0-7)
 * @param pass_thresh
 *               Packets will begin slowly dropping when there are less than
 *               this many packet buffers free in FPA 0.
 * @param drop_thresh
 *               All incomming packets will be dropped when there are less
 *               than this many free packet buffers in FPA 0.
 * @return Zero on success. Negative on failure
 */
extern int cvmx_ipd_setup_red_queue(int queue, int pass_thresh, int drop_thresh);

/**
 * Setup Random Early Drop to automatically begin dropping packets.
 *
 * @param pass_thresh
 *               Packets will begin slowly dropping when there are less than
 *               this many packet buffers free in FPA 0.
 * @param drop_thresh
 *               All incomming packets will be dropped when there are less
 *               than this many free packet buffers in FPA 0.
 * @return Zero on success. Negative on failure
 */
extern int cvmx_ipd_setup_red(int pass_thresh, int drop_thresh);

#ifdef	__cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif

#endif /*  __CVMX_IPD_H__ */
