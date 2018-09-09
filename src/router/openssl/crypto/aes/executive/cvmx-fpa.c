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
 * Support library for the hardware Free Pool Allocator.
 *
 * <hr>$Revision: 89291 $<hr>
 *
 */

#include "cvmx.h"
#include "cvmx-fpa.h"
#include "cvmx-helper-fpa.h"
#include "cvmx-ipd.h"
#include "cvmx-bootmem.h"

/**
 * Current state of all the pools. Use access functions
 * instead of using it directly.
 */
CVMX_SHARED cvmx_fpa_pool_info_t cvmx_fpa_pool_info[CVMX_MAX_NODES][CVMX_FPA3_NUM_POOLS];

CVMX_SHARED cvmx_fpa_aura_info_t cvmx_fpa_aura_info[CVMX_MAX_NODES][CVMX_FPA_AURA_NUM];

/**
 * Setup a FPA pool to control a new block of memory. The
 * buffer pointer must be a virtual address.
 *
 * @param pool       Pool to initialize
 *                   0 <= pool < 8
 * @param name       Constant character string to name this pool.
 *                   String is not copied.
 * @param buffer     Pointer to the block of memory to use. This must be
 *                   accessible by all processors and external hardware.
 * @param block_size Size for each block controlled by the FPA
 * @param num_blocks Number of blocks
 *
 * @return 0 on Success,
 *         -1 on failure
 */
int cvmx_fpa_setup_pool(uint64_t pool, const char *name, void *buffer,
			uint64_t block_size, uint64_t num_blocks)
{
	char *ptr;
	uint16_t iter_i, iter_j;

	if (!buffer) {
		cvmx_dprintf("ERROR: cvmx_fpa_setup_pool: NULL buffer pointer!\n");
		return -1;
	}
	if (pool >= CVMX_FPA_NUM_POOLS) {
		cvmx_dprintf("ERROR: cvmx_fpa_setup_pool: Illegal pool!\n");
		return -1;
	}

	if (block_size < CVMX_FPA_MIN_BLOCK_SIZE) {
		cvmx_dprintf("ERROR: cvmx_fpa_setup_pool: Block size too small.\n");
		return -1;
	}

	if (((unsigned long)buffer & (CVMX_FPA_ALIGNMENT - 1)) != 0) {
		cvmx_dprintf("ERROR: cvmx_fpa_setup_pool: Buffer not aligned properly.\n");
		return -1;
	}

	for (iter_i = 0; iter_i < CVMX_MAX_NODES; iter_i++) {
		for (iter_j = 0; iter_j < CVMX_FPA_NUM_POOLS; iter_j++) {
			if (strncmp(cvmx_fpa_pool_info[iter_i][iter_j].name,
						name,
						CVMX_FPA_POOL_NAME_LEN - 1)
					== 0) {
				cvmx_dprintf("ERROR: cvmx_fpa_setup_pool: "
						"Pool name %s is in use by "
						"fpa pool[%d][%d] \n",
						name, iter_i, iter_j);
				return -1;
			}
		}
	}

	/* FPA3 is handled differently */
	if ((octeon_has_feature(OCTEON_FEATURE_FPA3))) {
		if (__cvmx_fpa3_setup_pool(pool, name, buffer, block_size, 
					   num_blocks) < 0)
			return -1;
	} else {
		if (strlen(name) > CVMX_FPA_POOL_NAME_LEN - 1) {
			cvmx_dprintf("WARNING: cvmx_fpa_setup_pool: "
				     "name length exceeding limit, truncating.\n");
			strncpy(cvmx_fpa_pool_info[0][pool].name, name,
				CVMX_FPA_POOL_NAME_LEN - 1);
			cvmx_fpa_pool_info[0][pool].
				name[CVMX_FPA_POOL_NAME_LEN - 1] = '\0';
		} else
			strcpy(cvmx_fpa_pool_info[0][pool].name, name);

		cvmx_fpa_pool_info[0][pool].size = block_size;
		cvmx_fpa_pool_info[0][pool].starting_element_count = num_blocks;
		cvmx_fpa_pool_info[0][pool].base = buffer;

		ptr = (char *)buffer;
		while (num_blocks--) {
			cvmx_fpa_free(ptr, pool, 0);
			ptr += block_size;
		}
	}

	return 0;
}

int cvmx_fpa_config_red_params(int node, int qos_avg_en, int red_lvl_dly, int avg_dly)
{
	cvmx_fpa_gen_cfg_t fpa_cfg;
	cvmx_fpa_red_delay_t red_delay;

	fpa_cfg.u64 = cvmx_read_csr_node(node,CVMX_FPA_GEN_CFG);
	fpa_cfg.s.avg_en = qos_avg_en;
	fpa_cfg.s.lvl_dly = red_lvl_dly;
	cvmx_write_csr_node(node,CVMX_FPA_GEN_CFG,fpa_cfg.u64);

	red_delay.u64 = cvmx_read_csr_node(node,CVMX_FPA_RED_DELAY);
	red_delay.s.avg_dly = avg_dly;
	cvmx_write_csr_node(node,CVMX_FPA_RED_DELAY,red_delay.u64);
	return 0;
}

int cvmx_fpa_pool_stack_init(int node, int pool, const char *name, int mem_node,
		int max_buffer_cnt, enum fpa_pool_alignment align,
		int buffer_sz)
{
	uint64_t stack_base;
	int stack_memory_size = (max_buffer_cnt * 128)/ 29 + 128;
	cvmx_fpa_poolx_cfg_t pool_cfg;
	uint16_t iter_i, iter_j;

	stack_memory_size = (stack_memory_size >> 7 ) << 7;
	stack_base = cvmx_bootmem_phy_alloc_range(stack_memory_size,
						  CVMX_CACHE_LINE_SIZE,0,0);
	if (stack_base == 0)
		return -1;

	for (iter_i = 0; iter_i < CVMX_MAX_NODES; iter_i++) {
		for (iter_j = 0; iter_j < CVMX_FPA3_NUM_POOLS; iter_j++) {
			if (strncmp(cvmx_fpa_pool_info[iter_i][iter_j].name,
						name,
						CVMX_FPA_POOL_NAME_LEN - 1)
					== 0) {
				cvmx_dprintf("ERROR: cvmx_fpa_pool_stack_init: "
						"Pool name %s is in use by "
						"fpa pool[%d][%d] \n",
						name, iter_i, iter_j);
				return -1;
			}
		}
	}

	if (strlen(name) > CVMX_FPA_POOL_NAME_LEN - 1 ) {
		cvmx_dprintf("ERROR: cvmx_fpa_pool_stack_init: "
				"name length exceeding limit, truncating.\n");
		strncpy(cvmx_fpa_pool_info[node][pool].name, name,
				CVMX_FPA_POOL_NAME_LEN - 1);
		cvmx_fpa_pool_info[node][pool].
			name[CVMX_FPA_POOL_NAME_LEN - 1] = '\0';
	} else
		strcpy(cvmx_fpa_pool_info[node][pool].name, name);

	cvmx_fpa_pool_info[node][pool].size = buffer_sz;
	cvmx_fpa_pool_info[node][pool].max_buffer_cnt = max_buffer_cnt;
	cvmx_fpa_pool_info[node][pool].stack_base = stack_base;
	cvmx_write_csr_node(node, CVMX_FPA_POOLX_STACK_BASE(pool), stack_base);
	cvmx_write_csr_node(node, CVMX_FPA_POOLX_STACK_ADDR(pool), stack_base);
	pool_cfg.u64 = 0;
	if (align == FPA_NATURAL_ALIGNMENT) {
		uint64_t stack_end = stack_base + stack_memory_size;
		cvmx_fpa_poolx_stack_end_t stack_end_reg;
		stack_end_reg.u64 = 0;
		stack_end_reg.s.addr = stack_end >> 7;
		stack_end_reg.s.addr++;
		pool_cfg.cn78xx.nat_align = 1;
		pool_cfg.cn78xx.buf_size = buffer_sz >> 7;
		cvmx_write_csr_node(node, CVMX_FPA_POOLX_STACK_END(pool),
				    stack_end_reg.u64);
	}
	pool_cfg.cn78xx.l_type = 0x2;
	pool_cfg.cn78xx.ena = 0;
	//cvmx_dprintf("pool=%d config=%llx\n", pool, (ULL) pool_cfg.u64);
	cvmx_write_csr_node(node, CVMX_FPA_POOLX_CFG(pool), pool_cfg.u64);
	pool_cfg.cn78xx.ena = 1;
	cvmx_write_csr_node(node, CVMX_FPA_POOLX_CFG(pool), pool_cfg.u64);
	return 0;
}

int cvmx_fpa_aura_init(int node, int aura, const char *name, int mem_node,
		       void *buffers, int buffers_cnt, int ptr_dis)
{
	char *ptr;
	int pool_num;
	uint64_t pool_size;
	uint16_t iter_i, iter_j;

	for (iter_i = 0; iter_i < CVMX_MAX_NODES; iter_i++) {
		for (iter_j = 0; iter_j < CVMX_FPA_AURA_NUM; iter_j++) {
			if (strncmp(cvmx_fpa_aura_info[iter_i][iter_j].name,
						name,
						CVMX_FPA_AURA_NAME_LEN - 1)
					== 0) {
				cvmx_dprintf("ERROR: cvmx_fpa_aura_init: "
						"Aura name %s is in use by "
						"fpa aura[%d][%d] \n",
						name, iter_i, iter_j);
				return -1;
			}
		}
	}

	if (strlen(name) >  CVMX_FPA_AURA_NAME_LEN - 1) {
		cvmx_dprintf("ERROR: cvmx_fpa_aura_init: "
				"name length exceeding limit, truncating.\n");
		strncpy(cvmx_fpa_aura_info[node][aura].name, name,
				CVMX_FPA_AURA_NAME_LEN - 1);
		cvmx_fpa_aura_info[node][aura].
			name[CVMX_FPA_AURA_NAME_LEN - 1] = '\0';
	} else
		strcpy(cvmx_fpa_aura_info[node][aura].name, name);

	pool_num = cvmx_fpa_aura_info[node][aura].pool_num;
	if ((pool_num < 0) || (pool_num > CVMX_FPA3_NUM_POOLS)) {
		cvmx_dprintf("ERROR: invalid pool_num=%d for aura=%d:%d\n",
			     pool_num, node, aura);
		return -1;
	}
	pool_size = buffers_cnt * cvmx_fpa_pool_info[node][pool_num].size;
	/* TODO: Allocate buffers to be aligned on ??*/
	if (buffers)
		ptr = (char *)buffers;
	else
		ptr = (char *)cvmx_bootmem_alloc_range(pool_size,
						       CVMX_CACHE_LINE_SIZE,
						       0,0);
	if (ptr == NULL) {
		cvmx_dprintf("ERROR: Failed to allocated bootme size=%llx\n",
			     CAST_ULL(pool_size));
		return -1;
	}

	__cvmx_fpa_aura_cfg(node, aura, pool_num, buffers_cnt, ptr_dis);
	while(buffers_cnt--) {
		/*cvmx_dprintf("buf_cnt = %d ptr=%p sz=%llx buffersz=%llx\n",
			     buffers_cnt, ptr, (ULL) pool_size,
			     (ULL) cvmx_fpa_pool_info[node][pool_num].size);*/
		cvmx_fpa_free_aura(ptr, node, aura, 0);
		ptr += cvmx_fpa_pool_info[node][pool_num].size;
	}
	return 0;
}

/**
 * This will map auras specified in the auras_index[] array to specified
 * FPA pool_index.
 * The array is assumed to contain count number of entries.
 * @param count is the number of entries in the auras_index array.
 * @pool_index is the index of the fpa pool.
 * @return 0 on success
 */
int cvmx_fpa_assign_auras(int node, int auras_index[], int count,
			  int pool_index)
{
	int i;

	/* TODO: Also need to use range object to allocate the specified
	aura_index */
	for(i=0; i<count; i++) {
		int aura_index = auras_index[i];
		if ((aura_index < 0) || (aura_index > CVMX_FPA_AURA_NUM)) {
			cvmx_dprintf("ERROR: invalid aura index=%d\n",
				     aura_index);
			return -1;
		}
		cvmx_fpa_aura_info[node][aura_index].pool_num = pool_index;
	}
	return 0;
}


/**
 * Shutdown a Memory pool and validate that it had all of
 * the buffers originally placed in it. This should only be
 * called by one processor after all hardware has finished
 * using the pool. Most like you will want to have called
 * cvmx_helper_shutdown_packet_io_global() before this
 * function to make sure all FPA buffers are out of the packet
 * IO hardware.
 *
 * @param pool   Pool to shutdown
 *
 * @return Zero on success
 *         - Positive is count of missing buffers
 *         - Negative is too many buffers or corrupted pointers
 */
uint64_t cvmx_fpa_shutdown_pool(uint64_t pool)
{
	int errors = 0;
	int count = 0;
	int expected_count = cvmx_fpa_pool_info[0][pool].starting_element_count;
	uint64_t base = cvmx_ptr_to_phys(cvmx_fpa_pool_info[0][pool].base);
	uint64_t finish = base + cvmx_fpa_pool_info[0][pool].size * expected_count;

	count = 0;
	while (1) {
		uint64_t address;
		void *ptr = cvmx_fpa_alloc(pool);
		if (!ptr)
			break;

		address = cvmx_ptr_to_phys(ptr);
		if ((address >= base) && (address < finish) &&
		    (((address - base) % cvmx_fpa_pool_info[0][pool].size) == 0)) {
			count++;
		} else {
			cvmx_dprintf("ERROR: cvmx_fpa_shutdown_pool: Illegal "
				     "address 0x%llx in pool %s(%d)\n",
				     CAST_ULL(address),
				     cvmx_fpa_pool_info[0][pool].name,
				     (int)pool);
			errors++;
		}
	}

	if (count < expected_count) {
		cvmx_dprintf("ERROR: cvmx_fpa_shutdown_pool: Pool %s(%d) "
			     "missing %d buffers\n",
			     cvmx_fpa_pool_info[0][pool].name, (int)pool,
			     expected_count - count);
	} else if (count > expected_count) {
		cvmx_dprintf("ERROR: cvmx_fpa_shutdown_pool: Pool %s(%d) "
			     "had %d duplicate buffers\n",
			     cvmx_fpa_pool_info[0][pool].name, (int)pool,
			     count - expected_count);
	}

	if (errors) {
		cvmx_dprintf("ERROR: cvmx_fpa_shutdown_pool: Pool %s(%d) "
			     "started at 0x%llx, ended at 0x%llx, with a step "
			     "of 0x%x\n", cvmx_fpa_pool_info[0][pool].name,
			     (int)pool, CAST_ULL(base), CAST_ULL(finish),
			     (int)cvmx_fpa_pool_info[0][pool].size);
		return -errors;
	} else
		return expected_count - count;
}

/**
 * Gets the block size of buffer in specified pool
 * @param pool	 Pool to get the block size from
 * @return       Size of buffer in specified pool
 */
#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
uint64_t cvmx_fpa_get_block_size(uint64_t pool)
{
	return (cvmx_fpa_pool_info[0][pool].size);

}
#endif

/**
 * Initialize global configuration for FPA block.
 */
int cvmx_fpa_global_initialize(void)
{
	cvmx_fpa_enable();
	return 0;
}

/**
 * Initialize global configuration for FPA block for specified node.
 */
int cvmx_fpa_global_init_node(int node)
{
	cvmx_fpa_enable();
	return 0;
}
