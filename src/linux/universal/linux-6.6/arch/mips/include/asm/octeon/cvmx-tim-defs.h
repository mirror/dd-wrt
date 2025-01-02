/***********************license start***************
 * Copyright (c) 2003-2014  Cavium Inc. (support@cavium.com). All rights
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
 * cvmx-tim-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon tim.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_TIM_DEFS_H__
#define __CVMX_TIM_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_TIM_BIST_RESULT CVMX_TIM_BIST_RESULT_FUNC()
static inline uint64_t CVMX_TIM_BIST_RESULT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_TIM_BIST_RESULT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180058000020ull);
}
#else
#define CVMX_TIM_BIST_RESULT (CVMX_ADD_IO_SEG(0x0001180058000020ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_TIM_DBG2 CVMX_TIM_DBG2_FUNC()
static inline uint64_t CVMX_TIM_DBG2_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_TIM_DBG2 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800580000A0ull);
}
#else
#define CVMX_TIM_DBG2 (CVMX_ADD_IO_SEG(0x00011800580000A0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_TIM_DBG3 CVMX_TIM_DBG3_FUNC()
static inline uint64_t CVMX_TIM_DBG3_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_TIM_DBG3 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800580000A8ull);
}
#else
#define CVMX_TIM_DBG3 (CVMX_ADD_IO_SEG(0x00011800580000A8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_TIM_ECC_CFG CVMX_TIM_ECC_CFG_FUNC()
static inline uint64_t CVMX_TIM_ECC_CFG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_TIM_ECC_CFG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180058000018ull);
}
#else
#define CVMX_TIM_ECC_CFG (CVMX_ADD_IO_SEG(0x0001180058000018ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_TIM_ENGX_ACTIVE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_TIM_ENGX_ACTIVE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180058001000ull) + ((offset) & 3) * 8;
}
#else
#define CVMX_TIM_ENGX_ACTIVE(offset) (CVMX_ADD_IO_SEG(0x0001180058001000ull) + ((offset) & 3) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_TIM_FR_RN_CYCLES CVMX_TIM_FR_RN_CYCLES_FUNC()
static inline uint64_t CVMX_TIM_FR_RN_CYCLES_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_TIM_FR_RN_CYCLES not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800580000C0ull);
}
#else
#define CVMX_TIM_FR_RN_CYCLES (CVMX_ADD_IO_SEG(0x00011800580000C0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_TIM_FR_RN_GPIOS CVMX_TIM_FR_RN_GPIOS_FUNC()
static inline uint64_t CVMX_TIM_FR_RN_GPIOS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_TIM_FR_RN_GPIOS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800580000C8ull);
}
#else
#define CVMX_TIM_FR_RN_GPIOS (CVMX_ADD_IO_SEG(0x00011800580000C8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_TIM_FR_RN_TT CVMX_TIM_FR_RN_TT_FUNC()
static inline uint64_t CVMX_TIM_FR_RN_TT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_TIM_FR_RN_TT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180058000010ull);
}
#else
#define CVMX_TIM_FR_RN_TT (CVMX_ADD_IO_SEG(0x0001180058000010ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_TIM_GPIO_EN CVMX_TIM_GPIO_EN_FUNC()
static inline uint64_t CVMX_TIM_GPIO_EN_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_TIM_GPIO_EN not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180058000080ull);
}
#else
#define CVMX_TIM_GPIO_EN (CVMX_ADD_IO_SEG(0x0001180058000080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_TIM_INT0 CVMX_TIM_INT0_FUNC()
static inline uint64_t CVMX_TIM_INT0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_TIM_INT0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180058000030ull);
}
#else
#define CVMX_TIM_INT0 (CVMX_ADD_IO_SEG(0x0001180058000030ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_TIM_INT0_EN CVMX_TIM_INT0_EN_FUNC()
static inline uint64_t CVMX_TIM_INT0_EN_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_TIM_INT0_EN not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180058000038ull);
}
#else
#define CVMX_TIM_INT0_EN (CVMX_ADD_IO_SEG(0x0001180058000038ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_TIM_INT0_EVENT CVMX_TIM_INT0_EVENT_FUNC()
static inline uint64_t CVMX_TIM_INT0_EVENT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_TIM_INT0_EVENT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180058000040ull);
}
#else
#define CVMX_TIM_INT0_EVENT (CVMX_ADD_IO_SEG(0x0001180058000040ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_TIM_INT_ECCERR CVMX_TIM_INT_ECCERR_FUNC()
static inline uint64_t CVMX_TIM_INT_ECCERR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_TIM_INT_ECCERR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180058000060ull);
}
#else
#define CVMX_TIM_INT_ECCERR (CVMX_ADD_IO_SEG(0x0001180058000060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_TIM_INT_ECCERR_EN CVMX_TIM_INT_ECCERR_EN_FUNC()
static inline uint64_t CVMX_TIM_INT_ECCERR_EN_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_TIM_INT_ECCERR_EN not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180058000068ull);
}
#else
#define CVMX_TIM_INT_ECCERR_EN (CVMX_ADD_IO_SEG(0x0001180058000068ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_TIM_INT_ECCERR_EVENT0 CVMX_TIM_INT_ECCERR_EVENT0_FUNC()
static inline uint64_t CVMX_TIM_INT_ECCERR_EVENT0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_TIM_INT_ECCERR_EVENT0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180058000070ull);
}
#else
#define CVMX_TIM_INT_ECCERR_EVENT0 (CVMX_ADD_IO_SEG(0x0001180058000070ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_TIM_INT_ECCERR_EVENT1 CVMX_TIM_INT_ECCERR_EVENT1_FUNC()
static inline uint64_t CVMX_TIM_INT_ECCERR_EVENT1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_TIM_INT_ECCERR_EVENT1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180058000078ull);
}
#else
#define CVMX_TIM_INT_ECCERR_EVENT1 (CVMX_ADD_IO_SEG(0x0001180058000078ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_TIM_MEM_DEBUG0 CVMX_TIM_MEM_DEBUG0_FUNC()
static inline uint64_t CVMX_TIM_MEM_DEBUG0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN5XXX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_TIM_MEM_DEBUG0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180058001100ull);
}
#else
#define CVMX_TIM_MEM_DEBUG0 (CVMX_ADD_IO_SEG(0x0001180058001100ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_TIM_MEM_DEBUG1 CVMX_TIM_MEM_DEBUG1_FUNC()
static inline uint64_t CVMX_TIM_MEM_DEBUG1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN5XXX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_TIM_MEM_DEBUG1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180058001108ull);
}
#else
#define CVMX_TIM_MEM_DEBUG1 (CVMX_ADD_IO_SEG(0x0001180058001108ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_TIM_MEM_DEBUG2 CVMX_TIM_MEM_DEBUG2_FUNC()
static inline uint64_t CVMX_TIM_MEM_DEBUG2_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN5XXX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_TIM_MEM_DEBUG2 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180058001110ull);
}
#else
#define CVMX_TIM_MEM_DEBUG2 (CVMX_ADD_IO_SEG(0x0001180058001110ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_TIM_MEM_RING0 CVMX_TIM_MEM_RING0_FUNC()
static inline uint64_t CVMX_TIM_MEM_RING0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN5XXX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_TIM_MEM_RING0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180058001000ull);
}
#else
#define CVMX_TIM_MEM_RING0 (CVMX_ADD_IO_SEG(0x0001180058001000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_TIM_MEM_RING1 CVMX_TIM_MEM_RING1_FUNC()
static inline uint64_t CVMX_TIM_MEM_RING1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN5XXX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_TIM_MEM_RING1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180058001008ull);
}
#else
#define CVMX_TIM_MEM_RING1 (CVMX_ADD_IO_SEG(0x0001180058001008ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_TIM_REG_BIST_RESULT CVMX_TIM_REG_BIST_RESULT_FUNC()
static inline uint64_t CVMX_TIM_REG_BIST_RESULT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN5XXX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_TIM_REG_BIST_RESULT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180058000080ull);
}
#else
#define CVMX_TIM_REG_BIST_RESULT (CVMX_ADD_IO_SEG(0x0001180058000080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_TIM_REG_ERROR CVMX_TIM_REG_ERROR_FUNC()
static inline uint64_t CVMX_TIM_REG_ERROR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN5XXX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_TIM_REG_ERROR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180058000088ull);
}
#else
#define CVMX_TIM_REG_ERROR (CVMX_ADD_IO_SEG(0x0001180058000088ull))
#endif
#define CVMX_TIM_REG_FLAGS (CVMX_ADD_IO_SEG(0x0001180058000000ull))
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_TIM_REG_INT_MASK CVMX_TIM_REG_INT_MASK_FUNC()
static inline uint64_t CVMX_TIM_REG_INT_MASK_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN5XXX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_TIM_REG_INT_MASK not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180058000090ull);
}
#else
#define CVMX_TIM_REG_INT_MASK (CVMX_ADD_IO_SEG(0x0001180058000090ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_TIM_REG_READ_IDX CVMX_TIM_REG_READ_IDX_FUNC()
static inline uint64_t CVMX_TIM_REG_READ_IDX_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN5XXX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_TIM_REG_READ_IDX not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180058000008ull);
}
#else
#define CVMX_TIM_REG_READ_IDX (CVMX_ADD_IO_SEG(0x0001180058000008ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_TIM_RINGX_AURA(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_TIM_RINGX_AURA(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180058002C00ull) + ((offset) & 63) * 8;
}
#else
#define CVMX_TIM_RINGX_AURA(offset) (CVMX_ADD_IO_SEG(0x0001180058002C00ull) + ((offset) & 63) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_TIM_RINGX_CTL0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_TIM_RINGX_CTL0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180058002000ull) + ((offset) & 63) * 8;
}
#else
#define CVMX_TIM_RINGX_CTL0(offset) (CVMX_ADD_IO_SEG(0x0001180058002000ull) + ((offset) & 63) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_TIM_RINGX_CTL1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_TIM_RINGX_CTL1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180058002400ull) + ((offset) & 63) * 8;
}
#else
#define CVMX_TIM_RINGX_CTL1(offset) (CVMX_ADD_IO_SEG(0x0001180058002400ull) + ((offset) & 63) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_TIM_RINGX_CTL2(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_TIM_RINGX_CTL2(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180058002800ull) + ((offset) & 63) * 8;
}
#else
#define CVMX_TIM_RINGX_CTL2(offset) (CVMX_ADD_IO_SEG(0x0001180058002800ull) + ((offset) & 63) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_TIM_RINGX_DBG0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_TIM_RINGX_DBG0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180058003000ull) + ((offset) & 63) * 8;
}
#else
#define CVMX_TIM_RINGX_DBG0(offset) (CVMX_ADD_IO_SEG(0x0001180058003000ull) + ((offset) & 63) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_TIM_RINGX_DBG1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_TIM_RINGX_DBG1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180058001200ull) + ((offset) & 63) * 8;
}
#else
#define CVMX_TIM_RINGX_DBG1(offset) (CVMX_ADD_IO_SEG(0x0001180058001200ull) + ((offset) & 63) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_TIM_RINGX_REL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_TIM_RINGX_REL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180058003000ull) + ((offset) & 63) * 8;
}
#else
#define CVMX_TIM_RINGX_REL(offset) (CVMX_ADD_IO_SEG(0x0001180058003000ull) + ((offset) & 63) * 8)
#endif

/**
 * cvmx_tim_bist_result
 *
 * This register provides access to the internal timer BIST results. Each bit is the BIST result
 * of an individual memory (per bit, 0 = pass and 1 = fail).
 */
union cvmx_tim_bist_result {
	uint64_t u64;
	struct cvmx_tim_bist_result_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t tstmp_mem                    : 1;  /**< BIST result of the Time Stamp memory. */
	uint64_t wqe_fifo                     : 1;  /**< BIST result of the NCB_WQE FIFO. */
	uint64_t lslr_fifo                    : 1;  /**< BIST result of the NCB_LSLR FIFO. */
	uint64_t rds_mem                      : 1;  /**< BIST result of the RDS memory. */
#else
	uint64_t rds_mem                      : 1;
	uint64_t lslr_fifo                    : 1;
	uint64_t wqe_fifo                     : 1;
	uint64_t tstmp_mem                    : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_tim_bist_result_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t wqe_fifo                     : 1;  /**< BIST result of the NCB_WQE FIFO (0=pass, !0=fail) */
	uint64_t lslr_fifo                    : 1;  /**< BIST result of the NCB_LSLR FIFO (0=pass, !0=fail) */
	uint64_t rds_mem                      : 1;  /**< BIST result of the RDS memory (0=pass, !0=fail) */
#else
	uint64_t rds_mem                      : 1;
	uint64_t lslr_fifo                    : 1;
	uint64_t wqe_fifo                     : 1;
	uint64_t reserved_3_63                : 61;
#endif
	} cn68xx;
	struct cvmx_tim_bist_result_cn68xx    cn68xxp1;
	struct cvmx_tim_bist_result_s         cn78xx;
};
typedef union cvmx_tim_bist_result cvmx_tim_bist_result_t;

/**
 * cvmx_tim_dbg2
 */
union cvmx_tim_dbg2 {
	uint64_t u64;
	struct cvmx_tim_dbg2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t mem_alloc_reg                : 8;  /**< IOI load memory allocation status. */
	uint64_t reserved_46_55               : 10;
	uint64_t rwf_fifo_level               : 6;  /**< IOI requests FIFO level. */
	uint64_t wqe_fifo_level               : 8;  /**< IOI WQE LD FIFO level. */
	uint64_t reserved_16_31               : 16;
	uint64_t fsm3_state                   : 4;  /**< FSM 3 current state. */
	uint64_t fsm2_state                   : 4;  /**< FSM 2 current state. */
	uint64_t fsm1_state                   : 4;  /**< FSM 1 current state. */
	uint64_t fsm0_state                   : 4;  /**< FSM 0 current state. */
#else
	uint64_t fsm0_state                   : 4;
	uint64_t fsm1_state                   : 4;
	uint64_t fsm2_state                   : 4;
	uint64_t fsm3_state                   : 4;
	uint64_t reserved_16_31               : 16;
	uint64_t wqe_fifo_level               : 8;
	uint64_t rwf_fifo_level               : 6;
	uint64_t reserved_46_55               : 10;
	uint64_t mem_alloc_reg                : 8;
#endif
	} s;
	struct cvmx_tim_dbg2_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t mem_alloc_reg                : 8;  /**< NCB Load Memory Allocation status */
	uint64_t reserved_51_55               : 5;
	uint64_t gnt_fifo_level               : 3;  /**< NCB GRANT FIFO level */
	uint64_t reserved_45_47               : 3;
	uint64_t rwf_fifo_level               : 5;  /**< NCB requests FIFO level */
	uint64_t wqe_fifo_level               : 8;  /**< NCB WQE LD FIFO level */
	uint64_t reserved_16_31               : 16;
	uint64_t fsm3_state                   : 4;  /**< FSM 3 current state */
	uint64_t fsm2_state                   : 4;  /**< FSM 2 current state */
	uint64_t fsm1_state                   : 4;  /**< FSM 1 current state */
	uint64_t fsm0_state                   : 4;  /**< FSM 0 current state */
#else
	uint64_t fsm0_state                   : 4;
	uint64_t fsm1_state                   : 4;
	uint64_t fsm2_state                   : 4;
	uint64_t fsm3_state                   : 4;
	uint64_t reserved_16_31               : 16;
	uint64_t wqe_fifo_level               : 8;
	uint64_t rwf_fifo_level               : 5;
	uint64_t reserved_45_47               : 3;
	uint64_t gnt_fifo_level               : 3;
	uint64_t reserved_51_55               : 5;
	uint64_t mem_alloc_reg                : 8;
#endif
	} cn68xx;
	struct cvmx_tim_dbg2_cn68xx           cn68xxp1;
	struct cvmx_tim_dbg2_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t mem_alloc_reg                : 8;  /**< IOI load memory allocation status. */
	uint64_t reserved_53_55               : 3;
	uint64_t gnt_fifo_level               : 4;  /**< IOI grant FIFO level. */
	uint64_t fpa_fifo_level               : 3;  /**< FPA FIFO level. */
	uint64_t rwf_fifo_level               : 6;  /**< IOI requests FIFO level. */
	uint64_t wqe_fifo_level               : 8;  /**< IOI WQE LD FIFO level. */
	uint64_t reserved_16_31               : 16;
	uint64_t fsm3_state                   : 4;  /**< FSM 3 current state. */
	uint64_t fsm2_state                   : 4;  /**< FSM 2 current state. */
	uint64_t fsm1_state                   : 4;  /**< FSM 1 current state. */
	uint64_t fsm0_state                   : 4;  /**< FSM 0 current state. */
#else
	uint64_t fsm0_state                   : 4;
	uint64_t fsm1_state                   : 4;
	uint64_t fsm2_state                   : 4;
	uint64_t fsm3_state                   : 4;
	uint64_t reserved_16_31               : 16;
	uint64_t wqe_fifo_level               : 8;
	uint64_t rwf_fifo_level               : 6;
	uint64_t fpa_fifo_level               : 3;
	uint64_t gnt_fifo_level               : 4;
	uint64_t reserved_53_55               : 3;
	uint64_t mem_alloc_reg                : 8;
#endif
	} cn78xx;
};
typedef union cvmx_tim_dbg2 cvmx_tim_dbg2_t;

/**
 * cvmx_tim_dbg3
 */
union cvmx_tim_dbg3 {
	uint64_t u64;
	struct cvmx_tim_dbg3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rings_pending_vec            : 64; /**< Pending rings vector. Indicates which ring in TIM are pending traversal. Bit 0 represents
                                                         ring 0 while bit 63 represents ring 63. */
#else
	uint64_t rings_pending_vec            : 64;
#endif
	} s;
	struct cvmx_tim_dbg3_s                cn68xx;
	struct cvmx_tim_dbg3_s                cn68xxp1;
	struct cvmx_tim_dbg3_s                cn78xx;
};
typedef union cvmx_tim_dbg3 cvmx_tim_dbg3_t;

/**
 * cvmx_tim_ecc_cfg
 */
union cvmx_tim_ecc_cfg {
	uint64_t u64;
	struct cvmx_tim_ecc_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t ecc_flp_syn                  : 2;  /**< ECC flip syndrome. Flip the ECC's syndrome for testing purposes, to test SBE and DBE ECC
                                                         interrupts. */
	uint64_t ecc_en                       : 1;  /**< Enable ECC correction of the ring data structure memory. */
#else
	uint64_t ecc_en                       : 1;
	uint64_t ecc_flp_syn                  : 2;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_tim_ecc_cfg_s             cn68xx;
	struct cvmx_tim_ecc_cfg_s             cn68xxp1;
	struct cvmx_tim_ecc_cfg_s             cn78xx;
};
typedef union cvmx_tim_ecc_cfg cvmx_tim_ecc_cfg_t;

/**
 * cvmx_tim_eng#_active
 */
union cvmx_tim_engx_active {
	uint64_t u64;
	struct cvmx_tim_engx_active_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t act                          : 1;  /**< Engine active. For diagnostic use. */
	uint64_t reserved_6_7                 : 2;
	uint64_t ring_id                      : 6;  /**< Current ring ID. For diagnostic use. */
#else
	uint64_t ring_id                      : 6;
	uint64_t reserved_6_7                 : 2;
	uint64_t act                          : 1;
	uint64_t reserved_9_63                : 55;
#endif
	} s;
	struct cvmx_tim_engx_active_s         cn78xx;
};
typedef union cvmx_tim_engx_active cvmx_tim_engx_active_t;

/**
 * cvmx_tim_fr_rn_cycles
 */
union cvmx_tim_fr_rn_cycles {
	uint64_t u64;
	struct cvmx_tim_fr_rn_cycles_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t count                        : 64; /**< Count of system coprocessor-clock cycles. This register is only writable when
                                                         TIM_REG_FLAGS[ENA_TIM] = 0. */
#else
	uint64_t count                        : 64;
#endif
	} s;
	struct cvmx_tim_fr_rn_cycles_s        cn78xx;
};
typedef union cvmx_tim_fr_rn_cycles cvmx_tim_fr_rn_cycles_t;

/**
 * cvmx_tim_fr_rn_gpios
 */
union cvmx_tim_fr_rn_gpios {
	uint64_t u64;
	struct cvmx_tim_fr_rn_gpios_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t count                        : 64; /**< Count of GPIO cycles. This register is only writable when TIM_REG_FLAGS[ENA_TIM] = 0. */
#else
	uint64_t count                        : 64;
#endif
	} s;
	struct cvmx_tim_fr_rn_gpios_s         cn78xx;
};
typedef union cvmx_tim_fr_rn_gpios cvmx_tim_fr_rn_gpios_t;

/**
 * cvmx_tim_fr_rn_tt
 *
 * Notes:
 * For every 64 entries in a bucket interval should be at
 * least 1us.
 * Minimal recommended value for Threshold register is 1us
 */
union cvmx_tim_fr_rn_tt {
	uint64_t u64;
	struct cvmx_tim_fr_rn_tt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_54_63               : 10;
	uint64_t thld_gp                      : 22; /**< Free Running Timer Threshold. Defines the reset value
                                                         for the free running timer when it reaches zero during
                                                         it's count down. This threshold only applies to the
                                                         timer that is driven by GPIO edge as defined at
                                                         TIM_REG_FLAGS.GPIO_EDGE
                                                         ***NOTE: Added in pass 2.0 */
	uint64_t reserved_22_31               : 10;
	uint64_t fr_rn_tt                     : 22; /**< Free Running Timer Threshold. Defines the reset value
                                                         for the free running timer when it reaches zero during
                                                         it's count down.
                                                         FR_RN_TT will be used in both cases where free running
                                                         clock is driven externally or internally.
                                                         Interval programming guidelines:
                                                         For every 64 entries in a bucket interval should be at
                                                         least 1us.
                                                         Minimal recommended value for FR_RN_TT is 1us. */
#else
	uint64_t fr_rn_tt                     : 22;
	uint64_t reserved_22_31               : 10;
	uint64_t thld_gp                      : 22;
	uint64_t reserved_54_63               : 10;
#endif
	} s;
	struct cvmx_tim_fr_rn_tt_s            cn68xx;
	struct cvmx_tim_fr_rn_tt_cn68xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_22_63               : 42;
	uint64_t fr_rn_tt                     : 22; /**< Free Running Timer Threshold. Defines the reset value
                                                         for the free running timer when it reaches zero during
                                                         it's count down.
                                                         FR_RN_TT will be used in both cases where free running
                                                         clock is driven externally or internally.
                                                         Interval programming guidelines:
                                                         For every 64 entries in a bucket interval should be at
                                                         least 1us.
                                                         Minimal recommended value for FR_RN_TT is 1us. */
#else
	uint64_t fr_rn_tt                     : 22;
	uint64_t reserved_22_63               : 42;
#endif
	} cn68xxp1;
};
typedef union cvmx_tim_fr_rn_tt cvmx_tim_fr_rn_tt_t;

/**
 * cvmx_tim_gpio_en
 */
union cvmx_tim_gpio_en {
	uint64_t u64;
	struct cvmx_tim_gpio_en_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t gpio_en                      : 64; /**< Each bit corresponds to rings [63:0] respectively. This register reflects the values
                                                         written to TIM_RING()_CTL1 [ENA_GPIO]. For debug only; Reserved. */
#else
	uint64_t gpio_en                      : 64;
#endif
	} s;
	struct cvmx_tim_gpio_en_s             cn68xx;
	struct cvmx_tim_gpio_en_s             cn78xx;
};
typedef union cvmx_tim_gpio_en cvmx_tim_gpio_en_t;

/**
 * cvmx_tim_int0
 *
 * A ring is in error if its interval has elapsed more than once without having been serviced,
 * either due to too many events in this ring's previous interval, or another ring having too
 * many events to process within this ring's interval. This is usually a programming error where
 * the number of entries in the bucket is too large for the interval specified across the rings.
 * When in error, TIM may process events in an interval later than requested. Any bit in the INT
 * field should be cleared by writing one to it.
 */
union cvmx_tim_int0 {
	uint64_t u64;
	struct cvmx_tim_int0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t int0                         : 64; /**< Interrupt bit per ring. Each bit indicates the ring number in error. Throws INTSN
                                                         TIM_INTSN_E::TIM_RING()_SLOW. */
#else
	uint64_t int0                         : 64;
#endif
	} s;
	struct cvmx_tim_int0_s                cn68xx;
	struct cvmx_tim_int0_s                cn68xxp1;
	struct cvmx_tim_int0_s                cn78xx;
};
typedef union cvmx_tim_int0 cvmx_tim_int0_t;

/**
 * cvmx_tim_int0_en
 *
 * Notes:
 * When bit at TIM_INT0_EN is set it enables the corresponding TIM_INTO's bit for interrupt generation
 * If enable bit is cleared the corresponding bit at TIM_INT0 will still be set.
 * Interrupt to the cores is generated by : |(TIM_INT0 & TIM_INT0_EN0)
 */
union cvmx_tim_int0_en {
	uint64_t u64;
	struct cvmx_tim_int0_en_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t int0_en                      : 64; /**< Bit enable corresponding to TIM_INT0. */
#else
	uint64_t int0_en                      : 64;
#endif
	} s;
	struct cvmx_tim_int0_en_s             cn68xx;
	struct cvmx_tim_int0_en_s             cn68xxp1;
};
typedef union cvmx_tim_int0_en cvmx_tim_int0_en_t;

/**
 * cvmx_tim_int0_event
 */
union cvmx_tim_int0_event {
	uint64_t u64;
	struct cvmx_tim_int0_event_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t ring_id                      : 6;  /**< The first Ring ID where an interrupt occurred. */
#else
	uint64_t ring_id                      : 6;
	uint64_t reserved_6_63                : 58;
#endif
	} s;
	struct cvmx_tim_int0_event_s          cn68xx;
	struct cvmx_tim_int0_event_s          cn68xxp1;
};
typedef union cvmx_tim_int0_event cvmx_tim_int0_event_t;

/**
 * cvmx_tim_int_eccerr
 *
 * Notes:
 * Each bit in this reg is set regardless of TIM_INT_ECCERR_EN value.
 *
 */
union cvmx_tim_int_eccerr {
	uint64_t u64;
	struct cvmx_tim_int_eccerr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t ctl_dbe                      : 1;  /**< TIM CTL memory had a double-bit error. Throws INTSN TIM_INTSN_E::TIM_ECCERR_CTL_DBE. */
	uint64_t ctl_sbe                      : 1;  /**< TIM CTL memory had a single-bit error. Throws INTSN TIM_INTSN_E::TIM_ECCERR_CTL_SBE. */
	uint64_t dbe                          : 1;  /**< TIM RDS memory had a double-bit error. Throws INTSN TIM_INTSN_E::TIM_ECCERR_DBE. */
	uint64_t sbe                          : 1;  /**< TIM RDS memory had a single-bit error. Throws INTSN TIM_INTSN_E::TIM_ECCERR_SBE. */
#else
	uint64_t sbe                          : 1;
	uint64_t dbe                          : 1;
	uint64_t ctl_sbe                      : 1;
	uint64_t ctl_dbe                      : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_tim_int_eccerr_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t dbe                          : 1;  /**< TIM RDS memory had a Double Bit Error */
	uint64_t sbe                          : 1;  /**< TIM RDS memory had a Single Bit Error */
#else
	uint64_t sbe                          : 1;
	uint64_t dbe                          : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} cn68xx;
	struct cvmx_tim_int_eccerr_cn68xx     cn68xxp1;
	struct cvmx_tim_int_eccerr_s          cn78xx;
};
typedef union cvmx_tim_int_eccerr cvmx_tim_int_eccerr_t;

/**
 * cvmx_tim_int_eccerr_en
 *
 * Notes:
 * When mask bit is set, the corresponding bit in TIM_INT_ECCERR is enabled. If mask bit is cleared the
 * corresponding bit in TIM_INT_ECCERR will still be set but interrupt will not be reported.
 */
union cvmx_tim_int_eccerr_en {
	uint64_t u64;
	struct cvmx_tim_int_eccerr_en_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t dbe_en                       : 1;  /**< Bit mask corresponding to TIM_REG_ECCERR.DBE */
	uint64_t sbe_en                       : 1;  /**< Bit mask corresponding to TIM_REG_ECCERR.SBE */
#else
	uint64_t sbe_en                       : 1;
	uint64_t dbe_en                       : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_tim_int_eccerr_en_s       cn68xx;
	struct cvmx_tim_int_eccerr_en_s       cn68xxp1;
};
typedef union cvmx_tim_int_eccerr_en cvmx_tim_int_eccerr_en_t;

/**
 * cvmx_tim_int_eccerr_event0
 */
union cvmx_tim_int_eccerr_event0 {
	uint64_t u64;
	struct cvmx_tim_int_eccerr_event0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t add                          : 7;  /**< Memory address where the error occurred. */
#else
	uint64_t add                          : 7;
	uint64_t reserved_7_63                : 57;
#endif
	} s;
	struct cvmx_tim_int_eccerr_event0_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_15_63               : 49;
	uint64_t synd                         : 7;  /**< ECC Syndrome */
	uint64_t add                          : 8;  /**< Memory address where the Error occurred. */
#else
	uint64_t add                          : 8;
	uint64_t synd                         : 7;
	uint64_t reserved_15_63               : 49;
#endif
	} cn68xx;
	struct cvmx_tim_int_eccerr_event0_cn68xx cn68xxp1;
	struct cvmx_tim_int_eccerr_event0_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t synd                         : 7;  /**< ECC syndrome bits. */
	uint64_t add                          : 7;  /**< Memory address where the error occurred. */
#else
	uint64_t add                          : 7;
	uint64_t synd                         : 7;
	uint64_t reserved_14_63               : 50;
#endif
	} cn78xx;
};
typedef union cvmx_tim_int_eccerr_event0 cvmx_tim_int_eccerr_event0_t;

/**
 * cvmx_tim_int_eccerr_event1
 */
union cvmx_tim_int_eccerr_event1 {
	uint64_t u64;
	struct cvmx_tim_int_eccerr_event1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_55_63               : 9;
	uint64_t org_ecc                      : 7;  /**< Original ECC bits where the error occurred. */
	uint64_t org_rds_dat                  : 48; /**< Memory original data where the error occurred. */
#else
	uint64_t org_rds_dat                  : 48;
	uint64_t org_ecc                      : 7;
	uint64_t reserved_55_63               : 9;
#endif
	} s;
	struct cvmx_tim_int_eccerr_event1_s   cn68xx;
	struct cvmx_tim_int_eccerr_event1_s   cn68xxp1;
	struct cvmx_tim_int_eccerr_event1_s   cn78xx;
};
typedef union cvmx_tim_int_eccerr_event1 cvmx_tim_int_eccerr_event1_t;

/**
 * cvmx_tim_mem_debug0
 *
 * Notes:
 * Internal per-ring state intended for debug use only - tim.ctl[47:0]
 * This CSR is a memory of 16 entries, and thus, the TIM_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.
 */
union cvmx_tim_mem_debug0 {
	uint64_t u64;
	struct cvmx_tim_mem_debug0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t ena                          : 1;  /**< Ring timer enable */
	uint64_t reserved_46_46               : 1;
	uint64_t count                        : 22; /**< Time offset for the ring
                                                         Set to INTERVAL and counts down by 1 every 1024
                                                         cycles when ENA==1. The HW forces a bucket
                                                         traversal (and resets COUNT to INTERVAL) whenever
                                                         the decrement would cause COUNT to go negative.
                                                         COUNT is unpredictable whenever ENA==0.
                                                         COUNT is reset to INTERVAL whenever TIM_MEM_RING1
                                                         is written for the ring. */
	uint64_t reserved_22_23               : 2;
	uint64_t interval                     : 22; /**< Timer interval - 1 */
#else
	uint64_t interval                     : 22;
	uint64_t reserved_22_23               : 2;
	uint64_t count                        : 22;
	uint64_t reserved_46_46               : 1;
	uint64_t ena                          : 1;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_tim_mem_debug0_s          cn30xx;
	struct cvmx_tim_mem_debug0_s          cn31xx;
	struct cvmx_tim_mem_debug0_s          cn38xx;
	struct cvmx_tim_mem_debug0_s          cn38xxp2;
	struct cvmx_tim_mem_debug0_s          cn50xx;
	struct cvmx_tim_mem_debug0_s          cn52xx;
	struct cvmx_tim_mem_debug0_s          cn52xxp1;
	struct cvmx_tim_mem_debug0_s          cn56xx;
	struct cvmx_tim_mem_debug0_s          cn56xxp1;
	struct cvmx_tim_mem_debug0_s          cn58xx;
	struct cvmx_tim_mem_debug0_s          cn58xxp1;
	struct cvmx_tim_mem_debug0_s          cn61xx;
	struct cvmx_tim_mem_debug0_s          cn63xx;
	struct cvmx_tim_mem_debug0_s          cn63xxp1;
	struct cvmx_tim_mem_debug0_s          cn66xx;
	struct cvmx_tim_mem_debug0_s          cn70xx;
	struct cvmx_tim_mem_debug0_s          cn70xxp1;
	struct cvmx_tim_mem_debug0_s          cnf71xx;
};
typedef union cvmx_tim_mem_debug0 cvmx_tim_mem_debug0_t;

/**
 * cvmx_tim_mem_debug1
 *
 * Notes:
 * Internal per-ring state intended for debug use only - tim.sta[63:0]
 * This CSR is a memory of 16 entries, and thus, the TIM_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.
 */
union cvmx_tim_mem_debug1 {
	uint64_t u64;
	struct cvmx_tim_mem_debug1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bucket                       : 13; /**< Current bucket[12:0]
                                                         Reset to 0 whenever TIM_MEM_RING0 is written for
                                                         the ring. Incremented (modulo BSIZE) once per
                                                         bucket traversal.
                                                         See TIM_MEM_DEBUG2[BUCKET]. */
	uint64_t base                         : 31; /**< Pointer[35:5] to bucket[0] */
	uint64_t bsize                        : 20; /**< Number of buckets - 1 */
#else
	uint64_t bsize                        : 20;
	uint64_t base                         : 31;
	uint64_t bucket                       : 13;
#endif
	} s;
	struct cvmx_tim_mem_debug1_s          cn30xx;
	struct cvmx_tim_mem_debug1_s          cn31xx;
	struct cvmx_tim_mem_debug1_s          cn38xx;
	struct cvmx_tim_mem_debug1_s          cn38xxp2;
	struct cvmx_tim_mem_debug1_s          cn50xx;
	struct cvmx_tim_mem_debug1_s          cn52xx;
	struct cvmx_tim_mem_debug1_s          cn52xxp1;
	struct cvmx_tim_mem_debug1_s          cn56xx;
	struct cvmx_tim_mem_debug1_s          cn56xxp1;
	struct cvmx_tim_mem_debug1_s          cn58xx;
	struct cvmx_tim_mem_debug1_s          cn58xxp1;
	struct cvmx_tim_mem_debug1_s          cn61xx;
	struct cvmx_tim_mem_debug1_s          cn63xx;
	struct cvmx_tim_mem_debug1_s          cn63xxp1;
	struct cvmx_tim_mem_debug1_s          cn66xx;
	struct cvmx_tim_mem_debug1_s          cn70xx;
	struct cvmx_tim_mem_debug1_s          cn70xxp1;
	struct cvmx_tim_mem_debug1_s          cnf71xx;
};
typedef union cvmx_tim_mem_debug1 cvmx_tim_mem_debug1_t;

/**
 * cvmx_tim_mem_debug2
 *
 * Notes:
 * Internal per-ring state intended for debug use only - tim.sta[95:64]
 * This CSR is a memory of 16 entries, and thus, the TIM_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.
 */
union cvmx_tim_mem_debug2 {
	uint64_t u64;
	struct cvmx_tim_mem_debug2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t cpool                        : 3;  /**< Free list used to free chunks */
	uint64_t csize                        : 13; /**< Number of words per chunk */
	uint64_t reserved_7_7                 : 1;
	uint64_t bucket                       : 7;  /**< Current bucket[19:13]
                                                         See TIM_MEM_DEBUG1[BUCKET]. */
#else
	uint64_t bucket                       : 7;
	uint64_t reserved_7_7                 : 1;
	uint64_t csize                        : 13;
	uint64_t cpool                        : 3;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_tim_mem_debug2_s          cn30xx;
	struct cvmx_tim_mem_debug2_s          cn31xx;
	struct cvmx_tim_mem_debug2_s          cn38xx;
	struct cvmx_tim_mem_debug2_s          cn38xxp2;
	struct cvmx_tim_mem_debug2_s          cn50xx;
	struct cvmx_tim_mem_debug2_s          cn52xx;
	struct cvmx_tim_mem_debug2_s          cn52xxp1;
	struct cvmx_tim_mem_debug2_s          cn56xx;
	struct cvmx_tim_mem_debug2_s          cn56xxp1;
	struct cvmx_tim_mem_debug2_s          cn58xx;
	struct cvmx_tim_mem_debug2_s          cn58xxp1;
	struct cvmx_tim_mem_debug2_s          cn61xx;
	struct cvmx_tim_mem_debug2_s          cn63xx;
	struct cvmx_tim_mem_debug2_s          cn63xxp1;
	struct cvmx_tim_mem_debug2_s          cn66xx;
	struct cvmx_tim_mem_debug2_s          cn70xx;
	struct cvmx_tim_mem_debug2_s          cn70xxp1;
	struct cvmx_tim_mem_debug2_s          cnf71xx;
};
typedef union cvmx_tim_mem_debug2 cvmx_tim_mem_debug2_t;

/**
 * cvmx_tim_mem_ring0
 *
 * Notes:
 * TIM_MEM_RING0 must not be written for a ring when TIM_MEM_RING1[ENA] is set for the ring.
 * Every write to TIM_MEM_RING0 clears the current bucket for the ring. (The current bucket is
 * readable via TIM_MEM_DEBUG2[BUCKET],TIM_MEM_DEBUG1[BUCKET].)
 * BASE is a 32-byte aligned pointer[35:0].  Only pointer[35:5] are stored because pointer[4:0] = 0.
 * This CSR is a memory of 16 entries, and thus, the TIM_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.
 */
union cvmx_tim_mem_ring0 {
	uint64_t u64;
	struct cvmx_tim_mem_ring0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_55_63               : 9;
	uint64_t first_bucket                 : 31; /**< Pointer[35:5] to bucket[0] */
	uint64_t num_buckets                  : 20; /**< Number of buckets - 1 */
	uint64_t ring                         : 4;  /**< Ring ID */
#else
	uint64_t ring                         : 4;
	uint64_t num_buckets                  : 20;
	uint64_t first_bucket                 : 31;
	uint64_t reserved_55_63               : 9;
#endif
	} s;
	struct cvmx_tim_mem_ring0_s           cn30xx;
	struct cvmx_tim_mem_ring0_s           cn31xx;
	struct cvmx_tim_mem_ring0_s           cn38xx;
	struct cvmx_tim_mem_ring0_s           cn38xxp2;
	struct cvmx_tim_mem_ring0_s           cn50xx;
	struct cvmx_tim_mem_ring0_s           cn52xx;
	struct cvmx_tim_mem_ring0_s           cn52xxp1;
	struct cvmx_tim_mem_ring0_s           cn56xx;
	struct cvmx_tim_mem_ring0_s           cn56xxp1;
	struct cvmx_tim_mem_ring0_s           cn58xx;
	struct cvmx_tim_mem_ring0_s           cn58xxp1;
	struct cvmx_tim_mem_ring0_s           cn61xx;
	struct cvmx_tim_mem_ring0_s           cn63xx;
	struct cvmx_tim_mem_ring0_s           cn63xxp1;
	struct cvmx_tim_mem_ring0_s           cn66xx;
	struct cvmx_tim_mem_ring0_s           cn70xx;
	struct cvmx_tim_mem_ring0_s           cn70xxp1;
	struct cvmx_tim_mem_ring0_s           cnf71xx;
};
typedef union cvmx_tim_mem_ring0 cvmx_tim_mem_ring0_t;

/**
 * cvmx_tim_mem_ring1
 *
 * Notes:
 * After a 1->0 transition on ENA, the HW will still complete a bucket traversal for the ring
 * if it was pending or active prior to the transition. (SW must delay to ensure the completion
 * of the traversal before reprogramming the ring.)
 * Every write to TIM_MEM_RING1 resets the current time offset for the ring to the INTERVAL value.
 * (The current time offset for the ring is readable via TIM_MEM_DEBUG0[COUNT].)
 * CSIZE must be at least 16.  It is illegal to program CSIZE to a value that is less than 16.
 * This CSR is a memory of 16 entries, and thus, the TIM_REG_READ_IDX CSR must be written before any
 * CSR read operations to this address can be performed.
 */
union cvmx_tim_mem_ring1 {
	uint64_t u64;
	struct cvmx_tim_mem_ring1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_43_63               : 21;
	uint64_t enable                       : 1;  /**< Ring timer enable
                                                         When clear, the ring is disabled and TIM
                                                         will not traverse any new buckets for the ring. */
	uint64_t pool                         : 3;  /**< Free list used to free chunks */
	uint64_t words_per_chunk              : 13; /**< Number of words per chunk */
	uint64_t interval                     : 22; /**< Timer interval - 1, measured in 1024 cycle ticks */
	uint64_t ring                         : 4;  /**< Ring ID */
#else
	uint64_t ring                         : 4;
	uint64_t interval                     : 22;
	uint64_t words_per_chunk              : 13;
	uint64_t pool                         : 3;
	uint64_t enable                       : 1;
	uint64_t reserved_43_63               : 21;
#endif
	} s;
	struct cvmx_tim_mem_ring1_s           cn30xx;
	struct cvmx_tim_mem_ring1_s           cn31xx;
	struct cvmx_tim_mem_ring1_s           cn38xx;
	struct cvmx_tim_mem_ring1_s           cn38xxp2;
	struct cvmx_tim_mem_ring1_s           cn50xx;
	struct cvmx_tim_mem_ring1_s           cn52xx;
	struct cvmx_tim_mem_ring1_s           cn52xxp1;
	struct cvmx_tim_mem_ring1_s           cn56xx;
	struct cvmx_tim_mem_ring1_s           cn56xxp1;
	struct cvmx_tim_mem_ring1_s           cn58xx;
	struct cvmx_tim_mem_ring1_s           cn58xxp1;
	struct cvmx_tim_mem_ring1_s           cn61xx;
	struct cvmx_tim_mem_ring1_s           cn63xx;
	struct cvmx_tim_mem_ring1_s           cn63xxp1;
	struct cvmx_tim_mem_ring1_s           cn66xx;
	struct cvmx_tim_mem_ring1_s           cn70xx;
	struct cvmx_tim_mem_ring1_s           cn70xxp1;
	struct cvmx_tim_mem_ring1_s           cnf71xx;
};
typedef union cvmx_tim_mem_ring1 cvmx_tim_mem_ring1_t;

/**
 * cvmx_tim_reg_bist_result
 *
 * Notes:
 * Access to the internal BiST results
 * Each bit is the BiST result of an individual memory (per bit, 0=pass and 1=fail).
 */
union cvmx_tim_reg_bist_result {
	uint64_t u64;
	struct cvmx_tim_reg_bist_result_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t sta                          : 2;  /**< BiST result of the STA   memories (0=pass, !0=fail) */
	uint64_t ncb                          : 1;  /**< BiST result of the NCB   memories (0=pass, !0=fail) */
	uint64_t ctl                          : 1;  /**< BiST result of the CTL   memories (0=pass, !0=fail) */
#else
	uint64_t ctl                          : 1;
	uint64_t ncb                          : 1;
	uint64_t sta                          : 2;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_tim_reg_bist_result_s     cn30xx;
	struct cvmx_tim_reg_bist_result_s     cn31xx;
	struct cvmx_tim_reg_bist_result_s     cn38xx;
	struct cvmx_tim_reg_bist_result_s     cn38xxp2;
	struct cvmx_tim_reg_bist_result_s     cn50xx;
	struct cvmx_tim_reg_bist_result_s     cn52xx;
	struct cvmx_tim_reg_bist_result_s     cn52xxp1;
	struct cvmx_tim_reg_bist_result_s     cn56xx;
	struct cvmx_tim_reg_bist_result_s     cn56xxp1;
	struct cvmx_tim_reg_bist_result_s     cn58xx;
	struct cvmx_tim_reg_bist_result_s     cn58xxp1;
	struct cvmx_tim_reg_bist_result_s     cn61xx;
	struct cvmx_tim_reg_bist_result_s     cn63xx;
	struct cvmx_tim_reg_bist_result_s     cn63xxp1;
	struct cvmx_tim_reg_bist_result_s     cn66xx;
	struct cvmx_tim_reg_bist_result_s     cn70xx;
	struct cvmx_tim_reg_bist_result_s     cn70xxp1;
	struct cvmx_tim_reg_bist_result_s     cnf71xx;
};
typedef union cvmx_tim_reg_bist_result cvmx_tim_reg_bist_result_t;

/**
 * cvmx_tim_reg_error
 *
 * A ring is in error if its interval has elapsed more than once without having been serviced,
 * either due to too many events in this ring's previous interval, or another ring having too
 * many events to process within this ring's interval. This is usually a programming error where
 * the number of entries in the bucket is too large for the interval specified across the rings.
 * When in error, TIM may process events in an interval later than requested. Any bit in the INT
 * field should be cleared by writing one to it.
 */
union cvmx_tim_reg_error {
	uint64_t u64;
	struct cvmx_tim_reg_error_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t mask                         : 16; /**< Bit mask indicating the rings in error */
#else
	uint64_t mask                         : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_tim_reg_error_s           cn30xx;
	struct cvmx_tim_reg_error_s           cn31xx;
	struct cvmx_tim_reg_error_s           cn38xx;
	struct cvmx_tim_reg_error_s           cn38xxp2;
	struct cvmx_tim_reg_error_s           cn50xx;
	struct cvmx_tim_reg_error_s           cn52xx;
	struct cvmx_tim_reg_error_s           cn52xxp1;
	struct cvmx_tim_reg_error_s           cn56xx;
	struct cvmx_tim_reg_error_s           cn56xxp1;
	struct cvmx_tim_reg_error_s           cn58xx;
	struct cvmx_tim_reg_error_s           cn58xxp1;
	struct cvmx_tim_reg_error_s           cn61xx;
	struct cvmx_tim_reg_error_s           cn63xx;
	struct cvmx_tim_reg_error_s           cn63xxp1;
	struct cvmx_tim_reg_error_s           cn66xx;
	struct cvmx_tim_reg_error_s           cn70xx;
	struct cvmx_tim_reg_error_s           cn70xxp1;
	struct cvmx_tim_reg_error_s           cnf71xx;
};
typedef union cvmx_tim_reg_error cvmx_tim_reg_error_t;

/**
 * cvmx_tim_reg_flags
 *
 * This register provides flags for TIM.
 *
 */
union cvmx_tim_reg_flags {
	uint64_t u64;
	struct cvmx_tim_reg_flags_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t gpio_edge                    : 2;  /**< Edge used for GPIO timing.
                                                         0x0 = no edges and the timer tick is not generated.
                                                         0x1 = TIM counts low to high transitions.
                                                         0x2 = TIM counts high to low transitions.
                                                         0x3 = TIM counts both low to high and high to low transitions. */
	uint64_t ena_gpio                     : 1;  /**< Enable the external control of GPIO over the free
                                                         running timer.
                                                         When set, free running timer will be driven by GPIO.
                                                         Free running timer will count posedge or negedge of the
                                                         GPIO pin based on GPIO_EDGE register. */
	uint64_t ena_dfb                      : 1;  /**< Enable Don't Free Buffer. When set chunk buffer
                                                         would not be released by the TIM back to FPA. */
	uint64_t reset                        : 1;  /**< Reset oneshot pulse for free-running structures */
	uint64_t enable_dwb                   : 1;  /**< Enables non-zero DonwWriteBacks when set
                                                         When set, enables the use of
                                                         DontWriteBacks during the buffer freeing
                                                         operations. */
	uint64_t enable_timers                : 1;  /**< Enables the TIM section when set
                                                         When set, TIM is in normal operation.
                                                         When clear, time is effectively stopped for all
                                                         rings in TIM. */
#else
	uint64_t enable_timers                : 1;
	uint64_t enable_dwb                   : 1;
	uint64_t reset                        : 1;
	uint64_t ena_dfb                      : 1;
	uint64_t ena_gpio                     : 1;
	uint64_t gpio_edge                    : 2;
	uint64_t reserved_7_63                : 57;
#endif
	} s;
	struct cvmx_tim_reg_flags_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t reset                        : 1;  /**< Reset oneshot pulse for free-running structures */
	uint64_t enable_dwb                   : 1;  /**< Enables non-zero DonwWriteBacks when set
                                                         When set, enables the use of
                                                         DontWriteBacks during the buffer freeing
                                                         operations. */
	uint64_t enable_timers                : 1;  /**< Enables the TIM section when set
                                                         When set, TIM is in normal operation.
                                                         When clear, time is effectively stopped for all
                                                         rings in TIM. */
#else
	uint64_t enable_timers                : 1;
	uint64_t enable_dwb                   : 1;
	uint64_t reset                        : 1;
	uint64_t reserved_3_63                : 61;
#endif
	} cn30xx;
	struct cvmx_tim_reg_flags_cn30xx      cn31xx;
	struct cvmx_tim_reg_flags_cn30xx      cn38xx;
	struct cvmx_tim_reg_flags_cn30xx      cn38xxp2;
	struct cvmx_tim_reg_flags_cn30xx      cn50xx;
	struct cvmx_tim_reg_flags_cn30xx      cn52xx;
	struct cvmx_tim_reg_flags_cn30xx      cn52xxp1;
	struct cvmx_tim_reg_flags_cn30xx      cn56xx;
	struct cvmx_tim_reg_flags_cn30xx      cn56xxp1;
	struct cvmx_tim_reg_flags_cn30xx      cn58xx;
	struct cvmx_tim_reg_flags_cn30xx      cn58xxp1;
	struct cvmx_tim_reg_flags_cn30xx      cn61xx;
	struct cvmx_tim_reg_flags_cn30xx      cn63xx;
	struct cvmx_tim_reg_flags_cn30xx      cn63xxp1;
	struct cvmx_tim_reg_flags_cn30xx      cn66xx;
	struct cvmx_tim_reg_flags_s           cn68xx;
	struct cvmx_tim_reg_flags_s           cn68xxp1;
	struct cvmx_tim_reg_flags_cn30xx      cn70xx;
	struct cvmx_tim_reg_flags_cn30xx      cn70xxp1;
	struct cvmx_tim_reg_flags_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t gpio_edge                    : 2;  /**< Edge used for GPIO timing.
                                                         0x0 = no edges and the timer tick is not generated.
                                                         0x1 = TIM counts low to high transitions.
                                                         0x2 = TIM counts high to low transitions.
                                                         0x3 = TIM counts both low to high and high to low transitions. */
	uint64_t reserved_3_4                 : 2;
	uint64_t reset                        : 1;  /**< Reset. One-shot pulse for free-running timer FR_RN_HT. */
	uint64_t reserved_1_1                 : 1;
	uint64_t enable_timers                : 1;  /**< When set, TIM is in normal operation. When clear, time is effectively stopped for all
                                                         rings in TIM.
                                                         TIM has a counter (see FR_RN_HT) that causes a periodic tick. This counter is shared by
                                                         all rings. Each Timer tick causes the hardware to decrement the time count for all enabled
                                                         rings.
                                                         When ENA_TIM = 0, the hardware stops the shared periodic counter, FR_RN_HT, so there are
                                                         no more ticks, and there are no more new bucket traversals.
                                                         If ENA_TIM transitions 1->0, TIM longer creates new bucket traversals, but does traverse
                                                         any rings that previously expired and are pending hardware traversal. */
#else
	uint64_t enable_timers                : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t reset                        : 1;
	uint64_t reserved_3_4                 : 2;
	uint64_t gpio_edge                    : 2;
	uint64_t reserved_7_63                : 57;
#endif
	} cn78xx;
	struct cvmx_tim_reg_flags_cn30xx      cnf71xx;
};
typedef union cvmx_tim_reg_flags cvmx_tim_reg_flags_t;

/**
 * cvmx_tim_reg_int_mask
 *
 * Notes:
 * Note that this CSR is present only in chip revisions beginning with pass2.
 * When mask bit is set, the interrupt is enabled.
 */
union cvmx_tim_reg_int_mask {
	uint64_t u64;
	struct cvmx_tim_reg_int_mask_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t mask                         : 16; /**< Bit mask corresponding to TIM_REG_ERROR.MASK above */
#else
	uint64_t mask                         : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_tim_reg_int_mask_s        cn30xx;
	struct cvmx_tim_reg_int_mask_s        cn31xx;
	struct cvmx_tim_reg_int_mask_s        cn38xx;
	struct cvmx_tim_reg_int_mask_s        cn38xxp2;
	struct cvmx_tim_reg_int_mask_s        cn50xx;
	struct cvmx_tim_reg_int_mask_s        cn52xx;
	struct cvmx_tim_reg_int_mask_s        cn52xxp1;
	struct cvmx_tim_reg_int_mask_s        cn56xx;
	struct cvmx_tim_reg_int_mask_s        cn56xxp1;
	struct cvmx_tim_reg_int_mask_s        cn58xx;
	struct cvmx_tim_reg_int_mask_s        cn58xxp1;
	struct cvmx_tim_reg_int_mask_s        cn61xx;
	struct cvmx_tim_reg_int_mask_s        cn63xx;
	struct cvmx_tim_reg_int_mask_s        cn63xxp1;
	struct cvmx_tim_reg_int_mask_s        cn66xx;
	struct cvmx_tim_reg_int_mask_s        cn70xx;
	struct cvmx_tim_reg_int_mask_s        cn70xxp1;
	struct cvmx_tim_reg_int_mask_s        cnf71xx;
};
typedef union cvmx_tim_reg_int_mask cvmx_tim_reg_int_mask_t;

/**
 * cvmx_tim_reg_read_idx
 *
 * Notes:
 * Provides the read index during a CSR read operation to any of the CSRs that are physically stored
 * as memories.  The names of these CSRs begin with the prefix "TIM_MEM_".
 * IDX[7:0] is the read index.  INC[7:0] is an increment that is added to IDX[7:0] after any CSR read.
 * The intended use is to initially write this CSR such that IDX=0 and INC=1.  Then, the entire
 * contents of a CSR memory can be read with consecutive CSR read commands.
 */
union cvmx_tim_reg_read_idx {
	uint64_t u64;
	struct cvmx_tim_reg_read_idx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t inc                          : 8;  /**< Increment to add to current index for next index */
	uint64_t index                        : 8;  /**< Index to use for next memory CSR read */
#else
	uint64_t index                        : 8;
	uint64_t inc                          : 8;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_tim_reg_read_idx_s        cn30xx;
	struct cvmx_tim_reg_read_idx_s        cn31xx;
	struct cvmx_tim_reg_read_idx_s        cn38xx;
	struct cvmx_tim_reg_read_idx_s        cn38xxp2;
	struct cvmx_tim_reg_read_idx_s        cn50xx;
	struct cvmx_tim_reg_read_idx_s        cn52xx;
	struct cvmx_tim_reg_read_idx_s        cn52xxp1;
	struct cvmx_tim_reg_read_idx_s        cn56xx;
	struct cvmx_tim_reg_read_idx_s        cn56xxp1;
	struct cvmx_tim_reg_read_idx_s        cn58xx;
	struct cvmx_tim_reg_read_idx_s        cn58xxp1;
	struct cvmx_tim_reg_read_idx_s        cn61xx;
	struct cvmx_tim_reg_read_idx_s        cn63xx;
	struct cvmx_tim_reg_read_idx_s        cn63xxp1;
	struct cvmx_tim_reg_read_idx_s        cn66xx;
	struct cvmx_tim_reg_read_idx_s        cn70xx;
	struct cvmx_tim_reg_read_idx_s        cn70xxp1;
	struct cvmx_tim_reg_read_idx_s        cnf71xx;
};
typedef union cvmx_tim_reg_read_idx cvmx_tim_reg_read_idx_t;

/**
 * cvmx_tim_ring#_aura
 */
union cvmx_tim_ringx_aura {
	uint64_t u64;
	struct cvmx_tim_ringx_aura_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t aura                         : 16; /**< Aura number used to free and return chucks to. Bits <15:12> must be zero. */
#else
	uint64_t aura                         : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_tim_ringx_aura_s          cn78xx;
};
typedef union cvmx_tim_ringx_aura cvmx_tim_ringx_aura_t;

/**
 * cvmx_tim_ring#_ctl0
 *
 * Notes:
 * This CSR is a memory of 64 entries
 * After a 1 to 0 transition on ENA, the HW will still complete a bucket traversal for the ring
 * if it was pending or active prior to the transition. (SW must delay to ensure the completion
 * of the traversal before reprogramming the ring.)
 */
union cvmx_tim_ringx_ctl0 {
	uint64_t u64;
	struct cvmx_tim_ringx_ctl0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_tim_ringx_ctl0_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_47_63               : 17;
	uint64_t ena                          : 1;  /**< Ring timer enable */
	uint64_t intc                         : 2;  /**< Interval count for Error. Defines how many intervals
                                                         could elapse from bucket expiration till actual
                                                         bucket traversal before HW asserts an error.
                                                         Typical value is 0,1,2. */
	uint64_t timercount                   : 22; /**< Timer Count represents the ring offset; how many timer
                                                         ticks have left till the interval expiration.
                                                         Typical initialization value should be Interval/Constant,
                                                         it is recommended that constant should be unique per ring
                                                         This will create an offset between the rings.
                                                         Once ENA is set,
                                                         TIMERCOUNT counts down timer ticks. When TIMERCOUNT
                                                         reaches zero, ring's interval expired and the HW forces
                                                         a bucket traversal (and resets TIMERCOUNT to INTERVAL)
                                                         TIMERCOUNT is unpredictable whenever ENA==0.
                                                         It is SW responsibility to set TIMERCOUNT before
                                                         TIM_RINGX_CTL0.ENA transitions from 0 to 1.
                                                         When the field is set to X it would take X+1 timer tick
                                                         for the interval to expire. */
	uint64_t interval                     : 22; /**< Timer interval. Measured in Timer Ticks, where timer
                                                         ticks are defined by TIM_FR_RN_TT.FR_RN_TT. */
#else
	uint64_t interval                     : 22;
	uint64_t timercount                   : 22;
	uint64_t intc                         : 2;
	uint64_t ena                          : 1;
	uint64_t reserved_47_63               : 17;
#endif
	} cn68xx;
	struct cvmx_tim_ringx_ctl0_cn68xx     cn68xxp1;
	struct cvmx_tim_ringx_ctl0_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t expire_offset                : 32; /**< Time at which the next bucket will be serviced, or offset. See also TIM_RING()_REL
                                                         for the position relative to current time.
                                                         If TIM_RING()_CTL1[ENA] = 0, then contains an offset. When ENA transitions from a
                                                         zero to a one this offset will be added to the current time and loaded back into
                                                         EXPIRE_OFFSET. Thus the offset sets the delta time between ENA transitioning to one and
                                                         the very first time the ring will be serviced. Software should program different offsets
                                                         on each ring to reduce congestion to prevent many rings from otherwise expiring
                                                         concurrently.
                                                         If TIM_RING()_CTL1[ENA] = 1, then contains the time the next bucket will be serviced.
                                                         When EXPIRE_OFFSET reaches the current time (TIM_FR_RN_CYCLES or TIM_FR_RN_GPIOS),
                                                         EXPIRE_OFFSET is set to the next expiration time (current time plus
                                                         TIM_RING()_CTL0[INTERVAL]).
                                                         EXPIRE_OFFSET is unpredictable after ENA_GPIO changes or TIM_RING()_CTL1[ENA]
                                                         transitions from 1 to 0, and must be reprogrammed before (re-) setting
                                                         TIM_RING()_CTL1[ENA]. */
	uint64_t interval                     : 32; /**< Timer interval, measured in cycles or GPIO transitions.
                                                         For every 64 entries in a bucket, the interval should be at least 1u. Minimal recommended
                                                         value is 1u. */
#else
	uint64_t interval                     : 32;
	uint64_t expire_offset                : 32;
#endif
	} cn78xx;
};
typedef union cvmx_tim_ringx_ctl0 cvmx_tim_ringx_ctl0_t;

/**
 * cvmx_tim_ring#_ctl1
 *
 * Notes:
 * This CSR is a memory of 64 entries
 * ***NOTE: Added fields in pass 2.0
 */
union cvmx_tim_ringx_ctl1 {
	uint64_t u64;
	struct cvmx_tim_ringx_ctl1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_51_63               : 13;
	uint64_t rcf_busy                     : 1;  /**< Ring reconfiguration busy. When ENA is cleared, this bit will remain set until hardware
                                                         completes the idling of the ring. ENA must not be re-enabled until clear. */
	uint64_t intc                         : 2;  /**< Interval count for error. Defines how many intervals could elapse from bucket expiration
                                                         until actual bucket traversal before hardware asserts an error. Typical value is 0x0, 0x1,
                                                         0x2. */
	uint64_t ena                          : 1;  /**< Ring timer enable. After a 1 to 0 transition on ENA, the hardware still completes a bucket
                                                         traversal for the ring if it were pending or active prior to the transition. When
                                                         clearing, software must delay until TIM_RING()_REL[RING_ESR] = 0 to ensure the
                                                         completion of the traversal before reprogramming the ring. When setting, RCF_BUSY must be
                                                         clear. */
	uint64_t ena_gpio                     : 1;  /**< When set, ring's timer tick is generated by the GPIO timer. The GPIO edge is defined by
                                                         TIM_REG_FLAGS[GPIO_EDGE]. The default value (zero) means that timer ticks are generated
                                                         from the internal timer. To change ENA_GPIO:
                                                         1. TIM_RING()_CTL1[ENA] is cleared.
                                                         2. [ENA_GPIO] is changed.
                                                         3. TIM_RING()_CTL0[EXPIRE_OFFSET] is reprogrammed appropriately.
                                                         4. TIM_RING()_CTL1[ENA] is set. */
	uint64_t ena_prd                      : 1;  /**< Enable periodic mode, which disables the memory write of zeros to NUM_ENTRIES and
                                                         CHUNK_REMAINDER when a bucket is traversed. In periodic mode ENA_DFB and ENA_LDWB must
                                                         also be clear. */
	uint64_t reserved_44_44               : 1;
	uint64_t ena_dfb                      : 1;  /**< Enable don't free buffer. When set, chunk buffer is not released by the TIM back to FPA. */
	uint64_t cpool                        : 3;  /**< FPA Free list to free chunks to. */
	uint64_t bucket                       : 20; /**< Current bucket. Should be set to 0x0 by software at enable time. Incremented once per
                                                         bucket traversal. */
	uint64_t bsize                        : 20; /**< Number of buckets minus one. If BSIZE = 0, there is only one bucket in the ring. */
#else
	uint64_t bsize                        : 20;
	uint64_t bucket                       : 20;
	uint64_t cpool                        : 3;
	uint64_t ena_dfb                      : 1;
	uint64_t reserved_44_44               : 1;
	uint64_t ena_prd                      : 1;
	uint64_t ena_gpio                     : 1;
	uint64_t ena                          : 1;
	uint64_t intc                         : 2;
	uint64_t rcf_busy                     : 1;
	uint64_t reserved_51_63               : 13;
#endif
	} s;
	struct cvmx_tim_ringx_ctl1_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_47_63               : 17;
	uint64_t ena_gpio                     : 1;  /**< When set, ring's timer tick will be generated by the
                                                         GPIO Timer. GPIO edge is defined by
                                                         TIM_REG_FLAGS.GPIO_EDGE
                                                         Default value zero means that timer ticks will
                                                         be genearated from the Internal Timer */
	uint64_t ena_prd                      : 1;  /**< Enable Periodic Mode which would disable the memory
                                                         write of zeros to num_entries and chunk_remainder
                                                         when a bucket is traveresed. */
	uint64_t ena_dwb                      : 1;  /**< When set, enables the use of Dont Write Back during
                                                         FPA buffer freeing operations */
	uint64_t ena_dfb                      : 1;  /**< Enable Don't Free Buffer. When set chunk buffer
                                                         would not be released by the TIM back to FPA. */
	uint64_t cpool                        : 3;  /**< FPA Free list to free chunks to. */
	uint64_t bucket                       : 20; /**< Current bucket. Should be set to zero by SW at
                                                         enable time.
                                                         Incremented once per bucket traversal. */
	uint64_t bsize                        : 20; /**< Number of buckets minus one. If BSIZE==0 there is
                                                         only one bucket in the ring. */
#else
	uint64_t bsize                        : 20;
	uint64_t bucket                       : 20;
	uint64_t cpool                        : 3;
	uint64_t ena_dfb                      : 1;
	uint64_t ena_dwb                      : 1;
	uint64_t ena_prd                      : 1;
	uint64_t ena_gpio                     : 1;
	uint64_t reserved_47_63               : 17;
#endif
	} cn68xx;
	struct cvmx_tim_ringx_ctl1_cn68xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_43_63               : 21;
	uint64_t cpool                        : 3;  /**< FPA Free list to free chunks to. */
	uint64_t bucket                       : 20; /**< Current bucket. Should be set to zero by SW at
                                                         enable time.
                                                         Incremented once per bucket traversal. */
	uint64_t bsize                        : 20; /**< Number of buckets minus one. If BSIZE==0 there is
                                                         only one bucket in the ring. */
#else
	uint64_t bsize                        : 20;
	uint64_t bucket                       : 20;
	uint64_t cpool                        : 3;
	uint64_t reserved_43_63               : 21;
#endif
	} cn68xxp1;
	struct cvmx_tim_ringx_ctl1_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_51_63               : 13;
	uint64_t rcf_busy                     : 1;  /**< Ring reconfiguration busy. When ENA is cleared, this bit will remain set until hardware
                                                         completes the idling of the ring. ENA must not be re-enabled until clear. */
	uint64_t intc                         : 2;  /**< Interval count for error. Defines how many intervals could elapse from bucket expiration
                                                         until actual bucket traversal before hardware asserts an error. Typical value is 0x0, 0x1,
                                                         0x2. */
	uint64_t ena                          : 1;  /**< Ring timer enable. After a 1 to 0 transition on ENA, the hardware still completes a bucket
                                                         traversal for the ring if it were pending or active prior to the transition. When
                                                         clearing, software must delay until TIM_RING()_REL[RING_ESR] = 0 to ensure the
                                                         completion of the traversal before reprogramming the ring. When setting, RCF_BUSY must be
                                                         clear. */
	uint64_t ena_gpio                     : 1;  /**< When set, ring's timer tick is generated by the GPIO timer. The GPIO edge is defined by
                                                         TIM_REG_FLAGS[GPIO_EDGE]. The default value (zero) means that timer ticks are generated
                                                         from the internal timer. To change ENA_GPIO:
                                                         1. TIM_RING()_CTL1[ENA] is cleared.
                                                         2. [ENA_GPIO] is changed.
                                                         3. TIM_RING()_CTL0[EXPIRE_OFFSET] is reprogrammed appropriately.
                                                         4. TIM_RING()_CTL1[ENA] is set. */
	uint64_t ena_prd                      : 1;  /**< Enable periodic mode, which disables the memory write of zeros to NUM_ENTRIES and
                                                         CHUNK_REMAINDER when a bucket is traversed. In periodic mode ENA_DFB and ENA_LDWB must
                                                         also be clear. */
	uint64_t ena_ldwb                     : 1;  /**< When set, enables the use of Load and Don't-Write-Back when reading timer entry cache lines. */
	uint64_t ena_dfb                      : 1;  /**< Enable don't free buffer. When set, chunk buffer is not released by the TIM back to FPA. */
	uint64_t reserved_40_42               : 3;
	uint64_t bucket                       : 20; /**< Current bucket. Should be set to 0x0 by software at enable time. Incremented once per
                                                         bucket traversal. */
	uint64_t bsize                        : 20; /**< Number of buckets minus one. If BSIZE = 0, there is only one bucket in the ring. */
#else
	uint64_t bsize                        : 20;
	uint64_t bucket                       : 20;
	uint64_t reserved_40_42               : 3;
	uint64_t ena_dfb                      : 1;
	uint64_t ena_ldwb                     : 1;
	uint64_t ena_prd                      : 1;
	uint64_t ena_gpio                     : 1;
	uint64_t ena                          : 1;
	uint64_t intc                         : 2;
	uint64_t rcf_busy                     : 1;
	uint64_t reserved_51_63               : 13;
#endif
	} cn78xx;
};
typedef union cvmx_tim_ringx_ctl1 cvmx_tim_ringx_ctl1_t;

/**
 * cvmx_tim_ring#_ctl2
 *
 * Notes:
 * BASE is a 32-byte aligned pointer[35:0].  Only pointer[35:5] are stored because pointer[4:0] = 0.
 * This CSR is a memory of 64 entries
 */
union cvmx_tim_ringx_ctl2 {
	uint64_t u64;
	struct cvmx_tim_ringx_ctl2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_tim_ringx_ctl2_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_47_63               : 17;
	uint64_t csize                        : 13; /**< Number of words per chunk. CSIZE mod(16) should be
                                                         zero. */
	uint64_t reserved_31_33               : 3;
	uint64_t base                         : 31; /**< Pointer[35:5] to bucket[0] */
#else
	uint64_t base                         : 31;
	uint64_t reserved_31_33               : 3;
	uint64_t csize                        : 13;
	uint64_t reserved_47_63               : 17;
#endif
	} cn68xx;
	struct cvmx_tim_ringx_ctl2_cn68xx     cn68xxp1;
	struct cvmx_tim_ringx_ctl2_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_53_63               : 11;
	uint64_t csize                        : 13; /**< Number of eight-byte words per chunk. CSIZE mod(16) should be zero. */
	uint64_t reserved_37_39               : 3;
	uint64_t base                         : 37; /**< Pointer<41:5> to bucket<0>. */
#else
	uint64_t base                         : 37;
	uint64_t reserved_37_39               : 3;
	uint64_t csize                        : 13;
	uint64_t reserved_53_63               : 11;
#endif
	} cn78xx;
};
typedef union cvmx_tim_ringx_ctl2 cvmx_tim_ringx_ctl2_t;

/**
 * cvmx_tim_ring#_dbg0
 */
union cvmx_tim_ringx_dbg0 {
	uint64_t u64;
	struct cvmx_tim_ringx_dbg0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t fr_rn_ht                     : 22; /**< Free Running Hardware Timer. Shared by all rings and is
                                                         used to generate the Timer Tick based on
                                                         FR_RN_TT. */
	uint64_t timercount                   : 22; /**< Timer Count represents the ring's offset.
                                                         Refer to TIM_RINGX_CTL0. */
	uint64_t cur_bucket                   : 20; /**< Current bucket. Indicates the ring's current bucket.
                                                         Refer to TIM_RINGX_CTL1.BUCKET. */
#else
	uint64_t cur_bucket                   : 20;
	uint64_t timercount                   : 22;
	uint64_t fr_rn_ht                     : 22;
#endif
	} s;
	struct cvmx_tim_ringx_dbg0_s          cn68xx;
	struct cvmx_tim_ringx_dbg0_s          cn68xxp1;
};
typedef union cvmx_tim_ringx_dbg0 cvmx_tim_ringx_dbg0_t;

/**
 * cvmx_tim_ring#_dbg1
 */
union cvmx_tim_ringx_dbg1 {
	uint64_t u64;
	struct cvmx_tim_ringx_dbg1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t ring_esr                     : 2;  /**< Ring Expiration Status Register.
                                                         This register hold the expiration status of the ring.
                                                         2'b00 - Ring was recently traversed.
                                                         2'b01 - Interval expired. Ring is queued to be traversed.
                                                         2'b10 - 1st interval expiration while ring is queued to be
                                                         traversed.
                                                         2'b11 - 2nd interval expiration while ring is queued to be
                                                         traversed. */
#else
	uint64_t ring_esr                     : 2;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_tim_ringx_dbg1_s          cn68xx;
	struct cvmx_tim_ringx_dbg1_s          cn68xxp1;
};
typedef union cvmx_tim_ringx_dbg1 cvmx_tim_ringx_dbg1_t;

/**
 * cvmx_tim_ring#_rel
 *
 * Current positions of the TIM walker in both time and ring position, for easy synchronization
 * with software.
 */
union cvmx_tim_ringx_rel {
	uint64_t u64;
	struct cvmx_tim_ringx_rel_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t cur_bucket                   : 20; /**< Current bucket. Indicates the ring's current bucket. See TIM_RING()_CTL1[BUCKET]. */
	uint64_t reserved_34_43               : 10;
	uint64_t ring_esr                     : 2;  /**< Ring expiration status register. These registers hold the expiration status of the ring.
                                                         0x0 = Ring has not expired.
                                                         0x1 = Interval expired. Ring is queued to be traversed.
                                                         0x2 = First interval expiration while ring is queued to be traversed.
                                                         0x3 = Second interval expiration while ring is queued to be traversed.
                                                         This field is zeroed when TIM_RING()_CTL1[ENA] transitions from 0 to 1. */
	uint64_t timercount                   : 32; /**< Timer count indicates how many timer ticks are left until the interval expiration,
                                                         calculated as TIM_RING()_CTL0[EXPIRE_OFFSET] minus current time (TIM_FR_RN_CYCLES or
                                                         TIM_FR_RN_GPIOS).
                                                         Once ENA = 1, TIMERCOUNT will be observed to count down timer ticks. When TIMERCOUNT
                                                         reaches 0x0, the ring's interval expired and the hardware forces a bucket traversal (and
                                                         increments RING_ESR).
                                                         Typical initialization value should be interval/constant; Cavium recommends that the
                                                         constant be unique per ring. This creates an offset between the rings.
                                                         TIMERCOUNT becomes and remains unpredictable whenever ENA = 0 or ENA_GPIO changes. It is
                                                         software's responsibility to set TIMERCOUNT before TIM_RING()_CTL1[ENA] transitions
                                                         from 0 -> 1. */
#else
	uint64_t timercount                   : 32;
	uint64_t ring_esr                     : 2;
	uint64_t reserved_34_43               : 10;
	uint64_t cur_bucket                   : 20;
#endif
	} s;
	struct cvmx_tim_ringx_rel_s           cn78xx;
};
typedef union cvmx_tim_ringx_rel cvmx_tim_ringx_rel_t;

#endif
