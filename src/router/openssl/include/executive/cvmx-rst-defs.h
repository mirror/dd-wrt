/***********************license start***************
 * Copyright (c) 2003-2013  Cavium Inc. (support@cavium.com). All rights
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
#define CVMX_RST_BOOT CVMX_RST_BOOT_FUNC()
static inline uint64_t CVMX_RST_BOOT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_RST_BOOT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180006001600ull);
}
#else
#define CVMX_RST_BOOT (CVMX_ADD_IO_SEG(0x0001180006001600ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_RST_CFG CVMX_RST_CFG_FUNC()
static inline uint64_t CVMX_RST_CFG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX)))
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
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_RST_CKILL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180006001638ull);
}
#else
#define CVMX_RST_CKILL (CVMX_ADD_IO_SEG(0x0001180006001638ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_RST_CTLX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_RST_CTLX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180006001640ull) + ((offset) & 3) * 8;
}
#else
#define CVMX_RST_CTLX(offset) (CVMX_ADD_IO_SEG(0x0001180006001640ull) + ((offset) & 3) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_RST_DELAY CVMX_RST_DELAY_FUNC()
static inline uint64_t CVMX_RST_DELAY_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_RST_DELAY not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180006001608ull);
}
#else
#define CVMX_RST_DELAY (CVMX_ADD_IO_SEG(0x0001180006001608ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_RST_INT CVMX_RST_INT_FUNC()
static inline uint64_t CVMX_RST_INT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_RST_INT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180006001628ull);
}
#else
#define CVMX_RST_INT (CVMX_ADD_IO_SEG(0x0001180006001628ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_RST_OCX CVMX_RST_OCX_FUNC()
static inline uint64_t CVMX_RST_OCX_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_RST_OCX not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180006001618ull);
}
#else
#define CVMX_RST_OCX (CVMX_ADD_IO_SEG(0x0001180006001618ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_RST_POWER_DBG CVMX_RST_POWER_DBG_FUNC()
static inline uint64_t CVMX_RST_POWER_DBG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
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
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_RST_PP_POWER not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180006001700ull);
}
#else
#define CVMX_RST_PP_POWER (CVMX_ADD_IO_SEG(0x0001180006001700ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_RST_SOFT_PRSTX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3)))))
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
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_RST_SOFT_RST not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180006001680ull);
}
#else
#define CVMX_RST_SOFT_RST (CVMX_ADD_IO_SEG(0x0001180006001680ull))
#endif

/**
 * cvmx_rst_boot
 */
union cvmx_rst_boot {
	uint64_t u64;
	struct cvmx_rst_boot_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t chipkill                     : 1;  /**< A 0->1 transition of CHIPKILL starts the CHIPKILL timer. When CHIPKILL=1 and the timer
                                                         expires, internal chip reset is asserted forever until the next chip reset. The CHIPKILL
                                                         timer can be stopped only by a chip (cold, warm, soft) reset. The length of the CHIPKILL
                                                         timer is specified by RST_CKILL[TIMER]. */
	uint64_t jtcsrdis                     : 1;  /**< When set, internal CSR access via JTAG TAP controller is disabled This field resets to 1
                                                         in Authentik mode, else 0. */
	uint64_t ejtagdis                     : 1;  /**< When set, external EJTAG access is disabled This field resets to 1 in Authentik mode, else 0. */
	uint64_t romen                        : 1;  /**< When set, Authentik/eMMC boot ROM is visible in the boot bus address space. This field
                                                         resets to 1 in an Authentik part or when booting from eMMC. Else, resets to 0. */
	uint64_t ckill_ppdis                  : 1;  /**< When set, cores other than 0 are disabled during a CHIPKILL.  Writes have no effect when
                                                         RST_BOOT[CHIPKILL]=1. */
	uint64_t jt_tstmode                   : 1;  /**< JTAG test mode. */
	uint64_t vrm_err                      : 1;  /**< VRM did not complete operations within 5.25mS of DCOK being asserted. PLLs were released
                                                         automatically. */
	uint64_t reserved_37_56               : 20;
	uint64_t c_mul                        : 7;  /**< Core-clock multiplier. C_MUL = (core-clock speed) / (ref-clock speed). 'ref-clock speed'
                                                         should always be 50MHz. */
	uint64_t pnr_mul                      : 6;  /**< Coprocessor-clock multiplier. PNR_MUL = (coprocessor-clock speed) /(ref-clock speed).
                                                         'ref-clock speed' should always be 50MHz.
                                                         For PCIe Gen1, the coprocessor-clock speed must be greater than 250MHz; for PCIe Gen2, the
                                                         coprocessor-clock speed must be greater than 500MHz; for PCIe Gen3, the coprocessor-clock
                                                         speed must be greater than 800MHz. */
	uint64_t reserved_21_23               : 3;
	uint64_t lboot_oci                    : 3;  /**< Last boot cause mask; resets only with DCOK.
                                                         <20> Warm reset due to OCI Link 2 going down.
                                                         <19> Warm reset due to OCI Link 1 going down.
                                                         <18> Warm reset due to OCI Link 0 going down. */
	uint64_t lboot_ext                    : 6;  /**< Last boot cause mask; resets only with DCOK.
                                                         <17> Warm reset due to Cntl3 link-down or hot-reset.
                                                         <16> Warm reset due to Cntl2 link-down or hot-reset.
                                                         <15> Cntl3 reset due to PERST3_L pin.
                                                         <14> Cntl2 reset due to PERST2_L pin.
                                                         <13> Warm reset due to PERST3_L pin.
                                                         <12> Warm reset due to PERST2_L pin. */
	uint64_t lboot                        : 10; /**< Last boot cause mask; resets only with DCOK.
                                                         <11> Soft reset due to watchdog.
                                                         <10> Soft reset due to RST_SOFT_RST write.
                                                         <9> Warm reset due to Cntl0 link-down or hot-reset.
                                                         <8> Warm reset due to Cntl1 link-down or hot-reset.
                                                         <7> Cntl1 reset due to PERST1_L pin.
                                                         <6> Cntl0 reset due to PERST0_L pin.
                                                         <5> Warm reset due to PERST1_L pin.
                                                         <4> Warm reset due to PERST0_L pin.
                                                         <3> Warm reset due to CHIP_RESET_L pin.
                                                         <2> Cold reset due to PLL_DC_OK pin. */
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
	struct cvmx_rst_boot_s                cn78xx;
};
typedef union cvmx_rst_boot cvmx_rst_boot_t;

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
	uint64_t soft_clr_bist                : 1;  /**< Perform clear BIST during soft reset, instead of a full BIST. A warm/soft reset does not
                                                         change this field. Note that a cold reset always performs a full BIST. */
#else
	uint64_t soft_clr_bist                : 1;
	uint64_t warm_clr_bist                : 1;
	uint64_t cntl_clr_bist                : 1;
	uint64_t reserved_3_5                 : 3;
	uint64_t bist_delay                   : 58;
#endif
	} s;
	struct cvmx_rst_cfg_s                 cn70xx;
	struct cvmx_rst_cfg_s                 cn78xx;
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
	struct cvmx_rst_ckill_s               cn78xx;
};
typedef union cvmx_rst_ckill cvmx_rst_ckill_t;

/**
 * cvmx_rst_ctl#
 */
union cvmx_rst_ctlx {
	uint64_t u64;
	struct cvmx_rst_ctlx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t prst_link                    : 1;  /**< Controls whether corresponding controller link-down or hot-reset causes the assertion of
                                                         RST_SOFT_PRST*[SOFT_PRST].
                                                         A warm/soft reset does not change this field. On cold reset, this field is initialized to
                                                         0. */
	uint64_t rst_done                     : 1;  /**< Read-only access to controller reset status. RST_DONE is always zero (i.e. the controller
                                                         is held in reset) when:
                                                         RST_SOFT_PRST*[SOFT_PRST] = 1, or
                                                         RST_RCV = 1 and PERST*_L pin is asserted. */
	uint64_t rst_link                     : 1;  /**< Reset link. Controls whether corresponding controller link-down reset or hot reset causes
                                                         a warm chip reset. On cold reset, this field is initialized as follows:
                                                         0 when RST_CTL*[HOST_MODE] = 1
                                                         1 when RST_CTL*[HOST_MODE] = 0
                                                         Note that a link-down or hot-reset event can never cause a warm chip reset when the
                                                         controller is in reset (i.e. can never cause a warm reset when RST_DONE = 0). */
	uint64_t host_mode                    : 1;  /**< Read-only access to the corresponding straps indicating PCIE*_MODE is host. For
                                                         controllers 2 and 3 this field is always set. */
	uint64_t reserved_4_5                 : 2;
	uint64_t rst_drv                      : 1;  /**< Controls whether PERST*_L is driven. A warm/soft reset does not change this field. On cold
                                                         reset, this field is initialized as follows:
                                                         0 when RST_CTL*[HOST_MODE] = 0
                                                         1 when RST_CTL*[HOST_MODE] = 1
                                                         When set, CN78XX drives the corresponding PERST*_L pin. Otherwise, CN78XX does not drive
                                                         the corresponding PERST*_L pin. */
	uint64_t rst_rcv                      : 1;  /**< Controls whether PERST*_L is received. A warm/soft reset will not change this field. On
                                                         cold reset, this field is initialized as follows:
                                                         0 when RST_CTL*[HOST_MODE] = 1
                                                         1 when RST_CTL*[HOST_MODE] = 0
                                                         When RST_RCV is equal to 1, the PERST*_L value is received and may be used to reset the
                                                         controller and (optionally, based on RST_CHIP) warm reset the chip.
                                                         When RST_RCV is equal to 1 (and RST_CHIP = 0), RST_INT[PERST*] gets set when the PERST*_L
                                                         pin asserts. (This interrupt can alert software whenever the external reset pin initiates
                                                         a controller reset sequence.)
                                                         RST_VAL gives the PERST*_L pin value when RST_RCV = 1.
                                                         When RST_RCV = 0, the PERST*_L pin value is ignored. */
	uint64_t rst_chip                     : 1;  /**< Controls whether PERST*_L causes a chip warm reset like CHIP_RESET_L. A warm/soft reset
                                                         will not change this field. On cold reset, this field is initialized to 0.
                                                         When RST_RCV = 0, RST_CHIP is ignored.
                                                         When RST_RCV = 1, RST_CHIP = 1, and PERST*_L asserts, a chip warm reset is generated. */
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
	struct cvmx_rst_ctlx_s                cn78xx;
};
typedef union cvmx_rst_ctlx cvmx_rst_ctlx_t;

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
	struct cvmx_rst_delay_s               cn78xx;
};
typedef union cvmx_rst_delay cvmx_rst_delay_t;

/**
 * cvmx_rst_int
 */
union cvmx_rst_int {
	uint64_t u64;
	struct cvmx_rst_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t perst                        : 4;  /**< PERST*_L asserted while RST_CTL*[RST_RCV] = 1 and RST_CTL*[RST_CHIP] = 0. One bit
                                                         corresponds to each controller. Throws RST_INTSN_E::RST_INT_PERST(0..3). */
	uint64_t reserved_4_7                 : 4;
	uint64_t rst_link                     : 4;  /**< A controller link-down/hot-reset occurred while RST_CTL*[RST_LINK] = 0. Software must
                                                         assert then deassert RST_SOFT_PRST*[SOFT_PRST]. One bit corresponds to each controller.
                                                         Throws RST_INTSN_E::RST_INT_LINK(0..3). */
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
	uint64_t perst                        : 3;  /**< PERST*_L asserted while RST_CTL*[RST_RCV] = 1 and RST_CTL*[RST_CHIP] = 0. One bit
                                                         corresponds to each controller. */
	uint64_t reserved_3_7                 : 5;
	uint64_t rst_link                     : 3;  /**< A controller link-down/hot-reset occurred while RST_CTL*[RST_LINK] = 0. Software must
                                                         assert then deassert RST_SOFT_PRST*[SOFT_PRST]. One bit corresponds to each controller. */
#else
	uint64_t rst_link                     : 3;
	uint64_t reserved_3_7                 : 5;
	uint64_t perst                        : 3;
	uint64_t reserved_11_63               : 53;
#endif
	} cn70xx;
	struct cvmx_rst_int_s                 cn78xx;
};
typedef union cvmx_rst_int cvmx_rst_int_t;

/**
 * cvmx_rst_ocx
 */
union cvmx_rst_ocx {
	uint64_t u64;
	struct cvmx_rst_ocx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t rst_link                     : 3;  /**< Controls whether corresponding OCX link going down causes a chip reset. A warm/soft reset
                                                         does not change this field. On cold reset, this field is initialized to 0. */
#else
	uint64_t rst_link                     : 3;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_rst_ocx_s                 cn78xx;
};
typedef union cvmx_rst_ocx cvmx_rst_ocx_t;

/**
 * cvmx_rst_power_dbg
 */
union cvmx_rst_power_dbg {
	uint64_t u64;
	struct cvmx_rst_power_dbg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t str                          : 3;  /**< Reserved. INTERNAL: Internal power driver strength. Resets only on cold reset. */
#else
	uint64_t str                          : 3;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_rst_power_dbg_s           cn78xx;
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
	uint64_t gate                         : 48; /**< Powerdown enable. When both a bit and the corresponding CIU3_PP_RST bit are set, the core
                                                         has voltage removed to save power. In typical operation these bits are setup during
                                                         initialization and PP resets are controlled through CIU3_PP_RST. These bits may only be
                                                         changed when the corresponding core is in reset using CIU3_PP_RST. */
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
	struct cvmx_rst_pp_power_s            cn78xx;
};
typedef union cvmx_rst_pp_power cvmx_rst_pp_power_t;

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
                                                         If the RST_CTL*[HOST_MODE] = 0, SOFT_PRST resets to 0.
                                                         If the RST_CTL*[HOST_MODE] = 1, SOFT_PRST resets to 1.
                                                         When CN78XX is configured to drive PERST*_L (i.e.
                                                         RST_CTL(0..3)[RST_DRV] = 1), this controls the output value on PERST*_L. */
#else
	uint64_t soft_prst                    : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_rst_soft_prstx_s          cn70xx;
	struct cvmx_rst_soft_prstx_s          cn78xx;
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
	uint64_t soft_rst                     : 1;  /**< When set, resets the CN78XX core. When performing a soft reset from a remote PCIe host,
                                                         always read this register and wait for the results before setting SOFT_RST to 1. */
#else
	uint64_t soft_rst                     : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_rst_soft_rst_s            cn70xx;
	struct cvmx_rst_soft_rst_s            cn78xx;
};
typedef union cvmx_rst_soft_rst cvmx_rst_soft_rst_t;

#endif
