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
 * cvmx-xcv-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon xcv.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_XCV_DEFS_H__
#define __CVMX_XCV_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_XCV_BATCH_CRD_RET CVMX_XCV_BATCH_CRD_RET_FUNC()
static inline uint64_t CVMX_XCV_BATCH_CRD_RET_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_XCV_BATCH_CRD_RET not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DB000100ull);
}
#else
#define CVMX_XCV_BATCH_CRD_RET (CVMX_ADD_IO_SEG(0x00011800DB000100ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_XCV_COMP_CTL CVMX_XCV_COMP_CTL_FUNC()
static inline uint64_t CVMX_XCV_COMP_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_XCV_COMP_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DB000020ull);
}
#else
#define CVMX_XCV_COMP_CTL (CVMX_ADD_IO_SEG(0x00011800DB000020ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_XCV_CTL CVMX_XCV_CTL_FUNC()
static inline uint64_t CVMX_XCV_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_XCV_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DB000030ull);
}
#else
#define CVMX_XCV_CTL (CVMX_ADD_IO_SEG(0x00011800DB000030ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_XCV_DLL_CTL CVMX_XCV_DLL_CTL_FUNC()
static inline uint64_t CVMX_XCV_DLL_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_XCV_DLL_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DB000010ull);
}
#else
#define CVMX_XCV_DLL_CTL (CVMX_ADD_IO_SEG(0x00011800DB000010ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_XCV_ECO CVMX_XCV_ECO_FUNC()
static inline uint64_t CVMX_XCV_ECO_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_XCV_ECO not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DB000200ull);
}
#else
#define CVMX_XCV_ECO (CVMX_ADD_IO_SEG(0x00011800DB000200ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_XCV_INBND_STATUS CVMX_XCV_INBND_STATUS_FUNC()
static inline uint64_t CVMX_XCV_INBND_STATUS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_XCV_INBND_STATUS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DB000080ull);
}
#else
#define CVMX_XCV_INBND_STATUS (CVMX_ADD_IO_SEG(0x00011800DB000080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_XCV_INT CVMX_XCV_INT_FUNC()
static inline uint64_t CVMX_XCV_INT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_XCV_INT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DB000040ull);
}
#else
#define CVMX_XCV_INT (CVMX_ADD_IO_SEG(0x00011800DB000040ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_XCV_RESET CVMX_XCV_RESET_FUNC()
static inline uint64_t CVMX_XCV_RESET_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX)))
		cvmx_warn("CVMX_XCV_RESET not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800DB000000ull);
}
#else
#define CVMX_XCV_RESET (CVMX_ADD_IO_SEG(0x00011800DB000000ull))
#endif

/**
 * cvmx_xcv_batch_crd_ret
 */
union cvmx_xcv_batch_crd_ret {
	uint64_t u64;
	struct cvmx_xcv_batch_crd_ret_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t crd_ret                      : 1;  /**< In case of the reset event, when this register is written XCV sends out the
                                                         initial credit batch to BGX. Initial credit value of 16. The write will only
                                                         take effect when XCV_RESET[ENABLE] is set. */
#else
	uint64_t crd_ret                      : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_xcv_batch_crd_ret_s       cn73xx;
};
typedef union cvmx_xcv_batch_crd_ret cvmx_xcv_batch_crd_ret_t;

/**
 * cvmx_xcv_comp_ctl
 *
 * This register controls programmable compensation.
 *
 */
union cvmx_xcv_comp_ctl {
	uint64_t u64;
	struct cvmx_xcv_comp_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t drv_byp                      : 1;  /**< When set, bypass the compensation controller and use
                                                         DRV_NCTL and DRV_PCTL. */
	uint64_t reserved_61_62               : 2;
	uint64_t cmp_pctl                     : 5;  /**< PCTL drive strength from the hardware compensation controller. */
	uint64_t reserved_53_55               : 3;
	uint64_t cmp_nctl                     : 5;  /**< NCTL drive strength from the hardware compensation controller. */
	uint64_t reserved_45_47               : 3;
	uint64_t drv_pctl                     : 5;  /**< PCTL drive strength to use in bypass mode.
                                                         Value of 11 is for 50 ohm termination. */
	uint64_t reserved_37_39               : 3;
	uint64_t drv_nctl                     : 5;  /**< NCTL drive strength to use in bypass mode.
                                                         Value of 14 is for 50 ohm termination. */
	uint64_t reserved_31_31               : 1;
	uint64_t pctl_lock                    : 1;  /**< PCTL lock. */
	uint64_t pctl_sat                     : 1;  /**< PCTL saturate. */
	uint64_t reserved_28_28               : 1;
	uint64_t nctl_lock                    : 1;  /**< NCTL lock. */
	uint64_t reserved_1_26                : 26;
	uint64_t nctl_sat                     : 1;  /**< NCTL saturate. */
#else
	uint64_t nctl_sat                     : 1;
	uint64_t reserved_1_26                : 26;
	uint64_t nctl_lock                    : 1;
	uint64_t reserved_28_28               : 1;
	uint64_t pctl_sat                     : 1;
	uint64_t pctl_lock                    : 1;
	uint64_t reserved_31_31               : 1;
	uint64_t drv_nctl                     : 5;
	uint64_t reserved_37_39               : 3;
	uint64_t drv_pctl                     : 5;
	uint64_t reserved_45_47               : 3;
	uint64_t cmp_nctl                     : 5;
	uint64_t reserved_53_55               : 3;
	uint64_t cmp_pctl                     : 5;
	uint64_t reserved_61_62               : 2;
	uint64_t drv_byp                      : 1;
#endif
	} s;
	struct cvmx_xcv_comp_ctl_s            cn73xx;
};
typedef union cvmx_xcv_comp_ctl cvmx_xcv_comp_ctl_t;

/**
 * cvmx_xcv_ctl
 *
 * This register contains the status control bits.
 *
 */
union cvmx_xcv_ctl {
	uint64_t u64;
	struct cvmx_xcv_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t lpbk_ext                     : 1;  /**< Enable external loopback mode. External loopback loops the RX datapath to the TX
                                                         datapath. For correct operation, the following CSRs must be configured:
                                                         * XCV_CTL[SPEED]          = 0x2.
                                                         * XCV_DLL_CTL[REFCLK_SEL] = 1.
                                                         * XCV_RESET[CLKRST]       = 1. */
	uint64_t lpbk_int                     : 1;  /**< Enable internal loopback mode. Internal loopback loops the TX datapath to the RX
                                                         datapath. For correct operation, the following CSRs must be configured:
                                                         * XCV_CTL[SPEED]          = 0x2.
                                                         * XCV_DLL_CTL[REFCLK_SEL] = 0.
                                                         * XCV_RESET[CLKRST]       = 0. */
	uint64_t speed                        : 2;  /**< XCV operational speed:
                                                         0x0 = 10 Mbps.
                                                         0x1 = 100 Mbps.
                                                         0x2 = 1Gbps.
                                                         0x3 = Reserved. */
#else
	uint64_t speed                        : 2;
	uint64_t lpbk_int                     : 1;
	uint64_t lpbk_ext                     : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_xcv_ctl_s                 cn73xx;
};
typedef union cvmx_xcv_ctl cvmx_xcv_ctl_t;

/**
 * cvmx_xcv_dll_ctl
 *
 * The RGMII timing specification requires that devices transmit clock and
 * data synchronously. The specification requires external sources (namely
 * the PC board trace routes) to introduce the appropriate 1.5 to 2.0 ns of
 * delay.
 *
 * To eliminate the need for the PC board delays, the RGMII interface has optional
 * on-board DLLs for both transmit and receive. For correct operation, at most one
 * of the transmitter, board, or receiver involved in an RGMII link should
 * introduce delay. By default/reset, the RGMII receivers delay the received clock,
 * and the RGMII transmitters do not delay the transmitted clock. Whether this
 * default works as-is with a given link partner depends on the behavior of the
 * link partner and the PC board.
 *
 * These are the possible modes of RGMII receive operation:
 *
 * * XCV_DLL_CTL[CLKRX_BYP] = 0 (reset value) - The RGMII
 * receive interface introduces clock delay using its internal DLL.
 * This mode is appropriate if neither the remote
 * transmitter nor the PC board delays the clock.
 *
 * * XCV_DLL_CTL[CLKRX_BYP] = 1, [CLKRX_SET] = 0x0 - The
 * RGMII receive interface introduces no clock delay. This mode
 * is appropriate if either the remote transmitter or the PC board
 * delays the clock.
 *
 * These are the possible modes of RGMII transmit operation:
 *
 * * XCV_DLL_CTL[CLKTX_BYP] = 1, [CLKTX_SET] = 0x0 (reset value) -
 * The RGMII transmit interface introduces no clock
 * delay. This mode is appropriate is either the remote receiver
 * or the PC board delays the clock.
 *
 * * XCV_DLL_CTL[CLKTX_BYP] = 0 - The RGMII transmit
 * interface introduces clock delay using its internal DLL.
 * This mode is appropriate if neither the remote receiver
 * nor the PC board delays the clock.
 */
union cvmx_xcv_dll_ctl {
	uint64_t u64;
	struct cvmx_xcv_dll_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t lock                         : 1;  /**< Reserved. */
	uint64_t clk_set                      : 7;  /**< The clock delay as determined by the on-board hardware DLL. */
	uint64_t clkrx_byp                    : 1;  /**< Bypass the RX clock delay setting.
                                                         Skews RXC from RXD, RXCTL.
                                                         By default, hardware internally shifts the RXC clock
                                                         to sample RXD,RXCTL assuming clock and data and
                                                         sourced synchronously from the link partner. */
	uint64_t clkrx_set                    : 7;  /**< RX clock delay setting to use in bypass mode.
                                                         Skews RXC from RXD. */
	uint64_t clktx_byp                    : 1;  /**< Bypass the TX clock delay setting.
                                                         Skews TXC from TXD, TXCTL.
                                                         By default, clock and data and sourced
                                                         synchronously. */
	uint64_t clktx_set                    : 7;  /**< TX clock delay setting to use in bypass mode.
                                                         Skews TXC from TXD. */
	uint64_t reserved_2_7                 : 6;
	uint64_t refclk_sel                   : 2;  /**< Select the reference clock to use.  Normal RGMII specification requires a 125MHz
                                                         oscillator.
                                                         To reduce system cost, a 500MHz coprocessor clock can be divided down and remove the
                                                         requirements for the external oscillator. Additionally, in some well defined systems, the
                                                         link partner may be able to source the RXC. The RGMII would operate correctly in 1000Mbs
                                                         mode only.
                                                         0x0 = RGMII REFCLK.
                                                         0x1 = RGMII RXC (1000Mbs only).
                                                         0x2 = Divided coprocessor clock.
                                                         0x3 = Reserved. */
#else
	uint64_t refclk_sel                   : 2;
	uint64_t reserved_2_7                 : 6;
	uint64_t clktx_set                    : 7;
	uint64_t clktx_byp                    : 1;
	uint64_t clkrx_set                    : 7;
	uint64_t clkrx_byp                    : 1;
	uint64_t clk_set                      : 7;
	uint64_t lock                         : 1;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_xcv_dll_ctl_s             cn73xx;
};
typedef union cvmx_xcv_dll_ctl cvmx_xcv_dll_ctl_t;

/**
 * cvmx_xcv_eco
 */
union cvmx_xcv_eco {
	uint64_t u64;
	struct cvmx_xcv_eco_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t eco_rw                       : 16; /**< N/A */
#else
	uint64_t eco_rw                       : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_xcv_eco_s                 cn73xx;
};
typedef union cvmx_xcv_eco cvmx_xcv_eco_t;

/**
 * cvmx_xcv_inbnd_status
 *
 * This register contains RGMII in-band status.
 *
 */
union cvmx_xcv_inbnd_status {
	uint64_t u64;
	struct cvmx_xcv_inbnd_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t duplex                       : 1;  /**< RGMII in-band status - link duplex:
                                                         0 = Half-duplex.
                                                         1 = Full-duplex. */
	uint64_t speed                        : 2;  /**< RGMII in-band status - link speed:
                                                         0x0 = 10 Mbps.
                                                         0x1 = 100 Mbps.
                                                         0x2 = 1000 Mbps.
                                                         0x3 = Reserved. */
	uint64_t link                         : 1;  /**< RGMII in-band status - link enable/up:
                                                         0 = Link down.
                                                         1 = Link up. */
#else
	uint64_t link                         : 1;
	uint64_t speed                        : 2;
	uint64_t duplex                       : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_xcv_inbnd_status_s        cn73xx;
};
typedef union cvmx_xcv_inbnd_status cvmx_xcv_inbnd_status_t;

/**
 * cvmx_xcv_int
 *
 * This register controls interrupts.
 *
 */
union cvmx_xcv_int {
	uint64_t u64;
	struct cvmx_xcv_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t tx_ovrflw                    : 1;  /**< TX FIFO overflow. */
	uint64_t tx_undflw                    : 1;  /**< TX FIFO underflow. */
	uint64_t incomp_byte                  : 1;  /**< Flags the incomplete byte cases for 10/100 mode. */
	uint64_t duplex                       : 1;  /**< Flags the in-band status change on link duplex. */
	uint64_t reserved_2_2                 : 1;
	uint64_t speed                        : 1;  /**< Flags the in-band status change on link speed. */
	uint64_t link                         : 1;  /**< Flags the in-band status change on link up/down status. */
#else
	uint64_t link                         : 1;
	uint64_t speed                        : 1;
	uint64_t reserved_2_2                 : 1;
	uint64_t duplex                       : 1;
	uint64_t incomp_byte                  : 1;
	uint64_t tx_undflw                    : 1;
	uint64_t tx_ovrflw                    : 1;
	uint64_t reserved_7_63                : 57;
#endif
	} s;
	struct cvmx_xcv_int_s                 cn73xx;
};
typedef union cvmx_xcv_int cvmx_xcv_int_t;

/**
 * cvmx_xcv_reset
 *
 * This register controls reset.
 *
 */
union cvmx_xcv_reset {
	uint64_t u64;
	struct cvmx_xcv_reset_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t enable                       : 1;  /**< Port enable. */
	uint64_t reserved_16_62               : 47;
	uint64_t clkrst                       : 1;  /**< DLL CLK reset.  CLKRST must be set if DLL bypass mode
                                                         XCV_DLL_CTL[CLKRX_BYP,CLKTX_BYP] is used. */
	uint64_t reserved_12_14               : 3;
	uint64_t dllrst                       : 1;  /**< DLL reset. */
	uint64_t reserved_8_10                : 3;
	uint64_t comp                         : 1;  /**< Compensation enable. */
	uint64_t reserved_4_6                 : 3;
	uint64_t tx_pkt_rst_n                 : 1;  /**< Packet reset for TX. */
	uint64_t tx_dat_rst_n                 : 1;  /**< Datapath reset for TX. */
	uint64_t rx_pkt_rst_n                 : 1;  /**< Packet reset for RX. */
	uint64_t rx_dat_rst_n                 : 1;  /**< Datapath reset for RX. */
#else
	uint64_t rx_dat_rst_n                 : 1;
	uint64_t rx_pkt_rst_n                 : 1;
	uint64_t tx_dat_rst_n                 : 1;
	uint64_t tx_pkt_rst_n                 : 1;
	uint64_t reserved_4_6                 : 3;
	uint64_t comp                         : 1;
	uint64_t reserved_8_10                : 3;
	uint64_t dllrst                       : 1;
	uint64_t reserved_12_14               : 3;
	uint64_t clkrst                       : 1;
	uint64_t reserved_16_62               : 47;
	uint64_t enable                       : 1;
#endif
	} s;
	struct cvmx_xcv_reset_s               cn73xx;
};
typedef union cvmx_xcv_reset cvmx_xcv_reset_t;

#endif
