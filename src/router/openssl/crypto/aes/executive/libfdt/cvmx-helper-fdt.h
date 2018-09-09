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
 * If compiled for U-Boot, just provide wrappers to the equivalent U-Boot
 * functions.
 */

#ifndef __CVMX_HELPER_FDT_H__
#ifdef __U_BOOT__
# include <libfdt.h>
# include <fdtdec.h>
#else
# include "libfdt.h"

typedef uint64_t fdt_addr_t;
typedef uint64_t fdt_size_t;

# define FDT_ADDR_T_NONE (-1ULL)
#endif

/** Type of GPIO pin */
enum cvmx_gpio_type {
	CVMX_GPIO_PIN_OCTEON,	/** GPIO pin is directly connected to OCTEON */
	CVMX_GPIO_PIN_PCA953X,	/** GPIO pin is NXP PCA953X compat chip */
	CVMX_GPIO_PIN_PCF857X,	/** GPIO pin is NXP PCF857X compat chip */
	CVMX_GPIO_PIN_OTHER,	/** GPIO pin is something else */
};

#ifdef __U_BOOT__

/**
 * Helper to return the address property
 *
 * @param[in] fdt_addr	pointer to FDT blob
 * @param node		node to read address from
 * @param prop_name	property name to read
 *
 * @return address of property or FDT_ADDR_T_NONE if not found
 */
static inline fdt_addr_t cvmx_fdt_get_addr(const void *fdt_addr, int node,
					   const char *prop_name)
{
	return fdtdec_get_addr(fdt_addr, node, prop_name);
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
static inline int cvmx_fdt_get_int(const void *fdt_addr, int node,
				   const char *prop_name, int default_val)
{
	return fdtdec_get_int(fdt_addr, node, prop_name, default_val);
}

static inline uint64_t cvmx_fdt_get_uint64(const void *fdt_addr, int node,
					   const char *prop_name,
					   uint64_t default_val)
{
	return fdtdec_get_uint64(fdt_addr, node, prop_name, default_val);
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
static inline int cvmx_fdt_lookup_phandle(const void *fdt_addr, int node,
					  const char *prop_name)
{
	return fdtdec_lookup_phandle(fdt_addr, node, prop_name);
}

/**
 * Translate an address from the device tree into a CPU physical address by
 * walking up the device tree and applying bus mappings along the way.
 *
 * This uses #size-cells and #address-cells.
 *
 * @param[in]	fdt_addr	Address of flat device tree
 * @param	node		node to start translating from
 * @param[in]	in_addr		Address to translate
 *
 * @return	Translated address or FDT_ADDR_T_NONE if address cannot be
 * 		translated.
 */
static inline uint64_t cvmx_fdt_translate_address(const void *fdt_addr,
						  int node,
						  const uint32_t *in_addr)
{
	return fdt_translate_address(fdt_addr, node, in_addr);
}

#else
/**
 * Helper to return the address property
 *
 * @param[in] fdt_addr	pointer to FDT blob
 * @param node		node to read address from
 * @param prop_name	property name to read
 *
 * @return address of property or FDT_ADDR_T_NONE if not found
 */
uint64_t cvmx_fdt_get_addr(const void *fdt_addr, int node,
			   const char *prop_name);

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
		     int default_val);

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
			     const char *prop_name, uint64_t default_val);

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
			    const char *prop_name);

/**
 * Translate an address from the device tree into a CPU physical address by
 * walking up the device tree and applying bus mappings along the way.
 *
 * This uses #size-cells and #address-cells.
 *
 * @param[in]	fdt_addr	Address of flat device tree
 * @param	node		node to start translating from
 * @param[in]	in_addr		Address to translate
 *
 * @return	Translated address or FDT_ADDR_T_NONE if address cannot be
 * 		translated.
 */
uint64_t cvmx_fdt_translate_address(const void *fdt_addr, int node,
				    const uint32_t *in_addr);
#endif

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
					   int *size);

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
				    int *bus, int *addr);

#endif /* CVMX_HELPER_FDT_H__ */
