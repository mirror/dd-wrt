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

/*
 * File version info: $Id$
 *
 * PKOv3 helper file
 */

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <linux/module.h>
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-ilk.h>
#include <asm/octeon/cvmx-pko3.h>
#include <asm/octeon/cvmx-pko3-resources.h>
#include <asm/octeon/cvmx-helper-pko3.h>
#include <asm/octeon/cvmx-helper-cfg.h>
#else
#include "cvmx.h"
#include "cvmx-ilk.h"
#include "cvmx-pko3.h"
#include "cvmx-pko3-resources.h"
#include "cvmx-helper-pko3.h"
#include "cvmx-helper-cfg.h"
#endif

#define IPD2PKO_INDEX_CACHE_Y(ipd_port)	((ipd_port) >> 8)
#define IPD2PKO_INDEX_CACHE_X(ipd_port)	((ipd_port) & 0xff)

static CVMX_SHARED int ipd2pko_index_cache[CVMX_MAX_NODES][16]
    [CVMX_HELPER_CFG_MAX_PORT_PER_IFACE] = {
	[0 ... CVMX_MAX_NODES - 1] = {[0 ... 15] = {
			[0 ... CVMX_HELPER_CFG_MAX_PORT_PER_IFACE - 1] =
				CVMX_HELPER_CFG_INVALID_VALUE }}};

int cvmx_cfg_pko_port_index[CVMX_MAX_NODES][CVMX_HELPER_MAX_IFACE]
    [CVMX_HELPER_CFG_MAX_PORT_PER_IFACE] = {
	[0 ... CVMX_MAX_NODES - 1] = {[0 ... CVMX_HELPER_MAX_IFACE - 1] = {
		[0 ... CVMX_HELPER_CFG_MAX_PORT_PER_IFACE - 1] =
			CVMX_HELPER_CFG_INVALID_VALUE }}};

/* channels are present at L2 queue level by default */
int cvmx_pko_default_channel_level;

extern CVMX_SHARED int64_t cvmx_port_queue_list[CVMX_MAX_NODES][CVMX_PKO_MAX_MACS];

/*
 * @INTERNAL
 * This function gets pko port index for a given interface/port
 * on 78XX.
 *
 * @param node is the node on which this configuration is derived.
 * @param interface is the interface number.
 * @param port is the port number in interface.
 * @return pko_port_index value.
 *
 */
static int __cvmx_cfg_pko_port_index(int node, int interface, int port)
{
	return cvmx_cfg_pko_port_index[node][interface][port];
}

/*
 * This function gets pko port index for ipd_port.
 *
 * @param node is the node on which this mapping is needed.
 * @param ipd_port is the ipd_port for which pko port index is needed.
 * @return pko_port_index value.
 *
 */
int __cvmx_cfg_get_ipd2pko_index(int node, int ipd_port)
{
	int ipd_y, ipd_x;

	ipd_y = IPD2PKO_INDEX_CACHE_Y(ipd_port);
	ipd_x = IPD2PKO_INDEX_CACHE_X(ipd_port);

	return ipd2pko_index_cache[node][ipd_y]
	    [(ipd_port & 0x800) ? ((ipd_x >> 4) & 3) : ipd_x];
}

/*
 * @INTERNAL
 * This function initializes ipd2pko index cache for each interface/port.
 *
 * @param node the node on which this intialization applies.
 * @return 0 on success.
 *
 */
static int __cvmx_cfg_init_ipd2pko_index_cache(int node)
{
	int i, j, n;
	int ipd_y, ipd_x, ipd_port;

	for (i = 0; i < cvmx_helper_get_number_of_interfaces(); i++) {
		n = cvmx_helper_interface_enumerate(i);

		for (j = 0; j < n; j++) {
			ipd_port = cvmx_helper_get_ipd_port(i, j);
			ipd_y = IPD2PKO_INDEX_CACHE_Y(ipd_port);
			ipd_x = IPD2PKO_INDEX_CACHE_X(ipd_port);
			ipd2pko_index_cache[node][ipd_y]
			    [(ipd_port & 0x800) ? ((ipd_x >> 4) & 3) : ipd_x] =
				__cvmx_cfg_pko_port_index(node, i, j);
		}
	}

	return 0;
}

/*
 * @INTERNAL
 * This function initializes default pko queue parameters configuration for
 * interfaces/ports, which are not configured explicitly by application.
 *
 * @param node the node on which this default port and queue config applies.
 * @return 0 on success.
 *
 */
int __cvmx_pko_init_port_config_data_78xx(int node)
{
	int interface, num_interfaces, num_ports, port;
	cvmx_helper_interface_mode_t mode;
	int pko_mac_num;

	/* init queue topology for macs, which are not initialized */
	num_interfaces = cvmx_helper_get_number_of_interfaces();
	for (interface = 0; interface < num_interfaces; interface++) {

		mode = cvmx_helper_interface_get_mode(interface);
		num_ports = cvmx_helper_interface_enumerate(interface);

		if ((mode == CVMX_HELPER_INTERFACE_MODE_NPI) ||
			(mode == CVMX_HELPER_INTERFACE_MODE_LOOP))
			num_ports = 1;

		/* Port queue initialization for ILK interface */
		if (mode == CVMX_HELPER_INTERFACE_MODE_ILK) {
			int num_chans = num_ports;
			#define INIT_MAX_ILK_CHANS 16
			int l2_chan_q[INIT_MAX_ILK_CHANS], l2_rr_quantum[INIT_MAX_ILK_CHANS];
			int i, rr_quantum = 10, res;

			/* ILK has channels all go to the same mac */
			pko_mac_num = __cvmx_pko_get_mac_num(interface, 0); 
			if (pko_mac_num < 0)
				continue;

			/* check if port queue is valid for this mac */
			if (cvmx_port_queue_list[node][pko_mac_num] >= 0) 
				continue;

			if (num_chans > INIT_MAX_ILK_CHANS)
				num_chans = INIT_MAX_ILK_CHANS;
			for (i = 0 ; i < num_chans; i++) {
				l2_chan_q[i] = -1;
				l2_rr_quantum[i] = rr_quantum;
				rr_quantum += 10;
			}
			/* allocate num_chans l2 channel queues */
			res = cvmx_pko_port_queue_setup_rr_childs(node, interface, 0, l2_chan_q, 
								   l2_rr_quantum, num_chans, 0);

			if (res < 0) {
				cvmx_dprintf("Error: Could not setup ILK Channel queues\n");
				continue;
			}

			/* configure channel level */
			cvmx_pko_setup_channel_credit_level(node, cvmx_pko_default_channel_level);

			for (i = 0; i < num_chans; i++) {
				int l3_q, l4_q, l5_q, dq;
				l3_q = cvmx_pko_l2_queue_setup_static_childs(node, interface, i, l2_chan_q[i], -1, 0);
				l4_q = cvmx_pko_l3_queue_setup_static_childs(node, interface, i, l3_q, -1, 0);
				l5_q = cvmx_pko_l4_queue_setup_static_childs(node, interface, i, l4_q, -1, 0);
				dq = cvmx_pko_l5_queue_setup_static_childs(node, interface, i, l5_q, -1, 0); 
				if (dq < 0) {
					cvmx_dprintf("Error: could not add dqs\n");
				}
				/* map channels to l2 queues */
				cvmx_pko_map_channel(node, l2_chan_q[i], cvmx_helper_get_ipd_port(interface, i));
			}

			continue;
		}

		/* intialize queue topology for other interfaces */
		for (port = 0; port < num_ports; port++) {
			pko_mac_num = __cvmx_pko_get_mac_num(interface, port);
			if (pko_mac_num < 0)
				continue;

			/* check if port queue is valid for this mac */
			if (cvmx_port_queue_list[node][pko_mac_num] < 0) {
				/* queue topology is not present for this mac */
				int l2_q, l3_q, l4_q, l5_q, dq;

				/* setup simple passthrough topology for this mac */
				l2_q = cvmx_pko_port_queue_setup_static_childs(node, interface, port, -1, 0);
				l3_q = cvmx_pko_l2_queue_setup_static_childs(node, interface, port, l2_q, -1, 0);
				l4_q = cvmx_pko_l3_queue_setup_static_childs(node, interface, port, l3_q, -1, 0);
				l5_q = cvmx_pko_l4_queue_setup_static_childs(node, interface, port, l4_q, -1, 0);
				dq = cvmx_pko_l5_queue_setup_static_childs(node, interface, port, l5_q, -1, 0);
				if (dq < 0) {
					cvmx_dprintf("Error: could not add dqs\n");
				}
				/* map channel to l2 queue */
				cvmx_pko_map_channel(node, l2_q, cvmx_helper_get_ipd_port(interface, port));
			}
		}
	}

	__cvmx_cfg_init_ipd2pko_index_cache(node);

	return 0;
}

/*
 * @INTERNAL
 * This function gets pko port index for a given interface/port.
 * pko port index is used for mapping ipd_port to pko ports.
 *
 * @param node the node on which this information is needed.
 * @param interface is the interface number.
 * @param port is the port number of the interface type.
 * @return pko_port_index on success or -1 on failure.
 *
 */
int __cvmx_pko_get_port_index(int node, int interface, int port)
{

	int port_index;

	/* check if port index is already allocated */
	port_index = cvmx_cfg_pko_port_index[node][interface][port];

	if (port_index < 0) {
		/* allocate port_index */
		port_index = cvmx_pko_alloc_pko_port_index(node, interface, -1, 1);
		if (port_index < 0) {
			cvmx_dprintf ("Could not allocate port index\n");
			return -1;
		}

		cvmx_cfg_pko_port_index[node][interface][port] = port_index;
	}

	return port_index;
}
