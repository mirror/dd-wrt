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

/**
 * @file
 *
 * IPD Support.
 *
 * <hr>$Revision: 58943 $<hr>
 */
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <linux/module.h>
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-bootmem.h>
#include <asm/octeon/cvmx-pip-defs.h>
#include <asm/octeon/cvmx-dbg-defs.h>
#include <asm/octeon/cvmx-sso-defs.h>

#include <asm/octeon/cvmx-fpa.h>
#include <asm/octeon/cvmx-wqe.h>
#include <asm/octeon/cvmx-ipd.h>
#include <asm/octeon/cvmx-clock.h>
#include <asm/octeon/cvmx-helper-errata.h>
#include <asm/octeon/cvmx-helper-cfg.h>
#include <asm/octeon/cvmx-pki.h>
#else
#include "cvmx.h"
#include "cvmx-sysinfo.h"
#include "cvmx-bootmem.h"
#include "cvmx-version.h"
#include "cvmx-error.h"

#include "cvmx-fpa.h"
#include "cvmx-wqe.h"
#include "cvmx-ipd.h"
#include "cvmx-pki.h"
#include "cvmx-helper-errata.h"
#include "cvmx-helper-cfg.h"
#endif

CVMX_SHARED cvmx_ipd_config_t cvmx_ipd_cfg = {.first_mbuf_skip = 184,
						.ipd_enable = 1,
						.cache_mode = CVMX_IPD_OPC_MODE_STT,
						.packet_pool = {0, 2048, 0},
						.wqe_pool = {1, 128, 0},
						.port_config = { CVMX_PIP_PORT_CFG_MODE_SKIPL2,
								CVMX_POW_TAG_TYPE_ORDERED,
								CVMX_PIP_TAG_MODE_TUPLE,
								.tag_fields = {0,0,0,0,0,0,0,0,0,0,1 }
						}
					};
EXPORT_SYMBOL(cvmx_ipd_cfg);


int cvmx_ipd_set_config(cvmx_ipd_config_t ipd_config)
{
	cvmx_ipd_cfg = ipd_config;
	return 0;
}

void cvmx_ipd_get_config(cvmx_ipd_config_t *ipd_config)
{
	*ipd_config = cvmx_ipd_cfg;
}

void cvmx_ipd_set_packet_pool_buffer_count(uint64_t buffer_count)
{
	cvmx_ipd_cfg.packet_pool.buffer_count = buffer_count;
}

void cvmx_ipd_set_packet_pool_config(int64_t pool, uint64_t buffer_size,
				        uint64_t buffer_count)
{
	if(OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		int64_t aura = pool;
		cvmx_pki_set_default_pool_config(0, pool, buffer_size, buffer_count);
		cvmx_pki_set_default_aura_config(0, aura, pool, buffer_count);
	}
	else {
		cvmx_ipd_cfg.packet_pool.pool_num = pool;
		cvmx_ipd_cfg.packet_pool.buffer_size = buffer_size;
		cvmx_ipd_cfg.packet_pool.buffer_count = buffer_count;
	}
}
EXPORT_SYMBOL(cvmx_ipd_set_packet_pool_config);

void cvmx_ipd_set_wqe_pool_buffer_count(uint64_t buffer_count)
{
	cvmx_ipd_cfg.wqe_pool.buffer_count = buffer_count;
}

void cvmx_ipd_set_wqe_pool_config(int64_t pool, uint64_t buffer_size,
				       uint64_t buffer_count)
{
	cvmx_ipd_cfg.wqe_pool.pool_num = pool;
	cvmx_ipd_cfg.wqe_pool.buffer_size = buffer_size;
	cvmx_ipd_cfg.wqe_pool.buffer_count = buffer_count;
}
EXPORT_SYMBOL(cvmx_ipd_set_wqe_pool_config);

static void __cvmx_ipd_free_ptr_v1(void)
{
	unsigned wqe_pool = cvmx_fpa_get_wqe_pool();
	int i;
	union cvmx_ipd_ptr_count ptr_count;
	union cvmx_ipd_prc_port_ptr_fifo_ctl prc_port_fifo;
        int packet_pool = (int)cvmx_fpa_get_packet_pool();

	/* Only CN38XXp{1,2} cannot read pointer out of the IPD */
	if (OCTEON_IS_MODEL(OCTEON_CN38XX_PASS2))
		return;

	ptr_count.u64 = cvmx_read_csr(CVMX_IPD_PTR_COUNT);

	/* Handle Work Queue Entry in cn56xx and cn52xx */
	if (octeon_has_feature(OCTEON_FEATURE_NO_WPTR)) {
		union cvmx_ipd_ctl_status ctl_status;
		ctl_status.u64 = cvmx_read_csr(CVMX_IPD_CTL_STATUS);
		if (ctl_status.s.no_wptr)
			wqe_pool = packet_pool;
	}

	/* Free the prefetched WQE */
	if (ptr_count.s.wqev_cnt) {
		union cvmx_ipd_wqe_ptr_valid wqe_ptr_valid;
		wqe_ptr_valid.u64 = cvmx_read_csr(CVMX_IPD_WQE_PTR_VALID);
		cvmx_fpa_free(cvmx_phys_to_ptr((uint64_t)wqe_ptr_valid.s.ptr << 7),
			      wqe_pool, 0);
	}

	/* Free all WQE in the fifo */
	if (ptr_count.s.wqe_pcnt) {
		int i;
		union cvmx_ipd_pwp_ptr_fifo_ctl pwp_fifo;
		pwp_fifo.u64 = cvmx_read_csr(CVMX_IPD_PWP_PTR_FIFO_CTL);
		for (i = 0; i < ptr_count.s.wqe_pcnt; i++) {
			pwp_fifo.s.cena = 0;
			pwp_fifo.s.raddr = pwp_fifo.s.max_cnts + (pwp_fifo.s.wraddr + i) % pwp_fifo.s.max_cnts;
			cvmx_write_csr(CVMX_IPD_PWP_PTR_FIFO_CTL, pwp_fifo.u64);
			pwp_fifo.u64 = cvmx_read_csr(CVMX_IPD_PWP_PTR_FIFO_CTL);
			cvmx_fpa_free(cvmx_phys_to_ptr((uint64_t)pwp_fifo.s.ptr << 7),
				      wqe_pool, 0);
		}
		pwp_fifo.s.cena = 1;
		cvmx_write_csr(CVMX_IPD_PWP_PTR_FIFO_CTL, pwp_fifo.u64);
	}

	/* Free the prefetched packet */
	if (ptr_count.s.pktv_cnt) {
		union cvmx_ipd_pkt_ptr_valid pkt_ptr_valid;
		pkt_ptr_valid.u64 = cvmx_read_csr(CVMX_IPD_PKT_PTR_VALID);
		cvmx_fpa_free(cvmx_phys_to_ptr((uint64_t)pkt_ptr_valid.s.ptr << 7),
			      packet_pool, 0);
	}

	/* Free the per port prefetched packets */
	prc_port_fifo.u64 = cvmx_read_csr(CVMX_IPD_PRC_PORT_PTR_FIFO_CTL);

	for (i = 0; i < prc_port_fifo.s.max_pkt; i++) {
		prc_port_fifo.s.cena = 0;
		prc_port_fifo.s.raddr = i % prc_port_fifo.s.max_pkt;
		cvmx_write_csr(CVMX_IPD_PRC_PORT_PTR_FIFO_CTL,
			       prc_port_fifo.u64);
		prc_port_fifo.u64 = cvmx_read_csr(CVMX_IPD_PRC_PORT_PTR_FIFO_CTL);
		cvmx_fpa_free(cvmx_phys_to_ptr((uint64_t)prc_port_fifo.s.ptr << 7),
			      packet_pool, 0);
	}
	prc_port_fifo.s.cena = 1;
	cvmx_write_csr(CVMX_IPD_PRC_PORT_PTR_FIFO_CTL, prc_port_fifo.u64);

	/* Free all packets in the holding fifo */
	if (ptr_count.s.pfif_cnt) {
		int i;
		union cvmx_ipd_prc_hold_ptr_fifo_ctl prc_hold_fifo;

		prc_hold_fifo.u64 = cvmx_read_csr(CVMX_IPD_PRC_HOLD_PTR_FIFO_CTL);

		for (i = 0; i < ptr_count.s.pfif_cnt; i++) {
			prc_hold_fifo.s.cena = 0;
			prc_hold_fifo.s.raddr = (prc_hold_fifo.s.praddr + i) % prc_hold_fifo.s.max_pkt;
			cvmx_write_csr(CVMX_IPD_PRC_HOLD_PTR_FIFO_CTL,
				       prc_hold_fifo.u64);
			prc_hold_fifo.u64 = cvmx_read_csr(CVMX_IPD_PRC_HOLD_PTR_FIFO_CTL);
			cvmx_fpa_free(cvmx_phys_to_ptr((uint64_t)prc_hold_fifo.s.ptr << 7),
				      packet_pool, 0);
		}
		prc_hold_fifo.s.cena = 1;
		cvmx_write_csr(CVMX_IPD_PRC_HOLD_PTR_FIFO_CTL,
			       prc_hold_fifo.u64);
	}

	/* Free all packets in the fifo */
	if (ptr_count.s.pkt_pcnt) {
		int i;
		union cvmx_ipd_pwp_ptr_fifo_ctl pwp_fifo;
		pwp_fifo.u64 = cvmx_read_csr(CVMX_IPD_PWP_PTR_FIFO_CTL);

		for (i = 0; i < ptr_count.s.pkt_pcnt; i++) {
			pwp_fifo.s.cena = 0;
			pwp_fifo.s.raddr = (pwp_fifo.s.praddr + i) % pwp_fifo.s.max_cnts;
			cvmx_write_csr(CVMX_IPD_PWP_PTR_FIFO_CTL, pwp_fifo.u64);
			pwp_fifo.u64 = cvmx_read_csr(CVMX_IPD_PWP_PTR_FIFO_CTL);
			cvmx_fpa_free(cvmx_phys_to_ptr((uint64_t)pwp_fifo.s.ptr << 7),
				      packet_pool, 0);
		}
		pwp_fifo.s.cena = 1;
		cvmx_write_csr(CVMX_IPD_PWP_PTR_FIFO_CTL, pwp_fifo.u64);
	}
}

static void __cvmx_ipd_free_ptr_v2(void)
{
	int no_wptr = 0;
	int i;
	union cvmx_ipd_port_ptr_fifo_ctl port_ptr_fifo;
	union cvmx_ipd_ptr_count ptr_count;
	int packet_pool = (int)cvmx_fpa_get_packet_pool();
	int wqe_pool = (int)cvmx_fpa_get_wqe_pool();
	ptr_count.u64 = cvmx_read_csr(CVMX_IPD_PTR_COUNT);

	/* Handle Work Queue Entry in cn68xx */
	if (octeon_has_feature(OCTEON_FEATURE_NO_WPTR)) {
		union cvmx_ipd_ctl_status ctl_status;
		ctl_status.u64 = cvmx_read_csr(CVMX_IPD_CTL_STATUS);
		if (ctl_status.s.no_wptr)
			no_wptr = 1;
	}

	/* Free the prefetched WQE */
	if (ptr_count.s.wqev_cnt) {
		union cvmx_ipd_next_wqe_ptr next_wqe_ptr;
		next_wqe_ptr.u64 = cvmx_read_csr(CVMX_IPD_NEXT_WQE_PTR);
		if (no_wptr)
			cvmx_fpa_free(cvmx_phys_to_ptr((uint64_t)next_wqe_ptr.s.ptr << 7),
				      packet_pool, 0);
		else
			cvmx_fpa_free(cvmx_phys_to_ptr((uint64_t)next_wqe_ptr.s.ptr << 7),
				      wqe_pool, 0);
	}

	/* Free all WQE in the fifo */
	if (ptr_count.s.wqe_pcnt) {
		union cvmx_ipd_free_ptr_fifo_ctl free_fifo;
		union cvmx_ipd_free_ptr_value free_ptr_value;
		free_fifo.u64 = cvmx_read_csr(CVMX_IPD_FREE_PTR_FIFO_CTL);
		for (i = 0; i < ptr_count.s.wqe_pcnt; i++) {
			free_fifo.s.cena = 0;
			free_fifo.s.raddr = free_fifo.s.max_cnts + (free_fifo.s.wraddr + i) % free_fifo.s.max_cnts;
			cvmx_write_csr(CVMX_IPD_FREE_PTR_FIFO_CTL,
				       free_fifo.u64);
			free_fifo.u64 = cvmx_read_csr(CVMX_IPD_FREE_PTR_FIFO_CTL);
			free_ptr_value.u64 = cvmx_read_csr(CVMX_IPD_FREE_PTR_VALUE);
			if (no_wptr)
				cvmx_fpa_free(cvmx_phys_to_ptr((uint64_t)free_ptr_value.s.ptr << 7),
					      packet_pool, 0);
			else
				cvmx_fpa_free(cvmx_phys_to_ptr((uint64_t)free_ptr_value.s.ptr << 7),
					      wqe_pool, 0);
		}
		free_fifo.s.cena = 1;
		cvmx_write_csr(CVMX_IPD_FREE_PTR_FIFO_CTL, free_fifo.u64);
	}

	/* Free the prefetched packet */
	if (ptr_count.s.pktv_cnt) {
		union cvmx_ipd_next_pkt_ptr next_pkt_ptr;
		next_pkt_ptr.u64 = cvmx_read_csr(CVMX_IPD_NEXT_PKT_PTR);
		cvmx_fpa_free(cvmx_phys_to_ptr((uint64_t)next_pkt_ptr.s.ptr << 7),
			      packet_pool, 0);
	}

	/* Free the per port prefetched packets */
	port_ptr_fifo.u64 = cvmx_read_csr(CVMX_IPD_PORT_PTR_FIFO_CTL);

	for (i = 0; i < port_ptr_fifo.s.max_pkt; i++) {
		port_ptr_fifo.s.cena = 0;
		port_ptr_fifo.s.raddr = i % port_ptr_fifo.s.max_pkt;
		cvmx_write_csr(CVMX_IPD_PORT_PTR_FIFO_CTL, port_ptr_fifo.u64);
		port_ptr_fifo.u64 = cvmx_read_csr(CVMX_IPD_PORT_PTR_FIFO_CTL);
		cvmx_fpa_free(cvmx_phys_to_ptr((uint64_t)port_ptr_fifo.s.ptr << 7),
			      packet_pool, 0);
	}
	port_ptr_fifo.s.cena = 1;
	cvmx_write_csr(CVMX_IPD_PORT_PTR_FIFO_CTL, port_ptr_fifo.u64);

	/* Free all packets in the holding fifo */
	if (ptr_count.s.pfif_cnt) {
		union cvmx_ipd_hold_ptr_fifo_ctl hold_ptr_fifo;

		hold_ptr_fifo.u64 = cvmx_read_csr(CVMX_IPD_HOLD_PTR_FIFO_CTL);

		for (i = 0; i < ptr_count.s.pfif_cnt; i++) {
			hold_ptr_fifo.s.cena = 0;
			hold_ptr_fifo.s.raddr = (hold_ptr_fifo.s.praddr + i) % hold_ptr_fifo.s.max_pkt;
			cvmx_write_csr(CVMX_IPD_HOLD_PTR_FIFO_CTL,
				       hold_ptr_fifo.u64);
			hold_ptr_fifo.u64 = cvmx_read_csr(CVMX_IPD_HOLD_PTR_FIFO_CTL);
			cvmx_fpa_free(cvmx_phys_to_ptr((uint64_t)hold_ptr_fifo.s.ptr << 7),
				      packet_pool, 0);
		}
		hold_ptr_fifo.s.cena = 1;
		cvmx_write_csr(CVMX_IPD_HOLD_PTR_FIFO_CTL, hold_ptr_fifo.u64);
	}

	/* Free all packets in the fifo */
	if (ptr_count.s.pkt_pcnt) {
		union cvmx_ipd_free_ptr_fifo_ctl free_fifo;
		union cvmx_ipd_free_ptr_value free_ptr_value;
		free_fifo.u64 = cvmx_read_csr(CVMX_IPD_FREE_PTR_FIFO_CTL);

		for (i = 0; i < ptr_count.s.pkt_pcnt; i++) {
			free_fifo.s.cena = 0;
			free_fifo.s.raddr = (free_fifo.s.praddr + i) % free_fifo.s.max_cnts;
			cvmx_write_csr(CVMX_IPD_FREE_PTR_FIFO_CTL,
				       free_fifo.u64);
			free_fifo.u64 = cvmx_read_csr(CVMX_IPD_FREE_PTR_FIFO_CTL);
			free_ptr_value.u64 = cvmx_read_csr(CVMX_IPD_FREE_PTR_VALUE);
			cvmx_fpa_free(cvmx_phys_to_ptr((uint64_t)free_ptr_value.s.ptr << 7),
				      packet_pool, 0);
		}
		free_fifo.s.cena = 1;
		cvmx_write_csr(CVMX_IPD_FREE_PTR_FIFO_CTL, free_fifo.u64);
	}
}

/**
 * @INTERNAL
 * This function is called by cvmx_helper_shutdown() to extract
 * all FPA buffers out of the IPD and PIP. After this function
 * completes, all FPA buffers that were prefetched by IPD and PIP
 * wil be in the apropriate FPA pool. This functions does not reset
 * PIP or IPD as FPA pool zero must be empty before the reset can
 * be performed. WARNING: It is very important that IPD and PIP be
 * reset soon after a call to this function.
 */
void __cvmx_ipd_free_ptr(void)
{
	if (octeon_has_feature(OCTEON_FEATURE_PKND))
		__cvmx_ipd_free_ptr_v2();
	else
		__cvmx_ipd_free_ptr_v1();
}

void cvmx_ipd_config(uint64_t mbuff_size,
		     uint64_t first_mbuff_skip,
		     uint64_t not_first_mbuff_skip,
		     uint64_t first_back, uint64_t second_back,
		     uint64_t wqe_fpa_pool, cvmx_ipd_mode_t cache_mode,
		     uint64_t back_pres_enable_flag)
{
	cvmx_ipd_1st_mbuff_skip_t first_skip;
	cvmx_ipd_mbuff_not_first_skip_t not_first_skip;
	cvmx_ipd_packet_mbuff_size_t size;
	cvmx_ipd_1st_next_ptr_back_t first_back_struct;
	cvmx_ipd_second_next_ptr_back_t second_back_struct;
	cvmx_ipd_wqe_fpa_queue_t wqe_pool;
	cvmx_ipd_ctl_status_t ipd_ctl_reg;

	first_skip.u64 = 0;
	first_skip.s.skip_sz = first_mbuff_skip;
	cvmx_write_csr(CVMX_IPD_1ST_MBUFF_SKIP, first_skip.u64);

	not_first_skip.u64 = 0;
	not_first_skip.s.skip_sz = not_first_mbuff_skip;
	cvmx_write_csr(CVMX_IPD_NOT_1ST_MBUFF_SKIP, not_first_skip.u64);

	size.u64 = 0;
	size.s.mb_size = mbuff_size;
	cvmx_write_csr(CVMX_IPD_PACKET_MBUFF_SIZE, size.u64);

	first_back_struct.u64 = 0;
	first_back_struct.s.back = first_back;
	cvmx_write_csr(CVMX_IPD_1st_NEXT_PTR_BACK, first_back_struct.u64);

	second_back_struct.u64 = 0;
	second_back_struct.s.back = second_back;
	cvmx_write_csr(CVMX_IPD_2nd_NEXT_PTR_BACK, second_back_struct.u64);

	wqe_pool.u64 = 0;
	wqe_pool.s.wqe_pool = wqe_fpa_pool;
	cvmx_write_csr(CVMX_IPD_WQE_FPA_QUEUE, wqe_pool.u64);

	ipd_ctl_reg.u64 = cvmx_read_csr(CVMX_IPD_CTL_STATUS);
	ipd_ctl_reg.s.opc_mode = cache_mode;
	ipd_ctl_reg.s.pbp_en = back_pres_enable_flag;
	cvmx_write_csr(CVMX_IPD_CTL_STATUS, ipd_ctl_reg.u64);

	/* Note: the example RED code that used to be here has been moved to
	   cvmx_helper_setup_red */
}

/**
 * Enable IPD
 */
void cvmx_ipd_enable(void)
{
	cvmx_ipd_ctl_status_t ipd_reg;

	ipd_reg.u64 = cvmx_read_csr(CVMX_IPD_CTL_STATUS);

	/*
	 * busy-waiting for rst_done in o68
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN68XX))
		while (ipd_reg.s.rst_done != 0)
			ipd_reg.u64 = cvmx_read_csr(CVMX_IPD_CTL_STATUS);

	if (ipd_reg.s.ipd_en)
		cvmx_dprintf("Warning: Enabling IPD when IPD already enabled.\n");

	ipd_reg.s.ipd_en = 1;


	if(cvmx_ipd_cfg.enable_len_M8_fix) {
		if (!OCTEON_IS_MODEL(OCTEON_CN38XX_PASS2))
			ipd_reg.s.len_m8 = 1;
	}

	cvmx_write_csr(CVMX_IPD_CTL_STATUS, ipd_reg.u64);
}
EXPORT_SYMBOL(cvmx_ipd_enable);

/**
 * Disable IPD
 */
void cvmx_ipd_disable(void)
{
	cvmx_ipd_ctl_status_t ipd_reg;
	ipd_reg.u64 = cvmx_read_csr(CVMX_IPD_CTL_STATUS);
	ipd_reg.s.ipd_en = 0;
	cvmx_write_csr(CVMX_IPD_CTL_STATUS, ipd_reg.u64);
}
EXPORT_SYMBOL(cvmx_ipd_disable);
