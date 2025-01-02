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
 * cvmx-dpi-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon dpi.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_DPI_DEFS_H__
#define __CVMX_DPI_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DPI_BIST_STATUS CVMX_DPI_BIST_STATUS_FUNC()
static inline uint64_t CVMX_DPI_BIST_STATUS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF71XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_DPI_BIST_STATUS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001DF0000000000ull);
}
#else
#define CVMX_DPI_BIST_STATUS (CVMX_ADD_IO_SEG(0x0001DF0000000000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DPI_CTL CVMX_DPI_CTL_FUNC()
static inline uint64_t CVMX_DPI_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF71XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_DPI_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001DF0000000040ull);
}
#else
#define CVMX_DPI_CTL (CVMX_ADD_IO_SEG(0x0001DF0000000040ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DPI_DMAX_COUNTS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 7)))))
		cvmx_warn("CVMX_DPI_DMAX_COUNTS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001DF0000000300ull) + ((offset) & 7) * 8;
}
#else
#define CVMX_DPI_DMAX_COUNTS(offset) (CVMX_ADD_IO_SEG(0x0001DF0000000300ull) + ((offset) & 7) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DPI_DMAX_DBELL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 7)))))
		cvmx_warn("CVMX_DPI_DMAX_DBELL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001DF0000000200ull) + ((offset) & 7) * 8;
}
#else
#define CVMX_DPI_DMAX_DBELL(offset) (CVMX_ADD_IO_SEG(0x0001DF0000000200ull) + ((offset) & 7) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DPI_DMAX_ERR_RSP_STATUS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 7)))))
		cvmx_warn("CVMX_DPI_DMAX_ERR_RSP_STATUS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001DF0000000A80ull) + ((offset) & 7) * 8;
}
#else
#define CVMX_DPI_DMAX_ERR_RSP_STATUS(offset) (CVMX_ADD_IO_SEG(0x0001DF0000000A80ull) + ((offset) & 7) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DPI_DMAX_IBUFF_SADDR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 7)))))
		cvmx_warn("CVMX_DPI_DMAX_IBUFF_SADDR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001DF0000000280ull) + ((offset) & 7) * 8;
}
#else
#define CVMX_DPI_DMAX_IBUFF_SADDR(offset) (CVMX_ADD_IO_SEG(0x0001DF0000000280ull) + ((offset) & 7) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DPI_DMAX_IFLIGHT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 7)))))
		cvmx_warn("CVMX_DPI_DMAX_IFLIGHT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001DF0000000A00ull) + ((offset) & 7) * 8;
}
#else
#define CVMX_DPI_DMAX_IFLIGHT(offset) (CVMX_ADD_IO_SEG(0x0001DF0000000A00ull) + ((offset) & 7) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DPI_DMAX_NADDR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 7)))))
		cvmx_warn("CVMX_DPI_DMAX_NADDR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001DF0000000380ull) + ((offset) & 7) * 8;
}
#else
#define CVMX_DPI_DMAX_NADDR(offset) (CVMX_ADD_IO_SEG(0x0001DF0000000380ull) + ((offset) & 7) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DPI_DMAX_REQBNK0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 7)))))
		cvmx_warn("CVMX_DPI_DMAX_REQBNK0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001DF0000000400ull) + ((offset) & 7) * 8;
}
#else
#define CVMX_DPI_DMAX_REQBNK0(offset) (CVMX_ADD_IO_SEG(0x0001DF0000000400ull) + ((offset) & 7) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DPI_DMAX_REQBNK1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 7)))))
		cvmx_warn("CVMX_DPI_DMAX_REQBNK1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001DF0000000480ull) + ((offset) & 7) * 8;
}
#else
#define CVMX_DPI_DMAX_REQBNK1(offset) (CVMX_ADD_IO_SEG(0x0001DF0000000480ull) + ((offset) & 7) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DPI_DMAX_REQQ_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 7)))))
		cvmx_warn("CVMX_DPI_DMAX_REQQ_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001DF0000000180ull) + ((offset) & 7) * 8;
}
#else
#define CVMX_DPI_DMAX_REQQ_CTL(offset) (CVMX_ADD_IO_SEG(0x0001DF0000000180ull) + ((offset) & 7) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DPI_DMA_CONTROL CVMX_DPI_DMA_CONTROL_FUNC()
static inline uint64_t CVMX_DPI_DMA_CONTROL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF71XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_DPI_DMA_CONTROL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001DF0000000048ull);
}
#else
#define CVMX_DPI_DMA_CONTROL (CVMX_ADD_IO_SEG(0x0001DF0000000048ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DPI_DMA_ENGX_EN(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 5))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 5))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 5))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 5))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 5))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 5))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 5))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 5))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 5))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 5)))))
		cvmx_warn("CVMX_DPI_DMA_ENGX_EN(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001DF0000000080ull) + ((offset) & 7) * 8;
}
#else
#define CVMX_DPI_DMA_ENGX_EN(offset) (CVMX_ADD_IO_SEG(0x0001DF0000000080ull) + ((offset) & 7) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DPI_DMA_PPX_CNT(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
			if ((offset <= 3))
				return CVMX_ADD_IO_SEG(0x0001DF0000000B00ull) + ((offset) & 3) * 8;
			break;
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 31))
				return CVMX_ADD_IO_SEG(0x0001DF0000000B00ull) + ((offset) & 31) * 8;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 15))
				return CVMX_ADD_IO_SEG(0x0001DF0000000C00ull) + ((offset) & 15) * 8;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 47))
					return CVMX_ADD_IO_SEG(0x0001DF0000000C00ull) + ((offset) & 63) * 8;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 47))
					return CVMX_ADD_IO_SEG(0x0001DF0000000C00ull) + ((offset) & 63) * 8;

			break;
	}
	cvmx_warn("CVMX_DPI_DMA_PPX_CNT (offset = %lu) not supported on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001DF0000000C00ull) + ((offset) & 15) * 8;
}
#else
static inline uint64_t CVMX_DPI_DMA_PPX_CNT(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001DF0000000B00ull) + (offset) * 8;
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001DF0000000B00ull) + (offset) * 8;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001DF0000000C00ull) + (offset) * 8;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return CVMX_ADD_IO_SEG(0x0001DF0000000C00ull) + (offset) * 8;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return CVMX_ADD_IO_SEG(0x0001DF0000000C00ull) + (offset) * 8;

	}
	return CVMX_ADD_IO_SEG(0x0001DF0000000C00ull) + (offset) * 8;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DPI_DMA_PP_INT CVMX_DPI_DMA_PP_INT_FUNC()
static inline uint64_t CVMX_DPI_DMA_PP_INT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_DPI_DMA_PP_INT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001DF0000000038ull);
}
#else
#define CVMX_DPI_DMA_PP_INT (CVMX_ADD_IO_SEG(0x0001DF0000000038ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DPI_ECC_CTL CVMX_DPI_ECC_CTL_FUNC()
static inline uint64_t CVMX_DPI_ECC_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_DPI_ECC_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001DF0000000018ull);
}
#else
#define CVMX_DPI_ECC_CTL (CVMX_ADD_IO_SEG(0x0001DF0000000018ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DPI_ECC_INT CVMX_DPI_ECC_INT_FUNC()
static inline uint64_t CVMX_DPI_ECC_INT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_DPI_ECC_INT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001DF0000000020ull);
}
#else
#define CVMX_DPI_ECC_INT (CVMX_ADD_IO_SEG(0x0001DF0000000020ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DPI_ENGX_BUF(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 5))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 5))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 5))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 5))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 5))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 5))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 5))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 5))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 5))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 5)))))
		cvmx_warn("CVMX_DPI_ENGX_BUF(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001DF0000000880ull) + ((offset) & 7) * 8;
}
#else
#define CVMX_DPI_ENGX_BUF(offset) (CVMX_ADD_IO_SEG(0x0001DF0000000880ull) + ((offset) & 7) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DPI_INFO_REG CVMX_DPI_INFO_REG_FUNC()
static inline uint64_t CVMX_DPI_INFO_REG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF71XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_DPI_INFO_REG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001DF0000000980ull);
}
#else
#define CVMX_DPI_INFO_REG (CVMX_ADD_IO_SEG(0x0001DF0000000980ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DPI_INT_EN CVMX_DPI_INT_EN_FUNC()
static inline uint64_t CVMX_DPI_INT_EN_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_DPI_INT_EN not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001DF0000000010ull);
}
#else
#define CVMX_DPI_INT_EN (CVMX_ADD_IO_SEG(0x0001DF0000000010ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DPI_INT_REG CVMX_DPI_INT_REG_FUNC()
static inline uint64_t CVMX_DPI_INT_REG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF71XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_DPI_INT_REG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001DF0000000008ull);
}
#else
#define CVMX_DPI_INT_REG (CVMX_ADD_IO_SEG(0x0001DF0000000008ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DPI_NCBX_CFG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset == 0)))))
		cvmx_warn("CVMX_DPI_NCBX_CFG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001DF0000000800ull);
}
#else
#define CVMX_DPI_NCBX_CFG(offset) (CVMX_ADD_IO_SEG(0x0001DF0000000800ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DPI_NCB_CTL CVMX_DPI_NCB_CTL_FUNC()
static inline uint64_t CVMX_DPI_NCB_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_DPI_NCB_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001DF0000000028ull);
}
#else
#define CVMX_DPI_NCB_CTL (CVMX_ADD_IO_SEG(0x0001DF0000000028ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DPI_PINT_INFO CVMX_DPI_PINT_INFO_FUNC()
static inline uint64_t CVMX_DPI_PINT_INFO_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF71XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_DPI_PINT_INFO not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001DF0000000830ull);
}
#else
#define CVMX_DPI_PINT_INFO (CVMX_ADD_IO_SEG(0x0001DF0000000830ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DPI_PKT_ERR_RSP CVMX_DPI_PKT_ERR_RSP_FUNC()
static inline uint64_t CVMX_DPI_PKT_ERR_RSP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF71XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_DPI_PKT_ERR_RSP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001DF0000000078ull);
}
#else
#define CVMX_DPI_PKT_ERR_RSP (CVMX_ADD_IO_SEG(0x0001DF0000000078ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DPI_REQ_ERR_RSP CVMX_DPI_REQ_ERR_RSP_FUNC()
static inline uint64_t CVMX_DPI_REQ_ERR_RSP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF71XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_DPI_REQ_ERR_RSP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001DF0000000058ull);
}
#else
#define CVMX_DPI_REQ_ERR_RSP (CVMX_ADD_IO_SEG(0x0001DF0000000058ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DPI_REQ_ERR_RSP_EN CVMX_DPI_REQ_ERR_RSP_EN_FUNC()
static inline uint64_t CVMX_DPI_REQ_ERR_RSP_EN_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF71XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_DPI_REQ_ERR_RSP_EN not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001DF0000000068ull);
}
#else
#define CVMX_DPI_REQ_ERR_RSP_EN (CVMX_ADD_IO_SEG(0x0001DF0000000068ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DPI_REQ_ERR_RST CVMX_DPI_REQ_ERR_RST_FUNC()
static inline uint64_t CVMX_DPI_REQ_ERR_RST_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF71XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_DPI_REQ_ERR_RST not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001DF0000000060ull);
}
#else
#define CVMX_DPI_REQ_ERR_RST (CVMX_ADD_IO_SEG(0x0001DF0000000060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DPI_REQ_ERR_RST_EN CVMX_DPI_REQ_ERR_RST_EN_FUNC()
static inline uint64_t CVMX_DPI_REQ_ERR_RST_EN_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF71XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_DPI_REQ_ERR_RST_EN not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001DF0000000070ull);
}
#else
#define CVMX_DPI_REQ_ERR_RST_EN (CVMX_ADD_IO_SEG(0x0001DF0000000070ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DPI_REQ_ERR_SKIP_COMP CVMX_DPI_REQ_ERR_SKIP_COMP_FUNC()
static inline uint64_t CVMX_DPI_REQ_ERR_SKIP_COMP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF71XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_DPI_REQ_ERR_SKIP_COMP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001DF0000000838ull);
}
#else
#define CVMX_DPI_REQ_ERR_SKIP_COMP (CVMX_ADD_IO_SEG(0x0001DF0000000838ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DPI_REQ_GBL_EN CVMX_DPI_REQ_GBL_EN_FUNC()
static inline uint64_t CVMX_DPI_REQ_GBL_EN_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF71XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_DPI_REQ_GBL_EN not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001DF0000000050ull);
}
#else
#define CVMX_DPI_REQ_GBL_EN (CVMX_ADD_IO_SEG(0x0001DF0000000050ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DPI_SLI_PRTX_CFG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_DPI_SLI_PRTX_CFG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001DF0000000900ull) + ((offset) & 3) * 8;
}
#else
#define CVMX_DPI_SLI_PRTX_CFG(offset) (CVMX_ADD_IO_SEG(0x0001DF0000000900ull) + ((offset) & 3) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DPI_SLI_PRTX_ERR(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 3))
					return CVMX_ADD_IO_SEG(0x0001DF0000000920ull) + ((offset) & 3) * 8;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 3))
					return CVMX_ADD_IO_SEG(0x0001DF0000000920ull) + ((offset) & 3) * 8;
			if ((offset <= 3))
				return CVMX_ADD_IO_SEG(0x0001DF0000000920ull) + ((offset) & 3) * 8;
			break;
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:

			if (OCTEON_IS_MODEL(OCTEON_CN68XX_PASS1))
				if ((offset <= 1))
					return CVMX_ADD_IO_SEG(0x0001DF0000000928ull) + ((offset) & 1) * 8;

			if (OCTEON_IS_MODEL(OCTEON_CN68XX_PASS2))
				if ((offset <= 1))
					return CVMX_ADD_IO_SEG(0x0001DF0000000920ull) + ((offset) & 1) * 8;
			if ((offset <= 1))
				return CVMX_ADD_IO_SEG(0x0001DF0000000920ull) + ((offset) & 1) * 8;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset <= 2))
				return CVMX_ADD_IO_SEG(0x0001DF0000000920ull) + ((offset) & 3) * 8;
			break;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset <= 1))
				return CVMX_ADD_IO_SEG(0x0001DF0000000928ull) + ((offset) & 1) * 8;
			break;
	}
	cvmx_warn("CVMX_DPI_SLI_PRTX_ERR (offset = %lu) not supported on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001DF0000000920ull) + ((offset) & 3) * 8;
}
#else
static inline uint64_t CVMX_DPI_SLI_PRTX_ERR(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return CVMX_ADD_IO_SEG(0x0001DF0000000920ull) + (offset) * 8;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return CVMX_ADD_IO_SEG(0x0001DF0000000920ull) + (offset) * 8;
			return CVMX_ADD_IO_SEG(0x0001DF0000000920ull) + (offset) * 8;
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:

			if (OCTEON_IS_MODEL(OCTEON_CN68XX_PASS1))
				return CVMX_ADD_IO_SEG(0x0001DF0000000928ull) + (offset) * 8;

			if (OCTEON_IS_MODEL(OCTEON_CN68XX_PASS2))
				return CVMX_ADD_IO_SEG(0x0001DF0000000920ull) + (offset) * 8;
			return CVMX_ADD_IO_SEG(0x0001DF0000000920ull) + (offset) * 8;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001DF0000000920ull) + (offset) * 8;
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001DF0000000928ull) + (offset) * 8;
	}
	return CVMX_ADD_IO_SEG(0x0001DF0000000920ull) + (offset) * 8;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DPI_SLI_PRTX_ERR_INFO(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_DPI_SLI_PRTX_ERR_INFO(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001DF0000000940ull) + ((offset) & 3) * 8;
}
#else
#define CVMX_DPI_SLI_PRTX_ERR_INFO(offset) (CVMX_ADD_IO_SEG(0x0001DF0000000940ull) + ((offset) & 3) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DPI_SRIO_RX_BELLX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_DPI_SRIO_RX_BELLX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001DF0000080200ull) + ((offset) & 31) * 8;
}
#else
#define CVMX_DPI_SRIO_RX_BELLX(offset) (CVMX_ADD_IO_SEG(0x0001DF0000080200ull) + ((offset) & 31) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_DPI_SRIO_RX_BELL_SEQX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_DPI_SRIO_RX_BELL_SEQX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001DF0000080400ull) + ((offset) & 31) * 8;
}
#else
#define CVMX_DPI_SRIO_RX_BELL_SEQX(offset) (CVMX_ADD_IO_SEG(0x0001DF0000080400ull) + ((offset) & 31) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_DPI_SWA_Q_VMID CVMX_DPI_SWA_Q_VMID_FUNC()
static inline uint64_t CVMX_DPI_SWA_Q_VMID_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_DPI_SWA_Q_VMID not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001DF0000000030ull);
}
#else
#define CVMX_DPI_SWA_Q_VMID (CVMX_ADD_IO_SEG(0x0001DF0000000030ull))
#endif

/**
 * cvmx_dpi_bist_status
 *
 * This is the built-in self-test (BIST) status register. Each bit is the BIST result of an
 * individual memory (per bit, 0 = pass and 1 = fail).
 */
union cvmx_dpi_bist_status {
	uint64_t u64;
	struct cvmx_dpi_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_57_63               : 7;
	uint64_t bist                         : 57; /**< BIST results. Hardware sets a bit in BIST for memory that fails. */
#else
	uint64_t bist                         : 57;
	uint64_t reserved_57_63               : 7;
#endif
	} s;
	struct cvmx_dpi_bist_status_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_47_63               : 17;
	uint64_t bist                         : 47; /**< BIST Results.
                                                         HW sets a bit in BIST for for memory that fails
                                                         BIST. */
#else
	uint64_t bist                         : 47;
	uint64_t reserved_47_63               : 17;
#endif
	} cn61xx;
	struct cvmx_dpi_bist_status_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_45_63               : 19;
	uint64_t bist                         : 45; /**< BIST Results.
                                                         HW sets a bit in BIST for for memory that fails
                                                         BIST. */
#else
	uint64_t bist                         : 45;
	uint64_t reserved_45_63               : 19;
#endif
	} cn63xx;
	struct cvmx_dpi_bist_status_cn63xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_37_63               : 27;
	uint64_t bist                         : 37; /**< BIST Results.
                                                         HW sets a bit in BIST for for memory that fails
                                                         BIST. */
#else
	uint64_t bist                         : 37;
	uint64_t reserved_37_63               : 27;
#endif
	} cn63xxp1;
	struct cvmx_dpi_bist_status_cn61xx    cn66xx;
	struct cvmx_dpi_bist_status_cn63xx    cn68xx;
	struct cvmx_dpi_bist_status_cn63xx    cn68xxp1;
	struct cvmx_dpi_bist_status_cn61xx    cn70xx;
	struct cvmx_dpi_bist_status_cn61xx    cn70xxp1;
	struct cvmx_dpi_bist_status_s         cn73xx;
	struct cvmx_dpi_bist_status_s         cn78xx;
	struct cvmx_dpi_bist_status_cn78xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_51_63               : 13;
	uint64_t bist                         : 51; /**< BIST results. Hardware sets a bit in BIST for memory that fails. */
#else
	uint64_t bist                         : 51;
	uint64_t reserved_51_63               : 13;
#endif
	} cn78xxp1;
	struct cvmx_dpi_bist_status_cn61xx    cnf71xx;
	struct cvmx_dpi_bist_status_s         cnf75xx;
};
typedef union cvmx_dpi_bist_status cvmx_dpi_bist_status_t;

/**
 * cvmx_dpi_ctl
 *
 * This register provides the enable bit for the DMA and packet state machines.
 *
 */
union cvmx_dpi_ctl {
	uint64_t u64;
	struct cvmx_dpi_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t clk                          : 1;  /**< Status bit that indicates that the clks are running */
	uint64_t en                           : 1;  /**< Turns on the DMA and packet state machines. */
#else
	uint64_t en                           : 1;
	uint64_t clk                          : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_dpi_ctl_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t en                           : 1;  /**< Turns on the DMA and Packet state machines */
#else
	uint64_t en                           : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} cn61xx;
	struct cvmx_dpi_ctl_s                 cn63xx;
	struct cvmx_dpi_ctl_s                 cn63xxp1;
	struct cvmx_dpi_ctl_s                 cn66xx;
	struct cvmx_dpi_ctl_s                 cn68xx;
	struct cvmx_dpi_ctl_s                 cn68xxp1;
	struct cvmx_dpi_ctl_cn61xx            cn70xx;
	struct cvmx_dpi_ctl_cn61xx            cn70xxp1;
	struct cvmx_dpi_ctl_cn61xx            cn73xx;
	struct cvmx_dpi_ctl_cn61xx            cn78xx;
	struct cvmx_dpi_ctl_cn61xx            cn78xxp1;
	struct cvmx_dpi_ctl_cn61xx            cnf71xx;
	struct cvmx_dpi_ctl_cn61xx            cnf75xx;
};
typedef union cvmx_dpi_ctl cvmx_dpi_ctl_t;

/**
 * cvmx_dpi_dma#_counts
 *
 * These registers provide values for determining the number of instructions in the local
 * instruction FIFO.
 */
union cvmx_dpi_dmax_counts {
	uint64_t u64;
	struct cvmx_dpi_dmax_counts_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_39_63               : 25;
	uint64_t fcnt                         : 7;  /**< FIFO count. Specifies the number of words in the instruction FIFO locally cached within DPI. */
	uint64_t dbell                        : 32; /**< Doorbell. Specifies the number of available words of instructions to read. */
#else
	uint64_t dbell                        : 32;
	uint64_t fcnt                         : 7;
	uint64_t reserved_39_63               : 25;
#endif
	} s;
	struct cvmx_dpi_dmax_counts_s         cn61xx;
	struct cvmx_dpi_dmax_counts_s         cn63xx;
	struct cvmx_dpi_dmax_counts_s         cn63xxp1;
	struct cvmx_dpi_dmax_counts_s         cn66xx;
	struct cvmx_dpi_dmax_counts_s         cn68xx;
	struct cvmx_dpi_dmax_counts_s         cn68xxp1;
	struct cvmx_dpi_dmax_counts_s         cn70xx;
	struct cvmx_dpi_dmax_counts_s         cn70xxp1;
	struct cvmx_dpi_dmax_counts_s         cn73xx;
	struct cvmx_dpi_dmax_counts_s         cn78xx;
	struct cvmx_dpi_dmax_counts_s         cn78xxp1;
	struct cvmx_dpi_dmax_counts_s         cnf71xx;
	struct cvmx_dpi_dmax_counts_s         cnf75xx;
};
typedef union cvmx_dpi_dmax_counts cvmx_dpi_dmax_counts_t;

/**
 * cvmx_dpi_dma#_dbell
 *
 * This is the door bell register for the eight DMA instruction queues.
 *
 */
union cvmx_dpi_dmax_dbell {
	uint64_t u64;
	struct cvmx_dpi_dmax_dbell_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t dbell                        : 16; /**< Doorbell value. The value written to this register is added to the number of eight-byte
                                                         words to be read and processed for the low-priority DMA queue. */
#else
	uint64_t dbell                        : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_dpi_dmax_dbell_s          cn61xx;
	struct cvmx_dpi_dmax_dbell_s          cn63xx;
	struct cvmx_dpi_dmax_dbell_s          cn63xxp1;
	struct cvmx_dpi_dmax_dbell_s          cn66xx;
	struct cvmx_dpi_dmax_dbell_s          cn68xx;
	struct cvmx_dpi_dmax_dbell_s          cn68xxp1;
	struct cvmx_dpi_dmax_dbell_s          cn70xx;
	struct cvmx_dpi_dmax_dbell_s          cn70xxp1;
	struct cvmx_dpi_dmax_dbell_s          cn73xx;
	struct cvmx_dpi_dmax_dbell_s          cn78xx;
	struct cvmx_dpi_dmax_dbell_s          cn78xxp1;
	struct cvmx_dpi_dmax_dbell_s          cnf71xx;
	struct cvmx_dpi_dmax_dbell_s          cnf75xx;
};
typedef union cvmx_dpi_dmax_dbell cvmx_dpi_dmax_dbell_t;

/**
 * cvmx_dpi_dma#_err_rsp_status
 */
union cvmx_dpi_dmax_err_rsp_status {
	uint64_t u64;
	struct cvmx_dpi_dmax_err_rsp_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t status                       : 6;  /**< [STATUS] captures the ErrorResponse status of the last 6 instructions for each
                                                         instruction queue. [STATUS]<5> represents the status for first instruction in
                                                         instruction order while [STATUS]<0> represents the last or most recent
                                                         instruction. If [STATUS]<n> is set, then the Nth instruction in the given queue
                                                         experienced an ErrorResponse. Otherwise, it completed normally. */
#else
	uint64_t status                       : 6;
	uint64_t reserved_6_63                : 58;
#endif
	} s;
	struct cvmx_dpi_dmax_err_rsp_status_s cn61xx;
	struct cvmx_dpi_dmax_err_rsp_status_s cn66xx;
	struct cvmx_dpi_dmax_err_rsp_status_s cn68xx;
	struct cvmx_dpi_dmax_err_rsp_status_s cn68xxp1;
	struct cvmx_dpi_dmax_err_rsp_status_s cn70xx;
	struct cvmx_dpi_dmax_err_rsp_status_s cn70xxp1;
	struct cvmx_dpi_dmax_err_rsp_status_s cn73xx;
	struct cvmx_dpi_dmax_err_rsp_status_s cn78xx;
	struct cvmx_dpi_dmax_err_rsp_status_s cn78xxp1;
	struct cvmx_dpi_dmax_err_rsp_status_s cnf71xx;
	struct cvmx_dpi_dmax_err_rsp_status_s cnf75xx;
};
typedef union cvmx_dpi_dmax_err_rsp_status cvmx_dpi_dmax_err_rsp_status_t;

/**
 * cvmx_dpi_dma#_ibuff_saddr
 *
 * These registers provide the address to start reading instructions for the eight DMA
 * instruction queues.
 */
union cvmx_dpi_dmax_ibuff_saddr {
	uint64_t u64;
	struct cvmx_dpi_dmax_ibuff_saddr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_62_63               : 2;
	uint64_t csize                        : 14; /**< The size in 8-byte words of the DMA instruction chunk. This value should only be written
                                                         at known times in order to prevent corruption of the instruction queue. The minimum CSIZE
                                                         is 16 (one cache block). */
	uint64_t reserved_0_47                : 48;
#else
	uint64_t reserved_0_47                : 48;
	uint64_t csize                        : 14;
	uint64_t reserved_62_63               : 2;
#endif
	} s;
	struct cvmx_dpi_dmax_ibuff_saddr_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_62_63               : 2;
	uint64_t csize                        : 14; /**< The size in 8B words of the DMA Instruction Chunk.
                                                         This value should only be written at known times
                                                         in order to prevent corruption of the instruction
                                                         queue.  The minimum CSIZE is 16 (one cacheblock). */
	uint64_t reserved_41_47               : 7;
	uint64_t idle                         : 1;  /**< DMA Request Queue is IDLE */
	uint64_t reserved_36_39               : 4;
	uint64_t saddr                        : 29; /**< The 128 byte aligned starting or chunk address.
                                                         SADDR is address bit 35:7 of the starting
                                                         instructions address. When new chunks are fetched
                                                         by the HW, SADDR will be updated to reflect the
                                                         address of the current chunk.
                                                         A write to SADDR resets both the queue's doorbell
                                                         (DPI_DMAx_COUNTS[DBELL) and its tail pointer
                                                         (DPI_DMAx_NADDR[ADDR]). */
	uint64_t reserved_0_6                 : 7;
#else
	uint64_t reserved_0_6                 : 7;
	uint64_t saddr                        : 29;
	uint64_t reserved_36_39               : 4;
	uint64_t idle                         : 1;
	uint64_t reserved_41_47               : 7;
	uint64_t csize                        : 14;
	uint64_t reserved_62_63               : 2;
#endif
	} cn61xx;
	struct cvmx_dpi_dmax_ibuff_saddr_cn61xx cn63xx;
	struct cvmx_dpi_dmax_ibuff_saddr_cn61xx cn63xxp1;
	struct cvmx_dpi_dmax_ibuff_saddr_cn61xx cn66xx;
	struct cvmx_dpi_dmax_ibuff_saddr_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_62_63               : 2;
	uint64_t csize                        : 14; /**< The size in 8B words of the DMA Instruction Chunk.
                                                         This value should only be written at known times
                                                         in order to prevent corruption of the instruction
                                                         queue.  The minimum CSIZE is 16 (one cacheblock). */
	uint64_t reserved_41_47               : 7;
	uint64_t idle                         : 1;  /**< DMA Request Queue is IDLE */
	uint64_t saddr                        : 33; /**< The 128 byte aligned starting or chunk address.
                                                         SADDR is address bits 39:7 of the starting
                                                         instructions address. When new chunks are fetched
                                                         by the HW, SADDR will be updated to reflect the
                                                         address of the current chunk.
                                                         A write to SADDR resets both the queue's doorbell
                                                         (DPI_DMAx_COUNTS[DBELL) and its tail pointer
                                                         (DPI_DMAx_NADDR[ADDR]). */
	uint64_t reserved_0_6                 : 7;
#else
	uint64_t reserved_0_6                 : 7;
	uint64_t saddr                        : 33;
	uint64_t idle                         : 1;
	uint64_t reserved_41_47               : 7;
	uint64_t csize                        : 14;
	uint64_t reserved_62_63               : 2;
#endif
	} cn68xx;
	struct cvmx_dpi_dmax_ibuff_saddr_cn68xx cn68xxp1;
	struct cvmx_dpi_dmax_ibuff_saddr_cn61xx cn70xx;
	struct cvmx_dpi_dmax_ibuff_saddr_cn61xx cn70xxp1;
	struct cvmx_dpi_dmax_ibuff_saddr_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t idle                         : 1;  /**< DMA request queue is idle. When asserted, the associated request queue is idle. */
	uint64_t reserved_62_62               : 1;
	uint64_t csize                        : 14; /**< The size in 8-byte words of the DMA instruction chunk. This value should only be written
                                                         at known times in order to prevent corruption of the instruction queue. The minimum CSIZE
                                                         is 16 (one cache block). */
	uint64_t reserved_42_47               : 6;
	uint64_t saddr                        : 35; /**< Starting address. The 128-byte aligned starting or chunk address. SADDR is address bit
                                                         <41:7> of the starting instructions address. When new chunks are fetched by the hardware,
                                                         SADDR is updated to reflect the address of the current chunk. A write to SADDR resets both
                                                         the queue's doorbell (DPI_DMA()_COUNTS[DBELL) and its tail pointer
                                                         (DPI_DMA()_NADDR[ADDR]). */
	uint64_t reserved_0_6                 : 7;
#else
	uint64_t reserved_0_6                 : 7;
	uint64_t saddr                        : 35;
	uint64_t reserved_42_47               : 6;
	uint64_t csize                        : 14;
	uint64_t reserved_62_62               : 1;
	uint64_t idle                         : 1;
#endif
	} cn73xx;
	struct cvmx_dpi_dmax_ibuff_saddr_cn73xx cn78xx;
	struct cvmx_dpi_dmax_ibuff_saddr_cn73xx cn78xxp1;
	struct cvmx_dpi_dmax_ibuff_saddr_cn61xx cnf71xx;
	struct cvmx_dpi_dmax_ibuff_saddr_cn73xx cnf75xx;
};
typedef union cvmx_dpi_dmax_ibuff_saddr cvmx_dpi_dmax_ibuff_saddr_t;

/**
 * cvmx_dpi_dma#_iflight
 */
union cvmx_dpi_dmax_iflight {
	uint64_t u64;
	struct cvmx_dpi_dmax_iflight_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t cnt                          : 3;  /**< The number of instructions from a given queue that can be in flight to the DMA engines at
                                                         a time. Reset value matches the number of DMA engines. */
#else
	uint64_t cnt                          : 3;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_dpi_dmax_iflight_s        cn61xx;
	struct cvmx_dpi_dmax_iflight_s        cn66xx;
	struct cvmx_dpi_dmax_iflight_s        cn68xx;
	struct cvmx_dpi_dmax_iflight_s        cn68xxp1;
	struct cvmx_dpi_dmax_iflight_s        cn70xx;
	struct cvmx_dpi_dmax_iflight_s        cn70xxp1;
	struct cvmx_dpi_dmax_iflight_s        cn73xx;
	struct cvmx_dpi_dmax_iflight_s        cn78xx;
	struct cvmx_dpi_dmax_iflight_s        cn78xxp1;
	struct cvmx_dpi_dmax_iflight_s        cnf71xx;
	struct cvmx_dpi_dmax_iflight_s        cnf75xx;
};
typedef union cvmx_dpi_dmax_iflight cvmx_dpi_dmax_iflight_t;

/**
 * cvmx_dpi_dma#_naddr
 *
 * These registers provide the L2C addresses to read the next Ichunk data.
 *
 */
union cvmx_dpi_dmax_naddr {
	uint64_t u64;
	struct cvmx_dpi_dmax_naddr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_42_63               : 22;
	uint64_t addr                         : 42; /**< Address. Provides the next L2C address to read instructions. */
#else
	uint64_t addr                         : 42;
	uint64_t reserved_42_63               : 22;
#endif
	} s;
	struct cvmx_dpi_dmax_naddr_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t addr                         : 36; /**< The next L2C address to read DMA# instructions
                                                         from. */
#else
	uint64_t addr                         : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} cn61xx;
	struct cvmx_dpi_dmax_naddr_cn61xx     cn63xx;
	struct cvmx_dpi_dmax_naddr_cn61xx     cn63xxp1;
	struct cvmx_dpi_dmax_naddr_cn61xx     cn66xx;
	struct cvmx_dpi_dmax_naddr_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_40_63               : 24;
	uint64_t addr                         : 40; /**< The next L2C address to read DMA# instructions
                                                         from. */
#else
	uint64_t addr                         : 40;
	uint64_t reserved_40_63               : 24;
#endif
	} cn68xx;
	struct cvmx_dpi_dmax_naddr_cn68xx     cn68xxp1;
	struct cvmx_dpi_dmax_naddr_cn61xx     cn70xx;
	struct cvmx_dpi_dmax_naddr_cn61xx     cn70xxp1;
	struct cvmx_dpi_dmax_naddr_s          cn73xx;
	struct cvmx_dpi_dmax_naddr_s          cn78xx;
	struct cvmx_dpi_dmax_naddr_s          cn78xxp1;
	struct cvmx_dpi_dmax_naddr_cn61xx     cnf71xx;
	struct cvmx_dpi_dmax_naddr_s          cnf75xx;
};
typedef union cvmx_dpi_dmax_naddr cvmx_dpi_dmax_naddr_t;

/**
 * cvmx_dpi_dma#_reqbnk0
 *
 * These registers provide the current contents of the request state machine, bank 0.
 *
 */
union cvmx_dpi_dmax_reqbnk0 {
	uint64_t u64;
	struct cvmx_dpi_dmax_reqbnk0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t state                        : 64; /**< State. Provides the current contents of the request state machine. */
#else
	uint64_t state                        : 64;
#endif
	} s;
	struct cvmx_dpi_dmax_reqbnk0_s        cn61xx;
	struct cvmx_dpi_dmax_reqbnk0_s        cn63xx;
	struct cvmx_dpi_dmax_reqbnk0_s        cn63xxp1;
	struct cvmx_dpi_dmax_reqbnk0_s        cn66xx;
	struct cvmx_dpi_dmax_reqbnk0_s        cn68xx;
	struct cvmx_dpi_dmax_reqbnk0_s        cn68xxp1;
	struct cvmx_dpi_dmax_reqbnk0_s        cn70xx;
	struct cvmx_dpi_dmax_reqbnk0_s        cn70xxp1;
	struct cvmx_dpi_dmax_reqbnk0_s        cn73xx;
	struct cvmx_dpi_dmax_reqbnk0_s        cn78xx;
	struct cvmx_dpi_dmax_reqbnk0_s        cn78xxp1;
	struct cvmx_dpi_dmax_reqbnk0_s        cnf71xx;
	struct cvmx_dpi_dmax_reqbnk0_s        cnf75xx;
};
typedef union cvmx_dpi_dmax_reqbnk0 cvmx_dpi_dmax_reqbnk0_t;

/**
 * cvmx_dpi_dma#_reqbnk1
 *
 * These registers provide the current contents of the request state machine, bank 1.
 *
 */
union cvmx_dpi_dmax_reqbnk1 {
	uint64_t u64;
	struct cvmx_dpi_dmax_reqbnk1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t state                        : 64; /**< State. Provides the current contents of the request state machine. */
#else
	uint64_t state                        : 64;
#endif
	} s;
	struct cvmx_dpi_dmax_reqbnk1_s        cn61xx;
	struct cvmx_dpi_dmax_reqbnk1_s        cn63xx;
	struct cvmx_dpi_dmax_reqbnk1_s        cn63xxp1;
	struct cvmx_dpi_dmax_reqbnk1_s        cn66xx;
	struct cvmx_dpi_dmax_reqbnk1_s        cn68xx;
	struct cvmx_dpi_dmax_reqbnk1_s        cn68xxp1;
	struct cvmx_dpi_dmax_reqbnk1_s        cn70xx;
	struct cvmx_dpi_dmax_reqbnk1_s        cn70xxp1;
	struct cvmx_dpi_dmax_reqbnk1_s        cn73xx;
	struct cvmx_dpi_dmax_reqbnk1_s        cn78xx;
	struct cvmx_dpi_dmax_reqbnk1_s        cn78xxp1;
	struct cvmx_dpi_dmax_reqbnk1_s        cnf71xx;
	struct cvmx_dpi_dmax_reqbnk1_s        cnf75xx;
};
typedef union cvmx_dpi_dmax_reqbnk1 cvmx_dpi_dmax_reqbnk1_t;

/**
 * cvmx_dpi_dma#_reqq_ctl
 *
 * This register contains the control bits for transactions on the eight request queues.
 *
 */
union cvmx_dpi_dmax_reqq_ctl {
	uint64_t u64;
	struct cvmx_dpi_dmax_reqq_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t st_cmd                       : 1;  /**< When DPI issues a store full line command to the L2C that is to be cached, this field
                                                         select the type of store command to use:
                                                         0 = STF.
                                                         1 = STY. */
	uint64_t reserved_2_7                 : 6;
	uint64_t ld_cmd                       : 2;  /**< When DPI issues a load command to the L2C that is to be cached, this field select the type
                                                         of load command to use:
                                                         0x0 = LDD.
                                                         0x1 = LDI.
                                                         0x2 = LDE.
                                                         0x3 = LDY. */
#else
	uint64_t ld_cmd                       : 2;
	uint64_t reserved_2_7                 : 6;
	uint64_t st_cmd                       : 1;
	uint64_t reserved_9_63                : 55;
#endif
	} s;
	struct cvmx_dpi_dmax_reqq_ctl_s       cn73xx;
	struct cvmx_dpi_dmax_reqq_ctl_s       cn78xx;
	struct cvmx_dpi_dmax_reqq_ctl_s       cn78xxp1;
	struct cvmx_dpi_dmax_reqq_ctl_s       cnf75xx;
};
typedef union cvmx_dpi_dmax_reqq_ctl cvmx_dpi_dmax_reqq_ctl_t;

/**
 * cvmx_dpi_dma_control
 *
 * This register controls the operation of DMA input and output.
 *
 */
union cvmx_dpi_dma_control {
	uint64_t u64;
	struct cvmx_dpi_dma_control_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_62_63               : 2;
	uint64_t dici_mode                    : 1;  /**< DMA Instruction Completion Interrupt Mode
                                                         turns on mode to increment DPI_DMA_PPx_CNT
                                                         counters. */
	uint64_t pkt_en1                      : 1;  /**< Enables the 2nd packet interface.
                                                         When the packet interface is enabled, engine 4
                                                         is used for packets and is not available for DMA.
                                                         The packet interfaces must be enabled in order.
                                                         When PKT_EN1=1, then PKT_EN=1.
                                                         When PKT_EN1=1, then DMA_ENB<4>=0. */
	uint64_t ffp_dis                      : 1;  /**< Force forward progress disable. The DMA engines will compete for shared resources. If the
                                                         hardware detects that particular engines are not able to make requests to an interface,
                                                         the hardware will periodically trade-off throughput for fairness. */
	uint64_t commit_mode                  : 1;  /**< DMA engine commit mode. For diagnostic use only. */
	uint64_t pkt_hp                       : 1;  /**< High-Priority Mode for Packet Interface.
                                                         This mode has been deprecated. */
	uint64_t pkt_en                       : 1;  /**< Enables the packet interface. When the packet interface is enabled, engines 4 and 5 are
                                                         used for packets and are not available for DMA. When PKT_EN=1, then DMA_ENB<5>=0 and
                                                         DMA_ENB<4>=0. */
	uint64_t reserved_54_55               : 2;
	uint64_t dma_enb                      : 6;  /**< DMA engine enable. Enables the operation of the DMA engine. After being enabled an engine
                                                         should not be disabled while processing instructions.
                                                         When PKT_EN=1, then DMA_ENB<5>=0 and DMA_ENB<4>=0. */
	uint64_t wqecsdis                     : 1;  /**< Work queue completion status disable. See DPI_HDR_PT_WQP_E.
                                                         When [WQECSDIS] is set, DPI never writes completion status into a work queue entry. */
	uint64_t wqecsoff                     : 7;  /**< Work queue completion status byte offset. For a DPI_HDR_PT_WQP_E::STATUSCA
                                                         or DPI_HDR_PT_WQP_E::STATUSNC DPI DMA instruction, DPI writes a
                                                         non-DPI_CS_E::NOERR (i.e. nonzero) completion status byte to (big-endian
                                                         byte address) L2/DRAM address
                                                            (DPI_DMA_INSTR_HDR_S[PTR] & 0xFFFFFFFFFFFFFFF8) + [WQECSOFF]
                                                         With the reset value 0x7, DPI will write WORD0<7:0> of the WQE. */
	uint64_t zbwcsen                      : 1;  /**< Zero-byte-write completion status enable.
                                                         See DPI_HDR_PT_E::ZBC_CA and DPI_HDR_PT_E::ZBC_NC. */
	uint64_t wqecsmode                    : 2;  /**< WQE completion status mode. Relevant for DPI DMA instructions with
                                                         DPI_DMA_INSTR_HDR_S[PT]=DPI_HDR_PT_E::WQP when [WQECSDIS]=0.
                                                         0x0 = Normal behavior. DPI will not write the completion status byte for
                                                               DPI_HDR_PT_E::WQP DPI DMA instructions with DPI_CS_E::NOERR (i.e. zero)
                                                               completion status, regardless of the DPI_HDR_PT_WQP_E selection of
                                                               DPI_DMA_INSTR_HDR_S[PTR<2:0>]. DPI will write the completion
                                                               status byte for all other DPI_CS_E (i.e. nonzero) values
                                                               when DPI_DMA_INSTR_HDR_S[PTR<2:0>] is DPI_HDR_PT_WQP_E::STATUSCA
                                                               or DPI_HDR_PT_WQP_E::STATUSNC and [WQECSDIS] is clear.
                                                         0x1 = DPI will perform the completion status byte write for all
                                                               DPI_HDR_PT_E::WQP DPI DMA instructions when DPI_DMA_INSTR_HDR_S[PTR<2:0>]
                                                               is DPI_HDR_PT_WQP_E::STATUSCA or DPI_HDR_PT_WQP_E::STATUSNC
                                                               and [WQECSDIS] is clear, regardless of the DPI_CS_E completion
                                                               status value for the instruction.
                                                         0x2 = DPI will not wait for the completion status write commit before issuing
                                                               SSO work queue add.
                                                         0x3 = Both debug modes specified above (under 0x1 and 0x2) are enabled. */
	uint64_t reserved_35_36               : 2;
	uint64_t ncb_tag                      : 1;  /**< NCB tag enable. It allows DMA Read/Write transactions over NCB to be mapped to
                                                         individual request queues by using tags. This enables more parallelism, giving a
                                                         performance boost. */
	uint64_t b0_lend                      : 1;  /**< Endian-ness for DPI_HDR_PT_E::ZBC_CA and DPI_HDR_PT_E::ZBC_NC DPI DMA
                                                         instructions. When clear, DPI_DMA_INSTR_HDR_S[PTR] is big-endian for
                                                         these instructions. When set, DPI_DMA_INSTR_HDR_S[PTR] is little-endian
                                                         for these instructions. */
	uint64_t reserved_20_32               : 13;
	uint64_t o_add1                       : 1;  /**< Add one.
                                                         0 = The number of bytes in the DMA transfer is added to SLI_DMA()_CNT.
                                                         1 = Add 1 to the SLI_DMA()_CNT DMA counters. */
	uint64_t o_ro                         : 1;  /**< Relaxed ordering mode for DMA transactions */
	uint64_t o_ns                         : 1;  /**< No snoop. */
	uint64_t o_es                         : 2;  /**< Endian swap mode for DMA.
                                                         See SLI_ENDIANSWAP_E. */
	uint64_t o_mode                       : 1;  /**< Select PCI_POINTER mode.
                                                         0 = DPTR format 1 is used. Use register values for address; use pointer values for ES, NS,
                                                         RO.
                                                         1 = DPTR format 0 is used. Use pointer values for address; use register values for ES, NS,
                                                         RO. */
	uint64_t reserved_0_13                : 14;
#else
	uint64_t reserved_0_13                : 14;
	uint64_t o_mode                       : 1;
	uint64_t o_es                         : 2;
	uint64_t o_ns                         : 1;
	uint64_t o_ro                         : 1;
	uint64_t o_add1                       : 1;
	uint64_t reserved_20_32               : 13;
	uint64_t b0_lend                      : 1;
	uint64_t ncb_tag                      : 1;
	uint64_t reserved_35_36               : 2;
	uint64_t wqecsmode                    : 2;
	uint64_t zbwcsen                      : 1;
	uint64_t wqecsoff                     : 7;
	uint64_t wqecsdis                     : 1;
	uint64_t dma_enb                      : 6;
	uint64_t reserved_54_55               : 2;
	uint64_t pkt_en                       : 1;
	uint64_t pkt_hp                       : 1;
	uint64_t commit_mode                  : 1;
	uint64_t ffp_dis                      : 1;
	uint64_t pkt_en1                      : 1;
	uint64_t dici_mode                    : 1;
	uint64_t reserved_62_63               : 2;
#endif
	} s;
	struct cvmx_dpi_dma_control_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_62_63               : 2;
	uint64_t dici_mode                    : 1;  /**< DMA Instruction Completion Interrupt Mode
                                                         turns on mode to increment DPI_DMA_PPx_CNT
                                                         counters. */
	uint64_t pkt_en1                      : 1;  /**< Enables the 2nd packet interface.
                                                         When the packet interface is enabled, engine 4
                                                         is used for packets and is not available for DMA.
                                                         The packet interfaces must be enabled in order.
                                                         When PKT_EN1=1, then PKT_EN=1.
                                                         When PKT_EN1=1, then DMA_ENB<4>=0. */
	uint64_t ffp_dis                      : 1;  /**< Force forward progress disable
                                                         The DMA engines will compete for shared resources.
                                                         If the HW detects that particular engines are not
                                                         able to make requests to an interface, the HW
                                                         will periodically trade-off throughput for
                                                         fairness. */
	uint64_t commit_mode                  : 1;  /**< DMA Engine Commit Mode

                                                         When COMMIT_MODE=1, DPI considers an instruction
                                                         complete when the HW internally generates the
                                                         final write for the current instruction.

                                                         When COMMIT_MODE=0, DPI additionally waits for
                                                         the final write to reach the interface coherency
                                                         point to declare the instructions complete.

                                                         Please note: when COMMIT_MODE == 1, DPI may not
                                                         follow the HRM ordering rules.

                                                         DPI hardware performance may be better with
                                                         COMMIT_MODE == 1 than with COMMIT_MODE == 0 due
                                                         to the relaxed ordering rules.

                                                         If the HRM ordering rules are required, set
                                                         COMMIT_MODE == 0. */
	uint64_t pkt_hp                       : 1;  /**< High-Priority Mode for Packet Interface.
                                                         This mode has been deprecated. */
	uint64_t pkt_en                       : 1;  /**< Enables 1st the packet interface.
                                                         When the packet interface is enabled, engine 5
                                                         is used for packets and is not available for DMA.
                                                         When PKT_EN=1, then DMA_ENB<5>=0.
                                                         When PKT_EN1=1, then PKT_EN=1. */
	uint64_t reserved_54_55               : 2;
	uint64_t dma_enb                      : 6;  /**< DMA engine enable. Enables the operation of the
                                                         DMA engine. After being enabled an engine should
                                                         not be disabled while processing instructions.
                                                         When PKT_EN=1,  then DMA_ENB<5>=0.
                                                         When PKT_EN1=1, then DMA_ENB<4>=0. */
	uint64_t reserved_34_47               : 14;
	uint64_t b0_lend                      : 1;  /**< When set '1' and the DPI is in the mode to write
                                                         0 to L2C memory when a DMA is done, the address
                                                         to be written to will be treated as a Little
                                                         Endian address. */
	uint64_t dwb_denb                     : 1;  /**< When set '1', DPI will send a value in the DWB
                                                         field for a free page operation for the memory
                                                         that contained the data. */
	uint64_t dwb_ichk                     : 9;  /**< When Instruction Chunks for DMA operations are
                                                         freed this value is used for the DWB field of the
                                                         operation. */
	uint64_t fpa_que                      : 3;  /**< The FPA queue that the instruction-chunk page will
                                                         be returned to when used. */
	uint64_t o_add1                       : 1;  /**< When set '1' 1 will be added to the SLI_DMAX_CNT
                                                         DMA counters, if '0' then the number of bytes
                                                         in the dma transfer will be added to the
                                                         SLI_DMAX_CNT count register. */
	uint64_t o_ro                         : 1;  /**< Relaxed Ordering Mode for DMA. */
	uint64_t o_ns                         : 1;  /**< Nosnoop For DMA. */
	uint64_t o_es                         : 2;  /**< Endian Swap Mode for DMA. */
	uint64_t o_mode                       : 1;  /**< Select PCI_POINTER MODE to be used.
                                                         0=DPTR format 1 is used
                                                           use register values for address and pointer
                                                           values for ES, NS, RO
                                                         1=DPTR format 0 is used
                                                           use pointer values for address and register
                                                           values for ES, NS, RO */
	uint64_t reserved_0_13                : 14;
#else
	uint64_t reserved_0_13                : 14;
	uint64_t o_mode                       : 1;
	uint64_t o_es                         : 2;
	uint64_t o_ns                         : 1;
	uint64_t o_ro                         : 1;
	uint64_t o_add1                       : 1;
	uint64_t fpa_que                      : 3;
	uint64_t dwb_ichk                     : 9;
	uint64_t dwb_denb                     : 1;
	uint64_t b0_lend                      : 1;
	uint64_t reserved_34_47               : 14;
	uint64_t dma_enb                      : 6;
	uint64_t reserved_54_55               : 2;
	uint64_t pkt_en                       : 1;
	uint64_t pkt_hp                       : 1;
	uint64_t commit_mode                  : 1;
	uint64_t ffp_dis                      : 1;
	uint64_t pkt_en1                      : 1;
	uint64_t dici_mode                    : 1;
	uint64_t reserved_62_63               : 2;
#endif
	} cn61xx;
	struct cvmx_dpi_dma_control_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_61_63               : 3;
	uint64_t pkt_en1                      : 1;  /**< Enables the 2nd packet interface.
                                                         When the packet interface is enabled, engine 4
                                                         is used for packets and is not available for DMA.
                                                         The packet interfaces must be enabled in order.
                                                         When PKT_EN1=1, then PKT_EN=1.
                                                         When PKT_EN1=1, then DMA_ENB<4>=0. */
	uint64_t ffp_dis                      : 1;  /**< Force forward progress disable
                                                         The DMA engines will compete for shared resources.
                                                         If the HW detects that particular engines are not
                                                         able to make requests to an interface, the HW
                                                         will periodically trade-off throughput for
                                                         fairness. */
	uint64_t commit_mode                  : 1;  /**< DMA Engine Commit Mode

                                                         When COMMIT_MODE=0, DPI considers an instruction
                                                         complete when the HW internally generates the
                                                         final write for the current instruction.

                                                         When COMMIT_MODE=1, DPI additionally waits for
                                                         the final write to reach the interface coherency
                                                         point to declare the instructions complete.

                                                         Please note: when COMMIT_MODE == 0, DPI may not
                                                         follow the HRM ordering rules.

                                                         DPI hardware performance may be better with
                                                         COMMIT_MODE == 0 than with COMMIT_MODE == 1 due
                                                         to the relaxed ordering rules.

                                                         If the HRM ordering rules are required, set
                                                         COMMIT_MODE == 1. */
	uint64_t pkt_hp                       : 1;  /**< High-Priority Mode for Packet Interface.
                                                         This mode has been deprecated. */
	uint64_t pkt_en                       : 1;  /**< Enables 1st the packet interface.
                                                         When the packet interface is enabled, engine 5
                                                         is used for packets and is not available for DMA.
                                                         When PKT_EN=1, then DMA_ENB<5>=0.
                                                         When PKT_EN1=1, then PKT_EN=1. */
	uint64_t reserved_54_55               : 2;
	uint64_t dma_enb                      : 6;  /**< DMA engine enable. Enables the operation of the
                                                         DMA engine. After being enabled an engine should
                                                         not be disabled while processing instructions.
                                                         When PKT_EN=1,  then DMA_ENB<5>=0.
                                                         When PKT_EN1=1, then DMA_ENB<4>=0. */
	uint64_t reserved_34_47               : 14;
	uint64_t b0_lend                      : 1;  /**< When set '1' and the DPI is in the mode to write
                                                         0 to L2C memory when a DMA is done, the address
                                                         to be written to will be treated as a Little
                                                         Endian address. */
	uint64_t dwb_denb                     : 1;  /**< When set '1', DPI will send a value in the DWB
                                                         field for a free page operation for the memory
                                                         that contained the data. */
	uint64_t dwb_ichk                     : 9;  /**< When Instruction Chunks for DMA operations are
                                                         freed this value is used for the DWB field of the
                                                         operation. */
	uint64_t fpa_que                      : 3;  /**< The FPA queue that the instruction-chunk page will
                                                         be returned to when used. */
	uint64_t o_add1                       : 1;  /**< When set '1' 1 will be added to the DMA counters,
                                                         if '0' then the number of bytes in the dma
                                                         transfer will be added to the count register. */
	uint64_t o_ro                         : 1;  /**< Relaxed Ordering Mode for DMA. */
	uint64_t o_ns                         : 1;  /**< Nosnoop For DMA. */
	uint64_t o_es                         : 2;  /**< Endian Swap Mode for DMA. */
	uint64_t o_mode                       : 1;  /**< Select PCI_POINTER MODE to be used.
                                                         0=DPTR format 1 is used
                                                           use register values for address and pointer
                                                           values for ES, NS, RO
                                                         1=DPTR format 0 is used
                                                           use pointer values for address and register
                                                           values for ES, NS, RO */
	uint64_t reserved_0_13                : 14;
#else
	uint64_t reserved_0_13                : 14;
	uint64_t o_mode                       : 1;
	uint64_t o_es                         : 2;
	uint64_t o_ns                         : 1;
	uint64_t o_ro                         : 1;
	uint64_t o_add1                       : 1;
	uint64_t fpa_que                      : 3;
	uint64_t dwb_ichk                     : 9;
	uint64_t dwb_denb                     : 1;
	uint64_t b0_lend                      : 1;
	uint64_t reserved_34_47               : 14;
	uint64_t dma_enb                      : 6;
	uint64_t reserved_54_55               : 2;
	uint64_t pkt_en                       : 1;
	uint64_t pkt_hp                       : 1;
	uint64_t commit_mode                  : 1;
	uint64_t ffp_dis                      : 1;
	uint64_t pkt_en1                      : 1;
	uint64_t reserved_61_63               : 3;
#endif
	} cn63xx;
	struct cvmx_dpi_dma_control_cn63xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_59_63               : 5;
	uint64_t commit_mode                  : 1;  /**< DMA Engine Commit Mode

                                                         When COMMIT_MODE=1, DPI considers an instruction
                                                         complete when the HW internally generates the
                                                         final write for the current instruction.

                                                         When COMMIT_MODE=0, DPI additionally waits for
                                                         the final write to reach the interface coherency
                                                         point to declare the instructions complete.

                                                         Please note: when COMMIT_MODE == 1, DPI may not
                                                         follow the HRM ordering rules.

                                                         DPI hardware performance may be better with
                                                         COMMIT_MODE == 1 than with COMMIT_MODE == 0 due
                                                         to the relaxed ordering rules.

                                                         If the HRM ordering rules are required, set
                                                         COMMIT_MODE == 0. */
	uint64_t pkt_hp                       : 1;  /**< High-Priority Mode for Packet Interface.
                                                         Engine 5 will be serviced more frequently to
                                                         deliver more bandwidth to packet interface.
                                                         When PKT_EN=0, then PKT_HP=0. */
	uint64_t pkt_en                       : 1;  /**< Enables the packet interface.
                                                         When the packet interface is enabled, engine 5
                                                         is used for packets and is not available for DMA.
                                                         When PKT_EN=1, then DMA_ENB<5>=0.
                                                         When PKT_EN=0, then PKT_HP=0. */
	uint64_t reserved_54_55               : 2;
	uint64_t dma_enb                      : 6;  /**< DMA engine enable. Enables the operation of the
                                                         DMA engine. After being enabled an engine should
                                                         not be disabled while processing instructions.
                                                         When PKT_EN=1, then DMA_ENB<5>=0. */
	uint64_t reserved_34_47               : 14;
	uint64_t b0_lend                      : 1;  /**< When set '1' and the DPI is in the mode to write
                                                         0 to L2C memory when a DMA is done, the address
                                                         to be written to will be treated as a Little
                                                         Endian address. */
	uint64_t dwb_denb                     : 1;  /**< When set '1', DPI will send a value in the DWB
                                                         field for a free page operation for the memory
                                                         that contained the data. */
	uint64_t dwb_ichk                     : 9;  /**< When Instruction Chunks for DMA operations are
                                                         freed this value is used for the DWB field of the
                                                         operation. */
	uint64_t fpa_que                      : 3;  /**< The FPA queue that the instruction-chunk page will
                                                         be returned to when used. */
	uint64_t o_add1                       : 1;  /**< When set '1' 1 will be added to the DMA counters,
                                                         if '0' then the number of bytes in the dma
                                                         transfer will be added to the count register. */
	uint64_t o_ro                         : 1;  /**< Relaxed Ordering Mode for DMA. */
	uint64_t o_ns                         : 1;  /**< Nosnoop For DMA. */
	uint64_t o_es                         : 2;  /**< Endian Swap Mode for DMA. */
	uint64_t o_mode                       : 1;  /**< Select PCI_POINTER MODE to be used.
                                                         0=DPTR format 1 is used
                                                           use register values for address and pointer
                                                           values for ES, NS, RO
                                                         1=DPTR format 0 is used
                                                           use pointer values for address and register
                                                           values for ES, NS, RO */
	uint64_t reserved_0_13                : 14;
#else
	uint64_t reserved_0_13                : 14;
	uint64_t o_mode                       : 1;
	uint64_t o_es                         : 2;
	uint64_t o_ns                         : 1;
	uint64_t o_ro                         : 1;
	uint64_t o_add1                       : 1;
	uint64_t fpa_que                      : 3;
	uint64_t dwb_ichk                     : 9;
	uint64_t dwb_denb                     : 1;
	uint64_t b0_lend                      : 1;
	uint64_t reserved_34_47               : 14;
	uint64_t dma_enb                      : 6;
	uint64_t reserved_54_55               : 2;
	uint64_t pkt_en                       : 1;
	uint64_t pkt_hp                       : 1;
	uint64_t commit_mode                  : 1;
	uint64_t reserved_59_63               : 5;
#endif
	} cn63xxp1;
	struct cvmx_dpi_dma_control_cn63xx    cn66xx;
	struct cvmx_dpi_dma_control_cn61xx    cn68xx;
	struct cvmx_dpi_dma_control_cn63xx    cn68xxp1;
	struct cvmx_dpi_dma_control_cn61xx    cn70xx;
	struct cvmx_dpi_dma_control_cn61xx    cn70xxp1;
	struct cvmx_dpi_dma_control_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t ffp_dis                      : 1;  /**< Force forward progress disable. The DMA engines will compete for shared resources. If the
                                                         hardware detects that particular engines are not able to make requests to an interface,
                                                         the hardware will periodically trade-off throughput for fairness. */
	uint64_t commit_mode                  : 1;  /**< DMA engine commit mode. For diagnostic use only. */
	uint64_t reserved_57_57               : 1;
	uint64_t pkt_en                       : 1;  /**< Enables the packet interface. When the packet interface is enabled, engines 4 and 5 are
                                                         used for packets and are not available for DMA. When PKT_EN=1, then DMA_ENB<5>=0 and
                                                         DMA_ENB<4>=0. */
	uint64_t reserved_54_55               : 2;
	uint64_t dma_enb                      : 6;  /**< DMA engine enable. Enables the operation of the DMA engine. After being enabled an engine
                                                         should not be disabled while processing instructions.
                                                         When PKT_EN=1, then DMA_ENB<5>=0 and DMA_ENB<4>=0. */
	uint64_t wqecsdis                     : 1;  /**< Work queue completion status disable. See DPI_HDR_PT_WQP_E.
                                                         When [WQECSDIS] is set, DPI never writes completion status into a work queue entry. */
	uint64_t wqecsoff                     : 7;  /**< Work queue completion status byte offset. For a DPI_HDR_PT_WQP_E::STATUSCA
                                                         or DPI_HDR_PT_WQP_E::STATUSNC DPI DMA instruction, DPI writes a
                                                         non-DPI_CS_E::NOERR (i.e. nonzero) completion status byte to (big-endian
                                                         byte address) L2/DRAM address
                                                            (DPI_DMA_INSTR_HDR_S[PTR] & 0xFFFFFFFFFFFFFFF8) + [WQECSOFF]
                                                         With the reset value 0x7, DPI will write WORD0<7:0> of the WQE. */
	uint64_t zbwcsen                      : 1;  /**< Zero-byte-write completion status enable.
                                                         See DPI_HDR_PT_E::ZBC_CA and DPI_HDR_PT_E::ZBC_NC. */
	uint64_t wqecsmode                    : 2;  /**< WQE completion status mode. Relevant for DPI DMA instructions with
                                                         DPI_DMA_INSTR_HDR_S[PT]=DPI_HDR_PT_E::WQP when [WQECSDIS]=0.
                                                         0x0 = Normal behavior. DPI will not write the completion status byte for
                                                               DPI_HDR_PT_E::WQP DPI DMA instructions with DPI_CS_E::NOERR (i.e. zero)
                                                               completion status, regardless of the DPI_HDR_PT_WQP_E selection of
                                                               DPI_DMA_INSTR_HDR_S[PTR<2:0>]. DPI will write the completion
                                                               status byte for all other DPI_CS_E (i.e. nonzero) values
                                                               when DPI_DMA_INSTR_HDR_S[PTR<2:0>] is DPI_HDR_PT_WQP_E::STATUSCA
                                                               or DPI_HDR_PT_WQP_E::STATUSNC and [WQECSDIS] is clear.
                                                         0x1 = DPI will perform the completion status byte write for all
                                                               DPI_HDR_PT_E::WQP DPI DMA instructions when DPI_DMA_INSTR_HDR_S[PTR<2:0>]
                                                               is DPI_HDR_PT_WQP_E::STATUSCA or DPI_HDR_PT_WQP_E::STATUSNC
                                                               and [WQECSDIS] is clear, regardless of the DPI_CS_E completion
                                                               status value for the instruction.
                                                         0x2 = DPI will not wait for the completion status write commit before issuing
                                                               SSO work queue add.
                                                         0x3 = Both debug modes specified above (under 0x1 and 0x2) are enabled. */
	uint64_t reserved_35_36               : 2;
	uint64_t ncb_tag                      : 1;  /**< NCB tag enable. It allows DMA Read/Write transactions over NCB to be mapped to
                                                         individual request queues by using tags. This enables more parallelism, giving a
                                                         performance boost. */
	uint64_t b0_lend                      : 1;  /**< Endian-ness for DPI_HDR_PT_E::ZBC_CA and DPI_HDR_PT_E::ZBC_NC DPI DMA
                                                         instructions. When clear, DPI_DMA_INSTR_HDR_S[PTR] is big-endian for
                                                         these instructions. When set, DPI_DMA_INSTR_HDR_S[PTR] is little-endian
                                                         for these instructions. */
	uint64_t ldwb                         : 1;  /**< Load don't write back. When set, the hardware is able to issue LDWB commands for pointers
                                                         that are being freed. As a result, the line will not be written back when replaced.
                                                         When clear, the hardware issues regular load commands to the cache which cause the
                                                         line to be written back before being replaced. */
	uint64_t aura_ichk                    : 12; /**< AURA instruction chunk. The AURA that the instruction chunk for DMA operations page will
                                                         be returned to when freed. */
	uint64_t o_add1                       : 1;  /**< Add one.
                                                         0 = The number of bytes in the DMA transfer is added to SLI_DMA()_CNT.
                                                         1 = Add 1 to the SLI_DMA()_CNT DMA counters. */
	uint64_t o_ro                         : 1;  /**< Relaxed ordering mode for DMA transactions */
	uint64_t o_ns                         : 1;  /**< No snoop. */
	uint64_t o_es                         : 2;  /**< Endian swap mode for DMA.
                                                         See SLI_ENDIANSWAP_E. */
	uint64_t o_mode                       : 1;  /**< Select PCI_POINTER mode.
                                                         0 = DPTR format 1 is used. Use register values for address; use pointer values for ES, NS,
                                                         RO.
                                                         1 = DPTR format 0 is used. Use pointer values for address; use register values for ES, NS,
                                                         RO. */
	uint64_t reserved_0_13                : 14;
#else
	uint64_t reserved_0_13                : 14;
	uint64_t o_mode                       : 1;
	uint64_t o_es                         : 2;
	uint64_t o_ns                         : 1;
	uint64_t o_ro                         : 1;
	uint64_t o_add1                       : 1;
	uint64_t aura_ichk                    : 12;
	uint64_t ldwb                         : 1;
	uint64_t b0_lend                      : 1;
	uint64_t ncb_tag                      : 1;
	uint64_t reserved_35_36               : 2;
	uint64_t wqecsmode                    : 2;
	uint64_t zbwcsen                      : 1;
	uint64_t wqecsoff                     : 7;
	uint64_t wqecsdis                     : 1;
	uint64_t dma_enb                      : 6;
	uint64_t reserved_54_55               : 2;
	uint64_t pkt_en                       : 1;
	uint64_t reserved_57_57               : 1;
	uint64_t commit_mode                  : 1;
	uint64_t ffp_dis                      : 1;
	uint64_t reserved_60_63               : 4;
#endif
	} cn73xx;
	struct cvmx_dpi_dma_control_cn73xx    cn78xx;
	struct cvmx_dpi_dma_control_cn73xx    cn78xxp1;
	struct cvmx_dpi_dma_control_cn61xx    cnf71xx;
	struct cvmx_dpi_dma_control_cn73xx    cnf75xx;
};
typedef union cvmx_dpi_dma_control cvmx_dpi_dma_control_t;

/**
 * cvmx_dpi_dma_eng#_en
 *
 * These registers provide control for the DMA engines.
 *
 */
union cvmx_dpi_dma_engx_en {
	uint64_t u64;
	struct cvmx_dpi_dma_engx_en_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_39_63               : 25;
	uint64_t eng_molr                     : 7;  /**< Per-engine maximum outstanding read request. 0x0 means no maximum per engine
                                                         (although it will be still limited by the per port MOLR value.) */
	uint64_t reserved_8_31                : 24;
	uint64_t qen                          : 8;  /**< Instruction queue enable. This field specifies which of eight logical instruction queues
                                                         can be serviced by the DMA engine.
                                                           0 = Prevent service.
                                                           1 = Allow service.
                                                         Setting QEN = 0x0 effectively disables the engine. When DPI_DMA_CONTROL[PKT_EN] = 1, then
                                                         DPI_DMA_ENG4/5_EN [QEN] must be zero. */
#else
	uint64_t qen                          : 8;
	uint64_t reserved_8_31                : 24;
	uint64_t eng_molr                     : 7;
	uint64_t reserved_39_63               : 25;
#endif
	} s;
	struct cvmx_dpi_dma_engx_en_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t qen                          : 8;  /**< Controls which logical instruction queues can be
                                                         serviced by the DMA engine. Setting QEN==0
                                                         effectively disables the engine.
                                                         When DPI_DMA_CONTROL[PKT_EN] = 1, then
                                                         DPI_DMA_ENG5_EN[QEN] must be zero.
                                                         When DPI_DMA_CONTROL[PKT_EN1] = 1, then
                                                         DPI_DMA_ENG4_EN[QEN] must be zero. */
#else
	uint64_t qen                          : 8;
	uint64_t reserved_8_63                : 56;
#endif
	} cn61xx;
	struct cvmx_dpi_dma_engx_en_cn61xx    cn63xx;
	struct cvmx_dpi_dma_engx_en_cn61xx    cn63xxp1;
	struct cvmx_dpi_dma_engx_en_cn61xx    cn66xx;
	struct cvmx_dpi_dma_engx_en_cn61xx    cn68xx;
	struct cvmx_dpi_dma_engx_en_cn61xx    cn68xxp1;
	struct cvmx_dpi_dma_engx_en_cn61xx    cn70xx;
	struct cvmx_dpi_dma_engx_en_cn61xx    cn70xxp1;
	struct cvmx_dpi_dma_engx_en_s         cn73xx;
	struct cvmx_dpi_dma_engx_en_s         cn78xx;
	struct cvmx_dpi_dma_engx_en_s         cn78xxp1;
	struct cvmx_dpi_dma_engx_en_cn61xx    cnf71xx;
	struct cvmx_dpi_dma_engx_en_s         cnf75xx;
};
typedef union cvmx_dpi_dma_engx_en cvmx_dpi_dma_engx_en_t;

/**
 * cvmx_dpi_dma_pp#_cnt
 *
 * DPI_DMA_PP[0..3]_CNT  = DMA per PP Instr Done Counter
 * When DMA Instruction Completion Interrupt Mode DPI_DMA_CONTROL.DICI_MODE is enabled, every dma
 * instruction
 * that has the WQP=0 and a PTR value of 1..4 will incremrement DPI_DMA_PPx_CNT value-1 counter.
 * Instructions with WQP=0 and PTR values higher then 0x3F will still send a zero byte write.
 * Hardware reserves that values 5..63 for future use and will treat them as a PTR of 0 and do
 * nothing.
 */
union cvmx_dpi_dma_ppx_cnt {
	uint64_t u64;
	struct cvmx_dpi_dma_ppx_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t cnt                          : 16; /**< DPI DMA per-core instruction completion counter. DPI can increment a counter upon
                                                         completion of a DPI DMA instruction. DPI subtracts the value written
                                                         from [CNT] on a software write. A nonzero [CNT] asserts the corresponding
                                                         DPI_DMA_PP_INT bit and a 0->1 [CNT] transition throws the corresponding
                                                         DPI_INTSN_E::DPI_DMA_PP()_INT.
                                                         DPI increments the [CNT] selected by DPI_DMA_INSTR_HDR_S[PTR] by one after completing
                                                         a DPI_DMA_INSTR_HDR_S[PT]=DPI_HDR_PT_E::CNT DPI DMA instruction. */
#else
	uint64_t cnt                          : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_dpi_dma_ppx_cnt_s         cn61xx;
	struct cvmx_dpi_dma_ppx_cnt_s         cn68xx;
	struct cvmx_dpi_dma_ppx_cnt_s         cn70xx;
	struct cvmx_dpi_dma_ppx_cnt_s         cn70xxp1;
	struct cvmx_dpi_dma_ppx_cnt_s         cn73xx;
	struct cvmx_dpi_dma_ppx_cnt_s         cn78xx;
	struct cvmx_dpi_dma_ppx_cnt_s         cn78xxp1;
	struct cvmx_dpi_dma_ppx_cnt_s         cnf71xx;
	struct cvmx_dpi_dma_ppx_cnt_s         cnf75xx;
};
typedef union cvmx_dpi_dma_ppx_cnt cvmx_dpi_dma_ppx_cnt_t;

/**
 * cvmx_dpi_dma_pp_int
 */
union cvmx_dpi_dma_pp_int {
	uint64_t u64;
	struct cvmx_dpi_dma_pp_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t complete                     : 48; /**< DPI DMA per-core instruction completion interrupt. See DPI_DMA_PP()_CNT
                                                         and DPI_INTSN_E::DPI_DMA_PP()_INT. */
#else
	uint64_t complete                     : 48;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_dpi_dma_pp_int_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t complete                     : 16; /**< DPI DMA per-core instruction completion interrupt. See DPI_DMA_PP()_CNT
                                                         and DPI_INTSN_E::DPI_DMA_PP()_INT. */
#else
	uint64_t complete                     : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} cn73xx;
	struct cvmx_dpi_dma_pp_int_s          cn78xx;
	struct cvmx_dpi_dma_pp_int_s          cn78xxp1;
	struct cvmx_dpi_dma_pp_int_cn73xx     cnf75xx;
};
typedef union cvmx_dpi_dma_pp_int cvmx_dpi_dma_pp_int_t;

/**
 * cvmx_dpi_ecc_ctl
 *
 * This register allows inserting ECC errors for testing.
 *
 */
union cvmx_dpi_ecc_ctl {
	uint64_t u64;
	struct cvmx_dpi_ecc_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_33_63               : 31;
	uint64_t ram_cdis                     : 1;  /**< RDB RAM ECC correction disable. Each bit corresponds to a different RAM. */
	uint64_t reserved_17_31               : 15;
	uint64_t ram_flip1                    : 1;  /**< Flip syndrome bits on write. Flip syndrome bits <1> on writes to the corresponding RDB RAM
                                                         to test single-bit or double-bit error handling. */
	uint64_t reserved_1_15                : 15;
	uint64_t ram_flip0                    : 1;  /**< Flip syndrome bits on write. Flip syndrome bits <0> on writes to the corresponding RDB RAM
                                                         to test single-bit or double-bit error handling. */
#else
	uint64_t ram_flip0                    : 1;
	uint64_t reserved_1_15                : 15;
	uint64_t ram_flip1                    : 1;
	uint64_t reserved_17_31               : 15;
	uint64_t ram_cdis                     : 1;
	uint64_t reserved_33_63               : 31;
#endif
	} s;
	struct cvmx_dpi_ecc_ctl_s             cn73xx;
	struct cvmx_dpi_ecc_ctl_s             cn78xx;
	struct cvmx_dpi_ecc_ctl_s             cn78xxp1;
	struct cvmx_dpi_ecc_ctl_s             cnf75xx;
};
typedef union cvmx_dpi_ecc_ctl cvmx_dpi_ecc_ctl_t;

/**
 * cvmx_dpi_ecc_int
 *
 * This register contains ECC error interrupt summary bits.
 *
 */
union cvmx_dpi_ecc_int {
	uint64_t u64;
	struct cvmx_dpi_ecc_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_47_63               : 17;
	uint64_t ram_sbe                      : 15; /**< Set when a single-bit error is detected in the corresponding RAM. Throws
                                                         DPI_INTSN_E::DPI_ERR_RAM_SBE(). */
	uint64_t reserved_15_31               : 17;
	uint64_t ram_dbe                      : 15; /**< Set when a double-bit error is detected in the corresponding RAM. Throws
                                                         DPI_INTSN_E::DPI_ERR_RAM_DBE(). */
#else
	uint64_t ram_dbe                      : 15;
	uint64_t reserved_15_31               : 17;
	uint64_t ram_sbe                      : 15;
	uint64_t reserved_47_63               : 17;
#endif
	} s;
	struct cvmx_dpi_ecc_int_s             cn73xx;
	struct cvmx_dpi_ecc_int_s             cn78xx;
	struct cvmx_dpi_ecc_int_s             cn78xxp1;
	struct cvmx_dpi_ecc_int_s             cnf75xx;
};
typedef union cvmx_dpi_ecc_int cvmx_dpi_ecc_int_t;

/**
 * cvmx_dpi_eng#_buf
 *
 * Notes:
 * The total amount of storage allocated to the 6 DPI DMA engines (via DPI_ENG*_BUF[BLKS]) must not exceed 8KB.
 *
 */
union cvmx_dpi_engx_buf {
	uint64_t u64;
	struct cvmx_dpi_engx_buf_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_38_63               : 26;
	uint64_t compblks                     : 6;  /**< Computed engine block size */
	uint64_t reserved_10_31               : 22;
	uint64_t base                         : 6;  /**< The base address in 512-byte blocks of the DMA engine FIFO. */
	uint64_t blks                         : 4;  /**< The size of the DMA engine FIFO. The sum of the allocated FIFOs across all six
                                                         DPI_ENG()_BUF[BLKS] registers must not exceed the overall RDB memory size of
                                                         32 KB.
                                                         0x0 = Engine disabled.
                                                         0x1 = 0.5 KB FIFO.
                                                         0x2 = 1.0 KB FIFO.
                                                         0x3 = 1.5 KB FIFO.
                                                         0x4 = 2.0 KB FIFO.
                                                         0x5 = 2.5 KB FIFO.
                                                         0x6 = 3.0 KB FIFO.
                                                         0x7 = 3.5 KB FIFO.
                                                         0x8 = 4.0 KB FIFO.
                                                         0x9 = 6.0 KB FIFO.
                                                         0xA = 8.0 KB FIFO.
                                                         0xB = 16.0 KB FIFO. */
#else
	uint64_t blks                         : 4;
	uint64_t base                         : 6;
	uint64_t reserved_10_31               : 22;
	uint64_t compblks                     : 6;
	uint64_t reserved_38_63               : 26;
#endif
	} s;
	struct cvmx_dpi_engx_buf_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_37_63               : 27;
	uint64_t compblks                     : 5;  /**< Computed engine block size */
	uint64_t reserved_9_31                : 23;
	uint64_t base                         : 5;  /**< The base address in 512B blocks of the engine fifo */
	uint64_t blks                         : 4;  /**< The size of the engine fifo
                                                         Legal values are 0-10.
                                                         0  = Engine is disabled
                                                         1  = 0.5KB buffer
                                                         2  = 1.0KB buffer
                                                         3  = 1.5KB buffer
                                                         4  = 2.0KB buffer
                                                         5  = 2.5KB buffer
                                                         6  = 3.0KB buffer
                                                         7  = 3.5KB buffer
                                                         8  = 4.0KB buffer
                                                         9  = 6.0KB buffer
                                                         10 = 8.0KB buffer */
#else
	uint64_t blks                         : 4;
	uint64_t base                         : 5;
	uint64_t reserved_9_31                : 23;
	uint64_t compblks                     : 5;
	uint64_t reserved_37_63               : 27;
#endif
	} cn61xx;
	struct cvmx_dpi_engx_buf_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t base                         : 4;  /**< The base address in 512B blocks of the engine fifo */
	uint64_t blks                         : 4;  /**< The size in 512B blocks of the engine fifo
                                                         Legal values are 0-8.
                                                         0 = Engine is disabled
                                                         1 = 0.5KB buffer
                                                         2 = 1.0KB buffer
                                                         3 = 1.5KB buffer
                                                         4 = 2.0KB buffer
                                                         5 = 2.5KB buffer
                                                         6 = 3.0KB buffer
                                                         7 = 3.5KB buffer
                                                         8 = 4.0KB buffer */
#else
	uint64_t blks                         : 4;
	uint64_t base                         : 4;
	uint64_t reserved_8_63                : 56;
#endif
	} cn63xx;
	struct cvmx_dpi_engx_buf_cn63xx       cn63xxp1;
	struct cvmx_dpi_engx_buf_cn61xx       cn66xx;
	struct cvmx_dpi_engx_buf_cn61xx       cn68xx;
	struct cvmx_dpi_engx_buf_cn61xx       cn68xxp1;
	struct cvmx_dpi_engx_buf_cn61xx       cn70xx;
	struct cvmx_dpi_engx_buf_cn61xx       cn70xxp1;
	struct cvmx_dpi_engx_buf_s            cn73xx;
	struct cvmx_dpi_engx_buf_s            cn78xx;
	struct cvmx_dpi_engx_buf_s            cn78xxp1;
	struct cvmx_dpi_engx_buf_cn61xx       cnf71xx;
	struct cvmx_dpi_engx_buf_s            cnf75xx;
};
typedef union cvmx_dpi_engx_buf cvmx_dpi_engx_buf_t;

/**
 * cvmx_dpi_info_reg
 */
union cvmx_dpi_info_reg {
	uint64_t u64;
	struct cvmx_dpi_info_reg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t ffp                          : 4;  /**< Force forward progress indicator. */
	uint64_t reserved_2_3                 : 2;
	uint64_t ncb                          : 1;  /**< NCB register access. This bit gets set in normal operation when software reads a DPI
                                                         register through the IOI interface. */
	uint64_t rsl                          : 1;  /**< RSL register access. This bit is set in normal operation when software reads a DPI
                                                         register through the RSL interface. */
#else
	uint64_t rsl                          : 1;
	uint64_t ncb                          : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t ffp                          : 4;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_dpi_info_reg_s            cn61xx;
	struct cvmx_dpi_info_reg_s            cn63xx;
	struct cvmx_dpi_info_reg_cn63xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t ncb                          : 1;  /**< NCB Register Access
                                                         This interrupt will fire in normal operation
                                                         when SW reads a DPI register through the NCB
                                                         interface. */
	uint64_t rsl                          : 1;  /**< RSL Register Access
                                                         This interrupt will fire in normal operation
                                                         when SW reads a DPI register through the RSL
                                                         interface. */
#else
	uint64_t rsl                          : 1;
	uint64_t ncb                          : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} cn63xxp1;
	struct cvmx_dpi_info_reg_s            cn66xx;
	struct cvmx_dpi_info_reg_s            cn68xx;
	struct cvmx_dpi_info_reg_s            cn68xxp1;
	struct cvmx_dpi_info_reg_s            cn70xx;
	struct cvmx_dpi_info_reg_s            cn70xxp1;
	struct cvmx_dpi_info_reg_s            cn73xx;
	struct cvmx_dpi_info_reg_s            cn78xx;
	struct cvmx_dpi_info_reg_s            cn78xxp1;
	struct cvmx_dpi_info_reg_s            cnf71xx;
	struct cvmx_dpi_info_reg_s            cnf75xx;
};
typedef union cvmx_dpi_info_reg cvmx_dpi_info_reg_t;

/**
 * cvmx_dpi_int_en
 */
union cvmx_dpi_int_en {
	uint64_t u64;
	struct cvmx_dpi_int_en_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_28_63               : 36;
	uint64_t sprt3_rst                    : 1;  /**< DMA instruction was dropped because the source or
                                                          destination port was in reset.
                                                         this bit is set. */
	uint64_t sprt2_rst                    : 1;  /**< DMA instruction was dropped because the source or
                                                          destination port was in reset.
                                                         this bit is set. */
	uint64_t sprt1_rst                    : 1;  /**< DMA instruction was dropped because the source or
                                                          destination port was in reset.
                                                         this bit is set. */
	uint64_t sprt0_rst                    : 1;  /**< DMA instruction was dropped because the source or
                                                          destination port was in reset.
                                                         this bit is set. */
	uint64_t reserved_23_23               : 1;
	uint64_t req_badfil                   : 1;  /**< DMA instruction unexpected fill */
	uint64_t req_inull                    : 1;  /**< DMA instruction filled with NULL pointer */
	uint64_t req_anull                    : 1;  /**< DMA instruction filled with bad instruction */
	uint64_t req_undflw                   : 1;  /**< DMA instruction FIFO underflow */
	uint64_t req_ovrflw                   : 1;  /**< DMA instruction FIFO overflow */
	uint64_t req_badlen                   : 1;  /**< DMA instruction fetch with length */
	uint64_t req_badadr                   : 1;  /**< DMA instruction fetch with bad pointer */
	uint64_t dmadbo                       : 8;  /**< DMAx doorbell overflow. */
	uint64_t reserved_2_7                 : 6;
	uint64_t nfovr                        : 1;  /**< CSR Fifo Overflow */
	uint64_t nderr                        : 1;  /**< NCB Decode Error */
#else
	uint64_t nderr                        : 1;
	uint64_t nfovr                        : 1;
	uint64_t reserved_2_7                 : 6;
	uint64_t dmadbo                       : 8;
	uint64_t req_badadr                   : 1;
	uint64_t req_badlen                   : 1;
	uint64_t req_ovrflw                   : 1;
	uint64_t req_undflw                   : 1;
	uint64_t req_anull                    : 1;
	uint64_t req_inull                    : 1;
	uint64_t req_badfil                   : 1;
	uint64_t reserved_23_23               : 1;
	uint64_t sprt0_rst                    : 1;
	uint64_t sprt1_rst                    : 1;
	uint64_t sprt2_rst                    : 1;
	uint64_t sprt3_rst                    : 1;
	uint64_t reserved_28_63               : 36;
#endif
	} s;
	struct cvmx_dpi_int_en_s              cn61xx;
	struct cvmx_dpi_int_en_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_26_63               : 38;
	uint64_t sprt1_rst                    : 1;  /**< DMA instruction was dropped because the source or
                                                          destination port was in reset.
                                                         this bit is set. */
	uint64_t sprt0_rst                    : 1;  /**< DMA instruction was dropped because the source or
                                                          destination port was in reset.
                                                         this bit is set. */
	uint64_t reserved_23_23               : 1;
	uint64_t req_badfil                   : 1;  /**< DMA instruction unexpected fill */
	uint64_t req_inull                    : 1;  /**< DMA instruction filled with NULL pointer */
	uint64_t req_anull                    : 1;  /**< DMA instruction filled with bad instruction */
	uint64_t req_undflw                   : 1;  /**< DMA instruction FIFO underflow */
	uint64_t req_ovrflw                   : 1;  /**< DMA instruction FIFO overflow */
	uint64_t req_badlen                   : 1;  /**< DMA instruction fetch with length */
	uint64_t req_badadr                   : 1;  /**< DMA instruction fetch with bad pointer */
	uint64_t dmadbo                       : 8;  /**< DMAx doorbell overflow. */
	uint64_t reserved_2_7                 : 6;
	uint64_t nfovr                        : 1;  /**< CSR Fifo Overflow */
	uint64_t nderr                        : 1;  /**< NCB Decode Error */
#else
	uint64_t nderr                        : 1;
	uint64_t nfovr                        : 1;
	uint64_t reserved_2_7                 : 6;
	uint64_t dmadbo                       : 8;
	uint64_t req_badadr                   : 1;
	uint64_t req_badlen                   : 1;
	uint64_t req_ovrflw                   : 1;
	uint64_t req_undflw                   : 1;
	uint64_t req_anull                    : 1;
	uint64_t req_inull                    : 1;
	uint64_t req_badfil                   : 1;
	uint64_t reserved_23_23               : 1;
	uint64_t sprt0_rst                    : 1;
	uint64_t sprt1_rst                    : 1;
	uint64_t reserved_26_63               : 38;
#endif
	} cn63xx;
	struct cvmx_dpi_int_en_cn63xx         cn63xxp1;
	struct cvmx_dpi_int_en_s              cn66xx;
	struct cvmx_dpi_int_en_cn63xx         cn68xx;
	struct cvmx_dpi_int_en_cn63xx         cn68xxp1;
	struct cvmx_dpi_int_en_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_28_63               : 36;
	uint64_t sprt3_rst                    : 1;  /**< DMA instruction was dropped because the source or
                                                         destination port was in reset.
                                                         this bit is set. */
	uint64_t sprt2_rst                    : 1;  /**< DMA instruction was dropped because the source or
                                                         destination port was in reset.
                                                         this bit is set. */
	uint64_t sprt1_rst                    : 1;  /**< DMA instruction was dropped because the source or
                                                         destination port was in reset.
                                                         this bit is set. */
	uint64_t sprt0_rst                    : 1;  /**< DMA instruction was dropped because the source or
                                                         destination port was in reset.
                                                         this bit is set. */
	uint64_t reserved_23_23               : 1;
	uint64_t req_badfil                   : 1;  /**< DMA instruction unexpected fill */
	uint64_t req_inull                    : 1;  /**< DMA instruction filled with NULL pointer */
	uint64_t req_anull                    : 1;  /**< DMA instruction filled with bad instruction */
	uint64_t req_undflw                   : 1;  /**< DMA instruction FIFO underflow */
	uint64_t req_ovrflw                   : 1;  /**< DMA instruction FIFO overflow */
	uint64_t req_badlen                   : 1;  /**< DMA instruction fetch with length */
	uint64_t req_badadr                   : 1;  /**< DMA instruction fetch with bad pointer */
	uint64_t dmadbo                       : 8;  /**< DMAx doorbell overflow. */
	uint64_t reserved_7_2                 : 6;
	uint64_t nfovr                        : 1;  /**< CSR Fifo Overflow */
	uint64_t nderr                        : 1;  /**< NCB Decode Error */
#else
	uint64_t nderr                        : 1;
	uint64_t nfovr                        : 1;
	uint64_t reserved_7_2                 : 6;
	uint64_t dmadbo                       : 8;
	uint64_t req_badadr                   : 1;
	uint64_t req_badlen                   : 1;
	uint64_t req_ovrflw                   : 1;
	uint64_t req_undflw                   : 1;
	uint64_t req_anull                    : 1;
	uint64_t req_inull                    : 1;
	uint64_t req_badfil                   : 1;
	uint64_t reserved_23_23               : 1;
	uint64_t sprt0_rst                    : 1;
	uint64_t sprt1_rst                    : 1;
	uint64_t sprt2_rst                    : 1;
	uint64_t sprt3_rst                    : 1;
	uint64_t reserved_28_63               : 36;
#endif
	} cn70xx;
	struct cvmx_dpi_int_en_cn70xx         cn70xxp1;
	struct cvmx_dpi_int_en_s              cnf71xx;
};
typedef union cvmx_dpi_int_en cvmx_dpi_int_en_t;

/**
 * cvmx_dpi_int_reg
 *
 * This register contains error flags for DPI.
 *
 */
union cvmx_dpi_int_reg {
	uint64_t u64;
	struct cvmx_dpi_int_reg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_28_63               : 36;
	uint64_t sprt3_rst                    : 1;  /**< DMA instruction was dropped because the source or
                                                          destination port was in reset.
                                                         this bit is set. */
	uint64_t sprt2_rst                    : 1;  /**< DMA instruction was dropped because the source or
                                                          destination port was in reset.
                                                         this bit is set. */
	uint64_t sprt1_rst                    : 1;  /**< DMA instruction was dropped because the source or
                                                          destination port was in reset.
                                                         this bit is set. */
	uint64_t sprt0_rst                    : 1;  /**< DMA instruction was dropped because the source or
                                                          destination port was in reset.
                                                         this bit is set. */
	uint64_t reserved_23_23               : 1;
	uint64_t req_badfil                   : 1;  /**< Unexpected fill error. This bit is set when an instruction fill is received when there is
                                                         no outstanding request. Throws DPI_INTSN_E::DPI_INT_REQ_BADFIL. */
	uint64_t req_inull                    : 1;  /**< Interrupt should be ignored. */
	uint64_t req_anull                    : 1;  /**< Instruction bad error. This bit is set when a fetched instruction word was 0x0. Throws
                                                         DPI_INTSN_E::DPI_INT_REQ_ANULL. */
	uint64_t req_undflw                   : 1;  /**< Instruction FIFO underflow error. This bit is set when the instruction FIFO underflows.
                                                         Throws DPI_INTSN_E::DPI_INT_REQ_UNDFLW. */
	uint64_t req_ovrflw                   : 1;  /**< Instruction FIFO overflow error. This bit is set when the instruction FIFO overflows.
                                                         Throws DPI_INTSN_E::DPI_INT_REQ_OVRFLW. */
	uint64_t req_badlen                   : 1;  /**< Fetch with length zero error. This bit is set when DPI forms an instruction fetch with
                                                         length of zero. Throws DPI_INTSN_E::DPI_INT_REQ_BADLEN. */
	uint64_t req_badadr                   : 1;  /**< Fetch with bad pointer error. This bit is set when DPI forms an instruction fetch to the
                                                         NULL pointer. Throws DPI_INTSN_E::DPI_INT_REQ_BADADR. */
	uint64_t dmadbo                       : 8;  /**< Doorbell overflow error. DPI has a 32-bit counter for each request queue's outstanding
                                                         doorbell counts. A bit is set when the corresponding doorbell count overflows. Throws
                                                         DPI_INTSN_E::DPI_INT_DMADBO(). */
	uint64_t reserved_2_7                 : 6;
	uint64_t nfovr                        : 1;  /**< CSR FIFO overflow error. DPI can store up to 16 CSR requests, and the FIFO overflows if
                                                         that number is exceeded. Throws DPI_INTSN_E::DPI_INT_NFOVR. */
	uint64_t nderr                        : 1;  /**< IOI decode error. This bit is set when the DPI received an IOI transaction on the outbound
                                                         bus to the DPI device ID, but the command was not recognized. Throws
                                                         DPI_INTSN_E::DPI_INT_NDERR. */
#else
	uint64_t nderr                        : 1;
	uint64_t nfovr                        : 1;
	uint64_t reserved_2_7                 : 6;
	uint64_t dmadbo                       : 8;
	uint64_t req_badadr                   : 1;
	uint64_t req_badlen                   : 1;
	uint64_t req_ovrflw                   : 1;
	uint64_t req_undflw                   : 1;
	uint64_t req_anull                    : 1;
	uint64_t req_inull                    : 1;
	uint64_t req_badfil                   : 1;
	uint64_t reserved_23_23               : 1;
	uint64_t sprt0_rst                    : 1;
	uint64_t sprt1_rst                    : 1;
	uint64_t sprt2_rst                    : 1;
	uint64_t sprt3_rst                    : 1;
	uint64_t reserved_28_63               : 36;
#endif
	} s;
	struct cvmx_dpi_int_reg_s             cn61xx;
	struct cvmx_dpi_int_reg_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_26_63               : 38;
	uint64_t sprt1_rst                    : 1;  /**< DMA instruction was dropped because the source or
                                                          destination port was in reset.
                                                         this bit is set. */
	uint64_t sprt0_rst                    : 1;  /**< DMA instruction was dropped because the source or
                                                          destination port was in reset.
                                                         this bit is set. */
	uint64_t reserved_23_23               : 1;
	uint64_t req_badfil                   : 1;  /**< DMA instruction unexpected fill
                                                         Instruction fill when none outstanding. */
	uint64_t req_inull                    : 1;  /**< DMA instruction filled with NULL pointer
                                                         Next pointer was NULL. */
	uint64_t req_anull                    : 1;  /**< DMA instruction filled with bad instruction
                                                         Fetched instruction word was 0. */
	uint64_t req_undflw                   : 1;  /**< DMA instruction FIFO underflow
                                                         DPI tracks outstanding instructions fetches.
                                                         Interrupt will fire when FIFO underflows. */
	uint64_t req_ovrflw                   : 1;  /**< DMA instruction FIFO overflow
                                                         DPI tracks outstanding instructions fetches.
                                                         Interrupt will fire when FIFO overflows. */
	uint64_t req_badlen                   : 1;  /**< DMA instruction fetch with length
                                                         Interrupt will fire if DPI forms an instruction
                                                         fetch with length of zero. */
	uint64_t req_badadr                   : 1;  /**< DMA instruction fetch with bad pointer
                                                         Interrupt will fire if DPI forms an instruction
                                                         fetch to the NULL pointer. */
	uint64_t dmadbo                       : 8;  /**< DMAx doorbell overflow.
                                                         DPI has a 32-bit counter for each request's queue
                                                         outstanding doorbell counts. Interrupt will fire
                                                         if the count overflows. */
	uint64_t reserved_2_7                 : 6;
	uint64_t nfovr                        : 1;  /**< CSR Fifo Overflow
                                                         DPI can store upto 16 CSR request.  The FIFO will
                                                         overflow if that number is exceeded. */
	uint64_t nderr                        : 1;  /**< NCB Decode Error
                                                         DPI received a NCB transaction on the outbound
                                                         bus to the DPI deviceID, but the command was not
                                                         recognized. */
#else
	uint64_t nderr                        : 1;
	uint64_t nfovr                        : 1;
	uint64_t reserved_2_7                 : 6;
	uint64_t dmadbo                       : 8;
	uint64_t req_badadr                   : 1;
	uint64_t req_badlen                   : 1;
	uint64_t req_ovrflw                   : 1;
	uint64_t req_undflw                   : 1;
	uint64_t req_anull                    : 1;
	uint64_t req_inull                    : 1;
	uint64_t req_badfil                   : 1;
	uint64_t reserved_23_23               : 1;
	uint64_t sprt0_rst                    : 1;
	uint64_t sprt1_rst                    : 1;
	uint64_t reserved_26_63               : 38;
#endif
	} cn63xx;
	struct cvmx_dpi_int_reg_cn63xx        cn63xxp1;
	struct cvmx_dpi_int_reg_s             cn66xx;
	struct cvmx_dpi_int_reg_cn63xx        cn68xx;
	struct cvmx_dpi_int_reg_cn63xx        cn68xxp1;
	struct cvmx_dpi_int_reg_s             cn70xx;
	struct cvmx_dpi_int_reg_s             cn70xxp1;
	struct cvmx_dpi_int_reg_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_23_63               : 41;
	uint64_t req_badfil                   : 1;  /**< Unexpected fill error. This bit is set when an instruction fill is received when there is
                                                         no outstanding request. Throws DPI_INTSN_E::DPI_INT_REQ_BADFIL. */
	uint64_t req_inull                    : 1;  /**< Interrupt should be ignored. */
	uint64_t req_anull                    : 1;  /**< Instruction bad error. This bit is set when a fetched instruction word was 0x0. Throws
                                                         DPI_INTSN_E::DPI_INT_REQ_ANULL. */
	uint64_t req_undflw                   : 1;  /**< Instruction FIFO underflow error. This bit is set when the instruction FIFO underflows.
                                                         Throws DPI_INTSN_E::DPI_INT_REQ_UNDFLW. */
	uint64_t req_ovrflw                   : 1;  /**< Instruction FIFO overflow error. This bit is set when the instruction FIFO overflows.
                                                         Throws DPI_INTSN_E::DPI_INT_REQ_OVRFLW. */
	uint64_t req_badlen                   : 1;  /**< Fetch with length zero error. This bit is set when DPI forms an instruction fetch with
                                                         length of zero. Throws DPI_INTSN_E::DPI_INT_REQ_BADLEN. */
	uint64_t req_badadr                   : 1;  /**< Fetch with bad pointer error. This bit is set when DPI forms an instruction fetch to the
                                                         NULL pointer. Throws DPI_INTSN_E::DPI_INT_REQ_BADADR. */
	uint64_t dmadbo                       : 8;  /**< Doorbell overflow error. DPI has a 32-bit counter for each request queue's outstanding
                                                         doorbell counts. A bit is set when the corresponding doorbell count overflows. Throws
                                                         DPI_INTSN_E::DPI_INT_DMADBO(). */
	uint64_t reserved_2_7                 : 6;
	uint64_t nfovr                        : 1;  /**< CSR FIFO overflow error. DPI can store up to 16 CSR requests, and the FIFO overflows if
                                                         that number is exceeded. Throws DPI_INTSN_E::DPI_INT_NFOVR. */
	uint64_t nderr                        : 1;  /**< IOI decode error. This bit is set when the DPI received an IOI transaction on the outbound
                                                         bus to the DPI device ID, but the command was not recognized. Throws
                                                         DPI_INTSN_E::DPI_INT_NDERR. */
#else
	uint64_t nderr                        : 1;
	uint64_t nfovr                        : 1;
	uint64_t reserved_2_7                 : 6;
	uint64_t dmadbo                       : 8;
	uint64_t req_badadr                   : 1;
	uint64_t req_badlen                   : 1;
	uint64_t req_ovrflw                   : 1;
	uint64_t req_undflw                   : 1;
	uint64_t req_anull                    : 1;
	uint64_t req_inull                    : 1;
	uint64_t req_badfil                   : 1;
	uint64_t reserved_23_63               : 41;
#endif
	} cn73xx;
	struct cvmx_dpi_int_reg_cn73xx        cn78xx;
	struct cvmx_dpi_int_reg_s             cn78xxp1;
	struct cvmx_dpi_int_reg_s             cnf71xx;
	struct cvmx_dpi_int_reg_cn73xx        cnf75xx;
};
typedef union cvmx_dpi_int_reg cvmx_dpi_int_reg_t;

/**
 * cvmx_dpi_ncb#_cfg
 */
union cvmx_dpi_ncbx_cfg {
	uint64_t u64;
	struct cvmx_dpi_ncbx_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t molr                         : 6;  /**< Max outstanding load requests. Limits the number of outstanding load requests on the I/O
                                                         Interconnect. This value can range from 1 to 32. Setting a value of 0 halts all read
                                                         traffic to the I/O Interconnect. There are no restrictions on when this value can be
                                                         changed. */
#else
	uint64_t molr                         : 6;
	uint64_t reserved_6_63                : 58;
#endif
	} s;
	struct cvmx_dpi_ncbx_cfg_s            cn61xx;
	struct cvmx_dpi_ncbx_cfg_s            cn66xx;
	struct cvmx_dpi_ncbx_cfg_s            cn68xx;
	struct cvmx_dpi_ncbx_cfg_s            cn70xx;
	struct cvmx_dpi_ncbx_cfg_s            cn70xxp1;
	struct cvmx_dpi_ncbx_cfg_s            cn73xx;
	struct cvmx_dpi_ncbx_cfg_s            cn78xx;
	struct cvmx_dpi_ncbx_cfg_s            cn78xxp1;
	struct cvmx_dpi_ncbx_cfg_s            cnf71xx;
	struct cvmx_dpi_ncbx_cfg_s            cnf75xx;
};
typedef union cvmx_dpi_ncbx_cfg cvmx_dpi_ncbx_cfg_t;

/**
 * cvmx_dpi_ncb_ctl
 *
 * This register chooses which NCB interface DPI uses for L2/DRAM reads/writes.
 *
 */
union cvmx_dpi_ncb_ctl {
	uint64_t u64;
	struct cvmx_dpi_ncb_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_25_63               : 39;
	uint64_t ncbsel_prt_xor_dis           : 1;  /**< When set, prevents DPI_DMA_INSTR_HDR_S[FPORT,LPORT] from determining the IOI/NCB
                                                         DPI uses to process instructions. See [NCBSEL_SRC], [NCBSEL_DST], and
                                                         DPI_SLI_PRT()_CFG[NCBSEL]. */
	uint64_t reserved_21_23               : 3;
	uint64_t ncbsel_zbw                   : 1;  /**< Selects the IOBI/NCBI bus DPI uses for byte status writes by any DPI_HDR_PT_E::ZBW_CA,
                                                         DPI_HDR_PT_E::ZBW_NC, or DPI_HDR_PT_E::WQP DPI DMA instructions.
                                                         0 = DPI uses IOBI2/NCBI2 for all byte status writes.
                                                         1 = DPI uses IOBI3/NCBI3 for all byte status writes. */
	uint64_t reserved_17_19               : 3;
	uint64_t ncbsel_req                   : 1;  /**< Selects the IOB/NCB bus DPI uses for fetching the DPI_DMA_INSTR_HDR_S's,
                                                         the first pointers, and the last pointers. DPI does not read the source data of
                                                         the DPI DMA instruction via [NCBSEL_REQ] - see DPI_SLI_PRT()_CFG[NCBSEL] and
                                                         [NCBSEL_SRC].
                                                         0 = DPI uses IOBI2/IOBO2 (aka NCBI2/NCBO2) for all DPI_DMA_INSTR_HDR_S and pointer reads.
                                                         1 = DPI uses IOBI3/IOBO3 (aka NCBI3/NCBO3) for all DPI_DMA_INSTR_HDR_S and pointer reads. */
	uint64_t reserved_13_15               : 3;
	uint64_t ncbsel_dst                   : 1;  /**< The default IOI/NCB that DPI uses for DPI_HDR_XTYPE_E::INTERNAL_ONLY L2/DRAM data
                                                         writes.
                                                         In the normal case (DPI_NCB_CTL[NCBSEL_PRT_XOR_DIS] clear),
                                                            [NCBSEL_DST] XOR DPI_DMA_INSTR_HDR_S[LPORT<0>]
                                                         determines the IOI/NCB that DPI uses for writes.
                                                         0 = DPI uses IOBI2/IOBO2 (aka NCBI2/NCBO2) for DPI_HDR_XTYPE_E::INTERNAL_ONLY
                                                             DPI DMA instruction L2/DRAM writes.
                                                         1 = DPI uses IOBI3/IOBO3 (aka NCBI3/NCBO3) for DPI_HDR_XTYPE_E::INTERNAL_ONLY
                                                             DPI DMA instruction L2/DRAM writes.
                                                         If DPI_NCB_CTL[NCBSEL_PRT_XOR_DIS] is set, [NCBSEL_DST] solely determines the IOI/NCB.
                                                         The DPI_SLI_PRT()_CFG[NCBSEL] selected by DPI_DMA_INSTR_HDR_S[LPORT]
                                                         is the equivalent of [NCBSEL_DST] for DPI_HDR_XTYPE_E::INBOUND DPI DMA instructions.
                                                         (DPI_DMA_INSTR_HDR_S[FPORT<0>] is used with DPI_SLI_PRT()_CFG[NCBSEL]
                                                         for DPI_HDR_XTYPE_E::INBOUND, not DPI_DMA_INSTR_HDR_S[LPORT<0>].) */
	uint64_t reserved_9_11                : 3;
	uint64_t ncbsel_src                   : 1;  /**< The default IOI/NCB that DPI uses for DPI_HDR_XTYPE_E::INTERNAL_ONLY L2/DRAM data
                                                         reads.
                                                         In the normal case (DPI_NCB_CTL[NCBSEL_PRT_XOR_DIS] clear),
                                                            [NCBSEL_SRC] XOR DPI_DMA_INSTR_HDR_S[FPORT<0>]
                                                         determines the IOI/NCB that DPI uses.
                                                         0 = DPI uses IOBI2/IOBO2 (aka NCBI2/NCBO2) for DPI_HDR_XTYPE_E::INTERNAL_ONLY
                                                             DPI DMA instruction L2/DRAM data reads.
                                                         1 = DPI uses IOBI3/IOBO3 (aka NCBI3/NCBO3) for DPI_HDR_XTYPE_E::INTERNAL_ONLY
                                                             DPI DMA instruction L2/DRAM data reads.
                                                         If DPI_NCB_CTL[NCBSEL_PRT_XOR_DIS] is set, [NCBSEL_SRC] solely determines the IOI/NCB.
                                                         Note that DPI uses [NCBSEL_REQ] for DPI_DMA_INSTR_HDR_S and pointer IOI/NCB
                                                         reads, not [NCBSEL_SRC] nor DPI_DMA_INSTR_HDR_S[FPORT<0>].
                                                         The DPI_SLI_PRT()_CFG[NCBSEL] selected by DPI_DMA_INSTR_HDR_S[LPORT] is
                                                         the equivalent of [NCBSEL_SRC] for DPI_HDR_XTYPE_E::OUTBOUND DPI DMA instructions. */
	uint64_t reserved_1_7                 : 7;
	uint64_t prt                          : 1;  /**< Directs traffic to the specified NCB unit. (Pass 1 only) */
#else
	uint64_t prt                          : 1;
	uint64_t reserved_1_7                 : 7;
	uint64_t ncbsel_src                   : 1;
	uint64_t reserved_9_11                : 3;
	uint64_t ncbsel_dst                   : 1;
	uint64_t reserved_13_15               : 3;
	uint64_t ncbsel_req                   : 1;
	uint64_t reserved_17_19               : 3;
	uint64_t ncbsel_zbw                   : 1;
	uint64_t reserved_21_23               : 3;
	uint64_t ncbsel_prt_xor_dis           : 1;
	uint64_t reserved_25_63               : 39;
#endif
	} s;
	struct cvmx_dpi_ncb_ctl_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_25_63               : 39;
	uint64_t ncbsel_prt_xor_dis           : 1;  /**< When set, prevents DPI_DMA_INSTR_HDR_S[FPORT,LPORT] from determining the IOI/NCB
                                                         DPI uses to process instructions. See [NCBSEL_SRC], [NCBSEL_DST], and
                                                         DPI_SLI_PRT()_CFG[NCBSEL]. */
	uint64_t reserved_21_23               : 3;
	uint64_t ncbsel_zbw                   : 1;  /**< Selects the IOBI/NCBI bus DPI uses for byte status writes by any DPI_HDR_PT_E::ZBW_CA,
                                                         DPI_HDR_PT_E::ZBW_NC, or DPI_HDR_PT_E::WQP DPI DMA instructions.
                                                         0 = DPI uses IOBI2/NCBI2 for all byte status writes.
                                                         1 = DPI uses IOBI3/NCBI3 for all byte status writes. */
	uint64_t reserved_17_19               : 3;
	uint64_t ncbsel_req                   : 1;  /**< Selects the IOB/NCB bus DPI uses for fetching the DPI_DMA_INSTR_HDR_S's,
                                                         the first pointers, and the last pointers. DPI does not read the source data of
                                                         the DPI DMA instruction via [NCBSEL_REQ] - see DPI_SLI_PRT()_CFG[NCBSEL] and
                                                         [NCBSEL_SRC].
                                                         0 = DPI uses IOBI2/IOBO2 (aka NCBI2/NCBO2) for all DPI_DMA_INSTR_HDR_S and pointer reads.
                                                         1 = DPI uses IOBI3/IOBO3 (aka NCBI3/NCBO3) for all DPI_DMA_INSTR_HDR_S and pointer reads. */
	uint64_t reserved_13_15               : 3;
	uint64_t ncbsel_dst                   : 1;  /**< The default IOI/NCB that DPI uses for DPI_HDR_XTYPE_E::INTERNAL_ONLY L2/DRAM data
                                                         writes.
                                                         In the normal case (DPI_NCB_CTL[NCBSEL_PRT_XOR_DIS] clear),
                                                            [NCBSEL_DST] XOR DPI_DMA_INSTR_HDR_S[LPORT<0>]
                                                         determines the IOI/NCB that DPI uses for writes.
                                                         0 = DPI uses IOBI2/IOBO2 (aka NCBI2/NCBO2) for DPI_HDR_XTYPE_E::INTERNAL_ONLY
                                                             DPI DMA instruction L2/DRAM writes.
                                                         1 = DPI uses IOBI3/IOBO3 (aka NCBI3/NCBO3) for DPI_HDR_XTYPE_E::INTERNAL_ONLY
                                                             DPI DMA instruction L2/DRAM writes.
                                                         If DPI_NCB_CTL[NCBSEL_PRT_XOR_DIS] is set, [NCBSEL_DST] solely determines the IOI/NCB.
                                                         The DPI_SLI_PRT()_CFG[NCBSEL] selected by DPI_DMA_INSTR_HDR_S[LPORT]
                                                         is the equivalent of [NCBSEL_DST] for DPI_HDR_XTYPE_E::INBOUND DPI DMA instructions.
                                                         (DPI_DMA_INSTR_HDR_S[FPORT<0>] is used with DPI_SLI_PRT()_CFG[NCBSEL]
                                                         for DPI_HDR_XTYPE_E::INBOUND, not DPI_DMA_INSTR_HDR_S[LPORT<0>].) */
	uint64_t reserved_9_11                : 3;
	uint64_t ncbsel_src                   : 1;  /**< The default IOI/NCB that DPI uses for DPI_HDR_XTYPE_E::INTERNAL_ONLY L2/DRAM data
                                                         reads.
                                                         In the normal case (DPI_NCB_CTL[NCBSEL_PRT_XOR_DIS] clear),
                                                            [NCBSEL_SRC] XOR DPI_DMA_INSTR_HDR_S[FPORT<0>]
                                                         determines the IOI/NCB that DPI uses.
                                                         0 = DPI uses IOBI2/IOBO2 (aka NCBI2/NCBO2) for DPI_HDR_XTYPE_E::INTERNAL_ONLY
                                                             DPI DMA instruction L2/DRAM data reads.
                                                         1 = DPI uses IOBI3/IOBO3 (aka NCBI3/NCBO3) for DPI_HDR_XTYPE_E::INTERNAL_ONLY
                                                             DPI DMA instruction L2/DRAM data reads.
                                                         If DPI_NCB_CTL[NCBSEL_PRT_XOR_DIS] is set, [NCBSEL_SRC] solely determines the IOI/NCB.
                                                         Note that DPI uses [NCBSEL_REQ] for DPI_DMA_INSTR_HDR_S and pointer IOI/NCB
                                                         reads, not [NCBSEL_SRC] nor DPI_DMA_INSTR_HDR_S[FPORT<0>].
                                                         The DPI_SLI_PRT()_CFG[NCBSEL] selected by DPI_DMA_INSTR_HDR_S[LPORT] is
                                                         the equivalent of [NCBSEL_SRC] for DPI_HDR_XTYPE_E::OUTBOUND DPI DMA instructions. */
	uint64_t reserved_0_7                 : 8;
#else
	uint64_t reserved_0_7                 : 8;
	uint64_t ncbsel_src                   : 1;
	uint64_t reserved_9_11                : 3;
	uint64_t ncbsel_dst                   : 1;
	uint64_t reserved_13_15               : 3;
	uint64_t ncbsel_req                   : 1;
	uint64_t reserved_17_19               : 3;
	uint64_t ncbsel_zbw                   : 1;
	uint64_t reserved_21_23               : 3;
	uint64_t ncbsel_prt_xor_dis           : 1;
	uint64_t reserved_25_63               : 39;
#endif
	} cn73xx;
	struct cvmx_dpi_ncb_ctl_s             cn78xx;
	struct cvmx_dpi_ncb_ctl_s             cn78xxp1;
	struct cvmx_dpi_ncb_ctl_cn73xx        cnf75xx;
};
typedef union cvmx_dpi_ncb_ctl cvmx_dpi_ncb_ctl_t;

/**
 * cvmx_dpi_pint_info
 *
 * This register provides DPI packet interrupt information.
 *
 */
union cvmx_dpi_pint_info {
	uint64_t u64;
	struct cvmx_dpi_pint_info_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t iinfo                        : 6;  /**< Packet instruction doorbell count overflow information. */
	uint64_t reserved_6_7                 : 2;
	uint64_t sinfo                        : 6;  /**< Packet scatter list doorbell count overflow information. */
#else
	uint64_t sinfo                        : 6;
	uint64_t reserved_6_7                 : 2;
	uint64_t iinfo                        : 6;
	uint64_t reserved_14_63               : 50;
#endif
	} s;
	struct cvmx_dpi_pint_info_s           cn61xx;
	struct cvmx_dpi_pint_info_s           cn63xx;
	struct cvmx_dpi_pint_info_s           cn63xxp1;
	struct cvmx_dpi_pint_info_s           cn66xx;
	struct cvmx_dpi_pint_info_s           cn68xx;
	struct cvmx_dpi_pint_info_s           cn68xxp1;
	struct cvmx_dpi_pint_info_s           cn70xx;
	struct cvmx_dpi_pint_info_s           cn70xxp1;
	struct cvmx_dpi_pint_info_s           cn73xx;
	struct cvmx_dpi_pint_info_s           cn78xx;
	struct cvmx_dpi_pint_info_s           cn78xxp1;
	struct cvmx_dpi_pint_info_s           cnf71xx;
	struct cvmx_dpi_pint_info_s           cnf75xx;
};
typedef union cvmx_dpi_pint_info cvmx_dpi_pint_info_t;

/**
 * cvmx_dpi_pkt_err_rsp
 */
union cvmx_dpi_pkt_err_rsp {
	uint64_t u64;
	struct cvmx_dpi_pkt_err_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t pkterr                       : 1;  /**< Indicates that an ErrorResponse was received from the I/O subsystem. */
#else
	uint64_t pkterr                       : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_dpi_pkt_err_rsp_s         cn61xx;
	struct cvmx_dpi_pkt_err_rsp_s         cn63xx;
	struct cvmx_dpi_pkt_err_rsp_s         cn63xxp1;
	struct cvmx_dpi_pkt_err_rsp_s         cn66xx;
	struct cvmx_dpi_pkt_err_rsp_s         cn68xx;
	struct cvmx_dpi_pkt_err_rsp_s         cn68xxp1;
	struct cvmx_dpi_pkt_err_rsp_s         cn70xx;
	struct cvmx_dpi_pkt_err_rsp_s         cn70xxp1;
	struct cvmx_dpi_pkt_err_rsp_s         cn73xx;
	struct cvmx_dpi_pkt_err_rsp_s         cn78xx;
	struct cvmx_dpi_pkt_err_rsp_s         cn78xxp1;
	struct cvmx_dpi_pkt_err_rsp_s         cnf71xx;
	struct cvmx_dpi_pkt_err_rsp_s         cnf75xx;
};
typedef union cvmx_dpi_pkt_err_rsp cvmx_dpi_pkt_err_rsp_t;

/**
 * cvmx_dpi_req_err_rsp
 */
union cvmx_dpi_req_err_rsp {
	uint64_t u64;
	struct cvmx_dpi_req_err_rsp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t qerr                         : 8;  /**< Indicates which instruction queue received an ErrorResponse from the I/O subsystem.
                                                         Software must clear the bit before the corresponding instruction queue will continue
                                                         processing instructions if DPI_REQ_ERR_RSP_EN[EN] is set. */
#else
	uint64_t qerr                         : 8;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_dpi_req_err_rsp_s         cn61xx;
	struct cvmx_dpi_req_err_rsp_s         cn63xx;
	struct cvmx_dpi_req_err_rsp_s         cn63xxp1;
	struct cvmx_dpi_req_err_rsp_s         cn66xx;
	struct cvmx_dpi_req_err_rsp_s         cn68xx;
	struct cvmx_dpi_req_err_rsp_s         cn68xxp1;
	struct cvmx_dpi_req_err_rsp_s         cn70xx;
	struct cvmx_dpi_req_err_rsp_s         cn70xxp1;
	struct cvmx_dpi_req_err_rsp_s         cn73xx;
	struct cvmx_dpi_req_err_rsp_s         cn78xx;
	struct cvmx_dpi_req_err_rsp_s         cn78xxp1;
	struct cvmx_dpi_req_err_rsp_s         cnf71xx;
	struct cvmx_dpi_req_err_rsp_s         cnf75xx;
};
typedef union cvmx_dpi_req_err_rsp cvmx_dpi_req_err_rsp_t;

/**
 * cvmx_dpi_req_err_rsp_en
 */
union cvmx_dpi_req_err_rsp_en {
	uint64_t u64;
	struct cvmx_dpi_req_err_rsp_en_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t en                           : 8;  /**< Indicates which instruction queues should stop dispatching instructions when an
                                                         ErrorResponse is received from the I/O subsystem. */
#else
	uint64_t en                           : 8;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_dpi_req_err_rsp_en_s      cn61xx;
	struct cvmx_dpi_req_err_rsp_en_s      cn63xx;
	struct cvmx_dpi_req_err_rsp_en_s      cn63xxp1;
	struct cvmx_dpi_req_err_rsp_en_s      cn66xx;
	struct cvmx_dpi_req_err_rsp_en_s      cn68xx;
	struct cvmx_dpi_req_err_rsp_en_s      cn68xxp1;
	struct cvmx_dpi_req_err_rsp_en_s      cn70xx;
	struct cvmx_dpi_req_err_rsp_en_s      cn70xxp1;
	struct cvmx_dpi_req_err_rsp_en_s      cn73xx;
	struct cvmx_dpi_req_err_rsp_en_s      cn78xx;
	struct cvmx_dpi_req_err_rsp_en_s      cn78xxp1;
	struct cvmx_dpi_req_err_rsp_en_s      cnf71xx;
	struct cvmx_dpi_req_err_rsp_en_s      cnf75xx;
};
typedef union cvmx_dpi_req_err_rsp_en cvmx_dpi_req_err_rsp_en_t;

/**
 * cvmx_dpi_req_err_rst
 */
union cvmx_dpi_req_err_rst {
	uint64_t u64;
	struct cvmx_dpi_req_err_rst_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t qerr                         : 8;  /**< Reserved. */
#else
	uint64_t qerr                         : 8;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_dpi_req_err_rst_s         cn61xx;
	struct cvmx_dpi_req_err_rst_s         cn63xx;
	struct cvmx_dpi_req_err_rst_s         cn63xxp1;
	struct cvmx_dpi_req_err_rst_s         cn66xx;
	struct cvmx_dpi_req_err_rst_s         cn68xx;
	struct cvmx_dpi_req_err_rst_s         cn68xxp1;
	struct cvmx_dpi_req_err_rst_s         cn70xx;
	struct cvmx_dpi_req_err_rst_s         cn70xxp1;
	struct cvmx_dpi_req_err_rst_s         cn73xx;
	struct cvmx_dpi_req_err_rst_s         cn78xx;
	struct cvmx_dpi_req_err_rst_s         cn78xxp1;
	struct cvmx_dpi_req_err_rst_s         cnf71xx;
	struct cvmx_dpi_req_err_rst_s         cnf75xx;
};
typedef union cvmx_dpi_req_err_rst cvmx_dpi_req_err_rst_t;

/**
 * cvmx_dpi_req_err_rst_en
 */
union cvmx_dpi_req_err_rst_en {
	uint64_t u64;
	struct cvmx_dpi_req_err_rst_en_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t en                           : 8;  /**< Reserved. */
#else
	uint64_t en                           : 8;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_dpi_req_err_rst_en_s      cn61xx;
	struct cvmx_dpi_req_err_rst_en_s      cn63xx;
	struct cvmx_dpi_req_err_rst_en_s      cn63xxp1;
	struct cvmx_dpi_req_err_rst_en_s      cn66xx;
	struct cvmx_dpi_req_err_rst_en_s      cn68xx;
	struct cvmx_dpi_req_err_rst_en_s      cn68xxp1;
	struct cvmx_dpi_req_err_rst_en_s      cn70xx;
	struct cvmx_dpi_req_err_rst_en_s      cn70xxp1;
	struct cvmx_dpi_req_err_rst_en_s      cn73xx;
	struct cvmx_dpi_req_err_rst_en_s      cn78xx;
	struct cvmx_dpi_req_err_rst_en_s      cn78xxp1;
	struct cvmx_dpi_req_err_rst_en_s      cnf71xx;
	struct cvmx_dpi_req_err_rst_en_s      cnf75xx;
};
typedef union cvmx_dpi_req_err_rst_en cvmx_dpi_req_err_rst_en_t;

/**
 * cvmx_dpi_req_err_skip_comp
 */
union cvmx_dpi_req_err_skip_comp {
	uint64_t u64;
	struct cvmx_dpi_req_err_skip_comp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t en_rst                       : 8;  /**< Reserved. */
	uint64_t reserved_8_15                : 8;
	uint64_t en_rsp                       : 8;  /**< Reserved. */
#else
	uint64_t en_rsp                       : 8;
	uint64_t reserved_8_15                : 8;
	uint64_t en_rst                       : 8;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_dpi_req_err_skip_comp_s   cn61xx;
	struct cvmx_dpi_req_err_skip_comp_s   cn66xx;
	struct cvmx_dpi_req_err_skip_comp_s   cn68xx;
	struct cvmx_dpi_req_err_skip_comp_s   cn68xxp1;
	struct cvmx_dpi_req_err_skip_comp_s   cn70xx;
	struct cvmx_dpi_req_err_skip_comp_s   cn70xxp1;
	struct cvmx_dpi_req_err_skip_comp_s   cn73xx;
	struct cvmx_dpi_req_err_skip_comp_s   cn78xx;
	struct cvmx_dpi_req_err_skip_comp_s   cn78xxp1;
	struct cvmx_dpi_req_err_skip_comp_s   cnf71xx;
	struct cvmx_dpi_req_err_skip_comp_s   cnf75xx;
};
typedef union cvmx_dpi_req_err_skip_comp cvmx_dpi_req_err_skip_comp_t;

/**
 * cvmx_dpi_req_gbl_en
 */
union cvmx_dpi_req_gbl_en {
	uint64_t u64;
	struct cvmx_dpi_req_gbl_en_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t qen                          : 8;  /**< Indicates which instruction queues are enabled, and can dispatch instructions to a
                                                         requesting engine. */
#else
	uint64_t qen                          : 8;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_dpi_req_gbl_en_s          cn61xx;
	struct cvmx_dpi_req_gbl_en_s          cn63xx;
	struct cvmx_dpi_req_gbl_en_s          cn63xxp1;
	struct cvmx_dpi_req_gbl_en_s          cn66xx;
	struct cvmx_dpi_req_gbl_en_s          cn68xx;
	struct cvmx_dpi_req_gbl_en_s          cn68xxp1;
	struct cvmx_dpi_req_gbl_en_s          cn70xx;
	struct cvmx_dpi_req_gbl_en_s          cn70xxp1;
	struct cvmx_dpi_req_gbl_en_s          cn73xx;
	struct cvmx_dpi_req_gbl_en_s          cn78xx;
	struct cvmx_dpi_req_gbl_en_s          cn78xxp1;
	struct cvmx_dpi_req_gbl_en_s          cnf71xx;
	struct cvmx_dpi_req_gbl_en_s          cnf75xx;
};
typedef union cvmx_dpi_req_gbl_en cvmx_dpi_req_gbl_en_t;

/**
 * cvmx_dpi_sli_prt#_cfg
 *
 * This register configures the max read request size, max payload size, and max number of SLI
 * tags in use. Indexed by SLI_PORT_E.
 */
union cvmx_dpi_sli_prtx_cfg {
	uint64_t u64;
	struct cvmx_dpi_sli_prtx_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_29_63               : 35;
	uint64_t ncbsel                       : 1;  /**< The default IOI/NCB that DPI uses for DPI_HDR_XTYPE_E::OUTBOUND L2/DRAM data
                                                         reads and DPI_HDR_XTYPE_E::INBOUND L2/DRAM writes. DPI_DMA_INSTR_HDR_S[LPORT]
                                                         selects which DPI_SLI_PRT()_CFG[NCBSEL] is used for a DPI DMA instruction.
                                                         In the normal case (DPI_NCB_CTL[NCBSEL_PRT_XOR_DIS] clear),
                                                         _   [NCBSEL] XOR DPI_DMA_INSTR_HDR_S[FPORT<0>]
                                                         determines the IOI/NCB that DPI uses.
                                                         0 = DPI uses IOBI2/IOBO2 (aka NCBI2/NCBO2) for DPI_HDR_XTYPE_E::OUTBOUND and
                                                             DPI_HDR_XTYPE_E::INBOUND DPI DMA instructions.
                                                         1 = DPI uses IOBI3/IOBO3 (aka NCBI3/NCBO3) for DPI_HDR_XTYPE_E::OUTBOUND and
                                                             DPI_HDR_XTYPE_E::INBOUND DPI DMA instructions.
                                                         If DPI_NCB_CTL[NCBSEL_PRT_XOR_DIS] is set, [NCBSEL] solely determines the IOI/NCB.
                                                         Note that DPI uses DPI_NCB_CTL[NCBSEL_REQ] for reading DPI_DMA_INSTR_HDR_S's
                                                         and first/last pointers, not [NCBSEL]. DPI_NCB_CTL[NCBSEL_SRC,NCBSEL_DST]
                                                         are the equivalent of [NCBSEL] for DPI_HDR_XTYPE_E::INTERNAL_ONLY DPI DMA
                                                         instructions.
                                                         A typical configuration might be:
                                                         _ DPI_SLI_PRT(0)_CFG[NCBSEL]=1.
                                                         _ DPI_SLI_PRT(1)_CFG[NCBSEL]=1.
                                                         _ DPI_SLI_PRT(2)_CFG[NCBSEL]=0.
                                                         _ DPI_SLI_PRT(3)_CFG[NCBSEL]=0. */
	uint64_t reserved_25_27               : 3;
	uint64_t halt                         : 1;  /**< When set, HALT indicates that the MAC has detected a reset condition. No further
                                                         instructions that reference the MAC from any instruction queue will be issued until the
                                                         MAC comes out of reset and HALT is cleared in SLI_CTL_PORT()[DIS_PORT]. */
	uint64_t qlm_cfg                      : 4;  /**< Read only copy of the QLM CFG.
                                                         0 = MAC is PCIe.
                                                         1 = MAC is SRIO.
                                                         QLM_CFG does not indicate if a port is disabled.  GSER()_CFG can be used for
                                                         more complete QLM configuration information. */
	uint64_t reserved_17_19               : 3;
	uint64_t rd_mode                      : 1;  /**< Read mode.
                                                         0 = Exact read mode. The hardware reads on a four-byte granularity. In this mode, the
                                                         hardware may break a given read into 3 operations to satisfy DPI rules.
                                                         1 = Block mode. The hardware reads more data than requested in order to minimize the
                                                         number of operations necessary to complete the operation. The memory region must be memory
                                                         like. */
	uint64_t reserved_15_15               : 1;
	uint64_t molr                         : 7;  /**< Maximum outstanding load requests. Limits the number of outstanding load requests on the
                                                         port by restricting the number of tags used by the SLI to track load responses. This value
                                                         can range from 1 to 64 (maximum of 32 if port not configured for 8 lanes). Setting MOLR
                                                         to a value of 0 halts all read traffic to the port. There are no restrictions on when this
                                                         value can be changed. */
	uint64_t mps_lim                      : 1;  /**< MAC memory space write requests cannot cross the (naturally-aligned) MPS boundary.
                                                         When clear, DPI is allowed to issue a MAC memory- space read that crosses the naturally-
                                                         aligned boundary of size defined by MPS. (DPI will still only cross the boundary when it
                                                         would eliminate a write by doing so.)
                                                         When set, DPI will never issue a MAC memory space write that crosses the naturally-aligned
                                                         boundary of size defined by MPS.
                                                         MPS_LIM must be set when DPI is operating in SRIO mode (QLM_CFG=SRIO). */
	uint64_t reserved_5_6                 : 2;
	uint64_t mps                          : 1;  /**< Maximum payload size.
                                                         0 = 128B.
                                                         1 = 256B.
                                                         The MPS size must not exceed the size selected by PCIE*_CFG030[MPS] when DPI is
                                                         operating in PCIe mode (QLM_CFG=PCIe).
                                                         For CNF75XX, when DPI is operating in SRIO mode (QLM_CFG=SRIO), the MPS size
                                                         must not exceed 256B. */
	uint64_t mrrs_lim                     : 1;  /**< MAC memory space read requests cannot cross the (naturally-aligned) MRRS boundary.
                                                         When clear, DPI is allowed to issue a MAC memory-space read that crosses the naturally-
                                                         aligned boundary of size defined by MRRS. (DPI will still only cross the boundary when it
                                                         would eliminate a read by doing so.)
                                                         When set, DPI will never issue a MAC memory space read that crosses the naturally-aligned
                                                         boundary of size defined by MRRS.
                                                         MRRS_LIM must be set when DPI is operating in SRIO mode (QLM_CFG=SRIO). */
	uint64_t reserved_2_2                 : 1;
	uint64_t mrrs                         : 2;  /**< Maximum read-request size.
                                                         0x0 = 128B.
                                                         0x1 = 256B.
                                                         0x2 = 512B.
                                                         0x3 = 1024B.
                                                         The MRRS size must not exceed the size selected by PCIE*_CFG030[MRRS] when DPI
                                                         is operating in PCIe mode (QLM_CFG=PCIe).
                                                         For CNF75XX, when DPI is operating in SRIO mode (QLM_CFG=SRIO), the MRRS size
                                                         must not exceed 256B. */
#else
	uint64_t mrrs                         : 2;
	uint64_t reserved_2_2                 : 1;
	uint64_t mrrs_lim                     : 1;
	uint64_t mps                          : 1;
	uint64_t reserved_5_6                 : 2;
	uint64_t mps_lim                      : 1;
	uint64_t molr                         : 7;
	uint64_t reserved_15_15               : 1;
	uint64_t rd_mode                      : 1;
	uint64_t reserved_17_19               : 3;
	uint64_t qlm_cfg                      : 4;
	uint64_t halt                         : 1;
	uint64_t reserved_25_27               : 3;
	uint64_t ncbsel                       : 1;
	uint64_t reserved_29_63               : 35;
#endif
	} s;
	struct cvmx_dpi_sli_prtx_cfg_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_25_63               : 39;
	uint64_t halt                         : 1;  /**< When set, HALT indicates that the MAC has detected
                                                         a reset condition. No further instructions that
                                                         reference the MAC from any instruction Q will be
                                                         issued until the MAC comes out of reset and HALT
                                                         is cleared in SLI_CTL_PORTx[DIS_PORT]. */
	uint64_t qlm_cfg                      : 4;  /**< QLM_CFG is a function of MIO_QLMx_CFG[QLM_CFG]
                                                         QLM_CFG may contain values that are not normally
                                                         used for DMA and/or packet operations.
                                                         QLM_CFG does not indicate if a port is disabled.
                                                         MIO_QLMx_CFG can be used for more complete QLM
                                                         configuration information.
                                                         0000 = MAC is PCIe 1x4 (QLM) or 1x2 (DLM)
                                                         0001 = MAC is PCIe 2x1 (DLM only)
                                                         0010 = MAC is SGMII
                                                         0011 = MAC is XAUI
                                                         all other encodings are RESERVED */
	uint64_t reserved_17_19               : 3;
	uint64_t rd_mode                      : 1;  /**< Read Mode
                                                         0=Exact Read Mode
                                                           If the port is a PCIe port, the HW reads on a
                                                           4B granularity.  In this mode, the HW may break
                                                           a given read into 3 operations to satisify
                                                           PCIe rules.
                                                           If the port is a SRIO port, the HW follows the
                                                           SRIO read rules from the SRIO specification and
                                                            only issues 32*n, 16, and 8 byte  operations
                                                            on the SRIO bus.
                                                         1=Block Mode
                                                           The HW will read more data than requested in
                                                           order to minimize the number of operations
                                                           necessary to complete the operation.
                                                           The memory region must be memory like. */
	uint64_t reserved_14_15               : 2;
	uint64_t molr                         : 6;  /**< Max Outstanding Load Requests
                                                         Limits the number of oustanding load requests on
                                                         the port by restricting the number of tags
                                                         used by the SLI to track load responses.  This
                                                         value can range from 1 to 32 depending on the MAC
                                                         type and number of lanes.
                                                         MAC == PCIe:           Max is 32
                                                         MAC == sRio / 4 lanes: Max is 32
                                                         MAC == sRio / 2 lanes: Max is 16
                                                         MAC == sRio / 1 lane:  Max is  8
                                                         Reset value is computed based on the MAC config.
                                                         Setting MOLR to a value of 0 will halt all read
                                                         traffic to the port.  There are no restrictions
                                                         on when this value can be changed. */
	uint64_t mps_lim                      : 1;  /**< MAC memory space write requests cannot cross the
                                                         (naturally-aligned) MPS boundary.
                                                         When clear, DPI is allowed to issue a MAC memory
                                                         space read that crosses the naturally-aligned
                                                         boundary of size defined by MPS. (DPI will still
                                                         only cross the boundary when it would eliminate a
                                                         write by doing so.)
                                                         When set, DPI will never issue a MAC memory space
                                                         write that crosses the naturally-aligned boundary
                                                         of size defined by MPS. */
	uint64_t reserved_5_6                 : 2;
	uint64_t mps                          : 1;  /**< Max Payload Size
                                                                 0 = 128B
                                                                 1 = 256B
                                                         For PCIe MACs, this MPS size must not exceed
                                                               the size selected by PCIE*_CFG030[MPS].
                                                         For sRIO MACs, all MPS values are allowed. */
	uint64_t mrrs_lim                     : 1;  /**< MAC memory space read requests cannot cross the
                                                         (naturally-aligned) MRRS boundary.
                                                         When clear, DPI is allowed to issue a MAC memory
                                                         space read that crosses the naturally-aligned
                                                         boundary of size defined by MRRS. (DPI will still
                                                         only cross the boundary when it would eliminate a
                                                         read by doing so.)
                                                         When set, DPI will never issue a MAC memory space
                                                         read that crosses the naturally-aligned boundary
                                                         of size defined by MRRS. */
	uint64_t reserved_2_2                 : 1;
	uint64_t mrrs                         : 2;  /**< Max Read Request Size
                                                                 0 = 128B
                                                                 1 = 256B
                                                                 2 = 512B
                                                                 3 = 1024B
                                                         For PCIe MACs, this MRRS size must not exceed
                                                               the size selected by PCIE*_CFG030[MRRS].
                                                         For sRIO MACs, this MRRS size must be <= 256B. */
#else
	uint64_t mrrs                         : 2;
	uint64_t reserved_2_2                 : 1;
	uint64_t mrrs_lim                     : 1;
	uint64_t mps                          : 1;
	uint64_t reserved_5_6                 : 2;
	uint64_t mps_lim                      : 1;
	uint64_t molr                         : 6;
	uint64_t reserved_14_15               : 2;
	uint64_t rd_mode                      : 1;
	uint64_t reserved_17_19               : 3;
	uint64_t qlm_cfg                      : 4;
	uint64_t halt                         : 1;
	uint64_t reserved_25_63               : 39;
#endif
	} cn61xx;
	struct cvmx_dpi_sli_prtx_cfg_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_25_63               : 39;
	uint64_t halt                         : 1;  /**< When set, HALT indicates that the MAC has detected
                                                         a reset condition. No further instructions that
                                                         reference the MAC from any instruction Q will be
                                                         issued until the MAC comes out of reset and HALT
                                                         is cleared in SLI_CTL_PORTx[DIS_PORT]. */
	uint64_t reserved_21_23               : 3;
	uint64_t qlm_cfg                      : 1;  /**< Read only copy of the QLM CFG pin
                                                         Since QLM_CFG is simply a copy of the QLM CFG
                                                         pins, it may reflect values that are not normal
                                                         for DMA or packet operations. QLM_CFG does not
                                                         indicate if a port is disabled.
                                                         0= MAC is PCIe
                                                         1= MAC is SRIO */
	uint64_t reserved_17_19               : 3;
	uint64_t rd_mode                      : 1;  /**< Read Mode
                                                         0=Exact Read Mode
                                                           If the port is a PCIe port, the HW reads on a
                                                           4B granularity.  In this mode, the HW may break
                                                           a given read into 3 operations to satisify
                                                           PCIe rules.
                                                           If the port is a SRIO port, the HW follows the
                                                           SRIO read rules from the SRIO specification and
                                                            only issues 32*n, 16, and 8 byte  operations
                                                            on the SRIO bus.
                                                         1=Block Mode
                                                           The HW will read more data than requested in
                                                           order to minimize the number of operations
                                                           necessary to complete the operation.
                                                           The memory region must be memory like. */
	uint64_t reserved_14_15               : 2;
	uint64_t molr                         : 6;  /**< Max Outstanding Load Requests
                                                         Limits the number of oustanding load requests on
                                                         the port by restricting the number of tags
                                                         used by the SLI to track load responses.  This
                                                         value can range from 1 to 32. Setting a value of
                                                         0 will halt all read traffic to the port.  There
                                                         are no restrictions on when this value
                                                         can be changed. */
	uint64_t mps_lim                      : 1;  /**< MAC memory space write requests cannot cross the
                                                         (naturally-aligned) MPS boundary.
                                                         When clear, DPI is allowed to issue a MAC memory
                                                         space read that crosses the naturally-aligned
                                                         boundary of size defined by MPS. (DPI will still
                                                         only cross the boundary when it would eliminate a
                                                         write by doing so.)
                                                         When set, DPI will never issue a MAC memory space
                                                         write that crosses the naturally-aligned boundary
                                                         of size defined by MPS. */
	uint64_t reserved_5_6                 : 2;
	uint64_t mps                          : 1;  /**< Max Payload Size
                                                                 0 = 128B
                                                                 1 = 256B
                                                         For PCIe MACs, this MPS size must not exceed
                                                               the size selected by PCIE*_CFG030[MPS].
                                                         For sRIO MACs, all MPS values are allowed. */
	uint64_t mrrs_lim                     : 1;  /**< MAC memory space read requests cannot cross the
                                                         (naturally-aligned) MRRS boundary.
                                                         When clear, DPI is allowed to issue a MAC memory
                                                         space read that crosses the naturally-aligned
                                                         boundary of size defined by MRRS. (DPI will still
                                                         only cross the boundary when it would eliminate a
                                                         read by doing so.)
                                                         When set, DPI will never issue a MAC memory space
                                                         read that crosses the naturally-aligned boundary
                                                         of size defined by MRRS. */
	uint64_t reserved_2_2                 : 1;
	uint64_t mrrs                         : 2;  /**< Max Read Request Size
                                                                 0 = 128B
                                                                 1 = 256B
                                                                 2 = 512B
                                                                 3 = 1024B
                                                         For PCIe MACs, this MRRS size must not exceed
                                                               the size selected by PCIE*_CFG030[MRRS].
                                                         For sRIO MACs, this MRRS size must be <= 256B. */
#else
	uint64_t mrrs                         : 2;
	uint64_t reserved_2_2                 : 1;
	uint64_t mrrs_lim                     : 1;
	uint64_t mps                          : 1;
	uint64_t reserved_5_6                 : 2;
	uint64_t mps_lim                      : 1;
	uint64_t molr                         : 6;
	uint64_t reserved_14_15               : 2;
	uint64_t rd_mode                      : 1;
	uint64_t reserved_17_19               : 3;
	uint64_t qlm_cfg                      : 1;
	uint64_t reserved_21_23               : 3;
	uint64_t halt                         : 1;
	uint64_t reserved_25_63               : 39;
#endif
	} cn63xx;
	struct cvmx_dpi_sli_prtx_cfg_cn63xx   cn63xxp1;
	struct cvmx_dpi_sli_prtx_cfg_cn61xx   cn66xx;
	struct cvmx_dpi_sli_prtx_cfg_cn63xx   cn68xx;
	struct cvmx_dpi_sli_prtx_cfg_cn63xx   cn68xxp1;
	struct cvmx_dpi_sli_prtx_cfg_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_25_63               : 39;
	uint64_t halt                         : 1;  /**< When set, HALT indicates that the MAC has detected
                                                         a reset condition. No further instructions that
                                                         reference the MAC from any instruction Q will be
                                                         issued until the MAC comes out of reset and HALT
                                                         is cleared in SLI_CTL_PORTx[DIS_PORT]. */
	uint64_t reserved_17_23               : 7;
	uint64_t rd_mode                      : 1;  /**< Read Mode
                                                         0=Exact Read Mode
                                                         If the port is a PCIe port, the HW reads on a
                                                         4B granularity.  In this mode, the HW may break
                                                         a given read into 3 operations to satisify
                                                         PCIe rules. */
	uint64_t reserved_14_15               : 2;
	uint64_t molr                         : 6;  /**< Max Outstanding Load Requests
                                                         Limits the number of oustanding load requests on
                                                         the port by restricting the number of tags
                                                         used by the SLI to track load responses.  This
                                                         value can range from 1 to 32 depending on the MAC
                                                         type and number of lanes.
                                                         MAC == PCIe / 4 lanes: Max is 32
                                                         MAC == PCIe / 2 lanes: Max is 16
                                                         MAC == PCIe / 1 lane:  Max is  8
                                                         Reset value is computed based on the MAC config.
                                                         Setting MOLR to a value of 0 will halt all read
                                                         traffic to the port.  There are no restrictions
                                                         on when this value can be changed. */
	uint64_t mps_lim                      : 1;  /**< MAC memory space write requests cannot cross the
                                                         (naturally-aligned) MPS boundary.
                                                         When clear, DPI is allowed to issue a MAC memory
                                                         space read that crosses the naturally-aligned
                                                         boundary of size defined by MPS. (DPI will still
                                                         only cross the boundary when it would eliminate a
                                                         write by doing so.)
                                                         When set, DPI will never issue a MAC memory space
                                                         write that crosses the naturally-aligned boundary
                                                         of size defined by MPS. */
	uint64_t reserved_5_6                 : 2;
	uint64_t mps                          : 1;  /**< Max Payload Size
                                                         0 = 128B
                                                         1 = 256B
                                                         For PCIe MACs, this MPS size must not exceed
                                                         the size selected by PCIE*_CFG030[MPS]. */
	uint64_t mrrs_lim                     : 1;  /**< MAC memory space read requests cannot cross the
                                                         (naturally-aligned) MRRS boundary.
                                                         When clear, DPI is allowed to issue a MAC memory
                                                         space read that crosses the naturally-aligned
                                                         boundary of size defined by MRRS. (DPI will still
                                                         only cross the boundary when it would eliminate a
                                                         read by doing so.)
                                                         When set, DPI will never issue a MAC memory space
                                                         read that crosses the naturally-aligned boundary
                                                         of size defined by MRRS. */
	uint64_t reserved_2_2                 : 1;
	uint64_t mrrs                         : 2;  /**< Max Read Request Size
                                                         0 = 128B
                                                         1 = 256B
                                                         2 = 512B
                                                         3 = 1024B
                                                         For PCIe MACs, this MRRS size must not exceed
                                                         the size selected by PCIE*_CFG030[MRRS]. */
#else
	uint64_t mrrs                         : 2;
	uint64_t reserved_2_2                 : 1;
	uint64_t mrrs_lim                     : 1;
	uint64_t mps                          : 1;
	uint64_t reserved_5_6                 : 2;
	uint64_t mps_lim                      : 1;
	uint64_t molr                         : 6;
	uint64_t reserved_14_15               : 2;
	uint64_t rd_mode                      : 1;
	uint64_t reserved_17_23               : 7;
	uint64_t halt                         : 1;
	uint64_t reserved_25_63               : 39;
#endif
	} cn70xx;
	struct cvmx_dpi_sli_prtx_cfg_cn70xx   cn70xxp1;
	struct cvmx_dpi_sli_prtx_cfg_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_29_63               : 35;
	uint64_t ncbsel                       : 1;  /**< The default IOI/NCB that DPI uses for DPI_HDR_XTYPE_E::OUTBOUND L2/DRAM data
                                                         reads and DPI_HDR_XTYPE_E::INBOUND L2/DRAM writes. DPI_DMA_INSTR_HDR_S[LPORT]
                                                         selects which DPI_SLI_PRT()_CFG[NCBSEL] is used for a DPI DMA instruction.
                                                         In the normal case (DPI_NCB_CTL[NCBSEL_PRT_XOR_DIS] clear),
                                                         _   [NCBSEL] XOR DPI_DMA_INSTR_HDR_S[FPORT<0>]
                                                         determines the IOI/NCB that DPI uses.
                                                         0 = DPI uses IOBI2/IOBO2 (aka NCBI2/NCBO2) for DPI_HDR_XTYPE_E::OUTBOUND and
                                                             DPI_HDR_XTYPE_E::INBOUND DPI DMA instructions.
                                                         1 = DPI uses IOBI3/IOBO3 (aka NCBI3/NCBO3) for DPI_HDR_XTYPE_E::OUTBOUND and
                                                             DPI_HDR_XTYPE_E::INBOUND DPI DMA instructions.
                                                         If DPI_NCB_CTL[NCBSEL_PRT_XOR_DIS] is set, [NCBSEL] solely determines the IOI/NCB.
                                                         Note that DPI uses DPI_NCB_CTL[NCBSEL_REQ] for reading DPI_DMA_INSTR_HDR_S's
                                                         and first/last pointers, not [NCBSEL]. DPI_NCB_CTL[NCBSEL_SRC,NCBSEL_DST]
                                                         are the equivalent of [NCBSEL] for DPI_HDR_XTYPE_E::INTERNAL_ONLY DPI DMA
                                                         instructions.
                                                         A typical configuration might be:
                                                         _ DPI_SLI_PRT(0)_CFG[NCBSEL]=1.
                                                         _ DPI_SLI_PRT(1)_CFG[NCBSEL]=1.
                                                         _ DPI_SLI_PRT(2)_CFG[NCBSEL]=0.
                                                         _ DPI_SLI_PRT(3)_CFG[NCBSEL]=0. */
	uint64_t reserved_25_27               : 3;
	uint64_t halt                         : 1;  /**< When set, HALT indicates that the MAC has detected a reset condition. No further
                                                         instructions that reference the MAC from any instruction queue will be issued until the
                                                         MAC comes out of reset and HALT is cleared in SLI_CTL_PORT()[DIS_PORT]. */
	uint64_t reserved_21_23               : 3;
	uint64_t qlm_cfg                      : 1;  /**< Read only copy of the QLM CFG pin.
                                                         0 = MAC is DPI.
                                                         1 = Reserved.
                                                         Since QLM_CFG is simply a copy of the QLM CFG pins, it could reflect values that are not
                                                         normal for DMA or packet operations. QLM_CFG does not indicate if a port is disabled.
                                                         GSER()_CFG can be used for more complete QLM configuration information. */
	uint64_t reserved_17_19               : 3;
	uint64_t rd_mode                      : 1;  /**< Read mode.
                                                         0 = Exact read mode. The hardware reads on a four-byte granularity. In this mode, the
                                                         hardware may break a given read into 3 operations to satisfy DPI rules.
                                                         1 = Block mode. The hardware reads more data than requested in order to minimize the
                                                         number of operations necessary to complete the operation. The memory region must be memory
                                                         like. */
	uint64_t reserved_15_15               : 1;
	uint64_t molr                         : 7;  /**< Maximum outstanding load requests. Limits the number of outstanding load requests on the
                                                         port by restricting the number of tags used by the SLI to track load responses. This value
                                                         can range from 1 to 64 (maximum of 32 if port not configured for 8 lanes). Setting MOLR
                                                         to a value of 0 halts all read traffic to the port. There are no restrictions on when this
                                                         value can be changed. */
	uint64_t mps_lim                      : 1;  /**< MAC memory space write requests cannot cross the (naturally-aligned) MPS boundary.
                                                         When clear, DPI is allowed to issue a MAC memory- space read that crosses the naturally-
                                                         aligned boundary of size defined by MPS. (DPI will still only cross the boundary when it
                                                         would eliminate a write by doing so.)
                                                         When set, DPI will never issue a MAC memory space write that crosses the naturally-aligned
                                                         boundary of size defined by MPS. */
	uint64_t reserved_5_6                 : 2;
	uint64_t mps                          : 1;  /**< Maximum payload size.
                                                         0 = 128B.
                                                         1 = 256B.
                                                         The MPS size must not exceed the size selected by PCIE*_CFG030[MPS]. */
	uint64_t mrrs_lim                     : 1;  /**< MAC memory space read requests cannot cross the (naturally-aligned) MRRS boundary.
                                                         When clear, DPI is allowed to issue a MAC memory-space read that crosses the naturally-
                                                         aligned boundary of size defined by MRRS. (DPI will still only cross the boundary when it
                                                         would eliminate a read by doing so.)
                                                         When set, DPI will never issue a MAC memory space read that crosses the naturally-aligned
                                                         boundary of size defined by MRRS. */
	uint64_t reserved_2_2                 : 1;
	uint64_t mrrs                         : 2;  /**< Maximum read-request size.
                                                         0x0 = 128B.
                                                         0x1 = 256B.
                                                         0x2 = 512B.
                                                         0x3 = 1024B.
                                                         The MRRS size must not exceed the size selected by PCIE*_CFG030[MRRS]. */
#else
	uint64_t mrrs                         : 2;
	uint64_t reserved_2_2                 : 1;
	uint64_t mrrs_lim                     : 1;
	uint64_t mps                          : 1;
	uint64_t reserved_5_6                 : 2;
	uint64_t mps_lim                      : 1;
	uint64_t molr                         : 7;
	uint64_t reserved_15_15               : 1;
	uint64_t rd_mode                      : 1;
	uint64_t reserved_17_19               : 3;
	uint64_t qlm_cfg                      : 1;
	uint64_t reserved_21_23               : 3;
	uint64_t halt                         : 1;
	uint64_t reserved_25_27               : 3;
	uint64_t ncbsel                       : 1;
	uint64_t reserved_29_63               : 35;
#endif
	} cn73xx;
	struct cvmx_dpi_sli_prtx_cfg_cn73xx   cn78xx;
	struct cvmx_dpi_sli_prtx_cfg_cn73xx   cn78xxp1;
	struct cvmx_dpi_sli_prtx_cfg_cn61xx   cnf71xx;
	struct cvmx_dpi_sli_prtx_cfg_cn73xx   cnf75xx;
};
typedef union cvmx_dpi_sli_prtx_cfg cvmx_dpi_sli_prtx_cfg_t;

/**
 * cvmx_dpi_sli_prt#_err
 *
 * This register logs the address associated with the reported SLI error response.
 * Indexed by SLI_PORT_E.
 */
union cvmx_dpi_sli_prtx_err {
	uint64_t u64;
	struct cvmx_dpi_sli_prtx_err_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t addr                         : 61; /**< Address of the failed load request. Address is locked along with the
                                                         DPI_SLI_PRT()_ERR_INFO register. See the
                                                         DPI_SLI_PRT()_ERR_INFO[LOCK] description for further information. */
	uint64_t reserved_0_2                 : 3;
#else
	uint64_t reserved_0_2                 : 3;
	uint64_t addr                         : 61;
#endif
	} s;
	struct cvmx_dpi_sli_prtx_err_s        cn61xx;
	struct cvmx_dpi_sli_prtx_err_s        cn63xx;
	struct cvmx_dpi_sli_prtx_err_s        cn63xxp1;
	struct cvmx_dpi_sli_prtx_err_s        cn66xx;
	struct cvmx_dpi_sli_prtx_err_s        cn68xx;
	struct cvmx_dpi_sli_prtx_err_s        cn68xxp1;
	struct cvmx_dpi_sli_prtx_err_s        cn70xx;
	struct cvmx_dpi_sli_prtx_err_s        cn70xxp1;
	struct cvmx_dpi_sli_prtx_err_s        cn73xx;
	struct cvmx_dpi_sli_prtx_err_s        cn78xx;
	struct cvmx_dpi_sli_prtx_err_s        cn78xxp1;
	struct cvmx_dpi_sli_prtx_err_s        cnf71xx;
	struct cvmx_dpi_sli_prtx_err_s        cnf75xx;
};
typedef union cvmx_dpi_sli_prtx_err cvmx_dpi_sli_prtx_err_t;

/**
 * cvmx_dpi_sli_prt#_err_info
 *
 * This register logs information associated with the reported SLI error response.
 * Indexed by SLI_PORT_E.
 */
union cvmx_dpi_sli_prtx_err_info {
	uint64_t u64;
	struct cvmx_dpi_sli_prtx_err_info_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t lock                         : 1;  /**< DPI_SLI_PRT()_ERR and DPI_SLI_PRT()_ERR_INFO have captured and locked contents.
                                                         When CNXXXX first detects an ErrorResponse, the TYPE, REQQ, and ADDR of the error is saved
                                                         and an internal lock state is set so the data associated with the initial error is
                                                         preserved.
                                                         Subsequent ErrorResponses will optionally raise an interrupt, but will not modify the
                                                         TYPE, REQQ, or ADDR fields until the internal lock state is cleared.
                                                         Software can clear the internal lock state by writing a 1 to the appropriate bit in either
                                                         DPI_REQ_ERR_RSP or DPI_PKT_ERR_RSP depending on the TYPE field. Once the internal lock
                                                         state is cleared, the next ErrorResponse will set the TYPE, REQQ, and ADDR for the new
                                                         transaction. */
	uint64_t reserved_5_7                 : 3;
	uint64_t type                         : 1;  /**< Type of transaction that caused the ErrorResponse.
                                                         0 = DMA Instruction.
                                                         1 = PKT Instruction. */
	uint64_t reserved_3_3                 : 1;
	uint64_t reqq                         : 3;  /**< Request queue that made the failed load request. */
#else
	uint64_t reqq                         : 3;
	uint64_t reserved_3_3                 : 1;
	uint64_t type                         : 1;
	uint64_t reserved_5_7                 : 3;
	uint64_t lock                         : 1;
	uint64_t reserved_9_63                : 55;
#endif
	} s;
	struct cvmx_dpi_sli_prtx_err_info_s   cn61xx;
	struct cvmx_dpi_sli_prtx_err_info_s   cn63xx;
	struct cvmx_dpi_sli_prtx_err_info_s   cn63xxp1;
	struct cvmx_dpi_sli_prtx_err_info_s   cn66xx;
	struct cvmx_dpi_sli_prtx_err_info_s   cn68xx;
	struct cvmx_dpi_sli_prtx_err_info_s   cn68xxp1;
	struct cvmx_dpi_sli_prtx_err_info_s   cn70xx;
	struct cvmx_dpi_sli_prtx_err_info_s   cn70xxp1;
	struct cvmx_dpi_sli_prtx_err_info_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t pvf                          : 16; /**< Physical/virtual function that caused the ErrorResponse. DPI_DMA_FUNC_SEL_S format. */
	uint64_t reserved_9_15                : 7;
	uint64_t lock                         : 1;  /**< DPI_SLI_PRT()_ERR and DPI_SLI_PRT()_ERR_INFO have captured and locked contents.
                                                         When CNXXXX first detects an ErrorResponse, the TYPE, REQQ, and ADDR of the error is saved
                                                         and an internal lock state is set so the data associated with the initial error is
                                                         preserved.
                                                         Subsequent ErrorResponses will optionally raise an interrupt, but will not modify the
                                                         TYPE, REQQ, or ADDR fields until the internal lock state is cleared.
                                                         Software can clear the internal lock state by writing a 1 to the appropriate bit in either
                                                         DPI_REQ_ERR_RSP or DPI_PKT_ERR_RSP depending on the TYPE field. Once the internal lock
                                                         state is cleared, the next ErrorResponse will set the TYPE, REQQ, and ADDR for the new
                                                         transaction. */
	uint64_t reserved_5_7                 : 3;
	uint64_t type                         : 1;  /**< Type of transaction that caused the ErrorResponse.
                                                         0 = DMA Instruction.
                                                         1 = PKT Instruction. */
	uint64_t reserved_3_3                 : 1;
	uint64_t reqq                         : 3;  /**< Request queue that made the failed load request. */
#else
	uint64_t reqq                         : 3;
	uint64_t reserved_3_3                 : 1;
	uint64_t type                         : 1;
	uint64_t reserved_5_7                 : 3;
	uint64_t lock                         : 1;
	uint64_t reserved_9_15                : 7;
	uint64_t pvf                          : 16;
	uint64_t reserved_32_63               : 32;
#endif
	} cn73xx;
	struct cvmx_dpi_sli_prtx_err_info_cn73xx cn78xx;
	struct cvmx_dpi_sli_prtx_err_info_cn78xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_23_63               : 41;
	uint64_t vf                           : 7;  /**< Virtual function that caused the ErrorResponse. */
	uint64_t reserved_9_15                : 7;
	uint64_t lock                         : 1;  /**< DPI_SLI_PRT()_ERR and DPI_SLI_PRT()_ERR_INFO have captured and locked contents.
                                                         When CNXXXX first detects an ErrorResponse, the TYPE, REQQ, and ADDR of the error is saved
                                                         and an internal lock state is set so the data associated with the initial error is
                                                         preserved.
                                                         Subsequent ErrorResponses will optionally raise an interrupt, but will not modify the
                                                         TYPE, REQQ, or ADDR fields until the internal lock state is cleared.
                                                         Software can clear the internal lock state by writing a 1 to the appropriate bit in either
                                                         DPI_REQ_ERR_RSP or DPI_PKT_ERR_RSP depending on the TYPE field. Once the internal lock
                                                         state is cleared, the next ErrorResponse will set the TYPE, REQQ, and ADDR for the new
                                                         transaction. */
	uint64_t reserved_5_7                 : 3;
	uint64_t type                         : 1;  /**< Type of transaction that caused the ErrorResponse.
                                                         0 = DMA Instruction.
                                                         1 = PKT Instruction. */
	uint64_t reserved_3_3                 : 1;
	uint64_t reqq                         : 3;  /**< Request queue that made the failed load request. */
#else
	uint64_t reqq                         : 3;
	uint64_t reserved_3_3                 : 1;
	uint64_t type                         : 1;
	uint64_t reserved_5_7                 : 3;
	uint64_t lock                         : 1;
	uint64_t reserved_9_15                : 7;
	uint64_t vf                           : 7;
	uint64_t reserved_23_63               : 41;
#endif
	} cn78xxp1;
	struct cvmx_dpi_sli_prtx_err_info_s   cnf71xx;
	struct cvmx_dpi_sli_prtx_err_info_cn73xx cnf75xx;
};
typedef union cvmx_dpi_sli_prtx_err_info cvmx_dpi_sli_prtx_err_info_t;

/**
 * cvmx_dpi_srio_rx_bell#
 *
 * Reading this register pops an entry off the corresponding SRIO RX doorbell FIFO.
 * The chip supports 16 FIFOs per SRIO interface for a total of 32 FIFOs/Registers.
 * The MSB of the registers indicates the MAC while the 4 LSBs indicate the FIFO.
 * Information on the doorbell allocation can be found in SRIO()_RX_BELL_CTRL.
 */
union cvmx_dpi_srio_rx_bellx {
	uint64_t u64;
	struct cvmx_dpi_srio_rx_bellx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t data                         : 16; /**< Information field from received doorbell. */
	uint64_t sid                          : 16; /**< Doorbell source device ID <15:0>. */
	uint64_t count                        : 8;  /**< RX bell FIFO count.
                                                         Count must be > 0x0 for entry to be valid. */
	uint64_t reserved_5_7                 : 3;
	uint64_t dest_id                      : 1;  /**< Destination device ID. 0=Primary, 1=Secondary. */
	uint64_t id16                         : 1;  /**< Transaction type. 0=use ID[7:0], 1=use ID[15:0]. */
	uint64_t reserved_2_2                 : 1;
	uint64_t dpriority                    : 2;  /**< Doorbell priority. */
#else
	uint64_t dpriority                    : 2;
	uint64_t reserved_2_2                 : 1;
	uint64_t id16                         : 1;
	uint64_t dest_id                      : 1;
	uint64_t reserved_5_7                 : 3;
	uint64_t count                        : 8;
	uint64_t sid                          : 16;
	uint64_t data                         : 16;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_dpi_srio_rx_bellx_s       cnf75xx;
};
typedef union cvmx_dpi_srio_rx_bellx cvmx_dpi_srio_rx_bellx_t;

/**
 * cvmx_dpi_srio_rx_bell_seq#
 *
 * This register contains the value of the sequence counter when the doorbell
 * was received and a shadow copy of the Bell FIFO Count that can be read without
 * emptying the FIFO.  This register must be read prior to corresponding
 * DPI_SRIO_RX_BELL register to link the doorbell and sequence number.
 *
 * Information on the Doorbell Allocation can be found in SRIO()_RX_BELL_CTRL.
 */
union cvmx_dpi_srio_rx_bell_seqx {
	uint64_t u64;
	struct cvmx_dpi_srio_rx_bell_seqx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_40_63               : 24;
	uint64_t count                        : 8;  /**< FIFO count. */
	uint64_t sid                          : 32; /**< Sequence. */
#else
	uint64_t sid                          : 32;
	uint64_t count                        : 8;
	uint64_t reserved_40_63               : 24;
#endif
	} s;
	struct cvmx_dpi_srio_rx_bell_seqx_s   cnf75xx;
};
typedef union cvmx_dpi_srio_rx_bell_seqx cvmx_dpi_srio_rx_bell_seqx_t;

/**
 * cvmx_dpi_swa_q_vmid
 *
 * Not used.
 *
 */
union cvmx_dpi_swa_q_vmid {
	uint64_t u64;
	struct cvmx_dpi_swa_q_vmid_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t vmid7                        : 8;  /**< The SWA VMID for queue 7. */
	uint64_t vmid6                        : 8;  /**< The SWA VMID for queue 6. */
	uint64_t vmid5                        : 8;  /**< The SWA VMID for queue 5. */
	uint64_t vmid4                        : 8;  /**< The SWA VMID for queue 4. */
	uint64_t vmid3                        : 8;  /**< The SWA VMID for queue 3. */
	uint64_t vmid2                        : 8;  /**< The SWA VMID for queue 2. */
	uint64_t vmid1                        : 8;  /**< The SWA VMID for queue 1. */
	uint64_t vmid0                        : 8;  /**< The SWA VMID for queue 0. */
#else
	uint64_t vmid0                        : 8;
	uint64_t vmid1                        : 8;
	uint64_t vmid2                        : 8;
	uint64_t vmid3                        : 8;
	uint64_t vmid4                        : 8;
	uint64_t vmid5                        : 8;
	uint64_t vmid6                        : 8;
	uint64_t vmid7                        : 8;
#endif
	} s;
	struct cvmx_dpi_swa_q_vmid_s          cn73xx;
	struct cvmx_dpi_swa_q_vmid_s          cn78xx;
	struct cvmx_dpi_swa_q_vmid_s          cn78xxp1;
	struct cvmx_dpi_swa_q_vmid_s          cnf75xx;
};
typedef union cvmx_dpi_swa_q_vmid cvmx_dpi_swa_q_vmid_t;

#endif
