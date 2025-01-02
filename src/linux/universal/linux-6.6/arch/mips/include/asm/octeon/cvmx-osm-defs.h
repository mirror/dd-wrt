/***********************license start***************
 * Copyright (c) 2003-2017  Cavium Inc. (support@cavium.com). All rights
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
 * cvmx-osm-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon osm.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_OSM_DEFS_H__
#define __CVMX_OSM_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_OSM_ASE_RATE_LIMIT_CTRL CVMX_OSM_ASE_RATE_LIMIT_CTRL_FUNC()
static inline uint64_t CVMX_OSM_ASE_RATE_LIMIT_CTRL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_OSM_ASE_RATE_LIMIT_CTRL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DC002100ull);
}
#else
#define CVMX_OSM_ASE_RATE_LIMIT_CTRL (CVMX_ADD_IO_SEG(0x00011800DC002100ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OSM_BANKX_CTRL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 63))
					return CVMX_ADD_IO_SEG(0x00011800DC001000ull) + ((offset) & 63) * 8;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 63))
					return CVMX_ADD_IO_SEG(0x00011800DC001000ull) + ((offset) & 63) * 8;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 31))
				return CVMX_ADD_IO_SEG(0x0001DC0000001000ull) + ((offset) & 31) * 8;
			break;
	}
	cvmx_warn("CVMX_OSM_BANKX_CTRL (offset = %lu) not supported on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800DC001000ull) + ((offset) & 63) * 8;
}
#else
static inline uint64_t CVMX_OSM_BANKX_CTRL(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return CVMX_ADD_IO_SEG(0x00011800DC001000ull) + (offset) * 8;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return CVMX_ADD_IO_SEG(0x00011800DC001000ull) + (offset) * 8;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001DC0000001000ull) + (offset) * 8;
	}
	return CVMX_ADD_IO_SEG(0x00011800DC001000ull) + (offset) * 8;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_OSM_CLK_CFG CVMX_OSM_CLK_CFG_FUNC()
static inline uint64_t CVMX_OSM_CLK_CFG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_OSM_CLK_CFG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001DC0000000028ull);
}
#else
#define CVMX_OSM_CLK_CFG (CVMX_ADD_IO_SEG(0x0001DC0000000028ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_OSM_ECC_CTRL CVMX_OSM_ECC_CTRL_FUNC()
static inline uint64_t CVMX_OSM_ECC_CTRL_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return CVMX_ADD_IO_SEG(0x00011800DC000020ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return CVMX_ADD_IO_SEG(0x00011800DC000020ull);

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001DC0000000020ull);
			break;
	}
	cvmx_warn("CVMX_OSM_ECC_CTRL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DC000020ull);
}
#else
#define CVMX_OSM_ECC_CTRL CVMX_OSM_ECC_CTRL_FUNC()
static inline uint64_t CVMX_OSM_ECC_CTRL_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return CVMX_ADD_IO_SEG(0x00011800DC000020ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return CVMX_ADD_IO_SEG(0x00011800DC000020ull);

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001DC0000000020ull);
	}
	return CVMX_ADD_IO_SEG(0x00011800DC000020ull);
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_OSM_ECO CVMX_OSM_ECO_FUNC()
static inline uint64_t CVMX_OSM_ECO_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return CVMX_ADD_IO_SEG(0x00011800DC003000ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return CVMX_ADD_IO_SEG(0x00011800DC003000ull);

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001DC0000003000ull);
			break;
	}
	cvmx_warn("CVMX_OSM_ECO not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DC003000ull);
}
#else
#define CVMX_OSM_ECO CVMX_OSM_ECO_FUNC()
static inline uint64_t CVMX_OSM_ECO_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return CVMX_ADD_IO_SEG(0x00011800DC003000ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return CVMX_ADD_IO_SEG(0x00011800DC003000ull);

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001DC0000003000ull);
	}
	return CVMX_ADD_IO_SEG(0x00011800DC003000ull);
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_OSM_INT_INFO_ADDR CVMX_OSM_INT_INFO_ADDR_FUNC()
static inline uint64_t CVMX_OSM_INT_INFO_ADDR_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return CVMX_ADD_IO_SEG(0x00011800DC000018ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return CVMX_ADD_IO_SEG(0x00011800DC000018ull);

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001DC0000000018ull);
			break;
	}
	cvmx_warn("CVMX_OSM_INT_INFO_ADDR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DC000018ull);
}
#else
#define CVMX_OSM_INT_INFO_ADDR CVMX_OSM_INT_INFO_ADDR_FUNC()
static inline uint64_t CVMX_OSM_INT_INFO_ADDR_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return CVMX_ADD_IO_SEG(0x00011800DC000018ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return CVMX_ADD_IO_SEG(0x00011800DC000018ull);

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001DC0000000018ull);
	}
	return CVMX_ADD_IO_SEG(0x00011800DC000018ull);
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_OSM_INT_INFO_ECC CVMX_OSM_INT_INFO_ECC_FUNC()
static inline uint64_t CVMX_OSM_INT_INFO_ECC_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return CVMX_ADD_IO_SEG(0x00011800DC000010ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return CVMX_ADD_IO_SEG(0x00011800DC000010ull);

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001DC0000000010ull);
			break;
	}
	cvmx_warn("CVMX_OSM_INT_INFO_ECC not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DC000010ull);
}
#else
#define CVMX_OSM_INT_INFO_ECC CVMX_OSM_INT_INFO_ECC_FUNC()
static inline uint64_t CVMX_OSM_INT_INFO_ECC_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return CVMX_ADD_IO_SEG(0x00011800DC000010ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return CVMX_ADD_IO_SEG(0x00011800DC000010ull);

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001DC0000000010ull);
	}
	return CVMX_ADD_IO_SEG(0x00011800DC000010ull);
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_OSM_INT_STAT CVMX_OSM_INT_STAT_FUNC()
static inline uint64_t CVMX_OSM_INT_STAT_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return CVMX_ADD_IO_SEG(0x00011800DC000008ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return CVMX_ADD_IO_SEG(0x00011800DC000008ull);

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001DC0000000008ull);
			break;
	}
	cvmx_warn("CVMX_OSM_INT_STAT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DC000008ull);
}
#else
#define CVMX_OSM_INT_STAT CVMX_OSM_INT_STAT_FUNC()
static inline uint64_t CVMX_OSM_INT_STAT_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return CVMX_ADD_IO_SEG(0x00011800DC000008ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return CVMX_ADD_IO_SEG(0x00011800DC000008ull);

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001DC0000000008ull);
	}
	return CVMX_ADD_IO_SEG(0x00011800DC000008ull);
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OSM_MEMX_BIST_STATUS(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 7))
					return CVMX_ADD_IO_SEG(0x00011800DC002000ull) + ((offset) & 7) * 8;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 7))
					return CVMX_ADD_IO_SEG(0x00011800DC002000ull) + ((offset) & 7) * 8;

			break;
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return CVMX_ADD_IO_SEG(0x0001DC0000002000ull) + ((offset) & 1) * 8;
			break;
	}
	cvmx_warn("CVMX_OSM_MEMX_BIST_STATUS (offset = %lu) not supported on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800DC002000ull) + ((offset) & 7) * 8;
}
#else
static inline uint64_t CVMX_OSM_MEMX_BIST_STATUS(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return CVMX_ADD_IO_SEG(0x00011800DC002000ull) + (offset) * 8;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return CVMX_ADD_IO_SEG(0x00011800DC002000ull) + (offset) * 8;

		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001DC0000002000ull) + (offset) * 8;
	}
	return CVMX_ADD_IO_SEG(0x00011800DC002000ull) + (offset) * 8;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OSM_MEMX_DX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset <= 3)) && ((block_id <= 16383))))))
		cvmx_warn("CVMX_OSM_MEMX_DX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001DC0800000000ull) + (((offset) & 3) + ((block_id) & 16383) * 0x4ull) * 8;
}
#else
#define CVMX_OSM_MEMX_DX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001DC0800000000ull) + (((offset) & 3) + ((block_id) & 16383) * 0x4ull) * 8)
#endif

/**
 * cvmx_osm_ase_rate_limit_ctrl
 */
union cvmx_osm_ase_rate_limit_ctrl {
	uint64_t u64;
	struct cvmx_osm_ase_rate_limit_ctrl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t rwc_rate_limit               : 1;  /**< Reserved. */
	uint64_t bwc_rate_limit               : 1;  /**< Reserved. */
	uint64_t twc_rate_limit               : 1;  /**< Reserved. */
#else
	uint64_t twc_rate_limit               : 1;
	uint64_t bwc_rate_limit               : 1;
	uint64_t rwc_rate_limit               : 1;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_osm_ase_rate_limit_ctrl_s cn78xx;
	struct cvmx_osm_ase_rate_limit_ctrl_s cn78xxp1;
};
typedef union cvmx_osm_ase_rate_limit_ctrl cvmx_osm_ase_rate_limit_ctrl_t;

/**
 * cvmx_osm_bank#_ctrl
 */
union cvmx_osm_bankx_ctrl {
	uint64_t u64;
	struct cvmx_osm_bankx_ctrl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t bank_assign                  : 3;  /**< Port assignment for each memory bank. Memory structure is 64k words * 246 data bits (plus
                                                         ECC). This is further divided into 64 banks each containing 1k words * 246 data bits. A
                                                         bank can only support one access per cycle, this is implemented by assigning each bank to
                                                         a specific requester. Most requesters can only make one request per cycle so that mostly
                                                         solves the problem. RWC is the only requester that can make multiple requests per cycle
                                                         and will need to implement its own bank-aware scheduler to prevent bank conflicts. Bank
                                                         assignment can be reconfigured dynamically, but memory accesses to a bank must be quiesced
                                                         before that bank can be reassigned to another requester. A host request can access any
                                                         bank; arbitration logic will prevent bank conflicts for host requests.
                                                         Addresses: bit<15:10> = bank, bit<9:0> = offset.
                                                         Bank 0 corresponds to memory address 0x0000-0x03FF.
                                                         Bank 1 corresponds to memory address 0x0400-0x07FF.
                                                         Bank 63 corresponds to memory address 0xFC00-0xFFFF.
                                                         See OSM_BANK_ASSIGN_E for encoding. */
#else
	uint64_t bank_assign                  : 3;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_osm_bankx_ctrl_s          cn73xx;
	struct cvmx_osm_bankx_ctrl_s          cn78xx;
	struct cvmx_osm_bankx_ctrl_s          cn78xxp1;
};
typedef union cvmx_osm_bankx_ctrl cvmx_osm_bankx_ctrl_t;

/**
 * cvmx_osm_clk_cfg
 *
 * This is the general configuration register for the OSM block.
 *
 */
union cvmx_osm_clk_cfg {
	uint64_t u64;
	struct cvmx_osm_clk_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t clken                        : 1;  /**< Enable OSM clocks. Clear to disable clocking to disallow OSM accesses and save power. */
#else
	uint64_t clken                        : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_osm_clk_cfg_s             cn73xx;
};
typedef union cvmx_osm_clk_cfg cvmx_osm_clk_cfg_t;

/**
 * cvmx_osm_ecc_ctrl
 *
 * ECC control register.
 *
 */
union cvmx_osm_ecc_ctrl {
	uint64_t u64;
	struct cvmx_osm_ecc_ctrl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t flip_synd                    : 2;  /**< Flip syndrom<1:0> bits to generate 1-bit/2-bits error for testing.
                                                         0x0 = Normal operation.
                                                         0x1 = SBE on bit<0>.
                                                         0x2 = SBE on bit<1>.
                                                         0x3 = DBE on bits<1:0>. */
	uint64_t cor_dis                      : 1;  /**< Disables SBE correction. SBE/DBE are still detected. */
#else
	uint64_t cor_dis                      : 1;
	uint64_t flip_synd                    : 2;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_osm_ecc_ctrl_s            cn73xx;
	struct cvmx_osm_ecc_ctrl_s            cn78xx;
	struct cvmx_osm_ecc_ctrl_s            cn78xxp1;
};
typedef union cvmx_osm_ecc_ctrl cvmx_osm_ecc_ctrl_t;

/**
 * cvmx_osm_eco
 */
union cvmx_osm_eco {
	uint64_t u64;
	struct cvmx_osm_eco_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t eco_ro                       : 8;  /**< N/A */
	uint64_t eco_rw                       : 8;  /**< N/A */
#else
	uint64_t eco_rw                       : 8;
	uint64_t eco_ro                       : 8;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_osm_eco_s                 cn73xx;
	struct cvmx_osm_eco_s                 cn78xx;
	struct cvmx_osm_eco_s                 cn78xxp1;
};
typedef union cvmx_osm_eco cvmx_osm_eco_t;

/**
 * cvmx_osm_int_info_addr
 *
 * This register can be used to debug address errors (illegal bank). Fields are captured when
 * there are no outstanding address errors indicated in OSM_INT_STAT and a new address error
 * arrives. Prioritization for multiple events occurring at the same time is indicated by the
 * OSM_ADDR_ERR_SOURCE_E enumeration; highest encoded value has highest priority.
 */
union cvmx_osm_int_info_addr {
	uint64_t u64;
	struct cvmx_osm_int_info_addr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_35_63               : 29;
	uint64_t addr_err_source              : 3;  /**< Source of address error; see OSM_ADDR_ERR_SOURCE_E */
	uint64_t reserved_16_31               : 16;
	uint64_t addr_err_address             : 16; /**< RAM address of the address error. */
#else
	uint64_t addr_err_address             : 16;
	uint64_t reserved_16_31               : 16;
	uint64_t addr_err_source              : 3;
	uint64_t reserved_35_63               : 29;
#endif
	} s;
	struct cvmx_osm_int_info_addr_s       cn73xx;
	struct cvmx_osm_int_info_addr_s       cn78xx;
	struct cvmx_osm_int_info_addr_s       cn78xxp1;
};
typedef union cvmx_osm_int_info_addr cvmx_osm_int_info_addr_t;

/**
 * cvmx_osm_int_info_ecc
 *
 * This register can be used to debug ECC failures. Fields are captured when there are no
 * outstanding ECC errors indicated in OSM_INT_STAT and a new ECC error arrives. Prioritization
 * for multiple events occurring at the same time is indicated by the OSM_ECC_ERR_SOURCE_E
 * enumeration; highest encoded value has highest priority. For current bank assignment, see
 * OSM_BANK()_CTRL.
 */
union cvmx_osm_int_info_ecc {
	uint64_t u64;
	struct cvmx_osm_int_info_ecc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_37_63               : 27;
	uint64_t ecc_err_source               : 5;  /**< Source of ECC error; see OSM_ECC_ERR_SOURCE_E. */
	uint64_t reserved_16_31               : 16;
	uint64_t ecc_err_address              : 16; /**< RAM address of the ECC error. */
#else
	uint64_t ecc_err_address              : 16;
	uint64_t reserved_16_31               : 16;
	uint64_t ecc_err_source               : 5;
	uint64_t reserved_37_63               : 27;
#endif
	} s;
	struct cvmx_osm_int_info_ecc_s        cn73xx;
	struct cvmx_osm_int_info_ecc_s        cn78xx;
	struct cvmx_osm_int_info_ecc_s        cn78xxp1;
};
typedef union cvmx_osm_int_info_ecc cvmx_osm_int_info_ecc_t;

/**
 * cvmx_osm_int_stat
 *
 * For debugging output for ECC DBE/SBEs, see OSM_INT_INFO_ECC. Address errors happen when a
 * requester attempts to access a bank that was not assigned to it. For example, Bank 0 is
 * assigned to HFA, and HNA attempts to access it. For debugging output for address errors, see
 * OSM_INT_INFO_ADDR. For current bank assignment, see OSM_BANK()_CTRL.
 */
union cvmx_osm_int_stat {
	uint64_t u64;
	struct cvmx_osm_int_stat_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_23_63               : 41;
	uint64_t rwc3_addr_err                : 1;  /**< RWC3 port illegal bank address error. */
	uint64_t rwc2_addr_err                : 1;  /**< RWC2 port illegal bank address error. */
	uint64_t rwc1_addr_err                : 1;  /**< RWC1 port illegal bank address error. */
	uint64_t bwc_addr_err                 : 1;  /**< BWC port illegal bank address error. */
	uint64_t twc_addr_err                 : 1;  /**< TWC port illegal bank address error. */
	uint64_t hna_addr_err                 : 1;  /**< HNA port illegal bank address error. */
	uint64_t dfa_addr_err                 : 1;  /**< HFA port illegal bank address error. */
	uint64_t host_sbe                     : 1;  /**< Host port single-bit error. */
	uint64_t host_dbe                     : 1;  /**< Host port double-bit error. */
	uint64_t rwc3_sbe                     : 1;  /**< ASE RWC3 port single-bit error. */
	uint64_t rwc3_dbe                     : 1;  /**< ASE RWC3 port double-bit error. */
	uint64_t rwc2_sbe                     : 1;  /**< ASE RWC2 port single-bit error. */
	uint64_t rwc2_dbe                     : 1;  /**< ASE RWC2 port double-bit error. */
	uint64_t rwc1_sbe                     : 1;  /**< ASE RWC1 port single-bit error. */
	uint64_t rwc1_dbe                     : 1;  /**< ASE RWC1 port double-bit error. */
	uint64_t bwc_sbe                      : 1;  /**< ASE BWC port single-bit error. */
	uint64_t bwc_dbe                      : 1;  /**< ASE BWC port double-bit error. */
	uint64_t twc_sbe                      : 1;  /**< ASE TWC port single-bit error. */
	uint64_t twc_dbe                      : 1;  /**< ASE TWC port double-bit error. */
	uint64_t hna_sbe                      : 1;  /**< HNA port single-bit error. */
	uint64_t hna_dbe                      : 1;  /**< HNA port double-bit error. */
	uint64_t dfa_sbe                      : 1;  /**< HFA port single-bit error. */
	uint64_t dfa_dbe                      : 1;  /**< HFA port double-bit error. */
#else
	uint64_t dfa_dbe                      : 1;
	uint64_t dfa_sbe                      : 1;
	uint64_t hna_dbe                      : 1;
	uint64_t hna_sbe                      : 1;
	uint64_t twc_dbe                      : 1;
	uint64_t twc_sbe                      : 1;
	uint64_t bwc_dbe                      : 1;
	uint64_t bwc_sbe                      : 1;
	uint64_t rwc1_dbe                     : 1;
	uint64_t rwc1_sbe                     : 1;
	uint64_t rwc2_dbe                     : 1;
	uint64_t rwc2_sbe                     : 1;
	uint64_t rwc3_dbe                     : 1;
	uint64_t rwc3_sbe                     : 1;
	uint64_t host_dbe                     : 1;
	uint64_t host_sbe                     : 1;
	uint64_t dfa_addr_err                 : 1;
	uint64_t hna_addr_err                 : 1;
	uint64_t twc_addr_err                 : 1;
	uint64_t bwc_addr_err                 : 1;
	uint64_t rwc1_addr_err                : 1;
	uint64_t rwc2_addr_err                : 1;
	uint64_t rwc3_addr_err                : 1;
	uint64_t reserved_23_63               : 41;
#endif
	} s;
	struct cvmx_osm_int_stat_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t hna_addr_err                 : 1;  /**< HNA port illegal bank address error. */
	uint64_t dfa_addr_err                 : 1;  /**< HFA port illegal bank address error. */
	uint64_t reserved_4_15                : 12;
	uint64_t hna_sbe                      : 1;  /**< HNA port single-bit error. */
	uint64_t hna_dbe                      : 1;  /**< HNA port double-bit error. */
	uint64_t dfa_sbe                      : 1;  /**< HFA port single-bit error. */
	uint64_t dfa_dbe                      : 1;  /**< HFA port double-bit error. */
#else
	uint64_t dfa_dbe                      : 1;
	uint64_t dfa_sbe                      : 1;
	uint64_t hna_dbe                      : 1;
	uint64_t hna_sbe                      : 1;
	uint64_t reserved_4_15                : 12;
	uint64_t dfa_addr_err                 : 1;
	uint64_t hna_addr_err                 : 1;
	uint64_t reserved_18_63               : 46;
#endif
	} cn73xx;
	struct cvmx_osm_int_stat_s            cn78xx;
	struct cvmx_osm_int_stat_s            cn78xxp1;
};
typedef union cvmx_osm_int_stat cvmx_osm_int_stat_t;

/**
 * cvmx_osm_mem#_bist_status
 *
 * Results from BIST runs of OSM's memories. OSM_MEM is instantiated 2 times, each instance of
 * OSM_MEM has its own BIST_STATUS. Each OSM_MEM contains 1 BIST memory instances, so there are
 * 1 status bit per register.
 */
union cvmx_osm_memx_bist_status {
	uint64_t u64;
	struct cvmx_osm_memx_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t bist_status                  : 32; /**< BIST status with one bit corresponding to each BIST memory instance. */
#else
	uint64_t bist_status                  : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_osm_memx_bist_status_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t bist_status                  : 1;  /**< BIST status with one bit corresponding to each BIST memory instance. */
#else
	uint64_t bist_status                  : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} cn73xx;
	struct cvmx_osm_memx_bist_status_s    cn78xx;
	struct cvmx_osm_memx_bist_status_s    cn78xxp1;
};
typedef union cvmx_osm_memx_bist_status cvmx_osm_memx_bist_status_t;

/**
 * cvmx_osm_mem#_d#
 */
union cvmx_osm_memx_dx {
	uint64_t u64;
	struct cvmx_osm_memx_dx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t mem                          : 64; /**< OSM memory. THe last index in this memory corresponds to the row of data stored in OSM:
                                                         D(0) = Data<63:0>.
                                                         D(1) = Data<127:64>.
                                                         D(2) = Data<191:128>.
                                                         D(3) <63> = RAZ.
                                                         D(3) <62:54> = ECC.(RO)
                                                         D(3) <53:0> = Data<245:192>. */
#else
	uint64_t mem                          : 64;
#endif
	} s;
	struct cvmx_osm_memx_dx_s             cn73xx;
};
typedef union cvmx_osm_memx_dx cvmx_osm_memx_dx_t;

#endif
