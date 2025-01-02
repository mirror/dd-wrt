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
 * cvmx-ncb-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon ncb.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_NCB_DEFS_H__
#define __CVMX_NCB_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_NCB_ECC_CTL0 CVMX_NCB_ECC_CTL0_FUNC()
static inline uint64_t CVMX_NCB_ECC_CTL0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_NCB_ECC_CTL0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000EFFFD0ull);
}
#else
#define CVMX_NCB_ECC_CTL0 (CVMX_ADD_IO_SEG(0x0001540000EFFFD0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_NCB_ECC_DBE_STS0 CVMX_NCB_ECC_DBE_STS0_FUNC()
static inline uint64_t CVMX_NCB_ECC_DBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_NCB_ECC_DBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000EFFFF0ull);
}
#else
#define CVMX_NCB_ECC_DBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000EFFFF0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_NCB_ECC_DBE_STS_CMB0 CVMX_NCB_ECC_DBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_NCB_ECC_DBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_NCB_ECC_DBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000EFFFD8ull);
}
#else
#define CVMX_NCB_ECC_DBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000EFFFD8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_NCB_ECC_SBE_STS0 CVMX_NCB_ECC_SBE_STS0_FUNC()
static inline uint64_t CVMX_NCB_ECC_SBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_NCB_ECC_SBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000EFFFF8ull);
}
#else
#define CVMX_NCB_ECC_SBE_STS0 (CVMX_ADD_IO_SEG(0x0001540000EFFFF8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_NCB_ECC_SBE_STS_CMB0 CVMX_NCB_ECC_SBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_NCB_ECC_SBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_NCB_ECC_SBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001540000EFFFE8ull);
}
#else
#define CVMX_NCB_ECC_SBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x0001540000EFFFE8ull))
#endif

/**
 * cvmx_ncb_ecc_ctl0
 */
union cvmx_ncb_ecc_ctl0 {
	uint64_t u64;
	struct cvmx_ncb_ecc_ctl0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ncbi_l2_out_ram_flip         : 2;  /**< NCBI_L2_OUT_RAM flip syndrome bits on write. */
	uint64_t ncbi_l2_out_ram_cdis         : 1;  /**< NCBI_L2_OUT_RAM ECC correction disable. */
	uint64_t ncbi_pp_out_ram_flip         : 2;  /**< NCBI_PP_OUT_RAM flip syndrome bits on write. */
	uint64_t ncbi_pp_out_ram_cdis         : 1;  /**< NCBI_PP_OUT_RAM ECC correction disable. */
	uint64_t ncbo_pdm_cmd_dat_ram_flip    : 2;  /**< NCBO_PDM_CMD_DAT_RAM flip syndrome bits on write. */
	uint64_t ncbo_pdm_cmd_dat_ram_cdis    : 1;  /**< NCBO_PDM_CMD_DAT_RAM ECC correction disable. */
	uint64_t ncbi_l2_pdm_pref_ram_flip    : 2;  /**< NCBI_L2_PDM_PREF_RAM flip syndrome bits on write. */
	uint64_t ncbi_l2_pdm_pref_ram_cdis    : 1;  /**< NCBI_L2_PDM_PREF_RAM ECC correction disable. */
	uint64_t ncbo_pp_fif_ram_flip         : 2;  /**< NCBO_PP_FIF_RAM flip syndrome bits on write. */
	uint64_t ncbo_pp_fif_ram_cdis         : 1;  /**< NCBO_PP_FIF_RAM ECC correction disable. */
	uint64_t reserved_0_48                : 49;
#else
	uint64_t reserved_0_48                : 49;
	uint64_t ncbo_pp_fif_ram_cdis         : 1;
	uint64_t ncbo_pp_fif_ram_flip         : 2;
	uint64_t ncbi_l2_pdm_pref_ram_cdis    : 1;
	uint64_t ncbi_l2_pdm_pref_ram_flip    : 2;
	uint64_t ncbo_pdm_cmd_dat_ram_cdis    : 1;
	uint64_t ncbo_pdm_cmd_dat_ram_flip    : 2;
	uint64_t ncbi_pp_out_ram_cdis         : 1;
	uint64_t ncbi_pp_out_ram_flip         : 2;
	uint64_t ncbi_l2_out_ram_cdis         : 1;
	uint64_t ncbi_l2_out_ram_flip         : 2;
#endif
	} s;
	struct cvmx_ncb_ecc_ctl0_s            cn78xx;
};
typedef union cvmx_ncb_ecc_ctl0 cvmx_ncb_ecc_ctl0_t;

/**
 * cvmx_ncb_ecc_dbe_sts0
 */
union cvmx_ncb_ecc_dbe_sts0 {
	uint64_t u64;
	struct cvmx_ncb_ecc_dbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ncbi_l2_out_ram_dbe          : 1;  /**< Double-bit error for NCBI_L2_OUT_RAM. */
	uint64_t ncbi_pp_out_ram_dbe          : 1;  /**< Double-bit error for NCBI_PP_OUT_RAM. */
	uint64_t ncbo_pdm_cmd_dat_ram_dbe     : 1;  /**< Double-bit error for NCBO_PDM_CMD_DAT_RAM. */
	uint64_t ncbi_l2_pdm_pref_ram_dbe     : 1;  /**< Double-bit error for NCBI_L2_PDM_PREF_RAM. */
	uint64_t ncbo_pp_fif_ram_dbe          : 1;  /**< Double-bit error for NCBO_PP_FIF_RAM. */
	uint64_t reserved_0_58                : 59;
#else
	uint64_t reserved_0_58                : 59;
	uint64_t ncbo_pp_fif_ram_dbe          : 1;
	uint64_t ncbi_l2_pdm_pref_ram_dbe     : 1;
	uint64_t ncbo_pdm_cmd_dat_ram_dbe     : 1;
	uint64_t ncbi_pp_out_ram_dbe          : 1;
	uint64_t ncbi_l2_out_ram_dbe          : 1;
#endif
	} s;
	struct cvmx_ncb_ecc_dbe_sts0_s        cn78xx;
};
typedef union cvmx_ncb_ecc_dbe_sts0 cvmx_ncb_ecc_dbe_sts0_t;

/**
 * cvmx_ncb_ecc_dbe_sts_cmb0
 */
union cvmx_ncb_ecc_dbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_ncb_ecc_dbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ncb_dbe_cmb0                 : 1;  /**< Double-bit error for NCBI_L2_OUT_RAM. Throws PKO_INTSN_E::NCB_DBE_CMB0. */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t ncb_dbe_cmb0                 : 1;
#endif
	} s;
	struct cvmx_ncb_ecc_dbe_sts_cmb0_s    cn78xx;
};
typedef union cvmx_ncb_ecc_dbe_sts_cmb0 cvmx_ncb_ecc_dbe_sts_cmb0_t;

/**
 * cvmx_ncb_ecc_sbe_sts0
 */
union cvmx_ncb_ecc_sbe_sts0 {
	uint64_t u64;
	struct cvmx_ncb_ecc_sbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ncbi_l2_out_ram_sbe          : 1;  /**< Single-bit error for NCBI_L2_OUT_RAM. */
	uint64_t ncbi_pp_out_ram_sbe          : 1;  /**< Single-bit error for NCBI_PP_OUT_RAM. */
	uint64_t ncbo_pdm_cmd_dat_ram_sbe     : 1;  /**< Single-bit error for NCBO_PDM_CMD_DAT_RAM. */
	uint64_t ncbi_l2_pdm_pref_ram_sbe     : 1;  /**< Single-bit error for NCBI_L2_PDM_PREF_RAM. */
	uint64_t ncbo_pp_fif_ram_sbe          : 1;  /**< Single-bit error for NCBO_PP_FIF_RAM. */
	uint64_t reserved_0_58                : 59;
#else
	uint64_t reserved_0_58                : 59;
	uint64_t ncbo_pp_fif_ram_sbe          : 1;
	uint64_t ncbi_l2_pdm_pref_ram_sbe     : 1;
	uint64_t ncbo_pdm_cmd_dat_ram_sbe     : 1;
	uint64_t ncbi_pp_out_ram_sbe          : 1;
	uint64_t ncbi_l2_out_ram_sbe          : 1;
#endif
	} s;
	struct cvmx_ncb_ecc_sbe_sts0_s        cn78xx;
};
typedef union cvmx_ncb_ecc_sbe_sts0 cvmx_ncb_ecc_sbe_sts0_t;

/**
 * cvmx_ncb_ecc_sbe_sts_cmb0
 */
union cvmx_ncb_ecc_sbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_ncb_ecc_sbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ncb_sbe_cmb0                 : 1;  /**< Single-bit error for NCBI_L2_OUT_RAM. Throws PKO_INTSN_E::NCB_SBE_CMB0. */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t ncb_sbe_cmb0                 : 1;
#endif
	} s;
	struct cvmx_ncb_ecc_sbe_sts_cmb0_s    cn78xx;
};
typedef union cvmx_ncb_ecc_sbe_sts_cmb0 cvmx_ncb_ecc_sbe_sts_cmb0_t;

#endif
