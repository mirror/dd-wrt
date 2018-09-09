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
 * Source file for the zip (deflate) block
 *
 * <hr>$Revision: 89404 $<hr>
 */

#include "cvmx.h"
#include "cvmx-cmd-queue.h"
#include "cvmx-zip.h"
#include "cvmx-helper-fpa.h"

CVMX_SHARED cvmx_zip_config_t zip_config = {.zip_pool = {5,2048,0}, .aura = 5};

/**
 * Initialize the ZIP block
 *
 * @return Zero on success, negative on failure
 */
int cvmx_zip_initialize(void)
{
	cvmx_zip_cmd_buf_t zip_cmd_buf;
	cvmx_cmd_queue_result_t result;
	int zip_pool = (int)cvmx_fpa_get_zip_pool();
	uint64_t zip_pool_size = cvmx_fpa_get_zip_pool_block_size();

    	/*Initialize FPA pool for Zip pool buffers*/
#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	cvmx_fpa_global_initialize();
	if(zip_config.zip_pool.buffer_count != 0)
		__cvmx_helper_initialize_fpa_pool(zip_pool, zip_pool_size,
			zip_config.zip_pool.buffer_count, "Zip Buffers");
#endif

	result = cvmx_cmd_queue_initialize(CVMX_CMD_QUEUE_ZIP, 0, zip_pool, zip_pool_size);
	if (result != CVMX_CMD_QUEUE_SUCCESS)
		return -1;

	zip_cmd_buf.u64 = 0;
	zip_cmd_buf.s.dwb = zip_pool_size / 128;
	zip_cmd_buf.s.pool = zip_pool;
	zip_cmd_buf.s.size = zip_pool_size / 8;
	zip_cmd_buf.s.ptr = cvmx_ptr_to_phys(cvmx_cmd_queue_buffer(CVMX_CMD_QUEUE_ZIP)) >> 7;
	cvmx_write_csr(CVMX_ZIP_CMD_BUF, zip_cmd_buf.u64);
	cvmx_write_csr(CVMX_ZIP_ERROR, 1);
	cvmx_read_csr(CVMX_ZIP_CMD_BUF);	/* Read to make sure setup is complete */
	return 0;
}

/**
 * @INTERNAL
 * Initialize the ZIP queue buffer and zip block.
 *
 * @param queue : ZIP instruction queue
 * @param zcoremask : ZIP coremask to use for this queue
 *
 * @return Zero on success, negative on failure
 */
int cvmx_zip3_init(int queue, int zcoremask)
{
	cvmx_cmd_queue_result_t result;
	cvmx_zip_quex_sbuf_t	sbuf;
	int 			pool;
	uint64_t 		pool_size;
	int			aura;
	void			*ptr;
	uint64_t 		data;

	/* Validate arguments */
	if (queue < 0 || queue >= 8 ||
	    (zcoremask != 1 && zcoremask != 2 && zcoremask != 4)) {
		cvmx_dprintf("ERROR: cvmx_zip3_inti: Invalid arguments.\n");
		return -1;
	}

	/* Get the pool and aura to use */
	pool = (int)cvmx_fpa_get_zip_pool();
	pool_size = cvmx_fpa_get_zip_pool_block_size();
	aura = cvmx_fpa_get_zip_aura();

	/* Initialize the command queue */
	result = cvmx_cmd_queue_initialize(CVMX_CMD_QUEUE_ZIP_QUE(queue), 0,
					   pool, pool_size);
	if (result != CVMX_CMD_QUEUE_SUCCESS)
		return -1;

	/* Set the pool and aura to use */
	sbuf.u64 = 0;
	ptr = cvmx_cmd_queue_buffer(CVMX_CMD_QUEUE_ZIP_QUE(queue));
	sbuf.s.ptr = cvmx_ptr_to_phys(ptr) >> 7;
	sbuf.s.size = pool_size / 8;
	cvmx_write_csr(CVMX_ZIP_QUEX_SBUF(queue), sbuf.u64);
	cvmx_write_csr(CVMX_ZIP_QUEX_AURA(queue), (uint64_t)aura);

	/* Map the queue to a zip engine */
	cvmx_write_csr(CVMX_ZIP_QUEX_MAP(queue), zcoremask);

	/* Enable the queue */
	cvmx_write_csr(CVMX_ZIP_QUE_ENA, 1 << queue);

	/* Set the queue to high priority */
	data = cvmx_read_csr(CVMX_ZIP_QUE_PRI);
	data |= 1 << queue;
	cvmx_write_csr(CVMX_ZIP_QUE_PRI, data);

	return 0;
}

/**
 * Initialize the ZIP QUEUE buffer
 *
 * @param queue : ZIP instruction queue
 * @param zcoremask : ZIP coremask to use for this queue
 *
 * @return Zero on success, negative on failure
 */
int cvmx_zip_queue_initialize(int queue, int zcoremask)
{
	cvmx_zip_quex_buf_t zip_que_buf;
	cvmx_cmd_queue_result_t result;
	cvmx_zip_quex_map_t que_map;
	cvmx_zip_que_ena_t que_ena;
	cvmx_zip_int_reg_t int_reg;
	int zip_pool = (int)cvmx_fpa_get_zip_pool();
	uint64_t zip_pool_size = cvmx_fpa_get_zip_pool_block_size();

	/* Previous Octeon models has only one instruction queue, call
	   cvmx_zip_inititalize() to initialize the ZIP block */

	if (!OCTEON_IS_MODEL(OCTEON_CN68XX))
		return -1;

	result = cvmx_cmd_queue_initialize(CVMX_CMD_QUEUE_ZIP_QUE(queue), 0, zip_pool, zip_pool_size);
	if (result != CVMX_CMD_QUEUE_SUCCESS)
		return -1;

	/* 1. Program ZIP_QUE0/1_BUF to have the correct buffer pointer and
	   size configured for each instruction queue */
	zip_que_buf.u64 = 0;
	zip_que_buf.s.dwb = zip_pool_size / 128;
	zip_que_buf.s.pool = zip_pool;
	zip_que_buf.s.size = zip_pool_size / 8;
	zip_que_buf.s.ptr = cvmx_ptr_to_phys(cvmx_cmd_queue_buffer(CVMX_CMD_QUEUE_ZIP_QUE(queue))) >> 7;
	cvmx_write_csr(CVMX_ZIP_QUEX_BUF(queue), zip_que_buf.u64);

	/* 2. Change the queue-to-ZIP core mapping by programming ZIP_QUE0/1_MAP. */
	que_map.u64 = cvmx_read_csr(CVMX_ZIP_QUEX_MAP(queue));
	que_map.s.zce = zcoremask;
	cvmx_write_csr(CVMX_ZIP_QUEX_MAP(queue), que_map.u64);

	/* Enable the queue */
	que_ena.u64 = cvmx_read_csr(CVMX_ZIP_QUE_ENA);
	que_ena.s.ena |= (1 << queue);
	cvmx_write_csr(CVMX_ZIP_QUE_ENA, que_ena.u64);

	/* Use round robin to have equal priority for each instruction queue */
	cvmx_write_csr(CVMX_ZIP_QUE_PRI, 0x3);

	int_reg.u64 = cvmx_read_csr(CVMX_ZIP_INT_REG);
	if (queue)
		int_reg.s.doorbell1 = 1;
	else
		int_reg.s.doorbell0 = 1;

	cvmx_write_csr(CVMX_ZIP_INT_REG, int_reg.u64);
	/* Read back to make sure the setup is complete */
	cvmx_read_csr(CVMX_ZIP_QUEX_BUF(queue));
	return 0;
}

/**
 * Shutdown the ZIP block. ZIP must be idle when
 * this function is called.
 *
 * @return Zero on success, negative on failure
 */
int cvmx_zip_shutdown(void)
{
	cvmx_zip_cmd_ctl_t zip_cmd_ctl;

	if (cvmx_cmd_queue_length(CVMX_CMD_QUEUE_ZIP)) {
		cvmx_dprintf("ERROR: cvmx_zip_shutdown: ZIP not idle.\n");
		return -1;
	}

	zip_cmd_ctl.u64 = cvmx_read_csr(CVMX_ZIP_CMD_CTL);
	zip_cmd_ctl.s.reset = 1;
	cvmx_write_csr(CVMX_ZIP_CMD_CTL, zip_cmd_ctl.u64);
	cvmx_wait(100);

	cvmx_cmd_queue_shutdown(CVMX_CMD_QUEUE_ZIP);
	return 0;
}

/**
 * Shutdown the ZIP block for a queue. ZIP must be idle when
 * this function is called.
 *
 * @param queue   Zip instruction queue of the command
 *
 * @return Zero on success, negative on failure
 */
int cvmx_zip_queue_shutdown(int queue)
{
	cvmx_zip_cmd_ctl_t zip_cmd_ctl;

	if (cvmx_cmd_queue_length(CVMX_CMD_QUEUE_ZIP_QUE(queue))) {
		cvmx_dprintf("ERROR: cvmx_zip_shutdown: ZIP not idle.\n");
		return -1;
	}

	zip_cmd_ctl.u64 = cvmx_read_csr(CVMX_ZIP_CMD_CTL);
	zip_cmd_ctl.s.reset = 1;
	cvmx_write_csr(CVMX_ZIP_CMD_CTL, zip_cmd_ctl.u64);
	cvmx_wait(100);

	cvmx_cmd_queue_shutdown(CVMX_CMD_QUEUE_ZIP_QUE(queue));
	return 0;
}

/**
 * Submit a command to the ZIP block
 *
 * @param command Zip command to submit
 *
 * @return Zero on success, negative on failure
 */
int cvmx_zip_submit(cvmx_zip_command_t * command)
{
	cvmx_cmd_queue_result_t result = cvmx_cmd_queue_write(CVMX_CMD_QUEUE_ZIP, 1, 8, command->u64);
	if (result == CVMX_CMD_QUEUE_SUCCESS)
		cvmx_write_csr(CVMX_ADDR_DID(CVMX_FULL_DID(7, 0)), 8);
	return result;
}

/**
 * Submit a command to the ZIP block
 *
 * @param command Zip command to submit
 * @param queue   Zip instruction queue of the command
 *
 * @return Zero on success, negative on failure
 */
int cvmx_zip_queue_submit(cvmx_zip_command_t * command, int queue)
{
	cvmx_cmd_queue_result_t result = cvmx_cmd_queue_write(CVMX_CMD_QUEUE_ZIP_QUE(queue), 1, 8, command->u64);
	if (result == CVMX_CMD_QUEUE_SUCCESS)
		cvmx_write_csr((CVMX_ADDR_DID(CVMX_FULL_DID(7, 0)) | queue << 3), 8);
	return result;
}

void cvmx_zip_set_fpa_pool_config(int64_t pool, uint64_t buffer_size,
			      uint64_t buffer_count)
{
	zip_config.zip_pool.pool_num = pool;
	zip_config.zip_pool.buffer_size = buffer_size;
	zip_config.zip_pool.buffer_count = buffer_count;
}

void cvmx_zip_get_fpa_pool_config(cvmx_fpa_pool_config_t *zip_pool)
{
	*zip_pool = zip_config.zip_pool;
}
