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

#ifndef __CVMX_HELPER_PKO3_H__
#define __CVMX_HELPER_PKO3_H__

#ifdef	__cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

/*
 * Initialize PKO3 unit on the current node.
 *
 * Covers the common hardware, memory and global configuration.
 * Per-interface intialization is performed separately.
 *
 * @return 0 on success.
 *
 */
extern int cvmx_helper_pko3_init_global(unsigned int node);
int __cvmx_helper_pko3_init_global(unsigned int node, uint16_t gaura);

/** 
 * Initialize a simple interface with a a given number of
 * fair or prioritized queues.
 * This function will assign one channel per sub-interface.
 */
int __cvmx_pko3_config_gen_interface(int xiface, uint8_t subif,
				     uint8_t num_queues, bool prioritized);

/*
 * Configure and initialize PKO3 for an interface
 *
 * @param interface is the interface number to configure
 * @return 0 on success.
 *
 */
int cvmx_helper_pko3_init_interface(int xiface);
int __cvmx_pko3_helper_dqs_activate(int xiface, int index, bool min_pad);

/**
 * Uninitialize PKO3 interface
 *
 * Release all resources held by PKO for an interface.
 * The shutdown code is the same for all supported interfaces.
 */
extern int cvmx_helper_pko3_shut_interface(int xiface);

/**
 * Shutdown PKO3
 *
 * Should be called after all interfaces have been shut down on the PKO3.
 *
 * Disables the PKO, frees all its buffers.
 */
extern int cvmx_helper_pko3_shutdown(unsigned int node);

/**
 * Show integrated PKO configuration.
 *
 * @param node	   node number
 */
int cvmx_helper_pko3_config_dump(unsigned int node);

/**
 * Show integrated PKO statistics.
 *
 * @param node	   node number
 */
int cvmx_helper_pko3_stats_dump(unsigned int node);

/**
 * Clear PKO statistics.
 *
 * @param node	   node number
 */
void cvmx_helper_pko3_stats_clear(unsigned int node);

#ifdef	__cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif
#endif /* __CVMX_HELPER_PKO3_H__ */
