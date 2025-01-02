/***********************license start***************
 * Copyright (c) 2012  Cavium Inc. (support@cavium.com). All rights
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

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <linux/export.h>
#include <asm/octeon/cvmx-range.h>
#include <asm/octeon/cvmx-global-resources.h>
#include <asm/octeon/cvmx-bootmem.h>
#include <asm/octeon/cvmx-helper-cfg.h>
#include <asm/octeon/cvmx-helper.h>
#include <asm/octeon/cvmx-helper-util.h>
#else
#include "cvmx-range.h"
#include "cvmx-global-resources.h"
#include "cvmx-bootmem.h"
#include "cvmx-helper-cfg.h"
#include "cvmx-helper.h"
#include "cvmx-helper-util.h"
#endif

union interface_port
{
	struct
	{
#ifdef __BIG_ENDIAN_BITFIELD
		int port;
		int interface;
#else
		int interface;
		int port;
#endif
	} s;
	uint64_t u64;

};

static int dbg = 0;

static CVMX_SHARED int port_range_init = 0;

int __cvmx_pko_internal_ports_range_init(void)
{
	int rv=0;

	if (port_range_init)
		return 0;
	port_range_init = 1;
	rv = cvmx_create_global_resource_range(CVMX_GR_TAG_PKO_IPORTS, CVMX_HELPER_CFG_MAX_PKO_QUEUES);
	if (rv!=0) {
		cvmx_dprintf("ERROR : Failed to initalize pko internal port range\n");
	}
	return rv;
}


int cvmx_pko_internal_ports_alloc(int xiface, int port, uint64_t count)
{
	int ret_val = -1;
	union interface_port inf_port;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	__cvmx_pko_internal_ports_range_init();
	inf_port.s.interface = xi.interface;
	inf_port.s.port = port;
	ret_val = cvmx_allocate_global_resource_range(CVMX_GR_TAG_PKO_IPORTS, inf_port.u64, count, 1);
	if (dbg)
		cvmx_dprintf("internal port alloc : port=%02d base=%02d count=%02d \n",
			     (int) port, ret_val, (int) count);
	if (ret_val == -1)
		return ret_val;
	cvmx_cfg_port[xi.node][xi.interface][port].ccpp_pko_port_base = ret_val;
	cvmx_cfg_port[xi.node][xi.interface][port].ccpp_pko_num_ports  = count;
	return 0;
}

/*
 * Return the internal ports base
 *
 * @param  port   the port for which the queues are returned
 *
 * @return  0 on success
 *         -1 on failure
 */
int cvmx_pko_internal_ports_free(int xiface, int port)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int ret_val = -1;

	__cvmx_pko_internal_ports_range_init();
	ret_val = cvmx_free_global_resource_range_with_base(CVMX_GR_TAG_PKO_IPORTS,
						    cvmx_cfg_port[xi.node][xi.interface][port].ccpp_pko_port_base,
						    cvmx_cfg_port[xi.node][xi.interface][port].ccpp_pko_num_ports);
	if (ret_val != 0)
		return ret_val;
	cvmx_cfg_port[xi.node][xi.interface][port].ccpp_pko_port_base = CVMX_HELPER_CFG_INVALID_VALUE;
	cvmx_cfg_port[xi.node][xi.interface][port].ccpp_pko_num_ports  =  CVMX_HELPER_CFG_INVALID_VALUE;


	return 0;
}

void cvmx_pko_internal_ports_range_free_all(void)
{
	int interface, port;

	__cvmx_pko_internal_ports_range_init();
	for(interface = 0; interface < CVMX_HELPER_MAX_IFACE; interface++)
		for (port = 0; port < CVMX_HELPER_CFG_MAX_PORT_PER_IFACE;
			     port++) {
			if (cvmx_cfg_port[0][interface][port].ccpp_pko_port_base !=
				    CVMX_HELPER_CFG_INVALID_VALUE)
				cvmx_pko_internal_ports_free(interface, port);
		}
	//cvmx_range_show(pko_internal_ports_range);
}
EXPORT_SYMBOL(cvmx_pko_internal_ports_range_free_all);

void cvmx_pko_internal_ports_range_show(void)
{
	int interface, port;

	__cvmx_pko_internal_ports_range_init();
	cvmx_show_global_resource_range(CVMX_GR_TAG_PKO_IPORTS);
	for(interface = 0; interface < CVMX_HELPER_MAX_IFACE; interface++)
		for (port = 0; port < CVMX_HELPER_CFG_MAX_PORT_PER_IFACE;
				port ++) {
			if (cvmx_cfg_port[0][interface][port].ccpp_pko_port_base !=
				    CVMX_HELPER_CFG_INVALID_VALUE)
				cvmx_dprintf("interface=%d port=%d port_base=%d port_cnt=%d\n",
				     interface, port,
				     (int)cvmx_cfg_port[0][interface][port].ccpp_pko_port_base,
				     (int)cvmx_cfg_port[0][interface][port].ccpp_pko_num_ports);
		}
}
