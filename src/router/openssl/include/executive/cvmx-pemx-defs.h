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
 * cvmx-pemx-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon pemx.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_PEMX_DEFS_H__
#define __CVMX_PEMX_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PEMX_BAR1_INDEXX(unsigned long offset, unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if (((offset <= 15)) && ((block_id <= 1)))
				return CVMX_ADD_IO_SEG(0x00011800C00000A8ull) + (((offset) & 15) + ((block_id) & 1) * 0x200000ull) * 8;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if (((offset <= 15)) && ((block_id <= 2)))
				return CVMX_ADD_IO_SEG(0x00011800C0000100ull) + (((offset) & 15) + ((block_id) & 3) * 0x200000ull) * 8;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (((offset <= 15)) && ((block_id <= 3)))
				return CVMX_ADD_IO_SEG(0x00011800C0000100ull) + (((offset) & 15) + ((block_id) & 3) * 0x200000ull) * 8;
			break;
	}
	cvmx_warn("CVMX_PEMX_BAR1_INDEXX (%lu, %lu) not supported on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800C00000A8ull) + (((offset) & 15) + ((block_id) & 1) * 0x200000ull) * 8;
}
#else
static inline uint64_t CVMX_PEMX_BAR1_INDEXX(unsigned long offset, unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C00000A8ull) + ((offset) + (block_id) * 0x200000ull) * 8;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C0000100ull) + ((offset) + (block_id) * 0x200000ull) * 8;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C0000100ull) + ((offset) + (block_id) * 0x200000ull) * 8;
	}
	return CVMX_ADD_IO_SEG(0x00011800C00000A8ull) + ((offset) + (block_id) * 0x200000ull) * 8;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PEMX_BAR2_MASK(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return CVMX_ADD_IO_SEG(0x00011800C0000130ull) + ((block_id) & 1) * 0x1000000ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return CVMX_ADD_IO_SEG(0x00011800C00000B0ull) + ((block_id) & 3) * 0x1000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return CVMX_ADD_IO_SEG(0x00011800C00000B0ull) + ((block_id) & 3) * 0x1000000ull;
			break;
	}
	cvmx_warn("CVMX_PEMX_BAR2_MASK (block_id = %lu) not supported on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800C0000130ull) + ((block_id) & 1) * 0x1000000ull;
}
#else
static inline uint64_t CVMX_PEMX_BAR2_MASK(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C0000130ull) + (block_id) * 0x1000000ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C00000B0ull) + (block_id) * 0x1000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C00000B0ull) + (block_id) * 0x1000000ull;
	}
	return CVMX_ADD_IO_SEG(0x00011800C0000130ull) + (block_id) * 0x1000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PEMX_BAR_CTL(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return CVMX_ADD_IO_SEG(0x00011800C0000128ull) + ((block_id) & 1) * 0x1000000ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return CVMX_ADD_IO_SEG(0x00011800C00000A8ull) + ((block_id) & 3) * 0x1000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return CVMX_ADD_IO_SEG(0x00011800C00000A8ull) + ((block_id) & 3) * 0x1000000ull;
			break;
	}
	cvmx_warn("CVMX_PEMX_BAR_CTL (block_id = %lu) not supported on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800C0000128ull) + ((block_id) & 1) * 0x1000000ull;
}
#else
static inline uint64_t CVMX_PEMX_BAR_CTL(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C0000128ull) + (block_id) * 0x1000000ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C00000A8ull) + (block_id) * 0x1000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C00000A8ull) + (block_id) * 0x1000000ull;
	}
	return CVMX_ADD_IO_SEG(0x00011800C0000128ull) + (block_id) * 0x1000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PEMX_BIST_STATUS(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return CVMX_ADD_IO_SEG(0x00011800C0000018ull) + ((block_id) & 1) * 0x1000000ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return CVMX_ADD_IO_SEG(0x00011800C0000018ull) + ((block_id) & 3) * 0x1000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return CVMX_ADD_IO_SEG(0x00011800C0000440ull) + ((block_id) & 3) * 0x1000000ull;
			break;
	}
	cvmx_warn("CVMX_PEMX_BIST_STATUS (block_id = %lu) not supported on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800C0000018ull) + ((block_id) & 1) * 0x1000000ull;
}
#else
static inline uint64_t CVMX_PEMX_BIST_STATUS(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C0000018ull) + (block_id) * 0x1000000ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C0000018ull) + (block_id) * 0x1000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C0000440ull) + (block_id) * 0x1000000ull;
	}
	return CVMX_ADD_IO_SEG(0x00011800C0000018ull) + (block_id) * 0x1000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PEMX_BIST_STATUS2(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return CVMX_ADD_IO_SEG(0x00011800C0000420ull) + ((block_id) & 1) * 0x1000000ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return CVMX_ADD_IO_SEG(0x00011800C0000440ull) + ((block_id) & 3) * 0x1000000ull;
			break;
	}
	cvmx_warn("CVMX_PEMX_BIST_STATUS2 (block_id = %lu) not supported on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800C0000420ull) + ((block_id) & 1) * 0x1000000ull;
}
#else
static inline uint64_t CVMX_PEMX_BIST_STATUS2(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C0000420ull) + (block_id) * 0x1000000ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C0000440ull) + (block_id) * 0x1000000ull;
	}
	return CVMX_ADD_IO_SEG(0x00011800C0000420ull) + (block_id) * 0x1000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PEMX_CFG(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((block_id <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PEMX_CFG(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800C0000410ull) + ((block_id) & 3) * 0x1000000ull;
}
#else
#define CVMX_PEMX_CFG(block_id) (CVMX_ADD_IO_SEG(0x00011800C0000410ull) + ((block_id) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PEMX_CFG_RD(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((block_id <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((block_id <= 1)))))
		cvmx_warn("CVMX_PEMX_CFG_RD(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800C0000030ull) + ((block_id) & 3) * 0x1000000ull;
}
#else
#define CVMX_PEMX_CFG_RD(block_id) (CVMX_ADD_IO_SEG(0x00011800C0000030ull) + ((block_id) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PEMX_CFG_WR(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((block_id <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((block_id <= 1)))))
		cvmx_warn("CVMX_PEMX_CFG_WR(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800C0000028ull) + ((block_id) & 3) * 0x1000000ull;
}
#else
#define CVMX_PEMX_CFG_WR(block_id) (CVMX_ADD_IO_SEG(0x00011800C0000028ull) + ((block_id) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PEMX_CLK_EN(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((block_id <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PEMX_CLK_EN(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800C0000400ull) + ((block_id) & 3) * 0x1000000ull;
}
#else
#define CVMX_PEMX_CLK_EN(block_id) (CVMX_ADD_IO_SEG(0x00011800C0000400ull) + ((block_id) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PEMX_CPL_LUT_VALID(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((block_id <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((block_id <= 1)))))
		cvmx_warn("CVMX_PEMX_CPL_LUT_VALID(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800C0000098ull) + ((block_id) & 3) * 0x1000000ull;
}
#else
#define CVMX_PEMX_CPL_LUT_VALID(block_id) (CVMX_ADD_IO_SEG(0x00011800C0000098ull) + ((block_id) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PEMX_CTL_STATUS(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((block_id <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((block_id <= 1)))))
		cvmx_warn("CVMX_PEMX_CTL_STATUS(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800C0000000ull) + ((block_id) & 3) * 0x1000000ull;
}
#else
#define CVMX_PEMX_CTL_STATUS(block_id) (CVMX_ADD_IO_SEG(0x00011800C0000000ull) + ((block_id) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PEMX_CTL_STATUS2(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PEMX_CTL_STATUS2(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800C0000008ull) + ((block_id) & 3) * 0x1000000ull;
}
#else
#define CVMX_PEMX_CTL_STATUS2(block_id) (CVMX_ADD_IO_SEG(0x00011800C0000008ull) + ((block_id) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PEMX_DBG_INFO(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return CVMX_ADD_IO_SEG(0x00011800C0000008ull) + ((block_id) & 1) * 0x1000000ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return CVMX_ADD_IO_SEG(0x00011800C00000D0ull) + ((block_id) & 3) * 0x1000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return CVMX_ADD_IO_SEG(0x00011800C00000D0ull) + ((block_id) & 3) * 0x1000000ull;
			break;
	}
	cvmx_warn("CVMX_PEMX_DBG_INFO (block_id = %lu) not supported on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800C0000008ull) + ((block_id) & 1) * 0x1000000ull;
}
#else
static inline uint64_t CVMX_PEMX_DBG_INFO(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C0000008ull) + (block_id) * 0x1000000ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C00000D0ull) + (block_id) * 0x1000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C00000D0ull) + (block_id) * 0x1000000ull;
	}
	return CVMX_ADD_IO_SEG(0x00011800C0000008ull) + (block_id) * 0x1000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PEMX_DBG_INFO_EN(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((block_id <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((block_id <= 1)))))
		cvmx_warn("CVMX_PEMX_DBG_INFO_EN(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800C00000A0ull) + ((block_id) & 3) * 0x1000000ull;
}
#else
#define CVMX_PEMX_DBG_INFO_EN(block_id) (CVMX_ADD_IO_SEG(0x00011800C00000A0ull) + ((block_id) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PEMX_DIAG_STATUS(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((block_id <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((block_id <= 1)))))
		cvmx_warn("CVMX_PEMX_DIAG_STATUS(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800C0000020ull) + ((block_id) & 3) * 0x1000000ull;
}
#else
#define CVMX_PEMX_DIAG_STATUS(block_id) (CVMX_ADD_IO_SEG(0x00011800C0000020ull) + ((block_id) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PEMX_ECC_ENA(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return CVMX_ADD_IO_SEG(0x00011800C00000C0ull) + ((block_id) & 3) * 0x1000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return CVMX_ADD_IO_SEG(0x00011800C0000448ull) + ((block_id) & 3) * 0x1000000ull;
			break;
	}
	cvmx_warn("CVMX_PEMX_ECC_ENA (block_id = %lu) not supported on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800C0000448ull) + ((block_id) & 3) * 0x1000000ull;
}
#else
static inline uint64_t CVMX_PEMX_ECC_ENA(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C00000C0ull) + (block_id) * 0x1000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C0000448ull) + (block_id) * 0x1000000ull;
	}
	return CVMX_ADD_IO_SEG(0x00011800C0000448ull) + (block_id) * 0x1000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PEMX_ECC_SYND_CTRL(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return CVMX_ADD_IO_SEG(0x00011800C00000C8ull) + ((block_id) & 3) * 0x1000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return CVMX_ADD_IO_SEG(0x00011800C0000450ull) + ((block_id) & 3) * 0x1000000ull;
			break;
	}
	cvmx_warn("CVMX_PEMX_ECC_SYND_CTRL (block_id = %lu) not supported on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800C0000450ull) + ((block_id) & 3) * 0x1000000ull;
}
#else
static inline uint64_t CVMX_PEMX_ECC_SYND_CTRL(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C00000C8ull) + (block_id) * 0x1000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C0000450ull) + (block_id) * 0x1000000ull;
	}
	return CVMX_ADD_IO_SEG(0x00011800C0000450ull) + (block_id) * 0x1000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PEMX_INB_READ_CREDITS(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return CVMX_ADD_IO_SEG(0x00011800C0000138ull) + ((block_id) & 1) * 0x1000000ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return CVMX_ADD_IO_SEG(0x00011800C00000B8ull) + ((block_id) & 3) * 0x1000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return CVMX_ADD_IO_SEG(0x00011800C00000B8ull) + ((block_id) & 3) * 0x1000000ull;
			break;
	}
	cvmx_warn("CVMX_PEMX_INB_READ_CREDITS (block_id = %lu) not supported on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800C0000138ull) + ((block_id) & 1) * 0x1000000ull;
}
#else
static inline uint64_t CVMX_PEMX_INB_READ_CREDITS(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C0000138ull) + (block_id) * 0x1000000ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C00000B8ull) + (block_id) * 0x1000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C00000B8ull) + (block_id) * 0x1000000ull;
	}
	return CVMX_ADD_IO_SEG(0x00011800C0000138ull) + (block_id) * 0x1000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PEMX_INT_ENB(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return CVMX_ADD_IO_SEG(0x00011800C0000410ull) + ((block_id) & 1) * 0x1000000ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return CVMX_ADD_IO_SEG(0x00011800C0000430ull) + ((block_id) & 3) * 0x1000000ull;
			break;
	}
	cvmx_warn("CVMX_PEMX_INT_ENB (block_id = %lu) not supported on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800C0000410ull) + ((block_id) & 1) * 0x1000000ull;
}
#else
static inline uint64_t CVMX_PEMX_INT_ENB(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C0000410ull) + (block_id) * 0x1000000ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C0000430ull) + (block_id) * 0x1000000ull;
	}
	return CVMX_ADD_IO_SEG(0x00011800C0000410ull) + (block_id) * 0x1000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PEMX_INT_ENB_INT(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return CVMX_ADD_IO_SEG(0x00011800C0000418ull) + ((block_id) & 1) * 0x1000000ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return CVMX_ADD_IO_SEG(0x00011800C0000438ull) + ((block_id) & 3) * 0x1000000ull;
			break;
	}
	cvmx_warn("CVMX_PEMX_INT_ENB_INT (block_id = %lu) not supported on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800C0000418ull) + ((block_id) & 1) * 0x1000000ull;
}
#else
static inline uint64_t CVMX_PEMX_INT_ENB_INT(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C0000418ull) + (block_id) * 0x1000000ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C0000438ull) + (block_id) * 0x1000000ull;
	}
	return CVMX_ADD_IO_SEG(0x00011800C0000418ull) + (block_id) * 0x1000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PEMX_INT_SUM(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 1))
				return CVMX_ADD_IO_SEG(0x00011800C0000408ull) + ((block_id) & 1) * 0x1000000ull;
			break;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 2))
				return CVMX_ADD_IO_SEG(0x00011800C0000428ull) + ((block_id) & 3) * 0x1000000ull;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if ((block_id <= 3))
				return CVMX_ADD_IO_SEG(0x00011800C0000428ull) + ((block_id) & 3) * 0x1000000ull;
			break;
	}
	cvmx_warn("CVMX_PEMX_INT_SUM (block_id = %lu) not supported on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800C0000408ull) + ((block_id) & 1) * 0x1000000ull;
}
#else
static inline uint64_t CVMX_PEMX_INT_SUM(unsigned long block_id)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C0000408ull) + (block_id) * 0x1000000ull;
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C0000428ull) + (block_id) * 0x1000000ull;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00011800C0000428ull) + (block_id) * 0x1000000ull;
	}
	return CVMX_ADD_IO_SEG(0x00011800C0000408ull) + (block_id) * 0x1000000ull;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PEMX_ON(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((block_id <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PEMX_ON(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800C0000420ull) + ((block_id) & 3) * 0x1000000ull;
}
#else
#define CVMX_PEMX_ON(block_id) (CVMX_ADD_IO_SEG(0x00011800C0000420ull) + ((block_id) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PEMX_P2N_BAR0_START(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((block_id <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((block_id <= 1)))))
		cvmx_warn("CVMX_PEMX_P2N_BAR0_START(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800C0000080ull) + ((block_id) & 3) * 0x1000000ull;
}
#else
#define CVMX_PEMX_P2N_BAR0_START(block_id) (CVMX_ADD_IO_SEG(0x00011800C0000080ull) + ((block_id) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PEMX_P2N_BAR1_START(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((block_id <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((block_id <= 1)))))
		cvmx_warn("CVMX_PEMX_P2N_BAR1_START(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800C0000088ull) + ((block_id) & 3) * 0x1000000ull;
}
#else
#define CVMX_PEMX_P2N_BAR1_START(block_id) (CVMX_ADD_IO_SEG(0x00011800C0000088ull) + ((block_id) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PEMX_P2N_BAR2_START(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((block_id <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((block_id <= 1)))))
		cvmx_warn("CVMX_PEMX_P2N_BAR2_START(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800C0000090ull) + ((block_id) & 3) * 0x1000000ull;
}
#else
#define CVMX_PEMX_P2N_BAR2_START(block_id) (CVMX_ADD_IO_SEG(0x00011800C0000090ull) + ((block_id) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PEMX_P2P_BARX_END(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && (((offset <= 3)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && (((offset <= 3)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && (((offset <= 3)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 3))))))
		cvmx_warn("CVMX_PEMX_P2P_BARX_END(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800C0000048ull) + (((offset) & 3) + ((block_id) & 3) * 0x100000ull) * 16;
}
#else
#define CVMX_PEMX_P2P_BARX_END(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800C0000048ull) + (((offset) & 3) + ((block_id) & 3) * 0x100000ull) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PEMX_P2P_BARX_START(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && (((offset <= 3)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && (((offset <= 3)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && (((offset <= 3)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 3))))))
		cvmx_warn("CVMX_PEMX_P2P_BARX_START(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800C0000040ull) + (((offset) & 3) + ((block_id) & 3) * 0x100000ull) * 16;
}
#else
#define CVMX_PEMX_P2P_BARX_START(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800C0000040ull) + (((offset) & 3) + ((block_id) & 3) * 0x100000ull) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PEMX_QLM(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PEMX_QLM(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800C0000418ull) + ((block_id) & 3) * 0x1000000ull;
}
#else
#define CVMX_PEMX_QLM(block_id) (CVMX_ADD_IO_SEG(0x00011800C0000418ull) + ((block_id) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PEMX_SPI_CTL(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((block_id <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PEMX_SPI_CTL(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800C0000180ull) + ((block_id) & 3) * 0x1000000ull;
}
#else
#define CVMX_PEMX_SPI_CTL(block_id) (CVMX_ADD_IO_SEG(0x00011800C0000180ull) + ((block_id) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PEMX_SPI_DATA(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((block_id <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PEMX_SPI_DATA(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800C0000188ull) + ((block_id) & 3) * 0x1000000ull;
}
#else
#define CVMX_PEMX_SPI_DATA(block_id) (CVMX_ADD_IO_SEG(0x00011800C0000188ull) + ((block_id) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PEMX_STRAP(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((block_id <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3)))))
		cvmx_warn("CVMX_PEMX_STRAP(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800C0000408ull) + ((block_id) & 3) * 0x1000000ull;
}
#else
#define CVMX_PEMX_STRAP(block_id) (CVMX_ADD_IO_SEG(0x00011800C0000408ull) + ((block_id) & 3) * 0x1000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_PEMX_TLP_CREDITS(unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((block_id <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((block_id <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((block_id <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((block_id <= 1)))))
		cvmx_warn("CVMX_PEMX_TLP_CREDITS(%lu) is invalid on this chip\n", block_id);
	return CVMX_ADD_IO_SEG(0x00011800C0000038ull) + ((block_id) & 3) * 0x1000000ull;
}
#else
#define CVMX_PEMX_TLP_CREDITS(block_id) (CVMX_ADD_IO_SEG(0x00011800C0000038ull) + ((block_id) & 3) * 0x1000000ull)
#endif

/**
 * cvmx_pem#_bar1_index#
 *
 * Contains address index and control bits for access to memory ranges of BAR-1. Index is build
 * from supplied address [25:22].
 */
union cvmx_pemx_bar1_indexx {
	uint64_t u64;
	struct cvmx_pemx_bar1_indexx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t addr_idx                     : 20; /**< Address bits [37:22] sent to L2C */
	uint64_t ca                           : 1;  /**< Set '1' when access is not to be cached in L2. */
	uint64_t end_swp                      : 2;  /**< Endian Swap Mode */
	uint64_t addr_v                       : 1;  /**< Set '1' when the selected address range is valid. */
#else
	uint64_t addr_v                       : 1;
	uint64_t end_swp                      : 2;
	uint64_t ca                           : 1;
	uint64_t addr_idx                     : 20;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_pemx_bar1_indexx_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t addr_idx                     : 16; /**< Address bits [37:22] sent to L2C */
	uint64_t ca                           : 1;  /**< Set '1' when access is not to be cached in L2. */
	uint64_t end_swp                      : 2;  /**< Endian Swap Mode */
	uint64_t addr_v                       : 1;  /**< Set '1' when the selected address range is valid. */
#else
	uint64_t addr_v                       : 1;
	uint64_t end_swp                      : 2;
	uint64_t ca                           : 1;
	uint64_t addr_idx                     : 16;
	uint64_t reserved_20_63               : 44;
#endif
	} cn61xx;
	struct cvmx_pemx_bar1_indexx_cn61xx   cn63xx;
	struct cvmx_pemx_bar1_indexx_cn61xx   cn63xxp1;
	struct cvmx_pemx_bar1_indexx_cn61xx   cn66xx;
	struct cvmx_pemx_bar1_indexx_cn61xx   cn68xx;
	struct cvmx_pemx_bar1_indexx_cn61xx   cn68xxp1;
	struct cvmx_pemx_bar1_indexx_s        cn70xx;
	struct cvmx_pemx_bar1_indexx_s        cn78xx;
	struct cvmx_pemx_bar1_indexx_cn61xx   cnf71xx;
};
typedef union cvmx_pemx_bar1_indexx cvmx_pemx_bar1_indexx_t;

/**
 * cvmx_pem#_bar2_mask
 *
 * The mask pattern that is ANDED with the address from PCIe core for BAR2 hits.
 *
 */
union cvmx_pemx_bar2_mask {
	uint64_t u64;
	struct cvmx_pemx_bar2_mask_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_42_63               : 22;
	uint64_t mask                         : 39; /**< The value to be ANDED with the address sent to
                                                         the Octeon memory. */
	uint64_t reserved_0_2                 : 3;
#else
	uint64_t reserved_0_2                 : 3;
	uint64_t mask                         : 39;
	uint64_t reserved_42_63               : 22;
#endif
	} s;
	struct cvmx_pemx_bar2_mask_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_38_63               : 26;
	uint64_t mask                         : 35; /**< The value to be ANDED with the address sent to
                                                         the Octeon memory. */
	uint64_t reserved_0_2                 : 3;
#else
	uint64_t reserved_0_2                 : 3;
	uint64_t mask                         : 35;
	uint64_t reserved_38_63               : 26;
#endif
	} cn61xx;
	struct cvmx_pemx_bar2_mask_cn61xx     cn66xx;
	struct cvmx_pemx_bar2_mask_cn61xx     cn68xx;
	struct cvmx_pemx_bar2_mask_cn61xx     cn68xxp1;
	struct cvmx_pemx_bar2_mask_cn61xx     cn70xx;
	struct cvmx_pemx_bar2_mask_s          cn78xx;
	struct cvmx_pemx_bar2_mask_cn61xx     cnf71xx;
};
typedef union cvmx_pemx_bar2_mask cvmx_pemx_bar2_mask_t;

/**
 * cvmx_pem#_bar_ctl
 *
 * Contains control for BAR accesses.
 *
 */
union cvmx_pemx_bar_ctl {
	uint64_t u64;
	struct cvmx_pemx_bar_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t bar1_siz                     : 3;  /**< Pcie-Port0, Bar1 Size. 1 == 64MB, 2 == 128MB,
                                                         3 == 256MB, 4 == 512MB, 5 == 1024MB, 6 == 2048MB,
                                                         0 and 7 are reserved. */
	uint64_t bar2_enb                     : 1;  /**< When set '1' BAR2 is enable and will respond when
                                                         clear '0' BAR2 access will cause UR responses. */
	uint64_t bar2_esx                     : 2;  /**< Value will be XORed with pci-address[39:38] to
                                                         determine the endian swap mode. */
	uint64_t bar2_cax                     : 1;  /**< Value will be XORed with pcie-address[40] to
                                                         determine the L2 cache attribute.
                                                         Not cached in L2 if XOR result is 1 */
#else
	uint64_t bar2_cax                     : 1;
	uint64_t bar2_esx                     : 2;
	uint64_t bar2_enb                     : 1;
	uint64_t bar1_siz                     : 3;
	uint64_t reserved_7_63                : 57;
#endif
	} s;
	struct cvmx_pemx_bar_ctl_s            cn61xx;
	struct cvmx_pemx_bar_ctl_s            cn63xx;
	struct cvmx_pemx_bar_ctl_s            cn63xxp1;
	struct cvmx_pemx_bar_ctl_s            cn66xx;
	struct cvmx_pemx_bar_ctl_s            cn68xx;
	struct cvmx_pemx_bar_ctl_s            cn68xxp1;
	struct cvmx_pemx_bar_ctl_s            cn70xx;
	struct cvmx_pemx_bar_ctl_s            cn78xx;
	struct cvmx_pemx_bar_ctl_s            cnf71xx;
};
typedef union cvmx_pemx_bar_ctl cvmx_pemx_bar_ctl_t;

/**
 * cvmx_pem#_bist_status
 *
 * "PEM#_BIST_STATUS2 = PEM BIST Status Register
 * Results from BIST runs of PEM's memories."
 */
union cvmx_pemx_bist_status {
	uint64_t u64;
	struct cvmx_pemx_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_26_63               : 38;
	uint64_t retryc                       : 1;  /**< Retry Buffer Memory C */
	uint64_t reserved_24_24               : 1;
	uint64_t rqhdrb0                      : 1;  /**< Rx Queue Header Memory Buffer 0 */
	uint64_t rqhdrb1                      : 1;  /**< Rx Queue Header Memory Buffer 1 */
	uint64_t rqdatab0                     : 1;  /**< Rx Queue Data Buffer 0 */
	uint64_t rqdatab1                     : 1;  /**< Rx Queue Data Buffer 1 */
	uint64_t tlpan_d0                     : 1;  /**< BIST Status for the tlp_n_afifo_data0 */
	uint64_t tlpan_d1                     : 1;  /**< BIST Status for the tlp_n_afifo_data1 */
	uint64_t tlpan_ctl                    : 1;  /**< BIST Status for the tlp_n_afifo_ctl */
	uint64_t tlpap_d0                     : 1;  /**< BIST Status for the tlp_p_afifo_data0 */
	uint64_t tlpap_d1                     : 1;  /**< BIST Status for the tlp_p_afifo_data1 */
	uint64_t tlpap_ctl                    : 1;  /**< BIST Status for the tlp_p_afifo_ctl */
	uint64_t tlpac_d0                     : 1;  /**< BIST Status for the tlp_c_afifo_data0 */
	uint64_t tlpac_d1                     : 1;  /**< BIST Status for the tlp_c_afifo_data1 */
	uint64_t tlpac_ctl                    : 1;  /**< BIST Status for the tlp_c_afifo_ctl */
	uint64_t peai_p2e                     : 1;  /**< BIST Status for the peai__pesc_fifo */
	uint64_t tlpn_d0                      : 1;  /**< BIST Status for the tlp_n_fifo_data0 */
	uint64_t tlpn_d1                      : 1;  /**< BIST Status for the tlp_n_fifo_data1 */
	uint64_t reserved_0_7                 : 8;
#else
	uint64_t reserved_0_7                 : 8;
	uint64_t tlpn_d1                      : 1;
	uint64_t tlpn_d0                      : 1;
	uint64_t peai_p2e                     : 1;
	uint64_t tlpac_ctl                    : 1;
	uint64_t tlpac_d1                     : 1;
	uint64_t tlpac_d0                     : 1;
	uint64_t tlpap_ctl                    : 1;
	uint64_t tlpap_d1                     : 1;
	uint64_t tlpap_d0                     : 1;
	uint64_t tlpan_ctl                    : 1;
	uint64_t tlpan_d1                     : 1;
	uint64_t tlpan_d0                     : 1;
	uint64_t rqdatab1                     : 1;
	uint64_t rqdatab0                     : 1;
	uint64_t rqhdrb1                      : 1;
	uint64_t rqhdrb0                      : 1;
	uint64_t reserved_24_24               : 1;
	uint64_t retryc                       : 1;
	uint64_t reserved_26_63               : 38;
#endif
	} s;
	struct cvmx_pemx_bist_status_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t retry                        : 1;  /**< Retry Buffer. */
	uint64_t rqdata0                      : 1;  /**< Rx Queue Data Memory0. */
	uint64_t rqdata1                      : 1;  /**< Rx Queue Data Memory1. */
	uint64_t rqdata2                      : 1;  /**< Rx Queue Data Memory2. */
	uint64_t rqdata3                      : 1;  /**< Rx Queue Data Memory3. */
	uint64_t rqhdr1                       : 1;  /**< Rx Queue Header1. */
	uint64_t rqhdr0                       : 1;  /**< Rx Queue Header0. */
	uint64_t sot                          : 1;  /**< SOT Buffer. */
#else
	uint64_t sot                          : 1;
	uint64_t rqhdr0                       : 1;
	uint64_t rqhdr1                       : 1;
	uint64_t rqdata3                      : 1;
	uint64_t rqdata2                      : 1;
	uint64_t rqdata1                      : 1;
	uint64_t rqdata0                      : 1;
	uint64_t retry                        : 1;
	uint64_t reserved_8_63                : 56;
#endif
	} cn61xx;
	struct cvmx_pemx_bist_status_cn61xx   cn63xx;
	struct cvmx_pemx_bist_status_cn61xx   cn63xxp1;
	struct cvmx_pemx_bist_status_cn61xx   cn66xx;
	struct cvmx_pemx_bist_status_cn61xx   cn68xx;
	struct cvmx_pemx_bist_status_cn61xx   cn68xxp1;
	struct cvmx_pemx_bist_status_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t retry                        : 1;  /**< Retry Buffer Memory */
	uint64_t sot                          : 1;  /**< Start of Transfer Memory */
	uint64_t rqhdr0                       : 1;  /**< Rx Queue Header Memory 0 */
	uint64_t rqhdr1                       : 1;  /**< Rx Queue Header Memory 1 */
	uint64_t rqdata0                      : 1;  /**< Rx Queue Data Memory 0 */
	uint64_t rqdata1                      : 1;  /**< Rx Queue Data Memory 1 */
#else
	uint64_t rqdata1                      : 1;
	uint64_t rqdata0                      : 1;
	uint64_t rqhdr1                       : 1;
	uint64_t rqhdr0                       : 1;
	uint64_t sot                          : 1;
	uint64_t retry                        : 1;
	uint64_t reserved_6_63                : 58;
#endif
	} cn70xx;
	struct cvmx_pemx_bist_status_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_26_63               : 38;
	uint64_t retryc                       : 1;  /**< Retry Buffer Memory C */
	uint64_t sot                          : 1;  /**< Start of Transfer Memory */
	uint64_t rqhdrb0                      : 1;  /**< Rx Queue Header Memory Buffer 0 */
	uint64_t rqhdrb1                      : 1;  /**< Rx Queue Header Memory Buffer 1 */
	uint64_t rqdatab0                     : 1;  /**< Rx Queue Data Buffer 0 */
	uint64_t rqdatab1                     : 1;  /**< Rx Queue Data Buffer 1 */
	uint64_t tlpan_d0                     : 1;  /**< BIST Status for the tlp_n_afifo_data0 */
	uint64_t tlpan_d1                     : 1;  /**< BIST Status for the tlp_n_afifo_data1 */
	uint64_t tlpan_ctl                    : 1;  /**< BIST Status for the tlp_n_afifo_ctl */
	uint64_t tlpap_d0                     : 1;  /**< BIST Status for the tlp_p_afifo_data0 */
	uint64_t tlpap_d1                     : 1;  /**< BIST Status for the tlp_p_afifo_data1 */
	uint64_t tlpap_ctl                    : 1;  /**< BIST Status for the tlp_p_afifo_ctl */
	uint64_t tlpac_d0                     : 1;  /**< BIST Status for the tlp_c_afifo_data0 */
	uint64_t tlpac_d1                     : 1;  /**< BIST Status for the tlp_c_afifo_data1 */
	uint64_t tlpac_ctl                    : 1;  /**< BIST Status for the tlp_c_afifo_ctl */
	uint64_t peai_p2e                     : 1;  /**< BIST Status for the peai__pesc_fifo */
	uint64_t tlpn_d0                      : 1;  /**< BIST Status for the tlp_n_fifo_data0 */
	uint64_t tlpn_d1                      : 1;  /**< BIST Status for the tlp_n_fifo_data1 */
	uint64_t tlpn_ctl                     : 1;  /**< BIST Status for the tlp_n_fifo_ctl */
	uint64_t tlpp_d0                      : 1;  /**< BIST Status for the tlp_p_fifo_data0 */
	uint64_t tlpp_d1                      : 1;  /**< BIST Status for the tlp_p_fifo_data1 */
	uint64_t tlpp_ctl                     : 1;  /**< BIST Status for the tlp_p_fifo_ctl */
	uint64_t tlpc_d0                      : 1;  /**< BIST Status for the tlp_c_fifo_data0 */
	uint64_t tlpc_d1                      : 1;  /**< BIST Status for the tlp_c_fifo_data1 */
	uint64_t tlpc_ctl                     : 1;  /**< BIST Status for the tlp_c_fifo_ctl */
	uint64_t m2s                          : 1;  /**< BIST Status for the m2s_fifo */
#else
	uint64_t m2s                          : 1;
	uint64_t tlpc_ctl                     : 1;
	uint64_t tlpc_d1                      : 1;
	uint64_t tlpc_d0                      : 1;
	uint64_t tlpp_ctl                     : 1;
	uint64_t tlpp_d1                      : 1;
	uint64_t tlpp_d0                      : 1;
	uint64_t tlpn_ctl                     : 1;
	uint64_t tlpn_d1                      : 1;
	uint64_t tlpn_d0                      : 1;
	uint64_t peai_p2e                     : 1;
	uint64_t tlpac_ctl                    : 1;
	uint64_t tlpac_d1                     : 1;
	uint64_t tlpac_d0                     : 1;
	uint64_t tlpap_ctl                    : 1;
	uint64_t tlpap_d1                     : 1;
	uint64_t tlpap_d0                     : 1;
	uint64_t tlpan_ctl                    : 1;
	uint64_t tlpan_d1                     : 1;
	uint64_t tlpan_d0                     : 1;
	uint64_t rqdatab1                     : 1;
	uint64_t rqdatab0                     : 1;
	uint64_t rqhdrb1                      : 1;
	uint64_t rqhdrb0                      : 1;
	uint64_t sot                          : 1;
	uint64_t retryc                       : 1;
	uint64_t reserved_26_63               : 38;
#endif
	} cn78xx;
	struct cvmx_pemx_bist_status_cn61xx   cnf71xx;
};
typedef union cvmx_pemx_bist_status cvmx_pemx_bist_status_t;

/**
 * cvmx_pem#_bist_status2
 *
 * "PEM#_BIST_STATUS2 = PEM BIST Status Register
 * Results from BIST runs of PEM's memories."
 */
union cvmx_pemx_bist_status2 {
	uint64_t u64;
	struct cvmx_pemx_bist_status2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_13_63               : 51;
	uint64_t tlpn_d                       : 1;  /**< BIST Status for the tlp_n_fifo_data */
	uint64_t tlpn_ctl                     : 1;  /**< BIST Status for the tlp_n_fifo_ctl */
	uint64_t tlpp_d                       : 1;  /**< BIST Status for the tlp_p_fifo_data */
	uint64_t reserved_0_9                 : 10;
#else
	uint64_t reserved_0_9                 : 10;
	uint64_t tlpp_d                       : 1;
	uint64_t tlpn_ctl                     : 1;
	uint64_t tlpn_d                       : 1;
	uint64_t reserved_13_63               : 51;
#endif
	} s;
	struct cvmx_pemx_bist_status2_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t e2p_cpl                      : 1;  /**< BIST Status for the e2p_cpl_fifo */
	uint64_t e2p_n                        : 1;  /**< BIST Status for the e2p_n_fifo */
	uint64_t e2p_p                        : 1;  /**< BIST Status for the e2p_p_fifo */
	uint64_t peai_p2e                     : 1;  /**< BIST Status for the peai__pesc_fifo */
	uint64_t pef_tpf1                     : 1;  /**< BIST Status for the pef_tlp_p_fifo1 */
	uint64_t pef_tpf0                     : 1;  /**< BIST Status for the pef_tlp_p_fifo0 */
	uint64_t pef_tnf                      : 1;  /**< BIST Status for the pef_tlp_n_fifo */
	uint64_t pef_tcf1                     : 1;  /**< BIST Status for the pef_tlp_cpl_fifo1 */
	uint64_t pef_tc0                      : 1;  /**< BIST Status for the pef_tlp_cpl_fifo0 */
	uint64_t ppf                          : 1;  /**< BIST Status for the ppf_fifo */
#else
	uint64_t ppf                          : 1;
	uint64_t pef_tc0                      : 1;
	uint64_t pef_tcf1                     : 1;
	uint64_t pef_tnf                      : 1;
	uint64_t pef_tpf0                     : 1;
	uint64_t pef_tpf1                     : 1;
	uint64_t peai_p2e                     : 1;
	uint64_t e2p_p                        : 1;
	uint64_t e2p_n                        : 1;
	uint64_t e2p_cpl                      : 1;
	uint64_t reserved_10_63               : 54;
#endif
	} cn61xx;
	struct cvmx_pemx_bist_status2_cn61xx  cn63xx;
	struct cvmx_pemx_bist_status2_cn61xx  cn63xxp1;
	struct cvmx_pemx_bist_status2_cn61xx  cn66xx;
	struct cvmx_pemx_bist_status2_cn61xx  cn68xx;
	struct cvmx_pemx_bist_status2_cn61xx  cn68xxp1;
	struct cvmx_pemx_bist_status2_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t peai_p2e                     : 1;  /**< BIST Status for the peai__pesc_fifo */
	uint64_t tlpn_d                       : 1;  /**< BIST Status for the tlp_n_fifo_data */
	uint64_t tlpn_ctl                     : 1;  /**< BIST Status for the tlp_n_fifo_ctl */
	uint64_t tlpp_d                       : 1;  /**< BIST Status for the tlp_p_fifo_data */
	uint64_t tlpp_ctl                     : 1;  /**< BIST Status for the tlp_p_fifo_ctl */
	uint64_t tlpc_d                       : 1;  /**< BIST Status for the tlp_c_fifo_data */
	uint64_t tlpc_ctl                     : 1;  /**< BIST Status for the tlp_c_fifo_ctl */
	uint64_t tlpan_d                      : 1;  /**< BIST Status for the tlp_n_afifo_data */
	uint64_t tlpan_ctl                    : 1;  /**< BIST Status for the tlp_n_afifo_ctl */
	uint64_t tlpap_d                      : 1;  /**< BIST Status for the tlp_p_afifo_data */
	uint64_t tlpap_ctl                    : 1;  /**< BIST Status for the tlp_p_afifo_ctl */
	uint64_t tlpac_d                      : 1;  /**< BIST Status for the tlp_c_afifo_data */
	uint64_t tlpac_ctl                    : 1;  /**< BIST Status for the tlp_c_afifo_ctl */
	uint64_t m2s                          : 1;  /**< BIST Status for the m2s_fifo */
#else
	uint64_t m2s                          : 1;
	uint64_t tlpac_ctl                    : 1;
	uint64_t tlpac_d                      : 1;
	uint64_t tlpap_ctl                    : 1;
	uint64_t tlpap_d                      : 1;
	uint64_t tlpan_ctl                    : 1;
	uint64_t tlpan_d                      : 1;
	uint64_t tlpc_ctl                     : 1;
	uint64_t tlpc_d                       : 1;
	uint64_t tlpp_ctl                     : 1;
	uint64_t tlpp_d                       : 1;
	uint64_t tlpn_ctl                     : 1;
	uint64_t tlpn_d                       : 1;
	uint64_t peai_p2e                     : 1;
	uint64_t reserved_14_63               : 50;
#endif
	} cn70xx;
	struct cvmx_pemx_bist_status2_cn61xx  cnf71xx;
};
typedef union cvmx_pemx_bist_status2 cvmx_pemx_bist_status2_t;

/**
 * cvmx_pem#_cfg
 *
 * Configuration of the PCIe Application
 *
 */
union cvmx_pemx_cfg {
	uint64_t u64;
	struct cvmx_pemx_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t laneswap                     : 1;  /**< This field will overwrite the pin setting for lane swapping.
                                                         When set, lane swapping is performed to/from the SerDes.
                                                         When clear, no lane swapping is performed. */
	uint64_t reserved_2_3                 : 2;
	uint64_t md                           : 2;  /**< This field will overwrite the pin settings for speed.
                                                         00 - EP Mode, Gen1 Speed
                                                         01 - EP Mode, Gen2 Speed
                                                         10 - EP Mode, Gen3 Speed
                                                         11 - Rsvd */
#else
	uint64_t md                           : 2;
	uint64_t reserved_2_3                 : 2;
	uint64_t laneswap                     : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_pemx_cfg_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t laneswap                     : 1;  /**< This field will overwrite the pin setting for lane swapping.
                                                         When set, lane swapping is performed to/from the SerDes.
                                                         When clear, no lane swapping is performed. */
	uint64_t hostmd                       : 1;  /**< This field will overwrite the pin settings for host mode.
                                                         When set, the PEM is configured to be a Root Complex.
                                                         When clear, the PEM is configured to be an End Point. */
	uint64_t md                           : 3;  /**< This field will overwrite the pin settings for speed and lane
                                                         configuration. This value is used to set the Maximum Link Width
                                                         field in the core's Link Capabilities Register (CFG031) to
                                                         indicate the maximum number of lanes supported. Note that less
                                                         lanes than the specified maximum can be configured for use via
                                                         the core's Link Control Register (CFG032) Negotiated Link Width
                                                         field.
                                                         NOTE - The lower two bits of the MD field must
                                                         be the same across all configured PEMs!
                                                           000 - Gen2 Speed, 2-lanes
                                                           001 - Gen2 Speed, 1-lane
                                                           010 - Gen2 Speed, 4-lanes
                                                           011 - Rsvd
                                                           100 - Gen1 Speed, 2-lanes
                                                           101 - Gen1 Speed, 1-lane
                                                           110 - Gen1 Speed, 4-lanes
                                                           111 - Rsvd */
#else
	uint64_t md                           : 3;
	uint64_t hostmd                       : 1;
	uint64_t laneswap                     : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} cn70xx;
	struct cvmx_pemx_cfg_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t laneswap                     : 1;  /**< This field will overwrite the pin setting for lane swapping.
                                                         When set, lane swapping is performed to/from the SerDes.
                                                         When clear, no lane swapping is performed. */
	uint64_t lanes8                       : 1;  /**< This field will overwrite the pin setting for number of lanes.
                                                         When set, the PEM is configured for a maximum of 8-lanes,
                                                         When clear, the PEM is configured for a maximum of 4-lanes.
                                                         This value is used to set the Maximum Link Width field in the
                                                         core's Link Capabilities Register (CFG031) to indicate the
                                                         maximum number of lanes supported. Note that less lanes than
                                                         the specified maximum can be configured for use via the core's
                                                         Link Control Register (CFG032) Negotiated Link Width field. */
	uint64_t hostmd                       : 1;  /**< This field will overwrite the pin settings for host mode.
                                                         When set, the PEM is configured to be a Root Complex.
                                                         When clear, the PEM is configured to be an End Point. */
	uint64_t md                           : 2;  /**< This field will overwrite the pin settings for speed.
                                                         00 - EP Mode, Gen1 Speed
                                                         01 - EP Mode, Gen2 Speed
                                                         10 - EP Mode, Gen3 Speed
                                                         11 - Rsvd */
#else
	uint64_t md                           : 2;
	uint64_t hostmd                       : 1;
	uint64_t lanes8                       : 1;
	uint64_t laneswap                     : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} cn78xx;
};
typedef union cvmx_pemx_cfg cvmx_pemx_cfg_t;

/**
 * cvmx_pem#_cfg_rd
 *
 * Allows read access to the configuration in the PCIe Core.
 *
 */
union cvmx_pemx_cfg_rd {
	uint64_t u64;
	struct cvmx_pemx_cfg_rd_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 32; /**< Data. */
	uint64_t addr                         : 32; /**< Address to read. A write to this register
                                                         starts a read operation. */
#else
	uint64_t addr                         : 32;
	uint64_t data                         : 32;
#endif
	} s;
	struct cvmx_pemx_cfg_rd_s             cn61xx;
	struct cvmx_pemx_cfg_rd_s             cn63xx;
	struct cvmx_pemx_cfg_rd_s             cn63xxp1;
	struct cvmx_pemx_cfg_rd_s             cn66xx;
	struct cvmx_pemx_cfg_rd_s             cn68xx;
	struct cvmx_pemx_cfg_rd_s             cn68xxp1;
	struct cvmx_pemx_cfg_rd_s             cn70xx;
	struct cvmx_pemx_cfg_rd_s             cn78xx;
	struct cvmx_pemx_cfg_rd_s             cnf71xx;
};
typedef union cvmx_pemx_cfg_rd cvmx_pemx_cfg_rd_t;

/**
 * cvmx_pem#_cfg_wr
 *
 * Allows write access to the configuration in the PCIe Core.
 *
 */
union cvmx_pemx_cfg_wr {
	uint64_t u64;
	struct cvmx_pemx_cfg_wr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 32; /**< Data to write. A write to this register starts
                                                         a write operation. */
	uint64_t addr                         : 32; /**< Address to write. A write to this register starts
                                                         a write operation. */
#else
	uint64_t addr                         : 32;
	uint64_t data                         : 32;
#endif
	} s;
	struct cvmx_pemx_cfg_wr_s             cn61xx;
	struct cvmx_pemx_cfg_wr_s             cn63xx;
	struct cvmx_pemx_cfg_wr_s             cn63xxp1;
	struct cvmx_pemx_cfg_wr_s             cn66xx;
	struct cvmx_pemx_cfg_wr_s             cn68xx;
	struct cvmx_pemx_cfg_wr_s             cn68xxp1;
	struct cvmx_pemx_cfg_wr_s             cn70xx;
	struct cvmx_pemx_cfg_wr_s             cn78xx;
	struct cvmx_pemx_cfg_wr_s             cnf71xx;
};
typedef union cvmx_pemx_cfg_wr cvmx_pemx_cfg_wr_t;

/**
 * cvmx_pem#_clk_en
 *
 * Clock Enable for ECLK and PCE_CLK
 *
 */
union cvmx_pemx_clk_en {
	uint64_t u64;
	struct cvmx_pemx_clk_en_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t pceclk_gate                  : 1;  /**< When set, pce_clk is gated off.
                                                         When clear, pce_clk is enabled. */
	uint64_t csclk_gate                   : 1;  /**< When set, eclk is gated off.
                                                         When clear, eclk is enabled. */
#else
	uint64_t csclk_gate                   : 1;
	uint64_t pceclk_gate                  : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_pemx_clk_en_s             cn70xx;
	struct cvmx_pemx_clk_en_s             cn78xx;
};
typedef union cvmx_pemx_clk_en cvmx_pemx_clk_en_t;

/**
 * cvmx_pem#_cpl_lut_valid
 *
 * Bit set for outstanding tag read.
 *
 */
union cvmx_pemx_cpl_lut_valid {
	uint64_t u64;
	struct cvmx_pemx_cpl_lut_valid_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t tag                          : 64; /**< Bit vector set cooresponds to an outstanding tag
                                                         expecting a completion. */
#else
	uint64_t tag                          : 64;
#endif
	} s;
	struct cvmx_pemx_cpl_lut_valid_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t tag                          : 32; /**< Bit vector set cooresponds to an outstanding tag
                                                         expecting a completion. */
#else
	uint64_t tag                          : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} cn61xx;
	struct cvmx_pemx_cpl_lut_valid_cn61xx cn63xx;
	struct cvmx_pemx_cpl_lut_valid_cn61xx cn63xxp1;
	struct cvmx_pemx_cpl_lut_valid_cn61xx cn66xx;
	struct cvmx_pemx_cpl_lut_valid_cn61xx cn68xx;
	struct cvmx_pemx_cpl_lut_valid_cn61xx cn68xxp1;
	struct cvmx_pemx_cpl_lut_valid_cn61xx cn70xx;
	struct cvmx_pemx_cpl_lut_valid_s      cn78xx;
	struct cvmx_pemx_cpl_lut_valid_cn61xx cnf71xx;
};
typedef union cvmx_pemx_cpl_lut_valid cvmx_pemx_cpl_lut_valid_t;

/**
 * cvmx_pem#_ctl_status
 *
 * General control and status of the PEM.
 *
 */
union cvmx_pemx_ctl_status {
	uint64_t u64;
	struct cvmx_pemx_ctl_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_51_63               : 13;
	uint64_t inv_dpar                     : 1;  /**< Invert the generated parity to be written into the
                                                         the most significant Data Queue Buffer ram block
                                                         to force a parity error when it is later read. */
	uint64_t inv_hpar                     : 1;  /**< Invert the generated parity to be written into the
                                                         most significant Header Queue Buffer ram block
                                                         to force a parity error when it is later read. */
	uint64_t inv_rpar                     : 1;  /**< Invert the generated parity to be written into the
                                                         tmost significant Retry Buffer ram block to force
                                                         a parity error when it is later read. */
	uint64_t auto_sd                      : 1;  /**< Link Hardware Autonomous Speed Disable. */
	uint64_t dnum                         : 5;  /**< Primary bus device number. */
	uint64_t pbus                         : 8;  /**< Primary bus number. */
	uint64_t reserved_32_33               : 2;
	uint64_t cfg_rtry                     : 16; /**< The time x 0x10000 in core clocks to wait for a
                                                         CPL to a CFG RD that does not carry a Retry Status.
                                                         Until such time that the timeout occurs and Retry
                                                         Status is received for a CFG RD, the Read CFG Read
                                                         will be resent. A value of 0 disables retries and
                                                         treats a CPL Retry as a CPL UR.
                                                         When enabled only one CFG RD may be issued until
                                                         either successful completion or CPL UR. */
	uint64_t reserved_12_15               : 4;
	uint64_t pm_xtoff                     : 1;  /**< When WRITTEN with a '1' a single cycle pulse is
                                                         to the PCIe core pm_xmt_turnoff port. RC mode. */
	uint64_t pm_xpme                      : 1;  /**< When WRITTEN with a '1' a single cycle pulse is
                                                         to the PCIe core pm_xmt_pme port. EP mode. */
	uint64_t ob_p_cmd                     : 1;  /**< When WRITTEN with a '1' a single cycle pulse is
                                                         to the PCIe core outband_pwrup_cmd port. EP mode. */
	uint64_t reserved_7_8                 : 2;
	uint64_t nf_ecrc                      : 1;  /**< Do not forward peer-to-peer ECRC TLPs. */
	uint64_t dly_one                      : 1;  /**< When set the output client state machines will
                                                         wait one cycle before starting a new TLP out. */
	uint64_t lnk_enb                      : 1;  /**< When set '1' the link is enabled when '0' the
                                                         link is disabled. This bit only is active when in
                                                         RC mode. */
	uint64_t ro_ctlp                      : 1;  /**< When set '1' C-TLPs that have the RO bit set will
                                                         not wait for P-TLPs that normaly would be sent
                                                         first. */
	uint64_t fast_lm                      : 1;  /**< When '1' forces fast link mode. */
	uint64_t inv_ecrc                     : 1;  /**< When '1' causes the LSB of the ECRC to be inverted. */
	uint64_t inv_lcrc                     : 1;  /**< When '1' causes the LSB of the LCRC to be inverted. */
#else
	uint64_t inv_lcrc                     : 1;
	uint64_t inv_ecrc                     : 1;
	uint64_t fast_lm                      : 1;
	uint64_t ro_ctlp                      : 1;
	uint64_t lnk_enb                      : 1;
	uint64_t dly_one                      : 1;
	uint64_t nf_ecrc                      : 1;
	uint64_t reserved_7_8                 : 2;
	uint64_t ob_p_cmd                     : 1;
	uint64_t pm_xpme                      : 1;
	uint64_t pm_xtoff                     : 1;
	uint64_t reserved_12_15               : 4;
	uint64_t cfg_rtry                     : 16;
	uint64_t reserved_32_33               : 2;
	uint64_t pbus                         : 8;
	uint64_t dnum                         : 5;
	uint64_t auto_sd                      : 1;
	uint64_t inv_rpar                     : 1;
	uint64_t inv_hpar                     : 1;
	uint64_t inv_dpar                     : 1;
	uint64_t reserved_51_63               : 13;
#endif
	} s;
	struct cvmx_pemx_ctl_status_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t auto_sd                      : 1;  /**< Link Hardware Autonomous Speed Disable. */
	uint64_t dnum                         : 5;  /**< Primary bus device number. */
	uint64_t pbus                         : 8;  /**< Primary bus number. */
	uint64_t reserved_32_33               : 2;
	uint64_t cfg_rtry                     : 16; /**< The time x 0x10000 in core clocks to wait for a
                                                         CPL to a CFG RD that does not carry a Retry Status.
                                                         Until such time that the timeout occurs and Retry
                                                         Status is received for a CFG RD, the Read CFG Read
                                                         will be resent. A value of 0 disables retries and
                                                         treats a CPL Retry as a CPL UR.
                                                         When enabled only one CFG RD may be issued until
                                                         either successful completion or CPL UR. */
	uint64_t reserved_12_15               : 4;
	uint64_t pm_xtoff                     : 1;  /**< When WRITTEN with a '1' a single cycle pulse is
                                                         to the PCIe core pm_xmt_turnoff port. RC mode. */
	uint64_t pm_xpme                      : 1;  /**< When WRITTEN with a '1' a single cycle pulse is
                                                         to the PCIe core pm_xmt_pme port. EP mode. */
	uint64_t ob_p_cmd                     : 1;  /**< When WRITTEN with a '1' a single cycle pulse is
                                                         to the PCIe core outband_pwrup_cmd port. EP mode. */
	uint64_t reserved_7_8                 : 2;
	uint64_t nf_ecrc                      : 1;  /**< Do not forward peer-to-peer ECRC TLPs. */
	uint64_t dly_one                      : 1;  /**< When set the output client state machines will
                                                         wait one cycle before starting a new TLP out. */
	uint64_t lnk_enb                      : 1;  /**< When set '1' the link is enabled when '0' the
                                                         link is disabled. This bit only is active when in
                                                         RC mode. */
	uint64_t ro_ctlp                      : 1;  /**< When set '1' C-TLPs that have the RO bit set will
                                                         not wait for P-TLPs that normaly would be sent
                                                         first. */
	uint64_t fast_lm                      : 1;  /**< When '1' forces fast link mode. */
	uint64_t inv_ecrc                     : 1;  /**< When '1' causes the LSB of the ECRC to be inverted. */
	uint64_t inv_lcrc                     : 1;  /**< When '1' causes the LSB of the LCRC to be inverted. */
#else
	uint64_t inv_lcrc                     : 1;
	uint64_t inv_ecrc                     : 1;
	uint64_t fast_lm                      : 1;
	uint64_t ro_ctlp                      : 1;
	uint64_t lnk_enb                      : 1;
	uint64_t dly_one                      : 1;
	uint64_t nf_ecrc                      : 1;
	uint64_t reserved_7_8                 : 2;
	uint64_t ob_p_cmd                     : 1;
	uint64_t pm_xpme                      : 1;
	uint64_t pm_xtoff                     : 1;
	uint64_t reserved_12_15               : 4;
	uint64_t cfg_rtry                     : 16;
	uint64_t reserved_32_33               : 2;
	uint64_t pbus                         : 8;
	uint64_t dnum                         : 5;
	uint64_t auto_sd                      : 1;
	uint64_t reserved_48_63               : 16;
#endif
	} cn61xx;
	struct cvmx_pemx_ctl_status_cn61xx    cn63xx;
	struct cvmx_pemx_ctl_status_cn61xx    cn63xxp1;
	struct cvmx_pemx_ctl_status_cn61xx    cn66xx;
	struct cvmx_pemx_ctl_status_cn61xx    cn68xx;
	struct cvmx_pemx_ctl_status_cn61xx    cn68xxp1;
	struct cvmx_pemx_ctl_status_s         cn70xx;
	struct cvmx_pemx_ctl_status_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_51_63               : 13;
	uint64_t inv_dpar                     : 1;  /**< Invert the generated parity to be written into the
                                                         the most significant Data Queue Buffer ram block
                                                         to force a parity error when it is later read. */
	uint64_t reserved_48_49               : 2;
	uint64_t auto_sd                      : 1;  /**< Link Hardware Autonomous Speed Disable. */
	uint64_t dnum                         : 5;  /**< Primary bus device number. */
	uint64_t pbus                         : 8;  /**< Primary bus number. */
	uint64_t reserved_32_33               : 2;
	uint64_t cfg_rtry                     : 16; /**< The time x 0x10000 in core clocks to wait for a
                                                         CPL to a CFG RD that does not carry a Retry Status.
                                                         Until such time that the timeout occurs and Retry
                                                         Status is received for a CFG RD, the Read CFG Read
                                                         will be resent. A value of 0 disables retries and
                                                         treats a CPL Retry as a CPL UR.
                                                         When enabled only one CFG RD may be issued until
                                                         either successful completion or CPL UR. */
	uint64_t reserved_12_15               : 4;
	uint64_t pm_xtoff                     : 1;  /**< When WRITTEN with a '1' a single cycle pulse is
                                                         to the PCIe core pm_xmt_turnoff port. RC mode. */
	uint64_t pm_xpme                      : 1;  /**< When WRITTEN with a '1' a single cycle pulse is
                                                         to the PCIe core pm_xmt_pme port. EP mode. */
	uint64_t ob_p_cmd                     : 1;  /**< When WRITTEN with a '1' a single cycle pulse is
                                                         to the PCIe core outband_pwrup_cmd port. EP mode. */
	uint64_t reserved_7_8                 : 2;
	uint64_t nf_ecrc                      : 1;  /**< Do not forward peer-to-peer ECRC TLPs. */
	uint64_t dly_one                      : 1;  /**< When set the output client state machines will
                                                         wait one cycle before starting a new TLP out. */
	uint64_t lnk_enb                      : 1;  /**< When set '1' the link is enabled when '0' the
                                                         link is disabled. This bit only is active when in
                                                         RC mode. */
	uint64_t ro_ctlp                      : 1;  /**< When set '1' C-TLPs that have the RO bit set will
                                                         not wait for P-TLPs that normaly would be sent
                                                         first. */
	uint64_t fast_lm                      : 1;  /**< When '1' forces fast link mode. */
	uint64_t inv_ecrc                     : 1;  /**< When '1' causes the LSB of the ECRC to be inverted. */
	uint64_t inv_lcrc                     : 1;  /**< When '1' causes the LSB of the LCRC to be inverted. */
#else
	uint64_t inv_lcrc                     : 1;
	uint64_t inv_ecrc                     : 1;
	uint64_t fast_lm                      : 1;
	uint64_t ro_ctlp                      : 1;
	uint64_t lnk_enb                      : 1;
	uint64_t dly_one                      : 1;
	uint64_t nf_ecrc                      : 1;
	uint64_t reserved_7_8                 : 2;
	uint64_t ob_p_cmd                     : 1;
	uint64_t pm_xpme                      : 1;
	uint64_t pm_xtoff                     : 1;
	uint64_t reserved_12_15               : 4;
	uint64_t cfg_rtry                     : 16;
	uint64_t reserved_32_33               : 2;
	uint64_t pbus                         : 8;
	uint64_t dnum                         : 5;
	uint64_t auto_sd                      : 1;
	uint64_t reserved_48_49               : 2;
	uint64_t inv_dpar                     : 1;
	uint64_t reserved_51_63               : 13;
#endif
	} cn78xx;
	struct cvmx_pemx_ctl_status_cn61xx    cnf71xx;
};
typedef union cvmx_pemx_ctl_status cvmx_pemx_ctl_status_t;

/**
 * cvmx_pem#_ctl_status2
 *
 * Additional general control and status of the PEM.
 *
 */
union cvmx_pemx_ctl_status2 {
	uint64_t u64;
	struct cvmx_pemx_ctl_status2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t no_fwd_prg                   : 16; /**< The time x 0x10000 in core clocks to wait for the
                                                         TLP FIFOs to be able to unload an entry. If there is
                                                         no forward progress, such that the timeout occurs,
                                                         credits will be returned to the SLI and an interrupt
                                                         (if enabled) will be asserted. Any more TLPs received
                                                         will be dropped on the floor and the credits
                                                         associated with those TLPs will be returned, as well.
                                                         Note that 0xFFFF is a reserved value that will put
                                                         the PEM in the 'forward progress stopped' state immediately.
                                                         This state will hold until a mac reset is received. */
#else
	uint64_t no_fwd_prg                   : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_pemx_ctl_status2_s        cn78xx;
};
typedef union cvmx_pemx_ctl_status2 cvmx_pemx_ctl_status2_t;

/**
 * cvmx_pem#_dbg_info
 *
 * "PEM#_DBG_INFO = PEM Debug Information
 * General debug info."
 */
union cvmx_pemx_dbg_info {
	uint64_t u64;
	struct cvmx_pemx_dbg_info_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_58_63               : 6;
	uint64_t qhdr_b1_dbe                  : 1;  /**< Detected a Core Header Queue Bank1 double bit error */
	uint64_t qhdr_b1_sbe                  : 1;  /**< Detected a Core Header Queue Bank1 single bit error */
	uint64_t qhdr_b0_dbe                  : 1;  /**< Detected a Core Header Queue Bank0 double bit error */
	uint64_t qhdr_b0_sbe                  : 1;  /**< Detected a Core Header Queue Bank0 single bit error */
	uint64_t rtry_dbe                     : 1;  /**< Detected a Core Retry RAM double bit error */
	uint64_t rtry_sbe                     : 1;  /**< Detected a Core Retry RAM single bit error */
	uint64_t reserved_50_51               : 2;
	uint64_t c_d1_dbe                     : 1;  /**< Detected a TLP CPL Fifo data1 double bit error */
	uint64_t c_d1_sbe                     : 1;  /**< Detected a TLP CPL Fifo data1 single bit error */
	uint64_t c_d0_dbe                     : 1;  /**< Detected a TLP CPL Fifo data0 double bit error */
	uint64_t c_d0_sbe                     : 1;  /**< Detected a TLP CPL Fifo data0 single bit error */
	uint64_t reserved_34_45               : 12;
	uint64_t datq_pe                      : 1;  /**< Detected a Data Queue RAM parity error */
	uint64_t hdrq_pe                      : 1;  /**< Detected a Header Queue RAM parity error */
	uint64_t reserved_31_31               : 1;
	uint64_t ecrc_e                       : 1;  /**< Received a ECRC error.
                                                         radm_ecrc_err */
	uint64_t rawwpp                       : 1;  /**< Received a write with poisoned payload
                                                         radm_rcvd_wreq_poisoned */
	uint64_t racpp                        : 1;  /**< Received a completion with poisoned payload
                                                         radm_rcvd_cpl_poisoned */
	uint64_t ramtlp                       : 1;  /**< Received a malformed TLP
                                                         radm_mlf_tlp_err */
	uint64_t rarwdns                      : 1;  /**< Recieved a request which device does not support
                                                         radm_rcvd_ur_req */
	uint64_t caar                         : 1;  /**< Completer aborted a request
                                                         radm_rcvd_ca_req
                                                         This bit will never be set because Octeon does
                                                         not generate Completer Aborts. */
	uint64_t racca                        : 1;  /**< Received a completion with CA status
                                                         radm_rcvd_cpl_ca */
	uint64_t racur                        : 1;  /**< Received a completion with UR status
                                                         radm_rcvd_cpl_ur */
	uint64_t rauc                         : 1;  /**< Received an unexpected completion
                                                         radm_unexp_cpl_err */
	uint64_t rqo                          : 1;  /**< Receive queue overflow. Normally happens only when
                                                         flow control advertisements are ignored
                                                         radm_qoverflow */
	uint64_t fcuv                         : 1;  /**< Flow Control Update Violation (opt. checks)
                                                         int_xadm_fc_prot_err */
	uint64_t rpe                          : 1;  /**< When the PHY reports 8B/10B decode error
                                                         (RxStatus = 3b100) or disparity error
                                                         (RxStatus = 3b111), the signal rmlh_rcvd_err will
                                                         be asserted.
                                                         rmlh_rcvd_err */
	uint64_t fcpvwt                       : 1;  /**< Flow Control Protocol Violation (Watchdog Timer)
                                                         rtlh_fc_prot_err */
	uint64_t dpeoosd                      : 1;  /**< DLLP protocol error (out of sequence DLLP)
                                                         rdlh_prot_err */
	uint64_t rtwdle                       : 1;  /**< Received TLP with DataLink Layer Error
                                                         rdlh_bad_tlp_err */
	uint64_t rdwdle                       : 1;  /**< Received DLLP with DataLink Layer Error
                                                         rdlh_bad_dllp_err */
	uint64_t mre                          : 1;  /**< Max Retries Exceeded
                                                         xdlh_replay_num_rlover_err */
	uint64_t rte                          : 1;  /**< Replay Timer Expired
                                                         xdlh_replay_timeout_err
                                                         This bit is set when the REPLAY_TIMER expires in
                                                         the PCIE core. The probability of this bit being
                                                         set will increase with the traffic load. */
	uint64_t acto                         : 1;  /**< A Completion Timeout Occured
                                                         pedc_radm_cpl_timeout */
	uint64_t rvdm                         : 1;  /**< Received Vendor-Defined Message
                                                         pedc_radm_vendor_msg */
	uint64_t rumep                        : 1;  /**< Received Unlock Message (EP Mode Only)
                                                         pedc_radm_msg_unlock */
	uint64_t rptamrc                      : 1;  /**< Received PME Turnoff Acknowledge Message
                                                         (RC Mode only)
                                                         pedc_radm_pm_to_ack */
	uint64_t rpmerc                       : 1;  /**< Received PME Message (RC Mode only)
                                                         pedc_radm_pm_pme */
	uint64_t rfemrc                       : 1;  /**< Received Fatal Error Message (RC Mode only)
                                                         pedc_radm_fatal_err
                                                         Bit set when a message with ERR_FATAL is set. */
	uint64_t rnfemrc                      : 1;  /**< Received Non-Fatal Error Message (RC Mode only)
                                                         pedc_radm_nonfatal_err */
	uint64_t rcemrc                       : 1;  /**< Received Correctable Error Message (RC Mode only)
                                                         pedc_radm_correctable_err */
	uint64_t rpoison                      : 1;  /**< Received Poisoned TLP
                                                         pedc__radm_trgt1_poisoned & pedc__radm_trgt1_hv */
	uint64_t recrce                       : 1;  /**< Received ECRC Error
                                                         pedc_radm_trgt1_ecrc_err & pedc__radm_trgt1_eot */
	uint64_t rtlplle                      : 1;  /**< Received TLP has link layer error
                                                         pedc_radm_trgt1_dllp_abort & pedc__radm_trgt1_eot */
	uint64_t rtlpmal                      : 1;  /**< Received TLP is malformed or a message.
                                                         pedc_radm_trgt1_tlp_abort & pedc__radm_trgt1_eot
                                                         If the core receives a MSG (or Vendor Message)
                                                         this bit will be set. */
	uint64_t spoison                      : 1;  /**< Poisoned TLP sent
                                                         peai__client0_tlp_ep & peai__client0_tlp_hv */
#else
	uint64_t spoison                      : 1;
	uint64_t rtlpmal                      : 1;
	uint64_t rtlplle                      : 1;
	uint64_t recrce                       : 1;
	uint64_t rpoison                      : 1;
	uint64_t rcemrc                       : 1;
	uint64_t rnfemrc                      : 1;
	uint64_t rfemrc                       : 1;
	uint64_t rpmerc                       : 1;
	uint64_t rptamrc                      : 1;
	uint64_t rumep                        : 1;
	uint64_t rvdm                         : 1;
	uint64_t acto                         : 1;
	uint64_t rte                          : 1;
	uint64_t mre                          : 1;
	uint64_t rdwdle                       : 1;
	uint64_t rtwdle                       : 1;
	uint64_t dpeoosd                      : 1;
	uint64_t fcpvwt                       : 1;
	uint64_t rpe                          : 1;
	uint64_t fcuv                         : 1;
	uint64_t rqo                          : 1;
	uint64_t rauc                         : 1;
	uint64_t racur                        : 1;
	uint64_t racca                        : 1;
	uint64_t caar                         : 1;
	uint64_t rarwdns                      : 1;
	uint64_t ramtlp                       : 1;
	uint64_t racpp                        : 1;
	uint64_t rawwpp                       : 1;
	uint64_t ecrc_e                       : 1;
	uint64_t reserved_31_31               : 1;
	uint64_t hdrq_pe                      : 1;
	uint64_t datq_pe                      : 1;
	uint64_t reserved_34_45               : 12;
	uint64_t c_d0_sbe                     : 1;
	uint64_t c_d0_dbe                     : 1;
	uint64_t c_d1_sbe                     : 1;
	uint64_t c_d1_dbe                     : 1;
	uint64_t reserved_50_51               : 2;
	uint64_t rtry_sbe                     : 1;
	uint64_t rtry_dbe                     : 1;
	uint64_t qhdr_b0_sbe                  : 1;
	uint64_t qhdr_b0_dbe                  : 1;
	uint64_t qhdr_b1_sbe                  : 1;
	uint64_t qhdr_b1_dbe                  : 1;
	uint64_t reserved_58_63               : 6;
#endif
	} s;
	struct cvmx_pemx_dbg_info_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_31_63               : 33;
	uint64_t ecrc_e                       : 1;  /**< Received a ECRC error.
                                                         radm_ecrc_err */
	uint64_t rawwpp                       : 1;  /**< Received a write with poisoned payload
                                                         radm_rcvd_wreq_poisoned */
	uint64_t racpp                        : 1;  /**< Received a completion with poisoned payload
                                                         radm_rcvd_cpl_poisoned */
	uint64_t ramtlp                       : 1;  /**< Received a malformed TLP
                                                         radm_mlf_tlp_err */
	uint64_t rarwdns                      : 1;  /**< Recieved a request which device does not support
                                                         radm_rcvd_ur_req */
	uint64_t caar                         : 1;  /**< Completer aborted a request
                                                         radm_rcvd_ca_req
                                                         This bit will never be set because Octeon does
                                                         not generate Completer Aborts. */
	uint64_t racca                        : 1;  /**< Received a completion with CA status
                                                         radm_rcvd_cpl_ca */
	uint64_t racur                        : 1;  /**< Received a completion with UR status
                                                         radm_rcvd_cpl_ur */
	uint64_t rauc                         : 1;  /**< Received an unexpected completion
                                                         radm_unexp_cpl_err */
	uint64_t rqo                          : 1;  /**< Receive queue overflow. Normally happens only when
                                                         flow control advertisements are ignored
                                                         radm_qoverflow */
	uint64_t fcuv                         : 1;  /**< Flow Control Update Violation (opt. checks)
                                                         int_xadm_fc_prot_err */
	uint64_t rpe                          : 1;  /**< When the PHY reports 8B/10B decode error
                                                         (RxStatus = 3b100) or disparity error
                                                         (RxStatus = 3b111), the signal rmlh_rcvd_err will
                                                         be asserted.
                                                         rmlh_rcvd_err */
	uint64_t fcpvwt                       : 1;  /**< Flow Control Protocol Violation (Watchdog Timer)
                                                         rtlh_fc_prot_err */
	uint64_t dpeoosd                      : 1;  /**< DLLP protocol error (out of sequence DLLP)
                                                         rdlh_prot_err */
	uint64_t rtwdle                       : 1;  /**< Received TLP with DataLink Layer Error
                                                         rdlh_bad_tlp_err */
	uint64_t rdwdle                       : 1;  /**< Received DLLP with DataLink Layer Error
                                                         rdlh_bad_dllp_err */
	uint64_t mre                          : 1;  /**< Max Retries Exceeded
                                                         xdlh_replay_num_rlover_err */
	uint64_t rte                          : 1;  /**< Replay Timer Expired
                                                         xdlh_replay_timeout_err
                                                         This bit is set when the REPLAY_TIMER expires in
                                                         the PCIE core. The probability of this bit being
                                                         set will increase with the traffic load. */
	uint64_t acto                         : 1;  /**< A Completion Timeout Occured
                                                         pedc_radm_cpl_timeout */
	uint64_t rvdm                         : 1;  /**< Received Vendor-Defined Message
                                                         pedc_radm_vendor_msg */
	uint64_t rumep                        : 1;  /**< Received Unlock Message (EP Mode Only)
                                                         pedc_radm_msg_unlock */
	uint64_t rptamrc                      : 1;  /**< Received PME Turnoff Acknowledge Message
                                                         (RC Mode only)
                                                         pedc_radm_pm_to_ack */
	uint64_t rpmerc                       : 1;  /**< Received PME Message (RC Mode only)
                                                         pedc_radm_pm_pme */
	uint64_t rfemrc                       : 1;  /**< Received Fatal Error Message (RC Mode only)
                                                         pedc_radm_fatal_err
                                                         Bit set when a message with ERR_FATAL is set. */
	uint64_t rnfemrc                      : 1;  /**< Received Non-Fatal Error Message (RC Mode only)
                                                         pedc_radm_nonfatal_err */
	uint64_t rcemrc                       : 1;  /**< Received Correctable Error Message (RC Mode only)
                                                         pedc_radm_correctable_err */
	uint64_t rpoison                      : 1;  /**< Received Poisoned TLP
                                                         pedc__radm_trgt1_poisoned & pedc__radm_trgt1_hv */
	uint64_t recrce                       : 1;  /**< Received ECRC Error
                                                         pedc_radm_trgt1_ecrc_err & pedc__radm_trgt1_eot */
	uint64_t rtlplle                      : 1;  /**< Received TLP has link layer error
                                                         pedc_radm_trgt1_dllp_abort & pedc__radm_trgt1_eot */
	uint64_t rtlpmal                      : 1;  /**< Received TLP is malformed or a message.
                                                         pedc_radm_trgt1_tlp_abort & pedc__radm_trgt1_eot
                                                         If the core receives a MSG (or Vendor Message)
                                                         this bit will be set. */
	uint64_t spoison                      : 1;  /**< Poisoned TLP sent
                                                         peai__client0_tlp_ep & peai__client0_tlp_hv */
#else
	uint64_t spoison                      : 1;
	uint64_t rtlpmal                      : 1;
	uint64_t rtlplle                      : 1;
	uint64_t recrce                       : 1;
	uint64_t rpoison                      : 1;
	uint64_t rcemrc                       : 1;
	uint64_t rnfemrc                      : 1;
	uint64_t rfemrc                       : 1;
	uint64_t rpmerc                       : 1;
	uint64_t rptamrc                      : 1;
	uint64_t rumep                        : 1;
	uint64_t rvdm                         : 1;
	uint64_t acto                         : 1;
	uint64_t rte                          : 1;
	uint64_t mre                          : 1;
	uint64_t rdwdle                       : 1;
	uint64_t rtwdle                       : 1;
	uint64_t dpeoosd                      : 1;
	uint64_t fcpvwt                       : 1;
	uint64_t rpe                          : 1;
	uint64_t fcuv                         : 1;
	uint64_t rqo                          : 1;
	uint64_t rauc                         : 1;
	uint64_t racur                        : 1;
	uint64_t racca                        : 1;
	uint64_t caar                         : 1;
	uint64_t rarwdns                      : 1;
	uint64_t ramtlp                       : 1;
	uint64_t racpp                        : 1;
	uint64_t rawwpp                       : 1;
	uint64_t ecrc_e                       : 1;
	uint64_t reserved_31_63               : 33;
#endif
	} cn61xx;
	struct cvmx_pemx_dbg_info_cn61xx      cn63xx;
	struct cvmx_pemx_dbg_info_cn61xx      cn63xxp1;
	struct cvmx_pemx_dbg_info_cn61xx      cn66xx;
	struct cvmx_pemx_dbg_info_cn61xx      cn68xx;
	struct cvmx_pemx_dbg_info_cn61xx      cn68xxp1;
	struct cvmx_pemx_dbg_info_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_46_63               : 18;
	uint64_t c_c_dbe                      : 1;  /**< Detected a TLP CPL Fifo ctrl double bit error */
	uint64_t c_c_sbe                      : 1;  /**< Detected a TLP CPL Fifo ctrl single bit error */
	uint64_t c_d_dbe                      : 1;  /**< Detected a TLP CPL Fifo data double bit error */
	uint64_t c_d_sbe                      : 1;  /**< Detected a TLP CPL Fifo data single bit error */
	uint64_t n_c_dbe                      : 1;  /**< Detected a TLP Non-Posted Fifo ctrl double bit error */
	uint64_t n_c_sbe                      : 1;  /**< Detected a TLP Non-Posted fifo ctrl single bit error */
	uint64_t n_d_dbe                      : 1;  /**< Detected a TLP Non-Posted Fifo data double bit error */
	uint64_t n_d_sbe                      : 1;  /**< Detected a TLP Non-Posted Fifo data single bit error */
	uint64_t p_c_dbe                      : 1;  /**< Detected a TLP Posted Fifo ctrl double bit error */
	uint64_t p_c_sbe                      : 1;  /**< Detected a TLP Posted Fifo ctrl single bit error */
	uint64_t p_d_dbe                      : 1;  /**< Detected a TLP Posted Fifo data double bit error */
	uint64_t p_d_sbe                      : 1;  /**< Detected a TLP Posted Fifo data single bit error */
	uint64_t datq_pe                      : 1;  /**< Detected a Data Queue RAM parity error */
	uint64_t hdrq_pe                      : 1;  /**< Detected a Header Queue RAM parity error */
	uint64_t rtry_pe                      : 1;  /**< Detected a Retry RAM parity error */
	uint64_t ecrc_e                       : 1;  /**< Received a ECRC error.
                                                         radm_ecrc_err */
	uint64_t rawwpp                       : 1;  /**< Received a write with poisoned payload
                                                         radm_rcvd_wreq_poisoned */
	uint64_t racpp                        : 1;  /**< Received a completion with poisoned payload
                                                         radm_rcvd_cpl_poisoned */
	uint64_t ramtlp                       : 1;  /**< Received a malformed TLP
                                                         radm_mlf_tlp_err */
	uint64_t rarwdns                      : 1;  /**< Recieved a request which device does not support
                                                         radm_rcvd_ur_req */
	uint64_t caar                         : 1;  /**< Completer aborted a request
                                                         radm_rcvd_ca_req
                                                         This bit will never be set because Octeon does
                                                         not generate Completer Aborts. */
	uint64_t racca                        : 1;  /**< Received a completion with CA status
                                                         radm_rcvd_cpl_ca */
	uint64_t racur                        : 1;  /**< Received a completion with UR status
                                                         radm_rcvd_cpl_ur */
	uint64_t rauc                         : 1;  /**< Received an unexpected completion
                                                         radm_unexp_cpl_err */
	uint64_t rqo                          : 1;  /**< Receive queue overflow. Normally happens only when
                                                         flow control advertisements are ignored
                                                         radm_qoverflow */
	uint64_t fcuv                         : 1;  /**< Flow Control Update Violation (opt. checks)
                                                         int_xadm_fc_prot_err */
	uint64_t rpe                          : 1;  /**< When the PHY reports 8B/10B decode error
                                                         (RxStatus = 3b100) or disparity error
                                                         (RxStatus = 3b111), the signal rmlh_rcvd_err will
                                                         be asserted.
                                                         rmlh_rcvd_err */
	uint64_t fcpvwt                       : 1;  /**< Flow Control Protocol Violation (Watchdog Timer)
                                                         rtlh_fc_prot_err */
	uint64_t dpeoosd                      : 1;  /**< DLLP protocol error (out of sequence DLLP)
                                                         rdlh_prot_err */
	uint64_t rtwdle                       : 1;  /**< Received TLP with DataLink Layer Error
                                                         rdlh_bad_tlp_err */
	uint64_t rdwdle                       : 1;  /**< Received DLLP with DataLink Layer Error
                                                         rdlh_bad_dllp_err */
	uint64_t mre                          : 1;  /**< Max Retries Exceeded
                                                         xdlh_replay_num_rlover_err */
	uint64_t rte                          : 1;  /**< Replay Timer Expired
                                                         xdlh_replay_timeout_err
                                                         This bit is set when the REPLAY_TIMER expires in
                                                         the PCIE core. The probability of this bit being
                                                         set will increase with the traffic load. */
	uint64_t acto                         : 1;  /**< A Completion Timeout Occured
                                                         pedc_radm_cpl_timeout */
	uint64_t rvdm                         : 1;  /**< Received Vendor-Defined Message
                                                         pedc_radm_vendor_msg */
	uint64_t rumep                        : 1;  /**< Received Unlock Message (EP Mode Only)
                                                         pedc_radm_msg_unlock */
	uint64_t rptamrc                      : 1;  /**< Received PME Turnoff Acknowledge Message
                                                         (RC Mode only)
                                                         pedc_radm_pm_to_ack */
	uint64_t rpmerc                       : 1;  /**< Received PME Message (RC Mode only)
                                                         pedc_radm_pm_pme */
	uint64_t rfemrc                       : 1;  /**< Received Fatal Error Message (RC Mode only)
                                                         pedc_radm_fatal_err
                                                         Bit set when a message with ERR_FATAL is set. */
	uint64_t rnfemrc                      : 1;  /**< Received Non-Fatal Error Message (RC Mode only)
                                                         pedc_radm_nonfatal_err */
	uint64_t rcemrc                       : 1;  /**< Received Correctable Error Message (RC Mode only)
                                                         pedc_radm_correctable_err */
	uint64_t rpoison                      : 1;  /**< Received Poisoned TLP
                                                         pedc__radm_trgt1_poisoned & pedc__radm_trgt1_hv */
	uint64_t recrce                       : 1;  /**< Received ECRC Error
                                                         pedc_radm_trgt1_ecrc_err & pedc__radm_trgt1_eot */
	uint64_t rtlplle                      : 1;  /**< Received TLP has link layer error
                                                         pedc_radm_trgt1_dllp_abort & pedc__radm_trgt1_eot */
	uint64_t rtlpmal                      : 1;  /**< Received TLP is malformed or a message.
                                                         pedc_radm_trgt1_tlp_abort & pedc__radm_trgt1_eot
                                                         If the core receives a MSG (or Vendor Message)
                                                         or if a received AtomicOp viloates address/length rules,
                                                         this bit is set as well. */
	uint64_t spoison                      : 1;  /**< Poisoned TLP sent
                                                         peai__client0_tlp_ep & peai__client0_tlp_hv
                                                         peai__client1_tlp_ep & peai__client1_tlp_hv (atomic_op). */
#else
	uint64_t spoison                      : 1;
	uint64_t rtlpmal                      : 1;
	uint64_t rtlplle                      : 1;
	uint64_t recrce                       : 1;
	uint64_t rpoison                      : 1;
	uint64_t rcemrc                       : 1;
	uint64_t rnfemrc                      : 1;
	uint64_t rfemrc                       : 1;
	uint64_t rpmerc                       : 1;
	uint64_t rptamrc                      : 1;
	uint64_t rumep                        : 1;
	uint64_t rvdm                         : 1;
	uint64_t acto                         : 1;
	uint64_t rte                          : 1;
	uint64_t mre                          : 1;
	uint64_t rdwdle                       : 1;
	uint64_t rtwdle                       : 1;
	uint64_t dpeoosd                      : 1;
	uint64_t fcpvwt                       : 1;
	uint64_t rpe                          : 1;
	uint64_t fcuv                         : 1;
	uint64_t rqo                          : 1;
	uint64_t rauc                         : 1;
	uint64_t racur                        : 1;
	uint64_t racca                        : 1;
	uint64_t caar                         : 1;
	uint64_t rarwdns                      : 1;
	uint64_t ramtlp                       : 1;
	uint64_t racpp                        : 1;
	uint64_t rawwpp                       : 1;
	uint64_t ecrc_e                       : 1;
	uint64_t rtry_pe                      : 1;
	uint64_t hdrq_pe                      : 1;
	uint64_t datq_pe                      : 1;
	uint64_t p_d_sbe                      : 1;
	uint64_t p_d_dbe                      : 1;
	uint64_t p_c_sbe                      : 1;
	uint64_t p_c_dbe                      : 1;
	uint64_t n_d_sbe                      : 1;
	uint64_t n_d_dbe                      : 1;
	uint64_t n_c_sbe                      : 1;
	uint64_t n_c_dbe                      : 1;
	uint64_t c_d_sbe                      : 1;
	uint64_t c_d_dbe                      : 1;
	uint64_t c_c_sbe                      : 1;
	uint64_t c_c_dbe                      : 1;
	uint64_t reserved_46_63               : 18;
#endif
	} cn70xx;
	struct cvmx_pemx_dbg_info_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_58_63               : 6;
	uint64_t qhdr_b1_dbe                  : 1;  /**< Detected a Core Header Queue Bank1 double bit error */
	uint64_t qhdr_b1_sbe                  : 1;  /**< Detected a Core Header Queue Bank1 single bit error */
	uint64_t qhdr_b0_dbe                  : 1;  /**< Detected a Core Header Queue Bank0 double bit error */
	uint64_t qhdr_b0_sbe                  : 1;  /**< Detected a Core Header Queue Bank0 single bit error */
	uint64_t rtry_dbe                     : 1;  /**< Detected a Core Retry RAM double bit error */
	uint64_t rtry_sbe                     : 1;  /**< Detected a Core Retry RAM single bit error */
	uint64_t c_c_dbe                      : 1;  /**< Detected a TLP CPL Fifo ctrl double bit error */
	uint64_t c_c_sbe                      : 1;  /**< Detected a TLP CPL Fifo ctrl single bit error */
	uint64_t c_d1_dbe                     : 1;  /**< Detected a TLP CPL Fifo data1 double bit error */
	uint64_t c_d1_sbe                     : 1;  /**< Detected a TLP CPL Fifo data1 single bit error */
	uint64_t c_d0_dbe                     : 1;  /**< Detected a TLP CPL Fifo data0 double bit error */
	uint64_t c_d0_sbe                     : 1;  /**< Detected a TLP CPL Fifo data0 single bit error */
	uint64_t n_c_dbe                      : 1;  /**< Detected a TLP NP Fifo ctrl double bit error */
	uint64_t n_c_sbe                      : 1;  /**< Detected a TLP NP Fifo ctrl single bit error */
	uint64_t n_d1_dbe                     : 1;  /**< Detected a TLP NP Fifo data1 double bit error */
	uint64_t n_d1_sbe                     : 1;  /**< Detected a TLP NP Fifo data1 single bit error */
	uint64_t n_d0_dbe                     : 1;  /**< Detected a TLP NP Fifo data0 double bit error */
	uint64_t n_d0_sbe                     : 1;  /**< Detected a TLP NP fifo data0 single bit error */
	uint64_t p_c_dbe                      : 1;  /**< Detected a TLP Posted Fifo ctrl double bit error */
	uint64_t p_c_sbe                      : 1;  /**< Detected a TLP Posted Fifo ctrl single bit error */
	uint64_t p_d1_dbe                     : 1;  /**< Detected a TLP Posted Fifo data1 double bit error */
	uint64_t p_d1_sbe                     : 1;  /**< Detected a TLP Posted Fifo data1 single bit error */
	uint64_t p_d0_dbe                     : 1;  /**< Detected a TLP Posted Fifo data0 double bit error */
	uint64_t p_d0_sbe                     : 1;  /**< Detected a TLP Posted Fifo data0 single bit error */
	uint64_t datq_pe                      : 1;  /**< Detected a Data Queue RAM parity error */
	uint64_t reserved_32_32               : 1;
	uint64_t lofp                         : 1;  /**< Lack of Forward Progress at TLP FIFOs timeout occured. */
	uint64_t ecrc_e                       : 1;  /**< Received a ECRC error.
                                                         radm_ecrc_err */
	uint64_t rawwpp                       : 1;  /**< Received a write with poisoned payload
                                                         radm_rcvd_wreq_poisoned */
	uint64_t racpp                        : 1;  /**< Received a completion with poisoned payload
                                                         radm_rcvd_cpl_poisoned */
	uint64_t ramtlp                       : 1;  /**< Received a malformed TLP
                                                         radm_mlf_tlp_err */
	uint64_t rarwdns                      : 1;  /**< Recieved a request which device does not support
                                                         radm_rcvd_ur_req */
	uint64_t caar                         : 1;  /**< Completer aborted a request
                                                         radm_rcvd_ca_req
                                                         This bit will never be set because Octeon does
                                                         not generate Completer Aborts. */
	uint64_t racca                        : 1;  /**< Received a completion with CA status
                                                         radm_rcvd_cpl_ca */
	uint64_t racur                        : 1;  /**< Received a completion with UR status
                                                         radm_rcvd_cpl_ur */
	uint64_t rauc                         : 1;  /**< Received an unexpected completion
                                                         radm_unexp_cpl_err */
	uint64_t rqo                          : 1;  /**< Receive queue overflow. Normally happens only when
                                                         flow control advertisements are ignored
                                                         radm_qoverflow */
	uint64_t fcuv                         : 1;  /**< Flow Control Update Violation (opt. checks)
                                                         int_xadm_fc_prot_err */
	uint64_t rpe                          : 1;  /**< When the PHY reports 8B/10B decode error
                                                         (RxStatus = 3b100) or disparity error
                                                         (RxStatus = 3b111), the signal rmlh_rcvd_err will
                                                         be asserted.
                                                         rmlh_rcvd_err */
	uint64_t fcpvwt                       : 1;  /**< Flow Control Protocol Violation (Watchdog Timer)
                                                         rtlh_fc_prot_err */
	uint64_t dpeoosd                      : 1;  /**< DLLP protocol error (out of sequence DLLP)
                                                         rdlh_prot_err */
	uint64_t rtwdle                       : 1;  /**< Received TLP with DataLink Layer Error
                                                         rdlh_bad_tlp_err */
	uint64_t rdwdle                       : 1;  /**< Received DLLP with DataLink Layer Error
                                                         rdlh_bad_dllp_err */
	uint64_t mre                          : 1;  /**< Max Retries Exceeded
                                                         xdlh_replay_num_rlover_err */
	uint64_t rte                          : 1;  /**< Replay Timer Expired
                                                         xdlh_replay_timeout_err
                                                         This bit is set when the REPLAY_TIMER expires in
                                                         the PCIE core. The probability of this bit being
                                                         set will increase with the traffic load. */
	uint64_t acto                         : 1;  /**< A Completion Timeout Occured
                                                         pedc_radm_cpl_timeout */
	uint64_t rvdm                         : 1;  /**< Received Vendor-Defined Message
                                                         pedc_radm_vendor_msg */
	uint64_t rumep                        : 1;  /**< Received Unlock Message (EP Mode Only)
                                                         pedc_radm_msg_unlock */
	uint64_t rptamrc                      : 1;  /**< Received PME Turnoff Acknowledge Message
                                                         (RC Mode only)
                                                         pedc_radm_pm_to_ack */
	uint64_t rpmerc                       : 1;  /**< Received PME Message (RC Mode only)
                                                         pedc_radm_pm_pme */
	uint64_t rfemrc                       : 1;  /**< Received Fatal Error Message (RC Mode only)
                                                         pedc_radm_fatal_err
                                                         Bit set when a message with ERR_FATAL is set. */
	uint64_t rnfemrc                      : 1;  /**< Received Non-Fatal Error Message (RC Mode only)
                                                         pedc_radm_nonfatal_err */
	uint64_t rcemrc                       : 1;  /**< Received Correctable Error Message (RC Mode only)
                                                         pedc_radm_correctable_err */
	uint64_t rpoison                      : 1;  /**< Received Poisoned TLP
                                                         pedc__radm_trgt1_poisoned & pedc__radm_trgt1_hv */
	uint64_t recrce                       : 1;  /**< Received ECRC Error
                                                         pedc_radm_trgt1_ecrc_err & pedc__radm_trgt1_eot */
	uint64_t rtlplle                      : 1;  /**< Received TLP has link layer error
                                                         pedc_radm_trgt1_dllp_abort & pedc__radm_trgt1_eot */
	uint64_t rtlpmal                      : 1;  /**< Received TLP is malformed or a message.
                                                         pedc_radm_trgt1_tlp_abort & pedc__radm_trgt1_eot
                                                         If the core receives a MSG (or Vendor Message)
                                                         or if a received AtomicOp viloates address/length rules,
                                                         this bit is set as well. */
	uint64_t spoison                      : 1;  /**< Poisoned TLP sent
                                                         peai__client0_tlp_ep & peai__client0_tlp_hv
                                                         peai__client1_tlp_ep & peai__client1_tlp_hv (atomic_op).
                                                         Throws PEM_INTSN_E::PEM(0..3)_ERROR_SPOISON. */
#else
	uint64_t spoison                      : 1;
	uint64_t rtlpmal                      : 1;
	uint64_t rtlplle                      : 1;
	uint64_t recrce                       : 1;
	uint64_t rpoison                      : 1;
	uint64_t rcemrc                       : 1;
	uint64_t rnfemrc                      : 1;
	uint64_t rfemrc                       : 1;
	uint64_t rpmerc                       : 1;
	uint64_t rptamrc                      : 1;
	uint64_t rumep                        : 1;
	uint64_t rvdm                         : 1;
	uint64_t acto                         : 1;
	uint64_t rte                          : 1;
	uint64_t mre                          : 1;
	uint64_t rdwdle                       : 1;
	uint64_t rtwdle                       : 1;
	uint64_t dpeoosd                      : 1;
	uint64_t fcpvwt                       : 1;
	uint64_t rpe                          : 1;
	uint64_t fcuv                         : 1;
	uint64_t rqo                          : 1;
	uint64_t rauc                         : 1;
	uint64_t racur                        : 1;
	uint64_t racca                        : 1;
	uint64_t caar                         : 1;
	uint64_t rarwdns                      : 1;
	uint64_t ramtlp                       : 1;
	uint64_t racpp                        : 1;
	uint64_t rawwpp                       : 1;
	uint64_t ecrc_e                       : 1;
	uint64_t lofp                         : 1;
	uint64_t reserved_32_32               : 1;
	uint64_t datq_pe                      : 1;
	uint64_t p_d0_sbe                     : 1;
	uint64_t p_d0_dbe                     : 1;
	uint64_t p_d1_sbe                     : 1;
	uint64_t p_d1_dbe                     : 1;
	uint64_t p_c_sbe                      : 1;
	uint64_t p_c_dbe                      : 1;
	uint64_t n_d0_sbe                     : 1;
	uint64_t n_d0_dbe                     : 1;
	uint64_t n_d1_sbe                     : 1;
	uint64_t n_d1_dbe                     : 1;
	uint64_t n_c_sbe                      : 1;
	uint64_t n_c_dbe                      : 1;
	uint64_t c_d0_sbe                     : 1;
	uint64_t c_d0_dbe                     : 1;
	uint64_t c_d1_sbe                     : 1;
	uint64_t c_d1_dbe                     : 1;
	uint64_t c_c_sbe                      : 1;
	uint64_t c_c_dbe                      : 1;
	uint64_t rtry_sbe                     : 1;
	uint64_t rtry_dbe                     : 1;
	uint64_t qhdr_b0_sbe                  : 1;
	uint64_t qhdr_b0_dbe                  : 1;
	uint64_t qhdr_b1_sbe                  : 1;
	uint64_t qhdr_b1_dbe                  : 1;
	uint64_t reserved_58_63               : 6;
#endif
	} cn78xx;
	struct cvmx_pemx_dbg_info_cn61xx      cnf71xx;
};
typedef union cvmx_pemx_dbg_info cvmx_pemx_dbg_info_t;

/**
 * cvmx_pem#_dbg_info_en
 *
 * "PEM#_DBG_INFO_EN = PEM Debug Information Enable
 * Allows PEM_DBG_INFO to generate interrupts when cooresponding enable bit is set."
 */
union cvmx_pemx_dbg_info_en {
	uint64_t u64;
	struct cvmx_pemx_dbg_info_en_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_46_63               : 18;
	uint64_t tpcdbe1                      : 1;  /**< Allows PEM_DBG_INFO[45] to generate an interrupt. */
	uint64_t tpcsbe1                      : 1;  /**< Allows PEM_DBG_INFO[44] to generate an interrupt. */
	uint64_t tpcdbe0                      : 1;  /**< Allows PEM_DBG_INFO[43] to generate an interrupt. */
	uint64_t tpcsbe0                      : 1;  /**< Allows PEM_DBG_INFO[42] to generate an interrupt. */
	uint64_t tnfdbe1                      : 1;  /**< Allows PEM_DBG_INFO[41] to generate an interrupt. */
	uint64_t tnfsbe1                      : 1;  /**< Allows PEM_DBG_INFO[40] to generate an interrupt. */
	uint64_t tnfdbe0                      : 1;  /**< Allows PEM_DBG_INFO[39] to generate an interrupt. */
	uint64_t tnfsbe0                      : 1;  /**< Allows PEM_DBG_INFO[38] to generate an interrupt. */
	uint64_t tpfdbe1                      : 1;  /**< Allows PEM_DBG_INFO[37] to generate an interrupt. */
	uint64_t tpfsbe1                      : 1;  /**< Allows PEM_DBG_INFO[36] to generate an interrupt. */
	uint64_t tpfdbe0                      : 1;  /**< Allows PEM_DBG_INFO[35] to generate an interrupt. */
	uint64_t tpfsbe0                      : 1;  /**< Allows PEM_DBG_INFO[34] to generate an interrupt. */
	uint64_t datq_pe                      : 1;  /**< Allows PEM_DBG_INFO[33] to generate an interrupt. */
	uint64_t hdrq_pe                      : 1;  /**< Allows PEM_DBG_INFO[32] to generate an interrupt. */
	uint64_t rtry_pe                      : 1;  /**< Allows PEM_DBG_INFO[31] to generate an interrupt. */
	uint64_t ecrc_e                       : 1;  /**< Allows PEM_DBG_INFO[30] to generate an interrupt. */
	uint64_t rawwpp                       : 1;  /**< Allows PEM_DBG_INFO[29] to generate an interrupt. */
	uint64_t racpp                        : 1;  /**< Allows PEM_DBG_INFO[28] to generate an interrupt. */
	uint64_t ramtlp                       : 1;  /**< Allows PEM_DBG_INFO[27] to generate an interrupt. */
	uint64_t rarwdns                      : 1;  /**< Allows PEM_DBG_INFO[26] to generate an interrupt. */
	uint64_t caar                         : 1;  /**< Allows PEM_DBG_INFO[25] to generate an interrupt. */
	uint64_t racca                        : 1;  /**< Allows PEM_DBG_INFO[24] to generate an interrupt. */
	uint64_t racur                        : 1;  /**< Allows PEM_DBG_INFO[23] to generate an interrupt. */
	uint64_t rauc                         : 1;  /**< Allows PEM_DBG_INFO[22] to generate an interrupt. */
	uint64_t rqo                          : 1;  /**< Allows PEM_DBG_INFO[21] to generate an interrupt. */
	uint64_t fcuv                         : 1;  /**< Allows PEM_DBG_INFO[20] to generate an interrupt. */
	uint64_t rpe                          : 1;  /**< Allows PEM_DBG_INFO[19] to generate an interrupt. */
	uint64_t fcpvwt                       : 1;  /**< Allows PEM_DBG_INFO[18] to generate an interrupt. */
	uint64_t dpeoosd                      : 1;  /**< Allows PEM_DBG_INFO[17] to generate an interrupt. */
	uint64_t rtwdle                       : 1;  /**< Allows PEM_DBG_INFO[16] to generate an interrupt. */
	uint64_t rdwdle                       : 1;  /**< Allows PEM_DBG_INFO[15] to generate an interrupt. */
	uint64_t mre                          : 1;  /**< Allows PEM_DBG_INFO[14] to generate an interrupt. */
	uint64_t rte                          : 1;  /**< Allows PEM_DBG_INFO[13] to generate an interrupt. */
	uint64_t acto                         : 1;  /**< Allows PEM_DBG_INFO[12] to generate an interrupt. */
	uint64_t rvdm                         : 1;  /**< Allows PEM_DBG_INFO[11] to generate an interrupt. */
	uint64_t rumep                        : 1;  /**< Allows PEM_DBG_INFO[10] to generate an interrupt. */
	uint64_t rptamrc                      : 1;  /**< Allows PEM_DBG_INFO[9] to generate an interrupt. */
	uint64_t rpmerc                       : 1;  /**< Allows PEM_DBG_INFO[8] to generate an interrupt. */
	uint64_t rfemrc                       : 1;  /**< Allows PEM_DBG_INFO[7] to generate an interrupt. */
	uint64_t rnfemrc                      : 1;  /**< Allows PEM_DBG_INFO[6] to generate an interrupt. */
	uint64_t rcemrc                       : 1;  /**< Allows PEM_DBG_INFO[5] to generate an interrupt. */
	uint64_t rpoison                      : 1;  /**< Allows PEM_DBG_INFO[4] to generate an interrupt. */
	uint64_t recrce                       : 1;  /**< Allows PEM_DBG_INFO[3] to generate an interrupt. */
	uint64_t rtlplle                      : 1;  /**< Allows PEM_DBG_INFO[2] to generate an interrupt. */
	uint64_t rtlpmal                      : 1;  /**< Allows PEM_DBG_INFO[1] to generate an interrupt. */
	uint64_t spoison                      : 1;  /**< Allows PEM_DBG_INFO[0] to generate an interrupt. */
#else
	uint64_t spoison                      : 1;
	uint64_t rtlpmal                      : 1;
	uint64_t rtlplle                      : 1;
	uint64_t recrce                       : 1;
	uint64_t rpoison                      : 1;
	uint64_t rcemrc                       : 1;
	uint64_t rnfemrc                      : 1;
	uint64_t rfemrc                       : 1;
	uint64_t rpmerc                       : 1;
	uint64_t rptamrc                      : 1;
	uint64_t rumep                        : 1;
	uint64_t rvdm                         : 1;
	uint64_t acto                         : 1;
	uint64_t rte                          : 1;
	uint64_t mre                          : 1;
	uint64_t rdwdle                       : 1;
	uint64_t rtwdle                       : 1;
	uint64_t dpeoosd                      : 1;
	uint64_t fcpvwt                       : 1;
	uint64_t rpe                          : 1;
	uint64_t fcuv                         : 1;
	uint64_t rqo                          : 1;
	uint64_t rauc                         : 1;
	uint64_t racur                        : 1;
	uint64_t racca                        : 1;
	uint64_t caar                         : 1;
	uint64_t rarwdns                      : 1;
	uint64_t ramtlp                       : 1;
	uint64_t racpp                        : 1;
	uint64_t rawwpp                       : 1;
	uint64_t ecrc_e                       : 1;
	uint64_t rtry_pe                      : 1;
	uint64_t hdrq_pe                      : 1;
	uint64_t datq_pe                      : 1;
	uint64_t tpfsbe0                      : 1;
	uint64_t tpfdbe0                      : 1;
	uint64_t tpfsbe1                      : 1;
	uint64_t tpfdbe1                      : 1;
	uint64_t tnfsbe0                      : 1;
	uint64_t tnfdbe0                      : 1;
	uint64_t tnfsbe1                      : 1;
	uint64_t tnfdbe1                      : 1;
	uint64_t tpcsbe0                      : 1;
	uint64_t tpcdbe0                      : 1;
	uint64_t tpcsbe1                      : 1;
	uint64_t tpcdbe1                      : 1;
	uint64_t reserved_46_63               : 18;
#endif
	} s;
	struct cvmx_pemx_dbg_info_en_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_31_63               : 33;
	uint64_t ecrc_e                       : 1;  /**< Allows PEM_DBG_INFO[30] to generate an interrupt. */
	uint64_t rawwpp                       : 1;  /**< Allows PEM_DBG_INFO[29] to generate an interrupt. */
	uint64_t racpp                        : 1;  /**< Allows PEM_DBG_INFO[28] to generate an interrupt. */
	uint64_t ramtlp                       : 1;  /**< Allows PEM_DBG_INFO[27] to generate an interrupt. */
	uint64_t rarwdns                      : 1;  /**< Allows PEM_DBG_INFO[26] to generate an interrupt. */
	uint64_t caar                         : 1;  /**< Allows PEM_DBG_INFO[25] to generate an interrupt. */
	uint64_t racca                        : 1;  /**< Allows PEM_DBG_INFO[24] to generate an interrupt. */
	uint64_t racur                        : 1;  /**< Allows PEM_DBG_INFO[23] to generate an interrupt. */
	uint64_t rauc                         : 1;  /**< Allows PEM_DBG_INFO[22] to generate an interrupt. */
	uint64_t rqo                          : 1;  /**< Allows PEM_DBG_INFO[21] to generate an interrupt. */
	uint64_t fcuv                         : 1;  /**< Allows PEM_DBG_INFO[20] to generate an interrupt. */
	uint64_t rpe                          : 1;  /**< Allows PEM_DBG_INFO[19] to generate an interrupt. */
	uint64_t fcpvwt                       : 1;  /**< Allows PEM_DBG_INFO[18] to generate an interrupt. */
	uint64_t dpeoosd                      : 1;  /**< Allows PEM_DBG_INFO[17] to generate an interrupt. */
	uint64_t rtwdle                       : 1;  /**< Allows PEM_DBG_INFO[16] to generate an interrupt. */
	uint64_t rdwdle                       : 1;  /**< Allows PEM_DBG_INFO[15] to generate an interrupt. */
	uint64_t mre                          : 1;  /**< Allows PEM_DBG_INFO[14] to generate an interrupt. */
	uint64_t rte                          : 1;  /**< Allows PEM_DBG_INFO[13] to generate an interrupt. */
	uint64_t acto                         : 1;  /**< Allows PEM_DBG_INFO[12] to generate an interrupt. */
	uint64_t rvdm                         : 1;  /**< Allows PEM_DBG_INFO[11] to generate an interrupt. */
	uint64_t rumep                        : 1;  /**< Allows PEM_DBG_INFO[10] to generate an interrupt. */
	uint64_t rptamrc                      : 1;  /**< Allows PEM_DBG_INFO[9] to generate an interrupt. */
	uint64_t rpmerc                       : 1;  /**< Allows PEM_DBG_INFO[8] to generate an interrupt. */
	uint64_t rfemrc                       : 1;  /**< Allows PEM_DBG_INFO[7] to generate an interrupt. */
	uint64_t rnfemrc                      : 1;  /**< Allows PEM_DBG_INFO[6] to generate an interrupt. */
	uint64_t rcemrc                       : 1;  /**< Allows PEM_DBG_INFO[5] to generate an interrupt. */
	uint64_t rpoison                      : 1;  /**< Allows PEM_DBG_INFO[4] to generate an interrupt. */
	uint64_t recrce                       : 1;  /**< Allows PEM_DBG_INFO[3] to generate an interrupt. */
	uint64_t rtlplle                      : 1;  /**< Allows PEM_DBG_INFO[2] to generate an interrupt. */
	uint64_t rtlpmal                      : 1;  /**< Allows PEM_DBG_INFO[1] to generate an interrupt. */
	uint64_t spoison                      : 1;  /**< Allows PEM_DBG_INFO[0] to generate an interrupt. */
#else
	uint64_t spoison                      : 1;
	uint64_t rtlpmal                      : 1;
	uint64_t rtlplle                      : 1;
	uint64_t recrce                       : 1;
	uint64_t rpoison                      : 1;
	uint64_t rcemrc                       : 1;
	uint64_t rnfemrc                      : 1;
	uint64_t rfemrc                       : 1;
	uint64_t rpmerc                       : 1;
	uint64_t rptamrc                      : 1;
	uint64_t rumep                        : 1;
	uint64_t rvdm                         : 1;
	uint64_t acto                         : 1;
	uint64_t rte                          : 1;
	uint64_t mre                          : 1;
	uint64_t rdwdle                       : 1;
	uint64_t rtwdle                       : 1;
	uint64_t dpeoosd                      : 1;
	uint64_t fcpvwt                       : 1;
	uint64_t rpe                          : 1;
	uint64_t fcuv                         : 1;
	uint64_t rqo                          : 1;
	uint64_t rauc                         : 1;
	uint64_t racur                        : 1;
	uint64_t racca                        : 1;
	uint64_t caar                         : 1;
	uint64_t rarwdns                      : 1;
	uint64_t ramtlp                       : 1;
	uint64_t racpp                        : 1;
	uint64_t rawwpp                       : 1;
	uint64_t ecrc_e                       : 1;
	uint64_t reserved_31_63               : 33;
#endif
	} cn61xx;
	struct cvmx_pemx_dbg_info_en_cn61xx   cn63xx;
	struct cvmx_pemx_dbg_info_en_cn61xx   cn63xxp1;
	struct cvmx_pemx_dbg_info_en_cn61xx   cn66xx;
	struct cvmx_pemx_dbg_info_en_cn61xx   cn68xx;
	struct cvmx_pemx_dbg_info_en_cn61xx   cn68xxp1;
	struct cvmx_pemx_dbg_info_en_s        cn70xx;
	struct cvmx_pemx_dbg_info_en_cn61xx   cnf71xx;
};
typedef union cvmx_pemx_dbg_info_en cvmx_pemx_dbg_info_en_t;

/**
 * cvmx_pem#_diag_status
 *
 * Selection control for the cores diagnostic bus.
 *
 */
union cvmx_pemx_diag_status {
	uint64_t u64;
	struct cvmx_pemx_diag_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t pm_dst                       : 3;  /**< Current power management DSTATE. */
	uint64_t pm_stat                      : 1;  /**< Power Management Status. */
	uint64_t pm_en                        : 1;  /**< Power Management Event Enable. */
	uint64_t aux_en                       : 1;  /**< Auxilary Power Enable. */
#else
	uint64_t aux_en                       : 1;
	uint64_t pm_en                        : 1;
	uint64_t pm_stat                      : 1;
	uint64_t pm_dst                       : 3;
	uint64_t reserved_6_63                : 58;
#endif
	} s;
	struct cvmx_pemx_diag_status_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t pm_dst                       : 1;  /**< Current power management DSTATE. */
	uint64_t pm_stat                      : 1;  /**< Power Management Status. */
	uint64_t pm_en                        : 1;  /**< Power Management Event Enable. */
	uint64_t aux_en                       : 1;  /**< Auxilary Power Enable. */
#else
	uint64_t aux_en                       : 1;
	uint64_t pm_en                        : 1;
	uint64_t pm_stat                      : 1;
	uint64_t pm_dst                       : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} cn61xx;
	struct cvmx_pemx_diag_status_cn61xx   cn63xx;
	struct cvmx_pemx_diag_status_cn61xx   cn63xxp1;
	struct cvmx_pemx_diag_status_cn61xx   cn66xx;
	struct cvmx_pemx_diag_status_cn61xx   cn68xx;
	struct cvmx_pemx_diag_status_cn61xx   cn68xxp1;
	struct cvmx_pemx_diag_status_s        cn70xx;
	struct cvmx_pemx_diag_status_s        cn78xx;
	struct cvmx_pemx_diag_status_cn61xx   cnf71xx;
};
typedef union cvmx_pemx_diag_status cvmx_pemx_diag_status_t;

/**
 * cvmx_pem#_ecc_ena
 *
 * Contains enables for TLP FIFO ECC RAMs
 *
 */
union cvmx_pemx_ecc_ena {
	uint64_t u64;
	struct cvmx_pemx_ecc_ena_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_35_63               : 29;
	uint64_t qhdr_b1_ena                  : 1;  /**< ECC enable for Core's Q HDR Bank1 RAM */
	uint64_t qhdr_b0_ena                  : 1;  /**< ECC enable for Core's Q HDR Bank0 RAM */
	uint64_t rtry_ena                     : 1;  /**< ECC enable for Core's RETRY RAM */
	uint64_t reserved_9_31                : 23;
	uint64_t c_c_ena                      : 1;  /**< ECC enable for TLP CPL ctl Fifo */
	uint64_t c_d1_ena                     : 1;  /**< ECC enable for TLP CPL data1 Fifo */
	uint64_t c_d0_ena                     : 1;  /**< ECC enable for TLP CPL data0 Fifo */
	uint64_t reserved_0_5                 : 6;
#else
	uint64_t reserved_0_5                 : 6;
	uint64_t c_d0_ena                     : 1;
	uint64_t c_d1_ena                     : 1;
	uint64_t c_c_ena                      : 1;
	uint64_t reserved_9_31                : 23;
	uint64_t rtry_ena                     : 1;
	uint64_t qhdr_b0_ena                  : 1;
	uint64_t qhdr_b1_ena                  : 1;
	uint64_t reserved_35_63               : 29;
#endif
	} s;
	struct cvmx_pemx_ecc_ena_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t tlp_nc_ena                   : 1;  /**< ECC enable for TLP Non-Posted ctl Fifo */
	uint64_t tlp_nd_ena                   : 1;  /**< ECC enable for TLP Non-Posted data Fifo */
	uint64_t tlp_pc_ena                   : 1;  /**< ECC enable for TLP Posted ctl Fifo */
	uint64_t tlp_pd_ena                   : 1;  /**< ECC enable for TLP Posted data Fifo */
	uint64_t tlp_cc_ena                   : 1;  /**< ECC enable for TLP CPL ctl Fifo */
	uint64_t tlp_cd_ena                   : 1;  /**< ECC enable for TLP CPL data Fifo */
#else
	uint64_t tlp_cd_ena                   : 1;
	uint64_t tlp_cc_ena                   : 1;
	uint64_t tlp_pd_ena                   : 1;
	uint64_t tlp_pc_ena                   : 1;
	uint64_t tlp_nd_ena                   : 1;
	uint64_t tlp_nc_ena                   : 1;
	uint64_t reserved_6_63                : 58;
#endif
	} cn70xx;
	struct cvmx_pemx_ecc_ena_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_35_63               : 29;
	uint64_t qhdr_b1_ena                  : 1;  /**< ECC enable for Core's Q HDR Bank1 RAM */
	uint64_t qhdr_b0_ena                  : 1;  /**< ECC enable for Core's Q HDR Bank0 RAM */
	uint64_t rtry_ena                     : 1;  /**< ECC enable for Core's RETRY RAM */
	uint64_t reserved_9_31                : 23;
	uint64_t c_c_ena                      : 1;  /**< ECC enable for TLP CPL ctl Fifo */
	uint64_t c_d1_ena                     : 1;  /**< ECC enable for TLP CPL data1 Fifo */
	uint64_t c_d0_ena                     : 1;  /**< ECC enable for TLP CPL data0 Fifo */
	uint64_t n_c_ena                      : 1;  /**< ECC enable for TLP NP ctl Fifo */
	uint64_t n_d1_ena                     : 1;  /**< ECC enable for TLP NP data1 Fifo */
	uint64_t n_d0_ena                     : 1;  /**< ECC enable for TLP NP data0 Fifo */
	uint64_t p_c_ena                      : 1;  /**< ECC enable for TLP Posted ctl Fifo */
	uint64_t p_d1_ena                     : 1;  /**< ECC enable for TLP Posted data1 Fifo */
	uint64_t p_d0_ena                     : 1;  /**< ECC enable for TLP Posted data0 ifo */
#else
	uint64_t p_d0_ena                     : 1;
	uint64_t p_d1_ena                     : 1;
	uint64_t p_c_ena                      : 1;
	uint64_t n_d0_ena                     : 1;
	uint64_t n_d1_ena                     : 1;
	uint64_t n_c_ena                      : 1;
	uint64_t c_d0_ena                     : 1;
	uint64_t c_d1_ena                     : 1;
	uint64_t c_c_ena                      : 1;
	uint64_t reserved_9_31                : 23;
	uint64_t rtry_ena                     : 1;
	uint64_t qhdr_b0_ena                  : 1;
	uint64_t qhdr_b1_ena                  : 1;
	uint64_t reserved_35_63               : 29;
#endif
	} cn78xx;
};
typedef union cvmx_pemx_ecc_ena cvmx_pemx_ecc_ena_t;

/**
 * cvmx_pem#_ecc_synd_ctrl
 *
 * PEM_ECC_SYND_CTL
 * Contains Syndrome Control for TLP FIFO ECC RAMs
 */
union cvmx_pemx_ecc_synd_ctrl {
	uint64_t u64;
	struct cvmx_pemx_ecc_synd_ctrl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_38_63               : 26;
	uint64_t qhdr_b1_syn                  : 2;  /**< Syndrome Flip bits for Core's Q HDR Bank1 RAM */
	uint64_t qhdr_b0_syn                  : 2;  /**< Syndrome Flip bits for Core's Q HDR Bank0 RAM */
	uint64_t rtry_syn                     : 2;  /**< Syndrome Flip bits for Core's RETRY RAM */
	uint64_t reserved_18_31               : 14;
	uint64_t c_c_syn                      : 2;  /**< Syndrome Flip bits for TLP CPL ctl Fifo */
	uint64_t c_d1_syn                     : 2;  /**< Syndrome Flip bits for TLP CPL data1 Fifo */
	uint64_t c_d0_syn                     : 2;  /**< Syndrome Flip bits for TLP CPL data0 Fifo */
	uint64_t reserved_0_11                : 12;
#else
	uint64_t reserved_0_11                : 12;
	uint64_t c_d0_syn                     : 2;
	uint64_t c_d1_syn                     : 2;
	uint64_t c_c_syn                      : 2;
	uint64_t reserved_18_31               : 14;
	uint64_t rtry_syn                     : 2;
	uint64_t qhdr_b0_syn                  : 2;
	uint64_t qhdr_b1_syn                  : 2;
	uint64_t reserved_38_63               : 26;
#endif
	} s;
	struct cvmx_pemx_ecc_synd_ctrl_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t tlp_nc_syn                   : 2;  /**< Syndrome Flip bits for TLP Non-Posted ctl Fifo */
	uint64_t tlp_nd_syn                   : 2;  /**< Syndrome Flip bits for TLP Non-Posted data Fifo */
	uint64_t tlp_pc_syn                   : 2;  /**< Syndrome Flip bits for TLP Posted ctl Fifo */
	uint64_t tlp_pd_syn                   : 2;  /**< Syndrome Flip bits for TLP Posted data Fifo */
	uint64_t tlp_cc_syn                   : 2;  /**< Syndrome Flip bits for TLP CPL ctl Fifo */
	uint64_t tlp_cd_syn                   : 2;  /**< Syndrome Flip bits for TLP CPL data Fifo */
#else
	uint64_t tlp_cd_syn                   : 2;
	uint64_t tlp_cc_syn                   : 2;
	uint64_t tlp_pd_syn                   : 2;
	uint64_t tlp_pc_syn                   : 2;
	uint64_t tlp_nd_syn                   : 2;
	uint64_t tlp_nc_syn                   : 2;
	uint64_t reserved_12_63               : 52;
#endif
	} cn70xx;
	struct cvmx_pemx_ecc_synd_ctrl_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_38_63               : 26;
	uint64_t qhdr_b1_syn                  : 2;  /**< Syndrome Flip bits for Core's Q HDR Bank1 RAM */
	uint64_t qhdr_b0_syn                  : 2;  /**< Syndrome Flip bits for Core's Q HDR Bank0 RAM */
	uint64_t rtry_syn                     : 2;  /**< Syndrome Flip bits for Core's RETRY RAM */
	uint64_t reserved_18_31               : 14;
	uint64_t c_c_syn                      : 2;  /**< Syndrome Flip bits for TLP CPL ctl Fifo */
	uint64_t c_d1_syn                     : 2;  /**< Syndrome Flip bits for TLP CPL data1 Fifo */
	uint64_t c_d0_syn                     : 2;  /**< Syndrome Flip bits for TLP CPL data0 Fifo */
	uint64_t n_c_syn                      : 2;  /**< Syndrome Flip bits for TLP NP ctl Fifo */
	uint64_t n_d1_syn                     : 2;  /**< Syndrome Flip bits for TLP NP data1 Fifo */
	uint64_t n_d0_syn                     : 2;  /**< Syndrome Flip bits for TLP NP data0 Fifo */
	uint64_t p_c_syn                      : 2;  /**< Syndrome Flip bits for TLP Posted ctl Fifo */
	uint64_t p_d1_syn                     : 2;  /**< Syndrome Flip bits for TLP Posted data1 Fifo */
	uint64_t p_d0_syn                     : 2;  /**< Syndrome Flip bits for TLP Posted data0 Fifo */
#else
	uint64_t p_d0_syn                     : 2;
	uint64_t p_d1_syn                     : 2;
	uint64_t p_c_syn                      : 2;
	uint64_t n_d0_syn                     : 2;
	uint64_t n_d1_syn                     : 2;
	uint64_t n_c_syn                      : 2;
	uint64_t c_d0_syn                     : 2;
	uint64_t c_d1_syn                     : 2;
	uint64_t c_c_syn                      : 2;
	uint64_t reserved_18_31               : 14;
	uint64_t rtry_syn                     : 2;
	uint64_t qhdr_b0_syn                  : 2;
	uint64_t qhdr_b1_syn                  : 2;
	uint64_t reserved_38_63               : 26;
#endif
	} cn78xx;
};
typedef union cvmx_pemx_ecc_synd_ctrl cvmx_pemx_ecc_synd_ctrl_t;

/**
 * cvmx_pem#_inb_read_credits
 *
 * The number of in flight reads from PCIe core to SLI
 *
 */
union cvmx_pemx_inb_read_credits {
	uint64_t u64;
	struct cvmx_pemx_inb_read_credits_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t num                          : 7;  /**< The number of reads that may be in flight from
                                                         the PCIe core to the SLI. Min number is 2 max
                                                         number is 32. */
#else
	uint64_t num                          : 7;
	uint64_t reserved_7_63                : 57;
#endif
	} s;
	struct cvmx_pemx_inb_read_credits_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t num                          : 6;  /**< The number of reads that may be in flight from
                                                         the PCIe core to the SLI. Min number is 2 max
                                                         number is 32. */
#else
	uint64_t num                          : 6;
	uint64_t reserved_6_63                : 58;
#endif
	} cn61xx;
	struct cvmx_pemx_inb_read_credits_cn61xx cn66xx;
	struct cvmx_pemx_inb_read_credits_cn61xx cn68xx;
	struct cvmx_pemx_inb_read_credits_cn61xx cn70xx;
	struct cvmx_pemx_inb_read_credits_s   cn78xx;
	struct cvmx_pemx_inb_read_credits_cn61xx cnf71xx;
};
typedef union cvmx_pemx_inb_read_credits cvmx_pemx_inb_read_credits_t;

/**
 * cvmx_pem#_int_enb
 *
 * "PEM#_INT_ENB = PEM Interrupt Enable
 * Enables interrupt conditions for the PEM to generate an RSL interrupt."
 */
union cvmx_pemx_int_enb {
	uint64_t u64;
	struct cvmx_pemx_int_enb_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t crs_dr                       : 1;  /**< Enables PEM_INT_SUM[13] to generate an
                                                         interrupt to the MIO. */
	uint64_t crs_er                       : 1;  /**< Enables PEM_INT_SUM[12] to generate an
                                                         interrupt to the MIO. */
	uint64_t rdlk                         : 1;  /**< Enables PEM_INT_SUM[11] to generate an
                                                         interrupt to the MIO. */
	uint64_t exc                          : 1;  /**< Enables PEM_INT_SUM[10] to generate an
                                                         interrupt to the MIO. */
	uint64_t un_bx                        : 1;  /**< Enables PEM_INT_SUM[9] to generate an
                                                         interrupt to the MIO. */
	uint64_t un_b2                        : 1;  /**< Enables PEM_INT_SUM[8] to generate an
                                                         interrupt to the MIO. */
	uint64_t un_b1                        : 1;  /**< Enables PEM_INT_SUM[7] to generate an
                                                         interrupt to the MIO. */
	uint64_t up_bx                        : 1;  /**< Enables PEM_INT_SUM[6] to generate an
                                                         interrupt to the MIO. */
	uint64_t up_b2                        : 1;  /**< Enables PEM_INT_SUM[5] to generate an
                                                         interrupt to the MIO. */
	uint64_t up_b1                        : 1;  /**< Enables PEM_INT_SUM[4] to generate an
                                                         interrupt to the MIO. */
	uint64_t pmem                         : 1;  /**< Enables PEM_INT_SUM[3] to generate an
                                                         interrupt to the MIO. */
	uint64_t pmei                         : 1;  /**< Enables PEM_INT_SUM[2] to generate an
                                                         interrupt to the MIO. */
	uint64_t se                           : 1;  /**< Enables PEM_INT_SUM[1] to generate an
                                                         interrupt to the MIO. */
	uint64_t aeri                         : 1;  /**< Enables PEM_INT_SUM[0] to generate an
                                                         interrupt to the MIO. */
#else
	uint64_t aeri                         : 1;
	uint64_t se                           : 1;
	uint64_t pmei                         : 1;
	uint64_t pmem                         : 1;
	uint64_t up_b1                        : 1;
	uint64_t up_b2                        : 1;
	uint64_t up_bx                        : 1;
	uint64_t un_b1                        : 1;
	uint64_t un_b2                        : 1;
	uint64_t un_bx                        : 1;
	uint64_t exc                          : 1;
	uint64_t rdlk                         : 1;
	uint64_t crs_er                       : 1;
	uint64_t crs_dr                       : 1;
	uint64_t reserved_14_63               : 50;
#endif
	} s;
	struct cvmx_pemx_int_enb_s            cn61xx;
	struct cvmx_pemx_int_enb_s            cn63xx;
	struct cvmx_pemx_int_enb_s            cn63xxp1;
	struct cvmx_pemx_int_enb_s            cn66xx;
	struct cvmx_pemx_int_enb_s            cn68xx;
	struct cvmx_pemx_int_enb_s            cn68xxp1;
	struct cvmx_pemx_int_enb_s            cn70xx;
	struct cvmx_pemx_int_enb_s            cnf71xx;
};
typedef union cvmx_pemx_int_enb cvmx_pemx_int_enb_t;

/**
 * cvmx_pem#_int_enb_int
 *
 * "PEM#_INT_ENB_INT = PEM Interrupt Enable
 * Enables interrupt conditions for the PEM to generate an RSL interrupt."
 */
union cvmx_pemx_int_enb_int {
	uint64_t u64;
	struct cvmx_pemx_int_enb_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t crs_dr                       : 1;  /**< Enables PEM_INT_SUM[13] to generate an
                                                         interrupt to the SLI as SLI_INT_SUM[MAC#_INT]. */
	uint64_t crs_er                       : 1;  /**< Enables PEM_INT_SUM[12] to generate an
                                                         interrupt to the SLI as SLI_INT_SUM[MAC#_INT]. */
	uint64_t rdlk                         : 1;  /**< Enables PEM_INT_SUM[11] to generate an
                                                         interrupt to the SLI as SLI_INT_SUM[MAC#_INT]. */
	uint64_t exc                          : 1;  /**< Enables PEM_INT_SUM[10] to generate an
                                                         interrupt to the SLI as SLI_INT_SUM[MAC#_INT]. */
	uint64_t un_bx                        : 1;  /**< Enables PEM_INT_SUM[9] to generate an
                                                         interrupt to the SLI as SLI_INT_SUM[MAC#_INT]. */
	uint64_t un_b2                        : 1;  /**< Enables PEM_INT_SUM[8] to generate an
                                                         interrupt to the SLI as SLI_INT_SUM[MAC#_INT]. */
	uint64_t un_b1                        : 1;  /**< Enables PEM_INT_SUM[7] to generate an
                                                         interrupt to the SLI as SLI_INT_SUM[MAC#_INT]. */
	uint64_t up_bx                        : 1;  /**< Enables PEM_INT_SUM[6] to generate an
                                                         interrupt to the SLI as SLI_INT_SUM[MAC#_INT]. */
	uint64_t up_b2                        : 1;  /**< Enables PEM_INT_SUM[5] to generate an
                                                         interrupt to the SLI as SLI_INT_SUM[MAC#_INT]. */
	uint64_t up_b1                        : 1;  /**< Enables PEM_INT_SUM[4] to generate an
                                                         interrupt to the SLI as SLI_INT_SUM[MAC#_INT]. */
	uint64_t pmem                         : 1;  /**< Enables PEM_INT_SUM[3] to generate an
                                                         interrupt to the SLI as SLI_INT_SUM[MAC#_INT]. */
	uint64_t pmei                         : 1;  /**< Enables PEM_INT_SUM[2] to generate an
                                                         interrupt to the SLI as SLI_INT_SUM[MAC#_INT]. */
	uint64_t se                           : 1;  /**< Enables PEM_INT_SUM[1] to generate an
                                                         interrupt to the SLI as SLI_INT_SUM[MAC#_INT]. */
	uint64_t aeri                         : 1;  /**< Enables PEM_INT_SUM[0] to generate an
                                                         interrupt to the SLI as SLI_INT_SUM[MAC#_INT]. */
#else
	uint64_t aeri                         : 1;
	uint64_t se                           : 1;
	uint64_t pmei                         : 1;
	uint64_t pmem                         : 1;
	uint64_t up_b1                        : 1;
	uint64_t up_b2                        : 1;
	uint64_t up_bx                        : 1;
	uint64_t un_b1                        : 1;
	uint64_t un_b2                        : 1;
	uint64_t un_bx                        : 1;
	uint64_t exc                          : 1;
	uint64_t rdlk                         : 1;
	uint64_t crs_er                       : 1;
	uint64_t crs_dr                       : 1;
	uint64_t reserved_14_63               : 50;
#endif
	} s;
	struct cvmx_pemx_int_enb_int_s        cn61xx;
	struct cvmx_pemx_int_enb_int_s        cn63xx;
	struct cvmx_pemx_int_enb_int_s        cn63xxp1;
	struct cvmx_pemx_int_enb_int_s        cn66xx;
	struct cvmx_pemx_int_enb_int_s        cn68xx;
	struct cvmx_pemx_int_enb_int_s        cn68xxp1;
	struct cvmx_pemx_int_enb_int_s        cn70xx;
	struct cvmx_pemx_int_enb_int_s        cnf71xx;
};
typedef union cvmx_pemx_int_enb_int cvmx_pemx_int_enb_int_t;

/**
 * cvmx_pem#_int_sum
 *
 * "PEM#_INT_SUM = PEM Interrupt Summary
 * Interrupt conditions for the PEM."
 */
union cvmx_pemx_int_sum {
	uint64_t u64;
	struct cvmx_pemx_int_sum_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t intd                         : 1;  /**< The PCIe controller received an INTD. */
	uint64_t intc                         : 1;  /**< The PCIe controller received an INTC. */
	uint64_t intb                         : 1;  /**< The PCIe controller received an INTB. */
	uint64_t inta                         : 1;  /**< The PCIe controller received an INTA. */
	uint64_t reserved_14_59               : 46;
	uint64_t crs_dr                       : 1;  /**< Had a CRS Timeout when Retries were disabled. */
	uint64_t crs_er                       : 1;  /**< Had a CRS Timeout when Retries were enabled. */
	uint64_t rdlk                         : 1;  /**< Received Read Lock TLP. */
	uint64_t exc                          : 1;  /**< Set when the PEM_DBG_INFO register has a bit
                                                         set and its cooresponding PEM_DBG_INFO_EN bit
                                                         is set. */
	uint64_t un_bx                        : 1;  /**< Received N-TLP for an unknown Bar. */
	uint64_t un_b2                        : 1;  /**< Received N-TLP for Bar2 when bar2 is disabled. */
	uint64_t un_b1                        : 1;  /**< Received N-TLP for Bar1 when bar1 index valid
                                                         is not set. */
	uint64_t up_bx                        : 1;  /**< Received P-TLP for an unknown Bar. */
	uint64_t up_b2                        : 1;  /**< Received P-TLP for Bar2 when bar2 is disabeld. */
	uint64_t up_b1                        : 1;  /**< Received P-TLP for Bar1 when bar1 index valid
                                                         is not set. */
	uint64_t pmem                         : 1;  /**< Recived PME MSG.
                                                         (radm_pm_pme) */
	uint64_t pmei                         : 1;  /**< PME Interrupt.
                                                         (cfg_pme_int) */
	uint64_t se                           : 1;  /**< System Error, RC Mode Only.
                                                         (cfg_sys_err_rc) */
	uint64_t aeri                         : 1;  /**< Advanced Error Reporting Interrupt, RC Mode Only.
                                                         (cfg_aer_rc_err_int). */
#else
	uint64_t aeri                         : 1;
	uint64_t se                           : 1;
	uint64_t pmei                         : 1;
	uint64_t pmem                         : 1;
	uint64_t up_b1                        : 1;
	uint64_t up_b2                        : 1;
	uint64_t up_bx                        : 1;
	uint64_t un_b1                        : 1;
	uint64_t un_b2                        : 1;
	uint64_t un_bx                        : 1;
	uint64_t exc                          : 1;
	uint64_t rdlk                         : 1;
	uint64_t crs_er                       : 1;
	uint64_t crs_dr                       : 1;
	uint64_t reserved_14_59               : 46;
	uint64_t inta                         : 1;
	uint64_t intb                         : 1;
	uint64_t intc                         : 1;
	uint64_t intd                         : 1;
#endif
	} s;
	struct cvmx_pemx_int_sum_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t crs_dr                       : 1;  /**< Had a CRS Timeout when Retries were disabled. */
	uint64_t crs_er                       : 1;  /**< Had a CRS Timeout when Retries were enabled. */
	uint64_t rdlk                         : 1;  /**< Received Read Lock TLP. */
	uint64_t exc                          : 1;  /**< Set when the PEM_DBG_INFO register has a bit
                                                         set and its cooresponding PEM_DBG_INFO_EN bit
                                                         is set. */
	uint64_t un_bx                        : 1;  /**< Received N-TLP for an unknown Bar. */
	uint64_t un_b2                        : 1;  /**< Received N-TLP for Bar2 when bar2 is disabled. */
	uint64_t un_b1                        : 1;  /**< Received N-TLP for Bar1 when bar1 index valid
                                                         is not set. */
	uint64_t up_bx                        : 1;  /**< Received P-TLP for an unknown Bar. */
	uint64_t up_b2                        : 1;  /**< Received P-TLP for Bar2 when bar2 is disabeld. */
	uint64_t up_b1                        : 1;  /**< Received P-TLP for Bar1 when bar1 index valid
                                                         is not set. */
	uint64_t pmem                         : 1;  /**< Recived PME MSG.
                                                         (radm_pm_pme) */
	uint64_t pmei                         : 1;  /**< PME Interrupt.
                                                         (cfg_pme_int) */
	uint64_t se                           : 1;  /**< System Error, RC Mode Only.
                                                         (cfg_sys_err_rc) */
	uint64_t aeri                         : 1;  /**< Advanced Error Reporting Interrupt, RC Mode Only.
                                                         (cfg_aer_rc_err_int). */
#else
	uint64_t aeri                         : 1;
	uint64_t se                           : 1;
	uint64_t pmei                         : 1;
	uint64_t pmem                         : 1;
	uint64_t up_b1                        : 1;
	uint64_t up_b2                        : 1;
	uint64_t up_bx                        : 1;
	uint64_t un_b1                        : 1;
	uint64_t un_b2                        : 1;
	uint64_t un_bx                        : 1;
	uint64_t exc                          : 1;
	uint64_t rdlk                         : 1;
	uint64_t crs_er                       : 1;
	uint64_t crs_dr                       : 1;
	uint64_t reserved_14_63               : 50;
#endif
	} cn61xx;
	struct cvmx_pemx_int_sum_cn61xx       cn63xx;
	struct cvmx_pemx_int_sum_cn61xx       cn63xxp1;
	struct cvmx_pemx_int_sum_cn61xx       cn66xx;
	struct cvmx_pemx_int_sum_cn61xx       cn68xx;
	struct cvmx_pemx_int_sum_cn61xx       cn68xxp1;
	struct cvmx_pemx_int_sum_cn61xx       cn70xx;
	struct cvmx_pemx_int_sum_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t intd                         : 1;  /**< The PCIe controller received an INTD. */
	uint64_t intc                         : 1;  /**< The PCIe controller received an INTC. */
	uint64_t intb                         : 1;  /**< The PCIe controller received an INTB. */
	uint64_t inta                         : 1;  /**< The PCIe controller received an INTA. */
	uint64_t reserved_14_59               : 46;
	uint64_t crs_dr                       : 1;  /**< Had a CRS Timeout when Retries were disabled. */
	uint64_t crs_er                       : 1;  /**< Had a CRS Timeout when Retries were enabled. */
	uint64_t rdlk                         : 1;  /**< Received Read Lock TLP. */
	uint64_t reserved_10_10               : 1;
	uint64_t un_bx                        : 1;  /**< Received N-TLP for an unknown Bar. */
	uint64_t un_b2                        : 1;  /**< Received N-TLP for Bar2 when bar2 is disabled. */
	uint64_t un_b1                        : 1;  /**< Received N-TLP for Bar1 when bar1 index valid
                                                         is not set. */
	uint64_t up_bx                        : 1;  /**< Received P-TLP for an unknown Bar. */
	uint64_t up_b2                        : 1;  /**< Received P-TLP for Bar2 when bar2 is disabeld. */
	uint64_t up_b1                        : 1;  /**< Received P-TLP for Bar1 when bar1 index valid
                                                         is not set. */
	uint64_t reserved_3_3                 : 1;
	uint64_t pmei                         : 1;  /**< PME Interrupt.
                                                         (cfg_pme_int) */
	uint64_t se                           : 1;  /**< System Error, RC DEode Only.
                                                         (cfg_sys_err_rc) */
	uint64_t aeri                         : 1;  /**< Advanced Error Reporting Interrupt, RC Mode Only.
                                                         (cfg_aer_rc_err_int). */
#else
	uint64_t aeri                         : 1;
	uint64_t se                           : 1;
	uint64_t pmei                         : 1;
	uint64_t reserved_3_3                 : 1;
	uint64_t up_b1                        : 1;
	uint64_t up_b2                        : 1;
	uint64_t up_bx                        : 1;
	uint64_t un_b1                        : 1;
	uint64_t un_b2                        : 1;
	uint64_t un_bx                        : 1;
	uint64_t reserved_10_10               : 1;
	uint64_t rdlk                         : 1;
	uint64_t crs_er                       : 1;
	uint64_t crs_dr                       : 1;
	uint64_t reserved_14_59               : 46;
	uint64_t inta                         : 1;
	uint64_t intb                         : 1;
	uint64_t intc                         : 1;
	uint64_t intd                         : 1;
#endif
	} cn78xx;
	struct cvmx_pemx_int_sum_cn61xx       cnf71xx;
};
typedef union cvmx_pemx_int_sum cvmx_pemx_int_sum_t;

/**
 * cvmx_pem#_on
 *
 * PEM is configured and ready
 *
 */
union cvmx_pemx_on {
	uint64_t u64;
	struct cvmx_pemx_on_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t pemoor                       : 1;  /**< Indication to software that the PEM has been taken out of
                                                         reset (ie bist is done) and it is safe to configure core CSRs. */
	uint64_t pemon                        : 1;  /**< Indication to the QLM that the PEM is out of reset, configured
                                                         and ready to send/receive traffic.  Setting this bit will take
                                                         the configured PIPE out of reset. */
#else
	uint64_t pemon                        : 1;
	uint64_t pemoor                       : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_pemx_on_s                 cn70xx;
	struct cvmx_pemx_on_s                 cn78xx;
};
typedef union cvmx_pemx_on cvmx_pemx_on_t;

/**
 * cvmx_pem#_p2n_bar0_start
 *
 * The starting address for addresses to forwarded to the SLI in RC Mode.
 *
 */
union cvmx_pemx_p2n_bar0_start {
	uint64_t u64;
	struct cvmx_pemx_p2n_bar0_start_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t addr                         : 50; /**< The starting address of the 16KB address space that
                                                         is the BAR0 address space. */
	uint64_t reserved_0_13                : 14;
#else
	uint64_t reserved_0_13                : 14;
	uint64_t addr                         : 50;
#endif
	} s;
	struct cvmx_pemx_p2n_bar0_start_s     cn61xx;
	struct cvmx_pemx_p2n_bar0_start_s     cn63xx;
	struct cvmx_pemx_p2n_bar0_start_s     cn63xxp1;
	struct cvmx_pemx_p2n_bar0_start_s     cn66xx;
	struct cvmx_pemx_p2n_bar0_start_s     cn68xx;
	struct cvmx_pemx_p2n_bar0_start_s     cn68xxp1;
	struct cvmx_pemx_p2n_bar0_start_s     cn70xx;
	struct cvmx_pemx_p2n_bar0_start_s     cn78xx;
	struct cvmx_pemx_p2n_bar0_start_s     cnf71xx;
};
typedef union cvmx_pemx_p2n_bar0_start cvmx_pemx_p2n_bar0_start_t;

/**
 * cvmx_pem#_p2n_bar1_start
 *
 * The starting address for addresses to forwarded to the SLI in RC Mode.
 *
 */
union cvmx_pemx_p2n_bar1_start {
	uint64_t u64;
	struct cvmx_pemx_p2n_bar1_start_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t addr                         : 38; /**< The starting address of the 64MB address space
                                                         that is the BAR1 address space. */
	uint64_t reserved_0_25                : 26;
#else
	uint64_t reserved_0_25                : 26;
	uint64_t addr                         : 38;
#endif
	} s;
	struct cvmx_pemx_p2n_bar1_start_s     cn61xx;
	struct cvmx_pemx_p2n_bar1_start_s     cn63xx;
	struct cvmx_pemx_p2n_bar1_start_s     cn63xxp1;
	struct cvmx_pemx_p2n_bar1_start_s     cn66xx;
	struct cvmx_pemx_p2n_bar1_start_s     cn68xx;
	struct cvmx_pemx_p2n_bar1_start_s     cn68xxp1;
	struct cvmx_pemx_p2n_bar1_start_s     cn70xx;
	struct cvmx_pemx_p2n_bar1_start_s     cn78xx;
	struct cvmx_pemx_p2n_bar1_start_s     cnf71xx;
};
typedef union cvmx_pemx_p2n_bar1_start cvmx_pemx_p2n_bar1_start_t;

/**
 * cvmx_pem#_p2n_bar2_start
 *
 * The starting address for addresses to forwarded to the SLI in RC Mode.
 *
 */
union cvmx_pemx_p2n_bar2_start {
	uint64_t u64;
	struct cvmx_pemx_p2n_bar2_start_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_pemx_p2n_bar2_start_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t addr                         : 23; /**< The starting address of the 2^41 address space
                                                         that is the BAR2 address space. */
	uint64_t reserved_0_40                : 41;
#else
	uint64_t reserved_0_40                : 41;
	uint64_t addr                         : 23;
#endif
	} cn61xx;
	struct cvmx_pemx_p2n_bar2_start_cn61xx cn63xx;
	struct cvmx_pemx_p2n_bar2_start_cn61xx cn63xxp1;
	struct cvmx_pemx_p2n_bar2_start_cn61xx cn66xx;
	struct cvmx_pemx_p2n_bar2_start_cn61xx cn68xx;
	struct cvmx_pemx_p2n_bar2_start_cn61xx cn68xxp1;
	struct cvmx_pemx_p2n_bar2_start_cn61xx cn70xx;
	struct cvmx_pemx_p2n_bar2_start_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t addr                         : 19; /**< The starting address of the 2^45 address space
                                                         that is the BAR2 address space. */
	uint64_t reserved_0_44                : 45;
#else
	uint64_t reserved_0_44                : 45;
	uint64_t addr                         : 19;
#endif
	} cn78xx;
	struct cvmx_pemx_p2n_bar2_start_cn61xx cnf71xx;
};
typedef union cvmx_pemx_p2n_bar2_start cvmx_pemx_p2n_bar2_start_t;

/**
 * cvmx_pem#_p2p_bar#_end
 *
 * "PEM_P2P_BAR#_END = PEM Peer-To-Peer BAR0 End
 * The ending address for addresses to forwarded to the PCIe peer port."
 */
union cvmx_pemx_p2p_barx_end {
	uint64_t u64;
	struct cvmx_pemx_p2p_barx_end_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t addr                         : 52; /**< The ending address of the address window created
                                                         this field and the PEM_P2P_BAR0_START[63:12]
                                                         field. The full 64-bits of address are created by:
                                                         [ADDR[63:12], 12'b0]. */
	uint64_t reserved_0_11                : 12;
#else
	uint64_t reserved_0_11                : 12;
	uint64_t addr                         : 52;
#endif
	} s;
	struct cvmx_pemx_p2p_barx_end_s       cn63xx;
	struct cvmx_pemx_p2p_barx_end_s       cn63xxp1;
	struct cvmx_pemx_p2p_barx_end_s       cn66xx;
	struct cvmx_pemx_p2p_barx_end_s       cn68xx;
	struct cvmx_pemx_p2p_barx_end_s       cn68xxp1;
	struct cvmx_pemx_p2p_barx_end_s       cn78xx;
};
typedef union cvmx_pemx_p2p_barx_end cvmx_pemx_p2p_barx_end_t;

/**
 * cvmx_pem#_p2p_bar#_start
 *
 * "PEM_P2P_BAR#_START = PEM Peer-To-Peer BAR0 Start
 * The starting address and enable for addresses to forwarded to the PCIe peer port."
 */
union cvmx_pemx_p2p_barx_start {
	uint64_t u64;
	struct cvmx_pemx_p2p_barx_start_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t addr                         : 52; /**< The starting address of the address window created
                                                         by this field and the PEM_P2P_BAR0_END[63:12]
                                                         field. The full 64-bits of address are created by:
                                                         [ADDR[63:12], 12'b0]. */
	uint64_t reserved_2_11                : 10;
	uint64_t dst                          : 2;  /**< The destination peer of the address window created
                                                         by this field and the PEM_P2P_BAR0_END[63:12]
                                                         field. It is illegal to configure the destination peer
                                                         to match the source. */
#else
	uint64_t dst                          : 2;
	uint64_t reserved_2_11                : 10;
	uint64_t addr                         : 52;
#endif
	} s;
	struct cvmx_pemx_p2p_barx_start_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t addr                         : 52; /**< The starting address of the address window created
                                                         by this field and the PEM_P2P_BAR0_END[63:12]
                                                         field. The full 64-bits of address are created by:
                                                         [ADDR[63:12], 12'b0]. */
	uint64_t reserved_0_11                : 12;
#else
	uint64_t reserved_0_11                : 12;
	uint64_t addr                         : 52;
#endif
	} cn63xx;
	struct cvmx_pemx_p2p_barx_start_cn63xx cn63xxp1;
	struct cvmx_pemx_p2p_barx_start_cn63xx cn66xx;
	struct cvmx_pemx_p2p_barx_start_cn63xx cn68xx;
	struct cvmx_pemx_p2p_barx_start_cn63xx cn68xxp1;
	struct cvmx_pemx_p2p_barx_start_s     cn78xx;
};
typedef union cvmx_pemx_p2p_barx_start cvmx_pemx_p2p_barx_start_t;

/**
 * cvmx_pem#_qlm
 *
 * Configuration of the PEM3 QLM
 *
 */
union cvmx_pemx_qlm {
	uint64_t u64;
	struct cvmx_pemx_qlm_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t pem3qlm                      : 1;  /**< " When set, PEM3 is configured to send/receive traffic to QLM4.
                                                         When clear, PEM3 is configured to send/receive traffic to QLM3.
                                                         Note that this bit can only be set for PEM3, for all other
                                                         PEMs it has no function.
                                                         Note that this bit must only be set when the associated PHYs
                                                         are in reset and PEM3 is receiving no clocks." */
#else
	uint64_t pem3qlm                      : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_pemx_qlm_s                cn78xx;
};
typedef union cvmx_pemx_qlm cvmx_pemx_qlm_t;

/**
 * cvmx_pem#_spi_ctl
 *
 * PEM#_SPI_CTL register.
 *
 */
union cvmx_pemx_spi_ctl {
	uint64_t u64;
	struct cvmx_pemx_spi_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t start_busy                   : 1;  /**< Start/Busy status. Starts SPI xctn when written, reads 1 when EEPROM busy, 0 when complete. */
	uint64_t tvalid                       : 1;  /**< Reads 1 if at least one valid entry was read from EEPROM and written to a CSR. Write to
                                                         clear status. */
	uint64_t cmd                          : 3;  /**< SPI commands; WREN(110), WRDI(100), READ(011), WRITE(010), RDSR(101), WRSR(001) */
	uint64_t adr                          : 9;  /**< EEPROM read/write address. */
#else
	uint64_t adr                          : 9;
	uint64_t cmd                          : 3;
	uint64_t tvalid                       : 1;
	uint64_t start_busy                   : 1;
	uint64_t reserved_14_63               : 50;
#endif
	} s;
	struct cvmx_pemx_spi_ctl_s            cn70xx;
	struct cvmx_pemx_spi_ctl_s            cn78xx;
};
typedef union cvmx_pemx_spi_ctl cvmx_pemx_spi_ctl_t;

/**
 * cvmx_pem#_spi_data
 *
 * "PEM#_SPI read/write data register. Contains most recently read or written data and is
 * unpredictable upon power-up"
 */
union cvmx_pemx_spi_data {
	uint64_t u64;
	struct cvmx_pemx_spi_data_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t preamble                     : 16; /**< EEPROM PREAMBLE read or write data. */
	uint64_t reserved_45_47               : 3;
	uint64_t cs2                          : 1;  /**< EEPROM CS2 read or write data bit. */
	uint64_t adr                          : 12; /**< EEPROM CFG ADR read or write data. */
	uint64_t data                         : 32; /**< EEPROM DATA read or write data. */
#else
	uint64_t data                         : 32;
	uint64_t adr                          : 12;
	uint64_t cs2                          : 1;
	uint64_t reserved_45_47               : 3;
	uint64_t preamble                     : 16;
#endif
	} s;
	struct cvmx_pemx_spi_data_s           cn70xx;
	struct cvmx_pemx_spi_data_s           cn78xx;
};
typedef union cvmx_pemx_spi_data cvmx_pemx_spi_data_t;

/**
 * cvmx_pem#_strap
 *
 * "Below are in pesc_csr
 * The input strapping pins"
 */
union cvmx_pemx_strap {
	uint64_t u64;
	struct cvmx_pemx_strap_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t pilaneswap                   : 1;  /**< The value of the pi_select_laneswap pin */
	uint64_t reserved_0_2                 : 3;
#else
	uint64_t reserved_0_2                 : 3;
	uint64_t pilaneswap                   : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_pemx_strap_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t pilaneswap                   : 1;  /**< The value of the pi_select_laneswap pin */
	uint64_t pimode                       : 3;  /**< The value of the pi_select_mode[2:0] pins */
#else
	uint64_t pimode                       : 3;
	uint64_t pilaneswap                   : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} cn70xx;
	struct cvmx_pemx_strap_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t pilaneswap                   : 1;  /**< The value of the pi_select_laneswap pin */
	uint64_t pilanes8                     : 1;  /**< The value of the pi_select_8lanes pin */
	uint64_t pimode                       : 2;  /**< The value of the pi_select_mode[1:0] pins */
#else
	uint64_t pimode                       : 2;
	uint64_t pilanes8                     : 1;
	uint64_t pilaneswap                   : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} cn78xx;
};
typedef union cvmx_pemx_strap cvmx_pemx_strap_t;

/**
 * cvmx_pem#_tlp_credits
 *
 * Specifies the number of credits the PEM for use in moving TLPs. When this register is written
 * the credit values are
 * reset to the register value. A write to this register should take place BEFORE traffic flow
 * starts.
 */
union cvmx_pemx_tlp_credits {
	uint64_t u64;
	struct cvmx_pemx_tlp_credits_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_56_63               : 8;
	uint64_t peai_ppf                     : 8;  /**< TLP credits for Completion TLPs in the Peer.
                                                         The value in this register should not be changed.
                                                         Values other than 0x80 can lead to unpredictable
                                                         behavior */
	uint64_t pem_cpl                      : 8;  /**< TLP 16B credits for Completion TLPs in the Peer.
                                                         Legal values are 0x12 to 0x40. */
	uint64_t pem_np                       : 8;  /**< TLP 16B credits for Non-Posted TLPs in the Peer.
                                                         Legal values are 0x4 to 0x8. */
	uint64_t pem_p                        : 8;  /**< TLP 16B credits for Posted TLPs in the Peer.
                                                         Legal values are 0x12 to 0x40. */
	uint64_t sli_cpl                      : 8;  /**< TLP credits for Completion TLPs in the SLI.
                                                         Legal values are 0x24 to 0x80. */
	uint64_t sli_np                       : 8;  /**< TLP credits for Non-Posted TLPs in the SLI.
                                                         Legal values are 0x4 to 0x10. */
	uint64_t sli_p                        : 8;  /**< TLP credits for Posted TLPs in the SLI.
                                                         Legal values are 0x24 to 0x80. */
#else
	uint64_t sli_p                        : 8;
	uint64_t sli_np                       : 8;
	uint64_t sli_cpl                      : 8;
	uint64_t pem_p                        : 8;
	uint64_t pem_np                       : 8;
	uint64_t pem_cpl                      : 8;
	uint64_t peai_ppf                     : 8;
	uint64_t reserved_56_63               : 8;
#endif
	} s;
	struct cvmx_pemx_tlp_credits_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_56_63               : 8;
	uint64_t peai_ppf                     : 8;  /**< TLP credits for Completion TLPs in the Peer.
                                                         The value in this register should not be changed.
                                                         Values other than 0x80 can lead to unpredictable
                                                         behavior */
	uint64_t reserved_24_47               : 24;
	uint64_t sli_cpl                      : 8;  /**< TLP credits for Completion TLPs in the SLI.
                                                         Legal values are 0x24 to 0x80. */
	uint64_t sli_np                       : 8;  /**< TLP credits for Non-Posted TLPs in the SLI.
                                                         Legal values are 0x4 to 0x10. */
	uint64_t sli_p                        : 8;  /**< TLP credits for Posted TLPs in the SLI.
                                                         Legal values are 0x24 to 0x80. */
#else
	uint64_t sli_p                        : 8;
	uint64_t sli_np                       : 8;
	uint64_t sli_cpl                      : 8;
	uint64_t reserved_24_47               : 24;
	uint64_t peai_ppf                     : 8;
	uint64_t reserved_56_63               : 8;
#endif
	} cn61xx;
	struct cvmx_pemx_tlp_credits_s        cn63xx;
	struct cvmx_pemx_tlp_credits_s        cn63xxp1;
	struct cvmx_pemx_tlp_credits_s        cn66xx;
	struct cvmx_pemx_tlp_credits_s        cn68xx;
	struct cvmx_pemx_tlp_credits_s        cn68xxp1;
	struct cvmx_pemx_tlp_credits_cn61xx   cn70xx;
	struct cvmx_pemx_tlp_credits_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t pem_cpl                      : 8;  /**< TLP 16B credits for Completion TLPs in the Peer.
                                                         Legal values are 0x12 to 0x40. */
	uint64_t pem_np                       : 8;  /**< TLP 16B credits for Non-Posted TLPs in the Peer.
                                                         Legal values are 0x4 to 0x8. */
	uint64_t pem_p                        : 8;  /**< TLP 16B credits for Posted TLPs in the Peer.
                                                         Legal values are 0x12 to 0x40. */
	uint64_t sli_cpl                      : 8;  /**< TLP 8B credits for Completion TLPs in the SLI.
                                                         Legal values are 0x24 to 0x80. When multiple PEMS
                                                         are wire-OR'd together, the sum of all PEMs' SLI_CPL
                                                         fields must not exceed 0x80. The reset value for this
                                                         register is based on the pi_select_8lanes and the
                                                         pi_select_mode straps. For EP mode PEMs, the 8lanes strap
                                                         determines the credits reset value; 1'b0 = 4 lanes = 64
                                                         credits, 1'b1 = 8 lanes = 128 credits. For RC mode PEMs,
                                                         the credits are reset assuming a 4 lane configuration.
                                                         SW can bump this value up in the case of 8 lane RC mode
                                                         PEMS. SW may need to reprogram this register for performance
                                                         reasons (configured as the only PEM, but as 4-lane - in
                                                         this case, this PEM can take all of the credits). */
	uint64_t sli_np                       : 8;  /**< TLP 8B credits for Non-Posted TLPs in the SLI.
                                                         Legal values are 0x8 to 0x10. When multiple PEMS
                                                         are wire-OR'd together, the sum of all PEMs' SLI_CPL
                                                         fields must not exceed 0x80. The reset value for this
                                                         register is based on the pi_select_8lanes and the
                                                         pi_select_mode straps. For EP mode PEMs, the 8lanes strap
                                                         determines the credits reset value; 1'b0 = 4 lanes = 8
                                                         credits, 1'b1 = 8 lanes = 16 credits. For RC mode PEMs,
                                                         the credits are reset assuming a 4 lane configuration.
                                                         SW can bump this value up in the case of 8 lane RC mode
                                                         PEMS. SW may need to reprogram this register for performance
                                                         reasons (configured as the only PEM, but as 4-lane - in
                                                         this case, this PEM can take all of the credits). */
	uint64_t sli_p                        : 8;  /**< TLP 8B credits for Posted TLPs in the SLI.
                                                         Legal values are 0x24 to 0x80. When multiple PEMS
                                                         are wire-OR'd together, the sum of all PEMs' SLI_CPL
                                                         fields must not exceed 0x80. The reset value for this
                                                         register is based on the pi_select_8lanes and the
                                                         pi_select_mode straps. For EP mode PEMs, the 8lanes strap
                                                         determines the credits reset value; 1'b0 = 4 lanes = 64
                                                         credits, 1'b1 = 8 lanes = 128 credits. For RC mode PEMs,
                                                         the credits are reset assuming a 4 lane configuration.
                                                         SW can bump this value up in the case of 8 lane RC mode
                                                         PEMS. SW may need to reprogram this register for performance
                                                         reasons (configured as the only PEM, but as 4-lane - in
                                                         this case, this PEM can take all of the credits). */
#else
	uint64_t sli_p                        : 8;
	uint64_t sli_np                       : 8;
	uint64_t sli_cpl                      : 8;
	uint64_t pem_p                        : 8;
	uint64_t pem_np                       : 8;
	uint64_t pem_cpl                      : 8;
	uint64_t reserved_48_63               : 16;
#endif
	} cn78xx;
	struct cvmx_pemx_tlp_credits_cn61xx   cnf71xx;
};
typedef union cvmx_pemx_tlp_credits cvmx_pemx_tlp_credits_t;

#endif
