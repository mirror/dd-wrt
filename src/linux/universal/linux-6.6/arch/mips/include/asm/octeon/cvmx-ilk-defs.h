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
 * cvmx-ilk-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon ilk.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_ILK_DEFS_H__
#define __CVMX_ILK_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ILK_BIST_SUM CVMX_ILK_BIST_SUM_FUNC()
static inline uint64_t CVMX_ILK_BIST_SUM_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ILK_BIST_SUM not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180014000038ull);
}
#else
#define CVMX_ILK_BIST_SUM (CVMX_ADD_IO_SEG(0x0001180014000038ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ILK_GBL_CFG CVMX_ILK_GBL_CFG_FUNC()
static inline uint64_t CVMX_ILK_GBL_CFG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ILK_GBL_CFG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180014000000ull);
}
#else
#define CVMX_ILK_GBL_CFG (CVMX_ADD_IO_SEG(0x0001180014000000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ILK_GBL_ERR_CFG CVMX_ILK_GBL_ERR_CFG_FUNC()
static inline uint64_t CVMX_ILK_GBL_ERR_CFG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ILK_GBL_ERR_CFG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180014000058ull);
}
#else
#define CVMX_ILK_GBL_ERR_CFG (CVMX_ADD_IO_SEG(0x0001180014000058ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ILK_GBL_INT CVMX_ILK_GBL_INT_FUNC()
static inline uint64_t CVMX_ILK_GBL_INT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ILK_GBL_INT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180014000008ull);
}
#else
#define CVMX_ILK_GBL_INT (CVMX_ADD_IO_SEG(0x0001180014000008ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ILK_GBL_INT_EN CVMX_ILK_GBL_INT_EN_FUNC()
static inline uint64_t CVMX_ILK_GBL_INT_EN_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_ILK_GBL_INT_EN not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180014000010ull);
}
#else
#define CVMX_ILK_GBL_INT_EN (CVMX_ADD_IO_SEG(0x0001180014000010ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ILK_INT_SUM CVMX_ILK_INT_SUM_FUNC()
static inline uint64_t CVMX_ILK_INT_SUM_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_ILK_INT_SUM not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180014000030ull);
}
#else
#define CVMX_ILK_INT_SUM (CVMX_ADD_IO_SEG(0x0001180014000030ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_LNEX_TRN_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 15))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 15)))))
		cvmx_warn("CVMX_ILK_LNEX_TRN_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800140380F0ull) + ((offset) & 15) * 1024;
}
#else
#define CVMX_ILK_LNEX_TRN_CTL(offset) (CVMX_ADD_IO_SEG(0x00011800140380F0ull) + ((offset) & 15) * 1024)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_LNEX_TRN_LD(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 15))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 15)))))
		cvmx_warn("CVMX_ILK_LNEX_TRN_LD(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800140380E0ull) + ((offset) & 15) * 1024;
}
#else
#define CVMX_ILK_LNEX_TRN_LD(offset) (CVMX_ADD_IO_SEG(0x00011800140380E0ull) + ((offset) & 15) * 1024)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_LNEX_TRN_LP(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 15))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 15)))))
		cvmx_warn("CVMX_ILK_LNEX_TRN_LP(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800140380E8ull) + ((offset) & 15) * 1024;
}
#else
#define CVMX_ILK_LNEX_TRN_LP(offset) (CVMX_ADD_IO_SEG(0x00011800140380E8ull) + ((offset) & 15) * 1024)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ILK_LNE_DBG CVMX_ILK_LNE_DBG_FUNC()
static inline uint64_t CVMX_ILK_LNE_DBG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ILK_LNE_DBG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180014030008ull);
}
#else
#define CVMX_ILK_LNE_DBG (CVMX_ADD_IO_SEG(0x0001180014030008ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ILK_LNE_STS_MSG CVMX_ILK_LNE_STS_MSG_FUNC()
static inline uint64_t CVMX_ILK_LNE_STS_MSG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ILK_LNE_STS_MSG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180014030000ull);
}
#else
#define CVMX_ILK_LNE_STS_MSG (CVMX_ADD_IO_SEG(0x0001180014030000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ILK_RID_CFG CVMX_ILK_RID_CFG_FUNC()
static inline uint64_t CVMX_ILK_RID_CFG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ILK_RID_CFG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180014000050ull);
}
#else
#define CVMX_ILK_RID_CFG (CVMX_ADD_IO_SEG(0x0001180014000050ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ILK_RXF_IDX_PMAP CVMX_ILK_RXF_IDX_PMAP_FUNC()
static inline uint64_t CVMX_ILK_RXF_IDX_PMAP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_ILK_RXF_IDX_PMAP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180014000020ull);
}
#else
#define CVMX_ILK_RXF_IDX_PMAP (CVMX_ADD_IO_SEG(0x0001180014000020ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ILK_RXF_MEM_PMAP CVMX_ILK_RXF_MEM_PMAP_FUNC()
static inline uint64_t CVMX_ILK_RXF_MEM_PMAP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_ILK_RXF_MEM_PMAP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180014000028ull);
}
#else
#define CVMX_ILK_RXF_MEM_PMAP (CVMX_ADD_IO_SEG(0x0001180014000028ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RXX_BYTE_CNTX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 255)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 255)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_ILK_RXX_BYTE_CNTX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180014023000ull) + (((offset) & 255) + ((block_id) & 1) * 0x800ull) * 8;
}
#else
#define CVMX_ILK_RXX_BYTE_CNTX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180014023000ull) + (((offset) & 255) + ((block_id) & 1) * 0x800ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RXX_CAL_ENTRYX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 287)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 287)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_ILK_RXX_CAL_ENTRYX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180014021000ull) + (((offset) & 511) + ((block_id) & 1) * 0x800ull) * 8;
}
#else
#define CVMX_ILK_RXX_CAL_ENTRYX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180014021000ull) + (((offset) & 511) + ((block_id) & 1) * 0x800ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RXX_CFG0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_RXX_CFG0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014020000ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_RXX_CFG0(offset) (CVMX_ADD_IO_SEG(0x0001180014020000ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RXX_CFG1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_RXX_CFG1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014020008ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_RXX_CFG1(offset) (CVMX_ADD_IO_SEG(0x0001180014020008ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RXX_CHAX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 255)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 255)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_ILK_RXX_CHAX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180014002000ull) + (((offset) & 255) + ((block_id) & 1) * 0x200ull) * 8;
}
#else
#define CVMX_ILK_RXX_CHAX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180014002000ull) + (((offset) & 255) + ((block_id) & 1) * 0x200ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RXX_CHA_XONX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 3)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_ILK_RXX_CHA_XONX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180014020400ull) + (((offset) & 3) + ((block_id) & 1) * 0x800ull) * 8;
}
#else
#define CVMX_ILK_RXX_CHA_XONX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180014020400ull) + (((offset) & 3) + ((block_id) & 1) * 0x800ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RXX_ERR_CFG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_RXX_ERR_CFG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800140200E0ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_RXX_ERR_CFG(offset) (CVMX_ADD_IO_SEG(0x00011800140200E0ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RXX_FLOW_CTL0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_RXX_FLOW_CTL0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014020090ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_RXX_FLOW_CTL0(offset) (CVMX_ADD_IO_SEG(0x0001180014020090ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RXX_FLOW_CTL1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_RXX_FLOW_CTL1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014020098ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_RXX_FLOW_CTL1(offset) (CVMX_ADD_IO_SEG(0x0001180014020098ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RXX_IDX_CAL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_RXX_IDX_CAL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800140200A0ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_RXX_IDX_CAL(offset) (CVMX_ADD_IO_SEG(0x00011800140200A0ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RXX_IDX_STAT0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_RXX_IDX_STAT0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014020070ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_RXX_IDX_STAT0(offset) (CVMX_ADD_IO_SEG(0x0001180014020070ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RXX_IDX_STAT1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_RXX_IDX_STAT1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014020078ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_RXX_IDX_STAT1(offset) (CVMX_ADD_IO_SEG(0x0001180014020078ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RXX_INT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_RXX_INT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014020010ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_RXX_INT(offset) (CVMX_ADD_IO_SEG(0x0001180014020010ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RXX_INT_EN(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_RXX_INT_EN(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014020018ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_RXX_INT_EN(offset) (CVMX_ADD_IO_SEG(0x0001180014020018ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RXX_JABBER(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_RXX_JABBER(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800140200B8ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_RXX_JABBER(offset) (CVMX_ADD_IO_SEG(0x00011800140200B8ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RXX_MEM_CAL0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_RXX_MEM_CAL0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800140200A8ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_RXX_MEM_CAL0(offset) (CVMX_ADD_IO_SEG(0x00011800140200A8ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RXX_MEM_CAL1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_RXX_MEM_CAL1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800140200B0ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_RXX_MEM_CAL1(offset) (CVMX_ADD_IO_SEG(0x00011800140200B0ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RXX_MEM_STAT0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_RXX_MEM_STAT0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014020080ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_RXX_MEM_STAT0(offset) (CVMX_ADD_IO_SEG(0x0001180014020080ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RXX_MEM_STAT1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_RXX_MEM_STAT1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014020088ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_RXX_MEM_STAT1(offset) (CVMX_ADD_IO_SEG(0x0001180014020088ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RXX_PKT_CNTX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 255)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 255)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_ILK_RXX_PKT_CNTX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180014022000ull) + (((offset) & 255) + ((block_id) & 1) * 0x800ull) * 8;
}
#else
#define CVMX_ILK_RXX_PKT_CNTX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180014022000ull) + (((offset) & 255) + ((block_id) & 1) * 0x800ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RXX_RID(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_RXX_RID(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800140200C0ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_RXX_RID(offset) (CVMX_ADD_IO_SEG(0x00011800140200C0ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RXX_STAT0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_RXX_STAT0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014020020ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_RXX_STAT0(offset) (CVMX_ADD_IO_SEG(0x0001180014020020ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RXX_STAT1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_RXX_STAT1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014020028ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_RXX_STAT1(offset) (CVMX_ADD_IO_SEG(0x0001180014020028ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RXX_STAT2(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_RXX_STAT2(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014020030ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_RXX_STAT2(offset) (CVMX_ADD_IO_SEG(0x0001180014020030ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RXX_STAT3(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_RXX_STAT3(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014020038ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_RXX_STAT3(offset) (CVMX_ADD_IO_SEG(0x0001180014020038ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RXX_STAT4(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_RXX_STAT4(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014020040ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_RXX_STAT4(offset) (CVMX_ADD_IO_SEG(0x0001180014020040ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RXX_STAT5(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_RXX_STAT5(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014020048ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_RXX_STAT5(offset) (CVMX_ADD_IO_SEG(0x0001180014020048ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RXX_STAT6(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_RXX_STAT6(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014020050ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_RXX_STAT6(offset) (CVMX_ADD_IO_SEG(0x0001180014020050ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RXX_STAT7(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_RXX_STAT7(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014020058ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_RXX_STAT7(offset) (CVMX_ADD_IO_SEG(0x0001180014020058ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RXX_STAT8(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_RXX_STAT8(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014020060ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_RXX_STAT8(offset) (CVMX_ADD_IO_SEG(0x0001180014020060ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RXX_STAT9(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_RXX_STAT9(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014020068ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_RXX_STAT9(offset) (CVMX_ADD_IO_SEG(0x0001180014020068ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RX_LNEX_CFG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 15))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 15)))))
		cvmx_warn("CVMX_ILK_RX_LNEX_CFG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014038000ull) + ((offset) & 15) * 1024;
}
#else
#define CVMX_ILK_RX_LNEX_CFG(offset) (CVMX_ADD_IO_SEG(0x0001180014038000ull) + ((offset) & 15) * 1024)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RX_LNEX_INT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 15))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 15)))))
		cvmx_warn("CVMX_ILK_RX_LNEX_INT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014038008ull) + ((offset) & 15) * 1024;
}
#else
#define CVMX_ILK_RX_LNEX_INT(offset) (CVMX_ADD_IO_SEG(0x0001180014038008ull) + ((offset) & 15) * 1024)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RX_LNEX_INT_EN(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 7)))))
		cvmx_warn("CVMX_ILK_RX_LNEX_INT_EN(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014038010ull) + ((offset) & 7) * 1024;
}
#else
#define CVMX_ILK_RX_LNEX_INT_EN(offset) (CVMX_ADD_IO_SEG(0x0001180014038010ull) + ((offset) & 7) * 1024)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RX_LNEX_STAT0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 15))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 15)))))
		cvmx_warn("CVMX_ILK_RX_LNEX_STAT0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014038018ull) + ((offset) & 15) * 1024;
}
#else
#define CVMX_ILK_RX_LNEX_STAT0(offset) (CVMX_ADD_IO_SEG(0x0001180014038018ull) + ((offset) & 15) * 1024)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RX_LNEX_STAT1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 15))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 15)))))
		cvmx_warn("CVMX_ILK_RX_LNEX_STAT1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014038020ull) + ((offset) & 15) * 1024;
}
#else
#define CVMX_ILK_RX_LNEX_STAT1(offset) (CVMX_ADD_IO_SEG(0x0001180014038020ull) + ((offset) & 15) * 1024)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RX_LNEX_STAT10(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 15))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 15)))))
		cvmx_warn("CVMX_ILK_RX_LNEX_STAT10(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014038068ull) + ((offset) & 15) * 1024;
}
#else
#define CVMX_ILK_RX_LNEX_STAT10(offset) (CVMX_ADD_IO_SEG(0x0001180014038068ull) + ((offset) & 15) * 1024)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RX_LNEX_STAT2(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 15))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 15)))))
		cvmx_warn("CVMX_ILK_RX_LNEX_STAT2(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014038028ull) + ((offset) & 15) * 1024;
}
#else
#define CVMX_ILK_RX_LNEX_STAT2(offset) (CVMX_ADD_IO_SEG(0x0001180014038028ull) + ((offset) & 15) * 1024)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RX_LNEX_STAT3(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 15))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 15)))))
		cvmx_warn("CVMX_ILK_RX_LNEX_STAT3(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014038030ull) + ((offset) & 15) * 1024;
}
#else
#define CVMX_ILK_RX_LNEX_STAT3(offset) (CVMX_ADD_IO_SEG(0x0001180014038030ull) + ((offset) & 15) * 1024)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RX_LNEX_STAT4(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 15))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 15)))))
		cvmx_warn("CVMX_ILK_RX_LNEX_STAT4(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014038038ull) + ((offset) & 15) * 1024;
}
#else
#define CVMX_ILK_RX_LNEX_STAT4(offset) (CVMX_ADD_IO_SEG(0x0001180014038038ull) + ((offset) & 15) * 1024)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RX_LNEX_STAT5(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 15))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 15)))))
		cvmx_warn("CVMX_ILK_RX_LNEX_STAT5(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014038040ull) + ((offset) & 15) * 1024;
}
#else
#define CVMX_ILK_RX_LNEX_STAT5(offset) (CVMX_ADD_IO_SEG(0x0001180014038040ull) + ((offset) & 15) * 1024)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RX_LNEX_STAT6(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 15))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 15)))))
		cvmx_warn("CVMX_ILK_RX_LNEX_STAT6(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014038048ull) + ((offset) & 15) * 1024;
}
#else
#define CVMX_ILK_RX_LNEX_STAT6(offset) (CVMX_ADD_IO_SEG(0x0001180014038048ull) + ((offset) & 15) * 1024)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RX_LNEX_STAT7(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 15))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 15)))))
		cvmx_warn("CVMX_ILK_RX_LNEX_STAT7(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014038050ull) + ((offset) & 15) * 1024;
}
#else
#define CVMX_ILK_RX_LNEX_STAT7(offset) (CVMX_ADD_IO_SEG(0x0001180014038050ull) + ((offset) & 15) * 1024)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RX_LNEX_STAT8(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 15))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 15)))))
		cvmx_warn("CVMX_ILK_RX_LNEX_STAT8(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014038058ull) + ((offset) & 15) * 1024;
}
#else
#define CVMX_ILK_RX_LNEX_STAT8(offset) (CVMX_ADD_IO_SEG(0x0001180014038058ull) + ((offset) & 15) * 1024)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_RX_LNEX_STAT9(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 7))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 15))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 15)))))
		cvmx_warn("CVMX_ILK_RX_LNEX_STAT9(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014038060ull) + ((offset) & 15) * 1024;
}
#else
#define CVMX_ILK_RX_LNEX_STAT9(offset) (CVMX_ADD_IO_SEG(0x0001180014038060ull) + ((offset) & 15) * 1024)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_ILK_SER_CFG CVMX_ILK_SER_CFG_FUNC()
static inline uint64_t CVMX_ILK_SER_CFG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_ILK_SER_CFG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180014000018ull);
}
#else
#define CVMX_ILK_SER_CFG (CVMX_ADD_IO_SEG(0x0001180014000018ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_TXX_BYTE_CNTX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 255)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 255)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_ILK_TXX_BYTE_CNTX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180014013000ull) + (((offset) & 255) + ((block_id) & 1) * 0x800ull) * 8;
}
#else
#define CVMX_ILK_TXX_BYTE_CNTX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180014013000ull) + (((offset) & 255) + ((block_id) & 1) * 0x800ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_TXX_CAL_ENTRYX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 287)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 287)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_ILK_TXX_CAL_ENTRYX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180014011000ull) + (((offset) & 511) + ((block_id) & 1) * 0x800ull) * 8;
}
#else
#define CVMX_ILK_TXX_CAL_ENTRYX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180014011000ull) + (((offset) & 511) + ((block_id) & 1) * 0x800ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_TXX_CFG0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_TXX_CFG0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014010000ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_TXX_CFG0(offset) (CVMX_ADD_IO_SEG(0x0001180014010000ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_TXX_CFG1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_TXX_CFG1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014010008ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_TXX_CFG1(offset) (CVMX_ADD_IO_SEG(0x0001180014010008ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_TXX_CHA_XONX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 3)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_ILK_TXX_CHA_XONX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180014010400ull) + (((offset) & 3) + ((block_id) & 1) * 0x800ull) * 8;
}
#else
#define CVMX_ILK_TXX_CHA_XONX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180014010400ull) + (((offset) & 3) + ((block_id) & 1) * 0x800ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_TXX_DBG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_TXX_DBG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014010070ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_TXX_DBG(offset) (CVMX_ADD_IO_SEG(0x0001180014010070ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_TXX_ERR_CFG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_TXX_ERR_CFG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800140100B0ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_TXX_ERR_CFG(offset) (CVMX_ADD_IO_SEG(0x00011800140100B0ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_TXX_FLOW_CTL0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_TXX_FLOW_CTL0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014010048ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_TXX_FLOW_CTL0(offset) (CVMX_ADD_IO_SEG(0x0001180014010048ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_TXX_FLOW_CTL1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_TXX_FLOW_CTL1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014010050ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_TXX_FLOW_CTL1(offset) (CVMX_ADD_IO_SEG(0x0001180014010050ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_TXX_IDX_CAL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_TXX_IDX_CAL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014010058ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_TXX_IDX_CAL(offset) (CVMX_ADD_IO_SEG(0x0001180014010058ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_TXX_IDX_PMAP(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_TXX_IDX_PMAP(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014010010ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_TXX_IDX_PMAP(offset) (CVMX_ADD_IO_SEG(0x0001180014010010ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_TXX_IDX_STAT0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_TXX_IDX_STAT0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014010020ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_TXX_IDX_STAT0(offset) (CVMX_ADD_IO_SEG(0x0001180014010020ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_TXX_IDX_STAT1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_TXX_IDX_STAT1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014010028ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_TXX_IDX_STAT1(offset) (CVMX_ADD_IO_SEG(0x0001180014010028ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_TXX_INT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_TXX_INT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014010078ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_TXX_INT(offset) (CVMX_ADD_IO_SEG(0x0001180014010078ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_TXX_INT_EN(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_TXX_INT_EN(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014010080ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_TXX_INT_EN(offset) (CVMX_ADD_IO_SEG(0x0001180014010080ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_TXX_MEM_CAL0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_TXX_MEM_CAL0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014010060ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_TXX_MEM_CAL0(offset) (CVMX_ADD_IO_SEG(0x0001180014010060ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_TXX_MEM_CAL1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_TXX_MEM_CAL1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014010068ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_TXX_MEM_CAL1(offset) (CVMX_ADD_IO_SEG(0x0001180014010068ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_TXX_MEM_PMAP(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_TXX_MEM_PMAP(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014010018ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_TXX_MEM_PMAP(offset) (CVMX_ADD_IO_SEG(0x0001180014010018ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_TXX_MEM_STAT0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_TXX_MEM_STAT0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014010030ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_TXX_MEM_STAT0(offset) (CVMX_ADD_IO_SEG(0x0001180014010030ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_TXX_MEM_STAT1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_TXX_MEM_STAT1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014010038ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_TXX_MEM_STAT1(offset) (CVMX_ADD_IO_SEG(0x0001180014010038ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_TXX_PIPE(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_TXX_PIPE(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014010088ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_TXX_PIPE(offset) (CVMX_ADD_IO_SEG(0x0001180014010088ull) + ((offset) & 1) * 16384)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_TXX_PKT_CNTX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 255)) && ((block_id <= 1)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 255)) && ((block_id <= 1))))))
		cvmx_warn("CVMX_ILK_TXX_PKT_CNTX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180014012000ull) + (((offset) & 255) + ((block_id) & 1) * 0x800ull) * 8;
}
#else
#define CVMX_ILK_TXX_PKT_CNTX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180014012000ull) + (((offset) & 255) + ((block_id) & 1) * 0x800ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_ILK_TXX_RMATCH(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_ILK_TXX_RMATCH(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180014010040ull) + ((offset) & 1) * 16384;
}
#else
#define CVMX_ILK_TXX_RMATCH(offset) (CVMX_ADD_IO_SEG(0x0001180014010040ull) + ((offset) & 1) * 16384)
#endif

/**
 * cvmx_ilk_bist_sum
 */
union cvmx_ilk_bist_sum {
	uint64_t u64;
	struct cvmx_ilk_bist_sum_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rxf_x2p                      : 1;  /**< BIST result of the X2P memory 0 (rxf.x2p_fif_mem). */
	uint64_t rxf_mem19                    : 1;  /**< BIST result of the RX FIFO bank3 memory 4 (rxf.rx_fif_bnk3_mem4). */
	uint64_t rxf_mem18                    : 1;  /**< BIST result of the RX FIFO bank3 memory 3 (rxf.rx_fif_bnk3_mem3. */
	uint64_t rxf_mem17                    : 1;  /**< BIST result of the RX FIFO bank3 memory 2 (rxf.rx_fif_bnk3_mem2). */
	uint64_t rxf_mem16                    : 1;  /**< BIST result of the RX FIFO bank3 memory 1 (rxf.rx_fif_bnk3_mem1). */
	uint64_t rxf_mem15                    : 1;  /**< BIST result of the RX FIFO bank3 memory 0 (rxf.rx_fif_bnk3_mem0). */
	uint64_t reserved_52_57               : 6;
	uint64_t rxf_mem8                     : 1;  /**< BIST result of the RX FIFO bank1 memory 3 (rxf.rx_fif_bnk1_mem3). */
	uint64_t rxf_mem7                     : 1;  /**< BIST result of the RX FIFO bank1 memory 2 (rxf.rx_fif_bnk1_mem2). */
	uint64_t rxf_mem6                     : 1;  /**< BIST result of the RX FIFO bank1 memory 1 (rxf.rx_fif_bnk1_mem1). */
	uint64_t rxf_mem5                     : 1;  /**< BIST result of the RX FIFO bank1 memory 0 (rxf.rx_fif_bnk1_mem0). */
	uint64_t rxf_mem4                     : 1;  /**< BIST result of the RX FIFO bank0 memory 4 (rxf.rx_fif_bnk0_mem4). */
	uint64_t rxf_mem3                     : 1;  /**< BIST result of the RX FIFO bank0 memory 3 (rxf.rx_fif_bnk0_mem3. */
	uint64_t reserved_36_45               : 10;
	uint64_t rle7_dsk1                    : 1;  /**< Reserved. */
	uint64_t rle7_dsk0                    : 1;  /**< Reserved. */
	uint64_t rle6_dsk1                    : 1;  /**< Reserved. */
	uint64_t rle6_dsk0                    : 1;  /**< Reserved. */
	uint64_t rle5_dsk1                    : 1;  /**< Reserved. */
	uint64_t rle5_dsk0                    : 1;  /**< Reserved. */
	uint64_t rle4_dsk1                    : 1;  /**< Reserved. */
	uint64_t rle4_dsk0                    : 1;  /**< Reserved. */
	uint64_t rle3_dsk1                    : 1;  /**< Reserved. */
	uint64_t rle3_dsk0                    : 1;  /**< Reserved. */
	uint64_t rle2_dsk1                    : 1;  /**< Reserved. */
	uint64_t rle2_dsk0                    : 1;  /**< Reserved. */
	uint64_t rle1_dsk1                    : 1;  /**< Reserved. */
	uint64_t rle1_dsk0                    : 1;  /**< Reserved. */
	uint64_t rle0_dsk1                    : 1;  /**< Reserved. */
	uint64_t rle0_dsk0                    : 1;  /**< Reserved. */
	uint64_t rlk1_pmap                    : 1;  /**< BIST result of the RX link1 port-kind map (rlk1.pmap.pmap_mem_fif). */
	uint64_t reserved_18_18               : 1;
	uint64_t rlk1_fwc                     : 1;  /**< BIST result of the RX link1 calendar table memory (rlk1.fwc.cal_mem). */
	uint64_t reserved_16_16               : 1;
	uint64_t rlk0_pmap                    : 1;  /**< BIST result of the RX link0 port-kind map (rlk0.pmap.pmap_mem_fif). */
	uint64_t rlk0_stat1                   : 1;  /**< BIST result of the RX link0 byte count memory (rlk0.csr.byte_cnt_mem). */
	uint64_t rlk0_fwc                     : 1;  /**< BIST result of the RX link0 calendar table memory (rlk0.fwc.cal_mem). */
	uint64_t rlk0_stat                    : 1;  /**< BIST result of the RX link0 packet count memory (rlk0.csr.pkt_cnt_mem). */
	uint64_t tlk1_stat1                   : 1;  /**< BIST result of the TX link1 byte count memory (tlk1.csr.byte_cnt_mem). */
	uint64_t tlk1_fwc                     : 1;  /**< BIST result of the TX link1 calendar table memory (tlk1.fwc.cal_mem). */
	uint64_t reserved_9_9                 : 1;
	uint64_t tlk1_txf2                    : 1;  /**< Reserved. */
	uint64_t tlk1_txf1                    : 1;  /**< Reserved. */
	uint64_t tlk1_txf0                    : 1;  /**< BIST result of the TX link1 FIFO memory (tlk1.txf.txf_mem_fif). */
	uint64_t tlk0_stat1                   : 1;  /**< BIST result of the TX link0 byte count memory (tlk0.csr.byte_cnt_mem). */
	uint64_t tlk0_fwc                     : 1;  /**< BIST result of the TX link0 calendar table memory (tlk0.fwc.cal_mem). */
	uint64_t reserved_3_3                 : 1;
	uint64_t tlk0_txf2                    : 1;  /**< Reserved. */
	uint64_t tlk0_txf1                    : 1;  /**< Reserved. */
	uint64_t tlk0_txf0                    : 1;  /**< BIST result of the TX link0 FIFO memory (tlk0.txf.txf_mem_fif). */
#else
	uint64_t tlk0_txf0                    : 1;
	uint64_t tlk0_txf1                    : 1;
	uint64_t tlk0_txf2                    : 1;
	uint64_t reserved_3_3                 : 1;
	uint64_t tlk0_fwc                     : 1;
	uint64_t tlk0_stat1                   : 1;
	uint64_t tlk1_txf0                    : 1;
	uint64_t tlk1_txf1                    : 1;
	uint64_t tlk1_txf2                    : 1;
	uint64_t reserved_9_9                 : 1;
	uint64_t tlk1_fwc                     : 1;
	uint64_t tlk1_stat1                   : 1;
	uint64_t rlk0_stat                    : 1;
	uint64_t rlk0_fwc                     : 1;
	uint64_t rlk0_stat1                   : 1;
	uint64_t rlk0_pmap                    : 1;
	uint64_t reserved_16_16               : 1;
	uint64_t rlk1_fwc                     : 1;
	uint64_t reserved_18_18               : 1;
	uint64_t rlk1_pmap                    : 1;
	uint64_t rle0_dsk0                    : 1;
	uint64_t rle0_dsk1                    : 1;
	uint64_t rle1_dsk0                    : 1;
	uint64_t rle1_dsk1                    : 1;
	uint64_t rle2_dsk0                    : 1;
	uint64_t rle2_dsk1                    : 1;
	uint64_t rle3_dsk0                    : 1;
	uint64_t rle3_dsk1                    : 1;
	uint64_t rle4_dsk0                    : 1;
	uint64_t rle4_dsk1                    : 1;
	uint64_t rle5_dsk0                    : 1;
	uint64_t rle5_dsk1                    : 1;
	uint64_t rle6_dsk0                    : 1;
	uint64_t rle6_dsk1                    : 1;
	uint64_t rle7_dsk0                    : 1;
	uint64_t rle7_dsk1                    : 1;
	uint64_t reserved_36_45               : 10;
	uint64_t rxf_mem3                     : 1;
	uint64_t rxf_mem4                     : 1;
	uint64_t rxf_mem5                     : 1;
	uint64_t rxf_mem6                     : 1;
	uint64_t rxf_mem7                     : 1;
	uint64_t rxf_mem8                     : 1;
	uint64_t reserved_52_57               : 6;
	uint64_t rxf_mem15                    : 1;
	uint64_t rxf_mem16                    : 1;
	uint64_t rxf_mem17                    : 1;
	uint64_t rxf_mem18                    : 1;
	uint64_t rxf_mem19                    : 1;
	uint64_t rxf_x2p                      : 1;
#endif
	} s;
	struct cvmx_ilk_bist_sum_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_58_63               : 6;
	uint64_t rxf_x2p1                     : 1;  /**< Bist status of rxf.x2p_fif_mem1 */
	uint64_t rxf_x2p0                     : 1;  /**< Bist status of rxf.x2p_fif_mem0 */
	uint64_t rxf_pmap                     : 1;  /**< Bist status of rxf.rx_map_mem */
	uint64_t rxf_mem2                     : 1;  /**< Bist status of rxf.rx_fif_mem2 */
	uint64_t rxf_mem1                     : 1;  /**< Bist status of rxf.rx_fif_mem1 */
	uint64_t rxf_mem0                     : 1;  /**< Bist status of rxf.rx_fif_mem0 */
	uint64_t reserved_36_51               : 16;
	uint64_t rle7_dsk1                    : 1;  /**< Bist status of lne.rle7.dsk.dsk_fif_mem1 */
	uint64_t rle7_dsk0                    : 1;  /**< Bist status of lne.rle7.dsk.dsk_fif_mem0 */
	uint64_t rle6_dsk1                    : 1;  /**< Bist status of lne.rle6.dsk.dsk_fif_mem1 */
	uint64_t rle6_dsk0                    : 1;  /**< Bist status of lne.rle6.dsk.dsk_fif_mem0 */
	uint64_t rle5_dsk1                    : 1;  /**< Bist status of lne.rle5.dsk.dsk_fif_mem1 */
	uint64_t rle5_dsk0                    : 1;  /**< Bist status of lne.rle5.dsk.dsk_fif_mem0 */
	uint64_t rle4_dsk1                    : 1;  /**< Bist status of lne.rle4.dsk.dsk_fif_mem1 */
	uint64_t rle4_dsk0                    : 1;  /**< Bist status of lne.rle4.dsk.dsk_fif_mem0 */
	uint64_t rle3_dsk1                    : 1;  /**< Bist status of lne.rle3.dsk.dsk_fif_mem1 */
	uint64_t rle3_dsk0                    : 1;  /**< Bist status of lne.rle3.dsk.dsk_fif_mem0 */
	uint64_t rle2_dsk1                    : 1;  /**< Bist status of lne.rle2.dsk.dsk_fif_mem1 */
	uint64_t rle2_dsk0                    : 1;  /**< Bist status of lne.rle2.dsk.dsk_fif_mem0 */
	uint64_t rle1_dsk1                    : 1;  /**< Bist status of lne.rle1.dsk.dsk_fif_mem1 */
	uint64_t rle1_dsk0                    : 1;  /**< Bist status of lne.rle1.dsk.dsk_fif_mem0 */
	uint64_t rle0_dsk1                    : 1;  /**< Bist status of lne.rle0.dsk.dsk_fif_mem1 */
	uint64_t rle0_dsk0                    : 1;  /**< Bist status of lne.rle0.dsk.dsk_fif_mem0 */
	uint64_t reserved_19_19               : 1;
	uint64_t rlk1_stat1                   : 1;  /**< Bist status of rlk1.csr.stat_mem1    ***NOTE: Added in pass 2.0 */
	uint64_t rlk1_fwc                     : 1;  /**< Bist status of rlk1.fwc.cal_chan_ram */
	uint64_t rlk1_stat                    : 1;  /**< Bist status of rlk1.csr.stat_mem0 */
	uint64_t reserved_15_15               : 1;
	uint64_t rlk0_stat1                   : 1;  /**< Bist status of rlk0.csr.stat_mem1    ***NOTE: Added in pass 2.0 */
	uint64_t rlk0_fwc                     : 1;  /**< Bist status of rlk0.fwc.cal_chan_ram */
	uint64_t rlk0_stat                    : 1;  /**< Bist status of rlk0.csr.stat_mem0 */
	uint64_t tlk1_stat1                   : 1;  /**< Bist status of tlk1.csr.stat_mem1 */
	uint64_t tlk1_fwc                     : 1;  /**< Bist status of tlk1.fwc.cal_chan_ram */
	uint64_t tlk1_stat0                   : 1;  /**< Bist status of tlk1.csr.stat_mem0 */
	uint64_t tlk1_txf2                    : 1;  /**< Bist status of tlk1.txf.tx_map_mem */
	uint64_t tlk1_txf1                    : 1;  /**< Bist status of tlk1.txf.tx_fif_mem1 */
	uint64_t tlk1_txf0                    : 1;  /**< Bist status of tlk1.txf.tx_fif_mem0 */
	uint64_t tlk0_stat1                   : 1;  /**< Bist status of tlk0.csr.stat_mem1 */
	uint64_t tlk0_fwc                     : 1;  /**< Bist status of tlk0.fwc.cal_chan_ram */
	uint64_t tlk0_stat0                   : 1;  /**< Bist status of tlk0.csr.stat_mem0 */
	uint64_t tlk0_txf2                    : 1;  /**< Bist status of tlk0.txf.tx_map_mem */
	uint64_t tlk0_txf1                    : 1;  /**< Bist status of tlk0.txf.tx_fif_mem1 */
	uint64_t tlk0_txf0                    : 1;  /**< Bist status of tlk0.txf.tx_fif_mem0 */
#else
	uint64_t tlk0_txf0                    : 1;
	uint64_t tlk0_txf1                    : 1;
	uint64_t tlk0_txf2                    : 1;
	uint64_t tlk0_stat0                   : 1;
	uint64_t tlk0_fwc                     : 1;
	uint64_t tlk0_stat1                   : 1;
	uint64_t tlk1_txf0                    : 1;
	uint64_t tlk1_txf1                    : 1;
	uint64_t tlk1_txf2                    : 1;
	uint64_t tlk1_stat0                   : 1;
	uint64_t tlk1_fwc                     : 1;
	uint64_t tlk1_stat1                   : 1;
	uint64_t rlk0_stat                    : 1;
	uint64_t rlk0_fwc                     : 1;
	uint64_t rlk0_stat1                   : 1;
	uint64_t reserved_15_15               : 1;
	uint64_t rlk1_stat                    : 1;
	uint64_t rlk1_fwc                     : 1;
	uint64_t rlk1_stat1                   : 1;
	uint64_t reserved_19_19               : 1;
	uint64_t rle0_dsk0                    : 1;
	uint64_t rle0_dsk1                    : 1;
	uint64_t rle1_dsk0                    : 1;
	uint64_t rle1_dsk1                    : 1;
	uint64_t rle2_dsk0                    : 1;
	uint64_t rle2_dsk1                    : 1;
	uint64_t rle3_dsk0                    : 1;
	uint64_t rle3_dsk1                    : 1;
	uint64_t rle4_dsk0                    : 1;
	uint64_t rle4_dsk1                    : 1;
	uint64_t rle5_dsk0                    : 1;
	uint64_t rle5_dsk1                    : 1;
	uint64_t rle6_dsk0                    : 1;
	uint64_t rle6_dsk1                    : 1;
	uint64_t rle7_dsk0                    : 1;
	uint64_t rle7_dsk1                    : 1;
	uint64_t reserved_36_51               : 16;
	uint64_t rxf_mem0                     : 1;
	uint64_t rxf_mem1                     : 1;
	uint64_t rxf_mem2                     : 1;
	uint64_t rxf_pmap                     : 1;
	uint64_t rxf_x2p0                     : 1;
	uint64_t rxf_x2p1                     : 1;
	uint64_t reserved_58_63               : 6;
#endif
	} cn68xx;
	struct cvmx_ilk_bist_sum_cn68xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_58_63               : 6;
	uint64_t rxf_x2p1                     : 1;  /**< Bist status of rxf.x2p_fif_mem1 */
	uint64_t rxf_x2p0                     : 1;  /**< Bist status of rxf.x2p_fif_mem0 */
	uint64_t rxf_pmap                     : 1;  /**< Bist status of rxf.rx_map_mem */
	uint64_t rxf_mem2                     : 1;  /**< Bist status of rxf.rx_fif_mem2 */
	uint64_t rxf_mem1                     : 1;  /**< Bist status of rxf.rx_fif_mem1 */
	uint64_t rxf_mem0                     : 1;  /**< Bist status of rxf.rx_fif_mem0 */
	uint64_t reserved_36_51               : 16;
	uint64_t rle7_dsk1                    : 1;  /**< Bist status of lne.rle7.dsk.dsk_fif_mem1 */
	uint64_t rle7_dsk0                    : 1;  /**< Bist status of lne.rle7.dsk.dsk_fif_mem0 */
	uint64_t rle6_dsk1                    : 1;  /**< Bist status of lne.rle6.dsk.dsk_fif_mem1 */
	uint64_t rle6_dsk0                    : 1;  /**< Bist status of lne.rle6.dsk.dsk_fif_mem0 */
	uint64_t rle5_dsk1                    : 1;  /**< Bist status of lne.rle5.dsk.dsk_fif_mem1 */
	uint64_t rle5_dsk0                    : 1;  /**< Bist status of lne.rle5.dsk.dsk_fif_mem0 */
	uint64_t rle4_dsk1                    : 1;  /**< Bist status of lne.rle4.dsk.dsk_fif_mem1 */
	uint64_t rle4_dsk0                    : 1;  /**< Bist status of lne.rle4.dsk.dsk_fif_mem0 */
	uint64_t rle3_dsk1                    : 1;  /**< Bist status of lne.rle3.dsk.dsk_fif_mem1 */
	uint64_t rle3_dsk0                    : 1;  /**< Bist status of lne.rle3.dsk.dsk_fif_mem0 */
	uint64_t rle2_dsk1                    : 1;  /**< Bist status of lne.rle2.dsk.dsk_fif_mem1 */
	uint64_t rle2_dsk0                    : 1;  /**< Bist status of lne.rle2.dsk.dsk_fif_mem0 */
	uint64_t rle1_dsk1                    : 1;  /**< Bist status of lne.rle1.dsk.dsk_fif_mem1 */
	uint64_t rle1_dsk0                    : 1;  /**< Bist status of lne.rle1.dsk.dsk_fif_mem0 */
	uint64_t rle0_dsk1                    : 1;  /**< Bist status of lne.rle0.dsk.dsk_fif_mem1 */
	uint64_t rle0_dsk0                    : 1;  /**< Bist status of lne.rle0.dsk.dsk_fif_mem0 */
	uint64_t reserved_18_19               : 2;
	uint64_t rlk1_fwc                     : 1;  /**< Bist status of rlk1.fwc.cal_chan_ram */
	uint64_t rlk1_stat                    : 1;  /**< Bist status of rlk1.csr.stat_mem */
	uint64_t reserved_14_15               : 2;
	uint64_t rlk0_fwc                     : 1;  /**< Bist status of rlk0.fwc.cal_chan_ram */
	uint64_t rlk0_stat                    : 1;  /**< Bist status of rlk0.csr.stat_mem */
	uint64_t reserved_11_11               : 1;
	uint64_t tlk1_fwc                     : 1;  /**< Bist status of tlk1.fwc.cal_chan_ram */
	uint64_t tlk1_stat                    : 1;  /**< Bist status of tlk1.csr.stat_mem */
	uint64_t tlk1_txf2                    : 1;  /**< Bist status of tlk1.txf.tx_map_mem */
	uint64_t tlk1_txf1                    : 1;  /**< Bist status of tlk1.txf.tx_fif_mem1 */
	uint64_t tlk1_txf0                    : 1;  /**< Bist status of tlk1.txf.tx_fif_mem0 */
	uint64_t reserved_5_5                 : 1;
	uint64_t tlk0_fwc                     : 1;  /**< Bist status of tlk0.fwc.cal_chan_ram */
	uint64_t tlk0_stat                    : 1;  /**< Bist status of tlk0.csr.stat_mem */
	uint64_t tlk0_txf2                    : 1;  /**< Bist status of tlk0.txf.tx_map_mem */
	uint64_t tlk0_txf1                    : 1;  /**< Bist status of tlk0.txf.tx_fif_mem1 */
	uint64_t tlk0_txf0                    : 1;  /**< Bist status of tlk0.txf.tx_fif_mem0 */
#else
	uint64_t tlk0_txf0                    : 1;
	uint64_t tlk0_txf1                    : 1;
	uint64_t tlk0_txf2                    : 1;
	uint64_t tlk0_stat                    : 1;
	uint64_t tlk0_fwc                     : 1;
	uint64_t reserved_5_5                 : 1;
	uint64_t tlk1_txf0                    : 1;
	uint64_t tlk1_txf1                    : 1;
	uint64_t tlk1_txf2                    : 1;
	uint64_t tlk1_stat                    : 1;
	uint64_t tlk1_fwc                     : 1;
	uint64_t reserved_11_11               : 1;
	uint64_t rlk0_stat                    : 1;
	uint64_t rlk0_fwc                     : 1;
	uint64_t reserved_14_15               : 2;
	uint64_t rlk1_stat                    : 1;
	uint64_t rlk1_fwc                     : 1;
	uint64_t reserved_18_19               : 2;
	uint64_t rle0_dsk0                    : 1;
	uint64_t rle0_dsk1                    : 1;
	uint64_t rle1_dsk0                    : 1;
	uint64_t rle1_dsk1                    : 1;
	uint64_t rle2_dsk0                    : 1;
	uint64_t rle2_dsk1                    : 1;
	uint64_t rle3_dsk0                    : 1;
	uint64_t rle3_dsk1                    : 1;
	uint64_t rle4_dsk0                    : 1;
	uint64_t rle4_dsk1                    : 1;
	uint64_t rle5_dsk0                    : 1;
	uint64_t rle5_dsk1                    : 1;
	uint64_t rle6_dsk0                    : 1;
	uint64_t rle6_dsk1                    : 1;
	uint64_t rle7_dsk0                    : 1;
	uint64_t rle7_dsk1                    : 1;
	uint64_t reserved_36_51               : 16;
	uint64_t rxf_mem0                     : 1;
	uint64_t rxf_mem1                     : 1;
	uint64_t rxf_mem2                     : 1;
	uint64_t rxf_pmap                     : 1;
	uint64_t rxf_x2p0                     : 1;
	uint64_t rxf_x2p1                     : 1;
	uint64_t reserved_58_63               : 6;
#endif
	} cn68xxp1;
	struct cvmx_ilk_bist_sum_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rxf_x2p                      : 1;  /**< BIST result of the X2P memory 0 (rxf.x2p_fif_mem). */
	uint64_t rxf_mem19                    : 1;  /**< BIST result of the RX FIFO bank3 memory 4 (rxf.rx_fif_bnk3_mem4). */
	uint64_t rxf_mem18                    : 1;  /**< BIST result of the RX FIFO bank3 memory 3 (rxf.rx_fif_bnk3_mem3. */
	uint64_t rxf_mem17                    : 1;  /**< BIST result of the RX FIFO bank3 memory 2 (rxf.rx_fif_bnk3_mem2). */
	uint64_t rxf_mem16                    : 1;  /**< BIST result of the RX FIFO bank3 memory 1 (rxf.rx_fif_bnk3_mem1). */
	uint64_t rxf_mem15                    : 1;  /**< BIST result of the RX FIFO bank3 memory 0 (rxf.rx_fif_bnk3_mem0). */
	uint64_t rxf_mem14                    : 1;  /**< BIST result of the RX FIFO bank2 memory 4 (rxf.rx_fif_bnk2_mem4). */
	uint64_t rxf_mem13                    : 1;  /**< BIST result of the RX FIFO bank2 memory 3 (rxf.rx_fif_bnk2_mem3). */
	uint64_t rxf_mem12                    : 1;  /**< BIST result of the RX FIFO bank2 memory 2 (rxf.rx_fif_bnk2_mem2). */
	uint64_t rxf_mem11                    : 1;  /**< BIST result of the RX FIFO bank2 memory 1 (rxf.rx_fif_bnk2_mem1). */
	uint64_t rxf_mem10                    : 1;  /**< BIST result of the RX FIFO bank2 memory 0 (rxf.rx_fif_bnk2_mem0). */
	uint64_t rxf_mem9                     : 1;  /**< BIST result of the RX FIFO bank1 memory 4 (rxf.rx_fif_bnk1_mem4). */
	uint64_t rxf_mem8                     : 1;  /**< BIST result of the RX FIFO bank1 memory 3 (rxf.rx_fif_bnk1_mem3). */
	uint64_t rxf_mem7                     : 1;  /**< BIST result of the RX FIFO bank1 memory 2 (rxf.rx_fif_bnk1_mem2). */
	uint64_t rxf_mem6                     : 1;  /**< BIST result of the RX FIFO bank1 memory 1 (rxf.rx_fif_bnk1_mem1). */
	uint64_t rxf_mem5                     : 1;  /**< BIST result of the RX FIFO bank1 memory 0 (rxf.rx_fif_bnk1_mem0). */
	uint64_t rxf_mem4                     : 1;  /**< BIST result of the RX FIFO bank0 memory 4 (rxf.rx_fif_bnk0_mem4). */
	uint64_t rxf_mem3                     : 1;  /**< BIST result of the RX FIFO bank0 memory 3 (rxf.rx_fif_bnk0_mem3. */
	uint64_t rxf_mem2                     : 1;  /**< BIST result of the RX FIFO bank0 memory 2 (rxf.rx_fif_bnk0_mem2). */
	uint64_t rxf_mem1                     : 1;  /**< BIST result of the RX FIFO bank0 memory 1 (rxf.rx_fif_bnk0_mem1). */
	uint64_t rxf_mem0                     : 1;  /**< BIST result of the RX FIFO bank0 memory 0 (rxf.rx_fif_bnk0_mem0). */
	uint64_t reserved_36_42               : 7;
	uint64_t rle7_dsk1                    : 1;  /**< Reserved. */
	uint64_t rle7_dsk0                    : 1;  /**< Reserved. */
	uint64_t rle6_dsk1                    : 1;  /**< Reserved. */
	uint64_t rle6_dsk0                    : 1;  /**< Reserved. */
	uint64_t rle5_dsk1                    : 1;  /**< Reserved. */
	uint64_t rle5_dsk0                    : 1;  /**< Reserved. */
	uint64_t rle4_dsk1                    : 1;  /**< Reserved. */
	uint64_t rle4_dsk0                    : 1;  /**< Reserved. */
	uint64_t rle3_dsk1                    : 1;  /**< Reserved. */
	uint64_t rle3_dsk0                    : 1;  /**< Reserved. */
	uint64_t rle2_dsk1                    : 1;  /**< Reserved. */
	uint64_t rle2_dsk0                    : 1;  /**< Reserved. */
	uint64_t rle1_dsk1                    : 1;  /**< Reserved. */
	uint64_t rle1_dsk0                    : 1;  /**< Reserved. */
	uint64_t rle0_dsk1                    : 1;  /**< Reserved. */
	uint64_t rle0_dsk0                    : 1;  /**< Reserved. */
	uint64_t rlk1_pmap                    : 1;  /**< BIST result of the RX link1 port-kind map (rlk1.pmap.pmap_mem_fif). */
	uint64_t rlk1_stat                    : 1;  /**< BIST result of the RX link1 packet count memory (rlk1.csr.pkt_cnt_mem). */
	uint64_t rlk1_fwc                     : 1;  /**< BIST result of the RX link1 calendar table memory (rlk1.fwc.cal_mem). */
	uint64_t rlk1_stat1                   : 1;  /**< BIST result of the RX link1 byte count memory (rlk1.csr.byte_cnt_mem). */
	uint64_t rlk0_pmap                    : 1;  /**< BIST result of the RX link0 port-kind map (rlk0.pmap.pmap_mem_fif). */
	uint64_t rlk0_stat1                   : 1;  /**< BIST result of the RX link0 byte count memory (rlk0.csr.byte_cnt_mem). */
	uint64_t rlk0_fwc                     : 1;  /**< BIST result of the RX link0 calendar table memory (rlk0.fwc.cal_mem). */
	uint64_t rlk0_stat                    : 1;  /**< BIST result of the RX link0 packet count memory (rlk0.csr.pkt_cnt_mem). */
	uint64_t tlk1_stat1                   : 1;  /**< BIST result of the TX link1 byte count memory (tlk1.csr.byte_cnt_mem). */
	uint64_t tlk1_fwc                     : 1;  /**< BIST result of the TX link1 calendar table memory (tlk1.fwc.cal_mem). */
	uint64_t tlk1_stat0                   : 1;  /**< BIST result of the TX link1 packet count memory (tlk1.csr.pkt_cnt_mem). */
	uint64_t tlk1_txf2                    : 1;  /**< Reserved. */
	uint64_t tlk1_txf1                    : 1;  /**< Reserved. */
	uint64_t tlk1_txf0                    : 1;  /**< BIST result of the TX link1 FIFO memory (tlk1.txf.txf_mem_fif). */
	uint64_t tlk0_stat1                   : 1;  /**< BIST result of the TX link0 byte count memory (tlk0.csr.byte_cnt_mem). */
	uint64_t tlk0_fwc                     : 1;  /**< BIST result of the TX link0 calendar table memory (tlk0.fwc.cal_mem). */
	uint64_t tlk0_stat0                   : 1;  /**< BIST result of the TX link0 packet count memory (tlk0.csr.pkt_cnt_mem). */
	uint64_t tlk0_txf2                    : 1;  /**< Reserved. */
	uint64_t tlk0_txf1                    : 1;  /**< Reserved. */
	uint64_t tlk0_txf0                    : 1;  /**< BIST result of the TX link0 FIFO memory (tlk0.txf.txf_mem_fif). */
#else
	uint64_t tlk0_txf0                    : 1;
	uint64_t tlk0_txf1                    : 1;
	uint64_t tlk0_txf2                    : 1;
	uint64_t tlk0_stat0                   : 1;
	uint64_t tlk0_fwc                     : 1;
	uint64_t tlk0_stat1                   : 1;
	uint64_t tlk1_txf0                    : 1;
	uint64_t tlk1_txf1                    : 1;
	uint64_t tlk1_txf2                    : 1;
	uint64_t tlk1_stat0                   : 1;
	uint64_t tlk1_fwc                     : 1;
	uint64_t tlk1_stat1                   : 1;
	uint64_t rlk0_stat                    : 1;
	uint64_t rlk0_fwc                     : 1;
	uint64_t rlk0_stat1                   : 1;
	uint64_t rlk0_pmap                    : 1;
	uint64_t rlk1_stat1                   : 1;
	uint64_t rlk1_fwc                     : 1;
	uint64_t rlk1_stat                    : 1;
	uint64_t rlk1_pmap                    : 1;
	uint64_t rle0_dsk0                    : 1;
	uint64_t rle0_dsk1                    : 1;
	uint64_t rle1_dsk0                    : 1;
	uint64_t rle1_dsk1                    : 1;
	uint64_t rle2_dsk0                    : 1;
	uint64_t rle2_dsk1                    : 1;
	uint64_t rle3_dsk0                    : 1;
	uint64_t rle3_dsk1                    : 1;
	uint64_t rle4_dsk0                    : 1;
	uint64_t rle4_dsk1                    : 1;
	uint64_t rle5_dsk0                    : 1;
	uint64_t rle5_dsk1                    : 1;
	uint64_t rle6_dsk0                    : 1;
	uint64_t rle6_dsk1                    : 1;
	uint64_t rle7_dsk0                    : 1;
	uint64_t rle7_dsk1                    : 1;
	uint64_t reserved_36_42               : 7;
	uint64_t rxf_mem0                     : 1;
	uint64_t rxf_mem1                     : 1;
	uint64_t rxf_mem2                     : 1;
	uint64_t rxf_mem3                     : 1;
	uint64_t rxf_mem4                     : 1;
	uint64_t rxf_mem5                     : 1;
	uint64_t rxf_mem6                     : 1;
	uint64_t rxf_mem7                     : 1;
	uint64_t rxf_mem8                     : 1;
	uint64_t rxf_mem9                     : 1;
	uint64_t rxf_mem10                    : 1;
	uint64_t rxf_mem11                    : 1;
	uint64_t rxf_mem12                    : 1;
	uint64_t rxf_mem13                    : 1;
	uint64_t rxf_mem14                    : 1;
	uint64_t rxf_mem15                    : 1;
	uint64_t rxf_mem16                    : 1;
	uint64_t rxf_mem17                    : 1;
	uint64_t rxf_mem18                    : 1;
	uint64_t rxf_mem19                    : 1;
	uint64_t rxf_x2p                      : 1;
#endif
	} cn78xx;
	struct cvmx_ilk_bist_sum_cn78xx       cn78xxp1;
};
typedef union cvmx_ilk_bist_sum cvmx_ilk_bist_sum_t;

/**
 * cvmx_ilk_gbl_cfg
 */
union cvmx_ilk_gbl_cfg {
	uint64_t u64;
	struct cvmx_ilk_gbl_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t rid_rstdis                   : 1;  /**< Disable automatic reassembly-ID error recovery. For diagnostic use only. */
	uint64_t reset                        : 1;  /**< Reset ILK. For diagnostic use only. */
	uint64_t cclk_dis                     : 1;  /**< Disable ILK conditional clocking. For diagnostic use only. */
	uint64_t rxf_xlink                    : 1;  /**< Reserved. */
#else
	uint64_t rxf_xlink                    : 1;
	uint64_t cclk_dis                     : 1;
	uint64_t reset                        : 1;
	uint64_t rid_rstdis                   : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_ilk_gbl_cfg_s             cn68xx;
	struct cvmx_ilk_gbl_cfg_cn68xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t cclk_dis                     : 1;  /**< Disable ILK conditional clocking.   For diagnostic use only. */
	uint64_t rxf_xlink                    : 1;  /**< Causes external loopback traffic to switch links.  Enabling
                                                         this allow simultaneous use of external and internal loopback. */
#else
	uint64_t rxf_xlink                    : 1;
	uint64_t cclk_dis                     : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} cn68xxp1;
	struct cvmx_ilk_gbl_cfg_s             cn78xx;
	struct cvmx_ilk_gbl_cfg_s             cn78xxp1;
};
typedef union cvmx_ilk_gbl_cfg cvmx_ilk_gbl_cfg_t;

/**
 * cvmx_ilk_gbl_err_cfg
 */
union cvmx_ilk_gbl_err_cfg {
	uint64_t u64;
	struct cvmx_ilk_gbl_err_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t rxf_flip                     : 2;  /**< Testing feature. Flip syndrome bits <1:0> on writes to the RXF RAM to test single-bit or
                                                         double-bit errors. */
	uint64_t x2p_flip                     : 2;  /**< Testing feature. Flip syndrome bits <1:0> on writes to the X2P RAM to test single-bit or
                                                         double-bit errors. */
	uint64_t reserved_2_15                : 14;
	uint64_t rxf_cor_dis                  : 1;  /**< Disable ECC corrector on RXF. */
	uint64_t x2p_cor_dis                  : 1;  /**< Disable ECC corrector on X2P. */
#else
	uint64_t x2p_cor_dis                  : 1;
	uint64_t rxf_cor_dis                  : 1;
	uint64_t reserved_2_15                : 14;
	uint64_t x2p_flip                     : 2;
	uint64_t rxf_flip                     : 2;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_ilk_gbl_err_cfg_s         cn78xx;
	struct cvmx_ilk_gbl_err_cfg_s         cn78xxp1;
};
typedef union cvmx_ilk_gbl_err_cfg cvmx_ilk_gbl_err_cfg_t;

/**
 * cvmx_ilk_gbl_int
 */
union cvmx_ilk_gbl_int {
	uint64_t u64;
	struct cvmx_ilk_gbl_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t x2p_dbe                      : 1;  /**< X2P double-bit error. Throws ILK_INTSN_E::ILK_GBL_X2P_DBE. */
	uint64_t x2p_sbe                      : 1;  /**< X2P single-bit error. Throws ILK_INTSN_E::ILK_GBL_X2P_SBE. */
	uint64_t rxf_dbe                      : 1;  /**< RXF double-bit error. Throws ILK_INTSN_E::ILK_GBL_RXF_DBE. */
	uint64_t rxf_sbe                      : 1;  /**< RXF single-bit error. Throws ILK_INTSN_E::ILK_GBL_RXF_SBE. */
	uint64_t rxf_push_full                : 1;  /**< RXF overflow. Throws ILK_INTSN_E::ILK_GBL_RXF_PUSH_FULL. */
	uint64_t rxf_pop_empty                : 1;  /**< RXF underflow. Throws ILK_INTSN_E::ILK_GBL_RXF_POP_EMPTY. */
	uint64_t rxf_ctl_perr                 : 1;  /**< Reserved. */
	uint64_t rxf_lnk1_perr                : 1;  /**< Reserved. */
	uint64_t rxf_lnk0_perr                : 1;  /**< Reserved. */
#else
	uint64_t rxf_lnk0_perr                : 1;
	uint64_t rxf_lnk1_perr                : 1;
	uint64_t rxf_ctl_perr                 : 1;
	uint64_t rxf_pop_empty                : 1;
	uint64_t rxf_push_full                : 1;
	uint64_t rxf_sbe                      : 1;
	uint64_t rxf_dbe                      : 1;
	uint64_t x2p_sbe                      : 1;
	uint64_t x2p_dbe                      : 1;
	uint64_t reserved_9_63                : 55;
#endif
	} s;
	struct cvmx_ilk_gbl_int_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t rxf_push_full                : 1;  /**< RXF overflow */
	uint64_t rxf_pop_empty                : 1;  /**< RXF underflow */
	uint64_t rxf_ctl_perr                 : 1;  /**< RXF parity error occurred on sideband control signals.  Data
                                                         cycle will be dropped. */
	uint64_t rxf_lnk1_perr                : 1;  /**< RXF parity error occurred on RxLink1 packet data
                                                         Packet will be marked with error at eop */
	uint64_t rxf_lnk0_perr                : 1;  /**< RXF parity error occurred on RxLink0 packet data.  Packet will
                                                         be marked with error at eop */
#else
	uint64_t rxf_lnk0_perr                : 1;
	uint64_t rxf_lnk1_perr                : 1;
	uint64_t rxf_ctl_perr                 : 1;
	uint64_t rxf_pop_empty                : 1;
	uint64_t rxf_push_full                : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} cn68xx;
	struct cvmx_ilk_gbl_int_cn68xx        cn68xxp1;
	struct cvmx_ilk_gbl_int_s             cn78xx;
	struct cvmx_ilk_gbl_int_s             cn78xxp1;
};
typedef union cvmx_ilk_gbl_int cvmx_ilk_gbl_int_t;

/**
 * cvmx_ilk_gbl_int_en
 */
union cvmx_ilk_gbl_int_en {
	uint64_t u64;
	struct cvmx_ilk_gbl_int_en_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t rxf_push_full                : 1;  /**< RXF overflow */
	uint64_t rxf_pop_empty                : 1;  /**< RXF underflow */
	uint64_t rxf_ctl_perr                 : 1;  /**< RXF parity error occurred on sideband control signals.  Data
                                                         cycle will be dropped. */
	uint64_t rxf_lnk1_perr                : 1;  /**< RXF parity error occurred on RxLink1 packet data
                                                         Packet will be marked with error at eop */
	uint64_t rxf_lnk0_perr                : 1;  /**< RXF parity error occurred on RxLink0 packet data
                                                         Packet will be marked with error at eop */
#else
	uint64_t rxf_lnk0_perr                : 1;
	uint64_t rxf_lnk1_perr                : 1;
	uint64_t rxf_ctl_perr                 : 1;
	uint64_t rxf_pop_empty                : 1;
	uint64_t rxf_push_full                : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_ilk_gbl_int_en_s          cn68xx;
	struct cvmx_ilk_gbl_int_en_s          cn68xxp1;
};
typedef union cvmx_ilk_gbl_int_en cvmx_ilk_gbl_int_en_t;

/**
 * cvmx_ilk_int_sum
 */
union cvmx_ilk_int_sum {
	uint64_t u64;
	struct cvmx_ilk_int_sum_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_13_63               : 51;
	uint64_t rle7_int                     : 1;  /**< RxLane7 interrupt status. See ILK_RX_LNE7_INT */
	uint64_t rle6_int                     : 1;  /**< RxLane6 interrupt status. See ILK_RX_LNE6_INT */
	uint64_t rle5_int                     : 1;  /**< RxLane5 interrupt status. See ILK_RX_LNE5_INT */
	uint64_t rle4_int                     : 1;  /**< RxLane4 interrupt status. See ILK_RX_LNE4_INT */
	uint64_t rle3_int                     : 1;  /**< RxLane3 interrupt status. See ILK_RX_LNE3_INT */
	uint64_t rle2_int                     : 1;  /**< RxLane2 interrupt status. See ILK_RX_LNE2_INT */
	uint64_t rle1_int                     : 1;  /**< RxLane1 interrupt status. See ILK_RX_LNE1_INT */
	uint64_t rle0_int                     : 1;  /**< RxLane0 interrupt status. See ILK_RX_LNE0_INT */
	uint64_t rlk1_int                     : 1;  /**< RxLink1 interrupt status. See ILK_RX1_INT */
	uint64_t rlk0_int                     : 1;  /**< RxLink0 interrupt status. See ILK_RX0_INT */
	uint64_t tlk1_int                     : 1;  /**< TxLink1 interrupt status. See ILK_TX1_INT */
	uint64_t tlk0_int                     : 1;  /**< TxLink0 interrupt status. See ILK_TX0_INT */
	uint64_t gbl_int                      : 1;  /**< Global interrupt status. See ILK_GBL_INT */
#else
	uint64_t gbl_int                      : 1;
	uint64_t tlk0_int                     : 1;
	uint64_t tlk1_int                     : 1;
	uint64_t rlk0_int                     : 1;
	uint64_t rlk1_int                     : 1;
	uint64_t rle0_int                     : 1;
	uint64_t rle1_int                     : 1;
	uint64_t rle2_int                     : 1;
	uint64_t rle3_int                     : 1;
	uint64_t rle4_int                     : 1;
	uint64_t rle5_int                     : 1;
	uint64_t rle6_int                     : 1;
	uint64_t rle7_int                     : 1;
	uint64_t reserved_13_63               : 51;
#endif
	} s;
	struct cvmx_ilk_int_sum_s             cn68xx;
	struct cvmx_ilk_int_sum_s             cn68xxp1;
};
typedef union cvmx_ilk_int_sum cvmx_ilk_int_sum_t;

/**
 * cvmx_ilk_lne#_trn_ctl
 */
union cvmx_ilk_lnex_trn_ctl {
	uint64_t u64;
	struct cvmx_ilk_lnex_trn_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t trn_lock                     : 1;  /**< Link training RX frame lock. */
	uint64_t trn_done                     : 1;  /**< Link training done. */
	uint64_t trn_ena                      : 1;  /**< Link training enable. */
	uint64_t eie_det                      : 1;  /**< Reserved. */
#else
	uint64_t eie_det                      : 1;
	uint64_t trn_ena                      : 1;
	uint64_t trn_done                     : 1;
	uint64_t trn_lock                     : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_ilk_lnex_trn_ctl_s        cn78xx;
	struct cvmx_ilk_lnex_trn_ctl_s        cn78xxp1;
};
typedef union cvmx_ilk_lnex_trn_ctl cvmx_ilk_lnex_trn_ctl_t;

/**
 * cvmx_ilk_lne#_trn_ld
 */
union cvmx_ilk_lnex_trn_ld {
	uint64_t u64;
	struct cvmx_ilk_lnex_trn_ld_s {
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
	struct cvmx_ilk_lnex_trn_ld_s         cn78xx;
	struct cvmx_ilk_lnex_trn_ld_s         cn78xxp1;
};
typedef union cvmx_ilk_lnex_trn_ld cvmx_ilk_lnex_trn_ld_t;

/**
 * cvmx_ilk_lne#_trn_lp
 */
union cvmx_ilk_lnex_trn_lp {
	uint64_t u64;
	struct cvmx_ilk_lnex_trn_lp_s {
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
	struct cvmx_ilk_lnex_trn_lp_s         cn78xx;
	struct cvmx_ilk_lnex_trn_lp_s         cn78xxp1;
};
typedef union cvmx_ilk_lnex_trn_lp cvmx_ilk_lnex_trn_lp_t;

/**
 * cvmx_ilk_lne_dbg
 */
union cvmx_ilk_lne_dbg {
	uint64_t u64;
	struct cvmx_ilk_lne_dbg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t tx_bad_crc32                 : 1;  /**< Send one diagnostic word with bad CRC32 to the selected lane. Note that it injects just once. */
	uint64_t tx_bad_6467_cnt              : 5;  /**< Specifies the number of bad 64/67 bit code words on the selected lane. */
	uint64_t tx_bad_sync_cnt              : 3;  /**< Specifies the number of bad sync words on the selected lane. */
	uint64_t tx_bad_scram_cnt             : 3;  /**< Specifies the number of bad scrambler state on the selected lane. */
	uint64_t tx_bad_lane_sel              : 16; /**< Select the lane to apply the error-injection counts. */
	uint64_t tx_dis_dispr                 : 16; /**< Per-lane disparity disable. For diagnostic use only. */
	uint64_t tx_dis_scram                 : 16; /**< Per-lane scrambler disable. For diagnostic use only. */
#else
	uint64_t tx_dis_scram                 : 16;
	uint64_t tx_dis_dispr                 : 16;
	uint64_t tx_bad_lane_sel              : 16;
	uint64_t tx_bad_scram_cnt             : 3;
	uint64_t tx_bad_sync_cnt              : 3;
	uint64_t tx_bad_6467_cnt              : 5;
	uint64_t tx_bad_crc32                 : 1;
	uint64_t reserved_60_63               : 4;
#endif
	} s;
	struct cvmx_ilk_lne_dbg_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t tx_bad_crc32                 : 1;  /**< Send 1 diagnostic word with bad CRC32 to the selected lane.
                                                         Note: injects just once */
	uint64_t tx_bad_6467_cnt              : 5;  /**< Send N bad 64B/67B codewords on selected lane */
	uint64_t tx_bad_sync_cnt              : 3;  /**< Send N bad sync words on selected lane */
	uint64_t tx_bad_scram_cnt             : 3;  /**< Send N bad scram state on selected lane */
	uint64_t reserved_40_47               : 8;
	uint64_t tx_bad_lane_sel              : 8;  /**< Select lane to apply error injection counts */
	uint64_t reserved_24_31               : 8;
	uint64_t tx_dis_dispr                 : 8;  /**< Per-lane disparity disable */
	uint64_t reserved_8_15                : 8;
	uint64_t tx_dis_scram                 : 8;  /**< Per-lane scrambler disable */
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
	} cn68xx;
	struct cvmx_ilk_lne_dbg_cn68xx        cn68xxp1;
	struct cvmx_ilk_lne_dbg_s             cn78xx;
	struct cvmx_ilk_lne_dbg_s             cn78xxp1;
};
typedef union cvmx_ilk_lne_dbg cvmx_ilk_lne_dbg_t;

/**
 * cvmx_ilk_lne_sts_msg
 */
union cvmx_ilk_lne_sts_msg {
	uint64_t u64;
	struct cvmx_ilk_lne_sts_msg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rx_lnk_stat                  : 16; /**< Link status received in the diagnostic word (per-lane); 1 means healthy (according to the
                                                         Interlaken specification). */
	uint64_t rx_lne_stat                  : 16; /**< Lane status received in the diagnostic word (per-lane); 1 means healthy (according to the
                                                         Interlaken specification). */
	uint64_t tx_lnk_stat                  : 16; /**< Link status transmitted in the diagnostic word (per-lane); 1 means healthy (according to
                                                         the Interlaken specification). */
	uint64_t tx_lne_stat                  : 16; /**< Lane status transmitted in the diagnostic word (per-lane); 1 means healthy (according to
                                                         the Interlaken specification). */
#else
	uint64_t tx_lne_stat                  : 16;
	uint64_t tx_lnk_stat                  : 16;
	uint64_t rx_lne_stat                  : 16;
	uint64_t rx_lnk_stat                  : 16;
#endif
	} s;
	struct cvmx_ilk_lne_sts_msg_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_56_63               : 8;
	uint64_t rx_lnk_stat                  : 8;  /**< Link status received in the diagnostic word (per-lane)
                                                         '1' means healthy (according to the Interlaken spec) */
	uint64_t reserved_40_47               : 8;
	uint64_t rx_lne_stat                  : 8;  /**< Lane status received in the diagnostic word (per-lane)
                                                         '1' means healthy (according to the Interlaken spec) */
	uint64_t reserved_24_31               : 8;
	uint64_t tx_lnk_stat                  : 8;  /**< Link status transmitted in the diagnostic word (per-lane)
                                                         '1' means healthy (according to the Interlaken spec) */
	uint64_t reserved_8_15                : 8;
	uint64_t tx_lne_stat                  : 8;  /**< Lane status transmitted in the diagnostic word (per-lane)
                                                         '1' means healthy (according to the Interlaken spec) */
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
	} cn68xx;
	struct cvmx_ilk_lne_sts_msg_cn68xx    cn68xxp1;
	struct cvmx_ilk_lne_sts_msg_s         cn78xx;
	struct cvmx_ilk_lne_sts_msg_s         cn78xxp1;
};
typedef union cvmx_ilk_lne_sts_msg cvmx_ilk_lne_sts_msg_t;

/**
 * cvmx_ilk_rid_cfg
 */
union cvmx_ilk_rid_cfg {
	uint64_t u64;
	struct cvmx_ilk_rid_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_39_63               : 25;
	uint64_t max_cnt                      : 7;  /**< Maximum number of reassembly IDs (RIDs) allowed for both links. If
                                                         an SOP arrives and the total number of RIDs already allocated to
                                                         both links is at least [MAX_CNT], the packet is dropped.
                                                         An SOP allocates a RID; an EOP frees a RID. ILK_RX()_RID can be used to further
                                                         restrict each link individually. */
	uint64_t reserved_7_31                : 25;
	uint64_t base                         : 7;  /**< The base RID for ILK. There is a shared pool of 96 RIDs for all MACs.
                                                         See PKI_REASM_E. ILK can allocate any RID in the range of
                                                         _ [BASE] -> ([BASE]+([MAX_CNT]-1)).
                                                         BASE and MAX_CNT must be constrained such that:
                                                         _ 1) [BASE] >= 2.
                                                         _ 2) [BASE] + [MAX_CNT] <= 96.
                                                         _ 3) [BASE]..([BASE]+([MAX_CNT]-1)) does not overlap with any other MAC programming.
                                                         The reset value for this CSR has been chosen such that all these conditions are satisfied.
                                                         The reset value supports up to a total of 64 outstanding incomplete packets between ILK0
                                                         and ILK1.
                                                         Changes to BASE must only occur when ILK0 and ILK1 are both quiescent (i.e. Both ILK0 and
                                                         ILK1 receive interfaces are down and the RX fifo is empty). */
#else
	uint64_t base                         : 7;
	uint64_t reserved_7_31                : 25;
	uint64_t max_cnt                      : 7;
	uint64_t reserved_39_63               : 25;
#endif
	} s;
	struct cvmx_ilk_rid_cfg_s             cn78xx;
	struct cvmx_ilk_rid_cfg_s             cn78xxp1;
};
typedef union cvmx_ilk_rid_cfg cvmx_ilk_rid_cfg_t;

/**
 * cvmx_ilk_rx#_byte_cnt#
 */
union cvmx_ilk_rxx_byte_cntx {
	uint64_t u64;
	struct cvmx_ilk_rxx_byte_cntx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_40_63               : 24;
	uint64_t rx_bytes                     : 40; /**< Number of bytes received per channel. Wraps on overflow. On overflow, sets
                                                         ILK_RX()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t rx_bytes                     : 40;
	uint64_t reserved_40_63               : 24;
#endif
	} s;
	struct cvmx_ilk_rxx_byte_cntx_s       cn78xx;
	struct cvmx_ilk_rxx_byte_cntx_s       cn78xxp1;
};
typedef union cvmx_ilk_rxx_byte_cntx cvmx_ilk_rxx_byte_cntx_t;

/**
 * cvmx_ilk_rx#_cal_entry#
 */
union cvmx_ilk_rxx_cal_entryx {
	uint64_t u64;
	struct cvmx_ilk_rxx_cal_entryx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_34_63               : 30;
	uint64_t ctl                          : 2;  /**< Select source of XON/XOFF for entry (IDX * 8) + 0.
                                                         0 = PKO backpressure channel.
                                                         1 = Link.
                                                         2 = XOFF.
                                                         3 = XON.
                                                         This field applies to one of bits <55>, <47>, or <31> in the Interlaken control word. */
	uint64_t reserved_8_31                : 24;
	uint64_t channel                      : 8;  /**< PKO channel for the calendar table entry. Unused if CTL == 0x1. */
#else
	uint64_t channel                      : 8;
	uint64_t reserved_8_31                : 24;
	uint64_t ctl                          : 2;
	uint64_t reserved_34_63               : 30;
#endif
	} s;
	struct cvmx_ilk_rxx_cal_entryx_s      cn78xx;
	struct cvmx_ilk_rxx_cal_entryx_s      cn78xxp1;
};
typedef union cvmx_ilk_rxx_cal_entryx cvmx_ilk_rxx_cal_entryx_t;

/**
 * cvmx_ilk_rx#_cfg0
 */
union cvmx_ilk_rxx_cfg0 {
	uint64_t u64;
	struct cvmx_ilk_rxx_cfg0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ext_lpbk_fc                  : 1;  /**< Enable RX-TX flow-control external loopback. */
	uint64_t ext_lpbk                     : 1;  /**< Enable RX-TX data external loopback. Note that with differing transmit and receive clocks,
                                                         skip word are inserted/deleted. */
	uint64_t reserved_60_61               : 2;
	uint64_t lnk_stats_wrap               : 1;  /**< Upon overflow, a statistics counter should wrap instead of
                                                         saturating. */
	uint64_t bcw_push                     : 1;  /**< Reserved. */
	uint64_t mproto_ign                   : 1;  /**< Reserved. */
	uint64_t ptrn_mode                    : 1;  /**< Reserved. */
	uint64_t lnk_stats_rdclr              : 1;  /**< Enable that a CSR read operation to ILK_RX()_STAT0..ILK_RX()_STAT9 clears the counter
                                                         after returning its current value. */
	uint64_t lnk_stats_ena                : 1;  /**< Enable link-statistics counters. */
	uint64_t mltuse_fc_ena                : 1;  /**< Use multiuse field for calendar. */
	uint64_t cal_ena                      : 1;  /**< Enable the RX calendar. When the calendar table is disabled, all port-pipes receive XON. */
	uint64_t mfrm_len                     : 13; /**< The quantity of data sent on each lane including one sync word, scrambler state,
                                                         diagnostic word, zero or more skip words, and the data payload. Must be large than
                                                         _ ILK_TX()_CFG1[SKIP_CNT] + 32.
                                                         Supported range:
                                                         _ ILK_TX()_CFG1[SKIP_CNT] + 32 < [MFRM_LEN] <= 4096 */
	uint64_t brst_shrt                    : 7;  /**< Minimum interval between burst control words, as a multiple of eight bytes. Supported
                                                         range from 8 to 512 bytes (i.e. 4 <= [BRST_SHRT] <= 64).
                                                         This field affects the ILK_RX()_STAT4[BRST_SHRT_ERR_CNT] counter. It does not affect
                                                         correct operation of the link. */
	uint64_t lane_rev                     : 1;  /**< Lane reversal. When enabled, lane destriping is performed from most-significant lane
                                                         enabled to least-significant lane enabled. [LANE_ENA] must be 0 before changing
                                                         [LANE_REV]. */
	uint64_t brst_max                     : 5;  /**< Maximum size of a data burst, as a multiple of 64-byte blocks. Supported range is from 64
                                                         to 1024 bytes
                                                         (i.e. 0 < [BRST_MAX] <= 16).
                                                         This field affects the ILK_RX()_STAT2[BRST_NOT_FULL_CNT] and
                                                         ILK_RX()_STAT3[BRST_MAX_ERR_CNT] counters. It does not affect correct operation of the
                                                         link. */
	uint64_t reserved_25_25               : 1;
	uint64_t cal_depth                    : 9;  /**< Indicates the number of valid entries in the calendar.   Supported range from 1 to 288. */
	uint64_t lane_ena                     : 16; /**< Lane-enable mask. The link is enabled if any lane is enabled. The same lane should not be
                                                         enabled in multiple ILK_RXn_CFG0. Each bit of [LANE_ENA] maps to an RX lane (RLE) and a
                                                         QLM lane. Note that [LANE_REV] has no effect on this mapping.
                                                         _ [LANE_ENA<0>]  = RLE0  = QLM4 lane 0.
                                                         _ [LANE_ENA<1>]  = RLE1  = QLM4 lane 1.
                                                         _ [LANE_ENA<2>]  = RLE2  = QLM4 lane 2.
                                                         _ [LANE_ENA<3>]  = RLE3  = QLM4 lane 3.
                                                         _ [LANE_ENA<4>]  = RLE4  = QLM5 lane 0.
                                                         _ [LANE_ENA<5>]  = RLE5  = QLM5 lane 1.
                                                         _ [LANE_ENA<6>]  = RLE6  = QLM5 lane 2.
                                                         _ [LANE_ENA<7>]  = RLE7  = QLM5 lane 3.
                                                         _ [LANE_ENA<8>]  = RLE8  = QLM6 lane 0.
                                                         _ [LANE_ENA<9>]  = RLE9  = QLM6 lane 1.
                                                         _ [LANE_ENA<10>] = RLE10 = QLM6 lane 2.
                                                         _ [LANE_ENA<11>] = RLE11 = QLM6 lane 3.
                                                         _ [LANE_ENA<12>] = RLE12 = QLM7 lane 0.
                                                         _ [LANE_ENA<13>] = RLE13 = QLM7 lane 1.
                                                         _ [LANE_ENA<14>] = RLE14 = QLM7 lane 2.
                                                         _ [LANE_ENA<15>] = RLE15 = QLM7 lane 3. */
#else
	uint64_t lane_ena                     : 16;
	uint64_t cal_depth                    : 9;
	uint64_t reserved_25_25               : 1;
	uint64_t brst_max                     : 5;
	uint64_t lane_rev                     : 1;
	uint64_t brst_shrt                    : 7;
	uint64_t mfrm_len                     : 13;
	uint64_t cal_ena                      : 1;
	uint64_t mltuse_fc_ena                : 1;
	uint64_t lnk_stats_ena                : 1;
	uint64_t lnk_stats_rdclr              : 1;
	uint64_t ptrn_mode                    : 1;
	uint64_t mproto_ign                   : 1;
	uint64_t bcw_push                     : 1;
	uint64_t lnk_stats_wrap               : 1;
	uint64_t reserved_60_61               : 2;
	uint64_t ext_lpbk                     : 1;
	uint64_t ext_lpbk_fc                  : 1;
#endif
	} s;
	struct cvmx_ilk_rxx_cfg0_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ext_lpbk_fc                  : 1;  /**< Enable Rx-Tx flowcontrol loopback (external) */
	uint64_t ext_lpbk                     : 1;  /**< Enable Rx-Tx data loopback (external). Note that with differing
                                                         transmit & receive clocks, skip word are  inserted/deleted */
	uint64_t reserved_60_61               : 2;
	uint64_t lnk_stats_wrap               : 1;  /**< Upon overflow, a statistics counter should wrap instead of
                                                         saturating.

                                                         ***NOTE: Added in pass 2.0 */
	uint64_t bcw_push                     : 1;  /**< The 8 byte burst control word containing the SOP will be
                                                         prepended to the corresponding packet.

                                                         ***NOTE: Added in pass 2.0 */
	uint64_t mproto_ign                   : 1;  /**< When LA_MODE=1 and MPROTO_IGN=0, the multi-protocol bit of the
                                                         LA control word is used to determine if the burst is an LA or
                                                         non-LA burst.   When LA_MODE=1 and MPROTO_IGN=1, all bursts
                                                         are treated LA.   When LA_MODE=0, this field is ignored

                                                         ***NOTE: Added in pass 2.0 */
	uint64_t ptrn_mode                    : 1;  /**< Reserved */
	uint64_t lnk_stats_rdclr              : 1;  /**< CSR read to ILK_RXx_STAT* clears the counter after returning
                                                         its current value. */
	uint64_t lnk_stats_ena                : 1;  /**< Enable link statistics counters */
	uint64_t mltuse_fc_ena                : 1;  /**< Use multi-use field for calendar */
	uint64_t cal_ena                      : 1;  /**< Enable Rx calendar.  When the calendar table is disabled, all
                                                         port-pipes receive XON. */
	uint64_t mfrm_len                     : 13; /**< The quantity of data sent on each lane including one sync word,
                                                         scrambler state, diag word, zero or more skip words, and the
                                                         data  payload.  Must be large than ILK_RXX_CFG1[SKIP_CNT]+9.
                                                         Supported range:ILK_RXX_CFG1[SKIP_CNT]+9 < MFRM_LEN <= 4096) */
	uint64_t brst_shrt                    : 7;  /**< Minimum interval between burst control words, as a multiple of
                                                         8 bytes.  Supported range from 8 bytes to 512 (ie. 0 <
                                                         BRST_SHRT <= 64)
                                                         This field affects the ILK_RX*_STAT4[BRST_SHRT_ERR_CNT]
                                                         counter. It does not affect correct operation of the link. */
	uint64_t lane_rev                     : 1;  /**< Lane reversal.   When enabled, lane de-striping is performed
                                                         from most significant lane enabled to least significant lane
                                                         enabled.  LANE_ENA must be zero before changing LANE_REV. */
	uint64_t brst_max                     : 5;  /**< Maximum size of a data burst, as a multiple of 64 byte blocks.
                                                         Supported range is from 64 bytes to  1024 bytes. (ie. 0 <
                                                         BRST_MAX <= 16)
                                                         This field affects the ILK_RX*_STAT2[BRST_NOT_FULL_CNT] and
                                                         ILK_RX*_STAT3[BRST_MAX_ERR_CNT] counters. It does not affect
                                                         correct operation of the link. */
	uint64_t reserved_25_25               : 1;
	uint64_t cal_depth                    : 9;  /**< Number of valid entries in the calendar.   Supported range from
                                                         1 to 288. */
	uint64_t reserved_8_15                : 8;
	uint64_t lane_ena                     : 8;  /**< Lane enable mask.  Link is enabled if any lane is enabled.  The
                                                         same lane should not be enabled in multiple ILK_RXx_CFG0.  Each
                                                         bit of LANE_ENA maps to a RX lane (RLE) and a QLM lane.  NOTE:
                                                         LANE_REV has no effect on this mapping.

                                                               LANE_ENA[0] = RLE0 = QLM1 lane 0
                                                               LANE_ENA[1] = RLE1 = QLM1 lane 1
                                                               LANE_ENA[2] = RLE2 = QLM1 lane 2
                                                               LANE_ENA[3] = RLE3 = QLM1 lane 3
                                                               LANE_ENA[4] = RLE4 = QLM2 lane 0
                                                               LANE_ENA[5] = RLE5 = QLM2 lane 1
                                                               LANE_ENA[6] = RLE6 = QLM2 lane 2
                                                               LANE_ENA[7] = RLE7 = QLM2 lane 3 */
#else
	uint64_t lane_ena                     : 8;
	uint64_t reserved_8_15                : 8;
	uint64_t cal_depth                    : 9;
	uint64_t reserved_25_25               : 1;
	uint64_t brst_max                     : 5;
	uint64_t lane_rev                     : 1;
	uint64_t brst_shrt                    : 7;
	uint64_t mfrm_len                     : 13;
	uint64_t cal_ena                      : 1;
	uint64_t mltuse_fc_ena                : 1;
	uint64_t lnk_stats_ena                : 1;
	uint64_t lnk_stats_rdclr              : 1;
	uint64_t ptrn_mode                    : 1;
	uint64_t mproto_ign                   : 1;
	uint64_t bcw_push                     : 1;
	uint64_t lnk_stats_wrap               : 1;
	uint64_t reserved_60_61               : 2;
	uint64_t ext_lpbk                     : 1;
	uint64_t ext_lpbk_fc                  : 1;
#endif
	} cn68xx;
	struct cvmx_ilk_rxx_cfg0_cn68xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ext_lpbk_fc                  : 1;  /**< Enable Rx-Tx flowcontrol loopback (external) */
	uint64_t ext_lpbk                     : 1;  /**< Enable Rx-Tx data loopback (external). Note that with differing
                                                         transmit & receive clocks, skip word are  inserted/deleted */
	uint64_t reserved_57_61               : 5;
	uint64_t ptrn_mode                    : 1;  /**< Enable programmable test pattern mode */
	uint64_t lnk_stats_rdclr              : 1;  /**< CSR read to ILK_RXx_STAT* clears the counter after returning
                                                         its current value. */
	uint64_t lnk_stats_ena                : 1;  /**< Enable link statistics counters */
	uint64_t mltuse_fc_ena                : 1;  /**< Use multi-use field for calendar */
	uint64_t cal_ena                      : 1;  /**< Enable Rx calendar.  When the calendar table is disabled, all
                                                         port-pipes receive XON. */
	uint64_t mfrm_len                     : 13; /**< The quantity of data sent on each lane including one sync word,
                                                         scrambler state, diag word, zero or more skip words, and the
                                                         data  payload.  Must be large than ILK_RXX_CFG1[SKIP_CNT]+9.
                                                         Supported range:ILK_RXX_CFG1[SKIP_CNT]+9 < MFRM_LEN <= 4096) */
	uint64_t brst_shrt                    : 7;  /**< Minimum interval between burst control words, as a multiple of
                                                         8 bytes.  Supported range from 8 bytes to 512 (ie. 0 <
                                                         BRST_SHRT <= 64)
                                                         This field affects the ILK_RX*_STAT4[BRST_SHRT_ERR_CNT]
                                                         counter. It does not affect correct operation of the link. */
	uint64_t lane_rev                     : 1;  /**< Lane reversal.   When enabled, lane de-striping is performed
                                                         from most significant lane enabled to least significant lane
                                                         enabled.  LANE_ENA must be zero before changing LANE_REV. */
	uint64_t brst_max                     : 5;  /**< Maximum size of a data burst, as a multiple of 64 byte blocks.
                                                         Supported range is from 64 bytes to  1024 bytes. (ie. 0 <
                                                         BRST_MAX <= 16)
                                                         This field affects the ILK_RX*_STAT2[BRST_NOT_FULL_CNT] and
                                                         ILK_RX*_STAT3[BRST_MAX_ERR_CNT] counters. It does not affect
                                                         correct operation of the link. */
	uint64_t reserved_25_25               : 1;
	uint64_t cal_depth                    : 9;  /**< Number of valid entries in the calendar.   Supported range from
                                                         1 to 288. */
	uint64_t reserved_8_15                : 8;
	uint64_t lane_ena                     : 8;  /**< Lane enable mask.  Link is enabled if any lane is enabled.  The
                                                         same lane should not be enabled in multiple ILK_RXx_CFG0.  Each
                                                         bit of LANE_ENA maps to a RX lane (RLE) and a QLM lane.  NOTE:
                                                         LANE_REV has no effect on this mapping.

                                                               LANE_ENA[0] = RLE0 = QLM1 lane 0
                                                               LANE_ENA[1] = RLE1 = QLM1 lane 1
                                                               LANE_ENA[2] = RLE2 = QLM1 lane 2
                                                               LANE_ENA[3] = RLE3 = QLM1 lane 3
                                                               LANE_ENA[4] = RLE4 = QLM2 lane 0
                                                               LANE_ENA[5] = RLE5 = QLM2 lane 1
                                                               LANE_ENA[6] = RLE6 = QLM2 lane 2
                                                               LANE_ENA[7] = RLE7 = QLM2 lane 3 */
#else
	uint64_t lane_ena                     : 8;
	uint64_t reserved_8_15                : 8;
	uint64_t cal_depth                    : 9;
	uint64_t reserved_25_25               : 1;
	uint64_t brst_max                     : 5;
	uint64_t lane_rev                     : 1;
	uint64_t brst_shrt                    : 7;
	uint64_t mfrm_len                     : 13;
	uint64_t cal_ena                      : 1;
	uint64_t mltuse_fc_ena                : 1;
	uint64_t lnk_stats_ena                : 1;
	uint64_t lnk_stats_rdclr              : 1;
	uint64_t ptrn_mode                    : 1;
	uint64_t reserved_57_61               : 5;
	uint64_t ext_lpbk                     : 1;
	uint64_t ext_lpbk_fc                  : 1;
#endif
	} cn68xxp1;
	struct cvmx_ilk_rxx_cfg0_s            cn78xx;
	struct cvmx_ilk_rxx_cfg0_s            cn78xxp1;
};
typedef union cvmx_ilk_rxx_cfg0 cvmx_ilk_rxx_cfg0_t;

/**
 * cvmx_ilk_rx#_cfg1
 */
union cvmx_ilk_rxx_cfg1 {
	uint64_t u64;
	struct cvmx_ilk_rxx_cfg1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_62_63               : 2;
	uint64_t rx_fifo_cnt                  : 12; /**< Number of 64-bit words currently consumed by this link in the RX FIFO. */
	uint64_t reserved_49_49               : 1;
	uint64_t rx_fifo_hwm                  : 13; /**< Number of 64-bit words consumed by this link before switch transmitted link flow-control
                                                         status from XON to XOFF. LSB must be zero. A typical single-link configuration should set
                                                         this to 2048. A typical multi-link configuration should set this to NL*128 where NL is the
                                                         number of lanes enabled for a given link.
                                                         _ XON = [RX_FIFO_CNT] < [RX_FIFO_HWM]
                                                         _ XOFF = [RX_FIFO_CNT] >= ]RX_FIFO_HWM]. */
	uint64_t reserved_35_35               : 1;
	uint64_t rx_fifo_max                  : 13; /**< Specifies the maximum number of 64-bit words consumed by this link in the RX FIFO. The sum
                                                         of all links should be equal to 4096 (32KB). LSB must be zero. Typically set to
                                                         [RX_FIFO_HWM] * 2. */
	uint64_t pkt_flush                    : 1;  /**< Packet receive flush. Setting this bit causes all open packets to be error-out, just as
                                                         though the link went down. */
	uint64_t pkt_ena                      : 1;  /**< Packet receive enable. When set to 0, any received SOP causes the entire packet to be dropped. */
	uint64_t la_mode                      : 1;  /**< Reserved. */
	uint64_t tx_link_fc                   : 1;  /**< Link flow-control status transmitted by the TX-link XON (=1) when [RX_FIFO_CNT] <=
                                                         [RX_FIFO_HWM] and lane alignment is done. */
	uint64_t rx_link_fc                   : 1;  /**< Link flow-control status received in burst/idle control words. XOFF (=0) causes TX-link to
                                                         stop transmitting on all channels. */
	uint64_t rx_align_ena                 : 1;  /**< Enable the lane alignment. This should only be done after all enabled lanes have achieved
                                                         word boundary lock and scrambler synchronization. Note that hardware clears this when any
                                                         participating lane loses either word boundary lock or scrambler synchronization. */
	uint64_t rx_bdry_lock_ena             : 16; /**< Enable word-boundary lock. While disabled, received data is tossed. Once enabled, received
                                                         data is searched for legal two-bit patterns. Automatically cleared for disabled lanes. */
#else
	uint64_t rx_bdry_lock_ena             : 16;
	uint64_t rx_align_ena                 : 1;
	uint64_t rx_link_fc                   : 1;
	uint64_t tx_link_fc                   : 1;
	uint64_t la_mode                      : 1;
	uint64_t pkt_ena                      : 1;
	uint64_t pkt_flush                    : 1;
	uint64_t rx_fifo_max                  : 13;
	uint64_t reserved_35_35               : 1;
	uint64_t rx_fifo_hwm                  : 13;
	uint64_t reserved_49_49               : 1;
	uint64_t rx_fifo_cnt                  : 12;
	uint64_t reserved_62_63               : 2;
#endif
	} s;
	struct cvmx_ilk_rxx_cfg1_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_62_63               : 2;
	uint64_t rx_fifo_cnt                  : 12; /**< Number of 64-bit words currently consumed by this link in the
                                                         RX fifo. */
	uint64_t reserved_48_49               : 2;
	uint64_t rx_fifo_hwm                  : 12; /**< Number of 64-bit words consumed by this link before switch
                                                         transmitted link flow control status from XON to XOFF.

                                                         XON  = RX_FIFO_CNT < RX_FIFO_HWM
                                                         XOFF = RX_FIFO_CNT >= RX_FIFO_HWM. */
	uint64_t reserved_34_35               : 2;
	uint64_t rx_fifo_max                  : 12; /**< Maximum number of 64-bit words consumed by this link in the RX
                                                         fifo.  The sum of all links should be equal to 2048 (16KB) */
	uint64_t pkt_flush                    : 1;  /**< Packet receive flush.  Writing PKT_FLUSH=1 will cause all open
                                                         packets to be error-out, just as though the link went down. */
	uint64_t pkt_ena                      : 1;  /**< Packet receive enable.  When PKT_ENA=0, any received SOP causes
                                                         the entire packet to be dropped. */
	uint64_t la_mode                      : 1;  /**< 0 = Interlaken
                                                         1 = Interlaken Look-Aside */
	uint64_t tx_link_fc                   : 1;  /**< Link flow control status transmitted by the Tx-Link
                                                         XON (=1) when RX_FIFO_CNT <= RX_FIFO_HWM and lane alignment is
                                                         done */
	uint64_t rx_link_fc                   : 1;  /**< Link flow control status received in burst/idle control words.
                                                         XOFF (=0) will cause Tx-Link to stop transmitting on all
                                                         channels. */
	uint64_t rx_align_ena                 : 1;  /**< Enable the lane alignment.  This should only be done after all
                                                         enabled lanes have achieved word boundary lock and scrambler
                                                         synchronization.  Note: Hardware will clear this when any
                                                         participating lane loses either word boundary lock or scrambler
                                                         synchronization */
	uint64_t reserved_8_15                : 8;
	uint64_t rx_bdry_lock_ena             : 8;  /**< Enable word boundary lock.  While disabled, received data is
                                                         tossed.  Once enabled,  received data is searched for legal
                                                         2bit patterns.  Automatically cleared for disabled lanes. */
#else
	uint64_t rx_bdry_lock_ena             : 8;
	uint64_t reserved_8_15                : 8;
	uint64_t rx_align_ena                 : 1;
	uint64_t rx_link_fc                   : 1;
	uint64_t tx_link_fc                   : 1;
	uint64_t la_mode                      : 1;
	uint64_t pkt_ena                      : 1;
	uint64_t pkt_flush                    : 1;
	uint64_t rx_fifo_max                  : 12;
	uint64_t reserved_34_35               : 2;
	uint64_t rx_fifo_hwm                  : 12;
	uint64_t reserved_48_49               : 2;
	uint64_t rx_fifo_cnt                  : 12;
	uint64_t reserved_62_63               : 2;
#endif
	} cn68xx;
	struct cvmx_ilk_rxx_cfg1_cn68xx       cn68xxp1;
	struct cvmx_ilk_rxx_cfg1_s            cn78xx;
	struct cvmx_ilk_rxx_cfg1_s            cn78xxp1;
};
typedef union cvmx_ilk_rxx_cfg1 cvmx_ilk_rxx_cfg1_t;

/**
 * cvmx_ilk_rx#_cha#
 */
union cvmx_ilk_rxx_chax {
	uint64_t u64;
	struct cvmx_ilk_rxx_chax_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t port_kind                    : 6;  /**< Specify the port kind for the channel. */
#else
	uint64_t port_kind                    : 6;
	uint64_t reserved_6_63                : 58;
#endif
	} s;
	struct cvmx_ilk_rxx_chax_s            cn78xx;
	struct cvmx_ilk_rxx_chax_s            cn78xxp1;
};
typedef union cvmx_ilk_rxx_chax cvmx_ilk_rxx_chax_t;

/**
 * cvmx_ilk_rx#_cha_xon#
 */
union cvmx_ilk_rxx_cha_xonx {
	uint64_t u64;
	struct cvmx_ilk_rxx_cha_xonx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t xon                          : 64; /**< Flow control status for channels 0-255, where a 0 indicates the presence of backpressure
                                                         (i.e. XOFF) and 1 indicates the absence of backpressure (i.e. XON).
                                                         _ ILK_RX(0..1)_CHA_XON[0] -- Channels 63-0.
                                                         _ ILK_RX(0..1)_CHA_XON[1] -- Channels 127-64.
                                                         _ ILK_RX(0..1)_CHA_XON[2] -- Channels 191-128.
                                                         _ ILK_RX(0..1)_CHA_XON[3] -- Channels 255-192. */
#else
	uint64_t xon                          : 64;
#endif
	} s;
	struct cvmx_ilk_rxx_cha_xonx_s        cn78xx;
	struct cvmx_ilk_rxx_cha_xonx_s        cn78xxp1;
};
typedef union cvmx_ilk_rxx_cha_xonx cvmx_ilk_rxx_cha_xonx_t;

/**
 * cvmx_ilk_rx#_err_cfg
 */
union cvmx_ilk_rxx_err_cfg {
	uint64_t u64;
	struct cvmx_ilk_rxx_err_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t fwc_flip                     : 2;  /**< Testing feature. Flip syndrome bits <1:0> on writes to the FWC RAM to test single-bit or
                                                         double-bit errors. */
	uint64_t pmap_flip                    : 2;  /**< Testing feature. Flip syndrome bits <1:0> on writes to the PMAP RAM to test single-bit or
                                                         double-bit errors. */
	uint64_t reserved_2_15                : 14;
	uint64_t fwc_cor_dis                  : 1;  /**< Disable ECC corrector on FWC. */
	uint64_t pmap_cor_dis                 : 1;  /**< Disable ECC corrector on PMAP. */
#else
	uint64_t pmap_cor_dis                 : 1;
	uint64_t fwc_cor_dis                  : 1;
	uint64_t reserved_2_15                : 14;
	uint64_t pmap_flip                    : 2;
	uint64_t fwc_flip                     : 2;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_ilk_rxx_err_cfg_s         cn78xx;
	struct cvmx_ilk_rxx_err_cfg_s         cn78xxp1;
};
typedef union cvmx_ilk_rxx_err_cfg cvmx_ilk_rxx_err_cfg_t;

/**
 * cvmx_ilk_rx#_flow_ctl0
 */
union cvmx_ilk_rxx_flow_ctl0 {
	uint64_t u64;
	struct cvmx_ilk_rxx_flow_ctl0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t status                       : 64; /**< Flow control status for port-pipes 63-0, where a 1 indicates
                                                         the presence of backpressure (ie. XOFF) and 0 indicates the
                                                         absence of backpressure (ie. XON) */
#else
	uint64_t status                       : 64;
#endif
	} s;
	struct cvmx_ilk_rxx_flow_ctl0_s       cn68xx;
	struct cvmx_ilk_rxx_flow_ctl0_s       cn68xxp1;
};
typedef union cvmx_ilk_rxx_flow_ctl0 cvmx_ilk_rxx_flow_ctl0_t;

/**
 * cvmx_ilk_rx#_flow_ctl1
 */
union cvmx_ilk_rxx_flow_ctl1 {
	uint64_t u64;
	struct cvmx_ilk_rxx_flow_ctl1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t status                       : 64; /**< Flow control status for port-pipes 127-64, where a 1 indicates
                                                         the presence of backpressure (ie. XOFF) and 0 indicates the
                                                         absence of backpressure (ie. XON) */
#else
	uint64_t status                       : 64;
#endif
	} s;
	struct cvmx_ilk_rxx_flow_ctl1_s       cn68xx;
	struct cvmx_ilk_rxx_flow_ctl1_s       cn68xxp1;
};
typedef union cvmx_ilk_rxx_flow_ctl1 cvmx_ilk_rxx_flow_ctl1_t;

/**
 * cvmx_ilk_rx#_idx_cal
 */
union cvmx_ilk_rxx_idx_cal {
	uint64_t u64;
	struct cvmx_ilk_rxx_idx_cal_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t inc                          : 6;  /**< Increment to add to current index for next index. NOTE:
                                                         Increment performed after access to   ILK_RXx_MEM_CAL1 */
	uint64_t reserved_6_7                 : 2;
	uint64_t index                        : 6;  /**< Specify the group of 8 entries accessed by the next CSR
                                                         read/write to calendar table memory.  Software must never write
                                                         IDX >= 36 */
#else
	uint64_t index                        : 6;
	uint64_t reserved_6_7                 : 2;
	uint64_t inc                          : 6;
	uint64_t reserved_14_63               : 50;
#endif
	} s;
	struct cvmx_ilk_rxx_idx_cal_s         cn68xx;
	struct cvmx_ilk_rxx_idx_cal_s         cn68xxp1;
};
typedef union cvmx_ilk_rxx_idx_cal cvmx_ilk_rxx_idx_cal_t;

/**
 * cvmx_ilk_rx#_idx_stat0
 */
union cvmx_ilk_rxx_idx_stat0 {
	uint64_t u64;
	struct cvmx_ilk_rxx_idx_stat0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t clr                          : 1;  /**< CSR read to ILK_RXx_MEM_STAT0 clears the selected counter after
                                                         returning its current value. */
	uint64_t reserved_24_30               : 7;
	uint64_t inc                          : 8;  /**< Increment to add to current index for next index */
	uint64_t reserved_8_15                : 8;
	uint64_t index                        : 8;  /**< Specify the channel accessed during the next CSR read to the
                                                         ILK_RXx_MEM_STAT0 */
#else
	uint64_t index                        : 8;
	uint64_t reserved_8_15                : 8;
	uint64_t inc                          : 8;
	uint64_t reserved_24_30               : 7;
	uint64_t clr                          : 1;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_ilk_rxx_idx_stat0_s       cn68xx;
	struct cvmx_ilk_rxx_idx_stat0_s       cn68xxp1;
};
typedef union cvmx_ilk_rxx_idx_stat0 cvmx_ilk_rxx_idx_stat0_t;

/**
 * cvmx_ilk_rx#_idx_stat1
 */
union cvmx_ilk_rxx_idx_stat1 {
	uint64_t u64;
	struct cvmx_ilk_rxx_idx_stat1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t clr                          : 1;  /**< CSR read to ILK_RXx_MEM_STAT1 clears the selected counter after
                                                         returning its current value. */
	uint64_t reserved_24_30               : 7;
	uint64_t inc                          : 8;  /**< Increment to add to current index for next index */
	uint64_t reserved_8_15                : 8;
	uint64_t index                        : 8;  /**< Specify the channel accessed during the next CSR read to the
                                                         ILK_RXx_MEM_STAT1 */
#else
	uint64_t index                        : 8;
	uint64_t reserved_8_15                : 8;
	uint64_t inc                          : 8;
	uint64_t reserved_24_30               : 7;
	uint64_t clr                          : 1;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_ilk_rxx_idx_stat1_s       cn68xx;
	struct cvmx_ilk_rxx_idx_stat1_s       cn68xxp1;
};
typedef union cvmx_ilk_rxx_idx_stat1 cvmx_ilk_rxx_idx_stat1_t;

/**
 * cvmx_ilk_rx#_int
 */
union cvmx_ilk_rxx_int {
	uint64_t u64;
	struct cvmx_ilk_rxx_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_13_63               : 51;
	uint64_t pmap_dbe                     : 1;  /**< Port-kind map double-bit error. Throws ILK_INTSN_E::ILK_RX()_PMAP_DBE. */
	uint64_t pmap_sbe                     : 1;  /**< Port-kind map single-bit error. Throws ILK_INTSN_E::ILK_RX()_PMAP_SBE. */
	uint64_t fwc_dbe                      : 1;  /**< Flow control calendar table double-bit error. Throws ILK_INTSN_E::ILK_RX()_FWC_DBE. */
	uint64_t fwc_sbe                      : 1;  /**< Flow control calendar table single-bit error. Throws ILK_INTSN_E::ILK_RX()_FWC_SBE. */
	uint64_t pkt_drop_sop                 : 1;  /**< Entire packet dropped due to RX_FIFO_CNT == RX_FIFO_MAX, lack of reassembly IDs or because
                                                         ILK_RX()_CFG1[PKT_ENA]=0. Throws ILK_INTSN_E::ILK_RX()_PKT_DROP_SOP. */
	uint64_t pkt_drop_rid                 : 1;  /**< Entire packet dropped due to the lack of reassembly IDs or because
                                                         ILK_RX()_CFG1[PKT_ENA]=0. Throws ILK_INTSN_E::ILK_RX()_PKT_DROP_RID. */
	uint64_t pkt_drop_rxf                 : 1;  /**< Some/all of a packet dropped due to RX_FIFO_CNT == RX_FIFO_MAX. Throws
                                                         ILK_INTSN_E::ILK_RX()_PKT_DROP_RXF. */
	uint64_t lane_bad_word                : 1;  /**< A lane encountered either a bad 64/67 bit codeword or an unknown control word type. Throws
                                                         ILK_INTSN_E::ILK_RX()_LANE_BAD_WORD. */
	uint64_t stat_cnt_ovfl                : 1;  /**< Statistics counter overflow. Throws ILK_INTSN_E::ILK_RX()_STAT_CNT_OVFL. */
	uint64_t lane_align_done              : 1;  /**< Lane alignment successful. Throws ILK_INTSN_E::ILK_RX()_LANE_ALIGN_DONE. */
	uint64_t word_sync_done               : 1;  /**< All enabled lanes have achieved word boundary lock and scrambler synchronization. Lane
                                                         alignment may now be enabled. Throws ILK_INTSN_E::ILK_RX()_WORD_SYNC_DONE. */
	uint64_t crc24_err                    : 1;  /**< Burst CRC24 error. All open packets receive an error. Throws
                                                         ILK_INTSN_E::ILK_RX()_CRC24_ERR. */
	uint64_t lane_align_fail              : 1;  /**< Lane alignment fails (4 tries). Hardware repeats lane alignment until is succeeds or until
                                                         ILK_RX()_CFG1[RX_ALIGN_ENA] is cleared. Throws
                                                         ILK_INTSN_E::ILK_RX()_LANE_ALIGN_FAIL. */
#else
	uint64_t lane_align_fail              : 1;
	uint64_t crc24_err                    : 1;
	uint64_t word_sync_done               : 1;
	uint64_t lane_align_done              : 1;
	uint64_t stat_cnt_ovfl                : 1;
	uint64_t lane_bad_word                : 1;
	uint64_t pkt_drop_rxf                 : 1;
	uint64_t pkt_drop_rid                 : 1;
	uint64_t pkt_drop_sop                 : 1;
	uint64_t fwc_sbe                      : 1;
	uint64_t fwc_dbe                      : 1;
	uint64_t pmap_sbe                     : 1;
	uint64_t pmap_dbe                     : 1;
	uint64_t reserved_13_63               : 51;
#endif
	} s;
	struct cvmx_ilk_rxx_int_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t pkt_drop_sop                 : 1;  /**< Entire packet dropped due to RX_FIFO_CNT == RX_FIFO_MAX,
                                                         lack of reassembly-ids or because ILK_RXX_CFG1[PKT_ENA]=0      | $RW
                                                         because ILK_RXX_CFG1[PKT_ENA]=0

                                                         ***NOTE: Added in pass 2.0 */
	uint64_t pkt_drop_rid                 : 1;  /**< Entire packet dropped due to the lack of reassembly-ids or
                                                         because ILK_RXX_CFG1[PKT_ENA]=0 */
	uint64_t pkt_drop_rxf                 : 1;  /**< Some/all of a packet dropped due to RX_FIFO_CNT == RX_FIFO_MAX */
	uint64_t lane_bad_word                : 1;  /**< A lane encountered either a bad 64B/67B codeword or an unknown
                                                         control word type. */
	uint64_t stat_cnt_ovfl                : 1;  /**< Statistics counter overflow */
	uint64_t lane_align_done              : 1;  /**< Lane alignment successful */
	uint64_t word_sync_done               : 1;  /**< All enabled lanes have achieved word boundary lock and
                                                         scrambler synchronization.  Lane alignment may now be enabled. */
	uint64_t crc24_err                    : 1;  /**< Burst CRC24 error.  All open packets will be receive an error. */
	uint64_t lane_align_fail              : 1;  /**< Lane Alignment fails (4 tries).  Hardware will repeat lane
                                                         alignment until is succeeds or until ILK_RXx_CFG1[RX_ALIGN_ENA]
                                                         is cleared. */
#else
	uint64_t lane_align_fail              : 1;
	uint64_t crc24_err                    : 1;
	uint64_t word_sync_done               : 1;
	uint64_t lane_align_done              : 1;
	uint64_t stat_cnt_ovfl                : 1;
	uint64_t lane_bad_word                : 1;
	uint64_t pkt_drop_rxf                 : 1;
	uint64_t pkt_drop_rid                 : 1;
	uint64_t pkt_drop_sop                 : 1;
	uint64_t reserved_9_63                : 55;
#endif
	} cn68xx;
	struct cvmx_ilk_rxx_int_cn68xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t pkt_drop_rid                 : 1;  /**< Entire packet dropped due to the lack of reassembly-ids or
                                                         because ILK_RXX_CFG1[PKT_ENA]=0 */
	uint64_t pkt_drop_rxf                 : 1;  /**< Some/all of a packet dropped due to RX_FIFO_CNT == RX_FIFO_MAX */
	uint64_t lane_bad_word                : 1;  /**< A lane encountered either a bad 64B/67B codeword or an unknown
                                                         control word type. */
	uint64_t stat_cnt_ovfl                : 1;  /**< Statistics counter overflow */
	uint64_t lane_align_done              : 1;  /**< Lane alignment successful */
	uint64_t word_sync_done               : 1;  /**< All enabled lanes have achieved word boundary lock and
                                                         scrambler synchronization.  Lane alignment may now be enabled. */
	uint64_t crc24_err                    : 1;  /**< Burst CRC24 error.  All open packets will be receive an error. */
	uint64_t lane_align_fail              : 1;  /**< Lane Alignment fails (4 tries).  Hardware will repeat lane
                                                         alignment until is succeeds or until ILK_RXx_CFG1[RX_ALIGN_ENA]
                                                         is cleared. */
#else
	uint64_t lane_align_fail              : 1;
	uint64_t crc24_err                    : 1;
	uint64_t word_sync_done               : 1;
	uint64_t lane_align_done              : 1;
	uint64_t stat_cnt_ovfl                : 1;
	uint64_t lane_bad_word                : 1;
	uint64_t pkt_drop_rxf                 : 1;
	uint64_t pkt_drop_rid                 : 1;
	uint64_t reserved_8_63                : 56;
#endif
	} cn68xxp1;
	struct cvmx_ilk_rxx_int_s             cn78xx;
	struct cvmx_ilk_rxx_int_s             cn78xxp1;
};
typedef union cvmx_ilk_rxx_int cvmx_ilk_rxx_int_t;

/**
 * cvmx_ilk_rx#_int_en
 */
union cvmx_ilk_rxx_int_en {
	uint64_t u64;
	struct cvmx_ilk_rxx_int_en_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t pkt_drop_sop                 : 1;  /**< Entire packet dropped due to RX_FIFO_CNT == RX_FIFO_MAX,
                                                         lack of reassembly-ids or because ILK_RXX_CFG1[PKT_ENA]=0      | $PRW
                                                         because ILK_RXX_CFG1[PKT_ENA]=0

                                                         ***NOTE: Added in pass 2.0 */
	uint64_t pkt_drop_rid                 : 1;  /**< Entire packet dropped due to the lack of reassembly-ids or
                                                         because ILK_RXX_CFG1[PKT_ENA]=0 */
	uint64_t pkt_drop_rxf                 : 1;  /**< Some/all of a packet dropped due to RX_FIFO_CNT == RX_FIFO_MAX */
	uint64_t lane_bad_word                : 1;  /**< A lane encountered either a bad 64B/67B codeword or an unknown
                                                         control word type. */
	uint64_t stat_cnt_ovfl                : 1;  /**< Statistics counter overflow */
	uint64_t lane_align_done              : 1;  /**< Lane alignment successful */
	uint64_t word_sync_done               : 1;  /**< All enabled lanes have achieved word boundary lock and
                                                         scrambler synchronization.  Lane alignment may now be enabled. */
	uint64_t crc24_err                    : 1;  /**< Burst CRC24 error.  All open packets will be receive an error. */
	uint64_t lane_align_fail              : 1;  /**< Lane Alignment fails (4 tries) */
#else
	uint64_t lane_align_fail              : 1;
	uint64_t crc24_err                    : 1;
	uint64_t word_sync_done               : 1;
	uint64_t lane_align_done              : 1;
	uint64_t stat_cnt_ovfl                : 1;
	uint64_t lane_bad_word                : 1;
	uint64_t pkt_drop_rxf                 : 1;
	uint64_t pkt_drop_rid                 : 1;
	uint64_t pkt_drop_sop                 : 1;
	uint64_t reserved_9_63                : 55;
#endif
	} s;
	struct cvmx_ilk_rxx_int_en_s          cn68xx;
	struct cvmx_ilk_rxx_int_en_cn68xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t pkt_drop_rid                 : 1;  /**< Entire packet dropped due to the lack of reassembly-ids or
                                                         because ILK_RXX_CFG1[PKT_ENA]=0 */
	uint64_t pkt_drop_rxf                 : 1;  /**< Some/all of a packet dropped due to RX_FIFO_CNT == RX_FIFO_MAX */
	uint64_t lane_bad_word                : 1;  /**< A lane encountered either a bad 64B/67B codeword or an unknown
                                                         control word type. */
	uint64_t stat_cnt_ovfl                : 1;  /**< Statistics counter overflow */
	uint64_t lane_align_done              : 1;  /**< Lane alignment successful */
	uint64_t word_sync_done               : 1;  /**< All enabled lanes have achieved word boundary lock and
                                                         scrambler synchronization.  Lane alignment may now be enabled. */
	uint64_t crc24_err                    : 1;  /**< Burst CRC24 error.  All open packets will be receive an error. */
	uint64_t lane_align_fail              : 1;  /**< Lane Alignment fails (4 tries) */
#else
	uint64_t lane_align_fail              : 1;
	uint64_t crc24_err                    : 1;
	uint64_t word_sync_done               : 1;
	uint64_t lane_align_done              : 1;
	uint64_t stat_cnt_ovfl                : 1;
	uint64_t lane_bad_word                : 1;
	uint64_t pkt_drop_rxf                 : 1;
	uint64_t pkt_drop_rid                 : 1;
	uint64_t reserved_8_63                : 56;
#endif
	} cn68xxp1;
};
typedef union cvmx_ilk_rxx_int_en cvmx_ilk_rxx_int_en_t;

/**
 * cvmx_ilk_rx#_jabber
 */
union cvmx_ilk_rxx_jabber {
	uint64_t u64;
	struct cvmx_ilk_rxx_jabber_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t cnt                          : 16; /**< Byte count for jabber check. Failing packets are truncated to CNT bytes.
                                                         Hardware tracks the size of up to two concurrent packets per link. If using segment mode
                                                         with more than two channels, some large packets might not be flagged or truncated.
                                                         CNT must be 8-byte aligned such that CNT[2:0] = 0x0. */
#else
	uint64_t cnt                          : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_ilk_rxx_jabber_s          cn68xx;
	struct cvmx_ilk_rxx_jabber_s          cn68xxp1;
	struct cvmx_ilk_rxx_jabber_s          cn78xx;
	struct cvmx_ilk_rxx_jabber_s          cn78xxp1;
};
typedef union cvmx_ilk_rxx_jabber cvmx_ilk_rxx_jabber_t;

/**
 * cvmx_ilk_rx#_mem_cal0
 *
 * Notes:
 * Software must program the calendar table prior to enabling the
 * link.
 *
 * Software must always write ILK_RXx_MEM_CAL0 then ILK_RXx_MEM_CAL1.
 * Software must never write them in reverse order or write one without
 * writing the other.
 *
 * A given calendar table entry has no effect on PKO pipe
 * backpressure when either:
 *  - ENTRY_CTLx=Link (1), or
 *  - ENTRY_CTLx=XON (3) and PORT_PIPEx is outside the range of ILK_TXx_PIPE[BASE/NUMP].
 *
 * Within the 8 calendar table entries of one IDX value, if more
 * than one affects the same PKO pipe, XOFF always wins over XON,
 * regardless of the calendar table order.
 *
 * Software must always read ILK_RXx_MEM_CAL0 then ILK_RXx_MEM_CAL1.  Software
 * must never read them in reverse order or read one without reading the
 * other.
 */
union cvmx_ilk_rxx_mem_cal0 {
	uint64_t u64;
	struct cvmx_ilk_rxx_mem_cal0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t entry_ctl3                   : 2;  /**< XON/XOFF destination for entry (IDX*8)+3

                                                         - 0: PKO port-pipe  Apply backpressure received from the
                                                                            remote tranmitter to the PKO pipe selected
                                                                            by PORT_PIPE3.

                                                         - 1: Link           Apply the backpressure received from the
                                                                            remote transmitter to link backpressure.
                                                                            PORT_PIPE3 is unused.

                                                         - 2: XOFF           Apply XOFF to the PKO pipe selected by
                                                                            PORT_PIPE3.

                                                         - 3: XON            Apply XON to the PKO pipe selected by
                                                                            PORT_PIPE3. The calendar table entry is
                                                                            effectively unused if PORT_PIPE3 is out of
                                                                            range of ILK_TXx_PIPE[BASE/NUMP].

                                                         This field applies to one of bits <52>, <44>, or <28> in the
                                                         Interlaken control word. */
	uint64_t port_pipe3                   : 7;  /**< Select PKO port-pipe for calendar table entry (IDX*8)+3

                                                         PORT_PIPE3 must reside in the range of ILK_TXx_PIPE[BASE/NUMP]
                                                         when ENTRY_CTL3 is "XOFF" (2) or "PKO port-pipe" (0). */
	uint64_t entry_ctl2                   : 2;  /**< XON/XOFF destination for entry (IDX*8)+2

                                                         - 0: PKO port-pipe  Apply backpressure received from the
                                                                            remote tranmitter to the PKO pipe selected
                                                                            by PORT_PIPE2.

                                                         - 1: Link           Apply the backpressure received from the
                                                                            remote transmitter to link backpressure.
                                                                            PORT_PIPE2 is unused.

                                                         - 2: XOFF           Apply XOFF to the PKO pipe selected by
                                                                            PORT_PIPE2.

                                                         - 3: XON            Apply XON to the PKO pipe selected by
                                                                            PORT_PIPE2. The calendar table entry is
                                                                            effectively unused if PORT_PIPE2 is out of
                                                                            range of ILK_TXx_PIPE[BASE/NUMP].

                                                         This field applies to one of bits <53>, <45>, or <29> in the
                                                         Interlaken control word. */
	uint64_t port_pipe2                   : 7;  /**< Select PKO port-pipe for calendar table entry (IDX*8)+2

                                                         PORT_PIPE2 must reside in the range of ILK_TXx_PIPE[BASE/NUMP]
                                                         when ENTRY_CTL2 is "XOFF" (2) or "PKO port-pipe" (0). */
	uint64_t entry_ctl1                   : 2;  /**< XON/XOFF destination for entry (IDX*8)+1

                                                         - 0: PKO port-pipe  Apply backpressure received from the
                                                                            remote tranmitter to the PKO pipe selected
                                                                            by PORT_PIPE1.

                                                         - 1: Link           Apply the backpressure received from the
                                                                            remote transmitter to link backpressure.
                                                                            PORT_PIPE1 is unused.

                                                         - 2: XOFF           Apply XOFF to the PKO pipe selected by
                                                                            PORT_PIPE1.

                                                         - 3: XON            Apply XON to the PKO pipe selected by
                                                                            PORT_PIPE1. The calendar table entry is
                                                                            effectively unused if PORT_PIPE1 is out of
                                                                            range of ILK_TXx_PIPE[BASE/NUMP].

                                                         This field applies to one of bits <54>, <46>, or <30> in the
                                                         Interlaken control word. */
	uint64_t port_pipe1                   : 7;  /**< Select PKO port-pipe for calendar table entry (IDX*8)+1

                                                         PORT_PIPE1 must reside in the range of ILK_TXx_PIPE[BASE/NUMP]
                                                         when ENTRY_CTL1 is "XOFF" (2) or "PKO port-pipe" (0). */
	uint64_t entry_ctl0                   : 2;  /**< XON/XOFF destination for entry (IDX*8)+0

                                                         - 0: PKO port-pipe  Apply backpressure received from the
                                                                            remote tranmitter to the PKO pipe selected
                                                                            by PORT_PIPE0.

                                                         - 1: Link           Apply the backpressure received from the
                                                                            remote transmitter to link backpressure.
                                                                            PORT_PIPE0 is unused.

                                                         - 2: XOFF           Apply XOFF to the PKO pipe selected by
                                                                            PORT_PIPE0.

                                                         - 3: XON            Apply XON to the PKO pipe selected by
                                                                            PORT_PIPE0. The calendar table entry is
                                                                            effectively unused if PORT_PIPEx is out of
                                                                            range of ILK_TXx_PIPE[BASE/NUMP].

                                                         This field applies to one of bits <55>, <47>, or <31> in the
                                                         Interlaken control word. */
	uint64_t port_pipe0                   : 7;  /**< Select PKO port-pipe for calendar table entry (IDX*8)+0

                                                         PORT_PIPE0 must reside in the range of ILK_TXx_PIPE[BASE/NUMP]
                                                         when ENTRY_CTL0 is "XOFF" (2) or "PKO port-pipe" (0). */
#else
	uint64_t port_pipe0                   : 7;
	uint64_t entry_ctl0                   : 2;
	uint64_t port_pipe1                   : 7;
	uint64_t entry_ctl1                   : 2;
	uint64_t port_pipe2                   : 7;
	uint64_t entry_ctl2                   : 2;
	uint64_t port_pipe3                   : 7;
	uint64_t entry_ctl3                   : 2;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_ilk_rxx_mem_cal0_s        cn68xx;
	struct cvmx_ilk_rxx_mem_cal0_s        cn68xxp1;
};
typedef union cvmx_ilk_rxx_mem_cal0 cvmx_ilk_rxx_mem_cal0_t;

/**
 * cvmx_ilk_rx#_mem_cal1
 *
 * Notes:
 * Software must program the calendar table prior to enabling the
 * link.
 *
 * Software must always write ILK_RXx_MEM_CAL0 then ILK_RXx_MEM_CAL1.
 * Software must never write them in reverse order or write one without
 * writing the other.
 *
 * A given calendar table entry has no effect on PKO pipe
 * backpressure when either:
 *  - ENTRY_CTLx=Link (1), or
 *  - ENTRY_CTLx=XON (3) and PORT_PIPEx is outside the range of ILK_TXx_PIPE[BASE/NUMP].
 *
 * Within the 8 calendar table entries of one IDX value, if more
 * than one affects the same PKO pipe, XOFF always wins over XON,
 * regardless of the calendar table order.
 *
 * Software must always read ILK_RXx_MEM_CAL0 then ILK_Rx_MEM_CAL1.  Software
 * must never read them in reverse order or read one without reading the
 * other.
 */
union cvmx_ilk_rxx_mem_cal1 {
	uint64_t u64;
	struct cvmx_ilk_rxx_mem_cal1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t entry_ctl7                   : 2;  /**< XON/XOFF destination for entry (IDX*8)+7

                                                         - 0: PKO port-pipe  Apply backpressure received from the
                                                                            remote tranmitter to the PKO pipe selected
                                                                            by PORT_PIPE7.

                                                         - 1: Link           Apply the backpressure received from the
                                                                            remote transmitter to link backpressure.
                                                                            PORT_PIPE7 is unused.

                                                         - 2: XOFF           Apply XOFF to the PKO pipe selected by
                                                                            PORT_PIPE7.

                                                         - 3: XON            Apply XON to the PKO pipe selected by
                                                                            PORT_PIPE7. The calendar table entry is
                                                                            effectively unused if PORT_PIPE3 is out of
                                                                            range of ILK_TXx_PIPE[BASE/NUMP].

                                                         This field applies to one of bits <48>, <40>, or <24> in the
                                                         Interlaken control word. */
	uint64_t port_pipe7                   : 7;  /**< Select PKO port-pipe for calendar table entry (IDX*8)+7

                                                         PORT_PIPE7 must reside in the range of ILK_TXx_PIPE[BASE/NUMP]
                                                         when ENTRY_CTL7 is "XOFF" (2) or "PKO port-pipe" (0). */
	uint64_t entry_ctl6                   : 2;  /**< XON/XOFF destination for entry (IDX*8)+6

                                                         - 0: PKO port-pipe  Apply backpressure received from the
                                                                            remote tranmitter to the PKO pipe selected
                                                                            by PORT_PIPE6.

                                                         - 1: Link           Apply the backpressure received from the
                                                                            remote transmitter to link backpressure.
                                                                            PORT_PIPE6 is unused.

                                                         - 2: XOFF           Apply XOFF to the PKO pipe selected by
                                                                            PORT_PIPE6.

                                                         - 3: XON            Apply XON to the PKO pipe selected by
                                                                            PORT_PIPE6. The calendar table entry is
                                                                            effectively unused if PORT_PIPE6 is out of
                                                                            range of ILK_TXx_PIPE[BASE/NUMP].

                                                         This field applies to one of bits <49>, <41>, or <25> in the
                                                         Interlaken control word. */
	uint64_t port_pipe6                   : 7;  /**< Select PKO port-pipe for calendar table entry (IDX*8)+6

                                                         PORT_PIPE6 must reside in the range of ILK_TXx_PIPE[BASE/NUMP]
                                                         when ENTRY_CTL6 is "XOFF" (2) or "PKO port-pipe" (0). */
	uint64_t entry_ctl5                   : 2;  /**< XON/XOFF destination for entry (IDX*8)+5

                                                         - 0: PKO port-pipe  Apply backpressure received from the
                                                                            remote tranmitter to the PKO pipe selected
                                                                            by PORT_PIPE5.

                                                         - 1: Link           Apply the backpressure received from the
                                                                            remote transmitter to link backpressure.
                                                                            PORT_PIPE5 is unused.

                                                         - 2: XOFF           Apply XOFF to the PKO pipe selected by
                                                                            PORT_PIPE5.

                                                         - 3: XON            Apply XON to the PKO pipe selected by
                                                                            PORT_PIPE5. The calendar table entry is
                                                                            effectively unused if PORT_PIPE5 is out of
                                                                            range of ILK_TXx_PIPE[BASE/NUMP].

                                                         This field applies to one of bits <50>, <42>, or <26> in the
                                                         Interlaken control word. */
	uint64_t port_pipe5                   : 7;  /**< Select PKO port-pipe for calendar table entry (IDX*8)+5

                                                         PORT_PIPE5 must reside in the range of ILK_TXx_PIPE[BASE/NUMP]
                                                         when ENTRY_CTL5 is "XOFF" (2) or "PKO port-pipe" (0). */
	uint64_t entry_ctl4                   : 2;  /**< XON/XOFF destination for entry (IDX*8)+4

                                                         - 0: PKO port-pipe  Apply backpressure received from the
                                                                            remote tranmitter to the PKO pipe selected
                                                                            by PORT_PIPE4.

                                                         - 1: Link           Apply the backpressure received from the
                                                                            remote transmitter to link backpressure.
                                                                            PORT_PIPE4 is unused.

                                                         - 2: XOFF           Apply XOFF to the PKO pipe selected by
                                                                            PORT_PIPE4.

                                                         - 3: XON            Apply XON to the PKO pipe selected by
                                                                            PORT_PIPE4. The calendar table entry is
                                                                            effectively unused if PORT_PIPE4 is out of
                                                                            range of ILK_TXx_PIPE[BASE/NUMP].

                                                         This field applies to one of bits <51>, <43>, or <27> in the
                                                         Interlaken control word. */
	uint64_t port_pipe4                   : 7;  /**< Select PKO port-pipe for calendar table entry (IDX*8)+4

                                                         PORT_PIPE4 must reside in the range of ILK_TXx_PIPE[BASE/NUMP]
                                                         when ENTRY_CTL4 is "XOFF" (2) or "PKO port-pipe" (0). */
#else
	uint64_t port_pipe4                   : 7;
	uint64_t entry_ctl4                   : 2;
	uint64_t port_pipe5                   : 7;
	uint64_t entry_ctl5                   : 2;
	uint64_t port_pipe6                   : 7;
	uint64_t entry_ctl6                   : 2;
	uint64_t port_pipe7                   : 7;
	uint64_t entry_ctl7                   : 2;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_ilk_rxx_mem_cal1_s        cn68xx;
	struct cvmx_ilk_rxx_mem_cal1_s        cn68xxp1;
};
typedef union cvmx_ilk_rxx_mem_cal1 cvmx_ilk_rxx_mem_cal1_t;

/**
 * cvmx_ilk_rx#_mem_stat0
 */
union cvmx_ilk_rxx_mem_stat0 {
	uint64_t u64;
	struct cvmx_ilk_rxx_mem_stat0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_28_63               : 36;
	uint64_t rx_pkt                       : 28; /**< Number of packets received (256M)
                                                         Channel selected by ILK_RXx_IDX_STAT0[IDX].  Saturates.
                                                         Interrupt on saturation if ILK_RXX_INT_EN[STAT_CNT_OVFL]=1. */
#else
	uint64_t rx_pkt                       : 28;
	uint64_t reserved_28_63               : 36;
#endif
	} s;
	struct cvmx_ilk_rxx_mem_stat0_s       cn68xx;
	struct cvmx_ilk_rxx_mem_stat0_s       cn68xxp1;
};
typedef union cvmx_ilk_rxx_mem_stat0 cvmx_ilk_rxx_mem_stat0_t;

/**
 * cvmx_ilk_rx#_mem_stat1
 */
union cvmx_ilk_rxx_mem_stat1 {
	uint64_t u64;
	struct cvmx_ilk_rxx_mem_stat1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t rx_bytes                     : 36; /**< Number of bytes received (64GB)
                                                         Channel selected by ILK_RXx_IDX_STAT1[IDX].    Saturates.
                                                         Interrupt on saturation if ILK_RXX_INT_EN[STAT_CNT_OVFL]=1. */
#else
	uint64_t rx_bytes                     : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_ilk_rxx_mem_stat1_s       cn68xx;
	struct cvmx_ilk_rxx_mem_stat1_s       cn68xxp1;
};
typedef union cvmx_ilk_rxx_mem_stat1 cvmx_ilk_rxx_mem_stat1_t;

/**
 * cvmx_ilk_rx#_pkt_cnt#
 */
union cvmx_ilk_rxx_pkt_cntx {
	uint64_t u64;
	struct cvmx_ilk_rxx_pkt_cntx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_34_63               : 30;
	uint64_t rx_pkt                       : 34; /**< Number of packets received per channel. Wraps on overflow. On overflow, sets
                                                         ILK_RX()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t rx_pkt                       : 34;
	uint64_t reserved_34_63               : 30;
#endif
	} s;
	struct cvmx_ilk_rxx_pkt_cntx_s        cn78xx;
	struct cvmx_ilk_rxx_pkt_cntx_s        cn78xxp1;
};
typedef union cvmx_ilk_rxx_pkt_cntx cvmx_ilk_rxx_pkt_cntx_t;

/**
 * cvmx_ilk_rx#_rid
 */
union cvmx_ilk_rxx_rid {
	uint64_t u64;
	struct cvmx_ilk_rxx_rid_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t max_cnt                      : 7;  /**< Maximum number of reassembly IDs allowed for a given link. If an SOP arrives and the link
                                                         has already allocated at least MAX_CNT reassembly IDs, the packet is dropped.
                                                         An SOP allocates a reassembly ID; an EOP frees a reassembly ID. */
#else
	uint64_t max_cnt                      : 7;
	uint64_t reserved_7_63                : 57;
#endif
	} s;
	struct cvmx_ilk_rxx_rid_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t max_cnt                      : 6;  /**< Maximum number of reassembly-ids allowed for a given link.  If
                                                         an SOP arrives and the link has already allocated at least
                                                         MAX_CNT reassembly-ids, the packet will be dropped.

                                                         Note: An an SOP allocates a reassembly-ids.
                                                         Note: An an EOP frees a reassembly-ids.

                                                         ***NOTE: Added in pass 2.0 */
#else
	uint64_t max_cnt                      : 6;
	uint64_t reserved_6_63                : 58;
#endif
	} cn68xx;
	struct cvmx_ilk_rxx_rid_s             cn78xx;
	struct cvmx_ilk_rxx_rid_s             cn78xxp1;
};
typedef union cvmx_ilk_rxx_rid cvmx_ilk_rxx_rid_t;

/**
 * cvmx_ilk_rx#_stat0
 */
union cvmx_ilk_rxx_stat0 {
	uint64_t u64;
	struct cvmx_ilk_rxx_stat0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_35_63               : 29;
	uint64_t crc24_match_cnt              : 35; /**< Indicates the number of CRC24 matches received. Wraps on overflow if
                                                         ILK_RX()_CFG0[LNK_STATS_WRAP]=1. Otherwise, saturates. On overflow/saturate, sets
                                                         ILK_RX()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t crc24_match_cnt              : 35;
	uint64_t reserved_35_63               : 29;
#endif
	} s;
	struct cvmx_ilk_rxx_stat0_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_33_63               : 31;
	uint64_t crc24_match_cnt              : 33; /**< Number of CRC24 matches received.  Saturates.  Interrupt on
                                                         saturation if ILK_RXX_INT_EN[STAT_CNT_OVFL]=1. */
#else
	uint64_t crc24_match_cnt              : 33;
	uint64_t reserved_33_63               : 31;
#endif
	} cn68xx;
	struct cvmx_ilk_rxx_stat0_cn68xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_27_63               : 37;
	uint64_t crc24_match_cnt              : 27; /**< Number of CRC24 matches received.  Saturates.  Interrupt on
                                                         saturation if ILK_RXX_INT_EN[STAT_CNT_OVFL]=1. */
#else
	uint64_t crc24_match_cnt              : 27;
	uint64_t reserved_27_63               : 37;
#endif
	} cn68xxp1;
	struct cvmx_ilk_rxx_stat0_s           cn78xx;
	struct cvmx_ilk_rxx_stat0_s           cn78xxp1;
};
typedef union cvmx_ilk_rxx_stat0 cvmx_ilk_rxx_stat0_t;

/**
 * cvmx_ilk_rx#_stat1
 */
union cvmx_ilk_rxx_stat1 {
	uint64_t u64;
	struct cvmx_ilk_rxx_stat1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t crc24_err_cnt                : 20; /**< Indicates the number of bursts with a detected CRC error. Wraps on overflow if
                                                         ILK_RX()_CFG0[LNK_STATS_WRAP]=1. Otherwise, saturates. On overflow/saturate, sets
                                                         ILK_RX()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t crc24_err_cnt                : 20;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_ilk_rxx_stat1_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t crc24_err_cnt                : 18; /**< Number of bursts with a detected CRC error.  Saturates.
                                                         Interrupt on saturation if ILK_RXX_INT_EN[STAT_CNT_OVFL]=1. */
#else
	uint64_t crc24_err_cnt                : 18;
	uint64_t reserved_18_63               : 46;
#endif
	} cn68xx;
	struct cvmx_ilk_rxx_stat1_cn68xx      cn68xxp1;
	struct cvmx_ilk_rxx_stat1_s           cn78xx;
	struct cvmx_ilk_rxx_stat1_s           cn78xxp1;
};
typedef union cvmx_ilk_rxx_stat1 cvmx_ilk_rxx_stat1_t;

/**
 * cvmx_ilk_rx#_stat2
 */
union cvmx_ilk_rxx_stat2 {
	uint64_t u64;
	struct cvmx_ilk_rxx_stat2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_50_63               : 14;
	uint64_t brst_not_full_cnt            : 18; /**< Indicates the number of bursts received that terminated without an EOP and contained fewer
                                                         than BurstMax words. Wraps on overflow if ILK_RX()_CFG0[LNK_STATS_WRAP]=1. Otherwise,
                                                         saturates. On overflow/saturate, sets ILK_RX()_INT[STAT_CNT_OVFL]. */
	uint64_t reserved_30_31               : 2;
	uint64_t brst_cnt                     : 30; /**< Indicates the number of bursts correctly received. (i.e. good CRC24, not in violation of
                                                         BurstMax or BurstShort). Wraps on overflow if ILK_RX()_CFG0[LNK_STATS_WRAP]=1.
                                                         Otherwise, saturates. On overflow/saturate, sets ILK_RX()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t brst_cnt                     : 30;
	uint64_t reserved_30_31               : 2;
	uint64_t brst_not_full_cnt            : 18;
	uint64_t reserved_50_63               : 14;
#endif
	} s;
	struct cvmx_ilk_rxx_stat2_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t brst_not_full_cnt            : 16; /**< Number of bursts received which terminated without an eop and
                                                         contained fewer than BurstMax words.  Saturates.  Interrupt on
                                                         saturation if ILK_RXX_INT_EN[STAT_CNT_OVFL]=1. */
	uint64_t reserved_28_31               : 4;
	uint64_t brst_cnt                     : 28; /**< Number of bursts correctly received. (ie. good CRC24, not in
                                                         violation of BurstMax or BurstShort) */
#else
	uint64_t brst_cnt                     : 28;
	uint64_t reserved_28_31               : 4;
	uint64_t brst_not_full_cnt            : 16;
	uint64_t reserved_48_63               : 16;
#endif
	} cn68xx;
	struct cvmx_ilk_rxx_stat2_cn68xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t brst_not_full_cnt            : 16; /**< Number of bursts received which terminated without an eop and
                                                         contained fewer than BurstMax words.  Saturates.  Interrupt on
                                                         saturation if ILK_RXX_INT_EN[STAT_CNT_OVFL]=1. */
	uint64_t reserved_16_31               : 16;
	uint64_t brst_cnt                     : 16; /**< Number of bursts correctly received. (ie. good CRC24, not in
                                                         violation of BurstMax or BurstShort) */
#else
	uint64_t brst_cnt                     : 16;
	uint64_t reserved_16_31               : 16;
	uint64_t brst_not_full_cnt            : 16;
	uint64_t reserved_48_63               : 16;
#endif
	} cn68xxp1;
	struct cvmx_ilk_rxx_stat2_s           cn78xx;
	struct cvmx_ilk_rxx_stat2_s           cn78xxp1;
};
typedef union cvmx_ilk_rxx_stat2 cvmx_ilk_rxx_stat2_t;

/**
 * cvmx_ilk_rx#_stat3
 */
union cvmx_ilk_rxx_stat3 {
	uint64_t u64;
	struct cvmx_ilk_rxx_stat3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t brst_max_err_cnt             : 18; /**< Indicates the number of bursts received longer than the BurstMax parameter. Wraps on
                                                         overflow if ILK_RX()_CFG0[LNK_STATS_WRAP]=1. Otherwise, saturates. On
                                                         overflow/saturate, sets ILK_RX()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t brst_max_err_cnt             : 18;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_ilk_rxx_stat3_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t brst_max_err_cnt             : 16; /**< Number of bursts received longer than the BurstMax parameter */
#else
	uint64_t brst_max_err_cnt             : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} cn68xx;
	struct cvmx_ilk_rxx_stat3_cn68xx      cn68xxp1;
	struct cvmx_ilk_rxx_stat3_s           cn78xx;
	struct cvmx_ilk_rxx_stat3_s           cn78xxp1;
};
typedef union cvmx_ilk_rxx_stat3 cvmx_ilk_rxx_stat3_t;

/**
 * cvmx_ilk_rx#_stat4
 */
union cvmx_ilk_rxx_stat4 {
	uint64_t u64;
	struct cvmx_ilk_rxx_stat4_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t brst_shrt_err_cnt            : 18; /**< Indicates the number of bursts received that violate the BurstShort parameter. Wraps on
                                                         overflow if ILK_RX()_CFG0[LNK_STATS_WRAP]=1. Otherwise, saturates. On
                                                         overflow/saturate, sets ILK_RX()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t brst_shrt_err_cnt            : 18;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_ilk_rxx_stat4_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t brst_shrt_err_cnt            : 16; /**< Number of bursts received that violate the BurstShort
                                                         parameter.  Saturates.  Interrupt on saturation if
                                                         ILK_RXX_INT_EN[STAT_CNT_OVFL]=1. */
#else
	uint64_t brst_shrt_err_cnt            : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} cn68xx;
	struct cvmx_ilk_rxx_stat4_cn68xx      cn68xxp1;
	struct cvmx_ilk_rxx_stat4_s           cn78xx;
	struct cvmx_ilk_rxx_stat4_s           cn78xxp1;
};
typedef union cvmx_ilk_rxx_stat4 cvmx_ilk_rxx_stat4_t;

/**
 * cvmx_ilk_rx#_stat5
 */
union cvmx_ilk_rxx_stat5 {
	uint64_t u64;
	struct cvmx_ilk_rxx_stat5_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_25_63               : 39;
	uint64_t align_cnt                    : 25; /**< Indicates the number of alignment sequences received (i.e. those that do not violate the
                                                         current alignment). Wraps on overflow if ILK_RX()_CFG0[LNK_STATS_WRAP]=1. Otherwise,
                                                         saturates. On overflow/saturate, sets ILK_RX()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t align_cnt                    : 25;
	uint64_t reserved_25_63               : 39;
#endif
	} s;
	struct cvmx_ilk_rxx_stat5_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_23_63               : 41;
	uint64_t align_cnt                    : 23; /**< Number of alignment sequences received  (ie. those that do not
                                                         violate the current alignment).  Saturates.  Interrupt on
                                                         saturation if ILK_RXX_INT_EN[STAT_CNT_OVFL]=1. */
#else
	uint64_t align_cnt                    : 23;
	uint64_t reserved_23_63               : 41;
#endif
	} cn68xx;
	struct cvmx_ilk_rxx_stat5_cn68xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t align_cnt                    : 16; /**< Number of alignment sequences received  (ie. those that do not
                                                         violate the current alignment).  Saturates.  Interrupt on
                                                         saturation if ILK_RXX_INT_EN[STAT_CNT_OVFL]=1. */
#else
	uint64_t align_cnt                    : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} cn68xxp1;
	struct cvmx_ilk_rxx_stat5_s           cn78xx;
	struct cvmx_ilk_rxx_stat5_s           cn78xxp1;
};
typedef union cvmx_ilk_rxx_stat5 cvmx_ilk_rxx_stat5_t;

/**
 * cvmx_ilk_rx#_stat6
 */
union cvmx_ilk_rxx_stat6 {
	uint64_t u64;
	struct cvmx_ilk_rxx_stat6_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t align_err_cnt                : 18; /**< Indicates the number of alignment sequences received in error (i.e. those that violate the
                                                         current alignment). Wraps on overflow if ILK_RX()_CFG0[LNK_STATS_WRAP]=1. Otherwise,
                                                         saturates. On overflow/saturate, sets ILK_RX()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t align_err_cnt                : 18;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_ilk_rxx_stat6_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t align_err_cnt                : 16; /**< Number of alignment sequences received in error (ie. those that
                                                         violate the current alignment).  Saturates.  Interrupt on
                                                         saturation if ILK_RXX_INT_EN[STAT_CNT_OVFL]=1. */
#else
	uint64_t align_err_cnt                : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} cn68xx;
	struct cvmx_ilk_rxx_stat6_cn68xx      cn68xxp1;
	struct cvmx_ilk_rxx_stat6_s           cn78xx;
	struct cvmx_ilk_rxx_stat6_s           cn78xxp1;
};
typedef union cvmx_ilk_rxx_stat6 cvmx_ilk_rxx_stat6_t;

/**
 * cvmx_ilk_rx#_stat7
 */
union cvmx_ilk_rxx_stat7 {
	uint64_t u64;
	struct cvmx_ilk_rxx_stat7_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t bad_64b67b_cnt               : 18; /**< Indicates the number of bad 64/67 bit code words.Wraps on overflow if
                                                         ILK_RX()_CFG0[LNK_STATS_WRAP]=1. Otherwise, saturates. On overflow/saturate, sets
                                                         ILK_RX()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t bad_64b67b_cnt               : 18;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_ilk_rxx_stat7_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t bad_64b67b_cnt               : 16; /**< Number of bad 64B/67B codewords.  Saturates.  Interrupt on
                                                         saturation if ILK_RXX_INT_EN[STAT_CNT_OVFL]=1. */
#else
	uint64_t bad_64b67b_cnt               : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} cn68xx;
	struct cvmx_ilk_rxx_stat7_cn68xx      cn68xxp1;
	struct cvmx_ilk_rxx_stat7_s           cn78xx;
	struct cvmx_ilk_rxx_stat7_s           cn78xxp1;
};
typedef union cvmx_ilk_rxx_stat7 cvmx_ilk_rxx_stat7_t;

/**
 * cvmx_ilk_rx#_stat8
 */
union cvmx_ilk_rxx_stat8 {
	uint64_t u64;
	struct cvmx_ilk_rxx_stat8_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t pkt_drop_rid_cnt             : 16; /**< Indicates the number of packets dropped due to the lack of reassembly IDs or because
                                                         ILK_RX()_CFG1[PKT_ENA] = 0. Wraps on overflow if ILK_RX()_CFG0[LNK_STATS_WRAP]=1.
                                                         Otherwise, saturates. On overflow/saturate, sets ILK_RX()_INT[STAT_CNT_OVFL]. */
	uint64_t pkt_drop_rxf_cnt             : 16; /**< Indicates the number of packets dropped due to RX_FIFO_CNT >= RX_FIFO_MAX. Wraps on
                                                         overflow if ILK_RX()_CFG0[LNK_STATS_WRAP]=1.Otherwise, saturates. On
                                                         overflow/saturate, sets ILK_RX()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t pkt_drop_rxf_cnt             : 16;
	uint64_t pkt_drop_rid_cnt             : 16;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_ilk_rxx_stat8_s           cn68xx;
	struct cvmx_ilk_rxx_stat8_s           cn68xxp1;
	struct cvmx_ilk_rxx_stat8_s           cn78xx;
	struct cvmx_ilk_rxx_stat8_s           cn78xxp1;
};
typedef union cvmx_ilk_rxx_stat8 cvmx_ilk_rxx_stat8_t;

/**
 * cvmx_ilk_rx#_stat9
 *
 * This register is reserved.
 *
 */
union cvmx_ilk_rxx_stat9 {
	uint64_t u64;
	struct cvmx_ilk_rxx_stat9_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_ilk_rxx_stat9_s           cn68xx;
	struct cvmx_ilk_rxx_stat9_s           cn68xxp1;
	struct cvmx_ilk_rxx_stat9_s           cn78xx;
	struct cvmx_ilk_rxx_stat9_s           cn78xxp1;
};
typedef union cvmx_ilk_rxx_stat9 cvmx_ilk_rxx_stat9_t;

/**
 * cvmx_ilk_rx_lne#_cfg
 */
union cvmx_ilk_rx_lnex_cfg {
	uint64_t u64;
	struct cvmx_ilk_rx_lnex_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t rx_dis_psh_skip              : 1;  /**< When asserted, skip words are discarded in the lane logic; when deasserted, skip words are
                                                         destripped.
                                                         If the lane is in internal loopback mode, this field is ignored and skip words are always
                                                         discarded in the lane logic. */
	uint64_t reserved_7_7                 : 1;
	uint64_t rx_dis_disp_chk              : 1;  /**< Disable the RX disparity check, see ILK_RX_LNE()_INT[DISP_ERR]. */
	uint64_t rx_scrm_sync                 : 1;  /**< RX scrambler-synchronization status. A 1 means synchronization has been achieved. */
	uint64_t rx_bdry_sync                 : 1;  /**< RX word-boundary-synchronization status. A 1 means synchronization has been achieved */
	uint64_t rx_dis_ukwn                  : 1;  /**< Disable normal response to unknown words. Unknown words are still logged but do not cause
                                                         an error to all open channels. */
	uint64_t rx_dis_scram                 : 1;  /**< Disable lane scrambler. For diagnostic use only. */
	uint64_t stat_rdclr                   : 1;  /**< A CSR read operation to ILK_RX_LNEn_STAT* clears the selected counter after returning its
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
	struct cvmx_ilk_rx_lnex_cfg_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t rx_dis_psh_skip              : 1;  /**< When RX_DIS_PSH_SKIP=0, skip words are de-stripped.
                                                         When RX_DIS_PSH_SKIP=1, skip words are discarded in the lane
                                                         logic.

                                                         If the lane is in internal loopback mode, RX_DIS_PSH_SKIP
                                                         is ignored and skip words are always discarded in the lane
                                                         logic.

                                                         ***NOTE: Added in pass 2.0 */
	uint64_t reserved_6_7                 : 2;
	uint64_t rx_scrm_sync                 : 1;  /**< Rx scrambler synchronization status
                                                         '1' means synchronization achieved

                                                         ***NOTE: Added in pass 2.0 */
	uint64_t rx_bdry_sync                 : 1;  /**< Rx word boundary sync status
                                                         '1' means synchronization achieved */
	uint64_t rx_dis_ukwn                  : 1;  /**< Disable normal response to unknown words.  They are still
                                                         logged but do not cause an error to all open channels. */
	uint64_t rx_dis_scram                 : 1;  /**< Disable lane scrambler (debug) */
	uint64_t stat_rdclr                   : 1;  /**< CSR read to ILK_RX_LNEx_STAT* clears the selected counter after
                                                         returning its current value. */
	uint64_t stat_ena                     : 1;  /**< Enable RX lane statistics counters */
#else
	uint64_t stat_ena                     : 1;
	uint64_t stat_rdclr                   : 1;
	uint64_t rx_dis_scram                 : 1;
	uint64_t rx_dis_ukwn                  : 1;
	uint64_t rx_bdry_sync                 : 1;
	uint64_t rx_scrm_sync                 : 1;
	uint64_t reserved_6_7                 : 2;
	uint64_t rx_dis_psh_skip              : 1;
	uint64_t reserved_9_63                : 55;
#endif
	} cn68xx;
	struct cvmx_ilk_rx_lnex_cfg_cn68xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t rx_bdry_sync                 : 1;  /**< Rx word boundary sync status
                                                         '1' means synchronization achieved */
	uint64_t rx_dis_ukwn                  : 1;  /**< Disable normal response to unknown words.  They are still
                                                         logged but do not cause an error to all open channels */
	uint64_t rx_dis_scram                 : 1;  /**< Disable lane scrambler (debug) */
	uint64_t stat_rdclr                   : 1;  /**< CSR read to ILK_RX_LNEx_STAT* clears the selected counter after
                                                         returning its current value. */
	uint64_t stat_ena                     : 1;  /**< Enable RX lane statistics counters */
#else
	uint64_t stat_ena                     : 1;
	uint64_t stat_rdclr                   : 1;
	uint64_t rx_dis_scram                 : 1;
	uint64_t rx_dis_ukwn                  : 1;
	uint64_t rx_bdry_sync                 : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} cn68xxp1;
	struct cvmx_ilk_rx_lnex_cfg_s         cn78xx;
	struct cvmx_ilk_rx_lnex_cfg_s         cn78xxp1;
};
typedef union cvmx_ilk_rx_lnex_cfg cvmx_ilk_rx_lnex_cfg_t;

/**
 * cvmx_ilk_rx_lne#_int
 */
union cvmx_ilk_rx_lnex_int {
	uint64_t u64;
	struct cvmx_ilk_rx_lnex_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t disp_err                     : 1;  /**< RX disparity error encountered. Throws ILK_INTSN_E::ILK_RXLNE()_DISP_ERR. */
	uint64_t bad_64b67b                   : 1;  /**< Bad 64/67 bit code word encountered. Once the bad word reaches the burst control unit (as
                                                         denoted by ILK_RX()_INT[LANE_BAD_WORD]) it is discarded and all open packets receive
                                                         an error. Throws ILK_INTSN_E::ILK_RXLNE()_BAD_64B67B. */
	uint64_t stat_cnt_ovfl                : 1;  /**< Rx lane statistic counter overflow. Throws ILK_INTSN_E::ILK_RXLNE()_STAT_CNT_OVFL. */
	uint64_t stat_msg                     : 1;  /**< Status bits for the link or a lane transitioned from a 1 (healthy) to a 0 (problem).
                                                         Throws ILK_INTSN_E::ILK_RXLNE()_STAT_MSG. */
	uint64_t dskew_fifo_ovfl              : 1;  /**< RX deskew FIFO overflow occurred. Throws ILK_INTSN_E::ILK_RXLNE()_DSKEW_FIFO_OVFL. */
	uint64_t scrm_sync_loss               : 1;  /**< Four consecutive bad sync words or three consecutive scramble state mismatches. Throws
                                                         ILK_INTSN_E::ILK_RXLNE()_SCRM_SYNC_LOSS. */
	uint64_t ukwn_cntl_word               : 1;  /**< Unknown framing control word. Block type does not match any of (SYNC,SCRAM,SKIP,DIAG).
                                                         Throws ILK_INTSN_E::ILK_RXLNE()_UKWN_CNTL_WORD. */
	uint64_t crc32_err                    : 1;  /**< Diagnostic CRC32 errors. Throws ILK_INTSN_E::ILK_RXLNE()_CRC32_ERR. */
	uint64_t bdry_sync_loss               : 1;  /**< RX logic loses word boundary sync (16 tries). Hardware will automatically attempt to
                                                         regain word boundary sync. Throws ILK_INTSN_E::ILK_RXLNE()_BDRY_SYNC_LOSS. */
	uint64_t serdes_lock_loss             : 1;  /**< RX SerDes loses lock. Throws ILK_INTSN_E::ILK_RXLNE()_SERDES_LOCK_LOSS. */
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
	struct cvmx_ilk_rx_lnex_int_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t bad_64b67b                   : 1;  /**< Bad 64B/67B codeword encountered.  Once the bad word reaches
                                                         the burst control unit (as deonted by
                                                         ILK_RXx_INT[LANE_BAD_WORD]) it will be tossed and all open
                                                         packets will receive an error. */
	uint64_t stat_cnt_ovfl                : 1;  /**< Rx lane statistic counter overflow */
	uint64_t stat_msg                     : 1;  /**< Status bits for the link or a lane transitioned from a '1'
                                                         (healthy) to a '0' (problem) */
	uint64_t dskew_fifo_ovfl              : 1;  /**< Rx deskew fifo overflow occurred. */
	uint64_t scrm_sync_loss               : 1;  /**< 4 consecutive bad sync words or 3 consecutive scramble state
                                                         mismatches */
	uint64_t ukwn_cntl_word               : 1;  /**< Unknown framing control word. Block type does not match any of
                                                         (SYNC,SCRAM,SKIP,DIAG) */
	uint64_t crc32_err                    : 1;  /**< Diagnostic CRC32 errors */
	uint64_t bdry_sync_loss               : 1;  /**< Rx logic loses word boundary sync (16 tries).  Hardware will
                                                         automatically attempt to regain word boundary sync */
	uint64_t serdes_lock_loss             : 1;  /**< Rx SERDES loses lock */
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
	uint64_t reserved_9_63                : 55;
#endif
	} cn68xx;
	struct cvmx_ilk_rx_lnex_int_cn68xx    cn68xxp1;
	struct cvmx_ilk_rx_lnex_int_s         cn78xx;
	struct cvmx_ilk_rx_lnex_int_s         cn78xxp1;
};
typedef union cvmx_ilk_rx_lnex_int cvmx_ilk_rx_lnex_int_t;

/**
 * cvmx_ilk_rx_lne#_int_en
 */
union cvmx_ilk_rx_lnex_int_en {
	uint64_t u64;
	struct cvmx_ilk_rx_lnex_int_en_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t bad_64b67b                   : 1;  /**< Bad 64B/67B codeword encountered.  Once the bad word reaches
                                                         the burst control unit (as deonted by
                                                         ILK_RXx_INT[LANE_BAD_WORD]) it will be tossed and all open
                                                         packets will receive an error. */
	uint64_t stat_cnt_ovfl                : 1;  /**< Rx lane statistic counter overflow */
	uint64_t stat_msg                     : 1;  /**< Status bits for the link or a lane transitioned from a '1'
                                                         (healthy) to a '0' (problem) */
	uint64_t dskew_fifo_ovfl              : 1;  /**< Rx deskew fifo overflow occurred. */
	uint64_t scrm_sync_loss               : 1;  /**< 4 consecutive bad sync words or 3 consecutive scramble state
                                                         mismatches */
	uint64_t ukwn_cntl_word               : 1;  /**< Unknown framing control word. Block type does not match any of
                                                         (SYNC,SCRAM,SKIP,DIAG) */
	uint64_t crc32_err                    : 1;  /**< Diagnostic CRC32 error */
	uint64_t bdry_sync_loss               : 1;  /**< Rx logic loses word boundary sync (16 tries).  Hardware will
                                                         automatically attempt to regain word boundary sync */
	uint64_t serdes_lock_loss             : 1;  /**< Rx SERDES loses lock */
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
	uint64_t reserved_9_63                : 55;
#endif
	} s;
	struct cvmx_ilk_rx_lnex_int_en_s      cn68xx;
	struct cvmx_ilk_rx_lnex_int_en_s      cn68xxp1;
};
typedef union cvmx_ilk_rx_lnex_int_en cvmx_ilk_rx_lnex_int_en_t;

/**
 * cvmx_ilk_rx_lne#_stat0
 */
union cvmx_ilk_rx_lnex_stat0 {
	uint64_t u64;
	struct cvmx_ilk_rx_lnex_stat0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t ser_lock_loss_cnt            : 18; /**< Indicates the number of times the lane lost clock-data-recovery. On overflow, saturates
                                                         and sets ILK_RX_LNE()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t ser_lock_loss_cnt            : 18;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_ilk_rx_lnex_stat0_s       cn68xx;
	struct cvmx_ilk_rx_lnex_stat0_s       cn68xxp1;
	struct cvmx_ilk_rx_lnex_stat0_s       cn78xx;
	struct cvmx_ilk_rx_lnex_stat0_s       cn78xxp1;
};
typedef union cvmx_ilk_rx_lnex_stat0 cvmx_ilk_rx_lnex_stat0_t;

/**
 * cvmx_ilk_rx_lne#_stat1
 */
union cvmx_ilk_rx_lnex_stat1 {
	uint64_t u64;
	struct cvmx_ilk_rx_lnex_stat1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t bdry_sync_loss_cnt           : 18; /**< Indicates the number of times a lane lost word-boundary synchronization. On overflow,
                                                         saturates and sets ILK_RX_LNE()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t bdry_sync_loss_cnt           : 18;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_ilk_rx_lnex_stat1_s       cn68xx;
	struct cvmx_ilk_rx_lnex_stat1_s       cn68xxp1;
	struct cvmx_ilk_rx_lnex_stat1_s       cn78xx;
	struct cvmx_ilk_rx_lnex_stat1_s       cn78xxp1;
};
typedef union cvmx_ilk_rx_lnex_stat1 cvmx_ilk_rx_lnex_stat1_t;

/**
 * cvmx_ilk_rx_lne#_stat10
 */
union cvmx_ilk_rx_lnex_stat10 {
	uint64_t u64;
	struct cvmx_ilk_rx_lnex_stat10_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_43_63               : 21;
	uint64_t prbs_bad                     : 11; /**< Indicates the number of training frames with bad PRBS. On overflow, saturates and sets
                                                         ILK_RX_LNE()_INT[STAT_CNT_OVFL]. */
	uint64_t reserved_11_31               : 21;
	uint64_t prbs_good                    : 11; /**< Indicates the number of training frames with correct PRBS. On overflow, saturates and sets
                                                         ILK_RX_LNE()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t prbs_good                    : 11;
	uint64_t reserved_11_31               : 21;
	uint64_t prbs_bad                     : 11;
	uint64_t reserved_43_63               : 21;
#endif
	} s;
	struct cvmx_ilk_rx_lnex_stat10_s      cn78xx;
	struct cvmx_ilk_rx_lnex_stat10_s      cn78xxp1;
};
typedef union cvmx_ilk_rx_lnex_stat10 cvmx_ilk_rx_lnex_stat10_t;

/**
 * cvmx_ilk_rx_lne#_stat2
 */
union cvmx_ilk_rx_lnex_stat2 {
	uint64_t u64;
	struct cvmx_ilk_rx_lnex_stat2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_50_63               : 14;
	uint64_t syncw_good_cnt               : 18; /**< Indicates the number of good synchronization words. On overflow, saturates and sets
                                                         ILK_RX_LNE()_INT[STAT_CNT_OVFL]. */
	uint64_t reserved_18_31               : 14;
	uint64_t syncw_bad_cnt                : 18; /**< Indicates the number of bad synchronization words. On overflow, saturates and sets
                                                         ILK_RX_LNE()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t syncw_bad_cnt                : 18;
	uint64_t reserved_18_31               : 14;
	uint64_t syncw_good_cnt               : 18;
	uint64_t reserved_50_63               : 14;
#endif
	} s;
	struct cvmx_ilk_rx_lnex_stat2_s       cn68xx;
	struct cvmx_ilk_rx_lnex_stat2_s       cn68xxp1;
	struct cvmx_ilk_rx_lnex_stat2_s       cn78xx;
	struct cvmx_ilk_rx_lnex_stat2_s       cn78xxp1;
};
typedef union cvmx_ilk_rx_lnex_stat2 cvmx_ilk_rx_lnex_stat2_t;

/**
 * cvmx_ilk_rx_lne#_stat3
 */
union cvmx_ilk_rx_lnex_stat3 {
	uint64_t u64;
	struct cvmx_ilk_rx_lnex_stat3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t bad_64b67b_cnt               : 18; /**< Indicates the number of bad 64/67 bit words, meaning bit <65> or bit <64> has been
                                                         corrupted. On overflow, saturates and sets ILK_RX_LNE()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t bad_64b67b_cnt               : 18;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_ilk_rx_lnex_stat3_s       cn68xx;
	struct cvmx_ilk_rx_lnex_stat3_s       cn68xxp1;
	struct cvmx_ilk_rx_lnex_stat3_s       cn78xx;
	struct cvmx_ilk_rx_lnex_stat3_s       cn78xxp1;
};
typedef union cvmx_ilk_rx_lnex_stat3 cvmx_ilk_rx_lnex_stat3_t;

/**
 * cvmx_ilk_rx_lne#_stat4
 */
union cvmx_ilk_rx_lnex_stat4 {
	uint64_t u64;
	struct cvmx_ilk_rx_lnex_stat4_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_59_63               : 5;
	uint64_t cntl_word_cnt                : 27; /**< Indicates the number of control words received. SOn overflow, saturates and sets
                                                         ILK_RX_LNE()_INT[STAT_CNT_OVFL]. */
	uint64_t reserved_27_31               : 5;
	uint64_t data_word_cnt                : 27; /**< Indicates the number of data words received. On overflow, saturates and sets
                                                         ILK_RX_LNE()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t data_word_cnt                : 27;
	uint64_t reserved_27_31               : 5;
	uint64_t cntl_word_cnt                : 27;
	uint64_t reserved_59_63               : 5;
#endif
	} s;
	struct cvmx_ilk_rx_lnex_stat4_s       cn68xx;
	struct cvmx_ilk_rx_lnex_stat4_s       cn68xxp1;
	struct cvmx_ilk_rx_lnex_stat4_s       cn78xx;
	struct cvmx_ilk_rx_lnex_stat4_s       cn78xxp1;
};
typedef union cvmx_ilk_rx_lnex_stat4 cvmx_ilk_rx_lnex_stat4_t;

/**
 * cvmx_ilk_rx_lne#_stat5
 */
union cvmx_ilk_rx_lnex_stat5 {
	uint64_t u64;
	struct cvmx_ilk_rx_lnex_stat5_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t unkwn_word_cnt               : 18; /**< Indicates the number of unknown control words.On overflow, saturates and sets
                                                         ILK_RX_LNE()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t unkwn_word_cnt               : 18;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_ilk_rx_lnex_stat5_s       cn68xx;
	struct cvmx_ilk_rx_lnex_stat5_s       cn68xxp1;
	struct cvmx_ilk_rx_lnex_stat5_s       cn78xx;
	struct cvmx_ilk_rx_lnex_stat5_s       cn78xxp1;
};
typedef union cvmx_ilk_rx_lnex_stat5 cvmx_ilk_rx_lnex_stat5_t;

/**
 * cvmx_ilk_rx_lne#_stat6
 */
union cvmx_ilk_rx_lnex_stat6 {
	uint64_t u64;
	struct cvmx_ilk_rx_lnex_stat6_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t scrm_sync_loss_cnt           : 18; /**< Indicates the number of times scrambler synchronization was lost (due to either four
                                                         consecutive bad sync words or three consecutive scrambler-state mismatches). On overflow,
                                                         saturates and sets ILK_RX_LNE()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t scrm_sync_loss_cnt           : 18;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_ilk_rx_lnex_stat6_s       cn68xx;
	struct cvmx_ilk_rx_lnex_stat6_s       cn68xxp1;
	struct cvmx_ilk_rx_lnex_stat6_s       cn78xx;
	struct cvmx_ilk_rx_lnex_stat6_s       cn78xxp1;
};
typedef union cvmx_ilk_rx_lnex_stat6 cvmx_ilk_rx_lnex_stat6_t;

/**
 * cvmx_ilk_rx_lne#_stat7
 */
union cvmx_ilk_rx_lnex_stat7 {
	uint64_t u64;
	struct cvmx_ilk_rx_lnex_stat7_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t scrm_match_cnt               : 18; /**< Indicates the number of scrambler-state matches received. On overflow, saturates and sets
                                                         ILK_RX_LNE()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t scrm_match_cnt               : 18;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_ilk_rx_lnex_stat7_s       cn68xx;
	struct cvmx_ilk_rx_lnex_stat7_s       cn68xxp1;
	struct cvmx_ilk_rx_lnex_stat7_s       cn78xx;
	struct cvmx_ilk_rx_lnex_stat7_s       cn78xxp1;
};
typedef union cvmx_ilk_rx_lnex_stat7 cvmx_ilk_rx_lnex_stat7_t;

/**
 * cvmx_ilk_rx_lne#_stat8
 */
union cvmx_ilk_rx_lnex_stat8 {
	uint64_t u64;
	struct cvmx_ilk_rx_lnex_stat8_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t skipw_good_cnt               : 18; /**< Indicates the number of good skip words. On overflow, saturates and sets
                                                         ILK_RX_LNE()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t skipw_good_cnt               : 18;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_ilk_rx_lnex_stat8_s       cn68xx;
	struct cvmx_ilk_rx_lnex_stat8_s       cn68xxp1;
	struct cvmx_ilk_rx_lnex_stat8_s       cn78xx;
	struct cvmx_ilk_rx_lnex_stat8_s       cn78xxp1;
};
typedef union cvmx_ilk_rx_lnex_stat8 cvmx_ilk_rx_lnex_stat8_t;

/**
 * cvmx_ilk_rx_lne#_stat9
 */
union cvmx_ilk_rx_lnex_stat9 {
	uint64_t u64;
	struct cvmx_ilk_rx_lnex_stat9_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_50_63               : 14;
	uint64_t crc32_err_cnt                : 18; /**< Indicates the number of errors in the lane CRC. On overflow, saturates and sets
                                                         ILK_RX_LNE()_INT[STAT_CNT_OVFL]. */
	uint64_t reserved_27_31               : 5;
	uint64_t crc32_match_cnt              : 27; /**< Indicates the number of CRC32 matches received. On overflow, saturates and sets
                                                         ILK_RX_LNE()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t crc32_match_cnt              : 27;
	uint64_t reserved_27_31               : 5;
	uint64_t crc32_err_cnt                : 18;
	uint64_t reserved_50_63               : 14;
#endif
	} s;
	struct cvmx_ilk_rx_lnex_stat9_s       cn68xx;
	struct cvmx_ilk_rx_lnex_stat9_s       cn68xxp1;
	struct cvmx_ilk_rx_lnex_stat9_s       cn78xx;
	struct cvmx_ilk_rx_lnex_stat9_s       cn78xxp1;
};
typedef union cvmx_ilk_rx_lnex_stat9 cvmx_ilk_rx_lnex_stat9_t;

/**
 * cvmx_ilk_rxf_idx_pmap
 */
union cvmx_ilk_rxf_idx_pmap {
	uint64_t u64;
	struct cvmx_ilk_rxf_idx_pmap_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_25_63               : 39;
	uint64_t inc                          : 9;  /**< Increment to add to current index for next index. */
	uint64_t reserved_9_15                : 7;
	uint64_t index                        : 9;  /**< Specify the link/channel accessed by the next CSR read/write to
                                                         port map memory.   IDX[8]=link, IDX[7:0]=channel */
#else
	uint64_t index                        : 9;
	uint64_t reserved_9_15                : 7;
	uint64_t inc                          : 9;
	uint64_t reserved_25_63               : 39;
#endif
	} s;
	struct cvmx_ilk_rxf_idx_pmap_s        cn68xx;
	struct cvmx_ilk_rxf_idx_pmap_s        cn68xxp1;
};
typedef union cvmx_ilk_rxf_idx_pmap cvmx_ilk_rxf_idx_pmap_t;

/**
 * cvmx_ilk_rxf_mem_pmap
 */
union cvmx_ilk_rxf_mem_pmap {
	uint64_t u64;
	struct cvmx_ilk_rxf_mem_pmap_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t port_kind                    : 6;  /**< Specify the port-kind for the link/channel selected by
                                                         ILK_RXF_IDX_PMAP[IDX] */
#else
	uint64_t port_kind                    : 6;
	uint64_t reserved_6_63                : 58;
#endif
	} s;
	struct cvmx_ilk_rxf_mem_pmap_s        cn68xx;
	struct cvmx_ilk_rxf_mem_pmap_s        cn68xxp1;
};
typedef union cvmx_ilk_rxf_mem_pmap cvmx_ilk_rxf_mem_pmap_t;

/**
 * cvmx_ilk_ser_cfg
 */
union cvmx_ilk_ser_cfg {
	uint64_t u64;
	struct cvmx_ilk_ser_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_57_63               : 7;
	uint64_t ser_rxpol_auto               : 1;  /**< SerDes lane receive polarity auto detection mode. */
	uint64_t ser_rxpol                    : 16; /**< SerDes lane receive polarity.
                                                         0 = RX without inversion.
                                                         1 = RX with inversion.
                                                         Note that ILK_RX()_CFG0[LANE_REV] has no effect on this mapping.
                                                         _ [SER_RXPOL<0>]  = QLM4 lane 0.
                                                         _ [SER_RXPOL<1>]  = QLM4 lane 1.
                                                         _ [SER_RXPOL<2>]  = QLM4 lane 2.
                                                         _ [SER_RXPOL<3>]  = QLM4 lane 3.
                                                         _ [SER_RXPOL<4>]  = QLM5 lane 0.
                                                         _ [SER_RXPOL<5>]  = QLM5 lane 1.
                                                         _ [SER_RXPOL<6>]  = QLM5 lane 2.
                                                         _ [SER_RXPOL<7>]  = QLM5 lane 3.
                                                         _ [SER_RXPOL<8>]  = QLM6 lane 0.
                                                         _ [SER_RXPOL<9>]  = QLM6 lane 1.
                                                         _ [SER_RXPOL<10>] = QLM6 lane 2.
                                                         _ [SER_RXPOL<11>] = QLM6 lane 3.
                                                         _ [SER_RXPOL<12>] = QLM7 lane 0.
                                                         _ [SER_RXPOL<13>] = QLM7 lane 1.
                                                         _ [SER_RXPOL<14>] = QLM7 lane 2.
                                                         _ [SER_RXPOL<15>] = QLM7 lane 3. */
	uint64_t ser_txpol                    : 16; /**< SerDes lane transmit polarity.
                                                         0 = TX without inversion.
                                                         1 = TX with inversion.
                                                         Note that ILK_TX()_CFG0[LANE_REV] has no effect on this mapping.
                                                         _ [SER_TXPOL<0>]  = QLM4 lane 0.
                                                         _ [SER_TXPOL<1>]  = QLM4 lane 1.
                                                         _ [SER_TXPOL<2>]  = QLM4 lane 2.
                                                         _ [SER_TXPOL<3>]  = QLM4 lane 3.
                                                         _ [SER_TXPOL<4>]  = QLM5 lane 0.
                                                         _ [SER_TXPOL<5>]  = QLM5 lane 1.
                                                         _ [SER_TXPOL<6>]  = QLM5 lane 2.
                                                         _ [SER_TXPOL<7>]  = QLM5 lane 3.
                                                         _ [SER_TXPOL<8>]  = QLM6 lane 0.
                                                         _ [SER_TXPOL<9>]  = QLM6 lane 1.
                                                         _ [SER_TXPOL<10>] = QLM6 lane 2.
                                                         _ [SER_TXPOL<11>] = QLM6 lane 3.
                                                         _ [SER_TXPOL<12>] = QLM7 lane 0.
                                                         _ [SER_TXPOL<13>] = QLM7 lane 1.
                                                         _ [SER_TXPOL<14>] = QLM7 lane 2.
                                                         _ [SER_TXPOL<15>] = QLM7 lane 3. */
	uint64_t ser_reset_n                  : 16; /**< SerDes lane reset. Should be set when the GSER is ready to transfer data, as indicated
                                                         by the corresponding GSER()_QLM_STAT[RST_RDY]. Note that
                                                         neither ILK_TX()_CFG0[LANE_REV] nor ILK_RX()_CFG0[LANE_REV] has an effect on this mapping.
                                                         The correlation of [SER_RESET_N] bits to GSER's is as follows:
                                                         _ [SER_RESET_N<0>]  = QLM4 lane 0, GSER(4)_QLM_STAT[RST_RDY].
                                                         _ [SER_RESET_N<1>]  = QLM4 lane 1, GSER(4)_QLM_STAT[RST_RDY].
                                                         _ [SER_RESET_N<2>]  = QLM4 lane 2, GSER(4)_QLM_STAT[RST_RDY].
                                                         _ [SER_RESET_N<3>]  = QLM4 lane 3, GSER(4)_QLM_STAT[RST_RDY].
                                                         _ [SER_RESET_N<4>]  = QLM5 lane 0, GSER(5)_QLM_STAT[RST_RDY].
                                                         _ [SER_RESET_N<5>]  = QLM5 lane 1, GSER(5)_QLM_STAT[RST_RDY].
                                                         _ [SER_RESET_N<6>]  = QLM5 lane 2, GSER(5)_QLM_STAT[RST_RDY].
                                                         _ [SER_RESET_N<7>]  = QLM5 lane 3, GSER(5)_QLM_STAT[RST_RDY].
                                                         _ [SER_RESET_N<8>]  = QLM6 lane 0, GSER(6)_QLM_STAT[RST_RDY].
                                                         _ [SER_RESET_N<9>]  = QLM6 lane 1, GSER(6)_QLM_STAT[RST_RDY].
                                                         _ [SER_RESET_N<10>] = QLM6 lane 2, GSER(6)_QLM_STAT[RST_RDY].
                                                         _ [SER_RESET_N<11>] = QLM6 lane 3, GSER(6)_QLM_STAT[RST_RDY].
                                                         _ [SER_RESET_N<12>] = QLM7 lane 0, GSER(7)_QLM_STAT[RST_RDY].
                                                         _ [SER_RESET_N<13>] = QLM7 lane 1, GSER(7)_QLM_STAT[RST_RDY].
                                                         _ [SER_RESET_N<14>] = QLM7 lane 2, GSER(7)_QLM_STAT[RST_RDY].
                                                         _ [SER_RESET_N<15>] = QLM7 lane 3, GSER(7)_QLM_STAT[RST_RDY]. */
	uint64_t ser_pwrup                    : 4;  /**< Reserved. */
	uint64_t ser_haul                     : 4;  /**< Reserved. */
#else
	uint64_t ser_haul                     : 4;
	uint64_t ser_pwrup                    : 4;
	uint64_t ser_reset_n                  : 16;
	uint64_t ser_txpol                    : 16;
	uint64_t ser_rxpol                    : 16;
	uint64_t ser_rxpol_auto               : 1;
	uint64_t reserved_57_63               : 7;
#endif
	} s;
	struct cvmx_ilk_ser_cfg_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_57_63               : 7;
	uint64_t ser_rxpol_auto               : 1;  /**< Serdes lane receive polarity auto detection mode */
	uint64_t reserved_48_55               : 8;
	uint64_t ser_rxpol                    : 8;  /**< Serdes lane receive polarity
                                                         - 0: rx without inversion
                                                         - 1: rx with inversion */
	uint64_t reserved_32_39               : 8;
	uint64_t ser_txpol                    : 8;  /**< Serdes lane transmit polarity
                                                         - 0: tx without inversion
                                                         - 1: tx with inversion */
	uint64_t reserved_16_23               : 8;
	uint64_t ser_reset_n                  : 8;  /**< Serdes lane reset */
	uint64_t reserved_6_7                 : 2;
	uint64_t ser_pwrup                    : 2;  /**< Serdes modules (QLM) power up. */
	uint64_t reserved_2_3                 : 2;
	uint64_t ser_haul                     : 2;  /**< Serdes module (QLM) haul mode */
#else
	uint64_t ser_haul                     : 2;
	uint64_t reserved_2_3                 : 2;
	uint64_t ser_pwrup                    : 2;
	uint64_t reserved_6_7                 : 2;
	uint64_t ser_reset_n                  : 8;
	uint64_t reserved_16_23               : 8;
	uint64_t ser_txpol                    : 8;
	uint64_t reserved_32_39               : 8;
	uint64_t ser_rxpol                    : 8;
	uint64_t reserved_48_55               : 8;
	uint64_t ser_rxpol_auto               : 1;
	uint64_t reserved_57_63               : 7;
#endif
	} cn68xx;
	struct cvmx_ilk_ser_cfg_cn68xx        cn68xxp1;
	struct cvmx_ilk_ser_cfg_s             cn78xx;
	struct cvmx_ilk_ser_cfg_s             cn78xxp1;
};
typedef union cvmx_ilk_ser_cfg cvmx_ilk_ser_cfg_t;

/**
 * cvmx_ilk_tx#_byte_cnt#
 */
union cvmx_ilk_txx_byte_cntx {
	uint64_t u64;
	struct cvmx_ilk_txx_byte_cntx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_40_63               : 24;
	uint64_t tx_bytes                     : 40; /**< Number of bytes transmitted per channel. Wraps on overflow. On overflow, sets
                                                         ILK_TX()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t tx_bytes                     : 40;
	uint64_t reserved_40_63               : 24;
#endif
	} s;
	struct cvmx_ilk_txx_byte_cntx_s       cn78xx;
	struct cvmx_ilk_txx_byte_cntx_s       cn78xxp1;
};
typedef union cvmx_ilk_txx_byte_cntx cvmx_ilk_txx_byte_cntx_t;

/**
 * cvmx_ilk_tx#_cal_entry#
 */
union cvmx_ilk_txx_cal_entryx {
	uint64_t u64;
	struct cvmx_ilk_txx_cal_entryx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_34_63               : 30;
	uint64_t ctl                          : 2;  /**< Select source of XON/XOFF for entry (IDX * 8) + 0:
                                                         0 = PKI backpressure channel.
                                                         1 = Link.
                                                         2 = XOFF.
                                                         3 = XON.
                                                         This field applies to one of bits <55>, <47>, or <31> in the Interlaken control word. */
	uint64_t reserved_8_31                : 24;
	uint64_t channel                      : 8;  /**< PKI channel for the calendar table entry. Unused if CTL != 0. */
#else
	uint64_t channel                      : 8;
	uint64_t reserved_8_31                : 24;
	uint64_t ctl                          : 2;
	uint64_t reserved_34_63               : 30;
#endif
	} s;
	struct cvmx_ilk_txx_cal_entryx_s      cn78xx;
	struct cvmx_ilk_txx_cal_entryx_s      cn78xxp1;
};
typedef union cvmx_ilk_txx_cal_entryx cvmx_ilk_txx_cal_entryx_t;

/**
 * cvmx_ilk_tx#_cfg0
 */
union cvmx_ilk_txx_cfg0 {
	uint64_t u64;
	struct cvmx_ilk_txx_cfg0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ext_lpbk_fc                  : 1;  /**< Enable RX-TX flow-control external loopback. */
	uint64_t ext_lpbk                     : 1;  /**< Enable RX-TX data external loopback. Note that with differing transmit and receive clocks,
                                                         skip word are inserted/deleted */
	uint64_t int_lpbk                     : 1;  /**< Enable TX-RX internal loopback. */
	uint64_t txf_byp_dis                  : 1;  /**< Disable TXF bypass. */
	uint64_t reserved_57_59               : 3;
	uint64_t ptrn_mode                    : 1;  /**< Reserved. */
	uint64_t lnk_stats_rdclr              : 1;  /**< CSR read to ILK_TXx_PKT_CNT or ILK_TXx_BYTE_CNT clears the counter after returning its
                                                         current value. */
	uint64_t lnk_stats_ena                : 1;  /**< Enable link statistics counters. */
	uint64_t mltuse_fc_ena                : 1;  /**< When set, the multiuse field of control words contains flow-control status. Otherwise, the
                                                         multiuse field contains ILK_TX()_CFG1[TX_MLTUSE].   This field must not be changed unless
                                                         ILK_TX()_CFG0.LANE_ENA=0.  Setting ILK_TX()_CFG0.MLTUSE_FC_ENA=1 requires
                                                         ILK_TX()_CFG0.CAL_ENA=1. */
	uint64_t cal_ena                      : 1;  /**< Enable TX calendar. When not asserted, the default calendar is used:
                                                         First control word:
                                                         _ entry 0 = link.
                                                         _ entry 1 = backpressure ID 0.
                                                         _ entry 2 = backpressure ID 1.
                                                         _ ...
                                                         _ entry 15 = backpressure ID 14.
                                                         Second control word:
                                                         _ entry 16 = link.
                                                         _ entry 17 = backpressure ID 15.
                                                         _ entry 18 = backpressure ID 16.
                                                         _ ...
                                                         This continues until the calendar depth is reached.
                                                         To disable backpressure completely, enable the calendar table and program each calendar
                                                         table entry to transmit XON. */
	uint64_t mfrm_len                     : 13; /**< The quantity of data sent on each lane including one sync word, scrambler state, diag
                                                         word, zero or more skip words, and the data payload. Must be large than
                                                         _ ILK_TX()_CFG1[SKIP_CNT] + 32.
                                                         Supported range:
                                                         _ ILK_TX()_CFG1[SKIP_CNT] + 32 < [MFRM_LEN] <= 4096 */
	uint64_t brst_shrt                    : 7;  /**< Minimum interval between burst control words, as a multiple of eight bytes. Supported
                                                         range from eight to 512 bytes (i.e. 0 < [BRST_SHRT] <= 64). */
	uint64_t lane_rev                     : 1;  /**< Lane reversal. When enabled, lane striping is performed from most significant lane enabled
                                                         to least significant lane enabled. [LANE_ENA] must be zero before changing [LANE_REV]. */
	uint64_t brst_max                     : 5;  /**< Maximum size of a data burst, as a multiple of 64-byte blocks. Supported range is from 64
                                                         to 1024 bytes (i.e. 0 < [BRST_MAX] <= 16). */
	uint64_t reserved_25_25               : 1;
	uint64_t cal_depth                    : 9;  /**< Number of valid entries in the calendar. [CAL_DEPTH][2:0] must be zero. Supported range is
                                                         from 0 to 288.
                                                         If [CAL_DEPTH] = 0x0, the calendar is completely disabled and all transmit flow control
                                                         status is XOFF. */
	uint64_t lane_ena                     : 16; /**< Lane enable mask. Link is enabled if any lane is enabled. The same lane should not be
                                                         enabled in multiple
                                                         ILK_TX0/1_CFG0. Each bit of [LANE_ENA] maps to a TX lane (TLE) and a QLM lane. Note that
                                                         [LANE_REV] has no effect on this mapping.
                                                         _ [LANE_ENA<0>]  = TLE0   =  QLM4 lane 0.
                                                         _ [LANE_ENA<1>]  = TLE1   =  QLM4 lane 1.
                                                         _ [LANE_ENA<2>]  = TLE2   =  QLM4 lane 2.
                                                         _ [LANE_ENA<3>]  = TLE3   =  QLM4 lane 3.
                                                         _ [LANE_ENA<4>]  = TLE4   =  QLM5 lane 0.
                                                         _ [LANE_ENA<5>]  = TLE5   =  QLM5 lane 1.
                                                         _ [LANE_ENA<6>]  = TLE6   =  QLM5 lane 2.
                                                         _ [LANE_ENA<7>]  = TLE7   =  QLM5 lane 3.
                                                         _ [LANE_ENA<8>]  = TLE8   =  QLM6 lane 0.
                                                         _ [LANE_ENA<9>]  = TLE9   =  QLM6 lane 1.
                                                         _ [LANE_ENA<10>] = TLE10  =  QLM6 lane 2.
                                                         _ [LANE_ENA<11>] = TLE11  =  QLM6 lane 3.
                                                         _ [LANE_ENA<12>] = TLE12  =  QLM7 lane 0.
                                                         _ [LANE_ENA<13>] = TLE13  =  QLM7 lane 1.
                                                         _ [LANE_ENA<14>] = TLE14  =  QLM7 lane 2.
                                                         _ [LANE_ENA<15>] = TLE15  =  QLM7 lane 3. */
#else
	uint64_t lane_ena                     : 16;
	uint64_t cal_depth                    : 9;
	uint64_t reserved_25_25               : 1;
	uint64_t brst_max                     : 5;
	uint64_t lane_rev                     : 1;
	uint64_t brst_shrt                    : 7;
	uint64_t mfrm_len                     : 13;
	uint64_t cal_ena                      : 1;
	uint64_t mltuse_fc_ena                : 1;
	uint64_t lnk_stats_ena                : 1;
	uint64_t lnk_stats_rdclr              : 1;
	uint64_t ptrn_mode                    : 1;
	uint64_t reserved_57_59               : 3;
	uint64_t txf_byp_dis                  : 1;
	uint64_t int_lpbk                     : 1;
	uint64_t ext_lpbk                     : 1;
	uint64_t ext_lpbk_fc                  : 1;
#endif
	} s;
	struct cvmx_ilk_txx_cfg0_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ext_lpbk_fc                  : 1;  /**< Enable Rx-Tx flowcontrol loopback (external) */
	uint64_t ext_lpbk                     : 1;  /**< Enable Rx-Tx data loopback (external). Note that with differing
                                                         transmit & receive clocks, skip word are  inserted/deleted */
	uint64_t int_lpbk                     : 1;  /**< Enable Tx-Rx loopback (internal) */
	uint64_t reserved_57_60               : 4;
	uint64_t ptrn_mode                    : 1;  /**< Enable programmable test pattern mode.  This mode allows
                                                         software to send a packet containing a programmable pattern.
                                                         While in this mode, the scramblers and disparity inversion will
                                                         be disabled.  In addition, no framing layer control words will
                                                         be transmitted (ie. no SYNC, scrambler state, skip, or
                                                         diagnostic words will be transmitted).

                                                         NOTE: Software must first write ILK_TXX_CFG0[LANE_ENA]=0 before
                                                         enabling/disabling this mode. */
	uint64_t reserved_55_55               : 1;
	uint64_t lnk_stats_ena                : 1;  /**< Enable link statistics counters */
	uint64_t mltuse_fc_ena                : 1;  /**< When set, the multi-use field of control words will contain
                                                         flow control status.  Otherwise, the multi-use field will
                                                         contain ILK_TXX_CFG1[TX_MLTUSE] */
	uint64_t cal_ena                      : 1;  /**< Enable Tx calendar, else default calendar used:
                                                              First control word:
                                                               Entry 0  = link
                                                               Entry 1  = backpressue id 0
                                                               Entry 2  = backpressue id 1
                                                               ...etc.
                                                            Second control word:
                                                               Entry 16 = link
                                                               Entry 17 = backpressue id 15
                                                               Entry 18 = backpressue id 16
                                                               ...etc.
                                                         This continues until the status for all 64 backpressue ids gets
                                                         transmitted (ie. 0-68 calendar table entries).  The remaining 3
                                                         calendar table entries (ie. 69-71) will always transmit XOFF.

                                                         To disable backpressure completely, enable the calendar table
                                                         and program each calendar table entry to transmit XON */
	uint64_t mfrm_len                     : 13; /**< The quantity of data sent on each lane including one sync word,
                                                         scrambler state, diag word, zero or more skip words, and the
                                                         data  payload.  Must be large than ILK_TXX_CFG1[SKIP_CNT]+9.
                                                         Supported range:ILK_TXX_CFG1[SKIP_CNT]+9 < MFRM_LEN <= 4096) */
	uint64_t brst_shrt                    : 7;  /**< Minimum interval between burst control words, as a multiple of
                                                         8 bytes.  Supported range from 8 bytes to 512 (ie. 0 <
                                                         BRST_SHRT <= 64) */
	uint64_t lane_rev                     : 1;  /**< Lane reversal.   When enabled, lane striping is performed from
                                                         most significant lane enabled to least significant lane
                                                         enabled.  LANE_ENA must be zero before changing LANE_REV. */
	uint64_t brst_max                     : 5;  /**< Maximum size of a data burst, as a multiple of 64 byte blocks.
                                                         Supported range is from 64 bytes to 1024 bytes. (ie. 0 <
                                                         BRST_MAX <= 16) */
	uint64_t reserved_25_25               : 1;
	uint64_t cal_depth                    : 9;  /**< Number of valid entries in the calendar.  CAL_DEPTH[2:0] must
                                                         be zero.  Supported range from 8 to 288.  If CAL_ENA is 0,
                                                         this field has no effect and the calendar depth is 72 entries. */
	uint64_t reserved_8_15                : 8;
	uint64_t lane_ena                     : 8;  /**< Lane enable mask.  Link is enabled if any lane is enabled.  The
                                                         same lane should not be enabled in multiple ILK_TXx_CFG0.  Each
                                                         bit of LANE_ENA maps to a TX lane (TLE) and a QLM lane.  NOTE:
                                                         LANE_REV has no effect on this mapping.

                                                               LANE_ENA[0] = TLE0 = QLM1 lane 0
                                                               LANE_ENA[1] = TLE1 = QLM1 lane 1
                                                               LANE_ENA[2] = TLE2 = QLM1 lane 2
                                                               LANE_ENA[3] = TLE3 = QLM1 lane 3
                                                               LANE_ENA[4] = TLE4 = QLM2 lane 0
                                                               LANE_ENA[5] = TLE5 = QLM2 lane 1
                                                               LANE_ENA[6] = TLE6 = QLM2 lane 2
                                                               LANE_ENA[7] = TLE7 = QLM2 lane 3 */
#else
	uint64_t lane_ena                     : 8;
	uint64_t reserved_8_15                : 8;
	uint64_t cal_depth                    : 9;
	uint64_t reserved_25_25               : 1;
	uint64_t brst_max                     : 5;
	uint64_t lane_rev                     : 1;
	uint64_t brst_shrt                    : 7;
	uint64_t mfrm_len                     : 13;
	uint64_t cal_ena                      : 1;
	uint64_t mltuse_fc_ena                : 1;
	uint64_t lnk_stats_ena                : 1;
	uint64_t reserved_55_55               : 1;
	uint64_t ptrn_mode                    : 1;
	uint64_t reserved_57_60               : 4;
	uint64_t int_lpbk                     : 1;
	uint64_t ext_lpbk                     : 1;
	uint64_t ext_lpbk_fc                  : 1;
#endif
	} cn68xx;
	struct cvmx_ilk_txx_cfg0_cn68xx       cn68xxp1;
	struct cvmx_ilk_txx_cfg0_s            cn78xx;
	struct cvmx_ilk_txx_cfg0_s            cn78xxp1;
};
typedef union cvmx_ilk_txx_cfg0 cvmx_ilk_txx_cfg0_t;

/**
 * cvmx_ilk_tx#_cfg1
 */
union cvmx_ilk_txx_cfg1 {
	uint64_t u64;
	struct cvmx_ilk_txx_cfg1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ser_low                      : 4;  /**< Reserved. */
	uint64_t reserved_53_59               : 7;
	uint64_t brst_min                     : 5;  /**< Minimum size of a data burst, as a multiple of 32-byte blocks. 0 disables the scheduling
                                                         enhancement. When nonzero, must satisfy:
                                                         _ (BRST_SHRT*8) <= (BRST_MIN*32) <= (BRST_MAX*64)/2. */
	uint64_t reserved_43_47               : 5;
	uint64_t ser_limit                    : 10; /**< Reduce latency by limiting the amount of data in flight for each SerDes. If 0x0, hardware
                                                         will compute it. Otherwise, SER_LIMIT must be set as follows:
                                                         _ SER_LIMIT >= 148 + (BAUD / SCLK) * (12 + (NUM_LANES/2))
                                                         For instance, for sclk=1.1GHz,BAUD=10.3125,NUM_LANES=16 :
                                                         _ SER_LIMIT >= 148 + (10.3125 / 1.1 * (12 + (12/2))
                                                         _ SER_LIMIT >= 317 */
	uint64_t pkt_busy                     : 1;  /**< Packet busy. When [PKT_ENA]=0, [PKT_BUSY]=1 indicates the TX-link is
                                                         transmitting data. When [PKT_ENA]=1, [PKT_BUSY] is undefined. */
	uint64_t pipe_crd_dis                 : 1;  /**< Disable channel credits. Should be set to 1 when PKO is configured to ignore channel credits. */
	uint64_t ptp_delay                    : 5;  /**< Reserved. */
	uint64_t skip_cnt                     : 4;  /**< Number of skip words to insert after the scrambler state. */
	uint64_t pkt_flush                    : 1;  /**< Packet transmit flush. When asserted, the TxFIFO continuously drains; all data is dropped.
                                                         Software should first write [PKT_ENA] = 0 and wait for [PKT_BUSY] = 0. */
	uint64_t pkt_ena                      : 1;  /**< Packet transmit enable. When asserted, the TX-link stops transmitting packets, as per
                                                         [RX_LINK_FC_PKT]. */
	uint64_t la_mode                      : 1;  /**< Reserved. */
	uint64_t tx_link_fc                   : 1;  /**< Link flow-control status transmitted by the TX-link. XON (=1) when RX_FIFO_CNT <=
                                                         RX_FIFO_HWM and lane alignment is done */
	uint64_t rx_link_fc                   : 1;  /**< Link flow-control status received in burst/idle control words. When [RX_LINK_FC_IGN] = 0,
                                                         XOFF (=0) causes TX-link to stop transmitting on all channels. */
	uint64_t reserved_12_16               : 5;
	uint64_t tx_link_fc_jam               : 1;  /**< All flow-control transmitted in burst/idle control words are XOFF whenever TX_LINK_FC = 0
                                                         (XOFF). Assert this field to allow link XOFF to automatically XOFF all channels. */
	uint64_t rx_link_fc_pkt               : 1;  /**< Link flow-control received in burst/idle control words causes TX-link to stop transmitting
                                                         at the end of a packet instead of
                                                         the end of a burst. */
	uint64_t rx_link_fc_ign               : 1;  /**< Ignore the link flow-control status received in burst/idle control words */
	uint64_t rmatch                       : 1;  /**< Reserved. */
	uint64_t tx_mltuse                    : 8;  /**< Multiuse bits are used when ILK_TX()_CFG0[MLTUSE_FC_ENA] = 0. */
#else
	uint64_t tx_mltuse                    : 8;
	uint64_t rmatch                       : 1;
	uint64_t rx_link_fc_ign               : 1;
	uint64_t rx_link_fc_pkt               : 1;
	uint64_t tx_link_fc_jam               : 1;
	uint64_t reserved_12_16               : 5;
	uint64_t rx_link_fc                   : 1;
	uint64_t tx_link_fc                   : 1;
	uint64_t la_mode                      : 1;
	uint64_t pkt_ena                      : 1;
	uint64_t pkt_flush                    : 1;
	uint64_t skip_cnt                     : 4;
	uint64_t ptp_delay                    : 5;
	uint64_t pipe_crd_dis                 : 1;
	uint64_t pkt_busy                     : 1;
	uint64_t ser_limit                    : 10;
	uint64_t reserved_43_47               : 5;
	uint64_t brst_min                     : 5;
	uint64_t reserved_53_59               : 7;
	uint64_t ser_low                      : 4;
#endif
	} s;
	struct cvmx_ilk_txx_cfg1_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_33_63               : 31;
	uint64_t pkt_busy                     : 1;  /**< Tx-Link is transmitting data. */
	uint64_t pipe_crd_dis                 : 1;  /**< Disable pipe credits.   Should be set when PKO is configure to
                                                         ignore pipe credits. */
	uint64_t ptp_delay                    : 5;  /**< Timestamp commit delay.  Must not be zero. */
	uint64_t skip_cnt                     : 4;  /**< Number of skip words to insert after the scrambler state */
	uint64_t pkt_flush                    : 1;  /**< Packet transmit flush.  While PKT_FLUSH=1, the TxFifo will
                                                         continuously drain; all data will be dropped.  Software should
                                                         first write PKT_ENA=0 and wait for PKT_BUSY=0. */
	uint64_t pkt_ena                      : 1;  /**< Packet transmit enable.  When PKT_ENA=0, the Tx-Link will stop
                                                         transmitting packets, as per RX_LINK_FC_PKT */
	uint64_t la_mode                      : 1;  /**< Enable Interlaken Look-Aside traffic.   When LA_MODE=1 and
                                                         REMAP=1 for a given port-pipe, bit[39] of the 8-byte header
                                                         specifies the packet should be decoded as an LA packet. */
	uint64_t tx_link_fc                   : 1;  /**< Link flow control status transmitted by the Tx-Link
                                                         XON (=1) when RX_FIFO_CNT <= RX_FIFO_HWM and lane alignment is
                                                         done */
	uint64_t rx_link_fc                   : 1;  /**< Link flow control status received in burst/idle control words.
                                                         When RX_LINK_FC_IGN=0, XOFF (=0) will cause Tx-Link to stop
                                                         transmitting on all channels. */
	uint64_t reserved_12_16               : 5;
	uint64_t tx_link_fc_jam               : 1;  /**< All flow control transmitted in burst/idle control words will
                                                         be XOFF whenever TX_LINK_FC is XOFF.   Enable this to allow
                                                         link XOFF to automatically XOFF all channels. */
	uint64_t rx_link_fc_pkt               : 1;  /**< Link flow control received in burst/idle control words causes
                                                         Tx-Link to stop transmitting at the end of a packet instead of
                                                         the end of a burst */
	uint64_t rx_link_fc_ign               : 1;  /**< Ignore the link flow control status received in burst/idle
                                                         control words */
	uint64_t rmatch                       : 1;  /**< Enable rate matching circuitry */
	uint64_t tx_mltuse                    : 8;  /**< Multiple Use bits used when ILKx_TX_CFG[LA_MODE=0] and
                                                         ILKx_TX_CFG[MLTUSE_FC_ENA] is zero */
#else
	uint64_t tx_mltuse                    : 8;
	uint64_t rmatch                       : 1;
	uint64_t rx_link_fc_ign               : 1;
	uint64_t rx_link_fc_pkt               : 1;
	uint64_t tx_link_fc_jam               : 1;
	uint64_t reserved_12_16               : 5;
	uint64_t rx_link_fc                   : 1;
	uint64_t tx_link_fc                   : 1;
	uint64_t la_mode                      : 1;
	uint64_t pkt_ena                      : 1;
	uint64_t pkt_flush                    : 1;
	uint64_t skip_cnt                     : 4;
	uint64_t ptp_delay                    : 5;
	uint64_t pipe_crd_dis                 : 1;
	uint64_t pkt_busy                     : 1;
	uint64_t reserved_33_63               : 31;
#endif
	} cn68xx;
	struct cvmx_ilk_txx_cfg1_cn68xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t pipe_crd_dis                 : 1;  /**< Disable pipe credits.   Should be set when PKO is configure to
                                                         ignore pipe credits. */
	uint64_t ptp_delay                    : 5;  /**< Timestamp commit delay.  Must not be zero. */
	uint64_t skip_cnt                     : 4;  /**< Number of skip words to insert after the scrambler state */
	uint64_t pkt_flush                    : 1;  /**< Packet transmit flush.  While PKT_FLUSH=1, the TxFifo will
                                                         continuously drain; all data will be dropped.  Software should
                                                         first write PKT_ENA=0 and wait packet transmission to stop. */
	uint64_t pkt_ena                      : 1;  /**< Packet transmit enable.  When PKT_ENA=0, the Tx-Link will stop
                                                         transmitting packets, as per RX_LINK_FC_PKT */
	uint64_t la_mode                      : 1;  /**< 0 = Interlaken
                                                         1 = Interlaken Look-Aside */
	uint64_t tx_link_fc                   : 1;  /**< Link flow control status transmitted by the Tx-Link
                                                         XON (=1) when RX_FIFO_CNT <= RX_FIFO_HWM and lane alignment is
                                                         done */
	uint64_t rx_link_fc                   : 1;  /**< Link flow control status received in burst/idle control words.
                                                         When RX_LINK_FC_IGN=0, XOFF (=0) will cause Tx-Link to stop
                                                         transmitting on all channels. */
	uint64_t reserved_12_16               : 5;
	uint64_t tx_link_fc_jam               : 1;  /**< All flow control transmitted in burst/idle control words will
                                                         be XOFF whenever TX_LINK_FC is XOFF.   Enable this to allow
                                                         link XOFF to automatically XOFF all channels. */
	uint64_t rx_link_fc_pkt               : 1;  /**< Link flow control received in burst/idle control words causes
                                                         Tx-Link to stop transmitting at the end of a packet instead of
                                                         the end of a burst */
	uint64_t rx_link_fc_ign               : 1;  /**< Ignore the link flow control status received in burst/idle
                                                         control words */
	uint64_t rmatch                       : 1;  /**< Enable rate matching circuitry */
	uint64_t tx_mltuse                    : 8;  /**< Multiple Use bits used when ILKx_TX_CFG[LA_MODE=0] and
                                                         ILKx_TX_CFG[MLTUSE_FC_ENA] is zero */
#else
	uint64_t tx_mltuse                    : 8;
	uint64_t rmatch                       : 1;
	uint64_t rx_link_fc_ign               : 1;
	uint64_t rx_link_fc_pkt               : 1;
	uint64_t tx_link_fc_jam               : 1;
	uint64_t reserved_12_16               : 5;
	uint64_t rx_link_fc                   : 1;
	uint64_t tx_link_fc                   : 1;
	uint64_t la_mode                      : 1;
	uint64_t pkt_ena                      : 1;
	uint64_t pkt_flush                    : 1;
	uint64_t skip_cnt                     : 4;
	uint64_t ptp_delay                    : 5;
	uint64_t pipe_crd_dis                 : 1;
	uint64_t reserved_32_63               : 32;
#endif
	} cn68xxp1;
	struct cvmx_ilk_txx_cfg1_s            cn78xx;
	struct cvmx_ilk_txx_cfg1_s            cn78xxp1;
};
typedef union cvmx_ilk_txx_cfg1 cvmx_ilk_txx_cfg1_t;

/**
 * cvmx_ilk_tx#_cha_xon#
 */
union cvmx_ilk_txx_cha_xonx {
	uint64_t u64;
	struct cvmx_ilk_txx_cha_xonx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t status                       : 64; /**< PKI flow control status for backpressure ID 255-0, where a 0 indicates the presence of
                                                         backpressure (i.e. XOFF) and 1 indicates the absence of backpressure (i.e. XON).
                                                         _ ILK_TX(0..1)_CHA_XON[0] -- Channels 63-0.
                                                         _ ILK_TX(0..1)_CHA_XON[1] -- Channels 127-64.
                                                         _ ILK_TX(0..1)_CHA_XON[2] -- Channels 191-128.
                                                         _ ILK_TX(0..1)_CHA_XON[3] -- Channels 255-192. */
#else
	uint64_t status                       : 64;
#endif
	} s;
	struct cvmx_ilk_txx_cha_xonx_s        cn78xx;
	struct cvmx_ilk_txx_cha_xonx_s        cn78xxp1;
};
typedef union cvmx_ilk_txx_cha_xonx cvmx_ilk_txx_cha_xonx_t;

/**
 * cvmx_ilk_tx#_dbg
 */
union cvmx_ilk_txx_dbg {
	uint64_t u64;
	struct cvmx_ilk_txx_dbg_s {
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
	struct cvmx_ilk_txx_dbg_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t tx_bad_crc24                 : 1;  /**< Send a control word with bad CRC24.  Hardware will clear this
                                                         field once the injection is performed. */
	uint64_t tx_bad_ctlw2                 : 1;  /**< Send a control word without the control bit set */
	uint64_t tx_bad_ctlw1                 : 1;  /**< Send a data word with the control bit set */
#else
	uint64_t tx_bad_ctlw1                 : 1;
	uint64_t tx_bad_ctlw2                 : 1;
	uint64_t tx_bad_crc24                 : 1;
	uint64_t reserved_3_63                : 61;
#endif
	} cn68xx;
	struct cvmx_ilk_txx_dbg_cn68xx        cn68xxp1;
	struct cvmx_ilk_txx_dbg_s             cn78xx;
	struct cvmx_ilk_txx_dbg_s             cn78xxp1;
};
typedef union cvmx_ilk_txx_dbg cvmx_ilk_txx_dbg_t;

/**
 * cvmx_ilk_tx#_err_cfg
 */
union cvmx_ilk_txx_err_cfg {
	uint64_t u64;
	struct cvmx_ilk_txx_err_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t fwc_flip                     : 2;  /**< Testing feature. Flip syndrome bits <1:0> on writes to the FWC RAM to test single-bit or
                                                         double-bit errors. */
	uint64_t txf_flip                     : 2;  /**< Testing feature. Flip syndrome bits <1:0> on writes to the TXF RAM to test single-bit or
                                                         double-bit errors. */
	uint64_t reserved_2_15                : 14;
	uint64_t fwc_cor_dis                  : 1;  /**< Disable ECC corrector on FWC. */
	uint64_t txf_cor_dis                  : 1;  /**< Disable ECC corrector on TXF. */
#else
	uint64_t txf_cor_dis                  : 1;
	uint64_t fwc_cor_dis                  : 1;
	uint64_t reserved_2_15                : 14;
	uint64_t txf_flip                     : 2;
	uint64_t fwc_flip                     : 2;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_ilk_txx_err_cfg_s         cn78xx;
	struct cvmx_ilk_txx_err_cfg_s         cn78xxp1;
};
typedef union cvmx_ilk_txx_err_cfg cvmx_ilk_txx_err_cfg_t;

/**
 * cvmx_ilk_tx#_flow_ctl0
 */
union cvmx_ilk_txx_flow_ctl0 {
	uint64_t u64;
	struct cvmx_ilk_txx_flow_ctl0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t status                       : 64; /**< IPD flow control status for backpressue id 63-0, where a 0
                                                         indicates the presence of backpressure (ie. XOFF) and 1
                                                         indicates the absence of backpressure (ie. XON) */
#else
	uint64_t status                       : 64;
#endif
	} s;
	struct cvmx_ilk_txx_flow_ctl0_s       cn68xx;
	struct cvmx_ilk_txx_flow_ctl0_s       cn68xxp1;
};
typedef union cvmx_ilk_txx_flow_ctl0 cvmx_ilk_txx_flow_ctl0_t;

/**
 * cvmx_ilk_tx#_flow_ctl1
 *
 * Notes:
 * Do not publish.
 *
 */
union cvmx_ilk_txx_flow_ctl1 {
	uint64_t u64;
	struct cvmx_ilk_txx_flow_ctl1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_ilk_txx_flow_ctl1_s       cn68xx;
	struct cvmx_ilk_txx_flow_ctl1_s       cn68xxp1;
};
typedef union cvmx_ilk_txx_flow_ctl1 cvmx_ilk_txx_flow_ctl1_t;

/**
 * cvmx_ilk_tx#_idx_cal
 */
union cvmx_ilk_txx_idx_cal {
	uint64_t u64;
	struct cvmx_ilk_txx_idx_cal_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t inc                          : 6;  /**< Increment to add to current index for next index. NOTE:
                                                         Increment only performed after *MEM_CAL1 access (ie. not
                                                         *MEM_CAL0) */
	uint64_t reserved_6_7                 : 2;
	uint64_t index                        : 6;  /**< Specify the group of 8 entries accessed by the next CSR
                                                         read/write to calendar table memory.  Software must ensure IDX
                                                         is <36 whenever writing to *MEM_CAL1 */
#else
	uint64_t index                        : 6;
	uint64_t reserved_6_7                 : 2;
	uint64_t inc                          : 6;
	uint64_t reserved_14_63               : 50;
#endif
	} s;
	struct cvmx_ilk_txx_idx_cal_s         cn68xx;
	struct cvmx_ilk_txx_idx_cal_s         cn68xxp1;
};
typedef union cvmx_ilk_txx_idx_cal cvmx_ilk_txx_idx_cal_t;

/**
 * cvmx_ilk_tx#_idx_pmap
 */
union cvmx_ilk_txx_idx_pmap {
	uint64_t u64;
	struct cvmx_ilk_txx_idx_pmap_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_23_63               : 41;
	uint64_t inc                          : 7;  /**< Increment to add to current index for next index. */
	uint64_t reserved_7_15                : 9;
	uint64_t index                        : 7;  /**< Specify the port-pipe accessed by the next CSR read/write to
                                                         ILK_TXx_MEM_PMAP.   Note that IDX=n is always port-pipe n,
                                                         regardless of ILK_TXx_PIPE[BASE] */
#else
	uint64_t index                        : 7;
	uint64_t reserved_7_15                : 9;
	uint64_t inc                          : 7;
	uint64_t reserved_23_63               : 41;
#endif
	} s;
	struct cvmx_ilk_txx_idx_pmap_s        cn68xx;
	struct cvmx_ilk_txx_idx_pmap_s        cn68xxp1;
};
typedef union cvmx_ilk_txx_idx_pmap cvmx_ilk_txx_idx_pmap_t;

/**
 * cvmx_ilk_tx#_idx_stat0
 */
union cvmx_ilk_txx_idx_stat0 {
	uint64_t u64;
	struct cvmx_ilk_txx_idx_stat0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t clr                          : 1;  /**< CSR read to ILK_TXx_MEM_STAT0 clears the selected counter after
                                                         returning its current value. */
	uint64_t reserved_24_30               : 7;
	uint64_t inc                          : 8;  /**< Increment to add to current index for next index */
	uint64_t reserved_8_15                : 8;
	uint64_t index                        : 8;  /**< Specify the channel accessed during the next CSR read to the
                                                         ILK_TXx_MEM_STAT0 */
#else
	uint64_t index                        : 8;
	uint64_t reserved_8_15                : 8;
	uint64_t inc                          : 8;
	uint64_t reserved_24_30               : 7;
	uint64_t clr                          : 1;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_ilk_txx_idx_stat0_s       cn68xx;
	struct cvmx_ilk_txx_idx_stat0_s       cn68xxp1;
};
typedef union cvmx_ilk_txx_idx_stat0 cvmx_ilk_txx_idx_stat0_t;

/**
 * cvmx_ilk_tx#_idx_stat1
 */
union cvmx_ilk_txx_idx_stat1 {
	uint64_t u64;
	struct cvmx_ilk_txx_idx_stat1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t clr                          : 1;  /**< CSR read to ILK_TXx_MEM_STAT1 clears the selected counter after
                                                         returning its current value. */
	uint64_t reserved_24_30               : 7;
	uint64_t inc                          : 8;  /**< Increment to add to current index for next index */
	uint64_t reserved_8_15                : 8;
	uint64_t index                        : 8;  /**< Specify the channel accessed during the next CSR read to the
                                                         ILK_TXx_MEM_STAT1 */
#else
	uint64_t index                        : 8;
	uint64_t reserved_8_15                : 8;
	uint64_t inc                          : 8;
	uint64_t reserved_24_30               : 7;
	uint64_t clr                          : 1;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_ilk_txx_idx_stat1_s       cn68xx;
	struct cvmx_ilk_txx_idx_stat1_s       cn68xxp1;
};
typedef union cvmx_ilk_txx_idx_stat1 cvmx_ilk_txx_idx_stat1_t;

/**
 * cvmx_ilk_tx#_int
 */
union cvmx_ilk_txx_int {
	uint64_t u64;
	struct cvmx_ilk_txx_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t fwc_dbe                      : 1;  /**< Flow control calendar table double-bit error. Throws ILK_INTSN_E::ILK_TX()_FWC_DBE. */
	uint64_t fwc_sbe                      : 1;  /**< Flow control calendar table single-bit error. Throws ILK_INTSN_E::ILK_TX()_FWC_SBE. */
	uint64_t txf_dbe                      : 1;  /**< TX FIFO double-bit error. Throws ILK_INTSN_E::ILK_TX()_TXF_DBE. */
	uint64_t txf_sbe                      : 1;  /**< TX FIFO single-bit error. Throws ILK_INTSN_E::ILK_TX()_TXF_SBE. */
	uint64_t stat_cnt_ovfl                : 1;  /**< Statistics counter overflow. Throws ILK_INTSN_E::ILK_TX()_STAT_CNT_OVFL. */
	uint64_t bad_pipe                     : 1;  /**< Reserved. */
	uint64_t bad_seq                      : 1;  /**< Received sequence is not SOP followed by 0 or more data cycles followed by EOP. Throws
                                                         ILK_INTSN_E::ILK_TX()_BAD_SEQ. */
	uint64_t txf_err                      : 1;  /**< Reserved. */
#else
	uint64_t txf_err                      : 1;
	uint64_t bad_seq                      : 1;
	uint64_t bad_pipe                     : 1;
	uint64_t stat_cnt_ovfl                : 1;
	uint64_t txf_sbe                      : 1;
	uint64_t txf_dbe                      : 1;
	uint64_t fwc_sbe                      : 1;
	uint64_t fwc_dbe                      : 1;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_ilk_txx_int_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t stat_cnt_ovfl                : 1;  /**< Statistics counter overflow */
	uint64_t bad_pipe                     : 1;  /**< Received a PKO port-pipe out of the range specified by
                                                         ILK_TXX_PIPE */
	uint64_t bad_seq                      : 1;  /**< Received sequence is not SOP followed by 0 or more data cycles
                                                         followed by EOP.  PKO config assigned multiple engines to the
                                                         same ILK Tx Link. */
	uint64_t txf_err                      : 1;  /**< TX fifo parity error occurred.  At EOP time, EOP_Format will
                                                         reflect the error. */
#else
	uint64_t txf_err                      : 1;
	uint64_t bad_seq                      : 1;
	uint64_t bad_pipe                     : 1;
	uint64_t stat_cnt_ovfl                : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} cn68xx;
	struct cvmx_ilk_txx_int_cn68xx        cn68xxp1;
	struct cvmx_ilk_txx_int_s             cn78xx;
	struct cvmx_ilk_txx_int_s             cn78xxp1;
};
typedef union cvmx_ilk_txx_int cvmx_ilk_txx_int_t;

/**
 * cvmx_ilk_tx#_int_en
 */
union cvmx_ilk_txx_int_en {
	uint64_t u64;
	struct cvmx_ilk_txx_int_en_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t stat_cnt_ovfl                : 1;  /**< Statistics counter overflow */
	uint64_t bad_pipe                     : 1;  /**< Received a PKO port-pipe out of the range specified by
                                                         ILK_TXX_PIPE. */
	uint64_t bad_seq                      : 1;  /**< Received sequence is not SOP followed by 0 or more data cycles
                                                         followed by EOP.  PKO config assigned multiple engines to the
                                                         same ILK Tx Link. */
	uint64_t txf_err                      : 1;  /**< TX fifo parity error occurred.  At EOP time, EOP_Format will
                                                         reflect the error. */
#else
	uint64_t txf_err                      : 1;
	uint64_t bad_seq                      : 1;
	uint64_t bad_pipe                     : 1;
	uint64_t stat_cnt_ovfl                : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_ilk_txx_int_en_s          cn68xx;
	struct cvmx_ilk_txx_int_en_s          cn68xxp1;
};
typedef union cvmx_ilk_txx_int_en cvmx_ilk_txx_int_en_t;

/**
 * cvmx_ilk_tx#_mem_cal0
 *
 * Notes:
 * Software must always read ILK_TXx_MEM_CAL0 then ILK_TXx_MEM_CAL1.  Software
 * must never read them in reverse order or read one without reading the
 * other.
 *
 * Software must always write ILK_TXx_MEM_CAL0 then ILK_TXx_MEM_CAL1.
 * Software must never write them in reverse order or write one without
 * writing the other.
 */
union cvmx_ilk_txx_mem_cal0 {
	uint64_t u64;
	struct cvmx_ilk_txx_mem_cal0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t entry_ctl3                   : 2;  /**< Select source of XON/XOFF for entry (IDX*8)+3
                                                            - 0: IPD backpressue id
                                                            - 1: Link
                                                            - 2: XOFF
                                                            - 3: XON
                                                         This field applies to one of bits <52>, <44>, or <28> in the
                                                         Interlaken control word. */
	uint64_t reserved_33_33               : 1;
	uint64_t bpid3                        : 6;  /**< Select IPD backpressue id for calendar table entry (IDX*8)+3
                                                         (unused if ENTRY_CTL3 != 0) */
	uint64_t entry_ctl2                   : 2;  /**< Select source of XON/XOFF for entry (IDX*8)+2
                                                            - 0: IPD backpressue id
                                                            - 1: Link
                                                            - 2: XOFF
                                                            - 3: XON
                                                         This field applies to one of bits <53>, <45>, or <29> in the
                                                         Interlaken control word. */
	uint64_t reserved_24_24               : 1;
	uint64_t bpid2                        : 6;  /**< Select IPD backpressue id for calendar table entry (IDX*8)+2
                                                         (unused if ENTRY_CTL2 != 0) */
	uint64_t entry_ctl1                   : 2;  /**< Select source of XON/XOFF for entry (IDX*8)+1
                                                            - 0: IPD backpressue id
                                                            - 1: Link
                                                            - 2: XOFF
                                                            - 3: XON
                                                         This field applies to one of bits <54>, <46>, or <30> in the
                                                         Interlaken control word. */
	uint64_t reserved_15_15               : 1;
	uint64_t bpid1                        : 6;  /**< Select IPD backpressue id for calendar table entry (IDX*8)+1
                                                         (unused if ENTRY_CTL1 != 0) */
	uint64_t entry_ctl0                   : 2;  /**< Select source of XON/XOFF for entry (IDX*8)+0
                                                            - 0: IPD backpressue id
                                                            - 1: Link
                                                            - 2: XOFF
                                                            - 3: XON
                                                         This field applies to one of bits <55>, <47>, or <31> in the
                                                         Interlaken control word. */
	uint64_t reserved_6_6                 : 1;
	uint64_t bpid0                        : 6;  /**< Select IPD backpressue id for calendar table entry (IDX*8)+0
                                                         (unused if ENTRY_CTL0 != 0) */
#else
	uint64_t bpid0                        : 6;
	uint64_t reserved_6_6                 : 1;
	uint64_t entry_ctl0                   : 2;
	uint64_t bpid1                        : 6;
	uint64_t reserved_15_15               : 1;
	uint64_t entry_ctl1                   : 2;
	uint64_t bpid2                        : 6;
	uint64_t reserved_24_24               : 1;
	uint64_t entry_ctl2                   : 2;
	uint64_t bpid3                        : 6;
	uint64_t reserved_33_33               : 1;
	uint64_t entry_ctl3                   : 2;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_ilk_txx_mem_cal0_s        cn68xx;
	struct cvmx_ilk_txx_mem_cal0_s        cn68xxp1;
};
typedef union cvmx_ilk_txx_mem_cal0 cvmx_ilk_txx_mem_cal0_t;

/**
 * cvmx_ilk_tx#_mem_cal1
 *
 * Notes:
 * Software must always read ILK_TXx_MEM_CAL0 then ILK_TXx_MEM_CAL1.  Software
 * must never read them in reverse order or read one without reading the
 * other.
 *
 * Software must always write ILK_TXx_MEM_CAL0 then ILK_TXx_MEM_CAL1.
 * Software must never write them in reverse order or write one without
 * writing the other.
 */
union cvmx_ilk_txx_mem_cal1 {
	uint64_t u64;
	struct cvmx_ilk_txx_mem_cal1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t entry_ctl7                   : 2;  /**< Select source of XON/XOFF for entry (IDX*8)+7
                                                            - 0: IPD backpressue id
                                                            - 1: Link
                                                            - 2: XOFF
                                                            - 3: XON
                                                         This field applies to one of bits <48>, <40>, or <24> in the
                                                         Interlaken control word. */
	uint64_t reserved_33_33               : 1;
	uint64_t bpid7                        : 6;  /**< Select IPD backpressue id for calendar table entry (IDX*8)+7
                                                         (unused if ENTRY_CTL7 != 0) */
	uint64_t entry_ctl6                   : 2;  /**< Select source of XON/XOFF for entry (IDX*8)+6
                                                            - 0: IPD backpressue id
                                                            - 1: Link
                                                            - 2: XOFF
                                                            - 3: XON
                                                         This field applies to one of bits <49>, <41>, or <25> in the
                                                         Interlaken control word. */
	uint64_t reserved_24_24               : 1;
	uint64_t bpid6                        : 6;  /**< Select IPD backpressue id for calendar table entry (IDX*8)+6
                                                         (unused if ENTRY_CTL6 != 0) */
	uint64_t entry_ctl5                   : 2;  /**< Select source of XON/XOFF for entry (IDX*8)+5
                                                            - 0: IPD backpressue id
                                                            - 1: Link
                                                            - 2: XOFF
                                                            - 3: XON
                                                         This field applies to one of bits <50>, <42>, or <26> in the
                                                         Interlaken control word. */
	uint64_t reserved_15_15               : 1;
	uint64_t bpid5                        : 6;  /**< Select IPD backpressue id for calendar table entry (IDX*8)+5
                                                         (unused if ENTRY_CTL5 != 0) */
	uint64_t entry_ctl4                   : 2;  /**< Select source of XON/XOFF for entry (IDX*8)+4
                                                            - 0: IPD backpressue id
                                                            - 1: Link
                                                            - 2: XOFF
                                                            - 3: XON
                                                         This field applies to one of bits <51>, <43>, or <27> in the
                                                         Interlaken control word. */
	uint64_t reserved_6_6                 : 1;
	uint64_t bpid4                        : 6;  /**< Select IPD backpressue id for calendar table entry (IDX*8)+4
                                                         (unused if ENTRY_CTL4 != 0) */
#else
	uint64_t bpid4                        : 6;
	uint64_t reserved_6_6                 : 1;
	uint64_t entry_ctl4                   : 2;
	uint64_t bpid5                        : 6;
	uint64_t reserved_15_15               : 1;
	uint64_t entry_ctl5                   : 2;
	uint64_t bpid6                        : 6;
	uint64_t reserved_24_24               : 1;
	uint64_t entry_ctl6                   : 2;
	uint64_t bpid7                        : 6;
	uint64_t reserved_33_33               : 1;
	uint64_t entry_ctl7                   : 2;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_ilk_txx_mem_cal1_s        cn68xx;
	struct cvmx_ilk_txx_mem_cal1_s        cn68xxp1;
};
typedef union cvmx_ilk_txx_mem_cal1 cvmx_ilk_txx_mem_cal1_t;

/**
 * cvmx_ilk_tx#_mem_pmap
 */
union cvmx_ilk_txx_mem_pmap {
	uint64_t u64;
	struct cvmx_ilk_txx_mem_pmap_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_17_63               : 47;
	uint64_t remap                        : 1;  /**< Dynamically select channel using bits[39:32] of an 8-byte
                                                         header prepended to any packet transmitted on the port-pipe
                                                         selected by ILK_TXx_IDX_PMAP[IDX].

                                                         ***NOTE: Added in pass 2.0 */
	uint64_t reserved_8_15                : 8;
	uint64_t channel                      : 8;  /**< Specify the channel for the port-pipe selected by
                                                         ILK_TXx_IDX_PMAP[IDX] */
#else
	uint64_t channel                      : 8;
	uint64_t reserved_8_15                : 8;
	uint64_t remap                        : 1;
	uint64_t reserved_17_63               : 47;
#endif
	} s;
	struct cvmx_ilk_txx_mem_pmap_s        cn68xx;
	struct cvmx_ilk_txx_mem_pmap_cn68xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t channel                      : 8;  /**< Specify the channel for the port-pipe selected by
                                                         ILK_TXx_IDX_PMAP[IDX] */
#else
	uint64_t channel                      : 8;
	uint64_t reserved_8_63                : 56;
#endif
	} cn68xxp1;
};
typedef union cvmx_ilk_txx_mem_pmap cvmx_ilk_txx_mem_pmap_t;

/**
 * cvmx_ilk_tx#_mem_stat0
 */
union cvmx_ilk_txx_mem_stat0 {
	uint64_t u64;
	struct cvmx_ilk_txx_mem_stat0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_28_63               : 36;
	uint64_t tx_pkt                       : 28; /**< Number of packets transmitted per channel (256M)
                                                         Channel selected by ILK_TXx_IDX_STAT0[IDX].  Interrupt on
                                                         saturation if ILK_TXX_INT_EN[STAT_CNT_OVFL]=1. */
#else
	uint64_t tx_pkt                       : 28;
	uint64_t reserved_28_63               : 36;
#endif
	} s;
	struct cvmx_ilk_txx_mem_stat0_s       cn68xx;
	struct cvmx_ilk_txx_mem_stat0_s       cn68xxp1;
};
typedef union cvmx_ilk_txx_mem_stat0 cvmx_ilk_txx_mem_stat0_t;

/**
 * cvmx_ilk_tx#_mem_stat1
 */
union cvmx_ilk_txx_mem_stat1 {
	uint64_t u64;
	struct cvmx_ilk_txx_mem_stat1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t tx_bytes                     : 36; /**< Number of bytes transmitted per channel (64GB) Channel selected
                                                         by ILK_TXx_IDX_STAT1[IDX].    Saturates.  Interrupt on
                                                         saturation if ILK_TXX_INT_EN[STAT_CNT_OVFL]=1. */
#else
	uint64_t tx_bytes                     : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_ilk_txx_mem_stat1_s       cn68xx;
	struct cvmx_ilk_txx_mem_stat1_s       cn68xxp1;
};
typedef union cvmx_ilk_txx_mem_stat1 cvmx_ilk_txx_mem_stat1_t;

/**
 * cvmx_ilk_tx#_pipe
 */
union cvmx_ilk_txx_pipe {
	uint64_t u64;
	struct cvmx_ilk_txx_pipe_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t nump                         : 8;  /**< Number of pipes assigned to this Tx Link */
	uint64_t reserved_7_15                : 9;
	uint64_t base                         : 7;  /**< When NUMP is non-zero, indicates the base pipe number this
                                                         Tx link will accept.  This Tx will accept PKO packets from
                                                         pipes in the range of:  BASE .. (BASE+(NUMP-1))

                                                           BASE and NUMP must be constrained such that
                                                           1) BASE+(NUMP-1) < 127
                                                           2) Each used PKO pipe must map to exactly
                                                              one port|channel
                                                           3) The pipe ranges must be consistent with
                                                              the PKO configuration. */
#else
	uint64_t base                         : 7;
	uint64_t reserved_7_15                : 9;
	uint64_t nump                         : 8;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_ilk_txx_pipe_s            cn68xx;
	struct cvmx_ilk_txx_pipe_s            cn68xxp1;
};
typedef union cvmx_ilk_txx_pipe cvmx_ilk_txx_pipe_t;

/**
 * cvmx_ilk_tx#_pkt_cnt#
 */
union cvmx_ilk_txx_pkt_cntx {
	uint64_t u64;
	struct cvmx_ilk_txx_pkt_cntx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_34_63               : 30;
	uint64_t tx_pkt                       : 34; /**< Number of packets transmitted per channel. Wraps on overflow. On overflow, sets
                                                         ILK_TX()_INT[STAT_CNT_OVFL]. */
#else
	uint64_t tx_pkt                       : 34;
	uint64_t reserved_34_63               : 30;
#endif
	} s;
	struct cvmx_ilk_txx_pkt_cntx_s        cn78xx;
	struct cvmx_ilk_txx_pkt_cntx_s        cn78xxp1;
};
typedef union cvmx_ilk_txx_pkt_cntx cvmx_ilk_txx_pkt_cntx_t;

/**
 * cvmx_ilk_tx#_rmatch
 */
union cvmx_ilk_txx_rmatch {
	uint64_t u64;
	struct cvmx_ilk_txx_rmatch_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_50_63               : 14;
	uint64_t grnlrty                      : 2;  /**< Reserved. */
	uint64_t brst_limit                   : 16; /**< Reserved. */
	uint64_t time_limit                   : 16; /**< Reserved. */
	uint64_t rate_limit                   : 16; /**< Reserved. */
#else
	uint64_t rate_limit                   : 16;
	uint64_t time_limit                   : 16;
	uint64_t brst_limit                   : 16;
	uint64_t grnlrty                      : 2;
	uint64_t reserved_50_63               : 14;
#endif
	} s;
	struct cvmx_ilk_txx_rmatch_s          cn68xx;
	struct cvmx_ilk_txx_rmatch_s          cn68xxp1;
	struct cvmx_ilk_txx_rmatch_s          cn78xx;
	struct cvmx_ilk_txx_rmatch_s          cn78xxp1;
};
typedef union cvmx_ilk_txx_rmatch cvmx_ilk_txx_rmatch_t;

#endif
