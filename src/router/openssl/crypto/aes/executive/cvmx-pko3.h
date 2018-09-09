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

#ifndef __CVMX_PKO3_H__
#define __CVMX_PKO3_H__

#ifdef	__cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <asm/octeon/cvmx-pko-defs.h>
#include <asm/octeon/cvmx-helper.h>
#include <asm/octeon/cvmx-pko3-queue.h>
#include <asm/octeon/cvmx-ilk.h>
#else
#include "cvmx-pko-defs.h"
#include "cvmx-pko3-queue.h"
#include "cvmx-helper.h"
#include "cvmx-ilk.h"
#endif

/* dwords are from 1-16 */
/* scratch line for LMT operations */
#define CVMX_PKO_LMTLINE 2ull
#define CVMX_PKO_LMTLINE_START_BYTE (CVMX_PKO_LMTLINE) * (CVMX_CACHE_LINE_SIZE)
#define CVMX_LMTDMA_ADDR(dwords) (0xffffffffffffa400ull + ((dwords-1)<<3))
#define CVMX_LMTDMA_WORD(dword) (CVMX_PKO_LMTLINE_START_BYTE + (dword << 3))

#define CVMX_PKO_NUM_PORT_QUEUES    32
#define CVMX_PKO_NUM_L2_QUEUES     512
#define CVMX_PKO_NUM_L3_QUEUES     512
#define CVMX_PKO_NUM_L4_QUEUES    1024
#define CVMX_PKO_NUM_L5_QUEUES    1024
#define CVMX_PKO_NUM_DESCR_QUEUES 1024

#define CVMX_PKO_MAX_MACS 28

enum {
	CVMX_PKO_DQ_SEND = 0ULL,
	CVMX_PKO_DQ_OPEN = 1ULL,
	CVMX_PKO_DQ_CLOSE = 2ULL,
	CVMX_PKO_DQ_QUERY = 3ULL
};

union cvmx_pko_query_rtn_s {
	uint64_t u64;
	struct {
		CVMX_BITFIELD_FIELD(uint64_t dqstatus	: 4,
		CVMX_BITFIELD_FIELD(uint64_t rsvd_50_59	:10,
		CVMX_BITFIELD_FIELD(uint64_t dqop	: 2,
		CVMX_BITFIELD_FIELD(uint64_t depth	:48,
			))));
	} s;
};
typedef union cvmx_pko_query_rtn_s cvmx_pko_query_rtn_s_t;
/* Three bit codes */
#define CVMX_PKO_SENDSUBDC_LINK		0x0
#define CVMX_PKO_SENDSUBDC_GATHER	0x1
#define CVMX_PKO_SENDSUBDC_JUMP		0x2
/* Four bit codes */
#define CVMX_PKO_SENDSUBDC_FREE		0x9
#define CVMX_PKO_SENDSUBDC_WORK		0xA
#define CVMX_PKO_SENDSUBDC_AURA		0xB
#define CVMX_PKO_SENDSUBDC_MEM		0xC
#define CVMX_PKO_SENDSUBDC_EXT		0xD
#define CVMX_PKO_SENDSUBDC_CRC		0xE
#define CVMX_PKO_SENDSUBDC_IMM		0xF

/* pko buf ptr */
union cvmx_pko_buf_ptr {
	uint64_t u64;
	struct {
		CVMX_BITFIELD_FIELD(uint64_t size	:16,
		CVMX_BITFIELD_FIELD(uint64_t subdc3	: 3,
		CVMX_BITFIELD_FIELD(uint64_t i		: 1,
		CVMX_BITFIELD_FIELD(uint64_t rsvd_42_43	: 2,
		CVMX_BITFIELD_FIELD(uint64_t addr	:42,
			)))));
	} s;
};
typedef union cvmx_pko_buf_ptr cvmx_pko_buf_ptr_t;

#define cvmx_pko_send_gather cvmx_pko_buf_ptr
#define cvmx_pko_send_link cvmx_pko_buf_ptr

/* pko command descriptor */
union cvmx_pko_send_hdr {
	uint64_t u64;
	struct {
		CVMX_BITFIELD_FIELD(uint64_t rsvd_60_63	:4,
		CVMX_BITFIELD_FIELD(uint64_t aura	:12,
		CVMX_BITFIELD_FIELD(uint64_t ckl4	:2,
		CVMX_BITFIELD_FIELD(uint64_t ckl3	:1,
		CVMX_BITFIELD_FIELD(uint64_t ds		:1,
		CVMX_BITFIELD_FIELD(uint64_t le		:1,
		CVMX_BITFIELD_FIELD(uint64_t n2		:1,
		CVMX_BITFIELD_FIELD(uint64_t ii		:1,
		CVMX_BITFIELD_FIELD(uint64_t df		:1,
		CVMX_BITFIELD_FIELD(uint64_t rsvd_39	:1,
		CVMX_BITFIELD_FIELD(uint64_t format	:7,
		CVMX_BITFIELD_FIELD(uint64_t l4ptr	:8,
		CVMX_BITFIELD_FIELD(uint64_t l3ptr	:8,
		CVMX_BITFIELD_FIELD(uint64_t total	:16,
			))))))))))))));
	} s;
};
typedef union cvmx_pko_send_hdr cvmx_pko_send_hdr_t;

union cvmx_pko_send_work {
	uint64_t u64;
	struct {
		CVMX_BITFIELD_FIELD(uint64_t rsvd_62_63	:2,
		CVMX_BITFIELD_FIELD(uint64_t grp	:10,
		CVMX_BITFIELD_FIELD(uint64_t tt		:2,
		CVMX_BITFIELD_FIELD(uint64_t rsvd_48_49	:2,
		CVMX_BITFIELD_FIELD(uint64_t subdc4	:4,
		CVMX_BITFIELD_FIELD(uint64_t rsvd_42_43	:2,
		CVMX_BITFIELD_FIELD(uint64_t addr	:42,
			)))))));
	} s;
};

union cvmx_pko_send_dma {
	uint64_t u64;
	struct {
		CVMX_BITFIELD_FIELD(uint64_t scraddr	: 8,
		CVMX_BITFIELD_FIELD(uint64_t rtnlen	: 8,
		CVMX_BITFIELD_FIELD(uint64_t did	: 8,
		CVMX_BITFIELD_FIELD(uint64_t node	: 4,
		CVMX_BITFIELD_FIELD(uint64_t rsvd_26_35	: 10,
		CVMX_BITFIELD_FIELD(uint64_t dq		: 10,
		CVMX_BITFIELD_FIELD(uint64_t rsvd_0_15	: 16,
			)))))));
	} s;
};
typedef union cvmx_pko_send_dma cvmx_pko_send_dma_t;

static inline uint64_t build_mask(uint64_t bits)
{
    return ~((~0x0ull) << bits);
}

/*
 * This function gets PKO mac num for a interface/port.
 *
 * @param interface is the interface number.
 * @param port is the port number.
 * @return returns mac number if successful or -1 on failure.
 */
static inline int __cvmx_pko_get_mac_num(int interface, int port)
{
	cvmx_helper_interface_mode_t mode;
	int interface_index;

	mode = cvmx_helper_interface_get_mode(interface);
	switch (mode) {
		case CVMX_HELPER_INTERFACE_MODE_SGMII:
		case CVMX_HELPER_INTERFACE_MODE_RXAUI:
		case CVMX_HELPER_INTERFACE_MODE_XAUI:
			return (4 + 4 * interface + port);
		case CVMX_HELPER_INTERFACE_MODE_LOOP:
			return 0;
		case CVMX_HELPER_INTERFACE_MODE_NPI:
			return 1;
		case CVMX_HELPER_INTERFACE_MODE_ILK:
			interface_index = (interface - CVMX_ILK_GBL_BASE());
			return (2 + interface_index);
		default:
			return -1;
	}
}

/*
 * Configure Channel credit level in PKO.
 *
 * @param node is to specify the node to which this configuration is applied.
 * @param level specifies the level at which pko channel queues will be configured,
 *              level : 0 -> L2, level : 1 -> L3 queues.
 * @return returns 0 if successful and -1 on failure.
 */
static inline int cvmx_pko_setup_channel_credit_level(int node, int level)
{
	union cvmx_pko_channel_level channel_level;

	if (level != 0 || level != 1)
		return -1;

	channel_level.u64 = 0;
	channel_level.s.cc_level = level;
	cvmx_write_csr_node(node, CVMX_PKO_CHANNEL_LEVEL, channel_level.u64);

	return 0;

}

/*
 * Map channel number to PKO queue(L2 or L3 queues depending on channel credit
 * level).
 *
 * @param node is to specify the node to which this configuration is applied.
 * @param queue specifies the l2 or l3 queue depending on channel credit level.
 * @param channel specifies the channel number to map to the queue.
 * @return returns none.
 */
static inline void cvmx_pko_map_channel(int node, int queue, int channel)
{
	union cvmx_pko_l3_l2_sqx_channel sqx_channel;

	sqx_channel.u64 = cvmx_read_csr_node(node, CVMX_PKO_L3_L2_SQX_CHANNEL(queue));

	sqx_channel.s.cc_channel = channel;

	cvmx_write_csr_node(node, CVMX_PKO_L3_L2_SQX_CHANNEL(queue), sqx_channel.u64);

}

/*
 * This function configures port queue scheduling and topology parameters
 * in hardware.
 *
 * @param node is to specify the node to which this configuration is applied.
 * @param port_queue is the port queue number to be configured.
 * @param child_base is the first child queue number in the static prioriy childs.
 * @param child_rr_prio is the round robin childs priority.
 * @param mac_num is the mac number of the mac that will be tied to this port_queue.
 * @return returns none.
 */
static inline void cvmx_pko_configure_port_queue(int node, int port_queue,
		 				 int child_base, int child_rr_prio,
						 int mac_num)
{
	cvmx_pko_l1_sqx_topology_t pko_l1_topology;
	cvmx_pko_l1_sqx_shape_t pko_l1_shape;
	cvmx_pko_l1_sqx_link_t pko_l1_link;

	pko_l1_topology.u64 = 0;
	pko_l1_topology.s.prio_anchor = child_base;
	pko_l1_topology.s.link = mac_num;
	pko_l1_topology.s.rr_prio = child_rr_prio;
	cvmx_write_csr_node(node, CVMX_PKO_L1_SQX_TOPOLOGY(port_queue), pko_l1_topology.u64);

	pko_l1_shape.u64 = 0;
	pko_l1_shape.s.link = mac_num;
	cvmx_write_csr_node(node, CVMX_PKO_L1_SQX_SHAPE(port_queue), pko_l1_shape.u64);

	pko_l1_link.u64 = 0;
	pko_l1_link.s.link = mac_num;
	cvmx_write_csr_node(node, CVMX_PKO_L1_SQX_LINK(port_queue), pko_l1_link.u64);
}

/*
 * This function configures level 2 queues scheduling and topology parameters
 * in hardware.
 *
 * @param node is to specify the node to which this configuration is applied.
 * @param queue is the level2 queue number to be configured.
 * @param parent_queue is the parent queue at next level for this l2 queue.
 * @param prio is this queue's priority in parent's scheduler.
 * @param rr_quantum is this queue's round robin quantum value.
 * @param child_base is the first child queue number in the static prioriy childs.
 * @param child_rr_prio is the round robin childs priority.
 * @return returns none.
 */
static inline void cvmx_pko_configure_l2_queue(int node, int queue, int parent_queue,
					       int prio, int rr_quantum,
					       int child_base, int child_rr_prio)
{
	cvmx_pko_l2_sqx_schedule_t pko_sq_sched;
	cvmx_pko_l2_sqx_topology_t pko_sq_topology;

	/* scheduler configuration for this sq in the parent queue */
	pko_sq_sched.u64 = 0;
	pko_sq_sched.s.prio = prio;
	pko_sq_sched.s.rr_quantum = rr_quantum;
	cvmx_write_csr_node(node, CVMX_PKO_L2_SQX_SCHEDULE(queue), pko_sq_sched.u64);

	/* topology configuration */
	pko_sq_topology.u64 = 0;
	pko_sq_topology.s.parent = parent_queue;
	pko_sq_topology.s.prio_anchor = child_base;
	pko_sq_topology.s.rr_prio = child_rr_prio;
	cvmx_write_csr_node(node, CVMX_PKO_L2_SQX_TOPOLOGY(queue), pko_sq_topology.u64);

}

/*
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
 * @return returns none.
 */
static inline void cvmx_pko_configure_l3_queue(int node, int queue, int parent_queue,
					       int prio, int rr_quantum,
					       int child_base, int child_rr_prio)
{
	cvmx_pko_l3_sqx_schedule_t pko_sq_sched;
	cvmx_pko_l3_sqx_topology_t pko_sq_topology;

	/* scheduler configuration for this sq in the parent queue */
	pko_sq_sched.u64 = 0;
	pko_sq_sched.s.prio = prio;
	pko_sq_sched.s.rr_quantum = rr_quantum;
	cvmx_write_csr_node(node, CVMX_PKO_L3_SQX_SCHEDULE(queue), pko_sq_sched.u64);

	/* topology configuration */
	pko_sq_topology.u64 = 0;
	pko_sq_topology.s.parent = parent_queue;
	pko_sq_topology.s.prio_anchor = child_base;
	pko_sq_topology.s.rr_prio = child_rr_prio;
	cvmx_write_csr_node(node, CVMX_PKO_L3_SQX_TOPOLOGY(queue), pko_sq_topology.u64);

}

/*
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
 * @return returns none.
 */
static inline void cvmx_pko_configure_l4_queue(int node, int queue, int parent_queue,
					       int prio, int rr_quantum,
					       int child_base, int child_rr_prio)
{
	cvmx_pko_l4_sqx_schedule_t pko_sq_sched;
	cvmx_pko_l4_sqx_topology_t pko_sq_topology;

	/* scheduler configuration for this sq in the parent queue */
	pko_sq_sched.u64 = 0;
	pko_sq_sched.s.prio = prio;
	pko_sq_sched.s.rr_quantum = rr_quantum;
	cvmx_write_csr_node(node, CVMX_PKO_L4_SQX_SCHEDULE(queue), pko_sq_sched.u64);

	/* topology configuration */
	pko_sq_topology.u64 = 0;
	pko_sq_topology.s.parent = parent_queue;
	pko_sq_topology.s.prio_anchor = child_base;
	pko_sq_topology.s.rr_prio = child_rr_prio;
	cvmx_write_csr_node(node, CVMX_PKO_L4_SQX_TOPOLOGY(queue), pko_sq_topology.u64);
}

/*
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
 * @return returns none.
 */
static inline void cvmx_pko_configure_l5_queue(int node, int queue, int parent_queue,
					       int prio, int rr_quantum,
					       int child_base, int child_rr_prio)
{
	cvmx_pko_l5_sqx_schedule_t pko_sq_sched;
	cvmx_pko_l5_sqx_topology_t pko_sq_topology;

	/* scheduler configuration for this sq in the parent queue */
	pko_sq_sched.u64 = 0;
	pko_sq_sched.s.prio = prio;
	pko_sq_sched.s.rr_quantum = rr_quantum;
	cvmx_write_csr_node(node, CVMX_PKO_L5_SQX_SCHEDULE(queue), pko_sq_sched.u64);

	/* topology configuration */
	pko_sq_topology.u64 = 0;
	pko_sq_topology.s.parent = parent_queue;
	pko_sq_topology.s.prio_anchor = child_base;
	pko_sq_topology.s.rr_prio = child_rr_prio;
	cvmx_write_csr_node(node, CVMX_PKO_L5_SQX_TOPOLOGY(queue), pko_sq_topology.u64);
}

/*
 * This function configures descriptor queues scheduling and topology parameters
 * in hardware.
 *
 * @param node is to specify the node to which this configuration is applied.
 * @param dq is the descriptor queue number to be configured.
 * @param parent_queue is the parent queue at next level for this dq.
 * @param prio is this queue's priority in parent's scheduler.
 * @param rr_quantum is this queue's round robin quantum value.
 * @return returns none.
 */
static inline void cvmx_pko_configure_dq(int node, int dq, int parent_queue,
					 int prio, int rr_quantum)
{
	cvmx_pko_dqx_schedule_t pko_dq_sched;
	cvmx_pko_dqx_topology_t pko_dq_topology;

	/* scheduler configuration for this dq in the parent queue */
	pko_dq_sched.u64 = 0;
	pko_dq_sched.s.prio = prio;
	pko_dq_sched.s.rr_quantum = rr_quantum;
	cvmx_write_csr_node(node, CVMX_PKO_DQX_SCHEDULE(dq), pko_dq_sched.u64);

	/* topology configuration */
	pko_dq_topology.u64 = 0;
	pko_dq_topology.s.parent = parent_queue;
	cvmx_write_csr_node(node, CVMX_PKO_DQX_TOPOLOGY(dq), pko_dq_topology.u64);

}

/*
 * Open configured descriptor queues before queueing packets into them.
 *
 * @param node is to specify the node to which this configuration is applied.
 * @param dq is the descriptor queue number to be opened.
 * @return returns 0 on sucess or -1 on failure.
 */
static inline int cvmx_pko_dq_open(int node, int dq)
{
	uint64_t pko_query_ld_addr = 0;
	cvmx_pko_query_rtn_s_t pko_status;

	pko_query_ld_addr |= (uint64_t)dq << 16;
	pko_query_ld_addr |= (uint64_t)CVMX_PKO_DQ_OPEN << 32; /* open */
	pko_query_ld_addr |= (uint64_t)node << 36;
	pko_query_ld_addr |= 0x51ull << 40;
	pko_query_ld_addr |= 1ull << 48;
	pko_query_ld_addr |= 1ull << 63;

	pko_status.u64 = cvmx_read64_uint64(pko_query_ld_addr);
	if (pko_status.s.dqstatus) {
		cvmx_dprintf("%d: Error: Failed to open dq :%d error: %d\n",
				__LINE__, dq, pko_status.s.dqstatus);
		return -1;
	}

	return 0;
}

/*
 * Write PKO descriptor commands to scratch location specified by offset.
 *
 * @param offset is to select the location in LMTLINE to write the command.
 * @param descr_word is the descriptor command word to be written in LMTLINE.
 * @return none.
 */
static inline void cvmx_pko_write_descr_word(int offset, uint64_t descr_word) {

	/* write packet descriptor in scratch */
	cvmx_scratch_write64(CVMX_LMTDMA_WORD(offset), descr_word);

}

/*
 * Sends PKO descriptor commands written on scratch to PKO by issuing
 * LMTDMA.
 * @param node is to specify which node's pko block to send the commands.
 * @param dq is the queue to write the commands to.
 * @param num_descr_words is the total number of descriptor words.
 * @param scraddr is the scratch line to return the status of send operation.
 * @param scr_len is the return status len if scraddr is specified.
 * @return returns none.
 */
static inline void cvmx_pko_send_command(int node, int dq, int num_descr_words,
				         uint64_t scraddr, int scr_len) {

	cvmx_pko_send_dma_t pko_send_dma_data;

	/* build store data for LMTDMA */
	pko_send_dma_data.u64 = 0;
	pko_send_dma_data.s.scraddr = scraddr >> 3;
	pko_send_dma_data.s.rtnlen = scr_len;
	pko_send_dma_data.s.did = 0x51;
	pko_send_dma_data.s.node = node;
	pko_send_dma_data.s.dq = dq;

	CVMX_SYNCIOBDMA;

	/* issue LMTDMA */
	cvmx_write64_uint64(CVMX_LMTDMA_ADDR(num_descr_words), pko_send_dma_data.u64);

	CVMX_SYNCIOBDMA;
}

/*
 * PKO global intialization for 78XX.
 *
 * @param node is the node on which PKO block is initialized.
 * @return none.
 */
void cvmx_pko_initialize_global_78xx(int node);

/*
 * Transmit packets through pko on specified node and queue.
 *
 * @param node is to specify which node's pko block to send the commands.
 * @param dq is the queue to write the commands to.
 * @param bufptr specifies packet in linked or gather mode.
 * @param packet_len is the total packet len of the packet in bufptr.
 * @param aura_free is the aura to free packet buffers after trasnmit.
 * @return returns 0 if successful and -1 on failure.
 */
int cvmx_pko_transmit_packet(int node, int dq, cvmx_pko_buf_ptr_t bufptr,
			     int packet_len, int aura_free);

/*
 * Enables PKO on specified node for 78XX.
 *
 * @param node is the node on which PKO will be enabled.
 * @return none.
 */
void cvmx_pko_enable_78xx(int node);

/*
 * PKO setup per Interface/port.
 *
 * @param node is to specify which node's pko block for this setup.
 * @param interface is the interface number.
 * @return returns 0 if successful and -1 on failure.
 */
int cvmx_pko_setup_interface_78xx(int node, int interface);


#ifdef	__cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif
#endif /* __CVMX_PKO3_H__ */
