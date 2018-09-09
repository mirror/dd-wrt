/***********************license start***************
 * Copyright (c) 2011-2013  Cavium Inc. (support@cavium.com). All rights
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
 * Helper utilities for qlm.
 *
 * <hr>$Revision: 100238 $<hr>
 */
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-bootmem.h>
#include <asm/octeon/cvmx-helper-jtag.h>
#include <asm/octeon/cvmx-qlm.h>
#include <asm/octeon/cvmx-clock.h>
#include <asm/octeon/cvmx-gmxx-defs.h>
#include <asm/octeon/cvmx-gserx-defs.h>
#include <asm/octeon/cvmx-sriox-defs.h>
#include <asm/octeon/cvmx-sriomaintx-defs.h>
#include <asm/octeon/cvmx-pciercx-defs.h>
#include <asm/octeon/cvmx-pemx-defs.h>
#elif defined(CVMX_BUILD_FOR_UBOOT)
#include <common.h>
#include <asm/arch/cvmx.h>
#include <asm/arch/cvmx-bootmem.h>
#include <asm/arch/cvmx-helper-jtag.h>
#include <asm/arch/cvmx-qlm.h>
#else
#include "cvmx.h"
#include "cvmx-bootmem.h"
#include "cvmx-helper-jtag.h"
#include "cvmx-qlm.h"
#endif

#ifdef CVMX_BUILD_FOR_UBOOT
DECLARE_GLOBAL_DATA_PTR;
#endif

/**
 * The JTAG chain for CN52XX and CN56XX is 4 * 268 bits long, or 1072.
 * CN5XXX full chain shift is:
 *     new data => lane 3 => lane 2 => lane 1 => lane 0 => data out
 * The JTAG chain for CN63XX is 4 * 300 bits long, or 1200.
 * The JTAG chain for CN68XX is 4 * 304 bits long, or 1216.
 * The JTAG chain for CN66XX/CN61XX/CNF71XX is 4 * 304 bits long, or 1216.
 * CN6XXX full chain shift is:
 *     new data => lane 0 => lane 1 => lane 2 => lane 3 => data out
 * Shift LSB first, get LSB out
 */
#ifndef _MIPS_ARCH_OCTEON2
extern const __cvmx_qlm_jtag_field_t __cvmx_qlm_jtag_field_cn52xx[];
extern const __cvmx_qlm_jtag_field_t __cvmx_qlm_jtag_field_cn56xx[];
#endif
extern const __cvmx_qlm_jtag_field_t __cvmx_qlm_jtag_field_cn63xx[];
extern const __cvmx_qlm_jtag_field_t __cvmx_qlm_jtag_field_cn66xx[];
extern const __cvmx_qlm_jtag_field_t __cvmx_qlm_jtag_field_cn68xx[];

#define CVMX_QLM_JTAG_UINT32 40
#ifdef CVMX_BUILD_FOR_LINUX_HOST
extern void octeon_remote_read_mem(void *buffer, uint64_t physical_address, int length);
extern void octeon_remote_write_mem(uint64_t physical_address, const void *buffer, int length);
uint32_t __cvmx_qlm_jtag_xor_ref[5][CVMX_QLM_JTAG_UINT32];
#else
typedef uint32_t qlm_jtag_uint32_t[CVMX_QLM_JTAG_UINT32];
CVMX_SHARED qlm_jtag_uint32_t *__cvmx_qlm_jtag_xor_ref;
#endif

/**
 * Return the number of QLMs supported by the chip
 *
 * @return  Number of QLMs
 */
int cvmx_qlm_get_num(void)
{
	if (OCTEON_IS_MODEL(OCTEON_CN68XX))
		return 5;
	else if (OCTEON_IS_MODEL(OCTEON_CN66XX))
		return 3;
	else if (OCTEON_IS_MODEL(OCTEON_CN63XX))
		return 3;
	else if (OCTEON_IS_MODEL(OCTEON_CN61XX))
		return 3;
#ifndef _MIPS_ARCH_OCTEON2
	else if (OCTEON_IS_MODEL(OCTEON_CN56XX))
		return 4;
	else if (OCTEON_IS_MODEL(OCTEON_CN52XX))
		return 2;
#endif
	else if (OCTEON_IS_MODEL(OCTEON_CNF71XX))
		return 2;
	//cvmx_dprintf("Warning: cvmx_qlm_get_num: This chip does not have QLMs\n");
	return 0;
}

/**
 * Return the qlm number based on the interface
 *
 * @param interface  Interface to look up
 */
int cvmx_qlm_interface(int interface)
{
	if (OCTEON_IS_MODEL(OCTEON_CN61XX)) {
		return (interface == 0) ? 2 : 0;
	} else if (OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX)) {
		return 2 - interface;
	} else if (OCTEON_IS_MODEL(OCTEON_CNF71XX)) {
		if (interface == 0)
			return 0;
		else
			cvmx_dprintf("Warning: cvmx_qlm_interface: Invalid interface %d\n", interface);
	} else if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		switch (interface) {
		case 0:
			return MUX_78XX_IFACE0 ? 2 : 0;
		case 1:
			return MUX_78XX_IFACE1 ? 3 : 1;
		case 2:
		case 3:
		case 4:
		case 5:
			return interface + 2;
		default:
			break;
		}
	} else {
		/* Must be cn68XX */
		switch (interface) {
		case 1:
			return 0;
		default:
			return interface;
		}
	}
	return -1;
}

/**
 * Return number of lanes for a given qlm
 *
 * @return  Number of lanes
 */
int cvmx_qlm_get_lanes(int qlm)
{
	if (OCTEON_IS_MODEL(OCTEON_CN61XX) && qlm == 1)
		return 2;
	else if (OCTEON_IS_MODEL(OCTEON_CNF71XX))
		return 2;

	return 4;
}

/**
 * Get the QLM JTAG fields based on Octeon model on the supported chips.
 *
 * @return  qlm_jtag_field_t structure
 */
const __cvmx_qlm_jtag_field_t *cvmx_qlm_jtag_get_field(void)
{
	/* Figure out which JTAG chain description we're using */
	if (OCTEON_IS_MODEL(OCTEON_CN68XX))
		return __cvmx_qlm_jtag_field_cn68xx;
	else if (OCTEON_IS_MODEL(OCTEON_CN66XX)
		 || OCTEON_IS_MODEL(OCTEON_CN61XX)
		 || OCTEON_IS_MODEL(OCTEON_CNF71XX))
		return __cvmx_qlm_jtag_field_cn66xx;
	else if (OCTEON_IS_MODEL(OCTEON_CN63XX))
		return __cvmx_qlm_jtag_field_cn63xx;
#ifndef _MIPS_ARCH_OCTEON2
	else if (OCTEON_IS_MODEL(OCTEON_CN56XX))
		return __cvmx_qlm_jtag_field_cn56xx;
	else if (OCTEON_IS_MODEL(OCTEON_CN52XX))
		return __cvmx_qlm_jtag_field_cn52xx;
#endif
	else {
		//cvmx_dprintf("cvmx_qlm_jtag_get_field: Needs update for this chip\n");
		return NULL;
	}
}

/**
 * Get the QLM JTAG length by going through qlm_jtag_field for each
 * Octeon model that is supported
 *
 * @return return the length.
 */
int cvmx_qlm_jtag_get_length(void)
{
	const __cvmx_qlm_jtag_field_t *qlm_ptr = cvmx_qlm_jtag_get_field();
	int length = 0;

	/* Figure out how many bits are in the JTAG chain */
	while (qlm_ptr != NULL && qlm_ptr->name) {
		if (qlm_ptr->stop_bit > length)
			length = qlm_ptr->stop_bit + 1;
		qlm_ptr++;
	}
	return length;
}

/**
 * Initialize the QLM layer
 */
void cvmx_qlm_init(void)
{
	int qlm;
	int qlm_jtag_length;
	char *qlm_jtag_name = "cvmx_qlm_jtag";
	int qlm_jtag_size = CVMX_QLM_JTAG_UINT32 * 8 * 4;
	static uint64_t qlm_base = 0;
	const cvmx_bootmem_named_block_desc_t *desc;

	if (OCTEON_IS_OCTEON3())
		return;

#ifndef CVMX_BUILD_FOR_LINUX_HOST
	/* Skip actual JTAG accesses on simulator */
	if (cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_SIM)
		return;
#endif

	qlm_jtag_length = cvmx_qlm_jtag_get_length();

	if (4 * qlm_jtag_length > (int)sizeof(__cvmx_qlm_jtag_xor_ref[0]) * 8) {
		cvmx_dprintf("ERROR: cvmx_qlm_init: JTAG chain larger than XOR ref size\n");
		return;
	}

	/* No need to initialize the initial JTAG state if cvmx_qlm_jtag
	   named block is already created. */
	if ((desc = cvmx_bootmem_find_named_block(qlm_jtag_name)) != NULL) {
#ifdef CVMX_BUILD_FOR_LINUX_HOST
		char buffer[qlm_jtag_size];

		octeon_remote_read_mem(buffer, desc->base_addr, qlm_jtag_size);
		memcpy(__cvmx_qlm_jtag_xor_ref, buffer, qlm_jtag_size);
#else
		__cvmx_qlm_jtag_xor_ref = cvmx_phys_to_ptr(desc->base_addr);
#endif
		/* Initialize the internal JTAG */
		cvmx_helper_qlm_jtag_init();
		return;
	}

	/* Create named block to store the initial JTAG state. */
	qlm_base = cvmx_bootmem_phy_named_block_alloc(qlm_jtag_size, 0, 0, 128, qlm_jtag_name, CVMX_BOOTMEM_FLAG_END_ALLOC);

	if (qlm_base == -1ull) {
		cvmx_dprintf("ERROR: cvmx_qlm_init: Error in creating %s named block\n", qlm_jtag_name);
		return;
	}
#ifndef CVMX_BUILD_FOR_LINUX_HOST
	__cvmx_qlm_jtag_xor_ref = cvmx_phys_to_ptr(qlm_base);
#endif
	memset(__cvmx_qlm_jtag_xor_ref, 0, qlm_jtag_size);

	/* Initialize the internal JTAG */
	cvmx_helper_qlm_jtag_init();

	/* Read the XOR defaults for the JTAG chain */
	for (qlm = 0; qlm < cvmx_qlm_get_num(); qlm++) {
		int i;
		int num_lanes = cvmx_qlm_get_lanes(qlm);
		/* Shift all zeros in the chain to make sure all fields are at
		   reset defaults */
		cvmx_helper_qlm_jtag_shift_zeros(qlm, qlm_jtag_length * num_lanes);
		cvmx_helper_qlm_jtag_update(qlm);

		/* Capture the reset defaults */
		cvmx_helper_qlm_jtag_capture(qlm);
		/* Save the reset defaults. This will shift out too much data, but
		   the extra zeros don't hurt anything */
		for (i = 0; i < CVMX_QLM_JTAG_UINT32; i++)
			__cvmx_qlm_jtag_xor_ref[qlm][i] = cvmx_helper_qlm_jtag_shift(qlm, 32, 0);
	}

#ifdef CVMX_BUILD_FOR_LINUX_HOST
	/* Update the initial state for oct-remote utils. */
	{
		char buffer[qlm_jtag_size];

		memcpy(buffer, &__cvmx_qlm_jtag_xor_ref, qlm_jtag_size);
		octeon_remote_write_mem(qlm_base, buffer, qlm_jtag_size);
	}
#endif

	/* Apply all QLM errata workarounds. */
	__cvmx_qlm_speed_tweak();
	__cvmx_qlm_pcie_idle_dac_tweak();
}

/**
 * Lookup the bit information for a JTAG field name
 *
 * @param name   Name to lookup
 *
 * @return Field info, or NULL on failure
 */
static const __cvmx_qlm_jtag_field_t *__cvmx_qlm_lookup_field(const char *name)
{
	const __cvmx_qlm_jtag_field_t *ptr = cvmx_qlm_jtag_get_field();
	while (ptr->name) {
		if (strcmp(name, ptr->name) == 0)
			return ptr;
		ptr++;
	}
	cvmx_dprintf("__cvmx_qlm_lookup_field: Illegal field name %s\n", name);
	return NULL;
}

/**
 * Get a field in a QLM JTAG chain
 *
 * @param qlm    QLM to get
 * @param lane   Lane in QLM to get
 * @param name   String name of field
 *
 * @return JTAG field value
 */
uint64_t cvmx_qlm_jtag_get(int qlm, int lane, const char *name)
{
	const __cvmx_qlm_jtag_field_t *field = __cvmx_qlm_lookup_field(name);
	int qlm_jtag_length = cvmx_qlm_jtag_get_length();
	int num_lanes = cvmx_qlm_get_lanes(qlm);

	if (!field)
		return 0;

	/* Capture the current settings */
	cvmx_helper_qlm_jtag_capture(qlm);
	/* Shift past lanes we don't care about. CN6XXX/7XXX shifts lane 0 first, CN3XXX/5XXX shifts lane 3 first */
	if (OCTEON_IS_MODEL(OCTEON_CN5XXX))
		cvmx_helper_qlm_jtag_shift_zeros(qlm, qlm_jtag_length * (lane));	/* Shift to the start of the field */
	else
		cvmx_helper_qlm_jtag_shift_zeros(qlm, qlm_jtag_length * (num_lanes - 1 - lane));	/* Shift to the start of the field */
	cvmx_helper_qlm_jtag_shift_zeros(qlm, field->start_bit);
	/* Shift out the value and return it */
	return cvmx_helper_qlm_jtag_shift(qlm, field->stop_bit - field->start_bit + 1, 0);
}

/**
 * Set a field in a QLM JTAG chain
 *
 * @param qlm    QLM to set
 * @param lane   Lane in QLM to set, or -1 for all lanes
 * @param name   String name of field
 * @param value  Value of the field
 */
void cvmx_qlm_jtag_set(int qlm, int lane, const char *name, uint64_t value)
{
	int i, l;
	uint32_t shift_values[CVMX_QLM_JTAG_UINT32];
	int num_lanes = cvmx_qlm_get_lanes(qlm);
	const __cvmx_qlm_jtag_field_t *field = __cvmx_qlm_lookup_field(name);
	int qlm_jtag_length = cvmx_qlm_jtag_get_length();
	int total_length = qlm_jtag_length * num_lanes;
	int bits = 0;

	if (!field)
		return;

	/* Get the current state */
	cvmx_helper_qlm_jtag_capture(qlm);
	for (i = 0; i < CVMX_QLM_JTAG_UINT32; i++)
		shift_values[i] = cvmx_helper_qlm_jtag_shift(qlm, 32, 0);

	/* Put new data in our local array */
	for (l = 0; l < num_lanes; l++) {
		uint64_t new_value = value;
		int bits;
		int adj_lanes;

		if ((l != lane) && (lane != -1))
			continue;

		if (OCTEON_IS_MODEL(OCTEON_CN5XXX))
			adj_lanes = l * qlm_jtag_length;
		else
			adj_lanes = (num_lanes - 1 - l) * qlm_jtag_length;

		for (bits = field->start_bit + adj_lanes; bits <= field->stop_bit + adj_lanes; bits++) {
			if (new_value & 1)
				shift_values[bits / 32] |= 1 << (bits & 31);
			else
				shift_values[bits / 32] &= ~(1 << (bits & 31));
			new_value >>= 1;
		}
	}

	/* Shift out data and xor with reference */
	while (bits < total_length) {
		uint32_t shift = shift_values[bits / 32] ^ __cvmx_qlm_jtag_xor_ref[qlm][bits / 32];
		int width = total_length - bits;
		if (width > 32)
			width = 32;
		cvmx_helper_qlm_jtag_shift(qlm, width, shift);
		bits += 32;
	}

	/* Update the new data */
	cvmx_helper_qlm_jtag_update(qlm);
	/* Always give the QLM 1ms to settle after every update. This may not
	   always be needed, but some of the options make significant
	   electrical changes */
	cvmx_wait_usec(1000);
}

/**
 * Errata G-16094: QLM Gen2 Equalizer Default Setting Change.
 * CN68XX pass 1.x and CN66XX pass 1.x QLM tweak. This function tweaks the
 * JTAG setting for a QLMs to run better at 5 and 6.25Ghz.
 */
void __cvmx_qlm_speed_tweak(void)
{
	cvmx_mio_qlmx_cfg_t qlm_cfg;
	int num_qlms = cvmx_qlm_get_num();
	int qlm;

	/* Workaround for Errata (G-16467) */
	if (OCTEON_IS_MODEL(OCTEON_CN68XX_PASS2_X)) {
		for (qlm = 0; qlm < num_qlms; qlm++) {
			int ir50dac;
			/* This workaround only applies to QLMs running at 6.25Ghz */
			if (cvmx_qlm_get_gbaud_mhz(qlm) == 6250) {
#ifdef CVMX_QLM_DUMP_STATE
				cvmx_dprintf("%s:%d: QLM%d: Applying workaround for Errata G-16467\n", __func__, __LINE__, qlm);
				cvmx_qlm_display_registers(qlm);
				cvmx_dprintf("\n");
#endif
				cvmx_qlm_jtag_set(qlm, -1, "cfg_cdr_trunc", 0);
				/* Hold the QLM in reset */
				cvmx_qlm_jtag_set(qlm, -1, "cfg_rst_n_set", 0);
				cvmx_qlm_jtag_set(qlm, -1, "cfg_rst_n_clr", 1);
				/* Forcfe TX to be idle */
				cvmx_qlm_jtag_set(qlm, -1, "cfg_tx_idle_clr", 0);
				cvmx_qlm_jtag_set(qlm, -1, "cfg_tx_idle_set", 1);
				if (OCTEON_IS_MODEL(OCTEON_CN68XX_PASS2_0)) {
					ir50dac = cvmx_qlm_jtag_get(qlm, 0, "ir50dac");
					while (++ir50dac <= 31)
						cvmx_qlm_jtag_set(qlm, -1, "ir50dac", ir50dac);
				}
				cvmx_qlm_jtag_set(qlm, -1, "div4_byp", 0);
				cvmx_qlm_jtag_set(qlm, -1, "clkf_byp", 16);
				cvmx_qlm_jtag_set(qlm, -1, "serdes_pll_byp", 1);
				cvmx_qlm_jtag_set(qlm, -1, "spdsel_byp", 1);
#ifdef CVMX_QLM_DUMP_STATE
				cvmx_dprintf("%s:%d: QLM%d: Done applying workaround for Errata G-16467\n", __func__, __LINE__, qlm);
				cvmx_qlm_display_registers(qlm);
				cvmx_dprintf("\n\n");
#endif
				/* The QLM will be taken out of reset later when ILK/XAUI are initialized. */
			}
		}

#ifndef CVMX_BUILD_FOR_LINUX_HOST
		/* These QLM tuning parameters are specific to EBB6800
		   eval boards using Cavium QLM cables. These should be
		   removed or tunned based on customer boards. */
		if (cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_EBB6800) {
			for (qlm = 0; qlm < num_qlms; qlm++) {
#ifdef CVMX_QLM_DUMP_STATE
				cvmx_dprintf("Setting tunning parameters for QLM%d\n", qlm);
#endif
				cvmx_qlm_jtag_set(qlm, -1, "biasdrv_hs_ls_byp", 12);
				cvmx_qlm_jtag_set(qlm, -1, "biasdrv_hf_byp", 12);
				cvmx_qlm_jtag_set(qlm, -1, "biasdrv_lf_ls_byp", 12);
				cvmx_qlm_jtag_set(qlm, -1, "biasdrv_lf_byp", 12);
				cvmx_qlm_jtag_set(qlm, -1, "tcoeff_hf_byp", 15);
				cvmx_qlm_jtag_set(qlm, -1, "tcoeff_hf_ls_byp", 15);
				cvmx_qlm_jtag_set(qlm, -1, "tcoeff_lf_ls_byp", 15);
				cvmx_qlm_jtag_set(qlm, -1, "tcoeff_lf_byp", 15);
				cvmx_qlm_jtag_set(qlm, -1, "rx_cap_gen2", 0);
				cvmx_qlm_jtag_set(qlm, -1, "rx_eq_gen2", 11);
				cvmx_qlm_jtag_set(qlm, -1, "serdes_tx_byp", 1);
			}
		}
		else if (cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_NIC68_4) {
			for (qlm = 0; qlm < num_qlms; qlm++) {
#ifdef CVMX_QLM_DUMP_STATE
				cvmx_dprintf("Setting tunning parameters for QLM%d\n", qlm);
#endif
				cvmx_qlm_jtag_set(qlm, -1, "biasdrv_hs_ls_byp", 30);
				cvmx_qlm_jtag_set(qlm, -1, "biasdrv_hf_byp", 30);
				cvmx_qlm_jtag_set(qlm, -1, "biasdrv_lf_ls_byp", 30);
				cvmx_qlm_jtag_set(qlm, -1, "biasdrv_lf_byp", 30);
				cvmx_qlm_jtag_set(qlm, -1, "tcoeff_hf_byp", 0);
				cvmx_qlm_jtag_set(qlm, -1, "tcoeff_hf_ls_byp", 0);
				cvmx_qlm_jtag_set(qlm, -1, "tcoeff_lf_ls_byp", 0);
				cvmx_qlm_jtag_set(qlm, -1, "tcoeff_lf_byp", 0);
				cvmx_qlm_jtag_set(qlm, -1, "rx_cap_gen2", 1);
				cvmx_qlm_jtag_set(qlm, -1, "rx_eq_gen2", 8);
				cvmx_qlm_jtag_set(qlm, -1, "serdes_tx_byp", 1);
			}
		}
#endif
	}

	/* G-16094 QLM Gen2 Equalizer Default Setting Change */
	else if (OCTEON_IS_MODEL(OCTEON_CN68XX_PASS1_X)
		 || OCTEON_IS_MODEL(OCTEON_CN66XX_PASS1_X)) {
		/* Loop through the QLMs */
		for (qlm = 0; qlm < num_qlms; qlm++) {
			/* Read the QLM speed */
			qlm_cfg.u64 = cvmx_read_csr(CVMX_MIO_QLMX_CFG(qlm));

			/* If the QLM is at 6.25Ghz or 5Ghz then program JTAG */
			if ((qlm_cfg.s.qlm_spd == 5) || (qlm_cfg.s.qlm_spd == 12) || (qlm_cfg.s.qlm_spd == 0) || (qlm_cfg.s.qlm_spd == 6) || (qlm_cfg.s.qlm_spd == 11)) {
				cvmx_qlm_jtag_set(qlm, -1, "rx_cap_gen2", 0x1);
				cvmx_qlm_jtag_set(qlm, -1, "rx_eq_gen2", 0x8);
			}
		}
	}
}

/**
 * Errata G-16174: QLM Gen2 PCIe IDLE DAC change.
 * CN68XX pass 1.x, CN66XX pass 1.x and CN63XX pass 1.0-2.2 QLM tweak.
 * This function tweaks the JTAG setting for a QLMs for PCIe to run better.
 */
void __cvmx_qlm_pcie_idle_dac_tweak(void)
{
	int num_qlms = 0;
	int qlm;

	if (OCTEON_IS_MODEL(OCTEON_CN68XX_PASS1_X))
		num_qlms = 5;
	else if (OCTEON_IS_MODEL(OCTEON_CN66XX_PASS1_X))
		num_qlms = 3;
	else if (OCTEON_IS_MODEL(OCTEON_CN63XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CN63XX_PASS2_X))
		num_qlms = 3;
	else
		return;

	/* Loop through the QLMs */
	for (qlm = 0; qlm < num_qlms; qlm++)
		cvmx_qlm_jtag_set(qlm, -1, "idle_dac", 0x2);
}

void __cvmx_qlm_pcie_cfg_rxd_set_tweak(int qlm, int lane)
{
	if (OCTEON_IS_MODEL(OCTEON_CN6XXX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)) {
		cvmx_qlm_jtag_set(qlm, lane, "cfg_rxd_set", 0x1);
	}
}

/**
 * Get the speed (Gbaud) of the QLM in Mhz.
 *
 * @param qlm    QLM to examine
 *
 * @return Speed in Mhz
 */
int cvmx_qlm_get_gbaud_mhz(int qlm)
{
	if (OCTEON_IS_MODEL(OCTEON_CN63XX)) {
		if (qlm == 2) {
			cvmx_gmxx_inf_mode_t inf_mode;
			inf_mode.u64 = cvmx_read_csr(CVMX_GMXX_INF_MODE(0));
			switch (inf_mode.s.speed) {
			case 0:
				return 5000;	/* 5     Gbaud */
			case 1:
				return 2500;	/* 2.5   Gbaud */
			case 2:
				return 2500;	/* 2.5   Gbaud */
			case 3:
				return 1250;	/* 1.25  Gbaud */
			case 4:
				return 1250;	/* 1.25  Gbaud */
			case 5:
				return 6250;	/* 6.25  Gbaud */
			case 6:
				return 5000;	/* 5     Gbaud */
			case 7:
				return 2500;	/* 2.5   Gbaud */
			case 8:
				return 3125;	/* 3.125 Gbaud */
			case 9:
				return 2500;	/* 2.5   Gbaud */
			case 10:
				return 1250;	/* 1.25  Gbaud */
			case 11:
				return 5000;	/* 5     Gbaud */
			case 12:
				return 6250;	/* 6.25  Gbaud */
			case 13:
				return 3750;	/* 3.75  Gbaud */
			case 14:
				return 3125;	/* 3.125 Gbaud */
			default:
				return 0;	/* Disabled */
			}
		} else {
			cvmx_sriox_status_reg_t status_reg;
			status_reg.u64 = cvmx_read_csr(CVMX_SRIOX_STATUS_REG(qlm));
			if (status_reg.s.srio) {
				cvmx_sriomaintx_port_0_ctl2_t sriomaintx_port_0_ctl2;
				sriomaintx_port_0_ctl2.u32 = cvmx_read_csr(CVMX_SRIOMAINTX_PORT_0_CTL2(qlm));
				switch (sriomaintx_port_0_ctl2.s.sel_baud) {
				case 1:
					return 1250;	/* 1.25  Gbaud */
				case 2:
					return 2500;	/* 2.5   Gbaud */
				case 3:
					return 3125;	/* 3.125 Gbaud */
				case 4:
					return 5000;	/* 5     Gbaud */
				case 5:
					return 6250;	/* 6.250 Gbaud */
				default:
					return 0;	/* Disabled */
				}
			} else {
				cvmx_pciercx_cfg032_t pciercx_cfg032;
				pciercx_cfg032.u32 = cvmx_read_csr(CVMX_PCIERCX_CFG032(qlm));
				switch (pciercx_cfg032.s.ls) {
				case 1:
					return 2500;
				case 2:
					return 5000;
				case 4:
					return 8000;
				default:
					{
						cvmx_mio_rst_boot_t mio_rst_boot;
						mio_rst_boot.u64 = cvmx_read_csr(CVMX_MIO_RST_BOOT);
						if ((qlm == 0) && mio_rst_boot.s.qlm0_spd == 0xf)
							return 0;
						if ((qlm == 1) && mio_rst_boot.s.qlm1_spd == 0xf)
							return 0;
						return 5000;	/* Best guess I can make */
					}
				}
			}
		}
	} else if (OCTEON_IS_OCTEON2()) {
		cvmx_mio_qlmx_cfg_t qlm_cfg;

		qlm_cfg.u64 = cvmx_read_csr(CVMX_MIO_QLMX_CFG(qlm));
		switch (qlm_cfg.s.qlm_spd) {
		case 0:
			return 5000;	/* 5     Gbaud */
		case 1:
			return 2500;	/* 2.5   Gbaud */
		case 2:
			return 2500;	/* 2.5   Gbaud */
		case 3:
			return 1250;	/* 1.25  Gbaud */
		case 4:
			return 1250;	/* 1.25  Gbaud */
		case 5:
			return 6250;	/* 6.25  Gbaud */
		case 6:
			return 5000;	/* 5     Gbaud */
		case 7:
			return 2500;	/* 2.5   Gbaud */
		case 8:
			return 3125;	/* 3.125 Gbaud */
		case 9:
			return 2500;	/* 2.5   Gbaud */
		case 10:
			return 1250;	/* 1.25  Gbaud */
		case 11:
			return 5000;	/* 5     Gbaud */
		case 12:
			return 6250;	/* 6.25  Gbaud */
		case 13:
			return 3750;	/* 3.75  Gbaud */
		case 14:
			return 3125;	/* 3.125 Gbaud */
		default:
			return 0;	/* Disabled */
		}
	} else if (OCTEON_IS_MODEL(OCTEON_CN70XX)) {
		cvmx_gserx_dlmx_mpll_multiplier_t mpll_multiplier;
		uint64_t meas_refclock;
		uint64_t freq;

		/* Measure the reference clock */
		meas_refclock = cvmx_qlm_measure_clock(qlm);
		/* Multiply to get the final frequency */
		mpll_multiplier.u64 = cvmx_read_csr(CVMX_GSERX_DLMX_MPLL_MULTIPLIER(qlm, 0));
		freq = meas_refclock * mpll_multiplier.s.mpll_multiplier;
		freq = (freq + 500000) / 1000000;
		return freq;
	}
	return 0;
}

static enum cvmx_qlm_mode __cvmx_qlm_get_mode_cn70xx(int qlm)
{
#ifndef CVMX_BUILD_FOR_LINUX_HOST
	if (cvmx_sysinfo_get()->board_type != CVMX_BOARD_TYPE_SIM) {
		union cvmx_gserx_dlmx_phy_reset phy_reset;

		phy_reset.u64 = cvmx_read_csr(CVMX_GSERX_DLMX_PHY_RESET(qlm, 0));
		if (phy_reset.s.phy_reset)
			return CVMX_QLM_MODE_DISABLED;

	}
#endif

	switch(qlm) {
	case 0: /* DLM0/DLM1 - SGMII/QSGMII/RXAUI */
		{
			union cvmx_gmxx_inf_mode inf_mode0, inf_mode1;

			inf_mode0.u64 = cvmx_read_csr(CVMX_GMXX_INF_MODE(0));
			inf_mode1.u64 = cvmx_read_csr(CVMX_GMXX_INF_MODE(1));

			/* SGMII0 SGMII1 */
			switch (inf_mode0.s.mode) {
			case CVMX_GMX_INF_MODE_SGMII:
				switch (inf_mode1.s.mode) {
				case CVMX_GMX_INF_MODE_SGMII:
					return CVMX_QLM_MODE_SGMII_SGMII;
				case CVMX_GMX_INF_MODE_QSGMII:
					return CVMX_QLM_MODE_SGMII_QSGMII;
				default:
					return CVMX_QLM_MODE_SGMII_DISABLED;
				}
			case CVMX_GMX_INF_MODE_QSGMII:
				switch (inf_mode1.s.mode) {
				case CVMX_GMX_INF_MODE_SGMII:
					return CVMX_QLM_MODE_QSGMII_SGMII;
				case CVMX_GMX_INF_MODE_QSGMII:
					return CVMX_QLM_MODE_QSGMII_QSGMII;
				default:
					return CVMX_QLM_MODE_QSGMII_DISABLED;
				}
			case CVMX_GMX_INF_MODE_RXAUI:
				return CVMX_QLM_MODE_RXAUI_1X2;
			default:
				switch (inf_mode1.s.mode) {
				case CVMX_GMX_INF_MODE_SGMII:
					return CVMX_QLM_MODE_DISABLED_SGMII;
				case CVMX_GMX_INF_MODE_QSGMII:
					return CVMX_QLM_MODE_DISABLED_QSGMII;
				default:
				return CVMX_QLM_MODE_DISABLED;
			}
		}
		}
	case 1:  /* Sata / pem0 */
		{
			union cvmx_gserx_sata_cfg sata_cfg;
			union cvmx_pemx_cfg pem0_cfg;

			sata_cfg.u64 = cvmx_read_csr(CVMX_GSERX_SATA_CFG(0));
			pem0_cfg.u64 = cvmx_read_csr(CVMX_PEMX_CFG(0));

			switch(pem0_cfg.cn70xx.md) {
			case CVMX_PEM_MD_GEN2_2LANE:
			case CVMX_PEM_MD_GEN1_2LANE:
				return CVMX_QLM_MODE_PCIE_1X2;
			case CVMX_PEM_MD_GEN2_1LANE:
			case CVMX_PEM_MD_GEN1_1LANE:
				if (sata_cfg.s.sata_en)
					/* Both PEM0 and PEM1 */
					return CVMX_QLM_MODE_PCIE_2X1;
				else
					/* Only PEM0 */
					return CVMX_QLM_MODE_PCIE_1X1;
			case CVMX_PEM_MD_GEN2_4LANE:
			case CVMX_PEM_MD_GEN1_4LANE:
				return CVMX_QLM_MODE_PCIE;
			default:
				return CVMX_QLM_MODE_DISABLED;
			}
		}
	case 2:
		{
			union cvmx_gserx_sata_cfg sata_cfg;
			union cvmx_pemx_cfg pem0_cfg, pem1_cfg, pem2_cfg;

			sata_cfg.u64 = cvmx_read_csr(CVMX_GSERX_SATA_CFG(0));
			pem0_cfg.u64 = cvmx_read_csr(CVMX_PEMX_CFG(0));
			pem1_cfg.u64 = cvmx_read_csr(CVMX_PEMX_CFG(1));
			pem2_cfg.u64 = cvmx_read_csr(CVMX_PEMX_CFG(2));

			if (sata_cfg.s.sata_en)
				return CVMX_QLM_MODE_SATA_2X1;
			if (pem0_cfg.cn70xx.md == CVMX_PEM_MD_GEN2_4LANE
			    || pem0_cfg.cn70xx.md == CVMX_PEM_MD_GEN1_4LANE)
				return CVMX_QLM_MODE_PCIE;
			if (pem1_cfg.cn70xx.md == CVMX_PEM_MD_GEN2_2LANE
			    || pem1_cfg.cn70xx.md == CVMX_PEM_MD_GEN1_2LANE) {
				return CVMX_QLM_MODE_PCIE_1X2;
			}
			if (pem1_cfg.cn70xx.md == CVMX_PEM_MD_GEN2_1LANE
			    || pem1_cfg.cn70xx.md == CVMX_PEM_MD_GEN1_1LANE) {
				if (pem2_cfg.cn70xx.md == CVMX_PEM_MD_GEN2_1LANE
				    || pem2_cfg.cn70xx.md == CVMX_PEM_MD_GEN1_1LANE) {
					return CVMX_QLM_MODE_PCIE_2X1;
				} else
					return CVMX_QLM_MODE_PCIE_1X1;
			}
			if (pem2_cfg.cn70xx.md == CVMX_PEM_MD_GEN2_1LANE
			    || pem2_cfg.cn70xx.md == CVMX_PEM_MD_GEN1_1LANE)
				return CVMX_QLM_MODE_PCIE_2X1;
			return CVMX_QLM_MODE_DISABLED;
		}
	default:
		return CVMX_QLM_MODE_DISABLED;
	}

	return CVMX_QLM_MODE_DISABLED;
}

/*
 * Get the DLM mode for the interface based on the interface type.
 *
 * @param interface_type   0 - SGMII/QSGMII/RXAUI interface
 *                         1 - PCIe
 *                         2 - SATA
 * @param interface        interface to use
 * @return  the qlm mode the interface is
 */
enum cvmx_qlm_mode cvmx_qlm_get_dlm_mode(int interface_type, int interface)
{
	switch (interface_type) {
	case 0:  /* SGMII/QSGMII/RXAUI */
	{
		enum cvmx_qlm_mode qlm_mode = __cvmx_qlm_get_mode_cn70xx(0);
		switch (interface) {
		case 0:
			switch (qlm_mode) {
			case CVMX_QLM_MODE_SGMII_SGMII:
			case CVMX_QLM_MODE_SGMII_DISABLED:
			case CVMX_QLM_MODE_SGMII_QSGMII:
				return CVMX_QLM_MODE_SGMII;
			case CVMX_QLM_MODE_QSGMII_QSGMII:
			case CVMX_QLM_MODE_QSGMII_DISABLED:
			case CVMX_QLM_MODE_QSGMII_SGMII:
				return CVMX_QLM_MODE_QSGMII;
			case CVMX_QLM_MODE_RXAUI_1X2:
				return CVMX_QLM_MODE_RXAUI;
			default:
				return CVMX_QLM_MODE_DISABLED;
			}
		case 1:
			switch (qlm_mode) {
			case CVMX_QLM_MODE_SGMII_SGMII:
			case CVMX_QLM_MODE_DISABLED_SGMII:
			case CVMX_QLM_MODE_QSGMII_SGMII:
				return CVMX_QLM_MODE_SGMII;
			case CVMX_QLM_MODE_QSGMII_QSGMII:
			case CVMX_QLM_MODE_DISABLED_QSGMII:
			case CVMX_QLM_MODE_SGMII_QSGMII:
				return CVMX_QLM_MODE_QSGMII;
			default:
				return CVMX_QLM_MODE_DISABLED;
			}
		default:
			return qlm_mode;
		}
	}
	case 1:  /* PCIe */
	{
		enum cvmx_qlm_mode qlm_mode1 = __cvmx_qlm_get_mode_cn70xx(1);
		enum cvmx_qlm_mode qlm_mode2 = __cvmx_qlm_get_mode_cn70xx(2);

		switch (interface) {
		case 0: /* PCIe0 can be DLM1 with 1, 2 or 4 lanes */
			return qlm_mode1;
		case 1: /* PCIe1 can be in DLM1 1 lane(1), DLM2 1 lane(0) or 2 lanes(0-1) */
			if (qlm_mode1 == CVMX_QLM_MODE_PCIE_2X1)
				return CVMX_QLM_MODE_PCIE_2X1;
			else if (qlm_mode2 == CVMX_QLM_MODE_PCIE_1X2 ||
				 qlm_mode2 == CVMX_QLM_MODE_PCIE_2X1)
				return qlm_mode2;
			else
				return CVMX_QLM_MODE_DISABLED;
		case 2: /* PCIe2 can be DLM2 1 lanes(1) */
			if (qlm_mode2 == CVMX_QLM_MODE_PCIE_2X1)
				return qlm_mode2;
			else
				return CVMX_QLM_MODE_DISABLED;
		default:
			return CVMX_QLM_MODE_DISABLED;
		}
	}
	case 2:  /* SATA */
	{
		enum cvmx_qlm_mode qlm_mode = __cvmx_qlm_get_mode_cn70xx(2);

		if (qlm_mode == CVMX_QLM_MODE_SATA_2X1)
			return CVMX_QLM_MODE_SATA_2X1;
		else
			return CVMX_QLM_MODE_DISABLED;
	}
	default:
		return CVMX_QLM_MODE_DISABLED;
	}
}

static enum cvmx_qlm_mode __cvmx_qlm_get_mode_cn6xxx(int qlm)
{
	cvmx_mio_qlmx_cfg_t qlmx_cfg;

	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		qlmx_cfg.u64 = cvmx_read_csr(CVMX_MIO_QLMX_CFG(qlm));
		/* QLM is disabled when QLM SPD is 15. */
		if (qlmx_cfg.s.qlm_spd == 15)
			return CVMX_QLM_MODE_DISABLED;

		switch (qlmx_cfg.s.qlm_cfg) {
		case 0:	/* PCIE */
			return CVMX_QLM_MODE_PCIE;
		case 1:	/* ILK */
			return CVMX_QLM_MODE_ILK;
		case 2:	/* SGMII */
			return CVMX_QLM_MODE_SGMII;
		case 3:	/* XAUI */
			return CVMX_QLM_MODE_XAUI;
		case 7:	/* RXAUI */
			return CVMX_QLM_MODE_RXAUI;
		default:
			return CVMX_QLM_MODE_DISABLED;
		}
	} else if (OCTEON_IS_MODEL(OCTEON_CN66XX)) {
		qlmx_cfg.u64 = cvmx_read_csr(CVMX_MIO_QLMX_CFG(qlm));
		/* QLM is disabled when QLM SPD is 15. */
		if (qlmx_cfg.s.qlm_spd == 15)
			return CVMX_QLM_MODE_DISABLED;

		switch (qlmx_cfg.s.qlm_cfg) {
		case 0x9:	/* SGMII */
			return CVMX_QLM_MODE_SGMII;
		case 0xb:	/* XAUI */
			return CVMX_QLM_MODE_XAUI;
		case 0x0:	/* PCIE gen2 */
		case 0x8:	/* PCIE gen2 (alias) */
		case 0x2:	/* PCIE gen1 */
		case 0xa:	/* PCIE gen1 (alias) */
			return CVMX_QLM_MODE_PCIE;
		case 0x1:	/* SRIO 1x4 short */
		case 0x3:	/* SRIO 1x4 long */
			return CVMX_QLM_MODE_SRIO_1X4;
		case 0x4:	/* SRIO 2x2 short */
		case 0x6:	/* SRIO 2x2 long */
			return CVMX_QLM_MODE_SRIO_2X2;
		case 0x5:	/* SRIO 4x1 short */
		case 0x7:	/* SRIO 4x1 long */
			if (!OCTEON_IS_MODEL(OCTEON_CN66XX_PASS1_0))
				return CVMX_QLM_MODE_SRIO_4X1;
		default:
			return CVMX_QLM_MODE_DISABLED;
		}
	} else if (OCTEON_IS_MODEL(OCTEON_CN63XX)) {
		cvmx_sriox_status_reg_t status_reg;
		/* For now skip qlm2 */
		if (qlm == 2) {
			cvmx_gmxx_inf_mode_t inf_mode;
			inf_mode.u64 = cvmx_read_csr(CVMX_GMXX_INF_MODE(0));
			if (inf_mode.s.speed == 15)
				return CVMX_QLM_MODE_DISABLED;
			else if (inf_mode.s.mode == 0)
				return CVMX_QLM_MODE_SGMII;
			else
				return CVMX_QLM_MODE_XAUI;
		}
		status_reg.u64 = cvmx_read_csr(CVMX_SRIOX_STATUS_REG(qlm));
		if (status_reg.s.srio)
			return CVMX_QLM_MODE_SRIO_1X4;
		else
			return CVMX_QLM_MODE_PCIE;
	} else if (OCTEON_IS_MODEL(OCTEON_CN61XX)) {
		qlmx_cfg.u64 = cvmx_read_csr(CVMX_MIO_QLMX_CFG(qlm));
		/* QLM is disabled when QLM SPD is 15. */
		if (qlmx_cfg.s.qlm_spd == 15)
			return CVMX_QLM_MODE_DISABLED;

		switch (qlm) {
		case 0:
			switch (qlmx_cfg.s.qlm_cfg) {
			case 0:	/* PCIe 1x4 gen2 / gen1 */
				return CVMX_QLM_MODE_PCIE;
			case 2:	/* SGMII */
				return CVMX_QLM_MODE_SGMII;
			case 3:	/* XAUI */
				return CVMX_QLM_MODE_XAUI;
			default:
				return CVMX_QLM_MODE_DISABLED;
			}
			break;
		case 1:
			switch (qlmx_cfg.s.qlm_cfg) {
			case 0:	/* PCIe 1x2 gen2 / gen1 */
				return CVMX_QLM_MODE_PCIE_1X2;
			case 1:	/* PCIe 2x1 gen2 / gen1 */
				return CVMX_QLM_MODE_PCIE_2X1;
			default:
				return CVMX_QLM_MODE_DISABLED;
			}
			break;
		case 2:
			switch (qlmx_cfg.s.qlm_cfg) {
			case 2:	/* SGMII */
				return CVMX_QLM_MODE_SGMII;
			case 3:	/* XAUI */
				return CVMX_QLM_MODE_XAUI;
			default:
				return CVMX_QLM_MODE_DISABLED;
			}
			break;
		}
	} else if (OCTEON_IS_MODEL(OCTEON_CNF71XX)) {
		qlmx_cfg.u64 = cvmx_read_csr(CVMX_MIO_QLMX_CFG(qlm));
		/* QLM is disabled when QLM SPD is 15. */
		if (qlmx_cfg.s.qlm_spd == 15)
			return CVMX_QLM_MODE_DISABLED;

		switch (qlm) {
		case 0:
			if (qlmx_cfg.s.qlm_cfg == 2)	/* SGMII */
				return CVMX_QLM_MODE_SGMII;
			break;
		case 1:
			switch (qlmx_cfg.s.qlm_cfg) {
			case 0:	/* PCIe 1x2 gen2 / gen1 */
				return CVMX_QLM_MODE_PCIE_1X2;
			case 1:	/* PCIe 2x1 gen2 / gen1 */
				return CVMX_QLM_MODE_PCIE_2X1;
			default:
				return CVMX_QLM_MODE_DISABLED;
			}
			break;
		}
	}
	return CVMX_QLM_MODE_DISABLED;
}

static enum cvmx_qlm_mode __cvmx_qlm_get_mode_cn78xx(int qlm)
{
	/*
	 * Until gser configuration is in place, we hard code the
	 * qlm mode here. This means that for the time being, this
	 * function and __cvmx_get_mode_cn78xx() have to be in sync
	 * since they are both hard coded. Note that register
	 * CVMX_MIO_QLMX_CFG is not yet modeled by the simulator.
	 */
	switch(qlm) {
	case 0:
	case 1:
		return CVMX_QLM_MODE_SGMII;
	case 4:
		return CVMX_QLM_MODE_ILK;
	case 5:
	case 6:
	case 7:
		return CVMX_QLM_MODE_XAUI;
	}
	return CVMX_QLM_MODE_DISABLED;
}

/*
 * Read QLM and return mode.
 */
enum cvmx_qlm_mode cvmx_qlm_get_mode(int qlm)
{
	if (OCTEON_IS_OCTEON2())
		return __cvmx_qlm_get_mode_cn6xxx(qlm);
	else if (OCTEON_IS_MODEL(OCTEON_CN70XX))
		return __cvmx_qlm_get_mode_cn70xx(qlm);
	else if (OCTEON_IS_MODEL(OCTEON_CN78XX))
		return __cvmx_qlm_get_mode_cn78xx(qlm);

	return CVMX_QLM_MODE_DISABLED;
}

/**
 * Measure the reference clock of a QLM
 *
 * @param qlm    QLM to measure
 *
 * @return Clock rate in Hz
 *       */
int cvmx_qlm_measure_clock(int qlm)
{
	cvmx_mio_ptp_clock_cfg_t ptp_clock;
	uint64_t count;
	uint64_t start_cycle, stop_cycle;
#ifdef CVMX_BUILD_FOR_UBOOT
	int ref_clock[16] = {0};
#else
	static int ref_clock[16] = {0};
#endif

	if (ref_clock[qlm])
		return ref_clock[qlm];

	if (OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN5XXX))
		return -1;

	/* Force the reference to 156.25Mhz when running in simulation.
	   This supports the most speeds */
#ifdef CVMX_BUILD_FOR_UBOOT
	if (gd->board_type == CVMX_BOARD_TYPE_SIM)
		return 156250000;
#elif !defined(CVMX_BUILD_FOR_LINUX_HOST)
	if (cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_SIM)
		return 156250000;
#endif
	/* Disable the PTP event counter while we configure it */
	ptp_clock.u64 = cvmx_read_csr(CVMX_MIO_PTP_CLOCK_CFG);	/* For CN63XXp1 errata */
	ptp_clock.s.evcnt_en = 0;
	cvmx_write_csr(CVMX_MIO_PTP_CLOCK_CFG, ptp_clock.u64);
	/* Count on rising edge, Choose which QLM to count */
	ptp_clock.u64 = cvmx_read_csr(CVMX_MIO_PTP_CLOCK_CFG);	/* For CN63XXp1 errata */
	ptp_clock.s.evcnt_edge = 0;
	ptp_clock.s.evcnt_in = 0x10 + qlm;
	cvmx_write_csr(CVMX_MIO_PTP_CLOCK_CFG, ptp_clock.u64);
	/* Clear MIO_PTP_EVT_CNT */
	cvmx_read_csr(CVMX_MIO_PTP_EVT_CNT);	/* For CN63XXp1 errata */
	count = cvmx_read_csr(CVMX_MIO_PTP_EVT_CNT);
	cvmx_write_csr(CVMX_MIO_PTP_EVT_CNT, -count);
	/* Set MIO_PTP_EVT_CNT to 1 billion */
	cvmx_write_csr(CVMX_MIO_PTP_EVT_CNT, 1000000000);
	/* Enable the PTP event counter */
	ptp_clock.u64 = cvmx_read_csr(CVMX_MIO_PTP_CLOCK_CFG);	/* For CN63XXp1 errata */
	ptp_clock.s.evcnt_en = 1;
	cvmx_write_csr(CVMX_MIO_PTP_CLOCK_CFG, ptp_clock.u64);
	start_cycle = cvmx_clock_get_count(CVMX_CLOCK_CORE);
	/* Wait for 50ms */
	cvmx_wait_usec(50000);
	/* Read the counter */
	cvmx_read_csr(CVMX_MIO_PTP_EVT_CNT);	/* For CN63XXp1 errata */
	count = cvmx_read_csr(CVMX_MIO_PTP_EVT_CNT);
	stop_cycle = cvmx_clock_get_count(CVMX_CLOCK_CORE);
	/* Disable the PTP event counter */
	ptp_clock.u64 = cvmx_read_csr(CVMX_MIO_PTP_CLOCK_CFG);	/* For CN63XXp1 errata */
	ptp_clock.s.evcnt_en = 0;
	cvmx_write_csr(CVMX_MIO_PTP_CLOCK_CFG, ptp_clock.u64);
	/* Clock counted down, so reverse it */
	count = 1000000000 - count;
	/* Return the rate */
	ref_clock[qlm] = count * cvmx_clock_get_rate(CVMX_CLOCK_CORE) / (stop_cycle - start_cycle);
	return ref_clock[qlm];
}

void cvmx_qlm_display_registers(int qlm)
{
	int num_lanes = cvmx_qlm_get_lanes(qlm);
	int lane;
	const __cvmx_qlm_jtag_field_t *ptr = cvmx_qlm_jtag_get_field();

	cvmx_dprintf("%29s", "Field[<stop bit>:<start bit>]");
	for (lane = 0; lane < num_lanes; lane++)
		cvmx_dprintf("\t      Lane %d", lane);
	cvmx_dprintf("\n");

	while (ptr != NULL && ptr->name) {
		cvmx_dprintf("%20s[%3d:%3d]", ptr->name, ptr->stop_bit, ptr->start_bit);
		for (lane = 0; lane < num_lanes; lane++) {
			uint64_t val;
			int tx_byp = 0;
			/* Make sure serdes_tx_byp is set for displaying
			   TX amplitude and TX demphasis field values. */
			if (strncmp(ptr->name, "biasdrv_", 8) == 0 ||
				strncmp(ptr->name, "tcoeff_", 7) == 0) {
				tx_byp = cvmx_qlm_jtag_get(qlm, lane, "serdes_tx_byp");
				if (tx_byp == 0) {
					cvmx_dprintf("\t \t");
					continue;
				}
			}
			val = cvmx_qlm_jtag_get(qlm, lane, ptr->name);
			cvmx_dprintf("\t%4llu (0x%04llx)", (unsigned long long)val, (unsigned long long)val);
		}
		cvmx_dprintf("\n");
		ptr++;
	}
}

/**
 * Due to errata G-720, the 2nd order CDR circuit on CN52XX pass
 * 1 doesn't work properly. The following code disables 2nd order
 * CDR for the specified QLM.
 *
 * @param qlm    QLM to disable 2nd order CDR for.
 */
void __cvmx_helper_errata_qlm_disable_2nd_order_cdr(int qlm)
{
	int lane;
	/* Apply the workaround only once. */
	cvmx_ciu_qlm_jtgd_t qlm_jtgd;
	qlm_jtgd.u64 = cvmx_read_csr(CVMX_CIU_QLM_JTGD);
	if (qlm_jtgd.s.select != 0)
		return;

	cvmx_helper_qlm_jtag_init();
	/* We need to load all four lanes of the QLM, a total of 1072 bits */
	for (lane = 0; lane < 4; lane++) {
		/*
		 * Each lane has 268 bits. We need to set
		 * cfg_cdr_incx<67:64> = 3 and
		 * cfg_cdr_secord<77> = 1.
		 * All other bits are zero. Bits go in LSB first,
		 * so start off with the zeros for bits <63:0>.
		 */
		cvmx_helper_qlm_jtag_shift_zeros(qlm, 63 - 0 + 1);
		/* cfg_cdr_incx<67:64>=3 */
		cvmx_helper_qlm_jtag_shift(qlm, 67 - 64 + 1, 3);
		/* Zeros for bits <76:68> */
		cvmx_helper_qlm_jtag_shift_zeros(qlm, 76 - 68 + 1);
		/* cfg_cdr_secord<77>=1 */
		cvmx_helper_qlm_jtag_shift(qlm, 77 - 77 + 1, 1);
		/* Zeros for bits <267:78> */
		cvmx_helper_qlm_jtag_shift_zeros(qlm, 267 - 78 + 1);
	}
	cvmx_helper_qlm_jtag_update(qlm);
}
