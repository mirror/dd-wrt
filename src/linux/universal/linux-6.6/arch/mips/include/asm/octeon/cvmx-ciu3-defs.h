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
 * cvmx-ciu3-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon ciu3.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_CIU3_DEFS_H__
#define __CVMX_CIU3_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU3_BIST CVMX_CIU3_BIST_FUNC()
static inline uint64_t CVMX_CIU3_BIST_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_CIU3_BIST not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00010100000001C0ull);
}
#else
#define CVMX_CIU3_BIST (CVMX_ADD_IO_SEG(0x00010100000001C0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU3_CONST CVMX_CIU3_CONST_FUNC()
static inline uint64_t CVMX_CIU3_CONST_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_CIU3_CONST not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001010000000220ull);
}
#else
#define CVMX_CIU3_CONST (CVMX_ADD_IO_SEG(0x0001010000000220ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU3_CTL CVMX_CIU3_CTL_FUNC()
static inline uint64_t CVMX_CIU3_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_CIU3_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00010100000000E0ull);
}
#else
#define CVMX_CIU3_CTL (CVMX_ADD_IO_SEG(0x00010100000000E0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU3_DESTX_IO_INT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 4))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 4))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 4))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 4)))))
		cvmx_warn("CVMX_CIU3_DESTX_IO_INT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001010000210000ull) + ((offset) & 7) * 8;
}
#else
#define CVMX_CIU3_DESTX_IO_INT(offset) (CVMX_ADD_IO_SEG(0x0001010000210000ull) + ((offset) & 7) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU3_DESTX_PP_INT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 47))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 143))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 143))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 47)))))
		cvmx_warn("CVMX_CIU3_DESTX_PP_INT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001010000200000ull) + ((offset) & 255) * 8;
}
#else
#define CVMX_CIU3_DESTX_PP_INT(offset) (CVMX_ADD_IO_SEG(0x0001010000200000ull) + ((offset) & 255) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU3_GSTOP CVMX_CIU3_GSTOP_FUNC()
static inline uint64_t CVMX_CIU3_GSTOP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_CIU3_GSTOP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001010000000140ull);
}
#else
#define CVMX_CIU3_GSTOP (CVMX_ADD_IO_SEG(0x0001010000000140ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU3_IDTX_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 255)))))
		cvmx_warn("CVMX_CIU3_IDTX_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001010000110000ull) + ((offset) & 255) * 8;
}
#else
#define CVMX_CIU3_IDTX_CTL(offset) (CVMX_ADD_IO_SEG(0x0001010000110000ull) + ((offset) & 255) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU3_IDTX_IO(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 255)))))
		cvmx_warn("CVMX_CIU3_IDTX_IO(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001010000130000ull) + ((offset) & 255) * 8;
}
#else
#define CVMX_CIU3_IDTX_IO(offset) (CVMX_ADD_IO_SEG(0x0001010000130000ull) + ((offset) & 255) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU3_IDTX_PPX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset == 0)) && ((block_id <= 255)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset == 0)) && ((block_id <= 255)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset == 0)) && ((block_id <= 255)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset == 0)) && ((block_id <= 255))))))
		cvmx_warn("CVMX_CIU3_IDTX_PPX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001010000120000ull) + ((block_id) & 255) * 0x20ull;
}
#else
#define CVMX_CIU3_IDTX_PPX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001010000120000ull) + ((block_id) & 255) * 0x20ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU3_INTR_RAM_ECC_CTL CVMX_CIU3_INTR_RAM_ECC_CTL_FUNC()
static inline uint64_t CVMX_CIU3_INTR_RAM_ECC_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_CIU3_INTR_RAM_ECC_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001010000000260ull);
}
#else
#define CVMX_CIU3_INTR_RAM_ECC_CTL (CVMX_ADD_IO_SEG(0x0001010000000260ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU3_INTR_RAM_ECC_ST CVMX_CIU3_INTR_RAM_ECC_ST_FUNC()
static inline uint64_t CVMX_CIU3_INTR_RAM_ECC_ST_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_CIU3_INTR_RAM_ECC_ST not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001010000000280ull);
}
#else
#define CVMX_CIU3_INTR_RAM_ECC_ST (CVMX_ADD_IO_SEG(0x0001010000000280ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU3_INTR_READY CVMX_CIU3_INTR_READY_FUNC()
static inline uint64_t CVMX_CIU3_INTR_READY_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_CIU3_INTR_READY not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00010100000002A0ull);
}
#else
#define CVMX_CIU3_INTR_READY (CVMX_ADD_IO_SEG(0x00010100000002A0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU3_INTR_SLOWDOWN CVMX_CIU3_INTR_SLOWDOWN_FUNC()
static inline uint64_t CVMX_CIU3_INTR_SLOWDOWN_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_CIU3_INTR_SLOWDOWN not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001010000000240ull);
}
#else
#define CVMX_CIU3_INTR_SLOWDOWN (CVMX_ADD_IO_SEG(0x0001010000000240ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU3_ISCX_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1048575))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1048575))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1048575))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1048575)))))
		cvmx_warn("CVMX_CIU3_ISCX_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001010080000000ull) + ((offset) & 1048575) * 8;
}
#else
#define CVMX_CIU3_ISCX_CTL(offset) (CVMX_ADD_IO_SEG(0x0001010080000000ull) + ((offset) & 1048575) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU3_ISCX_W1C(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1048575))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1048575))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1048575))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1048575)))))
		cvmx_warn("CVMX_CIU3_ISCX_W1C(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001010090000000ull) + ((offset) & 1048575) * 8;
}
#else
#define CVMX_CIU3_ISCX_W1C(offset) (CVMX_ADD_IO_SEG(0x0001010090000000ull) + ((offset) & 1048575) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU3_ISCX_W1S(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1048575))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1048575))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1048575))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1048575)))))
		cvmx_warn("CVMX_CIU3_ISCX_W1S(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00010100A0000000ull) + ((offset) & 1048575) * 8;
}
#else
#define CVMX_CIU3_ISCX_W1S(offset) (CVMX_ADD_IO_SEG(0x00010100A0000000ull) + ((offset) & 1048575) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU3_NMI CVMX_CIU3_NMI_FUNC()
static inline uint64_t CVMX_CIU3_NMI_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_CIU3_NMI not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001010000000160ull);
}
#else
#define CVMX_CIU3_NMI (CVMX_ADD_IO_SEG(0x0001010000000160ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU3_SISCX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 191))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 191))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 191))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 191)))))
		cvmx_warn("CVMX_CIU3_SISCX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001010000220000ull) + ((offset) & 255) * 8;
}
#else
#define CVMX_CIU3_SISCX(offset) (CVMX_ADD_IO_SEG(0x0001010000220000ull) + ((offset) & 255) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU3_TIMX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 9))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 9))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 9))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 9)))))
		cvmx_warn("CVMX_CIU3_TIMX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001010000010000ull) + ((offset) & 15) * 8;
}
#else
#define CVMX_CIU3_TIMX(offset) (CVMX_ADD_IO_SEG(0x0001010000010000ull) + ((offset) & 15) * 8)
#endif

/**
 * cvmx_ciu3_bist
 */
union cvmx_ciu3_bist {
	uint64_t u64;
	struct cvmx_ciu3_bist_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t bist                         : 9;  /**< BIST results. Hardware sets a bit for each memory that fails BIST. */
#else
	uint64_t bist                         : 9;
	uint64_t reserved_9_63                : 55;
#endif
	} s;
	struct cvmx_ciu3_bist_s               cn73xx;
	struct cvmx_ciu3_bist_s               cn78xx;
	struct cvmx_ciu3_bist_s               cn78xxp1;
	struct cvmx_ciu3_bist_s               cnf75xx;
};
typedef union cvmx_ciu3_bist cvmx_ciu3_bist_t;

/**
 * cvmx_ciu3_const
 */
union cvmx_ciu3_const {
	uint64_t u64;
	struct cvmx_ciu3_const_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dests_io                     : 16; /**< Number of entries in CIU3_DEST(0..4)_IO_INT. */
	uint64_t pintsn                       : 16; /**< Physical INTSNs implemented. */
	uint64_t dests_pp                     : 16; /**< Number of entries in CIU3_DEST(0..47)_PP_INT. */
	uint64_t idt                          : 16; /**< Number of entries in CIU3_IDT(0..255)_CTL. */
#else
	uint64_t idt                          : 16;
	uint64_t dests_pp                     : 16;
	uint64_t pintsn                       : 16;
	uint64_t dests_io                     : 16;
#endif
	} s;
	struct cvmx_ciu3_const_s              cn73xx;
	struct cvmx_ciu3_const_s              cn78xx;
	struct cvmx_ciu3_const_s              cn78xxp1;
	struct cvmx_ciu3_const_s              cnf75xx;
};
typedef union cvmx_ciu3_const cvmx_ciu3_const_t;

/**
 * cvmx_ciu3_ctl
 */
union cvmx_ciu3_ctl {
	uint64_t u64;
	struct cvmx_ciu3_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t mcd_sel                      : 2;  /**< When a MCD interrupt is requested via the IDT, which MCD number to pulse:
                                                         0x0 = MCD0.
                                                         0x1 = MCD1.
                                                         0x2 = MCD2.
                                                         0x3 = Reserved. */
	uint64_t iscmem_le                    : 1;  /**< Reserved. */
	uint64_t seq_dis                      : 1;  /**< Disable running sequencer only when required to reduce power, and run continuously. For
                                                         diagnostic use only. */
	uint64_t cclk_dis                     : 1;  /**< Disable power saving conditional clocking. */
#else
	uint64_t cclk_dis                     : 1;
	uint64_t seq_dis                      : 1;
	uint64_t iscmem_le                    : 1;
	uint64_t mcd_sel                      : 2;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_ciu3_ctl_s                cn73xx;
	struct cvmx_ciu3_ctl_s                cn78xx;
	struct cvmx_ciu3_ctl_s                cn78xxp1;
	struct cvmx_ciu3_ctl_s                cnf75xx;
};
typedef union cvmx_ciu3_ctl cvmx_ciu3_ctl_t;

/**
 * cvmx_ciu3_dest#_io_int
 *
 * This register contains reduced interrupt source numbers for delivery to software, indexed by
 * I/O bridge number. Fields are identical to CIU3_DEST()_PP_INT.
 *
 * None of CIU3_DEST(CIU_DEST_IO_E::SRIO(*))_IO_INT should be used.
 */
union cvmx_ciu3_destx_io_int {
	uint64_t u64;
	struct cvmx_ciu3_destx_io_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_52_63               : 12;
	uint64_t intsn                        : 20; /**< Interrupt source number causing the current interrupt, or most recent interrupt if INTR is
                                                         clear. Note this field is not stored in the DEST ram itself; it is instead read from
                                                         CIU3_IDT()_CTL[INTSN]. */
	uint64_t reserved_10_31               : 22;
	uint64_t intidt                       : 8;  /**< IDT entry number causing the current interrupt, or most recent interrupt if INTR is clear. */
	uint64_t newint                       : 1;  /**< New interrupt to be delivered. Internal state, for diagnostic use only. */
	uint64_t intr                         : 1;  /**< Interrupt pending. This bit is recalculated when CIU3_ISC()_CTL or interrupts
                                                         change, so does not need to be cleared by software. */
#else
	uint64_t intr                         : 1;
	uint64_t newint                       : 1;
	uint64_t intidt                       : 8;
	uint64_t reserved_10_31               : 22;
	uint64_t intsn                        : 20;
	uint64_t reserved_52_63               : 12;
#endif
	} s;
	struct cvmx_ciu3_destx_io_int_s       cn73xx;
	struct cvmx_ciu3_destx_io_int_s       cn78xx;
	struct cvmx_ciu3_destx_io_int_s       cn78xxp1;
	struct cvmx_ciu3_destx_io_int_s       cnf75xx;
};
typedef union cvmx_ciu3_destx_io_int cvmx_ciu3_destx_io_int_t;

/**
 * cvmx_ciu3_dest#_pp_int
 *
 * This register contains reduced interrupt source numbers for delivery to software, indexed by
 * CIU_DEST_E.
 */
union cvmx_ciu3_destx_pp_int {
	uint64_t u64;
	struct cvmx_ciu3_destx_pp_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_52_63               : 12;
	uint64_t intsn                        : 20; /**< Interrupt source number causing the current interrupt, or most recent interrupt if INTR is
                                                         clear. Note this field is not stored in the DEST ram itself; it is instead read from
                                                         CIU3_IDT()_CTL[INTSN]. */
	uint64_t reserved_10_31               : 22;
	uint64_t intidt                       : 8;  /**< IDT entry number causing the current interrupt, or most recent interrupt if INTR is clear. */
	uint64_t newint                       : 1;  /**< New interrupt to be delivered. Internal state, for diagnostic use only. */
	uint64_t intr                         : 1;  /**< Interrupt pending. This bit is recalculated when CIU3_ISC()_CTL or interrupts
                                                         change, so does not need to be cleared by software. */
#else
	uint64_t intr                         : 1;
	uint64_t newint                       : 1;
	uint64_t intidt                       : 8;
	uint64_t reserved_10_31               : 22;
	uint64_t intsn                        : 20;
	uint64_t reserved_52_63               : 12;
#endif
	} s;
	struct cvmx_ciu3_destx_pp_int_s       cn73xx;
	struct cvmx_ciu3_destx_pp_int_s       cn78xx;
	struct cvmx_ciu3_destx_pp_int_s       cn78xxp1;
	struct cvmx_ciu3_destx_pp_int_s       cnf75xx;
};
typedef union cvmx_ciu3_destx_pp_int cvmx_ciu3_destx_pp_int_t;

/**
 * cvmx_ciu3_gstop
 */
union cvmx_ciu3_gstop {
	uint64_t u64;
	struct cvmx_ciu3_gstop_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t gstop                        : 1;  /**< Set global-stop mode. */
#else
	uint64_t gstop                        : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_ciu3_gstop_s              cn73xx;
	struct cvmx_ciu3_gstop_s              cn78xx;
	struct cvmx_ciu3_gstop_s              cn78xxp1;
	struct cvmx_ciu3_gstop_s              cnf75xx;
};
typedef union cvmx_ciu3_gstop cvmx_ciu3_gstop_t;

/**
 * cvmx_ciu3_idt#_ctl
 *
 * Entry zero of the IDT is reserved to mean 'no interrupt' and is not writable by software.
 *
 */
union cvmx_ciu3_idtx_ctl {
	uint64_t u64;
	struct cvmx_ciu3_idtx_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_52_63               : 12;
	uint64_t intsn                        : 20; /**< Interrupt source number causing the current interrupt, or most recent interrupt if INTR is
                                                         clear. */
	uint64_t reserved_4_31                : 28;
	uint64_t intr                         : 1;  /**< Interrupt pending */
	uint64_t newint                       : 1;  /**< New interrupt to be delivered. Internal state, for diagnostic use only. */
	uint64_t ip_num                       : 2;  /**< Destination interrupt priority level to receive this interrupt. Only used for core
                                                         interrupts; for IO interrupts this level must be zero.
                                                         0x0 = IP2, or I/O interrupt.
                                                         0x1 = IP3.
                                                         0x2 = IP4.
                                                         0x3 = Reserved. */
#else
	uint64_t ip_num                       : 2;
	uint64_t newint                       : 1;
	uint64_t intr                         : 1;
	uint64_t reserved_4_31                : 28;
	uint64_t intsn                        : 20;
	uint64_t reserved_52_63               : 12;
#endif
	} s;
	struct cvmx_ciu3_idtx_ctl_s           cn73xx;
	struct cvmx_ciu3_idtx_ctl_s           cn78xx;
	struct cvmx_ciu3_idtx_ctl_s           cn78xxp1;
	struct cvmx_ciu3_idtx_ctl_s           cnf75xx;
};
typedef union cvmx_ciu3_idtx_ctl cvmx_ciu3_idtx_ctl_t;

/**
 * cvmx_ciu3_idt#_io
 *
 * Entry zero of the IDT is reserved to mean 'no interrupt' and is not writable by software.
 *
 */
union cvmx_ciu3_idtx_io {
	uint64_t u64;
	struct cvmx_ciu3_idtx_io_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t io                           : 5;  /**< Bitmask of which IO bridges or MCD to receive interrupts via this IDT.
                                                         Enumerated by CIU_DEST_IO_E.
                                                         All of bits [IO<CIU_DEST_IO_E::SRIO(*)>] must be clear. */
#else
	uint64_t io                           : 5;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_ciu3_idtx_io_s            cn73xx;
	struct cvmx_ciu3_idtx_io_s            cn78xx;
	struct cvmx_ciu3_idtx_io_s            cn78xxp1;
	struct cvmx_ciu3_idtx_io_s            cnf75xx;
};
typedef union cvmx_ciu3_idtx_io cvmx_ciu3_idtx_io_t;

/**
 * cvmx_ciu3_idt#_pp#
 *
 * Entry zero of the IDT is reserved to mean 'no interrupt' and is not writable by software. The
 * second (PP) index in this register is always zero to allow expansion beyond 64 cores.
 */
union cvmx_ciu3_idtx_ppx {
	uint64_t u64;
	struct cvmx_ciu3_idtx_ppx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t pp                           : 48; /**< Bitmask of which cores receive interrupts via this IDT. */
#else
	uint64_t pp                           : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_ciu3_idtx_ppx_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t pp                           : 16; /**< Bitmask of which cores receive interrupts via this IDT. */
#else
	uint64_t pp                           : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} cn73xx;
	struct cvmx_ciu3_idtx_ppx_s           cn78xx;
	struct cvmx_ciu3_idtx_ppx_s           cn78xxp1;
	struct cvmx_ciu3_idtx_ppx_cn73xx      cnf75xx;
};
typedef union cvmx_ciu3_idtx_ppx cvmx_ciu3_idtx_ppx_t;

/**
 * cvmx_ciu3_intr_ram_ecc_ctl
 */
union cvmx_ciu3_intr_ram_ecc_ctl {
	uint64_t u64;
	struct cvmx_ciu3_intr_ram_ecc_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t flip_synd                    : 2;  /**< Testing feature. Flip syndrome to generate single-bit or double-bit errors. FLIP_SYND<0>
                                                         generates even numbered bits errors; FLIP_SYND<1> generates odd bits errors. */
	uint64_t ecc_ena                      : 1;  /**< ECC enable. When set, enables the 9-bit ECC check/correct logic for CIU interrupt-enable
                                                         RAM. With ECC enabled, the ECC code is generated and written in memory, and then later on
                                                         reads, is used to check and correct single-bit errors and detect double-bit errors. */
#else
	uint64_t ecc_ena                      : 1;
	uint64_t flip_synd                    : 2;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_ciu3_intr_ram_ecc_ctl_s   cn73xx;
	struct cvmx_ciu3_intr_ram_ecc_ctl_s   cn78xx;
	struct cvmx_ciu3_intr_ram_ecc_ctl_s   cn78xxp1;
	struct cvmx_ciu3_intr_ram_ecc_ctl_s   cnf75xx;
};
typedef union cvmx_ciu3_intr_ram_ecc_ctl cvmx_ciu3_intr_ram_ecc_ctl_t;

/**
 * cvmx_ciu3_intr_ram_ecc_st
 */
union cvmx_ciu3_intr_ram_ecc_st {
	uint64_t u64;
	struct cvmx_ciu3_intr_ram_ecc_st_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_52_63               : 12;
	uint64_t addr                         : 20; /**< Latch the address for latest SBE/DBE that occurred. */
	uint64_t reserved_6_31                : 26;
	uint64_t sisc_dbe                     : 1;  /**< SISC Double-bit error observed. Throws CIU_INTSN_E::CIU_ECC_SISC_DBE. */
	uint64_t sisc_sbe                     : 1;  /**< SISC Single-bit error observed. Throws CIU_INTSN_E::CIU_ECC_SISC_SBE. */
	uint64_t idt_dbe                      : 1;  /**< IDT Double-bit error observed. Throws CIU_INTSN_E::CIU_ECC_IDT_DBE. */
	uint64_t idt_sbe                      : 1;  /**< IDT Single-bit error observed. Throws CIU_INTSN_E::CIU_ECC_IDT_SBE. */
	uint64_t isc_dbe                      : 1;  /**< ISC Double-bit error observed. Throws CIU_INTSN_E::CIU_ECC_ISC_DBE. */
	uint64_t isc_sbe                      : 1;  /**< ISC Single-bit error observed. Throws CIU_INTSN_E::CIU_ECC_ISC_SBE. */
#else
	uint64_t isc_sbe                      : 1;
	uint64_t isc_dbe                      : 1;
	uint64_t idt_sbe                      : 1;
	uint64_t idt_dbe                      : 1;
	uint64_t sisc_sbe                     : 1;
	uint64_t sisc_dbe                     : 1;
	uint64_t reserved_6_31                : 26;
	uint64_t addr                         : 20;
	uint64_t reserved_52_63               : 12;
#endif
	} s;
	struct cvmx_ciu3_intr_ram_ecc_st_s    cn73xx;
	struct cvmx_ciu3_intr_ram_ecc_st_s    cn78xx;
	struct cvmx_ciu3_intr_ram_ecc_st_s    cn78xxp1;
	struct cvmx_ciu3_intr_ram_ecc_st_s    cnf75xx;
};
typedef union cvmx_ciu3_intr_ram_ecc_st cvmx_ciu3_intr_ram_ecc_st_t;

/**
 * cvmx_ciu3_intr_ready
 */
union cvmx_ciu3_intr_ready {
	uint64_t u64;
	struct cvmx_ciu3_intr_ready_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_46_63               : 18;
	uint64_t index                        : 14; /**< Scanner index. If [READY] set, the current index, else the index the scanner stopped at.
                                                         For diagnostic use only. */
	uint64_t reserved_1_31                : 31;
	uint64_t ready                        : 1;  /**< CIU background scanner is running. If set, CIU is performing a background scan searching
                                                         for
                                                         secondary interrupts. Write one to force a new scan. For diagnostic use only. */
#else
	uint64_t ready                        : 1;
	uint64_t reserved_1_31                : 31;
	uint64_t index                        : 14;
	uint64_t reserved_46_63               : 18;
#endif
	} s;
	struct cvmx_ciu3_intr_ready_s         cn73xx;
	struct cvmx_ciu3_intr_ready_s         cn78xx;
	struct cvmx_ciu3_intr_ready_s         cn78xxp1;
	struct cvmx_ciu3_intr_ready_s         cnf75xx;
};
typedef union cvmx_ciu3_intr_ready cvmx_ciu3_intr_ready_t;

/**
 * cvmx_ciu3_intr_slowdown
 */
union cvmx_ciu3_intr_slowdown {
	uint64_t u64;
	struct cvmx_ciu3_intr_slowdown_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t ctl                          : 3;  /**< Slow down CIU interrupt walker processing time. IRQ2/3/4 for all cores are sent to the
                                                         core (MRC) in a serial bus to reduce global routing. There is no backpressure mechanism
                                                         designed for this scheme. It will only be a problem when SCLK is faster; this control will
                                                         process 1 interrupt in 4*(2^CTL) SCLK cycles. For example:
                                                         0x0 = sclk_freq/aclk_freq ratio is 4.
                                                         0x1 = sclk_freq/aclk_freq ratio is 8. */
#else
	uint64_t ctl                          : 3;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_ciu3_intr_slowdown_s      cn73xx;
	struct cvmx_ciu3_intr_slowdown_s      cn78xx;
	struct cvmx_ciu3_intr_slowdown_s      cn78xxp1;
	struct cvmx_ciu3_intr_slowdown_s      cnf75xx;
};
typedef union cvmx_ciu3_intr_slowdown cvmx_ciu3_intr_slowdown_t;

/**
 * cvmx_ciu3_isc#_ctl
 *
 * Sparse table indexed by INTSN.
 *
 */
union cvmx_ciu3_iscx_ctl {
	uint64_t u64;
	struct cvmx_ciu3_iscx_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t idt                          : 8;  /**< Interrupt delivery table entry number. Zero indicates IDT delivery is disabled. This field
                                                         may only be changed when EN was previously clear, though it may be changed with the same
                                                         write that sets EN. Thus if EN is set, to change IDT two register writes are required, the
                                                         first to clear EN (perhaps by a store to CIU3_ISC()_W1C), and the second to make
                                                         the change to IDT. */
	uint64_t imp                          : 1;  /**< Entry implemented. Although the table has 1M entries, most of those do not correspond to
                                                         any INTSN, and as such are not implemented.
                                                         1 = The IDT and EN fields for this index are R/W, and may have a corresponding INTSN,
                                                         although some indices may have IMP set but not have any INTSN use.
                                                         0 = The IDT and EN fields for this index are RAZ, and do not have any corresponding
                                                         INTSN. */
	uint64_t reserved_2_14                : 13;
	uint64_t en                           : 1;  /**< Enable interrupt delivery. */
	uint64_t raw                          : 1;  /**< Interrupt pending before masking. Note read only, must use
                                                         CIU3_ISC()_W1C/CIU3_ISC()_W1S to toggle. */
#else
	uint64_t raw                          : 1;
	uint64_t en                           : 1;
	uint64_t reserved_2_14                : 13;
	uint64_t imp                          : 1;
	uint64_t idt                          : 8;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_ciu3_iscx_ctl_s           cn73xx;
	struct cvmx_ciu3_iscx_ctl_s           cn78xx;
	struct cvmx_ciu3_iscx_ctl_s           cn78xxp1;
	struct cvmx_ciu3_iscx_ctl_s           cnf75xx;
};
typedef union cvmx_ciu3_iscx_ctl cvmx_ciu3_iscx_ctl_t;

/**
 * cvmx_ciu3_isc#_w1c
 */
union cvmx_ciu3_iscx_w1c {
	uint64_t u64;
	struct cvmx_ciu3_iscx_w1c_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t en                           : 1;  /**< Clear enable interrupt delivery. See CIU3_ISC()_CTL[EN]. */
	uint64_t raw                          : 1;  /**< Clear interrupt pending. See CIU3_ISC()_CTL[RAW]. */
#else
	uint64_t raw                          : 1;
	uint64_t en                           : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_ciu3_iscx_w1c_s           cn73xx;
	struct cvmx_ciu3_iscx_w1c_s           cn78xx;
	struct cvmx_ciu3_iscx_w1c_s           cn78xxp1;
	struct cvmx_ciu3_iscx_w1c_s           cnf75xx;
};
typedef union cvmx_ciu3_iscx_w1c cvmx_ciu3_iscx_w1c_t;

/**
 * cvmx_ciu3_isc#_w1s
 */
union cvmx_ciu3_iscx_w1s {
	uint64_t u64;
	struct cvmx_ciu3_iscx_w1s_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t en                           : 1;  /**< Set enable interrupt delivery. See CIU3_ISC()_CTL[EN]. */
	uint64_t raw                          : 1;  /**< Set interrupt pending. See CIU3_ISC()_CTL[RAW]. */
#else
	uint64_t raw                          : 1;
	uint64_t en                           : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_ciu3_iscx_w1s_s           cn73xx;
	struct cvmx_ciu3_iscx_w1s_s           cn78xx;
	struct cvmx_ciu3_iscx_w1s_s           cn78xxp1;
	struct cvmx_ciu3_iscx_w1s_s           cnf75xx;
};
typedef union cvmx_ciu3_iscx_w1s cvmx_ciu3_iscx_w1s_t;

/**
 * cvmx_ciu3_nmi
 */
union cvmx_ciu3_nmi {
	uint64_t u64;
	struct cvmx_ciu3_nmi_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t nmi                          : 48; /**< Writing a 1 to a bit sends an NMI pulse to the corresponding core vector. */
#else
	uint64_t nmi                          : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_ciu3_nmi_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t nmi                          : 16; /**< Writing a 1 to a bit sends an NMI pulse to the corresponding core vector. */
#else
	uint64_t nmi                          : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} cn73xx;
	struct cvmx_ciu3_nmi_s                cn78xx;
	struct cvmx_ciu3_nmi_s                cn78xxp1;
	struct cvmx_ciu3_nmi_cn73xx           cnf75xx;
};
typedef union cvmx_ciu3_nmi cvmx_ciu3_nmi_t;

/**
 * cvmx_ciu3_sisc#
 */
union cvmx_ciu3_siscx {
	uint64_t u64;
	struct cvmx_ciu3_siscx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t en                           : 64; /**< Indicates each corresponding ISC entry has enabled & interrupting entry. For diagnostic use only. */
#else
	uint64_t en                           : 64;
#endif
	} s;
	struct cvmx_ciu3_siscx_s              cn73xx;
	struct cvmx_ciu3_siscx_s              cn78xx;
	struct cvmx_ciu3_siscx_s              cn78xxp1;
	struct cvmx_ciu3_siscx_s              cnf75xx;
};
typedef union cvmx_ciu3_siscx cvmx_ciu3_siscx_t;

/**
 * cvmx_ciu3_tim#
 */
union cvmx_ciu3_timx {
	uint64_t u64;
	struct cvmx_ciu3_timx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_37_63               : 27;
	uint64_t one_shot                     : 1;  /**< One-shot mode when LEN != 0x0:
                                                         0 = Timer is in periodic mode.
                                                         1 = Timer is in one-shot mode. */
	uint64_t len                          : 36; /**< Time-out length in coprocessor clock cycles. The timer disabled when LEN = 0x0. Periodic
                                                         interrupts will occur every LEN+1 coprocessor clock cycles when ONE_SHOT = 0 */
#else
	uint64_t len                          : 36;
	uint64_t one_shot                     : 1;
	uint64_t reserved_37_63               : 27;
#endif
	} s;
	struct cvmx_ciu3_timx_s               cn73xx;
	struct cvmx_ciu3_timx_s               cn78xx;
	struct cvmx_ciu3_timx_s               cn78xxp1;
	struct cvmx_ciu3_timx_s               cnf75xx;
};
typedef union cvmx_ciu3_timx cvmx_ciu3_timx_t;

#endif
