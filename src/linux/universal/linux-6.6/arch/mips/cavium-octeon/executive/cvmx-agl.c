/***********************license start***************
 * Copyright (c) 2013  Cavium Inc. (support@cavium.com). All rights
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
 * Functions for RGMII (MGMT) initialization, configuration,
 * and monitoring.
 *
 */
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <asm/octeon/octeon.h>
#include <asm/octeon/cvmx-clock.h>
#include <asm/octeon/cvmx-helper.h>
#include <asm/octeon/cvmx-helper-board.h>
#include <asm/octeon/cvmx-helper-cfg.h>
#include <asm/octeon/cvmx-agl-defs.h>
#include <asm/octeon/cvmx-agl.h>
#else
#include "cvmx.h"
#include "cvmx-agl.h"
#include "cvmx-helper-board.h"
#include "cvmx-agl-defs.h"
#include "cvmx-helper-cfg.h"
#endif
/*
 * @param port port to enable
 *
 * @return Zero on success, negative on failure
 */
int cvmx_agl_enable(int port)
{
	cvmx_agl_gmx_inf_mode_t agl_gmx_inf_mode;
	cvmx_agl_gmx_rxx_frm_ctl_t rxx_frm_ctl;

	rxx_frm_ctl.u64 = 0;
	rxx_frm_ctl.s.pre_align = 1;
	rxx_frm_ctl.s.pad_len = 1;	/* When set, disables the length check for non-min sized pkts with padding in the client data */
	rxx_frm_ctl.s.vlan_len = 1;	/* When set, disables the length check for VLAN pkts */
	rxx_frm_ctl.s.pre_free = 1;	/* When set, PREAMBLE checking is  less strict */
	rxx_frm_ctl.s.ctl_smac = 0;	/* Control Pause Frames can match station SMAC */
	rxx_frm_ctl.s.ctl_mcst = 1;	/* Control Pause Frames can match globally assign Multicast address */
	rxx_frm_ctl.s.ctl_bck = 1;	/* Forward pause information to TX block */
	rxx_frm_ctl.s.ctl_drp = 1;	/* Drop Control Pause Frames */
	rxx_frm_ctl.s.pre_strp = 1;	/* Strip off the preamble */
	rxx_frm_ctl.s.pre_chk = 1;	/* This port is configured to send PREAMBLE+SFD to begin every frame.  GMX checks that the PREAMBLE is sent correctly */
	cvmx_write_csr(CVMX_AGL_GMX_RXX_FRM_CTL(port), rxx_frm_ctl.u64);

	/* Enable the AGL block */
	if (OCTEON_IS_MODEL(OCTEON_CN5XXX)) {
		agl_gmx_inf_mode.u64 = 0;
		agl_gmx_inf_mode.s.en = 1;
		cvmx_write_csr(CVMX_AGL_GMX_INF_MODE, agl_gmx_inf_mode.u64);
	}
	return 0;
}

cvmx_helper_link_info_t cvmx_agl_link_get(int port)
{
	cvmx_helper_link_info_t result;
	int interface, port_index;

	/* For simulator also set the link up */
	if (cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_SIM) {
		result.u64 = 0;
		result.s.full_duplex = 1;
		result.s.link_up = 1;
		result.s.speed = 100;
		return result;
	}

	/* Fake IPD port is used on some older models. */
	if (port < 0)
		return __cvmx_helper_board_link_get(port);

	/* Simulator does not have PHY, use some defaults. */
	interface = cvmx_helper_get_interface_num(port);
	port_index = cvmx_helper_get_interface_index_num(port);
	if (cvmx_helper_get_port_force_link_up(interface, port_index)) {
		result.u64 = 0;
		result.s.full_duplex = 1;
		result.s.link_up = 1;
		result.s.speed = 1000;
		return result;
	}

	return __cvmx_helper_board_link_get(port);
}

/*
 * Set MII/RGMII link based on mode.
 *
 * @param port   interface port to set the link.
 * @param link_info  Link status
 *
 * @return       0 on success and 1 on failure
 */
int cvmx_agl_link_set(int port, cvmx_helper_link_info_t link_info)
{
	cvmx_agl_gmx_prtx_cfg_t agl_gmx_prtx;

	/* Disable GMX before we make any changes. */
	agl_gmx_prtx.u64 = cvmx_read_csr(CVMX_AGL_GMX_PRTX_CFG(port));
	agl_gmx_prtx.s.en = 0;
	agl_gmx_prtx.s.tx_en = 0;
	agl_gmx_prtx.s.rx_en = 0;
	cvmx_write_csr(CVMX_AGL_GMX_PRTX_CFG(port), agl_gmx_prtx.u64);

	if (OCTEON_IS_MODEL(OCTEON_CN6XXX) || OCTEON_IS_OCTEON3()) {
		uint64_t one_second = cvmx_clock_get_rate(CVMX_CLOCK_CORE);
		/* Wait for GMX to be idle */
		if (CVMX_WAIT_FOR_FIELD64(CVMX_AGL_GMX_PRTX_CFG(port),
					  cvmx_agl_gmx_prtx_cfg_t, rx_idle,
					  ==, 1, one_second)
		    || CVMX_WAIT_FOR_FIELD64(CVMX_AGL_GMX_PRTX_CFG(port),
					     cvmx_agl_gmx_prtx_cfg_t, tx_idle,
					     ==, 1, one_second)) {
			cvmx_dprintf("AGL%d: Timeout waiting for GMX to be idle\n", port);
			return -1;
		}
	}

	agl_gmx_prtx.u64 = cvmx_read_csr(CVMX_AGL_GMX_PRTX_CFG(port));

	/* Set duplex mode */
	if (!link_info.s.link_up)
		agl_gmx_prtx.s.duplex = 1;	/* Force full duplex on down links */
	else
		agl_gmx_prtx.s.duplex = link_info.s.full_duplex;

	switch (link_info.s.speed) {
	case 10:
		agl_gmx_prtx.s.speed = 0;
		agl_gmx_prtx.s.slottime = 0;
		if (OCTEON_IS_MODEL(OCTEON_CN6XXX) || OCTEON_IS_OCTEON3()) {
			agl_gmx_prtx.s.speed_msb = 1;
			agl_gmx_prtx.s.burst = 1;
		}
		break;

	case 100:
		agl_gmx_prtx.s.speed = 0;
		agl_gmx_prtx.s.slottime = 0;
		if (OCTEON_IS_MODEL(OCTEON_CN6XXX) || OCTEON_IS_OCTEON3()) {
			agl_gmx_prtx.s.speed_msb = 0;
			agl_gmx_prtx.s.burst = 1;
		}
		break;

	case 1000:
		/* 1000 MBits is only supported on 6XXX chips */
		if (OCTEON_IS_MODEL(OCTEON_CN6XXX) || OCTEON_IS_OCTEON3()) {
			agl_gmx_prtx.s.speed_msb = 0;
			agl_gmx_prtx.s.speed = 1;
			agl_gmx_prtx.s.slottime = 1;	/* Only matters for half-duplex */
			agl_gmx_prtx.s.burst = agl_gmx_prtx.s.duplex;
		}
		break;

		/* No link */
	case 0:
	default:
		break;
	}

	/* Write the new GMX setting with the port still disabled. */
	cvmx_write_csr(CVMX_AGL_GMX_PRTX_CFG(port), agl_gmx_prtx.u64);

	/* Read GMX CFG again to make sure the config is completed. */
	agl_gmx_prtx.u64 = cvmx_read_csr(CVMX_AGL_GMX_PRTX_CFG(port));

	if (OCTEON_IS_MODEL(OCTEON_CN6XXX) || OCTEON_IS_OCTEON3()) {
		cvmx_agl_gmx_txx_clk_t agl_clk;
		cvmx_agl_prtx_ctl_t prt_ctl;
		prt_ctl.u64 = cvmx_read_csr(CVMX_AGL_PRTX_CTL(port));
		agl_clk.u64 = cvmx_read_csr(CVMX_AGL_GMX_TXX_CLK(port));
		/* MII (both speeds) and RGMII 1000 setting */
		agl_clk.s.clk_cnt = 1;
		/* Check other speeds for RGMII mode */
		if ((prt_ctl.s.mode == 0) || OCTEON_IS_OCTEON3()) {
			if (link_info.s.speed == 10)
				agl_clk.s.clk_cnt = 50;
			else if (link_info.s.speed == 100)
				agl_clk.s.clk_cnt = 5;
		}
		cvmx_write_csr(CVMX_AGL_GMX_TXX_CLK(port), agl_clk.u64);
	}

	/* Enable transmit and receive ports */
	agl_gmx_prtx.s.tx_en = 1;
	agl_gmx_prtx.s.rx_en = 1;
	cvmx_write_csr(CVMX_AGL_GMX_PRTX_CFG(port), agl_gmx_prtx.u64);

	/* Enable the link. */
	agl_gmx_prtx.s.en = 1;
	cvmx_write_csr(CVMX_AGL_GMX_PRTX_CFG(port), agl_gmx_prtx.u64);

	if (OCTEON_IS_OCTEON3()) {
		union cvmx_agl_prtx_ctl agl_prtx_ctl;
		/* Enable the interface, set clkrst */
		agl_prtx_ctl.u64 = cvmx_read_csr(CVMX_AGL_PRTX_CTL(port));
		agl_prtx_ctl.s.clkrst = 1;
		cvmx_write_csr(CVMX_AGL_PRTX_CTL(port), agl_prtx_ctl.u64);
		cvmx_read_csr(CVMX_AGL_PRTX_CTL(port));
		agl_prtx_ctl.s.enable = 1;
		cvmx_write_csr(CVMX_AGL_PRTX_CTL(port), agl_prtx_ctl.u64);
		/* Read the value back to force the previous write */
		cvmx_read_csr(CVMX_AGL_PRTX_CTL(port));
	}

	return 0;
}
