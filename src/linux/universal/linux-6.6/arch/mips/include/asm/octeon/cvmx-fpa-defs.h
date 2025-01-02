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
 * cvmx-fpa-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon fpa.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_FPA_DEFS_H__
#define __CVMX_FPA_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_FPA_ADDR_RANGE_ERROR CVMX_FPA_ADDR_RANGE_ERROR_FUNC()
static inline uint64_t CVMX_FPA_ADDR_RANGE_ERROR_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001180028000458ull);
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return CVMX_ADD_IO_SEG(0x0001280000000458ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return CVMX_ADD_IO_SEG(0x0001280000000458ull);
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001280000000458ull);
			break;
	}
	cvmx_warn("CVMX_FPA_ADDR_RANGE_ERROR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001280000000458ull);
}
#else
#define CVMX_FPA_ADDR_RANGE_ERROR CVMX_FPA_ADDR_RANGE_ERROR_FUNC()
static inline uint64_t CVMX_FPA_ADDR_RANGE_ERROR_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001180028000458ull);
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return CVMX_ADD_IO_SEG(0x0001280000000458ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return CVMX_ADD_IO_SEG(0x0001280000000458ull);
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001280000000458ull);
	}
	return CVMX_ADD_IO_SEG(0x0001280000000458ull);
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_FPA_AURAX_CFG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 511))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_FPA_AURAX_CFG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001280020100000ull) + ((offset) & 1023) * 8;
}
#else
#define CVMX_FPA_AURAX_CFG(offset) (CVMX_ADD_IO_SEG(0x0001280020100000ull) + ((offset) & 1023) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_FPA_AURAX_CNT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 511))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_FPA_AURAX_CNT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001280020200000ull) + ((offset) & 1023) * 8;
}
#else
#define CVMX_FPA_AURAX_CNT(offset) (CVMX_ADD_IO_SEG(0x0001280020200000ull) + ((offset) & 1023) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_FPA_AURAX_CNT_ADD(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 511))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_FPA_AURAX_CNT_ADD(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001280020300000ull) + ((offset) & 1023) * 8;
}
#else
#define CVMX_FPA_AURAX_CNT_ADD(offset) (CVMX_ADD_IO_SEG(0x0001280020300000ull) + ((offset) & 1023) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_FPA_AURAX_CNT_LEVELS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 511))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_FPA_AURAX_CNT_LEVELS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001280020800000ull) + ((offset) & 1023) * 8;
}
#else
#define CVMX_FPA_AURAX_CNT_LEVELS(offset) (CVMX_ADD_IO_SEG(0x0001280020800000ull) + ((offset) & 1023) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_FPA_AURAX_CNT_LIMIT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 511))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_FPA_AURAX_CNT_LIMIT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001280020400000ull) + ((offset) & 1023) * 8;
}
#else
#define CVMX_FPA_AURAX_CNT_LIMIT(offset) (CVMX_ADD_IO_SEG(0x0001280020400000ull) + ((offset) & 1023) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_FPA_AURAX_CNT_THRESHOLD(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 511))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_FPA_AURAX_CNT_THRESHOLD(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001280020500000ull) + ((offset) & 1023) * 8;
}
#else
#define CVMX_FPA_AURAX_CNT_THRESHOLD(offset) (CVMX_ADD_IO_SEG(0x0001280020500000ull) + ((offset) & 1023) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_FPA_AURAX_INT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 511))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_FPA_AURAX_INT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001280020600000ull) + ((offset) & 1023) * 8;
}
#else
#define CVMX_FPA_AURAX_INT(offset) (CVMX_ADD_IO_SEG(0x0001280020600000ull) + ((offset) & 1023) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_FPA_AURAX_POOL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 511))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_FPA_AURAX_POOL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001280020000000ull) + ((offset) & 1023) * 8;
}
#else
#define CVMX_FPA_AURAX_POOL(offset) (CVMX_ADD_IO_SEG(0x0001280020000000ull) + ((offset) & 1023) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_FPA_AURAX_POOL_LEVELS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 511))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 511)))))
		cvmx_warn("CVMX_FPA_AURAX_POOL_LEVELS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001280020700000ull) + ((offset) & 1023) * 8;
}
#else
#define CVMX_FPA_AURAX_POOL_LEVELS(offset) (CVMX_ADD_IO_SEG(0x0001280020700000ull) + ((offset) & 1023) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_FPA_BIST_STATUS CVMX_FPA_BIST_STATUS_FUNC()
static inline uint64_t CVMX_FPA_BIST_STATUS_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN30XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN50XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN38XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN31XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN58XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800280000E8ull);
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return CVMX_ADD_IO_SEG(0x00012800000000E8ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return CVMX_ADD_IO_SEG(0x00012800000000E8ull);
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00012800000000E8ull);
			break;
	}
	cvmx_warn("CVMX_FPA_BIST_STATUS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00012800000000E8ull);
}
#else
#define CVMX_FPA_BIST_STATUS CVMX_FPA_BIST_STATUS_FUNC()
static inline uint64_t CVMX_FPA_BIST_STATUS_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN56XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN30XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN50XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN38XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN31XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN52XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN58XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800280000E8ull);
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return CVMX_ADD_IO_SEG(0x00012800000000E8ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return CVMX_ADD_IO_SEG(0x00012800000000E8ull);
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00012800000000E8ull);
	}
	return CVMX_ADD_IO_SEG(0x00012800000000E8ull);
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_FPA_CLK_COUNT CVMX_FPA_CLK_COUNT_FUNC()
static inline uint64_t CVMX_FPA_CLK_COUNT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_FPA_CLK_COUNT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00012800000000F0ull);
}
#else
#define CVMX_FPA_CLK_COUNT (CVMX_ADD_IO_SEG(0x00012800000000F0ull))
#endif
#define CVMX_FPA_CTL_STATUS (CVMX_ADD_IO_SEG(0x0001180028000050ull))
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_FPA_ECC_CTL CVMX_FPA_ECC_CTL_FUNC()
static inline uint64_t CVMX_FPA_ECC_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_FPA_ECC_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001280000000058ull);
}
#else
#define CVMX_FPA_ECC_CTL (CVMX_ADD_IO_SEG(0x0001280000000058ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_FPA_ECC_INT CVMX_FPA_ECC_INT_FUNC()
static inline uint64_t CVMX_FPA_ECC_INT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_FPA_ECC_INT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001280000000068ull);
}
#else
#define CVMX_FPA_ECC_INT (CVMX_ADD_IO_SEG(0x0001280000000068ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_FPA_ERR_INT CVMX_FPA_ERR_INT_FUNC()
static inline uint64_t CVMX_FPA_ERR_INT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_FPA_ERR_INT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001280000000040ull);
}
#else
#define CVMX_FPA_ERR_INT (CVMX_ADD_IO_SEG(0x0001280000000040ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_FPA_FPF0_MARKS CVMX_FPA_FPF0_MARKS_FUNC()
static inline uint64_t CVMX_FPA_FPF0_MARKS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN38XX) || OCTEON_IS_MODEL(OCTEON_CN56XX) || OCTEON_IS_MODEL(OCTEON_CN58XX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_FPA_FPF0_MARKS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180028000000ull);
}
#else
#define CVMX_FPA_FPF0_MARKS (CVMX_ADD_IO_SEG(0x0001180028000000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_FPA_FPF0_SIZE CVMX_FPA_FPF0_SIZE_FUNC()
static inline uint64_t CVMX_FPA_FPF0_SIZE_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN38XX) || OCTEON_IS_MODEL(OCTEON_CN56XX) || OCTEON_IS_MODEL(OCTEON_CN58XX) || OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_FPA_FPF0_SIZE not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180028000058ull);
}
#else
#define CVMX_FPA_FPF0_SIZE (CVMX_ADD_IO_SEG(0x0001180028000058ull))
#endif
#define CVMX_FPA_FPF1_MARKS CVMX_FPA_FPFX_MARKS(1)
#define CVMX_FPA_FPF2_MARKS CVMX_FPA_FPFX_MARKS(2)
#define CVMX_FPA_FPF3_MARKS CVMX_FPA_FPFX_MARKS(3)
#define CVMX_FPA_FPF4_MARKS CVMX_FPA_FPFX_MARKS(4)
#define CVMX_FPA_FPF5_MARKS CVMX_FPA_FPFX_MARKS(5)
#define CVMX_FPA_FPF6_MARKS CVMX_FPA_FPFX_MARKS(6)
#define CVMX_FPA_FPF7_MARKS CVMX_FPA_FPFX_MARKS(7)
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_FPA_FPF8_MARKS CVMX_FPA_FPF8_MARKS_FUNC()
static inline uint64_t CVMX_FPA_FPF8_MARKS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_FPA_FPF8_MARKS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180028000240ull);
}
#else
#define CVMX_FPA_FPF8_MARKS (CVMX_ADD_IO_SEG(0x0001180028000240ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_FPA_FPF8_SIZE CVMX_FPA_FPF8_SIZE_FUNC()
static inline uint64_t CVMX_FPA_FPF8_SIZE_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_FPA_FPF8_SIZE not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180028000248ull);
}
#else
#define CVMX_FPA_FPF8_SIZE (CVMX_ADD_IO_SEG(0x0001180028000248ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_FPA_FPFX_MARKS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN38XX) && (((offset >= 1) && (offset <= 7)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && (((offset >= 1) && (offset <= 7)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN58XX) && (((offset >= 1) && (offset <= 7)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && (((offset >= 1) && (offset <= 7)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && (((offset >= 1) && (offset <= 7)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && (((offset >= 1) && (offset <= 7)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && (((offset >= 1) && (offset <= 7)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset >= 1) && (offset <= 7)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && (((offset >= 1) && (offset <= 7))))))
		cvmx_warn("CVMX_FPA_FPFX_MARKS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180028000008ull) + ((offset) & 7) * 8 - 8*1;
}
#else
#define CVMX_FPA_FPFX_MARKS(offset) (CVMX_ADD_IO_SEG(0x0001180028000008ull) + ((offset) & 7) * 8 - 8*1)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_FPA_FPFX_SIZE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN38XX) && (((offset >= 1) && (offset <= 7)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && (((offset >= 1) && (offset <= 7)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN58XX) && (((offset >= 1) && (offset <= 7)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && (((offset >= 1) && (offset <= 7)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && (((offset >= 1) && (offset <= 7)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && (((offset >= 1) && (offset <= 7)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && (((offset >= 1) && (offset <= 7)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset >= 1) && (offset <= 7)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && (((offset >= 1) && (offset <= 7))))))
		cvmx_warn("CVMX_FPA_FPFX_SIZE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180028000060ull) + ((offset) & 7) * 8 - 8*1;
}
#else
#define CVMX_FPA_FPFX_SIZE(offset) (CVMX_ADD_IO_SEG(0x0001180028000060ull) + ((offset) & 7) * 8 - 8*1)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_FPA_GEN_CFG CVMX_FPA_GEN_CFG_FUNC()
static inline uint64_t CVMX_FPA_GEN_CFG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_FPA_GEN_CFG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001280000000050ull);
}
#else
#define CVMX_FPA_GEN_CFG (CVMX_ADD_IO_SEG(0x0001280000000050ull))
#endif
#define CVMX_FPA_INT_ENB (CVMX_ADD_IO_SEG(0x0001180028000048ull))
#define CVMX_FPA_INT_SUM (CVMX_ADD_IO_SEG(0x0001180028000040ull))
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_FPA_PACKET_THRESHOLD CVMX_FPA_PACKET_THRESHOLD_FUNC()
static inline uint64_t CVMX_FPA_PACKET_THRESHOLD_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_FPA_PACKET_THRESHOLD not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180028000460ull);
}
#else
#define CVMX_FPA_PACKET_THRESHOLD (CVMX_ADD_IO_SEG(0x0001180028000460ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_FPA_POOLX_AVAILABLE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_FPA_POOLX_AVAILABLE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001280010300000ull) + ((offset) & 63) * 8;
}
#else
#define CVMX_FPA_POOLX_AVAILABLE(offset) (CVMX_ADD_IO_SEG(0x0001280010300000ull) + ((offset) & 63) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_FPA_POOLX_CFG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_FPA_POOLX_CFG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001280010000000ull) + ((offset) & 63) * 8;
}
#else
#define CVMX_FPA_POOLX_CFG(offset) (CVMX_ADD_IO_SEG(0x0001280010000000ull) + ((offset) & 63) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_FPA_POOLX_END_ADDR(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
			if ((offset <= 7))
				return CVMX_ADD_IO_SEG(0x0001180028000358ull) + ((offset) & 7) * 8;
			break;
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 8))
				return CVMX_ADD_IO_SEG(0x0001180028000358ull) + ((offset) & 15) * 8;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 31))
				return CVMX_ADD_IO_SEG(0x0001280010600000ull) + ((offset) & 31) * 8;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 63))
					return CVMX_ADD_IO_SEG(0x0001280010600000ull) + ((offset) & 63) * 8;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 63))
					return CVMX_ADD_IO_SEG(0x0001280010600000ull) + ((offset) & 63) * 8;

			break;
	}
	cvmx_warn("CVMX_FPA_POOLX_END_ADDR (offset = %lu) not supported on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001280010600000ull) + ((offset) & 31) * 8;
}
#else
static inline uint64_t CVMX_FPA_POOLX_END_ADDR(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001180028000358ull) + (offset) * 8;
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001180028000358ull) + (offset) * 8;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001280010600000ull) + (offset) * 8;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return CVMX_ADD_IO_SEG(0x0001280010600000ull) + (offset) * 8;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return CVMX_ADD_IO_SEG(0x0001280010600000ull) + (offset) * 8;

	}
	return CVMX_ADD_IO_SEG(0x0001280010600000ull) + (offset) * 8;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_FPA_POOLX_FPF_MARKS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_FPA_POOLX_FPF_MARKS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001280010100000ull) + ((offset) & 63) * 8;
}
#else
#define CVMX_FPA_POOLX_FPF_MARKS(offset) (CVMX_ADD_IO_SEG(0x0001280010100000ull) + ((offset) & 63) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_FPA_POOLX_INT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_FPA_POOLX_INT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001280010A00000ull) + ((offset) & 63) * 8;
}
#else
#define CVMX_FPA_POOLX_INT(offset) (CVMX_ADD_IO_SEG(0x0001280010A00000ull) + ((offset) & 63) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_FPA_POOLX_OP_PC(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_FPA_POOLX_OP_PC(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001280010F00000ull) + ((offset) & 63) * 8;
}
#else
#define CVMX_FPA_POOLX_OP_PC(offset) (CVMX_ADD_IO_SEG(0x0001280010F00000ull) + ((offset) & 63) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_FPA_POOLX_STACK_ADDR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_FPA_POOLX_STACK_ADDR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001280010900000ull) + ((offset) & 63) * 8;
}
#else
#define CVMX_FPA_POOLX_STACK_ADDR(offset) (CVMX_ADD_IO_SEG(0x0001280010900000ull) + ((offset) & 63) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_FPA_POOLX_STACK_BASE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_FPA_POOLX_STACK_BASE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001280010700000ull) + ((offset) & 63) * 8;
}
#else
#define CVMX_FPA_POOLX_STACK_BASE(offset) (CVMX_ADD_IO_SEG(0x0001280010700000ull) + ((offset) & 63) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_FPA_POOLX_STACK_END(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_FPA_POOLX_STACK_END(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001280010800000ull) + ((offset) & 63) * 8;
}
#else
#define CVMX_FPA_POOLX_STACK_END(offset) (CVMX_ADD_IO_SEG(0x0001280010800000ull) + ((offset) & 63) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_FPA_POOLX_START_ADDR(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
			if ((offset <= 7))
				return CVMX_ADD_IO_SEG(0x0001180028000258ull) + ((offset) & 7) * 8;
			break;
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 8))
				return CVMX_ADD_IO_SEG(0x0001180028000258ull) + ((offset) & 15) * 8;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 31))
				return CVMX_ADD_IO_SEG(0x0001280010500000ull) + ((offset) & 31) * 8;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 63))
					return CVMX_ADD_IO_SEG(0x0001280010500000ull) + ((offset) & 63) * 8;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 63))
					return CVMX_ADD_IO_SEG(0x0001280010500000ull) + ((offset) & 63) * 8;

			break;
	}
	cvmx_warn("CVMX_FPA_POOLX_START_ADDR (offset = %lu) not supported on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001280010500000ull) + ((offset) & 31) * 8;
}
#else
static inline uint64_t CVMX_FPA_POOLX_START_ADDR(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001180028000258ull) + (offset) * 8;
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001180028000258ull) + (offset) * 8;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001280010500000ull) + (offset) * 8;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return CVMX_ADD_IO_SEG(0x0001280010500000ull) + (offset) * 8;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return CVMX_ADD_IO_SEG(0x0001280010500000ull) + (offset) * 8;

	}
	return CVMX_ADD_IO_SEG(0x0001280010500000ull) + (offset) * 8;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_FPA_POOLX_THRESHOLD(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
			if ((offset <= 7))
				return CVMX_ADD_IO_SEG(0x0001180028000140ull) + ((offset) & 7) * 8;
			break;
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 8))
				return CVMX_ADD_IO_SEG(0x0001180028000140ull) + ((offset) & 15) * 8;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 31))
				return CVMX_ADD_IO_SEG(0x0001280010400000ull) + ((offset) & 31) * 8;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 63))
					return CVMX_ADD_IO_SEG(0x0001280010400000ull) + ((offset) & 63) * 8;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 63))
					return CVMX_ADD_IO_SEG(0x0001280010400000ull) + ((offset) & 63) * 8;

			break;
	}
	cvmx_warn("CVMX_FPA_POOLX_THRESHOLD (offset = %lu) not supported on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001280010400000ull) + ((offset) & 31) * 8;
}
#else
static inline uint64_t CVMX_FPA_POOLX_THRESHOLD(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001180028000140ull) + (offset) * 8;
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001180028000140ull) + (offset) * 8;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001280010400000ull) + (offset) * 8;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return CVMX_ADD_IO_SEG(0x0001280010400000ull) + (offset) * 8;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return CVMX_ADD_IO_SEG(0x0001280010400000ull) + (offset) * 8;

	}
	return CVMX_ADD_IO_SEG(0x0001280010400000ull) + (offset) * 8;
}
#endif
#define CVMX_FPA_QUE0_PAGE_INDEX CVMX_FPA_QUEX_PAGE_INDEX(0)
#define CVMX_FPA_QUE1_PAGE_INDEX CVMX_FPA_QUEX_PAGE_INDEX(1)
#define CVMX_FPA_QUE2_PAGE_INDEX CVMX_FPA_QUEX_PAGE_INDEX(2)
#define CVMX_FPA_QUE3_PAGE_INDEX CVMX_FPA_QUEX_PAGE_INDEX(3)
#define CVMX_FPA_QUE4_PAGE_INDEX CVMX_FPA_QUEX_PAGE_INDEX(4)
#define CVMX_FPA_QUE5_PAGE_INDEX CVMX_FPA_QUEX_PAGE_INDEX(5)
#define CVMX_FPA_QUE6_PAGE_INDEX CVMX_FPA_QUEX_PAGE_INDEX(6)
#define CVMX_FPA_QUE7_PAGE_INDEX CVMX_FPA_QUEX_PAGE_INDEX(7)
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_FPA_QUE8_PAGE_INDEX CVMX_FPA_QUE8_PAGE_INDEX_FUNC()
static inline uint64_t CVMX_FPA_QUE8_PAGE_INDEX_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_FPA_QUE8_PAGE_INDEX not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180028000250ull);
}
#else
#define CVMX_FPA_QUE8_PAGE_INDEX (CVMX_ADD_IO_SEG(0x0001180028000250ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_FPA_QUEX_AVAILABLE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN30XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN31XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN38XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN50XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN58XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 8))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 7)))))
		cvmx_warn("CVMX_FPA_QUEX_AVAILABLE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180028000098ull) + ((offset) & 15) * 8;
}
#else
#define CVMX_FPA_QUEX_AVAILABLE(offset) (CVMX_ADD_IO_SEG(0x0001180028000098ull) + ((offset) & 15) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_FPA_QUEX_PAGE_INDEX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN30XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN31XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN38XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN50XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN58XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 7)))))
		cvmx_warn("CVMX_FPA_QUEX_PAGE_INDEX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800280000F0ull) + ((offset) & 7) * 8;
}
#else
#define CVMX_FPA_QUEX_PAGE_INDEX(offset) (CVMX_ADD_IO_SEG(0x00011800280000F0ull) + ((offset) & 7) * 8)
#endif
#define CVMX_FPA_QUE_ACT (CVMX_ADD_IO_SEG(0x0001180028000138ull))
#define CVMX_FPA_QUE_EXP (CVMX_ADD_IO_SEG(0x0001180028000130ull))
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_FPA_RD_LATENCY_PC CVMX_FPA_RD_LATENCY_PC_FUNC()
static inline uint64_t CVMX_FPA_RD_LATENCY_PC_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_FPA_RD_LATENCY_PC not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001280000000610ull);
}
#else
#define CVMX_FPA_RD_LATENCY_PC (CVMX_ADD_IO_SEG(0x0001280000000610ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_FPA_RD_REQ_PC CVMX_FPA_RD_REQ_PC_FUNC()
static inline uint64_t CVMX_FPA_RD_REQ_PC_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_FPA_RD_REQ_PC not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001280000000600ull);
}
#else
#define CVMX_FPA_RD_REQ_PC (CVMX_ADD_IO_SEG(0x0001280000000600ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_FPA_RED_DELAY CVMX_FPA_RED_DELAY_FUNC()
static inline uint64_t CVMX_FPA_RED_DELAY_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_FPA_RED_DELAY not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001280000000100ull);
}
#else
#define CVMX_FPA_RED_DELAY (CVMX_ADD_IO_SEG(0x0001280000000100ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_FPA_SFT_RST CVMX_FPA_SFT_RST_FUNC()
static inline uint64_t CVMX_FPA_SFT_RST_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_FPA_SFT_RST not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001280000000000ull);
}
#else
#define CVMX_FPA_SFT_RST (CVMX_ADD_IO_SEG(0x0001280000000000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_FPA_WART_CTL CVMX_FPA_WART_CTL_FUNC()
static inline uint64_t CVMX_FPA_WART_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN5XXX)))
		cvmx_warn("CVMX_FPA_WART_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800280000D8ull);
}
#else
#define CVMX_FPA_WART_CTL (CVMX_ADD_IO_SEG(0x00011800280000D8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_FPA_WART_STATUS CVMX_FPA_WART_STATUS_FUNC()
static inline uint64_t CVMX_FPA_WART_STATUS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN5XXX)))
		cvmx_warn("CVMX_FPA_WART_STATUS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800280000E0ull);
}
#else
#define CVMX_FPA_WART_STATUS (CVMX_ADD_IO_SEG(0x00011800280000E0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_FPA_WQE_THRESHOLD CVMX_FPA_WQE_THRESHOLD_FUNC()
static inline uint64_t CVMX_FPA_WQE_THRESHOLD_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_FPA_WQE_THRESHOLD not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180028000468ull);
}
#else
#define CVMX_FPA_WQE_THRESHOLD (CVMX_ADD_IO_SEG(0x0001180028000468ull))
#endif

/**
 * cvmx_fpa_addr_range_error
 *
 * When any FPA_POOL()_INT[RANGE] error occurs, this register is latched with additional
 * error information.
 */
union cvmx_fpa_addr_range_error {
	uint64_t u64;
	struct cvmx_fpa_addr_range_error_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_fpa_addr_range_error_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_38_63               : 26;
	uint64_t pool                         : 5;  /**< Pool address sent to. */
	uint64_t addr                         : 33; /**< Failing address. */
#else
	uint64_t addr                         : 33;
	uint64_t pool                         : 5;
	uint64_t reserved_38_63               : 26;
#endif
	} cn61xx;
	struct cvmx_fpa_addr_range_error_cn61xx cn66xx;
	struct cvmx_fpa_addr_range_error_cn61xx cn68xx;
	struct cvmx_fpa_addr_range_error_cn61xx cn68xxp1;
	struct cvmx_fpa_addr_range_error_cn61xx cn70xx;
	struct cvmx_fpa_addr_range_error_cn61xx cn70xxp1;
	struct cvmx_fpa_addr_range_error_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_54_63               : 10;
	uint64_t pool                         : 6;  /**< Pool that address was sent to. */
	uint64_t reserved_42_47               : 6;
	uint64_t addr                         : 42; /**< Failing address. */
#else
	uint64_t addr                         : 42;
	uint64_t reserved_42_47               : 6;
	uint64_t pool                         : 6;
	uint64_t reserved_54_63               : 10;
#endif
	} cn73xx;
	struct cvmx_fpa_addr_range_error_cn73xx cn78xx;
	struct cvmx_fpa_addr_range_error_cn73xx cn78xxp1;
	struct cvmx_fpa_addr_range_error_cn61xx cnf71xx;
	struct cvmx_fpa_addr_range_error_cn73xx cnf75xx;
};
typedef union cvmx_fpa_addr_range_error cvmx_fpa_addr_range_error_t;

/**
 * cvmx_fpa_aura#_cfg
 *
 * This register configures aura backpressure, etc.
 *
 */
union cvmx_fpa_aurax_cfg {
	uint64_t u64;
	struct cvmx_fpa_aurax_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t ptr_dis                      : 1;  /**< Disable aura tracking pointer allocates/returns.
                                                         0 = When FPA allocates a pointer from this aura (including coprocessor or core requests),
                                                         increment the count. When a pointer is returned to FPA for this aura (including
                                                         coprocessor or core requests), decrement the count. Note that PKI may prefetch buffers it
                                                         later returns, this may result in the count intermittently being higher than the number of
                                                         buffers actually in use by packets visible to software.
                                                         1 = Pointer allocations/returns will not automatically change the count.
                                                         Note specific requests to change the count, including FPA_AURA()_CNT_ADD,
                                                         PKO_SEND_AURA_S, or PKI_AURA()_CFG[PKT_ADD] will be applied regardless of the
                                                         setting of this bit. */
	uint64_t avg_con                      : 9;  /**< This value controls how much of each present average resource level is used to
                                                         calculate the new resource level. The value is a number from 0 to 256, which
                                                         represents [AVG_CON]/256 of the average resource level that will be used in the
                                                         calculation:
                                                         _  next_LEVEL = ([AVG_CON]/256) * prev_LEVEL
                                                         _  + (1-([AVG_CON]/256)) * count
                                                         Note setting this value to zero will disable averaging, and always use the most immediate
                                                         levels. FPA_GEN_CFG[AVG_EN] must be set and FPA_GEN_CFG[LVL_DLY] must be nonzero to
                                                         globally enable averaging. FPA_RED_DELAY[AVG_DLY] controls the periodicity of the level
                                                         calculations. */
#else
	uint64_t avg_con                      : 9;
	uint64_t ptr_dis                      : 1;
	uint64_t reserved_10_63               : 54;
#endif
	} s;
	struct cvmx_fpa_aurax_cfg_s           cn73xx;
	struct cvmx_fpa_aurax_cfg_s           cn78xx;
	struct cvmx_fpa_aurax_cfg_s           cn78xxp1;
	struct cvmx_fpa_aurax_cfg_s           cnf75xx;
};
typedef union cvmx_fpa_aurax_cfg cvmx_fpa_aurax_cfg_t;

/**
 * cvmx_fpa_aura#_cnt
 */
union cvmx_fpa_aurax_cnt {
	uint64_t u64;
	struct cvmx_fpa_aurax_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_40_63               : 24;
	uint64_t cnt                          : 40; /**< The current aura count. */
#else
	uint64_t cnt                          : 40;
	uint64_t reserved_40_63               : 24;
#endif
	} s;
	struct cvmx_fpa_aurax_cnt_s           cn73xx;
	struct cvmx_fpa_aurax_cnt_s           cn78xx;
	struct cvmx_fpa_aurax_cnt_s           cn78xxp1;
	struct cvmx_fpa_aurax_cnt_s           cnf75xx;
};
typedef union cvmx_fpa_aurax_cnt cvmx_fpa_aurax_cnt_t;

/**
 * cvmx_fpa_aura#_cnt_add
 */
union cvmx_fpa_aurax_cnt_add {
	uint64_t u64;
	struct cvmx_fpa_aurax_cnt_add_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_40_63               : 24;
	uint64_t cnt                          : 40; /**< The value to be added to FPA_AURA()_CNT. The value may alternatively be a 2's
                                                         complement of a value to be subtracted. Subtraction or addition that results in overflow
                                                         will zero the count, not roll-around, and set either FPA_ERR_INT[CNT_ADD] or
                                                         FPA_ERR_INT[CNT_SUB].
                                                         This register is intended for use when FPA_AURA()_CFG[PTR_DIS] is set.  If
                                                         FPA_AURA()_CFG[PTR_DIS] is clear, this register would typically only be used if buffers
                                                         are being re-provisioned. */
#else
	uint64_t cnt                          : 40;
	uint64_t reserved_40_63               : 24;
#endif
	} s;
	struct cvmx_fpa_aurax_cnt_add_s       cn73xx;
	struct cvmx_fpa_aurax_cnt_add_s       cn78xx;
	struct cvmx_fpa_aurax_cnt_add_s       cn78xxp1;
	struct cvmx_fpa_aurax_cnt_add_s       cnf75xx;
};
typedef union cvmx_fpa_aurax_cnt_add cvmx_fpa_aurax_cnt_add_t;

/**
 * cvmx_fpa_aura#_cnt_levels
 */
union cvmx_fpa_aurax_cnt_levels {
	uint64_t u64;
	struct cvmx_fpa_aurax_cnt_levels_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_41_63               : 23;
	uint64_t drop_dis                     : 1;  /**< Disable aura DROP based on the [DROP] level. */
	uint64_t bp_ena                       : 1;  /**< Enable backpressure based on [BP] level. If set FPA_GEN_CFG[LVL_DLY] must be nonzero.
                                                         PKI_AURA()_CFG[ENA_BP] must also be set for backpressure to propagate through PKI. */
	uint64_t red_ena                      : 1;  /**< Enable aura RED based on [DROP] and [PASS] levels. If set FPA_GEN_CFG[LVL_DLY] must be
                                                         nonzero.
                                                         If set, aura RED is performed on core requests with
                                                         FPA_ALLOC_LD_S/FPA_ALLOC_IOBDMA_S[RED] set, and also may be performed on the
                                                         first PKI allocation request for a packet (depends on PKI style and aura
                                                         configuration). */
	uint64_t shift                        : 6;  /**< Right shift to FPA_AURA()_CNT[CNT] to create a narrower depth for aura QOS and
                                                         backpressure calculations. PKI saturates the shifted FPA_AURA()_CNT[CNT] to
                                                         8-bits, and compares this 8-bit shifted and saturated depth directly to
                                                         [DROP/BP]. PKI also creates [LEVEL], which is a moving average of the 8-bit
                                                         shifted and saturated depth of the aura, for comparison to [DROP/PASS] in RED
                                                         calculations. */
	uint64_t bp                           : 8;  /**< Backpressure can assert if the current 8-bit shifted and saturated FPA_AURA()_CNT[CNT] is
                                                         equal to or greater than this value. */
	uint64_t drop                         : 8;  /**< If [RED_ENA]=1 and RED processing is requested, the packet will be dropped if
                                                         [LEVEL] is equal to or greater than this value.
                                                         If [DROP_DIS]=0 and DROP processing is requested, the packet will be dropped if
                                                         the current 8-bit shifted and saturated FPA_AURA()_CNT[CNT] is equal to or greater
                                                         than this value. */
	uint64_t pass                         : 8;  /**< Aura RED processing will not drop an allocation request if [LEVEL] is less than this value. */
	uint64_t level                        : 8;  /**< Current moving average of the 8-bit shifted and saturated FPA_AURA()_CNT[CNT].
                                                         The lower [LEVEL] is, the more free resources. The highest [LEVEL]'s indicate buffer
                                                         exhaustion.
                                                         See [SHIFT]. */
#else
	uint64_t level                        : 8;
	uint64_t pass                         : 8;
	uint64_t drop                         : 8;
	uint64_t bp                           : 8;
	uint64_t shift                        : 6;
	uint64_t red_ena                      : 1;
	uint64_t bp_ena                       : 1;
	uint64_t drop_dis                     : 1;
	uint64_t reserved_41_63               : 23;
#endif
	} s;
	struct cvmx_fpa_aurax_cnt_levels_s    cn73xx;
	struct cvmx_fpa_aurax_cnt_levels_s    cn78xx;
	struct cvmx_fpa_aurax_cnt_levels_s    cn78xxp1;
	struct cvmx_fpa_aurax_cnt_levels_s    cnf75xx;
};
typedef union cvmx_fpa_aurax_cnt_levels cvmx_fpa_aurax_cnt_levels_t;

/**
 * cvmx_fpa_aura#_cnt_limit
 */
union cvmx_fpa_aurax_cnt_limit {
	uint64_t u64;
	struct cvmx_fpa_aurax_cnt_limit_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_40_63               : 24;
	uint64_t limit                        : 40; /**< When FPA_AURA()_CNT is equal to or greater than this value, any allocations using
                                                         this aura will fail. This allows a hard resource division between multiple auras sharing a
                                                         common pool. */
#else
	uint64_t limit                        : 40;
	uint64_t reserved_40_63               : 24;
#endif
	} s;
	struct cvmx_fpa_aurax_cnt_limit_s     cn73xx;
	struct cvmx_fpa_aurax_cnt_limit_s     cn78xx;
	struct cvmx_fpa_aurax_cnt_limit_s     cn78xxp1;
	struct cvmx_fpa_aurax_cnt_limit_s     cnf75xx;
};
typedef union cvmx_fpa_aurax_cnt_limit cvmx_fpa_aurax_cnt_limit_t;

/**
 * cvmx_fpa_aura#_cnt_threshold
 */
union cvmx_fpa_aurax_cnt_threshold {
	uint64_t u64;
	struct cvmx_fpa_aurax_cnt_threshold_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_40_63               : 24;
	uint64_t thresh                       : 40; /**< When FPA_AURA()_CNT, after being modified, is equal to or crosses this value (i.e.
                                                         value was greater than, then becomes less than, or the value was less than and becomes
                                                         greater than) the corresponding bit in FPA_AURA()_INT is set. */
#else
	uint64_t thresh                       : 40;
	uint64_t reserved_40_63               : 24;
#endif
	} s;
	struct cvmx_fpa_aurax_cnt_threshold_s cn73xx;
	struct cvmx_fpa_aurax_cnt_threshold_s cn78xx;
	struct cvmx_fpa_aurax_cnt_threshold_s cn78xxp1;
	struct cvmx_fpa_aurax_cnt_threshold_s cnf75xx;
};
typedef union cvmx_fpa_aurax_cnt_threshold cvmx_fpa_aurax_cnt_threshold_t;

/**
 * cvmx_fpa_aura#_int
 */
union cvmx_fpa_aurax_int {
	uint64_t u64;
	struct cvmx_fpa_aurax_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t thresh                       : 1;  /**< Watermark interrupt pending. Set and throws FPA_INTSN_E::FPA_AURA()_THRESH when
                                                         FPA_AURA()_CNT, after being modified, is equal to or crosses
                                                         FPA_AURA()_CNT_THRESHOLD (i.e. value was greater than, then becomes less then, or
                                                         value was less than, and becomes greater than). */
#else
	uint64_t thresh                       : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_fpa_aurax_int_s           cn73xx;
	struct cvmx_fpa_aurax_int_s           cn78xx;
	struct cvmx_fpa_aurax_int_s           cn78xxp1;
	struct cvmx_fpa_aurax_int_s           cnf75xx;
};
typedef union cvmx_fpa_aurax_int cvmx_fpa_aurax_int_t;

/**
 * cvmx_fpa_aura#_pool
 *
 * Provides the mapping from each aura to the pool number.
 *
 */
union cvmx_fpa_aurax_pool {
	uint64_t u64;
	struct cvmx_fpa_aurax_pool_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t pool                         : 6;  /**< Indicates which pool corresponds to each aura. Bit 5 should always be 0 (limit 32 pools) */
#else
	uint64_t pool                         : 6;
	uint64_t reserved_6_63                : 58;
#endif
	} s;
	struct cvmx_fpa_aurax_pool_s          cn73xx;
	struct cvmx_fpa_aurax_pool_s          cn78xx;
	struct cvmx_fpa_aurax_pool_s          cn78xxp1;
	struct cvmx_fpa_aurax_pool_s          cnf75xx;
};
typedef union cvmx_fpa_aurax_pool cvmx_fpa_aurax_pool_t;

/**
 * cvmx_fpa_aura#_pool_levels
 */
union cvmx_fpa_aurax_pool_levels {
	uint64_t u64;
	struct cvmx_fpa_aurax_pool_levels_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_41_63               : 23;
	uint64_t drop_dis                     : 1;  /**< Disables aura-unique pool DROP based on the [DROP] level. */
	uint64_t bp_ena                       : 1;  /**< Enable aura-unique pool backpressure based on [BP] level. If set FPA_GEN_CFG[LVL_DLY] must
                                                         be nonzero. */
	uint64_t red_ena                      : 1;  /**< Enable aura-unique pool RED based on [DROP] and [PASS] levels. If set FPA_GEN_CFG[LVL_DLY]
                                                         must be nonzero.
                                                         If set, aura-unique pool RED is performed on core requests with
                                                         FPA_ALLOC_LD_S/FPA_ALLOC_IOBDMA_S[RED] set, and also may be performed on the first PKI
                                                         allocation request for a packet (depending on PKI style and aura configuration). */
	uint64_t shift                        : 6;  /**< Right shift to FPA_POOL()_AVAILABLE[COUNT] used to create a narrower depth for
                                                         aura-unique pool QOS and backpressure calculations. PKI saturates the shifted
                                                         FPA_POOL()_AVAILABLE[COUNT] to 8-bits for the aura, and compares this 8-bit
                                                         shifted and saturated depth directly to [DROP/BP]. PKI also creates [LEVEL],
                                                         which is a moving average of the 8-bit shifted and saturated depth for the aura,
                                                         for comparison to [DROP/PASS] in aura-unique pool RED calculations.
                                                         Though [SHIFT] may differ amongst the auras sharing a given pool, they may most
                                                         commonly be the same (i.e. the 8-bit shifted and saturated depth and [LEVEL] may
                                                         typically be the same for all auras sharing a pool), with the [DROP/PASS/BP]
                                                         configuration providing aura-uniqueness in aura-unique pool RED/DROP/BP
                                                         processing. */
	uint64_t bp                           : 8;  /**< Backpressure can assert if the current 8-bit shifted and saturated
                                                         FPA_POOL()_AVAILABLE[COUNT] for the aura is equal to or less than this value. */
	uint64_t drop                         : 8;  /**< If [RED_ENA]=1 and RED processing is requested, the packet will be dropped if
                                                         [LEVEL] is equal to or less than this value.
                                                         If [DROP_DIS]=0 and DROP processing is requested, the packet will be dropped
                                                         if the current 8-bit shifted and saturated FPA_POOL()_AVAILABLE[COUNT] for the
                                                         aura is equal to or less than this value. */
	uint64_t pass                         : 8;  /**< Aura-unique pool RED processing will not drop an allocation request if [LEVEL] is larger
                                                         than this value. */
	uint64_t level                        : 8;  /**< Current moving average of the 8-bit shifted and saturated FPA_POOL()_AVAILABLE[COUNT] for
                                                         the aura.
                                                         The higher [LEVEL] is, the more free resources. The lowest [LEVEL]'s indicate buffer
                                                         exhaustion.
                                                         See [SHIFT]. */
#else
	uint64_t level                        : 8;
	uint64_t pass                         : 8;
	uint64_t drop                         : 8;
	uint64_t bp                           : 8;
	uint64_t shift                        : 6;
	uint64_t red_ena                      : 1;
	uint64_t bp_ena                       : 1;
	uint64_t drop_dis                     : 1;
	uint64_t reserved_41_63               : 23;
#endif
	} s;
	struct cvmx_fpa_aurax_pool_levels_s   cn73xx;
	struct cvmx_fpa_aurax_pool_levels_s   cn78xx;
	struct cvmx_fpa_aurax_pool_levels_s   cn78xxp1;
	struct cvmx_fpa_aurax_pool_levels_s   cnf75xx;
};
typedef union cvmx_fpa_aurax_pool_levels cvmx_fpa_aurax_pool_levels_t;

/**
 * cvmx_fpa_bist_status
 *
 * This register provides the result of the BIST run on the FPA memories.
 *
 */
union cvmx_fpa_bist_status {
	uint64_t u64;
	struct cvmx_fpa_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_fpa_bist_status_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t frd                          : 1;  /**< fpa_frd  memory bist status. */
	uint64_t fpf0                         : 1;  /**< fpa_fpf0 memory bist status. */
	uint64_t fpf1                         : 1;  /**< fpa_fpf1 memory bist status. */
	uint64_t ffr                          : 1;  /**< fpa_ffr  memory bist status. */
	uint64_t fdr                          : 1;  /**< fpa_fdr  memory bist status. */
#else
	uint64_t fdr                          : 1;
	uint64_t ffr                          : 1;
	uint64_t fpf1                         : 1;
	uint64_t fpf0                         : 1;
	uint64_t frd                          : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} cn30xx;
	struct cvmx_fpa_bist_status_cn30xx    cn31xx;
	struct cvmx_fpa_bist_status_cn30xx    cn38xx;
	struct cvmx_fpa_bist_status_cn30xx    cn38xxp2;
	struct cvmx_fpa_bist_status_cn30xx    cn50xx;
	struct cvmx_fpa_bist_status_cn30xx    cn52xx;
	struct cvmx_fpa_bist_status_cn30xx    cn52xxp1;
	struct cvmx_fpa_bist_status_cn30xx    cn56xx;
	struct cvmx_fpa_bist_status_cn30xx    cn56xxp1;
	struct cvmx_fpa_bist_status_cn30xx    cn58xx;
	struct cvmx_fpa_bist_status_cn30xx    cn58xxp1;
	struct cvmx_fpa_bist_status_cn30xx    cn61xx;
	struct cvmx_fpa_bist_status_cn30xx    cn63xx;
	struct cvmx_fpa_bist_status_cn30xx    cn63xxp1;
	struct cvmx_fpa_bist_status_cn30xx    cn66xx;
	struct cvmx_fpa_bist_status_cn30xx    cn68xx;
	struct cvmx_fpa_bist_status_cn30xx    cn68xxp1;
	struct cvmx_fpa_bist_status_cn30xx    cn70xx;
	struct cvmx_fpa_bist_status_cn30xx    cn70xxp1;
	struct cvmx_fpa_bist_status_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_38_63               : 26;
	uint64_t status                       : 38; /**< Memory BIST status. */
#else
	uint64_t status                       : 38;
	uint64_t reserved_38_63               : 26;
#endif
	} cn73xx;
	struct cvmx_fpa_bist_status_cn73xx    cn78xx;
	struct cvmx_fpa_bist_status_cn73xx    cn78xxp1;
	struct cvmx_fpa_bist_status_cn30xx    cnf71xx;
	struct cvmx_fpa_bist_status_cn73xx    cnf75xx;
};
typedef union cvmx_fpa_bist_status cvmx_fpa_bist_status_t;

/**
 * cvmx_fpa_clk_count
 *
 * This register counts the number of coprocessor-clock cycles since the deassertion of reset.
 *
 */
union cvmx_fpa_clk_count {
	uint64_t u64;
	struct cvmx_fpa_clk_count_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t clk_cnt                      : 64; /**< Clock count. This counter is cleared to 0x0 when reset is applied and increments on every
                                                         rising edge of the coprocessor clock. */
#else
	uint64_t clk_cnt                      : 64;
#endif
	} s;
	struct cvmx_fpa_clk_count_s           cn73xx;
	struct cvmx_fpa_clk_count_s           cn78xx;
	struct cvmx_fpa_clk_count_s           cn78xxp1;
	struct cvmx_fpa_clk_count_s           cnf75xx;
};
typedef union cvmx_fpa_clk_count cvmx_fpa_clk_count_t;

/**
 * cvmx_fpa_ctl_status
 *
 * The FPA's interrupt enable register.
 *
 */
union cvmx_fpa_ctl_status {
	uint64_t u64;
	struct cvmx_fpa_ctl_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_21_63               : 43;
	uint64_t free_en                      : 1;  /**< Enables the setting of the INT_SUM_[FREE*] bits. */
	uint64_t ret_off                      : 1;  /**< When set NCB devices returning pointer will be
                                                         stalled. */
	uint64_t req_off                      : 1;  /**< When set NCB devices requesting pointers will be
                                                         stalled. */
	uint64_t reset                        : 1;  /**< When set causes a reset of the FPA with the */
	uint64_t use_ldt                      : 1;  /**< When clear '0' the FPA will use LDT to load
                                                         pointers from the L2C. This is a PASS-2 field. */
	uint64_t use_stt                      : 1;  /**< When clear '0' the FPA will use STT to store
                                                         pointers to the L2C. This is a PASS-2 field. */
	uint64_t enb                          : 1;  /**< Must be set to 1 AFTER writing all config registers
                                                         and 10 cycles have past. If any of the config
                                                         register are written after writing this bit the
                                                         FPA may begin to operate incorrectly. */
	uint64_t mem1_err                     : 7;  /**< Causes a flip of the ECC bit associated 38:32
                                                         respective to bit 6:0 of this field, for FPF
                                                         FIFO 1. */
	uint64_t mem0_err                     : 7;  /**< Causes a flip of the ECC bit associated 38:32
                                                         respective to bit 6:0 of this field, for FPF
                                                         FIFO 0. */
#else
	uint64_t mem0_err                     : 7;
	uint64_t mem1_err                     : 7;
	uint64_t enb                          : 1;
	uint64_t use_stt                      : 1;
	uint64_t use_ldt                      : 1;
	uint64_t reset                        : 1;
	uint64_t req_off                      : 1;
	uint64_t ret_off                      : 1;
	uint64_t free_en                      : 1;
	uint64_t reserved_21_63               : 43;
#endif
	} s;
	struct cvmx_fpa_ctl_status_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t reset                        : 1;  /**< When set causes a reset of the FPA with the
                                                         exception of the RSL. */
	uint64_t use_ldt                      : 1;  /**< When clear '0' the FPA will use LDT to load
                                                         pointers from the L2C. */
	uint64_t use_stt                      : 1;  /**< When clear '0' the FPA will use STT to store
                                                         pointers to the L2C. */
	uint64_t enb                          : 1;  /**< Must be set to 1 AFTER writing all config registers
                                                         and 10 cycles have past. If any of the config
                                                         register are written after writing this bit the
                                                         FPA may begin to operate incorrectly. */
	uint64_t mem1_err                     : 7;  /**< Causes a flip of the ECC bit associated 38:32
                                                         respective to bit 6:0 of this field, for FPF
                                                         FIFO 1. */
	uint64_t mem0_err                     : 7;  /**< Causes a flip of the ECC bit associated 38:32
                                                         respective to bit 6:0 of this field, for FPF
                                                         FIFO 0. */
#else
	uint64_t mem0_err                     : 7;
	uint64_t mem1_err                     : 7;
	uint64_t enb                          : 1;
	uint64_t use_stt                      : 1;
	uint64_t use_ldt                      : 1;
	uint64_t reset                        : 1;
	uint64_t reserved_18_63               : 46;
#endif
	} cn30xx;
	struct cvmx_fpa_ctl_status_cn30xx     cn31xx;
	struct cvmx_fpa_ctl_status_cn30xx     cn38xx;
	struct cvmx_fpa_ctl_status_cn30xx     cn38xxp2;
	struct cvmx_fpa_ctl_status_cn30xx     cn50xx;
	struct cvmx_fpa_ctl_status_cn30xx     cn52xx;
	struct cvmx_fpa_ctl_status_cn30xx     cn52xxp1;
	struct cvmx_fpa_ctl_status_cn30xx     cn56xx;
	struct cvmx_fpa_ctl_status_cn30xx     cn56xxp1;
	struct cvmx_fpa_ctl_status_cn30xx     cn58xx;
	struct cvmx_fpa_ctl_status_cn30xx     cn58xxp1;
	struct cvmx_fpa_ctl_status_s          cn61xx;
	struct cvmx_fpa_ctl_status_s          cn63xx;
	struct cvmx_fpa_ctl_status_cn30xx     cn63xxp1;
	struct cvmx_fpa_ctl_status_s          cn66xx;
	struct cvmx_fpa_ctl_status_s          cn68xx;
	struct cvmx_fpa_ctl_status_s          cn68xxp1;
	struct cvmx_fpa_ctl_status_s          cn70xx;
	struct cvmx_fpa_ctl_status_s          cn70xxp1;
	struct cvmx_fpa_ctl_status_s          cnf71xx;
};
typedef union cvmx_fpa_ctl_status cvmx_fpa_ctl_status_t;

/**
 * cvmx_fpa_ecc_ctl
 *
 * This register allows inserting ECC errors for testing.
 *
 */
union cvmx_fpa_ecc_ctl {
	uint64_t u64;
	struct cvmx_fpa_ecc_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_62_63               : 2;
	uint64_t ram_flip1                    : 20; /**< Flip syndrome bits on write. Flip syndrome bits <1> on writes to the corresponding ram to
                                                         test single-bit or double-bit error handling. */
	uint64_t reserved_41_41               : 1;
	uint64_t ram_flip0                    : 20; /**< Flip syndrome bits on write. Flip syndrome bits <0> on writes to the corresponding ram to
                                                         test single-bit or double-bit error handling. */
	uint64_t reserved_20_20               : 1;
	uint64_t ram_cdis                     : 20; /**< RAM ECC correction disable. Each bit corresponds to a different RAM. */
#else
	uint64_t ram_cdis                     : 20;
	uint64_t reserved_20_20               : 1;
	uint64_t ram_flip0                    : 20;
	uint64_t reserved_41_41               : 1;
	uint64_t ram_flip1                    : 20;
	uint64_t reserved_62_63               : 2;
#endif
	} s;
	struct cvmx_fpa_ecc_ctl_s             cn73xx;
	struct cvmx_fpa_ecc_ctl_s             cn78xx;
	struct cvmx_fpa_ecc_ctl_s             cn78xxp1;
	struct cvmx_fpa_ecc_ctl_s             cnf75xx;
};
typedef union cvmx_fpa_ecc_ctl cvmx_fpa_ecc_ctl_t;

/**
 * cvmx_fpa_ecc_int
 *
 * This register contains ECC error interrupt summary bits.
 *
 */
union cvmx_fpa_ecc_int {
	uint64_t u64;
	struct cvmx_fpa_ecc_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_52_63               : 12;
	uint64_t ram_dbe                      : 20; /**< Set when a double-bit error is detected in the corresponding RAM. Throws
                                                         FPA_INTSN_E::FPA_ERR_RAM_DBE. */
	uint64_t reserved_20_31               : 12;
	uint64_t ram_sbe                      : 20; /**< Set when a single-bit error is detected in the corresponding RAM. Throws
                                                         FPA_INTSN_E::FPA_ERR_RAM_SBE. */
#else
	uint64_t ram_sbe                      : 20;
	uint64_t reserved_20_31               : 12;
	uint64_t ram_dbe                      : 20;
	uint64_t reserved_52_63               : 12;
#endif
	} s;
	struct cvmx_fpa_ecc_int_s             cn73xx;
	struct cvmx_fpa_ecc_int_s             cn78xx;
	struct cvmx_fpa_ecc_int_s             cn78xxp1;
	struct cvmx_fpa_ecc_int_s             cnf75xx;
};
typedef union cvmx_fpa_ecc_int cvmx_fpa_ecc_int_t;

/**
 * cvmx_fpa_err_int
 *
 * This register contains the global (non-pool) error interrupt summary bits of the FPA.
 *
 */
union cvmx_fpa_err_int {
	uint64_t u64;
	struct cvmx_fpa_err_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t hw_sub                       : 1;  /**< Set when hardware does a subtract to the count that caused the counter to wrap. Throws
                                                         FPA_INTSN_E::FPA_ERR_HW_SUB. */
	uint64_t hw_add                       : 1;  /**< Set when hardware does an add to the count that caused the counter to wrap. Throws
                                                         FPA_INTSN_E::FPA_ERR_HW_ADD. */
	uint64_t cnt_sub                      : 1;  /**< Set when a write to FPA_AURA()_CNT_ADD does a subtract to the count that would have
                                                         caused the counter to wrap, so the count was zeroed. Throws FPA_INTSN_E::FPA_ERR_CNT_SUB. */
	uint64_t cnt_add                      : 1;  /**< Set when a write to FPA_AURA()_CNT_ADD does an add to the count that would have
                                                         caused the counter to wrap, so the count was zeroed. Throws FPA_INTSN_E::FPA_ERR_CNT_ADD. */
#else
	uint64_t cnt_add                      : 1;
	uint64_t cnt_sub                      : 1;
	uint64_t hw_add                       : 1;
	uint64_t hw_sub                       : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_fpa_err_int_s             cn73xx;
	struct cvmx_fpa_err_int_s             cn78xx;
	struct cvmx_fpa_err_int_s             cn78xxp1;
	struct cvmx_fpa_err_int_s             cnf75xx;
};
typedef union cvmx_fpa_err_int cvmx_fpa_err_int_t;

/**
 * cvmx_fpa_fpf#_marks
 *
 * "The high and low watermark register that determines when we write and read free pages from
 * L2C
 * for Queue 1. The value of FPF_RD and FPF_WR should have at least a 33 difference. Recommend
 * value
 * is FPF_RD == (FPA_FPF#_SIZE[FPF_SIZ] * .25) and FPF_WR == (FPA_FPF#_SIZE[FPF_SIZ] * .75)"
 */
union cvmx_fpa_fpfx_marks {
	uint64_t u64;
	struct cvmx_fpa_fpfx_marks_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_22_63               : 42;
	uint64_t fpf_wr                       : 11; /**< When the number of free-page-pointers in a
                                                          queue exceeds this value the FPA will write
                                                          32-page-pointers of that queue to DRAM.
                                                         The MAX value for this field should be
                                                         FPA_FPF1_SIZE[FPF_SIZ]-2. */
	uint64_t fpf_rd                       : 11; /**< When the number of free-page-pointers in a
                                                          queue drops below this value and there are
                                                          free-page-pointers in DRAM, the FPA will
                                                          read one page (32 pointers) from DRAM.
                                                         This maximum value for this field should be
                                                         FPA_FPF1_SIZE[FPF_SIZ]-34. The min number
                                                         for this would be 16. */
#else
	uint64_t fpf_rd                       : 11;
	uint64_t fpf_wr                       : 11;
	uint64_t reserved_22_63               : 42;
#endif
	} s;
	struct cvmx_fpa_fpfx_marks_s          cn38xx;
	struct cvmx_fpa_fpfx_marks_s          cn38xxp2;
	struct cvmx_fpa_fpfx_marks_s          cn56xx;
	struct cvmx_fpa_fpfx_marks_s          cn56xxp1;
	struct cvmx_fpa_fpfx_marks_s          cn58xx;
	struct cvmx_fpa_fpfx_marks_s          cn58xxp1;
	struct cvmx_fpa_fpfx_marks_s          cn61xx;
	struct cvmx_fpa_fpfx_marks_s          cn63xx;
	struct cvmx_fpa_fpfx_marks_s          cn63xxp1;
	struct cvmx_fpa_fpfx_marks_s          cn66xx;
	struct cvmx_fpa_fpfx_marks_s          cn68xx;
	struct cvmx_fpa_fpfx_marks_s          cn68xxp1;
	struct cvmx_fpa_fpfx_marks_s          cn70xx;
	struct cvmx_fpa_fpfx_marks_s          cn70xxp1;
	struct cvmx_fpa_fpfx_marks_s          cnf71xx;
};
typedef union cvmx_fpa_fpfx_marks cvmx_fpa_fpfx_marks_t;

/**
 * cvmx_fpa_fpf#_size
 *
 * "FPA_FPFX_SIZE = FPA's Queue 1-7 Free Page FIFO Size
 * The number of page pointers that will be kept local to the FPA for this Queue. FPA Queues are
 * assigned in order from Queue 0 to Queue 7, though only Queue 0 through Queue x can be used.
 * The sum of the 8 (0-7) FPA_FPF#_SIZE registers must be limited to 2048."
 */
union cvmx_fpa_fpfx_size {
	uint64_t u64;
	struct cvmx_fpa_fpfx_size_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_11_63               : 53;
	uint64_t fpf_siz                      : 11; /**< The number of entries assigned in the FPA FIFO
                                                         (used to hold page-pointers) for this Queue.
                                                         The value of this register must divisable by 2,
                                                         and the FPA will ignore bit [0] of this register.
                                                         The total of the FPF_SIZ field of the 8 (0-7)
                                                         FPA_FPF#_SIZE registers must not exceed 2048.
                                                         After writing this field the FPA will need 10
                                                         core clock cycles to be ready for operation. The
                                                         assignment of location in the FPA FIFO must
                                                         start with Queue 0, then 1, 2, etc.
                                                         The number of useable entries will be FPF_SIZ-2. */
#else
	uint64_t fpf_siz                      : 11;
	uint64_t reserved_11_63               : 53;
#endif
	} s;
	struct cvmx_fpa_fpfx_size_s           cn38xx;
	struct cvmx_fpa_fpfx_size_s           cn38xxp2;
	struct cvmx_fpa_fpfx_size_s           cn56xx;
	struct cvmx_fpa_fpfx_size_s           cn56xxp1;
	struct cvmx_fpa_fpfx_size_s           cn58xx;
	struct cvmx_fpa_fpfx_size_s           cn58xxp1;
	struct cvmx_fpa_fpfx_size_s           cn61xx;
	struct cvmx_fpa_fpfx_size_s           cn63xx;
	struct cvmx_fpa_fpfx_size_s           cn63xxp1;
	struct cvmx_fpa_fpfx_size_s           cn66xx;
	struct cvmx_fpa_fpfx_size_s           cn68xx;
	struct cvmx_fpa_fpfx_size_s           cn68xxp1;
	struct cvmx_fpa_fpfx_size_s           cn70xx;
	struct cvmx_fpa_fpfx_size_s           cn70xxp1;
	struct cvmx_fpa_fpfx_size_s           cnf71xx;
};
typedef union cvmx_fpa_fpfx_size cvmx_fpa_fpfx_size_t;

/**
 * cvmx_fpa_fpf0_marks
 *
 * "The high and low watermark register that determines when we write and read free pages from
 * L2C
 * for Queue 0. The value of FPF_RD and FPF_WR should have at least a 33 difference. Recommend
 * value
 * is FPF_RD == (FPA_FPF#_SIZE[FPF_SIZ] * .25) and FPF_WR == (FPA_FPF#_SIZE[FPF_SIZ] * .75)"
 */
union cvmx_fpa_fpf0_marks {
	uint64_t u64;
	struct cvmx_fpa_fpf0_marks_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t fpf_wr                       : 12; /**< When the number of free-page-pointers in a
                                                          queue exceeds this value the FPA will write
                                                          32-page-pointers of that queue to DRAM.
                                                         The MAX value for this field should be
                                                         FPA_FPF0_SIZE[FPF_SIZ]-2. */
	uint64_t fpf_rd                       : 12; /**< When the number of free-page-pointers in a
                                                         queue drops below this value and there are
                                                         free-page-pointers in DRAM, the FPA will
                                                         read one page (32 pointers) from DRAM.
                                                         This maximum value for this field should be
                                                         FPA_FPF0_SIZE[FPF_SIZ]-34. The min number
                                                         for this would be 16. */
#else
	uint64_t fpf_rd                       : 12;
	uint64_t fpf_wr                       : 12;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_fpa_fpf0_marks_s          cn38xx;
	struct cvmx_fpa_fpf0_marks_s          cn38xxp2;
	struct cvmx_fpa_fpf0_marks_s          cn56xx;
	struct cvmx_fpa_fpf0_marks_s          cn56xxp1;
	struct cvmx_fpa_fpf0_marks_s          cn58xx;
	struct cvmx_fpa_fpf0_marks_s          cn58xxp1;
	struct cvmx_fpa_fpf0_marks_s          cn61xx;
	struct cvmx_fpa_fpf0_marks_s          cn63xx;
	struct cvmx_fpa_fpf0_marks_s          cn63xxp1;
	struct cvmx_fpa_fpf0_marks_s          cn66xx;
	struct cvmx_fpa_fpf0_marks_s          cn68xx;
	struct cvmx_fpa_fpf0_marks_s          cn68xxp1;
	struct cvmx_fpa_fpf0_marks_s          cn70xx;
	struct cvmx_fpa_fpf0_marks_s          cn70xxp1;
	struct cvmx_fpa_fpf0_marks_s          cnf71xx;
};
typedef union cvmx_fpa_fpf0_marks cvmx_fpa_fpf0_marks_t;

/**
 * cvmx_fpa_fpf0_size
 *
 * "The number of page pointers that will be kept local to the FPA for this Queue. FPA Queues are
 * assigned in order from Queue 0 to Queue 7, though only Queue 0 through Queue x can be used.
 * The sum of the 8 (0-7) FPA_FPF#_SIZE registers must be limited to 2048."
 */
union cvmx_fpa_fpf0_size {
	uint64_t u64;
	struct cvmx_fpa_fpf0_size_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t fpf_siz                      : 12; /**< The number of entries assigned in the FPA FIFO
                                                         (used to hold page-pointers) for this Queue.
                                                         The value of this register must divisable by 2,
                                                         and the FPA will ignore bit [0] of this register.
                                                         The total of the FPF_SIZ field of the 8 (0-7)
                                                         FPA_FPF#_SIZE registers must not exceed 2048.
                                                         After writing this field the FPA will need 10
                                                         core clock cycles to be ready for operation. The
                                                         assignment of location in the FPA FIFO must
                                                         start with Queue 0, then 1, 2, etc.
                                                         The number of useable entries will be FPF_SIZ-2. */
#else
	uint64_t fpf_siz                      : 12;
	uint64_t reserved_12_63               : 52;
#endif
	} s;
	struct cvmx_fpa_fpf0_size_s           cn38xx;
	struct cvmx_fpa_fpf0_size_s           cn38xxp2;
	struct cvmx_fpa_fpf0_size_s           cn56xx;
	struct cvmx_fpa_fpf0_size_s           cn56xxp1;
	struct cvmx_fpa_fpf0_size_s           cn58xx;
	struct cvmx_fpa_fpf0_size_s           cn58xxp1;
	struct cvmx_fpa_fpf0_size_s           cn61xx;
	struct cvmx_fpa_fpf0_size_s           cn63xx;
	struct cvmx_fpa_fpf0_size_s           cn63xxp1;
	struct cvmx_fpa_fpf0_size_s           cn66xx;
	struct cvmx_fpa_fpf0_size_s           cn68xx;
	struct cvmx_fpa_fpf0_size_s           cn68xxp1;
	struct cvmx_fpa_fpf0_size_s           cn70xx;
	struct cvmx_fpa_fpf0_size_s           cn70xxp1;
	struct cvmx_fpa_fpf0_size_s           cnf71xx;
};
typedef union cvmx_fpa_fpf0_size cvmx_fpa_fpf0_size_t;

/**
 * cvmx_fpa_fpf8_marks
 *
 * Reserved through 0x238 for additional thresholds
 *
 *                  FPA_FPF8_MARKS = FPA's Queue 8 Free Page FIFO Read Write Marks
 *
 * The high and low watermark register that determines when we write and read free pages from L2C
 * for Queue 8. The value of FPF_RD and FPF_WR should have at least a 33 difference. Recommend value
 * is FPF_RD == (FPA_FPF#_SIZE[FPF_SIZ] * .25) and FPF_WR == (FPA_FPF#_SIZE[FPF_SIZ] * .75)
 */
union cvmx_fpa_fpf8_marks {
	uint64_t u64;
	struct cvmx_fpa_fpf8_marks_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_22_63               : 42;
	uint64_t fpf_wr                       : 11; /**< When the number of free-page-pointers in a
                                                         queue exceeds this value the FPA will write
                                                         32-page-pointers of that queue to DRAM.
                                                         The MAX value for this field should be
                                                         FPA_FPF0_SIZE[FPF_SIZ]-2. */
	uint64_t fpf_rd                       : 11; /**< When the number of free-page-pointers in a
                                                         queue drops below this value and there are
                                                         free-page-pointers in DRAM, the FPA will
                                                         read one page (32 pointers) from DRAM.
                                                         This maximum value for this field should be
                                                         FPA_FPF0_SIZE[FPF_SIZ]-34. The min number
                                                         for this would be 16. */
#else
	uint64_t fpf_rd                       : 11;
	uint64_t fpf_wr                       : 11;
	uint64_t reserved_22_63               : 42;
#endif
	} s;
	struct cvmx_fpa_fpf8_marks_s          cn68xx;
	struct cvmx_fpa_fpf8_marks_s          cn68xxp1;
};
typedef union cvmx_fpa_fpf8_marks cvmx_fpa_fpf8_marks_t;

/**
 * cvmx_fpa_fpf8_size
 *
 * FPA_FPF8_SIZE = FPA's Queue 8 Free Page FIFO Size
 *
 * The number of page pointers that will be kept local to the FPA for this Queue. FPA Queues are
 * assigned in order from Queue 0 to Queue 7, though only Queue 0 through Queue x can be used.
 * The sum of the 9 (0-8) FPA_FPF#_SIZE registers must be limited to 2048.
 */
union cvmx_fpa_fpf8_size {
	uint64_t u64;
	struct cvmx_fpa_fpf8_size_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t fpf_siz                      : 12; /**< The number of entries assigned in the FPA FIFO
                                                         (used to hold page-pointers) for this Queue.
                                                         The value of this register must divisable by 2,
                                                         and the FPA will ignore bit [0] of this register.
                                                         The total of the FPF_SIZ field of the 8 (0-7)
                                                         FPA_FPF#_SIZE registers must not exceed 2048.
                                                         After writing this field the FPA will need 10
                                                         core clock cycles to be ready for operation. The
                                                         assignment of location in the FPA FIFO must
                                                         start with Queue 0, then 1, 2, etc.
                                                         The number of useable entries will be FPF_SIZ-2. */
#else
	uint64_t fpf_siz                      : 12;
	uint64_t reserved_12_63               : 52;
#endif
	} s;
	struct cvmx_fpa_fpf8_size_s           cn68xx;
	struct cvmx_fpa_fpf8_size_s           cn68xxp1;
};
typedef union cvmx_fpa_fpf8_size cvmx_fpa_fpf8_size_t;

/**
 * cvmx_fpa_gen_cfg
 *
 * This register provides FPA control and status information.
 *
 */
union cvmx_fpa_gen_cfg {
	uint64_t u64;
	struct cvmx_fpa_gen_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t halfrate                     : 1;  /**< Half rate. Limit peak alloc/free rate to half of peak to insure all alloc/frees are
                                                         visible to OCLA. */
	uint64_t ocla_bp                      : 1;  /**< OCLA backpressure enable. When OCLA FIFOs are near full, allow OCLA to backpressure
                                                         alloc/frees. See also [HALFRATE]. */
	uint64_t lvl_dly                      : 6;  /**< Levelizer delay. Number of cycles between level computations for backpressure and RED.
                                                         Increasing values decrease power and leave additional bandwidth for allocate/deallocates.
                                                         Zero disables, one indicates a level computation every other cycle, etc. Once set to
                                                         nonzero must not be later set to zero without resetting FPA. */
	uint64_t pools                        : 2;  /**< Number of pools. Each halving of the number of pools doubles the buffering available to
                                                         the remaining pools, leading to some improvement in memory bandwidth. Value must not be
                                                         changed if FPA_POOL()_CFG[ENA] is set for any pool.
                                                         0x0 = Reserved.
                                                         0x1 = 32 pools, 216 FPF entries per pool.
                                                         0x2 = 16 pools, 432 FPF entries per pool.
                                                         0x3 = Reserved. */
	uint64_t avg_en                       : 1;  /**< QoS averaging enable. When set, RED calculations use average buffer levels. When clear,
                                                         RED calculations use the current values. */
	uint64_t clk_override                 : 1;  /**< Conditional clock override. */
#else
	uint64_t clk_override                 : 1;
	uint64_t avg_en                       : 1;
	uint64_t pools                        : 2;
	uint64_t lvl_dly                      : 6;
	uint64_t ocla_bp                      : 1;
	uint64_t halfrate                     : 1;
	uint64_t reserved_12_63               : 52;
#endif
	} s;
	struct cvmx_fpa_gen_cfg_s             cn73xx;
	struct cvmx_fpa_gen_cfg_s             cn78xx;
	struct cvmx_fpa_gen_cfg_s             cn78xxp1;
	struct cvmx_fpa_gen_cfg_s             cnf75xx;
};
typedef union cvmx_fpa_gen_cfg cvmx_fpa_gen_cfg_t;

/**
 * cvmx_fpa_int_enb
 *
 * The FPA's interrupt enable register.
 *
 */
union cvmx_fpa_int_enb {
	uint64_t u64;
	struct cvmx_fpa_int_enb_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_50_63               : 14;
	uint64_t paddr_e                      : 1;  /**< When set (1) and bit 49 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t reserved_44_48               : 5;
	uint64_t free7                        : 1;  /**< When set (1) and bit 43 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t free6                        : 1;  /**< When set (1) and bit 42 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t free5                        : 1;  /**< When set (1) and bit 41 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t free4                        : 1;  /**< When set (1) and bit 40 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t free3                        : 1;  /**< When set (1) and bit 39 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t free2                        : 1;  /**< When set (1) and bit 38 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t free1                        : 1;  /**< When set (1) and bit 37 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t free0                        : 1;  /**< When set (1) and bit 36 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t pool7th                      : 1;  /**< When set (1) and bit 35 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t pool6th                      : 1;  /**< When set (1) and bit 34 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t pool5th                      : 1;  /**< When set (1) and bit 33 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t pool4th                      : 1;  /**< When set (1) and bit 32 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t pool3th                      : 1;  /**< When set (1) and bit 31 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t pool2th                      : 1;  /**< When set (1) and bit 30 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t pool1th                      : 1;  /**< When set (1) and bit 29 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t pool0th                      : 1;  /**< When set (1) and bit 28 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q7_perr                      : 1;  /**< When set (1) and bit 27 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q7_coff                      : 1;  /**< When set (1) and bit 26 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q7_und                       : 1;  /**< When set (1) and bit 25 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q6_perr                      : 1;  /**< When set (1) and bit 24 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q6_coff                      : 1;  /**< When set (1) and bit 23 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q6_und                       : 1;  /**< When set (1) and bit 22 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q5_perr                      : 1;  /**< When set (1) and bit 21 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q5_coff                      : 1;  /**< When set (1) and bit 20 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q5_und                       : 1;  /**< When set (1) and bit 19 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q4_perr                      : 1;  /**< When set (1) and bit 18 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q4_coff                      : 1;  /**< When set (1) and bit 17 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q4_und                       : 1;  /**< When set (1) and bit 16 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q3_perr                      : 1;  /**< When set (1) and bit 15 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q3_coff                      : 1;  /**< When set (1) and bit 14 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q3_und                       : 1;  /**< When set (1) and bit 13 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q2_perr                      : 1;  /**< When set (1) and bit 12 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q2_coff                      : 1;  /**< When set (1) and bit 11 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q2_und                       : 1;  /**< When set (1) and bit 10 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q1_perr                      : 1;  /**< When set (1) and bit 9 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q1_coff                      : 1;  /**< When set (1) and bit 8 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q1_und                       : 1;  /**< When set (1) and bit 7 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q0_perr                      : 1;  /**< When set (1) and bit 6 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q0_coff                      : 1;  /**< When set (1) and bit 5 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q0_und                       : 1;  /**< When set (1) and bit 4 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t fed1_dbe                     : 1;  /**< When set (1) and bit 3 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t fed1_sbe                     : 1;  /**< When set (1) and bit 2 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t fed0_dbe                     : 1;  /**< When set (1) and bit 1 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t fed0_sbe                     : 1;  /**< When set (1) and bit 0 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
#else
	uint64_t fed0_sbe                     : 1;
	uint64_t fed0_dbe                     : 1;
	uint64_t fed1_sbe                     : 1;
	uint64_t fed1_dbe                     : 1;
	uint64_t q0_und                       : 1;
	uint64_t q0_coff                      : 1;
	uint64_t q0_perr                      : 1;
	uint64_t q1_und                       : 1;
	uint64_t q1_coff                      : 1;
	uint64_t q1_perr                      : 1;
	uint64_t q2_und                       : 1;
	uint64_t q2_coff                      : 1;
	uint64_t q2_perr                      : 1;
	uint64_t q3_und                       : 1;
	uint64_t q3_coff                      : 1;
	uint64_t q3_perr                      : 1;
	uint64_t q4_und                       : 1;
	uint64_t q4_coff                      : 1;
	uint64_t q4_perr                      : 1;
	uint64_t q5_und                       : 1;
	uint64_t q5_coff                      : 1;
	uint64_t q5_perr                      : 1;
	uint64_t q6_und                       : 1;
	uint64_t q6_coff                      : 1;
	uint64_t q6_perr                      : 1;
	uint64_t q7_und                       : 1;
	uint64_t q7_coff                      : 1;
	uint64_t q7_perr                      : 1;
	uint64_t pool0th                      : 1;
	uint64_t pool1th                      : 1;
	uint64_t pool2th                      : 1;
	uint64_t pool3th                      : 1;
	uint64_t pool4th                      : 1;
	uint64_t pool5th                      : 1;
	uint64_t pool6th                      : 1;
	uint64_t pool7th                      : 1;
	uint64_t free0                        : 1;
	uint64_t free1                        : 1;
	uint64_t free2                        : 1;
	uint64_t free3                        : 1;
	uint64_t free4                        : 1;
	uint64_t free5                        : 1;
	uint64_t free6                        : 1;
	uint64_t free7                        : 1;
	uint64_t reserved_44_48               : 5;
	uint64_t paddr_e                      : 1;
	uint64_t reserved_50_63               : 14;
#endif
	} s;
	struct cvmx_fpa_int_enb_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_28_63               : 36;
	uint64_t q7_perr                      : 1;  /**< When set (1) and bit 27 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q7_coff                      : 1;  /**< When set (1) and bit 26 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q7_und                       : 1;  /**< When set (1) and bit 25 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q6_perr                      : 1;  /**< When set (1) and bit 24 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q6_coff                      : 1;  /**< When set (1) and bit 23 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q6_und                       : 1;  /**< When set (1) and bit 22 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q5_perr                      : 1;  /**< When set (1) and bit 21 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q5_coff                      : 1;  /**< When set (1) and bit 20 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q5_und                       : 1;  /**< When set (1) and bit 19 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q4_perr                      : 1;  /**< When set (1) and bit 18 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q4_coff                      : 1;  /**< When set (1) and bit 17 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q4_und                       : 1;  /**< When set (1) and bit 16 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q3_perr                      : 1;  /**< When set (1) and bit 15 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q3_coff                      : 1;  /**< When set (1) and bit 14 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q3_und                       : 1;  /**< When set (1) and bit 13 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q2_perr                      : 1;  /**< When set (1) and bit 12 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q2_coff                      : 1;  /**< When set (1) and bit 11 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q2_und                       : 1;  /**< When set (1) and bit 10 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q1_perr                      : 1;  /**< When set (1) and bit 9 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q1_coff                      : 1;  /**< When set (1) and bit 8 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q1_und                       : 1;  /**< When set (1) and bit 7 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q0_perr                      : 1;  /**< When set (1) and bit 6 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q0_coff                      : 1;  /**< When set (1) and bit 5 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q0_und                       : 1;  /**< When set (1) and bit 4 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t fed1_dbe                     : 1;  /**< When set (1) and bit 3 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t fed1_sbe                     : 1;  /**< When set (1) and bit 2 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t fed0_dbe                     : 1;  /**< When set (1) and bit 1 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t fed0_sbe                     : 1;  /**< When set (1) and bit 0 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
#else
	uint64_t fed0_sbe                     : 1;
	uint64_t fed0_dbe                     : 1;
	uint64_t fed1_sbe                     : 1;
	uint64_t fed1_dbe                     : 1;
	uint64_t q0_und                       : 1;
	uint64_t q0_coff                      : 1;
	uint64_t q0_perr                      : 1;
	uint64_t q1_und                       : 1;
	uint64_t q1_coff                      : 1;
	uint64_t q1_perr                      : 1;
	uint64_t q2_und                       : 1;
	uint64_t q2_coff                      : 1;
	uint64_t q2_perr                      : 1;
	uint64_t q3_und                       : 1;
	uint64_t q3_coff                      : 1;
	uint64_t q3_perr                      : 1;
	uint64_t q4_und                       : 1;
	uint64_t q4_coff                      : 1;
	uint64_t q4_perr                      : 1;
	uint64_t q5_und                       : 1;
	uint64_t q5_coff                      : 1;
	uint64_t q5_perr                      : 1;
	uint64_t q6_und                       : 1;
	uint64_t q6_coff                      : 1;
	uint64_t q6_perr                      : 1;
	uint64_t q7_und                       : 1;
	uint64_t q7_coff                      : 1;
	uint64_t q7_perr                      : 1;
	uint64_t reserved_28_63               : 36;
#endif
	} cn30xx;
	struct cvmx_fpa_int_enb_cn30xx        cn31xx;
	struct cvmx_fpa_int_enb_cn30xx        cn38xx;
	struct cvmx_fpa_int_enb_cn30xx        cn38xxp2;
	struct cvmx_fpa_int_enb_cn30xx        cn50xx;
	struct cvmx_fpa_int_enb_cn30xx        cn52xx;
	struct cvmx_fpa_int_enb_cn30xx        cn52xxp1;
	struct cvmx_fpa_int_enb_cn30xx        cn56xx;
	struct cvmx_fpa_int_enb_cn30xx        cn56xxp1;
	struct cvmx_fpa_int_enb_cn30xx        cn58xx;
	struct cvmx_fpa_int_enb_cn30xx        cn58xxp1;
	struct cvmx_fpa_int_enb_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_50_63               : 14;
	uint64_t paddr_e                      : 1;  /**< When set (1) and bit 49 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t res_44                       : 5;  /**< Reserved */
	uint64_t free7                        : 1;  /**< When set (1) and bit 43 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t free6                        : 1;  /**< When set (1) and bit 42 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t free5                        : 1;  /**< When set (1) and bit 41 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t free4                        : 1;  /**< When set (1) and bit 40 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t free3                        : 1;  /**< When set (1) and bit 39 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t free2                        : 1;  /**< When set (1) and bit 38 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t free1                        : 1;  /**< When set (1) and bit 37 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t free0                        : 1;  /**< When set (1) and bit 36 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t pool7th                      : 1;  /**< When set (1) and bit 35 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t pool6th                      : 1;  /**< When set (1) and bit 34 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t pool5th                      : 1;  /**< When set (1) and bit 33 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t pool4th                      : 1;  /**< When set (1) and bit 32 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t pool3th                      : 1;  /**< When set (1) and bit 31 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t pool2th                      : 1;  /**< When set (1) and bit 30 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t pool1th                      : 1;  /**< When set (1) and bit 29 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t pool0th                      : 1;  /**< When set (1) and bit 28 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q7_perr                      : 1;  /**< When set (1) and bit 27 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q7_coff                      : 1;  /**< When set (1) and bit 26 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q7_und                       : 1;  /**< When set (1) and bit 25 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q6_perr                      : 1;  /**< When set (1) and bit 24 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q6_coff                      : 1;  /**< When set (1) and bit 23 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q6_und                       : 1;  /**< When set (1) and bit 22 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q5_perr                      : 1;  /**< When set (1) and bit 21 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q5_coff                      : 1;  /**< When set (1) and bit 20 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q5_und                       : 1;  /**< When set (1) and bit 19 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q4_perr                      : 1;  /**< When set (1) and bit 18 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q4_coff                      : 1;  /**< When set (1) and bit 17 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q4_und                       : 1;  /**< When set (1) and bit 16 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q3_perr                      : 1;  /**< When set (1) and bit 15 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q3_coff                      : 1;  /**< When set (1) and bit 14 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q3_und                       : 1;  /**< When set (1) and bit 13 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q2_perr                      : 1;  /**< When set (1) and bit 12 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q2_coff                      : 1;  /**< When set (1) and bit 11 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q2_und                       : 1;  /**< When set (1) and bit 10 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q1_perr                      : 1;  /**< When set (1) and bit 9 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q1_coff                      : 1;  /**< When set (1) and bit 8 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q1_und                       : 1;  /**< When set (1) and bit 7 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q0_perr                      : 1;  /**< When set (1) and bit 6 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q0_coff                      : 1;  /**< When set (1) and bit 5 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q0_und                       : 1;  /**< When set (1) and bit 4 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t fed1_dbe                     : 1;  /**< When set (1) and bit 3 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t fed1_sbe                     : 1;  /**< When set (1) and bit 2 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t fed0_dbe                     : 1;  /**< When set (1) and bit 1 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t fed0_sbe                     : 1;  /**< When set (1) and bit 0 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
#else
	uint64_t fed0_sbe                     : 1;
	uint64_t fed0_dbe                     : 1;
	uint64_t fed1_sbe                     : 1;
	uint64_t fed1_dbe                     : 1;
	uint64_t q0_und                       : 1;
	uint64_t q0_coff                      : 1;
	uint64_t q0_perr                      : 1;
	uint64_t q1_und                       : 1;
	uint64_t q1_coff                      : 1;
	uint64_t q1_perr                      : 1;
	uint64_t q2_und                       : 1;
	uint64_t q2_coff                      : 1;
	uint64_t q2_perr                      : 1;
	uint64_t q3_und                       : 1;
	uint64_t q3_coff                      : 1;
	uint64_t q3_perr                      : 1;
	uint64_t q4_und                       : 1;
	uint64_t q4_coff                      : 1;
	uint64_t q4_perr                      : 1;
	uint64_t q5_und                       : 1;
	uint64_t q5_coff                      : 1;
	uint64_t q5_perr                      : 1;
	uint64_t q6_und                       : 1;
	uint64_t q6_coff                      : 1;
	uint64_t q6_perr                      : 1;
	uint64_t q7_und                       : 1;
	uint64_t q7_coff                      : 1;
	uint64_t q7_perr                      : 1;
	uint64_t pool0th                      : 1;
	uint64_t pool1th                      : 1;
	uint64_t pool2th                      : 1;
	uint64_t pool3th                      : 1;
	uint64_t pool4th                      : 1;
	uint64_t pool5th                      : 1;
	uint64_t pool6th                      : 1;
	uint64_t pool7th                      : 1;
	uint64_t free0                        : 1;
	uint64_t free1                        : 1;
	uint64_t free2                        : 1;
	uint64_t free3                        : 1;
	uint64_t free4                        : 1;
	uint64_t free5                        : 1;
	uint64_t free6                        : 1;
	uint64_t free7                        : 1;
	uint64_t res_44                       : 5;
	uint64_t paddr_e                      : 1;
	uint64_t reserved_50_63               : 14;
#endif
	} cn61xx;
	struct cvmx_fpa_int_enb_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_44_63               : 20;
	uint64_t free7                        : 1;  /**< When set (1) and bit 43 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t free6                        : 1;  /**< When set (1) and bit 42 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t free5                        : 1;  /**< When set (1) and bit 41 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t free4                        : 1;  /**< When set (1) and bit 40 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t free3                        : 1;  /**< When set (1) and bit 39 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t free2                        : 1;  /**< When set (1) and bit 38 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t free1                        : 1;  /**< When set (1) and bit 37 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t free0                        : 1;  /**< When set (1) and bit 36 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t pool7th                      : 1;  /**< When set (1) and bit 35 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t pool6th                      : 1;  /**< When set (1) and bit 34 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t pool5th                      : 1;  /**< When set (1) and bit 33 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t pool4th                      : 1;  /**< When set (1) and bit 32 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t pool3th                      : 1;  /**< When set (1) and bit 31 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t pool2th                      : 1;  /**< When set (1) and bit 30 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t pool1th                      : 1;  /**< When set (1) and bit 29 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t pool0th                      : 1;  /**< When set (1) and bit 28 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q7_perr                      : 1;  /**< When set (1) and bit 27 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q7_coff                      : 1;  /**< When set (1) and bit 26 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q7_und                       : 1;  /**< When set (1) and bit 25 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q6_perr                      : 1;  /**< When set (1) and bit 24 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q6_coff                      : 1;  /**< When set (1) and bit 23 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q6_und                       : 1;  /**< When set (1) and bit 22 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q5_perr                      : 1;  /**< When set (1) and bit 21 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q5_coff                      : 1;  /**< When set (1) and bit 20 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q5_und                       : 1;  /**< When set (1) and bit 19 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q4_perr                      : 1;  /**< When set (1) and bit 18 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q4_coff                      : 1;  /**< When set (1) and bit 17 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q4_und                       : 1;  /**< When set (1) and bit 16 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q3_perr                      : 1;  /**< When set (1) and bit 15 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q3_coff                      : 1;  /**< When set (1) and bit 14 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q3_und                       : 1;  /**< When set (1) and bit 13 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q2_perr                      : 1;  /**< When set (1) and bit 12 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q2_coff                      : 1;  /**< When set (1) and bit 11 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q2_und                       : 1;  /**< When set (1) and bit 10 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q1_perr                      : 1;  /**< When set (1) and bit 9 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q1_coff                      : 1;  /**< When set (1) and bit 8 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q1_und                       : 1;  /**< When set (1) and bit 7 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q0_perr                      : 1;  /**< When set (1) and bit 6 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q0_coff                      : 1;  /**< When set (1) and bit 5 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q0_und                       : 1;  /**< When set (1) and bit 4 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t fed1_dbe                     : 1;  /**< When set (1) and bit 3 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t fed1_sbe                     : 1;  /**< When set (1) and bit 2 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t fed0_dbe                     : 1;  /**< When set (1) and bit 1 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t fed0_sbe                     : 1;  /**< When set (1) and bit 0 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
#else
	uint64_t fed0_sbe                     : 1;
	uint64_t fed0_dbe                     : 1;
	uint64_t fed1_sbe                     : 1;
	uint64_t fed1_dbe                     : 1;
	uint64_t q0_und                       : 1;
	uint64_t q0_coff                      : 1;
	uint64_t q0_perr                      : 1;
	uint64_t q1_und                       : 1;
	uint64_t q1_coff                      : 1;
	uint64_t q1_perr                      : 1;
	uint64_t q2_und                       : 1;
	uint64_t q2_coff                      : 1;
	uint64_t q2_perr                      : 1;
	uint64_t q3_und                       : 1;
	uint64_t q3_coff                      : 1;
	uint64_t q3_perr                      : 1;
	uint64_t q4_und                       : 1;
	uint64_t q4_coff                      : 1;
	uint64_t q4_perr                      : 1;
	uint64_t q5_und                       : 1;
	uint64_t q5_coff                      : 1;
	uint64_t q5_perr                      : 1;
	uint64_t q6_und                       : 1;
	uint64_t q6_coff                      : 1;
	uint64_t q6_perr                      : 1;
	uint64_t q7_und                       : 1;
	uint64_t q7_coff                      : 1;
	uint64_t q7_perr                      : 1;
	uint64_t pool0th                      : 1;
	uint64_t pool1th                      : 1;
	uint64_t pool2th                      : 1;
	uint64_t pool3th                      : 1;
	uint64_t pool4th                      : 1;
	uint64_t pool5th                      : 1;
	uint64_t pool6th                      : 1;
	uint64_t pool7th                      : 1;
	uint64_t free0                        : 1;
	uint64_t free1                        : 1;
	uint64_t free2                        : 1;
	uint64_t free3                        : 1;
	uint64_t free4                        : 1;
	uint64_t free5                        : 1;
	uint64_t free6                        : 1;
	uint64_t free7                        : 1;
	uint64_t reserved_44_63               : 20;
#endif
	} cn63xx;
	struct cvmx_fpa_int_enb_cn30xx        cn63xxp1;
	struct cvmx_fpa_int_enb_cn61xx        cn66xx;
	struct cvmx_fpa_int_enb_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_50_63               : 14;
	uint64_t paddr_e                      : 1;  /**< When set (1) and bit 49 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t pool8th                      : 1;  /**< When set (1) and bit 48 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q8_perr                      : 1;  /**< When set (1) and bit 47 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q8_coff                      : 1;  /**< When set (1) and bit 46 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q8_und                       : 1;  /**< When set (1) and bit 45 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t free8                        : 1;  /**< When set (1) and bit 44 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t free7                        : 1;  /**< When set (1) and bit 43 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t free6                        : 1;  /**< When set (1) and bit 42 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t free5                        : 1;  /**< When set (1) and bit 41 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t free4                        : 1;  /**< When set (1) and bit 40 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t free3                        : 1;  /**< When set (1) and bit 39 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t free2                        : 1;  /**< When set (1) and bit 38 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t free1                        : 1;  /**< When set (1) and bit 37 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t free0                        : 1;  /**< When set (1) and bit 36 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t pool7th                      : 1;  /**< When set (1) and bit 35 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t pool6th                      : 1;  /**< When set (1) and bit 34 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t pool5th                      : 1;  /**< When set (1) and bit 33 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t pool4th                      : 1;  /**< When set (1) and bit 32 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t pool3th                      : 1;  /**< When set (1) and bit 31 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t pool2th                      : 1;  /**< When set (1) and bit 30 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t pool1th                      : 1;  /**< When set (1) and bit 29 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t pool0th                      : 1;  /**< When set (1) and bit 28 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q7_perr                      : 1;  /**< When set (1) and bit 27 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q7_coff                      : 1;  /**< When set (1) and bit 26 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q7_und                       : 1;  /**< When set (1) and bit 25 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q6_perr                      : 1;  /**< When set (1) and bit 24 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q6_coff                      : 1;  /**< When set (1) and bit 23 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q6_und                       : 1;  /**< When set (1) and bit 22 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q5_perr                      : 1;  /**< When set (1) and bit 21 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q5_coff                      : 1;  /**< When set (1) and bit 20 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q5_und                       : 1;  /**< When set (1) and bit 19 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q4_perr                      : 1;  /**< When set (1) and bit 18 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q4_coff                      : 1;  /**< When set (1) and bit 17 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q4_und                       : 1;  /**< When set (1) and bit 16 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q3_perr                      : 1;  /**< When set (1) and bit 15 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q3_coff                      : 1;  /**< When set (1) and bit 14 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q3_und                       : 1;  /**< When set (1) and bit 13 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q2_perr                      : 1;  /**< When set (1) and bit 12 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q2_coff                      : 1;  /**< When set (1) and bit 11 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q2_und                       : 1;  /**< When set (1) and bit 10 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q1_perr                      : 1;  /**< When set (1) and bit 9 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q1_coff                      : 1;  /**< When set (1) and bit 8 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q1_und                       : 1;  /**< When set (1) and bit 7 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q0_perr                      : 1;  /**< When set (1) and bit 6 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q0_coff                      : 1;  /**< When set (1) and bit 5 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t q0_und                       : 1;  /**< When set (1) and bit 4 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t fed1_dbe                     : 1;  /**< When set (1) and bit 3 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t fed1_sbe                     : 1;  /**< When set (1) and bit 2 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t fed0_dbe                     : 1;  /**< When set (1) and bit 1 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
	uint64_t fed0_sbe                     : 1;  /**< When set (1) and bit 0 of the FPA_INT_SUM
                                                         register is asserted the FPA will assert an
                                                         interrupt. */
#else
	uint64_t fed0_sbe                     : 1;
	uint64_t fed0_dbe                     : 1;
	uint64_t fed1_sbe                     : 1;
	uint64_t fed1_dbe                     : 1;
	uint64_t q0_und                       : 1;
	uint64_t q0_coff                      : 1;
	uint64_t q0_perr                      : 1;
	uint64_t q1_und                       : 1;
	uint64_t q1_coff                      : 1;
	uint64_t q1_perr                      : 1;
	uint64_t q2_und                       : 1;
	uint64_t q2_coff                      : 1;
	uint64_t q2_perr                      : 1;
	uint64_t q3_und                       : 1;
	uint64_t q3_coff                      : 1;
	uint64_t q3_perr                      : 1;
	uint64_t q4_und                       : 1;
	uint64_t q4_coff                      : 1;
	uint64_t q4_perr                      : 1;
	uint64_t q5_und                       : 1;
	uint64_t q5_coff                      : 1;
	uint64_t q5_perr                      : 1;
	uint64_t q6_und                       : 1;
	uint64_t q6_coff                      : 1;
	uint64_t q6_perr                      : 1;
	uint64_t q7_und                       : 1;
	uint64_t q7_coff                      : 1;
	uint64_t q7_perr                      : 1;
	uint64_t pool0th                      : 1;
	uint64_t pool1th                      : 1;
	uint64_t pool2th                      : 1;
	uint64_t pool3th                      : 1;
	uint64_t pool4th                      : 1;
	uint64_t pool5th                      : 1;
	uint64_t pool6th                      : 1;
	uint64_t pool7th                      : 1;
	uint64_t free0                        : 1;
	uint64_t free1                        : 1;
	uint64_t free2                        : 1;
	uint64_t free3                        : 1;
	uint64_t free4                        : 1;
	uint64_t free5                        : 1;
	uint64_t free6                        : 1;
	uint64_t free7                        : 1;
	uint64_t free8                        : 1;
	uint64_t q8_und                       : 1;
	uint64_t q8_coff                      : 1;
	uint64_t q8_perr                      : 1;
	uint64_t pool8th                      : 1;
	uint64_t paddr_e                      : 1;
	uint64_t reserved_50_63               : 14;
#endif
	} cn68xx;
	struct cvmx_fpa_int_enb_cn68xx        cn68xxp1;
	struct cvmx_fpa_int_enb_cn61xx        cn70xx;
	struct cvmx_fpa_int_enb_cn61xx        cn70xxp1;
	struct cvmx_fpa_int_enb_cn61xx        cnf71xx;
};
typedef union cvmx_fpa_int_enb cvmx_fpa_int_enb_t;

/**
 * cvmx_fpa_int_sum
 *
 * Contains the different interrupt summary bits of the FPA.
 *
 */
union cvmx_fpa_int_sum {
	uint64_t u64;
	struct cvmx_fpa_int_sum_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_50_63               : 14;
	uint64_t paddr_e                      : 1;  /**< Set when a pointer address does not fall in the
                                                         address range for a pool specified by
                                                         FPA_POOLX_START_ADDR and FPA_POOLX_END_ADDR. */
	uint64_t pool8th                      : 1;  /**< Set when FPA_QUE8_AVAILABLE is equal to
                                                         FPA_POOL8_THRESHOLD[THRESH] and a pointer is
                                                         allocated or de-allocated. */
	uint64_t q8_perr                      : 1;  /**< Set when a Queue8 pointer read from the stack in
                                                         the L2C does not have the FPA owner ship bit set. */
	uint64_t q8_coff                      : 1;  /**< Set when a Queue8 stack end tag is present and
                                                         the count available is greater than than pointers
                                                         present in the FPA. */
	uint64_t q8_und                       : 1;  /**< Set when a Queue8 page count available goes
                                                         negative. */
	uint64_t free8                        : 1;  /**< When a pointer for POOL8 is freed bit is set. */
	uint64_t free7                        : 1;  /**< When a pointer for POOL7 is freed bit is set. */
	uint64_t free6                        : 1;  /**< When a pointer for POOL6 is freed bit is set. */
	uint64_t free5                        : 1;  /**< When a pointer for POOL5 is freed bit is set. */
	uint64_t free4                        : 1;  /**< When a pointer for POOL4 is freed bit is set. */
	uint64_t free3                        : 1;  /**< When a pointer for POOL3 is freed bit is set. */
	uint64_t free2                        : 1;  /**< When a pointer for POOL2 is freed bit is set. */
	uint64_t free1                        : 1;  /**< When a pointer for POOL1 is freed bit is set. */
	uint64_t free0                        : 1;  /**< When a pointer for POOL0 is freed bit is set. */
	uint64_t pool7th                      : 1;  /**< Set when FPA_QUE7_AVAILABLE is equal to
                                                         FPA_POOL7_THRESHOLD[THRESH] and a pointer is
                                                         allocated or de-allocated. */
	uint64_t pool6th                      : 1;  /**< Set when FPA_QUE6_AVAILABLE is equal to
                                                         FPA_POOL6_THRESHOLD[THRESH] and a pointer is
                                                         allocated or de-allocated. */
	uint64_t pool5th                      : 1;  /**< Set when FPA_QUE5_AVAILABLE is equal to
                                                         FPA_POOL5_THRESHOLD[THRESH] and a pointer is
                                                         allocated or de-allocated. */
	uint64_t pool4th                      : 1;  /**< Set when FPA_QUE4_AVAILABLE is equal to
                                                         FPA_POOL4_THRESHOLD[THRESH] and a pointer is
                                                         allocated or de-allocated. */
	uint64_t pool3th                      : 1;  /**< Set when FPA_QUE3_AVAILABLE is equal to
                                                         FPA_POOL3_THRESHOLD[THRESH] and a pointer is
                                                         allocated or de-allocated. */
	uint64_t pool2th                      : 1;  /**< Set when FPA_QUE2_AVAILABLE is equal to
                                                         FPA_POOL2_THRESHOLD[THRESH] and a pointer is
                                                         allocated or de-allocated. */
	uint64_t pool1th                      : 1;  /**< Set when FPA_QUE1_AVAILABLE is equal to
                                                         FPA_POOL1_THRESHOLD[THRESH] and a pointer is
                                                         allocated or de-allocated. */
	uint64_t pool0th                      : 1;  /**< Set when FPA_QUE0_AVAILABLE is equal to
                                                         FPA_POOL`_THRESHOLD[THRESH] and a pointer is
                                                         allocated or de-allocated. */
	uint64_t q7_perr                      : 1;  /**< Set when a Queue0 pointer read from the stack in
                                                         the L2C does not have the FPA owner ship bit set. */
	uint64_t q7_coff                      : 1;  /**< Set when a Queue0 stack end tag is present and
                                                         the count available is greater than than pointers
                                                         present in the FPA. */
	uint64_t q7_und                       : 1;  /**< Set when a Queue0 page count available goes
                                                         negative. */
	uint64_t q6_perr                      : 1;  /**< Set when a Queue0 pointer read from the stack in
                                                         the L2C does not have the FPA owner ship bit set. */
	uint64_t q6_coff                      : 1;  /**< Set when a Queue0 stack end tag is present and
                                                         the count available is greater than than pointers
                                                         present in the FPA. */
	uint64_t q6_und                       : 1;  /**< Set when a Queue0 page count available goes
                                                         negative. */
	uint64_t q5_perr                      : 1;  /**< Set when a Queue0 pointer read from the stack in
                                                         the L2C does not have the FPA owner ship bit set. */
	uint64_t q5_coff                      : 1;  /**< Set when a Queue0 stack end tag is present and
                                                         the count available is greater than than pointers
                                                         present in the FPA. */
	uint64_t q5_und                       : 1;  /**< Set when a Queue0 page count available goes
                                                         negative. */
	uint64_t q4_perr                      : 1;  /**< Set when a Queue0 pointer read from the stack in
                                                         the L2C does not have the FPA owner ship bit set. */
	uint64_t q4_coff                      : 1;  /**< Set when a Queue0 stack end tag is present and
                                                         the count available is greater than than pointers
                                                         present in the FPA. */
	uint64_t q4_und                       : 1;  /**< Set when a Queue0 page count available goes
                                                         negative. */
	uint64_t q3_perr                      : 1;  /**< Set when a Queue0 pointer read from the stack in
                                                         the L2C does not have the FPA owner ship bit set. */
	uint64_t q3_coff                      : 1;  /**< Set when a Queue0 stack end tag is present and
                                                         the count available is greater than than pointers
                                                         present in the FPA. */
	uint64_t q3_und                       : 1;  /**< Set when a Queue0 page count available goes
                                                         negative. */
	uint64_t q2_perr                      : 1;  /**< Set when a Queue0 pointer read from the stack in
                                                         the L2C does not have the FPA owner ship bit set. */
	uint64_t q2_coff                      : 1;  /**< Set when a Queue0 stack end tag is present and
                                                         the count available is greater than than pointers
                                                         present in the FPA. */
	uint64_t q2_und                       : 1;  /**< Set when a Queue0 page count available goes
                                                         negative. */
	uint64_t q1_perr                      : 1;  /**< Set when a Queue0 pointer read from the stack in
                                                         the L2C does not have the FPA owner ship bit set. */
	uint64_t q1_coff                      : 1;  /**< Set when a Queue0 stack end tag is present and
                                                         the count available is greater than pointers
                                                         present in the FPA. */
	uint64_t q1_und                       : 1;  /**< Set when a Queue0 page count available goes
                                                         negative. */
	uint64_t q0_perr                      : 1;  /**< Set when a Queue0 pointer read from the stack in
                                                         the L2C does not have the FPA owner ship bit set. */
	uint64_t q0_coff                      : 1;  /**< Set when a Queue0 stack end tag is present and
                                                         the count available is greater than pointers
                                                         present in the FPA. */
	uint64_t q0_und                       : 1;  /**< Set when a Queue0 page count available goes
                                                         negative. */
	uint64_t fed1_dbe                     : 1;  /**< Set when a Double Bit Error is detected in FPF1. */
	uint64_t fed1_sbe                     : 1;  /**< Set when a Single Bit Error is detected in FPF1. */
	uint64_t fed0_dbe                     : 1;  /**< Set when a Double Bit Error is detected in FPF0. */
	uint64_t fed0_sbe                     : 1;  /**< Set when a Single Bit Error is detected in FPF0. */
#else
	uint64_t fed0_sbe                     : 1;
	uint64_t fed0_dbe                     : 1;
	uint64_t fed1_sbe                     : 1;
	uint64_t fed1_dbe                     : 1;
	uint64_t q0_und                       : 1;
	uint64_t q0_coff                      : 1;
	uint64_t q0_perr                      : 1;
	uint64_t q1_und                       : 1;
	uint64_t q1_coff                      : 1;
	uint64_t q1_perr                      : 1;
	uint64_t q2_und                       : 1;
	uint64_t q2_coff                      : 1;
	uint64_t q2_perr                      : 1;
	uint64_t q3_und                       : 1;
	uint64_t q3_coff                      : 1;
	uint64_t q3_perr                      : 1;
	uint64_t q4_und                       : 1;
	uint64_t q4_coff                      : 1;
	uint64_t q4_perr                      : 1;
	uint64_t q5_und                       : 1;
	uint64_t q5_coff                      : 1;
	uint64_t q5_perr                      : 1;
	uint64_t q6_und                       : 1;
	uint64_t q6_coff                      : 1;
	uint64_t q6_perr                      : 1;
	uint64_t q7_und                       : 1;
	uint64_t q7_coff                      : 1;
	uint64_t q7_perr                      : 1;
	uint64_t pool0th                      : 1;
	uint64_t pool1th                      : 1;
	uint64_t pool2th                      : 1;
	uint64_t pool3th                      : 1;
	uint64_t pool4th                      : 1;
	uint64_t pool5th                      : 1;
	uint64_t pool6th                      : 1;
	uint64_t pool7th                      : 1;
	uint64_t free0                        : 1;
	uint64_t free1                        : 1;
	uint64_t free2                        : 1;
	uint64_t free3                        : 1;
	uint64_t free4                        : 1;
	uint64_t free5                        : 1;
	uint64_t free6                        : 1;
	uint64_t free7                        : 1;
	uint64_t free8                        : 1;
	uint64_t q8_und                       : 1;
	uint64_t q8_coff                      : 1;
	uint64_t q8_perr                      : 1;
	uint64_t pool8th                      : 1;
	uint64_t paddr_e                      : 1;
	uint64_t reserved_50_63               : 14;
#endif
	} s;
	struct cvmx_fpa_int_sum_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_28_63               : 36;
	uint64_t q7_perr                      : 1;  /**< Set when a Queue0 pointer read from the stack in
                                                         the L2C does not have the FPA owner ship bit set. */
	uint64_t q7_coff                      : 1;  /**< Set when a Queue0 stack end tag is present and
                                                         the count available is greater than than pointers
                                                         present in the FPA. */
	uint64_t q7_und                       : 1;  /**< Set when a Queue0 page count available goes
                                                         negative. */
	uint64_t q6_perr                      : 1;  /**< Set when a Queue0 pointer read from the stack in
                                                         the L2C does not have the FPA owner ship bit set. */
	uint64_t q6_coff                      : 1;  /**< Set when a Queue0 stack end tag is present and
                                                         the count available is greater than than pointers
                                                         present in the FPA. */
	uint64_t q6_und                       : 1;  /**< Set when a Queue0 page count available goes
                                                         negative. */
	uint64_t q5_perr                      : 1;  /**< Set when a Queue0 pointer read from the stack in
                                                         the L2C does not have the FPA owner ship bit set. */
	uint64_t q5_coff                      : 1;  /**< Set when a Queue0 stack end tag is present and
                                                         the count available is greater than than pointers
                                                         present in the FPA. */
	uint64_t q5_und                       : 1;  /**< Set when a Queue0 page count available goes
                                                         negative. */
	uint64_t q4_perr                      : 1;  /**< Set when a Queue0 pointer read from the stack in
                                                         the L2C does not have the FPA owner ship bit set. */
	uint64_t q4_coff                      : 1;  /**< Set when a Queue0 stack end tag is present and
                                                         the count available is greater than than pointers
                                                         present in the FPA. */
	uint64_t q4_und                       : 1;  /**< Set when a Queue0 page count available goes
                                                         negative. */
	uint64_t q3_perr                      : 1;  /**< Set when a Queue0 pointer read from the stack in
                                                         the L2C does not have the FPA owner ship bit set. */
	uint64_t q3_coff                      : 1;  /**< Set when a Queue0 stack end tag is present and
                                                         the count available is greater than than pointers
                                                         present in the FPA. */
	uint64_t q3_und                       : 1;  /**< Set when a Queue0 page count available goes
                                                         negative. */
	uint64_t q2_perr                      : 1;  /**< Set when a Queue0 pointer read from the stack in
                                                         the L2C does not have the FPA owner ship bit set. */
	uint64_t q2_coff                      : 1;  /**< Set when a Queue0 stack end tag is present and
                                                         the count available is greater than than pointers
                                                         present in the FPA. */
	uint64_t q2_und                       : 1;  /**< Set when a Queue0 page count available goes
                                                         negative. */
	uint64_t q1_perr                      : 1;  /**< Set when a Queue0 pointer read from the stack in
                                                         the L2C does not have the FPA owner ship bit set. */
	uint64_t q1_coff                      : 1;  /**< Set when a Queue0 stack end tag is present and
                                                         the count available is greater than pointers
                                                         present in the FPA. */
	uint64_t q1_und                       : 1;  /**< Set when a Queue0 page count available goes
                                                         negative. */
	uint64_t q0_perr                      : 1;  /**< Set when a Queue0 pointer read from the stack in
                                                         the L2C does not have the FPA owner ship bit set. */
	uint64_t q0_coff                      : 1;  /**< Set when a Queue0 stack end tag is present and
                                                         the count available is greater than pointers
                                                         present in the FPA. */
	uint64_t q0_und                       : 1;  /**< Set when a Queue0 page count available goes
                                                         negative. */
	uint64_t fed1_dbe                     : 1;  /**< Set when a Double Bit Error is detected in FPF1. */
	uint64_t fed1_sbe                     : 1;  /**< Set when a Single Bit Error is detected in FPF1. */
	uint64_t fed0_dbe                     : 1;  /**< Set when a Double Bit Error is detected in FPF0. */
	uint64_t fed0_sbe                     : 1;  /**< Set when a Single Bit Error is detected in FPF0. */
#else
	uint64_t fed0_sbe                     : 1;
	uint64_t fed0_dbe                     : 1;
	uint64_t fed1_sbe                     : 1;
	uint64_t fed1_dbe                     : 1;
	uint64_t q0_und                       : 1;
	uint64_t q0_coff                      : 1;
	uint64_t q0_perr                      : 1;
	uint64_t q1_und                       : 1;
	uint64_t q1_coff                      : 1;
	uint64_t q1_perr                      : 1;
	uint64_t q2_und                       : 1;
	uint64_t q2_coff                      : 1;
	uint64_t q2_perr                      : 1;
	uint64_t q3_und                       : 1;
	uint64_t q3_coff                      : 1;
	uint64_t q3_perr                      : 1;
	uint64_t q4_und                       : 1;
	uint64_t q4_coff                      : 1;
	uint64_t q4_perr                      : 1;
	uint64_t q5_und                       : 1;
	uint64_t q5_coff                      : 1;
	uint64_t q5_perr                      : 1;
	uint64_t q6_und                       : 1;
	uint64_t q6_coff                      : 1;
	uint64_t q6_perr                      : 1;
	uint64_t q7_und                       : 1;
	uint64_t q7_coff                      : 1;
	uint64_t q7_perr                      : 1;
	uint64_t reserved_28_63               : 36;
#endif
	} cn30xx;
	struct cvmx_fpa_int_sum_cn30xx        cn31xx;
	struct cvmx_fpa_int_sum_cn30xx        cn38xx;
	struct cvmx_fpa_int_sum_cn30xx        cn38xxp2;
	struct cvmx_fpa_int_sum_cn30xx        cn50xx;
	struct cvmx_fpa_int_sum_cn30xx        cn52xx;
	struct cvmx_fpa_int_sum_cn30xx        cn52xxp1;
	struct cvmx_fpa_int_sum_cn30xx        cn56xx;
	struct cvmx_fpa_int_sum_cn30xx        cn56xxp1;
	struct cvmx_fpa_int_sum_cn30xx        cn58xx;
	struct cvmx_fpa_int_sum_cn30xx        cn58xxp1;
	struct cvmx_fpa_int_sum_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_50_63               : 14;
	uint64_t paddr_e                      : 1;  /**< Set when a pointer address does not fall in the
                                                         address range for a pool specified by
                                                         FPA_POOLX_START_ADDR and FPA_POOLX_END_ADDR. */
	uint64_t reserved_44_48               : 5;
	uint64_t free7                        : 1;  /**< When a pointer for POOL7 is freed bit is set. */
	uint64_t free6                        : 1;  /**< When a pointer for POOL6 is freed bit is set. */
	uint64_t free5                        : 1;  /**< When a pointer for POOL5 is freed bit is set. */
	uint64_t free4                        : 1;  /**< When a pointer for POOL4 is freed bit is set. */
	uint64_t free3                        : 1;  /**< When a pointer for POOL3 is freed bit is set. */
	uint64_t free2                        : 1;  /**< When a pointer for POOL2 is freed bit is set. */
	uint64_t free1                        : 1;  /**< When a pointer for POOL1 is freed bit is set. */
	uint64_t free0                        : 1;  /**< When a pointer for POOL0 is freed bit is set. */
	uint64_t pool7th                      : 1;  /**< Set when FPA_QUE7_AVAILABLE is equal to
                                                         FPA_POOL7_THRESHOLD[THRESH] and a pointer is
                                                         allocated or de-allocated. */
	uint64_t pool6th                      : 1;  /**< Set when FPA_QUE6_AVAILABLE is equal to
                                                         FPA_POOL6_THRESHOLD[THRESH] and a pointer is
                                                         allocated or de-allocated. */
	uint64_t pool5th                      : 1;  /**< Set when FPA_QUE5_AVAILABLE is equal to
                                                         FPA_POOL5_THRESHOLD[THRESH] and a pointer is
                                                         allocated or de-allocated. */
	uint64_t pool4th                      : 1;  /**< Set when FPA_QUE4_AVAILABLE is equal to
                                                         FPA_POOL4_THRESHOLD[THRESH] and a pointer is
                                                         allocated or de-allocated. */
	uint64_t pool3th                      : 1;  /**< Set when FPA_QUE3_AVAILABLE is equal to
                                                         FPA_POOL3_THRESHOLD[THRESH] and a pointer is
                                                         allocated or de-allocated. */
	uint64_t pool2th                      : 1;  /**< Set when FPA_QUE2_AVAILABLE is equal to
                                                         FPA_POOL2_THRESHOLD[THRESH] and a pointer is
                                                         allocated or de-allocated. */
	uint64_t pool1th                      : 1;  /**< Set when FPA_QUE1_AVAILABLE is equal to
                                                         FPA_POOL1_THRESHOLD[THRESH] and a pointer is
                                                         allocated or de-allocated. */
	uint64_t pool0th                      : 1;  /**< Set when FPA_QUE0_AVAILABLE is equal to
                                                         FPA_POOL`_THRESHOLD[THRESH] and a pointer is
                                                         allocated or de-allocated. */
	uint64_t q7_perr                      : 1;  /**< Set when a Queue0 pointer read from the stack in
                                                         the L2C does not have the FPA owner ship bit set. */
	uint64_t q7_coff                      : 1;  /**< Set when a Queue0 stack end tag is present and
                                                         the count available is greater than than pointers
                                                         present in the FPA. */
	uint64_t q7_und                       : 1;  /**< Set when a Queue0 page count available goes
                                                         negative. */
	uint64_t q6_perr                      : 1;  /**< Set when a Queue0 pointer read from the stack in
                                                         the L2C does not have the FPA owner ship bit set. */
	uint64_t q6_coff                      : 1;  /**< Set when a Queue0 stack end tag is present and
                                                         the count available is greater than than pointers
                                                         present in the FPA. */
	uint64_t q6_und                       : 1;  /**< Set when a Queue0 page count available goes
                                                         negative. */
	uint64_t q5_perr                      : 1;  /**< Set when a Queue0 pointer read from the stack in
                                                         the L2C does not have the FPA owner ship bit set. */
	uint64_t q5_coff                      : 1;  /**< Set when a Queue0 stack end tag is present and
                                                         the count available is greater than than pointers
                                                         present in the FPA. */
	uint64_t q5_und                       : 1;  /**< Set when a Queue0 page count available goes
                                                         negative. */
	uint64_t q4_perr                      : 1;  /**< Set when a Queue0 pointer read from the stack in
                                                         the L2C does not have the FPA owner ship bit set. */
	uint64_t q4_coff                      : 1;  /**< Set when a Queue0 stack end tag is present and
                                                         the count available is greater than than pointers
                                                         present in the FPA. */
	uint64_t q4_und                       : 1;  /**< Set when a Queue0 page count available goes
                                                         negative. */
	uint64_t q3_perr                      : 1;  /**< Set when a Queue0 pointer read from the stack in
                                                         the L2C does not have the FPA owner ship bit set. */
	uint64_t q3_coff                      : 1;  /**< Set when a Queue0 stack end tag is present and
                                                         the count available is greater than than pointers
                                                         present in the FPA. */
	uint64_t q3_und                       : 1;  /**< Set when a Queue0 page count available goes
                                                         negative. */
	uint64_t q2_perr                      : 1;  /**< Set when a Queue0 pointer read from the stack in
                                                         the L2C does not have the FPA owner ship bit set. */
	uint64_t q2_coff                      : 1;  /**< Set when a Queue0 stack end tag is present and
                                                         the count available is greater than than pointers
                                                         present in the FPA. */
	uint64_t q2_und                       : 1;  /**< Set when a Queue0 page count available goes
                                                         negative. */
	uint64_t q1_perr                      : 1;  /**< Set when a Queue0 pointer read from the stack in
                                                         the L2C does not have the FPA owner ship bit set. */
	uint64_t q1_coff                      : 1;  /**< Set when a Queue0 stack end tag is present and
                                                         the count available is greater than pointers
                                                         present in the FPA. */
	uint64_t q1_und                       : 1;  /**< Set when a Queue0 page count available goes
                                                         negative. */
	uint64_t q0_perr                      : 1;  /**< Set when a Queue0 pointer read from the stack in
                                                         the L2C does not have the FPA owner ship bit set. */
	uint64_t q0_coff                      : 1;  /**< Set when a Queue0 stack end tag is present and
                                                         the count available is greater than pointers
                                                         present in the FPA. */
	uint64_t q0_und                       : 1;  /**< Set when a Queue0 page count available goes
                                                         negative. */
	uint64_t fed1_dbe                     : 1;  /**< Set when a Double Bit Error is detected in FPF1. */
	uint64_t fed1_sbe                     : 1;  /**< Set when a Single Bit Error is detected in FPF1. */
	uint64_t fed0_dbe                     : 1;  /**< Set when a Double Bit Error is detected in FPF0. */
	uint64_t fed0_sbe                     : 1;  /**< Set when a Single Bit Error is detected in FPF0. */
#else
	uint64_t fed0_sbe                     : 1;
	uint64_t fed0_dbe                     : 1;
	uint64_t fed1_sbe                     : 1;
	uint64_t fed1_dbe                     : 1;
	uint64_t q0_und                       : 1;
	uint64_t q0_coff                      : 1;
	uint64_t q0_perr                      : 1;
	uint64_t q1_und                       : 1;
	uint64_t q1_coff                      : 1;
	uint64_t q1_perr                      : 1;
	uint64_t q2_und                       : 1;
	uint64_t q2_coff                      : 1;
	uint64_t q2_perr                      : 1;
	uint64_t q3_und                       : 1;
	uint64_t q3_coff                      : 1;
	uint64_t q3_perr                      : 1;
	uint64_t q4_und                       : 1;
	uint64_t q4_coff                      : 1;
	uint64_t q4_perr                      : 1;
	uint64_t q5_und                       : 1;
	uint64_t q5_coff                      : 1;
	uint64_t q5_perr                      : 1;
	uint64_t q6_und                       : 1;
	uint64_t q6_coff                      : 1;
	uint64_t q6_perr                      : 1;
	uint64_t q7_und                       : 1;
	uint64_t q7_coff                      : 1;
	uint64_t q7_perr                      : 1;
	uint64_t pool0th                      : 1;
	uint64_t pool1th                      : 1;
	uint64_t pool2th                      : 1;
	uint64_t pool3th                      : 1;
	uint64_t pool4th                      : 1;
	uint64_t pool5th                      : 1;
	uint64_t pool6th                      : 1;
	uint64_t pool7th                      : 1;
	uint64_t free0                        : 1;
	uint64_t free1                        : 1;
	uint64_t free2                        : 1;
	uint64_t free3                        : 1;
	uint64_t free4                        : 1;
	uint64_t free5                        : 1;
	uint64_t free6                        : 1;
	uint64_t free7                        : 1;
	uint64_t reserved_44_48               : 5;
	uint64_t paddr_e                      : 1;
	uint64_t reserved_50_63               : 14;
#endif
	} cn61xx;
	struct cvmx_fpa_int_sum_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_44_63               : 20;
	uint64_t free7                        : 1;  /**< When a pointer for POOL7 is freed bit is set. */
	uint64_t free6                        : 1;  /**< When a pointer for POOL6 is freed bit is set. */
	uint64_t free5                        : 1;  /**< When a pointer for POOL5 is freed bit is set. */
	uint64_t free4                        : 1;  /**< When a pointer for POOL4 is freed bit is set. */
	uint64_t free3                        : 1;  /**< When a pointer for POOL3 is freed bit is set. */
	uint64_t free2                        : 1;  /**< When a pointer for POOL2 is freed bit is set. */
	uint64_t free1                        : 1;  /**< When a pointer for POOL1 is freed bit is set. */
	uint64_t free0                        : 1;  /**< When a pointer for POOL0 is freed bit is set. */
	uint64_t pool7th                      : 1;  /**< Set when FPA_QUE7_AVAILABLE is equal to
                                                         FPA_POOL7_THRESHOLD[THRESH] and a pointer is
                                                         allocated or de-allocated. */
	uint64_t pool6th                      : 1;  /**< Set when FPA_QUE6_AVAILABLE is equal to
                                                         FPA_POOL6_THRESHOLD[THRESH] and a pointer is
                                                         allocated or de-allocated. */
	uint64_t pool5th                      : 1;  /**< Set when FPA_QUE5_AVAILABLE is equal to
                                                         FPA_POOL5_THRESHOLD[THRESH] and a pointer is
                                                         allocated or de-allocated. */
	uint64_t pool4th                      : 1;  /**< Set when FPA_QUE4_AVAILABLE is equal to
                                                         FPA_POOL4_THRESHOLD[THRESH] and a pointer is
                                                         allocated or de-allocated. */
	uint64_t pool3th                      : 1;  /**< Set when FPA_QUE3_AVAILABLE is equal to
                                                         FPA_POOL3_THRESHOLD[THRESH] and a pointer is
                                                         allocated or de-allocated. */
	uint64_t pool2th                      : 1;  /**< Set when FPA_QUE2_AVAILABLE is equal to
                                                         FPA_POOL2_THRESHOLD[THRESH] and a pointer is
                                                         allocated or de-allocated. */
	uint64_t pool1th                      : 1;  /**< Set when FPA_QUE1_AVAILABLE is equal to
                                                         FPA_POOL1_THRESHOLD[THRESH] and a pointer is
                                                         allocated or de-allocated. */
	uint64_t pool0th                      : 1;  /**< Set when FPA_QUE0_AVAILABLE is equal to
                                                         FPA_POOL`_THRESHOLD[THRESH] and a pointer is
                                                         allocated or de-allocated. */
	uint64_t q7_perr                      : 1;  /**< Set when a Queue0 pointer read from the stack in
                                                         the L2C does not have the FPA owner ship bit set. */
	uint64_t q7_coff                      : 1;  /**< Set when a Queue0 stack end tag is present and
                                                         the count available is greater than than pointers
                                                         present in the FPA. */
	uint64_t q7_und                       : 1;  /**< Set when a Queue0 page count available goes
                                                         negative. */
	uint64_t q6_perr                      : 1;  /**< Set when a Queue0 pointer read from the stack in
                                                         the L2C does not have the FPA owner ship bit set. */
	uint64_t q6_coff                      : 1;  /**< Set when a Queue0 stack end tag is present and
                                                         the count available is greater than than pointers
                                                         present in the FPA. */
	uint64_t q6_und                       : 1;  /**< Set when a Queue0 page count available goes
                                                         negative. */
	uint64_t q5_perr                      : 1;  /**< Set when a Queue0 pointer read from the stack in
                                                         the L2C does not have the FPA owner ship bit set. */
	uint64_t q5_coff                      : 1;  /**< Set when a Queue0 stack end tag is present and
                                                         the count available is greater than than pointers
                                                         present in the FPA. */
	uint64_t q5_und                       : 1;  /**< Set when a Queue0 page count available goes
                                                         negative. */
	uint64_t q4_perr                      : 1;  /**< Set when a Queue0 pointer read from the stack in
                                                         the L2C does not have the FPA owner ship bit set. */
	uint64_t q4_coff                      : 1;  /**< Set when a Queue0 stack end tag is present and
                                                         the count available is greater than than pointers
                                                         present in the FPA. */
	uint64_t q4_und                       : 1;  /**< Set when a Queue0 page count available goes
                                                         negative. */
	uint64_t q3_perr                      : 1;  /**< Set when a Queue0 pointer read from the stack in
                                                         the L2C does not have the FPA owner ship bit set. */
	uint64_t q3_coff                      : 1;  /**< Set when a Queue0 stack end tag is present and
                                                         the count available is greater than than pointers
                                                         present in the FPA. */
	uint64_t q3_und                       : 1;  /**< Set when a Queue0 page count available goes
                                                         negative. */
	uint64_t q2_perr                      : 1;  /**< Set when a Queue0 pointer read from the stack in
                                                         the L2C does not have the FPA owner ship bit set. */
	uint64_t q2_coff                      : 1;  /**< Set when a Queue0 stack end tag is present and
                                                         the count available is greater than than pointers
                                                         present in the FPA. */
	uint64_t q2_und                       : 1;  /**< Set when a Queue0 page count available goes
                                                         negative. */
	uint64_t q1_perr                      : 1;  /**< Set when a Queue0 pointer read from the stack in
                                                         the L2C does not have the FPA owner ship bit set. */
	uint64_t q1_coff                      : 1;  /**< Set when a Queue0 stack end tag is present and
                                                         the count available is greater than pointers
                                                         present in the FPA. */
	uint64_t q1_und                       : 1;  /**< Set when a Queue0 page count available goes
                                                         negative. */
	uint64_t q0_perr                      : 1;  /**< Set when a Queue0 pointer read from the stack in
                                                         the L2C does not have the FPA owner ship bit set. */
	uint64_t q0_coff                      : 1;  /**< Set when a Queue0 stack end tag is present and
                                                         the count available is greater than pointers
                                                         present in the FPA. */
	uint64_t q0_und                       : 1;  /**< Set when a Queue0 page count available goes
                                                         negative. */
	uint64_t fed1_dbe                     : 1;  /**< Set when a Double Bit Error is detected in FPF1. */
	uint64_t fed1_sbe                     : 1;  /**< Set when a Single Bit Error is detected in FPF1. */
	uint64_t fed0_dbe                     : 1;  /**< Set when a Double Bit Error is detected in FPF0. */
	uint64_t fed0_sbe                     : 1;  /**< Set when a Single Bit Error is detected in FPF0. */
#else
	uint64_t fed0_sbe                     : 1;
	uint64_t fed0_dbe                     : 1;
	uint64_t fed1_sbe                     : 1;
	uint64_t fed1_dbe                     : 1;
	uint64_t q0_und                       : 1;
	uint64_t q0_coff                      : 1;
	uint64_t q0_perr                      : 1;
	uint64_t q1_und                       : 1;
	uint64_t q1_coff                      : 1;
	uint64_t q1_perr                      : 1;
	uint64_t q2_und                       : 1;
	uint64_t q2_coff                      : 1;
	uint64_t q2_perr                      : 1;
	uint64_t q3_und                       : 1;
	uint64_t q3_coff                      : 1;
	uint64_t q3_perr                      : 1;
	uint64_t q4_und                       : 1;
	uint64_t q4_coff                      : 1;
	uint64_t q4_perr                      : 1;
	uint64_t q5_und                       : 1;
	uint64_t q5_coff                      : 1;
	uint64_t q5_perr                      : 1;
	uint64_t q6_und                       : 1;
	uint64_t q6_coff                      : 1;
	uint64_t q6_perr                      : 1;
	uint64_t q7_und                       : 1;
	uint64_t q7_coff                      : 1;
	uint64_t q7_perr                      : 1;
	uint64_t pool0th                      : 1;
	uint64_t pool1th                      : 1;
	uint64_t pool2th                      : 1;
	uint64_t pool3th                      : 1;
	uint64_t pool4th                      : 1;
	uint64_t pool5th                      : 1;
	uint64_t pool6th                      : 1;
	uint64_t pool7th                      : 1;
	uint64_t free0                        : 1;
	uint64_t free1                        : 1;
	uint64_t free2                        : 1;
	uint64_t free3                        : 1;
	uint64_t free4                        : 1;
	uint64_t free5                        : 1;
	uint64_t free6                        : 1;
	uint64_t free7                        : 1;
	uint64_t reserved_44_63               : 20;
#endif
	} cn63xx;
	struct cvmx_fpa_int_sum_cn30xx        cn63xxp1;
	struct cvmx_fpa_int_sum_cn61xx        cn66xx;
	struct cvmx_fpa_int_sum_s             cn68xx;
	struct cvmx_fpa_int_sum_s             cn68xxp1;
	struct cvmx_fpa_int_sum_cn61xx        cn70xx;
	struct cvmx_fpa_int_sum_cn61xx        cn70xxp1;
	struct cvmx_fpa_int_sum_cn61xx        cnf71xx;
};
typedef union cvmx_fpa_int_sum cvmx_fpa_int_sum_t;

/**
 * cvmx_fpa_packet_threshold
 *
 * When the value of FPA_QUE0_AVAILABLE[QUE_SIZ] is Less than the value of this register a low
 * pool count signal is sent to the
 * PCIe packet instruction engine (to make it stop reading instructions) and to the Packet-
 * Arbiter informing it to not give grants
 * to packets MAC with the exception of the PCIe MAC.
 */
union cvmx_fpa_packet_threshold {
	uint64_t u64;
	struct cvmx_fpa_packet_threshold_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t thresh                       : 32; /**< Packet Threshold. */
#else
	uint64_t thresh                       : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_fpa_packet_threshold_s    cn61xx;
	struct cvmx_fpa_packet_threshold_s    cn63xx;
	struct cvmx_fpa_packet_threshold_s    cn66xx;
	struct cvmx_fpa_packet_threshold_s    cn68xx;
	struct cvmx_fpa_packet_threshold_s    cn68xxp1;
	struct cvmx_fpa_packet_threshold_s    cn70xx;
	struct cvmx_fpa_packet_threshold_s    cn70xxp1;
	struct cvmx_fpa_packet_threshold_s    cnf71xx;
};
typedef union cvmx_fpa_packet_threshold cvmx_fpa_packet_threshold_t;

/**
 * cvmx_fpa_pool#_available
 */
union cvmx_fpa_poolx_available {
	uint64_t u64;
	struct cvmx_fpa_poolx_available_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t count                        : 36; /**< The number of free pages available in this pool. */
#else
	uint64_t count                        : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_fpa_poolx_available_s     cn73xx;
	struct cvmx_fpa_poolx_available_s     cn78xx;
	struct cvmx_fpa_poolx_available_s     cn78xxp1;
	struct cvmx_fpa_poolx_available_s     cnf75xx;
};
typedef union cvmx_fpa_poolx_available cvmx_fpa_poolx_available_t;

/**
 * cvmx_fpa_pool#_cfg
 */
union cvmx_fpa_poolx_cfg {
	uint64_t u64;
	struct cvmx_fpa_poolx_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_43_63               : 21;
	uint64_t buf_size                     : 11; /**< Buffer size in 128-byte cache lines. Must be zero if [NAT_ALIGN] is clear. Buffer sizes
                                                         are supported that are any multiple of 128 bytes in the range of 128 bytes to 128 KB. */
	uint64_t reserved_31_31               : 1;
	uint64_t buf_offset                   : 15; /**< Number of 128-byte cache lines to offset the stored pointer. This field is sign extended
                                                         so that two's complement numbers may be used to do subtractions.
                                                         If [NAT_ALIGN] is clear, the pointer stored in the pool is normally the freed pointer
                                                         adjusted by [BUF_OFFSET]. [BUF_OFFSET] will normally be zero or negative to adjust the
                                                         pointer back to the beginning of the buffer.)
                                                         If [NAT_ALIGN] is set, the pointer stored in the pool is normally [BUF_OFFSET] from the
                                                         beginning of the buffer. [BUF_OFFSET] will normally be zero or positive to adjust the
                                                         pointer into the buffer. */
	uint64_t reserved_5_15                : 11;
	uint64_t l_type                       : 2;  /**< Type of load to send to L2.
                                                         0x0 = LDD.
                                                         0x1 = LDT.
                                                         0x2 = Load with DWB.
                                                         0x3 = Reserved. */
	uint64_t s_type                       : 1;  /**< Type of store to use when FPA sends stores to L2:
                                                         0 = use STF.
                                                         1 = use STT. */
	uint64_t nat_align                    : 1;  /**< Returning buffers should be rounded to the nearest natural alignment specified with
                                                         [BUF_SIZE]. */
	uint64_t ena                          : 1;  /**< Enable. Must be set after writing pool configuration, if clear any allocations
                                                         will fail and returns will be dropped. If any pool configuration is changed
                                                         while this bit is set (or until traffic is quiesced after clearing), the FPA may
                                                         operate incorrectly. */
#else
	uint64_t ena                          : 1;
	uint64_t nat_align                    : 1;
	uint64_t s_type                       : 1;
	uint64_t l_type                       : 2;
	uint64_t reserved_5_15                : 11;
	uint64_t buf_offset                   : 15;
	uint64_t reserved_31_31               : 1;
	uint64_t buf_size                     : 11;
	uint64_t reserved_43_63               : 21;
#endif
	} s;
	struct cvmx_fpa_poolx_cfg_s           cn73xx;
	struct cvmx_fpa_poolx_cfg_s           cn78xx;
	struct cvmx_fpa_poolx_cfg_s           cn78xxp1;
	struct cvmx_fpa_poolx_cfg_s           cnf75xx;
};
typedef union cvmx_fpa_poolx_cfg cvmx_fpa_poolx_cfg_t;

/**
 * cvmx_fpa_pool#_end_addr
 *
 * Pointers sent to this pool after alignment must be equal to or less than this address.
 *
 */
union cvmx_fpa_poolx_end_addr {
	uint64_t u64;
	struct cvmx_fpa_poolx_end_addr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_fpa_poolx_end_addr_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_33_63               : 31;
	uint64_t addr                         : 33; /**< Address<39:7>. */
#else
	uint64_t addr                         : 33;
	uint64_t reserved_33_63               : 31;
#endif
	} cn61xx;
	struct cvmx_fpa_poolx_end_addr_cn61xx cn66xx;
	struct cvmx_fpa_poolx_end_addr_cn61xx cn68xx;
	struct cvmx_fpa_poolx_end_addr_cn61xx cn68xxp1;
	struct cvmx_fpa_poolx_end_addr_cn61xx cn70xx;
	struct cvmx_fpa_poolx_end_addr_cn61xx cn70xxp1;
	struct cvmx_fpa_poolx_end_addr_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_42_63               : 22;
	uint64_t addr                         : 35; /**< Address. */
	uint64_t reserved_0_6                 : 7;
#else
	uint64_t reserved_0_6                 : 7;
	uint64_t addr                         : 35;
	uint64_t reserved_42_63               : 22;
#endif
	} cn73xx;
	struct cvmx_fpa_poolx_end_addr_cn73xx cn78xx;
	struct cvmx_fpa_poolx_end_addr_cn73xx cn78xxp1;
	struct cvmx_fpa_poolx_end_addr_cn61xx cnf71xx;
	struct cvmx_fpa_poolx_end_addr_cn73xx cnf75xx;
};
typedef union cvmx_fpa_poolx_end_addr cvmx_fpa_poolx_end_addr_t;

/**
 * cvmx_fpa_pool#_fpf_marks
 *
 * The low watermark register that determines when we read free pages from L2C.
 *
 */
union cvmx_fpa_poolx_fpf_marks {
	uint64_t u64;
	struct cvmx_fpa_poolx_fpf_marks_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_27_63               : 37;
	uint64_t fpf_rd                       : 11; /**< When the number of free-page pointers in a pool drops below this value and there are free-
                                                         page pointers in DRAM, the FPA reads one page of pointers from DRAM. The recommended value
                                                         for this field is:
                                                         _  fpf_sz * 0.75
                                                         _  where, fpf_sz = 108 * 2^FPA_GEN_CFG[POOLS].
                                                         The maximum value is fpf_sz - 48.
                                                         It is recommended that software APIs represent this value as a percentage of fpf_sz, as
                                                         fpf_sz may vary between products. */
	uint64_t reserved_11_15               : 5;
	uint64_t fpf_level                    : 11; /**< Reserved. */
#else
	uint64_t fpf_level                    : 11;
	uint64_t reserved_11_15               : 5;
	uint64_t fpf_rd                       : 11;
	uint64_t reserved_27_63               : 37;
#endif
	} s;
	struct cvmx_fpa_poolx_fpf_marks_s     cn73xx;
	struct cvmx_fpa_poolx_fpf_marks_s     cn78xx;
	struct cvmx_fpa_poolx_fpf_marks_s     cn78xxp1;
	struct cvmx_fpa_poolx_fpf_marks_s     cnf75xx;
};
typedef union cvmx_fpa_poolx_fpf_marks cvmx_fpa_poolx_fpf_marks_t;

/**
 * cvmx_fpa_pool#_int
 *
 * This register indicates pool interrupts.
 *
 */
union cvmx_fpa_poolx_int {
	uint64_t u64;
	struct cvmx_fpa_poolx_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t thresh                       : 1;  /**< Set and throws FPA_INTSN_E::FPA_POOL()_THRESH when FPA_POOL()_AVAILABLE is equal
                                                         to FPA_POOL()_THRESHOLD and a pointer is allocated or deallocated. */
	uint64_t range                        : 1;  /**< Set and throws FPA_INTSN_E::FPA_POOL()_RANGE when a pointer address does not fall in
                                                         the address range for that pool specified by FPA_POOL()_START_ADDR and
                                                         FPA_POOL()_END_ADDR. */
	uint64_t crcerr                       : 1;  /**< Set and throws FPA_INTSN_E::FPA_POOL()_CRCERR when a page read from the DRAM contains
                                                         inconsistent data (FPA ownership CRC does not match what FPA wrote). Most likely indicates
                                                         the stack has been fatally corrupted. */
	uint64_t ovfls                        : 1;  /**< Set and throws FPA_INTSN_E::FPA_POOL()_OVFLS on stack overflow; when
                                                         FPA_POOL()_STACK_ADDR would exceed FPA_POOL()_STACK_END. */
#else
	uint64_t ovfls                        : 1;
	uint64_t crcerr                       : 1;
	uint64_t range                        : 1;
	uint64_t thresh                       : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_fpa_poolx_int_s           cn73xx;
	struct cvmx_fpa_poolx_int_s           cn78xx;
	struct cvmx_fpa_poolx_int_s           cn78xxp1;
	struct cvmx_fpa_poolx_int_s           cnf75xx;
};
typedef union cvmx_fpa_poolx_int cvmx_fpa_poolx_int_t;

/**
 * cvmx_fpa_pool#_op_pc
 */
union cvmx_fpa_poolx_op_pc {
	uint64_t u64;
	struct cvmx_fpa_poolx_op_pc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t count                        : 64; /**< Number of allocations or returns performed to this pool, including those that
                                                         fail due to RED/DROP. Does not include aura count change requests (from PKI/PKO)
                                                         that come without allocation/returns. */
#else
	uint64_t count                        : 64;
#endif
	} s;
	struct cvmx_fpa_poolx_op_pc_s         cn73xx;
	struct cvmx_fpa_poolx_op_pc_s         cn78xx;
	struct cvmx_fpa_poolx_op_pc_s         cn78xxp1;
	struct cvmx_fpa_poolx_op_pc_s         cnf75xx;
};
typedef union cvmx_fpa_poolx_op_pc cvmx_fpa_poolx_op_pc_t;

/**
 * cvmx_fpa_pool#_stack_addr
 */
union cvmx_fpa_poolx_stack_addr {
	uint64_t u64;
	struct cvmx_fpa_poolx_stack_addr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_42_63               : 22;
	uint64_t addr                         : 35; /**< Next address. The address of the next stack write. Must be initialized to
                                                         FPA_POOL()_STACK_BASE[ADDR] when stack is created. */
	uint64_t reserved_0_6                 : 7;
#else
	uint64_t reserved_0_6                 : 7;
	uint64_t addr                         : 35;
	uint64_t reserved_42_63               : 22;
#endif
	} s;
	struct cvmx_fpa_poolx_stack_addr_s    cn73xx;
	struct cvmx_fpa_poolx_stack_addr_s    cn78xx;
	struct cvmx_fpa_poolx_stack_addr_s    cn78xxp1;
	struct cvmx_fpa_poolx_stack_addr_s    cnf75xx;
};
typedef union cvmx_fpa_poolx_stack_addr cvmx_fpa_poolx_stack_addr_t;

/**
 * cvmx_fpa_pool#_stack_base
 */
union cvmx_fpa_poolx_stack_base {
	uint64_t u64;
	struct cvmx_fpa_poolx_stack_base_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_42_63               : 22;
	uint64_t addr                         : 35; /**< Base address. The lowest address used by the pool's stack. */
	uint64_t reserved_0_6                 : 7;
#else
	uint64_t reserved_0_6                 : 7;
	uint64_t addr                         : 35;
	uint64_t reserved_42_63               : 22;
#endif
	} s;
	struct cvmx_fpa_poolx_stack_base_s    cn73xx;
	struct cvmx_fpa_poolx_stack_base_s    cn78xx;
	struct cvmx_fpa_poolx_stack_base_s    cn78xxp1;
	struct cvmx_fpa_poolx_stack_base_s    cnf75xx;
};
typedef union cvmx_fpa_poolx_stack_base cvmx_fpa_poolx_stack_base_t;

/**
 * cvmx_fpa_pool#_stack_end
 */
union cvmx_fpa_poolx_stack_end {
	uint64_t u64;
	struct cvmx_fpa_poolx_stack_end_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_42_63               : 22;
	uint64_t addr                         : 35; /**< Stack ending address plus one line; hardware will never write this address. If
                                                         FPA_POOL()_STACK_ADDR is equal to this value, the stack is full. */
	uint64_t reserved_0_6                 : 7;
#else
	uint64_t reserved_0_6                 : 7;
	uint64_t addr                         : 35;
	uint64_t reserved_42_63               : 22;
#endif
	} s;
	struct cvmx_fpa_poolx_stack_end_s     cn73xx;
	struct cvmx_fpa_poolx_stack_end_s     cn78xx;
	struct cvmx_fpa_poolx_stack_end_s     cn78xxp1;
	struct cvmx_fpa_poolx_stack_end_s     cnf75xx;
};
typedef union cvmx_fpa_poolx_stack_end cvmx_fpa_poolx_stack_end_t;

/**
 * cvmx_fpa_pool#_start_addr
 *
 * Pointers sent to this pool after alignment must be equal to or greater than this address.
 *
 */
union cvmx_fpa_poolx_start_addr {
	uint64_t u64;
	struct cvmx_fpa_poolx_start_addr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_fpa_poolx_start_addr_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_33_63               : 31;
	uint64_t addr                         : 33; /**< Address<39:7>. */
#else
	uint64_t addr                         : 33;
	uint64_t reserved_33_63               : 31;
#endif
	} cn61xx;
	struct cvmx_fpa_poolx_start_addr_cn61xx cn66xx;
	struct cvmx_fpa_poolx_start_addr_cn61xx cn68xx;
	struct cvmx_fpa_poolx_start_addr_cn61xx cn68xxp1;
	struct cvmx_fpa_poolx_start_addr_cn61xx cn70xx;
	struct cvmx_fpa_poolx_start_addr_cn61xx cn70xxp1;
	struct cvmx_fpa_poolx_start_addr_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_42_63               : 22;
	uint64_t addr                         : 35; /**< Address. Defaults to 1 so that a NULL pointer free will cause an exception. */
	uint64_t reserved_0_6                 : 7;
#else
	uint64_t reserved_0_6                 : 7;
	uint64_t addr                         : 35;
	uint64_t reserved_42_63               : 22;
#endif
	} cn73xx;
	struct cvmx_fpa_poolx_start_addr_cn73xx cn78xx;
	struct cvmx_fpa_poolx_start_addr_cn73xx cn78xxp1;
	struct cvmx_fpa_poolx_start_addr_cn61xx cnf71xx;
	struct cvmx_fpa_poolx_start_addr_cn73xx cnf75xx;
};
typedef union cvmx_fpa_poolx_start_addr cvmx_fpa_poolx_start_addr_t;

/**
 * cvmx_fpa_pool#_threshold
 *
 * FPA_POOLX_THRESHOLD = FPA's Pool 0-7 Threshold
 * When the value of FPA_QUEX_AVAILABLE is equal to FPA_POOLX_THRESHOLD[THRESH] when a pointer is
 * allocated
 * or deallocated, set interrupt FPA_INT_SUM[POOLXTH].
 */
union cvmx_fpa_poolx_threshold {
	uint64_t u64;
	struct cvmx_fpa_poolx_threshold_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t thresh                       : 36; /**< Threshold for the pool. When the value of FPA_POOL()_AVAILABLE[COUNT] is equal to
                                                         [THRESH] and a pointer is allocated or freed, set interrupt
                                                         FPA_INTSN_E::FPA_POOL()_THRESH. */
#else
	uint64_t thresh                       : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_fpa_poolx_threshold_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_29_63               : 35;
	uint64_t thresh                       : 29; /**< The Threshold. */
#else
	uint64_t thresh                       : 29;
	uint64_t reserved_29_63               : 35;
#endif
	} cn61xx;
	struct cvmx_fpa_poolx_threshold_cn61xx cn63xx;
	struct cvmx_fpa_poolx_threshold_cn61xx cn66xx;
	struct cvmx_fpa_poolx_threshold_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t thresh                       : 32; /**< The Threshold. */
#else
	uint64_t thresh                       : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} cn68xx;
	struct cvmx_fpa_poolx_threshold_cn68xx cn68xxp1;
	struct cvmx_fpa_poolx_threshold_cn61xx cn70xx;
	struct cvmx_fpa_poolx_threshold_cn61xx cn70xxp1;
	struct cvmx_fpa_poolx_threshold_s     cn73xx;
	struct cvmx_fpa_poolx_threshold_s     cn78xx;
	struct cvmx_fpa_poolx_threshold_s     cn78xxp1;
	struct cvmx_fpa_poolx_threshold_cn61xx cnf71xx;
	struct cvmx_fpa_poolx_threshold_s     cnf75xx;
};
typedef union cvmx_fpa_poolx_threshold cvmx_fpa_poolx_threshold_t;

/**
 * cvmx_fpa_que#_available
 *
 * FPA_QUEX_PAGES_AVAILABLE = FPA's Queue 0-7 Free Page Available Register
 * The number of page pointers that are available in the FPA and local DRAM.
 */
union cvmx_fpa_quex_available {
	uint64_t u64;
	struct cvmx_fpa_quex_available_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t que_siz                      : 32; /**< The number of free pages available in this Queue.
                                                         In PASS-1 this field was [25:0]. */
#else
	uint64_t que_siz                      : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_fpa_quex_available_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_29_63               : 35;
	uint64_t que_siz                      : 29; /**< The number of free pages available in this Queue. */
#else
	uint64_t que_siz                      : 29;
	uint64_t reserved_29_63               : 35;
#endif
	} cn30xx;
	struct cvmx_fpa_quex_available_cn30xx cn31xx;
	struct cvmx_fpa_quex_available_cn30xx cn38xx;
	struct cvmx_fpa_quex_available_cn30xx cn38xxp2;
	struct cvmx_fpa_quex_available_cn30xx cn50xx;
	struct cvmx_fpa_quex_available_cn30xx cn52xx;
	struct cvmx_fpa_quex_available_cn30xx cn52xxp1;
	struct cvmx_fpa_quex_available_cn30xx cn56xx;
	struct cvmx_fpa_quex_available_cn30xx cn56xxp1;
	struct cvmx_fpa_quex_available_cn30xx cn58xx;
	struct cvmx_fpa_quex_available_cn30xx cn58xxp1;
	struct cvmx_fpa_quex_available_cn30xx cn61xx;
	struct cvmx_fpa_quex_available_cn30xx cn63xx;
	struct cvmx_fpa_quex_available_cn30xx cn63xxp1;
	struct cvmx_fpa_quex_available_cn30xx cn66xx;
	struct cvmx_fpa_quex_available_s      cn68xx;
	struct cvmx_fpa_quex_available_s      cn68xxp1;
	struct cvmx_fpa_quex_available_cn30xx cn70xx;
	struct cvmx_fpa_quex_available_cn30xx cn70xxp1;
	struct cvmx_fpa_quex_available_cn30xx cnf71xx;
};
typedef union cvmx_fpa_quex_available cvmx_fpa_quex_available_t;

/**
 * cvmx_fpa_que#_page_index
 *
 * The present index page for queue 0 of the FPA.
 * This number reflects the number of pages of pointers that have been written to memory
 * for this queue.
 */
union cvmx_fpa_quex_page_index {
	uint64_t u64;
	struct cvmx_fpa_quex_page_index_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_25_63               : 39;
	uint64_t pg_num                       : 25; /**< Page number. */
#else
	uint64_t pg_num                       : 25;
	uint64_t reserved_25_63               : 39;
#endif
	} s;
	struct cvmx_fpa_quex_page_index_s     cn30xx;
	struct cvmx_fpa_quex_page_index_s     cn31xx;
	struct cvmx_fpa_quex_page_index_s     cn38xx;
	struct cvmx_fpa_quex_page_index_s     cn38xxp2;
	struct cvmx_fpa_quex_page_index_s     cn50xx;
	struct cvmx_fpa_quex_page_index_s     cn52xx;
	struct cvmx_fpa_quex_page_index_s     cn52xxp1;
	struct cvmx_fpa_quex_page_index_s     cn56xx;
	struct cvmx_fpa_quex_page_index_s     cn56xxp1;
	struct cvmx_fpa_quex_page_index_s     cn58xx;
	struct cvmx_fpa_quex_page_index_s     cn58xxp1;
	struct cvmx_fpa_quex_page_index_s     cn61xx;
	struct cvmx_fpa_quex_page_index_s     cn63xx;
	struct cvmx_fpa_quex_page_index_s     cn63xxp1;
	struct cvmx_fpa_quex_page_index_s     cn66xx;
	struct cvmx_fpa_quex_page_index_s     cn68xx;
	struct cvmx_fpa_quex_page_index_s     cn68xxp1;
	struct cvmx_fpa_quex_page_index_s     cn70xx;
	struct cvmx_fpa_quex_page_index_s     cn70xxp1;
	struct cvmx_fpa_quex_page_index_s     cnf71xx;
};
typedef union cvmx_fpa_quex_page_index cvmx_fpa_quex_page_index_t;

/**
 * cvmx_fpa_que8_page_index
 *
 * FPA_QUE8_PAGE_INDEX = FPA's Queue7 Page Index
 *
 * The present index page for queue 7 of the FPA.
 * This number reflects the number of pages of pointers that have been written to memory
 * for this queue.
 * Because the address space is 38-bits the number of 128 byte pages could cause this register value to wrap.
 */
union cvmx_fpa_que8_page_index {
	uint64_t u64;
	struct cvmx_fpa_que8_page_index_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_25_63               : 39;
	uint64_t pg_num                       : 25; /**< Page number. */
#else
	uint64_t pg_num                       : 25;
	uint64_t reserved_25_63               : 39;
#endif
	} s;
	struct cvmx_fpa_que8_page_index_s     cn68xx;
	struct cvmx_fpa_que8_page_index_s     cn68xxp1;
};
typedef union cvmx_fpa_que8_page_index cvmx_fpa_que8_page_index_t;

/**
 * cvmx_fpa_que_act
 *
 * "When a INT_SUM[PERR#] occurs this will be latched with the value read from L2C.
 * This is latched on the first error and will not latch again unitl all errors are cleared."
 */
union cvmx_fpa_que_act {
	uint64_t u64;
	struct cvmx_fpa_que_act_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_29_63               : 35;
	uint64_t act_que                      : 3;  /**< FPA-queue-number read from memory. */
	uint64_t act_indx                     : 26; /**< Page number read from memory. */
#else
	uint64_t act_indx                     : 26;
	uint64_t act_que                      : 3;
	uint64_t reserved_29_63               : 35;
#endif
	} s;
	struct cvmx_fpa_que_act_s             cn30xx;
	struct cvmx_fpa_que_act_s             cn31xx;
	struct cvmx_fpa_que_act_s             cn38xx;
	struct cvmx_fpa_que_act_s             cn38xxp2;
	struct cvmx_fpa_que_act_s             cn50xx;
	struct cvmx_fpa_que_act_s             cn52xx;
	struct cvmx_fpa_que_act_s             cn52xxp1;
	struct cvmx_fpa_que_act_s             cn56xx;
	struct cvmx_fpa_que_act_s             cn56xxp1;
	struct cvmx_fpa_que_act_s             cn58xx;
	struct cvmx_fpa_que_act_s             cn58xxp1;
	struct cvmx_fpa_que_act_s             cn61xx;
	struct cvmx_fpa_que_act_s             cn63xx;
	struct cvmx_fpa_que_act_s             cn63xxp1;
	struct cvmx_fpa_que_act_s             cn66xx;
	struct cvmx_fpa_que_act_s             cn68xx;
	struct cvmx_fpa_que_act_s             cn68xxp1;
	struct cvmx_fpa_que_act_s             cn70xx;
	struct cvmx_fpa_que_act_s             cn70xxp1;
	struct cvmx_fpa_que_act_s             cnf71xx;
};
typedef union cvmx_fpa_que_act cvmx_fpa_que_act_t;

/**
 * cvmx_fpa_que_exp
 *
 * "When a INT_SUM[PERR#] occurs this will be latched with the expected value.
 * This is latched on the first error and will not latch again unitl all errors are cleared."
 */
union cvmx_fpa_que_exp {
	uint64_t u64;
	struct cvmx_fpa_que_exp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_29_63               : 35;
	uint64_t exp_que                      : 3;  /**< Expected fpa-queue-number read from memory. */
	uint64_t exp_indx                     : 26; /**< Expected page number read from memory. */
#else
	uint64_t exp_indx                     : 26;
	uint64_t exp_que                      : 3;
	uint64_t reserved_29_63               : 35;
#endif
	} s;
	struct cvmx_fpa_que_exp_s             cn30xx;
	struct cvmx_fpa_que_exp_s             cn31xx;
	struct cvmx_fpa_que_exp_s             cn38xx;
	struct cvmx_fpa_que_exp_s             cn38xxp2;
	struct cvmx_fpa_que_exp_s             cn50xx;
	struct cvmx_fpa_que_exp_s             cn52xx;
	struct cvmx_fpa_que_exp_s             cn52xxp1;
	struct cvmx_fpa_que_exp_s             cn56xx;
	struct cvmx_fpa_que_exp_s             cn56xxp1;
	struct cvmx_fpa_que_exp_s             cn58xx;
	struct cvmx_fpa_que_exp_s             cn58xxp1;
	struct cvmx_fpa_que_exp_s             cn61xx;
	struct cvmx_fpa_que_exp_s             cn63xx;
	struct cvmx_fpa_que_exp_s             cn63xxp1;
	struct cvmx_fpa_que_exp_s             cn66xx;
	struct cvmx_fpa_que_exp_s             cn68xx;
	struct cvmx_fpa_que_exp_s             cn68xxp1;
	struct cvmx_fpa_que_exp_s             cn70xx;
	struct cvmx_fpa_que_exp_s             cn70xxp1;
	struct cvmx_fpa_que_exp_s             cnf71xx;
};
typedef union cvmx_fpa_que_exp cvmx_fpa_que_exp_t;

/**
 * cvmx_fpa_rd_latency_pc
 */
union cvmx_fpa_rd_latency_pc {
	uint64_t u64;
	struct cvmx_fpa_rd_latency_pc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t count                        : 64; /**< Number of cycles waiting for L2C pool read returns. Incremented every
                                                         coprocessor-clock by the number of transactions outstanding in that cycle. This
                                                         may be divided by FPA_RD_REQ_PC to determine the average read latency. */
#else
	uint64_t count                        : 64;
#endif
	} s;
	struct cvmx_fpa_rd_latency_pc_s       cn73xx;
	struct cvmx_fpa_rd_latency_pc_s       cn78xx;
	struct cvmx_fpa_rd_latency_pc_s       cn78xxp1;
	struct cvmx_fpa_rd_latency_pc_s       cnf75xx;
};
typedef union cvmx_fpa_rd_latency_pc cvmx_fpa_rd_latency_pc_t;

/**
 * cvmx_fpa_rd_req_pc
 */
union cvmx_fpa_rd_req_pc {
	uint64_t u64;
	struct cvmx_fpa_rd_req_pc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t count                        : 64; /**< Number of L2C pool read requests. */
#else
	uint64_t count                        : 64;
#endif
	} s;
	struct cvmx_fpa_rd_req_pc_s           cn73xx;
	struct cvmx_fpa_rd_req_pc_s           cn78xx;
	struct cvmx_fpa_rd_req_pc_s           cn78xxp1;
	struct cvmx_fpa_rd_req_pc_s           cnf75xx;
};
typedef union cvmx_fpa_rd_req_pc cvmx_fpa_rd_req_pc_t;

/**
 * cvmx_fpa_red_delay
 */
union cvmx_fpa_red_delay {
	uint64_t u64;
	struct cvmx_fpa_red_delay_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t avg_dly                      : 14; /**< Average-queue-size delay. The number of levelizer-clock cycles to wait (1024 *
                                                         ([AVG_DLY]+1) * (FPA_GEN_CFG[LVL_DLY]+1)) coprocessor clocks) between calculating the
                                                         moving average for each aura level. Note the minimum value of 2048 cycles implies that at
                                                         100 M packets/sec, 1.2 GHz coprocessor clock, approximately 160 packets may arrive between
                                                         average calculations.
                                                         Larger FPA_GEN_CFG[LVL_DLY] values cause the backpressure indications and moving averages
                                                         of all aura levels to track changes in the actual free space more slowly. Larger [AVG_DLY]
                                                         also causes the moving averages of all aura levels to track changes in the actual free
                                                         space more slowly, but does not affect backpressure. Larger
                                                         FPA_AURA()_CFG[AVG_CON]) values causes a specific aura to track more slowly, but
                                                         only affects an individual aura level, rather than all. */
#else
	uint64_t avg_dly                      : 14;
	uint64_t reserved_14_63               : 50;
#endif
	} s;
	struct cvmx_fpa_red_delay_s           cn73xx;
	struct cvmx_fpa_red_delay_s           cn78xx;
	struct cvmx_fpa_red_delay_s           cn78xxp1;
	struct cvmx_fpa_red_delay_s           cnf75xx;
};
typedef union cvmx_fpa_red_delay cvmx_fpa_red_delay_t;

/**
 * cvmx_fpa_sft_rst
 *
 * Allows soft reset.
 *
 */
union cvmx_fpa_sft_rst {
	uint64_t u64;
	struct cvmx_fpa_sft_rst_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t busy                         : 1;  /**< When 1, FPA is busy completing reset. No access except the reading of this bit should
                                                         occur to the FPA until this is clear. */
	uint64_t reserved_1_62                : 62;
	uint64_t rst                          : 1;  /**< Reset. When set to 1 by software, FPA gets a short reset pulse (three cycles in duration).
                                                         Following a write to this register and prior to performing another FPA operation, software
                                                         must write SSO_BIST_STATUS0 (or any register on the same IOI bus as FPA) and read it back. */
#else
	uint64_t rst                          : 1;
	uint64_t reserved_1_62                : 62;
	uint64_t busy                         : 1;
#endif
	} s;
	struct cvmx_fpa_sft_rst_s             cn73xx;
	struct cvmx_fpa_sft_rst_s             cn78xx;
	struct cvmx_fpa_sft_rst_s             cn78xxp1;
	struct cvmx_fpa_sft_rst_s             cnf75xx;
};
typedef union cvmx_fpa_sft_rst cvmx_fpa_sft_rst_t;

/**
 * cvmx_fpa_wart_ctl
 *
 * FPA_WART_CTL = FPA's WART Control
 *
 * Control and status for the WART block.
 */
union cvmx_fpa_wart_ctl {
	uint64_t u64;
	struct cvmx_fpa_wart_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t ctl                          : 16; /**< Control information. */
#else
	uint64_t ctl                          : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_fpa_wart_ctl_s            cn30xx;
	struct cvmx_fpa_wart_ctl_s            cn31xx;
	struct cvmx_fpa_wart_ctl_s            cn38xx;
	struct cvmx_fpa_wart_ctl_s            cn38xxp2;
	struct cvmx_fpa_wart_ctl_s            cn50xx;
	struct cvmx_fpa_wart_ctl_s            cn52xx;
	struct cvmx_fpa_wart_ctl_s            cn52xxp1;
	struct cvmx_fpa_wart_ctl_s            cn56xx;
	struct cvmx_fpa_wart_ctl_s            cn56xxp1;
	struct cvmx_fpa_wart_ctl_s            cn58xx;
	struct cvmx_fpa_wart_ctl_s            cn58xxp1;
};
typedef union cvmx_fpa_wart_ctl cvmx_fpa_wart_ctl_t;

/**
 * cvmx_fpa_wart_status
 *
 * FPA_WART_STATUS = FPA's WART Status
 *
 * Control and status for the WART block.
 */
union cvmx_fpa_wart_status {
	uint64_t u64;
	struct cvmx_fpa_wart_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t status                       : 32; /**< Status information. */
#else
	uint64_t status                       : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_fpa_wart_status_s         cn30xx;
	struct cvmx_fpa_wart_status_s         cn31xx;
	struct cvmx_fpa_wart_status_s         cn38xx;
	struct cvmx_fpa_wart_status_s         cn38xxp2;
	struct cvmx_fpa_wart_status_s         cn50xx;
	struct cvmx_fpa_wart_status_s         cn52xx;
	struct cvmx_fpa_wart_status_s         cn52xxp1;
	struct cvmx_fpa_wart_status_s         cn56xx;
	struct cvmx_fpa_wart_status_s         cn56xxp1;
	struct cvmx_fpa_wart_status_s         cn58xx;
	struct cvmx_fpa_wart_status_s         cn58xxp1;
};
typedef union cvmx_fpa_wart_status cvmx_fpa_wart_status_t;

/**
 * cvmx_fpa_wqe_threshold
 *
 * "When the value of FPA_QUE#_AVAILABLE[QUE_SIZ] (\# is determined by the value of
 * IPD_WQE_FPA_QUEUE) is Less than the value of this
 * register a low pool count signal is sent to the PCIe packet instruction engine (to make it
 * stop reading instructions) and to the
 * Packet-Arbiter informing it to not give grants to packets MAC with the exception of the PCIe
 * MAC."
 */
union cvmx_fpa_wqe_threshold {
	uint64_t u64;
	struct cvmx_fpa_wqe_threshold_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t thresh                       : 32; /**< WQE Threshold. */
#else
	uint64_t thresh                       : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_fpa_wqe_threshold_s       cn61xx;
	struct cvmx_fpa_wqe_threshold_s       cn63xx;
	struct cvmx_fpa_wqe_threshold_s       cn66xx;
	struct cvmx_fpa_wqe_threshold_s       cn68xx;
	struct cvmx_fpa_wqe_threshold_s       cn68xxp1;
	struct cvmx_fpa_wqe_threshold_s       cn70xx;
	struct cvmx_fpa_wqe_threshold_s       cn70xxp1;
	struct cvmx_fpa_wqe_threshold_s       cnf71xx;
};
typedef union cvmx_fpa_wqe_threshold cvmx_fpa_wqe_threshold_t;

#endif
