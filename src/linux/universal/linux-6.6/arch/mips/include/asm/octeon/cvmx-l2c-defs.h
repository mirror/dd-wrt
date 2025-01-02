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
 * cvmx-l2c-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon l2c.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_L2C_DEFS_H__
#define __CVMX_L2C_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_BIG_CTL CVMX_L2C_BIG_CTL_FUNC()
static inline uint64_t CVMX_L2C_BIG_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF71XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_L2C_BIG_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180080800030ull);
}
#else
#define CVMX_L2C_BIG_CTL (CVMX_ADD_IO_SEG(0x0001180080800030ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_BST CVMX_L2C_BST_FUNC()
static inline uint64_t CVMX_L2C_BST_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_L2C_BST not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800808007F8ull);
}
#else
#define CVMX_L2C_BST (CVMX_ADD_IO_SEG(0x00011800808007F8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_BST0 CVMX_L2C_BST0_FUNC()
static inline uint64_t CVMX_L2C_BST0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN5XXX)))
		cvmx_warn("CVMX_L2C_BST0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800800007F8ull);
}
#else
#define CVMX_L2C_BST0 (CVMX_ADD_IO_SEG(0x00011800800007F8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_BST1 CVMX_L2C_BST1_FUNC()
static inline uint64_t CVMX_L2C_BST1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN5XXX)))
		cvmx_warn("CVMX_L2C_BST1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800800007F0ull);
}
#else
#define CVMX_L2C_BST1 (CVMX_ADD_IO_SEG(0x00011800800007F0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_BST2 CVMX_L2C_BST2_FUNC()
static inline uint64_t CVMX_L2C_BST2_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN5XXX)))
		cvmx_warn("CVMX_L2C_BST2 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800800007E8ull);
}
#else
#define CVMX_L2C_BST2 (CVMX_ADD_IO_SEG(0x00011800800007E8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_BST_MEMX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_L2C_BST_MEMX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080C007F8ull) + ((offset) & 3) * 0x40000ull;
}
#else
#define CVMX_L2C_BST_MEMX(offset) (CVMX_ADD_IO_SEG(0x0001180080C007F8ull) + ((offset) & 3) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_BST_TDTX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_L2C_BST_TDTX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080A007F0ull) + ((offset) & 3) * 0x40000ull;
}
#else
#define CVMX_L2C_BST_TDTX(offset) (CVMX_ADD_IO_SEG(0x0001180080A007F0ull) + ((offset) & 3) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_BST_TTGX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_L2C_BST_TTGX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080A007F8ull) + ((offset) & 3) * 0x40000ull;
}
#else
#define CVMX_L2C_BST_TTGX(offset) (CVMX_ADD_IO_SEG(0x0001180080A007F8ull) + ((offset) & 3) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_CBCX_BIST_STATUS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_L2C_CBCX_BIST_STATUS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080E007F8ull) + ((offset) & 3) * 0x40000ull;
}
#else
#define CVMX_L2C_CBCX_BIST_STATUS(offset) (CVMX_ADD_IO_SEG(0x0001180080E007F8ull) + ((offset) & 3) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_CBCX_DLL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_L2C_CBCX_DLL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080E00018ull) + ((offset) & 3) * 0x40000ull;
}
#else
#define CVMX_L2C_CBCX_DLL(offset) (CVMX_ADD_IO_SEG(0x0001180080E00018ull) + ((offset) & 3) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_CBCX_HOLEERR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_L2C_CBCX_HOLEERR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080E007D0ull) + ((offset) & 3) * 0x40000ull;
}
#else
#define CVMX_L2C_CBCX_HOLEERR(offset) (CVMX_ADD_IO_SEG(0x0001180080E007D0ull) + ((offset) & 3) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_CBCX_INT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_L2C_CBCX_INT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080E00028ull) + ((offset) & 3) * 0x40000ull;
}
#else
#define CVMX_L2C_CBCX_INT(offset) (CVMX_ADD_IO_SEG(0x0001180080E00028ull) + ((offset) & 3) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_CBCX_IOCERR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_L2C_CBCX_IOCERR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080E007E8ull) + ((offset) & 3) * 0x40000ull;
}
#else
#define CVMX_L2C_CBCX_IOCERR(offset) (CVMX_ADD_IO_SEG(0x0001180080E007E8ull) + ((offset) & 3) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_CBCX_IODISOCIERR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_L2C_CBCX_IODISOCIERR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080E007D8ull) + ((offset) & 3) * 0x40000ull;
}
#else
#define CVMX_L2C_CBCX_IODISOCIERR(offset) (CVMX_ADD_IO_SEG(0x0001180080E007D8ull) + ((offset) & 3) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_CBCX_MIBERR(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if (((offset >= 2) && (offset <= 3)))
					return CVMX_ADD_IO_SEG(0x0001180080E807E0ull) + ((offset) & 3) * 0x40000ull - 262144*2;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if (((offset >= 2) && (offset <= 3)))
					return CVMX_ADD_IO_SEG(0x0001180080E807E0ull) + ((offset) & 3) * 0x40000ull - 262144*2;

			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset == 1))
				return CVMX_ADD_IO_SEG(0x0001180080E807E0ull) + ((offset) & 1) * 0x40000ull - 262144*1;
			break;
	}
	cvmx_warn("CVMX_L2C_CBCX_MIBERR (offset = %lu) not supported on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080E807E0ull) + ((offset) & 1) * 0x40000ull - 262144*1;
}
#else
static inline uint64_t CVMX_L2C_CBCX_MIBERR(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return CVMX_ADD_IO_SEG(0x0001180080E807E0ull) + (offset) * 0x40000ull - 262144*2;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return CVMX_ADD_IO_SEG(0x0001180080E807E0ull) + (offset) * 0x40000ull - 262144*2;

		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001180080E807E0ull) + (offset) * 0x40000ull - 262144*1;
	}
	return CVMX_ADD_IO_SEG(0x0001180080E807E0ull) + (offset) * 0x40000ull - 262144*1;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_CBCX_RSDERR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_L2C_CBCX_RSDERR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080E007F0ull) + ((offset) & 3) * 0x40000ull;
}
#else
#define CVMX_L2C_CBCX_RSDERR(offset) (CVMX_ADD_IO_SEG(0x0001180080E007F0ull) + ((offset) & 3) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_CFG CVMX_L2C_CFG_FUNC()
static inline uint64_t CVMX_L2C_CFG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN5XXX)))
		cvmx_warn("CVMX_L2C_CFG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180080000000ull);
}
#else
#define CVMX_L2C_CFG (CVMX_ADD_IO_SEG(0x0001180080000000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_COP0_ADR CVMX_L2C_COP0_ADR_FUNC()
static inline uint64_t CVMX_L2C_COP0_ADR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_L2C_COP0_ADR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180080800038ull);
}
#else
#define CVMX_L2C_COP0_ADR (CVMX_ADD_IO_SEG(0x0001180080800038ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_COP0_DAT CVMX_L2C_COP0_DAT_FUNC()
static inline uint64_t CVMX_L2C_COP0_DAT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_L2C_COP0_DAT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180080800040ull);
}
#else
#define CVMX_L2C_COP0_DAT (CVMX_ADD_IO_SEG(0x0001180080800040ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_COP0_MAPX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 1023) || ((offset >= 16128) && (offset <= 16383)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1535) || ((offset >= 16128) && (offset <= 16383)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 2559) || ((offset >= 16128) && (offset <= 16383)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 8191) || ((offset >= 16128) && (offset <= 16383)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 1023) || ((offset >= 16128) && (offset <= 16383))))))
		cvmx_warn("CVMX_L2C_COP0_MAPX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080940000ull) + ((offset) & 16383) * 8;
}
#else
#define CVMX_L2C_COP0_MAPX(offset) (CVMX_ADD_IO_SEG(0x0001180080940000ull) + ((offset) & 16383) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_CTL CVMX_L2C_CTL_FUNC()
static inline uint64_t CVMX_L2C_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF71XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_L2C_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180080800000ull);
}
#else
#define CVMX_L2C_CTL (CVMX_ADD_IO_SEG(0x0001180080800000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_DBG CVMX_L2C_DBG_FUNC()
static inline uint64_t CVMX_L2C_DBG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN5XXX)))
		cvmx_warn("CVMX_L2C_DBG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180080000030ull);
}
#else
#define CVMX_L2C_DBG (CVMX_ADD_IO_SEG(0x0001180080000030ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_DUT CVMX_L2C_DUT_FUNC()
static inline uint64_t CVMX_L2C_DUT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN5XXX)))
		cvmx_warn("CVMX_L2C_DUT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180080000050ull);
}
#else
#define CVMX_L2C_DUT (CVMX_ADD_IO_SEG(0x0001180080000050ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_DUT_MAPX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1535))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 2559))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 8191))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_L2C_DUT_MAPX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080E00000ull) + ((offset) & 8191) * 8;
}
#else
#define CVMX_L2C_DUT_MAPX(offset) (CVMX_ADD_IO_SEG(0x0001180080E00000ull) + ((offset) & 8191) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_ECC_CTL CVMX_L2C_ECC_CTL_FUNC()
static inline uint64_t CVMX_L2C_ECC_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_L2C_ECC_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180080800010ull);
}
#else
#define CVMX_L2C_ECC_CTL (CVMX_ADD_IO_SEG(0x0001180080800010ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_ERR_TDTX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_L2C_ERR_TDTX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080A007E0ull) + ((offset) & 3) * 0x40000ull;
}
#else
#define CVMX_L2C_ERR_TDTX(offset) (CVMX_ADD_IO_SEG(0x0001180080A007E0ull) + ((offset) & 3) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_ERR_TTGX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_L2C_ERR_TTGX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080A007E8ull) + ((offset) & 3) * 0x40000ull;
}
#else
#define CVMX_L2C_ERR_TTGX(offset) (CVMX_ADD_IO_SEG(0x0001180080A007E8ull) + ((offset) & 3) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_ERR_VBFX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_L2C_ERR_VBFX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080C007F0ull) + ((offset) & 3) * 0x40000ull;
}
#else
#define CVMX_L2C_ERR_VBFX(offset) (CVMX_ADD_IO_SEG(0x0001180080C007F0ull) + ((offset) & 3) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_ERR_XMC CVMX_L2C_ERR_XMC_FUNC()
static inline uint64_t CVMX_L2C_ERR_XMC_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_L2C_ERR_XMC not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800808007D8ull);
}
#else
#define CVMX_L2C_ERR_XMC (CVMX_ADD_IO_SEG(0x00011800808007D8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_GRPWRR0 CVMX_L2C_GRPWRR0_FUNC()
static inline uint64_t CVMX_L2C_GRPWRR0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN56XX)))
		cvmx_warn("CVMX_L2C_GRPWRR0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800800000C8ull);
}
#else
#define CVMX_L2C_GRPWRR0 (CVMX_ADD_IO_SEG(0x00011800800000C8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_GRPWRR1 CVMX_L2C_GRPWRR1_FUNC()
static inline uint64_t CVMX_L2C_GRPWRR1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN56XX)))
		cvmx_warn("CVMX_L2C_GRPWRR1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800800000D0ull);
}
#else
#define CVMX_L2C_GRPWRR1 (CVMX_ADD_IO_SEG(0x00011800800000D0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_INT_EN CVMX_L2C_INT_EN_FUNC()
static inline uint64_t CVMX_L2C_INT_EN_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN56XX)))
		cvmx_warn("CVMX_L2C_INT_EN not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180080000100ull);
}
#else
#define CVMX_L2C_INT_EN (CVMX_ADD_IO_SEG(0x0001180080000100ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_INT_ENA CVMX_L2C_INT_ENA_FUNC()
static inline uint64_t CVMX_L2C_INT_ENA_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_L2C_INT_ENA not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180080800020ull);
}
#else
#define CVMX_L2C_INT_ENA (CVMX_ADD_IO_SEG(0x0001180080800020ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_INT_REG CVMX_L2C_INT_REG_FUNC()
static inline uint64_t CVMX_L2C_INT_REG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_L2C_INT_REG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180080800018ull);
}
#else
#define CVMX_L2C_INT_REG (CVMX_ADD_IO_SEG(0x0001180080800018ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_INT_STAT CVMX_L2C_INT_STAT_FUNC()
static inline uint64_t CVMX_L2C_INT_STAT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN56XX)))
		cvmx_warn("CVMX_L2C_INT_STAT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800800000F8ull);
}
#else
#define CVMX_L2C_INT_STAT (CVMX_ADD_IO_SEG(0x00011800800000F8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_INVX_PFC(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_L2C_INVX_PFC(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080800820ull) + ((offset) & 7) * 64;
}
#else
#define CVMX_L2C_INVX_PFC(offset) (CVMX_ADD_IO_SEG(0x0001180080800820ull) + ((offset) & 7) * 64)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_IOCX_PFC(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset == 0))
					return CVMX_ADD_IO_SEG(0x0001180080800828ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset == 0))
					return CVMX_ADD_IO_SEG(0x0001180080800828ull);
			if ((offset == 0))
				return CVMX_ADD_IO_SEG(0x0001180080800828ull);
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return CVMX_ADD_IO_SEG(0x0001180080800420ull);
			break;
	}
	cvmx_warn("CVMX_L2C_IOCX_PFC (offset = %lu) not supported on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080800828ull);
}
#else
static inline uint64_t CVMX_L2C_IOCX_PFC(unsigned long offset __attribute__ ((unused)))
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return CVMX_ADD_IO_SEG(0x0001180080800828ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return CVMX_ADD_IO_SEG(0x0001180080800828ull);
			return CVMX_ADD_IO_SEG(0x0001180080800828ull);
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001180080800420ull);
	}
	return CVMX_ADD_IO_SEG(0x0001180080800828ull);
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_IORX_PFC(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset == 0))
					return CVMX_ADD_IO_SEG(0x0001180080800830ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset == 0))
					return CVMX_ADD_IO_SEG(0x0001180080800830ull);
			if ((offset == 0))
				return CVMX_ADD_IO_SEG(0x0001180080800830ull);
			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return CVMX_ADD_IO_SEG(0x0001180080800428ull);
			break;
	}
	cvmx_warn("CVMX_L2C_IORX_PFC (offset = %lu) not supported on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080800830ull);
}
#else
static inline uint64_t CVMX_L2C_IORX_PFC(unsigned long offset __attribute__ ((unused)))
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return CVMX_ADD_IO_SEG(0x0001180080800830ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return CVMX_ADD_IO_SEG(0x0001180080800830ull);
			return CVMX_ADD_IO_SEG(0x0001180080800830ull);
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001180080800428ull);
	}
	return CVMX_ADD_IO_SEG(0x0001180080800830ull);
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_LCKBASE CVMX_L2C_LCKBASE_FUNC()
static inline uint64_t CVMX_L2C_LCKBASE_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN5XXX)))
		cvmx_warn("CVMX_L2C_LCKBASE not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180080000058ull);
}
#else
#define CVMX_L2C_LCKBASE (CVMX_ADD_IO_SEG(0x0001180080000058ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_LCKOFF CVMX_L2C_LCKOFF_FUNC()
static inline uint64_t CVMX_L2C_LCKOFF_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN5XXX)))
		cvmx_warn("CVMX_L2C_LCKOFF not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180080000060ull);
}
#else
#define CVMX_L2C_LCKOFF (CVMX_ADD_IO_SEG(0x0001180080000060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_LFB0 CVMX_L2C_LFB0_FUNC()
static inline uint64_t CVMX_L2C_LFB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN5XXX)))
		cvmx_warn("CVMX_L2C_LFB0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180080000038ull);
}
#else
#define CVMX_L2C_LFB0 (CVMX_ADD_IO_SEG(0x0001180080000038ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_LFB1 CVMX_L2C_LFB1_FUNC()
static inline uint64_t CVMX_L2C_LFB1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN5XXX)))
		cvmx_warn("CVMX_L2C_LFB1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180080000040ull);
}
#else
#define CVMX_L2C_LFB1 (CVMX_ADD_IO_SEG(0x0001180080000040ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_LFB2 CVMX_L2C_LFB2_FUNC()
static inline uint64_t CVMX_L2C_LFB2_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN5XXX)))
		cvmx_warn("CVMX_L2C_LFB2 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180080000048ull);
}
#else
#define CVMX_L2C_LFB2 (CVMX_ADD_IO_SEG(0x0001180080000048ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_LFB3 CVMX_L2C_LFB3_FUNC()
static inline uint64_t CVMX_L2C_LFB3_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN5XXX)))
		cvmx_warn("CVMX_L2C_LFB3 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800800000B8ull);
}
#else
#define CVMX_L2C_LFB3 (CVMX_ADD_IO_SEG(0x00011800800000B8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_MCIX_BIST_STATUS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_L2C_MCIX_BIST_STATUS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080C007F8ull) + ((offset) & 3) * 0x40000ull;
}
#else
#define CVMX_L2C_MCIX_BIST_STATUS(offset) (CVMX_ADD_IO_SEG(0x0001180080C007F8ull) + ((offset) & 3) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_MCIX_ERR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_L2C_MCIX_ERR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080C007F0ull) + ((offset) & 3) * 0x40000ull;
}
#else
#define CVMX_L2C_MCIX_ERR(offset) (CVMX_ADD_IO_SEG(0x0001180080C007F0ull) + ((offset) & 3) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_MCIX_INT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_L2C_MCIX_INT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080C00028ull) + ((offset) & 3) * 0x40000ull;
}
#else
#define CVMX_L2C_MCIX_INT(offset) (CVMX_ADD_IO_SEG(0x0001180080C00028ull) + ((offset) & 3) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_OCI_CTL CVMX_L2C_OCI_CTL_FUNC()
static inline uint64_t CVMX_L2C_OCI_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_L2C_OCI_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180080800020ull);
}
#else
#define CVMX_L2C_OCI_CTL (CVMX_ADD_IO_SEG(0x0001180080800020ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_OOB CVMX_L2C_OOB_FUNC()
static inline uint64_t CVMX_L2C_OOB_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN56XX)))
		cvmx_warn("CVMX_L2C_OOB not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800800000D8ull);
}
#else
#define CVMX_L2C_OOB (CVMX_ADD_IO_SEG(0x00011800800000D8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_OOB1 CVMX_L2C_OOB1_FUNC()
static inline uint64_t CVMX_L2C_OOB1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN56XX)))
		cvmx_warn("CVMX_L2C_OOB1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800800000E0ull);
}
#else
#define CVMX_L2C_OOB1 (CVMX_ADD_IO_SEG(0x00011800800000E0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_OOB2 CVMX_L2C_OOB2_FUNC()
static inline uint64_t CVMX_L2C_OOB2_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN56XX)))
		cvmx_warn("CVMX_L2C_OOB2 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800800000E8ull);
}
#else
#define CVMX_L2C_OOB2 (CVMX_ADD_IO_SEG(0x00011800800000E8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_OOB3 CVMX_L2C_OOB3_FUNC()
static inline uint64_t CVMX_L2C_OOB3_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN56XX)))
		cvmx_warn("CVMX_L2C_OOB3 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800800000F0ull);
}
#else
#define CVMX_L2C_OOB3 (CVMX_ADD_IO_SEG(0x00011800800000F0ull))
#endif
#define CVMX_L2C_PFC0 CVMX_L2C_PFCX(0)
#define CVMX_L2C_PFC1 CVMX_L2C_PFCX(1)
#define CVMX_L2C_PFC2 CVMX_L2C_PFCX(2)
#define CVMX_L2C_PFC3 CVMX_L2C_PFCX(3)
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_PFCTL CVMX_L2C_PFCTL_FUNC()
static inline uint64_t CVMX_L2C_PFCTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN5XXX)))
		cvmx_warn("CVMX_L2C_PFCTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180080000090ull);
}
#else
#define CVMX_L2C_PFCTL (CVMX_ADD_IO_SEG(0x0001180080000090ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_PFCX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN30XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN31XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN38XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN50XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN58XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_L2C_PFCX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080000098ull) + ((offset) & 3) * 8;
}
#else
#define CVMX_L2C_PFCX(offset) (CVMX_ADD_IO_SEG(0x0001180080000098ull) + ((offset) & 3) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_PPGRP CVMX_L2C_PPGRP_FUNC()
static inline uint64_t CVMX_L2C_PPGRP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN52XX) || OCTEON_IS_MODEL(OCTEON_CN56XX)))
		cvmx_warn("CVMX_L2C_PPGRP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800800000C0ull);
}
#else
#define CVMX_L2C_PPGRP (CVMX_ADD_IO_SEG(0x00011800800000C0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_QOS_IOBX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_L2C_QOS_IOBX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080880200ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_L2C_QOS_IOBX(offset) (CVMX_ADD_IO_SEG(0x0001180080880200ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_QOS_PPX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 5))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 9))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 15))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 47))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 47))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 15)))))
		cvmx_warn("CVMX_L2C_QOS_PPX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080880000ull) + ((offset) & 63) * 8;
}
#else
#define CVMX_L2C_QOS_PPX(offset) (CVMX_ADD_IO_SEG(0x0001180080880000ull) + ((offset) & 63) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_QOS_WGT CVMX_L2C_QOS_WGT_FUNC()
static inline uint64_t CVMX_L2C_QOS_WGT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF71XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_L2C_QOS_WGT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180080800008ull);
}
#else
#define CVMX_L2C_QOS_WGT (CVMX_ADD_IO_SEG(0x0001180080800008ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_RSCX_PFC(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return CVMX_ADD_IO_SEG(0x0001180080800810ull) + ((offset) & 0) * 64;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 3))
				return CVMX_ADD_IO_SEG(0x0001180080800810ull) + ((offset) & 3) * 64;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 9))
					return CVMX_ADD_IO_SEG(0x0001180080800810ull) + ((offset) & 15) * 64;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 9))
					return CVMX_ADD_IO_SEG(0x0001180080800810ull) + ((offset) & 15) * 64;

			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return CVMX_ADD_IO_SEG(0x0001180080800410ull);
			break;
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 3))
				return CVMX_ADD_IO_SEG(0x0001180080800410ull) + ((offset) & 3) * 64;
			break;
	}
	cvmx_warn("CVMX_L2C_RSCX_PFC (offset = %lu) not supported on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080800810ull) + ((offset) & 3) * 64;
}
#else
static inline uint64_t CVMX_L2C_RSCX_PFC(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001180080800810ull) + (offset) * 64;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001180080800810ull) + (offset) * 64;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return CVMX_ADD_IO_SEG(0x0001180080800810ull) + (offset) * 64;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return CVMX_ADD_IO_SEG(0x0001180080800810ull) + (offset) * 64;

		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001180080800410ull);
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001180080800410ull) + (offset) * 64;
	}
	return CVMX_ADD_IO_SEG(0x0001180080800810ull) + (offset) * 64;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_RSDX_PFC(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return CVMX_ADD_IO_SEG(0x0001180080800818ull) + ((offset) & 0) * 64;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 3))
				return CVMX_ADD_IO_SEG(0x0001180080800818ull) + ((offset) & 3) * 64;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 9))
					return CVMX_ADD_IO_SEG(0x0001180080800818ull) + ((offset) & 15) * 64;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 9))
					return CVMX_ADD_IO_SEG(0x0001180080800818ull) + ((offset) & 15) * 64;

			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return CVMX_ADD_IO_SEG(0x0001180080800418ull);
			break;
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 3))
				return CVMX_ADD_IO_SEG(0x0001180080800418ull) + ((offset) & 3) * 64;
			break;
	}
	cvmx_warn("CVMX_L2C_RSDX_PFC (offset = %lu) not supported on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080800818ull) + ((offset) & 3) * 64;
}
#else
static inline uint64_t CVMX_L2C_RSDX_PFC(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001180080800818ull) + (offset) * 64;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001180080800818ull) + (offset) * 64;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return CVMX_ADD_IO_SEG(0x0001180080800818ull) + (offset) * 64;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return CVMX_ADD_IO_SEG(0x0001180080800818ull) + (offset) * 64;

		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001180080800418ull);
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001180080800418ull) + (offset) * 64;
	}
	return CVMX_ADD_IO_SEG(0x0001180080800818ull) + (offset) * 64;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_RTGX_ERR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7)))))
		cvmx_warn("CVMX_L2C_RTGX_ERR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080A00800ull) + ((offset) & 7) * 0x40000ull;
}
#else
#define CVMX_L2C_RTGX_ERR(offset) (CVMX_ADD_IO_SEG(0x0001180080A00800ull) + ((offset) & 7) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_SPAR0 CVMX_L2C_SPAR0_FUNC()
static inline uint64_t CVMX_L2C_SPAR0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN5XXX)))
		cvmx_warn("CVMX_L2C_SPAR0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180080000068ull);
}
#else
#define CVMX_L2C_SPAR0 (CVMX_ADD_IO_SEG(0x0001180080000068ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_SPAR1 CVMX_L2C_SPAR1_FUNC()
static inline uint64_t CVMX_L2C_SPAR1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN38XX) || OCTEON_IS_MODEL(OCTEON_CN56XX) || OCTEON_IS_MODEL(OCTEON_CN58XX)))
		cvmx_warn("CVMX_L2C_SPAR1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180080000070ull);
}
#else
#define CVMX_L2C_SPAR1 (CVMX_ADD_IO_SEG(0x0001180080000070ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_SPAR2 CVMX_L2C_SPAR2_FUNC()
static inline uint64_t CVMX_L2C_SPAR2_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN38XX) || OCTEON_IS_MODEL(OCTEON_CN56XX) || OCTEON_IS_MODEL(OCTEON_CN58XX)))
		cvmx_warn("CVMX_L2C_SPAR2 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180080000078ull);
}
#else
#define CVMX_L2C_SPAR2 (CVMX_ADD_IO_SEG(0x0001180080000078ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_SPAR3 CVMX_L2C_SPAR3_FUNC()
static inline uint64_t CVMX_L2C_SPAR3_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN38XX) || OCTEON_IS_MODEL(OCTEON_CN58XX)))
		cvmx_warn("CVMX_L2C_SPAR3 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180080000080ull);
}
#else
#define CVMX_L2C_SPAR3 (CVMX_ADD_IO_SEG(0x0001180080000080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_SPAR4 CVMX_L2C_SPAR4_FUNC()
static inline uint64_t CVMX_L2C_SPAR4_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN3XXX) || OCTEON_IS_MODEL(OCTEON_CN5XXX)))
		cvmx_warn("CVMX_L2C_SPAR4 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180080000088ull);
}
#else
#define CVMX_L2C_SPAR4 (CVMX_ADD_IO_SEG(0x0001180080000088ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_TADX_DLL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_L2C_TADX_DLL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080A00018ull) + ((offset) & 7) * 0x40000ull;
}
#else
#define CVMX_L2C_TADX_DLL(offset) (CVMX_ADD_IO_SEG(0x0001180080A00018ull) + ((offset) & 7) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_TADX_ECC0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_L2C_TADX_ECC0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080A00018ull) + ((offset) & 3) * 0x40000ull;
}
#else
#define CVMX_L2C_TADX_ECC0(offset) (CVMX_ADD_IO_SEG(0x0001180080A00018ull) + ((offset) & 3) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_TADX_ECC1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_L2C_TADX_ECC1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080A00020ull) + ((offset) & 3) * 0x40000ull;
}
#else
#define CVMX_L2C_TADX_ECC1(offset) (CVMX_ADD_IO_SEG(0x0001180080A00020ull) + ((offset) & 3) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_TADX_ERR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_L2C_TADX_ERR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080A007D0ull) + ((offset) & 7) * 0x40000ull;
}
#else
#define CVMX_L2C_TADX_ERR(offset) (CVMX_ADD_IO_SEG(0x0001180080A007D0ull) + ((offset) & 7) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_TADX_IEN(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_L2C_TADX_IEN(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080A00000ull) + ((offset) & 3) * 0x40000ull;
}
#else
#define CVMX_L2C_TADX_IEN(offset) (CVMX_ADD_IO_SEG(0x0001180080A00000ull) + ((offset) & 3) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_TADX_INT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_L2C_TADX_INT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080A00028ull) + ((offset) & 7) * 0x40000ull;
}
#else
#define CVMX_L2C_TADX_INT(offset) (CVMX_ADD_IO_SEG(0x0001180080A00028ull) + ((offset) & 7) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_TADX_PFC0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_L2C_TADX_PFC0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080A00400ull) + ((offset) & 3) * 0x40000ull;
}
#else
#define CVMX_L2C_TADX_PFC0(offset) (CVMX_ADD_IO_SEG(0x0001180080A00400ull) + ((offset) & 3) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_TADX_PFC1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_L2C_TADX_PFC1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080A00408ull) + ((offset) & 3) * 0x40000ull;
}
#else
#define CVMX_L2C_TADX_PFC1(offset) (CVMX_ADD_IO_SEG(0x0001180080A00408ull) + ((offset) & 3) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_TADX_PFC2(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_L2C_TADX_PFC2(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080A00410ull) + ((offset) & 3) * 0x40000ull;
}
#else
#define CVMX_L2C_TADX_PFC2(offset) (CVMX_ADD_IO_SEG(0x0001180080A00410ull) + ((offset) & 3) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_TADX_PFC3(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_L2C_TADX_PFC3(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080A00418ull) + ((offset) & 3) * 0x40000ull;
}
#else
#define CVMX_L2C_TADX_PFC3(offset) (CVMX_ADD_IO_SEG(0x0001180080A00418ull) + ((offset) & 3) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_TADX_PFCX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset <= 3)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset <= 3)) && ((block_id <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 7)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 3)) && ((block_id <= 7)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset <= 3)) && ((block_id <= 3))))))
		cvmx_warn("CVMX_L2C_TADX_PFCX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180080A00400ull) + (((offset) & 3) + ((block_id) & 7) * 0x8000ull) * 8;
}
#else
#define CVMX_L2C_TADX_PFCX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180080A00400ull) + (((offset) & 3) + ((block_id) & 7) * 0x8000ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_TADX_PRF(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_L2C_TADX_PRF(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080A00008ull) + ((offset) & 7) * 0x40000ull;
}
#else
#define CVMX_L2C_TADX_PRF(offset) (CVMX_ADD_IO_SEG(0x0001180080A00008ull) + ((offset) & 7) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_TADX_STAT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_L2C_TADX_STAT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080A00020ull) + ((offset) & 7) * 0x40000ull;
}
#else
#define CVMX_L2C_TADX_STAT(offset) (CVMX_ADD_IO_SEG(0x0001180080A00020ull) + ((offset) & 7) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_TADX_TAG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_L2C_TADX_TAG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080A00010ull) + ((offset) & 7) * 0x40000ull;
}
#else
#define CVMX_L2C_TADX_TAG(offset) (CVMX_ADD_IO_SEG(0x0001180080A00010ull) + ((offset) & 7) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_TADX_TIMEOUT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_L2C_TADX_TIMEOUT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080A007C8ull) + ((offset) & 7) * 0x40000ull;
}
#else
#define CVMX_L2C_TADX_TIMEOUT(offset) (CVMX_ADD_IO_SEG(0x0001180080A007C8ull) + ((offset) & 7) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_TADX_TIMETWO(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_L2C_TADX_TIMETWO(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080A007C0ull) + ((offset) & 7) * 0x40000ull;
}
#else
#define CVMX_L2C_TADX_TIMETWO(offset) (CVMX_ADD_IO_SEG(0x0001180080A007C0ull) + ((offset) & 7) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_TAD_CTL CVMX_L2C_TAD_CTL_FUNC()
static inline uint64_t CVMX_L2C_TAD_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_L2C_TAD_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180080800018ull);
}
#else
#define CVMX_L2C_TAD_CTL (CVMX_ADD_IO_SEG(0x0001180080800018ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_TBFX_BIST_STATUS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_L2C_TBFX_BIST_STATUS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080A007E8ull) + ((offset) & 7) * 0x40000ull;
}
#else
#define CVMX_L2C_TBFX_BIST_STATUS(offset) (CVMX_ADD_IO_SEG(0x0001180080A007E8ull) + ((offset) & 7) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_TDTX_BIST_STATUS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_L2C_TDTX_BIST_STATUS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080A007F0ull) + ((offset) & 7) * 0x40000ull;
}
#else
#define CVMX_L2C_TDTX_BIST_STATUS(offset) (CVMX_ADD_IO_SEG(0x0001180080A007F0ull) + ((offset) & 7) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_TQDX_ERR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_L2C_TQDX_ERR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080A007D8ull) + ((offset) & 7) * 0x40000ull;
}
#else
#define CVMX_L2C_TQDX_ERR(offset) (CVMX_ADD_IO_SEG(0x0001180080A007D8ull) + ((offset) & 7) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_TTGX_BIST_STATUS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_L2C_TTGX_BIST_STATUS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080A007F8ull) + ((offset) & 7) * 0x40000ull;
}
#else
#define CVMX_L2C_TTGX_BIST_STATUS(offset) (CVMX_ADD_IO_SEG(0x0001180080A007F8ull) + ((offset) & 7) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_TTGX_ERR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_L2C_TTGX_ERR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080A007E0ull) + ((offset) & 7) * 0x40000ull;
}
#else
#define CVMX_L2C_TTGX_ERR(offset) (CVMX_ADD_IO_SEG(0x0001180080A007E0ull) + ((offset) & 7) * 0x40000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_VER_ID CVMX_L2C_VER_ID_FUNC()
static inline uint64_t CVMX_L2C_VER_ID_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_L2C_VER_ID not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800808007E0ull);
}
#else
#define CVMX_L2C_VER_ID (CVMX_ADD_IO_SEG(0x00011800808007E0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_VER_IOB CVMX_L2C_VER_IOB_FUNC()
static inline uint64_t CVMX_L2C_VER_IOB_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_L2C_VER_IOB not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800808007F0ull);
}
#else
#define CVMX_L2C_VER_IOB (CVMX_ADD_IO_SEG(0x00011800808007F0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_VER_MSC CVMX_L2C_VER_MSC_FUNC()
static inline uint64_t CVMX_L2C_VER_MSC_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_L2C_VER_MSC not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800808007D0ull);
}
#else
#define CVMX_L2C_VER_MSC (CVMX_ADD_IO_SEG(0x00011800808007D0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_VER_PP CVMX_L2C_VER_PP_FUNC()
static inline uint64_t CVMX_L2C_VER_PP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_L2C_VER_PP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800808007E8ull);
}
#else
#define CVMX_L2C_VER_PP (CVMX_ADD_IO_SEG(0x00011800808007E8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_VIRTID_IOBX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_L2C_VIRTID_IOBX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800808C0200ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_L2C_VIRTID_IOBX(offset) (CVMX_ADD_IO_SEG(0x00011800808C0200ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_VIRTID_PPX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 5))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 9))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_L2C_VIRTID_PPX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800808C0000ull) + ((offset) & 31) * 8;
}
#else
#define CVMX_L2C_VIRTID_PPX(offset) (CVMX_ADD_IO_SEG(0x00011800808C0000ull) + ((offset) & 31) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_VRT_CTL CVMX_L2C_VRT_CTL_FUNC()
static inline uint64_t CVMX_L2C_VRT_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_L2C_VRT_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180080800010ull);
}
#else
#define CVMX_L2C_VRT_CTL (CVMX_ADD_IO_SEG(0x0001180080800010ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_VRT_MEMX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_L2C_VRT_MEMX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080900000ull) + ((offset) & 1023) * 8;
}
#else
#define CVMX_L2C_VRT_MEMX(offset) (CVMX_ADD_IO_SEG(0x0001180080900000ull) + ((offset) & 1023) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_WPAR_IOBX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_L2C_WPAR_IOBX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080840200ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_L2C_WPAR_IOBX(offset) (CVMX_ADD_IO_SEG(0x0001180080840200ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_WPAR_PPX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 5))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 9))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 15))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 47))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 47))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 15)))))
		cvmx_warn("CVMX_L2C_WPAR_PPX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080840000ull) + ((offset) & 63) * 8;
}
#else
#define CVMX_L2C_WPAR_PPX(offset) (CVMX_ADD_IO_SEG(0x0001180080840000ull) + ((offset) & 63) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_XMCX_PFC(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return CVMX_ADD_IO_SEG(0x0001180080800800ull) + ((offset) & 0) * 64;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 3))
				return CVMX_ADD_IO_SEG(0x0001180080800800ull) + ((offset) & 3) * 64;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 9))
					return CVMX_ADD_IO_SEG(0x0001180080800800ull) + ((offset) & 15) * 64;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 9))
					return CVMX_ADD_IO_SEG(0x0001180080800800ull) + ((offset) & 15) * 64;

			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return CVMX_ADD_IO_SEG(0x0001180080800400ull);
			break;
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 3))
				return CVMX_ADD_IO_SEG(0x0001180080800400ull) + ((offset) & 3) * 64;
			break;
	}
	cvmx_warn("CVMX_L2C_XMCX_PFC (offset = %lu) not supported on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080800800ull) + ((offset) & 3) * 64;
}
#else
static inline uint64_t CVMX_L2C_XMCX_PFC(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001180080800800ull) + (offset) * 64;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001180080800800ull) + (offset) * 64;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return CVMX_ADD_IO_SEG(0x0001180080800800ull) + (offset) * 64;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return CVMX_ADD_IO_SEG(0x0001180080800800ull) + (offset) * 64;

		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001180080800400ull);
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001180080800400ull) + (offset) * 64;
	}
	return CVMX_ADD_IO_SEG(0x0001180080800800ull) + (offset) * 64;
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_L2C_XMC_CMD CVMX_L2C_XMC_CMD_FUNC()
static inline uint64_t CVMX_L2C_XMC_CMD_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF71XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_L2C_XMC_CMD not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180080800028ull);
}
#else
#define CVMX_L2C_XMC_CMD (CVMX_ADD_IO_SEG(0x0001180080800028ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_L2C_XMDX_PFC(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return CVMX_ADD_IO_SEG(0x0001180080800808ull) + ((offset) & 0) * 64;
			break;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			if ((offset <= 3))
				return CVMX_ADD_IO_SEG(0x0001180080800808ull) + ((offset) & 3) * 64;
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset <= 9))
					return CVMX_ADD_IO_SEG(0x0001180080800808ull) + ((offset) & 15) * 64;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset <= 9))
					return CVMX_ADD_IO_SEG(0x0001180080800808ull) + ((offset) & 15) * 64;

			break;
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return CVMX_ADD_IO_SEG(0x0001180080800408ull);
			break;
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset <= 3))
				return CVMX_ADD_IO_SEG(0x0001180080800408ull) + ((offset) & 3) * 64;
			break;
	}
	cvmx_warn("CVMX_L2C_XMDX_PFC (offset = %lu) not supported on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180080800808ull) + ((offset) & 3) * 64;
}
#else
static inline uint64_t CVMX_L2C_XMDX_PFC(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN70XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001180080800808ull) + (offset) * 64;
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001180080800808ull) + (offset) * 64;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return CVMX_ADD_IO_SEG(0x0001180080800808ull) + (offset) * 64;
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return CVMX_ADD_IO_SEG(0x0001180080800808ull) + (offset) * 64;

		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001180080800408ull);
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x0001180080800408ull) + (offset) * 64;
	}
	return CVMX_ADD_IO_SEG(0x0001180080800808ull) + (offset) * 64;
}
#endif

/**
 * cvmx_l2c_big_ctl
 *
 * L2C_BIG_CTL = L2C Big memory control register
 *
 *
 * Notes:
 * (1) BIGRD interrupts can occur during normal operation as the PP's are allowed to prefetch to
 *     non-existent memory locations.  Therefore, BIGRD is for informational purposes only.
 *
 * (2) When HOLEWR/BIGWR blocks a store L2C_VER_ID, L2C_VER_PP, L2C_VER_IOB, and L2C_VER_MSC will be
 *     loaded just like a store which is blocked by VRTWR.  Additionally, L2C_ERR_XMC will be loaded.
 */
union cvmx_l2c_big_ctl {
	uint64_t u64;
	struct cvmx_l2c_big_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t maxdram                      : 4;  /**< Amount of configured DRAM.
                                                         0x0 = reserved.
                                                         0x1 = 512 MB.
                                                         0x2 = 1 GB.
                                                         0x3 = 2 GB.
                                                         0x4 = 4 GB.
                                                         0x5 = 8 GB.
                                                         0x6 = 16 GB.
                                                         0x7 = 32 GB.
                                                         0x8 = 64 GB.
                                                         0x9 = 128 GB.
                                                         0xA = 256 GB.
                                                         0xB = 512 GB.
                                                         0xC = 1 TB.
                                                         0xD-0xF= reserved.
                                                         Violations of this limit cause L2C to set L2C_TAD()_INT[BIGRD/BIGWR]. */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t maxdram                      : 4;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_l2c_big_ctl_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t maxdram                      : 4;  /**< Amount of configured DRAM
                                                             0 = reserved
                                                             1 = 512MB
                                                             2 = 1GB
                                                             3 = 2GB
                                                             4 = 4GB
                                                             5 = 8GB
                                                             6 = 16GB
                                                             7 = 32GB
                                                             8 = 64GB     (**reserved in 63xx**)
                                                             9 = 128GB    (**reserved in 63xx**)
                                                             10-15 reserved
                                                         Violations of this limit causes
                                                         L2C to set L2C_INT_REG[BIGRD/BIGWR]. */
	uint64_t reserved_1_3                 : 3;
	uint64_t disable                      : 1;  /**< When set, disables the BIGWR/BIGRD logic completely
                                                         and reverts HOLEWR to 63xx pass 1.x behavior.
                                                         When clear, BIGWR and HOLEWR block stores in the same
                                                         same manner as the VRT logic, and BIGRD is reported. */
#else
	uint64_t disable                      : 1;
	uint64_t reserved_1_3                 : 3;
	uint64_t maxdram                      : 4;
	uint64_t reserved_8_63                : 56;
#endif
	} cn61xx;
	struct cvmx_l2c_big_ctl_cn61xx        cn63xx;
	struct cvmx_l2c_big_ctl_cn61xx        cn66xx;
	struct cvmx_l2c_big_ctl_cn61xx        cn68xx;
	struct cvmx_l2c_big_ctl_cn61xx        cn68xxp1;
	struct cvmx_l2c_big_ctl_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t maxdram                      : 4;  /**< Amount of configured DRAM.
                                                         0x0 = reserved.
                                                         0x1 = 512 MB.
                                                         0x2 = 1 GB.
                                                         0x3 = 2 GB.
                                                         0x4 = 4 GB.
                                                         0x5 = 8 GB.
                                                         0x6 = 16 GB.
                                                         0x7 = 32 GB.
                                                         0x8 = 64 GB.
                                                         0x9 = 128 GB.
                                                         0xA = 256 GB.
                                                         0xB = 512 GB.
                                                         0xC-0xF= reserved.
                                                         Violations of this limit causes L2C to set L2C_TAD*_INT[BIGRD/BIGWR].
                                                         BIGRD interrupts can occur during normal operation as the cores are allowed to prefetch to
                                                         nonexistent memory locations. Therefore, BIGRD is for informational purposes only.
                                                         When a HOLERD/BIGRD occurs or HOLEWR/BIGWR blocks a store operation, L2C_TAD(0..0)_ERR is
                                                         loaded. L2C_TAD(0..0)_ERR is not locked for a BIGRD, however. */
	uint64_t reserved_1_3                 : 3;
	uint64_t disbig                       : 1;  /**< Disable the BIG/HOLE logic. When set, the BIG/HOLE is logic disabled completely. When
                                                         clear, BIGWR and HOLEWR block stores and BIGRD/HOLERD is reported. */
#else
	uint64_t disbig                       : 1;
	uint64_t reserved_1_3                 : 3;
	uint64_t maxdram                      : 4;
	uint64_t reserved_8_63                : 56;
#endif
	} cn70xx;
	struct cvmx_l2c_big_ctl_cn70xx        cn70xxp1;
	struct cvmx_l2c_big_ctl_cn70xx        cn73xx;
	struct cvmx_l2c_big_ctl_cn70xx        cn78xx;
	struct cvmx_l2c_big_ctl_cn70xx        cn78xxp1;
	struct cvmx_l2c_big_ctl_cn61xx        cnf71xx;
	struct cvmx_l2c_big_ctl_cn70xx        cnf75xx;
};
typedef union cvmx_l2c_big_ctl cvmx_l2c_big_ctl_t;

/**
 * cvmx_l2c_bst
 *
 * L2C_BST = L2C BIST Status
 *
 */
union cvmx_l2c_bst {
	uint64_t u64;
	struct cvmx_l2c_bst_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dutfl                        : 32; /**< BIST failure status for PP0-3 DUT */
	uint64_t rbffl                        : 4;  /**< BIST failure status for RBF0-3 */
	uint64_t xbffl                        : 4;  /**< BIST failure status for XBF0-3 */
	uint64_t tdpfl                        : 4;  /**< BIST failure status for TDP0-3 */
	uint64_t ioccmdfl                     : 4;  /**< BIST failure status for IOCCMD */
	uint64_t iocdatfl                     : 4;  /**< BIST failure status for IOCDAT */
	uint64_t dutresfl                     : 4;  /**< BIST failure status for DUTRES */
	uint64_t vrtfl                        : 4;  /**< BIST failure status for VRT0 */
	uint64_t tdffl                        : 4;  /**< BIST failure status for TDF0 */
#else
	uint64_t tdffl                        : 4;
	uint64_t vrtfl                        : 4;
	uint64_t dutresfl                     : 4;
	uint64_t iocdatfl                     : 4;
	uint64_t ioccmdfl                     : 4;
	uint64_t tdpfl                        : 4;
	uint64_t xbffl                        : 4;
	uint64_t rbffl                        : 4;
	uint64_t dutfl                        : 32;
#endif
	} s;
	struct cvmx_l2c_bst_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t dutfl                        : 4;  /**< BIST failure status for PP0-3 DUT */
	uint64_t reserved_17_31               : 15;
	uint64_t ioccmdfl                     : 1;  /**< BIST failure status for IOCCMD */
	uint64_t reserved_13_15               : 3;
	uint64_t iocdatfl                     : 1;  /**< BIST failure status for IOCDAT */
	uint64_t reserved_9_11                : 3;
	uint64_t dutresfl                     : 1;  /**< BIST failure status for DUTRES */
	uint64_t reserved_5_7                 : 3;
	uint64_t vrtfl                        : 1;  /**< BIST failure status for VRT0 */
	uint64_t reserved_1_3                 : 3;
	uint64_t tdffl                        : 1;  /**< BIST failure status for TDF0 */
#else
	uint64_t tdffl                        : 1;
	uint64_t reserved_1_3                 : 3;
	uint64_t vrtfl                        : 1;
	uint64_t reserved_5_7                 : 3;
	uint64_t dutresfl                     : 1;
	uint64_t reserved_9_11                : 3;
	uint64_t iocdatfl                     : 1;
	uint64_t reserved_13_15               : 3;
	uint64_t ioccmdfl                     : 1;
	uint64_t reserved_17_31               : 15;
	uint64_t dutfl                        : 4;
	uint64_t reserved_36_63               : 28;
#endif
	} cn61xx;
	struct cvmx_l2c_bst_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_38_63               : 26;
	uint64_t dutfl                        : 6;  /**< BIST failure status for PP0-5 DUT */
	uint64_t reserved_17_31               : 15;
	uint64_t ioccmdfl                     : 1;  /**< BIST failure status for IOCCMD */
	uint64_t reserved_13_15               : 3;
	uint64_t iocdatfl                     : 1;  /**< BIST failure status for IOCDAT */
	uint64_t reserved_9_11                : 3;
	uint64_t dutresfl                     : 1;  /**< BIST failure status for DUTRES */
	uint64_t reserved_5_7                 : 3;
	uint64_t vrtfl                        : 1;  /**< BIST failure status for VRT0 */
	uint64_t reserved_1_3                 : 3;
	uint64_t tdffl                        : 1;  /**< BIST failure status for TDF0 */
#else
	uint64_t tdffl                        : 1;
	uint64_t reserved_1_3                 : 3;
	uint64_t vrtfl                        : 1;
	uint64_t reserved_5_7                 : 3;
	uint64_t dutresfl                     : 1;
	uint64_t reserved_9_11                : 3;
	uint64_t iocdatfl                     : 1;
	uint64_t reserved_13_15               : 3;
	uint64_t ioccmdfl                     : 1;
	uint64_t reserved_17_31               : 15;
	uint64_t dutfl                        : 6;
	uint64_t reserved_38_63               : 26;
#endif
	} cn63xx;
	struct cvmx_l2c_bst_cn63xx            cn63xxp1;
	struct cvmx_l2c_bst_cn66xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_42_63               : 22;
	uint64_t dutfl                        : 10; /**< BIST failure status for PP0-9 DUT */
	uint64_t reserved_17_31               : 15;
	uint64_t ioccmdfl                     : 1;  /**< BIST failure status for IOCCMD */
	uint64_t reserved_13_15               : 3;
	uint64_t iocdatfl                     : 1;  /**< BIST failure status for IOCDAT */
	uint64_t reserved_9_11                : 3;
	uint64_t dutresfl                     : 1;  /**< BIST failure status for DUTRES */
	uint64_t reserved_5_7                 : 3;
	uint64_t vrtfl                        : 1;  /**< BIST failure status for VRT0 */
	uint64_t reserved_1_3                 : 3;
	uint64_t tdffl                        : 1;  /**< BIST failure status for TDF0 */
#else
	uint64_t tdffl                        : 1;
	uint64_t reserved_1_3                 : 3;
	uint64_t vrtfl                        : 1;
	uint64_t reserved_5_7                 : 3;
	uint64_t dutresfl                     : 1;
	uint64_t reserved_9_11                : 3;
	uint64_t iocdatfl                     : 1;
	uint64_t reserved_13_15               : 3;
	uint64_t ioccmdfl                     : 1;
	uint64_t reserved_17_31               : 15;
	uint64_t dutfl                        : 10;
	uint64_t reserved_42_63               : 22;
#endif
	} cn66xx;
	struct cvmx_l2c_bst_s                 cn68xx;
	struct cvmx_l2c_bst_s                 cn68xxp1;
	struct cvmx_l2c_bst_cn61xx            cnf71xx;
};
typedef union cvmx_l2c_bst cvmx_l2c_bst_t;

/**
 * cvmx_l2c_bst0
 *
 * L2C_BST0 = L2C BIST 0 CTL/STAT
 *
 */
union cvmx_l2c_bst0 {
	uint64_t u64;
	struct cvmx_l2c_bst0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t dtbnk                        : 1;  /**< DuTag Bank#
                                                         When DT=1(BAD), this field provides additional information
                                                         about which DuTag Bank (0/1) failed. */
	uint64_t wlb_msk                      : 4;  /**< Bist Results for WLB-MSK RAM [DP0-3]
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t dtcnt                        : 13; /**< DuTag BiST Counter (used to help isolate the failure)
                                                         [12]:    i (0=FORWARD/1=REVERSE pass)
                                                         [11:10]: j (Pattern# 1 of 4)
                                                         [9:4]:   k (DT Index 1 of 64)
                                                         [3:0]:   l (DT# 1 of 16 DTs) */
	uint64_t dt                           : 1;  /**< Bist Results for DuTAG RAM(s)
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t stin_msk                     : 1;  /**< Bist Results for STIN-MSK RAM
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t wlb_dat                      : 4;  /**< Bist Results for WLB-DAT RAM [DP0-3]
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
#else
	uint64_t wlb_dat                      : 4;
	uint64_t stin_msk                     : 1;
	uint64_t dt                           : 1;
	uint64_t dtcnt                        : 13;
	uint64_t wlb_msk                      : 4;
	uint64_t dtbnk                        : 1;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_l2c_bst0_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_23_63               : 41;
	uint64_t wlb_msk                      : 4;  /**< Bist Results for WLB-MSK RAM [DP0-3]
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t reserved_15_18               : 4;
	uint64_t dtcnt                        : 9;  /**< DuTag BiST Counter (used to help isolate the failure)
                                                         [8]:   i (0=FORWARD/1=REVERSE pass)
                                                         [7:6]: j (Pattern# 1 of 4)
                                                         [5:0]: k (DT Index 1 of 64) */
	uint64_t dt                           : 1;  /**< Bist Results for DuTAG RAM(s)
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t reserved_4_4                 : 1;
	uint64_t wlb_dat                      : 4;  /**< Bist Results for WLB-DAT RAM [DP0-3]
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
#else
	uint64_t wlb_dat                      : 4;
	uint64_t reserved_4_4                 : 1;
	uint64_t dt                           : 1;
	uint64_t dtcnt                        : 9;
	uint64_t reserved_15_18               : 4;
	uint64_t wlb_msk                      : 4;
	uint64_t reserved_23_63               : 41;
#endif
	} cn30xx;
	struct cvmx_l2c_bst0_cn31xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_23_63               : 41;
	uint64_t wlb_msk                      : 4;  /**< Bist Results for WLB-MSK RAM [DP0-3]
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t reserved_16_18               : 3;
	uint64_t dtcnt                        : 10; /**< DuTag BiST Counter (used to help isolate the failure)
                                                         [9]:   i (0=FORWARD/1=REVERSE pass)
                                                         [8:7]: j (Pattern# 1 of 4)
                                                         [6:1]: k (DT Index 1 of 64)
                                                         [0]:   l (DT# 1 of 2 DTs) */
	uint64_t dt                           : 1;  /**< Bist Results for DuTAG RAM(s)
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t stin_msk                     : 1;  /**< Bist Results for STIN-MSK RAM
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t wlb_dat                      : 4;  /**< Bist Results for WLB-DAT RAM [DP0-3]
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
#else
	uint64_t wlb_dat                      : 4;
	uint64_t stin_msk                     : 1;
	uint64_t dt                           : 1;
	uint64_t dtcnt                        : 10;
	uint64_t reserved_16_18               : 3;
	uint64_t wlb_msk                      : 4;
	uint64_t reserved_23_63               : 41;
#endif
	} cn31xx;
	struct cvmx_l2c_bst0_cn38xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_19_63               : 45;
	uint64_t dtcnt                        : 13; /**< DuTag BiST Counter (used to help isolate the failure)
                                                         [12]:    i (0=FORWARD/1=REVERSE pass)
                                                         [11:10]: j (Pattern# 1 of 4)
                                                         [9:4]:   k (DT Index 1 of 64)
                                                         [3:0]:   l (DT# 1 of 16 DTs) */
	uint64_t dt                           : 1;  /**< Bist Results for DuTAG RAM(s)
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t stin_msk                     : 1;  /**< Bist Results for STIN-MSK RAM
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t wlb_dat                      : 4;  /**< Bist Results for WLB-DAT RAM [DP0-3]
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
#else
	uint64_t wlb_dat                      : 4;
	uint64_t stin_msk                     : 1;
	uint64_t dt                           : 1;
	uint64_t dtcnt                        : 13;
	uint64_t reserved_19_63               : 45;
#endif
	} cn38xx;
	struct cvmx_l2c_bst0_cn38xx           cn38xxp2;
	struct cvmx_l2c_bst0_cn50xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t dtbnk                        : 1;  /**< DuTag Bank#
                                                         When DT=1(BAD), this field provides additional information
                                                         about which DuTag Bank (0/1) failed. */
	uint64_t wlb_msk                      : 4;  /**< Bist Results for WLB-MSK RAM [DP0-3]
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t reserved_16_18               : 3;
	uint64_t dtcnt                        : 10; /**< DuTag BiST Counter (used to help isolate the failure)
                                                         [9]:   i (0=FORWARD/1=REVERSE pass)
                                                         [8:7]: j (Pattern# 1 of 4)
                                                         [6:1]: k (DT Index 1 of 64)
                                                         [0]:   l (DT# 1 of 2 DTs) */
	uint64_t dt                           : 1;  /**< Bist Results for DuTAG RAM(s)
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t stin_msk                     : 1;  /**< Bist Results for STIN-MSK RAM
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t wlb_dat                      : 4;  /**< Bist Results for WLB-DAT RAM [DP0-3]
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
#else
	uint64_t wlb_dat                      : 4;
	uint64_t stin_msk                     : 1;
	uint64_t dt                           : 1;
	uint64_t dtcnt                        : 10;
	uint64_t reserved_16_18               : 3;
	uint64_t wlb_msk                      : 4;
	uint64_t dtbnk                        : 1;
	uint64_t reserved_24_63               : 40;
#endif
	} cn50xx;
	struct cvmx_l2c_bst0_cn50xx           cn52xx;
	struct cvmx_l2c_bst0_cn50xx           cn52xxp1;
	struct cvmx_l2c_bst0_s                cn56xx;
	struct cvmx_l2c_bst0_s                cn56xxp1;
	struct cvmx_l2c_bst0_s                cn58xx;
	struct cvmx_l2c_bst0_s                cn58xxp1;
};
typedef union cvmx_l2c_bst0 cvmx_l2c_bst0_t;

/**
 * cvmx_l2c_bst1
 *
 * L2C_BST1 = L2C BIST 1 CTL/STAT
 *
 */
union cvmx_l2c_bst1 {
	uint64_t u64;
	struct cvmx_l2c_bst1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t l2t                          : 9;  /**< Bist Results for L2T (USE+8SET RAMs)
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
#else
	uint64_t l2t                          : 9;
	uint64_t reserved_9_63                : 55;
#endif
	} s;
	struct cvmx_l2c_bst1_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t vwdf                         : 4;  /**< Bist Results for VWDF RAMs
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t lrf                          : 2;  /**< Bist Results for LRF RAMs (PLC+ILC)
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t vab_vwcf                     : 1;  /**< Bist Results for VAB VWCF_MEM
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t reserved_5_8                 : 4;
	uint64_t l2t                          : 5;  /**< Bist Results for L2T (USE+4SET RAMs)
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
#else
	uint64_t l2t                          : 5;
	uint64_t reserved_5_8                 : 4;
	uint64_t vab_vwcf                     : 1;
	uint64_t lrf                          : 2;
	uint64_t vwdf                         : 4;
	uint64_t reserved_16_63               : 48;
#endif
	} cn30xx;
	struct cvmx_l2c_bst1_cn30xx           cn31xx;
	struct cvmx_l2c_bst1_cn38xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t vwdf                         : 4;  /**< Bist Results for VWDF RAMs
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t lrf                          : 2;  /**< Bist Results for LRF RAMs (PLC+ILC)
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t vab_vwcf                     : 1;  /**< Bist Results for VAB VWCF_MEM
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t l2t                          : 9;  /**< Bist Results for L2T (USE+8SET RAMs)
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
#else
	uint64_t l2t                          : 9;
	uint64_t vab_vwcf                     : 1;
	uint64_t lrf                          : 2;
	uint64_t vwdf                         : 4;
	uint64_t reserved_16_63               : 48;
#endif
	} cn38xx;
	struct cvmx_l2c_bst1_cn38xx           cn38xxp2;
	struct cvmx_l2c_bst1_cn38xx           cn50xx;
	struct cvmx_l2c_bst1_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_19_63               : 45;
	uint64_t plc2                         : 1;  /**< Bist Results for PLC2 RAM
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t plc1                         : 1;  /**< Bist Results for PLC1 RAM
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t plc0                         : 1;  /**< Bist Results for PLC0 RAM
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t vwdf                         : 4;  /**< Bist Results for VWDF RAMs
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t reserved_11_11               : 1;
	uint64_t ilc                          : 1;  /**< Bist Results for ILC RAM
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t vab_vwcf                     : 1;  /**< Bist Results for VAB VWCF_MEM
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t l2t                          : 9;  /**< Bist Results for L2T (USE+8SET RAMs)
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
#else
	uint64_t l2t                          : 9;
	uint64_t vab_vwcf                     : 1;
	uint64_t ilc                          : 1;
	uint64_t reserved_11_11               : 1;
	uint64_t vwdf                         : 4;
	uint64_t plc0                         : 1;
	uint64_t plc1                         : 1;
	uint64_t plc2                         : 1;
	uint64_t reserved_19_63               : 45;
#endif
	} cn52xx;
	struct cvmx_l2c_bst1_cn52xx           cn52xxp1;
	struct cvmx_l2c_bst1_cn56xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t plc2                         : 1;  /**< Bist Results for LRF RAMs (ILC)
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t plc1                         : 1;  /**< Bist Results for LRF RAMs (ILC)
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t plc0                         : 1;  /**< Bist Results for LRF RAMs (ILC)
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t ilc                          : 1;  /**< Bist Results for LRF RAMs (ILC)
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t vwdf1                        : 4;  /**< Bist Results for VWDF1 RAMs
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t vwdf0                        : 4;  /**< Bist Results for VWDF0 RAMs
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t vab_vwcf1                    : 1;  /**< Bist Results for VAB VWCF1_MEM */
	uint64_t reserved_10_10               : 1;
	uint64_t vab_vwcf0                    : 1;  /**< Bist Results for VAB VWCF0_MEM
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t l2t                          : 9;  /**< Bist Results for L2T (USE+8SET RAMs)
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
#else
	uint64_t l2t                          : 9;
	uint64_t vab_vwcf0                    : 1;
	uint64_t reserved_10_10               : 1;
	uint64_t vab_vwcf1                    : 1;
	uint64_t vwdf0                        : 4;
	uint64_t vwdf1                        : 4;
	uint64_t ilc                          : 1;
	uint64_t plc0                         : 1;
	uint64_t plc1                         : 1;
	uint64_t plc2                         : 1;
	uint64_t reserved_24_63               : 40;
#endif
	} cn56xx;
	struct cvmx_l2c_bst1_cn56xx           cn56xxp1;
	struct cvmx_l2c_bst1_cn38xx           cn58xx;
	struct cvmx_l2c_bst1_cn38xx           cn58xxp1;
};
typedef union cvmx_l2c_bst1 cvmx_l2c_bst1_t;

/**
 * cvmx_l2c_bst2
 *
 * L2C_BST2 = L2C BIST 2 CTL/STAT
 *
 */
union cvmx_l2c_bst2 {
	uint64_t u64;
	struct cvmx_l2c_bst2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t mrb                          : 4;  /**< Bist Results for MRB RAMs
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t reserved_4_11                : 8;
	uint64_t ipcbst                       : 1;  /**< Bist Results for RFB IPC RAM
                                                         - 1: BAD */
	uint64_t picbst                       : 1;  /**< Bist Results for RFB PIC RAM
                                                         - 1: BAD */
	uint64_t xrdmsk                       : 1;  /**< Bist Results for RFB XRD-MSK RAM
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t xrddat                       : 1;  /**< Bist Results for RFB XRD-DAT RAM
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
#else
	uint64_t xrddat                       : 1;
	uint64_t xrdmsk                       : 1;
	uint64_t picbst                       : 1;
	uint64_t ipcbst                       : 1;
	uint64_t reserved_4_11                : 8;
	uint64_t mrb                          : 4;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_l2c_bst2_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t mrb                          : 4;  /**< Bist Results for MRB RAMs
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t rmdf                         : 4;  /**< Bist Results for RMDF RAMs
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t reserved_4_7                 : 4;
	uint64_t ipcbst                       : 1;  /**< Bist Results for RFB IPC RAM
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t reserved_2_2                 : 1;
	uint64_t xrdmsk                       : 1;  /**< Bist Results for RFB XRD-MSK RAM
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t xrddat                       : 1;  /**< Bist Results for RFB XRD-DAT RAM
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
#else
	uint64_t xrddat                       : 1;
	uint64_t xrdmsk                       : 1;
	uint64_t reserved_2_2                 : 1;
	uint64_t ipcbst                       : 1;
	uint64_t reserved_4_7                 : 4;
	uint64_t rmdf                         : 4;
	uint64_t mrb                          : 4;
	uint64_t reserved_16_63               : 48;
#endif
	} cn30xx;
	struct cvmx_l2c_bst2_cn30xx           cn31xx;
	struct cvmx_l2c_bst2_cn38xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t mrb                          : 4;  /**< Bist Results for MRB RAMs
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t rmdf                         : 4;  /**< Bist Results for RMDF RAMs
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t rhdf                         : 4;  /**< Bist Results for RHDF RAMs
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t ipcbst                       : 1;  /**< Bist Results for RFB IPC RAM
                                                         - 1: BAD */
	uint64_t picbst                       : 1;  /**< Bist Results for RFB PIC RAM
                                                         - 1: BAD */
	uint64_t xrdmsk                       : 1;  /**< Bist Results for RFB XRD-MSK RAM
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t xrddat                       : 1;  /**< Bist Results for RFB XRD-DAT RAM
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
#else
	uint64_t xrddat                       : 1;
	uint64_t xrdmsk                       : 1;
	uint64_t picbst                       : 1;
	uint64_t ipcbst                       : 1;
	uint64_t rhdf                         : 4;
	uint64_t rmdf                         : 4;
	uint64_t mrb                          : 4;
	uint64_t reserved_16_63               : 48;
#endif
	} cn38xx;
	struct cvmx_l2c_bst2_cn38xx           cn38xxp2;
	struct cvmx_l2c_bst2_cn30xx           cn50xx;
	struct cvmx_l2c_bst2_cn30xx           cn52xx;
	struct cvmx_l2c_bst2_cn30xx           cn52xxp1;
	struct cvmx_l2c_bst2_cn56xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t mrb                          : 4;  /**< Bist Results for MRB RAMs
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t rmdb                         : 4;  /**< Bist Results for RMDB RAMs
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t rhdb                         : 4;  /**< Bist Results for RHDB RAMs
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t ipcbst                       : 1;  /**< Bist Results for RFB IPC RAM
                                                         - 1: BAD */
	uint64_t picbst                       : 1;  /**< Bist Results for RFB PIC RAM
                                                         - 1: BAD */
	uint64_t xrdmsk                       : 1;  /**< Bist Results for RFB XRD-MSK RAM
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t xrddat                       : 1;  /**< Bist Results for RFB XRD-DAT RAM
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
#else
	uint64_t xrddat                       : 1;
	uint64_t xrdmsk                       : 1;
	uint64_t picbst                       : 1;
	uint64_t ipcbst                       : 1;
	uint64_t rhdb                         : 4;
	uint64_t rmdb                         : 4;
	uint64_t mrb                          : 4;
	uint64_t reserved_16_63               : 48;
#endif
	} cn56xx;
	struct cvmx_l2c_bst2_cn56xx           cn56xxp1;
	struct cvmx_l2c_bst2_cn56xx           cn58xx;
	struct cvmx_l2c_bst2_cn56xx           cn58xxp1;
};
typedef union cvmx_l2c_bst2 cvmx_l2c_bst2_t;

/**
 * cvmx_l2c_bst_mem#
 *
 * L2C_BST_MEM = L2C MEM BIST Status
 *
 *
 * Notes:
 * (1) CLEAR_BIST must be written to 1 before START_BIST is written to 1 using a separate CSR write.
 *
 * (2) CLEAR_BIST must not be changed after writing START_BIST to 1 until the BIST operation completes
 *     (indicated by START_BIST returning to 0) or operation is undefined.
 */
union cvmx_l2c_bst_memx {
	uint64_t u64;
	struct cvmx_l2c_bst_memx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t start_bist                   : 1;  /**< When written to 1, starts BIST.  Will read 1 until
                                                         BIST is complete (see Note). */
	uint64_t clear_bist                   : 1;  /**< When BIST is triggered, run clear BIST (see Note) */
	uint64_t reserved_5_61                : 57;
	uint64_t rdffl                        : 1;  /**< BIST failure status for RDF */
	uint64_t vbffl                        : 4;  /**< BIST failure status for VBF0-3 */
#else
	uint64_t vbffl                        : 4;
	uint64_t rdffl                        : 1;
	uint64_t reserved_5_61                : 57;
	uint64_t clear_bist                   : 1;
	uint64_t start_bist                   : 1;
#endif
	} s;
	struct cvmx_l2c_bst_memx_s            cn61xx;
	struct cvmx_l2c_bst_memx_s            cn63xx;
	struct cvmx_l2c_bst_memx_s            cn63xxp1;
	struct cvmx_l2c_bst_memx_s            cn66xx;
	struct cvmx_l2c_bst_memx_s            cn68xx;
	struct cvmx_l2c_bst_memx_s            cn68xxp1;
	struct cvmx_l2c_bst_memx_s            cnf71xx;
};
typedef union cvmx_l2c_bst_memx cvmx_l2c_bst_memx_t;

/**
 * cvmx_l2c_bst_tdt#
 *
 * L2C_BST_TDT = L2C TAD DaTa BIST Status
 *
 */
union cvmx_l2c_bst_tdtx {
	uint64_t u64;
	struct cvmx_l2c_bst_tdtx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t fbfrspfl                     : 8;  /**< BIST failure status for quad 0-7 FBF RSP read port */
	uint64_t sbffl                        : 8;  /**< BIST failure status for quad 0-7 SBF */
	uint64_t fbffl                        : 8;  /**< BIST failure status for quad 0-7 FBF WRP read port */
	uint64_t l2dfl                        : 8;  /**< BIST failure status for quad 0-7 L2D */
#else
	uint64_t l2dfl                        : 8;
	uint64_t fbffl                        : 8;
	uint64_t sbffl                        : 8;
	uint64_t fbfrspfl                     : 8;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_l2c_bst_tdtx_s            cn61xx;
	struct cvmx_l2c_bst_tdtx_s            cn63xx;
	struct cvmx_l2c_bst_tdtx_cn63xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t sbffl                        : 8;  /**< BIST failure status for quad 0-7 SBF */
	uint64_t fbffl                        : 8;  /**< BIST failure status for quad 0-7 FBF */
	uint64_t l2dfl                        : 8;  /**< BIST failure status for quad 0-7 L2D */
#else
	uint64_t l2dfl                        : 8;
	uint64_t fbffl                        : 8;
	uint64_t sbffl                        : 8;
	uint64_t reserved_24_63               : 40;
#endif
	} cn63xxp1;
	struct cvmx_l2c_bst_tdtx_s            cn66xx;
	struct cvmx_l2c_bst_tdtx_s            cn68xx;
	struct cvmx_l2c_bst_tdtx_s            cn68xxp1;
	struct cvmx_l2c_bst_tdtx_s            cnf71xx;
};
typedef union cvmx_l2c_bst_tdtx cvmx_l2c_bst_tdtx_t;

/**
 * cvmx_l2c_bst_ttg#
 *
 * L2C_BST_TTG = L2C TAD TaG BIST Status
 *
 */
union cvmx_l2c_bst_ttgx {
	uint64_t u64;
	struct cvmx_l2c_bst_ttgx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_17_63               : 47;
	uint64_t lrufl                        : 1;  /**< BIST failure status for tag LRU */
	uint64_t tagfl                        : 16; /**< BIST failure status for tag ways 0-15 */
#else
	uint64_t tagfl                        : 16;
	uint64_t lrufl                        : 1;
	uint64_t reserved_17_63               : 47;
#endif
	} s;
	struct cvmx_l2c_bst_ttgx_s            cn61xx;
	struct cvmx_l2c_bst_ttgx_s            cn63xx;
	struct cvmx_l2c_bst_ttgx_s            cn63xxp1;
	struct cvmx_l2c_bst_ttgx_s            cn66xx;
	struct cvmx_l2c_bst_ttgx_s            cn68xx;
	struct cvmx_l2c_bst_ttgx_s            cn68xxp1;
	struct cvmx_l2c_bst_ttgx_s            cnf71xx;
};
typedef union cvmx_l2c_bst_ttgx cvmx_l2c_bst_ttgx_t;

/**
 * cvmx_l2c_cbc#_bist_status
 */
union cvmx_l2c_cbcx_bist_status {
	uint64_t u64;
	struct cvmx_l2c_cbcx_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t rsdfl                        : 32; /**< BIST failure status for RSDQW0-31. */
#else
	uint64_t rsdfl                        : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_l2c_cbcx_bist_status_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_34_63               : 30;
	uint64_t ioccmdfl                     : 2;  /**< BIST failure status for IOCCMD0-1. */
	uint64_t rsdfl                        : 32; /**< BIST failure status for RSDQW0-31. */
#else
	uint64_t rsdfl                        : 32;
	uint64_t ioccmdfl                     : 2;
	uint64_t reserved_34_63               : 30;
#endif
	} cn70xx;
	struct cvmx_l2c_cbcx_bist_status_cn70xx cn70xxp1;
	struct cvmx_l2c_cbcx_bist_status_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_37_63               : 27;
	uint64_t mibfl                        : 5;  /**< Reserved. */
	uint64_t rsdfl                        : 32; /**< BIST failure status for RSDQW0-31. */
#else
	uint64_t rsdfl                        : 32;
	uint64_t mibfl                        : 5;
	uint64_t reserved_37_63               : 27;
#endif
	} cn73xx;
	struct cvmx_l2c_cbcx_bist_status_cn73xx cn78xx;
	struct cvmx_l2c_cbcx_bist_status_cn73xx cn78xxp1;
	struct cvmx_l2c_cbcx_bist_status_cn73xx cnf75xx;
};
typedef union cvmx_l2c_cbcx_bist_status cvmx_l2c_cbcx_bist_status_t;

/**
 * cvmx_l2c_cbc#_dll
 *
 * Register for DLL observability.
 *
 */
union cvmx_l2c_cbcx_dll {
	uint64_t u64;
	struct cvmx_l2c_cbcx_dll_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t pd_pos_rclk_refclk           : 1;  /**< Phase detector output. */
	uint64_t pdl_rclk_refclk              : 1;  /**< Phase detector output. */
	uint64_t pdr_rclk_refclk              : 1;  /**< Phase detector output. */
	uint64_t clk_invert                   : 1;  /**< Clock invert. */
	uint64_t dly_elem_enable              : 16; /**< Delay element enable. */
	uint64_t dll_setting                  : 12; /**< DLL setting. */
	uint64_t dll_state                    : 3;  /**< DLL state. */
	uint64_t dll_lock                     : 1;  /**< DLL locked. */
#else
	uint64_t dll_lock                     : 1;
	uint64_t dll_state                    : 3;
	uint64_t dll_setting                  : 12;
	uint64_t dly_elem_enable              : 16;
	uint64_t clk_invert                   : 1;
	uint64_t pdr_rclk_refclk              : 1;
	uint64_t pdl_rclk_refclk              : 1;
	uint64_t pd_pos_rclk_refclk           : 1;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_l2c_cbcx_dll_s            cn73xx;
	struct cvmx_l2c_cbcx_dll_s            cn78xx;
	struct cvmx_l2c_cbcx_dll_s            cn78xxp1;
	struct cvmx_l2c_cbcx_dll_s            cnf75xx;
};
typedef union cvmx_l2c_cbcx_dll cvmx_l2c_cbcx_dll_t;

/**
 * cvmx_l2c_cbc#_holeerr
 *
 * This register records error information for HOLE* interrupts. The first HOLEWR error locks the
 * register until the logged error type is cleared; HOLERD never locks the register.
 */
union cvmx_l2c_cbcx_holeerr {
	uint64_t u64;
	struct cvmx_l2c_cbcx_holeerr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t holerd                       : 1;  /**< Logged information is for a HOLERD error. */
	uint64_t holewr                       : 1;  /**< Logged information is for a HOLEWR error. */
	uint64_t reserved_59_61               : 3;
	uint64_t cmd                          : 8;  /**< Encoding of XMC command causing error. */
	uint64_t source                       : 7;  /**< XMC 'source' of request causing error. If SOURCE<6>==0, then SOURCE<5:0> is PPID, else
                                                         SOURCE<3:0> is BUSID of the IOB which made the request. */
	uint64_t node                         : 4;  /**< CCPI is not present. Should always read 0. */
	uint64_t addr                         : 40; /**< XMC address causing the error. This field is the physical address after hole removal and
                                                         index aliasing (if enabled). (The hole is between DR0 and DR1. Remove the hole by
                                                         subtracting 256MB from all L2/DRAM physical addresses >= 512 MB.) */
#else
	uint64_t addr                         : 40;
	uint64_t node                         : 4;
	uint64_t source                       : 7;
	uint64_t cmd                          : 8;
	uint64_t reserved_59_61               : 3;
	uint64_t holewr                       : 1;
	uint64_t holerd                       : 1;
#endif
	} s;
	struct cvmx_l2c_cbcx_holeerr_s        cn73xx;
	struct cvmx_l2c_cbcx_holeerr_s        cn78xx;
	struct cvmx_l2c_cbcx_holeerr_s        cn78xxp1;
	struct cvmx_l2c_cbcx_holeerr_s        cnf75xx;
};
typedef union cvmx_l2c_cbcx_holeerr cvmx_l2c_cbcx_holeerr_t;

/**
 * cvmx_l2c_cbc#_int
 *
 * This register is for CBC-based interrupts.
 *
 */
union cvmx_l2c_cbcx_int {
	uint64_t u64;
	struct cvmx_l2c_cbcx_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t holerd                       : 1;  /**< Read reference to 256MB hole occurred. */
	uint64_t holewr                       : 1;  /**< Write reference to 256MB hole occurred. */
	uint64_t iowrdisoci                   : 1;  /**< Reserved. */
	uint64_t iorddisoci                   : 1;  /**< Reserved. */
	uint64_t mibdbe                       : 1;  /**< Reserved. */
	uint64_t mibsbe                       : 1;  /**< Reserved. */
	uint64_t ioccmddbe                    : 1;  /**< IOCCMD double-bit error occurred. See L2C_CBC(0..3)_IOCERR for logged information. */
	uint64_t ioccmdsbe                    : 1;  /**< IOCCMD single-bit error occurred. See L2C_CBC(0..3)_IOCERR for logged information. */
	uint64_t rsddbe                       : 1;  /**< RSD double-bit error occurred. See L2C_CBC()_RSDERR for logged information.
                                                         An indication of a hardware failure and may be considered fatal. */
	uint64_t rsdsbe                       : 1;  /**< RSD single-bit error occurred. See L2C_CBC()_RSDERR for logged
                                                         information. Hardware automatically corrected the error. Software may choose to
                                                         count the number of these single-bit errors. */
#else
	uint64_t rsdsbe                       : 1;
	uint64_t rsddbe                       : 1;
	uint64_t ioccmdsbe                    : 1;
	uint64_t ioccmddbe                    : 1;
	uint64_t mibsbe                       : 1;
	uint64_t mibdbe                       : 1;
	uint64_t iorddisoci                   : 1;
	uint64_t iowrdisoci                   : 1;
	uint64_t holewr                       : 1;
	uint64_t holerd                       : 1;
	uint64_t reserved_10_63               : 54;
#endif
	} s;
	struct cvmx_l2c_cbcx_int_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t ioccmddbe                    : 1;  /**< IOCCMD double-bit error occurred. See L2C_CBC(0..3)_IOCERR for logged information. */
	uint64_t ioccmdsbe                    : 1;  /**< IOCCMD single-bit error occurred. See L2C_CBC(0..3)_IOCERR for logged information. */
	uint64_t rsddbe                       : 1;  /**< RSD double-bit error occurred. See L2C_CBC()_RSDERR for logged information. */
	uint64_t rsdsbe                       : 1;  /**< RSD single-bit error occurred. See L2C_CBC()_RSDERR for logged information. */
#else
	uint64_t rsdsbe                       : 1;
	uint64_t rsddbe                       : 1;
	uint64_t ioccmdsbe                    : 1;
	uint64_t ioccmddbe                    : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} cn70xx;
	struct cvmx_l2c_cbcx_int_cn70xx       cn70xxp1;
	struct cvmx_l2c_cbcx_int_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t holerd                       : 1;  /**< Read reference to 256MB hole occurred. */
	uint64_t holewr                       : 1;  /**< Write reference to 256MB hole occurred. */
	uint64_t iowrdisoci                   : 1;  /**< Reserved. */
	uint64_t iorddisoci                   : 1;  /**< Reserved. */
	uint64_t mibdbe                       : 1;  /**< Reserved. */
	uint64_t mibsbe                       : 1;  /**< Reserved. */
	uint64_t reserved_2_3                 : 2;
	uint64_t rsddbe                       : 1;  /**< RSD double-bit error occurred. See L2C_CBC()_RSDERR for logged information.
                                                         An indication of a hardware failure and may be considered fatal. */
	uint64_t rsdsbe                       : 1;  /**< RSD single-bit error occurred. See L2C_CBC()_RSDERR for logged
                                                         information. Hardware automatically corrected the error. Software may choose to
                                                         count the number of these single-bit errors. */
#else
	uint64_t rsdsbe                       : 1;
	uint64_t rsddbe                       : 1;
	uint64_t reserved_2_3                 : 2;
	uint64_t mibsbe                       : 1;
	uint64_t mibdbe                       : 1;
	uint64_t iorddisoci                   : 1;
	uint64_t iowrdisoci                   : 1;
	uint64_t holewr                       : 1;
	uint64_t holerd                       : 1;
	uint64_t reserved_10_63               : 54;
#endif
	} cn73xx;
	struct cvmx_l2c_cbcx_int_cn73xx       cn78xx;
	struct cvmx_l2c_cbcx_int_cn73xx       cn78xxp1;
	struct cvmx_l2c_cbcx_int_cn73xx       cnf75xx;
};
typedef union cvmx_l2c_cbcx_int cvmx_l2c_cbcx_int_t;

/**
 * cvmx_l2c_cbc#_iocerr
 *
 * Reserved.
 *
 */
union cvmx_l2c_cbcx_iocerr {
	uint64_t u64;
	struct cvmx_l2c_cbcx_iocerr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t cmddbe                       : 1;  /**< INDEX/SYN corresponds to a double-bit IOCCMD ECC error. */
	uint64_t cmdsbe                       : 1;  /**< INDEX/SYN corresponds to a single-bit IOCCMD ECC error. */
	uint64_t reserved_40_61               : 22;
	uint64_t syn                          : 8;  /**< Error syndrome. */
	uint64_t reserved_3_31                : 29;
	uint64_t xmcnum                       : 3;  /**< Indicates the XMC that had the error. */
#else
	uint64_t xmcnum                       : 3;
	uint64_t reserved_3_31                : 29;
	uint64_t syn                          : 8;
	uint64_t reserved_40_61               : 22;
	uint64_t cmdsbe                       : 1;
	uint64_t cmddbe                       : 1;
#endif
	} s;
	struct cvmx_l2c_cbcx_iocerr_s         cn70xx;
	struct cvmx_l2c_cbcx_iocerr_s         cn70xxp1;
	struct cvmx_l2c_cbcx_iocerr_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} cn73xx;
	struct cvmx_l2c_cbcx_iocerr_cn73xx    cn78xx;
	struct cvmx_l2c_cbcx_iocerr_cn73xx    cn78xxp1;
	struct cvmx_l2c_cbcx_iocerr_cn73xx    cnf75xx;
};
typedef union cvmx_l2c_cbcx_iocerr cvmx_l2c_cbcx_iocerr_t;

/**
 * cvmx_l2c_cbc#_iodisocierr
 *
 * Reserved.
 *
 */
union cvmx_l2c_cbcx_iodisocierr {
	uint64_t u64;
	struct cvmx_l2c_cbcx_iodisocierr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t iorddisoci                   : 1;  /**< Reserved. */
	uint64_t iowrdisoci                   : 1;  /**< Reserved. */
	uint64_t reserved_59_61               : 3;
	uint64_t cmd                          : 7;  /**< Reserved. */
	uint64_t ppvid                        : 6;  /**< Reserved. */
	uint64_t node                         : 2;  /**< Reserved. */
	uint64_t did                          : 8;  /**< Reserved. */
	uint64_t addr                         : 36; /**< Reserved. */
#else
	uint64_t addr                         : 36;
	uint64_t did                          : 8;
	uint64_t node                         : 2;
	uint64_t ppvid                        : 6;
	uint64_t cmd                          : 7;
	uint64_t reserved_59_61               : 3;
	uint64_t iowrdisoci                   : 1;
	uint64_t iorddisoci                   : 1;
#endif
	} s;
	struct cvmx_l2c_cbcx_iodisocierr_s    cn73xx;
	struct cvmx_l2c_cbcx_iodisocierr_s    cn78xx;
	struct cvmx_l2c_cbcx_iodisocierr_s    cn78xxp1;
	struct cvmx_l2c_cbcx_iodisocierr_s    cnf75xx;
};
typedef union cvmx_l2c_cbcx_iodisocierr cvmx_l2c_cbcx_iodisocierr_t;

/**
 * cvmx_l2c_cbc#_miberr
 *
 * Reserved.
 *
 */
union cvmx_l2c_cbcx_miberr {
	uint64_t u64;
	struct cvmx_l2c_cbcx_miberr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t mibdbe                       : 1;  /**< Reserved. */
	uint64_t mibsbe                       : 1;  /**< Reserved. */
	uint64_t reserved_40_61               : 22;
	uint64_t syn                          : 8;  /**< Reserved. */
	uint64_t reserved_3_31                : 29;
	uint64_t memid                        : 2;  /**< Reserved. */
	uint64_t mibnum                       : 1;  /**< Reserved. */
#else
	uint64_t mibnum                       : 1;
	uint64_t memid                        : 2;
	uint64_t reserved_3_31                : 29;
	uint64_t syn                          : 8;
	uint64_t reserved_40_61               : 22;
	uint64_t mibsbe                       : 1;
	uint64_t mibdbe                       : 1;
#endif
	} s;
	struct cvmx_l2c_cbcx_miberr_s         cn73xx;
	struct cvmx_l2c_cbcx_miberr_s         cn78xx;
	struct cvmx_l2c_cbcx_miberr_s         cn78xxp1;
	struct cvmx_l2c_cbcx_miberr_s         cnf75xx;
};
typedef union cvmx_l2c_cbcx_miberr cvmx_l2c_cbcx_miberr_t;

/**
 * cvmx_l2c_cbc#_rsderr
 *
 * This register records error information for all CBC RSD errors. An error locks the
 * [INDEX] and [SYN] and sets the bit corresponding to the error received. RSDDBE
 * errors take priority and overwrite an earlier logged RSDSBE error. Only one of
 * [RSDSBE]/[RSDDBE] is set at any given time and serves to document which error the
 * INDEX/[SYN] is associated with. The syndrome is recorded for DBE errors, though the
 * utility of the value is not clear.
 */
union cvmx_l2c_cbcx_rsderr {
	uint64_t u64;
	struct cvmx_l2c_cbcx_rsderr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rsddbe                       : 1;  /**< INDEX/SYN corresponds to a double-bit RSD ECC error */
	uint64_t rsdsbe                       : 1;  /**< INDEX/SYN corresponds to a single-bit RSD ECC error */
	uint64_t reserved_40_61               : 22;
	uint64_t syn                          : 8;  /**< Error syndrome. */
	uint64_t reserved_9_31                : 23;
	uint64_t tadnum                       : 3;  /**< Indicates the TAD FIFO containing the error. */
	uint64_t qwnum                        : 2;  /**< Indicates the QW containing the error. */
	uint64_t rsdnum                       : 4;  /**< Indicates the RSD that had the error. */
#else
	uint64_t rsdnum                       : 4;
	uint64_t qwnum                        : 2;
	uint64_t tadnum                       : 3;
	uint64_t reserved_9_31                : 23;
	uint64_t syn                          : 8;
	uint64_t reserved_40_61               : 22;
	uint64_t rsdsbe                       : 1;
	uint64_t rsddbe                       : 1;
#endif
	} s;
	struct cvmx_l2c_cbcx_rsderr_s         cn70xx;
	struct cvmx_l2c_cbcx_rsderr_s         cn70xxp1;
	struct cvmx_l2c_cbcx_rsderr_s         cn73xx;
	struct cvmx_l2c_cbcx_rsderr_s         cn78xx;
	struct cvmx_l2c_cbcx_rsderr_s         cn78xxp1;
	struct cvmx_l2c_cbcx_rsderr_s         cnf75xx;
};
typedef union cvmx_l2c_cbcx_rsderr cvmx_l2c_cbcx_rsderr_t;

/**
 * cvmx_l2c_cfg
 *
 * Specify the RSL base addresses for the block
 *
 *                  L2C_CFG = L2C Configuration
 *
 * Description:
 */
union cvmx_l2c_cfg {
	uint64_t u64;
	struct cvmx_l2c_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bstrun                       : 1;  /**< L2 Data Store Bist Running
                                                         Indicates when the L2C HW Bist sequence(short or long) is
                                                         running. [L2C ECC Bist FSM is not in the RESET/DONE state] */
	uint64_t lbist                        : 1;  /**< L2C Data Store Long Bist Sequence
                                                         When the previous state was '0' and SW writes a '1',
                                                         the long bist sequence (enhanced 13N March) is performed.
                                                         SW can then read the L2C_CFG[BSTRUN] which will indicate
                                                         that the long bist sequence is running. When BSTRUN-=0,
                                                         the state of the L2D_BST[0-3] registers contain information
                                                         which reflects the status of the recent long bist sequence.
                                                         NOTE: SW must never write LBIST=0 while Long Bist is running
                                                         (ie: when BSTRUN=1 never write LBIST=0).
                                                         NOTE: LBIST is disabled if the MIO_FUS_DAT2.BIST_DIS
                                                         Fuse is blown. */
	uint64_t xor_bank                     : 1;  /**< L2C XOR Bank Bit
                                                         When both LMC's are enabled(DPRES1=1/DPRES0=1), this
                                                         bit determines how addresses are assigned to
                                                         LMC port(s).
                                                            XOR_BANK|  LMC#
                                                          ----------+---------------------------------
                                                              0     |   byte address[7]
                                                              1     |   byte address[7] XOR byte address[12]
                                                         Example: If both LMC ports are enabled (DPRES1=1/DPRES0=1)
                                                         and XOR_BANK=1, then addr[7] XOR addr[12] is used to determine
                                                         which LMC Port# a reference is directed to. */
	uint64_t dpres1                       : 1;  /**< DDR1 Present/LMC1 Enable
                                                         When DPRES1 is set, LMC#1 is enabled(DDR1 pins at
                                                         the BOTTOM of the chip are active).
                                                         NOTE: When both LMC ports are enabled(DPRES1=1/DPRES0=1),
                                                         see XOR_BANK bit to determine how a reference is
                                                         assigned to a DDR/LMC port. (Also, in dual-LMC configuration,
                                                         the address sent to the targeted LMC port is the
                                                         address shifted right by one).
                                                         NOTE: For power-savings, the DPRES1 is also used to
                                                         disable DDR1/LMC1 clocks. */
	uint64_t dpres0                       : 1;  /**< DDR0 Present/LMC0 Enable
                                                         When DPRES0 is set, LMC#0 is enabled(DDR0 pins at
                                                         the BOTTOM of the chip are active).
                                                         NOTE: When both LMC ports are enabled(DPRES1=1/DPRES0=1),
                                                         see XOR_BANK bit to determine how a reference is
                                                         assigned to a DDR/LMC port. (Also, in dual-LMC configuration,
                                                         the address sent to the targeted LMC port is the
                                                         address shifted right by one).
                                                         NOTE: For power-savings, the DPRES0 is also used to
                                                         disable DDR0/LMC0 clocks. */
	uint64_t dfill_dis                    : 1;  /**< L2C Dual Fill Disable
                                                         When set, the L2C dual-fill performance feature is
                                                         disabled.
                                                         NOTE: This bit is only intended to evaluate the
                                                         effectiveness of the dual-fill feature. For OPTIMAL
                                                         performance, this bit should ALWAYS be zero. */
	uint64_t fpexp                        : 4;  /**< [CYA] Forward Progress Counter Exponent
                                                         NOTE: Should NOT be exposed to customer! [FOR DEBUG ONLY]
                                                         When FPEN is enabled and the LFB is empty, the
                                                         forward progress counter (FPCNT) is initialized to:
                                                            FPCNT[24:0] = 2^(9+FPEXP)
                                                         When the LFB is non-empty the FPCNT is decremented
                                                         (every eclk interval). If the FPCNT reaches zero,
                                                         the LFB no longer accepts new requests until either
                                                            a) all of the current LFB entries have completed
                                                               (to ensure forward progress).
                                                            b) FPEMPTY=0 and another forward progress count
                                                               interval timeout expires.
                                                         EXAMPLE USE: If FPEXP=2, the FPCNT = 2048 eclks.
                                                         (For eclk=500MHz(2ns), this would be ~4us). */
	uint64_t fpempty                      : 1;  /**< [CYA] Forward Progress Counter Empty
                                                         NOTE: Should NOT be exposed to customer! [FOR DEBUG ONLY]
                                                         When set, if the forward progress counter expires,
                                                         all new LFB-NQs are stopped UNTIL all current LFB
                                                         entries have completed.
                                                         When clear, if the forward progress counter expires,
                                                         all new LFB-NQs are stopped UNTIL either
                                                           a) all current LFB entries have completed.
                                                           b) another forward progress interval expires
                                                         NOTE: We may want to FREEZE/HANG the system when
                                                         we encounter an LFB entry cannot complete, and there
                                                         may be times when we want to allow further LFB-NQs
                                                         to be permitted to help in further analyzing the
                                                         source */
	uint64_t fpen                         : 1;  /**< [CYA] Forward Progress Counter Enable
                                                         NOTE: Should NOT be exposed to customer! [FOR DEBUG ONLY]
                                                         When set, enables the Forward Progress Counter to
                                                         prevent new LFB entries from enqueueing until ALL
                                                         current LFB entries have completed. */
	uint64_t idxalias                     : 1;  /**< L2C Index Alias Enable
                                                         When set, the L2 Tag/Data Store will alias the 11-bit
                                                         index with the low order 11-bits of the tag.
                                                            index[17:7] =  (tag[28:18] ^ index[17:7])
                                                         NOTE: This bit must only be modified at boot time,
                                                         when it can be guaranteed that no blocks have been
                                                         loaded into the L2 Cache.
                                                         The index aliasing is a performance enhancement feature
                                                         which reduces the L2 cache thrashing experienced for
                                                         regular stride references.
                                                         NOTE: The index alias is stored in the LFB and VAB, and
                                                         its effects are reversed for memory references (Victims,
                                                         STT-Misses and Read-Misses) */
	uint64_t mwf_crd                      : 4;  /**< MWF Credit Threshold: When the remaining MWF credits
                                                         become less than or equal to the MWF_CRD, the L2C will
                                                         assert l2c__lmi_mwd_hiwater_a to signal the LMC to give
                                                         writes (victims) higher priority. */
	uint64_t rsp_arb_mode                 : 1;  /**< RSP Arbitration Mode:
                                                         - 0: Fixed Priority [HP=RFB, RMCF, RHCF, STRSP, LP=STRSC]
                                                         - 1: Round Robin: [RFB(reflected I/O), RMCF(RdMiss),
                                                             RHCF(RdHit), STRSP(ST RSP w/ invalidate),
                                                             STRSC(ST RSP no invalidate)] */
	uint64_t rfb_arb_mode                 : 1;  /**< RFB Arbitration Mode:
                                                         - 0: Fixed Priority -
                                                             IOB->PP requests are higher priority than
                                                             PP->IOB requests
                                                         - 1: Round Robin -
                                                             I/O requests from PP and IOB are serviced in
                                                             round robin */
	uint64_t lrf_arb_mode                 : 1;  /**< RF Arbitration Mode:
                                                         - 0: Fixed Priority -
                                                             IOB memory requests are higher priority than PP
                                                             memory requests.
                                                         - 1: Round Robin -
                                                             Memory requests from PP and IOB are serviced in
                                                             round robin. */
#else
	uint64_t lrf_arb_mode                 : 1;
	uint64_t rfb_arb_mode                 : 1;
	uint64_t rsp_arb_mode                 : 1;
	uint64_t mwf_crd                      : 4;
	uint64_t idxalias                     : 1;
	uint64_t fpen                         : 1;
	uint64_t fpempty                      : 1;
	uint64_t fpexp                        : 4;
	uint64_t dfill_dis                    : 1;
	uint64_t dpres0                       : 1;
	uint64_t dpres1                       : 1;
	uint64_t xor_bank                     : 1;
	uint64_t lbist                        : 1;
	uint64_t bstrun                       : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_l2c_cfg_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t fpexp                        : 4;  /**< [CYA] Forward Progress Counter Exponent
                                                         NOTE: Should NOT be exposed to customer! [FOR DEBUG ONLY]
                                                         When FPEN is enabled and the LFB is empty, the
                                                         forward progress counter (FPCNT) is initialized to:
                                                            FPCNT[24:0] = 2^(9+FPEXP)
                                                         When the LFB is non-empty the FPCNT is decremented
                                                         (every eclk interval). If the FPCNT reaches zero,
                                                         the LFB no longer accepts new requests until either
                                                            a) all of the current LFB entries have completed
                                                               (to ensure forward progress).
                                                            b) FPEMPTY=0 and another forward progress count
                                                               interval timeout expires.
                                                         EXAMPLE USE: If FPEXP=2, the FPCNT = 2048 eclks.
                                                         (For eclk=500MHz(2ns), this would be ~4us). */
	uint64_t fpempty                      : 1;  /**< [CYA] Forward Progress Counter Empty
                                                         NOTE: Should NOT be exposed to customer! [FOR DEBUG ONLY]
                                                         When set, if the forward progress counter expires,
                                                         all new LFB-NQs are stopped UNTIL all current LFB
                                                         entries have completed.
                                                         When clear, if the forward progress counter expires,
                                                         all new LFB-NQs are stopped UNTIL either
                                                           a) all current LFB entries have completed.
                                                           b) another forward progress interval expires
                                                         NOTE: We may want to FREEZE/HANG the system when
                                                         we encounter an LFB entry cannot complete, and there
                                                         may be times when we want to allow further LFB-NQs
                                                         to be permitted to help in further analyzing the
                                                         source */
	uint64_t fpen                         : 1;  /**< [CYA] Forward Progress Counter Enable
                                                         NOTE: Should NOT be exposed to customer! [FOR DEBUG ONLY]
                                                         When set, enables the Forward Progress Counter to
                                                         prevent new LFB entries from enqueueing until ALL
                                                         current LFB entries have completed. */
	uint64_t idxalias                     : 1;  /**< L2C Index Alias Enable
                                                         When set, the L2 Tag/Data Store will alias the 8-bit
                                                         index with the low order 8-bits of the tag.
                                                            index[14:7] =  (tag[22:15] ^ index[14:7])
                                                         NOTE: This bit must only be modified at boot time,
                                                         when it can be guaranteed that no blocks have been
                                                         loaded into the L2 Cache.
                                                         The index aliasing is a performance enhancement feature
                                                         which reduces the L2 cache thrashing experienced for
                                                         regular stride references.
                                                         NOTE: The index alias is stored in the LFB and VAB, and
                                                         its effects are reversed for memory references (Victims,
                                                         STT-Misses and Read-Misses) */
	uint64_t mwf_crd                      : 4;  /**< MWF Credit Threshold: When the remaining MWF credits
                                                         become less than or equal to the MWF_CRD, the L2C will
                                                         assert l2c__lmi_mwd_hiwater_a to signal the LMC to give
                                                         writes (victims) higher priority. */
	uint64_t rsp_arb_mode                 : 1;  /**< RSP Arbitration Mode:
                                                         - 0: Fixed Priority [HP=RFB, RMCF, RHCF, STRSP, LP=STRSC]
                                                         - 1: Round Robin: [RFB(reflected I/O), RMCF(RdMiss),
                                                             RHCF(RdHit), STRSP(ST RSP w/ invalidate),
                                                             STRSC(ST RSP no invalidate)] */
	uint64_t rfb_arb_mode                 : 1;  /**< RFB Arbitration Mode:
                                                         - 0: Fixed Priority -
                                                             IOB->PP requests are higher priority than
                                                             PP->IOB requests
                                                         - 1: Round Robin -
                                                             I/O requests from PP and IOB are serviced in
                                                             round robin */
	uint64_t lrf_arb_mode                 : 1;  /**< RF Arbitration Mode:
                                                         - 0: Fixed Priority -
                                                             IOB memory requests are higher priority than PP
                                                             memory requests.
                                                         - 1: Round Robin -
                                                             Memory requests from PP and IOB are serviced in
                                                             round robin. */
#else
	uint64_t lrf_arb_mode                 : 1;
	uint64_t rfb_arb_mode                 : 1;
	uint64_t rsp_arb_mode                 : 1;
	uint64_t mwf_crd                      : 4;
	uint64_t idxalias                     : 1;
	uint64_t fpen                         : 1;
	uint64_t fpempty                      : 1;
	uint64_t fpexp                        : 4;
	uint64_t reserved_14_63               : 50;
#endif
	} cn30xx;
	struct cvmx_l2c_cfg_cn30xx            cn31xx;
	struct cvmx_l2c_cfg_cn30xx            cn38xx;
	struct cvmx_l2c_cfg_cn30xx            cn38xxp2;
	struct cvmx_l2c_cfg_cn50xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bstrun                       : 1;  /**< L2 Data Store Bist Running
                                                         Indicates when the L2C HW Bist sequence(short or long) is
                                                         running. [L2C ECC Bist FSM is not in the RESET/DONE state] */
	uint64_t lbist                        : 1;  /**< L2C Data Store Long Bist Sequence
                                                         When the previous state was '0' and SW writes a '1',
                                                         the long bist sequence (enhanced 13N March) is performed.
                                                         SW can then read the L2C_CFG[BSTRUN] which will indicate
                                                         that the long bist sequence is running. When BSTRUN-=0,
                                                         the state of the L2D_BST[0-3] registers contain information
                                                         which reflects the status of the recent long bist sequence.
                                                         NOTE: SW must never write LBIST=0 while Long Bist is running
                                                         (ie: when BSTRUN=1 never write LBIST=0). */
	uint64_t reserved_14_17               : 4;
	uint64_t fpexp                        : 4;  /**< [CYA] Forward Progress Counter Exponent
                                                         NOTE: Should NOT be exposed to customer! [FOR DEBUG ONLY]
                                                         When FPEN is enabled and the LFB is empty, the
                                                         forward progress counter (FPCNT) is initialized to:
                                                            FPCNT[24:0] = 2^(9+FPEXP)
                                                         When the LFB is non-empty the FPCNT is decremented
                                                         (every eclk interval). If the FPCNT reaches zero,
                                                         the LFB no longer accepts new requests until either
                                                            a) all of the current LFB entries have completed
                                                               (to ensure forward progress).
                                                            b) FPEMPTY=0 and another forward progress count
                                                               interval timeout expires.
                                                         EXAMPLE USE: If FPEXP=2, the FPCNT = 2048 eclks.
                                                         (For eclk=500MHz(2ns), this would be ~4us). */
	uint64_t fpempty                      : 1;  /**< [CYA] Forward Progress Counter Empty
                                                         NOTE: Should NOT be exposed to customer! [FOR DEBUG ONLY]
                                                         When set, if the forward progress counter expires,
                                                         all new LFB-NQs are stopped UNTIL all current LFB
                                                         entries have completed.
                                                         When clear, if the forward progress counter expires,
                                                         all new LFB-NQs are stopped UNTIL either
                                                           a) all current LFB entries have completed.
                                                           b) another forward progress interval expires
                                                         NOTE: We may want to FREEZE/HANG the system when
                                                         we encounter an LFB entry cannot complete, and there
                                                         may be times when we want to allow further LFB-NQs
                                                         to be permitted to help in further analyzing the
                                                         source */
	uint64_t fpen                         : 1;  /**< [CYA] Forward Progress Counter Enable
                                                         NOTE: Should NOT be exposed to customer! [FOR DEBUG ONLY]
                                                         When set, enables the Forward Progress Counter to
                                                         prevent new LFB entries from enqueueing until ALL
                                                         current LFB entries have completed. */
	uint64_t idxalias                     : 1;  /**< L2C Index Alias Enable
                                                         When set, the L2 Tag/Data Store will alias the 7-bit
                                                         index with the low order 7-bits of the tag.
                                                            index[13:7] =  (tag[20:14] ^ index[13:7])
                                                         NOTE: This bit must only be modified at boot time,
                                                         when it can be guaranteed that no blocks have been
                                                         loaded into the L2 Cache.
                                                         The index aliasing is a performance enhancement feature
                                                         which reduces the L2 cache thrashing experienced for
                                                         regular stride references.
                                                         NOTE: The index alias is stored in the LFB and VAB, and
                                                         its effects are reversed for memory references (Victims,
                                                         STT-Misses and Read-Misses) */
	uint64_t mwf_crd                      : 4;  /**< MWF Credit Threshold: When the remaining MWF credits
                                                         become less than or equal to the MWF_CRD, the L2C will
                                                         assert l2c__lmi_mwd_hiwater_a to signal the LMC to give
                                                         writes (victims) higher priority. */
	uint64_t rsp_arb_mode                 : 1;  /**< RSP Arbitration Mode:
                                                         - 0: Fixed Priority [HP=RFB, RMCF, RHCF, STRSP, LP=STRSC]
                                                         - 1: Round Robin: [RFB(reflected I/O), RMCF(RdMiss),
                                                             RHCF(RdHit), STRSP(ST RSP w/ invalidate),
                                                             STRSC(ST RSP no invalidate)] */
	uint64_t rfb_arb_mode                 : 1;  /**< RFB Arbitration Mode:
                                                         - 0: Fixed Priority -
                                                             IOB->PP requests are higher priority than
                                                             PP->IOB requests
                                                         - 1: Round Robin -
                                                             I/O requests from PP and IOB are serviced in
                                                             round robin */
	uint64_t lrf_arb_mode                 : 1;  /**< RF Arbitration Mode:
                                                         - 0: Fixed Priority -
                                                             IOB memory requests are higher priority than PP
                                                             memory requests.
                                                         - 1: Round Robin -
                                                             Memory requests from PP and IOB are serviced in
                                                             round robin. */
#else
	uint64_t lrf_arb_mode                 : 1;
	uint64_t rfb_arb_mode                 : 1;
	uint64_t rsp_arb_mode                 : 1;
	uint64_t mwf_crd                      : 4;
	uint64_t idxalias                     : 1;
	uint64_t fpen                         : 1;
	uint64_t fpempty                      : 1;
	uint64_t fpexp                        : 4;
	uint64_t reserved_14_17               : 4;
	uint64_t lbist                        : 1;
	uint64_t bstrun                       : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} cn50xx;
	struct cvmx_l2c_cfg_cn50xx            cn52xx;
	struct cvmx_l2c_cfg_cn50xx            cn52xxp1;
	struct cvmx_l2c_cfg_s                 cn56xx;
	struct cvmx_l2c_cfg_s                 cn56xxp1;
	struct cvmx_l2c_cfg_cn58xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t bstrun                       : 1;  /**< L2 Data Store Bist Running
                                                         Indicates when the L2C HW Bist sequence(short or long) is
                                                         running. [L2C ECC Bist FSM is not in the RESET/DONE state] */
	uint64_t lbist                        : 1;  /**< L2C Data Store Long Bist Sequence
                                                         When the previous state was '0' and SW writes a '1',
                                                         the long bist sequence (enhanced 13N March) is performed.
                                                         SW can then read the L2C_CFG[BSTRUN] which will indicate
                                                         that the long bist sequence is running. When BSTRUN-=0,
                                                         the state of the L2D_BST[0-3] registers contain information
                                                         which reflects the status of the recent long bist sequence.
                                                         NOTE: SW must never write LBIST=0 while Long Bist is running
                                                         (ie: when BSTRUN=1 never write LBIST=0).
                                                         NOTE: LBIST is disabled if the MIO_FUS_DAT2.BIST_DIS
                                                         Fuse is blown. */
	uint64_t reserved_15_17               : 3;
	uint64_t dfill_dis                    : 1;  /**< L2C Dual Fill Disable
                                                         When set, the L2C dual-fill performance feature is
                                                         disabled.
                                                         NOTE: This bit is only intended to evaluate the
                                                         effectiveness of the dual-fill feature. For OPTIMAL
                                                         performance, this bit should ALWAYS be zero. */
	uint64_t fpexp                        : 4;  /**< [CYA] Forward Progress Counter Exponent
                                                         NOTE: Should NOT be exposed to customer! [FOR DEBUG ONLY]
                                                         When FPEN is enabled and the LFB is empty, the
                                                         forward progress counter (FPCNT) is initialized to:
                                                            FPCNT[24:0] = 2^(9+FPEXP)
                                                         When the LFB is non-empty the FPCNT is decremented
                                                         (every eclk interval). If the FPCNT reaches zero,
                                                         the LFB no longer accepts new requests until either
                                                            a) all of the current LFB entries have completed
                                                               (to ensure forward progress).
                                                            b) FPEMPTY=0 and another forward progress count
                                                               interval timeout expires.
                                                         EXAMPLE USE: If FPEXP=2, the FPCNT = 2048 eclks.
                                                         (For eclk=500MHz(2ns), this would be ~4us). */
	uint64_t fpempty                      : 1;  /**< [CYA] Forward Progress Counter Empty
                                                         NOTE: Should NOT be exposed to customer! [FOR DEBUG ONLY]
                                                         When set, if the forward progress counter expires,
                                                         all new LFB-NQs are stopped UNTIL all current LFB
                                                         entries have completed.
                                                         When clear, if the forward progress counter expires,
                                                         all new LFB-NQs are stopped UNTIL either
                                                           a) all current LFB entries have completed.
                                                           b) another forward progress interval expires
                                                         NOTE: We may want to FREEZE/HANG the system when
                                                         we encounter an LFB entry cannot complete, and there
                                                         may be times when we want to allow further LFB-NQs
                                                         to be permitted to help in further analyzing the
                                                         source */
	uint64_t fpen                         : 1;  /**< [CYA] Forward Progress Counter Enable
                                                         NOTE: Should NOT be exposed to customer! [FOR DEBUG ONLY]
                                                         When set, enables the Forward Progress Counter to
                                                         prevent new LFB entries from enqueueing until ALL
                                                         current LFB entries have completed. */
	uint64_t idxalias                     : 1;  /**< L2C Index Alias Enable
                                                         When set, the L2 Tag/Data Store will alias the 11-bit
                                                         index with the low order 11-bits of the tag.
                                                            index[17:7] =  (tag[28:18] ^ index[17:7])
                                                         NOTE: This bit must only be modified at boot time,
                                                         when it can be guaranteed that no blocks have been
                                                         loaded into the L2 Cache.
                                                         The index aliasing is a performance enhancement feature
                                                         which reduces the L2 cache thrashing experienced for
                                                         regular stride references.
                                                         NOTE: The index alias is stored in the LFB and VAB, and
                                                         its effects are reversed for memory references (Victims,
                                                         STT-Misses and Read-Misses) */
	uint64_t mwf_crd                      : 4;  /**< MWF Credit Threshold: When the remaining MWF credits
                                                         become less than or equal to the MWF_CRD, the L2C will
                                                         assert l2c__lmi_mwd_hiwater_a to signal the LMC to give
                                                         writes (victims) higher priority. */
	uint64_t rsp_arb_mode                 : 1;  /**< RSP Arbitration Mode:
                                                         - 0: Fixed Priority [HP=RFB, RMCF, RHCF, STRSP, LP=STRSC]
                                                         - 1: Round Robin: [RFB(reflected I/O), RMCF(RdMiss),
                                                             RHCF(RdHit), STRSP(ST RSP w/ invalidate),
                                                             STRSC(ST RSP no invalidate)] */
	uint64_t rfb_arb_mode                 : 1;  /**< RFB Arbitration Mode:
                                                         - 0: Fixed Priority -
                                                             IOB->PP requests are higher priority than
                                                             PP->IOB requests
                                                         - 1: Round Robin -
                                                             I/O requests from PP and IOB are serviced in
                                                             round robin */
	uint64_t lrf_arb_mode                 : 1;  /**< RF Arbitration Mode:
                                                         - 0: Fixed Priority -
                                                             IOB memory requests are higher priority than PP
                                                             memory requests.
                                                         - 1: Round Robin -
                                                             Memory requests from PP and IOB are serviced in
                                                             round robin. */
#else
	uint64_t lrf_arb_mode                 : 1;
	uint64_t rfb_arb_mode                 : 1;
	uint64_t rsp_arb_mode                 : 1;
	uint64_t mwf_crd                      : 4;
	uint64_t idxalias                     : 1;
	uint64_t fpen                         : 1;
	uint64_t fpempty                      : 1;
	uint64_t fpexp                        : 4;
	uint64_t dfill_dis                    : 1;
	uint64_t reserved_15_17               : 3;
	uint64_t lbist                        : 1;
	uint64_t bstrun                       : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} cn58xx;
	struct cvmx_l2c_cfg_cn58xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_15_63               : 49;
	uint64_t dfill_dis                    : 1;  /**< L2C Dual Fill Disable
                                                         When set, the L2C dual-fill performance feature is
                                                         disabled.
                                                         NOTE: This bit is only intended to evaluate the
                                                         effectiveness of the dual-fill feature. For OPTIMAL
                                                         performance, this bit should ALWAYS be zero. */
	uint64_t fpexp                        : 4;  /**< [CYA] Forward Progress Counter Exponent
                                                         NOTE: Should NOT be exposed to customer! [FOR DEBUG ONLY]
                                                         When FPEN is enabled and the LFB is empty, the
                                                         forward progress counter (FPCNT) is initialized to:
                                                            FPCNT[24:0] = 2^(9+FPEXP)
                                                         When the LFB is non-empty the FPCNT is decremented
                                                         (every eclk interval). If the FPCNT reaches zero,
                                                         the LFB no longer accepts new requests until either
                                                            a) all of the current LFB entries have completed
                                                               (to ensure forward progress).
                                                            b) FPEMPTY=0 and another forward progress count
                                                               interval timeout expires.
                                                         EXAMPLE USE: If FPEXP=2, the FPCNT = 2048 eclks.
                                                         (For eclk=500MHz(2ns), this would be ~4us). */
	uint64_t fpempty                      : 1;  /**< [CYA] Forward Progress Counter Empty
                                                         NOTE: Should NOT be exposed to customer! [FOR DEBUG ONLY]
                                                         When set, if the forward progress counter expires,
                                                         all new LFB-NQs are stopped UNTIL all current LFB
                                                         entries have completed.
                                                         When clear, if the forward progress counter expires,
                                                         all new LFB-NQs are stopped UNTIL either
                                                           a) all current LFB entries have completed.
                                                           b) another forward progress interval expires
                                                         NOTE: We may want to FREEZE/HANG the system when
                                                         we encounter an LFB entry cannot complete, and there
                                                         may be times when we want to allow further LFB-NQs
                                                         to be permitted to help in further analyzing the
                                                         source */
	uint64_t fpen                         : 1;  /**< [CYA] Forward Progress Counter Enable
                                                         NOTE: Should NOT be exposed to customer! [FOR DEBUG ONLY]
                                                         When set, enables the Forward Progress Counter to
                                                         prevent new LFB entries from enqueueing until ALL
                                                         current LFB entries have completed. */
	uint64_t idxalias                     : 1;  /**< L2C Index Alias Enable
                                                         When set, the L2 Tag/Data Store will alias the 11-bit
                                                         index with the low order 11-bits of the tag.
                                                            index[17:7] =  (tag[28:18] ^ index[17:7])
                                                         NOTE: This bit must only be modified at boot time,
                                                         when it can be guaranteed that no blocks have been
                                                         loaded into the L2 Cache.
                                                         The index aliasing is a performance enhancement feature
                                                         which reduces the L2 cache thrashing experienced for
                                                         regular stride references.
                                                         NOTE: The index alias is stored in the LFB and VAB, and
                                                         its effects are reversed for memory references (Victims,
                                                         STT-Misses and Read-Misses) */
	uint64_t mwf_crd                      : 4;  /**< MWF Credit Threshold: When the remaining MWF credits
                                                         become less than or equal to the MWF_CRD, the L2C will
                                                         assert l2c__lmi_mwd_hiwater_a to signal the LMC to give
                                                         writes (victims) higher priority. */
	uint64_t rsp_arb_mode                 : 1;  /**< RSP Arbitration Mode:
                                                         - 0: Fixed Priority [HP=RFB, RMCF, RHCF, STRSP, LP=STRSC]
                                                         - 1: Round Robin: [RFB(reflected I/O), RMCF(RdMiss),
                                                             RHCF(RdHit), STRSP(ST RSP w/ invalidate),
                                                             STRSC(ST RSP no invalidate)] */
	uint64_t rfb_arb_mode                 : 1;  /**< RFB Arbitration Mode:
                                                         - 0: Fixed Priority -
                                                             IOB->PP requests are higher priority than
                                                             PP->IOB requests
                                                         - 1: Round Robin -
                                                             I/O requests from PP and IOB are serviced in
                                                             round robin */
	uint64_t lrf_arb_mode                 : 1;  /**< RF Arbitration Mode:
                                                         - 0: Fixed Priority -
                                                             IOB memory requests are higher priority than PP
                                                             memory requests.
                                                         - 1: Round Robin -
                                                             Memory requests from PP and IOB are serviced in
                                                             round robin. */
#else
	uint64_t lrf_arb_mode                 : 1;
	uint64_t rfb_arb_mode                 : 1;
	uint64_t rsp_arb_mode                 : 1;
	uint64_t mwf_crd                      : 4;
	uint64_t idxalias                     : 1;
	uint64_t fpen                         : 1;
	uint64_t fpempty                      : 1;
	uint64_t fpexp                        : 4;
	uint64_t dfill_dis                    : 1;
	uint64_t reserved_15_63               : 49;
#endif
	} cn58xxp1;
};
typedef union cvmx_l2c_cfg cvmx_l2c_cfg_t;

/**
 * cvmx_l2c_cop0_adr
 *
 * Provides the address of the COP0 register to read/write when L2C_COP0_DAT is accessed.
 *
 * 1. RD and SEL are as defined in the description of core coprocessor 0 registers.
 *
 * 2. If the PPID is outside the range of valid cores, and not 255 (broadcast), or if
 * the core in question is in reset a write will be ignored and reads will timeout the
 * RSL bus.
 *
 * 3. If a COP0 register cannot be accessed by this mechanism the write be silently
 * ignored and the read data will be 0x2bad2bad2bad2bad. Otherwise, if the COP0
 * register doesn't exist, the read data value will be 0x000000000baddeed.
 */
union cvmx_l2c_cop0_adr {
	uint64_t u64;
	struct cvmx_l2c_cop0_adr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t ppid                         : 8;  /**< Core to access; use 0xFF for broadcast write. Broadcast reads are unpredictable. */
	uint64_t reserved_15_15               : 1;
	uint64_t mbz                          : 6;  /**< Must be written to zero. */
	uint64_t root                         : 1;  /**< If 1, root register is accessed, if 0 guest register is accessed. */
	uint64_t rd                           : 5;  /**< COP0 register number. */
	uint64_t sel                          : 3;  /**< COP0 select. */
#else
	uint64_t sel                          : 3;
	uint64_t rd                           : 5;
	uint64_t root                         : 1;
	uint64_t mbz                          : 6;
	uint64_t reserved_15_15               : 1;
	uint64_t ppid                         : 8;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_l2c_cop0_adr_s            cn70xx;
	struct cvmx_l2c_cop0_adr_s            cn70xxp1;
	struct cvmx_l2c_cop0_adr_s            cn73xx;
	struct cvmx_l2c_cop0_adr_s            cn78xx;
	struct cvmx_l2c_cop0_adr_s            cn78xxp1;
	struct cvmx_l2c_cop0_adr_s            cnf75xx;
};
typedef union cvmx_l2c_cop0_adr cvmx_l2c_cop0_adr_t;

/**
 * cvmx_l2c_cop0_dat
 *
 * Provides data access for the COP0 register specified by the L2C_COP0_ADR register.
 *
 */
union cvmx_l2c_cop0_dat {
	uint64_t u64;
	struct cvmx_l2c_cop0_dat_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Data to write to specified COP0 register or return data for a read. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_l2c_cop0_dat_s            cn70xx;
	struct cvmx_l2c_cop0_dat_s            cn70xxp1;
	struct cvmx_l2c_cop0_dat_s            cn73xx;
	struct cvmx_l2c_cop0_dat_s            cn78xx;
	struct cvmx_l2c_cop0_dat_s            cn78xxp1;
	struct cvmx_l2c_cop0_dat_s            cnf75xx;
};
typedef union cvmx_l2c_cop0_dat cvmx_l2c_cop0_dat_t;

/**
 * cvmx_l2c_cop0_map#
 *
 * L2C_COP0_MAP = PP COP0 register memory mapped region
 *
 * Description: PP COP0 register mapped region.
 *
 * NOTE: for 63xx, if the PPID is outside the range of 0-3,63 the write will be ignored and reads
 * will return 0x2bad2bad2bad2bad
 *
 * Notes:
 * (1) There are 256 COP0 registers per PP.  Registers 0-255 map to PP0's COP0 registers, 256-511 are
 *     mapped to PP1's, etc.  A special set X PP63 (registers 16128-16383) are for broadcast writes.
 *     Any write done to these registers will take effect in ALL PPs.  Note the means the L2C_COP0_MAP
 *     register to access can be gotten by:
 *
 *         REGNUM = [ PPID[5:0], rd[4:0], sel[2:0] ]
 *
 *     where rd and sel are as defined in the HRM description of Core Coprocessor 0 registers
 *     and note 4 below.
 *
 * (2) if a COP0 register cannot be accessed by this mechanism the write be silently ignored and the
 *     read data will be 0xBADDEED.
 *
 * (3) for 61xx, if the PPID is outside the range of 0-3,63 or if the PP in question is in reset a
 *     write will be ignored and reads will timeout the RSL bus.
 *
 * (4) Referring to note (1) above, the following rd/sel values are supported:
 *
 *     NOTE: Put only the "Customer type" in HRM. do not put the "Real type" in HRM.
 *
 *                    Customer                                                    Real
 *        rd     sel     type         Description                                 type
 *     ======+=======+==========+==============================================+=========
 *        4      2       RO          COP0 UserLocal                                RW
 *        7      0       RO          COP0 HWREna                                   RW
 *        9      0       RO          COP0 Count                                    RW
 *        9      6       RO          COP0 CvmCount                                 RW
 *        9      7       RO          COP0 CvmCtl                                   RW
 *       11      0       RO          COP0 Compare                                  RW
 *       11      6       RW          COP0 PowThrottle                              RW
 *       12      0       RO          COP0 Status                                   RW
 *       12      1       RO          COP0 IntCtl                                   RO
 *       12      2       RO          COP0 SRSCtl                                   RO
 *       13      0       RO          COP0 Cause                                    RW
 *       14      0       RO          COP0 EPC                                      RW
 *       15      0       RO          COP0 PrID                                     RO
 *       15      1       RO          COP0 EBase                                    RW
 *       16      0       RO          PC Issue Debug Info (see details below)       RO
 *       16      1       RO          PC Fetch Debug Info (see details below)       RO
 *       16      2       RO          PC Fill Debug Info (see details below)        RO
 *       16      3       RO          PC Misc Debug Info (see details below)        RO
 *       18      0       RO          COP0 WatchLo0                                 RW
 *       19      0       RO          COP0 WatchHi0                                 RW
 *       22      0       RO          COP0 MultiCoreDebug                           RW
 *       22      1                   COP0 VoltageMonitor                           RW
 *       23      0       RO          COP0 Debug                                    RW
 *       23      6       RO          COP0 Debug2                                   RO
 *       24      0       RO          COP0 DEPC                                     RW
 *       25      0       RO          COP0 PerfCnt Control0                         RW
 *       25      1       RO          COP0 PerfCnt Counter0                         RW
 *       25      2       RO          COP0 PerfCnt Control1                         RW
 *       25      3       RO          COP0 PerfCnt Counter1                         RW
 *       27      0       RO          COP0 CacheErr (icache)                        RW
 *       28      0       RO          COP0 TagLo (icache)                           RW
 *       28      1       RO          COP0 DataLo (icache)                          RW
 *       29      1       RO          COP0 DataHi (icache)                          RW
 *       30      0       RO          COP0 ErrorEPC                                 RW
 *       31      0       RO          COP0 DESAVE                                   RW
 *       31      2       RO          COP0 Scratch                                  RW
 *       31      3       RO          COP0 Scratch1                                 RW
 *       31      4       RO          COP0 Scratch2                                 RW
 *
 *     - PC Issue Debug Info
 *
 *       - 63:2 pc0_5a<63:2> // often VA<63:2> of the next instruction to issue
 *                           //    but can also be the VA of an instruction executing/replaying on pipe 0
 *                           //    or can also be a VA being filled into the instruction cache
 *                           //    or can also be unpredictable
 *                           // <61:49> RAZ
 *       1    illegal      // set when illegal VA
 *       0    delayslot    // set when VA is delayslot (prior branch may be either taken or not taken)
 *
 *     - PC Fetch Debug Info
 *
 *       - 63:0 fetch_address_3a // VA being fetched from the instruction cache
 *                               // <61:49>, <1:0> RAZ
 *
 *     - PC Fill Debug Info
 *
 *       - 63:0 fill_address_4a<63:2> // VA<63:2> being filled into instruction cache
 *                                    // valid when waiting_for_ifill_4a is set (see PC Misc Debug Info below)
 *                                    // <61:49> RAZ
 *          1 illegal               // set when illegal VA
 *          0 RAZ
 *
 *     - PC Misc Debug Info
 *
 *       - 63:3 RAZ
 *          2 mem_stall_3a         // stall term from L1 memory system
 *          1 waiting_for_pfill_4a // when waiting_for_ifill_4a is set, indicates whether instruction cache fill is due to a prefetch
 *          0 waiting_for_ifill_4a // set when there is an outstanding instruction cache fill
 */
union cvmx_l2c_cop0_mapx {
	uint64_t u64;
	struct cvmx_l2c_cop0_mapx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Data to write to/read from designated PP's COP0
                                                         register. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_l2c_cop0_mapx_s           cn61xx;
	struct cvmx_l2c_cop0_mapx_s           cn63xx;
	struct cvmx_l2c_cop0_mapx_s           cn63xxp1;
	struct cvmx_l2c_cop0_mapx_s           cn66xx;
	struct cvmx_l2c_cop0_mapx_s           cn68xx;
	struct cvmx_l2c_cop0_mapx_s           cn68xxp1;
	struct cvmx_l2c_cop0_mapx_s           cnf71xx;
};
typedef union cvmx_l2c_cop0_mapx cvmx_l2c_cop0_mapx_t;

/**
 * cvmx_l2c_ctl
 *
 * L2C_CTL = L2C Control
 *
 *
 * Notes:
 * (1) If MAXVAB is != 0, VAB_THRESH should be less than MAXVAB.
 *
 * (2) L2DFDBE and L2DFSBE allows software to generate L2DSBE, L2DDBE, VBFSBE, and VBFDBE errors for
 *     the purposes of testing error handling code.  When one (or both) of these bits are set a PL2
 *     which misses in the L2 will fill with the appropriate error in the first 2 OWs of the fill.
 *     Software can determine which OW pair gets the error by choosing the desired fill order
 *     (address<6:5>).  A PL2 which hits in the L2 will not inject any errors.  Therefore sending a
 *     WBIL2 prior to the PL2 is recommended to make a miss likely (if multiple processors are involved
 *     software must be careful to be sure no other processor or IO device can bring the block into the
 *     L2).
 *
 *     To generate a VBFSBE or VBFDBE, software must first get the cache block into the cache with an
 *     error using a PL2 which misses the L2.  Then a store partial to a portion of the cache block
 *     without the error must change the block to dirty.  Then, a subsequent WBL2/WBIL2/victim will
 *     trigger the VBFSBE/VBFDBE error.
 */
union cvmx_l2c_ctl {
	uint64_t u64;
	struct cvmx_l2c_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_29_63               : 35;
	uint64_t rdf_fast                     : 1;  /**< When 0, delay read data fifo from DCLK to RCLK by one
                                                         cycle.  Needed when DCLK:RCLK ratio > 3:1.  Should be
                                                         set before DDR traffic begins and only changed when
                                                         memory traffic is idle. */
	uint64_t disstgl2i                    : 1;  /**< Disable STGL2Is from changing the tags. */
	uint64_t l2dfsbe                      : 1;  /**< Force single bit ECC error on PL2 allocates (2) */
	uint64_t l2dfdbe                      : 1;  /**< Force double bit ECC error on PL2 allocates (2) */
	uint64_t discclk                      : 1;  /**< Disable conditional clocking in L2C PNR blocks. */
	uint64_t maxvab                       : 4;  /**< Maximum VABs in use at once
                                                         (0 means 16, 1-15 as expected) */
	uint64_t maxlfb                       : 4;  /**< Maximum LFBs in use at once
                                                         (0 means 16, 1-15 as expected) */
	uint64_t rsp_arb_mode                 : 1;  /**< Arbitration mode for RSC/RSD bus. 0 = round-robin; 1 = static priority.
                                                         1. IOR data.
                                                         2. STIN/FILLs.
                                                         3. STDN/SCDN/SCFL. */
	uint64_t xmc_arb_mode                 : 1;  /**< Arbitration mode for ADD bus QOS queues. 0 = fully determined through QOS, 1 = QOS0
                                                         highest priority; QOS 1-7 use normal mode. */
	uint64_t reserved_2_13                : 12;
	uint64_t disecc                       : 1;  /**< Tag and data ECC disable. */
	uint64_t disidxalias                  : 1;  /**< Index alias disable. */
#else
	uint64_t disidxalias                  : 1;
	uint64_t disecc                       : 1;
	uint64_t reserved_2_13                : 12;
	uint64_t xmc_arb_mode                 : 1;
	uint64_t rsp_arb_mode                 : 1;
	uint64_t maxlfb                       : 4;
	uint64_t maxvab                       : 4;
	uint64_t discclk                      : 1;
	uint64_t l2dfdbe                      : 1;
	uint64_t l2dfsbe                      : 1;
	uint64_t disstgl2i                    : 1;
	uint64_t rdf_fast                     : 1;
	uint64_t reserved_29_63               : 35;
#endif
	} s;
	struct cvmx_l2c_ctl_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_29_63               : 35;
	uint64_t rdf_fast                     : 1;  /**< When 0, delay read data fifo from DCLK to RCLK by one
                                                         cycle.  Needed when DCLK:RCLK ratio > 3:1.  Should be
                                                         set before DDR traffic begins and only changed when
                                                         memory traffic is idle. */
	uint64_t disstgl2i                    : 1;  /**< Disable STGL2I's from changing the tags */
	uint64_t l2dfsbe                      : 1;  /**< Force single bit ECC error on PL2 allocates (2) */
	uint64_t l2dfdbe                      : 1;  /**< Force double bit ECC error on PL2 allocates (2) */
	uint64_t discclk                      : 1;  /**< Disable conditional clocking in L2C PNR blocks */
	uint64_t maxvab                       : 4;  /**< Maximum VABs in use at once
                                                         (0 means 16, 1-15 as expected) */
	uint64_t maxlfb                       : 4;  /**< Maximum LFBs in use at once
                                                         (0 means 16, 1-15 as expected) */
	uint64_t rsp_arb_mode                 : 1;  /**< Arbitration mode for RSC/RSD bus
                                                         == 0, round-robin
                                                         == 1, static priority
                                                             1. IOR data
                                                             2. STIN/FILLs
                                                             3. STDN/SCDN/SCFL */
	uint64_t xmc_arb_mode                 : 1;  /**< Arbitration mode for XMC QOS queues
                                                         == 0, fully determined through QOS
                                                         == 1, QOS0 highest priority, QOS1-3 use normal mode */
	uint64_t ef_ena                       : 1;  /**< LMC early fill enable */
	uint64_t ef_cnt                       : 7;  /**< LMC early fill count
                                                         Specifies the number of cycles after the first LMC
                                                         fill cycle to wait before requesting a fill on the
                                                         RSC/RSD bus.
                                                           // 7 dclks (we've received 1st out of 8
                                                           // by the time we start counting)
                                                           ef_cnt = ((LMCn_CONFIG[MODE32b] ? 14 : 7) *
                                                                     dclk0_period) / rclk_period;
                                                           // + 1 rclk if the dclk and rclk edges don't
                                                           // stay in the same position
                                                           if ((dclk0_gen.period % rclk_gen.period) != 0)
                                                              ef_cnt = ef_cnt + 1;
                                                           // + 2 rclk synchronization uncertainty
                                                           ef_cnt = ef_cnt + 2;
                                                           // - 3 rclks to recognize first write
                                                           ef_cnt = ef_cnt - 3;
                                                           // + 3 rclks to perform first write
                                                           ef_cnt = ef_cnt + 3;
                                                           // - 9 rclks minimum latency from counter expire
                                                           // to final fbf read
                                                           ef_cnt = ef_cnt - 9; */
	uint64_t vab_thresh                   : 4;  /**< VAB Threshold
                                                         When the number of valid VABs exceeds this number the
                                                         L2C increases the priority of all writes in the LMC. */
	uint64_t disecc                       : 1;  /**< Tag and Data ECC Disable */
	uint64_t disidxalias                  : 1;  /**< Index Alias Disable */
#else
	uint64_t disidxalias                  : 1;
	uint64_t disecc                       : 1;
	uint64_t vab_thresh                   : 4;
	uint64_t ef_cnt                       : 7;
	uint64_t ef_ena                       : 1;
	uint64_t xmc_arb_mode                 : 1;
	uint64_t rsp_arb_mode                 : 1;
	uint64_t maxlfb                       : 4;
	uint64_t maxvab                       : 4;
	uint64_t discclk                      : 1;
	uint64_t l2dfdbe                      : 1;
	uint64_t l2dfsbe                      : 1;
	uint64_t disstgl2i                    : 1;
	uint64_t rdf_fast                     : 1;
	uint64_t reserved_29_63               : 35;
#endif
	} cn61xx;
	struct cvmx_l2c_ctl_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_28_63               : 36;
	uint64_t disstgl2i                    : 1;  /**< Disable STGL2I's from changing the tags */
	uint64_t l2dfsbe                      : 1;  /**< Force single bit ECC error on PL2 allocates (2) */
	uint64_t l2dfdbe                      : 1;  /**< Force double bit ECC error on PL2 allocates (2) */
	uint64_t discclk                      : 1;  /**< Disable conditional clocking in L2C PNR blocks */
	uint64_t maxvab                       : 4;  /**< Maximum VABs in use at once
                                                         (0 means 16, 1-15 as expected) */
	uint64_t maxlfb                       : 4;  /**< Maximum LFBs in use at once
                                                         (0 means 16, 1-15 as expected) */
	uint64_t rsp_arb_mode                 : 1;  /**< Arbitration mode for RSC/RSD bus
                                                         == 0, round-robin
                                                         == 1, static priority
                                                             1. IOR data
                                                             2. STIN/FILLs
                                                             3. STDN/SCDN/SCFL */
	uint64_t xmc_arb_mode                 : 1;  /**< Arbitration mode for XMC QOS queues
                                                         == 0, fully determined through QOS
                                                         == 1, QOS0 highest priority, QOS1-3 use normal mode */
	uint64_t ef_ena                       : 1;  /**< LMC early fill enable */
	uint64_t ef_cnt                       : 7;  /**< LMC early fill count
                                                         Specifies the number of cycles after the first LMC
                                                         fill cycle to wait before requesting a fill on the
                                                         RSC/RSD bus.
                                                           // 7 dclks (we've received 1st out of 8
                                                           // by the time we start counting)
                                                           ef_cnt = (7 * dclk0_period) / rclk_period;
                                                           // + 1 rclk if the dclk and rclk edges don't
                                                           // stay in the same position
                                                           if ((dclk0_gen.period % rclk_gen.period) != 0)
                                                              ef_cnt = ef_cnt + 1;
                                                           // + 2 rclk synchronization uncertainty
                                                           ef_cnt = ef_cnt + 2;
                                                           // - 3 rclks to recognize first write
                                                           ef_cnt = ef_cnt - 3;
                                                           // + 3 rclks to perform first write
                                                           ef_cnt = ef_cnt + 3;
                                                           // - 9 rclks minimum latency from counter expire
                                                           // to final fbf read
                                                           ef_cnt = ef_cnt - 9; */
	uint64_t vab_thresh                   : 4;  /**< VAB Threshold
                                                         When the number of valid VABs exceeds this number the
                                                         L2C increases the priority of all writes in the LMC. */
	uint64_t disecc                       : 1;  /**< Tag and Data ECC Disable */
	uint64_t disidxalias                  : 1;  /**< Index Alias Disable */
#else
	uint64_t disidxalias                  : 1;
	uint64_t disecc                       : 1;
	uint64_t vab_thresh                   : 4;
	uint64_t ef_cnt                       : 7;
	uint64_t ef_ena                       : 1;
	uint64_t xmc_arb_mode                 : 1;
	uint64_t rsp_arb_mode                 : 1;
	uint64_t maxlfb                       : 4;
	uint64_t maxvab                       : 4;
	uint64_t discclk                      : 1;
	uint64_t l2dfdbe                      : 1;
	uint64_t l2dfsbe                      : 1;
	uint64_t disstgl2i                    : 1;
	uint64_t reserved_28_63               : 36;
#endif
	} cn63xx;
	struct cvmx_l2c_ctl_cn63xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_25_63               : 39;
	uint64_t discclk                      : 1;  /**< Disable conditional clocking in L2C PNR blocks */
	uint64_t maxvab                       : 4;  /**< Maximum VABs in use at once
                                                         (0 means 16, 1-15 as expected) */
	uint64_t maxlfb                       : 4;  /**< Maximum LFBs in use at once
                                                         (0 means 16, 1-15 as expected) */
	uint64_t rsp_arb_mode                 : 1;  /**< Arbitration mode for RSC/RSD bus
                                                         == 0, round-robin
                                                         == 1, static priority
                                                             1. IOR data
                                                             2. STIN/FILLs
                                                             3. STDN/SCDN/SCFL */
	uint64_t xmc_arb_mode                 : 1;  /**< Arbitration mode for XMC QOS queues
                                                         == 0, fully determined through QOS
                                                         == 1, QOS0 highest priority, QOS1-3 use normal mode */
	uint64_t ef_ena                       : 1;  /**< LMC early fill enable */
	uint64_t ef_cnt                       : 7;  /**< LMC early fill count
                                                         Specifies the number of cycles after the first LMC
                                                         fill cycle to wait before requesting a fill on the
                                                         RSC/RSD bus.
                                                           // 7 dclks (we've received 1st out of 8
                                                           // by the time we start counting)
                                                           ef_cnt = (7 * dclk0_period) / rclk_period;
                                                           // + 1 rclk if the dclk and rclk edges don't
                                                           // stay in the same position
                                                           if ((dclk0_gen.period % rclk_gen.period) != 0)
                                                              ef_cnt = ef_cnt + 1;
                                                           // + 2 rclk synchronization uncertainty
                                                           ef_cnt = ef_cnt + 2;
                                                           // - 3 rclks to recognize first write
                                                           ef_cnt = ef_cnt - 3;
                                                           // + 3 rclks to perform first write
                                                           ef_cnt = ef_cnt + 3;
                                                           // - 9 rclks minimum latency from counter expire
                                                           // to final fbf read
                                                           ef_cnt = ef_cnt - 9; */
	uint64_t vab_thresh                   : 4;  /**< VAB Threshold
                                                         When the number of valid VABs exceeds this number the
                                                         L2C increases the priority of all writes in the LMC. */
	uint64_t disecc                       : 1;  /**< Tag and Data ECC Disable */
	uint64_t disidxalias                  : 1;  /**< Index Alias Disable */
#else
	uint64_t disidxalias                  : 1;
	uint64_t disecc                       : 1;
	uint64_t vab_thresh                   : 4;
	uint64_t ef_cnt                       : 7;
	uint64_t ef_ena                       : 1;
	uint64_t xmc_arb_mode                 : 1;
	uint64_t rsp_arb_mode                 : 1;
	uint64_t maxlfb                       : 4;
	uint64_t maxvab                       : 4;
	uint64_t discclk                      : 1;
	uint64_t reserved_25_63               : 39;
#endif
	} cn63xxp1;
	struct cvmx_l2c_ctl_cn61xx            cn66xx;
	struct cvmx_l2c_ctl_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_30_63               : 34;
	uint64_t sepcmt                       : 1;  /**< Sends all invals before the corresponding commit. */
	uint64_t rdf_fast                     : 1;  /**< When 0, delay read data fifo from DCLK to RCLK by one
                                                         cycle.  Needed when DCLK:RCLK ratio > 3:1.  Should be
                                                         set before DDR traffic begins and only changed when
                                                         memory traffic is idle. */
	uint64_t disstgl2i                    : 1;  /**< Disable STGL2I's from changing the tags */
	uint64_t l2dfsbe                      : 1;  /**< Force single bit ECC error on PL2 allocates (2) */
	uint64_t l2dfdbe                      : 1;  /**< Force double bit ECC error on PL2 allocates (2) */
	uint64_t discclk                      : 1;  /**< Disable conditional clocking in L2C PNR blocks */
	uint64_t maxvab                       : 4;  /**< Maximum VABs in use at once
                                                         (0 means 16, 1-15 as expected) */
	uint64_t maxlfb                       : 4;  /**< Maximum LFBs in use at once
                                                         (0 means 16, 1-15 as expected) */
	uint64_t rsp_arb_mode                 : 1;  /**< Arbitration mode for RSC/RSD bus
                                                         == 0, round-robin
                                                         == 1, static priority
                                                             1. IOR data
                                                             2. STIN/FILLs
                                                             3. STDN/SCDN/SCFL */
	uint64_t xmc_arb_mode                 : 1;  /**< Arbitration mode for XMC QOS queues
                                                         == 0, fully determined through QOS
                                                         == 1, QOS0 highest priority, QOS1-7 use normal mode */
	uint64_t ef_ena                       : 1;  /**< LMC early fill enable */
	uint64_t ef_cnt                       : 7;  /**< LMC early fill count
                                                         Specifies the number of cycles after the first LMC
                                                         fill cycle to wait before requesting a fill on the
                                                         RSC/RSD bus.
                                                           // 7 dclks (we've received 1st out of 8
                                                           // by the time we start counting)
                                                           ef_cnt = (7 * dclk0_period) / rclk_period;
                                                           // + 1 rclk if the dclk and rclk edges don't
                                                           // stay in the same position
                                                           if ((dclk0_gen.period % rclk_gen.period) != 0)
                                                              ef_cnt = ef_cnt + 1;
                                                           // + 2 rclk synchronization uncertainty
                                                           ef_cnt = ef_cnt + 2;
                                                           // - 3 rclks to recognize first write
                                                           ef_cnt = ef_cnt - 3;
                                                           // + 3 rclks to perform first write
                                                           ef_cnt = ef_cnt + 3;
                                                           // - 9 rclks minimum latency from counter expire
                                                           // to final fbf read
                                                           ef_cnt = ef_cnt - 9; */
	uint64_t vab_thresh                   : 4;  /**< VAB Threshold
                                                         When the number of valid VABs exceeds this number the
                                                         L2C increases the priority of all writes in the LMC. */
	uint64_t disecc                       : 1;  /**< Tag and Data ECC Disable */
	uint64_t disidxalias                  : 1;  /**< Index Alias Disable */
#else
	uint64_t disidxalias                  : 1;
	uint64_t disecc                       : 1;
	uint64_t vab_thresh                   : 4;
	uint64_t ef_cnt                       : 7;
	uint64_t ef_ena                       : 1;
	uint64_t xmc_arb_mode                 : 1;
	uint64_t rsp_arb_mode                 : 1;
	uint64_t maxlfb                       : 4;
	uint64_t maxvab                       : 4;
	uint64_t discclk                      : 1;
	uint64_t l2dfdbe                      : 1;
	uint64_t l2dfsbe                      : 1;
	uint64_t disstgl2i                    : 1;
	uint64_t rdf_fast                     : 1;
	uint64_t sepcmt                       : 1;
	uint64_t reserved_30_63               : 34;
#endif
	} cn68xx;
	struct cvmx_l2c_ctl_cn63xx            cn68xxp1;
	struct cvmx_l2c_ctl_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t ocla_qos                     : 3;  /**< QOS level for the transactions from OCLA to L2C. */
	uint64_t reserved_28_28               : 1;
	uint64_t disstgl2i                    : 1;  /**< Disable STGL2Is from changing the tags. */
	uint64_t reserved_25_26               : 2;
	uint64_t discclk                      : 1;  /**< Disable conditional clocking in L2C PNR blocks. */
	uint64_t reserved_16_23               : 8;
	uint64_t rsp_arb_mode                 : 1;  /**< Arbitration mode for RSC/RSD bus. 0 = round-robin; 1 = static priority.
                                                         1. IOR data.
                                                         2. STIN/FILLs.
                                                         3. STDN/SCDN/SCFL. */
	uint64_t xmc_arb_mode                 : 1;  /**< Arbitration mode for ADD bus QOS queues. 0 = fully determined through QOS, 1 = QOS0
                                                         highest priority; QOS 1-7 use normal mode. */
	uint64_t rdf_cnt                      : 8;  /**< Defines the sample point of the LMC response data in the DDR-clock/core-clock crossing.
                                                         For optimal performance set to
                                                         10 * (DDR-clock period/core-clock period) - 1.
                                                         To disable set to 0. All other values are reserved. */
	uint64_t reserved_2_5                 : 4;
	uint64_t disecc                       : 1;  /**< Tag and data ECC disable. */
	uint64_t disidxalias                  : 1;  /**< Index alias disable. */
#else
	uint64_t disidxalias                  : 1;
	uint64_t disecc                       : 1;
	uint64_t reserved_2_5                 : 4;
	uint64_t rdf_cnt                      : 8;
	uint64_t xmc_arb_mode                 : 1;
	uint64_t rsp_arb_mode                 : 1;
	uint64_t reserved_16_23               : 8;
	uint64_t discclk                      : 1;
	uint64_t reserved_25_26               : 2;
	uint64_t disstgl2i                    : 1;
	uint64_t reserved_28_28               : 1;
	uint64_t ocla_qos                     : 3;
	uint64_t reserved_32_63               : 32;
#endif
	} cn70xx;
	struct cvmx_l2c_ctl_cn70xx            cn70xxp1;
	struct cvmx_l2c_ctl_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t ocla_qos                     : 3;  /**< QOS level for the transactions from OCLA to L2C. */
	uint64_t reserved_28_28               : 1;
	uint64_t disstgl2i                    : 1;  /**< Disable STGL2Is from changing the tags. */
	uint64_t reserved_25_26               : 2;
	uint64_t discclk                      : 1;  /**< Disable conditional clocking in L2C PNR blocks. */
	uint64_t reserved_16_23               : 8;
	uint64_t rsp_arb_mode                 : 1;  /**< Arbitration mode for RSC/RSD bus. 0 = round-robin; 1 = static priority.
                                                         1. IOR data.
                                                         2. STIN/FILLs.
                                                         3. STDN/SCDN/SCFL. */
	uint64_t xmc_arb_mode                 : 1;  /**< Arbitration mode for ADD bus QOS queues. 0 = fully determined through QOS, 1 = QOS0
                                                         highest priority; QOS 1-7 use normal mode. */
	uint64_t rdf_cnt                      : 8;  /**< Defines the sample point of the LMC response data in the DDR-clock/core-clock crossing.
                                                         For optimal performance set to
                                                         10 * (DDR-clock period/core-clock period) - 1.
                                                         To disable set to 0. All other values are reserved. */
	uint64_t reserved_4_5                 : 2;
	uint64_t disldwb                      : 1;  /**< Suppresses the DWB functionality of any received LDWB, effectively turning them into LDTs. */
	uint64_t dissblkdty                   : 1;  /**< Disable bandwidth optimization between L2 and LMC which only transfers modified
                                                         sub-blocks when possible. */
	uint64_t disecc                       : 1;  /**< Tag and data ECC disable. */
	uint64_t disidxalias                  : 1;  /**< Index alias disable. */
#else
	uint64_t disidxalias                  : 1;
	uint64_t disecc                       : 1;
	uint64_t dissblkdty                   : 1;
	uint64_t disldwb                      : 1;
	uint64_t reserved_4_5                 : 2;
	uint64_t rdf_cnt                      : 8;
	uint64_t xmc_arb_mode                 : 1;
	uint64_t rsp_arb_mode                 : 1;
	uint64_t reserved_16_23               : 8;
	uint64_t discclk                      : 1;
	uint64_t reserved_25_26               : 2;
	uint64_t disstgl2i                    : 1;
	uint64_t reserved_28_28               : 1;
	uint64_t ocla_qos                     : 3;
	uint64_t reserved_32_63               : 32;
#endif
	} cn73xx;
	struct cvmx_l2c_ctl_cn73xx            cn78xx;
	struct cvmx_l2c_ctl_cn73xx            cn78xxp1;
	struct cvmx_l2c_ctl_cn61xx            cnf71xx;
	struct cvmx_l2c_ctl_cn73xx            cnf75xx;
};
typedef union cvmx_l2c_ctl cvmx_l2c_ctl_t;

/**
 * cvmx_l2c_dbg
 *
 * L2C_DBG = L2C DEBUG Register
 *
 * Description: L2C Tag/Data Store Debug Register
 *
 * Notes:
 * (1) When using the L2T, L2D or FINV Debug probe feature, the LDD command WILL NOT update the DuTags.
 * (2) L2T, L2D, FINV MUST BE mutually exclusive (only one set)
 * (3) Force Invalidate is intended as a means for SW to invalidate the L2 Cache while also writing back
 *     dirty data to memory to maintain coherency.
 * (4) L2 Cache Lock Down feature MUST BE disabled (L2C_LCKBASE[LCK_ENA]=0) if ANY of the L2C debug
 *     features (L2T, L2D, FINV) are enabled.
 */
union cvmx_l2c_dbg {
	uint64_t u64;
	struct cvmx_l2c_dbg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_15_63               : 49;
	uint64_t lfb_enum                     : 4;  /**< Specifies the LFB Entry# which is to be captured. */
	uint64_t lfb_dmp                      : 1;  /**< LFB Dump Enable: When written(=1), the contents of
                                                         the LFB specified by LFB_ENUM[3:0] are captured
                                                         into the L2C_LFB(0/1/2) registers.
                                                         NOTE: Some fields of the LFB entry are unpredictable
                                                         and dependent on usage. This is only intended to be
                                                         used for HW debug. */
	uint64_t ppnum                        : 4;  /**< When L2C_DBG[L2T] or L2C_DBG[L2D] or L2C_DBG[FINV]
                                                         is enabled, this field determines which one-of-16
                                                         PPs is selected as the diagnostic PP. */
	uint64_t set                          : 3;  /**< When L2C_DBG[L2T] or L2C_DBG[L2D] or L2C_DBG[FINV]
                                                         is enabled, this field determines 1-of-n targeted
                                                         sets to act upon.
                                                         NOTE: L2C_DBG[SET] must never equal a crippled or
                                                         unusable set (see UMSK* registers and Cripple mode
                                                         fuses). */
	uint64_t finv                         : 1;  /**< Flush-Invalidate.
                                                         When flush-invalidate is enable (FINV=1), all STF
                                                         (L1 store-miss) commands generated from the diagnostic PP
                                                         (L2C_DBG[PPNUM]) will invalidate the specified set
                                                         (L2C_DBG[SET]) at the index specified in the STF
                                                         address[17:7]. If a dirty block is detected (D=1), it is
                                                         written back to memory. The contents of the invalid
                                                         L2 Cache line is also 'scrubbed' with the STF write data.
                                                         NOTE: If L2C_CFG[IDXALIAS]=1, the index specified in
                                                         STF address[17:7] refers to the 'aliased' address.
                                                         NOTE: An STF command with write data=ZEROES can be
                                                         generated by SW using the Prefetch instruction with
                                                         Hint=30d "prepare for Store", followed by a SYNCW.
                                                         What is seen at the L2C as an STF w/wrdcnt=0 with all
                                                         of its mask bits clear (indicates zero-fill data).
                                                         A flush-invalidate will 'force-hit' the L2 cache at
                                                         [index,set] and invalidate the entry (V=0/D=0/L=0/U=0).
                                                         If the cache block is dirty, it is also written back
                                                         to memory. The DuTag state is probed/updated as normal
                                                         for an STF request.
                                                         TYPICAL APPLICATIONS:
                                                            1) L2 Tag/Data ECC SW Recovery
                                                            2) Cache Unlocking
                                                         NOTE: If the cacheline had been previously LOCKED(L=1),
                                                         a flush-invalidate operation will explicitly UNLOCK
                                                         (L=0) the set/index specified.
                                                         NOTE: The diagnostic PP cores can generate STF
                                                         commands to the L2 Cache whenever all 128 bytes in a
                                                         block are written. SW must take this into consideration
                                                         to avoid 'errant' Flush-Invalidates. */
	uint64_t l2d                          : 1;  /**< When enabled (and L2C_DBG[L2T]=0), fill data is
                                                         returned directly from the L2 Data Store
                                                         (regardless of hit/miss) when an LDD(L1 load-miss) command
                                                         is issued from a PP determined by the L2C_DBG[PPNUM]
                                                         field. The selected set# is determined by the
                                                         L2C_DBG[SET] field, and the index is determined
                                                         from the address[17:7] associated with the LDD
                                                         command.
                                                         This 'force-hit' will NOT alter the current L2 Tag
                                                         state OR the DuTag state. */
	uint64_t l2t                          : 1;  /**< When enabled, L2 Tag information [V,D,L,U,phys_addr[33:18]]
                                                         is returned on the data bus starting at +32(and +96) bytes
                                                         offset from the beginning of cacheline when an LDD
                                                         (L1 load-miss) command is issued from a PP determined by
                                                         the L2C_DBG[PPNUM] field.
                                                         The selected L2 set# is determined by the L2C_DBG[SET]
                                                         field, and the L2 index is determined from the
                                                         phys_addr[17:7] associated with the LDD command.
                                                         This 'L2 force-hit' will NOT alter the current L2 Tag
                                                         state OR the DuTag state.
                                                         NOTE: The diagnostic PP should issue a d-stream load
                                                         to an aligned cacheline+0x20(+0x60) in order to have the
                                                         return VDLUTAG information (in OW2/OW6) written directly
                                                         into the proper PP register. The diagnostic PP should also
                                                         flush it's local L1 cache after use(to ensure data
                                                         coherency).
                                                         NOTE: The position of the VDLUTAG data in the destination
                                                         register is dependent on the endian mode(big/little).
                                                         NOTE: N3K-Pass2 modification. (This bit's functionality
                                                         has changed since Pass1-in the following way).
                                                         NOTE: (For L2C BitMap testing of L2 Data Store OW ECC):
                                                         If L2D_ERR[ECC_ENA]=0, the OW ECC from the selected
                                                         half cacheline (see: L2D_ERR[BMHCLSEL] is also
                                                         conditionally latched into the L2D_FSYN0/1 CSRs if an
                                                         LDD command is detected from the diagnostic PP(L2C_DBG[PPNUM]). */
#else
	uint64_t l2t                          : 1;
	uint64_t l2d                          : 1;
	uint64_t finv                         : 1;
	uint64_t set                          : 3;
	uint64_t ppnum                        : 4;
	uint64_t lfb_dmp                      : 1;
	uint64_t lfb_enum                     : 4;
	uint64_t reserved_15_63               : 49;
#endif
	} s;
	struct cvmx_l2c_dbg_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_13_63               : 51;
	uint64_t lfb_enum                     : 2;  /**< Specifies the LFB Entry# which is to be captured. */
	uint64_t lfb_dmp                      : 1;  /**< LFB Dump Enable: When written(=1), the contents of
                                                         the LFB specified by LFB_ENUM are captured
                                                         into the L2C_LFB(0/1/2) registers.
                                                         NOTE: Some fields of the LFB entry are unpredictable
                                                         and dependent on usage. This is only intended to be
                                                         used for HW debug. */
	uint64_t reserved_7_9                 : 3;
	uint64_t ppnum                        : 1;  /**< When L2C_DBG[L2T] or L2C_DBG[L2D] or L2C_DBG[FINV]
                                                         is enabled, this field determines which
                                                         PP is selected as the diagnostic PP.
                                                         NOTE: For CN30XX single core PPNUM=0 (MBZ) */
	uint64_t reserved_5_5                 : 1;
	uint64_t set                          : 2;  /**< When L2C_DBG[L2T] or L2C_DBG[L2D] or L2C_DBG[FINV]
                                                         is enabled, this field determines 1-of-n targeted
                                                         sets to act upon.
                                                         NOTE: L2C_DBG[SET] must never equal a crippled or
                                                         unusable set (see UMSK* registers and Cripple mode
                                                         fuses). */
	uint64_t finv                         : 1;  /**< Flush-Invalidate.
                                                         When flush-invalidate is enable (FINV=1), all STF
                                                         (L1 store-miss) commands generated from the PP will invalidate
                                                         the specified set(L2C_DBG[SET]) at the index specified
                                                         in the STF address[14:7]. If a dirty block is detected(D=1),
                                                         it is written back to memory. The contents of the invalid
                                                         L2 Cache line is also 'scrubbed' with the STF write data.
                                                         NOTE: If L2C_CFG[IDXALIAS]=1, the index specified in
                                                         STF address[14:7] refers to the 'aliased' address.
                                                         NOTE: An STF command with write data=ZEROES can be
                                                         generated by SW using the Prefetch instruction with
                                                         Hint=30d "prepare for Store", followed by a SYNCW.
                                                         What is seen at the L2C as an STF w/wrdcnt=0 with all
                                                         of its mask bits clear (indicates zero-fill data).
                                                         A flush-invalidate will 'force-hit' the L2 cache at
                                                         [index,set] and invalidate the entry (V=0/D=0/L=0/U=0).
                                                         If the cache block is dirty, it is also written back
                                                         to memory. The DuTag state is probed/updated as normal
                                                         for an STF request.
                                                         TYPICAL APPLICATIONS:
                                                            1) L2 Tag/Data ECC SW Recovery
                                                            2) Cache Unlocking
                                                         NOTE: If the cacheline had been previously LOCKED(L=1),
                                                         a flush-invalidate operation will explicitly UNLOCK
                                                         (L=0) the set/index specified.
                                                         NOTE: The PP can generate STF(L1 store-miss)
                                                         commands to the L2 Cache whenever all 128 bytes in a
                                                         block are written. SW must take this into consideration
                                                         to avoid 'errant' Flush-Invalidates. */
	uint64_t l2d                          : 1;  /**< When enabled (and L2C_DBG[L2T]=0), fill data is
                                                         returned directly from the L2 Data Store
                                                         (regardless of hit/miss) when an LDD(L1 load-miss)
                                                         command is issued from the PP.
                                                         The selected set# is determined by the
                                                         L2C_DBG[SET] field, and the index is determined
                                                         from the address[14:7] associated with the LDD
                                                         command.
                                                         This 'force-hit' will NOT alter the current L2 Tag
                                                         state OR the DuTag state. */
	uint64_t l2t                          : 1;  /**< When enabled, L2 Tag information [V,D,L,U,phys_addr[33:15]]
                                                         is returned on the data bus starting at +32(and +96) bytes
                                                         offset from the beginning of cacheline when an LDD
                                                         (L1 load-miss) command is issued from the PP.
                                                         The selected L2 set# is determined by the L2C_DBG[SET]
                                                         field, and the L2 index is determined from the
                                                         phys_addr[14:7] associated with the LDD command.
                                                         This 'L2 force-hit' will NOT alter the current L2 Tag
                                                         state OR the DuTag state.
                                                         NOTE: The diagnostic PP should issue a d-stream load
                                                         to an aligned cacheline+0x20(+0x60) in order to have the
                                                         return VDLUTAG information (in OW2/OW6) written directly
                                                         into the proper PP register. The diagnostic PP should also
                                                         flush it's local L1 cache after use(to ensure data
                                                         coherency).
                                                         NOTE: The position of the VDLUTAG data in the destination
                                                         register is dependent on the endian mode(big/little).
                                                         NOTE: (For L2C BitMap testing of L2 Data Store OW ECC):
                                                         If L2D_ERR[ECC_ENA]=0, the OW ECC from the selected
                                                         half cacheline (see: L2D_ERR[BMHCLSEL] is also
                                                         conditionally latched into the L2D_FSYN0/1 CSRs if an
                                                         LDD(L1 load-miss) is detected. */
#else
	uint64_t l2t                          : 1;
	uint64_t l2d                          : 1;
	uint64_t finv                         : 1;
	uint64_t set                          : 2;
	uint64_t reserved_5_5                 : 1;
	uint64_t ppnum                        : 1;
	uint64_t reserved_7_9                 : 3;
	uint64_t lfb_dmp                      : 1;
	uint64_t lfb_enum                     : 2;
	uint64_t reserved_13_63               : 51;
#endif
	} cn30xx;
	struct cvmx_l2c_dbg_cn31xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t lfb_enum                     : 3;  /**< Specifies the LFB Entry# which is to be captured. */
	uint64_t lfb_dmp                      : 1;  /**< LFB Dump Enable: When written(=1), the contents of
                                                         the LFB specified by LFB_ENUM are captured
                                                         into the L2C_LFB(0/1/2) registers.
                                                         NOTE: Some fields of the LFB entry are unpredictable
                                                         and dependent on usage. This is only intended to be
                                                         used for HW debug. */
	uint64_t reserved_7_9                 : 3;
	uint64_t ppnum                        : 1;  /**< When L2C_DBG[L2T] or L2C_DBG[L2D] or L2C_DBG[FINV]
                                                         is enabled, this field determines which
                                                         PP is selected as the diagnostic PP. */
	uint64_t reserved_5_5                 : 1;
	uint64_t set                          : 2;  /**< When L2C_DBG[L2T] or L2C_DBG[L2D] or L2C_DBG[FINV]
                                                         is enabled, this field determines 1-of-n targeted
                                                         sets to act upon.
                                                         NOTE: L2C_DBG[SET] must never equal a crippled or
                                                         unusable set (see UMSK* registers and Cripple mode
                                                         fuses). */
	uint64_t finv                         : 1;  /**< Flush-Invalidate.
                                                         When flush-invalidate is enable (FINV=1), all STF
                                                         (L1 store-miss) commands generated from the diagnostic PP
                                                         (L2C_DBG[PPNUM]) will invalidate the specified set
                                                         (L2C_DBG[SET]) at the index specified in the STF
                                                         address[15:7]. If a dirty block is detected (D=1), it is
                                                         written back to memory. The contents of the invalid
                                                         L2 Cache line is also 'scrubbed' with the STF write data.
                                                         NOTE: If L2C_CFG[IDXALIAS]=1, the index specified in
                                                         STF address[15:7] refers to the 'aliased' address.
                                                         NOTE: An STF command with write data=ZEROES can be
                                                         generated by SW using the Prefetch instruction with
                                                         Hint=30d "prepare for Store", followed by a SYNCW.
                                                         What is seen at the L2C as an STF w/wrdcnt=0 with all
                                                         of its mask bits clear (indicates zero-fill data).
                                                         A flush-invalidate will 'force-hit' the L2 cache at
                                                         [index,set] and invalidate the entry (V=0/D=0/L=0/U=0).
                                                         If the cache block is dirty, it is also written back
                                                         to memory. The DuTag state is probed/updated as normal
                                                         for an STF request.
                                                         TYPICAL APPLICATIONS:
                                                            1) L2 Tag/Data ECC SW Recovery
                                                            2) Cache Unlocking
                                                         NOTE: If the cacheline had been previously LOCKED(L=1),
                                                         a flush-invalidate operation will explicitly UNLOCK
                                                         (L=0) the set/index specified.
                                                         NOTE: The diagnostic PP cores can generate STF(L1 store-miss)
                                                         commands to the L2 Cache whenever all 128 bytes in a
                                                         block are written. SW must take this into consideration
                                                         to avoid 'errant' Flush-Invalidates. */
	uint64_t l2d                          : 1;  /**< When enabled (and L2C_DBG[L2T]=0), fill data is
                                                         returned directly from the L2 Data Store
                                                         (regardless of hit/miss) when an LDD(L1 load-miss)
                                                         command is issued from a PP determined by the
                                                         L2C_DBG[PPNUM] field. The selected set# is determined
                                                         by the L2C_DBG[SET] field, and the index is determined
                                                         from the address[15:7] associated with the LDD command.
                                                         This 'L2 force-hit' will NOT alter the current L2 Tag
                                                         state OR the DuTag state. */
	uint64_t l2t                          : 1;  /**< When enabled, L2 Tag information [V,D,L,U,phys_addr[33:16]]
                                                         is returned on the data bus starting at +32(and +96) bytes
                                                         offset from the beginning of cacheline when an LDD
                                                         (L1 load-miss) command is issued from a PP determined by
                                                         the L2C_DBG[PPNUM] field.
                                                         The selected L2 set# is determined by the L2C_DBG[SET]
                                                         field, and the L2 index is determined from the
                                                         phys_addr[15:7] associated with the LDD command.
                                                         This 'L2 force-hit' will NOT alter the current L2 Tag
                                                         state OR the DuTag state.
                                                         NOTE: The diagnostic PP should issue a d-stream load
                                                         to an aligned cacheline+0x20(+0x60) in order to have the
                                                         return VDLUTAG information (in OW2/OW6) written directly
                                                         into the proper PP register. The diagnostic PP should also
                                                         flush it's local L1 cache after use(to ensure data
                                                         coherency).
                                                         NOTE: The position of the VDLUTAG data in the destination
                                                         register is dependent on the endian mode(big/little).
                                                         NOTE: (For L2C BitMap testing of L2 Data Store OW ECC):
                                                         If L2D_ERR[ECC_ENA]=0, the OW ECC from the selected
                                                         half cacheline (see: L2D_ERR[BMHCLSEL] is also
                                                         conditionally latched into the L2D_FSYN0/1 CSRs if an
                                                         LDD(L1 load-miss) is detected from the diagnostic PP
                                                         (L2C_DBG[PPNUM]). */
#else
	uint64_t l2t                          : 1;
	uint64_t l2d                          : 1;
	uint64_t finv                         : 1;
	uint64_t set                          : 2;
	uint64_t reserved_5_5                 : 1;
	uint64_t ppnum                        : 1;
	uint64_t reserved_7_9                 : 3;
	uint64_t lfb_dmp                      : 1;
	uint64_t lfb_enum                     : 3;
	uint64_t reserved_14_63               : 50;
#endif
	} cn31xx;
	struct cvmx_l2c_dbg_s                 cn38xx;
	struct cvmx_l2c_dbg_s                 cn38xxp2;
	struct cvmx_l2c_dbg_cn50xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t lfb_enum                     : 3;  /**< Specifies the LFB Entry# which is to be captured. */
	uint64_t lfb_dmp                      : 1;  /**< LFB Dump Enable: When written(=1), the contents of
                                                         the LFB specified by LFB_ENUM[2:0] are captured
                                                         into the L2C_LFB(0/1/2) registers.
                                                         NOTE: Some fields of the LFB entry are unpredictable
                                                         and dependent on usage. This is only intended to be
                                                         used for HW debug. */
	uint64_t reserved_7_9                 : 3;
	uint64_t ppnum                        : 1;  /**< When L2C_DBG[L2T] or L2C_DBG[L2D] or L2C_DBG[FINV]
                                                         is enabled, this field determines which 1-of-2
                                                         PPs is selected as the diagnostic PP. */
	uint64_t set                          : 3;  /**< When L2C_DBG[L2T] or L2C_DBG[L2D] or L2C_DBG[FINV]
                                                         is enabled, this field determines 1-of-n targeted
                                                         sets to act upon.
                                                         NOTE: L2C_DBG[SET] must never equal a crippled or
                                                         unusable set (see UMSK* registers and Cripple mode
                                                         fuses). */
	uint64_t finv                         : 1;  /**< Flush-Invalidate.
                                                         When flush-invalidate is enable (FINV=1), all STF
                                                         (L1 store-miss) commands generated from the diagnostic PP
                                                         (L2C_DBG[PPNUM]) will invalidate the specified set
                                                         (L2C_DBG[SET]) at the index specified in the STF
                                                         address[13:7]. If a dirty block is detected (D=1), it is
                                                         written back to memory. The contents of the invalid
                                                         L2 Cache line is also 'scrubbed' with the STF write data.
                                                         NOTE: If L2C_CFG[IDXALIAS]=1, the index specified in
                                                         STF address[13:7] refers to the 'aliased' address.
                                                         NOTE: An STF command with write data=ZEROES can be
                                                         generated by SW using the Prefetch instruction with
                                                         Hint=30d "prepare for Store", followed by a SYNCW.
                                                         What is seen at the L2C as an STF w/wrdcnt=0 with all
                                                         of its mask bits clear (indicates zero-fill data).
                                                         A flush-invalidate will 'force-hit' the L2 cache at
                                                         [index,set] and invalidate the entry (V=0/D=0/L=0/U=0).
                                                         If the cache block is dirty, it is also written back
                                                         to memory. The DuTag state is probed/updated as normal
                                                         for an STF request.
                                                         TYPICAL APPLICATIONS:
                                                            1) L2 Tag/Data ECC SW Recovery
                                                            2) Cache Unlocking
                                                         NOTE: If the cacheline had been previously LOCKED(L=1),
                                                         a flush-invalidate operation will explicitly UNLOCK
                                                         (L=0) the set/index specified.
                                                         NOTE: The diagnostic PP cores can generate STF
                                                         commands to the L2 Cache whenever all 128 bytes in a
                                                         block are written. SW must take this into consideration
                                                         to avoid 'errant' Flush-Invalidates. */
	uint64_t l2d                          : 1;  /**< When enabled (and L2C_DBG[L2T]=0), fill data is
                                                         returned directly from the L2 Data Store
                                                         (regardless of hit/miss) when an LDD(L1 load-miss) command
                                                         is issued from a PP determined by the L2C_DBG[PPNUM]
                                                         field. The selected set# is determined by the
                                                         L2C_DBG[SET] field, and the index is determined
                                                         from the address[13:7] associated with the LDD
                                                         command.
                                                         This 'force-hit' will NOT alter the current L2 Tag
                                                         state OR the DuTag state. */
	uint64_t l2t                          : 1;  /**< When enabled, L2 Tag information [V,D,L,U,phys_addr[33:14]]
                                                         is returned on the data bus starting at +32(and +96) bytes
                                                         offset from the beginning of cacheline when an LDD
                                                         (L1 load-miss) command is issued from a PP determined by
                                                         the L2C_DBG[PPNUM] field.
                                                         The selected L2 set# is determined by the L2C_DBG[SET]
                                                         field, and the L2 index is determined from the
                                                         phys_addr[13:7] associated with the LDD command.
                                                         This 'L2 force-hit' will NOT alter the current L2 Tag
                                                         state OR the DuTag state.
                                                         NOTE: The diagnostic PP should issue a d-stream load
                                                         to an aligned cacheline+0x20(+0x60) in order to have the
                                                         return VDLUTAG information (in OW2/OW6) written directly
                                                         into the proper PP register. The diagnostic PP should also
                                                         flush it's local L1 cache after use(to ensure data
                                                         coherency).
                                                         NOTE: The position of the VDLUTAG data in the destination
                                                         register is dependent on the endian mode(big/little).
                                                         NOTE: (For L2C BitMap testing of L2 Data Store OW ECC):
                                                         If L2D_ERR[ECC_ENA]=0, the OW ECC from the selected
                                                         half cacheline (see: L2D_ERR[BMHCLSEL] is also
                                                         conditionally latched into the L2D_FSYN0/1 CSRs if an
                                                         LDD command is detected from the diagnostic PP(L2C_DBG[PPNUM]). */
#else
	uint64_t l2t                          : 1;
	uint64_t l2d                          : 1;
	uint64_t finv                         : 1;
	uint64_t set                          : 3;
	uint64_t ppnum                        : 1;
	uint64_t reserved_7_9                 : 3;
	uint64_t lfb_dmp                      : 1;
	uint64_t lfb_enum                     : 3;
	uint64_t reserved_14_63               : 50;
#endif
	} cn50xx;
	struct cvmx_l2c_dbg_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t lfb_enum                     : 3;  /**< Specifies the LFB Entry# which is to be captured. */
	uint64_t lfb_dmp                      : 1;  /**< LFB Dump Enable: When written(=1), the contents of
                                                         the LFB specified by LFB_ENUM[2:0] are captured
                                                         into the L2C_LFB(0/1/2) registers.
                                                         NOTE: Some fields of the LFB entry are unpredictable
                                                         and dependent on usage. This is only intended to be
                                                         used for HW debug. */
	uint64_t reserved_8_9                 : 2;
	uint64_t ppnum                        : 2;  /**< When L2C_DBG[L2T] or L2C_DBG[L2D] or L2C_DBG[FINV]
                                                         is enabled, this field determines which 1-of-4
                                                         PPs is selected as the diagnostic PP. */
	uint64_t set                          : 3;  /**< When L2C_DBG[L2T] or L2C_DBG[L2D] or L2C_DBG[FINV]
                                                         is enabled, this field determines 1-of-n targeted
                                                         sets to act upon.
                                                         NOTE: L2C_DBG[SET] must never equal a crippled or
                                                         unusable set (see UMSK* registers and Cripple mode
                                                         fuses). */
	uint64_t finv                         : 1;  /**< Flush-Invalidate.
                                                         When flush-invalidate is enable (FINV=1), all STF
                                                         (L1 store-miss) commands generated from the diagnostic PP
                                                         (L2C_DBG[PPNUM]) will invalidate the specified set
                                                         (L2C_DBG[SET]) at the index specified in the STF
                                                         address[15:7]. If a dirty block is detected (D=1), it is
                                                         written back to memory. The contents of the invalid
                                                         L2 Cache line is also 'scrubbed' with the STF write data.
                                                         NOTE: If L2C_CFG[IDXALIAS]=1, the index specified in
                                                         STF address[15:7] refers to the 'aliased' address.
                                                         NOTE: An STF command with write data=ZEROES can be
                                                         generated by SW using the Prefetch instruction with
                                                         Hint=30d "prepare for Store", followed by a SYNCW.
                                                         What is seen at the L2C as an STF w/wrdcnt=0 with all
                                                         of its mask bits clear (indicates zero-fill data).
                                                         A flush-invalidate will 'force-hit' the L2 cache at
                                                         [index,set] and invalidate the entry (V=0/D=0/L=0/U=0).
                                                         If the cache block is dirty, it is also written back
                                                         to memory. The DuTag state is probed/updated as normal
                                                         for an STF request.
                                                         TYPICAL APPLICATIONS:
                                                            1) L2 Tag/Data ECC SW Recovery
                                                            2) Cache Unlocking
                                                         NOTE: If the cacheline had been previously LOCKED(L=1),
                                                         a flush-invalidate operation will explicitly UNLOCK
                                                         (L=0) the set/index specified.
                                                         NOTE: The diagnostic PP cores can generate STF
                                                         commands to the L2 Cache whenever all 128 bytes in a
                                                         block are written. SW must take this into consideration
                                                         to avoid 'errant' Flush-Invalidates. */
	uint64_t l2d                          : 1;  /**< When enabled (and L2C_DBG[L2T]=0), fill data is
                                                         returned directly from the L2 Data Store
                                                         (regardless of hit/miss) when an LDD(L1 load-miss) command
                                                         is issued from a PP determined by the L2C_DBG[PPNUM]
                                                         field. The selected set# is determined by the
                                                         L2C_DBG[SET] field, and the index is determined
                                                         from the address[15:7] associated with the LDD
                                                         command.
                                                         This 'force-hit' will NOT alter the current L2 Tag
                                                         state OR the DuTag state. */
	uint64_t l2t                          : 1;  /**< When enabled, L2 Tag information [V,D,L,U,phys_addr[33:16]]
                                                         is returned on the data bus starting at +32(and +96) bytes
                                                         offset from the beginning of cacheline when an LDD
                                                         (L1 load-miss) command is issued from a PP determined by
                                                         the L2C_DBG[PPNUM] field.
                                                         The selected L2 set# is determined by the L2C_DBG[SET]
                                                         field, and the L2 index is determined from the
                                                         phys_addr[15:7] associated with the LDD command.
                                                         This 'L2 force-hit' will NOT alter the current L2 Tag
                                                         state OR the DuTag state.
                                                         NOTE: The diagnostic PP should issue a d-stream load
                                                         to an aligned cacheline+0x20(+0x60) in order to have the
                                                         return VDLUTAG information (in OW2/OW6) written directly
                                                         into the proper PP register. The diagnostic PP should also
                                                         flush it's local L1 cache after use(to ensure data
                                                         coherency).
                                                         NOTE: The position of the VDLUTAG data in the destination
                                                         register is dependent on the endian mode(big/little).
                                                         NOTE: (For L2C BitMap testing of L2 Data Store OW ECC):
                                                         If L2D_ERR[ECC_ENA]=0, the OW ECC from the selected
                                                         half cacheline (see: L2D_ERR[BMHCLSEL] is also
                                                         conditionally latched into the L2D_FSYN0/1 CSRs if an
                                                         LDD command is detected from the diagnostic PP(L2C_DBG[PPNUM]). */
#else
	uint64_t l2t                          : 1;
	uint64_t l2d                          : 1;
	uint64_t finv                         : 1;
	uint64_t set                          : 3;
	uint64_t ppnum                        : 2;
	uint64_t reserved_8_9                 : 2;
	uint64_t lfb_dmp                      : 1;
	uint64_t lfb_enum                     : 3;
	uint64_t reserved_14_63               : 50;
#endif
	} cn52xx;
	struct cvmx_l2c_dbg_cn52xx            cn52xxp1;
	struct cvmx_l2c_dbg_s                 cn56xx;
	struct cvmx_l2c_dbg_s                 cn56xxp1;
	struct cvmx_l2c_dbg_s                 cn58xx;
	struct cvmx_l2c_dbg_s                 cn58xxp1;
};
typedef union cvmx_l2c_dbg cvmx_l2c_dbg_t;

/**
 * cvmx_l2c_dut
 *
 * L2C_DUT = L2C DUTAG Register
 *
 * Description: L2C Duplicate Tag State Register
 *
 * Notes:
 * (1) When using the L2T, L2D or FINV Debug probe feature, an LDD command issued by the diagnostic PP
 *     WILL NOT update the DuTags.
 * (2) L2T, L2D, FINV MUST BE mutually exclusive (only one enabled at a time).
 * (3) Force Invalidate is intended as a means for SW to invalidate the L2 Cache while also writing back
 *     dirty data to memory to maintain coherency. (A side effect of FINV is that an LDD L2 fill is
 *     launched which fills data into the L2 DS).
 */
union cvmx_l2c_dut {
	uint64_t u64;
	struct cvmx_l2c_dut_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t dtena                        : 1;  /**< DuTag Diagnostic read enable.
                                                         When L2C_DUT[DTENA]=1, all LDD(L1 load-miss)
                                                         commands issued from the diagnostic PP
                                                         (L2C_DBG[PPNUM]) will capture the DuTag state (V|L1TAG)
                                                         of the PP#(specified in the LDD address[29:26] into
                                                         the L2C_DUT CSR register. This allows the diagPP to
                                                         read ALL DuTags (from any PP).
                                                         The DuTag Set# to capture is extracted from the LDD
                                                         address[25:20]. The diagnostic PP would issue the
                                                         LDD then read the L2C_DUT register (one at a time).
                                                         This LDD 'L2 force-hit' will NOT alter the current L2
                                                         Tag State OR the DuTag state.
                                                         NOTE: For CN58XX the DuTag SIZE has doubled (to 16KB)
                                                         where each DuTag is organized as 2x 64-way entries.
                                                         The LDD address[7] determines which 1(of-2) internal
                                                         64-ways to select.
                                                         The fill data is returned directly from the L2 Data
                                                         Store(regardless of hit/miss) when an LDD command
                                                         is issued from a PP determined by the L2C_DBG[PPNUM]
                                                         field. The selected L2 Set# is determined by the
                                                         L2C_DBG[SET] field, and the index is determined
                                                         from the address[17:7] associated with the LDD
                                                         command.
                                                         This 'L2 force-hit' will NOT alter the current L2 Tag
                                                         state OR the DuTag state.
                                                         NOTE: In order for the DiagPP to generate an LDD command
                                                         to the L2C, it must first force an L1 Dcache flush. */
	uint64_t reserved_30_30               : 1;
	uint64_t dt_vld                       : 1;  /**< Duplicate L1 Tag Valid bit latched in for previous
                                                         LDD(L1 load-miss) command sourced by diagnostic PP. */
	uint64_t dt_tag                       : 29; /**< Duplicate L1 Tag[35:7] latched in for previous
                                                         LDD(L1 load-miss) command sourced by diagnostic PP. */
#else
	uint64_t dt_tag                       : 29;
	uint64_t dt_vld                       : 1;
	uint64_t reserved_30_30               : 1;
	uint64_t dtena                        : 1;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_l2c_dut_s                 cn30xx;
	struct cvmx_l2c_dut_s                 cn31xx;
	struct cvmx_l2c_dut_s                 cn38xx;
	struct cvmx_l2c_dut_s                 cn38xxp2;
	struct cvmx_l2c_dut_s                 cn50xx;
	struct cvmx_l2c_dut_s                 cn52xx;
	struct cvmx_l2c_dut_s                 cn52xxp1;
	struct cvmx_l2c_dut_s                 cn56xx;
	struct cvmx_l2c_dut_s                 cn56xxp1;
	struct cvmx_l2c_dut_s                 cn58xx;
	struct cvmx_l2c_dut_s                 cn58xxp1;
};
typedef union cvmx_l2c_dut cvmx_l2c_dut_t;

/**
 * cvmx_l2c_dut_map#
 *
 * L2C_DUT_MAP = L2C DUT memory map region
 *
 * Description: Address of the start of the region mapped to the duplicate tag.  Can be used to read
 * and write the raw duplicate tag CAM.  Writes should be used only with great care as they can easily
 * destroy the coherency of the memory system.  In any case this region is expected to only be used
 * for debug.
 *
 * This base address should be combined with PP virtual ID, L1 way and L1 set to produce the final
 * address as follows:
 *     addr<63:13>      L2C_DUT_MAP<63:13>
 *     addr<12:11>      PP VID
 *     addr<10:6>       L1 way
 *     addr<5:3>        L1 set
 *     addr<2:0>        UNUSED
 *
 * Notes:
 * (1) The tag is 37:10 from the 38-bit OCTEON physical address after hole removal. (The hole is between DR0
 * and DR1. Remove the hole by subtracting 256MB from 38-bit OCTEON L2/DRAM physical addresses >= 512 MB.)
 */
union cvmx_l2c_dut_mapx {
	uint64_t u64;
	struct cvmx_l2c_dut_mapx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_38_63               : 26;
	uint64_t tag                          : 28; /**< The tag value (see Note 1) */
	uint64_t reserved_1_9                 : 9;
	uint64_t valid                        : 1;  /**< The valid bit */
#else
	uint64_t valid                        : 1;
	uint64_t reserved_1_9                 : 9;
	uint64_t tag                          : 28;
	uint64_t reserved_38_63               : 26;
#endif
	} s;
	struct cvmx_l2c_dut_mapx_s            cn61xx;
	struct cvmx_l2c_dut_mapx_s            cn63xx;
	struct cvmx_l2c_dut_mapx_s            cn63xxp1;
	struct cvmx_l2c_dut_mapx_s            cn66xx;
	struct cvmx_l2c_dut_mapx_s            cn68xx;
	struct cvmx_l2c_dut_mapx_s            cn68xxp1;
	struct cvmx_l2c_dut_mapx_s            cnf71xx;
};
typedef union cvmx_l2c_dut_mapx cvmx_l2c_dut_mapx_t;

/**
 * cvmx_l2c_ecc_ctl
 *
 * Flip ECC bits to generate single-bit or double-bit ECC errors in all instances of a given
 * memory type. Encodings are as follows.
 * 0x0 = No error.
 * 0x1 = Single-bit error on ECC<0>.
 * 0x2 = Single-bit error on ECC<1>.
 * 0x3 = Double-bit error on ECC<1:0>.
 *
 * L2DFLIP allows software to generate L2DSBE, L2DDBE, VBFSBE, and VBFDBE errors for the purposes
 * of testing error handling code. When one (or both) of these bits are set, a PL2 that misses in
 * the L2 will fill with the appropriate error in the first two OWs of the fill. Software can
 * determine which OW pair gets the error by choosing the desired fill order (address<6:5>). A
 * PL2 that hits in the L2 will not inject any errors. Therefore sending a WBIL2 prior to the PL2
 * is recommended to make a miss likely. (If multiple processors are involved, software must be
 * sure that no other processor or I/O device can bring the block into the L2).
 *
 * To generate a VBFSBE or VBFDBE, software must first get the cache block into the cache with an
 * error using a PL2 that misses the L2. Then a store partial to a portion of the cache block
 * without the error must change the block to dirty. Then, a subsequent WBL2/WBIL2/victim will
 * trigger the VBFSBE/VBFDBE error.
 */
union cvmx_l2c_ecc_ctl {
	uint64_t u64;
	struct cvmx_l2c_ecc_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t mibflip                      : 2;  /**< Reserved. */
	uint64_t l2dflip                      : 2;  /**< Generate an ECC error in the L2D. See note above. */
	uint64_t l2tflip                      : 2;  /**< Generate an ECC error in the L2T. */
	uint64_t rdfflip                      : 2;  /**< Generate an ECC error in RDF memory. */
	uint64_t xmdflip                      : 2;  /**< Generate an ECC error in all corresponding CBC XMD memories. */
	uint64_t ioccmdflip                   : 2;  /**< Generate an ECC error in all corresponding IOCCMD memories. */
#else
	uint64_t ioccmdflip                   : 2;
	uint64_t xmdflip                      : 2;
	uint64_t rdfflip                      : 2;
	uint64_t l2tflip                      : 2;
	uint64_t l2dflip                      : 2;
	uint64_t mibflip                      : 2;
	uint64_t reserved_12_63               : 52;
#endif
	} s;
	struct cvmx_l2c_ecc_ctl_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t l2dflip                      : 2;  /**< Generate an ECC error in the L2D. See note above. */
	uint64_t l2tflip                      : 2;  /**< Generate an ECC error in the L2T. */
	uint64_t rdfflip                      : 2;  /**< Generate an ECC error in RDF memory. */
	uint64_t xmdflip                      : 2;  /**< Generate an ECC error in all corresponding CBC XMD memories. */
	uint64_t ioccmdflip                   : 2;  /**< Generate an ECC error in all corresponding IOCCMD memories. */
#else
	uint64_t ioccmdflip                   : 2;
	uint64_t xmdflip                      : 2;
	uint64_t rdfflip                      : 2;
	uint64_t l2tflip                      : 2;
	uint64_t l2dflip                      : 2;
	uint64_t reserved_10_63               : 54;
#endif
	} cn70xx;
	struct cvmx_l2c_ecc_ctl_cn70xx        cn70xxp1;
	struct cvmx_l2c_ecc_ctl_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t mibflip                      : 2;  /**< Reserved. */
	uint64_t l2dflip                      : 2;  /**< Generate an ECC error in the L2D. See note above. */
	uint64_t l2tflip                      : 2;  /**< Generate an ECC error in the L2T. */
	uint64_t rdfflip                      : 2;  /**< Generate an ECC error in RDF memory. */
	uint64_t xmdflip                      : 2;  /**< Generate an ECC error in all corresponding CBC XMD memories. */
	uint64_t reserved_0_1                 : 2;
#else
	uint64_t reserved_0_1                 : 2;
	uint64_t xmdflip                      : 2;
	uint64_t rdfflip                      : 2;
	uint64_t l2tflip                      : 2;
	uint64_t l2dflip                      : 2;
	uint64_t mibflip                      : 2;
	uint64_t reserved_12_63               : 52;
#endif
	} cn73xx;
	struct cvmx_l2c_ecc_ctl_cn73xx        cn78xx;
	struct cvmx_l2c_ecc_ctl_cn73xx        cn78xxp1;
	struct cvmx_l2c_ecc_ctl_cn73xx        cnf75xx;
};
typedef union cvmx_l2c_ecc_ctl cvmx_l2c_ecc_ctl_t;

/**
 * cvmx_l2c_err_tdt#
 *
 * L2C_ERR_TDT = L2C TAD DaTa Error Info
 *
 *
 * Notes:
 * (1) If the status bit corresponding to the value of the TYPE field is not set the WAYIDX/SYN fields
 *     are not associated with the errors currently logged by the status bits and should be ignored.
 *     This can occur, for example, because of a race between a write to clear a DBE and a new, lower
 *     priority, SBE error occuring.  If the SBE arrives prior to the DBE clear the WAYIDX/SYN fields
 *     will still be locked, but the new SBE error status bit will still be set.
 *
 * (2) The four types of errors have differing priorities.  Priority (from lowest to highest) is SBE,
 *     VSBE, DBE, VDBE.  A error will lock the WAYIDX, and SYN fields for other errors of equal or
 *     lower priority until cleared by software.  This means that the error information is always
 *     (assuming the TYPE field matches) for the highest priority error logged in the status bits.
 *
 * (3) If VSBE or VDBE are set (and the TYPE field matches), the WAYIDX fields are valid and the
 *     syndrome can be found in L2C_ERR_VBF.
 *
 * (4) The syndrome is recorded for DBE errors, though the utility of the value is not clear.
 */
union cvmx_l2c_err_tdtx {
	uint64_t u64;
	struct cvmx_l2c_err_tdtx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dbe                          : 1;  /**< L2D Double-Bit error has occurred */
	uint64_t sbe                          : 1;  /**< L2D Single-Bit error has occurred */
	uint64_t vdbe                         : 1;  /**< VBF Double-Bit error has occurred */
	uint64_t vsbe                         : 1;  /**< VBF Single-Bit error has occurred */
	uint64_t syn                          : 10; /**< L2D syndrome (valid only for SBE/DBE, not VSBE/VDBE) */
	uint64_t reserved_22_49               : 28;
	uint64_t wayidx                       : 18; /**< Way, index, OW of the L2 block containing the error */
	uint64_t reserved_2_3                 : 2;
	uint64_t type                         : 2;  /**< The type of error the WAYIDX,SYN were latched for.
                                                         0 - VSBE
                                                         1 - VDBE
                                                         2 - SBE
                                                         3 - DBE */
#else
	uint64_t type                         : 2;
	uint64_t reserved_2_3                 : 2;
	uint64_t wayidx                       : 18;
	uint64_t reserved_22_49               : 28;
	uint64_t syn                          : 10;
	uint64_t vsbe                         : 1;
	uint64_t vdbe                         : 1;
	uint64_t sbe                          : 1;
	uint64_t dbe                          : 1;
#endif
	} s;
	struct cvmx_l2c_err_tdtx_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dbe                          : 1;  /**< L2D Double-Bit error has occurred */
	uint64_t sbe                          : 1;  /**< L2D Single-Bit error has occurred */
	uint64_t vdbe                         : 1;  /**< VBF Double-Bit error has occurred */
	uint64_t vsbe                         : 1;  /**< VBF Single-Bit error has occurred */
	uint64_t syn                          : 10; /**< L2D syndrome (valid only for SBE/DBE, not VSBE/VDBE) */
	uint64_t reserved_20_49               : 30;
	uint64_t wayidx                       : 16; /**< Way, index, OW of the L2 block containing the error */
	uint64_t reserved_2_3                 : 2;
	uint64_t type                         : 2;  /**< The type of error the WAYIDX,SYN were latched for.
                                                         0 - VSBE
                                                         1 - VDBE
                                                         2 - SBE
                                                         3 - DBE */
#else
	uint64_t type                         : 2;
	uint64_t reserved_2_3                 : 2;
	uint64_t wayidx                       : 16;
	uint64_t reserved_20_49               : 30;
	uint64_t syn                          : 10;
	uint64_t vsbe                         : 1;
	uint64_t vdbe                         : 1;
	uint64_t sbe                          : 1;
	uint64_t dbe                          : 1;
#endif
	} cn61xx;
	struct cvmx_l2c_err_tdtx_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dbe                          : 1;  /**< L2D Double-Bit error has occurred */
	uint64_t sbe                          : 1;  /**< L2D Single-Bit error has occurred */
	uint64_t vdbe                         : 1;  /**< VBF Double-Bit error has occurred */
	uint64_t vsbe                         : 1;  /**< VBF Single-Bit error has occurred */
	uint64_t syn                          : 10; /**< L2D syndrome (valid only for SBE/DBE, not VSBE/VDBE) */
	uint64_t reserved_21_49               : 29;
	uint64_t wayidx                       : 17; /**< Way, index, OW of the L2 block containing the error */
	uint64_t reserved_2_3                 : 2;
	uint64_t type                         : 2;  /**< The type of error the WAYIDX,SYN were latched for.
                                                         0 - VSBE
                                                         1 - VDBE
                                                         2 - SBE
                                                         3 - DBE */
#else
	uint64_t type                         : 2;
	uint64_t reserved_2_3                 : 2;
	uint64_t wayidx                       : 17;
	uint64_t reserved_21_49               : 29;
	uint64_t syn                          : 10;
	uint64_t vsbe                         : 1;
	uint64_t vdbe                         : 1;
	uint64_t sbe                          : 1;
	uint64_t dbe                          : 1;
#endif
	} cn63xx;
	struct cvmx_l2c_err_tdtx_cn63xx       cn63xxp1;
	struct cvmx_l2c_err_tdtx_cn63xx       cn66xx;
	struct cvmx_l2c_err_tdtx_s            cn68xx;
	struct cvmx_l2c_err_tdtx_s            cn68xxp1;
	struct cvmx_l2c_err_tdtx_cn61xx       cnf71xx;
};
typedef union cvmx_l2c_err_tdtx cvmx_l2c_err_tdtx_t;

/**
 * cvmx_l2c_err_ttg#
 *
 * L2C_ERR_TTG = L2C TAD TaG Error Info
 *
 *
 * Notes:
 * (1) The priority of errors (highest to lowest) is DBE, SBE, NOWAY.  An error will lock the SYN, and
 *     WAYIDX fields for equal or lower priority errors until cleared by software.
 *
 * (2) The syndrome is recorded for DBE errors, though the utility of the value is not clear.
 *
 * (3) A NOWAY error does not change the value of the SYN field, and leaves WAYIDX[20:17]
 *     unpredictable.  WAYIDX[16:7] is the L2 block index associated with the command which had no way
 *     to allocate.
 *
 * (4) If the status bit corresponding to the value of the TYPE field is not set the WAYIDX/SYN fields
 *     are not associated with the errors currently logged by the status bits and should be ignored.
 *     This can occur, for example, because of a race between a write to clear a DBE and a new, lower
 *     priority, SBE error occuring.  If the SBE arrives prior to the DBE clear the WAYIDX/SYN fields
 *     will still be locked, but the new SBE error status bit will still be set.
 */
union cvmx_l2c_err_ttgx {
	uint64_t u64;
	struct cvmx_l2c_err_ttgx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dbe                          : 1;  /**< Double-Bit ECC error */
	uint64_t sbe                          : 1;  /**< Single-Bit ECC error */
	uint64_t noway                        : 1;  /**< No way was available for allocation.
                                                         L2C sets NOWAY during its processing of a
                                                         transaction whenever it needed/wanted to allocate
                                                         a WAY in the L2 cache, but was unable to. NOWAY==1
                                                         is (generally) not an indication that L2C failed to
                                                         complete transactions. Rather, it is a hint of
                                                         possible performance degradation. (For example, L2C
                                                         must read-modify-write DRAM for every transaction
                                                         that updates some, but not all, of the bytes in a
                                                         cache block, misses in the L2 cache, and cannot
                                                         allocate a WAY.) There is one "failure" case where
                                                         L2C will set NOWAY: when it cannot leave a block
                                                         locked in the L2 cache as part of a LCKL2
                                                         transaction. */
	uint64_t reserved_56_60               : 5;
	uint64_t syn                          : 6;  /**< Syndrome for the single-bit error */
	uint64_t reserved_22_49               : 28;
	uint64_t wayidx                       : 15; /**< Way and index of the L2 block containing the error */
	uint64_t reserved_2_6                 : 5;
	uint64_t type                         : 2;  /**< The type of error the WAYIDX,SYN were latched for.
                                                         0 - not valid
                                                         1 - NOWAY
                                                         2 - SBE
                                                         3 - DBE */
#else
	uint64_t type                         : 2;
	uint64_t reserved_2_6                 : 5;
	uint64_t wayidx                       : 15;
	uint64_t reserved_22_49               : 28;
	uint64_t syn                          : 6;
	uint64_t reserved_56_60               : 5;
	uint64_t noway                        : 1;
	uint64_t sbe                          : 1;
	uint64_t dbe                          : 1;
#endif
	} s;
	struct cvmx_l2c_err_ttgx_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dbe                          : 1;  /**< Double-Bit ECC error */
	uint64_t sbe                          : 1;  /**< Single-Bit ECC error */
	uint64_t noway                        : 1;  /**< No way was available for allocation.
                                                         L2C sets NOWAY during its processing of a
                                                         transaction whenever it needed/wanted to allocate
                                                         a WAY in the L2 cache, but was unable to. NOWAY==1
                                                         is (generally) not an indication that L2C failed to
                                                         complete transactions. Rather, it is a hint of
                                                         possible performance degradation. (For example, L2C
                                                         must read-modify-write DRAM for every transaction
                                                         that updates some, but not all, of the bytes in a
                                                         cache block, misses in the L2 cache, and cannot
                                                         allocate a WAY.) There is one "failure" case where
                                                         L2C will set NOWAY: when it cannot leave a block
                                                         locked in the L2 cache as part of a LCKL2
                                                         transaction. */
	uint64_t reserved_56_60               : 5;
	uint64_t syn                          : 6;  /**< Syndrome for the single-bit error */
	uint64_t reserved_20_49               : 30;
	uint64_t wayidx                       : 13; /**< Way and index of the L2 block containing the error */
	uint64_t reserved_2_6                 : 5;
	uint64_t type                         : 2;  /**< The type of error the WAYIDX,SYN were latched for.
                                                         0 - not valid
                                                         1 - NOWAY
                                                         2 - SBE
                                                         3 - DBE */
#else
	uint64_t type                         : 2;
	uint64_t reserved_2_6                 : 5;
	uint64_t wayidx                       : 13;
	uint64_t reserved_20_49               : 30;
	uint64_t syn                          : 6;
	uint64_t reserved_56_60               : 5;
	uint64_t noway                        : 1;
	uint64_t sbe                          : 1;
	uint64_t dbe                          : 1;
#endif
	} cn61xx;
	struct cvmx_l2c_err_ttgx_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t dbe                          : 1;  /**< Double-Bit ECC error */
	uint64_t sbe                          : 1;  /**< Single-Bit ECC error */
	uint64_t noway                        : 1;  /**< No way was available for allocation.
                                                         L2C sets NOWAY during its processing of a
                                                         transaction whenever it needed/wanted to allocate
                                                         a WAY in the L2 cache, but was unable to. NOWAY==1
                                                         is (generally) not an indication that L2C failed to
                                                         complete transactions. Rather, it is a hint of
                                                         possible performance degradation. (For example, L2C
                                                         must read-modify-write DRAM for every transaction
                                                         that updates some, but not all, of the bytes in a
                                                         cache block, misses in the L2 cache, and cannot
                                                         allocate a WAY.) There is one "failure" case where
                                                         L2C will set NOWAY: when it cannot leave a block
                                                         locked in the L2 cache as part of a LCKL2
                                                         transaction. */
	uint64_t reserved_56_60               : 5;
	uint64_t syn                          : 6;  /**< Syndrome for the single-bit error */
	uint64_t reserved_21_49               : 29;
	uint64_t wayidx                       : 14; /**< Way and index of the L2 block containing the error */
	uint64_t reserved_2_6                 : 5;
	uint64_t type                         : 2;  /**< The type of error the WAYIDX,SYN were latched for.
                                                         0 - not valid
                                                         1 - NOWAY
                                                         2 - SBE
                                                         3 - DBE */
#else
	uint64_t type                         : 2;
	uint64_t reserved_2_6                 : 5;
	uint64_t wayidx                       : 14;
	uint64_t reserved_21_49               : 29;
	uint64_t syn                          : 6;
	uint64_t reserved_56_60               : 5;
	uint64_t noway                        : 1;
	uint64_t sbe                          : 1;
	uint64_t dbe                          : 1;
#endif
	} cn63xx;
	struct cvmx_l2c_err_ttgx_cn63xx       cn63xxp1;
	struct cvmx_l2c_err_ttgx_cn63xx       cn66xx;
	struct cvmx_l2c_err_ttgx_s            cn68xx;
	struct cvmx_l2c_err_ttgx_s            cn68xxp1;
	struct cvmx_l2c_err_ttgx_cn61xx       cnf71xx;
};
typedef union cvmx_l2c_err_ttgx cvmx_l2c_err_ttgx_t;

/**
 * cvmx_l2c_err_vbf#
 *
 * L2C_ERR_VBF = L2C VBF Error Info
 *
 *
 * Notes:
 * (1) The way/index information is stored in L2C_ERR_TDT, assuming no later interrupt occurred to
 *     overwrite the information.  See the notes associated with L2C_ERR_TDT for full details.
 *
 * (2) The first VSBE will lock the register for other VSBE's.  A VDBE, however, will overwrite a
 *     previously logged VSBE.  Once a VDBE has been logged all later errors will not be logged.  This
 *     means that if VDBE is set the information in the register is for the VDBE, if VDBE is clear and
 *     VSBE is set the register contains information about the VSBE.
 *
 * (3) The syndrome is recorded for VDBE errors, though the utility of the value is not clear.
 *
 * (4) If the status bit corresponding to the value of the TYPE field is not set the SYN field is not
 *     associated with the errors currently logged by the status bits and should be ignored.  This can
 *     occur, for example, because of a race between a write to clear a VDBE and a new, lower priority,
 *     VSBE error occuring.  If the VSBE arrives prior to the VDBE clear the SYN field will still be
 *     locked, but the new VSBE error status bit will still be set.
 */
union cvmx_l2c_err_vbfx {
	uint64_t u64;
	struct cvmx_l2c_err_vbfx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_62_63               : 2;
	uint64_t vdbe                         : 1;  /**< VBF Double-Bit error has occurred */
	uint64_t vsbe                         : 1;  /**< VBF Single-Bit error has occurred */
	uint64_t vsyn                         : 10; /**< VBF syndrome (valid only if VSBE/VDBE is set) */
	uint64_t reserved_2_49                : 48;
	uint64_t type                         : 2;  /**< The type of error the SYN were latched for.
                                                         0 - VSBE
                                                         1 - VDBE */
#else
	uint64_t type                         : 2;
	uint64_t reserved_2_49                : 48;
	uint64_t vsyn                         : 10;
	uint64_t vsbe                         : 1;
	uint64_t vdbe                         : 1;
	uint64_t reserved_62_63               : 2;
#endif
	} s;
	struct cvmx_l2c_err_vbfx_s            cn61xx;
	struct cvmx_l2c_err_vbfx_s            cn63xx;
	struct cvmx_l2c_err_vbfx_s            cn63xxp1;
	struct cvmx_l2c_err_vbfx_s            cn66xx;
	struct cvmx_l2c_err_vbfx_s            cn68xx;
	struct cvmx_l2c_err_vbfx_s            cn68xxp1;
	struct cvmx_l2c_err_vbfx_s            cnf71xx;
};
typedef union cvmx_l2c_err_vbfx cvmx_l2c_err_vbfx_t;

/**
 * cvmx_l2c_err_xmc
 *
 * L2C_ERR_XMC = L2C XMC request error
 *
 * Description: records error information for HOLE*, BIG* and VRT* interrupts.
 *
 * Notes:
 * (1) The first BIGWR/HOLEWR/VRT* interrupt will lock the register until L2C_INT_REG[6:1] are
 *     cleared.
 *
 * (2) ADDR<15:0> will always be zero for VRT* interrupts.
 *
 * (3) ADDR is the 38-bit OCTEON physical address after hole removal. (The hole is between DR0
 *     and DR1. Remove the hole by subtracting 256MB from all 38-bit OCTEON L2/DRAM physical addresses
 *     >= 512 MB.)
 *
 * (4) For 63xx pass 2.0 and all 68xx ADDR<15:0> will ALWAYS be zero.
 */
union cvmx_l2c_err_xmc {
	uint64_t u64;
	struct cvmx_l2c_err_xmc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t cmd                          : 6;  /**< XMC command or request causing error
                                                         TRA_FILT_CMD indicates encoding */
	uint64_t reserved_54_57               : 4;
	uint64_t sid                          : 6;  /**< XMC sid of request causing error
                                                         0-3      => selects a cnMIPS core
                                                         else if a STF/STT/STP:
                                                          8,9     => FPA, TIM, HFA, USB, PCIe, MIX, DPI,
                                                                     BOOTDMA, ZIP, RAD, or POW
                                                          0xA     => IOB DWB engine
                                                          0xC,0xD => PIP/IPD
                                                         else (if a read/fill):
                                                          8       => FPA, TIM, HFA, USB, PCIe, MIX, DPI,
                                                                     BOOTDMA, ZIP, RAD, or POW
                                                          9       => PKO */
	uint64_t reserved_38_47               : 10;
	uint64_t addr                         : 38; /**< XMC address causing the error (see Notes 2 and 3) */
#else
	uint64_t addr                         : 38;
	uint64_t reserved_38_47               : 10;
	uint64_t sid                          : 6;
	uint64_t reserved_54_57               : 4;
	uint64_t cmd                          : 6;
#endif
	} s;
	struct cvmx_l2c_err_xmc_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t cmd                          : 6;  /**< XMC command or request causing error
                                                         TRA_FILT_CMD indicates encoding */
	uint64_t reserved_52_57               : 6;
	uint64_t sid                          : 4;  /**< XMC sid of request causing error
                                                         0-3      => selects a cnMIPS core
                                                         else if a STF/STT/STP:
                                                          8,9     => FPA, TIM, HFA, USB, PCIe, MIX, DPI,
                                                                     BOOTDMA, ZIP, RAD, or POW
                                                          0xA     => IOB DWB engine
                                                          0xC,0xD => PIP/IPD
                                                         else (if a read/fill):
                                                          8       => FPA, TIM, HFA, USB, PCIe, MIX, DPI,
                                                                     BOOTDMA, ZIP, RAD, or POW
                                                          9       => PKO */
	uint64_t reserved_38_47               : 10;
	uint64_t addr                         : 38; /**< XMC address causing the error (see Notes 2 and 3) */
#else
	uint64_t addr                         : 38;
	uint64_t reserved_38_47               : 10;
	uint64_t sid                          : 4;
	uint64_t reserved_52_57               : 6;
	uint64_t cmd                          : 6;
#endif
	} cn61xx;
	struct cvmx_l2c_err_xmc_cn61xx        cn63xx;
	struct cvmx_l2c_err_xmc_cn61xx        cn63xxp1;
	struct cvmx_l2c_err_xmc_cn66xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t cmd                          : 6;  /**< XMC command or request causing error
                                                         TRA_FILT_CMD indicates encoding */
	uint64_t reserved_53_57               : 5;
	uint64_t sid                          : 5;  /**< XMC sid of request causing error
                                                         0-9      => selects a cnMIPS core
                                                         else if a STF/STT/STT:
                                                          0x10,0x11 => FPA, TIM, HFA, USB, PCIe, MIX, DPI,
                                                                       BOOTDMA, ZIP, RAD, or POW
                                                          0x12      => IOB DWB engine
                                                          0x14,0x15 => PIP/IPD
                                                         else (if a read/fill):
                                                          0x10      => FPA, TIM, HFA, USB, PCIe, MIX, DPI,
                                                                       BOOTDMA, ZIP, RAD, or POW
                                                          0x11      => PKO */
	uint64_t reserved_38_47               : 10;
	uint64_t addr                         : 38; /**< XMC address causing the error (see Notes 2 and 3) */
#else
	uint64_t addr                         : 38;
	uint64_t reserved_38_47               : 10;
	uint64_t sid                          : 5;
	uint64_t reserved_53_57               : 5;
	uint64_t cmd                          : 6;
#endif
	} cn66xx;
	struct cvmx_l2c_err_xmc_s             cn68xx;
	struct cvmx_l2c_err_xmc_s             cn68xxp1;
	struct cvmx_l2c_err_xmc_cn61xx        cnf71xx;
};
typedef union cvmx_l2c_err_xmc cvmx_l2c_err_xmc_t;

/**
 * cvmx_l2c_grpwrr0
 *
 * L2C_GRPWRR0 = L2C PP Weighted Round \#0 Register
 *
 * Description: Defines Weighted rounds(32) for Group PLC0,PLC1
 *
 * Notes:
 * - Starvation of a group 'could' occur, unless SW takes the precaution to ensure that each GROUP
 * participates in at least 1(of 32) rounds (ie: At least 1 bit(of 32) should be clear).
 */
union cvmx_l2c_grpwrr0 {
	uint64_t u64;
	struct cvmx_l2c_grpwrr0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t plc1rmsk                     : 32; /**< PLC1 Group#1 Weighted Round Mask
                                                         Each bit represents 1 of 32 rounds
                                                         for Group \#1's participation. When a 'round' bit is
                                                         set, Group#1 is 'masked' and DOES NOT participate.
                                                         When a 'round' bit is clear, Group#1 WILL
                                                         participate in the arbitration for this round. */
	uint64_t plc0rmsk                     : 32; /**< PLC Group#0 Weighted Round Mask
                                                         Each bit represents 1 of 32 rounds
                                                         for Group \#0's participation. When a 'round' bit is
                                                         set, Group#0 is 'masked' and DOES NOT participate.
                                                         When a 'round' bit is clear, Group#0 WILL
                                                         participate in the arbitration for this round. */
#else
	uint64_t plc0rmsk                     : 32;
	uint64_t plc1rmsk                     : 32;
#endif
	} s;
	struct cvmx_l2c_grpwrr0_s             cn52xx;
	struct cvmx_l2c_grpwrr0_s             cn52xxp1;
	struct cvmx_l2c_grpwrr0_s             cn56xx;
	struct cvmx_l2c_grpwrr0_s             cn56xxp1;
};
typedef union cvmx_l2c_grpwrr0 cvmx_l2c_grpwrr0_t;

/**
 * cvmx_l2c_grpwrr1
 *
 * L2C_GRPWRR1 = L2C PP Weighted Round \#1 Register
 *
 * Description: Defines Weighted Rounds(32) for Group PLC2,ILC
 *
 * Notes:
 * - Starvation of a group 'could' occur, unless SW takes the precaution to ensure that each GROUP
 * participates in at least 1(of 32) rounds (ie: At least 1 bit(of 32) should be clear).
 */
union cvmx_l2c_grpwrr1 {
	uint64_t u64;
	struct cvmx_l2c_grpwrr1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ilcrmsk                      : 32; /**< ILC (IOB) Weighted Round Mask
                                                         Each bit represents 1 of 32 rounds
                                                         for IOB participation. When a 'round' bit is
                                                         set, IOB is 'masked' and DOES NOT participate.
                                                         When a 'round' bit is clear, IOB WILL
                                                         participate in the arbitration for this round. */
	uint64_t plc2rmsk                     : 32; /**< PLC Group#2 Weighted Round Mask
                                                         Each bit represents 1 of 32 rounds
                                                         for Group \#2's participation. When a 'round' bit is
                                                         set, Group#2 is 'masked' and DOES NOT participate.
                                                         When a 'round' bit is clear, Group#2 WILL
                                                         participate in the arbitration for this round. */
#else
	uint64_t plc2rmsk                     : 32;
	uint64_t ilcrmsk                      : 32;
#endif
	} s;
	struct cvmx_l2c_grpwrr1_s             cn52xx;
	struct cvmx_l2c_grpwrr1_s             cn52xxp1;
	struct cvmx_l2c_grpwrr1_s             cn56xx;
	struct cvmx_l2c_grpwrr1_s             cn56xxp1;
};
typedef union cvmx_l2c_grpwrr1 cvmx_l2c_grpwrr1_t;

/**
 * cvmx_l2c_int_en
 *
 * L2C_INT_EN = L2C Global Interrupt Enable Register
 *
 * Description:
 */
union cvmx_l2c_int_en {
	uint64_t u64;
	struct cvmx_l2c_int_en_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t lck2ena                      : 1;  /**< L2 Tag Lock Error2 Interrupt Enable bit
                                                         NOTE: This is the 'same' bit as L2T_ERR[LCK_INTENA2] */
	uint64_t lckena                       : 1;  /**< L2 Tag Lock Error Interrupt Enable bit
                                                         NOTE: This is the 'same' bit as L2T_ERR[LCK_INTENA] */
	uint64_t l2ddeden                     : 1;  /**< L2 Data ECC Double Error Detect(DED) Interrupt Enable bit
                                                         When set, allows interrupts to be reported on double bit
                                                         (uncorrectable) errors from the L2 Data Arrays.
                                                         NOTE: This is the 'same' bit as L2D_ERR[DED_INTENA] */
	uint64_t l2dsecen                     : 1;  /**< L2 Data ECC Single Error Correct(SEC) Interrupt Enable bit
                                                         When set, allows interrupts to be reported on single bit
                                                         (correctable) errors from the L2 Data Arrays.
                                                         NOTE: This is the 'same' bit as L2D_ERR[SEC_INTENA] */
	uint64_t l2tdeden                     : 1;  /**< L2 Tag ECC Double Error Detect(DED) Interrupt
                                                         NOTE: This is the 'same' bit as L2T_ERR[DED_INTENA] */
	uint64_t l2tsecen                     : 1;  /**< L2 Tag ECC Single Error Correct(SEC) Interrupt
                                                         Enable bit. When set, allows interrupts to be
                                                         reported on single bit (correctable) errors from
                                                         the L2 Tag Arrays.
                                                         NOTE: This is the 'same' bit as L2T_ERR[SEC_INTENA] */
	uint64_t oob3en                       : 1;  /**< DMA Out of Bounds Interrupt Enable Range#3 */
	uint64_t oob2en                       : 1;  /**< DMA Out of Bounds Interrupt Enable Range#2 */
	uint64_t oob1en                       : 1;  /**< DMA Out of Bounds Interrupt Enable Range#1 */
#else
	uint64_t oob1en                       : 1;
	uint64_t oob2en                       : 1;
	uint64_t oob3en                       : 1;
	uint64_t l2tsecen                     : 1;
	uint64_t l2tdeden                     : 1;
	uint64_t l2dsecen                     : 1;
	uint64_t l2ddeden                     : 1;
	uint64_t lckena                       : 1;
	uint64_t lck2ena                      : 1;
	uint64_t reserved_9_63                : 55;
#endif
	} s;
	struct cvmx_l2c_int_en_s              cn52xx;
	struct cvmx_l2c_int_en_s              cn52xxp1;
	struct cvmx_l2c_int_en_s              cn56xx;
	struct cvmx_l2c_int_en_s              cn56xxp1;
};
typedef union cvmx_l2c_int_en cvmx_l2c_int_en_t;

/**
 * cvmx_l2c_int_ena
 *
 * L2C_INT_ENA = L2C Interrupt Enable
 *
 */
union cvmx_l2c_int_ena {
	uint64_t u64;
	struct cvmx_l2c_int_ena_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t bigrd                        : 1;  /**< Read reference past MAXDRAM enable */
	uint64_t bigwr                        : 1;  /**< Write reference past MAXDRAM enable */
	uint64_t vrtpe                        : 1;  /**< Virtualization memory parity error */
	uint64_t vrtadrng                     : 1;  /**< Address outside of virtualization range enable */
	uint64_t vrtidrng                     : 1;  /**< Virtualization ID out of range enable */
	uint64_t vrtwr                        : 1;  /**< Virtualization ID prevented a write enable */
	uint64_t holewr                       : 1;  /**< Write reference to 256MB hole enable */
	uint64_t holerd                       : 1;  /**< Read reference to 256MB hole enable */
#else
	uint64_t holerd                       : 1;
	uint64_t holewr                       : 1;
	uint64_t vrtwr                        : 1;
	uint64_t vrtidrng                     : 1;
	uint64_t vrtadrng                     : 1;
	uint64_t vrtpe                        : 1;
	uint64_t bigwr                        : 1;
	uint64_t bigrd                        : 1;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_l2c_int_ena_s             cn61xx;
	struct cvmx_l2c_int_ena_s             cn63xx;
	struct cvmx_l2c_int_ena_cn63xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t vrtpe                        : 1;  /**< Virtualization memory parity error */
	uint64_t vrtadrng                     : 1;  /**< Address outside of virtualization range enable */
	uint64_t vrtidrng                     : 1;  /**< Virtualization ID out of range enable */
	uint64_t vrtwr                        : 1;  /**< Virtualization ID prevented a write enable */
	uint64_t holewr                       : 1;  /**< Write reference to 256MB hole enable */
	uint64_t holerd                       : 1;  /**< Read reference to 256MB hole enable */
#else
	uint64_t holerd                       : 1;
	uint64_t holewr                       : 1;
	uint64_t vrtwr                        : 1;
	uint64_t vrtidrng                     : 1;
	uint64_t vrtadrng                     : 1;
	uint64_t vrtpe                        : 1;
	uint64_t reserved_6_63                : 58;
#endif
	} cn63xxp1;
	struct cvmx_l2c_int_ena_s             cn66xx;
	struct cvmx_l2c_int_ena_s             cn68xx;
	struct cvmx_l2c_int_ena_s             cn68xxp1;
	struct cvmx_l2c_int_ena_s             cnf71xx;
};
typedef union cvmx_l2c_int_ena cvmx_l2c_int_ena_t;

/**
 * cvmx_l2c_int_reg
 *
 * L2C_INT_REG = L2C Interrupt Register
 *
 */
union cvmx_l2c_int_reg {
	uint64_t u64;
	struct cvmx_l2c_int_reg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t tad3                         : 1;  /**< When set, the enabled interrupt is in
                                                         the L2C_TAD3_INT CSR */
	uint64_t tad2                         : 1;  /**< When set, the enabled interrupt is in
                                                         the L2C_TAD2_INT CSR */
	uint64_t tad1                         : 1;  /**< When set, the enabled interrupt is in
                                                         the L2C_TAD1_INT CSR */
	uint64_t tad0                         : 1;  /**< When set, the enabled interrupt is in
                                                         the L2C_TAD0_INT CSR */
	uint64_t reserved_8_15                : 8;
	uint64_t bigrd                        : 1;  /**< Read reference past L2C_BIG_CTL[MAXDRAM] occurred */
	uint64_t bigwr                        : 1;  /**< Write reference past L2C_BIG_CTL[MAXDRAM] occurred */
	uint64_t vrtpe                        : 1;  /**< L2C_VRT_MEM read found a parity error
                                                         Whenever an L2C_VRT_MEM read finds a parity error,
                                                         that L2C_VRT_MEM cannot cause stores to be blocked.
                                                         Software should correct the error. */
	uint64_t vrtadrng                     : 1;  /**< Address outside of virtualization range
                                                         Set when a L2C_VRT_CTL[MEMSZ] violation blocked a
                                                         store.
                                                         L2C_VRT_CTL[OOBERR] must be set for L2C to set this. */
	uint64_t vrtidrng                     : 1;  /**< Virtualization ID out of range
                                                         Set when a L2C_VRT_CTL[NUMID] violation blocked a
                                                         store. */
	uint64_t vrtwr                        : 1;  /**< Virtualization ID prevented a write
                                                         Set when L2C_VRT_MEM blocked a store. */
	uint64_t holewr                       : 1;  /**< Write reference to 256MB hole occurred */
	uint64_t holerd                       : 1;  /**< Read reference to 256MB hole occurred */
#else
	uint64_t holerd                       : 1;
	uint64_t holewr                       : 1;
	uint64_t vrtwr                        : 1;
	uint64_t vrtidrng                     : 1;
	uint64_t vrtadrng                     : 1;
	uint64_t vrtpe                        : 1;
	uint64_t bigwr                        : 1;
	uint64_t bigrd                        : 1;
	uint64_t reserved_8_15                : 8;
	uint64_t tad0                         : 1;
	uint64_t tad1                         : 1;
	uint64_t tad2                         : 1;
	uint64_t tad3                         : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_l2c_int_reg_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_17_63               : 47;
	uint64_t tad0                         : 1;  /**< When set, the enabled interrupt is in
                                                         the L2C_TAD0_INT CSR */
	uint64_t reserved_8_15                : 8;
	uint64_t bigrd                        : 1;  /**< Read reference past L2C_BIG_CTL[MAXDRAM] occurred */
	uint64_t bigwr                        : 1;  /**< Write reference past L2C_BIG_CTL[MAXDRAM] occurred */
	uint64_t vrtpe                        : 1;  /**< L2C_VRT_MEM read found a parity error
                                                         Whenever an L2C_VRT_MEM read finds a parity error,
                                                         that L2C_VRT_MEM cannot cause stores to be blocked.
                                                         Software should correct the error. */
	uint64_t vrtadrng                     : 1;  /**< Address outside of virtualization range
                                                         Set when a L2C_VRT_CTL[MEMSZ] violation blocked a
                                                         store.
                                                         L2C_VRT_CTL[OOBERR] must be set for L2C to set this. */
	uint64_t vrtidrng                     : 1;  /**< Virtualization ID out of range
                                                         Set when a L2C_VRT_CTL[NUMID] violation blocked a
                                                         store. */
	uint64_t vrtwr                        : 1;  /**< Virtualization ID prevented a write
                                                         Set when L2C_VRT_MEM blocked a store. */
	uint64_t holewr                       : 1;  /**< Write reference to 256MB hole occurred */
	uint64_t holerd                       : 1;  /**< Read reference to 256MB hole occurred */
#else
	uint64_t holerd                       : 1;
	uint64_t holewr                       : 1;
	uint64_t vrtwr                        : 1;
	uint64_t vrtidrng                     : 1;
	uint64_t vrtadrng                     : 1;
	uint64_t vrtpe                        : 1;
	uint64_t bigwr                        : 1;
	uint64_t bigrd                        : 1;
	uint64_t reserved_8_15                : 8;
	uint64_t tad0                         : 1;
	uint64_t reserved_17_63               : 47;
#endif
	} cn61xx;
	struct cvmx_l2c_int_reg_cn61xx        cn63xx;
	struct cvmx_l2c_int_reg_cn63xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_17_63               : 47;
	uint64_t tad0                         : 1;  /**< When set, the enabled interrupt is in either
                                                         the L2C_ERR_TDT0 or L2C_ERR_TTG0 CSR */
	uint64_t reserved_6_15                : 10;
	uint64_t vrtpe                        : 1;  /**< L2C_VRT_MEM read found a parity error
                                                         Whenever an L2C_VRT_MEM read finds a parity error,
                                                         that L2C_VRT_MEM cannot cause stores to be blocked.
                                                         Software should correct the error. */
	uint64_t vrtadrng                     : 1;  /**< Address outside of virtualization range
                                                         Set when a L2C_VRT_CTL[MEMSZ] violation blocked a
                                                         store.
                                                         L2C_VRT_CTL[OOBERR] must be set for L2C to set this. */
	uint64_t vrtidrng                     : 1;  /**< Virtualization ID out of range
                                                         Set when a L2C_VRT_CTL[NUMID] violation blocked a
                                                         store. */
	uint64_t vrtwr                        : 1;  /**< Virtualization ID prevented a write
                                                         Set when L2C_VRT_MEM blocked a store. */
	uint64_t holewr                       : 1;  /**< Write reference to 256MB hole occurred */
	uint64_t holerd                       : 1;  /**< Read reference to 256MB hole occurred */
#else
	uint64_t holerd                       : 1;
	uint64_t holewr                       : 1;
	uint64_t vrtwr                        : 1;
	uint64_t vrtidrng                     : 1;
	uint64_t vrtadrng                     : 1;
	uint64_t vrtpe                        : 1;
	uint64_t reserved_6_15                : 10;
	uint64_t tad0                         : 1;
	uint64_t reserved_17_63               : 47;
#endif
	} cn63xxp1;
	struct cvmx_l2c_int_reg_cn61xx        cn66xx;
	struct cvmx_l2c_int_reg_s             cn68xx;
	struct cvmx_l2c_int_reg_s             cn68xxp1;
	struct cvmx_l2c_int_reg_cn61xx        cnf71xx;
};
typedef union cvmx_l2c_int_reg cvmx_l2c_int_reg_t;

/**
 * cvmx_l2c_int_stat
 *
 * L2C_INT_STAT = L2C Global Interrupt Status Register
 *
 * Description:
 */
union cvmx_l2c_int_stat {
	uint64_t u64;
	struct cvmx_l2c_int_stat_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t lck2                         : 1;  /**< HW detected a case where a Rd/Wr Miss from PP#n
                                                         could not find an available/unlocked set (for
                                                         replacement).
                                                         Most likely, this is a result of SW mixing SET
                                                         PARTITIONING with ADDRESS LOCKING. If SW allows
                                                         another PP to LOCKDOWN all SETs available to PP#n,
                                                         then a Rd/Wr Miss from PP#n will be unable
                                                         to determine a 'valid' replacement set (since LOCKED
                                                         addresses should NEVER be replaced).
                                                         If such an event occurs, the HW will select the smallest
                                                         available SET(specified by UMSK'x)' as the replacement
                                                         set, and the address is unlocked.
                                                         NOTE: This is the 'same' bit as L2T_ERR[LCKERR2] */
	uint64_t lck                          : 1;  /**< SW attempted to LOCK DOWN the last available set of
                                                         the INDEX (which is ignored by HW - but reported to SW).
                                                         The LDD(L1 load-miss) for the LOCK operation is completed
                                                         successfully, however the address is NOT locked.
                                                         NOTE: 'Available' sets takes the L2C_SPAR*[UMSK*]
                                                         into account. For example, if diagnostic PPx has
                                                         UMSKx defined to only use SETs [1:0], and SET1 had
                                                         been previously LOCKED, then an attempt to LOCK the
                                                         last available SET0 would result in a LCKERR. (This
                                                         is to ensure that at least 1 SET at each INDEX is
                                                         not LOCKED for general use by other PPs).
                                                         NOTE: This is the 'same' bit as L2T_ERR[LCKERR] */
	uint64_t l2dded                       : 1;  /**< L2D Double Error detected (DED)
                                                         NOTE: This is the 'same' bit as L2D_ERR[DED_ERR] */
	uint64_t l2dsec                       : 1;  /**< L2D Single Error corrected (SEC)
                                                         NOTE: This is the 'same' bit as L2D_ERR[SEC_ERR] */
	uint64_t l2tded                       : 1;  /**< L2T Double Bit Error detected (DED)
                                                         During every L2 Tag Probe, all 8 sets Tag's (at a
                                                         given index) are checked for double bit errors(DBEs).
                                                         This bit is set if ANY of the 8 sets contains a DBE.
                                                         DBEs also generated an interrupt(if enabled).
                                                         NOTE: This is the 'same' bit as L2T_ERR[DED_ERR] */
	uint64_t l2tsec                       : 1;  /**< L2T Single Bit Error corrected (SEC) status
                                                         During every L2 Tag Probe, all 8 sets Tag's (at a
                                                         given index) are checked for single bit errors(SBEs).
                                                         This bit is set if ANY of the 8 sets contains an SBE.
                                                         SBEs are auto corrected in HW and generate an
                                                         interrupt(if enabled).
                                                         NOTE: This is the 'same' bit as L2T_ERR[SEC_ERR] */
	uint64_t oob3                         : 1;  /**< DMA Out of Bounds Interrupt Status Range#3 */
	uint64_t oob2                         : 1;  /**< DMA Out of Bounds Interrupt Status Range#2 */
	uint64_t oob1                         : 1;  /**< DMA Out of Bounds Interrupt Status Range#1 */
#else
	uint64_t oob1                         : 1;
	uint64_t oob2                         : 1;
	uint64_t oob3                         : 1;
	uint64_t l2tsec                       : 1;
	uint64_t l2tded                       : 1;
	uint64_t l2dsec                       : 1;
	uint64_t l2dded                       : 1;
	uint64_t lck                          : 1;
	uint64_t lck2                         : 1;
	uint64_t reserved_9_63                : 55;
#endif
	} s;
	struct cvmx_l2c_int_stat_s            cn52xx;
	struct cvmx_l2c_int_stat_s            cn52xxp1;
	struct cvmx_l2c_int_stat_s            cn56xx;
	struct cvmx_l2c_int_stat_s            cn56xxp1;
};
typedef union cvmx_l2c_int_stat cvmx_l2c_int_stat_t;

/**
 * cvmx_l2c_inv#_pfc
 */
union cvmx_l2c_invx_pfc {
	uint64_t u64;
	struct cvmx_l2c_invx_pfc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t count                        : 64; /**< Current counter value */
#else
	uint64_t count                        : 64;
#endif
	} s;
	struct cvmx_l2c_invx_pfc_s            cn70xx;
	struct cvmx_l2c_invx_pfc_s            cn70xxp1;
	struct cvmx_l2c_invx_pfc_s            cn73xx;
	struct cvmx_l2c_invx_pfc_s            cn78xx;
	struct cvmx_l2c_invx_pfc_s            cn78xxp1;
	struct cvmx_l2c_invx_pfc_s            cnf75xx;
};
typedef union cvmx_l2c_invx_pfc cvmx_l2c_invx_pfc_t;

/**
 * cvmx_l2c_ioc#_pfc
 *
 * L2C_IOC_PFC = L2C IOC Performance Counter(s)
 *
 */
union cvmx_l2c_iocx_pfc {
	uint64_t u64;
	struct cvmx_l2c_iocx_pfc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t count                        : 64; /**< Current counter value */
#else
	uint64_t count                        : 64;
#endif
	} s;
	struct cvmx_l2c_iocx_pfc_s            cn61xx;
	struct cvmx_l2c_iocx_pfc_s            cn63xx;
	struct cvmx_l2c_iocx_pfc_s            cn63xxp1;
	struct cvmx_l2c_iocx_pfc_s            cn66xx;
	struct cvmx_l2c_iocx_pfc_s            cn68xx;
	struct cvmx_l2c_iocx_pfc_s            cn68xxp1;
	struct cvmx_l2c_iocx_pfc_s            cn70xx;
	struct cvmx_l2c_iocx_pfc_s            cn70xxp1;
	struct cvmx_l2c_iocx_pfc_s            cn73xx;
	struct cvmx_l2c_iocx_pfc_s            cn78xx;
	struct cvmx_l2c_iocx_pfc_s            cn78xxp1;
	struct cvmx_l2c_iocx_pfc_s            cnf71xx;
	struct cvmx_l2c_iocx_pfc_s            cnf75xx;
};
typedef union cvmx_l2c_iocx_pfc cvmx_l2c_iocx_pfc_t;

/**
 * cvmx_l2c_ior#_pfc
 *
 * L2C_IOR_PFC = L2C IOR Performance Counter(s)
 *
 */
union cvmx_l2c_iorx_pfc {
	uint64_t u64;
	struct cvmx_l2c_iorx_pfc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t count                        : 64; /**< Current counter value */
#else
	uint64_t count                        : 64;
#endif
	} s;
	struct cvmx_l2c_iorx_pfc_s            cn61xx;
	struct cvmx_l2c_iorx_pfc_s            cn63xx;
	struct cvmx_l2c_iorx_pfc_s            cn63xxp1;
	struct cvmx_l2c_iorx_pfc_s            cn66xx;
	struct cvmx_l2c_iorx_pfc_s            cn68xx;
	struct cvmx_l2c_iorx_pfc_s            cn68xxp1;
	struct cvmx_l2c_iorx_pfc_s            cn70xx;
	struct cvmx_l2c_iorx_pfc_s            cn70xxp1;
	struct cvmx_l2c_iorx_pfc_s            cn73xx;
	struct cvmx_l2c_iorx_pfc_s            cn78xx;
	struct cvmx_l2c_iorx_pfc_s            cn78xxp1;
	struct cvmx_l2c_iorx_pfc_s            cnf71xx;
	struct cvmx_l2c_iorx_pfc_s            cnf75xx;
};
typedef union cvmx_l2c_iorx_pfc cvmx_l2c_iorx_pfc_t;

/**
 * cvmx_l2c_lckbase
 *
 * L2C_LCKBASE = L2C LockDown Base Register
 *
 * Description: L2C LockDown Base Register
 *
 * Notes:
 * (1) SW RESTRICTION \#1: SW must manage the L2 Data Store lockdown space such that at least 1
 *     set per cache line remains in the 'unlocked' (normal) state to allow general caching operations.
 *     If SW violates this restriction, a status bit is set (LCK_ERR) and an interrupt is posted.
 *     [this limits the total lockdown space to 7/8ths of the total L2 data store = 896KB]
 * (2) IOB initiated LDI commands are ignored (only PP initiated LDI/LDD commands are considered
 *     for lockdown).
 * (3) To 'unlock' a locked cache line, SW can use the FLUSH-INVAL CSR mechanism (see L2C_DBG[FINV]).
 * (4) LCK_ENA MUST only be activated when debug modes are disabled (L2C_DBG[L2T], L2C_DBG[L2D], L2C_DBG[FINV]).
 */
union cvmx_l2c_lckbase {
	uint64_t u64;
	struct cvmx_l2c_lckbase_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_31_63               : 33;
	uint64_t lck_base                     : 27; /**< Base Memory block address[33:7]. Specifies the
                                                         starting address of the lockdown region. */
	uint64_t reserved_1_3                 : 3;
	uint64_t lck_ena                      : 1;  /**< L2 Cache Lock Enable
                                                         When the LCK_ENA=1, all LDI(I-stream Load) or
                                                         LDD(L1 load-miss) commands issued from the
                                                         diagnostic PP (specified by the L2C_DBG[PPNUM]),
                                                         which fall within a predefined lockdown address
                                                         range (specified by: [lck_base:lck_base+lck_offset])
                                                         are LOCKED in the L2 cache. The LOCKED state is
                                                         denoted using an explicit L2 Tag bit (L=1).
                                                         If the LOCK request L2-Hits (on ANY SET), then data is
                                                         returned from the L2 and the hit set is updated to the
                                                         LOCKED state. NOTE: If the Hit Set# is outside the
                                                         available sets for a given PP (see UMSK'x'), the
                                                         the LOCK bit is still SET. If the programmer's intent
                                                         is to explicitly LOCK addresses into 'available' sets,
                                                         care must be taken to flush-invalidate the cache first
                                                         (to avoid such situations). Not following this procedure
                                                         can lead to LCKERR2 interrupts.
                                                         If the LOCK request L2-Misses, a replacment set is
                                                         chosen(from the available sets (UMSK'x').
                                                         If the replacement set contains a dirty-victim it is
                                                         written back to memory. Memory read data is then written
                                                         into the replacement set, and the replacment SET is
                                                         updated to the LOCKED state(L=1).
                                                         NOTE: SETs that contain LOCKED addresses are
                                                         excluded from the replacement set selection algorithm.
                                                         NOTE: The LDD command will allocate the DuTag as normal.
                                                         NOTE: If L2C_CFG[IDXALIAS]=1, the address is 'aliased' first
                                                         before being checked against the lockdown address
                                                         range. To ensure an 'aliased' address is properly locked,
                                                         it is recommmended that SW preload the 'aliased' locked adddress
                                                         into the L2C_LCKBASE[LCK_BASE] register (while keeping
                                                         L2C_LCKOFF[LCK_OFFSET]=0).
                                                         NOTE: The OCTEON(N3) implementation only supports 16GB(MAX) of
                                                         physical memory. Therefore, only byte address[33:0] are used
                                                         (ie: address[35:34] are ignored). */
#else
	uint64_t lck_ena                      : 1;
	uint64_t reserved_1_3                 : 3;
	uint64_t lck_base                     : 27;
	uint64_t reserved_31_63               : 33;
#endif
	} s;
	struct cvmx_l2c_lckbase_s             cn30xx;
	struct cvmx_l2c_lckbase_s             cn31xx;
	struct cvmx_l2c_lckbase_s             cn38xx;
	struct cvmx_l2c_lckbase_s             cn38xxp2;
	struct cvmx_l2c_lckbase_s             cn50xx;
	struct cvmx_l2c_lckbase_s             cn52xx;
	struct cvmx_l2c_lckbase_s             cn52xxp1;
	struct cvmx_l2c_lckbase_s             cn56xx;
	struct cvmx_l2c_lckbase_s             cn56xxp1;
	struct cvmx_l2c_lckbase_s             cn58xx;
	struct cvmx_l2c_lckbase_s             cn58xxp1;
};
typedef union cvmx_l2c_lckbase cvmx_l2c_lckbase_t;

/**
 * cvmx_l2c_lckoff
 *
 * L2C_LCKOFF = L2C LockDown OFFSET Register
 *
 * Description: L2C LockDown OFFSET Register
 *
 * Notes:
 * (1) The generation of the end lockdown block address will 'wrap'.
 * (2) The minimum granularity for lockdown is 1 cache line (= 128B block)
 */
union cvmx_l2c_lckoff {
	uint64_t u64;
	struct cvmx_l2c_lckoff_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t lck_offset                   : 10; /**< LockDown block Offset. Used in determining
                                                         the ending block address of the lockdown
                                                         region:
                                                         End Lockdown block Address[33:7] =
                                                         LCK_BASE[33:7]+LCK_OFFSET[9:0] */
#else
	uint64_t lck_offset                   : 10;
	uint64_t reserved_10_63               : 54;
#endif
	} s;
	struct cvmx_l2c_lckoff_s              cn30xx;
	struct cvmx_l2c_lckoff_s              cn31xx;
	struct cvmx_l2c_lckoff_s              cn38xx;
	struct cvmx_l2c_lckoff_s              cn38xxp2;
	struct cvmx_l2c_lckoff_s              cn50xx;
	struct cvmx_l2c_lckoff_s              cn52xx;
	struct cvmx_l2c_lckoff_s              cn52xxp1;
	struct cvmx_l2c_lckoff_s              cn56xx;
	struct cvmx_l2c_lckoff_s              cn56xxp1;
	struct cvmx_l2c_lckoff_s              cn58xx;
	struct cvmx_l2c_lckoff_s              cn58xxp1;
};
typedef union cvmx_l2c_lckoff cvmx_l2c_lckoff_t;

/**
 * cvmx_l2c_lfb0
 *
 * L2C_LFB0 = L2C LFB DEBUG 0 Register
 *
 * Description: L2C LFB Contents (Status Bits)
 */
union cvmx_l2c_lfb0 {
	uint64_t u64;
	struct cvmx_l2c_lfb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t stcpnd                       : 1;  /**< LFB STC Pending Status */
	uint64_t stpnd                        : 1;  /**< LFB ST* Pending Status */
	uint64_t stinv                        : 1;  /**< LFB ST* Invalidate Status */
	uint64_t stcfl                        : 1;  /**< LFB STC=FAIL Status */
	uint64_t vam                          : 1;  /**< Valid Full Address Match Status */
	uint64_t inxt                         : 4;  /**< Next LFB Pointer(invalid if ITL=1) */
	uint64_t itl                          : 1;  /**< LFB Tail of List Indicator */
	uint64_t ihd                          : 1;  /**< LFB Head of List Indicator */
	uint64_t set                          : 3;  /**< SET# used for DS-OP (hit=hset/miss=rset) */
	uint64_t vabnum                       : 4;  /**< VAB# used for LMC Miss Launch(valid only if VAM=1) */
	uint64_t sid                          : 9;  /**< LFB Source ID */
	uint64_t cmd                          : 4;  /**< LFB Command */
	uint64_t vld                          : 1;  /**< LFB Valid */
#else
	uint64_t vld                          : 1;
	uint64_t cmd                          : 4;
	uint64_t sid                          : 9;
	uint64_t vabnum                       : 4;
	uint64_t set                          : 3;
	uint64_t ihd                          : 1;
	uint64_t itl                          : 1;
	uint64_t inxt                         : 4;
	uint64_t vam                          : 1;
	uint64_t stcfl                        : 1;
	uint64_t stinv                        : 1;
	uint64_t stpnd                        : 1;
	uint64_t stcpnd                       : 1;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_l2c_lfb0_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t stcpnd                       : 1;  /**< LFB STC Pending Status */
	uint64_t stpnd                        : 1;  /**< LFB ST* Pending Status */
	uint64_t stinv                        : 1;  /**< LFB ST* Invalidate Status */
	uint64_t stcfl                        : 1;  /**< LFB STC=FAIL Status */
	uint64_t vam                          : 1;  /**< Valid Full Address Match Status */
	uint64_t reserved_25_26               : 2;
	uint64_t inxt                         : 2;  /**< Next LFB Pointer(invalid if ITL=1) */
	uint64_t itl                          : 1;  /**< LFB Tail of List Indicator */
	uint64_t ihd                          : 1;  /**< LFB Head of List Indicator */
	uint64_t reserved_20_20               : 1;
	uint64_t set                          : 2;  /**< SET# used for DS-OP (hit=hset/miss=rset) */
	uint64_t reserved_16_17               : 2;
	uint64_t vabnum                       : 2;  /**< VAB# used for LMC Miss Launch(valid only if VAM=1) */
	uint64_t sid                          : 9;  /**< LFB Source ID */
	uint64_t cmd                          : 4;  /**< LFB Command */
	uint64_t vld                          : 1;  /**< LFB Valid */
#else
	uint64_t vld                          : 1;
	uint64_t cmd                          : 4;
	uint64_t sid                          : 9;
	uint64_t vabnum                       : 2;
	uint64_t reserved_16_17               : 2;
	uint64_t set                          : 2;
	uint64_t reserved_20_20               : 1;
	uint64_t ihd                          : 1;
	uint64_t itl                          : 1;
	uint64_t inxt                         : 2;
	uint64_t reserved_25_26               : 2;
	uint64_t vam                          : 1;
	uint64_t stcfl                        : 1;
	uint64_t stinv                        : 1;
	uint64_t stpnd                        : 1;
	uint64_t stcpnd                       : 1;
	uint64_t reserved_32_63               : 32;
#endif
	} cn30xx;
	struct cvmx_l2c_lfb0_cn31xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t stcpnd                       : 1;  /**< LFB STC Pending Status */
	uint64_t stpnd                        : 1;  /**< LFB ST* Pending Status */
	uint64_t stinv                        : 1;  /**< LFB ST* Invalidate Status */
	uint64_t stcfl                        : 1;  /**< LFB STC=FAIL Status */
	uint64_t vam                          : 1;  /**< Valid Full Address Match Status */
	uint64_t reserved_26_26               : 1;
	uint64_t inxt                         : 3;  /**< Next LFB Pointer(invalid if ITL=1) */
	uint64_t itl                          : 1;  /**< LFB Tail of List Indicator */
	uint64_t ihd                          : 1;  /**< LFB Head of List Indicator */
	uint64_t reserved_20_20               : 1;
	uint64_t set                          : 2;  /**< SET# used for DS-OP (hit=hset/miss=rset) */
	uint64_t reserved_17_17               : 1;
	uint64_t vabnum                       : 3;  /**< VAB# used for LMC Miss Launch(valid only if VAM=1) */
	uint64_t sid                          : 9;  /**< LFB Source ID */
	uint64_t cmd                          : 4;  /**< LFB Command */
	uint64_t vld                          : 1;  /**< LFB Valid */
#else
	uint64_t vld                          : 1;
	uint64_t cmd                          : 4;
	uint64_t sid                          : 9;
	uint64_t vabnum                       : 3;
	uint64_t reserved_17_17               : 1;
	uint64_t set                          : 2;
	uint64_t reserved_20_20               : 1;
	uint64_t ihd                          : 1;
	uint64_t itl                          : 1;
	uint64_t inxt                         : 3;
	uint64_t reserved_26_26               : 1;
	uint64_t vam                          : 1;
	uint64_t stcfl                        : 1;
	uint64_t stinv                        : 1;
	uint64_t stpnd                        : 1;
	uint64_t stcpnd                       : 1;
	uint64_t reserved_32_63               : 32;
#endif
	} cn31xx;
	struct cvmx_l2c_lfb0_s                cn38xx;
	struct cvmx_l2c_lfb0_s                cn38xxp2;
	struct cvmx_l2c_lfb0_cn50xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t stcpnd                       : 1;  /**< LFB STC Pending Status */
	uint64_t stpnd                        : 1;  /**< LFB ST* Pending Status */
	uint64_t stinv                        : 1;  /**< LFB ST* Invalidate Status */
	uint64_t stcfl                        : 1;  /**< LFB STC=FAIL Status */
	uint64_t vam                          : 1;  /**< Valid Full Address Match Status */
	uint64_t reserved_26_26               : 1;
	uint64_t inxt                         : 3;  /**< Next LFB Pointer(invalid if ITL=1) */
	uint64_t itl                          : 1;  /**< LFB Tail of List Indicator */
	uint64_t ihd                          : 1;  /**< LFB Head of List Indicator */
	uint64_t set                          : 3;  /**< SET# used for DS-OP (hit=hset/miss=rset) */
	uint64_t reserved_17_17               : 1;
	uint64_t vabnum                       : 3;  /**< VAB# used for LMC Miss Launch(valid only if VAM=1) */
	uint64_t sid                          : 9;  /**< LFB Source ID */
	uint64_t cmd                          : 4;  /**< LFB Command */
	uint64_t vld                          : 1;  /**< LFB Valid */
#else
	uint64_t vld                          : 1;
	uint64_t cmd                          : 4;
	uint64_t sid                          : 9;
	uint64_t vabnum                       : 3;
	uint64_t reserved_17_17               : 1;
	uint64_t set                          : 3;
	uint64_t ihd                          : 1;
	uint64_t itl                          : 1;
	uint64_t inxt                         : 3;
	uint64_t reserved_26_26               : 1;
	uint64_t vam                          : 1;
	uint64_t stcfl                        : 1;
	uint64_t stinv                        : 1;
	uint64_t stpnd                        : 1;
	uint64_t stcpnd                       : 1;
	uint64_t reserved_32_63               : 32;
#endif
	} cn50xx;
	struct cvmx_l2c_lfb0_cn50xx           cn52xx;
	struct cvmx_l2c_lfb0_cn50xx           cn52xxp1;
	struct cvmx_l2c_lfb0_s                cn56xx;
	struct cvmx_l2c_lfb0_s                cn56xxp1;
	struct cvmx_l2c_lfb0_s                cn58xx;
	struct cvmx_l2c_lfb0_s                cn58xxp1;
};
typedef union cvmx_l2c_lfb0 cvmx_l2c_lfb0_t;

/**
 * cvmx_l2c_lfb1
 *
 * L2C_LFB1 = L2C LFB DEBUG 1 Register
 *
 * Description: L2C LFB Contents (Wait Bits)
 */
union cvmx_l2c_lfb1 {
	uint64_t u64;
	struct cvmx_l2c_lfb1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_19_63               : 45;
	uint64_t dsgoing                      : 1;  /**< LFB DS Going (in flight) */
	uint64_t bid                          : 2;  /**< LFB DS Bid# */
	uint64_t wtrsp                        : 1;  /**< LFB Waiting for RSC Response [FILL,STRSP] completion */
	uint64_t wtdw                         : 1;  /**< LFB Waiting for DS-WR completion */
	uint64_t wtdq                         : 1;  /**< LFB Waiting for LFB-DQ */
	uint64_t wtwhp                        : 1;  /**< LFB Waiting for Write-Hit Partial L2 DS-WR completion */
	uint64_t wtwhf                        : 1;  /**< LFB Waiting for Write-Hit Full L2 DS-WR completion */
	uint64_t wtwrm                        : 1;  /**< LFB Waiting for Write-Miss L2 DS-WR completion */
	uint64_t wtstm                        : 1;  /**< LFB Waiting for Write-Miss L2 DS-WR completion */
	uint64_t wtrda                        : 1;  /**< LFB Waiting for Read-Miss L2 DS-WR completion */
	uint64_t wtstdt                       : 1;  /**< LFB Waiting for all ST write Data to arrive on XMD bus */
	uint64_t wtstrsp                      : 1;  /**< LFB Waiting for ST RSC/RSD to be issued on RSP
                                                         (with invalidates) */
	uint64_t wtstrsc                      : 1;  /**< LFB Waiting for ST RSC-Only to be issued on RSP
                                                         (no-invalidates) */
	uint64_t wtvtm                        : 1;  /**< LFB Waiting for Victim Read L2 DS-RD completion */
	uint64_t wtmfl                        : 1;  /**< LFB Waiting for Memory Fill completion to MRB */
	uint64_t prbrty                       : 1;  /**< Probe-Retry Detected - waiting for probe completion */
	uint64_t wtprb                        : 1;  /**< LFB Waiting for Probe */
	uint64_t vld                          : 1;  /**< LFB Valid */
#else
	uint64_t vld                          : 1;
	uint64_t wtprb                        : 1;
	uint64_t prbrty                       : 1;
	uint64_t wtmfl                        : 1;
	uint64_t wtvtm                        : 1;
	uint64_t wtstrsc                      : 1;
	uint64_t wtstrsp                      : 1;
	uint64_t wtstdt                       : 1;
	uint64_t wtrda                        : 1;
	uint64_t wtstm                        : 1;
	uint64_t wtwrm                        : 1;
	uint64_t wtwhf                        : 1;
	uint64_t wtwhp                        : 1;
	uint64_t wtdq                         : 1;
	uint64_t wtdw                         : 1;
	uint64_t wtrsp                        : 1;
	uint64_t bid                          : 2;
	uint64_t dsgoing                      : 1;
	uint64_t reserved_19_63               : 45;
#endif
	} s;
	struct cvmx_l2c_lfb1_s                cn30xx;
	struct cvmx_l2c_lfb1_s                cn31xx;
	struct cvmx_l2c_lfb1_s                cn38xx;
	struct cvmx_l2c_lfb1_s                cn38xxp2;
	struct cvmx_l2c_lfb1_s                cn50xx;
	struct cvmx_l2c_lfb1_s                cn52xx;
	struct cvmx_l2c_lfb1_s                cn52xxp1;
	struct cvmx_l2c_lfb1_s                cn56xx;
	struct cvmx_l2c_lfb1_s                cn56xxp1;
	struct cvmx_l2c_lfb1_s                cn58xx;
	struct cvmx_l2c_lfb1_s                cn58xxp1;
};
typedef union cvmx_l2c_lfb1 cvmx_l2c_lfb1_t;

/**
 * cvmx_l2c_lfb2
 *
 * L2C_LFB2 = L2C LFB DEBUG 2 Register
 *
 * Description: L2C LFB Contents Tag/Index
 */
union cvmx_l2c_lfb2 {
	uint64_t u64;
	struct cvmx_l2c_lfb2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_l2c_lfb2_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_27_63               : 37;
	uint64_t lfb_tag                      : 19; /**< LFB TAG[33:15] */
	uint64_t lfb_idx                      : 8;  /**< LFB IDX[14:7] */
#else
	uint64_t lfb_idx                      : 8;
	uint64_t lfb_tag                      : 19;
	uint64_t reserved_27_63               : 37;
#endif
	} cn30xx;
	struct cvmx_l2c_lfb2_cn31xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_27_63               : 37;
	uint64_t lfb_tag                      : 17; /**< LFB TAG[33:16] */
	uint64_t lfb_idx                      : 10; /**< LFB IDX[15:7] */
#else
	uint64_t lfb_idx                      : 10;
	uint64_t lfb_tag                      : 17;
	uint64_t reserved_27_63               : 37;
#endif
	} cn31xx;
	struct cvmx_l2c_lfb2_cn31xx           cn38xx;
	struct cvmx_l2c_lfb2_cn31xx           cn38xxp2;
	struct cvmx_l2c_lfb2_cn50xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_27_63               : 37;
	uint64_t lfb_tag                      : 20; /**< LFB TAG[33:14] */
	uint64_t lfb_idx                      : 7;  /**< LFB IDX[13:7] */
#else
	uint64_t lfb_idx                      : 7;
	uint64_t lfb_tag                      : 20;
	uint64_t reserved_27_63               : 37;
#endif
	} cn50xx;
	struct cvmx_l2c_lfb2_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_27_63               : 37;
	uint64_t lfb_tag                      : 18; /**< LFB TAG[33:16] */
	uint64_t lfb_idx                      : 9;  /**< LFB IDX[15:7] */
#else
	uint64_t lfb_idx                      : 9;
	uint64_t lfb_tag                      : 18;
	uint64_t reserved_27_63               : 37;
#endif
	} cn52xx;
	struct cvmx_l2c_lfb2_cn52xx           cn52xxp1;
	struct cvmx_l2c_lfb2_cn56xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_27_63               : 37;
	uint64_t lfb_tag                      : 16; /**< LFB TAG[33:18] */
	uint64_t lfb_idx                      : 11; /**< LFB IDX[17:7] */
#else
	uint64_t lfb_idx                      : 11;
	uint64_t lfb_tag                      : 16;
	uint64_t reserved_27_63               : 37;
#endif
	} cn56xx;
	struct cvmx_l2c_lfb2_cn56xx           cn56xxp1;
	struct cvmx_l2c_lfb2_cn56xx           cn58xx;
	struct cvmx_l2c_lfb2_cn56xx           cn58xxp1;
};
typedef union cvmx_l2c_lfb2 cvmx_l2c_lfb2_t;

/**
 * cvmx_l2c_lfb3
 *
 * L2C_LFB3 = L2C LFB DEBUG 3 Register
 *
 * Description: LFB High Water Mark Register
 */
union cvmx_l2c_lfb3 {
	uint64_t u64;
	struct cvmx_l2c_lfb3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t stpartdis                    : 1;  /**< STP/C Performance Enhancement Disable
                                                         When clear, all STP/C(store partials) will take 2 cycles
                                                         to complete (power-on default).
                                                         When set, all STP/C(store partials) will take 4 cycles
                                                         to complete.
                                                         NOTE: It is recommended to keep this bit ALWAYS ZERO. */
	uint64_t lfb_hwm                      : 4;  /**< LFB High Water Mark
                                                         Determines \#of LFB Entries in use before backpressure
                                                         is asserted.
                                                            HWM=0:   1 LFB Entry available
                                                                       - ...
                                                            HWM=15: 16 LFB Entries available */
#else
	uint64_t lfb_hwm                      : 4;
	uint64_t stpartdis                    : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_l2c_lfb3_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t stpartdis                    : 1;  /**< STP/C Performance Enhancement Disable
                                                         When clear, all STP/C(store partials) will take 2 cycles
                                                         to complete (power-on default).
                                                         When set, all STP/C(store partials) will take 4 cycles
                                                         to complete.
                                                         NOTE: It is recommended to keep this bit ALWAYS ZERO. */
	uint64_t reserved_2_3                 : 2;
	uint64_t lfb_hwm                      : 2;  /**< LFB High Water Mark
                                                         Determines \#of LFB Entries in use before backpressure
                                                         is asserted.
                                                            HWM=0:   1 LFB Entry available
                                                                       - ...
                                                            HWM=3:   4 LFB Entries available */
#else
	uint64_t lfb_hwm                      : 2;
	uint64_t reserved_2_3                 : 2;
	uint64_t stpartdis                    : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} cn30xx;
	struct cvmx_l2c_lfb3_cn31xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t stpartdis                    : 1;  /**< STP/C Performance Enhancement Disable
                                                         When clear, all STP/C(store partials) will take 2 cycles
                                                         to complete (power-on default).
                                                         When set, all STP/C(store partials) will take 4 cycles
                                                         to complete.
                                                         NOTE: It is recommended to keep this bit ALWAYS ZERO. */
	uint64_t reserved_3_3                 : 1;
	uint64_t lfb_hwm                      : 3;  /**< LFB High Water Mark
                                                         Determines \#of LFB Entries in use before backpressure
                                                         is asserted.
                                                            HWM=0:   1 LFB Entry available
                                                                       - ...
                                                            HWM=7:   8 LFB Entries available */
#else
	uint64_t lfb_hwm                      : 3;
	uint64_t reserved_3_3                 : 1;
	uint64_t stpartdis                    : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} cn31xx;
	struct cvmx_l2c_lfb3_s                cn38xx;
	struct cvmx_l2c_lfb3_s                cn38xxp2;
	struct cvmx_l2c_lfb3_cn31xx           cn50xx;
	struct cvmx_l2c_lfb3_cn31xx           cn52xx;
	struct cvmx_l2c_lfb3_cn31xx           cn52xxp1;
	struct cvmx_l2c_lfb3_s                cn56xx;
	struct cvmx_l2c_lfb3_s                cn56xxp1;
	struct cvmx_l2c_lfb3_s                cn58xx;
	struct cvmx_l2c_lfb3_s                cn58xxp1;
};
typedef union cvmx_l2c_lfb3 cvmx_l2c_lfb3_t;

/**
 * cvmx_l2c_mci#_bist_status
 *
 * If clear BIST is desired, [CLEAR_BIST] must be written to 1 before [START_BIST] is written to
 * 1
 * using a separate CSR write operation.
 * [CLEAR_BIST] must not be changed after writing [START_BIST] to 1 until the BIST operation
 * completes (indicated by [START_BIST] returning to 0) or operation is undefined.
 */
union cvmx_l2c_mcix_bist_status {
	uint64_t u64;
	struct cvmx_l2c_mcix_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t start_bist                   : 1;  /**< When written to 1, starts BIST. Remains 1 until BIST is complete. */
	uint64_t clear_bist                   : 1;  /**< When BIST is triggered, run clear BIST. */
	uint64_t reserved_2_61                : 60;
	uint64_t vbffl                        : 2;  /**< BIST failure status for VBF0-1. */
#else
	uint64_t vbffl                        : 2;
	uint64_t reserved_2_61                : 60;
	uint64_t clear_bist                   : 1;
	uint64_t start_bist                   : 1;
#endif
	} s;
	struct cvmx_l2c_mcix_bist_status_s    cn70xx;
	struct cvmx_l2c_mcix_bist_status_s    cn70xxp1;
	struct cvmx_l2c_mcix_bist_status_s    cn73xx;
	struct cvmx_l2c_mcix_bist_status_s    cn78xx;
	struct cvmx_l2c_mcix_bist_status_s    cn78xxp1;
	struct cvmx_l2c_mcix_bist_status_s    cnf75xx;
};
typedef union cvmx_l2c_mcix_bist_status cvmx_l2c_mcix_bist_status_t;

/**
 * cvmx_l2c_mci#_err
 *
 * This register records error information for all MCI errors.
 * An error locks [VBF4], [INDEX], [SYN0], and [SYN1] and sets the bit corresponding to the error
 * received. VBFDBE errors take priority and will overwrite an earlier logged VBFSBE error. The
 * information from exactly one VBF read is present at any given time and serves to document
 * which error(s) were present in the read with the highest priority error.
 * The syndrome is recorded for DBE errors, though the utility of the value is not clear.
 */
union cvmx_l2c_mcix_err {
	uint64_t u64;
	struct cvmx_l2c_mcix_err_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t vbfdbe1                      : 1;  /**< INDEX/SYN1 corresponds to a double-bit VBF ECC error. */
	uint64_t vbfdbe0                      : 1;  /**< INDEX/SYN0 corresponds to a double-bit VBF ECC error. */
	uint64_t vbfsbe1                      : 1;  /**< INDEX/SYN1 corresponds to a single-bit VBF ECC error. */
	uint64_t vbfsbe0                      : 1;  /**< INDEX/SYN0 corresponds to a single-bit VBF ECC error. */
	uint64_t reserved_48_59               : 12;
	uint64_t syn1                         : 8;  /**< Error syndrome for QW1 (<127:64>). */
	uint64_t syn0                         : 8;  /**< Error syndrome for QW0 (<63:0>). */
	uint64_t reserved_12_31               : 20;
	uint64_t vbf4                         : 1;  /**< When 1, errors were from VBF (4+a), when 0, from VBF (0+a). */
	uint64_t index                        : 7;  /**< VBF index which was read and had the error(s). */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t index                        : 7;
	uint64_t vbf4                         : 1;
	uint64_t reserved_12_31               : 20;
	uint64_t syn0                         : 8;
	uint64_t syn1                         : 8;
	uint64_t reserved_48_59               : 12;
	uint64_t vbfsbe0                      : 1;
	uint64_t vbfsbe1                      : 1;
	uint64_t vbfdbe0                      : 1;
	uint64_t vbfdbe1                      : 1;
#endif
	} s;
	struct cvmx_l2c_mcix_err_s            cn70xx;
	struct cvmx_l2c_mcix_err_s            cn70xxp1;
	struct cvmx_l2c_mcix_err_s            cn73xx;
	struct cvmx_l2c_mcix_err_s            cn78xx;
	struct cvmx_l2c_mcix_err_s            cn78xxp1;
	struct cvmx_l2c_mcix_err_s            cnf75xx;
};
typedef union cvmx_l2c_mcix_err cvmx_l2c_mcix_err_t;

/**
 * cvmx_l2c_mci#_int
 *
 * This register is for MCI-based interrupts.
 *
 */
union cvmx_l2c_mcix_int {
	uint64_t u64;
	struct cvmx_l2c_mcix_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t vbfdbe                       : 1;  /**< VBF double-bit error occurred. See L2C_MCI()_ERR for logged information.
                                                         An indication of a hardware failure and may be considered fatal. */
	uint64_t vbfsbe                       : 1;  /**< VBF single-bit error occurred. See L2C_MCI()_ERR for logged information.
                                                         Hardware corrected the failure. Software may choose to count these single-bit errors. */
#else
	uint64_t vbfsbe                       : 1;
	uint64_t vbfdbe                       : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_l2c_mcix_int_s            cn70xx;
	struct cvmx_l2c_mcix_int_s            cn70xxp1;
	struct cvmx_l2c_mcix_int_s            cn73xx;
	struct cvmx_l2c_mcix_int_s            cn78xx;
	struct cvmx_l2c_mcix_int_s            cn78xxp1;
	struct cvmx_l2c_mcix_int_s            cnf75xx;
};
typedef union cvmx_l2c_mcix_int cvmx_l2c_mcix_int_t;

/**
 * cvmx_l2c_oci_ctl
 */
union cvmx_l2c_oci_ctl {
	uint64_t u64;
	struct cvmx_l2c_oci_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_31_63               : 33;
	uint64_t ncpend                       : 1;  /**< An indication that a node change is pending.  Hardware sets this bit when
                                                         OCX_COM_NODE[ID] is changed and clears the bit when the node change has taken
                                                         effect. */
	uint64_t lock_local_cas               : 1;  /**< Reserved. */
	uint64_t lock_local_stc               : 1;  /**< Reserved. */
	uint64_t lock_local_pp                : 1;  /**< Reserved. */
	uint64_t lngtolen                     : 5;  /**< Reserved. */
	uint64_t shtolen                      : 5;  /**< Reserved. */
	uint64_t shtoioen                     : 1;  /**< Reserved. */
	uint64_t shtoen                       : 3;  /**< Reserved. */
	uint64_t shto                         : 1;  /**< Reserved. */
	uint64_t inv_mode                     : 2;  /**< Reserved. */
	uint64_t cas_fdx                      : 1;  /**< Reserved. */
	uint64_t rldd_psha                    : 1;  /**< Reserved. */
	uint64_t lock_local_iob               : 1;  /**< Reserved. */
	uint64_t iofrcl                       : 1;  /**< Reserved. */
	uint64_t gksegnode                    : 2;  /**< Reserved. */
	uint64_t enaoci                       : 4;  /**< CCPI is not present. Any attempt to enable it will be ignored. */
#else
	uint64_t enaoci                       : 4;
	uint64_t gksegnode                    : 2;
	uint64_t iofrcl                       : 1;
	uint64_t lock_local_iob               : 1;
	uint64_t rldd_psha                    : 1;
	uint64_t cas_fdx                      : 1;
	uint64_t inv_mode                     : 2;
	uint64_t shto                         : 1;
	uint64_t shtoen                       : 3;
	uint64_t shtoioen                     : 1;
	uint64_t shtolen                      : 5;
	uint64_t lngtolen                     : 5;
	uint64_t lock_local_pp                : 1;
	uint64_t lock_local_stc               : 1;
	uint64_t lock_local_cas               : 1;
	uint64_t ncpend                       : 1;
	uint64_t reserved_31_63               : 33;
#endif
	} s;
	struct cvmx_l2c_oci_ctl_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_30_63               : 34;
	uint64_t lock_local_cas               : 1;  /**< Reserved. */
	uint64_t lock_local_stc               : 1;  /**< Reserved. */
	uint64_t lock_local_pp                : 1;  /**< Reserved. */
	uint64_t lngtolen                     : 5;  /**< Reserved. */
	uint64_t shtolen                      : 5;  /**< Reserved. */
	uint64_t shtoioen                     : 1;  /**< Reserved. */
	uint64_t shtoen                       : 3;  /**< Reserved. */
	uint64_t shto                         : 1;  /**< Reserved. */
	uint64_t inv_mode                     : 2;  /**< Reserved. */
	uint64_t cas_fdx                      : 1;  /**< Reserved. */
	uint64_t rldd_psha                    : 1;  /**< Reserved. */
	uint64_t lock_local_iob               : 1;  /**< Reserved. */
	uint64_t iofrcl                       : 1;  /**< Reserved. */
	uint64_t gksegnode                    : 2;  /**< Reserved. */
	uint64_t enaoci                       : 4;  /**< CCPI is not present. Any attempt to enable it will be ignored. */
#else
	uint64_t enaoci                       : 4;
	uint64_t gksegnode                    : 2;
	uint64_t iofrcl                       : 1;
	uint64_t lock_local_iob               : 1;
	uint64_t rldd_psha                    : 1;
	uint64_t cas_fdx                      : 1;
	uint64_t inv_mode                     : 2;
	uint64_t shto                         : 1;
	uint64_t shtoen                       : 3;
	uint64_t shtoioen                     : 1;
	uint64_t shtolen                      : 5;
	uint64_t lngtolen                     : 5;
	uint64_t lock_local_pp                : 1;
	uint64_t lock_local_stc               : 1;
	uint64_t lock_local_cas               : 1;
	uint64_t reserved_30_63               : 34;
#endif
	} cn73xx;
	struct cvmx_l2c_oci_ctl_s             cn78xx;
	struct cvmx_l2c_oci_ctl_cn73xx        cn78xxp1;
	struct cvmx_l2c_oci_ctl_cn73xx        cnf75xx;
};
typedef union cvmx_l2c_oci_ctl cvmx_l2c_oci_ctl_t;

/**
 * cvmx_l2c_oob
 *
 * L2C_OOB = L2C Out of Bounds Global Enables
 *
 * Description: Defines DMA "Out of Bounds" global enables.
 */
union cvmx_l2c_oob {
	uint64_t u64;
	struct cvmx_l2c_oob_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t dwbena                       : 1;  /**< DMA Out of Bounds Range Checker for DMA DWB
                                                         commands (Don't WriteBack).
                                                         When enabled, any DMA DWB commands which hit 1-of-3
                                                         out of bounds regions will be logged into
                                                         L2C_INT_STAT[OOB*] CSRs and the DMA store WILL
                                                         NOT occur. If the corresponding L2C_INT_EN[OOB*]
                                                         is enabled, an interrupt will also be reported. */
	uint64_t stena                        : 1;  /**< DMA Out of Bounds Range Checker for DMA store
                                                         commands (STF/P/T).
                                                         When enabled, any DMA store commands (STF/P/T) which
                                                         hit 1-of-3 out of bounds regions will be logged into
                                                         L2C_INT_STAT[OOB*] CSRs and the DMA store WILL
                                                         NOT occur. If the corresponding L2C_INT_EN[OOB*]
                                                         is enabled, an interrupt will also be reported. */
#else
	uint64_t stena                        : 1;
	uint64_t dwbena                       : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_l2c_oob_s                 cn52xx;
	struct cvmx_l2c_oob_s                 cn52xxp1;
	struct cvmx_l2c_oob_s                 cn56xx;
	struct cvmx_l2c_oob_s                 cn56xxp1;
};
typedef union cvmx_l2c_oob cvmx_l2c_oob_t;

/**
 * cvmx_l2c_oob1
 *
 * L2C_OOB1 = L2C Out of Bounds Range Checker
 *
 * Description: Defines DMA "Out of Bounds" region \#1. If a DMA initiated write transaction generates an address
 * within the specified region, the write is 'ignored' and an interrupt is generated to alert software.
 */
union cvmx_l2c_oob1 {
	uint64_t u64;
	struct cvmx_l2c_oob1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t fadr                         : 27; /**< DMA initated Memory Range Checker Failing Address
                                                         When L2C_INT_STAT[OOB1]=1, this field indicates the
                                                         DMA cacheline address.
                                                         (addr[33:7] = full cacheline address captured)
                                                         NOTE: FADR is locked down until L2C_INT_STAT[OOB1]
                                                         is cleared. */
	uint64_t fsrc                         : 1;  /**< DMA Out of Bounds Failing Source Command
                                                         When L2C_INT_STAT[OOB1]=1, this field indicates the
                                                         type of DMA command.
                                                          - 0: ST* (STF/P/T)
                                                          - 1: DWB (Don't WriteBack)
                                                         NOTE: FSRC is locked down until L2C_INT_STAT[OOB1]
                                                         is cleared. */
	uint64_t reserved_34_35               : 2;
	uint64_t sadr                         : 14; /**< DMA initated Memory Range Checker Starting Address
                                                         (1MB granularity) */
	uint64_t reserved_14_19               : 6;
	uint64_t size                         : 14; /**< DMA Out of Bounds Range Checker Size
                                                         (1MB granularity)
                                                         Example: 0: 0MB / 1: 1MB
                                                         The range check is for:
                                                             (SADR<<20) <= addr[33:0] < (((SADR+SIZE) & 0x3FFF)<<20)
                                                         SW NOTE: SADR+SIZE could be setup to potentially wrap
                                                         the 34bit ending bounds address. */
#else
	uint64_t size                         : 14;
	uint64_t reserved_14_19               : 6;
	uint64_t sadr                         : 14;
	uint64_t reserved_34_35               : 2;
	uint64_t fsrc                         : 1;
	uint64_t fadr                         : 27;
#endif
	} s;
	struct cvmx_l2c_oob1_s                cn52xx;
	struct cvmx_l2c_oob1_s                cn52xxp1;
	struct cvmx_l2c_oob1_s                cn56xx;
	struct cvmx_l2c_oob1_s                cn56xxp1;
};
typedef union cvmx_l2c_oob1 cvmx_l2c_oob1_t;

/**
 * cvmx_l2c_oob2
 *
 * L2C_OOB2 = L2C Out of Bounds Range Checker
 *
 * Description: Defines DMA "Out of Bounds" region \#2. If a DMA initiated write transaction generates an address
 * within the specified region, the write is 'ignored' and an interrupt is generated to alert software.
 */
union cvmx_l2c_oob2 {
	uint64_t u64;
	struct cvmx_l2c_oob2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t fadr                         : 27; /**< DMA initated Memory Range Checker Failing Address
                                                         When L2C_INT_STAT[OOB2]=1, this field indicates the
                                                         DMA cacheline address.
                                                         (addr[33:7] = full cacheline address captured)
                                                         NOTE: FADR is locked down until L2C_INT_STAT[OOB2]
                                                         is cleared. */
	uint64_t fsrc                         : 1;  /**< DMA Out of Bounds Failing Source Command
                                                         When L2C_INT_STAT[OOB2]=1, this field indicates the
                                                         type of DMA command.
                                                          - 0: ST* (STF/P/T)
                                                          - 1: DWB (Don't WriteBack)
                                                         NOTE: FSRC is locked down until L2C_INT_STAT[OOB2]
                                                         is cleared. */
	uint64_t reserved_34_35               : 2;
	uint64_t sadr                         : 14; /**< DMA initated Memory Range Checker Starting Address
                                                         (1MB granularity) */
	uint64_t reserved_14_19               : 6;
	uint64_t size                         : 14; /**< DMA Out of Bounds Range Checker Size
                                                         (1MB granularity)
                                                         Example: 0: 0MB / 1: 1MB
                                                         The range check is for:
                                                             (SADR<<20) <= addr[33:0] < (((SADR+SIZE) & 0x3FFF)<<20)
                                                         SW NOTE: SADR+SIZE could be setup to potentially wrap
                                                         the 34bit ending bounds address. */
#else
	uint64_t size                         : 14;
	uint64_t reserved_14_19               : 6;
	uint64_t sadr                         : 14;
	uint64_t reserved_34_35               : 2;
	uint64_t fsrc                         : 1;
	uint64_t fadr                         : 27;
#endif
	} s;
	struct cvmx_l2c_oob2_s                cn52xx;
	struct cvmx_l2c_oob2_s                cn52xxp1;
	struct cvmx_l2c_oob2_s                cn56xx;
	struct cvmx_l2c_oob2_s                cn56xxp1;
};
typedef union cvmx_l2c_oob2 cvmx_l2c_oob2_t;

/**
 * cvmx_l2c_oob3
 *
 * L2C_OOB3 = L2C Out of Bounds Range Checker
 *
 * Description: Defines DMA "Out of Bounds" region \#3. If a DMA initiated write transaction generates an address
 * within the specified region, the write is 'ignored' and an interrupt is generated to alert software.
 */
union cvmx_l2c_oob3 {
	uint64_t u64;
	struct cvmx_l2c_oob3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t fadr                         : 27; /**< DMA initated Memory Range Checker Failing Address
                                                         When L2C_INT_STAT[OOB3]=1, this field indicates the
                                                         DMA cacheline address.
                                                         (addr[33:7] = full cacheline address captured)
                                                         NOTE: FADR is locked down until L2C_INT_STAT[00B3]
                                                         is cleared. */
	uint64_t fsrc                         : 1;  /**< DMA Out of Bounds Failing Source Command
                                                         When L2C_INT_STAT[OOB3]=1, this field indicates the
                                                         type of DMA command.
                                                          - 0: ST* (STF/P/T)
                                                          - 1: DWB (Don't WriteBack)
                                                         NOTE: FSRC is locked down until L2C_INT_STAT[00B3]
                                                         is cleared. */
	uint64_t reserved_34_35               : 2;
	uint64_t sadr                         : 14; /**< DMA initated Memory Range Checker Starting Address
                                                         (1MB granularity) */
	uint64_t reserved_14_19               : 6;
	uint64_t size                         : 14; /**< DMA Out of Bounds Range Checker Size
                                                         (1MB granularity)
                                                         Example: 0: 0MB / 1: 1MB
                                                         The range check is for:
                                                             (SADR<<20) <= addr[33:0] < (((SADR+SIZE) & 0x3FFF)<<20)
                                                         SW NOTE: SADR+SIZE could be setup to potentially wrap
                                                         the 34bit ending bounds address. */
#else
	uint64_t size                         : 14;
	uint64_t reserved_14_19               : 6;
	uint64_t sadr                         : 14;
	uint64_t reserved_34_35               : 2;
	uint64_t fsrc                         : 1;
	uint64_t fadr                         : 27;
#endif
	} s;
	struct cvmx_l2c_oob3_s                cn52xx;
	struct cvmx_l2c_oob3_s                cn52xxp1;
	struct cvmx_l2c_oob3_s                cn56xx;
	struct cvmx_l2c_oob3_s                cn56xxp1;
};
typedef union cvmx_l2c_oob3 cvmx_l2c_oob3_t;

/**
 * cvmx_l2c_pfc#
 *
 * L2C_PFC0 = L2 Performance Counter \#0
 *
 * Description:
 */
union cvmx_l2c_pfcx {
	uint64_t u64;
	struct cvmx_l2c_pfcx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t pfcnt0                       : 36; /**< Performance Counter \#0 */
#else
	uint64_t pfcnt0                       : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_l2c_pfcx_s                cn30xx;
	struct cvmx_l2c_pfcx_s                cn31xx;
	struct cvmx_l2c_pfcx_s                cn38xx;
	struct cvmx_l2c_pfcx_s                cn38xxp2;
	struct cvmx_l2c_pfcx_s                cn50xx;
	struct cvmx_l2c_pfcx_s                cn52xx;
	struct cvmx_l2c_pfcx_s                cn52xxp1;
	struct cvmx_l2c_pfcx_s                cn56xx;
	struct cvmx_l2c_pfcx_s                cn56xxp1;
	struct cvmx_l2c_pfcx_s                cn58xx;
	struct cvmx_l2c_pfcx_s                cn58xxp1;
};
typedef union cvmx_l2c_pfcx cvmx_l2c_pfcx_t;

/**
 * cvmx_l2c_pfctl
 *
 * L2C_PFCTL = L2 Performance Counter Control Register
 *
 * Description: Controls the actions of the 4 Performance Counters
 *
 * Notes:
 * - There are four 36b performance counter registers which can simultaneously count events.
 * Each Counter's event is programmably selected via the corresponding CNTxSEL field:
 *       CNTxSEL[5:0]    Event
 *    -----------------+-----------------------
 *             0       | Cycles
 *             1       | L2 LDI Command Miss (NOTE: Both PP and IOB are cabable of generating LDI)
 *             2       | L2 LDI Command Hit  (NOTE: Both PP and IOB are cabable of generating LDI)
 *             3       | L2 non-LDI Command Miss
 *             4       | L2 non-LDI Command Hit
 *             5       | L2 Miss (total)
 *             6       | L2 Hit (total)
 *             7       | L2 Victim Buffer Hit (Retry Probe)
 *             8       | LFB-NQ Index Conflict
 *             9       | L2 Tag Probe (issued - could be VB-Retried)
 *            10       | L2 Tag Update (completed - note: some CMD types do not update)
 *            11       | L2 Tag Probe Completed (beyond VB-RTY window)
 *            12       | L2 Tag Dirty Victim
 *            13       | L2 Data Store NOP
 *            14       | L2 Data Store READ
 *            15       | L2 Data Store WRITE
 *            16       | Memory Fill Data valid (1 strobe/32B)
 *            17       | Memory Write Request
 *            18       | Memory Read Request
 *            19       | Memory Write Data valid (1 strobe/32B)
 *            20       | XMC NOP (XMC Bus Idle)
 *            21       | XMC LDT (Load-Through Request)
 *            22       | XMC LDI (L2 Load I-Stream Request)
 *            23       | XMC LDD (L2 Load D-stream Request)
 *            24       | XMC STF (L2 Store Full cacheline Request)
 *            25       | XMC STT (L2 Store Through Request)
 *            26       | XMC STP (L2 Store Partial Request)
 *            27       | XMC STC (L2 Store Conditional Request)
 *            28       | XMC DWB (L2 Don't WriteBack Request)
 *            29       | XMC PL2 (L2 Prefetch Request)
 *            30       | XMC PSL1 (L1 Prefetch Request)
 *            31       | XMC IOBLD
 *            32       | XMC IOBST
 *            33       | XMC IOBDMA
 *            34       | XMC IOBRSP
 *            35       | XMD Bus valid (all)
 *            36       | XMD Bus valid (DST=L2C) Memory Data
 *            37       | XMD Bus valid (DST=IOB) REFL Data
 *            38       | XMD Bus valid (DST=PP) IOBRSP Data
 *            39       | RSC NOP
 *            40       | RSC STDN
 *            41       | RSC FILL
 *            42       | RSC REFL
 *            43       | RSC STIN
 *            44       | RSC SCIN
 *            45       | RSC SCFL
 *            46       | RSC SCDN
 *            47       | RSD Data Valid
 *            48       | RSD Data Valid (FILL)
 *            49       | RSD Data Valid (STRSP)
 *            50       | RSD Data Valid (REFL)
 *            51       | LRF-REQ (LFB-NQ)
 *            52       | DT RD-ALLOC (LDD/PSL1 Commands)
 *            53       | DT WR-INVAL (ST* Commands)
 */
union cvmx_l2c_pfctl {
	uint64_t u64;
	struct cvmx_l2c_pfctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t cnt3rdclr                    : 1;  /**< Performance Counter 3 Read Clear
                                                         When set, all CSR reads of the L2C_PFC3
                                                         register will auto-clear the counter. This allows
                                                         SW to maintain 'cumulative' counters in SW.
                                                         NOTE: If the CSR read occurs in the same cycle as
                                                         the 'event' to be counted, the counter will
                                                         properly reflect the event. */
	uint64_t cnt2rdclr                    : 1;  /**< Performance Counter 2 Read Clear
                                                         When set, all CSR reads of the L2C_PFC2
                                                         register will auto-clear the counter. This allows
                                                         SW to maintain 'cumulative' counters in SW.
                                                         NOTE: If the CSR read occurs in the same cycle as
                                                         the 'event' to be counted, the counter will
                                                         properly reflect the event. */
	uint64_t cnt1rdclr                    : 1;  /**< Performance Counter 1 Read Clear
                                                         When set, all CSR reads of the L2C_PFC1
                                                         register will auto-clear the counter. This allows
                                                         SW to maintain 'cumulative' counters in SW.
                                                         NOTE: If the CSR read occurs in the same cycle as
                                                         the 'event' to be counted, the counter will
                                                         properly reflect the event. */
	uint64_t cnt0rdclr                    : 1;  /**< Performance Counter 0 Read Clear
                                                         When set, all CSR reads of the L2C_PFC0
                                                         register will 'auto-clear' the counter. This allows
                                                         SW to maintain accurate 'cumulative' counters.
                                                         NOTE: If the CSR read occurs in the same cycle as
                                                         the 'event' to be counted, the counter will
                                                         properly reflect the event. */
	uint64_t cnt3ena                      : 1;  /**< Performance Counter 3 Enable
                                                         When this bit is set, the performance counter
                                                         is enabled. */
	uint64_t cnt3clr                      : 1;  /**< Performance Counter 3 Clear
                                                         When the CSR write occurs, if this bit is set,
                                                         the performance counter is cleared. Otherwise,
                                                         it will resume counting from its current value. */
	uint64_t cnt3sel                      : 6;  /**< Performance Counter 3 Event Selector
                                                         (see list of selectable events to count in NOTES) */
	uint64_t cnt2ena                      : 1;  /**< Performance Counter 2 Enable
                                                         When this bit is set, the performance counter
                                                         is enabled. */
	uint64_t cnt2clr                      : 1;  /**< Performance Counter 2 Clear
                                                         When the CSR write occurs, if this bit is set,
                                                         the performance counter is cleared. Otherwise,
                                                         it will resume counting from its current value. */
	uint64_t cnt2sel                      : 6;  /**< Performance Counter 2 Event Selector
                                                         (see list of selectable events to count in NOTES) */
	uint64_t cnt1ena                      : 1;  /**< Performance Counter 1 Enable
                                                         When this bit is set, the performance counter
                                                         is enabled. */
	uint64_t cnt1clr                      : 1;  /**< Performance Counter 1 Clear
                                                         When the CSR write occurs, if this bit is set,
                                                         the performance counter is cleared. Otherwise,
                                                         it will resume counting from its current value. */
	uint64_t cnt1sel                      : 6;  /**< Performance Counter 1 Event Selector
                                                         (see list of selectable events to count in NOTES) */
	uint64_t cnt0ena                      : 1;  /**< Performance Counter 0 Enable
                                                         When this bit is set, the performance counter
                                                         is enabled. */
	uint64_t cnt0clr                      : 1;  /**< Performance Counter 0 Clear
                                                         When the CSR write occurs, if this bit is set,
                                                         the performance counter is cleared. Otherwise,
                                                         it will resume counting from its current value. */
	uint64_t cnt0sel                      : 6;  /**< Performance Counter 0 Event Selector
                                                         (see list of selectable events to count in NOTES) */
#else
	uint64_t cnt0sel                      : 6;
	uint64_t cnt0clr                      : 1;
	uint64_t cnt0ena                      : 1;
	uint64_t cnt1sel                      : 6;
	uint64_t cnt1clr                      : 1;
	uint64_t cnt1ena                      : 1;
	uint64_t cnt2sel                      : 6;
	uint64_t cnt2clr                      : 1;
	uint64_t cnt2ena                      : 1;
	uint64_t cnt3sel                      : 6;
	uint64_t cnt3clr                      : 1;
	uint64_t cnt3ena                      : 1;
	uint64_t cnt0rdclr                    : 1;
	uint64_t cnt1rdclr                    : 1;
	uint64_t cnt2rdclr                    : 1;
	uint64_t cnt3rdclr                    : 1;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_l2c_pfctl_s               cn30xx;
	struct cvmx_l2c_pfctl_s               cn31xx;
	struct cvmx_l2c_pfctl_s               cn38xx;
	struct cvmx_l2c_pfctl_s               cn38xxp2;
	struct cvmx_l2c_pfctl_s               cn50xx;
	struct cvmx_l2c_pfctl_s               cn52xx;
	struct cvmx_l2c_pfctl_s               cn52xxp1;
	struct cvmx_l2c_pfctl_s               cn56xx;
	struct cvmx_l2c_pfctl_s               cn56xxp1;
	struct cvmx_l2c_pfctl_s               cn58xx;
	struct cvmx_l2c_pfctl_s               cn58xxp1;
};
typedef union cvmx_l2c_pfctl cvmx_l2c_pfctl_t;

/**
 * cvmx_l2c_ppgrp
 *
 * L2C_PPGRP = L2C PP Group Number
 *
 * Description: Defines the PP(Packet Processor) PLC Group \# (0,1,2)
 */
union cvmx_l2c_ppgrp {
	uint64_t u64;
	struct cvmx_l2c_ppgrp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t pp11grp                      : 2;  /**< PP11 PLC Group# (0,1,2) */
	uint64_t pp10grp                      : 2;  /**< PP10 PLC Group# (0,1,2) */
	uint64_t pp9grp                       : 2;  /**< PP9 PLC Group# (0,1,2) */
	uint64_t pp8grp                       : 2;  /**< PP8 PLC Group# (0,1,2) */
	uint64_t pp7grp                       : 2;  /**< PP7 PLC Group# (0,1,2) */
	uint64_t pp6grp                       : 2;  /**< PP6 PLC Group# (0,1,2) */
	uint64_t pp5grp                       : 2;  /**< PP5 PLC Group# (0,1,2) */
	uint64_t pp4grp                       : 2;  /**< PP4 PLC Group# (0,1,2) */
	uint64_t pp3grp                       : 2;  /**< PP3 PLC Group# (0,1,2) */
	uint64_t pp2grp                       : 2;  /**< PP2 PLC Group# (0,1,2) */
	uint64_t pp1grp                       : 2;  /**< PP1 PLC Group# (0,1,2) */
	uint64_t pp0grp                       : 2;  /**< PP0 PLC Group# (0,1,2) */
#else
	uint64_t pp0grp                       : 2;
	uint64_t pp1grp                       : 2;
	uint64_t pp2grp                       : 2;
	uint64_t pp3grp                       : 2;
	uint64_t pp4grp                       : 2;
	uint64_t pp5grp                       : 2;
	uint64_t pp6grp                       : 2;
	uint64_t pp7grp                       : 2;
	uint64_t pp8grp                       : 2;
	uint64_t pp9grp                       : 2;
	uint64_t pp10grp                      : 2;
	uint64_t pp11grp                      : 2;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_l2c_ppgrp_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t pp3grp                       : 2;  /**< PP3 PLC Group# (0,1,2) */
	uint64_t pp2grp                       : 2;  /**< PP2 PLC Group# (0,1,2) */
	uint64_t pp1grp                       : 2;  /**< PP1 PLC Group# (0,1,2) */
	uint64_t pp0grp                       : 2;  /**< PP0 PLC Group# (0,1,2) */
#else
	uint64_t pp0grp                       : 2;
	uint64_t pp1grp                       : 2;
	uint64_t pp2grp                       : 2;
	uint64_t pp3grp                       : 2;
	uint64_t reserved_8_63                : 56;
#endif
	} cn52xx;
	struct cvmx_l2c_ppgrp_cn52xx          cn52xxp1;
	struct cvmx_l2c_ppgrp_s               cn56xx;
	struct cvmx_l2c_ppgrp_s               cn56xxp1;
};
typedef union cvmx_l2c_ppgrp cvmx_l2c_ppgrp_t;

/**
 * cvmx_l2c_qos_iob#
 *
 * L2C_QOS_IOB = L2C IOB QOS level
 *
 * Description:
 */
union cvmx_l2c_qos_iobx {
	uint64_t u64;
	struct cvmx_l2c_qos_iobx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t dwblvl                       : 3;  /**< QOS level for DWB commands. */
	uint64_t reserved_3_3                 : 1;
	uint64_t lvl                          : 3;  /**< QOS level for non-DWB commands. */
#else
	uint64_t lvl                          : 3;
	uint64_t reserved_3_3                 : 1;
	uint64_t dwblvl                       : 3;
	uint64_t reserved_7_63                : 57;
#endif
	} s;
	struct cvmx_l2c_qos_iobx_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t dwblvl                       : 2;  /**< QOS level for DWB commands. */
	uint64_t reserved_2_3                 : 2;
	uint64_t lvl                          : 2;  /**< QOS level for non-DWB commands. */
#else
	uint64_t lvl                          : 2;
	uint64_t reserved_2_3                 : 2;
	uint64_t dwblvl                       : 2;
	uint64_t reserved_6_63                : 58;
#endif
	} cn61xx;
	struct cvmx_l2c_qos_iobx_cn61xx       cn63xx;
	struct cvmx_l2c_qos_iobx_cn61xx       cn63xxp1;
	struct cvmx_l2c_qos_iobx_cn61xx       cn66xx;
	struct cvmx_l2c_qos_iobx_s            cn68xx;
	struct cvmx_l2c_qos_iobx_s            cn68xxp1;
	struct cvmx_l2c_qos_iobx_s            cn70xx;
	struct cvmx_l2c_qos_iobx_s            cn70xxp1;
	struct cvmx_l2c_qos_iobx_s            cn73xx;
	struct cvmx_l2c_qos_iobx_s            cn78xx;
	struct cvmx_l2c_qos_iobx_s            cn78xxp1;
	struct cvmx_l2c_qos_iobx_cn61xx       cnf71xx;
	struct cvmx_l2c_qos_iobx_s            cnf75xx;
};
typedef union cvmx_l2c_qos_iobx cvmx_l2c_qos_iobx_t;

/**
 * cvmx_l2c_qos_pp#
 *
 * L2C_QOS_PP = L2C PP QOS level
 *
 * Description:
 */
union cvmx_l2c_qos_ppx {
	uint64_t u64;
	struct cvmx_l2c_qos_ppx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t lvl                          : 3;  /**< QOS level to use for this core. */
#else
	uint64_t lvl                          : 3;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_l2c_qos_ppx_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t lvl                          : 2;  /**< QOS level to use for this PP. */
#else
	uint64_t lvl                          : 2;
	uint64_t reserved_2_63                : 62;
#endif
	} cn61xx;
	struct cvmx_l2c_qos_ppx_cn61xx        cn63xx;
	struct cvmx_l2c_qos_ppx_cn61xx        cn63xxp1;
	struct cvmx_l2c_qos_ppx_cn61xx        cn66xx;
	struct cvmx_l2c_qos_ppx_s             cn68xx;
	struct cvmx_l2c_qos_ppx_s             cn68xxp1;
	struct cvmx_l2c_qos_ppx_s             cn70xx;
	struct cvmx_l2c_qos_ppx_s             cn70xxp1;
	struct cvmx_l2c_qos_ppx_s             cn73xx;
	struct cvmx_l2c_qos_ppx_s             cn78xx;
	struct cvmx_l2c_qos_ppx_s             cn78xxp1;
	struct cvmx_l2c_qos_ppx_cn61xx        cnf71xx;
	struct cvmx_l2c_qos_ppx_s             cnf75xx;
};
typedef union cvmx_l2c_qos_ppx cvmx_l2c_qos_ppx_t;

/**
 * cvmx_l2c_qos_wgt
 *
 * L2C_QOS_WGT = L2C QOS weights
 *
 */
union cvmx_l2c_qos_wgt {
	uint64_t u64;
	struct cvmx_l2c_qos_wgt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t wgt7                         : 8;  /**< Weight for QOS level 7. */
	uint64_t wgt6                         : 8;  /**< Weight for QOS level 6. */
	uint64_t wgt5                         : 8;  /**< Weight for QOS level 5. */
	uint64_t wgt4                         : 8;  /**< Weight for QOS level 4. */
	uint64_t wgt3                         : 8;  /**< Weight for QOS level 3. */
	uint64_t wgt2                         : 8;  /**< Weight for QOS level 2. */
	uint64_t wgt1                         : 8;  /**< Weight for QOS level 1. */
	uint64_t wgt0                         : 8;  /**< Weight for QOS level 0. */
#else
	uint64_t wgt0                         : 8;
	uint64_t wgt1                         : 8;
	uint64_t wgt2                         : 8;
	uint64_t wgt3                         : 8;
	uint64_t wgt4                         : 8;
	uint64_t wgt5                         : 8;
	uint64_t wgt6                         : 8;
	uint64_t wgt7                         : 8;
#endif
	} s;
	struct cvmx_l2c_qos_wgt_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t wgt3                         : 8;  /**< Weight for QOS level 3 */
	uint64_t wgt2                         : 8;  /**< Weight for QOS level 2 */
	uint64_t wgt1                         : 8;  /**< Weight for QOS level 1 */
	uint64_t wgt0                         : 8;  /**< Weight for QOS level 0 */
#else
	uint64_t wgt0                         : 8;
	uint64_t wgt1                         : 8;
	uint64_t wgt2                         : 8;
	uint64_t wgt3                         : 8;
	uint64_t reserved_32_63               : 32;
#endif
	} cn61xx;
	struct cvmx_l2c_qos_wgt_cn61xx        cn63xx;
	struct cvmx_l2c_qos_wgt_cn61xx        cn63xxp1;
	struct cvmx_l2c_qos_wgt_cn61xx        cn66xx;
	struct cvmx_l2c_qos_wgt_s             cn68xx;
	struct cvmx_l2c_qos_wgt_s             cn68xxp1;
	struct cvmx_l2c_qos_wgt_s             cn70xx;
	struct cvmx_l2c_qos_wgt_s             cn70xxp1;
	struct cvmx_l2c_qos_wgt_s             cn73xx;
	struct cvmx_l2c_qos_wgt_s             cn78xx;
	struct cvmx_l2c_qos_wgt_s             cn78xxp1;
	struct cvmx_l2c_qos_wgt_cn61xx        cnf71xx;
	struct cvmx_l2c_qos_wgt_s             cnf75xx;
};
typedef union cvmx_l2c_qos_wgt cvmx_l2c_qos_wgt_t;

/**
 * cvmx_l2c_rsc#_pfc
 *
 * L2C_RSC_PFC = L2C RSC Performance Counter(s)
 *
 */
union cvmx_l2c_rscx_pfc {
	uint64_t u64;
	struct cvmx_l2c_rscx_pfc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t count                        : 64; /**< Current counter value */
#else
	uint64_t count                        : 64;
#endif
	} s;
	struct cvmx_l2c_rscx_pfc_s            cn61xx;
	struct cvmx_l2c_rscx_pfc_s            cn63xx;
	struct cvmx_l2c_rscx_pfc_s            cn63xxp1;
	struct cvmx_l2c_rscx_pfc_s            cn66xx;
	struct cvmx_l2c_rscx_pfc_s            cn68xx;
	struct cvmx_l2c_rscx_pfc_s            cn68xxp1;
	struct cvmx_l2c_rscx_pfc_s            cn70xx;
	struct cvmx_l2c_rscx_pfc_s            cn70xxp1;
	struct cvmx_l2c_rscx_pfc_s            cn73xx;
	struct cvmx_l2c_rscx_pfc_s            cn78xx;
	struct cvmx_l2c_rscx_pfc_s            cn78xxp1;
	struct cvmx_l2c_rscx_pfc_s            cnf71xx;
	struct cvmx_l2c_rscx_pfc_s            cnf75xx;
};
typedef union cvmx_l2c_rscx_pfc cvmx_l2c_rscx_pfc_t;

/**
 * cvmx_l2c_rsd#_pfc
 *
 * L2C_RSD_PFC = L2C RSD Performance Counter(s)
 *
 */
union cvmx_l2c_rsdx_pfc {
	uint64_t u64;
	struct cvmx_l2c_rsdx_pfc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t count                        : 64; /**< Current counter value */
#else
	uint64_t count                        : 64;
#endif
	} s;
	struct cvmx_l2c_rsdx_pfc_s            cn61xx;
	struct cvmx_l2c_rsdx_pfc_s            cn63xx;
	struct cvmx_l2c_rsdx_pfc_s            cn63xxp1;
	struct cvmx_l2c_rsdx_pfc_s            cn66xx;
	struct cvmx_l2c_rsdx_pfc_s            cn68xx;
	struct cvmx_l2c_rsdx_pfc_s            cn68xxp1;
	struct cvmx_l2c_rsdx_pfc_s            cn70xx;
	struct cvmx_l2c_rsdx_pfc_s            cn70xxp1;
	struct cvmx_l2c_rsdx_pfc_s            cn73xx;
	struct cvmx_l2c_rsdx_pfc_s            cn78xx;
	struct cvmx_l2c_rsdx_pfc_s            cn78xxp1;
	struct cvmx_l2c_rsdx_pfc_s            cnf71xx;
	struct cvmx_l2c_rsdx_pfc_s            cnf75xx;
};
typedef union cvmx_l2c_rsdx_pfc cvmx_l2c_rsdx_pfc_t;

/**
 * cvmx_l2c_rtg#_err
 *
 * This register records error information for all RTG SBE/DBE errors.
 * The priority of errors (lowest to highest) is SBE, DBE. An error locks [SYN], [WAY],
 * and [L2IDX] for equal or lower priority errors until cleared by software.
 * The syndrome is recorded for DBE errors, though the utility of the value is not clear.
 * [L2IDX]<19:7> is the L2 block index associated with the command which had no way to allocate.
 */
union cvmx_l2c_rtgx_err {
	uint64_t u64;
	struct cvmx_l2c_rtgx_err_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rtgdbe                       : 1;  /**< Information refers to a double-bit RTG ECC error. */
	uint64_t rtgsbe                       : 1;  /**< Information refers to a single-bit RTG ECC error. */
	uint64_t reserved_39_61               : 23;
	uint64_t syn                          : 7;  /**< Syndrome for the single-bit error. */
	uint64_t reserved_24_31               : 8;
	uint64_t way                          : 4;  /**< Way of the L2 block containing the error. */
	uint64_t l2idx                        : 13; /**< Index of the L2 block containing the error.
                                                         See L2C_TAD()_INT_W1C[RTGSBE] for an important use of this field. */
	uint64_t reserved_0_6                 : 7;
#else
	uint64_t reserved_0_6                 : 7;
	uint64_t l2idx                        : 13;
	uint64_t way                          : 4;
	uint64_t reserved_24_31               : 8;
	uint64_t syn                          : 7;
	uint64_t reserved_39_61               : 23;
	uint64_t rtgsbe                       : 1;
	uint64_t rtgdbe                       : 1;
#endif
	} s;
	struct cvmx_l2c_rtgx_err_s            cn78xx;
};
typedef union cvmx_l2c_rtgx_err cvmx_l2c_rtgx_err_t;

/**
 * cvmx_l2c_spar0
 *
 * L2C_SPAR0 = L2 Set Partitioning Register (PP0-3)
 *
 * Description: L2 Set Partitioning Register
 *
 * Notes:
 * - When a bit is set in the UMSK'x' register, a memory command issued from PP='x' will NOT select that
 *   set for replacement.
 * - There MUST ALWAYS BE at least 1 bit clear in each UMSK'x' register for proper L2 cache operation
 * - NOTES: When L2C FUSE[136] is blown(CRIP_256K), then SETS#7-4 are SET in all UMSK'x' registers
 *          When L2C FUSE[137] is blown(CRIP_128K), then SETS#7-2 are SET in all UMSK'x' registers
 */
union cvmx_l2c_spar0 {
	uint64_t u64;
	struct cvmx_l2c_spar0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t umsk3                        : 8;  /**< PP[3] L2 'DO NOT USE' set partition mask */
	uint64_t umsk2                        : 8;  /**< PP[2] L2 'DO NOT USE' set partition mask */
	uint64_t umsk1                        : 8;  /**< PP[1] L2 'DO NOT USE' set partition mask */
	uint64_t umsk0                        : 8;  /**< PP[0] L2 'DO NOT USE' set partition mask */
#else
	uint64_t umsk0                        : 8;
	uint64_t umsk1                        : 8;
	uint64_t umsk2                        : 8;
	uint64_t umsk3                        : 8;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_l2c_spar0_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t umsk0                        : 4;  /**< PP[0] L2 'DO NOT USE' set partition mask */
#else
	uint64_t umsk0                        : 4;
	uint64_t reserved_4_63                : 60;
#endif
	} cn30xx;
	struct cvmx_l2c_spar0_cn31xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t umsk1                        : 4;  /**< PP[1] L2 'DO NOT USE' set partition mask */
	uint64_t reserved_4_7                 : 4;
	uint64_t umsk0                        : 4;  /**< PP[0] L2 'DO NOT USE' set partition mask */
#else
	uint64_t umsk0                        : 4;
	uint64_t reserved_4_7                 : 4;
	uint64_t umsk1                        : 4;
	uint64_t reserved_12_63               : 52;
#endif
	} cn31xx;
	struct cvmx_l2c_spar0_s               cn38xx;
	struct cvmx_l2c_spar0_s               cn38xxp2;
	struct cvmx_l2c_spar0_cn50xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t umsk1                        : 8;  /**< PP[1] L2 'DO NOT USE' set partition mask */
	uint64_t umsk0                        : 8;  /**< PP[0] L2 'DO NOT USE' set partition mask */
#else
	uint64_t umsk0                        : 8;
	uint64_t umsk1                        : 8;
	uint64_t reserved_16_63               : 48;
#endif
	} cn50xx;
	struct cvmx_l2c_spar0_s               cn52xx;
	struct cvmx_l2c_spar0_s               cn52xxp1;
	struct cvmx_l2c_spar0_s               cn56xx;
	struct cvmx_l2c_spar0_s               cn56xxp1;
	struct cvmx_l2c_spar0_s               cn58xx;
	struct cvmx_l2c_spar0_s               cn58xxp1;
};
typedef union cvmx_l2c_spar0 cvmx_l2c_spar0_t;

/**
 * cvmx_l2c_spar1
 *
 * L2C_SPAR1 = L2 Set Partitioning Register (PP4-7)
 *
 * Description: L2 Set Partitioning Register
 *
 * Notes:
 * - When a bit is set in the UMSK'x' register, a memory command issued from PP='x' will NOT select that
 *   set for replacement.
 * - There should ALWAYS BE at least 1 bit clear in each UMSK'x' register for proper L2 cache operation
 * - NOTES: When L2C FUSE[136] is blown(CRIP_1024K), then SETS#7-4 are SET in all UMSK'x' registers
 *          When L2C FUSE[137] is blown(CRIP_512K), then SETS#7-2 are SET in all UMSK'x' registers
 */
union cvmx_l2c_spar1 {
	uint64_t u64;
	struct cvmx_l2c_spar1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t umsk7                        : 8;  /**< PP[7] L2 'DO NOT USE' set partition mask */
	uint64_t umsk6                        : 8;  /**< PP[6] L2 'DO NOT USE' set partition mask */
	uint64_t umsk5                        : 8;  /**< PP[5] L2 'DO NOT USE' set partition mask */
	uint64_t umsk4                        : 8;  /**< PP[4] L2 'DO NOT USE' set partition mask */
#else
	uint64_t umsk4                        : 8;
	uint64_t umsk5                        : 8;
	uint64_t umsk6                        : 8;
	uint64_t umsk7                        : 8;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_l2c_spar1_s               cn38xx;
	struct cvmx_l2c_spar1_s               cn38xxp2;
	struct cvmx_l2c_spar1_s               cn56xx;
	struct cvmx_l2c_spar1_s               cn56xxp1;
	struct cvmx_l2c_spar1_s               cn58xx;
	struct cvmx_l2c_spar1_s               cn58xxp1;
};
typedef union cvmx_l2c_spar1 cvmx_l2c_spar1_t;

/**
 * cvmx_l2c_spar2
 *
 * L2C_SPAR2 = L2 Set Partitioning Register (PP8-11)
 *
 * Description: L2 Set Partitioning Register
 *
 * Notes:
 * - When a bit is set in the UMSK'x' register, a memory command issued from PP='x' will NOT select that
 *   set for replacement.
 * - There should ALWAYS BE at least 1 bit clear in each UMSK'x' register for proper L2 cache operation
 * - NOTES: When L2C FUSE[136] is blown(CRIP_1024K), then SETS#7-4 are SET in all UMSK'x' registers
 *          When L2C FUSE[137] is blown(CRIP_512K), then SETS#7-2 are SET in all UMSK'x' registers
 */
union cvmx_l2c_spar2 {
	uint64_t u64;
	struct cvmx_l2c_spar2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t umsk11                       : 8;  /**< PP[11] L2 'DO NOT USE' set partition mask */
	uint64_t umsk10                       : 8;  /**< PP[10] L2 'DO NOT USE' set partition mask */
	uint64_t umsk9                        : 8;  /**< PP[9] L2 'DO NOT USE' set partition mask */
	uint64_t umsk8                        : 8;  /**< PP[8] L2 'DO NOT USE' set partition mask */
#else
	uint64_t umsk8                        : 8;
	uint64_t umsk9                        : 8;
	uint64_t umsk10                       : 8;
	uint64_t umsk11                       : 8;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_l2c_spar2_s               cn38xx;
	struct cvmx_l2c_spar2_s               cn38xxp2;
	struct cvmx_l2c_spar2_s               cn56xx;
	struct cvmx_l2c_spar2_s               cn56xxp1;
	struct cvmx_l2c_spar2_s               cn58xx;
	struct cvmx_l2c_spar2_s               cn58xxp1;
};
typedef union cvmx_l2c_spar2 cvmx_l2c_spar2_t;

/**
 * cvmx_l2c_spar3
 *
 * L2C_SPAR3 = L2 Set Partitioning Register (PP12-15)
 *
 * Description: L2 Set Partitioning Register
 *
 * Notes:
 * - When a bit is set in the UMSK'x' register, a memory command issued from PP='x' will NOT select that
 *   set for replacement.
 * - There should ALWAYS BE at least 1 bit clear in each UMSK'x' register for proper L2 cache operation
 * - NOTES: When L2C FUSE[136] is blown(CRIP_1024K), then SETS#7-4 are SET in all UMSK'x' registers
 *          When L2C FUSE[137] is blown(CRIP_512K), then SETS#7-2 are SET in all UMSK'x' registers
 */
union cvmx_l2c_spar3 {
	uint64_t u64;
	struct cvmx_l2c_spar3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t umsk15                       : 8;  /**< PP[15] L2 'DO NOT USE' set partition mask */
	uint64_t umsk14                       : 8;  /**< PP[14] L2 'DO NOT USE' set partition mask */
	uint64_t umsk13                       : 8;  /**< PP[13] L2 'DO NOT USE' set partition mask */
	uint64_t umsk12                       : 8;  /**< PP[12] L2 'DO NOT USE' set partition mask */
#else
	uint64_t umsk12                       : 8;
	uint64_t umsk13                       : 8;
	uint64_t umsk14                       : 8;
	uint64_t umsk15                       : 8;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_l2c_spar3_s               cn38xx;
	struct cvmx_l2c_spar3_s               cn38xxp2;
	struct cvmx_l2c_spar3_s               cn58xx;
	struct cvmx_l2c_spar3_s               cn58xxp1;
};
typedef union cvmx_l2c_spar3 cvmx_l2c_spar3_t;

/**
 * cvmx_l2c_spar4
 *
 * L2C_SPAR4 = L2 Set Partitioning Register (IOB)
 *
 * Description: L2 Set Partitioning Register
 *
 * Notes:
 * - When a bit is set in the UMSK'x' register, a memory command issued from PP='x' will NOT select that
 *   set for replacement.
 * - There should ALWAYS BE at least 1 bit clear in each UMSK'x' register for proper L2 cache operation
 * - NOTES: When L2C FUSE[136] is blown(CRIP_256K), then SETS#7-4 are SET in all UMSK'x' registers
 *          When L2C FUSE[137] is blown(CRIP_128K), then SETS#7-2 are SET in all UMSK'x' registers
 */
union cvmx_l2c_spar4 {
	uint64_t u64;
	struct cvmx_l2c_spar4_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t umskiob                      : 8;  /**< IOB L2 'DO NOT USE' set partition mask */
#else
	uint64_t umskiob                      : 8;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_l2c_spar4_cn30xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t umskiob                      : 4;  /**< IOB L2 'DO NOT USE' set partition mask */
#else
	uint64_t umskiob                      : 4;
	uint64_t reserved_4_63                : 60;
#endif
	} cn30xx;
	struct cvmx_l2c_spar4_cn30xx          cn31xx;
	struct cvmx_l2c_spar4_s               cn38xx;
	struct cvmx_l2c_spar4_s               cn38xxp2;
	struct cvmx_l2c_spar4_s               cn50xx;
	struct cvmx_l2c_spar4_s               cn52xx;
	struct cvmx_l2c_spar4_s               cn52xxp1;
	struct cvmx_l2c_spar4_s               cn56xx;
	struct cvmx_l2c_spar4_s               cn56xxp1;
	struct cvmx_l2c_spar4_s               cn58xx;
	struct cvmx_l2c_spar4_s               cn58xxp1;
};
typedef union cvmx_l2c_spar4 cvmx_l2c_spar4_t;

/**
 * cvmx_l2c_tad#_dll
 *
 * This register provides the parameters for DLL observability.
 *
 */
union cvmx_l2c_tadx_dll {
	uint64_t u64;
	struct cvmx_l2c_tadx_dll_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t pd_pos_rclk_refclk           : 1;  /**< Phase detector output. */
	uint64_t pdl_rclk_refclk              : 1;  /**< Phase detector output. */
	uint64_t pdr_rclk_refclk              : 1;  /**< Phase detector output. */
	uint64_t clk_invert                   : 1;  /**< Clock invert. */
	uint64_t dly_elem_enable              : 16; /**< Delay element enable. */
	uint64_t dll_setting                  : 12; /**< DLL setting. */
	uint64_t dll_state                    : 3;  /**< DLL state. */
	uint64_t dll_lock                     : 1;  /**< DLL lock: 1 = locked, 0 = unlocked. */
#else
	uint64_t dll_lock                     : 1;
	uint64_t dll_state                    : 3;
	uint64_t dll_setting                  : 12;
	uint64_t dly_elem_enable              : 16;
	uint64_t clk_invert                   : 1;
	uint64_t pdr_rclk_refclk              : 1;
	uint64_t pdl_rclk_refclk              : 1;
	uint64_t pd_pos_rclk_refclk           : 1;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_l2c_tadx_dll_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t dll_setting                  : 12; /**< DLL setting. */
	uint64_t dll_state                    : 3;  /**< DLL state. */
	uint64_t dll_lock                     : 1;  /**< DLL lock: 1 = locked, 0 = unlocked. */
#else
	uint64_t dll_lock                     : 1;
	uint64_t dll_state                    : 3;
	uint64_t dll_setting                  : 12;
	uint64_t reserved_16_63               : 48;
#endif
	} cn70xx;
	struct cvmx_l2c_tadx_dll_cn70xx       cn70xxp1;
	struct cvmx_l2c_tadx_dll_s            cn73xx;
	struct cvmx_l2c_tadx_dll_s            cn78xx;
	struct cvmx_l2c_tadx_dll_s            cn78xxp1;
	struct cvmx_l2c_tadx_dll_s            cnf75xx;
};
typedef union cvmx_l2c_tadx_dll cvmx_l2c_tadx_dll_t;

/**
 * cvmx_l2c_tad#_ecc0
 *
 * L2C_TAD_ECC0 = L2C ECC logging
 *
 * Description: holds the syndromes for a L2D read generated from L2C_XMC_CMD
 */
union cvmx_l2c_tadx_ecc0 {
	uint64_t u64;
	struct cvmx_l2c_tadx_ecc0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_58_63               : 6;
	uint64_t ow3ecc                       : 10; /**< ECC for OW3 of cache block */
	uint64_t reserved_42_47               : 6;
	uint64_t ow2ecc                       : 10; /**< ECC for OW2 of cache block */
	uint64_t reserved_26_31               : 6;
	uint64_t ow1ecc                       : 10; /**< ECC for OW1 of cache block */
	uint64_t reserved_10_15               : 6;
	uint64_t ow0ecc                       : 10; /**< ECC for OW0 of cache block */
#else
	uint64_t ow0ecc                       : 10;
	uint64_t reserved_10_15               : 6;
	uint64_t ow1ecc                       : 10;
	uint64_t reserved_26_31               : 6;
	uint64_t ow2ecc                       : 10;
	uint64_t reserved_42_47               : 6;
	uint64_t ow3ecc                       : 10;
	uint64_t reserved_58_63               : 6;
#endif
	} s;
	struct cvmx_l2c_tadx_ecc0_s           cn61xx;
	struct cvmx_l2c_tadx_ecc0_s           cn63xx;
	struct cvmx_l2c_tadx_ecc0_s           cn63xxp1;
	struct cvmx_l2c_tadx_ecc0_s           cn66xx;
	struct cvmx_l2c_tadx_ecc0_s           cn68xx;
	struct cvmx_l2c_tadx_ecc0_s           cn68xxp1;
	struct cvmx_l2c_tadx_ecc0_s           cnf71xx;
};
typedef union cvmx_l2c_tadx_ecc0 cvmx_l2c_tadx_ecc0_t;

/**
 * cvmx_l2c_tad#_ecc1
 *
 * L2C_TAD_ECC1 = L2C ECC logging
 *
 * Description: holds the syndromes for a L2D read generated from L2C_XMC_CMD
 */
union cvmx_l2c_tadx_ecc1 {
	uint64_t u64;
	struct cvmx_l2c_tadx_ecc1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_58_63               : 6;
	uint64_t ow7ecc                       : 10; /**< ECC for OW7 of cache block */
	uint64_t reserved_42_47               : 6;
	uint64_t ow6ecc                       : 10; /**< ECC for OW6 of cache block */
	uint64_t reserved_26_31               : 6;
	uint64_t ow5ecc                       : 10; /**< ECC for OW5 of cache block */
	uint64_t reserved_10_15               : 6;
	uint64_t ow4ecc                       : 10; /**< ECC for OW4 of cache block */
#else
	uint64_t ow4ecc                       : 10;
	uint64_t reserved_10_15               : 6;
	uint64_t ow5ecc                       : 10;
	uint64_t reserved_26_31               : 6;
	uint64_t ow6ecc                       : 10;
	uint64_t reserved_42_47               : 6;
	uint64_t ow7ecc                       : 10;
	uint64_t reserved_58_63               : 6;
#endif
	} s;
	struct cvmx_l2c_tadx_ecc1_s           cn61xx;
	struct cvmx_l2c_tadx_ecc1_s           cn63xx;
	struct cvmx_l2c_tadx_ecc1_s           cn63xxp1;
	struct cvmx_l2c_tadx_ecc1_s           cn66xx;
	struct cvmx_l2c_tadx_ecc1_s           cn68xx;
	struct cvmx_l2c_tadx_ecc1_s           cn68xxp1;
	struct cvmx_l2c_tadx_ecc1_s           cnf71xx;
};
typedef union cvmx_l2c_tadx_ecc1 cvmx_l2c_tadx_ecc1_t;

/**
 * cvmx_l2c_tad#_err
 *
 * This register records error information for BIG* interrupts. The BIG logic only
 * applies to local addresses. The first BIGWR error will lock the register until the
 * logged error type is cleared; BIGRD never locks the register.
 */
union cvmx_l2c_tadx_err {
	uint64_t u64;
	struct cvmx_l2c_tadx_err_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bigrd                        : 1;  /**< Logged information is for a BIGRD error. */
	uint64_t bigwr                        : 1;  /**< Logged information is for a BIGWR error. */
	uint64_t reserved_59_61               : 3;
	uint64_t cmd                          : 8;  /**< Encoding of XMC command causing error. */
	uint64_t source                       : 7;  /**< XMC source of request causing error. If [SOURCE]<6>==0, then [SOURCE]<5:0> is PPID, else
                                                         [SOURCE]<3:0> is BUSID of the IOB which made the request. If [CMD]<7>==0, this field is
                                                         unpredictable. */
	uint64_t node                         : 4;  /**< Reserved. Will always be 0. */
	uint64_t addr                         : 40; /**< XMC address causing the error. [ADDR]<6:0> is unpredictable for *DISOCI and BIG*
                                                         errors. This field is the physical address after hole removal and index aliasing
                                                         (if enabled). (The hole is between DR0 and DR1. Remove the hole by subtracting
                                                         256MB from all L2/DRAM physical addresses >= 512 MB.) */
#else
	uint64_t addr                         : 40;
	uint64_t node                         : 4;
	uint64_t source                       : 7;
	uint64_t cmd                          : 8;
	uint64_t reserved_59_61               : 3;
	uint64_t bigwr                        : 1;
	uint64_t bigrd                        : 1;
#endif
	} s;
	struct cvmx_l2c_tadx_err_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bigrd                        : 1;  /**< Logged information is for a BIGRD error. */
	uint64_t bigwr                        : 1;  /**< Logged information is for a BIGWR error. */
	uint64_t holerd                       : 1;  /**< Logged information is for a HOLERD error. */
	uint64_t holewr                       : 1;  /**< Logged information is for a HOLEWR error. */
	uint64_t reserved_58_59               : 2;
	uint64_t cmd                          : 7;  /**< XMC command of request causing error. */
	uint64_t source                       : 7;  /**< XMC 'source' of request causing error. If SOURCE<6>==0, SOURCE<5:0> = PPID else
                                                         SOURCE<3:0> is BUSID of IOB which made the request. */
	uint64_t node                         : 4;  /**< Always zero. */
	uint64_t addr                         : 40; /**< XMC address causing the error. This field is the physical address after hole removal and
                                                         index aliasing (if enabled). (The hole is between DR0 and DR1. Remove the hole by
                                                         subtracting 256MB from all L2/DRAM physical addresses >= 512 MB.) */
#else
	uint64_t addr                         : 40;
	uint64_t node                         : 4;
	uint64_t source                       : 7;
	uint64_t cmd                          : 7;
	uint64_t reserved_58_59               : 2;
	uint64_t holewr                       : 1;
	uint64_t holerd                       : 1;
	uint64_t bigwr                        : 1;
	uint64_t bigrd                        : 1;
#endif
	} cn70xx;
	struct cvmx_l2c_tadx_err_cn70xx       cn70xxp1;
	struct cvmx_l2c_tadx_err_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bigrd                        : 1;  /**< Logged information is for a BIGRD error. */
	uint64_t bigwr                        : 1;  /**< Logged information is for a BIGWR error. */
	uint64_t rddisoci                     : 1;  /**< Reserved. */
	uint64_t wrdisoci                     : 1;  /**< Reserved. */
	uint64_t reserved_59_59               : 1;
	uint64_t cmd                          : 8;  /**< Encoding of XMC command causing error. */
	uint64_t source                       : 7;  /**< XMC source of request causing error. If [SOURCE]<6>==0, then [SOURCE]<5:0> is PPID, else
                                                         [SOURCE]<3:0> is BUSID of the IOB which made the request. If [CMD]<7>==0, this field is
                                                         unpredictable. */
	uint64_t node                         : 4;  /**< Reserved. Will always be 0. */
	uint64_t addr                         : 40; /**< XMC address causing the error. [ADDR]<6:0> is unpredictable for *DISOCI and BIG*
                                                         errors. This field is the physical address after hole removal and index aliasing
                                                         (if enabled). (The hole is between DR0 and DR1. Remove the hole by subtracting
                                                         256MB from all L2/DRAM physical addresses >= 512 MB.) */
#else
	uint64_t addr                         : 40;
	uint64_t node                         : 4;
	uint64_t source                       : 7;
	uint64_t cmd                          : 8;
	uint64_t reserved_59_59               : 1;
	uint64_t wrdisoci                     : 1;
	uint64_t rddisoci                     : 1;
	uint64_t bigwr                        : 1;
	uint64_t bigrd                        : 1;
#endif
	} cn73xx;
	struct cvmx_l2c_tadx_err_cn73xx       cn78xx;
	struct cvmx_l2c_tadx_err_cn73xx       cn78xxp1;
	struct cvmx_l2c_tadx_err_cn73xx       cnf75xx;
};
typedef union cvmx_l2c_tadx_err cvmx_l2c_tadx_err_t;

/**
 * cvmx_l2c_tad#_ien
 *
 * L2C_TAD_IEN = L2C TAD Interrupt Enable
 *
 */
union cvmx_l2c_tadx_ien {
	uint64_t u64;
	struct cvmx_l2c_tadx_ien_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t wrdislmc                     : 1;  /**< Illegal Write to Disabled LMC Error enable
                                                         Enables L2C_TADX_INT[WRDISLMC] to
                                                         assert L2C_INT_REG[TADX] (and cause an interrupt) */
	uint64_t rddislmc                     : 1;  /**< Illegal Read  to Disabled LMC Error enable
                                                         Enables L2C_TADX_INT[RDDISLMC] to
                                                         assert L2C_INT_REG[TADX] (and cause an interrupt) */
	uint64_t noway                        : 1;  /**< No way available interrupt enable
                                                         Enables L2C_ERR_TTGX[NOWAY]/L2C_TADX_INT[NOWAY] to
                                                         assert L2C_INT_REG[TADX] (and cause an interrupt) */
	uint64_t vbfdbe                       : 1;  /**< VBF Double-Bit Error enable
                                                         Enables L2C_ERR_TDTX[VDBE]/L2C_TADX_INT[VBFSBE] to
                                                         assert L2C_INT_REG[TADX] (and cause an interrupt) */
	uint64_t vbfsbe                       : 1;  /**< VBF Single-Bit Error enable
                                                         Enables L2C_ERR_TDTX[VSBE]/L2C_TADX_INT[VBFSBE] to
                                                         assert L2C_INT_REG[TADX] (and cause an interrupt) */
	uint64_t tagdbe                       : 1;  /**< TAG Double-Bit Error enable
                                                         Enables L2C_ERR_TTGX[DBE]/L2C_TADX_INT[TAGDBE] to
                                                         assert L2C_INT_REG[TADX] (and cause an interrupt) */
	uint64_t tagsbe                       : 1;  /**< TAG Single-Bit Error enable
                                                         Enables L2C_ERR_TTGX[SBE]/L2C_TADX_INT[TAGSBE] to
                                                         assert L2C_INT_REG[TADX] (and cause an interrupt) */
	uint64_t l2ddbe                       : 1;  /**< L2D Double-Bit Error enable
                                                         Enables L2C_ERR_TDTX[DBE]/L2C_TADX_INT[L2DDBE] to
                                                         assert L2C_INT_REG[TADX] (and cause an interrupt) */
	uint64_t l2dsbe                       : 1;  /**< L2D Single-Bit Error enable
                                                         Enables L2C_ERR_TDTX[SBE]/L2C_TADX_INT[L2DSBE] to
                                                         assert L2C_INT_REG[TADX] (and cause an interrupt) */
#else
	uint64_t l2dsbe                       : 1;
	uint64_t l2ddbe                       : 1;
	uint64_t tagsbe                       : 1;
	uint64_t tagdbe                       : 1;
	uint64_t vbfsbe                       : 1;
	uint64_t vbfdbe                       : 1;
	uint64_t noway                        : 1;
	uint64_t rddislmc                     : 1;
	uint64_t wrdislmc                     : 1;
	uint64_t reserved_9_63                : 55;
#endif
	} s;
	struct cvmx_l2c_tadx_ien_s            cn61xx;
	struct cvmx_l2c_tadx_ien_s            cn63xx;
	struct cvmx_l2c_tadx_ien_cn63xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t noway                        : 1;  /**< No way available interrupt enable
                                                         Enables L2C_ERR_TTGX[NOWAY] to assert
                                                         L2C_INT_REG[TADX] (and cause an interrupt) */
	uint64_t vbfdbe                       : 1;  /**< VBF Double-Bit Error enable
                                                         Enables L2C_ERR_TDTX[VSBE] to assert
                                                         L2C_INT_REG[TADX] (and cause an interrupt) */
	uint64_t vbfsbe                       : 1;  /**< VBF Single-Bit Error enable
                                                         Enables L2C_ERR_TDTX[VSBE] to assert
                                                         L2C_INT_REG[TADX] (and cause an interrupt) */
	uint64_t tagdbe                       : 1;  /**< TAG Double-Bit Error enable
                                                         Enables L2C_ERR_TTGX[DBE] to assert
                                                         L2C_INT_REG[TADX] (and cause an interrupt) */
	uint64_t tagsbe                       : 1;  /**< TAG Single-Bit Error enable
                                                         Enables L2C_ERR_TTGX[SBE] to assert
                                                         L2C_INT_REG[TADX] (and cause an interrupt) */
	uint64_t l2ddbe                       : 1;  /**< L2D Double-Bit Error enable
                                                         Enables L2C_ERR_TDTX[DBE] to assert
                                                         L2C_INT_REG[TADX] (and cause an interrupt) */
	uint64_t l2dsbe                       : 1;  /**< L2D Single-Bit Error enable
                                                         Enables L2C_ERR_TDTX[SBE] to assert
                                                         L2C_INT_REG[TADX] (and cause an interrupt) */
#else
	uint64_t l2dsbe                       : 1;
	uint64_t l2ddbe                       : 1;
	uint64_t tagsbe                       : 1;
	uint64_t tagdbe                       : 1;
	uint64_t vbfsbe                       : 1;
	uint64_t vbfdbe                       : 1;
	uint64_t noway                        : 1;
	uint64_t reserved_7_63                : 57;
#endif
	} cn63xxp1;
	struct cvmx_l2c_tadx_ien_s            cn66xx;
	struct cvmx_l2c_tadx_ien_s            cn68xx;
	struct cvmx_l2c_tadx_ien_s            cn68xxp1;
	struct cvmx_l2c_tadx_ien_s            cnf71xx;
};
typedef union cvmx_l2c_tadx_ien cvmx_l2c_tadx_ien_t;

/**
 * cvmx_l2c_tad#_int
 *
 * This register is for TAD-based interrupts.
 *
 */
union cvmx_l2c_tadx_int {
	uint64_t u64;
	struct cvmx_l2c_tadx_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t wrdisoci                     : 1;  /**< Reserved. */
	uint64_t rddisoci                     : 1;  /**< Reserved. */
	uint64_t rtgdbe                       : 1;  /**< RTG double-bit error.
                                                         See L2C_TAD()_RTG_ERR for logged information.
                                                         An indication of a hardware failure and may be considered fatal. */
	uint64_t rtgsbe                       : 1;  /**< RTG single-bit error on a read. See L2C_TAD()_RTG_ERR for logged
                                                         information. When [RTGSBE] is set, hardware corrected the error before using the
                                                         RTG tag, but did not correct any stored value. When [RTGSBE] is set, software
                                                         should eject the RTG location indicated by the corresponding
                                                         L2C_TAD()_RTG_ERR[WAY,L2IDX] before clearing [RTGSBE]. Otherwise, hardware may
                                                         encounter the error again the next time the same RTG location is
                                                         referenced. Software may also choose to count the number of these single-bit
                                                         errors.
                                                         The eject should use a CACHE 0x3 instruction with an effective address of:
                                                         <pre>
                                                           payload<24> = 1
                                                           payload<23:20> = L2C_TAD()_RTG_ERR[WAY]
                                                           payload<19:7>  = L2C_TAD()_RTG_ERR[L2IDX]
                                                         </pre> */
	uint64_t reserved_18_31               : 14;
	uint64_t lfbto                        : 1;  /**< An LFB entry (or more) has encountered a timeout condition When LFBTO timeout condition
                                                         occurs L2C_TAD()_TIMEOUT is loaded. L2C_TAD()_TIMEOUT is loaded with info from the
                                                         first LFB that timed out. if multiple LFB timed out simultaneously, then the it will
                                                         capture info from the lowest LFB number that timed out.
                                                         Should not occur during normal operation.  OCI/CCPI link failures may cause this
                                                         failure. This may be an indication of hardware failure, and may be considered
                                                         fatal. */
	uint64_t reserved_15_16               : 2;
	uint64_t bigrd                        : 1;  /**< Read reference past L2C_BIG_CTL[MAXDRAM] occurred. [BIGRD] interrupts can occur during
                                                         normal operation as the cores are allowed to prefetch to nonexistent memory locations.
                                                         Therefore, [BIGRD] is for informational purposes only. See L2C_TAD()_ERR for logged
                                                         information. */
	uint64_t bigwr                        : 1;  /**< Write reference past L2C_BIG_CTL[MAXDRAM] occurred. See L2C_TAD()_ERR for logged information. */
	uint64_t holerd                       : 1;  /**< Read reference to 256MB hole occurred. */
	uint64_t holewr                       : 1;  /**< Write reference to 256MB hole occurred. */
	uint64_t reserved_2_10                : 9;
	uint64_t l2ddbe                       : 1;  /**< L2D double-bit error occurred. See L2C_TAD()_TQD_ERR for logged information. An
                                                         indication of a hardware failure and may be considered fatal. */
	uint64_t l2dsbe                       : 1;  /**< L2D single-bit error on a read. See L2C_TAD()_TQD_ERR for logged
                                                         information. When [L2DSBE] is set, hardware corrected the error before using the
                                                         data, but did not correct any stored value. When [L2DSBE] is set, software
                                                         should eject the cache block indicated by the corresponding
                                                         L2C_TAD()_TQD_ERR[QDNUM,L2DIDX] before clearing [L2DSBE]. Otherwise, hardware
                                                         may encounter the error again the next time the same L2D location is
                                                         referenced. Software may also choose to count the number of these single-bit
                                                         errors.
                                                         The eject should use a CACHE 0x3 instruction with an effective address of:
                                                         <pre>
                                                           payload<24:22> = 0
                                                           payload<21:18> = L2C_TAD()_TQD_ERR[L2DIDX]<10:7>  // way
                                                           payload<17:11> = L2C_TAD()_TQD_ERR[L2DIDX]<6:0>   // index<10:4>
                                                           payload<10:9>  = L2C_TAD()_TQD_ERR[L2DIDX]<12:11> // index<3:2>
                                                           payload<8:7>   = tad             // index<1:0>
                                                         </pre>
                                                         where tad is the TAD index from this CSR. Note that L2C_CTL[DISIDXALIAS] has no
                                                         effect on the payload. */
#else
	uint64_t l2dsbe                       : 1;
	uint64_t l2ddbe                       : 1;
	uint64_t reserved_2_10                : 9;
	uint64_t holewr                       : 1;
	uint64_t holerd                       : 1;
	uint64_t bigwr                        : 1;
	uint64_t bigrd                        : 1;
	uint64_t reserved_15_16               : 2;
	uint64_t lfbto                        : 1;
	uint64_t reserved_18_31               : 14;
	uint64_t rtgsbe                       : 1;
	uint64_t rtgdbe                       : 1;
	uint64_t rddisoci                     : 1;
	uint64_t wrdisoci                     : 1;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_l2c_tadx_int_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t wrdislmc                     : 1;  /**< Illegal Write to Disabled LMC Error
                                                         A DRAM write arrived before the LMC(s) were enabled */
	uint64_t rddislmc                     : 1;  /**< Illegal Read  to Disabled LMC Error
                                                         A DRAM read  arrived before the LMC(s) were enabled */
	uint64_t noway                        : 1;  /**< No way available interrupt
                                                         Shadow copy of L2C_ERR_TTGX[NOWAY]
                                                         Writes of 1 also clear L2C_ERR_TTGX[NOWAY] */
	uint64_t vbfdbe                       : 1;  /**< VBF Double-Bit Error
                                                         Shadow copy of L2C_ERR_TDTX[VDBE]
                                                         Writes of 1 also clear L2C_ERR_TDTX[VDBE] */
	uint64_t vbfsbe                       : 1;  /**< VBF Single-Bit Error
                                                         Shadow copy of L2C_ERR_TDTX[VSBE]
                                                         Writes of 1 also clear L2C_ERR_TDTX[VSBE] */
	uint64_t tagdbe                       : 1;  /**< TAG Double-Bit Error
                                                         Shadow copy of L2C_ERR_TTGX[DBE]
                                                         Writes of 1 also clear L2C_ERR_TTGX[DBE] */
	uint64_t tagsbe                       : 1;  /**< TAG Single-Bit Error
                                                         Shadow copy of L2C_ERR_TTGX[SBE]
                                                         Writes of 1 also clear L2C_ERR_TTGX[SBE] */
	uint64_t l2ddbe                       : 1;  /**< L2D Double-Bit Error
                                                         Shadow copy of L2C_ERR_TDTX[DBE]
                                                         Writes of 1 also clear L2C_ERR_TDTX[DBE] */
	uint64_t l2dsbe                       : 1;  /**< L2D Single-Bit Error
                                                         Shadow copy of L2C_ERR_TDTX[SBE]
                                                         Writes of 1 also clear L2C_ERR_TDTX[SBE] */
#else
	uint64_t l2dsbe                       : 1;
	uint64_t l2ddbe                       : 1;
	uint64_t tagsbe                       : 1;
	uint64_t tagdbe                       : 1;
	uint64_t vbfsbe                       : 1;
	uint64_t vbfdbe                       : 1;
	uint64_t noway                        : 1;
	uint64_t rddislmc                     : 1;
	uint64_t wrdislmc                     : 1;
	uint64_t reserved_9_63                : 55;
#endif
	} cn61xx;
	struct cvmx_l2c_tadx_int_cn61xx       cn63xx;
	struct cvmx_l2c_tadx_int_cn61xx       cn66xx;
	struct cvmx_l2c_tadx_int_cn61xx       cn68xx;
	struct cvmx_l2c_tadx_int_cn61xx       cn68xxp1;
	struct cvmx_l2c_tadx_int_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_34_63               : 30;
	uint64_t rtgdbe                       : 1;  /**< RTG double-bit error */
	uint64_t rtgsbe                       : 1;  /**< RTG single-bit error */
	uint64_t reserved_17_31               : 15;
	uint64_t wrdislmc                     : 1;  /**< Illegal write to disabled LMC error. A DRAM write arrived before the LMC(s) were enabled. */
	uint64_t rddislmc                     : 1;  /**< Illegal read to disabled LMC error. A DRAM read arrived before the LMC(s) were enabled. */
	uint64_t bigrd                        : 1;  /**< Read reference past L2C_BIG_CTL[MAXDRAM] occurred. */
	uint64_t bigwr                        : 1;  /**< Write reference past L2C_BIG_CTL[MAXDRAM] occurred. */
	uint64_t holerd                       : 1;  /**< Read reference to 256MB hole occurred. */
	uint64_t holewr                       : 1;  /**< Write reference to 256MB hole occurred. */
	uint64_t noway                        : 1;  /**< No way was available for allocation. L2C sets [NOWAY] during its processing of a
                                                         transaction
                                                         whenever it needed/wanted to allocate a WAY in the L2 cache, but was unable to. When this
                                                         bit = 1, it is (generally) not an indication that L2C failed to complete transactions.
                                                         Rather, it is a hint of possible performance degradation. (For example, L2C must read-
                                                         modify-write DRAM for every transaction that updates some, but not all, of the bytes in a
                                                         cache block, misses in the L2 cache, and cannot allocate a WAY.) There is one 'failure'
                                                         case where L2C sets [NOWAY]: when it cannot leave a block locked in the L2 cache as part
                                                         of
                                                         a LCKL2 transaction. See L2C_TTG()_ERR for logged information. */
	uint64_t tagdbe                       : 1;  /**< TAG double-bit error occurred. See L2C_TTG()_ERR for logged information. */
	uint64_t tagsbe                       : 1;  /**< TAG single-bit error occurred. See L2C_TTG()_ERR for logged information. */
	uint64_t reserved_6_7                 : 2;
	uint64_t fbfdbe                       : 1;  /**< FBF double-bit error occurred. See L2C_TQD()_ERR for logged information. */
	uint64_t fbfsbe                       : 1;  /**< FBF single-bit error occurred. See L2C_TQD()_ERR for logged information. */
	uint64_t sbfdbe                       : 1;  /**< SBF double-bit error occurred. See L2C_TQD()_ERR for logged information. */
	uint64_t sbfsbe                       : 1;  /**< SBF single-bit error occurred. See L2C_TQD()_ERR for logged information. */
	uint64_t l2ddbe                       : 1;  /**< L2D double-bit error occurred. See L2C_TQD()_ERR for logged information. */
	uint64_t l2dsbe                       : 1;  /**< L2D single-bit error occurred. See L2C_TQD()_ERR for logged information. */
#else
	uint64_t l2dsbe                       : 1;
	uint64_t l2ddbe                       : 1;
	uint64_t sbfsbe                       : 1;
	uint64_t sbfdbe                       : 1;
	uint64_t fbfsbe                       : 1;
	uint64_t fbfdbe                       : 1;
	uint64_t reserved_6_7                 : 2;
	uint64_t tagsbe                       : 1;
	uint64_t tagdbe                       : 1;
	uint64_t noway                        : 1;
	uint64_t holewr                       : 1;
	uint64_t holerd                       : 1;
	uint64_t bigwr                        : 1;
	uint64_t bigrd                        : 1;
	uint64_t rddislmc                     : 1;
	uint64_t wrdislmc                     : 1;
	uint64_t reserved_17_31               : 15;
	uint64_t rtgsbe                       : 1;
	uint64_t rtgdbe                       : 1;
	uint64_t reserved_34_63               : 30;
#endif
	} cn70xx;
	struct cvmx_l2c_tadx_int_cn70xx       cn70xxp1;
	struct cvmx_l2c_tadx_int_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t wrdisoci                     : 1;  /**< Reserved. */
	uint64_t rddisoci                     : 1;  /**< Reserved. */
	uint64_t rtgdbe                       : 1;  /**< RTG double-bit error.
                                                         See L2C_TAD()_RTG_ERR for logged information.
                                                         An indication of a hardware failure and may be considered fatal. */
	uint64_t rtgsbe                       : 1;  /**< RTG single-bit error on a read. See L2C_TAD()_RTG_ERR for logged
                                                         information. When [RTGSBE] is set, hardware corrected the error before using the
                                                         RTG tag, but did not correct any stored value. When [RTGSBE] is set, software
                                                         should eject the RTG location indicated by the corresponding
                                                         L2C_TAD()_RTG_ERR[WAY,L2IDX] before clearing [RTGSBE]. Otherwise, hardware may
                                                         encounter the error again the next time the same RTG location is
                                                         referenced. Software may also choose to count the number of these single-bit
                                                         errors.
                                                         The eject should use a CACHE 0x3 instruction with an effective address of:
                                                         <pre>
                                                           payload<24> = 1
                                                           payload<23:20> = L2C_TAD()_RTG_ERR[WAY]
                                                           payload<19:7>  = L2C_TAD()_RTG_ERR[L2IDX]
                                                         </pre> */
	uint64_t reserved_18_31               : 14;
	uint64_t lfbto                        : 1;  /**< An LFB entry (or more) has encountered a timeout condition When LFBTO timeout condition
                                                         occurs L2C_TAD()_TIMEOUT is loaded. L2C_TAD()_TIMEOUT is loaded with info from the
                                                         first LFB that timed out. if multiple LFB timed out simultaneously, then the it will
                                                         capture info from the lowest LFB number that timed out.
                                                         Should not occur during normal operation.  OCI/CCPI link failures may cause this
                                                         failure. This may be an indication of hardware failure, and may be considered
                                                         fatal. */
	uint64_t wrdislmc                     : 1;  /**< Illegal write to disabled LMC error. A DRAM write arrived before LMC was enabled.
                                                         Should not occur during normal operation.
                                                         This may be considered fatal. */
	uint64_t rddislmc                     : 1;  /**< Illegal read to disabled LMC error. A DRAM read arrived before LMC was enabled.
                                                         Should not occur during normal operation.
                                                         This may be considered fatal. */
	uint64_t bigrd                        : 1;  /**< Read reference past L2C_BIG_CTL[MAXDRAM] occurred. [BIGRD] interrupts can occur during
                                                         normal operation as the cores are allowed to prefetch to nonexistent memory locations.
                                                         Therefore, [BIGRD] is for informational purposes only. See L2C_TAD()_ERR for logged
                                                         information. */
	uint64_t bigwr                        : 1;  /**< Write reference past L2C_BIG_CTL[MAXDRAM] occurred. See L2C_TAD()_ERR for logged information. */
	uint64_t reserved_11_12               : 2;
	uint64_t noway                        : 1;  /**< No way was available for allocation. L2C sets [NOWAY] during its processing of a
                                                         transaction
                                                         whenever it needed/wanted to allocate a WAY in the L2 cache, but was unable to. When this
                                                         bit = 1, it is (generally) not an indication that L2C failed to complete transactions.
                                                         Rather, it is a hint of possible performance degradation. (For example, L2C must read-
                                                         modify-write DRAM for every transaction that updates some, but not all, of the bytes in a
                                                         cache block, misses in the L2 cache, and cannot allocate a WAY.) There is one 'failure'
                                                         case where L2C sets [NOWAY]: when it cannot leave a block locked in the L2 cache as part
                                                         of
                                                         a LCKL2 transaction. See L2C_TTG()_ERR for logged information. */
	uint64_t tagdbe                       : 1;  /**< TAG double-bit error occurred. See L2C_TTG()_ERR for logged information.
                                                         This is an indication of a hardware failure and may be considered fatal. */
	uint64_t tagsbe                       : 1;  /**< TAG single-bit error on a read. See L2C_TAD()_TTG_ERR for logged
                                                         information. When [TAGSBE] is set, hardware corrected the error before using the
                                                         tag, but did not correct any stored value. When [TAGSBE] is set, software should
                                                         eject the TAG location indicated by the corresponding
                                                         L2C_TAD()_TTG_ERR[WAY,L2IDX] before clearing [TAGSBE]. Otherwise, hardware may
                                                         encounter the error again the next time the same TAG location is
                                                         referenced. Software may also choose to count the number of these single-bit
                                                         errors.
                                                         The eject should use a CACHE 0x3 instruction with an effective address of:
                                                           <pre>
                                                           payload<24> = 0
                                                           payload<23:20> = L2C_TAD()_TTG_ERR[WAY]
                                                           payload<19:7>  = L2C_TAD()_TTG_ERR[L2IDX]
                                                           </pre>
                                                         Note that L2C_CTL[DISIDXALIAS] has no effect on this payload. */
	uint64_t reserved_6_7                 : 2;
	uint64_t fbfdbe                       : 1;  /**< FBF double-bit error occurred. See L2C_TAD()_TQD_ERR for logged information. An
                                                         indication of a hardware failure and may be considered fatal. */
	uint64_t fbfsbe                       : 1;  /**< FBF single-bit error on a read. See L2C_TAD()_TQD_ERR for logged
                                                         information. Hardware automatically corrected the error. Software may choose to
                                                         count the number of these single-bit errors. */
	uint64_t sbfdbe                       : 1;  /**< SBF double-bit error occurred. See L2C_TAD()_TQD_ERR for logged information. An
                                                         indication of a hardware failure and may be considered fatal. */
	uint64_t sbfsbe                       : 1;  /**< SBF single-bit error on a read. See L2C_TAD()_TQD_ERR for logged
                                                         information. Hardware automatically corrected the error. Software may choose to
                                                         count the number of these single-bit errors. */
	uint64_t l2ddbe                       : 1;  /**< L2D double-bit error occurred. See L2C_TAD()_TQD_ERR for logged information. An
                                                         indication of a hardware failure and may be considered fatal. */
	uint64_t l2dsbe                       : 1;  /**< L2D single-bit error on a read. See L2C_TAD()_TQD_ERR for logged
                                                         information. When [L2DSBE] is set, hardware corrected the error before using the
                                                         data, but did not correct any stored value. When [L2DSBE] is set, software
                                                         should eject the cache block indicated by the corresponding
                                                         L2C_TAD()_TQD_ERR[QDNUM,L2DIDX] before clearing [L2DSBE]. Otherwise, hardware
                                                         may encounter the error again the next time the same L2D location is
                                                         referenced. Software may also choose to count the number of these single-bit
                                                         errors.
                                                         The eject should use a CACHE 0x3 instruction with an effective address of:
                                                         <pre>
                                                           payload<24:22> = 0
                                                           payload<21:18> = L2C_TAD()_TQD_ERR[L2DIDX]<10:7>  // way
                                                           payload<17:11> = L2C_TAD()_TQD_ERR[L2DIDX]<6:0>   // index<10:4>
                                                           payload<10:9>  = L2C_TAD()_TQD_ERR[L2DIDX]<12:11> // index<3:2>
                                                           payload<8:7>   = tad             // index<1:0>
                                                         </pre>
                                                         where tad is the TAD index from this CSR. Note that L2C_CTL[DISIDXALIAS] has no
                                                         effect on the payload. */
#else
	uint64_t l2dsbe                       : 1;
	uint64_t l2ddbe                       : 1;
	uint64_t sbfsbe                       : 1;
	uint64_t sbfdbe                       : 1;
	uint64_t fbfsbe                       : 1;
	uint64_t fbfdbe                       : 1;
	uint64_t reserved_6_7                 : 2;
	uint64_t tagsbe                       : 1;
	uint64_t tagdbe                       : 1;
	uint64_t noway                        : 1;
	uint64_t reserved_11_12               : 2;
	uint64_t bigwr                        : 1;
	uint64_t bigrd                        : 1;
	uint64_t rddislmc                     : 1;
	uint64_t wrdislmc                     : 1;
	uint64_t lfbto                        : 1;
	uint64_t reserved_18_31               : 14;
	uint64_t rtgsbe                       : 1;
	uint64_t rtgdbe                       : 1;
	uint64_t rddisoci                     : 1;
	uint64_t wrdisoci                     : 1;
	uint64_t reserved_36_63               : 28;
#endif
	} cn73xx;
	struct cvmx_l2c_tadx_int_cn73xx       cn78xx;
	struct cvmx_l2c_tadx_int_cn73xx       cn78xxp1;
	struct cvmx_l2c_tadx_int_cn61xx       cnf71xx;
	struct cvmx_l2c_tadx_int_cn73xx       cnf75xx;
};
typedef union cvmx_l2c_tadx_int cvmx_l2c_tadx_int_t;

/**
 * cvmx_l2c_tad#_pfc#
 */
union cvmx_l2c_tadx_pfcx {
	uint64_t u64;
	struct cvmx_l2c_tadx_pfcx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t count                        : 64; /**< Current counter value */
#else
	uint64_t count                        : 64;
#endif
	} s;
	struct cvmx_l2c_tadx_pfcx_s           cn70xx;
	struct cvmx_l2c_tadx_pfcx_s           cn70xxp1;
	struct cvmx_l2c_tadx_pfcx_s           cn73xx;
	struct cvmx_l2c_tadx_pfcx_s           cn78xx;
	struct cvmx_l2c_tadx_pfcx_s           cn78xxp1;
	struct cvmx_l2c_tadx_pfcx_s           cnf75xx;
};
typedef union cvmx_l2c_tadx_pfcx cvmx_l2c_tadx_pfcx_t;

/**
 * cvmx_l2c_tad#_pfc0
 *
 * L2C_TAD_PFC0 = L2C TAD Performance Counter 0
 *
 */
union cvmx_l2c_tadx_pfc0 {
	uint64_t u64;
	struct cvmx_l2c_tadx_pfc0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t count                        : 64; /**< Current counter value */
#else
	uint64_t count                        : 64;
#endif
	} s;
	struct cvmx_l2c_tadx_pfc0_s           cn61xx;
	struct cvmx_l2c_tadx_pfc0_s           cn63xx;
	struct cvmx_l2c_tadx_pfc0_s           cn63xxp1;
	struct cvmx_l2c_tadx_pfc0_s           cn66xx;
	struct cvmx_l2c_tadx_pfc0_s           cn68xx;
	struct cvmx_l2c_tadx_pfc0_s           cn68xxp1;
	struct cvmx_l2c_tadx_pfc0_s           cnf71xx;
};
typedef union cvmx_l2c_tadx_pfc0 cvmx_l2c_tadx_pfc0_t;

/**
 * cvmx_l2c_tad#_pfc1
 *
 * L2C_TAD_PFC1 = L2C TAD Performance Counter 1
 *
 */
union cvmx_l2c_tadx_pfc1 {
	uint64_t u64;
	struct cvmx_l2c_tadx_pfc1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t count                        : 64; /**< Current counter value */
#else
	uint64_t count                        : 64;
#endif
	} s;
	struct cvmx_l2c_tadx_pfc1_s           cn61xx;
	struct cvmx_l2c_tadx_pfc1_s           cn63xx;
	struct cvmx_l2c_tadx_pfc1_s           cn63xxp1;
	struct cvmx_l2c_tadx_pfc1_s           cn66xx;
	struct cvmx_l2c_tadx_pfc1_s           cn68xx;
	struct cvmx_l2c_tadx_pfc1_s           cn68xxp1;
	struct cvmx_l2c_tadx_pfc1_s           cnf71xx;
};
typedef union cvmx_l2c_tadx_pfc1 cvmx_l2c_tadx_pfc1_t;

/**
 * cvmx_l2c_tad#_pfc2
 *
 * L2C_TAD_PFC2 = L2C TAD Performance Counter 2
 *
 */
union cvmx_l2c_tadx_pfc2 {
	uint64_t u64;
	struct cvmx_l2c_tadx_pfc2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t count                        : 64; /**< Current counter value */
#else
	uint64_t count                        : 64;
#endif
	} s;
	struct cvmx_l2c_tadx_pfc2_s           cn61xx;
	struct cvmx_l2c_tadx_pfc2_s           cn63xx;
	struct cvmx_l2c_tadx_pfc2_s           cn63xxp1;
	struct cvmx_l2c_tadx_pfc2_s           cn66xx;
	struct cvmx_l2c_tadx_pfc2_s           cn68xx;
	struct cvmx_l2c_tadx_pfc2_s           cn68xxp1;
	struct cvmx_l2c_tadx_pfc2_s           cnf71xx;
};
typedef union cvmx_l2c_tadx_pfc2 cvmx_l2c_tadx_pfc2_t;

/**
 * cvmx_l2c_tad#_pfc3
 *
 * L2C_TAD_PFC3 = L2C TAD Performance Counter 3
 *
 */
union cvmx_l2c_tadx_pfc3 {
	uint64_t u64;
	struct cvmx_l2c_tadx_pfc3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t count                        : 64; /**< Current counter value */
#else
	uint64_t count                        : 64;
#endif
	} s;
	struct cvmx_l2c_tadx_pfc3_s           cn61xx;
	struct cvmx_l2c_tadx_pfc3_s           cn63xx;
	struct cvmx_l2c_tadx_pfc3_s           cn63xxp1;
	struct cvmx_l2c_tadx_pfc3_s           cn66xx;
	struct cvmx_l2c_tadx_pfc3_s           cn68xx;
	struct cvmx_l2c_tadx_pfc3_s           cn68xxp1;
	struct cvmx_l2c_tadx_pfc3_s           cnf71xx;
};
typedef union cvmx_l2c_tadx_pfc3 cvmx_l2c_tadx_pfc3_t;

/**
 * cvmx_l2c_tad#_prf
 *
 * All four counters are equivalent and can use any of the defined selects.
 *
 */
union cvmx_l2c_tadx_prf {
	uint64_t u64;
	struct cvmx_l2c_tadx_prf_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t cnt3sel                      : 8;  /**< Selects event to count for L2C_TAD(0..3)_PFC3. Enumerated by L2C_TAD_PRF_SEL_E. */
	uint64_t cnt2sel                      : 8;  /**< Selects event to count for L2C_TAD(0..3)_PFC2. Enumerated by L2C_TAD_PRF_SEL_E. */
	uint64_t cnt1sel                      : 8;  /**< Selects event to count for L2C_TAD(0..3)_PFC1. Enumerated by L2C_TAD_PRF_SEL_E. */
	uint64_t cnt0sel                      : 8;  /**< Selects event to count for L2C_TAD(0..3)_PFC0. Enumerated by L2C_TAD_PRF_SEL_E. */
#else
	uint64_t cnt0sel                      : 8;
	uint64_t cnt1sel                      : 8;
	uint64_t cnt2sel                      : 8;
	uint64_t cnt3sel                      : 8;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_l2c_tadx_prf_s            cn61xx;
	struct cvmx_l2c_tadx_prf_s            cn63xx;
	struct cvmx_l2c_tadx_prf_s            cn63xxp1;
	struct cvmx_l2c_tadx_prf_s            cn66xx;
	struct cvmx_l2c_tadx_prf_s            cn68xx;
	struct cvmx_l2c_tadx_prf_s            cn68xxp1;
	struct cvmx_l2c_tadx_prf_s            cn70xx;
	struct cvmx_l2c_tadx_prf_s            cn70xxp1;
	struct cvmx_l2c_tadx_prf_s            cn73xx;
	struct cvmx_l2c_tadx_prf_s            cn78xx;
	struct cvmx_l2c_tadx_prf_s            cn78xxp1;
	struct cvmx_l2c_tadx_prf_s            cnf71xx;
	struct cvmx_l2c_tadx_prf_s            cnf75xx;
};
typedef union cvmx_l2c_tadx_prf cvmx_l2c_tadx_prf_t;

/**
 * cvmx_l2c_tad#_stat
 *
 * This register holds information about the instantaneous state of the TAD.
 *
 */
union cvmx_l2c_tadx_stat {
	uint64_t u64;
	struct cvmx_l2c_tadx_stat_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t lfb_valid_cnt                : 6;  /**< The number of LFBs in use. */
	uint64_t reserved_5_7                 : 3;
	uint64_t vbf_inuse_cnt                : 5;  /**< The number of MCI VBFs in use. */
#else
	uint64_t vbf_inuse_cnt                : 5;
	uint64_t reserved_5_7                 : 3;
	uint64_t lfb_valid_cnt                : 6;
	uint64_t reserved_14_63               : 50;
#endif
	} s;
	struct cvmx_l2c_tadx_stat_s           cn73xx;
	struct cvmx_l2c_tadx_stat_s           cn78xx;
	struct cvmx_l2c_tadx_stat_s           cnf75xx;
};
typedef union cvmx_l2c_tadx_stat cvmx_l2c_tadx_stat_t;

/**
 * cvmx_l2c_tad#_tag
 *
 * This register holds the tag information for LTGL2I and STGL2I commands.
 *
 */
union cvmx_l2c_tadx_tag {
	uint64_t u64;
	struct cvmx_l2c_tadx_tag_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sblkdty                      : 4;  /**< Sub-block dirty bits. Ignored/loaded with 0 for RTG accesses. If TS is Invalid (0)
                                                         [SBLKDTY]
                                                         must be 0 or operation is undefined. */
	uint64_t reserved_6_59                : 54;
	uint64_t node                         : 2;  /**< Reserved. */
	uint64_t reserved_1_3                 : 3;
	uint64_t lock                         : 1;  /**< The lock bit. If setting [LOCK], the USE bit should also be set or the operation is
                                                         undefined.  Ignored/loaded with 0 for RTG accesses. */
#else
	uint64_t lock                         : 1;
	uint64_t reserved_1_3                 : 3;
	uint64_t node                         : 2;
	uint64_t reserved_6_59                : 54;
	uint64_t sblkdty                      : 4;
#endif
	} s;
	struct cvmx_l2c_tadx_tag_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_46_63               : 18;
	uint64_t ecc                          : 6;  /**< The tag ECC */
	uint64_t reserved_36_39               : 4;
	uint64_t tag                          : 19; /**< The tag (see notes 1 and 3) */
	uint64_t reserved_4_16                : 13;
	uint64_t use                          : 1;  /**< The LRU use bit */
	uint64_t valid                        : 1;  /**< The valid bit */
	uint64_t dirty                        : 1;  /**< The dirty bit */
	uint64_t lock                         : 1;  /**< The lock bit */
#else
	uint64_t lock                         : 1;
	uint64_t dirty                        : 1;
	uint64_t valid                        : 1;
	uint64_t use                          : 1;
	uint64_t reserved_4_16                : 13;
	uint64_t tag                          : 19;
	uint64_t reserved_36_39               : 4;
	uint64_t ecc                          : 6;
	uint64_t reserved_46_63               : 18;
#endif
	} cn61xx;
	struct cvmx_l2c_tadx_tag_cn61xx       cn63xx;
	struct cvmx_l2c_tadx_tag_cn61xx       cn63xxp1;
	struct cvmx_l2c_tadx_tag_cn61xx       cn66xx;
	struct cvmx_l2c_tadx_tag_cn61xx       cn68xx;
	struct cvmx_l2c_tadx_tag_cn61xx       cn68xxp1;
	struct cvmx_l2c_tadx_tag_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sblkdty                      : 4;  /**< Sub-block dirty bits. */
	uint64_t reserved_56_59               : 4;
	uint64_t businfo                      : 8;  /**< The businfo bits. Legal values: when [55]==1, we are in idmode and [54:50] must be 0,
                                                         [49:48] are the PPVID of the PP which could be holding the block; when [55]==0, we are in
                                                         bus mask mode and [54:49] must be 0 [48] is 1 if any of the PP's could contain the
                                                         block. Operation is undefined if an STGL2I causes an illegal value to be written to the L2
                                                         TAGs. LTGL2Is will only load legal values into this register. */
	uint64_t reserved_47_47               : 1;
	uint64_t ecc                          : 7;  /**< The tag ECC */
	uint64_t tag                          : 23; /**< The tag. The tag is the corresponding bits from the L2C+LMC internal L2/DRAM byte address. */
	uint64_t reserved_4_16                : 13;
	uint64_t used                         : 1;  /**< The LRU use bit. If setting [LOCK], the USE bit should also be set or the operation
                                                         is undefined. */
	uint64_t valid                        : 1;  /**< The valid bit */
	uint64_t dirty                        : 1;  /**< The dirty bit */
	uint64_t lock                         : 1;  /**< The lock bit. If setting [LOCK], the USE bit should also be set or the operation is
                                                         undefined. */
#else
	uint64_t lock                         : 1;
	uint64_t dirty                        : 1;
	uint64_t valid                        : 1;
	uint64_t used                         : 1;
	uint64_t reserved_4_16                : 13;
	uint64_t tag                          : 23;
	uint64_t ecc                          : 7;
	uint64_t reserved_47_47               : 1;
	uint64_t businfo                      : 8;
	uint64_t reserved_56_59               : 4;
	uint64_t sblkdty                      : 4;
#endif
	} cn70xx;
	struct cvmx_l2c_tadx_tag_cn70xx       cn70xxp1;
	struct cvmx_l2c_tadx_tag_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sblkdty                      : 4;  /**< Sub-block dirty bits. Ignored/loaded with 0 for RTG accesses. If TS is Invalid (0)
                                                         [SBLKDTY]
                                                         must be 0 or operation is undefined. */
	uint64_t reserved_58_59               : 2;
	uint64_t businfo                      : 9;  /**< The bus information bits. Legal values: when <57>==1, we are in idmode and
                                                         <56:53> must be 0, <52:49> are the PPVID of the PP which could be holding the
                                                         block; when <57>==0, we are in bus mask mode and <56:52> must be 0 and if any of
                                                         <51:48> is 1 then any of the PP's on that bus (3..0) could contain the
                                                         block. Operation is undefined if an STGL2I causes an illegal value to be written
                                                         to the L2 TAGs. LTGL2Is will only load legal values into this register. */
	uint64_t ecc                          : 7;  /**< The tag ECC. This field is undefined if L2C_CTL[DISECC] is not 1 when the LTGL2I reads the tags. */
	uint64_t reserved_40_41               : 2;
	uint64_t tag                          : 22; /**< The tag. TAG<39:18> is the corresponding bits from the L2C+LMC internal L2/DRAM byte
                                                         address. */
	uint64_t reserved_6_17                : 12;
	uint64_t node                         : 2;  /**< Reserved. */
	uint64_t ts                           : 2;  /**< The tag state.
                                                         0x0 = Invalid.
                                                         0x1 = Shared.
                                                         0x2 = Exclusive.
                                                         Note that a local address will never have the value of exclusive as that state is incloded
                                                         as shared in the TAG and invalid in the RTG. */
	uint64_t used                         : 1;  /**< The LRU use bit. If setting [LOCK], the USE bit should also be set or the operation
                                                         is undefined.  Ignored/loaded with 0 for RTG accesses. */
	uint64_t lock                         : 1;  /**< The lock bit. If setting [LOCK], the USE bit should also be set or the operation is
                                                         undefined.  Ignored/loaded with 0 for RTG accesses. */
#else
	uint64_t lock                         : 1;
	uint64_t used                         : 1;
	uint64_t ts                           : 2;
	uint64_t node                         : 2;
	uint64_t reserved_6_17                : 12;
	uint64_t tag                          : 22;
	uint64_t reserved_40_41               : 2;
	uint64_t ecc                          : 7;
	uint64_t businfo                      : 9;
	uint64_t reserved_58_59               : 2;
	uint64_t sblkdty                      : 4;
#endif
	} cn73xx;
	struct cvmx_l2c_tadx_tag_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t sblkdty                      : 4;  /**< Sub-block dirty bits. Ignored/loaded with 0 for RTG accesses. If TS is Invalid (0)
                                                         [SBLKDTY]
                                                         must be 0 or operation is undefined. */
	uint64_t reserved_58_59               : 2;
	uint64_t businfo                      : 9;  /**< The bus information bits. Ignored/loaded with 0 for RTG accesses. */
	uint64_t ecc                          : 7;  /**< The tag ECC. This field is undefined if L2C_CTL[DISECC] is not 1 when the LTGL2I reads the tags. */
	uint64_t tag                          : 22; /**< The tag. TAG<39:20> is the corresponding bits from the L2C+LMC internal L2/DRAM byte
                                                         address. TAG<41:40> is the CCPI node of the address. The RTG must always have the
                                                         TAG<41:40> equal to the current node or operation is undefined. */
	uint64_t reserved_6_19                : 14;
	uint64_t node                         : 2;  /**< The node ID for the remote node which holds this block. Ignored/loaded with 0 for TAG accesses. */
	uint64_t ts                           : 2;  /**< The tag state.
                                                         0x0 = Invalid.
                                                         0x1 = Shared.
                                                         0x2 = Exclusive.
                                                         Note that a local address will never have the value of exclusive as that state is incloded
                                                         as shared in the TAG and invalid in the RTG. */
	uint64_t used                         : 1;  /**< The LRU use bit. If setting [LOCK], the USE bit should also be set or the operation
                                                         is undefined.  Ignored/loaded with 0 for RTG accesses. */
	uint64_t lock                         : 1;  /**< The lock bit. If setting [LOCK], the USE bit should also be set or the operation is
                                                         undefined.  Ignored/loaded with 0 for RTG accesses. */
#else
	uint64_t lock                         : 1;
	uint64_t used                         : 1;
	uint64_t ts                           : 2;
	uint64_t node                         : 2;
	uint64_t reserved_6_19                : 14;
	uint64_t tag                          : 22;
	uint64_t ecc                          : 7;
	uint64_t businfo                      : 9;
	uint64_t reserved_58_59               : 2;
	uint64_t sblkdty                      : 4;
#endif
	} cn78xx;
	struct cvmx_l2c_tadx_tag_cn78xx       cn78xxp1;
	struct cvmx_l2c_tadx_tag_cn61xx       cnf71xx;
	struct cvmx_l2c_tadx_tag_cn73xx       cnf75xx;
};
typedef union cvmx_l2c_tadx_tag cvmx_l2c_tadx_tag_t;

/**
 * cvmx_l2c_tad#_timeout
 *
 * This register records error information for an LFBTO (LFB TimeOut). The first LFBTO error will
 * lock the register until the logged error type s cleared. If multiple LFBs timed out
 * simultaneously, then this will contain the information form the lowest LFB number that has
 * timed-out. The address can be for the original transaction address or the replacement address
 * (if both could have timed out, then the transaction address will be here).
 */
union cvmx_l2c_tadx_timeout {
	uint64_t u64;
	struct cvmx_l2c_tadx_timeout_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t infolfb                      : 1;  /**< Logged address information is for the LFB original transaction. */
	uint64_t infovab                      : 1;  /**< Logged address information is for the VAB (replacement). If both this and [INFOLFB] is
                                                         set,
                                                         then both could have timed out, but info captured is from the original LFB. */
	uint64_t reserved_57_61               : 5;
	uint64_t lfbnum                       : 5;  /**< The LFB number of the entry that timed out, and have its info captures in this register. */
	uint64_t cmd                          : 8;  /**< Encoding of XMC command causing error. */
	uint64_t node                         : 4;  /**< Home node of the address causing the error. Similar to [ADDR] below, this can be the
                                                         request address (if [INFOLFB] is set), else it is the replacement address (if [INFOLFB] is
                                                         clear & [INFOVAB] is set). */
	uint64_t addr                         : 33; /**< Cache line address causing the error. This can be either the request address or the
                                                         replacement (if [INFOLFB] is set), else it is the replacement address (if [INFOLFB] is
                                                         clear &
                                                         [INFOVAB] is set). This address is a physical address. L2C performs hole removal and index
                                                         aliasing (if enabled) on the written address and uses that for the command. This hole-
                                                         removed/index-aliased address is what is returned on a read of L2C_XMC_CMD. */
	uint64_t reserved_0_6                 : 7;
#else
	uint64_t reserved_0_6                 : 7;
	uint64_t addr                         : 33;
	uint64_t node                         : 4;
	uint64_t cmd                          : 8;
	uint64_t lfbnum                       : 5;
	uint64_t reserved_57_61               : 5;
	uint64_t infovab                      : 1;
	uint64_t infolfb                      : 1;
#endif
	} s;
	struct cvmx_l2c_tadx_timeout_s        cn73xx;
	struct cvmx_l2c_tadx_timeout_s        cn78xx;
	struct cvmx_l2c_tadx_timeout_s        cn78xxp1;
	struct cvmx_l2c_tadx_timeout_s        cnf75xx;
};
typedef union cvmx_l2c_tadx_timeout cvmx_l2c_tadx_timeout_t;

/**
 * cvmx_l2c_tad#_timetwo
 *
 * This register records the number of LFB entries that have timed out.
 *
 */
union cvmx_l2c_tadx_timetwo {
	uint64_t u64;
	struct cvmx_l2c_tadx_timetwo_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_33_63               : 31;
	uint64_t sid                          : 4;  /**< Source id of the original request, that is 'source' of request. This is only valid if the
                                                         request is a local request (valid if L2C_TAD()_TIMEOUT[CMD] is an XMC request and not
                                                         relevant if it is an CCPI request). */
	uint64_t busid                        : 4;  /**< Busid of the original request, that is 'source' of request. */
	uint64_t vabst                        : 3;  /**< This is the LFB internal state if INFOLFB is set, else will contain VAB internal state if
                                                         INFOVAB is set. */
	uint64_t lfbst                        : 14; /**< This is the LFB internal state if INFOLFB is set, else will contain VAB internal state if
                                                         INFOVAB is set. */
	uint64_t tocnt                        : 8;  /**< This is a running count of the LFB that has timed out ... the count will saturate at 0xFF.
                                                         Will clear when the LFBTO interrupt is cleared. */
#else
	uint64_t tocnt                        : 8;
	uint64_t lfbst                        : 14;
	uint64_t vabst                        : 3;
	uint64_t busid                        : 4;
	uint64_t sid                          : 4;
	uint64_t reserved_33_63               : 31;
#endif
	} s;
	struct cvmx_l2c_tadx_timetwo_s        cn73xx;
	struct cvmx_l2c_tadx_timetwo_s        cn78xx;
	struct cvmx_l2c_tadx_timetwo_s        cn78xxp1;
	struct cvmx_l2c_tadx_timetwo_s        cnf75xx;
};
typedef union cvmx_l2c_tadx_timetwo cvmx_l2c_tadx_timetwo_t;

/**
 * cvmx_l2c_tad_ctl
 *
 * In CNXXXX, MAXLFB, EXLRQ, EXRRQ, EXFWD, EXVIC refer to half-TAD LFBs/VABs. Therefore, even
 * though there are 24 LFBs/VABs in a full TAD, the number applies to both halves.
 * * If [MAXLFB] is written to 0 or 13-15 operation is undefined. (CN78XX pass 1.0).
 * * If [MAXLFB] is != 0, [VBF_THRESH] should be less than [MAXLFB].
 * * If [MAXVBF] is != 0, [VBF_THRESH] should be less than [MAXVBF].
 * * If [MAXLFB] != 0, [EXLRQ] + [EXRRQ] + [EXFWD] + [EXVIC] must be less than or equal to MAXLFB
 * - 3.
 */
union cvmx_l2c_tad_ctl {
	uint64_t u64;
	struct cvmx_l2c_tad_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_33_63               : 31;
	uint64_t frcnalc                      : 1;  /**< When set, all cache accesses are forced to not allocate in the local L2. */
	uint64_t disrstp                      : 1;  /**< Reserved. */
	uint64_t wtlmcwrdn                    : 1;  /**< Be more conservative with LFB done relative to LMC writes. */
	uint64_t wtinvdn                      : 1;  /**< Be more conservative with LFB done relative to invalidates. */
	uint64_t wtfilldn                     : 1;  /**< Be more conservative with LFB done relative to fills. */
	uint64_t exlrq                        : 4;  /**< Reserved. */
	uint64_t exrrq                        : 4;  /**< Reserved. */
	uint64_t exfwd                        : 4;  /**< Reserved. */
	uint64_t exvic                        : 4;  /**< Reserved. */
	uint64_t vbf_thresh                   : 4;  /**< VBF threshold. When the number of in-use VBFs exceeds this number the L2C TAD increases
                                                         the priority of all its write operations in the LMC.
                                                         If [MAXLFB] is != 0x0, [VBF_THRESH] should be less than [MAXLFB].
                                                         If [MAXVBF] is != 0x0, [VBF_THRESH] should be less than [MAXVBF]. */
	uint64_t maxvbf                       : 4;  /**< Maximum VBFs in use at once (0 means 16, 1-15 as expected). */
	uint64_t maxlfb                       : 4;  /**< Maximum VABs/LFBs in use at once (0 means 16, 1-15 as expected). */
#else
	uint64_t maxlfb                       : 4;
	uint64_t maxvbf                       : 4;
	uint64_t vbf_thresh                   : 4;
	uint64_t exvic                        : 4;
	uint64_t exfwd                        : 4;
	uint64_t exrrq                        : 4;
	uint64_t exlrq                        : 4;
	uint64_t wtfilldn                     : 1;
	uint64_t wtinvdn                      : 1;
	uint64_t wtlmcwrdn                    : 1;
	uint64_t disrstp                      : 1;
	uint64_t frcnalc                      : 1;
	uint64_t reserved_33_63               : 31;
#endif
	} s;
	struct cvmx_l2c_tad_ctl_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_11_63               : 53;
	uint64_t vbf_thresh                   : 3;  /**< VBF threshold. When the number of in-use VBFs exceeds this number the L2C TAD increases
                                                         the priority of all its write operations in the LMC. */
	uint64_t reserved_7_7                 : 1;
	uint64_t maxvbf                       : 3;  /**< Maximum VABs/LFBs in use at once (0 means 16, 1-15 as expected). */
	uint64_t reserved_3_3                 : 1;
	uint64_t maxlfb                       : 3;  /**< Maximum VABs/LFBs in use at once (0 means 8, 1-7 as expected) */
#else
	uint64_t maxlfb                       : 3;
	uint64_t reserved_3_3                 : 1;
	uint64_t maxvbf                       : 3;
	uint64_t reserved_7_7                 : 1;
	uint64_t vbf_thresh                   : 3;
	uint64_t reserved_11_63               : 53;
#endif
	} cn70xx;
	struct cvmx_l2c_tad_ctl_cn70xx        cn70xxp1;
	struct cvmx_l2c_tad_ctl_s             cn73xx;
	struct cvmx_l2c_tad_ctl_s             cn78xx;
	struct cvmx_l2c_tad_ctl_s             cn78xxp1;
	struct cvmx_l2c_tad_ctl_s             cnf75xx;
};
typedef union cvmx_l2c_tad_ctl cvmx_l2c_tad_ctl_t;

/**
 * cvmx_l2c_tbf#_bist_status
 */
union cvmx_l2c_tbfx_bist_status {
	uint64_t u64;
	struct cvmx_l2c_tbfx_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t vbffl                        : 16; /**< BIST failure status for VBF ([QD7H1,QD7H0, ... , QD0H1, QD0H0]) */
	uint64_t sbffl                        : 16; /**< BIST failure status for SBF ([QD7H1,QD7H0, ... , QD0H1, QD0H0]) */
	uint64_t fbfrspfl                     : 16; /**< BIST failure status for FBF RSP port ([QD7H1,QD7H0, ... , QD0H1, QD0H0]) */
	uint64_t fbfwrpfl                     : 16; /**< BIST failure status for FBF WRP port ([QD7H1,QD7H0, ... , QD0H1, QD0H0]) */
#else
	uint64_t fbfwrpfl                     : 16;
	uint64_t fbfrspfl                     : 16;
	uint64_t sbffl                        : 16;
	uint64_t vbffl                        : 16;
#endif
	} s;
	struct cvmx_l2c_tbfx_bist_status_s    cn70xx;
	struct cvmx_l2c_tbfx_bist_status_s    cn70xxp1;
	struct cvmx_l2c_tbfx_bist_status_s    cn73xx;
	struct cvmx_l2c_tbfx_bist_status_s    cn78xx;
	struct cvmx_l2c_tbfx_bist_status_s    cn78xxp1;
	struct cvmx_l2c_tbfx_bist_status_s    cnf75xx;
};
typedef union cvmx_l2c_tbfx_bist_status cvmx_l2c_tbfx_bist_status_t;

/**
 * cvmx_l2c_tdt#_bist_status
 */
union cvmx_l2c_tdtx_bist_status {
	uint64_t u64;
	struct cvmx_l2c_tdtx_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t l2dfl                        : 16; /**< BIST failure status for L2D ([QD7H1,QD7H0, ... , QD0H1, QD0H0]) */
#else
	uint64_t l2dfl                        : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_l2c_tdtx_bist_status_s    cn70xx;
	struct cvmx_l2c_tdtx_bist_status_s    cn70xxp1;
	struct cvmx_l2c_tdtx_bist_status_s    cn73xx;
	struct cvmx_l2c_tdtx_bist_status_s    cn78xx;
	struct cvmx_l2c_tdtx_bist_status_s    cn78xxp1;
	struct cvmx_l2c_tdtx_bist_status_s    cnf75xx;
};
typedef union cvmx_l2c_tdtx_bist_status cvmx_l2c_tdtx_bist_status_t;

/**
 * cvmx_l2c_tqd#_err
 *
 * This register records error information for all L2D/SBF/FBF errors.
 * An error locks [L2DIDX] and [SYN] and sets the bit corresponding to the error received.
 * DBE errors take priority and overwrite an earlier logged SBE error. Only one of SBE/DBE is set
 * at any given time and serves to document which error the [L2DIDX]/[SYN] is associated with.
 * The syndrome is recorded for DBE errors, though the utility of the value is not clear.
 */
union cvmx_l2c_tqdx_err {
	uint64_t u64;
	struct cvmx_l2c_tqdx_err_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t l2ddbe                       : 1;  /**< L2DIDX/SYN corresponds to a double-bit L2D ECC error. */
	uint64_t sbfdbe                       : 1;  /**< L2DIDX/SYN corresponds to a double-bit SBF ECC error. */
	uint64_t fbfdbe                       : 1;  /**< L2DIDX/SYN corresponds to a double-bit FBF ECC error. */
	uint64_t l2dsbe                       : 1;  /**< L2DIDX/SYN corresponds to a single-bit L2D ECC error. */
	uint64_t sbfsbe                       : 1;  /**< L2DIDX/SYN corresponds to a single-bit SBF ECC error. */
	uint64_t fbfsbe                       : 1;  /**< L2DIDX/SYN corresponds to a single-bit FBF ECC error. */
	uint64_t reserved_40_57               : 18;
	uint64_t syn                          : 8;  /**< Error syndrome. */
	uint64_t reserved_18_31               : 14;
	uint64_t qdnum                        : 3;  /**< Quad containing the error. */
	uint64_t qdhlf                        : 1;  /**< Quad half of the containing the error. */
	uint64_t l2didx                       : 14; /**< For L2D errors, index within the quad-half containing the error. For SBF and FBF errors
                                                         <13:5> is 0x0 and <4:0> is the index of the error (<4:1> is lfbnum<3:0>, <0> is addr<5>).
                                                         See L2C_TAD()_INT[L2DSBE] for an important use of this field. */
#else
	uint64_t l2didx                       : 14;
	uint64_t qdhlf                        : 1;
	uint64_t qdnum                        : 3;
	uint64_t reserved_18_31               : 14;
	uint64_t syn                          : 8;
	uint64_t reserved_40_57               : 18;
	uint64_t fbfsbe                       : 1;
	uint64_t sbfsbe                       : 1;
	uint64_t l2dsbe                       : 1;
	uint64_t fbfdbe                       : 1;
	uint64_t sbfdbe                       : 1;
	uint64_t l2ddbe                       : 1;
#endif
	} s;
	struct cvmx_l2c_tqdx_err_s            cn70xx;
	struct cvmx_l2c_tqdx_err_s            cn70xxp1;
	struct cvmx_l2c_tqdx_err_s            cn73xx;
	struct cvmx_l2c_tqdx_err_s            cn78xx;
	struct cvmx_l2c_tqdx_err_s            cn78xxp1;
	struct cvmx_l2c_tqdx_err_s            cnf75xx;
};
typedef union cvmx_l2c_tqdx_err cvmx_l2c_tqdx_err_t;

/**
 * cvmx_l2c_ttg#_bist_status
 */
union cvmx_l2c_ttgx_bist_status {
	uint64_t u64;
	struct cvmx_l2c_ttgx_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_50_63               : 14;
	uint64_t xmdmskfl                     : 2;  /**< BIST failure status for RSTP XMDMSK memories. */
	uint64_t rtgfl                        : 16; /**< Reserved. */
	uint64_t reserved_18_31               : 14;
	uint64_t lrulfbfl                     : 1;  /**< Reserved, always zero. */
	uint64_t lrufl                        : 1;  /**< BIST failure status for tag LRU. */
	uint64_t tagfl                        : 16; /**< BIST failure status for TAG ways. */
#else
	uint64_t tagfl                        : 16;
	uint64_t lrufl                        : 1;
	uint64_t lrulfbfl                     : 1;
	uint64_t reserved_18_31               : 14;
	uint64_t rtgfl                        : 16;
	uint64_t xmdmskfl                     : 2;
	uint64_t reserved_50_63               : 14;
#endif
	} s;
	struct cvmx_l2c_ttgx_bist_status_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t rtgfl                        : 16; /**< Always zero for 70xx. */
	uint64_t reserved_18_31               : 14;
	uint64_t lrulfbfl                     : 1;  /**< BIST failure status for LRULFB memory */
	uint64_t lrufl                        : 1;  /**< BIST failure status for tag LRU. */
	uint64_t tagfl                        : 16; /**< BIST failure status for TAG ways. */
#else
	uint64_t tagfl                        : 16;
	uint64_t lrufl                        : 1;
	uint64_t lrulfbfl                     : 1;
	uint64_t reserved_18_31               : 14;
	uint64_t rtgfl                        : 16;
	uint64_t reserved_48_63               : 16;
#endif
	} cn70xx;
	struct cvmx_l2c_ttgx_bist_status_cn70xx cn70xxp1;
	struct cvmx_l2c_ttgx_bist_status_s    cn73xx;
	struct cvmx_l2c_ttgx_bist_status_s    cn78xx;
	struct cvmx_l2c_ttgx_bist_status_s    cn78xxp1;
	struct cvmx_l2c_ttgx_bist_status_s    cnf75xx;
};
typedef union cvmx_l2c_ttgx_bist_status cvmx_l2c_ttgx_bist_status_t;

/**
 * cvmx_l2c_ttg#_err
 *
 * This register records error information for all TAG SBE/DBE/NOWAY errors.
 * The priority of errors (lowest to highest) is NOWAY, SBE, DBE. An error locks [SYN], [WAY],
 * and [L2IDX] for equal or lower priority errors until cleared by software.
 * The syndrome is recorded for DBE errors, though the utility of the value is not clear.
 * A NOWAY error does not change the value of [SYN] field, and leaves [WAY] unpredictable.
 * L2IDX<17:7> is the L2 block index associated with the command which had no way to allocate.
 */
union cvmx_l2c_ttgx_err {
	uint64_t u64;
	struct cvmx_l2c_ttgx_err_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t tagdbe                       : 1;  /**< Information refers to a double-bit TAG ECC error. */
	uint64_t tagsbe                       : 1;  /**< Information refers to a single-bit TAG ECC error. */
	uint64_t noway                        : 1;  /**< Information refers to a NOWAY error. */
	uint64_t reserved_39_60               : 22;
	uint64_t syn                          : 7;  /**< Syndrome for the single-bit error. */
	uint64_t reserved_0_31                : 32;
#else
	uint64_t reserved_0_31                : 32;
	uint64_t syn                          : 7;
	uint64_t reserved_39_60               : 22;
	uint64_t noway                        : 1;
	uint64_t tagsbe                       : 1;
	uint64_t tagdbe                       : 1;
#endif
	} s;
	struct cvmx_l2c_ttgx_err_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t tagdbe                       : 1;  /**< Information refers to a double-bit TAG ECC error. */
	uint64_t tagsbe                       : 1;  /**< Information refers to a single-bit TAG ECC error. */
	uint64_t noway                        : 1;  /**< Information refers to a NOWAY error. */
	uint64_t reserved_39_60               : 22;
	uint64_t syn                          : 7;  /**< Syndrome for the single-bit error. */
	uint64_t reserved_19_31               : 13;
	uint64_t way                          : 2;  /**< Way of the L2 block containing the error */
	uint64_t l2idx                        : 10; /**< Index of the L2 block containing the error */
	uint64_t reserved_0_6                 : 7;
#else
	uint64_t reserved_0_6                 : 7;
	uint64_t l2idx                        : 10;
	uint64_t way                          : 2;
	uint64_t reserved_19_31               : 13;
	uint64_t syn                          : 7;
	uint64_t reserved_39_60               : 22;
	uint64_t noway                        : 1;
	uint64_t tagsbe                       : 1;
	uint64_t tagdbe                       : 1;
#endif
	} cn70xx;
	struct cvmx_l2c_ttgx_err_cn70xx       cn70xxp1;
	struct cvmx_l2c_ttgx_err_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t tagdbe                       : 1;  /**< Information refers to a double-bit TAG ECC error. */
	uint64_t tagsbe                       : 1;  /**< Information refers to a single-bit TAG ECC error. */
	uint64_t noway                        : 1;  /**< Information refers to a NOWAY error. */
	uint64_t reserved_39_60               : 22;
	uint64_t syn                          : 7;  /**< Syndrome for the single-bit error. */
	uint64_t reserved_22_31               : 10;
	uint64_t way                          : 4;  /**< Way of the L2 block containing the error. */
	uint64_t l2idx                        : 11; /**< Index of the L2 block containing the error.
                                                         See L2C_TAD()_INT[TAGSBE] for an important use of this field. */
	uint64_t reserved_0_6                 : 7;
#else
	uint64_t reserved_0_6                 : 7;
	uint64_t l2idx                        : 11;
	uint64_t way                          : 4;
	uint64_t reserved_22_31               : 10;
	uint64_t syn                          : 7;
	uint64_t reserved_39_60               : 22;
	uint64_t noway                        : 1;
	uint64_t tagsbe                       : 1;
	uint64_t tagdbe                       : 1;
#endif
	} cn73xx;
	struct cvmx_l2c_ttgx_err_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t tagdbe                       : 1;  /**< Information refers to a double-bit TAG ECC error. */
	uint64_t tagsbe                       : 1;  /**< Information refers to a single-bit TAG ECC error. */
	uint64_t noway                        : 1;  /**< Information refers to a NOWAY error. */
	uint64_t reserved_39_60               : 22;
	uint64_t syn                          : 7;  /**< Syndrome for the single-bit error. */
	uint64_t reserved_24_31               : 8;
	uint64_t way                          : 4;  /**< Way of the L2 block containing the error. */
	uint64_t l2idx                        : 13; /**< Index of the L2 block containing the error.
                                                         See L2C_TAD()_INT[TAGSBE] for an important use of this field. */
	uint64_t reserved_0_6                 : 7;
#else
	uint64_t reserved_0_6                 : 7;
	uint64_t l2idx                        : 13;
	uint64_t way                          : 4;
	uint64_t reserved_24_31               : 8;
	uint64_t syn                          : 7;
	uint64_t reserved_39_60               : 22;
	uint64_t noway                        : 1;
	uint64_t tagsbe                       : 1;
	uint64_t tagdbe                       : 1;
#endif
	} cn78xx;
	struct cvmx_l2c_ttgx_err_cn78xx       cn78xxp1;
	struct cvmx_l2c_ttgx_err_cn73xx       cnf75xx;
};
typedef union cvmx_l2c_ttgx_err cvmx_l2c_ttgx_err_t;

/**
 * cvmx_l2c_ver_id
 *
 * L2C_VER_ID = L2C Virtualization ID Error Register
 *
 * Description: records virtualization IDs associated with HOLEWR/BIGWR/VRTWR/VRTIDRNG/VRTADRNG interrupts.
 */
union cvmx_l2c_ver_id {
	uint64_t u64;
	struct cvmx_l2c_ver_id_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t mask                         : 64; /**< Mask of virtualization IDs which had a
                                                         HOLEWR/BIGWR/VRTWR error */
#else
	uint64_t mask                         : 64;
#endif
	} s;
	struct cvmx_l2c_ver_id_s              cn61xx;
	struct cvmx_l2c_ver_id_s              cn63xx;
	struct cvmx_l2c_ver_id_s              cn63xxp1;
	struct cvmx_l2c_ver_id_s              cn66xx;
	struct cvmx_l2c_ver_id_s              cn68xx;
	struct cvmx_l2c_ver_id_s              cn68xxp1;
	struct cvmx_l2c_ver_id_s              cnf71xx;
};
typedef union cvmx_l2c_ver_id cvmx_l2c_ver_id_t;

/**
 * cvmx_l2c_ver_iob
 *
 * L2C_VER_IOB = L2C Virtualization ID IOB Error Register
 *
 * Description: records IOBs associated with HOLEWR/BIGWR/VRTWR/VRTIDRNG/VRTADRNG interrupts.
 */
union cvmx_l2c_ver_iob {
	uint64_t u64;
	struct cvmx_l2c_ver_iob_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t mask                         : 2;  /**< Mask of IOBs which had a HOLEWR/BIGWR/VRTWR error */
#else
	uint64_t mask                         : 2;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_l2c_ver_iob_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t mask                         : 1;  /**< Mask of IOBs which had a HOLEWR/BIGWR/VRTWR error */
#else
	uint64_t mask                         : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} cn61xx;
	struct cvmx_l2c_ver_iob_cn61xx        cn63xx;
	struct cvmx_l2c_ver_iob_cn61xx        cn63xxp1;
	struct cvmx_l2c_ver_iob_cn61xx        cn66xx;
	struct cvmx_l2c_ver_iob_s             cn68xx;
	struct cvmx_l2c_ver_iob_s             cn68xxp1;
	struct cvmx_l2c_ver_iob_cn61xx        cnf71xx;
};
typedef union cvmx_l2c_ver_iob cvmx_l2c_ver_iob_t;

/**
 * cvmx_l2c_ver_msc
 *
 * L2C_VER_MSC = L2C Virtualization Miscellaneous Error Register (not in 63xx pass 1.x)
 *
 * Description: records type of command associated with HOLEWR/BIGWR/VRTWR/VRTIDRNG/VRTADRNG interrupts
 */
union cvmx_l2c_ver_msc {
	uint64_t u64;
	struct cvmx_l2c_ver_msc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t invl2                        : 1;  /**< If set, a INVL2 caused HOLEWR/BIGWR/VRT* to set */
	uint64_t dwb                          : 1;  /**< If set, a DWB caused HOLEWR/BIGWR/VRT* to set */
#else
	uint64_t dwb                          : 1;
	uint64_t invl2                        : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_l2c_ver_msc_s             cn61xx;
	struct cvmx_l2c_ver_msc_s             cn63xx;
	struct cvmx_l2c_ver_msc_s             cn66xx;
	struct cvmx_l2c_ver_msc_s             cn68xx;
	struct cvmx_l2c_ver_msc_s             cn68xxp1;
	struct cvmx_l2c_ver_msc_s             cnf71xx;
};
typedef union cvmx_l2c_ver_msc cvmx_l2c_ver_msc_t;

/**
 * cvmx_l2c_ver_pp
 *
 * L2C_VER_PP = L2C Virtualization ID PP Error Register
 *
 * Description: records PPs associated with HOLEWR/BIGWR/VRTWR/VRTIDRNG/VRTADRNG interrupts.
 */
union cvmx_l2c_ver_pp {
	uint64_t u64;
	struct cvmx_l2c_ver_pp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t mask                         : 32; /**< Mask of PPs which had a HOLEWR/BIGWR/VRTWR error */
#else
	uint64_t mask                         : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_l2c_ver_pp_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t mask                         : 4;  /**< Mask of PPs which had a HOLEWR/BIGWR/VRTWR error */
#else
	uint64_t mask                         : 4;
	uint64_t reserved_4_63                : 60;
#endif
	} cn61xx;
	struct cvmx_l2c_ver_pp_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t mask                         : 6;  /**< Mask of PPs which had a HOLEWR/BIGWR/VRTWR error */
#else
	uint64_t mask                         : 6;
	uint64_t reserved_6_63                : 58;
#endif
	} cn63xx;
	struct cvmx_l2c_ver_pp_cn63xx         cn63xxp1;
	struct cvmx_l2c_ver_pp_cn66xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t mask                         : 10; /**< Mask of PPs which had a HOLEWR/BIGWR/VRTWR error */
#else
	uint64_t mask                         : 10;
	uint64_t reserved_10_63               : 54;
#endif
	} cn66xx;
	struct cvmx_l2c_ver_pp_s              cn68xx;
	struct cvmx_l2c_ver_pp_s              cn68xxp1;
	struct cvmx_l2c_ver_pp_cn61xx         cnf71xx;
};
typedef union cvmx_l2c_ver_pp cvmx_l2c_ver_pp_t;

/**
 * cvmx_l2c_virtid_iob#
 *
 * L2C_VIRTID_IOB = L2C IOB virtualization ID
 *
 * Description:
 */
union cvmx_l2c_virtid_iobx {
	uint64_t u64;
	struct cvmx_l2c_virtid_iobx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t dwbid                        : 6;  /**< Virtualization ID to use for DWB commands */
	uint64_t reserved_6_7                 : 2;
	uint64_t id                           : 6;  /**< Virtualization ID to use for non-DWB commands */
#else
	uint64_t id                           : 6;
	uint64_t reserved_6_7                 : 2;
	uint64_t dwbid                        : 6;
	uint64_t reserved_14_63               : 50;
#endif
	} s;
	struct cvmx_l2c_virtid_iobx_s         cn61xx;
	struct cvmx_l2c_virtid_iobx_s         cn63xx;
	struct cvmx_l2c_virtid_iobx_s         cn63xxp1;
	struct cvmx_l2c_virtid_iobx_s         cn66xx;
	struct cvmx_l2c_virtid_iobx_s         cn68xx;
	struct cvmx_l2c_virtid_iobx_s         cn68xxp1;
	struct cvmx_l2c_virtid_iobx_s         cnf71xx;
};
typedef union cvmx_l2c_virtid_iobx cvmx_l2c_virtid_iobx_t;

/**
 * cvmx_l2c_virtid_pp#
 *
 * L2C_VIRTID_PP = L2C PP virtualization ID
 *
 * Description:
 */
union cvmx_l2c_virtid_ppx {
	uint64_t u64;
	struct cvmx_l2c_virtid_ppx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t id                           : 6;  /**< Virtualization ID to use for this PP. */
#else
	uint64_t id                           : 6;
	uint64_t reserved_6_63                : 58;
#endif
	} s;
	struct cvmx_l2c_virtid_ppx_s          cn61xx;
	struct cvmx_l2c_virtid_ppx_s          cn63xx;
	struct cvmx_l2c_virtid_ppx_s          cn63xxp1;
	struct cvmx_l2c_virtid_ppx_s          cn66xx;
	struct cvmx_l2c_virtid_ppx_s          cn68xx;
	struct cvmx_l2c_virtid_ppx_s          cn68xxp1;
	struct cvmx_l2c_virtid_ppx_s          cnf71xx;
};
typedef union cvmx_l2c_virtid_ppx cvmx_l2c_virtid_ppx_t;

/**
 * cvmx_l2c_vrt_ctl
 *
 * L2C_VRT_CTL = L2C Virtualization control register
 *
 */
union cvmx_l2c_vrt_ctl {
	uint64_t u64;
	struct cvmx_l2c_vrt_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t ooberr                       : 1;  /**< Whether out of bounds writes are an error
                                                         Determines virtualization hardware behavior for
                                                         a store to an L2/DRAM address larger than
                                                         indicated by MEMSZ. If OOBERR is set, all these
                                                         stores (from any virtualization ID) are blocked. If
                                                         OOBERR is clear, none of these stores are blocked. */
	uint64_t reserved_7_7                 : 1;
	uint64_t memsz                        : 3;  /**< Memory space coverage of L2C_VRT_MEM (encoded)
                                                         0 = 1GB
                                                         1 = 2GB
                                                         2 = 4GB
                                                         3 = 8GB
                                                         4 = 16GB
                                                         5 = 32GB
                                                         6 = 64GB (**reserved in 63xx**)
                                                         7 = 128GB (**reserved in 63xx**) */
	uint64_t numid                        : 3;  /**< Number of allowed virtualization IDs (encoded)
                                                             0 = 2
                                                             1 = 4
                                                             2 = 8
                                                             3 = 16
                                                             4 = 32
                                                             5 = 64
                                                             6,7 illegal
                                                         Violations of this limit causes
                                                         L2C to set L2C_INT_REG[VRTIDRNG]. */
	uint64_t enable                       : 1;  /**< Global virtualization enable
                                                         When ENABLE is clear, stores are never blocked by
                                                         the L2C virtualization hardware and none of NUMID,
                                                         MEMSZ, OOBERR are used. */
#else
	uint64_t enable                       : 1;
	uint64_t numid                        : 3;
	uint64_t memsz                        : 3;
	uint64_t reserved_7_7                 : 1;
	uint64_t ooberr                       : 1;
	uint64_t reserved_9_63                : 55;
#endif
	} s;
	struct cvmx_l2c_vrt_ctl_s             cn61xx;
	struct cvmx_l2c_vrt_ctl_s             cn63xx;
	struct cvmx_l2c_vrt_ctl_s             cn63xxp1;
	struct cvmx_l2c_vrt_ctl_s             cn66xx;
	struct cvmx_l2c_vrt_ctl_s             cn68xx;
	struct cvmx_l2c_vrt_ctl_s             cn68xxp1;
	struct cvmx_l2c_vrt_ctl_s             cnf71xx;
};
typedef union cvmx_l2c_vrt_ctl cvmx_l2c_vrt_ctl_t;

/**
 * cvmx_l2c_vrt_mem#
 *
 * L2C_VRT_MEM = L2C Virtualization Memory
 *
 * Description: Virtualization memory mapped region.  There are 1024 32b
 * byte-parity protected entries.
 *
 * Notes:
 * When a DATA bit is set in L2C_VRT_MEM when L2C virtualization is enabled, L2C
 * prevents the selected virtual machine from storing to the selected L2/DRAM region.
 * L2C uses L2C_VRT_MEM to block stores when:
 *  - L2C_VRT_CTL[ENABLE] is set, and
 *  - the address of the store exists in L2C+LMC internal L2/DRAM Address space
 *    and is within the L2C_VRT_CTL[MEMSZ] bounds, and
 *  - the virtID of the store is within the L2C_VRT_CTL[NUMID] bounds
 *
 * L2C_VRT_MEM is never used for these L2C transactions which are always allowed:
 *   - L2C CMI L2/DRAM transactions that cannot modify L2/DRAM, and
 *   - any L2/DRAM transaction originated from L2C_XMC_CMD
 *
 * L2C_VRT_MEM contains one DATA bit per L2C+LMC internal L2/DRAM region and virtID indicating whether the store
 * to the region is allowed. The granularity of the checking is the region size, which is:
 *       2 ^^ (L2C_VRT_CTL[NUMID]+L2C_VRT_CTL[MEMSZ]+16)
 * which ranges from a minimum of 64KB to a maximum of 256MB, depending on the size
 * of L2/DRAM that is protected and the number of virtual machines.
 *
 * The L2C_VRT_MEM DATA bit that L2C uses is:
 *
 *   l2c_vrt_mem_bit_index = address >> (L2C_VRT_CTL[MEMSZ]+L2C_VRT_CTL[NUMID]+16); // address is a byte address
 *   l2c_vrt_mem_bit_index = l2c_vrt_mem_bit_index | (virtID << (14-L2C_VRT_CTL[NUMID]));
 *
 *   L2C_VRT_MEM(l2c_vrt_mem_bit_index >> 5)[DATA<l2c_vrt_mem_bit_index & 0x1F>] is used
 *
 * A specific example:
 *
 *   L2C_VRT_CTL[NUMID]=2 (i.e. 8 virtual machine ID's used)
 *   L2C_VRT_CTL[MEMSZ]=4 (i.e. L2C_VRT_MEM covers 16 GB)
 *
 *   L2/DRAM region size (granularity) is 4MB
 *
 *   l2c_vrt_mem_bit_index<14:12> = virtID<2:0>
 *   l2c_vrt_mem_bit_index<11:0> = address<33:22>
 *
 *   For L2/DRAM physical address 0x51000000 with virtID=5:
 *      L2C_VRT_MEM648[DATA<4>] determines when the store is allowed (648 is decimal, not hex)
 */
union cvmx_l2c_vrt_memx {
	uint64_t u64;
	struct cvmx_l2c_vrt_memx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t parity                       : 4;  /**< Parity to write into (or read from) the
                                                         virtualization memory.
                                                         PARITY<i> is the even parity of DATA<(i*8)+7:i*8> */
	uint64_t data                         : 32; /**< Data to write into (or read from) the
                                                         virtualization memory. */
#else
	uint64_t data                         : 32;
	uint64_t parity                       : 4;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_l2c_vrt_memx_s            cn61xx;
	struct cvmx_l2c_vrt_memx_s            cn63xx;
	struct cvmx_l2c_vrt_memx_s            cn63xxp1;
	struct cvmx_l2c_vrt_memx_s            cn66xx;
	struct cvmx_l2c_vrt_memx_s            cn68xx;
	struct cvmx_l2c_vrt_memx_s            cn68xxp1;
	struct cvmx_l2c_vrt_memx_s            cnf71xx;
};
typedef union cvmx_l2c_vrt_memx cvmx_l2c_vrt_memx_t;

/**
 * cvmx_l2c_wpar_iob#
 *
 * L2C_WPAR_IOB = L2C IOB way partitioning
 *
 *
 * Notes:
 * (1) The read value of MASK will include bits set because of the L2C cripple fuses.
 *
 */
union cvmx_l2c_wpar_iobx {
	uint64_t u64;
	struct cvmx_l2c_wpar_iobx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t mask                         : 16; /**< Way partitioning mask (1 means do not use). The read value of [MASK] includes bits set
                                                         because of the L2C cripple fuses. */
#else
	uint64_t mask                         : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_l2c_wpar_iobx_s           cn61xx;
	struct cvmx_l2c_wpar_iobx_s           cn63xx;
	struct cvmx_l2c_wpar_iobx_s           cn63xxp1;
	struct cvmx_l2c_wpar_iobx_s           cn66xx;
	struct cvmx_l2c_wpar_iobx_s           cn68xx;
	struct cvmx_l2c_wpar_iobx_s           cn68xxp1;
	struct cvmx_l2c_wpar_iobx_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t mask                         : 4;  /**< Way partitioning mask (1 means do not use). The read value of [MASK] includes bits set
                                                         because of the L2C cripple fuses. */
#else
	uint64_t mask                         : 4;
	uint64_t reserved_4_63                : 60;
#endif
	} cn70xx;
	struct cvmx_l2c_wpar_iobx_cn70xx      cn70xxp1;
	struct cvmx_l2c_wpar_iobx_s           cn73xx;
	struct cvmx_l2c_wpar_iobx_s           cn78xx;
	struct cvmx_l2c_wpar_iobx_s           cn78xxp1;
	struct cvmx_l2c_wpar_iobx_s           cnf71xx;
	struct cvmx_l2c_wpar_iobx_s           cnf75xx;
};
typedef union cvmx_l2c_wpar_iobx cvmx_l2c_wpar_iobx_t;

/**
 * cvmx_l2c_wpar_pp#
 *
 * L2C_WPAR_PP = L2C PP way partitioning
 *
 *
 * Notes:
 * (1) The read value of MASK will include bits set because of the L2C cripple fuses.
 *
 */
union cvmx_l2c_wpar_ppx {
	uint64_t u64;
	struct cvmx_l2c_wpar_ppx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t mask                         : 16; /**< Way partitioning mask (1 means do not use). The read value of [MASK] includes bits set
                                                         because of the L2C cripple fuses. */
#else
	uint64_t mask                         : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_l2c_wpar_ppx_s            cn61xx;
	struct cvmx_l2c_wpar_ppx_s            cn63xx;
	struct cvmx_l2c_wpar_ppx_s            cn63xxp1;
	struct cvmx_l2c_wpar_ppx_s            cn66xx;
	struct cvmx_l2c_wpar_ppx_s            cn68xx;
	struct cvmx_l2c_wpar_ppx_s            cn68xxp1;
	struct cvmx_l2c_wpar_ppx_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t mask                         : 4;  /**< Way partitioning mask (1 means do not use). The read value of [MASK] includes bits set
                                                         because of the L2C cripple fuses. */
#else
	uint64_t mask                         : 4;
	uint64_t reserved_4_63                : 60;
#endif
	} cn70xx;
	struct cvmx_l2c_wpar_ppx_cn70xx       cn70xxp1;
	struct cvmx_l2c_wpar_ppx_s            cn73xx;
	struct cvmx_l2c_wpar_ppx_s            cn78xx;
	struct cvmx_l2c_wpar_ppx_s            cn78xxp1;
	struct cvmx_l2c_wpar_ppx_s            cnf71xx;
	struct cvmx_l2c_wpar_ppx_s            cnf75xx;
};
typedef union cvmx_l2c_wpar_ppx cvmx_l2c_wpar_ppx_t;

/**
 * cvmx_l2c_xmc#_pfc
 *
 * L2C_XMC_PFC = L2C XMC Performance Counter(s)
 *
 */
union cvmx_l2c_xmcx_pfc {
	uint64_t u64;
	struct cvmx_l2c_xmcx_pfc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t count                        : 64; /**< Current counter value */
#else
	uint64_t count                        : 64;
#endif
	} s;
	struct cvmx_l2c_xmcx_pfc_s            cn61xx;
	struct cvmx_l2c_xmcx_pfc_s            cn63xx;
	struct cvmx_l2c_xmcx_pfc_s            cn63xxp1;
	struct cvmx_l2c_xmcx_pfc_s            cn66xx;
	struct cvmx_l2c_xmcx_pfc_s            cn68xx;
	struct cvmx_l2c_xmcx_pfc_s            cn68xxp1;
	struct cvmx_l2c_xmcx_pfc_s            cn70xx;
	struct cvmx_l2c_xmcx_pfc_s            cn70xxp1;
	struct cvmx_l2c_xmcx_pfc_s            cn73xx;
	struct cvmx_l2c_xmcx_pfc_s            cn78xx;
	struct cvmx_l2c_xmcx_pfc_s            cn78xxp1;
	struct cvmx_l2c_xmcx_pfc_s            cnf71xx;
	struct cvmx_l2c_xmcx_pfc_s            cnf75xx;
};
typedef union cvmx_l2c_xmcx_pfc cvmx_l2c_xmcx_pfc_t;

/**
 * cvmx_l2c_xmc_cmd
 *
 * Note the following:
 * The ADD bus command chosen must not be a IOB-destined command or operation is UNDEFINED.
 *
 * The ADD bus command will have SID forced to IOB, DID forced to L2C, no virtualization checks
 * performed (always pass), and xmdmsk forced to 0. Note that this implies that commands that
 * REQUIRE a STORE cycle (STP, STC, SAA, FAA, FAS) should not be used or the results are
 * unpredictable. The sid = IOB means that the way partitioning used for the command is
 * L2C_WPAR_IOB(). Neither L2C_QOS_IOB() nor L2C_QOS_PP() are used for these
 * commands.
 *
 * Any FILL responses generated by the ADD bus command are ignored. Generated STINs, however,
 * will correctly invalidate the required cores.
 *
 * A write that arrives while [INUSE] is set will block until [INUSE] clears. This
 * gives software two options when needing to issue a stream of write operations to L2C_XMC_CMD:
 * polling on [INUSE], or allowing hardware to handle the interlock -- at the expense of
 * locking up the RSL bus for potentially tens of cycles at a time while waiting for an available
 * LFB/VAB entry. Note that when [INUSE] clears, the only ordering it implies is that
 * software can send another ADD bus command. Subsequent commands may complete out of order
 * relative to earlier commands.
 *
 * The address written to L2C_XMC_CMD is a physical address. L2C performs hole removal and index
 * aliasing (if enabled) on the written address and uses that for the command. This hole
 * removed/index aliased address is what is returned on a read of the L2C_XMC_CMD register.
 */
union cvmx_l2c_xmc_cmd {
	uint64_t u64;
	struct cvmx_l2c_xmc_cmd_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t inuse                        : 1;  /**< Set to 1 by hardware upon receiving a write; cleared when command has issued (not
                                                         necessarily completed, but ordered relative to other traffic) and hardware can accept
                                                         another command. */
	uint64_t reserved_47_62               : 16;
	uint64_t qos                          : 3;  /**< QOS level to use for simulated ADD bus request. */
	uint64_t node                         : 4;  /**< Ignored because CCPI is not present. Will return the value written, but the
                                                         simulated ADD bus request will use a node of 0. */
	uint64_t addr                         : 40; /**< Address to use for simulated ADD bus request. (The address written to L2C_XMC_CMD is a
                                                         physical address. L2C performs hole removal and index aliasing (if enabled) on the written
                                                         address and uses that for the command. This hole-removed/index-aliased address is what is
                                                         returned on a read of L2C_XMC_CMD.) */
#else
	uint64_t addr                         : 40;
	uint64_t node                         : 4;
	uint64_t qos                          : 3;
	uint64_t reserved_47_62               : 16;
	uint64_t inuse                        : 1;
#endif
	} s;
	struct cvmx_l2c_xmc_cmd_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t inuse                        : 1;  /**< Set to 1 by HW upon receiving a write, cleared when
                                                         command has issued (not necessarily completed, but
                                                         ordered relative to other traffic) and HW can accept
                                                         another command. */
	uint64_t cmd                          : 6;  /**< Command to use for simulated XMC request
                                                         a new request can be accepted */
	uint64_t reserved_38_56               : 19;
	uint64_t addr                         : 38; /**< Address to use for simulated XMC request (see Note 6) */
#else
	uint64_t addr                         : 38;
	uint64_t reserved_38_56               : 19;
	uint64_t cmd                          : 6;
	uint64_t inuse                        : 1;
#endif
	} cn61xx;
	struct cvmx_l2c_xmc_cmd_cn61xx        cn63xx;
	struct cvmx_l2c_xmc_cmd_cn61xx        cn63xxp1;
	struct cvmx_l2c_xmc_cmd_cn61xx        cn66xx;
	struct cvmx_l2c_xmc_cmd_cn61xx        cn68xx;
	struct cvmx_l2c_xmc_cmd_cn61xx        cn68xxp1;
	struct cvmx_l2c_xmc_cmd_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t inuse                        : 1;  /**< Set to 1 by hardware upon receiving a write; cleared when command has issued (not
                                                         necessarily completed, but ordered relative to other traffic) and hardware can accept
                                                         another command. */
	uint64_t cmd                          : 7;  /**< Command to use for simulated ADD bus request. A new request can be accepted. */
	uint64_t reserved_47_55               : 9;
	uint64_t qos                          : 3;  /**< QOS level to use for simulated ADD bus request. */
	uint64_t node                         : 4;  /**< Must be zero. */
	uint64_t addr                         : 40; /**< Address to use for simulated ADD bus request. (The address written to L2C_XMC_CMD is a
                                                         physical address. L2C performs hole removal and index aliasing (if enabled) on the written
                                                         address and uses that for the command. This hole-removed/index-aliased address is what is
                                                         returned on a read of L2C_XMC_CMD.) */
#else
	uint64_t addr                         : 40;
	uint64_t node                         : 4;
	uint64_t qos                          : 3;
	uint64_t reserved_47_55               : 9;
	uint64_t cmd                          : 7;
	uint64_t inuse                        : 1;
#endif
	} cn70xx;
	struct cvmx_l2c_xmc_cmd_cn70xx        cn70xxp1;
	struct cvmx_l2c_xmc_cmd_cn70xx        cn73xx;
	struct cvmx_l2c_xmc_cmd_cn70xx        cn78xx;
	struct cvmx_l2c_xmc_cmd_cn70xx        cn78xxp1;
	struct cvmx_l2c_xmc_cmd_cn61xx        cnf71xx;
	struct cvmx_l2c_xmc_cmd_cn70xx        cnf75xx;
};
typedef union cvmx_l2c_xmc_cmd cvmx_l2c_xmc_cmd_t;

/**
 * cvmx_l2c_xmd#_pfc
 *
 * L2C_XMD_PFC = L2C XMD Performance Counter(s)
 *
 */
union cvmx_l2c_xmdx_pfc {
	uint64_t u64;
	struct cvmx_l2c_xmdx_pfc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t count                        : 64; /**< Current counter value */
#else
	uint64_t count                        : 64;
#endif
	} s;
	struct cvmx_l2c_xmdx_pfc_s            cn61xx;
	struct cvmx_l2c_xmdx_pfc_s            cn63xx;
	struct cvmx_l2c_xmdx_pfc_s            cn63xxp1;
	struct cvmx_l2c_xmdx_pfc_s            cn66xx;
	struct cvmx_l2c_xmdx_pfc_s            cn68xx;
	struct cvmx_l2c_xmdx_pfc_s            cn68xxp1;
	struct cvmx_l2c_xmdx_pfc_s            cn70xx;
	struct cvmx_l2c_xmdx_pfc_s            cn70xxp1;
	struct cvmx_l2c_xmdx_pfc_s            cn73xx;
	struct cvmx_l2c_xmdx_pfc_s            cn78xx;
	struct cvmx_l2c_xmdx_pfc_s            cn78xxp1;
	struct cvmx_l2c_xmdx_pfc_s            cnf71xx;
	struct cvmx_l2c_xmdx_pfc_s            cnf75xx;
};
typedef union cvmx_l2c_xmdx_pfc cvmx_l2c_xmdx_pfc_t;

#endif
