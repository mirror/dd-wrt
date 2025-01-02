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
 * Functions for SRIO initialization, configuration,
 * and monitoring.
 *
 * <hr>$Revision: 41586 $<hr>
 */
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-clock.h>
#include <asm/octeon/cvmx-helper.h>
#include <asm/octeon/cvmx-qlm.h>
#include <asm/octeon/cvmx-srio.h>
#include <asm/octeon/cvmx-pip-defs.h>
#include <asm/octeon/cvmx-sriox-defs.h>
#include <asm/octeon/cvmx-sriomaintx-defs.h>
#include <asm/octeon/cvmx-dpi-defs.h>
#include <asm/octeon/cvmx-pki.h>
#else

#include "cvmx.h"
#include "cvmx-helper.h"
#include "cvmx-srio.h"
#include "cvmx-qlm.h"
#include "cvmx-pki.h"
#endif

static const int debug = 0;

/**
 * @INTERNAL
 * Convert interface number to sRIO link number
 * per SoC model.
 *
 * @param xiface Interface to convert
 *
 * @return Srio link number
 */
int __cvmx_helper_srio_port(int xiface)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int  srio_port = -1;

	if (!octeon_has_feature(OCTEON_FEATURE_SRIO))
		return -1;

	if (xi.node != 0)
		return -1;

	if (OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
		srio_port = xi.interface - 1;
		if (srio_port > 1)
			srio_port = -1;
	} else if (OCTEON_IS_MODEL(OCTEON_CN66XX)) {
		srio_port = xi.interface - 4;
		if (srio_port > 3 || srio_port == 1)
			srio_port = -1;
	} else {
		srio_port = xi.interface - 4;
		if (srio_port > 1)
			srio_port = -1;
	}

	return srio_port;
}

/**
 * @INTERNAL
 * Probe a SRIO interface and determine the number of ports
 * connected to it. The SRIO interface should still be down
 * after this call.
 *
 * @param xiface Interface to probe
 *
 * @return Number of ports on the interface. Zero to disable.
 */
int __cvmx_helper_srio_probe(int xiface)
{
	cvmx_sriox_status_reg_t srio_status_reg;
	int srio_port;

	if (!octeon_has_feature(OCTEON_FEATURE_SRIO))
		return 0;

	srio_port = __cvmx_helper_srio_port(xiface);
	if (srio_port < 0)
		return 0;

	/* Read MIO_QLMX_CFG CSRs to find SRIO mode. */
	if (OCTEON_IS_MODEL(OCTEON_CN66XX)) {
		/* QLM0 exclusively used by sRIO */
		enum cvmx_qlm_mode mode = cvmx_qlm_get_mode(0);
		switch (srio_port) {
		case 0:	/* 1x4 lane */
			if (mode == CVMX_QLM_MODE_SRIO_1X4 ||
			    mode == CVMX_QLM_MODE_SRIO_2X2 ||
			    mode == CVMX_QLM_MODE_SRIO_4X1)
				return 2;
			break;
		case 2:	/* 2x2 lane */
			if (mode == CVMX_QLM_MODE_SRIO_2X2 ||
			    mode == CVMX_QLM_MODE_SRIO_4X1)
				return 2;
			break;
		case 3:	/* 4x1 long/short */
			if (mode == CVMX_QLM_MODE_SRIO_4X1)
				return 2;
			break;
		/* Note: srio_port 1 does not exist! */
		default:
			break;
		}
		return 0;
	} else if (OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
		srio_status_reg.u64 =
			cvmx_read_csr(CVMX_SRIOX_STATUS_REG(srio_port));
		if (srio_status_reg.s.srio)
			return 2;
		else
			return 0;
	}

	srio_status_reg.u64 = cvmx_read_csr(CVMX_SRIOX_STATUS_REG(srio_port));
	if (srio_status_reg.s.srio)
		return 2;
	else
		return 0;
}

/**
 * @INTERNAL
 * Bringup and enable SRIO interface. After this call packet
 * I/O should be fully functional. This is called with IPD
 * enabled but PKO disabled.
 *
 * @param xiface Interface to bring up
 *
 * @return Zero on success, negative on failure
 */
int __cvmx_helper_srio_enable(int xiface)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int num_ports = cvmx_helper_ports_on_interface(xi.interface);
	int index;
	cvmx_sriomaintx_core_enables_t sriomaintx_core_enables;
	cvmx_sriox_imsg_ctrl_t sriox_imsg_ctrl;
	cvmx_sriox_status_reg_t srio_status_reg;
	cvmx_dpi_ctl_t dpi_ctl;
	int srio_port;

	srio_port = __cvmx_helper_srio_port(xiface);
	if (srio_port < 0)
		return -1;

	if (debug)
		cvmx_dprintf("%s: interface %d SRIO%d ports %d\n",
			__func__, xi.interface, srio_port, num_ports);

	/* Make sure register access is allowed */
	srio_status_reg.u64 = cvmx_read_csr(CVMX_SRIOX_STATUS_REG(srio_port));

	if (debug)
		cvmx_dprintf("%s: SRIO%d srio_status_reg=%#llx\n",
			__func__, srio_port, CAST_ULL(srio_status_reg.u64));

	if (!srio_status_reg.s.access) {
		cvmx_printf("WARNING: %s: SRIO%d is not enabled\n",
			__func__, srio_port);
		return -1;
	}

	/* All SRIO ports have a cvmx_srio_rx_message_header_t header
	   on them that must be skipped by IPD */
	for (index = 0; index < num_ports; index++) {
		cvmx_sriox_omsg_portx_t sriox_omsg_portx;
		cvmx_sriox_omsg_sp_mrx_t sriox_omsg_sp_mrx;
		cvmx_sriox_omsg_fmp_mrx_t sriox_omsg_fmp_mrx;
		cvmx_sriox_omsg_nmp_mrx_t sriox_omsg_nmp_mrx;

		if (!octeon_has_feature(OCTEON_FEATURE_PKI)) {
			int ipd_port = cvmx_helper_get_ipd_port(xiface, index);
			cvmx_pip_prt_cfgx_t port_config;
			port_config.u64 =
				cvmx_read_csr(CVMX_PIP_PRT_CFGX(ipd_port));
			/* Only change the skip if
			 * the user hasn't already set it
			 */
			if (!port_config.s.skip) {
				port_config.s.skip =
					sizeof(cvmx_srio_rx_message_header_t);
				cvmx_write_csr(CVMX_PIP_PRT_CFGX(ipd_port),
					port_config.u64);
			}
		} else {
			bool use_inst_hdr = false;
			int pknd, style, i, fcs;
			struct cvmx_pki_pkind_config pkind_cfg;
			struct cvmx_pki_style_config style_cfg;
			struct cvmx_pki_qpg_config qpg_cfg;

			fcs = __cvmx_helper_get_has_fcs(xiface);

			/* Initialize sRIO INST_HDR_S registers */
			// FIXME Move this code to cvmx-srio.c !!
			if (use_inst_hdr)
			    for(i = 0; i < 256 && index == 0; i++) {
				cvmx_sriox_imsg_inst_hdrx_t inst_hdr;
				inst_hdr.u64 = (1ull << 63) |(1ull << 47) |
					(0x1a91a9 << 8) | i ; // only tag
				cvmx_write_csr(
					CVMX_SRIOX_IMSG_INST_HDRX(i, srio_port),
					 inst_hdr.u64);
			}

			/* Find out the intended PKIND for this port */
			pknd = cvmx_helper_get_pknd(xiface, index);

			/* Modify PKIND configuration */
			cvmx_pki_read_pkind_config(0, pknd, &pkind_cfg);
			pkind_cfg.initial_parse_mode = CVMX_PKI_PARSE_LA_TO_LG;

			/* 16 bytes contain SRIO header and INST_HDR_S data */
			pkind_cfg.fcs_skip = 16;
			pkind_cfg.inst_skip = 16;

			/* Use this to honor the INST_HDR_S from sRIO regs */
			if (use_inst_hdr) {
				pkind_cfg.inst_skip = 8;
				pkind_cfg.parse_en.inst_hdr = 1;
			}
			pkind_cfg.fcs_pres = fcs;

			cvmx_pki_write_pkind_config(0, pknd, &pkind_cfg);

			style = cvmx_pki_get_pkind_style(0, pknd);

			if (debug)
				cvmx_dprintf("%s: configuring PKI style %d\n",
					__func__, style);
			/* Customize style */
			cvmx_pki_read_style_config(0, style,
				CVMX_PKI_CLUSTER_ALL, &style_cfg);

			/* FIXME
			 * the following lines are only meant
			 * to mitigate bugs in the PKI config
			 * code, and should be redundant once
			 * these bugs get fixed.
			 */
			if (debug)
				cvmx_dprintf("%s: Style %d FCS check %d strip %d\n",
					__func__, style,
					style_cfg.parm_cfg.fcs_chk,
					style_cfg.parm_cfg.fcs_strip);

			/* do not include port# in QPG index calculation */
			style_cfg.parm_cfg.qpg_port_msb = 0;

			/* Force FCS checking off for sRIO */
			style_cfg.parm_cfg.fcs_chk = fcs;
			style_cfg.parm_cfg.fcs_strip = fcs;

			cvmx_pki_write_style_config(0, style,
				CVMX_PKI_CLUSTER_ALL, &style_cfg);
			if (debug)
				cvmx_dprintf(" Style qpg_base = %d, "
				"dis_padd %d, dis_aura %d\n",
				 style_cfg.parm_cfg.qpg_base,
				 style_cfg.parm_cfg.qpg_dis_padd,
				 style_cfg.parm_cfg.qpg_dis_aura);

			cvmx_pki_read_qpg_entry(0,
				style_cfg.parm_cfg.qpg_base, &qpg_cfg);
			if (debug)
				cvmx_dprintf(" Aura=%d\n", qpg_cfg.aura_num);

			/* Set PKIND in SRIO IMSG register */
			if (debug)
				cvmx_dprintf("%s: setting PKIND=%d\n",
				__func__, pknd);
			cvmx_srio_set_pkind(srio_port, index, pknd);
		}

		/* Enable TX with PKO */
		sriox_omsg_portx.u64 = cvmx_read_csr(CVMX_SRIOX_OMSG_PORTX(index, srio_port));
		if (OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
			sriox_omsg_portx.cnf75xx.enable = 1;
		} else {
			sriox_omsg_portx.s.port = (srio_port) * 2 + index;
			sriox_omsg_portx.s.enable = 1;
		}
		cvmx_write_csr(CVMX_SRIOX_OMSG_PORTX(index, srio_port), sriox_omsg_portx.u64);

		/* Allow OMSG controller to send regardless of the state of any other
		   controller. Allow messages to different IDs and MBOXes to go in
		   parallel */
		sriox_omsg_sp_mrx.u64 = 0;
		sriox_omsg_sp_mrx.s.xmbox_sp = 1;
		sriox_omsg_sp_mrx.s.ctlr_sp = 1;
		sriox_omsg_sp_mrx.s.ctlr_fmp = 1;
		sriox_omsg_sp_mrx.s.ctlr_nmp = 1;
		sriox_omsg_sp_mrx.s.id_sp = 1;
		sriox_omsg_sp_mrx.s.id_fmp = 1;
		sriox_omsg_sp_mrx.s.id_nmp = 1;
		sriox_omsg_sp_mrx.s.mbox_sp = 1;
		sriox_omsg_sp_mrx.s.mbox_fmp = 1;
		sriox_omsg_sp_mrx.s.mbox_nmp = 1;
		sriox_omsg_sp_mrx.s.all_psd = 1;
		cvmx_write_csr(CVMX_SRIOX_OMSG_SP_MRX(index, srio_port), sriox_omsg_sp_mrx.u64);

		/* Allow OMSG controller to send regardless of the state of any other
		   controller. Allow messages to different IDs and MBOXes to go in
		   parallel */
		sriox_omsg_fmp_mrx.u64 = 0;
		sriox_omsg_fmp_mrx.s.ctlr_sp = 1;
		sriox_omsg_fmp_mrx.s.ctlr_fmp = 1;
		sriox_omsg_fmp_mrx.s.ctlr_nmp = 1;
		sriox_omsg_fmp_mrx.s.id_sp = 1;
		sriox_omsg_fmp_mrx.s.id_fmp = 1;
		sriox_omsg_fmp_mrx.s.id_nmp = 1;
		sriox_omsg_fmp_mrx.s.mbox_sp = 1;
		sriox_omsg_fmp_mrx.s.mbox_fmp = 1;
		sriox_omsg_fmp_mrx.s.mbox_nmp = 1;
		sriox_omsg_fmp_mrx.s.all_psd = 1;
		cvmx_write_csr(CVMX_SRIOX_OMSG_FMP_MRX(index, srio_port), sriox_omsg_fmp_mrx.u64);

		/* Once the first part of a message is accepted, always acept the rest
		   of the message */
		sriox_omsg_nmp_mrx.u64 = 0;
		sriox_omsg_nmp_mrx.s.all_sp = 1;
		sriox_omsg_nmp_mrx.s.all_fmp = 1;
		sriox_omsg_nmp_mrx.s.all_nmp = 1;
		cvmx_write_csr(CVMX_SRIOX_OMSG_NMP_MRX(index, srio_port), sriox_omsg_nmp_mrx.u64);

	}

	/* Choose the receive controller based on the mailbox */
	sriox_imsg_ctrl.u64 = cvmx_read_csr(CVMX_SRIOX_IMSG_CTRL(srio_port));
	sriox_imsg_ctrl.s.prt_sel = 0;
	sriox_imsg_ctrl.s.mbox = 0xa;
	cvmx_write_csr(CVMX_SRIOX_IMSG_CTRL(srio_port), sriox_imsg_ctrl.u64);

	/* DPI must be enabled for us to RX messages */
	/* FIXME: This condition is questionable */
	dpi_ctl.u64 = cvmx_read_csr(CVMX_DPI_CTL);
	if (!OCTEON_IS_MODEL(OCTEON_CNF75XX))
		dpi_ctl.s.clk = 1;
	dpi_ctl.s.en = 1;
	cvmx_write_csr(CVMX_DPI_CTL, dpi_ctl.u64);

	/* Enable RX */
	if (debug)
		cvmx_dprintf("%s: SRIO%u enabling RX via maint regs\n",
			__func__, srio_port);

	if (!cvmx_srio_config_read32(srio_port, 0, -1, 0, 0, CVMX_SRIOMAINTX_CORE_ENABLES(srio_port), &sriomaintx_core_enables.u32)) {
		sriomaintx_core_enables.s.imsg0 = 1;
		sriomaintx_core_enables.s.imsg1 = 1;
		cvmx_srio_config_write32(srio_port, 0, -1, 0, 0, CVMX_SRIOMAINTX_CORE_ENABLES(srio_port), sriomaintx_core_enables.u32);
	}

	if (debug)
		cvmx_dprintf("%s: SRIO%u packets enabled\n",
			__func__, srio_port);
	return 0;
}
EXPORT_SYMBOL(__cvmx_helper_srio_enable);

/**
 * @INTERNAL
 * Return the link state of an IPD/PKO port as returned by SRIO link status.
 *
 * @param ipd_port IPD/PKO port to query
 *
 * @return Link state
 */
cvmx_helper_link_info_t __cvmx_helper_srio_link_get(int ipd_port)
{
	int xiface = cvmx_helper_get_interface_num(ipd_port);
	int srio_port;
	cvmx_helper_link_info_t result;
	cvmx_sriox_status_reg_t srio_status_reg;
	cvmx_sriomaintx_port_0_err_stat_t sriomaintx_port_0_err_stat;
	cvmx_sriomaintx_port_0_ctl_t sriomaintx_port_0_ctl;
	cvmx_sriomaintx_port_0_ctl2_t sriomaintx_port_0_ctl2;

	result.u64 = 0;
	srio_port = __cvmx_helper_srio_port(xiface);
	if (srio_port < 0)
		return result;

	/* Make sure register access is allowed */
	srio_status_reg.u64 = cvmx_read_csr(CVMX_SRIOX_STATUS_REG(srio_port));
	if (!srio_status_reg.s.access)
		return result;

	/* Read the port link status */
	if (cvmx_srio_config_read32(srio_port, 0, -1, 0, 0, CVMX_SRIOMAINTX_PORT_0_ERR_STAT(srio_port), &sriomaintx_port_0_err_stat.u32))
		return result;

	/* Return if link is down */
	if (!sriomaintx_port_0_err_stat.s.pt_ok)
		return result;

	/* Read the port link width and speed */
	if (cvmx_srio_config_read32(srio_port, 0, -1, 0, 0, CVMX_SRIOMAINTX_PORT_0_CTL(srio_port), &sriomaintx_port_0_ctl.u32))
		return result;
	if (cvmx_srio_config_read32(srio_port, 0, -1, 0, 0, CVMX_SRIOMAINTX_PORT_0_CTL2(srio_port), &sriomaintx_port_0_ctl2.u32))
		return result;

	/* Link is up */
	result.s.full_duplex = 1;
	result.s.link_up = 1;
	switch (sriomaintx_port_0_ctl2.s.sel_baud) {
	case 1:
		result.s.speed = 1250;
		break;
	case 2:
		result.s.speed = 2500;
		break;
	case 3:
		result.s.speed = 3125;
		break;
	case 4:
		result.s.speed = 5000;
		break;
	case 5:
		result.s.speed = 6250;
		break;
	default:
		result.s.speed = 0;
		break;
	}
	switch (sriomaintx_port_0_ctl.s.it_width) {
	case 2:		/* Four lanes */
		result.s.speed *= 4;
		break;
	case 3:		/* Two lanes */
		result.s.speed *= 2;
		break;
	}
	return result;
}

/**
 * @INTERNAL
 * Configure an IPD/PKO port for the specified link state. This
 * function does not influence auto negotiation at the PHY level.
 * The passed link state must always match the link state returned
 * by cvmx_helper_link_get(). It is normally best to use
 * cvmx_helper_link_autoconf() instead.
 *
 * @param ipd_port  IPD/PKO port to configure
 * @param link_info The new link state
 *
 * @return Zero on success, negative on failure
 */
int __cvmx_helper_srio_link_set(int ipd_port, cvmx_helper_link_info_t link_info)
{
	/* FIXME: Add support for changing baud rate, reduce lanes, etc. */
	return 0;
}

