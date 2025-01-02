/***********************license start***************
 * Copyright (c) 2014-2015  Cavium Inc. (support@cavium.com). All rights
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
 * Functions to configure the BGX MAC.
 *
 * <hr>$Id$<hr>
 */
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-helper.h>
#include <asm/octeon/cvmx-clock.h>
#include <asm/octeon/cvmx-qlm.h>
#include <asm/octeon/cvmx-helper-bgx.h>
#include <asm/octeon/cvmx-helper-board.h>
#include <asm/octeon/cvmx-helper-cfg.h>
#include <asm/octeon/cvmx-bgxx-defs.h>
#include <asm/octeon/cvmx-gserx-defs.h>
#include <asm/octeon/cvmx-xcv-defs.h>
#else
#include "cvmx.h"
#include "cvmx-helper.h"
#include "cvmx-helper-bgx.h"
#include "cvmx-helper-board.h"
#include "cvmx-helper-cfg.h"
#include "cvmx-qlm.h"
#endif

/* Enable this define to see BGX error messages */
/*#define DEBUG_BGX */

/* Enable this variable to trace functions called for initializing BGX */
static const int debug = 0;

/**
 * cvmx_helper_bgx_override_autoneg(int xiface, int index) is a function pointer
 * to override enabling/disabling of autonegotiation for SGMII, 10G-KR or 40G-KR4
 * interfaces. This function is called when interface is initialized.
 */
CVMX_SHARED int(*cvmx_helper_bgx_override_autoneg)(int xiface, int index) = NULL;

/*
 * cvmx_helper_bgx_override_fec(int xiface) is a function pointer
 * to override enabling/disabling of FEC for 10G interfaces. This function
 * is called when interface is initialized.
 */
CVMX_SHARED int(*cvmx_helper_bgx_override_fec)(int xiface, int index) = NULL;

/**
 * Delay after enabling an interface based on the mode.  Different modes take
 * different amounts of time.
 */
static void
__cvmx_helper_bgx_interface_enable_delay(cvmx_helper_interface_mode_t mode)
{
	/* Don't delay if we running under the simulator. */
	if (cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_SIM)
		return;

	switch (mode) {
	case CVMX_HELPER_INTERFACE_MODE_10G_KR:
	case CVMX_HELPER_INTERFACE_MODE_40G_KR4:
	case CVMX_HELPER_INTERFACE_MODE_XLAUI:
	case CVMX_HELPER_INTERFACE_MODE_XFI:
		cvmx_wait_usec(250000);
		break;
	case CVMX_HELPER_INTERFACE_MODE_RXAUI:
	case CVMX_HELPER_INTERFACE_MODE_XAUI:
		cvmx_wait_usec(100000);
		break;
	case CVMX_HELPER_INTERFACE_MODE_SGMII:
		cvmx_wait_usec(50000);
		break;
	default:
		cvmx_wait_usec(50000);
		break;
	}
}

/**
 * @INTERNAL
 *
 * Returns number of ports based on interface
 * @param xiface Which xiface
 * @return Number of ports based on xiface
 */
int __cvmx_helper_bgx_enumerate(int xiface)
{
	cvmx_bgxx_cmr_tx_lmacs_t lmacs;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	lmacs.u64 = cvmx_read_csr_node(xi.node, CVMX_BGXX_CMR_TX_LMACS(xi.interface));
	return lmacs.s.lmacs;

}

/**
 * @INTERNAL
 *
 * Returns mode of each BGX LMAC (port).
 * This is different than 'cvmx_helper_interface_get_mode()' which
 * provides mode of an entire interface, but when BGX is in "mixed"
 * mode this function should be called instead to get the protocol
 * for each port (BGX LMAC) individually.
 * Both function return the same enumerated mode.
 *
 * @param xiface is the global interface identifier
 * @param index is the interface port index
 * @returns mode of the individual port
 */
cvmx_helper_interface_mode_t cvmx_helper_bgx_get_mode(int xiface, int index)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	cvmx_bgxx_cmrx_config_t cmr_config;
	cvmx_bgxx_spux_br_pmd_control_t pmd_control;

	cmr_config.u64 = cvmx_read_csr_node(xi.node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface));

	switch (cmr_config.s.lmac_type)
	{
	case 0:
		return CVMX_HELPER_INTERFACE_MODE_SGMII;
		break;
	case 1:
		return CVMX_HELPER_INTERFACE_MODE_XAUI;
		break;
	case 2:
		return CVMX_HELPER_INTERFACE_MODE_RXAUI;
		break;
	case 3:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return cvmx_helper_interface_get_mode(xiface);
		pmd_control.u64 = cvmx_read_csr_node(xi.node,
					CVMX_BGXX_SPUX_BR_PMD_CONTROL(index,
							xi.interface));
		if (pmd_control.s.train_en)
			return CVMX_HELPER_INTERFACE_MODE_10G_KR;
		else
			return CVMX_HELPER_INTERFACE_MODE_XFI;
		break;
	case 4:
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
			return cvmx_helper_interface_get_mode(xiface);
		pmd_control.u64 = cvmx_read_csr_node(xi.node,
					CVMX_BGXX_SPUX_BR_PMD_CONTROL(index,
							xi.interface));
		if (pmd_control.s.train_en)
			return CVMX_HELPER_INTERFACE_MODE_40G_KR4;
		else
			return CVMX_HELPER_INTERFACE_MODE_XLAUI;
		break;
	case 5:
		return CVMX_HELPER_INTERFACE_MODE_RGMII;
		break;
	default:
		return CVMX_HELPER_INTERFACE_MODE_DISABLED;
		break;
	}
}
EXPORT_SYMBOL(cvmx_helper_bgx_get_mode);

/**
 * @INTERNAL
 * Disable the BGX port
 *
 * @param xipd_port IPD port of the BGX interface to disable
 */
void cvmx_helper_bgx_disable(int xipd_port)
{
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(xipd_port);
	int node = xi.node;
	int index = cvmx_helper_get_interface_index_num(xp.port);
	cvmx_bgxx_cmrx_config_t cmr_config;

	cmr_config.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface));
	if (debug)
		cvmx_dprintf("%s: Disabling tx and rx packets on xipd port 0x%x\n",
			     __func__, xipd_port);
	cmr_config.s.data_pkt_tx_en = 0;
	cmr_config.s.data_pkt_rx_en = 0;
	cvmx_write_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface), cmr_config.u64);
}

static int __cvmx_helper_bgx_rgmii_speed(cvmx_helper_link_info_t link_info)
{
	cvmx_xcv_reset_t xcv_reset;
	cvmx_xcv_ctl_t xcv_ctl;
	cvmx_xcv_batch_crd_ret_t crd_ret;
	cvmx_xcv_dll_ctl_t dll_ctl;
	cvmx_xcv_comp_ctl_t comp_ctl;
	int speed;
	int up = link_info.s.link_up;
	int do_credits;

	if (link_info.s.speed == 100)
		speed = 1;
	else if (link_info.s.speed == 10)
		speed = 0;
	else
		speed = 2;

	xcv_reset.u64 = cvmx_read_csr(CVMX_XCV_RESET);
	xcv_ctl.u64 = cvmx_read_csr(CVMX_XCV_CTL);
	do_credits = up && !xcv_reset.s.enable;

	if (xcv_ctl.s.lpbk_int) {
		xcv_reset.s.clkrst = 0;
		cvmx_write_csr(CVMX_XCV_RESET, xcv_reset.u64);
	}

	if (up && (!xcv_reset.s.enable || (xcv_ctl.s.speed != speed))) {
		if (debug)
			cvmx_dprintf("%s: *** Enabling XCV block\n", __func__);
		/* Enable the XCV block */
		xcv_reset.u64 = cvmx_read_csr(CVMX_XCV_RESET);
		xcv_reset.s.enable = 1;
		cvmx_write_csr(CVMX_XCV_RESET, xcv_reset.u64);

		/* Set operating mode */
		xcv_ctl.u64 = cvmx_read_csr(CVMX_XCV_CTL);
		xcv_ctl.s.speed = speed;
		cvmx_write_csr(CVMX_XCV_CTL, xcv_ctl.u64);

		/* Configure DLL - enable or bypass bypass */
		/* TX no bypass, RX bypass */
		dll_ctl.u64 = cvmx_read_csr(CVMX_XCV_DLL_CTL);
		dll_ctl.s.clkrx_set = 0;
		dll_ctl.s.clkrx_byp = 1;
		dll_ctl.s.clktx_byp = 0;
		cvmx_write_csr(CVMX_XCV_DLL_CTL, dll_ctl.u64);

		/* Enable */
		dll_ctl.u64 = cvmx_read_csr(CVMX_XCV_DLL_CTL);
		dll_ctl.s.refclk_sel = 0;
		cvmx_write_csr(CVMX_XCV_DLL_CTL, dll_ctl.u64);
		xcv_reset.u64 = cvmx_read_csr(CVMX_XCV_RESET);
		xcv_reset.s.dllrst = 0;
		cvmx_write_csr(CVMX_XCV_RESET, xcv_reset.u64);

		/* Delay deems to be need so XCV_DLL_CTL[CLK_SET] works */
		cvmx_wait_usec(10);

		comp_ctl.u64 = cvmx_read_csr(CVMX_XCV_COMP_CTL);
		//comp_ctl.s.drv_pctl = 0;
		//comp_ctl.s.drv_nctl = 0;
		comp_ctl.s.drv_byp = 0;
		cvmx_write_csr(CVMX_XCV_COMP_CTL, comp_ctl.u64);

		/* enable */
		xcv_reset.u64 = cvmx_read_csr(CVMX_XCV_RESET);
		xcv_reset.s.comp = 1;
		cvmx_write_csr(CVMX_XCV_RESET, xcv_reset.u64);

		/* setup the RXC */
		xcv_reset.u64 = cvmx_read_csr(CVMX_XCV_RESET);
		xcv_reset.s.clkrst = !xcv_ctl.s.lpbk_int;
		cvmx_write_csr(CVMX_XCV_RESET, xcv_reset.u64);

		/* datapaths come out of the reset
		 * - the datapath resets will disengage BGX from the RGMII
		 *   interface
		 * - XCV will continue to return TX credits for each tick that
		 *   is sent on the TX data path
		 */
		xcv_reset.u64 = cvmx_read_csr(CVMX_XCV_RESET);
		xcv_reset.s.tx_dat_rst_n = 1;
		xcv_reset.s.rx_dat_rst_n = 1;
		cvmx_write_csr(CVMX_XCV_RESET, xcv_reset.u64);
	} else if (debug) {
		cvmx_dprintf("%s: *** Not enabling XCV\n", __func__);
		cvmx_dprintf("  up: %s, xcv_reset.s.enable: %d, xcv_ctl.s.speed: %d, speed: %d\n",
			     up ? "true" : "false", xcv_reset.s.enable,
			     xcv_ctl.s.speed, speed);
	}

	/* enable the packet flow
	 * - The packet resets will be only disengage on packet boundaries
	 * - XCV will continue to return TX credits for each tick that is
	 *   sent on the TX datapath
	 */
	xcv_reset.u64 = cvmx_read_csr(CVMX_XCV_RESET);
	xcv_reset.s.tx_pkt_rst_n = up;
	xcv_reset.s.rx_pkt_rst_n = up;
	cvmx_write_csr(CVMX_XCV_RESET, xcv_reset.u64);

	/* Full reset when link is down */
	if (!up) {
		if (debug)
			cvmx_dprintf("%s: *** Disabling XCV reset\n", __func__);
		/* wait 2*MTU in time */
		cvmx_wait_usec(10000);
		/* reset the world */
		cvmx_write_csr(CVMX_XCV_RESET, 0);
	}

	/* grant PKO TX credits */
	if (do_credits) {
		crd_ret.u64 = cvmx_read_csr(CVMX_XCV_BATCH_CRD_RET);
		crd_ret.s.crd_ret = 1;
		cvmx_write_csr(CVMX_XCV_BATCH_CRD_RET, crd_ret.u64);
	}

	return 0;
}

static void __cvmx_bgx_common_init_pknd(int xiface, int index)
{
	int num_ports;
	int num_chl = 16;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int node = xi.node;
	int pknd;
	cvmx_bgxx_cmrx_rx_bp_on_t bgx_rx_bp_on;
	cvmx_bgxx_cmrx_rx_id_map_t cmr_rx_id_map;
	cvmx_bgxx_cmr_chan_msk_and_t chan_msk_and;
	cvmx_bgxx_cmr_chan_msk_or_t chan_msk_or;

	if (debug)
		cvmx_dprintf("%s: interface %u:%d/%d\n",
		__func__, xi.node, xi.interface, index);

	num_ports = cvmx_helper_ports_on_interface(xiface);
	/* Modify bp_on mark, depending on number of LMACS on that interface
	and write it for every port */
	bgx_rx_bp_on.u64 = 0;
	bgx_rx_bp_on.s.mark = (CVMX_BGX_RX_FIFO_SIZE / (num_ports * 4 * 16));

	/* Setup pkind */
	pknd = cvmx_helper_get_pknd(xiface, index);
	cmr_rx_id_map.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMRX_RX_ID_MAP(index, xi.interface));
	cmr_rx_id_map.s.pknd = pknd;
	/* Change the default reassembly id (RID), as max 14 RIDs allowed */
	if (OCTEON_IS_MODEL(OCTEON_CN73XX))
		cmr_rx_id_map.s.rid = ((4 * xi.interface) + 2 + index);
	cvmx_write_csr_node(node, CVMX_BGXX_CMRX_RX_ID_MAP(index, xi.interface),
			    cmr_rx_id_map.u64);
	/* Set backpressure channel mask AND/OR registers */
	chan_msk_and.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMR_CHAN_MSK_AND(xi.interface));
	chan_msk_or.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMR_CHAN_MSK_OR(xi.interface));
	chan_msk_and.s.msk_and |= ((1 << num_chl) - 1) << (16 * index);
	chan_msk_or.s.msk_or |= ((1 << num_chl) - 1) << (16 * index);
	cvmx_write_csr_node(node, CVMX_BGXX_CMR_CHAN_MSK_AND(xi.interface), chan_msk_and.u64);
	cvmx_write_csr_node(node, CVMX_BGXX_CMR_CHAN_MSK_OR(xi.interface), chan_msk_or.u64);
	/* set rx back pressure (bp_on) on value */
	cvmx_write_csr_node(node, CVMX_BGXX_CMRX_RX_BP_ON(index, xi.interface), bgx_rx_bp_on.u64);
}

/**
 * @INTERNAL
 * Probe a SGMII interface and determine the number of ports
 * connected to it. The SGMII interface should still be down after
 * this call. This is used by interfaces using the bgx mac.
 *
 * @param xiface Interface to probe
 *
 * @return Number of ports on the interface. Zero to disable.
 */
int __cvmx_helper_bgx_probe(int xiface)
{
	return __cvmx_helper_bgx_enumerate(xiface);
}
EXPORT_SYMBOL(__cvmx_helper_bgx_probe);

/**
 * @INTERNAL
 * Return the size of the BGX TX_FIFO for a given LMAC,
 * or 0 if the requested LMAC is inactive.
 *
 * TBD: Need also to add a "__cvmx_helper_bgx_speed()" function to
 * return the speed of each LMAC.
 */
int __cvmx_helper_bgx_fifo_size(int xiface, unsigned lmac)
{
	cvmx_bgxx_cmr_tx_lmacs_t lmacs;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	unsigned tx_fifo_size = CVMX_BGX_TX_FIFO_SIZE;

	/* FIXME: Add validation for interface# < BGX_count */
	lmacs.u64 = cvmx_read_csr_node(xi.node,
		CVMX_BGXX_CMR_TX_LMACS(xi.interface));

	switch (lmacs.s.lmacs) {
	case 1:
		if (lmac > 0)
			return 0;
		else
			return tx_fifo_size;
	case 2:
		if (lmac > 1)
			return 0;
		else
			return tx_fifo_size >> 1;
	case 3:
		if (lmac > 2)
			return 0;
		else
			return tx_fifo_size >> 2;
	case 4:
		if (lmac > 3)
			return 0;
		else
			return tx_fifo_size >> 2;
	default:
		return 0;
	}
}

/**
 * @INTERNAL
 * Perform initialization required only once for an SGMII port.
 *
 * @param xiface Interface to init
 * @param index     Index of prot on the interface
 *
 * @return Zero on success, negative on failure
 */
static int __cvmx_helper_bgx_sgmii_hardware_init_one_time(int xiface, int index)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int node = xi.node;
	const uint64_t clock_mhz = cvmx_clock_get_rate_node(node, CVMX_CLOCK_SCLK) / 1000000;
	cvmx_bgxx_gmp_pcs_miscx_ctl_t gmp_misc_ctl;
	cvmx_bgxx_gmp_pcs_linkx_timer_t gmp_timer;

	if (!cvmx_helper_is_port_valid(xi.interface, index))
		return 0;

	if (debug)
		cvmx_dprintf("%s: interface %u:%d/%d\n",
		__func__, xi.node, xi.interface, index);

	/*
	 * Write PCS*_LINK*_TIMER_COUNT_REG[COUNT] with the
	 * appropriate value. 1000BASE-X specifies a 10ms
	 * interval. SGMII specifies a 1.6ms interval.
	 */
	gmp_misc_ctl.u64 = cvmx_read_csr_node(node, CVMX_BGXX_GMP_PCS_MISCX_CTL(index, xi.interface));
	/* Adjust the MAC mode if requested by device tree */
	gmp_misc_ctl.s.mac_phy =
		cvmx_helper_get_mac_phy_mode(xiface, index);
	gmp_misc_ctl.s.mode =
		cvmx_helper_get_1000x_mode(xiface, index);
	cvmx_write_csr_node(node, CVMX_BGXX_GMP_PCS_MISCX_CTL(index, xi.interface), gmp_misc_ctl.u64);

	gmp_timer.u64 = cvmx_read_csr_node(node, CVMX_BGXX_GMP_PCS_LINKX_TIMER(index, xi.interface));
	if (gmp_misc_ctl.s.mode)
		/* 1000BASE-X */
		gmp_timer.s.count = (10000ull * clock_mhz) >> 10;
	else
		/* SGMII */
		gmp_timer.s.count = (1600ull * clock_mhz) >> 10;

	cvmx_write_csr_node(node, CVMX_BGXX_GMP_PCS_LINKX_TIMER(index, xi.interface), gmp_timer.u64);

	/*
	 * Write the advertisement register to be used as the
	 * tx_Config_Reg<D15:D0> of the autonegotiation.  In
	 * 1000BASE-X mode, tx_Config_Reg<D15:D0> is PCS*_AN*_ADV_REG.
	 * In SGMII PHY mode, tx_Config_Reg<D15:D0> is
	 * PCS*_SGM*_AN_ADV_REG.  In SGMII MAC mode,
	 * tx_Config_Reg<D15:D0> is the fixed value 0x4001, so this
	 * step can be skipped.
	 */
	if (gmp_misc_ctl.s.mode) {
		/* 1000BASE-X */
		cvmx_bgxx_gmp_pcs_anx_adv_t gmp_an_adv;
		gmp_an_adv.u64 = cvmx_read_csr_node(node, CVMX_BGXX_GMP_PCS_ANX_ADV(index, xi.interface));
		gmp_an_adv.s.rem_flt = 0;
		gmp_an_adv.s.pause = 3;
		gmp_an_adv.s.hfd = 1;
		gmp_an_adv.s.fd = 1;
		cvmx_write_csr_node(node, CVMX_BGXX_GMP_PCS_ANX_ADV(index, xi.interface), gmp_an_adv.u64);
	} else {
		if (gmp_misc_ctl.s.mac_phy) {
			/* PHY Mode */
			cvmx_bgxx_gmp_pcs_sgmx_an_adv_t gmp_sgmx_an_adv;
			gmp_sgmx_an_adv.u64 = cvmx_read_csr_node(node, CVMX_BGXX_GMP_PCS_SGMX_AN_ADV(index, xi.interface));
			gmp_sgmx_an_adv.s.dup = 1;
			gmp_sgmx_an_adv.s.speed = 2;
			cvmx_write_csr_node(node, CVMX_BGXX_GMP_PCS_SGMX_AN_ADV(index, xi.interface),
				       gmp_sgmx_an_adv.u64);
		} else {
			/* MAC Mode - Nothing to do */
		}
	}
	return 0;
}

/**
 * @INTERNAL
 * Bring up the SGMII interface to be ready for packet I/O but
 * leave I/O disabled using the GMX override. This function
 * follows the bringup documented in 10.6.3 of the manual.
 *
 * @param xiface Interface to bringup
 * @param num_ports Number of ports on the interface
 *
 * @return Zero on success, negative on failure
 */
static int __cvmx_helper_bgx_sgmii_hardware_init(int xiface, int num_ports)
{
	int index;
	int do_link_set = 1;

	for (index = 0; index < num_ports; index++) {
		int xipd_port = cvmx_helper_get_ipd_port(xiface, index);
		cvmx_helper_interface_mode_t mode;

		if (!cvmx_helper_is_port_valid(xiface, index))
			continue;

		__cvmx_helper_bgx_port_init(xipd_port, 0);

		mode = cvmx_helper_bgx_get_mode(xiface, index);
		if (mode == CVMX_HELPER_INTERFACE_MODE_RGMII)
			continue;

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
		/*
		 * Linux kernel driver will call ....link_set with the
		 * proper link state. In the simulator there is no
		 * link state polling and hence it is set from
		 * here.
		 */
		if (!(cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_SIM))
			do_link_set = 0;
#endif
		if (do_link_set)
			__cvmx_helper_bgx_sgmii_link_set(xipd_port,
					__cvmx_helper_bgx_sgmii_link_get(xipd_port));
	}

	return 0;
}

/**
 * @INTERNAL
 * Bringup and enable a SGMII interface. After this call packet
 * I/O should be fully functional. This is called with IPD
 * enabled but PKO disabled. This is used by interfaces using
 * the bgx mac.
 *
 * @param xiface Interface to bring up
 *
 * @return Zero on success, negative on failure
 */
int __cvmx_helper_bgx_sgmii_enable(int xiface)
{
	int num_ports;

	num_ports = cvmx_helper_ports_on_interface(xiface);
	__cvmx_helper_bgx_sgmii_hardware_init(xiface, num_ports);

	return 0;
}

/**
 * @INTERNAL
 * Initialize the SERDES link for the first time or after a loss
 * of link.
 *
 * @param xiface Interface to init
 * @param index     Index of prot on the interface
 *
 * @return Zero on success, negative on failure
 */
static int __cvmx_helper_bgx_sgmii_hardware_init_link(int xiface, int index)
{
	cvmx_bgxx_gmp_pcs_mrx_control_t gmp_control;
	cvmx_bgxx_gmp_pcs_miscx_ctl_t gmp_misc_ctl;
	cvmx_bgxx_cmrx_config_t cmr_config;
	int phy_mode, mode_1000x;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int interface = xi.interface;
	int node = xi.node;
	int autoneg = 0;

	if (!cvmx_helper_is_port_valid(xiface, index))
		return 0;

	if (debug)
		cvmx_dprintf("%s: interface %u:%d/%d\n",
		__func__, xi.node, xi.interface, index);

	gmp_control.u64 = cvmx_read_csr_node(node, CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, xi.interface));
	/* Take PCS through a reset sequence */
	if (cvmx_sysinfo_get()->board_type != CVMX_BOARD_TYPE_SIM) {
		gmp_control.s.reset = 1;
		cvmx_write_csr_node(node, CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, xi.interface),
		       					     gmp_control.u64);

		/* Wait until GMP_PCS_MRX_CONTROL[reset] comes out of reset */
		if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, xi.interface),
				cvmx_bgxx_gmp_pcs_mrx_control_t, reset, ==, 0, 10000)) {
			cvmx_dprintf("SGMII%d: Timeout waiting for port %d to finish reset\n", interface, index);
			return -1;
		}
	}

	cmr_config.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface));

	gmp_control.u64 = cvmx_read_csr_node(node, CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, xi.interface));
	if (cvmx_helper_get_port_phy_present(xiface, index)) {
		gmp_control.s.pwr_dn = 0;
	} else {
		gmp_control.s.spdmsb = 1;
		gmp_control.s.spdlsb = 0;
		gmp_control.s.pwr_dn = 0;
	}
	/* Write GMP_PCS_MR*_CONTROL[RST_AN]=1 to ensure a fresh SGMII
	   negotiation starts. */
	autoneg = cvmx_helper_get_port_autonegotiation(xiface, index);
	gmp_control.s.rst_an = 1;
	gmp_control.s.an_en = (cmr_config.s.lmac_type != 5) && autoneg;
	cvmx_write_csr_node(node, CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, xi.interface), gmp_control.u64);

	phy_mode = cvmx_helper_get_mac_phy_mode(xiface, index);
	mode_1000x = cvmx_helper_get_1000x_mode(xiface, index);

	gmp_misc_ctl.u64 = cvmx_read_csr_node(node, CVMX_BGXX_GMP_PCS_MISCX_CTL(index, xi.interface));
	gmp_misc_ctl.s.mac_phy = phy_mode;
	gmp_misc_ctl.s.mode = mode_1000x;
	cvmx_write_csr_node(node, CVMX_BGXX_GMP_PCS_MISCX_CTL(index, xi.interface), gmp_misc_ctl.u64);

	if (phy_mode || !autoneg)
		/* In PHY mode we can't query the link status so we just
		   assume that the link is up */
		return 0;

	/* Wait for GMP_PCS_MRX_CONTROL[an_cpt] to be set, indicating that
	   SGMII autonegotiation is complete. In MAC mode this isn't an
	   ethernet link, but a link between OCTEON and PHY. */

	if ((cvmx_sysinfo_get()->board_type != CVMX_BOARD_TYPE_SIM) &&
	    (cmr_config.s.lmac_type != 5) &&
	     CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_BGXX_GMP_PCS_MRX_STATUS(index, xi.interface),
				   cvmx_bgxx_gmp_pcs_mrx_status_t, an_cpt,
				   ==, 1, 10000)) {
		cvmx_dprintf("SGMII%d: Port %d link timeout\n", interface, index);
		return -1;
	}

	return 0;
}

/**
 * @INTERNAL
 * Configure an SGMII link to the specified speed after the SERDES
 * link is up.
 *
 * @param xiface Interface to init
 * @param index     Index of prot on the interface
 * @param link_info Link state to configure
 *
 * @return Zero on success, negative on failure
 */
static int __cvmx_helper_bgx_sgmii_hardware_init_link_speed(int xiface,
							    int index,
							    cvmx_helper_link_info_t link_info)
{
	cvmx_bgxx_cmrx_config_t cmr_config;
	cvmx_bgxx_gmp_pcs_miscx_ctl_t gmp_miscx_ctl;
	cvmx_bgxx_gmp_gmi_prtx_cfg_t gmp_prtx_cfg;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int node = xi.node;

	if (!cvmx_helper_is_port_valid(xiface, index))
		return 0;

	if (debug)
		cvmx_dprintf("%s: interface %u:%d/%d\n",
		__func__, xi.node, xi.interface, index);

	/* Disable GMX before we make any changes. */
	cmr_config.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface));
	cmr_config.s.data_pkt_tx_en = 0;
	cmr_config.s.data_pkt_rx_en = 0;
	cvmx_write_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface), cmr_config.u64);

	/* Wait for GMX to be idle */
	if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_BGXX_GMP_GMI_PRTX_CFG(index, xi.interface),
				  cvmx_bgxx_gmp_gmi_prtx_cfg_t, rx_idle, ==, 1, 10000) ||
	    CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_BGXX_GMP_GMI_PRTX_CFG(index, xi.interface),
				  cvmx_bgxx_gmp_gmi_prtx_cfg_t, tx_idle, ==, 1, 10000)) {
		cvmx_dprintf("SGMII%d:%d: Timeout waiting for port %d to be idle\n",
			     node, xi.interface, index);
		return -1;
	}

	/* Read GMX CFG again to make sure the disable completed */
	gmp_prtx_cfg.u64 = cvmx_read_csr_node(node, CVMX_BGXX_GMP_GMI_PRTX_CFG(index, xi.interface));

	/*
	 * Get the misc control for PCS. We will need to set the
	 * duplication amount.
	 */
	gmp_miscx_ctl.u64 = cvmx_read_csr_node(node, CVMX_BGXX_GMP_PCS_MISCX_CTL(index, xi.interface));

	/*
	 * Use GMXENO to force the link down if the status we get says
	 * it should be down.
	 */
	gmp_miscx_ctl.s.gmxeno = !link_info.s.link_up;

	/* Only change the duplex setting if the link is up */
	if (link_info.s.link_up)
		gmp_prtx_cfg.s.duplex = link_info.s.full_duplex;

	/* Do speed based setting for GMX */
	switch (link_info.s.speed) {
	case 10:
		gmp_prtx_cfg.s.speed = 0;
		gmp_prtx_cfg.s.speed_msb = 1;
		gmp_prtx_cfg.s.slottime = 0;
		/* Setting from GMX-603 */
		gmp_miscx_ctl.s.samp_pt = 25;
		cvmx_write_csr_node(node, CVMX_BGXX_GMP_GMI_TXX_SLOT(index, xi.interface), 64);
		cvmx_write_csr_node(node, CVMX_BGXX_GMP_GMI_TXX_BURST(index, xi.interface), 0);
		break;
	case 100:
		gmp_prtx_cfg.s.speed = 0;
		gmp_prtx_cfg.s.speed_msb = 0;
		gmp_prtx_cfg.s.slottime = 0;
		gmp_miscx_ctl.s.samp_pt = 0x5;
		cvmx_write_csr_node(node, CVMX_BGXX_GMP_GMI_TXX_SLOT(index, xi.interface), 64);
		cvmx_write_csr_node(node, CVMX_BGXX_GMP_GMI_TXX_BURST(index, xi.interface), 0);
		break;
	case 1000:
		gmp_prtx_cfg.s.speed = 1;
		gmp_prtx_cfg.s.speed_msb = 0;
		gmp_prtx_cfg.s.slottime = 1;
		gmp_miscx_ctl.s.samp_pt = 1;
		cvmx_write_csr_node(node, CVMX_BGXX_GMP_GMI_TXX_SLOT(index, xi.interface), 512);
		if (gmp_prtx_cfg.s.duplex)
			/* full duplex */
			cvmx_write_csr_node(node, CVMX_BGXX_GMP_GMI_TXX_BURST(index, xi.interface), 0);
		else
			/* half duplex */
			cvmx_write_csr_node(node, CVMX_BGXX_GMP_GMI_TXX_BURST(index, xi.interface), 8192);
		break;
	default:
		break;
	}

	/* Write the new misc control for PCS */
	cvmx_write_csr_node(node, CVMX_BGXX_GMP_PCS_MISCX_CTL(index, xi.interface),
		       gmp_miscx_ctl.u64);

	/* Write the new GMX settings with the port still disabled */
	cvmx_write_csr_node(node, CVMX_BGXX_GMP_GMI_PRTX_CFG(index, xi.interface),
			    gmp_prtx_cfg.u64);

	/* Read GMX CFG again to make sure the config completed */
	cvmx_read_csr_node(node, CVMX_BGXX_GMP_GMI_PRTX_CFG(index, xi.interface));

	/* Enable back BGX. */
	cmr_config.u64 = cvmx_read_csr_node(node,
					    CVMX_BGXX_CMRX_CONFIG(index,
								  xi.interface));
	if (debug)
		cvmx_dprintf("%s: Enabling tx and rx packets on %d:%d\n",
			     __func__, xi.interface, index);
	cmr_config.s.data_pkt_tx_en = 1;
	cmr_config.s.data_pkt_rx_en = 1;
	cvmx_write_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface),
			    cmr_config.u64);

	return 0;
}

/**
 * Enables or disables forward error correction
 *
 * @param	xiface	interface
 * @param	index	port index
 * @param	enable	set to true to enable FEC, false to disable
 *
 * @return	0 for success, -1 on error
 *
 * @NOTE:	If autonegotiation is enabled then autonegotiation will be
 *		restarted for negotiating FEC.
 */
int cvmx_helper_set_fec(int xiface, int index, bool enable)
{
	cvmx_bgxx_spux_fec_control_t spu_fec_control;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int interface = xi.interface;
	int node = xi.node;
	cvmx_helper_interface_mode_t mode;

	if (debug)
		cvmx_dprintf("%s: interface: %u:%d/%d: %sable\n",
			     __func__, interface, node, index,
	       enable ? "en" : "dis");
	if (cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_SIM)
		return 0;

	mode = cvmx_helper_bgx_get_mode(xiface, index);

	/* FEC is only supported for KR mode and XLAUI*/
	if (mode != CVMX_HELPER_INTERFACE_MODE_10G_KR &&
	    mode != CVMX_HELPER_INTERFACE_MODE_40G_KR4 &&
	    mode != CVMX_HELPER_INTERFACE_MODE_XLAUI &&
	    mode != CVMX_HELPER_INTERFACE_MODE_XFI)
		return 0;

	spu_fec_control.u64 =
		cvmx_read_csr_node(node,
				   CVMX_BGXX_SPUX_FEC_CONTROL(index, interface));

	spu_fec_control.s.fec_en = enable;
	cvmx_write_csr_node(node, CVMX_BGXX_SPUX_FEC_CONTROL(index, interface),
			    spu_fec_control.u64);

	cvmx_helper_set_port_fec(xiface, index, enable);

	if (cvmx_helper_get_port_autonegotiation(xiface, index))
		return cvmx_helper_set_autonegotiation(xiface, index, true);
	else
		return 0;
}

/**
 * Enables or disables autonegotiation for an interface.
 *
 * @param	xiface	interface to set autonegotiation
 * @param	index	port index
 * @param	enable	true to enable autonegotiation, false to disable it
 *
 * @return	0 for success, -1 on error.
 */
int cvmx_helper_set_autonegotiation(int xiface, int index, bool enable)
{
	union cvmx_bgxx_gmp_pcs_mrx_control gmp_control;
	union cvmx_bgxx_spux_an_control spu_an_control;
	union cvmx_bgxx_spux_an_adv spu_an_adv;
	union cvmx_bgxx_spux_fec_control spu_fec_control;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int interface = xi.interface;
	int node = xi.node;
	cvmx_helper_interface_mode_t mode;
	spu_fec_control.u64 = 0;

	if (debug)
		cvmx_dprintf("%s: interface: %u:%d/%d: %sable\n",
			     __func__, interface, node, index,
			     enable ? "en" : "dis");
	if (cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_SIM)
		return 0;

	mode = cvmx_helper_bgx_get_mode(xiface, index);

	switch (mode) {
	case CVMX_HELPER_INTERFACE_MODE_RGMII:
		enable = false;
	case CVMX_HELPER_INTERFACE_MODE_SGMII:
		gmp_control.u64 =
			cvmx_read_csr_node(node,
					   CVMX_BGXX_GMP_PCS_MRX_CONTROL(index,
									 interface));
		gmp_control.s.an_en = enable;
		gmp_control.s.rst_an = enable;
		cvmx_write_csr_node(node,
				    CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, interface),
				    gmp_control.u64);
		if (enable &&
		    CVMX_WAIT_FOR_FIELD64_NODE(node,
					       CVMX_BGXX_GMP_PCS_MRX_STATUS(index,
									    interface),
					       cvmx_bgxx_gmp_pcs_mrx_status_t,
					       an_cpt, ==, 1, 10000)) {
			cvmx_dprintf("SGMII%d: Port %d link timeout\n",
				     interface, index);
			return -1;
		}
		break;
	case CVMX_HELPER_INTERFACE_MODE_XAUI:
	case CVMX_HELPER_INTERFACE_MODE_XFI:
	case CVMX_HELPER_INTERFACE_MODE_10G_KR:
	case CVMX_HELPER_INTERFACE_MODE_XLAUI:
	case CVMX_HELPER_INTERFACE_MODE_40G_KR4:
		spu_an_adv.u64 =
		cvmx_read_csr_node(node,
				   CVMX_BGXX_SPUX_AN_ADV(index, interface));
		spu_an_adv.s.fec_req = spu_fec_control.s.fec_en;
		spu_an_adv.s.fec_able = 1;
		spu_an_adv.s.a100g_cr10 = 0;
		spu_an_adv.s.a40g_cr4 = 0;
		spu_an_adv.s.a40g_kr4 =
				(mode == CVMX_HELPER_INTERFACE_MODE_40G_KR4);
		spu_an_adv.s.a10g_kr =
				(mode == CVMX_HELPER_INTERFACE_MODE_10G_KR);
		spu_an_adv.s.a10g_kx4 = 0;
		spu_an_adv.s.a1g_kx = 0;
		spu_an_adv.s.xnp_able = 0;
		spu_an_adv.s.rf = 0;
		cvmx_write_csr_node(node,
				    CVMX_BGXX_SPUX_AN_ADV(index, interface),
				    spu_an_adv.u64);
		spu_fec_control.u64 =
			cvmx_read_csr_node(node,
					   CVMX_BGXX_SPUX_FEC_CONTROL(index,
								      interface));
		spu_an_control.u64 =
			cvmx_read_csr_node(node,
					   CVMX_BGXX_SPUX_AN_CONTROL(index,
								     interface));
		spu_an_control.s.an_en = enable;
		spu_an_control.s.xnp_en = 0;
		spu_an_control.s.an_restart = enable;
		cvmx_write_csr_node(node,
				    CVMX_BGXX_SPUX_AN_CONTROL(index, interface),
				    spu_an_control.u64);

		if (enable &&
		    CVMX_WAIT_FOR_FIELD64_NODE(node,
					       CVMX_BGXX_SPUX_AN_STATUS(index,
									interface),
					       cvmx_bgxx_spux_an_status_t,
					       an_complete, ==, 1, 10000)) {
			cvmx_dprintf("XAUI/XLAUI/XFI%d: Port %d link timeout\n",
				     interface, index);
			return -1;
		}
		break;
	default:
		break;
	}

	cvmx_helper_set_port_autonegotiation(xiface, index, enable);


	return 0;
}

/**
 * @INTERNAL
 * Return the link state of an IPD/PKO port as returned by
 * auto negotiation. The result of this function may not match
 * Octeon's link config if auto negotiation has changed since
 * the last call to cvmx_helper_link_set(). This is used by
 * interfaces using the bgx mac.
 *
 * @param xipd_port IPD/PKO port to query
 *
 * @return Link state
 */
cvmx_helper_link_info_t __cvmx_helper_bgx_sgmii_link_get(int xipd_port)
{
	cvmx_helper_link_info_t result;
	cvmx_bgxx_gmp_pcs_mrx_control_t gmp_control;
	cvmx_bgxx_gmp_pcs_miscx_ctl_t gmp_misc_ctl;
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(xipd_port);
	int node = xi.node;
	int index = cvmx_helper_get_interface_index_num(xp.port);

	result.u64 = 0;

	if (!cvmx_helper_is_port_valid(xiface, index))
		return result;

	if (debug)
		cvmx_dprintf("%s: interface %u:%d/%d\n",
		__func__, xi.node, xi.interface, index);

	if (cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_SIM) {
		/* The simulator gives you a simulated 1Gbps full duplex link */
		result.s.link_up = 1;
		result.s.full_duplex = 1;
		result.s.speed = 1000;
		return result;
	}

	gmp_control.u64 = cvmx_read_csr_node(node,
					     CVMX_BGXX_GMP_PCS_MRX_CONTROL(index,
									   xi.interface));
	if (gmp_control.s.loopbck1) {
		int qlm = cvmx_qlm_lmac(xiface, index);
		int speed;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			speed = cvmx_qlm_get_gbaud_mhz_node(node, qlm);
		else
			speed = cvmx_qlm_get_gbaud_mhz(qlm);
		/* Force 1Gbps full duplex link for internal loopback */
		result.s.link_up = 1;
		result.s.full_duplex = 1;
		result.s.speed = speed * 8 / 10;
		return result;
	}

	gmp_misc_ctl.u64 = cvmx_read_csr_node(node,
					      CVMX_BGXX_GMP_PCS_MISCX_CTL(index,
									  xi.interface));
	if (gmp_misc_ctl.s.mac_phy ||
	    cvmx_helper_get_port_force_link_up(xiface, index)) {
		int qlm = cvmx_qlm_lmac(xiface, index);
		int speed;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			speed = cvmx_qlm_get_gbaud_mhz_node(node, qlm);
		else
			speed = cvmx_qlm_get_gbaud_mhz(qlm);
		/* PHY Mode */
		/* Note that this also works for 1000base-X mode */

		result.s.speed = speed * 8 / 10;
		result.s.full_duplex = 1;
		result.s.link_up = 1;
		return result;
	} else {
		/* MAC Mode */
		result = __cvmx_helper_board_link_get(xipd_port);
	}
	return result;
}

/**
 * This sequence brings down the link for the XCV RGMII interface
 *
 * @param interface	Interface (BGX) number.  Port index is always 0
 */
static void __cvmx_helper_bgx_rgmii_link_set_down(int interface)
{
	union cvmx_xcv_reset xcv_reset;
	union cvmx_bgxx_cmrx_config cmr_config;
	union cvmx_bgxx_gmp_pcs_mrx_control mr_control;
	union cvmx_bgxx_cmrx_rx_fifo_len rx_fifo_len;
	union cvmx_bgxx_cmrx_tx_fifo_len tx_fifo_len;

	xcv_reset.u64 = cvmx_read_csr(CVMX_XCV_RESET);
	xcv_reset.s.rx_pkt_rst_n = 0;
	cvmx_write_csr(CVMX_XCV_RESET, xcv_reset.u64);
	cvmx_read_csr(CVMX_XCV_RESET);
	cvmx_wait_usec(10000);	/* Wait for 1 MTU */

	cmr_config.u64 = cvmx_read_csr(CVMX_BGXX_CMRX_CONFIG(0, interface));
	cmr_config.s.data_pkt_rx_en = 0;
	cvmx_write_csr(CVMX_BGXX_CMRX_CONFIG(0, interface), cmr_config.u64);

	/* Wait for RX and TX to be idle */
	do {
		rx_fifo_len.u64 =
			cvmx_read_csr(CVMX_BGXX_CMRX_RX_FIFO_LEN(0, interface));
		tx_fifo_len.u64 =
			cvmx_read_csr(CVMX_BGXX_CMRX_TX_FIFO_LEN(0, interface));
	} while (rx_fifo_len.s.fifo_len > 0 && tx_fifo_len.s.lmac_idle != 1);

	cmr_config.u64 = cvmx_read_csr(CVMX_BGXX_CMRX_CONFIG(0, interface));
	cmr_config.s.data_pkt_tx_en = 0;
	cvmx_write_csr(CVMX_BGXX_CMRX_CONFIG(0, interface), cmr_config.u64);

	xcv_reset.u64 = cvmx_read_csr(CVMX_XCV_RESET);
	xcv_reset.s.tx_pkt_rst_n = 0;
	cvmx_write_csr(CVMX_XCV_RESET, xcv_reset.u64);
	mr_control.u64 = cvmx_read_csr(CVMX_BGXX_GMP_PCS_MRX_CONTROL(0, interface));
	mr_control.s.pwr_dn = 1;
	cvmx_write_csr(CVMX_BGXX_GMP_PCS_MRX_CONTROL(0, interface),
		       mr_control.u64);
}

/**
 * Sets a BGS SGMII link down.
 *
 * @param node	Octeon node number
 * @param iface	BGX interface number
 * @param index	BGX port index
 */
static void __cvmx_helper_bgx_sgmii_link_set_down(int node, int iface,
						  int index)
{
	union cvmx_bgxx_gmp_pcs_miscx_ctl gmp_misc_ctl;
	union cvmx_bgxx_gmp_pcs_mrx_control gmp_control;
	union cvmx_bgxx_cmrx_config cmr_config;

	cmr_config.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index,
									iface));
	cmr_config.s.data_pkt_tx_en = 0;
	cmr_config.s.data_pkt_rx_en = 0;
	cvmx_write_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, iface),
			    cmr_config.u64);

	gmp_misc_ctl.u64 =
		cvmx_read_csr_node(node,
				   CVMX_BGXX_GMP_PCS_MISCX_CTL(index, iface));

	/* Disable autonegotiation only when in MAC mode. */
	if (gmp_misc_ctl.s.mac_phy == 0) {
		gmp_control.u64 =
			cvmx_read_csr_node(node,
					   CVMX_BGXX_GMP_PCS_MRX_CONTROL(index,
									 iface));
		gmp_control.s.an_en = 0;
		cvmx_write_csr_node(node,
				    CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, iface),
				    gmp_control.u64);
	}

	/* Use GMXENO to force the link down.  It will get reenabled later... */
	gmp_misc_ctl.s.gmxeno = 1;
	cvmx_write_csr_node(node, CVMX_BGXX_GMP_PCS_MISCX_CTL(index, iface),
		     gmp_misc_ctl.u64);
	cvmx_read_csr_node(node, CVMX_BGXX_GMP_PCS_MISCX_CTL(index, iface));
}

/**
 * @INTERNAL
 * Configure an IPD/PKO port for the specified link state. This
 * function does not influence auto negotiation at the PHY level.
 * The passed link state must always match the link state returned
 * by cvmx_helper_link_get(). It is normally best to use
 * cvmx_helper_link_autoconf() instead. This is used by interfaces
 * using the bgx mac.
 *
 * @param xipd_port  IPD/PKO port to configure
 * @param link_info The new link state
 *
 * @return Zero on success, negative on failure
 */
int __cvmx_helper_bgx_sgmii_link_set(int xipd_port,
				     cvmx_helper_link_info_t link_info)
{
	cvmx_bgxx_cmrx_config_t cmr_config;
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(xipd_port);
	int node = xi.node;
	int index = cvmx_helper_get_interface_index_num(xp.port);
	const int iface = xi.interface;
	int rc = 0;

	if (!cvmx_helper_is_port_valid(xiface, index))
		return 0;

	if (debug)
		cvmx_dprintf("%s: interface %u:%d/%d\n",
		__func__, xi.node, xi.interface, index);

	cmr_config.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index,
									iface));
	if (link_info.s.link_up) {
		cmr_config.s.enable = 1;
		cvmx_write_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, iface),
				    cmr_config.u64);
		/* Apply workaround for errata BGX-22429 */
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && index) {
			cvmx_bgxx_cmrx_config_t cmr0;
			cmr0.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMRX_CONFIG(0, iface));
			cmr0.s.enable = 1;
			cvmx_write_csr_node(node, CVMX_BGXX_CMRX_CONFIG(0, iface), cmr0.u64);
		}
		__cvmx_helper_bgx_sgmii_hardware_init_link(xiface, index);
	} else if (cvmx_helper_bgx_is_rgmii(xi.interface, index)) {
		if (debug)
			cvmx_dprintf("%s: Bringing down XCV RGMII interface %d\n",
				     __func__, xi.interface);
		__cvmx_helper_bgx_rgmii_link_set_down(xi.interface);
	} else { /* Link is down, not RGMII */
		__cvmx_helper_bgx_sgmii_link_set_down(node, iface, index);
		return 0;
	}
	rc = __cvmx_helper_bgx_sgmii_hardware_init_link_speed(xiface, index,
							      link_info);
	if (cvmx_helper_bgx_is_rgmii(xiface, index))
		rc = __cvmx_helper_bgx_rgmii_speed(link_info);

	return rc;
}


/**
 * @INTERNAL
 * Bringup XAUI interface. After this call packet I/O should be
 * fully functional.
 *
 * @param index port on interface to bring up
 * @param xiface Interface to bring up
 *
 * @return Zero on success, negative on failure
 */
static int __cvmx_helper_bgx_xaui_init(int index, int xiface)
{
	cvmx_bgxx_cmrx_config_t cmr_config;
	cvmx_bgxx_spux_br_pmd_control_t pmd_control;
	cvmx_bgxx_spux_misc_control_t spu_misc_control;
	cvmx_bgxx_spux_control1_t spu_control1;
	cvmx_bgxx_spux_an_control_t spu_an_control;
	cvmx_bgxx_spux_an_adv_t spu_an_adv;
	cvmx_bgxx_spux_fec_control_t spu_fec_control;
	cvmx_bgxx_spu_dbg_control_t spu_dbg_control;
	cvmx_bgxx_smux_tx_append_t  smu_tx_append;
	cvmx_bgxx_smux_tx_ctl_t smu_tx_ctl;
	cvmx_helper_interface_mode_t mode;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int interface = xi.interface;
	int node = xi.node;
	int use_auto_neg = 0;
	int kr_mode = 0;

	if (debug)
		cvmx_dprintf("%s: interface %u:%d/%d\n",
		__func__, xi.node, xi.interface, index);

	mode = cvmx_helper_bgx_get_mode(xiface, index);

	if (mode == CVMX_HELPER_INTERFACE_MODE_10G_KR
	    || mode == CVMX_HELPER_INTERFACE_MODE_40G_KR4) {
		kr_mode = 1;
		if (cvmx_helper_bgx_override_autoneg)
			use_auto_neg = cvmx_helper_bgx_override_autoneg(xiface,
									index);
		else
			use_auto_neg = cvmx_helper_get_port_autonegotiation(xiface,
									    index);
	}

	/* NOTE: This code was moved first, out of order compared to the HRM
	   because the RESET causes all SPU registers to loose their value */
	/* 4. Next, bring up the SMU/SPU and the BGX reconciliation layer logic: */
	/* 4a. Take SMU/SPU through a reset sequence. Write
	   BGX(0..5)_SPU(0..3)_CONTROL1[RESET] = 1. Read
	   BGX(0..5)_SPU(0..3)_CONTROL1[RESET] until it changes value to 0. Keep
	   BGX(0..5)_SPU(0..3)_MISC_CONTROL[RX_PACKET_DIS] = 1 to disable
	   reception. */
	if (cvmx_sysinfo_get()->board_type != CVMX_BOARD_TYPE_SIM) {
		spu_control1.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_CONTROL1(index, xi.interface));
		spu_control1.s.reset = 1;
		cvmx_write_csr_node(node, CVMX_BGXX_SPUX_CONTROL1(index, xi.interface), spu_control1.u64);

		/* 1. Wait for PCS to come out of reset */
		if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_BGXX_SPUX_CONTROL1(index, xi.interface),
				cvmx_bgxx_spux_control1_t, reset, ==, 0, 10000)) {
			cvmx_dprintf("BGX%d:%d: SPU stuck in reset\n", node, interface);
			return -1;
		}

		/* 2. Write BGX(0..5)_CMR(0..3)_CONFIG[ENABLE] to 0,
		      BGX(0..5)_SPU(0..3)_CONTROL1[LO_PWR] = 1 and
		      BGX(0..5)_SPU(0..3)_MISC_CONTROL[RX_PACKET_DIS] = 1. */
		spu_control1.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_CONTROL1(index, xi.interface));
		spu_control1.s.lo_pwr = 1;
		cvmx_write_csr_node(node, CVMX_BGXX_SPUX_CONTROL1(index, xi.interface), spu_control1.u64);

		spu_misc_control.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_MISC_CONTROL(index, xi.interface));
		spu_misc_control.s.rx_packet_dis = 1;
		cvmx_write_csr_node(node, CVMX_BGXX_SPUX_MISC_CONTROL(index, xi.interface), spu_misc_control.u64);

		/* 3. At this point, it may be appropriate to disable all BGX and SMU/SPU
		    interrupts, as a number of them will occur during bring-up of the Link.
		    - zero BGX(0..5)_SMU(0..3)_RX_INT
		    - zero BGX(0..5)_SMU(0..3)_TX_INT
		    - zero BGX(0..5)_SPU(0..3)_INT */
		cvmx_write_csr_node(node, CVMX_BGXX_SMUX_RX_INT(index, xi.interface),
			cvmx_read_csr_node(node, CVMX_BGXX_SMUX_RX_INT(index, xi.interface)));
		cvmx_write_csr_node(node, CVMX_BGXX_SMUX_TX_INT(index, xi.interface),
			cvmx_read_csr_node(node, CVMX_BGXX_SMUX_TX_INT(index, xi.interface)));
		cvmx_write_csr_node(node, CVMX_BGXX_SPUX_INT(index, xi.interface),
			cvmx_read_csr_node(node, CVMX_BGXX_SPUX_INT(index, xi.interface)));

		/* 4. Configure the BGX LMAC. */
		/* 4a. Configure the LMAC type (40GBASE-R/10GBASE-R/RXAUI/XAUI) and
		     SerDes selection in the BGX(0..5)_CMR(0..3)_CONFIG register, but keep
		     the ENABLE, DATA_PKT_TX_EN and DATA_PKT_RX_EN bits clear. */
		/* Already done in bgx_setup_one_time */

		/* 4b. Write BGX(0..5)_SPU(0..3)_CONTROL1[LO_PWR] = 1 and
		     BGX(0..5)_SPU(0..3)_MISC_CONTROL[RX_PACKET_DIS] = 1. */
		/* 4b. Initialize the selected SerDes lane(s) in the QLM. See Section
		      28.1.2.2 in the GSER chapter. */
		/* Already done in QLM setup */

		/* 4c. For 10GBASE-KR or 40GBASE-KR, enable link training by writing
		     BGX(0..5)_SPU(0..3)_BR_PMD_CONTROL[TRAIN_EN] = 1. */

		if (kr_mode && !OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)) {
			cvmx_write_csr_node(node, CVMX_BGXX_SPUX_BR_PMD_LP_CUP(index, interface), 0);
			cvmx_write_csr_node(node, CVMX_BGXX_SPUX_BR_PMD_LD_CUP(index, interface), 0);
			cvmx_write_csr_node(node, CVMX_BGXX_SPUX_BR_PMD_LD_REP(index, interface), 0);
			pmd_control.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, interface));
			pmd_control.s.train_en = 1;
			cvmx_write_csr_node(node, CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, interface), pmd_control.u64);
		}
	} else { /* enable for simulator */
		cmr_config.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface));
		cmr_config.s.enable = 1;
		cvmx_write_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface), cmr_config.u64);
	}

	/* 4d. Program all other relevant BGX configuration while
	       BGX(0..5)_CMR(0..3)_CONFIG[ENABLE] = 0. This includes all things
	       described in this chapter. */
	/* Always add FCS to PAUSE frames */
	smu_tx_append.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SMUX_TX_APPEND(index, xi.interface));
	smu_tx_append.s.fcs_c = 1;
	cvmx_write_csr_node(node, CVMX_BGXX_SMUX_TX_APPEND(index, xi.interface), smu_tx_append.u64);

	/* 4e. If Forward Error Correction is desired for 10GBASE-R or 40GBASE-R,
	       enable it by writing BGX(0..5)_SPU(0..3)_FEC_CONTROL[FEC_EN] = 1. */
	if (cvmx_sysinfo_get()->board_type != CVMX_BOARD_TYPE_SIM) {
		/* FEC is optional for 10GBASE-KR, 40GBASE-KR4, and XLAUI. We're going
		to disable it by default */
		spu_fec_control.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_FEC_CONTROL(index, xi.interface));
		if (cvmx_helper_bgx_override_fec)
			spu_fec_control.s.fec_en =
				cvmx_helper_bgx_override_fec(xiface, index);
		else
			spu_fec_control.s.fec_en =
				cvmx_helper_get_port_fec(xiface, index);
		cvmx_write_csr_node(node, CVMX_BGXX_SPUX_FEC_CONTROL(index, xi.interface), spu_fec_control.u64);

		/* 4f. If Auto-Negotiation is desired, configure and enable
		      Auto-Negotiation as described in Section 33.6.2. */
		spu_an_control.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_AN_CONTROL(index, xi.interface));
		/* Disable extended next pages */
		spu_an_control.s.xnp_en = 0;
		spu_an_control.s.an_en = use_auto_neg;
		cvmx_write_csr_node(node, CVMX_BGXX_SPUX_AN_CONTROL(index, xi.interface), spu_an_control.u64);

		spu_fec_control.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_FEC_CONTROL(index, xi.interface));
		spu_an_adv.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_AN_ADV(index, xi.interface));
		spu_an_adv.s.fec_req = spu_fec_control.s.fec_en;
		spu_an_adv.s.fec_able = 1;
		spu_an_adv.s.a100g_cr10 = 0;
		spu_an_adv.s.a40g_cr4 = 0;
		spu_an_adv.s.a40g_kr4 = (mode == CVMX_HELPER_INTERFACE_MODE_40G_KR4);
		spu_an_adv.s.a10g_kr = (mode == CVMX_HELPER_INTERFACE_MODE_10G_KR);
		spu_an_adv.s.a10g_kx4 = 0;
		spu_an_adv.s.a1g_kx = 0;
		spu_an_adv.s.xnp_able = 0;
		spu_an_adv.s.rf = 0;
		cvmx_write_csr_node(node, CVMX_BGXX_SPUX_AN_ADV(index, xi.interface), spu_an_adv.u64);

		/* 3. Set BGX(0..5)_SPU_DBG_CONTROL[AN_ARB_LINK_CHK_EN] = 1. */
		spu_dbg_control.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPU_DBG_CONTROL(xi.interface));
		spu_dbg_control.s.an_nonce_match_dis = 1; /* Needed for loopback */
		spu_dbg_control.s.an_arb_link_chk_en |= kr_mode;
		cvmx_write_csr_node(node, CVMX_BGXX_SPU_DBG_CONTROL(xi.interface), spu_dbg_control.u64);

		/* 4. Execute the link bring-up sequence in Section 33.6.3. */

		/* 5. If the auto-negotiation protocol is successful,
		    BGX(0..5)_SPU(0..3)_AN_ADV[AN_COMPLETE] is set along with
		    BGX(0..5)_SPU(0..3)_INT[AN_COMPLETE] when the link is up. */

		/* 3h. Set BGX(0..5)_CMR(0..3)_CONFIG[ENABLE] = 1 and
		    BGX(0..5)_SPU(0..3)_CONTROL1[LO_PWR] = 0 to enable the LMAC. */
		cmr_config.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface));
		cmr_config.s.enable = 1;
		cvmx_write_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface), cmr_config.u64);
		/* Apply workaround for errata BGX-22429 */
		if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && index) {
			cvmx_bgxx_cmrx_config_t cmr0;
			cmr0.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMRX_CONFIG(0, xi.interface));
			cmr0.s.enable = 1;
			cvmx_write_csr_node(node, CVMX_BGXX_CMRX_CONFIG(0, xi.interface), cmr0.u64);
		}

		spu_control1.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_CONTROL1(index, xi.interface));
		spu_control1.s.lo_pwr = 0;
		cvmx_write_csr_node(node, CVMX_BGXX_SPUX_CONTROL1(index, xi.interface), spu_control1.u64);
	}

	/* 4g. Set the polarity and lane swapping of the QLM SerDes. Refer to
	   Section 33.4.1, BGX(0..5)_SPU(0..3)_MISC_CONTROL[XOR_TXPLRT,XOR_RXPLRT]
	   and BGX(0..5)_SPU(0..3)_MISC_CONTROL[TXPLRT,RXPLRT]. */

	/* 4c. Write BGX(0..5)_SPU(0..3)_CONTROL1[LO_PWR] = 0. */
	spu_control1.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_CONTROL1(index, xi.interface));
	spu_control1.s.lo_pwr = 0;
	cvmx_write_csr_node(node, CVMX_BGXX_SPUX_CONTROL1(index, xi.interface), spu_control1.u64);

	/* 4d. Select Deficit Idle Count mode and unidirectional enable/disable
	   via BGX(0..5)_SMU(0..3)_TX_CTL[DIC_EN,UNI_EN]. */
	smu_tx_ctl.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SMUX_TX_CTL(index, xi.interface));
	smu_tx_ctl.s.dic_en = 1;
	smu_tx_ctl.s.uni_en = 0;
	cvmx_write_csr_node(node, CVMX_BGXX_SMUX_TX_CTL(index, xi.interface), smu_tx_ctl.u64);

	{
		/* Calculate the number of s-clk cycles per usec. */
		const uint64_t clock_mhz = cvmx_clock_get_rate_node(node, CVMX_CLOCK_SCLK) / 1000000;
		cvmx_bgxx_spu_dbg_control_t dbg_control;
		dbg_control.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPU_DBG_CONTROL(xi.interface));
		dbg_control.s.us_clk_period = clock_mhz - 1;
		cvmx_write_csr_node(node, CVMX_BGXX_SPU_DBG_CONTROL(xi.interface), dbg_control.u64);
	}
	/* The PHY often takes at least 100ms to stabilize */
	__cvmx_helper_bgx_interface_enable_delay(mode);
	return 0;
}

static void __cvmx_bgx_start_training(int node, int unit, int index)
{
	cvmx_bgxx_spux_int_t spu_int;
	cvmx_bgxx_spux_br_pmd_control_t pmd_control;
	cvmx_bgxx_spux_an_control_t an_control;

	/* Clear the training interrupts (W1C) */
	spu_int.u64 = 0;
	spu_int.s.training_failure = 1;
	spu_int.s.training_done = 1;
	cvmx_write_csr_node(node, CVMX_BGXX_SPUX_INT(index, unit), spu_int.u64);

	/* These registers aren't cleared when training is restarted. Manually
	   clear them as per Errata BGX-20968. */
	cvmx_write_csr_node(node, CVMX_BGXX_SPUX_BR_PMD_LP_CUP(index, unit), 0);
	cvmx_write_csr_node(node, CVMX_BGXX_SPUX_BR_PMD_LD_CUP(index, unit), 0);
	cvmx_write_csr_node(node, CVMX_BGXX_SPUX_BR_PMD_LD_REP(index, unit), 0);

	/* Disable autonegotiation */
	an_control.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_AN_CONTROL(index, unit));
	an_control.s.an_en = 0;
	cvmx_write_csr_node(node, CVMX_BGXX_SPUX_AN_CONTROL(index, unit), an_control.u64);
	cvmx_wait_usec(1);

	/* Restart training */
	pmd_control.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, unit));
	pmd_control.s.train_en = 1;
	cvmx_write_csr_node(node, CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, unit), pmd_control.u64);

	cvmx_wait_usec(1);
	pmd_control.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, unit));
	pmd_control.s.train_restart = 1;
	cvmx_write_csr_node(node, CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, unit), pmd_control.u64);
}

static void __cvmx_bgx_restart_training(int node, int unit, int index)
{
	cvmx_bgxx_spux_int_t spu_int;
	cvmx_bgxx_spux_br_pmd_control_t pmd_control;

	/* Clear the training interrupts (W1C) */
	spu_int.u64 = 0;
	spu_int.s.training_failure = 1;
	spu_int.s.training_done = 1;
	cvmx_write_csr_node(node, CVMX_BGXX_SPUX_INT(index, unit), spu_int.u64);

	cvmx_wait_usec(1700);  /* Wait 1.7 msec */

	/* These registers aren't cleared when training is restarted. Manually
	   clear them as per Errata BGX-20968. */
	cvmx_write_csr_node(node, CVMX_BGXX_SPUX_BR_PMD_LP_CUP(index, unit), 0);
	cvmx_write_csr_node(node, CVMX_BGXX_SPUX_BR_PMD_LD_CUP(index, unit), 0);
	cvmx_write_csr_node(node, CVMX_BGXX_SPUX_BR_PMD_LD_REP(index, unit), 0);

	/* Restart training */
	pmd_control.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, unit));
	pmd_control.s.train_restart = 1;
	cvmx_write_csr_node(node, CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, unit), pmd_control.u64);
}

/*
 * @INTERNAL
 * Wrapper function to configure the BGX, does not enable.
 *
 * @param xipd_port IPD/PKO port to configure.
 * @param phy_pres  If set, enable disparity, only applies to RXAUI interface
 *
 * @return Zero on success, negative on failure.
 */
int __cvmx_helper_bgx_port_init(int xipd_port, int phy_pres)
{
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(xipd_port);
	int index = cvmx_helper_get_interface_index_num(xp.port);
	cvmx_helper_interface_mode_t mode;

	if (debug)
		cvmx_dprintf("%s: interface %u:%d/%d\n",
		__func__, xi.node, xi.interface, index);

	mode = cvmx_helper_bgx_get_mode(xiface, index);

	__cvmx_bgx_common_init_pknd(xiface, index);

	if (mode == CVMX_HELPER_INTERFACE_MODE_SGMII
	    || mode == CVMX_HELPER_INTERFACE_MODE_RGMII) {
		cvmx_bgxx_gmp_gmi_txx_thresh_t gmi_tx_thresh;
		cvmx_bgxx_gmp_gmi_txx_append_t gmp_txx_append;
		cvmx_bgxx_gmp_gmi_txx_sgmii_ctl_t gmp_sgmii_ctl;

		/* Set TX Threshold */
		gmi_tx_thresh.u64 = 0;
		gmi_tx_thresh.s.cnt = 0x20;
		cvmx_write_csr_node(xi.node, CVMX_BGXX_GMP_GMI_TXX_THRESH(index, xi.interface),
				    gmi_tx_thresh.u64);
		__cvmx_helper_bgx_sgmii_hardware_init_one_time(xiface, index);
		gmp_txx_append.u64 = cvmx_read_csr_node(xi.node,
					CVMX_BGXX_GMP_GMI_TXX_APPEND(index, xi.interface));
		gmp_sgmii_ctl.u64 = cvmx_read_csr_node(xi.node, CVMX_BGXX_GMP_GMI_TXX_SGMII_CTL(index, xi.interface));
		gmp_sgmii_ctl.s.align = gmp_txx_append.s.preamble ? 0 : 1;
		cvmx_write_csr_node(xi.node, CVMX_BGXX_GMP_GMI_TXX_SGMII_CTL(index, xi.interface),
			    gmp_sgmii_ctl.u64);
		if (mode == CVMX_HELPER_INTERFACE_MODE_RGMII) {
			/* Disable XCV interface when initialized */
			union cvmx_xcv_reset xcv_reset;
			if (debug)
				cvmx_dprintf("%s: Disabling RGMII XCV interface\n",
					     __func__);
			xcv_reset.u64 = cvmx_read_csr(CVMX_XCV_RESET);
			xcv_reset.s.enable = 0;
			xcv_reset.s.tx_pkt_rst_n = 0;
			xcv_reset.s.rx_pkt_rst_n = 0;
			cvmx_write_csr(CVMX_XCV_RESET, xcv_reset.u64);
		}
	} else {
		int res, cred;
		cvmx_bgxx_smux_tx_thresh_t smu_tx_thresh;

		res = __cvmx_helper_bgx_xaui_init(index, xiface);
		if (res == -1) {
#ifdef DEBUG_BGX
			cvmx_dprintf("Failed to enable XAUI for %d:BGX(%d,%d)\n", xi.node, xi.interface, index);
#endif
			return res;
		}
		/* See BVX_SMU_TX_THRESH register descriptin */
		cred = __cvmx_helper_bgx_fifo_size(xiface, index) >> 4;
		smu_tx_thresh.u64 = 0;
		smu_tx_thresh.s.cnt = cred - 10;
		cvmx_write_csr_node(xi.node,
			CVMX_BGXX_SMUX_TX_THRESH(index, xi.interface),
			smu_tx_thresh.u64);
		if (debug)
			cvmx_dprintf("%s: BGX%d:%d TX-thresh=%d\n",
				__func__, xi.interface, index, smu_tx_thresh.s.cnt);

		/* Set disparity for RXAUI interface as described in the
		Marvell RXAUI Interface specification. */
		if (mode == CVMX_HELPER_INTERFACE_MODE_RXAUI && phy_pres) {
			cvmx_bgxx_spux_misc_control_t misc_control;
			misc_control.u64 = cvmx_read_csr_node(xi.node,
					CVMX_BGXX_SPUX_MISC_CONTROL(index, xi.interface));
			misc_control.s.intlv_rdisp = 1;
			cvmx_write_csr_node(xi.node, CVMX_BGXX_SPUX_MISC_CONTROL(index, xi.interface),
					    misc_control.u64);
		}
	}
	return 0;
}
EXPORT_SYMBOL(__cvmx_helper_bgx_port_init);


/**
 * @INTERNAL
 * Configure a port for internal and/or external loopback. Internal loopback
 * causes packets sent by the port to be received by Octeon. External loopback
 * causes packets received from the wire to sent out again. This is used by
 * interfaces using the bgx mac.
 *
 * @param xipd_port IPD/PKO port to loopback.
 * @param enable_internal
 *                 Non zero if you want internal loopback
 * @param enable_external
 *                 Non zero if you want external loopback
 *
 * @return Zero on success, negative on failure.
 */
int __cvmx_helper_bgx_sgmii_configure_loopback(int xipd_port, int enable_internal,
					   int enable_external)
{
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(xipd_port);
	int node = xi.node;
	int index = cvmx_helper_get_interface_index_num(xp.port);
	cvmx_bgxx_gmp_pcs_mrx_control_t gmp_mrx_control;
	cvmx_bgxx_gmp_pcs_miscx_ctl_t gmp_misc_ctl;

	if (!cvmx_helper_is_port_valid(xiface, index))
		return 0;

	if (debug)
		cvmx_dprintf("%s: interface %u:%d/%d\n",
		__func__, xi.node, xi.interface, index);

	if (cvmx_helper_bgx_is_rgmii(xi.interface, index)) {
		cvmx_xcv_ctl_t xcv_ctl;
		cvmx_helper_link_info_t link_info;

		xcv_ctl.u64 = cvmx_read_csr(CVMX_XCV_CTL);
		xcv_ctl.s.lpbk_int = enable_internal;
		xcv_ctl.s.lpbk_ext = enable_external;
		cvmx_write_csr(CVMX_XCV_CTL, xcv_ctl.u64);

		/* Initialize link and speed */
		__cvmx_helper_bgx_sgmii_hardware_init_link(xiface, index);
		link_info = __cvmx_helper_bgx_sgmii_link_get(xipd_port);
		__cvmx_helper_bgx_sgmii_hardware_init_link_speed(xiface, index, link_info);
		__cvmx_helper_bgx_rgmii_speed(link_info);
	} else {
		gmp_mrx_control.u64 = cvmx_read_csr_node(node, CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, xi.interface));
		gmp_mrx_control.s.loopbck1 = enable_internal;
		cvmx_write_csr_node(node, CVMX_BGXX_GMP_PCS_MRX_CONTROL(index, xi.interface), gmp_mrx_control.u64);

		gmp_misc_ctl.u64 = cvmx_read_csr_node(node, CVMX_BGXX_GMP_PCS_MISCX_CTL(index, xi.interface));
		gmp_misc_ctl.s.loopbck2 = enable_external;
		cvmx_write_csr_node(node, CVMX_BGXX_GMP_PCS_MISCX_CTL(index, xi.interface), gmp_misc_ctl.u64);
		__cvmx_helper_bgx_sgmii_hardware_init_link(xiface, index);
	}


	return 0;
}

static int __cvmx_helper_bgx_xaui_link_init(int index, int xiface)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int node = xi.node;
	cvmx_bgxx_spux_status1_t spu_status1;
	cvmx_bgxx_spux_status2_t spu_status2;
	cvmx_bgxx_spux_br_status2_t br_status2;
	cvmx_bgxx_spux_int_t spu_int;
	cvmx_bgxx_spux_misc_control_t spu_misc_control;
	cvmx_bgxx_cmrx_config_t cmr_config;
	cvmx_helper_interface_mode_t mode;
	int use_training = 0;
	int rgmii_first = 0;
	int qlm = cvmx_qlm_lmac(xiface, index);
	int use_ber = 0;

	if (debug)
		cvmx_dprintf("%s: interface %u:%d/%d\n",
		__func__, xi.node, xi.interface, index);

	rgmii_first = cvmx_helper_bgx_is_rgmii(xi.interface, index);

	mode = cvmx_helper_bgx_get_mode(xiface, index);
	if (mode == CVMX_HELPER_INTERFACE_MODE_10G_KR
	    || mode == CVMX_HELPER_INTERFACE_MODE_40G_KR4)
		use_training = 1;

	if ((mode == CVMX_HELPER_INTERFACE_MODE_XFI
	     || mode == CVMX_HELPER_INTERFACE_MODE_XLAUI
	     || mode == CVMX_HELPER_INTERFACE_MODE_10G_KR
	     || mode == CVMX_HELPER_INTERFACE_MODE_40G_KR4)
	     && (cvmx_sysinfo_get()->board_type != CVMX_BOARD_TYPE_SIM))
		use_ber = 1;

	/* Disable packet reception, CMR as well as SPU block */
	cmr_config.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface));
	cmr_config.s.data_pkt_tx_en = 0;
	cmr_config.s.data_pkt_rx_en = 0;
	cvmx_write_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface),
			    cmr_config.u64);
	spu_misc_control.u64 = cvmx_read_csr_node(node,
						  CVMX_BGXX_SPUX_MISC_CONTROL(index, xi.interface));
	spu_misc_control.s.rx_packet_dis = 1;
	cvmx_write_csr_node(node, CVMX_BGXX_SPUX_MISC_CONTROL(index, xi.interface),
			    spu_misc_control.u64);

	if (cvmx_sysinfo_get()->board_type != CVMX_BOARD_TYPE_SIM) {
		cvmx_bgxx_spux_an_control_t spu_an_control;
		cvmx_bgxx_spux_an_status_t spu_an_status;
		cvmx_bgxx_spux_br_pmd_control_t pmd_control;

		spu_an_control.u64 = cvmx_read_csr_node(node,
					CVMX_BGXX_SPUX_AN_CONTROL(index, xi.interface));
		if (spu_an_control.s.an_en) {
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)) {
				cvmx_bgxx_spux_int_t spu_int;
				spu_int.u64 = cvmx_read_csr_node(node,
						CVMX_BGXX_SPUX_INT(index, xi.interface));
				if (!spu_int.s.an_link_good) {
					static uint64_t restart_auto_neg[2][6][4] = { [0 ... 1][0 ... 5] = { [0 ... 3] = 0 } };
					uint64_t now = cvmx_clock_get_count(CVMX_CLOCK_SCLK);
					uint64_t next_restart = restart_auto_neg[node][xi.interface][index]
						 + (cvmx_clock_get_rate_node(node, CVMX_CLOCK_SCLK) * 2);
					if (now >= next_restart)
						return -1;

					restart_auto_neg[node][xi.interface][index] = now;

					/* Clear the auto negotiation (W1C) */
					spu_int.u64 = 0;
					spu_int.s.an_complete = 1;
					spu_int.s.an_link_good = 1;
					spu_int.s.an_page_rx = 1;
					cvmx_write_csr_node(node, CVMX_BGXX_SPUX_INT(index, xi.interface), spu_int.u64);
					/* Restart auto negotiation */
					spu_an_control.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_AN_CONTROL(index, xi.interface));
					spu_an_control.s.an_restart = 1;
					cvmx_write_csr_node(node, CVMX_BGXX_SPUX_AN_CONTROL(index, xi.interface), spu_an_control.u64);
					return -1;
				}
			} else {
				spu_an_status.u64 = cvmx_read_csr_node(node,
						CVMX_BGXX_SPUX_AN_STATUS(index, xi.interface));
				if (!spu_an_status.s.an_complete) {
					static uint64_t restart_auto_neg[2][6][4] = { [0 ... 1][0 ... 5] = { [0 ... 3] = 0 } };
					uint64_t now = cvmx_clock_get_count(CVMX_CLOCK_SCLK);
					uint64_t next_restart = restart_auto_neg[node][xi.interface][index]
						 + (cvmx_clock_get_rate_node(node, CVMX_CLOCK_SCLK) * 2);
					if (now >= next_restart) {
#ifdef DEBUG_BGX
						cvmx_dprintf("WARNING: BGX%d:%d: Waiting for autoneg to complete\n", xi.interface, index);
#endif
						return -1;
					}

					restart_auto_neg[node][xi.interface][index] = now;
					/* Restart auto negotiation */
					spu_an_control.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_AN_CONTROL(index, xi.interface));
					spu_an_control.s.an_restart = 1;
					cvmx_write_csr_node(node, CVMX_BGXX_SPUX_AN_CONTROL(index, xi.interface), spu_an_control.u64);
					return -1;
				}
			}
		}

		if (use_training) {
			spu_int.u64 = cvmx_read_csr_node(node,
						  CVMX_BGXX_SPUX_INT(index, xi.interface));
			pmd_control.u64 = cvmx_read_csr_node(node,
						CVMX_BGXX_SPUX_BR_PMD_CONTROL(index, xi.interface));
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)
			    && pmd_control.s.train_en == 0) {
				__cvmx_bgx_start_training(node, xi.interface, index);
				return -1;
			}
			cvmx_qlm_gser_errata_27882(node, qlm, index);
			spu_int.u64 = cvmx_read_csr_node(node,
						CVMX_BGXX_SPUX_INT(index, xi.interface));

			if (spu_int.s.training_failure && !OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)) {
				__cvmx_bgx_restart_training(node, xi.interface, index);
				return -1;
			}
			if (!spu_int.s.training_done) {
				cvmx_dprintf("Waiting for link training\n");
				return -1;
			}
		}

		/* (GSER-21957) GSER RX Equalization may make >= 5gbaud non-KR
		   channel with DXAUI, RXAUI, XFI and XLAUI, we need to perform
		   RX equalization when the link is receiving data the first time */
		if (use_training == 0) {
			int lane = index;
			cvmx_bgxx_spux_control1_t control1;

			cmr_config.u64 = cvmx_read_csr_node(node,
					CVMX_BGXX_CMRX_CONFIG(index, xi.interface));
			control1.u64 = cvmx_read_csr_node(node,
					CVMX_BGXX_SPUX_CONTROL1(index, xi.interface));
			if (control1.s.loopbck) {
				/* Skip RX equalization when in loopback */
			} else if (mode == CVMX_HELPER_INTERFACE_MODE_XLAUI
				   || mode == CVMX_HELPER_INTERFACE_MODE_XAUI) {
				lane = -1;
				if (__cvmx_qlm_rx_equalization(node, qlm, lane)) {
#ifdef DEBUG_BGX
					cvmx_dprintf("%d:%d:%d: Waiting for RX Equalization on QLM%d\n", node, xi.interface, index, qlm);
#endif
					return -1;
				}
				/* If BGX2 uses both dlms, then configure other dlm also. */
				if (OCTEON_IS_MODEL(OCTEON_CN73XX) && xi.interface == 2) {
					if (__cvmx_qlm_rx_equalization(node, 6, lane)) {
#ifdef DEBUG_BGX
						cvmx_dprintf("%d:%d:%d: Waiting for RX Equalization on QLM%d\n", node, xi.interface, index, qlm);
#endif
						return -1;
					}
				}
			/* RXAUI */
			} else if (mode == CVMX_HELPER_INTERFACE_MODE_RXAUI) {
				lane = index * 2;
				if (OCTEON_IS_MODEL(OCTEON_CN73XX)
				    && index >= 2
				    && xi.interface == 2) {
					lane = 0;
				}
				if (rgmii_first)
					lane--;
				if (__cvmx_qlm_rx_equalization(node, qlm, lane)
				    || __cvmx_qlm_rx_equalization(node, qlm, lane + 1)) {
#ifdef DEBUG_BGX
					cvmx_dprintf("%d:%d:%d: Waiting for RX Equalization on QLM%d\n", node, xi.interface, index, qlm);
#endif
					return -1;
				}
			/* XFI */
			} else if (cmr_config.s.lmac_type != 5) {
				if (rgmii_first)
					lane--;
				if (OCTEON_IS_MODEL(OCTEON_CN73XX)
				    && index >= 2
				    && xi.interface == 2) {
					lane = index - 2;
				} else if (OCTEON_IS_MODEL(OCTEON_CNF75XX)
					   && index >= 2) {
					lane = index - 2;
				}
				if (__cvmx_qlm_rx_equalization(node, qlm, lane)) {
#ifdef DEBUG_BGX
					cvmx_dprintf("%d:%d:%d: Waiting for RX Equalization on QLM%d\n", node, xi.interface, index, qlm);
#endif
					return -1;
				}
			}
		}

		if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_BGXX_SPUX_CONTROL1(index, xi.interface),
					  cvmx_bgxx_spux_control1_t, reset, ==, 0, 10000)) {
#ifdef DEBUG_BGX
			cvmx_dprintf("ERROR: %d:BGX%d:%d: PCS in reset", node, xi.interface, index);
#endif
			return -1;
		}

		if (use_ber) {
			if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_BGXX_SPUX_BR_STATUS1(index, xi.interface),
					  cvmx_bgxx_spux_br_status1_t, blk_lock, ==, 1, 10000)) {
#ifdef DEBUG_BGX
				cvmx_dprintf("ERROR: %d:BGX%d:%d: BASE-R PCS block not locked\n", node, xi.interface, index);

				if (mode == CVMX_HELPER_INTERFACE_MODE_XLAUI
				    || mode == CVMX_HELPER_INTERFACE_MODE_40G_KR4) {
					cvmx_bgxx_spux_br_algn_status_t bstatus;
					bstatus.u64 = cvmx_read_csr_node(node,
						CVMX_BGXX_SPUX_BR_ALGN_STATUS(index, xi.interface));
					cvmx_dprintf("ERROR: %d:BGX%d:%d: LANE BLOCK_LOCK:%x LANE MARKER_LOCK:%x\n",
						node, xi.interface, index,
						bstatus.s.block_lock,
						bstatus.s.marker_lock);
				}
#endif
                		return -1;
			}
		} else {
			/* (5) Check to make sure that the link appears up and stable. */
			/* Wait for PCS to be aligned */
			if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_BGXX_SPUX_BX_STATUS(index, xi.interface),
				  cvmx_bgxx_spux_bx_status_t, alignd, ==, 1, 10000)) {
#ifdef DEBUG_BGX
				cvmx_dprintf("ERROR: %d:BGX%d:%d: PCS not aligned\n", node, xi.interface, index);
#endif
				return -1;
			}
		}

		if (use_ber) {
			/* Set the BGXX_SPUX_BR_STATUS2.latched_lock bit (latching low).
			   This will be checked prior to enabling packet tx and rx,
			   ensuring block lock is sustained throughout the BGX link-up
			   procedure */
			br_status2.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_BR_STATUS2(index, xi.interface));
			br_status2.s.latched_lock = 1;
			cvmx_write_csr_node(node, CVMX_BGXX_SPUX_BR_STATUS2(index, xi.interface), br_status2.u64);
		}

		/* Clear rcvflt bit (latching high) and read it back */
		spu_status2.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_STATUS2(index, xi.interface));
		spu_status2.s.rcvflt = 1;
		cvmx_write_csr_node(node, CVMX_BGXX_SPUX_STATUS2(index, xi.interface), spu_status2.u64);

		spu_status2.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_STATUS2(index, xi.interface));
		if (spu_status2.s.rcvflt) {
#ifdef DEBUG_BGX
			cvmx_dprintf("ERROR: %d:BGX%d:%d: Receive fault, need to retry\n",
					node, xi.interface, index);
#endif
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && use_training)
				__cvmx_bgx_restart_training(node, xi.interface, index);
			/* cvmx_dprintf("training restarting\n"); */
			return -1;
		}

		/* Wait for MAC RX to be ready */
		if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_BGXX_SMUX_RX_CTL(index, xi.interface),
					  cvmx_bgxx_smux_rx_ctl_t, status, ==, 0, 10000)) {
#ifdef DEBUG_BGX
			cvmx_dprintf("ERROR: %d:BGX%d:%d: RX not ready\n", node, xi.interface, index);
#endif
			return -1;
		}

		/* Wait for BGX RX to be idle */
		if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_BGXX_SMUX_CTRL(index, xi.interface),
				  cvmx_bgxx_smux_ctrl_t, rx_idle, ==, 1, 10000)) {
#ifdef DEBUG_BGX
			cvmx_dprintf("ERROR: %d:BGX%d:%d: RX not idle\n", node, xi.interface, index);
#endif
			return -1;
		}

		/* Wait for GMX TX to be idle */
		if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_BGXX_SMUX_CTRL(index, xi.interface),
				  cvmx_bgxx_smux_ctrl_t, tx_idle, ==, 1, 10000)) {
#ifdef DEBUG_BGX
			cvmx_dprintf("ERROR: %d:BGX%d:%d: TX not idle\n", node, xi.interface, index);
#endif
			return -1;
		}

		/* rcvflt should still be 0 */
		spu_status2.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_STATUS2(index, xi.interface));
		if (spu_status2.s.rcvflt) {
#ifdef DEBUG_BGX
			cvmx_dprintf("ERROR: %d:BGX%d:%d: Receive fault, need to retry\n", node, xi.interface, index);
#endif
			return -1;
		}

		/* Receive link is latching low. Force it high and verify it */
		spu_status1.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_STATUS1(index, xi.interface));
		spu_status1.s.rcv_lnk = 1;
		cvmx_write_csr_node(node, CVMX_BGXX_SPUX_STATUS1(index, xi.interface), spu_status1.u64);

		if (CVMX_WAIT_FOR_FIELD64_NODE(node, CVMX_BGXX_SPUX_STATUS1(index, xi.interface),
				cvmx_bgxx_spux_status1_t, rcv_lnk, ==, 1, 10000)) {
#ifdef DEBUG_BGX
			cvmx_dprintf("ERROR: %d:BGX%d:%d: Receive link down\n", node, xi.interface, index);
#endif
			return -1;
		}
	}

	if (use_ber) {
		/* Clear out the BGX error counters/bits. These errors are expected
		   as part of the BGX link up procedure */
		/* BIP_ERR counters clear as part of this read */
		cvmx_read_csr_node(node, CVMX_BGXX_SPUX_BR_BIP_ERR_CNT(index, xi.interface));
		/* BER_CNT and ERR_BLKs clear as part of this read */
		br_status2.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_BR_STATUS2(index, xi.interface));
		/* Checking that latched BLOCK_LOCK is still set (Block Lock never lost) */
		if (!br_status2.s.latched_lock) {
			cvmx_dprintf("ERROR: %d:BGX%d:%d: BASE-R PCS block lock lost, need to retry\n", node, xi.interface, index);
			return -1;
		}

		/* If set, clear the LATCHED_BER by writing it to a one.  */
		if (br_status2.s.latched_ber)
			cvmx_write_csr_node(node, CVMX_BGXX_SPUX_BR_STATUS2(index, xi.interface), br_status2.u64);

		/* Complete a BER test. If latched_ber = 1, then BER >= 10e^4 */
		/* The BER detection time window is 125us for 10GBASE-R (1250us for 40G). */
		cvmx_wait_usec(1500);
		br_status2.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_BR_STATUS2(index, xi.interface));
		if (br_status2.s.latched_ber) {
#ifdef DEBUG_BGX
			cvmx_dprintf("ERROR: %d:BGX%d:%d: BER test failed, BER >= 10e^4, need to retry\n", node, xi.interface, index);
#endif
			return -1;
		}
	}

	/* (7) Enable packet transmit and receive */
	spu_misc_control.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_MISC_CONTROL(index, xi.interface));
	spu_misc_control.s.rx_packet_dis = 0;
	cvmx_write_csr_node(node, CVMX_BGXX_SPUX_MISC_CONTROL(index, xi.interface), spu_misc_control.u64);

	if (debug)
		cvmx_dprintf("%s: Enabling tx and rx data packets\n", __func__);
	cmr_config.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface));
	cmr_config.s.data_pkt_tx_en = 1;
	cmr_config.s.data_pkt_rx_en = 1;
	cvmx_write_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface),
			    cmr_config.u64);
	return 0;
}

int __cvmx_helper_bgx_xaui_enable(int xiface)
{
	int index;
	cvmx_helper_interface_mode_t mode;
	int num_ports = cvmx_helper_ports_on_interface(xiface);

	for (index = 0; index < num_ports; index++) {
		int res;
		int xipd_port = cvmx_helper_get_ipd_port(xiface, index);
		int phy_pres;
		struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
		static int count[CVMX_MAX_NODES][CVMX_HELPER_MAX_IFACE][CVMX_HELPER_CFG_MAX_PORT_PER_IFACE] = {
			[0 ... CVMX_MAX_NODES - 1][0 ... CVMX_HELPER_MAX_IFACE - 1] = {
				[0 ... CVMX_HELPER_CFG_MAX_PORT_PER_IFACE - 1] = 0 } };

		mode = cvmx_helper_bgx_get_mode(xiface, index);

		/* Set disparity for RXAUI interface as described in the
		   Marvell RXAUI Interface specification. */
		if (mode == CVMX_HELPER_INTERFACE_MODE_RXAUI &&
				  (cvmx_helper_get_port_phy_present(xiface, index)))
			phy_pres = 1;
		else
			phy_pres = 0;
		__cvmx_helper_bgx_port_init(xipd_port, phy_pres);

retry_link:
		res = __cvmx_helper_bgx_xaui_link_init(index, xiface);
		/* RX Equalization or autonegotiation can take little longer
		   retry the link maybe 5 times for now */
		if ((res == -1) && (count[xi.node][xi.interface][index] < 5)) {
			count[xi.node][xi.interface][index]++;
#ifdef DEBUG_BGX
			cvmx_dprintf("%d:BGX(%d,%d): Failed to get link, retrying\n", xi.node, xi.interface, index);
#endif
			goto retry_link;
		}

		if (res == -1) {
#ifdef DEBUG_BGX
			cvmx_dprintf("%d:BGX(%d,%d): Failed to get link\n", xi.node, xi.interface, index);
#endif
			continue;
		}
	}
	return 0;
}

cvmx_helper_link_info_t __cvmx_helper_bgx_xaui_link_get(int xipd_port)
{
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(xipd_port);
	int index = cvmx_helper_get_interface_index_num(xp.port);
	cvmx_bgxx_spux_status1_t spu_status1;
	cvmx_bgxx_smux_tx_ctl_t smu_tx_ctl;
	cvmx_bgxx_smux_rx_ctl_t smu_rx_ctl;
	cvmx_bgxx_cmrx_config_t cmr_config;
	cvmx_helper_link_info_t result;
	cvmx_helper_interface_mode_t mode;

	result.u64 = 0;

	if (debug)
		cvmx_dprintf("%s: interface %u:%d/%d\n",
		__func__, xi.node, xi.interface, index);

	mode = cvmx_helper_bgx_get_mode(xiface, index);
	if (mode == CVMX_HELPER_INTERFACE_MODE_RGMII)
		return __cvmx_helper_bgx_sgmii_link_get(xipd_port);

	spu_status1.u64 = cvmx_read_csr_node(xi.node, CVMX_BGXX_SPUX_STATUS1(index, xi.interface));
	smu_tx_ctl.u64 = cvmx_read_csr_node(xi.node, CVMX_BGXX_SMUX_TX_CTL(index, xi.interface));
	smu_rx_ctl.u64 = cvmx_read_csr_node(xi.node, CVMX_BGXX_SMUX_RX_CTL(index, xi.interface));

	if ((smu_tx_ctl.s.ls == 0)     &&
	    (smu_rx_ctl.s.status == 0) &&
	    (spu_status1.s.rcv_lnk)) {
		int lanes;
		int qlm = cvmx_qlm_lmac(xiface, index);
		uint64_t speed;
		result.s.link_up = 1;
		result.s.full_duplex = 1;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			speed = cvmx_qlm_get_gbaud_mhz_node(xi.node, qlm);
		else
			speed = cvmx_qlm_get_gbaud_mhz(qlm);

		cmr_config.u64 = cvmx_read_csr_node(xi.node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface));
		switch(cmr_config.s.lmac_type) {
		default:
		case 1:  // XAUI
			speed = (speed * 8 + 5) / 10;
			lanes = 4;
			break;
		case 2:  // RXAUI
			speed = (speed * 8 + 5) / 10;
			lanes = 2;
			break;
		case 3:  // XFI
			speed = (speed * 64 + 33) / 66;
			lanes = 1;
			break;
		case 4:  // XLAUI
			/* Adjust the speed when XLAUI is configured at 6.250Gbps */
			if (speed == 6250)
				speed = 6445;
			speed = (speed * 64 + 33) / 66;
			lanes = 4;
			break;
		}

		if (debug)
			cvmx_dprintf("%s: baud: %llu, lanes: %d\n", __func__,
				     (unsigned long long)speed, lanes);
		speed *= lanes;
		result.s.speed = speed;
	} else {
		int res;
		res = __cvmx_helper_bgx_xaui_link_init(index, xiface);
		if (res == -1) {
#ifdef DEBUG_BGX
			cvmx_dprintf("Failed to get %d:BGX(%d,%d) link\n", xi.node, xi.interface, index);
#endif
			return result;
		}
	}

	return result;
}

int __cvmx_helper_bgx_xaui_link_set(int xipd_port, cvmx_helper_link_info_t link_info)
{
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(xipd_port);
	int node = xi.node;
	int index = cvmx_helper_get_interface_index_num(xp.port);
	cvmx_bgxx_smux_tx_ctl_t smu_tx_ctl;
	cvmx_bgxx_smux_rx_ctl_t smu_rx_ctl;
	cvmx_bgxx_spux_status1_t spu_status1;
	cvmx_helper_interface_mode_t mode;

	if (debug)
		cvmx_dprintf("%s: interface %u:%d/%d\n",
		__func__, xi.node, xi.interface, index);

	mode = cvmx_helper_bgx_get_mode(xiface, index);
	if (mode == CVMX_HELPER_INTERFACE_MODE_RGMII)
		return __cvmx_helper_bgx_sgmii_link_set(xipd_port, link_info);

	smu_tx_ctl.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SMUX_TX_CTL(index, xi.interface));
	smu_rx_ctl.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SMUX_RX_CTL(index, xi.interface));
	spu_status1.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_STATUS1(index, xi.interface));

	/* If the link shouldn't be up, then just return */
	if (!link_info.s.link_up)
		return 0;

	/* Do nothing if both RX and TX are happy */
	if ((smu_tx_ctl.s.ls == 0) && (smu_rx_ctl.s.status == 0) && spu_status1.s.rcv_lnk)
		return 0;

	/* Bring the link up */
	return __cvmx_helper_bgx_xaui_link_init(index, xiface);
}

int __cvmx_helper_bgx_xaui_configure_loopback(int xipd_port,
						     int enable_internal,
						     int enable_external)
{
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	struct cvmx_xport xp = cvmx_helper_ipd_port_to_xport(xipd_port);
	int node = xi.node;
	int index = cvmx_helper_get_interface_index_num(xp.port);
	cvmx_bgxx_spux_control1_t spu_control1;
	cvmx_bgxx_smux_ext_loopback_t smu_ext_loopback;

	if (debug)
		cvmx_dprintf("%s: interface %u:%d/%d\n",
		__func__, xi.node, xi.interface, index);

	/* INT_BEAT_GEN must be set for loopback if the QLMs are not clocked. Set
	   it whenever we use internal loopback */
	if (enable_internal) {
		cvmx_bgxx_cmrx_config_t cmr_config;
		cmr_config.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface));
		cmr_config.s.int_beat_gen = 1;
		cvmx_write_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface), cmr_config.u64);
	}
	/* Set the internal loop */
	spu_control1.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SPUX_CONTROL1(index, xi.interface));
	spu_control1.s.loopbck = enable_internal;
	cvmx_write_csr_node(node, CVMX_BGXX_SPUX_CONTROL1(index, xi.interface), spu_control1.u64);
	/* Set the external loop */
	smu_ext_loopback.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SMUX_EXT_LOOPBACK(index, xi.interface));
	smu_ext_loopback.s.en = enable_external;
	cvmx_write_csr_node(node, CVMX_BGXX_SMUX_EXT_LOOPBACK(index, xi.interface), smu_ext_loopback.u64);

	return __cvmx_helper_bgx_xaui_link_init(index, xiface);
}


int __cvmx_helper_bgx_mixed_enable(int xiface)
{
	int index;
	int num_ports = cvmx_helper_ports_on_interface(xiface);
	cvmx_helper_interface_mode_t mode;

	for (index = 0; index < num_ports; index++) {
		int xipd_port, phy_pres = 0;

		if (!cvmx_helper_is_port_valid(xiface, index))
			continue;

		mode = cvmx_helper_bgx_get_mode(xiface, index);

		xipd_port = cvmx_helper_get_ipd_port(xiface, index);

		if (mode == CVMX_HELPER_INTERFACE_MODE_RXAUI
		    && (cvmx_helper_get_port_phy_present(xiface, index)))
			phy_pres = 1;

		if (__cvmx_helper_bgx_port_init(xipd_port, phy_pres))
			continue;

		/* For RGMII interface, initialize the link after PKO is setup */
		if (mode == CVMX_HELPER_INTERFACE_MODE_RGMII)
			continue;
		/* Call SGMII init code for lmac_type = 0|5 */
		else if (mode == CVMX_HELPER_INTERFACE_MODE_SGMII) {
			int do_link_set = 1;
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
			if (!(cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_SIM))
				do_link_set = 0;
#endif

			if (do_link_set)
				__cvmx_helper_bgx_sgmii_link_set(xipd_port,
					__cvmx_helper_bgx_sgmii_link_get(xipd_port));
		/* All other lmac type call XAUI init code */
		} else {
			int res;
			struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
			static int count[CVMX_MAX_NODES][CVMX_HELPER_MAX_IFACE][CVMX_HELPER_CFG_MAX_PORT_PER_IFACE] = {
				[0 ... CVMX_MAX_NODES - 1][0 ... CVMX_HELPER_MAX_IFACE - 1] = {
					[0 ... CVMX_HELPER_CFG_MAX_PORT_PER_IFACE - 1] = 0 } };

retry_link:
			res = __cvmx_helper_bgx_xaui_link_init(index, xiface);
			/* RX Equalization or autonegotiation can take little longer
			   retry the link maybe 5 times for now */
			if ((res == -1) && (count[xi.node][xi.interface][index] < 5)) {
				count[xi.node][xi.interface][index]++;
				goto retry_link;
			}

			if (res == -1) {
#ifdef DEBUG_BGX
				cvmx_dprintf("Failed to get %d:BGX(%d,%d) link\n", xi.node, xi.interface, index);
#endif
				continue;
			}
		}
	}
	return 0;
}

cvmx_helper_link_info_t __cvmx_helper_bgx_mixed_link_get(int xipd_port)
{
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	int index = cvmx_helper_get_interface_index_num(xipd_port);
	cvmx_helper_interface_mode_t mode;

	mode = cvmx_helper_bgx_get_mode(xiface, index);
	if (mode == CVMX_HELPER_INTERFACE_MODE_SGMII
	    || mode == CVMX_HELPER_INTERFACE_MODE_RGMII)
		return __cvmx_helper_bgx_sgmii_link_get(xipd_port);
	else
		return __cvmx_helper_bgx_xaui_link_get(xipd_port);
}

int __cvmx_helper_bgx_mixed_link_set(int xipd_port, cvmx_helper_link_info_t link_info)
{
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	int index = cvmx_helper_get_interface_index_num(xipd_port);
	cvmx_helper_interface_mode_t mode;

	mode = cvmx_helper_bgx_get_mode(xiface, index);
	if (mode == CVMX_HELPER_INTERFACE_MODE_SGMII
	    || mode == CVMX_HELPER_INTERFACE_MODE_RGMII)
		return __cvmx_helper_bgx_sgmii_link_set(xipd_port, link_info);
	else
		return __cvmx_helper_bgx_xaui_link_set(xipd_port, link_info);
}

int __cvmx_helper_bgx_mixed_configure_loopback(int xipd_port,
						     int enable_internal,
						     int enable_external)
{
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	int index = cvmx_helper_get_interface_index_num(xipd_port);
	cvmx_helper_interface_mode_t mode;

	mode = cvmx_helper_bgx_get_mode(xiface, index);
	if (mode == CVMX_HELPER_INTERFACE_MODE_SGMII
	    || mode == CVMX_HELPER_INTERFACE_MODE_RGMII)
		return __cvmx_helper_bgx_sgmii_configure_loopback(xipd_port,
						enable_internal, enable_external);
	else
		return __cvmx_helper_bgx_xaui_configure_loopback(xipd_port,
						enable_internal, enable_external);
}

/**
 * @INTERNAL
 * Configure Priority-Based Flow Control (a.k.a. PFC/CBFC)
 * on a specific BGX interface/port.
 */
void __cvmx_helper_bgx_xaui_config_pfc(unsigned node,
		unsigned interface, unsigned index, bool pfc_enable)
{
	int xiface = cvmx_helper_node_interface_to_xiface(node, interface);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	cvmx_bgxx_smux_cbfc_ctl_t cbfc_ctl;

	if (debug)
		cvmx_dprintf("%s: interface %u:%d/%d\n",
		__func__, xi.node, xi.interface, index);

	cbfc_ctl.u64 = cvmx_read_csr_node(node,
		CVMX_BGXX_SMUX_CBFC_CTL(index, xi.interface)
		);

	/* Enable all PFC controls if requiested */
	cbfc_ctl.s.rx_en = pfc_enable;
	cbfc_ctl.s.tx_en = pfc_enable;
#if 0
	cbfc_ctl.s.bck_en = 1;
	cbfc_ctl.s.phys_en = 0xff;
	cbfc_ctl.s.logl_en = 0xff;
	cbfc_ctl.s.drp_en = pfc_enable;
#endif
	if (debug)
		cvmx_dprintf("%s: CVMX_BGXX_SMUX_CBFC_CTL(%d,%d)=%#llx\n",
			__func__, index, xi.interface, (unsigned long long)cbfc_ctl.u64);
	cvmx_write_csr_node(node,
		CVMX_BGXX_SMUX_CBFC_CTL(index, xi.interface),
		cbfc_ctl.u64);
}


/**
 * This function control how the hardware handles incoming PAUSE
 * packets. The most common modes of operation:
 * ctl_bck = 1, ctl_drp = 1: hardware handles everything
 * ctl_bck = 0, ctl_drp = 0: software sees all PAUSE frames
 * ctl_bck = 0, ctl_drp = 1: all PAUSE frames are completely ignored
 * @param node		node number.
 * @param interface	interface number
 * @param index		port number
 * @param ctl_bck	1: Forward PAUSE information to TX block
 * @param ctl_drp	1: Drop control PAUSE frames.
 */
void cvmx_helper_bgx_rx_pause_ctl(unsigned node, unsigned interface,
			unsigned index, unsigned ctl_bck, unsigned ctl_drp)
{
	int xiface = cvmx_helper_node_interface_to_xiface(node, interface);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	cvmx_bgxx_smux_rx_frm_ctl_t frm_ctl;

	if (debug)
		cvmx_dprintf("%s: interface %u:%d/%d\n",
		__func__, xi.node, xi.interface, index);

	frm_ctl.u64 = cvmx_read_csr_node(node, CVMX_BGXX_SMUX_RX_FRM_CTL(index, xi.interface));
	frm_ctl.s.ctl_bck = ctl_bck;
	frm_ctl.s.ctl_drp = ctl_drp;
	cvmx_write_csr_node(node, CVMX_BGXX_SMUX_RX_FRM_CTL(index, xi.interface), frm_ctl.u64);
}

/**
 * This function configures the receive action taken for multicast, broadcast
 * and dmac filter match packets.
 * @param node		node number.
 * @param interface	interface number
 * @param index		port number
 * @param cam_accept	0: reject packets on dmac filter match
 *                      1: accept packet on dmac filter match
 * @param mcast_mode	0x0 = Force reject all multicast packets
 *                      0x1 = Force accept all multicast packets
 *                      0x2 = Use the address filter CAM
 * @param bcast_accept  0 = Reject all broadcast packets
 *                      1 = Accept all broadcast packets
 */
void cvmx_helper_bgx_rx_adr_ctl(unsigned node, unsigned interface, unsigned index,
                                 unsigned cam_accept, unsigned mcast_mode, unsigned bcast_accept)
{
	int xiface = cvmx_helper_node_interface_to_xiface(node, interface);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
        cvmx_bgxx_cmrx_rx_adr_ctl_t adr_ctl;

	if (debug)
		cvmx_dprintf("%s: interface %u:%d/%d\n",
		__func__, xi.node, xi.interface, index);

        adr_ctl.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMRX_RX_ADR_CTL(index, xi.interface));
        adr_ctl.s.cam_accept = cam_accept;
        adr_ctl.s.mcst_mode = mcast_mode;
        adr_ctl.s.bcst_accept = bcast_accept;

        cvmx_write_csr_node(node, CVMX_BGXX_CMRX_RX_ADR_CTL(index, xi.interface), adr_ctl.u64);
}

/**
 * Function to control the generation of FCS, padding by the BGX
 *
 */
void cvmx_helper_bgx_tx_options(unsigned node,
	unsigned interface, unsigned index,
	bool fcs_enable, bool pad_enable)
{
	cvmx_bgxx_cmrx_config_t cmr_config;
	cvmx_bgxx_gmp_gmi_txx_append_t gmp_txx_append;
	cvmx_bgxx_gmp_gmi_txx_min_pkt_t gmp_min_pkt;
	cvmx_bgxx_smux_tx_min_pkt_t smu_min_pkt;
	cvmx_bgxx_smux_tx_append_t  smu_tx_append;
	int xiface = cvmx_helper_node_interface_to_xiface(node, interface);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (!cvmx_helper_is_port_valid(xiface, index))
		return;

	if (debug)
		cvmx_dprintf("%s: interface %u:%d/%d, fcs: %s, pad: %s\n",
			     __func__, xi.node, xi.interface, index,
			     fcs_enable ? "true" : "false",
			     pad_enable ? "true" : "false");

	cmr_config.u64 = cvmx_read_csr_node(node,
		CVMX_BGXX_CMRX_CONFIG(index, xi.interface));

	(void) cmr_config;	/* In case we need LMAC_TYPE later */

	/* Setting options for both BGX subsystems, regardless of LMAC type */

	/* Set GMP (SGMII) Tx options */
	gmp_min_pkt.u64 = 0;
	/* per HRM Sec 34.3.4.4 */
	gmp_min_pkt.s.min_size = 59;
	cvmx_write_csr_node(node,
		CVMX_BGXX_GMP_GMI_TXX_MIN_PKT(index, xi.interface),
		gmp_min_pkt.u64);
	gmp_txx_append.u64 = cvmx_read_csr_node(node,
		CVMX_BGXX_GMP_GMI_TXX_APPEND(index, xi.interface));
	gmp_txx_append.s.fcs = fcs_enable;
	gmp_txx_append.s.pad = pad_enable;
	cvmx_write_csr_node(node,
		CVMX_BGXX_GMP_GMI_TXX_APPEND(index, xi.interface),
		gmp_txx_append.u64);

	/* Set SMUX (XAUI/XFI) Tx options */
	/* HRM Sec 33.3.4.3 should read 64 */
	smu_min_pkt.u64 = 0;
	smu_min_pkt.s.min_size = 0x40;
	cvmx_write_csr_node(node,
		CVMX_BGXX_SMUX_TX_MIN_PKT(index, xi.interface),
		smu_min_pkt.u64);
	smu_tx_append.u64 = cvmx_read_csr_node(node,
		CVMX_BGXX_SMUX_TX_APPEND(index, xi.interface));
	smu_tx_append.s.fcs_d = fcs_enable; /* Set data-packet FCS */
	smu_tx_append.s.pad = pad_enable;
	cvmx_write_csr_node(node,
		CVMX_BGXX_SMUX_TX_APPEND(index, xi.interface),
		smu_tx_append.u64);
}

/**
 * Set mac for the ipd_port
 *
 * @param xipd_port ipd_port to set the mac
 * @param bcst      If set, accept all broadcast packets
 * @param mcst      Multicast mode
 * 		    0 = Force reject all multicast packets
 * 		    1 = Force accept all multicast packets
 * 		    2 = use the address filter CAM.
 * @param mac       mac address for the ipd_port, or 0 to disable MAC filtering
 */
void cvmx_helper_bgx_set_mac(int xipd_port, int bcst, int mcst, uint64_t mac)
{
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int node = xi.node;
	int index;
	cvmx_bgxx_cmr_rx_adrx_cam_t adr_cam;
	cvmx_bgxx_cmrx_rx_adr_ctl_t adr_ctl;
	cvmx_bgxx_cmrx_config_t cmr_config;
	int saved_state_tx, saved_state_rx;

	index = cvmx_helper_get_interface_index_num(xipd_port);

	if (!cvmx_helper_is_port_valid(xiface, index))
		return;

	if (debug)
		cvmx_dprintf("%s: interface %u:%d/%d\n",
		__func__, xi.node, xi.interface, index);

	cmr_config.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface));
	saved_state_tx = cmr_config.s.data_pkt_tx_en;
	saved_state_rx = cmr_config.s.data_pkt_rx_en;
	cmr_config.s.data_pkt_tx_en = 0;
	cmr_config.s.data_pkt_rx_en = 0;
	cvmx_write_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface), cmr_config.u64);

	/* Set the mac */
	adr_cam.u64 = 0;
	adr_cam.s.id = index;

	if (mac != 0ull)
		adr_cam.s.en = 1;
	adr_cam.s.adr = mac;

	cvmx_write_csr_node(node, CVMX_BGXX_CMR_RX_ADRX_CAM(index * 8, xi.interface), adr_cam.u64);

	adr_ctl.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMRX_RX_ADR_CTL(index, xi.interface));
	if (mac != 0ull)
		adr_ctl.s.cam_accept = 1;  /* Accept the packet on DMAC CAM address */
	else
		adr_ctl.s.cam_accept = 0;  /* No filtering, promiscous */

	adr_ctl.s.mcst_mode = mcst;   /* Use the address filter CAM */
	adr_ctl.s.bcst_accept = bcst; /* Accept all broadcast packets */
	cvmx_write_csr_node(node, CVMX_BGXX_CMRX_RX_ADR_CTL(index, xi.interface), adr_ctl.u64);
	/* Set SMAC for PAUSE frames */
	cvmx_write_csr_node(node, CVMX_BGXX_GMP_GMI_SMACX(index, xi.interface), mac);

	/* Restore back the interface state */
	cmr_config.s.data_pkt_tx_en = saved_state_tx;
	cmr_config.s.data_pkt_rx_en = saved_state_rx;
	cvmx_write_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface), cmr_config.u64);

	/* Wait 100ms after bringing up the link to give the PHY some time */
	if (cmr_config.s.enable) {
		cvmx_helper_interface_mode_t mode;
		mode = cvmx_helper_bgx_get_mode(xiface, index);
		__cvmx_helper_bgx_interface_enable_delay(mode);
	}
}

/**
 * Disables the sending of flow control (pause) frames on the specified
 * BGX port(s).
 *
 * @param xiface Which xiface
 * @param port_mask Mask (4bits) of which ports on the interface to disable
 *                  backpressure on.
 *                  1 => disable backpressure
 *                  0 => enable backpressure
 *
 * @return 0 on success
 *         -1 on error
 *
 * FIXME: Should change the API to handle a single port in every
 * invokation, for consistency with other API calls.
 */
int cvmx_bgx_set_backpressure_override(int xiface, unsigned port_mask)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	cvmx_bgxx_cmr_rx_ovr_bp_t rx_ovr_bp;
	int node = xi.node;

	if (xi.interface >= CVMX_HELPER_MAX_GMX)
		return 0;

	if (debug)
		cvmx_dprintf("%s: interface %u:%d port_mask=%#x\n",
			__func__, xi.node, xi.interface, port_mask);

	/* Check for valid arguments */
	rx_ovr_bp.u64 = 0;
	rx_ovr_bp.s.en = port_mask;	/* Per port Enable back pressure override */
	rx_ovr_bp.s.ign_fifo_bp = port_mask;	/* Ignore the RX FIFO full when computing BP */

	cvmx_write_csr_node(node, CVMX_BGXX_CMR_RX_OVR_BP(xi.interface), rx_ovr_bp.u64);
	return 0;
}

/**
 * Set maximum packet size for a BGX port
 *
 */
void cvmx_helper_bgx_set_jabber(int xiface, unsigned index,
	unsigned size)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int node;
	cvmx_helper_interface_mode_t mode;

	if (!octeon_has_feature(OCTEON_FEATURE_BGX))
		return;

	if (!cvmx_helper_is_port_valid(xiface, index))
		return;

	node = xi.node;

	mode = cvmx_helper_bgx_get_mode(xiface, index);

	/* Set GMI or SMUX register based on lmac_type */
	if (mode == CVMX_HELPER_INTERFACE_MODE_SGMII
	    || mode == CVMX_HELPER_INTERFACE_MODE_RGMII) {
		cvmx_write_csr_node(node,
				CVMX_BGXX_GMP_GMI_RXX_JABBER(index, xi.interface), size);
	} else {
		cvmx_write_csr_node(node,
			CVMX_BGXX_SMUX_RX_JABBER(index, xi.interface), size);
	}
}

/**
 * Shutdown a BGX port
 *
 */
int cvmx_helper_bgx_shutdown_port(int xiface, int index)
{
	cvmx_bgxx_cmrx_config_t cmr_config;
	int node;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	node = xi.node;

	if (xi.interface >= CVMX_HELPER_MAX_GMX)
		return 0;

	if (debug)
		cvmx_dprintf("%s: interface %u:%d/%d\n",
		__func__, node, xi.interface, index);

	if (!cvmx_helper_is_port_valid(xiface, index))
		return 0;

	/* Disable BGX CMR before we make any changes. */
	cmr_config.u64 = cvmx_read_csr_node(node,
		CVMX_BGXX_CMRX_CONFIG(index, xi.interface));

	cmr_config.s.data_pkt_tx_en = 0;
	cmr_config.s.data_pkt_rx_en = 0;
	cvmx_write_csr_node(node, CVMX_BGXX_CMRX_CONFIG(index, xi.interface),
		cmr_config.u64);

	/* Clear pending common interrupts */
	cvmx_write_csr_node(node, CVMX_BGXX_CMRX_INT(index, xi.interface), 0x7);

	if (cmr_config.s.lmac_type == 0
	    || cmr_config.s.lmac_type == 5) {	/* SGMII */
		/* Clear GMP interrupts */
		cvmx_write_csr_node(node,
			CVMX_BGXX_GMP_GMI_RXX_INT(index, xi.interface), 0xfff);
		cvmx_write_csr_node(node,
			CVMX_BGXX_GMP_GMI_TXX_INT(index, xi.interface), 0x1f);
		/* Wait for GMX to be idle */
		if (CVMX_WAIT_FOR_FIELD64_NODE(node,
			CVMX_BGXX_GMP_GMI_PRTX_CFG(index, xi.interface),
			cvmx_bgxx_gmp_gmi_prtx_cfg_t, rx_idle, ==, 1, 10000) ||
		    CVMX_WAIT_FOR_FIELD64_NODE(node,
			CVMX_BGXX_GMP_GMI_PRTX_CFG(index, xi.interface),
			cvmx_bgxx_gmp_gmi_prtx_cfg_t, tx_idle, ==, 1, 10000)) {
				cvmx_printf("ERROR: %s: SGMII: "
				"Timeout waiting for port %u:%d/%d to stop\n",
				__func__, node, xi.interface, index);
				return -1;
			}
		/* Read GMX CFG again to make sure the disable completed */
		cvmx_read_csr_node(node,
			CVMX_BGXX_GMP_GMI_PRTX_CFG(index, xi.interface));
		/* FIXME Disable RGMII interface */
	} else {		/* XAUI/XFI/10-KR */
		/* Clear all pending SMUX interrupts */
		cvmx_write_csr_node(node, CVMX_BGXX_SMUX_RX_INT(index, xi.interface),
			0xfff);
		cvmx_write_csr_node(node, CVMX_BGXX_SMUX_TX_INT(index, xi.interface),
			0x1f);
		cvmx_write_csr_node(node, CVMX_BGXX_SPUX_INT(index, xi.interface),
			0x7fff);

		/* Wait for GMX RX to be idle */
		if (CVMX_WAIT_FOR_FIELD64_NODE(node,
			CVMX_BGXX_SMUX_CTRL(index, xi.interface),
			cvmx_bgxx_smux_ctrl_t, rx_idle, ==, 1, 10000) ||
		    CVMX_WAIT_FOR_FIELD64_NODE(node,
			CVMX_BGXX_SMUX_CTRL(index, xi.interface),
			cvmx_bgxx_smux_ctrl_t, tx_idle, ==, 1, 10000)) {
				cvmx_printf("ERROR: %s: XAUI: "
				"Timeout waiting for port %u:%d/%d to stop\n",
				__func__, node, xi.interface, index);
				return -1;
		}
	}
	return 0;
}

#ifdef CVMX_DUMP_BGX
/**
 * Dump configuration and status of BGX ports
 */
/* The following (high level) funcs are implemented in this section */
int cvmx_dump_bgx_config(unsigned bgx);
int cvmx_dump_bgx_status(unsigned bgx);
int cvmx_dump_bgx_config_node(unsigned node, unsigned bgx);
int cvmx_dump_bgx_status_node(unsigned node, unsigned bgx);

/*
 * The following macros helps to easyly print 'formated table'
 * for up to 4 LMACs of single BGX device (same macros used for GSER)
 * MACROS automaticaly handle data types ('unsigned' or 'const char *') and
 * indexing (by predefined 'ind' string used as index).
 * NOTE that there are different groups of macros:
 *  'PRn' 	means it prints unconditionaly, data are 'unsigned' type
 *  'PRns'	means it prints unconditionaly, data are 'const char *' type
 *  'PRc' 	means it print only if 'cond' argument is 'true'
 *  'PRd' 	means it print only if 'data' argument is != 0
 *  'PRcd' 	means it print only if ('cond'== true && 'data' != 0)
 *  'PRM' 	means it print only when 'mask' bits are != 0, skip otherwise
 * 		NOTE: the loop is 'for (_mask = mask; _mask > 0; _mask >>= 1)'
 * 		which means order is mask_bits 0,1,2,3.. and the last unset bits
 * 		will not be handled at all (i.e. not 'skipped' with '<   ---   >')
 * Other useful combinations of them exists like:
 *  'PRMc' 	means it print only when 'mask' bits are != 0 and 'cond' = true
 *  'PRMcs' 	same as above but data type is 'const char *' instead of 'unsigned'
 * For example one use 'PRMcd' to print 'data' only for 'mask' mapped LMAC fields
 * if 'cond' is met and 'data' != 0 (example: Frame Check 'FSCERR' (check for
 * FCS errors) was enabled with 'cond' and err realy happens (detected) (i.e.
 * 'data' != 0) and only then it will be printed
 * (and only for LMACs mapped with 'mask' bits = 1, other fields skipped)
 * NOTE: Where applicable references to Hardware Manual are available.
 */


/* define FORCE_COND=1 in order to force prints unconditionally (DEBUG, test)
 * define FORCE_COND=0 in order to prints conditionally (NORMAL MODE, skip unimportant)
 * if run time control is needed just define FORCE_COND as local static variable
 * Enable only one of the following '#define FORCED_COND(cond)' macros
 * use it like this: 'if ( FORCED_COND(cond) )'
 */
/* #define FORCE_COND	1	*/
/* #define FORCED_COND(cond)	((cond) || FORCE_COND) */
/* ..or.. the next line to exclude all overhead code */
#define FORCED_COND(cond)	(cond)


#ifndef USE_PRx_MACROS
#define USE_PRx_MACROS

/* Always print - no test of 'data' values */
/* for (1..N) printf(format, data) */
#define PRn(header, N, format, data)			\
do {							\
	unsigned ind;					\
	cvmx_dprintf("%-48s", header);			\
	for (ind = 0; ind < N; ind++)			\
		cvmx_dprintf(format, data);		\
	cvmx_dprintf("\n");				\
} while(0);

#define PRns	PRn

/* Always print - no test, skip data for mask[x]=0 */
#define PRMn(header, mask, format, data)				\
do {									\
	unsigned ind, _mask;						\
	cvmx_dprintf("%-48s", header);					\
	for (_mask = mask, ind = 0; _mask > 0; _mask >>= 1, ind++)	\
		if (_mask & 1)						\
			cvmx_dprintf(format, data);			\
		else							\
			cvmx_dprintf("%15s","");			\
	cvmx_dprintf("\n");						\
} while(0);

#define PRMns PRMn

/* mask + data != 0 */
#define PRMd(header, mask, format, data)					\
do {										\
	unsigned cnt, ind, _mask = mask;					\
	for (cnt = 0, ind = 0; _mask > 0; _mask >>= 1, ind++)			\
		if (data != 0)							\
			cnt++;							\
	if (FORCED_COND(cnt)) { /* at least one item != 0 =>print */		\
		cvmx_dprintf("%-48s", header);					\
		for (_mask = mask, ind = 0; _mask > 0; _mask >>= 1, ind++)	\
			if (FORCED_COND((_mask & 1) && data!= 0))		\
				cvmx_dprintf(format, data);			\
			else							\
				cvmx_dprintf("%15s","");			\
		cvmx_dprintf("\n");						\
	}									\
} while(0);

/* mask + cond != 0 */
#define PRMc(header, mask, cond, format, data)					\
do {										\
	unsigned cnt, ind, _mask = mask;					\
	for (cnt = 0, ind = 0; _mask > 0; _mask >>= 1, ind++)			\
		if (cond != 0)							\
			cnt++;							\
	if (FORCED_COND(cnt)) { /* at least one item != 0 =>print */		\
		cvmx_dprintf("%-48s", header);					\
		for (_mask = mask, ind = 0; _mask > 0; _mask >>= 1, ind++)	\
			if (FORCED_COND((_mask & 1) && cond))			\
				cvmx_dprintf(format, data);			\
			else							\
				cvmx_dprintf("%15s","");			\
		cvmx_dprintf("\n");						\
	}									\
} while(0);

#define PRMcs	PRMc

/* for (mask[i]==1) if (cond && data) printf(format, data) */
#define PRMcd(header, mask, cond, format, data)					\
do {										\
	unsigned cnt, ind, _mask = mask;					\
	for (cnt = 0, ind = 0; _mask > 0; _mask >>= 1, ind++)			\
		if (cond && data != 0)						\
			cnt++;							\
	if (FORCED_COND(cnt)) { /* at least one item != 0 =>print */		\
		cvmx_dprintf("%-48s", header);					\
		for (_mask = mask, ind = 0; _mask > 0; _mask >>= 1, ind++)	\
		if ( FORCED_COND(cond && (data!=0) && (_mask & 1)) )		\
			cvmx_dprintf(format, data);				\
		else								\
			cvmx_dprintf("%15s","");				\
		cvmx_dprintf("\n");						\
	}									\
} while(0);

/* Test 'data' int values and print only if data != 0 */
/* for (1..N) if (data != 0) printf(format, data) */
#define PRd(header, N, format, data)					\
do {									\
	unsigned ind, cnt;						\
	for (cnt = 0, ind = 0; ind < N; ind++)				\
		if (data != 0)						\
			cnt++;						\
	if (FORCED_COND(cnt)) { /* at least one item != 0 =>print */	\
		cvmx_dprintf("%-48s", header);				\
		for (ind = 0; ind < N; ind++)				\
			cvmx_dprintf(format, data);			\
		cvmx_dprintf("\n");					\
	}								\
} while(0);

/* for (1..N) if (cond) printf(format, data) */
#define PRc(header, N, cond, format, data)				\
do {									\
	unsigned ind, cnt;						\
	for (cnt = 0, ind = 0; ind < N; ind++)				\
		if (cond != 0)						\
			cnt++;						\
	if (FORCED_COND(cnt)) { /* at least one item != 0 =>print */	\
		cvmx_dprintf("%-48s", header);				\
		for (ind = 0; ind < N; ind++)				\
		if ( FORCED_COND(cond) )				\
			cvmx_dprintf(format, data);			\
		else							\
			cvmx_dprintf("%15s","");			\
		cvmx_dprintf("\n");					\
	}								\
} while(0);

#define PRcs PRc

/* for (1..N) if (cond && data) printf(format, data) */
#define PRcd(header, N, cond, format, data)				\
do {									\
	unsigned ind, cnt;						\
	for (cnt = 0, ind = 0; ind < N; ind++)				\
		if (cond && data != 0)					\
			cnt++;						\
	if (FORCED_COND(cnt)) { /* at least one item != 0 =>print */	\
		cvmx_dprintf("%-48s", header);				\
		for (ind = 0; ind < N; ind++)				\
		if ( FORCED_COND(cond && data) )			\
			cvmx_dprintf(format, data);			\
		else							\
			cvmx_dprintf("%15s","");			\
		cvmx_dprintf("\n");					\
	}								\
} while(0);

#endif

/*===================== common helping funcs =====================*/
/* NOTE: Even many of these functions gets as arguments numer of used LMACs
 * when needed they create an apropriate 'mask' and skip unrelated LMAC fields
 * (for example if a functions is GMP specific it will handle only LMACs with
 * SGMII or 1000BASE-X type and skip XFI LMAC)
 */
/* The following funcs are implemented in this section */
const char *get_lmac_type_name(uint8_t lmac_type, uint8_t pcs_misc_ctl_mode);
int get_num_pcs_lanes(uint8_t lmac_type);
const char *get_bind_lanes_per_lmac(uint8_t ind, uint8_t lmac_type, uint8_t lane_to_sds);
int cvmx_helper_bgx_link_status(int node, int bgx, unsigned N);

/* return name of interface type ("SGMII", "XAUI", etc. */
const char *get_lmac_type_name(uint8_t lmac_type, uint8_t pcs_misc_ctl_mode)
{
	static char *lmac_types[] = {
		"  SGMII   ",
		"XAUI/DXAUI",
		"  RXAUI   ",
		"  10G_R   ",
		"  40G_R   ",
		"  RGMII   "	/* only 73xx has that */
	};

	if ( (OCTEON_IS_MODEL(OCTEON_CN73XX) && lmac_type > 5) ||
		(!OCTEON_IS_MODEL(OCTEON_CN78XX) && lmac_type > 4) )
		return "?????";
	if (lmac_type == 0 && pcs_misc_ctl_mode == 1)
		return "1000BASE-X";
	return lmac_types[lmac_type];
}

/* return number of lanes per interface type (SGMII, XAUI, etc. */
int get_num_pcs_lanes(uint8_t lmac_type)
{
	static int num_pcs_lanes[] = {
		1/*SGMII*/, 4/*XAUI*/, 2/*RXAUI*/, 1/*10G_R*/, 4/*40G_R*/
	};

	if (lmac_type > 4)
		return 0;	/* i.e 73xx lmac=5 (RGMII) or out of range */
	return num_pcs_lanes[lmac_type];
}

/* return SerDes lanes connected to lmac - string (up to 7 chars '0,1,2,3')
 * or up to 9 chars "RGMII/XCV" for RGMII(lmac_type=5)
 */
const char *get_bind_lanes_per_lmac(uint8_t ind, uint8_t lmac_type,
				     uint8_t lane_to_sds)
{
	static char tmp[4][9], bind_lanes_per_lmac[4][9];
	uint8_t i, n;

	if (lmac_type > 5 || ind > 3)
		return "  ???  ";
	if (ind != 0 && lmac_type == 5/*RGMII*/)
		return "ERR:RGMII";	/* Only LMAC0 can connect to RGMII/XCV */
	if (ind == 0 && lmac_type == 5/*RGMII*/)
		return "RGMII/XCV";	/* Only LMAC0 can connect to RGMII/XCV */
	bind_lanes_per_lmac[ind][0] = 0;
	for (i = 0; i < get_num_pcs_lanes(lmac_type); i++) {
		sprintf(tmp[ind], ",%1d", (lane_to_sds >> (/*2*i*/i<<1)) & 3);
		strcat( bind_lanes_per_lmac[ind], tmp[ind]);
	}
	/* max returned strlen("0,1,2,3")=7; let's center the string "nnn%snnn" */
	/* &bind_lanes_per_lmac[1]; - skip the first ',' */
	n = (9 - strlen(&bind_lanes_per_lmac[ind][1])) / 2;
	sprintf(tmp[ind], "%*s%s%*s", n, "", &bind_lanes_per_lmac[ind][1], n, "");

	/* 'ind' make sure we store results in different places and return
	 * different pointers - otherwise, if we store on same location and
	 * return the same pointer always (like return tmp;), ONLY the
	 * last result will be available. because it overwritten previous values
	 */
	return tmp[ind];
}

int cvmx_helper_bgx_number_rx_tx_lmacs(unsigned node, unsigned bgx, unsigned *pN)
{
	int N;
	cvmx_bgxx_cmr_rx_lmacs_t rx_lmacs;
	cvmx_bgxx_cmr_tx_lmacs_t tx_lmacs;

	*pN = 1; /* default return - most likely will be overwritten */

	rx_lmacs.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMR_RX_LMACS(bgx));
	tx_lmacs.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMR_TX_LMACS(bgx));
	cvmx_dprintf("NODE%d: BGX%d RX_LMACs = %d\n",
		     node, bgx, rx_lmacs.s.lmacs);
	cvmx_dprintf("NODE%d: BGX%d TX_LMACs = %d\n",
		     node, bgx, tx_lmacs.s.lmacs);
	if (rx_lmacs.s.lmacs == tx_lmacs.s.lmacs ) {
		*pN = rx_lmacs.s.lmacs;
		return 0;
	} else { /* This can happen only if BGX_config is wrong, so report it */
		N = __cvmx_helper_bgx_enumerate(
			cvmx_helper_node_interface_to_xiface(node, bgx) );
		cvmx_dprintf("<<<WARNING>>>: RX_LMACS != TX_LMACS;"
			" return  __cvmx_helper_bgx_enumerate(xiface)=%d\n", N);
		if (N >= 0)
			*pN = N;
		return -1;
	}
}

/* CN73xx-HM-0.95E 32.7 RGMII: XCV Registers */

int cvmx_helper_bgx_rgmii_config(int node, int bgx, unsigned N)
{
	cvmx_xcv_reset_t		xcv_reset;
	cvmx_xcv_dll_ctl_t		xcv_dll_ctl;
	cvmx_xcv_comp_ctl_t		xcv_comp_ctl;
	cvmx_xcv_ctl_t			xcv_ctl;

	xcv_reset.u64 = cvmx_read_csr_node(node, CVMX_XCV_RESET);
	xcv_dll_ctl.u64 = cvmx_read_csr_node(node, CVMX_XCV_DLL_CTL);
	xcv_comp_ctl.u64 = cvmx_read_csr_node(node, CVMX_XCV_COMP_CTL);
	xcv_ctl.u64 = cvmx_read_csr_node(node, CVMX_XCV_CTL);

	cvmx_dprintf("/*=== NODE%d: BGX%d RGMII config ===*/\n", node, bgx);

	PRns("RGMII: Port Enable(enable)", 1, "   %8s    ",
		xcv_reset.s.enable ? " Enabled" : "Disabled");
	PRn("RGMII: DLL CLK reset(clkrst)", 1, "       %1d       ",
		xcv_reset.s.clkrst);
	PRn("RGMII: DLL reset(dllrst)", 1, "       %1d       ",
		xcv_reset.s.dllrst);
	PRns("RGMII: (Drive strenght)Compensation enable(comp)", 1, "   %8s    ",
		xcv_reset.s.comp ? " Enabled" : "Disabled");
	PRn("RGMII: Packet reset for TX(tx_pkt_rst_n)", 1, "       %1d       ",
		xcv_reset.s.tx_pkt_rst_n);
	PRn("RGMII: Datapath reset for TX(tx_dat_rst_n)", 1, "       %1d       ",
		xcv_reset.s.tx_dat_rst_n);
	PRn("RGMII: Packet reset for Rx(rx_pkt_rst_n)", 1, "       %1d       ",
		xcv_reset.s.rx_pkt_rst_n);
	PRn("RGMII: Datapath reset for RX(rx_dat_rst_n)", 1, "       %1d       ",
		xcv_reset.s.rx_dat_rst_n);

	PRn("RGMII: The clock delay measured by DLL(clk_set)",
		1, "      %3d      ", xcv_dll_ctl.s.clk_set);
	PRn("RGMII: Bypass the RX clock delay(clkrx_byp)",
		1, "       %1d       ", xcv_dll_ctl.s.clkrx_byp);
	PRn("RGMII: RX clk delay when bypass mode(clkrx_set)",
		1, "      %3d      ", xcv_dll_ctl.s.clkrx_set);
	PRn("RGMII: Bypass the TX clock delay(clktx_byp)",
		1, "       %1d       ", xcv_dll_ctl.s.clktx_byp);
	PRn("RGMII: TX clk delay when bypass mode(clktx_set)",
		1, "      %3d      ", xcv_dll_ctl.s.clktx_set);
	PRns("RGMII: Reference clock to use(refclk_sel)", 1, "  %10s   ",
		xcv_dll_ctl.s.refclk_sel == 0 ? "   RGMII   " :
		xcv_dll_ctl.s.refclk_sel == 1 ? " RGMII RXC " :
		xcv_dll_ctl.s.refclk_sel == 2 ? " CopClk/N  " : " Reserved ");

	PRn("RGMII: Bypass Comp.(use DRV_[P,N]CTL)(drv_byp)", 1,
		"       %1d       ", xcv_comp_ctl.s.drv_byp);
	PRn("RGMII: Contr. PCTL drive strength(cmp_pctl)", 1,
		"       %2d      ", xcv_comp_ctl.s.cmp_pctl);
	PRn("RGMII: Contr. NCTL drive strength(cmp_nctl)", 1,
		"       %2d      ", xcv_comp_ctl.s.cmp_nctl);
	PRn("RGMII: Bypass PCTL drive strength(drv_pctl)", 1,
		"       %2d      ", xcv_comp_ctl.s.drv_pctl);
	PRn("RGMII: Bypass NCTL drive strength(drv_nctl)", 1,
		"       %2d      ", xcv_comp_ctl.s.drv_nctl);
	PRn("RGMII: PCTL Lock(pctl_lock)", 1,
		"       %1d       ", xcv_comp_ctl.s.pctl_lock);
	PRn("RGMII: PCTL Saturate(pctl_sat)", 1,
		"       %1d       ", xcv_comp_ctl.s.pctl_sat);
	PRn("RGMII: NCTL Lock(nctl_lock)", 1,
		"       %1d       ", xcv_comp_ctl.s.nctl_lock);
	PRn("RGMII: NCTL Saturate(nctl_sat)", 1,
		"       %1d       ", xcv_comp_ctl.s.nctl_sat);

	PRns("RGMII:XCV: External Loopback Enabled(lpbk_ext)", 1, "   %8s    ",
		xcv_ctl.s.lpbk_ext ? " Enabled" : "Disabled");
	PRns("RGMII:XCV: Internal Loopback Enabled(lpbk_int)", 1, "   %8s    ",
		xcv_ctl.s.lpbk_int ? " Enabled" : "Disabled");
	PRns("RGMII:XCV: Speed (speed)", 1, "   %8s    ",
		xcv_ctl.s.speed == 0 ? " 10 Mbps " :
		xcv_ctl.s.speed == 1 ? " 100 Mbps" :
		xcv_ctl.s.speed == 2 ? " 1 Gbps  " : "Reserved");
	return 0;
}

int cvmx_helper_bgx_rgmii_status(int node, int bgx, unsigned N)
{
	cvmx_xcv_int_t			xcv_int;
	cvmx_xcv_inbnd_status_t		xcv_inbnd_status;

	xcv_int.u64 = cvmx_read_csr_node(node, CVMX_XCV_INT);
	xcv_inbnd_status.u64 = cvmx_read_csr_node(node, CVMX_XCV_INBND_STATUS);

	cvmx_dprintf("/*=== NODE%d: BGX%d RGMII status ===*/\n", node, bgx);

	PRd("RGMII: TX FIFO overflow(tx_ovrflw)", 1,
		"       %1d       ", xcv_int.s.tx_ovrflw);
	PRd("RGMII: TX FIFO underflow(tx_undflw)", 1,
		"       %1d       ", xcv_int.s.tx_undflw);
	PRd("RGMII: Incomplete byte -10/100 mode(incomp_byte)", 1,
		"       %1d       ", xcv_int.s.incomp_byte);
	PRd("RGMII: Inband status change on link duplex", 1,
		"       %1d       ", xcv_int.s.duplex);
	PRd("RGMII: Inband status change on link duplex", 1,
		"       %1d       ", xcv_int.s.duplex);
	PRd("RGMII: Inband status change on link up/down", 1,
		"       %1d       ", xcv_int.s.speed);

	PRns("RGMII: RGMII inband status (duplex)", 1, "  %10s   ",
		xcv_inbnd_status.s.duplex ? "Full-duplex" : "Half-duplex");
	PRns("RGMII: RGMII inband status (speed)", 1, "   %8s    ",
		xcv_inbnd_status.s.speed == 0 ? " 10 Mbps " :
		xcv_inbnd_status.s.speed == 1 ? " 100 Mbps" :
		xcv_inbnd_status.s.speed == 2 ? " 1 Gbps " : "Reserved");
	PRns("RGMII: RGMII inband status (link)", 1, "   %9s   ",
		xcv_inbnd_status.s.duplex ? " Link-Up " : "Link-Down");
	return 0;
}


typedef struct {
	cvmx_bgxx_cmrx_config_t 	cmr_config;
	cvmx_bgxx_spux_bx_status_t	spu_bx_status;
	cvmx_bgxx_spux_br_status1_t	spu_br_status1;
	cvmx_bgxx_spux_br_algn_status_t	br_algn_status;
	cvmx_bgxx_smux_rx_ctl_t		rx_ctl;
	cvmx_bgxx_gmp_pcs_mrx_status_t	gmp_pcs_mr_status;
} lmac_link_status_t;

/* CN78xx-HM-0.992E 33.6.5 (SPU) Link Status and Changes */
/* CN78xx-HM-0.992E 34.2.2 (GMP) PCS Mode Determination */
/* Collect/Dump the BGX Link status for all modes (SGMII, XAUI, etc. */
int cvmx_helper_bgx_link_status(int node, int bgx, unsigned N)
{
	lmac_link_status_t lmac[4];
	unsigned ind;
	uint8_t mask, mask_sgmii, mask_xaui, mask_10g, mask_40g, lmac_type;

	mask = mask_sgmii = mask_xaui = mask_10g = mask_40g = 0;
	for (ind = 0; ind < N; ind++) {
		lmac[ind].cmr_config.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_CONFIG(ind, bgx));
		lmac[ind].rx_ctl.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_SMUX_RX_CTL(ind, bgx));
		lmac_type = lmac[ind].cmr_config.s.lmac_type;
		switch (lmac_type) {
			case 0: /* SGMII/1000BASE-X */
				/* the first read reads/clears the old lnk_st;
					repeat this func to see the current lnk_st value */
				lmac[ind].gmp_pcs_mr_status.u64 = cvmx_read_csr_node(node,
					CVMX_BGXX_GMP_PCS_MRX_STATUS(ind, bgx));
				break;
			case 1: /* 10GBASE-X/XAUI or DXAUI */
			case 2: /* RXAUI */
				lmac[ind].spu_bx_status.u64 = cvmx_read_csr_node(node,
					   CVMX_BGXX_SPUX_BX_STATUS(ind, bgx));
				break;
			case 3: /* 10BASE-R */
				lmac[ind].spu_br_status1.u64 = cvmx_read_csr_node(node,
					   CVMX_BGXX_SPUX_BR_STATUS1(ind, bgx));
				break;
			case 4: /* 40GBASE-R */
				lmac[ind].br_algn_status.u64 = cvmx_read_csr_node(node,
					   CVMX_BGXX_SPUX_BR_ALGN_STATUS(ind, bgx));
				break;
		}

		if (lmac_type == 0 /* SGMII/1000BASE-X */ )
			mask_sgmii |= (1 << ind);
		else if (lmac_type == 1 /* 10GBASE-X/XAUI or DXAUI */
			|| lmac_type == 2 /* RXAUI */)
			mask_xaui |= (1 << ind);
		else if (lmac_type == 3 /* 10GBASE-R */)
			mask_10g |= (1 << ind);
		else if (lmac_type == 3 /* 10GBASE-R */)
			mask_10g |= (1 << ind);
	}

	if (mask_sgmii != 0) {
		PRMns("GMP: LINK: Link Status(lnk_st)", mask_sgmii,
			"      %4s     ",
			lmac[ind].gmp_pcs_mr_status.s.lnk_st ? " Up " : "Down");
	}
	if (mask_xaui != 0) {
		PRMns("LINK: xXAUI SerDes Lane Sync(lsync)", mask_xaui,
			"      %3s      ",
			(lmac[ind].spu_bx_status.s.lsync & (1<<ind))
				? "Yes" : " No");
		PRMns("LINK: xXAUI SerDes Lane Align(align)", mask_xaui,
			"      %3s      ",
			(lmac[ind].spu_bx_status.s.alignd & (1<<ind))
				? "Yes" : " No");
	}
	if (mask_10g != 0) {
		PRMns("LINK: 10GBASE-R PCS Receive-link(rcv_lnk)", mask_10g,
			"      %3s      ",
			lmac[ind].spu_br_status1.s.rcv_lnk ? "Yes" : " No");
		PRMns("LINK: 10GBASE-R PCS block lock(blk_lock)", mask_10g,
			"      %3s      ",
			lmac[ind].spu_br_status1.s.blk_lock ? "Yes" : " No");
	}
	if (mask_40g != 0) {
		PRMn("LINK: 40GBASE-R PCS lane Lock(block_lock)", mask_40g,
			"       %d       ",
			(lmac[ind].br_algn_status.s.block_lock & (1<<ind)) != 0 );
		PRMn("LINK: 40GBASE-R PCS lane Align(align)", mask_40g,
			"       %d       ",
			lmac[ind].br_algn_status.s.alignd );
	}
	mask = (1 << N) - 1;
	PRMns("LINK: Reconcilation Status(status)", mask,"  %11s  ",
		lmac[ind].rx_ctl.s.status == 0 ? "  Link Up  " :
		lmac[ind].rx_ctl.s.status == 1 ? " Local Fail" :
		lmac[ind].rx_ctl.s.status == 2 ? "Remote Fail" : "???????????");

	return 0;
}

typedef struct {
	cvmx_bgxx_cmrx_config_t 		cmr_config;
	cvmx_bgxx_smux_rx_frm_chk_t	rx_frm_chk;
	cvmx_bgxx_smux_rx_frm_ctl_t	rx_frm_ctl;
	cvmx_bgxx_smux_rx_int_t		smu_rx_int;
	cvmx_bgxx_smux_hg2_control_t	hg2_control;
	cvmx_bgxx_spux_int_t		spu_int;
	cvmx_bgxx_cmrx_int_t		cmr_int;
} lmac_spu_rx_errors_t;


/*=============== SPU (xXAUI, 10G, 40G mode) help funcs ==================*/
/* The following funcs are implemented in this section */
int cvmx_helper_bgx_spu_rx_errors(int node, int bgx, unsigned N);
int cvmx_helper_bgx_spu_tx_errors(int node, int bgx, unsigned N);
int cvmx_helper_bgx_spu_loopback(int node, int bgx, unsigned N);

/* CN78xx-HM-0.992E 33.5.1 (SPU) Receive Errors/Exceptions Checks */
int cvmx_helper_bgx_spu_rx_errors(int node, int bgx, unsigned N)
{
	lmac_spu_rx_errors_t lmac[4];
	unsigned ind;
	uint8_t lmac_type, mask_xaui = 0;

	for (ind = 0; ind < N; ind++) {
		lmac[ind].rx_frm_chk.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_SMUX_RX_FRM_CHK(ind, bgx));
		lmac[ind].rx_frm_ctl.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_SMUX_RX_FRM_CTL(ind, bgx));
		lmac[ind].smu_rx_int.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_SMUX_RX_INT(ind, bgx));
		lmac[ind].spu_int.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_SPUX_INT(ind, bgx));
		lmac[ind].cmr_int.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_INT(ind, bgx));
		lmac[ind].cmr_config.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_CONFIG(ind, bgx));
		lmac_type = lmac[ind].cmr_config.s.lmac_type;
		mask_xaui |= ((lmac_type != 0) << ind);
	}
	PRMcd("SPU:RXERR: Too long packet(jabber)", mask_xaui,
		/*cond*/lmac[ind].rx_frm_chk.s.jabber,
		"       %d       ", /*data*/lmac[ind].smu_rx_int.s.jabber);
	PRMcd("SPU:RXERR: Err Ctrl Char (rcverr)", mask_xaui,
		/*cond*/lmac[ind].rx_frm_chk.s.rcverr,
		"       %d       ", /*data*/lmac[ind].smu_rx_int.s.rcverr);
	PRMcd("SPU:RXERR: Ctrl Frame FCS/CRC(fcserr_c)", mask_xaui,
		/*cond*/lmac[ind].rx_frm_chk.s.fcserr_c,
		"       %d       ", /*data*/lmac[ind].smu_rx_int.s.fcserr);
	PRMcd("SPU:RXERR: Data Frame FCS/CRC(fcserr_d)", mask_xaui,
		/*cond*/lmac[ind].rx_frm_chk.s.fcserr_d,
		"       %d       ", /*data*/lmac[ind].smu_rx_int.s.fcserr);
	PRMcd("SPU:RXERR: Not-enough-data(skperr)", mask_xaui,
		/*cond*/lmac[ind].rx_frm_chk.s.skperr,
		"       %d       ", /*data*/lmac[ind].smu_rx_int.s.skperr);
	PRMcd("SPU:RXERR: Bad Preamble(pcterr)", mask_xaui,
		/*cond*/lmac[ind].rx_frm_ctl.s.pre_chk,
		"       %d       ", /*data*/lmac[ind].smu_rx_int.s.pcterr);
	PRMcd("SPU:RXERR: Rx a reserv ctrl code group(rsverr)", mask_xaui, /*cond*/1,
		"       %d       ", /*data*/lmac[ind].smu_rx_int.s.rsverr);
	PRMcd("SPU:RXERR: PAUSE RxFIFO FULL DROP(pause_drp)", mask_xaui, /*cond*/1,
		"       %d       ", /*data*/lmac[ind].cmr_int.s.pause_drp);
	PRMcd("SPU:RXERR: Code group not aligned(loc_fault)", mask_xaui, /*cond*/1,
		"       %d       ", /*data*/lmac[ind].smu_rx_int.s.loc_fault);
	PRMcd("SPU:RXERR: 'loc_fault' on remote node(rem_fault)", mask_xaui, /*cond*/1,
		"       %d       ", /*data*/lmac[ind].smu_rx_int.s.rem_fault);
	PRMcd("SPU:RXERR: A reserved sequence(bad_seq)", mask_xaui, /*cond*/1,
		"       %d       ", /*data*/lmac[ind].smu_rx_int.s.bad_seq);
	PRMcd("SPU:RXERR: Bad termination symbol(bad_term)", mask_xaui, /*cond*/1,
		"       %d       ", /*data*/lmac[ind].smu_rx_int.s.bad_term);
	PRMcd("SPU:RXERR: Code group sync lost(synlos)", mask_xaui, /*cond*/1,
		"       %d       ", /*data*/lmac[ind].spu_int.s.synlos);
	PRMcd("SPU:RXERR: Bit lock is lost(bitlcklos)", mask_xaui, /*cond*/1,
		"       %d       ", /*data*/lmac[ind].spu_int.s.bitlckls);
	PRMcd("SPU:RXERR: Bad (CRC) HiGig2 message(hg2cc)", mask_xaui,
		/*cond*/lmac[ind].hg2_control.s.hg2rx_en,
		"       %d       ", /*data*/lmac[ind].smu_rx_int.s.hg2cc);
	PRMcd("SPU:RXERR: HiGig2 isn't phy/log link msg(hg2fld)", mask_xaui,
		/*cond*/lmac[ind].hg2_control.s.hg2rx_en,
		"       %d       ", /*data*/lmac[ind].smu_rx_int.s.hg2fld);
	return 0;
}

typedef struct {
	cvmx_bgxx_cmrx_config_t 		cmr_config;
	cvmx_bgxx_smux_tx_int_t		smu_tx_int;
} lmac_spu_tx_errors_t;

/* CN78xx-HM-0.992E 33.5.2 (SPU) Transmit Errors/Exceptions Checks */
int cvmx_helper_bgx_spu_tx_errors(int node, int bgx, unsigned N)
{
	lmac_spu_tx_errors_t lmac[4];
	unsigned ind;
	uint8_t lmac_type, mask_xaui = 0;

	for (ind = 0; ind < N; ind++) {
		lmac[ind].smu_tx_int.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_SMUX_TX_INT(ind, bgx));
		lmac[ind].cmr_config.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_CONFIG(ind, bgx));
		lmac_type = lmac[ind].cmr_config.s.lmac_type;
		mask_xaui |= ((lmac_type != 0) << ind);
	}
	PRMcd("SPU:TXERR: Packet Transfer Underflow(undflw)", mask_xaui, /*cond*/1,
	       "       %d       ", /*data*/lmac[ind].smu_tx_int.s.undflw);
	PRMcd("SPU:TXERR: PTP pkt when link down(fake_commit)", mask_xaui, /*cond*/1,
	       "       %d       ", /*data*/lmac[ind].smu_tx_int.s.fake_commit);
	return 0;
}

typedef struct {
	cvmx_bgxx_cmrx_config_t 		cmr_config;
	cvmx_bgxx_spux_control1_t		spu_control1;
	cvmx_bgxx_smux_ext_loopback_t		smu_ext_loopback;
} lmac_spu_loopback_t;

/* CN78xx-HM-0.992E 33.8 (SPU, SMU) Loopback */
int cvmx_helper_bgx_spu_loopback(int node, int bgx, unsigned N)
{
	lmac_spu_loopback_t lmac[4];
	unsigned ind;
	bool lb1[4]/*internal loopback*/, lb2[4]/* external loopback*/;
	uint8_t lmac_type, mask_xaui = 0;

	for (ind = 0; ind < N; ind++) {
		lmac[ind].cmr_config.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_CONFIG(ind, bgx));
		lmac[ind].spu_control1.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_SPUX_CONTROL1(ind, bgx));
		lmac[ind].smu_ext_loopback.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_SMUX_EXT_LOOPBACK(ind, bgx));

		lb1[ind] = (lmac[ind].spu_control1.s.loopbck == 1)/* int lb enabled */
			&& (lmac[ind].cmr_config.s.enable == 1);  /* Logical MAC/PCS enabled */
		lb2[ind] = (lmac[ind].smu_ext_loopback.s.en == 1);/* ext lb enabled */
		lmac_type = lmac[ind].cmr_config.s.lmac_type;
		mask_xaui |= ((lmac_type != 0) << ind);
	}
	PRMcs("SPU: Internal Loopback(loopback1)", mask_xaui, /*cond*/lb1[ind],
	       "   %8s    ", /*data*/lb1[ind] ? " Enabled" : "Disabled");
	PRMcs("SMU: External Loopback(en)", mask_xaui, /*cond*/lb2[ind],
	       "   %8s    ", /*data*/lb2[ind] ? " Enabled" : "Disabled");
	return 0;
}


/*===================== SGMII mode help funcs =====================*/
/* The following funcs are implemented in this section */
int cvmx_helper_bgx_gmp_rx_errors(int node, int bgx, unsigned N);
int cvmx_helper_bgx_gmp_tx_errors(int node, int bgx, unsigned N);
int cvmx_helper_bgx_gmp_pcs_errors(int node, int bgx, unsigned N);
int cvmx_helper_bgx_gmp_loopback(int node, int bgx, unsigned N);
int cvmx_helper_bgx_gmp_frm_ctl(int node, int bgx, unsigned N);

typedef struct {
	cvmx_bgxx_cmrx_config_t 		cmr_config;
	cvmx_bgxx_gmp_gmi_rxx_frm_chk_t		gmp_gmi_rx_frm_chk;
	cvmx_bgxx_gmp_gmi_rxx_frm_ctl_t		gmp_gmi_rx_frm_ctl;
	cvmx_bgxx_gmp_gmi_rxx_int_t		gmp_gmi_rx_int;
	cvmx_bgxx_gmp_pcs_intx_t		gmp_pcs_int;
} lmac_gmi_rx_errors_t;

/* CN78xx-HM-0.992E 34.4.1 (GMI) Receive Error/Exception Checks */
int cvmx_helper_bgx_gmp_rx_errors(int node, int bgx, unsigned N)
{
	lmac_gmi_rx_errors_t lmac[4];
	unsigned ind;
	uint8_t lmac_type, mask_gmii, mask_sgmii = 0, mask_rgmii = 0;;

	for (ind = 0; ind < N; ind++) {
		lmac[ind].gmp_gmi_rx_frm_chk.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_GMP_GMI_RXX_FRM_CHK(ind, bgx));
		lmac[ind].gmp_gmi_rx_frm_ctl.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_GMP_GMI_RXX_FRM_CTL(ind, bgx));
		lmac[ind].gmp_gmi_rx_int.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_GMP_GMI_RXX_INT(ind, bgx));
		lmac[ind].gmp_pcs_int.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_GMP_PCS_INTX(ind, bgx));
		lmac[ind].cmr_config.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_CONFIG(ind, bgx));
		lmac_type = lmac[ind].cmr_config.s.lmac_type;
		mask_sgmii |= ((lmac_type == 0) << ind);
		mask_rgmii |= ((lmac_type == 5) << ind);
	}
	mask_gmii = mask_sgmii || mask_rgmii;
	PRMcd("GMP:RXERR: Carrier Extend errors(carext)", mask_sgmii,
		/*cond*/lmac[ind].gmp_gmi_rx_frm_chk.s.carext,
		"       %d       ", /*data*/lmac[ind].gmp_gmi_rx_int.s.carext);
	PRMcd("GMP:RXERR: Too long packet(jabber)", mask_gmii,
		/*cond*/lmac[ind].gmp_gmi_rx_frm_chk.s.jabber,
		"       %d       ", /*data*/lmac[ind].gmp_gmi_rx_int.s.jabber);
	PRMcd("GMP:RXERR: Ctrl Frame FCS/CRC(fcserr)", mask_gmii,
		/*cond*/lmac[ind].gmp_gmi_rx_frm_chk.s.fcserr,
		"       %d       ", /*data*/lmac[ind].gmp_gmi_rx_int.s.fcserr);
	PRMcd("GMP:RXERR: Too short packet(minerr)", mask_gmii,
		/*cond*/lmac[ind].gmp_gmi_rx_frm_chk.s.minerr,
		"       %d       ", /*data*/lmac[ind].gmp_gmi_rx_int.s.minerr);
	PRMcd("GMP:RXERR: Data-reception error(rcverr)", mask_gmii,
		/*cond*/lmac[ind].gmp_gmi_rx_frm_chk.s.rcverr,
		"       %d       ", /*data*/lmac[ind].gmp_gmi_rx_int.s.rcverr);
	PRMcd("GMP:RXERR: Not-enough-data(skperr)", mask_gmii,
		/*cond*/lmac[ind].gmp_gmi_rx_frm_chk.s.skperr,
		"       %d       ", /*data*/lmac[ind].gmp_gmi_rx_int.s.skperr);
	PRMcd("GMP:RXERR: Overrun (data come too fast)(ovrerr)", mask_sgmii, /*cond*/1,
		"       %d       ", /*data*/lmac[ind].gmp_gmi_rx_int.s.ovrerr);
	PRMcd("GMP:RXERR: Bad Preamble(pcterr)", mask_gmii,
		/*cond*/lmac[ind].gmp_gmi_rx_frm_ctl.s.pre_chk,
		"       %d       ", /*data*/lmac[ind].gmp_gmi_rx_int.s.pcterr);
	PRMcd("GMP:RXERR: Rx a reserv ctrl code group(rsverr)", mask_gmii, /*cond*/1,
		"       %d       ", /*data*/lmac[ind].gmp_gmi_rx_int.s.rsverr);
	PRMcd("GMP:RXERR: False carrier detected(falerr)", mask_sgmii, /*cond*/1,
		"       %d       ", /*data*/lmac[ind].gmp_gmi_rx_int.s.falerr);
	PRMcd("GMP:RXERR: Collision detect (Half Dupl)(coldet)", mask_gmii, /*cond*/1,
		"       %d       ", /*data*/lmac[ind].gmp_gmi_rx_int.s.coldet);
	PRMcd("GMP:RXERR: Small Interframe Gap(ifgerr)", mask_sgmii, /*cond*/1,
		"       %d       ", /*data*/lmac[ind].gmp_gmi_rx_int.s.ifgerr);
/* the following field (pause_drp) doesn't exists, but it is listed in 34.4.1 Table34-3 */
/*	PRMcd("GMP:RXERR: PAUSE RxFIFO FULL DROP(pause_drp)", mask_gmii, 1, */
/*	       "       %d       ", lmac[ind].gmp_gmi_rx_int.s.pause_drp); */
	PRMcd("GMP:RXERR: A bad (10-bit) code group(rxerr)", mask_gmii, /*cond*/1,
	       "       %d       ", /*data*/lmac[ind].gmp_pcs_int.s.rxerr);
	PRMcd("GMP:RXERR: Lost sync lock(bit or group)(rxlock)", mask_gmii, /*cond*/1,
	       "       %d       ", /*data*/lmac[ind].gmp_pcs_int.s.rxlock);
	PRMcd("GMP:RXERR: Bad Sync (shouldn't occur)(sync_bad)", mask_gmii, /*cond*/1,
	       "       %d       ", /*data*/lmac[ind].gmp_pcs_int.s.sync_bad);
	PRMcd("GMP:RXERR: Internal Receive error(rxbad)", mask_gmii, /*cond*/1,
	       "       %d       ", /*data*/lmac[ind].gmp_pcs_int.s.rxbad);
	return 0;
}


typedef struct {
	cvmx_bgxx_cmrx_config_t 		cmr_config;
	cvmx_bgxx_gmp_gmi_txx_int_t		gmp_gmi_tx_int;
	cvmx_bgxx_gmp_pcs_intx_t		gmp_pcs_int;
	cvmx_bgxx_cmrx_int_t			cmr_int;
	cvmx_bgxx_cmr_bad_t			cmr_bad;
} lmac_gmp_tx_errors_t;

/* CN78xx-HM-0.992E 34.4.2 (GMP) Transmit Error/Exception Checks */
int cvmx_helper_bgx_gmp_tx_errors(int node, int bgx, unsigned N)
{
	lmac_gmp_tx_errors_t lmac[4];
	unsigned ind;
	uint8_t lmac_type, mask_sgmii = 0;

	for (ind = 0; ind < N; ind++) {
		lmac[ind].gmp_gmi_tx_int.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_GMP_GMI_TXX_INT(ind, bgx));
		lmac[ind].gmp_pcs_int.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_GMP_PCS_INTX(ind, bgx));
		lmac[ind].cmr_int.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_INT(ind, bgx));
		lmac[ind].cmr_int.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMR_BAD(bgx));
		lmac[ind].cmr_config.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_CONFIG(ind, bgx));
		lmac_type = lmac[ind].cmr_config.s.lmac_type;
		mask_sgmii |= ((lmac_type == 0) << ind);
	}
	PRMcd("GMP:TXERR: Wrong channel value(pko_nxc)", mask_sgmii, /*cond*/1,
	       "       %d       ", /*data*/lmac[ind].cmr_int.s.pko_nxc);
	PRMcd("GMP:TXERR: Packet Transfer Underflow(undflw)", mask_sgmii, /*cond*/1,
	       "       %d       ", /*data*/lmac[ind].gmp_gmi_tx_int.s.undflw);
	PRMcd("GMP:TXERR: Late Collision in Half Dplx(late_col)", mask_sgmii, /*cond*/1,
	       "       %d       ", /*data*/lmac[ind].gmp_gmi_tx_int.s.late_col);
	PRMcd("GMP:TXERR: Excessive Collision Half Dplx(xscol)", mask_sgmii, /*cond*/1,
	       "       %d       ", /*data*/lmac[ind].gmp_gmi_tx_int.s.xscol);
	PRMcd("GMP:TXERR: Excessive Deferral Half Dplx(xsdef)", mask_sgmii, /*cond*/1,
	       "       %d       ", /*data*/lmac[ind].gmp_gmi_tx_int.s.xsdef);
/* the following field (out_ovr) doesn't exists, but it is listed in 34.4.2 Table34-4 */
/* the following field (loststat) doesn't exists, but it is listed in 34.4.2 Table34-4 */
/* the following field (statovr) doesn't exists, but it is listed in 34.4.2 Table34-4 */
/* the following field (inb_nxa) doesn't exists, but it is listed in 34.4.2 Table34-4 */
	PRMcd("GMP:TXERR: Tx Internal Error(txbad)", mask_sgmii, /*cond*/1,
	       "       %d       ", /*data*/lmac[ind].gmp_pcs_int.s.txbad);
	PRMcd("GMP:TXERR: Tx Internal Error(txfifo)", mask_sgmii, /*cond*/1,
	       "       %d       ", /*data*/lmac[ind].gmp_pcs_int.s.txfifo);
	PRMcd("GMP:TXERR: Tx Internal Error(txfifu)", mask_sgmii, /*cond*/1,
	       "       %d       ", /*data*/lmac[ind].gmp_pcs_int.s.txfifu);
	return 0;
}


typedef struct {
	cvmx_bgxx_cmrx_config_t 		cmr_config;
	cvmx_bgxx_gmp_pcs_intx_t		gmp_pcs_int;
	cvmx_bgxx_gmp_pcs_mrx_status_t		gmp_pcs_mr_status;
} lmac_gmp_pcs_errors_t;

/* CN78xx-HM-0.992E 34.4.4 (GMP) PCS Error/Exception Checks */
int cvmx_helper_bgx_gmp_pcs_errors(int node, int bgx, unsigned N)
{
	lmac_gmp_pcs_errors_t lmac[4];
	unsigned ind;
	uint8_t lmac_type, mask_sgmii = 0;

	for (ind = 0; ind < N; ind++) {
		lmac[ind].gmp_pcs_int.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_GMP_PCS_INTX(ind, bgx));
		lmac[ind].gmp_pcs_mr_status.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_GMP_PCS_MRX_STATUS(ind, bgx));
		lmac[ind].cmr_config.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_CONFIG(ind, bgx));
		lmac_type = lmac[ind].cmr_config.s.lmac_type;
		mask_sgmii |= ((lmac_type == 0) << ind);
	}
	PRMcd("GMP:PCSERR: ANEG didn't found match mode(an_err)", mask_sgmii, /*cond*/1,
	       "       %d       ", /*data*/lmac[ind].gmp_pcs_int.s.an_err);
	PRMcd("GMP:PCSERR: Remote fault received(rm_flt)", mask_sgmii, /*cond*/1,
	       "       %d       ", /*data*/lmac[ind].gmp_pcs_mr_status.s.rm_flt);
	PRMcd("GMP:PCSERR: The xmit variable changes(xmit)", mask_sgmii, /*cond*/1,
	       "       %d       ", /*data*/lmac[ind].gmp_pcs_int.s.xmit);
	PRMcd("GMP:PCSERR: Link Speed mode change(lnkspd)", mask_sgmii, /*cond*/1,
	       "       %d       ", /*data*/lmac[ind].gmp_pcs_int.s.lnkspd);
	PRMcd("GMP:PCSERR: Duplex mode changes(dup)", mask_sgmii, /*cond*/1,
	       "       %d       ", /*data*/lmac[ind].gmp_pcs_int.s.dup);
	PRMcd("GMP:PCSERR: ANEG state machine fault(an_bad)", mask_sgmii, /*cond*/1,
	       "       %d       ", /*data*/lmac[ind].gmp_pcs_int.s.an_bad);
	return 0;
}


typedef struct {
	cvmx_bgxx_cmrx_config_t 		cmr_config;
	cvmx_bgxx_gmp_pcs_mrx_control_t		gmp_pcs_mr_control;
	cvmx_bgxx_gmp_pcs_miscx_ctl_t		gmp_pcs_misc_ctl;
} lmac_gmp_loopback_t;

/* CN78xx-HM-0.992E 34.7 (GMP) Loopback */
int cvmx_helper_bgx_gmp_loopback(int node, int bgx, unsigned N)
{
	lmac_gmp_loopback_t lmac[4];
	unsigned ind;
	bool lb1[4]/*internal loopback*/, lb2[4]/* external loopback*/;
	uint8_t lmac_type, mask_sgmii = 0;

	for (ind = 0; ind < N; ind++) {
		lmac[ind].cmr_config.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_CONFIG(ind, bgx));
		lmac[ind].gmp_pcs_mr_control.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_GMP_PCS_MRX_CONTROL(ind, bgx));
		lmac[ind].gmp_pcs_misc_ctl.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_GMP_PCS_MISCX_CTL(ind, bgx));

		lb1[ind] = (lmac[ind].gmp_pcs_mr_control.s.loopbck1 == 1)/* int lb enabled */
			&& (lmac[ind].cmr_config.s.enable == 1) 	/* Logical MAC/PCS enabled */
			&& (lmac[ind].gmp_pcs_misc_ctl.s.gmxeno == 0);	/* GMI enable override */
		lb2[ind] = (lmac[ind].gmp_pcs_misc_ctl.s.loopbck2 == 1);/* ext lb enabled */
		lmac_type = lmac[ind].cmr_config.s.lmac_type;
		mask_sgmii |= ((lmac_type == 0) << ind);
	}
	PRMcs("GMP: Internal Loopback(loopbck1)", mask_sgmii, /*cond*/lb1[ind],
	       "   %8s    ", /*data*/lb1[ind] ? " Enabled" : "Disabled");
	PRMcs("GMP: External Loopback(loopbck2)", mask_sgmii, /*cond*/lb2[ind],
	       "   %8s    ", /*data*/lb2[ind] ? " Enabled" : "Disabled");
	return 0;
}

typedef struct {
	cvmx_bgxx_cmrx_config_t 		cmr_config;
	cvmx_bgxx_gmp_gmi_rxx_frm_ctl_t		gmp_gmi_rx_frm_ctl;
} lmac_gmp_frm_ctl_t;

/* Print out the BGX Frame Control Configuration (which frame checvhs are enabled */
int cvmx_helper_bgx_gmp_frm_ctl(int node, int bgx, unsigned N)
{
	lmac_gmp_frm_ctl_t lmac[4];
	unsigned ind;
	uint8_t lmac_type, mask_sgmii = 0;

	for (ind = 0; ind < N; ind++) {
		lmac[ind].cmr_config.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_CONFIG(ind, bgx));
		lmac[ind].gmp_gmi_rx_frm_ctl.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_GMP_GMI_RXX_FRM_CTL(ind, bgx));

		lmac_type = lmac[ind].cmr_config.s.lmac_type;
		mask_sgmii |= ((lmac_type == 0) << ind);
	}
	PRMns("GMP:FRM_CTL: Timestamp Enable(ptp_mode)",
		mask_sgmii, "   %8s    ",
		lmac[ind].gmp_gmi_rx_frm_ctl.s.ptp_mode ? " Enabled" : "Disabled");
	PRMns("GMP:FRM_CTL: Don't modify MODbits(null_dis)",
		mask_sgmii, "   %8s    ",
		lmac[ind].gmp_gmi_rx_frm_ctl.s.null_dis ? " Enabled" : "Disabled");
	PRMns("GMP:FRM_CTL: Aligns the SFD byte(pre_align)",
		mask_sgmii, "   %8s    ",
		lmac[ind].gmp_gmi_rx_frm_ctl.s.pre_align ? " Enabled" : "Disabled");
	PRMns("GMP:FRM_CTL: Begin frame at first SFD(pre_free)",
		mask_sgmii, "   %8s    ",
		lmac[ind].gmp_gmi_rx_frm_ctl.s.pre_free ? " Enabled" : "Disabled");
	PRMns("GMP:FRM_CTL: PAUSE frm can match SMAC(ctl_smac)",
		mask_sgmii, "   %8s    ",
		lmac[ind].gmp_gmi_rx_frm_ctl.s.ctl_smac ? " Enabled" : "Disabled");
	PRMns("GMP:FRM_CTL: PAUSE frm can match MCAST(ctl_mcst)",
		mask_sgmii, "   %8s    ",
		lmac[ind].gmp_gmi_rx_frm_ctl.s.ctl_mcst ? " Enabled" : "Disabled");
	PRMns("GMP:FRM_CTL: Fwd PAUSE info to TX block(ctl_bck)",
		mask_sgmii, "   %8s    ",
		lmac[ind].gmp_gmi_rx_frm_ctl.s.ctl_bck ? " Enabled" : "Disabled");
	PRMns("GMP:FRM_CTL: Drop control-PAUSE frames(ctl_drp)",
		mask_sgmii, "   %8s    ",
		lmac[ind].gmp_gmi_rx_frm_ctl.s.ctl_drp ? " Enabled" : "Disabled");
	PRMns("GMP:FRM_CTL: Strip off the preamble (pre_strp)",
		mask_sgmii, "   %8s    ",
		lmac[ind].gmp_gmi_rx_frm_ctl.s.pre_strp ? " Enabled" : "Disabled");
	PRMns("GMP:FRM_CTL: Check preamble is correct(pre_chk)",
		mask_sgmii, "   %8s    ",
		lmac[ind].gmp_gmi_rx_frm_ctl.s.pre_chk ? " Enabled" : "Disabled");
	return 0;
}



/*===================== BGX CONFIG Dump func =====================*/
/* The following funcs are implemented in this section */
int cvmx_dump_bgx_config(unsigned bgx);
int cvmx_dump_bgx_config_node(unsigned node, unsigned bgx);

typedef struct {
	cvmx_bgxx_cmrx_config_t 	cmr_config;
	cvmx_bgxx_cmrx_rx_id_map_t 	cmr_rx_id_map;
	cvmx_bgxx_cmrx_rx_bp_drop_t 	cmr_rx_bp_drop;
	cvmx_bgxx_cmrx_rx_bp_on_t 	cmr_rx_bp_on;
	cvmx_bgxx_cmrx_rx_bp_off_t 	cmr_rx_bp_off;
	cvmx_bgxx_cmrx_rx_weight_t 	cmr_rx_weight;
	cvmx_bgxx_cmrx_rx_adr_ctl_t 	cmr_rx_adr_ctl;
	cvmx_bgxx_cmrx_rx_logl_xoff_t 	cmr_rx_logl_xoff;
	cvmx_bgxx_cmrx_rx_logl_xon_t 	cmr_rx_logl_xon;
	cvmx_bgxx_cmrx_tx_channel_t 	cmr_tx_channel;
	cvmx_bgxx_spux_an_control_t 	spu_an_control;
	cvmx_bgxx_spux_an_adv_t 	spu_an_adv;
	cvmx_bgxx_smux_cbfc_ctl_t 	smu_cbfc_ctl;
	cvmx_bgxx_smux_tx_ctl_t 	smu_tx_ctl;
	cvmx_bgxx_smux_rx_udd_skp_t	smu_rx_udd_skp;
	cvmx_bgxx_smux_tx_thresh_t 	smu_tx_thresh;
	cvmx_bgxx_smux_hg2_control_t 	smu_hg2_control;
	cvmx_helper_link_info_t 	link_info;
	cvmx_bgxx_gmp_pcs_miscx_ctl_t	gmp_pcs_misc_ctl;
	cvmx_bgxx_gmp_pcs_mrx_control_t	gmp_pcs_mr_control;
	cvmx_bgxx_gmp_pcs_anx_results_t gmp_pcs_an_results;
	cvmx_bgxx_gmp_pcs_mrx_status_t	gmp_pcs_mr_status;
	cvmx_bgxx_gmp_pcs_rxx_sync_t	gmp_pcs_rx_sync;
	cvmx_bgxx_gmp_gmi_rxx_frm_ctl_t	gmp_gmi_rx_frm_ctl;
	cvmx_bgxx_gmp_gmi_prtx_cfg_t	gmp_gmi_prt_cfg;
} lmac_config_t;

int cvmx_dump_bgx_config_node(unsigned node, unsigned bgx)
{
	lmac_config_t lmac[4];
	cvmx_bgxx_cmr_global_config_t global_config;
	cvmx_bgxx_cmr_rx_ovr_bp_t cmr_rx_ovr_bp;
	unsigned ind, N;
	uint8_t mask_aneg_ovrd, mask_aneg, lmac_type, lmac_gmii;
	uint8_t lmac_sgmii, lmac_rgmii, lmac_xaui, lmac_rxaui, lmac_10g_r, lmac_40g_r;
	int ipd_port, qlm, gbaud_mhz;

	lmac_sgmii = lmac_rgmii = lmac_xaui = lmac_rxaui = lmac_10g_r  = lmac_40g_r = 0;
	mask_aneg_ovrd = mask_aneg = 0;

	global_config.u64 = cvmx_read_csr_node(node,
		CVMX_BGXX_CMR_GLOBAL_CONFIG(bgx));

	cvmx_dprintf("\n/*===== BGX CONFIG Parameters			BGX%d =====*/\n", bgx);
	/* just report configured RX/Tx LMACS - don't check return */
	cvmx_helper_bgx_number_rx_tx_lmacs(node, bgx, &N);

	qlm = bgx<2 ? (global_config.s.pmux_sds_sel==1 ? bgx+2 : bgx) : bgx+2;
	cvmx_dprintf("NODE%d: BGX%d/lmac[0..%d] connected to QLM%d\n",
		node, bgx, N - 1, qlm);

	for (ind = 0; ind < N; ind++) {

		lmac[ind].cmr_config.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_CONFIG(ind, bgx));
		lmac[ind].cmr_rx_id_map.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_RX_ID_MAP(ind, bgx));
		ipd_port = cvmx_helper_get_ipd_port(bgx, ind);
		lmac[ind].link_info = cvmx_helper_link_get(ipd_port);
		lmac[ind].spu_an_control.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_SPUX_AN_CONTROL(ind, bgx));
		lmac[ind].spu_an_adv.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_SPUX_AN_ADV(ind, bgx));
		lmac[ind].cmr_rx_bp_drop.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_RX_BP_DROP(ind, bgx));
		lmac[ind].cmr_rx_bp_on.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_RX_BP_ON(ind, bgx));
		lmac[ind].cmr_rx_bp_off.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_RX_BP_OFF(ind, bgx));
		lmac[ind].cmr_rx_weight.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_RX_WEIGHT(ind, bgx));
		lmac[ind].cmr_rx_adr_ctl.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_RX_ADR_CTL(ind, bgx));
		lmac[ind].cmr_rx_logl_xoff.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_RX_LOGL_XOFF(ind, bgx));
		lmac[ind].cmr_rx_logl_xon.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_RX_LOGL_XON(ind, bgx));
		lmac[ind].cmr_tx_channel.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_TX_CHANNEL(ind, bgx));
		lmac[ind].gmp_pcs_misc_ctl.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_GMP_PCS_MISCX_CTL(ind, bgx));
		lmac[ind].gmp_pcs_mr_control.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_GMP_PCS_MRX_CONTROL(ind, bgx));
		lmac[ind].gmp_pcs_an_results.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_GMP_PCS_ANX_RESULTS(ind, bgx));
		lmac[ind].gmp_pcs_mr_status.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_GMP_PCS_MRX_STATUS(ind, bgx));
		lmac[ind].gmp_gmi_rx_frm_ctl.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_GMP_GMI_RXX_FRM_CTL(ind, bgx));
		lmac[ind].gmp_pcs_rx_sync.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_GMP_PCS_RXX_SYNC(ind, bgx));
		lmac[ind].smu_cbfc_ctl.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_SMUX_CBFC_CTL(ind, bgx));
		lmac[ind].smu_tx_ctl.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_SMUX_TX_CTL(ind, bgx));
		lmac[ind].smu_rx_udd_skp.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_SMUX_RX_UDD_SKP(ind, bgx));
		lmac[ind].smu_tx_thresh.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_SMUX_TX_THRESH(ind, bgx));
		lmac[ind].smu_hg2_control.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_SMUX_HG2_CONTROL(ind, bgx));
		lmac[ind].gmp_gmi_prt_cfg.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_GMP_GMI_PRTX_CFG(ind, bgx));

		lmac_type = lmac[ind].cmr_config.s.lmac_type;
		lmac_sgmii |= ((lmac_type == 0) << ind);
		lmac_xaui |= ((lmac_type == 1) << ind);
		lmac_rxaui |= ((lmac_type == 2) << ind);
		lmac_10g_r |= ((lmac_type == 3) << ind);
		lmac_40g_r |= ((lmac_type == 4) << ind);
		lmac_rgmii |= ((lmac_type == 5) << ind);

		if (lmac[ind].gmp_pcs_misc_ctl.s.an_ovrd /* ANEG Overrided */
		    || (lmac[ind].gmp_pcs_mr_control.s.an_en == 0) ) /*ANEG disabled */
			mask_aneg_ovrd |= (1 << ind);
		if ( (lmac[ind].gmp_pcs_mr_control.s.an_en == 1) /* ANEG Enabled */
/* NOTE: Do not check 'ANEG complete', just report what is when (ANEG_Enabled && !Overrided) */
/*			&& (lmac[ind].gmp_pcs_mr_status.s.an_cpt == 1)*/ /* ANEG complete */
			&& (lmac[ind].gmp_pcs_misc_ctl.s.an_ovrd == 0) )/* not overrided */
			mask_aneg |= (1 << ind);
	}

	PRn("LMAC:", N, "     lmac%d     ", ind);
	PRns("Type", N, "   %-10s  ",
		get_lmac_type_name(lmac[ind].cmr_config.s.lmac_type,
				   lmac[ind].gmp_pcs_misc_ctl.s.mode) );
	gbaud_mhz = cvmx_qlm_get_gbaud_mhz(qlm);
	PRn("QLM Baud Rate[MHz]", N, "     %5d     ",
	       gbaud_mhz);/* NOTE: OK - same rate for all LMACs */
	PRns("Master LMAC (Logical MAC/PCS) Enabled?(enable)", N, "   %8s    ",
	       lmac[ind].cmr_config.s.enable ? " Enabled" : "Disabled");
	PRn("Num PCS Lanes", N, "       %d       ",
	       get_num_pcs_lanes(lmac[ind].cmr_config.s.lmac_type));
	PRns("Binded to QLM lane(s)", N, "   %9s   ",
		get_bind_lanes_per_lmac(ind, lmac[ind].cmr_config.s.lmac_type,
				lmac[ind].cmr_config.s.lane_to_sds) );
	PRns("Mix Enabled?(mix_en)", N, "   %8s   ",
	       lmac[ind].cmr_config.s.mix_en ? " Enabled" : "Disabled");
	PRn("Port kind (pknd)", N, "       %d       ",
	       (lmac[ind].cmr_rx_id_map.s.pknd & 0x3F));

	cvmx_dprintf("/*=== Frame Check Control, Loopback, ANEG Parameters ===*/\n");
	lmac_gmii = lmac_sgmii || lmac_rgmii;
	if (lmac_sgmii/*SGMII/1000BASE-X*/ || lmac_rgmii/*RGMII*/) {
		PRcs("GMP: Power Down?(pwr_dn)", /* display only if in 'Power Down' */
			N, /*cond*/lmac[ind].gmp_pcs_mr_control.s.pwr_dn,
			"  %10s   ",
			lmac[ind].gmp_pcs_mr_control.s.pwr_dn
				? "Power Down" : " Power On "/*never displayed*/);
		PRMcs("GMP: PCS acts as (mac_phy)",	      lmac_sgmii,
		       (lmac[ind].gmp_pcs_misc_ctl.s.mode == 0) /* SGMII */,
		       "      %3s      ",
		       lmac[ind].gmp_pcs_misc_ctl.s.mac_phy == 0 ? "MAC" : "PHY");
		cvmx_helper_bgx_gmp_frm_ctl(node, bgx, N);
		if (mask_aneg_ovrd != 0) { /* ANEG is overrided */
#undef 			spd
#define 		spd  (lmac[ind].gmp_pcs_mr_control.s.spdmsb << 1 \
				| lmac[ind].gmp_pcs_mr_control.s.spdlsb)
			PRMns("GMP: SPEED  (Overrided)", mask_aneg_ovrd, "   %9s   ",
				spd == 0 ? "  10 Mb/s" :
				spd == 1 ? " 100 Mb/s" :
				spd == 2 ? "1000 Mb/s" : "   ???   ");
			PRMns("GMP: DUPLEX (Overrided)", mask_aneg_ovrd,
				"      %4s     ",
				lmac[ind].gmp_pcs_mr_control.s.dup ? "Full" : "Half");
		}
		if (mask_aneg != 0) { /* SPEED and DUPLEX from ANEG */
			//mask_aneg = 0xf;
			PRMns("GMP: ANEG Enabled?(an_en)",
				mask_aneg, "   %8s    ",
				lmac[ind].gmp_pcs_mr_control.s.an_en
					? " Enabled" : "Disabled");
			PRMcs("GMP: ANEG(Enabled) Completed?(an_cpt)",
				mask_aneg,
				/*cond*/lmac[ind].gmp_pcs_mr_control.s.an_en,
				"      %3s      ",
				lmac[ind].gmp_pcs_mr_status.s.an_cpt ? "Yes" : " No");
#undef 			spd
#define			spd  (lmac[ind].gmp_pcs_an_results.s.spd)
			PRMcs("GMP: SPEED  -from ANEG(Enabled && !Overrided)",
				mask_aneg,
				/*cond*/lmac[ind].gmp_pcs_mr_control.s.an_en,
				"   %9s   ",
				spd == 0 ? "  10 Mb/s" :
				spd == 1 ? " 100 Mb/s" :
				spd == 2 ? "1000 Mb/s" : "   ???   ");
			PRMcs("GMP: DUPLEX -from ANEG(Enabled && !Overrided)",
				mask_aneg,
				/*cond*/lmac[ind].gmp_pcs_mr_control.s.an_en,
				"      %4s     ",
				lmac[ind].gmp_pcs_an_results.s.dup ? "Full" : "Half");
		}
		PRMns("GMP: PCS_Link Status(lnk_st)",lmac_gmii, "      %4s     ",
			 lmac[ind].gmp_pcs_mr_status.s.lnk_st ? " Up " : "Down");
		PRMns("GMP: PCS_RX_SYNC Status(sync)",lmac_gmii, "    %8s   ",
			 lmac[ind].gmp_pcs_rx_sync.s.sync ? "  Sync  " : "NOT Sync");
		/* skip reporting polarity of serdes lines for now - low priority */
		/* 34.3.1.1 Receive Flow Control */
#undef 		rx_flow_ctl
#define		rx_flow_ctl (lmac[ind].gmp_gmi_rx_frm_ctl.s.ctl_bck << 1\
				| lmac[ind].gmp_gmi_rx_frm_ctl.s.ctl_drp)
		PRMns("GMP: Receive_Flow_Control (PAUSE pkt)", lmac_gmii, "  %12s ",
		        rx_flow_ctl == 1 /* bck=0 & drp=1*/ ? "  Ingnored  " :
		        rx_flow_ctl == 0 /* bck=0 & drp=0*/ ? " SW process " :
		        rx_flow_ctl == 2 /* bck=1 & drp=0*/ ? " send PAUSE " :
		        rx_flow_ctl == 3 /* bck=1 & drp=1*/ ? " HW process " :
		        "            ");
	}

	if (lmac_sgmii != 0/*SGMII/1000BASE-X*/) {

		PRMns("GMP: PCS_MODE(mode)",	      lmac_sgmii, "  %10s  ",
		       lmac[ind].gmp_pcs_misc_ctl.s.mode == 0
			? "   SGMII   " : "1000BASE-X");
		cvmx_helper_bgx_gmp_loopback(node, bgx, N);

		PRMd("GMP: Unidirectional (overwrite ANEG)", lmac_sgmii,
			"       %d       ", lmac[ind].gmp_pcs_mr_control.s.uni);

		PRMns("GMP:Duplex mode (SGMII/1000Base-X only)(duplex)", lmac_sgmii,
			"  %11s  ",
			lmac[ind].gmp_gmi_prt_cfg.s.duplex ? "Full-duplex" : "Half-duplex");
#undef		spd
#define		spd 	(lmac[ind].gmp_gmi_prt_cfg.s.speed_msb << 1 \
			| lmac[ind].gmp_gmi_prt_cfg.s.speed)
		PRMns("GMP:Speed (SGMII/1000Base-X only)(duplex)", lmac_sgmii,
			"   %9s   ",
			spd == 0 ? " 100 Mb/s" :
			spd == 1 ? "1000 Mb/s" :
			spd == 2 ? "  10 Mb/s" : "Reserved ");
	}

	if (lmac_rxaui != 0/*RXAUI*/) { /*ANEG not defined for RXAUI */
		cvmx_helper_bgx_spu_loopback(node, bgx, N);

		PRns("ANEG (not defined for RXAUI)", N, "  %11s  ", "not defined");
	}
	if (lmac_xaui || lmac_10g_r || lmac_40g_r) {
		/* for all modes, but [sgmii, rxaui] */
		uint8_t mask_134 = lmac_xaui | lmac_10g_r | lmac_40g_r;

		cvmx_helper_bgx_spu_loopback(node, bgx, N);

		PRMns("ANEG Enabled?(an_en)",	       mask_134,"   %8s    ",
		       lmac[ind].spu_an_control.s.an_en  ? " Enabled" : "Disabled");
		PRMd("ANEG_ADV: FEC req(fec_req)",    mask_134,"       %d       ",
		       lmac[ind].spu_an_adv.s.fec_req);
		PRMd("ANEG_ADV: FEC able(fec_able)",   mask_134,"       %d       ",
		       lmac[ind].spu_an_adv.s.fec_able);
		PRMd("ANEG_ADV: 40GBASE_CR4(a40g_cr4)",mask_134,"       %d       ",
		       lmac[ind].spu_an_adv.s.a40g_cr4);
		PRMd("ANEG_ADV: 40GBASE_KR4(a40g_kr4)",mask_134,"       %d       ",
		       lmac[ind].spu_an_adv.s.a40g_kr4);
		PRMd("ANEG_ADV: 10GBASE_KR(a10g_kr)", mask_134,"       %d       ",
		       lmac[ind].spu_an_adv.s.a10g_kr);
		PRMd("ANEG_ADV: 10GBASE_KX4(a10g_kx4)",mask_134,"       %d       ",
		       lmac[ind].spu_an_adv.s.a10g_kx4);
		PRMd("ANEG_ADV: 1000BASE_KX(a1g_kx)",mask_134,"       %d       ",
		       lmac[ind].spu_an_adv.s.a1g_kx);
		PRMd("ANEG_ADV: Asym PAUSE(asm_dir)", mask_134,"       %d       ",
		       lmac[ind].spu_an_adv.s.asm_dir);
		PRMd("ANEG_ADV: PAUSE able(pause)", mask_134,"       %d       ",
		       lmac[ind].spu_an_adv.s.pause);
	}


	cvmx_dprintf("/*=== BGX MISC CONFIG Parameters ===*/\n");
	PRns("Link Status (from (external) PHY ANEG)[SE API]", N,
	     "      %4s     ", lmac[ind].link_info.s.link_up ? " Up " : "Down");
	PRns("Link Duplex [SE API]",		N, "      %4s     ",
		lmac[ind].link_info.s.full_duplex ? "Full" : "Half");
	PRn("Link Speed (MBps) [SE API]",	N, "      %5d    ",
	       lmac[ind].link_info.s.speed);
	PRns("Data Packet Receive (data_pkt_rx_en)", N, "   %8s    ",
	       lmac[ind].cmr_config.s.data_pkt_rx_en ? " Enabled" : "Disabled");
	PRns("Data Packet Transmit(data_pkt_tx_en)", N, "   %8s    ",
	       lmac[ind].cmr_config.s.data_pkt_tx_en ? " Enabled" : "Disabled");

	cvmx_dprintf("/*=== Flow Control ===*/\n");
	cmr_rx_ovr_bp.u64 = cvmx_read_csr_node(node, CVMX_BGXX_CMR_RX_OVR_BP(bgx));
	PRns("Backpressure Override Enabled(en)", N,
		"       %3s     ",
		(cmr_rx_ovr_bp.s.en & (1<<ind)) ? "Yes" : " No");
	PRn("Transmit BP channel mask(msk:16bits)", N,
	"     0x%04x    ", lmac[ind].cmr_tx_channel.s.msk);
	PRn("Transmit Credit return BP ch. mask(dis:16bits)", N,
	"     0x%04x    ", lmac[ind].cmr_tx_channel.s.dis);
	PRn("BP Status[(O)/LMAC avail, (1)/should be BP](bp)", N,
		"        %d      ", (cmr_rx_ovr_bp.s.bp & (1<<ind)) ? 1 : 0);
	PRns("Ignore RX FIFO BP_ON signal for BP(ign_fifo_bp)", N,
		"       %3s     ",
		(cmr_rx_ovr_bp.s.ign_fifo_bp & (1<<ind)) ? "Yes" : " No");
	PRn("Rx BP Drop level[8 byte cycles](mark)", N, "      %3d      ",
		lmac[ind].cmr_rx_bp_drop.s.mark);
	PRn("Rx BP ON(high) level[bytes](mark)", 	N, "     %5d     ",
		lmac[ind].cmr_rx_bp_on.s.mark<<4);
	PRn("Rx BP OFF(low) level[bytes](mark)", 	N, "     %5d     ",
		lmac[ind].cmr_rx_bp_off.s.mark<<4);
	PRn("Rx RoundRobin WEIGHT(weight)", 	N, "      %3d      ",
		lmac[ind].cmr_rx_weight.s.weight);
	PRns("ADR_CTL: On DMAC CAM address match(cam_accept)",
		N, "    %7s    ",
		lmac[ind].cmr_rx_adr_ctl.s.cam_accept == 0 ? " Reject" : " Accept");
	PRns("ADR_CTL: MCST_MODE(mcst_mode)", N, " %13s ",
		lmac[ind].cmr_rx_adr_ctl.s.mcst_mode == 0 ? "  Reject All " :
		lmac[ind].cmr_rx_adr_ctl.s.mcst_mode == 1 ? "  Accept All " :
		lmac[ind].cmr_rx_adr_ctl.s.mcst_mode == 2 ? "AddrFilterCAM" :
		"     ???     ");
	PRns("ADR_CTL: BCST_ACCEPT(bcst_accept)", N, "    %7s    ",
		lmac[ind].cmr_rx_adr_ctl.s.bcst_accept == 0 ? " Reject" : " Accept");
	PRn("Channel BP: XOFF(cmr_rx_logl_xoff:16bits)", N,
	       "     0x%04x    ", lmac[ind].cmr_rx_logl_xoff.s.xoff);
	PRn("Channel BP: XON (cmr_rx_logl_xon: 16bits)", N,
	       "     0x%04x    ", lmac[ind].cmr_rx_logl_xon.s.xon);
	if (lmac_xaui || lmac_rxaui || lmac_10g_r || lmac_40g_r) {
		/* for all modes, but sgmii, rgmii */
		uint8_t mask_1234 = lmac_xaui | lmac_rxaui | lmac_10g_r | lmac_40g_r;

		PRMn("SMU: Physical BP Enable(phys_en: 16bits)", mask_1234,
		"     0x%04x    ", lmac[ind].smu_cbfc_ctl.s.phys_en);
		PRMn("SMU: Logical  BP Enable(logl_en: 16bits)", mask_1234,
		"     0x%04x    ", lmac[ind].smu_cbfc_ctl.s.logl_en);
		PRMns("SMU: Forward PFC/CBFC PAUSE to BP block(bck_en)", mask_1234,
			"      %3s      ",
			lmac[ind].smu_cbfc_ctl.s.bck_en ? "Yes" : " No");
		PRMns("SMU: Drop PFC/CBFC PAUSE frames(drp_en)", mask_1234,
			"      %3s      ",
			lmac[ind].smu_cbfc_ctl.s.drp_en ? "Yes" : " No");
		PRMns("SMU: Transmit PFC/CBFC PAUSE packets (tx_en)", mask_1234,
			"      %3s      ",
			lmac[ind].smu_cbfc_ctl.s.tx_en ? "Yes" : " No");
		PRMns("SMU: Receive  PFC/CBFC PAUSE packets (rx_en)", mask_1234,
			"      %3s      ",
			lmac[ind].smu_cbfc_ctl.s.rx_en ? "Yes" : " No");
		PRMn("SMU: 40GBASE-R TX marker interval cnt(spu_mrk_cnt)", mask_1234,
			"    %7d    ", lmac[ind].smu_tx_ctl.s.spu_mrk_cnt);

		PRMns("SMU: Gen. PAUSE when CMR do XOFF(l2p_bp_conv)", mask_1234,
			"      %3s      ",
			lmac[ind].smu_tx_ctl.s.l2p_bp_conv ? "Yes" : " No");
		PRMns("SMU: Bypass RX status (set to LS)(ls_byp)", mask_1234,
			"      %3s      ",
			lmac[ind].smu_tx_ctl.s.ls_byp ? "Yes" : " No");
		PRMns("SMU: Unidirectional Mode Enable(uni_en)", mask_1234,
			"   %8s    ",
			lmac[ind].smu_tx_ctl.s.uni_en ? " Enabled" : "Disabled");
		PRMns("SMU: Deficit Idle Counter Enable(dic_en)", mask_1234,
			"   %8s    ",
			lmac[ind].smu_tx_ctl.s.dic_en ? " Enabled" : "Disabled");
		PRMn("SMU: TX Thresh level[128-bits word](cnt)", mask_1234,
			"      %4d     ", lmac[ind].smu_tx_thresh.s.cnt);
		/* HiGig2 config - begin */
		/* it is enabled for all modes now (mask_1234),
		 * but probably must be XAUI only? (lmac_xaui) - wikipedia
		 * NOTE: HiGig2 configuration will be printed only if 'Enabled'
		 */
		PRMns("SMU: HiGig/HiGig2 mode Enabled?(hg_en)", mask_1234,
			"     %6s    ",
			lmac[ind].smu_tx_ctl.s.hg_en == 0 ? "  No   " :
			lmac[ind].smu_rx_udd_skp.s.len == 12 ? "HiGig " : "HiGig2");
		PRMcs("SMU: HiGig2 msg transmission(hg2tx_en)", mask_1234,
			/*cond*/(lmac[ind].smu_tx_ctl.s.hg_en
				&& lmac[ind].smu_rx_udd_skp.s.len == 16),
			"   %8s    ",
			lmac[ind].smu_hg2_control.s.hg2tx_en ? " Enabled" : "Disabled");
		PRMcs("SMU: HiGig2 msg receive/process(hg2rx_en)", mask_1234,
			/*cond*/(lmac[ind].smu_tx_ctl.s.hg_en
				&& lmac[ind].smu_rx_udd_skp.s.len == 16),
			"   %8s    ",
			lmac[ind].smu_hg2_control.s.hg2rx_en ? " Enabled" : "Disabled");
		PRMcs("SMU: HiGig2 Physical-link PAUSE(phys_en)", mask_1234,
			/*cond*/(lmac[ind].smu_tx_ctl.s.hg_en
				&& lmac[ind].smu_rx_udd_skp.s.len == 16),
			"   %8s    ",
			lmac[ind].smu_hg2_control.s.phys_en ? " Enabled" : "Disabled");
		PRMc("SMU: 16-bit XOF enables(logl_en)", mask_1234,
			/*cond*/(lmac[ind].smu_tx_ctl.s.hg_en
				&& lmac[ind].smu_rx_udd_skp.s.len == 16),
			"     0x%04x    ", lmac[ind].smu_hg2_control.s.logl_en);
		/* HiGig2 config - end */
	}
	if (lmac[0/*LMAC0*/].cmr_config.s.lmac_type == 5/*RGMII*/)
		cvmx_helper_bgx_rgmii_config(node, bgx, 1);

	return 0;
}


/*===================== BGX CONFIG Dump func =====================*/
/* The following funcs are implemented in this section */
int cvmx_dump_bgx_status(unsigned bgx);
int cvmx_dump_bgx_status_node(unsigned node, unsigned bgx);

/**
 * Dump status of BGX ports
 */
typedef struct {
	cvmx_bgxx_cmrx_config_t 	cmr_config;
	cvmx_bgxx_cmrx_rx_stat0_t 	rx_stat0;
	cvmx_bgxx_cmrx_rx_stat1_t 	rx_stat1;
	cvmx_bgxx_cmrx_rx_stat2_t 	rx_stat2;
	cvmx_bgxx_cmrx_rx_stat2_t 	rx_stat3;
	cvmx_bgxx_cmrx_rx_stat2_t 	rx_stat4;
	cvmx_bgxx_cmrx_rx_stat2_t 	rx_stat5;
	cvmx_bgxx_cmrx_rx_stat2_t 	rx_stat6;
	cvmx_bgxx_cmrx_rx_stat2_t 	rx_stat7;
	cvmx_bgxx_cmrx_rx_stat8_t 	rx_stat8;
	cvmx_bgxx_cmrx_rx_bp_status_t 	rx_bp_status;
	cvmx_helper_link_info_t 	link_info;
	cvmx_bgxx_cmrx_tx_stat0_t 	tx_stat0;
	cvmx_bgxx_cmrx_tx_stat1_t 	tx_stat1;
	cvmx_bgxx_cmrx_tx_stat2_t 	tx_stat2;
	cvmx_bgxx_cmrx_tx_stat3_t 	tx_stat3;
	cvmx_bgxx_cmrx_tx_stat4_t 	tx_stat4;
	cvmx_bgxx_cmrx_tx_stat5_t 	tx_stat5;
	cvmx_bgxx_cmrx_tx_stat6_t 	tx_stat6;
	cvmx_bgxx_cmrx_tx_stat7_t 	tx_stat7;
	cvmx_bgxx_cmrx_tx_stat8_t 	tx_stat8;
	cvmx_bgxx_cmrx_tx_stat9_t 	tx_stat9;
	cvmx_bgxx_cmrx_tx_stat10_t 	tx_stat10;
	cvmx_bgxx_cmrx_tx_stat11_t 	tx_stat11;
	cvmx_bgxx_cmrx_tx_stat12_t 	tx_stat12;
	cvmx_bgxx_cmrx_tx_stat13_t 	tx_stat13;
	cvmx_bgxx_cmrx_tx_stat14_t 	tx_stat14;
	cvmx_bgxx_cmrx_tx_stat15_t 	tx_stat15;
	cvmx_bgxx_cmrx_tx_stat16_t 	tx_stat16;
	cvmx_bgxx_cmrx_tx_stat17_t 	tx_stat17;
	cvmx_bgxx_cmrx_rx_fifo_len_t 	rx_fifo_len;
	cvmx_bgxx_cmrx_tx_fifo_len_t 	tx_fifo_len;
} lmac_status_t;

int cvmx_dump_bgx_status_node(unsigned node, unsigned bgx)
{
	lmac_status_t lmac[4];
	cvmx_bgxx_cmr_global_config_t global_config;
	unsigned ind, N;
	uint8_t lmac_type;
	uint8_t lmac_sgmii, lmac_rgmii, lmac_xaui, lmac_rxaui, lmac_10g_r, lmac_40g_r;
	int ipd_port, qlm;

	lmac_sgmii = lmac_rgmii = lmac_xaui = lmac_rxaui = lmac_10g_r  = lmac_40g_r = 0;

	global_config.u64 = cvmx_read_csr_node(node,
		CVMX_BGXX_CMR_GLOBAL_CONFIG(bgx));

	cvmx_dprintf("\n/*===== BGX Status report			BGX%d =====*/\n", bgx);

	/* just report configured RX/Tx LMACS - don't check return */
	cvmx_helper_bgx_number_rx_tx_lmacs(node, bgx, &N);

	qlm = bgx<2 ? (global_config.s.pmux_sds_sel==1 ? bgx+2 : bgx) : bgx+2;
	cvmx_dprintf("NODE%d: BGX%d/lmac[0..%d] connected to QLM%d\n",
		node, bgx, N - 1,qlm);

	for (ind = 0; ind < N; ind++) {
		lmac[ind].cmr_config.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_CONFIG(ind, bgx));
		lmac[ind].rx_bp_status.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_RX_BP_STATUS(ind, bgx));
		lmac[ind].rx_stat0.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_RX_STAT0(ind, bgx));
		lmac[ind].rx_stat1.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_RX_STAT1(ind, bgx));
		lmac[ind].rx_stat2.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_RX_STAT2(ind, bgx));
		lmac[ind].rx_stat3.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_RX_STAT3(ind, bgx));
		lmac[ind].rx_stat4.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_RX_STAT4(ind, bgx));
		lmac[ind].rx_stat5.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_RX_STAT5(ind, bgx));
		lmac[ind].rx_stat6.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_RX_STAT6(ind, bgx));
		lmac[ind].rx_stat7.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_RX_STAT7(ind, bgx));
		lmac[ind].rx_stat8.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_RX_STAT8(ind, bgx));
		ipd_port = cvmx_helper_get_ipd_port(bgx, ind);
		lmac[ind].link_info = cvmx_helper_link_get(ipd_port);
		lmac[ind].tx_stat0.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_TX_STAT0(ind, bgx));
		lmac[ind].tx_stat1.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_TX_STAT1(ind, bgx));
		lmac[ind].tx_stat2.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_TX_STAT2(ind, bgx));
		lmac[ind].tx_stat3.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_TX_STAT3(ind, bgx));
		lmac[ind].tx_stat4.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_TX_STAT4(ind, bgx));
		lmac[ind].tx_stat5.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_TX_STAT5(ind, bgx));
		lmac[ind].tx_stat6.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_TX_STAT6(ind, bgx));
		lmac[ind].tx_stat7.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_TX_STAT7(ind, bgx));
		lmac[ind].tx_stat8.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_TX_STAT8(ind, bgx));
		lmac[ind].tx_stat9.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_TX_STAT9(ind, bgx));
		lmac[ind].tx_stat10.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_TX_STAT10(ind, bgx));
		lmac[ind].tx_stat11.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_TX_STAT11(ind, bgx));
		lmac[ind].tx_stat12.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_TX_STAT12(ind, bgx));
		lmac[ind].tx_stat13.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_TX_STAT13(ind, bgx));
		lmac[ind].tx_stat14.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_TX_STAT14(ind, bgx));
		lmac[ind].tx_stat15.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_TX_STAT15(ind, bgx));
		lmac[ind].tx_stat16.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_TX_STAT16(ind, bgx));
		lmac[ind].tx_stat17.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_TX_STAT17(ind, bgx));
		lmac[ind].rx_fifo_len.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_RX_FIFO_LEN(ind, bgx));
		lmac[ind].tx_fifo_len.u64 = cvmx_read_csr_node(node,
			CVMX_BGXX_CMRX_TX_FIFO_LEN(ind, bgx));

		lmac_type = lmac[ind].cmr_config.s.lmac_type;
		lmac_sgmii |= ((lmac_type == 0/*SGMII*/) << ind);
		lmac_xaui  |= ((lmac_type == 1/*XAUI*/)  << ind);
		lmac_rxaui |= ((lmac_type == 2/*RXAUI*/) << ind);
		lmac_10g_r |= ((lmac_type == 3/*10G_R*/) << ind);
		lmac_40g_r |= ((lmac_type == 4/*40G_R*/) << ind);
		lmac_rgmii |= ((lmac_type == 5/*RGMII*/) << ind);
	}

	PRn("LMAC:",			N, "     lmac%d     ", ind);
	PRns("Link Status (from ANEG)[SE API]",N,"      %4s     ",
		lmac[ind].link_info.s.link_up ? " Up " : "Down");
	PRns("Link Duplex [SE API]",		N, "      %4s     ",
		lmac[ind].link_info.s.full_duplex ? "Full" : "Half");
	PRn("Link Speed (MBps) [SE API]",	N, "     %5d     ",
		lmac[ind].link_info.s.speed);
	cvmx_helper_bgx_link_status(node, bgx, N);
	PRn("RX FIFO LEN(fifo_len)", N, "     %5d     ",
	       lmac[ind].rx_fifo_len.s.fifo_len);
	PRn("TX FIFO LEN(fifo_len)", N, "     %5d     ",
	       lmac[ind].tx_fifo_len.s.fifo_len);

	cvmx_dprintf("/*=== Loopback, RX, TX, (PCS) Errors ===*/\n");
	if (lmac_sgmii) { /* SGMII mode */
		cvmx_helper_bgx_gmp_loopback(node, bgx, N);
	}
	if (lmac_sgmii || lmac_rgmii) { /* SGMII or RGMII mode */
		cvmx_helper_bgx_gmp_rx_errors(node, bgx, N);
		cvmx_helper_bgx_gmp_tx_errors(node, bgx, N);
		cvmx_helper_bgx_gmp_pcs_errors(node, bgx, N);
	}
	if (lmac_xaui || lmac_rxaui || lmac_10g_r || lmac_40g_r) {
		/* for all modes, but sgmii, rgmii */
		cvmx_helper_bgx_spu_loopback(node, bgx, N);
		cvmx_helper_bgx_spu_rx_errors(node, bgx, N);
		cvmx_helper_bgx_spu_tx_errors(node, bgx, N);
	}

	if (lmac[0/*LMAC0*/].cmr_config.s.lmac_type == 5/*RGMII*/)
		cvmx_helper_bgx_rgmii_status(node, bgx, 1);

	/*
	 * NOTE: Most of the following lines will be printed only if 'data'!=0
	 * i.e. 'counters' with 'zero' value will be skipped/hiden entirely
	 */
	cvmx_dprintf ("/*=== Rx and TX Statistics - common for all modes ===*/\n");
	PRd("RX0: Received Pakets(cnt)", N, "%14lld ",
	       CAST_ULL(lmac[ind].rx_stat0.s.cnt));
	PRd("RX1: Octets of received packets(cnt)", N, "%14lld ",
	       CAST_ULL(lmac[ind].rx_stat1.s.cnt));
	PRd("RX2: Received FC or PAUSE packets(cnt)", N, "%14lld ",
	       CAST_ULL(lmac[ind].rx_stat2.s.cnt));
	PRd("RX3: FC and PAUSE octs(cnt)",N, "%14lld ",
	       CAST_ULL(lmac[ind].rx_stat3.s.cnt));
	PRd("RX4: Total DMAC pkts(cnt)", 	N, "%14lld ",
	       CAST_ULL(lmac[ind].rx_stat4.s.cnt));
	PRd("RX5: DMAC filter octs(cnt)", N, "%14lld ",
	       CAST_ULL(lmac[ind].rx_stat5.s.cnt));
	PRd("RX6: Full FIFO drop pkts(cnt)",N,"%14lld ",
	       CAST_ULL(lmac[ind].rx_stat6.s.cnt));
	PRd("RX7: Full FIFO drop octs(cnt)",N,"%14lld ",
	       CAST_ULL(lmac[ind].rx_stat7.s.cnt));
	PRn("RX8: Total Errors(cnt)",	N, "%14lld ",
	       CAST_ULL(lmac[ind].rx_stat8.s.cnt));
	PRn("RX Backpressure[cnt](bp)",	N, "%14lld ",
	       CAST_ULL(lmac[ind].rx_bp_status.s.bp));
	PRd("TX0: Packets dropped - excess. collisions(xscol)", N, "%14lld ",
	       CAST_ULL(lmac[ind].tx_stat0.s.xscol));
	PRd("TX1: Packets dropped - excessive deferral(xsdef)", N, "%14lld ",
	       CAST_ULL(lmac[ind].tx_stat1.s.xsdef));
	PRd("TX2: Multi Colissions packets(mcol)", N,"%14lld ",
	       CAST_ULL(lmac[ind].tx_stat2.s.mcol));
	PRd("TX3: Single Collision packets(scol)", N, "%14lld ",
	       CAST_ULL(lmac[ind].tx_stat3.s.scol));
	PRd("TX4: Total sent octets(octs)", N, "%14lld ",
	       CAST_ULL(lmac[ind].tx_stat4.s.octs));
	PRd("TX5: Total sent packets(pkts)", N, "%14lld ",
	       CAST_ULL(lmac[ind].tx_stat5.s.pkts));
	PRd("TX6: Packets sent with octet count < 64", N, "%14lld ",
		CAST_ULL(lmac[ind].tx_stat6.s.hist0));
	PRd("TX7: Packets sent with octet count == 64", N, "%14lld ",
		CAST_ULL(lmac[ind].tx_stat7.s.hist1));
	PRd("TX8: Packets sent with octet count 64-127", N, "%14lld ",
		CAST_ULL(lmac[ind].tx_stat8.s.hist2));
	PRd("TX9: Packets sent with octet count 128-255", N, "%14lld ",
		CAST_ULL(lmac[ind].tx_stat9.s.hist3));
	PRd("TX10: Packets sent with octet count 256-511", N, "%14lld ",
		CAST_ULL(lmac[ind].tx_stat10.s.hist4));
	PRd("TX11: Packets sent with octet count 512-1023", N, "%14lld ",
		CAST_ULL(lmac[ind].tx_stat11.s.hist5));
	PRd("TX12: Packets sent with octet count 1024-1518", N, "%14lld ",
		CAST_ULL(lmac[ind].tx_stat12.s.hist6));
	PRd("TX13: Packets sent with octet count > 1518", N, "%14lld ",
		CAST_ULL(lmac[ind].tx_stat13.s.hist7));
	PRd("TX14: Packets sent to broadcast DMAC(bcst)", N, "%14lld ",
		CAST_ULL(lmac[ind].tx_stat14.s.bcst));
	PRd("TX15: Packets sent to multicast DMAC(mcst)", N, "%14lld ",
		CAST_ULL(lmac[ind].tx_stat15.s.mcst));
	PRd("TX16: Number of underflow packets (undflw)", N, "%14lld ",
		CAST_ULL(lmac[ind].tx_stat16.s.undflw));
	PRd("TX17: Number of BGX gen. PAUSE/PFC ctl pkts(ctl)", N, "%14lld ",
		CAST_ULL(lmac[ind].tx_stat17.s.ctl));

	return 0;
}

int cvmx_dump_bgx_status(unsigned bgx)
{
	return cvmx_dump_bgx_status_node(0, bgx);
}

int cvmx_dump_bgx_config(unsigned bgx)
{
	return cvmx_dump_bgx_config_node(0, bgx);
}

#endif
