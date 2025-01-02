/***********************license start***************
 * Copyright (c) 2013-2016  Cavium Inc. (support@cavium.com). All rights
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
 *
 *   * Neither the name of Cavium Inc. nor the names of
 *     its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written
 *     permission.
 *
 * This Software, including technical data, may be subject to U.S. export control
 * laws, including the U.S. Export Administration Act and its  associated
 * regulations, and may be subject to export or import  regulations in other
 * countries.
 *
 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND CAVIUM INC. MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY REPRESENTATION
 * OR DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT DEFECTS, AND CAVIUM
 * SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES OF TITLE,
 * MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF
 * VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. THE ENTIRE  RISK ARISING OUT OF USE OR
 * PERFORMANCE OF THE SOFTWARE LIES WITH YOU.
 ***********************license end**************************************/

/**
 * @file
 *
 * Interface to the CN70XX, CN78XX hardware BCH engine.
 *
 * <hr>$Revision: 79788 $<hr>
 */


#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
# include <asm/octeon/cvmx.h>
# include <asm/octeon/cvmx-config.h>
# include <asm/octeon/cvmx-bch-defs.h>
# include <asm/octeon/cvmx-bch.h>
# include <asm/octeon/cvmx-fpa.h>
# include <asm/octeon/cvmx-helper-fpa.h>
# include <asm/octeon/cvmx-cmd-queue.h>
#include <linux/kernel.h>
#include <linux/slab.h>

#elif defined(CVMX_BUILD_FOR_UBOOT)

# include <common.h>
# include <asm/arch/cvmx.h>
# include <asm/arch/cvmx-bch-defs.h>
# include <asm/arch/cvmx-bch.h>
# include <asm/arch/cvmx-fpa.h>
# include <asm/arch/cvmx-helper-fpa.h>
# include <asm/arch/cvmx-cmd-queue.h>
#else
# include "cvmx.h"
# include "cvmx-bch-defs.h"
# include "cvmx-bch.h"
# include "cvmx-fpa.h"
# include "cvmx-helper-fpa.h"
# include "cvmx-cmd-queue.h"
#endif

#ifndef CVMX_BUILD_FOR_UBOOT
# ifdef DEBUG
#  define debug(fmt, args...)	cvmx_dprintf(fmt, ##args)
# else
#  define debug(fmt, args...)
# endif
#endif

CVMX_SHARED cvmx_bch_app_config_t bch_config = {
	.command_queue_pool = {6, 1024, 0},
	.aura = 6
};

const unsigned bch_buf_count = 16;
static	int bch_pool = -1;
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
extern int cvm_oct_alloc_fpa_pool(int pool, int size);
extern int cvm_oct_mem_empty_fpa(int pool, int elements);
extern int cvm_oct_mem_fill_fpa(int pool, int elements);
#endif

/**
 * Initialize the BCH block
 *
 * @return Zero on success, negative on failure
 */
int cvmx_bch_initialize(void)
{
	cvmx_bch_cmd_buf_t bch_cmd_buf;
	cvmx_bch_ctl_t bch_ctl;
	cvmx_cmd_queue_result_t result;

	uint64_t bch_pool_size;
	int buf_cnt;

	/* Initialize FPA pool for BCH pool buffers */
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
	/* Dynamically allocate pool */
	bch_pool = -1;
	bch_pool_size = CVMX_FPA_OUTPUT_BUFFER_POOL_SIZE;
	buf_cnt = bch_buf_count;

	debug("pool: %d, pool size: %llu, bufcount %d\n",
		bch_pool, bch_pool_size, buf_cnt);
	/* Setup the FPA */
	if (octeon_has_feature(OCTEON_FEATURE_FPA3)) {
		cvmx_fpa3_pool_t cmd_pool;
		cvmx_fpa3_gaura_t cmd_aura;
		struct kmem_cache *cmd_pool_cache;
		void *cmd_pool_stack;
		octeon_fpa3_init(0);
		octeon_fpa3_pool_init(0, bch_pool, &cmd_pool, &cmd_pool_stack, 4096);
		octeon_fpa3_aura_init(cmd_pool, bch_pool, &cmd_aura, buf_cnt, 20480);
		bch_pool = cmd_aura.laura;
		cmd_pool_cache = kmem_cache_create("bch_cmd", bch_pool_size, 128, 0, NULL);
		if (!cmd_pool_cache) {
			printk("cvm_oct_alloc_fpa_pool(%d, %lld)\n",
						bch_pool, bch_pool_size);
			return -ENOMEM;
		}
		octeon_mem_fill_fpa3(0, cmd_pool_cache, cmd_aura, 128);
	} else {
		cvmx_fpa1_enable();

		bch_pool = cvm_oct_alloc_fpa_pool(bch_pool, bch_pool_size);
		if (bch_pool < 0) {
			pr_err("cvm_oct_alloc_fpa_pool(%d, %lld)\n",
						bch_pool, bch_pool_size);
			return -ENOMEM;
		}


		cvm_oct_mem_fill_fpa(bch_pool, buf_cnt);
	}
#else
	bch_pool = (int)cvmx_fpa_get_bch_pool();
	bch_pool_size = cvmx_fpa_get_bch_pool_block_size();
	i = buf_cnt = bch_buf_count;

	debug("%s: pool: %d, pool size: %llu, buffer count: %u\n", __func__,
	      bch_pool, bch_pool_size, buf_cnt);

	cvmx_fpa_global_initialize();

	if (buf_cnt != 0) {
		i = cvmx_fpa_setup_pool(bch_pool, "BCH Buffers", NULL,
			bch_pool_size, buf_cnt);
		if (i < 0) {
			cvmx_printf("ERROR: %s: failed to init pool %d\n",
				__func__, bch_pool);
			return -1;
		}
		bch_pool = i;
	}
#endif
	result = cvmx_cmd_queue_initialize(CVMX_CMD_QUEUE_BCH, 0, bch_pool,
					   bch_pool_size);

	if (result != CVMX_CMD_QUEUE_SUCCESS) {
		cvmx_dprintf("%s: Error %d initializing command queue for BCH "
			     "pool: %d, size: %llu\n",
			     __func__, result, bch_pool,
			     (unsigned long long)bch_pool_size);
		return -1;
	}

	bch_cmd_buf.u64 = 0;

	if (OCTEON_IS_MODEL(OCTEON_CN70XX)) {
		bch_cmd_buf.cn70xx.dwb = bch_pool_size / 128;
		bch_cmd_buf.cn70xx.pool = bch_pool;
		bch_cmd_buf.cn70xx.size = bch_pool_size / 8;
		bch_cmd_buf.cn70xx.ptr = cvmx_ptr_to_phys(
			cvmx_cmd_queue_buffer(CVMX_CMD_QUEUE_BCH)) >> 7;
		cvmx_write_csr(CVMX_BCH_CMD_BUF, bch_cmd_buf.u64);
	} else if (OCTEON_IS_MODEL(OCTEON_CN73XX) ||
		OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
		cvmx_bch_cmd_ptr_t bch_cmd_ptr;

		bch_cmd_buf.cn73xx.ldwb = bch_pool_size / 128;
		bch_cmd_buf.cn73xx.aura = bch_pool;
		bch_cmd_buf.cn73xx.size = bch_pool_size / 8;
		bch_cmd_ptr.cn73xx.ptr = cvmx_ptr_to_phys(
			cvmx_cmd_queue_buffer(CVMX_CMD_QUEUE_BCH)) >> 7;
		cvmx_write_csr(CVMX_BCH_CMD_PTR, bch_cmd_ptr.u64);
		cvmx_write_csr(CVMX_BCH_CMD_BUF, bch_cmd_buf.u64);
	}

	/* Clear pending error interrupts */
	if (OCTEON_IS_MODEL(OCTEON_CN70XX)) {
		cvmx_write_csr(CVMX_BCH_GEN_INT, 7);
		cvmx_write_csr(CVMX_BCH_GEN_INT_EN, 0);
	}

	bch_ctl.u64 = cvmx_read_csr(CVMX_BCH_CTL);

	/* Enable the unit */
	if (OCTEON_IS_MODEL(OCTEON_CN70XX)) {
		bch_ctl.cn70xx.free_ena = 1;
		bch_ctl.cn70xx.one_cmd = 0;
		bch_ctl.cn70xx.erase_disable = 0;
	} else if (OCTEON_IS_MODEL(OCTEON_CN73XX) ||
		OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
		bch_ctl.cn73xx.one_cmd = 0;
		bch_ctl.cn73xx.erase_disable = 0;
	}

	cvmx_write_csr(CVMX_BCH_CTL, bch_ctl.u64);
	cvmx_read_csr(CVMX_BCH_CMD_BUF);
	return 0;
}
EXPORT_SYMBOL(cvmx_bch_initialize);

/**
 * Shutdown the BCH block
 *
 * @return Zero on success, negative on failure
 */
int cvmx_bch_shutdown(void)
{
	cvmx_bch_ctl_t bch_ctl;


	debug("%s: ENTER\n", __func__);
	bch_ctl.u64 = cvmx_read_csr(CVMX_BCH_CTL);

	if (OCTEON_IS_MODEL(OCTEON_CN70XX)) {
		bch_ctl.cn70xx.reset = 1;
	} else if (OCTEON_IS_MODEL(OCTEON_CN73XX) ||
		OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
		bch_ctl.cn73xx.reset = 1;
	}

	cvmx_write_csr(CVMX_BCH_CTL, bch_ctl.u64);
	cvmx_wait(128);

	cvmx_cmd_queue_shutdown(CVMX_CMD_QUEUE_BCH);

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL

	{
		cvm_oct_mem_empty_fpa(bch_pool,bch_buf_count);
	}
#else
	bch_pool = (int)cvmx_fpa_get_bch_pool();
	cvmx_fpa_shutdown_pool(bch_pool);
#endif
	return 0;
}
EXPORT_SYMBOL(cvmx_bch_shutdown);

/**
 * Sets the internal FPA pool data structure for bch pool.
 * @param pool	fpa pool number to use
 * @param buffer_size	buffer size of pool
 * @param buffer_count	number of buffers to allocate to pool
 */
void cvmx_bch_set_cmd_que_pool_config(int64_t pool, uint64_t buffer_size,
				       uint64_t buffer_count)
{
	bch_config.command_queue_pool.pool_num = pool;
	bch_config.command_queue_pool.buffer_size = buffer_size;
	bch_config.command_queue_pool.buffer_count = buffer_count;
}

/**
 * Gets the FPA pool data from internal data structure
 * @param bch_pool pointer to the fpa data structure to copy data
 */
void cvmx_bch_get_cmd_que_pool_config(cvmx_fpa_pool_config_t *bch_pool)
{
	*bch_pool = bch_config.command_queue_pool;
}

/**
 * Given a data block calculate the ecc data and fill in the response
 *
 * @param[in] block	8-byte aligned pointer to data block to calculate ECC
 * @param block_size	Size of block in bytes, must be a multiple of two.
 * @param ecc_level	Number of errors that must be corrected.  The number of
 *			parity bytes is equal to ((15 * ecc_level) + 7) / 8.
 *			Must be 4, 8, 16, 24, 32, 40, 48, 56, 60 or 64.
 * @param[out] ecc	8-byte aligned pointer to where ecc data should go
 * @param[in] response	pointer to where responses will be written.
 *
 * @return Zero on success, negative on failure.
 */
int cvmx_bch_encode(const void *block, uint16_t block_size,
		    uint8_t ecc_level, void *ecc,
		    volatile cvmx_bch_response_t *response)
{
	cvmx_bch_command_t command;
	cvmx_cmd_queue_result_t result;

	debug("%s(%p, %u, %u, %p, %p) ENTRY\n", __func__, block, block_size,
	      ecc_level, ecc, response);
	memset(&result, 0, sizeof(result));
	memset(&command, 0, sizeof(command));
	command.s.cword.ecc_gen = CVMX_BCH_INST_ECC_GENERATION;
	command.s.cword.ecc_level = ecc_level;
	command.s.cword.size = block_size;

	command.s.oword.ptr = cvmx_ptr_to_phys(ecc);
	command.s.iword.ptr = cvmx_ptr_to_phys((void *)block);
	command.s.resp.ptr = cvmx_ptr_to_phys((void *)response);
	debug("Cmd: cword:0x%llx, oword:0x%llx, iword:0x%llx, resp:0x%llx\n",
	      command.u64[0], command.u64[1], command.u64[2], command.u64[3]);
	result = cvmx_cmd_queue_write(CVMX_CMD_QUEUE_BCH, 1,
				      sizeof(command) / sizeof(uint64_t),
				      command.u64);
	if (result != CVMX_CMD_QUEUE_SUCCESS) {
		cvmx_dprintf("%s: Error writing to queue\n", __func__);
		return -1;
	}

	cvmx_bch_write_doorbell(1);

	return 0;
}
EXPORT_SYMBOL(cvmx_bch_encode);

/**
 * Given a data block and ecc data correct the data block
 *
 * @param[in] block_ecc_in	8-byte aligned pointer to data block with ECC
 *				data concatenated to the end to correct
 * @param block_size		Size of block in bytes, must be a multiple of
 *				two.
 * @param ecc_level		Number of errors that must be corrected.  The
 *				number of parity bytes is equal to
 *				((15 * ecc_level) + 7) / 8.
 *				Must be 4, 8, 16, 24, 32, 40, 48, 56, 60 or 64.
 * @param[out] block_out	8-byte aligned pointer to corrected data buffer.
 *				This should not be the same as block_ecc_in.
 * @param[in] response		pointer to where responses will be written.
 *
 * @return Zero on success, negative on failure.
 */
int cvmx_bch_decode(const void *block_ecc_in, uint16_t block_size,
		    uint8_t ecc_level, volatile void *block_out,
		    volatile cvmx_bch_response_t *response)
{
	cvmx_bch_command_t command;
	cvmx_cmd_queue_result_t result;

	debug("%s(%p, %u, %u, %p, %p) ENTRY\n", __func__, block_ecc_in,
	      block_size, ecc_level, block_out, response);
	memset(&result, 0, sizeof(result));
	memset(&command, 0, sizeof(command));
	command.s.cword.ecc_gen = CVMX_BCH_INST_BLOCK_CORRECTION;
	command.s.cword.ecc_level = ecc_level;
	command.s.cword.size = block_size;

	command.s.oword.ptr = cvmx_ptr_to_phys((void *)block_out);
	command.s.iword.ptr = cvmx_ptr_to_phys((void *)block_ecc_in);
	command.s.resp.ptr = cvmx_ptr_to_phys((void *)response);
	debug("Cmd: cword:0x%llx, oword:0x%llx, iword:0x%llx, resp:0x%llx\n",
	      command.u64[0], command.u64[1], command.u64[2], command.u64[3]);
	result = cvmx_cmd_queue_write(CVMX_CMD_QUEUE_BCH, 1,
				      sizeof(command) / sizeof(uint64_t),
				      command.u64);
	if (result != CVMX_CMD_QUEUE_SUCCESS) {
		cvmx_dprintf("%s: Error writing to queue\n", __func__);
		return -1;
	}

	cvmx_bch_write_doorbell(1);
	return 0;
}
EXPORT_SYMBOL(cvmx_bch_decode);
