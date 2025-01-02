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
 * cvmx-ila-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon ila.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_ILA_DEFS_H__
#define __CVMX_ILA_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ILA_BIST_SUM CVMX_ILA_BIST_SUM_FUNC()
static inline uint64_t CVMX_ILA_BIST_SUM_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ILA_BIST_SUM not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180017000038ull);
}
#else
#define CVMX_ILA_BIST_SUM (CVMX_ADD_IO_SEG(0x0001180017000038ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ILA_GBL_CFG CVMX_ILA_GBL_CFG_FUNC()
static inline uint64_t CVMX_ILA_GBL_CFG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ILA_GBL_CFG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180017000000ull);
}
#else
#define CVMX_ILA_GBL_CFG (CVMX_ADD_IO_SEG(0x0001180017000000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_LNEX_TRN_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7)))))
		cvmx_warn("CVMX_ILA_LNEX_TRN_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800170380F0ull) + ((offset) & 7) * 1024;
}
#else
#define CVMX_ILA_LNEX_TRN_CTL(offset) (CVMX_ADD_IO_SEG(0x00011800170380F0ull) + ((offset) & 7) * 1024)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_LNEX_TRN_LD(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7)))))
		cvmx_warn("CVMX_ILA_LNEX_TRN_LD(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800170380E0ull) + ((offset) & 7) * 1024;
}
#else
#define CVMX_ILA_LNEX_TRN_LD(offset) (CVMX_ADD_IO_SEG(0x00011800170380E0ull) + ((offset) & 7) * 1024)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_LNEX_TRN_LP(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7)))))
		cvmx_warn("CVMX_ILA_LNEX_TRN_LP(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800170380E8ull) + ((offset) & 7) * 1024;
}
#else
#define CVMX_ILA_LNEX_TRN_LP(offset) (CVMX_ADD_IO_SEG(0x00011800170380E8ull) + ((offset) & 7) * 1024)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ILA_LNE_DBG CVMX_ILA_LNE_DBG_FUNC()
static inline uint64_t CVMX_ILA_LNE_DBG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ILA_LNE_DBG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180017030008ull);
}
#else
#define CVMX_ILA_LNE_DBG (CVMX_ADD_IO_SEG(0x0001180017030008ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ILA_LNE_STS_MSG CVMX_ILA_LNE_STS_MSG_FUNC()
static inline uint64_t CVMX_ILA_LNE_STS_MSG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ILA_LNE_STS_MSG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180017030000ull);
}
#else
#define CVMX_ILA_LNE_STS_MSG (CVMX_ADD_IO_SEG(0x0001180017030000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_RXX_BYTE_CNTX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 1)) && ((block_id == 0))))))
		cvmx_warn("CVMX_ILA_RXX_BYTE_CNTX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00011800170200A0ull) + (((offset) & 1) + ((block_id) & 0) * 0x0ull) * 8;
}
#else
#define CVMX_ILA_RXX_BYTE_CNTX(offset, block_id) (CVMX_ADD_IO_SEG(0x00011800170200A0ull) + (((offset) & 1) + ((block_id) & 0) * 0x0ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_RXX_CFG0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_ILA_RXX_CFG0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180017020000ull);
}
#else
#define CVMX_ILA_RXX_CFG0(offset) (CVMX_ADD_IO_SEG(0x0001180017020000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_RXX_CFG1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_ILA_RXX_CFG1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180017020008ull);
}
#else
#define CVMX_ILA_RXX_CFG1(offset) (CVMX_ADD_IO_SEG(0x0001180017020008ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_RXX_CHA_XON(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_ILA_RXX_CHA_XON(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180017020090ull);
}
#else
#define CVMX_ILA_RXX_CHA_XON(offset) (CVMX_ADD_IO_SEG(0x0001180017020090ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_RXX_INT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_ILA_RXX_INT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180017020010ull);
}
#else
#define CVMX_ILA_RXX_INT(offset) (CVMX_ADD_IO_SEG(0x0001180017020010ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_RXX_PKT_CNTX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 1)) && ((block_id == 0))))))
		cvmx_warn("CVMX_ILA_RXX_PKT_CNTX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180017020080ull) + (((offset) & 1) + ((block_id) & 0) * 0x0ull) * 8;
}
#else
#define CVMX_ILA_RXX_PKT_CNTX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180017020080ull) + (((offset) & 1) + ((block_id) & 0) * 0x0ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_RXX_STAT0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_ILA_RXX_STAT0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180017020020ull);
}
#else
#define CVMX_ILA_RXX_STAT0(offset) (CVMX_ADD_IO_SEG(0x0001180017020020ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_RXX_STAT1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_ILA_RXX_STAT1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180017020028ull);
}
#else
#define CVMX_ILA_RXX_STAT1(offset) (CVMX_ADD_IO_SEG(0x0001180017020028ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_RXX_STAT2(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_ILA_RXX_STAT2(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180017020030ull);
}
#else
#define CVMX_ILA_RXX_STAT2(offset) (CVMX_ADD_IO_SEG(0x0001180017020030ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_RXX_STAT3(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_ILA_RXX_STAT3(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180017020038ull);
}
#else
#define CVMX_ILA_RXX_STAT3(offset) (CVMX_ADD_IO_SEG(0x0001180017020038ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_RXX_STAT4(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_ILA_RXX_STAT4(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180017020040ull);
}
#else
#define CVMX_ILA_RXX_STAT4(offset) (CVMX_ADD_IO_SEG(0x0001180017020040ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_RXX_STAT5(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_ILA_RXX_STAT5(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180017020048ull);
}
#else
#define CVMX_ILA_RXX_STAT5(offset) (CVMX_ADD_IO_SEG(0x0001180017020048ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_RXX_STAT6(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_ILA_RXX_STAT6(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180017020050ull);
}
#else
#define CVMX_ILA_RXX_STAT6(offset) (CVMX_ADD_IO_SEG(0x0001180017020050ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_RXX_STAT7(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_ILA_RXX_STAT7(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180017020058ull);
}
#else
#define CVMX_ILA_RXX_STAT7(offset) (CVMX_ADD_IO_SEG(0x0001180017020058ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_RXX_STAT8(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_ILA_RXX_STAT8(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180017020060ull);
}
#else
#define CVMX_ILA_RXX_STAT8(offset) (CVMX_ADD_IO_SEG(0x0001180017020060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_RXX_STAT9(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_ILA_RXX_STAT9(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180017020068ull);
}
#else
#define CVMX_ILA_RXX_STAT9(offset) (CVMX_ADD_IO_SEG(0x0001180017020068ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_RX_LNEX_CFG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7)))))
		cvmx_warn("CVMX_ILA_RX_LNEX_CFG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180017038000ull) + ((offset) & 7) * 1024;
}
#else
#define CVMX_ILA_RX_LNEX_CFG(offset) (CVMX_ADD_IO_SEG(0x0001180017038000ull) + ((offset) & 7) * 1024)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_RX_LNEX_INT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7)))))
		cvmx_warn("CVMX_ILA_RX_LNEX_INT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180017038008ull) + ((offset) & 7) * 1024;
}
#else
#define CVMX_ILA_RX_LNEX_INT(offset) (CVMX_ADD_IO_SEG(0x0001180017038008ull) + ((offset) & 7) * 1024)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_RX_LNEX_STAT0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7)))))
		cvmx_warn("CVMX_ILA_RX_LNEX_STAT0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180017038018ull) + ((offset) & 7) * 1024;
}
#else
#define CVMX_ILA_RX_LNEX_STAT0(offset) (CVMX_ADD_IO_SEG(0x0001180017038018ull) + ((offset) & 7) * 1024)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_RX_LNEX_STAT1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7)))))
		cvmx_warn("CVMX_ILA_RX_LNEX_STAT1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180017038020ull) + ((offset) & 7) * 1024;
}
#else
#define CVMX_ILA_RX_LNEX_STAT1(offset) (CVMX_ADD_IO_SEG(0x0001180017038020ull) + ((offset) & 7) * 1024)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_RX_LNEX_STAT10(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7)))))
		cvmx_warn("CVMX_ILA_RX_LNEX_STAT10(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180017038068ull) + ((offset) & 7) * 1024;
}
#else
#define CVMX_ILA_RX_LNEX_STAT10(offset) (CVMX_ADD_IO_SEG(0x0001180017038068ull) + ((offset) & 7) * 1024)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_RX_LNEX_STAT2(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7)))))
		cvmx_warn("CVMX_ILA_RX_LNEX_STAT2(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180017038028ull) + ((offset) & 7) * 1024;
}
#else
#define CVMX_ILA_RX_LNEX_STAT2(offset) (CVMX_ADD_IO_SEG(0x0001180017038028ull) + ((offset) & 7) * 1024)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_RX_LNEX_STAT3(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7)))))
		cvmx_warn("CVMX_ILA_RX_LNEX_STAT3(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180017038030ull) + ((offset) & 7) * 1024;
}
#else
#define CVMX_ILA_RX_LNEX_STAT3(offset) (CVMX_ADD_IO_SEG(0x0001180017038030ull) + ((offset) & 7) * 1024)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_RX_LNEX_STAT4(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7)))))
		cvmx_warn("CVMX_ILA_RX_LNEX_STAT4(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180017038038ull) + ((offset) & 7) * 1024;
}
#else
#define CVMX_ILA_RX_LNEX_STAT4(offset) (CVMX_ADD_IO_SEG(0x0001180017038038ull) + ((offset) & 7) * 1024)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_RX_LNEX_STAT5(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7)))))
		cvmx_warn("CVMX_ILA_RX_LNEX_STAT5(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180017038040ull) + ((offset) & 7) * 1024;
}
#else
#define CVMX_ILA_RX_LNEX_STAT5(offset) (CVMX_ADD_IO_SEG(0x0001180017038040ull) + ((offset) & 7) * 1024)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_RX_LNEX_STAT6(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7)))))
		cvmx_warn("CVMX_ILA_RX_LNEX_STAT6(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180017038048ull) + ((offset) & 7) * 1024;
}
#else
#define CVMX_ILA_RX_LNEX_STAT6(offset) (CVMX_ADD_IO_SEG(0x0001180017038048ull) + ((offset) & 7) * 1024)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_RX_LNEX_STAT7(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7)))))
		cvmx_warn("CVMX_ILA_RX_LNEX_STAT7(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180017038050ull) + ((offset) & 7) * 1024;
}
#else
#define CVMX_ILA_RX_LNEX_STAT7(offset) (CVMX_ADD_IO_SEG(0x0001180017038050ull) + ((offset) & 7) * 1024)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_RX_LNEX_STAT8(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7)))))
		cvmx_warn("CVMX_ILA_RX_LNEX_STAT8(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180017038058ull) + ((offset) & 7) * 1024;
}
#else
#define CVMX_ILA_RX_LNEX_STAT8(offset) (CVMX_ADD_IO_SEG(0x0001180017038058ull) + ((offset) & 7) * 1024)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_RX_LNEX_STAT9(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 7)))))
		cvmx_warn("CVMX_ILA_RX_LNEX_STAT9(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180017038060ull) + ((offset) & 7) * 1024;
}
#else
#define CVMX_ILA_RX_LNEX_STAT9(offset) (CVMX_ADD_IO_SEG(0x0001180017038060ull) + ((offset) & 7) * 1024)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ILA_SER_CFG CVMX_ILA_SER_CFG_FUNC()
static inline uint64_t CVMX_ILA_SER_CFG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ILA_SER_CFG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180017000018ull);
}
#else
#define CVMX_ILA_SER_CFG (CVMX_ADD_IO_SEG(0x0001180017000018ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_TXX_BYTE_CNTX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 1)) && ((block_id == 0))))))
		cvmx_warn("CVMX_ILA_TXX_BYTE_CNTX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180017010040ull) + (((offset) & 1) + ((block_id) & 0) * 0x0ull) * 8;
}
#else
#define CVMX_ILA_TXX_BYTE_CNTX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180017010040ull) + (((offset) & 1) + ((block_id) & 0) * 0x0ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_TXX_CFG0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_ILA_TXX_CFG0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180017010000ull);
}
#else
#define CVMX_ILA_TXX_CFG0(offset) (CVMX_ADD_IO_SEG(0x0001180017010000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_TXX_CFG1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_ILA_TXX_CFG1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180017010008ull);
}
#else
#define CVMX_ILA_TXX_CFG1(offset) (CVMX_ADD_IO_SEG(0x0001180017010008ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_TXX_CHA_XON(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_ILA_TXX_CHA_XON(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180017010088ull);
}
#else
#define CVMX_ILA_TXX_CHA_XON(offset) (CVMX_ADD_IO_SEG(0x0001180017010088ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_TXX_DBG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_ILA_TXX_DBG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180017010090ull);
}
#else
#define CVMX_ILA_TXX_DBG(offset) (CVMX_ADD_IO_SEG(0x0001180017010090ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_TXX_ERR_CFG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_ILA_TXX_ERR_CFG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800170100A0ull);
}
#else
#define CVMX_ILA_TXX_ERR_CFG(offset) (CVMX_ADD_IO_SEG(0x00011800170100A0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_TXX_INT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_ILA_TXX_INT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180017010098ull);
}
#else
#define CVMX_ILA_TXX_INT(offset) (CVMX_ADD_IO_SEG(0x0001180017010098ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_TXX_PKT_CNTX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 1)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 1)) && ((block_id == 0))))))
		cvmx_warn("CVMX_ILA_TXX_PKT_CNTX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180017010020ull) + (((offset) & 1) + ((block_id) & 0) * 0x0ull) * 8;
}
#else
#define CVMX_ILA_TXX_PKT_CNTX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180017010020ull) + (((offset) & 1) + ((block_id) & 0) * 0x0ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILA_TXX_RMATCH(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_ILA_TXX_RMATCH(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180017010080ull);
}
#else
#define CVMX_ILA_TXX_RMATCH(offset) (CVMX_ADD_IO_SEG(0x0001180017010080ull))
#endif

/**
 * cvmx_ila_bist_sum
 */
union cvmx_ila_bist_sum {
	uint64_t u64;
	struct cvmx_ila_bist_sum_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t tlk0_txf0                    : 1;  /**< BIST status of tlk0.txf.tx_fif_mem. */
#else
	uint64_t tlk0_txf0                    : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_ila_bist_sum_s            cn78xx;
	struct cvmx_ila_bist_sum_s            cn78xxp1;
};
typedef union cvmx_ila_bist_sum cvmx_ila_bist_sum_t;

/**
 * cvmx_ila_gbl_cfg
 */
union cvmx_ila_gbl_cfg {
	uint64_t u64;
	struct cvmx_ila_gbl_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t reset                        : 1;  /**< Reset ILA. For diagnostic use only. */
	uint64_t cclk_dis                     : 1;  /**< Disable ILA conditional clocking.   For diagnostic use only. */
	uint64_t reserved_0_0                 : 1;
#else
	uint64_t reserved_0_0                 : 1;
	uint64_t cclk_dis                     : 1;
	uint64_t reset                        : 1;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_ila_gbl_cfg_s             cn78xx;
	struct cvmx_ila_gbl_cfg_s             cn78xxp1;
};
typedef union cvmx_ila_gbl_cfg cvmx_ila_gbl_cfg_t;

/**
 * cvmx_ila_lne#_trn_ctl
 */
union cvmx_ila_lnex_trn_ctl {
	uint64_t u64;
	struct cvmx_ila_lnex_trn_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t trn_lock                     : 1;  /**< Link training RX frame lock */
	uint64_t trn_done                     : 1;  /**< Link training done */
	uint64_t trn_ena                      : 1;  /**< Link training enable */
	uint64_t eie_det                      : 1;  /**< Reserved. */
#else
	uint64_t eie_det                      : 1;
	uint64_t trn_ena                      : 1;
	uint64_t trn_done                     : 1;
	uint64_t trn_lock                     : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_ila_lnex_trn_ctl_s        cn78xx;
	struct cvmx_ila_lnex_trn_ctl_s        cn78xxp1;
};
typedef union cvmx_ila_lnex_trn_ctl cvmx_ila_lnex_trn_ctl_t;

/**
 * cvmx_ila_lne#_trn_ld
 */
union cvmx_ila_lnex_trn_ld {
	uint64_t u64;
	struct cvmx_ila_lnex_trn_ld_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t lp_manual                    : 1;  /**< Software must write LP_MANUAL=1 when performing manually training. */
	uint64_t reserved_49_62               : 14;
	uint64_t ld_cu_val                    : 1;  /**< Local device coefficient update field valid. */
	uint64_t ld_cu_dat                    : 16; /**< Local device coefficient update field data. */
	uint64_t reserved_17_31               : 15;
	uint64_t ld_sr_val                    : 1;  /**< Local device status report field valid. */
	uint64_t ld_sr_dat                    : 16; /**< Local device status report field data. */
#else
	uint64_t ld_sr_dat                    : 16;
	uint64_t ld_sr_val                    : 1;
	uint64_t reserved_17_31               : 15;
	uint64_t ld_cu_dat                    : 16;
	uint64_t ld_cu_val                    : 1;
	uint64_t reserved_49_62               : 14;
	uint64_t lp_manual                    : 1;
#endif
	} s;
	struct cvmx_ila_lnex_trn_ld_s         cn78xx;
	struct cvmx_ila_lnex_trn_ld_s         cn78xxp1;
};
typedef union cvmx_ila_lnex_trn_ld cvmx_ila_lnex_trn_ld_t;

/**
 * cvmx_ila_lne#_trn_lp
 */
union cvmx_ila_lnex_trn_lp {
	uint64_t u64;
	struct cvmx_ila_lnex_trn_lp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_49_63               : 15;
	uint64_t lp_cu_val                    : 1;  /**< Link partner coefficient update field valid. */
	uint64_t lp_cu_dat                    : 16; /**< Link partner coefficient update field data. */
	uint64_t reserved_17_31               : 15;
	uint64_t lp_sr_val                    : 1;  /**< Link partner status report field valid. */
	uint64_t lp_sr_dat                    : 16; /**< Link partner status report field data. */
#else
	uint64_t lp_sr_dat                    : 16;
	uint64_t lp_sr_val                    : 1;
	uint64_t reserved_17_31               : 15;
	uint64_t lp_cu_dat                    : 16;
	uint64_t lp_cu_val                    : 1;
	uint64_t reserved_49_63               : 15;
#endif
	} s;
	struct cvmx_ila_lnex_trn_lp_s         cn78xx;
	struct cvmx_ila_lnex_trn_lp_s         cn78xxp1;
};
typedef union cvmx_ila_lnex_trn_lp cvmx_ila_lnex_trn_lp_t;

/**
 * cvmx_ila_lne_dbg
 */
union cvmx_ila_lne_dbg {
	uint64_t u64;
	struct cvmx_ila_lne_dbg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t tx_bad_crc32                 : 1;  /**< Send one diagnostic word with bad CRC32 to the selected lane. Note that it injects just once. */
	uint64_t tx_bad_6467_cnt              : 5;  /**< Specifies the number of bad 64/67 bit codewords on the selected lane. */
	uint64_t tx_bad_sync_cnt              : 3;  /**< Specifies the number of bad sync words on the selected lane. */
	uint64_t tx_bad_scram_cnt             : 3;  /**< Specifies the number of bad scrambler state on the selected lane. */
	uint64_t reserved_40_47               : 8;
	uint64_t tx_bad_lane_sel              : 8;  /**< Select the lane to apply the error-injection counts. */
	uint64_t reserved_24_31               : 8;
	uint64_t tx_dis_dispr                 : 8;  /**< Per-lane disparity disable. */
	uint64_t reserved_8_15                : 8;
	uint64_t tx_dis_scram                 : 8;  /**< Per-lane scrambler disable. */
#else
	uint64_t tx_dis_scram                 : 8;
	uint64_t reserved_8_15                : 8;
	uint64_t tx_dis_dispr                 : 8;
	uint64_t reserved_24_31               : 8;
	uint64_t tx_bad_lane_sel              : 8;
	uint64_t reserved_40_47               : 8;
	uint64_t tx_bad_scram_cnt             : 3;
	uint64_t tx_bad_sync_cnt              : 3;
	uint64_t tx_bad_6467_cnt              : 5;
	uint64_t tx_bad_crc32                 : 1;
	uint64_t reserved_60_63               : 4;
#endif
	} s;
	struct cvmx_ila_lne_dbg_s             cn78xx;
	struct cvmx_ila_lne_dbg_s             cn78xxp1;
};
typedef union cvmx_ila_lne_dbg cvmx_ila_lne_dbg_t;

/**
 * cvmx_ila_lne_sts_msg
 */
union cvmx_ila_lne_sts_msg {
	uint64_t u64;
	struct cvmx_ila_lne_sts_msg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_56_63               : 8;
	uint64_t rx_lnk_stat                  : 8;  /**< Link status received in the diagnostic word (per-lane); 1 means healthy (according to the
                                                         Interlaken specification). */
	uint64_t reserved_40_47               : 8;
	uint64_t rx_lne_stat                  : 8;  /**< Lane status received in the diagnostic word (per-lane); 1 means healthy (according to the
                                                         Interlaken specification). */
	uint64_t reserved_24_31               : 8;
	uint64_t tx_lnk_stat                  : 8;  /**< Link status transmitted in the diagnostic word (per-lane); 1 means healthy (according to
                                                         the Interlaken specification). */
	uint64_t reserved_8_15                : 8;
	uint64_t tx_lne_stat                  : 8;  /**< Lane status transmitted in the diagnostic word (per-lane); 1 means healthy (according to
                                                         the Interlaken specification). */
#else
	uint64_t tx_lne_stat                  : 8;
	uint64_t reserved_8_15                : 8;
	uint64_t tx_lnk_stat                  : 8;
	uint64_t reserved_24_31               : 8;
	uint64_t rx_lne_stat                  : 8;
	uint64_t reserved_40_47               : 8;
	uint64_t rx_lnk_stat                  : 8;
	uint64_t reserved_56_63               : 8;
#endif
	} s;
	struct cvmx_ila_lne_sts_msg_s         cn78xx;
	struct cvmx_ila_lne_sts_msg_s         cn78xxp1;
};
typedef union cvmx_ila_lne_sts_msg cvmx_ila_lne_sts_msg_t;

/**
 * cvmx_ila_rx#_byte_cnt#
 */
union cvmx_ila_rxx_byte_cntx {
	uint64_t u64;
	struct cvmx_ila_rxx_byte_cntx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_40_63               : 24;
	uint64_t rx_bytes                     : 40; /**< Indicates the number of bytes received per channel. Wraps on overflow. On overflow, sets
                                                         ILA_RX()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t rx_bytes                     : 40;
	uint64_t reserved_40_63               : 24;
#endif
	} s;
	struct cvmx_ila_rxx_byte_cntx_s       cn78xx;
	struct cvmx_ila_rxx_byte_cntx_s       cn78xxp1;
};
typedef union cvmx_ila_rxx_byte_cntx cvmx_ila_rxx_byte_cntx_t;

/**
 * cvmx_ila_rx#_cfg0
 */
union cvmx_ila_rxx_cfg0 {
	uint64_t u64;
	struct cvmx_ila_rxx_cfg0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_63_63               : 1;
	uint64_t ext_lpbk                     : 1;  /**< Enable RX-TX data external loopback. Note that with differing transmit and receive clocks,
                                                         skip word are inserted/deleted. */
	uint64_t reserved_60_61               : 2;
	uint64_t lnk_stats_wrap               : 1;  /**< Enable that upon overflow, statistics should wrap instead of saturating. */
	uint64_t reserved_56_58               : 3;
	uint64_t lnk_stats_rdclr              : 1;  /**< Enable that a CSR read operation to ILA_RX()_STAT0..ILA_RX()_STAT9 clears the counter
                                                         after returning its current value. */
	uint64_t lnk_stats_ena                : 1;  /**< Enable link-statistics counters. */
	uint64_t reserved_52_53               : 2;
	uint64_t mfrm_len                     : 13; /**< The quantity of data sent on each lane including one sync word, scrambler state,
                                                         diagnostic word, zero or more skip words, and the data payload. Must be larger than
                                                         ILA_TX()_CFG1[SKIP_CNT] + 32.
                                                         Supported range:
                                                         _ ILA_TX()_CFG1[SKIP_CNT] + 32 < [MFRM_LEN] <= 4096 */
	uint64_t brst_shrt                    : 7;  /**< Minimum interval between burst control words, as a multiple of eight bytes. Supported
                                                         range from 8 to 512 bytes (i.e. 0 < BRST_SHRT <= 64).
                                                         This field affects the ILA_RX()_STAT4[BRST_SHRT_ERR_CNT] counter. It does not affect
                                                         correct operation of the link. */
	uint64_t lane_rev                     : 1;  /**< Lane reversal. When enabled, lane destriping is performed from most-significant lane
                                                         enabled to least-significant lane enabled. [LANE_ENA] must be 0 before changing
                                                         [LANE_REV]. */
	uint64_t brst_max                     : 5;  /**< Maximum size of a data burst, as a multiple of 64-byte blocks. Supported range is from 64
                                                         to 1024 bytes
                                                         (i.e. 0 < BRST_MAX <= 16).
                                                         This field affects the ILA_RX()_STAT2[BRST_NOT_FULL_CNT] and
                                                         ILA_RX()_STAT3[BRST_MAX_ERR_CNT] counters. It does not affect correct operation of the
                                                         link. */
	uint64_t reserved_8_25                : 18;
	uint64_t lane_ena                     : 8;  /**< Lane enable mask. The link is enabled if any lane is enabled. The same lane should not be
                                                         enabled in multiple ILA_RXn_CFG0. Each bit of [LANE_ENA] maps to an RX lane (RLE) and a
                                                         QLM lane. Note that [LANE_REV] has no effect on this mapping.
                                                         _ [LANE_ENA<0>]  = RLE0  = QLM2 lane 0.
                                                         _ [LANE_ENA<1>]  = RLE1  = QLM2 lane 1.
                                                         _ [LANE_ENA<2>]  = RLE2  = QLM2 lane 2.
                                                         _ [LANE_ENA<3>]  = RLE3  = QLM2 lane 3.
                                                         _ [LANE_ENA<4>]  = RLE4  = QLM3 lane 0.
                                                         _ [LANE_ENA<5>]  = RLE5  = QLM3 lane 1.
                                                         _ [LANE_ENA<6>]  = RLE6  = QLM3 lane 2.
                                                         _ [LANE_ENA<7>]  = RLE7  = QLM3 lane 3. */
#else
	uint64_t lane_ena                     : 8;
	uint64_t reserved_8_25                : 18;
	uint64_t brst_max                     : 5;
	uint64_t lane_rev                     : 1;
	uint64_t brst_shrt                    : 7;
	uint64_t mfrm_len                     : 13;
	uint64_t reserved_52_53               : 2;
	uint64_t lnk_stats_ena                : 1;
	uint64_t lnk_stats_rdclr              : 1;
	uint64_t reserved_56_58               : 3;
	uint64_t lnk_stats_wrap               : 1;
	uint64_t reserved_60_61               : 2;
	uint64_t ext_lpbk                     : 1;
	uint64_t reserved_63_63               : 1;
#endif
	} s;
	struct cvmx_ila_rxx_cfg0_s            cn78xx;
	struct cvmx_ila_rxx_cfg0_s            cn78xxp1;
};
typedef union cvmx_ila_rxx_cfg0 cvmx_ila_rxx_cfg0_t;

/**
 * cvmx_ila_rx#_cfg1
 */
union cvmx_ila_rxx_cfg1 {
	uint64_t u64;
	struct cvmx_ila_rxx_cfg1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_22_63               : 42;
	uint64_t pkt_flush                    : 1;  /**< Packet receive flush. Setting this bit causes all open packets to be error-out, just as
                                                         though the link went down. */
	uint64_t pkt_ena                      : 1;  /**< Packet receive enable. When set to 0, any received SOP causes the entire packet to be dropped. */
	uint64_t reserved_19_19               : 1;
	uint64_t tx_link_fc                   : 1;  /**< Link flow-control status transmitted by the TX-link XON (=1) when [RX_FIFO_CNT] <=
                                                         [RX_FIFO_HWM] and lane alignment is done. */
	uint64_t rx_link_fc                   : 1;  /**< Link flow-control status received in burst/idle control words. XOFF (=0) causes TX-link to
                                                         stop transmitting on all channels. */
	uint64_t rx_align_ena                 : 1;  /**< Enable the lane alignment. This should only be done after all enabled lanes have achieved
                                                         word boundary lock and scrambler synchronization. Note that hardware clears this when any
                                                         participating lane loses either word boundary lock or scrambler synchronization. */
	uint64_t reserved_8_15                : 8;
	uint64_t rx_bdry_lock_ena             : 8;  /**< Enable word-boundary lock. While disabled, received data is tossed. Once enabled, received
                                                         data is searched for legal two-bit patterns. Automatically cleared for disabled lanes. */
#else
	uint64_t rx_bdry_lock_ena             : 8;
	uint64_t reserved_8_15                : 8;
	uint64_t rx_align_ena                 : 1;
	uint64_t rx_link_fc                   : 1;
	uint64_t tx_link_fc                   : 1;
	uint64_t reserved_19_19               : 1;
	uint64_t pkt_ena                      : 1;
	uint64_t pkt_flush                    : 1;
	uint64_t reserved_22_63               : 42;
#endif
	} s;
	struct cvmx_ila_rxx_cfg1_s            cn78xx;
	struct cvmx_ila_rxx_cfg1_s            cn78xxp1;
};
typedef union cvmx_ila_rxx_cfg1 cvmx_ila_rxx_cfg1_t;

/**
 * cvmx_ila_rx#_cha_xon
 */
union cvmx_ila_rxx_cha_xon {
	uint64_t u64;
	struct cvmx_ila_rxx_cha_xon_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t ch1_xon                      : 1;  /**< Flow-control status for channel 1, where a value of 0 indicates the presence of
                                                         backpressure (i.e. XOFF) and a value of 1 indicates the absence of backpressure (i.e. XON) */
	uint64_t ch0_xon                      : 1;  /**< Flow-control status for channel 0, where a value of 0 indicates the presence of
                                                         backpressure (i.e. XOFF) and a value of 1 indicates the absence of backpressure (i.e. XON) */
#else
	uint64_t ch0_xon                      : 1;
	uint64_t ch1_xon                      : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_ila_rxx_cha_xon_s         cn78xx;
	struct cvmx_ila_rxx_cha_xon_s         cn78xxp1;
};
typedef union cvmx_ila_rxx_cha_xon cvmx_ila_rxx_cha_xon_t;

/**
 * cvmx_ila_rx#_int
 */
union cvmx_ila_rxx_int {
	uint64_t u64;
	struct cvmx_ila_rxx_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t lane_bad_word                : 1;  /**< A lane encountered either a bad 64/67 bit code word or an unknown control-word type. */
	uint64_t stat_cnt_ovfl                : 1;  /**< Statistics counter overflow. */
	uint64_t lane_align_done              : 1;  /**< Lane alignment successful. */
	uint64_t word_sync_done               : 1;  /**< All enabled lanes have achieved word-boundary lock and scrambler synchronization. Lane
                                                         alignment may now be enabled. */
	uint64_t crc24_err                    : 1;  /**< Burst CRC24 error. All open packets receive an error. */
	uint64_t lane_align_fail              : 1;  /**< Lane alignment fails after four tries. Hardware repeats lane alignment until is succeeds
                                                         or until ILA_RX()_CFG1[RX_ALIGN_ENA] = 0. */
#else
	uint64_t lane_align_fail              : 1;
	uint64_t crc24_err                    : 1;
	uint64_t word_sync_done               : 1;
	uint64_t lane_align_done              : 1;
	uint64_t stat_cnt_ovfl                : 1;
	uint64_t lane_bad_word                : 1;
	uint64_t reserved_6_63                : 58;
#endif
	} s;
	struct cvmx_ila_rxx_int_s             cn78xx;
	struct cvmx_ila_rxx_int_s             cn78xxp1;
};
typedef union cvmx_ila_rxx_int cvmx_ila_rxx_int_t;

/**
 * cvmx_ila_rx#_pkt_cnt#
 */
union cvmx_ila_rxx_pkt_cntx {
	uint64_t u64;
	struct cvmx_ila_rxx_pkt_cntx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_34_63               : 30;
	uint64_t rx_pkt                       : 34; /**< Indicates the number of packets received per channel. Wraps on overflow. On overflow, sets
                                                         ILA_RX()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t rx_pkt                       : 34;
	uint64_t reserved_34_63               : 30;
#endif
	} s;
	struct cvmx_ila_rxx_pkt_cntx_s        cn78xx;
	struct cvmx_ila_rxx_pkt_cntx_s        cn78xxp1;
};
typedef union cvmx_ila_rxx_pkt_cntx cvmx_ila_rxx_pkt_cntx_t;

/**
 * cvmx_ila_rx#_stat0
 */
union cvmx_ila_rxx_stat0 {
	uint64_t u64;
	struct cvmx_ila_rxx_stat0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_33_63               : 31;
	uint64_t crc24_match_cnt              : 33; /**< Indicates the number of CRC24 matches received. Wraps on overflow if
                                                         ILA_RX()_CFG0[LNK_STATS_WRAP]=1. Otherwise, saturates. On overflow/saturate, sets
                                                         ILA_RX()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t crc24_match_cnt              : 33;
	uint64_t reserved_33_63               : 31;
#endif
	} s;
	struct cvmx_ila_rxx_stat0_s           cn78xx;
	struct cvmx_ila_rxx_stat0_s           cn78xxp1;
};
typedef union cvmx_ila_rxx_stat0 cvmx_ila_rxx_stat0_t;

/**
 * cvmx_ila_rx#_stat1
 */
union cvmx_ila_rxx_stat1 {
	uint64_t u64;
	struct cvmx_ila_rxx_stat1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t crc24_err_cnt                : 18; /**< Indicates the number of bursts with a detected CRC error. Wraps on overflow if
                                                         ILA_RX()_CFG0[LNK_STATS_WRAP]=1. Otherwise, saturates. On overflow/saturate, sets
                                                         ILA_RX()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t crc24_err_cnt                : 18;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_ila_rxx_stat1_s           cn78xx;
	struct cvmx_ila_rxx_stat1_s           cn78xxp1;
};
typedef union cvmx_ila_rxx_stat1 cvmx_ila_rxx_stat1_t;

/**
 * cvmx_ila_rx#_stat2
 */
union cvmx_ila_rxx_stat2 {
	uint64_t u64;
	struct cvmx_ila_rxx_stat2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t brst_not_full_cnt            : 16; /**< Indicates the number of bursts received that terminated without an EOP and contained fewer
                                                         than BurstMax words. Wraps on overflow if ILA_RX()_CFG0[LNK_STATS_WRAP]=1. Otherwise,
                                                         saturates. On overflow/saturate, sets ILA_RX()_INT[STAT_CNT_OVFL]. */
	uint64_t reserved_28_31               : 4;
	uint64_t brst_cnt                     : 28; /**< Indicates the number of bursts correctly received (i.e. good CRC24, not in violation of
                                                         BurstMax or BurstShort). Wraps on overflow if ILA_RX()_CFG0[LNK_STATS_WRAP]=1. Otherwise,
                                                         saturates. On overflow/saturate, sets ILA_RX()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t brst_cnt                     : 28;
	uint64_t reserved_28_31               : 4;
	uint64_t brst_not_full_cnt            : 16;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_ila_rxx_stat2_s           cn78xx;
	struct cvmx_ila_rxx_stat2_s           cn78xxp1;
};
typedef union cvmx_ila_rxx_stat2 cvmx_ila_rxx_stat2_t;

/**
 * cvmx_ila_rx#_stat3
 */
union cvmx_ila_rxx_stat3 {
	uint64_t u64;
	struct cvmx_ila_rxx_stat3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t brst_max_err_cnt             : 16; /**< Indicates the number of bursts received longer than the BurstMax parameter. Wraps on
                                                         overflow if ILA_RX()_CFG0[LNK_STATS_WRAP]=1. Otherwise, saturates. On overflow/saturate,
                                                         sets ILA_RX()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t brst_max_err_cnt             : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_ila_rxx_stat3_s           cn78xx;
	struct cvmx_ila_rxx_stat3_s           cn78xxp1;
};
typedef union cvmx_ila_rxx_stat3 cvmx_ila_rxx_stat3_t;

/**
 * cvmx_ila_rx#_stat4
 */
union cvmx_ila_rxx_stat4 {
	uint64_t u64;
	struct cvmx_ila_rxx_stat4_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t brst_shrt_err_cnt            : 16; /**< Indicates the number of bursts received that violate the BurstShort parameter. Wraps on
                                                         overflow if ILA_RX()_CFG0[LNK_STATS_WRAP]=1. Otherwise, saturates. On overflow/saturate,
                                                         sets ILA_RX()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t brst_shrt_err_cnt            : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_ila_rxx_stat4_s           cn78xx;
	struct cvmx_ila_rxx_stat4_s           cn78xxp1;
};
typedef union cvmx_ila_rxx_stat4 cvmx_ila_rxx_stat4_t;

/**
 * cvmx_ila_rx#_stat5
 */
union cvmx_ila_rxx_stat5 {
	uint64_t u64;
	struct cvmx_ila_rxx_stat5_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_23_63               : 41;
	uint64_t align_cnt                    : 23; /**< Indicates the number of alignment sequences received (i.e. those that do not violate the
                                                         current alignment). Wraps on overflow if ILA_RX()_CFG0[LNK_STATS_WRAP]=1. Otherwise,
                                                         saturates. On overflow/saturate, sets ILA_RX()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t align_cnt                    : 23;
	uint64_t reserved_23_63               : 41;
#endif
	} s;
	struct cvmx_ila_rxx_stat5_s           cn78xx;
	struct cvmx_ila_rxx_stat5_s           cn78xxp1;
};
typedef union cvmx_ila_rxx_stat5 cvmx_ila_rxx_stat5_t;

/**
 * cvmx_ila_rx#_stat6
 */
union cvmx_ila_rxx_stat6 {
	uint64_t u64;
	struct cvmx_ila_rxx_stat6_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t align_err_cnt                : 16; /**< Indicates the number of alignment sequences received in error (i.e. those that violate the
                                                         current alignment). Wraps on overflow if ILA_RX()_CFG0[LNK_STATS_WRAP]=1. Otherwise,
                                                         saturates. On overflow/saturate, sets ILA_RX()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t align_err_cnt                : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_ila_rxx_stat6_s           cn78xx;
	struct cvmx_ila_rxx_stat6_s           cn78xxp1;
};
typedef union cvmx_ila_rxx_stat6 cvmx_ila_rxx_stat6_t;

/**
 * cvmx_ila_rx#_stat7
 */
union cvmx_ila_rxx_stat7 {
	uint64_t u64;
	struct cvmx_ila_rxx_stat7_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t bad_64b67b_cnt               : 16; /**< Indicates the number of bad 64/67 bit code words. Wraps on overflow if
                                                         ILA_RX()_CFG0[LNK_STATS_WRAP]=1. Otherwise, saturates. On overflow/saturate, sets
                                                         ILA_RX()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t bad_64b67b_cnt               : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_ila_rxx_stat7_s           cn78xx;
	struct cvmx_ila_rxx_stat7_s           cn78xxp1;
};
typedef union cvmx_ila_rxx_stat7 cvmx_ila_rxx_stat7_t;

/**
 * cvmx_ila_rx#_stat8
 *
 * This register is reserved.
 *
 */
union cvmx_ila_rxx_stat8 {
	uint64_t u64;
	struct cvmx_ila_rxx_stat8_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_ila_rxx_stat8_s           cn78xx;
	struct cvmx_ila_rxx_stat8_s           cn78xxp1;
};
typedef union cvmx_ila_rxx_stat8 cvmx_ila_rxx_stat8_t;

/**
 * cvmx_ila_rx#_stat9
 *
 * This register is reserved.
 *
 */
union cvmx_ila_rxx_stat9 {
	uint64_t u64;
	struct cvmx_ila_rxx_stat9_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_ila_rxx_stat9_s           cn78xx;
	struct cvmx_ila_rxx_stat9_s           cn78xxp1;
};
typedef union cvmx_ila_rxx_stat9 cvmx_ila_rxx_stat9_t;

/**
 * cvmx_ila_rx_lne#_cfg
 */
union cvmx_ila_rx_lnex_cfg {
	uint64_t u64;
	struct cvmx_ila_rx_lnex_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t rx_dis_psh_skip              : 1;  /**< When asserted, skip words are discarded in the lane logic; when deasserted, skip words are
                                                         destripped.
                                                         If the lane is in internal loopback mode, this field is ignored and skip words are always
                                                         discarded in the lane logic. */
	uint64_t reserved_7_7                 : 1;
	uint64_t rx_dis_disp_chk              : 1;  /**< Disable the RX disparity check, see ILA_RX_LNE()_INT[DISP_ERR]. */
	uint64_t rx_scrm_sync                 : 1;  /**< RX scrambler-synchronization status. A 1 means synchronization has been achieved. */
	uint64_t rx_bdry_sync                 : 1;  /**< RX word-boundary-synchronization status. A 1 means synchronization has been achieved */
	uint64_t rx_dis_ukwn                  : 1;  /**< Disable normal response to unknown words. Unknown words are still logged but do not cause
                                                         an error to all open channels. */
	uint64_t rx_dis_scram                 : 1;  /**< Disable lane scrambler. For diagnostic use only. */
	uint64_t stat_rdclr                   : 1;  /**< A CSR read operation to ILA_RX_LNEn_STAT* clears the selected counter after returning its
                                                         current value. */
	uint64_t stat_ena                     : 1;  /**< Enable RX lane statistics counters. */
#else
	uint64_t stat_ena                     : 1;
	uint64_t stat_rdclr                   : 1;
	uint64_t rx_dis_scram                 : 1;
	uint64_t rx_dis_ukwn                  : 1;
	uint64_t rx_bdry_sync                 : 1;
	uint64_t rx_scrm_sync                 : 1;
	uint64_t rx_dis_disp_chk              : 1;
	uint64_t reserved_7_7                 : 1;
	uint64_t rx_dis_psh_skip              : 1;
	uint64_t reserved_9_63                : 55;
#endif
	} s;
	struct cvmx_ila_rx_lnex_cfg_s         cn78xx;
	struct cvmx_ila_rx_lnex_cfg_s         cn78xxp1;
};
typedef union cvmx_ila_rx_lnex_cfg cvmx_ila_rx_lnex_cfg_t;

/**
 * cvmx_ila_rx_lne#_int
 */
union cvmx_ila_rx_lnex_int {
	uint64_t u64;
	struct cvmx_ila_rx_lnex_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t disp_err                     : 1;  /**< RX disparity error encountered. Throws ILA_INTSN_E::ILA_RXLNE()_DISP_ERR. */
	uint64_t bad_64b67b                   : 1;  /**< Bad 64/67 bit code word encountered. Once the bad word reaches the burst-control unit (as
                                                         indicated by ILA_RX()_INT[LANE_BAD_WORD]) it is discarded and all open packets receive an
                                                         error. Throws ILA_INTSN_E::ILA_RXLNE()_BAD_64B67B. */
	uint64_t stat_cnt_ovfl                : 1;  /**< RX-lane statistic counter overflow. Throws ILA_INTSN_E::ILA_RXLNE()_STAT_CNT_OVFL. */
	uint64_t stat_msg                     : 1;  /**< Indicates that status bits for the link or a lane transitioned from a 1 (healthy) to a 0
                                                         (problem). Throws ILA_INTSN_E::ILA_RXLNE()_STAT_MSG. */
	uint64_t dskew_fifo_ovfl              : 1;  /**< RX deskew-FIFO overflow occurred. Throws ILA_INTSN_E::ILA_RXLNE()_DSKEW_FIFO_OVFL. */
	uint64_t scrm_sync_loss               : 1;  /**< Indicates that four consecutive bad sync words or three consecutive scramble-state
                                                         mismatches occurred. Throws ILA_INTSN_E::ILA_RXLNE()_SCRM_SYNC_LOSS. */
	uint64_t ukwn_cntl_word               : 1;  /**< Unknown framing-control word. Block type does not match any of (SYNC,SCRAM,SKIP,DIAG).
                                                         Throws ILA_INTSN_E::ILA_RXLNE()_UKWN_CNTL_WORD. */
	uint64_t crc32_err                    : 1;  /**< Diagnostic CRC32 errors. Throws ILA_INTSN_E::ILA_RXLNE()_CRC32_ERR. */
	uint64_t bdry_sync_loss               : 1;  /**< RX logic loses word-boundary sync (16 tries). Hardware automatically attempts to regain
                                                         word-boundary sync. Throws ILA_INTSN_E::ILA_RXLNE()_BDRY_SYNC_LOSS. */
	uint64_t serdes_lock_loss             : 1;  /**< RX SerDes loses lock. Throws ILA_INTSN_E::ILA_RXLNE()_SERDES_LOCK_LOSS. */
#else
	uint64_t serdes_lock_loss             : 1;
	uint64_t bdry_sync_loss               : 1;
	uint64_t crc32_err                    : 1;
	uint64_t ukwn_cntl_word               : 1;
	uint64_t scrm_sync_loss               : 1;
	uint64_t dskew_fifo_ovfl              : 1;
	uint64_t stat_msg                     : 1;
	uint64_t stat_cnt_ovfl                : 1;
	uint64_t bad_64b67b                   : 1;
	uint64_t disp_err                     : 1;
	uint64_t reserved_10_63               : 54;
#endif
	} s;
	struct cvmx_ila_rx_lnex_int_s         cn78xx;
	struct cvmx_ila_rx_lnex_int_s         cn78xxp1;
};
typedef union cvmx_ila_rx_lnex_int cvmx_ila_rx_lnex_int_t;

/**
 * cvmx_ila_rx_lne#_stat0
 */
union cvmx_ila_rx_lnex_stat0 {
	uint64_t u64;
	struct cvmx_ila_rx_lnex_stat0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t ser_lock_loss_cnt            : 18; /**< Indicates the number of times the lane lost clock-data-recovery. On overflow, saturates
                                                         and sets ILA_RX_LNE()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t ser_lock_loss_cnt            : 18;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_ila_rx_lnex_stat0_s       cn78xx;
	struct cvmx_ila_rx_lnex_stat0_s       cn78xxp1;
};
typedef union cvmx_ila_rx_lnex_stat0 cvmx_ila_rx_lnex_stat0_t;

/**
 * cvmx_ila_rx_lne#_stat1
 */
union cvmx_ila_rx_lnex_stat1 {
	uint64_t u64;
	struct cvmx_ila_rx_lnex_stat1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t bdry_sync_loss_cnt           : 18; /**< Indicates the number of times a lane lost word-boundary synchronization. On overflow,
                                                         saturates and sets ILA_RX_LNE()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t bdry_sync_loss_cnt           : 18;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_ila_rx_lnex_stat1_s       cn78xx;
	struct cvmx_ila_rx_lnex_stat1_s       cn78xxp1;
};
typedef union cvmx_ila_rx_lnex_stat1 cvmx_ila_rx_lnex_stat1_t;

/**
 * cvmx_ila_rx_lne#_stat10
 */
union cvmx_ila_rx_lnex_stat10 {
	uint64_t u64;
	struct cvmx_ila_rx_lnex_stat10_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_43_63               : 21;
	uint64_t prbs_bad                     : 11; /**< Indicates the number of training frames with bad PRBS. On overflow, saturates and sets
                                                         ILA_RX_LNE()_INT[STAT_CNT_OVFL]. */
	uint64_t reserved_11_31               : 21;
	uint64_t prbs_good                    : 11; /**< Indicates the number of training frames with correct PRBS. On overflow, saturates and sets
                                                         ILA_RX_LNE()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t prbs_good                    : 11;
	uint64_t reserved_11_31               : 21;
	uint64_t prbs_bad                     : 11;
	uint64_t reserved_43_63               : 21;
#endif
	} s;
	struct cvmx_ila_rx_lnex_stat10_s      cn78xx;
	struct cvmx_ila_rx_lnex_stat10_s      cn78xxp1;
};
typedef union cvmx_ila_rx_lnex_stat10 cvmx_ila_rx_lnex_stat10_t;

/**
 * cvmx_ila_rx_lne#_stat2
 */
union cvmx_ila_rx_lnex_stat2 {
	uint64_t u64;
	struct cvmx_ila_rx_lnex_stat2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_50_63               : 14;
	uint64_t syncw_good_cnt               : 18; /**< Indicates the number of good synchronization words. On overflow, saturates and sets
                                                         ILA_RX_LNE()_INT[STAT_CNT_OVFL]. */
	uint64_t reserved_18_31               : 14;
	uint64_t syncw_bad_cnt                : 18; /**< Indicates the number of bad synchronization words. On overflow, saturates and sets
                                                         ILA_RX_LNE()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t syncw_bad_cnt                : 18;
	uint64_t reserved_18_31               : 14;
	uint64_t syncw_good_cnt               : 18;
	uint64_t reserved_50_63               : 14;
#endif
	} s;
	struct cvmx_ila_rx_lnex_stat2_s       cn78xx;
	struct cvmx_ila_rx_lnex_stat2_s       cn78xxp1;
};
typedef union cvmx_ila_rx_lnex_stat2 cvmx_ila_rx_lnex_stat2_t;

/**
 * cvmx_ila_rx_lne#_stat3
 */
union cvmx_ila_rx_lnex_stat3 {
	uint64_t u64;
	struct cvmx_ila_rx_lnex_stat3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t bad_64b67b_cnt               : 18; /**< Indicates the number of bad 64/67 bit words, meaning bit <65> or bit <64> has been
                                                         corrupted. On overflow, saturates and sets ILA_RX_LNE()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t bad_64b67b_cnt               : 18;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_ila_rx_lnex_stat3_s       cn78xx;
	struct cvmx_ila_rx_lnex_stat3_s       cn78xxp1;
};
typedef union cvmx_ila_rx_lnex_stat3 cvmx_ila_rx_lnex_stat3_t;

/**
 * cvmx_ila_rx_lne#_stat4
 */
union cvmx_ila_rx_lnex_stat4 {
	uint64_t u64;
	struct cvmx_ila_rx_lnex_stat4_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_59_63               : 5;
	uint64_t cntl_word_cnt                : 27; /**< Indicates the number of control words received. On overflow, saturates and sets
                                                         ILA_RX_LNE()_INT[STAT_CNT_OVFL]. */
	uint64_t reserved_27_31               : 5;
	uint64_t data_word_cnt                : 27; /**< Indicates the number of data words received. On overflow, saturates and sets
                                                         ILA_RX_LNE()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t data_word_cnt                : 27;
	uint64_t reserved_27_31               : 5;
	uint64_t cntl_word_cnt                : 27;
	uint64_t reserved_59_63               : 5;
#endif
	} s;
	struct cvmx_ila_rx_lnex_stat4_s       cn78xx;
	struct cvmx_ila_rx_lnex_stat4_s       cn78xxp1;
};
typedef union cvmx_ila_rx_lnex_stat4 cvmx_ila_rx_lnex_stat4_t;

/**
 * cvmx_ila_rx_lne#_stat5
 */
union cvmx_ila_rx_lnex_stat5 {
	uint64_t u64;
	struct cvmx_ila_rx_lnex_stat5_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t unkwn_word_cnt               : 18; /**< Indicates the number of unknown control words. On overflow, saturates and sets
                                                         ILA_RX_LNE()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t unkwn_word_cnt               : 18;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_ila_rx_lnex_stat5_s       cn78xx;
	struct cvmx_ila_rx_lnex_stat5_s       cn78xxp1;
};
typedef union cvmx_ila_rx_lnex_stat5 cvmx_ila_rx_lnex_stat5_t;

/**
 * cvmx_ila_rx_lne#_stat6
 */
union cvmx_ila_rx_lnex_stat6 {
	uint64_t u64;
	struct cvmx_ila_rx_lnex_stat6_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t scrm_sync_loss_cnt           : 18; /**< Indicates the number of times scrambler synchronization was lost (due to either four
                                                         consecutive bad sync words or three consecutive scrambler-state mismatches). On overflow,
                                                         saturates and sets ILA_RX_LNE()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t scrm_sync_loss_cnt           : 18;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_ila_rx_lnex_stat6_s       cn78xx;
	struct cvmx_ila_rx_lnex_stat6_s       cn78xxp1;
};
typedef union cvmx_ila_rx_lnex_stat6 cvmx_ila_rx_lnex_stat6_t;

/**
 * cvmx_ila_rx_lne#_stat7
 */
union cvmx_ila_rx_lnex_stat7 {
	uint64_t u64;
	struct cvmx_ila_rx_lnex_stat7_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t scrm_match_cnt               : 18; /**< Indicates the number of scrambler-state matches received. On overflow, saturates and sets
                                                         ILA_RX_LNE()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t scrm_match_cnt               : 18;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_ila_rx_lnex_stat7_s       cn78xx;
	struct cvmx_ila_rx_lnex_stat7_s       cn78xxp1;
};
typedef union cvmx_ila_rx_lnex_stat7 cvmx_ila_rx_lnex_stat7_t;

/**
 * cvmx_ila_rx_lne#_stat8
 */
union cvmx_ila_rx_lnex_stat8 {
	uint64_t u64;
	struct cvmx_ila_rx_lnex_stat8_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t skipw_good_cnt               : 18; /**< Indicates the number of good skip words. On overflow, saturates and sets
                                                         ILA_RX_LNE()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t skipw_good_cnt               : 18;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_ila_rx_lnex_stat8_s       cn78xx;
	struct cvmx_ila_rx_lnex_stat8_s       cn78xxp1;
};
typedef union cvmx_ila_rx_lnex_stat8 cvmx_ila_rx_lnex_stat8_t;

/**
 * cvmx_ila_rx_lne#_stat9
 */
union cvmx_ila_rx_lnex_stat9 {
	uint64_t u64;
	struct cvmx_ila_rx_lnex_stat9_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_50_63               : 14;
	uint64_t crc32_err_cnt                : 18; /**< Indicates the number of errors in the lane CRC. On overflow, saturates and sets
                                                         ILA_RX_LNE()_INT[STAT_CNT_OVFL]. */
	uint64_t reserved_27_31               : 5;
	uint64_t crc32_match_cnt              : 27; /**< Indicates the number of CRC32 matches received. On overflow, saturates and sets
                                                         ILA_RX_LNE()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t crc32_match_cnt              : 27;
	uint64_t reserved_27_31               : 5;
	uint64_t crc32_err_cnt                : 18;
	uint64_t reserved_50_63               : 14;
#endif
	} s;
	struct cvmx_ila_rx_lnex_stat9_s       cn78xx;
	struct cvmx_ila_rx_lnex_stat9_s       cn78xxp1;
};
typedef union cvmx_ila_rx_lnex_stat9 cvmx_ila_rx_lnex_stat9_t;

/**
 * cvmx_ila_ser_cfg
 */
union cvmx_ila_ser_cfg {
	uint64_t u64;
	struct cvmx_ila_ser_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_57_63               : 7;
	uint64_t ser_rxpol_auto               : 1;  /**< SerDes lane receive polarity autodetection mode. */
	uint64_t reserved_48_55               : 8;
	uint64_t ser_rxpol                    : 8;  /**< SerDes lane receive polarity.
                                                         0x0 = RX without inversion.
                                                         0x1 = RX with inversion.
                                                         Note that ILK_RX()_CFG0[LANE_REV] has no effect on this mapping.
                                                          _ [SER_RXPOL<0>]  = QLM2 lane 0.
                                                          _ [SER_RXPOL<1>]  = QLM2 lane 1.
                                                          _ [SER_RXPOL<2>]  = QLM2 lane 2.
                                                          _ [SER_RXPOL<3>]  = QLM2 lane 3.
                                                          _ [SER_RXPOL<4>]  = QLM3 lane 0.
                                                          _ [SER_RXPOL<5>]  = QLM3 lane 1.
                                                          _ [SER_RXPOL<6>]  = QLM3 lane 2.
                                                          _ [SER_RXPOL<7>]  = QLM3 lane 3. */
	uint64_t reserved_32_39               : 8;
	uint64_t ser_txpol                    : 8;  /**< SerDes lane transmit polarity.
                                                         0x0 = TX without inversion.
                                                         0x1 = TX with inversion.
                                                         Note that ILK_TX()_CFG0[LANE_REV] has no effect on this mapping.
                                                          _ [SER_TXPOL<0>]  = QLM2 lane 0.
                                                          _ [SER_TXPOL<1>]  = QLM2 lane 1.
                                                          _ [SER_TXPOL<2>]  = QLM2 lane 2.
                                                          _ [SER_TXPOL<3>]  = QLM2 lane 3.
                                                          _ [SER_TXPOL<4>]  = QLM3 lane 0.
                                                          _ [SER_TXPOL<5>]  = QLM3 lane 1.
                                                          _ [SER_TXPOL<6>]  = QLM3 lane 2.
                                                          _ [SER_TXPOL<7>]  = QLM3 lane 3. */
	uint64_t reserved_16_23               : 8;
	uint64_t ser_reset_n                  : 8;  /**< SerDes lane reset. Should be set when the GSER is ready to transfer data, as indicated
                                                         by the corresponding GSER()_QLM_STAT[RST_RDY]. Note that
                                                         neither ILK_TX()_CFG0[LANE_REV] nor ILK_RX()_CFG0[LANE_REV] has an effect on this mapping.
                                                         The correlation of [SER_RESET_N] bits to GSER's is as follows:
                                                         _ [SER_RESET_N<0>]  = QLM2 lane 0, GSER(2)_QLM_STAT[RST_RDY].
                                                         _ [SER_RESET_N<1>]  = QLM2 lane 1, GSER(2)_QLM_STAT[RST_RDY].
                                                         _ [SER_RESET_N<2>]  = QLM2 lane 2, GSER(2)_QLM_STAT[RST_RDY].
                                                         _ [SER_RESET_N<3>]  = QLM2 lane 3, GSER(2)_QLM_STAT[RST_RDY].
                                                         _ [SER_RESET_N<4>]  = QLM3 lane 0, GSER(3)_QLM_STAT[RST_RDY].
                                                         _ [SER_RESET_N<5>]  = QLM3 lane 1, GSER(3)_QLM_STAT[RST_RDY].
                                                         _ [SER_RESET_N<6>]  = QLM3 lane 2, GSER(3)_QLM_STAT[RST_RDY].
                                                         _ [SER_RESET_N<7>]  = QLM3 lane 3, GSER(3)_QLM_STAT[RST_RDY]. */
	uint64_t reserved_0_7                 : 8;
#else
	uint64_t reserved_0_7                 : 8;
	uint64_t ser_reset_n                  : 8;
	uint64_t reserved_16_23               : 8;
	uint64_t ser_txpol                    : 8;
	uint64_t reserved_32_39               : 8;
	uint64_t ser_rxpol                    : 8;
	uint64_t reserved_48_55               : 8;
	uint64_t ser_rxpol_auto               : 1;
	uint64_t reserved_57_63               : 7;
#endif
	} s;
	struct cvmx_ila_ser_cfg_s             cn78xx;
	struct cvmx_ila_ser_cfg_s             cn78xxp1;
};
typedef union cvmx_ila_ser_cfg cvmx_ila_ser_cfg_t;

/**
 * cvmx_ila_tx#_byte_cnt#
 */
union cvmx_ila_txx_byte_cntx {
	uint64_t u64;
	struct cvmx_ila_txx_byte_cntx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_40_63               : 24;
	uint64_t tx_bytes                     : 40; /**< Number of bytes transmitted per channel. Wraps on overflow. On overflow, sets
                                                         ILA_TX()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t tx_bytes                     : 40;
	uint64_t reserved_40_63               : 24;
#endif
	} s;
	struct cvmx_ila_txx_byte_cntx_s       cn78xx;
	struct cvmx_ila_txx_byte_cntx_s       cn78xxp1;
};
typedef union cvmx_ila_txx_byte_cntx cvmx_ila_txx_byte_cntx_t;

/**
 * cvmx_ila_tx#_cfg0
 */
union cvmx_ila_txx_cfg0 {
	uint64_t u64;
	struct cvmx_ila_txx_cfg0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_63_63               : 1;
	uint64_t ext_lpbk                     : 1;  /**< Enable RX-TX data external loopback. Note that with differing transmit and receive clocks,
                                                         skip word are inserted/deleted. Must set ILA_TX()_CFG1[RX_LINK_FC_IGN] whenever enabling
                                                         external loopback. */
	uint64_t int_lpbk                     : 1;  /**< Enable TX-RX internal loopback. */
	uint64_t txf_byp_dis                  : 1;  /**< Disable TXF bypass. */
	uint64_t reserved_56_59               : 4;
	uint64_t lnk_stats_rdclr              : 1;  /**< CSR read to ILA_TX(0)_STAT* clears the counter after returning its current value. */
	uint64_t lnk_stats_ena                : 1;  /**< Enable link statistics counters */
	uint64_t reserved_52_53               : 2;
	uint64_t mfrm_len                     : 13; /**< The quantity of data sent on each lane including one sync word, scrambler state, diag
                                                         word, zero or more skip words, and the data payload. Must be larger than
                                                         ILA_TX()_CFG1[SKIP_CNT] + 9.
                                                         Supported range:
                                                         _ ILA_TX()_CFG1[SKIP_CNT] + 9 < [MFRM_LEN] <= 4096) */
	uint64_t brst_shrt                    : 7;  /**< Minimum interval between burst control words, as a multiple of eight bytes. Supported
                                                         range from eight to 512 bytes
                                                         (i.e. 0 < [BRST_SHRT] <= 64). */
	uint64_t lane_rev                     : 1;  /**< Lane reversal.   When enabled, lane striping is performed from most significant lane
                                                         enabled to least significant lane enabled. [LANE_ENA] must be zero before changing
                                                         [LANE_REV]. */
	uint64_t brst_max                     : 5;  /**< Maximum size of a data burst, as a multiple of 64 byte blocks.
                                                         Supported range is from 64 bytes to 1024 bytes
                                                         (i.e. 0 < [BRST_MAX] <= 16). */
	uint64_t reserved_8_25                : 18;
	uint64_t lane_ena                     : 8;  /**< Lane enable mask. Link is enabled if any lane is enabled. The same lane should not be
                                                         enabled in multiple ILA_TX()_CFG0. Each bit of LANE_ENA maps to a TX lane (TLE) and a QLM
                                                         lane. Note that [LANE_REV] has no effect on this mapping.
                                                         _ [LANE_ENA<0>]  = TLE0  =  QLM2 lane 0.
                                                         _ [LANE_ENA<1>]  = TLE1  =  QLM2 lane 1.
                                                         _ [LANE_ENA<2>]  = TLE2  =  QLM2 lane 2.
                                                         _ [LANE_ENA<3>]  = TLE3  =  QLM2 lane 3.
                                                         _ [LANE_ENA<4>]  = TLE4  =  QLM3 lane 0.
                                                         _ [LANE_ENA<5>]  = TLE5  =  QLM3 lane 1.
                                                         _ [LANE_ENA<6>]  = TLE6  =  QLM3 lane 2.
                                                         _ [LANE_ENA<7>]  = TLE7  =  QLM3 lane 3. */
#else
	uint64_t lane_ena                     : 8;
	uint64_t reserved_8_25                : 18;
	uint64_t brst_max                     : 5;
	uint64_t lane_rev                     : 1;
	uint64_t brst_shrt                    : 7;
	uint64_t mfrm_len                     : 13;
	uint64_t reserved_52_53               : 2;
	uint64_t lnk_stats_ena                : 1;
	uint64_t lnk_stats_rdclr              : 1;
	uint64_t reserved_56_59               : 4;
	uint64_t txf_byp_dis                  : 1;
	uint64_t int_lpbk                     : 1;
	uint64_t ext_lpbk                     : 1;
	uint64_t reserved_63_63               : 1;
#endif
	} s;
	struct cvmx_ila_txx_cfg0_s            cn78xx;
	struct cvmx_ila_txx_cfg0_s            cn78xxp1;
};
typedef union cvmx_ila_txx_cfg0 cvmx_ila_txx_cfg0_t;

/**
 * cvmx_ila_tx#_cfg1
 */
union cvmx_ila_txx_cfg1 {
	uint64_t u64;
	struct cvmx_ila_txx_cfg1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ser_low                      : 4;  /**< Reserved. */
	uint64_t reserved_43_59               : 17;
	uint64_t ser_limit                    : 10; /**< Reduce latency by limiting the amount of data in flight for each SerDes. If 0x0, hardware
                                                         will compute it. Otherwise, SER_LIMIT must be set as follows:
                                                         _ SER_LIMIT >= 148 + (BAUD / SCLK) * (12 + NUM_LANES)
                                                         For instance, for SCLK=1.1GHz,BAUD=10.3125,NUM_LANES=8:
                                                         _ SER_LIMIT >= 148 + (10.3125 / 1.1 * (12+ 8))
                                                         _ SER_LIMIT >= 336 */
	uint64_t pkt_busy                     : 1;  /**< Packet busy. When [PKT_ENA]=0, [PKT_BUSY]=1 indicates the TX-link is
                                                         transmitting data. When [PKT_ENA]=1, [PKT_BUSY] is undefined. */
	uint64_t reserved_26_31               : 6;
	uint64_t skip_cnt                     : 4;  /**< Number of skip words to insert after the scrambler state. */
	uint64_t pkt_flush                    : 1;  /**< Packet transmit flush. When asserted, the TxFIFO continuously drains; all data is dropped.
                                                         Software should first write
                                                         PKT_ENA = 0 and wait for PKT_BUSY = 0. */
	uint64_t pkt_ena                      : 1;  /**< Packet transmit enable. When zero, the TX-link stops transmitting packets, as per RX_LINK_FC_PKT. */
	uint64_t la_mode                      : 1;  /**< Enable Interlaken look-aside traffic. Used to set the protocol type of idle words. */
	uint64_t reserved_12_18               : 7;
	uint64_t tx_link_fc_jam               : 1;  /**< Reserved. */
	uint64_t rx_link_fc_pkt               : 1;  /**< Flow-control received in burst/idle control words cause TX-link to stop transmitting at
                                                         the end of a packet instead of the end of a burst. */
	uint64_t rx_link_fc_ign               : 1;  /**< Ignore flow-control status received in burst/idle control words. */
	uint64_t rmatch                       : 1;  /**< Enable rate-matching circuitry. */
	uint64_t reserved_0_7                 : 8;
#else
	uint64_t reserved_0_7                 : 8;
	uint64_t rmatch                       : 1;
	uint64_t rx_link_fc_ign               : 1;
	uint64_t rx_link_fc_pkt               : 1;
	uint64_t tx_link_fc_jam               : 1;
	uint64_t reserved_12_18               : 7;
	uint64_t la_mode                      : 1;
	uint64_t pkt_ena                      : 1;
	uint64_t pkt_flush                    : 1;
	uint64_t skip_cnt                     : 4;
	uint64_t reserved_26_31               : 6;
	uint64_t pkt_busy                     : 1;
	uint64_t ser_limit                    : 10;
	uint64_t reserved_43_59               : 17;
	uint64_t ser_low                      : 4;
#endif
	} s;
	struct cvmx_ila_txx_cfg1_s            cn78xx;
	struct cvmx_ila_txx_cfg1_s            cn78xxp1;
};
typedef union cvmx_ila_txx_cfg1 cvmx_ila_txx_cfg1_t;

/**
 * cvmx_ila_tx#_cha_xon
 */
union cvmx_ila_txx_cha_xon {
	uint64_t u64;
	struct cvmx_ila_txx_cha_xon_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t ch1_xon                      : 1;  /**< Flow-control status for channel 1, where a value of 0 indicates the presence of
                                                         backpressure (i.e. XOFF) and a value of 1 indicates the absence of backpressure (i.e.
                                                         XON). */
	uint64_t ch0_xon                      : 1;  /**< Flow-control status for channel 0, where a value of 0 indicates the presence of
                                                         backpressure (i.e. XOFF) and a value of 1 indicates the absence of backpressure (i.e.
                                                         XON). */
#else
	uint64_t ch0_xon                      : 1;
	uint64_t ch1_xon                      : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_ila_txx_cha_xon_s         cn78xx;
	struct cvmx_ila_txx_cha_xon_s         cn78xxp1;
};
typedef union cvmx_ila_txx_cha_xon cvmx_ila_txx_cha_xon_t;

/**
 * cvmx_ila_tx#_dbg
 */
union cvmx_ila_txx_dbg {
	uint64_t u64;
	struct cvmx_ila_txx_dbg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_29_63               : 35;
	uint64_t data_rate                    : 13; /**< Reserved. */
	uint64_t low_delay                    : 6;  /**< Reserved. */
	uint64_t reserved_3_9                 : 7;
	uint64_t tx_bad_crc24                 : 1;  /**< Send a control word with bad CRC24. Hardware clears this field once the injection is performed. */
	uint64_t tx_bad_ctlw2                 : 1;  /**< Send a control word without the control bit set. */
	uint64_t tx_bad_ctlw1                 : 1;  /**< Send a data word with the control bit set. */
#else
	uint64_t tx_bad_ctlw1                 : 1;
	uint64_t tx_bad_ctlw2                 : 1;
	uint64_t tx_bad_crc24                 : 1;
	uint64_t reserved_3_9                 : 7;
	uint64_t low_delay                    : 6;
	uint64_t data_rate                    : 13;
	uint64_t reserved_29_63               : 35;
#endif
	} s;
	struct cvmx_ila_txx_dbg_s             cn78xx;
	struct cvmx_ila_txx_dbg_s             cn78xxp1;
};
typedef union cvmx_ila_txx_dbg cvmx_ila_txx_dbg_t;

/**
 * cvmx_ila_tx#_err_cfg
 */
union cvmx_ila_txx_err_cfg {
	uint64_t u64;
	struct cvmx_ila_txx_err_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t txf_flip                     : 2;  /**< Testing feature. Flip syndrome bits <1:0> on writes to the TXF RAM to test single-bit or
                                                         double-bit errors. */
	uint64_t reserved_1_15                : 15;
	uint64_t txf_cor_dis                  : 1;  /**< Disable ECC corrector on TXF. */
#else
	uint64_t txf_cor_dis                  : 1;
	uint64_t reserved_1_15                : 15;
	uint64_t txf_flip                     : 2;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_ila_txx_err_cfg_s         cn78xx;
	struct cvmx_ila_txx_err_cfg_s         cn78xxp1;
};
typedef union cvmx_ila_txx_err_cfg cvmx_ila_txx_err_cfg_t;

/**
 * cvmx_ila_tx#_int
 */
union cvmx_ila_txx_int {
	uint64_t u64;
	struct cvmx_ila_txx_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t txf_dbe                      : 1;  /**< TX FIFO double-bit error. Throws ILA_INTSN_E::ILA_TX()_TXF_DBE. See also
                                                         ILA_TX()_ERR_CFG. */
	uint64_t txf_sbe                      : 1;  /**< TX FIFO single-bit error. Throws ILA_INTSN_E::ILA_TX()_TXF_SBE. See also
                                                         ILA_TX()_ERR_CFG. */
	uint64_t stat_cnt_ovfl                : 1;  /**< Statistics counter overflow. Throws ILA_INTSN_E::ILA_TX()_STAT_CNT_OVFL. */
	uint64_t reserved_0_2                 : 3;
#else
	uint64_t reserved_0_2                 : 3;
	uint64_t stat_cnt_ovfl                : 1;
	uint64_t txf_sbe                      : 1;
	uint64_t txf_dbe                      : 1;
	uint64_t reserved_6_63                : 58;
#endif
	} s;
	struct cvmx_ila_txx_int_s             cn78xx;
	struct cvmx_ila_txx_int_s             cn78xxp1;
};
typedef union cvmx_ila_txx_int cvmx_ila_txx_int_t;

/**
 * cvmx_ila_tx#_pkt_cnt#
 */
union cvmx_ila_txx_pkt_cntx {
	uint64_t u64;
	struct cvmx_ila_txx_pkt_cntx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_28_63               : 36;
	uint64_t tx_pkt                       : 28; /**< Number of packets transmitted per channel. Wraps on overflow. On overflow, sets
                                                         ILA_TX()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t tx_pkt                       : 28;
	uint64_t reserved_28_63               : 36;
#endif
	} s;
	struct cvmx_ila_txx_pkt_cntx_s        cn78xx;
	struct cvmx_ila_txx_pkt_cntx_s        cn78xxp1;
};
typedef union cvmx_ila_txx_pkt_cntx cvmx_ila_txx_pkt_cntx_t;

/**
 * cvmx_ila_tx#_rmatch
 */
union cvmx_ila_txx_rmatch {
	uint64_t u64;
	struct cvmx_ila_txx_rmatch_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_50_63               : 14;
	uint64_t grnlrty                      : 2;  /**< Reserved. */
	uint64_t brst_limit                   : 16; /**< Size of token bucket, also the maximum quantity of data that can be burst across the
                                                         interface before invoking rate-limiting logic. */
	uint64_t time_limit                   : 16; /**< Number of cycles per time interval. Must be >= 4. */
	uint64_t rate_limit                   : 16; /**< Number of tokens added to the bucket when the interval timer expires. */
#else
	uint64_t rate_limit                   : 16;
	uint64_t time_limit                   : 16;
	uint64_t brst_limit                   : 16;
	uint64_t grnlrty                      : 2;
	uint64_t reserved_50_63               : 14;
#endif
	} s;
	struct cvmx_ila_txx_rmatch_s          cn78xx;
	struct cvmx_ila_txx_rmatch_s          cn78xxp1;
};
typedef union cvmx_ila_txx_rmatch cvmx_ila_txx_rmatch_t;

#endif
