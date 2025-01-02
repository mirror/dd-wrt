/***********************license start***************
 * Copyright (c) 2014  Cavium Inc. (support@cavium.com). All rights
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
#include <linux/module.h>
#include <linux/errno.h>
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-fpa3.h>
#include <asm/octeon/cvmx-clock.h>
#include <asm/octeon/cvmx-hwpko.h>
#include <asm/octeon/cvmx-pko3.h>
#include <asm/octeon/cvmx-pko3-resources.h>
#include <asm/octeon/cvmx-helper-pko3.h>
#include <asm/octeon/cvmx-helper-bgx.h>
#else
#include "cvmx.h"
#include "cvmx-hwpko.h"	/* For legacy support */
#include "cvmx-fpa3.h"
#include "cvmx-pko3.h"
#include "cvmx-pko3-resources.h"
#include "cvmx-helper-pko3.h"
#include "cvmx-helper-bgx.h"
#include <errno.h>
#endif


static const int debug = 0;
#ifdef	__BIG_ENDIAN_BITFIELD
static const bool __native_le = 0;
#else
static const bool __native_le = 1;
#endif

#define CVMX_DUMP_REGX(reg) 	\
	if(debug)		\
	cvmx_dprintf("%s=%#llx\n",#reg,(long long)cvmx_read_csr_node(node,reg))

static int cvmx_pko_setup_macs(int node);

/*
 * PKO descriptor queue operation error string
 *
 * @param dqstatus is the enumeration returned from hardware,
 * 	  PKO_QUERY_RTN_S[DQSTATUS].
 *
 * @return static constant string error description
 */
const char * pko_dqstatus_error(pko_query_dqstatus_t dqstatus)
{
	char * str = "PKO Undefined error";

	switch(dqstatus) {
	case PKO_DQSTATUS_PASS :
		str = "No error";
		break;
	case PKO_DQSTATUS_BADSTATE :
		str = "PKO queue not ready";
		break;
	case PKO_DQSTATUS_NOFPABUF :
		str ="PKO failed to allocate buffer from FPA";
		break;
	case PKO_DQSTATUS_NOPKOBUF :
		str = "PKO out of buffers";
		break;
	case PKO_DQSTATUS_FAILRTNPTR :
		str = "PKO failed to return buffer to FPA";
		break;
	case PKO_DQSTATUS_ALREADY :
		str = "PKO queue already opened";
		break;
	case PKO_DQSTATUS_NOTCREATED:
		str = "PKO queue has not been created";
		break;
	case PKO_DQSTATUS_NOTEMPTY :
		str = "PKO queue is not empty";
		break;
	case PKO_DQSTATUS_SENDPKTDROP :
		str = "Illegal PKO command construct";
		break;
	}
	return str;
}

/*
 * PKO global intialization for 78XX.
 *
 * @param node is the node on which PKO block is initialized.
 * @return none.
 */
int cvmx_pko3_hw_init_global(int node, uint16_t aura)
{
	cvmx_pko_dpfi_flush_t pko_flush;
	cvmx_pko_dpfi_fpa_aura_t pko_aura;
	cvmx_pko_dpfi_ena_t dpfi_enable;
	cvmx_pko_ptf_iobp_cfg_t ptf_iobp_cfg;
	cvmx_pko_pdm_cfg_t pko_pdm_cfg;
	cvmx_pko_enable_t pko_enable;
	cvmx_pko_dpfi_status_t dpfi_status;
	cvmx_pko_status_t pko_status;
	cvmx_pko_shaper_cfg_t shaper_cfg;
	uint64_t cycles;
	const unsigned timeout = 100;	/* 100 milliseconds */

	if (node != (aura >> 10))
		cvmx_printf("WARNING: AURA vs PKO node mismatch\n");

	pko_enable.u64 = cvmx_read_csr_node(node, CVMX_PKO_ENABLE);
	if (pko_enable.s.enable) {
		cvmx_printf("WARNING: %s: PKO already enabled on node %u\n",
			__func__, node);
		return 0;
	}
	/* Enable color awareness. */
	shaper_cfg.u64 = cvmx_read_csr_node(node, CVMX_PKO_SHAPER_CFG);
	shaper_cfg.s.color_aware = 1;
	cvmx_write_csr_node(node, CVMX_PKO_SHAPER_CFG, shaper_cfg.u64);

	/* Clear FLUSH command to be sure */
	pko_flush.u64 = 0;
	pko_flush.s.flush_en = 0;
	cvmx_write_csr_node(node, CVMX_PKO_DPFI_FLUSH, pko_flush.u64);

	/* set the aura number in pko, use aura node from parameter */
	pko_aura.u64 = 0;
	pko_aura.s.node = aura >> 10;
	pko_aura.s.laura = aura;
	cvmx_write_csr_node(node, CVMX_PKO_DPFI_FPA_AURA, pko_aura.u64);

	CVMX_DUMP_REGX(CVMX_PKO_DPFI_FPA_AURA);

	dpfi_enable.u64 = 0;
	dpfi_enable.s.enable = 1;
	cvmx_write_csr_node(node, CVMX_PKO_DPFI_ENA, dpfi_enable.u64);

	/* Prepare timeout */
        cycles = cvmx_get_cycle();
        cycles += cvmx_clock_get_rate(CVMX_CLOCK_CORE)/1000 * timeout;

	/* Wait until all pointers have been returned */
	do {
		pko_status.u64 = cvmx_read_csr_node(node, CVMX_PKO_STATUS);
		if (cycles < cvmx_get_cycle())
			break;
	} while (!pko_status.s.pko_rdy);

	if (!pko_status.s.pko_rdy) {
		dpfi_status.u64 = cvmx_read_csr_node(node, CVMX_PKO_DPFI_STATUS);
		cvmx_printf("ERROR: %s: PKO DFPI failed, "
			"PKO_STATUS=%#llx DPFI_STATUS=%#llx\n", __func__,
			(unsigned long long) pko_status.u64,
			(unsigned long long) dpfi_status.u64);
		return -1;
	}

	/* Set max outstanding requests in IOBP for any FIFO.*/
	ptf_iobp_cfg.u64 = cvmx_read_csr_node(node, CVMX_PKO_PTF_IOBP_CFG);
	if(OCTEON_IS_MODEL(OCTEON_CN78XX))
		ptf_iobp_cfg.s.max_read_size = 0x10; /* Recommended by HRM.*/
	else
		/* Reduce the value from recommended 0x10 to avoid
		 * getting "underflow" condition in the BGX TX FIFO.*/
		ptf_iobp_cfg.s.max_read_size = 3;
	cvmx_write_csr_node(node, CVMX_PKO_PTF_IOBP_CFG, ptf_iobp_cfg.u64);

	/* Set minimum packet size per Ethernet standard */
	pko_pdm_cfg.u64 = 0;
	pko_pdm_cfg.s.pko_pad_minlen = 0x3c;	/* 60 bytes before FCS */
	cvmx_write_csr_node(node, CVMX_PKO_PDM_CFG, pko_pdm_cfg.u64);

	/* Initialize MACs and FIFOs */
	cvmx_pko_setup_macs(node);

	/* enable PKO, although interfaces and queues are not up yet */
	pko_enable.u64 = 0;
	pko_enable.s.enable = 1;
	cvmx_write_csr_node(node, CVMX_PKO_ENABLE, pko_enable.u64);

	/* PKO_RDY set indicates succesful initialization */
	pko_status.u64 = cvmx_read_csr_node(node, CVMX_PKO_STATUS);
	if (pko_status.s.pko_rdy)
		return 0;

	cvmx_printf("ERROR: %s: failed, PKO_STATUS=%#llx\n", __func__,
		(unsigned long long) pko_status.u64);
	return -1;
}

/**
 * Shutdown the entire PKO
 */
int cvmx_pko3_hw_disable(int node)
{
	cvmx_pko_dpfi_flush_t pko_flush;
	cvmx_pko_dpfi_status_t dpfi_status;
	cvmx_pko_dpfi_ena_t dpfi_enable;
	cvmx_pko_enable_t pko_enable;
	cvmx_pko_status_t pko_status;
	uint64_t cycles;
	const unsigned timeout = 10;	/* 10 milliseconds */
	unsigned mac_num, fifo, i;
	unsigned null_mac_num, null_fifo_num, fifo_grp_count, pq_count;

	(void) pko_status;

	/* Wait until there are no in-flight packets */
	for(i = mac_num = 0; mac_num < __cvmx_pko3_num_macs(); mac_num++) {
		cvmx_pko_ptfx_status_t ptf_status;
		ptf_status.u64 =
			cvmx_read_csr_node(node, CVMX_PKO_PTFX_STATUS(mac_num));
		if (debug)
			cvmx_dprintf("%s: MAC %u in-flight %u total %u\n",
				__func__, mac_num,
				ptf_status.s.in_flight_cnt,
				ptf_status.s.total_in_flight_cnt);
		if (ptf_status.s.mac_num == 0x1f)
			continue;
		if (ptf_status.s.in_flight_cnt != 0) {
			cvmx_printf("WARNING: %s: MAC %d in-flight %d\n",
				__func__, mac_num, ptf_status.s.in_flight_cnt);
			mac_num --;
			cvmx_wait(1000);
		}
	}

	/* disable PKO - all packets should be out by now */
	pko_enable.u64 = 0;
	pko_enable.s.enable = 0;
	cvmx_write_csr_node(node, CVMX_PKO_ENABLE, pko_enable.u64);

	/* Assign NULL MAC# for L1/SQ disabled state */
        if (OCTEON_IS_MODEL(OCTEON_CN73XX)) {
		null_mac_num = 0x0f;
		null_fifo_num = 0x1f;
		fifo_grp_count = 4;
		pq_count = 16;
	} else if (OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
		null_mac_num = 0x0a;
		null_fifo_num = 0x1f;
		fifo_grp_count = 4;
		pq_count = 16;
	} else {
		null_mac_num = 0x1c;
		null_fifo_num = 0x1f;
		fifo_grp_count = 8;
		pq_count = 32;
	}

	/* Reset L1_PQ */
	for(i = 0; i < pq_count; i++) {
		cvmx_pko_l1_sqx_topology_t pko_l1_topology;
		cvmx_pko_l1_sqx_shape_t pko_l1_shape;
		cvmx_pko_l1_sqx_link_t pko_l1_link;
		pko_l1_topology.u64 = 0;
		pko_l1_topology.s.link = null_mac_num;
		cvmx_write_csr_node(node, CVMX_PKO_L1_SQX_TOPOLOGY(i),
			pko_l1_topology.u64);

		pko_l1_shape.u64 = 0;
		pko_l1_shape.s.link = null_mac_num;
		cvmx_write_csr_node(node, CVMX_PKO_L1_SQX_SHAPE(i), pko_l1_shape.u64);

		pko_l1_link.u64 = 0;
		pko_l1_link.s.link = null_mac_num;
		cvmx_write_csr_node(node, CVMX_PKO_L1_SQX_LINK(i), pko_l1_link.u64);

	}

	/* Reset all MAC configurations */
	for(mac_num = 0; mac_num < __cvmx_pko3_num_macs(); mac_num++) {
		cvmx_pko_macx_cfg_t pko_mac_cfg;

		pko_mac_cfg.u64 = 0;
		pko_mac_cfg.s.fifo_num = null_fifo_num;
		cvmx_write_csr_node(node, CVMX_PKO_MACX_CFG(mac_num),
			pko_mac_cfg.u64);
	}

	/* Reset all FIFO groups */
	for(fifo = 0; fifo < fifo_grp_count; fifo++) {
		cvmx_pko_ptgfx_cfg_t pko_ptgfx_cfg;

		pko_ptgfx_cfg.u64 = cvmx_read_csr_node(node, CVMX_PKO_PTGFX_CFG(fifo));
		/* Simulator asserts if an unused group is reset */
		if (pko_ptgfx_cfg.u64 == 0)
			continue;
		pko_ptgfx_cfg.u64 = 0;
		pko_ptgfx_cfg.s.reset = 1;
		cvmx_write_csr_node(node, CVMX_PKO_PTGFX_CFG(fifo),
					pko_ptgfx_cfg.u64);
	}

	/* Set FLUSH_EN to return cached pointers to FPA */
	pko_flush.u64 = 0;
	pko_flush.s.flush_en = 1;
	cvmx_write_csr_node(node, CVMX_PKO_DPFI_FLUSH, pko_flush.u64);

	/* Prepare timeout */
	cycles = cvmx_get_cycle();
	cycles += cvmx_clock_get_rate(CVMX_CLOCK_CORE)/1000 * timeout;

	/* Wait until all pointers have been returned */
	do {
		dpfi_status.u64 = cvmx_read_csr_node(node, CVMX_PKO_DPFI_STATUS);
		if (cycles < cvmx_get_cycle())
			break;
	} while (!dpfi_status.s.cache_flushed);

	/* disable PKO buffer manager, should return all buffers to FPA */
	dpfi_enable.u64 = 0;
	dpfi_enable.s.enable = 0;
	cvmx_write_csr_node(node, CVMX_PKO_DPFI_ENA, dpfi_enable.u64);

	CVMX_DUMP_REGX(CVMX_PKO_DPFI_ENA);
	CVMX_DUMP_REGX(CVMX_PKO_DPFI_STATUS);
	CVMX_DUMP_REGX(CVMX_PKO_STATUS);

	/* Clear the FLUSH_EN bit, as we are done */
	pko_flush.u64 = 0;
	cvmx_write_csr_node(node, CVMX_PKO_DPFI_FLUSH, pko_flush.u64);
	CVMX_DUMP_REGX(CVMX_PKO_DPFI_FLUSH);

	if (dpfi_status.s.cache_flushed == 0) {
		cvmx_printf("%s: ERROR: timeout waiting for PKO3 ptr flush\n",
			__FUNCTION__);
		return -1;
	}

	return 0;
}

/*
 * Configure Channel credit level in PKO.
 *
 * @param node is to specify the node to which this configuration is applied.
 * @param level specifies the level at which pko channel queues will be configured,
 * @return returns 0 if successful and -1 on failure.
 */
int cvmx_pko3_channel_credit_level(int node, enum cvmx_pko3_level_e level)
{
	union cvmx_pko_channel_level channel_level;

	channel_level.u64 = 0;

	if (level == CVMX_PKO_L2_QUEUES)
		channel_level.s.cc_level = 0;
	else if (level == CVMX_PKO_L3_QUEUES)
		channel_level.s.cc_level = 1;
	else
		return -1;

	cvmx_write_csr_node(node, CVMX_PKO_CHANNEL_LEVEL, channel_level.u64);

	return 0;

}

/** Open configured descriptor queues before queueing packets into them.
 *
 * @param node is to specify the node to which this configuration is applied.
 * @param dq is the descriptor queue number to be opened.
 * @return returns 0 on sucess or -1 on failure.
 */
int cvmx_pko_dq_open(int node, int dq)
{
	cvmx_pko_query_rtn_t pko_status;
	pko_query_dqstatus_t dqstatus;
	cvmx_pko3_dq_params_t *pParam;

	if(debug)
		cvmx_dprintf("%s: DEBUG: dq %u\n", __FUNCTION__, dq);

	__cvmx_pko3_dq_param_setup(node);

	pko_status = __cvmx_pko3_do_dma(node, dq, NULL, 0, CVMX_PKO_DQ_OPEN);

	dqstatus = pko_status.s.dqstatus;

	if (dqstatus == PKO_DQSTATUS_ALREADY)
		return 0;
	if (dqstatus != PKO_DQSTATUS_PASS) {
		cvmx_printf("%s: ERROR: Failed to open dq :%u: %s\n",
				__FUNCTION__, dq,
				pko_dqstatus_error(dqstatus));
		return -1;
	}

	/* Setup the descriptor queue software parameters */
	pParam = cvmx_pko3_dq_parameters(node, dq);
	if (pParam != NULL) {
		pParam->depth = pko_status.s.depth;
		if (pParam->limit == 0)
			pParam->limit = 1024;	/* last-resort default */
	}

	return 0;
}


/**
 * Close a descriptor queue
 *
 * @param node is to specify the node to which this configuration is applied.
 * @param dq is the descriptor queue number to be opened.
 * @return returns 0 on sucess or -1 on failure.
 *
 * This should be called before changing the DQ parent link, topology,
 * or when shutting down the PKO.
 */
int cvmx_pko3_dq_close(int node, int dq)
{
	cvmx_pko_query_rtn_t pko_status;
	pko_query_dqstatus_t dqstatus;

	if(debug)
		cvmx_dprintf("%s: DEBUG: dq %u\n", __FUNCTION__, dq);

	pko_status = __cvmx_pko3_do_dma(node, dq, NULL, 0, CVMX_PKO_DQ_CLOSE);

	dqstatus = pko_status.s.dqstatus;

	if (dqstatus == PKO_DQSTATUS_NOTCREATED)
		return 0;

	if (dqstatus != PKO_DQSTATUS_PASS) {
		cvmx_printf("WARNING: %s: Failed to close dq :%u: %s\n",
				__FUNCTION__, dq,
				pko_dqstatus_error(dqstatus));
		cvmx_dprintf("DEBUG: %s: dq %u depth %u\n",
			__FUNCTION__, dq, (unsigned) pko_status.s.depth);
	}

	return 0;
}

/**
 * Drain a descriptor queue
 *
 * Before closing a DQ, this call will drain all pending traffic
 * on the DQ to the NULL MAC, which will circumvent any traffic
 * shaping and flow control to quickly reclaim all packet buffers.
 *
 * @param node is to specify the node to which this configuration is applied.
 * @param dq is the descriptor queue number to be drained.
 */
void cvmx_pko3_dq_drain(int node, int dq)
{
	cvmx_pko_dqx_sw_xoff_t rxoff;

	rxoff.u64 = 0;
	rxoff.s.drain_null_link = 1;
	rxoff.s.drain = 1;
	rxoff.s.xoff = 0;

	cvmx_write_csr_node(node, CVMX_PKO_DQX_SW_XOFF(dq), rxoff.u64);

	cvmx_wait(100);

	rxoff.u64 = 0;
	cvmx_write_csr_node(node, CVMX_PKO_DQX_SW_XOFF(dq), rxoff.u64);
}

 /**
 * Query a descriptor queue
 *
 * @param node is to specify the node to which this configuration is applied.
 * @param dq is the descriptor queue number to be opened.
 * @return returns the descriptor queue depth on sucess or -1 on failure.
 *
 * This should be called before changing the DQ parent link, topology,
 * or when shutting down the PKO.
 */
int cvmx_pko3_dq_query(int node, int dq)
{
	cvmx_pko_query_rtn_t pko_status;
	pko_query_dqstatus_t dqstatus;

	pko_status = __cvmx_pko3_do_dma(node, dq, NULL, 0, CVMX_PKO_DQ_QUERY);

	dqstatus = pko_status.s.dqstatus;

	if (dqstatus != PKO_DQSTATUS_PASS) {
		cvmx_printf("%s: ERROR: Failed to query dq :%u: %s\n",
				__FUNCTION__, dq,
				pko_dqstatus_error(dqstatus));
		return -1;
	}

	/* Temp: debug for HW */
	if (pko_status.s.depth > 0)
		cvmx_dprintf("%s: DEBUG: dq %u depth %u\n",
			__FUNCTION__, dq, (unsigned) pko_status.s.depth);

	return pko_status.s.depth;
}

/*
 * PKO initialization of MACs and FIFOs
 *
 * All MACs are configured and assigned a specific FIFO,
 * and each FIFO is configured with size for a best utilization
 * of available FIFO resources.
 *
 * @param node is to specify which node's pko block for this setup.
 * @return returns 0 if successful and -1 on failure.
 *
 * Note: This function contains model-specific code.
 */
static int cvmx_pko_setup_macs(int node)
{
	unsigned interface;
	unsigned port, num_ports;
	unsigned mac_num, fifo, pri, cnt;
	cvmx_helper_interface_mode_t mode;
        const unsigned num_interfaces = cvmx_helper_get_number_of_interfaces();
	uint8_t fifo_group_cfg[8];
	uint8_t fifo_group_spd[8];
	unsigned fifo_count = 0;
	unsigned max_fifos = 0, fifo_groups = 0;
	struct {
		uint8_t fifo_cnt;
		uint8_t fifo_id;
		uint8_t pri;
		uint8_t spd;
		uint8_t mac_fifo_cnt;
	} cvmx_pko3_mac_table[32];

        if(OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		max_fifos = 28;	/* exclusive of NULL FIFO */
		fifo_groups = 8;/* inclusive of NULL PTGF */
	}
        if(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
		max_fifos = 16;
		fifo_groups = 5;
	}

	/* Initialize FIFO allocation table */
	memset(&fifo_group_cfg, 0, sizeof(fifo_group_cfg));
	memset(&fifo_group_spd, 0, sizeof(fifo_group_spd));
	memset(cvmx_pko3_mac_table, 0, sizeof(cvmx_pko3_mac_table));

	/* Initialize all MACs as disabled */
	for(mac_num = 0; mac_num < __cvmx_pko3_num_macs(); mac_num++) {
		cvmx_pko3_mac_table[mac_num].pri = 0;
		cvmx_pko3_mac_table[mac_num].fifo_cnt = 0;
		cvmx_pko3_mac_table[mac_num].fifo_id = 0x1f;
	}

	for(interface = 0; interface < num_interfaces; interface ++) {
		int xiface =  cvmx_helper_node_interface_to_xiface(node, interface);
		/* Interface type for ALL interfaces */
		mode = cvmx_helper_interface_get_mode(xiface);
		num_ports = cvmx_helper_interface_enumerate(xiface);

		if(mode == CVMX_HELPER_INTERFACE_MODE_DISABLED)
			continue;
		/*
		 * Non-BGX interfaces:
		 * Each of these interfaces has a single MAC really.
		 */
		if ((mode == CVMX_HELPER_INTERFACE_MODE_ILK) ||
		    (mode == CVMX_HELPER_INTERFACE_MODE_NPI) ||
		    (mode == CVMX_HELPER_INTERFACE_MODE_LOOP))
			num_ports = 1;

		for (port = 0; port < num_ports; port++) {
			int i;

			/* Get the per-port mode for BGX-interfaces */
			if (interface < CVMX_HELPER_MAX_GMX)
			    mode = cvmx_helper_bgx_get_mode(xiface, port);
			/* In MIXED mode, LMACs can run different protocols */

			/* convert interface/port to mac number */
			i = __cvmx_pko3_get_mac_num(xiface, port);
			if (i < 0 || i >= (int) __cvmx_pko3_num_macs()) {
				cvmx_printf("%s: ERROR: interface %d:%u "
				    "port %d has no MAC %d/%d\n",
				    __func__, node, interface, port,
				    i, __cvmx_pko3_num_macs());
				continue;
			}

			if(mode == CVMX_HELPER_INTERFACE_MODE_RXAUI) {
				unsigned bgx_fifo_size = 
				  __cvmx_helper_bgx_fifo_size(xiface, port);
				cvmx_pko3_mac_table[i].mac_fifo_cnt = 
				    bgx_fifo_size / (CVMX_BGX_TX_FIFO_SIZE/4);
				cvmx_pko3_mac_table[i].pri = 2;
				cvmx_pko3_mac_table[i].spd = 10;
				cvmx_pko3_mac_table[i].fifo_cnt = 2;
			} else if (mode == CVMX_HELPER_INTERFACE_MODE_XLAUI) {
				unsigned bgx_fifo_size = 
				  __cvmx_helper_bgx_fifo_size(xiface, port);
				cvmx_pko3_mac_table[i].mac_fifo_cnt = 
				    bgx_fifo_size / (CVMX_BGX_TX_FIFO_SIZE/4);
				cvmx_pko3_mac_table[i].pri = 4;
				cvmx_pko3_mac_table[i].spd = 40;
				cvmx_pko3_mac_table[i].fifo_cnt = 4;
			} else if (mode == CVMX_HELPER_INTERFACE_MODE_XAUI) {
				unsigned bgx_fifo_size = 
				  __cvmx_helper_bgx_fifo_size(xiface, port);
				cvmx_pko3_mac_table[i].mac_fifo_cnt = 
				    bgx_fifo_size / (CVMX_BGX_TX_FIFO_SIZE/4);
				cvmx_pko3_mac_table[i].pri = 3;
				cvmx_pko3_mac_table[i].fifo_cnt = 4;
				/* DXAUI at 20G, or XAU at 10G */
				cvmx_pko3_mac_table[i].spd = 20;
			} else if (mode == CVMX_HELPER_INTERFACE_MODE_XFI) {
				unsigned bgx_fifo_size = 
				  __cvmx_helper_bgx_fifo_size(xiface, port);
				cvmx_pko3_mac_table[i].mac_fifo_cnt = 
				    bgx_fifo_size / (CVMX_BGX_TX_FIFO_SIZE/4);
				cvmx_pko3_mac_table[i].pri = 3;
				cvmx_pko3_mac_table[i].fifo_cnt = 4;
				cvmx_pko3_mac_table[i].spd = 10;
			} else if (mode == CVMX_HELPER_INTERFACE_MODE_LOOP) {
				cvmx_pko3_mac_table[i].fifo_cnt = 1;
				cvmx_pko3_mac_table[i].pri = 1;
				cvmx_pko3_mac_table[i].spd = 1;
				cvmx_pko3_mac_table[i].mac_fifo_cnt = 1;
			} else if (mode == CVMX_HELPER_INTERFACE_MODE_ILK ||
				mode == CVMX_HELPER_INTERFACE_MODE_SRIO) {
				cvmx_pko3_mac_table[i].fifo_cnt = 4;
				cvmx_pko3_mac_table[i].pri = 3;
				/* ILK/SRIO: speed depends on lane count */
				cvmx_pko3_mac_table[i].spd = 40;
				cvmx_pko3_mac_table[i].mac_fifo_cnt = 4;
			} else if (mode == CVMX_HELPER_INTERFACE_MODE_NPI) {
				cvmx_pko3_mac_table[i].fifo_cnt = 4;
				cvmx_pko3_mac_table[i].pri = 2;
				/* Actual speed depends on PCIe lanes/mode */
				cvmx_pko3_mac_table[i].spd = 50;
				/* SLI Tx FIFO size to be revisitted */
				cvmx_pko3_mac_table[i].mac_fifo_cnt = 1;
			} else {
				/* Other BGX interface modes: SGMII/RGMII */
				unsigned bgx_fifo_size = 
				  __cvmx_helper_bgx_fifo_size(xiface, port);
				cvmx_pko3_mac_table[i].mac_fifo_cnt = 
				    bgx_fifo_size / (CVMX_BGX_TX_FIFO_SIZE/4);
				cvmx_pko3_mac_table[i].fifo_cnt = 1;
				cvmx_pko3_mac_table[i].pri = 1;
				cvmx_pko3_mac_table[i].spd = 1;
			}

			if(debug)
				cvmx_dprintf("%s: intf %d:%u port %u %s mac %02u cnt %u macfifo %uk spd %u\n",
					     __FUNCTION__, node, interface, port,
				cvmx_helper_interface_mode_to_string(mode),
				i,
				cvmx_pko3_mac_table[i].fifo_cnt,
				cvmx_pko3_mac_table[i].mac_fifo_cnt*8,
				cvmx_pko3_mac_table[i].spd);

		} /* for port */
	} /* for interface */

	/* Count the number of requested FIFOs */
	for(fifo_count = mac_num = 0; mac_num < __cvmx_pko3_num_macs(); mac_num ++)
		fifo_count += cvmx_pko3_mac_table[mac_num].fifo_cnt;

	if(debug)
		cvmx_dprintf("%s: initially requested FIFO count %u\n",
			__FUNCTION__, fifo_count);

	/* Heuristically trim FIFO count to fit in available number */
	pri = 1; cnt = 4;
	while(fifo_count > max_fifos) {
		for(mac_num=0; mac_num < __cvmx_pko3_num_macs(); mac_num ++) {
			if (cvmx_pko3_mac_table[mac_num].fifo_cnt == cnt &&
			    cvmx_pko3_mac_table[mac_num].pri <= pri) {
				cvmx_pko3_mac_table[mac_num].fifo_cnt >>= 1;
				fifo_count -=
					cvmx_pko3_mac_table[mac_num].fifo_cnt;
			}
			if (fifo_count <= max_fifos)
				break;
		}
		if (pri >= 4) {
			pri = 1;
			cnt >>= 1;
		} else
			pri ++;
		if( cnt == 0)
			break;
	}

	if(debug)
		cvmx_dprintf("%s: adjusted FIFO count %u\n",
			__FUNCTION__, fifo_count);


	/* Special case for NULL Virtual FIFO */
	fifo_group_cfg[fifo_groups-1] = 0;
	/* there is no MAC connected to NULL FIFO */

	/* Configure MAC units, and attach a FIFO to each */
	for(fifo = 0, cnt = 4; cnt > 0; cnt >>= 1 ) {
		unsigned g;
		for(mac_num = 0; mac_num < __cvmx_pko3_num_macs(); mac_num++) {
			if(cvmx_pko3_mac_table[mac_num].fifo_cnt < cnt ||
			  cvmx_pko3_mac_table[mac_num].fifo_id != 0x1f)
				continue;

			/* Attach FIFO to MAC */
			cvmx_pko3_mac_table[mac_num].fifo_id = fifo;
			g = fifo >> 2;
			/* Sum speed for FIFO group */
			fifo_group_spd[g] += cvmx_pko3_mac_table[mac_num].spd;

			if(cnt == 4)
				fifo_group_cfg[g] = 4; /* 10k,0,0,0 */
			else if(cnt == 2 && (fifo & 0x3) == 0)
				fifo_group_cfg[g] = 3; /* 5k,0,5k,0 */
			else if (cnt == 2 && fifo_group_cfg[g] == 3)
				/* no change */;
			else if(cnt == 1 && (fifo & 0x2) &&
				fifo_group_cfg[g] == 3)
				fifo_group_cfg[g] = 1; /* 5k,0,2.5k 2.5k*/
			else if(cnt == 1 && (fifo & 0x3)==0x3)
				/* no change */;
			else if (cnt == 1)
				fifo_group_cfg[g] = 0; /* 2.5k x 4 */
			else
				cvmx_printf("ERROR: %s: internal error\n",__func__);

			fifo += cnt;
		}
	}

	/* Check if there was no error in FIFO allocation */
	if (fifo > max_fifos) {
		cvmx_printf("ERROR: %s: Internal error FIFO %u\n",
			__func__, fifo);
		return -1;
	}

	if(debug)
		cvmx_dprintf("%s: used %u of FIFOs\n",
			__FUNCTION__, fifo);

	/* Now configure all FIFO groups */
	for(fifo = 0; fifo < fifo_groups; fifo++) {
		cvmx_pko_ptgfx_cfg_t pko_ptgfx_cfg;

		pko_ptgfx_cfg.u64 =
			cvmx_read_csr_node(node, CVMX_PKO_PTGFX_CFG(fifo));
		if( pko_ptgfx_cfg.s.size != fifo_group_cfg[fifo])
			pko_ptgfx_cfg.s.reset = 1;
		pko_ptgfx_cfg.s.size = fifo_group_cfg[fifo] ;
		if( fifo_group_spd[fifo] >= 40 )
			if( pko_ptgfx_cfg.s.size >= 3)
				pko_ptgfx_cfg.s.rate = 3;	/* 50 Gbps */
			else
				pko_ptgfx_cfg.s.rate = 2;	/* 25 Gbps */
		else if( fifo_group_spd[fifo] >= 20 )
			pko_ptgfx_cfg.s.rate = 2;	/* 25 Gbps */
		else if( fifo_group_spd[fifo] >= 10 )
			pko_ptgfx_cfg.s.rate = 1;	/* 12.5 Gbps */
		else
			pko_ptgfx_cfg.s.rate = 0;	/* 6.25 Gbps */

		if(debug)
			cvmx_dprintf("%s: FIFO %#x-%#x size=%u "
				"speed=%d rate=%d\n",
				__func__, fifo*4, fifo*4+3,
				 pko_ptgfx_cfg.s.size,
				fifo_group_spd[fifo],
				pko_ptgfx_cfg.s.rate);

		cvmx_write_csr_node(node, CVMX_PKO_PTGFX_CFG(fifo),
					pko_ptgfx_cfg.u64);
		pko_ptgfx_cfg.s.reset = 0;
		cvmx_write_csr_node(node, CVMX_PKO_PTGFX_CFG(fifo),
					pko_ptgfx_cfg.u64);
	}

	/* Configure all MACs assigned FIFO number */
	for(mac_num = 0; mac_num < __cvmx_pko3_num_macs(); mac_num++) {
		cvmx_pko_macx_cfg_t pko_mac_cfg;

		if(debug)
			cvmx_dprintf("%s: mac#%02u: fifo=%#x cnt=%u speed=%d\n",
			__FUNCTION__, mac_num,
			cvmx_pko3_mac_table[mac_num].fifo_id,
			cvmx_pko3_mac_table[mac_num].fifo_cnt,
			cvmx_pko3_mac_table[mac_num].spd);

		pko_mac_cfg.u64 =
			cvmx_read_csr_node(node, CVMX_PKO_MACX_CFG(mac_num));
		pko_mac_cfg.s.fifo_num = cvmx_pko3_mac_table[mac_num].fifo_id;
		cvmx_write_csr_node(node, CVMX_PKO_MACX_CFG(mac_num),
			pko_mac_cfg.u64);
	}

	/* Setup PKO MCI0/MCI1/SKID credits */
	for(mac_num = 0; mac_num < __cvmx_pko3_num_macs(); mac_num++) {
		cvmx_pko_mci0_max_credx_t pko_mci0_max_cred;
		cvmx_pko_mci1_max_credx_t pko_mci1_max_cred;
		cvmx_pko_macx_cfg_t pko_mac_cfg;
		unsigned fifo_credit, mac_credit, skid_credit;
		unsigned pko_fifo_cnt, fifo_size;
		unsigned mac_fifo_cnt;
		unsigned tmp;
		int saved_fifo_num;

		pko_fifo_cnt = cvmx_pko3_mac_table[mac_num].fifo_cnt;
		mac_fifo_cnt = cvmx_pko3_mac_table[mac_num].mac_fifo_cnt;

		/* Skip unused MACs */
		if (pko_fifo_cnt == 0)
			continue;

		/* Check for sanity */
		if (pko_fifo_cnt > 4)
			pko_fifo_cnt = 1;

		fifo_size = (2 * 1024) + (1024 / 2); /* 2.5KiB */
		fifo_credit = pko_fifo_cnt * fifo_size;
		
		if (mac_num == 0) {
			/* loopback */
			mac_credit = 4096; /* From HRM Sec 13.0 */
			skid_credit = 0;
		} else if (mac_num == 1) {
			 /* DPI */
			mac_credit = 2 * 1024;
			skid_credit = 0;
		} else if (octeon_has_feature(OCTEON_FEATURE_ILK) &&
			(mac_num & 0xfe) == 2) {
			/* ILK0, ILK1: MAC 2,3 */
			mac_credit = 4 * 1024; /* 4KB fifo */
			skid_credit = 0;
		} else if (octeon_has_feature(OCTEON_FEATURE_SRIO) &&
			(mac_num >= 6) && (mac_num <= 9)) {
			/* SRIO0, SRIO1: MAC 6..9 */
			mac_credit = 1024 / 2;
			skid_credit = 0;
		} else {
			/* BGX */
			mac_credit = mac_fifo_cnt * 8 * 1024;
			skid_credit = mac_fifo_cnt * 256;
		}

		if (debug) cvmx_dprintf(
			"%s: mac %u "
			"pko_fifo_credit=%u mac_credit=%u\n",
			__FUNCTION__, mac_num, fifo_credit, mac_credit);

		tmp = (fifo_credit + mac_credit) / 16;
		pko_mci0_max_cred.u64 = 0;
		pko_mci0_max_cred.s.max_cred_lim = tmp;

		/* Check for overflow */
		if (pko_mci0_max_cred.s.max_cred_lim != tmp) {
			cvmx_printf("WARNING: %s: MCI0 credit overflow\n",__func__);
			pko_mci0_max_cred.s.max_cred_lim = 0xfff;
		}

		/* Pass 2 PKO hardware does not use the MCI0 credits */
		if(OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			cvmx_write_csr_node(node,
					CVMX_PKO_MCI0_MAX_CREDX(mac_num),
					pko_mci0_max_cred.u64);

		/* The original CSR formula is the correct one after all */
		tmp = (mac_credit) / 16;
		pko_mci1_max_cred.u64 = 0;
		pko_mci1_max_cred.s.max_cred_lim = tmp;

		/* Check for overflow */
		if (pko_mci1_max_cred.s.max_cred_lim != tmp) {
			cvmx_printf("WARNING: %s: MCI1 credit overflow\n",__func__);
			pko_mci1_max_cred.s.max_cred_lim = 0xfff;
		}

		cvmx_write_csr_node(node, CVMX_PKO_MCI1_MAX_CREDX(mac_num),
					pko_mci1_max_cred.u64);

		tmp = (skid_credit / 256) >> 1; /* valid 0,1,2 */
		pko_mac_cfg.u64 =
			cvmx_read_csr_node(node, CVMX_PKO_MACX_CFG(mac_num));

		/* The PKO_MACX_CFG bits cannot be changed unless FIFO_MUM=0x1f (unused fifo) */
		saved_fifo_num = pko_mac_cfg.s.fifo_num;
		pko_mac_cfg.s.fifo_num = 0x1f;
		pko_mac_cfg.s.skid_max_cnt = tmp;
		cvmx_write_csr_node(node, CVMX_PKO_MACX_CFG(mac_num),
			pko_mac_cfg.u64);

		pko_mac_cfg.u64 =
			cvmx_read_csr_node(node, CVMX_PKO_MACX_CFG(mac_num));
		pko_mac_cfg.s.fifo_num = saved_fifo_num;
		cvmx_write_csr_node(node, CVMX_PKO_MACX_CFG(mac_num),
			pko_mac_cfg.u64);

		if (debug) {
			pko_mci0_max_cred.u64 =
			     cvmx_read_csr_node(node, CVMX_PKO_MCI0_MAX_CREDX(mac_num));
			pko_mci1_max_cred.u64 =
			     cvmx_read_csr_node(node, CVMX_PKO_MCI1_MAX_CREDX(mac_num));
			pko_mac_cfg.u64 =
			     cvmx_read_csr_node(node, CVMX_PKO_MACX_CFG(mac_num));
			cvmx_dprintf(
			"%s: mac %u PKO_MCI0_MAX_CREDX=%u PKO_MCI1_MAX_CREDX=%u PKO_MACX_CFG[SKID_MAX_CNT]=%u\n",
			__FUNCTION__,  mac_num,
			pko_mci0_max_cred.s.max_cred_lim,
			pko_mci1_max_cred.s.max_cred_lim,
			pko_mac_cfg.s.skid_max_cnt);
		}
	} /* for mac_num */

	return 0;
}

/**
 * @INTERNAL
 * Backward compatibility for collecting statistics from PKO3
 *
 * NOTE:
 * The good stats are in BGX block.
 */
void cvmx_pko3_get_legacy_port_stats(uint16_t ipd_port,
	unsigned clear, cvmx_pko_port_status_t * stats)
{
	unsigned dq, dq_base, dq_num;
	unsigned node = cvmx_get_node_num();

	dq_base = cvmx_pko3_get_queue_base(ipd_port);
	dq_num = cvmx_pko3_get_queue_num(ipd_port);

	stats->packets = 0;
	stats->octets = 0;
	stats->doorbell = 0;	/* NOTE: PKO3 does not have a doorbell */

	for(dq = dq_base; dq < (dq_base+dq_num); dq++ ) {
		cvmx_pko_dqx_packets_t pkts;
		cvmx_pko_dqx_bytes_t byts;

		/* NOTE: clearing of these counters is non-atomic */
		pkts.u64 = cvmx_read_csr_node(node, CVMX_PKO_DQX_PACKETS(dq));
		if(clear)
			cvmx_write_csr_node(node,CVMX_PKO_DQX_PACKETS(dq),0ull);

		byts.u64 = cvmx_read_csr_node(node, CVMX_PKO_DQX_BYTES(dq));
		if(clear)
			cvmx_write_csr_node(node, CVMX_PKO_DQX_BYTES(dq), 0ull);

		stats->packets  += pkts.s.count;
		stats->octets  += byts.s.count;
	} /* for dq */

}

/** Set MAC options
 *
 * The options supported are the parameters below:
 *
 * @param xiface The physical interface number
 * @param index The physical sub-interface port
 * @param fcs_enable Enable FCS generation
 * @param pad_enable Enable padding to minimum packet size
 * @param fcs_sop_off Number of bytes at start of packet to exclude from FCS
 *
 * The typical use for `fcs_sop_off` is when the interface is configured
 * to use a header such as HighGig to precede every Ethernet packet,
 * such a header usually does not partake in the CRC32 computation stream,
 * and its size must be set with this parameter.
 *
 * @return Returns 0 on success, -1 if interface/port is invalid.
 */
int cvmx_pko3_interface_options(int xiface, int index,
			bool fcs_enable, bool pad_enable,
			unsigned fcs_sop_off)
{
	int mac_num;
	cvmx_pko_macx_cfg_t pko_mac_cfg;
	unsigned fifo_num;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (debug)
		cvmx_dprintf("%s: intf %u:%u/%u fcs=%d pad=%d\n",
			__func__, xi.node, xi.interface, index,
			fcs_enable, pad_enable);

	mac_num = __cvmx_pko3_get_mac_num(xiface, index);
	if(mac_num < 0) {
		cvmx_printf("ERROR: %s: invalid interface %u:%u/%u\n",
			__func__, xi.node, xi.interface, index);
		return -1;
	}

	pko_mac_cfg.u64 = cvmx_read_csr_node(xi.node, CVMX_PKO_MACX_CFG(mac_num));

	/* If MAC is not assigned, return an error */
	if (pko_mac_cfg.s.fifo_num == 0x1f) {
		cvmx_printf("ERROR: %s: unused interface %u:%u/%u\n",
			__func__, xi.node, xi.interface, index);
		return -1;
	}

	if (pko_mac_cfg.s.min_pad_ena == pad_enable &&
	    pko_mac_cfg.s.fcs_ena == fcs_enable) {
		if (debug)
			cvmx_dprintf("%s: mac %#x unchanged\n",
				__func__, mac_num);
		return 0;
	}

	/* WORKAROUND: Pass1 won't allow change any bits unless FIFO_NUM=0x1f */
	fifo_num = pko_mac_cfg.s.fifo_num;
	pko_mac_cfg.s.fifo_num = 0x1f;

	pko_mac_cfg.s.min_pad_ena = pad_enable;
	pko_mac_cfg.s.fcs_ena = fcs_enable;
	pko_mac_cfg.s.fcs_sop_off = fcs_sop_off;

	cvmx_write_csr_node(xi.node, CVMX_PKO_MACX_CFG(mac_num), pko_mac_cfg.u64);

	pko_mac_cfg.s.fifo_num = fifo_num;
	cvmx_write_csr_node(xi.node, CVMX_PKO_MACX_CFG(mac_num), pko_mac_cfg.u64);

	if (debug)
		cvmx_dprintf("%s: PKO_MAC[%u]CFG=%#llx\n",__func__,
			mac_num, (unsigned long long)
			cvmx_read_csr_node(xi.node, CVMX_PKO_MACX_CFG(mac_num)));

	return 0;
}
EXPORT_SYMBOL(cvmx_pko3_interface_options);

/** Set Descriptor Queue options
 *
 * The `min_pad` parameter must be in agreement with the interface-level
 * padding option for all descriptor queues assigned to that particular
 * interface/port.
 *
 * @param node on which to operate
 * @param dq descriptor queue to set
 * @param min_pad minimum padding to set for dq
 */
void cvmx_pko3_dq_options(unsigned node, unsigned dq, bool min_pad)
{
	cvmx_pko_pdm_dqx_minpad_t reg;

	dq &= (1<<10)-1;
	reg.u64 = cvmx_read_csr_node(node, CVMX_PKO_PDM_DQX_MINPAD(dq));
	reg.s.minpad = min_pad;
	cvmx_write_csr_node(node, CVMX_PKO_PDM_DQX_MINPAD(dq), reg.u64);
}

/**
 * Get number of PKO internal buffers available
 *
 * This function may be used to throttle output processing
 * when the PKO runs out of internal buffers, to avoid discarding
 * of packets or returning error results from transmission function.
 *
 * Returns negative numbers on error, positive number ti nidicate the
 * number of buffers available, or 0 when no more buffers are available.
 *
 * @INTERNAL
 */
int cvmx_pko3_internal_buffer_count(unsigned node)
{
	cvmx_pko_dpfi_fpa_aura_t pko_aura;
	unsigned laura, pool;
	long long avail1, avail2;

	/* get the aura number in pko, use aura node from parameter */
	pko_aura.u64 = cvmx_read_csr_node(node, CVMX_PKO_DPFI_FPA_AURA);
	laura = pko_aura.s.laura;

	/* form here on, node is the AURA node */
	node = pko_aura.s.node;

	/* get the POOL number for this AURA */
	pool = cvmx_read_csr_node(node, CVMX_FPA_AURAX_POOL(laura));

	avail1 = cvmx_read_csr_node(node, CVMX_FPA_POOLX_AVAILABLE(pool));

	avail2 = cvmx_read_csr_node(node, CVMX_FPA_AURAX_CNT_LIMIT(laura)) -
		cvmx_read_csr_node(node, CVMX_FPA_AURAX_CNT(laura));

	if (avail1 < avail2)
		return avail1;

	return avail2;
}

/**
 * @INTERNAL
 *
 * Get actual PKO FIFO buffer size for a given port
 *
 * Since the FIFOs are allocated dynamically based on supply/demand
 * heuristics, it may be useful in some instances to know the
 * actual FIFO size allocated to any specific port at run time.
 *
 * @param xiface global interface number
 * @param index port index on interface
 * @return Returns the per-port FIFO size in bytes, or 0 if the port
 * has not been configured, or a negative number if the interface and
 * index numbers are not valid.
 */
int
cvmx_pko3_port_fifo_size(unsigned int xiface, unsigned index)
{
	unsigned node;
	unsigned mac_num;
	unsigned fifo_grp, fifo_off;
	cvmx_pko_macx_cfg_t pko_mac_cfg;
	cvmx_pko_ptgfx_cfg_t pko_ptgfx_cfg;
        cvmx_xiface_t xi = cvmx_helper_xiface_to_node_interface(xiface);
	int ret;

	node = xi.node;
	ret = __cvmx_pko3_get_mac_num(xiface, index);
	if( ret < 0)
		return ret;

	if (debug)
		cvmx_dprintf("%s: iface=%u:%u/%u mac %d\n",
			__func__, xi.node, xi.interface, index, ret);

	mac_num = ret;

	/* Check for the special value on onused MACs */
	if (mac_num == 0x1f)
		return 0;

	pko_mac_cfg.u64 = cvmx_read_csr_node(node, CVMX_PKO_MACX_CFG(mac_num));

	fifo_grp = pko_mac_cfg.s.fifo_num >> 2;
	fifo_off = pko_mac_cfg.s.fifo_num & 0x3;
	pko_ptgfx_cfg.u64 = cvmx_read_csr_node(node,
		CVMX_PKO_PTGFX_CFG(fifo_grp));

	ret = (2 <<10) + (1 << 9);	/* set 2.5KBytes base FIFO size */

	switch (pko_ptgfx_cfg.s.size) {
		case 0:
			/* 2.5l, 2.5k, 2.5k, 2.5k */
			break;
		case 1:
			/* 5.0k, 0.0k, 2.5k, 2.5k */
			if (fifo_off == 1)
				ret = 0;
			if (fifo_off == 0)
				ret *= 2;
			break;
		case 2:
			/* 2.5k, 2.5k, 5.0k, 0.0k */
			if (fifo_off == 3)
				ret = 0;
			if (fifo_off == 2)
				ret *= 2;
			break;
		case 3:
			/* 5k, 0, 5k, 0 */
			if ((fifo_off & 1) != 0)
				ret = 0;
			ret *= 2;
			break;
		case 4:
			/* 10k, 0, 0, 0 */
			if (fifo_off != 0)
				ret = 0;
			ret *= 4;
			break;
		default:
			ret = -1;
	}
	return ret;
}
EXPORT_SYMBOL(cvmx_pko3_port_fifo_size);

/**
 * @INTERNAL
 *
 * Stop an interface port transmission and wait until its FIFO is empty.
 *
 */
int cvmx_pko3_port_xoff(unsigned int xiface, unsigned index)
{
	cvmx_pko_l1_sqx_topology_t pko_l1_topology;
	cvmx_pko_l1_sqx_sw_xoff_t pko_l1_xoff;
	cvmx_pko_ptfx_status_t pko_ptfx_status;
	cvmx_pko_macx_cfg_t pko_mac_cfg;
	cvmx_pko_mci1_cred_cntx_t cred_cnt;
	unsigned node, pq, num_pq, mac_num, fifo_num;
	int ret;
	cvmx_xiface_t xi = cvmx_helper_xiface_to_node_interface(xiface);

	node = xi.node;
	ret = __cvmx_pko3_get_mac_num(xiface, index);

	if (debug)
		cvmx_dprintf("%s: iface=%u:%u/%u mac %d\n",
			__func__, xi.node, xi.interface, index, ret);

	if (ret < 0)
		return ret;

	mac_num = ret;

	if (mac_num == 0x1f)
		return 0;

	pko_mac_cfg.u64 = cvmx_read_csr_node(node, CVMX_PKO_MACX_CFG(mac_num));
	fifo_num = pko_mac_cfg.s.fifo_num;

	/* Verify the FIFO number is correct */
	pko_ptfx_status.u64 = cvmx_read_csr_node(node,
			CVMX_PKO_PTFX_STATUS(fifo_num));

	if (debug)
		cvmx_dprintf("%s: mac %d fifo %d, fifo mac %d\n",
		__func__, mac_num, fifo_num, pko_ptfx_status.s.mac_num);

	cvmx_warn_if (pko_ptfx_status.s.mac_num != mac_num,
		"PKO3 FIFO number does not match MAC\n");

	num_pq = cvmx_pko3_num_level_queues(CVMX_PKO_PORT_QUEUES);
	/* Find the L1/PQ connected to the MAC for this interface */
	for (pq = 0; pq < num_pq; pq ++) {
		pko_l1_topology.u64 = cvmx_read_csr_node(node,
			CVMX_PKO_L1_SQX_TOPOLOGY(pq));
		if (pko_l1_topology.s.link == mac_num)
			break;
	}


	if (debug)
		cvmx_dprintf("%s: L1_PQ%u LINK %d MAC_NUM %d\n",
			__func__, pq, pko_l1_topology.s.link, mac_num);

	if (pq >= num_pq)
		return -1;

	if (debug) {
		pko_ptfx_status.u64 = cvmx_read_csr_node(node,
			CVMX_PKO_PTFX_STATUS(fifo_num));
		ret = pko_ptfx_status.s.in_flight_cnt;
		cvmx_dprintf("%s: FIFO %d in-flight %d packets\n",
		    __func__, fifo_num, ret);
	}

	/* Turn the XOFF bit on */
	pko_l1_xoff.u64 = cvmx_read_csr_node(node,
		CVMX_PKO_L1_SQX_SW_XOFF(pq));
	pko_l1_xoff.s.xoff = 1;
	cvmx_write_csr_node(node,
		CVMX_PKO_L1_SQX_SW_XOFF(pq), pko_l1_xoff.u64);

	ret = 1 << 22;
	/* Wait for PKO TX FIFO to drain */
	do {
		CVMX_SYNC;
		pko_ptfx_status.u64 = cvmx_read_csr_node(node,
			CVMX_PKO_PTFX_STATUS(fifo_num));
	} while (pko_ptfx_status.s.in_flight_cnt != 0 && ret--);

	if (pko_ptfx_status.s.in_flight_cnt != 0) {
		cvmx_warn("%s: FIFO %d failed to drain\n",
		    __func__, fifo_num);
	}

	if (debug)
		cvmx_dprintf("%s: FIFO %d drained in %d cycles\n",
		    __func__, fifo_num, (1 << 22) - ret);

	/* Wait for MAC TX FIFO to drain. */
	do {
		cred_cnt.u64 = cvmx_read_csr_node(node, CVMX_PKO_MCI1_CRED_CNTX(mac_num));
	} while (cred_cnt.s.cred_cnt != 0);

	return 0;
}

/**
 * @INTERNAL
 *
 * Resume transmission on an interface port.
 *
 */
int cvmx_pko3_port_xon(unsigned int xiface, unsigned index)
{
	cvmx_pko_l1_sqx_topology_t pko_l1_topology;
	cvmx_pko_l1_sqx_sw_xoff_t pko_l1_xoff;
	unsigned node, pq, num_pq, mac_num;
	int ret;
        cvmx_xiface_t xi = cvmx_helper_xiface_to_node_interface(xiface);

	num_pq = cvmx_pko3_num_level_queues(CVMX_PKO_PORT_QUEUES);
	node = xi.node;
	ret = __cvmx_pko3_get_mac_num(xiface, index);

	if (debug)
		cvmx_dprintf("%s: iface=%u:%u/%u mac %d\n",
			__func__, xi.node, xi.interface, index, ret);

	if (ret < 0)
		return ret;

	mac_num = ret;

	if (mac_num == 0x1f)
		return 0;

	/* Find the L1/PQ connected to the MAC for this interface */
	for (pq = 0; pq < num_pq; pq ++) {
		pko_l1_topology.u64 = cvmx_read_csr_node(node,
			CVMX_PKO_L1_SQX_TOPOLOGY(pq));
		if (pko_l1_topology.s.link == mac_num)
			break;
	}

	if (debug)
		cvmx_dprintf("%s: L1_PQ%u LINK %d MAC_NUM %d\n",
			__func__, pq, pko_l1_topology.s.link, mac_num);

	if (pq >= num_pq)
		return -1;

	/* Turn the XOFF bit off */
	pko_l1_xoff.u64 = cvmx_read_csr_node(node,
		CVMX_PKO_L1_SQX_SW_XOFF(pq));
	ret = pko_l1_xoff.s.xoff;
	pko_l1_xoff.s.xoff = 0;
	cvmx_write_csr_node(node,
		CVMX_PKO_L1_SQX_SW_XOFF(pq), pko_l1_xoff.u64);

	return ret;
}

/******************************************************************************
*
* New PKO3 API - Experimental
*
******************************************************************************/

/**
 * Initialize packet descriptor
 *
 * Desciptor storage is provided by the caller,
 * use this function to initialize the descriptor to a known
 * empty state.
 *
 * @param pdesc Packet Desciptor.
 *
 * Do not use this function when creating a descriptor from a
 * Work Queue Entry.
 *
 * The default setting of the 'free_bufs' attribute is 'false'.
 */
void cvmx_pko3_pdesc_init(cvmx_pko3_pdesc_t *pdesc)
{
	cvmx_pko_send_aura_t *ext_s;

	memset(pdesc, 0, sizeof(*pdesc));

	/* Start with HDR_S and HDR_EXT_S in first two words, all 0's */
	pdesc->num_words = 2;

	pdesc->hdr_s = (void *) &pdesc->word[0];
	ext_s = (void *) &pdesc->word[1];
	ext_s->s.subdc4 = CVMX_PKO_SENDSUBDC_EXT;

	pdesc->last_aura = -1;
	pdesc->jb_aura = -1;

	/* Empty packets, can not decode header offsets (yet) */
	pdesc->hdr_offsets = 1;
}

/**
 * Create a packet descriptor from WQE
 *
 * Populate a packet descriptor with a packet data and meta-data
 * located in the Work Queue Entry.
 * After this function, it is safe to call 'cvmx-wqe-free()'
 * to release the WQE buffer if separate from data buffers.
 * This function discards any data or meta-data that may have
 * been present in the packet descriptor previously, and does not
 * require the call to 'cvmx_pko3_pdesc_init()'.
 *
 * @param pdesc Packet Desciptor.
 * @param wqe Work Queue Entry as returned from `cvmx_get_work()'
 * @param free_bufs Automatically free data buffers when transmission complete.
 *
 * This function is the quickest way to prepare a received packet
 * represented by a WQE for transmission via any output queue to
 * an output port.
 * If the packet data is to be transmitted unmodified, call
 * 'cvmx_pko_pdesc_transmit()' immediately after this function
 * returns.
 */
int cvmx_pko3_pdesc_from_wqe(cvmx_pko3_pdesc_t *pdesc, cvmx_wqe_78xx_t *wqe,
	bool free_bufs)
{
	unsigned node;
	cvmx_pko_send_hdr_t *hdr_s;
	cvmx_pko_send_aura_t *ext_s;
	cvmx_pko_buf_ptr_t *buf_s;
	cvmx_buf_ptr_pki_t pki_bptr;
        cvmx_pki_stylex_buf_t     style_buf_reg;


	/* Verify the WQE is legit */
	if (cvmx_unlikely(wqe->word2.software || wqe->pki_wqe_translated)) {
		cvmx_printf("%s: ERROR: invalid WQE\n", __func__);
		return -1;
	}

	/* descriptor provided by caller, reset state */
	memset(pdesc, 0, sizeof(*pdesc));
	pdesc->jb_aura = -1;

	/* 1st word is SEND_HDR_S header */
	hdr_s = pdesc->hdr_s = (void *) &pdesc->word[0];
	/* 2nd word is the SEND_EXT_S header */
	ext_s = (void *) &pdesc->word[1];
	ext_s->s.subdc4 = CVMX_PKO_SENDSUBDC_EXT;
	pdesc->num_words = 2;

	hdr_s->s.format = 0;	/* Only 0 works for Pass1 */
	hdr_s->s.ds = 0;	/* don't send, never used */

        if(OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
		hdr_s->s.n2 = 0;	/* L2 allocate everything */
	else
		hdr_s->s.n2 = 1;	/* No L2 allocate works faster */

	/* Default buffer freeing setting, may be overriden by "i" */
	hdr_s->s.df = !free_bufs;

	/* Inherit GAURA */
	pdesc->last_aura =
	hdr_s->s.aura = wqe->word0.aura;

	/* Get the NODE on which this packet was received */
	node = pdesc->last_aura >> 10;

	/* Import total packet length */
	hdr_s->s.total = wqe->word1.len ;

	/* Read the PKI_STYLEX_BUF register for this packet style */
        style_buf_reg.u64 = cvmx_read_csr_node(node,
		CVMX_PKI_STYLEX_BUF(wqe->word0.style));

	/* mirror PKI endianness state: */
	hdr_s->s.le = style_buf_reg.s.pkt_lend;
#if	CVMX_ENABLE_PARAMETER_CHECKING
	if (hdr_s->s.le != __native_le)
		cvmx_printf("%s: WARNING: "
			"packet endianness mismatch\n",__func__);
#endif

#if 0 // WQE fields not used (yet?)
		wqe->word0.pki.pknd
		wqe->word0.pki.channel

		wqe->word1.cn78xx.tag
		wqe->word1.cn78xx.tag_type
		wqe->word1.cn78xx.grp
#endif

	/* Carry-over layer protocol detection from PKI */
	pdesc->pki_word2 = wqe->word2;

	/* check if WQE WORD4 is present */
	if (style_buf_reg.s.wqe_hsz != 0 || style_buf_reg.s.first_skip > 4) {
		pdesc->pki_word4_present = 1;
		/* Carry-over protocol header offsets */
		pdesc->pki_word4 = wqe->word4;
	}


	/* Checksum recalculation is not needed, until headers get modified */
	/* NOTE: Simulator does not support CKL3/CKL4, so this is not tested */
	hdr_s->s.ckl4 = CKL4ALG_NONE;
	hdr_s->s.ckl3 = 0;

	/* Convert WQE buffer ptr to LINK_S or GATHER_S bufptr in descriptor */
	pki_bptr = wqe->packet_ptr;
	buf_s = (void *) &pdesc->word[pdesc->num_words++];
	buf_s->u64 = 0;
	buf_s->s.addr = pki_bptr.addr;
	buf_s->s.size = pki_bptr.size;

	/* use LINK_S if more than one buf present, calculate headroom */
	if (cvmx_unlikely(wqe->word0.bufs > 1)) {
		pdesc->headroom =  (style_buf_reg.s.first_skip) << 3;
		buf_s->s.subdc3 = CVMX_PKO_SENDSUBDC_LINK;
	} else {
		pdesc->headroom =  (1 + style_buf_reg.s.first_skip) << 3;
		buf_s->s.subdc3 = CVMX_PKO_SENDSUBDC_GATHER;
	}
	pdesc->headroom += wqe->word0.apad;

	return 0;
}

/**
 * @INTERNAL
 *
 * Add arbitrary subcommand to a packet desciptor.
 *
 * This function will also allocate a jump buffer when
 * the primary LMTDMA buffer is exhausted.
 * The jump buffer is allocated from the internal PKO3 aura
 * on the node where this function is running.
 */
static int cvmx_pko3_pdesc_subdc_add(cvmx_pko3_pdesc_t *pdesc,
		uint64_t subdc)
{
	cvmx_pko_send_hdr_t *hdr_s;
	cvmx_pko_send_aura_t *ext_s;
	cvmx_pko_buf_ptr_t *jump_s;
	const unsigned jump_buf_size = 4*1024 / sizeof(uint64_t);
	unsigned i;

	/* Simple handling while fitting the command buffer */
	if (cvmx_likely(pdesc->num_words < 15 && pdesc->jump_buf == NULL)) {
		pdesc->word[ pdesc->num_words ] = subdc;
		pdesc->num_words ++;
		return pdesc->num_words;
	}

        /* SEND_JUMP_S missing on Pass1 */
        if(OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)) {
                cvmx_printf("%s: ERROR: too many segments\n",__func__);
                return -E2BIG;
        }

	hdr_s = (void *) &pdesc->word[0];
	ext_s = (void *) &pdesc->word[1];

	/* Allocate jump buffer */
	if (cvmx_unlikely(pdesc->jump_buf == NULL)) {
		uint16_t pko_gaura;
		cvmx_fpa3_gaura_t aura;
		unsigned fpa_node = cvmx_get_node_num();

		/* Allocate jump buffer from PKO internal FPA AURA, size=4KiB */
		pko_gaura = __cvmx_pko3_aura_get(fpa_node);
		aura = __cvmx_fpa3_gaura(pko_gaura >> 10, pko_gaura & 0x3ff);

		pdesc->jump_buf = cvmx_fpa3_alloc(aura);
                if(pdesc->jump_buf == NULL)
                        return -EINVAL;

		/* Save the JB aura for later */
		pdesc->jb_aura = pko_gaura;

		/* Move most of the command to the jump buffer */
		memcpy(pdesc->jump_buf, &pdesc->word[2],
			(pdesc->num_words-2)*sizeof(uint64_t));
		jump_s = (void *) &pdesc->word[2];
		jump_s->u64 = 0;
		jump_s->s.addr = cvmx_ptr_to_phys(pdesc->jump_buf);
		jump_s->s.i = !hdr_s->s.df;	/* F= ~DF */
		jump_s->s.size = pdesc->num_words - 2;
		jump_s->s.subdc3 = CVMX_PKO_SENDSUBDC_JUMP;

		/* Now the LMTDMA buffer has only HDR_S, EXT_S, JUMP_S */
		pdesc->num_words = 3;
	}

	/* Add the new subcommand to the jump buffer */
	jump_s = (void *) &pdesc->word[2];
	i = jump_s->s.size;

	/* Avoid overrunning jump buffer */
	if (i >= (jump_buf_size-2)) {
                cvmx_printf("%s: ERROR: too many segments\n",__func__);
		return -E2BIG;
	}

	pdesc->jump_buf[i] = subdc;
	jump_s->s.size++;

	(void) ext_s;

	return(i + pdesc->num_words);
}

/**
 * Send a packet in a desciptor to an output port via an output queue.
 *
 * A call to this function must follow all other functions that
 * create a packet descriptor from WQE, or after initializing an
 * empty descriptor and filling it with one or more data fragments.
 * After this function is called, the content of the packet descriptor
 * can no longer be used, and are undefined.
 *
 * @param pdesc Packet Desciptor.
 * @param dq Descriptor Queue associated with the desired output port
 * @param tag Flow Tag pointer for packet ordering or NULL
 * @return Returns 0 on success, -1 on error.
 *
 */
int cvmx_pko3_pdesc_transmit(cvmx_pko3_pdesc_t *pdesc, uint16_t dq,
	uint32_t *tag)
{
        cvmx_pko_query_rtn_t pko_status;
	cvmx_pko_send_aura_t aura_s;
	uint8_t port_node;
	int rc;

	/* Add last AURA_S for jump_buf, if present */
	if (cvmx_unlikely(pdesc->jump_buf != NULL) &&
	    (pdesc->last_aura != pdesc->jb_aura)) {
		/* The last AURA_S subdc refers to the jump_buf itself */
		aura_s.s.aura = pdesc->jb_aura;
		aura_s.s.offset = 0;
		aura_s.s.alg = AURAALG_NOP;
		aura_s.s.subdc4 = CVMX_PKO_SENDSUBDC_AURA;
		pdesc->last_aura = pdesc->jb_aura;

		rc = cvmx_pko3_pdesc_subdc_add(pdesc, aura_s.u64);
		if (rc < 0)
			return -1;
	}

	/* SEND_WORK_S must be the very last subdc */
	if (cvmx_unlikely(pdesc->send_work_s != 0ULL)) {
		rc = cvmx_pko3_pdesc_subdc_add(pdesc, pdesc->send_work_s);
		if (rc < 0)
			return -1;
		pdesc->send_work_s = 0ULL;
	}

        /* Derive destination node from dq */
	port_node = dq >> 10;
	dq &= (1<<10)-1;

	/* To preserve packet order, go atomic with DQ-specific tag */
	if (tag != NULL)
		cvmx_pow_tag_sw(*tag ^ dq, CVMX_POW_TAG_TYPE_ATOMIC);

        /* Send the PKO3 command into the Descriptor Queue */
        pko_status = __cvmx_pko3_do_dma(port_node, dq,
                pdesc->word, pdesc->num_words, CVMX_PKO_DQ_SEND);

        /* Map PKO3 result codes to legacy return values */
        if (pko_status.s.dqstatus == PKO_DQSTATUS_PASS)
                return 0;

#if 0
        cvmx_printf("%s: ERROR: failed to enqueue: %s\n",
                                __FUNCTION__,
                                pko_dqstatus_error(pko_status.s.dqstatus));
#endif

	return -1;
}

int cvmx_pko3_pdesc_append_free(cvmx_pko3_pdesc_t *pdesc, uint64_t addr,
			     unsigned gaura)
{
	cvmx_pko_send_hdr_t *hdr_s;
	cvmx_pko_send_free_t free_s;
	cvmx_pko_send_aura_t aura_s;

	hdr_s = (void *) &pdesc->word[0];

	if (pdesc->last_aura == -1) {
		pdesc->last_aura = hdr_s->s.aura = gaura;
	} else if (pdesc->last_aura != (short) gaura) {
		aura_s.s.aura = gaura;
		aura_s.s.offset = 0;
		aura_s.s.alg = AURAALG_NOP;
		aura_s.s.subdc4 = CVMX_PKO_SENDSUBDC_AURA;
		pdesc->last_aura = gaura;
		if (cvmx_pko3_pdesc_subdc_add(pdesc, aura_s.u64) < 0)
			return -1;
	}

	free_s.u64 = 0;
	free_s.s.subdc4 = CVMX_PKO_SENDSUBDC_FREE;
	free_s.s.addr = addr;

	return cvmx_pko3_pdesc_subdc_add(pdesc, free_s.u64);
}

/**
 * Append a packet segment to a packet descriptor
 *
 * After a packet descriptor is initialized, one or more
 * packet data segments can be added to the packet,
 * in the order in which they should be transmitted.
 *
 * The size of the resulting packet will be equal to the
 * sum of the segments appended by this function.
 * Every segment may be contained in a buffer that belongs
 * to a different FPA 'aura', and may be automatically
 * released back to that aura, if required.
 *
 * @param pdesc Packet Desciptor.
 * @param p_data Address of the segment first byte (virtual).
 * @param data_bytes Size of the data segment (in bytes).
 * @param gaura A global FPA 'aura' where the packet buffer was allocated from.
 *
 * The 'gaura' parameter contains the node number where the buffer pool
 * is located, and has only a meaning if the 'free_buf' argument is 'true'.
 * The buffer being added will be automatically freed upon transmission
 * along with all other buffers in this descriptor, or not, depending
 * on the descriptor 'free_bufs' attribute that is set during
 * descriptor creation, or changed subsequently with a call to
 * 'cvmx_pko3_pdesc_set_free()'.
 *
 * @return Returns 0 on success, -1 on error.
 */
int cvmx_pko3_pdesc_buf_append(cvmx_pko3_pdesc_t *pdesc, void *p_data,
		unsigned data_bytes, unsigned gaura)
{
	cvmx_pko_send_hdr_t *hdr_s;
	cvmx_pko_buf_ptr_t gather_s;
	cvmx_pko_send_aura_t aura_s;
	int rc;

	if (pdesc->mem_s_ix > 0) {
		cvmx_printf("ERROR: %s: subcommand restriction violated\n", __func__);
		return -1;
	}

	hdr_s = (void *) &pdesc->word[0];

	if (gaura != (unsigned)-1) {
	    if (pdesc->last_aura == -1 ) {
		unsigned buf_sz = 128;

		/* First mbuf, calculate headroom */
#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
		cvmx_fpa3_gaura_t aura;
		aura = __cvmx_fpa3_gaura(gaura >> 10, gaura & 0x3ff);
                buf_sz = cvmx_fpa3_get_aura_buf_size(aura);
#endif
		pdesc->headroom = (unsigned long)p_data & (buf_sz-1);
		pdesc->last_aura = hdr_s->s.aura = gaura;
	    } else if(pdesc->last_aura != (short) gaura) {
		aura_s.s.aura = gaura;
		aura_s.s.offset = 0;
		aura_s.s.alg = AURAALG_NOP;
		aura_s.s.subdc4 = CVMX_PKO_SENDSUBDC_AURA;
		pdesc->last_aura = gaura;

		rc = cvmx_pko3_pdesc_subdc_add(pdesc, aura_s.u64);
		if (rc < 0)
			return -1;
	    }
	}

	gather_s.u64 = 0;
	gather_s.s.addr = cvmx_ptr_to_phys(p_data);
	gather_s.s.size = data_bytes;
	hdr_s->s.total += data_bytes;
	gather_s.s.i = 0;	/* follow HDR_S[DF] setting */
	gather_s.s.subdc3 = CVMX_PKO_SENDSUBDC_GATHER;

	rc = cvmx_pko3_pdesc_subdc_add(pdesc, gather_s.u64);
	if (rc < 0)
		return -1;

	return rc;
}

/**
 * Add a Work Entry for packet transmission notification
 *
 * Add a subcommand to notify of packet transmission completion
 * via a Work Queue entry over the SSO.
 * The Work Queue entry may be a 'software' event, or the content
 * of a packet.
 *
 * @param pdesc Packet Desciptor, memory provided by caller.
 * @param wqe Work Queue Entry in a model-native format.
 * @param node The OCI node of the SSO where the WQE will be delivered.
 * @param group The SSO group where the WQE is delivered.
 * @param tt The SSO Tag Type for the WQE. If tt is not NULL, tag should be a
 * valid tag value.
 * @param tag Valid tag value to assign to WQE
 *
 * @return Returns 0 on success, -1 on error.
 *
 * Restrictions:
 * There can be only one such notification per packet descriptor,
 * but this function may be called at any time after the descriptor
 * is first created from WQE or initialized, and before
 * starting transmission.
 *
 */
int cvmx_pko3_pdesc_notify_wqe(cvmx_pko3_pdesc_t *pdesc, cvmx_wqe_78xx_t *wqe,
	uint8_t node, uint8_t group, uint8_t tt, uint32_t tag)
{
	cvmx_pko_send_work_t work_s;

	/*
	 * There can be only one SEND_WORK_S entry in the command
	 * and it must be the very last subcommand
	 */
	if (pdesc->send_work_s != 0) {
		cvmx_printf("ERROR: %s: Only one SEND_WORK_S is allowed\n", __func__);
		return -1;
	}

	work_s.u64 = 0;
	work_s.s.subdc4 = CVMX_PKO_SENDSUBDC_WORK;
	work_s.s.addr = cvmx_ptr_to_phys(wqe);
	work_s.s.grp = (group & 0xff) | (node  << 8);
	work_s.s.tt = tt;

	wqe->word1.rsvd_0 = 0;
	wqe->word1.rsvd_1 = 0;
	wqe->word1.tag = tag;
	wqe->word1.tag_type = tt;
	wqe->word1.grp = work_s.s.grp;

	/* Store in descriptor for now, apply just before LMTDMA-ing */
	pdesc->send_work_s = work_s.u64;

	return 0;
}

/**
 * Request atomic memory decrement at transmission completion
 *
 * Each packet descriptor may contain several decrement notification
 * requests, but these requests must only be made after all of the
 * packet data segments have been added, and before packet transmission
 * commences.
 *
 * Only decrement of a 64-bit memory location is supported.
 *
 * @param pdesc Packet Descriptor.
 * @param p_counter A pointer to an atomic 64-bit memory location.
 *
 * @return Returns 0 on success, -1 on failure.
 */
int cvmx_pko3_pdesc_notify_decrement(cvmx_pko3_pdesc_t *pdesc,
	volatile uint64_t *p_counter)
{
	int rc;
	/* 64-bit decrement is the only supported operation */
	cvmx_pko_send_mem_t mem_s = {.s={
		.subdc4 = CVMX_PKO_SENDSUBDC_MEM,
		.dsz = MEMDSZ_B64, .alg = MEMALG_SUB,
		.offset = 1,
#ifdef	_NOT_IN_SIM_
		/* Enforce MEM before SSO submission if both present */
		.wmem = 1
#endif
		}};

	mem_s.s.addr = cvmx_ptr_to_phys(CASTPTR(void,p_counter));



	rc = cvmx_pko3_pdesc_subdc_add(pdesc, mem_s.u64);

	/*
	 * SEND_MEM_S must be after all LINK_S/FATHER_S/IMM_S
	 * subcommands, set the index to prevent further data
	 * subcommands.
	 */
	if (rc > 0)
		pdesc->mem_s_ix = rc;

	return rc;
}

/**
 * Request atomic memory clear at transmission completion
 *
 * Each packet descriptor may contain several notification
 * requests, but these request must only be made after all of the
 * packet data segments have been added, and before packet transmission
 * commences.
 *
 * Clearing of a single byte is requested by this function.
 *
 * @param pdesc Packet Descriptor.
 * @param p_mem A pointer to a byte location.
 *
 * @return Returns 0 on success, -1 on failure.
 */
int cvmx_pko3_pdesc_notify_memclr(cvmx_pko3_pdesc_t *pdesc,
	volatile uint8_t *p_mem)
{
	int rc;
	/* 640bit decrement is the only supported operation */
	cvmx_pko_send_mem_t mem_s = {.s={
		.subdc4 = CVMX_PKO_SENDSUBDC_MEM,
		.dsz = MEMDSZ_B8, .alg = MEMALG_SET,
		.offset = 0,
		}};

	mem_s.s.addr = cvmx_ptr_to_phys(CASTPTR(void,p_mem));

	rc = cvmx_pko3_pdesc_subdc_add(pdesc, mem_s.u64);

	/*
	 * SEND_MEM_S must be after all LINK_S/FATHER_S/IMM_S
	 * subcommands, set the index to prevent further data
	 * subcommands.
	 */
	if (rc > 0)
		pdesc->mem_s_ix = rc;

	return rc;
}


/**
 * @INTERNAL
 *
 * Decode packet header and calculate protocol header offsets
 *
 * The protocol information and layer offset is derived
 * from the results if decoding done by the PKI,
 * and the appropriate PKO fields are filled.
 *
 * The function assumes the headers have not been modified
 * since converted from WQE, and does not (yet) implement
 * software-based decoding to handle modified or originated
 * packets correctly.
 *
 * @note
 * Need to add simple accessors to read the decoded protocol fields.
 */
static int cvmx_pko3_pdesc_hdr_offsets(cvmx_pko3_pdesc_t *pdesc)
{
	cvmx_pko_send_hdr_t *hdr_s;

	if (pdesc->hdr_offsets)
		return 0;

	if (!pdesc->pki_word4_present)
		return -EINVAL;

	pdesc->hdr_s = hdr_s = (void *) &pdesc->word[0];

	/* Match IPv5/IPv6 protocols with/without options */
	if ((pdesc->pki_word2.lc_hdr_type & 0x1c)
		== CVMX_PKI_LTYPE_E_IP4) {
		hdr_s->s.l3ptr = pdesc->pki_word4.ptr_layer_c;

		/* Match TCP/UDP/SCTP group */
		if ((pdesc->pki_word2.lf_hdr_type & 0x18) == CVMX_PKI_LTYPE_E_TCP)
			hdr_s->s.l4ptr = pdesc->pki_word4.ptr_layer_f;

		if (pdesc->pki_word2.lf_hdr_type == CVMX_PKI_LTYPE_E_UDP)
			pdesc->ckl4_alg = CKL4ALG_UDP;
		if (pdesc->pki_word2.lf_hdr_type == CVMX_PKI_LTYPE_E_TCP)
			pdesc->ckl4_alg = CKL4ALG_TCP;
		if (pdesc->pki_word2.lf_hdr_type == CVMX_PKI_LTYPE_E_SCTP)
			pdesc->ckl4_alg = CKL4ALG_SCTP;
	}
	/* May need to add logic for ARP, IPfrag packets here */

	pdesc->hdr_offsets = 1;	/* make sure its done once */
	return 0;
}

/*
 * @INTERNAL
 *
 * memcpy() a reverse endian memory region.
 * where both the source and destination are the reverse endianness
 * with respect to native byte order.
 */
static void memcpy_swap(void *dst, const void *src, unsigned bytes)
{
	uint8_t *d = dst;
	const uint8_t *s = src;
	unsigned i;
	const unsigned swizzle = 0x7;	/* 64-bit invariant endianness */

	for(i = 0; i < bytes; i++)
		d[i ^ swizzle] = s[i ^ swizzle];
}

/*
 * @INTERNAL
 *
 * memcpy() with swizzling, from reverse endianness to native byte order.
 */
static void memcpy_from_swap(void *dst, const void *src, unsigned bytes)
{
	uint8_t *d = dst;
	const uint8_t *s = src;
	unsigned i;
	const unsigned swizzle = 0x7;	/* 64-bit invariant endianness */

	for(i = 0; i < bytes; i++)
		d[i] = s[i ^ swizzle];
}

/*
 * @INTERNAL
 *
 * memcpy() with swizzling, from native byte order to the reverse endianness.
 */
static void memcpy_to_swap(void *dst, const void *src, unsigned bytes)
{
	uint8_t *d = dst;
	const uint8_t *s = src;
	unsigned i;
	const unsigned swizzle = 0x7;	/* 64-bit invariant endianness */

	for(i = 0; i < bytes; i++)
		d[i ^ swizzle] = s[i];
}

/**
 * Prepend a data segment to the packet descriptor
 *
 * Useful for pushing additional headers
 *
 * The initial implementation is confined by the size of the
 * "headroom" in the first packet buffer attached to the descriptor.
 * Future version may prepend additional buffers when this head room
 * is insufficient, but currently will return -1 when headrom is
 * insufficient.
 *
 * On success, the function returns the remaining headroom in the buffer.
 *
 */
int cvmx_pko3_pdesc_hdr_push(cvmx_pko3_pdesc_t *pdesc,
	const void *p_data, uint8_t data_bytes, uint8_t layer)
{
	cvmx_pko_send_hdr_t *hdr_s;
	cvmx_pko_buf_ptr_t *gather_s;
	short headroom;
	void *p;	/* old data location */
	void *q;	/* new data location */
	bool endian_swap;

	headroom = pdesc->headroom;

	if ((short)data_bytes > headroom)
		return -ENOSPC;

	hdr_s = (void *)&pdesc->word[0];
	endian_swap = (hdr_s->s.le != __native_le);

	/* Get GATTHER_S/LINK_S subcommand location */
	if (cvmx_likely(pdesc->jump_buf == NULL))
		/* Without JB, first data buf is in 3rd command word */
		gather_s = (void *)&pdesc->word[2];
	else
		/* With JB, its first word is the first buffer */
		gather_s = (void *)pdesc->jump_buf;

	/* Verify the subcommand is of the expected type */
	if (cvmx_unlikely(gather_s->s.subdc3 != CVMX_PKO_SENDSUBDC_LINK &&
			gather_s->s.subdc3 != CVMX_PKO_SENDSUBDC_GATHER))
		return -EINVAL;

	/* adjust  address and size values */
	p = cvmx_phys_to_ptr(gather_s->s.addr);
	q			= p - data_bytes;
	gather_s->s.addr	-= data_bytes;
	gather_s->s.size	+= data_bytes;
	hdr_s->s.total		+= data_bytes;
	headroom		-= data_bytes;

	/* Move link pointer if the descriptor is SEND_LINK_S */
	if (gather_s->s.subdc3 == CVMX_PKO_SENDSUBDC_LINK) {
		if (cvmx_likely(!endian_swap))
			memcpy(q-8, p-8, 8);
		else
			memcpy_swap(q-8, p-8, 8);
	}

	if (cvmx_likely(!endian_swap))
		memcpy(q, p_data, data_bytes);
	else
		memcpy_to_swap(q, p_data, data_bytes);

	pdesc->headroom = headroom;

	/* Adjust higher level protocol header offset */
	cvmx_pko3_pdesc_hdr_offsets(pdesc);
	if (layer <= 4 ) {
		pdesc->hdr_s->s.l4ptr += data_bytes;
	}

	if (layer <= 3) {
		pdesc->hdr_s->s.l3ptr += data_bytes;
	}

	if (layer >= 3) {
		/* Set CKL3 only for IPv4 */
		if ((pdesc->pki_word2.lc_hdr_type & 0x1e)
			== CVMX_PKI_LTYPE_E_IP4)
			hdr_s->s.ckl3 = 1;
		hdr_s->s.ckl4 = pdesc->ckl4_alg;
	}

	return headroom;
}


/**
 * Remove some bytes from start of packet
 *
 * Useful for popping a header from a packet.
 * It only needs to find the first segment, and adjust its address,
 * as well as segment and total sizes.
 *
 * Returns new packet size, or -1 if the trimmed size exceeds the
 * size of the first data segment.
 */
int cvmx_pko3_pdesc_hdr_pop(cvmx_pko3_pdesc_t *pdesc,
		void *hdr_buf, unsigned num_bytes)
{
	cvmx_pko_send_hdr_t *hdr_s;
	cvmx_pko_buf_ptr_t *gather_s;
	short headroom;
	void *p;
	void *q;
	bool endian_swap;

	headroom = pdesc->headroom;

	hdr_s = (void *)&pdesc->word[0];
	endian_swap = (hdr_s->s.le != __native_le);

	if (hdr_s->s.total < num_bytes)
		return -ENOSPC;

	/* Get GATTHER_S/LINK_S subcommand location */
	if (cvmx_likely(pdesc->jump_buf == NULL))
		/* Without JB, first data buf is in 3rd command word */
		gather_s = (void *)&pdesc->word[2];
	else
		/* With JB, its first word is the first buffer */
		gather_s = (void *)pdesc->jump_buf;

	/* Verify the subcommand is of the expected type */
	if (cvmx_unlikely(gather_s->s.subdc3 != CVMX_PKO_SENDSUBDC_LINK &&
			gather_s->s.subdc3 != CVMX_PKO_SENDSUBDC_GATHER))
		return -EINVAL;

	/* Can't trim more than the content of the first buffer */
	if (gather_s->s.size < num_bytes)
		return -ENOMEM;

	/* adjust  address and size values */
	p = cvmx_phys_to_ptr(gather_s->s.addr);
	q			= p + num_bytes;
	gather_s->s.addr	+= num_bytes;
	gather_s->s.size	-= num_bytes;
	hdr_s->s.total		-= num_bytes;
	headroom		+= num_bytes;

	if (hdr_buf != NULL) {
		/* Retreive popped header to user buffer */
		if (cvmx_likely(!endian_swap)) {
			memcpy(hdr_buf, p, num_bytes);
		} else {
			memcpy_from_swap(hdr_buf, p, num_bytes);
		}
	}

	/* Move link pointer if the descriptor is SEND_LINK_S */
	if (gather_s->s.subdc3 == CVMX_PKO_SENDSUBDC_LINK) {
		if (cvmx_likely(!endian_swap))
			memcpy(q-8, p-8, 8);
		else
			memcpy_swap(q-8, p-8, 8);
	}

	pdesc->headroom = headroom;

	/* Adjust higher level protocol header offset */
	cvmx_pko3_pdesc_hdr_offsets(pdesc);
	if (num_bytes < pdesc->hdr_s->s.l3ptr) {
		pdesc->hdr_s->s.l3ptr -= num_bytes;
		pdesc->hdr_s->s.l4ptr -= num_bytes;
	} else if (num_bytes < pdesc->hdr_s->s.l4ptr) {
		pdesc->hdr_s->s.l3ptr = 0;
		pdesc->hdr_s->s.l4ptr -= num_bytes;
	} else {
		pdesc->hdr_s->s.l3ptr = 0;
		pdesc->hdr_s->s.l4ptr = 0;
		hdr_s->s.ckl4 = CKL4ALG_NONE;
	}

	return hdr_s->s.total;
}

/**
 * Peek into some header field of a packet
 *
 * Will return a number of bytes of packet header data at an arbitrary offset
 * which must reside within the first packet data buffer.
 *
 */
int cvmx_pko3_pdesc_hdr_peek(cvmx_pko3_pdesc_t *pdesc,
		void *hdr_buf, unsigned num_bytes, unsigned offset)
{
	cvmx_pko_send_hdr_t *hdr_s;
	cvmx_pko_buf_ptr_t *gather_s;
	void *p;
	bool endian_swap;

	hdr_s = (void *)&pdesc->word[0];
	endian_swap = (hdr_s->s.le != __native_le);

	if (hdr_s->s.total < (num_bytes+offset))
		return -ENOSPC;

	/* Get GATTHER_S/LINK_S subcommand location */
	if (cvmx_likely(pdesc->jump_buf == NULL))
		/* Without JB, first data buf is in 3rd command word */
		gather_s = (void *)&pdesc->word[2];
	else
		/* With JB, its first word is the first buffer */
		gather_s = (void *)pdesc->jump_buf;

	/* Verify the subcommand is of the expected type */
	if (cvmx_unlikely(gather_s->s.subdc3 != CVMX_PKO_SENDSUBDC_LINK &&
			gather_s->s.subdc3 != CVMX_PKO_SENDSUBDC_GATHER))
		return -EINVAL;

	/* Can't peek more than the content of the first buffer */
	if (gather_s->s.size <= offset)
		return -ENOMEM;
	if ((gather_s->s.size-offset) < num_bytes)
		num_bytes = gather_s->s.size-offset;

	/* adjust address */
	p = cvmx_phys_to_ptr(gather_s->s.addr) + offset;

	if (hdr_buf == NULL)
		return -EINVAL;

	/* Copy requested bytes */
	if (cvmx_likely(!endian_swap)) {
		memcpy(hdr_buf, p, num_bytes);
	} else {
		memcpy_from_swap(hdr_buf, p, num_bytes);
	}

	return num_bytes;
}

/**
 * Set the packet descriptor automatic-free attribute
 *
 * Override the 'free_bufs' attribute that was set during
 * packet descriptor creation, or by an earlier call to
 * this function.
 * Setting the 'buf_free" attribute to 'true' will cause
 * the PKO3 to free all buffers associated with this packet
 * descriptor to be released upon transmission complete.
 * Setting this attribute to 'false' allows e.g. using the
 * same descriptor to transmit a packet out of several ports
 * with a minimum overhead.
 */
void cvmx_pko3_pdesc_set_free(cvmx_pko3_pdesc_t *pdesc, bool free_bufs)
{
	cvmx_pko_send_hdr_t *hdr_s;
	cvmx_pko_buf_ptr_t *jump_s;

	hdr_s = (void *)&pdesc->word[0];
	hdr_s->s.df = !free_bufs;

	if (cvmx_likely(pdesc->jump_buf == NULL))
		return;
	jump_s = (void *) &pdesc->word[2];
	jump_s->s.i = free_bufs; /* F=free */
}
