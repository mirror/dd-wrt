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
 * cvmx-sli-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon sli.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_SLI_DEFS_H__
#define __CVMX_SLI_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_BIST_STATUS CVMX_SLI_BIST_STATUS_FUNC()
static inline uint64_t CVMX_SLI_BIST_STATUS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_BIST_STATUS not supported on this chip\n");
	return 0x0000000000000580ull;
}
#else
#define CVMX_SLI_BIST_STATUS (0x0000000000000580ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_CTL_PORTX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SLI_CTL_PORTX(%lu) is invalid on this chip\n", offset);
	return 0x0000000000000050ull + ((offset) & 3) * 16;
}
#else
#define CVMX_SLI_CTL_PORTX(offset) (0x0000000000000050ull + ((offset) & 3) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_CTL_STATUS CVMX_SLI_CTL_STATUS_FUNC()
static inline uint64_t CVMX_SLI_CTL_STATUS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_CTL_STATUS not supported on this chip\n");
	return 0x0000000000000570ull;
}
#else
#define CVMX_SLI_CTL_STATUS (0x0000000000000570ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_DATA_OUT_CNT CVMX_SLI_DATA_OUT_CNT_FUNC()
static inline uint64_t CVMX_SLI_DATA_OUT_CNT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_DATA_OUT_CNT not supported on this chip\n");
	return 0x00000000000005F0ull;
}
#else
#define CVMX_SLI_DATA_OUT_CNT (0x00000000000005F0ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_DBG_DATA CVMX_SLI_DBG_DATA_FUNC()
static inline uint64_t CVMX_SLI_DBG_DATA_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_DBG_DATA not supported on this chip\n");
	return 0x0000000000000310ull;
}
#else
#define CVMX_SLI_DBG_DATA (0x0000000000000310ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_DBG_SELECT CVMX_SLI_DBG_SELECT_FUNC()
static inline uint64_t CVMX_SLI_DBG_SELECT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_DBG_SELECT not supported on this chip\n");
	return 0x0000000000000300ull;
}
#else
#define CVMX_SLI_DBG_SELECT (0x0000000000000300ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_DMAX_CNT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SLI_DMAX_CNT(%lu) is invalid on this chip\n", offset);
	return 0x0000000000000400ull + ((offset) & 1) * 16;
}
#else
#define CVMX_SLI_DMAX_CNT(offset) (0x0000000000000400ull + ((offset) & 1) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_DMAX_INT_LEVEL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SLI_DMAX_INT_LEVEL(%lu) is invalid on this chip\n", offset);
	return 0x00000000000003E0ull + ((offset) & 1) * 16;
}
#else
#define CVMX_SLI_DMAX_INT_LEVEL(offset) (0x00000000000003E0ull + ((offset) & 1) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_DMAX_TIM(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SLI_DMAX_TIM(%lu) is invalid on this chip\n", offset);
	return 0x0000000000000420ull + ((offset) & 1) * 16;
}
#else
#define CVMX_SLI_DMAX_TIM(offset) (0x0000000000000420ull + ((offset) & 1) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_INT_ENB_CIU CVMX_SLI_INT_ENB_CIU_FUNC()
static inline uint64_t CVMX_SLI_INT_ENB_CIU_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_INT_ENB_CIU not supported on this chip\n");
	return 0x0000000000003CD0ull;
}
#else
#define CVMX_SLI_INT_ENB_CIU (0x0000000000003CD0ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_INT_ENB_PORTX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SLI_INT_ENB_PORTX(%lu) is invalid on this chip\n", offset);
	return 0x0000000000000340ull + ((offset) & 3) * 16;
}
#else
#define CVMX_SLI_INT_ENB_PORTX(offset) (0x0000000000000340ull + ((offset) & 3) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_INT_SUM CVMX_SLI_INT_SUM_FUNC()
static inline uint64_t CVMX_SLI_INT_SUM_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_INT_SUM not supported on this chip\n");
	return 0x0000000000000330ull;
}
#else
#define CVMX_SLI_INT_SUM (0x0000000000000330ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_LAST_WIN_RDATA0 CVMX_SLI_LAST_WIN_RDATA0_FUNC()
static inline uint64_t CVMX_SLI_LAST_WIN_RDATA0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_LAST_WIN_RDATA0 not supported on this chip\n");
	return 0x0000000000000600ull;
}
#else
#define CVMX_SLI_LAST_WIN_RDATA0 (0x0000000000000600ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_LAST_WIN_RDATA1 CVMX_SLI_LAST_WIN_RDATA1_FUNC()
static inline uint64_t CVMX_SLI_LAST_WIN_RDATA1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_LAST_WIN_RDATA1 not supported on this chip\n");
	return 0x0000000000000610ull;
}
#else
#define CVMX_SLI_LAST_WIN_RDATA1 (0x0000000000000610ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_LAST_WIN_RDATA2 CVMX_SLI_LAST_WIN_RDATA2_FUNC()
static inline uint64_t CVMX_SLI_LAST_WIN_RDATA2_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_LAST_WIN_RDATA2 not supported on this chip\n");
	return 0x00000000000006C0ull;
}
#else
#define CVMX_SLI_LAST_WIN_RDATA2 (0x00000000000006C0ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_LAST_WIN_RDATA3 CVMX_SLI_LAST_WIN_RDATA3_FUNC()
static inline uint64_t CVMX_SLI_LAST_WIN_RDATA3_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_LAST_WIN_RDATA3 not supported on this chip\n");
	return 0x00000000000006D0ull;
}
#else
#define CVMX_SLI_LAST_WIN_RDATA3 (0x00000000000006D0ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MAC_CREDIT_CNT CVMX_SLI_MAC_CREDIT_CNT_FUNC()
static inline uint64_t CVMX_SLI_MAC_CREDIT_CNT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_MAC_CREDIT_CNT not supported on this chip\n");
	return 0x0000000000003D70ull;
}
#else
#define CVMX_SLI_MAC_CREDIT_CNT (0x0000000000003D70ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MAC_CREDIT_CNT2 CVMX_SLI_MAC_CREDIT_CNT2_FUNC()
static inline uint64_t CVMX_SLI_MAC_CREDIT_CNT2_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_MAC_CREDIT_CNT2 not supported on this chip\n");
	return 0x0000000000003E10ull;
}
#else
#define CVMX_SLI_MAC_CREDIT_CNT2 (0x0000000000003E10ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MAC_NUMBER CVMX_SLI_MAC_NUMBER_FUNC()
static inline uint64_t CVMX_SLI_MAC_NUMBER_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_MAC_NUMBER not supported on this chip\n");
	return 0x0000000000003E00ull;
}
#else
#define CVMX_SLI_MAC_NUMBER (0x0000000000003E00ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MEM_ACCESS_CTL CVMX_SLI_MEM_ACCESS_CTL_FUNC()
static inline uint64_t CVMX_SLI_MEM_ACCESS_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_MEM_ACCESS_CTL not supported on this chip\n");
	return 0x00000000000002F0ull;
}
#else
#define CVMX_SLI_MEM_ACCESS_CTL (0x00000000000002F0ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_MEM_ACCESS_SUBIDX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && (((offset >= 12) && (offset <= 27)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && (((offset >= 12) && (offset <= 27)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && (((offset >= 12) && (offset <= 27)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && (((offset >= 12) && (offset <= 27)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && (((offset >= 12) && (offset <= 27)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset >= 12) && (offset <= 27)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && (((offset >= 12) && (offset <= 27))))))
		cvmx_warn("CVMX_SLI_MEM_ACCESS_SUBIDX(%lu) is invalid on this chip\n", offset);
	return 0x00000000000000E0ull + ((offset) & 31) * 16 - 16*12;
}
#else
#define CVMX_SLI_MEM_ACCESS_SUBIDX(offset) (0x00000000000000E0ull + ((offset) & 31) * 16 - 16*12)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MEM_CTL CVMX_SLI_MEM_CTL_FUNC()
static inline uint64_t CVMX_SLI_MEM_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_SLI_MEM_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011F00000105E0ull);
}
#else
#define CVMX_SLI_MEM_CTL (CVMX_ADD_IO_SEG(0x00011F00000105E0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MEM_INT_SUM CVMX_SLI_MEM_INT_SUM_FUNC()
static inline uint64_t CVMX_SLI_MEM_INT_SUM_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_SLI_MEM_INT_SUM not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011F00000105D0ull);
}
#else
#define CVMX_SLI_MEM_INT_SUM (CVMX_ADD_IO_SEG(0x00011F00000105D0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_MSIXX_TABLE_ADDR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 64)))))
		cvmx_warn("CVMX_SLI_MSIXX_TABLE_ADDR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011F0000016000ull) + ((offset) & 127) * 16;
}
#else
#define CVMX_SLI_MSIXX_TABLE_ADDR(offset) (CVMX_ADD_IO_SEG(0x00011F0000016000ull) + ((offset) & 127) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_MSIXX_TABLE_DATA(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 64)))))
		cvmx_warn("CVMX_SLI_MSIXX_TABLE_DATA(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011F0000016800ull) + ((offset) & 127) * 16;
}
#else
#define CVMX_SLI_MSIXX_TABLE_DATA(offset) (CVMX_ADD_IO_SEG(0x00011F0000016800ull) + ((offset) & 127) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSIX_PBA0 CVMX_SLI_MSIX_PBA0_FUNC()
static inline uint64_t CVMX_SLI_MSIX_PBA0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_SLI_MSIX_PBA0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011F0000017000ull);
}
#else
#define CVMX_SLI_MSIX_PBA0 (CVMX_ADD_IO_SEG(0x00011F0000017000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSIX_PBA1 CVMX_SLI_MSIX_PBA1_FUNC()
static inline uint64_t CVMX_SLI_MSIX_PBA1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_SLI_MSIX_PBA1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011F0000017010ull);
}
#else
#define CVMX_SLI_MSIX_PBA1 (CVMX_ADD_IO_SEG(0x00011F0000017010ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_ENB0 CVMX_SLI_MSI_ENB0_FUNC()
static inline uint64_t CVMX_SLI_MSI_ENB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_MSI_ENB0 not supported on this chip\n");
	return 0x0000000000003C50ull;
}
#else
#define CVMX_SLI_MSI_ENB0 (0x0000000000003C50ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_ENB1 CVMX_SLI_MSI_ENB1_FUNC()
static inline uint64_t CVMX_SLI_MSI_ENB1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_MSI_ENB1 not supported on this chip\n");
	return 0x0000000000003C60ull;
}
#else
#define CVMX_SLI_MSI_ENB1 (0x0000000000003C60ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_ENB2 CVMX_SLI_MSI_ENB2_FUNC()
static inline uint64_t CVMX_SLI_MSI_ENB2_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_MSI_ENB2 not supported on this chip\n");
	return 0x0000000000003C70ull;
}
#else
#define CVMX_SLI_MSI_ENB2 (0x0000000000003C70ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_ENB3 CVMX_SLI_MSI_ENB3_FUNC()
static inline uint64_t CVMX_SLI_MSI_ENB3_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_MSI_ENB3 not supported on this chip\n");
	return 0x0000000000003C80ull;
}
#else
#define CVMX_SLI_MSI_ENB3 (0x0000000000003C80ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_RCV0 CVMX_SLI_MSI_RCV0_FUNC()
static inline uint64_t CVMX_SLI_MSI_RCV0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_MSI_RCV0 not supported on this chip\n");
	return 0x0000000000003C10ull;
}
#else
#define CVMX_SLI_MSI_RCV0 (0x0000000000003C10ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_RCV1 CVMX_SLI_MSI_RCV1_FUNC()
static inline uint64_t CVMX_SLI_MSI_RCV1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_MSI_RCV1 not supported on this chip\n");
	return 0x0000000000003C20ull;
}
#else
#define CVMX_SLI_MSI_RCV1 (0x0000000000003C20ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_RCV2 CVMX_SLI_MSI_RCV2_FUNC()
static inline uint64_t CVMX_SLI_MSI_RCV2_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_MSI_RCV2 not supported on this chip\n");
	return 0x0000000000003C30ull;
}
#else
#define CVMX_SLI_MSI_RCV2 (0x0000000000003C30ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_RCV3 CVMX_SLI_MSI_RCV3_FUNC()
static inline uint64_t CVMX_SLI_MSI_RCV3_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_MSI_RCV3 not supported on this chip\n");
	return 0x0000000000003C40ull;
}
#else
#define CVMX_SLI_MSI_RCV3 (0x0000000000003C40ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_RD_MAP CVMX_SLI_MSI_RD_MAP_FUNC()
static inline uint64_t CVMX_SLI_MSI_RD_MAP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_MSI_RD_MAP not supported on this chip\n");
	return 0x0000000000003CA0ull;
}
#else
#define CVMX_SLI_MSI_RD_MAP (0x0000000000003CA0ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_W1C_ENB0 CVMX_SLI_MSI_W1C_ENB0_FUNC()
static inline uint64_t CVMX_SLI_MSI_W1C_ENB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_MSI_W1C_ENB0 not supported on this chip\n");
	return 0x0000000000003CF0ull;
}
#else
#define CVMX_SLI_MSI_W1C_ENB0 (0x0000000000003CF0ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_W1C_ENB1 CVMX_SLI_MSI_W1C_ENB1_FUNC()
static inline uint64_t CVMX_SLI_MSI_W1C_ENB1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_MSI_W1C_ENB1 not supported on this chip\n");
	return 0x0000000000003D00ull;
}
#else
#define CVMX_SLI_MSI_W1C_ENB1 (0x0000000000003D00ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_W1C_ENB2 CVMX_SLI_MSI_W1C_ENB2_FUNC()
static inline uint64_t CVMX_SLI_MSI_W1C_ENB2_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_MSI_W1C_ENB2 not supported on this chip\n");
	return 0x0000000000003D10ull;
}
#else
#define CVMX_SLI_MSI_W1C_ENB2 (0x0000000000003D10ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_W1C_ENB3 CVMX_SLI_MSI_W1C_ENB3_FUNC()
static inline uint64_t CVMX_SLI_MSI_W1C_ENB3_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_MSI_W1C_ENB3 not supported on this chip\n");
	return 0x0000000000003D20ull;
}
#else
#define CVMX_SLI_MSI_W1C_ENB3 (0x0000000000003D20ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_W1S_ENB0 CVMX_SLI_MSI_W1S_ENB0_FUNC()
static inline uint64_t CVMX_SLI_MSI_W1S_ENB0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_MSI_W1S_ENB0 not supported on this chip\n");
	return 0x0000000000003D30ull;
}
#else
#define CVMX_SLI_MSI_W1S_ENB0 (0x0000000000003D30ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_W1S_ENB1 CVMX_SLI_MSI_W1S_ENB1_FUNC()
static inline uint64_t CVMX_SLI_MSI_W1S_ENB1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_MSI_W1S_ENB1 not supported on this chip\n");
	return 0x0000000000003D40ull;
}
#else
#define CVMX_SLI_MSI_W1S_ENB1 (0x0000000000003D40ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_W1S_ENB2 CVMX_SLI_MSI_W1S_ENB2_FUNC()
static inline uint64_t CVMX_SLI_MSI_W1S_ENB2_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_MSI_W1S_ENB2 not supported on this chip\n");
	return 0x0000000000003D50ull;
}
#else
#define CVMX_SLI_MSI_W1S_ENB2 (0x0000000000003D50ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_W1S_ENB3 CVMX_SLI_MSI_W1S_ENB3_FUNC()
static inline uint64_t CVMX_SLI_MSI_W1S_ENB3_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_MSI_W1S_ENB3 not supported on this chip\n");
	return 0x0000000000003D60ull;
}
#else
#define CVMX_SLI_MSI_W1S_ENB3 (0x0000000000003D60ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_MSI_WR_MAP CVMX_SLI_MSI_WR_MAP_FUNC()
static inline uint64_t CVMX_SLI_MSI_WR_MAP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_MSI_WR_MAP not supported on this chip\n");
	return 0x0000000000003C90ull;
}
#else
#define CVMX_SLI_MSI_WR_MAP (0x0000000000003C90ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PCIE_MSI_RCV CVMX_SLI_PCIE_MSI_RCV_FUNC()
static inline uint64_t CVMX_SLI_PCIE_MSI_RCV_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PCIE_MSI_RCV not supported on this chip\n");
	return 0x0000000000003CB0ull;
}
#else
#define CVMX_SLI_PCIE_MSI_RCV (0x0000000000003CB0ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PCIE_MSI_RCV_B1 CVMX_SLI_PCIE_MSI_RCV_B1_FUNC()
static inline uint64_t CVMX_SLI_PCIE_MSI_RCV_B1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PCIE_MSI_RCV_B1 not supported on this chip\n");
	return 0x0000000000000650ull;
}
#else
#define CVMX_SLI_PCIE_MSI_RCV_B1 (0x0000000000000650ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PCIE_MSI_RCV_B2 CVMX_SLI_PCIE_MSI_RCV_B2_FUNC()
static inline uint64_t CVMX_SLI_PCIE_MSI_RCV_B2_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PCIE_MSI_RCV_B2 not supported on this chip\n");
	return 0x0000000000000660ull;
}
#else
#define CVMX_SLI_PCIE_MSI_RCV_B2 (0x0000000000000660ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PCIE_MSI_RCV_B3 CVMX_SLI_PCIE_MSI_RCV_B3_FUNC()
static inline uint64_t CVMX_SLI_PCIE_MSI_RCV_B3_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PCIE_MSI_RCV_B3 not supported on this chip\n");
	return 0x0000000000000670ull;
}
#else
#define CVMX_SLI_PCIE_MSI_RCV_B3 (0x0000000000000670ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKTX_CNTS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_SLI_PKTX_CNTS(%lu) is invalid on this chip\n", offset);
	return 0x0000000000002400ull + ((offset) & 63) * 16;
}
#else
#define CVMX_SLI_PKTX_CNTS(offset) (0x0000000000002400ull + ((offset) & 63) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKTX_INPUT_CONTROL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_SLI_PKTX_INPUT_CONTROL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011F0000014000ull) + ((offset) & 63) * 16;
}
#else
#define CVMX_SLI_PKTX_INPUT_CONTROL(offset) (CVMX_ADD_IO_SEG(0x00011F0000014000ull) + ((offset) & 63) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKTX_INSTR_BADDR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_SLI_PKTX_INSTR_BADDR(%lu) is invalid on this chip\n", offset);
	return 0x0000000000002800ull + ((offset) & 63) * 16;
}
#else
#define CVMX_SLI_PKTX_INSTR_BADDR(offset) (0x0000000000002800ull + ((offset) & 63) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKTX_INSTR_BAOFF_DBELL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_SLI_PKTX_INSTR_BAOFF_DBELL(%lu) is invalid on this chip\n", offset);
	return 0x0000000000002C00ull + ((offset) & 63) * 16;
}
#else
#define CVMX_SLI_PKTX_INSTR_BAOFF_DBELL(offset) (0x0000000000002C00ull + ((offset) & 63) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKTX_INSTR_FIFO_RSIZE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_SLI_PKTX_INSTR_FIFO_RSIZE(%lu) is invalid on this chip\n", offset);
	return 0x0000000000003000ull + ((offset) & 63) * 16;
}
#else
#define CVMX_SLI_PKTX_INSTR_FIFO_RSIZE(offset) (0x0000000000003000ull + ((offset) & 63) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKTX_INSTR_HEADER(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_SLI_PKTX_INSTR_HEADER(%lu) is invalid on this chip\n", offset);
	return 0x0000000000003400ull + ((offset) & 31) * 16;
}
#else
#define CVMX_SLI_PKTX_INSTR_HEADER(offset) (0x0000000000003400ull + ((offset) & 31) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKTX_INT_LEVELS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_SLI_PKTX_INT_LEVELS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011F0000014400ull) + ((offset) & 63) * 16;
}
#else
#define CVMX_SLI_PKTX_INT_LEVELS(offset) (CVMX_ADD_IO_SEG(0x00011F0000014400ull) + ((offset) & 63) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKTX_IN_BP(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_SLI_PKTX_IN_BP(%lu) is invalid on this chip\n", offset);
	return 0x0000000000003800ull + ((offset) & 31) * 16;
}
#else
#define CVMX_SLI_PKTX_IN_BP(offset) (0x0000000000003800ull + ((offset) & 31) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKTX_OUTPUT_CONTROL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_SLI_PKTX_OUTPUT_CONTROL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011F0000014800ull) + ((offset) & 63) * 16;
}
#else
#define CVMX_SLI_PKTX_OUTPUT_CONTROL(offset) (CVMX_ADD_IO_SEG(0x00011F0000014800ull) + ((offset) & 63) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKTX_OUT_SIZE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_SLI_PKTX_OUT_SIZE(%lu) is invalid on this chip\n", offset);
	return 0x0000000000000C00ull + ((offset) & 63) * 16;
}
#else
#define CVMX_SLI_PKTX_OUT_SIZE(offset) (0x0000000000000C00ull + ((offset) & 63) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKTX_SLIST_BADDR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_SLI_PKTX_SLIST_BADDR(%lu) is invalid on this chip\n", offset);
	return 0x0000000000001400ull + ((offset) & 63) * 16;
}
#else
#define CVMX_SLI_PKTX_SLIST_BADDR(offset) (0x0000000000001400ull + ((offset) & 63) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKTX_SLIST_BAOFF_DBELL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_SLI_PKTX_SLIST_BAOFF_DBELL(%lu) is invalid on this chip\n", offset);
	return 0x0000000000001800ull + ((offset) & 63) * 16;
}
#else
#define CVMX_SLI_PKTX_SLIST_BAOFF_DBELL(offset) (0x0000000000001800ull + ((offset) & 63) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKTX_SLIST_FIFO_RSIZE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_SLI_PKTX_SLIST_FIFO_RSIZE(%lu) is invalid on this chip\n", offset);
	return 0x0000000000001C00ull + ((offset) & 63) * 16;
}
#else
#define CVMX_SLI_PKTX_SLIST_FIFO_RSIZE(offset) (0x0000000000001C00ull + ((offset) & 63) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKTX_VF_SIG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_SLI_PKTX_VF_SIG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011F0000014C00ull) + ((offset) & 63) * 16;
}
#else
#define CVMX_SLI_PKTX_VF_SIG(offset) (CVMX_ADD_IO_SEG(0x00011F0000014C00ull) + ((offset) & 63) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_CNT_INT CVMX_SLI_PKT_CNT_INT_FUNC()
static inline uint64_t CVMX_SLI_PKT_CNT_INT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_CNT_INT not supported on this chip\n");
	return 0x0000000000001130ull;
}
#else
#define CVMX_SLI_PKT_CNT_INT (0x0000000000001130ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_CNT_INT_ENB CVMX_SLI_PKT_CNT_INT_ENB_FUNC()
static inline uint64_t CVMX_SLI_PKT_CNT_INT_ENB_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_CNT_INT_ENB not supported on this chip\n");
	return 0x0000000000001150ull;
}
#else
#define CVMX_SLI_PKT_CNT_INT_ENB (0x0000000000001150ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_CTL CVMX_SLI_PKT_CTL_FUNC()
static inline uint64_t CVMX_SLI_PKT_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_CTL not supported on this chip\n");
	return 0x0000000000001220ull;
}
#else
#define CVMX_SLI_PKT_CTL (0x0000000000001220ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_DATA_OUT_ES CVMX_SLI_PKT_DATA_OUT_ES_FUNC()
static inline uint64_t CVMX_SLI_PKT_DATA_OUT_ES_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_DATA_OUT_ES not supported on this chip\n");
	return 0x00000000000010B0ull;
}
#else
#define CVMX_SLI_PKT_DATA_OUT_ES (0x00000000000010B0ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_DATA_OUT_NS CVMX_SLI_PKT_DATA_OUT_NS_FUNC()
static inline uint64_t CVMX_SLI_PKT_DATA_OUT_NS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_DATA_OUT_NS not supported on this chip\n");
	return 0x00000000000010A0ull;
}
#else
#define CVMX_SLI_PKT_DATA_OUT_NS (0x00000000000010A0ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_DATA_OUT_ROR CVMX_SLI_PKT_DATA_OUT_ROR_FUNC()
static inline uint64_t CVMX_SLI_PKT_DATA_OUT_ROR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_DATA_OUT_ROR not supported on this chip\n");
	return 0x0000000000001090ull;
}
#else
#define CVMX_SLI_PKT_DATA_OUT_ROR (0x0000000000001090ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_DPADDR CVMX_SLI_PKT_DPADDR_FUNC()
static inline uint64_t CVMX_SLI_PKT_DPADDR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_DPADDR not supported on this chip\n");
	return 0x0000000000001080ull;
}
#else
#define CVMX_SLI_PKT_DPADDR (0x0000000000001080ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_INPUT_CONTROL CVMX_SLI_PKT_INPUT_CONTROL_FUNC()
static inline uint64_t CVMX_SLI_PKT_INPUT_CONTROL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_INPUT_CONTROL not supported on this chip\n");
	return 0x0000000000001170ull;
}
#else
#define CVMX_SLI_PKT_INPUT_CONTROL (0x0000000000001170ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_INSTR_ENB CVMX_SLI_PKT_INSTR_ENB_FUNC()
static inline uint64_t CVMX_SLI_PKT_INSTR_ENB_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_INSTR_ENB not supported on this chip\n");
	return 0x0000000000001000ull;
}
#else
#define CVMX_SLI_PKT_INSTR_ENB (0x0000000000001000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_INSTR_RD_SIZE CVMX_SLI_PKT_INSTR_RD_SIZE_FUNC()
static inline uint64_t CVMX_SLI_PKT_INSTR_RD_SIZE_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_INSTR_RD_SIZE not supported on this chip\n");
	return 0x00000000000011A0ull;
}
#else
#define CVMX_SLI_PKT_INSTR_RD_SIZE (0x00000000000011A0ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_INSTR_SIZE CVMX_SLI_PKT_INSTR_SIZE_FUNC()
static inline uint64_t CVMX_SLI_PKT_INSTR_SIZE_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_INSTR_SIZE not supported on this chip\n");
	return 0x0000000000001020ull;
}
#else
#define CVMX_SLI_PKT_INSTR_SIZE (0x0000000000001020ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_INT CVMX_SLI_PKT_INT_FUNC()
static inline uint64_t CVMX_SLI_PKT_INT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_SLI_PKT_INT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011F0000011160ull);
}
#else
#define CVMX_SLI_PKT_INT (CVMX_ADD_IO_SEG(0x00011F0000011160ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_INT_LEVELS CVMX_SLI_PKT_INT_LEVELS_FUNC()
static inline uint64_t CVMX_SLI_PKT_INT_LEVELS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_INT_LEVELS not supported on this chip\n");
	return 0x0000000000001120ull;
}
#else
#define CVMX_SLI_PKT_INT_LEVELS (0x0000000000001120ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_IN_BP CVMX_SLI_PKT_IN_BP_FUNC()
static inline uint64_t CVMX_SLI_PKT_IN_BP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_IN_BP not supported on this chip\n");
	return 0x0000000000001210ull;
}
#else
#define CVMX_SLI_PKT_IN_BP (0x0000000000001210ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKT_IN_DONEX_CNTS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 31))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_SLI_PKT_IN_DONEX_CNTS(%lu) is invalid on this chip\n", offset);
	return 0x0000000000002000ull + ((offset) & 63) * 16;
}
#else
#define CVMX_SLI_PKT_IN_DONEX_CNTS(offset) (0x0000000000002000ull + ((offset) & 63) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_IN_INSTR_COUNTS CVMX_SLI_PKT_IN_INSTR_COUNTS_FUNC()
static inline uint64_t CVMX_SLI_PKT_IN_INSTR_COUNTS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_IN_INSTR_COUNTS not supported on this chip\n");
	return 0x0000000000001200ull;
}
#else
#define CVMX_SLI_PKT_IN_INSTR_COUNTS (0x0000000000001200ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_IN_INT CVMX_SLI_PKT_IN_INT_FUNC()
static inline uint64_t CVMX_SLI_PKT_IN_INT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_SLI_PKT_IN_INT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011F0000011150ull);
}
#else
#define CVMX_SLI_PKT_IN_INT (CVMX_ADD_IO_SEG(0x00011F0000011150ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_IN_PCIE_PORT CVMX_SLI_PKT_IN_PCIE_PORT_FUNC()
static inline uint64_t CVMX_SLI_PKT_IN_PCIE_PORT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_IN_PCIE_PORT not supported on this chip\n");
	return 0x00000000000011B0ull;
}
#else
#define CVMX_SLI_PKT_IN_PCIE_PORT (0x00000000000011B0ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_IPTR CVMX_SLI_PKT_IPTR_FUNC()
static inline uint64_t CVMX_SLI_PKT_IPTR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_IPTR not supported on this chip\n");
	return 0x0000000000001070ull;
}
#else
#define CVMX_SLI_PKT_IPTR (0x0000000000001070ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_MAC0_SIG0 CVMX_SLI_PKT_MAC0_SIG0_FUNC()
static inline uint64_t CVMX_SLI_PKT_MAC0_SIG0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_SLI_PKT_MAC0_SIG0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011F0000011300ull);
}
#else
#define CVMX_SLI_PKT_MAC0_SIG0 (CVMX_ADD_IO_SEG(0x00011F0000011300ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_MAC0_SIG1 CVMX_SLI_PKT_MAC0_SIG1_FUNC()
static inline uint64_t CVMX_SLI_PKT_MAC0_SIG1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_SLI_PKT_MAC0_SIG1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011F0000011310ull);
}
#else
#define CVMX_SLI_PKT_MAC0_SIG1 (CVMX_ADD_IO_SEG(0x00011F0000011310ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_MAC1_SIG0 CVMX_SLI_PKT_MAC1_SIG0_FUNC()
static inline uint64_t CVMX_SLI_PKT_MAC1_SIG0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_SLI_PKT_MAC1_SIG0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011F0000011320ull);
}
#else
#define CVMX_SLI_PKT_MAC1_SIG0 (CVMX_ADD_IO_SEG(0x00011F0000011320ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_MAC1_SIG1 CVMX_SLI_PKT_MAC1_SIG1_FUNC()
static inline uint64_t CVMX_SLI_PKT_MAC1_SIG1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_SLI_PKT_MAC1_SIG1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011F0000011330ull);
}
#else
#define CVMX_SLI_PKT_MAC1_SIG1 (CVMX_ADD_IO_SEG(0x00011F0000011330ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PKT_MACX_RINFO(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3)))))
		cvmx_warn("CVMX_SLI_PKT_MACX_RINFO(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011F0000011030ull) + ((offset) & 3) * 16;
}
#else
#define CVMX_SLI_PKT_MACX_RINFO(offset) (CVMX_ADD_IO_SEG(0x00011F0000011030ull) + ((offset) & 3) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_MEM_CTL CVMX_SLI_PKT_MEM_CTL_FUNC()
static inline uint64_t CVMX_SLI_PKT_MEM_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_SLI_PKT_MEM_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011F0000011120ull);
}
#else
#define CVMX_SLI_PKT_MEM_CTL (CVMX_ADD_IO_SEG(0x00011F0000011120ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_OUTPUT_WMARK CVMX_SLI_PKT_OUTPUT_WMARK_FUNC()
static inline uint64_t CVMX_SLI_PKT_OUTPUT_WMARK_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_OUTPUT_WMARK not supported on this chip\n");
	return 0x0000000000001180ull;
}
#else
#define CVMX_SLI_PKT_OUTPUT_WMARK (0x0000000000001180ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_OUT_BMODE CVMX_SLI_PKT_OUT_BMODE_FUNC()
static inline uint64_t CVMX_SLI_PKT_OUT_BMODE_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_OUT_BMODE not supported on this chip\n");
	return 0x00000000000010D0ull;
}
#else
#define CVMX_SLI_PKT_OUT_BMODE (0x00000000000010D0ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_OUT_BP_EN CVMX_SLI_PKT_OUT_BP_EN_FUNC()
static inline uint64_t CVMX_SLI_PKT_OUT_BP_EN_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_SLI_PKT_OUT_BP_EN not supported on this chip\n");
	return 0x0000000000001240ull;
}
#else
#define CVMX_SLI_PKT_OUT_BP_EN (0x0000000000001240ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_OUT_ENB CVMX_SLI_PKT_OUT_ENB_FUNC()
static inline uint64_t CVMX_SLI_PKT_OUT_ENB_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_OUT_ENB not supported on this chip\n");
	return 0x0000000000001010ull;
}
#else
#define CVMX_SLI_PKT_OUT_ENB (0x0000000000001010ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_PCIE_PORT CVMX_SLI_PKT_PCIE_PORT_FUNC()
static inline uint64_t CVMX_SLI_PKT_PCIE_PORT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_PCIE_PORT not supported on this chip\n");
	return 0x00000000000010E0ull;
}
#else
#define CVMX_SLI_PKT_PCIE_PORT (0x00000000000010E0ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_PORT_IN_RST CVMX_SLI_PKT_PORT_IN_RST_FUNC()
static inline uint64_t CVMX_SLI_PKT_PORT_IN_RST_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_PORT_IN_RST not supported on this chip\n");
	return 0x00000000000011F0ull;
}
#else
#define CVMX_SLI_PKT_PORT_IN_RST (0x00000000000011F0ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_RING_RST CVMX_SLI_PKT_RING_RST_FUNC()
static inline uint64_t CVMX_SLI_PKT_RING_RST_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_SLI_PKT_RING_RST not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011F00000111E0ull);
}
#else
#define CVMX_SLI_PKT_RING_RST (CVMX_ADD_IO_SEG(0x00011F00000111E0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_SLIST_ES CVMX_SLI_PKT_SLIST_ES_FUNC()
static inline uint64_t CVMX_SLI_PKT_SLIST_ES_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_SLIST_ES not supported on this chip\n");
	return 0x0000000000001050ull;
}
#else
#define CVMX_SLI_PKT_SLIST_ES (0x0000000000001050ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_SLIST_NS CVMX_SLI_PKT_SLIST_NS_FUNC()
static inline uint64_t CVMX_SLI_PKT_SLIST_NS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_SLIST_NS not supported on this chip\n");
	return 0x0000000000001040ull;
}
#else
#define CVMX_SLI_PKT_SLIST_NS (0x0000000000001040ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_SLIST_ROR CVMX_SLI_PKT_SLIST_ROR_FUNC()
static inline uint64_t CVMX_SLI_PKT_SLIST_ROR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_SLIST_ROR not supported on this chip\n");
	return 0x0000000000001030ull;
}
#else
#define CVMX_SLI_PKT_SLIST_ROR (0x0000000000001030ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_TIME_INT CVMX_SLI_PKT_TIME_INT_FUNC()
static inline uint64_t CVMX_SLI_PKT_TIME_INT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_TIME_INT not supported on this chip\n");
	return 0x0000000000001140ull;
}
#else
#define CVMX_SLI_PKT_TIME_INT (0x0000000000001140ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_PKT_TIME_INT_ENB CVMX_SLI_PKT_TIME_INT_ENB_FUNC()
static inline uint64_t CVMX_SLI_PKT_TIME_INT_ENB_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_PKT_TIME_INT_ENB not supported on this chip\n");
	return 0x0000000000001160ull;
}
#else
#define CVMX_SLI_PKT_TIME_INT_ENB (0x0000000000001160ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_PORTX_PKIND(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_SLI_PORTX_PKIND(%lu) is invalid on this chip\n", offset);
	return 0x0000000000000800ull + ((offset) & 31) * 16;
}
#else
#define CVMX_SLI_PORTX_PKIND(offset) (0x0000000000000800ull + ((offset) & 31) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SLI_S2M_PORTX_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN70XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 3))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_SLI_S2M_PORTX_CTL(%lu) is invalid on this chip\n", offset);
	return 0x0000000000003D80ull + ((offset) & 3) * 16;
}
#else
#define CVMX_SLI_S2M_PORTX_CTL(offset) (0x0000000000003D80ull + ((offset) & 3) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_SCRATCH_1 CVMX_SLI_SCRATCH_1_FUNC()
static inline uint64_t CVMX_SLI_SCRATCH_1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_SCRATCH_1 not supported on this chip\n");
	return 0x00000000000003C0ull;
}
#else
#define CVMX_SLI_SCRATCH_1 (0x00000000000003C0ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_SCRATCH_2 CVMX_SLI_SCRATCH_2_FUNC()
static inline uint64_t CVMX_SLI_SCRATCH_2_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_SCRATCH_2 not supported on this chip\n");
	return 0x00000000000003D0ull;
}
#else
#define CVMX_SLI_SCRATCH_2 (0x00000000000003D0ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_STATE1 CVMX_SLI_STATE1_FUNC()
static inline uint64_t CVMX_SLI_STATE1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_STATE1 not supported on this chip\n");
	return 0x0000000000000620ull;
}
#else
#define CVMX_SLI_STATE1 (0x0000000000000620ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_STATE2 CVMX_SLI_STATE2_FUNC()
static inline uint64_t CVMX_SLI_STATE2_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_STATE2 not supported on this chip\n");
	return 0x0000000000000630ull;
}
#else
#define CVMX_SLI_STATE2 (0x0000000000000630ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_STATE3 CVMX_SLI_STATE3_FUNC()
static inline uint64_t CVMX_SLI_STATE3_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_STATE3 not supported on this chip\n");
	return 0x0000000000000640ull;
}
#else
#define CVMX_SLI_STATE3 (0x0000000000000640ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_TX_PIPE CVMX_SLI_TX_PIPE_FUNC()
static inline uint64_t CVMX_SLI_TX_PIPE_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_SLI_TX_PIPE not supported on this chip\n");
	return 0x0000000000001230ull;
}
#else
#define CVMX_SLI_TX_PIPE (0x0000000000001230ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_WINDOW_CTL CVMX_SLI_WINDOW_CTL_FUNC()
static inline uint64_t CVMX_SLI_WINDOW_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_WINDOW_CTL not supported on this chip\n");
	return 0x00000000000002E0ull;
}
#else
#define CVMX_SLI_WINDOW_CTL (0x00000000000002E0ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_WIN_RD_ADDR CVMX_SLI_WIN_RD_ADDR_FUNC()
static inline uint64_t CVMX_SLI_WIN_RD_ADDR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_WIN_RD_ADDR not supported on this chip\n");
	return 0x0000000000000010ull;
}
#else
#define CVMX_SLI_WIN_RD_ADDR (0x0000000000000010ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_WIN_RD_DATA CVMX_SLI_WIN_RD_DATA_FUNC()
static inline uint64_t CVMX_SLI_WIN_RD_DATA_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_WIN_RD_DATA not supported on this chip\n");
	return 0x0000000000000040ull;
}
#else
#define CVMX_SLI_WIN_RD_DATA (0x0000000000000040ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_WIN_WR_ADDR CVMX_SLI_WIN_WR_ADDR_FUNC()
static inline uint64_t CVMX_SLI_WIN_WR_ADDR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_WIN_WR_ADDR not supported on this chip\n");
	return 0x0000000000000000ull;
}
#else
#define CVMX_SLI_WIN_WR_ADDR (0x0000000000000000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_WIN_WR_DATA CVMX_SLI_WIN_WR_DATA_FUNC()
static inline uint64_t CVMX_SLI_WIN_WR_DATA_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_WIN_WR_DATA not supported on this chip\n");
	return 0x0000000000000020ull;
}
#else
#define CVMX_SLI_WIN_WR_DATA (0x0000000000000020ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SLI_WIN_WR_MASK CVMX_SLI_WIN_WR_MASK_FUNC()
static inline uint64_t CVMX_SLI_WIN_WR_MASK_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN61XX) || OCTEON_IS_MODEL(OCTEON_CN63XX) || OCTEON_IS_MODEL(OCTEON_CN66XX) || OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN70XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CNF71XX)))
		cvmx_warn("CVMX_SLI_WIN_WR_MASK not supported on this chip\n");
	return 0x0000000000000030ull;
}
#else
#define CVMX_SLI_WIN_WR_MASK (0x0000000000000030ull)
#endif

/**
 * cvmx_sli_bist_status
 *
 * This register contains results from BIST runs of MAC's memories: 0 = pass (or BIST in
 * progress/never run), 1 = fail.
 */
union cvmx_sli_bist_status {
	uint64_t u64;
	struct cvmx_sli_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t ncb_req                      : 1;  /**< BIST Status for IOI Request FIFO. */
	uint64_t n2p0_c                       : 1;  /**< BIST Status for N2P Port0 Cmd */
	uint64_t n2p0_o                       : 1;  /**< BIST Status for N2P Port0 Data */
	uint64_t n2p1_c                       : 1;  /**< BIST Status for N2P Port1 cmd. */
	uint64_t n2p1_o                       : 1;  /**< BIST Status for N2P Port1 data. */
	uint64_t cpl_p0                       : 1;  /**< BIST Status for CPL Port 0 */
	uint64_t cpl_p1                       : 1;  /**< BIST Status for CPL Port 1 */
	uint64_t reserved_19_24               : 6;
	uint64_t p2n0_c0                      : 1;  /**< BIST Status for P2N Port0 C0 */
	uint64_t p2n0_c1                      : 1;  /**< BIST Status for P2N Port0 C1 */
	uint64_t p2n0_n                       : 1;  /**< BIST Status for P2N Port0 N */
	uint64_t p2n0_p0                      : 1;  /**< BIST Status for P2N Port0 P0 */
	uint64_t p2n0_p1                      : 1;  /**< BIST Status for P2N Port0 P1 */
	uint64_t p2n1_c0                      : 1;  /**< BIST Status for P2N Port1 C0 */
	uint64_t p2n1_c1                      : 1;  /**< BIST Status for P2N Port1 C1 */
	uint64_t p2n1_n                       : 1;  /**< BIST Status for P2N Port1 N */
	uint64_t p2n1_p0                      : 1;  /**< BIST Status for P2N Port1 P0 */
	uint64_t p2n1_p1                      : 1;  /**< BIST Status for P2N Port1 P1 */
	uint64_t reserved_6_8                 : 3;
	uint64_t dsi1_1                       : 1;  /**< BIST Status for DSI1 Memory 1 */
	uint64_t dsi1_0                       : 1;  /**< BIST Status for DSI1 Memory 0 */
	uint64_t dsi0_1                       : 1;  /**< BIST Status for DSI0 Memory 1 */
	uint64_t dsi0_0                       : 1;  /**< BIST Status for DSI0 Memory 0 */
	uint64_t msi                          : 1;  /**< BIST Status for MSI Memory Map */
	uint64_t ncb_cmd                      : 1;  /**< BIST Status for NCB Outbound Commands */
#else
	uint64_t ncb_cmd                      : 1;
	uint64_t msi                          : 1;
	uint64_t dsi0_0                       : 1;
	uint64_t dsi0_1                       : 1;
	uint64_t dsi1_0                       : 1;
	uint64_t dsi1_1                       : 1;
	uint64_t reserved_6_8                 : 3;
	uint64_t p2n1_p1                      : 1;
	uint64_t p2n1_p0                      : 1;
	uint64_t p2n1_n                       : 1;
	uint64_t p2n1_c1                      : 1;
	uint64_t p2n1_c0                      : 1;
	uint64_t p2n0_p1                      : 1;
	uint64_t p2n0_p0                      : 1;
	uint64_t p2n0_n                       : 1;
	uint64_t p2n0_c1                      : 1;
	uint64_t p2n0_c0                      : 1;
	uint64_t reserved_19_24               : 6;
	uint64_t cpl_p1                       : 1;
	uint64_t cpl_p0                       : 1;
	uint64_t n2p1_o                       : 1;
	uint64_t n2p1_c                       : 1;
	uint64_t n2p0_o                       : 1;
	uint64_t n2p0_c                       : 1;
	uint64_t ncb_req                      : 1;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_bist_status_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_31_63               : 33;
	uint64_t n2p0_c                       : 1;  /**< BIST Status for N2P Port0 Cmd */
	uint64_t n2p0_o                       : 1;  /**< BIST Status for N2P Port0 Data */
	uint64_t reserved_27_28               : 2;
	uint64_t cpl_p0                       : 1;  /**< BIST Status for CPL Port 0 */
	uint64_t cpl_p1                       : 1;  /**< BIST Status for CPL Port 1 */
	uint64_t reserved_19_24               : 6;
	uint64_t p2n0_c0                      : 1;  /**< BIST Status for P2N Port0 C0 */
	uint64_t p2n0_c1                      : 1;  /**< BIST Status for P2N Port0 C1 */
	uint64_t p2n0_n                       : 1;  /**< BIST Status for P2N Port0 N */
	uint64_t p2n0_p0                      : 1;  /**< BIST Status for P2N Port0 P0 */
	uint64_t p2n0_p1                      : 1;  /**< BIST Status for P2N Port0 P1 */
	uint64_t p2n1_c0                      : 1;  /**< BIST Status for P2N Port1 C0 */
	uint64_t p2n1_c1                      : 1;  /**< BIST Status for P2N Port1 C1 */
	uint64_t p2n1_n                       : 1;  /**< BIST Status for P2N Port1 N */
	uint64_t p2n1_p0                      : 1;  /**< BIST Status for P2N Port1 P0 */
	uint64_t p2n1_p1                      : 1;  /**< BIST Status for P2N Port1 P1 */
	uint64_t reserved_6_8                 : 3;
	uint64_t dsi1_1                       : 1;  /**< BIST Status for DSI1 Memory 1 */
	uint64_t dsi1_0                       : 1;  /**< BIST Status for DSI1 Memory 0 */
	uint64_t dsi0_1                       : 1;  /**< BIST Status for DSI0 Memory 1 */
	uint64_t dsi0_0                       : 1;  /**< BIST Status for DSI0 Memory 0 */
	uint64_t msi                          : 1;  /**< BIST Status for MSI Memory Map */
	uint64_t ncb_cmd                      : 1;  /**< BIST Status for NCB Outbound Commands */
#else
	uint64_t ncb_cmd                      : 1;
	uint64_t msi                          : 1;
	uint64_t dsi0_0                       : 1;
	uint64_t dsi0_1                       : 1;
	uint64_t dsi1_0                       : 1;
	uint64_t dsi1_1                       : 1;
	uint64_t reserved_6_8                 : 3;
	uint64_t p2n1_p1                      : 1;
	uint64_t p2n1_p0                      : 1;
	uint64_t p2n1_n                       : 1;
	uint64_t p2n1_c1                      : 1;
	uint64_t p2n1_c0                      : 1;
	uint64_t p2n0_p1                      : 1;
	uint64_t p2n0_p0                      : 1;
	uint64_t p2n0_n                       : 1;
	uint64_t p2n0_c1                      : 1;
	uint64_t p2n0_c0                      : 1;
	uint64_t reserved_19_24               : 6;
	uint64_t cpl_p1                       : 1;
	uint64_t cpl_p0                       : 1;
	uint64_t reserved_27_28               : 2;
	uint64_t n2p0_o                       : 1;
	uint64_t n2p0_c                       : 1;
	uint64_t reserved_31_63               : 33;
#endif
	} cn61xx;
	struct cvmx_sli_bist_status_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_31_63               : 33;
	uint64_t n2p0_c                       : 1;  /**< BIST Status for N2P Port0 Cmd */
	uint64_t n2p0_o                       : 1;  /**< BIST Status for N2P Port0 Data */
	uint64_t n2p1_c                       : 1;  /**< BIST Status for N2P Port1 Cmd */
	uint64_t n2p1_o                       : 1;  /**< BIST Status for N2P Port1 Data */
	uint64_t cpl_p0                       : 1;  /**< BIST Status for CPL Port 0 */
	uint64_t cpl_p1                       : 1;  /**< BIST Status for CPL Port 1 */
	uint64_t reserved_19_24               : 6;
	uint64_t p2n0_c0                      : 1;  /**< BIST Status for P2N Port0 C0 */
	uint64_t p2n0_c1                      : 1;  /**< BIST Status for P2N Port0 C1 */
	uint64_t p2n0_n                       : 1;  /**< BIST Status for P2N Port0 N */
	uint64_t p2n0_p0                      : 1;  /**< BIST Status for P2N Port0 P0 */
	uint64_t p2n0_p1                      : 1;  /**< BIST Status for P2N Port0 P1 */
	uint64_t p2n1_c0                      : 1;  /**< BIST Status for P2N Port1 C0 */
	uint64_t p2n1_c1                      : 1;  /**< BIST Status for P2N Port1 C1 */
	uint64_t p2n1_n                       : 1;  /**< BIST Status for P2N Port1 N */
	uint64_t p2n1_p0                      : 1;  /**< BIST Status for P2N Port1 P0 */
	uint64_t p2n1_p1                      : 1;  /**< BIST Status for P2N Port1 P1 */
	uint64_t reserved_6_8                 : 3;
	uint64_t dsi1_1                       : 1;  /**< BIST Status for DSI1 Memory 1 */
	uint64_t dsi1_0                       : 1;  /**< BIST Status for DSI1 Memory 0 */
	uint64_t dsi0_1                       : 1;  /**< BIST Status for DSI0 Memory 1 */
	uint64_t dsi0_0                       : 1;  /**< BIST Status for DSI0 Memory 0 */
	uint64_t msi                          : 1;  /**< BIST Status for MSI Memory Map */
	uint64_t ncb_cmd                      : 1;  /**< BIST Status for NCB Outbound Commands */
#else
	uint64_t ncb_cmd                      : 1;
	uint64_t msi                          : 1;
	uint64_t dsi0_0                       : 1;
	uint64_t dsi0_1                       : 1;
	uint64_t dsi1_0                       : 1;
	uint64_t dsi1_1                       : 1;
	uint64_t reserved_6_8                 : 3;
	uint64_t p2n1_p1                      : 1;
	uint64_t p2n1_p0                      : 1;
	uint64_t p2n1_n                       : 1;
	uint64_t p2n1_c1                      : 1;
	uint64_t p2n1_c0                      : 1;
	uint64_t p2n0_p1                      : 1;
	uint64_t p2n0_p0                      : 1;
	uint64_t p2n0_n                       : 1;
	uint64_t p2n0_c1                      : 1;
	uint64_t p2n0_c0                      : 1;
	uint64_t reserved_19_24               : 6;
	uint64_t cpl_p1                       : 1;
	uint64_t cpl_p0                       : 1;
	uint64_t n2p1_o                       : 1;
	uint64_t n2p1_c                       : 1;
	uint64_t n2p0_o                       : 1;
	uint64_t n2p0_c                       : 1;
	uint64_t reserved_31_63               : 33;
#endif
	} cn63xx;
	struct cvmx_sli_bist_status_cn63xx    cn63xxp1;
	struct cvmx_sli_bist_status_cn61xx    cn66xx;
	struct cvmx_sli_bist_status_s         cn68xx;
	struct cvmx_sli_bist_status_s         cn68xxp1;
	struct cvmx_sli_bist_status_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_31_63               : 33;
	uint64_t n2p0_c                       : 1;  /**< BIST Status for N2P Port0 Cmd */
	uint64_t n2p0_o                       : 1;  /**< BIST Status for N2P Port0 Data */
	uint64_t reserved_27_28               : 2;
	uint64_t cpl_p0                       : 1;  /**< BIST Status for CPL Port 0 */
	uint64_t cpl_p1                       : 1;  /**< BIST Status for CPL Port 1 */
	uint64_t reserved_19_24               : 6;
	uint64_t p2n0_c0                      : 1;  /**< BIST Status for P2N Port0 C0 */
	uint64_t reserved_17_17               : 1;
	uint64_t p2n0_n                       : 1;  /**< BIST Status for P2N Port0 N */
	uint64_t p2n0_p0                      : 1;  /**< BIST Status for P2N Port0 P0 */
	uint64_t reserved_14_14               : 1;
	uint64_t p2n1_c0                      : 1;  /**< BIST Status for P2N Port1 C0 */
	uint64_t reserved_12_12               : 1;
	uint64_t p2n1_n                       : 1;  /**< BIST Status for P2N Port1 N */
	uint64_t p2n1_p0                      : 1;  /**< BIST Status for P2N Port1 P0 */
	uint64_t reserved_6_9                 : 4;
	uint64_t dsi1_1                       : 1;  /**< BIST Status for DSI1 Memory 1 */
	uint64_t dsi1_0                       : 1;  /**< BIST Status for DSI1 Memory 0 */
	uint64_t dsi0_1                       : 1;  /**< BIST Status for DSI0 Memory 1 */
	uint64_t dsi0_0                       : 1;  /**< BIST Status for DSI0 Memory 0 */
	uint64_t msi                          : 1;  /**< BIST Status for MSI Memory Map */
	uint64_t ncb_cmd                      : 1;  /**< BIST Status for NCB Outbound Commands */
#else
	uint64_t ncb_cmd                      : 1;
	uint64_t msi                          : 1;
	uint64_t dsi0_0                       : 1;
	uint64_t dsi0_1                       : 1;
	uint64_t dsi1_0                       : 1;
	uint64_t dsi1_1                       : 1;
	uint64_t reserved_6_9                 : 4;
	uint64_t p2n1_p0                      : 1;
	uint64_t p2n1_n                       : 1;
	uint64_t reserved_12_12               : 1;
	uint64_t p2n1_c0                      : 1;
	uint64_t reserved_14_14               : 1;
	uint64_t p2n0_p0                      : 1;
	uint64_t p2n0_n                       : 1;
	uint64_t reserved_17_17               : 1;
	uint64_t p2n0_c0                      : 1;
	uint64_t reserved_19_24               : 6;
	uint64_t cpl_p1                       : 1;
	uint64_t cpl_p0                       : 1;
	uint64_t reserved_27_28               : 2;
	uint64_t n2p0_o                       : 1;
	uint64_t n2p0_c                       : 1;
	uint64_t reserved_31_63               : 33;
#endif
	} cn70xx;
	struct cvmx_sli_bist_status_s         cn78xx;
	struct cvmx_sli_bist_status_cn61xx    cnf71xx;
};
typedef union cvmx_sli_bist_status cvmx_sli_bist_status_t;

/**
 * cvmx_sli_ctl_port#
 *
 * These registers contains control information for access for Port0 through Port3.
 *
 */
union cvmx_sli_ctl_portx {
	uint64_t u64;
	struct cvmx_sli_ctl_portx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_22_63               : 42;
	uint64_t intd                         : 1;  /**< When '0' Intd wire asserted. Before mapping. */
	uint64_t intc                         : 1;  /**< When '0' Intc wire asserted. Before mapping. */
	uint64_t intb                         : 1;  /**< When '0' Intb wire asserted. Before mapping. */
	uint64_t inta                         : 1;  /**< When '0' Inta wire asserted. Before mapping. */
	uint64_t dis_port                     : 1;  /**< When set the output to the MAC is disabled. This
                                                         occurs when the MAC reset line transitions from
                                                         de-asserted to asserted. Writing a '1' to this
                                                         location will clear this condition when the MAC is
                                                         no longer in reset and the output to the MAC is at
                                                         the begining of a transfer. */
	uint64_t waitl_com                    : 1;  /**< When set '1' casues the SLI to wait for a commit
                                                         from the L2C before sending additional completions
                                                         to the L2C from a MAC.
                                                         Set this for more conservative behavior. Clear
                                                         this for more aggressive, higher-performance
                                                         behavior */
	uint64_t intd_map                     : 2;  /**< Maps INTD to INTA(00), INTB(01), INTC(10) or
                                                         INTD (11). */
	uint64_t intc_map                     : 2;  /**< Maps INTC to INTA(00), INTB(01), INTC(10) or
                                                         INTD (11). */
	uint64_t intb_map                     : 2;  /**< Maps INTB to INTA(00), INTB(01), INTC(10) or
                                                         INTD (11). */
	uint64_t inta_map                     : 2;  /**< Maps INTA to INTA(00), INTB(01), INTC(10) or
                                                         INTD (11). */
	uint64_t ctlp_ro                      : 1;  /**< Relaxed ordering enable for Completion TLPS. */
	uint64_t reserved_6_6                 : 1;
	uint64_t ptlp_ro                      : 1;  /**< Relaxed ordering enable for Posted TLPS. */
	uint64_t reserved_1_4                 : 4;
	uint64_t wait_com                     : 1;  /**< When set '1' casues the SLI to wait for a commit
                                                         from the L2C before sending additional stores to
                                                         the L2C from a MAC.
                                                         The SLI will request a commit on the last store
                                                         if more than one STORE operation is required on
                                                         the NCB.
                                                         Most applications will not notice a difference, so
                                                         should not set this bit. Setting the bit is more
                                                         conservative on ordering, lower performance */
#else
	uint64_t wait_com                     : 1;
	uint64_t reserved_1_4                 : 4;
	uint64_t ptlp_ro                      : 1;
	uint64_t reserved_6_6                 : 1;
	uint64_t ctlp_ro                      : 1;
	uint64_t inta_map                     : 2;
	uint64_t intb_map                     : 2;
	uint64_t intc_map                     : 2;
	uint64_t intd_map                     : 2;
	uint64_t waitl_com                    : 1;
	uint64_t dis_port                     : 1;
	uint64_t inta                         : 1;
	uint64_t intb                         : 1;
	uint64_t intc                         : 1;
	uint64_t intd                         : 1;
	uint64_t reserved_22_63               : 42;
#endif
	} s;
	struct cvmx_sli_ctl_portx_s           cn61xx;
	struct cvmx_sli_ctl_portx_s           cn63xx;
	struct cvmx_sli_ctl_portx_s           cn63xxp1;
	struct cvmx_sli_ctl_portx_s           cn66xx;
	struct cvmx_sli_ctl_portx_s           cn68xx;
	struct cvmx_sli_ctl_portx_s           cn68xxp1;
	struct cvmx_sli_ctl_portx_s           cn70xx;
	struct cvmx_sli_ctl_portx_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t dis_port                     : 1;  /**< When set, the output to the MAC is disabled. This occurs when the MAC reset line
                                                         transitions from de-asserted to asserted. Writing a 1 to this location clears this
                                                         condition when the MAC is no longer in reset and the output to the MAC is at the beginning
                                                         of a transfer. */
	uint64_t waitl_com                    : 1;  /**< When set to 1, causes the SLI to wait for a commit from the L2C before sending additional
                                                         completions to the L2C from the MAC.
                                                         Set this for more conservative behavior. Clear this for more aggressive, higher-
                                                         performance behavior. */
	uint64_t reserved_8_15                : 8;
	uint64_t ctlp_ro                      : 1;  /**< Relaxed ordering enable for completion TLPS */
	uint64_t reserved_6_6                 : 1;
	uint64_t ptlp_ro                      : 1;  /**< Relaxed ordering enable for posted TLPS */
	uint64_t reserved_1_4                 : 4;
	uint64_t wait_com                     : 1;  /**< Wait for commit. When set to 1, causes the SLI to wait for a commit from the L2C before
                                                         sending additional stores to the L2C from the MAC. The SLI requests a commit on the last
                                                         store if more than one STORE operation is required on the IOI. Most applications will not
                                                         notice a difference, so this bit should not be set. Setting the bit is more conservative
                                                         on ordering, lower performance. */
#else
	uint64_t wait_com                     : 1;
	uint64_t reserved_1_4                 : 4;
	uint64_t ptlp_ro                      : 1;
	uint64_t reserved_6_6                 : 1;
	uint64_t ctlp_ro                      : 1;
	uint64_t reserved_8_15                : 8;
	uint64_t waitl_com                    : 1;
	uint64_t dis_port                     : 1;
	uint64_t reserved_18_63               : 46;
#endif
	} cn78xx;
	struct cvmx_sli_ctl_portx_s           cnf71xx;
};
typedef union cvmx_sli_ctl_portx cvmx_sli_ctl_portx_t;

/**
 * cvmx_sli_ctl_status
 *
 * This register contains control and status for SLI. Write operations to this register are not
 * ordered with write/read operations to the MAC memory space. To ensure that a write has
 * completed, software must read the register before making an access (i.e. MAC memory space)
 * that requires the value of this register to be updated.
 */
union cvmx_sli_ctl_status {
	uint64_t u64;
	struct cvmx_sli_ctl_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t m2s1_ncbi                    : 4;  /**< Contains the IOBI that traffic from M2S1 is placed on. */
	uint64_t m2s0_ncbi                    : 4;  /**< Contains the IOBI that traffic from M2S0 is placed on. */
	uint64_t oci_id                       : 4;  /**< The OCI ID. */
	uint64_t p1_ntags                     : 6;  /**< Number of tags available for MAC port 1.
                                                         In RC mode, one tag is needed for each outbound TLP that requires a CPL TLP.
                                                         In EP mode, the number of tags required for a TLP request is 1 per 64-bytes of CPL data +
                                                         1.
                                                         This field should only be written as part of a reset sequence and before issuing any read
                                                         operations, CFGs, or I/O transactions from the core(s). */
	uint64_t p0_ntags                     : 6;  /**< Number of tags available for outbound TLPs to the
                                                         MACS. One tag is needed for each outbound TLP that
                                                         requires a CPL TLP.
                                                         This field should only be written as part of
                                                         reset sequence, before issuing any reads, CFGs, or
                                                         IO transactions from the core(s). */
	uint64_t chip_rev                     : 8;  /**< The chip revision. */
#else
	uint64_t chip_rev                     : 8;
	uint64_t p0_ntags                     : 6;
	uint64_t p1_ntags                     : 6;
	uint64_t oci_id                       : 4;
	uint64_t m2s0_ncbi                    : 4;
	uint64_t m2s1_ncbi                    : 4;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_ctl_status_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t p0_ntags                     : 6;  /**< Number of tags available for outbound TLPs to the
                                                         MACS. One tag is needed for each outbound TLP that
                                                         requires a CPL TLP.
                                                         This field should only be written as part of
                                                         reset sequence, before issuing any reads, CFGs, or
                                                         IO transactions from the core(s). */
	uint64_t chip_rev                     : 8;  /**< The chip revision. */
#else
	uint64_t chip_rev                     : 8;
	uint64_t p0_ntags                     : 6;
	uint64_t reserved_14_63               : 50;
#endif
	} cn61xx;
	struct cvmx_sli_ctl_status_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t p1_ntags                     : 6;  /**< Number of tags available for MAC Port1.
                                                         In RC mode 1 tag is needed for each outbound TLP
                                                         that requires a CPL TLP. In Endpoint mode the
                                                         number of tags required for a TLP request is
                                                         1 per 64-bytes of CPL data + 1.
                                                         This field should only be written as part of
                                                         reset sequence, before issuing any reads, CFGs, or
                                                         IO transactions from the core(s). */
	uint64_t p0_ntags                     : 6;  /**< Number of tags available for MAC Port0.
                                                         In RC mode 1 tag is needed for each outbound TLP
                                                         that requires a CPL TLP. In Endpoint mode the
                                                         number of tags required for a TLP request is
                                                         1 per 64-bytes of CPL data + 1.
                                                         This field should only be written as part of
                                                         reset sequence, before issuing any reads, CFGs, or
                                                         IO transactions from the core(s). */
	uint64_t chip_rev                     : 8;  /**< The chip revision. */
#else
	uint64_t chip_rev                     : 8;
	uint64_t p0_ntags                     : 6;
	uint64_t p1_ntags                     : 6;
	uint64_t reserved_20_63               : 44;
#endif
	} cn63xx;
	struct cvmx_sli_ctl_status_cn63xx     cn63xxp1;
	struct cvmx_sli_ctl_status_cn61xx     cn66xx;
	struct cvmx_sli_ctl_status_cn63xx     cn68xx;
	struct cvmx_sli_ctl_status_cn63xx     cn68xxp1;
	struct cvmx_sli_ctl_status_cn63xx     cn70xx;
	struct cvmx_sli_ctl_status_s          cn78xx;
	struct cvmx_sli_ctl_status_cn61xx     cnf71xx;
};
typedef union cvmx_sli_ctl_status cvmx_sli_ctl_status_t;

/**
 * cvmx_sli_data_out_cnt
 *
 * This register contains the EXEC data out FIFO count and the data unload counter.
 *
 */
union cvmx_sli_data_out_cnt {
	uint64_t u64;
	struct cvmx_sli_data_out_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_44_63               : 20;
	uint64_t p1_ucnt                      : 16; /**< FIFO1 Unload Count. This counter is incremented by
                                                         '1' every time a word is removed from Data Out
                                                         FIFO1. Whose count is shown in P1_FCNT. */
	uint64_t p1_fcnt                      : 6;  /**< FIFO1 Data Out Count. Number of address data words
                                                         presently buffered in the FIFO1.
                                                         MACs associated with FIFO1: NONE */
	uint64_t p0_ucnt                      : 16; /**< FIFO0 Unload Count. This counter is incremented by
                                                         '1' every time a word is removed from Data Out
                                                         FIFO0. Whose count is shown in P0_FCNT. */
	uint64_t p0_fcnt                      : 6;  /**< FIFO0 Data Out Count. Number of address data words
                                                         presently buffered in the FIFO0.
                                                         MACs associated with FIFO0: PCIe0, PCIe1 */
#else
	uint64_t p0_fcnt                      : 6;
	uint64_t p0_ucnt                      : 16;
	uint64_t p1_fcnt                      : 6;
	uint64_t p1_ucnt                      : 16;
	uint64_t reserved_44_63               : 20;
#endif
	} s;
	struct cvmx_sli_data_out_cnt_s        cn61xx;
	struct cvmx_sli_data_out_cnt_s        cn63xx;
	struct cvmx_sli_data_out_cnt_s        cn63xxp1;
	struct cvmx_sli_data_out_cnt_s        cn66xx;
	struct cvmx_sli_data_out_cnt_s        cn68xx;
	struct cvmx_sli_data_out_cnt_s        cn68xxp1;
	struct cvmx_sli_data_out_cnt_s        cn70xx;
	struct cvmx_sli_data_out_cnt_s        cn78xx;
	struct cvmx_sli_data_out_cnt_s        cnf71xx;
};
typedef union cvmx_sli_data_out_cnt cvmx_sli_data_out_cnt_t;

/**
 * cvmx_sli_dbg_data
 *
 * SLI_DBG_DATA = SLI Debug Data Register
 *
 * Value returned on the debug-data lines from the RSLs
 */
union cvmx_sli_dbg_data {
	uint64_t u64;
	struct cvmx_sli_dbg_data_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t dsel_ext                     : 1;  /**< Allows changes in the external pins to set the
                                                         debug select value. */
	uint64_t data                         : 17; /**< Value on the debug data lines. */
#else
	uint64_t data                         : 17;
	uint64_t dsel_ext                     : 1;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_sli_dbg_data_s            cn61xx;
	struct cvmx_sli_dbg_data_s            cn63xx;
	struct cvmx_sli_dbg_data_s            cn63xxp1;
	struct cvmx_sli_dbg_data_s            cn66xx;
	struct cvmx_sli_dbg_data_s            cn68xx;
	struct cvmx_sli_dbg_data_s            cn68xxp1;
	struct cvmx_sli_dbg_data_s            cnf71xx;
};
typedef union cvmx_sli_dbg_data cvmx_sli_dbg_data_t;

/**
 * cvmx_sli_dbg_select
 *
 * SLI_DBG_SELECT = Debug Select Register
 *
 * Contains the debug select value last written to the RSLs.
 */
union cvmx_sli_dbg_select {
	uint64_t u64;
	struct cvmx_sli_dbg_select_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_33_63               : 31;
	uint64_t adbg_sel                     : 1;  /**< When set '1' the SLI_DBG_DATA[DATA] will only be
                                                         loaded when SLI_DBG_DATA[DATA] bit [16] is a '1'.
                                                         When the debug data comes from an Async-RSL bit
                                                         16 is used to tell that the data present is valid. */
	uint64_t dbg_sel                      : 32; /**< When this register is written the RML will write
                                                         all "F"s to the previous RTL to disable it from
                                                         sending Debug-Data. The RML will then send a write
                                                         to the new RSL with the supplied Debug-Select
                                                         value. Because it takes time for the new Debug
                                                         Select value to take effect and the requested
                                                         Debug-Data to return, time is needed to the new
                                                         Debug-Data to arrive.  The inititator of the Debug
                                                         Select should issue a read to a CSR before reading
                                                         the Debug Data (this read could also be to the
                                                         SLI_DBG_DATA but the returned value for the first
                                                         read will return NS data. */
#else
	uint64_t dbg_sel                      : 32;
	uint64_t adbg_sel                     : 1;
	uint64_t reserved_33_63               : 31;
#endif
	} s;
	struct cvmx_sli_dbg_select_s          cn61xx;
	struct cvmx_sli_dbg_select_s          cn63xx;
	struct cvmx_sli_dbg_select_s          cn63xxp1;
	struct cvmx_sli_dbg_select_s          cn66xx;
	struct cvmx_sli_dbg_select_s          cn68xx;
	struct cvmx_sli_dbg_select_s          cn68xxp1;
	struct cvmx_sli_dbg_select_s          cnf71xx;
};
typedef union cvmx_sli_dbg_select cvmx_sli_dbg_select_t;

/**
 * cvmx_sli_dma#_cnt
 *
 * These registers contain the DMA count values.
 *
 */
union cvmx_sli_dmax_cnt {
	uint64_t u64;
	struct cvmx_sli_dmax_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t cnt                          : 32; /**< The DMA counter.
                                                         Writing this field will cause the written value
                                                         to be subtracted from DMA. HW will optionally
                                                         increment this field after it completes an
                                                         OUTBOUND or EXTERNAL-ONLY DMA instruction. These
                                                         increments may cause interrupts. Refer to
                                                         SLI_DMAx_INT_LEVEL and SLI_INT_SUM[DCNT,DTIME]. */
#else
	uint64_t cnt                          : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_dmax_cnt_s            cn61xx;
	struct cvmx_sli_dmax_cnt_s            cn63xx;
	struct cvmx_sli_dmax_cnt_s            cn63xxp1;
	struct cvmx_sli_dmax_cnt_s            cn66xx;
	struct cvmx_sli_dmax_cnt_s            cn68xx;
	struct cvmx_sli_dmax_cnt_s            cn68xxp1;
	struct cvmx_sli_dmax_cnt_s            cn70xx;
	struct cvmx_sli_dmax_cnt_s            cn78xx;
	struct cvmx_sli_dmax_cnt_s            cnf71xx;
};
typedef union cvmx_sli_dmax_cnt cvmx_sli_dmax_cnt_t;

/**
 * cvmx_sli_dma#_int_level
 *
 * These registers contain the thresholds for DMA count and timer interrupts.
 *
 */
union cvmx_sli_dmax_int_level {
	uint64_t u64;
	struct cvmx_sli_dmax_int_level_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t time                         : 32; /**< Whenever the SLI_DMAx_TIM[TIM] timer exceeds
                                                         this value, SLI_INT_SUM[DTIME<x>] is set.
                                                         The SLI_DMAx_TIM[TIM] timer increments every SLI
                                                         clock whenever SLI_DMAx_CNT[CNT]!=0, and is
                                                         cleared when SLI_INT_SUM[DTIME<x>] is written with
                                                         one. */
	uint64_t cnt                          : 32; /**< Whenever SLI_DMAx_CNT[CNT] exceeds this value,
                                                         SLI_INT_SUM[DCNT<x>] is set. */
#else
	uint64_t cnt                          : 32;
	uint64_t time                         : 32;
#endif
	} s;
	struct cvmx_sli_dmax_int_level_s      cn61xx;
	struct cvmx_sli_dmax_int_level_s      cn63xx;
	struct cvmx_sli_dmax_int_level_s      cn63xxp1;
	struct cvmx_sli_dmax_int_level_s      cn66xx;
	struct cvmx_sli_dmax_int_level_s      cn68xx;
	struct cvmx_sli_dmax_int_level_s      cn68xxp1;
	struct cvmx_sli_dmax_int_level_s      cn70xx;
	struct cvmx_sli_dmax_int_level_s      cn78xx;
	struct cvmx_sli_dmax_int_level_s      cnf71xx;
};
typedef union cvmx_sli_dmax_int_level cvmx_sli_dmax_int_level_t;

/**
 * cvmx_sli_dma#_tim
 *
 * These registers contain the DMA timer values.
 *
 */
union cvmx_sli_dmax_tim {
	uint64_t u64;
	struct cvmx_sli_dmax_tim_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t tim                          : 32; /**< The DMA timer value.
                                                         The timer will increment when SLI_DMAx_CNT[CNT]!=0
                                                         and will clear when SLI_DMAx_CNT[CNT]==0 */
#else
	uint64_t tim                          : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_dmax_tim_s            cn61xx;
	struct cvmx_sli_dmax_tim_s            cn63xx;
	struct cvmx_sli_dmax_tim_s            cn63xxp1;
	struct cvmx_sli_dmax_tim_s            cn66xx;
	struct cvmx_sli_dmax_tim_s            cn68xx;
	struct cvmx_sli_dmax_tim_s            cn68xxp1;
	struct cvmx_sli_dmax_tim_s            cn70xx;
	struct cvmx_sli_dmax_tim_s            cn78xx;
	struct cvmx_sli_dmax_tim_s            cnf71xx;
};
typedef union cvmx_sli_dmax_tim cvmx_sli_dmax_tim_t;

/**
 * cvmx_sli_int_enb_ciu
 *
 * Used to enable the various interrupting conditions of SLI
 *
 */
union cvmx_sli_int_enb_ciu {
	uint64_t u64;
	struct cvmx_sli_int_enb_ciu_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_62_63               : 2;
	uint64_t pipe_err                     : 1;  /**< Illegal packet csr address. */
	uint64_t ill_pad                      : 1;  /**< Illegal packet csr address. */
	uint64_t sprt3_err                    : 1;  /**< Error Response received on SLI port 3. */
	uint64_t sprt2_err                    : 1;  /**< Error Response received on SLI port 2. */
	uint64_t sprt1_err                    : 1;  /**< Error Response received on SLI port 1. */
	uint64_t sprt0_err                    : 1;  /**< Error Response received on SLI port 0. */
	uint64_t pins_err                     : 1;  /**< Read Error during packet instruction fetch. */
	uint64_t pop_err                      : 1;  /**< Read Error during packet scatter pointer fetch. */
	uint64_t pdi_err                      : 1;  /**< Read Error during packet data fetch. */
	uint64_t pgl_err                      : 1;  /**< Read Error during gather list fetch. */
	uint64_t pin_bp                       : 1;  /**< Packet Input Count exceeded WMARK. */
	uint64_t pout_err                     : 1;  /**< Packet Out Interrupt, Error From PKO. */
	uint64_t psldbof                      : 1;  /**< Packet Scatterlist Doorbell Count Overflow. */
	uint64_t pidbof                       : 1;  /**< Packet Instruction Doorbell Count Overflow. */
	uint64_t reserved_38_47               : 10;
	uint64_t dtime                        : 2;  /**< DMA Timer Interrupts */
	uint64_t dcnt                         : 2;  /**< DMA Count Interrupts */
	uint64_t dmafi                        : 2;  /**< DMA set Forced Interrupts */
	uint64_t reserved_29_31               : 3;
	uint64_t mio_int2                     : 1;  /**< Enables SLI_INT_SUM[28] to generate an
                                                         interrupt on the RSL.
                                                         THIS SHOULD NEVER BE SET */
	uint64_t m3_un_wi                     : 1;  /**< Reserved. */
	uint64_t m3_un_b0                     : 1;  /**< Reserved. */
	uint64_t m3_up_wi                     : 1;  /**< Reserved. */
	uint64_t m3_up_b0                     : 1;  /**< Reserved. */
	uint64_t m2_un_wi                     : 1;  /**< Reserved. */
	uint64_t m2_un_b0                     : 1;  /**< Reserved. */
	uint64_t m2_up_wi                     : 1;  /**< Reserved. */
	uint64_t m2_up_b0                     : 1;  /**< Reserved. */
	uint64_t reserved_18_19               : 2;
	uint64_t mio_int1                     : 1;  /**< Enables SLI_INT_SUM[17] to generate an
                                                         interrupt on the RSL.
                                                         THIS SHOULD NEVER BE SET */
	uint64_t mio_int0                     : 1;  /**< Enables SLI_INT_SUM[16] to generate an
                                                         interrupt on the RSL.
                                                         THIS SHOULD NEVER BE SET */
	uint64_t m1_un_wi                     : 1;  /**< Enables SLI_INT_SUM[15] to generate an
                                                         interrupt on the RSL. */
	uint64_t m1_un_b0                     : 1;  /**< Enables SLI_INT_SUM[14] to generate an
                                                         interrupt on the RSL. */
	uint64_t m1_up_wi                     : 1;  /**< Enables SLI_INT_SUM[13] to generate an
                                                         interrupt on the RSL. */
	uint64_t m1_up_b0                     : 1;  /**< Enables SLI_INT_SUM[12] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_un_wi                     : 1;  /**< Enables SLI_INT_SUM[11] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_un_b0                     : 1;  /**< Enables SLI_INT_SUM[10] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_up_wi                     : 1;  /**< Enables SLI_INT_SUM[9] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_up_b0                     : 1;  /**< Enables SLI_INT_SUM[8] to generate an
                                                         interrupt on the RSL. */
	uint64_t reserved_6_7                 : 2;
	uint64_t ptime                        : 1;  /**< Enables SLI_INT_SUM[5] to generate an
                                                         interrupt on the RSL. */
	uint64_t pcnt                         : 1;  /**< Enables SLI_INT_SUM[4] to generate an
                                                         interrupt on the RSL. */
	uint64_t iob2big                      : 1;  /**< Enables SLI_INT_SUM[3] to generate an
                                                         interrupt on the RSL. */
	uint64_t bar0_to                      : 1;  /**< Enables SLI_INT_SUM[2] to generate an
                                                         interrupt on the RSL. */
	uint64_t reserved_1_1                 : 1;
	uint64_t rml_to                       : 1;  /**< Enables SLI_INT_SUM[0] to generate an
                                                         interrupt on the RSL. */
#else
	uint64_t rml_to                       : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t bar0_to                      : 1;
	uint64_t iob2big                      : 1;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t reserved_6_7                 : 2;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t mio_int0                     : 1;
	uint64_t mio_int1                     : 1;
	uint64_t reserved_18_19               : 2;
	uint64_t m2_up_b0                     : 1;
	uint64_t m2_up_wi                     : 1;
	uint64_t m2_un_b0                     : 1;
	uint64_t m2_un_wi                     : 1;
	uint64_t m3_up_b0                     : 1;
	uint64_t m3_up_wi                     : 1;
	uint64_t m3_un_b0                     : 1;
	uint64_t m3_un_wi                     : 1;
	uint64_t mio_int2                     : 1;
	uint64_t reserved_29_31               : 3;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_38_47               : 10;
	uint64_t pidbof                       : 1;
	uint64_t psldbof                      : 1;
	uint64_t pout_err                     : 1;
	uint64_t pin_bp                       : 1;
	uint64_t pgl_err                      : 1;
	uint64_t pdi_err                      : 1;
	uint64_t pop_err                      : 1;
	uint64_t pins_err                     : 1;
	uint64_t sprt0_err                    : 1;
	uint64_t sprt1_err                    : 1;
	uint64_t sprt2_err                    : 1;
	uint64_t sprt3_err                    : 1;
	uint64_t ill_pad                      : 1;
	uint64_t pipe_err                     : 1;
	uint64_t reserved_62_63               : 2;
#endif
	} s;
	struct cvmx_sli_int_enb_ciu_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_61_63               : 3;
	uint64_t ill_pad                      : 1;  /**< Illegal packet csr address. */
	uint64_t sprt3_err                    : 1;  /**< Error Response received on SLI port 3. */
	uint64_t sprt2_err                    : 1;  /**< Error Response received on SLI port 2. */
	uint64_t sprt1_err                    : 1;  /**< Error Response received on SLI port 1. */
	uint64_t sprt0_err                    : 1;  /**< Error Response received on SLI port 0. */
	uint64_t pins_err                     : 1;  /**< Read Error during packet instruction fetch. */
	uint64_t pop_err                      : 1;  /**< Read Error during packet scatter pointer fetch. */
	uint64_t pdi_err                      : 1;  /**< Read Error during packet data fetch. */
	uint64_t pgl_err                      : 1;  /**< Read Error during gather list fetch. */
	uint64_t pin_bp                       : 1;  /**< Packet Input Count exceeded WMARK. */
	uint64_t pout_err                     : 1;  /**< Packet Out Interrupt, Error From PKO. */
	uint64_t psldbof                      : 1;  /**< Packet Scatterlist Doorbell Count Overflow. */
	uint64_t pidbof                       : 1;  /**< Packet Instruction Doorbell Count Overflow. */
	uint64_t reserved_38_47               : 10;
	uint64_t dtime                        : 2;  /**< DMA Timer Interrupts */
	uint64_t dcnt                         : 2;  /**< DMA Count Interrupts */
	uint64_t dmafi                        : 2;  /**< DMA set Forced Interrupts */
	uint64_t reserved_28_31               : 4;
	uint64_t m3_un_wi                     : 1;  /**< Reserved. */
	uint64_t m3_un_b0                     : 1;  /**< Reserved. */
	uint64_t m3_up_wi                     : 1;  /**< Reserved. */
	uint64_t m3_up_b0                     : 1;  /**< Reserved. */
	uint64_t m2_un_wi                     : 1;  /**< Reserved. */
	uint64_t m2_un_b0                     : 1;  /**< Reserved. */
	uint64_t m2_up_wi                     : 1;  /**< Reserved. */
	uint64_t m2_up_b0                     : 1;  /**< Reserved. */
	uint64_t reserved_18_19               : 2;
	uint64_t mio_int1                     : 1;  /**< Enables SLI_INT_SUM[17] to generate an
                                                         interrupt on the RSL.
                                                         THIS SHOULD NEVER BE SET */
	uint64_t mio_int0                     : 1;  /**< Enables SLI_INT_SUM[16] to generate an
                                                         interrupt on the RSL.
                                                         THIS SHOULD NEVER BE SET */
	uint64_t m1_un_wi                     : 1;  /**< Enables SLI_INT_SUM[15] to generate an
                                                         interrupt on the RSL. */
	uint64_t m1_un_b0                     : 1;  /**< Enables SLI_INT_SUM[14] to generate an
                                                         interrupt on the RSL. */
	uint64_t m1_up_wi                     : 1;  /**< Enables SLI_INT_SUM[13] to generate an
                                                         interrupt on the RSL. */
	uint64_t m1_up_b0                     : 1;  /**< Enables SLI_INT_SUM[12] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_un_wi                     : 1;  /**< Enables SLI_INT_SUM[11] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_un_b0                     : 1;  /**< Enables SLI_INT_SUM[10] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_up_wi                     : 1;  /**< Enables SLI_INT_SUM[9] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_up_b0                     : 1;  /**< Enables SLI_INT_SUM[8] to generate an
                                                         interrupt on the RSL. */
	uint64_t reserved_6_7                 : 2;
	uint64_t ptime                        : 1;  /**< Enables SLI_INT_SUM[5] to generate an
                                                         interrupt on the RSL. */
	uint64_t pcnt                         : 1;  /**< Enables SLI_INT_SUM[4] to generate an
                                                         interrupt on the RSL. */
	uint64_t iob2big                      : 1;  /**< Enables SLI_INT_SUM[3] to generate an
                                                         interrupt on the RSL. */
	uint64_t bar0_to                      : 1;  /**< Enables SLI_INT_SUM[2] to generate an
                                                         interrupt on the RSL. */
	uint64_t reserved_1_1                 : 1;
	uint64_t rml_to                       : 1;  /**< Enables SLI_INT_SUM[0] to generate an
                                                         interrupt on the RSL. */
#else
	uint64_t rml_to                       : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t bar0_to                      : 1;
	uint64_t iob2big                      : 1;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t reserved_6_7                 : 2;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t mio_int0                     : 1;
	uint64_t mio_int1                     : 1;
	uint64_t reserved_18_19               : 2;
	uint64_t m2_up_b0                     : 1;
	uint64_t m2_up_wi                     : 1;
	uint64_t m2_un_b0                     : 1;
	uint64_t m2_un_wi                     : 1;
	uint64_t m3_up_b0                     : 1;
	uint64_t m3_up_wi                     : 1;
	uint64_t m3_un_b0                     : 1;
	uint64_t m3_un_wi                     : 1;
	uint64_t reserved_28_31               : 4;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_38_47               : 10;
	uint64_t pidbof                       : 1;
	uint64_t psldbof                      : 1;
	uint64_t pout_err                     : 1;
	uint64_t pin_bp                       : 1;
	uint64_t pgl_err                      : 1;
	uint64_t pdi_err                      : 1;
	uint64_t pop_err                      : 1;
	uint64_t pins_err                     : 1;
	uint64_t sprt0_err                    : 1;
	uint64_t sprt1_err                    : 1;
	uint64_t sprt2_err                    : 1;
	uint64_t sprt3_err                    : 1;
	uint64_t ill_pad                      : 1;
	uint64_t reserved_61_63               : 3;
#endif
	} cn61xx;
	struct cvmx_sli_int_enb_ciu_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_61_63               : 3;
	uint64_t ill_pad                      : 1;  /**< Illegal packet csr address. */
	uint64_t reserved_58_59               : 2;
	uint64_t sprt1_err                    : 1;  /**< Error Response received on SLI port 1. */
	uint64_t sprt0_err                    : 1;  /**< Error Response received on SLI port 0. */
	uint64_t pins_err                     : 1;  /**< Read Error during packet instruction fetch. */
	uint64_t pop_err                      : 1;  /**< Read Error during packet scatter pointer fetch. */
	uint64_t pdi_err                      : 1;  /**< Read Error during packet data fetch. */
	uint64_t pgl_err                      : 1;  /**< Read Error during gather list fetch. */
	uint64_t pin_bp                       : 1;  /**< Packet Input Count exceeded WMARK. */
	uint64_t pout_err                     : 1;  /**< Packet Out Interrupt, Error From PKO. */
	uint64_t psldbof                      : 1;  /**< Packet Scatterlist Doorbell Count Overflow. */
	uint64_t pidbof                       : 1;  /**< Packet Instruction Doorbell Count Overflow. */
	uint64_t reserved_38_47               : 10;
	uint64_t dtime                        : 2;  /**< DMA Timer Interrupts */
	uint64_t dcnt                         : 2;  /**< DMA Count Interrupts */
	uint64_t dmafi                        : 2;  /**< DMA set Forced Interrupts */
	uint64_t reserved_18_31               : 14;
	uint64_t mio_int1                     : 1;  /**< Enables SLI_INT_SUM[17] to generate an
                                                         interrupt on the RSL.
                                                         THIS SHOULD NEVER BE SET */
	uint64_t mio_int0                     : 1;  /**< Enables SLI_INT_SUM[16] to generate an
                                                         interrupt on the RSL.
                                                         THIS SHOULD NEVER BE SET */
	uint64_t m1_un_wi                     : 1;  /**< Enables SLI_INT_SUM[15] to generate an
                                                         interrupt on the RSL. */
	uint64_t m1_un_b0                     : 1;  /**< Enables SLI_INT_SUM[14] to generate an
                                                         interrupt on the RSL. */
	uint64_t m1_up_wi                     : 1;  /**< Enables SLI_INT_SUM[13] to generate an
                                                         interrupt on the RSL. */
	uint64_t m1_up_b0                     : 1;  /**< Enables SLI_INT_SUM[12] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_un_wi                     : 1;  /**< Enables SLI_INT_SUM[11] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_un_b0                     : 1;  /**< Enables SLI_INT_SUM[10] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_up_wi                     : 1;  /**< Enables SLI_INT_SUM[9] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_up_b0                     : 1;  /**< Enables SLI_INT_SUM[8] to generate an
                                                         interrupt on the RSL. */
	uint64_t reserved_6_7                 : 2;
	uint64_t ptime                        : 1;  /**< Enables SLI_INT_SUM[5] to generate an
                                                         interrupt on the RSL. */
	uint64_t pcnt                         : 1;  /**< Enables SLI_INT_SUM[4] to generate an
                                                         interrupt on the RSL. */
	uint64_t iob2big                      : 1;  /**< Enables SLI_INT_SUM[3] to generate an
                                                         interrupt on the RSL. */
	uint64_t bar0_to                      : 1;  /**< Enables SLI_INT_SUM[2] to generate an
                                                         interrupt on the RSL. */
	uint64_t reserved_1_1                 : 1;
	uint64_t rml_to                       : 1;  /**< Enables SLI_INT_SUM[0] to generate an
                                                         interrupt on the RSL. */
#else
	uint64_t rml_to                       : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t bar0_to                      : 1;
	uint64_t iob2big                      : 1;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t reserved_6_7                 : 2;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t mio_int0                     : 1;
	uint64_t mio_int1                     : 1;
	uint64_t reserved_18_31               : 14;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_38_47               : 10;
	uint64_t pidbof                       : 1;
	uint64_t psldbof                      : 1;
	uint64_t pout_err                     : 1;
	uint64_t pin_bp                       : 1;
	uint64_t pgl_err                      : 1;
	uint64_t pdi_err                      : 1;
	uint64_t pop_err                      : 1;
	uint64_t pins_err                     : 1;
	uint64_t sprt0_err                    : 1;
	uint64_t sprt1_err                    : 1;
	uint64_t reserved_58_59               : 2;
	uint64_t ill_pad                      : 1;
	uint64_t reserved_61_63               : 3;
#endif
	} cn63xx;
	struct cvmx_sli_int_enb_ciu_cn63xx    cn63xxp1;
	struct cvmx_sli_int_enb_ciu_cn61xx    cn66xx;
	struct cvmx_sli_int_enb_ciu_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_62_63               : 2;
	uint64_t pipe_err                     : 1;  /**< Illegal packet csr address. */
	uint64_t ill_pad                      : 1;  /**< Illegal packet csr address. */
	uint64_t reserved_58_59               : 2;
	uint64_t sprt1_err                    : 1;  /**< Error Response received on SLI port 1. */
	uint64_t sprt0_err                    : 1;  /**< Error Response received on SLI port 0. */
	uint64_t pins_err                     : 1;  /**< Read Error during packet instruction fetch. */
	uint64_t pop_err                      : 1;  /**< Read Error during packet scatter pointer fetch. */
	uint64_t pdi_err                      : 1;  /**< Read Error during packet data fetch. */
	uint64_t pgl_err                      : 1;  /**< Read Error during gather list fetch. */
	uint64_t reserved_51_51               : 1;
	uint64_t pout_err                     : 1;  /**< Packet Out Interrupt, Error From PKO. */
	uint64_t psldbof                      : 1;  /**< Packet Scatterlist Doorbell Count Overflow. */
	uint64_t pidbof                       : 1;  /**< Packet Instruction Doorbell Count Overflow. */
	uint64_t reserved_38_47               : 10;
	uint64_t dtime                        : 2;  /**< DMA Timer Interrupts */
	uint64_t dcnt                         : 2;  /**< DMA Count Interrupts */
	uint64_t dmafi                        : 2;  /**< DMA set Forced Interrupts */
	uint64_t reserved_18_31               : 14;
	uint64_t mio_int1                     : 1;  /**< Enables SLI_INT_SUM[17] to generate an
                                                         interrupt on the RSL.
                                                         THIS SHOULD NEVER BE SET */
	uint64_t mio_int0                     : 1;  /**< Enables SLI_INT_SUM[16] to generate an
                                                         interrupt on the RSL.
                                                         THIS SHOULD NEVER BE SET */
	uint64_t m1_un_wi                     : 1;  /**< Enables SLI_INT_SUM[15] to generate an
                                                         interrupt on the RSL. */
	uint64_t m1_un_b0                     : 1;  /**< Enables SLI_INT_SUM[14] to generate an
                                                         interrupt on the RSL. */
	uint64_t m1_up_wi                     : 1;  /**< Enables SLI_INT_SUM[13] to generate an
                                                         interrupt on the RSL. */
	uint64_t m1_up_b0                     : 1;  /**< Enables SLI_INT_SUM[12] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_un_wi                     : 1;  /**< Enables SLI_INT_SUM[11] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_un_b0                     : 1;  /**< Enables SLI_INT_SUM[10] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_up_wi                     : 1;  /**< Enables SLI_INT_SUM[9] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_up_b0                     : 1;  /**< Enables SLI_INT_SUM[8] to generate an
                                                         interrupt on the RSL. */
	uint64_t reserved_6_7                 : 2;
	uint64_t ptime                        : 1;  /**< Enables SLI_INT_SUM[5] to generate an
                                                         interrupt on the RSL. */
	uint64_t pcnt                         : 1;  /**< Enables SLI_INT_SUM[4] to generate an
                                                         interrupt on the RSL. */
	uint64_t iob2big                      : 1;  /**< Enables SLI_INT_SUM[3] to generate an
                                                         interrupt on the RSL. */
	uint64_t bar0_to                      : 1;  /**< Enables SLI_INT_SUM[2] to generate an
                                                         interrupt on the RSL. */
	uint64_t reserved_1_1                 : 1;
	uint64_t rml_to                       : 1;  /**< Enables SLI_INT_SUM[0] to generate an
                                                         interrupt on the RSL. */
#else
	uint64_t rml_to                       : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t bar0_to                      : 1;
	uint64_t iob2big                      : 1;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t reserved_6_7                 : 2;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t mio_int0                     : 1;
	uint64_t mio_int1                     : 1;
	uint64_t reserved_18_31               : 14;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_38_47               : 10;
	uint64_t pidbof                       : 1;
	uint64_t psldbof                      : 1;
	uint64_t pout_err                     : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t pgl_err                      : 1;
	uint64_t pdi_err                      : 1;
	uint64_t pop_err                      : 1;
	uint64_t pins_err                     : 1;
	uint64_t sprt0_err                    : 1;
	uint64_t sprt1_err                    : 1;
	uint64_t reserved_58_59               : 2;
	uint64_t ill_pad                      : 1;
	uint64_t pipe_err                     : 1;
	uint64_t reserved_62_63               : 2;
#endif
	} cn68xx;
	struct cvmx_sli_int_enb_ciu_cn68xx    cn68xxp1;
	struct cvmx_sli_int_enb_ciu_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_61_63               : 3;
	uint64_t ill_pad                      : 1;  /**< Illegal packet csr address. */
	uint64_t sprt3_err                    : 1;  /**< Error Response received on SLI port 3. */
	uint64_t sprt2_err                    : 1;  /**< Error Response received on SLI port 2. */
	uint64_t sprt1_err                    : 1;  /**< Error Response received on SLI port 1. */
	uint64_t sprt0_err                    : 1;  /**< Error Response received on SLI port 0. */
	uint64_t pins_err                     : 1;  /**< Read Error during packet instruction fetch. */
	uint64_t pop_err                      : 1;  /**< Read Error during packet scatter pointer fetch. */
	uint64_t pdi_err                      : 1;  /**< Read Error during packet data fetch. */
	uint64_t pgl_err                      : 1;  /**< Read Error during gather list fetch. */
	uint64_t pin_bp                       : 1;  /**< Packet Input Count exceeded WMARK. */
	uint64_t pout_err                     : 1;  /**< Packet Out Interrupt, Error From PKO. */
	uint64_t psldbof                      : 1;  /**< Packet Scatterlist Doorbell Count Overflow. */
	uint64_t pidbof                       : 1;  /**< Packet Instruction Doorbell Count Overflow. */
	uint64_t reserved_38_47               : 10;
	uint64_t dtime                        : 2;  /**< DMA Timer Interrupts */
	uint64_t dcnt                         : 2;  /**< DMA Count Interrupts */
	uint64_t dmafi                        : 2;  /**< DMA set Forced Interrupts */
	uint64_t reserved_29_31               : 3;
	uint64_t mio_int2                     : 1;  /**< Enables SLI_INT_SUM[28] to generate an
                                                         interrupt on the RSL.
                                                         THIS SHOULD NEVER BE SET */
	uint64_t m3_un_wi                     : 1;  /**< Reserved. */
	uint64_t m3_un_b0                     : 1;  /**< Reserved. */
	uint64_t m3_up_wi                     : 1;  /**< Reserved. */
	uint64_t m3_up_b0                     : 1;  /**< Reserved. */
	uint64_t m2_un_wi                     : 1;  /**< Enables SLI_INT_SUM[23] to generate an
                                                         interrupt on the RSL. */
	uint64_t m2_un_b0                     : 1;  /**< Enables SLI_INT_SUM[22] to generate an
                                                         interrupt on the RSL. */
	uint64_t m2_up_wi                     : 1;  /**< Enables SLI_INT_SUM[21] to generate an
                                                         interrupt on the RSL. */
	uint64_t m2_up_b0                     : 1;  /**< Enables SLI_INT_SUM[20] to generate an
                                                         interrupt on the RSL. */
	uint64_t reserved_18_19               : 2;
	uint64_t mio_int1                     : 1;  /**< Enables SLI_INT_SUM[17] to generate an
                                                         interrupt on the RSL.
                                                         THIS SHOULD NEVER BE SET */
	uint64_t mio_int0                     : 1;  /**< Enables SLI_INT_SUM[16] to generate an
                                                         interrupt on the RSL.
                                                         THIS SHOULD NEVER BE SET */
	uint64_t m1_un_wi                     : 1;  /**< Enables SLI_INT_SUM[15] to generate an
                                                         interrupt on the RSL. */
	uint64_t m1_un_b0                     : 1;  /**< Enables SLI_INT_SUM[14] to generate an
                                                         interrupt on the RSL. */
	uint64_t m1_up_wi                     : 1;  /**< Enables SLI_INT_SUM[13] to generate an
                                                         interrupt on the RSL. */
	uint64_t m1_up_b0                     : 1;  /**< Enables SLI_INT_SUM[12] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_un_wi                     : 1;  /**< Enables SLI_INT_SUM[11] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_un_b0                     : 1;  /**< Enables SLI_INT_SUM[10] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_up_wi                     : 1;  /**< Enables SLI_INT_SUM[9] to generate an
                                                         interrupt on the RSL. */
	uint64_t m0_up_b0                     : 1;  /**< Enables SLI_INT_SUM[8] to generate an
                                                         interrupt on the RSL. */
	uint64_t reserved_6_7                 : 2;
	uint64_t ptime                        : 1;  /**< Enables SLI_INT_SUM[5] to generate an
                                                         interrupt on the RSL. */
	uint64_t pcnt                         : 1;  /**< Enables SLI_INT_SUM[4] to generate an
                                                         interrupt on the RSL. */
	uint64_t iob2big                      : 1;  /**< Enables SLI_INT_SUM[3] to generate an
                                                         interrupt on the RSL. */
	uint64_t bar0_to                      : 1;  /**< Enables SLI_INT_SUM[2] to generate an
                                                         interrupt on the RSL. */
	uint64_t reserved_1_1                 : 1;
	uint64_t rml_to                       : 1;  /**< Enables SLI_INT_SUM[0] to generate an
                                                         interrupt on the RSL. */
#else
	uint64_t rml_to                       : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t bar0_to                      : 1;
	uint64_t iob2big                      : 1;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t reserved_6_7                 : 2;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t mio_int0                     : 1;
	uint64_t mio_int1                     : 1;
	uint64_t reserved_18_19               : 2;
	uint64_t m2_up_b0                     : 1;
	uint64_t m2_up_wi                     : 1;
	uint64_t m2_un_b0                     : 1;
	uint64_t m2_un_wi                     : 1;
	uint64_t m3_up_b0                     : 1;
	uint64_t m3_up_wi                     : 1;
	uint64_t m3_un_b0                     : 1;
	uint64_t m3_un_wi                     : 1;
	uint64_t mio_int2                     : 1;
	uint64_t reserved_29_31               : 3;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_38_47               : 10;
	uint64_t pidbof                       : 1;
	uint64_t psldbof                      : 1;
	uint64_t pout_err                     : 1;
	uint64_t pin_bp                       : 1;
	uint64_t pgl_err                      : 1;
	uint64_t pdi_err                      : 1;
	uint64_t pop_err                      : 1;
	uint64_t pins_err                     : 1;
	uint64_t sprt0_err                    : 1;
	uint64_t sprt1_err                    : 1;
	uint64_t sprt2_err                    : 1;
	uint64_t sprt3_err                    : 1;
	uint64_t ill_pad                      : 1;
	uint64_t reserved_61_63               : 3;
#endif
	} cn70xx;
	struct cvmx_sli_int_enb_ciu_cn61xx    cnf71xx;
};
typedef union cvmx_sli_int_enb_ciu cvmx_sli_int_enb_ciu_t;

/**
 * cvmx_sli_int_enb_port#
 *
 * When a field in this register is set, and a corresponding interrupt condition asserts in
 * SLI_INT_SUM, an interrupt is generated. Interrupts can be sent to PCIe0 or PCIe1.
 */
union cvmx_sli_int_enb_portx {
	uint64_t u64;
	struct cvmx_sli_int_enb_portx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_62_63               : 2;
	uint64_t pipe_err                     : 1;  /**< Out of range PIPE value. */
	uint64_t ill_pad                      : 1;  /**< Illegal packet csr address. */
	uint64_t sprt3_err                    : 1;  /**< Error Response received on SLI port 3. */
	uint64_t sprt2_err                    : 1;  /**< Error Response received on SLI port 2. */
	uint64_t sprt1_err                    : 1;  /**< Error Response received on SLI port 1. */
	uint64_t sprt0_err                    : 1;  /**< Error Response received on SLI port 0. */
	uint64_t pins_err                     : 1;  /**< Read Error during packet instruction fetch. */
	uint64_t pop_err                      : 1;  /**< Read Error during packet scatter pointer fetch. */
	uint64_t pdi_err                      : 1;  /**< Read Error during packet data fetch. */
	uint64_t pgl_err                      : 1;  /**< Read Error during gather list fetch. */
	uint64_t pin_bp                       : 1;  /**< Packet Input Count exceeded WMARK. */
	uint64_t pout_err                     : 1;  /**< Packet Out Interrupt, Error From PKO. */
	uint64_t psldbof                      : 1;  /**< Packet Scatterlist Doorbell Count Overflow. */
	uint64_t pidbof                       : 1;  /**< Packet Instruction Doorbell Count Overflow. */
	uint64_t reserved_38_47               : 10;
	uint64_t dtime                        : 2;  /**< DMA Timer Interrupts */
	uint64_t dcnt                         : 2;  /**< DMA Count Interrupts */
	uint64_t dmafi                        : 2;  /**< DMA set Forced Interrupts */
	uint64_t reserved_30_31               : 2;
	uint64_t mac2_int                     : 1;  /**< Enables SLI_INT_SUM[29] to generate an
                                                         interrupt to the PCIE-Port2 for MSI/inta.
                                                         SLI_INT_ENB_PORT2[MAC0_INT] sould NEVER be set.
                                                         SLI_INT_ENB_PORT2[MAC1_INT] sould NEVER be set. */
	uint64_t reserved_28_28               : 1;
	uint64_t m3_un_wi                     : 1;  /**< Reserved. */
	uint64_t m3_un_b0                     : 1;  /**< Reserved. */
	uint64_t m3_up_wi                     : 1;  /**< Reserved. */
	uint64_t m3_up_b0                     : 1;  /**< Reserved. */
	uint64_t m2_un_wi                     : 1;  /**< Reserved. */
	uint64_t m2_un_b0                     : 1;  /**< Reserved. */
	uint64_t m2_up_wi                     : 1;  /**< Reserved. */
	uint64_t m2_up_b0                     : 1;  /**< Reserved. */
	uint64_t mac1_int                     : 1;  /**< Enables SLI_INT_SUM[19] to generate an
                                                         interrupt to the PCIE-Port1 for MSI/inta.
                                                         The valuse of this bit has NO effect on PCIE Port0.
                                                         SLI_INT_ENB_PORT0[MAC1_INT] sould NEVER be set. */
	uint64_t mac0_int                     : 1;  /**< Enables SLI_INT_SUM[18] to generate an
                                                         interrupt to the PCIE-Port0 for MSI/inta.
                                                         The valus of this bit has NO effect on PCIE Port1.
                                                         SLI_INT_ENB_PORT1[MAC0_INT] sould NEVER be set. */
	uint64_t mio_int1                     : 1;  /**< Enables SLI_INT_SUM[17] to generate an
                                                         interrupt to the PCIE core for MSI/inta.
                                                         SLI_INT_ENB_PORT0[MIO_INT1] should NEVER be set. */
	uint64_t mio_int0                     : 1;  /**< Enables SLI_INT_SUM[16] to generate an
                                                         interrupt to the PCIE core for MSI/inta.
                                                         SLI_INT_ENB_PORT1[MIO_INT0] should NEVER be set. */
	uint64_t m1_un_wi                     : 1;  /**< Enables SLI_INT_SUM[15] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m1_un_b0                     : 1;  /**< Enables SLI_INT_SUM[14] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m1_up_wi                     : 1;  /**< Enables SLI_INT_SUM[13] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m1_up_b0                     : 1;  /**< Enables SLI_INT_SUM[12] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_un_wi                     : 1;  /**< Enables SLI_INT_SUM[11] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_un_b0                     : 1;  /**< Enables SLI_INT_SUM[10] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_up_wi                     : 1;  /**< Enables SLI_INT_SUM[9] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_up_b0                     : 1;  /**< Enables SLI_INT_SUM[8] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t mio_int3                     : 1;  /**< Enables SLI_INT_SUM[MIO_INT3] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t reserved_6_6                 : 1;
	uint64_t ptime                        : 1;  /**< Enables SLI_INT_SUM[5] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t pcnt                         : 1;  /**< Enables SLI_INT_SUM[4] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t iob2big                      : 1;  /**< Enables SLI_INT_SUM[3] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t bar0_to                      : 1;  /**< Enables SLI_INT_SUM[2] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t reserved_1_1                 : 1;
	uint64_t rml_to                       : 1;  /**< Enables SLI_INT_SUM[0] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
#else
	uint64_t rml_to                       : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t bar0_to                      : 1;
	uint64_t iob2big                      : 1;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t reserved_6_6                 : 1;
	uint64_t mio_int3                     : 1;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t mio_int0                     : 1;
	uint64_t mio_int1                     : 1;
	uint64_t mac0_int                     : 1;
	uint64_t mac1_int                     : 1;
	uint64_t m2_up_b0                     : 1;
	uint64_t m2_up_wi                     : 1;
	uint64_t m2_un_b0                     : 1;
	uint64_t m2_un_wi                     : 1;
	uint64_t m3_up_b0                     : 1;
	uint64_t m3_up_wi                     : 1;
	uint64_t m3_un_b0                     : 1;
	uint64_t m3_un_wi                     : 1;
	uint64_t reserved_28_28               : 1;
	uint64_t mac2_int                     : 1;
	uint64_t reserved_30_31               : 2;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_38_47               : 10;
	uint64_t pidbof                       : 1;
	uint64_t psldbof                      : 1;
	uint64_t pout_err                     : 1;
	uint64_t pin_bp                       : 1;
	uint64_t pgl_err                      : 1;
	uint64_t pdi_err                      : 1;
	uint64_t pop_err                      : 1;
	uint64_t pins_err                     : 1;
	uint64_t sprt0_err                    : 1;
	uint64_t sprt1_err                    : 1;
	uint64_t sprt2_err                    : 1;
	uint64_t sprt3_err                    : 1;
	uint64_t ill_pad                      : 1;
	uint64_t pipe_err                     : 1;
	uint64_t reserved_62_63               : 2;
#endif
	} s;
	struct cvmx_sli_int_enb_portx_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_61_63               : 3;
	uint64_t ill_pad                      : 1;  /**< Illegal packet csr address. */
	uint64_t sprt3_err                    : 1;  /**< Error Response received on SLI port 3. */
	uint64_t sprt2_err                    : 1;  /**< Error Response received on SLI port 2. */
	uint64_t sprt1_err                    : 1;  /**< Error Response received on SLI port 1. */
	uint64_t sprt0_err                    : 1;  /**< Error Response received on SLI port 0. */
	uint64_t pins_err                     : 1;  /**< Read Error during packet instruction fetch. */
	uint64_t pop_err                      : 1;  /**< Read Error during packet scatter pointer fetch. */
	uint64_t pdi_err                      : 1;  /**< Read Error during packet data fetch. */
	uint64_t pgl_err                      : 1;  /**< Read Error during gather list fetch. */
	uint64_t pin_bp                       : 1;  /**< Packet Input Count exceeded WMARK. */
	uint64_t pout_err                     : 1;  /**< Packet Out Interrupt, Error From PKO. */
	uint64_t psldbof                      : 1;  /**< Packet Scatterlist Doorbell Count Overflow. */
	uint64_t pidbof                       : 1;  /**< Packet Instruction Doorbell Count Overflow. */
	uint64_t reserved_38_47               : 10;
	uint64_t dtime                        : 2;  /**< DMA Timer Interrupts */
	uint64_t dcnt                         : 2;  /**< DMA Count Interrupts */
	uint64_t dmafi                        : 2;  /**< DMA set Forced Interrupts */
	uint64_t reserved_28_31               : 4;
	uint64_t m3_un_wi                     : 1;  /**< Reserved. */
	uint64_t m3_un_b0                     : 1;  /**< Reserved. */
	uint64_t m3_up_wi                     : 1;  /**< Reserved. */
	uint64_t m3_up_b0                     : 1;  /**< Reserved. */
	uint64_t m2_un_wi                     : 1;  /**< Reserved. */
	uint64_t m2_un_b0                     : 1;  /**< Reserved. */
	uint64_t m2_up_wi                     : 1;  /**< Reserved. */
	uint64_t m2_up_b0                     : 1;  /**< Reserved. */
	uint64_t mac1_int                     : 1;  /**< Enables SLI_INT_SUM[19] to generate an
                                                         interrupt to the PCIE-Port1 for MSI/inta.
                                                         The valuse of this bit has NO effect on PCIE Port0.
                                                         SLI_INT_ENB_PORT0[MAC1_INT] sould NEVER be set. */
	uint64_t mac0_int                     : 1;  /**< Enables SLI_INT_SUM[18] to generate an
                                                         interrupt to the PCIE-Port0 for MSI/inta.
                                                         The valus of this bit has NO effect on PCIE Port1.
                                                         SLI_INT_ENB_PORT1[MAC0_INT] sould NEVER be set. */
	uint64_t mio_int1                     : 1;  /**< Enables SLI_INT_SUM[17] to generate an
                                                         interrupt to the PCIE core for MSI/inta.
                                                         SLI_INT_ENB_PORT0[MIO_INT1] should NEVER be set. */
	uint64_t mio_int0                     : 1;  /**< Enables SLI_INT_SUM[16] to generate an
                                                         interrupt to the PCIE core for MSI/inta.
                                                         SLI_INT_ENB_PORT1[MIO_INT0] should NEVER be set. */
	uint64_t m1_un_wi                     : 1;  /**< Enables SLI_INT_SUM[15] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m1_un_b0                     : 1;  /**< Enables SLI_INT_SUM[14] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m1_up_wi                     : 1;  /**< Enables SLI_INT_SUM[13] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m1_up_b0                     : 1;  /**< Enables SLI_INT_SUM[12] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_un_wi                     : 1;  /**< Enables SLI_INT_SUM[11] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_un_b0                     : 1;  /**< Enables SLI_INT_SUM[10] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_up_wi                     : 1;  /**< Enables SLI_INT_SUM[9] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_up_b0                     : 1;  /**< Enables SLI_INT_SUM[8] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t reserved_6_7                 : 2;
	uint64_t ptime                        : 1;  /**< Enables SLI_INT_SUM[5] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t pcnt                         : 1;  /**< Enables SLI_INT_SUM[4] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t iob2big                      : 1;  /**< Enables SLI_INT_SUM[3] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t bar0_to                      : 1;  /**< Enables SLI_INT_SUM[2] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t reserved_1_1                 : 1;
	uint64_t rml_to                       : 1;  /**< Enables SLI_INT_SUM[0] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
#else
	uint64_t rml_to                       : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t bar0_to                      : 1;
	uint64_t iob2big                      : 1;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t reserved_6_7                 : 2;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t mio_int0                     : 1;
	uint64_t mio_int1                     : 1;
	uint64_t mac0_int                     : 1;
	uint64_t mac1_int                     : 1;
	uint64_t m2_up_b0                     : 1;
	uint64_t m2_up_wi                     : 1;
	uint64_t m2_un_b0                     : 1;
	uint64_t m2_un_wi                     : 1;
	uint64_t m3_up_b0                     : 1;
	uint64_t m3_up_wi                     : 1;
	uint64_t m3_un_b0                     : 1;
	uint64_t m3_un_wi                     : 1;
	uint64_t reserved_28_31               : 4;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_38_47               : 10;
	uint64_t pidbof                       : 1;
	uint64_t psldbof                      : 1;
	uint64_t pout_err                     : 1;
	uint64_t pin_bp                       : 1;
	uint64_t pgl_err                      : 1;
	uint64_t pdi_err                      : 1;
	uint64_t pop_err                      : 1;
	uint64_t pins_err                     : 1;
	uint64_t sprt0_err                    : 1;
	uint64_t sprt1_err                    : 1;
	uint64_t sprt2_err                    : 1;
	uint64_t sprt3_err                    : 1;
	uint64_t ill_pad                      : 1;
	uint64_t reserved_61_63               : 3;
#endif
	} cn61xx;
	struct cvmx_sli_int_enb_portx_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_61_63               : 3;
	uint64_t ill_pad                      : 1;  /**< Illegal packet csr address. */
	uint64_t reserved_58_59               : 2;
	uint64_t sprt1_err                    : 1;  /**< Error Response received on SLI port 1. */
	uint64_t sprt0_err                    : 1;  /**< Error Response received on SLI port 0. */
	uint64_t pins_err                     : 1;  /**< Read Error during packet instruction fetch. */
	uint64_t pop_err                      : 1;  /**< Read Error during packet scatter pointer fetch. */
	uint64_t pdi_err                      : 1;  /**< Read Error during packet data fetch. */
	uint64_t pgl_err                      : 1;  /**< Read Error during gather list fetch. */
	uint64_t pin_bp                       : 1;  /**< Packet Input Count exceeded WMARK. */
	uint64_t pout_err                     : 1;  /**< Packet Out Interrupt, Error From PKO. */
	uint64_t psldbof                      : 1;  /**< Packet Scatterlist Doorbell Count Overflow. */
	uint64_t pidbof                       : 1;  /**< Packet Instruction Doorbell Count Overflow. */
	uint64_t reserved_38_47               : 10;
	uint64_t dtime                        : 2;  /**< DMA Timer Interrupts */
	uint64_t dcnt                         : 2;  /**< DMA Count Interrupts */
	uint64_t dmafi                        : 2;  /**< DMA set Forced Interrupts */
	uint64_t reserved_20_31               : 12;
	uint64_t mac1_int                     : 1;  /**< Enables SLI_INT_SUM[19] to generate an
                                                         interrupt to the PCIE-Port1 for MSI/inta.
                                                         The valuse of this bit has NO effect on PCIE Port0.
                                                         SLI_INT_ENB_PORT0[MAC1_INT] sould NEVER be set. */
	uint64_t mac0_int                     : 1;  /**< Enables SLI_INT_SUM[18] to generate an
                                                         interrupt to the PCIE-Port0 for MSI/inta.
                                                         The valus of this bit has NO effect on PCIE Port1.
                                                         SLI_INT_ENB_PORT1[MAC0_INT] sould NEVER be set. */
	uint64_t mio_int1                     : 1;  /**< Enables SLI_INT_SUM[17] to generate an
                                                         interrupt to the PCIE core for MSI/inta.
                                                         SLI_INT_ENB_PORT0[MIO_INT1] should NEVER be set. */
	uint64_t mio_int0                     : 1;  /**< Enables SLI_INT_SUM[16] to generate an
                                                         interrupt to the PCIE core for MSI/inta.
                                                         SLI_INT_ENB_PORT1[MIO_INT0] should NEVER be set. */
	uint64_t m1_un_wi                     : 1;  /**< Enables SLI_INT_SUM[15] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m1_un_b0                     : 1;  /**< Enables SLI_INT_SUM[14] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m1_up_wi                     : 1;  /**< Enables SLI_INT_SUM[13] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m1_up_b0                     : 1;  /**< Enables SLI_INT_SUM[12] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_un_wi                     : 1;  /**< Enables SLI_INT_SUM[11] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_un_b0                     : 1;  /**< Enables SLI_INT_SUM[10] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_up_wi                     : 1;  /**< Enables SLI_INT_SUM[9] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_up_b0                     : 1;  /**< Enables SLI_INT_SUM[8] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t reserved_6_7                 : 2;
	uint64_t ptime                        : 1;  /**< Enables SLI_INT_SUM[5] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t pcnt                         : 1;  /**< Enables SLI_INT_SUM[4] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t iob2big                      : 1;  /**< Enables SLI_INT_SUM[3] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t bar0_to                      : 1;  /**< Enables SLI_INT_SUM[2] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t reserved_1_1                 : 1;
	uint64_t rml_to                       : 1;  /**< Enables SLI_INT_SUM[0] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
#else
	uint64_t rml_to                       : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t bar0_to                      : 1;
	uint64_t iob2big                      : 1;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t reserved_6_7                 : 2;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t mio_int0                     : 1;
	uint64_t mio_int1                     : 1;
	uint64_t mac0_int                     : 1;
	uint64_t mac1_int                     : 1;
	uint64_t reserved_20_31               : 12;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_38_47               : 10;
	uint64_t pidbof                       : 1;
	uint64_t psldbof                      : 1;
	uint64_t pout_err                     : 1;
	uint64_t pin_bp                       : 1;
	uint64_t pgl_err                      : 1;
	uint64_t pdi_err                      : 1;
	uint64_t pop_err                      : 1;
	uint64_t pins_err                     : 1;
	uint64_t sprt0_err                    : 1;
	uint64_t sprt1_err                    : 1;
	uint64_t reserved_58_59               : 2;
	uint64_t ill_pad                      : 1;
	uint64_t reserved_61_63               : 3;
#endif
	} cn63xx;
	struct cvmx_sli_int_enb_portx_cn63xx  cn63xxp1;
	struct cvmx_sli_int_enb_portx_cn61xx  cn66xx;
	struct cvmx_sli_int_enb_portx_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_62_63               : 2;
	uint64_t pipe_err                     : 1;  /**< Out of range PIPE value. */
	uint64_t ill_pad                      : 1;  /**< Illegal packet csr address. */
	uint64_t reserved_58_59               : 2;
	uint64_t sprt1_err                    : 1;  /**< Error Response received on SLI port 1. */
	uint64_t sprt0_err                    : 1;  /**< Error Response received on SLI port 0. */
	uint64_t pins_err                     : 1;  /**< Read Error during packet instruction fetch. */
	uint64_t pop_err                      : 1;  /**< Read Error during packet scatter pointer fetch. */
	uint64_t pdi_err                      : 1;  /**< Read Error during packet data fetch. */
	uint64_t pgl_err                      : 1;  /**< Read Error during gather list fetch. */
	uint64_t reserved_51_51               : 1;
	uint64_t pout_err                     : 1;  /**< Packet Out Interrupt, Error From PKO. */
	uint64_t psldbof                      : 1;  /**< Packet Scatterlist Doorbell Count Overflow. */
	uint64_t pidbof                       : 1;  /**< Packet Instruction Doorbell Count Overflow. */
	uint64_t reserved_38_47               : 10;
	uint64_t dtime                        : 2;  /**< DMA Timer Interrupts */
	uint64_t dcnt                         : 2;  /**< DMA Count Interrupts */
	uint64_t dmafi                        : 2;  /**< DMA set Forced Interrupts */
	uint64_t reserved_20_31               : 12;
	uint64_t mac1_int                     : 1;  /**< Enables SLI_INT_SUM[19] to generate an
                                                         interrupt to the PCIE-Port1 for MSI/inta.
                                                         The valuse of this bit has NO effect on PCIE Port0.
                                                         SLI_INT_ENB_PORT0[MAC1_INT] sould NEVER be set. */
	uint64_t mac0_int                     : 1;  /**< Enables SLI_INT_SUM[18] to generate an
                                                         interrupt to the PCIE-Port0 for MSI/inta.
                                                         The valus of this bit has NO effect on PCIE Port1.
                                                         SLI_INT_ENB_PORT1[MAC0_INT] sould NEVER be set. */
	uint64_t mio_int1                     : 1;  /**< Enables SLI_INT_SUM[17] to generate an
                                                         interrupt to the PCIE core for MSI/inta.
                                                         SLI_INT_ENB_PORT0[MIO_INT1] should NEVER be set. */
	uint64_t mio_int0                     : 1;  /**< Enables SLI_INT_SUM[16] to generate an
                                                         interrupt to the PCIE core for MSI/inta.
                                                         SLI_INT_ENB_PORT1[MIO_INT0] should NEVER be set. */
	uint64_t m1_un_wi                     : 1;  /**< Enables SLI_INT_SUM[15] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m1_un_b0                     : 1;  /**< Enables SLI_INT_SUM[14] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m1_up_wi                     : 1;  /**< Enables SLI_INT_SUM[13] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m1_up_b0                     : 1;  /**< Enables SLI_INT_SUM[12] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_un_wi                     : 1;  /**< Enables SLI_INT_SUM[11] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_un_b0                     : 1;  /**< Enables SLI_INT_SUM[10] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_up_wi                     : 1;  /**< Enables SLI_INT_SUM[9] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_up_b0                     : 1;  /**< Enables SLI_INT_SUM[8] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t reserved_6_7                 : 2;
	uint64_t ptime                        : 1;  /**< Enables SLI_INT_SUM[5] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t pcnt                         : 1;  /**< Enables SLI_INT_SUM[4] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t iob2big                      : 1;  /**< Enables SLI_INT_SUM[3] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t bar0_to                      : 1;  /**< Enables SLI_INT_SUM[2] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t reserved_1_1                 : 1;
	uint64_t rml_to                       : 1;  /**< Enables SLI_INT_SUM[0] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
#else
	uint64_t rml_to                       : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t bar0_to                      : 1;
	uint64_t iob2big                      : 1;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t reserved_6_7                 : 2;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t mio_int0                     : 1;
	uint64_t mio_int1                     : 1;
	uint64_t mac0_int                     : 1;
	uint64_t mac1_int                     : 1;
	uint64_t reserved_20_31               : 12;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_38_47               : 10;
	uint64_t pidbof                       : 1;
	uint64_t psldbof                      : 1;
	uint64_t pout_err                     : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t pgl_err                      : 1;
	uint64_t pdi_err                      : 1;
	uint64_t pop_err                      : 1;
	uint64_t pins_err                     : 1;
	uint64_t sprt0_err                    : 1;
	uint64_t sprt1_err                    : 1;
	uint64_t reserved_58_59               : 2;
	uint64_t ill_pad                      : 1;
	uint64_t pipe_err                     : 1;
	uint64_t reserved_62_63               : 2;
#endif
	} cn68xx;
	struct cvmx_sli_int_enb_portx_cn68xx  cn68xxp1;
	struct cvmx_sli_int_enb_portx_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_61_63               : 3;
	uint64_t ill_pad                      : 1;  /**< Illegal packet csr address. */
	uint64_t sprt3_err                    : 1;  /**< Error Response received on SLI port 3. */
	uint64_t sprt2_err                    : 1;  /**< Error Response received on SLI port 2. */
	uint64_t sprt1_err                    : 1;  /**< Error Response received on SLI port 1. */
	uint64_t sprt0_err                    : 1;  /**< Error Response received on SLI port 0. */
	uint64_t pins_err                     : 1;  /**< Read Error during packet instruction fetch. */
	uint64_t pop_err                      : 1;  /**< Read Error during packet scatter pointer fetch. */
	uint64_t pdi_err                      : 1;  /**< Read Error during packet data fetch. */
	uint64_t pgl_err                      : 1;  /**< Read Error during gather list fetch. */
	uint64_t pin_bp                       : 1;  /**< Packet Input Count exceeded WMARK. */
	uint64_t pout_err                     : 1;  /**< Packet Out Interrupt, Error From PKO. */
	uint64_t psldbof                      : 1;  /**< Packet Scatterlist Doorbell Count Overflow. */
	uint64_t pidbof                       : 1;  /**< Packet Instruction Doorbell Count Overflow. */
	uint64_t reserved_38_47               : 10;
	uint64_t dtime                        : 2;  /**< DMA Timer Interrupts */
	uint64_t dcnt                         : 2;  /**< DMA Count Interrupts */
	uint64_t dmafi                        : 2;  /**< DMA set Forced Interrupts */
	uint64_t reserved_30_31               : 2;
	uint64_t mac2_int                     : 1;  /**< Enables SLI_INT_SUM[29] to generate an
                                                         interrupt to the PCIE-Port2 for MSI/inta.
                                                         SLI_INT_ENB_PORT2[MAC0_INT] sould NEVER be set.
                                                         SLI_INT_ENB_PORT2[MAC1_INT] sould NEVER be set. */
	uint64_t mio_int2                     : 1;  /**< Enables SLI_INT_SUM[28] to generate an
                                                         interrupt to the PCIE core for MSI/inta.
                                                         SLI_INT_ENB_PORT2[MIO_INT2] should NEVER be set. */
	uint64_t m3_un_wi                     : 1;  /**< Reserved. */
	uint64_t m3_un_b0                     : 1;  /**< Reserved. */
	uint64_t m3_up_wi                     : 1;  /**< Reserved. */
	uint64_t m3_up_b0                     : 1;  /**< Reserved. */
	uint64_t m2_un_wi                     : 1;  /**< Enables SLI_INT_SUM[23] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m2_un_b0                     : 1;  /**< Enables SLI_INT_SUM[22] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m2_up_wi                     : 1;  /**< Enables SLI_INT_SUM[21] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m2_up_b0                     : 1;  /**< Enables SLI_INT_SUM[20] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t mac1_int                     : 1;  /**< Enables SLI_INT_SUM[19] to generate an
                                                         interrupt to the PCIE-Port1 for MSI/inta.
                                                         The valuse of this bit has NO effect on PCIE Port0.
                                                         SLI_INT_ENB_PORT0[MAC1_INT] sould NEVER be set. */
	uint64_t mac0_int                     : 1;  /**< Enables SLI_INT_SUM[18] to generate an
                                                         interrupt to the PCIE-Port0 for MSI/inta.
                                                         The valus of this bit has NO effect on PCIE Port1.
                                                         SLI_INT_ENB_PORT1[MAC0_INT] sould NEVER be set. */
	uint64_t mio_int1                     : 1;  /**< Enables SLI_INT_SUM[17] to generate an
                                                         interrupt to the PCIE core for MSI/inta.
                                                         SLI_INT_ENB_PORT0[MIO_INT1] should NEVER be set. */
	uint64_t mio_int0                     : 1;  /**< Enables SLI_INT_SUM[16] to generate an
                                                         interrupt to the PCIE core for MSI/inta.
                                                         SLI_INT_ENB_PORT1[MIO_INT0] should NEVER be set. */
	uint64_t m1_un_wi                     : 1;  /**< Enables SLI_INT_SUM[15] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m1_un_b0                     : 1;  /**< Enables SLI_INT_SUM[14] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m1_up_wi                     : 1;  /**< Enables SLI_INT_SUM[13] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m1_up_b0                     : 1;  /**< Enables SLI_INT_SUM[12] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_un_wi                     : 1;  /**< Enables SLI_INT_SUM[11] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_un_b0                     : 1;  /**< Enables SLI_INT_SUM[10] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_up_wi                     : 1;  /**< Enables SLI_INT_SUM[9] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t m0_up_b0                     : 1;  /**< Enables SLI_INT_SUM[8] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t reserved_6_7                 : 2;
	uint64_t ptime                        : 1;  /**< Enables SLI_INT_SUM[5] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t pcnt                         : 1;  /**< Enables SLI_INT_SUM[4] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t iob2big                      : 1;  /**< Enables SLI_INT_SUM[3] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t bar0_to                      : 1;  /**< Enables SLI_INT_SUM[2] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t reserved_1_1                 : 1;
	uint64_t rml_to                       : 1;  /**< Enables SLI_INT_SUM[0] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
#else
	uint64_t rml_to                       : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t bar0_to                      : 1;
	uint64_t iob2big                      : 1;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t reserved_6_7                 : 2;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t mio_int0                     : 1;
	uint64_t mio_int1                     : 1;
	uint64_t mac0_int                     : 1;
	uint64_t mac1_int                     : 1;
	uint64_t m2_up_b0                     : 1;
	uint64_t m2_up_wi                     : 1;
	uint64_t m2_un_b0                     : 1;
	uint64_t m2_un_wi                     : 1;
	uint64_t m3_up_b0                     : 1;
	uint64_t m3_up_wi                     : 1;
	uint64_t m3_un_b0                     : 1;
	uint64_t m3_un_wi                     : 1;
	uint64_t mio_int2                     : 1;
	uint64_t mac2_int                     : 1;
	uint64_t reserved_30_31               : 2;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_38_47               : 10;
	uint64_t pidbof                       : 1;
	uint64_t psldbof                      : 1;
	uint64_t pout_err                     : 1;
	uint64_t pin_bp                       : 1;
	uint64_t pgl_err                      : 1;
	uint64_t pdi_err                      : 1;
	uint64_t pop_err                      : 1;
	uint64_t pins_err                     : 1;
	uint64_t sprt0_err                    : 1;
	uint64_t sprt1_err                    : 1;
	uint64_t sprt2_err                    : 1;
	uint64_t sprt3_err                    : 1;
	uint64_t ill_pad                      : 1;
	uint64_t reserved_61_63               : 3;
#endif
	} cn70xx;
	struct cvmx_sli_int_enb_portx_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t sprt3_err                    : 1;  /**< Enables SLI_INT_SUM[SPRT3_ERR] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t sprt2_err                    : 1;  /**< Enables SLI_INT_SUM[SPRT2_ERR] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t sprt1_err                    : 1;  /**< Enables SLI_INT_SUM[SPRT1_ERR] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t sprt0_err                    : 1;  /**< Enables SLI_INT_SUM[SPRT0_ERR] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t pins_err                     : 1;  /**< Enables SLI_INT_SUM[PINS_ERR] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t pop_err                      : 1;  /**< Enables SLI_INT_SUM[POP_ERR] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t pdi_err                      : 1;  /**< Enables SLI_INT_SUM[PDI_ERR] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t pgl_err                      : 1;  /**< Enables SLI_INT_SUM[PGL_ERR] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t reserved_50_51               : 2;
	uint64_t psldbof                      : 1;  /**< Enables SLI_INT_SUM[PSLDBOF] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t pidbof                       : 1;  /**< Enables SLI_INT_SUM[PIDBOF] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t reserved_38_47               : 10;
	uint64_t dtime                        : 2;  /**< Enables SLI_INT_SUM[DTIME] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t dcnt                         : 2;  /**< Enables SLI_INT_SUM[DCNT] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t dmafi                        : 2;  /**< Enables SLI_INT_SUM[DMAFI] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t reserved_29_31               : 3;
	uint64_t vf_err                       : 1;  /**< Illegal access from VF */
	uint64_t m3_un_wi                     : 1;  /**< Enables SLI_INT_SUM[M3_UN_WI] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t m3_un_b0                     : 1;  /**< Enables SLI_INT_SUM[M3_UN_B0] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t m3_up_wi                     : 1;  /**< Enables SLI_INT_SUM[M3_UP_WI] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t m3_up_b0                     : 1;  /**< Enables SLI_INT_SUM[M3_UP_B0] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t m2_un_wi                     : 1;  /**< Enables SLI_INT_SUM[M2_UN_WI] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t m2_un_b0                     : 1;  /**< Enables SLI_INT_SUM[M2_UN_B0] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t m2_up_wi                     : 1;  /**< Enables SLI_INT_SUM[M2_UP_WI] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t m2_up_b0                     : 1;  /**< Enables SLI_INT_SUM[M2_UP_B0] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t reserved_18_19               : 2;
	uint64_t mio_int1                     : 1;  /**< Enables SLI_INT_SUM[MIO_INT1] to generate an interrupt to the MAC core for MSI/INTA.
                                                         SLI_INT_ENB_PORT0[MIO_INT1] should never be set. */
	uint64_t mio_int0                     : 1;  /**< Enables SLI_INT_SUM[MIO_INT0] to generate an interrupt to the MAC core for MSI/INTA.
                                                         SLI_INT_ENB_PORT1[MIO_INT0] should never be set. */
	uint64_t m1_un_wi                     : 1;  /**< Enables SLI_INT_SUM[M1_UN_WI] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t m1_un_b0                     : 1;  /**< Enables SLI_INT_SUM[M1_UN_B0] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t m1_up_wi                     : 1;  /**< Enables SLI_INT_SUM[M1_UP_WI] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t m1_up_b0                     : 1;  /**< Enables SLI_INT_SUM[M1_UP_B0] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t m0_un_wi                     : 1;  /**< Enables SLI_INT_SUM[M0_UN_WI] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t m0_un_b0                     : 1;  /**< Enables SLI_INT_SUM[M0_UN_B0] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t m0_up_wi                     : 1;  /**< Enables SLI_INT_SUM[M0_UP_WI] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t m0_up_b0                     : 1;  /**< Enables SLI_INT_SUM[M0_UP_B0] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t mio_int3                     : 1;  /**< Enables SLI_INT_SUM[MIO_INT3] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t mio_int2                     : 1;  /**< Enables SLI_INT_SUM[MIO_INT2] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t ptime                        : 1;  /**< Enables SLI_INT_SUM[PTIME] to generate an interrupt to the MAC core for MSI/INTA. */
	uint64_t pcnt                         : 1;  /**< Enables SLI_INT_SUM[4] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
	uint64_t reserved_1_3                 : 3;
	uint64_t rml_to                       : 1;  /**< Enables SLI_INT_SUM[0] to generate an
                                                         interrupt to the PCIE core for MSI/inta. */
#else
	uint64_t rml_to                       : 1;
	uint64_t reserved_1_3                 : 3;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t mio_int2                     : 1;
	uint64_t mio_int3                     : 1;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t mio_int0                     : 1;
	uint64_t mio_int1                     : 1;
	uint64_t reserved_18_19               : 2;
	uint64_t m2_up_b0                     : 1;
	uint64_t m2_up_wi                     : 1;
	uint64_t m2_un_b0                     : 1;
	uint64_t m2_un_wi                     : 1;
	uint64_t m3_up_b0                     : 1;
	uint64_t m3_up_wi                     : 1;
	uint64_t m3_un_b0                     : 1;
	uint64_t m3_un_wi                     : 1;
	uint64_t vf_err                       : 1;
	uint64_t reserved_29_31               : 3;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_38_47               : 10;
	uint64_t pidbof                       : 1;
	uint64_t psldbof                      : 1;
	uint64_t reserved_50_51               : 2;
	uint64_t pgl_err                      : 1;
	uint64_t pdi_err                      : 1;
	uint64_t pop_err                      : 1;
	uint64_t pins_err                     : 1;
	uint64_t sprt0_err                    : 1;
	uint64_t sprt1_err                    : 1;
	uint64_t sprt2_err                    : 1;
	uint64_t sprt3_err                    : 1;
	uint64_t reserved_60_63               : 4;
#endif
	} cn78xx;
	struct cvmx_sli_int_enb_portx_cn61xx  cnf71xx;
};
typedef union cvmx_sli_int_enb_portx cvmx_sli_int_enb_portx_t;

/**
 * cvmx_sli_int_sum
 *
 * The fields in this register are set when an interrupt condition occurs; write 1 to clear. All
 * fields of the CSR are valid when a PF reads the CSR. When read by a VF, only fields PTIME and
 * PCNT are valid.
 */
union cvmx_sli_int_sum {
	uint64_t u64;
	struct cvmx_sli_int_sum_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_62_63               : 2;
	uint64_t pipe_err                     : 1;  /**< Set when a PIPE value outside range is received. */
	uint64_t ill_pad                      : 1;  /**< Set when a BAR0 address R/W falls into theaddress
                                                         range of the Packet-CSR, but for an unused
                                                         address. */
	uint64_t sprt3_err                    : 1;  /**< Reserved. */
	uint64_t sprt2_err                    : 1;  /**< Reserved. */
	uint64_t sprt1_err                    : 1;  /**< When an error response received on SLI port 1
                                                         this bit is set. */
	uint64_t sprt0_err                    : 1;  /**< When an error response received on SLI port 0
                                                         this bit is set. */
	uint64_t pins_err                     : 1;  /**< When a read error occurs on a packet instruction
                                                         this bit is set. */
	uint64_t pop_err                      : 1;  /**< When a read error occurs on a packet scatter
                                                         pointer pair this bit is set. */
	uint64_t pdi_err                      : 1;  /**< When a read error occurs on a packet data read
                                                         this bit is set. */
	uint64_t pgl_err                      : 1;  /**< When a read error occurs on a packet gather list
                                                         read this bit is set. */
	uint64_t pin_bp                       : 1;  /**< Packet input count has exceeded the WMARK.
                                                         See SLI_PKT_IN_BP */
	uint64_t pout_err                     : 1;  /**< Set when PKO sends packet data with the error bit
                                                         set. */
	uint64_t psldbof                      : 1;  /**< Packet Scatterlist Doorbell count overflowed. Which
                                                         doorbell can be found in DPI_PINT_INFO[PSLDBOF] */
	uint64_t pidbof                       : 1;  /**< Packet Instruction Doorbell count overflowed. Which
                                                         doorbell can be found in DPI_PINT_INFO[PIDBOF] */
	uint64_t reserved_38_47               : 10;
	uint64_t dtime                        : 2;  /**< Whenever SLI_DMAx_CNT[CNT] is not 0, the
                                                         SLI_DMAx_TIM[TIM] timer increments every SLI
                                                         clock.
                                                         DTIME[x] is set whenever SLI_DMAx_TIM[TIM] >
                                                         SLI_DMAx_INT_LEVEL[TIME].
                                                         DTIME[x] is normally cleared by clearing
                                                         SLI_DMAx_CNT[CNT] (which also clears
                                                         SLI_DMAx_TIM[TIM]). */
	uint64_t dcnt                         : 2;  /**< DCNT[x] is set whenever SLI_DMAx_CNT[CNT] >
                                                         SLI_DMAx_INT_LEVEL[CNT].
                                                         DCNT[x] is normally cleared by decreasing
                                                         SLI_DMAx_CNT[CNT]. */
	uint64_t dmafi                        : 2;  /**< DMA set Forced Interrupts. */
	uint64_t reserved_30_31               : 2;
	uint64_t mac2_int                     : 1;  /**< Interrupt from MAC2.
                                                         See PEM2_INT_SUM (enabled by PEM2_INT_ENB_INT) */
	uint64_t reserved_28_28               : 1;
	uint64_t m3_un_wi                     : 1;  /**< Reserved. */
	uint64_t m3_un_b0                     : 1;  /**< Reserved. */
	uint64_t m3_up_wi                     : 1;  /**< Reserved. */
	uint64_t m3_up_b0                     : 1;  /**< Reserved. */
	uint64_t m2_un_wi                     : 1;  /**< Reserved. */
	uint64_t m2_un_b0                     : 1;  /**< Reserved. */
	uint64_t m2_up_wi                     : 1;  /**< Reserved. */
	uint64_t m2_up_b0                     : 1;  /**< Reserved. */
	uint64_t mac1_int                     : 1;  /**< Interrupt from MAC1.
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB_INT) */
	uint64_t mac0_int                     : 1;  /**< Interrupt from MAC0.
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB_INT) */
	uint64_t mio_int1                     : 1;  /**< Interrupt from MIO for PORT 1.
                                                         See CIU_INT33_SUM0, CIU_INT_SUM1
                                                         (enabled by CIU_INT33_EN0, CIU_INT33_EN1) */
	uint64_t mio_int0                     : 1;  /**< Interrupt from MIO for PORT 0.
                                                         See CIU_INT32_SUM0, CIU_INT_SUM1
                                                         (enabled by CIU_INT32_EN0, CIU_INT32_EN1) */
	uint64_t m1_un_wi                     : 1;  /**< Received Unsupported N-TLP for Window Register
                                                         from MAC 1. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m1_un_b0                     : 1;  /**< Received Unsupported N-TLP for Bar0 from MAC 1.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t m1_up_wi                     : 1;  /**< Received Unsupported P-TLP for Window Register
                                                         from MAC 1. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m1_up_b0                     : 1;  /**< Received Unsupported P-TLP for Bar0 from MAC 1.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t m0_un_wi                     : 1;  /**< Received Unsupported N-TLP for Window Register
                                                         from MAC 0. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m0_un_b0                     : 1;  /**< Received Unsupported N-TLP for Bar0 from MAC 0.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t m0_up_wi                     : 1;  /**< Received Unsupported P-TLP for Window Register
                                                         from MAC 0. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m0_up_b0                     : 1;  /**< Received Unsupported P-TLP for Bar0 from MAC 0.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t mio_int3                     : 1;  /**< Interrupt from MIO for Port 3. Throws SLI_INTSN_E::SLI_INT_MIO_INT3. */
	uint64_t reserved_6_6                 : 1;
	uint64_t ptime                        : 1;  /**< Packet Timer has an interrupt. Which rings can
                                                         be found in SLI_PKT_TIME_INT. */
	uint64_t pcnt                         : 1;  /**< Packet Counter has an interrupt. Which rings can
                                                         be found in SLI_PKT_CNT_INT. */
	uint64_t iob2big                      : 1;  /**< A requested IOBDMA is to large. */
	uint64_t bar0_to                      : 1;  /**< BAR0 R/W to a NCB device did not receive
                                                         read-data/commit in 0xffff core clocks. */
	uint64_t reserved_1_1                 : 1;
	uint64_t rml_to                       : 1;  /**< A read or write transfer did not complete
                                                         within 0xffff core clocks. */
#else
	uint64_t rml_to                       : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t bar0_to                      : 1;
	uint64_t iob2big                      : 1;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t reserved_6_6                 : 1;
	uint64_t mio_int3                     : 1;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t mio_int0                     : 1;
	uint64_t mio_int1                     : 1;
	uint64_t mac0_int                     : 1;
	uint64_t mac1_int                     : 1;
	uint64_t m2_up_b0                     : 1;
	uint64_t m2_up_wi                     : 1;
	uint64_t m2_un_b0                     : 1;
	uint64_t m2_un_wi                     : 1;
	uint64_t m3_up_b0                     : 1;
	uint64_t m3_up_wi                     : 1;
	uint64_t m3_un_b0                     : 1;
	uint64_t m3_un_wi                     : 1;
	uint64_t reserved_28_28               : 1;
	uint64_t mac2_int                     : 1;
	uint64_t reserved_30_31               : 2;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_38_47               : 10;
	uint64_t pidbof                       : 1;
	uint64_t psldbof                      : 1;
	uint64_t pout_err                     : 1;
	uint64_t pin_bp                       : 1;
	uint64_t pgl_err                      : 1;
	uint64_t pdi_err                      : 1;
	uint64_t pop_err                      : 1;
	uint64_t pins_err                     : 1;
	uint64_t sprt0_err                    : 1;
	uint64_t sprt1_err                    : 1;
	uint64_t sprt2_err                    : 1;
	uint64_t sprt3_err                    : 1;
	uint64_t ill_pad                      : 1;
	uint64_t pipe_err                     : 1;
	uint64_t reserved_62_63               : 2;
#endif
	} s;
	struct cvmx_sli_int_sum_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_61_63               : 3;
	uint64_t ill_pad                      : 1;  /**< Set when a BAR0 address R/W falls into theaddress
                                                         range of the Packet-CSR, but for an unused
                                                         address. */
	uint64_t sprt3_err                    : 1;  /**< Reserved. */
	uint64_t sprt2_err                    : 1;  /**< Reserved. */
	uint64_t sprt1_err                    : 1;  /**< When an error response received on SLI port 1
                                                         this bit is set. */
	uint64_t sprt0_err                    : 1;  /**< When an error response received on SLI port 0
                                                         this bit is set. */
	uint64_t pins_err                     : 1;  /**< When a read error occurs on a packet instruction
                                                         this bit is set. */
	uint64_t pop_err                      : 1;  /**< When a read error occurs on a packet scatter
                                                         pointer pair this bit is set. */
	uint64_t pdi_err                      : 1;  /**< When a read error occurs on a packet data read
                                                         this bit is set. */
	uint64_t pgl_err                      : 1;  /**< When a read error occurs on a packet gather list
                                                         read this bit is set. */
	uint64_t pin_bp                       : 1;  /**< Packet input count has exceeded the WMARK.
                                                         See SLI_PKT_IN_BP */
	uint64_t pout_err                     : 1;  /**< Set when PKO sends packet data with the error bit
                                                         set. */
	uint64_t psldbof                      : 1;  /**< Packet Scatterlist Doorbell count overflowed. Which
                                                         doorbell can be found in DPI_PINT_INFO[PSLDBOF] */
	uint64_t pidbof                       : 1;  /**< Packet Instruction Doorbell count overflowed. Which
                                                         doorbell can be found in DPI_PINT_INFO[PIDBOF] */
	uint64_t reserved_38_47               : 10;
	uint64_t dtime                        : 2;  /**< Whenever SLI_DMAx_CNT[CNT] is not 0, the
                                                         SLI_DMAx_TIM[TIM] timer increments every SLI
                                                         clock.
                                                         DTIME[x] is set whenever SLI_DMAx_TIM[TIM] >
                                                         SLI_DMAx_INT_LEVEL[TIME].
                                                         DTIME[x] is normally cleared by clearing
                                                         SLI_DMAx_CNT[CNT] (which also clears
                                                         SLI_DMAx_TIM[TIM]). */
	uint64_t dcnt                         : 2;  /**< DCNT[x] is set whenever SLI_DMAx_CNT[CNT] >
                                                         SLI_DMAx_INT_LEVEL[CNT].
                                                         DCNT[x] is normally cleared by decreasing
                                                         SLI_DMAx_CNT[CNT]. */
	uint64_t dmafi                        : 2;  /**< DMA set Forced Interrupts. */
	uint64_t reserved_28_31               : 4;
	uint64_t m3_un_wi                     : 1;  /**< Reserved. */
	uint64_t m3_un_b0                     : 1;  /**< Reserved. */
	uint64_t m3_up_wi                     : 1;  /**< Reserved. */
	uint64_t m3_up_b0                     : 1;  /**< Reserved. */
	uint64_t m2_un_wi                     : 1;  /**< Reserved. */
	uint64_t m2_un_b0                     : 1;  /**< Reserved. */
	uint64_t m2_up_wi                     : 1;  /**< Reserved. */
	uint64_t m2_up_b0                     : 1;  /**< Reserved. */
	uint64_t mac1_int                     : 1;  /**< Interrupt from MAC1.
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB_INT) */
	uint64_t mac0_int                     : 1;  /**< Interrupt from MAC0.
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB_INT) */
	uint64_t mio_int1                     : 1;  /**< Interrupt from MIO for PORT 1.
                                                         See CIU_INT33_SUM0, CIU_INT_SUM1
                                                         (enabled by CIU_INT33_EN0, CIU_INT33_EN1) */
	uint64_t mio_int0                     : 1;  /**< Interrupt from MIO for PORT 0.
                                                         See CIU_INT32_SUM0, CIU_INT_SUM1
                                                         (enabled by CIU_INT32_EN0, CIU_INT32_EN1) */
	uint64_t m1_un_wi                     : 1;  /**< Received Unsupported N-TLP for Window Register
                                                         from MAC 1. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m1_un_b0                     : 1;  /**< Received Unsupported N-TLP for Bar0 from MAC 1.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t m1_up_wi                     : 1;  /**< Received Unsupported P-TLP for Window Register
                                                         from MAC 1. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m1_up_b0                     : 1;  /**< Received Unsupported P-TLP for Bar0 from MAC 1.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t m0_un_wi                     : 1;  /**< Received Unsupported N-TLP for Window Register
                                                         from MAC 0. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m0_un_b0                     : 1;  /**< Received Unsupported N-TLP for Bar0 from MAC 0.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t m0_up_wi                     : 1;  /**< Received Unsupported P-TLP for Window Register
                                                         from MAC 0. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m0_up_b0                     : 1;  /**< Received Unsupported P-TLP for Bar0 from MAC 0.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t reserved_6_7                 : 2;
	uint64_t ptime                        : 1;  /**< Packet Timer has an interrupt. Which rings can
                                                         be found in SLI_PKT_TIME_INT. */
	uint64_t pcnt                         : 1;  /**< Packet Counter has an interrupt. Which rings can
                                                         be found in SLI_PKT_CNT_INT. */
	uint64_t iob2big                      : 1;  /**< A requested IOBDMA is to large. */
	uint64_t bar0_to                      : 1;  /**< BAR0 R/W to a NCB device did not receive
                                                         read-data/commit in 0xffff core clocks. */
	uint64_t reserved_1_1                 : 1;
	uint64_t rml_to                       : 1;  /**< A read or write transfer did not complete
                                                         within 0xffff core clocks. */
#else
	uint64_t rml_to                       : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t bar0_to                      : 1;
	uint64_t iob2big                      : 1;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t reserved_6_7                 : 2;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t mio_int0                     : 1;
	uint64_t mio_int1                     : 1;
	uint64_t mac0_int                     : 1;
	uint64_t mac1_int                     : 1;
	uint64_t m2_up_b0                     : 1;
	uint64_t m2_up_wi                     : 1;
	uint64_t m2_un_b0                     : 1;
	uint64_t m2_un_wi                     : 1;
	uint64_t m3_up_b0                     : 1;
	uint64_t m3_up_wi                     : 1;
	uint64_t m3_un_b0                     : 1;
	uint64_t m3_un_wi                     : 1;
	uint64_t reserved_28_31               : 4;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_38_47               : 10;
	uint64_t pidbof                       : 1;
	uint64_t psldbof                      : 1;
	uint64_t pout_err                     : 1;
	uint64_t pin_bp                       : 1;
	uint64_t pgl_err                      : 1;
	uint64_t pdi_err                      : 1;
	uint64_t pop_err                      : 1;
	uint64_t pins_err                     : 1;
	uint64_t sprt0_err                    : 1;
	uint64_t sprt1_err                    : 1;
	uint64_t sprt2_err                    : 1;
	uint64_t sprt3_err                    : 1;
	uint64_t ill_pad                      : 1;
	uint64_t reserved_61_63               : 3;
#endif
	} cn61xx;
	struct cvmx_sli_int_sum_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_61_63               : 3;
	uint64_t ill_pad                      : 1;  /**< Set when a BAR0 address R/W falls into theaddress
                                                         range of the Packet-CSR, but for an unused
                                                         address. */
	uint64_t reserved_58_59               : 2;
	uint64_t sprt1_err                    : 1;  /**< When an error response received on SLI port 1
                                                         this bit is set. */
	uint64_t sprt0_err                    : 1;  /**< When an error response received on SLI port 0
                                                         this bit is set. */
	uint64_t pins_err                     : 1;  /**< When a read error occurs on a packet instruction
                                                         this bit is set. */
	uint64_t pop_err                      : 1;  /**< When a read error occurs on a packet scatter
                                                         pointer pair this bit is set. */
	uint64_t pdi_err                      : 1;  /**< When a read error occurs on a packet data read
                                                         this bit is set. */
	uint64_t pgl_err                      : 1;  /**< When a read error occurs on a packet gather list
                                                         read this bit is set. */
	uint64_t pin_bp                       : 1;  /**< Packet input count has exceeded the WMARK.
                                                         See SLI_PKT_IN_BP */
	uint64_t pout_err                     : 1;  /**< Set when PKO sends packet data with the error bit
                                                         set. */
	uint64_t psldbof                      : 1;  /**< Packet Scatterlist Doorbell count overflowed. Which
                                                         doorbell can be found in DPI_PINT_INFO[PSLDBOF] */
	uint64_t pidbof                       : 1;  /**< Packet Instruction Doorbell count overflowed. Which
                                                         doorbell can be found in DPI_PINT_INFO[PIDBOF] */
	uint64_t reserved_38_47               : 10;
	uint64_t dtime                        : 2;  /**< Whenever SLI_DMAx_CNT[CNT] is not 0, the
                                                         SLI_DMAx_TIM[TIM] timer increments every SLI
                                                         clock.
                                                         DTIME[x] is set whenever SLI_DMAx_TIM[TIM] >
                                                         SLI_DMAx_INT_LEVEL[TIME].
                                                         DTIME[x] is normally cleared by clearing
                                                         SLI_DMAx_CNT[CNT] (which also clears
                                                         SLI_DMAx_TIM[TIM]). */
	uint64_t dcnt                         : 2;  /**< DCNT[x] is set whenever SLI_DMAx_CNT[CNT] >
                                                         SLI_DMAx_INT_LEVEL[CNT].
                                                         DCNT[x] is normally cleared by decreasing
                                                         SLI_DMAx_CNT[CNT]. */
	uint64_t dmafi                        : 2;  /**< DMA set Forced Interrupts. */
	uint64_t reserved_20_31               : 12;
	uint64_t mac1_int                     : 1;  /**< Interrupt from MAC1.
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB_INT) */
	uint64_t mac0_int                     : 1;  /**< Interrupt from MAC0.
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB_INT) */
	uint64_t mio_int1                     : 1;  /**< Interrupt from MIO for PORT 1.
                                                         See CIU_INT33_SUM0, CIU_INT_SUM1
                                                         (enabled by CIU_INT33_EN0, CIU_INT33_EN1) */
	uint64_t mio_int0                     : 1;  /**< Interrupt from MIO for PORT 0.
                                                         See CIU_INT32_SUM0, CIU_INT_SUM1
                                                         (enabled by CIU_INT32_EN0, CIU_INT32_EN1) */
	uint64_t m1_un_wi                     : 1;  /**< Received Unsupported N-TLP for Window Register
                                                         from MAC 1. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m1_un_b0                     : 1;  /**< Received Unsupported N-TLP for Bar0 from MAC 1.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t m1_up_wi                     : 1;  /**< Received Unsupported P-TLP for Window Register
                                                         from MAC 1. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m1_up_b0                     : 1;  /**< Received Unsupported P-TLP for Bar0 from MAC 1.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t m0_un_wi                     : 1;  /**< Received Unsupported N-TLP for Window Register
                                                         from MAC 0. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m0_un_b0                     : 1;  /**< Received Unsupported N-TLP for Bar0 from MAC 0.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t m0_up_wi                     : 1;  /**< Received Unsupported P-TLP for Window Register
                                                         from MAC 0. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m0_up_b0                     : 1;  /**< Received Unsupported P-TLP for Bar0 from MAC 0.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t reserved_6_7                 : 2;
	uint64_t ptime                        : 1;  /**< Packet Timer has an interrupt. Which rings can
                                                         be found in SLI_PKT_TIME_INT. */
	uint64_t pcnt                         : 1;  /**< Packet Counter has an interrupt. Which rings can
                                                         be found in SLI_PKT_CNT_INT. */
	uint64_t iob2big                      : 1;  /**< A requested IOBDMA is to large. */
	uint64_t bar0_to                      : 1;  /**< BAR0 R/W to a NCB device did not receive
                                                         read-data/commit in 0xffff core clocks. */
	uint64_t reserved_1_1                 : 1;
	uint64_t rml_to                       : 1;  /**< A read or write transfer did not complete
                                                         within 0xffff core clocks. */
#else
	uint64_t rml_to                       : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t bar0_to                      : 1;
	uint64_t iob2big                      : 1;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t reserved_6_7                 : 2;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t mio_int0                     : 1;
	uint64_t mio_int1                     : 1;
	uint64_t mac0_int                     : 1;
	uint64_t mac1_int                     : 1;
	uint64_t reserved_20_31               : 12;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_38_47               : 10;
	uint64_t pidbof                       : 1;
	uint64_t psldbof                      : 1;
	uint64_t pout_err                     : 1;
	uint64_t pin_bp                       : 1;
	uint64_t pgl_err                      : 1;
	uint64_t pdi_err                      : 1;
	uint64_t pop_err                      : 1;
	uint64_t pins_err                     : 1;
	uint64_t sprt0_err                    : 1;
	uint64_t sprt1_err                    : 1;
	uint64_t reserved_58_59               : 2;
	uint64_t ill_pad                      : 1;
	uint64_t reserved_61_63               : 3;
#endif
	} cn63xx;
	struct cvmx_sli_int_sum_cn63xx        cn63xxp1;
	struct cvmx_sli_int_sum_cn61xx        cn66xx;
	struct cvmx_sli_int_sum_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_62_63               : 2;
	uint64_t pipe_err                     : 1;  /**< Set when a PIPE value outside range is received. */
	uint64_t ill_pad                      : 1;  /**< Set when a BAR0 address R/W falls into theaddress
                                                         range of the Packet-CSR, but for an unused
                                                         address. */
	uint64_t reserved_58_59               : 2;
	uint64_t sprt1_err                    : 1;  /**< When an error response received on SLI port 1
                                                         this bit is set. */
	uint64_t sprt0_err                    : 1;  /**< When an error response received on SLI port 0
                                                         this bit is set. */
	uint64_t pins_err                     : 1;  /**< When a read error occurs on a packet instruction
                                                         this bit is set. */
	uint64_t pop_err                      : 1;  /**< When a read error occurs on a packet scatter
                                                         pointer pair this bit is set. */
	uint64_t pdi_err                      : 1;  /**< When a read error occurs on a packet data read
                                                         this bit is set. */
	uint64_t pgl_err                      : 1;  /**< When a read error occurs on a packet gather list
                                                         read this bit is set. */
	uint64_t reserved_51_51               : 1;
	uint64_t pout_err                     : 1;  /**< Set when PKO sends packet data with the error bit
                                                         set. */
	uint64_t psldbof                      : 1;  /**< Packet Scatterlist Doorbell count overflowed. Which
                                                         doorbell can be found in DPI_PINT_INFO[PSLDBOF] */
	uint64_t pidbof                       : 1;  /**< Packet Instruction Doorbell count overflowed. Which
                                                         doorbell can be found in DPI_PINT_INFO[PIDBOF] */
	uint64_t reserved_38_47               : 10;
	uint64_t dtime                        : 2;  /**< Whenever SLI_DMAx_CNT[CNT] is not 0, the
                                                         SLI_DMAx_TIM[TIM] timer increments every SLI
                                                         clock.
                                                         DTIME[x] is set whenever SLI_DMAx_TIM[TIM] >
                                                         SLI_DMAx_INT_LEVEL[TIME].
                                                         DTIME[x] is normally cleared by clearing
                                                         SLI_DMAx_CNT[CNT] (which also clears
                                                         SLI_DMAx_TIM[TIM]). */
	uint64_t dcnt                         : 2;  /**< DCNT[x] is set whenever SLI_DMAx_CNT[CNT] >
                                                         SLI_DMAx_INT_LEVEL[CNT].
                                                         DCNT[x] is normally cleared by decreasing
                                                         SLI_DMAx_CNT[CNT]. */
	uint64_t dmafi                        : 2;  /**< DMA set Forced Interrupts. */
	uint64_t reserved_20_31               : 12;
	uint64_t mac1_int                     : 1;  /**< Interrupt from MAC1.
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB_INT) */
	uint64_t mac0_int                     : 1;  /**< Interrupt from MAC0.
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB_INT) */
	uint64_t mio_int1                     : 1;  /**< Interrupt from MIO for PORT 1.
                                                         See CIU_INT33_SUM0, CIU_INT_SUM1
                                                         (enabled by CIU_INT33_EN0, CIU_INT33_EN1) */
	uint64_t mio_int0                     : 1;  /**< Interrupt from MIO for PORT 0.
                                                         See CIU_INT32_SUM0, CIU_INT_SUM1
                                                         (enabled by CIU_INT32_EN0, CIU_INT32_EN1) */
	uint64_t m1_un_wi                     : 1;  /**< Received Unsupported N-TLP for Window Register
                                                         from MAC 1. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m1_un_b0                     : 1;  /**< Received Unsupported N-TLP for Bar0 from MAC 1.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t m1_up_wi                     : 1;  /**< Received Unsupported P-TLP for Window Register
                                                         from MAC 1. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m1_up_b0                     : 1;  /**< Received Unsupported P-TLP for Bar0 from MAC 1.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t m0_un_wi                     : 1;  /**< Received Unsupported N-TLP for Window Register
                                                         from MAC 0. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m0_un_b0                     : 1;  /**< Received Unsupported N-TLP for Bar0 from MAC 0.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t m0_up_wi                     : 1;  /**< Received Unsupported P-TLP for Window Register
                                                         from MAC 0. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m0_up_b0                     : 1;  /**< Received Unsupported P-TLP for Bar0 from MAC 0.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t reserved_6_7                 : 2;
	uint64_t ptime                        : 1;  /**< Packet Timer has an interrupt. Which rings can
                                                         be found in SLI_PKT_TIME_INT. */
	uint64_t pcnt                         : 1;  /**< Packet Counter has an interrupt. Which rings can
                                                         be found in SLI_PKT_CNT_INT. */
	uint64_t iob2big                      : 1;  /**< A requested IOBDMA is to large. */
	uint64_t bar0_to                      : 1;  /**< BAR0 R/W to a NCB device did not receive
                                                         read-data/commit in 0xffff core clocks. */
	uint64_t reserved_1_1                 : 1;
	uint64_t rml_to                       : 1;  /**< A read or write transfer did not complete
                                                         within 0xffff core clocks. */
#else
	uint64_t rml_to                       : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t bar0_to                      : 1;
	uint64_t iob2big                      : 1;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t reserved_6_7                 : 2;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t mio_int0                     : 1;
	uint64_t mio_int1                     : 1;
	uint64_t mac0_int                     : 1;
	uint64_t mac1_int                     : 1;
	uint64_t reserved_20_31               : 12;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_38_47               : 10;
	uint64_t pidbof                       : 1;
	uint64_t psldbof                      : 1;
	uint64_t pout_err                     : 1;
	uint64_t reserved_51_51               : 1;
	uint64_t pgl_err                      : 1;
	uint64_t pdi_err                      : 1;
	uint64_t pop_err                      : 1;
	uint64_t pins_err                     : 1;
	uint64_t sprt0_err                    : 1;
	uint64_t sprt1_err                    : 1;
	uint64_t reserved_58_59               : 2;
	uint64_t ill_pad                      : 1;
	uint64_t pipe_err                     : 1;
	uint64_t reserved_62_63               : 2;
#endif
	} cn68xx;
	struct cvmx_sli_int_sum_cn68xx        cn68xxp1;
	struct cvmx_sli_int_sum_cn70xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_61_63               : 3;
	uint64_t ill_pad                      : 1;  /**< Set when a BAR0 address R/W falls into theaddress
                                                         range of the Packet-CSR, but for an unused
                                                         address. */
	uint64_t sprt3_err                    : 1;  /**< Reserved. */
	uint64_t sprt2_err                    : 1;  /**< When an error response received on SLI port 2
                                                         this bit is set. */
	uint64_t sprt1_err                    : 1;  /**< When an error response received on SLI port 1
                                                         this bit is set. */
	uint64_t sprt0_err                    : 1;  /**< When an error response received on SLI port 0
                                                         this bit is set. */
	uint64_t pins_err                     : 1;  /**< When a read error occurs on a packet instruction
                                                         this bit is set. */
	uint64_t pop_err                      : 1;  /**< When a read error occurs on a packet scatter
                                                         pointer pair this bit is set. */
	uint64_t pdi_err                      : 1;  /**< When a read error occurs on a packet data read
                                                         this bit is set. */
	uint64_t pgl_err                      : 1;  /**< When a read error occurs on a packet gather list
                                                         read this bit is set. */
	uint64_t pin_bp                       : 1;  /**< Packet input count has exceeded the WMARK.
                                                         See SLI_PKT_IN_BP */
	uint64_t pout_err                     : 1;  /**< Set when PKO sends packet data with the error bit
                                                         set. */
	uint64_t psldbof                      : 1;  /**< Packet Scatterlist Doorbell count overflowed. Which
                                                         doorbell can be found in DPI_PINT_INFO[PSLDBOF] */
	uint64_t pidbof                       : 1;  /**< Packet Instruction Doorbell count overflowed. Which
                                                         doorbell can be found in DPI_PINT_INFO[PIDBOF] */
	uint64_t reserved_38_47               : 10;
	uint64_t dtime                        : 2;  /**< Whenever SLI_DMAx_CNT[CNT] is not 0, the
                                                         SLI_DMAx_TIM[TIM] timer increments every SLI
                                                         clock.
                                                         DTIME[x] is set whenever SLI_DMAx_TIM[TIM] >
                                                         SLI_DMAx_INT_LEVEL[TIME].
                                                         DTIME[x] is normally cleared by clearing
                                                         SLI_DMAx_CNT[CNT] (which also clears
                                                         SLI_DMAx_TIM[TIM]). */
	uint64_t dcnt                         : 2;  /**< DCNT[x] is set whenever SLI_DMAx_CNT[CNT] >
                                                         SLI_DMAx_INT_LEVEL[CNT].
                                                         DCNT[x] is normally cleared by decreasing
                                                         SLI_DMAx_CNT[CNT]. */
	uint64_t dmafi                        : 2;  /**< DMA set Forced Interrupts. */
	uint64_t reserved_30_31               : 2;
	uint64_t mac2_int                     : 1;  /**< Interrupt from MAC2.
                                                         See PEM2_INT_SUM (enabled by PEM2_INT_ENB_INT) */
	uint64_t mio_int2                     : 1;  /**< Interrupt from MIO for PORT 2.
                                                         See CIU_INT32_SUM0, CIU_INT_SUM1
                                                         (enabled by CIU_INT32_EN0, CIU_INT32_EN1) */
	uint64_t m3_un_wi                     : 1;  /**< Reserved. */
	uint64_t m3_un_b0                     : 1;  /**< Reserved. */
	uint64_t m3_up_wi                     : 1;  /**< Reserved. */
	uint64_t m3_up_b0                     : 1;  /**< Reserved. */
	uint64_t m2_un_wi                     : 1;  /**< Received Unsupported N-TLP for Window Register
                                                         from MAC 0. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m2_un_b0                     : 1;  /**< Received Unsupported N-TLP for Bar0 from MAC 0.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t m2_up_wi                     : 1;  /**< Received Unsupported P-TLP for Window Register
                                                         from MAC 0. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m2_up_b0                     : 1;  /**< Received Unsupported P-TLP for Bar0 from MAC 0.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t mac1_int                     : 1;  /**< Interrupt from MAC1.
                                                         See PEM1_INT_SUM (enabled by PEM1_INT_ENB_INT) */
	uint64_t mac0_int                     : 1;  /**< Interrupt from MAC0.
                                                         See PEM0_INT_SUM (enabled by PEM0_INT_ENB_INT) */
	uint64_t mio_int1                     : 1;  /**< Interrupt from MIO for PORT 1.
                                                         See CIU_INT33_SUM0, CIU_INT_SUM1
                                                         (enabled by CIU_INT33_EN0, CIU_INT33_EN1) */
	uint64_t mio_int0                     : 1;  /**< Interrupt from MIO for PORT 0.
                                                         See CIU_INT32_SUM0, CIU_INT_SUM1
                                                         (enabled by CIU_INT32_EN0, CIU_INT32_EN1) */
	uint64_t m1_un_wi                     : 1;  /**< Received Unsupported N-TLP for Window Register
                                                         from MAC 1. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m1_un_b0                     : 1;  /**< Received Unsupported N-TLP for Bar0 from MAC 1.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t m1_up_wi                     : 1;  /**< Received Unsupported P-TLP for Window Register
                                                         from MAC 1. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m1_up_b0                     : 1;  /**< Received Unsupported P-TLP for Bar0 from MAC 1.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t m0_un_wi                     : 1;  /**< Received Unsupported N-TLP for Window Register
                                                         from MAC 0. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m0_un_b0                     : 1;  /**< Received Unsupported N-TLP for Bar0 from MAC 0.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t m0_up_wi                     : 1;  /**< Received Unsupported P-TLP for Window Register
                                                         from MAC 0. This occurs when the window registers
                                                         are disabeld and a window register access occurs. */
	uint64_t m0_up_b0                     : 1;  /**< Received Unsupported P-TLP for Bar0 from MAC 0.
                                                         This occurs when the BAR 0 address space is
                                                         disabeled. */
	uint64_t reserved_6_7                 : 2;
	uint64_t ptime                        : 1;  /**< Packet Timer has an interrupt. Which rings can
                                                         be found in SLI_PKT_TIME_INT. */
	uint64_t pcnt                         : 1;  /**< Packet Counter has an interrupt. Which rings can
                                                         be found in SLI_PKT_CNT_INT. */
	uint64_t iob2big                      : 1;  /**< A requested IOBDMA is to large. */
	uint64_t bar0_to                      : 1;  /**< BAR0 R/W to a NCB device did not receive
                                                         read-data/commit in 0xffff core clocks. */
	uint64_t reserved_1_1                 : 1;
	uint64_t rml_to                       : 1;  /**< A read or write transfer did not complete
                                                         within 0xffff core clocks. */
#else
	uint64_t rml_to                       : 1;
	uint64_t reserved_1_1                 : 1;
	uint64_t bar0_to                      : 1;
	uint64_t iob2big                      : 1;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t reserved_6_7                 : 2;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t mio_int0                     : 1;
	uint64_t mio_int1                     : 1;
	uint64_t mac0_int                     : 1;
	uint64_t mac1_int                     : 1;
	uint64_t m2_up_b0                     : 1;
	uint64_t m2_up_wi                     : 1;
	uint64_t m2_un_b0                     : 1;
	uint64_t m2_un_wi                     : 1;
	uint64_t m3_up_b0                     : 1;
	uint64_t m3_up_wi                     : 1;
	uint64_t m3_un_b0                     : 1;
	uint64_t m3_un_wi                     : 1;
	uint64_t mio_int2                     : 1;
	uint64_t mac2_int                     : 1;
	uint64_t reserved_30_31               : 2;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_38_47               : 10;
	uint64_t pidbof                       : 1;
	uint64_t psldbof                      : 1;
	uint64_t pout_err                     : 1;
	uint64_t pin_bp                       : 1;
	uint64_t pgl_err                      : 1;
	uint64_t pdi_err                      : 1;
	uint64_t pop_err                      : 1;
	uint64_t pins_err                     : 1;
	uint64_t sprt0_err                    : 1;
	uint64_t sprt1_err                    : 1;
	uint64_t sprt2_err                    : 1;
	uint64_t sprt3_err                    : 1;
	uint64_t ill_pad                      : 1;
	uint64_t reserved_61_63               : 3;
#endif
	} cn70xx;
	struct cvmx_sli_int_sum_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t sprt3_err                    : 1;  /**< SLI port 3 error. When an error response is received on SLI port 3, this bit is set.
                                                         Throws SLI_INTSN_E::SLI_INT_SPRT3_ERR. */
	uint64_t sprt2_err                    : 1;  /**< SLI port 2 error. When an error response is received on SLI port 2, this bit is set.
                                                         Throws SLI_INTSN_E::SLI_INT_SPRT2_ERR. */
	uint64_t sprt1_err                    : 1;  /**< SLI port 1 error. When an error response is received on SLI port 1, this bit is set.
                                                         Throws SLI_INTSN_E::SLI_INT_SPRT1_ERR. */
	uint64_t sprt0_err                    : 1;  /**< SLI port 0 error. When an error response is received on SLI port 0, this bit is set.
                                                         Throws SLI_INTSN_E::SLI_INT_SPRT0_ERR. */
	uint64_t pins_err                     : 1;  /**< Packet instruction read error. When a read error occurs on a packet instruction, this bit
                                                         is set. Throws SLI_INTSN_E::SLI_INT_PINS_ERR. */
	uint64_t pop_err                      : 1;  /**< Packet scatter pointer pair error. When a read error occurs on a packet scatter pointer
                                                         pair, this bit is set. Throws SLI_INTSN_E::SLI_INT_POP_ERR. */
	uint64_t pdi_err                      : 1;  /**< Packet data read error. When a read error occurs on a packet data read, this bit is set.
                                                         Throws SLI_INTSN_E::SLI_INT_PDI_ERR. */
	uint64_t pgl_err                      : 1;  /**< Packet gather list read error. When a read error occurs on a packet gather list read, this
                                                         bit is set. Throws SLI_INTSN_E::SLI_INT_PGL_ERR. */
	uint64_t reserved_50_51               : 2;
	uint64_t psldbof                      : 1;  /**< Packet scatter list doorbell count overflowed. Which doorbell can be found in
                                                         DPI_PINT_INFO[PSLDBOF]. Throws SLI_INTSN_E::SLI_INT_PSLDBOF. */
	uint64_t pidbof                       : 1;  /**< Packet instruction doorbell count overflowed. Which doorbell can be found in
                                                         DPI_PINT_INFO[PIDBOF]. Throws SLI_INTSN_E::SLI_INT_PIDBOF. */
	uint64_t reserved_38_47               : 10;
	uint64_t dtime                        : 2;  /**< Whenever SLI_DMAx_CNT[CNT] is not 0, the SLI_DMAx_TIM[TIM] timer increments every SLI
                                                         clock. DTIME<x> is set whenever SLI_DMAx_TIM[TIM] > SLI_DMAx_INT_LEVEL[TIME]. DTIME<x> is
                                                         normally cleared by clearing SLI_DMAx_CNT[CNT] (which also clears SLI_DMAx_TIM[TIM]).
                                                         Throws SLI_INTSN_E::SLI_INT_DTIME. */
	uint64_t dcnt                         : 2;  /**< DCNT<x> is set whenever SLI_DMAx_CNT[CNT] > SLI_DMAx_INT_LEVEL[CNT]. DCNT<x> is normally
                                                         cleared by decreasing SLI_DMAx_CNT[CNT]. Throws SLI_INTSN_E::SLI_INT_DCNT. */
	uint64_t dmafi                        : 2;  /**< DMA set forced interrupts. Throws SLI_INTSN_E::SLI_INT_DMAFI. */
	uint64_t reserved_29_31               : 3;
	uint64_t vf_err                       : 1;  /**< Illegal access from VF. Throws SLI_INTSN_E::SLI_INT_VF_ERR. */
	uint64_t m3_un_wi                     : 1;  /**< Received unsupported N-TLP for window register from MAC 3. This occurs when the window
                                                         registers are disabled and a window register access occurs. Throws
                                                         SLI_INTSN_E::SLI_INT_M3_UN_WI. */
	uint64_t m3_un_b0                     : 1;  /**< Received unsupported N-TLP for Bar0 from MAC 3. This occurs when the BAR 0 address space
                                                         is disabled. Throws SLI_INTSN_E::SLI_INT_M3_UN_B0. */
	uint64_t m3_up_wi                     : 1;  /**< Received unsupported P-TLP for window register from MAC 3. This occurs when the window
                                                         registers are disabled and a window register access occurs. Throws
                                                         SLI_INTSN_E::SLI_INT_M3_UP_WI. */
	uint64_t m3_up_b0                     : 1;  /**< Received unsupported P-TLP for Bar0 from MAC 3. This occurs when the BAR 0 address space
                                                         is disabled. Throws SLI_INTSN_E::SLI_INT_M3_UP_B0. */
	uint64_t m2_un_wi                     : 1;  /**< Received unsupported N-TLP for window register from MAC 2. This occurs when the window
                                                         registers are disabled and a window register access occurs. Throws
                                                         SLI_INTSN_E::SLI_INT_M2_UN_WI. */
	uint64_t m2_un_b0                     : 1;  /**< Received unsupported N-TLP for Bar0 from MAC 2. This occurs when the BAR 0 address space
                                                         is disabled. Throws SLI_INTSN_E::SLI_INT_M2_UN_B0. */
	uint64_t m2_up_wi                     : 1;  /**< Received unsupported P-TLP for window register from MAC 2. This occurs when the window
                                                         registers are disabled and a window register access occurs. Throws
                                                         SLI_INTSN_E::SLI_INT_M2_UP_WI. */
	uint64_t m2_up_b0                     : 1;  /**< Received unsupported P-TLP for Bar0 from MAC 2. This occurs when the BAR 0 address space
                                                         is disabled. Throws SLI_INTSN_E::SLI_INT_M2_UP_B0. */
	uint64_t reserved_18_19               : 2;
	uint64_t mio_int1                     : 1;  /**< Interrupt from MIO for PORT 1. Throws SLI_INTSN_E::SLI_INT_MIO_INT1. */
	uint64_t mio_int0                     : 1;  /**< Interrupt from MIO for PORT 0. Throws SLI_INTSN_E::SLI_INT_MIO_INT0. */
	uint64_t m1_un_wi                     : 1;  /**< Received unsupported N-TLP for window register from MAC 1. This occurs when the window
                                                         registers are disabled and a window register access occurs. Throws
                                                         SLI_INTSN_E::SLI_INT_M1_UN_WI. */
	uint64_t m1_un_b0                     : 1;  /**< Received unsupported N-TLP for Bar 0 from MAC 1. This occurs when the BAR 0 address space
                                                         is disabled. Throws SLI_INTSN_E::SLI_INT_M1_UN_B0. */
	uint64_t m1_up_wi                     : 1;  /**< Received unsupported P-TLP for window register from MAC 1. This occurs when the window
                                                         registers are disabled and a window register access occurs. Throws
                                                         SLI_INTSN_E::SLI_INT_M1_UP_WI. */
	uint64_t m1_up_b0                     : 1;  /**< Received unsupported P-TLP for Bar 0 from MAC 1. This occurs when the BAR 0 address space
                                                         is disabled. Throws SLI_INTSN_E::SLI_INT_M1_UP_B0. */
	uint64_t m0_un_wi                     : 1;  /**< Received unsupported N-TLP for window register from MAC 0. This occurs when the window
                                                         registers are disabled and a window register access occurs. Throws
                                                         SLI_INTSN_E::SLI_INT_M0_UP_WI. */
	uint64_t m0_un_b0                     : 1;  /**< Received unsupported N-TLP for Bar 0 from MAC 0. This occurs when the BAR 0 address space
                                                         is disabled. Throws SLI_INTSN_E::SLI_INT_M0_UP_B0. */
	uint64_t m0_up_wi                     : 1;  /**< Received unsupported P-TLP for window register from MAC 0. This occurs when the window
                                                         registers are disabled and a window register access occurs. Throws
                                                         SLI_INTSN_E::SLI_INT_M0_UP_WI. */
	uint64_t m0_up_b0                     : 1;  /**< Received unsupported P-TLP for Bar 0 from MAC 0. This occurs when the BAR 0 address space
                                                         is disabled. Throws SLI_INTSN_E::SLI_INT_M0_UP_B0. */
	uint64_t mio_int3                     : 1;  /**< Interrupt from MIO for Port 3. Throws SLI_INTSN_E::SLI_INT_MIO_INT3. */
	uint64_t mio_int2                     : 1;  /**< Interrupt from MIO for Port 2. Throws SLI_INTSN_E::SLI_INT_MIO_INT2. */
	uint64_t ptime                        : 1;  /**< Packet timer has an interrupt. The specific rings can be found in SLI_PKT_TIME_INT. Throws
                                                         SLI_INTSN_E::SLI_INT_PTIME. */
	uint64_t pcnt                         : 1;  /**< Packet counter has an interrupt. The specific rings can be found in SLI_PKT_CNT_INT.
                                                         Throws SLI_INTSN_E::SLI_INT_PCNT. */
	uint64_t reserved_1_3                 : 3;
	uint64_t rml_to                       : 1;  /**< A read or write transfer to a RSL that did not complete within SLI_WINDOW_CTL[TIME] sclk
                                                         cycles, or
                                                         a notification from the OCI that is has sent a previously written command and can take
                                                         another within
                                                         SLI_WINDOW_CTL[OCX_TIME]. Throws a SLI_INTSN_E::SLI_INT_RML_TO. */
#else
	uint64_t rml_to                       : 1;
	uint64_t reserved_1_3                 : 3;
	uint64_t pcnt                         : 1;
	uint64_t ptime                        : 1;
	uint64_t mio_int2                     : 1;
	uint64_t mio_int3                     : 1;
	uint64_t m0_up_b0                     : 1;
	uint64_t m0_up_wi                     : 1;
	uint64_t m0_un_b0                     : 1;
	uint64_t m0_un_wi                     : 1;
	uint64_t m1_up_b0                     : 1;
	uint64_t m1_up_wi                     : 1;
	uint64_t m1_un_b0                     : 1;
	uint64_t m1_un_wi                     : 1;
	uint64_t mio_int0                     : 1;
	uint64_t mio_int1                     : 1;
	uint64_t reserved_18_19               : 2;
	uint64_t m2_up_b0                     : 1;
	uint64_t m2_up_wi                     : 1;
	uint64_t m2_un_b0                     : 1;
	uint64_t m2_un_wi                     : 1;
	uint64_t m3_up_b0                     : 1;
	uint64_t m3_up_wi                     : 1;
	uint64_t m3_un_b0                     : 1;
	uint64_t m3_un_wi                     : 1;
	uint64_t vf_err                       : 1;
	uint64_t reserved_29_31               : 3;
	uint64_t dmafi                        : 2;
	uint64_t dcnt                         : 2;
	uint64_t dtime                        : 2;
	uint64_t reserved_38_47               : 10;
	uint64_t pidbof                       : 1;
	uint64_t psldbof                      : 1;
	uint64_t reserved_50_51               : 2;
	uint64_t pgl_err                      : 1;
	uint64_t pdi_err                      : 1;
	uint64_t pop_err                      : 1;
	uint64_t pins_err                     : 1;
	uint64_t sprt0_err                    : 1;
	uint64_t sprt1_err                    : 1;
	uint64_t sprt2_err                    : 1;
	uint64_t sprt3_err                    : 1;
	uint64_t reserved_60_63               : 4;
#endif
	} cn78xx;
	struct cvmx_sli_int_sum_cn61xx        cnf71xx;
};
typedef union cvmx_sli_int_sum cvmx_sli_int_sum_t;

/**
 * cvmx_sli_last_win_rdata0
 *
 * The data from the last initiated window read by MAC 0.
 *
 */
union cvmx_sli_last_win_rdata0 {
	uint64_t u64;
	struct cvmx_sli_last_win_rdata0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Last window read data. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_sli_last_win_rdata0_s     cn61xx;
	struct cvmx_sli_last_win_rdata0_s     cn63xx;
	struct cvmx_sli_last_win_rdata0_s     cn63xxp1;
	struct cvmx_sli_last_win_rdata0_s     cn66xx;
	struct cvmx_sli_last_win_rdata0_s     cn68xx;
	struct cvmx_sli_last_win_rdata0_s     cn68xxp1;
	struct cvmx_sli_last_win_rdata0_s     cn70xx;
	struct cvmx_sli_last_win_rdata0_s     cnf71xx;
};
typedef union cvmx_sli_last_win_rdata0 cvmx_sli_last_win_rdata0_t;

/**
 * cvmx_sli_last_win_rdata1
 *
 * The data from the last initiated window read by MAC 1.
 *
 */
union cvmx_sli_last_win_rdata1 {
	uint64_t u64;
	struct cvmx_sli_last_win_rdata1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Last window read data. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_sli_last_win_rdata1_s     cn61xx;
	struct cvmx_sli_last_win_rdata1_s     cn63xx;
	struct cvmx_sli_last_win_rdata1_s     cn63xxp1;
	struct cvmx_sli_last_win_rdata1_s     cn66xx;
	struct cvmx_sli_last_win_rdata1_s     cn68xx;
	struct cvmx_sli_last_win_rdata1_s     cn68xxp1;
	struct cvmx_sli_last_win_rdata1_s     cn70xx;
	struct cvmx_sli_last_win_rdata1_s     cnf71xx;
};
typedef union cvmx_sli_last_win_rdata1 cvmx_sli_last_win_rdata1_t;

/**
 * cvmx_sli_last_win_rdata2
 *
 * The data from the last initiated window read by MAC 2.
 *
 */
union cvmx_sli_last_win_rdata2 {
	uint64_t u64;
	struct cvmx_sli_last_win_rdata2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Last window read data. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_sli_last_win_rdata2_s     cn61xx;
	struct cvmx_sli_last_win_rdata2_s     cn66xx;
	struct cvmx_sli_last_win_rdata2_s     cn70xx;
	struct cvmx_sli_last_win_rdata2_s     cnf71xx;
};
typedef union cvmx_sli_last_win_rdata2 cvmx_sli_last_win_rdata2_t;

/**
 * cvmx_sli_last_win_rdata3
 *
 * The data from the last initiated window read by MAC 3.
 *
 */
union cvmx_sli_last_win_rdata3 {
	uint64_t u64;
	struct cvmx_sli_last_win_rdata3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Last window read data. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_sli_last_win_rdata3_s     cn61xx;
	struct cvmx_sli_last_win_rdata3_s     cn66xx;
	struct cvmx_sli_last_win_rdata3_s     cn70xx;
	struct cvmx_sli_last_win_rdata3_s     cnf71xx;
};
typedef union cvmx_sli_last_win_rdata3 cvmx_sli_last_win_rdata3_t;

/**
 * cvmx_sli_mac_credit_cnt
 *
 * This register contains the number of credits for the MAC port FIFOs used by the SLI. This
 * value needs to be set before S2M traffic flow starts. A write operation to this register
 * causes the credit counts in the SLI for the MAC ports to be reset to the value in this
 * register if the corresponding disable bit in this register is set to 0.
 */
union cvmx_sli_mac_credit_cnt {
	uint64_t u64;
	struct cvmx_sli_mac_credit_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_54_63               : 10;
	uint64_t p1_c_d                       : 1;  /**< When set does not allow writing of P1_CCNT. */
	uint64_t p1_n_d                       : 1;  /**< When set does not allow writing of P1_NCNT. */
	uint64_t p1_p_d                       : 1;  /**< When set does not allow writing of P1_PCNT. */
	uint64_t p0_c_d                       : 1;  /**< When set does not allow writing of P0_CCNT. */
	uint64_t p0_n_d                       : 1;  /**< When set does not allow writing of P0_NCNT. */
	uint64_t p0_p_d                       : 1;  /**< When set does not allow writing of P0_PCNT. */
	uint64_t p1_ccnt                      : 8;  /**< Port1 C-TLP FIFO Credits.
                                                         Legal values are 0x25 to 0x80. */
	uint64_t p1_ncnt                      : 8;  /**< Port1 N-TLP FIFO Credits.
                                                         Legal values are 0x5 to 0x10. */
	uint64_t p1_pcnt                      : 8;  /**< Port1 P-TLP FIFO Credits.
                                                         Legal values are 0x25 to 0x80. */
	uint64_t p0_ccnt                      : 8;  /**< Port0 C-TLP FIFO Credits.
                                                         Legal values are 0x25 to 0x80. */
	uint64_t p0_ncnt                      : 8;  /**< Port0 N-TLP FIFO Credits.
                                                         Legal values are 0x5 to 0x10. */
	uint64_t p0_pcnt                      : 8;  /**< Port0 P-TLP FIFO Credits.
                                                         Legal values are 0x25 to 0x80. */
#else
	uint64_t p0_pcnt                      : 8;
	uint64_t p0_ncnt                      : 8;
	uint64_t p0_ccnt                      : 8;
	uint64_t p1_pcnt                      : 8;
	uint64_t p1_ncnt                      : 8;
	uint64_t p1_ccnt                      : 8;
	uint64_t p0_p_d                       : 1;
	uint64_t p0_n_d                       : 1;
	uint64_t p0_c_d                       : 1;
	uint64_t p1_p_d                       : 1;
	uint64_t p1_n_d                       : 1;
	uint64_t p1_c_d                       : 1;
	uint64_t reserved_54_63               : 10;
#endif
	} s;
	struct cvmx_sli_mac_credit_cnt_s      cn61xx;
	struct cvmx_sli_mac_credit_cnt_s      cn63xx;
	struct cvmx_sli_mac_credit_cnt_cn63xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t p1_ccnt                      : 8;  /**< Port1 C-TLP FIFO Credits.
                                                         Legal values are 0x25 to 0x80. */
	uint64_t p1_ncnt                      : 8;  /**< Port1 N-TLP FIFO Credits.
                                                         Legal values are 0x5 to 0x10. */
	uint64_t p1_pcnt                      : 8;  /**< Port1 P-TLP FIFO Credits.
                                                         Legal values are 0x25 to 0x80. */
	uint64_t p0_ccnt                      : 8;  /**< Port0 C-TLP FIFO Credits.
                                                         Legal values are 0x25 to 0x80. */
	uint64_t p0_ncnt                      : 8;  /**< Port0 N-TLP FIFO Credits.
                                                         Legal values are 0x5 to 0x10. */
	uint64_t p0_pcnt                      : 8;  /**< Port0 P-TLP FIFO Credits.
                                                         Legal values are 0x25 to 0x80. */
#else
	uint64_t p0_pcnt                      : 8;
	uint64_t p0_ncnt                      : 8;
	uint64_t p0_ccnt                      : 8;
	uint64_t p1_pcnt                      : 8;
	uint64_t p1_ncnt                      : 8;
	uint64_t p1_ccnt                      : 8;
	uint64_t reserved_48_63               : 16;
#endif
	} cn63xxp1;
	struct cvmx_sli_mac_credit_cnt_s      cn66xx;
	struct cvmx_sli_mac_credit_cnt_s      cn68xx;
	struct cvmx_sli_mac_credit_cnt_s      cn68xxp1;
	struct cvmx_sli_mac_credit_cnt_s      cn70xx;
	struct cvmx_sli_mac_credit_cnt_s      cn78xx;
	struct cvmx_sli_mac_credit_cnt_s      cnf71xx;
};
typedef union cvmx_sli_mac_credit_cnt cvmx_sli_mac_credit_cnt_t;

/**
 * cvmx_sli_mac_credit_cnt2
 *
 * This register contains the number of credits for the MAC port FIFOs (for MACs 2 and 3) used by
 * the SLI. This value must be set before S2M traffic flow starts. A write to this register
 * causes the credit counts in the SLI for the MAC ports to be reset to the value in this
 * register.
 */
union cvmx_sli_mac_credit_cnt2 {
	uint64_t u64;
	struct cvmx_sli_mac_credit_cnt2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_54_63               : 10;
	uint64_t p3_c_d                       : 1;  /**< When set does not allow writing of P3_CCNT. */
	uint64_t p3_n_d                       : 1;  /**< When set does not allow writing of P3_NCNT. */
	uint64_t p3_p_d                       : 1;  /**< When set does not allow writing of P3_PCNT. */
	uint64_t p2_c_d                       : 1;  /**< When set does not allow writing of P2_CCNT. */
	uint64_t p2_n_d                       : 1;  /**< When set does not allow writing of P2_NCNT. */
	uint64_t p2_p_d                       : 1;  /**< When set does not allow writing of P2_PCNT. */
	uint64_t p3_ccnt                      : 8;  /**< Port3 C-TLP FIFO Credits.
                                                         Legal values are 0x25 to 0x80. */
	uint64_t p3_ncnt                      : 8;  /**< Port3 N-TLP FIFO Credits.
                                                         Legal values are 0x5 to 0x10. */
	uint64_t p3_pcnt                      : 8;  /**< Port3 P-TLP FIFO Credits.
                                                         Legal values are 0x25 to 0x80. */
	uint64_t p2_ccnt                      : 8;  /**< Port2 C-TLP FIFO Credits.
                                                         Legal values are 0x25 to 0x80. */
	uint64_t p2_ncnt                      : 8;  /**< Port2 N-TLP FIFO Credits.
                                                         Legal values are 0x5 to 0x10. */
	uint64_t p2_pcnt                      : 8;  /**< Port2 P-TLP FIFO Credits.
                                                         Legal values are 0x25 to 0x80. */
#else
	uint64_t p2_pcnt                      : 8;
	uint64_t p2_ncnt                      : 8;
	uint64_t p2_ccnt                      : 8;
	uint64_t p3_pcnt                      : 8;
	uint64_t p3_ncnt                      : 8;
	uint64_t p3_ccnt                      : 8;
	uint64_t p2_p_d                       : 1;
	uint64_t p2_n_d                       : 1;
	uint64_t p2_c_d                       : 1;
	uint64_t p3_p_d                       : 1;
	uint64_t p3_n_d                       : 1;
	uint64_t p3_c_d                       : 1;
	uint64_t reserved_54_63               : 10;
#endif
	} s;
	struct cvmx_sli_mac_credit_cnt2_s     cn61xx;
	struct cvmx_sli_mac_credit_cnt2_s     cn66xx;
	struct cvmx_sli_mac_credit_cnt2_s     cn70xx;
	struct cvmx_sli_mac_credit_cnt2_s     cn78xx;
	struct cvmx_sli_mac_credit_cnt2_s     cnf71xx;
};
typedef union cvmx_sli_mac_credit_cnt2 cvmx_sli_mac_credit_cnt2_t;

/**
 * cvmx_sli_mac_number
 *
 * When read from a MAC port, this register returns the MAC's port number, otherwise returns zero.
 *
 */
union cvmx_sli_mac_number {
	uint64_t u64;
	struct cvmx_sli_mac_number_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t a_mode                       : 1;  /**< SLI in Authenticate Mode. */
	uint64_t num                          : 8;  /**< The mac number. */
#else
	uint64_t num                          : 8;
	uint64_t a_mode                       : 1;
	uint64_t reserved_9_63                : 55;
#endif
	} s;
	struct cvmx_sli_mac_number_s          cn61xx;
	struct cvmx_sli_mac_number_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t num                          : 8;  /**< The mac number. */
#else
	uint64_t num                          : 8;
	uint64_t reserved_8_63                : 56;
#endif
	} cn63xx;
	struct cvmx_sli_mac_number_s          cn66xx;
	struct cvmx_sli_mac_number_cn63xx     cn68xx;
	struct cvmx_sli_mac_number_cn63xx     cn68xxp1;
	struct cvmx_sli_mac_number_s          cn70xx;
	struct cvmx_sli_mac_number_s          cn78xx;
	struct cvmx_sli_mac_number_s          cnf71xx;
};
typedef union cvmx_sli_mac_number cvmx_sli_mac_number_t;

/**
 * cvmx_sli_mem_access_ctl
 *
 * This register contains control signals for access to the MAC address space.
 *
 */
union cvmx_sli_mem_access_ctl {
	uint64_t u64;
	struct cvmx_sli_mem_access_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t max_word                     : 4;  /**< The maximum number of words to merge into a single
                                                         write operation from the PPs to the MAC. Legal
                                                         values are 1 to 16, where a '0' is treated as 16. */
	uint64_t timer                        : 10; /**< When the SLI starts a PP to MAC write it waits
                                                         no longer than the value of TIMER in eclks to
                                                         merge additional writes from the PPs into 1
                                                         large write. The values for this field is 1 to
                                                         1024 where a value of '0' is treated as 1024. */
#else
	uint64_t timer                        : 10;
	uint64_t max_word                     : 4;
	uint64_t reserved_14_63               : 50;
#endif
	} s;
	struct cvmx_sli_mem_access_ctl_s      cn61xx;
	struct cvmx_sli_mem_access_ctl_s      cn63xx;
	struct cvmx_sli_mem_access_ctl_s      cn63xxp1;
	struct cvmx_sli_mem_access_ctl_s      cn66xx;
	struct cvmx_sli_mem_access_ctl_s      cn68xx;
	struct cvmx_sli_mem_access_ctl_s      cn68xxp1;
	struct cvmx_sli_mem_access_ctl_s      cn70xx;
	struct cvmx_sli_mem_access_ctl_s      cn78xx;
	struct cvmx_sli_mem_access_ctl_s      cnf71xx;
};
typedef union cvmx_sli_mem_access_ctl cvmx_sli_mem_access_ctl_t;

/**
 * cvmx_sli_mem_access_subid#
 *
 * These registers contains address index and control bits for access to memory from cores.
 *
 */
union cvmx_sli_mem_access_subidx {
	uint64_t u64;
	struct cvmx_sli_mem_access_subidx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_43_63               : 21;
	uint64_t zero                         : 1;  /**< Causes all byte reads to be zero length reads.
                                                         Returns to the EXEC a zero for all read data.
                                                         This must be zero for sRIO ports. */
	uint64_t port                         : 3;  /**< Physical MAC Port that reads/writes to
                                                         this subid are sent to. Must be <= 1, as there are
                                                         only two ports present. */
	uint64_t nmerge                       : 1;  /**< When set, no merging is allowed in this window. */
	uint64_t esr                          : 2;  /**< ES<1:0> for reads to this subid.
                                                         ES<1:0> is the endian-swap attribute for these MAC
                                                         memory space reads. */
	uint64_t esw                          : 2;  /**< ES<1:0> for writes to this subid.
                                                         ES<1:0> is the endian-swap attribute for these MAC
                                                         memory space writes. */
	uint64_t wtype                        : 2;  /**< ADDRTYPE<1:0> for writes to this subid
                                                         For PCIe:
                                                         - ADDRTYPE<0> is the relaxed-order attribute
                                                         - ADDRTYPE<1> is the no-snoop attribute
                                                         For sRIO:
                                                         - ADDRTYPE<1:0> help select an SRIO*_S2M_TYPE*
                                                           entry */
	uint64_t rtype                        : 2;  /**< ADDRTYPE<1:0> for reads to this subid
                                                         For PCIe:
                                                         - ADDRTYPE<0> is the relaxed-order attribute
                                                         - ADDRTYPE<1> is the no-snoop attribute
                                                         For sRIO:
                                                         - ADDRTYPE<1:0> help select an SRIO*_S2M_TYPE*
                                                           entry */
	uint64_t reserved_0_29                : 30;
#else
	uint64_t reserved_0_29                : 30;
	uint64_t rtype                        : 2;
	uint64_t wtype                        : 2;
	uint64_t esw                          : 2;
	uint64_t esr                          : 2;
	uint64_t nmerge                       : 1;
	uint64_t port                         : 3;
	uint64_t zero                         : 1;
	uint64_t reserved_43_63               : 21;
#endif
	} s;
	struct cvmx_sli_mem_access_subidx_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_43_63               : 21;
	uint64_t zero                         : 1;  /**< Causes all byte reads to be zero length reads.
                                                         Returns to the EXEC a zero for all read data.
                                                         This must be zero for sRIO ports. */
	uint64_t port                         : 3;  /**< Physical MAC Port that reads/writes to
                                                         this subid are sent to. Must be <= 1, as there are
                                                         only two ports present. */
	uint64_t nmerge                       : 1;  /**< When set, no merging is allowed in this window. */
	uint64_t esr                          : 2;  /**< ES<1:0> for reads to this subid.
                                                         ES<1:0> is the endian-swap attribute for these MAC
                                                         memory space reads. */
	uint64_t esw                          : 2;  /**< ES<1:0> for writes to this subid.
                                                         ES<1:0> is the endian-swap attribute for these MAC
                                                         memory space writes. */
	uint64_t wtype                        : 2;  /**< ADDRTYPE<1:0> for writes to this subid
                                                         For PCIe:
                                                         - ADDRTYPE<0> is the relaxed-order attribute
                                                         - ADDRTYPE<1> is the no-snoop attribute
                                                         For sRIO:
                                                         - ADDRTYPE<1:0> help select an SRIO*_S2M_TYPE*
                                                           entry */
	uint64_t rtype                        : 2;  /**< ADDRTYPE<1:0> for reads to this subid
                                                         For PCIe:
                                                         - ADDRTYPE<0> is the relaxed-order attribute
                                                         - ADDRTYPE<1> is the no-snoop attribute
                                                         For sRIO:
                                                         - ADDRTYPE<1:0> help select an SRIO*_S2M_TYPE*
                                                           entry */
	uint64_t ba                           : 30; /**< Address Bits <63:34> for reads/writes that use
                                                         this subid. */
#else
	uint64_t ba                           : 30;
	uint64_t rtype                        : 2;
	uint64_t wtype                        : 2;
	uint64_t esw                          : 2;
	uint64_t esr                          : 2;
	uint64_t nmerge                       : 1;
	uint64_t port                         : 3;
	uint64_t zero                         : 1;
	uint64_t reserved_43_63               : 21;
#endif
	} cn61xx;
	struct cvmx_sli_mem_access_subidx_cn61xx cn63xx;
	struct cvmx_sli_mem_access_subidx_cn61xx cn63xxp1;
	struct cvmx_sli_mem_access_subidx_cn61xx cn66xx;
	struct cvmx_sli_mem_access_subidx_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_43_63               : 21;
	uint64_t zero                         : 1;  /**< Causes all byte reads to be zero length reads.
                                                         Returns to the EXEC a zero for all read data.
                                                         This must be zero for sRIO ports. */
	uint64_t port                         : 3;  /**< Physical MAC Port that reads/writes to
                                                         this subid are sent to. Must be <= 1, as there are
                                                         only two ports present. */
	uint64_t nmerge                       : 1;  /**< When set, no merging is allowed in this window. */
	uint64_t esr                          : 2;  /**< ES<1:0> for reads to this subid.
                                                         ES<1:0> is the endian-swap attribute for these MAC
                                                         memory space reads. */
	uint64_t esw                          : 2;  /**< ES<1:0> for writes to this subid.
                                                         ES<1:0> is the endian-swap attribute for these MAC
                                                         memory space writes. */
	uint64_t wtype                        : 2;  /**< ADDRTYPE<1:0> for writes to this subid
                                                         For PCIe:
                                                         - ADDRTYPE<0> is the relaxed-order attribute
                                                         - ADDRTYPE<1> is the no-snoop attribute */
	uint64_t rtype                        : 2;  /**< ADDRTYPE<1:0> for reads to this subid
                                                         For PCIe:
                                                         - ADDRTYPE<0> is the relaxed-order attribute
                                                         - ADDRTYPE<1> is the no-snoop attribute */
	uint64_t ba                           : 28; /**< Address Bits <63:36> for reads/writes that use
                                                         this subid. */
	uint64_t reserved_0_1                 : 2;
#else
	uint64_t reserved_0_1                 : 2;
	uint64_t ba                           : 28;
	uint64_t rtype                        : 2;
	uint64_t wtype                        : 2;
	uint64_t esw                          : 2;
	uint64_t esr                          : 2;
	uint64_t nmerge                       : 1;
	uint64_t port                         : 3;
	uint64_t zero                         : 1;
	uint64_t reserved_43_63               : 21;
#endif
	} cn68xx;
	struct cvmx_sli_mem_access_subidx_cn68xx cn68xxp1;
	struct cvmx_sli_mem_access_subidx_cn61xx cn70xx;
	struct cvmx_sli_mem_access_subidx_cn61xx cn78xx;
	struct cvmx_sli_mem_access_subidx_cn61xx cnf71xx;
};
typedef union cvmx_sli_mem_access_subidx cvmx_sli_mem_access_subidx_t;

/**
 * cvmx_sli_mem_ctl
 *
 * This register controls the ECC of the SLI memories.
 *
 */
union cvmx_sli_mem_ctl {
	uint64_t u64;
	struct cvmx_sli_mem_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_27_63               : 37;
	uint64_t tlpn1_fs                     : 2;  /**< Used to flip the synd. for p2n1_tlp_n_fifo. */
	uint64_t tlpn1_ecc                    : 1;  /**< When set the for p2n1_tlp_n_fifo will have an ECC not generated and checked. */
	uint64_t tlpp1_fs                     : 2;  /**< Used to flip the synd. for p2n1_tlp_p_fifo. */
	uint64_t tlpp1_ecc                    : 1;  /**< When set the for p2n1_tlp_p_fifo will have an ECC not generated and checked. */
	uint64_t tlpc1_fs                     : 2;  /**< Used to flip the synd. for p2n1_tlp_cpl_fifo. */
	uint64_t tlpc1_ecc                    : 1;  /**< When set the for p2n1_tlp_cpl_fifo will have an ECC not generated and checked. */
	uint64_t tlpn0_fs                     : 2;  /**< Used to flip the synd. for p2n0_tlp_n_fifo. */
	uint64_t tlpn0_ecc                    : 1;  /**< When set the for p2n0_tlp_n_fifo will have an ECC not generated and checked. */
	uint64_t tlpp0_fs                     : 2;  /**< Used to flip the synd. for p2n0_tlp_p_fifo. */
	uint64_t tlpp0_ecc                    : 1;  /**< When set the for p2n0_tlp_p_fifo will have an ECC not generated and checked. */
	uint64_t tlpc0_fs                     : 2;  /**< Used to flip the synd. for p2n0_tlp_cpl_fifo. */
	uint64_t tlpc0_ecc                    : 1;  /**< When set the for p2n0_tlp_cpl_fifo will have an ECC not generated and checked. */
	uint64_t nppr_fs                      : 2;  /**< Used to flip the synd. for nod_pp_req_fifo. */
	uint64_t nppr_ecc                     : 1;  /**< When set the for nod_pp_req_fifo  will have an ECC not generated and checked. */
	uint64_t cpl1_fs                      : 2;  /**< Used to flip the synd. for cpl1_fifo. */
	uint64_t cpl1_ecc                     : 1;  /**< When the set cpl1_fifo will have an ECC not generated and checked. */
	uint64_t cpl0_fs                      : 2;  /**< Used to flip the synd. for the cpl0_fifo. */
	uint64_t cpl0_ecc                     : 1;  /**< When set the cpl0_fifo will have an ECC not generated and checked. */
#else
	uint64_t cpl0_ecc                     : 1;
	uint64_t cpl0_fs                      : 2;
	uint64_t cpl1_ecc                     : 1;
	uint64_t cpl1_fs                      : 2;
	uint64_t nppr_ecc                     : 1;
	uint64_t nppr_fs                      : 2;
	uint64_t tlpc0_ecc                    : 1;
	uint64_t tlpc0_fs                     : 2;
	uint64_t tlpp0_ecc                    : 1;
	uint64_t tlpp0_fs                     : 2;
	uint64_t tlpn0_ecc                    : 1;
	uint64_t tlpn0_fs                     : 2;
	uint64_t tlpc1_ecc                    : 1;
	uint64_t tlpc1_fs                     : 2;
	uint64_t tlpp1_ecc                    : 1;
	uint64_t tlpp1_fs                     : 2;
	uint64_t tlpn1_ecc                    : 1;
	uint64_t tlpn1_fs                     : 2;
	uint64_t reserved_27_63               : 37;
#endif
	} s;
	struct cvmx_sli_mem_ctl_s             cn78xx;
};
typedef union cvmx_sli_mem_ctl cvmx_sli_mem_ctl_t;

/**
 * cvmx_sli_mem_int_sum
 *
 * Set when an interrupt condition occurs; write one to clear.
 *
 */
union cvmx_sli_mem_int_sum {
	uint64_t u64;
	struct cvmx_sli_mem_int_sum_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t tlpn1_dbe                    : 1;  /**< Set when the p2n1_tlp_n_fifo has a DBE. */
	uint64_t tlpn1_sbe                    : 1;  /**< Set when the p2n1_tlp_n_fifo has a SBE. */
	uint64_t tlpp1_dbe                    : 1;  /**< Set when the p2n1_tlp_p_fifo has a DBE. */
	uint64_t tlpp1_sbe                    : 1;  /**< Set when the p2n1_tlp_p_fifo has a SBE. */
	uint64_t tlpc1_dbe                    : 1;  /**< Set when the p2n1_tlp_cpl_fifo has a DBE. */
	uint64_t tlpc1_sbe                    : 1;  /**< Set when the p2n1_tlp_cpl_fifo has a SBE. */
	uint64_t tlpn0_dbe                    : 1;  /**< Set when the p2n0_tlp_n_fifo has a DBE. */
	uint64_t tlpn0_sbe                    : 1;  /**< Set when the p2n0_tlp_n_fifo has a SBE. */
	uint64_t tlpp0_dbe                    : 1;  /**< Set when the p2n0_tlp_p_fifo has a DBE. */
	uint64_t tlpp0_sbe                    : 1;  /**< Set when the p2n0_tlp_p_fifo has a SBE. */
	uint64_t tlpc0_dbe                    : 1;  /**< Set when the p2n0_tlp_cpl_fifo has a DBE. */
	uint64_t tlpc0_sbe                    : 1;  /**< Set when the p2n0_tlp_cpl_fifo has a SBE. */
	uint64_t nppr_dbe                     : 1;  /**< Set when the nod_pp_req_fifo has a DBE. */
	uint64_t nppr_sbe                     : 1;  /**< Set when the nod_pp_req_fifo has a SBE. */
	uint64_t cpl1_dbe                     : 1;  /**< Set when the cpl1_fifo has a DBE. */
	uint64_t cpl1_sbe                     : 1;  /**< Set when the cpl1_fifo has a SBE. */
	uint64_t cpl0_dbe                     : 1;  /**< Set when the cpl0_fifo has a DBE. */
	uint64_t cpl0_sbe                     : 1;  /**< Set when the cpl0_fifo has a SBE. */
#else
	uint64_t cpl0_sbe                     : 1;
	uint64_t cpl0_dbe                     : 1;
	uint64_t cpl1_sbe                     : 1;
	uint64_t cpl1_dbe                     : 1;
	uint64_t nppr_sbe                     : 1;
	uint64_t nppr_dbe                     : 1;
	uint64_t tlpc0_sbe                    : 1;
	uint64_t tlpc0_dbe                    : 1;
	uint64_t tlpp0_sbe                    : 1;
	uint64_t tlpp0_dbe                    : 1;
	uint64_t tlpn0_sbe                    : 1;
	uint64_t tlpn0_dbe                    : 1;
	uint64_t tlpc1_sbe                    : 1;
	uint64_t tlpc1_dbe                    : 1;
	uint64_t tlpp1_sbe                    : 1;
	uint64_t tlpp1_dbe                    : 1;
	uint64_t tlpn1_sbe                    : 1;
	uint64_t tlpn1_dbe                    : 1;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_sli_mem_int_sum_s         cn78xx;
};
typedef union cvmx_sli_mem_int_sum cvmx_sli_mem_int_sum_t;

/**
 * cvmx_sli_msi_enb0
 *
 * Used to enable the interrupt generation for the bits in the SLI_MSI_RCV0.
 *
 */
union cvmx_sli_msi_enb0 {
	uint64_t u64;
	struct cvmx_sli_msi_enb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t enb                          : 64; /**< Enables bit [63:0] of SLI_MSI_RCV0. */
#else
	uint64_t enb                          : 64;
#endif
	} s;
	struct cvmx_sli_msi_enb0_s            cn61xx;
	struct cvmx_sli_msi_enb0_s            cn63xx;
	struct cvmx_sli_msi_enb0_s            cn63xxp1;
	struct cvmx_sli_msi_enb0_s            cn66xx;
	struct cvmx_sli_msi_enb0_s            cn68xx;
	struct cvmx_sli_msi_enb0_s            cn68xxp1;
	struct cvmx_sli_msi_enb0_s            cn70xx;
	struct cvmx_sli_msi_enb0_s            cnf71xx;
};
typedef union cvmx_sli_msi_enb0 cvmx_sli_msi_enb0_t;

/**
 * cvmx_sli_msi_enb1
 *
 * Used to enable the interrupt generation for the bits in the SLI_MSI_RCV1.
 *
 */
union cvmx_sli_msi_enb1 {
	uint64_t u64;
	struct cvmx_sli_msi_enb1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t enb                          : 64; /**< Enables bit [63:0] of SLI_MSI_RCV1. */
#else
	uint64_t enb                          : 64;
#endif
	} s;
	struct cvmx_sli_msi_enb1_s            cn61xx;
	struct cvmx_sli_msi_enb1_s            cn63xx;
	struct cvmx_sli_msi_enb1_s            cn63xxp1;
	struct cvmx_sli_msi_enb1_s            cn66xx;
	struct cvmx_sli_msi_enb1_s            cn68xx;
	struct cvmx_sli_msi_enb1_s            cn68xxp1;
	struct cvmx_sli_msi_enb1_s            cn70xx;
	struct cvmx_sli_msi_enb1_s            cnf71xx;
};
typedef union cvmx_sli_msi_enb1 cvmx_sli_msi_enb1_t;

/**
 * cvmx_sli_msi_enb2
 *
 * Used to enable the interrupt generation for the bits in the SLI_MSI_RCV2.
 *
 */
union cvmx_sli_msi_enb2 {
	uint64_t u64;
	struct cvmx_sli_msi_enb2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t enb                          : 64; /**< Enables bit [63:0] of SLI_MSI_RCV2. */
#else
	uint64_t enb                          : 64;
#endif
	} s;
	struct cvmx_sli_msi_enb2_s            cn61xx;
	struct cvmx_sli_msi_enb2_s            cn63xx;
	struct cvmx_sli_msi_enb2_s            cn63xxp1;
	struct cvmx_sli_msi_enb2_s            cn66xx;
	struct cvmx_sli_msi_enb2_s            cn68xx;
	struct cvmx_sli_msi_enb2_s            cn68xxp1;
	struct cvmx_sli_msi_enb2_s            cn70xx;
	struct cvmx_sli_msi_enb2_s            cnf71xx;
};
typedef union cvmx_sli_msi_enb2 cvmx_sli_msi_enb2_t;

/**
 * cvmx_sli_msi_enb3
 *
 * Used to enable the interrupt generation for the bits in the SLI_MSI_RCV3.
 *
 */
union cvmx_sli_msi_enb3 {
	uint64_t u64;
	struct cvmx_sli_msi_enb3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t enb                          : 64; /**< Enables bit [63:0] of SLI_MSI_RCV3. */
#else
	uint64_t enb                          : 64;
#endif
	} s;
	struct cvmx_sli_msi_enb3_s            cn61xx;
	struct cvmx_sli_msi_enb3_s            cn63xx;
	struct cvmx_sli_msi_enb3_s            cn63xxp1;
	struct cvmx_sli_msi_enb3_s            cn66xx;
	struct cvmx_sli_msi_enb3_s            cn68xx;
	struct cvmx_sli_msi_enb3_s            cn68xxp1;
	struct cvmx_sli_msi_enb3_s            cn70xx;
	struct cvmx_sli_msi_enb3_s            cnf71xx;
};
typedef union cvmx_sli_msi_enb3 cvmx_sli_msi_enb3_t;

/**
 * cvmx_sli_msi_rcv0
 *
 * This register contains bits <63:0> of the 256 bits of MSI interrupts.
 *
 */
union cvmx_sli_msi_rcv0 {
	uint64_t u64;
	struct cvmx_sli_msi_rcv0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t intr                         : 64; /**< Bits 63-0 of the 256 bits of MSI interrupt. */
#else
	uint64_t intr                         : 64;
#endif
	} s;
	struct cvmx_sli_msi_rcv0_s            cn61xx;
	struct cvmx_sli_msi_rcv0_s            cn63xx;
	struct cvmx_sli_msi_rcv0_s            cn63xxp1;
	struct cvmx_sli_msi_rcv0_s            cn66xx;
	struct cvmx_sli_msi_rcv0_s            cn68xx;
	struct cvmx_sli_msi_rcv0_s            cn68xxp1;
	struct cvmx_sli_msi_rcv0_s            cn70xx;
	struct cvmx_sli_msi_rcv0_s            cn78xx;
	struct cvmx_sli_msi_rcv0_s            cnf71xx;
};
typedef union cvmx_sli_msi_rcv0 cvmx_sli_msi_rcv0_t;

/**
 * cvmx_sli_msi_rcv1
 *
 * This register contains bits <127:64> of the 256 bits of MSI interrupts.
 *
 */
union cvmx_sli_msi_rcv1 {
	uint64_t u64;
	struct cvmx_sli_msi_rcv1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t intr                         : 64; /**< Bits 127-64 of the 256 bits of MSI interrupt. */
#else
	uint64_t intr                         : 64;
#endif
	} s;
	struct cvmx_sli_msi_rcv1_s            cn61xx;
	struct cvmx_sli_msi_rcv1_s            cn63xx;
	struct cvmx_sli_msi_rcv1_s            cn63xxp1;
	struct cvmx_sli_msi_rcv1_s            cn66xx;
	struct cvmx_sli_msi_rcv1_s            cn68xx;
	struct cvmx_sli_msi_rcv1_s            cn68xxp1;
	struct cvmx_sli_msi_rcv1_s            cn70xx;
	struct cvmx_sli_msi_rcv1_s            cn78xx;
	struct cvmx_sli_msi_rcv1_s            cnf71xx;
};
typedef union cvmx_sli_msi_rcv1 cvmx_sli_msi_rcv1_t;

/**
 * cvmx_sli_msi_rcv2
 *
 * This register contains bits <191:128> of the 256 bits of MSI interrupts.
 *
 */
union cvmx_sli_msi_rcv2 {
	uint64_t u64;
	struct cvmx_sli_msi_rcv2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t intr                         : 64; /**< Bits 191-128 of the 256 bits of MSI interrupt. */
#else
	uint64_t intr                         : 64;
#endif
	} s;
	struct cvmx_sli_msi_rcv2_s            cn61xx;
	struct cvmx_sli_msi_rcv2_s            cn63xx;
	struct cvmx_sli_msi_rcv2_s            cn63xxp1;
	struct cvmx_sli_msi_rcv2_s            cn66xx;
	struct cvmx_sli_msi_rcv2_s            cn68xx;
	struct cvmx_sli_msi_rcv2_s            cn68xxp1;
	struct cvmx_sli_msi_rcv2_s            cn70xx;
	struct cvmx_sli_msi_rcv2_s            cn78xx;
	struct cvmx_sli_msi_rcv2_s            cnf71xx;
};
typedef union cvmx_sli_msi_rcv2 cvmx_sli_msi_rcv2_t;

/**
 * cvmx_sli_msi_rcv3
 *
 * This register contains bits <255:192> of the 256 bits of MSI interrupts.
 *
 */
union cvmx_sli_msi_rcv3 {
	uint64_t u64;
	struct cvmx_sli_msi_rcv3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t intr                         : 64; /**< Bits 255-192 of the 256 bits of MSI interrupt. */
#else
	uint64_t intr                         : 64;
#endif
	} s;
	struct cvmx_sli_msi_rcv3_s            cn61xx;
	struct cvmx_sli_msi_rcv3_s            cn63xx;
	struct cvmx_sli_msi_rcv3_s            cn63xxp1;
	struct cvmx_sli_msi_rcv3_s            cn66xx;
	struct cvmx_sli_msi_rcv3_s            cn68xx;
	struct cvmx_sli_msi_rcv3_s            cn68xxp1;
	struct cvmx_sli_msi_rcv3_s            cn70xx;
	struct cvmx_sli_msi_rcv3_s            cn78xx;
	struct cvmx_sli_msi_rcv3_s            cnf71xx;
};
typedef union cvmx_sli_msi_rcv3 cvmx_sli_msi_rcv3_t;

/**
 * cvmx_sli_msi_rd_map
 *
 * This register is used to read the mapping function of the SLI_PCIE_MSI_RCV to SLI_MSI_RCV
 * registers.
 */
union cvmx_sli_msi_rd_map {
	uint64_t u64;
	struct cvmx_sli_msi_rd_map_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t rd_int                       : 8;  /**< The value of the map at the location PREVIOUSLY
                                                         written to the MSI_INT field of this register. */
	uint64_t msi_int                      : 8;  /**< Selects the value that would be received when the
                                                         SLI_PCIE_MSI_RCV register is written. */
#else
	uint64_t msi_int                      : 8;
	uint64_t rd_int                       : 8;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_sli_msi_rd_map_s          cn61xx;
	struct cvmx_sli_msi_rd_map_s          cn63xx;
	struct cvmx_sli_msi_rd_map_s          cn63xxp1;
	struct cvmx_sli_msi_rd_map_s          cn66xx;
	struct cvmx_sli_msi_rd_map_s          cn68xx;
	struct cvmx_sli_msi_rd_map_s          cn68xxp1;
	struct cvmx_sli_msi_rd_map_s          cn70xx;
	struct cvmx_sli_msi_rd_map_s          cn78xx;
	struct cvmx_sli_msi_rd_map_s          cnf71xx;
};
typedef union cvmx_sli_msi_rd_map cvmx_sli_msi_rd_map_t;

/**
 * cvmx_sli_msi_w1c_enb0
 *
 * Used to clear bits in SLI_MSI_ENB0.
 *
 */
union cvmx_sli_msi_w1c_enb0 {
	uint64_t u64;
	struct cvmx_sli_msi_w1c_enb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t clr                          : 64; /**< A write of '1' to a vector will clear the
                                                         cooresponding bit in SLI_MSI_ENB0.
                                                         A read to this address will return 0. */
#else
	uint64_t clr                          : 64;
#endif
	} s;
	struct cvmx_sli_msi_w1c_enb0_s        cn61xx;
	struct cvmx_sli_msi_w1c_enb0_s        cn63xx;
	struct cvmx_sli_msi_w1c_enb0_s        cn63xxp1;
	struct cvmx_sli_msi_w1c_enb0_s        cn66xx;
	struct cvmx_sli_msi_w1c_enb0_s        cn68xx;
	struct cvmx_sli_msi_w1c_enb0_s        cn68xxp1;
	struct cvmx_sli_msi_w1c_enb0_s        cn70xx;
	struct cvmx_sli_msi_w1c_enb0_s        cnf71xx;
};
typedef union cvmx_sli_msi_w1c_enb0 cvmx_sli_msi_w1c_enb0_t;

/**
 * cvmx_sli_msi_w1c_enb1
 *
 * Used to clear bits in SLI_MSI_ENB1.
 *
 */
union cvmx_sli_msi_w1c_enb1 {
	uint64_t u64;
	struct cvmx_sli_msi_w1c_enb1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t clr                          : 64; /**< A write of '1' to a vector will clear the
                                                         cooresponding bit in SLI_MSI_ENB1.
                                                         A read to this address will return 0. */
#else
	uint64_t clr                          : 64;
#endif
	} s;
	struct cvmx_sli_msi_w1c_enb1_s        cn61xx;
	struct cvmx_sli_msi_w1c_enb1_s        cn63xx;
	struct cvmx_sli_msi_w1c_enb1_s        cn63xxp1;
	struct cvmx_sli_msi_w1c_enb1_s        cn66xx;
	struct cvmx_sli_msi_w1c_enb1_s        cn68xx;
	struct cvmx_sli_msi_w1c_enb1_s        cn68xxp1;
	struct cvmx_sli_msi_w1c_enb1_s        cn70xx;
	struct cvmx_sli_msi_w1c_enb1_s        cnf71xx;
};
typedef union cvmx_sli_msi_w1c_enb1 cvmx_sli_msi_w1c_enb1_t;

/**
 * cvmx_sli_msi_w1c_enb2
 *
 * Used to clear bits in SLI_MSI_ENB2.
 *
 */
union cvmx_sli_msi_w1c_enb2 {
	uint64_t u64;
	struct cvmx_sli_msi_w1c_enb2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t clr                          : 64; /**< A write of '1' to a vector will clear the
                                                         cooresponding bit in SLI_MSI_ENB2.
                                                         A read to this address will return 0. */
#else
	uint64_t clr                          : 64;
#endif
	} s;
	struct cvmx_sli_msi_w1c_enb2_s        cn61xx;
	struct cvmx_sli_msi_w1c_enb2_s        cn63xx;
	struct cvmx_sli_msi_w1c_enb2_s        cn63xxp1;
	struct cvmx_sli_msi_w1c_enb2_s        cn66xx;
	struct cvmx_sli_msi_w1c_enb2_s        cn68xx;
	struct cvmx_sli_msi_w1c_enb2_s        cn68xxp1;
	struct cvmx_sli_msi_w1c_enb2_s        cn70xx;
	struct cvmx_sli_msi_w1c_enb2_s        cnf71xx;
};
typedef union cvmx_sli_msi_w1c_enb2 cvmx_sli_msi_w1c_enb2_t;

/**
 * cvmx_sli_msi_w1c_enb3
 *
 * Used to clear bits in SLI_MSI_ENB3.
 *
 */
union cvmx_sli_msi_w1c_enb3 {
	uint64_t u64;
	struct cvmx_sli_msi_w1c_enb3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t clr                          : 64; /**< A write of '1' to a vector will clear the
                                                         cooresponding bit in SLI_MSI_ENB3.
                                                         A read to this address will return 0. */
#else
	uint64_t clr                          : 64;
#endif
	} s;
	struct cvmx_sli_msi_w1c_enb3_s        cn61xx;
	struct cvmx_sli_msi_w1c_enb3_s        cn63xx;
	struct cvmx_sli_msi_w1c_enb3_s        cn63xxp1;
	struct cvmx_sli_msi_w1c_enb3_s        cn66xx;
	struct cvmx_sli_msi_w1c_enb3_s        cn68xx;
	struct cvmx_sli_msi_w1c_enb3_s        cn68xxp1;
	struct cvmx_sli_msi_w1c_enb3_s        cn70xx;
	struct cvmx_sli_msi_w1c_enb3_s        cnf71xx;
};
typedef union cvmx_sli_msi_w1c_enb3 cvmx_sli_msi_w1c_enb3_t;

/**
 * cvmx_sli_msi_w1s_enb0
 *
 * Used to set bits in SLI_MSI_ENB0.
 *
 */
union cvmx_sli_msi_w1s_enb0 {
	uint64_t u64;
	struct cvmx_sli_msi_w1s_enb0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t set                          : 64; /**< A write of '1' to a vector will set the
                                                         cooresponding bit in SLI_MSI_ENB0.
                                                         A read to this address will return 0. */
#else
	uint64_t set                          : 64;
#endif
	} s;
	struct cvmx_sli_msi_w1s_enb0_s        cn61xx;
	struct cvmx_sli_msi_w1s_enb0_s        cn63xx;
	struct cvmx_sli_msi_w1s_enb0_s        cn63xxp1;
	struct cvmx_sli_msi_w1s_enb0_s        cn66xx;
	struct cvmx_sli_msi_w1s_enb0_s        cn68xx;
	struct cvmx_sli_msi_w1s_enb0_s        cn68xxp1;
	struct cvmx_sli_msi_w1s_enb0_s        cn70xx;
	struct cvmx_sli_msi_w1s_enb0_s        cnf71xx;
};
typedef union cvmx_sli_msi_w1s_enb0 cvmx_sli_msi_w1s_enb0_t;

/**
 * cvmx_sli_msi_w1s_enb1
 *
 * SLI_MSI_W1S_ENB0 = SLI MSI Write 1 To Set Enable1
 * Used to set bits in SLI_MSI_ENB1.
 */
union cvmx_sli_msi_w1s_enb1 {
	uint64_t u64;
	struct cvmx_sli_msi_w1s_enb1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t set                          : 64; /**< A write of '1' to a vector will set the
                                                         cooresponding bit in SLI_MSI_ENB1.
                                                         A read to this address will return 0. */
#else
	uint64_t set                          : 64;
#endif
	} s;
	struct cvmx_sli_msi_w1s_enb1_s        cn61xx;
	struct cvmx_sli_msi_w1s_enb1_s        cn63xx;
	struct cvmx_sli_msi_w1s_enb1_s        cn63xxp1;
	struct cvmx_sli_msi_w1s_enb1_s        cn66xx;
	struct cvmx_sli_msi_w1s_enb1_s        cn68xx;
	struct cvmx_sli_msi_w1s_enb1_s        cn68xxp1;
	struct cvmx_sli_msi_w1s_enb1_s        cn70xx;
	struct cvmx_sli_msi_w1s_enb1_s        cnf71xx;
};
typedef union cvmx_sli_msi_w1s_enb1 cvmx_sli_msi_w1s_enb1_t;

/**
 * cvmx_sli_msi_w1s_enb2
 *
 * Used to set bits in SLI_MSI_ENB2.
 *
 */
union cvmx_sli_msi_w1s_enb2 {
	uint64_t u64;
	struct cvmx_sli_msi_w1s_enb2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t set                          : 64; /**< A write of '1' to a vector will set the
                                                         cooresponding bit in SLI_MSI_ENB2.
                                                         A read to this address will return 0. */
#else
	uint64_t set                          : 64;
#endif
	} s;
	struct cvmx_sli_msi_w1s_enb2_s        cn61xx;
	struct cvmx_sli_msi_w1s_enb2_s        cn63xx;
	struct cvmx_sli_msi_w1s_enb2_s        cn63xxp1;
	struct cvmx_sli_msi_w1s_enb2_s        cn66xx;
	struct cvmx_sli_msi_w1s_enb2_s        cn68xx;
	struct cvmx_sli_msi_w1s_enb2_s        cn68xxp1;
	struct cvmx_sli_msi_w1s_enb2_s        cn70xx;
	struct cvmx_sli_msi_w1s_enb2_s        cnf71xx;
};
typedef union cvmx_sli_msi_w1s_enb2 cvmx_sli_msi_w1s_enb2_t;

/**
 * cvmx_sli_msi_w1s_enb3
 *
 * Used to set bits in SLI_MSI_ENB3.
 *
 */
union cvmx_sli_msi_w1s_enb3 {
	uint64_t u64;
	struct cvmx_sli_msi_w1s_enb3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t set                          : 64; /**< A write of '1' to a vector will set the
                                                         cooresponding bit in SLI_MSI_ENB3.
                                                         A read to this address will return 0. */
#else
	uint64_t set                          : 64;
#endif
	} s;
	struct cvmx_sli_msi_w1s_enb3_s        cn61xx;
	struct cvmx_sli_msi_w1s_enb3_s        cn63xx;
	struct cvmx_sli_msi_w1s_enb3_s        cn63xxp1;
	struct cvmx_sli_msi_w1s_enb3_s        cn66xx;
	struct cvmx_sli_msi_w1s_enb3_s        cn68xx;
	struct cvmx_sli_msi_w1s_enb3_s        cn68xxp1;
	struct cvmx_sli_msi_w1s_enb3_s        cn70xx;
	struct cvmx_sli_msi_w1s_enb3_s        cnf71xx;
};
typedef union cvmx_sli_msi_w1s_enb3 cvmx_sli_msi_w1s_enb3_t;

/**
 * cvmx_sli_msi_wr_map
 *
 * This register is used to write the mapping function of the SLI_PCIE_MSI_RCV to SLI_MSI_RCV
 * registers. At reset, the mapping function is one-to-one, that is MSI_INT 1 maps to CIU_INT 1,
 * 2 to 2, 3 to 3, etc.
 */
union cvmx_sli_msi_wr_map {
	uint64_t u64;
	struct cvmx_sli_msi_wr_map_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t ciu_int                      : 8;  /**< Selects which bit in the SLI_MSI_RCV# (0-255)
                                                         will be set when the value specified in the
                                                         MSI_INT of this register is recevied during a
                                                         write to the SLI_PCIE_MSI_RCV register. */
	uint64_t msi_int                      : 8;  /**< Selects the value that would be received when the
                                                         SLI_PCIE_MSI_RCV register is written. */
#else
	uint64_t msi_int                      : 8;
	uint64_t ciu_int                      : 8;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_sli_msi_wr_map_s          cn61xx;
	struct cvmx_sli_msi_wr_map_s          cn63xx;
	struct cvmx_sli_msi_wr_map_s          cn63xxp1;
	struct cvmx_sli_msi_wr_map_s          cn66xx;
	struct cvmx_sli_msi_wr_map_s          cn68xx;
	struct cvmx_sli_msi_wr_map_s          cn68xxp1;
	struct cvmx_sli_msi_wr_map_s          cn70xx;
	struct cvmx_sli_msi_wr_map_s          cn78xx;
	struct cvmx_sli_msi_wr_map_s          cnf71xx;
};
typedef union cvmx_sli_msi_wr_map cvmx_sli_msi_wr_map_t;

/**
 * cvmx_sli_msix#_table_addr
 *
 * "The MSI-X table must be addressed on a 8-byte aligned boundary and cannot be burst read or
 * written.
 *
 * The MSI-X Table is 65 entries deep. Each MAC can see up to 64 VF ring entries and its own
 * PF entry.
 *
 * The first 64 entries contain MSI-X vectors for each of the 64 DPI Packet rings. Entries, or
 * rings, are assigned to PEM0 and/or PEM2 based on the SRN, RPVF, and TNR registers.  Each VF
 * has access to only its own entries and therefore will see the MSI-X Table as smaller than 64
 * entries, unless all 64 rings are assigned to it. A VF must not try to configure more entries
 * than it owns. A VF will always see its first entry as entry 0. The actual MSI-X Table Offset
 * is calculated as follows;   (SRN + ((VF#-1) * RPVF)).
 *
 *           RPVF - Rings VF believes it owns - Max \# VFs
 *             1        0                          64
 *             2        0,1                        32
 *             4        0,1,2,3                    16
 *             8        0,1,2,3,4,5,6,7             8
 *            16        0,1,2,...13,14,15           4
 *            32        0,1,2,...29,30,31           2
 *            64        0,1,2,...61,62,63           1
 *
 * A MAC's PF can access the entries for its VF (a total of TNR, regardless of VF Mode).
 *
 * The last (i.e. 65th) entry is for PCIe related errors and is only accessible by the
 * PF. There are actually two entries at the 65th location; one for each MAC's PF. The hardware
 * will enable writing to and reading from the appropriate entry based on the pcsr_src vector.
 *
 * In PF Mode there is no virtual function support, but a PF can configure up to 65 entries
 * (up to 64 VF rings + 1 PF ring) for itself."
 */
union cvmx_sli_msixx_table_addr {
	uint64_t u64;
	struct cvmx_sli_msixx_table_addr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t addr                         : 64; /**< Message address[63:0]. */
#else
	uint64_t addr                         : 64;
#endif
	} s;
	struct cvmx_sli_msixx_table_addr_s    cn78xx;
};
typedef union cvmx_sli_msixx_table_addr cvmx_sli_msixx_table_addr_t;

/**
 * cvmx_sli_msix#_table_data
 *
 * The MSI-X table must be addressed on a 8-byte aligned boundary and cannot be burst read or
 * written. F/PF access is the same as described for the SLI_MSIX(0..64)_TABLE_ADDR.
 */
union cvmx_sli_msixx_table_data {
	uint64_t u64;
	struct cvmx_sli_msixx_table_data_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_33_63               : 31;
	uint64_t vector_ctl                   : 1;  /**< Message mask. */
	uint64_t data                         : 32; /**< Message data[31:0]. */
#else
	uint64_t data                         : 32;
	uint64_t vector_ctl                   : 1;
	uint64_t reserved_33_63               : 31;
#endif
	} s;
	struct cvmx_sli_msixx_table_data_s    cn78xx;
};
typedef union cvmx_sli_msixx_table_data cvmx_sli_msixx_table_data_t;

/**
 * cvmx_sli_msix_pba0
 *
 * "The MSI-X Pending Bit Array must be addressed on a 8-byte aligned boundary and
 * cannot be burst read.
 * In VF Mode, a PF will find its error interrupt in bit position 0. Bits [63:1]
 * are returned as zero.
 *
 * A VF will find its pending completion interrupts (one for each of the configured
 * (up to 64) DPI Packet Rings) in bit positions [(RPVF-1):0]. If RPVF<64, bits [63:RPVF]
 * are returned as zero.
 *
 * Each VF can read their own pending completion interrupts based on the ring/VF
 * configuration. Therefore, a VF sees the PBA as smaller than what is shown below
 * (unless it owns all 64 entries).  Unassigned bits will return zeros.
 *           RPVF      Interrupts per VF       Pending bits returned
 *           ----      -----------------       ---------------------
 *             1                1               MSG_PND0
 *             2                2               MSG_PND1  - MSG_PND0
 *             4                4               MSG_PND3  - MSG_PND0
 *             8                8               MSG_PND7  - MSG_PND0
 *            16               16               MSG_PND15 - MSG_PND0
 *            32               32               MSG_PND31 - MSG_PND0
 *            64               64               MSG_PND63 - MSG_PND0
 *
 * In PF Mode there is no virtual function support, but the PF can configure up to 65
 * entries (up to 64 DPI Packet Rings plus 1 PF ring) for itself.
 *
 * In PF Mode, if SLI_PEM#_TNR=63 (i.e. 64 total DPI Packet Rings configured), a PF will
 * find its pending completion interrupts in bit positions [63:0]. When SLI_PEM#_TNR=63,
 * the PF will find its PCIe error interrupt in SLI_MSIX_PBA1, bit position 0.
 *
 * If SLI_PEM#_TNR<63 (i.e. 1, 2, 4, 8, 16, or 32 rings configured), a PF will find its
 * ring pending completion interrupts in bit positions [TNR:0]. It will find its PCIe
 * error interrupt in bit position [(TNR+1)]. Bits [63:(TNR+2)] are returned as zero.
 * When SLI_PEM#_TNR<63 in PF Mode, SLI_MSIX_PBA1 is not used and returns zeros."
 */
union cvmx_sli_msix_pba0 {
	uint64_t u64;
	struct cvmx_sli_msix_pba0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t msg_pnd                      : 64; /**< VF message pending[63:0] vector. */
#else
	uint64_t msg_pnd                      : 64;
#endif
	} s;
	struct cvmx_sli_msix_pba0_s           cn78xx;
};
typedef union cvmx_sli_msix_pba0 cvmx_sli_msix_pba0_t;

/**
 * cvmx_sli_msix_pba1
 *
 * "The MSI-X pending bit array must be addressed on a 8-byte aligned boundary and cannot be
 * burst read.
 * PF_PND is assigned to PCIe related errors. The error bit is only be found in PBA1 when the MAC
 * is in PF Mode (i.e. no virtualization) and SLI_PEM#_TNR=63 (i.e. 64 total DPI Packet Rings
 * configured).
 * This register is accessible by the PF; a VF read returns zeros. A read by a particular PF only
 * returns its own pending status. That is, both PFs read this register, but the hardware ensures
 * that the PF only sees its own status."
 */
union cvmx_sli_msix_pba1 {
	uint64_t u64;
	struct cvmx_sli_msix_pba1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t pf_pnd                       : 1;  /**< PF message pending. */
#else
	uint64_t pf_pnd                       : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_sli_msix_pba1_s           cn78xx;
};
typedef union cvmx_sli_msix_pba1 cvmx_sli_msix_pba1_t;

/**
 * cvmx_sli_pcie_msi_rcv
 *
 * This is the register where MSI write operations are directed from the MAC.
 *
 */
union cvmx_sli_pcie_msi_rcv {
	uint64_t u64;
	struct cvmx_sli_pcie_msi_rcv_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t intr                         : 8;  /**< A write to this register will result in a bit in
                                                         one of the SLI_MSI_RCV# registers being set.
                                                         Which bit is set is dependent on the previously
                                                         written using the SLI_MSI_WR_MAP register or if
                                                         not previously written the reset value of the MAP. */
#else
	uint64_t intr                         : 8;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_sli_pcie_msi_rcv_s        cn61xx;
	struct cvmx_sli_pcie_msi_rcv_s        cn63xx;
	struct cvmx_sli_pcie_msi_rcv_s        cn63xxp1;
	struct cvmx_sli_pcie_msi_rcv_s        cn66xx;
	struct cvmx_sli_pcie_msi_rcv_s        cn68xx;
	struct cvmx_sli_pcie_msi_rcv_s        cn68xxp1;
	struct cvmx_sli_pcie_msi_rcv_s        cn70xx;
	struct cvmx_sli_pcie_msi_rcv_s        cn78xx;
	struct cvmx_sli_pcie_msi_rcv_s        cnf71xx;
};
typedef union cvmx_sli_pcie_msi_rcv cvmx_sli_pcie_msi_rcv_t;

/**
 * cvmx_sli_pcie_msi_rcv_b1
 *
 * This register is where MSI write operations are directed from the MAC. This CSR can be used by
 * the PCIe MACs.
 */
union cvmx_sli_pcie_msi_rcv_b1 {
	uint64_t u64;
	struct cvmx_sli_pcie_msi_rcv_b1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t intr                         : 8;  /**< A write to this register will result in a bit in
                                                         one of the SLI_MSI_RCV# registers being set.
                                                         Which bit is set is dependent on the previously
                                                         written using the SLI_MSI_WR_MAP register or if
                                                         not previously written the reset value of the MAP. */
	uint64_t reserved_0_7                 : 8;
#else
	uint64_t reserved_0_7                 : 8;
	uint64_t intr                         : 8;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_sli_pcie_msi_rcv_b1_s     cn61xx;
	struct cvmx_sli_pcie_msi_rcv_b1_s     cn63xx;
	struct cvmx_sli_pcie_msi_rcv_b1_s     cn63xxp1;
	struct cvmx_sli_pcie_msi_rcv_b1_s     cn66xx;
	struct cvmx_sli_pcie_msi_rcv_b1_s     cn68xx;
	struct cvmx_sli_pcie_msi_rcv_b1_s     cn68xxp1;
	struct cvmx_sli_pcie_msi_rcv_b1_s     cn70xx;
	struct cvmx_sli_pcie_msi_rcv_b1_s     cn78xx;
	struct cvmx_sli_pcie_msi_rcv_b1_s     cnf71xx;
};
typedef union cvmx_sli_pcie_msi_rcv_b1 cvmx_sli_pcie_msi_rcv_b1_t;

/**
 * cvmx_sli_pcie_msi_rcv_b2
 *
 * This register is where MSI write operations are directed from the MAC.  This CSR can be used
 * by PCIe MACs.
 */
union cvmx_sli_pcie_msi_rcv_b2 {
	uint64_t u64;
	struct cvmx_sli_pcie_msi_rcv_b2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t intr                         : 8;  /**< A write to this register will result in a bit in
                                                         one of the SLI_MSI_RCV# registers being set.
                                                         Which bit is set is dependent on the previously
                                                         written using the SLI_MSI_WR_MAP register or if
                                                         not previously written the reset value of the MAP. */
	uint64_t reserved_0_15                : 16;
#else
	uint64_t reserved_0_15                : 16;
	uint64_t intr                         : 8;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_sli_pcie_msi_rcv_b2_s     cn61xx;
	struct cvmx_sli_pcie_msi_rcv_b2_s     cn63xx;
	struct cvmx_sli_pcie_msi_rcv_b2_s     cn63xxp1;
	struct cvmx_sli_pcie_msi_rcv_b2_s     cn66xx;
	struct cvmx_sli_pcie_msi_rcv_b2_s     cn68xx;
	struct cvmx_sli_pcie_msi_rcv_b2_s     cn68xxp1;
	struct cvmx_sli_pcie_msi_rcv_b2_s     cn70xx;
	struct cvmx_sli_pcie_msi_rcv_b2_s     cn78xx;
	struct cvmx_sli_pcie_msi_rcv_b2_s     cnf71xx;
};
typedef union cvmx_sli_pcie_msi_rcv_b2 cvmx_sli_pcie_msi_rcv_b2_t;

/**
 * cvmx_sli_pcie_msi_rcv_b3
 *
 * This register is where MSI write operations are directed from the MAC. This CSR can be used by
 * PCIe MACs.
 */
union cvmx_sli_pcie_msi_rcv_b3 {
	uint64_t u64;
	struct cvmx_sli_pcie_msi_rcv_b3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t intr                         : 8;  /**< A write to this register will result in a bit in
                                                         one of the SLI_MSI_RCV# registers being set.
                                                         Which bit is set is dependent on the previously
                                                         written using the SLI_MSI_WR_MAP register or if
                                                         not previously written the reset value of the MAP. */
	uint64_t reserved_0_23                : 24;
#else
	uint64_t reserved_0_23                : 24;
	uint64_t intr                         : 8;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_pcie_msi_rcv_b3_s     cn61xx;
	struct cvmx_sli_pcie_msi_rcv_b3_s     cn63xx;
	struct cvmx_sli_pcie_msi_rcv_b3_s     cn63xxp1;
	struct cvmx_sli_pcie_msi_rcv_b3_s     cn66xx;
	struct cvmx_sli_pcie_msi_rcv_b3_s     cn68xx;
	struct cvmx_sli_pcie_msi_rcv_b3_s     cn68xxp1;
	struct cvmx_sli_pcie_msi_rcv_b3_s     cn70xx;
	struct cvmx_sli_pcie_msi_rcv_b3_s     cn78xx;
	struct cvmx_sli_pcie_msi_rcv_b3_s     cnf71xx;
};
typedef union cvmx_sli_pcie_msi_rcv_b3 cvmx_sli_pcie_msi_rcv_b3_t;

/**
 * cvmx_sli_pkt#_cnts
 *
 * This register contains the counters for output rings.
 *
 */
union cvmx_sli_pktx_cnts {
	uint64_t u64;
	struct cvmx_sli_pktx_cnts_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t po_int                       : 1;  /**< "Returns a 1 when either the corresponding bit in SLI_PKT_TIME_INT[RING[\#]] or
                                                         SLI_PKT_CNT_INT[RING[\#]] is set." */
	uint64_t pi_int                       : 1;  /**< Returns a 1 when corresponding bit of SLI_PKT_IN_INT[RING[\#]] is set. */
	uint64_t reserved_54_61               : 8;
	uint64_t timer                        : 22; /**< Timer incremented every 1024 core clocks
                                                         when SLI_PKTS#_CNTS[CNT] is non zero. Field
                                                         cleared when SLI_PKTS#_CNTS[CNT] goes to 0.
                                                         Field is also cleared when SLI_PKT_TIME_INT is
                                                         cleared.
                                                         The first increment of this count can occur
                                                         between 0 to 1023 core clocks. */
	uint64_t cnt                          : 32; /**< ring counter. This field is incremented as
                                                         packets are sent out and decremented in response to
                                                         writes to this field.
                                                         When SLI_PKT_OUT_BMODE is '0' a value of 1 is
                                                         added to the register for each packet, when '1'
                                                         and the info-pointer is NOT used the length of the
                                                         packet plus 8 is added, when '1' and info-pointer
                                                         mode IS used the packet length is added to this
                                                         field. */
#else
	uint64_t cnt                          : 32;
	uint64_t timer                        : 22;
	uint64_t reserved_54_61               : 8;
	uint64_t pi_int                       : 1;
	uint64_t po_int                       : 1;
#endif
	} s;
	struct cvmx_sli_pktx_cnts_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_54_63               : 10;
	uint64_t timer                        : 22; /**< Timer incremented every 1024 core clocks
                                                         when SLI_PKTS#_CNTS[CNT] is non zero. Field
                                                         cleared when SLI_PKTS#_CNTS[CNT] goes to 0.
                                                         Field is also cleared when SLI_PKT_TIME_INT is
                                                         cleared.
                                                         The first increment of this count can occur
                                                         between 0 to 1023 core clocks. */
	uint64_t cnt                          : 32; /**< ring counter. This field is incremented as
                                                         packets are sent out and decremented in response to
                                                         writes to this field.
                                                         When SLI_PKT_OUT_BMODE is '0' a value of 1 is
                                                         added to the register for each packet, when '1'
                                                         and the info-pointer is NOT used the length of the
                                                         packet plus 8 is added, when '1' and info-pointer
                                                         mode IS used the packet length is added to this
                                                         field. */
#else
	uint64_t cnt                          : 32;
	uint64_t timer                        : 22;
	uint64_t reserved_54_63               : 10;
#endif
	} cn61xx;
	struct cvmx_sli_pktx_cnts_cn61xx      cn63xx;
	struct cvmx_sli_pktx_cnts_cn61xx      cn63xxp1;
	struct cvmx_sli_pktx_cnts_cn61xx      cn66xx;
	struct cvmx_sli_pktx_cnts_cn61xx      cn68xx;
	struct cvmx_sli_pktx_cnts_cn61xx      cn68xxp1;
	struct cvmx_sli_pktx_cnts_cn61xx      cn70xx;
	struct cvmx_sli_pktx_cnts_s           cn78xx;
	struct cvmx_sli_pktx_cnts_cn61xx      cnf71xx;
};
typedef union cvmx_sli_pktx_cnts cvmx_sli_pktx_cnts_t;

/**
 * cvmx_sli_pkt#_in_bp
 *
 * "SLI_PKT[0..31]_IN_BP = SLI Packet ring# Input Backpressure
 * The counters and thresholds for input packets to apply backpressure to processing of the
 * packets."
 */
union cvmx_sli_pktx_in_bp {
	uint64_t u64;
	struct cvmx_sli_pktx_in_bp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t wmark                        : 32; /**< When CNT is greater than this threshold no more
                                                         packets will be processed for this ring.
                                                         When writing this field of the SLI_PKT#_IN_BP
                                                         register, use a 4-byte write so as to not write
                                                         any other field of this register. */
	uint64_t cnt                          : 32; /**< ring counter. This field is incremented by one
                                                         whenever OCTEON receives, buffers, and creates a
                                                         work queue entry for a packet that arrives by the
                                                         cooresponding input ring. A write to this field
                                                         will be subtracted from the field value.
                                                         When writing this field of the SLI_PKT#_IN_BP
                                                         register, use a 4-byte write so as to not write
                                                         any other field of this register. */
#else
	uint64_t cnt                          : 32;
	uint64_t wmark                        : 32;
#endif
	} s;
	struct cvmx_sli_pktx_in_bp_s          cn61xx;
	struct cvmx_sli_pktx_in_bp_s          cn63xx;
	struct cvmx_sli_pktx_in_bp_s          cn63xxp1;
	struct cvmx_sli_pktx_in_bp_s          cn66xx;
	struct cvmx_sli_pktx_in_bp_s          cn70xx;
	struct cvmx_sli_pktx_in_bp_s          cnf71xx;
};
typedef union cvmx_sli_pktx_in_bp cvmx_sli_pktx_in_bp_t;

/**
 * cvmx_sli_pkt#_input_control
 *
 * This register is the control for read operations for gather list and instructions.
 *
 */
union cvmx_sli_pktx_input_control {
	uint64_t u64;
	struct cvmx_sli_pktx_input_control_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_39_63               : 25;
	uint64_t vf_num                       : 7;  /**< The VF number that this ring belongs to. A value of 0 means that this ring belongs to the
                                                         PF. The VF number is the value sent to and received from the MAC. This means that if ring
                                                         x belongs to MAC0 and ring y belongs to MAC1, they both could have a VF_NUM of 1. Legal
                                                         value are 0-64. */
	uint64_t reserved_31_31               : 1;
	uint64_t mac_num                      : 2;  /**< The MAC that the ring belongs to. Legal value are 0-3. */
	uint64_t reserved_27_28               : 2;
	uint64_t rdsize                       : 2;  /**< Number of instructions to be read in one MAC read request for the 4 ports, 16 rings. Two
                                                         bit value are:
                                                         0x0: 1 Instruction
                                                         0x1: 2 Instructions
                                                         0x2: 3 Instructions
                                                         0x3: 4 Instructions */
	uint64_t is_64b                       : 1;  /**< When IS_64B=1, instruction input ring i uses 64B. */
	uint64_t rst                          : 1;  /**< Packet reset. This bit is set for a ring when the ring enters the reset state. This can be
                                                         done by writing a 1 to the field, when a FLR associated with the ring occurs, or when an
                                                         error response is received for a read done by the ring. See SLI_INT_SUM[PGL_ERR]. When
                                                         receiving a PGL_ERR interrupt, software should:
                                                         1. Wait 2ms to allow any outstanding reads to return or be timed out.
                                                         2. Write a 0 to this bit.
                                                         3. Start up the packet input/output again (all previous CSR setting of the packet-
                                                         input/output will be lost).
                                                         See also SLI_PKT_RING_RST[RST]. */
	uint64_t enb                          : 1;  /**< When ENB=1, instruction input ring i is enabled. */
	uint64_t pbp_dhi                      : 13; /**< PBP_DHI replaces address bits that are used
                                                         for parse mode and skip-length when
                                                         SLI_PKTi_INSTR_HEADER[PBP]=1.
                                                         PBP_DHI becomes either MACADD<63:55> or MACADD<59:51>
                                                         for the instruction DPTR reads in this case.
                                                         The instruction DPTR reads are called
                                                         "First Direct" or "First Indirect" in the HRM.
                                                         When PBP=1, if "First Direct" and USE_CSR=0, PBP_DHI
                                                         becomes MACADD<59:51>, else MACADD<63:55>. */
	uint64_t d_nsr                        : 1;  /**< ADDRTYPE<1> or MACADD<61> for packet input data read operations. D_NSR becomes either
                                                         ADDRTYPE<1> or MACADD<61> for MAC memory space read operations of packet input data
                                                         fetched for any packet input ring. ADDRTYPE<1> if USE_CSR = 1, else MACADD<61>. In the
                                                         latter case, ADDRTYPE<1> comes from DPTR<61>. ADDRTYPE<1> is the no-snoop attribute for
                                                         PCIe. */
	uint64_t d_esr                        : 2;  /**< ES<1:0> or MACADD<63:62> for packet input data read operations. D_ESR becomes either
                                                         ES<1:0> or MACADD<63:62> for MAC memory space read operations of packet input data fetched
                                                         for any packet input ring. ES<1:0> if USE_CSR = 1, else MACADD<63:62>. In the latter case,
                                                         ES<1:0> comes from DPTR<63:62>. ES<1:0> is the endian-swap attribute for these MAC memory
                                                         space read operations. */
	uint64_t d_ror                        : 1;  /**< ADDRTYPE<0> or MACADD<60> for packet input data read operations. D_ROR becomes either
                                                         ADDRTYPE<0> or MACADD<60> for MAC memory space read operations of packet input data
                                                         fetched for any packet input ring. ADDRTYPE<0> if USE_CSR = 1, else MACADD<60>. In the
                                                         latter case, ADDRTYPE<0> comes from DPTR<60>. ADDRTYPE<0> is the relaxed-order attribute
                                                         for PCIe. */
	uint64_t use_csr                      : 1;  /**< When set to 1, the CSR value is used for ROR, ESR, and NSR. When clear to 0, the value in
                                                         DPTR is used. In turn, the bits not used for ROR, ESR, and NSR are used for bits [63:60]
                                                         of the address used to fetch packet data. */
	uint64_t nsr                          : 1;  /**< ADDRTYPE<1> for packet input instruction read operations and gather list (i.e. DPI
                                                         component) read operations from MAC memory space. ADDRTYPE<1> is the no-snoop attribute
                                                         for PCIe. */
	uint64_t esr                          : 2;  /**< ES<1:0> for packet input instruction read operations and gather list (i.e. DPI component)
                                                         read operations from MAC memory space. ES<1:0> is the endian-swap attribute for these MAC
                                                         memory space read operations. */
	uint64_t ror                          : 1;  /**< ADDRTYPE<0> for packet input instruction read operations and gather list (i.e. DPI
                                                         component) read operations from MAC memory space. ADDRTYPE<0> is the relaxed-order
                                                         attribute for PCIe. */
#else
	uint64_t ror                          : 1;
	uint64_t esr                          : 2;
	uint64_t nsr                          : 1;
	uint64_t use_csr                      : 1;
	uint64_t d_ror                        : 1;
	uint64_t d_esr                        : 2;
	uint64_t d_nsr                        : 1;
	uint64_t pbp_dhi                      : 13;
	uint64_t enb                          : 1;
	uint64_t rst                          : 1;
	uint64_t is_64b                       : 1;
	uint64_t rdsize                       : 2;
	uint64_t reserved_27_28               : 2;
	uint64_t mac_num                      : 2;
	uint64_t reserved_31_31               : 1;
	uint64_t vf_num                       : 7;
	uint64_t reserved_39_63               : 25;
#endif
	} s;
	struct cvmx_sli_pktx_input_control_s  cn78xx;
};
typedef union cvmx_sli_pktx_input_control cvmx_sli_pktx_input_control_t;

/**
 * cvmx_sli_pkt#_instr_baddr
 *
 * This register contains the start-of-instruction for input packets. The address must be
 * addressed-aligned to the size of the instruction.
 */
union cvmx_sli_pktx_instr_baddr {
	uint64_t u64;
	struct cvmx_sli_pktx_instr_baddr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t addr                         : 61; /**< Base address for Instructions. */
	uint64_t reserved_0_2                 : 3;
#else
	uint64_t reserved_0_2                 : 3;
	uint64_t addr                         : 61;
#endif
	} s;
	struct cvmx_sli_pktx_instr_baddr_s    cn61xx;
	struct cvmx_sli_pktx_instr_baddr_s    cn63xx;
	struct cvmx_sli_pktx_instr_baddr_s    cn63xxp1;
	struct cvmx_sli_pktx_instr_baddr_s    cn66xx;
	struct cvmx_sli_pktx_instr_baddr_s    cn68xx;
	struct cvmx_sli_pktx_instr_baddr_s    cn68xxp1;
	struct cvmx_sli_pktx_instr_baddr_s    cn70xx;
	struct cvmx_sli_pktx_instr_baddr_s    cn78xx;
	struct cvmx_sli_pktx_instr_baddr_s    cnf71xx;
};
typedef union cvmx_sli_pktx_instr_baddr cvmx_sli_pktx_instr_baddr_t;

/**
 * cvmx_sli_pkt#_instr_baoff_dbell
 *
 * This register contains the doorbell and base address offset for the next read.
 *
 */
union cvmx_sli_pktx_instr_baoff_dbell {
	uint64_t u64;
	struct cvmx_sli_pktx_instr_baoff_dbell_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t aoff                         : 32; /**< The offset from the SLI_PKT[0..31]_INSTR_BADDR
                                                         where the next instruction will be read. */
	uint64_t dbell                        : 32; /**< Instruction doorbell count. Writes to this field
                                                         will increment the value here. Reads will return
                                                         present value. A write of 0xffffffff will set the
                                                         DBELL and AOFF fields to '0'. */
#else
	uint64_t dbell                        : 32;
	uint64_t aoff                         : 32;
#endif
	} s;
	struct cvmx_sli_pktx_instr_baoff_dbell_s cn61xx;
	struct cvmx_sli_pktx_instr_baoff_dbell_s cn63xx;
	struct cvmx_sli_pktx_instr_baoff_dbell_s cn63xxp1;
	struct cvmx_sli_pktx_instr_baoff_dbell_s cn66xx;
	struct cvmx_sli_pktx_instr_baoff_dbell_s cn68xx;
	struct cvmx_sli_pktx_instr_baoff_dbell_s cn68xxp1;
	struct cvmx_sli_pktx_instr_baoff_dbell_s cn70xx;
	struct cvmx_sli_pktx_instr_baoff_dbell_s cn78xx;
	struct cvmx_sli_pktx_instr_baoff_dbell_s cnf71xx;
};
typedef union cvmx_sli_pktx_instr_baoff_dbell cvmx_sli_pktx_instr_baoff_dbell_t;

/**
 * cvmx_sli_pkt#_instr_fifo_rsize
 *
 * This register contains the FIFO field and ring size for instructions.
 *
 */
union cvmx_sli_pktx_instr_fifo_rsize {
	uint64_t u64;
	struct cvmx_sli_pktx_instr_fifo_rsize_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t max                          : 9;  /**< Max Fifo Size. */
	uint64_t rrp                          : 9;  /**< Fifo read pointer. */
	uint64_t wrp                          : 9;  /**< Fifo write pointer. */
	uint64_t fcnt                         : 5;  /**< Fifo count. */
	uint64_t rsize                        : 32; /**< Instruction ring size. */
#else
	uint64_t rsize                        : 32;
	uint64_t fcnt                         : 5;
	uint64_t wrp                          : 9;
	uint64_t rrp                          : 9;
	uint64_t max                          : 9;
#endif
	} s;
	struct cvmx_sli_pktx_instr_fifo_rsize_s cn61xx;
	struct cvmx_sli_pktx_instr_fifo_rsize_s cn63xx;
	struct cvmx_sli_pktx_instr_fifo_rsize_s cn63xxp1;
	struct cvmx_sli_pktx_instr_fifo_rsize_s cn66xx;
	struct cvmx_sli_pktx_instr_fifo_rsize_s cn68xx;
	struct cvmx_sli_pktx_instr_fifo_rsize_s cn68xxp1;
	struct cvmx_sli_pktx_instr_fifo_rsize_s cn70xx;
	struct cvmx_sli_pktx_instr_fifo_rsize_s cn78xx;
	struct cvmx_sli_pktx_instr_fifo_rsize_s cnf71xx;
};
typedef union cvmx_sli_pktx_instr_fifo_rsize cvmx_sli_pktx_instr_fifo_rsize_t;

/**
 * cvmx_sli_pkt#_instr_header
 *
 * "SLI_PKT[0..31]_INSTR_HEADER = SLI Packet ring# Instruction Header.
 * VAlues used to build input packet header."
 */
union cvmx_sli_pktx_instr_header {
	uint64_t u64;
	struct cvmx_sli_pktx_instr_header_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_44_63               : 20;
	uint64_t pbp                          : 1;  /**< Enable Packet-by-packet mode.
                                                         Allows DPI to generate PKT_INST_HDR[PM,SL]
                                                         differently per DPI instruction.
                                                         USE_IHDR must be set whenever PBP is set. */
	uint64_t reserved_38_42               : 5;
	uint64_t rparmode                     : 2;  /**< Parse Mode. Becomes PKT_INST_HDR[PM]
                                                         when DPI_INST_HDR[R]==1 and PBP==0 */
	uint64_t reserved_35_35               : 1;
	uint64_t rskp_len                     : 7;  /**< Skip Length. Becomes PKT_INST_HDR[SL]
                                                         when DPI_INST_HDR[R]==1 and PBP==0 */
	uint64_t rngrpext                     : 2;  /**< Becomes PKT_INST_HDR[GRPEXT]
                                                         when DPI_INST_HDR[R]==1 */
	uint64_t rnqos                        : 1;  /**< Becomes PKT_INST_HDR[NQOS]
                                                         when DPI_INST_HDR[R]==1 */
	uint64_t rngrp                        : 1;  /**< Becomes PKT_INST_HDR[NGRP]
                                                         when DPI_INST_HDR[R]==1 */
	uint64_t rntt                         : 1;  /**< Becomes PKT_INST_HDR[NTT]
                                                         when DPI_INST_HDR[R]==1 */
	uint64_t rntag                        : 1;  /**< Becomes PKT_INST_HDR[NTAG]
                                                         when DPI_INST_HDR[R]==1 */
	uint64_t use_ihdr                     : 1;  /**< When set '1' DPI always prepends a PKT_INST_HDR
                                                         as part of the packet data sent to PIP/IPD,
                                                         regardless of DPI_INST_HDR[R]. (DPI also always
                                                         prepends a PKT_INST_HDR when DPI_INST_HDR[R]=1.)
                                                         USE_IHDR must be set whenever PBP is set. */
	uint64_t reserved_16_20               : 5;
	uint64_t par_mode                     : 2;  /**< Parse Mode. Becomes PKT_INST_HDR[PM]
                                                         when DPI_INST_HDR[R]==0 and USE_IHDR==1 and PBP==0 */
	uint64_t reserved_13_13               : 1;
	uint64_t skp_len                      : 7;  /**< Skip Length. Becomes PKT_INST_HDR[SL]
                                                         when DPI_INST_HDR[R]==0 and USE_IHDR==1 and PBP==0 */
	uint64_t ngrpext                      : 2;  /**< Becomes PKT_INST_HDR[GRPEXT]
                                                         when DPI_INST_HDR[R]==0 (and USE_IHDR==1) */
	uint64_t nqos                         : 1;  /**< Becomes PKT_INST_HDR[NQOS]
                                                         when DPI_INST_HDR[R]==0 (and USE_IHDR==1) */
	uint64_t ngrp                         : 1;  /**< Becomes PKT_INST_HDR[NGRP]
                                                         when DPI_INST_HDR[R]==0 (and USE_IHDR==1) */
	uint64_t ntt                          : 1;  /**< Becomes PKT_INST_HDR[NTT]
                                                         when DPI_INST_HDR[R]==0 (and USE_IHDR==1) */
	uint64_t ntag                         : 1;  /**< Becomes PKT_INST_HDR[NTAG]
                                                         when DPI_INST_HDR[R]==0 (and USE_IHDR==1) */
#else
	uint64_t ntag                         : 1;
	uint64_t ntt                          : 1;
	uint64_t ngrp                         : 1;
	uint64_t nqos                         : 1;
	uint64_t ngrpext                      : 2;
	uint64_t skp_len                      : 7;
	uint64_t reserved_13_13               : 1;
	uint64_t par_mode                     : 2;
	uint64_t reserved_16_20               : 5;
	uint64_t use_ihdr                     : 1;
	uint64_t rntag                        : 1;
	uint64_t rntt                         : 1;
	uint64_t rngrp                        : 1;
	uint64_t rnqos                        : 1;
	uint64_t rngrpext                     : 2;
	uint64_t rskp_len                     : 7;
	uint64_t reserved_35_35               : 1;
	uint64_t rparmode                     : 2;
	uint64_t reserved_38_42               : 5;
	uint64_t pbp                          : 1;
	uint64_t reserved_44_63               : 20;
#endif
	} s;
	struct cvmx_sli_pktx_instr_header_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_44_63               : 20;
	uint64_t pbp                          : 1;  /**< Enable Packet-by-packet mode.
                                                         Allows DPI to generate PKT_INST_HDR[PM,SL]
                                                         differently per DPI instruction.
                                                         USE_IHDR must be set whenever PBP is set. */
	uint64_t reserved_38_42               : 5;
	uint64_t rparmode                     : 2;  /**< Parse Mode. Becomes PKT_INST_HDR[PM]
                                                         when DPI_INST_HDR[R]==1 and PBP==0 */
	uint64_t reserved_35_35               : 1;
	uint64_t rskp_len                     : 7;  /**< Skip Length. Becomes PKT_INST_HDR[SL]
                                                         when DPI_INST_HDR[R]==1 and PBP==0 */
	uint64_t reserved_26_27               : 2;
	uint64_t rnqos                        : 1;  /**< Becomes PKT_INST_HDR[NQOS]
                                                         when DPI_INST_HDR[R]==1 */
	uint64_t rngrp                        : 1;  /**< Becomes PKT_INST_HDR[NGRP]
                                                         when DPI_INST_HDR[R]==1 */
	uint64_t rntt                         : 1;  /**< Becomes PKT_INST_HDR[NTT]
                                                         when DPI_INST_HDR[R]==1 */
	uint64_t rntag                        : 1;  /**< Becomes PKT_INST_HDR[NTAG]
                                                         when DPI_INST_HDR[R]==1 */
	uint64_t use_ihdr                     : 1;  /**< When set '1' DPI always prepends a PKT_INST_HDR
                                                         as part of the packet data sent to PIP/IPD,
                                                         regardless of DPI_INST_HDR[R]. (DPI also always
                                                         prepends a PKT_INST_HDR when DPI_INST_HDR[R]=1.)
                                                         USE_IHDR must be set whenever PBP is set. */
	uint64_t reserved_16_20               : 5;
	uint64_t par_mode                     : 2;  /**< Parse Mode. Becomes PKT_INST_HDR[PM]
                                                         when DPI_INST_HDR[R]==0 and USE_IHDR==1 and PBP==0 */
	uint64_t reserved_13_13               : 1;
	uint64_t skp_len                      : 7;  /**< Skip Length. Becomes PKT_INST_HDR[SL]
                                                         when DPI_INST_HDR[R]==0 and USE_IHDR==1 and PBP==0 */
	uint64_t reserved_4_5                 : 2;
	uint64_t nqos                         : 1;  /**< Becomes PKT_INST_HDR[NQOS]
                                                         when DPI_INST_HDR[R]==0 (and USE_IHDR==1) */
	uint64_t ngrp                         : 1;  /**< Becomes PKT_INST_HDR[NGRP]
                                                         when DPI_INST_HDR[R]==0 (and USE_IHDR==1) */
	uint64_t ntt                          : 1;  /**< Becomes PKT_INST_HDR[NTT]
                                                         when DPI_INST_HDR[R]==0 (and USE_IHDR==1) */
	uint64_t ntag                         : 1;  /**< Becomes PKT_INST_HDR[NTAG]
                                                         when DPI_INST_HDR[R]==0 (and USE_IHDR==1) */
#else
	uint64_t ntag                         : 1;
	uint64_t ntt                          : 1;
	uint64_t ngrp                         : 1;
	uint64_t nqos                         : 1;
	uint64_t reserved_4_5                 : 2;
	uint64_t skp_len                      : 7;
	uint64_t reserved_13_13               : 1;
	uint64_t par_mode                     : 2;
	uint64_t reserved_16_20               : 5;
	uint64_t use_ihdr                     : 1;
	uint64_t rntag                        : 1;
	uint64_t rntt                         : 1;
	uint64_t rngrp                        : 1;
	uint64_t rnqos                        : 1;
	uint64_t reserved_26_27               : 2;
	uint64_t rskp_len                     : 7;
	uint64_t reserved_35_35               : 1;
	uint64_t rparmode                     : 2;
	uint64_t reserved_38_42               : 5;
	uint64_t pbp                          : 1;
	uint64_t reserved_44_63               : 20;
#endif
	} cn61xx;
	struct cvmx_sli_pktx_instr_header_cn61xx cn63xx;
	struct cvmx_sli_pktx_instr_header_cn61xx cn63xxp1;
	struct cvmx_sli_pktx_instr_header_cn61xx cn66xx;
	struct cvmx_sli_pktx_instr_header_s   cn68xx;
	struct cvmx_sli_pktx_instr_header_cn61xx cn68xxp1;
	struct cvmx_sli_pktx_instr_header_cn61xx cn70xx;
	struct cvmx_sli_pktx_instr_header_cn61xx cnf71xx;
};
typedef union cvmx_sli_pktx_instr_header cvmx_sli_pktx_instr_header_t;

/**
 * cvmx_sli_pkt#_int_levels
 *
 * This register contains output-packet interrupt levels.
 *
 */
union cvmx_sli_pktx_int_levels {
	uint64_t u64;
	struct cvmx_sli_pktx_int_levels_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_54_63               : 10;
	uint64_t time                         : 22; /**< Output ring counter time interrupt threshold.
                                                         SLI sets SLI_PKT_TIME_INT[PORT<i>] whenever SLI_PKTi_CNTS[TIMER] > TIME. */
	uint64_t cnt                          : 32; /**< Output ring counter interrupt threshold. SLI sets SLI_PKT_CNT_INT[PORT<i>] whenever
                                                         SLI_PKTi_CNTS[CNT] > CNT. */
#else
	uint64_t cnt                          : 32;
	uint64_t time                         : 22;
	uint64_t reserved_54_63               : 10;
#endif
	} s;
	struct cvmx_sli_pktx_int_levels_s     cn78xx;
};
typedef union cvmx_sli_pktx_int_levels cvmx_sli_pktx_int_levels_t;

/**
 * cvmx_sli_pkt#_out_size
 *
 * This register contains the BSIZE and ISIZE for output packet rings.
 *
 */
union cvmx_sli_pktx_out_size {
	uint64_t u64;
	struct cvmx_sli_pktx_out_size_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_23_63               : 41;
	uint64_t isize                        : 7;  /**< INFO BYTES size (bytes) for ring X. Legal sizes
                                                         are 0 to 120. Not used in buffer-pointer-only mode. */
	uint64_t bsize                        : 16; /**< BUFFER SIZE (bytes) for ring X. */
#else
	uint64_t bsize                        : 16;
	uint64_t isize                        : 7;
	uint64_t reserved_23_63               : 41;
#endif
	} s;
	struct cvmx_sli_pktx_out_size_s       cn61xx;
	struct cvmx_sli_pktx_out_size_s       cn63xx;
	struct cvmx_sli_pktx_out_size_s       cn63xxp1;
	struct cvmx_sli_pktx_out_size_s       cn66xx;
	struct cvmx_sli_pktx_out_size_s       cn68xx;
	struct cvmx_sli_pktx_out_size_s       cn68xxp1;
	struct cvmx_sli_pktx_out_size_s       cn70xx;
	struct cvmx_sli_pktx_out_size_s       cn78xx;
	struct cvmx_sli_pktx_out_size_s       cnf71xx;
};
typedef union cvmx_sli_pktx_out_size cvmx_sli_pktx_out_size_t;

/**
 * cvmx_sli_pkt#_output_control
 *
 * This register is the control for read operations for gather list and instructions.
 *
 */
union cvmx_sli_pktx_output_control {
	uint64_t u64;
	struct cvmx_sli_pktx_output_control_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t tenb                         : 1;  /**< Output ring packet timer interrupt enable. When both TENB and corresponding
                                                         SLI_PKT_TIME_INT[PORT<i>] are set, for any i, SLI_INT_SUM[PTIME] is set, which can cause
                                                         an interrupt. */
	uint64_t cenb                         : 1;  /**< Output ring packet counter interrupt enable. When both CENB and corresponding
                                                         SLI_PKT_CNT_INT[PORT<i>] are set, for any i, SLI_INT_SUM[PCNT] is set, which can cause an
                                                         interrupt. */
	uint64_t iptr                         : 1;  /**< When IPTR=1, packet output ring is in info-pointer mode; otherwise the packet output ring
                                                         is in buffer-pointer-only mode. */
	uint64_t es                           : 2;  /**< ES or MACADD<63:62> for buffer/info write operations to buffer/info pair MAC memory space
                                                         addresses fetched from packet output ring. ES<1:0> if SLI_PKTx_OUTPUT_CONTROL[DPTR]=1,
                                                         else MACADD<63:62>. In the latter case, ES<1:0> comes from DPTR<63:62>. ES<1:0> is the
                                                         endian-swap attribute for these MAC memory space writes. */
	uint64_t nsr                          : 1;  /**< ADDRTYPE<1> or MACADD<61> for buffer/info write operations. NSR    becomes either
                                                         ADDRTYPE<1> or MACADD<61> for writes to buffer/info pair MAC memory space addresses
                                                         fetched from packet output ring. ADDRTYPE<1> if SLI_PKTx_OUTPUT_CONTROL[DPTR]=1, else
                                                         MACADD<61>. In the latter case, ADDRTYPE<1> comes from DPTR<61>. ADDRTYPE<1> is the no-
                                                         snoop attribute for PCIe. */
	uint64_t ror                          : 1;  /**< ADDRTYPE<0> or MACADD<60> for buffer/info write operations. ROR    becomes either
                                                         ADDRTYPE<0> or MACADD<60> for writes to buffer/info pair MAC memory space addresses
                                                         fetched from packet output ring. ADDRTYPE<0> if SLI_PKTx_OUTPUT_CONTROL[DPTR]=1, else
                                                         MACADD<60>. In the latter case, ADDRTYPE<0> comes from DPTR<60>. ADDRTYPE<0> is the
                                                         relaxed-order attribute for PCIe. */
	uint64_t dptr                         : 1;  /**< Determines whether buffer/info pointers are DPTR format 0 or DPTR format 1. When DPTR=1,
                                                         the buffer/info pointers fetched from packet output ring are DPTR format 0. When DPTR=0,
                                                         the buffer/info pointers fetched from packet output ring i are DPTR format 1. (Replace
                                                         SLI_PKT(0..63)_INPUT_CONTROL[D_ESR,D_NSR,D_ROR] in the descriptions of DPTR format 0/1 in
                                                         DPI Instruction Input Initialization with SLI_PKTx_OUTPUT_CONTROL[ES],
                                                         SLI_PKTx_OUTPUT_CONTROL[NSR], and SLI_PKTx_OUTPUT_CONTROL[ROR], respectively, though.) */
	uint64_t bmode                        : 1;  /**< Determines whether SLI_PKTx_CNTS[CNT] is a byte or packet counter. When BMODE=1,
                                                         SLI_PKTx_CNTS[CNT] is a byte counter, else SLI_PKTx_CNTS[CNT] is a packet counter. */
	uint64_t es_p                         : 2;  /**< ES<1:0> for the packet output ring reads that fetch buffer/info pointer pairs. ES<1:0> is
                                                         the endian-swap attribute for these MAC memory space reads. */
	uint64_t nsr_p                        : 1;  /**< ADDRTYPE<1> or MACADD<61> for buffer/info write operations. NSR    becomes either
                                                         ADDRTYPE<1> or MACADD<61> for writes to buffer/info pair MAC memory space addresses
                                                         fetched from packet output ring. ADDRTYPE<1> if SLI_PKT(0..63)_OUTPUT_CONTROL[DPTR]=1,
                                                         else MACADD<61>. In the latter case, ADDRTYPE<1> comes from DPTR<61>. ADDRTYPE<1> is the
                                                         no-snoop attribute for PCIe. */
	uint64_t ror_p                        : 1;  /**< ADDRTYPE<0> for the packet output ring reads that fetch buffer/info pointer pairs. ROR
                                                         becomes ADDRTYPE<0> in DPI/SLI reads that fetch buffer/info pairs from packet output ring
                                                         (from address SLI_PKTx_SLIST_BADDR+ in MAC memory space.) ADDRTYPE<0> is the relaxed-order
                                                         attribute for PCIe. */
	uint64_t enb                          : 1;  /**< When ENB=1, packet output ring is enabled. If an error occurs on reading pointers or a FLR
                                                         occurs that the ring belongs to, this bit will be cleared to 0. Also see SLI_PKT_OUT_ENB. */
#else
	uint64_t enb                          : 1;
	uint64_t ror_p                        : 1;
	uint64_t nsr_p                        : 1;
	uint64_t es_p                         : 2;
	uint64_t bmode                        : 1;
	uint64_t dptr                         : 1;
	uint64_t ror                          : 1;
	uint64_t nsr                          : 1;
	uint64_t es                           : 2;
	uint64_t iptr                         : 1;
	uint64_t cenb                         : 1;
	uint64_t tenb                         : 1;
	uint64_t reserved_14_63               : 50;
#endif
	} s;
	struct cvmx_sli_pktx_output_control_s cn78xx;
};
typedef union cvmx_sli_pktx_output_control cvmx_sli_pktx_output_control_t;

/**
 * cvmx_sli_pkt#_slist_baddr
 *
 * This register contains the start of scatter list for output-packet pointers. This address must
 * be 16-byte aligned.
 */
union cvmx_sli_pktx_slist_baddr {
	uint64_t u64;
	struct cvmx_sli_pktx_slist_baddr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t addr                         : 60; /**< Base address for scatter list pointers. */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t addr                         : 60;
#endif
	} s;
	struct cvmx_sli_pktx_slist_baddr_s    cn61xx;
	struct cvmx_sli_pktx_slist_baddr_s    cn63xx;
	struct cvmx_sli_pktx_slist_baddr_s    cn63xxp1;
	struct cvmx_sli_pktx_slist_baddr_s    cn66xx;
	struct cvmx_sli_pktx_slist_baddr_s    cn68xx;
	struct cvmx_sli_pktx_slist_baddr_s    cn68xxp1;
	struct cvmx_sli_pktx_slist_baddr_s    cn70xx;
	struct cvmx_sli_pktx_slist_baddr_s    cn78xx;
	struct cvmx_sli_pktx_slist_baddr_s    cnf71xx;
};
typedef union cvmx_sli_pktx_slist_baddr cvmx_sli_pktx_slist_baddr_t;

/**
 * cvmx_sli_pkt#_slist_baoff_dbell
 *
 * This register contains the doorbell and base-address offset for next read operation.
 *
 */
union cvmx_sli_pktx_slist_baoff_dbell {
	uint64_t u64;
	struct cvmx_sli_pktx_slist_baoff_dbell_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t aoff                         : 32; /**< The offset from the SLI_PKT[0..31]_SLIST_BADDR
                                                         where the next SList pointer will be read.
                                                         A write of 0xFFFFFFFF to the DBELL field will
                                                         clear DBELL and AOFF */
	uint64_t dbell                        : 32; /**< Scatter list doorbell count. Writes to this field
                                                         will increment the value here. Reads will return
                                                         present value. The value of this field is
                                                         decremented as read operations are ISSUED for
                                                         scatter pointers.
                                                         A write of 0xFFFFFFFF will clear DBELL and AOFF */
#else
	uint64_t dbell                        : 32;
	uint64_t aoff                         : 32;
#endif
	} s;
	struct cvmx_sli_pktx_slist_baoff_dbell_s cn61xx;
	struct cvmx_sli_pktx_slist_baoff_dbell_s cn63xx;
	struct cvmx_sli_pktx_slist_baoff_dbell_s cn63xxp1;
	struct cvmx_sli_pktx_slist_baoff_dbell_s cn66xx;
	struct cvmx_sli_pktx_slist_baoff_dbell_s cn68xx;
	struct cvmx_sli_pktx_slist_baoff_dbell_s cn68xxp1;
	struct cvmx_sli_pktx_slist_baoff_dbell_s cn70xx;
	struct cvmx_sli_pktx_slist_baoff_dbell_s cn78xx;
	struct cvmx_sli_pktx_slist_baoff_dbell_s cnf71xx;
};
typedef union cvmx_sli_pktx_slist_baoff_dbell cvmx_sli_pktx_slist_baoff_dbell_t;

/**
 * cvmx_sli_pkt#_slist_fifo_rsize
 *
 * This register contains the number of scatter pointer pairs in the scatter list.
 *
 */
union cvmx_sli_pktx_slist_fifo_rsize {
	uint64_t u64;
	struct cvmx_sli_pktx_slist_fifo_rsize_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t rsize                        : 32; /**< The number of scatter pointer pairs contained in
                                                         the scatter list ring. */
#else
	uint64_t rsize                        : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_pktx_slist_fifo_rsize_s cn61xx;
	struct cvmx_sli_pktx_slist_fifo_rsize_s cn63xx;
	struct cvmx_sli_pktx_slist_fifo_rsize_s cn63xxp1;
	struct cvmx_sli_pktx_slist_fifo_rsize_s cn66xx;
	struct cvmx_sli_pktx_slist_fifo_rsize_s cn68xx;
	struct cvmx_sli_pktx_slist_fifo_rsize_s cn68xxp1;
	struct cvmx_sli_pktx_slist_fifo_rsize_s cn70xx;
	struct cvmx_sli_pktx_slist_fifo_rsize_s cn78xx;
	struct cvmx_sli_pktx_slist_fifo_rsize_s cnf71xx;
};
typedef union cvmx_sli_pktx_slist_fifo_rsize cvmx_sli_pktx_slist_fifo_rsize_t;

/**
 * cvmx_sli_pkt#_vf_sig
 *
 * This register is used to signal between PF/VF. These 64 CSRs are index by VF number.
 *
 */
union cvmx_sli_pktx_vf_sig {
	uint64_t u64;
	struct cvmx_sli_pktx_vf_sig_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Field can be Read or written to by PF and owning VF. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_sli_pktx_vf_sig_s         cn78xx;
};
typedef union cvmx_sli_pktx_vf_sig cvmx_sli_pktx_vf_sig_t;

/**
 * cvmx_sli_pkt_cnt_int
 *
 * The packets rings that are interrupting because of Packet Counters.
 *
 */
union cvmx_sli_pkt_cnt_int {
	uint64_t u64;
	struct cvmx_sli_pkt_cnt_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_sli_pkt_cnt_int_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t port                         : 32; /**< Output ring packet counter interrupt bits
                                                         SLI sets PORT<i> whenever
                                                         SLI_PKTi_CNTS[CNT] > SLI_PKT_INT_LEVELS[CNT].
                                                         SLI_PKT_CNT_INT_ENB[PORT<i>] is the corresponding
                                                         enable. */
#else
	uint64_t port                         : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} cn61xx;
	struct cvmx_sli_pkt_cnt_int_cn61xx    cn63xx;
	struct cvmx_sli_pkt_cnt_int_cn61xx    cn63xxp1;
	struct cvmx_sli_pkt_cnt_int_cn61xx    cn66xx;
	struct cvmx_sli_pkt_cnt_int_cn61xx    cn68xx;
	struct cvmx_sli_pkt_cnt_int_cn61xx    cn68xxp1;
	struct cvmx_sli_pkt_cnt_int_cn61xx    cn70xx;
	struct cvmx_sli_pkt_cnt_int_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ring                         : 64; /**< Output ring packet counter interrupt bits
                                                         SLI sets RING<i> whenever
                                                         SLI_PKTi_CNTS[CNT] > SLI_PKT_INT_LEVELS[CNT].
                                                         SLI_PKT_CNT_INT_ENB[RING<i>] is the corresponding
                                                         enable. */
#else
	uint64_t ring                         : 64;
#endif
	} cn78xx;
	struct cvmx_sli_pkt_cnt_int_cn61xx    cnf71xx;
};
typedef union cvmx_sli_pkt_cnt_int cvmx_sli_pkt_cnt_int_t;

/**
 * cvmx_sli_pkt_cnt_int_enb
 *
 * Enable for the packets rings that are interrupting because of Packet Counters.
 *
 */
union cvmx_sli_pkt_cnt_int_enb {
	uint64_t u64;
	struct cvmx_sli_pkt_cnt_int_enb_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t port                         : 32; /**< Output ring packet counter interrupt enables
                                                         When both PORT<i> and corresponding
                                                         SLI_PKT_CNT_INT[PORT<i>] are set, for any i,
                                                         then SLI_INT_SUM[PCNT] is set, which can cause
                                                         an interrupt. */
#else
	uint64_t port                         : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_pkt_cnt_int_enb_s     cn61xx;
	struct cvmx_sli_pkt_cnt_int_enb_s     cn63xx;
	struct cvmx_sli_pkt_cnt_int_enb_s     cn63xxp1;
	struct cvmx_sli_pkt_cnt_int_enb_s     cn66xx;
	struct cvmx_sli_pkt_cnt_int_enb_s     cn68xx;
	struct cvmx_sli_pkt_cnt_int_enb_s     cn68xxp1;
	struct cvmx_sli_pkt_cnt_int_enb_s     cn70xx;
	struct cvmx_sli_pkt_cnt_int_enb_s     cnf71xx;
};
typedef union cvmx_sli_pkt_cnt_int_enb cvmx_sli_pkt_cnt_int_enb_t;

/**
 * cvmx_sli_pkt_ctl
 *
 * Control for packets.
 *
 */
union cvmx_sli_pkt_ctl {
	uint64_t u64;
	struct cvmx_sli_pkt_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t ring_en                      : 1;  /**< When '0' forces "relative Q position" received
                                                         from PKO to be zero, and replicates the back-
                                                         pressure indication for the first ring attached
                                                         to a PKO port across all the rings attached to a
                                                         PKO port. When '1' backpressure is on a per
                                                         port/ring. */
	uint64_t pkt_bp                       : 4;  /**< When set '1' enable the port level backpressure for
                                                         PKO ports associated with the bit. */
#else
	uint64_t pkt_bp                       : 4;
	uint64_t ring_en                      : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_sli_pkt_ctl_s             cn61xx;
	struct cvmx_sli_pkt_ctl_s             cn63xx;
	struct cvmx_sli_pkt_ctl_s             cn63xxp1;
	struct cvmx_sli_pkt_ctl_s             cn66xx;
	struct cvmx_sli_pkt_ctl_s             cn68xx;
	struct cvmx_sli_pkt_ctl_s             cn68xxp1;
	struct cvmx_sli_pkt_ctl_s             cn70xx;
	struct cvmx_sli_pkt_ctl_s             cnf71xx;
};
typedef union cvmx_sli_pkt_ctl cvmx_sli_pkt_ctl_t;

/**
 * cvmx_sli_pkt_data_out_es
 *
 * The Endian Swap for writing Data Out.
 *
 */
union cvmx_sli_pkt_data_out_es {
	uint64_t u64;
	struct cvmx_sli_pkt_data_out_es_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t es                           : 64; /**< ES<1:0> or MACADD<63:62> for buffer/info writes.
                                                         ES<2i+1:2i> becomes either ES<1:0> or
                                                         MACADD<63:62> for writes to buffer/info pair
                                                         MAC memory space addresses fetched from packet
                                                         output ring i. ES<1:0> if SLI_PKT_DPADDR[DPTR<i>]=1
                                                         , else MACADD<63:62>.
                                                         In the latter case, ES<1:0> comes from DPTR<63:62>.
                                                         ES<1:0> is the endian-swap attribute for these MAC
                                                         memory space writes. */
#else
	uint64_t es                           : 64;
#endif
	} s;
	struct cvmx_sli_pkt_data_out_es_s     cn61xx;
	struct cvmx_sli_pkt_data_out_es_s     cn63xx;
	struct cvmx_sli_pkt_data_out_es_s     cn63xxp1;
	struct cvmx_sli_pkt_data_out_es_s     cn66xx;
	struct cvmx_sli_pkt_data_out_es_s     cn68xx;
	struct cvmx_sli_pkt_data_out_es_s     cn68xxp1;
	struct cvmx_sli_pkt_data_out_es_s     cn70xx;
	struct cvmx_sli_pkt_data_out_es_s     cnf71xx;
};
typedef union cvmx_sli_pkt_data_out_es cvmx_sli_pkt_data_out_es_t;

/**
 * cvmx_sli_pkt_data_out_ns
 *
 * The NS field for the TLP when writing packet data.
 *
 */
union cvmx_sli_pkt_data_out_ns {
	uint64_t u64;
	struct cvmx_sli_pkt_data_out_ns_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t nsr                          : 32; /**< ADDRTYPE<1> or MACADD<61> for buffer/info writes.
                                                         NSR<i> becomes either ADDRTYPE<1> or MACADD<61>
                                                         for writes to buffer/info pair MAC memory space
                                                         addresses fetched from packet output ring i.
                                                         ADDRTYPE<1> if SLI_PKT_DPADDR[DPTR<i>]=1, else
                                                         MACADD<61>.
                                                         In the latter case,ADDRTYPE<1> comes from DPTR<61>.
                                                         ADDRTYPE<1> is the no-snoop attribute for PCIe
                                                         , helps select an SRIO*_S2M_TYPE* entry with sRIO. */
#else
	uint64_t nsr                          : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_pkt_data_out_ns_s     cn61xx;
	struct cvmx_sli_pkt_data_out_ns_s     cn63xx;
	struct cvmx_sli_pkt_data_out_ns_s     cn63xxp1;
	struct cvmx_sli_pkt_data_out_ns_s     cn66xx;
	struct cvmx_sli_pkt_data_out_ns_s     cn68xx;
	struct cvmx_sli_pkt_data_out_ns_s     cn68xxp1;
	struct cvmx_sli_pkt_data_out_ns_s     cn70xx;
	struct cvmx_sli_pkt_data_out_ns_s     cnf71xx;
};
typedef union cvmx_sli_pkt_data_out_ns cvmx_sli_pkt_data_out_ns_t;

/**
 * cvmx_sli_pkt_data_out_ror
 *
 * The ROR field for the TLP when writing Packet Data.
 *
 */
union cvmx_sli_pkt_data_out_ror {
	uint64_t u64;
	struct cvmx_sli_pkt_data_out_ror_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t ror                          : 32; /**< ADDRTYPE<0> or MACADD<60> for buffer/info writes.
                                                         ROR<i> becomes either ADDRTYPE<0> or MACADD<60>
                                                         for writes to buffer/info pair MAC memory space
                                                         addresses fetched from packet output ring i.
                                                         ADDRTYPE<0> if SLI_PKT_DPADDR[DPTR<i>]=1, else
                                                         MACADD<60>.
                                                         In the latter case,ADDRTYPE<0> comes from DPTR<60>.
                                                         ADDRTYPE<0> is the relaxed-order attribute for PCIe
                                                         , helps select an SRIO*_S2M_TYPE* entry with sRIO. */
#else
	uint64_t ror                          : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_pkt_data_out_ror_s    cn61xx;
	struct cvmx_sli_pkt_data_out_ror_s    cn63xx;
	struct cvmx_sli_pkt_data_out_ror_s    cn63xxp1;
	struct cvmx_sli_pkt_data_out_ror_s    cn66xx;
	struct cvmx_sli_pkt_data_out_ror_s    cn68xx;
	struct cvmx_sli_pkt_data_out_ror_s    cn68xxp1;
	struct cvmx_sli_pkt_data_out_ror_s    cn70xx;
	struct cvmx_sli_pkt_data_out_ror_s    cnf71xx;
};
typedef union cvmx_sli_pkt_data_out_ror cvmx_sli_pkt_data_out_ror_t;

/**
 * cvmx_sli_pkt_dpaddr
 *
 * Used to detemine address and attributes for packet data writes.
 *
 */
union cvmx_sli_pkt_dpaddr {
	uint64_t u64;
	struct cvmx_sli_pkt_dpaddr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t dptr                         : 32; /**< Determines whether buffer/info pointers are
                                                         DPTR format 0 or DPTR format 1.
                                                         When DPTR<i>=1, the buffer/info pointers fetched
                                                         from packet output ring i are DPTR format 0.
                                                         When DPTR<i>=0, the buffer/info pointers fetched
                                                         from packet output ring i are DPTR format 1.
                                                         (Replace SLI_PKT_INPUT_CONTROL[D_ESR,D_NSR,D_ROR]
                                                         in the HRM descriptions of DPTR format 0/1 with
                                                         SLI_PKT_DATA_OUT_ES[ES<2i+1:2i>],
                                                         SLI_PKT_DATA_OUT_NS[NSR<i>], and
                                                         SLI_PKT_DATA_OUT_ROR[ROR<i>], respectively,
                                                         though.) */
#else
	uint64_t dptr                         : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_pkt_dpaddr_s          cn61xx;
	struct cvmx_sli_pkt_dpaddr_s          cn63xx;
	struct cvmx_sli_pkt_dpaddr_s          cn63xxp1;
	struct cvmx_sli_pkt_dpaddr_s          cn66xx;
	struct cvmx_sli_pkt_dpaddr_s          cn68xx;
	struct cvmx_sli_pkt_dpaddr_s          cn68xxp1;
	struct cvmx_sli_pkt_dpaddr_s          cn70xx;
	struct cvmx_sli_pkt_dpaddr_s          cnf71xx;
};
typedef union cvmx_sli_pkt_dpaddr cvmx_sli_pkt_dpaddr_t;

/**
 * cvmx_sli_pkt_in_bp
 *
 * Which input rings have backpressure applied.
 *
 */
union cvmx_sli_pkt_in_bp {
	uint64_t u64;
	struct cvmx_sli_pkt_in_bp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t bp                           : 32; /**< A packet input  ring that has its count greater
                                                         than its WMARK will have backpressure applied.
                                                         Each of the 32 bits coorespond to an input ring.
                                                         When '1' that ring has backpressure applied an
                                                         will fetch no more instructions, but will process
                                                         any previously fetched instructions. */
#else
	uint64_t bp                           : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_pkt_in_bp_s           cn61xx;
	struct cvmx_sli_pkt_in_bp_s           cn63xx;
	struct cvmx_sli_pkt_in_bp_s           cn63xxp1;
	struct cvmx_sli_pkt_in_bp_s           cn66xx;
	struct cvmx_sli_pkt_in_bp_s           cn70xx;
	struct cvmx_sli_pkt_in_bp_s           cnf71xx;
};
typedef union cvmx_sli_pkt_in_bp cvmx_sli_pkt_in_bp_t;

/**
 * cvmx_sli_pkt_in_done#_cnts
 *
 * This register contains counters for instructions completed on input rings.
 *
 */
union cvmx_sli_pkt_in_donex_cnts {
	uint64_t u64;
	struct cvmx_sli_pkt_in_donex_cnts_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t po_int                       : 1;  /**< "Returns a 1 when either the corresponding bit in SLI_PKT_TIME_INT[RING[\#]] or
                                                         SLI_PKT_CNT_INT[RING[\#]] is set." */
	uint64_t pi_int                       : 1;  /**< "Returns a 1 when corresponding bit of SLI_PKT_IN_INT[RING[\#]] is set. Writing a 1 to this
                                                         field clears the corresponding bit in SLI_PKT_IN_INT[RING[\#]]." */
	uint64_t reserved_49_61               : 13;
	uint64_t cint_enb                     : 1;  /**< When set, allows corresponding bit in SLI_PKT_IN_INT[RING[\#]] to be set. */
	uint64_t wmark                        : 16; /**< "When the value of SLI_PKT_IN_DONE#_CNTS[CNT[15:0]] is updated to be equal to
                                                         SLI_PKT_IN_DONE#_CNTS[WMARK[15:0]] and SLI_PKT_IN_DONE#_CNTS[CINT_ENB] is also set, the
                                                         corresponding bit in SLI_PKT_IN_INT[RING[\#]] will be set." */
	uint64_t cnt                          : 32; /**< This field is incrmented by '1' when an instruction
                                                         is completed. This field is incremented as the
                                                         last of the data is read from the MAC. */
#else
	uint64_t cnt                          : 32;
	uint64_t wmark                        : 16;
	uint64_t cint_enb                     : 1;
	uint64_t reserved_49_61               : 13;
	uint64_t pi_int                       : 1;
	uint64_t po_int                       : 1;
#endif
	} s;
	struct cvmx_sli_pkt_in_donex_cnts_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t cnt                          : 32; /**< This field is incrmented by '1' when an instruction
                                                         is completed. This field is incremented as the
                                                         last of the data is read from the MAC. */
#else
	uint64_t cnt                          : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} cn61xx;
	struct cvmx_sli_pkt_in_donex_cnts_cn61xx cn63xx;
	struct cvmx_sli_pkt_in_donex_cnts_cn61xx cn63xxp1;
	struct cvmx_sli_pkt_in_donex_cnts_cn61xx cn66xx;
	struct cvmx_sli_pkt_in_donex_cnts_cn61xx cn68xx;
	struct cvmx_sli_pkt_in_donex_cnts_cn61xx cn68xxp1;
	struct cvmx_sli_pkt_in_donex_cnts_cn61xx cn70xx;
	struct cvmx_sli_pkt_in_donex_cnts_s   cn78xx;
	struct cvmx_sli_pkt_in_donex_cnts_cn61xx cnf71xx;
};
typedef union cvmx_sli_pkt_in_donex_cnts cvmx_sli_pkt_in_donex_cnts_t;

/**
 * cvmx_sli_pkt_in_instr_counts
 *
 * This register contains keeps track of the number of instructions read into the FIFO and
 * packets sent to PKI.
 */
union cvmx_sli_pkt_in_instr_counts {
	uint64_t u64;
	struct cvmx_sli_pkt_in_instr_counts_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t wr_cnt                       : 32; /**< Shows the number of packets sent to the IPD. */
	uint64_t rd_cnt                       : 32; /**< Shows the value of instructions that have had reads
                                                         issued for them.
                                                         to the Packet-ring is in reset. */
#else
	uint64_t rd_cnt                       : 32;
	uint64_t wr_cnt                       : 32;
#endif
	} s;
	struct cvmx_sli_pkt_in_instr_counts_s cn61xx;
	struct cvmx_sli_pkt_in_instr_counts_s cn63xx;
	struct cvmx_sli_pkt_in_instr_counts_s cn63xxp1;
	struct cvmx_sli_pkt_in_instr_counts_s cn66xx;
	struct cvmx_sli_pkt_in_instr_counts_s cn68xx;
	struct cvmx_sli_pkt_in_instr_counts_s cn68xxp1;
	struct cvmx_sli_pkt_in_instr_counts_s cn70xx;
	struct cvmx_sli_pkt_in_instr_counts_s cn78xx;
	struct cvmx_sli_pkt_in_instr_counts_s cnf71xx;
};
typedef union cvmx_sli_pkt_in_instr_counts cvmx_sli_pkt_in_instr_counts_t;

/**
 * cvmx_sli_pkt_in_int
 *
 * When read by a VF, this register informs which rings owned by the VF (0 to 63) have an
 * interrupt pending. In PF mode, this register returns an unpredictable value. Writing 1s to
 * clear this register clears both packet count an packet time interrupts. The clearing of the
 * interrupts will be reflected in SLI_PKT_CNT_INT and SLI_PKT_TIME_INT.
 */
union cvmx_sli_pkt_in_int {
	uint64_t u64;
	struct cvmx_sli_pkt_in_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ring                         : 64; /**< "Set when SLI_PKT_IN_DONE#_CNTS[CNT[15:0]] is updated to be equal to
                                                         SLI_PKT_IN_DONE#_CNTS[WMARK[15:0]] and SLI_PKT_IN_DONE#_CNTS[CINT_ENB] is set. Cleared
                                                         when SLI_PKT_IN_DONE#_CNTS[PI_INT] is cleared." */
#else
	uint64_t ring                         : 64;
#endif
	} s;
	struct cvmx_sli_pkt_in_int_s          cn78xx;
};
typedef union cvmx_sli_pkt_in_int cvmx_sli_pkt_in_int_t;

/**
 * cvmx_sli_pkt_in_pcie_port
 *
 * Assigns Packet Input rings to MAC ports.
 *
 */
union cvmx_sli_pkt_in_pcie_port {
	uint64_t u64;
	struct cvmx_sli_pkt_in_pcie_port_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pp                           : 64; /**< The MAC port that the Packet ring number is
                                                         assigned. Two bits are used per ring (i.e. ring 0
                                                         [1:0], ring 1 [3:2], ....). A value of '0 means
                                                         that the Packetring is assign to MAC Port 0, a '1'
                                                         MAC Port 1, a '2' MAC Port 2, and a '3' MAC Port 3. */
#else
	uint64_t pp                           : 64;
#endif
	} s;
	struct cvmx_sli_pkt_in_pcie_port_s    cn61xx;
	struct cvmx_sli_pkt_in_pcie_port_s    cn63xx;
	struct cvmx_sli_pkt_in_pcie_port_s    cn63xxp1;
	struct cvmx_sli_pkt_in_pcie_port_s    cn66xx;
	struct cvmx_sli_pkt_in_pcie_port_s    cn68xx;
	struct cvmx_sli_pkt_in_pcie_port_s    cn68xxp1;
	struct cvmx_sli_pkt_in_pcie_port_s    cn70xx;
	struct cvmx_sli_pkt_in_pcie_port_s    cnf71xx;
};
typedef union cvmx_sli_pkt_in_pcie_port cvmx_sli_pkt_in_pcie_port_t;

/**
 * cvmx_sli_pkt_input_control
 *
 * Control for reads for gather list and instructions.
 *
 */
union cvmx_sli_pkt_input_control {
	uint64_t u64;
	struct cvmx_sli_pkt_input_control_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t prd_erst                     : 1;  /**< PRD Error Reset */
	uint64_t prd_rds                      : 7;  /**< PRD Reads Out */
	uint64_t gii_erst                     : 1;  /**< GII Error Reset */
	uint64_t gii_rds                      : 7;  /**< GII Reads Out */
	uint64_t reserved_41_47               : 7;
	uint64_t prc_idle                     : 1;  /**< PRC In IDLE */
	uint64_t reserved_24_39               : 16;
	uint64_t pin_rst                      : 1;  /**< Packet In Reset. When a gather-list read receives
                                                         an error this bit (along with SLI_INT_SUM[PGL_ERR])
                                                         is set. When receiveing a PGL_ERR interrupt the SW
                                                         should:
                                                         1. Wait 2ms to allow any outstanding reads to return
                                                            or be timed out.
                                                         2. Write a '0' to this bit.
                                                         3. Startup the packet input again (all previous
                                                            CSR setting of the packet-input will be lost). */
	uint64_t pkt_rr                       : 1;  /**< When set '1' the input packet selection will be
                                                         made with a Round Robin arbitration. When '0'
                                                         the input packet ring is fixed in priority,
                                                         where the lower ring number has higher priority. */
	uint64_t pbp_dhi                      : 13; /**< PBP_DHI replaces address bits that are used
                                                         for parse mode and skip-length when
                                                         SLI_PKTi_INSTR_HEADER[PBP]=1.
                                                         PBP_DHI becomes either MACADD<63:55> or MACADD<59:51>
                                                         for the instruction DPTR reads in this case.
                                                         The instruction DPTR reads are called
                                                         "First Direct" or "First Indirect" in the HRM.
                                                         When PBP=1, if "First Direct" and USE_CSR=0, PBP_DHI
                                                         becomes MACADD<59:51>, else MACADD<63:55>. */
	uint64_t d_nsr                        : 1;  /**< ADDRTYPE<1> or MACADD<61> for packet input data
                                                         reads.
                                                         D_NSR becomes either ADDRTYPE<1> or MACADD<61>
                                                         for MAC memory space reads of packet input data
                                                         fetched for any packet input ring.
                                                         ADDRTYPE<1> if USE_CSR=1, else MACADD<61>.
                                                         In the latter case, ADDRTYPE<1> comes from DPTR<61>.
                                                         ADDRTYPE<1> is the no-snoop attribute for PCIe
                                                         , helps select an SRIO*_S2M_TYPE* entry with sRIO. */
	uint64_t d_esr                        : 2;  /**< ES<1:0> or MACADD<63:62> for packet input data
                                                         reads.
                                                         D_ESR becomes either ES<1:0> or MACADD<63:62>
                                                         for MAC memory space reads of packet input data
                                                         fetched for any packet input ring.
                                                         ES<1:0> if USE_CSR=1, else MACADD<63:62>.
                                                         In the latter case, ES<1:0> comes from DPTR<63:62>.
                                                         ES<1:0> is the endian-swap attribute for these MAC
                                                         memory space reads. */
	uint64_t d_ror                        : 1;  /**< ADDRTYPE<0> or MACADD<60> for packet input data
                                                         reads.
                                                         D_ROR becomes either ADDRTYPE<0> or MACADD<60>
                                                         for MAC memory space reads of packet input data
                                                         fetched for any packet input ring.
                                                         ADDRTYPE<0> if USE_CSR=1, else MACADD<60>.
                                                         In the latter case, ADDRTYPE<0> comes from DPTR<60>.
                                                         ADDRTYPE<0> is the relaxed-order attribute for PCIe
                                                         , helps select an SRIO*_S2M_TYPE* entry with sRIO. */
	uint64_t use_csr                      : 1;  /**< When set '1' the csr value will be used for
                                                         ROR, ESR, and NSR. When clear '0' the value in
                                                         DPTR will be used. In turn the bits not used for
                                                         ROR, ESR, and NSR, will be used for bits [63:60]
                                                         of the address used to fetch packet data. */
	uint64_t nsr                          : 1;  /**< ADDRTYPE<1> for packet input instruction reads and
                                                         gather list (i.e. DPI component) reads from MAC
                                                         memory space.
                                                         ADDRTYPE<1> is the no-snoop attribute for PCIe
                                                         , helps select an SRIO*_S2M_TYPE* entry with sRIO. */
	uint64_t esr                          : 2;  /**< ES<1:0> for packet input instruction reads and
                                                         gather list (i.e. DPI component) reads from MAC
                                                         memory space.
                                                         ES<1:0> is the endian-swap attribute for these MAC
                                                         memory space reads. */
	uint64_t ror                          : 1;  /**< ADDRTYPE<0> for packet input instruction reads and
                                                         gather list (i.e. DPI component) reads from MAC
                                                         memory space.
                                                         ADDRTYPE<0> is the relaxed-order attribute for PCIe
                                                         , helps select an SRIO*_S2M_TYPE* entry with sRIO. */
#else
	uint64_t ror                          : 1;
	uint64_t esr                          : 2;
	uint64_t nsr                          : 1;
	uint64_t use_csr                      : 1;
	uint64_t d_ror                        : 1;
	uint64_t d_esr                        : 2;
	uint64_t d_nsr                        : 1;
	uint64_t pbp_dhi                      : 13;
	uint64_t pkt_rr                       : 1;
	uint64_t pin_rst                      : 1;
	uint64_t reserved_24_39               : 16;
	uint64_t prc_idle                     : 1;
	uint64_t reserved_41_47               : 7;
	uint64_t gii_rds                      : 7;
	uint64_t gii_erst                     : 1;
	uint64_t prd_rds                      : 7;
	uint64_t prd_erst                     : 1;
#endif
	} s;
	struct cvmx_sli_pkt_input_control_s   cn61xx;
	struct cvmx_sli_pkt_input_control_cn63xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_23_63               : 41;
	uint64_t pkt_rr                       : 1;  /**< When set '1' the input packet selection will be
                                                         made with a Round Robin arbitration. When '0'
                                                         the input packet ring is fixed in priority,
                                                         where the lower ring number has higher priority. */
	uint64_t pbp_dhi                      : 13; /**< PBP_DHI replaces address bits that are used
                                                         for parse mode and skip-length when
                                                         SLI_PKTi_INSTR_HEADER[PBP]=1.
                                                         PBP_DHI becomes either MACADD<63:55> or MACADD<59:51>
                                                         for the instruction DPTR reads in this case.
                                                         The instruction DPTR reads are called
                                                         "First Direct" or "First Indirect" in the HRM.
                                                         When PBP=1, if "First Direct" and USE_CSR=0, PBP_DHI
                                                         becomes MACADD<59:51>, else MACADD<63:55>. */
	uint64_t d_nsr                        : 1;  /**< ADDRTYPE<1> or MACADD<61> for packet input data
                                                         reads.
                                                         D_NSR becomes either ADDRTYPE<1> or MACADD<61>
                                                         for MAC memory space reads of packet input data
                                                         fetched for any packet input ring.
                                                         ADDRTYPE<1> if USE_CSR=1, else MACADD<61>.
                                                         In the latter case, ADDRTYPE<1> comes from DPTR<61>.
                                                         ADDRTYPE<1> is the no-snoop attribute for PCIe
                                                         , helps select an SRIO*_S2M_TYPE* entry with sRIO. */
	uint64_t d_esr                        : 2;  /**< ES<1:0> or MACADD<63:62> for packet input data
                                                         reads.
                                                         D_ESR becomes either ES<1:0> or MACADD<63:62>
                                                         for MAC memory space reads of packet input data
                                                         fetched for any packet input ring.
                                                         ES<1:0> if USE_CSR=1, else MACADD<63:62>.
                                                         In the latter case, ES<1:0> comes from DPTR<63:62>.
                                                         ES<1:0> is the endian-swap attribute for these MAC
                                                         memory space reads. */
	uint64_t d_ror                        : 1;  /**< ADDRTYPE<0> or MACADD<60> for packet input data
                                                         reads.
                                                         D_ROR becomes either ADDRTYPE<0> or MACADD<60>
                                                         for MAC memory space reads of packet input data
                                                         fetched for any packet input ring.
                                                         ADDRTYPE<0> if USE_CSR=1, else MACADD<60>.
                                                         In the latter case, ADDRTYPE<0> comes from DPTR<60>.
                                                         ADDRTYPE<0> is the relaxed-order attribute for PCIe
                                                         , helps select an SRIO*_S2M_TYPE* entry with sRIO. */
	uint64_t use_csr                      : 1;  /**< When set '1' the csr value will be used for
                                                         ROR, ESR, and NSR. When clear '0' the value in
                                                         DPTR will be used. In turn the bits not used for
                                                         ROR, ESR, and NSR, will be used for bits [63:60]
                                                         of the address used to fetch packet data. */
	uint64_t nsr                          : 1;  /**< ADDRTYPE<1> for packet input instruction reads and
                                                         gather list (i.e. DPI component) reads from MAC
                                                         memory space.
                                                         ADDRTYPE<1> is the no-snoop attribute for PCIe
                                                         , helps select an SRIO*_S2M_TYPE* entry with sRIO. */
	uint64_t esr                          : 2;  /**< ES<1:0> for packet input instruction reads and
                                                         gather list (i.e. DPI component) reads from MAC
                                                         memory space.
                                                         ES<1:0> is the endian-swap attribute for these MAC
                                                         memory space reads. */
	uint64_t ror                          : 1;  /**< ADDRTYPE<0> for packet input instruction reads and
                                                         gather list (i.e. DPI component) reads from MAC
                                                         memory space.
                                                         ADDRTYPE<0> is the relaxed-order attribute for PCIe
                                                         , helps select an SRIO*_S2M_TYPE* entry with sRIO. */
#else
	uint64_t ror                          : 1;
	uint64_t esr                          : 2;
	uint64_t nsr                          : 1;
	uint64_t use_csr                      : 1;
	uint64_t d_ror                        : 1;
	uint64_t d_esr                        : 2;
	uint64_t d_nsr                        : 1;
	uint64_t pbp_dhi                      : 13;
	uint64_t pkt_rr                       : 1;
	uint64_t reserved_23_63               : 41;
#endif
	} cn63xx;
	struct cvmx_sli_pkt_input_control_cn63xx cn63xxp1;
	struct cvmx_sli_pkt_input_control_s   cn66xx;
	struct cvmx_sli_pkt_input_control_s   cn68xx;
	struct cvmx_sli_pkt_input_control_s   cn68xxp1;
	struct cvmx_sli_pkt_input_control_s   cn70xx;
	struct cvmx_sli_pkt_input_control_s   cnf71xx;
};
typedef union cvmx_sli_pkt_input_control cvmx_sli_pkt_input_control_t;

/**
 * cvmx_sli_pkt_instr_enb
 *
 * "This register enables the instruction fetch for a packet ring. This is the PF version also
 * see SLI_PKT#_INPUT_CONTROL[ENB]."
 */
union cvmx_sli_pkt_instr_enb {
	uint64_t u64;
	struct cvmx_sli_pkt_instr_enb_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t enb                          : 64; /**< When ENB<i>=1, instruction input ring i is enabled. */
#else
	uint64_t enb                          : 64;
#endif
	} s;
	struct cvmx_sli_pkt_instr_enb_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t enb                          : 32; /**< When ENB<i>=1, instruction input ring i is enabled. */
#else
	uint64_t enb                          : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} cn61xx;
	struct cvmx_sli_pkt_instr_enb_cn61xx  cn63xx;
	struct cvmx_sli_pkt_instr_enb_cn61xx  cn63xxp1;
	struct cvmx_sli_pkt_instr_enb_cn61xx  cn66xx;
	struct cvmx_sli_pkt_instr_enb_cn61xx  cn68xx;
	struct cvmx_sli_pkt_instr_enb_cn61xx  cn68xxp1;
	struct cvmx_sli_pkt_instr_enb_cn61xx  cn70xx;
	struct cvmx_sli_pkt_instr_enb_s       cn78xx;
	struct cvmx_sli_pkt_instr_enb_cn61xx  cnf71xx;
};
typedef union cvmx_sli_pkt_instr_enb cvmx_sli_pkt_instr_enb_t;

/**
 * cvmx_sli_pkt_instr_rd_size
 *
 * The number of instruction allowed to be read at one time.
 *
 */
union cvmx_sli_pkt_instr_rd_size {
	uint64_t u64;
	struct cvmx_sli_pkt_instr_rd_size_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rdsize                       : 64; /**< Number of instructions to be read in one MAC read
                                                         request for the 4 ports - 8 rings. Every two bits
                                                         (i.e. 1:0, 3:2, 5:4..) are assign to the port/ring
                                                         combinations.
                                                         - 15:0  PKIPort0,Ring 7..0  31:16 PKIPort1,Ring 7..0
                                                         - 47:32 PKIPort2,Ring 7..0  63:48 PKIPort3,Ring 7..0
                                                         Two bit value are:
                                                         0 - 1 Instruction
                                                         1 - 2 Instructions
                                                         2 - 3 Instructions
                                                         3 - 4 Instructions */
#else
	uint64_t rdsize                       : 64;
#endif
	} s;
	struct cvmx_sli_pkt_instr_rd_size_s   cn61xx;
	struct cvmx_sli_pkt_instr_rd_size_s   cn63xx;
	struct cvmx_sli_pkt_instr_rd_size_s   cn63xxp1;
	struct cvmx_sli_pkt_instr_rd_size_s   cn66xx;
	struct cvmx_sli_pkt_instr_rd_size_s   cn68xx;
	struct cvmx_sli_pkt_instr_rd_size_s   cn68xxp1;
	struct cvmx_sli_pkt_instr_rd_size_s   cn70xx;
	struct cvmx_sli_pkt_instr_rd_size_s   cnf71xx;
};
typedef union cvmx_sli_pkt_instr_rd_size cvmx_sli_pkt_instr_rd_size_t;

/**
 * cvmx_sli_pkt_instr_size
 *
 * Determines if instructions are 64 or 32 byte in size for a Packet-ring.
 *
 */
union cvmx_sli_pkt_instr_size {
	uint64_t u64;
	struct cvmx_sli_pkt_instr_size_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t is_64b                       : 32; /**< When IS_64B<i>=1, instruction input ring i uses 64B
                                                         instructions, else 32B instructions. */
#else
	uint64_t is_64b                       : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_pkt_instr_size_s      cn61xx;
	struct cvmx_sli_pkt_instr_size_s      cn63xx;
	struct cvmx_sli_pkt_instr_size_s      cn63xxp1;
	struct cvmx_sli_pkt_instr_size_s      cn66xx;
	struct cvmx_sli_pkt_instr_size_s      cn68xx;
	struct cvmx_sli_pkt_instr_size_s      cn68xxp1;
	struct cvmx_sli_pkt_instr_size_s      cn70xx;
	struct cvmx_sli_pkt_instr_size_s      cnf71xx;
};
typedef union cvmx_sli_pkt_instr_size cvmx_sli_pkt_instr_size_t;

/**
 * cvmx_sli_pkt_int
 *
 * When read by a VF, this register informs which rings owned by the VF (0 to 63) have an
 * interrupt pending. In PF mode, this register returns an unpredictable value. Writing 1s to
 * clear this register clears both packet count an packet time interrupts. The clearing of the
 * interrupts will be reflected in SLI_PKT_CNT_INT and SLI_PKT_TIME_INT.
 */
union cvmx_sli_pkt_int {
	uint64_t u64;
	struct cvmx_sli_pkt_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ring                         : 64; /**< Ring 63-0 has packet count or packet time interrupt. */
#else
	uint64_t ring                         : 64;
#endif
	} s;
	struct cvmx_sli_pkt_int_s             cn78xx;
};
typedef union cvmx_sli_pkt_int cvmx_sli_pkt_int_t;

/**
 * cvmx_sli_pkt_int_levels
 *
 * SLI_PKT_INT_LEVELS = SLI's Packet Interrupt Levels
 * Output packet interrupt levels.
 */
union cvmx_sli_pkt_int_levels {
	uint64_t u64;
	struct cvmx_sli_pkt_int_levels_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_54_63               : 10;
	uint64_t time                         : 22; /**< Output ring counter time interrupt threshold
                                                         SLI sets SLI_PKT_TIME_INT[PORT<i>] whenever
                                                         SLI_PKTi_CNTS[TIMER] > TIME */
	uint64_t cnt                          : 32; /**< Output ring counter interrupt threshold
                                                         SLI sets SLI_PKT_CNT_INT[PORT<i>] whenever
                                                         SLI_PKTi_CNTS[CNT] > CNT */
#else
	uint64_t cnt                          : 32;
	uint64_t time                         : 22;
	uint64_t reserved_54_63               : 10;
#endif
	} s;
	struct cvmx_sli_pkt_int_levels_s      cn61xx;
	struct cvmx_sli_pkt_int_levels_s      cn63xx;
	struct cvmx_sli_pkt_int_levels_s      cn63xxp1;
	struct cvmx_sli_pkt_int_levels_s      cn66xx;
	struct cvmx_sli_pkt_int_levels_s      cn68xx;
	struct cvmx_sli_pkt_int_levels_s      cn68xxp1;
	struct cvmx_sli_pkt_int_levels_s      cn70xx;
	struct cvmx_sli_pkt_int_levels_s      cnf71xx;
};
typedef union cvmx_sli_pkt_int_levels cvmx_sli_pkt_int_levels_t;

/**
 * cvmx_sli_pkt_iptr
 *
 * Controls using the Info-Pointer to store length and data.
 *
 */
union cvmx_sli_pkt_iptr {
	uint64_t u64;
	struct cvmx_sli_pkt_iptr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t iptr                         : 32; /**< When IPTR<i>=1, packet output ring i is in info-
                                                         pointer mode, else buffer-pointer-only mode. */
#else
	uint64_t iptr                         : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_pkt_iptr_s            cn61xx;
	struct cvmx_sli_pkt_iptr_s            cn63xx;
	struct cvmx_sli_pkt_iptr_s            cn63xxp1;
	struct cvmx_sli_pkt_iptr_s            cn66xx;
	struct cvmx_sli_pkt_iptr_s            cn68xx;
	struct cvmx_sli_pkt_iptr_s            cn68xxp1;
	struct cvmx_sli_pkt_iptr_s            cn70xx;
	struct cvmx_sli_pkt_iptr_s            cnf71xx;
};
typedef union cvmx_sli_pkt_iptr cvmx_sli_pkt_iptr_t;

/**
 * cvmx_sli_pkt_mac#_rinfo
 *
 * This register sets the total number and starting number of rings used by the MAC.
 *
 */
union cvmx_sli_pkt_macx_rinfo {
	uint64_t u64;
	struct cvmx_sli_pkt_macx_rinfo_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_40_63               : 24;
	uint64_t rpvf                         : 8;  /**< The number of rings assigned to a VF for this MAC. Legal values are 0 to 64. */
	uint64_t reserved_24_31               : 8;
	uint64_t trs                          : 8;  /**< The number of rings assigned to the MAC. Legal value are 0 to 64. */
	uint64_t reserved_7_15                : 9;
	uint64_t srn                          : 7;  /**< The starting ring number used by the MAC. Legal value are 0 to 63. */
#else
	uint64_t srn                          : 7;
	uint64_t reserved_7_15                : 9;
	uint64_t trs                          : 8;
	uint64_t reserved_24_31               : 8;
	uint64_t rpvf                         : 8;
	uint64_t reserved_40_63               : 24;
#endif
	} s;
	struct cvmx_sli_pkt_macx_rinfo_s      cn78xx;
};
typedef union cvmx_sli_pkt_macx_rinfo cvmx_sli_pkt_macx_rinfo_t;

/**
 * cvmx_sli_pkt_mac0_sig0
 *
 * This register is used to signal between PF/VF. This CSR can be R/W by the PF from MAC0 and any VF.
 *
 */
union cvmx_sli_pkt_mac0_sig0 {
	uint64_t u64;
	struct cvmx_sli_pkt_mac0_sig0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Field can be read or written to by PF and any VF. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_sli_pkt_mac0_sig0_s       cn78xx;
};
typedef union cvmx_sli_pkt_mac0_sig0 cvmx_sli_pkt_mac0_sig0_t;

/**
 * cvmx_sli_pkt_mac0_sig1
 *
 * This register is used to signal between PF/VF. This CSR can be R/W by the PF from MAC0 and any VF.
 *
 */
union cvmx_sli_pkt_mac0_sig1 {
	uint64_t u64;
	struct cvmx_sli_pkt_mac0_sig1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Field can be read or written to by PF and any VF. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_sli_pkt_mac0_sig1_s       cn78xx;
};
typedef union cvmx_sli_pkt_mac0_sig1 cvmx_sli_pkt_mac0_sig1_t;

/**
 * cvmx_sli_pkt_mac1_sig0
 *
 * This register is used to signal between PF/VF. This CSR can be R/W by the PF from MAC1 and any VF.
 *
 */
union cvmx_sli_pkt_mac1_sig0 {
	uint64_t u64;
	struct cvmx_sli_pkt_mac1_sig0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Field can be read or written to by PF and any VF. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_sli_pkt_mac1_sig0_s       cn78xx;
};
typedef union cvmx_sli_pkt_mac1_sig0 cvmx_sli_pkt_mac1_sig0_t;

/**
 * cvmx_sli_pkt_mac1_sig1
 *
 * This register is used to signal between PF/VF. This CSR can be R/W by the PF from MAC1 and any VF.
 *
 */
union cvmx_sli_pkt_mac1_sig1 {
	uint64_t u64;
	struct cvmx_sli_pkt_mac1_sig1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Field can be read or written to by PF and any VF. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_sli_pkt_mac1_sig1_s       cn78xx;
};
typedef union cvmx_sli_pkt_mac1_sig1 cvmx_sli_pkt_mac1_sig1_t;

/**
 * cvmx_sli_pkt_mem_ctl
 *
 * This register controls the ECC of the SLI packet memories.
 *
 */
union cvmx_sli_pkt_mem_ctl {
	uint64_t u64;
	struct cvmx_sli_pkt_mem_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_44_63               : 20;
	uint64_t msid_fs                      : 2;  /**< Used to flip the synd. for pcsr_ncsr_msix_data_flip_synd. */
	uint64_t msia_fs                      : 2;  /**< Used to flip the synd. for pcsr_ncsr_msix_addr_flip_synd. */
	uint64_t msi_ecc                      : 1;  /**< When set pcsr_ncsr_msix_ecc_enawill have an ECC not generated and checked. */
	uint64_t posi_fs                      : 2;  /**< Used to flip the synd. for pout_signal_csr_flip_synd. */
	uint64_t posi_ecc                     : 1;  /**< When set pout_signal_csr_cor_dis will have an ECC not generated and checked. */
	uint64_t pos_fs                       : 2;  /**< Used to flip the synd. for pcsr_pout_size_csr_flip_synd. */
	uint64_t pos_ecc                      : 1;  /**< When set  will have an ECC not generated and checked. */
	uint64_t pinm_fs                      : 2;  /**< Used to flip the synd. for pcsr_instr_mem_csr_flip_synd. */
	uint64_t pinm_ecc                     : 1;  /**< When set pcsr_instr_mem_csr_cor_dis will have an ECC not generated and checked. */
	uint64_t pind_fs                      : 2;  /**< Used to flip the synd. for pcsr_in_done_csr_flip_synd. */
	uint64_t pind_ecc                     : 1;  /**< When set pcsr_in_done_csr_cor_dis will have an ECC not generated and checked. */
	uint64_t point_fs                     : 2;  /**< Used to flip the synd. for pout_int_csr_flip_synd. */
	uint64_t point_ecc                    : 1;  /**< When set pout_int_csr_cor_dis will have an ECC not generated and checked. */
	uint64_t slist_fs                     : 2;  /**< Used to flip the synd. for pcsr_slist_csr_flip_synd. */
	uint64_t slist_ecc                    : 1;  /**< When set pcsr_slist_csr_cor_dis will have an ECC not generated and checked. */
	uint64_t pop1_fs                      : 2;  /**< Used to flip the synd for packet-out-pointer memory1. */
	uint64_t pop1_ecc                     : 1;  /**< When set Packet Out Pointer memory1 will have an ECC not generated and checked. */
	uint64_t pop0_fs                      : 2;  /**< Used to flip the synd for packet-out-pointer memory0. */
	uint64_t pop0_ecc                     : 1;  /**< When set packet-out-pointer memory0 will have an ECC not generated and checked. */
	uint64_t pfp_fs                       : 2;  /**< Used to flip the synd for packet-out-pointer memory. */
	uint64_t pfp_ecc                      : 1;  /**< When set packet-out-pointer memory will have an ECC not generated and checked. */
	uint64_t pbn_fs                       : 2;  /**< Used to flip the synd for pointer-base-number memory. */
	uint64_t pbn_ecc                      : 1;  /**< When set pointer-base-number memory will have an ECC not generated and checked. */
	uint64_t pdf_fs                       : 2;  /**< Used to flip the synd for packet-data-info memory. */
	uint64_t pdf_ecc                      : 1;  /**< When set packet data memory will have an ECC not generated and checked. */
	uint64_t psf_fs                       : 2;  /**< Used to flip the synd for PSF memory. */
	uint64_t psf_ecc                      : 1;  /**< When set PSF memory will have an ECC not generated and checked. */
	uint64_t poi_fs                       : 2;  /**< Used to flip the synd for packet-out-info memory. */
	uint64_t poi_ecc                      : 1;  /**< When set Packet Out Info memory will have an ECC not generated and checked. */
#else
	uint64_t poi_ecc                      : 1;
	uint64_t poi_fs                       : 2;
	uint64_t psf_ecc                      : 1;
	uint64_t psf_fs                       : 2;
	uint64_t pdf_ecc                      : 1;
	uint64_t pdf_fs                       : 2;
	uint64_t pbn_ecc                      : 1;
	uint64_t pbn_fs                       : 2;
	uint64_t pfp_ecc                      : 1;
	uint64_t pfp_fs                       : 2;
	uint64_t pop0_ecc                     : 1;
	uint64_t pop0_fs                      : 2;
	uint64_t pop1_ecc                     : 1;
	uint64_t pop1_fs                      : 2;
	uint64_t slist_ecc                    : 1;
	uint64_t slist_fs                     : 2;
	uint64_t point_ecc                    : 1;
	uint64_t point_fs                     : 2;
	uint64_t pind_ecc                     : 1;
	uint64_t pind_fs                      : 2;
	uint64_t pinm_ecc                     : 1;
	uint64_t pinm_fs                      : 2;
	uint64_t pos_ecc                      : 1;
	uint64_t pos_fs                       : 2;
	uint64_t posi_ecc                     : 1;
	uint64_t posi_fs                      : 2;
	uint64_t msi_ecc                      : 1;
	uint64_t msia_fs                      : 2;
	uint64_t msid_fs                      : 2;
	uint64_t reserved_44_63               : 20;
#endif
	} s;
	struct cvmx_sli_pkt_mem_ctl_s         cn78xx;
};
typedef union cvmx_sli_pkt_mem_ctl cvmx_sli_pkt_mem_ctl_t;

/**
 * cvmx_sli_pkt_out_bmode
 *
 * Control the updating of the SLI_PKT#_CNT register.
 *
 */
union cvmx_sli_pkt_out_bmode {
	uint64_t u64;
	struct cvmx_sli_pkt_out_bmode_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t bmode                        : 32; /**< Determines whether SLI_PKTi_CNTS[CNT] is a byte or
                                                         packet counter.
                                                         When BMODE<i>=1, SLI_PKTi_CNTS[CNT] is a byte
                                                         counter, else SLI_PKTi_CNTS[CNT] is a packet
                                                         counter. */
#else
	uint64_t bmode                        : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_pkt_out_bmode_s       cn61xx;
	struct cvmx_sli_pkt_out_bmode_s       cn63xx;
	struct cvmx_sli_pkt_out_bmode_s       cn63xxp1;
	struct cvmx_sli_pkt_out_bmode_s       cn66xx;
	struct cvmx_sli_pkt_out_bmode_s       cn68xx;
	struct cvmx_sli_pkt_out_bmode_s       cn68xxp1;
	struct cvmx_sli_pkt_out_bmode_s       cn70xx;
	struct cvmx_sli_pkt_out_bmode_s       cnf71xx;
};
typedef union cvmx_sli_pkt_out_bmode cvmx_sli_pkt_out_bmode_t;

/**
 * cvmx_sli_pkt_out_bp_en
 *
 * This register enables sending backpressure to PKO.
 *
 */
union cvmx_sli_pkt_out_bp_en {
	uint64_t u64;
	struct cvmx_sli_pkt_out_bp_en_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bp_en                        : 64; /**< When set, enable the channel-level backpressure to be sent to PKO. Backpressure is sent to
                                                         the PKO on the channels 0x100-0x13F. */
#else
	uint64_t bp_en                        : 64;
#endif
	} s;
	struct cvmx_sli_pkt_out_bp_en_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t bp_en                        : 32; /**< When set '1' enable the ring level backpressure
                                                         to be sent to PKO. Backpressure is sent to the
                                                         PKO on the PIPE number associated with the ring.
                                                         (See SLI_TX_PIPE for ring to pipe associations). */
#else
	uint64_t bp_en                        : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} cn68xx;
	struct cvmx_sli_pkt_out_bp_en_cn68xx  cn68xxp1;
	struct cvmx_sli_pkt_out_bp_en_s       cn78xx;
};
typedef union cvmx_sli_pkt_out_bp_en cvmx_sli_pkt_out_bp_en_t;

/**
 * cvmx_sli_pkt_out_enb
 *
 * "This register enables the output packet engines. This is the PF version. Also see
 * SLI_PKT#_OUTPUT_CONTROL[ENB]."
 */
union cvmx_sli_pkt_out_enb {
	uint64_t u64;
	struct cvmx_sli_pkt_out_enb_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t enb                          : 64; /**< When ENB<i>=1, packet output ring i is enabled.
                                                         If an error occurs on reading pointers for an
                                                         output ring, the ring will be disabled by clearing
                                                         the bit associated with the ring to '0'. */
#else
	uint64_t enb                          : 64;
#endif
	} s;
	struct cvmx_sli_pkt_out_enb_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t enb                          : 32; /**< When ENB<i>=1, packet output ring i is enabled.
                                                         If an error occurs on reading pointers for an
                                                         output ring, the ring will be disabled by clearing
                                                         the bit associated with the ring to '0'. */
#else
	uint64_t enb                          : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} cn61xx;
	struct cvmx_sli_pkt_out_enb_cn61xx    cn63xx;
	struct cvmx_sli_pkt_out_enb_cn61xx    cn63xxp1;
	struct cvmx_sli_pkt_out_enb_cn61xx    cn66xx;
	struct cvmx_sli_pkt_out_enb_cn61xx    cn68xx;
	struct cvmx_sli_pkt_out_enb_cn61xx    cn68xxp1;
	struct cvmx_sli_pkt_out_enb_cn61xx    cn70xx;
	struct cvmx_sli_pkt_out_enb_s         cn78xx;
	struct cvmx_sli_pkt_out_enb_cn61xx    cnf71xx;
};
typedef union cvmx_sli_pkt_out_enb cvmx_sli_pkt_out_enb_t;

/**
 * cvmx_sli_pkt_output_wmark
 *
 * This register sets the value that determines when backpressure is applied to the PKO. When
 * SLI_PKT(0..63)_SLIST_BAOFF_DBELL[DBELL] is less than [WMARK], backpressure is sent to PKO for
 * the associated channel.
 */
union cvmx_sli_pkt_output_wmark {
	uint64_t u64;
	struct cvmx_sli_pkt_output_wmark_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t wmark                        : 32; /**< Value when DBELL count drops below backpressure
                                                         for the ring will be applied to the PKO. */
#else
	uint64_t wmark                        : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_pkt_output_wmark_s    cn61xx;
	struct cvmx_sli_pkt_output_wmark_s    cn63xx;
	struct cvmx_sli_pkt_output_wmark_s    cn63xxp1;
	struct cvmx_sli_pkt_output_wmark_s    cn66xx;
	struct cvmx_sli_pkt_output_wmark_s    cn68xx;
	struct cvmx_sli_pkt_output_wmark_s    cn68xxp1;
	struct cvmx_sli_pkt_output_wmark_s    cn70xx;
	struct cvmx_sli_pkt_output_wmark_s    cn78xx;
	struct cvmx_sli_pkt_output_wmark_s    cnf71xx;
};
typedef union cvmx_sli_pkt_output_wmark cvmx_sli_pkt_output_wmark_t;

/**
 * cvmx_sli_pkt_pcie_port
 *
 * Assigns Packet Ports to MAC ports.
 *
 */
union cvmx_sli_pkt_pcie_port {
	uint64_t u64;
	struct cvmx_sli_pkt_pcie_port_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pp                           : 64; /**< The physical MAC  port that the output ring uses.
                                                         Two bits are used per ring (i.e. ring 0 [1:0],
                                                         ring 1 [3:2], ....). A value of '0 means
                                                         that the Packetring is assign to MAC Port 0, a '1'
                                                         MAC Port 1, '2' and '3' are reserved. */
#else
	uint64_t pp                           : 64;
#endif
	} s;
	struct cvmx_sli_pkt_pcie_port_s       cn61xx;
	struct cvmx_sli_pkt_pcie_port_s       cn63xx;
	struct cvmx_sli_pkt_pcie_port_s       cn63xxp1;
	struct cvmx_sli_pkt_pcie_port_s       cn66xx;
	struct cvmx_sli_pkt_pcie_port_s       cn68xx;
	struct cvmx_sli_pkt_pcie_port_s       cn68xxp1;
	struct cvmx_sli_pkt_pcie_port_s       cn70xx;
	struct cvmx_sli_pkt_pcie_port_s       cnf71xx;
};
typedef union cvmx_sli_pkt_pcie_port cvmx_sli_pkt_pcie_port_t;

/**
 * cvmx_sli_pkt_port_in_rst
 *
 * SLI_PKT_PORT_IN_RST = SLI Packet Port In Reset
 * Vector bits related to ring-port for ones that are reset.
 */
union cvmx_sli_pkt_port_in_rst {
	uint64_t u64;
	struct cvmx_sli_pkt_port_in_rst_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t in_rst                       : 32; /**< When asserted '1' the vector bit cooresponding
                                                         to the inbound Packet-ring is in reset. */
	uint64_t out_rst                      : 32; /**< When asserted '1' the vector bit cooresponding
                                                         to the outbound Packet-ring is in reset. */
#else
	uint64_t out_rst                      : 32;
	uint64_t in_rst                       : 32;
#endif
	} s;
	struct cvmx_sli_pkt_port_in_rst_s     cn61xx;
	struct cvmx_sli_pkt_port_in_rst_s     cn63xx;
	struct cvmx_sli_pkt_port_in_rst_s     cn63xxp1;
	struct cvmx_sli_pkt_port_in_rst_s     cn66xx;
	struct cvmx_sli_pkt_port_in_rst_s     cn68xx;
	struct cvmx_sli_pkt_port_in_rst_s     cn68xxp1;
	struct cvmx_sli_pkt_port_in_rst_s     cn70xx;
	struct cvmx_sli_pkt_port_in_rst_s     cnf71xx;
};
typedef union cvmx_sli_pkt_port_in_rst cvmx_sli_pkt_port_in_rst_t;

/**
 * cvmx_sli_pkt_ring_rst
 *
 * This register shows which rings are in reset. See also SLI_PKT(0..63)_INPUT_CONTROL[RST].
 *
 */
union cvmx_sli_pkt_ring_rst {
	uint64_t u64;
	struct cvmx_sli_pkt_ring_rst_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rst                          : 64; /**< Ring in reset. When asserted 1, the vector bit corresponding to the packet ring is in reset. */
#else
	uint64_t rst                          : 64;
#endif
	} s;
	struct cvmx_sli_pkt_ring_rst_s        cn78xx;
};
typedef union cvmx_sli_pkt_ring_rst cvmx_sli_pkt_ring_rst_t;

/**
 * cvmx_sli_pkt_slist_es
 *
 * The Endian Swap for Scatter List Read.
 *
 */
union cvmx_sli_pkt_slist_es {
	uint64_t u64;
	struct cvmx_sli_pkt_slist_es_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t es                           : 64; /**< ES<1:0> for the packet output ring reads that
                                                         fetch buffer/info pointer pairs.
                                                         ES<2i+1:2i> becomes ES<1:0> in DPI/SLI reads that
                                                         fetch buffer/info pairs from packet output ring i
                                                         (from address SLI_PKTi_SLIST_BADDR+ in MAC memory
                                                         space.)
                                                         ES<1:0> is the endian-swap attribute for these MAC
                                                         memory space reads. */
#else
	uint64_t es                           : 64;
#endif
	} s;
	struct cvmx_sli_pkt_slist_es_s        cn61xx;
	struct cvmx_sli_pkt_slist_es_s        cn63xx;
	struct cvmx_sli_pkt_slist_es_s        cn63xxp1;
	struct cvmx_sli_pkt_slist_es_s        cn66xx;
	struct cvmx_sli_pkt_slist_es_s        cn68xx;
	struct cvmx_sli_pkt_slist_es_s        cn68xxp1;
	struct cvmx_sli_pkt_slist_es_s        cn70xx;
	struct cvmx_sli_pkt_slist_es_s        cnf71xx;
};
typedef union cvmx_sli_pkt_slist_es cvmx_sli_pkt_slist_es_t;

/**
 * cvmx_sli_pkt_slist_ns
 *
 * The NS field for the TLP when fetching Scatter List.
 *
 */
union cvmx_sli_pkt_slist_ns {
	uint64_t u64;
	struct cvmx_sli_pkt_slist_ns_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t nsr                          : 32; /**< ADDRTYPE<1> for the packet output ring reads that
                                                         fetch buffer/info pointer pairs.
                                                         NSR<i> becomes ADDRTYPE<1> in DPI/SLI reads that
                                                         fetch buffer/info pairs from packet output ring i
                                                         (from address SLI_PKTi_SLIST_BADDR+ in MAC memory
                                                         space.)
                                                         ADDRTYPE<1> is the relaxed-order attribute for PCIe
                                                         , helps select an SRIO*_S2M_TYPE* entry with sRIO. */
#else
	uint64_t nsr                          : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_pkt_slist_ns_s        cn61xx;
	struct cvmx_sli_pkt_slist_ns_s        cn63xx;
	struct cvmx_sli_pkt_slist_ns_s        cn63xxp1;
	struct cvmx_sli_pkt_slist_ns_s        cn66xx;
	struct cvmx_sli_pkt_slist_ns_s        cn68xx;
	struct cvmx_sli_pkt_slist_ns_s        cn68xxp1;
	struct cvmx_sli_pkt_slist_ns_s        cn70xx;
	struct cvmx_sli_pkt_slist_ns_s        cnf71xx;
};
typedef union cvmx_sli_pkt_slist_ns cvmx_sli_pkt_slist_ns_t;

/**
 * cvmx_sli_pkt_slist_ror
 *
 * The ROR field for the TLP when fetching Scatter List.
 *
 */
union cvmx_sli_pkt_slist_ror {
	uint64_t u64;
	struct cvmx_sli_pkt_slist_ror_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t ror                          : 32; /**< ADDRTYPE<0> for the packet output ring reads that
                                                         fetch buffer/info pointer pairs.
                                                         ROR<i> becomes ADDRTYPE<0> in DPI/SLI reads that
                                                         fetch buffer/info pairs from packet output ring i
                                                         (from address SLI_PKTi_SLIST_BADDR+ in MAC memory
                                                         space.)
                                                         ADDRTYPE<0> is the relaxed-order attribute for PCIe
                                                         , helps select an SRIO*_S2M_TYPE* entry with sRIO. */
#else
	uint64_t ror                          : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_pkt_slist_ror_s       cn61xx;
	struct cvmx_sli_pkt_slist_ror_s       cn63xx;
	struct cvmx_sli_pkt_slist_ror_s       cn63xxp1;
	struct cvmx_sli_pkt_slist_ror_s       cn66xx;
	struct cvmx_sli_pkt_slist_ror_s       cn68xx;
	struct cvmx_sli_pkt_slist_ror_s       cn68xxp1;
	struct cvmx_sli_pkt_slist_ror_s       cn70xx;
	struct cvmx_sli_pkt_slist_ror_s       cnf71xx;
};
typedef union cvmx_sli_pkt_slist_ror cvmx_sli_pkt_slist_ror_t;

/**
 * cvmx_sli_pkt_time_int
 *
 * The packets rings that are interrupting because of Packet Timers.
 *
 */
union cvmx_sli_pkt_time_int {
	uint64_t u64;
	struct cvmx_sli_pkt_time_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_sli_pkt_time_int_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t port                         : 32; /**< Output ring packet timer interrupt bits
                                                         SLI sets PORT<i> whenever
                                                         SLI_PKTi_CNTS[TIMER] > SLI_PKT_INT_LEVELS[TIME].
                                                         SLI_PKT_TIME_INT_ENB[PORT<i>] is the corresponding
                                                         enable. */
#else
	uint64_t port                         : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} cn61xx;
	struct cvmx_sli_pkt_time_int_cn61xx   cn63xx;
	struct cvmx_sli_pkt_time_int_cn61xx   cn63xxp1;
	struct cvmx_sli_pkt_time_int_cn61xx   cn66xx;
	struct cvmx_sli_pkt_time_int_cn61xx   cn68xx;
	struct cvmx_sli_pkt_time_int_cn61xx   cn68xxp1;
	struct cvmx_sli_pkt_time_int_cn61xx   cn70xx;
	struct cvmx_sli_pkt_time_int_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ring                         : 64; /**< Output ring packet timer interrupt bits
                                                         SLI sets RING<i> whenever
                                                         SLI_PKTi_CNTS[TIMER] > SLI_PKT_INT_LEVELS[TIME].
                                                         SLI_PKT_TIME_INT_ENB[RING<i>] is the corresponding
                                                         enable. */
#else
	uint64_t ring                         : 64;
#endif
	} cn78xx;
	struct cvmx_sli_pkt_time_int_cn61xx   cnf71xx;
};
typedef union cvmx_sli_pkt_time_int cvmx_sli_pkt_time_int_t;

/**
 * cvmx_sli_pkt_time_int_enb
 *
 * The packets rings that are interrupting because of Packet Timers.
 *
 */
union cvmx_sli_pkt_time_int_enb {
	uint64_t u64;
	struct cvmx_sli_pkt_time_int_enb_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t port                         : 32; /**< Output ring packet timer interrupt enables
                                                         When both PORT<i> and corresponding
                                                         SLI_PKT_TIME_INT[PORT<i>] are set, for any i,
                                                         then SLI_INT_SUM[PTIME] is set, which can cause
                                                         an interrupt. */
#else
	uint64_t port                         : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sli_pkt_time_int_enb_s    cn61xx;
	struct cvmx_sli_pkt_time_int_enb_s    cn63xx;
	struct cvmx_sli_pkt_time_int_enb_s    cn63xxp1;
	struct cvmx_sli_pkt_time_int_enb_s    cn66xx;
	struct cvmx_sli_pkt_time_int_enb_s    cn68xx;
	struct cvmx_sli_pkt_time_int_enb_s    cn68xxp1;
	struct cvmx_sli_pkt_time_int_enb_s    cn70xx;
	struct cvmx_sli_pkt_time_int_enb_s    cnf71xx;
};
typedef union cvmx_sli_pkt_time_int_enb cvmx_sli_pkt_time_int_enb_t;

/**
 * cvmx_sli_port#_pkind
 *
 * SLI_PORT[0..31]_PKIND = SLI Port Pkind
 *
 * The SLI/DPI supports 32 input rings for fetching input packets. This register maps the input-rings (0-31) to a PKIND.
 */
union cvmx_sli_portx_pkind {
	uint64_t u64;
	struct cvmx_sli_portx_pkind_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_25_63               : 39;
	uint64_t rpk_enb                      : 1;  /**< Alternate PKT_INST_HDR PKind Enable for this ring.
                                                         When RPK_ENB==1 and DPI prepends
                                                         a PKT_INST_HDR to a packet, the pkind for the
                                                         packet is PKINDR (rather than PKIND), and any
                                                         special PIP/IPD processing of the DPI packet is
                                                         disabled (see PIP_PRT_CFG*[INST_HDR,HIGIG_EN]).
                                                         (DPI prepends a PKT_INST_HDR when either
                                                         DPI_INST_HDR[R]==1 for the packet or
                                                         SLI_PKT*_INSTR_HEADER[USE_IHDR]==1 for the ring.)
                                                         When RPK_ENB==0, PKIND is the pkind for all
                                                         packets through the input ring, and
                                                         PIP/IPD will process a DPI packet that has a
                                                         PKT_INST_HDR specially. */
	uint64_t reserved_22_23               : 2;
	uint64_t pkindr                       : 6;  /**< Port Kind For this Ring used with packets
                                                         that include a DPI-prepended PKT_INST_HDR
                                                         when RPK_ENB is set. */
	uint64_t reserved_14_15               : 2;
	uint64_t bpkind                       : 6;  /**< Back-pressure pkind for this Ring. */
	uint64_t reserved_6_7                 : 2;
	uint64_t pkind                        : 6;  /**< Port Kind For this Ring. */
#else
	uint64_t pkind                        : 6;
	uint64_t reserved_6_7                 : 2;
	uint64_t bpkind                       : 6;
	uint64_t reserved_14_15               : 2;
	uint64_t pkindr                       : 6;
	uint64_t reserved_22_23               : 2;
	uint64_t rpk_enb                      : 1;
	uint64_t reserved_25_63               : 39;
#endif
	} s;
	struct cvmx_sli_portx_pkind_s         cn68xx;
	struct cvmx_sli_portx_pkind_cn68xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t bpkind                       : 6;  /**< Back-pressure pkind for this Ring. */
	uint64_t reserved_6_7                 : 2;
	uint64_t pkind                        : 6;  /**< Port Kind For this Ring. */
#else
	uint64_t pkind                        : 6;
	uint64_t reserved_6_7                 : 2;
	uint64_t bpkind                       : 6;
	uint64_t reserved_14_63               : 50;
#endif
	} cn68xxp1;
};
typedef union cvmx_sli_portx_pkind cvmx_sli_portx_pkind_t;

/**
 * cvmx_sli_s2m_port#_ctl
 *
 * These registers contain control for access from SLI to a MAC port. Write operations to these
 * register are not ordered with write/read operations to the MAC Memory space. To ensure that a
 * write operation has completed, read the register before making an access (i.e. MAC memory
 * space) that requires the value of this register to be updated.
 */
union cvmx_sli_s2m_portx_ctl {
	uint64_t u64;
	struct cvmx_sli_s2m_portx_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t lcl_node                     : 1;  /**< Local OCI node. When set to 1, all window access is treated as local OCI access. Normally,
                                                         if address bits [37:36] of the window address CSRs are not equal to the chip's OCI value,
                                                         the window operation is sent to the OCI for remote chip access. This field, when set,
                                                         disables this and treats all access to be for the local OCI. */
	uint64_t wind_d                       : 1;  /**< When set '1' disables access to the Window
                                                         Registers from the MAC-Port.
                                                         When Authenticate-Mode is set the reset value of
                                                         this field is "1" else "0'. */
	uint64_t bar0_d                       : 1;  /**< When set '1' disables access from MAC to
                                                         BAR-0 address offsets: Less Than 0x330,
                                                         0x3CD0, and greater than 0x3D70 excluding
                                                         0x3e00.
                                                         When Authenticate-Mode is set the reset value of
                                                         this field is "1" else "0'. */
	uint64_t reserved_0_2                 : 3;
#else
	uint64_t reserved_0_2                 : 3;
	uint64_t bar0_d                       : 1;
	uint64_t wind_d                       : 1;
	uint64_t lcl_node                     : 1;
	uint64_t reserved_6_63                : 58;
#endif
	} s;
	struct cvmx_sli_s2m_portx_ctl_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t wind_d                       : 1;  /**< When set '1' disables access to the Window
                                                         Registers from the MAC-Port.
                                                         When Authenticate-Mode is set the reset value of
                                                         this field is "1" else "0'. */
	uint64_t bar0_d                       : 1;  /**< When set '1' disables access from MAC to
                                                         BAR-0 address offsets: Less Than 0x330,
                                                         0x3CD0, and greater than 0x3D70 excluding
                                                         0x3e00.
                                                         When Authenticate-Mode is set the reset value of
                                                         this field is "1" else "0'. */
	uint64_t mrrs                         : 3;  /**< Max Read Request Size
                                                                 0 = 128B
                                                                 1 = 256B
                                                                 2 = 512B
                                                                 3 = 1024B
                                                                 4 = 2048B
                                                                 5-7 = Reserved
                                                         This field should not exceed the desired
                                                               max read request size. This field is used to
                                                               determine if an IOBDMA is too large.
                                                         For a PCIe MAC, this field should not exceed
                                                               PCIE*_CFG030[MRRS].
                                                         For a sRIO MAC, this field should indicate a size
                                                               of 256B or smaller. */
#else
	uint64_t mrrs                         : 3;
	uint64_t bar0_d                       : 1;
	uint64_t wind_d                       : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} cn61xx;
	struct cvmx_sli_s2m_portx_ctl_cn61xx  cn63xx;
	struct cvmx_sli_s2m_portx_ctl_cn61xx  cn63xxp1;
	struct cvmx_sli_s2m_portx_ctl_cn61xx  cn66xx;
	struct cvmx_sli_s2m_portx_ctl_cn61xx  cn68xx;
	struct cvmx_sli_s2m_portx_ctl_cn61xx  cn68xxp1;
	struct cvmx_sli_s2m_portx_ctl_cn61xx  cn70xx;
	struct cvmx_sli_s2m_portx_ctl_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t lcl_node                     : 1;  /**< Local OCI node. When set to 1, all window access is treated as local OCI access. Normally,
                                                         if address bits [37:36] of the window address CSRs are not equal to the chip's OCI value,
                                                         the window operation is sent to the OCI for remote chip access. This field, when set,
                                                         disables this and treats all access to be for the local OCI. */
	uint64_t wind_d                       : 1;  /**< Window disable. When set to 1, disables access to the window registers from the MAC port. */
	uint64_t bar0_d                       : 1;  /**< BAR0 disable. When set to 1, disables access from the MAC to BAR0 for the following
                                                         address offsets:
                                                         0x0-0x32F
                                                         0x3CD0
                                                         greater than 0x3D70, excluding 0x3E00. */
	uint64_t ld_cmd                       : 2;  /**< When SLI issues a load command to the L2C that is to be cached, this field selects the
                                                         type of load command to use:
                                                         0 = LDD.
                                                         1 = LDI.
                                                         2 = LDE.
                                                         3 = LDY. */
	uint64_t reserved_0_0                 : 1;
#else
	uint64_t reserved_0_0                 : 1;
	uint64_t ld_cmd                       : 2;
	uint64_t bar0_d                       : 1;
	uint64_t wind_d                       : 1;
	uint64_t lcl_node                     : 1;
	uint64_t reserved_6_63                : 58;
#endif
	} cn78xx;
	struct cvmx_sli_s2m_portx_ctl_cn61xx  cnf71xx;
};
typedef union cvmx_sli_s2m_portx_ctl cvmx_sli_s2m_portx_ctl_t;

/**
 * cvmx_sli_scratch_1
 *
 * A general purpose 64 bit register for SW use.
 *
 */
union cvmx_sli_scratch_1 {
	uint64_t u64;
	struct cvmx_sli_scratch_1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< The value in this register is totaly SW dependent. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_sli_scratch_1_s           cn61xx;
	struct cvmx_sli_scratch_1_s           cn63xx;
	struct cvmx_sli_scratch_1_s           cn63xxp1;
	struct cvmx_sli_scratch_1_s           cn66xx;
	struct cvmx_sli_scratch_1_s           cn68xx;
	struct cvmx_sli_scratch_1_s           cn68xxp1;
	struct cvmx_sli_scratch_1_s           cn70xx;
	struct cvmx_sli_scratch_1_s           cn78xx;
	struct cvmx_sli_scratch_1_s           cnf71xx;
};
typedef union cvmx_sli_scratch_1 cvmx_sli_scratch_1_t;

/**
 * cvmx_sli_scratch_2
 *
 * These registers are general purpose 64-bit scratch registers for software use.
 *
 */
union cvmx_sli_scratch_2 {
	uint64_t u64;
	struct cvmx_sli_scratch_2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< The value in this register is totaly SW dependent. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_sli_scratch_2_s           cn61xx;
	struct cvmx_sli_scratch_2_s           cn63xx;
	struct cvmx_sli_scratch_2_s           cn63xxp1;
	struct cvmx_sli_scratch_2_s           cn66xx;
	struct cvmx_sli_scratch_2_s           cn68xx;
	struct cvmx_sli_scratch_2_s           cn68xxp1;
	struct cvmx_sli_scratch_2_s           cn70xx;
	struct cvmx_sli_scratch_2_s           cn78xx;
	struct cvmx_sli_scratch_2_s           cnf71xx;
};
typedef union cvmx_sli_scratch_2 cvmx_sli_scratch_2_t;

/**
 * cvmx_sli_state1
 *
 * This register contains state machines in SLI and is for debug.
 *
 */
union cvmx_sli_state1 {
	uint64_t u64;
	struct cvmx_sli_state1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t cpl1                         : 12; /**< CPL1 State */
	uint64_t cpl0                         : 12; /**< CPL0 State */
	uint64_t arb                          : 1;  /**< ARB State */
	uint64_t csr                          : 39; /**< CSR State */
#else
	uint64_t csr                          : 39;
	uint64_t arb                          : 1;
	uint64_t cpl0                         : 12;
	uint64_t cpl1                         : 12;
#endif
	} s;
	struct cvmx_sli_state1_s              cn61xx;
	struct cvmx_sli_state1_s              cn63xx;
	struct cvmx_sli_state1_s              cn63xxp1;
	struct cvmx_sli_state1_s              cn66xx;
	struct cvmx_sli_state1_s              cn68xx;
	struct cvmx_sli_state1_s              cn68xxp1;
	struct cvmx_sli_state1_s              cn70xx;
	struct cvmx_sli_state1_s              cn78xx;
	struct cvmx_sli_state1_s              cnf71xx;
};
typedef union cvmx_sli_state1 cvmx_sli_state1_t;

/**
 * cvmx_sli_state2
 *
 * This register contains state machines in SLI and is for debug.
 *
 */
union cvmx_sli_state2 {
	uint64_t u64;
	struct cvmx_sli_state2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_sli_state2_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_56_63               : 8;
	uint64_t nnp1                         : 8;  /**< NNP1 State */
	uint64_t reserved_47_47               : 1;
	uint64_t rac                          : 1;  /**< RAC State */
	uint64_t csm1                         : 15; /**< CSM1 State */
	uint64_t csm0                         : 15; /**< CSM0 State */
	uint64_t nnp0                         : 8;  /**< NNP0 State */
	uint64_t nnd                          : 8;  /**< NND State */
#else
	uint64_t nnd                          : 8;
	uint64_t nnp0                         : 8;
	uint64_t csm0                         : 15;
	uint64_t csm1                         : 15;
	uint64_t rac                          : 1;
	uint64_t reserved_47_47               : 1;
	uint64_t nnp1                         : 8;
	uint64_t reserved_56_63               : 8;
#endif
	} cn61xx;
	struct cvmx_sli_state2_cn61xx         cn63xx;
	struct cvmx_sli_state2_cn61xx         cn63xxp1;
	struct cvmx_sli_state2_cn61xx         cn66xx;
	struct cvmx_sli_state2_cn61xx         cn68xx;
	struct cvmx_sli_state2_cn61xx         cn68xxp1;
	struct cvmx_sli_state2_cn61xx         cn70xx;
	struct cvmx_sli_state2_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_57_63               : 7;
	uint64_t nnp1                         : 8;  /**< NNP1 state. */
	uint64_t reserved_48_48               : 1;
	uint64_t rac                          : 1;  /**< RAC state. */
	uint64_t csm1                         : 15; /**< CSM1 state. */
	uint64_t csm0                         : 15; /**< CSM0 state. */
	uint64_t nnp0                         : 8;  /**< NNP0 state. */
	uint64_t nnd                          : 9;  /**< NND state. */
#else
	uint64_t nnd                          : 9;
	uint64_t nnp0                         : 8;
	uint64_t csm0                         : 15;
	uint64_t csm1                         : 15;
	uint64_t rac                          : 1;
	uint64_t reserved_48_48               : 1;
	uint64_t nnp1                         : 8;
	uint64_t reserved_57_63               : 7;
#endif
	} cn78xx;
	struct cvmx_sli_state2_cn61xx         cnf71xx;
};
typedef union cvmx_sli_state2 cvmx_sli_state2_t;

/**
 * cvmx_sli_state3
 *
 * This register contains state machines in SLI and is for debug.
 *
 */
union cvmx_sli_state3 {
	uint64_t u64;
	struct cvmx_sli_state3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_sli_state3_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_56_63               : 8;
	uint64_t psm1                         : 15; /**< PSM1 State */
	uint64_t psm0                         : 15; /**< PSM0 State */
	uint64_t nsm1                         : 13; /**< NSM1 State */
	uint64_t nsm0                         : 13; /**< NSM0 State */
#else
	uint64_t nsm0                         : 13;
	uint64_t nsm1                         : 13;
	uint64_t psm0                         : 15;
	uint64_t psm1                         : 15;
	uint64_t reserved_56_63               : 8;
#endif
	} cn61xx;
	struct cvmx_sli_state3_cn61xx         cn63xx;
	struct cvmx_sli_state3_cn61xx         cn63xxp1;
	struct cvmx_sli_state3_cn61xx         cn66xx;
	struct cvmx_sli_state3_cn61xx         cn68xx;
	struct cvmx_sli_state3_cn61xx         cn68xxp1;
	struct cvmx_sli_state3_cn61xx         cn70xx;
	struct cvmx_sli_state3_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t psm1                         : 15; /**< PSM1 state. */
	uint64_t psm0                         : 15; /**< PSM0 state. */
	uint64_t nsm1                         : 15; /**< NSM1 state. */
	uint64_t nsm0                         : 15; /**< NSM0 State */
#else
	uint64_t nsm0                         : 15;
	uint64_t nsm1                         : 15;
	uint64_t psm0                         : 15;
	uint64_t psm1                         : 15;
	uint64_t reserved_60_63               : 4;
#endif
	} cn78xx;
	struct cvmx_sli_state3_cn61xx         cnf71xx;
};
typedef union cvmx_sli_state3 cvmx_sli_state3_t;

/**
 * cvmx_sli_tx_pipe
 *
 * SLI_TX_PIPE = SLI Packet TX Pipe
 *
 * Contains the starting pipe number and number of pipes used by the SLI packet Output.
 * If a packet is recevied from PKO with an out of range PIPE number, the following occurs:
 * - SLI_INT_SUM[PIPE_ERR] is set.
 * - the out of range pipe value is used for returning credits to the PKO.
 * - the PCIe packet engine will treat the PIPE value to be equal to [BASE].
 */
union cvmx_sli_tx_pipe {
	uint64_t u64;
	struct cvmx_sli_tx_pipe_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t nump                         : 8;  /**< Number of pipes the the SLI/DPI supports.
                                                         When this value is 4 or less there is a performance
                                                         advantage for output packets.
                                                         The SLI/DPI can support up to 32 pipes assigned to
                                                         packet-rings 0 - 31. */
	uint64_t reserved_7_15                : 9;
	uint64_t base                         : 7;  /**< When NUMP is non-zero, indicates the base pipe
                                                         number the SLI/DPI will accept.
                                                         The SLI/DPI will accept pko packets from pipes in
                                                         the range of:
                                                           BASE .. (BASE+(NUMP-1))
                                                         BASE and NUMP must be constrained such that
                                                           1) BASE+(NUMP-1) < 127
                                                           2) Each used PKO pipe must map to exactly
                                                              one ring. Where BASE == ring 0, BASE+1 == to
                                                              ring 1, etc
                                                           3) The pipe ranges must be consistent with
                                                              the PKO configuration. */
#else
	uint64_t base                         : 7;
	uint64_t reserved_7_15                : 9;
	uint64_t nump                         : 8;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_sli_tx_pipe_s             cn68xx;
	struct cvmx_sli_tx_pipe_s             cn68xxp1;
};
typedef union cvmx_sli_tx_pipe cvmx_sli_tx_pipe_t;

/**
 * cvmx_sli_win_rd_addr
 *
 * This register contains the address to be read when the SLI_WIN_RD_DATA register is read. This
 * register should NOT be used to read SLI_* registers.
 */
union cvmx_sli_win_rd_addr {
	uint64_t u64;
	struct cvmx_sli_win_rd_addr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_51_63               : 13;
	uint64_t ld_cmd                       : 2;  /**< The load command sent wit hthe read.
                                                         0x3 == Load 8-bytes, 0x2 == Load 4-bytes,
                                                         0x1 == Load 2-bytes, 0x0 == Load 1-bytes, */
	uint64_t iobit                        : 1;  /**< A 1 or 0 can be written here but will not be used
                                                         in address generation. */
	uint64_t rd_addr                      : 48; /**< The address to be read from.
                                                         [47:40] = NCB_ID
                                                         [39:0]  = Address
                                                         When [47:43] == SLI & [42:40] == 0 bits [39:0] are:
                                                              [39:32] == x, Not Used
                                                              [31:24] == RSL_ID
                                                              [23:0]  == RSL Register Offset */
#else
	uint64_t rd_addr                      : 48;
	uint64_t iobit                        : 1;
	uint64_t ld_cmd                       : 2;
	uint64_t reserved_51_63               : 13;
#endif
	} s;
	struct cvmx_sli_win_rd_addr_s         cn61xx;
	struct cvmx_sli_win_rd_addr_s         cn63xx;
	struct cvmx_sli_win_rd_addr_s         cn63xxp1;
	struct cvmx_sli_win_rd_addr_s         cn66xx;
	struct cvmx_sli_win_rd_addr_s         cn68xx;
	struct cvmx_sli_win_rd_addr_s         cn68xxp1;
	struct cvmx_sli_win_rd_addr_s         cn70xx;
	struct cvmx_sli_win_rd_addr_s         cn78xx;
	struct cvmx_sli_win_rd_addr_s         cnf71xx;
};
typedef union cvmx_sli_win_rd_addr cvmx_sli_win_rd_addr_t;

/**
 * cvmx_sli_win_rd_data
 *
 * This register contains the address to be read when the SLI_WIN_RD_DATA register is read.
 *
 */
union cvmx_sli_win_rd_data {
	uint64_t u64;
	struct cvmx_sli_win_rd_data_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rd_data                      : 64; /**< The read data. */
#else
	uint64_t rd_data                      : 64;
#endif
	} s;
	struct cvmx_sli_win_rd_data_s         cn61xx;
	struct cvmx_sli_win_rd_data_s         cn63xx;
	struct cvmx_sli_win_rd_data_s         cn63xxp1;
	struct cvmx_sli_win_rd_data_s         cn66xx;
	struct cvmx_sli_win_rd_data_s         cn68xx;
	struct cvmx_sli_win_rd_data_s         cn68xxp1;
	struct cvmx_sli_win_rd_data_s         cn70xx;
	struct cvmx_sli_win_rd_data_s         cn78xx;
	struct cvmx_sli_win_rd_data_s         cnf71xx;
};
typedef union cvmx_sli_win_rd_data cvmx_sli_win_rd_data_t;

/**
 * cvmx_sli_win_wr_addr
 *
 * Add Lock Register (Set on Read, Clear on write), SW uses to control access to BAR0 space.
 * Total Address is 16Kb; 0x0000 - 0x3fff, 0x000 - 0x7fe(Reg, every other 8B)
 * General  5kb; 0x0000 - 0x13ff, 0x000 - 0x27e(Reg-General)
 * PktMem  10Kb; 0x1400 - 0x3bff, 0x280 - 0x77e(Reg-General-Packet)
 * Rsvd     1Kb; 0x3c00 - 0x3fff, 0x780 - 0x7fe(Reg-NCB Only Mode)
 * SLI_WIN_WR_ADDR = SLI Window Write Address Register
 * Contains the address to be writen to when a write operation is started by writing the
 * SLI_WIN_WR_DATA register (see below).
 * This register should NOT be used to write SLI_* registers.
 */
union cvmx_sli_win_wr_addr {
	uint64_t u64;
	struct cvmx_sli_win_wr_addr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_49_63               : 15;
	uint64_t iobit                        : 1;  /**< A 1 or 0 can be written here but this will always
                                                         read as '0'. */
	uint64_t wr_addr                      : 45; /**< The address that will be written to when the
                                                         SLI_WIN_WR_DATA register is written.
                                                         [47:40] = NCB_ID
                                                         [39:3]  = Address
                                                         When [47:43] == SLI & [42:40] == 0 bits [39:0] are:
                                                              [39:32] == x, Not Used
                                                              [31:24] == RSL_ID
                                                              [23:3]  == RSL Register Offset */
	uint64_t reserved_0_2                 : 3;
#else
	uint64_t reserved_0_2                 : 3;
	uint64_t wr_addr                      : 45;
	uint64_t iobit                        : 1;
	uint64_t reserved_49_63               : 15;
#endif
	} s;
	struct cvmx_sli_win_wr_addr_s         cn61xx;
	struct cvmx_sli_win_wr_addr_s         cn63xx;
	struct cvmx_sli_win_wr_addr_s         cn63xxp1;
	struct cvmx_sli_win_wr_addr_s         cn66xx;
	struct cvmx_sli_win_wr_addr_s         cn68xx;
	struct cvmx_sli_win_wr_addr_s         cn68xxp1;
	struct cvmx_sli_win_wr_addr_s         cn70xx;
	struct cvmx_sli_win_wr_addr_s         cn78xx;
	struct cvmx_sli_win_wr_addr_s         cnf71xx;
};
typedef union cvmx_sli_win_wr_addr cvmx_sli_win_wr_addr_t;

/**
 * cvmx_sli_win_wr_data
 *
 * This register contains the data to write to the address located in the SLI_WIN_WR_ADDR
 * register. Writing the least-significant byte of this register causes a write operation to take
 * place.
 */
union cvmx_sli_win_wr_data {
	uint64_t u64;
	struct cvmx_sli_win_wr_data_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t wr_data                      : 64; /**< The data to be written. Whenever the LSB of this
                                                         register is written, the Window Write will take
                                                         place. */
#else
	uint64_t wr_data                      : 64;
#endif
	} s;
	struct cvmx_sli_win_wr_data_s         cn61xx;
	struct cvmx_sli_win_wr_data_s         cn63xx;
	struct cvmx_sli_win_wr_data_s         cn63xxp1;
	struct cvmx_sli_win_wr_data_s         cn66xx;
	struct cvmx_sli_win_wr_data_s         cn68xx;
	struct cvmx_sli_win_wr_data_s         cn68xxp1;
	struct cvmx_sli_win_wr_data_s         cn70xx;
	struct cvmx_sli_win_wr_data_s         cn78xx;
	struct cvmx_sli_win_wr_data_s         cnf71xx;
};
typedef union cvmx_sli_win_wr_data cvmx_sli_win_wr_data_t;

/**
 * cvmx_sli_win_wr_mask
 *
 * This register contains the mask for the data in the SLI_WIN_WR_DATA Register.
 *
 */
union cvmx_sli_win_wr_mask {
	uint64_t u64;
	struct cvmx_sli_win_wr_mask_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t wr_mask                      : 8;  /**< The data to be written. When a bit is '1'
                                                         the corresponding byte will be written. The values
                                                         of this field must be contiguos and for 1, 2, 4, or
                                                         8 byte operations and aligned to operation size.
                                                         A Value of 0 will produce unpredictable results */
#else
	uint64_t wr_mask                      : 8;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_sli_win_wr_mask_s         cn61xx;
	struct cvmx_sli_win_wr_mask_s         cn63xx;
	struct cvmx_sli_win_wr_mask_s         cn63xxp1;
	struct cvmx_sli_win_wr_mask_s         cn66xx;
	struct cvmx_sli_win_wr_mask_s         cn68xx;
	struct cvmx_sli_win_wr_mask_s         cn68xxp1;
	struct cvmx_sli_win_wr_mask_s         cn70xx;
	struct cvmx_sli_win_wr_mask_s         cn78xx;
	struct cvmx_sli_win_wr_mask_s         cnf71xx;
};
typedef union cvmx_sli_win_wr_mask cvmx_sli_win_wr_mask_t;

/**
 * cvmx_sli_window_ctl
 *
 * Access to register space on the IOI (caused by window read/write operations) waits for a
 * period of time specified by this register before timing out.
 */
union cvmx_sli_window_ctl {
	uint64_t u64;
	struct cvmx_sli_window_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ocx_time                     : 32; /**< When a command acknowledge or a request to fetch read-data is expected from the OCI, The
                                                         SLI will
                                                         wait this many sclks before determining the OCI is not going to respond and timeout the
                                                         request. */
	uint64_t time                         : 32; /**< Time to wait in core clocks for a
                                                         BAR0 access to completeon the NCB
                                                         before timing out. A value of 0 will cause no
                                                         timeouts. A minimum value of 0x200000 should be
                                                         used when this register is not set to 0x0. */
#else
	uint64_t time                         : 32;
	uint64_t ocx_time                     : 32;
#endif
	} s;
	struct cvmx_sli_window_ctl_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t time                         : 32; /**< Time to wait in core clocks for a
                                                         BAR0 access to completeon the NCB
                                                         before timing out. A value of 0 will cause no
                                                         timeouts. A minimum value of 0x200000 should be
                                                         used when this register is not set to 0x0. */
#else
	uint64_t time                         : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} cn61xx;
	struct cvmx_sli_window_ctl_cn61xx     cn63xx;
	struct cvmx_sli_window_ctl_cn61xx     cn63xxp1;
	struct cvmx_sli_window_ctl_cn61xx     cn66xx;
	struct cvmx_sli_window_ctl_cn61xx     cn68xx;
	struct cvmx_sli_window_ctl_cn61xx     cn68xxp1;
	struct cvmx_sli_window_ctl_cn61xx     cn70xx;
	struct cvmx_sli_window_ctl_s          cn78xx;
	struct cvmx_sli_window_ctl_cn61xx     cnf71xx;
};
typedef union cvmx_sli_window_ctl cvmx_sli_window_ctl_t;

#endif
