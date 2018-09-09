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
 * Helper utilities for qlm.
 *
 * <hr>$Revision$<hr>
 */
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-helper.h>
#include <asm/octeon/cvmx-qlm.h>
#include <asm/octeon/cvmx-gserx-defs.h>
#else
#include "cvmx.h"
#include "cvmx-helper.h"
#include "cvmx-qlm.h"
#endif


/**
 * @INTERNAL
 * Configure the gser pll registers.
 *
 * @param interface Interface to bring up
 *
 * @param mode      Mode to configure the gser as
 *
 * @param qlm       QLM attached to this interface
 *
 * @parma num_ports Number of ports on this interface
 *
 * @return Zero on success, negative on failure
 */
static int gser_pll_init(int				interface,
			 cvmx_helper_interface_mode_t	mode,
			 int				qlm,
			 int				num_ports)
{
	cvmx_gserx_pll_px_mode_0_t	gser_pll_p_mode_0;
	cvmx_gserx_pll_px_mode_1_t	gser_pll_p_mode_1;
	cvmx_gserx_lanex_px_mode_0_t	gser_lane_p_mode_0;
	int				lane_mode;
	int				i;

	/* Figure out the lane mode */
	switch (mode) {
	case CVMX_HELPER_INTERFACE_MODE_SGMII:
		lane_mode = 0x6;
		break;

	case CVMX_HELPER_INTERFACE_MODE_XAUI:
		lane_mode = 0x4;
		break;

	default:
		lane_mode = 0;
		break;
	}

	/* Configure pll_p_mode_0 */
	gser_pll_p_mode_0.u64 = 0;
	switch (mode) {
	case CVMX_HELPER_INTERFACE_MODE_SGMII:
		gser_pll_p_mode_0.s.pll_icp = 1;
		gser_pll_p_mode_0.s.pll_rloop = 3;
		gser_pll_p_mode_0.s.pll_pcs_div = 0x28;
		break;

	case CVMX_HELPER_INTERFACE_MODE_XAUI:
		gser_pll_p_mode_0.s.pll_icp = 1;
		gser_pll_p_mode_0.s.pll_rloop = 3;
		gser_pll_p_mode_0.s.pll_pcs_div = 0x24;
		break;

	default:
		break;
	}
	cvmx_write_csr(CVMX_GSERX_PLL_PX_MODE_0(qlm, lane_mode),
		       gser_pll_p_mode_0.u64);

	/* Configure pll_p_mode_1 */
	gser_pll_p_mode_1.u64 = 0;
	switch (mode) {
	case CVMX_HELPER_INTERFACE_MODE_SGMII:
		gser_pll_p_mode_1.s.pll_div = 16;
		gser_pll_p_mode_1.s.pll_opr = 0;
		gser_pll_p_mode_1.s.pll_pcie3en = 0;
		gser_pll_p_mode_1.s.pll_cpadj = 3;
		gser_pll_p_mode_1.s.pll_16p5en = 1;
		break;

	case CVMX_HELPER_INTERFACE_MODE_XAUI:
		gser_pll_p_mode_1.s.pll_div = 20;
		gser_pll_p_mode_1.s.pll_opr = 0;
		gser_pll_p_mode_1.s.pll_pcie3en = 0;
		gser_pll_p_mode_1.s.pll_cpadj = 2;
		gser_pll_p_mode_1.s.pll_16p5en = 1;
		break;

	default:
		break;
	}
	cvmx_write_csr(CVMX_GSERX_PLL_PX_MODE_1(qlm, lane_mode),
		       gser_pll_p_mode_1.u64);

	/* Configure lane_p_mode */
	for (i = 0; i < num_ports; i++) {
		gser_lane_p_mode_0.u64 = 0;
		gser_lane_p_mode_0.s.srate = 0;
		gser_lane_p_mode_0.s.rx_mode = 3;
		gser_lane_p_mode_0.s.tx_mode = 3;

		switch (mode) {
		case CVMX_HELPER_INTERFACE_MODE_SGMII:
			gser_lane_p_mode_0.s.ctle = 0;
			gser_lane_p_mode_0.s.pcie = 0;
			gser_lane_p_mode_0.s.tx_ldiv = 2;
			gser_lane_p_mode_0.s.rx_ldiv = 2;
			break;

		case CVMX_HELPER_INTERFACE_MODE_XAUI:
			gser_lane_p_mode_0.s.ctle = 0;
			gser_lane_p_mode_0.s.pcie = 0;
			gser_lane_p_mode_0.s.tx_ldiv = 2;
			gser_lane_p_mode_0.s.rx_ldiv = 2;
			break;

		default:
			break;
		}
		cvmx_write_csr(CVMX_GSERX_LANEX_PX_MODE_0(qlm, i, lane_mode),
			       gser_lane_p_mode_0.u64);
	}

	return 0;
}

/**
 * @INTERNAL
 * Configure the gser.
 *
 * @param interface Interface to bring up
 *
 * @param mode      Mode to configure the gser as
 *
 * @return Zero on success, negative on failure
 */
int gser_init(int				interface,
	      cvmx_helper_interface_mode_t	mode)
{
	cvmx_gserx_phy_ctl_t		gser_phy_ctl;
	cvmx_gserx_cfg_t		gser_cfg;
	cvmx_gserx_rx_coast_t		gser_rx_coast;
	cvmx_gserx_rx_eie_deten_t	gser_rx_eie_deten;
	cvmx_gserx_lane_mode_t		gser_lane_mode;
	cvmx_gserx_qlm_stat_t		gser_qlm_stat;
	cvmx_gserx_pll_stat_t		gser_pll_stat;
	cvmx_gserx_rx_eie_detsts_t	gser_rx_eie_detsts;
	int				lane_mode;
	int				qlm;
	int				num_ports;

	qlm = cvmx_qlm_interface(interface);
	num_ports = cvmx_helper_ports_on_interface(interface);

	/* Figure out the lane mode */
	switch (mode) {
	case CVMX_HELPER_INTERFACE_MODE_SGMII:
		lane_mode = 0x6;
		break;

	case CVMX_HELPER_INTERFACE_MODE_XAUI:
		lane_mode = 0x4;
		break;

	default:
		lane_mode = 0;
		break;
	}

	/* Power up phy, but keek it in reset */
	gser_phy_ctl.u64 = 0;
	gser_phy_ctl.s.phy_pd = 0;
	gser_phy_ctl.s.phy_reset = 1;
	cvmx_write_csr(CVMX_GSERX_PHY_CTL(qlm), gser_phy_ctl.u64);

	/* Set gser for the interface mode */
	gser_cfg.u64 = 0;
	gser_cfg.s.ila = mode == CVMX_HELPER_INTERFACE_MODE_ILK ? 1 : 0;
	gser_cfg.s.bgx = mode == CVMX_HELPER_INTERFACE_MODE_ILK ? 0 : 1;
	gser_cfg.s.bgx_quad = mode == CVMX_HELPER_INTERFACE_MODE_XAUI ? 1 : 0;
	gser_cfg.s.bgx_dual = 0;
	gser_cfg.s.pcie = 0;
	cvmx_write_csr(CVMX_GSERX_CFG(qlm), gser_cfg.u64);

	/* Enable the port lanes */
	gser_rx_coast.u64 = cvmx_read_csr(CVMX_GSERX_RX_COAST(qlm));
	gser_rx_coast.s.coast |= ((1 << num_ports) - 1);
	cvmx_write_csr(CVMX_GSERX_RX_COAST(qlm), gser_rx_coast.u64);

	gser_rx_eie_deten.u64 = cvmx_read_csr(CVMX_GSERX_RX_EIE_DETEN(qlm));
	gser_rx_eie_deten.s.eiede |= ((1 << num_ports) - 1);
	cvmx_write_csr(CVMX_GSERX_RX_EIE_DETEN(qlm), gser_rx_eie_deten.u64);

	/* Lane mode */
	gser_lane_mode.u64 = 0;
	gser_lane_mode.s.lmode = lane_mode;
	cvmx_write_csr(CVMX_GSERX_LANE_MODE(qlm), gser_lane_mode.u64);

	/* Bring phy out of reset */
	gser_phy_ctl.u64 = cvmx_read_csr(CVMX_GSERX_PHY_CTL(qlm));
	gser_phy_ctl.s.phy_reset = 0;
	cvmx_write_csr(CVMX_GSERX_PHY_CTL(qlm), gser_phy_ctl.u64);
	gser_phy_ctl.u64 = cvmx_read_csr(CVMX_GSERX_PHY_CTL(qlm));

	/*
	 * Wait 250 ns until the managment interface is ready to accept
	 * read/write commands.
	 */
	cvmx_wait_usec(3);

	/* Configure the gser pll */
	gser_pll_init(interface, mode, qlm, num_ports);

	/* Wait for reset to complete and the PLL to lock */
	do {
		gser_qlm_stat.u64 = cvmx_read_csr(CVMX_GSERX_QLM_STAT(qlm));
		gser_pll_stat.u64 = cvmx_read_csr(CVMX_GSERX_PLL_STAT(qlm));
	} while(!gser_qlm_stat.s.rst_rdy || !gser_pll_stat.s.pll_lock);

	/* Wait for cdrlock */
	do {
		gser_rx_eie_detsts.u64 =
			cvmx_read_csr(CVMX_GSERX_RX_EIE_DETSTS(qlm));
	} while((gser_rx_eie_detsts.s.cdrlock & 0xf) != 0xf);

	return 0;
}
