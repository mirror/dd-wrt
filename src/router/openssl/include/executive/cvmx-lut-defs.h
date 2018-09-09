/***********************license start***************
 * Copyright (c) 2003-2012  Cavium Inc. (support@cavium.com). All rights
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
 * cvmx-lut-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon lut.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_LUT_DEFS_H__
#define __CVMX_LUT_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_LUT_ECC_CTL0 CVMX_LUT_ECC_CTL0_FUNC()
static inline uint64_t CVMX_LUT_ECC_CTL0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_LUT_ECC_CTL0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000BFFFD0ull);
}
#else
#define CVMX_LUT_ECC_CTL0 (CVMX_ADD_IO_SEG(0x0001540000BFFFD0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_LUT_ECC_DBE_STS0 CVMX_LUT_ECC_DBE_STS0_FUNC()
static inline uint64_t CVMX_LUT_ECC_DBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_LUT_ECC_DBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000BFFFF0ull);
}
#else
#define CVMX_LUT_ECC_DBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000BFFFF0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_LUT_ECC_DBE_STS_CMB0 CVMX_LUT_ECC_DBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_LUT_ECC_DBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_LUT_ECC_DBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000BFFFD8ull);
}
#else
#define CVMX_LUT_ECC_DBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000BFFFD8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_LUT_ECC_SBE_STS0 CVMX_LUT_ECC_SBE_STS0_FUNC()
static inline uint64_t CVMX_LUT_ECC_SBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_LUT_ECC_SBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000BFFFF8ull);
}
#else
#define CVMX_LUT_ECC_SBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000BFFFF8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_LUT_ECC_SBE_STS_CMB0 CVMX_LUT_ECC_SBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_LUT_ECC_SBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_LUT_ECC_SBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000BFFFE8ull);
}
#else
#define CVMX_LUT_ECC_SBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000BFFFE8ull))
#endif

/**
 * cvmx_lut_ecc_ctl0
 */
union cvmx_lut_ecc_ctl0 {
	uint64_t u64;
	struct cvmx_lut_ecc_ctl0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t c2q_lut_ram_flip             : 2;  /**< C2Q_LUT_RAM flip syndrome bits on write. */
	uint64_t c2q_lut_ram_cdis             : 1;  /**< C2Q_LUT_RAM ECC correction disable. */
	uint64_t reserved_0_60                : 61;
#else
	uint64_t reserved_0_60                : 61;
	uint64_t c2q_lut_ram_cdis             : 1;
	uint64_t c2q_lut_ram_flip             : 2;
#endif
	} s;
	struct cvmx_lut_ecc_ctl0_s            cn78xx;
};
typedef union cvmx_lut_ecc_ctl0 cvmx_lut_ecc_ctl0_t;

/**
 * cvmx_lut_ecc_dbe_sts0
 */
union cvmx_lut_ecc_dbe_sts0 {
	uint64_t u64;
	struct cvmx_lut_ecc_dbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t c2q_lut_ram_dbe              : 1;  /**< Double-bit error for C2Q_LUT_RAM. */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t c2q_lut_ram_dbe              : 1;
#endif
	} s;
	struct cvmx_lut_ecc_dbe_sts0_s        cn78xx;
};
typedef union cvmx_lut_ecc_dbe_sts0 cvmx_lut_ecc_dbe_sts0_t;

/**
 * cvmx_lut_ecc_dbe_sts_cmb0
 */
union cvmx_lut_ecc_dbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_lut_ecc_dbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t lut_dbe_cmb0                 : 1;  /**< Double-bit error for C2Q_LUT_RAM. Throws PKO_INTSN_E::LUT_DBE_CMB0. */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t lut_dbe_cmb0                 : 1;
#endif
	} s;
	struct cvmx_lut_ecc_dbe_sts_cmb0_s    cn78xx;
};
typedef union cvmx_lut_ecc_dbe_sts_cmb0 cvmx_lut_ecc_dbe_sts_cmb0_t;

/**
 * cvmx_lut_ecc_sbe_sts0
 */
union cvmx_lut_ecc_sbe_sts0 {
	uint64_t u64;
	struct cvmx_lut_ecc_sbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t c2q_lut_ram_sbe              : 1;  /**< Single-bit error for C2Q_LUT_RAM. */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t c2q_lut_ram_sbe              : 1;
#endif
	} s;
	struct cvmx_lut_ecc_sbe_sts0_s        cn78xx;
};
typedef union cvmx_lut_ecc_sbe_sts0 cvmx_lut_ecc_sbe_sts0_t;

/**
 * cvmx_lut_ecc_sbe_sts_cmb0
 */
union cvmx_lut_ecc_sbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_lut_ecc_sbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t lut_sbe_cmb0                 : 1;  /**< Single-bit error for C2Q_LUT_RAM. Throws PKO_INTSN_E::LUT_SBE_CMB0. */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t lut_sbe_cmb0                 : 1;
#endif
	} s;
	struct cvmx_lut_ecc_sbe_sts_cmb0_s    cn78xx;
};
typedef union cvmx_lut_ecc_sbe_sts_cmb0 cvmx_lut_ecc_sbe_sts_cmb0_t;

#endif
