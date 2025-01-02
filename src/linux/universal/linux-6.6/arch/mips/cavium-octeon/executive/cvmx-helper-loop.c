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
 * Functions for LOOP initialization, configuration,
 * and monitoring.
 *
 * <hr>$Revision: 115656 $<hr>
 */
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-helper.h>
#include <asm/octeon/cvmx-pip-defs.h>
#include <asm/octeon/cvmx-pko-defs.h>
#include <asm/octeon/cvmx-lbk-defs.h>
#include <asm/octeon/cvmx-pki.h>
#else
#include "cvmx.h"
#include "cvmx-helper.h"
#include "cvmx-pki.h"
#endif

int __cvmx_helper_loop_enumerate(int xiface)
{
	return OCTEON_IS_MODEL(OCTEON_CN68XX) ? 8 : (OCTEON_IS_MODEL(OCTEON_CNF71XX) ? 2 : 4);
}

/**
 * @INTERNAL
 * Probe a LOOP interface and determine the number of ports
 * connected to it. The LOOP interface should still be down
 * after this call.
 *
 * @param xiface Interface to probe
 *
 * @return Number of ports on the interface. Zero to disable.
 */
int __cvmx_helper_loop_probe(int xiface)
{
	return __cvmx_helper_loop_enumerate(xiface);
}

/**
 * @INTERNAL
 * Bringup and enable a LOOP interface. After this call packet
 * I/O should be fully functional. This is called with IPD
 * enabled but PKO disabled.
 *
 * @param interface Interface to bring up
 *
 * @return Zero on success, negative on failure
 */
int __cvmx_helper_loop_enable(int xiface)
{
	cvmx_pip_prt_cfgx_t port_cfg;
	int num_ports, index;
	unsigned long offset;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	num_ports = __cvmx_helper_get_num_ipd_ports(xiface);
	/*
	 * We need to disable length checking so packet < 64 bytes and jumbo
	 * frames don't get errors
	 */
	for (index = 0; index < num_ports; index++) {
		offset = ((octeon_has_feature(OCTEON_FEATURE_PKND)) ? cvmx_helper_get_pknd(xiface, index) : cvmx_helper_get_ipd_port(xiface, index));

		if (octeon_has_feature(OCTEON_FEATURE_PKI)) {
			cvmx_pki_endis_l2_errs(xi.node, offset, 1, 0, 0);
                        cvmx_pki_endis_fcs_check(xi.node, offset, 0, 0);
		} else {
			port_cfg.u64 = cvmx_read_csr(CVMX_PIP_PRT_CFGX(offset));
			port_cfg.s.maxerr_en = 0;
			port_cfg.s.minerr_en = 0;
			cvmx_write_csr(CVMX_PIP_PRT_CFGX(offset), port_cfg.u64);
		}
	}

	/*
	 * Disable FCS stripping for loopback ports
	 */
	if (!octeon_has_feature(OCTEON_FEATURE_PKND)) {
		cvmx_ipd_sub_port_fcs_t ipd_sub_port_fcs;
		ipd_sub_port_fcs.u64 = cvmx_read_csr(CVMX_IPD_SUB_PORT_FCS);
		ipd_sub_port_fcs.s.port_bit2 = 0;
		cvmx_write_csr(CVMX_IPD_SUB_PORT_FCS, ipd_sub_port_fcs.u64);
	}
	/*
 	 * Set PKND and BPID for loopback ports.
 	 */
	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		cvmx_pko_reg_loopback_pkind_t lp_pknd;
		cvmx_pko_reg_loopback_bpid_t lp_bpid;

		for (index = 0; index < num_ports; index++) {
			int pknd = cvmx_helper_get_pknd(xiface, index);
			int bpid = cvmx_helper_get_bpid(xiface, index);

			lp_pknd.u64 = cvmx_read_csr(CVMX_PKO_REG_LOOPBACK_PKIND);
			lp_bpid.u64 = cvmx_read_csr(CVMX_PKO_REG_LOOPBACK_BPID);

			if (index == 0)
				lp_pknd.s.num_ports = num_ports;

			switch(index) {
			case 0:
				lp_pknd.s.pkind0 = pknd;
				lp_bpid.s.bpid0 = bpid;
				break;
			case 1:
				lp_pknd.s.pkind1 = pknd;
				lp_bpid.s.bpid1 = bpid;
				break;
			case 2:
				lp_pknd.s.pkind2 = pknd;
				lp_bpid.s.bpid2 = bpid;
				break;
			case 3:
				lp_pknd.s.pkind3 = pknd;
				lp_bpid.s.bpid3 = bpid;
				break;
			case 4:
				lp_pknd.s.pkind4 = pknd;
				lp_bpid.s.bpid4 = bpid;
				break;
			case 5:
				lp_pknd.s.pkind5 = pknd;
				lp_bpid.s.bpid5 = bpid;
				break;
			case 6:
				lp_pknd.s.pkind6 = pknd;
				lp_bpid.s.bpid6 = bpid;
				break;
			case 7:
				lp_pknd.s.pkind7 = pknd;
				lp_bpid.s.bpid7 = bpid;
				break;
			}
			cvmx_write_csr(CVMX_PKO_REG_LOOPBACK_PKIND, lp_pknd.u64);
			cvmx_write_csr(CVMX_PKO_REG_LOOPBACK_BPID, lp_bpid.u64);
		}
	} else if (octeon_has_feature(OCTEON_FEATURE_BGX)) {
		cvmx_lbk_chx_pkind_t lbk_pkind;

		for (index = 0; index < num_ports; index++) {
			lbk_pkind.u64 = 0;
			lbk_pkind.s.pkind = cvmx_helper_get_pknd(xiface, index);
			cvmx_write_csr_node(xi.node, CVMX_LBK_CHX_PKIND(index), lbk_pkind.u64);
		}
	}

	return 0;
}

