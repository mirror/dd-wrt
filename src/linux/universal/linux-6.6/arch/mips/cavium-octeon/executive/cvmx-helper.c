/***********************license start***************
 * Copyright (c) 2003-2015  Cavium Inc. (support@cavium.com). All rights
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
#include <linux/slab.h>
#include <linux/export.h>
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-bootmem.h>
#include <asm/octeon/cvmx-bgxx-defs.h>
#include <asm/octeon/cvmx-sriox-defs.h>
#include <asm/octeon/cvmx-npi-defs.h>
#include <asm/octeon/cvmx-mio-defs.h>
#include <asm/octeon/cvmx-pcsx-defs.h>
#include <asm/octeon/cvmx-pexp-defs.h>
#include <asm/octeon/cvmx-pip-defs.h>
#include <asm/octeon/cvmx-asxx-defs.h>
#include <asm/octeon/cvmx-gmxx-defs.h>
#include <asm/octeon/cvmx-gserx-defs.h>
#include <asm/octeon/cvmx-smix-defs.h>
#include <asm/octeon/cvmx-dbg-defs.h>
#include <asm/octeon/cvmx-sso-defs.h>

#include <asm/octeon/cvmx-agl.h>
#include <asm/octeon/cvmx-gmx.h>
#include <asm/octeon/cvmx-fpa.h>
#include <asm/octeon/cvmx-pip.h>
#include <asm/octeon/cvmx-hwpko.h>
#include <asm/octeon/cvmx-pko3.h>
#include <asm/octeon/cvmx-ipd.h>
#include <asm/octeon/cvmx-qlm.h>
#include <asm/octeon/cvmx-spi.h>
#include <asm/octeon/cvmx-clock.h>
#include <asm/octeon/cvmx-helper.h>
#include <asm/octeon/cvmx-helper-bgx.h>
#include <asm/octeon/cvmx-helper-board.h>
#include <asm/octeon/cvmx-helper-errata.h>
#include <asm/octeon/cvmx-helper-cfg.h>
#include <asm/octeon/cvmx-helper-pki.h>
#include <asm/octeon/cvmx-helper-fpa.h>
#include <asm/octeon/cvmx-pki.h>
#include <asm/octeon/cvmx-helper-pko.h>
#include <asm/octeon/cvmx-helper-pko3.h>
#include <asm/octeon/cvmx-helper-ipd.h>
#elif defined(__U_BOOT__)
#include <common.h>
#include <asm/arch/cvmx.h>
#include <asm/arch/cvmx-sysinfo.h>
#include <asm/arch/cvmx-bootmem.h>
#include <asm/arch/cvmx-version.h>
#include <asm/arch/cvmx-gmx.h>
#include <asm/arch/cvmx-error.h>

#include <asm/arch/cvmx-fpa.h>
#include <asm/arch/cvmx-pip.h>
#include <asm/arch/cvmx-pko.h>
#include <asm/arch/cvmx-pko3.h>
#include <asm/arch/cvmx-ipd.h>
#include <asm/arch/cvmx-qlm.h>
#include <asm/arch/cvmx-spi.h>
#include <asm/arch/cvmx-helper.h>
#include <asm/arch/cvmx-helper-bgx.h>
#include <asm/arch/cvmx-helper-board.h>
#include <asm/arch/cvmx-helper-errata.h>
#include <asm/arch/cvmx-helper-cfg.h>
#include <asm/arch/cvmx-helper-pki.h>
#include <asm/arch/cvmx-helper-fpa.h>
#include <asm/arch/cvmx-pki.h>
#include <asm/arch/cvmx-helper-pko.h>
#include <asm/arch/cvmx-helper-pko3.h>
#include <asm/arch/cvmx-helper-ipd.h>
#include <asm/arch/cvmx-helper-fdt.h>
#include <asm/arch/cvmx-helper-sfp.h>
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
#include "cvmx-helper-bgx.h"
#include "cvmx-helper-board.h"
#include "cvmx-helper-errata.h"
#include "cvmx-helper-cfg.h"
#include "cvmx-helper-pki.h"
#include "cvmx-helper-fpa.h"
#include "cvmx-pki.h"
#include "cvmx-helper-pko.h"
#include "cvmx-helper-pko3.h"
#include "cvmx-helper-ipd.h"
#include "libfdt/cvmx-helper-fdt.h"
#include "cvmx-helper-sfp.h"
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
struct iface_ops {
	cvmx_helper_interface_mode_t	mode;
	int				(*enumerate)(int xiface);
	int				(*probe)(int xiface);
	int				(*enable)(int xiface);
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
static const struct iface_ops iface_ops_dis = {
	.mode		= CVMX_HELPER_INTERFACE_MODE_DISABLED,
};

/**
 * @INTERNAL
 * This structure specifies the interface methods used by interfaces
 * configured as gmii.
 */
static const struct iface_ops iface_ops_gmii = {
	.mode		= CVMX_HELPER_INTERFACE_MODE_GMII,
	.enumerate	= __cvmx_helper_rgmii_probe,
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
static const struct iface_ops iface_ops_rgmii = {
	.mode		= CVMX_HELPER_INTERFACE_MODE_RGMII,
	.enumerate	= __cvmx_helper_rgmii_probe,
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
static const struct iface_ops iface_ops_sgmii = {
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
static const struct iface_ops iface_ops_bgx_sgmii = {
	.mode		= CVMX_HELPER_INTERFACE_MODE_SGMII,
	.enumerate	= __cvmx_helper_bgx_enumerate,
	.probe		= __cvmx_helper_bgx_probe,
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
static const struct iface_ops iface_ops_qsgmii = {
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
static const struct iface_ops iface_ops_xaui = {
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
 * configured as xaui using the gmx mac.
 */
static const struct iface_ops iface_ops_bgx_xaui = {
	.mode		= CVMX_HELPER_INTERFACE_MODE_XAUI,
	.enumerate	= __cvmx_helper_bgx_enumerate,
	.probe		= __cvmx_helper_bgx_probe,
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
static const struct iface_ops iface_ops_rxaui = {
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
 * configured as xaui using the gmx mac.
 */
static const struct iface_ops iface_ops_bgx_rxaui = {
	.mode		= CVMX_HELPER_INTERFACE_MODE_RXAUI,
	.enumerate	= __cvmx_helper_bgx_enumerate,
	.probe		= __cvmx_helper_bgx_probe,
	.enable		= __cvmx_helper_bgx_xaui_enable,
	.link_get	= __cvmx_helper_bgx_xaui_link_get,
	.link_set	= __cvmx_helper_bgx_xaui_link_set,
	.loopback	= __cvmx_helper_bgx_xaui_configure_loopback,
};

/**
 * @INTERNAL
 * This structure specifies the interface methods used by interfaces
 * configured as xlaui.
 */
static const struct iface_ops iface_ops_bgx_xlaui = {
	.mode		= CVMX_HELPER_INTERFACE_MODE_XLAUI,
	.enumerate	= __cvmx_helper_bgx_enumerate,
	.probe		= __cvmx_helper_bgx_probe,
	.enable		= __cvmx_helper_bgx_xaui_enable,
	.link_get	= __cvmx_helper_bgx_xaui_link_get,
	.link_set	= __cvmx_helper_bgx_xaui_link_set,
	.loopback	= __cvmx_helper_bgx_xaui_configure_loopback,
};

/**
 * @INTERNAL
 * This structure specifies the interface methods used by interfaces
 * configured as xfi.
 */
static const struct iface_ops iface_ops_bgx_xfi = {
	.mode		= CVMX_HELPER_INTERFACE_MODE_XFI,
	.enumerate	= __cvmx_helper_bgx_enumerate,
	.probe		= __cvmx_helper_bgx_probe,
	.enable		= __cvmx_helper_bgx_xaui_enable,
	.link_get	= __cvmx_helper_bgx_xaui_link_get,
	.link_set	= __cvmx_helper_bgx_xaui_link_set,
	.loopback	= __cvmx_helper_bgx_xaui_configure_loopback,
};

static const struct iface_ops iface_ops_bgx_10G_KR = {
	.mode		= CVMX_HELPER_INTERFACE_MODE_10G_KR,
	.enumerate	= __cvmx_helper_bgx_enumerate,
	.probe		= __cvmx_helper_bgx_probe,
	.enable		= __cvmx_helper_bgx_xaui_enable,
	.link_get	= __cvmx_helper_bgx_xaui_link_get,
	.link_set	= __cvmx_helper_bgx_xaui_link_set,
	.loopback	= __cvmx_helper_bgx_xaui_configure_loopback,
};

static const struct iface_ops iface_ops_bgx_40G_KR4 = {
	.mode		= CVMX_HELPER_INTERFACE_MODE_40G_KR4,
	.enumerate	= __cvmx_helper_bgx_enumerate,
	.probe		= __cvmx_helper_bgx_probe,
	.enable		= __cvmx_helper_bgx_xaui_enable,
	.link_get	= __cvmx_helper_bgx_xaui_link_get,
	.link_set	= __cvmx_helper_bgx_xaui_link_set,
	.loopback	= __cvmx_helper_bgx_xaui_configure_loopback,
};

/**
 * @INTERNAL
 * This structure specifies the interface methods used by interfaces
 * configured as ilk.
 */
static const struct iface_ops iface_ops_ilk = {
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
static const struct iface_ops iface_ops_npi = {
	.mode		= CVMX_HELPER_INTERFACE_MODE_NPI,
	.enumerate	= __cvmx_helper_npi_probe,
	.probe		= __cvmx_helper_npi_probe,
	.enable		= __cvmx_helper_npi_enable,
};

/**
 * @INTERNAL
 * This structure specifies the interface methods used by interfaces
 * configured as srio.
 */
static const struct iface_ops iface_ops_srio = {
	.mode		= CVMX_HELPER_INTERFACE_MODE_SRIO,
	.enumerate	= __cvmx_helper_srio_probe,
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
static const struct iface_ops iface_ops_agl = {
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
static const struct iface_ops iface_ops_picmg = {
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
static const struct iface_ops iface_ops_spi = {
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
 * configured as mixed mode, some ports are sgmii and some are xfi.
 */
static const struct iface_ops iface_ops_bgx_mixed = {
	.mode		= CVMX_HELPER_INTERFACE_MODE_MIXED,
	.enumerate	= __cvmx_helper_bgx_enumerate,
	.probe		= __cvmx_helper_bgx_probe,
	.enable		= __cvmx_helper_bgx_mixed_enable,
	.link_get	= __cvmx_helper_bgx_mixed_link_get,
	.link_set	= __cvmx_helper_bgx_mixed_link_set,
	.loopback	= __cvmx_helper_bgx_mixed_configure_loopback,
};

/**
 * @INTERNAL
 * This structure specifies the interface methods used by interfaces
 * configured as loop.
 */
static const struct iface_ops iface_ops_loop = {
	.mode		= CVMX_HELPER_INTERFACE_MODE_LOOP,
	.enumerate	= __cvmx_helper_loop_enumerate,
	.probe		= __cvmx_helper_loop_probe,
};

CVMX_SHARED const struct iface_ops *iface_node_ops[CVMX_MAX_NODES][CVMX_HELPER_MAX_IFACE];
#define iface_ops iface_node_ops[0]

struct cvmx_iface {
	int cvif_ipd_nports;
	int cvif_has_fcs;	/* PKO fcs for this interface. */
	enum cvmx_pko_padding cvif_padding;
	cvmx_helper_link_info_t *cvif_ipd_port_link_info;
};

/*
 * This has to be static as u-boot expects to probe an interface and
 * gets the number of its ports.
 */
static CVMX_SHARED struct cvmx_iface cvmx_interfaces[CVMX_MAX_NODES][CVMX_HELPER_MAX_IFACE];



int __cvmx_helper_get_num_ipd_ports(int xiface)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	struct cvmx_iface *piface;

	if (xi.interface >= cvmx_helper_get_number_of_interfaces())
		return -1;

	piface = &cvmx_interfaces[xi.node][xi.interface];
	return piface->cvif_ipd_nports;
}

enum cvmx_pko_padding __cvmx_helper_get_pko_padding(int xiface)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	struct cvmx_iface *piface;

	if (xi.interface >= cvmx_helper_get_number_of_interfaces())
		return CVMX_PKO_PADDING_NONE;

	piface = &cvmx_interfaces[xi.node][xi.interface];
	return piface->cvif_padding;
}
EXPORT_SYMBOL(__cvmx_helper_get_pko_padding);

int __cvmx_helper_init_interface(int xiface, int num_ipd_ports,
				 int has_fcs, enum cvmx_pko_padding pad)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	struct cvmx_iface *piface;
	cvmx_helper_link_info_t *p;
	int i;
	int sz;
#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	uint64_t addr;
	char name[32];
#endif

	if (xi.interface >= cvmx_helper_get_number_of_interfaces())
		return -1;

	piface = &cvmx_interfaces[xi.node][xi.interface];
	piface->cvif_ipd_nports = num_ipd_ports;
	piface->cvif_padding = pad;

	piface->cvif_has_fcs = has_fcs;

	/*
	 * allocate the per-ipd_port link_info structure
	 */
	sz = piface->cvif_ipd_nports * sizeof(cvmx_helper_link_info_t);
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
	if (sz == 0)
		sz = sizeof(cvmx_helper_link_info_t);
	piface->cvif_ipd_port_link_info = (cvmx_helper_link_info_t *) kmalloc(sz, GFP_KERNEL);
	if (ZERO_OR_NULL_PTR(piface->cvif_ipd_port_link_info))
		panic("Cannot allocate memory in __cvmx_helper_init_interface.");
#else
	snprintf(name, sizeof(name), "__int_%d_link_info", xi.interface);
	addr = CAST64(cvmx_bootmem_alloc_named_range_once(sz, 0, 0,
		__alignof(cvmx_helper_link_info_t), name, NULL));
	piface->cvif_ipd_port_link_info = (cvmx_helper_link_info_t *) __cvmx_phys_addr_to_ptr(addr, sz);
#endif
	if (!piface->cvif_ipd_port_link_info) {
		if (sz != 0)
			cvmx_dprintf("iface %d failed to alloc link info\n",
				     xi.interface);
		return -1;
	}

	/* Initialize them */
	p = piface->cvif_ipd_port_link_info;

	for (i = 0; i < piface->cvif_ipd_nports; i++) {
		(*p).u64 = 0;
		p++;
	}
	return 0;
}


/*
 * Shut down the interfaces; free the resources.
 * @INTERNAL
 */
void __cvmx_helper_shutdown_interfaces_node(unsigned int node)
{
	int i;
	int nifaces;		/* number of interfaces */
	struct cvmx_iface *piface;

	nifaces = cvmx_helper_get_number_of_interfaces();
	for (i = 0; i < nifaces; i++) {
		piface = &cvmx_interfaces[node][i];
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
		if (piface->cvif_ipd_port_link_info)
			kfree(piface->cvif_ipd_port_link_info);
#elif defined(__U_BOOT__) && 0
		if (piface->cvif_ipd_port_link_info) {
			char name[32];
			snprintf(name, sizeof(name),
				 "__int_%d_link_info", i);
			cvmx_bootmem_phy_named_block_free(name, 0);
		}
#else
		/*
		 * For SE apps, bootmem was meant to be allocated and never
		 * freed.
		 */
#endif
		piface->cvif_ipd_port_link_info = 0;
	}
}

void __cvmx_helper_shutdown_interfaces(void)
{
	unsigned int node = cvmx_get_node_num();
	__cvmx_helper_shutdown_interfaces_node(node);
}

int __cvmx_helper_set_link_info(int xiface, int index, cvmx_helper_link_info_t link_info)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	struct cvmx_iface *piface;

	if (xi.interface >= cvmx_helper_get_number_of_interfaces())
		return -1;

	piface = &cvmx_interfaces[xi.node][xi.interface];

	if (piface->cvif_ipd_port_link_info) {
		piface->cvif_ipd_port_link_info[index] = link_info;
		return 0;
	}

	return -1;
}

cvmx_helper_link_info_t __cvmx_helper_get_link_info(int xiface, int port)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	struct cvmx_iface *piface;
	cvmx_helper_link_info_t err;

	err.u64 = 0;

	if (xi.interface >= cvmx_helper_get_number_of_interfaces())
		return err;
	piface = &cvmx_interfaces[xi.node][xi.interface];

	if (piface->cvif_ipd_port_link_info)
		return piface->cvif_ipd_port_link_info[port];

	return err;
}

/**
 * Returns if FCS is enabled for the specified interface and port
 *
 * @param xiface - interface to check
 *
 * @return zero if FCS is not used, otherwise FCS is used.
 */
int __cvmx_helper_get_has_fcs(int xiface)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	return cvmx_interfaces[xi.node][xi.interface].cvif_has_fcs;
}
EXPORT_SYMBOL(__cvmx_helper_get_has_fcs);

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
	else if (OCTEON_IS_MODEL(OCTEON_CNF75XX))
		return 5;
	else if (OCTEON_IS_MODEL(OCTEON_CN73XX))
		return 5;
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
 * @param xiface xiface to get the port count for
 *
 * @return Number of ports on interface. Can be Zero.
 */
int cvmx_helper_ports_on_interface(int xiface)
{
	if (octeon_has_feature(OCTEON_FEATURE_PKND))
		return cvmx_helper_interface_enumerate(xiface);
	else
		return __cvmx_helper_get_num_ipd_ports(xiface);

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
static cvmx_helper_interface_mode_t __cvmx_get_mode_cn78xx(int xiface)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	/* SGMII/RXAUI/XAUI */
	if (xi.interface < 6) {
		int qlm = cvmx_qlm_lmac(xiface, 0);
		enum cvmx_qlm_mode qlm_mode;

		if (qlm == -1) {
			iface_node_ops[xi.node][xi.interface] = &iface_ops_dis;
			return iface_node_ops[xi.node][xi.interface]->mode;
		}
		qlm_mode = cvmx_qlm_get_mode_cn78xx(xi.node, qlm);

		if (qlm_mode == CVMX_QLM_MODE_SGMII)
			iface_node_ops[xi.node][xi.interface] = &iface_ops_bgx_sgmii;
		else if (qlm_mode == CVMX_QLM_MODE_XAUI)
			iface_node_ops[xi.node][xi.interface] = &iface_ops_bgx_xaui;
		else if (qlm_mode == CVMX_QLM_MODE_XLAUI)
			iface_node_ops[xi.node][xi.interface] = &iface_ops_bgx_xlaui;
		else if (qlm_mode == CVMX_QLM_MODE_XFI)
			iface_node_ops[xi.node][xi.interface] = &iface_ops_bgx_xfi;
#ifndef CVMX_BUILD_FOR_UBOOT
		/* Disable 10G_KR and 40G_KR4 in u-boot, 78xx pass1.x has
		   errata related to training */
		else if (qlm_mode == CVMX_QLM_MODE_10G_KR)
			iface_node_ops[xi.node][xi.interface] = &iface_ops_bgx_10G_KR;
		else if (qlm_mode == CVMX_QLM_MODE_40G_KR4)
			iface_node_ops[xi.node][xi.interface] = &iface_ops_bgx_40G_KR4;
#endif
		else if (qlm_mode == CVMX_QLM_MODE_RXAUI)
			iface_node_ops[xi.node][xi.interface] = &iface_ops_bgx_rxaui;
		else
			iface_node_ops[xi.node][xi.interface] = &iface_ops_dis;
	} else if (xi.interface < 8) {
		enum cvmx_qlm_mode qlm_mode;
		int found = 0;
		int i;
		int intf, lane_mask;

		if (xi.interface == 6) {
			intf = 6;
			lane_mask = cvmx_ilk_lane_mask[xi.node][0];
		} else {
			intf = 7;
			lane_mask = cvmx_ilk_lane_mask[xi.node][1];
		}
		switch (lane_mask) {
		default:
		case 0x0:
			iface_node_ops[xi.node][intf] = &iface_ops_dis;
			break;
		case 0xf:
			qlm_mode = cvmx_qlm_get_mode_cn78xx(xi.node, 4);
			if (qlm_mode == CVMX_QLM_MODE_ILK)
				iface_node_ops[xi.node][intf] = &iface_ops_ilk;
			else
				iface_node_ops[xi.node][intf] = &iface_ops_dis;
			break;
		case 0xff:
			found = 0;
			for (i = 4; i < 6; i++) {
				qlm_mode = cvmx_qlm_get_mode_cn78xx(xi.node, i);
				if (qlm_mode == CVMX_QLM_MODE_ILK)
					found++;
			}
			if (found == 2)
				iface_node_ops[xi.node][intf] = &iface_ops_ilk;
			else
				iface_node_ops[xi.node][intf] = &iface_ops_dis;
			break;
		case 0xfff:
			found = 0;
			for (i = 4; i < 7; i++) {
				qlm_mode = cvmx_qlm_get_mode_cn78xx(xi.node, i);
				if (qlm_mode == CVMX_QLM_MODE_ILK)
					found++;
			}
			if (found == 3)
				iface_node_ops[xi.node][intf] = &iface_ops_ilk;
			else
				iface_node_ops[xi.node][intf] = &iface_ops_dis;
			break;
		case 0xff00:
			found = 0;
			for (i = 6; i < 8; i++) {
				qlm_mode = cvmx_qlm_get_mode_cn78xx(xi.node, i);
				if (qlm_mode == CVMX_QLM_MODE_ILK)
					found++;
			}
			if (found == 2)
				iface_node_ops[xi.node][intf] = &iface_ops_ilk;
			else
				iface_node_ops[xi.node][intf] = &iface_ops_dis;
			break;
		case 0xf0:
			qlm_mode = cvmx_qlm_get_mode_cn78xx(xi.node, 5);
			if (qlm_mode == CVMX_QLM_MODE_ILK)
				iface_node_ops[xi.node][intf] = &iface_ops_ilk;
			else
				iface_node_ops[xi.node][intf] = &iface_ops_dis;
			break;
		case 0xf00:
			qlm_mode = cvmx_qlm_get_mode_cn78xx(xi.node, 6);
			if (qlm_mode == CVMX_QLM_MODE_ILK)
				iface_node_ops[xi.node][intf] = &iface_ops_ilk;
			else
				iface_node_ops[xi.node][intf] = &iface_ops_dis;
			break;
		case 0xf000:
			qlm_mode = cvmx_qlm_get_mode_cn78xx(xi.node, 7);
			if (qlm_mode == CVMX_QLM_MODE_ILK)
				iface_node_ops[xi.node][intf] = &iface_ops_ilk;
			else
				iface_node_ops[xi.node][intf] = &iface_ops_dis;
			break;
		case 0xfff0:
			found = 0;
			for (i = 5; i < 8; i++) {
				qlm_mode = cvmx_qlm_get_mode_cn78xx(xi.node, i);
				if (qlm_mode == CVMX_QLM_MODE_ILK)
					found++;
			}
			if (found == 3)
				iface_node_ops[xi.node][intf] = &iface_ops_ilk;
			else
				iface_node_ops[xi.node][intf] = &iface_ops_dis;
			break;
		}
	} else if (xi.interface == 8) { /* DPI */
		int qlm = 0;
		for (qlm = 0; qlm < 5; qlm++) {
			/* if GSERX_CFG[pcie] == 1, then enable npi */
			if (cvmx_read_csr_node(xi.node, CVMX_GSERX_CFG(qlm)) & 0x1) {
				iface_node_ops[xi.node][xi.interface] = &iface_ops_npi;
				return iface_node_ops[xi.node][xi.interface]->mode;
			}
		}
		iface_node_ops[xi.node][xi.interface] = &iface_ops_dis;
	} else if (xi.interface == 9) { /* LOOP */
		iface_node_ops[xi.node][xi.interface] = &iface_ops_loop;
	} else
		iface_node_ops[xi.node][xi.interface] = &iface_ops_dis;

	return iface_node_ops[xi.node][xi.interface]->mode;
}

/**
 * @INTERNAL
 * Return interface mode for CN73XX.
 */
static cvmx_helper_interface_mode_t __cvmx_get_mode_cn73xx(int xiface)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int interface = xi.interface;

	/* SGMII/XAUI/XLAUI/XFI */
	if (interface < 3) {
		int qlm = cvmx_qlm_lmac(xiface, 0);
		enum cvmx_qlm_mode qlm_mode;

		if (qlm == -1) {
			iface_ops[interface] = &iface_ops_dis;
			return iface_ops[interface]->mode;
		}
		qlm_mode = cvmx_qlm_get_mode(qlm);

		switch (qlm_mode) {
		case CVMX_QLM_MODE_SGMII:
		case CVMX_QLM_MODE_SGMII_2X1:
		case CVMX_QLM_MODE_RGMII_SGMII:
		case CVMX_QLM_MODE_RGMII_SGMII_1X1:
			iface_ops[interface] = &iface_ops_bgx_sgmii;
			break;
		case CVMX_QLM_MODE_XAUI:
		case CVMX_QLM_MODE_RGMII_XAUI:
			iface_ops[interface] = &iface_ops_bgx_xaui;
			break;
		case CVMX_QLM_MODE_RXAUI:
		case CVMX_QLM_MODE_RXAUI_1X2:
		case CVMX_QLM_MODE_RGMII_RXAUI:
			iface_ops[interface] = &iface_ops_bgx_rxaui;
			break;
		case CVMX_QLM_MODE_XLAUI:
		case CVMX_QLM_MODE_RGMII_XLAUI:
			iface_ops[interface] = &iface_ops_bgx_xlaui;
			break;
		case CVMX_QLM_MODE_XFI:
		case CVMX_QLM_MODE_XFI_1X2:
		case CVMX_QLM_MODE_RGMII_XFI:
			iface_ops[interface] = &iface_ops_bgx_xfi;
			break;
		case CVMX_QLM_MODE_10G_KR:
		case CVMX_QLM_MODE_10G_KR_1X2:
		case CVMX_QLM_MODE_RGMII_10G_KR:
			iface_ops[interface] = &iface_ops_bgx_10G_KR;
			break;
		case CVMX_QLM_MODE_40G_KR4:
		case CVMX_QLM_MODE_RGMII_40G_KR4:
			iface_ops[interface] = &iface_ops_bgx_40G_KR4;
			break;
		case CVMX_QLM_MODE_MIXED:
			iface_ops[interface] = &iface_ops_bgx_mixed;
			break;
		default:
			iface_ops[interface] = &iface_ops_dis;
			break;
		}
	} else if (interface == 3) /* DPI */
		iface_ops[interface] = &iface_ops_npi;
	else if (interface == 4) /* LOOP */
		iface_ops[interface] = &iface_ops_loop;
	else
		iface_ops[interface] = &iface_ops_dis;

	return iface_ops[interface]->mode;
}


/**
 * @INTERNAL
 * Return interface mode for CNF75XX.
 *
 * CNF75XX has a single BGX block, which is attached to two DLMs,
 * the first, GSER4 only supports SGMII mode, while the second,
 * GSER5 supports 1G/10G single late modes, i.e. SGMII, XFI, 10G-KR.
 * Each half-BGX is thus designated as a separate interface with two ports each.
 */
static cvmx_helper_interface_mode_t __cvmx_get_mode_cnf75xx(int xiface)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int interface = xi.interface;

	/* BGX0: SGMII (DLM4/DLM5)/XFI(DLM5)  */
	if (interface < 1) {
		enum cvmx_qlm_mode qlm_mode;
		int qlm = cvmx_qlm_lmac(xiface, 0);

		if (qlm == -1) {
			iface_ops[interface] = &iface_ops_dis;
			return iface_ops[interface]->mode;
		}
		qlm_mode = cvmx_qlm_get_mode(qlm);

		switch (qlm_mode) {
		case CVMX_QLM_MODE_SGMII:
		case CVMX_QLM_MODE_SGMII_2X1:
			iface_ops[interface] = &iface_ops_bgx_sgmii;
			break;
		case CVMX_QLM_MODE_XFI_1X2:
			iface_ops[interface] = &iface_ops_bgx_xfi;
			break;
		case CVMX_QLM_MODE_10G_KR_1X2:
			iface_ops[interface] = &iface_ops_bgx_10G_KR;
			break;
		case CVMX_QLM_MODE_MIXED:
			iface_ops[interface] = &iface_ops_bgx_mixed;
			break;
		default:
			iface_ops[interface] = &iface_ops_dis;
			break;
		}
	} else if ((interface < 3) && OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
		cvmx_sriox_status_reg_t sriox_status_reg;
		int srio_port = interface - 1;
		sriox_status_reg.u64 =
			cvmx_read_csr(CVMX_SRIOX_STATUS_REG(srio_port));

		if (sriox_status_reg.s.srio)
			iface_ops[interface] = &iface_ops_srio;
		else
			iface_ops[interface] = &iface_ops_dis;
	} else if (interface == 3) /* DPI */
		iface_ops[interface] = &iface_ops_npi;
	else if (interface == 4) /* LOOP */
		iface_ops[interface] = &iface_ops_loop;
	else
		iface_ops[interface] = &iface_ops_dis;

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
	{
		union cvmx_mio_qlmx_cfg qlm_cfg1;
		/* Check if PCIe0/PCIe1 is configured for PCIe */
		qlm_cfg.u64 = cvmx_read_csr(CVMX_MIO_QLMX_CFG(3));
		qlm_cfg1.u64 = cvmx_read_csr(CVMX_MIO_QLMX_CFG(1));
		/* QLM is disabled when QLM SPD is 15. */
		if ((qlm_cfg.s.qlm_spd != 15 && qlm_cfg.s.qlm_cfg == 0)
                    || (qlm_cfg1.s.qlm_spd != 15 && qlm_cfg1.s.qlm_cfg == 0))
			iface_ops[interface] = &iface_ops_npi;
		else
			iface_ops[interface] = &iface_ops_dis;
	}
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
 * @param xiface Interface to probe
 *
 * @return Mode of the interface. Unknown or unsupported interfaces return
 *         DISABLED.
 */
cvmx_helper_interface_mode_t cvmx_helper_interface_get_mode(int xiface)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (xi.interface < 0 ||
	    xi.interface >= cvmx_helper_get_number_of_interfaces())
		return CVMX_HELPER_INTERFACE_MODE_DISABLED;

	/*
	 * Check if the interface mode has been already cached. If it has,
	 * simply return it. Otherwise, fall through the rest of the code to
	 * determine the interface mode and cache it in iface_ops.
	 */
	if (iface_node_ops[xi.node][xi.interface] != NULL) {
		cvmx_helper_interface_mode_t mode;
		mode = iface_node_ops[xi.node][xi.interface]->mode;
		return mode;
	}

	/*
	 * OCTEON III models
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN70XX))
		return __cvmx_get_mode_cn70xx(xi.interface);

	if (OCTEON_IS_MODEL(OCTEON_CN78XX))
		return __cvmx_get_mode_cn78xx(xiface);

	if (OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
		cvmx_helper_interface_mode_t mode;
		mode = __cvmx_get_mode_cnf75xx(xiface);
		return mode;
	}

	if (OCTEON_IS_MODEL(OCTEON_CN73XX)) {
		cvmx_helper_interface_mode_t mode;
		mode = __cvmx_get_mode_cn73xx(xiface);
		return mode;
	}

	/*
	 * Octeon II models
	 */
	if (OCTEON_IS_OCTEON2())
		return __cvmx_get_mode_octeon2(xi.interface);

	/*
	 * Octeon and Octeon Plus models
	 */
	if (xi.interface == 2)
		iface_ops[xi.interface] = &iface_ops_npi;
	else if (xi.interface == 3) {
		if (OCTEON_IS_MODEL(OCTEON_CN56XX)
		    || OCTEON_IS_MODEL(OCTEON_CN52XX))
			iface_ops[xi.interface] = &iface_ops_loop;
		else
			iface_ops[xi.interface] = &iface_ops_dis;
	}
	else if (xi.interface == 0 &&
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
		iface_ops[xi.interface] = &iface_ops_gmii;
	}
	else if ((xi.interface == 1)
	    && (OCTEON_IS_MODEL(OCTEON_CN31XX)
		|| OCTEON_IS_MODEL(OCTEON_CN30XX)
		|| OCTEON_IS_MODEL(OCTEON_CN50XX)
		|| OCTEON_IS_MODEL(OCTEON_CN52XX)))
		/* Interface 1 is always disabled on CN31XX and CN30XX */
		iface_ops[xi.interface] = &iface_ops_dis;
	else {
		union cvmx_gmxx_inf_mode mode;
		mode.u64 = cvmx_read_csr(CVMX_GMXX_INF_MODE(xi.interface));

		if (OCTEON_IS_MODEL(OCTEON_CN56XX) ||
		    OCTEON_IS_MODEL(OCTEON_CN52XX)) {
			switch (mode.cn56xx.mode) {
			case 0:
				iface_ops[xi.interface] = &iface_ops_dis;
				break;

			case 1:
				iface_ops[xi.interface] = &iface_ops_xaui;
				break;

			case 2:
				iface_ops[xi.interface] = &iface_ops_sgmii;
				break;

			case 3:
				iface_ops[xi.interface] = &iface_ops_picmg;
				break;

			default:
				iface_ops[xi.interface] = &iface_ops_dis;
				break;
			}
		} else {
			if (!mode.s.en)
				iface_ops[xi.interface] = &iface_ops_dis;
			else if (mode.s.type) {
				if (OCTEON_IS_MODEL(OCTEON_CN38XX) ||
				    OCTEON_IS_MODEL(OCTEON_CN58XX))
					iface_ops[xi.interface] = &iface_ops_spi;
				else
					iface_ops[xi.interface] = &iface_ops_gmii;
			} else
				iface_ops[xi.interface] = &iface_ops_rgmii;
		}
	}

	return iface_ops[xi.interface]->mode;
}
EXPORT_SYMBOL(cvmx_helper_interface_get_mode);

/**
 * Determine the actual number of hardware ports connected to an
 * interface. It doesn't setup the ports or enable them.
 *
 * @param xiface Interface to enumerate
 *
 * @return The number of ports on the interface, negative on failure
 */
int cvmx_helper_interface_enumerate(int xiface)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int	result = 0;

	cvmx_helper_interface_get_mode(xiface);
	if (iface_node_ops[xi.node][xi.interface]->enumerate)
		result = iface_node_ops[xi.node][xi.interface]->enumerate(xiface);

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
 * @param xiface Interface to probe
 *
 * @return Zero on success, negative on failure
 */
int cvmx_helper_interface_probe(int xiface)
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
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	nports = -1;
	has_fcs = 0;

	cvmx_helper_interface_get_mode(xiface);
	if (iface_node_ops[xi.node][xi.interface]->probe)
		nports = iface_node_ops[xi.node][xi.interface]->probe(xiface);

	switch (iface_node_ops[xi.node][xi.interface]->mode) {
		/* These types don't support ports to IPD/PKO */
	case CVMX_HELPER_INTERFACE_MODE_DISABLED:
	case CVMX_HELPER_INTERFACE_MODE_PCIE:
		nports = 0;
		break;
		/* XAUI is a single high speed port */
	case CVMX_HELPER_INTERFACE_MODE_XAUI:
	case CVMX_HELPER_INTERFACE_MODE_RXAUI:
	case CVMX_HELPER_INTERFACE_MODE_XLAUI:
	case CVMX_HELPER_INTERFACE_MODE_XFI:
	case CVMX_HELPER_INTERFACE_MODE_10G_KR:
	case CVMX_HELPER_INTERFACE_MODE_40G_KR4:
	case CVMX_HELPER_INTERFACE_MODE_MIXED:
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

	nports = __cvmx_helper_board_interface_probe(xiface, nports);
	__cvmx_helper_init_interface(xiface, nports, has_fcs, padding);
	/* Make sure all global variables propagate to other cores */
	CVMX_SYNCWS;

	return 0;
}
EXPORT_SYMBOL(cvmx_helper_interface_probe);

/**
 * @INTERNAL
 * Setup backpressure.
 *
 * @return Zero on success, negative on failure
 */
static int __cvmx_helper_global_setup_backpressure(int node)
{
	unsigned int bpmask;
	int interface;
	int num_interfaces = cvmx_helper_get_number_of_interfaces();

	for (interface = 0; interface < num_interfaces; interface++) {
		int xiface = cvmx_helper_node_interface_to_xiface(node, interface);
		switch (cvmx_helper_interface_get_mode(xiface)) {
		case CVMX_HELPER_INTERFACE_MODE_DISABLED:
		case CVMX_HELPER_INTERFACE_MODE_PCIE:
		case CVMX_HELPER_INTERFACE_MODE_SRIO:
		case CVMX_HELPER_INTERFACE_MODE_ILK:
		case CVMX_HELPER_INTERFACE_MODE_NPI:
		case CVMX_HELPER_INTERFACE_MODE_PICMG:
			break;
		case CVMX_HELPER_INTERFACE_MODE_LOOP:
		case CVMX_HELPER_INTERFACE_MODE_XAUI:
		case CVMX_HELPER_INTERFACE_MODE_RXAUI:
		case CVMX_HELPER_INTERFACE_MODE_XLAUI:
		case CVMX_HELPER_INTERFACE_MODE_XFI:
		case CVMX_HELPER_INTERFACE_MODE_10G_KR:
		case CVMX_HELPER_INTERFACE_MODE_40G_KR4:
			bpmask = (cvmx_rgmii_backpressure_dis) ? 0xF : 0;
			if (octeon_has_feature(OCTEON_FEATURE_BGX))
				cvmx_bgx_set_backpressure_override(xiface, bpmask);
			break;
		case CVMX_HELPER_INTERFACE_MODE_RGMII:
		case CVMX_HELPER_INTERFACE_MODE_GMII:
		case CVMX_HELPER_INTERFACE_MODE_SPI:
		case CVMX_HELPER_INTERFACE_MODE_SGMII:
		case CVMX_HELPER_INTERFACE_MODE_QSGMII:
		case CVMX_HELPER_INTERFACE_MODE_MIXED:
			bpmask = (cvmx_rgmii_backpressure_dis) ? 0xF : 0;
			if (octeon_has_feature(OCTEON_FEATURE_BGX))
				cvmx_bgx_set_backpressure_override(xiface, bpmask);
			else
				cvmx_gmx_set_backpressure_override(interface, bpmask);
			break;
		case CVMX_HELPER_INTERFACE_MODE_AGL:
			bpmask = (cvmx_rgmii_backpressure_dis) ? 0x1 : 0;
			cvmx_agl_set_backpressure_override(interface, bpmask);
			break;
		}
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
	cvmx_helper_interface_mode_t mode0, mode1;

	mode0 = cvmx_helper_interface_get_mode(0);
	mode1 = cvmx_helper_interface_get_mode(1);

	if (!((cvmx_sysinfo_get()->board_type != CVMX_BOARD_TYPE_SIM) &&
	     (OCTEON_IS_MODEL(OCTEON_CN3XXX) ||
	      OCTEON_IS_MODEL(OCTEON_CN5XXX))))
		return 0;

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
 * @param xiface Interface to enable
 *
 * @return Zero on success, negative on failure
 */
int __cvmx_helper_packet_hardware_enable(int xiface)
{
	int result = 0;
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);

	if (iface_node_ops[xi.node][xi.interface]->enable)
		result = iface_node_ops[xi.node][xi.interface]->enable(xiface);
	result |= __cvmx_helper_board_hardware_enable(xiface);
	return result;
}
EXPORT_SYMBOL(__cvmx_helper_packet_hardware_enable);

int cvmx_helper_ipd_and_packet_input_enable(void)
{
	return cvmx_helper_ipd_and_packet_input_enable_node(cvmx_get_node_num());
}

/**
 * Called after all internal packet IO paths are setup. This
 * function enables IPD/PIP and begins packet input and output.
 *
 * @return Zero on success, negative on failure
 */
int cvmx_helper_ipd_and_packet_input_enable_node(int node)
{
	int num_interfaces;
	int interface;
	int num_ports;

	if (octeon_has_feature(OCTEON_FEATURE_PKI)) {
		cvmx_helper_pki_enable(node);
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
		int xiface = cvmx_helper_node_interface_to_xiface(node, interface);
		num_ports = cvmx_helper_ports_on_interface(xiface);
		if (num_ports > 0)
			__cvmx_helper_packet_hardware_enable(xiface);
	}

	/* Finally enable PKO now that the entire path is up and running */
	/* enable pko */
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		; // cvmx_pko_enable_78xx(0); already enabled
	} else {
#ifdef CVMX_BUILD_FOR_STANDALONE
		__cvmx_install_gmx_error_handler_for_xaui();
#endif
		cvmx_pko_enable();
	}

	if ((OCTEON_IS_MODEL(OCTEON_CN31XX_PASS1) ||
	     OCTEON_IS_MODEL(OCTEON_CN30XX_PASS1)) &&
	    (cvmx_sysinfo_get()->board_type != CVMX_BOARD_TYPE_SIM))
		__cvmx_helper_errata_fix_ipd_ptr_alignment();
	return 0;
}
EXPORT_SYMBOL(cvmx_helper_ipd_and_packet_input_enable_node);

/**
 * Initialize the PIP, IPD, and PKO hardware to support
 * simple priority based queues for the ethernet ports. Each
 * port is configured with a number of priority queues based
 * on CVMX_PKO_QUEUES_PER_PORT_* where each queue is lower
 * priority than the previous.
 *
 * @return Zero on success, non-zero on failure
 */
int cvmx_helper_initialize_packet_io_node(unsigned int node)
{
	int result = 0;
	int interface;
	int xiface;
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
		l2c_ctl.u64 = cvmx_read_csr_node(node, CVMX_L2C_CTL);
		l2c_ctl.s.rsp_arb_mode = 1;
		l2c_ctl.s.xmc_arb_mode = 0;
		cvmx_write_csr_node(node, CVMX_L2C_CTL, l2c_ctl.u64);
	} else {
		l2c_cfg.u64 = cvmx_read_csr(CVMX_L2C_CFG);
		l2c_cfg.s.lrf_arb_mode = 0;
		l2c_cfg.s.rfb_arb_mode = 0;
		cvmx_write_csr(CVMX_L2C_CFG, l2c_cfg.u64);
	}

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	if (cvmx_sysinfo_get()->board_type != CVMX_BOARD_TYPE_SIM) {
		int smi_inf;
		int i;

		/* Newer chips have more than one SMI/MDIO interface */
		if (OCTEON_IS_MODEL(OCTEON_CN68XX) ||
		    OCTEON_IS_MODEL(OCTEON_CN78XX))
			smi_inf = 4;
		else if (OCTEON_IS_MODEL(OCTEON_CN73XX) ||
			OCTEON_IS_MODEL(OCTEON_CNF75XX))
			smi_inf = 2;
		else if (OCTEON_IS_MODEL(OCTEON_CN3XXX) ||
			 OCTEON_IS_MODEL(OCTEON_CN58XX) ||
			 OCTEON_IS_MODEL(OCTEON_CN50XX))
			smi_inf = 1;
		else
			smi_inf = 2;

		for (i = 0; i < smi_inf; i++) {
			/* Make sure SMI/MDIO is enabled so we can query PHYs */
			smix_en.u64 = cvmx_read_csr_node(node, CVMX_SMIX_EN(i));
			if (!smix_en.s.en) {
				smix_en.s.en = 1;
				cvmx_write_csr_node(node, CVMX_SMIX_EN(i), smix_en.u64);
			}
		}
	}
#endif

	__cvmx_helper_init_port_valid();//vinita_to_do ask it need to be modify for multinode

	for (interface = 0; interface < num_interfaces; interface++) {
		xiface = cvmx_helper_node_interface_to_xiface(node, interface);
		result |= cvmx_helper_interface_probe(xiface);
	}

	/* PKO3 init precedes that of interfaces */
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		__cvmx_helper_init_port_config_data(node);
		result = cvmx_helper_pko3_init_global(node);
	}
	else {
		result = cvmx_helper_pko_init();
	}

	/* Errata SSO-29000, Disabling power saving SSO conditional clocking */
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		cvmx_sso_ws_cfg_t cfg;
		cfg.u64 = cvmx_read_csr_node(node, CVMX_SSO_WS_CFG);
		cfg.s.sso_cclk_dis = 1;
		cvmx_write_csr_node(node, CVMX_SSO_WS_CFG, cfg.u64);
	}

	if (result < 0)
		return result;

	for (interface = 0; interface < num_interfaces; interface++) {
		xiface = cvmx_helper_node_interface_to_xiface(node, interface);
		/* Skip invalid/disabled interfaces */
		if (cvmx_helper_ports_on_interface(xiface) <= 0)
			continue;
		cvmx_printf("Node %d Interface %d has %d ports (%s)\n", node,
			    interface,
			    cvmx_helper_ports_on_interface(xiface),
			    cvmx_helper_interface_mode_to_string(cvmx_helper_interface_get_mode(xiface)));

		result |= __cvmx_helper_ipd_setup_interface(xiface);
		if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE))
			result |= cvmx_helper_pko3_init_interface(xiface);
		else
			result |= __cvmx_helper_interface_setup_pko(interface);
	}

	if (octeon_has_feature(OCTEON_FEATURE_PKI))
		result |= __cvmx_helper_pki_global_setup(node);
	else
		result |= __cvmx_helper_ipd_global_setup();

	/* Enable any flow control and backpressure */
	result |= __cvmx_helper_global_setup_backpressure(node);

	/* export app config if set */
	if (cvmx_export_app_config) {
		result |= (*cvmx_export_app_config)();
	}

	if (cvmx_ipd_cfg.ipd_enable && cvmx_pki_dflt_init[node])
		result |= cvmx_helper_ipd_and_packet_input_enable_node(node);
	return result;
}
EXPORT_SYMBOL(cvmx_helper_initialize_packet_io_node);

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
	unsigned int node = cvmx_get_node_num();
	return cvmx_helper_initialize_packet_io_node(node);
}
EXPORT_SYMBOL(cvmx_helper_initialize_packet_io_global);

/**
 * Does core local initialization for packet io
 *
 * @return Zero on success, non-zero on failure
 */
int cvmx_helper_initialize_packet_io_local(void)
{
	if (octeon_has_feature(OCTEON_FEATURE_CN78XX_WQE)) {
		__cvmx_pko3_dq_table_setup();
	}
	return 0;
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
 * Helper function for global packet IO shutdown
 */
int cvmx_helper_shutdown_packet_io_global_cn78xx(int node)
{
	int num_interfaces = cvmx_helper_get_number_of_interfaces();
	cvmx_wqe_t *work;
	int interface;
	int result = 0;

	/* Shut down all interfaces and disable TX and RX on all ports */
	for (interface = 0; interface < num_interfaces; interface++) {
		int xiface = cvmx_helper_node_interface_to_xiface(node, interface);
		int index;
		int num_ports = cvmx_helper_ports_on_interface(xiface);

		if (num_ports > 4)
			num_ports = 4;

		cvmx_bgx_set_backpressure_override(xiface, 0);
		for (index = 0; index < num_ports; index++) {
#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
			cvmx_helper_link_info_t link_info;
#endif
			if (!cvmx_helper_is_port_valid(xiface, index))
				continue;

			cvmx_helper_bgx_shutdown_port(xiface, index);
#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
			/* Turn off link LEDs */
			link_info.u64 = 0;
			__cvmx_update_link_led(xiface, index, link_info);
#endif
		}
	}

	/* Stop input first */
	cvmx_helper_pki_shutdown(node);

	/* Retrieve all packets from the SSO and free them */
	result = 0;
	while ((work = cvmx_pow_work_request_sync(CVMX_POW_WAIT))) {
		cvmx_helper_free_pki_pkt_data(work);
		cvmx_wqe_pki_free(work);
		result++;
	}

	if (result > 0)
		cvmx_dprintf("%s: Purged %d packets from SSO\n",
			__func__, result);

	/*
	 * No need to wait for PKO queues to drain,
	 * dq_close() drains the queues to NULL.
	 */

	/* Shutdown PKO interfaces */
	for (interface = 0; interface < num_interfaces; interface++) {
		int xiface = cvmx_helper_node_interface_to_xiface(node, interface);
		cvmx_helper_pko3_shut_interface(xiface);
	}

	/* Disable MAC address filtering */
	for (interface = 0; interface < num_interfaces; interface++) {
		int xiface = cvmx_helper_node_interface_to_xiface(node, interface);
		switch (cvmx_helper_interface_get_mode(xiface)) {
		case CVMX_HELPER_INTERFACE_MODE_XAUI:
		case CVMX_HELPER_INTERFACE_MODE_RXAUI:
		case CVMX_HELPER_INTERFACE_MODE_XLAUI:
		case CVMX_HELPER_INTERFACE_MODE_XFI:
		case CVMX_HELPER_INTERFACE_MODE_10G_KR:
		case CVMX_HELPER_INTERFACE_MODE_40G_KR4:
		case CVMX_HELPER_INTERFACE_MODE_SGMII:
		case CVMX_HELPER_INTERFACE_MODE_MIXED:
		{
			int index;
			int num_ports = cvmx_helper_ports_on_interface(xiface);

			for (index = 0; index < num_ports; index++) {
				if (!cvmx_helper_is_port_valid(xiface, index))
					continue;

				/* Reset MAC filtering */
				cvmx_helper_bgx_rx_adr_ctl(node, interface, index, 0, 0, 0);
			}
			break;
		}
		default:
			break;
		}
	}

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	for (interface = 0; interface < num_interfaces; interface++) {
		int index;
		int xiface = cvmx_helper_node_interface_to_xiface(node, interface);
		int num_ports = cvmx_helper_ports_on_interface(xiface);

		for (index = 0; index < num_ports; index++) {
			/* Doing this twice should clear it since no packets
			 * can be received.
			 */
			cvmx_update_rx_activity_led(xiface, index, false);
			cvmx_update_rx_activity_led(xiface, index, false);
		}
	}
#endif

	/* Shutdown the PKO unit */
	result = cvmx_helper_pko3_shutdown(node);

	/* Release interface structures */
	__cvmx_helper_shutdown_interfaces();

	return result;
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
	int node = cvmx_get_node_num();
	cvmx_pcsx_mrx_control_reg_t control_reg;

	if (octeon_has_feature(OCTEON_FEATURE_BGX)) {
		return cvmx_helper_shutdown_packet_io_global_cn78xx(node);
	}

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
	result = __cvmx_helper_pko_drain();
	if (result < 0)
		cvmx_dprintf("WARNING: %s: "
			"Failed to drain some PKO queues\n",
			__func__);

	/* Step 3: Disable TX and RX on all ports */
	for (interface = 0; interface < num_interfaces; interface++) {
		int xiface = cvmx_helper_node_interface_to_xiface(node, interface);
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
			num_ports = cvmx_helper_ports_on_interface(xiface);
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
			num_ports = cvmx_helper_ports_on_interface(xiface);
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
				/* For SGMII some PHYs require that the PCS
				 * interface be powered down and reset (i.e.
				 * Atheros/Qualcomm PHYs).
				 */
				if (cvmx_helper_interface_get_mode(interface) ==
				    CVMX_HELPER_INTERFACE_MODE_SGMII) {
					uint64_t reg;

					reg = CVMX_PCSX_MRX_CONTROL_REG(index,
									interface);
					/* Power down the interface */
					control_reg.u64 = cvmx_read_csr(reg);
					control_reg.s.pwr_dn = 1;
					cvmx_write_csr(reg, control_reg.u64);
					cvmx_read_csr(reg);
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
		default:
			break;
		}
	}

	/* Step 4: Retrieve all packets from the POW and free them */
	while ((work = cvmx_pow_work_request_sync(CVMX_POW_WAIT))) {
		cvmx_helper_free_packet_data(work);
		cvmx_fpa1_free(work, wqe_pool, 0);
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
				void *buffer = cvmx_fpa1_alloc(packet_pool);
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
						if (cvmx_hwpko_send_packet_finish(port, queue, pko_command, packet, CVMX_PKO_LOCK_CMD_QUEUE)) {
							cvmx_dprintf("ERROR: Unable to align IPD counters (PKO failed)\n");
							break;
						}
					}
					cvmx_fpa1_free(buffer, packet_pool, 0);

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
		int xiface = cvmx_helper_node_interface_to_xiface(node, interface);
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
			num_ports = cvmx_helper_ports_on_interface(xiface);
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
		default:
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
		struct cvmx_buffer_list *buffer = cvmx_fpa1_alloc(0);
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
		cvmx_fpa1_free(pool0_buffers, 0, 0);
		pool0_buffers = n;
	}

	/* Step 12: Release interface structures */
	__cvmx_helper_shutdown_interfaces();

	return result;
}

EXPORT_SYMBOL(cvmx_helper_shutdown_packet_io_global);

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
 * @param xipd_port IPD/PKO port to auto configure
 *
 * @return Link state after configure
 */
cvmx_helper_link_info_t cvmx_helper_link_autoconf(int xipd_port)
{
	cvmx_helper_link_info_t link_info;
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	int index = cvmx_helper_get_interface_index_num(xipd_port);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int interface = xi.interface;
	if (interface == -1 || index == -1 ||
	    index >= cvmx_helper_ports_on_interface(xiface)) {
		link_info.u64 = 0;
		return link_info;
	}


	link_info = cvmx_helper_link_get(xipd_port);
	if (link_info.u64 == (__cvmx_helper_get_link_info(xiface, index)).u64)
		return link_info;

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	if (!link_info.s.link_up)
		cvmx_error_disable_group(CVMX_ERROR_GROUP_ETHERNET, xipd_port);
#endif

	/* If we fail to set the link speed, port_link_info will not change */
	cvmx_helper_link_set(xipd_port, link_info);

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	if (link_info.s.link_up)
		cvmx_error_enable_group(CVMX_ERROR_GROUP_ETHERNET, xipd_port);
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
 * @param xipd_port IPD/PKO port to query
 *
 * @return Link state
 */
cvmx_helper_link_info_t cvmx_helper_link_get(int xipd_port)
{
	cvmx_helper_link_info_t result;
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	int index = cvmx_helper_get_interface_index_num(xipd_port);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	struct cvmx_fdt_sfp_info *sfp_info;
#endif

	/*
	 * The default result will be a down link unless the code
	 * below changes it.
	 */
	result.u64 = 0;

	if (__cvmx_helper_xiface_is_null(xiface) || index == -1 ||
	    index >= cvmx_helper_ports_on_interface(xiface)) {
		return result;
	}

	if (iface_node_ops[xi.node][xi.interface]->link_get)
		result = iface_node_ops[xi.node][xi.interface]->link_get(xipd_port);

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	if (xipd_port >= 0) {
		__cvmx_update_link_led(xiface, index, result);

		sfp_info = cvmx_helper_cfg_get_sfp_info(xiface, index);

		if (sfp_info &&
		    (!result.s.link_up ||
		     (result.s.link_up && sfp_info->last_mod_abs)))
			cvmx_sfp_check_mod_abs(sfp_info,
					       sfp_info->mod_abs_data);
	}
#endif
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
 * @param xipd_port  IPD/PKO port to configure
 * @param link_info The new link state
 *
 * @return Zero on success, negative on failure
 */
int cvmx_helper_link_set(int xipd_port, cvmx_helper_link_info_t link_info)
{
	int result = -1;
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int index = cvmx_helper_get_interface_index_num(xipd_port);

	if (__cvmx_helper_xiface_is_null(xiface) || index == -1 ||
	    index >= cvmx_helper_ports_on_interface(xiface))
		return -1;

	if (iface_node_ops[xi.node][xi.interface]->link_set)
		result = iface_node_ops[xi.node][xi.interface]->link_set(xipd_port, link_info);

	/*
	 * Set the port_link_info here so that the link status is
	 * updated no matter how cvmx_helper_link_set is called. We
	 * don't change the value if link_set failed.
	 */
	if (result == 0)
		__cvmx_helper_set_link_info(xiface, index, link_info);
	return result;
}
EXPORT_SYMBOL(cvmx_helper_link_set);

/**
 * Configure a port for internal and/or external loopback. Internal loopback
 * causes packets sent by the port to be received by Octeon. External loopback
 * causes packets received from the wire to sent out again.
 *
 * @param xipd_port IPD/PKO port to loopback.
 * @param enable_internal
 *                 Non zero if you want internal loopback
 * @param enable_external
 *                 Non zero if you want external loopback
 *
 * @return Zero on success, negative on failure.
 */
int cvmx_helper_configure_loopback(int xipd_port, int enable_internal,
				   int enable_external)
{
	int result = -1;
	int xiface = cvmx_helper_get_interface_num(xipd_port);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	int index = cvmx_helper_get_interface_index_num(xipd_port);

	if (index >= cvmx_helper_ports_on_interface(xiface))
		return -1;

	cvmx_helper_interface_get_mode(xiface);
	if (iface_node_ops[xi.node][xi.interface]->loopback)
		result = iface_node_ops[xi.node][xi.interface]->loopback(xipd_port,
									 enable_internal,
									 enable_external);

	return result;
}

void cvmx_helper_setup_simulator_io_buffer_counts(int node, int num_packet_buffers,
					    int pko_buffers)
{
	if (octeon_has_feature(OCTEON_FEATURE_PKI)) {
		cvmx_helper_pki_set_dflt_pool_buffer(node, num_packet_buffers);
		cvmx_helper_pki_set_dflt_aura_buffer(node, num_packet_buffers);

	} else {
		cvmx_ipd_set_packet_pool_buffer_count(num_packet_buffers);
		cvmx_ipd_set_wqe_pool_buffer_count(num_packet_buffers);
		cvmx_pko_set_cmd_queue_pool_buffer_count(pko_buffers);
	}
}

void *cvmx_helper_mem_alloc(int node, uint64_t alloc_size, uint64_t align)
{
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
	return kmalloc(alloc_size, GFP_NOIO | GFP_DMA);
#else
	int64_t paddr;

	paddr = cvmx_bootmem_phy_alloc_range(alloc_size, align,
				     cvmx_addr_on_node(node, 0ull),
				     cvmx_addr_on_node(node, 0xffffffffff));
	if (paddr <= 0ll) {
		cvmx_printf("ERROR: %s failed size %u\n",
			__func__, (unsigned) alloc_size);
		return NULL;
	}
	return cvmx_phys_to_ptr(paddr);
#endif
}

void cvmx_helper_mem_free(void *buffer, uint64_t size)
{
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
	kfree(buffer);
#else
	__cvmx_bootmem_phy_free(cvmx_ptr_to_phys(buffer), size, 0);
#endif
}

