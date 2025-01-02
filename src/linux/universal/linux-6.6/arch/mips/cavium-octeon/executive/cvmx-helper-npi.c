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
 * Functions for NPI initialization, configuration,
 * and monitoring.
 *
 * <hr>$Revision: 120569 $<hr>
 */
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-helper.h>
#include <asm/octeon/cvmx-hwpko.h>
#include <asm/octeon/cvmx-pexp-defs.h>
#include <asm/octeon/cvmx-sli-defs.h>
#include <asm/octeon/cvmx-pip-defs.h>
#include <asm/octeon/cvmx-pki.h>
#else
#include "cvmx.h"
#include "cvmx-hwpko.h"
#include "cvmx-helper.h"
#include "cvmx-pki.h"
#endif

int CVMX_SHARED cvmx_npi_num_pipes = -1;

/**
 * Sets the number of pipe used by SLI packet output in the variable,
 * which then later used for setting it up in HW
 */
void cvmx_npi_config_set_num_pipes(int num_pipes)
{
	cvmx_npi_num_pipes = num_pipes;
}

/**
 * @INTERNAL
 * Probe a NPI interface and determine the number of ports
 * connected to it. The NPI interface should still be down
 * after this call.
 *
 * @param interface Interface to probe
 *
 * @return Number of ports on the interface. Zero to disable.
 */
int __cvmx_helper_npi_probe(int interface)
{
	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		return 32;
        } else if (OCTEON_IS_MODEL(OCTEON_CN73XX)) {
		return 128;
        } else if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
                return 64;
        } else if (!(OCTEON_IS_MODEL(OCTEON_CN52XX_PASS1_X) ||
		   OCTEON_IS_MODEL(OCTEON_CN56XX_PASS1_X) ||
		   OCTEON_IS_MODEL(OCTEON_CN31XX) ||
		   OCTEON_IS_MODEL(OCTEON_CN50XX) ||
		   OCTEON_IS_MODEL(OCTEON_CN30XX))) {
		/* The packet engines didn't exist before cn56xx pass 2 */
			return 4;
	}
	return 0;
}

/**
 * @INTERNAL
 * Bringup and enable a NPI interface. After this call packet
 * I/O should be fully functional. This is called with IPD
 * enabled but PKO disabled.
 *
 * @param xiface Interface to bring up
 *
 * @return Zero on success, negative on failure
 */
int __cvmx_helper_npi_enable(int xiface)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int interface = xi.interface;
	int port;
	int num_ports = cvmx_helper_ports_on_interface(interface);

	if (OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN58XX))
		/*
		 * Enables are controlled by the remote host, so
		 * nothing to do here.
		 */
		return 0;

	/*
	 * On CN50XX, CN52XX, and CN56XX we need to disable length
	 * checking so packet < 64 bytes and jumbo frames don't get
	 * errors.
	 */
	for (port = 0; port < num_ports; port++) {
		union cvmx_pip_prt_cfgx port_cfg;
		int ipd_port = (octeon_has_feature(OCTEON_FEATURE_PKND)) ?
				cvmx_helper_get_pknd(interface, port) : cvmx_helper_get_ipd_port(interface, port);
		if (octeon_has_feature(OCTEON_FEATURE_PKI)) {
			unsigned int node = cvmx_get_node_num();
			cvmx_pki_endis_l2_errs(node, ipd_port, 0, 0, 0);

		} else {
			port_cfg.u64 = cvmx_read_csr(CVMX_PIP_PRT_CFGX(ipd_port));
			port_cfg.s.lenerr_en = 0;
			port_cfg.s.maxerr_en = 0;
			port_cfg.s.minerr_en = 0;
			cvmx_write_csr(CVMX_PIP_PRT_CFGX(ipd_port), port_cfg.u64);
		}
		if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
			/* Set up pknd and bpid */
			union cvmx_sli_portx_pkind config;
			config.u64 = cvmx_read_csr(CVMX_PEXP_SLI_PORTX_PKIND(port));
			config.s.bpkind = cvmx_helper_get_bpid(interface, port);
			config.s.pkind = cvmx_helper_get_pknd(interface, port);
			cvmx_write_csr(CVMX_PEXP_SLI_PORTX_PKIND(port), config.u64);
		}
	}

	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		/*
		 * Set up pko pipes.
		 */
		union cvmx_sli_tx_pipe config;
		config.u64 = cvmx_read_csr(CVMX_PEXP_SLI_TX_PIPE);
		config.s.base = __cvmx_pko_get_pipe(interface, 0);
		config.s.nump = cvmx_npi_num_pipes < 0 ? num_ports : cvmx_npi_num_pipes;
		cvmx_write_csr(CVMX_PEXP_SLI_TX_PIPE, config.u64);
	}

	/* Enables are controlled by the remote host, so nothing to do here */
	return 0;
}
