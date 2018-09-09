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
 * <hr>$Revision: 0 $<hr>
 */

#ifndef __CVMX_PKO3_RESOURCES_H__
#define __CVMX_PKO3_RESOURCES_H__

#ifdef	__cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

enum {
	CVMX_PKO_PORT_QUEUES = 0,
	CVMX_PKO_L2_QUEUES,
	CVMX_PKO_L3_QUEUES,
	CVMX_PKO_L4_QUEUES,
	CVMX_PKO_L5_QUEUES,
	CVMX_PKO_DESCR_QUEUES
};

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
extern int cvmx_pko_alloc_queues(int node, int level, int owner, int queue_base, int num_queues);

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
extern int cvmx_pko_alloc_queues_many(int node, int level, int owner, int queues_allocated[], int num_queues);

/*
 * Allocate or reserve PKO Port Index. 
 *
 * @param node is the node number for this resource.
 * @param owner is the owner of PKO Port Index.
 * @param port_index_base is the PKO Port Index number(specify -1 to allocate).
 * @param num_index is the number of PKO Port Index that have to be reserved or allocated.
 * @return returns port_index_base if successful or -1 on failure.
 */
extern int cvmx_pko_alloc_pko_port_index(int node, int owner, int port_index_base, int num_index);


#ifdef	__cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif
#endif /* __CVMX_PKO3_RESOURCES_H__ */
