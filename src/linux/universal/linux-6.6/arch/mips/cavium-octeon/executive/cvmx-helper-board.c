/***********************license start***************
 * Copyright (c) 2003-2014  Cavium Inc. (support@cavium.com). All rights
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
# include <linux/module.h>
# include <asm/octeon/cvmx.h>
# include <asm/octeon/cvmx-app-init.h>
# include <asm/octeon/cvmx-smix-defs.h>
# include <asm/octeon/cvmx-gmxx-defs.h>
# include <asm/octeon/cvmx-asxx-defs.h>
# include <asm/octeon/cvmx-mdio.h>
# include <asm/octeon/cvmx-helper.h>
# include <asm/octeon/cvmx-helper-util.h>
# include <asm/octeon/cvmx-helper-board.h>
# include <asm/octeon/cvmx-helper-cfg.h>
# include <asm/octeon/cvmx-twsi.h>
#elif !defined(__U_BOOT__)
# include "cvmx.h"
# include "cvmx-app-init.h"
# include "cvmx-sysinfo.h"
# include "cvmx-twsi.h"
# include "cvmx-mdio.h"
# include "cvmx-helper.h"
# include "cvmx-helper-util.h"
# include "cvmx-helper-board.h"
# include "cvmx-helper-cfg.h"
# include "libfdt/cvmx-helper-fdt.h"
# include "cvmx-gpio.h"
# include "cvmx-qlm.h"
# include "octeon_mem_map.h"
# include "cvmx-bootmem.h"
#else
# include <common.h>
# include <malloc.h>
# include <i2c.h>
# include <asm/arch/cvmx.h>
# include <asm/arch/cvmx-sysinfo.h>
# include <asm/arch/cvmx-twsi.h>
# include <asm/arch/cvmx-mdio.h>
# include <asm/arch/cvmx-helper.h>
# include <asm/arch/cvmx-helper-util.h>
# include <asm/arch/cvmx-helper-board.h>
# include <asm/arch/cvmx-helper-cfg.h>
# include <asm/arch/cvmx-helper-fdt.h>
# include <asm/arch/cvmx-gpio.h>
# include <asm/arch/cvmx-qlm.h>
# include <asm/arch/octeon_mem_map.h>
# include <asm/arch/cvmx-bootmem.h>
#endif

/**
 * cvmx_override_board_link_get(int ipd_port) is a function
 * pointer. It is meant to allow customization of the process of
 * talking to a PHY to determine link speed. It is called every
 * time a PHY must be polled for link status. Users should set
 * this pointer to a function before calling any cvmx-helper
 * operations.
 */
CVMX_SHARED cvmx_helper_link_info_t(*cvmx_override_board_link_get)(int ipd_port) = NULL;

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
/** Set this to 1 to enable lots of debugging output */
static const int device_tree_dbg = 0;
#endif

/**
 * @INTERNAL
 * Get link state of Cortina PHY
 *
 * @param phy_info PHY information
 *
 * @return link state information from PHY
 */
static cvmx_helper_link_info_t
__cvmx_get_cortina_phy_link_state(cvmx_phy_info_t *phy_info);

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
/**
 * @INTERNAL
 * Get link state of generic C22 compliant PHYs
 */
static cvmx_helper_link_info_t
__cvmx_get_generic_8023_c22_phy_link_state(cvmx_phy_info_t *phy_info);

/**
 * @INTERNAL
 * Get link state of generic C45 compliant PHYs
 */
static cvmx_helper_link_info_t
__get_generic_8023_c45_phy_link_state(cvmx_phy_info_t *phy_info);

/**
 * @INTERNAL
 * Get link state of marvell PHY
 */
static cvmx_helper_link_info_t
__get_marvell_phy_link_state(cvmx_phy_info_t *phy_info);

/**
 * @INTERNAL
 * Get link state of Aquantia PHY
 */
static cvmx_helper_link_info_t
__get_aquantia_phy_link_state(cvmx_phy_info_t *phy_info);

/**
 * @INTERNAL
 * Get link state of the Vitesse VSC8490 PHY
 */
static cvmx_helper_link_info_t
__get_vitesse_vsc8490_phy_link_state(cvmx_phy_info_t *phy_info);
#endif

/**
 * @INTERNAL
 * Get link state of broadcom PHY
 *
 * @param phy_info	PHY information
 */
static cvmx_helper_link_info_t
__get_broadcom_phy_link_state(cvmx_phy_info_t *phy_info);

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
/**
 * Return the MII PHY address associated with the given IPD
 * port. The phy address is obtained from the device tree.
 *
 * @param[out] phy_info - PHY information data structure updated
 * @param ipd_port Octeon IPD port to get the MII address for.
 *
 * @return MII PHY address and bus number, -1 on error, -2 if PHY info missing (OK).
 */
int __get_phy_info_from_dt(cvmx_phy_info_t *phy_info, int ipd_port);

/**
 * @INTERNAL
 * Get link state of generic gigabit PHY
 *
 * @param phy_info - information about the PHY
 *
 * @returns link status of the PHY
 */
static cvmx_helper_link_info_t
__cvmx_get_generic_8023_c22_phy_link_state(cvmx_phy_info_t *phy_info);

static cvmx_helper_link_info_t
__cvmx_get_qualcomm_s17_phy_link_state(cvmx_phy_info_t *phy_info);

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
 * @param     aliases    - offset of alias in device tree
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
	const int dbg = device_tree_dbg;

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
		cvmx_dprintf("ipdd_port=%d pip_path=%s pip=%d ",
			     ipd_port, pip_path, pip);
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
static int __switch_mdio_mux(const cvmx_phy_info_t *phy_info);

static int __mdiobus_addr_to_unit(uint32_t addr)
{
	int unit = (addr >> 7) & 3;
	if (!OCTEON_IS_MODEL(OCTEON_CN68XX) && !OCTEON_IS_MODEL(OCTEON_CN78XX))
		unit >>= 1;
	return unit;
}

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
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
	int phandle;
	int smi_offset;
	int gpio_offset;
	uint64_t smi_addr = 0;
	int len;
	uint32_t *pgpio_handle;
	int gpio_count = 0;
	uint32_t *prop_val;
	int offset;
	const char *prop_name;

	if (device_tree_dbg)
		cvmx_dprintf("%s(%p, 0x%x, 0x%x)\n", __func__, phy_info,
			     mdio_offset, mux_offset);
	if (fdt_addr == 0)
		fdt_addr = __cvmx_phys_addr_to_ptr(cvmx_sysinfo_get()->fdt_addr,
						   OCTEON_FDT_MAX_SIZE);

	/* Get register value to put onto the GPIO lines to select */
	phy_info->gpio_value = cvmx_fdt_get_int(fdt_addr, mdio_offset, "reg", -1);
	if (phy_info->gpio_value < 0) {
		cvmx_dprintf("Could not get register value for muxed MDIO bus from DT\n");
		return -1;
	}

	smi_offset = cvmx_fdt_lookup_phandle(fdt_addr, mux_offset,
					   "mdio-parent-bus");
	if (smi_offset < 0) {
		cvmx_dprintf("Invalid SMI offset for muxed MDIO interface in device tree\n");
		return -1;
	}
	smi_addr = cvmx_fdt_get_uint64(fdt_addr, smi_offset, "reg", 0);

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
#endif

/**
 * @INTERNAL
 * Converts a BGX address to the node, interface and port number
 *
 * @param bgx_addr	Address of CSR register
 *
 * @return node, interface and port number, will be -1 for invalid address.
 */
static struct cvmx_xiface
__cvmx_bgx_reg_addr_to_xiface(uint64_t bgx_addr)
{
	struct cvmx_xiface xi = {-1, -1};

	xi.node = cvmx_csr_addr_to_node(bgx_addr);
	bgx_addr = cvmx_csr_addr_strip_node(bgx_addr);
	if ((bgx_addr & 0xFFFFFFFFF0000000) != 0x00011800E0000000) {
		cvmx_dprintf("%s: Invalid BGX address 0x%llx\n",
			     __func__, (unsigned long long)bgx_addr);
		xi.node = -1;
		return xi;
	}
	xi.interface = (bgx_addr >> 24) & 0x0F;

	return xi;
}

static void __cvmx_mdio_addr_to_node_bus(uint64_t addr, int *node, int *bus)
{
	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		if (node)
			*node = cvmx_csr_addr_to_node(addr);
		addr = cvmx_csr_addr_strip_node(addr);
	} else {
		if (node)
			*node = 0;
	}
	if (OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		switch (addr) {
		case 0x0001180000003800:
			*bus = 0;
			break;
		case 0x0001180000003880:
			*bus = 1;
			break;
		case 0x0001180000003900:
			*bus = 2;
			break;
		case 0x0001180000003980:
			*bus = 3;
			break;
		default:
			*bus = -1;
			cvmx_printf("%s: Invalid SMI bus address 0x%llx\n",
				    __func__, (unsigned long long) addr);
			break;
		}
	} else if (OCTEON_IS_MODEL(OCTEON_CN73XX)
		   || OCTEON_IS_MODEL(OCTEON_CNF75XX)) {
		switch (addr) {
		case 0x0001180000003800:
			*bus = 0;
			break;
		case 0x0001180000003880:
			*bus = 1;
			break;
		default:
			*bus = -1;
			cvmx_printf("%s: Invalid SMI bus address 0x%llx\n",
				    __func__, (unsigned long long) addr);
			break;
		}
	} else {
		switch (addr) {
		case 0x0001180000001800:
			*bus = 0;
			break;
		case 0x0001180000001900:
			*bus = 1;
			break;
		default:
			*bus = -1;
			cvmx_printf("%s: Invalid SMI bus address 0x%llx\n",
				    __func__, (unsigned long long) addr);
			break;
		}
	}
}

/**
 * Writes to a Microsemi VSC7224 16-bit register
 *
 * @param[in]	i2c_bus	i2c bus data structure (must be enabled)
 * @param	addr	Address of VSC7224 on the i2c bus
 * @param	reg	8-bit register number to write to
 * @param	val	16-bit value to write
 *
 * @return	0 for success
 */
static int cvmx_write_vsc7224_reg(const struct cvmx_fdt_i2c_bus_info *i2c_bus,
				  uint8_t addr, uint8_t reg, uint16_t val)
{
	int bus = cvmx_fdt_i2c_get_root_bus(i2c_bus);
#ifdef __U_BOOT__
	uint8_t buffer[2];

	i2c_set_bus_num(bus);
	buffer[0] = val >> 8;
	buffer[1] = val & 0xff;
	i2c_write(addr, reg, 1, buffer, 2);
#else
	cvmx_twsix_write_ia(bus, addr, reg, 2, 1, (uint64_t)val);
#endif
	return 0;
}

/**
 * Writes to a Microsemi VSC7224 16-bit register
 *
 * @param[in]	i2c_bus	i2c bus data structure (must be enabled)
 * @param	addr	Address of VSC7224 on the i2c bus
 * @param	reg	8-bit register number to write to
 *
 * @return	16-bit value or error if < 0
 */
static int cvmx_read_vsc7224_reg(const struct cvmx_fdt_i2c_bus_info *i2c_bus,
				 uint8_t addr, uint8_t reg)
{
	int bus = cvmx_fdt_i2c_get_root_bus(i2c_bus);
#ifdef __U_BOOT__
	uint8_t buffer[2];

	i2c_set_bus_num(bus);
	if (i2c_read(addr, reg, 1, buffer, 2))
		return -1;
	else
		return (buffer[0] << 8) | buffer[1];
#else
	uint64_t data;

	if (cvmx_twsix_read_ia(bus, addr, reg, 2, 1, &data) < 0)
		return -1;
	else
		return data & 0xffff;
#endif
	return 0;
}

/**
 * @INTERNAL
 * Return loss of signal
 *
 * @param	xiface	xinterface number
 * @param	index	port index on interface
 *
 * @return	0 if signal present, 1 if loss of signal.
 *
 * @NOTE:	A result of 0 is possible in some cases where the signal is
 *		not present.
 *
 * This is for use with __cvmx_qlm_rx_equilization
 */
int __cvmx_helper_get_los(int xiface, int index)
{
	struct cvmx_fdt_sfp_info *sfp;
	struct cvmx_vsc7224_chan *vsc7224_chan;
	struct cvmx_vsc7224 *vsc7224;
	int los = 0;
	int val;

	sfp = cvmx_helper_cfg_get_sfp_info(xiface, index);

	/* Check all SFP slots in the group
	 * NOTE: Usually there is only one SFP or QSFP slot except in the case
	 *	 where multiple SFP+ slots are grouped together for XLAUI mode.
	 */
	while (sfp && sfp->check_mod_abs) {
		los = sfp->check_mod_abs(sfp, sfp->mod_abs_data);
		if (los || los < 0) {
			if (device_tree_dbg)
				cvmx_dprintf("%s(0x%x, %d): los detected (mod_abs) los: %d\n",
					     __func__, xiface, index, los);
			return 1;
		}
		vsc7224_chan = sfp->vsc7224_chan;
		while (vsc7224_chan) {
			uint64_t done;
			int channel_num = vsc7224_chan->lane;
			int los_bit = 1 << channel_num;
			int lol_bit = 0x10 << channel_num;

			/* We only care about receive channels so skip rx.
			 * Also, in XFI mode we don't care about different
			 * XFI ports so skip those.
			 */
			if (vsc7224_chan->is_tx ||
			    vsc7224_chan->index != index ||
			    vsc7224_chan->xiface != xiface) {
				vsc7224_chan = vsc7224_chan->next;
				continue;
			}

			vsc7224 = vsc7224_chan->vsc7224;
			/* Poll for LoS/LoL for 2ms */
			cvmx_fdt_enable_i2c_bus(vsc7224->i2c_bus, true);
			cvmx_write_vsc7224_reg(vsc7224->i2c_bus,
					       vsc7224->i2c_addr, 0x7f, 0x40);
			done = cvmx_clock_get_count(CVMX_CLOCK_CORE) +
				2000 * cvmx_clock_get_rate(CVMX_CLOCK_CORE) / 1000000;
			do {
				val = cvmx_read_vsc7224_reg(vsc7224->i2c_bus,
							    vsc7224->i2c_addr,
							    0xc0);
				val &= (los_bit | lol_bit);
				if (val) {
					if (device_tree_dbg)
						cvmx_dprintf("%s(0x%x, %d): LOS/LOL detected from VSC7224: 0x%x\n",
							     __func__, xiface,
							     index, val);
					return 1;
				}
			} while (cvmx_clock_get_count(CVMX_CLOCK_CORE) < done);
			/* Move to the next channel */
			vsc7224_chan = vsc7224_chan->next;
		}
		/* Move to the next SFP+ slot */
		sfp = sfp->next;
	}
	if (device_tree_dbg)
		cvmx_dprintf("%s(0x%x, %d): los: 0\n", __func__, xiface, index);
	return 0;
}

/**
 * Function called whenever mod_abs/mod_prs has changed for Microsemi VSC7224
 *
 * @param	sfp	pointer to SFP data structure
 * @param	val	1 if absent, 0 if present, otherwise not set
 * @param	data	user-defined data
 *
 * @return	0 for success, -1 on error
 */
int cvmx_sfp_vsc7224_mod_abs_changed(struct cvmx_fdt_sfp_info *sfp, int val,
				     void *data)
{
	int err;
	struct cvmx_sfp_mod_info *mod_info;
	int length;
	struct cvmx_vsc7224 *vsc7224;
	struct cvmx_vsc7224_chan *vsc7224_chan;
	struct cvmx_vsc7224_tap *taps, *match = NULL;
	const int dbg = device_tree_dbg;
	int i;

	if (dbg)
		cvmx_dprintf("%s(%s, %d, %p): Module %s\n", __func__,
			     sfp->name, val, data, val ? "absent" : "present");
	if (val)
		return 0;

	/* We're here if we detect that the module is now present */
	err = cvmx_sfp_read_i2c_eeprom(sfp);
	if (err) {
		cvmx_dprintf("%s: Error reading the SFP module eeprom for %s\n",
			     __func__, sfp->name);
		return err;
	}
	mod_info = &sfp->sfp_info;

	if (!mod_info->valid || !sfp->valid) {
		if (dbg)
			cvmx_dprintf("%s: Module data is invalid\n", __func__);
		return -1;
	}

	vsc7224_chan = sfp->vsc7224_chan;
	while (vsc7224_chan) {
		/* We don't do any rx tuning */
		if (!vsc7224_chan->is_tx) {
			vsc7224_chan = vsc7224_chan->next;
			continue;
		}

		/* Walk through all the channels */
		taps = vsc7224_chan->taps;
		if (mod_info->limiting)
			length = 0;
		else
			length = mod_info->max_copper_cable_len;
		if (dbg)
			cvmx_dprintf("%s: limiting: %d, length: %d\n", __func__,
				     mod_info->limiting, length);

		/* Find a matching length in the taps table */
		for (i = 0; i < vsc7224_chan->num_taps; i++) {
			if (length >= taps->len)
				match = taps;
			taps++;
		}
		if (!match) {
			cvmx_dprintf("%s(%s, %d, %p): Error: no matching tap for length %d\n",
				     __func__, sfp->name, val, data, length);
			return -1;
		}
		if (dbg)
			cvmx_dprintf("%s(%s): Applying %cx taps to vsc7224 %s:%d for cable length %d+\n",
				     __func__, sfp->name,
				     vsc7224_chan->is_tx ? 't' : 'r',
				     vsc7224_chan->vsc7224->name,
				     vsc7224_chan->lane,
				     match->len);
		/* Program the taps */
		vsc7224 = vsc7224_chan->vsc7224;
		cvmx_fdt_enable_i2c_bus(vsc7224->i2c_bus, true);
		cvmx_write_vsc7224_reg(vsc7224->i2c_bus, vsc7224->i2c_addr,
				       0x7f, vsc7224_chan->lane);
		if (!vsc7224_chan->maintap_disable)
			cvmx_write_vsc7224_reg(vsc7224->i2c_bus,
					       vsc7224->i2c_addr,
					       0x99, match->main_tap);
		if (!vsc7224_chan->pretap_disable)
			cvmx_write_vsc7224_reg(vsc7224->i2c_bus,
					       vsc7224->i2c_addr,
					       0x9a, match->pre_tap);
		if (!vsc7224_chan->posttap_disable)
			cvmx_write_vsc7224_reg(vsc7224->i2c_bus,
					       vsc7224->i2c_addr,
					       0x9b, match->post_tap);

		/* Re-use val and disable taps if needed */
		if (vsc7224_chan->maintap_disable ||
		    vsc7224_chan->pretap_disable  ||
		    vsc7224_chan->posttap_disable) {
			val = cvmx_read_vsc7224_reg(vsc7224->i2c_bus,
						    vsc7224->i2c_addr, 0x97);
			if (vsc7224_chan->maintap_disable)
				val |= 0x800;
			if (vsc7224_chan->pretap_disable)
				val |= 0x1000;
			if (vsc7224_chan->posttap_disable)
				val |= 0x400;
			cvmx_write_vsc7224_reg(vsc7224->i2c_bus,
					       vsc7224->i2c_addr, 0x97, val);
		}
		cvmx_fdt_enable_i2c_bus(vsc7224->i2c_bus, false);
		vsc7224_chan = vsc7224_chan->next;
	}

	return err;
}
#define CS4224_PP_LINE_SDS_COMMON_STX0_TX_OUTPUT_CTRLA	0x108F
#define CS4224_PP_LINE_SDS_COMMON_STX0_TX_OUTPUT_CTRLB	0x1090
#define CS4224_PP_LINE_SDS_DSP_MSEQ_SPARE22_LSB		0x12AC
#define CS4224_PP_LINE_SDS_DSP_MSEQ_SPARE22_MSB		0x12AD
#define CS4224_PP_LINE_SDS_DSP_MSEQ_SPARE24_LSB		0x12B0
#define CS4224_PP_HOST_SDS_DSP_MSEQ_SPARE22_LSB		0x1AAC
#define CS4224_PP_HOST_SDS_DSP_MSEQ_SPARE22_MSB		0x1AAD
#define CS4224_PP_HOST_SDS_DSP_MSEQ_SPARE24_LSB		0x1AB0

/**
 * Changes the mode the CS4343 operates in as well as the equalization
 *
 * @param[in]	phy_info	Pointer to phy data structure
 * @param	reg		slice number of phy
 * @param[in]	sfp		pointer to sfp information
 *
 * @return	0 for success, otherwise error
 */
static int cvmx_cs4343_set_slice_mode(const struct cvmx_phy_info *phy_info,
				      int reg,
				      const struct cvmx_fdt_sfp_info *sfp)
{
	const struct cvmx_sfp_mod_info *mod_info = &sfp->sfp_info;
	struct cvmx_cs4343_slice_info *slice = &phy_info->cs4343_info->slice[reg];
	int offset = slice->reg_offset;
	int err;
	uint32_t phy_addr = phy_info->phy_addr;
	int val;

	if (mod_info->rate == CVMX_SFP_RATE_1G) {	/* 1000Base-X */
		if (device_tree_dbg)
			cvmx_dprintf("%s: Setting slice %d to 1000Base-x mode for SFP slot %s\n",
				     __func__, reg, sfp->name);
		err = 0;
		err |= cvmx_mdio_45_write(phy_addr >> 8, phy_addr & 0xff, 0,
					  offset + CS4224_PP_LINE_SDS_DSP_MSEQ_SPARE22_MSB,
					  0);
		err |= cvmx_mdio_45_write(phy_addr >> 8, phy_addr & 0xff, 0,
					  offset + CS4224_PP_LINE_SDS_DSP_MSEQ_SPARE24_LSB,
					  9);
		cvmx_wait_usec(10000);
		err |= cvmx_mdio_45_write(phy_addr >> 8, phy_addr & 0xff, 0,
					  offset + CS4224_PP_HOST_SDS_DSP_MSEQ_SPARE22_MSB,
					  0);
		err |= cvmx_mdio_45_write(phy_addr >> 8, phy_addr & 0xff, 0,
					  offset + CS4224_PP_HOST_SDS_DSP_MSEQ_SPARE24_LSB,
					  9);
		cvmx_wait_usec(10000);
		err |= cvmx_mdio_45_write(phy_addr >> 8, phy_addr & 0xff, 0,
					  offset + CS4224_PP_LINE_SDS_DSP_MSEQ_SPARE22_MSB,
					  0x8000);
		val = cvmx_mdio_45_read(phy_addr >> 8, phy_addr & 0xff, 0,
					offset + CS4224_PP_LINE_SDS_DSP_MSEQ_SPARE22_LSB);
		err |= cvmx_mdio_45_write(phy_addr >> 8, phy_addr & 0xff, 0,
					  offset + CS4224_PP_LINE_SDS_DSP_MSEQ_SPARE24_LSB,
					  val);
		cvmx_wait_usec(10000);
		err |= cvmx_mdio_45_write(phy_addr >> 8, phy_addr & 0xff, 0,
					  offset + CS4224_PP_HOST_SDS_DSP_MSEQ_SPARE22_MSB,
					  0x8000);
		val = cvmx_mdio_45_read(phy_addr >> 8, phy_addr & 0xff, 0,
					offset + CS4224_PP_HOST_SDS_DSP_MSEQ_SPARE22_LSB);
		err |= cvmx_mdio_45_write(phy_addr >> 8, phy_addr & 0xff, 0,
					  offset + CS4224_PP_HOST_SDS_DSP_MSEQ_SPARE24_LSB,
					  val);
		cvmx_wait_usec(10000);
		val = ((slice->basex_stx_cmode_res & 7) << 12)     |
		      ((slice->basex_stx_drv_lower_cm & 0xf) << 8) |
		      (slice->basex_stx_level & 0x3f);
		err |= cvmx_mdio_45_write(phy_addr >> 8, phy_addr & 0xff, 0,
					  offset + CS4224_PP_LINE_SDS_COMMON_STX0_TX_OUTPUT_CTRLA,
					  val);
		val = cvmx_mdio_45_read(phy_addr >> 8, phy_addr & 0xff, 0,
					offset + CS4224_PP_LINE_SDS_COMMON_STX0_TX_OUTPUT_CTRLB);
		val &= 0xC000;
		val |= (slice->basex_stx_pre_peak << 8) & 0x1f;
		val |= (slice->basex_stx_muxsubrate_sel << 7) & 0x80;
		val |= slice->basex_stx_post_peak & 0x3f;
		cvmx_mdio_45_write(phy_addr >> 8, phy_addr & 0xff, 0,
				   offset + CS4224_PP_LINE_SDS_COMMON_STX0_TX_OUTPUT_CTRLB,
				   val);
		cvmx_wait_usec(10000);
	} else if (mod_info->limiting) {	/* SR type */
		if (device_tree_dbg)
			cvmx_dprintf("%s: Setting slice %d to SR mode for SFP slot %s\n",
				     __func__, reg, sfp->name);
		err = 0;
		err |= cvmx_mdio_45_write(phy_addr >> 8, phy_addr & 0xff, 0,
					  offset + CS4224_PP_LINE_SDS_DSP_MSEQ_SPARE22_MSB,
					  0);
		err |= cvmx_mdio_45_write(phy_addr >> 8, phy_addr & 0xff, 0,
					  offset + CS4224_PP_LINE_SDS_DSP_MSEQ_SPARE24_LSB,
					  9);
		cvmx_wait_usec(10000);
		err |= cvmx_mdio_45_write(phy_addr >> 8, phy_addr & 0xff, 0,
					  offset + CS4224_PP_HOST_SDS_DSP_MSEQ_SPARE22_MSB,
					  0);
		err |= cvmx_mdio_45_write(phy_addr >> 8, phy_addr & 0xff, 0,
					  offset + CS4224_PP_HOST_SDS_DSP_MSEQ_SPARE24_LSB,
					  9);
		cvmx_wait_usec(10000);
		val = ((slice->sr_stx_cmode_res & 7) << 12)     |
		      ((slice->sr_stx_drv_lower_cm & 0xf) << 8) |
		      (slice->sr_stx_level & 0x3f);
		err |= cvmx_mdio_45_write(phy_addr >> 8, phy_addr & 0xff, 0,
					  offset + CS4224_PP_LINE_SDS_COMMON_STX0_TX_OUTPUT_CTRLA,
					  val);
		val = cvmx_mdio_45_read(phy_addr >> 8, phy_addr & 0xff, 0,
					offset + CS4224_PP_LINE_SDS_COMMON_STX0_TX_OUTPUT_CTRLB);
		val &= 0xC000;
		val |= (slice->sr_stx_pre_peak << 8) & 0x1f;
		val |= (slice->sr_stx_muxsubrate_sel << 7) & 0x80;
		val |= slice->sr_stx_post_peak & 0x3f;
		cvmx_mdio_45_write(phy_addr >> 8, phy_addr & 0xff, 0,
				   offset + CS4224_PP_LINE_SDS_COMMON_STX0_TX_OUTPUT_CTRLB,
				   val);
		cvmx_wait_usec(10000);
	} else {				/* CX type */
		if (device_tree_dbg)
			cvmx_dprintf("%s: Setting slice %d to CX mode for SFP slot %s\n",
				     __func__, reg, sfp->name);
		err = 0;
		err |= cvmx_mdio_45_write(phy_addr >> 8, phy_addr & 0xff, 0,
					  offset + CS4224_PP_LINE_SDS_DSP_MSEQ_SPARE22_MSB,
					  0);
		err |= cvmx_mdio_45_write(phy_addr >> 8, phy_addr & 0xff, 0,
					  offset + CS4224_PP_LINE_SDS_DSP_MSEQ_SPARE24_LSB,
					  5);
		cvmx_wait_usec(10000);
		err |= cvmx_mdio_45_write(phy_addr >> 8, phy_addr & 0xff, 0,
					  offset + CS4224_PP_HOST_SDS_DSP_MSEQ_SPARE22_MSB,
					  0);
		err |= cvmx_mdio_45_write(phy_addr >> 8, phy_addr & 0xff, 0,
					  offset + CS4224_PP_HOST_SDS_DSP_MSEQ_SPARE24_LSB,
					  5);
		cvmx_wait_usec(10000);
		val = ((slice->cx_stx_cmode_res & 7) << 12)     |
		      ((slice->cx_stx_drv_lower_cm & 0xf) << 8) |
		      (slice->cx_stx_level & 0x3f);
		err |= cvmx_mdio_45_write(phy_addr >> 8, phy_addr & 0xff, 0,
					  offset + CS4224_PP_LINE_SDS_COMMON_STX0_TX_OUTPUT_CTRLA,
					  val);
		val = cvmx_mdio_45_read(phy_addr >> 8, phy_addr & 0xff, 0,
					offset + CS4224_PP_LINE_SDS_COMMON_STX0_TX_OUTPUT_CTRLB);
		val &= 0xC000;
		val |= (slice->cx_stx_pre_peak << 8) & 0x1f;
		val |= (slice->cx_stx_muxsubrate_sel << 7) & 0x80;
		val |= slice->cx_stx_post_peak & 0x3f;
		cvmx_mdio_45_write(phy_addr >> 8, phy_addr & 0xff, 0,
				   offset + CS4224_PP_LINE_SDS_COMMON_STX0_TX_OUTPUT_CTRLB,
				   val);
		cvmx_wait_usec(10000);
	}
	return err;
}

/**
 * Function called whenever mod_abs/mod_prs has changed for the Inphi CS4343
 *
 * @param	sfp	pointer to SFP data structure
 * @param	val	1 if absent, 0 if present, otherwise not set
 * @param	data	user-defined data
 *
 * @return	0 for success, -1 on error
 */
int cvmx_sfp_cs4343_mod_abs_changed(struct cvmx_fdt_sfp_info *sfp, int val,
				    void *data)
{
	const int dbg = device_tree_dbg;
	struct cvmx_phy_info *phy_info;
	struct cvmx_sfp_mod_info *mod_info;
	int err = -1;
	int i;

	if (dbg)
		cvmx_dprintf("%s(%s, %d, %p): Module %s\n", __func__,
			     sfp->name, val, data, val ? "absent" : "present");
	/* Ignore unplug */
	if (val)
		return 0;

	/* We're here if we detect that the module is now present */
	err = cvmx_sfp_read_i2c_eeprom(sfp);
	if (err) {
		cvmx_dprintf("%s: Error reading the SFP module eeprom for %s\n",
			     __func__, sfp->name);
		return err;
	}
	mod_info = &sfp->sfp_info;

	if (!mod_info->valid || !sfp->valid) {
		if (dbg)
			cvmx_dprintf("%s: Module data is invalid\n", __func__);
		return -1;
	}

	phy_info = cvmx_helper_get_port_phy_info(sfp->xiface, sfp->index);
	if (!phy_info || !phy_info->cs4343_info) {
		cvmx_dprintf("%s: Error: missing phy info or cs4343 info for xiface 0x%x, index %d\n",
			     __func__, sfp->xiface, sfp->index);
		return -1;
	}
	if (sfp->is_qsfp) {
		for (i = 0; i < 4; i++)
			err |= cvmx_cs4343_set_slice_mode(phy_info, i, sfp);
	} else {
		err = cvmx_cs4343_set_slice_mode(phy_info, phy_info->phy_sub_addr,
						 sfp);
	}

	if (err && device_tree_dbg)
		cvmx_dprintf("%s: Error setting slice mode for SFP %s\n",
			     __func__, sfp->name);
	return err;

}

static const char *cortina_compat_list_1[] = {
	"cortina,cs4343",
	"inphi,cs4343",
	"cortina,cs4223",
	"inphi,cs4223",
	NULL
};

static const char *cortina_compat_list_2[] = {
	"cortina,cs4343-slice",
	"inphi,cs4343-slice",
	"cortina,cs4223-slice",
	"inphi,cs4223-slice",
	NULL
};

/**
 * Return if the phy device is a Cortina multi-slice phy or a single slice
 *
 * @param[in]	phy_info	Pointer to phy information
 *
 * @return	1 for multi-slice, 2 for single slice, 0 for non-cs4343 device
 */
static int cvmx_is_cortina(const struct cvmx_phy_info *phy_info)
{
	static const void *fdt_addr;

	if (!fdt_addr)
		fdt_addr = __cvmx_phys_addr_to_ptr(cvmx_sysinfo_get()->fdt_addr,
						   OCTEON_FDT_MAX_SIZE);
	if (!cvmx_fdt_node_check_compatible_list(fdt_addr, phy_info->fdt_offset,
						 cortina_compat_list_1))
		return 1;
	if (!cvmx_fdt_node_check_compatible_list(fdt_addr, phy_info->fdt_offset,
						 cortina_compat_list_2))
		return 2;
	return 0;
}

/**
 * @INTERNAL
 * Figure out which mod_abs changed function to use based on the phy type
 *
 * @param	xiface	xinterface number
 * @param	index	port index on interface
 *
 * @return	0 for success, -1 on error
 *
 * This function figures out the proper mod_abs_changed function to use and
 * registers the appropriate function.  This should be called after the device
 * tree has been fully parsed for the given port as well as after all SFP
 * slots and any Microsemi VSC7224 devices have been parsed in the device tree.
 */
int cvmx_helper_phy_register_mod_abs_changed(int xiface, int index)
{
	struct cvmx_vsc7224_chan *vsc7224_chan;
	struct cvmx_phy_info *phy_info;
	struct cvmx_fdt_sfp_info *sfp_info;
	int cortina_type;

	if (device_tree_dbg)
		cvmx_dprintf("%s(0x%x, %d)\n", __func__, xiface, index);
	sfp_info = cvmx_helper_cfg_get_sfp_info(xiface, index);
	/* Don't return an error if no SFP module is registered */
	if (!sfp_info) {
		if (device_tree_dbg)
			cvmx_dprintf("%s: No SFP associated with 0x%x:%d\n",
				     __func__, xiface, index);
		return 0;
	}

	/* See if the Microsemi VSC7224 reclocking chip has been used */
	vsc7224_chan = cvmx_helper_cfg_get_vsc7224_chan_info(xiface, index);
	if (vsc7224_chan) {
		if (device_tree_dbg)
			cvmx_dprintf("%s: Registering VSC7224 handler\n",
				     __func__);
		cvmx_sfp_register_mod_abs_changed(sfp_info,
						  &cvmx_sfp_vsc7224_mod_abs_changed,
						  NULL);
		return 0;
	}

	/* Check which phy is used, i.e. Inphi CS4343 */
	phy_info = cvmx_helper_get_port_phy_info(xiface, index);
	if (!phy_info) {
		if (device_tree_dbg)
			cvmx_dprintf("%s: No phy associated with 0x%x:%d\n",
				     __func__, xiface, index);
		return 0;
	}

	cortina_type = cvmx_is_cortina(phy_info);
	switch (cortina_type) {
	case 1:
	case 2:
		if (device_tree_dbg)
			cvmx_dprintf("%s: Registering CS4343 handler\n",
				     __func__);
		cvmx_sfp_register_mod_abs_changed(sfp_info,
						  &cvmx_sfp_cs4343_mod_abs_changed,
						  NULL);
		return 0;
	default:
		cvmx_dprintf("%s(0x%x, %d): Unknown phy type, mod_abs changed not registered\n",
			     __func__, xiface, index);
		return -1;
	}
}

/** NOTE: Quick hack! */
int __cvmx_helper_78xx_parse_phy(struct cvmx_phy_info *phy_info, int ipd_port)
{
	static void *fdt_addr = NULL;
	const char *compat;
	int phy;
	int parent;
	uint64_t mdio_base;
	int node, bus;
	int phy_addr;
	int index = cvmx_helper_get_interface_index_num(ipd_port);
	int xiface = cvmx_helper_get_interface_num(ipd_port);
	int compat_len = 0;
	const int dbg = device_tree_dbg;
	int err;

	if (fdt_addr == NULL)
		fdt_addr = __cvmx_phys_addr_to_ptr(cvmx_sysinfo_get()->fdt_addr,
						   OCTEON_FDT_MAX_SIZE);

	if (device_tree_dbg)
		cvmx_dprintf("%s(0x%p, %d) ENTER\n",
			     __func__, phy_info, ipd_port);

	phy = cvmx_helper_get_phy_fdt_node_offset(xiface, index);
	if (dbg)
		cvmx_dprintf("%s: xiface: 0x%x, index: %d, ipd_port: %d, phy fdt offset: %d\n",
			     __func__, xiface, index, ipd_port, phy);
	if (phy < 0) {
		/* If this is the first time through we need to first parse the
		 * device tree to get the node offsets.
		 */
		if (device_tree_dbg)
			cvmx_dprintf("No config present, calling __cvmx_helper_parse_bgx_dt\n");
		if (__cvmx_helper_parse_bgx_dt(fdt_addr)) {
			cvmx_printf("Error: could not parse BGX device tree\n");
			return -1;
		}
		if (__cvmx_fdt_parse_vsc7224(fdt_addr)) {
			cvmx_dprintf("Error: could not parse Microsemi VSC7224 in DT\n");
			return -1;
		}
		if (octeon_has_feature(OCTEON_FEATURE_BGX_XCV) &&
		    __cvmx_helper_parse_bgx_rgmii_dt(fdt_addr)) {
			cvmx_printf("Error: could not parse BGX XCV device tree\n");
			return -1;
		}
		phy = cvmx_helper_get_phy_fdt_node_offset(xiface, index);
		if (phy < 0) {
			if (device_tree_dbg)
				cvmx_dprintf("%s: Could not get PHY node offset for IPD port 0x%x, xiface: 0x%x, index: %d\n",
					     __func__, ipd_port, xiface, index);
			return -1;
		}
		if (dbg)
			cvmx_dprintf("%s: phy: %d (%s)\n", __func__, phy,
				     fdt_get_name(fdt_addr, phy, NULL));
	}

	compat = (const char *)fdt_getprop(fdt_addr, phy, "compatible",
					   &compat_len);
	if (!compat) {
		cvmx_printf("ERROR: %d:%d:no compatible prop in phy\n", xiface, index);
		return -1;
	}

	if (dbg)
		cvmx_dprintf("  compatible: %s\n", compat);

	phy_info->fdt_offset = phy;
	phy_addr = cvmx_fdt_get_int(fdt_addr, phy, "reg", -1);
	if (phy_addr == -1) {
		cvmx_printf("Error: %d:%d:could not get PHY address\n",
			    xiface, index);
		return -1;
	}
	if (dbg)
		cvmx_dprintf("  PHY address: %d, compat: %s\n", phy_addr, compat);

	if (!memcmp("marvell", compat, strlen("marvell"))) {
		phy_info->phy_type = MARVELL_GENERIC_PHY;
		phy_info->link_function = __get_marvell_phy_link_state;
	} else if (!memcmp("broadcom", compat, strlen("broadcom"))) {
		phy_info->phy_type = BROADCOM_GENERIC_PHY;
		phy_info->link_function = __get_broadcom_phy_link_state;
	} else if (!memcmp("cortina", compat, strlen("cortina"))) {
		phy_info->phy_type = CORTINA_PHY;
		phy_info->link_function = __cvmx_get_cortina_phy_link_state;
	} else if (!strcmp("vitesse,vsc8490", compat)) {
		phy_info->phy_type = VITESSE_VSC8490_PHY;
		phy_info->link_function = __get_vitesse_vsc8490_phy_link_state;
	} else if (fdt_stringlist_contains(compat, compat_len,
					   "ethernet-phy-ieee802.3-c22")) {
		phy_info->phy_type = GENERIC_8023_C22_PHY;
		phy_info->link_function =
				__cvmx_get_generic_8023_c22_phy_link_state;
	} else if (fdt_stringlist_contains(compat, compat_len,
					   "ethernet-phy-ieee802.3-c45")) {
		phy_info->phy_type = GENERIC_8023_C22_PHY;
		phy_info->link_function = __get_generic_8023_c45_phy_link_state;
	}

	phy_info->ipd_port = ipd_port;
	phy_info->phy_sub_addr = 0;
	phy_info->direct_connect = 1;

	parent = fdt_parent_offset(fdt_addr, phy);
	if (!fdt_node_check_compatible(fdt_addr, parent, "ethernet-phy-nexus")) {
		if (device_tree_dbg)
			cvmx_dprintf("  nexus PHY found\n");
		if (phy_info->phy_type == CORTINA_PHY) {
			/* The Cortina CS422X uses the same PHY device for
			 * multiple ports for XFI.  In this case we use a
			 * nexus and each PHY address is the slice or
			 * sub-address and the actual PHY address is the
			 * nexus address.
			 */
			phy_info->phy_sub_addr = phy_addr;
			phy_addr = cvmx_fdt_get_int(fdt_addr, parent, "reg", -1);
			if (device_tree_dbg)
				cvmx_dprintf("  Cortina PHY real address: 0x%x\n", phy_addr);
		}
		parent = fdt_parent_offset(fdt_addr, parent);
	}

	if (dbg) {
		cvmx_dprintf("  Parent: %s\n",
			     fdt_get_name(fdt_addr, parent, NULL));
	}
	if (!fdt_node_check_compatible(fdt_addr, parent,
				       "cavium,octeon-3860-mdio")) {
		if (device_tree_dbg)
			cvmx_dprintf("  Found Octeon MDIO\n");
		mdio_base = cvmx_fdt_get_uint64(fdt_addr, parent, "reg",
						FDT_ADDR_T_NONE);
		if (device_tree_dbg)
			cvmx_dprintf("  MDIO address: 0x%llx\n",
				     (unsigned long long)mdio_base);

		mdio_base = cvmx_fdt_translate_address(fdt_addr, parent,
						       (uint32_t *)&mdio_base);
		if (device_tree_dbg)
			cvmx_dprintf("  Translated: 0x%llx\n",
				     (unsigned long long)mdio_base);
		if (mdio_base == FDT_ADDR_T_NONE) {
			cvmx_printf("Could not get MDIO base address from reg field\n");
			return -1;
		}
		__cvmx_mdio_addr_to_node_bus(mdio_base, &node, &bus);
		if (bus < 0) {
			cvmx_printf("Invalid MDIO address 0x%llx, could not detect bus and node\n",
				    (unsigned long long)mdio_base);
			return -1;
		}
		if (device_tree_dbg)
			cvmx_dprintf("  MDIO node: %d, bus: %d\n", node, bus);
		phy_info->mdio_unit = (node << 2) | (bus & 3);
		phy_info->phy_addr = phy_addr | (phy_info->mdio_unit << 8);
	} else {
		cvmx_printf("%s: Error: incompatible MDIO bus %s for IPD port %d\n",
			    __func__,
			    (const char *)fdt_get_name(fdt_addr, parent, NULL), ipd_port);
		return -1;
	}
	if (!fdt_node_check_compatible(fdt_addr, phy, "cortina,cs4343") ||
	    !fdt_node_check_compatible(fdt_addr, phy, "cortina,cs4343-slice")) {
		if (dbg)
			cvmx_dprintf("%s: Parsing cs4343 at 0x%x:%d\n",
				     __func__, xiface, index);
			err = cvmx_fdt_parse_cs4343(fdt_addr, phy, phy_info);
		if (err) {
			cvmx_dprintf("%s: Error parsing cs4343 phy in device tree\n",
				     __func__);
			return -1;
		}
		if (dbg)
			cvmx_dprintf("%s: Registering mod_abs changed function\n",
				     __func__);
		err = cvmx_helper_phy_register_mod_abs_changed(xiface, index);
		if (err) {
			cvmx_dprintf("%s: Error registering mod_abs changed handler for 0x%x:%d\n",
				     __func__, xiface, index);
			return -1;
		}
	}

	if (dbg)
		cvmx_dprintf("%s: EXIT 0\n", __func__);

	return 0;
}

void cvmx_rx_activity_led(int xiface, int index)
{

}

/**
 * Wrapper to allocate the LED data structure for SE, Linux and U-Boot
 *
 * @param[in]	leds	Pointer to LED data structure (checks if NULL to allocate)
 *
 * @return	pointer to existing LED data structure or to new LED data
 *		structure or NULL if out of memory.
 */
static struct cvmx_phy_gpio_leds *__alloc_leds(struct cvmx_phy_gpio_leds *leds)
{
	if (leds)
		return leds;

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
	leds = (struct cvmx_phy_gpio_leds *)kmalloc(sizeof(*leds), GFP_KERNEL);
#elif defined(__U_BOOT__)
	leds = (struct cvmx_phy_gpio_leds *)malloc(sizeof(*leds));
#else
	leds = (struct cvmx_phy_gpio_leds *)cvmx_bootmem_alloc(sizeof(*leds), 0);
#endif
	if (leds) {
		memset(leds, 0, sizeof(*leds));
		leds->error_gpio = -1;
		leds->link_status_gpio = -1;
		leds->rx_activity_gpio = -1;
		leds->tx_activity_gpio = -1;
		leds->error_gpio = -1;
		leds->rx_gpio_timer = -1;
		leds->tx_gpio_timer = -1;
		leds->link_poll_interval_ms = 1000;
		leds->activity_poll_interval_ms = 250;
	} else {
		cvmx_dprintf("%s: Out of memory!\n", __func__);
	}
	return leds;
}

/**
 * Wrapper to free LED data structure
 *
 * @param[in]	leds	pointer to free
 */
static inline void __free_leds(struct cvmx_phy_gpio_leds *leds)
{
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
	if (!leds)
		return;
	kfree(leds);
#elif defined(__U_BOOT__)
	free(leds);
#endif
}

/**
 * Gets all of the LED information for the specified LED
 *
 * @param[in]	fdt_addr	FDT address
 * @param	led_node	led node offset in device tree
 * @param[out]	active_low	set true if LED is active low
 * @param[out]	cpu_node	CPU node number GPIO is attached to
 * @param[out]	gpio		GPIO pin number
 *
 * @return	0 for success, -1 on error
 */
static int __get_led_gpio(void *fdt_addr, int led_node,
			  bool *active_low, int *cpu_node, int *gpio)
{
	int parent_node;
	int gpio_node;
	const int *val;
	int len = 0;

	parent_node = fdt_parent_offset(fdt_addr, led_node);
	if (parent_node < 0) {
		cvmx_dprintf("%s: Invalid parent node\n", __func__);
		return -1;
	}
	if (fdt_node_check_compatible(fdt_addr, parent_node, "gpio-leds")) {
		cvmx_dprintf("%s: LEDs not compatible\n", __func__);
		return -1;
	}

	val = fdt_getprop(fdt_addr, led_node, "gpios", &len);
	if (len < (int)(3 * sizeof(int))) {
		cvmx_dprintf("%s Invalid GPIO in device tree for LED\n", __func__);
	}
	gpio_node = fdt_node_offset_by_phandle(fdt_addr, val[0]);
	if (gpio_node < 0) {
		cvmx_dprintf("%s: Invalid GPIO phandle\n", __func__);
		return -1;
	}
	if (fdt_node_check_compatible(fdt_addr, gpio_node,
				      "cavium,octeon-3860-gpio") &&
	    fdt_node_check_compatible(fdt_addr, gpio_node,
				      "cavium,octeon-7890-gpio")) {
		cvmx_dprintf("%s: Error: Only native OCTEON GPIOs can be used for network LEDs\n",
			     __func__);
		return -1;
	}
	*gpio = fdt32_to_cpu(val[1]);
	*active_low = !!(fdt32_to_cpu(val[2]) & 1);
	*cpu_node = cvmx_fdt_get_cpu_node(fdt_addr, gpio_node);
	if (*cpu_node < 0) {
		cvmx_dprintf("%s: Could not get GPIO CPU node number\n",
			     __func__);
		return -1;
	}
	if (device_tree_dbg)
		cvmx_dprintf("Parsed LED label %s, CPU node: %d, GPIO pin %d, active %s\n",
			     (char *)fdt_getprop(fdt_addr, led_node, "label",
						 NULL),
			     *cpu_node, *gpio, *active_low ? "low" : "high");

	return 0;
}

/**
 * Parses an Ethernet port for LEDs hooked up to GPIO pins
 *
 * @param[in]	fdt_addr	Address of flat device tree
 * @param	port_node	FDT node offset for the port
 *
 * @return	Pointer to LED data structure or NULL if error or if LEDs are
 *		not used.
 */
struct cvmx_phy_gpio_leds *
__cvmx_helper_parse_gpio_leds(void *fdt_addr, int port_node, bool is_xiface)
{
	struct cvmx_phy_gpio_leds *leds = NULL;
	int led_node;
	int cpu_node = 0;
	int def_timer = 3;

	/* Get link status LED */
	led_node = cvmx_fdt_lookup_phandle(fdt_addr, port_node,
					   "cavium,link-status-led");
	if (led_node >= 0) {
		leds = __alloc_leds(leds);
		if (!leds)
			return NULL;
		if (__get_led_gpio(fdt_addr, led_node,
				   &leds->link_status_active_low, &cpu_node,
				   &leds->link_status_gpio)) {
			cvmx_dprintf("%s: Error getting link status LED\n",
				     __func__);
			__free_leds(leds);
			return NULL;
		}
	}

	/* Get RX activity LED */
	led_node = cvmx_fdt_lookup_phandle(fdt_addr, port_node,
					   "cavium,rx-activity-led");
	if (led_node >= 0) {
		leds = __alloc_leds(leds);
		if (!leds)
			return NULL;
		if (__get_led_gpio(fdt_addr, led_node,
				   &leds->rx_activity_active_low, &cpu_node,
				   &leds->rx_activity_gpio)) {
			cvmx_dprintf("%s: Error getting RX activity LED\n",
				     __func__);
			__free_leds(leds);
			return NULL;
		}
		leds->rx_activity_gpio |= (cpu_node << 8);
		leds->rx_activity_hz = cvmx_fdt_get_int(fdt_addr, port_node,
							"cavium,rx-activity-blink-rate",
							0);
		leds->rx_gpio_timer = cvmx_fdt_get_int(fdt_addr, port_node,
						       "cavium,rx-timer", 3);
		if (leds->rx_gpio_timer > 3 || leds->rx_gpio_timer < 0) {
			cvmx_printf("Error: RX GPIO timer in device tree is out of range!  Must be 0..3\n");
			leds->rx_gpio_timer = 3;
		}
	}

	/* Get TX activity LED */
	led_node = cvmx_fdt_lookup_phandle(fdt_addr, port_node,
					   "cavium,tx-activity-led");
	if (led_node >= 0) {
		leds = __alloc_leds(leds);
		if (!leds)
			return NULL;
		if (__get_led_gpio(fdt_addr, led_node,
				   &leds->tx_activity_active_low, &cpu_node,
				   &leds->tx_activity_gpio)) {
			cvmx_dprintf("%s: Error getting TX activity LED\n",
				     __func__);
			__free_leds(leds);
			return NULL;
		}
		leds->tx_activity_gpio |= (cpu_node << 8);
		leds->tx_activity_hz = cvmx_fdt_get_int(fdt_addr, port_node,
							"cavium,tx-activity-blink-rate",
							0);
		if ((leds->tx_activity_hz == leds->rx_activity_hz) ||
		    leds->rx_activity_hz == 0)
			def_timer = 3;
		else
			def_timer = 2;
		leds->tx_gpio_timer = cvmx_fdt_get_int(fdt_addr, port_node,
						       "cavium,tx-timer",
						       def_timer);
		if (leds->tx_gpio_timer > 3 || leds->tx_gpio_timer < 0) {
			cvmx_printf("Error: TX GPIO timer in device tree is out of range!  Must be 0..3\n");
			leds->tx_gpio_timer = 3;
		}
	}

	/* Get Error LED */
	led_node = cvmx_fdt_lookup_phandle(fdt_addr, port_node,
					   "cavium,error-led");
	if (led_node >= 0) {
		leds = __alloc_leds(leds);
		if (!leds)
			return NULL;
		if (__get_led_gpio(fdt_addr, led_node,
				   &leds->error_active_low, &cpu_node,
				   &leds->error_gpio)) {
			cvmx_dprintf("%s: Error getting TX activity LED\n",
				     __func__);
			__free_leds(leds);
			return NULL;
		}
		leds->error_gpio |= (cpu_node << 8);
	}
	if (leds) {
		leds->interface_leds = is_xiface;
		if (leds->rx_activity_hz > 0 && leds->rx_gpio_timer >= 0)
			/* Set the GPIO frequency and select the timer used */
			cvmx_gpio_set_freq(leds->rx_activity_gpio >> 8,
					   leds->rx_gpio_timer,
					   leds->rx_activity_hz);

		/* It doesn't matter if it's the same timer as RX since it means
		 * the values are the same anyway.
		 */
		if (leds->tx_activity_hz > 0 && leds->tx_gpio_timer >= 0)
			cvmx_gpio_set_freq(leds->tx_activity_gpio >> 8,
					   leds->tx_gpio_timer,
					   leds->tx_activity_hz);

		leds->link_poll_interval_ms =
			cvmx_fdt_get_int(fdt_addr, port_node,
					 "cavium,link-poll-interval-ms",
					 leds->link_poll_interval_ms);
		leds->activity_poll_interval_ms =
			cvmx_fdt_get_int(fdt_addr, port_node,
					 "cavium,activity-poll-interval-ms",
					 leds->activity_poll_interval_ms);
	}
	return leds;
}

/**
 * @INTERNAL
 * Parse the device tree and set whether a port is valid or not.
 *
 * @param fdt_addr	Pointer to device tree
 *
 * @return 0 for success, -1 on error.
 */
int __cvmx_helper_parse_bgx_dt(void *fdt_addr)
{
	int port_index;
	const int dbg = device_tree_dbg;
	struct cvmx_xiface xi;
	int fdt_port_node = -1;
	int fdt_interface_node;
	int fdt_phy_node;
	uint64_t reg_addr;
	int xiface;
	struct cvmx_phy_gpio_leds *gpio_leds = NULL;
	struct cvmx_fdt_sfp_info *sfp_info = NULL;
	struct cvmx_phy_info *phy_info;
	int sfp_node;
	static bool parsed;
	int err;
	int ipd_port;

	if (parsed) {
		if (dbg)
			cvmx_dprintf("%s: Already parsed\n", __func__);
		return 0;
	}
	while ((fdt_port_node = fdt_node_offset_by_compatible(fdt_addr, fdt_port_node,
					"cavium,octeon-7890-bgx-port")) >= 0) {
		/* Get the port number */
		port_index = cvmx_fdt_get_int(fdt_addr, fdt_port_node,
					      "reg", -1);
		if (port_index < 0) {
			cvmx_dprintf("Error: missing reg field for bgx port in device tree\n");
			return -1;
		}
		if (dbg) {
			cvmx_dprintf("%s: Parsing BGX port %d\n",
				     __func__, port_index);
		}
		/* Get the interface number */
		fdt_interface_node = fdt_parent_offset(fdt_addr, fdt_port_node);
		if (fdt_interface_node < 0) {
			cvmx_dprintf("Error: device tree corrupt!\n");
			return -1;
		}
		if (fdt_node_check_compatible(fdt_addr, fdt_interface_node,
					      "cavium,octeon-7890-bgx")) {
			cvmx_dprintf("Error: incompatible Ethernet MAC Nexus in device tree!\n");
			return -1;
		}
		reg_addr = cvmx_fdt_get_addr(fdt_addr, fdt_interface_node,
					     "reg");
		if (dbg)
			cvmx_dprintf("%s: BGX interface address: 0x%llx\n",
				     __func__, (unsigned long long)reg_addr);
		if (reg_addr == FDT_ADDR_T_NONE) {
			cvmx_dprintf("Device tree BGX node has invalid address 0x%llx\n",
				     (unsigned long long)reg_addr);
			return -1;
		}
		reg_addr = cvmx_fdt_translate_address(fdt_addr,
						      fdt_interface_node,
						      (uint32_t *)&reg_addr);
		xi = __cvmx_bgx_reg_addr_to_xiface(reg_addr);
		if (xi.node < 0) {
			cvmx_dprintf("Device tree BGX node has invalid address 0x%llx\n",
				     (unsigned long long)reg_addr);
			return -1;
		}
		if (dbg)
			cvmx_dprintf("%s: Found BGX node %d, interface %d\n",
				     __func__, xi.node, xi.interface);
		xiface = cvmx_helper_node_interface_to_xiface(xi.node,
							      xi.interface);
		cvmx_helper_set_port_fdt_node_offset(xiface, port_index,
						     fdt_port_node);
		cvmx_helper_set_port_valid(xiface, port_index, true);


		cvmx_helper_set_port_fdt_node_offset(xiface, port_index,
						     fdt_port_node);
		if (fdt_getprop(fdt_addr, fdt_port_node, "cavium,sgmii-mac-phy-mode",
			NULL))
			cvmx_helper_set_mac_phy_mode(xiface, port_index, true);
		else
			cvmx_helper_set_mac_phy_mode(xiface, port_index, false);

		if (fdt_getprop(fdt_addr, fdt_port_node, "cavium,force-link-up", NULL))
			cvmx_helper_set_port_force_link_up(xiface, port_index, true);
		else
			cvmx_helper_set_port_force_link_up(xiface, port_index, false);

		if (fdt_getprop(fdt_addr, fdt_port_node,
				"cavium,sgmii-mac-1000x-mode", NULL))
			cvmx_helper_set_1000x_mode(xiface, port_index, true);
		else
			cvmx_helper_set_1000x_mode(xiface, port_index, false);

		if (fdt_getprop(fdt_addr, fdt_port_node,
				"cavium,disable-autonegotiation", NULL))
			cvmx_helper_set_port_autonegotiation(xiface, port_index, false);
		else
			cvmx_helper_set_port_autonegotiation(xiface, port_index, true);

		fdt_phy_node = cvmx_fdt_lookup_phandle(fdt_addr, fdt_port_node,
						       "phy-handle");
		if (fdt_phy_node >= 0) {
			cvmx_helper_set_phy_fdt_node_offset(xiface, port_index,
							    fdt_phy_node);
			if (dbg) {
				cvmx_dprintf("%s: Setting PHY fdt node offset for interface 0x%x, port %d to %d\n",
					     __func__, xiface, port_index,
					     fdt_phy_node);
				cvmx_dprintf("%s: PHY node name: %s\n",
					     __func__,
					     fdt_get_name(fdt_addr,
							  fdt_phy_node, NULL));
			}
			cvmx_helper_set_port_phy_present(xiface, port_index, true);
			ipd_port = cvmx_helper_get_ipd_port(xiface, port_index);
			if (ipd_port >= 0) {
				if (dbg)
					cvmx_dprintf("%s: Allocating phy info for 0x%x:%d\n",
						     __func__, xiface,
						     port_index);
				phy_info = (cvmx_phy_info_t *)
					cvmx_bootmem_alloc(sizeof(*phy_info), 0);
				if (!phy_info) {
					cvmx_dprintf("%s: Out of memory\n",
						     __func__);
					return -1;
				}
				memset(phy_info, 0, sizeof(*phy_info));
				phy_info->phy_addr = -1;
				err = __get_phy_info_from_dt(phy_info, ipd_port);
				if (err) {
					cvmx_dprintf("%s: Error parsing phy info for ipd port %d\n",
						     __func__, ipd_port);
					return -1;
				}
				cvmx_helper_set_port_phy_info(xiface, port_index,
							      phy_info);
				if (dbg)
					cvmx_dprintf("%s: Saved phy info\n",
						     __func__);
			}
		} else {
			cvmx_helper_set_phy_fdt_node_offset(xiface, port_index,
							    -1);
			if (dbg)
				cvmx_dprintf("%s: No PHY fdt node offset for interface 0x%x, port %d to %d\n",
					     __func__, xiface, port_index,
					     fdt_phy_node);
			cvmx_helper_set_port_phy_present(xiface, port_index,
							 false);

		}
		gpio_leds = __cvmx_helper_parse_gpio_leds(fdt_addr,
							  fdt_port_node, false);
		if (gpio_leds) {
			cvmx_helper_set_port_phy_leds(xiface, port_index,
						      gpio_leds);
		} else {
			gpio_leds = __cvmx_helper_parse_gpio_leds(fdt_addr,
								  fdt_interface_node,
								  true);
			if (gpio_leds)
				cvmx_helper_set_port_phy_leds(xiface,
							      port_index,
							      gpio_leds);
		}
		sfp_node = cvmx_fdt_lookup_phandle(fdt_addr, fdt_port_node,
						   "qsfp-slot");
		if (sfp_node > 0) {
			/* We have a QSFP slot */
			sfp_info = cvmx_helper_fdt_parse_sfp_info(fdt_addr,
								  sfp_node);
			if (dbg)
				cvmx_dprintf("%s: Got QSFP slot node %d, info at %p\n",
					     __func__, sfp_node, sfp_info);
			sfp_info->xiface = xiface;
			sfp_info->index = port_index;

			cvmx_helper_cfg_set_sfp_info(xiface, port_index,
						     sfp_info);
			cvmx_helper_phy_register_mod_abs_changed(xiface,
								 port_index);
		} else {
			int i;
			uint32_t *phandles;
			int len;
			int sfp_node;

			/* Check for one or more SFP slots */
			phandles = (uint32_t *)fdt_getprop(fdt_addr,
							   fdt_port_node,
							   "sfp-slot", &len);
			if (phandles) {
				struct cvmx_fdt_sfp_info *first_sfp, *last_sfp;
				first_sfp = NULL;
				last_sfp = NULL;
				/* Get number of phandles */
				len /= sizeof(uint32_t);
				if (dbg)
					cvmx_dprintf("%s: Found %d sfp phandles\n",
						     __func__, len);
				for (i = 0; i < len; i++) {
					sfp_node = fdt_node_offset_by_phandle(fdt_addr,
									      fdt32_to_cpu(phandles[i]));
					if (sfp_node < 0)
						continue;
					sfp_info = cvmx_helper_fdt_parse_sfp_info(fdt_addr,
										  sfp_node);
					if (dbg)
						cvmx_dprintf("%s: SFP node %d at offset %d, sfp_info at %p\n",
							     __func__, i,
							     sfp_node,
							     sfp_info);
					/* Link them together */
					if (sfp_info) {
						if (!first_sfp)
							first_sfp = sfp_info;
						if (last_sfp)
							last_sfp->next = sfp_info;
						sfp_info->prev = last_sfp;
						sfp_info->xiface = xiface;
						sfp_info->index = port_index;
					}
				}
				cvmx_helper_cfg_set_sfp_info(xiface, port_index,
							     first_sfp);
				cvmx_helper_phy_register_mod_abs_changed(xiface,
									 port_index);
			} else if (dbg) {
				cvmx_dprintf("%s: No SFP slots found\n",
					     __func__);
			}
		}
	}
	parsed = true;
	return 0;
}

int __cvmx_helper_parse_bgx_rgmii_dt(const void *fdt_addr)
{
	uint64_t reg_addr;
	struct cvmx_xiface xi;
	int fdt_port_node = -1;
	int fdt_interface_node;
	int fdt_phy_node;
	int port_index;
	int xiface;
	int dbg = device_tree_dbg;

	/* There's only one xcv (RGMII) interface, so just search for the one
	 * that's part of a BGX entry.
	 */
	while ((fdt_port_node = fdt_node_offset_by_compatible(fdt_addr,
							      fdt_port_node,
							      "cavium,octeon-7360-xcv")) >= 0) {
		fdt_interface_node = fdt_parent_offset(fdt_addr, fdt_port_node);
		if (fdt_interface_node < 0) {
			cvmx_printf("Error: device tree corrupt!\n");
			return -1;
		}
		if (dbg)
			cvmx_dprintf("%s: XCV parent node compatible: %s\n",
				     __func__,
				     (char *)fdt_getprop(fdt_addr,
							 fdt_interface_node,
							 "compatible", NULL));
		if (!fdt_node_check_compatible(fdt_addr, fdt_interface_node,
					       "cavium,octeon-7890-bgx"))
			break;
	}
	if (fdt_port_node == -FDT_ERR_NOTFOUND) {
		if (dbg)
			cvmx_dprintf("No XCV/RGMII interface found in device tree\n");
		return 0;
	} else if (fdt_port_node < 0) {
		cvmx_dprintf("%s: Error %d parsing device tree\n",
			     __func__, fdt_port_node);
		return -1;
	}
	if (dbg) {
		char path[256];
		if (!fdt_get_path(fdt_addr, fdt_port_node, path, sizeof(path))) {
			cvmx_dprintf("xcv path: %s\n", path);
		}
		if (!fdt_get_path(fdt_addr, fdt_interface_node, path,
				  sizeof(path))) {
			cvmx_dprintf("interface path: %s\n", path);
		}
	}
	port_index = cvmx_fdt_get_int(fdt_addr, fdt_port_node, "reg", -1);
	if (port_index != 0) {
		cvmx_printf("%s: Error: port index (reg) must be 0, not %d.\n",
			    __func__, port_index);
		return -1;
	}
	reg_addr = cvmx_fdt_get_addr(fdt_addr, fdt_interface_node, "reg");
	if (reg_addr == FDT_ADDR_T_NONE) {
		cvmx_printf("%s: Error: could not get BGX interface address\n",
			    __func__);
		return -1;
	}
	/* We don't have to bother translating since only 78xx supports OCX and
	 * doesn't support RGMII.
	 */
	xi = __cvmx_bgx_reg_addr_to_xiface(reg_addr);
	if (dbg)
		cvmx_dprintf("%s: xi.node: %d, xi.interface: 0x%x, addr: 0x%llx\n", __func__,
			     xi.node, xi.interface,
			     (unsigned long long)reg_addr);
	if (xi.node < 0) {
		cvmx_printf("%s: Device tree BGX node has invalid address 0x%llx\n",
			    __func__, (unsigned long long)reg_addr);
		return -1;
	}
	if (dbg) {
		cvmx_dprintf("%s: Found XCV (RGMII) interface on interface %d\n",
			     __func__, xi.interface);
		cvmx_dprintf("  phy handle: 0x%x\n",
			     cvmx_fdt_get_int(fdt_addr, fdt_port_node,
					      "phy-handle", -1));
	}
	fdt_phy_node = cvmx_fdt_lookup_phandle(fdt_addr, fdt_port_node,
					       "phy-handle");
	if (dbg)
		cvmx_dprintf("%s: phy-handle node: 0x%x\n", __func__,
			     fdt_phy_node);
	xiface = cvmx_helper_node_interface_to_xiface(xi.node, xi.interface);

	cvmx_helper_set_port_fdt_node_offset(xiface, port_index,
					     fdt_port_node);
	if (fdt_phy_node >= 0) {
		if (dbg) {
			cvmx_dprintf("%s: Setting PHY fdt node offset for interface 0x%x, port %d to %d\n",
				     __func__, xiface, port_index,
				     fdt_phy_node);
			cvmx_dprintf("%s: PHY node name: %s\n",
				     __func__,
				     fdt_get_name(fdt_addr,
						  fdt_phy_node, NULL));
		}
		cvmx_helper_set_phy_fdt_node_offset(xiface, port_index,
						    fdt_phy_node);
		cvmx_helper_set_port_phy_present(xiface, port_index, true);
	} else {
		cvmx_helper_set_phy_fdt_node_offset(xiface, port_index, -1);
		if (dbg)
			cvmx_dprintf("%s: No PHY fdt node offset for interface 0x%x, port %d to %d\n",
				     __func__, xiface, port_index, fdt_phy_node);
		cvmx_helper_set_port_phy_present(xiface, port_index, false);
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
	int port_index;
	int aliases;
	const char *pip_path;
	char name_buffer[24];
	int pip, iface, eth;
	int dbg = device_tree_dbg;
	cvmx_helper_interface_mode_t mode;
	int xiface = cvmx_helper_get_interface_num(ipd_port);
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	uint32_t val;
	int phy_node_offset;
	int parse_bgx_dt_err;
	int parse_vsc7224_err;

	if (dbg)
		cvmx_dprintf("%s(%p, %d)\n", __func__, fdt_addr, ipd_port);
	if (octeon_has_feature(OCTEON_FEATURE_BGX)) {
		static int fdt_ports_initialized = 0;

		port_index = cvmx_helper_get_interface_index_num(ipd_port);

		if (!fdt_ports_initialized) {
			if (octeon_has_feature(OCTEON_FEATURE_BGX_XCV)) {
				if (!__cvmx_helper_parse_bgx_rgmii_dt(fdt_addr))
					fdt_ports_initialized = 1;
				parse_bgx_dt_err =
					__cvmx_helper_parse_bgx_dt(fdt_addr);
				parse_vsc7224_err =
					__cvmx_fdt_parse_vsc7224(fdt_addr);
				if (!parse_bgx_dt_err && !parse_vsc7224_err)
					fdt_ports_initialized = 1;
			} else {
				cvmx_dprintf("%s: Error parsing FDT\n",
					     __func__);
				return -1;
			}
		}

		return cvmx_helper_is_port_valid(xiface, port_index);
	}

	mode = cvmx_helper_interface_get_mode(xiface);

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
	case CVMX_HELPER_INTERFACE_MODE_XLAUI:
	case CVMX_HELPER_INTERFACE_MODE_XFI:
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
		cvmx_dprintf("%s: ERROR: "
			"/aliases not found in device tree fdt_addr=%p\n",
			__func__, fdt_addr);
		return -1;
	}

	pip_path = fdt_getprop(fdt_addr, aliases, "pip", NULL);
	if (!pip_path) {
		cvmx_dprintf("%s: ERROR: "
			"interface %x pip path not found in device tree\n",
		         __func__, xiface);
		return -1;
	}
	pip = fdt_path_offset(fdt_addr, pip_path);
	if (pip < 0) {
		cvmx_dprintf("%s: ERROR: "
			"interface %x pip not found in device tree\n",
			__func__, xiface);
		return -1;
	}
	snprintf(name_buffer, sizeof(name_buffer), "interface@%d", xi.interface);
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

	cvmx_helper_set_port_fdt_node_offset(xiface, port_index, eth);

	phy_node_offset = cvmx_fdt_get_int(fdt_addr, eth, "phy", -1);
	cvmx_helper_set_phy_fdt_node_offset(xiface, port_index,
					    phy_node_offset);

	if (fdt_getprop(fdt_addr, eth, "cavium,sgmii-mac-phy-mode", NULL))
		cvmx_helper_set_mac_phy_mode(xiface, port_index, true);
	else
		cvmx_helper_set_mac_phy_mode(xiface, port_index, false);

	if (fdt_getprop(fdt_addr, eth, "cavium,force-link-up", NULL))
		cvmx_helper_set_port_force_link_up(xiface, port_index, true);
	else
		cvmx_helper_set_port_force_link_up(xiface, port_index, false);

	if (fdt_getprop(fdt_addr, eth, "cavium,sgmii-mac-1000x-mode", NULL))
		cvmx_helper_set_1000x_mode(xiface, port_index, true);
	else
		cvmx_helper_set_1000x_mode(xiface, port_index, false);

	if (fdt_getprop(fdt_addr, eth, "cavium,disable-autonegotiation", NULL))
		cvmx_helper_set_port_autonegotiation(xiface, port_index, false);
	else
		cvmx_helper_set_port_autonegotiation(xiface, port_index, true);

	if (mode == CVMX_HELPER_INTERFACE_MODE_AGL) {
		bool tx_bypass = false;
		if (fdt_getprop(fdt_addr, eth, "cavium,rx-clk-delay-bypass", NULL))
			cvmx_helper_set_agl_rx_clock_delay_bypass(xiface,
								  port_index,
								  true);
		else
			cvmx_helper_set_agl_rx_clock_delay_bypass(xiface,
								  port_index,
								  false);

		val = cvmx_fdt_get_int(fdt_addr, eth, "cavium,rx-clk-skew", 0);
		cvmx_helper_set_agl_rx_clock_skew(xiface, port_index, val);

		if (fdt_getprop(fdt_addr, eth,
				"cavium,tx-clk-delay-bypass", NULL))
			tx_bypass = true;

		val = cvmx_fdt_get_int(fdt_addr, eth, "tx-clk-delay", 0);
		cvmx_helper_cfg_set_rgmii_tx_clk_delay(xiface,
						       port_index,
						       tx_bypass, val);

		val = cvmx_fdt_get_int(fdt_addr, eth, "cavium,refclk-sel", 0);
		cvmx_helper_set_agl_refclk_sel(xiface, port_index, val);
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
	int aliases, eth, phy, phy_parent, ret, i;
	int mdio_parent;
	const char *phy_compatible_str;
	const char *host_mode_str = NULL;
	int dbg = device_tree_dbg;
	int interface;
	int phy_addr_offset = 0;

	if (dbg)
		cvmx_dprintf("%s(%p, %d)\n", __func__, phy_info, ipd_port);

	if (octeon_has_feature(OCTEON_FEATURE_BGX))
		return __cvmx_helper_78xx_parse_phy(phy_info, ipd_port);

	if (fdt_addr == 0)
		fdt_addr = __cvmx_phys_addr_to_ptr(cvmx_sysinfo_get()->fdt_addr,
						   OCTEON_FDT_MAX_SIZE);

	phy_info->phy_addr = -1;
	phy_info->phy_sub_addr = 0;
	phy_info->ipd_port = ipd_port;
	phy_info->direct_connect = -1;
	phy_info->phy_type = (cvmx_phy_type_t) -1;
	for (i = 0; i < CVMX_PHY_MUX_MAX_GPIO; i++)
		phy_info->gpio[i] = -1;
	phy_info->mdio_unit = -1;
	phy_info->gpio_value = -1;
	phy_info->gpio_parent_mux_twsi = -1;
	phy_info->gpio_parent_mux_select = -1;
	phy_info->link_function = NULL;
	phy_info->fdt_offset = -1;
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
	phy = cvmx_fdt_lookup_phandle(fdt_addr, eth, "phy-handle");
	if (phy < 0) {
		cvmx_helper_interface_mode_t if_mode;
		/* Note that it's OK for RXAUI and ILK to not have a PHY
		 * connected (i.e. EBB boards in loopback).
		 */
		if (dbg)
			cvmx_dprintf("Cannot get phy-handle for ipd_port: %d\n",
				     ipd_port);
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

	phy_compatible_str = (const char *)fdt_getprop(fdt_addr, phy,
						       "compatible", NULL);
	if (!phy_compatible_str) {
		cvmx_dprintf("ERROR: no compatible prop in phy\n");
		return -1;
	}
	if (dbg)
		cvmx_dprintf("Checking compatible string \"%s\" for ipd port %d\n",
			     phy_compatible_str, ipd_port);
	phy_info->fdt_offset = phy;
	if (!memcmp("marvell", phy_compatible_str, strlen("marvell"))) {
		if (dbg)
			cvmx_dprintf("Marvell PHY detected for ipd_port %d\n",
				     ipd_port);
		phy_info->phy_type = MARVELL_GENERIC_PHY;
		phy_info->link_function = __get_marvell_phy_link_state;
	} else if (!memcmp("broadcom", phy_compatible_str, strlen("broadcom"))) {
		phy_info->phy_type = BROADCOM_GENERIC_PHY;
		phy_info->link_function = __get_broadcom_phy_link_state;
		if (dbg)
			cvmx_dprintf("Broadcom PHY detected for ipd_port %d\n",
				     ipd_port);
	} else if (!memcmp("vitesse", phy_compatible_str, strlen("vitesse"))) {
		if (dbg)
			cvmx_dprintf("Vitesse PHY detected for ipd_port %d\n",
				     ipd_port);
		if (!fdt_node_check_compatible(fdt_addr, phy, "vitesse,vsc8490")) {
			phy_info->phy_type = VITESSE_VSC8490_PHY;
			if (dbg)
				cvmx_dprintf("Vitesse VSC8490 detected\n");
			phy_info->link_function = __get_vitesse_vsc8490_phy_link_state;
		} else if (!fdt_node_check_compatible(fdt_addr, phy,
						      "ethernet-phy-ieee802.3-c22")) {
			phy_info->phy_type = GENERIC_8023_C22_PHY;
			phy_info->link_function =
					__cvmx_get_generic_8023_c22_phy_link_state;
			if (dbg)
				cvmx_dprintf("Vitesse 802.3 c22 detected\n");
		} else {
			phy_info->phy_type = GENERIC_8023_C45_PHY;
			phy_info->link_function =
				__get_generic_8023_c45_phy_link_state;
			if (dbg)
				cvmx_dprintf("Vitesse 802.3 c45 detected\n");
		}
	} else if (!memcmp("aquantia", phy_compatible_str, strlen("aquantia"))) {
		phy_info->phy_type = AQUANTIA_PHY;
		phy_info->link_function = __get_aquantia_phy_link_state;
		if (dbg)
			cvmx_dprintf("Aquantia c45 PHY detected\n");
	} else if (!memcmp("cortina", phy_compatible_str, strlen("cortina"))) {
		phy_info->phy_type = CORTINA_PHY;
		phy_info->link_function = __cvmx_get_cortina_phy_link_state;
		host_mode_str = (const char *)fdt_getprop(fdt_addr, phy,
							  "cortina,host-mode",
							  NULL);
		if (dbg)
			cvmx_dprintf("Cortina PHY detected for ipd_port %d\n",
				     ipd_port);
	} else if (!memcmp("ti", phy_compatible_str, strlen("ti"))) {
		phy_info->phy_type = GENERIC_8023_C45_PHY;
		phy_info->link_function = __get_generic_8023_c45_phy_link_state;
		if (dbg)
			cvmx_dprintf("TI PHY detected for ipd_port %d\n",
				     ipd_port);
	} else if (!fdt_node_check_compatible(fdt_addr, phy, "atheros,ar8334") ||
		   !fdt_node_check_compatible(fdt_addr, phy, "qualcomm,qca8334") ||
		   !fdt_node_check_compatible(fdt_addr, phy, "atheros,ar8337") ||
		   !fdt_node_check_compatible(fdt_addr, phy, "qualcomm,qca8337")) {
		phy_info->phy_type = QUALCOMM_S17;
		phy_info->link_function = __cvmx_get_qualcomm_s17_phy_link_state;
		if (dbg)
			cvmx_dprintf("Qualcomm QCA833X switch detected\n");
	} else if (!fdt_node_check_compatible(fdt_addr, phy,
					      "ethernet-phy-ieee802.3-c22")) {
		phy_info->phy_type = GENERIC_8023_C22_PHY;
		phy_info->link_function =
				__cvmx_get_generic_8023_c22_phy_link_state;
		if (dbg)
			cvmx_dprintf("Generic 802.3 c22 PHY detected\n");
	} else if (!fdt_node_check_compatible(fdt_addr, phy,
					      "ethernet-phy-ieee802.3-c45")) {
		phy_info->phy_type = GENERIC_8023_C45_PHY;
		phy_info->link_function = __get_generic_8023_c45_phy_link_state;
		if (dbg)
			cvmx_dprintf("Generic 802.3 c45 PHY detected\n");
	} else {
		cvmx_dprintf("Unknown PHY compatibility\n");
		phy_info->phy_type = -1;
		phy_info->link_function = NULL;
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
	if (ret == 0) {
		/* It's a nexus so check the grandparent. */
		phy_addr_offset = cvmx_fdt_get_int(fdt_addr, phy_parent,
						 "reg", 0);
		phy_parent = fdt_parent_offset(fdt_addr, phy_parent);
	}

	/* Check for a muxed MDIO interface */
	mdio_parent = fdt_parent_offset(fdt_addr, phy_parent);
	ret = fdt_node_check_compatible(fdt_addr, mdio_parent,
					"cavium,mdio-mux");
	if (ret == 0) {
		ret = __get_muxed_mdio_info_from_dt(phy_info, phy_parent,
						    mdio_parent);
		if (ret) {
			printf("Error reading mdio mux information for ipd port %d\n",
			       ipd_port);
			return -1;
		}
	}
	ret = fdt_node_check_compatible(fdt_addr, phy_parent,
					"cavium,octeon-3860-mdio");
	if (ret == 0) {
		uint32_t *mdio_reg_base = (uint32_t *) fdt_getprop(fdt_addr,
								   phy_parent,
								   "reg", 0);
		phy_info->direct_connect = 1;
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

	phy_info->phy_addr = cvmx_fdt_get_int(fdt_addr, phy, "reg", -1);
	if (phy_info->phy_addr < 0) {
		cvmx_dprintf("ERROR: Could not read phy address from reg in DT\n");
		return -1;
	}
	phy_info->phy_addr += phy_addr_offset;
	phy_info->phy_addr |= phy_info->mdio_unit << 8;
	if (dbg)
		cvmx_dprintf("%s(%p, %d) => 0x%x\n", __func__,
			     phy_info, ipd_port, phy_info->phy_addr);
	return phy_info->phy_addr;
}

/**
 * @INTERNAL
 * This function outputs the cvmx_phy_info_t data structure for the specified
 * port.
 *
 * @param phy_info - phy info data structure
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
	const int dbg = device_tree_dbg;
#endif
	if (cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_SIM)
		return -1;
#ifndef CVMX_BUILD_FOR_LINUX_KERNEL

	if (cvmx_sysinfo_get()->fdt_addr) {
		cvmx_phy_info_t phy_info;
		int retcode;

		if (dbg)
			cvmx_dprintf("%s(0x%x) getting phy info from device tree\n",
				     __func__, ipd_port);
		memset(&phy_info, 0, sizeof(phy_info));
		retcode = __get_phy_info_from_dt(&phy_info, ipd_port);

		if (retcode == -2) {
			if (device_tree_dbg)
				cvmx_dprintf("%s(0x%x): phy info missing in device tree\n",
					     __func__, ipd_port);
			return retcode;
		} else if (retcode < 0) {
			if (device_tree_dbg)
				cvmx_dprintf("%s: could not get phy info for port %d\n",
					     __func__, ipd_port);
			return retcode;
		}
		if (device_tree_dbg)
			cvmx_dprintf("%s: phy address: 0x%x\n",
				     __func__, phy_info.phy_addr);
		if (phy_info.phy_addr >= 0) {
			if (phy_info.direct_connect == 0) {
				if (device_tree_dbg)
					cvmx_dprintf("%s: Phy at address %d not directly connected, configuring mux\n",
						     __func__,
						     phy_info.phy_addr);
				__switch_mdio_mux(&phy_info);
			}
			if (device_tree_dbg)
				cvmx_dprintf("%s(0x%x): PHY address: 0x%x\n",
					     __func__, ipd_port,
					     phy_info.phy_addr);
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
 * @param phy_info PHY information
 *
 * @return link state information from PHY
 */
static cvmx_helper_link_info_t
__cvmx_get_cortina_phy_link_state(cvmx_phy_info_t *phy_info)
{
	cvmx_helper_link_info_t result;
	uint32_t phy_addr = phy_info->phy_addr;
	uint32_t bus = phy_addr >> 8;
	int value;
	uint32_t id;
	int slice_off;

	static const uint16_t cs4343_off[2][4] = {
		{ 0x3000, 0x2000, 0x1000, 0x0000 },
		{ 0x0000, 0x1000, 0x2000, 0x3000 }
	};
#define CS4XXX_GLOBAL_CHIPID_LSB			0x0000
#define CS4XXX_GLOBAL_CHIPID_MSB			0x0001
#define CS4321_GIGEPCS_LINE_STATUS			0xC01
#define CS4321_GPIO_GPIO_INTS				0x16D
#define CS4321_GLOBAL_GT_10KHZ_REF_CLK_CNT0		0x2E

#define CS4223_EFUSE_PDF_SKU				0x19F
#define CS4223_PP_LINE_SDS_DSP_MSEQ_STATUS		0x1237
#define CS4223_PP_LINE_SDS_DSP_MSEQ_DUPLEX_LOS_S	(1 << 0)
#define CS4223_PP_LINE_SDS_DSP_MSEQ_LOCAL_LOS_S		(1 << 1)
#define CS4223_PP_LINE_SDS_DSP_MSEQ_DUPLEX_LOCKD_S	(1 << 2)
#define CS4223_PP_LINE_SDS_DSP_MSEQ_LOCAL_LOCD_S	(1 << 3)
#define CS4223_PP_LINE_SDS_DSP_MSEQ_EXTERNAL_LOS_S	(1 << 4)
#define CS4223_PP_LINE_SDS_DSP_MSEQ_EDC_CONVERGED	(1 << 5)
#define CS4223_PP_HOST_SDS_DSP_MSEQ_STATUS		0x1A37
#define CS4223_PP_HOST_SDS_DSP_MSEQ_DUPLEX_LOS_S	(1 << 0)
#define CS4223_PP_HOST_SDS_DSP_MSEQ_LOCAL_LOS_S		(1 << 1)
#define CS4223_PP_HOST_SDS_DSP_MSEQ_DUPLEX_LOCKD_S	(1 << 2)
#define CS4223_PP_HOST_SDS_DSP_MSEQ_LOCAL_LOCD_S	(1 << 3)
#define CS4223_PP_HOST_SDS_DSP_MSEQ_EXTERNAL_LOS_S	(1 << 4)
#define CS4223_PP_HOST_SDS_DSP_MSEQ_EDC_CONVERGED	(1 << 5)

	result.u64 = 0;
	result.s.full_duplex = 1;

	phy_addr &= 0xff;

	/* Figure out which Cortina/Inphi PHY we're working with */
	value = cvmx_mdio_45_read(bus, phy_addr, 0, CS4XXX_GLOBAL_CHIPID_LSB);
	id = value;

	value = cvmx_mdio_45_read(bus, phy_addr, 0, CS4XXX_GLOBAL_CHIPID_MSB);
	id |= value << 16;
	/* Are we a CS4223/CS4343 PHY? */
	if ((id & 0x0FFFFFFF) == 0x000303e5) {
		/* For the CS4223 and CS4343 Cortina PHYs the link information
		 * is contained in one of the slices.  We use sub-addresses in
		 * order to calculate the appropriate register offset.
		 */
		int xiface;
		cvmx_helper_interface_mode_t mode;
		int sku;

		/* For XLAUI/40G KR mode we need to check the link of all four
		 * slices.  If any one of them is down then the entire link is
		 * down.
		 */
		xiface = cvmx_helper_get_interface_num(phy_info->ipd_port);
		mode = cvmx_helper_interface_get_mode(xiface);
		if ((mode == CVMX_HELPER_INTERFACE_MODE_XLAUI) ||
		    (mode == CVMX_HELPER_INTERFACE_MODE_40G_KR4)) {
			result.s.link_up = 1;
			for (slice_off = 0; slice_off < 0x4000;
			     slice_off += 0x1000) {
				value = cvmx_mdio_45_read(bus, phy_addr, 0,
							  CS4223_PP_LINE_SDS_DSP_MSEQ_STATUS + slice_off);
				if (!(value & CS4223_PP_LINE_SDS_DSP_MSEQ_EDC_CONVERGED))
					result.s.link_up = 0;
				value = cvmx_mdio_45_read(bus, phy_addr, 0,
							  CS4223_PP_HOST_SDS_DSP_MSEQ_STATUS + slice_off);
				if (!(value & CS4223_PP_HOST_SDS_DSP_MSEQ_EDC_CONVERGED))
					result.s.link_up = 0;
			}
			return result;
		}

		/* Note that the offsets get weird for the CS4343 */
		sku = cvmx_mdio_45_read(bus, phy_addr, 0, CS4223_EFUSE_PDF_SKU);
		if (sku == 0x12)	/* CS4343 */
			slice_off =
				cs4343_off[phy_addr & 1][phy_info->phy_sub_addr];
		else
			slice_off = phy_info->phy_sub_addr * 0x1000;

		/* For now we only support 40Gbps for this device. */
		value = cvmx_mdio_45_read(bus, phy_addr, 0,
					  CS4223_PP_LINE_SDS_DSP_MSEQ_STATUS + slice_off);
		result.s.link_up =
			!!(value & CS4223_PP_LINE_SDS_DSP_MSEQ_EDC_CONVERGED);
		value = cvmx_mdio_45_read(bus, phy_addr, 0,
					  CS4223_PP_HOST_SDS_DSP_MSEQ_STATUS + slice_off);
		if (!(value & CS4223_PP_HOST_SDS_DSP_MSEQ_EDC_CONVERGED))
			result.s.link_up = 0;
		return result;
	}

	/* If we're here we are dealing with a Cortina CS4318 type PHY.
	 *
	 * Right now to determine the speed we look at the reference clock.
	 * There's probably a better way but this should work.  For SGMII
	 * we use a 100MHz reference clock, for XAUI/RXAUI we use a 156.25MHz
	 * reference clock.
	 */
	value = cvmx_mdio_45_read(bus, phy_addr, 0,
				  CS4321_GLOBAL_GT_10KHZ_REF_CLK_CNT0);
	result.s.speed = (value == 10000) ? 1000 : 10000;

	if (result.s.speed == 1000) {
		value = cvmx_mdio_45_read(bus, phy_addr, 0,
					  CS4321_GIGEPCS_LINE_STATUS);
		result.s.link_up = (value >= 0) && !!(value & 4);
	} else {
		value = cvmx_mdio_45_read(bus, phy_addr, 0,
					  CS4321_GPIO_GPIO_INTS);
		result.s.link_up = (value >= 0) && !(value & 3);
	}

	return result;
}

/**
 * @INTERNAL
 * Get link state of generic C45 compliant PHYs
 */
static cvmx_helper_link_info_t
__get_generic_8023_c45_phy_link_state(cvmx_phy_info_t *phy_info)
{
	cvmx_helper_link_info_t result;
	int phy_status;
	int pma_ctrl1;
	uint32_t phy_addr = phy_info->phy_addr;

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

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
/**
 * @INTERNAL
 * Get link state of generic C45 compliant PHYs
 */
static cvmx_helper_link_info_t
__get_vitesse_vsc8490_phy_link_state(cvmx_phy_info_t *phy_info)
{
	cvmx_helper_link_info_t result;
	int phy_status;
	uint32_t phy_addr = phy_info->phy_addr;
	int xiface;
	cvmx_helper_interface_mode_t mode;

	xiface = cvmx_helper_get_interface_num(phy_info->ipd_port);
	mode = cvmx_helper_interface_get_mode(xiface);

	/* For 10G just use the generic 10G support */
	if (mode == CVMX_HELPER_INTERFACE_MODE_XAUI ||
	    mode == CVMX_HELPER_INTERFACE_MODE_RXAUI)
		return __get_generic_8023_c45_phy_link_state(phy_info);

	phy_status = cvmx_mdio_45_read(phy_addr >> 8, phy_addr & 0xff,
				       3, 0xe10d);

	result.u64 = 0;
	if ((phy_status & 0x111) != 0x111)
		return result;

	result.s.speed = 1000;
	result.s.full_duplex = 1;
	result.s.link_up = 1;

	return result;
}

/**
 * @INTERNAL
 * Get link state of Aquantia PHY
 */
static cvmx_helper_link_info_t
__get_aquantia_phy_link_state(cvmx_phy_info_t *phy_info)
{
	cvmx_helper_link_info_t result;
	uint16_t val;
	uint32_t phy_addr = phy_info->phy_addr;
	int bus = phy_info->mdio_unit;
	uint8_t con_state;
	static const uint16_t speeds[8] =
		{ 10, 100, 1000, 10000, 2500, 5000, 0, 0 };

	result.u64 = 0;

	/* Read PMA Receive Vendor State 1, bit 0 indicates that the Rx link
	 * is good
	 */
	val = cvmx_mdio_45_read(bus, phy_addr, 0x1, 0xe800);
	result.s.link_up = !!(val & 1);

	if (!result.s.link_up)
		return result;

	/* See if we're in autonegotiation training mode */

	/* Read the Autonegation Reserved Vendor Status 1 register */
	val = cvmx_mdio_45_read(bus, phy_addr, 7, 0xc810);
	/* Get the connection state, State 4 = connected */
	con_state = (val >> 9) & 0x1f;
	if (con_state == 2 || con_state == 3 || con_state == 0xa) {
		/* We're in autonegotiation mode so pretend the link is down */
		result.s.link_up = 0;
		return result;
	}

	result.s.full_duplex = 1;
	if (con_state == 4) {
		val = cvmx_mdio_45_read(bus, phy_addr, 7, 0xc800);
		result.s.speed = speeds[(val >> 1) & 7];
	} else {
		result.s.speed = 1000;
	}
	return result;
}
#endif

/**
 * @INTERNAL
 * Get link state of marvell PHY
 */
static cvmx_helper_link_info_t
__get_marvell_phy_link_state(cvmx_phy_info_t *phy_info)
{
	cvmx_helper_link_info_t result;
	int phy_status;
	uint32_t phy_addr = phy_info->phy_addr;

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

	/* Link is up = Speed/Duplex Resolved + RT-Link Up + G-Link Up. */
	if ((phy_status & 0x0c08) == 0x0c08) {
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
 *
 * @param phy_info	PHY information
 */
static cvmx_helper_link_info_t
__get_broadcom_phy_link_state(cvmx_phy_info_t *phy_info)
{
	cvmx_helper_link_info_t result;
	uint32_t phy_addr = phy_info->phy_addr;
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
 * @param phy_info - information about the PHY
 *
 * @returns link status of the PHY
 */
static cvmx_helper_link_info_t
__cvmx_get_generic_8023_c22_phy_link_state(cvmx_phy_info_t *phy_info)
{
	cvmx_helper_link_info_t result;
	uint32_t phy_addr = phy_info->phy_addr;
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

static cvmx_helper_link_info_t
__cvmx_get_qualcomm_s17_phy_link_state(cvmx_phy_info_t *phy_info)
{
	cvmx_helper_link_info_t result;
	uint32_t phy_addr = phy_info->phy_addr;
	int phy_status;
	int auto_status;

	result.u64 = 0;

	phy_status = cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff, 17);

	/* If bit 11 isn't set see if autonegotiation is turned off
	 * (bit 12, reg 0).  The resolved bit doesn't get set properly when
	 * autonegotiation is off, so force it.
	 */
	if ((phy_status & (1 << 11)) == 0) {
		auto_status = cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff, 0);
		if ((auto_status & (1 << 12)) == 0)
			phy_status |= 1 << 11;

	}
	/* Only return a link if the PHY has finished autonegotiation and set
	 * the resolved bit (bit 11).
	 */
	if (phy_status & (1 << 11)) {
		result.s.link_up = 1;
		result.s.full_duplex = !!(phy_status & (1 << 13));
		switch ((phy_status >> 14) & 3) {
		case 0:		/* 10Mbps */
			result.s.speed = 10;
			break;
		case 1:		/* 100Mbps */
			result.s.speed = 100;
			break;
		case 2:		/* 1Gbps */
			result.s.speed = 1000;
			break;
		default:	/* Illegal */
			result.u64 = 0;
			break;
		}
	}
	if (device_tree_dbg)
		cvmx_dprintf("   link: %s, duplex: %s, speed: %d\n",
			     result.s.link_up ? "up" : "down",
			     result.s.full_duplex ? "full" : "half",
			     result.s.speed);
	return result;
}
#endif

/**
 * @INTERNAL
 * Get link state using inband status
 */
static cvmx_helper_link_info_t
__get_inband_link_state(int ipd_port)
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
static int __switch_mdio_mux(const cvmx_phy_info_t *phy_info)
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
				/*cvmx_dprintf("%s: Error: could not read old MUX value for port %d\n",
					     __func__, ipd_port); */
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
 * Updates any GPIO link LEDs if present
 *
 * @param xiface	Interface number
 * @param index		Port index
 * @param result	Link status result
 */
void __cvmx_update_link_led(int xiface, int index,
			    cvmx_helper_link_info_t result)
{
	struct cvmx_phy_gpio_leds *gpio_leds;
	uint64_t mask;
	int node;

	/* Set link status LEDs if present */
	gpio_leds = cvmx_helper_get_port_phy_leds(xiface, index);
	if (gpio_leds && gpio_leds->link_status_gpio >= 0) {
		node = gpio_leds->link_status_gpio >> 8;
		mask = 1ULL << (gpio_leds->link_status_gpio & 0xff);
		if (result.s.link_up) {
			if (gpio_leds->link_status_active_low)
				cvmx_gpio_clear_node(node, mask);
			else
				cvmx_gpio_set_node(node, mask);
		} else {
			if (gpio_leds->link_status_active_low)
				cvmx_gpio_set_node(node, mask);
			else
				cvmx_gpio_clear_node(node, mask);
		}
	}
}

/**
 * Update the RX activity LED for the specified interface and port index
 *
 * @param xiface	Interface number
 * @param index		Port index
 * @param check_time	Check if the time has expired
 */
void cvmx_update_rx_activity_led(int xiface, int index, bool check_time)
{
	struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
	uint64_t rx_packets;
	struct cvmx_phy_gpio_leds *leds = cvmx_helper_get_port_phy_leds(xiface,
									index);
	static uint64_t tm_interval;

	if (!leds)
		return;

	if (check_time) {
		uint64_t current_time;

		if (!tm_interval)
			tm_interval = leds->activity_poll_interval_ms * cvmx_clock_get_rate(CVMX_CLOCK_CORE) / 1000;
		current_time = cvmx_clock_get_count(CVMX_CLOCK_CORE);

		if (current_time < leds->last_activity_poll_time + tm_interval)
			return;
	}

	if (octeon_has_feature(OCTEON_FEATURE_BGX)) {
		rx_packets = cvmx_read_csr_node(xi.node,
						CVMX_BGXX_CMRX_RX_STAT0(index,
									xi.interface));
	} else if (octeon_has_feature(OCTEON_FEATURE_PKND)) {
		int pknd = cvmx_helper_get_pknd(xi.interface, index);
		rx_packets = cvmx_read_csr(CVMX_PIP_STAT_INB_PKTS_PKNDX(pknd));

	} else {
		int port_num = cvmx_helper_get_ipd_port(xiface, index);
		rx_packets = cvmx_read_csr(CVMX_PIP_STAT_INB_PKTSX(port_num));
	}

	if (rx_packets != leds->last_rx_count) {
		cvmx_gpio_set_node(leds->rx_activity_gpio >> 8,
				   1 << (leds->rx_activity_gpio & 0xff));
		cvmx_gpio_cfg_sel(leds->rx_activity_gpio >> 8,
				  leds->rx_activity_gpio,
				  0x10 + leds->rx_gpio_timer);
		/*cvmx_gpio_set_node(leds->rx_activity_gpio >> 8,
				   1 << (leds->rx_activity_gpio & 0xff));*/

	} else {
		cvmx_gpio_cfg_sel(leds->rx_activity_gpio >> 8,
				  leds->rx_activity_gpio, 0);
		cvmx_gpio_clear_node(leds->rx_activity_gpio >> 8,
				     1 << (leds->rx_activity_gpio & 0xff));
	}
	leds->last_rx_count = rx_packets;
	if (check_time)
		leds->last_activity_poll_time =
					cvmx_clock_get_count(CVMX_CLOCK_CORE);
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
	cvmx_phy_info_t *phy_info = NULL;
	cvmx_phy_info_t local_phy_info;
	int xiface = 0, index = 0;
	bool use_inband = false;
	struct cvmx_fdt_sfp_info *sfp_info;
	const int dbg = device_tree_dbg;

	result.u64 = 0;

	if (cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_SIM) {
		/* The simulator gives you a simulated 1Gbps full duplex link */
		result.s.link_up = 1;
		result.s.full_duplex = 1;
		result.s.speed = 1000;
		return result;
	}

	if (ipd_port >= 0) {
		xiface = cvmx_helper_get_interface_num(ipd_port);
		index = cvmx_helper_get_interface_index_num(ipd_port);
		if (!cvmx_helper_get_port_autonegotiation(xiface, index)) {
			result.s.link_up = 1;
			result.s.full_duplex = 1;
			switch (cvmx_helper_interface_get_mode(xiface)) {
			case CVMX_HELPER_INTERFACE_MODE_RGMII:
			case CVMX_HELPER_INTERFACE_MODE_GMII:
			case CVMX_HELPER_INTERFACE_MODE_SGMII:
			case CVMX_HELPER_INTERFACE_MODE_QSGMII:
			case CVMX_HELPER_INTERFACE_MODE_AGL:
			case CVMX_HELPER_INTERFACE_MODE_SPI:
				if (OCTEON_IS_MODEL(OCTEON_CN70XX)) {
					struct cvmx_xiface xi = cvmx_helper_xiface_to_node_interface(xiface);
					uint64_t gbaud = cvmx_qlm_get_gbaud_mhz(0);
					result.s.speed = gbaud * 8 / 10;
					if (cvmx_qlm_get_dlm_mode(0, xi.interface) == CVMX_QLM_MODE_SGMII)
						result.s.speed >>= 1;
					else
						result.s.speed >>= 2;
				} else {
					result.s.speed = 1000;
				}
				break;
			case CVMX_HELPER_INTERFACE_MODE_RXAUI:
			case CVMX_HELPER_INTERFACE_MODE_XAUI:
			case CVMX_HELPER_INTERFACE_MODE_10G_KR:
			case CVMX_HELPER_INTERFACE_MODE_XFI:
				result.s.speed = 10000;
				break;
			case CVMX_HELPER_INTERFACE_MODE_XLAUI:
			case CVMX_HELPER_INTERFACE_MODE_40G_KR4:
				result.s.speed = 40000;
				break;
			default:
				break;
			}
#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
			__cvmx_update_link_led(xiface, index, result);
			sfp_info = cvmx_helper_cfg_get_sfp_info(xiface, index);
				/* If the link is down or the link is up but we still register
				 * the module as being absent, re-check mod_abs.
				 */
			if (sfp_info && (!result.s.link_up ||
			     (result.s.link_up && sfp_info->last_mod_abs))) {
					if (dbg)
						cvmx_dprintf("%s(%d): checking mod_abs\n",
							     __func__, ipd_port);
					cvmx_sfp_check_mod_abs(sfp_info,
							       sfp_info->mod_abs_data);
				}
#endif
			return result;
		}
		phy_info = cvmx_helper_get_port_phy_info(xiface, index);
		if (!phy_info) {
			if (device_tree_dbg)
				cvmx_dprintf("%s: phy info not saved in config, allocating for 0x%x:%d\n",
					     __func__, xiface, index);

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
			phy_info = (cvmx_phy_info_t *)kmalloc(sizeof(*phy_info),
							      GFP_KERNEL);
#else
			phy_info = cvmx_bootmem_alloc(sizeof(*phy_info), 0);
#endif
			if (!phy_info) {
				cvmx_dprintf("%s: Out of memory\n", __func__);
				return result;
			}
			memset(phy_info, 0, sizeof(*phy_info));
			phy_info->phy_addr = -1;
			if (dbg)
				cvmx_dprintf("%s: Setting phy info for 0x%x:%d to %p\n",
					     __func__, xiface, index, phy_info);
				cvmx_helper_set_port_phy_info(xiface, index,
							      phy_info);
		}
	} else {
		/* For management ports we don't store the PHY information
		 * so we use a local copy instead.
		 */
		phy_info = &local_phy_info;
		memset(phy_info, 0, sizeof(*phy_info));
		phy_info->phy_addr = -1;
	}

	if (phy_info->phy_addr == -1) {
		if (octeon_has_feature(OCTEON_FEATURE_BGX)) {
			if (__cvmx_helper_78xx_parse_phy(phy_info, ipd_port)) {
				/*cvmx_dprintf("Error parsing PHY info for 78xx for ipd port %d\n",
					       ipd_port); */
				phy_info->phy_addr = -1;
				use_inband = true;
			}
		} else if (__get_phy_info_from_dt(phy_info, ipd_port) < 0) {
			phy_info->phy_addr = -1;
			use_inband = true;
		}
	}

	/* If we can't get the PHY info from the device tree then try
	 * the inband state.
	 */
	if (use_inband) {
		if (OCTEON_IS_OCTEON1() || OCTEON_IS_MODEL(OCTEON_CN58XX) ||
		    OCTEON_IS_MODEL(OCTEON_CN50XX)) {
			result = __get_inband_link_state(ipd_port);
		} else {
			result.s.full_duplex = 1;
			result.s.link_up = 1;
			result.s.speed = 1000;
		}
#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
		if (ipd_port >= 0)
			__cvmx_update_link_led(xiface, index, result);
#endif
		return result;
	}

	if (phy_info->phy_addr < 0)
		return result;

	if (phy_info->direct_connect == 0)
		__switch_mdio_mux(phy_info);

	if (phy_info->link_function)
		result = phy_info->link_function(phy_info);
	else if (OCTEON_IS_OCTEON1() ||
		 OCTEON_IS_MODEL(OCTEON_CN58XX) ||
		 OCTEON_IS_MODEL(OCTEON_CN50XX))
		/* We don't have a PHY address, so attempt to use
		 * in-band status. It is really important that boards
		 * not supporting in-band status never get
		 * here. Reading broken in-band status tends to do bad
		 * things.
		 */
		result = __get_inband_link_state(ipd_port);
	else
		result = cvmx_helper_link_get(ipd_port);

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	if (ipd_port >= 0)
		__cvmx_update_link_led(xiface, index, result);
#endif
	sfp_info = cvmx_helper_cfg_get_sfp_info(xiface, index);
	if (sfp_info)
		/* If the link is down or the link is up but we still register
		 * the module as being absent, re-check mod_abs.
		 */
		if (!result.s.link_up ||
		    (result.s.link_up && sfp_info->last_mod_abs)) {
			if (dbg)
				cvmx_dprintf("%s(%d): checking mod_abs\n",
					     __func__, ipd_port);
			cvmx_sfp_check_mod_abs(sfp_info,
					       sfp_info->mod_abs_data);
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
	struct cvmx_phy_info phy_info;
	int phy_addr;
	int is_broadcom_phy = 0;
	int is_vitesse_phy = 0;
	int is_cortina_phy = 0;
	int is_ti_phy;
	int xiface = 0, index = 0;

	if (ipd_port >= 0) {
		xiface = cvmx_helper_get_interface_num(ipd_port);
		index = cvmx_helper_get_interface_index_num(ipd_port);
	}

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	if (cvmx_sysinfo_get()->fdt_addr) {
		return __cvmx_helper_board_link_get_from_dt(ipd_port);
	}
#endif

	/* Give the user a chance to override the processing of this function */
	if (cvmx_override_board_link_get) {
		result = cvmx_override_board_link_get(ipd_port);
#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
		if (ipd_port >= 0)
			__cvmx_update_link_led(xiface, index, result);
#endif

		return result;
	}

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
		if (ipd_port == CVMX_HELPER_BOARD_MGMT_IPD_PORT) {
			is_broadcom_phy = 1;
		}
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
		is_ti_phy = 1;
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

	memset(&phy_info, 0, sizeof(phy_info));
	phy_info.phy_addr = -1;
	phy_info.ipd_port = ipd_port;
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
	phy_addr = cvmx_helper_board_get_mii_address(ipd_port);
	phy_info.phy_addr = phy_addr;
#else
	if (device_tree_dbg)
		cvmx_dprintf("%s: Getting PHY info from device tree for IPD port %d\n",
			     __func__, ipd_port);
	if (__get_phy_info_from_dt(&phy_info, ipd_port) != 0) {
		if (device_tree_dbg)
			cvmx_dprintf("DT failed, getting addr from board\n");
		phy_info.phy_addr = cvmx_helper_board_get_mii_address(ipd_port);
	}
	phy_addr = phy_info.phy_addr;
#endif
	if (phy_addr >= 0) {
		if (is_cortina_phy) {
			result = __cvmx_get_cortina_phy_link_state(&phy_info);
		} else if (is_broadcom_phy) {
			result = __get_broadcom_phy_link_state(&phy_info);
		} else if (is_vitesse_phy || is_ti_phy) {
			result = __get_generic_8023_c45_phy_link_state(&phy_info);
		} else {
			/* This code assumes we are using a Marvell Gigabit PHY. */
			result = __get_marvell_phy_link_state(&phy_info);
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

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	if (ipd_port >= 0)
		__cvmx_update_link_led(xiface, index, result);
#endif
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
int cvmx_helper_board_link_set_phy(int phy_addr,
				   cvmx_helper_board_set_phy_link_flags_types_t link_flags,
				   cvmx_helper_link_info_t link_info)
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

		reg_status.u16 = cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff,
						CVMX_MDIO_PHY_REG_STATUS);
		reg_autoneg_adver.u16 = cvmx_mdio_read(phy_addr >> 8,
						       phy_addr & 0xff,
							CVMX_MDIO_PHY_REG_AUTONEG_ADVER);
		reg_autoneg_adver.s.advert_100base_t4 = reg_status.s.capable_100base_t4;
		reg_autoneg_adver.s.advert_10base_tx_full = reg_status.s.capable_10_full;
		reg_autoneg_adver.s.advert_10base_tx_half = reg_status.s.capable_10_half;
		reg_autoneg_adver.s.advert_100base_tx_full = reg_status.s.capable_100base_x_full;
		reg_autoneg_adver.s.advert_100base_tx_half = reg_status.s.capable_100base_x_half;
		cvmx_mdio_write(phy_addr >> 8, phy_addr & 0xff,
				CVMX_MDIO_PHY_REG_AUTONEG_ADVER,
				reg_autoneg_adver.u16);
		if (reg_status.s.capable_extended_status) {
			reg_extended_status.u16 = cvmx_mdio_read(phy_addr >> 8,
								 phy_addr & 0xff,
								 CVMX_MDIO_PHY_REG_EXTENDED_STATUS);
			reg_control_1000.u16 = cvmx_mdio_read(phy_addr >> 8,
							      phy_addr & 0xff,
							      CVMX_MDIO_PHY_REG_CONTROL_1000);
			reg_control_1000.s.advert_1000base_t_full = reg_extended_status.s.capable_1000base_t_full;
			reg_control_1000.s.advert_1000base_t_half = reg_extended_status.s.capable_1000base_t_half;
			cvmx_mdio_write(phy_addr >> 8, phy_addr & 0xff,
					CVMX_MDIO_PHY_REG_CONTROL_1000,
					reg_control_1000.u16);
		}
		reg_control.u16 = cvmx_mdio_read(phy_addr >> 8,
						 phy_addr & 0xff,
						 CVMX_MDIO_PHY_REG_CONTROL);
		reg_control.s.reset = 1;
		reg_control.s.autoneg_enable = 1;
		reg_control.s.restart_autoneg = 1;
		cvmx_mdio_write(phy_addr >> 8, phy_addr & 0xff,
				CVMX_MDIO_PHY_REG_CONTROL, reg_control.u16);
	} else if ((link_flags & set_phy_link_flags_autoneg)) {
		cvmx_mdio_phy_reg_control_t reg_control;
		cvmx_mdio_phy_reg_status_t reg_status;
		cvmx_mdio_phy_reg_autoneg_adver_t reg_autoneg_adver;
		cvmx_mdio_phy_reg_control_1000_t reg_control_1000;

		reg_status.u16 = cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff,
						CVMX_MDIO_PHY_REG_STATUS);
		reg_autoneg_adver.u16 = cvmx_mdio_read(phy_addr >> 8,
						       phy_addr & 0xff,
						       CVMX_MDIO_PHY_REG_AUTONEG_ADVER);
		reg_autoneg_adver.s.advert_100base_t4 = 0;
		reg_autoneg_adver.s.advert_10base_tx_full = 0;
		reg_autoneg_adver.s.advert_10base_tx_half = 0;
		reg_autoneg_adver.s.advert_100base_tx_full = 0;
		reg_autoneg_adver.s.advert_100base_tx_half = 0;
		if (reg_status.s.capable_extended_status) {
			reg_control_1000.u16 = cvmx_mdio_read(phy_addr >> 8,
							      phy_addr & 0xff,
							      CVMX_MDIO_PHY_REG_CONTROL_1000);
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
		cvmx_mdio_write(phy_addr >> 8, phy_addr & 0xff,
				CVMX_MDIO_PHY_REG_AUTONEG_ADVER,
				reg_autoneg_adver.u16);
		if (reg_status.s.capable_extended_status)
			cvmx_mdio_write(phy_addr >> 8, phy_addr & 0xff,
					CVMX_MDIO_PHY_REG_CONTROL_1000,
					reg_control_1000.u16);
		reg_control.u16 = cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff,
						 CVMX_MDIO_PHY_REG_CONTROL);
		reg_control.s.reset = 1;
		reg_control.s.autoneg_enable = 1;
		reg_control.s.restart_autoneg = 1;
		cvmx_mdio_write(phy_addr >> 8, phy_addr & 0xff,
				CVMX_MDIO_PHY_REG_CONTROL, reg_control.u16);
	} else {
		cvmx_mdio_phy_reg_control_t reg_control;
		reg_control.u16 = cvmx_mdio_read(phy_addr >> 8, phy_addr & 0xff,
						 CVMX_MDIO_PHY_REG_CONTROL);
		reg_control.s.reset = 1;
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
		cvmx_mdio_write(phy_addr >> 8, phy_addr & 0xff,
				CVMX_MDIO_PHY_REG_CONTROL, reg_control.u16);
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
EXPORT_SYMBOL(__cvmx_helper_board_usb_get_clock_type);
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
EXPORT_SYMBOL(__cvmx_helper_board_usb_get_num_ports);
