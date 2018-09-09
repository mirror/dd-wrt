/***********************license start***************
 * Copyright (c) 2014  Cavium Inc. (support@cavium.com). All rights
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
 * FDT Helper functions similar to those provided to U-Boot.
 *
 */
#if defined(CVMX_BUILD_FOR_LINUX_KERNEL)
# error This file should not be used with Linux!
#endif

#ifdef __U_BOOT__
# include <asm/arch/cvmx.h>
# include <asm/arch/cvmx-helper-fdt.h>
#else
# include "../cvmx.h"
# include "cvmx-helper-fdt.h"
#endif

#ifndef __U_BOOT__
/**
 * Helper to return the address property
 *
 * @param[in] fdt_addr	pointer to FDT blob
 * @param node		node to read address from
 * @param[in] prop_name	property name to read
 *
 * @return address of property or FDT_ADDR_T_NONE if not found
 */
uint64_t cvmx_fdt_get_addr(const void *fdt_addr, int node,
			   const char *prop_name)
{
	const uint64_t *pval;
	int len;

	pval = fdt_getprop(fdt_addr, node, prop_name, &len);
	if (pval && ((len == sizeof(uint64_t)) ||
		     (len == sizeof(uint64_t) * 2)))
		return fdt64_to_cpu(*pval);
	else
		return FDT_ADDR_T_NONE;
}

/**
 * Helper function to return an integer property
 *
 * @param[in] fdt_addr	pointer to FDT blob
 * @param node		node to read integer from
 * @param[in] prop_name	property name to read
 * @param default_val	default value to return if property doesn't exist
 *
 * @return	integer value of property or default_val if it doesn't exist.
 */
int cvmx_fdt_get_int(const void *fdt_addr, int node, const char *prop_name,
		     int default_val)
{
	const uint32_t *pval;
	int len;

	pval = fdt_getprop(fdt_addr, node, prop_name, &len);
	if (pval && (unsigned)len >= sizeof(int))
		return fdt32_to_cpu(*pval);
	else
		return default_val;
}

/**
 * Helper function to return a uint64_t property
 *
 * @param[in] fdt_addr	pointer to FDT blob
 * @param node		node to read integer from
 * @param[in] prop_name	property name to read
 * @param default_val	default value to return if property doesn't exist
 *
 * @return	uint64_t value of property or default_val if it doesn't exist.
 */
uint64_t cvmx_fdt_get_uint64(const void *fdt_addr, int node,
			     const char *prop_name, uint64_t default_val)
{
	const uint64_t *pval;
	int len;

	pval = fdt_getprop(fdt_addr, node, prop_name, &len);
	if (pval && (unsigned)len >= sizeof(uint64_t))
		return fdt64_to_cpu(*pval);
	else
		return default_val;
}

/**
 * Look up a phandle and follow it to its node then return the offset of that
 * node.
 *
 * @param[in] fdt_addr	pointer to FDT blob
 * @param node		node to read phandle from
 * @param[in] prop_name	name of property to find
 *
 * @return	node offset if found, -ve error code on error
 */
int cvmx_fdt_lookup_phandle(const void *fdt_addr, int node,
			    const char *prop_name)
{
	const uint32_t *phandle;

	phandle = fdt_getprop(fdt_addr, node, prop_name, NULL);
	if (phandle)
		return fdt_node_offset_by_phandle(fdt_addr,
						  fdt32_to_cpu(*phandle));
	else
		return -FDT_ERR_NOTFOUND;
}

static inline uint64_t __cvmx_fdt_read_num(const uint32_t *addr, int num_addr)
{
	if (num_addr == 2)
		return ((uint64_t)fdt32_to_cpu(*addr) << 32) |
			fdt32_to_cpu(*(addr + 1));
	else
		return fdt32_to_cpu(*addr);
}

static uint64_t cvmx_fdt_bus_map(const uint32_t *addr,
				 const uint32_t *range,
				 int num_addr, int num_size, int pna)
{
	uint64_t cp, s, da;

	cp = __cvmx_fdt_read_num(range, num_addr);
	range += num_addr;
	range += pna;
	s = __cvmx_fdt_read_num(range, num_size);
	da = __cvmx_fdt_read_num(addr, num_addr);

	if ((da < cp) || da >= (cp + s))
		return FDT_ADDR_T_NONE;

	return da - cp;
}

static void cvmx_fdt_bus_translate(uint32_t *addr, uint64_t offset,
				   int num_addr)
{
	uint64_t ba = __cvmx_fdt_read_num(addr, num_addr);
	memset(addr, 0, num_addr * sizeof(*addr));
	ba += offset;
	if (num_addr > 1)
		addr[num_addr - 2] = cpu_to_fdt32(ba >> 32);
	addr[num_addr - 1] = cpu_to_fdt32(ba & 0xffffffff);
}

/**
 * Outputs the number of address and size cells of a node
 *
 * @param[in] fdt_addr		Address of flat device tree
 * @param parent		node address of parent
 * @param[out] num_addr_cells	Number of address cells (NULL for none)
 * @param[out] num_size_cells	Number of size cells (NULL for none)
 */
static void cvmx_fdt_count_cells(const void *fdt_addr, int parent,
				 int *num_addr_cells, int *num_size_cells)
{
	const uint32_t *val;

	if (num_addr_cells) {
		val = fdt_getprop(fdt_addr, parent, "#address-cells", NULL);
		if (val)
			*num_addr_cells = fdt32_to_cpu(*val);
		else
			*num_addr_cells = 2;
	}
	if (num_size_cells) {
		val = fdt_getprop(fdt_addr, parent, "#size-cells", NULL);
		if (val)
			*num_size_cells = fdt32_to_cpu(*val);
		else
			*num_size_cells = 1;
	}
}

int __cvmx_fdt_translate_one(const void *fdt_addr, int parent, uint32_t *in_addr,
			     int num_addr, int num_size, int p_num_addr,
			     const char *prop_name)
{
	const uint32_t *ranges;
	int rlen;
	int rone;
	uint64_t offset = FDT_ADDR_T_NONE;

	ranges = fdt_getprop(fdt_addr, parent, prop_name, &rlen);
	/* If no ranges don't bother translating */
	if (!ranges || !rlen) {
		offset = __cvmx_fdt_read_num(in_addr, num_addr);
		memset(in_addr, 0, p_num_addr * sizeof(uint32_t));
		cvmx_fdt_bus_translate(in_addr, offset, p_num_addr);
		return 0;
	}

	/* Walk through the ranges */
	rlen /= sizeof(uint32_t);
	rone = num_addr + p_num_addr + num_size;

	/* Walk through the ranges */
	do {
		offset = cvmx_fdt_bus_map(in_addr, ranges,
					  num_addr, num_size, p_num_addr);
		if (offset == FDT_ADDR_T_NONE)
			return 1;
		rlen -= rone;
		ranges += rone;
	} while (rlen >= rone);

	memcpy(in_addr, ranges + num_addr, p_num_addr * sizeof(uint32_t));

	/* Translate to the parent bus space */
	cvmx_fdt_bus_translate(in_addr, offset, p_num_addr);
	return 0;
}

/**
 * Translate an address from the device tree into a CPU physical address by
 * walking up the device tree and applying bus mappings along the way.
 *
 * This uses #size-cells and #address-cells.
 *
 * @param[in]	fdt_addr	Address of flat device tree
 * @param	node		node to start translating from
 * @param	in_addr		Address to translate
 *
 * @return	Translated address or FDT_ADDR_T_NONE if address cannot be
 * 		translated.
 */
uint64_t cvmx_fdt_translate_address(const void *fdt_addr, int node,
				    const uint32_t *in_addr)
{
	uint32_t addr[4];
	int parent;
	int num_addr, num_size;
	int p_num_addr, p_num_size;

	parent = fdt_parent_offset(fdt_addr, node);
	if (parent < 0) {
		cvmx_dprintf("%s: No parent node!\n", __func__);
		return FDT_ADDR_T_NONE;
	}

	cvmx_fdt_count_cells(fdt_addr, parent, &num_addr, &num_size);

	if (num_addr < 0 || num_addr > 4 || num_size < 0) {
		cvmx_dprintf("%s: Bad cell count for %s\n", __func__,
			     fdt_get_name(fdt_addr, node, NULL));
		return FDT_ADDR_T_NONE;
	}

	memcpy(addr, in_addr, num_addr * sizeof(uint32_t));

	/* Translate */
	do {
		node = parent;
		parent = fdt_parent_offset(fdt_addr, node);

		/* Check if root */
		if (parent < 0)
			return __cvmx_fdt_read_num(addr, num_addr);

		cvmx_fdt_count_cells(fdt_addr, parent, &p_num_addr, &p_num_size);
		if (p_num_addr < 0 || p_num_addr > 4 || p_num_size < 0) {
			cvmx_dprintf("%s: Bad cell count for %s\n", __func__,
				     fdt_get_name(fdt_addr, node, NULL));
			return FDT_ADDR_T_NONE;
		}
		if (__cvmx_fdt_translate_one(fdt_addr, node, addr,
					     num_addr, num_size, p_num_addr,
					     "ranges"))
			return FDT_ADDR_T_NONE;
		num_addr = p_num_addr;
		num_size = p_num_size;
	} while (1);
}
#endif /* !defined(__U_BOOT__) */

/** Structure used to get type of GPIO from device tree */
struct gpio_compat {
	char *compatible;		/** Compatible string */
	enum cvmx_gpio_type type;	/** Type */
	int8_t size;			/** (max) Number of pins */
};

/**
 * List of GPIO types
 */
static const struct gpio_compat gpio_compat_list[] = {
	{ "cavium,octeon-3860-gpio", CVMX_GPIO_PIN_OCTEON, 20 },

	{ "nxp,pca9505",	CVMX_GPIO_PIN_PCA953X, 40 },
	{ "nxp,pca9534",	CVMX_GPIO_PIN_PCA953X, 8 },
	{ "nxp,pca9535",	CVMX_GPIO_PIN_PCA953X, 16 },
	{ "nxp,pca9536",	CVMX_GPIO_PIN_PCA953X, 4 },
	{ "nxp,pca9537",	CVMX_GPIO_PIN_PCA953X, 4 },
	{ "nxp,pca9538",	CVMX_GPIO_PIN_PCA953X, 8 },
	{ "nxp,pca9539",	CVMX_GPIO_PIN_PCA953X, 16 },
	{ "nxp,pca9554",	CVMX_GPIO_PIN_PCA953X, 8 },
	{ "nxp,pca9554a",	CVMX_GPIO_PIN_PCA953X, 8 },
	{ "nxp,pca9555",	CVMX_GPIO_PIN_PCA953X, 16 },
	{ "nxp,pca9555a",	CVMX_GPIO_PIN_PCA953X, 16 },
	{ "nxp,pca9557",	CVMX_GPIO_PIN_PCA953X, 8 },
	{ "nxp,pca9574",	CVMX_GPIO_PIN_PCA953X, 8 },
	{ "nxp,pca9575",	CVMX_GPIO_PIN_PCA953X, 16 },
	{ "maxim,max7310",	CVMX_GPIO_PIN_PCA953X, 8 },
	{ "maxim,max7312",	CVMX_GPIO_PIN_PCA953X, 16 },
	{ "maxim,max7313",	CVMX_GPIO_PIN_PCA953X, 16 },
	{ "maxim,max7315",	CVMX_GPIO_PIN_PCA953X, 8 },
	{ "ti,pca6107",		CVMX_GPIO_PIN_PCA953X, 8 },
	{ "ti,tca6408",		CVMX_GPIO_PIN_PCA953X, 8 },
	{ "ti,tca6416",		CVMX_GPIO_PIN_PCA953X, 16 },
	{ "ti,tca6424",		CVMX_GPIO_PIN_PCA953X, 24 },

	{ "nxp,pcf8574",	CVMX_GPIO_PIN_PCF857X, 8 },
	{ "nxp,pcf8574a",	CVMX_GPIO_PIN_PCF857X, 8 },
	{ "nxp,pc8574",		CVMX_GPIO_PIN_PCF857X, 8 },
	{ "nxp,pca9670",	CVMX_GPIO_PIN_PCF857X, 8 },
	{ "nxp,pca9672",	CVMX_GPIO_PIN_PCF857X, 8 },
	{ "nxp,pca9674",	CVMX_GPIO_PIN_PCF857X, 8 },
	{ "nxp,pca8575",	CVMX_GPIO_PIN_PCF857X, 16 },
	{ "nxp,pcf8575",	CVMX_GPIO_PIN_PCF857X, 16 },
	{ "nxp,pca9671",	CVMX_GPIO_PIN_PCF857X, 16 },
	{ "nxp,pca9673",	CVMX_GPIO_PIN_PCF857X, 16 },
	{ "nxp,pca9675",	CVMX_GPIO_PIN_PCF857X, 16 },
	{ "maxim,max7328",	CVMX_GPIO_PIN_PCF857X, 8 },
	{ "maxim,max7329",	CVMX_GPIO_PIN_PCF857X, 8 },
	{ "ti,tca9554",		CVMX_GPIO_PIN_PCF857X, 8 },
};

/**
 * Given a phandle to a GPIO device return the type of GPIO device it is.
 *
 * @param[in]	fdt_addr	Address of flat device tree
 * @param	phandle		phandle to GPIO
 * @param[out]	size		Number of pins (optional, may be NULL)
 *
 * @return	Type of GPIO device or PIN_ERROR if error
 */
enum cvmx_gpio_type cvmx_fdt_get_gpio_type(const void *fdt_addr, int phandle,
					   int *size)
{
	int node = fdt_node_offset_by_phandle(fdt_addr, phandle);
	int i;

	for (i = 0; i < (int)(sizeof(gpio_compat_list)/sizeof(gpio_compat_list[0])); i++) {
		if (!fdt_node_check_compatible(fdt_addr, node,
					       gpio_compat_list[i].compatible)) {
			if (size)
				*size = gpio_compat_list[i].size;
			return gpio_compat_list[i].type;
		}
	}

	return CVMX_GPIO_PIN_OTHER;
}

/**
 * Given a phandle to a GPIO node output the i2c bus and address
 *
 * @param[in]	fdt_addr	Address of FDT
 * @param	phandle		phandle of GPIO device
 * @param[out]	bus		TWSI bus number with node in bits 1-3, can be
 * 				NULL for none.
 * @param[out]	addr		TWSI address number, can be NULL for none
 *
 * @return	0 for success, error otherwise
 */
int cvmx_fdt_get_twsi_gpio_bus_addr(const void *fdt_addr, int phandle,
				    int *bus, int *addr)
{
	int node = fdt_node_offset_by_phandle(fdt_addr, phandle);
	int parent = fdt_parent_offset(fdt_addr, node);
	int reg;
	uint64_t addr64;

	if (node < 0 || parent < 0) {
		cvmx_dprintf("%s: Invalid phandle 0x%x\n", __func__, phandle);
		return -1;
	}

	if (fdt_node_check_compatible(fdt_addr, parent,
				      "cavium,octeon-3860-twsi")) {
		cvmx_dprintf("%s: Incompatible parent, must be native OCTEON i2c/twsi bus\n",
			     __func__);
		return -1;
	}

	if (addr) {
		reg = cvmx_fdt_get_int(fdt_addr, node, "reg", -1);
		if (reg < 0)
			return reg;
		*addr = reg;
	}
	if (bus) {
		addr64 = cvmx_fdt_get_addr(fdt_addr, parent, "reg");
		addr64 = cvmx_fdt_translate_address(fdt_addr, parent,
						    (uint32_t *)&addr64);
		if (addr64 == FDT_ADDR_T_NONE) {
			cvmx_dprintf("Cannot get TWSI node address\n");
			return -1;
		}

		switch (addr64) {
		case 0x1180000001000:
			*bus = 0;
			break;
		case 0x1180000001200:
			*bus = 1;
			break;
		default:
			cvmx_dprintf("Error: unknown TWSI node address 0x%llx\n",
				     (unsigned long long)addr64);
			return -1;
		}
	}
	return 0;
}
