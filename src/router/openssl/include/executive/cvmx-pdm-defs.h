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
 * cvmx-pdm-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon pdm.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_PDM_DEFS_H__
#define __CVMX_PDM_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PDM_ECC_CTL0 CVMX_PDM_ECC_CTL0_FUNC()
static inline uint64_t CVMX_PDM_ECC_CTL0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PDM_ECC_CTL0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400008FFFD0ull);
}
#else
#define CVMX_PDM_ECC_CTL0 (CVMX_ADD_IO_SEG(0x00015400008FFFD0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PDM_ECC_DBE_STS0 CVMX_PDM_ECC_DBE_STS0_FUNC()
static inline uint64_t CVMX_PDM_ECC_DBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PDM_ECC_DBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400008FFFF0ull);
}
#else
#define CVMX_PDM_ECC_DBE_STS0 (CVMX_ADD_IO_SEG(0x00015400008FFFF0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PDM_ECC_DBE_STS_CMB0 CVMX_PDM_ECC_DBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PDM_ECC_DBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PDM_ECC_DBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400008FFFD8ull);
}
#else
#define CVMX_PDM_ECC_DBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x00015400008FFFD8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PDM_ECC_SBE_STS0 CVMX_PDM_ECC_SBE_STS0_FUNC()
static inline uint64_t CVMX_PDM_ECC_SBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PDM_ECC_SBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400008FFFF8ull);
}
#else
#define CVMX_PDM_ECC_SBE_STS0 (CVMX_ADD_IO_SEG(0x00015400008FFFF8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PDM_ECC_SBE_STS_CMB0 CVMX_PDM_ECC_SBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PDM_ECC_SBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PDM_ECC_SBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400008FFFE8ull);
}
#else
#define CVMX_PDM_ECC_SBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x00015400008FFFE8ull))
#endif

/**
 * cvmx_pdm_ecc_ctl0
 */
union cvmx_pdm_ecc_ctl0 {
	uint64_t u64;
	struct cvmx_pdm_ecc_ctl0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t flshb_cache_lo_ram_flip      : 2;  /**< FLSHB_CACHE_LO_RAM flip syndrome bits on write. */
	uint64_t flshb_cache_lo_ram_cdis      : 1;  /**< FLSHB_CACHE_LO_RAM ECC correction disable. */
	uint64_t flshb_cache_hi_ram_flip      : 2;  /**< FLSHB_CACHE_HI_RAM flip syndrome bits on write. */
	uint64_t flshb_cache_hi_ram_cdis      : 1;  /**< FLSHB_CACHE_HI_RAM ECC correction disable. */
	uint64_t isrd_o_ram_flip              : 2;  /**< ISRD_O_RAM flip syndrome bits on write. */
	uint64_t isrd_o_ram_cdis              : 1;  /**< ISRD_O_RAM ECC correction disable. */
	uint64_t isrm_ca_iinst_ram_flip       : 2;  /**< ISRM_CA_IINST_RAM flip syndrome bits on write. */
	uint64_t isrm_ca_iinst_ram_cdis       : 1;  /**< ISRM_CA_IINST_RAM ECC correction disable. */
	uint64_t isrm_ca_cm_ram_flip          : 2;  /**< ISRM_CA_CM_RAM flip syndrome bits on write. */
	uint64_t isrm_ca_cm_ram_cdis          : 1;  /**< ISRM_CA_CM_RAM ECC correction disable. */
	uint64_t isrd_t_ram_flip              : 2;  /**< ISRD_T_RAM flip syndrome bits on write. */
	uint64_t isrd_t_ram_cdis              : 1;  /**< ISRD_T_RAM ECC correction disable. */
	uint64_t isrd_h_ram_flip              : 2;  /**< ISRD_H_RAM flip syndrome bits on write. */
	uint64_t isrd_h_ram_cdis              : 1;  /**< ISRD_H_RAM ECC correction disable. */
	uint64_t isrm_t_ram_flip              : 2;  /**< ISRM_T_RAM flip syndrome bits on write. */
	uint64_t isrm_t_ram_cdis              : 1;  /**< ISRM_T_RAM ECC correction disable. */
	uint64_t isrm_h_ram_flip              : 2;  /**< ISRM_H_RAM flip syndrome bits on write. */
	uint64_t isrm_h_ram_cdis              : 1;  /**< ISRM_H_RAM ECC correction disable. */
	uint64_t isrm_o_ram_flip              : 2;  /**< ISRM_O_RAM flip syndrome bits on write. */
	uint64_t isrm_o_ram_cdis              : 1;  /**< ISRM_O_RAM ECC correction disable. */
	uint64_t drp_hi_ram_flip              : 2;  /**< DRP_HI_RAM flip syndrome bits on write. */
	uint64_t drp_hi_ram_cdis              : 1;  /**< DRP_HI_RAM ECC correction disable. */
	uint64_t drp_lo_ram_flip              : 2;  /**< DRP_LO_RAM flip syndrome bits on write. */
	uint64_t drp_lo_ram_cdis              : 1;  /**< DRP_LO_RAM ECC correction disable. */
	uint64_t dwp_hi_ram_flip              : 2;  /**< DWP_HI_RAM flip syndrome bits on write. */
	uint64_t dwp_hi_ram_cdis              : 1;  /**< DWP_HI_RAM ECC correction disable. */
	uint64_t dwp_lo_ram_flip              : 2;  /**< DWP_LO_RAM flip syndrome bits on write. */
	uint64_t dwp_lo_ram_cdis              : 1;  /**< DWP_LO_RAM ECC correction disable. */
	uint64_t mwp_hi_ram_flip              : 2;  /**< MWP_HI_RAM flip syndrome bits on write. */
	uint64_t mwp_hi_ram_cdis              : 1;  /**< MWP_HI_RAM ECC correction disable. */
	uint64_t mwp_lo_ram_flip              : 2;  /**< MWP_LO_RAM flip syndrome bits on write. */
	uint64_t mwp_lo_ram_cdis              : 1;  /**< MWP_LO_RAM ECC correction disable. */
	uint64_t fillb_m_dat_ram_flip         : 2;  /**< FILLB_M_DAT_RAM flip syndrome bits on write. */
	uint64_t fillb_m_dat_ram_cdis         : 1;  /**< FILLB_M_DAT_RAM ECC correction disable. */
	uint64_t fillb_d_dat_ram_flip         : 2;  /**< FILLB_D_DAT_RAM flip syndrome bits on write. */
	uint64_t fillb_d_dat_ram_cdis         : 1;  /**< FILLB_D_DAT_RAM ECC correction disable. */
	uint64_t reserved_0_9                 : 10;
#else
	uint64_t reserved_0_9                 : 10;
	uint64_t fillb_d_dat_ram_cdis         : 1;
	uint64_t fillb_d_dat_ram_flip         : 2;
	uint64_t fillb_m_dat_ram_cdis         : 1;
	uint64_t fillb_m_dat_ram_flip         : 2;
	uint64_t mwp_lo_ram_cdis              : 1;
	uint64_t mwp_lo_ram_flip              : 2;
	uint64_t mwp_hi_ram_cdis              : 1;
	uint64_t mwp_hi_ram_flip              : 2;
	uint64_t dwp_lo_ram_cdis              : 1;
	uint64_t dwp_lo_ram_flip              : 2;
	uint64_t dwp_hi_ram_cdis              : 1;
	uint64_t dwp_hi_ram_flip              : 2;
	uint64_t drp_lo_ram_cdis              : 1;
	uint64_t drp_lo_ram_flip              : 2;
	uint64_t drp_hi_ram_cdis              : 1;
	uint64_t drp_hi_ram_flip              : 2;
	uint64_t isrm_o_ram_cdis              : 1;
	uint64_t isrm_o_ram_flip              : 2;
	uint64_t isrm_h_ram_cdis              : 1;
	uint64_t isrm_h_ram_flip              : 2;
	uint64_t isrm_t_ram_cdis              : 1;
	uint64_t isrm_t_ram_flip              : 2;
	uint64_t isrd_h_ram_cdis              : 1;
	uint64_t isrd_h_ram_flip              : 2;
	uint64_t isrd_t_ram_cdis              : 1;
	uint64_t isrd_t_ram_flip              : 2;
	uint64_t isrm_ca_cm_ram_cdis          : 1;
	uint64_t isrm_ca_cm_ram_flip          : 2;
	uint64_t isrm_ca_iinst_ram_cdis       : 1;
	uint64_t isrm_ca_iinst_ram_flip       : 2;
	uint64_t isrd_o_ram_cdis              : 1;
	uint64_t isrd_o_ram_flip              : 2;
	uint64_t flshb_cache_hi_ram_cdis      : 1;
	uint64_t flshb_cache_hi_ram_flip      : 2;
	uint64_t flshb_cache_lo_ram_cdis      : 1;
	uint64_t flshb_cache_lo_ram_flip      : 2;
#endif
	} s;
	struct cvmx_pdm_ecc_ctl0_s            cn78xx;
};
typedef union cvmx_pdm_ecc_ctl0 cvmx_pdm_ecc_ctl0_t;

/**
 * cvmx_pdm_ecc_dbe_sts0
 */
union cvmx_pdm_ecc_dbe_sts0 {
	uint64_t u64;
	struct cvmx_pdm_ecc_dbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t flshb_cache_lo_ram_dbe       : 1;  /**< Double-bit error for FLSHB_CACHE_LO_RAM. */
	uint64_t flshb_cache_hi_ram_dbe       : 1;  /**< Double-bit error for FLSHB_CACHE_HI_RAM. */
	uint64_t isrd_o_ram_dbe               : 1;  /**< Double-bit error for ISRD_O_RAM. */
	uint64_t isrm_ca_iinst_ram_dbe        : 1;  /**< Double-bit error for ISRM_CA_IINST_RAM. */
	uint64_t isrm_ca_cm_ram_dbe           : 1;  /**< Double-bit error for ISRM_CA_CM_RAM. */
	uint64_t isrd_t_ram_dbe               : 1;  /**< Double-bit error for ISRD_T_RAM. */
	uint64_t isrd_h_ram_dbe               : 1;  /**< Double-bit error for ISRD_H_RAM. */
	uint64_t isrm_t_ram_dbe               : 1;  /**< Double-bit error for ISRM_T_RAM. */
	uint64_t isrm_h_ram_dbe               : 1;  /**< Double-bit error for ISRM_H_RAM. */
	uint64_t isrm_o_ram_dbe               : 1;  /**< Double-bit error for ISRM_O_RAM. */
	uint64_t drp_hi_ram_dbe               : 1;  /**< Double-bit error for DRP_HI_RAM. */
	uint64_t drp_lo_ram_dbe               : 1;  /**< Double-bit error for DRP_LO_RAM. */
	uint64_t dwp_hi_ram_dbe               : 1;  /**< Double-bit error for DWP_HI_RAM. */
	uint64_t dwp_lo_ram_dbe               : 1;  /**< Double-bit error for DWP_LO_RAM. */
	uint64_t mwp_hi_ram_dbe               : 1;  /**< Double-bit error for MWP_HI_RAM. */
	uint64_t mwp_lo_ram_dbe               : 1;  /**< Double-bit error for MWP_LO_RAM. */
	uint64_t fillb_m_dat_ram_dbe          : 1;  /**< Double-bit error for FILLB_M_DAT_RAM. */
	uint64_t fillb_d_dat_ram_dbe          : 1;  /**< Double-bit error for FILLB_D_DAT_RAM. */
	uint64_t reserved_0_45                : 46;
#else
	uint64_t reserved_0_45                : 46;
	uint64_t fillb_d_dat_ram_dbe          : 1;
	uint64_t fillb_m_dat_ram_dbe          : 1;
	uint64_t mwp_lo_ram_dbe               : 1;
	uint64_t mwp_hi_ram_dbe               : 1;
	uint64_t dwp_lo_ram_dbe               : 1;
	uint64_t dwp_hi_ram_dbe               : 1;
	uint64_t drp_lo_ram_dbe               : 1;
	uint64_t drp_hi_ram_dbe               : 1;
	uint64_t isrm_o_ram_dbe               : 1;
	uint64_t isrm_h_ram_dbe               : 1;
	uint64_t isrm_t_ram_dbe               : 1;
	uint64_t isrd_h_ram_dbe               : 1;
	uint64_t isrd_t_ram_dbe               : 1;
	uint64_t isrm_ca_cm_ram_dbe           : 1;
	uint64_t isrm_ca_iinst_ram_dbe        : 1;
	uint64_t isrd_o_ram_dbe               : 1;
	uint64_t flshb_cache_hi_ram_dbe       : 1;
	uint64_t flshb_cache_lo_ram_dbe       : 1;
#endif
	} s;
	struct cvmx_pdm_ecc_dbe_sts0_s        cn78xx;
};
typedef union cvmx_pdm_ecc_dbe_sts0 cvmx_pdm_ecc_dbe_sts0_t;

/**
 * cvmx_pdm_ecc_dbe_sts_cmb0
 */
union cvmx_pdm_ecc_dbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pdm_ecc_dbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pdm_dbe_cmb0                 : 1;  /**< Double-bit error for FLSHB_CACHE_LO_RAM. Throws PKO_INTSN_E::PDM_DBE_CMB0. */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t pdm_dbe_cmb0                 : 1;
#endif
	} s;
	struct cvmx_pdm_ecc_dbe_sts_cmb0_s    cn78xx;
};
typedef union cvmx_pdm_ecc_dbe_sts_cmb0 cvmx_pdm_ecc_dbe_sts_cmb0_t;

/**
 * cvmx_pdm_ecc_sbe_sts0
 */
union cvmx_pdm_ecc_sbe_sts0 {
	uint64_t u64;
	struct cvmx_pdm_ecc_sbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t flshb_cache_lo_ram_sbe       : 1;  /**< Single-bit error for FLSHB_CACHE_LO_RAM. */
	uint64_t flshb_cache_hi_ram_sbe       : 1;  /**< Single-bit error for FLSHB_CACHE_HI_RAM. */
	uint64_t isrd_o_ram_sbe               : 1;  /**< Single-bit error for ISRD_O_RAM. */
	uint64_t isrm_ca_iinst_ram_sbe        : 1;  /**< Single-bit error for ISRM_CA_IINST_RAM. */
	uint64_t isrm_ca_cm_ram_sbe           : 1;  /**< Single-bit error for ISRM_CA_CM_RAM. */
	uint64_t isrd_t_ram_sbe               : 1;  /**< Single-bit error for ISRD_T_RAM. */
	uint64_t isrd_h_ram_sbe               : 1;  /**< Single-bit error for ISRD_H_RAM. */
	uint64_t isrm_t_ram_sbe               : 1;  /**< Single-bit error for ISRM_T_RAM. */
	uint64_t isrm_h_ram_sbe               : 1;  /**< Single-bit error for ISRM_H_RAM. */
	uint64_t isrm_o_ram_sbe               : 1;  /**< Single-bit error for ISRM_O_RAM. */
	uint64_t drp_hi_ram_sbe               : 1;  /**< Single-bit error for DRP_HI_RAM. */
	uint64_t drp_lo_ram_sbe               : 1;  /**< Single-bit error for DRP_LO_RAM. */
	uint64_t dwp_hi_ram_sbe               : 1;  /**< Single-bit error for DWP_HI_RAM. */
	uint64_t dwp_lo_ram_sbe               : 1;  /**< Single-bit error for DWP_LO_RAM. */
	uint64_t mwp_hi_ram_sbe               : 1;  /**< Single-bit error for MWP_HI_RAM. */
	uint64_t mwp_lo_ram_sbe               : 1;  /**< Single-bit error for MWP_LO_RAM. */
	uint64_t fillb_m_dat_ram_sbe          : 1;  /**< Single-bit error for FILLB_M_DAT_RAM. */
	uint64_t fillb_d_dat_ram_sbe          : 1;  /**< Single-bit error for FILLB_D_DAT_RAM. */
	uint64_t reserved_0_45                : 46;
#else
	uint64_t reserved_0_45                : 46;
	uint64_t fillb_d_dat_ram_sbe          : 1;
	uint64_t fillb_m_dat_ram_sbe          : 1;
	uint64_t mwp_lo_ram_sbe               : 1;
	uint64_t mwp_hi_ram_sbe               : 1;
	uint64_t dwp_lo_ram_sbe               : 1;
	uint64_t dwp_hi_ram_sbe               : 1;
	uint64_t drp_lo_ram_sbe               : 1;
	uint64_t drp_hi_ram_sbe               : 1;
	uint64_t isrm_o_ram_sbe               : 1;
	uint64_t isrm_h_ram_sbe               : 1;
	uint64_t isrm_t_ram_sbe               : 1;
	uint64_t isrd_h_ram_sbe               : 1;
	uint64_t isrd_t_ram_sbe               : 1;
	uint64_t isrm_ca_cm_ram_sbe           : 1;
	uint64_t isrm_ca_iinst_ram_sbe        : 1;
	uint64_t isrd_o_ram_sbe               : 1;
	uint64_t flshb_cache_hi_ram_sbe       : 1;
	uint64_t flshb_cache_lo_ram_sbe       : 1;
#endif
	} s;
	struct cvmx_pdm_ecc_sbe_sts0_s        cn78xx;
};
typedef union cvmx_pdm_ecc_sbe_sts0 cvmx_pdm_ecc_sbe_sts0_t;

/**
 * cvmx_pdm_ecc_sbe_sts_cmb0
 */
union cvmx_pdm_ecc_sbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_pdm_ecc_sbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pdm_sbe_cmb0                 : 1;  /**< Single-bit error for FLSHB_CACHE_LO_RAM. Throws PKO_INTSN_E::PDM_SBE_CMB0. */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t pdm_sbe_cmb0                 : 1;
#endif
	} s;
	struct cvmx_pdm_ecc_sbe_sts_cmb0_s    cn78xx;
};
typedef union cvmx_pdm_ecc_sbe_sts_cmb0 cvmx_pdm_ecc_sbe_sts_cmb0_t;

#endif
