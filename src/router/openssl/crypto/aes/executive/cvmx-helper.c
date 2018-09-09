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
 * Helper functions for common, but complicated tasks.
 *
 */
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <linux/export.h>
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-bootmem.h>
#include <asm/octeon/cvmx-sriox-defs.h>
#include <asm/octeon/cvmx-npi-defs.h>
#include <asm/octeon/cvmx-mio-defs.h>
#include <asm/octeon/cvmx-pexp-defs.h>
#include <asm/octeon/cvmx-pip-defs.h>
#include <asm/octeon/cvmx-asxx-defs.h>
#include <asm/octeon/cvmx-gmxx-defs.h>
#include <asm/octeon/cvmx-smix-defs.h>
#include <asm/octeon/cvmx-dbg-defs.h>
#include <asm/octeon/cvmx-sso-defs.h>

#include <asm/octeon/cvmx-agl.h>
#include <asm/octeon/cvmx-gmx.h>
#include <asm/octeon/cvmx-fpa.h>
#include <asm/octeon/cvmx-pip.h>
#include <asm/octeon/cvmx-pko.h>
#include <asm/octeon/cvmx-pko3.h>
#include <asm/octeon/cvmx-ipd.h>
#include <asm/octeon/cvmx-qlm.h>
#include <asm/octeon/cvmx-spi.h>
#include <asm/octeon/cvmx-clock.h>
#include <asm/octeon/cvmx-helper.h>
#include <asm/octeon/cvmx-helper-board.h>
#include <asm/octeon/cvmx-helper-errata.h>
#include <asm/octeon/cvmx-helper-cfg.h>
#include <asm/octeon/cvmx-helper-pki.h>
#include <asm/octeon/cvmx-pki.h>
#else
#include "cvmx.h"
#include "cvmx-sysinfo.h"
#include "cvmx-bootmem.h"
#include "cvmx-version.h"
#include "cvmx-gmx.h"
#include "cvmx-error.h"

#include "cvmx-fpa.h"
#include "cvmx-pip.h"
#include "cvmx-pko.h"
#include "cvmx-pko3.h"
#include "cvmx-ipd.h"
#include "cvmx-qlm.h"
#include "cvmx-spi.h"
#include "cvmx-helper.h"
#include "cvmx-helper-board.h"
#include "cvmx-helper-errata.h"
#include "cvmx-helper-cfg.h"
#include "cvmx-helper-pki.h"
#include "cvmx-pki.h"
#endif


/**
 * @INTERNAL
 * This structure specifies the interface methods used by an interface.
 *
 * @param mode		Interface mode.
 *
 * @param enumerate	Method the get number of interface ports.
 *
 * @param probe		Method to probe an interface to get the number of
 *			connected ports.
 *
 * @param enable	Method to enable an interface
 *
 * @param link_get	Method to get the state of an interface link.
 *
 * @param link_set	Method to configure an interface link to the specified
 *			state.
 *
 * @param loopback	Method to configure a port in loopback.
 */
struct iface_ops_s {
	cvmx_helper_interface_mode_t	mode;
	int				(*enumerate)(int interface);
	int				(*probe)(int interface);
	int				(*enable)(int interface);
	cvmx_helper_link_info_t		(*link_get)(int ipd_port);
	int				(*link_set)(int ipd_port,
					     cvmx_helper_link_info_t link_info);
	int				(*loopback)(int ipd_port,
						    int en_in, int en_ex);
};

/**
 * @INTERNAL
 * This structure is used by disabled interfaces.
 */
static const struct iface_ops_s iface_ops_dis = {
	.mode		= CVMX_HELPER_INTERFACE_MODE_DISABLED,
};

/**
 * @INTERNAL
 * This structure specifies the interface methods used by interfaces
 * configured as gmii.
 */
static const struct iface_ops_s iface_ops_gmii = {
	.mode		= CVMX_HELPER_INTERFACE_MODE_GMII,
	.enumerate	= __cvmx_helper_rgmii_enumerate,
	.probe		= __cvmx_helper_rgmii_probe,
	.enable		= __cvmx_helper_rgmii_enable,
	.link_get	= __cvmx_helper_gmii_link_get,
	.link_set	= __cvmx_helper_rgmii_link_set,
	.loopback	= __cvmx_helper_rgmii_configure_loopback,
};

/**
 * @INTERNAL
 * This structure specifies the interface methods used by interfaces
 * configured as rgmii.
 */
static const struct iface_ops_s iface_ops_rgmii = {
	.mode		= CVMX_HELPER_INTERFACE_MODE_RGMII,
	.enumerate	= __cvmx_helper_rgmii_enumerate,
	.probe		= __cvmx_helper_rgmii_probe,
	.enable		= __cvmx_helper_rgmii_enable,
	.link_get	= __cvmx_helper_rgmii_link_get,
	.link_set	= __cvmx_helper_rgmii_link_set,
	.loopback	= __cvmx_helper_rgmii_configure_loopback,
};

/**
 * @INTERNAL
 * This structure specifies the interface methods used by interfaces
 * configured as sgmii that use the gmx mac.
 */
static const struct iface_ops_s iface_ops_sgmii = {
	.mode		= CVMX_HELPER_INTERFACE_MODE_SGMII,
	.enumerate	= __cvmx_helper_sgmii_enumerate,
	.probe		= __cvmx_helper_sgmii_probe,
	.enable		= __cvmx_helper_sgmii_enable,
	.link_get	= __cvmx_helper_sgmii_link_get,
	.link_set	= __cvmx_helper_sgmii_link_set,
	.loopback	= __cvmx_helper_sgmii_configure_loopback,
};

/**
 * @INTERNAL
 * This structure specifies the interface methods used by interfaces
 * configured as sgmii that use the bgx mac.
 */
static const struct iface_ops_s iface_ops_bgx_sgmii = {
	.mode		= CVMX_HELPER_INTERFACE_MODE_SGMII,
	.enumerate	= __cvmx_helper_sgmii_enumerate,
	.probe		= __cvmx_helper_bgx_sgmii_probe,
	.enable		= __cvmx_helper_bgx_sgmii_enable,
	.link_get	= __cvmx_helper_bgx_sgmii_link_get,
	.link_set	= __cvmx_helper_bgx_sgmii_link_set,
	.loopback	= __cvmx_helper_bgx_sgmii_configure_loopback,
};

/**
 * @INTERNAL
 * This structure specifies the interface methods used by interfaces
 * configured as qsgmii.
 */
static const struct iface_ops_s iface_ops_qsgmii = {
	.mode		= CVMX_HELPER_INTERFACE_MODE_QSGMII,
	.enumerate	= __cvmx_helper_sgmii_enumerate,
	.probe		= __cvmx_helper_sgmii_probe,
	.enable		= __cvmx_helper_sgmii_enable,
	.link_get	= __cvmx_helper_sgmii_link_get,
	.link_set	= __cvmx_helper_sgmii_link_set,
	.loopback	= __cvmx_helper_sgmii_configure_loopback,
};

/**
 * @INTERNAL
 * This structure specifies the interface methods used by interfaces
 * configured as xaui using the gmx mac.
 */
static const struct iface_ops_s iface_ops_xaui = {
	.mode		= CVMX_HELPER_INTERFACE_MODE_XAUI,
	.enumerate	= __cvmx_helper_xaui_enumerate,
	.probe		= __cvmx_helper_xaui_probe,
	.enable		= __cvmx_helper_xaui_enable,
	.link_get	= __cvmx_helper_xaui_link_get,
	.link_set	= __cvmx_helper_xaui_link_set,
	.loopback	= __cvmx_helper_xaui_configure_loopback,
};

/**
 * @INTERNAL
 * This structure specifies the interface methods used by interfaces
 * configured as xaui using the bgx mac.
 */
static const struct iface_ops_s iface_ops_bgx_xaui = {
	.mode		= CVMX_HELPER_INTERFACE_MODE_XAUI,
	.enumerate	= __cvmx_helper_xaui_enumerate,
	.probe		= __cvmx_helper_bgx_xaui_probe,
	.enable		= __cvmx_helper_bgx_xaui_enable,
	.link_get	= __cvmx_helper_bgx_xaui_link_get,
	.link_set	= __cvmx_helper_bgx_xaui_link_set,
	.loopback	= __cvmx_helper_bgx_xaui_configure_loopback,
};

/**
 * @INTERNAL
 * This structure specifies the interface methods used by interfaces
 * configured as rxaui.
 */
static const struct iface_ops_s iface_ops_rxaui = {
	.mode		= CVMX_HELPER_INTERFACE_MODE_RXAUI,
	.enumerate	= __cvmx_helper_xaui_enumerate,
	.probe		= __cvmx_helper_xaui_probe,
	.enable		= __cvmx_helper_xaui_enable,
	.link_get	= __cvmx_helper_xaui_link_get,
	.link_set	= __cvmx_helper_xaui_link_set,
	.loopback	= __cvmx_helper_xaui_configure_loopback,
};

/**
 * @INTERNAL
 * This structure specifies the interface methods used by interfaces
 * configured as ilk.
 */
static const struct iface_ops_s iface_ops_ilk = {
	.mode		= CVMX_HELPER_INTERFACE_MODE_ILK,
	.enumerate	= __cvmx_helper_ilk_enumerate,
	.probe		= __cvmx_helper_ilk_probe,
	.enable		= __cvmx_helper_ilk_enable,
	.link_get	= __cvmx_helper_ilk_link_get,
	.link_set	= __cvmx_helper_ilk_link_set,
};

/**
 * @INTERNAL
 * This structure specifies the interface methods used by interfaces
 * configured as npi.
 */
static const struct iface_ops_s iface_ops_npi = {
	.mode		= CVMX_HELPER_INTERFACE_MODE_NPI,
	.enumerate	= __cvmx_helper_npi_enumerate,
	.probe		= __cvmx_helper_npi_probe,
	.enable		= __cvmx_helper_npi_enable,
};

/**
 * @INTERNAL
 * This structure specifies the interface methods used by interfaces
 * configured as srio.
 */
static const struct iface_ops_s iface_ops_srio = {
	.mode		= CVMX_HELPER_INTERFACE_MODE_SRIO,
	.enumerate	= __cvmx_helper_srio_enumerate,
	.probe		= __cvmx_helper_srio_probe,
	.enable		= __cvmx_helper_srio_enable,
	.link_get	= __cvmx_helper_srio_link_get,
	.link_set	= __cvmx_helper_srio_link_set,
};

/**
 * @INTERNAL
 * This structure specifies the interface methods used by interfaces
 * configured as agl.
 */
static const struct iface_ops_s iface_ops_agl = {
	.mode		= CVMX_HELPER_INTERFACE_MODE_AGL,
	.enumerate	= __cvmx_helper_agl_enumerate,
	.probe		= __cvmx_helper_agl_probe,
	.enable		= __cvmx_helper_agl_enable,
	.link_get	= __cvmx_helper_agl_link_get,
	.link_set	= __cvmx_helper_agl_link_set,
};

/**
 * @INTERNAL
 * This structure specifies the interface methods used by interfaces
 * configured as picmg.
 */
static const struct iface_ops_s iface_ops_picmg = {
	.mode		= CVMX_HELPER_INTERFACE_MODE_PICMG,
	.enumerate	= __cvmx_helper_sgmii_enumerate,
	.probe		= __cvmx_helper_sgmii_probe,
	.enable		= __cvmx_helper_sgmii_enable,
	.link_get	= __cvmx_helper_sgmii_link_get,
	.link_set	= __cvmx_helper_sgmii_link_set,
	.loopback	= __cvmx_helper_sgmii_configure_loopback,
};

/**
 * @INTERNAL
 * This structure specifies the interface methods used by interfaces
 * configured as spi.
 */
static const struct iface_ops_s iface_ops_spi = {
	.mode		= CVMX_HELPER_INTERFACE_MODE_SPI,
	.enumerate	= __cvmx_helper_spi_enumerate,
	.probe		= __cvmx_helper_spi_probe,
	.enable		= __cvmx_helper_spi_enable,
	.link_get	= __cvmx_helper_spi_link_get,
	.link_set	= __cvmx_helper_spi_link_set,
};

/**
 * @INTERNAL
 * This structure specifies the interface methods used by interfaces
 * configured as loop.
 */
static const struct iface_ops_s iface_ops_loop = {
	.mode		= CVMX_HELPER_INTERFACE_MODE_LOOP,
	.enumerate	= __cvmx_helper_loop_enumerate,
	.probe		= __cvmx_helper_loop_probe,
};

CVMX_SHARED const struct iface_ops_s *iface_ops[CVMX_HELPER_MAX_IFACE] = {
	[0 ... CVMX_HELPER_MAX_IFACE - 1] = NULL
};

CVMX_SHARED uint64_t  cvmx_rgmii_backpressure_dis = 1;

typedef int (*cvmx_export_config_t)(void);
cvmx_export_config_t cvmx_export_app_config;

void cvmx_rgmii_set_back_pressure(uint64_t backpressure_dis)
{
	cvmx_rgmii_backpressure_dis = backpressure_dis;
}

/*
 * internal functions that are not exported in the .h file but must be
 * declared to make gcc happy.
 */
extern cvmx_helper_link_info_t __cvmx_helper_get_link_info(int interface, int port);


/**
 * cvmx_override_pko_queue_priority(int pko_port, uint64_t
 * priorities[16]) is a function pointer. It is meant to allow
 * customization of the PKO queue priorities based on the port
 * number. Users should set this pointer to a function before
 * calling any cvmx-helper operations.
 */
CVMX_SHARED void (*cvmx_override_pko_queue_priority) (int ipd_port, uint64_t *priorities) = NULL;
EXPORT_SYMBOL(cvmx_override_pko_queue_priority);

/**
 * cvmx_override_iface_phy_mode(int interface, int index) is a function pointer.
 * It is meant to allow customization of interfaces which do not have a PHY.
 *
 * @returns 0 if MAC decides TX_CONFIG_REG or 1 if PHY decides  TX_CONFIG_REG.
 *
 * If this function pointer is NULL then it defaults to the MAC.
 */
CVMX_SHARED int (*cvmx_override_iface_phy_mode) (int interface, int index);
EXPORT_SYMBOL(cvmx_override_iface_phy_mode);

/**
 * cvmx_override_ipd_port_setup(int ipd_port) is a function
 * pointer. It is meant to allow customization of the IPD
 * port/port kind setup before packet input/output comes online.
 * It is called after cvmx-helper does the default IPD configuration,
 * but before IPD is enabled. Users should set this pointer to a
 * function before calling any cvmx-helper operations.
 */
CVMX_SHARED void (*cvmx_override_ipd_port_setup) (int ipd_port) = NULL;

/**
 * Return the number of interfaces the chip has. Each interface
 * may have multiple ports. Most chips support two interfaces,
 * but the CNX0XX and CNX1XX are exceptions. These only support
 * one interface.
 *
 * @return Number of interfaces on chip
 */
int cvmx_helper_get_number_of_interfaces(void)
{

	if (OCTEON_IS_MODEL(OCTEON_CN68XX))
		return 9;
	else if (OCTEON_IS_MODEL(OCTEON_CN66XX))
		if (OCTEON_IS_MODEL(OCTEON_CN66XX_PASS1_0))
			return 7;
		else
			return 8;
	else if (OCTEON_IS_MODEL(OCTEON_CN63XX))
		return 6;
	else if (OCTEON_IS_MODEL(OCTEON_CN56XX) ||
		 OCTEON_IS_MODEL(OCTEON_CN52XX) ||
		 OCTEON_IS_MODEL(OCTEON_CN61XX) ||
		 OCTEON_IS_MODEL(OCTEON_CNF71XX))
		return 4;
	else if (OCTEON_IS_MODEL(OCTEON_CN70XX))
		return 5;
	else if (OCTEON_IS_MODEL(OCTEON_CN78XX))
		return 10;
	else
		return 3;
}
EXPORT_SYMBOL(cvmx_helper_get_number_of_interfaces);

int __cvmx_helper_early_ports_on_interface(int interface)
{
	if (octeon_has_feature(OCTEON_FEATURE_PKND))
		return cvmx_helper_interface_enumerate(interface);
	else {
		int ports = cvmx_helper_interface_enumerate(interface);
		ports = __cvmx_helper_board_interface_probe(interface, ports);
		return ports;
	}
}
EXPORT_SYMBOL(__cvmx_helper_early_ports_on_interface);

/**
 * Return the number of ports on an interface. Depending on the
 * chip and configuration, this can be 1-16. A value of 0
 * specifies that the interface doesn't exist or isn't usable.
 *
 * @param interface Interface to get the port count for
 *
 * @return Number of ports on interface. Can be Zero.
 */
int cvmx_helper_ports_on_interface(int interface)
{
	if (octeon_has_feature(OCTEON_FEATURE_PKND))
		return cvmx_helper_interface_enumerate(interface);
	else
		return __cvmx_helper_get_num_ipd_ports(interface);
}
EXPORT_SYMBOL(cvmx_helper_ports_on_interface);

/**
 * @INTERNAL
 * Return interface mode for CN70XX.
 */
static cvmx_helper_interface_mode_t __cvmx_get_mode_cn70xx(int interface)
{
	/* SGMII/RXAUI/QSGMII */
	if (interface < 2) {
		enum cvmx_qlm_mode qlm_mode = cvmx_qlm_get_dlm_mode(0, interface);

		if (qlm_mode == CVMX_QLM_MODE_SGMII)
			iface_ops[interface] = &iface_ops_sgmii;
		else if (qlm_mode == CVMX_QLM_MODE_QSGMII)
			iface_ops[interface] = &iface_ops_qsgmii;
		else if (qlm_mode == CVMX_QLM_MODE_RXAUI)
			iface_ops[interface] = &iface_ops_rxaui;
		else
			iface_ops[interface] = &iface_ops_dis;
	}
	else if (interface == 2) /* DPI */
		iface_ops[interface] = &iface_ops_npi;
	else if (interface == 3) /* LOOP */
		iface_ops[interface] = &iface_ops_loop;
	else if (interface == 4) { /* RGMII (AGL) */
		cvmx_agl_prtx_ctl_t prtx_ctl;
		prtx_ctl.u64 = cvmx_read_csr(CVMX_AGL_PRTX_CTL(0));
		if (prtx_ctl.s.mode == 0)
			iface_ops[interface] = &iface_ops_agl;
		else
			iface_ops[interface] = &iface_ops_dis;
	}
	else
		iface_ops[interface] = &iface_ops_dis;

	return iface_ops[interface]->mode;
}

/**
 * @INTERNAL
 * Return interface mode for CN78XX.
 */
static cvmx_helper_interface_mode_t __cvmx_get_mode_cn78xx(int interface)
{
	/*
	 * Until gser configuration is in place, we hard code the interface
	 * mode here. This means that for the time being, this function and
	 * cvmx_qlm_get_mode() have to be in sync since they are both hard
	 * coded.
	 */
	switch (interface) {
	case 0:
	case 1:
		iface_ops[interface] = &iface_ops_bgx_sgmii;
		break;

	case 2:
		/*
		 * Interface 2's qlm is used by ilk so it can never be
		 * connected to a bgx mac. Disable it for now.
		 */
		iface_ops[interface] = &iface_ops_dis;
		break;

	case 3:
	case 4:
	case 5:
		iface_ops[interface] = &iface_ops_bgx_xaui;
		break;

	case 6:
	case 7:
		iface_ops[interface] = &iface_ops_ilk;
		break;

	case 8:
		iface_ops[interface] = &iface_ops_dis;
		break;

	case 9:
		iface_ops[interface] = &iface_ops_loop;
		break;

	default:
		iface_ops[interface] = &iface_ops_dis;
		break;
	}

	return iface_ops[interface]->mode;
}

/**
 * @INTERNAL
 * Return interface mode for CN68xx.
 */
static cvmx_helper_interface_mode_t __cvmx_get_mode_cn68xx(int interface)
{
	union cvmx_mio_qlmx_cfg qlm_cfg;

	switch (interface) {
	case 0:
		qlm_cfg.u64 = cvmx_read_csr(CVMX_MIO_QLMX_CFG(0));
		/* QLM is disabled when QLM SPD is 15. */
		if (qlm_cfg.s.qlm_spd == 15)
			iface_ops[interface] = &iface_ops_dis;
		else if (qlm_cfg.s.qlm_cfg == 7)
			iface_ops[interface] = &iface_ops_rxaui;
		else if (qlm_cfg.s.qlm_cfg == 2)
			iface_ops[interface] = &iface_ops_sgmii;
		else if (qlm_cfg.s.qlm_cfg == 3)
			iface_ops[interface] = &iface_ops_xaui;
		else
			iface_ops[interface] = &iface_ops_dis;
		break;

	case 1:
		qlm_cfg.u64 = cvmx_read_csr(CVMX_MIO_QLMX_CFG(0));
		/* QLM is disabled when QLM SPD is 15. */
		if (qlm_cfg.s.qlm_spd == 15)
			iface_ops[interface] = &iface_ops_dis;
		else if (qlm_cfg.s.qlm_cfg == 7)
			iface_ops[interface] = &iface_ops_rxaui;
		else
			iface_ops[interface] = &iface_ops_dis;
		break;

	case 2:
	case 3:
	case 4:
		qlm_cfg.u64 = cvmx_read_csr(CVMX_MIO_QLMX_CFG(interface));
		/* QLM is disabled when QLM SPD is 15. */
		if (qlm_cfg.s.qlm_spd == 15)
			iface_ops[interface] = &iface_ops_dis;
		else if (qlm_cfg.s.qlm_cfg == 2)
			iface_ops[interface] = &iface_ops_sgmii;
		else if (qlm_cfg.s.qlm_cfg == 3)
			iface_ops[interface] = &iface_ops_xaui;
		else
			iface_ops[interface] = &iface_ops_dis;
		break;

	case 5:
	case 6:
		qlm_cfg.u64 = cvmx_read_csr(CVMX_MIO_QLMX_CFG(interface - 4));
		/* QLM is disabled when QLM SPD is 15. */
		if (qlm_cfg.s.qlm_spd == 15)
			iface_ops[interface] = &iface_ops_dis;
		else if (qlm_cfg.s.qlm_cfg == 1)
			iface_ops[interface] = &iface_ops_ilk;
		else
			iface_ops[interface] = &iface_ops_dis;
		break;

	case 7:
		qlm_cfg.u64 = cvmx_read_csr(CVMX_MIO_QLMX_CFG(3));
		/* QLM is disabled when QLM SPD is 15. */
		if (qlm_cfg.s.qlm_spd == 15) {
			iface_ops[interface] = &iface_ops_dis;
		} else if (qlm_cfg.s.qlm_cfg != 0) {
			qlm_cfg.u64 = cvmx_read_csr(CVMX_MIO_QLMX_CFG(1));
			if (qlm_cfg.s.qlm_cfg != 0)
				iface_ops[interface] = &iface_ops_dis;
			else
				iface_ops[interface] = &iface_ops_npi;
		}
		else
			iface_ops[interface] = &iface_ops_npi;
		break;

	case 8:
		iface_ops[interface] = &iface_ops_loop;
		break;

	default:
		iface_ops[interface] = &iface_ops_dis;
		break;
	}

	return iface_ops[interface]->mode;
}

/**
 * @INTERNAL
 * Return interface mode for an Octeon II
 */
static cvmx_helper_interface_mode_t __cvmx_get_mode_octeon2(int interface)
{
	union cvmx_gmxx_inf_mode mode;

	if (OCTEON_IS_MODEL(OCTEON_CN68XX))
		return __cvmx_get_mode_cn68xx(interface);

	if (interface == 2)
		iface_ops[interface] = &iface_ops_npi;
	else if (interface == 3)
		iface_ops[interface] = &iface_ops_loop;
	else if ((OCTEON_IS_MODEL(OCTEON_CN63XX) &&
		  (interface == 4 || interface == 5)) ||
		 (OCTEON_IS_MODEL(OCTEON_CN66XX) &&
		  interface >= 4 && interface <= 7)) {
		/* Only present in CN63XX & CN66XX Octeon model */
		union cvmx_sriox_status_reg sriox_status_reg;

		/* cn66xx pass1.0 has only 2 SRIO interfaces. */
		if ((interface == 5 || interface == 7) &&
		    OCTEON_IS_MODEL(OCTEON_CN66XX_PASS1_0))
			iface_ops[interface] = &iface_ops_dis;
		else if (interface == 5 && OCTEON_IS_MODEL(OCTEON_CN66XX))
			/*
			 * Later passes of cn66xx support SRIO0 - x4/x2/x1,
			 * SRIO2 - x2/x1, SRIO3 - x1
			 */
			iface_ops[interface] = &iface_ops_dis;
		else {
			sriox_status_reg.u64 =
				cvmx_read_csr(CVMX_SRIOX_STATUS_REG(interface - 4));
			if (sriox_status_reg.s.srio)
				iface_ops[interface] = &iface_ops_srio;
			else
				iface_ops[interface] = &iface_ops_dis;
		}
	}
	else if (OCTEON_IS_MODEL(OCTEON_CN66XX)) {
		union cvmx_mio_qlmx_cfg mio_qlm_cfg;

		/* QLM2 is SGMII0 and QLM1 is SGMII1 */
		if (interface == 0)
			mio_qlm_cfg.u64 = cvmx_read_csr(CVMX_MIO_QLMX_CFG(2));
		else if (interface == 1)
			mio_qlm_cfg.u64 = cvmx_read_csr(CVMX_MIO_QLMX_CFG(1));
		else {
			iface_ops[interface] = &iface_ops_dis;
			return iface_ops[interface]->mode;
		}

		if (mio_qlm_cfg.s.qlm_spd == 15)
			iface_ops[interface] = &iface_ops_dis;
		else if (mio_qlm_cfg.s.qlm_cfg == 9)
			iface_ops[interface] = &iface_ops_sgmii;
		else if (mio_qlm_cfg.s.qlm_cfg == 11)
			iface_ops[interface] = &iface_ops_xaui;
		else
			iface_ops[interface] = &iface_ops_dis;
	} else if (OCTEON_IS_MODEL(OCTEON_CN61XX)) {
		union cvmx_mio_qlmx_cfg qlm_cfg;

		if (interface == 0)
			qlm_cfg.u64 = cvmx_read_csr(CVMX_MIO_QLMX_CFG(2));
		else if (interface == 1)
			qlm_cfg.u64 = cvmx_read_csr(CVMX_MIO_QLMX_CFG(0));
		else {
			iface_ops[interface] = &iface_ops_dis;
			return iface_ops[interface]->mode;
		}

		if (qlm_cfg.s.qlm_spd == 15)
			iface_ops[interface] = &iface_ops_dis;
		else if (qlm_cfg.s.qlm_cfg == 2)
			iface_ops[interface] = &iface_ops_sgmii;
		else if (qlm_cfg.s.qlm_cfg == 3)
			iface_ops[interface] = &iface_ops_xaui;
		else
			iface_ops[interface] = &iface_ops_dis;
	} else if (OCTEON_IS_MODEL(OCTEON_CNF71XX)) {
		if (interface == 0) {
			union cvmx_mio_qlmx_cfg qlm_cfg;
			qlm_cfg.u64 = cvmx_read_csr(CVMX_MIO_QLMX_CFG(0));
			if (qlm_cfg.s.qlm_cfg == 2)
				iface_ops[interface] = &iface_ops_sgmii;
			else
				iface_ops[interface] = &iface_ops_dis;
		}
		else
			iface_ops[interface] = &iface_ops_dis;
	}
	else if (interface == 1 && OCTEON_IS_MODEL(OCTEON_CN63XX))
		iface_ops[interface] = &iface_ops_dis;
	else {
		mode.u64 = cvmx_read_csr(CVMX_GMXX_INF_MODE(interface));

		if (OCTEON_IS_MODEL(OCTEON_CN63XX)) {
			switch (mode.cn63xx.mode) {
			case 0:
				iface_ops[interface] = &iface_ops_sgmii;
				break;

			case 1:
				iface_ops[interface] = &iface_ops_xaui;
				break;

			default:
				iface_ops[interface] = &iface_ops_dis;
				break;
			}
		} else {
			if (!mode.s.en)
				iface_ops[interface] = &iface_ops_dis;
			else if (mode.s.type)
				iface_ops[interface] = &iface_ops_gmii;
			else
				iface_ops[interface] = &iface_ops_rgmii;
		}
	}

	return iface_ops[interface]->mode;
}

/**
 * Get the operating mode of an interface. Depending on the Octeon
 * chip and configuration, this function returns an enumeration
 * of the type of packet I/O supported by an interface.
 *
 * @param interface Interface to probe
 *
 * @return Mode of the interface. Unknown or unsupported interfaces return
 *         DISABLED.
 */
cvmx_helper_interface_mode_t cvmx_helper_interface_get_mode(int interface)
{
	union cvmx_gmxx_inf_mode mode;

	if (interface < 0 ||
	    interface >= cvmx_helper_get_number_of_interfaces())
		return CVMX_HELPER_INTERFACE_MODE_DISABLED;

	/*
	 * Check if the interface mode has been already cached. If it has,
	 * simply return it. Otherwise, fall through the rest of the code to
	 * determine the interface mode and cache it in iface_ops.
	 */
	if (iface_ops[interface] != NULL)
		return iface_ops[interface]->mode;

	/*
	 * OCTEON III models
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN70XX))
		return __cvmx_get_mode_cn70xx(interface);

	if (OCTEON_IS_MODEL(OCTEON_CN78XX))
		return __cvmx_get_mode_cn78xx(interface);

	/*
	 * Octeon II models
	 */
	if (OCTEON_IS_OCTEON2())
		return __cvmx_get_mode_octeon2(interface);

	/*
	 * Octeon and Octeon Plus models
	 */
	if (interface == 2)
		iface_ops[interface] = &iface_ops_npi;
	else if (interface == 3) {
		if (OCTEON_IS_MODEL(OCTEON_CN56XX)
		    || OCTEON_IS_MODEL(OCTEON_CN52XX))
			iface_ops[interface] = &iface_ops_loop;
		else
			iface_ops[interface] = &iface_ops_dis;
	}
	else if (interface == 0 &&
	    cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_CN3005_EVB_HS5 &&
	    cvmx_sysinfo_get()->board_rev_major == 1) {
		/*
		 * Lie about interface type of CN3005 board.  This
		 * board has a switch on port 1 like the other
		 * evaluation boards, but it is connected over RGMII
		 * instead of GMII.  Report GMII mode so that the
		 * speed is forced to 1 Gbit full duplex.  Other than
		 * some initial configuration (which does not use the
		 * output of this function) there is no difference in
		 * setup between GMII and RGMII modes.
		 */
		iface_ops[interface] = &iface_ops_gmii;
	}
	else if ((interface == 1)
	    && (OCTEON_IS_MODEL(OCTEON_CN31XX)
		|| OCTEON_IS_MODEL(OCTEON_CN30XX)
		|| OCTEON_IS_MODEL(OCTEON_CN50XX)
		|| OCTEON_IS_MODEL(OCTEON_CN52XX)))
		/* Interface 1 is always disabled on CN31XX and CN30XX */
		iface_ops[interface] = &iface_ops_dis;
	else {
		mode.u64 = cvmx_read_csr(CVMX_GMXX_INF_MODE(interface));

		if (OCTEON_IS_MODEL(OCTEON_CN56XX) ||
		    OCTEON_IS_MODEL(OCTEON_CN52XX)) {
			switch (mode.cn56xx.mode) {
			case 0:
				iface_ops[interface] = &iface_ops_dis;
				break;

			case 1:
				iface_ops[interface] = &iface_ops_xaui;
				break;

			case 2:
				iface_ops[interface] = &iface_ops_sgmii;
				break;

			case 3:
				iface_ops[interface] = &iface_ops_picmg;
				break;

			default:
				iface_ops[interface] = &iface_ops_dis;
				break;
			}
		} else {
			if (!mode.s.en)
				iface_ops[interface] = &iface_ops_dis;
			else if (mode.s.type) {
				if (OCTEON_IS_MODEL(OCTEON_CN38XX) ||
				    OCTEON_IS_MODEL(OCTEON_CN58XX))
					iface_ops[interface] = &iface_ops_spi;
				else
					iface_ops[interface] = &iface_ops_gmii;
			} else
				iface_ops[interface] = &iface_ops_rgmii;
		}
	}

	return iface_ops[interface]->mode;
}
EXPORT_SYMBOL(cvmx_helper_interface_get_mode);

/**
 * @INTERNAL
 * Configure the IPD/PIP tagging and QoS options for a specific
 * port. This function determines the POW work queue entry
 * contents for a port. The setup performed here is controlled by
 * the defines in executive-config.h.
 *
 * @param ipd_port Port/Port kind to configure. This follows the IPD numbering,
 *                 not the per interface numbering
 *
 * @return Zero on success, negative on failure
 */
static int __cvmx_helper_port_setup_ipd(int ipd_port)
{
	union cvmx_pip_prt_cfgx port_config;
	union cvmx_pip_prt_tagx tag_config;

	if (octeon_has_feature(OCTEON_FEATURE_PKND)) {
		int interface, index, pknd;
		union cvmx_pip_prt_cfgbx prt_cfgbx;

		interface = cvmx_helper_get_interface_num(ipd_port);
		index = cvmx_helper_get_interface_index_num(ipd_port);
		pknd = cvmx_helper_get_pknd(interface, index);

		port_config.u64 = cvmx_read_csr(CVMX_PIP_PRT_CFGX(pknd));
		tag_config.u64 = cvmx_read_csr(CVMX_PIP_PRT_TAGX(pknd));

		port_config.s.qos = pknd & 0x7;

		/* Default BPID to use for packets on this port-kind */
		prt_cfgbx.u64 = cvmx_read_csr(CVMX_PIP_PRT_CFGBX(pknd));
		prt_cfgbx.s.bpid = pknd;
		cvmx_write_csr(CVMX_PIP_PRT_CFGBX(pknd), prt_cfgbx.u64);
	} else {
		port_config.u64 = cvmx_read_csr(CVMX_PIP_PRT_CFGX(ipd_port));
		tag_config.u64 = cvmx_read_csr(CVMX_PIP_PRT_TAGX(ipd_port));

		/* Have each port go to a different POW queue */
		port_config.s.qos = ipd_port & 0x7;
	}

	/* Process the headers and place the IP header in the work queue */
	port_config.s.mode = (cvmx_pip_port_parse_mode_t)cvmx_ipd_cfg.port_config.parse_mode;

	tag_config.s.ip6_src_flag  = cvmx_ipd_cfg.port_config.tag_fields.ipv6_src_ip;
	tag_config.s.ip6_dst_flag  = cvmx_ipd_cfg.port_config.tag_fields.ipv6_dst_ip;
	tag_config.s.ip6_sprt_flag = cvmx_ipd_cfg.port_config.tag_fields.ipv6_src_port;
	tag_config.s.ip6_dprt_flag = cvmx_ipd_cfg.port_config.tag_fields.ipv6_dst_port;
	tag_config.s.ip6_nxth_flag = cvmx_ipd_cfg.port_config.tag_fields.ipv6_next_header;
	tag_config.s.ip4_src_flag  = cvmx_ipd_cfg.port_config.tag_fields.ipv4_src_ip;
	tag_config.s.ip4_dst_flag  = cvmx_ipd_cfg.port_config.tag_fields.ipv4_dst_ip;
	tag_config.s.ip4_sprt_flag = cvmx_ipd_cfg.port_config.tag_fields.ipv4_src_port;
	tag_config.s.ip4_dprt_flag = cvmx_ipd_cfg.port_config.tag_fields.ipv4_dst_port;
	tag_config.s.ip4_pctl_flag = cvmx_ipd_cfg.port_config.tag_fields.ipv4_protocol;
	tag_config.s.inc_prt_flag  = cvmx_ipd_cfg.port_config.tag_fields.input_port;
	tag_config.s.tcp6_tag_type = (cvmx_pow_tag_type_t)cvmx_ipd_cfg.port_config.tag_type;
	tag_config.s.tcp4_tag_type = (cvmx_pow_tag_type_t)cvmx_ipd_cfg.port_config.tag_type;
	tag_config.s.ip6_tag_type  = (cvmx_pow_tag_type_t)cvmx_ipd_cfg.port_config.tag_type;
	tag_config.s.ip4_tag_type  = (cvmx_pow_tag_type_t)cvmx_ipd_cfg.port_config.tag_type;
	tag_config.s.non_tag_type  = (cvmx_pow_tag_type_t)cvmx_ipd_cfg.port_config.tag_type;

	/* Put all packets in group 0. Other groups can be used by the app */
	tag_config.s.grp = 0;

	cvmx_pip_config_port(ipd_port, port_config, tag_config);

	/* Give the user a chance to override our setting for each port */
	if (cvmx_override_ipd_port_setup)
		cvmx_override_ipd_port_setup(ipd_port);

	return 0;
}

/**
 * Enable or disable FCS stripping for all the ports on an interface.
 *
 * @param interface
 * @param nports number of ports
 * @param has_fcs 0 for disable and !0 for enable
 */
static int cvmx_helper_fcs_op(int interface, int nports, int has_fcs)
{
	uint64_t port_bit;
	int index;
	int pknd;
	union cvmx_pip_sub_pkind_fcsx pkind_fcsx;
	union cvmx_pip_prt_cfgx port_cfg;

	if (!octeon_has_feature(OCTEON_FEATURE_PKND))
		return 0;
	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		cvmx_helper_pki_set_fcs_op(0, interface, nports, has_fcs);
		return 0;
	}

	port_bit = 0;
	for (index = 0; index < nports; index++)
		port_bit |= ((uint64_t) 1 << cvmx_helper_get_pknd(interface, index));

	pkind_fcsx.u64 = cvmx_read_csr(CVMX_PIP_SUB_PKIND_FCSX(0));
	if (has_fcs)
		pkind_fcsx.s.port_bit |= port_bit;
	else
		pkind_fcsx.s.port_bit &= ~port_bit;
	cvmx_write_csr(CVMX_PIP_SUB_PKIND_FCSX(0), pkind_fcsx.u64);

	for (pknd = 0; pknd < 64; pknd++) {
		if ((1ull << pknd) & port_bit) {
			port_cfg.u64 = cvmx_read_csr(CVMX_PIP_PRT_CFGX(pknd));
			port_cfg.s.crc_en = (has_fcs) ? 1 : 0;
			cvmx_write_csr(CVMX_PIP_PRT_CFGX(pknd), port_cfg.u64);
		}
	}

	return 0;
}

/**
 * Determine the actual number of hardware ports connected to an
 * interface. It doesn't setup the ports or enable them.
 *
 * @param interface Interface to enumerate
 *
 * @return The number of ports on the interface, negative on failure
 */
int cvmx_helper_interface_enumerate(int interface)
{
	int	result = 0;

	cvmx_helper_interface_get_mode(interface);
	if (iface_ops[interface]->enumerate)
		result = iface_ops[interface]->enumerate(interface);

	return result;
}
EXPORT_SYMBOL(cvmx_helper_interface_enumerate);

/**
 * This function probes an interface to determine the actual number of
 * hardware ports connected to it. It does some setup the ports but
 * doesn't enable them. The main goal here is to set the global
 * interface_port_count[interface] correctly. Final hardware setup of
 * the ports will be performed later.
 *
 * @param interface Interface to probe
 *
 * @return Zero on success, negative on failure
 */
int cvmx_helper_interface_probe(int interface)
{
	/*
	 * At this stage in the game we don't want packets to be
	 * moving yet.  The following probe calls should perform
	 * hardware setup needed to determine port counts. Receive
	 * must still be disabled.
	 */
	int nports;
	int has_fcs;
	enum cvmx_pko_padding padding = CVMX_PKO_PADDING_NONE;

	nports = -1;
	has_fcs = 0;

	cvmx_helper_interface_get_mode(interface);
	if (iface_ops[interface]->probe)
		nports = iface_ops[interface]->probe(interface);

	switch (iface_ops[interface]->mode) {
		/* These types don't support ports to IPD/PKO */
	case CVMX_HELPER_INTERFACE_MODE_DISABLED:
	case CVMX_HELPER_INTERFACE_MODE_PCIE:
		nports = 0;
		break;
		/* XAUI is a single high speed port */
	case CVMX_HELPER_INTERFACE_MODE_XAUI:
	case CVMX_HELPER_INTERFACE_MODE_RXAUI:
		has_fcs = 1;
		padding = CVMX_PKO_PADDING_60;
		break;
		/*
		 * RGMII/GMII/MII are all treated about the same. Most
		 * functions refer to these ports as RGMII.
		 */
	case CVMX_HELPER_INTERFACE_MODE_RGMII:
	case CVMX_HELPER_INTERFACE_MODE_GMII:
		padding = CVMX_PKO_PADDING_60;
		break;
		/*
		 * SPI4 can have 1-16 ports depending on the device at
		 * the other end.
		 */
	case CVMX_HELPER_INTERFACE_MODE_SPI:
		padding = CVMX_PKO_PADDING_60;
		break;
		/*
		 * SGMII can have 1-4 ports depending on how many are
		 * hooked up.
		 */
	case CVMX_HELPER_INTERFACE_MODE_SGMII:
	case CVMX_HELPER_INTERFACE_MODE_QSGMII:
		padding = CVMX_PKO_PADDING_60;
	case CVMX_HELPER_INTERFACE_MODE_PICMG:
		has_fcs = 1;
		break;
		/* PCI target Network Packet Interface */
	case CVMX_HELPER_INTERFACE_MODE_NPI:
		break;
		/*
		 * Special loopback only ports. These are not the same
		 * as other ports in loopback mode.
		 */
	case CVMX_HELPER_INTERFACE_MODE_LOOP:
		break;
		/* SRIO has 2^N ports, where N is number of interfaces */
	case CVMX_HELPER_INTERFACE_MODE_SRIO:
		break;
	case CVMX_HELPER_INTERFACE_MODE_ILK:
		padding = CVMX_PKO_PADDING_60;
		has_fcs = 1;
		break;
	case CVMX_HELPER_INTERFACE_MODE_AGL:
		has_fcs = 1;
		break;
	}

	if (nports == -1)
		return -1;

	if (!octeon_has_feature(OCTEON_FEATURE_PKND))
		has_fcs = 0;

	nports = __cvmx_helper_board_interface_probe(interface, nports);
	__cvmx_helper_init_interface(interface, nports, has_fcs, padding);
	/* Make sure all global variables propagate to other cores */
	CVMX_SYNCWS;

	return 0;
}

/**
 * @INTERNAL
 * Setup the IPD/PIP for the ports on an interface. Packet
 * classification and tagging are set for every port on the
 * interface. The number of ports on the interface must already
 * have been probed.
 *
 * @param interface Interface to setup IPD/PIP for
 *
 * @return Zero on success, negative on failure
 */
static int __cvmx_helper_interface_setup_ipd(int interface)
{

	cvmx_helper_interface_mode_t mode;
	int ipd_port = cvmx_helper_get_ipd_port(interface, 0);
	int num_ports = cvmx_helper_ports_on_interface(interface);
	int delta;

	if (num_ports == CVMX_HELPER_CFG_INVALID_VALUE)
		return 0;

	mode = cvmx_helper_interface_get_mode(interface);

	if (!OCTEON_IS_MODEL(OCTEON_CN78XX))
		if (mode == CVMX_HELPER_INTERFACE_MODE_LOOP)
			__cvmx_helper_loop_enable(interface);

	delta = 1;
	if (octeon_has_feature(OCTEON_FEATURE_PKND)) {
		if (mode == CVMX_HELPER_INTERFACE_MODE_SGMII)
			delta = 16;
	}

	while (num_ports--) {
		if (!cvmx_helper_is_port_valid(interface, num_ports))
			continue;
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			__cvmx_helper_port_setup_pki(0, ipd_port);
		else
			__cvmx_helper_port_setup_ipd(ipd_port);
		ipd_port += delta;
	}

	/* FCS settings */
	cvmx_helper_fcs_op(interface,
	    cvmx_helper_ports_on_interface(interface),
	    __cvmx_helper_get_has_fcs(interface));

	return 0;
}

/** It allocate pools for packet and wqe pools
  * and sets up the FPA hardware
  */
#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
int __cvmx_helper_ipd_setup_fpa_pools(void)
{
	cvmx_fpa_global_initialize();
	if (cvmx_ipd_cfg.packet_pool.buffer_count == 0)
		return 0;
		__cvmx_helper_initialize_fpa_pool(cvmx_ipd_cfg.packet_pool.pool_num, cvmx_ipd_cfg.packet_pool.buffer_size,
					  cvmx_ipd_cfg.packet_pool.buffer_count, "Packet Buffers");
	if (cvmx_ipd_cfg.wqe_pool.buffer_count == 0)
		return 0;
		__cvmx_helper_initialize_fpa_pool(cvmx_ipd_cfg.wqe_pool.pool_num, cvmx_ipd_cfg.wqe_pool.buffer_size,
			cvmx_ipd_cfg.wqe_pool.buffer_count, "WQE Buffers");
	return 0;
}
#endif

/**
 * @INTERNAL
 * Setup global setting for IPD/PIP not related to a specific
 * interface or port. This must be called before IPD is enabled.
 *
 * @return Zero on success, negative on failure.
 */
static int __cvmx_helper_global_setup_ipd(void)
{
#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	/* Setup the packet and wqe pools*/
	__cvmx_helper_ipd_setup_fpa_pools();
#endif
	/* Setup the global packet input options */
	cvmx_ipd_config(cvmx_ipd_cfg.packet_pool.buffer_size / 8,
			cvmx_ipd_cfg.first_mbuf_skip / 8,
			cvmx_ipd_cfg.not_first_mbuf_skip / 8,
			/* The +8 is to account for the next ptr */
			(cvmx_ipd_cfg.first_mbuf_skip + 8) / 128,
			/* The +8 is to account for the next ptr */
			(cvmx_ipd_cfg.not_first_mbuf_skip + 8) / 128,
			cvmx_ipd_cfg.wqe_pool.pool_num,
			(cvmx_ipd_mode_t)(cvmx_ipd_cfg.cache_mode), 1);
	return 0;
}

/**
 * @INTERNAL
 * Setup the PKO for the ports on an interface. The number of
 * queues per port and the priority of each PKO output queue
 * is set here. PKO must be disabled when this function is called.
 *
 * @param interface Interface to setup PKO for
 *
 * @return Zero on success, negative on failure
 */
static int __cvmx_helper_interface_setup_pko(int interface)
{
	/*
	 * Each packet output queue has an associated priority. The
	 * higher the priority, the more often it can send a packet. A
	 * priority of 8 means it can send in all 8 rounds of
	 * contention. We're going to make each queue one less than
	 * the last.  The vector of priorities has been extended to
	 * support CN5xxx CPUs, where up to 16 queues can be
	 * associated to a port.  To keep backward compatibility we
	 * don't change the initial 8 priorities and replicate them in
	 * the second half.  With per-core PKO queues (PKO lockless
	 * operation) all queues have the same priority.
	 */
	/* uint64_t priorities[16] = {8,7,6,5,4,3,2,1,8,7,6,5,4,3,2,1}; */
	uint64_t priorities[16] = {[0 ... 15] = 8 };

	/*
	 * Setup the IPD/PIP and PKO for the ports discovered
	 * above. Here packet classification, tagging and output
	 * priorities are set.
	 */
	int ipd_port = cvmx_helper_get_ipd_port(interface, 0);
	int num_ports = cvmx_helper_ports_on_interface(interface);
	while (num_ports--) {
		if (!cvmx_helper_is_port_valid(interface, num_ports))
			continue;
		/*
		 * Give the user a chance to override the per queue
		 * priorities.
		 */
		if (cvmx_override_pko_queue_priority)
			cvmx_override_pko_queue_priority(ipd_port, priorities);

		cvmx_pko_config_port(ipd_port,
				     cvmx_pko_get_base_queue(ipd_port),
				     cvmx_pko_get_num_queues(ipd_port),
				     priorities);
		ipd_port++;
	}
	return 0;
}

/**
 * @INTERNAL
 * Setup global setting for PKO not related to a specific
 * interface or port. This must be called before PKO is enabled.
 *
 * @return Zero on success, negative on failure.
 */
static int __cvmx_helper_global_setup_pko(void)
{
	/*
	 * Disable tagwait FAU timeout. This needs to be done before
	 * anyone might start packet output using tags.
	 */
	union cvmx_iob_fau_timeout fau_to;
	fau_to.u64 = 0;
	fau_to.s.tout_val = 0xfff;
	fau_to.s.tout_enb = 0;
	cvmx_write_csr(CVMX_IOB_FAU_TIMEOUT, fau_to.u64);

	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		union cvmx_pko_reg_min_pkt min_pkt;

		min_pkt.u64 = 0;
		min_pkt.s.size1 = 59;
		min_pkt.s.size2 = 59;
		min_pkt.s.size3 = 59;
		min_pkt.s.size4 = 59;
		min_pkt.s.size5 = 59;
		min_pkt.s.size6 = 59;
		min_pkt.s.size7 = 59;
		cvmx_write_csr(CVMX_PKO_REG_MIN_PKT, min_pkt.u64);
	}

	return 0;
}

/**
 * @INTERNAL
 * Setup global backpressure setting.
 *
 * @return Zero on success, negative on failure
 */
static int __cvmx_helper_global_setup_backpressure(void)
{
	if (cvmx_rgmii_backpressure_dis) {
		/* Disable backpressure if configured to do so */
		/* Disable backpressure (pause frame) generation */
		int num_interfaces = cvmx_helper_get_number_of_interfaces();
		int interface;
		for (interface = 0; interface < num_interfaces; interface++) {
			switch (cvmx_helper_interface_get_mode(interface)) {
			case CVMX_HELPER_INTERFACE_MODE_DISABLED:
			case CVMX_HELPER_INTERFACE_MODE_PCIE:
			case CVMX_HELPER_INTERFACE_MODE_SRIO:
			case CVMX_HELPER_INTERFACE_MODE_ILK:
			case CVMX_HELPER_INTERFACE_MODE_NPI:
			case CVMX_HELPER_INTERFACE_MODE_LOOP:
			case CVMX_HELPER_INTERFACE_MODE_XAUI:
			case CVMX_HELPER_INTERFACE_MODE_RXAUI:
				break;
			case CVMX_HELPER_INTERFACE_MODE_RGMII:
			case CVMX_HELPER_INTERFACE_MODE_GMII:
			case CVMX_HELPER_INTERFACE_MODE_SPI:
			case CVMX_HELPER_INTERFACE_MODE_SGMII:
			case CVMX_HELPER_INTERFACE_MODE_QSGMII:
			case CVMX_HELPER_INTERFACE_MODE_PICMG:
				cvmx_gmx_set_backpressure_override(interface, 0xf);
				break;
			case CVMX_HELPER_INTERFACE_MODE_AGL:
				cvmx_agl_set_backpressure_override(interface, 0x1);
				break;
			}
		}
		//cvmx_dprintf("Disabling backpressure\n");
	}
	return 0;
}

/**
 * @INTERNAL
 * Verify the per port IPD backpressure is aligned properly.
 * @return Zero if working, non zero if misaligned
 */
int __cvmx_helper_backpressure_is_misaligned(void)
{
	uint64_t ipd_int_enb;
	union cvmx_ipd_ctl_status ipd_reg;
	uint64_t bp_status0;
	uint64_t bp_status1;
	const int port0 = 0;
	const int port1 = 16;
	cvmx_helper_interface_mode_t mode0 = cvmx_helper_interface_get_mode(0);
	cvmx_helper_interface_mode_t mode1 = cvmx_helper_interface_get_mode(1);

	/* Disable error interrupts while we check backpressure */
	ipd_int_enb = cvmx_read_csr(CVMX_IPD_INT_ENB);
	cvmx_write_csr(CVMX_IPD_INT_ENB, 0);

	/* Enable per port backpressure */
	ipd_reg.u64 = cvmx_read_csr(CVMX_IPD_CTL_STATUS);
	ipd_reg.s.pbp_en = 1;
	cvmx_write_csr(CVMX_IPD_CTL_STATUS, ipd_reg.u64);

	if (mode0 != CVMX_HELPER_INTERFACE_MODE_DISABLED) {
		/* Enable backpressure for port with a zero threshold */
		cvmx_write_csr(CVMX_IPD_PORTX_BP_PAGE_CNT(port0), 1 << 17);
		/* Add 1000 to the page count to simulate packets coming in */
		cvmx_write_csr(CVMX_IPD_SUB_PORT_BP_PAGE_CNT,
			       (port0 << 25) | 1000);
	}

	if (mode1 != CVMX_HELPER_INTERFACE_MODE_DISABLED) {
		/* Enable backpressure for port with a zero threshold */
		cvmx_write_csr(CVMX_IPD_PORTX_BP_PAGE_CNT(port1), 1 << 17);
		/* Add 1000 to the page count to simulate packets coming in */
		cvmx_write_csr(CVMX_IPD_SUB_PORT_BP_PAGE_CNT,
			       (port1 << 25) | 1000);
	}

	/* Wait 500 cycles for the BP to update */
	cvmx_wait(500);

	/* Read the BP state from the debug select register */
	switch (mode0) {
	case CVMX_HELPER_INTERFACE_MODE_SPI:
		cvmx_write_csr(CVMX_NPI_DBG_SELECT, 0x9004);
		bp_status0 = cvmx_read_csr(CVMX_DBG_DATA);
		bp_status0 = 0xffff & ~bp_status0;
		break;
	case CVMX_HELPER_INTERFACE_MODE_RGMII:
	case CVMX_HELPER_INTERFACE_MODE_GMII:
		cvmx_write_csr(CVMX_NPI_DBG_SELECT, 0x0e00);
		bp_status0 = 0xffff & cvmx_read_csr(CVMX_DBG_DATA);
		break;
	case CVMX_HELPER_INTERFACE_MODE_XAUI:
	case CVMX_HELPER_INTERFACE_MODE_SGMII:
	case CVMX_HELPER_INTERFACE_MODE_QSGMII:
	case CVMX_HELPER_INTERFACE_MODE_PICMG:
		cvmx_write_csr(CVMX_PEXP_NPEI_DBG_SELECT, 0x0e00);
		bp_status0 = 0xffff & cvmx_read_csr(CVMX_PEXP_NPEI_DBG_DATA);
		break;
	default:
		bp_status0 = 1 << port0;
		break;
	}

	/* Read the BP state from the debug select register */
	switch (mode1) {
	case CVMX_HELPER_INTERFACE_MODE_SPI:
		cvmx_write_csr(CVMX_NPI_DBG_SELECT, 0x9804);
		bp_status1 = cvmx_read_csr(CVMX_DBG_DATA);
		bp_status1 = 0xffff & ~bp_status1;
		break;
	case CVMX_HELPER_INTERFACE_MODE_RGMII:
	case CVMX_HELPER_INTERFACE_MODE_GMII:
		cvmx_write_csr(CVMX_NPI_DBG_SELECT, 0x1600);
		bp_status1 = 0xffff & cvmx_read_csr(CVMX_DBG_DATA);
		break;
	case CVMX_HELPER_INTERFACE_MODE_XAUI:
	case CVMX_HELPER_INTERFACE_MODE_SGMII:
	case CVMX_HELPER_INTERFACE_MODE_QSGMII:
	case CVMX_HELPER_INTERFACE_MODE_PICMG:
		cvmx_write_csr(CVMX_PEXP_NPEI_DBG_SELECT, 0x1600);
		bp_status1 = 0xffff & cvmx_read_csr(CVMX_PEXP_NPEI_DBG_DATA);
		break;
	default:
		bp_status1 = 1 << (port1 - 16);
		break;
	}

	if (mode0 != CVMX_HELPER_INTERFACE_MODE_DISABLED) {
		/* Shutdown BP */
		cvmx_write_csr(CVMX_IPD_SUB_PORT_BP_PAGE_CNT, (port0 << 25) | (0x1ffffff & -1000));
		cvmx_write_csr(CVMX_IPD_PORTX_BP_PAGE_CNT(port0), 0);
	}

	if (mode1 != CVMX_HELPER_INTERFACE_MODE_DISABLED) {
		/* Shutdown BP */
		cvmx_write_csr(CVMX_IPD_SUB_PORT_BP_PAGE_CNT, (port1 << 25) | (0x1ffffff & -1000));
		cvmx_write_csr(CVMX_IPD_PORTX_BP_PAGE_CNT(port1), 0);
	}

	/* Clear any error interrupts that might have been set */
	cvmx_write_csr(CVMX_IPD_INT_SUM, 0x1f);
	cvmx_write_csr(CVMX_IPD_INT_ENB, ipd_int_enb);

	return (bp_status0 != (1ull << port0)) || (bp_status1 != (1ull << (port1 - 16)));
}

/**
 * @INTERNAL
 * Enable packet input/output from the hardware. This function is
 * called after all internal setup is complete and IPD is enabled.
 * After this function completes, packets will be accepted from the
 * hardware ports. PKO should still be disabled to make sure packets
 * aren't sent out partially setup hardware.
 *
 * @param interface Interface to enable
 * @param iflags Interface flags
 * @param pflags Array of flags, one per port on the interface
 *
 * @return Zero on success, negative on failure
 */
static int __cvmx_helper_packet_hardware_enable(int interface)
{
	int result = 0;

	cvmx_helper_interface_get_mode(interface);
	if (iface_ops[interface]->enable)
		result = iface_ops[interface]->enable(interface);
	result |= __cvmx_helper_board_hardware_enable(interface);
	return result;
}

/**
 * Called after all internal packet IO paths are setup. This
 * function enables IPD/PIP and begins packet input and output.
 *
 * @return Zero on success, negative on failure
 */
int cvmx_helper_ipd_and_packet_input_enable(void)
{
	int num_interfaces;
	int interface;
	int num_ports;

	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		cvmx_pki_parse_enable(0, 0);
		/* Enable PKI */
		cvmx_pki_enable(0);
	} else
		/* Enable IPD */
		cvmx_ipd_enable();

	/*
	 * Time to enable hardware ports packet input and output. Note
	 * that at this point IPD/PIP must be fully functional and PKO
	 * must be disabled .
	 */
	num_interfaces = cvmx_helper_get_number_of_interfaces();
	for (interface = 0; interface < num_interfaces; interface++) {
		num_ports = cvmx_helper_ports_on_interface(interface);
		if (num_ports > 0)
			__cvmx_helper_packet_hardware_enable(interface);
	}

	/* Finally enable PKO now that the entire path is up and running */
	/* enable pko */
	if (OCTEON_IS_MODEL(OCTEON_CN78XX))
		cvmx_pko_enable_78xx(0);
	else
		cvmx_pko_enable();

	if ((OCTEON_IS_MODEL(OCTEON_CN31XX_PASS1) ||
	     OCTEON_IS_MODEL(OCTEON_CN30XX_PASS1)) &&
	    (cvmx_sysinfo_get()->board_type != CVMX_BOARD_TYPE_SIM))
		__cvmx_helper_errata_fix_ipd_ptr_alignment();
	return 0;
}
EXPORT_SYMBOL(cvmx_helper_ipd_and_packet_input_enable);

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL

int cvmx_helper_initialize_sso_78xx(int node, int wqe_entries)
{
	int pool = -1;
	int grp;
	/* Setup an FPA pool to store the SSO queues */
	const int MAX_SSO_ENTRIES = 4096;
	int num_blocks = 256 + 48 + ((MAX_SSO_ENTRIES+25)/26);
	int aura = -1;
	cvmx_sso_xaq_aura_t aura_reg;
	cvmx_sso_nw_tim_t nw_tim;
	cvmx_sso_aw_cfg_t aw_cfg;

	if (cvmx_fpa_allocate_fpa_pools(node, &pool, 1) == -1) {
		cvmx_dprintf("ERROR:allocating pool for sso_init\n");
		return -1;
	}
	if (cvmx_fpa_allocate_auras(node, &aura, 1) == -1) {
		cvmx_dprintf("ERROR:allocating aura fo sso_init\n");
		return -1;
	}
	cvmx_fpa_pool_stack_init(node, pool, "SSO Pool", 0,
			MAX_SSO_ENTRIES, FPA_NATURAL_ALIGNMENT, 4096);
	cvmx_fpa_assign_aura(node, aura, pool);
	cvmx_fpa_aura_init(node, aura, "SSO Aura", 0, NULL, num_blocks, 0);

	/* Initialize the 256 group/qos queues */
	for (grp = 0; grp < 256; grp++) {
		cvmx_sso_xaqx_head_ptr_t ptr;
		cvmx_sso_xaqx_head_next_t next;
		cvmx_sso_xaqx_tail_next_t tail;
		cvmx_sso_grpx_pref_t pref;
		uint64_t addr;
		void *buffer = cvmx_fpa_alloc_aura(node, aura);

		if (buffer == NULL) {
			cvmx_dprintf("ERROR: Failed to allocate buffer\n");
			return -1;
		}
		addr = cvmx_ptr_to_phys(buffer);
		ptr.u64 = 0;
		next.u64 = 0;
		tail.u64 = 0;
		ptr.s.ptr = addr;
		next.s.ptr = addr;
		tail.s.ptr = addr;

		cvmx_write_csr_node(node, CVMX_SSO_XAQX_HEAD_PTR(grp), ptr.u64);
		cvmx_write_csr_node(node, CVMX_SSO_XAQX_HEAD_NEXT(grp), next.u64);
		cvmx_write_csr_node(node, CVMX_SSO_XAQX_TAIL_NEXT(grp), tail.u64);
		/* Prefetch one cache line into L1 when a core gets work */
		pref.u64 = 0;
		pref.s.clines = 1;
		cvmx_write_csr_node(node, CVMX_SSO_GRPX_PREF(grp), pref.u64);
	}
	/* Set the aura number */
	aura_reg.u64 = 0;
	aura_reg.s.laura = aura;
	aura_reg.s.node = node;
	cvmx_write_csr_node(node, CVMX_SSO_XAQ_AURA, aura_reg.u64);

	/* Set work timeout to 1023 * 1k cycles */
	nw_tim.u64 = 0;
	nw_tim.s.nw_tim = 1023;
	cvmx_write_csr_node(node, CVMX_SSO_NW_TIM, nw_tim.u64);

	aw_cfg.u64 = cvmx_read_csr_node(node, CVMX_SSO_AW_CFG);
	aw_cfg.s.rwen = 1;
	cvmx_write_csr_node(node, CVMX_SSO_AW_CFG, aw_cfg.u64);
	return 0;

}

#define __CVMX_SSO_RWQ_SIZE 256
int cvmx_helper_initialize_sso(int wqe_entries)
{
	int cvm_oct_sso_number_rwq_bufs;
	char *mem;
	int i;
	cvmx_sso_cfg_t sso_cfg;
	cvmx_fpa_fpfx_marks_t fpa_marks;

	if (!OCTEON_IS_MODEL(OCTEON_CN68XX))
		return 0;

	/*
	 * CN68XX-P1 may reset with the wrong values, put in
	 * the correct values (FPA-15816).
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN68XX_PASS1_X)) {
		fpa_marks.u64 = 0;
		fpa_marks.s.fpf_wr = 0xa4;
		fpa_marks.s.fpf_rd = 0x40;
		cvmx_write_csr(CVMX_FPA_FPF8_MARKS, fpa_marks.u64);
	}

	cvm_oct_sso_number_rwq_bufs = ((wqe_entries - 1) / 26) + 1 + 48 + 8;

	mem = cvmx_bootmem_alloc(__CVMX_SSO_RWQ_SIZE * cvm_oct_sso_number_rwq_bufs, CVMX_CACHE_LINE_SIZE);
	if (mem == NULL) {
		cvmx_dprintf("Out of memory initializing sso pool\n");
		return -1;
	}
	/* Make sure RWI/RWO is disabled. */
	sso_cfg.u64 = cvmx_read_csr(CVMX_SSO_CFG);
	sso_cfg.s.rwen = 0;
	cvmx_write_csr(CVMX_SSO_CFG, sso_cfg.u64);

	for (i = cvm_oct_sso_number_rwq_bufs - 8; i > 0; i--) {
		cvmx_sso_rwq_psh_fptr_t fptr;

		for (;;) {
			fptr.u64 = cvmx_read_csr(CVMX_SSO_RWQ_PSH_FPTR);
			if (!fptr.s.full)
				break;
			cvmx_wait(1000);
		}
		fptr.s.fptr = cvmx_ptr_to_phys(mem) >> 7;
		cvmx_write_csr(CVMX_SSO_RWQ_PSH_FPTR, fptr.u64);
		mem = mem + __CVMX_SSO_RWQ_SIZE;
	}

	for (i = 0; i < 8; i++) {
		cvmx_sso_rwq_head_ptrx_t head_ptr;
		cvmx_sso_rwq_tail_ptrx_t tail_ptr;

		head_ptr.u64 = 0;
		tail_ptr.u64 = 0;
		head_ptr.s.ptr = cvmx_ptr_to_phys(mem) >> 7;
		tail_ptr.s.ptr = head_ptr.s.ptr;
		cvmx_write_csr(CVMX_SSO_RWQ_HEAD_PTRX(i), head_ptr.u64);
		cvmx_write_csr(CVMX_SSO_RWQ_TAIL_PTRX(i), tail_ptr.u64);
		mem = mem + __CVMX_SSO_RWQ_SIZE;
	}

	sso_cfg.u64 = cvmx_read_csr(CVMX_SSO_CFG);
	sso_cfg.s.rwen = 1;
	sso_cfg.s.dwb = cvmx_helper_cfg_opt_get(CVMX_HELPER_CFG_OPT_USE_DWB);
	sso_cfg.s.rwq_byp_dis = 0;
	sso_cfg.s.rwio_byp_dis = 0;
	cvmx_write_csr(CVMX_SSO_CFG, sso_cfg.u64);

	return 0;
}

int cvmx_helper_initialize_sso_node(int node, int wqe_entries)
{

	if (OCTEON_IS_MODEL(OCTEON_CN78XX))
		return cvmx_helper_initialize_sso_78xx(node, wqe_entries);
	else
		return cvmx_helper_initialize_sso(wqe_entries);
}

int cvmx_helper_uninitialize_sso(void)
{
	cvmx_fpa_quex_available_t queue_available;
	cvmx_sso_cfg_t sso_cfg;
	cvmx_sso_rwq_pop_fptr_t pop_fptr;
	cvmx_sso_rwq_psh_fptr_t fptr;
	cvmx_sso_fpage_cnt_t fpage_cnt;
	int num_to_transfer, i;

	if (!OCTEON_IS_MODEL(OCTEON_CN68XX))
		return 0;

	sso_cfg.u64 = cvmx_read_csr(CVMX_SSO_CFG);
	sso_cfg.s.rwen = 0;
	sso_cfg.s.rwq_byp_dis = 1;
	cvmx_write_csr(CVMX_SSO_CFG, sso_cfg.u64);
	cvmx_read_csr(CVMX_SSO_CFG);
	queue_available.u64 = cvmx_read_csr(CVMX_FPA_QUEX_AVAILABLE(8));

	/* Make CVMX_FPA_QUEX_AVAILABLE(8) % 16 == 0 */
	for (num_to_transfer = (16 - queue_available.s.que_siz) % 16; num_to_transfer > 0; num_to_transfer--) {
		do {
			pop_fptr.u64 = cvmx_read_csr(CVMX_SSO_RWQ_POP_FPTR);
		} while (!pop_fptr.s.val);
		for (;;) {
			fptr.u64 = cvmx_read_csr(CVMX_SSO_RWQ_PSH_FPTR);
			if (!fptr.s.full)
				break;
			cvmx_wait(1000);
		}
		fptr.s.fptr = pop_fptr.s.fptr;
		cvmx_write_csr(CVMX_SSO_RWQ_PSH_FPTR, fptr.u64);
	}
	cvmx_read_csr(CVMX_SSO_CFG);

	do {
		queue_available.u64 = cvmx_read_csr(CVMX_FPA_QUEX_AVAILABLE(8));
	} while (queue_available.s.que_siz % 16);

	sso_cfg.s.rwen = 1;
	sso_cfg.s.rwq_byp_dis = 0;
	cvmx_write_csr(CVMX_SSO_CFG, sso_cfg.u64);

	for (i = 0; i < 8; i++) {
		cvmx_sso_rwq_head_ptrx_t head_ptr;
		cvmx_sso_rwq_tail_ptrx_t tail_ptr;

		head_ptr.u64 = cvmx_read_csr(CVMX_SSO_RWQ_HEAD_PTRX(i));
		tail_ptr.u64 = cvmx_read_csr(CVMX_SSO_RWQ_TAIL_PTRX(i));
		if (head_ptr.s.ptr != tail_ptr.s.ptr) {
			cvmx_dprintf("head_ptr.s.ptr != tail_ptr.s.ptr, idx: %d\n", i);
		}

		cvmx_phys_to_ptr(((uint64_t) head_ptr.s.ptr) << 7);
		/* Leak the memory */
	}

	do {
		do {
			pop_fptr.u64 = cvmx_read_csr(CVMX_SSO_RWQ_POP_FPTR);
			if (pop_fptr.s.val) {
				cvmx_phys_to_ptr(((uint64_t) pop_fptr.s.fptr) << 7);
				/* Leak the memory */
			}
		} while (pop_fptr.s.val);
		fpage_cnt.u64 = cvmx_read_csr(CVMX_SSO_FPAGE_CNT);
	} while (fpage_cnt.s.fpage_cnt);

	sso_cfg.s.rwen = 0;
	sso_cfg.s.rwq_byp_dis = 0;
	cvmx_write_csr(CVMX_SSO_CFG, sso_cfg.u64);

	return 0;
}
#endif /* CVMX_BUILD_FOR_LINUX_KERNEL */
/**
 * Initialize the PIP, IPD, and PKO hardware to support
 * simple priority based queues for the ethernet ports. Each
 * port is configured with a number of priority queues based
 * on CVMX_PKO_QUEUES_PER_PORT_* where each queue is lower
 * priority than the previous.
 *
 * @return Zero on success, non-zero on failure
 */
int cvmx_helper_initialize_packet_io_global(void)
{
	int result = 0;
	int interface;
	union cvmx_l2c_cfg l2c_cfg;
#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	union cvmx_smix_en smix_en;
#endif
	const int num_interfaces = cvmx_helper_get_number_of_interfaces();

	/*
	 * CN52XX pass 1: Due to a bug in 2nd order CDR, it needs to
	 * be disabled.
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN52XX_PASS1_0))
		__cvmx_helper_errata_qlm_disable_2nd_order_cdr(1);

	/*
	 * Tell L2 to give the IOB statically higher priority compared
	 * to the cores. This avoids conditions where IO blocks might
	 * be starved under very high L2 loads.
	 */
	if (OCTEON_IS_OCTEON2() || OCTEON_IS_OCTEON3()) {
		union cvmx_l2c_ctl l2c_ctl;
		l2c_ctl.u64 = cvmx_read_csr(CVMX_L2C_CTL);
		l2c_ctl.s.rsp_arb_mode = 1;
		l2c_ctl.s.xmc_arb_mode = 0;
		cvmx_write_csr(CVMX_L2C_CTL, l2c_ctl.u64);
	} else {
		l2c_cfg.u64 = cvmx_read_csr(CVMX_L2C_CFG);
		l2c_cfg.s.lrf_arb_mode = 0;
		l2c_cfg.s.rfb_arb_mode = 0;
		cvmx_write_csr(CVMX_L2C_CFG, l2c_cfg.u64);
	}

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	if (cvmx_sysinfo_get()->board_type != CVMX_BOARD_TYPE_SIM) {
		int smi_inf = 1;
		int i;

		/* Newer chips have more than one SMI/MDIO interface */
		if (OCTEON_IS_MODEL(OCTEON_CN68XX))
			smi_inf = 4;
		else if (!OCTEON_IS_MODEL(OCTEON_CN3XXX)
			 && !OCTEON_IS_MODEL(OCTEON_CN58XX)
			 && !OCTEON_IS_MODEL(OCTEON_CN50XX))
			smi_inf = 2;

		for (i = 0; i < smi_inf; i++) {
			/* Make sure SMI/MDIO is enabled so we can query PHYs */
			smix_en.u64 = cvmx_read_csr(CVMX_SMIX_EN(i));
			if (!smix_en.s.en) {
				smix_en.s.en = 1;
				cvmx_write_csr(CVMX_SMIX_EN(i), smix_en.u64);
			}
		}
	}
#endif

	__cvmx_helper_init_port_valid();

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		if (cvmx_pki_initialize_data_structures(0))
			return -1;
	}


#endif

	for (interface = 0; interface < num_interfaces; interface++)
		result |= cvmx_helper_interface_probe(interface);

	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		__cvmx_helper_init_port_config_data();
		cvmx_pko_initialize_global_78xx(0);
	} else
		cvmx_pko_initialize_global();

	for (interface = 0; interface < num_interfaces; interface++) {
		if (cvmx_helper_ports_on_interface(interface) > 0) {
			cvmx_dprintf("Interface %d has %d ports (%s)\n",
				     interface,
				     cvmx_helper_ports_on_interface(interface),
				     cvmx_helper_interface_mode_to_string(cvmx_helper_interface_get_mode(interface)));
		}
		result |= __cvmx_helper_interface_setup_ipd(interface);
		if (OCTEON_IS_MODEL(OCTEON_CN78XX))
			cvmx_pko_setup_interface_78xx(0, interface);
		else if (!OCTEON_IS_MODEL(OCTEON_CN68XX))
			result |= __cvmx_helper_interface_setup_pko(interface);
	}

	if (OCTEON_IS_MODEL(OCTEON_CN78XX))
		result |= __cvmx_helper_global_setup_pki(0);
	else
		result |= __cvmx_helper_global_setup_ipd();

	if (!OCTEON_IS_MODEL(OCTEON_CN78XX))
		result |= __cvmx_helper_global_setup_pko();

	/* Enable any flow control and backpressure */
	result |= __cvmx_helper_global_setup_backpressure();

	/* export app config if set */
	if (cvmx_export_app_config) {
		result |= (*cvmx_export_app_config)();
	}

	if (cvmx_ipd_cfg.ipd_enable)
		result |= cvmx_helper_ipd_and_packet_input_enable();
	return result;
}
EXPORT_SYMBOL(cvmx_helper_initialize_packet_io_global);

/**
 * Does core local initialization for packet io
 *
 * @return Zero on success, non-zero on failure
 */
int cvmx_helper_initialize_packet_io_local(void)
{
	return cvmx_pko_initialize_local();
}

/**
 * wait for the pko queue to drain
 *
 * @param queue a valid pko queue
 * @return count is the length of the queue after calling this
 * function
 */
static int cvmx_helper_wait_pko_queue_drain(int queue)
{
	const int timeout = 5;	/* Wait up to 5 seconds for timeouts */
	int count;
	uint64_t start_cycle, stop_cycle;

	count = cvmx_cmd_queue_length(CVMX_CMD_QUEUE_PKO(queue));
	start_cycle = cvmx_get_cycle();
	stop_cycle = start_cycle + cvmx_clock_get_rate(CVMX_CLOCK_CORE) * timeout;
	while (count && (cvmx_get_cycle() < stop_cycle)) {
		cvmx_wait(10000);
		count = cvmx_cmd_queue_length(CVMX_CMD_QUEUE_PKO(queue));
	}

	return count;
}

struct cvmx_buffer_list {
	struct cvmx_buffer_list *next;
};

/**
 * Disables the sending of flow control (pause) frames on the specified
 * GMX port(s).
 *
 * @param interface Which interface (0 or 1)
 * @param port_mask Mask (4bits) of which ports on the interface to disable
 *                  backpressure on.
 *                  1 => disable backpressure
 *                  0 => enable backpressure
 *
 * @return 0 on success
 *         -1 on error
 */
int cvmx_gmx_set_backpressure_override(uint32_t interface, uint32_t port_mask)
{
	union cvmx_gmxx_tx_ovr_bp gmxx_tx_ovr_bp;
	/* Check for valid arguments */
	if (port_mask & ~0xf || interface & ~0x1)
		return -1;
	if (interface >= CVMX_HELPER_MAX_GMX)
		return -1;
	gmxx_tx_ovr_bp.u64 = 0;
	gmxx_tx_ovr_bp.s.en = port_mask;	/* Per port Enable back pressure override */
	gmxx_tx_ovr_bp.s.ign_full = port_mask;	/* Ignore the RX FIFO full when computing BP */
	cvmx_write_csr(CVMX_GMXX_TX_OVR_BP(interface), gmxx_tx_ovr_bp.u64);
	return 0;
}

/**
 * Disables the sending of flow control (pause) frames on the specified
 * AGL (RGMII) port(s).
 *
 * @param interface Which interface (0 or 1)
 * @param port_mask Mask (4bits) of which ports on the interface to disable
 *                  backpressure on.
 *                  1 => disable backpressure
 *                  0 => enable backpressure
 *
 * @return 0 on success
 *         -1 on error
 */
int cvmx_agl_set_backpressure_override(uint32_t interface, uint32_t port_mask)
{
	union cvmx_agl_gmx_tx_ovr_bp agl_gmx_tx_ovr_bp;
	int port = cvmx_helper_agl_get_port(interface);
	if (port == -1)
		return -1;
	/* Check for valid arguments */
	agl_gmx_tx_ovr_bp.u64 = 0;
	/* Per port Enable back pressure override */
	agl_gmx_tx_ovr_bp.s.en = port_mask;
	/* Ignore the RX FIFO full when computing BP */
	agl_gmx_tx_ovr_bp.s.ign_full = port_mask;
	cvmx_write_csr(CVMX_GMXX_TX_OVR_BP(port), agl_gmx_tx_ovr_bp.u64);
	return 0;
}

/**
 * Undo the initialization performed in
 * cvmx_helper_initialize_packet_io_global(). After calling this routine and the
 * local version on each core, packet IO for Octeon will be disabled and placed
 * in the initial reset state. It will then be safe to call the initialize
 * later on. Note that this routine does not empty the FPA pools. It frees all
 * buffers used by the packet IO hardware to the FPA so a function emptying the
 * FPA after shutdown should find all packet buffers in the FPA.
 *
 * @return Zero on success, negative on failure.
 */
int cvmx_helper_shutdown_packet_io_global(void)
{
	const int timeout = 5;	/* Wait up to 5 seconds for timeouts */
	int result = 0;
	int num_interfaces = cvmx_helper_get_number_of_interfaces();
	int interface;
	int num_ports;
	int index;
	struct cvmx_buffer_list *pool0_buffers;
	struct cvmx_buffer_list *pool0_buffers_tail;
	cvmx_wqe_t *work;
	union cvmx_ipd_ctl_status ipd_ctl_status;
    int packet_pool = (int)cvmx_fpa_get_packet_pool();
    uint64_t packet_pool_size = cvmx_fpa_get_packet_pool_block_size();
    int wqe_pool = (int)cvmx_fpa_get_wqe_pool();

    if (!OCTEON_IS_MODEL(OCTEON_CN78XX)) { /*vinita_to_do implement for 78xx*/
	/* Step 1: Disable all backpressure */
	for (interface = 0; interface < num_interfaces; interface++) {
		cvmx_helper_interface_mode_t mode = cvmx_helper_interface_get_mode(interface);
		if (mode == CVMX_HELPER_INTERFACE_MODE_AGL)
			cvmx_agl_set_backpressure_override(interface, 0x1);
		else if (mode != CVMX_HELPER_INTERFACE_MODE_DISABLED)
			cvmx_gmx_set_backpressure_override(interface, 0xf);
	}

step2:
	/* Step 2: Wait for the PKO queues to drain */
	if (octeon_has_feature(OCTEON_FEATURE_PKND)) {
		int queue, max_queue;

		max_queue = __cvmx_helper_cfg_pko_max_queue();
		for (queue = 0; queue < max_queue; queue++) {
			if (cvmx_helper_wait_pko_queue_drain(queue)) {
				result = -1;
				goto step3;
			}
		}
	} else {
		for (interface = 0; interface < num_interfaces; interface++) {
			num_ports = cvmx_helper_ports_on_interface(interface);
			for (index = 0; index < num_ports; index++) {
				int pko_port;
				int queue;
				int max_queue;
				if (!cvmx_helper_is_port_valid(interface, index))
					continue;
				pko_port = cvmx_helper_get_ipd_port(interface, index);
				queue = cvmx_pko_get_base_queue(pko_port);
				max_queue = queue + cvmx_pko_get_num_queues(pko_port);
				while (queue < max_queue) {
					if (cvmx_helper_wait_pko_queue_drain(queue)) {
						result = -1;
						goto step3;
					}
					queue++;
				}
			}
		}
	}

step3:
	/* Step 3: Disable TX and RX on all ports */
	for (interface = 0; interface < num_interfaces; interface++) {
		switch (cvmx_helper_interface_get_mode(interface)) {
		case CVMX_HELPER_INTERFACE_MODE_DISABLED:
		case CVMX_HELPER_INTERFACE_MODE_PCIE:
			/* Not a packet interface */
			break;
		case CVMX_HELPER_INTERFACE_MODE_NPI:
		case CVMX_HELPER_INTERFACE_MODE_SRIO:
		case CVMX_HELPER_INTERFACE_MODE_ILK:
			/*
			 * We don't handle the NPI/NPEI/SRIO packet
			 * engines. The caller must know these are
			 * idle.
			 */
			break;
		case CVMX_HELPER_INTERFACE_MODE_LOOP:
			/*
			 * Nothing needed. Once PKO is idle, the
			 * loopback devices must be idle.
			 */
			break;
		case CVMX_HELPER_INTERFACE_MODE_SPI:
			/*
			 * SPI cannot be disabled from Octeon. It is
			 * the responsibility of the caller to make
			 * sure SPI is idle before doing shutdown.
			 *
			 * Fall through and do the same processing as
			 * RGMII/GMII.
			 */
		case CVMX_HELPER_INTERFACE_MODE_GMII:
		case CVMX_HELPER_INTERFACE_MODE_RGMII:
			/* Disable outermost RX at the ASX block */
			cvmx_write_csr(CVMX_ASXX_RX_PRT_EN(interface), 0);
			num_ports = cvmx_helper_ports_on_interface(interface);
			if (num_ports > 4)
				num_ports = 4;
			for (index = 0; index < num_ports; index++) {
				union cvmx_gmxx_prtx_cfg gmx_cfg;
				if (!cvmx_helper_is_port_valid(interface, index))
					continue;
				gmx_cfg.u64 = cvmx_read_csr(CVMX_GMXX_PRTX_CFG(index, interface));
				gmx_cfg.s.en = 0;
				cvmx_write_csr(CVMX_GMXX_PRTX_CFG(index, interface), gmx_cfg.u64);
				/* Poll the GMX state machine waiting for it to become idle */
				cvmx_write_csr(CVMX_NPI_DBG_SELECT, interface * 0x800 + index * 0x100 + 0x880);
				if (CVMX_WAIT_FOR_FIELD64(CVMX_DBG_DATA, union cvmx_dbg_data,
							  data & 7, ==, 0, timeout * 1000000)) {
					cvmx_dprintf("GMX RX path timeout waiting for idle\n");
					result = -1;
				}
				if (CVMX_WAIT_FOR_FIELD64(CVMX_DBG_DATA, union cvmx_dbg_data,
							  data & 0xf, ==, 0, timeout * 1000000)) {
					cvmx_dprintf("GMX TX path timeout waiting for idle\n");
					result = -1;
				}
			}
			/* Disable outermost TX at the ASX block */
			cvmx_write_csr(CVMX_ASXX_TX_PRT_EN(interface), 0);
			/* Disable interrupts for interface */
			cvmx_write_csr(CVMX_ASXX_INT_EN(interface), 0);
			cvmx_write_csr(CVMX_GMXX_TX_INT_EN(interface), 0);
			break;
		case CVMX_HELPER_INTERFACE_MODE_XAUI:
		case CVMX_HELPER_INTERFACE_MODE_RXAUI:
		case CVMX_HELPER_INTERFACE_MODE_SGMII:
		case CVMX_HELPER_INTERFACE_MODE_QSGMII:
		case CVMX_HELPER_INTERFACE_MODE_PICMG:
			num_ports = cvmx_helper_ports_on_interface(interface);
			if (num_ports > 4)
				num_ports = 4;
			for (index = 0; index < num_ports; index++) {
				union cvmx_gmxx_prtx_cfg gmx_cfg;
				if (!cvmx_helper_is_port_valid(interface, index))
					continue;
				gmx_cfg.u64 = cvmx_read_csr(CVMX_GMXX_PRTX_CFG(index, interface));
				gmx_cfg.s.en = 0;
				cvmx_write_csr(CVMX_GMXX_PRTX_CFG(index, interface), gmx_cfg.u64);
				if (CVMX_WAIT_FOR_FIELD64(CVMX_GMXX_PRTX_CFG(index, interface),
							  union cvmx_gmxx_prtx_cfg, rx_idle, ==, 1, timeout * 1000000)) {
					cvmx_dprintf("GMX RX path timeout waiting for idle\n");
					result = -1;
				}
				if (CVMX_WAIT_FOR_FIELD64(CVMX_GMXX_PRTX_CFG(index, interface),
							  union cvmx_gmxx_prtx_cfg, tx_idle, ==, 1, timeout * 1000000)) {
					cvmx_dprintf("GMX TX path timeout waiting for idle\n");
					result = -1;
				}
			}
			break;
		case CVMX_HELPER_INTERFACE_MODE_AGL:
			{
				int port = cvmx_helper_agl_get_port(interface);
				union cvmx_agl_gmx_prtx_cfg agl_gmx_cfg;
				agl_gmx_cfg.u64 = cvmx_read_csr(CVMX_AGL_GMX_PRTX_CFG(port));
				agl_gmx_cfg.s.en = 0;
				cvmx_write_csr(CVMX_AGL_GMX_PRTX_CFG(port), agl_gmx_cfg.u64);
				if (CVMX_WAIT_FOR_FIELD64(CVMX_AGL_GMX_PRTX_CFG(port),
							  union cvmx_agl_gmx_prtx_cfg,
							  rx_idle, ==, 1,
							  timeout * 1000000)) {
					cvmx_dprintf("AGL RX path timeout waiting for idle\n");
					result = -1;
				}
				if (CVMX_WAIT_FOR_FIELD64(CVMX_AGL_GMX_PRTX_CFG(port),
							  union cvmx_agl_gmx_prtx_cfg,
							  tx_idle, ==, 1,
							  timeout * 1000000)) {
					cvmx_dprintf("AGL TX path timeout waiting for idle\n");
					result = -1;
				}
			}
			break;
		}
	}

	/* Step 4: Retrieve all packets from the POW and free them */
	while ((work = cvmx_pow_work_request_sync(CVMX_POW_WAIT))) {
		cvmx_helper_free_packet_data(work);
		cvmx_fpa_free(work, wqe_pool, 0);
	}

	/* Step 4b: Special workaround for pass 2 errata */
	if (OCTEON_IS_MODEL(OCTEON_CN38XX_PASS2)) {
		union cvmx_ipd_ptr_count ipd_cnt;
		int to_add;
		ipd_cnt.u64 = cvmx_read_csr(CVMX_IPD_PTR_COUNT);
		to_add = (ipd_cnt.s.wqev_cnt + ipd_cnt.s.wqe_pcnt) & 0x7;
		if (to_add) {
			int port = -1;
			cvmx_dprintf("Aligning CN38XX pass 2 IPD counters\n");
			if (cvmx_helper_interface_get_mode(0) == CVMX_HELPER_INTERFACE_MODE_RGMII)
				port = 0;
			else if (cvmx_helper_interface_get_mode(1) == CVMX_HELPER_INTERFACE_MODE_RGMII)
				port = 16;

			if (port != -1) {
				void *buffer = cvmx_fpa_alloc(packet_pool);
				if (buffer) {
					int queue = cvmx_pko_get_base_queue(port);
					cvmx_pko_command_word0_t pko_command;
					union cvmx_buf_ptr packet;
					uint64_t start_cycle;
					uint64_t stop_cycle;

					/* Populate a minimal packet */
					memset(buffer, 0xff, 6);
					memset(buffer + 6, 0, 54);
					pko_command.u64 = 0;
					pko_command.s.dontfree = 1;
					pko_command.s.total_bytes = 60;
					pko_command.s.segs = 1;
					packet.u64 = 0;
					packet.s.addr = cvmx_ptr_to_phys(buffer);
					packet.s.size = packet_pool_size;
					__cvmx_helper_rgmii_configure_loopback(port, 1, 0);
					while (to_add--) {
						cvmx_pko_send_packet_prepare(port, queue, CVMX_PKO_LOCK_CMD_QUEUE);
						if (cvmx_pko_send_packet_finish(port, queue, pko_command, packet, CVMX_PKO_LOCK_CMD_QUEUE)) {
							cvmx_dprintf("ERROR: Unable to align IPD counters (PKO failed)\n");
							break;
						}
					}
					cvmx_fpa_free(buffer, packet_pool, 0);

					/* Wait for the packets to loop back */
					start_cycle = cvmx_get_cycle();
					stop_cycle = start_cycle + cvmx_clock_get_rate(CVMX_CLOCK_CORE) * timeout;
					while (cvmx_cmd_queue_length(CVMX_CMD_QUEUE_PKO(queue)) && (cvmx_get_cycle() < stop_cycle))
						cvmx_wait(1000);

					cvmx_wait(1000);
					__cvmx_helper_rgmii_configure_loopback(port, 0, 0);
					if (to_add == -1)
						goto step2;
				} else {
					cvmx_dprintf("ERROR: Unable to align IPD counters (Packet pool empty)\n");
				}
			} else {
				cvmx_dprintf("ERROR: Unable to align IPD counters\n");
			}
		}
	}

	/* Step 5 */
	cvmx_ipd_disable();

	/* Step 6: Drain all prefetched buffers from IPD/PIP. Note that IPD/PIP
	   have not been reset yet */
	__cvmx_ipd_free_ptr();

	/* Step 7: Free the PKO command buffers and put PKO in reset */
	cvmx_pko_shutdown();

	/* Step 8: Disable MAC address filtering */
	for (interface = 0; interface < num_interfaces; interface++) {
		switch (cvmx_helper_interface_get_mode(interface)) {
		case CVMX_HELPER_INTERFACE_MODE_DISABLED:
		case CVMX_HELPER_INTERFACE_MODE_PCIE:
		case CVMX_HELPER_INTERFACE_MODE_SRIO:
		case CVMX_HELPER_INTERFACE_MODE_ILK:
		case CVMX_HELPER_INTERFACE_MODE_NPI:
		case CVMX_HELPER_INTERFACE_MODE_LOOP:
			break;
		case CVMX_HELPER_INTERFACE_MODE_XAUI:
		case CVMX_HELPER_INTERFACE_MODE_RXAUI:
		case CVMX_HELPER_INTERFACE_MODE_GMII:
		case CVMX_HELPER_INTERFACE_MODE_RGMII:
		case CVMX_HELPER_INTERFACE_MODE_SPI:
		case CVMX_HELPER_INTERFACE_MODE_SGMII:
		case CVMX_HELPER_INTERFACE_MODE_QSGMII:
		case CVMX_HELPER_INTERFACE_MODE_PICMG:
			num_ports = cvmx_helper_ports_on_interface(interface);
			if (num_ports > 4)
				num_ports = 4;
			for (index = 0; index < num_ports; index++) {
				if (!cvmx_helper_is_port_valid(interface, index))
					continue;
				cvmx_write_csr(CVMX_GMXX_RXX_ADR_CTL(index, interface), 1);
				cvmx_write_csr(CVMX_GMXX_RXX_ADR_CAM_EN(index, interface), 0);
				cvmx_write_csr(CVMX_GMXX_RXX_ADR_CAM0(index, interface), 0);
				cvmx_write_csr(CVMX_GMXX_RXX_ADR_CAM1(index, interface), 0);
				cvmx_write_csr(CVMX_GMXX_RXX_ADR_CAM2(index, interface), 0);
				cvmx_write_csr(CVMX_GMXX_RXX_ADR_CAM3(index, interface), 0);
				cvmx_write_csr(CVMX_GMXX_RXX_ADR_CAM4(index, interface), 0);
				cvmx_write_csr(CVMX_GMXX_RXX_ADR_CAM5(index, interface), 0);
			}
			break;
		case CVMX_HELPER_INTERFACE_MODE_AGL:
			{
				int port = cvmx_helper_agl_get_port(interface);
				cvmx_write_csr(CVMX_AGL_GMX_RXX_ADR_CTL(port), 1);
				cvmx_write_csr(CVMX_AGL_GMX_RXX_ADR_CAM_EN(port), 0);
				cvmx_write_csr(CVMX_AGL_GMX_RXX_ADR_CAM0(port), 0);
				cvmx_write_csr(CVMX_AGL_GMX_RXX_ADR_CAM1(port), 0);
				cvmx_write_csr(CVMX_AGL_GMX_RXX_ADR_CAM2(port), 0);
				cvmx_write_csr(CVMX_AGL_GMX_RXX_ADR_CAM3(port), 0);
				cvmx_write_csr(CVMX_AGL_GMX_RXX_ADR_CAM4(port), 0);
				cvmx_write_csr(CVMX_AGL_GMX_RXX_ADR_CAM5(port), 0);
			}
			break;
		}
	}

	/*
	 * Step 9: Drain all FPA buffers out of pool 0 before we reset
	 * IPD/PIP.  This is needed to keep IPD_QUE0_FREE_PAGE_CNT in
	 * sync. We temporarily keep the buffers in the pool0_buffers
	 * list.
	 */
	pool0_buffers = NULL;
	pool0_buffers_tail = NULL;
	while (1) {
		struct cvmx_buffer_list *buffer = cvmx_fpa_alloc(0);
		if (buffer) {
			buffer->next = NULL;

			if (pool0_buffers == NULL)
				pool0_buffers = buffer;
			else
				pool0_buffers_tail->next = buffer;

			pool0_buffers_tail = buffer;
		} else {
			break;
		}
	}

	/* Step 10: Reset IPD and PIP */
	ipd_ctl_status.u64 = cvmx_read_csr(CVMX_IPD_CTL_STATUS);
	ipd_ctl_status.s.reset = 1;
	cvmx_write_csr(CVMX_IPD_CTL_STATUS, ipd_ctl_status.u64);

	if ((cvmx_sysinfo_get()->board_type != CVMX_BOARD_TYPE_SIM) &&
	    (OCTEON_IS_MODEL(OCTEON_CN3XXX) ||
	     OCTEON_IS_MODEL(OCTEON_CN5XXX))) {
		/*
		 * only try 1000 times.  Normally if this works it
		 * will happen in the first 50 loops.
		 */
		int max_loops = 1000;
		int loop = 0;
		/*
		 * Per port backpressure counters can get misaligned
		 * after an IPD reset. This code realigns them by
		 * performing repeated resets. See IPD-13473.
		 */
		cvmx_wait(100);
		if (__cvmx_helper_backpressure_is_misaligned()) {
			cvmx_dprintf("Starting to align per port backpressure counters.\n");
			while (__cvmx_helper_backpressure_is_misaligned() &&
			       (loop++ < max_loops)) {
				cvmx_write_csr(CVMX_IPD_CTL_STATUS,
					       ipd_ctl_status.u64);
				cvmx_wait(123);
			}
			if (loop < max_loops)
				cvmx_dprintf("Completed aligning per port backpressure counters (%d loops).\n", loop);
			else
				cvmx_dprintf("ERROR: unable to align per port backpressure counters.\n");
			/* For now, don't hang.... */
		}
	}

	/* PIP_SFT_RST not present in CN38XXp{1,2} */
	if (!OCTEON_IS_MODEL(OCTEON_CN38XX_PASS2)) {
		union cvmx_pip_sft_rst pip_sft_rst;
		pip_sft_rst.u64 = cvmx_read_csr(CVMX_PIP_SFT_RST);
		pip_sft_rst.s.rst = 1;
		cvmx_write_csr(CVMX_PIP_SFT_RST, pip_sft_rst.u64);
	}

	/* Make sure IPD has finished reset. */
	if (OCTEON_IS_OCTEON2() || OCTEON_IS_MODEL(OCTEON_CN70XX)) {
		if (CVMX_WAIT_FOR_FIELD64(CVMX_IPD_CTL_STATUS,
					  union cvmx_ipd_ctl_status, rst_done, ==, 0, 1000)) {
			cvmx_dprintf("IPD reset timeout waiting for idle\n");
			result = -1;
		}
	}

	/* Step 11: Restore the FPA buffers into pool 0 */
	while (pool0_buffers) {
		struct cvmx_buffer_list *n = pool0_buffers->next;
		cvmx_fpa_free(pool0_buffers, 0, 0);
		pool0_buffers = n;
	}

	/* Step 12: Release interface structures */
	__cvmx_helper_shutdown_interfaces();
    }

    return result;
}

EXPORT_SYMBOL(cvmx_helper_shutdown_packet_io_global);

int cvmx_helper_shutdown_fpa_pools(int node)
{
	int result = 0;
#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	uint64_t pool;

	if (!OCTEON_IS_MODEL(OCTEON_CN78XX)) { /*vinita_to_do implement for 78xx */
		for (pool = 0; pool < CVMX_FPA_NUM_POOLS; pool++) {
			if (cvmx_fpa_get_block_size(pool) > 0)
				result |= cvmx_fpa_shutdown_pool(pool);
		}
	}
#endif
	return result;
}

/**
 * Does core local shutdown of packet io
 *
 * @return Zero on success, non-zero on failure
 */
int cvmx_helper_shutdown_packet_io_local(void)
{
	/*
	 * Currently there is nothing to do per core. This may change
	 * in the future.
	 */
	return 0;
}

/**
 * Auto configure an IPD/PKO port link state and speed. This
 * function basically does the equivalent of:
 * cvmx_helper_link_set(ipd_port, cvmx_helper_link_get(ipd_port));
 *
 * @param ipd_port IPD/PKO port to auto configure
 *
 * @return Link state after configure
 */
cvmx_helper_link_info_t cvmx_helper_link_autoconf(int ipd_port)
{
	cvmx_helper_link_info_t link_info;
	int interface = cvmx_helper_get_interface_num(ipd_port);
	int index = cvmx_helper_get_interface_index_num(ipd_port);

	if (interface == -1 || index == -1 ||
	    index >= cvmx_helper_ports_on_interface(interface)) {
		link_info.u64 = 0;
		return link_info;
	}

	link_info = cvmx_helper_link_get(ipd_port);
	if (link_info.u64 == (__cvmx_helper_get_link_info(interface, index)).u64)
		return link_info;

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	if (!link_info.s.link_up)
		cvmx_error_disable_group(CVMX_ERROR_GROUP_ETHERNET, ipd_port);
#endif

	/* If we fail to set the link speed, port_link_info will not change */
	cvmx_helper_link_set(ipd_port, link_info);

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	if (link_info.s.link_up)
		cvmx_error_enable_group(CVMX_ERROR_GROUP_ETHERNET, ipd_port);
#endif

	return link_info;
}
EXPORT_SYMBOL(cvmx_helper_link_autoconf);

/**
 * Return the link state of an IPD/PKO port as returned by
 * auto negotiation. The result of this function may not match
 * Octeon's link config if auto negotiation has changed since
 * the last call to cvmx_helper_link_set().
 *
 * @param ipd_port IPD/PKO port to query
 *
 * @return Link state
 */
cvmx_helper_link_info_t cvmx_helper_link_get(int ipd_port)
{
	cvmx_helper_link_info_t result;
	int interface = cvmx_helper_get_interface_num(ipd_port);
	int index = cvmx_helper_get_interface_index_num(ipd_port);

	/*
	 * The default result will be a down link unless the code
	 * below changes it.
	 */
	result.u64 = 0;

	if (interface == -1 || index == -1 ||
	    index >= cvmx_helper_ports_on_interface(interface))
		return result;

	cvmx_helper_interface_get_mode(interface);
	if (iface_ops[interface]->link_get)
		result = iface_ops[interface]->link_get(ipd_port);

	return result;
}
EXPORT_SYMBOL(cvmx_helper_link_get);

/**
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
int cvmx_helper_link_set(int ipd_port, cvmx_helper_link_info_t link_info)
{
	int result = -1;
	int interface = cvmx_helper_get_interface_num(ipd_port);
	int index = cvmx_helper_get_interface_index_num(ipd_port);

	if (interface == -1 || index == -1 ||
	    index >= cvmx_helper_ports_on_interface(interface))
		return -1;

	cvmx_helper_interface_get_mode(interface);
	if (iface_ops[interface]->link_set)
		result = iface_ops[interface]->link_set(ipd_port, link_info);

	/*
	 * Set the port_link_info here so that the link status is
	 * updated no matter how cvmx_helper_link_set is called. We
	 * don't change the value if link_set failed.
	 */
	if (result == 0)
		__cvmx_helper_set_link_info(interface, index, link_info);
	return result;
}
EXPORT_SYMBOL(cvmx_helper_link_set);

/**
 * Configure a port for internal and/or external loopback. Internal loopback
 * causes packets sent by the port to be received by Octeon. External loopback
 * causes packets received from the wire to sent out again.
 *
 * @param ipd_port IPD/PKO port to loopback.
 * @param enable_internal
 *                 Non zero if you want internal loopback
 * @param enable_external
 *                 Non zero if you want external loopback
 *
 * @return Zero on success, negative on failure.
 */
int cvmx_helper_configure_loopback(int ipd_port, int enable_internal,
				   int enable_external)
{
	int result = -1;
	int interface = cvmx_helper_get_interface_num(ipd_port);
	int index = cvmx_helper_get_interface_index_num(ipd_port);

	if (index >= cvmx_helper_ports_on_interface(interface))
		return -1;

	cvmx_helper_interface_get_mode(interface);
	if (iface_ops[interface]->loopback)
		result = iface_ops[interface]->loopback(ipd_port,
							enable_internal,
							enable_external);

	return result;
}

void cvmx_helper_setup_simulator_io_buffer_counts(int node, int num_packet_buffers,
					    int pko_buffers)
{
	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		cvmx_pki_set_default_pool_buffer_count(node, num_packet_buffers);

	} else {
		cvmx_ipd_set_packet_pool_buffer_count(num_packet_buffers);
		cvmx_ipd_set_wqe_pool_buffer_count(num_packet_buffers);
		cvmx_pko_set_cmd_queue_pool_buffer_count(pko_buffers);
	}
}

void cvmx_helper_set_wqe_no_ptr_mode(bool mode)
{
	if (!OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		cvmx_ipd_ctl_status_t ipd_ctl_status;
		ipd_ctl_status.u64 = cvmx_read_csr(CVMX_IPD_CTL_STATUS);
		ipd_ctl_status.s.no_wptr = mode;
		cvmx_write_csr(CVMX_IPD_CTL_STATUS, ipd_ctl_status.u64);
	}
}

void cvmx_helper_set_pkt_wqe_le_mode(bool mode)
{
	if (!OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		cvmx_ipd_ctl_status_t ipd_ctl_status;
		ipd_ctl_status.u64 = cvmx_read_csr(CVMX_IPD_CTL_STATUS);
		ipd_ctl_status.s.pkt_lend = mode;
		ipd_ctl_status.s.wqe_lend = mode;
		cvmx_write_csr(CVMX_IPD_CTL_STATUS, ipd_ctl_status.u64);
	}
}
