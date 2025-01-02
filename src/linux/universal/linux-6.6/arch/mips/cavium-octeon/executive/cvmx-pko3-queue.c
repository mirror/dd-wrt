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
 * File version info: $Rev:$
 *
 */

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <linux/module.h>
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-pko3.h>
#include <asm/octeon/cvmx-pko3-resources.h>
#include <asm/octeon/cvmx-pko3-queue.h>
#include <asm/octeon/cvmx-helper-pko3.h>
#include <asm/octeon/cvmx-bootmem.h>
#include <asm/octeon/cvmx-clock.h>
#else
#include "cvmx.h"
#include "cvmx-pko3.h"
#include "cvmx-pko3-resources.h"
#include "cvmx-pko3-queue.h"
#include "cvmx-helper-pko3.h"
#include "cvmx-bootmem.h"
#endif

/* Smalles Round-Robin quantum to use +1 */
#define	CVMX_PKO3_RR_QUANTUM_MIN	0x10

static int debug = 0;	/* 1 for basic, 2 for detailed trace */

struct cvmx_pko3_dq {
#ifdef __BIG_ENDIAN_BITFIELD
	unsigned	dq_count :6;	/* Number of descriptor queues */
	unsigned	dq_base :10;	/* Descriptor queue start number */
#define	CVMX_PKO3_SWIZZLE_IPD	0x0
#else
	unsigned	dq_base :10;	/* Descriptor queue start number */
	unsigned	dq_count :6;	/* Number of descriptor queues */

#define	CVMX_PKO3_SWIZZLE_IPD	0x3
#endif
};

/*
 * @INTERNAL
 * Descriptor Queue to IPD port mapping table.
 *
 * This pointer is per-core, contains the virtual address
 * of a global named block which has 2^12 entries per each
 * possible node.
 */
CVMX_TLS struct cvmx_pko3_dq *__cvmx_pko3_dq_table;

int cvmx_pko3_get_queue_base(int ipd_port)
{
	struct cvmx_pko3_dq *dq_table;
	int ret = -1;
	unsigned i;
	struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(ipd_port);

	/* get per-node table */
	if (cvmx_unlikely(__cvmx_pko3_dq_table == NULL))
		__cvmx_pko3_dq_table_setup();

	i = CVMX_PKO3_SWIZZLE_IPD ^ xp.port;

	/* get per-node table */
	dq_table = __cvmx_pko3_dq_table + CVMX_PKO3_IPD_NUM_MAX * xp.node;

	if (cvmx_likely(dq_table[i].dq_count > 0))
		ret = xp.node << 10 | dq_table[i].dq_base;
	else if (debug)
		cvmx_printf("ERROR: %s: no queues for ipd_port=%#x\n",
			__func__, ipd_port);

	return ret;
}
EXPORT_SYMBOL(cvmx_pko3_get_queue_base);

int cvmx_pko3_get_queue_num(int ipd_port)
{
	struct cvmx_pko3_dq *dq_table;
	int ret = -1;
	unsigned i;
	struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(ipd_port);

	/* get per-node table */
	if (cvmx_unlikely(__cvmx_pko3_dq_table == NULL))
		__cvmx_pko3_dq_table_setup();

	i = CVMX_PKO3_SWIZZLE_IPD ^ xp.port;

	/* get per-node table */
	dq_table = __cvmx_pko3_dq_table + CVMX_PKO3_IPD_NUM_MAX * xp.node;

	if (cvmx_likely(dq_table[i].dq_count > 0))
		ret = dq_table[i].dq_count;
	else if (debug)
		cvmx_dprintf("ERROR: %s: no queues for ipd_port=%#x\n",
			__func__, ipd_port);

	return ret;
}

/**
 * Get L1/Port Queue number assigned to interface port.
 *
 * @param xiface is interface number.
 * @param index is port index.
 */
int cvmx_pko3_get_port_queue(int xiface, int index)
{
	int queue;
	cvmx_pko_l1_sqx_topology_t qtop;
	int mac = __cvmx_pko3_get_mac_num(xiface, index);
	int nqueues = cvmx_pko3_num_level_queues(CVMX_PKO_PORT_QUEUES);
	cvmx_xiface_t xi = cvmx_helper_xiface_to_node_interface(xiface);

	for (queue = 0; queue < nqueues; queue++) {
		qtop.u64 = cvmx_read_csr_node(xi.node, CVMX_PKO_L1_SQX_TOPOLOGY(queue));
		if (qtop.s.link == mac)
			break;
	}
	if (queue >= nqueues)
		return -1;
	return queue;
}

/**
 * @INTERNAL
 *
 * Initialize port/dq table contents
 */
static void __cvmx_pko3_dq_table_init(void *ptr)
{
	unsigned size = sizeof(struct cvmx_pko3_dq) * CVMX_PKO3_IPD_NUM_MAX * CVMX_MAX_NODES;

	memset(ptr, 0, size);
}

/**
 * @INTERNAL
 *
 * Find or allocate global port/dq map table
 * which is a named table, contains entries for
 * all possible OCI nodes.
 *
 * The table global pointer is stored in core-local variable
 * so that every core will call this function once, on first use.
 */
int __cvmx_pko3_dq_table_setup(void)
{
	void *ptr;

	ptr = cvmx_bootmem_alloc_named_range_once(
		/* size */
		sizeof(struct cvmx_pko3_dq) * CVMX_PKO3_IPD_NUM_MAX * CVMX_MAX_NODES,
		/* min_addr, max_addr, align */
		0ull, 0ull, sizeof(struct cvmx_pko3_dq),
		/* name */
		"cvmx_pko3_global_dq_table",
		__cvmx_pko3_dq_table_init);

	if (debug)
		cvmx_dprintf("%s: dq_table_ptr=%p\n", __func__, ptr);

	if(ptr == NULL)
		return -1;

	__cvmx_pko3_dq_table = ptr;
	return 0;
}

/*
 * @INTERNAL
 * Register a range of Descriptor Queues with an interface port
 *
 * This function populates the DQ-to-IPD translation table
 * used by the application to retreive the DQ range (typically ordered
 * by priority) for a given IPD-port, which is either a physical port,
 * or a channel on a channelized interface (i.e. ILK).
 *
 * @param xiface is the physical interface number
 * @param index is either a physical port on an interface
 *        or a channel of an ILK interface
 * @param dq_base is the first Descriptor Queue number in a consecutive range
 * @param dq_count is the number of consecutive Descriptor Queues leading
 *        the same channel or port.
 *
 * Only a consecutive range of Descriptor Queues can be associated with any
 * given channel/port, and usually they are ordered from most to least
 * in terms of scheduling priority.
 *
 * Note: thus function only populates the node-local translation table.
 * NOTE: This function would be cleaner if it had a single ipd_port argument
 *
 * @returns 0 on success, -1 on failure.
 */
int __cvmx_pko3_ipd_dq_register(int xiface, int index,
		unsigned dq_base, unsigned dq_count)
{
	struct cvmx_pko3_dq *dq_table;
	int ipd_port;
	unsigned i;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	struct cvmx_xport xp;

        if(__cvmx_helper_xiface_is_null(xiface))
		ipd_port = cvmx_helper_node_to_ipd_port(xi.node, CVMX_PKO3_IPD_PORT_NULL);
	else {
		int p;
		p = cvmx_helper_get_ipd_port(xiface, index);
		if (p < 0) {
			cvmx_printf("ERROR: %s: xiface %#x has no IPD port\n",
			__func__, xiface);
			return -1;
		}
		ipd_port = p;
	}

	xp = cvmx_helper_ipd_port_to_xport(ipd_port);

	i = CVMX_PKO3_SWIZZLE_IPD ^ xp.port;

	/* get per-node table */
	if(__cvmx_pko3_dq_table == NULL)
		__cvmx_pko3_dq_table_setup();

	dq_table = __cvmx_pko3_dq_table + CVMX_PKO3_IPD_NUM_MAX * xi.node;

	if(debug)
		cvmx_dprintf("%s: ipd_port=%#x ix=%#x dq %u cnt %u\n",
			__func__, ipd_port, i, dq_base, dq_count);

	/* Check the IPD port has not already been configured */
	if(dq_table[i].dq_count > 0 ) {
		cvmx_printf("%s: ERROR: IPD %#x already registered\n",
			__func__, ipd_port);
		return -1;
	}

	/* Store DQ# range in the queue lookup table */
	dq_table[i].dq_base = dq_base;
	dq_table[i].dq_count = dq_count;

	return 0;
}

/**
 * @INTERNAL
 *
 * Unregister DQs associated with CHAN_E (IPD port)
 *
 * NOTE: This function would be cleaner if it had a single ipd_port argument
 */
int __cvmx_pko3_ipd_dq_unregister(int xiface, int index)
{
	struct cvmx_pko3_dq *dq_table;
	int ipd_port;
	unsigned i;
	struct cvmx_xport xp;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

        if(__cvmx_helper_xiface_is_null(xiface))
		ipd_port = cvmx_helper_node_to_ipd_port(xi.node, CVMX_PKO3_IPD_PORT_NULL);
	else {
		int p;
		p = cvmx_helper_get_ipd_port(xiface, index);
		if (p < 0) {
			cvmx_printf("ERROR: %s: xiface %#x has no IPD port\n",
			__func__, xiface);
			return -1;
		}
		ipd_port = p;
	}

	xp = cvmx_helper_ipd_port_to_xport(ipd_port);

	i = CVMX_PKO3_SWIZZLE_IPD ^ xp.port;

	/* get per-node table */
	if(__cvmx_pko3_dq_table == NULL)
		__cvmx_pko3_dq_table_setup();

	/* get per-node table */
	dq_table = __cvmx_pko3_dq_table + CVMX_PKO3_IPD_NUM_MAX * xi.node;

	if (dq_table[i].dq_count == 0) {
		cvmx_printf("WARNING: %s:ipd=%#x already released\n",
			__func__, ipd_port);
		return -1;
	}

	if(debug)
		cvmx_dprintf("%s:ipd_port=%#x release dq %u cnt %u\n",
			     __func__, ipd_port,
			     dq_table[i].dq_base,
			     dq_table[i].dq_count);

	dq_table[i].dq_count = 0;

	return 0;
}

/*
 * @INTERNAL
 * Convert normal CHAN_E (i.e. IPD port) value to compressed channel form
 * that is used to populate PKO_LUT.
 *
 * Note: This code may be model specific.
 */
static int cvmx_pko3_chan_2_xchan(uint16_t ipd_port)
{
	uint16_t xchan;
	uint8_t off;
	static const uint8_t *xchan_base = NULL;
	static const uint8_t xchan_base_cn78xx[16] = {
		/* IPD 0x000 */ 0x3c0 >> 4,	/* LBK */
		/* IPD 0x100 */ 0x380 >> 4,	/* DPI */
		/* IPD 0x200 */ 0xfff >> 4,	/* not used */
		/* IPD 0x300 */ 0xfff >> 4,	/* not used */
		/* IPD 0x400 */ 0x000 >> 4,	/* ILK0 */
		/* IPD 0x500 */ 0x100 >> 4,	/* ILK1 */
		/* IPD 0x600 */ 0xfff >> 4,	/* not used */
		/* IPD 0x700 */ 0xfff >> 4,	/* not used */
		/* IPD 0x800 */ 0x200 >> 4,	/* BGX0 */
		/* IPD 0x900 */ 0x240 >> 4,	/* BGX1 */
		/* IPD 0xa00 */ 0x280 >> 4,	/* BGX2 */
		/* IPD 0xb00 */ 0x2c0 >> 4,	/* BGX3 */
		/* IPD 0xc00 */ 0x300 >> 4,	/* BGX4 */
		/* IPD 0xd00 */ 0x340 >> 4,	/* BGX5 */
		/* IPD 0xe00 */ 0xfff >> 4,	/* not used */
		/* IPD 0xf00 */ 0xfff >> 4	/* not used */
	};
	static const uint8_t xchan_base_cn73xx[16] = {
		/* IPD 0x000 */ 0x0c0 >> 4,	/* LBK */
		/* IPD 0x100 */ 0x100 >> 4,	/* DPI */
		/* IPD 0x200 */ 0xfff >> 4,	/* not used */
		/* IPD 0x300 */ 0xfff >> 4,	/* not used */
		/* IPD 0x400 */ 0xfff >> 4,	/* not used */
		/* IPD 0x500 */ 0xfff >> 4,	/* not used */
		/* IPD 0x600 */ 0xfff >> 4,	/* not used */
		/* IPD 0x700 */ 0xfff >> 4,	/* not used */
		/* IPD 0x800 */ 0x000 >> 4,	/* BGX0 */
		/* IPD 0x900 */ 0x040 >> 4,	/* BGX1 */
		/* IPD 0xa00 */ 0x080 >> 4,	/* BGX2 */
		/* IPD 0xb00 */ 0xfff >> 4,	/* not used */
		/* IPD 0xc00 */ 0xfff >> 4,	/* not used */
		/* IPD 0xd00 */ 0xfff >> 4,	/* not used */
		/* IPD 0xe00 */ 0xfff >> 4,	/* not used */
		/* IPD 0xf00 */ 0xfff >> 4	/* not used */
	};
	static const uint8_t xchan_base_cn75xx[16] = {
		/* IPD 0x000 */ 0x040 >> 4,	/* LBK */
		/* IPD 0x100 */ 0x080 >> 4,	/* DPI */
		/* IPD 0x200 */ 0xeee >> 4,	/* SRIO0  noop */
		/* IPD 0x300 */ 0xfff >> 4,	/* not used */
		/* IPD 0x400 */ 0xfff >> 4,	/* not used */
		/* IPD 0x500 */ 0xfff >> 4,	/* not used */
		/* IPD 0x600 */ 0xfff >> 4,	/* not used */
		/* IPD 0x700 */ 0xfff >> 4,	/* not used */
		/* IPD 0x800 */ 0x000 >> 4,	/* BGX0 */
		/* IPD 0x900 */ 0xfff >> 4,	/* not used */
		/* IPD 0xa00 */ 0xfff >> 4,	/* not used */
		/* IPD 0xb00 */ 0xfff >> 4,	/* not used */
		/* IPD 0xc00 */ 0xfff >> 4,	/* not used */
		/* IPD 0xd00 */ 0xfff >> 4,	/* not used */
		/* IPD 0xe00 */ 0xfff >> 4,	/* not used */
		/* IPD 0xf00 */ 0xfff >> 4	/* not used */
	};

        if (OCTEON_IS_MODEL(OCTEON_CN73XX))
		xchan_base = xchan_base_cn73xx;
        if (OCTEON_IS_MODEL(OCTEON_CNF75XX))
		xchan_base = xchan_base_cn75xx;
        if (OCTEON_IS_MODEL(OCTEON_CN78XX))
		xchan_base = xchan_base_cn78xx;

	if (xchan_base == NULL)
		return -1;

	xchan = ipd_port >> 8;

	/* ILKx, DPI has 8 bits logical channels, others just 6 */
	if (((xchan & 0xfe) == 0x04) || xchan == 0x01)
		off = ipd_port & 0xff;
	else
		off = ipd_port & 0x3f;

	xchan = xchan_base[ xchan & 0xF ];

	if(xchan == 0xff)
		return -1;	/* Invalid IPD_PORT */
	else if (xchan == 0xee)
		return -2;	/* LUT not used */
	else
		return (xchan << 4) | off;
}

/*
 * Map channel number in PKO
 *
 * @param node is to specify the node to which this configuration is applied.
 * @param pq_num specifies the Port Queue (i.e. L1) queue number.
 * @param l2_l3_q_num  specifies L2/L3 queue number.
 * @param channel specifies the channel number to map to the queue.
 *
 * The channel assignment applies to L2 or L3 Shaper Queues depending
 * on the setting of channel credit level.
 *
 * @return returns none.
 */
void cvmx_pko3_map_channel(unsigned node,
	unsigned pq_num, unsigned l2_l3_q_num, uint16_t channel)
{
	union cvmx_pko_l3_l2_sqx_channel sqx_channel;
	cvmx_pko_lutx_t lutx;
	int xchan;

	sqx_channel.u64 = cvmx_read_csr_node(node,
		CVMX_PKO_L3_L2_SQX_CHANNEL(l2_l3_q_num));

	sqx_channel.s.cc_channel = channel;

	cvmx_write_csr_node(node,
		CVMX_PKO_L3_L2_SQX_CHANNEL(l2_l3_q_num), sqx_channel.u64);

	/* Convert CHAN_E into compressed channel */
	xchan =  cvmx_pko3_chan_2_xchan(channel);

	if (debug)
		cvmx_dprintf("%s: ipd_port=%#x xchan=%#x\n", __func__, channel, xchan);

	if(xchan < 0) {
		if (xchan == -1)
			cvmx_printf("%s: ERROR: channel %#x not recognized\n",
				__func__, channel);
		return;
	}

	lutx.u64 = 0;
	lutx.s.valid = 1;
	lutx.s.pq_idx = pq_num;
	lutx.s.queue_number = l2_l3_q_num;

	cvmx_write_csr_node(node, CVMX_PKO_LUTX(xchan), lutx.u64);

	if (debug)
		cvmx_dprintf("%s: channel %#x (compressed=%#x) mapped "
				"L2/L3 SQ=%u, PQ=%u\n",
			__func__, channel, xchan, l2_l3_q_num, pq_num);
}

/*
 * @INTERNAL
 * This function configures port queue scheduling and topology parameters
 * in hardware.
 *
 * @param node is to specify the node to which this configuration is applied.
 * @param port_queue is the port queue number to be configured.
 * @param mac_num is the mac number of the mac that will be tied to this port_queue.
 */
static void cvmx_pko_configure_port_queue(int node, int port_queue, int mac_num)
{
	cvmx_pko_l1_sqx_topology_t pko_l1_topology;
	cvmx_pko_l1_sqx_shape_t pko_l1_shape;
	cvmx_pko_l1_sqx_link_t pko_l1_link;

	pko_l1_topology.u64 = 0;
	pko_l1_topology.s.link = mac_num;
	cvmx_write_csr_node(node, CVMX_PKO_L1_SQX_TOPOLOGY(port_queue), pko_l1_topology.u64);

	pko_l1_shape.u64 = 0;
	pko_l1_shape.s.link = mac_num;
	cvmx_write_csr_node(node, CVMX_PKO_L1_SQX_SHAPE(port_queue), pko_l1_shape.u64);

	pko_l1_link.u64 = 0;
	pko_l1_link.s.link = mac_num;
	cvmx_write_csr_node(node, CVMX_PKO_L1_SQX_LINK(port_queue), pko_l1_link.u64);
}

/*
 * @INTERNAL
 * This function configures level 2 queues scheduling and topology parameters
 * in hardware.
 *
 * @param node is to specify the node to which this configuration is applied.
 * @param queue is the level3 queue number to be configured.
 * @param parent_queue is the parent queue at next level for this l3 queue.
 * @param prio is this queue's priority in parent's scheduler.
 * @param rr_quantum is this queue's round robin quantum value.
 * @param child_base is the first child queue number in the static prioriy childs.
 * @param child_rr_prio is the round robin childs priority.
 */
static void cvmx_pko_configure_l2_queue(int node, int queue, int parent_queue,
					       int prio, int rr_quantum,
					       int child_base, int child_rr_prio)
{
	cvmx_pko_l2_sqx_schedule_t pko_sq_sched;
	cvmx_pko_l2_sqx_topology_t pko_child_topology;
	cvmx_pko_l1_sqx_topology_t pko_parent_topology;

	/* parent topology configuration */
	pko_parent_topology.u64 = cvmx_read_csr_node(node,
			CVMX_PKO_L1_SQX_TOPOLOGY(parent_queue));
	pko_parent_topology.s.prio_anchor = child_base;
	pko_parent_topology.s.rr_prio = child_rr_prio;
	cvmx_write_csr_node(node,
			CVMX_PKO_L1_SQX_TOPOLOGY(parent_queue),
			pko_parent_topology.u64);

	if (debug>1) cvmx_dprintf("CVMX_PKO_L1_SQX_TOPOLOGY(%u): "
		"PRIO_ANCHOR=%u PARENT=%u\n",
		parent_queue, pko_parent_topology.s.prio_anchor,
		pko_parent_topology.s.link);

	/* scheduler configuration for this sq in the parent queue */
	pko_sq_sched.u64 = 0;
	pko_sq_sched.s.prio = prio;
	pko_sq_sched.s.rr_quantum = rr_quantum;
	cvmx_write_csr_node(node, CVMX_PKO_L2_SQX_SCHEDULE(queue), pko_sq_sched.u64);

	/* child topology configuration */
	pko_child_topology.u64 = 0;
	pko_child_topology.s.parent = parent_queue;
	cvmx_write_csr_node(node, CVMX_PKO_L2_SQX_TOPOLOGY(queue), pko_child_topology.u64);

}


/*
 * @INTERNAL
 * This function configures level 3 queues scheduling and topology parameters
 * in hardware.
 *
 * @param node is to specify the node to which this configuration is applied.
 * @param queue is the level3 queue number to be configured.
 * @param parent_queue is the parent queue at next level for this l3 queue.
 * @param prio is this queue's priority in parent's scheduler.
 * @param rr_quantum is this queue's round robin quantum value.
 * @param child_base is the first child queue number in the static prioriy childs.
 * @param child_rr_prio is the round robin childs priority.
 */
static void cvmx_pko_configure_l3_queue(int node, int queue, int parent_queue,
					       int prio, int rr_quantum,
					       int child_base, int child_rr_prio)
{
	cvmx_pko_l3_sqx_schedule_t pko_sq_sched;
	cvmx_pko_l3_sqx_topology_t pko_child_topology;
	cvmx_pko_l2_sqx_topology_t pko_parent_topology;

	/* parent topology configuration */
	pko_parent_topology.u64 = cvmx_read_csr_node(node,
			CVMX_PKO_L2_SQX_TOPOLOGY(parent_queue));
	pko_parent_topology.s.prio_anchor = child_base;
	pko_parent_topology.s.rr_prio = child_rr_prio;
	cvmx_write_csr_node(node,
			CVMX_PKO_L2_SQX_TOPOLOGY(parent_queue),
			pko_parent_topology.u64);

	if (debug>1) cvmx_dprintf("CVMX_PKO_L2_SQX_TOPOLOGY(%u): "
		"PRIO_ANCHOR=%u PARENT=%u\n",
		parent_queue, pko_parent_topology.s.prio_anchor,
		pko_parent_topology.s.parent);

	/* scheduler configuration for this sq in the parent queue */
	pko_sq_sched.u64 = 0;
	pko_sq_sched.s.prio = prio;
	pko_sq_sched.s.rr_quantum = rr_quantum;
	cvmx_write_csr_node(node, CVMX_PKO_L3_SQX_SCHEDULE(queue), pko_sq_sched.u64);

	/* child topology configuration */
	pko_child_topology.u64 = 0;
	pko_child_topology.s.parent = parent_queue;
	cvmx_write_csr_node(node, CVMX_PKO_L3_SQX_TOPOLOGY(queue), pko_child_topology.u64);

}

/*
 * @INTERNAL
 * This function configures level 4 queues scheduling and topology parameters
 * in hardware.
 *
 * @param node is to specify the node to which this configuration is applied.
 * @param queue is the level4 queue number to be configured.
 * @param parent_queue is the parent queue at next level for this l4 queue.
 * @param prio is this queue's priority in parent's scheduler.
 * @param rr_quantum is this queue's round robin quantum value.
 * @param child_base is the first child queue number in the static prioriy childs.
 * @param child_rr_prio is the round robin childs priority.
 */
static void cvmx_pko_configure_l4_queue(int node, int queue, int parent_queue,
					       int prio, int rr_quantum,
					       int child_base, int child_rr_prio)
{
	cvmx_pko_l4_sqx_schedule_t pko_sq_sched;
	cvmx_pko_l4_sqx_topology_t pko_child_topology;
	cvmx_pko_l3_sqx_topology_t pko_parent_topology;

	/* parent topology configuration */
	pko_parent_topology.u64 = cvmx_read_csr_node(node,
			CVMX_PKO_L3_SQX_TOPOLOGY(parent_queue));
	pko_parent_topology.s.prio_anchor = child_base;
	pko_parent_topology.s.rr_prio = child_rr_prio;
	cvmx_write_csr_node(node,
			CVMX_PKO_L3_SQX_TOPOLOGY(parent_queue),
			pko_parent_topology.u64);

	if (debug>1) cvmx_dprintf("CVMX_PKO_L3_SQX_TOPOLOGY(%u): "
		"PRIO_ANCHOR=%u PARENT=%u\n",
		parent_queue, pko_parent_topology.s.prio_anchor,
		pko_parent_topology.s.parent);

	/* scheduler configuration for this sq in the parent queue */
	pko_sq_sched.u64 = 0;
	pko_sq_sched.s.prio = prio;
	pko_sq_sched.s.rr_quantum = rr_quantum;
	cvmx_write_csr_node(node, CVMX_PKO_L4_SQX_SCHEDULE(queue), pko_sq_sched.u64);

	/* topology configuration */
	pko_child_topology.u64 = 0;
	pko_child_topology.s.parent = parent_queue;
	cvmx_write_csr_node(node, CVMX_PKO_L4_SQX_TOPOLOGY(queue), pko_child_topology.u64);
}

/*
 * @INTERNAL
 * This function configures level 5 queues scheduling and topology parameters
 * in hardware.
 *
 * @param node is to specify the node to which this configuration is applied.
 * @param queue is the level5 queue number to be configured.
 * @param parent_queue is the parent queue at next level for this l5 queue.
 * @param prio is this queue's priority in parent's scheduler.
 * @param rr_quantum is this queue's round robin quantum value.
 * @param child_base is the first child queue number in the static prioriy childs.
 * @param child_rr_prio is the round robin childs priority.
 */
static void cvmx_pko_configure_l5_queue(int node, int queue, int parent_queue,
					       int prio, int rr_quantum,
					       int child_base, int child_rr_prio)
{
	cvmx_pko_l5_sqx_schedule_t pko_sq_sched;
	cvmx_pko_l4_sqx_topology_t pko_parent_topology;
	cvmx_pko_l5_sqx_topology_t pko_child_topology;

	/* parent topology configuration */
	pko_parent_topology.u64 = cvmx_read_csr_node(node,
			CVMX_PKO_L4_SQX_TOPOLOGY(parent_queue));
	pko_parent_topology.s.prio_anchor = child_base;
	pko_parent_topology.s.rr_prio = child_rr_prio;
	cvmx_write_csr_node(node,
			CVMX_PKO_L4_SQX_TOPOLOGY(parent_queue),
			pko_parent_topology.u64);

	if (debug>1) cvmx_dprintf("CVMX_PKO_L4_SQX_TOPOLOGY(%u): "
		"PRIO_ANCHOR=%u PARENT=%u\n",
		parent_queue, pko_parent_topology.s.prio_anchor,
		pko_parent_topology.s.parent);

	/* scheduler configuration for this sq in the parent queue */
	pko_sq_sched.u64 = 0;
	pko_sq_sched.s.prio = prio;
	pko_sq_sched.s.rr_quantum = rr_quantum;
	cvmx_write_csr_node(node, CVMX_PKO_L5_SQX_SCHEDULE(queue), pko_sq_sched.u64);

	/* topology configuration */
	pko_child_topology.u64 = 0;
	pko_child_topology.s.parent = parent_queue;
	cvmx_write_csr_node(node, CVMX_PKO_L5_SQX_TOPOLOGY(queue), pko_child_topology.u64);
}

/*
 * @INTERNAL
 * This function configures descriptor queues scheduling and topology parameters
 * in hardware.
 *
 * @param node is to specify the node to which this configuration is applied.
 * @param dq is the descriptor queue number to be configured.
 * @param parent_queue is the parent queue at next level for this dq.
 * @param prio is this queue's priority in parent's scheduler.
 * @param rr_quantum is this queue's round robin quantum value.
 * @param child_base is the first child queue number in the static prioriy childs.
 * @param child_rr_prio is the round robin childs priority.
 */
static void cvmx_pko_configure_dq(int node, int dq, int parent_queue,
				int prio, int rr_quantum,
			       	int child_base, int child_rr_prio)
{
	cvmx_pko_dqx_schedule_t pko_dq_sched;
	cvmx_pko_dqx_topology_t pko_dq_topology;
	cvmx_pko_l5_sqx_topology_t pko_parent_topology;
	cvmx_pko_dqx_wm_ctl_t pko_dq_wm_ctl;
	unsigned long long parent_topology_reg;
	char lvl;

	if (debug)
		cvmx_dprintf("%s: dq %u parent %u child_base %u\n",
			     __func__, dq, parent_queue, child_base);

	if (__cvmx_pko3_sq_lvl_max() == CVMX_PKO_L5_QUEUES) {
		parent_topology_reg = CVMX_PKO_L5_SQX_TOPOLOGY(parent_queue);
		lvl = 5;
	} else if (__cvmx_pko3_sq_lvl_max() == CVMX_PKO_L3_QUEUES) {
		parent_topology_reg = CVMX_PKO_L3_SQX_TOPOLOGY(parent_queue);
		lvl = 3;
	} else
		return;

	if (debug)
		cvmx_dprintf("%s: parent_topology_reg=%#llx\n",
			__func__, parent_topology_reg);

	/* parent topology configuration */
	pko_parent_topology.u64 = cvmx_read_csr_node(node, parent_topology_reg);
	pko_parent_topology.s.prio_anchor = child_base;
	pko_parent_topology.s.rr_prio = child_rr_prio;
	cvmx_write_csr_node(node, parent_topology_reg,
			pko_parent_topology.u64);

	if (debug>1) cvmx_dprintf("CVMX_PKO_L%d_SQX_TOPOLOGY(%u): "
		"PRIO_ANCHOR=%u PARENT=%u\n", lvl,
		parent_queue, pko_parent_topology.s.prio_anchor,
		pko_parent_topology.s.parent);

	/* scheduler configuration for this dq in the parent queue */
	pko_dq_sched.u64 = 0;
	pko_dq_sched.s.prio = prio;
	pko_dq_sched.s.rr_quantum = rr_quantum;
	cvmx_write_csr_node(node, CVMX_PKO_DQX_SCHEDULE(dq), pko_dq_sched.u64);

	/* topology configuration */
	pko_dq_topology.u64 = 0;
	pko_dq_topology.s.parent = parent_queue;
	cvmx_write_csr_node(node, CVMX_PKO_DQX_TOPOLOGY(dq), pko_dq_topology.u64);

	/* configure for counting packets, not bytes at this level */
	pko_dq_wm_ctl.u64 = 0;
	pko_dq_wm_ctl.s.kind = 1;
	pko_dq_wm_ctl.s.enable = 0;
	cvmx_write_csr_node(node, CVMX_PKO_DQX_WM_CTL(dq), pko_dq_wm_ctl.u64);

	if (debug>1) {
		pko_dq_sched.u64 = cvmx_read_csr_node(node,
			CVMX_PKO_DQX_SCHEDULE(dq));
		pko_dq_topology.u64 = cvmx_read_csr_node(node,
			CVMX_PKO_DQX_TOPOLOGY(dq));
		cvmx_dprintf("CVMX_PKO_DQX_TOPOLOGY(%u)PARENT=%u "
			"CVMX_PKO_DQX_SCHEDULE(%u) PRIO=%u Q=%u\n",
			dq, pko_dq_topology.s.parent,
			dq, pko_dq_sched.s.prio, pko_dq_sched.s.rr_quantum);
	}
}


/*
 * @INTERNAL
 * The following structure selects the Scheduling Queue configuration
 * routine for each of the supported levels.
 * The initial content of the table will be setup in accordance
 * to the specific SoC model and its implemented resources
 */
struct pko3_cfg_tab_s {
	/* function pointer for to configure the given level, last=DQ */
	struct {
		uint8_t parent_level;
		void (*cfg_sq_func)(
			int node, int queue, int parent_queue,
			int prio, int rr_quantum,
			int child_base, int child_rr_prio);
	//XXX for debugging exagerated size
	} lvl[256];
};


static const struct pko3_cfg_tab_s pko3_cn78xx_cfg = {
	{
	[CVMX_PKO_L2_QUEUES] =
		{CVMX_PKO_PORT_QUEUES, cvmx_pko_configure_l2_queue },
	[CVMX_PKO_L3_QUEUES] =
		{CVMX_PKO_L2_QUEUES, cvmx_pko_configure_l3_queue },
	[CVMX_PKO_L4_QUEUES] =
		{CVMX_PKO_L3_QUEUES, cvmx_pko_configure_l4_queue },
	[CVMX_PKO_L5_QUEUES] =
		{CVMX_PKO_L4_QUEUES, cvmx_pko_configure_l5_queue },
	[CVMX_PKO_DESCR_QUEUES] =
		{CVMX_PKO_L5_QUEUES, cvmx_pko_configure_dq }
	}
};

static const struct pko3_cfg_tab_s pko3_cn73xx_cfg = {
	{
	[CVMX_PKO_L2_QUEUES] =
		{CVMX_PKO_PORT_QUEUES, cvmx_pko_configure_l2_queue },
	[CVMX_PKO_L3_QUEUES] =
		{CVMX_PKO_L2_QUEUES, cvmx_pko_configure_l3_queue },
	[CVMX_PKO_DESCR_QUEUES] =
		{CVMX_PKO_L3_QUEUES, cvmx_pko_configure_dq }
	}
};

/*
 * Configure Port Queue and its children Scheduler Queue
 *
 * Port Queues (a.k.a L1) are assigned 1-to-1 to MACs.
 * L2 Scheduler Queues are used for specifying channels, and thus there
 * could be multiple L2 SQs attached to a single L1 PQ, either in a
 * fair round-robin scheduling, or with static and/or round-robin priorities.
 *
 * @param node on which to operate
 * @param mac_num is the LMAC number to that is associated with the Port Queue,
 * @param pq_num is the number of the L1 PQ attached to the MAC
 *
 * @returns 0 on success, -1 on failure.
 */
int cvmx_pko3_pq_config(unsigned node, unsigned mac_num, unsigned pq_num)
{
	char b1[10];

	if(debug)
		cvmx_dprintf("%s: MAC%u -> %s\n",
			__func__, mac_num,
			 __cvmx_pko3_sq_str(b1, CVMX_PKO_PORT_QUEUES, pq_num));

	cvmx_pko_configure_port_queue(node, pq_num, mac_num);

	return 0;
}

/*
 * Configure L3 through L5 Scheduler Queues and Descriptor Queues
 *
 * The Scheduler Queues in Levels 3 to 5 and Descriptor Queues are
 * configured one-to-one or many-to-one to a single parent Scheduler
 * Queues. The level of the parent SQ is specified in an argument,
 * as well as the number of childer to attach to the specific parent.
 * The children can have fair round-robin or priority-based scheduling
 * when multiple children are assigned a single parent.
 *
 * @param node on which to operate
 * @param child_level  is the level of the child queue
 * @param parent_queue is the number of the parent Scheduler Queue
 * @param child_base is the number of the first child SQ or DQ to assign to
 * @param child_count is the number of consecutive children to assign
 * @param stat_prio_count is the priority setting for the children L2 SQs
 *
 * If <stat_prio_count> is -1, the Ln children will have equal Round-Robin
 * relationship with eachother. If <stat_prio_count> is 0, all Ln children
 * will be arranged in Weighted-Round-Robin, with the first having the most
 * precedence. If <stat_prio_count> is between 1 and 8, it indicates how
 * many children will have static priority settings (with the first having
 * the most precedence), with the remaining Ln children having WRR scheduling.
 *
 * @returns 0 on success, -1 on failure.
 *
 * Note: this function supports the configuration of node-local unit.
 */
int cvmx_pko3_sq_config_children(unsigned int node,
			enum cvmx_pko3_level_e child_level,
			unsigned parent_queue, unsigned child_base,
			unsigned child_count, int stat_prio_count)
{
	enum cvmx_pko3_level_e parent_level;
	unsigned num_elem = 0;
	unsigned rr_quantum, rr_count;
	unsigned child, prio, rr_prio;
	const struct pko3_cfg_tab_s *cfg_tbl = NULL;
	char b1[10], b2[10];

        if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		num_elem = NUM_ELEMENTS(pko3_cn78xx_cfg.lvl);
		cfg_tbl = &pko3_cn78xx_cfg;
	}
        if (OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
		num_elem = NUM_ELEMENTS(pko3_cn73xx_cfg.lvl);
		cfg_tbl = &pko3_cn73xx_cfg;
	}

	if (cfg_tbl == NULL || child_level >= num_elem) {
		cvmx_printf("ERROR: %s: model or level %#x invalid\n",
			__func__, child_level);
		return -1;
	}

	parent_level = cfg_tbl->lvl[child_level].parent_level;

	if (cfg_tbl->lvl[child_level].cfg_sq_func == NULL ||
	    cfg_tbl->lvl[child_level].parent_level == 0) {
		cvmx_printf("ERROR: %s: queue level %#x invalid\n",
			__func__, child_level);
		return -1;
	}

	/* First static priority is 0 - top precedence */
	prio = 0;

	if (stat_prio_count > (signed) child_count)
		stat_prio_count = child_count;

	/* Valid PRIO field is 0..9, limit maximum static priorities */
	if (stat_prio_count > 9)
		stat_prio_count = 9;

	/* Special case of a single child */
	if (child_count == 1) {
		rr_count = 0;
		rr_prio = 0xF;
	/* Special case for Fair-RR */
	} else if (stat_prio_count < 0) {
		rr_count = child_count;
		rr_prio = 0;
	} else {
		rr_count = child_count - stat_prio_count;
		rr_prio = stat_prio_count;
	}

	/* Compute highest RR_QUANTUM */
	if (stat_prio_count > 0)
		rr_quantum = CVMX_PKO3_RR_QUANTUM_MIN * rr_count;
	else
		rr_quantum = CVMX_PKO3_RR_QUANTUM_MIN;

	if(debug)
		cvmx_dprintf("%s: Parent %s child_base %u rr_pri %u\n",
		__func__, __cvmx_pko3_sq_str(b1, parent_level, parent_queue),
		child_base, rr_prio);

	/* Parent is configured with child */

	for(child = child_base; child < (child_base + child_count); child ++) {
		if (debug)
			cvmx_dprintf("%s: "
			    "Child %s of %s prio %u rr_quantum %#x\n",
			    __func__,
			    __cvmx_pko3_sq_str(b1, child_level, child),
			    __cvmx_pko3_sq_str(b2, parent_level, parent_queue),
			    prio, rr_quantum);

		cfg_tbl->lvl[child_level].cfg_sq_func(
			node, child, parent_queue, prio, rr_quantum,
			child_base, rr_prio);

		if (prio < rr_prio)
			prio ++;
		else if (stat_prio_count > 0)
			rr_quantum -= CVMX_PKO3_RR_QUANTUM_MIN;
	} /* for child */

	return 0;
}

/**
 * Convert bitrate and burst size to SQx_xIR register values
 *
 * @INTERNAL
 *
 * Common function to convert bit-rate (ie kilo-bits-per-sec)
 * and maximum burst (in bytes) values to PKO shaper register
 * format, that is a short-float type, with divisor.
 *
 * @param tclk is the time-wheel clock for the specific shaper
 * @param reg is a pointer to a register structure
 * @param rate_kbips is the requested bit rate in kilobits/sec
 * @param burst_bytes is the size of maximum burst in bytes
 *
 * @return A negative number means the transfer rate could
 * not be set within acceptable tolerance, and the actual
 * error in PPM is the negative of the returned value.
 * A positive value indicates that the bit rate was set
 * within acceptable tolerance, but the burst rate had an
 * error, which is returned in PPM.
 * A value of 0 means both measures were set within tolerance.
 *
 * Note that the bust error could be as a result of this function
 * enforcing the minimum MTU as the minimum burst size allowed.
 *
 */
static int cvmx_pko3_shaper_rate_compute(unsigned long tclk,
		cvmx_pko_l1_sqx_cir_t *reg,
		unsigned long rate_kbips, unsigned burst_bytes)
{
	const unsigned max_exp = 12;	/* maximum exponent */
	const unsigned tock_bytes_exp = 0;	/* tock rate in bytes */
	long long burst_v, rate_v;
	unsigned long long rate_tocks, burst_tocks;
	unsigned long min_burst;
	unsigned div_exp, mant, exp;
	unsigned long long tmp, fmax;

	if (debug)
		cvmx_dprintf("%s: tclk=%lu, rate=%lu kbps, burst=%u bytes\n",
			__func__, tclk, rate_kbips, burst_bytes);

	/* Convert API args into tocks: PSE native units */
	tmp = (1 << (3 + tock_bytes_exp))-1;
	tmp += rate_kbips;
	rate_tocks = (1000ULL * tmp) >> (3 + tock_bytes_exp);
	tmp = (1 << tock_bytes_exp) - 1;
	burst_tocks = (burst_bytes+tmp) >> tock_bytes_exp;

	/* Compute largest short-float that fits in register fields */
	fmax = CVMX_SHOFT_TO_U64((1<<CVMX_SHOFT_MANT_BITS)-1, max_exp);

	/* Find the biggest divider that has the short float fit */
	for (div_exp = 0; div_exp <= max_exp; div_exp++) {
		tmp = ((rate_tocks << div_exp) + (tclk / 2)) / tclk;
		if (tmp > fmax) {
			if (div_exp > 0)
				div_exp --;
			break;
		}
	}
	/* Make sure divider, rate are within valid range */
	if (div_exp > max_exp) {
		/* Minimum reached */
		div_exp = max_exp;
	} else if (div_exp == 0) {
		/* Maximum reached */
		if ((rate_tocks / tclk) > fmax)
			rate_tocks = fmax * tclk;
	}
	/* Store common divider */
	reg->s.rate_divider_exponent = div_exp;

	/* Burst register is the maximum accumulated credit count
	 * in bytes, which must not be less then the MTU, and
	 * should not be less than RATE/Tclk
	 */

	/* Find the minimum burst size needed for rate (burst ~ 4x rate) */
	min_burst = (rate_tocks << (div_exp + 4)) / tclk;

	/* Apply the minimum */
	if (burst_tocks < min_burst)
		burst_tocks = min_burst;

	/* Calculate the rate short float */
	tmp = (rate_tocks << (div_exp + 8)) / tclk;
	CVMX_SHOFT_FROM_U64(tmp, mant, exp);
	reg->s.rate_mantissa = mant;
	reg->s.rate_exponent = exp - 8;

	/* Calculate the BURST short float */
	tmp = (burst_tocks << 8);
	CVMX_SHOFT_FROM_U64(tmp, mant, exp);
	reg->s.burst_mantissa = mant;
	reg->s.burst_exponent = exp - 8 - 1;

	if (debug)
		cvmx_dprintf("%s: RATE=%llu BURST=%llu DIV_EXP=%d\n",
			__func__,
			CVMX_SHOFT_TO_U64(reg->s.rate_mantissa,
				reg->s.rate_exponent),
			CVMX_SHOFT_TO_U64(reg->s.burst_mantissa,
				(reg->s.burst_exponent + 1)), div_exp);

	/* Validate the resulting rate */
	rate_v = CVMX_SHOFT_TO_U64(reg->s.rate_mantissa,
				reg->s.rate_exponent) * tclk;
	/* Convert to kbips for comaring with argument */
	rate_v = (rate_v << (3+tock_bytes_exp)) /1000ULL;
	/* Finally apply divider for best accuracy */
	rate_v >>= div_exp;

	burst_v = CVMX_SHOFT_TO_U64(reg->s.burst_mantissa,
				(reg->s.burst_exponent + 1));
	/* Convert in additional bytes as in argument */
	burst_v = burst_v << (tock_bytes_exp);

	if (debug)
		cvmx_dprintf("%s: result rate=%'llu kbips burst=%llu bytes\n",
			__func__,rate_v, burst_v);

	/* Compute error in parts-per-million */
	rate_v = abs(rate_v - rate_kbips);
	burst_v = abs(burst_v - burst_bytes);

	if (debug)
		cvmx_dprintf("%s: diff rate=%llu burst=%llu ppm\n",
			__func__, rate_v, burst_v);

	rate_v = (rate_v * 1000000ULL) / rate_kbips;
	if (burst_bytes > 0)
		burst_v = (burst_v * 1000000ULL) / burst_bytes;
	else
		burst_v = 0;

	if (debug)
		cvmx_dprintf("%s: error rate=%llu burst=%llu ppm\n",
			__func__, rate_v, burst_v);

	/* Allow 1% error for CIR/PIR, and 5% for BURST */
	if (rate_v > 10000)
		return -rate_v;
	if (burst_v > 50000)
		return burst_v;

	return 0;
}

/**
 * Configure per-port CIR rate limit parameters
 *
 * This function configures rate limit at the L1/PQ level,
 * i.e. for an entire MAC or physical port.
 *
 * @param node The OCI node where the target port is located
 * @param pq_num The L1/PQ queue number for this setting
 * @param rate_kbips The desired throughput in kilo-bits-per-second
 * @param burst_bytes The size of a burst in bytes, at least MTU
 *
 * @return Returns zero if both settings applied within allowed tolerance,
 * otherwise the error is returned in parts-per-million.
 * 'rate_bps" error is e negative number, otherwise 'birst_rate' error
 * is returned as a positive integer.
 */
int cvmx_pko3_port_cir_set(unsigned node, unsigned pq_num,
		unsigned long rate_kbips, unsigned burst_bytes, int adj_bytes)
{
	unsigned long tclk;
	cvmx_pko_l1_sqx_cir_t sqx_cir;
	cvmx_pko_l1_sqx_shape_t shape;
	int rc;

	if (debug)
		cvmx_dprintf("%s: pq=%u rate=%lu kbps, burst=%u bytes\n",
			__func__, pq_num, rate_kbips, burst_bytes);

	sqx_cir.u64 = 0;

	/* When rate == 0, disable the shaper */
	if( rate_kbips == 0ULL) {
		sqx_cir.s.enable = 0;
		cvmx_write_csr_node(node, CVMX_PKO_L1_SQX_CIR(pq_num), sqx_cir.u64);
		return 0;
	}
	/* Compute time-wheel frequency */
	tclk = cvmx_pko3_pq_tw_clock_rate_node(node);

	/* Compute shaper values */
	rc = cvmx_pko3_shaper_rate_compute(tclk, &sqx_cir, rate_kbips, burst_bytes);

	/* Apply new settings */
	sqx_cir.s.enable = 1;
	sqx_cir.s. rate_divider_exponent = sqx_cir.s. rate_divider_exponent;
	sqx_cir.s. rate_mantissa  = sqx_cir.s. rate_mantissa;
	sqx_cir.s. rate_exponent  = sqx_cir.s. rate_exponent;
	sqx_cir.s. burst_mantissa = sqx_cir.s. burst_mantissa;
	sqx_cir.s. burst_exponent = sqx_cir.s. burst_exponent - 1;
	cvmx_write_csr_node(node, CVMX_PKO_L1_SQX_CIR(pq_num), sqx_cir.u64);

	shape.u64 = cvmx_read_csr_node(node, CVMX_PKO_L1_SQX_SHAPE(pq_num));
	shape.s.adjust = adj_bytes;
	cvmx_write_csr_node(node, CVMX_PKO_L1_SQX_SHAPE(pq_num), shape.u64);

	return rc;
}

/**
 * Configure per-queue CIR rate limit parameters
 *
 * This function configures rate limit at the descriptor queue level.
 *
 * @param node The OCI node where the target port is located
 * @param dq_num The descriptor queue number for this setting
 * @param rate_kbips The desired throughput in kilo-bits-per-second
 * @param burst_bytes The size of a burst in bytes, at least MTU
 *
 * @return Returns zero if both settings applied within allowed tolerance,
 * otherwise the error is returned in parts-per-million.
 * 'rate_bps" error is a negative number, otherwise 'birst_rate' error
 * is returned as a positive integer.
 */
int cvmx_pko3_dq_cir_set(unsigned node, unsigned dq_num,
		unsigned long rate_kbips, unsigned burst_bytes)
{
	unsigned long tclk;
	cvmx_pko_l1_sqx_cir_t sqx_cir;
	cvmx_pko_dqx_cir_t dqx_cir;
	int rc;

	dq_num &= cvmx_pko3_num_level_queues(CVMX_PKO_DESCR_QUEUES) - 1;
	if (debug)
		cvmx_dprintf("%s: dq=%u rate=%lu kbps, burst=%u bytes\n",
			__func__, dq_num, rate_kbips, burst_bytes);

	dqx_cir.u64 = 0;

	/* When rate == 0, disable the shaper */
	if( rate_kbips == 0ULL) {
		dqx_cir.s.enable = 0;
		cvmx_write_csr_node(node, CVMX_PKO_DQX_CIR(dq_num), dqx_cir.u64);
		return 0;
	}
	/* Compute time-wheel frequency */
	tclk = cvmx_pko3_dq_tw_clock_rate_node(node);

	/* Compute shaper values */
	rc = cvmx_pko3_shaper_rate_compute(tclk, &sqx_cir, rate_kbips, burst_bytes);

	/* Apply new settings */
	dqx_cir.s.enable = 1;
	dqx_cir.s. rate_divider_exponent = sqx_cir.s. rate_divider_exponent;
	dqx_cir.s. rate_mantissa  = sqx_cir.s. rate_mantissa;
	dqx_cir.s. rate_exponent  = sqx_cir.s. rate_exponent;
	dqx_cir.s. burst_mantissa = sqx_cir.s. burst_mantissa;
	dqx_cir.s. burst_exponent = sqx_cir.s. burst_exponent - 1;
	cvmx_write_csr_node(node, CVMX_PKO_DQX_CIR(dq_num), dqx_cir.u64);

	return rc;
}

/**
 * Configure per-queue PIR rate limit parameters
 *
 * This function configures rate limit at the descriptor queue level.
 *
 * @param node The OCI node where the target port is located
 * @param dq_num The descriptor queue number for this setting
 * @param rate_kbips The desired throughput in kilo-bits-per-second
 * @param burst_bytes The size of a burst in bytes, at least MTU
 *
 * @return Returns zero if both settings applied within allowed tolerance,
 * otherwise the error is returned in parts-per-million.
 * 'rate_bps" error is a negative number, otherwise 'birst_rate' error
 * is returned as a positive integer.
 */
int cvmx_pko3_dq_pir_set(unsigned node, unsigned dq_num,
		unsigned long rate_kbips, unsigned burst_bytes)
{
	unsigned long tclk;
	cvmx_pko_l1_sqx_cir_t sqx_cir;
	cvmx_pko_dqx_pir_t dqx_pir;
	int rc;

	dq_num &= cvmx_pko3_num_level_queues(CVMX_PKO_DESCR_QUEUES) - 1;
	if (debug)
		cvmx_dprintf("%s: dq=%u rate=%lu kbps, burst=%u bytes\n",
			__func__, dq_num, rate_kbips, burst_bytes);

	dqx_pir.u64 = 0;

	/* When rate == 0, disable the shaper */
	if( rate_kbips == 0ULL) {
		dqx_pir.s.enable = 0;
		cvmx_write_csr_node(node, CVMX_PKO_DQX_PIR(dq_num), dqx_pir.u64);
		return 0;
	}
	/* Compute time-wheel frequency */
	tclk = cvmx_pko3_dq_tw_clock_rate_node(node);

	/* Compute shaper values */
	rc = cvmx_pko3_shaper_rate_compute(tclk, &sqx_cir, rate_kbips, burst_bytes);

	/* Apply new settings */
	dqx_pir.s.enable = 1;
	dqx_pir.s. rate_divider_exponent = sqx_cir.s. rate_divider_exponent;
	dqx_pir.s. rate_mantissa  = sqx_cir.s. rate_mantissa;
	dqx_pir.s. rate_exponent  = sqx_cir.s. rate_exponent;
	dqx_pir.s. burst_mantissa = sqx_cir.s. burst_mantissa;
	dqx_pir.s. burst_exponent = sqx_cir.s. burst_exponent - 1;
	cvmx_write_csr_node(node, CVMX_PKO_DQX_PIR(dq_num), dqx_pir.u64);

	return rc;
}

/**
 * Configure per-queue treatment of excess traffic
 *
 * The default and most sensible behavior is to stall the packets
 * colored Red (i.e. exceeding the PIR rate in full 3-color mode).
 * There is also the option to discard excess traffic, which is
 * the desired action for some applications that do not rely on
 * back-pressure flow control.
 * The shaper may be programmed to pass the RED packets onwards,
 * which may be useful it the color is translated to a change
 * in packet priority on egress.
 *
 * @param node The OCI node where the target port is located
 * @param dq_num The descriptor queue number for this setting
 * @param red_act The action required for all packets in excess of PIR
 * @param len_adjust A 2's complement 8 bit value to add/subtract from
 * packet length for the purpose of shaping calculations, e.g.
 * a value of -14 will subtract the length of the Ethernet header
 * and hence only account IP packet size.
 *
 * @return N/A
 */
void cvmx_pko3_dq_red(unsigned node, unsigned dq_num, red_action_t red_act,
	int8_t len_adjust)
{
	cvmx_pko_dqx_shape_t dqx_shape;

	dq_num &= cvmx_pko3_num_level_queues(CVMX_PKO_DESCR_QUEUES) - 1;
	dqx_shape.u64 = 0;

	if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)) {
		if (len_adjust < 0)
			len_adjust = 0;
	}
	dqx_shape.s.adjust = len_adjust;

	switch(red_act) {
	default:
	case CVMX_PKO3_SHAPE_RED_STALL:
		dqx_shape.s.red_algo = 0x0;
		break;
	case CVMX_PKO3_SHAPE_RED_DISCARD:
		dqx_shape.s.red_algo = 0x3;
		break;
	case CVMX_PKO3_SHAPE_RED_PASS:
		dqx_shape.s.red_algo = 0x1;
		break;
	}
	cvmx_write_csr_node(node, CVMX_PKO_DQX_SHAPE(dq_num), dqx_shape.u64);
}

