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
 * cvmx-iobn-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon iobn.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_IOBN_DEFS_H__
#define __CVMX_IOBN_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOBN_BIST_STATUS CVMX_IOBN_BIST_STATUS_FUNC()
static inline uint64_t CVMX_IOBN_BIST_STATUS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_IOBN_BIST_STATUS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0000018ull);
}
#else
#define CVMX_IOBN_BIST_STATUS (CVMX_ADD_IO_SEG(0x00011800F0000018ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOBN_CHIP_CUR_PWR CVMX_IOBN_CHIP_CUR_PWR_FUNC()
static inline uint64_t CVMX_IOBN_CHIP_CUR_PWR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_IOBN_CHIP_CUR_PWR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0000068ull);
}
#else
#define CVMX_IOBN_CHIP_CUR_PWR (CVMX_ADD_IO_SEG(0x00011800F0000068ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOBN_CHIP_GLB_PWR_THROTTLE CVMX_IOBN_CHIP_GLB_PWR_THROTTLE_FUNC()
static inline uint64_t CVMX_IOBN_CHIP_GLB_PWR_THROTTLE_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_IOBN_CHIP_GLB_PWR_THROTTLE not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0000038ull);
}
#else
#define CVMX_IOBN_CHIP_GLB_PWR_THROTTLE (CVMX_ADD_IO_SEG(0x00011800F0000038ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOBN_CHIP_PWR_OUT CVMX_IOBN_CHIP_PWR_OUT_FUNC()
static inline uint64_t CVMX_IOBN_CHIP_PWR_OUT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_IOBN_CHIP_PWR_OUT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0000048ull);
}
#else
#define CVMX_IOBN_CHIP_PWR_OUT (CVMX_ADD_IO_SEG(0x00011800F0000048ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOBN_CONTROL CVMX_IOBN_CONTROL_FUNC()
static inline uint64_t CVMX_IOBN_CONTROL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_IOBN_CONTROL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0000008ull);
}
#else
#define CVMX_IOBN_CONTROL (CVMX_ADD_IO_SEG(0x00011800F0000008ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOBN_CREDITS CVMX_IOBN_CREDITS_FUNC()
static inline uint64_t CVMX_IOBN_CREDITS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_IOBN_CREDITS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0000028ull);
}
#else
#define CVMX_IOBN_CREDITS (CVMX_ADD_IO_SEG(0x00011800F0000028ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOBN_ECC CVMX_IOBN_ECC_FUNC()
static inline uint64_t CVMX_IOBN_ECC_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_IOBN_ECC not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0000010ull);
}
#else
#define CVMX_IOBN_ECC (CVMX_ADD_IO_SEG(0x00011800F0000010ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOBN_GBL_DLL CVMX_IOBN_GBL_DLL_FUNC()
static inline uint64_t CVMX_IOBN_GBL_DLL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_IOBN_GBL_DLL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0001000ull);
}
#else
#define CVMX_IOBN_GBL_DLL (CVMX_ADD_IO_SEG(0x00011800F0001000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOBN_HIGH_PRIORITY CVMX_IOBN_HIGH_PRIORITY_FUNC()
static inline uint64_t CVMX_IOBN_HIGH_PRIORITY_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_IOBN_HIGH_PRIORITY not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0000000ull);
}
#else
#define CVMX_IOBN_HIGH_PRIORITY (CVMX_ADD_IO_SEG(0x00011800F0000000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOBN_INT_SUM CVMX_IOBN_INT_SUM_FUNC()
static inline uint64_t CVMX_IOBN_INT_SUM_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_IOBN_INT_SUM not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0000020ull);
}
#else
#define CVMX_IOBN_INT_SUM (CVMX_ADD_IO_SEG(0x00011800F0000020ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_IOBN_NCBX_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_IOBN_NCBX_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800F0004000ull) + ((offset) & 3) * 8;
}
#else
#define CVMX_IOBN_NCBX_CTL(offset) (CVMX_ADD_IO_SEG(0x00011800F0004000ull) + ((offset) & 3) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOBN_PP_BIST_STATUS CVMX_IOBN_PP_BIST_STATUS_FUNC()
static inline uint64_t CVMX_IOBN_PP_BIST_STATUS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_IOBN_PP_BIST_STATUS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0000700ull);
}
#else
#define CVMX_IOBN_PP_BIST_STATUS (CVMX_ADD_IO_SEG(0x00011800F0000700ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_IOBN_ROC_DLL CVMX_IOBN_ROC_DLL_FUNC()
static inline uint64_t CVMX_IOBN_ROC_DLL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_IOBN_ROC_DLL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800F0001010ull);
}
#else
#define CVMX_IOBN_ROC_DLL (CVMX_ADD_IO_SEG(0x00011800F0001010ull))
#endif

/**
 * cvmx_iobn_bist_status
 *
 * This register contains the result of the BIST run on the IOB memories.
 *
 */
union cvmx_iobn_bist_status {
	uint64_t u64;
	struct cvmx_iobn_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_27_63               : 37;
	uint64_t rsdm3                        : 1;  /**< rsd_mem3_bstatus */
	uint64_t rsdm2                        : 1;  /**< rsd_mem2_bstatus */
	uint64_t rsdm1                        : 1;  /**< rsd_mem1_bstatus */
	uint64_t rsdm0                        : 1;  /**< rsd_mem0_bstatus */
	uint64_t iocf3                        : 1;  /**< iocfif3_bstatus */
	uint64_t iocf2                        : 1;  /**< iocfif2_bstatus */
	uint64_t iocf1                        : 1;  /**< iocfif1_bstatus */
	uint64_t iocf0                        : 1;  /**< iocfif0_bstatus */
	uint64_t iorf0                        : 1;  /**< iorfif0_bstatus */
	uint64_t iorf1                        : 1;  /**< iorfif1_bstatus */
	uint64_t iorf2                        : 1;  /**< iorfif2_bstatus */
	uint64_t iorf3                        : 1;  /**< iorfif3_bstatus */
	uint64_t xmcf                         : 1;  /**< xmcfif_bstatus */
	uint64_t ichpx                        : 1;  /**< icc_xmc_fifo_ecc_bstatus */
	uint64_t ide                          : 1;  /**< ide_ecc_bstatus */
	uint64_t icx3                         : 1;  /**< icc0_xmc_fifo_ecc_bstatus */
	uint64_t icx2                         : 1;  /**< icc0_xmc_fifo_ecc_bstatus */
	uint64_t icx1                         : 1;  /**< icc0_xmc_fifo_ecc_bstatus */
	uint64_t icx0                         : 1;  /**< icc0_xmc_fifo_ecc_bstatus */
	uint64_t immx3                        : 1;  /**< icm_mem_mask_xmd3_bstatus */
	uint64_t imdx3                        : 1;  /**< icm_mem_data_xmd3_bstatus */
	uint64_t immx2                        : 1;  /**< icm_mem_mask_xmd2_bstatus */
	uint64_t imdx2                        : 1;  /**< icm_mem_data_xmd2_bstatus */
	uint64_t immx1                        : 1;  /**< icm_mem_mask_xmd1_bstatus */
	uint64_t imdx1                        : 1;  /**< icm_mem_data_xmd1_bstatus */
	uint64_t immx0                        : 1;  /**< icm_mem_mask_xmd0_bstatus */
	uint64_t imdx0                        : 1;  /**< icm_mem_data_xmd0_bstatus */
#else
	uint64_t imdx0                        : 1;
	uint64_t immx0                        : 1;
	uint64_t imdx1                        : 1;
	uint64_t immx1                        : 1;
	uint64_t imdx2                        : 1;
	uint64_t immx2                        : 1;
	uint64_t imdx3                        : 1;
	uint64_t immx3                        : 1;
	uint64_t icx0                         : 1;
	uint64_t icx1                         : 1;
	uint64_t icx2                         : 1;
	uint64_t icx3                         : 1;
	uint64_t ide                          : 1;
	uint64_t ichpx                        : 1;
	uint64_t xmcf                         : 1;
	uint64_t iorf3                        : 1;
	uint64_t iorf2                        : 1;
	uint64_t iorf1                        : 1;
	uint64_t iorf0                        : 1;
	uint64_t iocf0                        : 1;
	uint64_t iocf1                        : 1;
	uint64_t iocf2                        : 1;
	uint64_t iocf3                        : 1;
	uint64_t rsdm0                        : 1;
	uint64_t rsdm1                        : 1;
	uint64_t rsdm2                        : 1;
	uint64_t rsdm3                        : 1;
	uint64_t reserved_27_63               : 37;
#endif
	} s;
	struct cvmx_iobn_bist_status_s        cn73xx;
	struct cvmx_iobn_bist_status_s        cn78xx;
	struct cvmx_iobn_bist_status_s        cn78xxp1;
	struct cvmx_iobn_bist_status_s        cnf75xx;
};
typedef union cvmx_iobn_bist_status cvmx_iobn_bist_status_t;

/**
 * cvmx_iobn_chip_cur_pwr
 *
 * This register contains the current power setting.
 *
 */
union cvmx_iobn_chip_cur_pwr {
	uint64_t u64;
	struct cvmx_iobn_chip_cur_pwr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t current_power_setting        : 8;  /**< Global throttling value currently being used. Throttling can force units (CPU cores, in
                                                         particular) idle for a portion of time, which will reduce power consumption. When
                                                         [CURRENT_POWER_SETTING] is equal to zero, the unit is idle most of the time and consumes
                                                         minimum power. When [CURRENT_POWER_SETTING] is equal to 0xFF, units are never idled to
                                                         reduce power. The hardware generally uses a [CURRENT_POWER_SETTING] value that is as large
                                                         as possible (in order to maximize performance) subject to the following constraints (in
                                                         priority order):
                                                         * PWR_MIN <= [CURRENT_POWER_SETTING] <= PWR_MAX.
                                                         * Power limits from the PWR_SETTING feedback control system.
                                                         In the case of the CPU cores, [CURRENT_POWER_SETTING] effectively limits the CP0
                                                         PowThrottle[POWLIM] value: effective POWLIM = MINIMUM([CURRENT_POWER_SETTING],
                                                         PowThrottle[POWLIM]) */
#else
	uint64_t current_power_setting        : 8;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_iobn_chip_cur_pwr_s       cn73xx;
	struct cvmx_iobn_chip_cur_pwr_s       cn78xx;
	struct cvmx_iobn_chip_cur_pwr_s       cn78xxp1;
	struct cvmx_iobn_chip_cur_pwr_s       cnf75xx;
};
typedef union cvmx_iobn_chip_cur_pwr cvmx_iobn_chip_cur_pwr_t;

/**
 * cvmx_iobn_chip_glb_pwr_throttle
 *
 * This register controls the min/max power settings.
 *
 */
union cvmx_iobn_chip_glb_pwr_throttle {
	uint64_t u64;
	struct cvmx_iobn_chip_glb_pwr_throttle_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_34_63               : 30;
	uint64_t pwr_bw                       : 2;  /**< Configures the reaction time of the closed-loop feedback control system for the
                                                         AVG_CHIP_POWER power approximation. Higher numbers decrease bandwidth, reducing response
                                                         time, which could lead to greater tracking error, but reduce ringing. */
	uint64_t pwr_max                      : 8;  /**< Reserved. */
	uint64_t pwr_min                      : 8;  /**< Reserved. */
	uint64_t pwr_setting                  : 16; /**< A power limiter for the chip. A limiter of the power consumption of the
                                                         chip. This power limiting is implemented by a closed-loop feedback control
                                                         system for the AVG_CHIP_POWER power approximation. The direct output of the
                                                         [PWR_SETTING] feedback control system is the CURRENT_POWER_SETTING value. The
                                                         power consumed by the chip (estimated currently by the AVG_CHIP_POWER value) is
                                                         an indirect output of the PWR_SETTING feedback control system. [PWR_SETTING] is
                                                         not used by the hardware when [PWR_MIN] equals [PWR_MAX]. [PWR_MIN] and
                                                         [PWR_MAX] threshold requirements always supersede [PWR_SETTING] limits. (For
                                                         maximum [PWR_SETTING] feedback control freedom, set [PWR_MIN]=0 and
                                                         [PWR_MAX]=0xff.)
                                                         [PWR_SETTING] equal to 0 forces the chip to consume near minimum
                                                         power. Increasing [PWR_SETTING] value from 0 to 0xFFFF increases the power that
                                                         the chip is allowed to consume linearly (roughly) from minimum to maximum. */
#else
	uint64_t pwr_setting                  : 16;
	uint64_t pwr_min                      : 8;
	uint64_t pwr_max                      : 8;
	uint64_t pwr_bw                       : 2;
	uint64_t reserved_34_63               : 30;
#endif
	} s;
	struct cvmx_iobn_chip_glb_pwr_throttle_s cn73xx;
	struct cvmx_iobn_chip_glb_pwr_throttle_s cn78xx;
	struct cvmx_iobn_chip_glb_pwr_throttle_s cn78xxp1;
	struct cvmx_iobn_chip_glb_pwr_throttle_s cnf75xx;
};
typedef union cvmx_iobn_chip_glb_pwr_throttle cvmx_iobn_chip_glb_pwr_throttle_t;

/**
 * cvmx_iobn_chip_pwr_out
 *
 * This register contains power numbers from the various partitions on the chip.
 *
 */
union cvmx_iobn_chip_pwr_out {
	uint64_t u64;
	struct cvmx_iobn_chip_pwr_out_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t cpu_pwr                      : 16; /**< An estimate of the current CPU core complex power consumption. The CPU core complex
                                                         includes the caches and DRAM controller(s), as well as all CPU cores. Linearly larger
                                                         values indicate linearly higher power consumption. This power consumption estimate is
                                                         energy per core clock. */
	uint64_t chip_power                   : 16; /**< An estimate of the current total power consumption by the chip. Linearly larger values
                                                         indicate linearly higher power consumption. [CHIP_POWER] is the sum of [CPU_PWR] and
                                                         [COPROC_POWER]. */
	uint64_t coproc_power                 : 16; /**< An estimate of the current coprocessor power consumption. Linearly larger values indicate
                                                         linearly higher power consumption. This estimate is energy per core clock, and will
                                                         generally decrease as the ratio of core to coprocessor clock speed increases. */
	uint64_t avg_chip_power               : 16; /**< An average of [CHIP_POWER], calculated using an IIR filter with an average weight of 16K
                                                         core clocks. */
#else
	uint64_t avg_chip_power               : 16;
	uint64_t coproc_power                 : 16;
	uint64_t chip_power                   : 16;
	uint64_t cpu_pwr                      : 16;
#endif
	} s;
	struct cvmx_iobn_chip_pwr_out_s       cn73xx;
	struct cvmx_iobn_chip_pwr_out_s       cn78xx;
	struct cvmx_iobn_chip_pwr_out_s       cn78xxp1;
	struct cvmx_iobn_chip_pwr_out_s       cnf75xx;
};
typedef union cvmx_iobn_chip_pwr_out cvmx_iobn_chip_pwr_out_t;

/**
 * cvmx_iobn_control
 *
 * This register contains various control bits for IOBN functionality.
 *
 */
union cvmx_iobn_control {
	uint64_t u64;
	struct cvmx_iobn_control_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t dwb_enb                      : 1;  /**< Don't-write-back enable. When cleared to 0, the IOBN does not do any don't-write-back operations. */
#else
	uint64_t dwb_enb                      : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_iobn_control_s            cn73xx;
	struct cvmx_iobn_control_s            cn78xx;
	struct cvmx_iobn_control_s            cn78xxp1;
	struct cvmx_iobn_control_s            cnf75xx;
};
typedef union cvmx_iobn_control cvmx_iobn_control_t;

/**
 * cvmx_iobn_credits
 *
 * This register controls the number of loads and stores each NCB can have to the L2.
 *
 */
union cvmx_iobn_credits {
	uint64_t u64;
	struct cvmx_iobn_credits_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_62_63               : 2;
	uint64_t ncb3_wr_crd                  : 6;  /**< NCB3 write credit. Each NCB can have 32 writes in flight to the L2; this is the
                                                         number by which to decrease the 32. */
	uint64_t reserved_54_55               : 2;
	uint64_t ncb3_rd_crd                  : 6;  /**< NCB3 read credit. Each NCB can have 32 reads in flight to the L2; this is the
                                                         number by which to decrease the 32. */
	uint64_t reserved_46_47               : 2;
	uint64_t ncb2_wr_crd                  : 6;  /**< NCB2 write credit. Each NCB can have 32 writes in flight to the L2; this is the
                                                         number by which to decrease the 32. */
	uint64_t reserved_38_39               : 2;
	uint64_t ncb2_rd_crd                  : 6;  /**< NCB2 read credit. Each NCB can have 32 reads in flight to the L2; this is the
                                                         number by which to decrease the 32. */
	uint64_t reserved_30_31               : 2;
	uint64_t ncb1_wr_crd                  : 6;  /**< NCB1 write credit. Each NCB can have 32 writes in flight to the L2; this is the
                                                         number by which to decrease the 32. */
	uint64_t reserved_22_23               : 2;
	uint64_t ncb1_rd_crd                  : 6;  /**< NCB1 read credit. Each NCB can have 32 reads in flight to the L2; this is the
                                                         number by which to decrease the 32. */
	uint64_t reserved_14_15               : 2;
	uint64_t ncb0_wr_crd                  : 6;  /**< NCB0 write credit. Each NCB can have 32 writes in flight to the L2; this is the
                                                         number by which to decrease the 32. */
	uint64_t reserved_6_7                 : 2;
	uint64_t ncb0_rd_crd                  : 6;  /**< NCB0 read credit. Each NCB can have 32 reads in flight to the L2; this is the
                                                         number by which to decrease the 32. */
#else
	uint64_t ncb0_rd_crd                  : 6;
	uint64_t reserved_6_7                 : 2;
	uint64_t ncb0_wr_crd                  : 6;
	uint64_t reserved_14_15               : 2;
	uint64_t ncb1_rd_crd                  : 6;
	uint64_t reserved_22_23               : 2;
	uint64_t ncb1_wr_crd                  : 6;
	uint64_t reserved_30_31               : 2;
	uint64_t ncb2_rd_crd                  : 6;
	uint64_t reserved_38_39               : 2;
	uint64_t ncb2_wr_crd                  : 6;
	uint64_t reserved_46_47               : 2;
	uint64_t ncb3_rd_crd                  : 6;
	uint64_t reserved_54_55               : 2;
	uint64_t ncb3_wr_crd                  : 6;
	uint64_t reserved_62_63               : 2;
#endif
	} s;
	struct cvmx_iobn_credits_s            cn73xx;
	struct cvmx_iobn_credits_s            cn78xx;
	struct cvmx_iobn_credits_s            cn78xxp1;
	struct cvmx_iobn_credits_s            cnf75xx;
};
typedef union cvmx_iobn_credits cvmx_iobn_credits_t;

/**
 * cvmx_iobn_ecc
 *
 * This register contains various control bits for IOBN ECC functionality.
 *
 */
union cvmx_iobn_ecc {
	uint64_t u64;
	struct cvmx_iobn_ecc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_58_63               : 6;
	uint64_t ioc3_ecc                     : 1;  /**< When set, disable core-to-NCB transfer ECC correction. */
	uint64_t ioc3_fs                      : 2;  /**< Used to flip the syndrome for core-to-NCB transfers. */
	uint64_t ioc2_ecc                     : 1;  /**< When set, disable core-to-NCB transfer ECC correction. */
	uint64_t ioc2_fs                      : 2;  /**< Used to flip the syndrome for core-to-NCB transfers. */
	uint64_t ioc1_ecc                     : 1;  /**< When set, disable core-to-NCB transfer ECC correction. */
	uint64_t ioc1_fs                      : 2;  /**< Used to flip the syndrome for core-to-NCB transfers. */
	uint64_t ioc0_ecc                     : 1;  /**< When set, disable core-to-NCB transfer ECC correction. */
	uint64_t ioc0_fs                      : 2;  /**< Used to flip the syndrome for core-to-NCB transfers. */
	uint64_t ior3_ecc                     : 1;  /**< When set, disable FILL data from NCB3 ECC correction. */
	uint64_t ior3_fs                      : 2;  /**< Used to flip the syndrome for FILL data from NCB3. */
	uint64_t ior2_ecc                     : 1;  /**< When set, disable FILL data from NCB2 ECC correction. */
	uint64_t ior2_fs                      : 2;  /**< Used to flip the syndrome for FILL data from NCB2. */
	uint64_t ior1_ecc                     : 1;  /**< When set, disable FILL data from NCB1 ECC correction. */
	uint64_t ior1_fs                      : 2;  /**< Used to flip the syndrome for FILL data from NCB1. */
	uint64_t ior0_ecc                     : 1;  /**< When set, disable FILL data from NCB0 ECC correction. */
	uint64_t ior0_fs                      : 2;  /**< Used to flip the syndrome for FILL data from NCB0. */
	uint64_t ide_ecc                      : 1;  /**< When set, disable DWB command to L2C ECC correction. */
	uint64_t ide_fs                       : 2;  /**< Used to flip the syndrome for DWB commands to the L2C. */
	uint64_t xmc0_hp_ecc                  : 1;  /**< When set, disable NCBI0 high-priority command to L2C ECC correction. */
	uint64_t xmc0_hp_fs                   : 2;  /**< Used to flip the syndrome for high-priority commands from NCBI0 to L2C. */
	uint64_t rsd3_ecc                     : 1;  /**< When set, disable NCBO3 response data ECC correction. */
	uint64_t rsd3_fs                      : 2;  /**< Used to flip the syndrome for NCBO3 response data. */
	uint64_t rsd2_ecc                     : 1;  /**< When set, disable NCBO2 response data ECC correction. */
	uint64_t rsd2_fs                      : 2;  /**< Used to flip the syndrome for NCBO2 response data. */
	uint64_t rsd1_ecc                     : 1;  /**< When set, disable NCBO1 response data ECC correction. */
	uint64_t rsd1_fs                      : 2;  /**< Used to flip the syndrome for NCBO1 response data. */
	uint64_t rsd0_ecc                     : 1;  /**< When set, disable NCBO0 response data ECC correction. */
	uint64_t rsd0_fs                      : 2;  /**< Used to flip the syndrome for NCBO0 response data. */
	uint64_t xmc3_ecc                     : 1;  /**< When set, disable NCBI3 to L2C ECC correction. */
	uint64_t xmc3_fs                      : 2;  /**< Used to flip the syndrome for commands from NCBI0 to the L2C. */
	uint64_t xmc2_ecc                     : 1;  /**< When set, disable NCBI2 to L2C ECC correction. */
	uint64_t xmc2_fs                      : 2;  /**< Used to flip the syndrome for commands from NCBI0 to the L2C. */
	uint64_t xmc1_ecc                     : 1;  /**< When set, disable NCBI1 command L2C ECC correction. */
	uint64_t xmc1_fs                      : 2;  /**< Used to flip the syndrome for commands from NCBI0 to the L2C. */
	uint64_t xmc0_ecc                     : 1;  /**< When set, disable NCBI0 command L2C ECC correction. */
	uint64_t xmc0_fs                      : 2;  /**< Used to flip the syndrome for commands from NCBI0 to the L2C. */
	uint64_t xmd3_ecc                     : 1;  /**< When set, NCBI3 data to L2C has ECC generated and checked. */
	uint64_t xmd2_ecc                     : 1;  /**< When set, NCBI2 data to L2C has ECC generated and checked. */
	uint64_t xmd1_ecc                     : 1;  /**< When set, NCBI1 data to L2C has ECC generated and checked. */
	uint64_t xmd0_ecc                     : 1;  /**< When set, NCBI0 data to L2C has ECC generated and checked. */
#else
	uint64_t xmd0_ecc                     : 1;
	uint64_t xmd1_ecc                     : 1;
	uint64_t xmd2_ecc                     : 1;
	uint64_t xmd3_ecc                     : 1;
	uint64_t xmc0_fs                      : 2;
	uint64_t xmc0_ecc                     : 1;
	uint64_t xmc1_fs                      : 2;
	uint64_t xmc1_ecc                     : 1;
	uint64_t xmc2_fs                      : 2;
	uint64_t xmc2_ecc                     : 1;
	uint64_t xmc3_fs                      : 2;
	uint64_t xmc3_ecc                     : 1;
	uint64_t rsd0_fs                      : 2;
	uint64_t rsd0_ecc                     : 1;
	uint64_t rsd1_fs                      : 2;
	uint64_t rsd1_ecc                     : 1;
	uint64_t rsd2_fs                      : 2;
	uint64_t rsd2_ecc                     : 1;
	uint64_t rsd3_fs                      : 2;
	uint64_t rsd3_ecc                     : 1;
	uint64_t xmc0_hp_fs                   : 2;
	uint64_t xmc0_hp_ecc                  : 1;
	uint64_t ide_fs                       : 2;
	uint64_t ide_ecc                      : 1;
	uint64_t ior0_fs                      : 2;
	uint64_t ior0_ecc                     : 1;
	uint64_t ior1_fs                      : 2;
	uint64_t ior1_ecc                     : 1;
	uint64_t ior2_fs                      : 2;
	uint64_t ior2_ecc                     : 1;
	uint64_t ior3_fs                      : 2;
	uint64_t ior3_ecc                     : 1;
	uint64_t ioc0_fs                      : 2;
	uint64_t ioc0_ecc                     : 1;
	uint64_t ioc1_fs                      : 2;
	uint64_t ioc1_ecc                     : 1;
	uint64_t ioc2_fs                      : 2;
	uint64_t ioc2_ecc                     : 1;
	uint64_t ioc3_fs                      : 2;
	uint64_t ioc3_ecc                     : 1;
	uint64_t reserved_58_63               : 6;
#endif
	} s;
	struct cvmx_iobn_ecc_s                cn73xx;
	struct cvmx_iobn_ecc_s                cn78xx;
	struct cvmx_iobn_ecc_s                cn78xxp1;
	struct cvmx_iobn_ecc_s                cnf75xx;
};
typedef union cvmx_iobn_ecc cvmx_iobn_ecc_t;

/**
 * cvmx_iobn_gbl_dll
 *
 * Status of the global core-clock DLL.
 *
 */
union cvmx_iobn_gbl_dll {
	uint64_t u64;
	struct cvmx_iobn_gbl_dll_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t pdr_rclk_refclk              : 1;  /**< Synchronized pdr_rclk_refclk from global core-clock DLL cmb0 phase detectors. */
	uint64_t pdl_rclk_refclk              : 1;  /**< Synchronized pdl_rclk_refclk from global core-clock DLL cmb0 phase detectors. */
	uint64_t pd_pos_rclk_refclk           : 1;  /**< Synchronized pd_pos_rclk_refclk from global core-clock DLL cmb0 phase detectors. */
	uint64_t dll_fsm_state_a              : 3;  /**< State for the global core-clock DLL, from the positive edge of refclk.
                                                         0x0 = TMD_IDLE.
                                                         0x1 = TMD_STATE1.
                                                         0x2 = TMD_STATE2.
                                                         0x3 = TMD_STATE3.
                                                         0x4 = TMD_STATE4.
                                                         0x5 = TMD_LOCKED. */
	uint64_t dll_lock                     : 1;  /**< The dll_lock signal from global core-clock DLL, from the positive edge of refclk. */
	uint64_t dll_clk_invert_out           : 1;  /**< The clk_invert setting from the global core-clock DLL, from the negative edge of refclk. */
	uint64_t dll_setting                  : 12; /**< The global core-clock DLL, from the negative edge of refclk. */
#else
	uint64_t dll_setting                  : 12;
	uint64_t dll_clk_invert_out           : 1;
	uint64_t dll_lock                     : 1;
	uint64_t dll_fsm_state_a              : 3;
	uint64_t pd_pos_rclk_refclk           : 1;
	uint64_t pdl_rclk_refclk              : 1;
	uint64_t pdr_rclk_refclk              : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_iobn_gbl_dll_s            cn73xx;
	struct cvmx_iobn_gbl_dll_s            cn78xx;
	struct cvmx_iobn_gbl_dll_s            cn78xxp1;
	struct cvmx_iobn_gbl_dll_s            cnf75xx;
};
typedef union cvmx_iobn_gbl_dll cvmx_iobn_gbl_dll_t;

/**
 * cvmx_iobn_high_priority
 *
 * For NCB0, this register sets which of the NCB devices (0-15) receive high-priority status for
 * access to L2C.
 */
union cvmx_iobn_high_priority {
	uint64_t u64;
	struct cvmx_iobn_high_priority_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t sso_hp                       : 1;  /**< SSO high priority. When this bit is set, the SSO receives high priority status, causing
                                                         the IOBN to make best effort, after servicing FPA when FPA_HP is set, to complete the SSO
                                                         L2C request first. */
	uint64_t fpa_hp                       : 1;  /**< FPA high priority. When this bit is set, the FPA receives high priority status, causing
                                                         the IOBN to make best effort to complete the FPA L2C request first. */
#else
	uint64_t fpa_hp                       : 1;
	uint64_t sso_hp                       : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_iobn_high_priority_s      cn73xx;
	struct cvmx_iobn_high_priority_s      cn78xx;
	struct cvmx_iobn_high_priority_s      cn78xxp1;
	struct cvmx_iobn_high_priority_s      cnf75xx;
};
typedef union cvmx_iobn_high_priority cvmx_iobn_high_priority_t;

/**
 * cvmx_iobn_int_sum
 *
 * This is the IOBN interrupt summary register.
 *
 */
union cvmx_iobn_int_sum {
	uint64_t u64;
	struct cvmx_iobn_int_sum_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_52_63               : 12;
	uint64_t icc3_dbe                     : 1;  /**< Double-bit error for ICC MEM3.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_ICC3_DBE. */
	uint64_t icc3_sbe                     : 1;  /**< Single-bit error for ICC MEM3.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_ICC3_SBE. */
	uint64_t icc2_dbe                     : 1;  /**< Double-bit error for ICC MEM2.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_ICC2_DBE. */
	uint64_t icc2_sbe                     : 1;  /**< Single-bit error for ICC MEM2.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_ICC2_SBE. */
	uint64_t icc1_dbe                     : 1;  /**< Double-bit error for ICC MEM1.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_ICC1_DBE. */
	uint64_t icc1_sbe                     : 1;  /**< Single-bit error for ICC MEM1.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_ICC1_SBE. */
	uint64_t icc0_dbe                     : 1;  /**< Double-bit error for ICC MEM0.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_ICC0_DBE. */
	uint64_t icc0_sbe                     : 1;  /**< Single-bit error for ICC MEM0.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_ICC0_SBE. */
	uint64_t ide_dbe                      : 1;  /**< Double-bit error for IDE MEM.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_IDE_DBE. */
	uint64_t ide_sbe                      : 1;  /**< Single-bit error for IDE MEM.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_IDE_SBE. */
	uint64_t xmc_dbe                      : 1;  /**< Double-bit error for XMC MEM.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_XMC_DBE. */
	uint64_t xmc_sbe                      : 1;  /**< Single-bit error for XMC MEM.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_XMC_SBE. */
	uint64_t xmdb3_dbe                    : 1;  /**< Double-bit error for XMDB MEM3.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_XMDB3_DBE. */
	uint64_t xmdb3_sbe                    : 1;  /**< Single-bit error for XMDB MEM3.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_XMDB3_SBE. */
	uint64_t xmdb2_dbe                    : 1;  /**< Double-bit error for XMDB MEM2.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_XMDB2_DBE. */
	uint64_t xmdb2_sbe                    : 1;  /**< Single-bit error for XMDB MEM2.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_XMDB2_SBE. */
	uint64_t xmdb1_dbe                    : 1;  /**< Double-bit error for XMDB MEM1.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_XMDB1_DBE. */
	uint64_t xmdb1_sbe                    : 1;  /**< Single-bit error for XMDB MEM1.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_XMDB1_SBE. */
	uint64_t xmdb0_dbe                    : 1;  /**< Double-bit error for XMDB MEM0.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_XMDB0_DBE. */
	uint64_t xmdb0_sbe                    : 1;  /**< Single-bit error for XMDB MEM0.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_XMDB0_SBE. */
	uint64_t xmda3_dbe                    : 1;  /**< Double-bit error for XMDA MEM3.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_XMDA3_DBE. */
	uint64_t xmda3_sbe                    : 1;  /**< Single-bit error for XMDA MEM3.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_XMDA3_SBE. */
	uint64_t xmda2_dbe                    : 1;  /**< Double-bit error for XMDA MEM2.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_XMDA2_DBE. */
	uint64_t xmda2_sbe                    : 1;  /**< Single-bit error for XMDA MEM2.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_XMDA2_SBE. */
	uint64_t xmda1_dbe                    : 1;  /**< Double-bit error for XMDA MEM1.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_XMDA1_DBE. */
	uint64_t xmda1_sbe                    : 1;  /**< Single-bit error for XMDA MEM1.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_XMDA1_SBE. */
	uint64_t xmda0_dbe                    : 1;  /**< Double-bit error for XMDA MEM0.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_XMDA0_DBE. */
	uint64_t xmda0_sbe                    : 1;  /**< Single-bit error for XMDA MEM0.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_XMDA0_SBE. */
	uint64_t ioc3_dbe                     : 1;  /**< Double-bit error for IOC3 MEM.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_IOC3_DBE. */
	uint64_t ioc3_sbe                     : 1;  /**< Single-bit error for IOC3 MEM.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_IOC3_SBE. */
	uint64_t ioc2_dbe                     : 1;  /**< Double-bit error for IOC2 MEM.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_IOC2_DBE. */
	uint64_t ioc2_sbe                     : 1;  /**< Single-bit error for IOC2 MEM.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_IOC2_SBE. */
	uint64_t ioc1_dbe                     : 1;  /**< Double-bit error for IOC1 MEM.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_IOC1_DBE. */
	uint64_t ioc1_sbe                     : 1;  /**< Single-bit error for IOC1 MEM.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_IOC1_SBE. */
	uint64_t ioc0_dbe                     : 1;  /**< Double-bit error for IOC0 MEM.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_IOC0_DBE. */
	uint64_t ioc0_sbe                     : 1;  /**< Single-bit error for IOC0 MEM.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_IOC0_SBE. */
	uint64_t ior3_dbe                     : 1;  /**< Double-bit error for IOR MEM3.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_IOR3_DBE. */
	uint64_t ior3_sbe                     : 1;  /**< Single-bit error for IOR MEM3.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_IOR3_SBE. */
	uint64_t ior2_dbe                     : 1;  /**< Double-bit error for IOR MEM2.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_IOR2_DBE. */
	uint64_t ior2_sbe                     : 1;  /**< Single-bit error for IOR MEM2.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_IOR2_SBE. */
	uint64_t ior1_dbe                     : 1;  /**< Double-bit error for IOR MEM1.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_IOR1_DBE. */
	uint64_t ior1_sbe                     : 1;  /**< Single-bit error for IOR MEM1.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_IOR1_SBE. */
	uint64_t ior0_dbe                     : 1;  /**< Double-bit error for IOR MEM0.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_IOR0_DBE. */
	uint64_t ior0_sbe                     : 1;  /**< Single-bit error for IOR MEM0.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_IOR0_SBE. */
	uint64_t rsd3_dbe                     : 1;  /**< Double-bit error for RSD MEM3.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_RSD3_DBE. */
	uint64_t rsd3_sbe                     : 1;  /**< Single-bit error for RSD MEM3.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_RSD3_SBE. */
	uint64_t rsd2_dbe                     : 1;  /**< Double-bit error for RSD MEM2.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_RSD2_DBE. */
	uint64_t rsd2_sbe                     : 1;  /**< Single-bit error for RSD MEM2.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_RSD2_SBE. */
	uint64_t rsd1_dbe                     : 1;  /**< Double-bit error for RSD MEM1.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_RSD1_DBE. */
	uint64_t rsd1_sbe                     : 1;  /**< Single-bit error for RSD MEM1.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_RSD1_SBE. */
	uint64_t rsd0_dbe                     : 1;  /**< Double-bit error for RSD MEM0.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_RSD0_DBE. */
	uint64_t rsd0_sbe                     : 1;  /**< Single-bit error for RSD MEM0.
                                                         Throws IOBN_INTSN_E::IOBN_INT_SUM_RSD0_SBE. */
#else
	uint64_t rsd0_sbe                     : 1;
	uint64_t rsd0_dbe                     : 1;
	uint64_t rsd1_sbe                     : 1;
	uint64_t rsd1_dbe                     : 1;
	uint64_t rsd2_sbe                     : 1;
	uint64_t rsd2_dbe                     : 1;
	uint64_t rsd3_sbe                     : 1;
	uint64_t rsd3_dbe                     : 1;
	uint64_t ior0_sbe                     : 1;
	uint64_t ior0_dbe                     : 1;
	uint64_t ior1_sbe                     : 1;
	uint64_t ior1_dbe                     : 1;
	uint64_t ior2_sbe                     : 1;
	uint64_t ior2_dbe                     : 1;
	uint64_t ior3_sbe                     : 1;
	uint64_t ior3_dbe                     : 1;
	uint64_t ioc0_sbe                     : 1;
	uint64_t ioc0_dbe                     : 1;
	uint64_t ioc1_sbe                     : 1;
	uint64_t ioc1_dbe                     : 1;
	uint64_t ioc2_sbe                     : 1;
	uint64_t ioc2_dbe                     : 1;
	uint64_t ioc3_sbe                     : 1;
	uint64_t ioc3_dbe                     : 1;
	uint64_t xmda0_sbe                    : 1;
	uint64_t xmda0_dbe                    : 1;
	uint64_t xmda1_sbe                    : 1;
	uint64_t xmda1_dbe                    : 1;
	uint64_t xmda2_sbe                    : 1;
	uint64_t xmda2_dbe                    : 1;
	uint64_t xmda3_sbe                    : 1;
	uint64_t xmda3_dbe                    : 1;
	uint64_t xmdb0_sbe                    : 1;
	uint64_t xmdb0_dbe                    : 1;
	uint64_t xmdb1_sbe                    : 1;
	uint64_t xmdb1_dbe                    : 1;
	uint64_t xmdb2_sbe                    : 1;
	uint64_t xmdb2_dbe                    : 1;
	uint64_t xmdb3_sbe                    : 1;
	uint64_t xmdb3_dbe                    : 1;
	uint64_t xmc_sbe                      : 1;
	uint64_t xmc_dbe                      : 1;
	uint64_t ide_sbe                      : 1;
	uint64_t ide_dbe                      : 1;
	uint64_t icc0_sbe                     : 1;
	uint64_t icc0_dbe                     : 1;
	uint64_t icc1_sbe                     : 1;
	uint64_t icc1_dbe                     : 1;
	uint64_t icc2_sbe                     : 1;
	uint64_t icc2_dbe                     : 1;
	uint64_t icc3_sbe                     : 1;
	uint64_t icc3_dbe                     : 1;
	uint64_t reserved_52_63               : 12;
#endif
	} s;
	struct cvmx_iobn_int_sum_s            cn73xx;
	struct cvmx_iobn_int_sum_s            cn78xx;
	struct cvmx_iobn_int_sum_s            cn78xxp1;
	struct cvmx_iobn_int_sum_s            cnf75xx;
};
typedef union cvmx_iobn_int_sum cvmx_iobn_int_sum_t;

/**
 * cvmx_iobn_ncb#_ctl
 *
 * This register controls the type of store operation used for full cache blocks stores.
 *
 */
union cvmx_iobn_ncbx_ctl {
	uint64_t u64;
	struct cvmx_iobn_ncbx_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t stp                          : 2;  /**< When a complete cache block is written a STP will be converted to:
                                                         0x0 = STF.
                                                         0x1 = STY.
                                                         0x2 = STT.
                                                         0x3 = Reserved. */
	uint64_t reserved_2_7                 : 6;
	uint64_t rstp                         : 2;  /**< When a complete cache block is written a RSTP will be converted to:
                                                         0x0 = STY.
                                                         0x1 = STT.
                                                         0x2 = STF.
                                                         0x3 = Reserved. */
#else
	uint64_t rstp                         : 2;
	uint64_t reserved_2_7                 : 6;
	uint64_t stp                          : 2;
	uint64_t reserved_10_63               : 54;
#endif
	} s;
	struct cvmx_iobn_ncbx_ctl_s           cn73xx;
	struct cvmx_iobn_ncbx_ctl_s           cn78xx;
	struct cvmx_iobn_ncbx_ctl_s           cn78xxp1;
	struct cvmx_iobn_ncbx_ctl_s           cnf75xx;
};
typedef union cvmx_iobn_ncbx_ctl cvmx_iobn_ncbx_ctl_t;

/**
 * cvmx_iobn_pp_bist_status
 *
 * This register contains the result of the BIST run on the cores.
 *
 */
union cvmx_iobn_pp_bist_status {
	uint64_t u64;
	struct cvmx_iobn_pp_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t pp_bstat                     : 48; /**< BIST Status of the cores. Bit vector position is the physical number of the core
                                                         (i.e. core 0 == PP_BSTAT<0>). Only even number bits are valid; all odd number bits
                                                         are read as 0. For odd number cores, see IOBP_PP_BIST_STATUS.
                                                         Software must bit-wise logical AND IOBN_PP_BIST_STATUS with CIU_FUSE before using
                                                         it. */
#else
	uint64_t pp_bstat                     : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_iobn_pp_bist_status_s     cn73xx;
	struct cvmx_iobn_pp_bist_status_s     cn78xx;
	struct cvmx_iobn_pp_bist_status_s     cn78xxp1;
	struct cvmx_iobn_pp_bist_status_s     cnf75xx;
};
typedef union cvmx_iobn_pp_bist_status cvmx_iobn_pp_bist_status_t;

/**
 * cvmx_iobn_roc_dll
 *
 * Status of the ROC core-clock DLL.
 *
 */
union cvmx_iobn_roc_dll {
	uint64_t u64;
	struct cvmx_iobn_roc_dll_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t max_dll_setting              : 12; /**< Max reported DLL setting. */
	uint64_t min_dll_setting              : 12; /**< Min reported DLL setting. */
	uint64_t reserved_32_35               : 4;
	uint64_t pdr_rclk_refclk              : 1;  /**< Synchronized pdr_rclk_refclk from ROC core-clock DLL cmb0 phase detectors. */
	uint64_t pdl_rclk_refclk              : 1;  /**< Synchronized pdl_rclk_refclk from ROC core-clock DLL cmb0 phase detectors. */
	uint64_t pd_pos_rclk_refclk           : 1;  /**< Synchronized pd_pos_rclk_refclk from ROC core-clock DLL cmb0 phase detectors. */
	uint64_t dll_lock                     : 1;  /**< The dll_lock signal from ROC core-clock DLL, from the positive edge of refclk. */
	uint64_t dll_dly_elem_en              : 16; /**< The ROC core-clock delay element enable setting, from the negative edge of refclk. */
	uint64_t dll_setting                  : 12; /**< The ROC core-clock DLL setting, from the negative edge of refclk. */
#else
	uint64_t dll_setting                  : 12;
	uint64_t dll_dly_elem_en              : 16;
	uint64_t dll_lock                     : 1;
	uint64_t pd_pos_rclk_refclk           : 1;
	uint64_t pdl_rclk_refclk              : 1;
	uint64_t pdr_rclk_refclk              : 1;
	uint64_t reserved_32_35               : 4;
	uint64_t min_dll_setting              : 12;
	uint64_t max_dll_setting              : 12;
	uint64_t reserved_60_63               : 4;
#endif
	} s;
	struct cvmx_iobn_roc_dll_s            cnf75xx;
};
typedef union cvmx_iobn_roc_dll cvmx_iobn_roc_dll_t;

#endif
