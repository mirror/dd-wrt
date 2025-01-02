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
 * File version info: $Rev$
 *
 * PKOv3 helper file
 */
/* #define	__SUPPORT_PFC_ON_XAUI */

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <linux/module.h>
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-clock.h>
#include <asm/octeon/cvmx-ilk.h>
#include <asm/octeon/cvmx-fpa3.h>
#include <asm/octeon/cvmx-pko3.h>
#include <asm/octeon/cvmx-pko3-resources.h>
#include <asm/octeon/cvmx-helper.h>
#include <asm/octeon/cvmx-helper-pko.h>
#include <asm/octeon/cvmx-helper-pko3.h>
#include <asm/octeon/cvmx-helper-bgx.h>
#include <asm/octeon/cvmx-helper-cfg.h>
#else
#include "cvmx.h"
#include "cvmx-ilk.h"
#include "cvmx-fpa3.h"
#include "cvmx-pko3.h"
#include "cvmx-pko3-resources.h"
#include "cvmx-helper.h"
#include "cvmx-helper-pko.h"
#include "cvmx-helper-pko3.h"
#include "cvmx-helper-bgx.h"
#include "cvmx-helper-cfg.h"
#endif

/* channels are present at L2 queue level by default */
static const enum cvmx_pko3_level_e
cvmx_pko_default_channel_level = CVMX_PKO_L2_QUEUES;

static const int debug = 0;

static CVMX_SHARED int __pko_pkt_budget, __pko_pkt_quota;

/* These global variables are relevant for boot CPU only */
static CVMX_SHARED cvmx_fpa3_gaura_t __cvmx_pko3_aura[CVMX_MAX_NODES];

/* This constant can not be modified, defined here for clarity only */
#define CVMX_PKO3_POOL_BUFFER_SIZE 4096 /* 78XX PKO requires 4KB */

/**
 * @INTERNAL
 *
 * Build an owner tag based on interface/port
 */
static int __cvmx_helper_pko3_res_owner(int ipd_port)
{
	int res_owner;
	const int res_owner_pfix = 0x19d0 << 14;

	ipd_port &= 0x3fff;	/* 12-bit for local CHAN_E value + node */

	res_owner = res_owner_pfix | ipd_port;

	return res_owner;
}

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
/**
 * Configure an AURA/POOL designated for PKO internal use.
 *
 * This pool is used for (a) memory buffers that store PKO descriptor queues,
 * (b) buffers for use with PKO_SEND_JUMP_S sub-header.
 *
 * The buffers of type (a) are never accessed by software, and their number
 * should be at least equal to 4 times the number of descriptor queues
 * in use.
 *
 * Type (b) buffers are consumed by PKO3 command-composition code,
 * and are released by the hardware upon completion of transmission.
 *
 * @returns -1 if the pool could not be established or 12-bit AURA
 * that includes the node number for use in PKO3 intialization call.
 *
 * NOTE: Linux kernel should pass its own aura to PKO3 initialization
 * function so that the buffers can be mapped into kernel space
 * for when software needs to adccess their contents.
 *
 */
static int __cvmx_pko3_config_memory(unsigned node)
{
	cvmx_fpa3_gaura_t aura;
	int aura_num;
	unsigned buf_count;
	bool small_mem;
	int i, num_intf = 0;
	const unsigned pkt_per_buf =
		(CVMX_PKO3_POOL_BUFFER_SIZE / sizeof(uint64_t) / 16);
	const unsigned base_buf_count = 1024*4;

	/* Simulator has limited memory, but uses one interface at a time */
	small_mem = cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_SIM;

	/* Count the number of live interfaces */
	for (i = 0; i < cvmx_helper_get_number_of_interfaces(); i++) {
		int xiface = cvmx_helper_node_interface_to_xiface(node, i);
		if ( CVMX_HELPER_INTERFACE_MODE_DISABLED !=
		    cvmx_helper_interface_get_mode(xiface))
			num_intf ++;
	}

#if defined(__U_BOOT__)
	buf_count = 1024;
	__pko_pkt_quota = buf_count * pkt_per_buf;
	__pko_pkt_budget = __pko_pkt_quota *  num_intf;
	(void) small_mem; (void) base_buf_count;
#else /* !U_BOOT */
	if (small_mem) {
		buf_count = base_buf_count + 1024;
		__pko_pkt_quota = buf_count * pkt_per_buf;
		__pko_pkt_budget = __pko_pkt_quota *  num_intf;
	} else {
		buf_count = 1024 / pkt_per_buf;

		/* Make room for 2 milliseconds at maximum packet rate */
		buf_count += 100 * 2000 / pkt_per_buf;

		/* Save per-interface queue depth quota */
		__pko_pkt_quota = buf_count * pkt_per_buf;

		/* Multiply by interface count */
		buf_count *= num_intf;

		/* Save total packet budget */
		__pko_pkt_budget = buf_count * pkt_per_buf;

		/* Add 4 more buffers per DQ */
		buf_count += base_buf_count;
	}
#endif /* !U_BOOT */

	if (debug)
		cvmx_dprintf("%s: Creating AURA with %u buffers for up to %d total packets,"
			" %d packets per interface\n",
			__func__, buf_count, __pko_pkt_budget, __pko_pkt_quota);

	aura = cvmx_fpa3_setup_aura_and_pool(node, -1,
		"PKO3 AURA", NULL,
		CVMX_PKO3_POOL_BUFFER_SIZE, buf_count);

	if (!__cvmx_fpa3_aura_valid(aura)) {
		cvmx_printf("ERROR: %s AURA create failed\n", __func__);
		return -1;
	}

	aura_num = aura.node << 10 | aura.laura;

	/* Store handle for destruction */
	__cvmx_pko3_aura[node] = aura;

	return aura_num;
}
#endif /* !CVMX_BUILD_FOR_LINUX_KERNEL */


/** Initialize a channelized port
 * This is intended for LOOP, ILK and NPI interfaces which have one MAC
 * per interface and need a channel per subinterface (e.g. ring).
 * Each channel then may have 'num_queues' descriptor queues
 * attached to it, which can also be prioritized or fair.
 */
static int __cvmx_pko3_config_chan_interface( int xiface, unsigned num_chans,
	uint8_t num_queues, bool prioritized)
{
	int l1_q_num;
	int l2_q_base;
	enum cvmx_pko3_level_e level;
	int res;
	int parent_q, child_q;
	unsigned chan, dq;
	int pko_mac_num;
	uint16_t ipd_port;
	int res_owner, prio;
	unsigned i;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	unsigned node = xi.node;
	char b1[12];

	if (num_queues == 0)
		num_queues = 1;
	if ((cvmx_pko3_num_level_queues(CVMX_PKO_DESCR_QUEUES) / num_chans) < 3)
		num_queues = 1;

	if (prioritized && num_queues > 1)
		prio = num_queues;
	else
		prio = -1;

	if(debug)
		cvmx_dprintf("%s: configuring xiface %u:%u with "
				"%u chans %u queues each\n",
				__FUNCTION__, xi.node, xi.interface,
				num_chans, num_queues);

	/* all channels all go to the same mac */
	pko_mac_num = __cvmx_pko3_get_mac_num(xiface, 0);
	if (pko_mac_num < 0) {
                cvmx_printf ("ERROR: %s: Invalid interface\n", __func__);
		return -1;
	}

	/* Resources of all channels on this port have common owner */
	ipd_port = cvmx_helper_get_ipd_port(xiface, 0);

	/* Build an identifiable owner */
	res_owner = __cvmx_helper_pko3_res_owner(ipd_port);

	/* Start configuration at L1/PQ */
	level = CVMX_PKO_PORT_QUEUES;

	/* Reserve port queue to make sure the MAC is not already configured */
        l1_q_num = cvmx_pko_alloc_queues(node, level, res_owner, -1, 1);

	if (l1_q_num < 0) {
                cvmx_printf ("ERROR: %s: Reserving L1 PQ\n", __func__);
		return -1;
	}

	res = cvmx_pko3_pq_config(node, pko_mac_num, l1_q_num);
	if (res < 0) {
                cvmx_printf ("ERROR: %s: Configuring L1 PQ\n", __func__);
		return -1;
	}

	/* next queue level = L2/SQ */
	level = __cvmx_pko3_sq_lvl_next(level);

        /* allocate level 2 queues, one per channel */
        l2_q_base = cvmx_pko_alloc_queues(node, level, res_owner,
					 -1, num_chans);
        if (l2_q_base < 0) {
                cvmx_printf ("ERROR: %s: allocation L2 SQ\n", __func__);
                return -1;
        }

	/* Configre <num_chans> L2 children for PQ, non-prioritized */
	res = cvmx_pko3_sq_config_children(node, level,
			l1_q_num, l2_q_base, num_chans, -1);

	if (res < 0) {
		cvmx_printf("ERROR: %s: Failed channel queues\n", __func__);
		return -1;
	}

	/* map channels to l2 queues */
	for (chan = 0; chan < num_chans; chan++) {
		ipd_port = cvmx_helper_get_ipd_port(xiface, chan);
		cvmx_pko3_map_channel(node,
			l1_q_num, l2_q_base + chan, ipd_port);
	}

	/* next queue level = L3/SQ */
	level = __cvmx_pko3_sq_lvl_next(level);
	parent_q = l2_q_base;

	do {
		child_q = cvmx_pko_alloc_queues(node, level,
			res_owner, -1, num_chans);

		if (child_q < 0) {
			cvmx_printf ("ERROR: %s: allocating %s\n",
				__func__,
				__cvmx_pko3_sq_str(b1, level, child_q));
			return -1;
		}

		for (i = 0; i < num_chans; i++) {
			res = cvmx_pko3_sq_config_children(node, level,
				parent_q + i, child_q + i, 1, 1);

			if (res < 0) {
				cvmx_printf ("ERROR: %s: configuring %s\n",
					__func__,
					__cvmx_pko3_sq_str(b1, level, child_q));
				return -1;
			}

		} /* for i */

		parent_q = child_q;
		level = __cvmx_pko3_sq_lvl_next(level);

	/* Terminate loop on DQ level, it has special handling */
	} while (level != CVMX_PKO_DESCR_QUEUES &&
		level != CVMX_PKO_LEVEL_INVAL);

	if (level != CVMX_PKO_DESCR_QUEUES) {
		cvmx_printf("ERROR: %s: level sequence error\n", __func__);
		return -1;
	}

	/* Configure DQs, num_dqs per chan */
	for (chan = 0; chan < num_chans; chan++) {

		res = cvmx_pko_alloc_queues(node, level,
			res_owner, -1, num_queues);

		if(res < 0) goto _fail;
		dq = res;

		if(OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_0) && (dq & 7))
			cvmx_dprintf("WARNING: %s: DQ# %u not integral of 8\n",
				__func__, dq);

		res = cvmx_pko3_sq_config_children(node, level, parent_q + chan,
			dq, num_queues, prio);
		if(res < 0) goto _fail;

		/* register DQ range with the translation table */
		res = __cvmx_pko3_ipd_dq_register(xiface,
				chan, dq, num_queues);
		if(res < 0) goto _fail;
	}

	return 0;
  _fail:
	cvmx_dprintf("ERROR: %s: configuring queues for xiface %u:%u chan %u\n",
				__FUNCTION__, xi.node, xi.interface, i);
	return -1;
}

/** Initialize a single Ethernet port with PFC-style channels
 *
 * One interface can contain multiple ports, this function is per-port
 * Here, a physical port is allocated 8 logical channel, one per VLAN
 * tag priority, one DQ is assigned to each channel, and all 8 DQs
 * are registered for that IPD port.
 * Note that the DQs are arrange such that the Ethernet QoS/PCP field
 * can be used as an offset to the value returned by cvmx_pko_base_queue_get().
 *
 * For HighGig2 mode, 16 channels may be desired, instead of 8,
 * but this function does not support that.
 */
static int __cvmx_pko3_config_pfc_interface(int xiface, unsigned port)
{
	enum cvmx_pko3_level_e level;
	int pko_mac_num;
	int l1_q_num, l2_q_base;
	int child_q, parent_q;
	int dq_base;
	int res;
	const unsigned num_chans = 8;
	cvmx_xiface_t xi = cvmx_helper_xiface_to_node_interface(xiface);
	unsigned node = xi.node;
	uint16_t ipd_port;
	int res_owner;
	char b1[12];
	unsigned i;

	if (debug)
		cvmx_dprintf("%s: configuring xiface %u:%u port %u "
			"with %u PFC channels\n",
			__func__, node, xi.interface, port, num_chans);

	/* Get MAC number for the iface/port */
	pko_mac_num = __cvmx_pko3_get_mac_num(xiface, port);
	if (pko_mac_num < 0) {
		cvmx_printf ("ERROR: %s: Invalid interface\n", __func__);
		return -1;
	}

	ipd_port = cvmx_helper_get_ipd_port(xiface, port);

	/* Build an identifiable owner identifier */
	res_owner = __cvmx_helper_pko3_res_owner(ipd_port);

	level = CVMX_PKO_PORT_QUEUES;

	/* Allocate port queue to make sure the MAC is not already configured */
	l1_q_num = cvmx_pko_alloc_queues(node, level, res_owner, -1, 1);

	if (l1_q_num < 0) {
		cvmx_printf ("ERROR: %s: allocation L1 PQ\n", __func__);
		return -1;
	}

	res = cvmx_pko3_pq_config(xi.node, pko_mac_num, l1_q_num);
	if (res < 0) {
                cvmx_printf ("ERROR: %s: Configuring %s\n", __func__,
				__cvmx_pko3_sq_str(b1, level, l1_q_num));
		return -1;
	}

	/* Determine the next queue level */
	level = __cvmx_pko3_sq_lvl_next(level);

	/* Allocate 'num_chans' L2 queues, one per channel */
	l2_q_base = cvmx_pko_alloc_queues(node, level, res_owner,
		-1, num_chans);
	if (l2_q_base < 0) {
		cvmx_printf ("ERROR: %s: allocation L2 SQ\n", __func__);
		return -1;
	}

	/* Configre <num_chans> L2 children for PQ, with static priority */
	res = cvmx_pko3_sq_config_children(node, level,
			l1_q_num, l2_q_base, num_chans, num_chans);

	if (res < 0) {
                cvmx_printf ("ERROR: %s: Configuring %s for PFC\n", __func__,
				__cvmx_pko3_sq_str(b1, level, l1_q_num));
		return -1;
	}

	/* Map each of the allocated channels */
	for (i = 0; i < num_chans; i++) {
		uint16_t chan;

		/* Get CHAN_E value for this PFC channel, PCP in low 3 bits */
		chan = ipd_port | cvmx_helper_prio2qos(i);

		cvmx_pko3_map_channel(node, l1_q_num, l2_q_base + i, chan);

	}

	/* Iterate through the levels until DQ and allocate 'num_chans'
	 * consecutive queues at each level and hook them up
	 * one-to-one with the parent level queues
	 */

	parent_q = l2_q_base;
	level = __cvmx_pko3_sq_lvl_next(level);

	do {

		child_q = cvmx_pko_alloc_queues(node, level,
			res_owner, -1, num_chans);

		if (child_q < 0) {
			cvmx_printf ("ERROR: %s: allocating %s\n",
				__func__,
				__cvmx_pko3_sq_str(b1, level, child_q));
			return -1;
		}

		for (i = 0; i < num_chans; i++) {
			res = cvmx_pko3_sq_config_children(node, level,
				parent_q + i, child_q + i, 1, 1);

			if (res < 0) {
				cvmx_printf ("ERROR: %s: configuring %s\n",
					__func__,
					__cvmx_pko3_sq_str(b1, level, child_q));
				return -1;
			}

		} /* for i */

		parent_q = child_q;
		level = __cvmx_pko3_sq_lvl_next(level);

	/* Terminate loop on DQ level, it has special handling */
	} while (level != CVMX_PKO_DESCR_QUEUES &&
		level != CVMX_PKO_LEVEL_INVAL);

	if (level != CVMX_PKO_DESCR_QUEUES) {
		cvmx_printf("ERROR: %s: level sequence error\n", __func__);
		return -1;
	}

	dq_base = cvmx_pko_alloc_queues(node, level, res_owner, -1, num_chans);
	if (dq_base < 0) {
		cvmx_printf ("ERROR: %s: allocating %s\n", __func__,
				__cvmx_pko3_sq_str(b1, level, dq_base));
		return -1;
	}

	/* Configure DQs in QoS order, so that QoS/PCP can be index */
	for (i = 0; i < num_chans; i++) {
		int dq_num = dq_base + cvmx_helper_prio2qos(i);
		res = cvmx_pko3_sq_config_children(node, level,
			parent_q + i, dq_num, 1, 1);
		if (res < 0) {
			cvmx_printf ("ERROR: %s: configuring %s\n", __func__,
				__cvmx_pko3_sq_str(b1, level, dq_num));
			return -1;
		}
	}

	/* register entire DQ range with the IPD translation table */
	__cvmx_pko3_ipd_dq_register(xiface, port, dq_base, num_chans);

	return 0;
}

/**
 * Initialize a simple interface with a a given number of
 * fair or prioritized queues.
 * This function will assign one channel per sub-interface.
 */
int __cvmx_pko3_config_gen_interface(int xiface, uint8_t subif,
	uint8_t num_queues, bool prioritized)
{
	cvmx_xiface_t xi = cvmx_helper_xiface_to_node_interface(xiface);
	uint8_t node = xi.node;
	int l1_q_num;
	int parent_q, child_q;
	int dq;
	int res, res_owner;
	int pko_mac_num;
	enum cvmx_pko3_level_e level;
	uint16_t ipd_port;
	int static_pri;
	char b1[12];

#if defined(__U_BOOT__)
	num_queues = 1;
#endif

	if (num_queues == 0) {
		num_queues = 1;
		cvmx_printf("WARNING: %s: xiface %#x misconfigured\n",
			__func__, xiface);
	}

	/* Configure DQs relative priority (a.k.a. scheduling) */
	if (prioritized) {
		/* With 8 queues or fewer, use static priority, else WRR */
		static_pri = (num_queues < 9)? num_queues: 0;
	} else {
		/* Set equal-RR scheduling among queues */
		static_pri = -1;
	}

	if (debug)
		cvmx_dprintf("%s: configuring xiface %u:%u/%u nq=%u %s\n",
			     __FUNCTION__, xi.node, xi.interface, subif,
			    num_queues, (prioritized)?"qos":"fair");

	/* Get MAC number for the iface/port */
	pko_mac_num = __cvmx_pko3_get_mac_num(xiface, subif);
	if (pko_mac_num < 0) {
		cvmx_printf ("ERROR: %s: Invalid interface %u:%u\n",
			__func__, xi.node, xi.interface);
		return -1;
	}

	ipd_port = cvmx_helper_get_ipd_port(xiface, subif);

	if(debug)
		cvmx_dprintf("%s: xiface %u:%u/%u ipd_port=%#03x\n",
			     __FUNCTION__, xi.node, xi.interface, subif,
				ipd_port);

	/* Build an identifiable owner identifier */
	res_owner = __cvmx_helper_pko3_res_owner(ipd_port);

	level = CVMX_PKO_PORT_QUEUES;

	/* Reserve port queue to make sure the MAC is not already configured */
	l1_q_num = cvmx_pko_alloc_queues(node, level, res_owner, -1, 1);

	if (l1_q_num < 0) {
		cvmx_printf("ERROR %s: xiface %u:%u/%u"
			" failed allocation L1 PQ\n",
			__func__, xi.node, xi.interface, subif);
		return -1;
	}

	res = cvmx_pko3_pq_config(node, pko_mac_num, l1_q_num);
	if (res < 0) {
                cvmx_printf ("ERROR %s: Configuring L1 PQ\n", __func__);
		return -1;
	}

	parent_q = l1_q_num;

	/* Determine the next queue level */
	level = __cvmx_pko3_sq_lvl_next(level);

	/* Simply chain queues 1-to-1 from L2 to one before DQ level */
	do {

		/* allocate next level queue */
		child_q = cvmx_pko_alloc_queues(node, level, res_owner, -1, 1);

		if (child_q < 0) {
			cvmx_printf ("ERROR: %s: allocating %s\n",
				__func__,
				__cvmx_pko3_sq_str(b1, level, child_q));
			return -1;
		}

		/* Configre newly allocated queue */
		res = cvmx_pko3_sq_config_children(node, level,
			parent_q, child_q, 1, 1);

		if (res < 0) {
			cvmx_printf ("ERROR: %s: configuring %s\n",
				__func__,
				__cvmx_pko3_sq_str(b1, level, child_q));
			return -1;
		}

		/* map IPD/channel to L2/L3 queues */
		if (level == cvmx_pko_default_channel_level)
			cvmx_pko3_map_channel(node,
				l1_q_num, child_q, ipd_port);

		/* Prepare for next level */
		level = __cvmx_pko3_sq_lvl_next(level);
		parent_q = child_q;

	/* Terminate loop on DQ level, it has special handling */
	} while (level != CVMX_PKO_DESCR_QUEUES &&
		level != CVMX_PKO_LEVEL_INVAL);

	if (level != CVMX_PKO_DESCR_QUEUES) {
		cvmx_printf("ERROR: %s: level sequence error\n", __func__);
		return -1;
	}

	/* Allocate descriptor queues for the port */
	dq = cvmx_pko_alloc_queues(node, level, res_owner, -1, num_queues);
	if (dq < 0) {
		cvmx_printf("ERROR: %s: could not reserve DQs\n", __func__);
		return -1;
	}

	res = cvmx_pko3_sq_config_children(node, level, parent_q,
		dq, num_queues, static_pri);
	if (res < 0) {
		cvmx_printf ("ERROR: %s: configuring %s\n", __func__,
				__cvmx_pko3_sq_str(b1, level, dq));
		return -1;
	}

	/* register DQ/IPD translation */
	__cvmx_pko3_ipd_dq_register(xiface, subif, dq, num_queues);

	if(debug)
		cvmx_dprintf("%s: xiface %u:%u/%u qs %u-%u\n",
			     __FUNCTION__, xi.node, xi.interface, subif,
				dq, dq+num_queues-1);
	return 0;
}
EXPORT_SYMBOL(__cvmx_pko3_config_gen_interface);

/** Initialize the NULL interface
 *
 * A NULL interface is a special case in that it is not
 * one of the enumerated interfaces in the system, and does
 * not apply to input either. Still, it can be very handy
 * for dealing with packets that should be discarded in
 * a generic, streamlined way.
 *
 * The Descriptor Queue 0 will be reserved for the NULL interface
 * and the normalized (i.e. IPD) port number has the all-ones value.
 */
static int __cvmx_pko3_config_null_interface(unsigned int node)
{
	int l1_q_num;
	int parent_q, child_q;
	enum cvmx_pko3_level_e level;
	int i, res, res_owner;
	int xiface, ipd_port;
	int num_dq = 1;	/* # of DQs for NULL */
	const int dq = 0;	/* Reserve DQ#0 for NULL */
	char pko_mac_num;
	char b1[12];

	if(OCTEON_IS_MODEL(OCTEON_CN78XX))
		pko_mac_num = 0x1C; /* MAC# 28 virtual MAC for NULL */
	else if (OCTEON_IS_MODEL(OCTEON_CN73XX))
		pko_mac_num = 0x0F; /* MAC# 16 virtual MAC for NULL */
	else if (OCTEON_IS_MODEL(OCTEON_CNF75XX))
		pko_mac_num = 0x0A; /* MAC# 10 virtual MAC for NULL */
	else
		return -1;

	if(OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_0))
		num_dq = 8;

	if(debug)
		cvmx_dprintf("%s: null iface dq=%u-%u\n",
			__FUNCTION__, dq, dq+num_dq-1);

	ipd_port = cvmx_helper_node_to_ipd_port(node, CVMX_PKO3_IPD_PORT_NULL);

	/* Build an identifiable owner identifier by MAC# for easy release */
	res_owner = __cvmx_helper_pko3_res_owner(ipd_port);
	if (res_owner < 0) {
		cvmx_dprintf ("%s: ERROR Invalid interface\n", __FUNCTION__);
		return -1;
	}

	level = CVMX_PKO_PORT_QUEUES;

	/* Allocate a port queue */
	l1_q_num = cvmx_pko_alloc_queues(node, level, res_owner, -1, 1);

	if (l1_q_num < 0) {
		cvmx_dprintf ("%s: ERROR reserving L1 SQ\n", __func__);
		return -1;
	}

	res = cvmx_pko3_pq_config(node, pko_mac_num, l1_q_num);
	if (res < 0) {
		cvmx_printf("ERROR: %s: PQ/L1 queue configuration\n", __func__);
		return -1;
	}

	parent_q = l1_q_num;

	/* Determine the next queue level */
	level = __cvmx_pko3_sq_lvl_next(level);

	/* Simply chain queues 1-to-1 from L2 to one before DQ level */
	do {

		/* allocate next level queue */
		child_q = cvmx_pko_alloc_queues(node, level, res_owner, -1, 1);

		if (child_q < 0) {
			cvmx_printf ("ERROR: %s: allocating %s\n",
				__func__,
				__cvmx_pko3_sq_str(b1, level, child_q));
			return -1;
		}

		/* Configre newly allocated queue */
		res = cvmx_pko3_sq_config_children(node, level,
			parent_q, child_q, 1, 1);

		if (res < 0) {
			cvmx_printf ("ERROR: %s: configuring %s\n",
				__func__,
				__cvmx_pko3_sq_str(b1, level, child_q));
			return -1;
		}

		/* Prepare for next level */
		level = __cvmx_pko3_sq_lvl_next(level);
		parent_q = child_q;

	/* Terminate loop on DQ level, it has special handling */
	} while (level != CVMX_PKO_DESCR_QUEUES &&
		level != CVMX_PKO_LEVEL_INVAL);

	if (level != CVMX_PKO_DESCR_QUEUES) {
		cvmx_printf("ERROR: %s: level sequence error\n", __func__);
		return -1;
	}

	/* Reserve 'num_dq' DQ's at 0 by convention */
	res = cvmx_pko_alloc_queues(node, level, res_owner, dq, num_dq);
	if (dq != res) {
		cvmx_dprintf("%s: ERROR: could not reserve DQs\n",
			__FUNCTION__);
		return -1;
	}

	res = cvmx_pko3_sq_config_children(node, level, parent_q,
		dq, num_dq, num_dq);
	if (res < 0) {
		cvmx_printf ("ERROR: %s: configuring %s\n", __func__,
				__cvmx_pko3_sq_str(b1, level, dq));
		return -1;
	}

	/* NULL interface does not need to map to a CHAN_E */

	/* register DQ/IPD translation */
	xiface = cvmx_helper_node_interface_to_xiface(node, __CVMX_XIFACE_NULL);
	__cvmx_pko3_ipd_dq_register(xiface, 0, dq, num_dq);

	/* open the null DQs here */
	for(i = 0; i < num_dq; i++) {
		unsigned limit = 128; /* NULL never really uses much */
		cvmx_pko_dq_open(node, dq + i);
		cvmx_pko3_dq_set_limit(node, dq + i, limit);
	}

	return 0;
}

/** Open all descriptor queues belonging to an interface/port
 * @INTERNAL
 */
int __cvmx_pko3_helper_dqs_activate(int xiface, int index, bool min_pad)
{
	int  ipd_port,dq_base, dq_count, i;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	unsigned limit;

	/* Get local IPD port for the interface */
	ipd_port = cvmx_helper_get_ipd_port(xiface, index);
	if(ipd_port < 0) {
		cvmx_printf("ERROR: %s: No IPD port for interface %d port %d\n",
			     __FUNCTION__, xiface, index);
		return -1;
	}

	/* Get DQ# range for the IPD port */
	dq_base = cvmx_pko3_get_queue_base(ipd_port);
	dq_count = cvmx_pko3_get_queue_num(ipd_port);
	if( dq_base < 0 || dq_count <= 0) {
		cvmx_printf("ERROR: %s: No descriptor queues for interface %d port %d\n",
			     __FUNCTION__, xiface, index);
		return -1;
	}

	/* Mask out node from global DQ# */
	dq_base &= (1<<10)-1;

	limit = __pko_pkt_quota / dq_count /
		cvmx_helper_interface_enumerate(xiface);

	for(i = 0; i < dq_count; i++) {
		/* FIXME: 2ms at 1Gbps max packet rate, make speed dependent */
		cvmx_pko_dq_open(xi.node, dq_base + i);
		cvmx_pko3_dq_options(xi.node, dq_base + i, min_pad);

		if (debug)
			cvmx_dprintf("%s: DQ%u limit %d\n",
				__func__, dq_base + i, limit);

		cvmx_pko3_dq_set_limit(xi.node, dq_base + i, limit);
		__pko_pkt_budget -= limit;
	}

	if (__pko_pkt_budget < 0)
		cvmx_printf("WARNING: %s: PKO buffer deficit %d\n",
			__func__, __pko_pkt_budget);
	else if (debug)
		cvmx_dprintf("%s: PKO remaining packet budget: %d\n",
			__func__, __pko_pkt_budget);

	return i;
}
EXPORT_SYMBOL(__cvmx_pko3_helper_dqs_activate);

/** Configure and initialize PKO3 for an interface
 *
 * @param xiface is the interface number to configure
 * @return 0 on success.
 */
int cvmx_helper_pko3_init_interface(int xiface)
{
	cvmx_helper_interface_mode_t mode;
	int subif, num_ports;
	bool fcs_enable, pad_enable, pad_enable_pko;
	uint8_t fcs_sof_off = 0;
	uint8_t num_queues = 1;
	bool qos = false, pfc = false;
	int res = -1;
	cvmx_xiface_t xi = cvmx_helper_xiface_to_node_interface(xiface);

	mode = cvmx_helper_interface_get_mode(xiface);
	num_ports = cvmx_helper_interface_enumerate(xiface);
	subif = 0;

	if ((unsigned) xi.interface <
		NUM_ELEMENTS(__cvmx_pko_queue_static_config
			.pknd.pko_cfg_iface)) {
		pfc = __cvmx_pko_queue_static_config
			.pknd.pko_cfg_iface[xi.interface]
			.pfc_enable;
		num_queues =
			__cvmx_pko_queue_static_config.
			pknd.pko_cfg_iface[xi.interface]
			.queues_per_port;
		qos =
			__cvmx_pko_queue_static_config.
			pknd.pko_cfg_iface[xi.interface]
			.qos_enable;
	}

	/* Force 8 DQs per port for pass 1.0 to circumvent limitations */
	if(OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_0))
		num_queues = 8;

	/* For ILK there is one IPD port per channel */
	if ((mode == CVMX_HELPER_INTERFACE_MODE_ILK))
		num_ports =  __cvmx_helper_ilk_enumerate(xiface);

	/* Skip non-existent interfaces */
	if (num_ports < 1) {
		cvmx_dprintf("ERROR: %s: invalid iface %u:%u\n",
			     __FUNCTION__, xi.node, xi.interface);
		return -1;
	}

	if (mode == CVMX_HELPER_INTERFACE_MODE_LOOP) {
		num_queues =
			__cvmx_pko_queue_static_config.
				pknd.pko_cfg_loop.queues_per_port;
		qos =
			__cvmx_pko_queue_static_config.
				pknd.pko_cfg_loop.qos_enable;

		if(OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_0))
			num_queues = 8;

		res = __cvmx_pko3_config_chan_interface(xiface, num_ports,
				num_queues, qos);
		if (res < 0) {
			goto __cfg_error;
		}
	}
	else if (mode == CVMX_HELPER_INTERFACE_MODE_NPI) {
		num_queues =
			__cvmx_pko_queue_static_config.
				pknd.pko_cfg_npi.queues_per_port;
		qos =
			__cvmx_pko_queue_static_config.
				pknd.pko_cfg_npi.qos_enable;

		if(OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_0))
			num_queues = 8;

		res = __cvmx_pko3_config_chan_interface(xiface, num_ports,
				num_queues, qos);
		if (res < 0) {
			goto __cfg_error;
		}
	}
	/* ILK-specific queue configuration */
	else if (mode == CVMX_HELPER_INTERFACE_MODE_ILK) {
		unsigned num_chans = __cvmx_helper_ilk_enumerate(xiface);
		num_queues = 8; qos = true; pfc = false;

		if (num_chans >= 128)
		 	num_queues = 1;
		else if (num_chans >= 64)
			num_queues = 2;
		else if (num_chans >= 32)
			num_queues = 4;
		else
			num_queues = 8;

		res = __cvmx_pko3_config_chan_interface(xiface, num_chans,
				num_queues, qos);
	}
	/* Setup all ethernet configured for PFC */
	else if (pfc) {
		/* PFC interfaces have 8 prioritized queues */
		for (subif = 0; subif < num_ports; subif++) {
			res = __cvmx_pko3_config_pfc_interface(
				xiface, subif);
			if (res < 0)
				goto __cfg_error;

			/* Enable PFC/CBFC on BGX */
			__cvmx_helper_bgx_xaui_config_pfc(xi.node,
				xi.interface, subif, true);
		}
	}
	/* All other interfaces follow static configuration */
	else {

		for (subif = 0; subif < num_ports; subif++) {
			res = __cvmx_pko3_config_gen_interface(xiface, subif,
				num_queues, qos);
			if (res < 0) {
				goto __cfg_error;
			}
		}
	}

	fcs_enable = __cvmx_helper_get_has_fcs(xiface);
	pad_enable = __cvmx_helper_get_pko_padding(xiface);

	/* Do not use PKO PAD/FCS generation on o78p1.x on BGX interfaces */
	if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
		pad_enable_pko = false;
	else
		pad_enable_pko = pad_enable;

	if(debug)
		cvmx_dprintf("%s: iface %u:%u FCS=%d pad=%d pko=%d\n",
			__func__, xi.node, xi.interface,
			fcs_enable, pad_enable, pad_enable_pko);

	/* Setup interface options */
	for (subif = 0; subif < num_ports; subif++) {

		/* Open interface/port DQs to allow transmission to begin */
		res = __cvmx_pko3_helper_dqs_activate(xiface,
			subif, pad_enable_pko);

		if (res < 0)
			goto __cfg_error;

		/* ILK has only one MAC, subif == logical-channel */
		if (mode == CVMX_HELPER_INTERFACE_MODE_ILK && subif > 0)
			continue;

		/* LOOP has only one MAC, subif == logical-channel */
		if (mode == CVMX_HELPER_INTERFACE_MODE_LOOP && subif > 0)
			continue;

		/* NPI has only one MAC, subif == 'ring' */
		if (mode == CVMX_HELPER_INTERFACE_MODE_NPI && subif > 0)
			continue;

		/* for sRIO there is 16 byte sRIO header, outside of FCS */
		if (mode == CVMX_HELPER_INTERFACE_MODE_SRIO)
			fcs_sof_off = 16;

		if (xi.interface >= CVMX_HELPER_MAX_GMX) {
			/* Non-BGX interface, use PKO for FCS/PAD */
			res = cvmx_pko3_interface_options(xiface, subif,
				fcs_enable, pad_enable_pko, fcs_sof_off);
		} else if (pad_enable == pad_enable_pko) {
			/* BGX interface: FCS/PAD done by PKO */
			res = cvmx_pko3_interface_options(xiface, subif,
				  fcs_enable, pad_enable, fcs_sof_off);
			cvmx_helper_bgx_tx_options(xi.node, xi.interface,
				subif, false, false);
		} else {
			/* BGX interface: FCS/PAD done by BGX */
			res = cvmx_pko3_interface_options(xiface, subif,
				  false, false, fcs_sof_off);
			cvmx_helper_bgx_tx_options(xi.node, xi.interface,
				subif, fcs_enable, pad_enable);
		}

		if(res < 0)
			cvmx_dprintf("WARNING: %s: "
				"option set failed on iface %u:%u/%u\n",
				__FUNCTION__, xi.node, xi.interface, subif);
		if (debug)
			cvmx_dprintf("%s: face %u:%u/%u fifo size %d\n",
				__func__, xi.node, xi.interface, subif,
				cvmx_pko3_port_fifo_size(xiface, subif));
	}
	return 0;

  __cfg_error:
	cvmx_dprintf("ERROR: %s: failed on iface %u:%u/%u\n",
		__FUNCTION__, xi.node, xi.interface, subif);
	return -1;
}

/**
 * Global initialization for PKO3
 *
 * Should only be called once on each node
 *
 * TBD: Resolve the kernel case.
 * When Linux eats up the entire memory, bootmem will be unable to
 * satisfy our request, and the memory needs to come from Linux free pages.
 */
int __cvmx_helper_pko3_init_global(unsigned int node, uint16_t gaura)
{
	int res;

	res = cvmx_pko3_hw_init_global(node, gaura);
	if(res < 0) {
		cvmx_dprintf("ERROR: %s:failed block initialization\n",
			__FUNCTION__);
		return res;
	}

	/* configure channel level */
	cvmx_pko3_channel_credit_level(node, cvmx_pko_default_channel_level);

	/* add NULL MAC/DQ setup */
	res = __cvmx_pko3_config_null_interface(node);
	if (res < 0)
		cvmx_dprintf("ERROR: %s: creating NULL interface\n",
			__FUNCTION__);

	return res;
}
EXPORT_SYMBOL(__cvmx_helper_pko3_init_global);

/**
 * Global initialization for PKO3
 *
 * Should only be called once on each node
 *
 * When Linux eats up the entire memory, bootmem will be unable to
 * satisfy our request, and the memory needs to come from Linux free pages.
 */
int cvmx_helper_pko3_init_global(unsigned int node)
{
	void *ptr;
	int res = -1;
	unsigned aura_num = ~0;
	cvmx_fpa3_gaura_t aura;

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	/* Allocate memory required by PKO3 */
	res = __cvmx_pko3_config_memory(node);
#endif
	if(res < 0) {
		cvmx_dprintf("ERROR: %s: PKO3 memory allocation error\n",
			__FUNCTION__);
		return res;
	}

	aura_num = res;
	aura = __cvmx_pko3_aura[node];

	/* Exercise the FPA to make sure the AURA is functional */
	ptr = cvmx_fpa3_alloc(aura);

	if (ptr == NULL )
		res = -1;
	else {
		cvmx_fpa3_free_nosync(ptr, aura, 0);
		res = 0;
	}

	if (res < 0) {
		cvmx_dprintf("ERROR: %s: FPA failure AURA=%u:%d\n",
			__func__, aura.node, aura.laura);
		return -1;
	}

	res = __cvmx_helper_pko3_init_global(node, aura_num);

	if (res < 0)
		cvmx_dprintf("ERROR: %s: failed to start PPKO\n",__func__);

	return res;
}

/**
 * Uninitialize PKO3 interface
 *
 * Release all resources held by PKO for an interface.
 * The shutdown code is the same for all supported interfaces.
 *
 * NOTE: The NULL virtual interface is identified by interface
 * number -1, which translates into IPD port 0xfff, MAC#28. [Kludge]
 */
int cvmx_helper_pko3_shut_interface(int xiface)
{
	int index, num_ports;
	int dq_base, dq_count;
	uint16_t ipd_port;
	int i, res_owner, res;
	enum cvmx_pko3_level_e level;
	cvmx_pko3_dq_params_t *pParam;
	uint64_t cycles;
	const unsigned timeout = 10;	/* milliseconds */
	cvmx_xiface_t xi = cvmx_helper_xiface_to_node_interface(xiface);

	if(__cvmx_helper_xiface_is_null(xiface)) {
		/* Special case for NULL interface */
		num_ports = 1;
	} else {
		cvmx_helper_interface_mode_t mode;
		mode = cvmx_helper_interface_get_mode(xiface);
		num_ports = cvmx_helper_interface_enumerate(xiface);
		(void) mode;
	}

	/* Skip non-existent interfaces silently */
	if(num_ports < 1) {
		return -1;
	}

	if (debug)
		cvmx_dprintf("%s: xiface %u:%d ports %d\n",
			__func__, xi.node, xi.interface , num_ports);

	for (index = 0; index < num_ports; index ++) {

		if (__cvmx_helper_xiface_is_null(xiface))
                        ipd_port = cvmx_helper_node_to_ipd_port(xi.node,
				CVMX_PKO3_IPD_PORT_NULL);
		else
			ipd_port = cvmx_helper_get_ipd_port(xiface, index);

		/* Retreive DQ range for the index */
                dq_base = cvmx_pko3_get_queue_base(ipd_port);
                dq_count = cvmx_pko3_get_queue_num(ipd_port);

                if( dq_base < 0 || dq_count < 0) {
                        cvmx_dprintf("ERROR: %s: No DQs for iface %u:%d/%u\n",
                                __FUNCTION__, xi.node, xi.interface, index);
			continue;
		}

		/* Get rid of node-number in DQ# */
		dq_base &= (1 << 10)-1;

		if (debug)
			cvmx_dprintf("%s: xiface %u:%d/%d dq %u-%u\n",
			__func__, xi.node, xi.interface, index,
			dq_base, dq_base + dq_count -1);

		/* Unregister the DQs for the port, should stop traffic */
		res = __cvmx_pko3_ipd_dq_unregister(xiface, index);
		if(res < 0) {
                        cvmx_dprintf("ERROR: %s: "
				"failed to unregister DQs iface %u/%d/%u\n",
                                __FUNCTION__, xi.node, xi.interface, index);
			continue;
		}

		/* Begin draining all queues */
		for(i = 0; i < dq_count; i++) {
			cvmx_pko3_dq_drain(xi.node, dq_base + i);
		}

		/* Wait for all queues to drain, and close them */
		for(i = 0; i < dq_count; i++) {
			/* Prepare timeout */
			cycles = cvmx_get_cycle();
			cycles += cvmx_clock_get_rate(CVMX_CLOCK_CORE)/1000 * timeout;

			/* Wait for queue to drain */
			do {
				res = cvmx_pko3_dq_query(xi.node, dq_base + i);
				if (cycles < cvmx_get_cycle())
					break;
			} while(res > 0);

			if (res != 0)
				cvmx_dprintf("ERROR: %s: querying queue %u\n",
					__FUNCTION__, dq_base + i);

			/* Close the queue, free internal buffers */
			res = cvmx_pko3_dq_close(xi.node, dq_base + i);

			if (res < 0)
				cvmx_dprintf("ERROR: %s: closing queue %u\n",
					__FUNCTION__, dq_base + i);

			/* Return DQ packet budget */
			pParam = cvmx_pko3_dq_parameters(xi.node, dq_base  + i);
			__pko_pkt_budget += pParam->limit;
			pParam->limit = 0;
		}

		/* Release all global resources owned by this interface/port */

		res_owner = __cvmx_helper_pko3_res_owner(ipd_port);
		if (res_owner < 0) {
			cvmx_dprintf ("ERROR: %s: no resource owner ticket\n",
				__FUNCTION__);
			continue;
		}

		/* Actuall PQ/SQ/DQ associations left intact */
		for(level = CVMX_PKO_PORT_QUEUES;
		    level != CVMX_PKO_LEVEL_INVAL;
		    level = __cvmx_pko3_sq_lvl_next(level)) {
			cvmx_pko_free_queues(xi.node, level, res_owner);
		}

	} /* for port */

	return 0;
}
EXPORT_SYMBOL(cvmx_helper_pko3_shut_interface);

/**
 * Shutdown PKO3
 *
 * Should be called after all interfaces have been shut down on the PKO3.
 *
 * Disables the PKO, frees all its buffers.
 */
int cvmx_helper_pko3_shutdown(unsigned int node)
{
	unsigned dq;
	int res;

	 /* destroy NULL interface here, only PKO knows about it */
	cvmx_helper_pko3_shut_interface(cvmx_helper_node_interface_to_xiface(node, __CVMX_XIFACE_NULL));

#ifdef	__PKO_DQ_CLOSE_ERRATA_FIXED
	/* Check that all DQs are closed */
	/* this seems to cause issue on HW:
	 * the error code differs from expected
	 */
	for(dq =0; dq < (1<<10); dq++) {
		res = cvmx_pko3_dq_close(node, dq);
		if (res != 0) {
			cvmx_dprintf("ERROR: %s: PKO3 descriptor queue %u "
				"could not be closed\n",
				__FUNCTION__, dq);
			return -1;
		}
	}
#endif
	(void) dq;
	res = cvmx_pko3_hw_disable(node);

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	/* shut down AURA/POOL we created, and free its resources */
	cvmx_fpa3_shutdown_aura_and_pool(__cvmx_pko3_aura[node]);
#endif /* CVMX_BUILD_FOR_LINUX_KERNEL */
	return res;
}
EXPORT_SYMBOL(cvmx_helper_pko3_shutdown);

/*#define	__PKO_HW_DEBUG*/
#ifdef	__PKO_HW_DEBUG
#define	CVMX_DUMP_REGX(reg) cvmx_dprintf("%s=%#llx\n",#reg,(long long)cvmx_read_csr(reg))
#define	CVMX_DUMP_REGD(reg) cvmx_dprintf("%s=%lld.\n",#reg,(long long)cvmx_read_csr(reg))
/*
 * function for debugging PKO reconfiguration
 */
void cvmx_fpa3_aura_dump_regs(unsigned node, uint16_t aura)
	{
	int pool_num =
		cvmx_read_csr_node(node,CVMX_FPA_AURAX_POOL(aura));

	CVMX_DUMP_REGX(CVMX_FPA_AURAX_POOL(aura));
	CVMX_DUMP_REGX(CVMX_FPA_POOLX_CFG(pool_num));
	CVMX_DUMP_REGX(CVMX_FPA_POOLX_OP_PC(pool_num));
	CVMX_DUMP_REGX(CVMX_FPA_POOLX_INT(pool_num));
	CVMX_DUMP_REGD(CVMX_FPA_POOLX_AVAILABLE(pool_num));
	CVMX_DUMP_REGD(CVMX_FPA_POOLX_THRESHOLD(pool_num));
	CVMX_DUMP_REGX(CVMX_FPA_AURAX_CFG(aura));
	CVMX_DUMP_REGX(CVMX_FPA_AURAX_INT(aura));
	CVMX_DUMP_REGD(CVMX_FPA_AURAX_CNT(aura));
	CVMX_DUMP_REGD(CVMX_FPA_AURAX_CNT_LIMIT(aura));
	CVMX_DUMP_REGX(CVMX_FPA_AURAX_CNT_THRESHOLD(aura));
	CVMX_DUMP_REGX(CVMX_FPA_AURAX_CNT_LEVELS(aura));
	CVMX_DUMP_REGX(CVMX_FPA_AURAX_POOL_LEVELS(aura));

	}

void cvmx_pko3_dump_regs(unsigned node)
{
	(void) node;
	CVMX_DUMP_REGX( CVMX_PKO_NCB_INT );
	CVMX_DUMP_REGX( CVMX_PKO_PEB_ERR_INT );
	CVMX_DUMP_REGX( CVMX_PKO_PDM_ECC_DBE_STS_CMB0 );
	CVMX_DUMP_REGX( CVMX_PKO_PDM_ECC_SBE_STS_CMB0 );
	CVMX_DUMP_REGX( CVMX_PKO_PEB_ECC_DBE_STS0 );
	CVMX_DUMP_REGX( CVMX_PKO_PEB_ECC_DBE_STS_CMB0 );
}

#endif	/* __PKO_HW_DEBUG */

#ifdef CVMX_DUMP_PKO
/*
 * Show PKO integrated configuration.
 * See function prototype in cvmx-helper-pko3.h
 */
#define PKO_PRN_HEADLEN  16 
#define PKO_PRN_DATALEN  64
#define PKO_PRN_LINELEN  (PKO_PRN_HEADLEN + PKO_PRN_DATALEN)
#define PKO_PRN_DPLEN(__n)  (PKO_PRN_DATALEN / __n)

#define DLMPRINT(__format, ...) \
do { \
	int __n; \
	sprintf(lines[1], __format, ## __VA_ARGS__); \
	__n = PKO_PRN_LINELEN - strlen(lines[1]) - 1; \
	memset(lines[0], '-', __n);  lines[0][__n] = '\0'; \
	cvmx_printf("%s %s\n", lines[0], lines[1]); \
} while (0)

#define PARPRINT(__offs, __head, __format, ...) \
do {\
	cvmx_printf("%*s%-*s", __offs, "", PKO_PRN_HEADLEN - __offs, __head); \
	cvmx_printf(__format, ## __VA_ARGS__); \
} while (0)

#define PKO_MAC_NUM	32
char *pko_macmap[PKO_MAC_NUM][3] = {
			/*CN78XX		CN73XX			CNF75XX*/
	[0]  = {"LBK      ",	"LBK      ",	"LBK      "},
	[1]  = {"DPI      ",	"DPI      ",	"DPI      "},
	[2]  = {"ILK0     ",	"BGX0:MAC0",	"BGX0:MAC0"},
	[3]  = {"ILK1     ",	"BGX0:MAC1",	"BGX0:MAC1"},
	[4]  = {"BGX0:MAC0",	"BGX0:MAC2",	"BGX0:MAC2"},
	[5]  = {"BGX0:MAC1",	"BGX0:MAC3",	"BGX0:MAC3"},
	[6]  = {"BGX0:MAC2",	"BGX1:MAC0",	"SRIO0-0  "},
	[7]  = {"BGX0:MAC3",	"BGX1:MAC1",	"SRIO0-1  "},
	[8]  = {"BGX1:MAC0",	"BGX1:MAC2",	"SRIO1-0  "},
	[9]  = {"BGX1:MAC1",	"BGX1:MAC3",	"SRIO1-1  "},
	[10] = {"BGX1:MAC2",	"BGX2:MAC0",	"NULL     "},
	[11] = {"BGX1:MAC3",	"BGX2:MAC1",	NULL},
	[12] = {"BGX2:MAC0",	"BGX2:MAC2",	NULL},
	[13] = {"BGX2:MAC1",	"BGX2:MAC3",	NULL},
	[14] = {"BGX2:MAC2",	"NULL     ",	NULL},
	[15] = {"BGX2:MAC3",	NULL,			NULL},
	[16] = {"BGX3:MAC0",	NULL,			NULL},
	[17] = {"BGX3:MAC1",	NULL,			NULL},
	[18] = {"BGX3:MAC2",	NULL,			NULL},
	[19] = {"BGX3:MAC3",	NULL,			NULL},
	[20] = {"BGX4:MAC0",	NULL,			NULL},
	[21] = {"BGX4:MAC1",	NULL,			NULL},
	[22] = {"BGX4:MAC2",	NULL,			NULL},
	[23] = {"BGX4:MAC3",	NULL,			NULL},
	[24] = {"BGX5:MAC0",	NULL,			NULL},
	[25] = {"BGX5:MAC1",	NULL,			NULL},
	[26] = {"BGX5:MAC2",	NULL,			NULL},
	[27] = {"BGX5:MAC3",	NULL,			NULL},
	[28] = {"NULL     ",	NULL,			NULL},
	[29] = {NULL,			NULL,			NULL},
	[30] = {NULL,			NULL,			NULL},
	[31] = {NULL,			NULL,			NULL}
};

int cvmx_helper_pko3_config_dump(unsigned int node)
{
	int queue, nqueues, group, base, nmacs, ngroups;
	cvmx_pko_dqx_sw_xoff_t dqxoff;
	cvmx_pko_dqx_topology_t dqtop;
	cvmx_pko_l5_sqx_topology_t l5top;
	cvmx_pko_l4_sqx_topology_t l4top;
	cvmx_pko_l3_sqx_topology_t l3top;
	cvmx_pko_l2_sqx_topology_t l2top;
	cvmx_pko_l1_sqx_topology_t l1top;
	cvmx_pko_dqx_schedule_t dqsch;
	cvmx_pko_l5_sqx_schedule_t l5sch;
	cvmx_pko_l4_sqx_schedule_t l4sch;
	cvmx_pko_l3_sqx_schedule_t l3sch;
	cvmx_pko_l2_sqx_schedule_t l2sch;
	cvmx_pko_l1_sqx_schedule_t l1sch;
	cvmx_pko_dqx_shape_t dqshape;
	cvmx_pko_l5_sqx_shape_t l5shape;
	cvmx_pko_l4_sqx_shape_t l4shape;
	cvmx_pko_l3_sqx_shape_t l3shape;
	cvmx_pko_l2_sqx_shape_t l2shape;
	cvmx_pko_l1_sqx_shape_t l1shape;
	cvmx_pko_dqx_cir_t dqcir;
	cvmx_pko_l5_sqx_cir_t l5cir;
	cvmx_pko_l4_sqx_cir_t l4cir;
	cvmx_pko_l3_sqx_cir_t l3cir;
	cvmx_pko_l2_sqx_cir_t l2cir;
	cvmx_pko_l1_sqx_cir_t l1cir;
	cvmx_pko_dqx_pir_t dqpir;
	cvmx_pko_l5_sqx_pir_t l5pir;
	cvmx_pko_l4_sqx_pir_t l4pir;
	cvmx_pko_l3_sqx_pir_t l3pir;
	cvmx_pko_l2_sqx_pir_t l2pir;
	cvmx_pko_macx_cfg_t maccfg;
	cvmx_pko_l3_l2_sqx_channel_t chcfg;
	cvmx_pko_channel_level_t chlvl;
	cvmx_pko_shaper_cfg_t shapercfg;
	cvmx_pko_l1_sqx_link_t l1link;
	uint32_t crc32, pcrc32;
	char lines[4][128];
	int ciren, piren;
	uint64_t dqsh_clk, pqsh_clk;
	int shaper_rate(int shclk, int man, int exp, int div) {
		return (CVMX_SHOFT_TO_U64(man, exp) * shclk >> div) * 8/*bits*/ / 1000000/*Mbps*/;
	}
	int shaper_burst(int man, int exp) {
		return CVMX_SHOFT_TO_U64(man, (exp + 1));
	}

	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) ||
		OCTEON_IS_MODEL(OCTEON_CN73XX) ||
		OCTEON_IS_MODEL(OCTEON_CNF75XX))) {
		cvmx_printf("PKO3 Config Dump is not supported on this OCTEON model\n");
		return 0;
	}
	dqsh_clk = cvmx_pko3_dq_tw_clock_rate_node(node);
	pqsh_clk = cvmx_pko3_pq_tw_clock_rate_node(node);

	memset(lines[3], '*', PKO_PRN_LINELEN);  lines[3][PKO_PRN_LINELEN] = '\0';
	cvmx_printf("\n%s\n", lines[3]);
	cvmx_printf("   PKO Configuration (Node %d)\n", node);
	cvmx_printf("%s\n", lines[3]);

	/* Global parameters: */
	chlvl.u64 = cvmx_read_csr_node(node, CVMX_PKO_CHANNEL_LEVEL);
	shapercfg.u64 = cvmx_read_csr_node(node, CVMX_PKO_SHAPER_CFG);
	PARPRINT(0, "CC Level", "%*d\n", PKO_PRN_DPLEN(1), chlvl.s.cc_level + 2);
	PARPRINT(0, "Color Aware", "%*s\n", PKO_PRN_DPLEN(1),
		(shapercfg.s.color_aware == 1) ? "Yes":"No");

	/* Queues: */
	cvmx_printf("%-*s%*s%*s%*s%*s%*s%*s%*s%*s\n", PKO_PRN_HEADLEN, "",
		PKO_PRN_DPLEN(8), "DQ", PKO_PRN_DPLEN(8), "L5", PKO_PRN_DPLEN(8), "L4",
		PKO_PRN_DPLEN(8), "L3", PKO_PRN_DPLEN(8),"L2", PKO_PRN_DPLEN(8), "L1",
		PKO_PRN_DPLEN(8), "MAC", PKO_PRN_DPLEN(8), "FIFO");

	nqueues = cvmx_pko3_num_level_queues(CVMX_PKO_DESCR_QUEUES);
	nmacs = __cvmx_pko3_num_macs();
	for (queue = 0, pcrc32 = 0, base = 0; queue < nqueues; queue++) {
		CVMX_MT_CRC_POLYNOMIAL(0x1edc6f41);
		CVMX_MT_CRC_IV(0xffffffff);
		/* Descriptor Queue Level: */
		dqxoff.u64 = cvmx_read_csr_node(node, CVMX_PKO_DQX_SW_XOFF(queue));
		CVMX_MT_CRC_DWORD(dqxoff.u64);
		dqtop.u64 = cvmx_read_csr_node(node, CVMX_PKO_DQX_TOPOLOGY(queue));
		CVMX_MT_CRC_DWORD(dqtop.u64);
		dqsch.u64 = cvmx_read_csr_node(node, CVMX_PKO_DQX_SCHEDULE(queue));
		CVMX_MT_CRC_DWORD(dqsch.u64);
		dqshape.u64 = cvmx_read_csr_node(node, CVMX_PKO_DQX_SHAPE(queue));
		CVMX_MT_CRC_DWORD(dqshape.u64);
		dqcir.u64 = cvmx_read_csr_node(node, CVMX_PKO_DQX_CIR(queue));
		CVMX_MT_CRC_DWORD(dqcir.u64);
		dqpir.u64 = cvmx_read_csr_node(node, CVMX_PKO_DQX_PIR(queue));
		CVMX_MT_CRC_DWORD(dqpir.u64);

		/* L5-L3 Queue Levels: */
		if (OCTEON_IS_MODEL(OCTEON_CN78XX)) { 
			l5top.u64 = cvmx_read_csr_node(node, CVMX_PKO_L5_SQX_TOPOLOGY(dqtop.s.parent));
			CVMX_MT_CRC_DWORD(l5top.u64);
			l4top.u64 = cvmx_read_csr_node(node, CVMX_PKO_L4_SQX_TOPOLOGY(l5top.s.parent));
			CVMX_MT_CRC_DWORD(l4top.u64);
			l3top.u64 = cvmx_read_csr_node(node, CVMX_PKO_L3_SQX_TOPOLOGY(l4top.s.parent));
			CVMX_MT_CRC_DWORD(l3top.u64);

			l5sch.u64 = cvmx_read_csr_node(node, CVMX_PKO_L5_SQX_SCHEDULE(dqtop.s.parent));
			CVMX_MT_CRC_DWORD(l5sch.u64);
			l4sch.u64 = cvmx_read_csr_node(node, CVMX_PKO_L4_SQX_SCHEDULE(l5top.s.parent));
			CVMX_MT_CRC_DWORD(l4sch.u64);
			l3sch.u64 = cvmx_read_csr_node(node, CVMX_PKO_L3_SQX_SCHEDULE(l4top.s.parent));
			CVMX_MT_CRC_DWORD(l3sch.u64);

			l5shape.u64 = cvmx_read_csr_node(node, CVMX_PKO_L5_SQX_SHAPE(dqtop.s.parent));
			CVMX_MT_CRC_DWORD(l5shape.u64);
			l4shape.u64 = cvmx_read_csr_node(node, CVMX_PKO_L4_SQX_SHAPE(l5top.s.parent));
			CVMX_MT_CRC_DWORD(l4shape.u64);
			l3shape.u64 = cvmx_read_csr_node(node, CVMX_PKO_L3_SQX_SHAPE(l4top.s.parent));
			CVMX_MT_CRC_DWORD(l3shape.u64);

			l5cir.u64 = cvmx_read_csr_node(node, CVMX_PKO_L5_SQX_CIR(dqtop.s.parent));
			CVMX_MT_CRC_DWORD(l5cir.u64);
			l4cir.u64 = cvmx_read_csr_node(node, CVMX_PKO_L4_SQX_CIR(l5top.s.parent));
			CVMX_MT_CRC_DWORD(l4cir.u64);
			l3cir.u64 = cvmx_read_csr_node(node, CVMX_PKO_L3_SQX_CIR(l4top.s.parent));
			CVMX_MT_CRC_DWORD(l3cir.u64);

			l5pir.u64 = cvmx_read_csr_node(node, CVMX_PKO_L5_SQX_PIR(dqtop.s.parent));
			CVMX_MT_CRC_DWORD(l5pir.u64);
			l4pir.u64 = cvmx_read_csr_node(node, CVMX_PKO_L4_SQX_PIR(l5top.s.parent));
			CVMX_MT_CRC_DWORD(l4pir.u64);
			l3pir.u64 = cvmx_read_csr_node(node, CVMX_PKO_L3_SQX_PIR(l4top.s.parent));
			CVMX_MT_CRC_DWORD(l3pir.u64);
		}
		else {
			l5top.u64 = l4top.u64 = 0;
			l3top.u64 = cvmx_read_csr_node(node, CVMX_PKO_L3_SQX_TOPOLOGY(dqtop.s.parent));
			CVMX_MT_CRC_DWORD(l3top.u64);

			l5sch.u64 = l4sch.u64 = 0;
			l3sch.u64 = cvmx_read_csr_node(node, CVMX_PKO_L3_SQX_SCHEDULE(dqtop.s.parent));
			CVMX_MT_CRC_DWORD(l3sch.u64);

			l5shape.u64 = l4shape.u64 = 0;
			l3shape.u64 = cvmx_read_csr_node(node, CVMX_PKO_L3_SQX_SHAPE(dqtop.s.parent));
			CVMX_MT_CRC_DWORD(l3shape.u64);

			l5cir.u64 = l4cir.u64 = l5pir.u64 = l4pir.u64 = 0;
			l3cir.u64 = cvmx_read_csr_node(node, CVMX_PKO_L3_SQX_CIR(dqtop.s.parent));
			CVMX_MT_CRC_DWORD(l3cir.u64);
			l3pir.u64 = cvmx_read_csr_node(node, CVMX_PKO_L3_SQX_PIR(dqtop.s.parent));
			CVMX_MT_CRC_DWORD(l3pir.u64);
		}
		/* L2-L1 Queue Levels: */
		l2top.u64 = cvmx_read_csr_node(node, CVMX_PKO_L2_SQX_TOPOLOGY(l3top.s.parent));
		CVMX_MT_CRC_DWORD(l2top.u64);
		l1top.u64 = cvmx_read_csr_node(node, CVMX_PKO_L1_SQX_TOPOLOGY(l2top.s.parent));
		CVMX_MT_CRC_DWORD(l1top.u64);

		l2sch.u64 = cvmx_read_csr_node(node, CVMX_PKO_L2_SQX_SCHEDULE(l3top.s.parent));
		CVMX_MT_CRC_DWORD(l2sch.u64);
		l1sch.u64 = cvmx_read_csr_node(node, CVMX_PKO_L1_SQX_SCHEDULE(l2top.s.parent));
		CVMX_MT_CRC_DWORD(l1sch.u64);

		l2shape.u64 = cvmx_read_csr_node(node, CVMX_PKO_L2_SQX_SHAPE(l3top.s.parent));
		CVMX_MT_CRC_DWORD(l2shape.u64);
		l1shape.u64 = cvmx_read_csr_node(node, CVMX_PKO_L1_SQX_SHAPE(l2top.s.parent));
		CVMX_MT_CRC_DWORD(l1shape.u64);

		l2cir.u64 = cvmx_read_csr_node(node, CVMX_PKO_L2_SQX_CIR(l3top.s.parent));
		CVMX_MT_CRC_DWORD(l2cir.u64);
		l1cir.u64 = cvmx_read_csr_node(node, CVMX_PKO_L1_SQX_CIR(l2top.s.parent));
		CVMX_MT_CRC_DWORD(l1cir.u64);

		l2pir.u64 = cvmx_read_csr_node(node, CVMX_PKO_L2_SQX_PIR(l3top.s.parent));
		CVMX_MT_CRC_DWORD(l2pir.u64);

		l1link.u64 = cvmx_read_csr_node(node, CVMX_PKO_L1_SQX_LINK(l2top.s.parent));
		CVMX_MT_CRC_DWORD(l1link.u64 & (0x1Full << 44 | 0x2ull));

		/* MAC/FIFO Level: */
		if (l1top.s.link > nmacs) {
			maccfg.u64 = 0;
			sprintf(lines[0], "Undef");
		}
		else if (l1top.s.link == nmacs) {
			maccfg.u64 = 0;
			sprintf(lines[0], "NULL");
		}
		else {
			maccfg.u64 = cvmx_read_csr_node(node, CVMX_PKO_MACX_CFG(l1top.s.link));
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				sprintf(lines[0], "%s", pko_macmap[l1top.s.link][0]);
			else if (OCTEON_IS_MODEL(OCTEON_CN73XX))
				sprintf(lines[0], "%s", pko_macmap[l1top.s.link][1]);
			else if (OCTEON_IS_MODEL(OCTEON_CNF75XX))
				sprintf(lines[0], "%s", pko_macmap[l1top.s.link][2]);
		}
		sprintf(lines[1], "%d", maccfg.s.fifo_num);
		CVMX_MT_CRC_DWORD(maccfg.u64);

		if (chlvl.s.cc_level == 0) { /* Level 2 as the Channel Level?*/
			chcfg.u64 = cvmx_read_csr_node(node, CVMX_PKO_L3_L2_SQX_CHANNEL(l3top.s.parent));
			sprintf(lines[2], "%s", "--");
			sprintf(lines[3], "0x%X", chcfg.s.cc_channel);
		}
		else {
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				chcfg.u64 = cvmx_read_csr_node(node, CVMX_PKO_L3_L2_SQX_CHANNEL(l4top.s.parent));
			else
				chcfg.u64 = cvmx_read_csr_node(node, CVMX_PKO_L3_L2_SQX_CHANNEL(dqtop.s.parent));
			sprintf(lines[2], "%d", chcfg.s.cc_channel);
			sprintf(lines[3], "%s", "--");
		}
		CVMX_MT_CRC_DWORD(chcfg.u64);
		CVMX_MF_CRC_IV(crc32);
		if (crc32 == pcrc32)
			continue;

		/* Display DQ...FIFO Queue-Chain configuration: */
		if (queue > 0 && (queue - 1) != base)
			cvmx_printf("\nDQ(s) %02d-%02d -- same as DQ %02d\n",
				queue - 1, base + 1, base);
		pcrc32 = crc32;
		base = queue;
		cvmx_printf("DQ%d:\n", queue);
		PARPRINT(2, "Channel", "%*s%-*s%*s%*s%*s%*s%-*s%*s\n",
			PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8) + 1, lines[0],
			PKO_PRN_DPLEN(8) - 1, "", PKO_PRN_DPLEN(8), lines[2],
			PKO_PRN_DPLEN(8), lines[3], PKO_PRN_DPLEN(8), "",
			PKO_PRN_DPLEN(8), chcfg.s.cc_enable ? "CC-En" : "CC-Dis",
			PKO_PRN_DPLEN(8), l1link.s.cc_enable ? "LC-En" : "LC-Dis");
		if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
			ciren = dqcir.s.enable + l5cir.s.enable + l4cir.s.enable +
				l3cir.s.enable + l2cir.s.enable + l1cir.s.enable;
			piren = dqpir.s.enable + l5pir.s.enable + l4pir.s.enable +
				l3pir.s.enable + l2pir.s.enable;

			PARPRINT(2, "Path", "%*s%*d%*d%*d%*d%*d%*d%*s\n",
				PKO_PRN_DPLEN(8), "--", PKO_PRN_DPLEN(8), dqtop.s.parent,
				PKO_PRN_DPLEN(8), l5top.s.parent, PKO_PRN_DPLEN(8), l4top.s.parent,
				PKO_PRN_DPLEN(8), l3top.s.parent, PKO_PRN_DPLEN(8), l2top.s.parent,
				PKO_PRN_DPLEN(8), l1top.s.link, PKO_PRN_DPLEN(8), lines[1]);
			PARPRINT(2, "Prio-Anchor", "%*s%*d%*d%*d%*d%*d%*s%*s\n",
				PKO_PRN_DPLEN(8), "--", PKO_PRN_DPLEN(8), l5top.s.prio_anchor,
				PKO_PRN_DPLEN(8), l4top.s.prio_anchor, PKO_PRN_DPLEN(8), l3top.s.prio_anchor,
				PKO_PRN_DPLEN(8), l2top.s.prio_anchor, PKO_PRN_DPLEN(8), l1top.s.prio_anchor,
				PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), "");
			PARPRINT(2, "Prio", "%*d%*d%*d%*d%*d%*s%*s%*s\n",
				PKO_PRN_DPLEN(8), dqsch.s.prio, PKO_PRN_DPLEN(8), l5sch.s.prio,
				PKO_PRN_DPLEN(8), l4sch.s.prio, PKO_PRN_DPLEN(8), l3sch.s.prio,
				PKO_PRN_DPLEN(8), l2sch.s.prio, PKO_PRN_DPLEN(8), "--",
				PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), "");
			PARPRINT(2, "RR-Prio", "%*s%*d%*d%*d%*d%*d%*s%*s\n",
				PKO_PRN_DPLEN(8), "--", PKO_PRN_DPLEN(8), l5top.s.rr_prio,
				PKO_PRN_DPLEN(8), l4top.s.rr_prio, PKO_PRN_DPLEN(8), l3top.s.rr_prio,
				PKO_PRN_DPLEN(8), l2top.s.rr_prio, PKO_PRN_DPLEN(8), l1top.s.rr_prio,
				PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), "");
			PARPRINT(2, "RR-Quantum", "%*x%*x%*x%*x%*x%*x%*s%*s\n",
				PKO_PRN_DPLEN(8), dqsch.s.rr_quantum, PKO_PRN_DPLEN(8), l5sch.s.rr_quantum,
				PKO_PRN_DPLEN(8), l4sch.s.rr_quantum, PKO_PRN_DPLEN(8), l3sch.s.rr_quantum,
				PKO_PRN_DPLEN(8), l2sch.s.rr_quantum, PKO_PRN_DPLEN(8), l1sch.s.rr_quantum,
				PKO_PRN_DPLEN(8), "(hex)", PKO_PRN_DPLEN(8), "");
			PARPRINT(2, "Len.Disable", "%*d%*d%*d%*d%*d%*d%*s%*s\n",
				PKO_PRN_DPLEN(8), dqshape.s.length_disable,
				PKO_PRN_DPLEN(8), l5shape.s.length_disable,
				PKO_PRN_DPLEN(8), l4shape.s.length_disable,
				PKO_PRN_DPLEN(8), l3shape.s.length_disable,
				PKO_PRN_DPLEN(8), l2shape.s.length_disable,
				PKO_PRN_DPLEN(8), l1shape.s.length_disable,
				PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), "");
			PARPRINT(2, "Len.Adjust", "%*d%*d%*d%*d%*d%*d%*s%*s\n",
				PKO_PRN_DPLEN(8), dqshape.s.adjust, PKO_PRN_DPLEN(8), l5shape.s.adjust,
				PKO_PRN_DPLEN(8), l4shape.s.adjust, PKO_PRN_DPLEN(8), l3shape.s.adjust,
				PKO_PRN_DPLEN(8), l2shape.s.adjust, PKO_PRN_DPLEN(8), l1shape.s.adjust,
				PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), "");
			if (ciren || piren) {
				PARPRINT(2, "YELLOW Dis", "%*d%*d%*d%*d%*d%*s%*s%*s\n",
					PKO_PRN_DPLEN(8), dqshape.s.yellow_disable,
					PKO_PRN_DPLEN(8), l5shape.s.yellow_disable,
					PKO_PRN_DPLEN(8), l4shape.s.yellow_disable,
					PKO_PRN_DPLEN(8), l3shape.s.yellow_disable,
					PKO_PRN_DPLEN(8), l2shape.s.yellow_disable, PKO_PRN_DPLEN(8), "--",
					PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), "");
				PARPRINT(2, "RED Dis", "%*d%*d%*d%*d%*d%*s%*s%*s\n",
					PKO_PRN_DPLEN(8), dqshape.s.red_disable,
					PKO_PRN_DPLEN(8), l5shape.s.red_disable,
					PKO_PRN_DPLEN(8), l4shape.s.red_disable,
					PKO_PRN_DPLEN(8), l3shape.s.red_disable,
					PKO_PRN_DPLEN(8), l2shape.s.red_disable, PKO_PRN_DPLEN(8), "--",
					PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), "");
				PARPRINT(2, "RED Algo", "%*d%*d%*d%*d%*d%*s%*s%*s\n",
					PKO_PRN_DPLEN(8), dqshape.s.red_algo, PKO_PRN_DPLEN(8), l5shape.s.red_algo,
					PKO_PRN_DPLEN(8), l4shape.s.red_algo, PKO_PRN_DPLEN(8), l3shape.s.red_algo,
					PKO_PRN_DPLEN(8), l2shape.s.red_algo, PKO_PRN_DPLEN(8), "--",
					PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), "");
			}
			if (ciren) {
				PARPRINT(2, "CIR Enable", "%*d%*d%*d%*d%*d%*d%*s%*s\n",
					PKO_PRN_DPLEN(8), dqcir.s.enable, PKO_PRN_DPLEN(8), l5cir.s.enable,
					PKO_PRN_DPLEN(8), l4cir.s.enable, PKO_PRN_DPLEN(8), l3cir.s.enable,
					PKO_PRN_DPLEN(8), l2cir.s.enable, PKO_PRN_DPLEN(8), l1cir.s.enable,
					PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), "");
				PARPRINT(2, "CIR Burst", "%*d%*d%*d%*d%*d%*d%*s%*s\n",
					PKO_PRN_DPLEN(8), shaper_burst(dqcir.s.burst_mantissa, dqcir.s.burst_exponent),
					PKO_PRN_DPLEN(8), shaper_burst(l5cir.s.burst_mantissa, l5cir.s.burst_exponent),
					PKO_PRN_DPLEN(8), shaper_burst(l4cir.s.burst_mantissa, l4cir.s.burst_exponent),
					PKO_PRN_DPLEN(8), shaper_burst(l3cir.s.burst_mantissa, l3cir.s.burst_exponent),
					PKO_PRN_DPLEN(8), shaper_burst(l2cir.s.burst_mantissa, l2cir.s.burst_exponent),
					PKO_PRN_DPLEN(8), shaper_burst(l1cir.s.burst_mantissa, l1cir.s.burst_exponent),
					PKO_PRN_DPLEN(8), "(bytes)", PKO_PRN_DPLEN(8), "");
				PARPRINT(2, "CIR Rate", "%*d%*d%*d%*d%*d%*d%*s%*s\n",
					PKO_PRN_DPLEN(8), shaper_rate(dqsh_clk, dqcir.s.rate_mantissa,
						dqcir.s.rate_exponent, dqcir.s.rate_divider_exponent),
					PKO_PRN_DPLEN(8), shaper_rate(dqsh_clk, l5cir.s.rate_mantissa,
						l5cir.s.rate_exponent, l5cir.s.rate_divider_exponent),
					PKO_PRN_DPLEN(8), shaper_rate(dqsh_clk, l4cir.s.rate_mantissa,
						l4cir.s.rate_exponent, l4cir.s.rate_divider_exponent),
					PKO_PRN_DPLEN(8), shaper_rate(dqsh_clk, l3cir.s.rate_mantissa,
						l3cir.s.rate_exponent, l3cir.s.rate_divider_exponent),
					PKO_PRN_DPLEN(8), shaper_rate(dqsh_clk, l2cir.s.rate_mantissa,
						l2cir.s.rate_exponent, l2cir.s.rate_divider_exponent),
					PKO_PRN_DPLEN(8), shaper_rate(pqsh_clk, l1cir.s.rate_mantissa,
						l1cir.s.rate_exponent, l1cir.s.rate_divider_exponent),
					PKO_PRN_DPLEN(8), "(Mbps)", PKO_PRN_DPLEN(8), "");
			}
			if (piren) {
				PARPRINT(2, "PIR Enable", "%*d%*d%*d%*d%*d%*s%*s%*s\n",
					PKO_PRN_DPLEN(8), dqpir.s.enable, PKO_PRN_DPLEN(8), l5pir.s.enable,
					PKO_PRN_DPLEN(8), l4pir.s.enable, PKO_PRN_DPLEN(8), l3pir.s.enable,
					PKO_PRN_DPLEN(8), l2pir.s.enable, PKO_PRN_DPLEN(8), "--",
					PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), "");
				PARPRINT(2, "PIR Burst", "%*d%*d%*d%*d%*d%*s%*s%*s\n",
					PKO_PRN_DPLEN(8), shaper_burst(dqpir.s.burst_mantissa, dqpir.s.burst_exponent),
					PKO_PRN_DPLEN(8), shaper_burst(l5pir.s.burst_mantissa, l5pir.s.burst_exponent),
					PKO_PRN_DPLEN(8), shaper_burst(l4pir.s.burst_mantissa, l4pir.s.burst_exponent),
					PKO_PRN_DPLEN(8), shaper_burst(l3pir.s.burst_mantissa, l3pir.s.burst_exponent),
					PKO_PRN_DPLEN(8), shaper_burst(l2pir.s.burst_mantissa, l2pir.s.burst_exponent),
					PKO_PRN_DPLEN(8), "--", PKO_PRN_DPLEN(8), "(bytes)", PKO_PRN_DPLEN(8), "");
				PARPRINT(2, "PIR Rate", "%*d%*d%*d%*d%*d%*s%*s%*s\n",
					PKO_PRN_DPLEN(8), shaper_rate(dqsh_clk, dqpir.s.rate_mantissa,
						dqpir.s.rate_exponent, dqpir.s.rate_divider_exponent),
					PKO_PRN_DPLEN(8), shaper_rate(dqsh_clk, l5pir.s.rate_mantissa,
						l5pir.s.rate_exponent, l5pir.s.rate_divider_exponent),
					PKO_PRN_DPLEN(8), shaper_rate(dqsh_clk, l4pir.s.rate_mantissa,
						l4pir.s.rate_exponent, l4pir.s.rate_divider_exponent),
					PKO_PRN_DPLEN(8), shaper_rate(dqsh_clk, l3pir.s.rate_mantissa,
						l3pir.s.rate_exponent, l3pir.s.rate_divider_exponent),
					PKO_PRN_DPLEN(8), shaper_rate(dqsh_clk, l2pir.s.rate_mantissa,
						l2pir.s.rate_exponent, l2pir.s.rate_divider_exponent),
					PKO_PRN_DPLEN(8), "--", PKO_PRN_DPLEN(8), "(Mbps)", PKO_PRN_DPLEN(8), "");
			}
		}
		else {
			ciren = dqcir.s.enable + l3cir.s.enable + l2cir.s.enable + l1cir.s.enable;
			piren = dqpir.s.enable + l3pir.s.enable + l2pir.s.enable;

			PARPRINT(2, "Path", "%*s%*s%*s%*d%*d%*d%*d%*s\n",
				PKO_PRN_DPLEN(8), "--", PKO_PRN_DPLEN(8), "",
				PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), dqtop.s.parent,
				PKO_PRN_DPLEN(8), l3top.s.parent, PKO_PRN_DPLEN(8), l2top.s.parent,
				PKO_PRN_DPLEN(8), l1top.s.link, PKO_PRN_DPLEN(8), lines[1]);
			PARPRINT(2, "Prio-Anchor", "%*s%*s%*s%*d%*d%*d%*s%*s\n",
				PKO_PRN_DPLEN(8), "--", PKO_PRN_DPLEN(8), "",
				PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), l3top.s.prio_anchor,
				PKO_PRN_DPLEN(8), l2top.s.prio_anchor, PKO_PRN_DPLEN(8), l1top.s.prio_anchor,
				PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), "");
			PARPRINT(2, "Prio", "%*d%*s%*s%*d%*d%*s%*s%*s\n",
				PKO_PRN_DPLEN(8), dqsch.s.prio, PKO_PRN_DPLEN(8), "",
				PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), l3sch.s.prio,
				PKO_PRN_DPLEN(8), l2sch.s.prio, PKO_PRN_DPLEN(8), "--",
				PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), "");
			PARPRINT(2, "RR-Prio", "%*s%*s%*s%*d%*d%*d%*s%*s\n",
				PKO_PRN_DPLEN(8), "--", PKO_PRN_DPLEN(8), "",
				PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), l3top.s.rr_prio,
				PKO_PRN_DPLEN(8), l2top.s.rr_prio, PKO_PRN_DPLEN(8), l1top.s.rr_prio,
				PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), "");
			PARPRINT(2, "RR-Quantum", "%*x%*s%*s%*x%*x%*x%*s%*s\n",
				PKO_PRN_DPLEN(8), dqsch.s.rr_quantum, PKO_PRN_DPLEN(8), "",
				PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), l3sch.s.rr_quantum,
				PKO_PRN_DPLEN(8), l2sch.s.rr_quantum, PKO_PRN_DPLEN(8), l1sch.s.rr_quantum,
				PKO_PRN_DPLEN(8), "(hex)", PKO_PRN_DPLEN(8), "");
			PARPRINT(2, "Len.Disable", "%*d%*s%*s%*d%*d%*d%*s%*s\n",
				PKO_PRN_DPLEN(8), dqshape.s.length_disable, PKO_PRN_DPLEN(8), "",
				PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), l3shape.s.length_disable,
				PKO_PRN_DPLEN(8), l2shape.s.length_disable,
				PKO_PRN_DPLEN(8), l1shape.s.length_disable,
				PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), "");
			PARPRINT(2, "Len.Adjust", "%*d%*s%*s%*d%*d%*d%*s%*s\n",
				PKO_PRN_DPLEN(8), dqshape.s.adjust, PKO_PRN_DPLEN(8), "",
				PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), l3shape.s.adjust,
				PKO_PRN_DPLEN(8), l2shape.s.adjust, PKO_PRN_DPLEN(8), l1shape.s.adjust,
				PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), "");
			if (ciren || piren) {
				PARPRINT(2, "YELLOW Dis", "%*d%*s%*s%*d%*d%*s%*s%*s\n",
					PKO_PRN_DPLEN(8), dqshape.s.yellow_disable, PKO_PRN_DPLEN(8), "",
					PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), l3shape.s.yellow_disable,
					PKO_PRN_DPLEN(8), l2shape.s.yellow_disable, PKO_PRN_DPLEN(8), "--",
					PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), "");
				PARPRINT(2, "RED Dis", "%*d%*s%*s%*d%*d%*s%*s%*s\n",
					PKO_PRN_DPLEN(8), dqshape.s.red_disable, PKO_PRN_DPLEN(8), "",
					PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), l3shape.s.red_disable,
					PKO_PRN_DPLEN(8), l2shape.s.red_disable, PKO_PRN_DPLEN(8), "--",
					PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), "");
				PARPRINT(2, "RED Algo", "%*d%*s%*s%*d%*d%*s%*s%*s\n",
					PKO_PRN_DPLEN(8), dqshape.s.red_algo, PKO_PRN_DPLEN(8), "",
					PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), l3shape.s.red_algo,
					PKO_PRN_DPLEN(8), l2shape.s.red_algo, PKO_PRN_DPLEN(8), "--",
					PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), "");
			}
			if (ciren) {
				PARPRINT(2, "CIR Enable", "%*d%*s%*s%*d%*d%*d%*s%*s\n",
					PKO_PRN_DPLEN(8), dqcir.s.enable, PKO_PRN_DPLEN(8), "",
					PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), l3cir.s.enable,
					PKO_PRN_DPLEN(8), l2cir.s.enable, PKO_PRN_DPLEN(8), l1cir.s.enable,
					PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), "");
				PARPRINT(2, "CIR Burst", "%*d%*s%*s%*d%*d%*d%*s%*s\n",
					PKO_PRN_DPLEN(8), shaper_burst(dqcir.s.burst_mantissa, dqcir.s.burst_exponent),
					PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), "",
					PKO_PRN_DPLEN(8), shaper_burst(l3cir.s.burst_mantissa, l3cir.s.burst_exponent),
					PKO_PRN_DPLEN(8), shaper_burst(l2cir.s.burst_mantissa, l2cir.s.burst_exponent),
					PKO_PRN_DPLEN(8), shaper_burst(l1cir.s.burst_mantissa, l1cir.s.burst_exponent),
					PKO_PRN_DPLEN(8), "(bytes)", PKO_PRN_DPLEN(8), "");
				PARPRINT(2, "CIR Rate", "%*d%*s%*s%*d%*d%*d%*s%*s\n",
					PKO_PRN_DPLEN(8), shaper_rate(dqsh_clk, dqcir.s.rate_mantissa,
						dqcir.s.rate_exponent, dqcir.s.rate_divider_exponent),
					PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), "",
					PKO_PRN_DPLEN(8), shaper_rate(dqsh_clk, l3cir.s.rate_mantissa,
						l3cir.s.rate_exponent, l3cir.s.rate_divider_exponent),
					PKO_PRN_DPLEN(8), shaper_rate(dqsh_clk, l2cir.s.rate_mantissa,
						l2cir.s.rate_exponent, l2cir.s.rate_divider_exponent),
					PKO_PRN_DPLEN(8), shaper_rate(pqsh_clk, l1cir.s.rate_mantissa,
						l1cir.s.rate_exponent, l1cir.s.rate_divider_exponent),
					PKO_PRN_DPLEN(8), "(Mbps)", PKO_PRN_DPLEN(8), "");
			}
			if (piren) {
				PARPRINT(2, "PIR Enable", "%*d%*s%*s%*d%*d%*s%*s%*s\n",
					PKO_PRN_DPLEN(8), dqpir.s.enable, PKO_PRN_DPLEN(8), "",
					PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), l3pir.s.enable,
					PKO_PRN_DPLEN(8), l2pir.s.enable, PKO_PRN_DPLEN(8), "--",
					PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), "");
				PARPRINT(2, "PIR Burst", "%*d%*s%*s%*d%*d%*s%*s%*s\n",
					PKO_PRN_DPLEN(8), shaper_burst(dqpir.s.burst_mantissa, dqpir.s.burst_exponent),
					PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), "",
					PKO_PRN_DPLEN(8), shaper_burst(l3pir.s.burst_mantissa, l3pir.s.burst_exponent),
					PKO_PRN_DPLEN(8), shaper_burst(l2pir.s.burst_mantissa, l2pir.s.burst_exponent),
					PKO_PRN_DPLEN(8), "--", PKO_PRN_DPLEN(8), "(bytes)", PKO_PRN_DPLEN(8), "");
				PARPRINT(2, "PIR Rate", "%*d%*s%*s%*d%*d%*s%*s%*s\n",
					PKO_PRN_DPLEN(8), shaper_rate(dqsh_clk, dqpir.s.rate_mantissa,
						dqpir.s.rate_exponent, dqpir.s.rate_divider_exponent),
					PKO_PRN_DPLEN(8), "", PKO_PRN_DPLEN(8), "",
					PKO_PRN_DPLEN(8), shaper_rate(dqsh_clk, l3pir.s.rate_mantissa,
						l3pir.s.rate_exponent, l3pir.s.rate_divider_exponent),
					PKO_PRN_DPLEN(8), shaper_rate(dqsh_clk, l2pir.s.rate_mantissa,
						l2pir.s.rate_exponent, l2pir.s.rate_divider_exponent),
					PKO_PRN_DPLEN(8), "--", PKO_PRN_DPLEN(8), "(Mbps)", PKO_PRN_DPLEN(8), "");
			}
		}
	}
	if ((queue - 1) != base)
		cvmx_printf("\nDQ(s) %02d-%02d -- same as DQ %02d\n",
			queue - 1, base + 1, base);

	/* Display FIFO Groups: */
	DLMPRINT("FIFO Groups:");
	if (OCTEON_IS_MODEL(OCTEON_CN78XX))
		ngroups = 8;
	else
		ngroups = 5;
	for (group = 0; group < ngroups; group++) {
		cvmx_pko_ptgfx_cfg_t fgcfg;
		char *pfg_sizes[8] = {
			[0] = " 2.5  2.5  2.5  2.5",
			[1] = " 5.0  n/a  2.5  2.5",
			[2] = " 2.5  2.5  5.0  n/a",
			[3] = " 5.0  n/a  5.0  n/a",
			[4] = "10.0  n/a  n/a  n/a",
			[5] = "???", [6] = "???", [7] = "???"};
		char *pfg_rates[8] = {
			[0] = "6.25", [1] = "12.5", [2] = "25.0", [3] = "50.0",
			[4] = "100.0", [5] = "???", [6] = "???", [7] = "???"
		};
		fgcfg.u64 = cvmx_read_csr_node(node, CVMX_PKO_PTGFX_CFG(group));
		sprintf(lines[0], "Group %d: (FIFOs)", group);
		if (group == (ngroups - 1)) {
			sprintf(lines[1], "Virtual/NULL");
			sprintf(lines[2], "10.0");
		} else {
			sprintf(lines[1], "%4d %4d %4d %4d",
				group * 4, group * 4 + 1, group * 4 + 2, group * 4 + 3);
			sprintf(lines[2], "%s", pfg_sizes[fgcfg.s.size]);
		}
		PARPRINT(0, lines[0], "%*s\n", PKO_PRN_DPLEN(1), lines[1]);
		PARPRINT(2, "Size (KB)", "%*s\n", PKO_PRN_DPLEN(1), lines[2]);
		PARPRINT(2, "Rate (Gbps)", "%*s\n", PKO_PRN_DPLEN(1), pfg_rates[fgcfg.s.rate]);
	}
	return 0;
}

static int __cvmx_pko3_find_l1q_from_dq(int node, int dq)
{
	cvmx_pko_dqx_topology_t dqtop;
	cvmx_pko_l5_sqx_topology_t l5top;
	cvmx_pko_l4_sqx_topology_t l4top;
	cvmx_pko_l3_sqx_topology_t l3top;
	cvmx_pko_l2_sqx_topology_t l2top;

	dqtop.u64 = cvmx_read_csr_node(node, CVMX_PKO_DQX_TOPOLOGY(dq));
	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) { 
		l5top.u64 = cvmx_read_csr_node(node, CVMX_PKO_L5_SQX_TOPOLOGY(dqtop.s.parent));
		l4top.u64 = cvmx_read_csr_node(node, CVMX_PKO_L4_SQX_TOPOLOGY(l5top.s.parent));
		l3top.u64 = cvmx_read_csr_node(node, CVMX_PKO_L3_SQX_TOPOLOGY(l4top.s.parent));
	}
	else
		l3top.u64 = cvmx_read_csr_node(node, CVMX_PKO_L3_SQX_TOPOLOGY(dqtop.s.parent));

	l2top.u64 = cvmx_read_csr_node(node, CVMX_PKO_L2_SQX_TOPOLOGY(l3top.s.parent));
	return l2top.s.parent;
}

#undef PKO_PRN_HEADLEN
#define PKO_PRN_HEADLEN  36 
#undef PKO_PRN_DATALEN
#define PKO_PRN_DATALEN  44
int cvmx_helper_pko3_stats_dump(unsigned int node)
{
	int queue, nqueues, n, nmacs;
	cvmx_pko_dqx_packets_t dq_pkts;
	cvmx_pko_dqx_bytes_t dq_bytes;
	cvmx_pko_dqx_dropped_packets_t dq_drppkts;
	cvmx_pko_dqx_dropped_bytes_t dq_drpbytes;
	cvmx_pko_l1_sqx_dropped_packets_t l1_drppkts;
	cvmx_pko_l1_sqx_dropped_bytes_t l1_drpbytes;
	cvmx_pko_l1_sqx_red_packets_t l1_redpkts;
	cvmx_pko_l1_sqx_red_bytes_t l1_redbytes;
	cvmx_pko_l1_sqx_yellow_packets_t l1_yelpkts;
	cvmx_pko_l1_sqx_yellow_bytes_t l1_yelbytes;
	cvmx_pko_l1_sqx_green_packets_t l1_grnpkts;
	cvmx_pko_l1_sqx_green_bytes_t l1_grnbytes;
	cvmx_pko_l1_sqx_topology_t l1top;
	cvmx_pko_macx_cfg_t maccfg;
	char lines[4][256];

	memset(lines[3], '*', PKO_PRN_LINELEN);  lines[3][PKO_PRN_LINELEN] = '\0';
	cvmx_printf("\n%s\n", lines[3]);
	cvmx_printf("   PKO Statistics (Node %d)\n", node);
	cvmx_printf("%s\n", lines[3]);
	DLMPRINT("Descriptor Queues:");
	nqueues = cvmx_pko3_num_level_queues(CVMX_PKO_DESCR_QUEUES);
	for (queue = 0; queue < nqueues; queue++) {
		dq_pkts.u64 = cvmx_read_csr_node(node, CVMX_PKO_DQX_PACKETS(queue));
		dq_bytes.u64 = cvmx_read_csr_node(node, CVMX_PKO_DQX_BYTES(queue));
		dq_drppkts.u64 = cvmx_read_csr_node(node, CVMX_PKO_DQX_DROPPED_PACKETS(queue));
		dq_drpbytes.u64 = cvmx_read_csr_node(node, CVMX_PKO_DQX_DROPPED_BYTES(queue));
		n = dq_pkts.s.count + dq_bytes.s.count + dq_drppkts.s.count + dq_drpbytes.s.count;
		if (n == 0)
			continue;
		cvmx_printf("DQ%d => L1-SQ%d\n", queue, __cvmx_pko3_find_l1q_from_dq(node, queue));
		if (dq_pkts.s.count)
			PARPRINT(4, "Packets", "%*lld\n", PKO_PRN_DPLEN(1), (long long)dq_pkts.s.count);
		if (dq_bytes.s.count)
			PARPRINT(4, "Bytes", "%*lld\n", PKO_PRN_DPLEN(1), (long long)dq_bytes.s.count);
		if (dq_drppkts.s.count)
			PARPRINT(4, "Dropped Packets", "%*lld\n",
				PKO_PRN_DPLEN(1), (long long)dq_drppkts.s.count);
		if (dq_drpbytes.s.count)
			PARPRINT(4, "Dropped Bytes", "%*lld\n",
				PKO_PRN_DPLEN(1), (long long)dq_drpbytes.s.count);
	}
	DLMPRINT("Port(L1) Queues:");
	nqueues = cvmx_pko3_num_level_queues(CVMX_PKO_PORT_QUEUES);
	nmacs = __cvmx_pko3_num_macs();
	for (queue = 0; queue < nqueues; queue++) {
		l1_grnpkts.u64 = cvmx_read_csr_node(node, CVMX_PKO_L1_SQX_GREEN_PACKETS(queue));
		l1_grnbytes.u64 = cvmx_read_csr_node(node, CVMX_PKO_L1_SQX_GREEN_BYTES(queue));
		l1_yelpkts.u64 = cvmx_read_csr_node(node, CVMX_PKO_L1_SQX_YELLOW_PACKETS(queue));
		l1_yelbytes.u64 = cvmx_read_csr_node(node, CVMX_PKO_L1_SQX_YELLOW_BYTES(queue));
		l1_redpkts.u64 = cvmx_read_csr_node(node, CVMX_PKO_L1_SQX_RED_PACKETS(queue));
		l1_redbytes.u64 = cvmx_read_csr_node(node, CVMX_PKO_L1_SQX_RED_BYTES(queue));
		l1_drppkts.u64 = cvmx_read_csr_node(node, CVMX_PKO_L1_SQX_DROPPED_PACKETS(queue));
		l1_drpbytes.u64 = cvmx_read_csr_node(node, CVMX_PKO_L1_SQX_DROPPED_BYTES(queue));
		n = l1_grnpkts.s.count + l1_grnbytes.s.count + l1_yelpkts.s.count +
			l1_yelbytes.s.count + l1_redpkts.s.count + l1_redbytes.s.count +
			l1_drppkts.s.count + l1_drpbytes.s.count;
		if (n == 0)
			continue;

		l1top.u64 = cvmx_read_csr_node(node, CVMX_PKO_L1_SQX_TOPOLOGY(queue));
		if (l1top.s.link > nmacs) {
			maccfg.u64 = 0;
			sprintf(lines[0], "Undef");
		} else if (l1top.s.link == nmacs) {
			maccfg.u64 = 0;
			sprintf(lines[0], "NULL");
		} else {
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				sprintf(lines[0], "%s", pko_macmap[l1top.s.link][0]);
			else if (OCTEON_IS_MODEL(OCTEON_CN73XX))
				sprintf(lines[0], "%s", pko_macmap[l1top.s.link][1]);
			else if (OCTEON_IS_MODEL(OCTEON_CNF75XX))
				sprintf(lines[0], "%s", pko_macmap[l1top.s.link][2]);
			maccfg.u64 = cvmx_read_csr_node(node, CVMX_PKO_MACX_CFG(l1top.s.link));
		}
		cvmx_printf("L1-SQ%d => MAC%d (%s) => FIFO%d:\n",
			queue, l1top.s.link, lines[0], maccfg.s.fifo_num);
		if (l1_grnpkts.s.count)
			PARPRINT(4, "Green Packets", "%*lld\n",
				PKO_PRN_DPLEN(1), (long long)l1_grnpkts.s.count);
		if (l1_grnbytes.s.count)
			PARPRINT(4, "Green Bytes", "%*lld\n",
				PKO_PRN_DPLEN(1), (long long)l1_grnbytes.s.count);
		if (l1_yelpkts.s.count)
			PARPRINT(4, "Yellow Packets", "%*lld\n",
				PKO_PRN_DPLEN(1), (long long)l1_yelpkts.s.count);
		if (l1_yelbytes.s.count)
			PARPRINT(4, "Yellow Bytes", "%*lld\n",
				PKO_PRN_DPLEN(1), (long long)l1_yelbytes.s.count);
		if (l1_redpkts.s.count)
			PARPRINT(4, "Red Packets", "%*lld\n",
				PKO_PRN_DPLEN(1), (long long)l1_redpkts.s.count);
		if (l1_redbytes.s.count)
			PARPRINT(4, "Red Bytes", "%*lld\n",
				PKO_PRN_DPLEN(1), (long long)l1_redbytes.s.count);
		if (l1_drppkts.s.count)
			PARPRINT(4, "Dropped Packets", "%*lld\n",
				PKO_PRN_DPLEN(1), (long long)l1_drppkts.s.count);
		if (l1_drpbytes.s.count)
			PARPRINT(4, "Dropped Bytes", "%*lld\n",
				PKO_PRN_DPLEN(1), (long long)l1_drpbytes.s.count);
	}
	return 0;
}
void cvmx_helper_pko3_stats_clear(unsigned int node)
{
	int queue, nqueues;

	nqueues = cvmx_pko3_num_level_queues(CVMX_PKO_DESCR_QUEUES);
	for (queue = 0; queue < nqueues; queue++) {
		cvmx_write_csr_node(node, CVMX_PKO_DQX_PACKETS(queue), 0ull);
		cvmx_write_csr_node(node, CVMX_PKO_DQX_BYTES(queue), 0ull);
		cvmx_write_csr_node(node, CVMX_PKO_DQX_DROPPED_PACKETS(queue), 0ull);
		cvmx_write_csr_node(node, CVMX_PKO_DQX_DROPPED_BYTES(queue), 0ull);
	}
	nqueues = cvmx_pko3_num_level_queues(CVMX_PKO_PORT_QUEUES);
	for (queue = 0; queue < nqueues; queue++) {
		cvmx_write_csr_node(node, CVMX_PKO_L1_SQX_GREEN_PACKETS(queue), 0ull);
		cvmx_write_csr_node(node, CVMX_PKO_L1_SQX_GREEN_BYTES(queue), 0ull);
		cvmx_write_csr_node(node, CVMX_PKO_L1_SQX_YELLOW_PACKETS(queue), 0ull);
		cvmx_write_csr_node(node, CVMX_PKO_L1_SQX_YELLOW_BYTES(queue), 0ull);
		cvmx_write_csr_node(node, CVMX_PKO_L1_SQX_RED_PACKETS(queue), 0ull);
		cvmx_write_csr_node(node, CVMX_PKO_L1_SQX_RED_BYTES(queue), 0ull);
		cvmx_write_csr_node(node, CVMX_PKO_L1_SQX_DROPPED_PACKETS(queue), 0ull);
		cvmx_write_csr_node(node, CVMX_PKO_L1_SQX_DROPPED_BYTES(queue), 0ull);
	}
	cvmx_write_csr_node(node, CVMX_PKO_PEB_ERR_INT, ~0ull);
	cvmx_write_csr_node(node, CVMX_PKO_NCB_INT, ~0ull);
}

#endif /* CVMX_DUMP_PKO */

