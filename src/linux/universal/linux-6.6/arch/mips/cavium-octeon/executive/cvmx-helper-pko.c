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
 * Helper Functions for the PKO
 *
 * $Id: cvmx-helper-pko.c 115744 2015-04-04 04:36:36Z awilliams $
 */

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <asm/octeon/octeon.h>
#include <asm/octeon/cvmx-hwpko.h>
#include <asm/octeon/cvmx-fpa1.h>
#include <asm/octeon/cvmx-clock.h>
#include <asm/octeon/cvmx-helper-cfg.h>
#else
#include "cvmx.h"
#include "cvmx-bootmem.h"
#include "cvmx-helper.h"
#include "cvmx-helper-ilk.h"
#include "cvmx-ipd.h"
#include "cvmx-fpa1.h"
#include "cvmx-hwpko.h"
#include "cvmx-global-resources.h"
#endif

/* TODO: XXX- these config data structures will go away soon! */
static CVMX_SHARED int64_t pko_fpa_config_pool = -1;
static CVMX_SHARED uint64_t pko_fpa_config_size = 1024;
static CVMX_SHARED uint64_t pko_fpa_config_count = 0;


/**
 * cvmx_override_pko_queue_priority(int pko_port, uint64_t
 * priorities[16]) is a function pointer. It is meant to allow
 * customization of the PKO queue priorities based on the port
 * number. Users should set this pointer to a function before
 * calling any cvmx-helper operations.
 */
CVMX_SHARED void (*cvmx_override_pko_queue_priority) (int ipd_port, uint8_t *priorities) = NULL;
EXPORT_SYMBOL(cvmx_override_pko_queue_priority);


void cvmx_pko_set_cmd_que_pool_config(int64_t pool, uint64_t buffer_size,
				    uint64_t buffer_count)
{
	pko_fpa_config_pool = pool;
	pko_fpa_config_size = buffer_size;
	pko_fpa_config_count = buffer_count;

}
EXPORT_SYMBOL(cvmx_pko_set_cmd_que_pool_config);

void cvmx_pko_set_cmd_queue_pool_buffer_count(uint64_t buffer_count)
{
	pko_fpa_config_count = buffer_count;
}

void cvmx_pko_get_cmd_que_pool_config(cvmx_fpa_pool_config_t *pko_pool)
{
	pko_pool->pool_num = pko_fpa_config_pool;
	pko_pool->buffer_size = pko_fpa_config_size;
	pko_pool->buffer_count = pko_fpa_config_count;
}

int64_t cvmx_fpa_get_pko_pool(void)
{
	return pko_fpa_config_pool;
}

/**
 * Gets the buffer size of pko pool
 */
uint64_t cvmx_fpa_get_pko_pool_block_size(void)
{
	return pko_fpa_config_size;
}

/**
 * Gets the buffer size  of pko pool
 */
uint64_t cvmx_fpa_get_pko_pool_buffer_count(void)
{
	return pko_fpa_config_count;
}

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
/**
 * Initialize PKO command queue buffer pool
 */
static int cvmx_helper_pko_pool_init(void)
{
	uint8_t pool;
	unsigned buf_count;
	unsigned pkt_buf_count;
	int rc;

	/* Reserve pool */
	pool = cvmx_fpa_get_pko_pool();

	/* Avoid redundant pool creation */
	if (cvmx_fpa_get_block_size(pool) > 0) {
#ifdef DEBUG
		cvmx_dprintf("WARNING: %s: "
			"pool %d already initialized\n",
			__func__, pool);
#endif
		/* It is up to the app to have sufficient buffer count */
		return pool;
	}

	/* Calculate buffer count: one per queue + 3-word-cmds * max_pkts */
	pkt_buf_count = cvmx_fpa_get_packet_pool_buffer_count();
	buf_count = CVMX_PKO_MAX_OUTPUT_QUEUES + (pkt_buf_count * 3) / 8;

	/* Allocate pools for pko command queues */
	rc = __cvmx_helper_initialize_fpa_pool(pool,
				cvmx_fpa_get_pko_pool_block_size(),
				buf_count, "PKO Cmd-bufs");

	if (rc < 0)
		cvmx_dprintf("%s: ERROR: in PKO buffer pool\n", __func__);

	pool = rc;
	return pool;
}
#endif


/**
 * Initialize the PKO
 *
 */
int cvmx_helper_pko_init(void)
{
#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	int rc;

	rc = cvmx_helper_pko_pool_init();
	if (rc < 0)
		return rc;
#else
	/* #	error "Pool number in kernel not implemented" */
#endif

	__cvmx_helper_init_port_config_data(0);

	cvmx_pko_hw_init(
		cvmx_fpa_get_pko_pool(),
		cvmx_fpa_get_pko_pool_block_size()
		);
	return 0;

}

/**
 * @INTERNAL
 * Setup the PKO for the ports on an interface. The number of
 * queues per port and the priority of each PKO output queue
 * is set here. PKO must be disabled when this function is called.
 *
 * @param interface Interface to setup PKO for
 *
 * @return Zero on success, negative on failure
 *
 * @note This is for PKO1/PKO2, and is not used for PKO3.
 */
int __cvmx_helper_interface_setup_pko(int interface)
{
	/*
	 * Each packet output queue has an associated priority. The
	 * higher the priority, the more often it can send a packet. A
	 * priority of 8 means it can send in all 8 rounds of
	 * contention. We're going to make each queue one less than
	 * the last.  The vector of priorities has been extended to
	 * support CN5xxx CPUs, where up to 16 queues can be
	 * associated to a port.  To keep backward compatibility we
	 * don't change the initial 8 priorities and replicate them in
	 * the second half.  With per-core PKO queues (PKO lockless
	 * operation) all queues have the same priority.
	 */
	/* uint8_t priorities[16] = {8,7,6,5,4,3,2,1,8,7,6,5,4,3,2,1}; */
	uint8_t priorities[16] = {[0 ... 15] = 8 };

	/*
	 * Setup the IPD/PIP and PKO for the ports discovered
	 * above. Here packet classification, tagging and output
	 * priorities are set.
	 */
	int num_ports = cvmx_helper_ports_on_interface(interface);
	while (num_ports--) {
		int ipd_port;

		if (!cvmx_helper_is_port_valid(interface, num_ports))
			continue;

		ipd_port = cvmx_helper_get_ipd_port(interface, num_ports);
		/*
		 * Give the user a chance to override the per queue
		 * priorities.
		 */
		if (cvmx_override_pko_queue_priority)
			cvmx_override_pko_queue_priority(ipd_port, priorities);

		cvmx_pko_config_port(ipd_port,
				     cvmx_pko_get_base_queue(ipd_port),
				     cvmx_pko_get_num_queues(ipd_port),
				     priorities);
		ipd_port++;
	}
	return 0;
	/* NOTE:
	 * Now this function is called for all chips including 68xx,
	 * but on the 68xx it does not enable multiple pko_iports per
	 * eport, while before it was doing 3 pko_iport per eport
	 * buf the reason for that is not clear.
	 */
}

/**
 * wait for the pko queue to drain
 *
 * @param queue a valid pko queue
 * @return count is the length of the queue after calling this
 * function
 */
static int cvmx_helper_wait_pko_queue_drain(int queue)
{
	const int timeout = 5;	/* Wait up to 5 seconds for timeouts */
	int count;
	uint64_t start_cycle, stop_cycle;

	count = cvmx_pko_queue_pend_count(queue);
	if (count < 0)
		return count;

	start_cycle = cvmx_get_cycle();
	stop_cycle = start_cycle + cvmx_clock_get_rate(CVMX_CLOCK_CORE) * timeout;
	while (count > 0 && (cvmx_get_cycle() < stop_cycle)) {
		cvmx_wait(10000);
		count = cvmx_pko_queue_pend_count(queue);
	}

	return count;
}

/**
 * @INTERNAL
 *
 * Drain and wait until all PKO queues are empty.
 */
int __cvmx_helper_pko_drain(void)
{
	int result = 0;

	if (octeon_has_feature(OCTEON_FEATURE_PKND)) {
		int queue, max_queue;

		/* PKO2 */
		max_queue = __cvmx_helper_cfg_pko_max_queue();
		for (queue = 0; queue < max_queue; queue++) {
			if (cvmx_helper_wait_pko_queue_drain(queue)) {
				result = -1;
				return result;
			}
		}
	} else {
		int num_interfaces = cvmx_helper_get_number_of_interfaces();
		int interface, num_ports, index;

		/* PKO1 */
		for (interface = 0; interface < num_interfaces; interface++) {
			num_ports = cvmx_helper_ports_on_interface(interface);
			for (index = 0; index < num_ports; index++) {
				int pko_port;
				int queue;
				int max_queue;
				if (!cvmx_helper_is_port_valid(interface, index))
					continue;
				pko_port = cvmx_helper_get_ipd_port(interface, index);
				queue = cvmx_pko_get_base_queue(pko_port);
				max_queue = queue + cvmx_pko_get_num_queues(pko_port);
				while (queue < max_queue) {
					if (cvmx_helper_wait_pko_queue_drain(queue)) {
						result = -1;
						return result;
					}
					queue++;
				}
			}
		}
	}
	return result;
}
