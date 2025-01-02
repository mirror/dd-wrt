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
 * cvmx-lbk-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon lbk.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_LBK_DEFS_H__
#define __CVMX_LBK_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_LBK_BIST_RESULT CVMX_LBK_BIST_RESULT_FUNC()
static inline uint64_t CVMX_LBK_BIST_RESULT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_LBK_BIST_RESULT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180012000020ull);
}
#else
#define CVMX_LBK_BIST_RESULT (CVMX_ADD_IO_SEG(0x0001180012000020ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_LBK_CHX_PKIND(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_LBK_CHX_PKIND(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180012000200ull) + ((offset) & 63) * 8;
}
#else
#define CVMX_LBK_CHX_PKIND(offset) (CVMX_ADD_IO_SEG(0x0001180012000200ull) + ((offset) & 63) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_LBK_CLK_GATE_CTL CVMX_LBK_CLK_GATE_CTL_FUNC()
static inline uint64_t CVMX_LBK_CLK_GATE_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_LBK_CLK_GATE_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180012000008ull);
}
#else
#define CVMX_LBK_CLK_GATE_CTL (CVMX_ADD_IO_SEG(0x0001180012000008ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_LBK_DAT_ERR_INFO CVMX_LBK_DAT_ERR_INFO_FUNC()
static inline uint64_t CVMX_LBK_DAT_ERR_INFO_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_LBK_DAT_ERR_INFO not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180012000050ull);
}
#else
#define CVMX_LBK_DAT_ERR_INFO (CVMX_ADD_IO_SEG(0x0001180012000050ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_LBK_ECC_CFG CVMX_LBK_ECC_CFG_FUNC()
static inline uint64_t CVMX_LBK_ECC_CFG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_LBK_ECC_CFG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180012000060ull);
}
#else
#define CVMX_LBK_ECC_CFG (CVMX_ADD_IO_SEG(0x0001180012000060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_LBK_INT CVMX_LBK_INT_FUNC()
static inline uint64_t CVMX_LBK_INT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_LBK_INT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180012000040ull);
}
#else
#define CVMX_LBK_INT (CVMX_ADD_IO_SEG(0x0001180012000040ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_LBK_SFT_RST CVMX_LBK_SFT_RST_FUNC()
static inline uint64_t CVMX_LBK_SFT_RST_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_LBK_SFT_RST not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180012000000ull);
}
#else
#define CVMX_LBK_SFT_RST (CVMX_ADD_IO_SEG(0x0001180012000000ull))
#endif

/**
 * cvmx_lbk_bist_result
 *
 * This register provides access to the internal BIST results. Each bit is the BIST result of an
 * individual memory (per bit, 0 = pass and 1 = fail).
 */
union cvmx_lbk_bist_result {
	uint64_t u64;
	struct cvmx_lbk_bist_result_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t dat                          : 1;  /**< BIST result of the Data FIFO RAM. */
#else
	uint64_t dat                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_lbk_bist_result_s         cn73xx;
	struct cvmx_lbk_bist_result_s         cn78xx;
	struct cvmx_lbk_bist_result_s         cn78xxp1;
	struct cvmx_lbk_bist_result_s         cnf75xx;
};
typedef union cvmx_lbk_bist_result cvmx_lbk_bist_result_t;

/**
 * cvmx_lbk_ch#_pkind
 */
union cvmx_lbk_chx_pkind {
	uint64_t u64;
	struct cvmx_lbk_chx_pkind_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t pkind                        : 6;  /**< Loopback pkind for the respective loopback channel. */
#else
	uint64_t pkind                        : 6;
	uint64_t reserved_6_63                : 58;
#endif
	} s;
	struct cvmx_lbk_chx_pkind_s           cn73xx;
	struct cvmx_lbk_chx_pkind_s           cn78xx;
	struct cvmx_lbk_chx_pkind_s           cn78xxp1;
	struct cvmx_lbk_chx_pkind_s           cnf75xx;
};
typedef union cvmx_lbk_chx_pkind cvmx_lbk_chx_pkind_t;

/**
 * cvmx_lbk_clk_gate_ctl
 *
 * This register is for diagnostic use only.
 *
 */
union cvmx_lbk_clk_gate_ctl {
	uint64_t u64;
	struct cvmx_lbk_clk_gate_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t dis                          : 1;  /**< Clock gate disable. When set, forces gated clock to always on. */
#else
	uint64_t dis                          : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_lbk_clk_gate_ctl_s        cn73xx;
	struct cvmx_lbk_clk_gate_ctl_s        cn78xx;
	struct cvmx_lbk_clk_gate_ctl_s        cn78xxp1;
	struct cvmx_lbk_clk_gate_ctl_s        cnf75xx;
};
typedef union cvmx_lbk_clk_gate_ctl cvmx_lbk_clk_gate_ctl_t;

/**
 * cvmx_lbk_dat_err_info
 */
union cvmx_lbk_dat_err_info {
	uint64_t u64;
	struct cvmx_lbk_dat_err_info_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_58_63               : 6;
	uint64_t dbe_ecc_out                  : 9;  /**< ECC_OUT captured from memory. Data is captured and retained as long as LBK_INT[DAT_DBE] is
                                                         asserted. */
	uint64_t dbe_synd                     : 9;  /**< Syndrome captured from memory. Data is captured and retained as long as LBK_INT[DAT_DBE]
                                                         is asserted. */
	uint64_t dbe_addr                     : 8;  /**< Address causing error in memory. Data is captured and retained as long as LBK_INT[DAT_DBE]
                                                         is asserted. */
	uint64_t reserved_26_31               : 6;
	uint64_t sbe_ecc_out                  : 9;  /**< ECC_OUT captured from memory. Data is captured and retained as long as LBK_INT[DAT_SBE] is
                                                         asserted. */
	uint64_t sbe_synd                     : 9;  /**< Syndrome captured from memory. Data is captured and retained as long as LBK_INT[DAT_SBE]
                                                         is asserted. */
	uint64_t sbe_addr                     : 8;  /**< Address causing error in memory. Data is captured and retained as long as LBK_INT[DAT_SBE]
                                                         is asserted. */
#else
	uint64_t sbe_addr                     : 8;
	uint64_t sbe_synd                     : 9;
	uint64_t sbe_ecc_out                  : 9;
	uint64_t reserved_26_31               : 6;
	uint64_t dbe_addr                     : 8;
	uint64_t dbe_synd                     : 9;
	uint64_t dbe_ecc_out                  : 9;
	uint64_t reserved_58_63               : 6;
#endif
	} s;
	struct cvmx_lbk_dat_err_info_s        cn73xx;
	struct cvmx_lbk_dat_err_info_s        cn78xx;
	struct cvmx_lbk_dat_err_info_s        cn78xxp1;
	struct cvmx_lbk_dat_err_info_s        cnf75xx;
};
typedef union cvmx_lbk_dat_err_info cvmx_lbk_dat_err_info_t;

/**
 * cvmx_lbk_ecc_cfg
 */
union cvmx_lbk_ecc_cfg {
	uint64_t u64;
	struct cvmx_lbk_ecc_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t dat_flip                     : 2;  /**< Testing feature. Flip syndrome bits <1:0> on writes to the Data FIFO RAM to test single-
                                                         bit or double-bit errors. */
	uint64_t dat_cdis                     : 1;  /**< Disable ECC corrector on Data FIFO RAM outputs. */
#else
	uint64_t dat_cdis                     : 1;
	uint64_t dat_flip                     : 2;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_lbk_ecc_cfg_s             cn73xx;
	struct cvmx_lbk_ecc_cfg_s             cn78xx;
	struct cvmx_lbk_ecc_cfg_s             cn78xxp1;
	struct cvmx_lbk_ecc_cfg_s             cnf75xx;
};
typedef union cvmx_lbk_ecc_cfg cvmx_lbk_ecc_cfg_t;

/**
 * cvmx_lbk_int
 */
union cvmx_lbk_int {
	uint64_t u64;
	struct cvmx_lbk_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t chan_oflow                   : 1;  /**< Internal assertion, packet channel credit FIFO had an overflow. Throws
                                                         LBK_INTSN_E::LBK_INT_CHAN_OFLOW. */
	uint64_t chan_uflow                   : 1;  /**< Internal assertion, packet channel credit FIFO had an underflow. Throws
                                                         LBK_INTSN_E::LBK_INT_CHAN_UFLOW. */
	uint64_t dat_oflow                    : 1;  /**< Internal assertion, packet Data FIFO had an overflow. Throws
                                                         LBK_INTSN_E::LBK_INT_DAT_OFLOW. */
	uint64_t dat_uflow                    : 1;  /**< Internal assertion, packet Data FIFO had an underflow. Throws
                                                         LBK_INTSN_E::LBK_INT_DAT_UFLOW. */
	uint64_t dat_dbe                      : 1;  /**< Data RAM had a ECC double bit error. Throws LBK_INTSN_E::LBK_INT_DAT_DBE. */
	uint64_t dat_sbe                      : 1;  /**< Data RAM had a ECC single bit error. Throws LBK_INTSN_E::LBK_INT_DAT_SBE. */
#else
	uint64_t dat_sbe                      : 1;
	uint64_t dat_dbe                      : 1;
	uint64_t dat_uflow                    : 1;
	uint64_t dat_oflow                    : 1;
	uint64_t chan_uflow                   : 1;
	uint64_t chan_oflow                   : 1;
	uint64_t reserved_6_63                : 58;
#endif
	} s;
	struct cvmx_lbk_int_s                 cn73xx;
	struct cvmx_lbk_int_s                 cn78xx;
	struct cvmx_lbk_int_s                 cn78xxp1;
	struct cvmx_lbk_int_s                 cnf75xx;
};
typedef union cvmx_lbk_int cvmx_lbk_int_t;

/**
 * cvmx_lbk_sft_rst
 */
union cvmx_lbk_sft_rst {
	uint64_t u64;
	struct cvmx_lbk_sft_rst_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t reset                        : 1;  /**< Reset. When set, causes a reset of LBK, excluding RSL. */
#else
	uint64_t reset                        : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_lbk_sft_rst_s             cn73xx;
	struct cvmx_lbk_sft_rst_s             cn78xx;
	struct cvmx_lbk_sft_rst_s             cn78xxp1;
	struct cvmx_lbk_sft_rst_s             cnf75xx;
};
typedef union cvmx_lbk_sft_rst cvmx_lbk_sft_rst_t;

#endif
