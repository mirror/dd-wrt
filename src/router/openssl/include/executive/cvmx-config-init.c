/***********************license start***************
 * Copyright (c) 2003-2010  Cavium Inc. (support@cavium.com). All rights
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
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-helper.h>
#include <asm/octeon/cvmx-helper-util.h>
#include <asm/octeon/cvmx-helper-cfg.h>
#include <asm/octeon/cvmx-pko.h>
#include <asm/octeon/cvmx-config.h>
#include <asm/octeon/cvmx-ipd.h>
#include <asm/octeon/cvmx-ilk.h>
#include <asm/octeon/cvmx-tim.h>
#include <asm/octeon/cvmx-zip.h>
#include <asm/octeon/cvmx-dma-engine.h>
#include <asm/octeon/cvmx-dfa.h>
#include <asm/octeon/cvmx-raid.h>
#include <asm/octeon/cvmx-bch.h>
#else
#include "cvmx.h"
#include "cvmx-pko.h"
#include "cvmx-helper-cfg.h"
#include "cvmx-ipd.h"
#include "cvmx-helper-spi.h"
#include "cvmx-llm.h"
#include "cvmx-ilk.h"
#include "cvmx-tim.h"
#include "cvmx-zip.h"
#include "cvmx-dma-engine.h"
#include "cvmx-dfa.h"
#include "cvmx-raid.h"
#include "cvmx-bch.h"
#endif

#ifdef CVMX_BUILD_FOR_UBOOT
#define USE_CVMX_CONFIG_H
#endif

#ifdef USE_CVMX_CONFIG_H
#include "cvmx-config.h"
#include "executive-config.h"
#include "cvmx-helper-check-defines.h"

#ifndef CVMX_FPA_OUTPUT_BUFFER_POOL
#define CVMX_FPA_OUTPUT_BUFFER_POOL -1
#define CVMX_FPA_OUTPUT_BUFFER_POOL_SIZE 0
#endif

#ifdef CVMX_ENABLE_HELPER_FUNCTIONS

#ifndef CVMX_HELPER_PKO_QUEUES_PER_PORT_INTERFACE0
#define CVMX_HELPER_PKO_QUEUES_PER_PORT_INTERFACE0 1
#endif

#ifndef CVMX_HELPER_PKO_QUEUES_PER_PORT_INTERFACE1
#define CVMX_HELPER_PKO_QUEUES_PER_PORT_INTERFACE1 1
#endif

#ifndef CVMX_HELPER_PKO_QUEUES_PER_PORT_INTERFACE2
#define CVMX_HELPER_PKO_QUEUES_PER_PORT_INTERFACE2 1
#endif

#ifndef CVMX_HELPER_PKO_QUEUES_PER_PORT_INTERFACE3
#define CVMX_HELPER_PKO_QUEUES_PER_PORT_INTERFACE3 1
#endif

#ifndef CVMX_HELPER_PKO_QUEUES_PER_PORT_INTERFACE4
#define CVMX_HELPER_PKO_QUEUES_PER_PORT_INTERFACE4 1
#endif

#ifndef CVMX_PKO_QUEUES_PER_PORT_LOOP
#define CVMX_PKO_QUEUES_PER_PORT_LOOP 1
#endif

#ifndef CVMX_PKO_QUEUES_PER_PORT_PCI
#define CVMX_PKO_QUEUES_PER_PORT_PCI 1
#endif

#ifndef CVMX_HELPER_PKO_MAX_PORTS_INTERFACE0
#define CVMX_HELPER_PKO_MAX_PORTS_INTERFACE0 16
#endif
#ifndef CVMX_HELPER_PKO_MAX_PORTS_INTERFACE1
#define CVMX_HELPER_PKO_MAX_PORTS_INTERFACE1 16
#endif
#ifndef CVMX_PKO_QUEUES_PER_PORT_SRIO0
	/* We use two queues per port for SRIO0. Having two queues per
	   port with two ports gives us four queues, one for each mailbox */
#define CVMX_PKO_QUEUES_PER_PORT_SRIO0 2
#endif
#ifndef CVMX_PKO_QUEUES_PER_PORT_SRIO1
	/* We use two queues per port for SRIO1. Having two queues per
	   port with two ports gives us four queues, one for each mailbox */
#define CVMX_PKO_QUEUES_PER_PORT_SRIO1 2
#endif
#ifndef CVMX_PKO_QUEUES_PER_PORT_SRIO2
	/* We use two queues per port for SRIO2. Having two queues per
	   port with two ports gives us four queues, one for each mailbox */
#define CVMX_PKO_QUEUES_PER_PORT_SRIO2 2
#endif
#ifndef CVMX_PKO_QUEUES_PER_PORT_SRIO3
	/* We use two queues per port for SRIO3. Having two queues per
	   port with two ports gives us four queues, one for each mailbox */
#define CVMX_PKO_QUEUES_PER_PORT_SRIO3 2
#endif

#ifndef CVMX_PKO_QUEUES_PER_PORT_INTERFACE2
#define CVMX_PKO_QUEUES_PER_PORT_INTERFACE2 1
#endif

#ifndef CVMX_PKO_QUEUES_PER_PORT_INTERFACE3
#define CVMX_PKO_QUEUES_PER_PORT_INTERFACE3 1
#endif

#ifndef CVMX_PKO_QUEUES_PER_PORT_INTERFACE4
#define CVMX_PKO_QUEUES_PER_PORT_INTERFACE4 1
#endif

int cvmx_pko_queue_static_config_non_pknd(void)
{
	int ports;
	ports = cvmx_helper_interface_enumerate(0);
	__cvmx_pko_queue_static_config.non_pknd.pko_ports_per_interface[0] = ports < CVMX_PKO_MAX_PORTS_INTERFACE0 ? ports : CVMX_PKO_MAX_PORTS_INTERFACE0;
	__cvmx_pko_queue_static_config.non_pknd.pko_queues_per_port_interface[0] =
		CVMX_PKO_QUEUES_PER_PORT_INTERFACE0;
	ports = cvmx_helper_interface_enumerate(1);
	__cvmx_pko_queue_static_config.non_pknd.pko_ports_per_interface[1] = ports < CVMX_PKO_MAX_PORTS_INTERFACE0 ? ports : CVMX_PKO_MAX_PORTS_INTERFACE1;
	__cvmx_pko_queue_static_config.non_pknd.pko_queues_per_port_interface[1] =
		CVMX_PKO_QUEUES_PER_PORT_INTERFACE1;
	__cvmx_pko_queue_static_config.non_pknd.pko_queues_per_port_pci = CVMX_PKO_QUEUES_PER_PORT_PCI;
	__cvmx_pko_queue_static_config.non_pknd.pko_queues_per_port_loop = CVMX_PKO_QUEUES_PER_PORT_LOOP;
	__cvmx_pko_queue_static_config.non_pknd.pko_queues_per_port_srio[0] = CVMX_PKO_QUEUES_PER_PORT_SRIO0;
	__cvmx_pko_queue_static_config.non_pknd.pko_queues_per_port_srio[1] = CVMX_PKO_QUEUES_PER_PORT_SRIO1;
	__cvmx_pko_queue_static_config.non_pknd.pko_queues_per_port_srio[2] = CVMX_PKO_QUEUES_PER_PORT_SRIO2;
	__cvmx_pko_queue_static_config.non_pknd.pko_queues_per_port_srio[3] = CVMX_PKO_QUEUES_PER_PORT_SRIO3;

	return 0;
}

int cvmx_pko_queue_static_config_pknd(void)
{
	__cvmx_pko_queue_static_config.pknd.pko_queues_per_port_interface[0] = CVMX_PKO_QUEUES_PER_PORT_INTERFACE0 ;
	__cvmx_pko_queue_static_config.pknd.pko_queues_per_port_interface[1] = CVMX_PKO_QUEUES_PER_PORT_INTERFACE1 ;
	__cvmx_pko_queue_static_config.pknd.pko_queues_per_port_interface[2] = CVMX_PKO_QUEUES_PER_PORT_INTERFACE2 ;
	__cvmx_pko_queue_static_config.pknd.pko_queues_per_port_interface[3] = CVMX_PKO_QUEUES_PER_PORT_INTERFACE3 ;
	__cvmx_pko_queue_static_config.pknd.pko_queues_per_port_interface[4] = CVMX_PKO_QUEUES_PER_PORT_INTERFACE4 ;
	__cvmx_pko_queue_static_config.pknd.pko_queues_per_port_loop         = CVMX_PKO_QUEUES_PER_PORT_LOOP;
	__cvmx_pko_queue_static_config.pknd.pko_queues_per_port_pci          = CVMX_PKO_QUEUES_PER_PORT_PCI;

	return 0;

}

int cvmx_pko_queue_static_config(void)
{
	if (octeon_has_feature(OCTEON_FEATURE_PKND)) {
		return cvmx_pko_queue_static_config_pknd();
	} else {
		return cvmx_pko_queue_static_config_non_pknd();
	}
}



int cvmx_ipd_config_init_from_cvmx_config(void)
{
	cvmx_ipd_config_t ipd_cfg;

	ipd_cfg.first_mbuf_skip     = CVMX_HELPER_FIRST_MBUFF_SKIP;
	ipd_cfg.not_first_mbuf_skip = CVMX_HELPER_NOT_FIRST_MBUFF_SKIP;
	ipd_cfg.ipd_enable = CVMX_HELPER_ENABLE_IPD;
#ifdef CVMX_HELPER_IPD_DRAM_MODE
	ipd_cfg.cache_mode = CVMX_HELPER_IPD_DRAM_MODE;
#else
	ipd_cfg.cache_mode = CVMX_IPD_OPC_MODE_STT;
#endif
#ifdef CVMX_ENABLE_LEN_M8_FIX
        ipd_cfg.enable_len_M8_fix = CVMX_ENABLE_LEN_M8_FIX;
#else
        ipd_cfg.enable_len_M8_fix = 0;
#endif
	ipd_cfg.port_config.tag_fields.ipv6_src_ip = CVMX_HELPER_INPUT_TAG_IPV6_SRC_IP;
	ipd_cfg.port_config.tag_fields.ipv6_dst_ip = CVMX_HELPER_INPUT_TAG_IPV6_DST_IP;
	ipd_cfg.port_config.tag_fields.ipv6_src_port = CVMX_HELPER_INPUT_TAG_IPV6_SRC_PORT;
	ipd_cfg.port_config.tag_fields.ipv6_dst_port = CVMX_HELPER_INPUT_TAG_IPV6_DST_PORT;
	ipd_cfg.port_config.tag_fields.ipv6_next_header = CVMX_HELPER_INPUT_TAG_IPV6_NEXT_HEADER;
	ipd_cfg.port_config.tag_fields.ipv4_src_ip = CVMX_HELPER_INPUT_TAG_IPV4_SRC_IP;
	ipd_cfg.port_config.tag_fields.ipv4_dst_ip = CVMX_HELPER_INPUT_TAG_IPV4_DST_IP;
	ipd_cfg.port_config.tag_fields.ipv4_src_port = CVMX_HELPER_INPUT_TAG_IPV4_SRC_PORT;
	ipd_cfg.port_config.tag_fields.ipv4_dst_port = CVMX_HELPER_INPUT_TAG_IPV4_DST_PORT;
	ipd_cfg.port_config.tag_fields.ipv4_protocol = CVMX_HELPER_INPUT_TAG_IPV4_PROTOCOL;
	ipd_cfg.port_config.tag_fields.input_port = CVMX_HELPER_INPUT_TAG_INPUT_PORT;
	ipd_cfg.port_config.parse_mode = CVMX_HELPER_INPUT_PORT_SKIP_MODE;
	ipd_cfg.port_config.tag_mode = CVMX_PIP_TAG_MODE_TUPLE;
	ipd_cfg.port_config.tag_type = CVMX_HELPER_INPUT_TAG_TYPE;
#ifndef CVMX_FPA_PACKET_POOL
#define CVMX_FPA_PACKET_POOL -1
#define CVMX_FPA_PACKET_POOL_SIZE 0
#endif
	ipd_cfg.packet_pool.pool_num = CVMX_FPA_PACKET_POOL;
	ipd_cfg.packet_pool.buffer_size = CVMX_FPA_PACKET_POOL_SIZE;
	ipd_cfg.packet_pool.buffer_count = 0;

#ifndef CVMX_FPA_WQE_POOL
#define CVMX_FPA_WQE_POOL -1
#define CVMX_FPA_WQE_POOL_SIZE 0
#endif
	ipd_cfg.wqe_pool.pool_num = CVMX_FPA_WQE_POOL;
	ipd_cfg.wqe_pool.buffer_size = CVMX_FPA_WQE_POOL_SIZE;
	ipd_cfg.wqe_pool.buffer_count = 0;
	cvmx_ipd_set_config(ipd_cfg);
	return 0;
}

int cvmx_rgmii_config_init_from_cvmx_config(void)
{
	uint64_t disable_backpressure = 0;

#ifdef CVMX_HELPER_DISABLE_RGMII_BACKPRESSURE
	disable_backpressure = CVMX_HELPER_DISABLE_RGMII_BACKPRESSURE;
#endif
	cvmx_rgmii_set_back_pressure(disable_backpressure);
	return 0;

}
#endif

void cvmx_outputbuffer_config_init_from_cvmx_config(void)
{
#if !defined(CVMX_BUILD_FOR_UBOOT)
	dma_config.command_queue_pool.pool_num = CVMX_FPA_OUTPUT_BUFFER_POOL;
	dma_config.command_queue_pool.buffer_size = CVMX_FPA_OUTPUT_BUFFER_POOL_SIZE;
	dma_config.command_queue_pool.buffer_count = 0;
	raid_config.command_queue_pool.pool_num = CVMX_FPA_OUTPUT_BUFFER_POOL;
	raid_config.command_queue_pool.buffer_size = CVMX_FPA_OUTPUT_BUFFER_POOL_SIZE;
	raid_config.command_queue_pool.buffer_count = 0;
#endif
}

#ifdef CVMX_ENABLE_TIMER_FUNCTIONS
void cvmx_timer_config_init_from_cvmx_config(void)
{
#ifndef CVMX_FPA_TIMER_POOL
#define CVMX_FPA_TIMER_POOL -1
#define CVMX_FPA_TIMER_POOL_SIZE 0
#endif
	timer_config.timer_pool.pool_num = CVMX_FPA_TIMER_POOL;
	timer_config.timer_pool.buffer_size = CVMX_FPA_TIMER_POOL_SIZE;
	timer_config.timer_pool.buffer_count = 0;

}
#endif

#ifdef CVMX_ENABLE_DFA_FUNCTIONS
void cvmx_dfa_config_init_from_cvmx_config(void)
{
#ifndef CVMX_FPA_DFA_POOL
#define CVMX_FPA_DFA_POOL -1
#define CVMX_FPA_DFA_POOL_SIZE 0
#endif
	dfa_config.dfa_pool.pool_num = CVMX_FPA_DFA_POOL;
	dfa_config.dfa_pool.buffer_size = CVMX_FPA_DFA_POOL_SIZE;
	dfa_config.dfa_pool.buffer_count = 0;
}
#endif

#ifdef CVMX_ENABLE_ZIP_FUNCTIONS
void cvmx_zip_config_init_from_cvmx_config(void)
{
#ifndef CVMX_FPA_ZIP_POOL
#define CVMX_FPA_ZIP_POOL -1
#define CVMX_FPA_ZIP_POOL_SIZE 0
#endif
	zip_config.zip_pool.pool_num = CVMX_FPA_ZIP_POOL;
	zip_config.zip_pool.buffer_size = CVMX_FPA_ZIP_POOL_SIZE;
	zip_config.zip_pool.buffer_count = 0;

}
#endif

#ifdef CVMX_ENABLE_BCH_FUNCTIONS
void cvmx_bch_config_init_from_cvmx_config(void)
{
#ifndef CVMX_FPA_BCH_POOL
#define CVMX_FPA_BCH_POOL -1
#define CVMX_FPA_BCH_POOL_SIZE 0
#endif
	bch_config.command_queue_pool.pool_num = CVMX_FPA_BCH_POOL;
	bch_config.command_queue_pool.buffer_size = CVMX_FPA_BCH_POOL_SIZE;
	bch_config.command_queue_pool.buffer_count = 0;
}
#endif

int cvmx_user_static_config(void)
{

#ifdef CVMX_ENABLE_HELPER_FUNCTIONS
	if (cvmx_pko_queue_static_config() != 0) {
		cvmx_dprintf("ERROR: cvmx_pko_queue_static_config() failed\n");
	}

	cvmx_ipd_config_init_from_cvmx_config();
	cvmx_rgmii_config_init_from_cvmx_config();

	cvmx_enable_helper_flag = 1;
	cvmx_pko_set_cmd_que_pool_config(CVMX_FPA_OUTPUT_BUFFER_POOL, CVMX_FPA_OUTPUT_BUFFER_POOL_SIZE, 0);
	//cvmx_pko_queue_show();
#endif

#if !defined (CVMX_BUILD_FOR_LINUX_KERNEL) && !defined(CVMX_BUILD_FOR_UBOOT)
        cvmx_llm_num_ports = CVMX_LLM_NUM_PORTS;
#endif

#ifdef CVMX_HELPER_SPI_TIMEOUT
        cvmx_spi_config_set_timeout(CVMX_HELPER_SPI_TIMEOUT);
#endif

#ifdef CVMX_HELPER_ILK_LA_MODE_INTERFACE0
        cvmx_ilk_LA_mode[0].ilk_LA_mode = CVMX_HELPER_ILK_LA_MODE_INTERFACE0;
#endif

#ifdef CVMX_HELPER_ILK_LA_MODE_INTERFACE1
        cvmx_ilk_LA_mode[1].ilk_LA_mode = CVMX_HELPER_ILK_LA_MODE_INTERFACE1;
#endif

#ifdef CVMX_HELPER_ILK_LA_MODE_CAL_ENABLE_INTERFACE0
	cvmx_ilk_LA_mode[0].ilk_LA_mode_cal_ena = CVMX_HELPER_ILK_LA_MODE_CAL_ENABLE_INTERFACE0;
#endif

#ifdef CVMX_HELPER_ILK_LA_MODE_CAL_ENABLE_INTERFACE1
	cvmx_ilk_LA_mode[1].ilk_LA_mode_cal_ena = CVMX_HELPER_ILK_LA_MODE_CAL_ENABLE_INTERFACE1;
#endif

#ifdef CVMX_HELPER_ILK_MAX_CHAN_INTERFACE0
	cvmx_ilk_chans[0] = CVMX_HELPER_ILK_MAX_CHAN_INTERFACE0;
#else
 	cvmx_ilk_chans[0] = 8;
#endif

#ifdef CVMX_HELPER_ILK_MAX_CHAN_INTERFACE1
	cvmx_ilk_chans[1] = CVMX_HELPER_ILK_MAX_CHAN_INTERFACE1;
#else
 	cvmx_ilk_chans[1] = 8;
#endif

/* Max number of channels configured for ILK0 */
#ifdef CVMX_HELPER_ILK_INTF_MASK_INTERFACE0
	cvmx_ilk_lane_mask[0] =  CVMX_HELPER_ILK_INTF_MASK_INTERFACE0;
#else
	cvmx_ilk_lane_mask[0] =  0xf;
#endif

/* Max number of channels configured for ILK1 */
#ifdef CVMX_HELPER_ILK_INTF_MASK_INTERFACE1
	cvmx_ilk_lane_mask[1] =  CVMX_HELPER_ILK_INTF_MASK_INTERFACE1 << 4;
#else
	cvmx_ilk_lane_mask[1] =  0xf << 4;
#endif

#ifdef CVMX_ENABLE_ZIP_FUNCTIONS
	cvmx_zip_config_init_from_cvmx_config();
#endif

#ifdef CVMX_ENABLE_DFA_FUNCTIONS
	cvmx_dfa_config_init_from_cvmx_config();
#endif

#ifdef  CVMX_ENABLE_TIMER_FUNCTIONS
	cvmx_timer_config_init_from_cvmx_config();
#endif

#ifdef CVMX_ENABLE_BCH_FUNCTIONS
	cvmx_bch_config_init_from_cvmx_config();
#endif

	cvmx_outputbuffer_config_init_from_cvmx_config();
	return 0;
}
EXPORT_SYMBOL(cvmx_user_static_config);

#else

int cvmx_user_static_config(void)
{
	// The empty stub is to keep cvmx_user_app_init() happy.
	return 0;

}
#endif

