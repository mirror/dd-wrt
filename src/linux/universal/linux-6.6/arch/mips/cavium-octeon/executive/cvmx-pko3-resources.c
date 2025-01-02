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
 * PKO resources.
 */

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <linux/module.h>
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-bootmem.h>
#include <asm/octeon/cvmx-pko3.h>
#include "asm/octeon/cvmx-global-resources.h"
#include <asm/octeon/cvmx-pko3-resources.h>
#include "asm/octeon/cvmx-range.h"
#else
#include "cvmx.h"
#include "cvmx-bootmem.h"
#include "cvmx-pko3.h"
#include "cvmx-global-resources.h"
#include "cvmx-pko3-resources.h"
#include "cvmx-range.h"
#endif

#define CVMX_GR_TAG_PKO_PORT_QUEUES(x)   cvmx_get_gr_tag('c','v','m','_','p','k','o','p','o','q','_',(x+'0'),'.','.','.','.')
#define CVMX_GR_TAG_PKO_L2_QUEUES(x)   	 cvmx_get_gr_tag('c','v','m','_','p','k','o','l','2','q','_',(x+'0'),'.','.','.','.')
#define CVMX_GR_TAG_PKO_L3_QUEUES(x)     cvmx_get_gr_tag('c','v','m','_','p','k','o','l','3','q','_',(x+'0'),'.','.','.','.')
#define CVMX_GR_TAG_PKO_L4_QUEUES(x)     cvmx_get_gr_tag('c','v','m','_','p','k','o','l','4','q','_',(x+'0'),'.','.','.','.')
#define CVMX_GR_TAG_PKO_L5_QUEUES(x)     cvmx_get_gr_tag('c','v','m','_','p','k','o','l','5','q','_',(x+'0'),'.','.','.','.')
#define CVMX_GR_TAG_PKO_DESCR_QUEUES(x)  cvmx_get_gr_tag('c','v','m','_','p','k','o','d','e','q','_',(x+'0'),'.','.','.','.')
#define CVMX_GR_TAG_PKO_PORT_INDEX(x)  	 cvmx_get_gr_tag('c','v','m','_','p','k','o','p','i','d','_',(x+'0'),'.','.','.','.')

/*
 * @INRWENAL
 * Per-DQ parameters, current and maximum queue depth counters
 */
CVMX_SHARED cvmx_pko3_dq_params_t  *__cvmx_pko3_dq_params[CVMX_MAX_NODES];

static const short cvmx_pko_num_queues_78XX[256] = 
{
	[CVMX_PKO_PORT_QUEUES] = 32,
	[CVMX_PKO_L2_QUEUES] = 512,
	[CVMX_PKO_L3_QUEUES] = 512,
	[CVMX_PKO_L4_QUEUES] = 1024,
	[CVMX_PKO_L5_QUEUES] = 1024,
	[CVMX_PKO_DESCR_QUEUES] = 1024
};

static const short cvmx_pko_num_queues_73XX[256] = 
{
	[CVMX_PKO_PORT_QUEUES] = 16,
	[CVMX_PKO_L2_QUEUES] = 256,
	[CVMX_PKO_L3_QUEUES] = 256,
	[CVMX_PKO_L4_QUEUES] = 0,
	[CVMX_PKO_L5_QUEUES] = 0,
	[CVMX_PKO_DESCR_QUEUES] = 256
};

int cvmx_pko3_num_level_queues(enum cvmx_pko3_level_e level)
{
	unsigned nq = 0, ne = 0;

	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		ne = NUM_ELEMENTS(cvmx_pko_num_queues_78XX);
		nq =  cvmx_pko_num_queues_78XX[level];
	}
	if (OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
		ne = NUM_ELEMENTS(cvmx_pko_num_queues_73XX);
		nq =  cvmx_pko_num_queues_73XX[level];
	}

	if (nq == 0 || level >= ne) {
		cvmx_printf("ERROR: %s: queue level %#x invalid\n",
			__func__, level);
		return -1;
	}

	return nq;
}

static inline struct global_resource_tag
__cvmx_pko_get_queues_resource_tag(int node, enum cvmx_pko3_level_e queue_level)
{
	if (cvmx_pko3_num_level_queues(queue_level) == 0) {
		cvmx_printf("ERROR: %s: queue level %#x invalid\n",
				__func__, queue_level);
		return CVMX_GR_TAG_INVALID;
	}

	switch(queue_level) {
		case CVMX_PKO_PORT_QUEUES:
			return CVMX_GR_TAG_PKO_PORT_QUEUES(node);
		case CVMX_PKO_L2_QUEUES:
			return CVMX_GR_TAG_PKO_L2_QUEUES(node);
		case CVMX_PKO_L3_QUEUES:
			return CVMX_GR_TAG_PKO_L3_QUEUES(node);
		case CVMX_PKO_L4_QUEUES:
			return CVMX_GR_TAG_PKO_L4_QUEUES(node);
		case CVMX_PKO_L5_QUEUES:
			return CVMX_GR_TAG_PKO_L5_QUEUES(node);
		case CVMX_PKO_DESCR_QUEUES:
			return CVMX_GR_TAG_PKO_DESCR_QUEUES(node);
		default:
			cvmx_printf("ERROR: %s: queue level %#x invalid\n",
				__func__, queue_level);
			return CVMX_GR_TAG_INVALID;
	}
}

/**
 * Allocate or reserve a pko resource - called by wrapper functions
 * @param tag processed global resource tag
 * @param base_queue if specified the queue to reserve
 * @param owner to be specified for resource
 * @param num_queues to allocate
 * @param max_num_queues for global resource
 */
int cvmx_pko_alloc_global_resource(struct global_resource_tag tag, int base_queue, int owner, int num_queues, int max_num_queues)
{
	int res;
	if(cvmx_create_global_resource_range(tag, max_num_queues)) {
		cvmx_dprintf("ERROR: Failed to create PKO3 resource: %lx-%lx\n",
			(unsigned long) tag.hi, (unsigned long) tag.lo);
		return -1;
	}
	if(base_queue >= 0) {
		res = cvmx_reserve_global_resource_range(tag, owner, base_queue, num_queues);
	} else {
		res = cvmx_allocate_global_resource_range(tag, owner, num_queues, 1);
	}
	if(res < 0) {
		cvmx_dprintf("ERROR: Failed to %s PKO3 tag %lx:%lx, %i %i %i %i.\n",
			((base_queue < 0) ? "allocate" : "reserve"),
			(unsigned long) tag.hi, (unsigned long) tag.lo,
			base_queue, owner, num_queues, max_num_queues);
		return -1;
	}

	return res;
}

/**
 * Allocate or reserve PKO queues - wrapper for cvmx_pko_alloc_global_resource
 *
 * @param node on which to allocate/reserve PKO queues
 * @param level of PKO queue
 * @param owner of reserved/allocated resources
 * @param base_queue to start reservation/allocatation
 * @param num_queues number of queues to be allocated
 * @return 0 on success, -1 on failure
 */
int cvmx_pko_alloc_queues(int node, int level, int owner, int base_queue, int num_queues)
{
	struct global_resource_tag tag = __cvmx_pko_get_queues_resource_tag(node, level);
	int max_num_queues = cvmx_pko3_num_level_queues(level);

	return cvmx_pko_alloc_global_resource(tag, base_queue, owner, num_queues, max_num_queues);
}
EXPORT_SYMBOL(cvmx_pko_alloc_queues);

/**
 * Free an allocated/reserved PKO queues for a certain level and owner
 *
 * @param node on which to allocate/reserve PKO queues
 * @param level of PKO queue
 * @param owner of reserved/allocated resources
 * @return 0 on success, -1 on failure
 */
int cvmx_pko_free_queues(int node, int level, int owner)
{
	struct global_resource_tag tag = __cvmx_pko_get_queues_resource_tag(node, level);

	return cvmx_free_global_resource_range_with_owner(tag, owner);
}
EXPORT_SYMBOL(cvmx_pko_free_queues);

/**
 * @INTERNAL
 *
 * Initialize the pointer to the descriptor queue parameter table.
 * The table is one named block per node, and may be shared between
 * applications.
 */
int __cvmx_pko3_dq_param_setup(unsigned node)
{
#if !defined(__U_BOOT__)
	cvmx_pko3_dq_params_t  *pParam;
	char block_name[] = "cvmx_pko3_dq_globl_param_0";
	unsigned i;

	pParam = __cvmx_pko3_dq_params[node];
	if (pParam != NULL)
		return 0;

	/* Adjust block name with node# */
	i = strlen(block_name);
	block_name[i-1] += node;

	/* Get number of descriptor queues for sizing the table */
	i = cvmx_pko3_num_level_queues(CVMX_PKO_DESCR_QUEUES);

	pParam = cvmx_bootmem_alloc_named_range_once(
		/* size */
		sizeof(cvmx_pko3_dq_params_t) * i,
		/* min_addr, max_addr, align */
		0ull, 0ull, sizeof(cvmx_pko3_dq_params_t),
		block_name, NULL);

	if (pParam == NULL)
		return -1;

	__cvmx_pko3_dq_params[node] = pParam;
#endif	/* ! U_BOOT */
	return 0;
}


