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
 * cvmx-bch-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon bch.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_BCH_DEFS_H__
#define __CVMX_BCH_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_BCH_BIST_RESULT CVMX_BCH_BIST_RESULT_FUNC()
static inline uint64_t CVMX_BCH_BIST_RESULT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_BCH_BIST_RESULT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180071000080ull);
}
#else
#define CVMX_BCH_BIST_RESULT (CVMX_ADD_IO_SEG(0x0001180071000080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_BCH_CMD_BUF CVMX_BCH_CMD_BUF_FUNC()
static inline uint64_t CVMX_BCH_CMD_BUF_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_BCH_CMD_BUF not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180071000008ull);
}
#else
#define CVMX_BCH_CMD_BUF (CVMX_ADD_IO_SEG(0x0001180071000008ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_BCH_CMD_PTR CVMX_BCH_CMD_PTR_FUNC()
static inline uint64_t CVMX_BCH_CMD_PTR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_BCH_CMD_PTR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180071000020ull);
}
#else
#define CVMX_BCH_CMD_PTR (CVMX_ADD_IO_SEG(0x0001180071000020ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_BCH_CTL CVMX_BCH_CTL_FUNC()
static inline uint64_t CVMX_BCH_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_BCH_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180071000000ull);
}
#else
#define CVMX_BCH_CTL (CVMX_ADD_IO_SEG(0x0001180071000000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_BCH_ECO CVMX_BCH_ECO_FUNC()
static inline uint64_t CVMX_BCH_ECO_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_BCH_ECO not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180071000030ull);
}
#else
#define CVMX_BCH_ECO (CVMX_ADD_IO_SEG(0x0001180071000030ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_BCH_ERR_CFG CVMX_BCH_ERR_CFG_FUNC()
static inline uint64_t CVMX_BCH_ERR_CFG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_BCH_ERR_CFG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180071000010ull);
}
#else
#define CVMX_BCH_ERR_CFG (CVMX_ADD_IO_SEG(0x0001180071000010ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_BCH_GEN_INT CVMX_BCH_GEN_INT_FUNC()
static inline uint64_t CVMX_BCH_GEN_INT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX)))
		cvmx_warn("CVMX_BCH_GEN_INT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180071000088ull);
}
#else
#define CVMX_BCH_GEN_INT (CVMX_ADD_IO_SEG(0x0001180071000088ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_BCH_GEN_INT_EN CVMX_BCH_GEN_INT_EN_FUNC()
static inline uint64_t CVMX_BCH_GEN_INT_EN_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX)))
		cvmx_warn("CVMX_BCH_GEN_INT_EN not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180071000090ull);
}
#else
#define CVMX_BCH_GEN_INT_EN (CVMX_ADD_IO_SEG(0x0001180071000090ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_BCH_REG_ERROR CVMX_BCH_REG_ERROR_FUNC()
static inline uint64_t CVMX_BCH_REG_ERROR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_BCH_REG_ERROR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180071000088ull);
}
#else
#define CVMX_BCH_REG_ERROR (CVMX_ADD_IO_SEG(0x0001180071000088ull))
#endif

/**
 * cvmx_bch_bist_result
 *
 * This register provides access to internal BIST results. Each bit is the BIST result of an
 * individual memory (per bit, 0 = pass, 1 = fail).
 */
union cvmx_bch_bist_result {
	uint64_t u64;
	struct cvmx_bch_bist_result_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t ncb_oub                      : 1;  /**< BIST result of the NCB_OUB memories. */
	uint64_t ncb_inb                      : 1;  /**< BIST result of the NCB_INB memories. */
	uint64_t dat                          : 4;  /**< BIST result of the DAT memories. */
#else
	uint64_t dat                          : 4;
	uint64_t ncb_inb                      : 1;
	uint64_t ncb_oub                      : 1;
	uint64_t reserved_6_63                : 58;
#endif
	} s;
	struct cvmx_bch_bist_result_s         cn70xx;
	struct cvmx_bch_bist_result_s         cn70xxp1;
	struct cvmx_bch_bist_result_s         cn73xx;
	struct cvmx_bch_bist_result_s         cnf75xx;
};
typedef union cvmx_bch_bist_result cvmx_bch_bist_result_t;

/**
 * cvmx_bch_cmd_buf
 *
 * This register sets the command-buffer parameters.
 *
 */
union cvmx_bch_cmd_buf {
	uint64_t u64;
	struct cvmx_bch_cmd_buf_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_46_63               : 18;
	uint64_t size                         : 13; /**< Number of uint64s per command buffer segment. */
	uint64_t ptr                          : 33; /**< Initial command buffer pointer[39:7] (128B-aligned). */
#else
	uint64_t ptr                          : 33;
	uint64_t size                         : 13;
	uint64_t reserved_46_63               : 18;
#endif
	} s;
	struct cvmx_bch_cmd_buf_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_58_63               : 6;
	uint64_t dwb                          : 9;  /**< Number of DontWriteBacks. */
	uint64_t pool                         : 3;  /**< Free list used to free command buffer segments. */
	uint64_t size                         : 13; /**< Number of uint64s per command buffer segment. */
	uint64_t ptr                          : 33; /**< Initial command buffer pointer[39:7] (128B-aligned). */
#else
	uint64_t ptr                          : 33;
	uint64_t size                         : 13;
	uint64_t pool                         : 3;
	uint64_t dwb                          : 9;
	uint64_t reserved_58_63               : 6;
#endif
	} cn70xx;
	struct cvmx_bch_cmd_buf_cn70xx        cn70xxp1;
	struct cvmx_bch_cmd_buf_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t aura                         : 12; /**< Aura to use when freeing command-buffer segments. */
	uint64_t ldwb                         : 1;  /**< When reading commands that end on cache line boundaries, use load-and-don't write back commands. */
	uint64_t dfb                          : 1;  /**< Don't free buffers to the FPA. */
	uint64_t size                         : 13; /**< Number of uint64s per command buffer segment. */
	uint64_t reserved_0_32                : 33;
#else
	uint64_t reserved_0_32                : 33;
	uint64_t size                         : 13;
	uint64_t dfb                          : 1;
	uint64_t ldwb                         : 1;
	uint64_t aura                         : 12;
	uint64_t reserved_60_63               : 4;
#endif
	} cn73xx;
	struct cvmx_bch_cmd_buf_cn73xx        cnf75xx;
};
typedef union cvmx_bch_cmd_buf cvmx_bch_cmd_buf_t;

/**
 * cvmx_bch_cmd_ptr
 *
 * This register sets the command-buffer parameters.
 *
 */
union cvmx_bch_cmd_ptr {
	uint64_t u64;
	struct cvmx_bch_cmd_ptr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_42_63               : 22;
	uint64_t ptr                          : 35; /**< Initial command-buffer pointer bits <41:7> (128-byte aligned). Overwritten each time the
                                                         command-buffer segment is exhausted. */
	uint64_t reserved_0_6                 : 7;
#else
	uint64_t reserved_0_6                 : 7;
	uint64_t ptr                          : 35;
	uint64_t reserved_42_63               : 22;
#endif
	} s;
	struct cvmx_bch_cmd_ptr_s             cn73xx;
	struct cvmx_bch_cmd_ptr_s             cnf75xx;
};
typedef union cvmx_bch_cmd_ptr cvmx_bch_cmd_ptr_t;

/**
 * cvmx_bch_ctl
 */
union cvmx_bch_ctl {
	uint64_t u64;
	struct cvmx_bch_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_23_63               : 41;
	uint64_t free_ena                     : 1;  /**< Enable freeing of command buffers. */
	uint64_t early_term                   : 4;  /**< Threshold of zero delta iterations before declaring early termination.
                                                         0 will force all iterations to run.  For diagnostic use only. */
	uint64_t one_cmd                      : 1;  /**< Execute a single operation at a time.  For diagnostic use only. */
	uint64_t erase_disable                : 1;  /**< When ERASE_DISABLE=0, erased blocks bypass the BCH correction.   The 16B result word
                                                         contains an erased block indication.
                                                         A block is considered erased if the number of zeros found in the block (data+ECC) is
                                                         less than half the ECC level.   For instance, a 2 KB block using ECC32 is considered
                                                         erased if few than 16 zeroes are found in the 2048+60 bytes. */
	uint64_t reserved_6_15                : 10;
	uint64_t max_read                     : 4;  /**< Maximum number of outstanding data read commands. MAX_READ is a throttle to control IOB
                                                         usage. Values greater than 0x8 are illegal. */
	uint64_t store_le                     : 1;  /**< Force STORE0 byte write address to little endian. */
	uint64_t reset                        : 1;  /**< Reset oneshot pulse (lasts for 4 cycles). */
#else
	uint64_t reset                        : 1;
	uint64_t store_le                     : 1;
	uint64_t max_read                     : 4;
	uint64_t reserved_6_15                : 10;
	uint64_t erase_disable                : 1;
	uint64_t one_cmd                      : 1;
	uint64_t early_term                   : 4;
	uint64_t free_ena                     : 1;
	uint64_t reserved_23_63               : 41;
#endif
	} s;
	struct cvmx_bch_ctl_s                 cn70xx;
	struct cvmx_bch_ctl_s                 cn70xxp1;
	struct cvmx_bch_ctl_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_22_63               : 42;
	uint64_t early_term                   : 4;  /**< Threshold of zero delta iterations before declaring early termination.
                                                         0 will force all iterations to run.  For diagnostic use only. */
	uint64_t one_cmd                      : 1;  /**< Execute a single operation at a time.  For diagnostic use only. */
	uint64_t erase_disable                : 1;  /**< When ERASE_DISABLE=0, erased blocks bypass the BCH correction.   The 16B result word
                                                         contains an erased block indication.
                                                         A block is considered erased if the number of zeros found in the block (data+ECC) is
                                                         less than half the ECC level.   For instance, a 2 KB block using ECC32 is considered
                                                         erased if few than 16 zeroes are found in the 2048+60 bytes. */
	uint64_t reserved_6_15                : 10;
	uint64_t max_read                     : 4;  /**< Maximum number of outstanding data read commands. MAX_READ is a throttle to control IOB
                                                         usage. Values greater than 0x8 are illegal. */
	uint64_t store_le                     : 1;  /**< Force STORE0 byte write address to little endian. */
	uint64_t reset                        : 1;  /**< Reset oneshot pulse (lasts for 4 cycles). */
#else
	uint64_t reset                        : 1;
	uint64_t store_le                     : 1;
	uint64_t max_read                     : 4;
	uint64_t reserved_6_15                : 10;
	uint64_t erase_disable                : 1;
	uint64_t one_cmd                      : 1;
	uint64_t early_term                   : 4;
	uint64_t reserved_22_63               : 42;
#endif
	} cn73xx;
	struct cvmx_bch_ctl_cn73xx            cnf75xx;
};
typedef union cvmx_bch_ctl cvmx_bch_ctl_t;

/**
 * cvmx_bch_eco
 */
union cvmx_bch_eco {
	uint64_t u64;
	struct cvmx_bch_eco_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t eco_rw                       : 32; /**< Reserved for ECO usage. */
#else
	uint64_t eco_rw                       : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_bch_eco_s                 cn73xx;
	struct cvmx_bch_eco_s                 cnf75xx;
};
typedef union cvmx_bch_eco cvmx_bch_eco_t;

/**
 * cvmx_bch_err_cfg
 */
union cvmx_bch_err_cfg {
	uint64_t u64;
	struct cvmx_bch_err_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t dat_flip                     : 2;  /**< Testing feature. Flip syndrome bits <1:0> on writes to the DAT ram to test single-bit or
                                                         double-bit errors. */
	uint64_t reserved_1_15                : 15;
	uint64_t dat_cor_dis                  : 1;  /**< Disable ECC corrector on DAT RAM. */
#else
	uint64_t dat_cor_dis                  : 1;
	uint64_t reserved_1_15                : 15;
	uint64_t dat_flip                     : 2;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_bch_err_cfg_s             cn70xx;
	struct cvmx_bch_err_cfg_s             cn70xxp1;
	struct cvmx_bch_err_cfg_s             cn73xx;
	struct cvmx_bch_err_cfg_s             cnf75xx;
};
typedef union cvmx_bch_err_cfg cvmx_bch_err_cfg_t;

/**
 * cvmx_bch_gen_int
 */
union cvmx_bch_gen_int {
	uint64_t u64;
	struct cvmx_bch_gen_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t dat_dbe                      : 1;  /**< An ECC uncorrectable error has occurred in the DAT RAM. */
	uint64_t dat_sbe                      : 1;  /**< An ECC correctable error has occurred in the DAT RAM. */
	uint64_t doorbell                     : 1;  /**< Error bit indicating a doorbell count has overflowed. */
#else
	uint64_t doorbell                     : 1;
	uint64_t dat_sbe                      : 1;
	uint64_t dat_dbe                      : 1;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_bch_gen_int_s             cn70xx;
	struct cvmx_bch_gen_int_s             cn70xxp1;
};
typedef union cvmx_bch_gen_int cvmx_bch_gen_int_t;

/**
 * cvmx_bch_gen_int_en
 *
 * When a mask bit is set, the corresponding interrupt is enabled.
 *
 */
union cvmx_bch_gen_int_en {
	uint64_t u64;
	struct cvmx_bch_gen_int_en_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t dat_dbe                      : 1;  /**< Interrupt-enable bit mask corresponding to the error mask in BCH_GEN_INT. */
	uint64_t dat_sbe                      : 1;  /**< Interrupt-enable bit mask corresponding to the error mask in BCH_GEN_INT. */
	uint64_t doorbell                     : 1;  /**< Interrupt-enable bit mask corresponding to the error mask in BCH_GEN_INT. */
#else
	uint64_t doorbell                     : 1;
	uint64_t dat_sbe                      : 1;
	uint64_t dat_dbe                      : 1;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_bch_gen_int_en_s          cn70xx;
	struct cvmx_bch_gen_int_en_s          cn70xxp1;
};
typedef union cvmx_bch_gen_int_en cvmx_bch_gen_int_en_t;

/**
 * cvmx_bch_reg_error
 */
union cvmx_bch_reg_error {
	uint64_t u64;
	struct cvmx_bch_reg_error_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t dat_dbe                      : 1;  /**< An ECC uncorrectable error has occurred in the DAT RAM.
                                                         Throws BCH_INTSN_E::BCH_FIFO_DBE. */
	uint64_t dat_sbe                      : 1;  /**< An ECC correctable error has occurred in the DAT RAM.
                                                         Throws BCH_INTSN_E::BCH_FIFO_SBE. */
	uint64_t doorbell                     : 1;  /**< Error bit indicating a doorbell count has overflowed.
                                                         Throws BCH_INTSN_E::BCH_FIFO_DOORBELL. */
#else
	uint64_t doorbell                     : 1;
	uint64_t dat_sbe                      : 1;
	uint64_t dat_dbe                      : 1;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_bch_reg_error_s           cn73xx;
	struct cvmx_bch_reg_error_s           cnf75xx;
};
typedef union cvmx_bch_reg_error cvmx_bch_reg_error_t;

#endif
