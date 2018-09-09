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
 * Helper Functions for the Configuration Framework
 *
 * <hr>$Revision: 0 $<hr>
 */
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <linux/export.h>
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-helper.h>
#include <asm/octeon/cvmx-helper-board.h>
#include <asm/octeon/cvmx-helper-util.h>
#include <asm/octeon/cvmx-helper-cfg.h>
#include <asm/octeon/cvmx-helper-ilk.h>
#include <asm/octeon/cvmx-ilk.h>
#include <asm/octeon/cvmx-range.h>
#include <asm/octeon/cvmx-global-resources.h>
#include <asm/octeon/cvmx-pko-internal-ports-range.h>
#else
#include "cvmx.h"
#include "cvmx-bootmem.h"
#include "cvmx-helper.h"
#include "cvmx-helper-board.h"
#include "cvmx-helper-util.h"
#include "cvmx-helper-cfg.h"
#include "cvmx-ilk.h"
#include "cvmx-adma.h"
#include "cvmx-helper-ilk.h"
#include "cvmx-pip.h"
#include "cvmx-range.h"
#include "cvmx-global-resources.h"
#include "cvmx-pko-internal-ports-range.h"
#endif

#if defined(min)
#else
#define min( a, b ) ( ( a ) < ( b ) ) ? ( a ) : ( b )
#endif

/* #define CVMX_HELPER_CFG_DEBUG */

CVMX_SHARED struct cvmx_cfg_port_param cvmx_cfg_port[CVMX_HELPER_MAX_IFACE][CVMX_HELPER_CFG_MAX_PORT_PER_IFACE] =
	{[0 ... CVMX_HELPER_MAX_IFACE - 1] = {[0 ... CVMX_HELPER_CFG_MAX_PORT_PER_IFACE - 1] =
				      	      { CVMX_HELPER_CFG_INVALID_VALUE, CVMX_HELPER_CFG_INVALID_VALUE,
				                CVMX_HELPER_CFG_INVALID_VALUE, CVMX_HELPER_CFG_INVALID_VALUE,
	                                        CVMX_HELPER_CFG_INVALID_VALUE, 0,
	                                        0, 0, 0, false}}};

/*
 * Indexed by the pko_port number
 */
static CVMX_SHARED int __cvmx_cfg_pko_highest_queue;
CVMX_SHARED struct cvmx_cfg_pko_port_param cvmx_pko_queue_table [CVMX_HELPER_CFG_MAX_PKO_PORT] =
{[0 ... CVMX_HELPER_CFG_MAX_PKO_PORT - 1] = {CVMX_HELPER_CFG_INVALID_VALUE,CVMX_HELPER_CFG_INVALID_VALUE}};

CVMX_SHARED cvmx_user_static_pko_queue_config_t __cvmx_pko_queue_static_config;

CVMX_SHARED struct cvmx_cfg_pko_port_map cvmx_cfg_pko_port_map[CVMX_HELPER_CFG_MAX_PKO_PORT] =
	{[0 ... CVMX_HELPER_CFG_MAX_PKO_PORT - 1] = {CVMX_HELPER_CFG_INVALID_VALUE,CVMX_HELPER_CFG_INVALID_VALUE,
						     CVMX_HELPER_CFG_INVALID_VALUE}};

/*
 * This array assists translation from ipd_port to pko_port.
 * The ``16'' is the rounded value for the 3rd 4-bit value of
 * ipd_port, used to differentiate ``interfaces.''
 */
static CVMX_SHARED struct cvmx_cfg_pko_port_pair ipd2pko_port_cache[16]
    [CVMX_HELPER_CFG_MAX_PORT_PER_IFACE] =
	{[0 ... 15] = {[0 ... CVMX_HELPER_CFG_MAX_PORT_PER_IFACE - 1] =
		       { CVMX_HELPER_CFG_INVALID_VALUE,	CVMX_HELPER_CFG_INVALID_VALUE}}};


#ifdef CVMX_USER_DEFINED_HELPER_CONFIG_INIT

static CVMX_SHARED int cvmx_cfg_default_pko_nqueues = 1;

/*
 * A pool for holding the pko_nqueues for the pko_ports assigned to a
 * physical port.
 */
static CVMX_SHARED uint8_t cvmx_cfg_pko_nqueue_pool[CVMX_HELPER_CFG_MAX_PKO_QUEUES] =
	{[0 ... CVMX_HELPER_CFG_MAX_PKO_QUEUES - 1] = 1 };

#endif

/*
 * Options
 *
 * Each array-elem's intial value is also the option's default value.
 */
static CVMX_SHARED uint64_t cvmx_cfg_opts[CVMX_HELPER_CFG_OPT_MAX] = {[0 ... CVMX_HELPER_CFG_OPT_MAX - 1] = 1 };

/*
 * MISC
 */
static CVMX_SHARED int cvmx_cfg_max_pko_engines;	/* # of PKO DMA engines
							   allocated */
int __cvmx_helper_cfg_pknd(int interface, int index)
{
	return cvmx_cfg_port[interface][index].ccpp_pknd;
}

int __cvmx_helper_cfg_bpid(int interface, int index)
{
	return cvmx_cfg_port[interface][index].ccpp_bpid;
}

int __cvmx_helper_cfg_pko_port_base(int interface, int index)
{
	return cvmx_cfg_port[interface][index].ccpp_pko_port_base;
}

int __cvmx_helper_cfg_pko_port_num(int interface, int index)
{
	return cvmx_cfg_port[interface][index].ccpp_pko_num_ports;
}

int __cvmx_helper_cfg_pko_queue_num(int pko_port)
{
	return cvmx_pko_queue_table[pko_port].ccppp_num_queues;
}

int __cvmx_helper_cfg_pko_queue_base(int pko_port)
{
	return cvmx_pko_queue_table[pko_port].ccppp_queue_base;
}

int __cvmx_helper_cfg_pko_max_queue(void)
{
	return __cvmx_cfg_pko_highest_queue;
}

int __cvmx_helper_cfg_pko_max_engine(void)
{
	return cvmx_cfg_max_pko_engines;
}

int cvmx_helper_cfg_opt_set(cvmx_helper_cfg_option_t opt, uint64_t val)
{
	if (opt >= CVMX_HELPER_CFG_OPT_MAX)
		return -1;

	cvmx_cfg_opts[opt] = val;

	return 0;
}

uint64_t cvmx_helper_cfg_opt_get(cvmx_helper_cfg_option_t opt)
{
	if (opt >= CVMX_HELPER_CFG_OPT_MAX)
		return (uint64_t) CVMX_HELPER_CFG_INVALID_VALUE;

	return cvmx_cfg_opts[opt];
}

/*
 * initialize the queue allocation list. the existing static allocation result
 * is used as a starting point to ensure backward compatibility.
 *
 * @return  0 on success
 *         -1 on failure
 */
int cvmx_pko_queue_grp_alloc(uint64_t start, uint64_t end, uint64_t count)
{
	uint64_t port;
	int ret_val;

	for (port = start; port < end; port++) {
		ret_val = cvmx_pko_queue_alloc(port, count);
		if (ret_val == -1)
		{
			cvmx_dprintf("ERROR: Failed to allocate queue for port=%d count=%d \n", (int) port,
				     (int) count);
			return ret_val;
		}
	}
	return 0;
}

int cvmx_pko_queue_init_from_cvmx_config_non_pknd(void)
{
	int ret_val = -1;
	uint64_t count, start, end;

	start = 0;
	end   = __cvmx_pko_queue_static_config.non_pknd.pko_ports_per_interface[0];
	count = __cvmx_pko_queue_static_config.non_pknd. pko_queues_per_port_interface[0];
	cvmx_pko_queue_grp_alloc(start,end,count);

	start = 16;
	end = start + __cvmx_pko_queue_static_config.non_pknd.pko_ports_per_interface[1];
	count = __cvmx_pko_queue_static_config.non_pknd.pko_queues_per_port_interface[1];
	ret_val = cvmx_pko_queue_grp_alloc(start,end,count);
	if (ret_val != 0)
		return -1;

	start = end; end = 36 ; count = __cvmx_pko_queue_static_config.non_pknd.pko_queues_per_port_pci;
	cvmx_pko_queue_grp_alloc(start,end,count);
	if (ret_val != 0)
		return -1;

	start = end; end = 40; count = __cvmx_pko_queue_static_config.non_pknd.pko_queues_per_port_loop;
	cvmx_pko_queue_grp_alloc(start,end,count);
	if (ret_val != 0)
		return -1;

	start = end; end = 42; count = __cvmx_pko_queue_static_config.non_pknd.pko_queues_per_port_srio[0];
	cvmx_pko_queue_grp_alloc(start, end, count);
	if (ret_val != 0)
		return -1;

	start = end; end = 44; count = __cvmx_pko_queue_static_config.non_pknd.pko_queues_per_port_srio[1];
	cvmx_pko_queue_grp_alloc(start, end, count);
	if (ret_val != 0)
		return -1;

	start = end; end = 46; count = __cvmx_pko_queue_static_config.non_pknd.pko_queues_per_port_srio[2];
	cvmx_pko_queue_grp_alloc(start,end, count);
	if (ret_val != 0)
		return -1;

	start = end; end = 48; count = __cvmx_pko_queue_static_config.non_pknd.pko_queues_per_port_srio[3];
	cvmx_pko_queue_grp_alloc(start, end, count);
	if (ret_val != 0)
		return -1;
	return 0;
}

static int CVMX_SHARED queue_range_init = 0;

int init_cvmx_pko_que_range(void)
{
	int rv = 0;

	if (queue_range_init)
		return 0;
	queue_range_init = 1;
	rv = cvmx_create_global_resource_range(CVMX_GR_TAG_PKO_QUEUES, CVMX_HELPER_CFG_MAX_PKO_QUEUES);
	if (rv!=0) {
		cvmx_dprintf("ERROR : Failed to initalize pko queues range\n");
	}
	return rv;
}


/*
 * get a block of "count" queues for "port"
 *
 * @param  port   the port for which the queues are requested
 * @param  count  the number of queues requested
 *
 * @return  0 on success
 *         -1 on failure
 */
int cvmx_pko_queue_alloc(uint64_t port, uint64_t count)
{
    int ret_val = -1;
    int highest_queue;

    init_cvmx_pko_que_range();
    if (port >= CVMX_HELPER_CFG_MAX_PKO_QUEUES) {
	    cvmx_dprintf("ERROR: %s port=%d > %d", __FUNCTION__, (int) port, CVMX_HELPER_CFG_MAX_PKO_QUEUES );
	    return -1;
    }
    ret_val = cvmx_allocate_global_resource_range(CVMX_GR_TAG_PKO_QUEUES, port, count, 1);
    //cvmx_dprintf("allocated pko que : port=%02d base=%02d count=%02d \n", (int) port, ret_val, (int) count);
    if (ret_val == -1)
        return ret_val;
    cvmx_pko_queue_table[port].ccppp_queue_base = ret_val;
    cvmx_pko_queue_table[port].ccppp_num_queues = count;

    highest_queue = ret_val + count - 1;
    if (highest_queue > __cvmx_cfg_pko_highest_queue)
	    __cvmx_cfg_pko_highest_queue = highest_queue;
    return 0;
}

/*
 * return the queues for "port"
 *
 * @param  port   the port for which the queues are returned
 *
 * @return  0 on success
 *         -1 on failure
 */
int cvmx_pko_queue_free(uint64_t port)
{
    int ret_val = -1;

    init_cvmx_pko_que_range();
    if (port >= CVMX_HELPER_CFG_MAX_PKO_QUEUES) {
	    cvmx_dprintf("ERROR: %s port=%d > %d", __FUNCTION__,
			 (int) port, CVMX_HELPER_CFG_MAX_PKO_QUEUES);
	    return -1;
    }

    ret_val = cvmx_free_global_resource_range_with_base(CVMX_GR_TAG_PKO_QUEUES,
							cvmx_pko_queue_table[port].ccppp_queue_base,
							cvmx_pko_queue_table[port].ccppp_num_queues);
    if (ret_val != 0)
        return ret_val;

    cvmx_pko_queue_table[port].ccppp_num_queues = 0;
    cvmx_pko_queue_table[port].ccppp_queue_base = CVMX_HELPER_CFG_INVALID_VALUE;
    ret_val = 0;
    return ret_val;
}

void cvmx_pko_queue_free_all(void)
{
	int i;

	for(i=0; i< CVMX_HELPER_CFG_MAX_PKO_PORT; i++)
		if (cvmx_pko_queue_table[i].ccppp_queue_base != CVMX_HELPER_CFG_INVALID_VALUE)
			cvmx_pko_queue_free(i);
	//cvmx_range_show(pko_queue_range);
}

void cvmx_pko_queue_show()
{
	int i;

	cvmx_show_global_resource_range(CVMX_GR_TAG_PKO_QUEUES);
	for(i=0; i< CVMX_HELPER_CFG_MAX_PKO_PORT; i++)
		if (cvmx_pko_queue_table[i].ccppp_queue_base != CVMX_HELPER_CFG_INVALID_VALUE)
			cvmx_dprintf("port=%d que_base=%d que_num=%d \n", i,
				     (int) cvmx_pko_queue_table[i].ccppp_queue_base,
				     (int) cvmx_pko_queue_table[i].ccppp_num_queues);
}

EXPORT_SYMBOL(__cvmx_helper_cfg_pknd);
EXPORT_SYMBOL(__cvmx_helper_cfg_bpid);
EXPORT_SYMBOL(__cvmx_helper_cfg_pko_port_base);
EXPORT_SYMBOL(__cvmx_helper_cfg_pko_port_num);
EXPORT_SYMBOL(__cvmx_helper_cfg_pko_queue_base);
EXPORT_SYMBOL(__cvmx_helper_cfg_pko_queue_num);
EXPORT_SYMBOL(__cvmx_helper_cfg_pko_max_queue);
EXPORT_SYMBOL(__cvmx_helper_cfg_pko_port_interface);
EXPORT_SYMBOL(__cvmx_helper_cfg_pko_port_index);
EXPORT_SYMBOL(__cvmx_helper_cfg_pko_port_eid);
EXPORT_SYMBOL(__cvmx_helper_cfg_pko_max_engine);
EXPORT_SYMBOL(cvmx_helper_cfg_opt_get);
EXPORT_SYMBOL(cvmx_helper_cfg_opt_set);
EXPORT_SYMBOL(cvmx_helper_cfg_ipd2pko_port_base);
EXPORT_SYMBOL(cvmx_helper_cfg_ipd2pko_port_num);
EXPORT_SYMBOL(cvmx_pko_queue_free_all);


void cvmx_helper_cfg_show_cfg(void)
{
	int i, j;

	for (i = 0; i < cvmx_helper_get_number_of_interfaces(); i++) {
		cvmx_dprintf("cvmx_helper_cfg_show_cfg: interface%d mode %10s nports%4d\n", i,
			     cvmx_helper_interface_mode_to_string(cvmx_helper_interface_get_mode(i)),
			     cvmx_helper_interface_enumerate(i));

		for (j = 0; j < cvmx_helper_interface_enumerate(i); j++) {
			cvmx_dprintf("\tpknd[%i][%d]%d", i, j, __cvmx_helper_cfg_pknd(i, j));
			cvmx_dprintf(" pko_port_base[%i][%d]%d", i, j, __cvmx_helper_cfg_pko_port_base(i, j));
			cvmx_dprintf(" pko_port_num[%i][%d]%d\n", i, j, __cvmx_helper_cfg_pko_port_num(i, j));
		}
	}

	for (i = 0; i < CVMX_HELPER_CFG_MAX_PKO_PORT; i++) {
		if (__cvmx_helper_cfg_pko_queue_base(i) != CVMX_HELPER_CFG_INVALID_VALUE) {
			cvmx_dprintf("cvmx_helper_cfg_show_cfg: pko_port%d qbase%d nqueues%d "
				     "interface%d index%d\n", i,
				     __cvmx_helper_cfg_pko_queue_base(i),
				     __cvmx_helper_cfg_pko_queue_num(i), __cvmx_helper_cfg_pko_port_interface(i),
				     __cvmx_helper_cfg_pko_port_index(i));
		}
	}
}

/*
 * initialize cvmx_cfg_pko_port_map
 */
void cvmx_helper_cfg_init_pko_port_map(void)
{
	int i, j, k;
	int pko_eid;
	int pko_port_base, pko_port_max;
	cvmx_helper_interface_mode_t mode;

	/*
	 * one pko_eid is allocated to each port except for ILK, NPI, and
	 * LOOP. Each of the three has one eid.
	 */
	pko_eid = 0;
	for (i = 0; i < cvmx_helper_get_number_of_interfaces(); i++) {
		mode = cvmx_helper_interface_get_mode(i);
		for (j = 0; j < cvmx_helper_interface_enumerate(i); j++) {
			pko_port_base = cvmx_cfg_port[i][j].ccpp_pko_port_base;
			pko_port_max = pko_port_base + cvmx_cfg_port[i][j].ccpp_pko_num_ports;
			cvmx_helper_cfg_assert(pko_port_base != CVMX_HELPER_CFG_INVALID_VALUE);
			cvmx_helper_cfg_assert(pko_port_max >= pko_port_base);
			for (k = pko_port_base; k < pko_port_max; k++) {
				cvmx_cfg_pko_port_map[k].ccppl_interface = i;
				cvmx_cfg_pko_port_map[k].ccppl_index = j;
				cvmx_cfg_pko_port_map[k].ccppl_eid = pko_eid;
			}

			if (!(mode == CVMX_HELPER_INTERFACE_MODE_NPI || mode == CVMX_HELPER_INTERFACE_MODE_LOOP
			      || mode == CVMX_HELPER_INTERFACE_MODE_ILK))
					pko_eid++;
		}

		if (mode == CVMX_HELPER_INTERFACE_MODE_NPI || mode == CVMX_HELPER_INTERFACE_MODE_LOOP ||
		    mode == CVMX_HELPER_INTERFACE_MODE_ILK)
			pko_eid++;
	}

	/*
	 * Legal pko_eids [0, 0x13] should not be exhausted.
	 */
	cvmx_helper_cfg_assert(pko_eid <= 0x14);

	cvmx_cfg_max_pko_engines = pko_eid;
}

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
void cvmx_helper_cfg_set_jabber_and_frame_max()
{
	int interface, port;

	/*Set the frame max size and jabber size to 65535. */
	for (interface = 0; interface < cvmx_helper_get_number_of_interfaces(); interface++) {
		/* Set the frame max size and jabber size to 65535, as the defaults
		   are too small. */
		cvmx_helper_interface_mode_t imode = cvmx_helper_interface_get_mode(interface);
		int num_ports = cvmx_helper_ports_on_interface(interface);

		switch (imode) {
		case CVMX_HELPER_INTERFACE_MODE_SGMII:
		case CVMX_HELPER_INTERFACE_MODE_QSGMII:
		case CVMX_HELPER_INTERFACE_MODE_XAUI:
		case CVMX_HELPER_INTERFACE_MODE_RXAUI:
			for (port = 0; port < num_ports; port++)
				cvmx_write_csr(CVMX_GMXX_RXX_JABBER(port, interface), 65535);
			/* Set max and min value for frame check */
			cvmx_pip_set_frame_check(interface, -1);
			break;

		case CVMX_HELPER_INTERFACE_MODE_RGMII:
		case CVMX_HELPER_INTERFACE_MODE_GMII:
			/* Set max and min value for frame check */
			cvmx_pip_set_frame_check(interface, -1);
			for (port = 0; port < num_ports; port++) {
				if (!OCTEON_IS_MODEL(OCTEON_CN50XX))
					cvmx_write_csr(CVMX_GMXX_RXX_FRM_MAX(port, interface), 65535);
				cvmx_write_csr(CVMX_GMXX_RXX_JABBER(port, interface), 65535);
			}
			break;
		case CVMX_HELPER_INTERFACE_MODE_ILK:
			/* Set max and min value for frame check */
			cvmx_pip_set_frame_check(interface, -1);
			for (port = 0; port < num_ports; port++) {
				int ipd_port = cvmx_helper_get_ipd_port(interface, port);
				cvmx_ilk_enable_la_header(ipd_port, 0);
			}
			break;
		case CVMX_HELPER_INTERFACE_MODE_SRIO:
			/* Set max and min value for frame check */
			cvmx_pip_set_frame_check(interface, -1);
			break;
		case CVMX_HELPER_INTERFACE_MODE_AGL:
			/* Set max and min value for frame check */
			cvmx_pip_set_frame_check(interface, -1);
			cvmx_write_csr(CVMX_AGL_GMX_RXX_FRM_MAX(0), 65535);
			cvmx_write_csr(CVMX_AGL_GMX_RXX_JABBER(0), 65535);
			break;
		default:
			break;
		}
	}
}

void cvmx_helper_cfg_store_short_packets_in_wqe()
{
	int interface, port;

	if (!OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		for (interface = 0; interface < cvmx_helper_get_number_of_interfaces(); interface++) {
			int num_ports = cvmx_helper_ports_on_interface(interface);
			/* Enable storing short packets only in the WQE */
			for (port = 0; port < num_ports; port++) {
				cvmx_pip_port_cfg_t port_cfg;
				int pknd = port;
				if (octeon_has_feature(OCTEON_FEATURE_PKND))
					pknd = cvmx_helper_get_pknd(interface, port);
				else
					pknd = cvmx_helper_get_ipd_port(interface, port);
				port_cfg.u64 = cvmx_read_csr(CVMX_PIP_PRT_CFGX(pknd));
				port_cfg.s.dyn_rs = 1;
				cvmx_write_csr(CVMX_PIP_PRT_CFGX(pknd), port_cfg.u64);
			}
		}
	}
}
#endif /* CVMX_BUILD_FOR_LINUX_KERNEL*/


int __cvmx_helper_cfg_pko_port_interface(int pko_port)
{
	return cvmx_cfg_pko_port_map[pko_port].ccppl_interface;
}

int __cvmx_helper_cfg_pko_port_index(int pko_port)
{
	return cvmx_cfg_pko_port_map[pko_port].ccppl_index;
}

int __cvmx_helper_cfg_pko_port_eid(int pko_port)
{
	return cvmx_cfg_pko_port_map[pko_port].ccppl_eid;
}

#define IPD2PKO_CACHE_Y(ipd_port)	(ipd_port) >> 8
#define IPD2PKO_CACHE_X(ipd_port)	(ipd_port) & 0xff

/*
 * ipd_port to pko_port translation cache
 */
int __cvmx_helper_cfg_init_ipd2pko_cache(void)
{
	int i, j, n;
	int ipd_y, ipd_x, ipd_port;

	for (i = 0; i < cvmx_helper_get_number_of_interfaces(); i++) {
		n = cvmx_helper_interface_enumerate(i);

		for (j = 0; j < n; j++) {
			ipd_port = cvmx_helper_get_ipd_port(i, j);
			ipd_y = IPD2PKO_CACHE_Y(ipd_port);
			ipd_x = IPD2PKO_CACHE_X(ipd_port);
			ipd2pko_port_cache[ipd_y]
			    [(ipd_port & 0x800) ? ((ipd_x >> 4) & 3) : ipd_x] = (struct cvmx_cfg_pko_port_pair) {
			__cvmx_helper_cfg_pko_port_base(i, j), __cvmx_helper_cfg_pko_port_num(i, j)};
		}
	}

	return 0;
}

int cvmx_helper_cfg_ipd2pko_port_base(int ipd_port)
{
	int ipd_y, ipd_x;

	ipd_y = IPD2PKO_CACHE_Y(ipd_port);
	ipd_x = IPD2PKO_CACHE_X(ipd_port);

	return ipd2pko_port_cache[ipd_y]
	    [(ipd_port & 0x800) ? ((ipd_x >> 4) & 3) : ipd_x].ccppp_base_port;
}

int cvmx_helper_cfg_ipd2pko_port_num(int ipd_port)
{
	int ipd_y, ipd_x;

	ipd_y = IPD2PKO_CACHE_Y(ipd_port);
	ipd_x = IPD2PKO_CACHE_X(ipd_port);

	return ipd2pko_port_cache[ipd_y]
	    [(ipd_port & 0x800) ? ((ipd_x >> 4) & 3) : ipd_x].ccppp_nports;
}

/**
 * Return the number of queues to be assigned to this pko_port
 *
 * @param pko_port
 * @return the number of queues for this pko_port
 *
 * Note: This function exists for backward compatibility.
 * CVMX_PKO_QUEUES_PER_PORT_XXXX defines no of queues per HW port.
 * pko_port is equivalent in pre-o68 SDK.
 */
int cvmx_helper_cfg_dft_nqueues(int pko_port)
{
	cvmx_helper_interface_mode_t mode;
	int interface;
	int n;

	n = 1;
	interface = __cvmx_helper_cfg_pko_port_interface(pko_port);
	if ((interface >= 0) && (interface <=4) )  {
		return __cvmx_pko_queue_static_config.pknd.pko_queues_per_port_interface[interface];
	}

	mode = cvmx_helper_interface_get_mode(interface);
	if (mode == CVMX_HELPER_INTERFACE_MODE_LOOP) {
		return __cvmx_pko_queue_static_config.pknd.pko_queues_per_port_loop;
	}
	if (mode == CVMX_HELPER_INTERFACE_MODE_NPI) {
		return __cvmx_pko_queue_static_config.pknd.pko_queues_per_port_pci;
	}
	return n;
}

static int cvmx_helper_cfg_init_pko_iports_and_queues_using_static_config(void)
{
	int pko_port_base = 0 ;
	int cvmx_cfg_default_pko_nports = 1;
	int i, j, n, k;
	int rv = 0;

	/* When not using config file, each port is assigned one internal pko port*/
	for (i = 0; i < cvmx_helper_get_number_of_interfaces(); i++) {
		n = cvmx_helper_interface_enumerate(i);
		for (j = 0; j < n; j++) {
			cvmx_cfg_port[i][j].ccpp_pko_port_base = pko_port_base;
			cvmx_cfg_port[i][j].ccpp_pko_num_ports = cvmx_cfg_default_pko_nports;
			/* Initalize interface early here so that the
			   cvmx_helper_cfg_dft_nqueues() below
			   can get the interface number corresponding to the pko port */
			for (k = pko_port_base; k < pko_port_base + cvmx_cfg_default_pko_nports; k++) {
				cvmx_cfg_pko_port_map[k].ccppl_interface = i;
			}
			pko_port_base += cvmx_cfg_default_pko_nports;
		}
	}
	cvmx_helper_cfg_assert(pko_port_base <= CVMX_HELPER_CFG_MAX_PKO_PORT);


	/* Assigning queues per pko */
	for (i = 0; i < pko_port_base; i++) {
		int base;
		n = cvmx_helper_cfg_dft_nqueues(i);
		base = cvmx_pko_queue_alloc(i, n);
		if (base == -1)  {
			cvmx_dprintf("ERROR: failed to alloc queues=%d for pko port=%d\n", n, i);
			rv = -1;
		}
	}
	return rv;
}

/**
 * Returns if port is valid for a given interface
 *
 * @param interface  interface to check
 * @param index      port index in the interface
 *
 * @return status of the port present or not.
 */
int cvmx_helper_is_port_valid(int interface, int index)
{
	return  cvmx_cfg_port[interface][index].valid;
}
EXPORT_SYMBOL(cvmx_helper_is_port_valid);

void cvmx_helper_set_port_valid(int interface, int index, bool valid)
{
	cvmx_cfg_port[interface][index].valid = valid;
}
EXPORT_SYMBOL(cvmx_helper_set_port_valid);

void cvmx_helper_set_mac_phy_mode(int interface, int index, bool valid)
{
	cvmx_cfg_port[interface][index].sgmii_phy_mode = valid;
}
EXPORT_SYMBOL(cvmx_helper_set_mac_phy_mode);

bool cvmx_helper_get_mac_phy_mode(int interface, int index)
{
	return cvmx_cfg_port[interface][index].sgmii_phy_mode;
}
EXPORT_SYMBOL(cvmx_helper_get_mac_phy_mode);

void cvmx_helper_set_1000x_mode(int interface, int index, bool valid)
{
	cvmx_cfg_port[interface][index].sgmii_1000x_mode = valid;
}
EXPORT_SYMBOL(cvmx_helper_set_1000x_mode);

bool cvmx_helper_get_1000x_mode(int interface, int index)
{
	return cvmx_cfg_port[interface][index].sgmii_1000x_mode;
}
EXPORT_SYMBOL(cvmx_helper_get_1000x_mode);

void cvmx_helper_set_agl_rx_clock_delay_bypass(int interface, int index,
					       bool valid)
{
	cvmx_cfg_port[interface][index].agl_rx_clk_delay_bypass = valid;
}
EXPORT_SYMBOL(cvmx_helper_set_agl_rx_clock_delay_bypass);

bool cvmx_helper_get_agl_rx_clock_delay_bypass(int interface, int index)
{
	return cvmx_cfg_port[interface][index].agl_rx_clk_delay_bypass;
}
EXPORT_SYMBOL(cvmx_helper_get_agl_rx_clock_delay_bypass);

void cvmx_helper_set_agl_rx_clock_skew(int interface, int index, uint8_t value)
{
	cvmx_cfg_port[interface][index].agl_rx_clk_skew = value;
}
EXPORT_SYMBOL(cvmx_helper_set_agl_rx_clock_skew);

uint8_t cvmx_helper_get_agl_rx_clock_skew(int interface, int index)
{
	return cvmx_cfg_port[interface][index].agl_rx_clk_skew;
}
EXPORT_SYMBOL(cvmx_helper_get_agl_rx_clock_skew);

void cvmx_helper_set_port_force_link_up(int interface, int index, bool value)
{
	cvmx_cfg_port[interface][index].force_link_up = value;
}
EXPORT_SYMBOL(cvmx_helper_set_port_force_link_up);

bool cvmx_helper_get_port_force_link_up(int interface, int index)
{
	return cvmx_cfg_port[interface][index].force_link_up;
}
EXPORT_SYMBOL(cvmx_helper_get_port_force_link_up);

int __cvmx_helper_init_port_valid(void)
{
#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	int i, j, n;
	bool valid;

	for (i = 0; i < cvmx_helper_get_number_of_interfaces(); i++) {
		static void *fdt_addr = 0;

		if (fdt_addr == 0)
			fdt_addr = __cvmx_phys_addr_to_ptr(cvmx_sysinfo_get()->fdt_addr,
							   (128*1024));
		n = cvmx_helper_interface_enumerate(i);
		for (j = 0; j < n; j++) {
			int ipd_port = cvmx_helper_get_ipd_port(i, j);
			valid = (__cvmx_helper_board_get_port_from_dt(fdt_addr,
								      ipd_port) == 1);
			cvmx_helper_set_port_valid(i, j, valid);
		}
	}
#endif
	return 0;
}

typedef int (*cvmx_import_config_t)(void);
cvmx_import_config_t cvmx_import_app_config = NULL;

int __cvmx_helper_init_port_config_data_local(void)
{
	int rv = 0;
	int dbg = 0;

	if (octeon_has_feature(OCTEON_FEATURE_PKND))
	{
		if (cvmx_import_app_config) {
			rv = (*cvmx_import_app_config)();
			if (rv != 0) {
				cvmx_dprintf("failed to import config\n");
				return -1;
			}
		}

		cvmx_helper_cfg_init_pko_port_map();
		__cvmx_helper_cfg_init_ipd2pko_cache();
	} else {
		if (cvmx_import_app_config) {
			rv = (*cvmx_import_app_config)();
			if (rv != 0) {
				cvmx_dprintf("failed to import config\n");
				return -1;
			}
		}
	}
	if (dbg) {
		cvmx_helper_cfg_show_cfg();
		cvmx_pko_queue_show();
	}
	return rv;
}

extern int is_app_config_string_set(void);

int cvmx_pko_alloc_iport_and_queues(int interface, int port, int port_cnt, int queue_cnt)
{
	int rv,p, port_start, cnt;

	if (octeon_has_feature(OCTEON_FEATURE_PKND)) {
		rv = cvmx_pko_internal_ports_alloc(interface, port, port_cnt);
		if (rv < 0)  {
			cvmx_dprintf("ERROR: failed to allocate internal ports for"
				     "interface=%d port=%d cnt=%d\n", interface, port,
				     port_cnt);
			return -1;
		}
		port_start =  __cvmx_helper_cfg_pko_port_base(interface, port);
		cnt  = __cvmx_helper_cfg_pko_port_num(interface, port);
	} else {
		port_start = cvmx_helper_get_ipd_port(interface, port);
		cnt = 1;
	}

	for (p = port_start; p < port_start + cnt; p++) {
		rv = cvmx_pko_queue_alloc(p, queue_cnt);
		if (rv < 0)  {
			cvmx_dprintf("ERROR: failed to allocate queues for port=%d"
				     "cnt=%d\n", p, queue_cnt);
			return -1;
		}
	}
	return 0;
}
EXPORT_SYMBOL(cvmx_pko_alloc_iport_and_queues);

int __cvmx_helper_init_port_config_data(void)
{

	int rv = 0;
	int i, j, n;
	int dbg = 0;
	int static_config_set = 0;
	int num_interfaces, interface;

#if  ( !defined(CVMX_BUILD_FOR_LINUX_KERNEL)  && !defined(__U_BOOT__))
	if (!(is_app_config_string_set()))
		static_config_set = 1;
#endif
#ifdef __U_BOOT__
	static_config_set = 1;
#endif

	if (octeon_has_feature(OCTEON_FEATURE_PKND))
	{
		int pknd = 0, bpid = 0;
		if (static_config_set) {
			cvmx_helper_cfg_init_pko_iports_and_queues_using_static_config();
		}
		/* Initialize pknd and bpid */
		for (i = 0; i < cvmx_helper_get_number_of_interfaces(); i++) {
			n = cvmx_helper_interface_enumerate(i);
			for (j = 0; j < n; j++) {
				cvmx_cfg_port[i][j].ccpp_pknd = pknd++;
				cvmx_cfg_port[i][j].ccpp_bpid = bpid++;
			}
		}

		cvmx_helper_cfg_assert(pknd <= CVMX_HELPER_CFG_MAX_PIP_PKND);
		cvmx_helper_cfg_assert(bpid <= CVMX_HELPER_CFG_MAX_PIP_BPID);

	} else {
		if (static_config_set) {
			cvmx_pko_queue_init_from_cvmx_config_non_pknd();
		}
	}

	/* init ports, queues which are not initialized */
	num_interfaces = cvmx_helper_get_number_of_interfaces();
	for (interface = 0; interface < num_interfaces; interface++) {
		int num_ports = __cvmx_helper_early_ports_on_interface(interface);
		int port, port_base, queue;

		for (port = 0; port < num_ports; port++) {
			bool init_req = false;

			if (octeon_has_feature(OCTEON_FEATURE_PKND)) {
				port_base = __cvmx_helper_cfg_pko_port_base(interface, port);
				if (port_base == CVMX_HELPER_CFG_INVALID_VALUE)
					init_req = true;
			} else {
				port_base = cvmx_helper_get_ipd_port(interface, port);
				queue = __cvmx_helper_cfg_pko_queue_base(port_base);
				if (queue == CVMX_HELPER_CFG_INVALID_VALUE)
					init_req = true;
			}

			if (init_req) {
				rv = cvmx_pko_alloc_iport_and_queues(interface,
								     port, 1, 1);
				if (rv < 0) {
					cvmx_dprintf("cvm_pko_alloc_iport_and_queues failed.\n");
					return rv;
				}
			}
		}
	}

	if (octeon_has_feature(OCTEON_FEATURE_PKND)) {
		cvmx_helper_cfg_init_pko_port_map();
		__cvmx_helper_cfg_init_ipd2pko_cache();
	}

	if (dbg) {
		cvmx_helper_cfg_show_cfg();
		cvmx_pko_queue_show();
	}
	return rv;
}
