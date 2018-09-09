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
 * Interface to RAID block. This is not available on all chips.
 *
 * <hr>$Revision: 85932 $<hr>
 */
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-cmd-queue.h>
#include <asm/octeon/cvmx-raid.h>
#else
#include "cvmx.h"
#include "cvmx-cmd-queue.h"
#include "cvmx-raid.h"
#include "cvmx-helper-fpa.h"
#endif

CVMX_SHARED cvmx_raid_config_t raid_config = {.command_queue_pool = {2,1024,0}};

/**
 * Initialize the RAID block
 *
 * @param polynomial Coefficients for the RAID polynomial
 *
 * @return Zero on success, negative on failure
 */
int cvmx_raid_initialize(cvmx_rad_reg_polynomial_t polynomial)
{
	cvmx_cmd_queue_result_t result;
	cvmx_rad_reg_cmd_buf_t rad_reg_cmd_buf;
	int outputbuffer_pool = (int)cvmx_fpa_get_raid_pool();
	uint64_t outputbuffer_pool_size = cvmx_fpa_get_raid_pool_block_size();

	cvmx_write_csr(CVMX_RAD_REG_POLYNOMIAL, polynomial.u64);

	/*Initialize FPA pool for raid command queue buffers*/
#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	cvmx_fpa_global_initialize();
	if(raid_config.command_queue_pool.buffer_count != 0)
		__cvmx_helper_initialize_fpa_pool(outputbuffer_pool, outputbuffer_pool_size,
			raid_config.command_queue_pool.buffer_count, "Raid Cmd Bufs");
#endif
	result = cvmx_cmd_queue_initialize(CVMX_CMD_QUEUE_RAID, 0, outputbuffer_pool, outputbuffer_pool_size);
	if (result != CVMX_CMD_QUEUE_SUCCESS)
		return -1;

	rad_reg_cmd_buf.u64 = 0;
	rad_reg_cmd_buf.cn52xx.dwb = outputbuffer_pool_size / 128;
	rad_reg_cmd_buf.cn52xx.pool = outputbuffer_pool;
	rad_reg_cmd_buf.s.size = outputbuffer_pool_size / 8;
	rad_reg_cmd_buf.s.ptr = cvmx_ptr_to_phys(cvmx_cmd_queue_buffer(CVMX_CMD_QUEUE_RAID)) >> 7;
	cvmx_write_csr(CVMX_RAD_REG_CMD_BUF, rad_reg_cmd_buf.u64);
	return 0;
}

EXPORT_SYMBOL(cvmx_raid_initialize);

/**
 * Shutdown the RAID block. RAID must be idle when
 * this function is called.
 *
 * @return Zero on success, negative on failure
 */
int cvmx_raid_shutdown(void)
{
	cvmx_rad_reg_ctl_t rad_reg_ctl;

	if (cvmx_cmd_queue_length(CVMX_CMD_QUEUE_RAID)) {
		cvmx_dprintf("ERROR: cvmx_raid_shutdown: RAID not idle.\n");
		return -1;
	}

	rad_reg_ctl.u64 = cvmx_read_csr(CVMX_RAD_REG_CTL);
	rad_reg_ctl.s.reset = 1;
	cvmx_write_csr(CVMX_RAD_REG_CTL, rad_reg_ctl.u64);
	cvmx_wait(100);

	cvmx_cmd_queue_shutdown(CVMX_CMD_QUEUE_RAID);
	cvmx_write_csr(CVMX_RAD_REG_CMD_BUF, 0);
	return 0;
}

EXPORT_SYMBOL(cvmx_raid_shutdown);

/**
 * Submit a command to the RAID block
 *
 * @param num_words Number of command words to submit
 * @param words     Command words
 *
 * @return Zero on success, negative on failure
 */
int cvmx_raid_submit(int num_words, cvmx_raid_word_t words[])
{
	cvmx_cmd_queue_result_t result = cvmx_cmd_queue_write(CVMX_CMD_QUEUE_RAID, 1, num_words, (uint64_t *) words);
	if (result == CVMX_CMD_QUEUE_SUCCESS)
		cvmx_write_csr(CVMX_ADDR_DID(CVMX_FULL_DID(14, 0)), num_words);
	return result;
}

void cvmx_raid_set_cmd_que_pool_config(int64_t pool, uint64_t buffer_size,
			      uint64_t buffer_count)
{
	raid_config.command_queue_pool.pool_num = pool;
	raid_config.command_queue_pool.buffer_size = buffer_size;
	raid_config.command_queue_pool.buffer_count = buffer_count;
}

void cvmx_raid_get_cmd_que_pool_config(cvmx_fpa_pool_config_t* cmd_que_pool)
{
	*cmd_que_pool = raid_config.command_queue_pool;
}

EXPORT_SYMBOL(cvmx_raid_submit);
