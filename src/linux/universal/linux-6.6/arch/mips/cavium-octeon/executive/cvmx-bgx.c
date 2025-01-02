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
 * Functions to configure the BGX MAC.
 *
 * <hr>$Revision$<hr>
 */
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-helper.h>
#include <asm/octeon/cvmx-qlm.h>
#include <asm/octeon/cvmx-bgxx-defs.h>
#else
#include "cvmx.h"
#include "cvmx-helper.h"
#include "cvmx-qlm.h"
#endif


/**
 * @INTERNAL
 * Configure the bgx mac for xaui.
 *
 * @param interface Interface to initialize
 *
 * @param num_ports Number of ports on interface
 *
 * @return Zero on success, negative on failure
 */
static int bgx_xaui_init(int 	interface,
			 int	num_ports)
{
	cvmx_bgxx_smux_tx_thresh_t	smu_tx_thresh;
	cvmx_bgxx_spux_control1_t	spu_control1;
	cvmx_bgxx_spux_misc_control_t	spu_misc_control;
	cvmx_bgxx_cmrx_config_t		cmr_config;
	int				val;
	int				i;

	smu_tx_thresh.u64 = 0;
	smu_tx_thresh.s.cnt = 0x30;
	cvmx_write_csr(CVMX_BGXX_SMUX_TX_THRESH(0, interface),
		       smu_tx_thresh.u64);

	spu_control1.u64 = cvmx_read_csr(CVMX_BGXX_SPUX_CONTROL1(0, interface));
	spu_control1.s.lo_pwr = 0;
	cvmx_write_csr(CVMX_BGXX_SPUX_CONTROL1(0, interface), spu_control1.u64);

	spu_misc_control.u64 =
		cvmx_read_csr(CVMX_BGXX_SPUX_MISC_CONTROL(0, interface));
	spu_misc_control.s.rx_packet_dis = 0;
	cvmx_write_csr(CVMX_BGXX_SPUX_MISC_CONTROL(0, interface),
		       spu_misc_control.u64);

	/* Check if interface 0 or 1 must be routed to the mix */
	if ((interface == 0 && MUX_78XX_IFACE0) ||
	    (interface == 1 && MUX_78XX_IFACE1))
		val = 1;
	else
		val = 0;

	/* Enable bgx */
	for (i = 0; i < num_ports; i++) {
		cmr_config.u64 =
			cvmx_read_csr(CVMX_BGXX_CMRX_CONFIG(i, interface));
		cmr_config.s.lmac_type = 1;
		cmr_config.s.mix_en = val;
		cmr_config.s.lane_to_sds = 0xe4;
		cmr_config.s.data_pkt_rx_en = 1;
		cmr_config.s.data_pkt_tx_en = 1;
		cmr_config.s.enable = 1;
		cvmx_write_csr(CVMX_BGXX_CMRX_CONFIG(i, interface),
			       cmr_config.u64);
	}

	return 0;
}

/**
 * @INTERNAL
 * Configure the bgx mac for sgmii.
 *
 * @param interface Interface to initialize
 *
 * @param num_ports Number of ports on interface
 *
 * @return Zero on success, negative on failure
 */
static int bgx_sgmii_init(int 	interface,
			  int	num_ports)
{
	cvmx_bgxx_gmp_gmi_txx_thresh_t	gmp_gmi_tx_thresh;
	cvmx_bgxx_gmp_gmi_prtx_cfg_t	gmp_gmi_prt_cfg;
	cvmx_bgxx_gmp_pcs_mrx_control_t	gmp_pcs_mr_control;
	cvmx_bgxx_gmp_pcs_miscx_ctl_t	gmp_pcs_misc_ctl;
	cvmx_bgxx_cmrx_config_t		cmr_config;
	int				val;
	int				i;

	for (i = 0; i < num_ports; i++) {
		/* Configure gmp */
		gmp_gmi_tx_thresh.u64 = 0;
		gmp_gmi_tx_thresh.s.cnt = 0x30;
		cvmx_write_csr(CVMX_BGXX_GMP_GMI_TXX_THRESH(i, interface),
			       gmp_gmi_tx_thresh.u64);

		gmp_gmi_prt_cfg.u64 = 0;
		gmp_gmi_prt_cfg.s.speed = 1;
		gmp_gmi_prt_cfg.s.speed_msb = 0;
		gmp_gmi_prt_cfg.s.duplex = 1;
		gmp_gmi_prt_cfg.s.slottime = 1;
		cvmx_write_csr(CVMX_BGXX_GMP_GMI_PRTX_CFG(i, interface),
			       gmp_gmi_prt_cfg.u64);

		gmp_pcs_mr_control.u64 = 
			cvmx_read_csr(CVMX_BGXX_GMP_PCS_MRX_CONTROL(i, interface));
		gmp_pcs_mr_control.s.pwr_dn = 0;
		cvmx_write_csr(CVMX_BGXX_GMP_PCS_MRX_CONTROL(i, interface),
			       gmp_pcs_mr_control.u64);

		gmp_pcs_misc_ctl.u64 = 
			cvmx_read_csr(CVMX_BGXX_GMP_PCS_MISCX_CTL(i, interface));
		gmp_pcs_misc_ctl.s.gmxeno = 0;
		gmp_pcs_misc_ctl.s.mac_phy = 0;
		gmp_pcs_misc_ctl.s.mode = 0;
		cvmx_write_csr(CVMX_BGXX_GMP_PCS_MISCX_CTL(i, interface),
			       gmp_pcs_misc_ctl.u64);
	}

	/* Check if interface 0 or 1 must be routed to the mix */
	if ((interface == 0 && MUX_78XX_IFACE0) ||
	    (interface == 1 && MUX_78XX_IFACE1))
		val = 1;
	else
		val = 0;

	/* Enable bgx */
	for (i = 0; i < num_ports; i++) {
		cmr_config.u64 =
			cvmx_read_csr(CVMX_BGXX_CMRX_CONFIG(i, interface));
		cmr_config.s.lmac_type = 0;
		cmr_config.s.mix_en = val;
		cmr_config.s.lane_to_sds = i;
		cmr_config.s.data_pkt_rx_en = 1;
		cmr_config.s.data_pkt_tx_en = 1;
		cmr_config.s.enable = 1;
		cvmx_write_csr(CVMX_BGXX_CMRX_CONFIG(i, interface),
			       cmr_config.u64);
	}

	return 0;
}

/**
 * @INTERNAL
 * Configure the bgx mac.
 *
 * @param interface Interface to bring up
 *
 * @param mode      Mode to configure the bgx mac as
 *
 * @return Zero on success, negative on failure
 */
int bgx_init(int				interface,
	     cvmx_helper_interface_mode_t	mode)
{
	cvmx_bgxx_cmrx_config_t		cmr_config;
	cvmx_bgxx_cmr_rx_lmacs_t	bgx_cmr_rx_lmacs;
	cvmx_bgxx_cmr_tx_lmacs_t	bgx_cmr_tx_lmacs;
	cvmx_bgxx_cmr_global_config_t	bgx_cmr_global_config;
	int				num_ports;
	int				val;
	int				i;

	num_ports = cvmx_helper_ports_on_interface(interface);

	/* Disable cmr for all interface ports */
	for (i = 0; i < num_ports; i++) {
		cmr_config.u64 =
			cvmx_read_csr(CVMX_BGXX_CMRX_CONFIG(i, interface));
		cmr_config.s.enable = 0;
		cvmx_write_csr(CVMX_BGXX_CMRX_CONFIG(i, interface),
			       cmr_config.u64);
	}

	bgx_cmr_rx_lmacs.u64 = 0;
	bgx_cmr_rx_lmacs.s.lmacs = num_ports;
	cvmx_write_csr(CVMX_BGXX_CMR_RX_LMACS(interface),
		       bgx_cmr_rx_lmacs.u64);

	bgx_cmr_tx_lmacs.u64 = 0;
	bgx_cmr_tx_lmacs.s.lmacs = num_ports;
	cvmx_write_csr(CVMX_BGXX_CMR_TX_LMACS(interface),
		       bgx_cmr_tx_lmacs.u64);

	/* Check if interface 0 or 1 must be routed to the mix */
	if ((interface == 0 && MUX_78XX_IFACE0) ||
	    (interface == 1 && MUX_78XX_IFACE1))
		val = 1;
	else
		val = 0;

	bgx_cmr_global_config.u64 = 0;
	bgx_cmr_global_config.s.pmux_sds_sel = val;
	cvmx_write_csr(CVMX_BGXX_CMR_GLOBAL_CONFIG(interface),
		       bgx_cmr_global_config.u64);

	if (mode == CVMX_HELPER_INTERFACE_MODE_XAUI)
		bgx_xaui_init(interface, num_ports);
	else if (mode == CVMX_HELPER_INTERFACE_MODE_SGMII)
		bgx_sgmii_init(interface, num_ports);
	else
		return -1;

	return 0;
}
