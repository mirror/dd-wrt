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
 * cvmx-lapx-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon lapx.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_LAPX_DEFS_H__
#define __CVMX_LAPX_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_LAPX_BIST_RESULT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_LAPX_BIST_RESULT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118000C010000ull) + ((offset) & 1) * 0x1000000ull;
}
#else
#define CVMX_LAPX_BIST_RESULT(offset) (CVMX_ADD_IO_SEG(0x000118000C010000ull) + ((offset) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_LAPX_CFG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_LAPX_CFG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118000C010040ull) + ((offset) & 1) * 0x1000000ull;
}
#else
#define CVMX_LAPX_CFG(offset) (CVMX_ADD_IO_SEG(0x000118000C010040ull) + ((offset) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_LAPX_EDAT_ERR_ST(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_LAPX_EDAT_ERR_ST(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118000C010220ull) + ((offset) & 1) * 0x1000000ull;
}
#else
#define CVMX_LAPX_EDAT_ERR_ST(offset) (CVMX_ADD_IO_SEG(0x000118000C010220ull) + ((offset) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_LAPX_EMSK_ERR_ST(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_LAPX_EMSK_ERR_ST(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118000C010218ull) + ((offset) & 1) * 0x1000000ull;
}
#else
#define CVMX_LAPX_EMSK_ERR_ST(offset) (CVMX_ADD_IO_SEG(0x000118000C010218ull) + ((offset) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_LAPX_ERR_CFG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_LAPX_ERR_CFG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118000C010050ull) + ((offset) & 1) * 0x1000000ull;
}
#else
#define CVMX_LAPX_ERR_CFG(offset) (CVMX_ADD_IO_SEG(0x000118000C010050ull) + ((offset) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_LAPX_EXPX_DATA(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 15)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 15)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_LAPX_EXPX_DATA(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x000118000C020000ull) + (((offset) & 15) + ((block_id) & 1) * 0x200000ull) * 8;
}
#else
#define CVMX_LAPX_EXPX_DATA(offset, block_id) (CVMX_ADD_IO_SEG(0x000118000C020000ull) + (((offset) & 15) + ((block_id) & 1) * 0x200000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_LAPX_EXPX_VALID(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 15)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 15)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_LAPX_EXPX_VALID(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x000118000C030000ull) + (((offset) & 15) + ((block_id) & 1) * 0x200000ull) * 8;
}
#else
#define CVMX_LAPX_EXPX_VALID(offset, block_id) (CVMX_ADD_IO_SEG(0x000118000C030000ull) + (((offset) & 15) + ((block_id) & 1) * 0x200000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_LAPX_FREE_STATE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_LAPX_FREE_STATE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118000C010100ull) + ((offset) & 1) * 0x1000000ull;
}
#else
#define CVMX_LAPX_FREE_STATE(offset) (CVMX_ADD_IO_SEG(0x000118000C010100ull) + ((offset) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_LAPX_GEN_INT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_LAPX_GEN_INT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118000C010010ull) + ((offset) & 1) * 0x1000000ull;
}
#else
#define CVMX_LAPX_GEN_INT(offset) (CVMX_ADD_IO_SEG(0x000118000C010010ull) + ((offset) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_LAPX_LABX_STATE(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 255)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 255)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_LAPX_LABX_STATE(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x000118000C060000ull) + (((offset) & 255) + ((block_id) & 1) * 0x200000ull) * 8;
}
#else
#define CVMX_LAPX_LABX_STATE(offset, block_id) (CVMX_ADD_IO_SEG(0x000118000C060000ull) + (((offset) & 255) + ((block_id) & 1) * 0x200000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_LAPX_LAB_DATAX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1535)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 1535)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_LAPX_LAB_DATAX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x000118000C080000ull) + (((offset) & 2047) + ((block_id) & 1) * 0x200000ull) * 8;
}
#else
#define CVMX_LAPX_LAB_DATAX(offset, block_id) (CVMX_ADD_IO_SEG(0x000118000C080000ull) + (((offset) & 2047) + ((block_id) & 1) * 0x200000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_LAPX_LAB_ERR_ST(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_LAPX_LAB_ERR_ST(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118000C010200ull) + ((offset) & 1) * 0x1000000ull;
}
#else
#define CVMX_LAPX_LAB_ERR_ST(offset) (CVMX_ADD_IO_SEG(0x000118000C010200ull) + ((offset) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_LAPX_NXT_ERR_ST(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_LAPX_NXT_ERR_ST(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118000C010210ull) + ((offset) & 1) * 0x1000000ull;
}
#else
#define CVMX_LAPX_NXT_ERR_ST(offset) (CVMX_ADD_IO_SEG(0x000118000C010210ull) + ((offset) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_LAPX_QUEX_CFG(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 2)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 2)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_LAPX_QUEX_CFG(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x000118000C040000ull) + (((offset) & 3) + ((block_id) & 1) * 0x200000ull) * 8;
}
#else
#define CVMX_LAPX_QUEX_CFG(offset, block_id) (CVMX_ADD_IO_SEG(0x000118000C040000ull) + (((offset) & 3) + ((block_id) & 1) * 0x200000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_LAPX_QUEX_STATE(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 2)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 2)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_LAPX_QUEX_STATE(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x000118000C050000ull) + (((offset) & 3) + ((block_id) & 1) * 0x200000ull) * 8;
}
#else
#define CVMX_LAPX_QUEX_STATE(offset, block_id) (CVMX_ADD_IO_SEG(0x000118000C050000ull) + (((offset) & 3) + ((block_id) & 1) * 0x200000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_LAPX_RESP_STATE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_LAPX_RESP_STATE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118000C010108ull) + ((offset) & 1) * 0x1000000ull;
}
#else
#define CVMX_LAPX_RESP_STATE(offset) (CVMX_ADD_IO_SEG(0x000118000C010108ull) + ((offset) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_LAPX_SFT_RST(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_LAPX_SFT_RST(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118000C000008ull) + ((offset) & 1) * 0x1000000ull;
}
#else
#define CVMX_LAPX_SFT_RST(offset) (CVMX_ADD_IO_SEG(0x000118000C000008ull) + ((offset) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_LAPX_STA_ERR_ST(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_LAPX_STA_ERR_ST(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118000C010208ull) + ((offset) & 1) * 0x1000000ull;
}
#else
#define CVMX_LAPX_STA_ERR_ST(offset) (CVMX_ADD_IO_SEG(0x000118000C010208ull) + ((offset) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_LAPX_TIMEOUT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_LAPX_TIMEOUT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118000C010060ull) + ((offset) & 1) * 0x1000000ull;
}
#else
#define CVMX_LAPX_TIMEOUT(offset) (CVMX_ADD_IO_SEG(0x000118000C010060ull) + ((offset) & 1) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_LAPX_XID_POS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_LAPX_XID_POS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118000C010070ull) + ((offset) & 1) * 0x1000000ull;
}
#else
#define CVMX_LAPX_XID_POS(offset) (CVMX_ADD_IO_SEG(0x000118000C010070ull) + ((offset) & 1) * 0x1000000ull)
#endif

/**
 * cvmx_lap#_bist_result
 *
 * This register provides access to the internal BIST results. Each bit is the BIST result of an
 * individual memory (per bit, 0 = pass and 1 = fail).
 */
union cvmx_lapx_bist_result {
	uint64_t u64;
	struct cvmx_lapx_bist_result_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t nbr                          : 1;  /**< BIST result of the NBR memory. */
	uint64_t edat                         : 1;  /**< BIST result of the EDAT memory. */
	uint64_t emsk                         : 1;  /**< BIST result of the EMSK memory. */
	uint64_t reserved_4_5                 : 2;
	uint64_t lab_dat                      : 2;  /**< BIST result of the LAB_DAT memory. */
	uint64_t ctl_nxt                      : 1;  /**< BIST result of the CTL_NXT memory. */
	uint64_t ctl_sta                      : 1;  /**< BIST result of the CTL_STA memory. */
#else
	uint64_t ctl_sta                      : 1;
	uint64_t ctl_nxt                      : 1;
	uint64_t lab_dat                      : 2;
	uint64_t reserved_4_5                 : 2;
	uint64_t emsk                         : 1;
	uint64_t edat                         : 1;
	uint64_t nbr                          : 1;
	uint64_t reserved_9_63                : 55;
#endif
	} s;
	struct cvmx_lapx_bist_result_s        cn78xx;
	struct cvmx_lapx_bist_result_s        cn78xxp1;
};
typedef union cvmx_lapx_bist_result cvmx_lapx_bist_result_t;

/**
 * cvmx_lap#_cfg
 *
 * This register contains flags to control the LAP unit.
 *
 */
union cvmx_lapx_cfg {
	uint64_t u64;
	struct cvmx_lapx_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t nbr_byp_dis                  : 1;  /**< Disable the NBR FIFO RAM bypass. For diagnostic use only. */
	uint64_t drop_xoff                    : 1;  /**< Drop outbound traffic on timeout and XOFF:
                                                         0 = If an ILA/ASE channel is backpressured, and a response timeout occurs, leave the
                                                         untransmitted traffic for later transmission after XON.
                                                         1 = If an ILA/ASE channel is backpressured, and a response timeout occurs, drop all
                                                         untransmitted traffic until XON returns. */
	uint64_t time_xoff                    : 1;  /**< Timeout increment on XOFF:
                                                         0 = If an ILA/ASE channel is backpressured, the response timeout counters are paused.
                                                         1 = If an ILA/ASE channel is backpressured, the response timeout counters continue to run.
                                                         This may result in timeouts due to temporary backpressure. */
	uint64_t ooo                          : 1;  /**< Out-of-order transaction ID mode:
                                                         0 = FIFO ordering on each ILA/ASE channel, no transaction id (XID) inserted.
                                                         1 = Allow out of order returns of packets from the TCAM/ASE; insert a transaction id (XID)
                                                         into request packets, and expect one in response packets.
                                                         LAP(1)_CFG[OOO] must be set as the ASE only supports out-of-order mode. */
	uint64_t ena                          : 1;  /**< Enable LAP.
                                                         0 = LAP disabled, any lookups will result in out-of-LAB interrupt.
                                                         1 = LAP enabled.
                                                         When ENA transitions from 0 to 1, LAP will build the free list and empty all queue lists.
                                                         Results are unpredictable if ENA is toggled with traffic outstanding. */
	uint64_t lab_size                     : 3;  /**< Number of LABs versus size of each LAB. This register may only be changed when [ENA]=0.
                                                         0x0 = 128 LABs, 16 words/LAB (1024 bits).
                                                         0x1 = 170 LABs, 12 words/LAB (768 bits).
                                                         0x2 = 256 LABs, 8 words/LAB (512 bits).
                                                         0x3-0x7 Reserved. */
#else
	uint64_t lab_size                     : 3;
	uint64_t ena                          : 1;
	uint64_t ooo                          : 1;
	uint64_t time_xoff                    : 1;
	uint64_t drop_xoff                    : 1;
	uint64_t nbr_byp_dis                  : 1;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_lapx_cfg_s                cn78xx;
	struct cvmx_lapx_cfg_s                cn78xxp1;
};
typedef union cvmx_lapx_cfg cvmx_lapx_cfg_t;

/**
 * cvmx_lap#_edat_err_st
 *
 * This register is for diagnostic use only.
 *
 */
union cvmx_lapx_edat_err_st {
	uint64_t u64;
	struct cvmx_lapx_edat_err_st_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t fsyn                         : 8;  /**< Syndrome of last expected mask ram ECC error. Latched when LAP()_GEN_INT[EDAT_SBE] or
                                                         [EDAT_DBE] set */
	uint64_t reserved_4_15                : 12;
	uint64_t fadr                         : 4;  /**< Address of last expected mask ram ECC error. Latched when LAP()_GEN_INT[EDAT_SBE] or
                                                         [EDAT_DBE] set. */
#else
	uint64_t fadr                         : 4;
	uint64_t reserved_4_15                : 12;
	uint64_t fsyn                         : 8;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_lapx_edat_err_st_s        cn78xx;
	struct cvmx_lapx_edat_err_st_s        cn78xxp1;
};
typedef union cvmx_lapx_edat_err_st cvmx_lapx_edat_err_st_t;

/**
 * cvmx_lap#_emsk_err_st
 *
 * This register is for diagnostic use only.
 *
 */
union cvmx_lapx_emsk_err_st {
	uint64_t u64;
	struct cvmx_lapx_emsk_err_st_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t fsyn                         : 8;  /**< Syndrome of last expected data ram ECC error. Latched when LAP()_GEN_INT[EMSK_SBE] or
                                                         [EMSK_DBE] set */
	uint64_t reserved_4_15                : 12;
	uint64_t fadr                         : 4;  /**< Address of last expected data ram ECC error. Latched when LAP()_GEN_INT[EMSK_SBE] or
                                                         [EMSK_DBE] set. */
#else
	uint64_t fadr                         : 4;
	uint64_t reserved_4_15                : 12;
	uint64_t fsyn                         : 8;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_lapx_emsk_err_st_s        cn78xx;
	struct cvmx_lapx_emsk_err_st_s        cn78xxp1;
};
typedef union cvmx_lapx_emsk_err_st cvmx_lapx_emsk_err_st_t;

/**
 * cvmx_lap#_err_cfg
 */
union cvmx_lapx_err_cfg {
	uint64_t u64;
	struct cvmx_lapx_err_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_28_63               : 36;
	uint64_t nbr_flip                     : 2;  /**< Testing feature. Flip syndrome bits <1:0> on writes to the NBR ram to test single-bit or
                                                         double-bit errors. */
	uint64_t edat_flip                    : 2;  /**< Testing feature. Flip syndrome bits <1:0> on writes to the EDAT ram to test single-bit or
                                                         double-bit errors. */
	uint64_t emsk_flip                    : 2;  /**< Testing feature. Flip syndrome bits <1:0> on writes to the EMSK ram to test single-bit or
                                                         double-bit errors. */
	uint64_t nxt_flip                     : 2;  /**< Testing feature. Flip syndrome bits <1:0> on writes to the NXT ram to test single-bit or
                                                         double-bit errors. */
	uint64_t sta_flip                     : 2;  /**< Testing feature. Flip syndrome bits <1:0> on writes to the STA ram to test single-bit or
                                                         double-bit errors. */
	uint64_t lab_flip                     : 2;  /**< Testing feature. Flip syndrome bits <1:0> on writes to the LAB ram to test single-bit or
                                                         double-bit errors. */
	uint64_t reserved_6_15                : 10;
	uint64_t nbr_cor_dis                  : 1;  /**< Disable ECC corrector on NBR. */
	uint64_t edat_cor_dis                 : 1;  /**< Disable ECC corrector on EDAT. */
	uint64_t emsk_cor_dis                 : 1;  /**< Disable ECC corrector on EMSK. */
	uint64_t nxt_cor_dis                  : 1;  /**< Disable ECC corrector on NXT. */
	uint64_t sta_cor_dis                  : 1;  /**< Disable ECC corrector on STA. */
	uint64_t lab_cor_dis                  : 1;  /**< Disable ECC corrector on LAB. */
#else
	uint64_t lab_cor_dis                  : 1;
	uint64_t sta_cor_dis                  : 1;
	uint64_t nxt_cor_dis                  : 1;
	uint64_t emsk_cor_dis                 : 1;
	uint64_t edat_cor_dis                 : 1;
	uint64_t nbr_cor_dis                  : 1;
	uint64_t reserved_6_15                : 10;
	uint64_t lab_flip                     : 2;
	uint64_t sta_flip                     : 2;
	uint64_t nxt_flip                     : 2;
	uint64_t emsk_flip                    : 2;
	uint64_t edat_flip                    : 2;
	uint64_t nbr_flip                     : 2;
	uint64_t reserved_28_63               : 36;
#endif
	} s;
	struct cvmx_lapx_err_cfg_s            cn78xx;
	struct cvmx_lapx_err_cfg_s            cn78xxp1;
};
typedef union cvmx_lapx_err_cfg cvmx_lapx_err_cfg_t;

/**
 * cvmx_lap#_exp#_data
 *
 * Configures exception error masking.
 *
 */
union cvmx_lapx_expx_data {
	uint64_t u64;
	struct cvmx_lapx_expx_data_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Data value expected. The packet must have bits matching this value if the corresponding
                                                         bits in LAP()_EXP()_VALID are set. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_lapx_expx_data_s          cn78xx;
	struct cvmx_lapx_expx_data_s          cn78xxp1;
};
typedef union cvmx_lapx_expx_data cvmx_lapx_expx_data_t;

/**
 * cvmx_lap#_exp#_valid
 *
 * Configures exception error masking.
 *
 */
union cvmx_lapx_expx_valid {
	uint64_t u64;
	struct cvmx_lapx_expx_valid_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t valid                        : 64; /**< Valid mask. Each bit corresponds to a bit in LAP()_EXP()_VALID:
                                                         0 = Corresponding bit is a don't care.
                                                         1 = Corresponding bit compared against LAP()_EXP()_VALID.
                                                         Note that some response bits indicated by LAP_CTL_RTN_S are for Interlaken control, and
                                                         thus should always be zero (don't care) in LAP(0..1)_EXP_VALID(0). */
#else
	uint64_t valid                        : 64;
#endif
	} s;
	struct cvmx_lapx_expx_valid_s         cn78xx;
	struct cvmx_lapx_expx_valid_s         cn78xxp1;
};
typedef union cvmx_lapx_expx_valid cvmx_lapx_expx_valid_t;

/**
 * cvmx_lap#_free_state
 *
 * This register is for diagnostic use only.
 *
 */
union cvmx_lapx_free_state {
	uint64_t u64;
	struct cvmx_lapx_free_state_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_40_63               : 24;
	uint64_t tail                         : 8;  /**< LAB number that is at tail of list. Unspecified if [LABS] = 0. */
	uint64_t reserved_24_31               : 8;
	uint64_t head                         : 8;  /**< LAB number that is at head of list. Unspecified if [LABS] = 0. */
	uint64_t reserved_9_15                : 7;
	uint64_t labs                         : 9;  /**< Number of LABs on list. */
#else
	uint64_t labs                         : 9;
	uint64_t reserved_9_15                : 7;
	uint64_t head                         : 8;
	uint64_t reserved_24_31               : 8;
	uint64_t tail                         : 8;
	uint64_t reserved_40_63               : 24;
#endif
	} s;
	struct cvmx_lapx_free_state_s         cn78xx;
	struct cvmx_lapx_free_state_s         cn78xxp1;
};
typedef union cvmx_lapx_free_state cvmx_lapx_free_state_t;

/**
 * cvmx_lap#_gen_int
 *
 * This register contains error flags for LAP.
 *
 */
union cvmx_lapx_gen_int {
	uint64_t u64;
	struct cvmx_lapx_gen_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_19_63               : 45;
	uint64_t xid_bad                      : 1;  /**< A response packet's transaction ID was targeted to an LAB not in PROCESSING state. Not
                                                         reported if packet also has CRC or MISMATCH errors. Typically indicates TCAM or
                                                         configuration error. Throws LAP_INTSN_E::LAP()_GEN_XID_BAD. */
	uint64_t nbr_dbe                      : 1;  /**< An ECC uncorrectable error has occurred in the NBR RAM. Throws
                                                         LAP_INTSN_E::LAP()_GEN_NBR_DBE. */
	uint64_t nbr_sbe                      : 1;  /**< An ECC correctable error has occurred in the NBR RAM. Throws
                                                         LAP_INTSN_E::LAP()_GEN_NBR_SBE. */
	uint64_t edat_dbe                     : 1;  /**< An ECC uncorrectable error has occurred in the EDAT RAM. Throws
                                                         LAP_INTSN_E::LAP()_GEN_EDAT_DBE. See also LAP()_EDAT_ERR_ST. */
	uint64_t edat_sbe                     : 1;  /**< An ECC correctable error has occurred in the EDAT RAM. Throws
                                                         LAP_INTSN_E::LAP()_GEN_EDAT_SBE. See also LAP()_EDAT_ERR_ST. */
	uint64_t emsk_dbe                     : 1;  /**< An ECC uncorrectable error has occurred in the EMSK RAM. Throws
                                                         LAP_INTSN_E::LAP()_GEN_EMSK_DBE. See also LAP()_EMSK_ERR_ST. */
	uint64_t emsk_sbe                     : 1;  /**< An ECC correctable error has occurred in the EMSK RAM. Throws
                                                         LAP_INTSN_E::LAP()_GEN_EMSK_SBE. See also LAP()_EMSK_ERR_ST. */
	uint64_t nxt_dbe                      : 1;  /**< An ECC uncorrectable error has occurred in the NXT RAM. Throws
                                                         LAP_INTSN_E::LAP()_GEN_NXT_DBE. See also LAP()_NXT_ERR_ST. */
	uint64_t nxt_sbe                      : 1;  /**< An ECC correctable error has occurred in the NXT RAM. Throws
                                                         LAP_INTSN_E::LAP()_GEN_NXT_SBE. See also LAP()_NXT_ERR_ST. */
	uint64_t sta_dbe                      : 1;  /**< An ECC uncorrectable error has occurred in the STA RAM. Throws
                                                         LAP_INTSN_E::LAP()_GEN_STA_DBE. See also LAP()_STA_ERR_ST. */
	uint64_t sta_sbe                      : 1;  /**< An ECC correctable error has occurred in the STA RAM. Throws
                                                         LAP_INTSN_E::LAP()_GEN_STA_SBE. See also LAP()_STA_ERR_ST. */
	uint64_t lab_dbe                      : 1;  /**< An ECC uncorrectable error has occurred in the LAB RAM. Throws
                                                         LAP_INTSN_E::LAP()_GEN_LAB_DBE. See also LAP()_LAB_ERR_ST. */
	uint64_t lab_sbe                      : 1;  /**< An ECC correctable error has occurred in the LAB RAM. Throws
                                                         LAP_INTSN_E::LAP()_GEN_LAB_SBE. See also LAP()_LAB_ERR_ST. */
	uint64_t reserved_4_5                 : 2;
	uint64_t timeout                      : 1;  /**< Indication timer transitioned an LAB to error state. This interrupt will typically be
                                                         masked off, as error delivery can be in-band. Throws LAP_INTSN_E::LAP()_GEN_TIMEOUT. */
	uint64_t new_exc                      : 1;  /**< Indication the exception queue contains any received packet. Software should check the
                                                         exception queue for new packets. Throws LAP_INTSN_E::LAP()_GEN_NEW_EXC. */
	uint64_t lost_exc                     : 1;  /**< Error indicating exception packet received with no LABs available on the exception queue;
                                                         the exception packet was dropped. Throws LAP_INTSN_E::LAP()_GEN_LOST_EXC. */
	uint64_t labs_out                     : 1;  /**< Error indicating did push with no free LABs available, or
                                                         LAP()_QUE()_CFG[MAX_LABS] was exceeded. Throws
                                                         LAP_INTSN_E::LAP()_GEN_LABS_OUT. */
#else
	uint64_t labs_out                     : 1;
	uint64_t lost_exc                     : 1;
	uint64_t new_exc                      : 1;
	uint64_t timeout                      : 1;
	uint64_t reserved_4_5                 : 2;
	uint64_t lab_sbe                      : 1;
	uint64_t lab_dbe                      : 1;
	uint64_t sta_sbe                      : 1;
	uint64_t sta_dbe                      : 1;
	uint64_t nxt_sbe                      : 1;
	uint64_t nxt_dbe                      : 1;
	uint64_t emsk_sbe                     : 1;
	uint64_t emsk_dbe                     : 1;
	uint64_t edat_sbe                     : 1;
	uint64_t edat_dbe                     : 1;
	uint64_t nbr_sbe                      : 1;
	uint64_t nbr_dbe                      : 1;
	uint64_t xid_bad                      : 1;
	uint64_t reserved_19_63               : 45;
#endif
	} s;
	struct cvmx_lapx_gen_int_s            cn78xx;
	struct cvmx_lapx_gen_int_s            cn78xxp1;
};
typedef union cvmx_lapx_gen_int cvmx_lapx_gen_int_t;

/**
 * cvmx_lap#_lab#_state
 *
 * This register indicates the state of the LAB. This is intended for diagnostics and resource
 * recovery; IOBDMA Read operations would normally be used to read this state instead.
 */
union cvmx_lapx_labx_state {
	uint64_t u64;
	struct cvmx_lapx_labx_state_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_56_63               : 8;
	uint64_t next                         : 8;  /**< Next LAB in chain of LABs assigned to this LAB's queue. */
	uint64_t reserved_16_47               : 32;
	uint64_t length                       : 4;  /**< Length of received data in words minus one. */
	uint64_t reserved_3_11                : 9;
	uint64_t dmapend                      : 1;  /**< IOBDMA or LMTDMA pending against this entry. */
	uint64_t recv                         : 1;  /**< Received. Indicates the LAB is in RECEIVED state. */
	uint64_t valid                        : 1;  /**< Valid. Set to indicate the LAB is non-EMPTY. If clear, all other fields in this register
                                                         are stale and ignored by hardware. */
#else
	uint64_t valid                        : 1;
	uint64_t recv                         : 1;
	uint64_t dmapend                      : 1;
	uint64_t reserved_3_11                : 9;
	uint64_t length                       : 4;
	uint64_t reserved_16_47               : 32;
	uint64_t next                         : 8;
	uint64_t reserved_56_63               : 8;
#endif
	} s;
	struct cvmx_lapx_labx_state_s         cn78xx;
	struct cvmx_lapx_labx_state_s         cn78xxp1;
};
typedef union cvmx_lapx_labx_state cvmx_lapx_labx_state_t;

/**
 * cvmx_lap#_lab_data#
 *
 * This register reads raw data from the LABs. The address is calculated from (LAB_number *
 * words_per_lab_from_table_in_LAP()_CFG[LAB_SIZE] + offset_in_LAB) * 8.
 */
union cvmx_lapx_lab_datax {
	uint64_t u64;
	struct cvmx_lapx_lab_datax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Data for specified packet word. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_lapx_lab_datax_s          cn78xx;
	struct cvmx_lapx_lab_datax_s          cn78xxp1;
};
typedef union cvmx_lapx_lab_datax cvmx_lapx_lab_datax_t;

/**
 * cvmx_lap#_lab_err_st
 *
 * This register is for diagnostic use only.
 *
 */
union cvmx_lapx_lab_err_st {
	uint64_t u64;
	struct cvmx_lapx_lab_err_st_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_26_63               : 38;
	uint64_t fsyn                         : 10; /**< Syndrome of last LAB data ram ECC error. Latched when LAP()_GEN_INT[LAB_SBE] or [LAB_DBE] set */
	uint64_t reserved_10_15               : 6;
	uint64_t fadr                         : 10; /**< Reserved. */
#else
	uint64_t fadr                         : 10;
	uint64_t reserved_10_15               : 6;
	uint64_t fsyn                         : 10;
	uint64_t reserved_26_63               : 38;
#endif
	} s;
	struct cvmx_lapx_lab_err_st_s         cn78xx;
	struct cvmx_lapx_lab_err_st_s         cn78xxp1;
};
typedef union cvmx_lapx_lab_err_st cvmx_lapx_lab_err_st_t;

/**
 * cvmx_lap#_nxt_err_st
 *
 * This register is for diagnostic use only.
 *
 */
union cvmx_lapx_nxt_err_st {
	uint64_t u64;
	struct cvmx_lapx_nxt_err_st_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_21_63               : 43;
	uint64_t fsyn                         : 5;  /**< Syndrome of last Next Pointer Ram ECC error. Latched when LAP()_GEN_INT[NXT_SBE] or
                                                         [NXT_DBE] set */
	uint64_t reserved_8_15                : 8;
	uint64_t fadr                         : 8;  /**< Address of last Next Pointer Ram ECC error. Latched when LAP()_GEN_INT[NXT_SBE] or
                                                         [NXT_DBE] set. */
#else
	uint64_t fadr                         : 8;
	uint64_t reserved_8_15                : 8;
	uint64_t fsyn                         : 5;
	uint64_t reserved_21_63               : 43;
#endif
	} s;
	struct cvmx_lapx_nxt_err_st_s         cn78xx;
	struct cvmx_lapx_nxt_err_st_s         cn78xxp1;
};
typedef union cvmx_lapx_nxt_err_st cvmx_lapx_nxt_err_st_t;

/**
 * cvmx_lap#_que#_cfg
 */
union cvmx_lapx_quex_cfg {
	uint64_t u64;
	struct cvmx_lapx_quex_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t max_labs                     : 9;  /**< Maximum number of LABS allowed to be assigned to this queue; compared against
                                                         LAP()_QUE()_STATE[LABS_RX] + LAP()_QUE()_STATE[LABS_PROC] to generate
                                                         errors. The total across all queues' [MAX_LABS] may be over-provisioned, in which case the
                                                         global LAP()_CFG[LAB_SIZE] number of LABs will throttle the
                                                         transaction count. */
#else
	uint64_t max_labs                     : 9;
	uint64_t reserved_9_63                : 55;
#endif
	} s;
	struct cvmx_lapx_quex_cfg_s           cn78xx;
	struct cvmx_lapx_quex_cfg_s           cn78xxp1;
};
typedef union cvmx_lapx_quex_cfg cvmx_lapx_quex_cfg_t;

/**
 * cvmx_lap#_que#_state
 */
union cvmx_lapx_quex_state {
	uint64_t u64;
	struct cvmx_lapx_quex_state_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_56_63               : 8;
	uint64_t tail                         : 8;  /**< LAB number that is at tail of this queue; the most recent one to enter the PROCESSING
                                                         state (though it may now be in RECEIVED). Unspecified if [LABS_PROC]+[LABS_RX]=0. */
	uint64_t reserved_44_47               : 4;
	uint64_t head_proc                    : 8;  /**< LAB number that is at head of the PROCESSING state list this queue. Unspecified if [LABS_PROC]=0. */
	uint64_t reserved_32_35               : 4;
	uint64_t head                         : 8;  /**< LAB number that is at head of the PROCESSING or RECEIVED state list in this queue.
                                                         Unspecified if [LABS_PROC]+[LABS_RX]=0. Valid for exception queue QUE(2) only, otherwise
                                                         diagnostic use only. */
	uint64_t reserved_21_23               : 3;
	uint64_t labs_proc                    : 9;  /**< Number of LABs in PROCESSING state in this queue. */
	uint64_t reserved_9_11                : 3;
	uint64_t labs_rx                      : 9;  /**< Number of LABs in RECEIVED state in this queue. Valid for exception queue QUE(2) only,
                                                         otherwise diagnostic use only. */
#else
	uint64_t labs_rx                      : 9;
	uint64_t reserved_9_11                : 3;
	uint64_t labs_proc                    : 9;
	uint64_t reserved_21_23               : 3;
	uint64_t head                         : 8;
	uint64_t reserved_32_35               : 4;
	uint64_t head_proc                    : 8;
	uint64_t reserved_44_47               : 4;
	uint64_t tail                         : 8;
	uint64_t reserved_56_63               : 8;
#endif
	} s;
	struct cvmx_lapx_quex_state_s         cn78xx;
	struct cvmx_lapx_quex_state_s         cn78xxp1;
};
typedef union cvmx_lapx_quex_state cvmx_lapx_quex_state_t;

/**
 * cvmx_lap#_resp_state
 *
 * This register is for diagnostic use only.
 *
 */
union cvmx_lapx_resp_state {
	uint64_t u64;
	struct cvmx_lapx_resp_state_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_40_63               : 24;
	uint64_t tail                         : 8;  /**< LAB number that is at tail of list. Unspecified if [LABS] = 0. */
	uint64_t reserved_24_31               : 8;
	uint64_t head                         : 8;  /**< LAB number that is at head of list. Unspecified if [LABS] = 0. */
	uint64_t reserved_9_15                : 7;
	uint64_t labs                         : 9;  /**< Number of LABs on list. */
#else
	uint64_t labs                         : 9;
	uint64_t reserved_9_15                : 7;
	uint64_t head                         : 8;
	uint64_t reserved_24_31               : 8;
	uint64_t tail                         : 8;
	uint64_t reserved_40_63               : 24;
#endif
	} s;
	struct cvmx_lapx_resp_state_s         cn78xx;
	struct cvmx_lapx_resp_state_s         cn78xxp1;
};
typedef union cvmx_lapx_resp_state cvmx_lapx_resp_state_t;

/**
 * cvmx_lap#_sft_rst
 */
union cvmx_lapx_sft_rst {
	uint64_t u64;
	struct cvmx_lapx_sft_rst_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t busy                         : 1;  /**< Initialization in progress. After reset asserts, LAP will set this bit until internal
                                                         structures are initialized. This bit must be read as zero before any configuration may be
                                                         done. */
	uint64_t reserved_1_62                : 62;
	uint64_t reset                        : 1;  /**< Reset. When set, causes a reset of the LAP, except RSL. */
#else
	uint64_t reset                        : 1;
	uint64_t reserved_1_62                : 62;
	uint64_t busy                         : 1;
#endif
	} s;
	struct cvmx_lapx_sft_rst_s            cn78xx;
	struct cvmx_lapx_sft_rst_s            cn78xxp1;
};
typedef union cvmx_lapx_sft_rst cvmx_lapx_sft_rst_t;

/**
 * cvmx_lap#_sta_err_st
 *
 * This register is for diagnostic use only.
 *
 */
union cvmx_lapx_sta_err_st {
	uint64_t u64;
	struct cvmx_lapx_sta_err_st_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_23_63               : 41;
	uint64_t fsyn                         : 7;  /**< Syndrome of last LAB state ram ECC error. Latched when LAP()_GEN_INT[STA_SBE] or [STA_DBE] set */
	uint64_t reserved_8_15                : 8;
	uint64_t fadr                         : 8;  /**< Address of last LAB state ram ECC error. Latched when LAP()_GEN_INT[STA_SBE] or [STA_DBE] set. */
#else
	uint64_t fadr                         : 8;
	uint64_t reserved_8_15                : 8;
	uint64_t fsyn                         : 7;
	uint64_t reserved_23_63               : 41;
#endif
	} s;
	struct cvmx_lapx_sta_err_st_s         cn78xx;
	struct cvmx_lapx_sta_err_st_s         cn78xxp1;
};
typedef union cvmx_lapx_sta_err_st cvmx_lapx_sta_err_st_t;

/**
 * cvmx_lap#_timeout
 */
union cvmx_lapx_timeout {
	uint64_t u64;
	struct cvmx_lapx_timeout_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t iobdma                       : 8;  /**< Timeout waiting for an IOBDMA in number of SCLKs minus one divided by 256. After between
                                                         one and two times this interval an IOBDMA request waiting for a packet will return no-data
                                                         as defined in Interlaken Control Word.
                                                         0x0 = Timeout between 256 and 511 cycles.
                                                         0x1 = Timeout between 512 and 1023 cycles.
                                                         0x2 = Timeout between 768 and 1535 cycles.
                                                         _ etc. */
	uint64_t reserved_12_15               : 4;
	uint64_t resp                         : 12; /**< Timeout waiting for a response in number of SCLKs minus one divided by 256. After between
                                                         one and two times this interval an in-flight LAB will be considered lost and marked as
                                                         RECEIVED with error. RESP must be set to >= (2 * LAP()_TIMEOUT[IOBDMA] + 1).
                                                         0x0 = Timeout between 256 and 511 cycles.
                                                         0x1 = Timeout between 512 and 1023 cycles.
                                                         0x2 = Timeout between 768 and 1535 cycles.
                                                         _ etc. */
#else
	uint64_t resp                         : 12;
	uint64_t reserved_12_15               : 4;
	uint64_t iobdma                       : 8;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_lapx_timeout_s            cn78xx;
	struct cvmx_lapx_timeout_s            cn78xxp1;
};
typedef union cvmx_lapx_timeout cvmx_lapx_timeout_t;

/**
 * cvmx_lap#_xid_pos
 *
 * Configures how to insert and extract transaction ids.
 *
 */
union cvmx_lapx_xid_pos {
	uint64_t u64;
	struct cvmx_lapx_xid_pos_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_33_63               : 31;
	uint64_t as_only                      : 1;  /**< Insert/extract in Interlaken-LA application specific AS fields. If set and REQ_WD or
                                                         RTN_WD are zero, the XID field that is inserted or extracted respectively will skip over
                                                         the non-application specific fields in the Interlaken LA header. */
	uint64_t reserved_26_31               : 6;
	uint64_t rtn_wd                       : 4;  /**< Extract transaction tag from this 64-bit word number of the return packet; typically the
                                                         same value as [REQ_WD]. Word 0 is the Interlaken control word, word 1 is the first word of
                                                         payload. */
	uint64_t rtn_lsb                      : 6;  /**< Extract transaction tag value's LSB from this position of the of request packet;
                                                         typically the same value as [REQ_LSB]. When [AS_ONLY] = 0, refers to a bit
                                                         position in the input word. When [AS_ONLY] = 1, refers to a bit position within
                                                         the Interlaken-LA application specific AS fields. */
	uint64_t reserved_10_15               : 6;
	uint64_t req_wd                       : 4;  /**< Insert transaction tag into this 64-bit word number of the request packet. Word 0 is the
                                                         Interlaken control word, word 1 is the first word of payload. */
	uint64_t req_lsb                      : 6;  /**< Insert transaction tag value's LSB into this LSB of the of request packet. When
                                                         [AS_ONLY] = 0, refers to a bit position in the output word. When [AS_ONLY] = 1,
                                                         refers to a bit position within the Interlaken-LA application specific AS
                                                         fields. */
#else
	uint64_t req_lsb                      : 6;
	uint64_t req_wd                       : 4;
	uint64_t reserved_10_15               : 6;
	uint64_t rtn_lsb                      : 6;
	uint64_t rtn_wd                       : 4;
	uint64_t reserved_26_31               : 6;
	uint64_t as_only                      : 1;
	uint64_t reserved_33_63               : 31;
#endif
	} s;
	struct cvmx_lapx_xid_pos_s            cn78xx;
	struct cvmx_lapx_xid_pos_s            cn78xxp1;
};
typedef union cvmx_lapx_xid_pos cvmx_lapx_xid_pos_t;

#endif
