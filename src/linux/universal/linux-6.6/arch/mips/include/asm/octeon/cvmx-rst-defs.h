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
 * cvmx-rst-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon rst.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_RST_DEFS_H__
#define __CVMX_RST_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_RST_BIST_TIMER CVMX_RST_BIST_TIMER_FUNC()
static inline uint64_t CVMX_RST_BIST_TIMER_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_RST_BIST_TIMER not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180006001760ull);
}
#else
#define CVMX_RST_BIST_TIMER (CVMX_ADD_IO_SEG(0x0001180006001760ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_RST_BOOT CVMX_RST_BOOT_FUNC()
static inline uint64_t CVMX_RST_BOOT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_RST_BOOT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180006001600ull);
}
#else
#define CVMX_RST_BOOT (CVMX_ADD_IO_SEG(0x0001180006001600ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_RST_BPHY_SOFT_RST CVMX_RST_BPHY_SOFT_RST_FUNC()
static inline uint64_t CVMX_RST_BPHY_SOFT_RST_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_RST_BPHY_SOFT_RST not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180006001698ull);
}
#else
#define CVMX_RST_BPHY_SOFT_RST (CVMX_ADD_IO_SEG(0x0001180006001698ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_RST_CFG CVMX_RST_CFG_FUNC()
static inline uint64_t CVMX_RST_CFG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_RST_CFG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180006001610ull);
}
#else
#define CVMX_RST_CFG (CVMX_ADD_IO_SEG(0x0001180006001610ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_RST_CKILL CVMX_RST_CKILL_FUNC()
static inline uint64_t CVMX_RST_CKILL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_RST_CKILL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180006001638ull);
}
#else
#define CVMX_RST_CKILL (CVMX_ADD_IO_SEG(0x0001180006001638ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_RST_COLD_DATAX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_RST_COLD_DATAX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800060017C0ull) + ((offset) & 3) * 8;
}
#else
#define CVMX_RST_COLD_DATAX(offset) (CVMX_ADD_IO_SEG(0x00011800060017C0ull) + ((offset) & 3) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_RST_CTLX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_RST_CTLX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180006001640ull) + ((offset) & 3) * 8;
}
#else
#define CVMX_RST_CTLX(offset) (CVMX_ADD_IO_SEG(0x0001180006001640ull) + ((offset) & 3) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_RST_DEBUG CVMX_RST_DEBUG_FUNC()
static inline uint64_t CVMX_RST_DEBUG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_RST_DEBUG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800060017B0ull);
}
#else
#define CVMX_RST_DEBUG (CVMX_ADD_IO_SEG(0x00011800060017B0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_RST_DELAY CVMX_RST_DELAY_FUNC()
static inline uint64_t CVMX_RST_DELAY_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_RST_DELAY not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180006001608ull);
}
#else
#define CVMX_RST_DELAY (CVMX_ADD_IO_SEG(0x0001180006001608ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_RST_ECO CVMX_RST_ECO_FUNC()
static inline uint64_t CVMX_RST_ECO_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_RST_ECO not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800060017B8ull);
}
#else
#define CVMX_RST_ECO (CVMX_ADD_IO_SEG(0x00011800060017B8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_RST_INT CVMX_RST_INT_FUNC()
static inline uint64_t CVMX_RST_INT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_RST_INT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180006001628ull);
}
#else
#define CVMX_RST_INT (CVMX_ADD_IO_SEG(0x0001180006001628ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_RST_INT_W1S CVMX_RST_INT_W1S_FUNC()
static inline uint64_t CVMX_RST_INT_W1S_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_RST_INT_W1S not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180006001630ull);
}
#else
#define CVMX_RST_INT_W1S (CVMX_ADD_IO_SEG(0x0001180006001630ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_RST_OCX CVMX_RST_OCX_FUNC()
static inline uint64_t CVMX_RST_OCX_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_RST_OCX not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180006001618ull);
}
#else
#define CVMX_RST_OCX (CVMX_ADD_IO_SEG(0x0001180006001618ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_RST_OUT_CTL CVMX_RST_OUT_CTL_FUNC()
static inline uint64_t CVMX_RST_OUT_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_RST_OUT_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180006001688ull);
}
#else
#define CVMX_RST_OUT_CTL (CVMX_ADD_IO_SEG(0x0001180006001688ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_RST_POWER_DBG CVMX_RST_POWER_DBG_FUNC()
static inline uint64_t CVMX_RST_POWER_DBG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_RST_POWER_DBG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180006001708ull);
}
#else
#define CVMX_RST_POWER_DBG (CVMX_ADD_IO_SEG(0x0001180006001708ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_RST_PP_POWER CVMX_RST_PP_POWER_FUNC()
static inline uint64_t CVMX_RST_PP_POWER_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_RST_PP_POWER not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180006001700ull);
}
#else
#define CVMX_RST_PP_POWER (CVMX_ADD_IO_SEG(0x0001180006001700ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_RST_REF_CNTR CVMX_RST_REF_CNTR_FUNC()
static inline uint64_t CVMX_RST_REF_CNTR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_RST_REF_CNTR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180006001758ull);
}
#else
#define CVMX_RST_REF_CNTR (CVMX_ADD_IO_SEG(0x0001180006001758ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_RST_SOFT_PRSTX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_RST_SOFT_PRSTX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800060016C0ull) + ((offset) & 3) * 8;
}
#else
#define CVMX_RST_SOFT_PRSTX(offset) (CVMX_ADD_IO_SEG(0x00011800060016C0ull) + ((offset) & 3) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_RST_SOFT_RST CVMX_RST_SOFT_RST_FUNC()
static inline uint64_t CVMX_RST_SOFT_RST_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_RST_SOFT_RST not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180006001680ull);
}
#else
#define CVMX_RST_SOFT_RST (CVMX_ADD_IO_SEG(0x0001180006001680ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_RST_THERMAL_ALERT CVMX_RST_THERMAL_ALERT_FUNC()
static inline uint64_t CVMX_RST_THERMAL_ALERT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_RST_THERMAL_ALERT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180006001690ull);
}
#else
#define CVMX_RST_THERMAL_ALERT (CVMX_ADD_IO_SEG(0x0001180006001690ull))
#endif

/**
 * cvmx_rst_bist_timer
 */
union cvmx_rst_bist_timer {
	uint64_t u64;
	struct cvmx_rst_bist_timer_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_29_63               : 35;
	uint64_t count                        : 29; /**< Number of 50 MHz reference clocks that have elapsed during bist and repair during the last
                                                         reset.
                                                         If MSB is set the BIST chain did not complete as expected. */
#else
	uint64_t count                        : 29;
	uint64_t reserved_29_63               : 35;
#endif
	} s;
	struct cvmx_rst_bist_timer_s          cn73xx;
	struct cvmx_rst_bist_timer_s          cn78xx;
	struct cvmx_rst_bist_timer_s          cnf75xx;
};
typedef union cvmx_rst_bist_timer cvmx_rst_bist_timer_t;

/**
 * cvmx_rst_boot
 */
union cvmx_rst_boot {
	uint64_t u64;
	struct cvmx_rst_boot_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t chipkill                     : 1;  /**< A 0-to-1 transition of CHIPKILL starts the CHIPKILL timer. When CHIPKILL=1 and the timer
                                                         expires, chip reset is asserted internally. The CHIPKILL timer can be stopped only by
                                                         a reset (cold, warm, soft). The length of the CHIPKILL timer is specified by
                                                         RST_CKILL[TIMER].  This bit and timer function as a delayed warm reset. */
	uint64_t jtcsrdis                     : 1;  /**< When set, internal CSR access via JTAG TAP controller is disabled This field resets to 1
                                                         in Authentik mode, else 0. */
	uint64_t ejtagdis                     : 1;  /**< When set, external EJTAG access is disabled. This field resets to 1 in Authentik mode, else 0. */
	uint64_t romen                        : 1;  /**< When set, Authentik/eMMC boot ROM is visible in the boot bus address space. This field
                                                         resets to 1 in an Authentik part or when booting from eMMC/SD or SPI. Else, resets to 0. */
	uint64_t ckill_ppdis                  : 1;  /**< Chipkill core disable. When set to 1, cores other than core 0 will immediately
                                                         be disabled when RST_BOOT[CHIPKILL] is set. Writes have no effect when
                                                         RST_BOOT[CHIPKILL]=1. */
	uint64_t jt_tstmode                   : 1;  /**< JTAG test mode. */
	uint64_t vrm_err                      : 1;  /**< VRM did not complete operations within 5.25mS of DCOK being asserted. PLLs were released
                                                         automatically. */
	uint64_t reserved_37_56               : 20;
	uint64_t c_mul                        : 7;  /**< Core-clock multiplier. C_MUL = (core-clock speed) / (ref-clock speed). 'ref-clock speed'
                                                         should always be 50MHz. */
	uint64_t pnr_mul                      : 6;  /**< Coprocessor-clock multiplier. PNR_MUL = (coprocessor-clock speed) /(ref-clock speed).
                                                         'ref-clock speed' should always be 50MHz. */
	uint64_t reserved_21_23               : 3;
	uint64_t lboot_oci                    : 3;  /**< Reserved. */
	uint64_t lboot_ext                    : 6;  /**< For CNF73XX, this field is reserved.
                                                         For CNF75XX, the last boot cause mask; resets only with DCOK.
                                                         <17> = Warm reset due to Cntl3 link-down or hot-reset.
                                                         <16> = Warm reset due to Cntl2 link-down or hot-reset.
                                                         <15> = Cntl3 reset due to PERST3_L pin.
                                                         <14> = Cntl2 reset due to PERST2_L pin.
                                                         <13> = Warm reset due to PERST3_L pin.
                                                         <12> = Warm reset due to PERST2_L pin. */
	uint64_t lboot                        : 10; /**< Last boot cause mask; resets only with DCOK.
                                                         <11> = Soft reset due to watchdog.
                                                         <10> = Soft reset due to RST_SOFT_RST write.
                                                         <9> = Warm reset due to Cntl1 link-down or hot-reset.
                                                         <8> = Warm reset due to Cntl0 link-down or hot-reset.
                                                         <7> = Cntl1 reset due to PERST1_L pin.
                                                         <6> = Cntl0 reset due to PERST0_L pin.
                                                         <5> = Warm reset due to PERST1_L pin.
                                                         <4> = Warm reset due to PERST0_L pin.
                                                         <3> = Warm reset due to CHIP_RESET_L pin.
                                                         <2> = Cold reset due to PLL_DC_OK pin. */
	uint64_t rboot                        : 1;  /**< Determines whether core 0 remains in reset after chip cold/warm/soft reset. */
	uint64_t rboot_pin                    : 1;  /**< Read-only access to REMOTE_BOOT pin. */
#else
	uint64_t rboot_pin                    : 1;
	uint64_t rboot                        : 1;
	uint64_t lboot                        : 10;
	uint64_t lboot_ext                    : 6;
	uint64_t lboot_oci                    : 3;
	uint64_t reserved_21_23               : 3;
	uint64_t pnr_mul                      : 6;
	uint64_t c_mul                        : 7;
	uint64_t reserved_37_56               : 20;
	uint64_t vrm_err                      : 1;
	uint64_t jt_tstmode                   : 1;
	uint64_t ckill_ppdis                  : 1;
	uint64_t romen                        : 1;
	uint64_t ejtagdis                     : 1;
	uint64_t jtcsrdis                     : 1;
	uint64_t chipkill                     : 1;
#endif
	} s;
	struct cvmx_rst_boot_s                cn70xx;
	struct cvmx_rst_boot_s                cn70xxp1;
	struct cvmx_rst_boot_s                cn73xx;
	struct cvmx_rst_boot_s                cn78xx;
	struct cvmx_rst_boot_s                cn78xxp1;
	struct cvmx_rst_boot_s                cnf75xx;
};
typedef union cvmx_rst_boot cvmx_rst_boot_t;

/**
 * cvmx_rst_bphy_soft_rst
 */
union cvmx_rst_bphy_soft_rst {
	uint64_t u64;
	struct cvmx_rst_bphy_soft_rst_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t soft_rst                     : 1;  /**< Reserved.  For diagnostic use only. */
#else
	uint64_t soft_rst                     : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_rst_bphy_soft_rst_s       cnf75xx;
};
typedef union cvmx_rst_bphy_soft_rst cvmx_rst_bphy_soft_rst_t;

/**
 * cvmx_rst_cfg
 */
union cvmx_rst_cfg {
	uint64_t u64;
	struct cvmx_rst_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bist_delay                   : 58; /**< Reserved. */
	uint64_t reserved_3_5                 : 3;
	uint64_t cntl_clr_bist                : 1;  /**< Perform clear BIST during control-only reset, instead of a full BIST. A warm/soft reset
                                                         will not change this field. */
	uint64_t warm_clr_bist                : 1;  /**< Perform clear BIST during warm reset, instead of a full BIST. A warm/soft reset does not
                                                         change this field. Note that a cold reset always performs a full BIST. */
	uint64_t reserved_0_0                 : 1;
#else
	uint64_t reserved_0_0                 : 1;
	uint64_t warm_clr_bist                : 1;
	uint64_t cntl_clr_bist                : 1;
	uint64_t reserved_3_5                 : 3;
	uint64_t bist_delay                   : 58;
#endif
	} s;
	struct cvmx_rst_cfg_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bist_delay                   : 58; /**< Reserved. */
	uint64_t reserved_3_5                 : 3;
	uint64_t cntl_clr_bist                : 1;  /**< Perform clear BIST during control-only reset, instead of a full BIST. A warm/soft reset
                                                         will not change this field. */
	uint64_t warm_clr_bist                : 1;  /**< Perform clear BIST during warm reset, instead of a full BIST. A warm/soft reset does not
                                                         change this field. Note that a cold reset always performs a full BIST. */
	uint64_t soft_clr_bist                : 1;  /**< Perform clear BIST during soft reset, instead of a full BIST. A warm/soft reset does not
                                                         change this field. Note that a cold reset always performs a full BIST. */
#else
	uint64_t soft_clr_bist                : 1;
	uint64_t warm_clr_bist                : 1;
	uint64_t cntl_clr_bist                : 1;
	uint64_t reserved_3_5                 : 3;
	uint64_t bist_delay                   : 58;
#endif
	} cn70xx;
	struct cvmx_rst_cfg_cn70xx            cn70xxp1;
	struct cvmx_rst_cfg_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bist_delay                   : 58; /**< Reserved. */
	uint64_t reserved_1_5                 : 5;
	uint64_t clr_bist                     : 1;  /**< Perform clear BIST during a reset, instead of a full BIST. A warm/soft reset does not
                                                         change this field. Note that a cold reset always performs a full BIST. */
#else
	uint64_t clr_bist                     : 1;
	uint64_t reserved_1_5                 : 5;
	uint64_t bist_delay                   : 58;
#endif
	} cn73xx;
	struct cvmx_rst_cfg_cn70xx            cn78xx;
	struct cvmx_rst_cfg_cn70xx            cn78xxp1;
	struct cvmx_rst_cfg_cn73xx            cnf75xx;
};
typedef union cvmx_rst_cfg cvmx_rst_cfg_t;

/**
 * cvmx_rst_ckill
 */
union cvmx_rst_ckill {
	uint64_t u64;
	struct cvmx_rst_ckill_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_47_63               : 17;
	uint64_t timer                        : 47; /**< Chipkill timer measured in SCLKs. Reads return current chipkill timer. Writes have no
                                                         effect when RST_BOOT[CHIPKILL] = 1. */
#else
	uint64_t timer                        : 47;
	uint64_t reserved_47_63               : 17;
#endif
	} s;
	struct cvmx_rst_ckill_s               cn70xx;
	struct cvmx_rst_ckill_s               cn70xxp1;
	struct cvmx_rst_ckill_s               cn73xx;
	struct cvmx_rst_ckill_s               cn78xx;
	struct cvmx_rst_ckill_s               cn78xxp1;
	struct cvmx_rst_ckill_s               cnf75xx;
};
typedef union cvmx_rst_ckill cvmx_rst_ckill_t;

/**
 * cvmx_rst_cold_data#
 */
union cvmx_rst_cold_datax {
	uint64_t u64;
	struct cvmx_rst_cold_datax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Scratch data registers preserved through warm reset.
                                                         Reset to 0x0 on cold reset. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_rst_cold_datax_s          cn73xx;
	struct cvmx_rst_cold_datax_s          cn78xx;
	struct cvmx_rst_cold_datax_s          cnf75xx;
};
typedef union cvmx_rst_cold_datax cvmx_rst_cold_datax_t;

/**
 * cvmx_rst_ctl#
 */
union cvmx_rst_ctlx {
	uint64_t u64;
	struct cvmx_rst_ctlx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t prst_link                    : 1;  /**< PEM reset on link down.
                                                         0 = Link-down or hot-reset will set RST_INT[RST_LINK] for the corresponding
                                                         controller, and (provided properly configured) the link should come back up
                                                         automatically.
                                                         1 = Link-down or hot-reset will set RST_INT[RST_LINK] for the corresponding
                                                         controller, and set RST_SOFT_PRST()[SOFT_PRST]. This will hold the link in reset
                                                         until software clears RST_SOFT_PRST()[SOFT_PRST].
                                                         A warm/soft reset does not change this field. On cold reset, this field is initialized to
                                                         0. */
	uint64_t rst_done                     : 1;  /**< Read-only access to controller reset status. RST_DONE is always zero (i.e. the controller
                                                         is held in reset) when:
                                                         * RST_SOFT_PRST()[SOFT_PRST] = 1, or
                                                         * RST_RCV = 1 and PERST*_L pin is asserted. */
	uint64_t rst_link                     : 1;  /**< Reset on link down. When set, a corresponding controller link-down reset or hot
                                                         reset causes a warm chip reset.
                                                         On cold reset, this field is initialized as follows:
                                                         _ 0 when RST_CTL()[HOST_MODE] = 1.
                                                         _ 1 when RST_CTL()[HOST_MODE] = 0.
                                                         Note that a link-down or hot-reset event can never cause a warm chip reset when the
                                                         controller is in reset (i.e. can never cause a warm reset when [RST_DONE] = 0). */
	uint64_t host_mode                    : 1;  /**< Read-only access to the corresponding PEM()_CFG[HOSTMD] field indicating PEMn is root
                                                         complex (host). */
	uint64_t reserved_4_5                 : 2;
	uint64_t rst_drv                      : 1;  /**< Controls whether PERST*_L is driven. A warm/soft reset does not change this field. On cold
                                                         reset, this field is initialized as follows:
                                                         _ 0 when RST_CTL()[HOST_MODE] = 0.
                                                         _ 1 when RST_CTL()[HOST_MODE] = 1.
                                                         When set, CNXXXX drives the corresponding PERST*_L pin. Otherwise, CNXXXX does not drive
                                                         the corresponding PERST*_L pin. */
	uint64_t rst_rcv                      : 1;  /**< Controls whether PERST*_L is received. A warm/soft reset will not change this field. On
                                                         cold reset, this field is initialized as follows:
                                                         _ 0 when RST_CTL()[HOST_MODE] = 1.
                                                         _ 1 when RST_CTL()[HOST_MODE] = 0.
                                                         When RST_RCV is equal to 1, the PERST*_L value is received and may be used to reset the
                                                         controller and (optionally, based on RST_CHIP) warm reset the chip.
                                                         When RST_RCV is equal to 1 (and RST_CHIP = 0), RST_INT[PERST*] gets set when the PERST*_L
                                                         pin asserts. (This interrupt can alert software whenever the external reset pin initiates
                                                         a controller reset sequence.)
                                                         RST_VAL gives the PERST*_L pin value when RST_RCV = 1.
                                                         When RST_RCV = 0, the PERST*_L pin value is ignored. */
	uint64_t rst_chip                     : 1;  /**< Controls whether PERST*_L causes a chip warm reset like CHIP_RESET_L. A warm/soft reset
                                                         will not change this field. On cold reset, this field is initialized to 0.
                                                         When [RST_RCV] = 0, [RST_CHIP] is ignored.
                                                         When [RST_RCV] = 1, [RST_CHIP] = 1, and PERST*_L asserts, a chip warm reset is generated. */
	uint64_t rst_val                      : 1;  /**< Read-only access to PERST*_L. Unpredictable when RST_RCV = 0.
                                                         Reads as 1 when RST_RCV is equal to 1 and the
                                                         PERST*_L pin is asserted.
                                                         Reads as 0 when RST_RCV = 1 and the PERST*_L pin is not asserted. */
#else
	uint64_t rst_val                      : 1;
	uint64_t rst_chip                     : 1;
	uint64_t rst_rcv                      : 1;
	uint64_t rst_drv                      : 1;
	uint64_t reserved_4_5                 : 2;
	uint64_t host_mode                    : 1;
	uint64_t rst_link                     : 1;
	uint64_t rst_done                     : 1;
	uint64_t prst_link                    : 1;
	uint64_t reserved_10_63               : 54;
#endif
	} s;
	struct cvmx_rst_ctlx_s                cn70xx;
	struct cvmx_rst_ctlx_s                cn70xxp1;
	struct cvmx_rst_ctlx_s                cn73xx;
	struct cvmx_rst_ctlx_s                cn78xx;
	struct cvmx_rst_ctlx_s                cn78xxp1;
	struct cvmx_rst_ctlx_s                cnf75xx;
};
typedef union cvmx_rst_ctlx cvmx_rst_ctlx_t;

/**
 * cvmx_rst_debug
 */
union cvmx_rst_debug {
	uint64_t u64;
	struct cvmx_rst_debug_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t clk_on                       : 1;  /**< Force conditional clock used for interrupt logic to always be on. For diagnostic use only. */
#else
	uint64_t clk_on                       : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_rst_debug_s               cn78xx;
	struct cvmx_rst_debug_s               cnf75xx;
};
typedef union cvmx_rst_debug cvmx_rst_debug_t;

/**
 * cvmx_rst_delay
 */
union cvmx_rst_delay {
	uint64_t u64;
	struct cvmx_rst_delay_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t warm_rst_dly                 : 16; /**< Warm reset delay. A warm reset immediately causes an early warm-reset notification, but
                                                         the assertion of warm reset is delayed this many coprocessor-clock cycles. A warm/soft
                                                         reset will not change this field.
                                                         This must be at least 500 DCLK cycles. */
	uint64_t soft_rst_dly                 : 16; /**< Soft reset delay. A soft reset immediately causes an early soft-reset notification, but
                                                         the assertion of soft reset is delayed this many coprocessor-clock cycles. A warm/soft
                                                         reset will not change this field.
                                                         This must be at least 500 DCLK cycles. */
#else
	uint64_t soft_rst_dly                 : 16;
	uint64_t warm_rst_dly                 : 16;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_rst_delay_s               cn70xx;
	struct cvmx_rst_delay_s               cn70xxp1;
	struct cvmx_rst_delay_s               cn73xx;
	struct cvmx_rst_delay_s               cn78xx;
	struct cvmx_rst_delay_s               cn78xxp1;
	struct cvmx_rst_delay_s               cnf75xx;
};
typedef union cvmx_rst_delay cvmx_rst_delay_t;

/**
 * cvmx_rst_eco
 */
union cvmx_rst_eco {
	uint64_t u64;
	struct cvmx_rst_eco_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t eco_rw                       : 32; /**< ECO flops. */
#else
	uint64_t eco_rw                       : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_rst_eco_s                 cn73xx;
	struct cvmx_rst_eco_s                 cn78xx;
	struct cvmx_rst_eco_s                 cnf75xx;
};
typedef union cvmx_rst_eco cvmx_rst_eco_t;

/**
 * cvmx_rst_int
 */
union cvmx_rst_int {
	uint64_t u64;
	struct cvmx_rst_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t perst                        : 4;  /**< PERST*_L asserted while RST_CTL()[RST_RCV] = 1 and RST_CTL()[RST_CHIP] = 0. One bit
                                                         corresponds to each controller. */
	uint64_t reserved_4_7                 : 4;
	uint64_t rst_link                     : 4;  /**< A controller link-down/hot-reset occurred while RST_CTL()[RST_LINK] = 0. Software must
                                                         assert then deassert RST_SOFT_PRST()[SOFT_PRST]. One bit corresponds to each controller. */
#else
	uint64_t rst_link                     : 4;
	uint64_t reserved_4_7                 : 4;
	uint64_t perst                        : 4;
	uint64_t reserved_12_63               : 52;
#endif
	} s;
	struct cvmx_rst_int_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_11_63               : 53;
	uint64_t perst                        : 3;  /**< PERST*_L asserted while RST_CTL()[RST_RCV] = 1 and RST_CTL()[RST_CHIP] = 0. One bit
                                                         corresponds to each controller. */
	uint64_t reserved_3_7                 : 5;
	uint64_t rst_link                     : 3;  /**< A controller link-down/hot-reset occurred while RST_CTL()[RST_LINK] = 0. Software must
                                                         assert then deassert RST_SOFT_PRST()[SOFT_PRST]. One bit corresponds to each controller. */
#else
	uint64_t rst_link                     : 3;
	uint64_t reserved_3_7                 : 5;
	uint64_t perst                        : 3;
	uint64_t reserved_11_63               : 53;
#endif
	} cn70xx;
	struct cvmx_rst_int_cn70xx            cn70xxp1;
	struct cvmx_rst_int_s                 cn73xx;
	struct cvmx_rst_int_s                 cn78xx;
	struct cvmx_rst_int_s                 cn78xxp1;
	struct cvmx_rst_int_s                 cnf75xx;
};
typedef union cvmx_rst_int cvmx_rst_int_t;

/**
 * cvmx_rst_int_w1s
 */
union cvmx_rst_int_w1s {
	uint64_t u64;
	struct cvmx_rst_int_w1s_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t perst                        : 4;  /**< PERST*_L asserted while RST_CTL()[RST_RCV] = 1 and RST_CTL()[RST_CHIP] = 0. One bit
                                                         corresponds to each controller. */
	uint64_t reserved_4_7                 : 4;
	uint64_t rst_link                     : 4;  /**< A controller link-down/hot-reset occurred while RST_CTL()[RST_LINK] = 0. Software must
                                                         assert then deassert RST_SOFT_PRST()[SOFT_PRST]. One bit corresponds to each controller. */
#else
	uint64_t rst_link                     : 4;
	uint64_t reserved_4_7                 : 4;
	uint64_t perst                        : 4;
	uint64_t reserved_12_63               : 52;
#endif
	} s;
	struct cvmx_rst_int_w1s_s             cn73xx;
	struct cvmx_rst_int_w1s_s             cn78xx;
	struct cvmx_rst_int_w1s_s             cnf75xx;
};
typedef union cvmx_rst_int_w1s cvmx_rst_int_w1s_t;

/**
 * cvmx_rst_ocx
 */
union cvmx_rst_ocx {
	uint64_t u64;
	struct cvmx_rst_ocx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t rst_link                     : 3;  /**< Controls whether corresponding OCX link going down causes a chip reset. A warm/soft reset
                                                         does not change this field. On cold reset, this field is initialized to 0. See
                                                         OCX_COM_LINK()_CTL for a description of what events can contribute to the link_down
                                                         condition. */
#else
	uint64_t rst_link                     : 3;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_rst_ocx_s                 cn78xx;
	struct cvmx_rst_ocx_s                 cn78xxp1;
};
typedef union cvmx_rst_ocx cvmx_rst_ocx_t;

/**
 * cvmx_rst_out_ctl
 */
union cvmx_rst_out_ctl {
	uint64_t u64;
	struct cvmx_rst_out_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t soft_rst                     : 1;  /**< Soft reset. When set to 1 by software, this field drives the RST_OUT_N pin
                                                         active low. In this case the field must also be cleared by software to deassert
                                                         the pin. The pin is also automatically asserted and deasserted by hardware
                                                         during a cold/warm/soft reset. */
#else
	uint64_t soft_rst                     : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_rst_out_ctl_s             cn73xx;
	struct cvmx_rst_out_ctl_s             cnf75xx;
};
typedef union cvmx_rst_out_ctl cvmx_rst_out_ctl_t;

/**
 * cvmx_rst_power_dbg
 */
union cvmx_rst_power_dbg {
	uint64_t u64;
	struct cvmx_rst_power_dbg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t str                          : 3;  /**< Reserved. */
#else
	uint64_t str                          : 3;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_rst_power_dbg_s           cn73xx;
	struct cvmx_rst_power_dbg_s           cn78xx;
	struct cvmx_rst_power_dbg_s           cn78xxp1;
	struct cvmx_rst_power_dbg_s           cnf75xx;
};
typedef union cvmx_rst_power_dbg cvmx_rst_power_dbg_t;

/**
 * cvmx_rst_pp_power
 *
 * These bits should only be changed while the corresponding PP is in reset (see CIU3_PP_RST).
 *
 */
union cvmx_rst_pp_power {
	uint64_t u64;
	struct cvmx_rst_pp_power_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t gate                         : 48; /**< Powerdown enable. When both a bit and the corresponding CIU_PP_RST bit are set, the core
                                                         has voltage removed to save power. In typical operation these bits are setup during
                                                         initialization and PP resets are controlled through CIU_PP_RST. These bits may only be
                                                         changed when the corresponding core is in reset using CIU_PP_RST.
                                                         The upper bits of this field remain accessible but will have no effect if the cores
                                                         are disabled. The number of bits cleared in CIU_FUSE[FUSE] indicate the number of cores. */
#else
	uint64_t gate                         : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_rst_pp_power_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t gate                         : 4;  /**< When set, corresponding PP has voltage removed to save power. */
#else
	uint64_t gate                         : 4;
	uint64_t reserved_4_63                : 60;
#endif
	} cn70xx;
	struct cvmx_rst_pp_power_cn70xx       cn70xxp1;
	struct cvmx_rst_pp_power_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t gate                         : 16; /**< Powerdown enable. When both a bit and the corresponding CIU_PP_RST bit are set, the core
                                                         has voltage removed to save power. In typical operation these bits are setup during
                                                         initialization and PP resets are controlled through CIU_PP_RST. These bits may only be
                                                         changed when the corresponding core is in reset using CIU_PP_RST.
                                                         The upper bits of this field remain accessible but will have no effect if the cores
                                                         are disabled. The number of bits cleared in CIU_FUSE[FUSE] indicate the number of cores. */
#else
	uint64_t gate                         : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} cn73xx;
	struct cvmx_rst_pp_power_s            cn78xx;
	struct cvmx_rst_pp_power_s            cn78xxp1;
	struct cvmx_rst_pp_power_cn73xx       cnf75xx;
};
typedef union cvmx_rst_pp_power cvmx_rst_pp_power_t;

/**
 * cvmx_rst_ref_cntr
 */
union cvmx_rst_ref_cntr {
	uint64_t u64;
	struct cvmx_rst_ref_cntr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t cnt                          : 64; /**< Count. The counter is initialized to 0x0 during a cold reset and is otherwise continuously
                                                         running.
                                                         CNT is incremented every reference clock cycle (i.e. at 50 MHz). */
#else
	uint64_t cnt                          : 64;
#endif
	} s;
	struct cvmx_rst_ref_cntr_s            cn73xx;
	struct cvmx_rst_ref_cntr_s            cn78xx;
	struct cvmx_rst_ref_cntr_s            cnf75xx;
};
typedef union cvmx_rst_ref_cntr cvmx_rst_ref_cntr_t;

/**
 * cvmx_rst_soft_prst#
 */
union cvmx_rst_soft_prstx {
	uint64_t u64;
	struct cvmx_rst_soft_prstx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t soft_prst                    : 1;  /**< Resets the PCIe logic and corresponding common logic associated with the SLI controller in
                                                         all modes, not just RC mode.
                                                         * If the RST_CTL()[HOST_MODE] = 0, SOFT_PRST resets to 0.
                                                         * If the RST_CTL()[HOST_MODE] = 1, SOFT_PRST resets to 1.
                                                         When CNXXXX is configured to drive PERST*_L (i.e.
                                                         RST_CTL()[RST_DRV] = 1), this controls the output value on PERST*_L. */
#else
	uint64_t soft_prst                    : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_rst_soft_prstx_s          cn70xx;
	struct cvmx_rst_soft_prstx_s          cn70xxp1;
	struct cvmx_rst_soft_prstx_s          cn73xx;
	struct cvmx_rst_soft_prstx_s          cn78xx;
	struct cvmx_rst_soft_prstx_s          cn78xxp1;
	struct cvmx_rst_soft_prstx_s          cnf75xx;
};
typedef union cvmx_rst_soft_prstx cvmx_rst_soft_prstx_t;

/**
 * cvmx_rst_soft_rst
 */
union cvmx_rst_soft_rst {
	uint64_t u64;
	struct cvmx_rst_soft_rst_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t soft_rst                     : 1;  /**< When set, resets the CNXXXX core. When performing a soft reset from a remote PCIe host,
                                                         always read this register and wait for the results before setting SOFT_RST to 1. */
#else
	uint64_t soft_rst                     : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_rst_soft_rst_s            cn70xx;
	struct cvmx_rst_soft_rst_s            cn70xxp1;
	struct cvmx_rst_soft_rst_s            cn73xx;
	struct cvmx_rst_soft_rst_s            cn78xx;
	struct cvmx_rst_soft_rst_s            cn78xxp1;
	struct cvmx_rst_soft_rst_s            cnf75xx;
};
typedef union cvmx_rst_soft_rst cvmx_rst_soft_rst_t;

/**
 * cvmx_rst_thermal_alert
 */
union cvmx_rst_thermal_alert {
	uint64_t u64;
	struct cvmx_rst_thermal_alert_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t trip                         : 1;  /**< This field is set by the onboard temperature sensor. For diagnostic use
                                                         only. The bit can only be cleared by a deassertion of the PLL_DC_OK pin which
                                                         completely resets the chip. */
	uint64_t reserved_1_7                 : 7;
	uint64_t alert                        : 1;  /**< Thermal alert status. When set to 1, indicates the temperature sensor is currently at the
                                                         failure threshold. */
#else
	uint64_t alert                        : 1;
	uint64_t reserved_1_7                 : 7;
	uint64_t trip                         : 1;
	uint64_t reserved_9_63                : 55;
#endif
	} s;
	struct cvmx_rst_thermal_alert_s       cn73xx;
	struct cvmx_rst_thermal_alert_s       cnf75xx;
};
typedef union cvmx_rst_thermal_alert cvmx_rst_thermal_alert_t;

#endif
