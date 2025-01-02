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
 * cvmx-rnm-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon rnm.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_RNM_DEFS_H__
#define __CVMX_RNM_DEFS_H__

#define CVMX_RNM_BIST_STATUS (CVMX_ADD_IO_SEG(0x0001180040000008ull))
#define CVMX_RNM_CTL_STATUS (CVMX_ADD_IO_SEG(0x0001180040000000ull))
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_RNM_EER_DBG CVMX_RNM_EER_DBG_FUNC()
static inline uint64_t CVMX_RNM_EER_DBG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF71XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_RNM_EER_DBG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180040000018ull);
}
#else
#define CVMX_RNM_EER_DBG (CVMX_ADD_IO_SEG(0x0001180040000018ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_RNM_EER_KEY CVMX_RNM_EER_KEY_FUNC()
static inline uint64_t CVMX_RNM_EER_KEY_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF71XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_RNM_EER_KEY not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180040000010ull);
}
#else
#define CVMX_RNM_EER_KEY (CVMX_ADD_IO_SEG(0x0001180040000010ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_RNM_SERIAL_NUM CVMX_RNM_SERIAL_NUM_FUNC()
static inline uint64_t CVMX_RNM_SERIAL_NUM_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF71XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_RNM_SERIAL_NUM not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180040000020ull);
}
#else
#define CVMX_RNM_SERIAL_NUM (CVMX_ADD_IO_SEG(0x0001180040000020ull))
#endif

/**
 * cvmx_rnm_bist_status
 *
 * This register is the RNM memory BIST status register, indicating status of built-in self-
 * tests. 0 = passed BIST, 1 = failed BIST.
 */
union cvmx_rnm_bist_status {
	uint64_t u64;
	struct cvmx_rnm_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t rrc                          : 1;  /**< Status of the RRC memory block BIST. 0 = passed BIST, 1 = failed BIST. */
	uint64_t mem                          : 1;  /**< Status of MEM memory block BIST. 0 = passed BIST, 1 = failed BIST. */
#else
	uint64_t mem                          : 1;
	uint64_t rrc                          : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_rnm_bist_status_s         cn30xx;
	struct cvmx_rnm_bist_status_s         cn31xx;
	struct cvmx_rnm_bist_status_s         cn38xx;
	struct cvmx_rnm_bist_status_s         cn38xxp2;
	struct cvmx_rnm_bist_status_s         cn50xx;
	struct cvmx_rnm_bist_status_s         cn52xx;
	struct cvmx_rnm_bist_status_s         cn52xxp1;
	struct cvmx_rnm_bist_status_s         cn56xx;
	struct cvmx_rnm_bist_status_s         cn56xxp1;
	struct cvmx_rnm_bist_status_s         cn58xx;
	struct cvmx_rnm_bist_status_s         cn58xxp1;
	struct cvmx_rnm_bist_status_s         cn61xx;
	struct cvmx_rnm_bist_status_s         cn63xx;
	struct cvmx_rnm_bist_status_s         cn63xxp1;
	struct cvmx_rnm_bist_status_s         cn66xx;
	struct cvmx_rnm_bist_status_s         cn68xx;
	struct cvmx_rnm_bist_status_s         cn68xxp1;
	struct cvmx_rnm_bist_status_s         cn70xx;
	struct cvmx_rnm_bist_status_s         cn70xxp1;
	struct cvmx_rnm_bist_status_s         cn73xx;
	struct cvmx_rnm_bist_status_s         cn78xx;
	struct cvmx_rnm_bist_status_s         cn78xxp1;
	struct cvmx_rnm_bist_status_s         cnf71xx;
	struct cvmx_rnm_bist_status_s         cnf75xx;
};
typedef union cvmx_rnm_bist_status cvmx_rnm_bist_status_t;

/**
 * cvmx_rnm_ctl_status
 *
 * RNM_CTL_STATUS = RNM's Control/Status Register
 *
 * The RNM's interrupt enable register.
 */
union cvmx_rnm_ctl_status {
	uint64_t u64;
	struct cvmx_rnm_ctl_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t dis_mak                      : 1;  /**< Disable use of master AES KEY. */
	uint64_t eer_lck                      : 1;  /**< Encryption enable register locked. */
	uint64_t eer_val                      : 1;  /**< Dormant encryption key match. */
	uint64_t ent_sel                      : 4;  /**< Select input to RNM FIFO.
                                                         0x0 = 0-7.
                                                         0x1 = 8-15.
                                                         0x2 = 16-23.
                                                         0x3 = 24-31.
                                                         0x4 = 32-39.
                                                         0x5 = 40-47.
                                                         0x6 = 48-55.
                                                         0x7 = 56-63.
                                                         0x8 = 64-71.
                                                         0x9 = 72-79.
                                                         0xA = 80-87.
                                                         0xB = 88-95.
                                                         0xC = 96-103.
                                                         0xD = 104-111.
                                                         0xE = 112-119.
                                                         0xF = 120-127. */
	uint64_t exp_ent                      : 1;  /**< Exported entropy enable for random number generator. The next random number is
                                                         available 80 coprocessor-clock cycles after switching this bit from 0 to 1. The
                                                         next random number is available 730 coprocessor-clock cycles after switching this
                                                         bit from 1 to 0. */
	uint64_t rng_rst                      : 1;  /**< Reset the RNG. Setting this bit to 1 cancels the generation of the current random
                                                         number. The next random number is available 730 coprocessor-clock cycles after this
                                                         bit is cleared if [EXP_ENT] is set to 0. The next random number is available 80
                                                         coprocessor-clock cycles after this bit is cleared if [EXP_ENT] is set to 1. This bit is
                                                         not automatically cleared. */
	uint64_t rnm_rst                      : 1;  /**< Reset the RNM. Setting this bit to 1 drops all RNM transactions in flight and clears
                                                         all stored numbers in the random number memory. Any outstanding NCBO credits will
                                                         not be returned. RNM will not respond to any pending NCBI grants. RNM can accept
                                                         new requests immediately after reset is cleared. This bit is not automatically
                                                         cleared and will not reset any CSR fields. */
	uint64_t rng_en                       : 1;  /**< Enables the output of the RNG. */
	uint64_t ent_en                       : 1;  /**< Entropy enable for random number generator. */
#else
	uint64_t ent_en                       : 1;
	uint64_t rng_en                       : 1;
	uint64_t rnm_rst                      : 1;
	uint64_t rng_rst                      : 1;
	uint64_t exp_ent                      : 1;
	uint64_t ent_sel                      : 4;
	uint64_t eer_val                      : 1;
	uint64_t eer_lck                      : 1;
	uint64_t dis_mak                      : 1;
	uint64_t reserved_12_63               : 52;
#endif
	} s;
	struct cvmx_rnm_ctl_status_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t rng_rst                      : 1;  /**< Reset RNG as core reset. */
	uint64_t rnm_rst                      : 1;  /**< Reset the RNM as core reset except for register
                                                         logic. */
	uint64_t rng_en                       : 1;  /**< Enable the output of the RNG. */
	uint64_t ent_en                       : 1;  /**< Entropy enable for random number generator. */
#else
	uint64_t ent_en                       : 1;
	uint64_t rng_en                       : 1;
	uint64_t rnm_rst                      : 1;
	uint64_t rng_rst                      : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} cn30xx;
	struct cvmx_rnm_ctl_status_cn30xx     cn31xx;
	struct cvmx_rnm_ctl_status_cn30xx     cn38xx;
	struct cvmx_rnm_ctl_status_cn30xx     cn38xxp2;
	struct cvmx_rnm_ctl_status_cn50xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t ent_sel                      : 4;  /**< ? */
	uint64_t exp_ent                      : 1;  /**< Exported entropy enable for random number generator */
	uint64_t rng_rst                      : 1;  /**< Reset RNG as core reset. */
	uint64_t rnm_rst                      : 1;  /**< Reset the RNM as core reset except for register
                                                         logic. */
	uint64_t rng_en                       : 1;  /**< Enable the output of the RNG. */
	uint64_t ent_en                       : 1;  /**< Entropy enable for random number generator. */
#else
	uint64_t ent_en                       : 1;
	uint64_t rng_en                       : 1;
	uint64_t rnm_rst                      : 1;
	uint64_t rng_rst                      : 1;
	uint64_t exp_ent                      : 1;
	uint64_t ent_sel                      : 4;
	uint64_t reserved_9_63                : 55;
#endif
	} cn50xx;
	struct cvmx_rnm_ctl_status_cn50xx     cn52xx;
	struct cvmx_rnm_ctl_status_cn50xx     cn52xxp1;
	struct cvmx_rnm_ctl_status_cn50xx     cn56xx;
	struct cvmx_rnm_ctl_status_cn50xx     cn56xxp1;
	struct cvmx_rnm_ctl_status_cn50xx     cn58xx;
	struct cvmx_rnm_ctl_status_cn50xx     cn58xxp1;
	struct cvmx_rnm_ctl_status_s          cn61xx;
	struct cvmx_rnm_ctl_status_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_11_63               : 53;
	uint64_t eer_lck                      : 1;  /**< Encryption enable register locked */
	uint64_t eer_val                      : 1;  /**< Dormant encryption key match */
	uint64_t ent_sel                      : 4;  /**< ? */
	uint64_t exp_ent                      : 1;  /**< Exported entropy enable for random number generator */
	uint64_t rng_rst                      : 1;  /**< Reset RNG as core reset. */
	uint64_t rnm_rst                      : 1;  /**< Reset the RNM as core reset except for register
                                                         logic. */
	uint64_t rng_en                       : 1;  /**< Enable the output of the RNG. */
	uint64_t ent_en                       : 1;  /**< Entropy enable for random number generator. */
#else
	uint64_t ent_en                       : 1;
	uint64_t rng_en                       : 1;
	uint64_t rnm_rst                      : 1;
	uint64_t rng_rst                      : 1;
	uint64_t exp_ent                      : 1;
	uint64_t ent_sel                      : 4;
	uint64_t eer_val                      : 1;
	uint64_t eer_lck                      : 1;
	uint64_t reserved_11_63               : 53;
#endif
	} cn63xx;
	struct cvmx_rnm_ctl_status_cn63xx     cn63xxp1;
	struct cvmx_rnm_ctl_status_s          cn66xx;
	struct cvmx_rnm_ctl_status_cn63xx     cn68xx;
	struct cvmx_rnm_ctl_status_cn63xx     cn68xxp1;
	struct cvmx_rnm_ctl_status_s          cn70xx;
	struct cvmx_rnm_ctl_status_s          cn70xxp1;
	struct cvmx_rnm_ctl_status_s          cn73xx;
	struct cvmx_rnm_ctl_status_s          cn78xx;
	struct cvmx_rnm_ctl_status_s          cn78xxp1;
	struct cvmx_rnm_ctl_status_s          cnf71xx;
	struct cvmx_rnm_ctl_status_s          cnf75xx;
};
typedef union cvmx_rnm_ctl_status cvmx_rnm_ctl_status_t;

/**
 * cvmx_rnm_eer_dbg
 *
 * This register is the encryption enable debug register.
 *
 */
union cvmx_rnm_eer_dbg {
	uint64_t u64;
	struct cvmx_rnm_eer_dbg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dat                          : 64; /**< Dormant encryption debug info. */
#else
	uint64_t dat                          : 64;
#endif
	} s;
	struct cvmx_rnm_eer_dbg_s             cn61xx;
	struct cvmx_rnm_eer_dbg_s             cn63xx;
	struct cvmx_rnm_eer_dbg_s             cn63xxp1;
	struct cvmx_rnm_eer_dbg_s             cn66xx;
	struct cvmx_rnm_eer_dbg_s             cn68xx;
	struct cvmx_rnm_eer_dbg_s             cn68xxp1;
	struct cvmx_rnm_eer_dbg_s             cn70xx;
	struct cvmx_rnm_eer_dbg_s             cn70xxp1;
	struct cvmx_rnm_eer_dbg_s             cn73xx;
	struct cvmx_rnm_eer_dbg_s             cn78xx;
	struct cvmx_rnm_eer_dbg_s             cn78xxp1;
	struct cvmx_rnm_eer_dbg_s             cnf71xx;
	struct cvmx_rnm_eer_dbg_s             cnf75xx;
};
typedef union cvmx_rnm_eer_dbg cvmx_rnm_eer_dbg_t;

/**
 * cvmx_rnm_eer_key
 *
 * This register is the encryption enable register.
 *
 */
union cvmx_rnm_eer_key {
	uint64_t u64;
	struct cvmx_rnm_eer_key_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t key                          : 64; /**< Dormant encryption key. If dormant crypto is fuse-enabled, crypto can be enabled by
                                                         writing this register with the correct key. */
#else
	uint64_t key                          : 64;
#endif
	} s;
	struct cvmx_rnm_eer_key_s             cn61xx;
	struct cvmx_rnm_eer_key_s             cn63xx;
	struct cvmx_rnm_eer_key_s             cn63xxp1;
	struct cvmx_rnm_eer_key_s             cn66xx;
	struct cvmx_rnm_eer_key_s             cn68xx;
	struct cvmx_rnm_eer_key_s             cn68xxp1;
	struct cvmx_rnm_eer_key_s             cn70xx;
	struct cvmx_rnm_eer_key_s             cn70xxp1;
	struct cvmx_rnm_eer_key_s             cn73xx;
	struct cvmx_rnm_eer_key_s             cn78xx;
	struct cvmx_rnm_eer_key_s             cn78xxp1;
	struct cvmx_rnm_eer_key_s             cnf71xx;
	struct cvmx_rnm_eer_key_s             cnf75xx;
};
typedef union cvmx_rnm_eer_key cvmx_rnm_eer_key_t;

/**
 * cvmx_rnm_serial_num
 *
 * RNM_SERIAL_NUM = RNM's fuse serial number register
 *
 * The RNM's fuse serial number register
 *
 * Notes:
 * Added RNM_SERIAL_NUM in pass 2.0
 *
 */
union cvmx_rnm_serial_num {
	uint64_t u64;
	struct cvmx_rnm_serial_num_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dat                          : 64; /**< Dormant encryption serial number. */
#else
	uint64_t dat                          : 64;
#endif
	} s;
	struct cvmx_rnm_serial_num_s          cn61xx;
	struct cvmx_rnm_serial_num_s          cn63xx;
	struct cvmx_rnm_serial_num_s          cn66xx;
	struct cvmx_rnm_serial_num_s          cn68xx;
	struct cvmx_rnm_serial_num_s          cn68xxp1;
	struct cvmx_rnm_serial_num_s          cn70xx;
	struct cvmx_rnm_serial_num_s          cn70xxp1;
	struct cvmx_rnm_serial_num_s          cn73xx;
	struct cvmx_rnm_serial_num_s          cn78xx;
	struct cvmx_rnm_serial_num_s          cn78xxp1;
	struct cvmx_rnm_serial_num_s          cnf71xx;
	struct cvmx_rnm_serial_num_s          cnf75xx;
};
typedef union cvmx_rnm_serial_num cvmx_rnm_serial_num_t;

#endif
