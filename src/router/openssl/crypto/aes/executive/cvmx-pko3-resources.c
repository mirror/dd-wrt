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
#include <asm/octeon/cvmx-pko3.h>
#include "asm/octeon/cvmx-global-resources.h"
#include <asm/octeon/cvmx-pko3-resources.h>
#include "asm/octeon/cvmx-range.h"
#else
#include "cvmx.h"
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

#define CVMX_GR_TAG_PKO_PORT_INDEX(x)  cvmx_get_gr_tag('c','v','m','_','p','k','o','p','i','d','_',(x+'0'),'.','.','.','.')

static inline struct global_resource_tag __cvmx_pko_get_queues_resource_tag(int node, int queue_level)
{
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
			return cvmx_get_gr_tag('i','n','v','a','l','i','d','.','.','.','.','.','.','.','.','.');
	}
}

static inline int __cvmx_pko_get_num_queues(int queue_level)
{
	switch(queue_level) {
		case CVMX_PKO_PORT_QUEUES:
			return CVMX_PKO_NUM_PORT_QUEUES;
		case CVMX_PKO_L2_QUEUES:
			return CVMX_PKO_NUM_L2_QUEUES;
		case CVMX_PKO_L3_QUEUES:
			return CVMX_PKO_NUM_L3_QUEUES;
		case CVMX_PKO_L4_QUEUES:
			return CVMX_PKO_NUM_L4_QUEUES;
		case CVMX_PKO_L5_QUEUES:
			return CVMX_PKO_NUM_L5_QUEUES;
		case CVMX_PKO_DESCR_QUEUES:
			return CVMX_PKO_NUM_DESCR_QUEUES;
		default:
			return -1; 
	}
}

/*
 * Allocate or reserve contiguous list of PKO queues.
 *
 * @param node is the node number for PKO queues.
 * @param level is the PKO queue level.
 * @param owner is the owner of PKO queue resources.
 * @param queue_base is the PKO queue base number(specify -1 to allocate).
 * @param num_queues is the number of PKO queues that have to be reserved or allocated.
 * @return returns queue_base if successful or -1 on failure.
 */
int cvmx_pko_alloc_queues(int node, int level, int owner, int queue_base, int num_queues) 
{
	struct global_resource_tag tag = __cvmx_pko_get_queues_resource_tag(node, level);
	int max_num_queues = __cvmx_pko_get_num_queues(level);

	if (cvmx_create_global_resource_range(tag, max_num_queues)) {
		cvmx_dprintf("ERROR: Failed to create pko level %d queues global resource\n", level);
		return -1;
	}

	if (queue_base >= 0) {
		queue_base = cvmx_reserve_global_resource_range(tag, owner, queue_base, num_queues);
	}
	else {
		queue_base = cvmx_allocate_global_resource_range(tag, owner, num_queues, 1);
	}

	if (queue_base == -1)
		cvmx_dprintf("Error: Failed to alloc/reserve pko level %d queues\n", level);

	return queue_base;
}
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
EXPORT_SYMBOL(cvmx_pko_alloc_queues);
#endif

/*
 * Allocate or reserve multiple PKO queues.
 *
 * @param node is the node number for PKO queues.
 * @param level is the PKO queue level.
 * @param owner is the owner of PKO queue resources.
 * @param queues_allocated is the PKO queue array list for reservation or allocation(specify -1 to allocate).
 * @param num_queues is the number of PKO queues that have to be reserved or allocated.
 * @return returns 0 if successful or -1 on failure.
 */
int cvmx_pko_alloc_queues_many(int node, int level, int owner, int queues_allocated[], int num_queues) 
{
	struct global_resource_tag tag = __cvmx_pko_get_queues_resource_tag(node, level);
	int max_num_queues = __cvmx_pko_get_num_queues(level);
	int count = 0, ret_val = 0;

	if (cvmx_create_global_resource_range(tag, max_num_queues)) {
		cvmx_dprintf("ERROR: Failed to create pko level %d queues global resource\n", level);
		return -1;
	}

	if (queues_allocated[0] >= 0) {
		for (count = 0; count < num_queues; count++) {
			queues_allocated[count] = cvmx_reserve_global_resource_range(tag, owner, queues_allocated[count], 1);
			if (queues_allocated[count] < 0) {
				cvmx_dprintf("Error: Failed to reserve pko level %d queues\n", level);
				return -1;
			}
		}
	}
	else {
		ret_val = cvmx_resource_alloc_many(tag, owner, num_queues, 
						   queues_allocated);
	}

	return ret_val;
}
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
EXPORT_SYMBOL(cvmx_pko_alloc_queues_many);
#endif

/*
 * Allocate or reserve PKO Port Index. 
 *
 * @param node is the node number for this resource.
 * @param owner is the owner of PKO Port Index.
 * @param port_index_base is the PKO Port Index number(specify -1 to allocate).
 * @param num_index is the number of PKO Port Index that have to be reserved or allocated.
 * @return returns port_index_base if successful or -1 on failure.
 */
int cvmx_pko_alloc_pko_port_index(int node, int owner, int port_index_base, int num_index) 
{

	if (cvmx_create_global_resource_range(CVMX_GR_TAG_PKO_PORT_INDEX(node), CVMX_PKO_NUM_PORT_INDEX)) {
		cvmx_dprintf("ERROR: Failed to create pko port index global resource\n");
		return -1;
	}

	if (port_index_base >= 0) {
		port_index_base = cvmx_reserve_global_resource_range(CVMX_GR_TAG_PKO_PORT_INDEX(node), owner, 
									port_index_base, num_index);
	}
	else {
		port_index_base = cvmx_allocate_global_resource_range(CVMX_GR_TAG_PKO_PORT_INDEX(node), owner,
									 num_index, 1);
	}

	if (port_index_base == -1)
		cvmx_dprintf("Error: Failed to alloc/reserve pko port index\n");

	return port_index_base;
}
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
EXPORT_SYMBOL(cvmx_pko_alloc_pko_port_index);
#endif
