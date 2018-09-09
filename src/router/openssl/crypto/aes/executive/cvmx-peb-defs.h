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
 * cvmx-peb-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon peb.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_PEB_DEFS_H__
#define __CVMX_PEB_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PEB_ECC_CTL0 CVMX_PEB_ECC_CTL0_FUNC()
static inline uint64_t CVMX_PEB_ECC_CTL0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PEB_ECC_CTL0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400009FFFD0ull);
}
#else
#define CVMX_PEB_ECC_CTL0 (CVMX_ADD_IO_SEG(0x00015400009FFFD0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PEB_ECC_DBE_STS0 CVMX_PEB_ECC_DBE_STS0_FUNC()
static inline uint64_t CVMX_PEB_ECC_DBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PEB_ECC_DBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400009FFFF0ull);
}
#else
#define CVMX_PEB_ECC_DBE_STS0 (CVMX_ADD_IO_SEG(0x00015400009FFFF0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PEB_ECC_DBE_STS_CMB0 CVMX_PEB_ECC_DBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PEB_ECC_DBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PEB_ECC_DBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400009FFFD8ull);
}
#else
#define CVMX_PEB_ECC_DBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x00015400009FFFD8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PEB_ECC_SBE_STS0 CVMX_PEB_ECC_SBE_STS0_FUNC()
static inline uint64_t CVMX_PEB_ECC_SBE_STS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PEB_ECC_SBE_STS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400009FFFF8ull);
}
#else
#define CVMX_PEB_ECC_SBE_STS0 (CVMX_ADD_IO_SEG(0x00015400009FFFF8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_PEB_ECC_SBE_STS_CMB0 CVMX_PEB_ECC_SBE_STS_CMB0_FUNC()
static inline uint64_t CVMX_PEB_ECC_SBE_STS_CMB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_PEB_ECC_SBE_STS_CMB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00015400009FFFE8ull);
}
#else
#define CVMX_PEB_ECC_SBE_STS_CMB0 (CVMX_ADD_IO_SEG(0x00015400009FFFE8ull))
#endif

/**
 * cvmx_peb_ecc_ctl0
 */
union cvmx_peb_ecc_ctl0 {
	uint64_t u64;
	struct cvmx_peb_ecc_ctl0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t iobp1_uid_fifo_ram_flip      : 2;  /**< IOBP1_UID_FIFO_RAM flip syndrome bits on write. */
	uint64_t iobp1_uid_fifo_ram_cdis      : 1;  /**< IOBP1_UID_FIFO_RAM ECC correction disable. */
	uint64_t iobp1_fifo_ram_flip          : 2;  /**< IOBP1_FIFO_RAM flip syndrome bits on write. */
	uint64_t iobp1_fifo_ram_cdis          : 1;  /**< IOBP1_FIFO_RAM ECC correction disable. */
	uint64_t iobp0_fifo_ram_flip          : 2;  /**< IOBP0_FIFO_RAM flip syndrome bits on write. */
	uint64_t iobp0_fifo_ram_cdis          : 1;  /**< IOBP0_FIFO_RAM ECC correction disable. */
	uint64_t pdm_resp_buf_i_ram_flip      : 2;  /**< PDM_RESP_BUF_I_RAM flip syndrome bits on write. */
	uint64_t pdm_resp_buf_i_ram_cdis      : 1;  /**< PDM_RESP_BUF_I_RAM ECC correction disable. */
	uint64_t pdm_pse_buf_ram_flip         : 2;  /**< PDM_PSE_BUF_RAM flip syndrome bits on write. */
	uint64_t pdm_pse_buf_ram_cdis         : 1;  /**< PDM_PSE_BUF_RAM ECC correction disable. */
	uint64_t peb_st_inf_ram_flip          : 2;  /**< PEB_ST_INF_RAM flip syndrome bits on write. */
	uint64_t peb_st_inf_ram_cdis          : 1;  /**< PEB_ST_INF_RAM ECC correction disable. */
	uint64_t jmp_bank3_ram_flip           : 2;  /**< JMP_BANK3_RAM flip syndrome bits on write. */
	uint64_t jmp_bank3_ram_cdis           : 1;  /**< JMP_BANK3_RAM ECC correction disable. */
	uint64_t jmp_bank2_ram_flip           : 2;  /**< JMP_BANK2_RAM flip syndrome bits on write. */
	uint64_t jmp_bank2_ram_cdis           : 1;  /**< JMP_BANK2_RAM ECC correction disable. */
	uint64_t jmp_bank1_ram_flip           : 2;  /**< JMP_BANK1_RAM flip syndrome bits on write. */
	uint64_t jmp_bank1_ram_cdis           : 1;  /**< JMP_BANK1_RAM ECC correction disable. */
	uint64_t jmp_bank0_ram_flip           : 2;  /**< JMP_BANK0_RAM flip syndrome bits on write. */
	uint64_t jmp_bank0_ram_cdis           : 1;  /**< JMP_BANK0_RAM ECC correction disable. */
	uint64_t pd_bank3_ram_flip            : 2;  /**< PD_BANK3_RAM flip syndrome bits on write. */
	uint64_t pd_bank3_ram_cdis            : 1;  /**< PD_BANK3_RAM ECC correction disable. */
	uint64_t pd_bank2_ram_flip            : 2;  /**< PD_BANK2_RAM flip syndrome bits on write. */
	uint64_t pd_bank2_ram_cdis            : 1;  /**< PD_BANK2_RAM ECC correction disable. */
	uint64_t pd_bank1_ram_flip            : 2;  /**< PD_BANK1_RAM flip syndrome bits on write. */
	uint64_t pd_bank1_ram_cdis            : 1;  /**< PD_BANK1_RAM ECC correction disable. */
	uint64_t pd_bank0_ram_flip            : 2;  /**< PD_BANK0_RAM flip syndrome bits on write. */
	uint64_t pd_bank0_ram_cdis            : 1;  /**< PD_BANK0_RAM ECC correction disable. */
	uint64_t pd_var_bank_ram_flip         : 2;  /**< PD_VAR_BANK_RAM flip syndrome bits on write. */
	uint64_t pd_var_bank_ram_cdis         : 1;  /**< PD_VAR_BANK_RAM ECC correction disable. */
	uint64_t tx_fifo_crc_ram_flip         : 2;  /**< TX_FIFO_CRC_RAM flip syndrome bits on write. */
	uint64_t tx_fifo_crc_ram_cdis         : 1;  /**< TX_FIFO_CRC_RAM ECC correction disable. */
	uint64_t tx_fifo_hdr_ram_flip         : 2;  /**< TX_FIFO_HDR_RAM flip syndrome bits on write. */
	uint64_t tx_fifo_hdr_ram_cdis         : 1;  /**< TX_FIFO_HDR_RAM ECC correction disable. */
	uint64_t tx_fifo_pkt_ram_flip         : 2;  /**< TX_FIFO_PKT_RAM flip syndrome bits on write. */
	uint64_t tx_fifo_pkt_ram_cdis         : 1;  /**< TX_FIFO_PKT_RAM ECC correction disable. */
	uint64_t add_work_fifo_flip           : 2;  /**< ADD_WORK_FIFO flip syndrome bits on write. */
	uint64_t add_work_fifo_cdis           : 1;  /**< ADD_WORK_FIFO ECC correction disable. */
	uint64_t send_mem_fifo_flip           : 2;  /**< SEND_MEM_FIFO flip syndrome bits on write. */
	uint64_t send_mem_fifo_cdis           : 1;  /**< SEND_MEM_FIFO ECC correction disable. */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t send_mem_fifo_cdis           : 1;
	uint64_t send_mem_fifo_flip           : 2;
	uint64_t add_work_fifo_cdis           : 1;
	uint64_t add_work_fifo_flip           : 2;
	uint64_t tx_fifo_pkt_ram_cdis         : 1;
	uint64_t tx_fifo_pkt_ram_flip         : 2;
	uint64_t tx_fifo_hdr_ram_cdis         : 1;
	uint64_t tx_fifo_hdr_ram_flip         : 2;
	uint64_t tx_fifo_crc_ram_cdis         : 1;
	uint64_t tx_fifo_crc_ram_flip         : 2;
	uint64_t pd_var_bank_ram_cdis         : 1;
	uint64_t pd_var_bank_ram_flip         : 2;
	uint64_t pd_bank0_ram_cdis            : 1;
	uint64_t pd_bank0_ram_flip            : 2;
	uint64_t pd_bank1_ram_cdis            : 1;
	uint64_t pd_bank1_ram_flip            : 2;
	uint64_t pd_bank2_ram_cdis            : 1;
	uint64_t pd_bank2_ram_flip            : 2;
	uint64_t pd_bank3_ram_cdis            : 1;
	uint64_t pd_bank3_ram_flip            : 2;
	uint64_t jmp_bank0_ram_cdis           : 1;
	uint64_t jmp_bank0_ram_flip           : 2;
	uint64_t jmp_bank1_ram_cdis           : 1;
	uint64_t jmp_bank1_ram_flip           : 2;
	uint64_t jmp_bank2_ram_cdis           : 1;
	uint64_t jmp_bank2_ram_flip           : 2;
	uint64_t jmp_bank3_ram_cdis           : 1;
	uint64_t jmp_bank3_ram_flip           : 2;
	uint64_t peb_st_inf_ram_cdis          : 1;
	uint64_t peb_st_inf_ram_flip          : 2;
	uint64_t pdm_pse_buf_ram_cdis         : 1;
	uint64_t pdm_pse_buf_ram_flip         : 2;
	uint64_t pdm_resp_buf_i_ram_cdis      : 1;
	uint64_t pdm_resp_buf_i_ram_flip      : 2;
	uint64_t iobp0_fifo_ram_cdis          : 1;
	uint64_t iobp0_fifo_ram_flip          : 2;
	uint64_t iobp1_fifo_ram_cdis          : 1;
	uint64_t iobp1_fifo_ram_flip          : 2;
	uint64_t iobp1_uid_fifo_ram_cdis      : 1;
	uint64_t iobp1_uid_fifo_ram_flip      : 2;
#endif
	} s;
	struct cvmx_peb_ecc_ctl0_s            cn78xx;
};
typedef union cvmx_peb_ecc_ctl0 cvmx_peb_ecc_ctl0_t;

/**
 * cvmx_peb_ecc_dbe_sts0
 */
union cvmx_peb_ecc_dbe_sts0 {
	uint64_t u64;
	struct cvmx_peb_ecc_dbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t iobp1_uid_fifo_ram_dbe       : 1;  /**< Double-bit error for IOBP1_UID_FIFO_RAM. */
	uint64_t iobp1_fifo_ram_dbe           : 1;  /**< Double-bit error for IOBP1_FIFO_RAM. */
	uint64_t iobp0_fifo_ram_dbe           : 1;  /**< Double-bit error for IOBP0_FIFO_RAM. */
	uint64_t pdm_resp_buf_i_ram_dbe       : 1;  /**< Double-bit error for PDM_RESP_BUF_I_RAM. */
	uint64_t pdm_pse_buf_ram_dbe          : 1;  /**< Double-bit error for PDM_PSE_BUF_RAM. */
	uint64_t peb_st_inf_ram_dbe           : 1;  /**< Double-bit error for PEB_ST_INF_RAM. */
	uint64_t jmp_bank3_ram_dbe            : 1;  /**< Double-bit error for JMP_BANK3_RAM. */
	uint64_t jmp_bank2_ram_dbe            : 1;  /**< Double-bit error for JMP_BANK2_RAM. */
	uint64_t jmp_bank1_ram_dbe            : 1;  /**< Double-bit error for JMP_BANK1_RAM. */
	uint64_t jmp_bank0_ram_dbe            : 1;  /**< Double-bit error for JMP_BANK0_RAM. */
	uint64_t pd_bank3_ram_dbe             : 1;  /**< Double-bit error for PD_BANK3_RAM. */
	uint64_t pd_bank2_ram_dbe             : 1;  /**< Double-bit error for PD_BANK2_RAM. */
	uint64_t pd_bank1_ram_dbe             : 1;  /**< Double-bit error for PD_BANK1_RAM. */
	uint64_t pd_bank0_ram_dbe             : 1;  /**< Double-bit error for PD_BANK0_RAM. */
	uint64_t pd_var_bank_ram_dbe          : 1;  /**< Double-bit error for PD_VAR_BANK_RAM. */
	uint64_t tx_fifo_crc_ram_dbe          : 1;  /**< Double-bit error for TX_FIFO_CRC_RAM. */
	uint64_t tx_fifo_hdr_ram_dbe          : 1;  /**< Double-bit error for TX_FIFO_HDR_RAM. */
	uint64_t tx_fifo_pkt_ram_dbe          : 1;  /**< Double-bit error for TX_FIFO_PKT_RAM. */
	uint64_t add_work_fifo_dbe            : 1;  /**< Double-bit error for ADD_WORK_FIFO. */
	uint64_t send_mem_fifo_dbe            : 1;  /**< Double-bit error for SEND_MEM_FIFO. */
	uint64_t reserved_0_43                : 44;
#else
	uint64_t reserved_0_43                : 44;
	uint64_t send_mem_fifo_dbe            : 1;
	uint64_t add_work_fifo_dbe            : 1;
	uint64_t tx_fifo_pkt_ram_dbe          : 1;
	uint64_t tx_fifo_hdr_ram_dbe          : 1;
	uint64_t tx_fifo_crc_ram_dbe          : 1;
	uint64_t pd_var_bank_ram_dbe          : 1;
	uint64_t pd_bank0_ram_dbe             : 1;
	uint64_t pd_bank1_ram_dbe             : 1;
	uint64_t pd_bank2_ram_dbe             : 1;
	uint64_t pd_bank3_ram_dbe             : 1;
	uint64_t jmp_bank0_ram_dbe            : 1;
	uint64_t jmp_bank1_ram_dbe            : 1;
	uint64_t jmp_bank2_ram_dbe            : 1;
	uint64_t jmp_bank3_ram_dbe            : 1;
	uint64_t peb_st_inf_ram_dbe           : 1;
	uint64_t pdm_pse_buf_ram_dbe          : 1;
	uint64_t pdm_resp_buf_i_ram_dbe       : 1;
	uint64_t iobp0_fifo_ram_dbe           : 1;
	uint64_t iobp1_fifo_ram_dbe           : 1;
	uint64_t iobp1_uid_fifo_ram_dbe       : 1;
#endif
	} s;
	struct cvmx_peb_ecc_dbe_sts0_s        cn78xx;
};
typedef union cvmx_peb_ecc_dbe_sts0 cvmx_peb_ecc_dbe_sts0_t;

/**
 * cvmx_peb_ecc_dbe_sts_cmb0
 */
union cvmx_peb_ecc_dbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_peb_ecc_dbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t peb_dbe_cmb0                 : 1;  /**< Double-bit error for IOBP1_UID_FIFO_RAM. Throws PKO_INTSN_E::PEB_DBE_CMB0. */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t peb_dbe_cmb0                 : 1;
#endif
	} s;
	struct cvmx_peb_ecc_dbe_sts_cmb0_s    cn78xx;
};
typedef union cvmx_peb_ecc_dbe_sts_cmb0 cvmx_peb_ecc_dbe_sts_cmb0_t;

/**
 * cvmx_peb_ecc_sbe_sts0
 */
union cvmx_peb_ecc_sbe_sts0 {
	uint64_t u64;
	struct cvmx_peb_ecc_sbe_sts0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t iobp1_uid_fifo_ram_sbe       : 1;  /**< Single-bit error for IOBP1_UID_FIFO_RAM. */
	uint64_t iobp1_fifo_ram_sbe           : 1;  /**< Single-bit error for IOBP1_FIFO_RAM. */
	uint64_t iobp0_fifo_ram_sbe           : 1;  /**< Single-bit error for IOBP0_FIFO_RAM. */
	uint64_t pdm_resp_buf_i_ram_sbe       : 1;  /**< Single-bit error for PDM_RESP_BUF_I_RAM. */
	uint64_t pdm_pse_buf_ram_sbe          : 1;  /**< Single-bit error for PDM_PSE_BUF_RAM. */
	uint64_t peb_st_inf_ram_sbe           : 1;  /**< Single-bit error for PEB_ST_INF_RAM. */
	uint64_t jmp_bank3_ram_sbe            : 1;  /**< Single-bit error for JMP_BANK3_RAM. */
	uint64_t jmp_bank2_ram_sbe            : 1;  /**< Single-bit error for JMP_BANK2_RAM. */
	uint64_t jmp_bank1_ram_sbe            : 1;  /**< Single-bit error for JMP_BANK1_RAM. */
	uint64_t jmp_bank0_ram_sbe            : 1;  /**< Single-bit error for JMP_BANK0_RAM. */
	uint64_t pd_bank3_ram_sbe             : 1;  /**< Single-bit error for PD_BANK3_RAM. */
	uint64_t pd_bank2_ram_sbe             : 1;  /**< Single-bit error for PD_BANK2_RAM. */
	uint64_t pd_bank1_ram_sbe             : 1;  /**< Single-bit error for PD_BANK1_RAM. */
	uint64_t pd_bank0_ram_sbe             : 1;  /**< Single-bit error for PD_BANK0_RAM. */
	uint64_t pd_var_bank_ram_sbe          : 1;  /**< Single-bit error for PD_VAR_BANK_RAM. */
	uint64_t tx_fifo_crc_ram_sbe          : 1;  /**< Single-bit error for TX_FIFO_CRC_RAM. */
	uint64_t tx_fifo_hdr_ram_sbe          : 1;  /**< Single-bit error for TX_FIFO_HDR_RAM. */
	uint64_t tx_fifo_pkt_ram_sbe          : 1;  /**< Single-bit error for TX_FIFO_PKT_RAM. */
	uint64_t add_work_fifo_sbe            : 1;  /**< Single-bit error for ADD_WORK_FIFO. */
	uint64_t send_mem_fifo_sbe            : 1;  /**< Single-bit error for SEND_MEM_FIFO. */
	uint64_t reserved_0_43                : 44;
#else
	uint64_t reserved_0_43                : 44;
	uint64_t send_mem_fifo_sbe            : 1;
	uint64_t add_work_fifo_sbe            : 1;
	uint64_t tx_fifo_pkt_ram_sbe          : 1;
	uint64_t tx_fifo_hdr_ram_sbe          : 1;
	uint64_t tx_fifo_crc_ram_sbe          : 1;
	uint64_t pd_var_bank_ram_sbe          : 1;
	uint64_t pd_bank0_ram_sbe             : 1;
	uint64_t pd_bank1_ram_sbe             : 1;
	uint64_t pd_bank2_ram_sbe             : 1;
	uint64_t pd_bank3_ram_sbe             : 1;
	uint64_t jmp_bank0_ram_sbe            : 1;
	uint64_t jmp_bank1_ram_sbe            : 1;
	uint64_t jmp_bank2_ram_sbe            : 1;
	uint64_t jmp_bank3_ram_sbe            : 1;
	uint64_t peb_st_inf_ram_sbe           : 1;
	uint64_t pdm_pse_buf_ram_sbe          : 1;
	uint64_t pdm_resp_buf_i_ram_sbe       : 1;
	uint64_t iobp0_fifo_ram_sbe           : 1;
	uint64_t iobp1_fifo_ram_sbe           : 1;
	uint64_t iobp1_uid_fifo_ram_sbe       : 1;
#endif
	} s;
	struct cvmx_peb_ecc_sbe_sts0_s        cn78xx;
};
typedef union cvmx_peb_ecc_sbe_sts0 cvmx_peb_ecc_sbe_sts0_t;

/**
 * cvmx_peb_ecc_sbe_sts_cmb0
 */
union cvmx_peb_ecc_sbe_sts_cmb0 {
	uint64_t u64;
	struct cvmx_peb_ecc_sbe_sts_cmb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t peb_sbe_cmb0                 : 1;  /**< Single-bit error for IOBP1_UID_FIFO_RAM. Throws PKO_INTSN_E::PEB_SBE_CMB0. */
	uint64_t reserved_0_62                : 63;
#else
	uint64_t reserved_0_62                : 63;
	uint64_t peb_sbe_cmb0                 : 1;
#endif
	} s;
	struct cvmx_peb_ecc_sbe_sts_cmb0_s    cn78xx;
};
typedef union cvmx_peb_ecc_sbe_sts_cmb0 cvmx_peb_ecc_sbe_sts_cmb0_t;

#endif
