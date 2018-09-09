/***********************license start***************
 * Copyright (c) 2003-2013  Cavium Inc. (support@cavium.com). All rights
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
 * Helper functions to abstract board specific data about
 * network ports from the rest of the cvmx-helper files.
 *
 */

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <linux/module.h>
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-app-init.h>
#include <asm/octeon/cvmx-smix-defs.h>
#include <asm/octeon/cvmx-gmxx-defs.h>
#include <asm/octeon/cvmx-asxx-defs.h>
#include <asm/octeon/cvmx-mdio.h>
#include <asm/octeon/cvmx-helper.h>
#include <asm/octeon/cvmx-helper-util.h>
#include <asm/octeon/cvmx-helper-board.h>
#include <asm/octeon/cvmx-helper-cfg.h>
#include <asm/octeon/cvmx-twsi.h>
#else
#include "cvmx.h"
#include "cvmx-app-init.h"
#include "cvmx-sysinfo.h"
#include "cvmx-twsi.h"
#include "cvmx-mdio.h"
#include "cvmx-helper.h"
#include "cvmx-helper-util.h"
#include "cvmx-helper-board.h"
#include "cvmx-helper-cfg.h"
#include "cvmx-gpio.h"
#include "octeon_mem_map.h"
#include "cvmx-bootmem.h"
#ifdef __U_BOOT__
# include <libfdt.h>
#else
# include "libfdt/libfdt.h"
#endif
#endif

/**
 * cvmx_override_board_link_get(int ipd_port) is a function
 * pointer. It is meant to allow customization of the process of
 * talking to a PHY to determine link speed. It is called every
 * time a PHY must be polled for link status. Users should set
 * this pointer to a function before calling any cvmx-helper
 * operations.
 */
CVMX_SHARED cvmx_helper_link_info_t(*cvmx_override_board_link_get) (int ipd_port) = NULL;

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL

static int device_tree_dbg = 0;

static void cvmx_retry_i2c_write(int twsi_id, uint8_t dev_addr,
				 uint16_t internal_addr, int num_bytes,
				 int ia_width_bytes, uint64_t data)
{
	int tries = 3;
	int r;
	do {
		r = cvmx_twsix_write_ia(twsi_id, dev_addr, internal_addr,
					num_bytes, ia_width_bytes, data);
	} while (tries-- > 0 && r < 0);
}

/**
 * Returns the Ethernet node offset in the device tree
 *
 * @param     fdt_addr - pointer to flat device tree in memory
 * @param     alias    - offset of alias in device tree
 * @param     ipd_port - ipd port number to look up
 *
 * @returns   offset of Ethernet node if >= 0, error if -1
 */
int __pip_eth_node(const void *fdt_addr, int aliases, int ipd_port)
{
	char name_buffer[20];
	const char *pip_path;
	int pip, iface, eth;
	int interface_num = cvmx_helper_get_interface_num(ipd_port);
	int interface_index = cvmx_helper_get_interface_index_num(ipd_port);
	cvmx_helper_interface_mode_t interface_mode =
			 cvmx_helper_interface_get_mode(interface_num);
	int dbg = device_tree_dbg;

	/* The following are not found in the device tree */
	switch (interface_mode) {
	case CVMX_HELPER_INTERFACE_MODE_ILK:
	case CVMX_HELPER_INTERFACE_MODE_LOOP:
	case CVMX_HELPER_INTERFACE_MODE_SRIO:
		cvmx_dprintf("ERROR: No node expected for interface: %d, port: %d, mode: %s\n",
			     interface_index,
			     ipd_port,
			     cvmx_helper_interface_mode_to_string(interface_mode));
		return -1;
	default:
		break;
	}
	pip_path = fdt_getprop(fdt_addr, aliases, "pip", NULL);
	if (!pip_path) {
		cvmx_dprintf("ERROR: pip path not found in device tree\n");
		return -1;
	}
	pip = fdt_path_offset(fdt_addr, pip_path);
	if (dbg)
		cvmx_dprintf("ipdd_port=%d pip_path=%s pip=%d ", ipd_port, pip_path, pip);
	if (pip < 0) {
		cvmx_dprintf("ERROR: pip not found in device tree\n");
		if (dbg)
			cvmx_dprintf("\n");
		return -1;
	}
	snprintf(name_buffer, sizeof(name_buffer), "interface@%d", interface_num);
	iface = fdt_subnode_offset(fdt_addr, pip, name_buffer);
	if (dbg)
		cvmx_dprintf("iface=%d ", iface);
	if (iface < 0) {
		if (dbg)
			cvmx_dprintf("ERROR : pip intf %d not found in device tree\n",
			     interface_num);
		return -1;
	}
	snprintf(name_buffer, sizeof(name_buffer), "ethernet@%x", interface_index);
	eth = fdt_subnode_offset(fdt_addr, iface, name_buffer);
	if (dbg)
		cvmx_dprintf("eth=%d \n", eth);
	if (eth < 0) {
		if (dbg)
			cvmx_dprintf("ERROR : pip interface@%d ethernet@%d not "
				     "found in device tree\n",
				     interface_num, interface_index);
		return -1;
	}
	return eth;
}

int __mix_eth_node(const void *fdt_addr, int aliases, int interface_index)
{
	char name_buffer[20];
	const char *mix_path;
	int mix;

	snprintf(name_buffer, sizeof(name_buffer), "mix%d", interface_index);
	mix_path = fdt_getprop(fdt_addr, aliases, name_buffer, NULL);
	if (!mix_path) {
		cvmx_dprintf("ERROR: mix%d path not found in device tree\n", interface_index);
	}
	mix = fdt_path_offset(fdt_addr, mix_path);
	if (mix < 0) {
		cvmx_dprintf("ERROR: %s not found in device tree\n", mix_path);
		return -1;
	}
	return mix;
}

/**
 * @INTERNAL
 * Switch MDIO mux to the specified port.
 */
static int __switch_mdio_mux(int ipd_port, const cvmx_phy_info_t *phy_info);

static int __mdiobus_addr_to_unit(uint32_t addr)
{
	int unit = (addr >> 7) & 3;
	if (!OCTEON_IS_MODEL(OCTEON_CN68XX))
		unit >>= 1;
	return unit;
}

/**
 * Parse the muxed MDIO interface information from the device tree
 *
 * @param phy_info - pointer to phy info data structure to update
 * @param mdio_offset - offset of MDIO bus
 * @param mux_offset - offset of MUX, parent of mdio_offset
 *
 * @return 0 for success or -1
 */
static int __get_muxed_mdio_info_from_dt(cvmx_phy_info_t *phy_info,
					 int mdio_offset, int mux_offset)
{
	static void *fdt_addr = 0;
	uint32_t *psmi_handle;
	int phandle;
	uint32_t *pgpio_handle;
	int smi_offset;
	int gpio_offset;
	uint64_t *smi_addrp;
	uint64_t smi_addr = 0;
	int len;
	int gpio_count = 0;
	uint32_t *prop_val;
	int offset;
	const char *prop_name;

	if (fdt_addr == 0)
		fdt_addr = __cvmx_phys_addr_to_ptr(cvmx_sysinfo_get()->fdt_addr,
						   OCTEON_FDT_MAX_SIZE);

	prop_val = (uint32_t *)fdt_getprop(fdt_addr, mdio_offset, "reg", NULL);
	if (!prop_val) {
		cvmx_dprintf("Could not get register value for muxed MDIO bus from DT\n");
		return -1;
	}
	/* Get register value to put onto the GPIO lines to select */
	phy_info->gpio_value = fdt32_to_cpu(*prop_val);

	psmi_handle = (uint32_t *)fdt_getprop(fdt_addr, mux_offset,
					      "mdio-parent-bus", NULL);
	if (psmi_handle == NULL) {
		cvmx_dprintf("Could not get MDIO parent bus for multiplexed bus from device tree\n");
		return -1;
	}

	phandle = fdt32_to_cpu(*psmi_handle);
	smi_offset = fdt_node_offset_by_phandle(fdt_addr, phandle);
	if (smi_offset < 0) {
		cvmx_dprintf("Invalid SMI offset for muxed MDIO interface in device tree\n");
		return -1;
	}
	smi_addrp = (uint64_t *)fdt_getprop(fdt_addr, smi_offset, "reg", &len);
	if ((len < (int)sizeof(uint64_t)) || smi_addrp == NULL) {
		cvmx_dprintf("Could not get register information for SMI interface from DT\n");
		return -1;
	}
	memcpy(&smi_addr, smi_addrp, sizeof(uint64_t));
	smi_addr = fdt64_to_cpu(smi_addr);

	/* Convert SMI address to a MDIO interface */
	switch (smi_addr) {
	case 0x1180000001800:
	case 0x1180000003800:	/* 68XX address */
		phy_info->mdio_unit = 0;
		break;
	case 0x1180000001900:
	case 0x1180000003880:
		phy_info->mdio_unit = 1;
		break;
	case 0x1180000003900:
		phy_info->mdio_unit = 2;
		break;
	case 0x1180000003980:
		phy_info->mdio_unit = 3;
		break;
	default:
		phy_info->mdio_unit = 1;
		break;
	}
	/* Find the GPIO MUX controller */
	pgpio_handle = (uint32_t *)fdt_getprop(fdt_addr, mux_offset,
					       "gpios", &len);
	if (!pgpio_handle || len < 12 || (len % 12) != 0 ||
	    len > CVMX_PHY_MUX_MAX_GPIO * 12) {
		cvmx_dprintf("Invalid GPIO for muxed MDIO controller in DT\n");
		return -1;
	}

	for (gpio_count = 0; gpio_count < len / 12; gpio_count++) {
		phandle = fdt32_to_cpu(pgpio_handle[gpio_count * 3]);
		phy_info->gpio[gpio_count] =
			 fdt32_to_cpu(pgpio_handle[gpio_count * 3 + 1]);
		gpio_offset = fdt_node_offset_by_phandle(fdt_addr, phandle);
		if (gpio_offset < 0) {
			cvmx_dprintf("Cannot access parent GPIO node in DT\n");
			return -1;
		}
		if (!fdt_node_check_compatible(fdt_addr, gpio_offset,
					       "cavium,octeon-3860-gpio")) {
			phy_info->gpio_type[gpio_count] = GPIO_OCTEON;
		} else if (!fdt_node_check_compatible(fdt_addr, gpio_offset,
						      "nxp,pca8574")) {

			/* GPIO is a TWSI GPIO unit which might sit behind
			 * another mux.
			 */
			phy_info->gpio_type[gpio_count] = GPIO_PCA8574;
			prop_val = (uint32_t *)fdt_getprop(fdt_addr,
							     gpio_offset, "reg",
							     NULL);
			if (!prop_val) {
				cvmx_dprintf("Could not find TWSI address of npx pca8574 GPIO from DT\n");
				return -1;
			}
			/* Get the TWSI address of the GPIO unit */
			phy_info->cvmx_gpio_twsi[gpio_count] =
						 fdt32_to_cpu(*prop_val);
			/* Get the selector on the GPIO mux if present */
			offset = fdt_parent_offset(fdt_addr, gpio_offset);
			prop_val = (uint32_t *)fdt_getprop(fdt_addr, offset,
							   "reg", NULL);
			if (prop_val) {
				phy_info->gpio_parent_mux_select =
					 fdt32_to_cpu(*prop_val);
				/* Go up another level */
				offset = fdt_parent_offset(fdt_addr, offset);
				if (!fdt_node_check_compatible(fdt_addr, offset,
							       "nxp,pca9548")) {
					prop_val = (uint32_t *)
						fdt_getprop(fdt_addr, offset,
							    "reg", NULL);
					if (!prop_val) {
						cvmx_dprintf("Could not read MUX TWSI address from DT\n");
						return -1;
					}
					phy_info->gpio_parent_mux_twsi =
							fdt32_to_cpu(*prop_val);
				}
			}
		} else {
			prop_name = (char *)fdt_getprop(fdt_addr, gpio_offset,
							"compatible", NULL);
			cvmx_dprintf("Unknown GPIO type %s\n", prop_name);
			return -1;
		}
	}
	return 0;
}

/**
 * Returns if a port is present on an interface
 *
 * @param fdt_addr - address fo flat device tree
 * @param ipd_port - IPD port number
 *
 * @return 1 if port is present, 0 if not present, -1 if error
 */
int __cvmx_helper_board_get_port_from_dt(void *fdt_addr, int ipd_port)
{
	int interface_num, port_index;
	int aliases;
	const char *pip_path;
	char name_buffer[24];
	int pip, iface, eth;
	int dbg = device_tree_dbg;
	cvmx_helper_interface_mode_t mode;
	uint32_t *val;

	interface_num = cvmx_helper_get_interface_num(ipd_port);
	mode = cvmx_helper_interface_get_mode(interface_num);

        switch (mode) {
        /* Device tree has information about the following mode types. */
        case CVMX_HELPER_INTERFACE_MODE_RGMII:
        case CVMX_HELPER_INTERFACE_MODE_GMII:
        case CVMX_HELPER_INTERFACE_MODE_SPI:
        case CVMX_HELPER_INTERFACE_MODE_XAUI:
        case CVMX_HELPER_INTERFACE_MODE_SGMII:
	case CVMX_HELPER_INTERFACE_MODE_QSGMII:
        case CVMX_HELPER_INTERFACE_MODE_RXAUI:
	case CVMX_HELPER_INTERFACE_MODE_AGL:
		aliases = 1;
		break;
	default:
		aliases = 0;
		break;
	}

	/* The device tree information is present on interfaces that have phy */
	if (!aliases)
		return 1;

	port_index = cvmx_helper_get_interface_index_num(ipd_port);

	aliases = fdt_path_offset(fdt_addr, "/aliases");
	if (aliases < 0) {
		cvmx_dprintf("%s: Error: /aliases not found in device tree fdt_addr=%p\n",
			     __func__, fdt_addr);
		return -1;
	}

	pip_path = fdt_getprop(fdt_addr, aliases, "pip", NULL);
	if (!pip_path) {
		cvmx_dprintf("%s: Error: pip path not found in device tree\n",
			     __func__);
		return -1;
	}
	pip = fdt_path_offset(fdt_addr, pip_path);
	if (pip < 0) {
		cvmx_dprintf("%s: Error: pip not found in device tree\n", __func__);
		return -1;
	}
	snprintf(name_buffer, sizeof(name_buffer), "interface@%d",
		 interface_num);
	iface = fdt_subnode_offset(fdt_addr, pip, name_buffer);
	if (iface < 0)
		return 0;
	snprintf(name_buffer, sizeof(name_buffer), "ethernet@%x", port_index);
	eth = fdt_subnode_offset(fdt_addr, iface, name_buffer);
	if (dbg)
		cvmx_dprintf("%s: eth subnode offset %d from %s\n",
			     __func__, eth, name_buffer);

	if (eth < 0)
		return -1;

	if (fdt_getprop(fdt_addr, eth, "cavium,sgmii-mac-phy-mode", NULL))
		cvmx_helper_set_mac_phy_mode(interface_num, port_index, true);
	else
		cvmx_helper_set_mac_phy_mode(interface_num, port_index, false);

	if (fdt_getprop(fdt_addr, eth, "cavium,force-link-up", NULL))
		cvmx_helper_set_port_force_link_up(interface_num, port_index,
						   true);
	else
		cvmx_helper_set_port_force_link_up(interface_num, port_index,
						   false);

	if (fdt_getprop(fdt_addr, eth, "cavium,sgmii-mac-1000x-mode", NULL))
		cvmx_helper_set_1000x_mode(interface_num, port_index, true);
	else
		cvmx_helper_set_1000x_mode(interface_num, port_index, false);

	if (mode == CVMX_HELPER_INTERFACE_MODE_AGL) {
		if (fdt_getprop(fdt_addr, eth,
				"cavium,rx-clk-delay-bypass", NULL))
			cvmx_helper_set_agl_rx_clock_delay_bypass(interface_num,
								  port_index,
								  true);
		else
			cvmx_helper_set_agl_rx_clock_delay_bypass(interface_num,
								  port_index,
								  false);

		val = (uint32_t *)fdt_getprop(fdt_addr, eth,
					      "cavium,rx-clk-skew", NULL);

		cvmx_helper_set_agl_rx_clock_skew(interface_num, port_index,
						  (val) ?
						  fdt32_to_cpu(*val) : 0);
	}
	return (eth >= 0);
}

/**
 * Return the MII PHY address associated with the given IPD
 * port. The phy address is obtained from the device tree.
 *
 * @param[out] phy_info - PHY information data structure updated
 * @param ipd_port Octeon IPD port to get the MII address for.
 *
 * @return MII PHY address and bus number, -1 on error, -2 if PHY info missing (OK).
 */
int __get_phy_info_from_dt(cvmx_phy_info_t *phy_info, int ipd_port)
{
	static void *fdt_addr = 0;

	if (fdt_addr == 0)
		fdt_addr = __cvmx_phys_addr_to_ptr(cvmx_sysinfo_get()->fdt_addr,
						   OCTEON_FDT_MAX_SIZE);

	uint32_t *phy_handle;
	int aliases, eth, phy, phy_parent, phandle, ret, len, i;
	int mdio_parent;
	const char *phy_compatible_str;
	const char *host_mode_str = NULL;
	uint32_t *phy_addr_ptr;
	uint32_t *psmi_handle;
	int smi_offset;
	uint64_t *smi_addrp;
	uint64_t smi_addr = 0;
	int dbg = device_tree_dbg;
	int interface;

	phy_info->phy_addr = -1;
	phy_info->direct_connect = -1;
	phy_info->phy_type = (cvmx_phy_type_t) -1;
	for (i = 0; i < CVMX_PHY_MUX_MAX_GPIO; i++)
		phy_info->gpio[i] = -1;
	phy_info->mdio_unit = -1;
	phy_info->gpio_value = -1;
	phy_info->gpio_parent_mux_twsi = -1;
	phy_info->gpio_parent_mux_select = -1;

	if (!fdt_addr) {
		cvmx_dprintf("No device tree found.\n");
		return -1;
	}

	aliases = fdt_path_offset(fdt_addr, "/aliases");
	if (aliases < 0) {
		cvmx_dprintf("Error: No /aliases node in device tree.\n");
		return -1;
	}
	if (ipd_port < 0) {
		int interface_index = ipd_port - CVMX_HELPER_BOARD_MGMT_IPD_PORT;
		eth = __mix_eth_node(fdt_addr, aliases, interface_index);
	} else {
		eth = __pip_eth_node(fdt_addr, aliases, ipd_port);
	}
	if (eth < 0) {
		if (dbg)
			cvmx_dprintf("ERROR : cannot find interface for "
				     "ipd_port=%d\n", ipd_port);
		return -1;
	}

	interface = cvmx_helper_get_interface_num(ipd_port);
	/* Get handle to phy */
	phy_handle = (uint32_t *) fdt_getprop(fdt_addr, eth, "phy-handle", NULL);
	if (!phy_handle) {
		cvmx_helper_interface_mode_t if_mode;
		/* Note that it's OK for RXAUI and ILK to not have a PHY
		 * connected (i.e. EBB boards in loopback).
		 */
		if_mode = cvmx_helper_interface_get_mode(interface);
		if (if_mode != CVMX_HELPER_INTERFACE_MODE_RXAUI &&
		    if_mode != CVMX_HELPER_INTERFACE_MODE_ILK) {
			if (dbg)
				cvmx_dprintf("ERROR : phy handle not found in device tree ipd_port=%d" "\n",
					     ipd_port);
			return -1;
		} else {
			return -2;
		}
	}
	phandle = fdt32_to_cpu(*phy_handle);
	phy = fdt_node_offset_by_phandle(fdt_addr, phandle);
	if (phy < 0) {
		cvmx_dprintf("ERROR : cannot find phy for ipd_port=%d ret=%d\n",
			     ipd_port, phy);
		return -1;
	}

	phy_compatible_str = (const char *)fdt_getprop(fdt_addr, phy,
						       "compatible", NULL);
	if (!phy_compatible_str) {
		cvmx_dprintf("ERROR: no compatible prop in phy\n");
		return -1;
	}
	if (device_tree_dbg)
		cvmx_dprintf("Checking compatible string \"%s\" for ipd port %d\n",
			     phy_compatible_str, ipd_port);
	if (!memcmp("marvell", phy_compatible_str, strlen("marvell"))) {
		if (device_tree_dbg)
			cvmx_dprintf("Marvell PHY detected for ipd_port %d\n",
				     ipd_port);
		phy_info->phy_type = MARVELL_GENERIC_PHY;
	} else if (!memcmp("broadcom", phy_compatible_str, strlen("broadcom"))) {
		phy_info->phy_type = BROADCOM_GENERIC_PHY;
		if (device_tree_dbg)
			cvmx_dprintf("Broadcom PHY detected for ipd_port %d\n",
				     ipd_port);
	} else if (!memcmp("vitesse", phy_compatible_str, strlen("vitesse"))) {
		if (device_tree_dbg)
			cvmx_dprintf("Vitesse PHY detected for ipd_port %d\n",
				     ipd_port);
		if (!fdt_node_check_compatible(fdt_addr, phy,
					       "ethernet-phy-ieee802.3-c22"))
			phy_info->phy_type = GENERIC_8023_C22_PHY;
		else
			phy_info->phy_type = VITESSE_GENERIC_PHY;
	} else if (!memcmp("cortina", phy_compatible_str, strlen("cortina"))) {
		phy_info->phy_type = CORTINA_PHY;
		host_mode_str = (const char *)fdt_getprop(fdt_addr, phy,
							  "cortina,host-mode",
							  NULL);
	} else if (!fdt_node_check_compatible(fdt_addr, phy,
					      "ethernet-phy-ieee802.3-c22")) {
		phy_info->phy_type = GENERIC_8023_C22_PHY;
	} else {
		cvmx_dprintf("Unknown PHY compatibility\n");
		phy_info->phy_type = -1;
	}

	phy_info->host_mode = CVMX_PHY_HOST_MODE_UNKNOWN;
	if (host_mode_str) {
		if (strcmp(host_mode_str, "rxaui") == 0) {
			phy_info->host_mode = CVMX_PHY_HOST_MODE_RXAUI;
		} else if (strcmp(host_mode_str, "xaui") == 0) {
			phy_info->host_mode = CVMX_PHY_HOST_MODE_XAUI;
		} else if (strcmp(host_mode_str, "sgmii") == 0) {
			phy_info->host_mode = CVMX_PHY_HOST_MODE_SGMII;
		} else if (strcmp(host_mode_str, "qsgmii") == 0) {
			phy_info->host_mode = CVMX_PHY_HOST_MODE_QSGMII;
		} else {
			cvmx_dprintf("Unknown PHY host mode\n");
		}
	}

	/* Check if PHY parent is the octeon MDIO bus. Some boards are connected
	   though a MUX and for them direct_connect_to_phy will be 0 */
	phy_parent = fdt_parent_offset(fdt_addr, phy);
	if (phy_parent < 0) {
		cvmx_dprintf("ERROR : cannot find phy parent for ipd_port=%d ret=%d\n",
			     ipd_port, phy_parent);
		return -1;
	}
	/* For multi-phy devices and devices on a MUX, go to the parent */
	ret = fdt_node_check_compatible(fdt_addr, phy_parent,
					"ethernet-phy-nexus");
	if (ret == 0)
		phy_parent = fdt_parent_offset(fdt_addr, phy_parent);

	/* Check for a muxed MDIO interface */
	mdio_parent = fdt_parent_offset(fdt_addr, phy_parent);
	ret = fdt_node_check_compatible(fdt_addr, mdio_parent,
					"cavium,mdio-mux");
	if (ret == 0) {
		ret = __get_muxed_mdio_info_from_dt(phy_info, phy_parent,
						    mdio_parent);
		/* Find the parent MDIO bus */
		psmi_handle = (uint32_t *)fdt_getprop(fdt_addr, mdio_parent,
						      "mdio-parent-bus", NULL);
		if (psmi_handle) {
			phandle = fdt32_to_cpu(*psmi_handle);
			smi_offset = fdt_node_offset_by_phandle(fdt_addr,
								phandle);
			if (smi_offset > 0) {
				smi_addrp = (uint64_t *)fdt_getprop(fdt_addr,
								    smi_offset,
								    "reg",
								    &len);
				if (smi_addrp != NULL && len > 8) {
					memcpy(&smi_addr, smi_addrp,
					       sizeof(uint64_t));
					smi_addr = fdt64_to_cpu(smi_addr);
				}
			} else {
				cvmx_dprintf("Could not find SMI handler for mux\n");
			}
		} else {
			cvmx_dprintf("%s: Could not get parent mdio bus\n",
				     __func__);
		}
		/* Find the GPIO MUX controller */
	}
	ret = fdt_node_check_compatible(fdt_addr, phy_parent, "cavium,octeon-3860-mdio");
	if (ret == 0) {
		phy_info->direct_connect = 1;
		uint32_t *mdio_reg_base = (uint32_t *) fdt_getprop(fdt_addr, phy_parent, "reg", 0);
		if (mdio_reg_base == 0) {
			cvmx_dprintf("ERROR : unable to get reg property in phy mdio\n");
			return -1;
		}
		phy_info->mdio_unit = __mdiobus_addr_to_unit(fdt32_to_cpu(mdio_reg_base[1]));
		if (device_tree_dbg) {
			cvmx_dprintf("phy parent=%s reg_base=%08x mdio_unit=%d \n",
				     fdt_get_name(fdt_addr, phy_parent, NULL),
				     (int)mdio_reg_base[1], phy_info->mdio_unit);
		}
	} else {
		phy_info->direct_connect = 0;
		/* The PHY is not directly connected to the Octeon MDIO bus.
		   SE doesn't  have abstractions for MDIO MUX or MDIO MUX drivers and
		   hence for the non direct cases code will be needed which is
		   board specific.
		   For now the the MDIO Unit is defaulted to 1.
		 */
		if (device_tree_dbg)
			cvmx_dprintf("%s PHY at address: %d is not directly connected\n",
				      __func__, phy_info->phy_addr);
	}

	phy_addr_ptr = (uint32_t *) fdt_getprop(fdt_addr, phy, "reg", NULL);
	if (!phy_addr_ptr) {
		cvmx_dprintf("ERROR: Could not read phy address from reg in DT\n");
		return -1;
	}
	phy_info->phy_addr = fdt32_to_cpu(*phy_addr_ptr) |
				phy_info->mdio_unit << 8;
	return phy_info->phy_addr;
}

/**
 * @INTERNAL
 * This function outputs the cvmx_phy_info_t data structure for the specified
 * port.
 *
 * @param - phy_info - phy info data structure
 * @param ipd_port - port to get phy info for
 *
 * @return 0 for success, -1 if info not available
 *
 * NOTE: The phy_info data structure is subject to change.
 */
int cvmx_helper_board_get_phy_info(cvmx_phy_info_t *phy_info, int ipd_port)
{
	int retcode;

	retcode = __get_phy_info_from_dt(phy_info, ipd_port);

	return (retcode >= 0) ? 0 : -1;
}

/**
 * Return the MII PHY address associated with the given IPD
 * port. The phy address is obtained from the device tree.
 *
 * @param ipd_port Octeon IPD port to get the MII address for.
 *
 * @return MII PHY address and bus number or -1 on error, -2 if phy info missing (OK).
 */

int cvmx_helper_board_get_mii_address_from_dt(int ipd_port)
{
	cvmx_phy_info_t phy_info;
	int retcode = __get_phy_info_from_dt(&phy_info, ipd_port);
	if (retcode >= 0)
		return phy_info.phy_addr;
	else
		return retcode;
}

/**
 * @INTERNAL
 * Return the host mode for the PHY from the device tree.
 * Currently only the Cortina CS4321 PHY needs this.
 *
 * @param ipd_port - ipd port number to get the host mode for
 *
 * @return host mode for phy
 */
cvmx_phy_host_mode_t __cvmx_helper_board_get_phy_host_mode_from_dt(int ipd_port)
{
	cvmx_phy_info_t phy_info;
	int retcode = __get_phy_info_from_dt(&phy_info, ipd_port);
	if (retcode < 0)
		return CVMX_PHY_HOST_MODE_UNKNOWN;
	return phy_info.host_mode;
}

/**
 * Return the host mode for the PHY.  Currently only the Cortina CS4321 PHY
 * needs this.
 *
 * @param ipd_port - ipd port number to get the host mode for
 *
 * @return host mode for phy
 */
cvmx_phy_host_mode_t cvmx_helper_board_get_phy_host_mode(int ipd_port)
{
	return __cvmx_helper_board_get_phy_host_mode_from_dt(ipd_port);
}
#endif	/* !CVMX_BUILD_FOR_LINUX_KERNEL */

/**
 * Return the MII PHY address associated with the given IPD
 * port. A result of -1 means there isn't a MII capable PHY
 * connected to this port. On chips supporting multiple MII
 * busses the bus number is encoded in bits <15:8>.
 *
 * This function must be modified for every new Octeon board.
 * Internally it uses switch statements based on the cvmx_sysinfo
 * data to determine board types and revisions. It replies on the
 * fact that every Octeon board receives a unique board type
 * enumeration from the bootloader.
 *
 * @param ipd_port Octeon IPD port to get the MII address for.
 *
 * @return MII PHY address and bus number or -1.
 */
int cvmx_helper_board_get_mii_address(int ipd_port)
{
#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	int dbg = device_tree_dbg;
#endif
	if (cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_SIM)
		return -1;
#ifndef CVMX_BUILD_FOR_LINUX_KERNEL

	if (cvmx_sysinfo_get()->fdt_addr) {
		cvmx_phy_info_t phy_info;
		int retcode = __get_phy_info_from_dt(&phy_info, ipd_port);

		if (retcode == -2) {
			return retcode;
		} else if (retcode < 0) {
			if (dbg)
				cvmx_dprintf("%s: could not get phy info for port %d\n",
					     __func__, ipd_port);
			return retcode;
		}
		if (phy_info.phy_addr >= 0) {
			if (phy_info.direct_connect == 0) {
				if (device_tree_dbg)
					cvmx_dprintf("%s: Phy at address %d not directly connected, configuring mux\n",
						     __func__,
						     phy_info.phy_addr);
				__switch_mdio_mux(ipd_port, &phy_info);
			}
			return phy_info.phy_addr;
		}
	}
#endif
	switch (cvmx_sysinfo_get()->board_type) {
	case CVMX_BOARD_TYPE_SIM:
		/* Simulator doesn't have MII */
		return -1;
	case CVMX_BOARD_TYPE_EBT3000:
	case CVMX_BOARD_TYPE_EBT5800:
	case CVMX_BOARD_TYPE_THUNDER:
	case CVMX_BOARD_TYPE_NICPRO2:
		/* Interface 0 is SPI4, interface 1 is RGMII */
		if ((ipd_port >= 16) && (ipd_port < 20))
			return ipd_port - 16;
		else
			return -1;
	case CVMX_BOARD_TYPE_LANAI2_A:
		if (ipd_port == 0)
			return 0;
		else
			return -1;
	case CVMX_BOARD_TYPE_LANAI2_U:
	case CVMX_BOARD_TYPE_LANAI2_G:
		if (ipd_port == 0)
			return 0x1c;
		else
			return -1;
	case CVMX_BOARD_TYPE_KODAMA:
	case CVMX_BOARD_TYPE_EBH3100:
	case CVMX_BOARD_TYPE_HIKARI:
	case CVMX_BOARD_TYPE_CN3010_EVB_HS5:
	case CVMX_BOARD_TYPE_CN3005_EVB_HS5:
	case CVMX_BOARD_TYPE_CN3020_EVB_HS5:
		/*
		 * Port 0 is WAN connected to a PHY, Port 1 is GMII
		 * connected to a switch
		 */
		if (ipd_port == 0)
			return 4;
		else if (ipd_port == 1)
			return 9;
		else
			return -1;
	case CVMX_BOARD_TYPE_NAC38:
		/* Board has 8 RGMII ports PHYs are 0-7 */
		if ((ipd_port >= 0) && (ipd_port < 4))
			return ipd_port;
		else if ((ipd_port >= 16) && (ipd_port < 20))
			return ipd_port - 16 + 4;
		else
			return -1;
	case CVMX_BOARD_TYPE_EBH3000:
		/* Board has dual SPI4 and no PHYs */
		return -1;
	case CVMX_BOARD_TYPE_EBT5810:
		/*
		 * Board has 10g PHYs hooked up to the MII controller
		 * on the IXF18201 MAC.  The 10G PHYS use clause 45
		 * MDIO which the CN58XX does not support. All MII
		 * accesses go through the IXF part.
		 */
		return -1;
	case CVMX_BOARD_TYPE_EBH5200:
	case CVMX_BOARD_TYPE_EBH5201:
	case CVMX_BOARD_TYPE_EBT5200:
		/* Board has 2 management ports */
		if ((ipd_port >= CVMX_HELPER_BOARD_MGMT_IPD_PORT) &&
		    (ipd_port < (CVMX_HELPER_BOARD_MGMT_IPD_PORT + 2)))
			return ipd_port - CVMX_HELPER_BOARD_MGMT_IPD_PORT;
		/*
		 * Board has 4 SGMII ports. The PHYs start right after the MII
		 * ports MII0 = 0, MII1 = 1, SGMII = 2-5.
		 */
		if ((ipd_port >= 0) && (ipd_port < 4))
			return ipd_port + 2;
		else
			return -1;
	case CVMX_BOARD_TYPE_EBH5600:
	case CVMX_BOARD_TYPE_EBH5601:
	case CVMX_BOARD_TYPE_EBH5610:
		/* Board has 1 management port */
		if (ipd_port == CVMX_HELPER_BOARD_MGMT_IPD_PORT)
			return 0;
		/*
		 * Board has 8 SGMII ports. 4 connect out, two connect
		 * to a switch, and 2 loop to each other
		 */
		if ((ipd_port >= 0) && (ipd_port < 4))
			return ipd_port + 1;
		else
			return -1;
	case CVMX_BOARD_TYPE_EBB5600:
		{
			static unsigned char qlm_switch_addr = 0;

			/* Board has 1 management port */
			if (ipd_port == CVMX_HELPER_BOARD_MGMT_IPD_PORT)
				return 0;

			/* Board has 8 SGMII ports. 4 connected QLM1, 4 connected QLM3 */
			if ((ipd_port >= 0) && (ipd_port < 4)) {
				if (qlm_switch_addr != 0x3) {
					qlm_switch_addr = 0x3;	/* QLM1 */
					cvmx_twsix_write_ia(0, 0x71, 0, 1, 1, qlm_switch_addr);
					cvmx_wait_usec(11000);	/* Let the write complete */
				}
				return ipd_port + 1 + (1 << 8);
			} else if ((ipd_port >= 16) && (ipd_port < 20)) {
				if (qlm_switch_addr != 0xC) {
					qlm_switch_addr = 0xC;	/* QLM3 */
					cvmx_twsix_write_ia(0, 0x71, 0, 1, 1, qlm_switch_addr);
					cvmx_wait_usec(11000);	/* Let the write complete */
				}
				return ipd_port - 16 + 1 + (1 << 8);
			} else
				return -1;
		}
	case CVMX_BOARD_TYPE_EBB6300:
		/* Board has 2 management ports */
		if ((ipd_port >= CVMX_HELPER_BOARD_MGMT_IPD_PORT) && (ipd_port < (CVMX_HELPER_BOARD_MGMT_IPD_PORT + 2)))
			return ipd_port - CVMX_HELPER_BOARD_MGMT_IPD_PORT + 4;
		if ((ipd_port >= 0) && (ipd_port < 4))
			return ipd_port + 1 + (1 << 8);
		else
			return -1;
	case CVMX_BOARD_TYPE_EBB6800:
		/* Board has 1 management ports */
		if (ipd_port == CVMX_HELPER_BOARD_MGMT_IPD_PORT)
			return 6;
		if (ipd_port >= 0x800 && ipd_port < 0x900)	/* QLM 0 */
			return 0x101 + ((ipd_port >> 4) & 3);	/* SMI 1 */
		if (ipd_port >= 0xa00 && ipd_port < 0xb00)	/* QLM 2 */
			return 0x201 + ((ipd_port >> 4) & 3);	/* SMI 2 */
		if (ipd_port >= 0xb00 && ipd_port < 0xc00)	/* QLM 3 */
			return 0x301 + ((ipd_port >> 4) & 3);	/* SMI 3 */
		if (ipd_port >= 0xc00 && ipd_port < 0xd00)	/* QLM 4 */
			return 0x001 + ((ipd_port >> 4) & 3);	/* SMI 0 */
		return -1;
	case CVMX_BOARD_TYPE_EP6300C:
		if (ipd_port == CVMX_HELPER_BOARD_MGMT_IPD_PORT)
			return 0x01;
		if (ipd_port == CVMX_HELPER_BOARD_MGMT_IPD_PORT + 1)
			return 0x02;
		{
			int interface = cvmx_helper_get_interface_num(ipd_port);
			int mode = cvmx_helper_interface_get_mode(interface);
			if (mode == CVMX_HELPER_INTERFACE_MODE_XAUI)
				return ipd_port;
			else if ((ipd_port >= 0) && (ipd_port < 4))
				return ipd_port + 3;
			else
				return -1;
		}
		break;
	case CVMX_BOARD_TYPE_CUST_NB5:
		if (ipd_port == 2)
			return 4;
		else
			return -1;
	case CVMX_BOARD_TYPE_NIC_XLE_4G:
		/* Board has 4 SGMII ports. connected QLM3(interface 1) */
		if ((ipd_port >= 16) && (ipd_port < 20))
			return ipd_port - 16 + 1;
		else
			return -1;
	case CVMX_BOARD_TYPE_NIC_XLE_10G:
	case CVMX_BOARD_TYPE_NIC10E:
		return -1;
	case CVMX_BOARD_TYPE_NIC4E:
		if (ipd_port >= 0 && ipd_port <= 3)
			return (ipd_port + 0x1f) & 0x1f;
		else
			return -1;
	case CVMX_BOARD_TYPE_NIC2E:
		if (ipd_port >= 0 && ipd_port <= 1)
			return ipd_port + 1;
		else
			return -1;
	case CVMX_BOARD_TYPE_REDWING:
		return -1;	/* No PHYs connected to Octeon */
	case CVMX_BOARD_TYPE_BBGW_REF:
		/*
		 * No PHYs are connected to Octeon, everything is
		 * through switch.
		 */
		return -1;
	case CVMX_BOARD_TYPE_CUST_WSX16:
		if (ipd_port >= 0 && ipd_port <= 3)
			return ipd_port;
		else if (ipd_port >= 16 && ipd_port <= 19)
			return ipd_port - 16 + 4;
		else
			return -1;
	case CVMX_BOARD_TYPE_UBNT_E100:
		if (ipd_port >= 0 && ipd_port <= 2)
			return 7 - ipd_port;
		else
			return -1;
	}

	/* Some unknown board. Somebody forgot to update this function... */
	cvmx_dprintf("%s: Unknown board type %d\n", __func__, cvmx_sysinfo_get()->board_type);
	return -1;
}

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
EXPORT_SYMBOL(cvmx_helper_board_get_mii_address);
#endif

/**
 * @INTERNAL
 * Get link state of Cortina PHY
 *
 * @param phy_addr MDIO address of PHY
 *
 * @return link state information from PHY
 */
cvmx_helper_link_info_t __cvmx_get_cortina_phy_link_state(int phy_addr)
{
	cvmx_helper_link_info_t result;
	int value;
#define CS4321_GIGEPCS_LINE_STATUS			0xC01
#define CS4321_GPIO_GPIO_INTS				0x16D
#define CS4321_GLOBAL_GT_10KHZ_REF_CLK_CNT0		0x2E

	result.u64 = 0;
	result.s.full_duplex = 1;

	/* Right now to determine the speed we look at the reference clock.
	 * There's probably a better way but this should work.  For SGMII
	 * we use a 100MHz reference clock, for XAUI/RXAUI we use a 156.25MHz
	 * reference clock.
	 */
	value = cvmx_mdio_45_read(phy_addr >> 8, phy_addr & 0xff,
				  0, CS4321_GLOBAL_GT_10KHZ_REF_CLK_CNT0);
	result.s.speed = (value == 10000) ? 1000 : 10000;

	if (result.s.speed == 1000) {
		value = cvmx_mdio_45_read(phy_addr >> 8, phy_addr & 0xff, 0,
					  CS4321_GIGEPCS_LINE_STATUS);
		result.s.link_up = (value >= 0) && !!(value & 4);
	} else {
		value = cvmx_mdio_45_read(phy_addr >> 8, phy_addr & 0xff, 0,
					  CS4321_GPIO_GPIO_INTS);
		result.s.link_up = (value >= 0) && !(value & 3);
	}

	return result;
}

/**
 * @INTERNAL
 * Get link state of Vitesse PHY
 */
static cvmx_helper_link_info_t __get_vitesse_phy_link_state(int phy_addr)
{
	cvmx_helper_link_info_t result;
	int phy_status;
	int pma_ctrl1;

	result.u64 = 0;
	pma_ctrl1 = cvmx_mdio_45_read(phy_addr >> 8, phy_addr & 0xff, 1, 0);
	if ((pma_ctrl1 & 0x207c) == 0x2040)
		result.s.speed = 10000;
	/* PMA Status 1 (1x0001) */
	phy_status = cvmx_mdio_45_read(phy_addr >> 8, phy_addr & 0xff, 1, 0xa);
	if (phy_status < 0)
		return result;

	result.s.full_duplex = 1;
	if ((phy_status & 1) == 0) {
		return result;
	}
	phy_status = cvmx_mdio_45_read(phy_addr >> 8, phy_addr & 0xff, 4, 0x18);
	if (phy_status < 0)
		return result;
	result.s.link_up = (phy_status & 0x1000) ? 1 : 0;

	return result;
}

/**
 * @INTERNAL
 * Get link state of marvell PHY
 */
static cvmx_helper_link_info_t __get_marvell_phy_link_state(int phy_addr)
{
	cvmx_helper_link_info_t result;
	int phy_status;

	result.u64 = 0;
	/* Set to page 0 */
	cvmx_mdio_write(phy_addr >> 8, phy_addr & 0xff, 22, 0);
	/* All the speed information can be read from register 17 in one go. */
	phy_status = cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff, 17);

	/* If the resolve bit 11 isn't set, see if autoneg is turned off
	   (bit 12, reg 0). The resolve bit doesn't get set properly when
	   autoneg is off, so force it */
	if ((phy_status & (1 << 11)) == 0) {
		int auto_status = cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff, 0);
		if ((auto_status & (1 << 12)) == 0)
			phy_status |= 1 << 11;
	}

	/* Only return a link if the PHY has finished auto negotiation
	   and set the resolved bit (bit 11) */
	if (phy_status & (1 << 11)) {
		result.s.link_up = 1;
		result.s.full_duplex = ((phy_status >> 13) & 1);
		switch ((phy_status >> 14) & 3) {
		case 0:	/* 10 Mbps */
			result.s.speed = 10;
			break;
		case 1:	/* 100 Mbps */
			result.s.speed = 100;
			break;
		case 2:	/* 1 Gbps */
			result.s.speed = 1000;
			break;
		case 3:	/* Illegal */
			result.u64 = 0;
			break;
		}
	}
	return result;
}

/**
 * @INTERNAL
 * Get link state of broadcom PHY
 */
static cvmx_helper_link_info_t __get_broadcom_phy_link_state(int phy_addr)
{
	cvmx_helper_link_info_t result;
	int phy_status;

	result.u64 = 0;
	/* Below we are going to read SMI/MDIO register 0x19 which works
	   on Broadcom parts */
	phy_status = cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff, 0x19);
	switch ((phy_status >> 8) & 0x7) {
	case 0:
		result.u64 = 0;
		break;
	case 1:
		result.s.link_up = 1;
		result.s.full_duplex = 0;
		result.s.speed = 10;
		break;
	case 2:
		result.s.link_up = 1;
		result.s.full_duplex = 1;
		result.s.speed = 10;
		break;
	case 3:
		result.s.link_up = 1;
		result.s.full_duplex = 0;
		result.s.speed = 100;
		break;
	case 4:
		result.s.link_up = 1;
		result.s.full_duplex = 1;
		result.s.speed = 100;
		break;
	case 5:
		result.s.link_up = 1;
		result.s.full_duplex = 1;
		result.s.speed = 100;
		break;
	case 6:
		result.s.link_up = 1;
		result.s.full_duplex = 0;
		result.s.speed = 1000;
		break;
	case 7:
		result.s.link_up = 1;
		result.s.full_duplex = 1;
		result.s.speed = 1000;
		break;
	}
	return result;
}

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
/**
 * @INTERNAL
 * Get link state of generic gigabit PHY
 *
 * @param phy_addr - address of PHY
 *
 * @returns link status of the PHY
 */
static cvmx_helper_link_info_t
__cvmx_get_generic_8023_c22_phy_link_state(int phy_addr)
{
	cvmx_helper_link_info_t result;
	int phy_basic_control;	/* Register 0x0 */
	int phy_basic_status;	/* Register 0x1 */
	int phy_anog_adv;	/* Register 0x4 */
	int phy_link_part_avail;/* Register 0x5 */
	int phy_control;	/* Register 0x9 */
	int phy_status;		/* Register 0xA */

	result.u64 = 0;

	phy_basic_status = cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff, 1);
	if (!(phy_basic_status & 0x4))	/* Check if link is up */
		return result;			/* Link is down, return link down */

	result.s.link_up = 1;
	phy_basic_control = cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff, 0);
	/* Check if autonegotiation is enabled and completed */
	if ((phy_basic_control & (1 << 12)) && (phy_basic_status & (1 << 5))) {
		phy_status = cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff, 0xA);
		phy_control = cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff, 0x9);

		phy_status &= phy_control << 2;
		phy_link_part_avail = cvmx_mdio_read(phy_addr >> 8,
						     phy_addr & 0xff, 0x5);
		phy_anog_adv = cvmx_mdio_read(phy_addr >> 8,
					      phy_addr & 0xff, 0x4);
		phy_link_part_avail &= phy_anog_adv;

		if (phy_status & 0xC00) {	/* Gigabit full or half */
			result.s.speed = 1000;
			result.s.full_duplex = !!(phy_status & 0x800);
		} else if (phy_link_part_avail & 0x0180) { /* 100 full or half */
			result.s.speed = 100;
			result.s.full_duplex = !!(phy_link_part_avail & 0x100);
		} else if (phy_link_part_avail & 0x0060) {
			result.s.speed = 10;
			result.s.full_duplex = !!(phy_link_part_avail & 0x0040);
		}
	} else {
		/* Not autonegotiated */
		result.s.full_duplex = !!(phy_basic_control & (1 << 8));

		if (phy_basic_control & (1 << 6))
			result.s.speed = 1000;
		else if (phy_basic_control & (1 << 13))
			result.s.speed = 100;
		else
			result.s.speed = 10;
	}
	return result;
}
#endif

/**
 * @INTERNAL
 * Get link state using inband status
 */
static cvmx_helper_link_info_t __get_inband_link_state(int ipd_port)
{
	cvmx_helper_link_info_t result;
	union cvmx_gmxx_rxx_rx_inbnd inband_status;
	int interface = cvmx_helper_get_interface_num(ipd_port);
	int index = cvmx_helper_get_interface_index_num(ipd_port);

	result.u64 = 0;
	inband_status.u64 = cvmx_read_csr(CVMX_GMXX_RXX_RX_INBND(index, interface));
	result.s.link_up = inband_status.s.status;
	result.s.full_duplex = inband_status.s.duplex;
	switch (inband_status.s.speed) {
	case 0:		/* 10 Mbps */
		result.s.speed = 10;
		break;
	case 1:		/* 100 Mbps */
		result.s.speed = 100;
		break;
	case 2:		/* 1 Gbps */
		result.s.speed = 1000;
		break;
	case 3:		/* Illegal */
		result.u64 = 0;
		break;
	}
	return result;
}

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
/**
 * @INTERNAL Writes the specified value to the TWSI MUX at the specified address
 *
 * @param twsi_addr - address on TWSI bus of the MUX
 * @param value - value to write to the mux.  Typically only one bit is set
 *
 * @returns the previous value of the MUX
 */
static int __set_twsi_mux(int twsi_addr, int value)
{
	int tries = 3;
	int old_val;

	do {
		old_val = cvmx_twsix_read8(0, twsi_addr, 0);
	} while (tries-- > 0 && old_val < 0);
	if (tries == 0)
		return -1;
	cvmx_retry_i2c_write(0, twsi_addr, 0, 1, 0, value);

	return old_val;
}
/**
 * @INTERNAL sets one or more GPIO pins in order to configure a MDIO mux
 *
 * @param[in] gpio_pin - array of num_bits gpio pins used to encode the value
 * @param num_bits - number of pins used to hold the value
 * @param type - type of GPIO, can be GPIO_OCTEON or GPIO_PCA8574
 * @param twsi_addr - TWSI address of NXP PCA8574, not used for OCTEON
 * @param value - value to place on the GPIO pins
 */
static void __set_gpio(const int gpio_pin[], int num_bits,
		       cvmx_phy_gpio_type_t type,
		       int twsi_addr, int value)
{
	int i;
	uint8_t new_value;

	switch (type) {
	case GPIO_OCTEON:
		for (i = 0; i < num_bits; i++) {
			cvmx_gpio_cfg(gpio_pin[i], 1);
			if ((value >> i) & 1)
				cvmx_gpio_set(1ULL << gpio_pin[i]);
			else
				cvmx_gpio_clear(1ULL << gpio_pin[i]);
		}
		break;
	case GPIO_PCA8574:
		new_value = 0xff;	/* For now just set all other bits to 1 */
		for (i = 0; i < num_bits; i++) {
			/* Toggle any 0 bits */
			if (((value >> i) & 1) == 0)
				new_value &= ~(1 << gpio_pin[i]);
		}
		cvmx_retry_i2c_write(0, twsi_addr, 0, 1, 0, new_value);
		break;
	default:
		cvmx_dprintf("%s: Unknown GPIO type\n", __func__);
		break;
	}
}

/**
 * @INTERNAL
 * Switch MDIO mux to the specified port.
 */
static int __switch_mdio_mux(int ipd_port, const cvmx_phy_info_t *phy_info)
{
	int i;
	int num_bits = 0;
	int old_mux = 0;
	/* This method is board specific and doesn't use the device tree
	   information as SE doesn't implement MDIO MUX abstration */
	switch (cvmx_sysinfo_get()->board_type) {
	default:
		/* Non-board specific code to switch MDIO muxes.
		 *
		 * This code uses data found in the device tree to manipulate
		 * GPIO signals and muxes.  It is able to handle both OCTEON
		 * GPIO pins and GPIO pins from devices on the TWSI bus.  Note
		 * that all of the controlling GPIO pins must be of the same
		 * type.
		 *
		 * In the case of a TWSI GPIO device, the device can even sit
		 * behind a TWSI mux, as is the case with the EBB6600 board.
		 *
		 * Word to board designers: please don't do that again!
		 */
		for (i = 0; i < CVMX_PHY_MUX_MAX_GPIO; i++) {
			if (phy_info->gpio[i] != -1)
				num_bits++;
		}
		if (num_bits == 0)
			return -1;
		/* For now we assume all of the GPIO lines are contiguous and
		 * the same type.
		 */
		if (phy_info->gpio_parent_mux_twsi >= 0) {
			old_mux = __set_twsi_mux(phy_info->gpio_parent_mux_twsi,
					         1 << phy_info->gpio_parent_mux_select);
			if (old_mux < 0) {
				cvmx_dprintf("%s: Error: could not read old MUX value for port %d\n",
					     __func__, ipd_port);
				return -1;
			}
		}
		__set_gpio(phy_info->gpio, num_bits, phy_info->gpio_type[0],
			   phy_info->cvmx_gpio_twsi[0], phy_info->gpio_value);

		/* Restore the TWSI mux to its previous state */
		if (phy_info->gpio_parent_mux_twsi >= 0)
			__set_twsi_mux(phy_info->gpio_parent_mux_twsi, old_mux);
		return 0;
	}
	/* should never get here */
	return -1;
}

/**
 * @INTERNAL
 * This function is used ethernet ports link speed. This functions uses the
 * device tree information to determine the phy address and type of PHY.
 * The only supproted PHYs are Marvell and Broadcom.
 *
 * @param ipd_port IPD input port associated with the port we want to get link
 *                 status for.
 *
 * @return The ports link status. If the link isn't fully resolved, this must
 *         return zero.
 */

cvmx_helper_link_info_t __cvmx_helper_board_link_get_from_dt(int ipd_port)
{
	cvmx_helper_link_info_t result;
	cvmx_phy_info_t phy_info;

	result.u64 = 0;
	if (cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_SIM) {
		/* The simulator gives you a simulated 1Gbps full duplex link */
		result.s.link_up = 1;
		result.s.full_duplex = 1;
		result.s.speed = 1000;
		return result;
	}

	if (__get_phy_info_from_dt(&phy_info, ipd_port) < 0) {
		/* If we can't get the PHY info from the device tree then try
		 * the inband state.
		 */
		if (OCTEON_IS_OCTEON1() || OCTEON_IS_MODEL(OCTEON_CN58XX) ||
		    OCTEON_IS_MODEL(OCTEON_CN50XX)) {
			result = __get_inband_link_state(ipd_port);
		} else {
			result.s.full_duplex = 1;
			result.s.link_up = 1;
			result.s.speed = 1000;
		}
		return result;
	}

	if (phy_info.phy_addr < 0) {
		cvmx_dprintf("%s: phy address invalid for port %d\n",
			     __func__, ipd_port);
		return result;
	}


	if (phy_info.direct_connect == 0) {
		if (device_tree_dbg)
			cvmx_dprintf("%s: Phy at address %d not directly connected\n",
				     __func__, phy_info.phy_addr);
		__switch_mdio_mux(ipd_port, &phy_info);
	}

	switch (phy_info.phy_type) {
	case VITESSE_GENERIC_PHY:
		result = __get_vitesse_phy_link_state(phy_info.phy_addr);
		break;
	case BROADCOM_GENERIC_PHY:
		result = __get_broadcom_phy_link_state(phy_info.phy_addr);
		break;
	case MARVELL_GENERIC_PHY:
		result = __get_marvell_phy_link_state(phy_info.phy_addr);
		break;
	case CORTINA_PHY:
		result = __cvmx_get_cortina_phy_link_state(phy_info.phy_addr);
		break;
	case GENERIC_8023_C22_PHY:
		result = __cvmx_get_generic_8023_c22_phy_link_state(phy_info.phy_addr);
		break;
	case INBAND_PHY:
	default:
		if (OCTEON_IS_OCTEON1() ||
		    OCTEON_IS_MODEL(OCTEON_CN58XX) ||
		    OCTEON_IS_MODEL(OCTEON_CN50XX))
			/*
			 * We don't have a PHY address, so attempt to use
			 * in-band status. It is really important that boards
			 * not supporting in-band status never get
			 * here. Reading broken in-band status tends to do bad
			 * things.
			 */
			result = __get_inband_link_state(ipd_port);
		else
			return cvmx_helper_link_get(ipd_port);
	}
	return result;

}
#endif /* CVMX_BUILD_FOR_LINUX_KERNEL */

/**
 * @INTERNAL
 * This function invokes  __cvmx_helper_board_link_get_from_dt when device tree
 * info is available. When the device tree information is not available then
 * this function is the board specific method of determining an
 * ethernet ports link speed. Most Octeon boards have Marvell PHYs
 * and are handled by the fall through case. This function must be
 * updated for boards that don't have the normal Marvell PHYs.
 *
 * This function must be modified for every new Octeon board.
 * Internally it uses switch statements based on the cvmx_sysinfo
 * data to determine board types and revisions. It relies on the
 * fact that every Octeon board receives a unique board type
 * enumeration from the bootloader.
 *
 * @param ipd_port IPD input port associated with the port we want to get link
 *                 status for.
 *
 * @return The ports link status. If the link isn't fully resolved, this must
 *         return zero.
 */
cvmx_helper_link_info_t __cvmx_helper_board_link_get(int ipd_port)
{
	cvmx_helper_link_info_t result;
	int phy_addr;
	int is_broadcom_phy = 0;
	int is_vitesse_phy = 0;
	int is_cortina_phy = 0;

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	if (cvmx_sysinfo_get()->fdt_addr) {
		return __cvmx_helper_board_link_get_from_dt(ipd_port);
	}
#endif

	/* Give the user a chance to override the processing of this function */
	if (cvmx_override_board_link_get)
		return cvmx_override_board_link_get(ipd_port);

	/* Unless we fix it later, all links are defaulted to down */
	result.u64 = 0;

	/*
	 * This switch statement should handle all ports that either
	 * don't use Marvell PHYS, or don't support in-band status.
	 */
	switch (cvmx_sysinfo_get()->board_type) {
	case CVMX_BOARD_TYPE_SIM:
		/* The simulator gives you a simulated 1Gbps full duplex link */
		result.s.link_up = 1;
		result.s.full_duplex = 1;
		result.s.speed = 1000;
		return result;
	case CVMX_BOARD_TYPE_LANAI2_A:
	case CVMX_BOARD_TYPE_LANAI2_U:
	case CVMX_BOARD_TYPE_LANAI2_G:
		break;
	case CVMX_BOARD_TYPE_EBH3100:
	case CVMX_BOARD_TYPE_CN3010_EVB_HS5:
	case CVMX_BOARD_TYPE_CN3005_EVB_HS5:
	case CVMX_BOARD_TYPE_CN3020_EVB_HS5:
		/* Port 1 on these boards is always Gigabit */
		if (ipd_port == 1) {
			result.s.link_up = 1;
			result.s.full_duplex = 1;
			result.s.speed = 1000;
			return result;
		}
		/* Fall through to the generic code below */
		break;
	case CVMX_BOARD_TYPE_EBH5600:
	case CVMX_BOARD_TYPE_EBH5601:
	case CVMX_BOARD_TYPE_EBH5610:
		/* Board has 1 management ports */
		if (ipd_port == CVMX_HELPER_BOARD_MGMT_IPD_PORT)
			is_broadcom_phy = 1;
		break;
	case CVMX_BOARD_TYPE_EBH5200:
	case CVMX_BOARD_TYPE_EBH5201:
	case CVMX_BOARD_TYPE_EBT5200:
		/* Board has 2 management ports */
		if ((ipd_port >= CVMX_HELPER_BOARD_MGMT_IPD_PORT) &&
		    (ipd_port < (CVMX_HELPER_BOARD_MGMT_IPD_PORT + 2)))
			is_broadcom_phy = 1;
		break;
	case CVMX_BOARD_TYPE_EBB6100:
	case CVMX_BOARD_TYPE_EBB6300:	/* Only for MII mode, with PHY addresses 0/1. Default is RGMII */
	case CVMX_BOARD_TYPE_EBB6600:	/* Only for MII mode, with PHY addresses 0/1. Default is RGMII */
		if ((ipd_port >= CVMX_HELPER_BOARD_MGMT_IPD_PORT) &&
		    (ipd_port < (CVMX_HELPER_BOARD_MGMT_IPD_PORT + 2))) {
			phy_addr = cvmx_helper_board_get_mii_address(ipd_port);
			if (phy_addr >= 0 && phy_addr <= 1)
				is_broadcom_phy = 1;
		}
		break;
	case CVMX_BOARD_TYPE_EP6300C:
		is_broadcom_phy = 1;
		break;
	case CVMX_BOARD_TYPE_CUST_NB5:
		/* Port 1 on these boards is always Gigabit */
		if (ipd_port == 1) {
			result.s.link_up = 1;
			result.s.full_duplex = 1;
			result.s.speed = 1000;
			return result;
		} else		/* The other port uses a broadcom PHY */
			is_broadcom_phy = 1;
		break;
	case CVMX_BOARD_TYPE_BBGW_REF:
		/* Port 1 on these boards is always Gigabit */
		if (ipd_port == 2) {
			/* Port 2 is not hooked up */
			result.u64 = 0;
			return result;
		} else {
			/* Ports 0 and 1 connect to the switch */
			result.s.link_up = 1;
			result.s.full_duplex = 1;
			result.s.speed = 1000;
			return result;
		}
	case CVMX_BOARD_TYPE_NIC4E:
	case CVMX_BOARD_TYPE_NIC2E:
		is_broadcom_phy = 1;
		break;
	case CVMX_BOARD_TYPE_SNIC10E:
		is_vitesse_phy = 1;
		break;
	case CVMX_BOARD_TYPE_NIC10E_66:
		if (cvmx_sysinfo_get()->board_rev_major >= 3)
			is_vitesse_phy = 1;
		else
			is_broadcom_phy = 1;
		break;
	case CVMX_BOARD_TYPE_NIC68_4:
		is_cortina_phy = 1;
		break;
	}

	phy_addr = cvmx_helper_board_get_mii_address(ipd_port);
	if (phy_addr >= 0) {
		if (is_cortina_phy) {
			result = __cvmx_get_cortina_phy_link_state(phy_addr);
		} else if (is_broadcom_phy) {
			result = __get_broadcom_phy_link_state(phy_addr);
		} else if (is_vitesse_phy) {
			result = __get_vitesse_phy_link_state(phy_addr);
		} else {
			/* This code assumes we are using a Marvell Gigabit PHY. */
			result = __get_marvell_phy_link_state(phy_addr);
		}
	} else if (OCTEON_IS_MODEL(OCTEON_CN3XXX) ||
		   OCTEON_IS_MODEL(OCTEON_CN58XX) ||
		   OCTEON_IS_MODEL(OCTEON_CN50XX)) {
		/*
		 * We don't have a PHY address, so attempt to use
		 * in-band status. It is really important that boards
		 * not supporting in-band status never get
		 * here. Reading broken in-band status tends to do bad
		 * things.
		 */
		result = __get_inband_link_state(ipd_port);
	} else {
		/*
		 * We don't have a PHY address and we don't have
		 * in-band status. There is no way to determine the
		 * link speed. Return down assuming this port isn't
		 * wired.
		 */
		result.u64 = 0;
	}

	/* If link is down, return all fields as zero. */
	if (!result.s.link_up)
		result.u64 = 0;

	return result;
}

/**
 * This function as a board specific method of changing the PHY
 * speed, duplex, and auto-negotiation. This programs the PHY and
 * not Octeon. This can be used to force Octeon's links to
 * specific settings.
 *
 * @param phy_addr  The address of the PHY to program
 * @param link_flags
 *                  Flags to control auto-negotiation.  Bit 0 is auto-negotiation
 *                  enable/disable to maintain backward compatibility.
 * @param link_info Link speed to program. If the speed is zero and auto-negotiation
 *                  is enabled, all possible negotiation speeds are advertised.
 *
 * @return Zero on success, negative on failure
 */
int cvmx_helper_board_link_set_phy(int phy_addr, cvmx_helper_board_set_phy_link_flags_types_t link_flags, cvmx_helper_link_info_t link_info)
{

	/* Set the flow control settings based on link_flags */
	if ((link_flags & set_phy_link_flags_flow_control_mask) != set_phy_link_flags_flow_control_dont_touch) {
		cvmx_mdio_phy_reg_autoneg_adver_t reg_autoneg_adver;
		reg_autoneg_adver.u16 = cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff, CVMX_MDIO_PHY_REG_AUTONEG_ADVER);
		reg_autoneg_adver.s.asymmetric_pause = (link_flags & set_phy_link_flags_flow_control_mask) == set_phy_link_flags_flow_control_enable;
		reg_autoneg_adver.s.pause = (link_flags & set_phy_link_flags_flow_control_mask) == set_phy_link_flags_flow_control_enable;
		cvmx_mdio_write(phy_addr >> 8, phy_addr & 0xff, CVMX_MDIO_PHY_REG_AUTONEG_ADVER, reg_autoneg_adver.u16);
	}

	/* If speed isn't set and autoneg is on advertise all supported modes */
	if ((link_flags & set_phy_link_flags_autoneg) && (link_info.s.speed == 0)) {
		cvmx_mdio_phy_reg_control_t reg_control;
		cvmx_mdio_phy_reg_status_t reg_status;
		cvmx_mdio_phy_reg_autoneg_adver_t reg_autoneg_adver;
		cvmx_mdio_phy_reg_extended_status_t reg_extended_status;
		cvmx_mdio_phy_reg_control_1000_t reg_control_1000;

		reg_status.u16 = cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff, CVMX_MDIO_PHY_REG_STATUS);
		reg_autoneg_adver.u16 = cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff, CVMX_MDIO_PHY_REG_AUTONEG_ADVER);
		reg_autoneg_adver.s.advert_100base_t4 = reg_status.s.capable_100base_t4;
		reg_autoneg_adver.s.advert_10base_tx_full = reg_status.s.capable_10_full;
		reg_autoneg_adver.s.advert_10base_tx_half = reg_status.s.capable_10_half;
		reg_autoneg_adver.s.advert_100base_tx_full = reg_status.s.capable_100base_x_full;
		reg_autoneg_adver.s.advert_100base_tx_half = reg_status.s.capable_100base_x_half;
		cvmx_mdio_write(phy_addr >> 8, phy_addr & 0xff, CVMX_MDIO_PHY_REG_AUTONEG_ADVER, reg_autoneg_adver.u16);
		if (reg_status.s.capable_extended_status) {
			reg_extended_status.u16 = cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff, CVMX_MDIO_PHY_REG_EXTENDED_STATUS);
			reg_control_1000.u16 = cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff, CVMX_MDIO_PHY_REG_CONTROL_1000);
			reg_control_1000.s.advert_1000base_t_full = reg_extended_status.s.capable_1000base_t_full;
			reg_control_1000.s.advert_1000base_t_half = reg_extended_status.s.capable_1000base_t_half;
			cvmx_mdio_write(phy_addr >> 8, phy_addr & 0xff, CVMX_MDIO_PHY_REG_CONTROL_1000, reg_control_1000.u16);
		}
		reg_control.u16 = cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff, CVMX_MDIO_PHY_REG_CONTROL);
		reg_control.s.autoneg_enable = 1;
		reg_control.s.restart_autoneg = 1;
		cvmx_mdio_write(phy_addr >> 8, phy_addr & 0xff, CVMX_MDIO_PHY_REG_CONTROL, reg_control.u16);
	} else if ((link_flags & set_phy_link_flags_autoneg)) {
		cvmx_mdio_phy_reg_control_t reg_control;
		cvmx_mdio_phy_reg_status_t reg_status;
		cvmx_mdio_phy_reg_autoneg_adver_t reg_autoneg_adver;
		cvmx_mdio_phy_reg_control_1000_t reg_control_1000;

		reg_status.u16 = cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff, CVMX_MDIO_PHY_REG_STATUS);
		reg_autoneg_adver.u16 = cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff, CVMX_MDIO_PHY_REG_AUTONEG_ADVER);
		reg_autoneg_adver.s.advert_100base_t4 = 0;
		reg_autoneg_adver.s.advert_10base_tx_full = 0;
		reg_autoneg_adver.s.advert_10base_tx_half = 0;
		reg_autoneg_adver.s.advert_100base_tx_full = 0;
		reg_autoneg_adver.s.advert_100base_tx_half = 0;
		if (reg_status.s.capable_extended_status) {
			reg_control_1000.u16 = cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff, CVMX_MDIO_PHY_REG_CONTROL_1000);
			reg_control_1000.s.advert_1000base_t_full = 0;
			reg_control_1000.s.advert_1000base_t_half = 0;
		}
		switch (link_info.s.speed) {
		case 10:
			reg_autoneg_adver.s.advert_10base_tx_full = link_info.s.full_duplex;
			reg_autoneg_adver.s.advert_10base_tx_half = !link_info.s.full_duplex;
			break;
		case 100:
			reg_autoneg_adver.s.advert_100base_tx_full = link_info.s.full_duplex;
			reg_autoneg_adver.s.advert_100base_tx_half = !link_info.s.full_duplex;
			break;
		case 1000:
			reg_control_1000.s.advert_1000base_t_full = link_info.s.full_duplex;
			reg_control_1000.s.advert_1000base_t_half = !link_info.s.full_duplex;
			break;
		}
		cvmx_mdio_write(phy_addr >> 8, phy_addr & 0xff, CVMX_MDIO_PHY_REG_AUTONEG_ADVER, reg_autoneg_adver.u16);
		if (reg_status.s.capable_extended_status)
			cvmx_mdio_write(phy_addr >> 8, phy_addr & 0xff, CVMX_MDIO_PHY_REG_CONTROL_1000, reg_control_1000.u16);
		reg_control.u16 = cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff, CVMX_MDIO_PHY_REG_CONTROL);
		reg_control.s.autoneg_enable = 1;
		reg_control.s.restart_autoneg = 1;
		cvmx_mdio_write(phy_addr >> 8, phy_addr & 0xff, CVMX_MDIO_PHY_REG_CONTROL, reg_control.u16);
	} else {
		cvmx_mdio_phy_reg_control_t reg_control;
		reg_control.u16 = cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff, CVMX_MDIO_PHY_REG_CONTROL);
		reg_control.s.autoneg_enable = 0;
		reg_control.s.reset = 1;
		reg_control.s.restart_autoneg = 1;
		reg_control.s.duplex = link_info.s.full_duplex;
		if (link_info.s.speed == 1000) {
			reg_control.s.speed_msb = 1;
			reg_control.s.speed_lsb = 0;
		} else if (link_info.s.speed == 100) {
			reg_control.s.speed_msb = 0;
			reg_control.s.speed_lsb = 1;
		} else if (link_info.s.speed == 10) {
			reg_control.s.speed_msb = 0;
			reg_control.s.speed_lsb = 0;
		}
		cvmx_mdio_write(phy_addr >> 8, phy_addr & 0xff, CVMX_MDIO_PHY_REG_CONTROL, reg_control.u16);
	}
	return 0;
}

/**
 * @INTERNAL
 * This function is called by cvmx_helper_interface_probe() after it
 * determines the number of ports Octeon can support on a specific
 * interface. This function is the per board location to override
 * this value. It is called with the number of ports Octeon might
 * support and should return the number of actual ports on the
 * board.
 *
 * This function must be modified for every new Octeon board.
 * Internally it uses switch statements based on the cvmx_sysinfo
 * data to determine board types and revisions. It relies on the
 * fact that every Octeon board receives a unique board type
 * enumeration from the bootloader.
 *
 * @param interface Interface to probe
 * @param supported_ports
 *                  Number of ports Octeon supports.
 *
 * @return Number of ports the actual board supports. Many times this will
 *         simple be "support_ports".
 */
int __cvmx_helper_board_interface_probe(int interface, int supported_ports)
{
	switch (cvmx_sysinfo_get()->board_type) {
	case CVMX_BOARD_TYPE_CN3005_EVB_HS5:
	case CVMX_BOARD_TYPE_LANAI2_A:
	case CVMX_BOARD_TYPE_LANAI2_U:
	case CVMX_BOARD_TYPE_LANAI2_G:
		if (interface == 0)
			return 2;
		break;
	case CVMX_BOARD_TYPE_BBGW_REF:
		if (interface == 0)
			return 2;
		break;
	case CVMX_BOARD_TYPE_NIC_XLE_4G:
		if (interface == 0)
			return 0;
		break;
		/*
		 * The 2nd interface on the EBH5600 is connected to
		 * the Marvel switch, which we don't support. Disable
		 * ports connected to it.
		 */
	case CVMX_BOARD_TYPE_EBH5600:
		if (interface == 1)
			return 0;
		break;
	case CVMX_BOARD_TYPE_EBB5600:
		if (cvmx_helper_interface_get_mode(interface) == CVMX_HELPER_INTERFACE_MODE_PICMG)
			return 0;
		break;
	case CVMX_BOARD_TYPE_EBT5810:
		/*
		 * Two ports on each SPI: 1 hooked to MAC, 1 loopback
		 * Loopback disabled by default.
		 */
		return 1;
	case CVMX_BOARD_TYPE_NIC2E:
		if (interface == 0)
			return 2;
	}
#ifdef CVMX_BUILD_FOR_UBOOT
	if (CVMX_HELPER_INTERFACE_MODE_SPI ==
	    cvmx_helper_interface_get_mode(interface) && getenv("disable_spi"))
		return 0;
#endif
	return supported_ports;
}

/**
 * @INTERNAL
 * Enable packet input/output from the hardware. This function is
 * called after by cvmx_helper_packet_hardware_enable() to
 * perform board specific initialization. For most boards
 * nothing is needed.
 *
 * @param interface Interface to enable
 *
 * @return Zero on success, negative on failure
 */
int __cvmx_helper_board_hardware_enable(int interface)
{
	if (cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_CN3005_EVB_HS5) {
		if (interface == 0) {
			/* Different config for switch port */
			cvmx_write_csr(CVMX_ASXX_TX_CLK_SETX(1, interface), 0);
			cvmx_write_csr(CVMX_ASXX_RX_CLK_SETX(1, interface), 0);
			/*
			 * Boards with gigabit WAN ports need a
			 * different setting that is compatible with
			 * 100 Mbit settings.
			 */
			cvmx_write_csr(CVMX_ASXX_TX_CLK_SETX(0, interface), 0xc);
			cvmx_write_csr(CVMX_ASXX_RX_CLK_SETX(0, interface), 0xc);
		}
	} else if (cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_LANAI2_U) {
		if (interface == 0) {
			cvmx_write_csr(CVMX_ASXX_TX_CLK_SETX(0, interface), 16);
			cvmx_write_csr(CVMX_ASXX_RX_CLK_SETX(0, interface), 16);
		}
	} else if (cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_CN3010_EVB_HS5) {
		/* Broadcom PHYs require different ASX clocks. Unfortunately
		   many customer don't define a new board Id and simply
		   mangle the CN3010_EVB_HS5 */
		if (interface == 0) {
			/* Some customers boards use a hacked up bootloader that identifies them as
			 ** CN3010_EVB_HS5 evaluation boards.  This leads to all kinds of configuration
			 ** problems.  Detect one case, and print warning, while trying to do the right thing.
			 */
			int phy_addr = cvmx_helper_board_get_mii_address(0);
			if (phy_addr != -1) {
				int phy_identifier = cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff, 0x2);
				/* Is it a Broadcom PHY? */
				if (phy_identifier == 0x0143) {
					cvmx_dprintf("\n");
					cvmx_dprintf("ERROR:\n");
					cvmx_dprintf("ERROR: Board type is CVMX_BOARD_TYPE_CN3010_EVB_HS5, but Broadcom PHY found.\n");
					cvmx_dprintf("ERROR: The board type is mis-configured, and software malfunctions are likely.\n");
					cvmx_dprintf("ERROR: All boards require a unique board type to identify them.\n");
					cvmx_dprintf("ERROR:\n");
					cvmx_dprintf("\n");
					cvmx_wait(1000000000);
					cvmx_write_csr(CVMX_ASXX_RX_CLK_SETX(0, interface), 5);
					cvmx_write_csr(CVMX_ASXX_TX_CLK_SETX(0, interface), 5);
				}
			}
		}
	} else if (cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_UBNT_E100) {
		cvmx_write_csr(CVMX_ASXX_RX_CLK_SETX(0, interface), 0);
		cvmx_write_csr(CVMX_ASXX_TX_CLK_SETX(0, interface), 0x10);
		cvmx_write_csr(CVMX_ASXX_RX_CLK_SETX(1, interface), 0);
		cvmx_write_csr(CVMX_ASXX_TX_CLK_SETX(1, interface), 0x10);
		cvmx_write_csr(CVMX_ASXX_RX_CLK_SETX(2, interface), 0);
		cvmx_write_csr(CVMX_ASXX_TX_CLK_SETX(2, interface), 0x10);
	}
	return 0;
}


/**
 * @INTERNAL
 * Gets the clock type used for the USB block based on board type.
 * Used by the USB code for auto configuration of clock type.
 *
 * @return USB clock type enumeration
 */
cvmx_helper_board_usb_clock_types_t __cvmx_helper_board_usb_get_clock_type(void)
{
#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	const void *fdt_addr = CASTPTR(const void *, cvmx_sysinfo_get()->fdt_addr);
	int nodeoffset;
	const void *nodep;
	int len;
	uint32_t speed = 0;
	const char *type = NULL;

	if (fdt_addr) {
		nodeoffset = fdt_path_offset(fdt_addr, "/soc/uctl");
		if (nodeoffset < 0)
			nodeoffset = fdt_path_offset(fdt_addr, "/soc/usbn");

		if (nodeoffset >= 0) {
			nodep = fdt_getprop(fdt_addr, nodeoffset, "refclk-type", &len);
			if (nodep != NULL && len > 0)
				type = (const char *)nodep;
			else
				type = "unknown";
			nodep = fdt_getprop(fdt_addr, nodeoffset, "refclk-frequency", &len);
			if (nodep != NULL && len == sizeof(uint32_t))
				speed = fdt32_to_cpu(*(int *)nodep);
			else
				speed = 0;
			if (!strcmp(type, "crystal")) {
				if (speed == 0 || speed == 12000000)
					return USB_CLOCK_TYPE_CRYSTAL_12;
				else
					printf("Warning: invalid crystal speed for USB clock type in FDT\n");
			} else if (!strcmp(type, "external")) {
				switch (speed) {
				case 12000000:
					return USB_CLOCK_TYPE_REF_12;
				case 24000000:
					return USB_CLOCK_TYPE_REF_24;
				case 0:
				case 48000000:
					return USB_CLOCK_TYPE_REF_48;
				default:
					printf("Warning: invalid USB clock speed of %u hz in FDT\n", (unsigned int)speed);
				}
			} else
				printf("Warning: invalid USB reference clock type \"%s\" in FDT\n", type ? type : "NULL");
		}
	}
#endif
	switch (cvmx_sysinfo_get()->board_type) {
	case CVMX_BOARD_TYPE_BBGW_REF:
	case CVMX_BOARD_TYPE_LANAI2_A:
	case CVMX_BOARD_TYPE_LANAI2_U:
	case CVMX_BOARD_TYPE_LANAI2_G:
	case CVMX_BOARD_TYPE_NIC10E_66:
	case CVMX_BOARD_TYPE_UBNT_E100:
		return USB_CLOCK_TYPE_CRYSTAL_12;
	case CVMX_BOARD_TYPE_NIC10E:
		return USB_CLOCK_TYPE_REF_12;
	default:
		break;
	}
	/* Most boards except NIC10e use a 12MHz crystal */
	if (OCTEON_IS_OCTEON2())
		return USB_CLOCK_TYPE_CRYSTAL_12;
	return USB_CLOCK_TYPE_REF_48;
}

/**
 * @INTERNAL
 * Adjusts the number of available USB ports on Octeon based on board
 * specifics.
 *
 * @param supported_ports expected number of ports based on chip type;
 *
 *
 * @return number of available usb ports, based on board specifics.
 *         Return value is supported_ports if function does not
 *         override.
 */
int __cvmx_helper_board_usb_get_num_ports(int supported_ports)
{
	switch (cvmx_sysinfo_get()->board_type) {
	case CVMX_BOARD_TYPE_NIC_XLE_4G:
	case CVMX_BOARD_TYPE_NIC2E:
	case CVMX_BOARD_TYPE_SNIC10E:
		return 0;
	case CVMX_BOARD_TYPE_NIC10E_66:
		if (cvmx_sysinfo_get()->board_rev_major >= 3)
			return 0;
	}

	return supported_ports;
}
