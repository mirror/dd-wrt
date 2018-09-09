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
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
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
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
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
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
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
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 4)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 143)))))
		cvmx_warn("CVMX_CIU3_DESTX_PP_INT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001010000200000ull) + ((offset) & 255) * 8;
}
#else
#define CVMX_CIU3_DESTX_PP_INT(offset) (CVMX_ADD_IO_SEG(0x0001010000200000ull) + ((offset) & 255) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU3_DINT CVMX_CIU3_DINT_FUNC()
static inline uint64_t CVMX_CIU3_DINT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_CIU3_DINT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001010000000180ull);
}
#else
#define CVMX_CIU3_DINT (CVMX_ADD_IO_SEG(0x0001010000000180ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU3_FUSE CVMX_CIU3_FUSE_FUNC()
static inline uint64_t CVMX_CIU3_FUSE_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_CIU3_FUSE not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00010100000001A0ull);
}
#else
#define CVMX_CIU3_FUSE (CVMX_ADD_IO_SEG(0x00010100000001A0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU3_GSTOP CVMX_CIU3_GSTOP_FUNC()
static inline uint64_t CVMX_CIU3_GSTOP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
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
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 255)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 255)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset == 0)) && ((block_id <= 255))))))
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
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
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
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
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
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
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
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_CIU3_INTR_SLOWDOWN not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001010000000240ull);
}
#else
#define CVMX_CIU3_INTR_SLOWDOWN (CVMX_ADD_IO_SEG(0x0001010000000240ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU3_ISCMEM_BASE CVMX_CIU3_ISCMEM_BASE_FUNC()
static inline uint64_t CVMX_CIU3_ISCMEM_BASE_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_CIU3_ISCMEM_BASE not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00010100000002C0ull);
}
#else
#define CVMX_CIU3_ISCMEM_BASE (CVMX_ADD_IO_SEG(0x00010100000002C0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU3_ISCX_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1048575)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1048575)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1048575)))))
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
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_CIU3_NMI not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001010000000160ull);
}
#else
#define CVMX_CIU3_NMI (CVMX_ADD_IO_SEG(0x0001010000000160ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU3_PP_DBG CVMX_CIU3_PP_DBG_FUNC()
static inline uint64_t CVMX_CIU3_PP_DBG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_CIU3_PP_DBG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001010000000120ull);
}
#else
#define CVMX_CIU3_PP_DBG (CVMX_ADD_IO_SEG(0x0001010000000120ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU3_PP_POKEX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 47)))))
		cvmx_warn("CVMX_CIU3_PP_POKEX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001010000030000ull) + ((offset) & 63) * 8;
}
#else
#define CVMX_CIU3_PP_POKEX(offset) (CVMX_ADD_IO_SEG(0x0001010000030000ull) + ((offset) & 63) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU3_PP_RST CVMX_CIU3_PP_RST_FUNC()
static inline uint64_t CVMX_CIU3_PP_RST_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_CIU3_PP_RST not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001010000000100ull);
}
#else
#define CVMX_CIU3_PP_RST (CVMX_ADD_IO_SEG(0x0001010000000100ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_CIU3_PP_RST_PENDING CVMX_CIU3_PP_RST_PENDING_FUNC()
static inline uint64_t CVMX_CIU3_PP_RST_PENDING_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_CIU3_PP_RST_PENDING not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001010000000110ull);
}
#else
#define CVMX_CIU3_PP_RST_PENDING (CVMX_ADD_IO_SEG(0x0001010000000110ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU3_SISCX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 191)))))
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
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 9)))))
		cvmx_warn("CVMX_CIU3_TIMX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001010000010000ull) + ((offset) & 15) * 8;
}
#else
#define CVMX_CIU3_TIMX(offset) (CVMX_ADD_IO_SEG(0x0001010000010000ull) + ((offset) & 15) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_CIU3_WDOGX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 47)))))
		cvmx_warn("CVMX_CIU3_WDOGX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001010000020000ull) + ((offset) & 63) * 8;
}
#else
#define CVMX_CIU3_WDOGX(offset) (CVMX_ADD_IO_SEG(0x0001010000020000ull) + ((offset) & 63) * 8)
#endif

/**
 * cvmx_ciu3_bist
 */
union cvmx_ciu3_bist {
	uint64_t u64;
	struct cvmx_ciu3_bist_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_11_63               : 53;
	uint64_t bist                         : 11; /**< BIST results. Hardware sets a bit for each memory that fails BIST. INTERNAL:
                                                         <10>= ncbo_crd_fif_mem0.
                                                         <9> = ciu_nbt_sso_req_ram.
                                                         <8> = ciu_nbt_rsp_ram.
                                                         <7> = ciu_sso_output_fifo_mem.
                                                         <6> = ciu_isc_ram2.
                                                         <5> = ciu_isc_ram1.
                                                         <4> = ciu_isc_ram0.
                                                         <3> = ciu_sist_ram.
                                                         <2> = ciu_idt_ram.
                                                         <1> = csr req_mem.
                                                         <0> = ciu3_wdg_ctl_mem. */
#else
	uint64_t bist                         : 11;
	uint64_t reserved_11_63               : 53;
#endif
	} s;
	struct cvmx_ciu3_bist_s               cn78xx;
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
	uint64_t dests_pp                     : 16; /**< Number of entries in CIU3_DEST(0..143)_PP_INT. */
	uint64_t idt                          : 16; /**< Number of entries in CIU3_IDT(0..255)_CTL. */
#else
	uint64_t idt                          : 16;
	uint64_t dests_pp                     : 16;
	uint64_t pintsn                       : 16;
	uint64_t dests_io                     : 16;
#endif
	} s;
	struct cvmx_ciu3_const_s              cn78xx;
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
                                                         0x0 = MCD0
                                                         0x1 = MCD1
                                                         0x2 = MCD2
                                                         0x3 = Reserved */
	uint64_t iscmem_le                    : 1;  /**< CIU3_ISCMEM_BASE points to a little-endian table. */
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
	struct cvmx_ciu3_ctl_s                cn78xx;
};
typedef union cvmx_ciu3_ctl cvmx_ciu3_ctl_t;

/**
 * cvmx_ciu3_dest#_io_int
 *
 * This register contains reduced interrupt source numbers for delivery to software, indexed by
 * I/O bridge number. Fields are identical to CIU3_DEST(0..143)_PP_INT.
 */
union cvmx_ciu3_destx_io_int {
	uint64_t u64;
	struct cvmx_ciu3_destx_io_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_52_63               : 12;
	uint64_t intsn                        : 20; /**< Interrupt source number causing the current interrupt, or most recent interrupt if INTR is
                                                         clear. Note this field is not stored in the DEST ram itself; it is instead read from
                                                         CIU3_IDT(0..255)_CTL[INTIDT][INTSN]. */
	uint64_t reserved_10_31               : 22;
	uint64_t intidt                       : 8;  /**< IDT entry number causing the current interrupt, or most recent interrupt if INTR is clear. */
	uint64_t newint                       : 1;  /**< New interrupt to be delivered. Internal state, for diagnostic use only. */
	uint64_t intr                         : 1;  /**< Interrupt pending. This bit is recalculated when CIU3_ISC(0..1048575)_CTL or interrupts
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
	struct cvmx_ciu3_destx_io_int_s       cn78xx;
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
                                                         CIU3_IDT(0..255)_CTL[INTIDT][INTSN]. */
	uint64_t reserved_10_31               : 22;
	uint64_t intidt                       : 8;  /**< IDT entry number causing the current interrupt, or most recent interrupt if INTR is clear. */
	uint64_t newint                       : 1;  /**< New interrupt to be delivered. Internal state, for diagnostic use only. */
	uint64_t intr                         : 1;  /**< Interrupt pending. This bit is recalculated when CIU3_ISC(0..1048575)_CTL or interrupts
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
	struct cvmx_ciu3_destx_pp_int_s       cn78xx;
};
typedef union cvmx_ciu3_destx_pp_int cvmx_ciu3_destx_pp_int_t;

/**
 * cvmx_ciu3_dint
 */
union cvmx_ciu3_dint {
	uint64_t u64;
	struct cvmx_ciu3_dint_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t dint                         : 48; /**< Writing a 1 to a bit sends a DINT pulse to corresponding core vector. */
#else
	uint64_t dint                         : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_ciu3_dint_s               cn78xx;
};
typedef union cvmx_ciu3_dint cvmx_ciu3_dint_t;

/**
 * cvmx_ciu3_fuse
 */
union cvmx_ciu3_fuse {
	uint64_t u64;
	struct cvmx_ciu3_fuse_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t fuse                         : 48; /**< Each bit set indicates a physical core is present. */
#else
	uint64_t fuse                         : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_ciu3_fuse_s               cn78xx;
};
typedef union cvmx_ciu3_fuse cvmx_ciu3_fuse_t;

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
	struct cvmx_ciu3_gstop_s              cn78xx;
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
                                                         clear. INTERNAL: HW does not store the 20 bit INTSN here; it instead stores the sparse 12
                                                         bit PINTSN, and maps it to INTSN on a read. */
	uint64_t reserved_4_31                : 28;
	uint64_t intr                         : 1;  /**< Interrupt pending */
	uint64_t newint                       : 1;  /**< New interrupt to be delivered. Internal state, for diagnostic use only. */
	uint64_t ip_num                       : 2;  /**< Destination interrupt priority level to receive this interrupt. Only used for core
                                                         interrupts; for IO interrupts this level must be zero.
                                                         0 = IP2, or I/O interrupt
                                                         1 = IP3
                                                         2 = IP4
                                                         3 = reserved */
#else
	uint64_t ip_num                       : 2;
	uint64_t newint                       : 1;
	uint64_t intr                         : 1;
	uint64_t reserved_4_31                : 28;
	uint64_t intsn                        : 20;
	uint64_t reserved_52_63               : 12;
#endif
	} s;
	struct cvmx_ciu3_idtx_ctl_s           cn78xx;
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
	uint64_t io                           : 5;  /**< IO bridges to receive interrupts via this IDT. Enumerated with CIU_DEST_IO_E. */
#else
	uint64_t io                           : 5;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_ciu3_idtx_io_s            cn78xx;
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
	uint64_t pp                           : 48; /**< Cores to receive interrupts via this IDT. */
#else
	uint64_t pp                           : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_ciu3_idtx_ppx_s           cn78xx;
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
	struct cvmx_ciu3_intr_ram_ecc_ctl_s   cn78xx;
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
	uint64_t sisc_dbe                     : 1;  /**< SISC Double-bit error observed. Throws CIU_INTSN_E::CIU3_ECC_SISC_DBE. */
	uint64_t sisc_sbe                     : 1;  /**< SISC Single-bit error observed. Throws CIU_INTSN_E::CIU3_ECC_SISC_SBE. */
	uint64_t idt_dbe                      : 1;  /**< IDT Double-bit error observed. Throws CIU_INTSN_E::CIU3_ECC_IDT_DBE. */
	uint64_t idt_sbe                      : 1;  /**< IDT Single-bit error observed. Throws CIU_INTSN_E::CIU3_ECC_IDT_SBE. */
	uint64_t isc_dbe                      : 1;  /**< ISC Double-bit error observed. Throws CIU_INTSN_E::CIU3_ECC_ISC_DBE. */
	uint64_t isc_sbe                      : 1;  /**< ISC Single-bit error observed. Throws CIU_INTSN_E::CIU3_ECC_ISC_SBE. */
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
	struct cvmx_ciu3_intr_ram_ecc_st_s    cn78xx;
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
	uint64_t sso_cnt                      : 16; /**< Number of SSO events waiting to be sent to SSO. */
	uint64_t reserved_1_15                : 15;
	uint64_t ready                        : 1;  /**< CIU is idle. If clear, CIU is performing a background scan searching for secondary
                                                         interrupts. Write one to force a new scan. For diagnostic use only. */
#else
	uint64_t ready                        : 1;
	uint64_t reserved_1_15                : 15;
	uint64_t sso_cnt                      : 16;
	uint64_t index                        : 14;
	uint64_t reserved_46_63               : 18;
#endif
	} s;
	struct cvmx_ciu3_intr_ready_s         cn78xx;
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
                                                         designed for this scheme. It will only be a problem when SCLK is faster; this Control will
                                                         process 1 interrupt in 4*2^CTL SCLK cycles. With different a setting, clock rate ratio can
                                                         handle:
                                                         SLOWDOWN sclk_freq/aclk_freq ratio
                                                         0 4
                                                         1 8
                                                         n 4*2^n */
#else
	uint64_t ctl                          : 3;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_ciu3_intr_slowdown_s      cn78xx;
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
	uint64_t idt                          : 8;  /**< Interrupt Delivery Table entry number. Zero indicates IDT delivery is disabled. This field
                                                         may only be changed when EN was previously clear, though it may be changed with the same
                                                         write that sets EN. Thus if EN is set, to change IDT two register writes are required, the
                                                         first to clear EN (perhaps by a store to CIU3_ISC(0..1048575)_W1C), and the second to make
                                                         the change to IDT. */
	uint64_t imp                          : 1;  /**< Entry implemented. Although the table has 1M entries, most of those do not correspond to
                                                         any INTSN, and as such are not implemented.
                                                         1 = This index is implemented, and the bits are R/W.
                                                         0 = This index is not implemented, all bits will return as zero. */
	uint64_t sso_pend                     : 1;  /**< Transaction needs to be sent to SSO. CIU internal state for diagnostic use. [SSO_PEND]
                                                         will be cleared when the entry is transmitted to SSO, or by a software clear of [SSO],
                                                         [RAW] or [EN]. */
	uint64_t reserved_3_13                : 11;
	uint64_t sso                          : 1;  /**< Use SSO delivery. */
	uint64_t en                           : 1;  /**< Enable interrupt delivery. Must be set for PP_NUM and IP_NUM to have effect. */
	uint64_t raw                          : 1;  /**< Interrupt pending before masking. Note read only, must use CIU3_ISC(0..1048575)_W1C/_W1S to toggle. */
#else
	uint64_t raw                          : 1;
	uint64_t en                           : 1;
	uint64_t sso                          : 1;
	uint64_t reserved_3_13                : 11;
	uint64_t sso_pend                     : 1;
	uint64_t imp                          : 1;
	uint64_t idt                          : 8;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_ciu3_iscx_ctl_s           cn78xx;
};
typedef union cvmx_ciu3_iscx_ctl cvmx_ciu3_iscx_ctl_t;

/**
 * cvmx_ciu3_isc#_w1c
 */
union cvmx_ciu3_iscx_w1c {
	uint64_t u64;
	struct cvmx_ciu3_iscx_w1c_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t sso                          : 1;  /**< Use SSO work-queue-entry delivery. */
	uint64_t en                           : 1;  /**< Clear enable interrupt delivery. See CIU3_ISC(0..1048575)_CTL[EN]. */
	uint64_t raw                          : 1;  /**< Clear interrupt pending. See CIU3_ISC(0..1048575)_CTL[RAW]. */
#else
	uint64_t raw                          : 1;
	uint64_t en                           : 1;
	uint64_t sso                          : 1;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_ciu3_iscx_w1c_s           cn78xx;
};
typedef union cvmx_ciu3_iscx_w1c cvmx_ciu3_iscx_w1c_t;

/**
 * cvmx_ciu3_isc#_w1s
 */
union cvmx_ciu3_iscx_w1s {
	uint64_t u64;
	struct cvmx_ciu3_iscx_w1s_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t sso                          : 1;  /**< Use SSO work-queue-entry delivery. */
	uint64_t en                           : 1;  /**< Set enable interrupt delivery. See CIU3_ISC(0..1048575)_CTL[EN]. */
	uint64_t raw                          : 1;  /**< Set interrupt pending. See CIU3_ISC(0..1048575)_CTL[RAW]. */
#else
	uint64_t raw                          : 1;
	uint64_t en                           : 1;
	uint64_t sso                          : 1;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_ciu3_iscx_w1s_s           cn78xx;
};
typedef union cvmx_ciu3_iscx_w1s cvmx_ciu3_iscx_w1s_t;

/**
 * cvmx_ciu3_iscmem_base
 */
union cvmx_ciu3_iscmem_base {
	uint64_t u64;
	struct cvmx_ciu3_iscmem_base_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_42_63               : 22;
	uint64_t addr                         : 38; /**< Base address of CIU_INTSN_E table. Note must be 16-byte aligned and is in root-physical
                                                         address space. Endinaness is selected with CIU3_CTL[ISCMEM_LE]. */
	uint64_t addrl4                       : 4;  /**< Lowest 4 bits of Base address of CIU_INTSN_E table, always zero as CIU_ISCMEM_S structure
                                                         is 16-byte aligned. */
#else
	uint64_t addrl4                       : 4;
	uint64_t addr                         : 38;
	uint64_t reserved_42_63               : 22;
#endif
	} s;
	struct cvmx_ciu3_iscmem_base_s        cn78xx;
};
typedef union cvmx_ciu3_iscmem_base cvmx_ciu3_iscmem_base_t;

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
	struct cvmx_ciu3_nmi_s                cn78xx;
};
typedef union cvmx_ciu3_nmi cvmx_ciu3_nmi_t;

/**
 * cvmx_ciu3_pp_dbg
 */
union cvmx_ciu3_pp_dbg {
	uint64_t u64;
	struct cvmx_ciu3_pp_dbg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t ppdbg                        : 48; /**< Debug[DM] value for each core, whether the cores are in debug mode or not. */
#else
	uint64_t ppdbg                        : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_ciu3_pp_dbg_s             cn78xx;
};
typedef union cvmx_ciu3_pp_dbg cvmx_ciu3_pp_dbg_t;

/**
 * cvmx_ciu3_pp_poke#
 */
union cvmx_ciu3_pp_pokex {
	uint64_t u64;
	struct cvmx_ciu3_pp_pokex_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t poke                         : 1;  /**< Core poke. Writing any value to this register does the following:
                                                         clears any pending interrupt generated by the associated watchdog
                                                         resets CIU3_WDOG(0..47)[STATE] to 0x0
                                                         sets CIU3_WDOG(0..47)[CNT] to ( CIU3_WDOG(0..47)[LEN] << 8).
                                                         Reading this register returns the associated CIU3_WDOG(0..47) register. */
#else
	uint64_t poke                         : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_ciu3_pp_pokex_s           cn78xx;
};
typedef union cvmx_ciu3_pp_pokex cvmx_ciu3_pp_pokex_t;

/**
 * cvmx_ciu3_pp_rst
 *
 * This register contains the reset control for each core. A 1 holds a core in reset, 0 release
 * from reset. It resets to all ones when REMOTE_BOOT is enabled or all ones excluding bit 0 when
 * REMOTE_BOOT is disabled. Writes to this register should occur only if the CIU3_PP_RST_PENDING
 * register is cleared.
 */
union cvmx_ciu3_pp_rst {
	uint64_t u64;
	struct cvmx_ciu3_pp_rst_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t rst                          : 47; /**< Core reset for cores 1 and above. Writing a 1 holds the corresponding core in reset,
                                                         writing a 0 releases from reset. */
	uint64_t rst0                         : 1;  /**< Core reset for core 0, depends on standalone mode. */
#else
	uint64_t rst0                         : 1;
	uint64_t rst                          : 47;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_ciu3_pp_rst_s             cn78xx;
};
typedef union cvmx_ciu3_pp_rst cvmx_ciu3_pp_rst_t;

/**
 * cvmx_ciu3_pp_rst_pending
 *
 * This register contains the reset status for each core.
 *
 */
union cvmx_ciu3_pp_rst_pending {
	uint64_t u64;
	struct cvmx_ciu3_pp_rst_pending_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t pend                         : 48; /**< Set if corresponding core is waiting to change its reset state. Normally a reset change
                                                         occurs immediately but if RST_PP_POWER[GATE] bit is set and the core is released from
                                                         reset a delay of 64K core clocks between each core reset will apply to satisfy power
                                                         management. */
#else
	uint64_t pend                         : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_ciu3_pp_rst_pending_s     cn78xx;
};
typedef union cvmx_ciu3_pp_rst_pending cvmx_ciu3_pp_rst_pending_t;

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
	struct cvmx_ciu3_siscx_s              cn78xx;
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
                                                         1 = timer is in one-shot mode, 0 = timer is in periodic mode. */
	uint64_t len                          : 36; /**< Time-out length in coprocessor clock cycles. The timer disabled when LEN = 0x0. Periodic
                                                         interrupts will occur every LEN+1 coprocessor clock cycles when ONE_SHOT = 0 */
#else
	uint64_t len                          : 36;
	uint64_t one_shot                     : 1;
	uint64_t reserved_37_63               : 27;
#endif
	} s;
	struct cvmx_ciu3_timx_s               cn78xx;
};
typedef union cvmx_ciu3_timx cvmx_ciu3_timx_t;

/**
 * cvmx_ciu3_wdog#
 */
union cvmx_ciu3_wdogx {
	uint64_t u64;
	struct cvmx_ciu3_wdogx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_46_63               : 18;
	uint64_t gstopen                      : 1;  /**< Global-stop enable. */
	uint64_t dstop                        : 1;  /**< Debug-stop enable. */
	uint64_t cnt                          : 24; /**< Number of 1024-cycle intervals until next watchdog expiration. Cleared on write to
                                                         associated CIU3_PP_POKE(0..47) register. */
	uint64_t len                          : 16; /**< Watchdog time-expiration length. The most-significant 16 bits of a 24-bit decrementer that
                                                         decrements every 1024 cycles. Must be set > 0. */
	uint64_t state                        : 2;  /**< Watchdog state. The number of watchdog time expirations since last core poke. Cleared on
                                                         write to associated CIU3_PP_POKE(0..47) register. */
	uint64_t mode                         : 2;  /**< Watchdog mode:
                                                         0x0 = Off 0x2 = Interrupt + NMI
                                                         0x1 = Interrupt only 0x3 = Interrupt + NMI + soft reset */
#else
	uint64_t mode                         : 2;
	uint64_t state                        : 2;
	uint64_t len                          : 16;
	uint64_t cnt                          : 24;
	uint64_t dstop                        : 1;
	uint64_t gstopen                      : 1;
	uint64_t reserved_46_63               : 18;
#endif
	} s;
	struct cvmx_ciu3_wdogx_s              cn78xx;
};
typedef union cvmx_ciu3_wdogx cvmx_ciu3_wdogx_t;

#endif
