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
 * cvmx-sso-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon sso.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_SSO_DEFS_H__
#define __CVMX_SSO_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_ACTIVE_CYCLES CVMX_SSO_ACTIVE_CYCLES_FUNC()
static inline uint64_t CVMX_SSO_ACTIVE_CYCLES_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_SSO_ACTIVE_CYCLES not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016700000010E8ull);
}
#else
#define CVMX_SSO_ACTIVE_CYCLES (CVMX_ADD_IO_SEG(0x00016700000010E8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_ACTIVE_CYCLESX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 2)))))
		cvmx_warn("CVMX_SSO_ACTIVE_CYCLESX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001670000001100ull) + ((offset) & 3) * 8;
}
#else
#define CVMX_SSO_ACTIVE_CYCLESX(offset) (CVMX_ADD_IO_SEG(0x0001670000001100ull) + ((offset) & 3) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_AW_ADD CVMX_SSO_AW_ADD_FUNC()
static inline uint64_t CVMX_SSO_AW_ADD_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SSO_AW_ADD not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000002080ull);
}
#else
#define CVMX_SSO_AW_ADD (CVMX_ADD_IO_SEG(0x0001670000002080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_AW_CFG CVMX_SSO_AW_CFG_FUNC()
static inline uint64_t CVMX_SSO_AW_CFG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SSO_AW_CFG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016700000010F0ull);
}
#else
#define CVMX_SSO_AW_CFG (CVMX_ADD_IO_SEG(0x00016700000010F0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_AW_ECO CVMX_SSO_AW_ECO_FUNC()
static inline uint64_t CVMX_SSO_AW_ECO_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SSO_AW_ECO not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000001030ull);
}
#else
#define CVMX_SSO_AW_ECO (CVMX_ADD_IO_SEG(0x0001670000001030ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_AW_READ_ARB CVMX_SSO_AW_READ_ARB_FUNC()
static inline uint64_t CVMX_SSO_AW_READ_ARB_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SSO_AW_READ_ARB not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000002090ull);
}
#else
#define CVMX_SSO_AW_READ_ARB (CVMX_ADD_IO_SEG(0x0001670000002090ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_AW_STATUS CVMX_SSO_AW_STATUS_FUNC()
static inline uint64_t CVMX_SSO_AW_STATUS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SSO_AW_STATUS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016700000010E0ull);
}
#else
#define CVMX_SSO_AW_STATUS (CVMX_ADD_IO_SEG(0x00016700000010E0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_AW_TAG_LATENCY_PC CVMX_SSO_AW_TAG_LATENCY_PC_FUNC()
static inline uint64_t CVMX_SSO_AW_TAG_LATENCY_PC_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SSO_AW_TAG_LATENCY_PC not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016700000020A8ull);
}
#else
#define CVMX_SSO_AW_TAG_LATENCY_PC (CVMX_ADD_IO_SEG(0x00016700000020A8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_AW_TAG_REQ_PC CVMX_SSO_AW_TAG_REQ_PC_FUNC()
static inline uint64_t CVMX_SSO_AW_TAG_REQ_PC_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SSO_AW_TAG_REQ_PC not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016700000020A0ull);
}
#else
#define CVMX_SSO_AW_TAG_REQ_PC (CVMX_ADD_IO_SEG(0x00016700000020A0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_AW_WE CVMX_SSO_AW_WE_FUNC()
static inline uint64_t CVMX_SSO_AW_WE_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SSO_AW_WE not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000001080ull);
}
#else
#define CVMX_SSO_AW_WE (CVMX_ADD_IO_SEG(0x0001670000001080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_BIST_STAT CVMX_SSO_BIST_STAT_FUNC()
static inline uint64_t CVMX_SSO_BIST_STAT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_SSO_BIST_STAT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000001078ull);
}
#else
#define CVMX_SSO_BIST_STAT (CVMX_ADD_IO_SEG(0x0001670000001078ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_BIST_STATUS0 CVMX_SSO_BIST_STATUS0_FUNC()
static inline uint64_t CVMX_SSO_BIST_STATUS0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SSO_BIST_STATUS0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000001200ull);
}
#else
#define CVMX_SSO_BIST_STATUS0 (CVMX_ADD_IO_SEG(0x0001670000001200ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_BIST_STATUS1 CVMX_SSO_BIST_STATUS1_FUNC()
static inline uint64_t CVMX_SSO_BIST_STATUS1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SSO_BIST_STATUS1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000001208ull);
}
#else
#define CVMX_SSO_BIST_STATUS1 (CVMX_ADD_IO_SEG(0x0001670000001208ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_BIST_STATUS2 CVMX_SSO_BIST_STATUS2_FUNC()
static inline uint64_t CVMX_SSO_BIST_STATUS2_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SSO_BIST_STATUS2 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000001210ull);
}
#else
#define CVMX_SSO_BIST_STATUS2 (CVMX_ADD_IO_SEG(0x0001670000001210ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_CFG CVMX_SSO_CFG_FUNC()
static inline uint64_t CVMX_SSO_CFG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_SSO_CFG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000001088ull);
}
#else
#define CVMX_SSO_CFG (CVMX_ADD_IO_SEG(0x0001670000001088ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_DS_PC CVMX_SSO_DS_PC_FUNC()
static inline uint64_t CVMX_SSO_DS_PC_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_SSO_DS_PC not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000001070ull);
}
#else
#define CVMX_SSO_DS_PC (CVMX_ADD_IO_SEG(0x0001670000001070ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_ECC_CTL0 CVMX_SSO_ECC_CTL0_FUNC()
static inline uint64_t CVMX_SSO_ECC_CTL0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SSO_ECC_CTL0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000001280ull);
}
#else
#define CVMX_SSO_ECC_CTL0 (CVMX_ADD_IO_SEG(0x0001670000001280ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_ECC_CTL1 CVMX_SSO_ECC_CTL1_FUNC()
static inline uint64_t CVMX_SSO_ECC_CTL1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SSO_ECC_CTL1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000001288ull);
}
#else
#define CVMX_SSO_ECC_CTL1 (CVMX_ADD_IO_SEG(0x0001670000001288ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_ECC_CTL2 CVMX_SSO_ECC_CTL2_FUNC()
static inline uint64_t CVMX_SSO_ECC_CTL2_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SSO_ECC_CTL2 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000001290ull);
}
#else
#define CVMX_SSO_ECC_CTL2 (CVMX_ADD_IO_SEG(0x0001670000001290ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_ERR CVMX_SSO_ERR_FUNC()
static inline uint64_t CVMX_SSO_ERR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_SSO_ERR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000001038ull);
}
#else
#define CVMX_SSO_ERR (CVMX_ADD_IO_SEG(0x0001670000001038ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_ERR0 CVMX_SSO_ERR0_FUNC()
static inline uint64_t CVMX_SSO_ERR0_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SSO_ERR0 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000001240ull);
}
#else
#define CVMX_SSO_ERR0 (CVMX_ADD_IO_SEG(0x0001670000001240ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_ERR1 CVMX_SSO_ERR1_FUNC()
static inline uint64_t CVMX_SSO_ERR1_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SSO_ERR1 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000001248ull);
}
#else
#define CVMX_SSO_ERR1 (CVMX_ADD_IO_SEG(0x0001670000001248ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_ERR2 CVMX_SSO_ERR2_FUNC()
static inline uint64_t CVMX_SSO_ERR2_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SSO_ERR2 not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000001250ull);
}
#else
#define CVMX_SSO_ERR2 (CVMX_ADD_IO_SEG(0x0001670000001250ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_ERR_ENB CVMX_SSO_ERR_ENB_FUNC()
static inline uint64_t CVMX_SSO_ERR_ENB_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_SSO_ERR_ENB not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000001030ull);
}
#else
#define CVMX_SSO_ERR_ENB (CVMX_ADD_IO_SEG(0x0001670000001030ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_FIDX_ECC_CTL CVMX_SSO_FIDX_ECC_CTL_FUNC()
static inline uint64_t CVMX_SSO_FIDX_ECC_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_SSO_FIDX_ECC_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016700000010D0ull);
}
#else
#define CVMX_SSO_FIDX_ECC_CTL (CVMX_ADD_IO_SEG(0x00016700000010D0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_FIDX_ECC_ST CVMX_SSO_FIDX_ECC_ST_FUNC()
static inline uint64_t CVMX_SSO_FIDX_ECC_ST_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_SSO_FIDX_ECC_ST not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016700000010D8ull);
}
#else
#define CVMX_SSO_FIDX_ECC_ST (CVMX_ADD_IO_SEG(0x00016700000010D8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_FPAGE_CNT CVMX_SSO_FPAGE_CNT_FUNC()
static inline uint64_t CVMX_SSO_FPAGE_CNT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_SSO_FPAGE_CNT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000001090ull);
}
#else
#define CVMX_SSO_FPAGE_CNT (CVMX_ADD_IO_SEG(0x0001670000001090ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_GRPX_AQ_CNT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_SSO_GRPX_AQ_CNT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001670020000700ull) + ((offset) & 255) * 0x10000ull;
}
#else
#define CVMX_SSO_GRPX_AQ_CNT(offset) (CVMX_ADD_IO_SEG(0x0001670020000700ull) + ((offset) & 255) * 0x10000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_GRPX_AQ_THR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_SSO_GRPX_AQ_THR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001670020000800ull) + ((offset) & 255) * 0x10000ull;
}
#else
#define CVMX_SSO_GRPX_AQ_THR(offset) (CVMX_ADD_IO_SEG(0x0001670020000800ull) + ((offset) & 255) * 0x10000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_GRPX_DS_PC(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_SSO_GRPX_DS_PC(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001670020001400ull) + ((offset) & 255) * 0x10000ull;
}
#else
#define CVMX_SSO_GRPX_DS_PC(offset) (CVMX_ADD_IO_SEG(0x0001670020001400ull) + ((offset) & 255) * 0x10000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_GRPX_EXT_PC(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_SSO_GRPX_EXT_PC(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001670020001100ull) + ((offset) & 255) * 0x10000ull;
}
#else
#define CVMX_SSO_GRPX_EXT_PC(offset) (CVMX_ADD_IO_SEG(0x0001670020001100ull) + ((offset) & 255) * 0x10000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_GRPX_IAQ_THR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_SSO_GRPX_IAQ_THR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001670020000000ull) + ((offset) & 255) * 0x10000ull;
}
#else
#define CVMX_SSO_GRPX_IAQ_THR(offset) (CVMX_ADD_IO_SEG(0x0001670020000000ull) + ((offset) & 255) * 0x10000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_GRPX_INT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_SSO_GRPX_INT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001670020000400ull) + ((offset) & 255) * 0x10000ull;
}
#else
#define CVMX_SSO_GRPX_INT(offset) (CVMX_ADD_IO_SEG(0x0001670020000400ull) + ((offset) & 255) * 0x10000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_GRPX_INT_CNT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_SSO_GRPX_INT_CNT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001670020000600ull) + ((offset) & 255) * 0x10000ull;
}
#else
#define CVMX_SSO_GRPX_INT_CNT(offset) (CVMX_ADD_IO_SEG(0x0001670020000600ull) + ((offset) & 255) * 0x10000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_GRPX_INT_THR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_SSO_GRPX_INT_THR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001670020000500ull) + ((offset) & 255) * 0x10000ull;
}
#else
#define CVMX_SSO_GRPX_INT_THR(offset) (CVMX_ADD_IO_SEG(0x0001670020000500ull) + ((offset) & 255) * 0x10000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_GRPX_PRI(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_SSO_GRPX_PRI(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001670020000200ull) + ((offset) & 255) * 0x10000ull;
}
#else
#define CVMX_SSO_GRPX_PRI(offset) (CVMX_ADD_IO_SEG(0x0001670020000200ull) + ((offset) & 255) * 0x10000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_GRPX_TAQ_THR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_SSO_GRPX_TAQ_THR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001670020000100ull) + ((offset) & 255) * 0x10000ull;
}
#else
#define CVMX_SSO_GRPX_TAQ_THR(offset) (CVMX_ADD_IO_SEG(0x0001670020000100ull) + ((offset) & 255) * 0x10000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_GRPX_TS_PC(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_SSO_GRPX_TS_PC(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001670020001300ull) + ((offset) & 255) * 0x10000ull;
}
#else
#define CVMX_SSO_GRPX_TS_PC(offset) (CVMX_ADD_IO_SEG(0x0001670020001300ull) + ((offset) & 255) * 0x10000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_GRPX_WA_PC(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_SSO_GRPX_WA_PC(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001670020001200ull) + ((offset) & 255) * 0x10000ull;
}
#else
#define CVMX_SSO_GRPX_WA_PC(offset) (CVMX_ADD_IO_SEG(0x0001670020001200ull) + ((offset) & 255) * 0x10000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_GRPX_WS_PC(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_SSO_GRPX_WS_PC(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001670020001000ull) + ((offset) & 255) * 0x10000ull;
}
#else
#define CVMX_SSO_GRPX_WS_PC(offset) (CVMX_ADD_IO_SEG(0x0001670020001000ull) + ((offset) & 255) * 0x10000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_GWE_CFG CVMX_SSO_GWE_CFG_FUNC()
static inline uint64_t CVMX_SSO_GWE_CFG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SSO_GWE_CFG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000001098ull);
}
#else
#define CVMX_SSO_GWE_CFG (CVMX_ADD_IO_SEG(0x0001670000001098ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_GWE_RANDOM CVMX_SSO_GWE_RANDOM_FUNC()
static inline uint64_t CVMX_SSO_GWE_RANDOM_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SSO_GWE_RANDOM not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016700000010B0ull);
}
#else
#define CVMX_SSO_GWE_RANDOM (CVMX_ADD_IO_SEG(0x00016700000010B0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_GW_ECO CVMX_SSO_GW_ECO_FUNC()
static inline uint64_t CVMX_SSO_GW_ECO_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SSO_GW_ECO not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000001038ull);
}
#else
#define CVMX_SSO_GW_ECO (CVMX_ADD_IO_SEG(0x0001670000001038ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_IDX_ECC_CTL CVMX_SSO_IDX_ECC_CTL_FUNC()
static inline uint64_t CVMX_SSO_IDX_ECC_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_SSO_IDX_ECC_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016700000010C0ull);
}
#else
#define CVMX_SSO_IDX_ECC_CTL (CVMX_ADD_IO_SEG(0x00016700000010C0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_IDX_ECC_ST CVMX_SSO_IDX_ECC_ST_FUNC()
static inline uint64_t CVMX_SSO_IDX_ECC_ST_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_SSO_IDX_ECC_ST not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016700000010C8ull);
}
#else
#define CVMX_SSO_IDX_ECC_ST (CVMX_ADD_IO_SEG(0x00016700000010C8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_IENTX_LINKS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 4095))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 4095))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_SSO_IENTX_LINKS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016700A0060000ull) + ((offset) & 4095) * 8;
}
#else
#define CVMX_SSO_IENTX_LINKS(offset) (CVMX_ADD_IO_SEG(0x00016700A0060000ull) + ((offset) & 4095) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_IENTX_PENDTAG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 4095))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 4095))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_SSO_IENTX_PENDTAG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016700A0040000ull) + ((offset) & 4095) * 8;
}
#else
#define CVMX_SSO_IENTX_PENDTAG(offset) (CVMX_ADD_IO_SEG(0x00016700A0040000ull) + ((offset) & 4095) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_IENTX_QLINKS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 4095))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 4095))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_SSO_IENTX_QLINKS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016700A0080000ull) + ((offset) & 4095) * 8;
}
#else
#define CVMX_SSO_IENTX_QLINKS(offset) (CVMX_ADD_IO_SEG(0x00016700A0080000ull) + ((offset) & 4095) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_IENTX_TAG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 4095))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 4095))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_SSO_IENTX_TAG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016700A0000000ull) + ((offset) & 4095) * 8;
}
#else
#define CVMX_SSO_IENTX_TAG(offset) (CVMX_ADD_IO_SEG(0x00016700A0000000ull) + ((offset) & 4095) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_IENTX_WQPGRP(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1023))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 4095))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 4095))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1023)))))
		cvmx_warn("CVMX_SSO_IENTX_WQPGRP(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016700A0020000ull) + ((offset) & 4095) * 8;
}
#else
#define CVMX_SSO_IENTX_WQPGRP(offset) (CVMX_ADD_IO_SEG(0x00016700A0020000ull) + ((offset) & 4095) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_IPL_CONFX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_SSO_IPL_CONFX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001670080080000ull) + ((offset) & 255) * 8;
}
#else
#define CVMX_SSO_IPL_CONFX(offset) (CVMX_ADD_IO_SEG(0x0001670080080000ull) + ((offset) & 255) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_IPL_DESCHEDX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_SSO_IPL_DESCHEDX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001670080060000ull) + ((offset) & 255) * 8;
}
#else
#define CVMX_SSO_IPL_DESCHEDX(offset) (CVMX_ADD_IO_SEG(0x0001670080060000ull) + ((offset) & 255) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_IPL_FREEX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 4))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 4))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 4))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 4)))))
		cvmx_warn("CVMX_SSO_IPL_FREEX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001670080000000ull) + ((offset) & 7) * 8;
}
#else
#define CVMX_SSO_IPL_FREEX(offset) (CVMX_ADD_IO_SEG(0x0001670080000000ull) + ((offset) & 7) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_IPL_IAQX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_SSO_IPL_IAQX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001670080040000ull) + ((offset) & 255) * 8;
}
#else
#define CVMX_SSO_IPL_IAQX(offset) (CVMX_ADD_IO_SEG(0x0001670080040000ull) + ((offset) & 255) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_IQ_CNTX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 7)))))
		cvmx_warn("CVMX_SSO_IQ_CNTX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001670000009000ull) + ((offset) & 7) * 8;
}
#else
#define CVMX_SSO_IQ_CNTX(offset) (CVMX_ADD_IO_SEG(0x0001670000009000ull) + ((offset) & 7) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_IQ_COM_CNT CVMX_SSO_IQ_COM_CNT_FUNC()
static inline uint64_t CVMX_SSO_IQ_COM_CNT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_SSO_IQ_COM_CNT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000001058ull);
}
#else
#define CVMX_SSO_IQ_COM_CNT (CVMX_ADD_IO_SEG(0x0001670000001058ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_IQ_INT CVMX_SSO_IQ_INT_FUNC()
static inline uint64_t CVMX_SSO_IQ_INT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_SSO_IQ_INT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000001048ull);
}
#else
#define CVMX_SSO_IQ_INT (CVMX_ADD_IO_SEG(0x0001670000001048ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_IQ_INT_EN CVMX_SSO_IQ_INT_EN_FUNC()
static inline uint64_t CVMX_SSO_IQ_INT_EN_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_SSO_IQ_INT_EN not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000001050ull);
}
#else
#define CVMX_SSO_IQ_INT_EN (CVMX_ADD_IO_SEG(0x0001670000001050ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_IQ_THRX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 7)))))
		cvmx_warn("CVMX_SSO_IQ_THRX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000167000000A000ull) + ((offset) & 7) * 8;
}
#else
#define CVMX_SSO_IQ_THRX(offset) (CVMX_ADD_IO_SEG(0x000167000000A000ull) + ((offset) & 7) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_NOS_CNT CVMX_SSO_NOS_CNT_FUNC()
static inline uint64_t CVMX_SSO_NOS_CNT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SSO_NOS_CNT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000001040ull);
}
#else
#define CVMX_SSO_NOS_CNT (CVMX_ADD_IO_SEG(0x0001670000001040ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_NW_TIM CVMX_SSO_NW_TIM_FUNC()
static inline uint64_t CVMX_SSO_NW_TIM_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SSO_NW_TIM not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000001028ull);
}
#else
#define CVMX_SSO_NW_TIM (CVMX_ADD_IO_SEG(0x0001670000001028ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_OTH_ECC_CTL CVMX_SSO_OTH_ECC_CTL_FUNC()
static inline uint64_t CVMX_SSO_OTH_ECC_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_SSO_OTH_ECC_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016700000010B0ull);
}
#else
#define CVMX_SSO_OTH_ECC_CTL (CVMX_ADD_IO_SEG(0x00016700000010B0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_OTH_ECC_ST CVMX_SSO_OTH_ECC_ST_FUNC()
static inline uint64_t CVMX_SSO_OTH_ECC_ST_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_SSO_OTH_ECC_ST not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016700000010B8ull);
}
#else
#define CVMX_SSO_OTH_ECC_ST (CVMX_ADD_IO_SEG(0x00016700000010B8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_PAGE_CNT CVMX_SSO_PAGE_CNT_FUNC()
static inline uint64_t CVMX_SSO_PAGE_CNT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SSO_PAGE_CNT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000001090ull);
}
#else
#define CVMX_SSO_PAGE_CNT (CVMX_ADD_IO_SEG(0x0001670000001090ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_PND_ECC_CTL CVMX_SSO_PND_ECC_CTL_FUNC()
static inline uint64_t CVMX_SSO_PND_ECC_CTL_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_SSO_PND_ECC_CTL not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016700000010A0ull);
}
#else
#define CVMX_SSO_PND_ECC_CTL (CVMX_ADD_IO_SEG(0x00016700000010A0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_PND_ECC_ST CVMX_SSO_PND_ECC_ST_FUNC()
static inline uint64_t CVMX_SSO_PND_ECC_ST_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_SSO_PND_ECC_ST not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016700000010A8ull);
}
#else
#define CVMX_SSO_PND_ECC_ST (CVMX_ADD_IO_SEG(0x00016700000010A8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_PPX_ARB(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 15))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 47))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 47))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 15)))))
		cvmx_warn("CVMX_SSO_PPX_ARB(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001670040000000ull) + ((offset) & 63) * 0x10000ull;
}
#else
#define CVMX_SSO_PPX_ARB(offset) (CVMX_ADD_IO_SEG(0x0001670040000000ull) + ((offset) & 63) * 0x10000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_PPX_GRP_MSK(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_SSO_PPX_GRP_MSK(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001670000006000ull) + ((offset) & 31) * 8;
}
#else
#define CVMX_SSO_PPX_GRP_MSK(offset) (CVMX_ADD_IO_SEG(0x0001670000006000ull) + ((offset) & 31) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_PPX_QOS_PRI(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 31)))))
		cvmx_warn("CVMX_SSO_PPX_QOS_PRI(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001670000003000ull) + ((offset) & 31) * 8;
}
#else
#define CVMX_SSO_PPX_QOS_PRI(offset) (CVMX_ADD_IO_SEG(0x0001670000003000ull) + ((offset) & 31) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_PPX_SX_GRPMSKX(unsigned long a, unsigned long b, unsigned long c)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((a <= 15)) && ((b <= 1)) && ((c == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((a <= 47)) && ((b <= 1)) && ((c <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((a <= 47)) && ((b <= 1)) && ((c <= 3)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((a <= 15)) && ((b <= 1)) && ((c == 0))))))
		cvmx_warn("CVMX_SSO_PPX_SX_GRPMSKX(%lu,%lu,%lu) is invalid on this chip\n", a, b, c);
	return CVMX_ADD_IO_SEG(0x0001670040001000ull) + ((a) << 16) + ((b) << 5) + ((c) << 3);
}
#else
#define CVMX_SSO_PPX_SX_GRPMSKX(a, b, c) (CVMX_ADD_IO_SEG(0x0001670040001000ull) + ((a) << 16) + ((b) << 5) + ((c) << 3))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_PP_STRICT CVMX_SSO_PP_STRICT_FUNC()
static inline uint64_t CVMX_SSO_PP_STRICT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_SSO_PP_STRICT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016700000010E0ull);
}
#else
#define CVMX_SSO_PP_STRICT (CVMX_ADD_IO_SEG(0x00016700000010E0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_QOSX_RND(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 7)))))
		cvmx_warn("CVMX_SSO_QOSX_RND(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001670000002000ull) + ((offset) & 7) * 8;
}
#else
#define CVMX_SSO_QOSX_RND(offset) (CVMX_ADD_IO_SEG(0x0001670000002000ull) + ((offset) & 7) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_QOS_THRX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 7)))))
		cvmx_warn("CVMX_SSO_QOS_THRX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000167000000B000ull) + ((offset) & 7) * 8;
}
#else
#define CVMX_SSO_QOS_THRX(offset) (CVMX_ADD_IO_SEG(0x000167000000B000ull) + ((offset) & 7) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_QOS_WE CVMX_SSO_QOS_WE_FUNC()
static inline uint64_t CVMX_SSO_QOS_WE_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_SSO_QOS_WE not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000001080ull);
}
#else
#define CVMX_SSO_QOS_WE (CVMX_ADD_IO_SEG(0x0001670000001080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_RESET CVMX_SSO_RESET_FUNC()
static inline uint64_t CVMX_SSO_RESET_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
					return CVMX_ADD_IO_SEG(0x00016700000010F8ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
					return CVMX_ADD_IO_SEG(0x00016700000010F8ull);
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00016700000010F8ull);
			break;
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00016700000010F0ull);
			break;
	}
	cvmx_warn("CVMX_SSO_RESET not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016700000010F8ull);
}
#else
#define CVMX_SSO_RESET CVMX_SSO_RESET_FUNC()
static inline uint64_t CVMX_SSO_RESET_FUNC(void)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CNF75XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return CVMX_ADD_IO_SEG(0x00016700000010F8ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return CVMX_ADD_IO_SEG(0x00016700000010F8ull);
		case OCTEON_CN73XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00016700000010F8ull);
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x00016700000010F0ull);
	}
	return CVMX_ADD_IO_SEG(0x00016700000010F8ull);
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_RWQ_HEAD_PTRX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 7)))))
		cvmx_warn("CVMX_SSO_RWQ_HEAD_PTRX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000167000000C000ull) + ((offset) & 7) * 8;
}
#else
#define CVMX_SSO_RWQ_HEAD_PTRX(offset) (CVMX_ADD_IO_SEG(0x000167000000C000ull) + ((offset) & 7) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_RWQ_POP_FPTR CVMX_SSO_RWQ_POP_FPTR_FUNC()
static inline uint64_t CVMX_SSO_RWQ_POP_FPTR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_SSO_RWQ_POP_FPTR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x000167000000C408ull);
}
#else
#define CVMX_SSO_RWQ_POP_FPTR (CVMX_ADD_IO_SEG(0x000167000000C408ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_RWQ_PSH_FPTR CVMX_SSO_RWQ_PSH_FPTR_FUNC()
static inline uint64_t CVMX_SSO_RWQ_PSH_FPTR_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_SSO_RWQ_PSH_FPTR not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x000167000000C400ull);
}
#else
#define CVMX_SSO_RWQ_PSH_FPTR (CVMX_ADD_IO_SEG(0x000167000000C400ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_RWQ_TAIL_PTRX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 7)))))
		cvmx_warn("CVMX_SSO_RWQ_TAIL_PTRX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000167000000C200ull) + ((offset) & 7) * 8;
}
#else
#define CVMX_SSO_RWQ_TAIL_PTRX(offset) (CVMX_ADD_IO_SEG(0x000167000000C200ull) + ((offset) & 7) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_SL_PPX_LINKS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 15))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 47))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 47))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 15)))))
		cvmx_warn("CVMX_SSO_SL_PPX_LINKS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001670060000040ull) + ((offset) & 63) * 0x10000ull;
}
#else
#define CVMX_SSO_SL_PPX_LINKS(offset) (CVMX_ADD_IO_SEG(0x0001670060000040ull) + ((offset) & 63) * 0x10000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_SL_PPX_PENDTAG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 15))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 47))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 47))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 15)))))
		cvmx_warn("CVMX_SSO_SL_PPX_PENDTAG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001670060000000ull) + ((offset) & 63) * 0x10000ull;
}
#else
#define CVMX_SSO_SL_PPX_PENDTAG(offset) (CVMX_ADD_IO_SEG(0x0001670060000000ull) + ((offset) & 63) * 0x10000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_SL_PPX_PENDWQP(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 15))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 47))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 47))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 15)))))
		cvmx_warn("CVMX_SSO_SL_PPX_PENDWQP(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001670060000010ull) + ((offset) & 63) * 0x10000ull;
}
#else
#define CVMX_SSO_SL_PPX_PENDWQP(offset) (CVMX_ADD_IO_SEG(0x0001670060000010ull) + ((offset) & 63) * 0x10000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_SL_PPX_TAG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 15))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 47))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 47))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 15)))))
		cvmx_warn("CVMX_SSO_SL_PPX_TAG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001670060000020ull) + ((offset) & 63) * 0x10000ull;
}
#else
#define CVMX_SSO_SL_PPX_TAG(offset) (CVMX_ADD_IO_SEG(0x0001670060000020ull) + ((offset) & 63) * 0x10000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_SL_PPX_WQP(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 15))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 47))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 47))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 15)))))
		cvmx_warn("CVMX_SSO_SL_PPX_WQP(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001670060000030ull) + ((offset) & 63) * 0x10000ull;
}
#else
#define CVMX_SSO_SL_PPX_WQP(offset) (CVMX_ADD_IO_SEG(0x0001670060000030ull) + ((offset) & 63) * 0x10000ull)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_TAQX_LINK(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 319))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1279))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1279))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 319)))))
		cvmx_warn("CVMX_SSO_TAQX_LINK(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016700C0000000ull) + ((offset) & 2047) * 4096;
}
#else
#define CVMX_SSO_TAQX_LINK(offset) (CVMX_ADD_IO_SEG(0x00016700C0000000ull) + ((offset) & 2047) * 4096)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_TAQX_WAEX_TAG(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset <= 12)) && ((block_id <= 319)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 12)) && ((block_id <= 1279)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 12)) && ((block_id <= 1279)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset <= 12)) && ((block_id <= 319))))))
		cvmx_warn("CVMX_SSO_TAQX_WAEX_TAG(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00016700D0000000ull) + (((offset) & 15) + ((block_id) & 2047) * 0x100ull) * 16;
}
#else
#define CVMX_SSO_TAQX_WAEX_TAG(offset, block_id) (CVMX_ADD_IO_SEG(0x00016700D0000000ull) + (((offset) & 15) + ((block_id) & 2047) * 0x100ull) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_TAQX_WAEX_WQP(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && (((offset <= 12)) && ((block_id <= 319)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 12)) && ((block_id <= 1279)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 12)) && ((block_id <= 1279)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && (((offset <= 12)) && ((block_id <= 319))))))
		cvmx_warn("CVMX_SSO_TAQX_WAEX_WQP(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x00016700D0000008ull) + (((offset) & 15) + ((block_id) & 2047) * 0x100ull) * 16;
}
#else
#define CVMX_SSO_TAQX_WAEX_WQP(offset, block_id) (CVMX_ADD_IO_SEG(0x00016700D0000008ull) + (((offset) & 15) + ((block_id) & 2047) * 0x100ull) * 16)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_TAQ_ADD CVMX_SSO_TAQ_ADD_FUNC()
static inline uint64_t CVMX_SSO_TAQ_ADD_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SSO_TAQ_ADD not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016700000020E0ull);
}
#else
#define CVMX_SSO_TAQ_ADD (CVMX_ADD_IO_SEG(0x00016700000020E0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_TAQ_CNT CVMX_SSO_TAQ_CNT_FUNC()
static inline uint64_t CVMX_SSO_TAQ_CNT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SSO_TAQ_CNT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016700000020C0ull);
}
#else
#define CVMX_SSO_TAQ_CNT (CVMX_ADD_IO_SEG(0x00016700000020C0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_TIAQX_STATUS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_SSO_TIAQX_STATUS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016700000C0000ull) + ((offset) & 255) * 8;
}
#else
#define CVMX_SSO_TIAQX_STATUS(offset) (CVMX_ADD_IO_SEG(0x00016700000C0000ull) + ((offset) & 255) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_TOAQX_STATUS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_SSO_TOAQX_STATUS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016700000D0000ull) + ((offset) & 255) * 8;
}
#else
#define CVMX_SSO_TOAQX_STATUS(offset) (CVMX_ADD_IO_SEG(0x00016700000D0000ull) + ((offset) & 255) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_TS_PC CVMX_SSO_TS_PC_FUNC()
static inline uint64_t CVMX_SSO_TS_PC_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_SSO_TS_PC not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000001068ull);
}
#else
#define CVMX_SSO_TS_PC (CVMX_ADD_IO_SEG(0x0001670000001068ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_WA_COM_PC CVMX_SSO_WA_COM_PC_FUNC()
static inline uint64_t CVMX_SSO_WA_COM_PC_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_SSO_WA_COM_PC not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000001060ull);
}
#else
#define CVMX_SSO_WA_COM_PC (CVMX_ADD_IO_SEG(0x0001670000001060ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_WA_PCX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 7)))))
		cvmx_warn("CVMX_SSO_WA_PCX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001670000005000ull) + ((offset) & 7) * 8;
}
#else
#define CVMX_SSO_WA_PCX(offset) (CVMX_ADD_IO_SEG(0x0001670000005000ull) + ((offset) & 7) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_WQ_INT CVMX_SSO_WQ_INT_FUNC()
static inline uint64_t CVMX_SSO_WQ_INT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_SSO_WQ_INT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000001000ull);
}
#else
#define CVMX_SSO_WQ_INT (CVMX_ADD_IO_SEG(0x0001670000001000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_WQ_INT_CNTX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_SSO_WQ_INT_CNTX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001670000008000ull) + ((offset) & 63) * 8;
}
#else
#define CVMX_SSO_WQ_INT_CNTX(offset) (CVMX_ADD_IO_SEG(0x0001670000008000ull) + ((offset) & 63) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_WQ_INT_PC CVMX_SSO_WQ_INT_PC_FUNC()
static inline uint64_t CVMX_SSO_WQ_INT_PC_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX) || OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SSO_WQ_INT_PC not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000001020ull);
}
#else
#define CVMX_SSO_WQ_INT_PC (CVMX_ADD_IO_SEG(0x0001670000001020ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_WQ_INT_THRX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_SSO_WQ_INT_THRX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001670000007000ull) + ((offset) & 63) * 8;
}
#else
#define CVMX_SSO_WQ_INT_THRX(offset) (CVMX_ADD_IO_SEG(0x0001670000007000ull) + ((offset) & 63) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_WQ_IQ_DIS CVMX_SSO_WQ_IQ_DIS_FUNC()
static inline uint64_t CVMX_SSO_WQ_IQ_DIS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN68XX)))
		cvmx_warn("CVMX_SSO_WQ_IQ_DIS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000001010ull);
}
#else
#define CVMX_SSO_WQ_IQ_DIS (CVMX_ADD_IO_SEG(0x0001670000001010ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_WS_CFG CVMX_SSO_WS_CFG_FUNC()
static inline uint64_t CVMX_SSO_WS_CFG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SSO_WS_CFG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000001088ull);
}
#else
#define CVMX_SSO_WS_CFG (CVMX_ADD_IO_SEG(0x0001670000001088ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_WS_ECO CVMX_SSO_WS_ECO_FUNC()
static inline uint64_t CVMX_SSO_WS_ECO_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SSO_WS_ECO not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000001048ull);
}
#else
#define CVMX_SSO_WS_ECO (CVMX_ADD_IO_SEG(0x0001670000001048ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_WS_PCX(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_SSO_WS_PCX(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001670000004000ull) + ((offset) & 63) * 8;
}
#else
#define CVMX_SSO_WS_PCX(offset) (CVMX_ADD_IO_SEG(0x0001670000004000ull) + ((offset) & 63) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_XAQX_HEAD_NEXT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_SSO_XAQX_HEAD_NEXT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016700000A0000ull) + ((offset) & 255) * 8;
}
#else
#define CVMX_SSO_XAQX_HEAD_NEXT(offset) (CVMX_ADD_IO_SEG(0x00016700000A0000ull) + ((offset) & 255) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_XAQX_HEAD_PTR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_SSO_XAQX_HEAD_PTR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001670000080000ull) + ((offset) & 255) * 8;
}
#else
#define CVMX_SSO_XAQX_HEAD_PTR(offset) (CVMX_ADD_IO_SEG(0x0001670000080000ull) + ((offset) & 255) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_XAQX_TAIL_NEXT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_SSO_XAQX_TAIL_NEXT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00016700000B0000ull) + ((offset) & 255) * 8;
}
#else
#define CVMX_SSO_XAQX_TAIL_NEXT(offset) (CVMX_ADD_IO_SEG(0x00016700000B0000ull) + ((offset) & 255) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_SSO_XAQX_TAIL_PTR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 63))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 255))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 63)))))
		cvmx_warn("CVMX_SSO_XAQX_TAIL_PTR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001670000090000ull) + ((offset) & 255) * 8;
}
#else
#define CVMX_SSO_XAQX_TAIL_PTR(offset) (CVMX_ADD_IO_SEG(0x0001670000090000ull) + ((offset) & 255) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_XAQ_AURA CVMX_SSO_XAQ_AURA_FUNC()
static inline uint64_t CVMX_SSO_XAQ_AURA_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SSO_XAQ_AURA not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001670000002100ull);
}
#else
#define CVMX_SSO_XAQ_AURA (CVMX_ADD_IO_SEG(0x0001670000002100ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_XAQ_LATENCY_PC CVMX_SSO_XAQ_LATENCY_PC_FUNC()
static inline uint64_t CVMX_SSO_XAQ_LATENCY_PC_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SSO_XAQ_LATENCY_PC not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016700000020B8ull);
}
#else
#define CVMX_SSO_XAQ_LATENCY_PC (CVMX_ADD_IO_SEG(0x00016700000020B8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_SSO_XAQ_REQ_PC CVMX_SSO_XAQ_REQ_PC_FUNC()
static inline uint64_t CVMX_SSO_XAQ_REQ_PC_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN73XX) || OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) || OCTEON_IS_MODEL(OCTEON_CNF75XX)))
		cvmx_warn("CVMX_SSO_XAQ_REQ_PC not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00016700000020B0ull);
}
#else
#define CVMX_SSO_XAQ_REQ_PC (CVMX_ADD_IO_SEG(0x00016700000020B0ull))
#endif

/**
 * cvmx_sso_active_cycles
 *
 * SSO_ACTIVE_CYCLES = SSO cycles SSO active
 *
 * This register counts every sclk cycle that the SSO clocks are active.
 * **NOTE: Added in pass 2.0
 */
union cvmx_sso_active_cycles {
	uint64_t u64;
	struct cvmx_sso_active_cycles_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t act_cyc                      : 64; /**< Counts number of active cycles. */
#else
	uint64_t act_cyc                      : 64;
#endif
	} s;
	struct cvmx_sso_active_cycles_s       cn68xx;
};
typedef union cvmx_sso_active_cycles cvmx_sso_active_cycles_t;

/**
 * cvmx_sso_active_cycles#
 *
 * This register counts every coprocessor clock (SCLK) cycle that the SSO clocks are active.
 *
 */
union cvmx_sso_active_cyclesx {
	uint64_t u64;
	struct cvmx_sso_active_cyclesx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t act_cyc                      : 64; /**< Counts the number of active cycles in each conditional clock domain. */
#else
	uint64_t act_cyc                      : 64;
#endif
	} s;
	struct cvmx_sso_active_cyclesx_s      cn73xx;
	struct cvmx_sso_active_cyclesx_s      cn78xx;
	struct cvmx_sso_active_cyclesx_s      cn78xxp1;
	struct cvmx_sso_active_cyclesx_s      cnf75xx;
};
typedef union cvmx_sso_active_cyclesx cvmx_sso_active_cyclesx_t;

/**
 * cvmx_sso_aw_add
 */
union cvmx_sso_aw_add {
	uint64_t u64;
	struct cvmx_sso_aw_add_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_30_63               : 34;
	uint64_t rsvd_free                    : 14; /**< Written value is added to SSO_AW_WE[RSVD_FREE] to prevent races between software and
                                                         hardware changes. This is a two's complement value so subtraction may also be performed. */
	uint64_t reserved_0_15                : 16;
#else
	uint64_t reserved_0_15                : 16;
	uint64_t rsvd_free                    : 14;
	uint64_t reserved_30_63               : 34;
#endif
	} s;
	struct cvmx_sso_aw_add_s              cn73xx;
	struct cvmx_sso_aw_add_s              cn78xx;
	struct cvmx_sso_aw_add_s              cn78xxp1;
	struct cvmx_sso_aw_add_s              cnf75xx;
};
typedef union cvmx_sso_aw_add cvmx_sso_aw_add_t;

/**
 * cvmx_sso_aw_cfg
 *
 * This register controls the operation of the add-work block (AW).
 *
 */
union cvmx_sso_aw_cfg {
	uint64_t u64;
	struct cvmx_sso_aw_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t ldt_short                    : 1;  /**< Use LDT to bypass L2 allocations when reading short form work. */
	uint64_t lol                          : 1;  /**< Reserved. */
	uint64_t xaq_alloc_dis                : 1;  /**< Disable FPA alloc requests to fill the SSO page cache. Also all existing cached free
                                                         buffers will be returned to FPA and will not be cached. */
	uint64_t ocla_bp                      : 1;  /**< OCLA backpressure enable. When OCLA FIFOs are near full, allow OCLA to backpressure AW pipeline. */
	uint64_t xaq_byp_dis                  : 1;  /**< Disable bypass path in add-work engine. For diagnostic use only. */
	uint64_t stt                          : 1;  /**< Use STT to bypass L2 allocation for XAQ store operations. */
	uint64_t ldt                          : 1;  /**< Use LDT to bypass L2 allocation for XAQ load operations. */
	uint64_t ldwb                         : 1;  /**< When reading XAQ cache lines, use LDWB transactions to invalidate the cache line. */
	uint64_t rwen                         : 1;  /**< Enable XAQ operations. This bit should be set after SSO_XAQ()_HEAD_PTR and
                                                         SSO_XAQ()_TAIL_PTR have been programmed. If cleared, all cached buffers will be
                                                         returned from the FPA as soon as possible, and TAQ arbitration is simplified. */
#else
	uint64_t rwen                         : 1;
	uint64_t ldwb                         : 1;
	uint64_t ldt                          : 1;
	uint64_t stt                          : 1;
	uint64_t xaq_byp_dis                  : 1;
	uint64_t ocla_bp                      : 1;
	uint64_t xaq_alloc_dis                : 1;
	uint64_t lol                          : 1;
	uint64_t ldt_short                    : 1;
	uint64_t reserved_9_63                : 55;
#endif
	} s;
	struct cvmx_sso_aw_cfg_s              cn73xx;
	struct cvmx_sso_aw_cfg_s              cn78xx;
	struct cvmx_sso_aw_cfg_s              cn78xxp1;
	struct cvmx_sso_aw_cfg_s              cnf75xx;
};
typedef union cvmx_sso_aw_cfg cvmx_sso_aw_cfg_t;

/**
 * cvmx_sso_aw_eco
 */
union cvmx_sso_aw_eco {
	uint64_t u64;
	struct cvmx_sso_aw_eco_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t eco_rw                       : 8;  /**< N/A */
#else
	uint64_t eco_rw                       : 8;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_sso_aw_eco_s              cn73xx;
	struct cvmx_sso_aw_eco_s              cnf75xx;
};
typedef union cvmx_sso_aw_eco cvmx_sso_aw_eco_t;

/**
 * cvmx_sso_aw_read_arb
 *
 * This register fine tunes the AW read arbiter and is for diagnostic use.
 *
 */
union cvmx_sso_aw_read_arb {
	uint64_t u64;
	struct cvmx_sso_aw_read_arb_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_30_63               : 34;
	uint64_t xaq_lev                      : 6;  /**< Current number of XAQ reads outstanding. */
	uint64_t reserved_21_23               : 3;
	uint64_t xaq_min                      : 5;  /**< Number of read slots reserved for XAQ exclusive use. Values > 16 will not result in
                                                         additional XAQ reads in flight, but will reduce maximum AW_TAG reads in flight. */
	uint64_t reserved_14_15               : 2;
	uint64_t aw_tag_lev                   : 6;  /**< Current number of tag reads outstanding. */
	uint64_t reserved_5_7                 : 3;
	uint64_t aw_tag_min                   : 5;  /**< Number of read slots reserved for AQ tag read exclusive use. */
#else
	uint64_t aw_tag_min                   : 5;
	uint64_t reserved_5_7                 : 3;
	uint64_t aw_tag_lev                   : 6;
	uint64_t reserved_14_15               : 2;
	uint64_t xaq_min                      : 5;
	uint64_t reserved_21_23               : 3;
	uint64_t xaq_lev                      : 6;
	uint64_t reserved_30_63               : 34;
#endif
	} s;
	struct cvmx_sso_aw_read_arb_s         cn73xx;
	struct cvmx_sso_aw_read_arb_s         cn78xx;
	struct cvmx_sso_aw_read_arb_s         cn78xxp1;
	struct cvmx_sso_aw_read_arb_s         cnf75xx;
};
typedef union cvmx_sso_aw_read_arb cvmx_sso_aw_read_arb_t;

/**
 * cvmx_sso_aw_status
 *
 * This register indicates the status of the add-work block (AW).
 *
 */
union cvmx_sso_aw_status {
	uint64_t u64;
	struct cvmx_sso_aw_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t xaq_buf_cached               : 6;  /**< Indicates number of FPA buffers cached inside SSO. */
#else
	uint64_t xaq_buf_cached               : 6;
	uint64_t reserved_6_63                : 58;
#endif
	} s;
	struct cvmx_sso_aw_status_s           cn73xx;
	struct cvmx_sso_aw_status_s           cn78xx;
	struct cvmx_sso_aw_status_s           cn78xxp1;
	struct cvmx_sso_aw_status_s           cnf75xx;
};
typedef union cvmx_sso_aw_status cvmx_sso_aw_status_t;

/**
 * cvmx_sso_aw_tag_latency_pc
 */
union cvmx_sso_aw_tag_latency_pc {
	uint64_t u64;
	struct cvmx_sso_aw_tag_latency_pc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t count                        : 64; /**< Number of cycles waiting for tag read returns. This may be divided by SSO_AW_TAG_REQ_PC to
                                                         determine the average read latency. */
#else
	uint64_t count                        : 64;
#endif
	} s;
	struct cvmx_sso_aw_tag_latency_pc_s   cn73xx;
	struct cvmx_sso_aw_tag_latency_pc_s   cn78xx;
	struct cvmx_sso_aw_tag_latency_pc_s   cn78xxp1;
	struct cvmx_sso_aw_tag_latency_pc_s   cnf75xx;
};
typedef union cvmx_sso_aw_tag_latency_pc cvmx_sso_aw_tag_latency_pc_t;

/**
 * cvmx_sso_aw_tag_req_pc
 */
union cvmx_sso_aw_tag_req_pc {
	uint64_t u64;
	struct cvmx_sso_aw_tag_req_pc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t count                        : 64; /**< Number of tag read requests. */
#else
	uint64_t count                        : 64;
#endif
	} s;
	struct cvmx_sso_aw_tag_req_pc_s       cn73xx;
	struct cvmx_sso_aw_tag_req_pc_s       cn78xx;
	struct cvmx_sso_aw_tag_req_pc_s       cn78xxp1;
	struct cvmx_sso_aw_tag_req_pc_s       cnf75xx;
};
typedef union cvmx_sso_aw_tag_req_pc cvmx_sso_aw_tag_req_pc_t;

/**
 * cvmx_sso_aw_we
 */
union cvmx_sso_aw_we {
	uint64_t u64;
	struct cvmx_sso_aw_we_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_29_63               : 35;
	uint64_t rsvd_free                    : 13; /**< Number of free reserved entries. Used to ensure that each group can get a specific number
                                                         of entries. Must always be greater than or equal to the sum across all
                                                         SSO_GRP()_IAQ_THR[RSVD_THR], and will generally be equal to that sum unless changes
                                                         to RSVD_THR are going to be made. To prevent races, software should not change this
                                                         register when SSO is being used; instead use SSO_AW_ADD[RSVD_FREE]. */
	uint64_t reserved_13_15               : 3;
	uint64_t free_cnt                     : 13; /**< Number of total free entries. */
#else
	uint64_t free_cnt                     : 13;
	uint64_t reserved_13_15               : 3;
	uint64_t rsvd_free                    : 13;
	uint64_t reserved_29_63               : 35;
#endif
	} s;
	struct cvmx_sso_aw_we_s               cn73xx;
	struct cvmx_sso_aw_we_s               cn78xx;
	struct cvmx_sso_aw_we_s               cn78xxp1;
	struct cvmx_sso_aw_we_s               cnf75xx;
};
typedef union cvmx_sso_aw_we cvmx_sso_aw_we_t;

/**
 * cvmx_sso_bist_stat
 *
 * SSO_BIST_STAT = SSO BIST Status Register
 *
 * Contains the BIST status for the SSO memories ('0' = pass, '1' = fail).
 * Note that PP BIST status is not reported here as it was in previous designs.
 *
 *   There may be more for DDR interface buffers.
 *   It's possible that a RAM will be used for SSO_PP_QOS_RND.
 */
union cvmx_sso_bist_stat {
	uint64_t u64;
	struct cvmx_sso_bist_stat_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_62_63               : 2;
	uint64_t odu_pref                     : 2;  /**< ODU Prefetch memory BIST status */
	uint64_t reserved_54_59               : 6;
	uint64_t fptr                         : 2;  /**< FPTR memory BIST status */
	uint64_t reserved_45_51               : 7;
	uint64_t rwo_dat                      : 1;  /**< RWO_DAT memory BIST status */
	uint64_t rwo                          : 2;  /**< RWO memory BIST status */
	uint64_t reserved_35_41               : 7;
	uint64_t rwi_dat                      : 1;  /**< RWI_DAT memory BIST status */
	uint64_t reserved_32_33               : 2;
	uint64_t soc                          : 1;  /**< SSO CAM BIST status */
	uint64_t reserved_28_30               : 3;
	uint64_t ncbo                         : 4;  /**< NCBO transmitter memory BIST status */
	uint64_t reserved_21_23               : 3;
	uint64_t index                        : 1;  /**< Index memory BIST status */
	uint64_t reserved_17_19               : 3;
	uint64_t fidx                         : 1;  /**< Forward index memory BIST status */
	uint64_t reserved_10_15               : 6;
	uint64_t pend                         : 2;  /**< Pending switch memory BIST status */
	uint64_t reserved_2_7                 : 6;
	uint64_t oth                          : 2;  /**< WQP, GRP memory BIST status */
#else
	uint64_t oth                          : 2;
	uint64_t reserved_2_7                 : 6;
	uint64_t pend                         : 2;
	uint64_t reserved_10_15               : 6;
	uint64_t fidx                         : 1;
	uint64_t reserved_17_19               : 3;
	uint64_t index                        : 1;
	uint64_t reserved_21_23               : 3;
	uint64_t ncbo                         : 4;
	uint64_t reserved_28_30               : 3;
	uint64_t soc                          : 1;
	uint64_t reserved_32_33               : 2;
	uint64_t rwi_dat                      : 1;
	uint64_t reserved_35_41               : 7;
	uint64_t rwo                          : 2;
	uint64_t rwo_dat                      : 1;
	uint64_t reserved_45_51               : 7;
	uint64_t fptr                         : 2;
	uint64_t reserved_54_59               : 6;
	uint64_t odu_pref                     : 2;
	uint64_t reserved_62_63               : 2;
#endif
	} s;
	struct cvmx_sso_bist_stat_s           cn68xx;
	struct cvmx_sso_bist_stat_cn68xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_54_63               : 10;
	uint64_t fptr                         : 2;  /**< FPTR memory BIST status */
	uint64_t reserved_45_51               : 7;
	uint64_t rwo_dat                      : 1;  /**< RWO_DAT memory BIST status */
	uint64_t rwo                          : 2;  /**< RWO memory BIST status */
	uint64_t reserved_35_41               : 7;
	uint64_t rwi_dat                      : 1;  /**< RWI_DAT memory BIST status */
	uint64_t reserved_32_33               : 2;
	uint64_t soc                          : 1;  /**< SSO CAM BIST status */
	uint64_t reserved_28_30               : 3;
	uint64_t ncbo                         : 4;  /**< NCBO transmitter memory BIST status */
	uint64_t reserved_21_23               : 3;
	uint64_t index                        : 1;  /**< Index memory BIST status */
	uint64_t reserved_17_19               : 3;
	uint64_t fidx                         : 1;  /**< Forward index memory BIST status */
	uint64_t reserved_10_15               : 6;
	uint64_t pend                         : 2;  /**< Pending switch memory BIST status */
	uint64_t reserved_2_7                 : 6;
	uint64_t oth                          : 2;  /**< WQP, GRP memory BIST status */
#else
	uint64_t oth                          : 2;
	uint64_t reserved_2_7                 : 6;
	uint64_t pend                         : 2;
	uint64_t reserved_10_15               : 6;
	uint64_t fidx                         : 1;
	uint64_t reserved_17_19               : 3;
	uint64_t index                        : 1;
	uint64_t reserved_21_23               : 3;
	uint64_t ncbo                         : 4;
	uint64_t reserved_28_30               : 3;
	uint64_t soc                          : 1;
	uint64_t reserved_32_33               : 2;
	uint64_t rwi_dat                      : 1;
	uint64_t reserved_35_41               : 7;
	uint64_t rwo                          : 2;
	uint64_t rwo_dat                      : 1;
	uint64_t reserved_45_51               : 7;
	uint64_t fptr                         : 2;
	uint64_t reserved_54_63               : 10;
#endif
	} cn68xxp1;
};
typedef union cvmx_sso_bist_stat cvmx_sso_bist_stat_t;

/**
 * cvmx_sso_bist_status0
 *
 * Contains the BIST status for the SSO memories.
 *
 */
union cvmx_sso_bist_status0 {
	uint64_t u64;
	struct cvmx_sso_bist_status0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t bist                         : 10; /**< Memory BIST status.
                                                         0 = Pass.
                                                         1 = Fail. */
#else
	uint64_t bist                         : 10;
	uint64_t reserved_10_63               : 54;
#endif
	} s;
	struct cvmx_sso_bist_status0_s        cn73xx;
	struct cvmx_sso_bist_status0_s        cn78xx;
	struct cvmx_sso_bist_status0_s        cn78xxp1;
	struct cvmx_sso_bist_status0_s        cnf75xx;
};
typedef union cvmx_sso_bist_status0 cvmx_sso_bist_status0_t;

/**
 * cvmx_sso_bist_status1
 *
 * Contains the BIST status for the SSO memories.
 *
 */
union cvmx_sso_bist_status1 {
	uint64_t u64;
	struct cvmx_sso_bist_status1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t bist                         : 7;  /**< Memory BIST status.
                                                         0 = Pass.
                                                         1 = Fail. */
#else
	uint64_t bist                         : 7;
	uint64_t reserved_7_63                : 57;
#endif
	} s;
	struct cvmx_sso_bist_status1_s        cn73xx;
	struct cvmx_sso_bist_status1_s        cn78xx;
	struct cvmx_sso_bist_status1_s        cn78xxp1;
	struct cvmx_sso_bist_status1_s        cnf75xx;
};
typedef union cvmx_sso_bist_status1 cvmx_sso_bist_status1_t;

/**
 * cvmx_sso_bist_status2
 *
 * Contains the BIST status for the SSO memories.
 *
 */
union cvmx_sso_bist_status2 {
	uint64_t u64;
	struct cvmx_sso_bist_status2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t bist                         : 9;  /**< Memory BIST status.
                                                         0 = Pass.
                                                         1 = Fail. */
#else
	uint64_t bist                         : 9;
	uint64_t reserved_9_63                : 55;
#endif
	} s;
	struct cvmx_sso_bist_status2_s        cn73xx;
	struct cvmx_sso_bist_status2_s        cn78xx;
	struct cvmx_sso_bist_status2_s        cn78xxp1;
	struct cvmx_sso_bist_status2_s        cnf75xx;
};
typedef union cvmx_sso_bist_status2 cvmx_sso_bist_status2_t;

/**
 * cvmx_sso_cfg
 *
 * SSO_CFG = SSO Config
 *
 * This register is an assortment of various SSO configuration bits.
 */
union cvmx_sso_cfg {
	uint64_t u64;
	struct cvmx_sso_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t qck_gw_rsp_adj               : 3;  /**< Fast GET_WORK response fine adjustment
                                                         Allowed values are 0, 1, and 2 (0 is quickest) */
	uint64_t qck_gw_rsp_dis               : 1;  /**< Disable faster response to GET_WORK */
	uint64_t qck_sw_dis                   : 1;  /**< Disable faster switch to UNSCHEDULED on GET_WORK */
	uint64_t rwq_alloc_dis                : 1;  /**< Disable FPA Alloc Requests when SSO_FPAGE_CNT < 16 */
	uint64_t soc_ccam_dis                 : 1;  /**< Disable power saving SOC conditional CAM
                                                         (**NOTE: Added in pass 2.0) */
	uint64_t sso_cclk_dis                 : 1;  /**< Disable power saving SSO conditional clocking
                                                         (**NOTE: Added in pass 2.0) */
	uint64_t rwo_flush                    : 1;  /**< Flush RWO engine
                                                         Allows outbound NCB entries to go immediately rather
                                                         than waiting for a complete fill packet. This register
                                                         is one-shot and clears itself each time it is set. */
	uint64_t wfe_thr                      : 1;  /**< Use 1 Work-fetch engine (instead of 4) */
	uint64_t rwio_byp_dis                 : 1;  /**< Disable Bypass path in RWI/RWO Engines */
	uint64_t rwq_byp_dis                  : 1;  /**< Disable Bypass path in RWQ Engine */
	uint64_t stt                          : 1;  /**< STT Setting for RW Stores */
	uint64_t ldt                          : 1;  /**< LDT Setting for RW Loads */
	uint64_t dwb                          : 1;  /**< DWB Setting for Return Page Requests
                                                         1 = 2 128B cache pages to issue DWB for
                                                         0 = 0 128B cache pages ro issue DWB for */
	uint64_t rwen                         : 1;  /**< Enable RWI/RWO operations
                                                         This bit should be set after SSO_RWQ_HEAD_PTRX and
                                                         SSO_RWQ_TAIL_PTRX have been programmed. */
#else
	uint64_t rwen                         : 1;
	uint64_t dwb                          : 1;
	uint64_t ldt                          : 1;
	uint64_t stt                          : 1;
	uint64_t rwq_byp_dis                  : 1;
	uint64_t rwio_byp_dis                 : 1;
	uint64_t wfe_thr                      : 1;
	uint64_t rwo_flush                    : 1;
	uint64_t sso_cclk_dis                 : 1;
	uint64_t soc_ccam_dis                 : 1;
	uint64_t rwq_alloc_dis                : 1;
	uint64_t qck_sw_dis                   : 1;
	uint64_t qck_gw_rsp_dis               : 1;
	uint64_t qck_gw_rsp_adj               : 3;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_sso_cfg_s                 cn68xx;
	struct cvmx_sso_cfg_cn68xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t rwo_flush                    : 1;  /**< Flush RWO engine
                                                         Allows outbound NCB entries to go immediately rather
                                                         than waiting for a complete fill packet. This register
                                                         is one-shot and clears itself each time it is set. */
	uint64_t wfe_thr                      : 1;  /**< Use 1 Work-fetch engine (instead of 4) */
	uint64_t rwio_byp_dis                 : 1;  /**< Disable Bypass path in RWI/RWO Engines */
	uint64_t rwq_byp_dis                  : 1;  /**< Disable Bypass path in RWQ Engine */
	uint64_t stt                          : 1;  /**< STT Setting for RW Stores */
	uint64_t ldt                          : 1;  /**< LDT Setting for RW Loads */
	uint64_t dwb                          : 1;  /**< DWB Setting for Return Page Requests
                                                         1 = 2 128B cache pages to issue DWB for
                                                         0 = 0 128B cache pages ro issue DWB for */
	uint64_t rwen                         : 1;  /**< Enable RWI/RWO operations
                                                         This bit should be set after SSO_RWQ_HEAD_PTRX and
                                                         SSO_RWQ_TAIL_PTRX have been programmed. */
#else
	uint64_t rwen                         : 1;
	uint64_t dwb                          : 1;
	uint64_t ldt                          : 1;
	uint64_t stt                          : 1;
	uint64_t rwq_byp_dis                  : 1;
	uint64_t rwio_byp_dis                 : 1;
	uint64_t wfe_thr                      : 1;
	uint64_t rwo_flush                    : 1;
	uint64_t reserved_8_63                : 56;
#endif
	} cn68xxp1;
};
typedef union cvmx_sso_cfg cvmx_sso_cfg_t;

/**
 * cvmx_sso_ds_pc
 *
 * SSO_DS_PC = SSO De-Schedule Performance Counter
 *
 * Counts the number of de-schedule requests.
 * Counter rolls over through zero when max value exceeded.
 */
union cvmx_sso_ds_pc {
	uint64_t u64;
	struct cvmx_sso_ds_pc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ds_pc                        : 64; /**< De-schedule performance counter */
#else
	uint64_t ds_pc                        : 64;
#endif
	} s;
	struct cvmx_sso_ds_pc_s               cn68xx;
	struct cvmx_sso_ds_pc_s               cn68xxp1;
};
typedef union cvmx_sso_ds_pc cvmx_sso_ds_pc_t;

/**
 * cvmx_sso_ecc_ctl0
 */
union cvmx_sso_ecc_ctl0 {
	uint64_t u64;
	struct cvmx_sso_ecc_ctl0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_30_63               : 34;
	uint64_t toaqt_flip                   : 2;  /**< TOAQT flip syndrome bits on write. */
	uint64_t toaqt_cdis                   : 1;  /**< TOAQT ECC correction disable. */
	uint64_t toaqh_flip                   : 2;  /**< TOAQH flip syndrome bits on write. */
	uint64_t toaqh_cdis                   : 1;  /**< TOAQH ECC correction disable. */
	uint64_t tiaqt_flip                   : 2;  /**< TIAQT flip syndrome bits on write. */
	uint64_t tiaqt_cdis                   : 1;  /**< TIAQT ECC correction disable. */
	uint64_t tiaqh_flip                   : 2;  /**< TIAQH flip syndrome bits on write. */
	uint64_t tiaqh_cdis                   : 1;  /**< TIAQH ECC correction disable. */
	uint64_t llm_flip                     : 2;  /**< LLM flip syndrome bits on write. */
	uint64_t llm_cdis                     : 1;  /**< LLM ECC correction disable. */
	uint64_t inp_flip                     : 2;  /**< INP flip syndrome bits on write. */
	uint64_t inp_cdis                     : 1;  /**< INP ECC correction disable. */
	uint64_t qtc_flip                     : 2;  /**< QTC flip syndrome bits on write. */
	uint64_t qtc_cdis                     : 1;  /**< QTC ECC correction disable. */
	uint64_t xaq_flip                     : 2;  /**< XAQ flip syndrome bits on write. */
	uint64_t xaq_cdis                     : 1;  /**< XAQ ECC correction disable. */
	uint64_t fff_flip                     : 2;  /**< FFF flip syndrome bits on write. */
	uint64_t fff_cdis                     : 1;  /**< FFF ECC correction disable. */
	uint64_t wes_flip                     : 2;  /**< WES flip syndrome bits on write. Flip syndrome bits <1:0> on writes to the ram to test
                                                         single-bit or double-bit error handling. */
	uint64_t wes_cdis                     : 1;  /**< WES ECC correction disable. */
#else
	uint64_t wes_cdis                     : 1;
	uint64_t wes_flip                     : 2;
	uint64_t fff_cdis                     : 1;
	uint64_t fff_flip                     : 2;
	uint64_t xaq_cdis                     : 1;
	uint64_t xaq_flip                     : 2;
	uint64_t qtc_cdis                     : 1;
	uint64_t qtc_flip                     : 2;
	uint64_t inp_cdis                     : 1;
	uint64_t inp_flip                     : 2;
	uint64_t llm_cdis                     : 1;
	uint64_t llm_flip                     : 2;
	uint64_t tiaqh_cdis                   : 1;
	uint64_t tiaqh_flip                   : 2;
	uint64_t tiaqt_cdis                   : 1;
	uint64_t tiaqt_flip                   : 2;
	uint64_t toaqh_cdis                   : 1;
	uint64_t toaqh_flip                   : 2;
	uint64_t toaqt_cdis                   : 1;
	uint64_t toaqt_flip                   : 2;
	uint64_t reserved_30_63               : 34;
#endif
	} s;
	struct cvmx_sso_ecc_ctl0_s            cn73xx;
	struct cvmx_sso_ecc_ctl0_s            cn78xx;
	struct cvmx_sso_ecc_ctl0_s            cn78xxp1;
	struct cvmx_sso_ecc_ctl0_s            cnf75xx;
};
typedef union cvmx_sso_ecc_ctl0 cvmx_sso_ecc_ctl0_t;

/**
 * cvmx_sso_ecc_ctl1
 */
union cvmx_sso_ecc_ctl1 {
	uint64_t u64;
	struct cvmx_sso_ecc_ctl1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_21_63               : 43;
	uint64_t thrint_flip                  : 2;  /**< THRINT flip syndrome bits on write. */
	uint64_t thrint_cdis                  : 1;  /**< THRINT ECC correction disable. */
	uint64_t mask_flip                    : 2;  /**< MASK flip syndrome bits on write. */
	uint64_t mask_cdis                    : 1;  /**< MASK ECC correction disable. */
	uint64_t gdw_flip                     : 2;  /**< GDW flip syndrome bits on write. */
	uint64_t gdw_cdis                     : 1;  /**< GDW ECC correction disable. */
	uint64_t qidx_flip                    : 2;  /**< QIDX flip syndrome bits on write. */
	uint64_t qidx_cdis                    : 1;  /**< QIDX ECC correction disable. */
	uint64_t tptr_flip                    : 2;  /**< TPTR flip syndrome bits on write. */
	uint64_t tptr_cdis                    : 1;  /**< TPTR ECC correction disable. */
	uint64_t hptr_flip                    : 2;  /**< HPTR flip syndrome bits on write. */
	uint64_t hptr_cdis                    : 1;  /**< HPTR ECC correction disable. */
	uint64_t cntr_flip                    : 2;  /**< CNTR flip syndrome bits on write. Flip syndrome bits <1:0> on writes to the ram to test
                                                         single-bit or double-bit error handling. */
	uint64_t cntr_cdis                    : 1;  /**< CNTR ECC correction disable. */
#else
	uint64_t cntr_cdis                    : 1;
	uint64_t cntr_flip                    : 2;
	uint64_t hptr_cdis                    : 1;
	uint64_t hptr_flip                    : 2;
	uint64_t tptr_cdis                    : 1;
	uint64_t tptr_flip                    : 2;
	uint64_t qidx_cdis                    : 1;
	uint64_t qidx_flip                    : 2;
	uint64_t gdw_cdis                     : 1;
	uint64_t gdw_flip                     : 2;
	uint64_t mask_cdis                    : 1;
	uint64_t mask_flip                    : 2;
	uint64_t thrint_cdis                  : 1;
	uint64_t thrint_flip                  : 2;
	uint64_t reserved_21_63               : 43;
#endif
	} s;
	struct cvmx_sso_ecc_ctl1_s            cn73xx;
	struct cvmx_sso_ecc_ctl1_s            cn78xx;
	struct cvmx_sso_ecc_ctl1_s            cn78xxp1;
	struct cvmx_sso_ecc_ctl1_s            cnf75xx;
};
typedef union cvmx_sso_ecc_ctl1 cvmx_sso_ecc_ctl1_t;

/**
 * cvmx_sso_ecc_ctl2
 */
union cvmx_sso_ecc_ctl2 {
	uint64_t u64;
	struct cvmx_sso_ecc_ctl2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_15_63               : 49;
	uint64_t ncbo_flip                    : 2;  /**< NCBO flip syndrome bits on write. */
	uint64_t ncbo_cdis                    : 1;  /**< NCBO ECC correction disable. */
	uint64_t pnd_flip                     : 2;  /**< PND flip syndrome bits on write. */
	uint64_t pnd_cdis                     : 1;  /**< PND ECC correction disable. */
	uint64_t oth_flip                     : 2;  /**< OTH flip syndrome bits on write. */
	uint64_t oth_cdis                     : 1;  /**< OTH ECC correction disable. */
	uint64_t nidx_flip                    : 2;  /**< NIDX flip syndrome bits on write. */
	uint64_t nidx_cdis                    : 1;  /**< NIDX ECC correction disable. */
	uint64_t pidx_flip                    : 2;  /**< PIDX flip syndrome bits on write. Flip syndrome bits <1:0> on writes to the ram to test
                                                         single-bit or double-bit error handling. */
	uint64_t pidx_cdis                    : 1;  /**< PIDX ECC correction disable. */
#else
	uint64_t pidx_cdis                    : 1;
	uint64_t pidx_flip                    : 2;
	uint64_t nidx_cdis                    : 1;
	uint64_t nidx_flip                    : 2;
	uint64_t oth_cdis                     : 1;
	uint64_t oth_flip                     : 2;
	uint64_t pnd_cdis                     : 1;
	uint64_t pnd_flip                     : 2;
	uint64_t ncbo_cdis                    : 1;
	uint64_t ncbo_flip                    : 2;
	uint64_t reserved_15_63               : 49;
#endif
	} s;
	struct cvmx_sso_ecc_ctl2_s            cn73xx;
	struct cvmx_sso_ecc_ctl2_s            cn78xx;
	struct cvmx_sso_ecc_ctl2_s            cn78xxp1;
	struct cvmx_sso_ecc_ctl2_s            cnf75xx;
};
typedef union cvmx_sso_ecc_ctl2 cvmx_sso_ecc_ctl2_t;

/**
 * cvmx_sso_err
 *
 * SSO_ERR = SSO Error Register
 *
 * Contains ECC and other misc error bits.
 *
 * <45> The free page error bit will assert when SSO_FPAGE_CNT <= 16 and
 *      SSO_CFG[RWEN] is 1.  Software will want to disable the interrupt
 *      associated with this error when recovering SSO pointers from the
 *      FPA and SSO.
 *
 * This register also contains the illegal operation error bits:
 *
 * <42> Received ADDWQ with tag specified as EMPTY
 * <41> Received illegal opcode
 * <40> Received SWTAG/SWTAG_FULL/SWTAG_DESCH/DESCH/UPD_WQP/GET_WORK/ALLOC_WE
 *      from WS with CLR_NSCHED pending
 * <39> Received CLR_NSCHED
 *      from WS with SWTAG_DESCH/DESCH/CLR_NSCHED pending
 * <38> Received SWTAG/SWTAG_FULL/SWTAG_DESCH/DESCH/UPD_WQP/GET_WORK/ALLOC_WE
 *      from WS with ALLOC_WE pending
 * <37> Received SWTAG/SWTAG_FULL/SWTAG_DESCH/DESCH/UPD_WQP/GET_WORK/ALLOC_WE/CLR_NSCHED
 *      from WS with GET_WORK pending
 * <36> Received SWTAG_FULL/SWTAG_DESCH
 *      with tag specified as UNSCHEDULED
 * <35> Received SWTAG/SWTAG_FULL/SWTAG_DESCH
 *      with tag specified as EMPTY
 * <34> Received SWTAG/SWTAG_FULL/SWTAG_DESCH/GET_WORK
 *      from WS with pending tag switch to ORDERED or ATOMIC
 * <33> Received SWTAG/SWTAG_DESCH/DESCH/UPD_WQP
 *      from WS in UNSCHEDULED state
 * <32> Received SWTAG/SWTAG_FULL/SWTAG_DESCH/DESCH/UPD_WQP
 *      from WS in EMPTY state
 */
union cvmx_sso_err {
	uint64_t u64;
	struct cvmx_sso_err_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t bfp                          : 1;  /**< Bad Fill Packet error
                                                         Last byte of the fill packet did not match 8'h1a */
	uint64_t awe                          : 1;  /**< Out-of-memory error (ADDWQ Request is dropped) */
	uint64_t fpe                          : 1;  /**< Free page error */
	uint64_t reserved_43_44               : 2;
	uint64_t iop                          : 11; /**< Illegal operation errors */
	uint64_t reserved_12_31               : 20;
	uint64_t pnd_dbe0                     : 1;  /**< Double bit error for even PND RAM */
	uint64_t pnd_sbe0                     : 1;  /**< Single bit error for even PND RAM */
	uint64_t pnd_dbe1                     : 1;  /**< Double bit error for odd PND RAM */
	uint64_t pnd_sbe1                     : 1;  /**< Single bit error for odd PND RAM */
	uint64_t oth_dbe0                     : 1;  /**< Double bit error for even OTH RAM */
	uint64_t oth_sbe0                     : 1;  /**< Single bit error for even OTH RAM */
	uint64_t oth_dbe1                     : 1;  /**< Double bit error for odd OTH RAM */
	uint64_t oth_sbe1                     : 1;  /**< Single bit error for odd OTH RAM */
	uint64_t idx_dbe                      : 1;  /**< Double bit error for IDX RAM */
	uint64_t idx_sbe                      : 1;  /**< Single bit error for IDX RAM */
	uint64_t fidx_dbe                     : 1;  /**< Double bit error for FIDX RAM */
	uint64_t fidx_sbe                     : 1;  /**< Single bit error for FIDX RAM */
#else
	uint64_t fidx_sbe                     : 1;
	uint64_t fidx_dbe                     : 1;
	uint64_t idx_sbe                      : 1;
	uint64_t idx_dbe                      : 1;
	uint64_t oth_sbe1                     : 1;
	uint64_t oth_dbe1                     : 1;
	uint64_t oth_sbe0                     : 1;
	uint64_t oth_dbe0                     : 1;
	uint64_t pnd_sbe1                     : 1;
	uint64_t pnd_dbe1                     : 1;
	uint64_t pnd_sbe0                     : 1;
	uint64_t pnd_dbe0                     : 1;
	uint64_t reserved_12_31               : 20;
	uint64_t iop                          : 11;
	uint64_t reserved_43_44               : 2;
	uint64_t fpe                          : 1;
	uint64_t awe                          : 1;
	uint64_t bfp                          : 1;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_sso_err_s                 cn68xx;
	struct cvmx_sso_err_s                 cn68xxp1;
};
typedef union cvmx_sso_err cvmx_sso_err_t;

/**
 * cvmx_sso_err0
 *
 * This register contains ECC and other miscellaneous error bits.
 *
 */
union cvmx_sso_err0 {
	uint64_t u64;
	struct cvmx_sso_err0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_52_63               : 12;
	uint64_t toaqt_dbe                    : 1;  /**< Double-bit error for TOAQT RAM. Throws SSO_INTSN_E::SSO_ERR0_TOAQT_DBE. */
	uint64_t toaqt_sbe                    : 1;  /**< Single-bit error for TOAQT RAM. Throws SSO_INTSN_E::SSO_ERR0_TOAQT_SBE. */
	uint64_t toaqh_dbe                    : 1;  /**< Double-bit error for TOAQH RAM. Throws SSO_INTSN_E::SSO_ERR0_TOAQH_DBE. */
	uint64_t toaqh_sbe                    : 1;  /**< Single-bit error for TOAQH RAM. Throws SSO_INTSN_E::SSO_ERR0_TOAQH_SBE. */
	uint64_t tiaqt_dbe                    : 1;  /**< Double-bit error for TIAQT RAM. Throws SSO_INTSN_E::SSO_ERR0_TIAQT_DBE. */
	uint64_t tiaqt_sbe                    : 1;  /**< Single-bit error for TIAQT RAM. Throws SSO_INTSN_E::SSO_ERR0_TIAQT_SBE. */
	uint64_t tiaqh_dbe                    : 1;  /**< Double-bit error for TIAQH RAM. Throws SSO_INTSN_E::SSO_ERR0_TIAQH_DBE. */
	uint64_t tiaqh_sbe                    : 1;  /**< Single-bit error for TIAQH RAM. Throws SSO_INTSN_E::SSO_ERR0_TIAQH_SBE. */
	uint64_t llm_dbe                      : 1;  /**< Double-bit error for LLM RAM. Throws SSO_INTSN_E::SSO_ERR0_LLM_DBE. */
	uint64_t llm_sbe                      : 1;  /**< Single-bit error for LLM RAM. Throws SSO_INTSN_E::SSO_ERR0_LLM_SBE. */
	uint64_t inp_dbe                      : 1;  /**< Double-bit error for INP RAM. Throws SSO_INTSN_E::SSO_ERR0_INP_DBE. */
	uint64_t inp_sbe                      : 1;  /**< Single-bit error for INP RAM. Throws SSO_INTSN_E::SSO_ERR0_INP_SBE. */
	uint64_t qtc_dbe                      : 1;  /**< Double-bit error for QTC RAM. Throws SSO_INTSN_E::SSO_ERR0_QTC_DBE. */
	uint64_t qtc_sbe                      : 1;  /**< Single-bit error for QTC RAM. Throws SSO_INTSN_E::SSO_ERR0_QTC_SBE. */
	uint64_t xaq_dbe                      : 1;  /**< Double-bit error for XAQ RAM. Throws SSO_INTSN_E::SSO_ERR0_XAQ_DBE. */
	uint64_t xaq_sbe                      : 1;  /**< Single-bit error for XAQ RAM. Throws SSO_INTSN_E::SSO_ERR0_XAQ_SBE. */
	uint64_t fff_dbe                      : 1;  /**< Double-bit error for  RAM. Throws SSO_INTSN_E::SSO_ERR0_FFF_DBE. */
	uint64_t fff_sbe                      : 1;  /**< Single-bit error for  RAM. Throws SSO_INTSN_E::SSO_ERR0_FFF_SBE. */
	uint64_t wes_dbe                      : 1;  /**< Double-bit error for WES RAM. Throws SSO_INTSN_E::SSO_ERR0_WES_DBE. */
	uint64_t wes_sbe                      : 1;  /**< Single-bit error for WES RAM. Throws SSO_INTSN_E::SSO_ERR0_WES_SBE. */
	uint64_t reserved_6_31                : 26;
	uint64_t addwq_dropped                : 1;  /**< Add work dropped due to wrong command/DID requested. Throws
                                                         SSO_INTSN_E::SSO_ERR0_ADDWQ_DROPPED. */
	uint64_t awempty                      : 1;  /**< Received add work with tag specified as EMPTY. Throws SSO_INTSN_E::SSO_ERR0_AWEMPTY. */
	uint64_t grpdis                       : 1;  /**< Add work to disabled group. An ADDWQ was received and dropped to a group with
                                                         SSO_GRP()_IAQ_THR[RSVD_THR] = 0. Throws SSO_INTSN_E::SSO_ERR0_GRPDIS. */
	uint64_t bfp                          : 1;  /**< Bad-fill-packet error. The WAE VLD_CRC field was incorrect, or the XAQ next address was
                                                         zero. Throws SSO_INTSN_E::SSO_ERR0_BFP. */
	uint64_t awe                          : 1;  /**< Out-of-memory error. SSO has dropped some add-work as a result, and this should
                                                         be considered fatal to SSO. Throws SSO_INTSN_E::SSO_ERR0_AWE.
                                                         This may indicate software did not allocate sufficient FPA buffers to cover all
                                                         possible outstanding work. */
	uint64_t fpe                          : 1;  /**< Free-page error. The free page error bit asserts when a new FPA page is
                                                         requested and FPA indicates there are no remaining free pages. SSO will keep
                                                         attempting to allocate pages, and if the situation persists the more critical
                                                         [AWE] error will be indicated. Throws SSO_INTSN_E::SSO_ERR0_FPE. */
#else
	uint64_t fpe                          : 1;
	uint64_t awe                          : 1;
	uint64_t bfp                          : 1;
	uint64_t grpdis                       : 1;
	uint64_t awempty                      : 1;
	uint64_t addwq_dropped                : 1;
	uint64_t reserved_6_31                : 26;
	uint64_t wes_sbe                      : 1;
	uint64_t wes_dbe                      : 1;
	uint64_t fff_sbe                      : 1;
	uint64_t fff_dbe                      : 1;
	uint64_t xaq_sbe                      : 1;
	uint64_t xaq_dbe                      : 1;
	uint64_t qtc_sbe                      : 1;
	uint64_t qtc_dbe                      : 1;
	uint64_t inp_sbe                      : 1;
	uint64_t inp_dbe                      : 1;
	uint64_t llm_sbe                      : 1;
	uint64_t llm_dbe                      : 1;
	uint64_t tiaqh_sbe                    : 1;
	uint64_t tiaqh_dbe                    : 1;
	uint64_t tiaqt_sbe                    : 1;
	uint64_t tiaqt_dbe                    : 1;
	uint64_t toaqh_sbe                    : 1;
	uint64_t toaqh_dbe                    : 1;
	uint64_t toaqt_sbe                    : 1;
	uint64_t toaqt_dbe                    : 1;
	uint64_t reserved_52_63               : 12;
#endif
	} s;
	struct cvmx_sso_err0_s                cn73xx;
	struct cvmx_sso_err0_s                cn78xx;
	struct cvmx_sso_err0_s                cn78xxp1;
	struct cvmx_sso_err0_s                cnf75xx;
};
typedef union cvmx_sso_err0 cvmx_sso_err0_t;

/**
 * cvmx_sso_err1
 *
 * This register contains ECC and other miscellaneous error bits.
 *
 */
union cvmx_sso_err1 {
	uint64_t u64;
	struct cvmx_sso_err1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t thrint_dbe                   : 1;  /**< Double-bit error for THRINT RAM. Throws SSO_INTSN_E::SSO_ERR1_THRINT_DBE. */
	uint64_t thrint_sbe                   : 1;  /**< Single-bit error for THRINT RAM. Throws SSO_INTSN_E::SSO_ERR1_THRINT_SBE. */
	uint64_t mask_dbe                     : 1;  /**< Double-bit error for MASK RAM. Throws SSO_INTSN_E::SSO_ERR1_MASK_DBE. */
	uint64_t mask_sbe                     : 1;  /**< Single-bit error for MASK RAM. Throws SSO_INTSN_E::SSO_ERR1_MASK_SBE. */
	uint64_t gdw_dbe                      : 1;  /**< Double-bit error for GDW RAM. Throws SSO_INTSN_E::SSO_ERR1_GDW_DBE. */
	uint64_t gdw_sbe                      : 1;  /**< Single-bit error for GDW RAM. Throws SSO_INTSN_E::SSO_ERR1_GDW_SBE. */
	uint64_t qidx_dbe                     : 1;  /**< Double-bit error for QIDX RAM. Throws SSO_INTSN_E::SSO_ERR1_QIDX_DBE. */
	uint64_t qidx_sbe                     : 1;  /**< Single-bit error for QIDX RAM. Throws SSO_INTSN_E::SSO_ERR1_QIDX_SBE. */
	uint64_t tptr_dbe                     : 1;  /**< Double-bit error for TPTR RAM. Throws SSO_INTSN_E::SSO_ERR1_TPTR_DBE. */
	uint64_t tptr_sbe                     : 1;  /**< Single-bit error for TPTR RAM. Throws SSO_INTSN_E::SSO_ERR1_TPTR_SBE. */
	uint64_t hptr_dbe                     : 1;  /**< Double-bit error for HPTR RAM. Throws SSO_INTSN_E::SSO_ERR1_HPTR_DBE. */
	uint64_t hptr_sbe                     : 1;  /**< Single-bit error for HPTR RAM. Throws SSO_INTSN_E::SSO_ERR1_HPTR_SBE. */
	uint64_t cntr_dbe                     : 1;  /**< Double-bit error for CNTR RAM. Throws SSO_INTSN_E::SSO_ERR1_CNTR_DBE. */
	uint64_t cntr_sbe                     : 1;  /**< Single-bit error for CNTR RAM. Throws SSO_INTSN_E::SSO_ERR1_CNTR_SBE. */
#else
	uint64_t cntr_sbe                     : 1;
	uint64_t cntr_dbe                     : 1;
	uint64_t hptr_sbe                     : 1;
	uint64_t hptr_dbe                     : 1;
	uint64_t tptr_sbe                     : 1;
	uint64_t tptr_dbe                     : 1;
	uint64_t qidx_sbe                     : 1;
	uint64_t qidx_dbe                     : 1;
	uint64_t gdw_sbe                      : 1;
	uint64_t gdw_dbe                      : 1;
	uint64_t mask_sbe                     : 1;
	uint64_t mask_dbe                     : 1;
	uint64_t thrint_sbe                   : 1;
	uint64_t thrint_dbe                   : 1;
	uint64_t reserved_14_63               : 50;
#endif
	} s;
	struct cvmx_sso_err1_s                cn73xx;
	struct cvmx_sso_err1_s                cn78xx;
	struct cvmx_sso_err1_s                cn78xxp1;
	struct cvmx_sso_err1_s                cnf75xx;
};
typedef union cvmx_sso_err1 cvmx_sso_err1_t;

/**
 * cvmx_sso_err2
 *
 * This register contains ECC and other miscellaneous error bits.
 *
 */
union cvmx_sso_err2 {
	uint64_t u64;
	struct cvmx_sso_err2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_42_63               : 22;
	uint64_t ncbo_dbe                     : 1;  /**< Double-bit error for NCBO RAM. Throws SSO_INTSN_E::SSO_ERR2_NCBO_DBE. */
	uint64_t ncbo_sbe                     : 1;  /**< Single-bit error for NCBO RAM. Throws SSO_INTSN_E::SSO_ERR2_NCBO_SBE. */
	uint64_t pnd_dbe                      : 1;  /**< Double-bit error for PND RAM. Throws SSO_INTSN_E::SSO_ERR2_PND_DBE. */
	uint64_t pnd_sbe                      : 1;  /**< Single-bit error for PND RAM. Throws SSO_INTSN_E::SSO_ERR2_PND_SBE. */
	uint64_t oth_dbe                      : 1;  /**< Double-bit error for odd OTH RAM. Throws SSO_INTSN_E::SSO_ERR2_OTH_DBE. */
	uint64_t oth_sbe                      : 1;  /**< Single-bit error for odd OTH RAM. Throws SSO_INTSN_E::SSO_ERR2_OTH_SBE. */
	uint64_t nidx_dbe                     : 1;  /**< Double-bit error for IDX RAM. Throws SSO_INTSN_E::SSO_ERR2_NIDX_DBE. */
	uint64_t nidx_sbe                     : 1;  /**< Single-bit error for IDX RAM. Throws SSO_INTSN_E::SSO_ERR2_NIDX_SBE. */
	uint64_t pidx_dbe                     : 1;  /**< Double-bit error for FIDX RAM. Throws SSO_INTSN_E::SSO_ERR2_PIDX_DBE. */
	uint64_t pidx_sbe                     : 1;  /**< Single-bit error for FIDX RAM. Throws SSO_INTSN_E::SSO_ERR2_PIDX_SBE. */
	uint64_t reserved_13_31               : 19;
	uint64_t iop                          : 13; /**< Illegal operation errors. Throws SSO_INTSN_E::SSO_ERR2_IOP<n>:
                                                         <12> = Received command before SSO_RESET[BUSY] cleared.
                                                         <11> = Reserved.
                                                         <10> = Reserved.
                                                         <9> = Received illegal opcode.
                                                         <8> = Received SWTAG/SWTAG_FULL/SWTAG_DESCH/DESCH/UPD_WQP_GRP/GET_WORK/ALLOC_WE from work
                                                         slot with CLR_NSCHED pending.
                                                         <7> = Received CLR_NSCHED from work slot with SWTAG_DESCH/DESCH/CLR_NSCHED pending.
                                                         <6> = Received SWTAG/SWTAG_FULL/SWTAG_DESCH/DESCH/UPD_WQP_GRP/GET_WORK/ALLOC_WE/CLR_NSCHED
                                                         from work slot with ALLOC_WE pending.
                                                         <5> = Received SWTAG/SWTAG_FULL/SWTAG_DESCH/DESCH/UPD_WQP_GRP/GET_WORK/ALLOC_WE/CLR_NSCHED
                                                         from work slot with GET_WORK pending.
                                                         <4> = Received SWTAG_FULL/SWTAG_DESCH with tag specified as UNTAGGED.
                                                         <3> = Received SWTAG/SWTAG_FULL/SWTAG_DESCH with tag specified as EMPTY.
                                                         <2> = Received SWTAG/SWTAG_FULL/SWTAG_DESCH/GET_WORK from work slot with pending tag
                                                         switch to ORDERED or ATOMIC.
                                                         <1> = Received SWTAG/SWTAG_DESCH/DESCH/UPD_WQP_GRP from work slot in UNTAGGED state.
                                                         <0> = Received SWTAG/SWTAG_FULL/SWTAG_DESCH/DESCH/UPD_WQP_GRP from work slot in EMPTY
                                                         state. */
#else
	uint64_t iop                          : 13;
	uint64_t reserved_13_31               : 19;
	uint64_t pidx_sbe                     : 1;
	uint64_t pidx_dbe                     : 1;
	uint64_t nidx_sbe                     : 1;
	uint64_t nidx_dbe                     : 1;
	uint64_t oth_sbe                      : 1;
	uint64_t oth_dbe                      : 1;
	uint64_t pnd_sbe                      : 1;
	uint64_t pnd_dbe                      : 1;
	uint64_t ncbo_sbe                     : 1;
	uint64_t ncbo_dbe                     : 1;
	uint64_t reserved_42_63               : 22;
#endif
	} s;
	struct cvmx_sso_err2_s                cn73xx;
	struct cvmx_sso_err2_s                cn78xx;
	struct cvmx_sso_err2_s                cn78xxp1;
	struct cvmx_sso_err2_s                cnf75xx;
};
typedef union cvmx_sso_err2 cvmx_sso_err2_t;

/**
 * cvmx_sso_err_enb
 *
 * SSO_ERR_ENB = SSO Error Enable Register
 *
 * Contains the interrupt enables corresponding to SSO_ERR.
 */
union cvmx_sso_err_enb {
	uint64_t u64;
	struct cvmx_sso_err_enb_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t bfp_ie                       : 1;  /**< Bad Fill Packet error interrupt enable */
	uint64_t awe_ie                       : 1;  /**< Add-work error interrupt enable */
	uint64_t fpe_ie                       : 1;  /**< Free Page error interrupt enable */
	uint64_t reserved_43_44               : 2;
	uint64_t iop_ie                       : 11; /**< Illegal operation interrupt enables */
	uint64_t reserved_12_31               : 20;
	uint64_t pnd_dbe0_ie                  : 1;  /**< Double bit error interrupt enable for even PND RAM */
	uint64_t pnd_sbe0_ie                  : 1;  /**< Single bit error interrupt enable for even PND RAM */
	uint64_t pnd_dbe1_ie                  : 1;  /**< Double bit error interrupt enable for odd PND RAM */
	uint64_t pnd_sbe1_ie                  : 1;  /**< Single bit error interrupt enable for odd PND RAM */
	uint64_t oth_dbe0_ie                  : 1;  /**< Double bit error interrupt enable for even OTH RAM */
	uint64_t oth_sbe0_ie                  : 1;  /**< Single bit error interrupt enable for even OTH RAM */
	uint64_t oth_dbe1_ie                  : 1;  /**< Double bit error interrupt enable for odd OTH RAM */
	uint64_t oth_sbe1_ie                  : 1;  /**< Single bit error interrupt enable for odd OTH RAM */
	uint64_t idx_dbe_ie                   : 1;  /**< Double bit error interrupt enable for IDX RAM */
	uint64_t idx_sbe_ie                   : 1;  /**< Single bit error interrupt enable for IDX RAM */
	uint64_t fidx_dbe_ie                  : 1;  /**< Double bit error interrupt enable for FIDX RAM */
	uint64_t fidx_sbe_ie                  : 1;  /**< Single bit error interrupt enable for FIDX RAM */
#else
	uint64_t fidx_sbe_ie                  : 1;
	uint64_t fidx_dbe_ie                  : 1;
	uint64_t idx_sbe_ie                   : 1;
	uint64_t idx_dbe_ie                   : 1;
	uint64_t oth_sbe1_ie                  : 1;
	uint64_t oth_dbe1_ie                  : 1;
	uint64_t oth_sbe0_ie                  : 1;
	uint64_t oth_dbe0_ie                  : 1;
	uint64_t pnd_sbe1_ie                  : 1;
	uint64_t pnd_dbe1_ie                  : 1;
	uint64_t pnd_sbe0_ie                  : 1;
	uint64_t pnd_dbe0_ie                  : 1;
	uint64_t reserved_12_31               : 20;
	uint64_t iop_ie                       : 11;
	uint64_t reserved_43_44               : 2;
	uint64_t fpe_ie                       : 1;
	uint64_t awe_ie                       : 1;
	uint64_t bfp_ie                       : 1;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_sso_err_enb_s             cn68xx;
	struct cvmx_sso_err_enb_s             cn68xxp1;
};
typedef union cvmx_sso_err_enb cvmx_sso_err_enb_t;

/**
 * cvmx_sso_fidx_ecc_ctl
 *
 * SSO_FIDX_ECC_CTL = SSO FIDX ECC Control
 *
 */
union cvmx_sso_fidx_ecc_ctl {
	uint64_t u64;
	struct cvmx_sso_fidx_ecc_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t flip_synd                    : 2;  /**< Testing feature. Flip Syndrom to generate single or
                                                         double bit error for the FIDX RAM. */
	uint64_t ecc_ena                      : 1;  /**< ECC Enable: When set will enable the 5 bit ECC
                                                         correct logic for the FIDX RAM. */
#else
	uint64_t ecc_ena                      : 1;
	uint64_t flip_synd                    : 2;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_sso_fidx_ecc_ctl_s        cn68xx;
	struct cvmx_sso_fidx_ecc_ctl_s        cn68xxp1;
};
typedef union cvmx_sso_fidx_ecc_ctl cvmx_sso_fidx_ecc_ctl_t;

/**
 * cvmx_sso_fidx_ecc_st
 *
 * SSO_FIDX_ECC_ST = SSO FIDX ECC Status
 *
 */
union cvmx_sso_fidx_ecc_st {
	uint64_t u64;
	struct cvmx_sso_fidx_ecc_st_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_27_63               : 37;
	uint64_t addr                         : 11; /**< Latch the address for latest sde/dbe occured
                                                         for the FIDX RAM */
	uint64_t reserved_9_15                : 7;
	uint64_t syndrom                      : 5;  /**< Report the latest error syndrom for the
                                                         FIDX RAM */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t syndrom                      : 5;
	uint64_t reserved_9_15                : 7;
	uint64_t addr                         : 11;
	uint64_t reserved_27_63               : 37;
#endif
	} s;
	struct cvmx_sso_fidx_ecc_st_s         cn68xx;
	struct cvmx_sso_fidx_ecc_st_s         cn68xxp1;
};
typedef union cvmx_sso_fidx_ecc_st cvmx_sso_fidx_ecc_st_t;

/**
 * cvmx_sso_fpage_cnt
 *
 * SSO_FPAGE_CNT = SSO Free Page Cnt
 *
 * This register keeps track of the number of free pages pointers available for use in external memory.
 */
union cvmx_sso_fpage_cnt {
	uint64_t u64;
	struct cvmx_sso_fpage_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t fpage_cnt                    : 32; /**< Free Page Cnt
                                                         HW updates this register. Writes to this register
                                                         are only for diagnostic purposes */
#else
	uint64_t fpage_cnt                    : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sso_fpage_cnt_s           cn68xx;
	struct cvmx_sso_fpage_cnt_s           cn68xxp1;
};
typedef union cvmx_sso_fpage_cnt cvmx_sso_fpage_cnt_t;

/**
 * cvmx_sso_grp#_aq_cnt
 */
union cvmx_sso_grpx_aq_cnt {
	uint64_t u64;
	struct cvmx_sso_grpx_aq_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_33_63               : 31;
	uint64_t aq_cnt                       : 33; /**< Number of total in-unit, transitional and external admission queue entries for each group */
#else
	uint64_t aq_cnt                       : 33;
	uint64_t reserved_33_63               : 31;
#endif
	} s;
	struct cvmx_sso_grpx_aq_cnt_s         cn73xx;
	struct cvmx_sso_grpx_aq_cnt_s         cn78xx;
	struct cvmx_sso_grpx_aq_cnt_s         cn78xxp1;
	struct cvmx_sso_grpx_aq_cnt_s         cnf75xx;
};
typedef union cvmx_sso_grpx_aq_cnt cvmx_sso_grpx_aq_cnt_t;

/**
 * cvmx_sso_grp#_aq_thr
 */
union cvmx_sso_grpx_aq_thr {
	uint64_t u64;
	struct cvmx_sso_grpx_aq_thr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_33_63               : 31;
	uint64_t aq_thr                       : 33; /**< Total admission queue entry threshold. Compared against SSO_GRP()_AQ_CNT for
                                                         triggering AQ interrupts. */
#else
	uint64_t aq_thr                       : 33;
	uint64_t reserved_33_63               : 31;
#endif
	} s;
	struct cvmx_sso_grpx_aq_thr_s         cn73xx;
	struct cvmx_sso_grpx_aq_thr_s         cn78xx;
	struct cvmx_sso_grpx_aq_thr_s         cn78xxp1;
	struct cvmx_sso_grpx_aq_thr_s         cnf75xx;
};
typedef union cvmx_sso_grpx_aq_thr cvmx_sso_grpx_aq_thr_t;

/**
 * cvmx_sso_grp#_ds_pc
 *
 * Counts the number of deschedule requests for each group. Counter rolls over through zero when
 * max value exceeded.
 */
union cvmx_sso_grpx_ds_pc {
	uint64_t u64;
	struct cvmx_sso_grpx_ds_pc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t cnt                          : 64; /**< Deschedule performance counter. Writes are for diagnostic use only, and defined only when
                                                         neither work nor GET_WORKs are present in the SSO. */
#else
	uint64_t cnt                          : 64;
#endif
	} s;
	struct cvmx_sso_grpx_ds_pc_s          cn73xx;
	struct cvmx_sso_grpx_ds_pc_s          cn78xx;
	struct cvmx_sso_grpx_ds_pc_s          cn78xxp1;
	struct cvmx_sso_grpx_ds_pc_s          cnf75xx;
};
typedef union cvmx_sso_grpx_ds_pc cvmx_sso_grpx_ds_pc_t;

/**
 * cvmx_sso_grp#_ext_pc
 *
 * Counts the number of cache lines of WAEs sent to L2/DDR. Counter rolls over through zero when
 * max value exceeded.
 */
union cvmx_sso_grpx_ext_pc {
	uint64_t u64;
	struct cvmx_sso_grpx_ext_pc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t cnt                          : 64; /**< External admission queue cache lines written. Each write corresponds to 13 WAEs. Writes
                                                         are for diagnostic use only, and defined only when neither work nor GET_WORKs are present
                                                         in the SSO. */
#else
	uint64_t cnt                          : 64;
#endif
	} s;
	struct cvmx_sso_grpx_ext_pc_s         cn73xx;
	struct cvmx_sso_grpx_ext_pc_s         cn78xx;
	struct cvmx_sso_grpx_ext_pc_s         cn78xxp1;
	struct cvmx_sso_grpx_ext_pc_s         cnf75xx;
};
typedef union cvmx_sso_grpx_ext_pc cvmx_sso_grpx_ext_pc_t;

/**
 * cvmx_sso_grp#_iaq_thr
 *
 * These registers contain the thresholds for allocating SSO in-unit admission queue entries, see
 * In-Unit Thresholds.
 */
union cvmx_sso_grpx_iaq_thr {
	uint64_t u64;
	struct cvmx_sso_grpx_iaq_thr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_61_63               : 3;
	uint64_t grp_cnt                      : 13; /**< Group's entry count. Number of internal entries allocated to IAQ, conflicted work, or CQ
                                                         in this group. */
	uint64_t reserved_45_47               : 3;
	uint64_t max_thr                      : 13; /**< Max threshold for this internal admission queue. If nonzero, must be >= [RSVD_THR] + 4.
                                                         To ensure full streaming performance to all cores, should be at least 208. Must not be
                                                         changed after traffic is sent to this group. */
	uint64_t reserved_13_31               : 19;
	uint64_t rsvd_thr                     : 13; /**< Threshold for reserved entries for this internal group queue. Should be at least
                                                         0x1 for any groups that must make forward progress when other group's work is
                                                         pending. Updates to this field must also update SSO_AW_ADD[RSVD_FREE]. Must not
                                                         be changed after traffic is sent to this group. */
#else
	uint64_t rsvd_thr                     : 13;
	uint64_t reserved_13_31               : 19;
	uint64_t max_thr                      : 13;
	uint64_t reserved_45_47               : 3;
	uint64_t grp_cnt                      : 13;
	uint64_t reserved_61_63               : 3;
#endif
	} s;
	struct cvmx_sso_grpx_iaq_thr_s        cn73xx;
	struct cvmx_sso_grpx_iaq_thr_s        cn78xx;
	struct cvmx_sso_grpx_iaq_thr_s        cn78xxp1;
	struct cvmx_sso_grpx_iaq_thr_s        cnf75xx;
};
typedef union cvmx_sso_grpx_iaq_thr cvmx_sso_grpx_iaq_thr_t;

/**
 * cvmx_sso_grp#_int
 *
 * Contains the per-group interrupts and are used to clear these interrupts. For more information
 * on this register, refer to Interrupts.
 */
union cvmx_sso_grpx_int {
	uint64_t u64;
	struct cvmx_sso_grpx_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t exe_dis                      : 1;  /**< Executable interrupt temporary disable. Corresponding [EXE_INT] bit cannot be set due to
                                                         IAQ_CNT/IAQ_THR check when this bit is set. EXE_DIS is cleared by hardware whenever:
                                                         * SSO_GRP()_INT_CNT[IAQ_CNT] is zero, or
                                                         * The hardware decrements the time counter for this group to zero, i.e.
                                                         SSO_GRP()_INT_CNT[TC_CNT] is equal to 1 when periodic counter SSO_WQ_INT_PC[PC] is
                                                         equal to 0. */
	uint64_t reserved_2_62                : 61;
	uint64_t exe_int                      : 1;  /**< Work-executable interrupt. Generally used to indicate work is waiting for software. Throws
                                                         SSO_INTSN_E::SSO_GRP()_EXE. Set by hardware whenever:
                                                         * SSO_GRP()_INT_CNT[IAQ_CNT] >= SSO_GRP()_INT_THR [IAQ_THR] and [IAQ_THR] != 0
                                                         and EXE_DIS is clear.
                                                         * SSO_GRP()_INT_CNT[DS_CNT] >= SSO_GRP()_INT_THR[DS_THR] and [DS_THR] != 0.
                                                         * SSO_GRP()_INT_CNT[CQ_CNT] >= SSO_GRP()_INT_THR[CQ_THR] and [CQ_THR] != 0.
                                                         * SSO_GRP()_INT_CNT[TC_CNT] is equal to 1 when periodic counter SSO_WQ_INT_PC[PC] is
                                                         equal to 0 and SSO_GRP()_INT_THR[TC_EN] is set and at least one of the following is
                                                         true:
                                                         _ SSO_GRP()_INT_CNT[IAQ_CNT] > 0
                                                         _ SSO_GRP()_INT_CNT[DS_CNT] > 0
                                                         _ SSO_GRP()_INT_CNT[CQ_CNT] > 0 */
	uint64_t aq_int                       : 1;  /**< External group queue threshold interrupt. Set if SSO_GRP()_AQ_CNT changes, and the
                                                         resulting value is equal to SSO_GRP()_AQ_THR. Throws
                                                         SSO_INTSN_E::SSO_GRP()_AQ. */
#else
	uint64_t aq_int                       : 1;
	uint64_t exe_int                      : 1;
	uint64_t reserved_2_62                : 61;
	uint64_t exe_dis                      : 1;
#endif
	} s;
	struct cvmx_sso_grpx_int_s            cn73xx;
	struct cvmx_sso_grpx_int_s            cn78xx;
	struct cvmx_sso_grpx_int_s            cn78xxp1;
	struct cvmx_sso_grpx_int_s            cnf75xx;
};
typedef union cvmx_sso_grpx_int cvmx_sso_grpx_int_t;

/**
 * cvmx_sso_grp#_int_cnt
 *
 * These registers contain a read-only copy of the counts used to trigger work-queue interrupts
 * (one per group). For more information on this register, refer to Interrupts.
 */
union cvmx_sso_grpx_int_cnt {
	uint64_t u64;
	struct cvmx_sso_grpx_int_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_61_63               : 3;
	uint64_t tc_cnt                       : 13; /**< Time counter current value for this group. Hardware sets this field to the value of
                                                         SSO_GRP()_INT_THR[TC_THR] whenever:
                                                         * Corresponding SSO_GRP()_INT_CNT[IAQ_CNT, DS_CNT and CQ_CNT] are all equal to 0x0.
                                                         * Corresponding SSO_GRP()_INT[EXE_INT] is written with a one to clear by software.
                                                         * Corresponding SSO_GRP()_INT[EXE_DIS] is written with a one to set by software.
                                                         * Corresponding SSO_GRP()_INT_THR is written by software.
                                                         * TC_CNT is equal to 1 and periodic counter SSO_WQ_INT_PC[PC] is equal to 0x0.
                                                         Otherwise, hardware decrements this field whenever the periodic counter SSO_WQ_INT_PC[PC]
                                                         is equal to 0x0. This field is 0x0 whenever SSO_GRP()_INT_THR[TC_THR] is equal to 0x0. */
	uint64_t reserved_45_47               : 3;
	uint64_t cq_cnt                       : 13; /**< Conflicted queue executable count for this group. */
	uint64_t reserved_29_31               : 3;
	uint64_t ds_cnt                       : 13; /**< Deschedule executable count for this group. */
	uint64_t reserved_13_15               : 3;
	uint64_t iaq_cnt                      : 13; /**< Work-queue entries for this in-unit admission queue. */
#else
	uint64_t iaq_cnt                      : 13;
	uint64_t reserved_13_15               : 3;
	uint64_t ds_cnt                       : 13;
	uint64_t reserved_29_31               : 3;
	uint64_t cq_cnt                       : 13;
	uint64_t reserved_45_47               : 3;
	uint64_t tc_cnt                       : 13;
	uint64_t reserved_61_63               : 3;
#endif
	} s;
	struct cvmx_sso_grpx_int_cnt_s        cn73xx;
	struct cvmx_sso_grpx_int_cnt_s        cn78xx;
	struct cvmx_sso_grpx_int_cnt_s        cn78xxp1;
	struct cvmx_sso_grpx_int_cnt_s        cnf75xx;
};
typedef union cvmx_sso_grpx_int_cnt cvmx_sso_grpx_int_cnt_t;

/**
 * cvmx_sso_grp#_int_thr
 *
 * These registers contain the thresholds for enabling and setting work-queue interrupts (one per
 * group). For more information on this register, refer to Interrupts.
 */
union cvmx_sso_grpx_int_thr {
	uint64_t u64;
	struct cvmx_sso_grpx_int_thr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t tc_en                        : 1;  /**< Time counter interrupt enable for this group. This field must be zero when [TC_THR] is 0x0. */
	uint64_t reserved_61_62               : 2;
	uint64_t tc_thr                       : 13; /**< Time counter interrupt threshold for this group. Compared against
                                                         SSO_GRP()_INT_CNT[TC_CNT]. When this field is equal to 0x0,
                                                         SSO_GRP()_INT_CNT[TC_CNT] is zero. */
	uint64_t reserved_45_47               : 3;
	uint64_t cq_thr                       : 13; /**< Conflicted queue count threshold for this group. Compared against
                                                         SSO_GRP()_INT_CNT[CQ_CNT]. When this field is 0x0, the threshold interrupt is
                                                         disabled. */
	uint64_t reserved_29_31               : 3;
	uint64_t ds_thr                       : 13; /**< Deschedule count threshold for this group. Compared against
                                                         SSO_GRP()_INT_CNT[DS_CNT]. When this field is 0x0, the threshold interrupt is
                                                         disabled. */
	uint64_t reserved_13_15               : 3;
	uint64_t iaq_thr                      : 13; /**< In-unit admission queue threshold for this group. Compared against
                                                         SSO_GRP()_INT_CNT[IAQ_CNT]. When this field is 0x0, the threshold interrupt is
                                                         disabled. */
#else
	uint64_t iaq_thr                      : 13;
	uint64_t reserved_13_15               : 3;
	uint64_t ds_thr                       : 13;
	uint64_t reserved_29_31               : 3;
	uint64_t cq_thr                       : 13;
	uint64_t reserved_45_47               : 3;
	uint64_t tc_thr                       : 13;
	uint64_t reserved_61_62               : 2;
	uint64_t tc_en                        : 1;
#endif
	} s;
	struct cvmx_sso_grpx_int_thr_s        cn73xx;
	struct cvmx_sso_grpx_int_thr_s        cn78xx;
	struct cvmx_sso_grpx_int_thr_s        cn78xxp1;
	struct cvmx_sso_grpx_int_thr_s        cnf75xx;
};
typedef union cvmx_sso_grpx_int_thr cvmx_sso_grpx_int_thr_t;

/**
 * cvmx_sso_grp#_pri
 *
 * Controls the priority and group affinity arbitration for each group.
 *
 */
union cvmx_sso_grpx_pri {
	uint64_t u64;
	struct cvmx_sso_grpx_pri_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_30_63               : 34;
	uint64_t wgt_left                     : 6;  /**< Cross-group arbitration credits remaining on this group. */
	uint64_t reserved_22_23               : 2;
	uint64_t weight                       : 6;  /**< Cross-group arbitration weight to apply to this group. Must be >= 0x2. */
	uint64_t reserved_12_15               : 4;
	uint64_t affinity                     : 4;  /**< Processor affinity arbitration weight to apply to this group. If zero, affinity
                                                         is disabled. A change to [AFFINITY] will not take effect until the old [AFFINITY]'s
                                                         value loaded into SSO_PP()_ARB[AFF_LEFT] has drained to zero. */
	uint64_t reserved_3_7                 : 5;
	uint64_t pri                          : 3;  /**< Priority for this group relative to other groups. To prevent a core from receiving work on
                                                         a group use SSO_PP()_S()_GRPMSK().
                                                         0x0 = highest priority.
                                                         0x7 = lowest priority.
                                                         Changing priority while GET_WORKs are in flight may result in a GET_WORK using either the
                                                         old or new priority, or a mix thereof. */
#else
	uint64_t pri                          : 3;
	uint64_t reserved_3_7                 : 5;
	uint64_t affinity                     : 4;
	uint64_t reserved_12_15               : 4;
	uint64_t weight                       : 6;
	uint64_t reserved_22_23               : 2;
	uint64_t wgt_left                     : 6;
	uint64_t reserved_30_63               : 34;
#endif
	} s;
	struct cvmx_sso_grpx_pri_s            cn73xx;
	struct cvmx_sso_grpx_pri_s            cn78xx;
	struct cvmx_sso_grpx_pri_s            cn78xxp1;
	struct cvmx_sso_grpx_pri_s            cnf75xx;
};
typedef union cvmx_sso_grpx_pri cvmx_sso_grpx_pri_t;

/**
 * cvmx_sso_grp#_taq_thr
 *
 * These registers contain the thresholds for allocating SSO transitory admission queue storage
 * buffers, see Transitory-Admission Thresholds.
 */
union cvmx_sso_grpx_taq_thr {
	uint64_t u64;
	struct cvmx_sso_grpx_taq_thr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_59_63               : 5;
	uint64_t grp_cnt                      : 11; /**< Group's entry count. Number of transitory admission buffers allocated to this group. */
	uint64_t reserved_43_47               : 5;
	uint64_t max_thr                      : 11; /**< Max threshold for this transitory admission queue, in buffers of 13 entries. Must be >= 3,
                                                         must be >= [RSVD_THR], and to ensure full streaming performance on this group, should be
                                                         at least 16 buffers. SSO may exceed this count using unreserved free buffers if and only
                                                         if persistently backpressured by IOBI. Must not be changed after traffic is sent to this
                                                         group. */
	uint64_t reserved_11_31               : 21;
	uint64_t rsvd_thr                     : 11; /**< Threshold for reserved entries for this transitory admission queue, in buffers
                                                         of 13 entries. Must be at least 3 buffers for any groups that are to be
                                                         used. Changes to this field must also update SSO_TAQ_ADD[RSVD_FREE]. Must not be
                                                         changed after traffic is sent to this group. */
#else
	uint64_t rsvd_thr                     : 11;
	uint64_t reserved_11_31               : 21;
	uint64_t max_thr                      : 11;
	uint64_t reserved_43_47               : 5;
	uint64_t grp_cnt                      : 11;
	uint64_t reserved_59_63               : 5;
#endif
	} s;
	struct cvmx_sso_grpx_taq_thr_s        cn73xx;
	struct cvmx_sso_grpx_taq_thr_s        cn78xx;
	struct cvmx_sso_grpx_taq_thr_s        cn78xxp1;
	struct cvmx_sso_grpx_taq_thr_s        cnf75xx;
};
typedef union cvmx_sso_grpx_taq_thr cvmx_sso_grpx_taq_thr_t;

/**
 * cvmx_sso_grp#_ts_pc
 *
 * Counts the number of tag switch requests for each group being switched to. Counter rolls over
 * through zero when max value exceeded.
 */
union cvmx_sso_grpx_ts_pc {
	uint64_t u64;
	struct cvmx_sso_grpx_ts_pc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t cnt                          : 64; /**< Tag switch performance counter. Writes are for diagnostic use only, and defined only when
                                                         neither work nor GET_WORKs are present in the SSO. */
#else
	uint64_t cnt                          : 64;
#endif
	} s;
	struct cvmx_sso_grpx_ts_pc_s          cn73xx;
	struct cvmx_sso_grpx_ts_pc_s          cn78xx;
	struct cvmx_sso_grpx_ts_pc_s          cn78xxp1;
	struct cvmx_sso_grpx_ts_pc_s          cnf75xx;
};
typedef union cvmx_sso_grpx_ts_pc cvmx_sso_grpx_ts_pc_t;

/**
 * cvmx_sso_grp#_wa_pc
 *
 * Counts the number of add new work requests for each group. The counter rolls over through zero
 * when the max value exceeded.
 */
union cvmx_sso_grpx_wa_pc {
	uint64_t u64;
	struct cvmx_sso_grpx_wa_pc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t cnt                          : 64; /**< Work add performance counter for group. Increments when work moves into IAQ. Writes are
                                                         for diagnostic use only, and defined only when neither work nor GET_WORKs are present in
                                                         the SSO. */
#else
	uint64_t cnt                          : 64;
#endif
	} s;
	struct cvmx_sso_grpx_wa_pc_s          cn73xx;
	struct cvmx_sso_grpx_wa_pc_s          cn78xx;
	struct cvmx_sso_grpx_wa_pc_s          cn78xxp1;
	struct cvmx_sso_grpx_wa_pc_s          cnf75xx;
};
typedef union cvmx_sso_grpx_wa_pc cvmx_sso_grpx_wa_pc_t;

/**
 * cvmx_sso_grp#_ws_pc
 *
 * Counts the number of work schedules for each group. The counter rolls over through zero when
 * the maximum value is exceeded.
 */
union cvmx_sso_grpx_ws_pc {
	uint64_t u64;
	struct cvmx_sso_grpx_ws_pc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t cnt                          : 64; /**< Work schedule performance counter for group. Writes are for diagnostic use only, and
                                                         defined only when neither work nor GET_WORKs are present in the SSO. */
#else
	uint64_t cnt                          : 64;
#endif
	} s;
	struct cvmx_sso_grpx_ws_pc_s          cn73xx;
	struct cvmx_sso_grpx_ws_pc_s          cn78xx;
	struct cvmx_sso_grpx_ws_pc_s          cn78xxp1;
	struct cvmx_sso_grpx_ws_pc_s          cnf75xx;
};
typedef union cvmx_sso_grpx_ws_pc cvmx_sso_grpx_ws_pc_t;

/**
 * cvmx_sso_gw_eco
 */
union cvmx_sso_gw_eco {
	uint64_t u64;
	struct cvmx_sso_gw_eco_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t eco_rw                       : 8;  /**< N/A */
#else
	uint64_t eco_rw                       : 8;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_sso_gw_eco_s              cn73xx;
	struct cvmx_sso_gw_eco_s              cnf75xx;
};
typedef union cvmx_sso_gw_eco cvmx_sso_gw_eco_t;

/**
 * cvmx_sso_gwe_cfg
 *
 * This register controls the operation of the get-work examiner (GWE).
 *
 */
union cvmx_sso_gwe_cfg {
	uint64_t u64;
	struct cvmx_sso_gwe_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t odu_ffpgw_dis                : 1;  /**< Disable flushing ODU on periodic restart of GWE */
	uint64_t gwe_rfpgw_dis                : 1;  /**< Disable periodic restart of GWE for pending get_work */
	uint64_t odu_prf_dis                  : 1;  /**< Disable ODU-initiated prefetches of WQEs into L2C
                                                         For diagnostic use only. */
	uint64_t reserved_0_8                 : 9;
#else
	uint64_t reserved_0_8                 : 9;
	uint64_t odu_prf_dis                  : 1;
	uint64_t gwe_rfpgw_dis                : 1;
	uint64_t odu_ffpgw_dis                : 1;
	uint64_t reserved_12_63               : 52;
#endif
	} s;
	struct cvmx_sso_gwe_cfg_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t odu_ffpgw_dis                : 1;  /**< Disable flushing ODU on periodic restart of GWE */
	uint64_t gwe_rfpgw_dis                : 1;  /**< Disable periodic restart of GWE for pending get_work */
	uint64_t odu_prf_dis                  : 1;  /**< Disable ODU-initiated prefetches of WQEs into L2C
                                                         For diagnostic use only. */
	uint64_t odu_bmp_dis                  : 1;  /**< Disable ODU bumps.
                                                         If SSO_PP_STRICT is true, could
                                                         prevent forward progress under some circumstances.
                                                         For diagnostic use only. */
	uint64_t reserved_5_7                 : 3;
	uint64_t gwe_hvy_dis                  : 1;  /**< Disable GWE automatic, proportional weight-increase
                                                         mechanism and use SSO_QOSX_RND values as-is.
                                                         For diagnostic use only. */
	uint64_t gwe_poe                      : 1;  /**< Pause GWE on extracts
                                                         For diagnostic use only. */
	uint64_t gwe_fpor                     : 1;  /**< Flush GWE pipeline when restarting GWE.
                                                         For diagnostic use only. */
	uint64_t gwe_rah                      : 1;  /**< Begin at head of input queues when restarting GWE.
                                                         For diagnostic use only. */
	uint64_t gwe_dis                      : 1;  /**< Disable Get-Work Examiner */
#else
	uint64_t gwe_dis                      : 1;
	uint64_t gwe_rah                      : 1;
	uint64_t gwe_fpor                     : 1;
	uint64_t gwe_poe                      : 1;
	uint64_t gwe_hvy_dis                  : 1;
	uint64_t reserved_5_7                 : 3;
	uint64_t odu_bmp_dis                  : 1;
	uint64_t odu_prf_dis                  : 1;
	uint64_t gwe_rfpgw_dis                : 1;
	uint64_t odu_ffpgw_dis                : 1;
	uint64_t reserved_12_63               : 52;
#endif
	} cn68xx;
	struct cvmx_sso_gwe_cfg_cn68xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t gwe_poe                      : 1;  /**< Pause GWE on extracts
                                                         For diagnostic use only. */
	uint64_t gwe_fpor                     : 1;  /**< Flush GWE pipeline when restarting GWE.
                                                         For diagnostic use only. */
	uint64_t gwe_rah                      : 1;  /**< Begin at head of input queues when restarting GWE.
                                                         For diagnostic use only. */
	uint64_t gwe_dis                      : 1;  /**< Disable Get-Work Examiner */
#else
	uint64_t gwe_dis                      : 1;
	uint64_t gwe_rah                      : 1;
	uint64_t gwe_fpor                     : 1;
	uint64_t gwe_poe                      : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} cn68xxp1;
	struct cvmx_sso_gwe_cfg_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t dis_wgt_credit               : 1;  /**< Disable group weight credits. When set, groups have infinite weight credit. */
	uint64_t ws_retries                   : 8;  /**< Work slot retries. When a given work-slot performs this number of retries without
                                                         successfully finding work then NO_WORK will be returned. Values 0, 1, 2, 3 are reserved. */
#else
	uint64_t ws_retries                   : 8;
	uint64_t dis_wgt_credit               : 1;
	uint64_t reserved_9_63                : 55;
#endif
	} cn73xx;
	struct cvmx_sso_gwe_cfg_cn73xx        cn78xx;
	struct cvmx_sso_gwe_cfg_cn73xx        cn78xxp1;
	struct cvmx_sso_gwe_cfg_cn73xx        cnf75xx;
};
typedef union cvmx_sso_gwe_cfg cvmx_sso_gwe_cfg_t;

/**
 * cvmx_sso_gwe_random
 *
 * This register contains the random search start position for the get-work examiner (GWE).
 *
 */
union cvmx_sso_gwe_random {
	uint64_t u64;
	struct cvmx_sso_gwe_random_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t rnd                          : 16; /**< Current random value, with low 8 bits indicating first group to start next get-work search
                                                         at. Changes on each work search, even if unsuccessful or retried. For diagnostic use only,
                                                         must not be zero. */
#else
	uint64_t rnd                          : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_sso_gwe_random_s          cn73xx;
	struct cvmx_sso_gwe_random_s          cn78xx;
	struct cvmx_sso_gwe_random_s          cn78xxp1;
	struct cvmx_sso_gwe_random_s          cnf75xx;
};
typedef union cvmx_sso_gwe_random cvmx_sso_gwe_random_t;

/**
 * cvmx_sso_idx_ecc_ctl
 *
 * SSO_IDX_ECC_CTL = SSO IDX ECC Control
 *
 */
union cvmx_sso_idx_ecc_ctl {
	uint64_t u64;
	struct cvmx_sso_idx_ecc_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t flip_synd                    : 2;  /**< Testing feature. Flip Syndrom to generate single or
                                                         double bit error for the IDX RAM. */
	uint64_t ecc_ena                      : 1;  /**< ECC Enable: When set will enable the 5 bit ECC
                                                         correct logic for the IDX RAM. */
#else
	uint64_t ecc_ena                      : 1;
	uint64_t flip_synd                    : 2;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_sso_idx_ecc_ctl_s         cn68xx;
	struct cvmx_sso_idx_ecc_ctl_s         cn68xxp1;
};
typedef union cvmx_sso_idx_ecc_ctl cvmx_sso_idx_ecc_ctl_t;

/**
 * cvmx_sso_idx_ecc_st
 *
 * SSO_IDX_ECC_ST = SSO IDX ECC Status
 *
 */
union cvmx_sso_idx_ecc_st {
	uint64_t u64;
	struct cvmx_sso_idx_ecc_st_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_27_63               : 37;
	uint64_t addr                         : 11; /**< Latch the address for latest sde/dbe occured
                                                         for the IDX RAM */
	uint64_t reserved_9_15                : 7;
	uint64_t syndrom                      : 5;  /**< Report the latest error syndrom for the
                                                         IDX RAM */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t syndrom                      : 5;
	uint64_t reserved_9_15                : 7;
	uint64_t addr                         : 11;
	uint64_t reserved_27_63               : 37;
#endif
	} s;
	struct cvmx_sso_idx_ecc_st_s          cn68xx;
	struct cvmx_sso_idx_ecc_st_s          cn68xxp1;
};
typedef union cvmx_sso_idx_ecc_st cvmx_sso_idx_ecc_st_t;

/**
 * cvmx_sso_ient#_links
 *
 * Returns unit memory status for an index.
 *
 */
union cvmx_sso_ientx_links {
	uint64_t u64;
	struct cvmx_sso_ientx_links_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_28_63               : 36;
	uint64_t prev_index                   : 12; /**< The previous entry in the tag chain. Unpredictable if the entry is at the head of the list
                                                         or the head of a conflicted tag chain. */
	uint64_t reserved_0_15                : 16;
#else
	uint64_t reserved_0_15                : 16;
	uint64_t prev_index                   : 12;
	uint64_t reserved_28_63               : 36;
#endif
	} s;
	struct cvmx_sso_ientx_links_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_26_63               : 38;
	uint64_t prev_index                   : 10; /**< The previous entry in the tag chain. Unpredictable if the entry is at the head of the list
                                                         or the head of a conflicted tag chain. */
	uint64_t reserved_11_15               : 5;
	uint64_t next_index_vld               : 1;  /**< The [NEXT_INDEX] is valid. Unpredictable unless the entry is the tail entry of an atomic tag chain. */
	uint64_t next_index                   : 10; /**< The next entry in the tag chain or conflicted tag chain. Unpredictable if the entry is at
                                                         the tail of the list. */
#else
	uint64_t next_index                   : 10;
	uint64_t next_index_vld               : 1;
	uint64_t reserved_11_15               : 5;
	uint64_t prev_index                   : 10;
	uint64_t reserved_26_63               : 38;
#endif
	} cn73xx;
	struct cvmx_sso_ientx_links_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_28_63               : 36;
	uint64_t prev_index                   : 12; /**< The previous entry in the tag chain. Unpredictable if the entry is at the head of the list
                                                         or the head of a conflicted tag chain. */
	uint64_t reserved_13_15               : 3;
	uint64_t next_index_vld               : 1;  /**< The [NEXT_INDEX] is valid. Unpredictable unless the entry is the tail entry of an atomic tag chain. */
	uint64_t next_index                   : 12; /**< The next entry in the tag chain or conflicted tag chain. Unpredictable if the entry is at
                                                         the tail of the list. */
#else
	uint64_t next_index                   : 12;
	uint64_t next_index_vld               : 1;
	uint64_t reserved_13_15               : 3;
	uint64_t prev_index                   : 12;
	uint64_t reserved_28_63               : 36;
#endif
	} cn78xx;
	struct cvmx_sso_ientx_links_cn78xx    cn78xxp1;
	struct cvmx_sso_ientx_links_cn73xx    cnf75xx;
};
typedef union cvmx_sso_ientx_links cvmx_sso_ientx_links_t;

/**
 * cvmx_sso_ient#_pendtag
 *
 * Returns unit memory status for an index.
 *
 */
union cvmx_sso_ientx_pendtag {
	uint64_t u64;
	struct cvmx_sso_ientx_pendtag_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_38_63               : 26;
	uint64_t pend_switch                  : 1;  /**< Set when there is a pending non-UNTAGGED SWTAG or SWTAG_FULL and the SSO entry has not
                                                         left the list for the original tag. */
	uint64_t reserved_34_36               : 3;
	uint64_t pend_tt                      : 2;  /**< The next tag type for the new tag list when PEND_SWITCH is set. Enumerated by SSO_TT_E. */
	uint64_t pend_tag                     : 32; /**< The next tag for the new tag list when PEND_SWITCH is set. */
#else
	uint64_t pend_tag                     : 32;
	uint64_t pend_tt                      : 2;
	uint64_t reserved_34_36               : 3;
	uint64_t pend_switch                  : 1;
	uint64_t reserved_38_63               : 26;
#endif
	} s;
	struct cvmx_sso_ientx_pendtag_s       cn73xx;
	struct cvmx_sso_ientx_pendtag_s       cn78xx;
	struct cvmx_sso_ientx_pendtag_s       cn78xxp1;
	struct cvmx_sso_ientx_pendtag_s       cnf75xx;
};
typedef union cvmx_sso_ientx_pendtag cvmx_sso_ientx_pendtag_t;

/**
 * cvmx_sso_ient#_qlinks
 *
 * Returns unit memory status for an index.
 *
 */
union cvmx_sso_ientx_qlinks {
	uint64_t u64;
	struct cvmx_sso_ientx_qlinks_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t next_index                   : 12; /**< The next entry in the AQ/CQ/DQ. */
#else
	uint64_t next_index                   : 12;
	uint64_t reserved_12_63               : 52;
#endif
	} s;
	struct cvmx_sso_ientx_qlinks_s        cn73xx;
	struct cvmx_sso_ientx_qlinks_s        cn78xx;
	struct cvmx_sso_ientx_qlinks_s        cn78xxp1;
	struct cvmx_sso_ientx_qlinks_s        cnf75xx;
};
typedef union cvmx_sso_ientx_qlinks cvmx_sso_ientx_qlinks_t;

/**
 * cvmx_sso_ient#_tag
 *
 * Returns unit memory status for an index.
 *
 */
union cvmx_sso_ientx_tag {
	uint64_t u64;
	struct cvmx_sso_ientx_tag_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_39_63               : 25;
	uint64_t tailc                        : 1;  /**< The SSO entry is the tail of tag chain that is conflicted. No conflicted chain exists if
                                                         [TAIL] is also set on the same entry. */
	uint64_t tail                         : 1;  /**< The SSO entry is the tail of tag chain that is descheduled. */
	uint64_t reserved_34_36               : 3;
	uint64_t tt                           : 2;  /**< The tag type of the SSO entry. Enumerated by SSO_TT_E. */
	uint64_t tag                          : 32; /**< The tag of the SSO entry. */
#else
	uint64_t tag                          : 32;
	uint64_t tt                           : 2;
	uint64_t reserved_34_36               : 3;
	uint64_t tail                         : 1;
	uint64_t tailc                        : 1;
	uint64_t reserved_39_63               : 25;
#endif
	} s;
	struct cvmx_sso_ientx_tag_s           cn73xx;
	struct cvmx_sso_ientx_tag_s           cn78xx;
	struct cvmx_sso_ientx_tag_s           cn78xxp1;
	struct cvmx_sso_ientx_tag_s           cnf75xx;
};
typedef union cvmx_sso_ientx_tag cvmx_sso_ientx_tag_t;

/**
 * cvmx_sso_ient#_wqpgrp
 *
 * Returns unit memory status for an index.
 *
 */
union cvmx_sso_ientx_wqpgrp {
	uint64_t u64;
	struct cvmx_sso_ientx_wqpgrp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_62_63               : 2;
	uint64_t head                         : 1;  /**< SSO entry is at the head of a tag chain that is descheduled. */
	uint64_t nosched                      : 1;  /**< The nosched bit for the SSO entry. */
	uint64_t reserved_58_59               : 2;
	uint64_t grp                          : 10; /**< Group of the SSO entry. */
	uint64_t reserved_42_47               : 6;
	uint64_t wqp                          : 42; /**< Work queue pointer held in the SSO entry. */
#else
	uint64_t wqp                          : 42;
	uint64_t reserved_42_47               : 6;
	uint64_t grp                          : 10;
	uint64_t reserved_58_59               : 2;
	uint64_t nosched                      : 1;
	uint64_t head                         : 1;
	uint64_t reserved_62_63               : 2;
#endif
	} s;
	struct cvmx_sso_ientx_wqpgrp_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_62_63               : 2;
	uint64_t head                         : 1;  /**< SSO entry is at the head of a tag chain that is descheduled. */
	uint64_t nosched                      : 1;  /**< The nosched bit for the SSO entry. */
	uint64_t reserved_56_59               : 4;
	uint64_t grp                          : 8;  /**< Group of the SSO entry. */
	uint64_t reserved_42_47               : 6;
	uint64_t wqp                          : 42; /**< Work queue pointer held in the SSO entry. */
#else
	uint64_t wqp                          : 42;
	uint64_t reserved_42_47               : 6;
	uint64_t grp                          : 8;
	uint64_t reserved_56_59               : 4;
	uint64_t nosched                      : 1;
	uint64_t head                         : 1;
	uint64_t reserved_62_63               : 2;
#endif
	} cn73xx;
	struct cvmx_sso_ientx_wqpgrp_s        cn78xx;
	struct cvmx_sso_ientx_wqpgrp_s        cn78xxp1;
	struct cvmx_sso_ientx_wqpgrp_cn73xx   cnf75xx;
};
typedef union cvmx_sso_ientx_wqpgrp cvmx_sso_ientx_wqpgrp_t;

/**
 * cvmx_sso_ipl_conf#
 *
 * Returns list status for the conflicted list indexed by group.  Register
 * fields are identical to those in SSO_IPL_IAQ() above.
 */
union cvmx_sso_ipl_confx {
	uint64_t u64;
	struct cvmx_sso_ipl_confx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_28_63               : 36;
	uint64_t queue_val                    : 1;  /**< One or more valid entries are in the queue. */
	uint64_t queue_one                    : 1;  /**< Exactly one valid entry is in the queue. */
	uint64_t reserved_25_25               : 1;
	uint64_t queue_head                   : 12; /**< Index of entry at the head of the queue. */
	uint64_t reserved_12_12               : 1;
	uint64_t queue_tail                   : 12; /**< Index of entry at the tail of the queue. */
#else
	uint64_t queue_tail                   : 12;
	uint64_t reserved_12_12               : 1;
	uint64_t queue_head                   : 12;
	uint64_t reserved_25_25               : 1;
	uint64_t queue_one                    : 1;
	uint64_t queue_val                    : 1;
	uint64_t reserved_28_63               : 36;
#endif
	} s;
	struct cvmx_sso_ipl_confx_s           cn73xx;
	struct cvmx_sso_ipl_confx_s           cn78xx;
	struct cvmx_sso_ipl_confx_s           cn78xxp1;
	struct cvmx_sso_ipl_confx_s           cnf75xx;
};
typedef union cvmx_sso_ipl_confx cvmx_sso_ipl_confx_t;

/**
 * cvmx_sso_ipl_desched#
 *
 * Returns list status for the deschedule list indexed by group.  Register
 * fields are identical to those in SSO_IPL_IAQ() above.
 */
union cvmx_sso_ipl_deschedx {
	uint64_t u64;
	struct cvmx_sso_ipl_deschedx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_28_63               : 36;
	uint64_t queue_val                    : 1;  /**< One or more valid entries are in the queue. */
	uint64_t queue_one                    : 1;  /**< Exactly one valid entry is in the queue. */
	uint64_t reserved_25_25               : 1;
	uint64_t queue_head                   : 12; /**< Index of entry at the head of the queue. */
	uint64_t reserved_12_12               : 1;
	uint64_t queue_tail                   : 12; /**< Index of entry at the tail of the queue. */
#else
	uint64_t queue_tail                   : 12;
	uint64_t reserved_12_12               : 1;
	uint64_t queue_head                   : 12;
	uint64_t reserved_25_25               : 1;
	uint64_t queue_one                    : 1;
	uint64_t queue_val                    : 1;
	uint64_t reserved_28_63               : 36;
#endif
	} s;
	struct cvmx_sso_ipl_deschedx_s        cn73xx;
	struct cvmx_sso_ipl_deschedx_s        cn78xx;
	struct cvmx_sso_ipl_deschedx_s        cn78xxp1;
	struct cvmx_sso_ipl_deschedx_s        cnf75xx;
};
typedef union cvmx_sso_ipl_deschedx cvmx_sso_ipl_deschedx_t;

/**
 * cvmx_sso_ipl_free#
 *
 * Returns list status.
 *
 */
union cvmx_sso_ipl_freex {
	uint64_t u64;
	struct cvmx_sso_ipl_freex_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_62_63               : 2;
	uint64_t qnum_head                    : 3;  /**< Subqueue with current head. */
	uint64_t qnum_tail                    : 3;  /**< Subqueue for next tail. */
	uint64_t reserved_28_55               : 28;
	uint64_t queue_val                    : 1;  /**< One or more valid entries are in this subqueue. */
	uint64_t reserved_25_26               : 2;
	uint64_t queue_head                   : 12; /**< Index of entry at the head of this subqueue. */
	uint64_t reserved_12_12               : 1;
	uint64_t queue_tail                   : 12; /**< Index of entry at the tail of this subqueue. */
#else
	uint64_t queue_tail                   : 12;
	uint64_t reserved_12_12               : 1;
	uint64_t queue_head                   : 12;
	uint64_t reserved_25_26               : 2;
	uint64_t queue_val                    : 1;
	uint64_t reserved_28_55               : 28;
	uint64_t qnum_tail                    : 3;
	uint64_t qnum_head                    : 3;
	uint64_t reserved_62_63               : 2;
#endif
	} s;
	struct cvmx_sso_ipl_freex_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_62_63               : 2;
	uint64_t qnum_head                    : 3;  /**< Subqueue with current head. */
	uint64_t qnum_tail                    : 3;  /**< Subqueue for next tail. */
	uint64_t reserved_28_55               : 28;
	uint64_t queue_val                    : 1;  /**< One or more valid entries are in this subqueue. */
	uint64_t reserved_23_26               : 4;
	uint64_t queue_head                   : 10; /**< Index of entry at the head of this subqueue. */
	uint64_t reserved_10_12               : 3;
	uint64_t queue_tail                   : 10; /**< Index of entry at the tail of this subqueue. */
#else
	uint64_t queue_tail                   : 10;
	uint64_t reserved_10_12               : 3;
	uint64_t queue_head                   : 10;
	uint64_t reserved_23_26               : 4;
	uint64_t queue_val                    : 1;
	uint64_t reserved_28_55               : 28;
	uint64_t qnum_tail                    : 3;
	uint64_t qnum_head                    : 3;
	uint64_t reserved_62_63               : 2;
#endif
	} cn73xx;
	struct cvmx_sso_ipl_freex_s           cn78xx;
	struct cvmx_sso_ipl_freex_s           cn78xxp1;
	struct cvmx_sso_ipl_freex_cn73xx      cnf75xx;
};
typedef union cvmx_sso_ipl_freex cvmx_sso_ipl_freex_t;

/**
 * cvmx_sso_ipl_iaq#
 *
 * Returns list status for the internal admission queue indexed by group.
 *
 */
union cvmx_sso_ipl_iaqx {
	uint64_t u64;
	struct cvmx_sso_ipl_iaqx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_28_63               : 36;
	uint64_t queue_val                    : 1;  /**< One or more valid entries are in the queue. */
	uint64_t queue_one                    : 1;  /**< Exactly one valid entry is in the queue. */
	uint64_t reserved_25_25               : 1;
	uint64_t queue_head                   : 12; /**< Index of entry at the head of the queue. */
	uint64_t reserved_12_12               : 1;
	uint64_t queue_tail                   : 12; /**< Index of entry at the tail of the queue. */
#else
	uint64_t queue_tail                   : 12;
	uint64_t reserved_12_12               : 1;
	uint64_t queue_head                   : 12;
	uint64_t reserved_25_25               : 1;
	uint64_t queue_one                    : 1;
	uint64_t queue_val                    : 1;
	uint64_t reserved_28_63               : 36;
#endif
	} s;
	struct cvmx_sso_ipl_iaqx_s            cn73xx;
	struct cvmx_sso_ipl_iaqx_s            cn78xx;
	struct cvmx_sso_ipl_iaqx_s            cn78xxp1;
	struct cvmx_sso_ipl_iaqx_s            cnf75xx;
};
typedef union cvmx_sso_ipl_iaqx cvmx_sso_ipl_iaqx_t;

/**
 * cvmx_sso_iq_cnt#
 *
 * CSR reserved addresses: (64): 0x8200..0x83f8
 * CSR align addresses: ===========================================================================================================
 * SSO_IQ_CNTX = SSO Input Queue Count Register
 *               (one per QOS level)
 *
 * Contains a read-only count of the number of work queue entries for each QOS
 * level. Counts both in-unit and in-memory entries.
 */
union cvmx_sso_iq_cntx {
	uint64_t u64;
	struct cvmx_sso_iq_cntx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t iq_cnt                       : 32; /**< Input queue count for QOS level X */
#else
	uint64_t iq_cnt                       : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sso_iq_cntx_s             cn68xx;
	struct cvmx_sso_iq_cntx_s             cn68xxp1;
};
typedef union cvmx_sso_iq_cntx cvmx_sso_iq_cntx_t;

/**
 * cvmx_sso_iq_com_cnt
 *
 * SSO_IQ_COM_CNT = SSO Input Queue Combined Count Register
 *
 * Contains a read-only count of the total number of work queue entries in all
 * QOS levels.  Counts both in-unit and in-memory entries.
 */
union cvmx_sso_iq_com_cnt {
	uint64_t u64;
	struct cvmx_sso_iq_com_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t iq_cnt                       : 32; /**< Input queue combined count */
#else
	uint64_t iq_cnt                       : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sso_iq_com_cnt_s          cn68xx;
	struct cvmx_sso_iq_com_cnt_s          cn68xxp1;
};
typedef union cvmx_sso_iq_com_cnt cvmx_sso_iq_com_cnt_t;

/**
 * cvmx_sso_iq_int
 *
 * SSO_IQ_INT = SSO Input Queue Interrupt Register
 *
 * Contains the bits (one per QOS level) that can trigger the input queue
 * interrupt.  An IQ_INT bit will be set if SSO_IQ_CNT#QOS# changes and the
 * resulting value is equal to SSO_IQ_THR#QOS#.
 */
union cvmx_sso_iq_int {
	uint64_t u64;
	struct cvmx_sso_iq_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t iq_int                       : 8;  /**< Input queue interrupt bits */
#else
	uint64_t iq_int                       : 8;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_sso_iq_int_s              cn68xx;
	struct cvmx_sso_iq_int_s              cn68xxp1;
};
typedef union cvmx_sso_iq_int cvmx_sso_iq_int_t;

/**
 * cvmx_sso_iq_int_en
 *
 * SSO_IQ_INT_EN = SSO Input Queue Interrupt Enable Register
 *
 * Contains the bits (one per QOS level) that enable the input queue interrupt.
 */
union cvmx_sso_iq_int_en {
	uint64_t u64;
	struct cvmx_sso_iq_int_en_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t int_en                       : 8;  /**< Input queue interrupt enable bits */
#else
	uint64_t int_en                       : 8;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_sso_iq_int_en_s           cn68xx;
	struct cvmx_sso_iq_int_en_s           cn68xxp1;
};
typedef union cvmx_sso_iq_int_en cvmx_sso_iq_int_en_t;

/**
 * cvmx_sso_iq_thr#
 *
 * CSR reserved addresses: (24): 0x9040..0x90f8
 * CSR align addresses: ===========================================================================================================
 * SSO_IQ_THRX = SSO Input Queue Threshold Register
 *               (one per QOS level)
 *
 * Threshold value for triggering input queue interrupts.
 */
union cvmx_sso_iq_thrx {
	uint64_t u64;
	struct cvmx_sso_iq_thrx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t iq_thr                       : 32; /**< Input queue threshold for QOS level X */
#else
	uint64_t iq_thr                       : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sso_iq_thrx_s             cn68xx;
	struct cvmx_sso_iq_thrx_s             cn68xxp1;
};
typedef union cvmx_sso_iq_thrx cvmx_sso_iq_thrx_t;

/**
 * cvmx_sso_nos_cnt
 *
 * Contains the number of work-queue entries on the no-schedule list.
 *
 */
union cvmx_sso_nos_cnt {
	uint64_t u64;
	struct cvmx_sso_nos_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_13_63               : 51;
	uint64_t nos_cnt                      : 13; /**< Number of work-queue entries on the no-schedule list. */
#else
	uint64_t nos_cnt                      : 13;
	uint64_t reserved_13_63               : 51;
#endif
	} s;
	struct cvmx_sso_nos_cnt_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t nos_cnt                      : 12; /**< Number of work queue entries on the no-schedule list */
#else
	uint64_t nos_cnt                      : 12;
	uint64_t reserved_12_63               : 52;
#endif
	} cn68xx;
	struct cvmx_sso_nos_cnt_cn68xx        cn68xxp1;
	struct cvmx_sso_nos_cnt_s             cn73xx;
	struct cvmx_sso_nos_cnt_s             cn78xx;
	struct cvmx_sso_nos_cnt_s             cn78xxp1;
	struct cvmx_sso_nos_cnt_s             cnf75xx;
};
typedef union cvmx_sso_nos_cnt cvmx_sso_nos_cnt_t;

/**
 * cvmx_sso_nw_tim
 *
 * Sets the minimum period for a new-work-request timeout. The period is specified in n-1
 * notation, with the increment value of 1024 clock cycles. Thus, a value of 0x0 in this register
 * translates to 1024 cycles, 0x1 translates to 2048 cycles, 0x2 translates to 3072 cycles, etc.
 */
union cvmx_sso_nw_tim {
	uint64_t u64;
	struct cvmx_sso_nw_tim_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t nw_tim                       : 10; /**< New-work-timer period.
                                                         0x0 = 1024 clock cycles.
                                                         0x1 = 2048 clock cycles.
                                                         0x2 = 3072 clock cycles.
                                                         _ ... etc. */
#else
	uint64_t nw_tim                       : 10;
	uint64_t reserved_10_63               : 54;
#endif
	} s;
	struct cvmx_sso_nw_tim_s              cn68xx;
	struct cvmx_sso_nw_tim_s              cn68xxp1;
	struct cvmx_sso_nw_tim_s              cn73xx;
	struct cvmx_sso_nw_tim_s              cn78xx;
	struct cvmx_sso_nw_tim_s              cn78xxp1;
	struct cvmx_sso_nw_tim_s              cnf75xx;
};
typedef union cvmx_sso_nw_tim cvmx_sso_nw_tim_t;

/**
 * cvmx_sso_oth_ecc_ctl
 *
 * SSO_OTH_ECC_CTL = SSO OTH ECC Control
 *
 */
union cvmx_sso_oth_ecc_ctl {
	uint64_t u64;
	struct cvmx_sso_oth_ecc_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t flip_synd1                   : 2;  /**< Testing feature. Flip Syndrom to generate single or
                                                         double bit error for the odd OTH RAM. */
	uint64_t ecc_ena1                     : 1;  /**< ECC Enable: When set will enable the 7 bit ECC
                                                         correct logic for the odd OTH RAM. */
	uint64_t flip_synd0                   : 2;  /**< Testing feature. Flip Syndrom to generate single or
                                                         double bit error for the even OTH RAM. */
	uint64_t ecc_ena0                     : 1;  /**< ECC Enable: When set will enable the 7 bit ECC
                                                         correct logic for the even OTH RAM. */
#else
	uint64_t ecc_ena0                     : 1;
	uint64_t flip_synd0                   : 2;
	uint64_t ecc_ena1                     : 1;
	uint64_t flip_synd1                   : 2;
	uint64_t reserved_6_63                : 58;
#endif
	} s;
	struct cvmx_sso_oth_ecc_ctl_s         cn68xx;
	struct cvmx_sso_oth_ecc_ctl_s         cn68xxp1;
};
typedef union cvmx_sso_oth_ecc_ctl cvmx_sso_oth_ecc_ctl_t;

/**
 * cvmx_sso_oth_ecc_st
 *
 * SSO_OTH_ECC_ST = SSO OTH ECC Status
 *
 */
union cvmx_sso_oth_ecc_st {
	uint64_t u64;
	struct cvmx_sso_oth_ecc_st_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_59_63               : 5;
	uint64_t addr1                        : 11; /**< Latch the address for latest sde/dbe occured
                                                         for the odd OTH RAM */
	uint64_t reserved_43_47               : 5;
	uint64_t syndrom1                     : 7;  /**< Report the latest error syndrom for the odd
                                                         OTH RAM */
	uint64_t reserved_27_35               : 9;
	uint64_t addr0                        : 11; /**< Latch the address for latest sde/dbe occured
                                                         for the even OTH RAM */
	uint64_t reserved_11_15               : 5;
	uint64_t syndrom0                     : 7;  /**< Report the latest error syndrom for the even
                                                         OTH RAM */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t syndrom0                     : 7;
	uint64_t reserved_11_15               : 5;
	uint64_t addr0                        : 11;
	uint64_t reserved_27_35               : 9;
	uint64_t syndrom1                     : 7;
	uint64_t reserved_43_47               : 5;
	uint64_t addr1                        : 11;
	uint64_t reserved_59_63               : 5;
#endif
	} s;
	struct cvmx_sso_oth_ecc_st_s          cn68xx;
	struct cvmx_sso_oth_ecc_st_s          cn68xxp1;
};
typedef union cvmx_sso_oth_ecc_st cvmx_sso_oth_ecc_st_t;

/**
 * cvmx_sso_page_cnt
 */
union cvmx_sso_page_cnt {
	uint64_t u64;
	struct cvmx_sso_page_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t cnt                          : 32; /**< In-use page count. Number of pages SSO has allocated and not yet returned. Excludes unused
                                                         pointers cached in SSO. Hardware updates this register. */
#else
	uint64_t cnt                          : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sso_page_cnt_s            cn73xx;
	struct cvmx_sso_page_cnt_s            cn78xx;
	struct cvmx_sso_page_cnt_s            cn78xxp1;
	struct cvmx_sso_page_cnt_s            cnf75xx;
};
typedef union cvmx_sso_page_cnt cvmx_sso_page_cnt_t;

/**
 * cvmx_sso_pnd_ecc_ctl
 *
 * SSO_PND_ECC_CTL = SSO PND ECC Control
 *
 */
union cvmx_sso_pnd_ecc_ctl {
	uint64_t u64;
	struct cvmx_sso_pnd_ecc_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t flip_synd1                   : 2;  /**< Testing feature. Flip Syndrom to generate single or
                                                         double bit error for the odd PND RAM. */
	uint64_t ecc_ena1                     : 1;  /**< ECC Enable: When set will enable the 7 bit ECC
                                                         correct logic for the odd PND RAM. */
	uint64_t flip_synd0                   : 2;  /**< Testing feature. Flip Syndrom to generate single or
                                                         double bit error for the even PND RAM. */
	uint64_t ecc_ena0                     : 1;  /**< ECC Enable: When set will enable the 7 bit ECC
                                                         correct logic for the even PND RAM. */
#else
	uint64_t ecc_ena0                     : 1;
	uint64_t flip_synd0                   : 2;
	uint64_t ecc_ena1                     : 1;
	uint64_t flip_synd1                   : 2;
	uint64_t reserved_6_63                : 58;
#endif
	} s;
	struct cvmx_sso_pnd_ecc_ctl_s         cn68xx;
	struct cvmx_sso_pnd_ecc_ctl_s         cn68xxp1;
};
typedef union cvmx_sso_pnd_ecc_ctl cvmx_sso_pnd_ecc_ctl_t;

/**
 * cvmx_sso_pnd_ecc_st
 *
 * SSO_PND_ECC_ST = SSO PND ECC Status
 *
 */
union cvmx_sso_pnd_ecc_st {
	uint64_t u64;
	struct cvmx_sso_pnd_ecc_st_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_59_63               : 5;
	uint64_t addr1                        : 11; /**< Latch the address for latest sde/dbe occured
                                                         for the odd PND RAM */
	uint64_t reserved_43_47               : 5;
	uint64_t syndrom1                     : 7;  /**< Report the latest error syndrom for the odd
                                                         PND RAM */
	uint64_t reserved_27_35               : 9;
	uint64_t addr0                        : 11; /**< Latch the address for latest sde/dbe occured
                                                         for the even PND RAM */
	uint64_t reserved_11_15               : 5;
	uint64_t syndrom0                     : 7;  /**< Report the latest error syndrom for the even
                                                         PND RAM */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t syndrom0                     : 7;
	uint64_t reserved_11_15               : 5;
	uint64_t addr0                        : 11;
	uint64_t reserved_27_35               : 9;
	uint64_t syndrom1                     : 7;
	uint64_t reserved_43_47               : 5;
	uint64_t addr1                        : 11;
	uint64_t reserved_59_63               : 5;
#endif
	} s;
	struct cvmx_sso_pnd_ecc_st_s          cn68xx;
	struct cvmx_sso_pnd_ecc_st_s          cn68xxp1;
};
typedef union cvmx_sso_pnd_ecc_st cvmx_sso_pnd_ecc_st_t;

/**
 * cvmx_sso_pp#_arb
 *
 * For diagnostic use, returns the group affinity arbitration state for each core.
 *
 */
union cvmx_sso_ppx_arb {
	uint64_t u64;
	struct cvmx_sso_ppx_arb_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t aff_left                     : 4;  /**< Core affinity arbitration credits remaining on the last serviced group. */
	uint64_t reserved_8_15                : 8;
	uint64_t last_grp                     : 8;  /**< Last group number serviced by this core. */
#else
	uint64_t last_grp                     : 8;
	uint64_t reserved_8_15                : 8;
	uint64_t aff_left                     : 4;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_sso_ppx_arb_s             cn73xx;
	struct cvmx_sso_ppx_arb_s             cn78xx;
	struct cvmx_sso_ppx_arb_s             cn78xxp1;
	struct cvmx_sso_ppx_arb_s             cnf75xx;
};
typedef union cvmx_sso_ppx_arb cvmx_sso_ppx_arb_t;

/**
 * cvmx_sso_pp#_grp_msk
 *
 * CSR reserved addresses: (24): 0x5040..0x50f8
 * CSR align addresses: ===========================================================================================================
 * SSO_PPX_GRP_MSK = SSO PP Group Mask Register
 *                   (one bit per group per PP)
 *
 * Selects which group(s) a PP belongs to.  A '1' in any bit position sets the
 * PP's membership in the corresponding group.  A value of 0x0 will prevent the
 * PP from receiving new work.
 *
 * Note that these do not contain QOS level priorities for each PP.  This is a
 * change from previous POW designs.
 */
union cvmx_sso_ppx_grp_msk {
	uint64_t u64;
	struct cvmx_sso_ppx_grp_msk_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t grp_msk                      : 64; /**< PPX group mask */
#else
	uint64_t grp_msk                      : 64;
#endif
	} s;
	struct cvmx_sso_ppx_grp_msk_s         cn68xx;
	struct cvmx_sso_ppx_grp_msk_s         cn68xxp1;
};
typedef union cvmx_sso_ppx_grp_msk cvmx_sso_ppx_grp_msk_t;

/**
 * cvmx_sso_pp#_qos_pri
 *
 * CSR reserved addresses: (56): 0x2040..0x21f8
 * CSR align addresses: ===========================================================================================================
 * SSO_PP(0..31)_QOS_PRI = SSO PP QOS Priority Register
 *                                (one field per IQ per PP)
 *
 * Contains the QOS level priorities for each PP.
 *      0x0       is the highest priority
 *      0x7       is the lowest priority
 *      0xf       prevents the PP from receiving work from that QOS level
 *      0x8-0xe   Reserved
 *
 * For a given PP, priorities should begin at 0x0, and remain contiguous
 * throughout the range.  Failure to do so may result in severe
 * performance degradation.
 *
 *
 * Priorities for IQs 0..7
 */
union cvmx_sso_ppx_qos_pri {
	uint64_t u64;
	struct cvmx_sso_ppx_qos_pri_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t qos7_pri                     : 4;  /**< QOS7 priority for PPX */
	uint64_t reserved_52_55               : 4;
	uint64_t qos6_pri                     : 4;  /**< QOS6 priority for PPX */
	uint64_t reserved_44_47               : 4;
	uint64_t qos5_pri                     : 4;  /**< QOS5 priority for PPX */
	uint64_t reserved_36_39               : 4;
	uint64_t qos4_pri                     : 4;  /**< QOS4 priority for PPX */
	uint64_t reserved_28_31               : 4;
	uint64_t qos3_pri                     : 4;  /**< QOS3 priority for PPX */
	uint64_t reserved_20_23               : 4;
	uint64_t qos2_pri                     : 4;  /**< QOS2 priority for PPX */
	uint64_t reserved_12_15               : 4;
	uint64_t qos1_pri                     : 4;  /**< QOS1 priority for PPX */
	uint64_t reserved_4_7                 : 4;
	uint64_t qos0_pri                     : 4;  /**< QOS0 priority for PPX */
#else
	uint64_t qos0_pri                     : 4;
	uint64_t reserved_4_7                 : 4;
	uint64_t qos1_pri                     : 4;
	uint64_t reserved_12_15               : 4;
	uint64_t qos2_pri                     : 4;
	uint64_t reserved_20_23               : 4;
	uint64_t qos3_pri                     : 4;
	uint64_t reserved_28_31               : 4;
	uint64_t qos4_pri                     : 4;
	uint64_t reserved_36_39               : 4;
	uint64_t qos5_pri                     : 4;
	uint64_t reserved_44_47               : 4;
	uint64_t qos6_pri                     : 4;
	uint64_t reserved_52_55               : 4;
	uint64_t qos7_pri                     : 4;
	uint64_t reserved_60_63               : 4;
#endif
	} s;
	struct cvmx_sso_ppx_qos_pri_s         cn68xx;
	struct cvmx_sso_ppx_qos_pri_s         cn68xxp1;
};
typedef union cvmx_sso_ppx_qos_pri cvmx_sso_ppx_qos_pri_t;

/**
 * cvmx_sso_pp#_s#_grpmsk#
 *
 * These registers select which group or groups a core belongs to. There are 2 sets of masks per
 * core, each with 1 register corresponding to 64 groups.
 */
union cvmx_sso_ppx_sx_grpmskx {
	uint64_t u64;
	struct cvmx_sso_ppx_sx_grpmskx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t grp_msk                      : 64; /**< Core group mask. A one in any bit position sets the core's membership in the corresponding
                                                         group for groups <63:0>.
                                                         A value of 0x0 in GRPMSK for a given core prevents the core from receiving new
                                                         work. Cores that will never receive work should use GRPMSK=0x0; while this setting
                                                         is not special in SSO, for backward and forward compatibility this may enable reallocation
                                                         of internal resources to the remaining (nonzero-mask) cores. */
#else
	uint64_t grp_msk                      : 64;
#endif
	} s;
	struct cvmx_sso_ppx_sx_grpmskx_s      cn73xx;
	struct cvmx_sso_ppx_sx_grpmskx_s      cn78xx;
	struct cvmx_sso_ppx_sx_grpmskx_s      cn78xxp1;
	struct cvmx_sso_ppx_sx_grpmskx_s      cnf75xx;
};
typedef union cvmx_sso_ppx_sx_grpmskx cvmx_sso_ppx_sx_grpmskx_t;

/**
 * cvmx_sso_pp_strict
 *
 * SSO_PP_STRICT = SSO Strict Priority
 *
 * This register controls getting work from the input queues.  If the bit
 * corresponding to a PP is set, that PP will not take work off the input
 * queues until it is known that there is no higher-priority work available.
 *
 * Setting SSO_PP_STRICT may incur a performance penalty if highest-priority
 * work is not found early.
 *
 * It is possible to starve a PP of work with SSO_PP_STRICT.  If the
 * SSO_PPX_GRP_MSK for a PP masks-out much of the work added to the input
 * queues that are higher-priority for that PP, and if there is a constant
 * stream of work through one or more of those higher-priority input queues,
 * then that PP may not accept work from lower-priority input queues.  This can
 * be alleviated by ensuring that most or all the work added to the
 * higher-priority input queues for a PP with SSO_PP_STRICT set are in a group
 * acceptable to that PP.
 *
 * It is also possible to neglect work in an input queue if SSO_PP_STRICT is
 * used.  If an input queue is a lower-priority queue for all PPs, and if all
 * the PPs have their corresponding bit in SSO_PP_STRICT set, then work may
 * never be taken (or be seldom taken) from that queue.  This can be alleviated
 * by ensuring that work in all input queues can be serviced by one or more PPs
 * that do not have SSO_PP_STRICT set, or that the input queue is the
 * highest-priority input queue for one or more PPs that do have SSO_PP_STRICT
 * set.
 */
union cvmx_sso_pp_strict {
	uint64_t u64;
	struct cvmx_sso_pp_strict_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t pp_strict                    : 32; /**< Corresponding PP operates in strict mode. */
#else
	uint64_t pp_strict                    : 32;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sso_pp_strict_s           cn68xx;
	struct cvmx_sso_pp_strict_s           cn68xxp1;
};
typedef union cvmx_sso_pp_strict cvmx_sso_pp_strict_t;

/**
 * cvmx_sso_qos#_rnd
 *
 * CSR align addresses: ===========================================================================================================
 * SSO_QOS(0..7)_RND = SSO QOS Issue Round Register
 *                (one per IQ)
 *
 * The number of arbitration rounds each QOS level participates in.
 */
union cvmx_sso_qosx_rnd {
	uint64_t u64;
	struct cvmx_sso_qosx_rnd_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t rnds_qos                     : 8;  /**< Number of rounds to participate in for IQ(X). */
#else
	uint64_t rnds_qos                     : 8;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_sso_qosx_rnd_s            cn68xx;
	struct cvmx_sso_qosx_rnd_s            cn68xxp1;
};
typedef union cvmx_sso_qosx_rnd cvmx_sso_qosx_rnd_t;

/**
 * cvmx_sso_qos_thr#
 *
 * CSR reserved addresses: (24): 0xa040..0xa0f8
 * CSR align addresses: ===========================================================================================================
 * SSO_QOS_THRX = SSO QOS Threshold Register
 *                (one per QOS level)
 *
 * Contains the thresholds for allocating SSO internal storage buffers.  If the
 * number of remaining free buffers drops below the minimum threshold (MIN_THR)
 * or the number of allocated buffers for this QOS level rises above the
 * maximum threshold (MAX_THR), future incoming work queue entries will be
 * buffered externally rather than internally.  This register also contains the
 * number of internal buffers currently allocated to this QOS level (BUF_CNT).
 */
union cvmx_sso_qos_thrx {
	uint64_t u64;
	struct cvmx_sso_qos_thrx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_40_63               : 24;
	uint64_t buf_cnt                      : 12; /**< # of internal buffers allocated to QOS level X */
	uint64_t reserved_26_27               : 2;
	uint64_t max_thr                      : 12; /**< Max threshold for QOS level X
                                                         For performance reasons, MAX_THR can have a slop of 6
                                                         WQE for QOS level X. */
	uint64_t reserved_12_13               : 2;
	uint64_t min_thr                      : 12; /**< Min threshold for QOS level X
                                                         For performance reasons, MIN_THR can have a slop of 6
                                                         WQEs for QOS level X. */
#else
	uint64_t min_thr                      : 12;
	uint64_t reserved_12_13               : 2;
	uint64_t max_thr                      : 12;
	uint64_t reserved_26_27               : 2;
	uint64_t buf_cnt                      : 12;
	uint64_t reserved_40_63               : 24;
#endif
	} s;
	struct cvmx_sso_qos_thrx_s            cn68xx;
	struct cvmx_sso_qos_thrx_s            cn68xxp1;
};
typedef union cvmx_sso_qos_thrx cvmx_sso_qos_thrx_t;

/**
 * cvmx_sso_qos_we
 *
 * SSO_QOS_WE = SSO WE Buffers
 *
 * This register contains a read-only count of the current number of free
 * buffers (FREE_CNT) and the total number of tag chain heads on the de-schedule list
 * (DES_CNT) (which is not the same as the total number of entries on all of the descheduled
 * tag chains.)
 */
union cvmx_sso_qos_we {
	uint64_t u64;
	struct cvmx_sso_qos_we_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_26_63               : 38;
	uint64_t des_cnt                      : 12; /**< Number of buffers on de-schedule list */
	uint64_t reserved_12_13               : 2;
	uint64_t free_cnt                     : 12; /**< Number of total free buffers */
#else
	uint64_t free_cnt                     : 12;
	uint64_t reserved_12_13               : 2;
	uint64_t des_cnt                      : 12;
	uint64_t reserved_26_63               : 38;
#endif
	} s;
	struct cvmx_sso_qos_we_s              cn68xx;
	struct cvmx_sso_qos_we_s              cn68xxp1;
};
typedef union cvmx_sso_qos_we cvmx_sso_qos_we_t;

/**
 * cvmx_sso_reset
 *
 * Writing a 1 to SSO_RESET[RESET] resets the SSO. After receiving a store to this CSR, the SSO
 * must not be sent any other operations for 2500 coprocessor (SCLK) cycles. Note that the
 * contents of this register are reset along with the rest of the SSO.
 */
union cvmx_sso_reset {
	uint64_t u64;
	struct cvmx_sso_reset_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t busy                         : 1;  /**< Initialization in progress. After reset asserts, SSO will set this bit until internal
                                                         structures are initialized. This bit must read as zero before any configuration may be
                                                         done. */
	uint64_t reserved_1_62                : 62;
	uint64_t reset                        : 1;  /**< Reset the SSO. */
#else
	uint64_t reset                        : 1;
	uint64_t reserved_1_62                : 62;
	uint64_t busy                         : 1;
#endif
	} s;
	struct cvmx_sso_reset_cn68xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t reset                        : 1;  /**< Reset the SSO */
#else
	uint64_t reset                        : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} cn68xx;
	struct cvmx_sso_reset_s               cn73xx;
	struct cvmx_sso_reset_s               cn78xx;
	struct cvmx_sso_reset_s               cn78xxp1;
	struct cvmx_sso_reset_s               cnf75xx;
};
typedef union cvmx_sso_reset cvmx_sso_reset_t;

/**
 * cvmx_sso_rwq_head_ptr#
 *
 * CSR reserved addresses: (24): 0xb040..0xb0f8
 * CSR align addresses: ===========================================================================================================
 * SSO_RWQ_HEAD_PTRX = SSO Remote Queue Head Register
 *                (one per QOS level)
 * Contains the ptr to the first entry of the remote linked list(s) for a particular
 * QoS level. SW should initialize the remote linked list(s) by programming
 * SSO_RWQ_HEAD_PTRX and SSO_RWQ_TAIL_PTRX to identical values.
 */
union cvmx_sso_rwq_head_ptrx {
	uint64_t u64;
	struct cvmx_sso_rwq_head_ptrx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_38_63               : 26;
	uint64_t ptr                          : 31; /**< Head Pointer */
	uint64_t reserved_5_6                 : 2;
	uint64_t rctr                         : 5;  /**< Index of next WQE entry in fill packet to be
                                                         processed (inbound queues) */
#else
	uint64_t rctr                         : 5;
	uint64_t reserved_5_6                 : 2;
	uint64_t ptr                          : 31;
	uint64_t reserved_38_63               : 26;
#endif
	} s;
	struct cvmx_sso_rwq_head_ptrx_s       cn68xx;
	struct cvmx_sso_rwq_head_ptrx_s       cn68xxp1;
};
typedef union cvmx_sso_rwq_head_ptrx cvmx_sso_rwq_head_ptrx_t;

/**
 * cvmx_sso_rwq_pop_fptr
 *
 * SSO_RWQ_POP_FPTR = SSO Pop Free Pointer
 *
 * This register is used by SW to remove pointers for buffer-reallocation and diagnostics, and
 * should only be used when SSO is idle.
 *
 * To remove ALL pointers, software must insure that there are modulus 16
 * pointers in the FPA.  To do this, SSO_CFG.RWQ_BYP_DIS must be set, the FPA
 * pointer count read, and enough fake buffers pushed via SSO_RWQ_PSH_FPTR to
 * bring the FPA pointer count up to mod 16.
 */
union cvmx_sso_rwq_pop_fptr {
	uint64_t u64;
	struct cvmx_sso_rwq_pop_fptr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t val                          : 1;  /**< Free Pointer Valid */
	uint64_t reserved_38_62               : 25;
	uint64_t fptr                         : 31; /**< Free Pointer */
	uint64_t reserved_0_6                 : 7;
#else
	uint64_t reserved_0_6                 : 7;
	uint64_t fptr                         : 31;
	uint64_t reserved_38_62               : 25;
	uint64_t val                          : 1;
#endif
	} s;
	struct cvmx_sso_rwq_pop_fptr_s        cn68xx;
	struct cvmx_sso_rwq_pop_fptr_s        cn68xxp1;
};
typedef union cvmx_sso_rwq_pop_fptr cvmx_sso_rwq_pop_fptr_t;

/**
 * cvmx_sso_rwq_psh_fptr
 *
 * CSR reserved addresses: (56): 0xc240..0xc3f8
 * SSO_RWQ_PSH_FPTR = SSO Free Pointer FIFO
 *
 * This register is used by SW to initialize the SSO with a pool of free
 * pointers by writing the FPTR field whenever FULL = 0. Free pointers are
 * fetched/released from/to the pool when accessing WQE entries stored remotely
 * (in remote linked lists).  Free pointers should be 128 byte aligned, each of
 * 256 bytes. This register should only be used when SSO is idle.
 *
 * Software needs to set aside buffering for
 *      8 + 48 + ROUNDUP(N/26)
 *
 * where as many as N DRAM work queue entries may be used.  The first 8 buffers
 * are used to setup the SSO_RWQ_HEAD_PTR and SSO_RWQ_TAIL_PTRs, and the
 * remainder are pushed via this register.
 *
 * IMPLEMENTATION NOTES--NOT FOR SPEC:
 *      48 avoids false out of buffer error due to (16) FPA and in-sso FPA buffering (32)
 *      26 is number of WAE's per 256B buffer
 */
union cvmx_sso_rwq_psh_fptr {
	uint64_t u64;
	struct cvmx_sso_rwq_psh_fptr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t full                         : 1;  /**< FIFO Full.  When set, the FPA is busy writing entries
                                                         and software must wait before adding new entries. */
	uint64_t reserved_38_62               : 25;
	uint64_t fptr                         : 31; /**< Free Pointer */
	uint64_t reserved_0_6                 : 7;
#else
	uint64_t reserved_0_6                 : 7;
	uint64_t fptr                         : 31;
	uint64_t reserved_38_62               : 25;
	uint64_t full                         : 1;
#endif
	} s;
	struct cvmx_sso_rwq_psh_fptr_s        cn68xx;
	struct cvmx_sso_rwq_psh_fptr_s        cn68xxp1;
};
typedef union cvmx_sso_rwq_psh_fptr cvmx_sso_rwq_psh_fptr_t;

/**
 * cvmx_sso_rwq_tail_ptr#
 *
 * CSR reserved addresses: (56): 0xc040..0xc1f8
 * SSO_RWQ_TAIL_PTRX = SSO Remote Queue Tail Register
 *                (one per QOS level)
 * Contains the ptr to the last entry of the remote linked list(s) for a particular
 * QoS level. SW must initialize the remote linked list(s) by programming
 * SSO_RWQ_HEAD_PTRX and SSO_RWQ_TAIL_PTRX to identical values.
 */
union cvmx_sso_rwq_tail_ptrx {
	uint64_t u64;
	struct cvmx_sso_rwq_tail_ptrx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_38_63               : 26;
	uint64_t ptr                          : 31; /**< Tail Pointer */
	uint64_t reserved_5_6                 : 2;
	uint64_t rctr                         : 5;  /**< Number of entries waiting to be sent out to external
                                                         RAM (outbound queues) */
#else
	uint64_t rctr                         : 5;
	uint64_t reserved_5_6                 : 2;
	uint64_t ptr                          : 31;
	uint64_t reserved_38_63               : 26;
#endif
	} s;
	struct cvmx_sso_rwq_tail_ptrx_s       cn68xx;
	struct cvmx_sso_rwq_tail_ptrx_s       cn68xxp1;
};
typedef union cvmx_sso_rwq_tail_ptrx cvmx_sso_rwq_tail_ptrx_t;

/**
 * cvmx_sso_sl_pp#_links
 *
 * Returns status of each core.
 *
 */
union cvmx_sso_sl_ppx_links {
	uint64_t u64;
	struct cvmx_sso_sl_ppx_links_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t tailc                        : 1;  /**< Set when this SSO entry is the tail of the conflicted tail chain, and so there are no
                                                         additional conflicts on this tag chain. */
	uint64_t reserved_60_62               : 3;
	uint64_t index                        : 12; /**< The SSO entry attached to the core. */
	uint64_t reserved_38_47               : 10;
	uint64_t grp                          : 10; /**< The group attached to the core (updated when new tag list entered on SWTAG_FULL). */
	uint64_t head                         : 1;  /**< Set when this SSO entry is at the head of its tag list, or when in the UNTAGGED or EMPTY state. */
	uint64_t tail                         : 1;  /**< Set when this SSO entry is at the tail of its tag list, or when in the UNTAGGED or EMPTY state. */
	uint64_t reserved_0_25                : 26;
#else
	uint64_t reserved_0_25                : 26;
	uint64_t tail                         : 1;
	uint64_t head                         : 1;
	uint64_t grp                          : 10;
	uint64_t reserved_38_47               : 10;
	uint64_t index                        : 12;
	uint64_t reserved_60_62               : 3;
	uint64_t tailc                        : 1;
#endif
	} s;
	struct cvmx_sso_sl_ppx_links_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t tailc                        : 1;  /**< Set when this SSO entry is the tail of the conflicted tail chain, and so there are no
                                                         additional conflicts on this tag chain. */
	uint64_t reserved_58_62               : 5;
	uint64_t index                        : 10; /**< The SSO entry attached to the core. */
	uint64_t reserved_36_47               : 12;
	uint64_t grp                          : 8;  /**< The group attached to the core (updated when new tag list entered on SWTAG_FULL). */
	uint64_t head                         : 1;  /**< Set when this SSO entry is at the head of its tag list, or when in the UNTAGGED or EMPTY state. */
	uint64_t tail                         : 1;  /**< Set when this SSO entry is at the tail of its tag list, or when in the UNTAGGED or EMPTY state. */
	uint64_t reserved_21_25               : 5;
	uint64_t revlink_index                : 10; /**< Prior SSO entry in the tag list when HEAD=0 and TT is not UNTAGGED nor EMPTY, otherwise
                                                         unpredictable. */
	uint64_t link_index_vld               : 1;  /**< Indicates LINK_INDEX is valid. LINK_INDEX_VLD is itself valid when TAIL=1 and TT=ATOMIC,
                                                         otherwise unpredictable. */
	uint64_t link_index                   : 10; /**< Next SSO entry in the tag list when LINK_INDEX_VLD=1, TAILC=0 and TT=ATOMIC,
                                                         otherwise unpredictable. */
#else
	uint64_t link_index                   : 10;
	uint64_t link_index_vld               : 1;
	uint64_t revlink_index                : 10;
	uint64_t reserved_21_25               : 5;
	uint64_t tail                         : 1;
	uint64_t head                         : 1;
	uint64_t grp                          : 8;
	uint64_t reserved_36_47               : 12;
	uint64_t index                        : 10;
	uint64_t reserved_58_62               : 5;
	uint64_t tailc                        : 1;
#endif
	} cn73xx;
	struct cvmx_sso_sl_ppx_links_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t tailc                        : 1;  /**< Set when this SSO entry is the tail of the conflicted tail chain, and so there are no
                                                         additional conflicts on this tag chain. */
	uint64_t reserved_60_62               : 3;
	uint64_t index                        : 12; /**< The SSO entry attached to the core. */
	uint64_t reserved_38_47               : 10;
	uint64_t grp                          : 10; /**< The group attached to the core (updated when new tag list entered on SWTAG_FULL).
                                                         The upper two bits are hardcoded to the node number. */
	uint64_t head                         : 1;  /**< Set when this SSO entry is at the head of its tag list, or when in the UNTAGGED or EMPTY state. */
	uint64_t tail                         : 1;  /**< Set when this SSO entry is at the tail of its tag list, or when in the UNTAGGED or EMPTY state. */
	uint64_t reserved_25_25               : 1;
	uint64_t revlink_index                : 12; /**< Prior SSO entry in the tag list when HEAD=0 and TT is not UNTAGGED nor EMPTY, otherwise
                                                         unpredictable. */
	uint64_t link_index_vld               : 1;  /**< Indicates LINK_INDEX is valid. LINK_INDEX_VLD is itself valid when TAIL=1 and TT=ATOMIC,
                                                         otherwise unpredictable. */
	uint64_t link_index                   : 12; /**< Next SSO entry in the tag list when LINK_INDEX_VLD=1, TAILC=0 and TT=ATOMIC,
                                                         otherwise unpredictable. */
#else
	uint64_t link_index                   : 12;
	uint64_t link_index_vld               : 1;
	uint64_t revlink_index                : 12;
	uint64_t reserved_25_25               : 1;
	uint64_t tail                         : 1;
	uint64_t head                         : 1;
	uint64_t grp                          : 10;
	uint64_t reserved_38_47               : 10;
	uint64_t index                        : 12;
	uint64_t reserved_60_62               : 3;
	uint64_t tailc                        : 1;
#endif
	} cn78xx;
	struct cvmx_sso_sl_ppx_links_cn78xx   cn78xxp1;
	struct cvmx_sso_sl_ppx_links_cn73xx   cnf75xx;
};
typedef union cvmx_sso_sl_ppx_links cvmx_sso_sl_ppx_links_t;

/**
 * cvmx_sso_sl_pp#_pendtag
 *
 * Returns status of each core.
 *
 */
union cvmx_sso_sl_ppx_pendtag {
	uint64_t u64;
	struct cvmx_sso_sl_ppx_pendtag_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pend_switch                  : 1;  /**< Set when there is a pending SWTAG, SWTAG_DESCHED, or SWTAG_FULL to ORDERED or ATOMIC. If
                                                         the register read was issued after an indexed GET_WORK, the DESCHED portion of a
                                                         SWTAG_DESCHED cannot still be pending. */
	uint64_t pend_get_work                : 1;  /**< Set when there is a pending GET_WORK. */
	uint64_t pend_get_work_wait           : 1;  /**< When PEND_GET_WORK is set, indicates that the WAITW bit was set. */
	uint64_t pend_nosched                 : 1;  /**< Set when nosched is desired and PEND_DESCHED is set. */
	uint64_t pend_nosched_clr             : 1;  /**< Set when there is a pending CLR_NSCHED. */
	uint64_t pend_desched                 : 1;  /**< Set when there is a pending DESCHED or SWTAG_DESCHED. */
	uint64_t pend_alloc_we                : 1;  /**< Set when there is a pending ALLOC_WE. */
	uint64_t pend_gw_insert               : 1;  /**< Set when there is a pending GET_WORK insertion. */
	uint64_t reserved_34_55               : 22;
	uint64_t pend_tt                      : 2;  /**< The tag type when PEND_SWITCH is set. */
	uint64_t pend_tag                     : 32; /**< The tag when PEND_SWITCH is set. */
#else
	uint64_t pend_tag                     : 32;
	uint64_t pend_tt                      : 2;
	uint64_t reserved_34_55               : 22;
	uint64_t pend_gw_insert               : 1;
	uint64_t pend_alloc_we                : 1;
	uint64_t pend_desched                 : 1;
	uint64_t pend_nosched_clr             : 1;
	uint64_t pend_nosched                 : 1;
	uint64_t pend_get_work_wait           : 1;
	uint64_t pend_get_work                : 1;
	uint64_t pend_switch                  : 1;
#endif
	} s;
	struct cvmx_sso_sl_ppx_pendtag_s      cn73xx;
	struct cvmx_sso_sl_ppx_pendtag_s      cn78xx;
	struct cvmx_sso_sl_ppx_pendtag_s      cn78xxp1;
	struct cvmx_sso_sl_ppx_pendtag_s      cnf75xx;
};
typedef union cvmx_sso_sl_ppx_pendtag cvmx_sso_sl_ppx_pendtag_t;

/**
 * cvmx_sso_sl_pp#_pendwqp
 *
 * Returns status of each core.
 *
 */
union cvmx_sso_sl_ppx_pendwqp {
	uint64_t u64;
	struct cvmx_sso_sl_ppx_pendwqp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pend_switch                  : 1;  /**< Set when there is a pending SWTAG, SWTAG_DESCHED, or SWTAG_FULL to ORDERED or ATOMIC. If
                                                         the status load was issued after an indexed GET_WORK, the DESCHED portion of a
                                                         SWTAG_DESCHED cannot still be pending. */
	uint64_t pend_get_work                : 1;  /**< Set when there is a pending GET_WORK. */
	uint64_t pend_get_work_wait           : 1;  /**< When PEND_GET_WORK is set, indicates that the WAITW bit was set. */
	uint64_t pend_nosched                 : 1;  /**< Set when nosched is desired and PEND_DESCHED is set. */
	uint64_t pend_nosched_clr             : 1;  /**< Set when there is a pending CLR_NSCHED. */
	uint64_t pend_desched                 : 1;  /**< Set when there is a pending DESCHED or SWTAG_DESCHED. */
	uint64_t pend_alloc_we                : 1;  /**< Set when there is a pending ALLOC_WE. */
	uint64_t reserved_56_56               : 1;
	uint64_t pend_index                   : 12; /**< The index when PEND_NOSCHED_CLR is set. */
	uint64_t reserved_42_43               : 2;
	uint64_t pend_wqp                     : 42; /**< The WQP when PEND_NOSCHED_CLR is set. */
#else
	uint64_t pend_wqp                     : 42;
	uint64_t reserved_42_43               : 2;
	uint64_t pend_index                   : 12;
	uint64_t reserved_56_56               : 1;
	uint64_t pend_alloc_we                : 1;
	uint64_t pend_desched                 : 1;
	uint64_t pend_nosched_clr             : 1;
	uint64_t pend_nosched                 : 1;
	uint64_t pend_get_work_wait           : 1;
	uint64_t pend_get_work                : 1;
	uint64_t pend_switch                  : 1;
#endif
	} s;
	struct cvmx_sso_sl_ppx_pendwqp_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t pend_switch                  : 1;  /**< Set when there is a pending SWTAG, SWTAG_DESCHED, or SWTAG_FULL to ORDERED or ATOMIC. If
                                                         the status load was issued after an indexed GET_WORK, the DESCHED portion of a
                                                         SWTAG_DESCHED cannot still be pending. */
	uint64_t pend_get_work                : 1;  /**< Set when there is a pending GET_WORK. */
	uint64_t pend_get_work_wait           : 1;  /**< When PEND_GET_WORK is set, indicates that the WAITW bit was set. */
	uint64_t pend_nosched                 : 1;  /**< Set when nosched is desired and PEND_DESCHED is set. */
	uint64_t pend_nosched_clr             : 1;  /**< Set when there is a pending CLR_NSCHED. */
	uint64_t pend_desched                 : 1;  /**< Set when there is a pending DESCHED or SWTAG_DESCHED. */
	uint64_t pend_alloc_we                : 1;  /**< Set when there is a pending ALLOC_WE. */
	uint64_t reserved_54_56               : 3;
	uint64_t pend_index                   : 10; /**< The index when PEND_NOSCHED_CLR is set. */
	uint64_t reserved_42_43               : 2;
	uint64_t pend_wqp                     : 42; /**< The WQP when PEND_NOSCHED_CLR is set. */
#else
	uint64_t pend_wqp                     : 42;
	uint64_t reserved_42_43               : 2;
	uint64_t pend_index                   : 10;
	uint64_t reserved_54_56               : 3;
	uint64_t pend_alloc_we                : 1;
	uint64_t pend_desched                 : 1;
	uint64_t pend_nosched_clr             : 1;
	uint64_t pend_nosched                 : 1;
	uint64_t pend_get_work_wait           : 1;
	uint64_t pend_get_work                : 1;
	uint64_t pend_switch                  : 1;
#endif
	} cn73xx;
	struct cvmx_sso_sl_ppx_pendwqp_s      cn78xx;
	struct cvmx_sso_sl_ppx_pendwqp_s      cn78xxp1;
	struct cvmx_sso_sl_ppx_pendwqp_cn73xx cnf75xx;
};
typedef union cvmx_sso_sl_ppx_pendwqp cvmx_sso_sl_ppx_pendwqp_t;

/**
 * cvmx_sso_sl_pp#_tag
 *
 * Returns status of each core.
 *
 */
union cvmx_sso_sl_ppx_tag {
	uint64_t u64;
	struct cvmx_sso_sl_ppx_tag_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t tailc                        : 1;  /**< Set when this SSO entry is the tail of the conflicted tail chain, and so there are no
                                                         additional conflicts on this tag chain. */
	uint64_t reserved_60_62               : 3;
	uint64_t index                        : 12; /**< The SSO entry attached to the core. */
	uint64_t reserved_46_47               : 2;
	uint64_t grp                          : 10; /**< The group attached to the core (updated when new tag list entered on SWTAG_FULL). */
	uint64_t head                         : 1;  /**< Set when this SSO entry is at the head of its tag list, or when in the UNTAGGED or EMPTY state. */
	uint64_t tail                         : 1;  /**< Set when this SSO entry is at the tail of its tag list, or when in the UNTAGGED or EMPTY state. */
	uint64_t tt                           : 2;  /**< The tag type attached to the core (updated when new tag list entered on SWTAG, SWTAG_FULL,
                                                         or SWTAG_DESCHED.) */
	uint64_t tag                          : 32; /**< The tag attached to the core (updated when new tag list entered on SWTAG, SWTAG_FULL, or
                                                         SWTAG_DESCHED.) */
#else
	uint64_t tag                          : 32;
	uint64_t tt                           : 2;
	uint64_t tail                         : 1;
	uint64_t head                         : 1;
	uint64_t grp                          : 10;
	uint64_t reserved_46_47               : 2;
	uint64_t index                        : 12;
	uint64_t reserved_60_62               : 3;
	uint64_t tailc                        : 1;
#endif
	} s;
	struct cvmx_sso_sl_ppx_tag_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t tailc                        : 1;  /**< Set when this SSO entry is the tail of the conflicted tail chain, and so there are no
                                                         additional conflicts on this tag chain. */
	uint64_t reserved_58_62               : 5;
	uint64_t index                        : 10; /**< The SSO entry attached to the core. */
	uint64_t reserved_44_47               : 4;
	uint64_t grp                          : 8;  /**< The group attached to the core (updated when new tag list entered on SWTAG_FULL). */
	uint64_t head                         : 1;  /**< Set when this SSO entry is at the head of its tag list, or when in the UNTAGGED or EMPTY state. */
	uint64_t tail                         : 1;  /**< Set when this SSO entry is at the tail of its tag list, or when in the UNTAGGED or EMPTY state. */
	uint64_t tt                           : 2;  /**< The tag type attached to the core (updated when new tag list entered on SWTAG, SWTAG_FULL,
                                                         or SWTAG_DESCHED.) */
	uint64_t tag                          : 32; /**< The tag attached to the core (updated when new tag list entered on SWTAG, SWTAG_FULL, or
                                                         SWTAG_DESCHED.) */
#else
	uint64_t tag                          : 32;
	uint64_t tt                           : 2;
	uint64_t tail                         : 1;
	uint64_t head                         : 1;
	uint64_t grp                          : 8;
	uint64_t reserved_44_47               : 4;
	uint64_t index                        : 10;
	uint64_t reserved_58_62               : 5;
	uint64_t tailc                        : 1;
#endif
	} cn73xx;
	struct cvmx_sso_sl_ppx_tag_s          cn78xx;
	struct cvmx_sso_sl_ppx_tag_s          cn78xxp1;
	struct cvmx_sso_sl_ppx_tag_cn73xx     cnf75xx;
};
typedef union cvmx_sso_sl_ppx_tag cvmx_sso_sl_ppx_tag_t;

/**
 * cvmx_sso_sl_pp#_wqp
 *
 * Returns status of each core.
 *
 */
union cvmx_sso_sl_ppx_wqp {
	uint64_t u64;
	struct cvmx_sso_sl_ppx_wqp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_58_63               : 6;
	uint64_t grp                          : 10; /**< The group attached to the core (updated when new tag list entered on SWTAG_FULL). */
	uint64_t reserved_42_47               : 6;
	uint64_t wqp                          : 42; /**< The WQP attached to the core (updated when new tag list entered on SWTAG_FULL.) */
#else
	uint64_t wqp                          : 42;
	uint64_t reserved_42_47               : 6;
	uint64_t grp                          : 10;
	uint64_t reserved_58_63               : 6;
#endif
	} s;
	struct cvmx_sso_sl_ppx_wqp_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_56_63               : 8;
	uint64_t grp                          : 8;  /**< The group attached to the core (updated when new tag list entered on SWTAG_FULL). */
	uint64_t reserved_42_47               : 6;
	uint64_t wqp                          : 42; /**< The WQP attached to the core (updated when new tag list entered on SWTAG_FULL.) */
#else
	uint64_t wqp                          : 42;
	uint64_t reserved_42_47               : 6;
	uint64_t grp                          : 8;
	uint64_t reserved_56_63               : 8;
#endif
	} cn73xx;
	struct cvmx_sso_sl_ppx_wqp_s          cn78xx;
	struct cvmx_sso_sl_ppx_wqp_s          cn78xxp1;
	struct cvmx_sso_sl_ppx_wqp_cn73xx     cnf75xx;
};
typedef union cvmx_sso_sl_ppx_wqp cvmx_sso_sl_ppx_wqp_t;

/**
 * cvmx_sso_taq#_link
 *
 * Returns TAQ status for a given line.
 *
 */
union cvmx_sso_taqx_link {
	uint64_t u64;
	struct cvmx_sso_taqx_link_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_11_63               : 53;
	uint64_t next                         : 11; /**< Next TAQ entry in linked list. Only valid when not at the tail of the list. */
#else
	uint64_t next                         : 11;
	uint64_t reserved_11_63               : 53;
#endif
	} s;
	struct cvmx_sso_taqx_link_s           cn73xx;
	struct cvmx_sso_taqx_link_s           cn78xx;
	struct cvmx_sso_taqx_link_s           cn78xxp1;
	struct cvmx_sso_taqx_link_s           cnf75xx;
};
typedef union cvmx_sso_taqx_link cvmx_sso_taqx_link_t;

/**
 * cvmx_sso_taq#_wae#_tag
 *
 * Returns TAQ status for a given line and WAE within that line.
 *
 */
union cvmx_sso_taqx_waex_tag {
	uint64_t u64;
	struct cvmx_sso_taqx_waex_tag_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_34_63               : 30;
	uint64_t tt                           : 2;  /**< The tag type of the TAQ entry. Enumerated by SSO_TT_E. */
	uint64_t tag                          : 32; /**< The tag of the TAQ entry. */
#else
	uint64_t tag                          : 32;
	uint64_t tt                           : 2;
	uint64_t reserved_34_63               : 30;
#endif
	} s;
	struct cvmx_sso_taqx_waex_tag_s       cn73xx;
	struct cvmx_sso_taqx_waex_tag_s       cn78xx;
	struct cvmx_sso_taqx_waex_tag_s       cn78xxp1;
	struct cvmx_sso_taqx_waex_tag_s       cnf75xx;
};
typedef union cvmx_sso_taqx_waex_tag cvmx_sso_taqx_waex_tag_t;

/**
 * cvmx_sso_taq#_wae#_wqp
 *
 * Returns TAQ status for a given line and WAE within that line.
 *
 */
union cvmx_sso_taqx_waex_wqp {
	uint64_t u64;
	struct cvmx_sso_taqx_waex_wqp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_42_63               : 22;
	uint64_t wqp                          : 42; /**< Work queue pointer held in the TAQ entry. Bits <2:0> are always zero. */
#else
	uint64_t wqp                          : 42;
	uint64_t reserved_42_63               : 22;
#endif
	} s;
	struct cvmx_sso_taqx_waex_wqp_s       cn73xx;
	struct cvmx_sso_taqx_waex_wqp_s       cn78xx;
	struct cvmx_sso_taqx_waex_wqp_s       cn78xxp1;
	struct cvmx_sso_taqx_waex_wqp_s       cnf75xx;
};
typedef union cvmx_sso_taqx_waex_wqp cvmx_sso_taqx_waex_wqp_t;

/**
 * cvmx_sso_taq_add
 */
union cvmx_sso_taq_add {
	uint64_t u64;
	struct cvmx_sso_taq_add_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_29_63               : 35;
	uint64_t rsvd_free                    : 13; /**< Written value is added to SSO_TAQ_CNT[RSVD_FREE] to prevent races between software and
                                                         hardware changes. This is a two's complement value so subtraction may also be performed. */
	uint64_t reserved_0_15                : 16;
#else
	uint64_t reserved_0_15                : 16;
	uint64_t rsvd_free                    : 13;
	uint64_t reserved_29_63               : 35;
#endif
	} s;
	struct cvmx_sso_taq_add_s             cn73xx;
	struct cvmx_sso_taq_add_s             cn78xx;
	struct cvmx_sso_taq_add_s             cn78xxp1;
	struct cvmx_sso_taq_add_s             cnf75xx;
};
typedef union cvmx_sso_taq_add cvmx_sso_taq_add_t;

/**
 * cvmx_sso_taq_cnt
 */
union cvmx_sso_taq_cnt {
	uint64_t u64;
	struct cvmx_sso_taq_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_27_63               : 37;
	uint64_t rsvd_free                    : 11; /**< Number of free reserved buffers. Used to ensure each group may get a specific number of
                                                         buffers. Must always be greater than or equal to the sum across all
                                                         SSO_GRP()_TAQ_THR[RSVD_THR], and will generally be equal to that sum unless changes
                                                         to RSVD_THR are going to be made. To prevent races, software should not change this
                                                         register when SSO is being used; instead use SSO_TAQ_ADD[RSVD_FREE]. Legal values are
                                                         0..0x140 */
	uint64_t reserved_11_15               : 5;
	uint64_t free_cnt                     : 11; /**< Number of total free buffers. */
#else
	uint64_t free_cnt                     : 11;
	uint64_t reserved_11_15               : 5;
	uint64_t rsvd_free                    : 11;
	uint64_t reserved_27_63               : 37;
#endif
	} s;
	struct cvmx_sso_taq_cnt_s             cn73xx;
	struct cvmx_sso_taq_cnt_s             cn78xx;
	struct cvmx_sso_taq_cnt_s             cn78xxp1;
	struct cvmx_sso_taq_cnt_s             cnf75xx;
};
typedef union cvmx_sso_taq_cnt cvmx_sso_taq_cnt_t;

/**
 * cvmx_sso_tiaq#_status
 *
 * Returns TAQ inbound status indexed by group.
 *
 */
union cvmx_sso_tiaqx_status {
	uint64_t u64;
	struct cvmx_sso_tiaqx_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t wae_head                     : 4;  /**< Head's WAE number within current cache line, 0-12. This provides the second index into
                                                         SSO_TAQ()_WAE()_TAG and SSO_TAQ()_WAE()_WQP. */
	uint64_t wae_tail                     : 4;  /**< When [WAE_USED] is nonzero, this provides the next free WAE number in the cache
                                                         line of the tail entry. If 0x0, the next entry will be placed at the beginning of
                                                         a new cache line. This provides the second index into SSO_TAQ()_WAE()_TAG and
                                                         SSO_TAQ()_WAE()_WQP. */
	uint64_t reserved_47_55               : 9;
	uint64_t wae_used                     : 15; /**< Number of WAEs in use. */
	uint64_t reserved_23_31               : 9;
	uint64_t ent_head                     : 11; /**< Head's entry number. This provides the first index into SSO_TAQ()_WAE()_TAG
                                                         and SSO_TAQ()_WAE()_WQP. */
	uint64_t reserved_11_11               : 1;
	uint64_t ent_tail                     : 11; /**< Tail's entry number. This provides the first index into SSO_TAQ()_WAE()_TAG
                                                         and SSO_TAQ()_WAE()_WQP. */
#else
	uint64_t ent_tail                     : 11;
	uint64_t reserved_11_11               : 1;
	uint64_t ent_head                     : 11;
	uint64_t reserved_23_31               : 9;
	uint64_t wae_used                     : 15;
	uint64_t reserved_47_55               : 9;
	uint64_t wae_tail                     : 4;
	uint64_t wae_head                     : 4;
#endif
	} s;
	struct cvmx_sso_tiaqx_status_s        cn73xx;
	struct cvmx_sso_tiaqx_status_s        cn78xx;
	struct cvmx_sso_tiaqx_status_s        cn78xxp1;
	struct cvmx_sso_tiaqx_status_s        cnf75xx;
};
typedef union cvmx_sso_tiaqx_status cvmx_sso_tiaqx_status_t;

/**
 * cvmx_sso_toaq#_status
 *
 * Returns TAQ outbound status indexed by group.
 *
 */
union cvmx_sso_toaqx_status {
	uint64_t u64;
	struct cvmx_sso_toaqx_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_62_63               : 2;
	uint64_t ext_vld                      : 1;  /**< External queuing is in use on this group. */
	uint64_t partial                      : 1;  /**< Partial cache line is allocated to tail of queue. */
	uint64_t wae_tail                     : 4;  /**< If [PARTIAL] is set, this provides the next free WAE number in the cache line of
                                                         the tail entry. If [PARTIAL] is clear, the next entry will be placed at the
                                                         beginning of a new cache line. This provides the second index into
                                                         SSO_TAQ()_WAE()_TAG and SSO_TAQ()_WAE()_WQP. */
	uint64_t reserved_43_55               : 13;
	uint64_t cl_used                      : 11; /**< Number of cache lines in use. */
	uint64_t reserved_23_31               : 9;
	uint64_t ent_head                     : 11; /**< Head's entry number. This provides the first index into SSO_TAQ()_WAE()_TAG
                                                         and SSO_TAQ()_WAE()_WQP. */
	uint64_t reserved_11_11               : 1;
	uint64_t ent_tail                     : 11; /**< Tail's entry number. This provides the first index into SSO_TAQ()_WAE()_TAG
                                                         and SSO_TAQ()_WAE()_WQP. */
#else
	uint64_t ent_tail                     : 11;
	uint64_t reserved_11_11               : 1;
	uint64_t ent_head                     : 11;
	uint64_t reserved_23_31               : 9;
	uint64_t cl_used                      : 11;
	uint64_t reserved_43_55               : 13;
	uint64_t wae_tail                     : 4;
	uint64_t partial                      : 1;
	uint64_t ext_vld                      : 1;
	uint64_t reserved_62_63               : 2;
#endif
	} s;
	struct cvmx_sso_toaqx_status_s        cn73xx;
	struct cvmx_sso_toaqx_status_s        cn78xx;
	struct cvmx_sso_toaqx_status_s        cn78xxp1;
	struct cvmx_sso_toaqx_status_s        cnf75xx;
};
typedef union cvmx_sso_toaqx_status cvmx_sso_toaqx_status_t;

/**
 * cvmx_sso_ts_pc
 *
 * SSO_TS_PC = SSO Tag Switch Performance Counter
 *
 * Counts the number of tag switch requests.
 * Counter rolls over through zero when max value exceeded.
 */
union cvmx_sso_ts_pc {
	uint64_t u64;
	struct cvmx_sso_ts_pc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ts_pc                        : 64; /**< Tag switch performance counter */
#else
	uint64_t ts_pc                        : 64;
#endif
	} s;
	struct cvmx_sso_ts_pc_s               cn68xx;
	struct cvmx_sso_ts_pc_s               cn68xxp1;
};
typedef union cvmx_sso_ts_pc cvmx_sso_ts_pc_t;

/**
 * cvmx_sso_wa_com_pc
 *
 * SSO_WA_COM_PC = SSO Work Add Combined Performance Counter
 *
 * Counts the number of add new work requests for all QOS levels.
 * Counter rolls over through zero when max value exceeded.
 */
union cvmx_sso_wa_com_pc {
	uint64_t u64;
	struct cvmx_sso_wa_com_pc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t wa_pc                        : 64; /**< Work add combined performance counter */
#else
	uint64_t wa_pc                        : 64;
#endif
	} s;
	struct cvmx_sso_wa_com_pc_s           cn68xx;
	struct cvmx_sso_wa_com_pc_s           cn68xxp1;
};
typedef union cvmx_sso_wa_com_pc cvmx_sso_wa_com_pc_t;

/**
 * cvmx_sso_wa_pc#
 *
 * CSR reserved addresses: (64): 0x4200..0x43f8
 * CSR align addresses: ===========================================================================================================
 * SSO_WA_PCX = SSO Work Add Performance Counter
 *             (one per QOS level)
 *
 * Counts the number of add new work requests for each QOS level.
 * Counter rolls over through zero when max value exceeded.
 */
union cvmx_sso_wa_pcx {
	uint64_t u64;
	struct cvmx_sso_wa_pcx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t wa_pc                        : 64; /**< Work add performance counter for QOS level X */
#else
	uint64_t wa_pc                        : 64;
#endif
	} s;
	struct cvmx_sso_wa_pcx_s              cn68xx;
	struct cvmx_sso_wa_pcx_s              cn68xxp1;
};
typedef union cvmx_sso_wa_pcx cvmx_sso_wa_pcx_t;

/**
 * cvmx_sso_wq_int
 *
 * Note, the old POW offsets ran from 0x0 to 0x3f8, leaving the next available slot at 0x400.
 * To ensure no overlap, start on 4k boundary: 0x1000.
 * SSO_WQ_INT = SSO Work Queue Interrupt Register
 *
 * Contains the bits (one per group) that set work queue interrupts and are
 * used to clear these interrupts.  For more information regarding this
 * register, see the interrupt section of the SSO spec.
 */
union cvmx_sso_wq_int {
	uint64_t u64;
	struct cvmx_sso_wq_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t wq_int                       : 64; /**< Work queue interrupt bits
                                                         Corresponding WQ_INT bit is set by HW whenever:
                                                           - SSO_WQ_INT_CNTX[IQ_CNT] >=
                                                             SSO_WQ_INT_THRX[IQ_THR] and the threshold
                                                             interrupt is not disabled.
                                                             SSO_WQ_IQ_DISX[IQ_DIS<X>]==1 disables the interrupt
                                                             SSO_WQ_INT_THRX[IQ_THR]==0 disables the int.
                                                           - SSO_WQ_INT_CNTX[DS_CNT] >=
                                                             SSO_WQ_INT_THRX[DS_THR] and the threshold
                                                             interrupt is not disabled
                                                             SSO_WQ_INT_THRX[DS_THR]==0 disables the int.
                                                           - SSO_WQ_INT_CNTX[TC_CNT]==1 when periodic
                                                             counter SSO_WQ_INT_PC[PC]==0 and
                                                             SSO_WQ_INT_THRX[TC_EN]==1 and at least one of:
                                                               - SSO_WQ_INT_CNTX[IQ_CNT] > 0
                                                               - SSO_WQ_INT_CNTX[DS_CNT] > 0 */
#else
	uint64_t wq_int                       : 64;
#endif
	} s;
	struct cvmx_sso_wq_int_s              cn68xx;
	struct cvmx_sso_wq_int_s              cn68xxp1;
};
typedef union cvmx_sso_wq_int cvmx_sso_wq_int_t;

/**
 * cvmx_sso_wq_int_cnt#
 *
 * CSR reserved addresses: (64): 0x7200..0x73f8
 * CSR align addresses: ===========================================================================================================
 * SSO_WQ_INT_CNTX = SSO Work Queue Interrupt Count Register
 *                   (one per group)
 *
 * Contains a read-only copy of the counts used to trigger work queue
 * interrupts.  For more information regarding this register, see the interrupt
 * section.
 */
union cvmx_sso_wq_int_cntx {
	uint64_t u64;
	struct cvmx_sso_wq_int_cntx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t tc_cnt                       : 4;  /**< Time counter current value for group X
                                                         HW sets TC_CNT to SSO_WQ_INT_THRX[TC_THR] whenever:
                                                           - corresponding SSO_WQ_INT_CNTX[IQ_CNT]==0 and
                                                             corresponding SSO_WQ_INT_CNTX[DS_CNT]==0
                                                           - corresponding SSO_WQ_INT[WQ_INT<X>] is written
                                                             with a 1 by SW
                                                           - corresponding SSO_WQ_IQ_DIS[IQ_DIS<X>] is written
                                                             with a 1 by SW
                                                           - corresponding SSO_WQ_INT_THRX is written by SW
                                                           - TC_CNT==1 and periodic counter
                                                             SSO_WQ_INT_PC[PC]==0
                                                         Otherwise, HW decrements TC_CNT whenever the
                                                         periodic counter SSO_WQ_INT_PC[PC]==0.
                                                         TC_CNT is 0 whenever SSO_WQ_INT_THRX[TC_THR]==0. */
	uint64_t reserved_26_27               : 2;
	uint64_t ds_cnt                       : 12; /**< De-schedule executable count for group X */
	uint64_t reserved_12_13               : 2;
	uint64_t iq_cnt                       : 12; /**< Input queue executable count for group X */
#else
	uint64_t iq_cnt                       : 12;
	uint64_t reserved_12_13               : 2;
	uint64_t ds_cnt                       : 12;
	uint64_t reserved_26_27               : 2;
	uint64_t tc_cnt                       : 4;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_sso_wq_int_cntx_s         cn68xx;
	struct cvmx_sso_wq_int_cntx_s         cn68xxp1;
};
typedef union cvmx_sso_wq_int_cntx cvmx_sso_wq_int_cntx_t;

/**
 * cvmx_sso_wq_int_pc
 *
 * Contains the threshold value for the work-executable interrupt periodic counter and also a
 * read-only copy of the periodic counter. For more information on this register, refer to
 * Interrupts.
 */
union cvmx_sso_wq_int_pc {
	uint64_t u64;
	struct cvmx_sso_wq_int_pc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t pc                           : 28; /**< Work-executable interrupt periodic counter. */
	uint64_t reserved_28_31               : 4;
	uint64_t pc_thr                       : 20; /**< Work-executable interrupt periodic counter threshold. Zero disables the counter.
                                                         If nonzero, the value must be >= 3. */
	uint64_t reserved_0_7                 : 8;
#else
	uint64_t reserved_0_7                 : 8;
	uint64_t pc_thr                       : 20;
	uint64_t reserved_28_31               : 4;
	uint64_t pc                           : 28;
	uint64_t reserved_60_63               : 4;
#endif
	} s;
	struct cvmx_sso_wq_int_pc_s           cn68xx;
	struct cvmx_sso_wq_int_pc_s           cn68xxp1;
	struct cvmx_sso_wq_int_pc_s           cn73xx;
	struct cvmx_sso_wq_int_pc_s           cn78xx;
	struct cvmx_sso_wq_int_pc_s           cn78xxp1;
	struct cvmx_sso_wq_int_pc_s           cnf75xx;
};
typedef union cvmx_sso_wq_int_pc cvmx_sso_wq_int_pc_t;

/**
 * cvmx_sso_wq_int_thr#
 *
 * CSR reserved addresses: (96): 0x6100..0x63f8
 * CSR align addresses: ===========================================================================================================
 * SSO_WQ_INT_THR(0..63) = SSO Work Queue Interrupt Threshold Registers
 *                         (one per group)
 *
 * Contains the thresholds for enabling and setting work queue interrupts.  For
 * more information, see the interrupt section.
 *
 * Note: Up to 16 of the SSO's internal storage buffers can be allocated
 * for hardware use and are therefore not available for incoming work queue
 * entries.  Additionally, any WS that is not in the EMPTY state consumes a
 * buffer.  Thus in a 32 PP system, it is not advisable to set either IQ_THR or
 * DS_THR to greater than 2048 - 16 - 32*2 = 1968.  Doing so may prevent the
 * interrupt from ever triggering.
 *
 * Priorities for QOS levels 0..7
 */
union cvmx_sso_wq_int_thrx {
	uint64_t u64;
	struct cvmx_sso_wq_int_thrx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_33_63               : 31;
	uint64_t tc_en                        : 1;  /**< Time counter interrupt enable for group X
                                                         TC_EN must be zero when TC_THR==0 */
	uint64_t tc_thr                       : 4;  /**< Time counter interrupt threshold for group X
                                                         When TC_THR==0, SSO_WQ_INT_CNTX[TC_CNT] is zero */
	uint64_t reserved_26_27               : 2;
	uint64_t ds_thr                       : 12; /**< De-schedule count threshold for group X
                                                         DS_THR==0 disables the threshold interrupt */
	uint64_t reserved_12_13               : 2;
	uint64_t iq_thr                       : 12; /**< Input queue count threshold for group X
                                                         IQ_THR==0 disables the threshold interrupt */
#else
	uint64_t iq_thr                       : 12;
	uint64_t reserved_12_13               : 2;
	uint64_t ds_thr                       : 12;
	uint64_t reserved_26_27               : 2;
	uint64_t tc_thr                       : 4;
	uint64_t tc_en                        : 1;
	uint64_t reserved_33_63               : 31;
#endif
	} s;
	struct cvmx_sso_wq_int_thrx_s         cn68xx;
	struct cvmx_sso_wq_int_thrx_s         cn68xxp1;
};
typedef union cvmx_sso_wq_int_thrx cvmx_sso_wq_int_thrx_t;

/**
 * cvmx_sso_wq_iq_dis
 *
 * CSR reserved addresses: (1): 0x1008..0x1008
 * SSO_WQ_IQ_DIS = SSO Input Queue Interrupt Temporary Disable Mask
 *
 * Contains the input queue interrupt temporary disable bits (one per group).
 * For more information regarding this register, see the interrupt section.
 */
union cvmx_sso_wq_iq_dis {
	uint64_t u64;
	struct cvmx_sso_wq_iq_dis_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t iq_dis                       : 64; /**< Input queue interrupt temporary disable mask
                                                         Corresponding SSO_WQ_INTX[WQ_INT<X>] bit cannot be
                                                         set due to IQ_CNT/IQ_THR check when this bit is set.
                                                         Corresponding IQ_DIS bit is cleared by HW whenever:
                                                          - SSO_WQ_INT_CNTX[IQ_CNT] is zero, or
                                                          - SSO_WQ_INT_CNTX[TC_CNT]==1 when periodic
                                                            counter SSO_WQ_INT_PC[PC]==0 */
#else
	uint64_t iq_dis                       : 64;
#endif
	} s;
	struct cvmx_sso_wq_iq_dis_s           cn68xx;
	struct cvmx_sso_wq_iq_dis_s           cn68xxp1;
};
typedef union cvmx_sso_wq_iq_dis cvmx_sso_wq_iq_dis_t;

/**
 * cvmx_sso_ws_cfg
 *
 * This register contains various SSO work-slot configuration bits.
 *
 */
union cvmx_sso_ws_cfg {
	uint64_t u64;
	struct cvmx_sso_ws_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_56_63               : 8;
	uint64_t ocla_bp                      : 8;  /**< Enable OCLA backpressure stalls. For diagnostic use only. */
	uint64_t reserved_7_47                : 41;
	uint64_t aw_clk_dis                   : 1;  /**< Reserved. */
	uint64_t gw_clk_dis                   : 1;  /**< Reserved. */
	uint64_t disable_pw                   : 1;  /**< Reserved. */
	uint64_t arbc_step_en                 : 1;  /**< Enable single-stepping WS CAM arbiter, twice per 16 clocks. For diagnostic use only. */
	uint64_t ncbo_step_en                 : 1;  /**< Enable single-stepping commands from NCBO, once per 32 clocks. For diagnostic use only. */
	uint64_t soc_ccam_dis                 : 1;  /**< Disable power saving SOC conditional CAM. */
	uint64_t sso_cclk_dis                 : 1;  /**< Disable power saving SSO conditional clocking, */
#else
	uint64_t sso_cclk_dis                 : 1;
	uint64_t soc_ccam_dis                 : 1;
	uint64_t ncbo_step_en                 : 1;
	uint64_t arbc_step_en                 : 1;
	uint64_t disable_pw                   : 1;
	uint64_t gw_clk_dis                   : 1;
	uint64_t aw_clk_dis                   : 1;
	uint64_t reserved_7_47                : 41;
	uint64_t ocla_bp                      : 8;
	uint64_t reserved_56_63               : 8;
#endif
	} s;
	struct cvmx_sso_ws_cfg_s              cn73xx;
	struct cvmx_sso_ws_cfg_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_56_63               : 8;
	uint64_t ocla_bp                      : 8;  /**< Enable OCLA backpressure stalls. For diagnostic use only. */
	uint64_t reserved_5_47                : 43;
	uint64_t disable_pw                   : 1;  /**< Reserved. */
	uint64_t arbc_step_en                 : 1;  /**< Enable single-stepping WS CAM arbiter, twice per 16 clocks. For diagnostic use only. */
	uint64_t ncbo_step_en                 : 1;  /**< Enable single-stepping commands from NCBO, once per 32 clocks. For diagnostic use only. */
	uint64_t soc_ccam_dis                 : 1;  /**< Disable power saving SOC conditional CAM. */
	uint64_t sso_cclk_dis                 : 1;  /**< Disable power saving SSO conditional clocking, */
#else
	uint64_t sso_cclk_dis                 : 1;
	uint64_t soc_ccam_dis                 : 1;
	uint64_t ncbo_step_en                 : 1;
	uint64_t arbc_step_en                 : 1;
	uint64_t disable_pw                   : 1;
	uint64_t reserved_5_47                : 43;
	uint64_t ocla_bp                      : 8;
	uint64_t reserved_56_63               : 8;
#endif
	} cn78xx;
	struct cvmx_sso_ws_cfg_cn78xx         cn78xxp1;
	struct cvmx_sso_ws_cfg_s              cnf75xx;
};
typedef union cvmx_sso_ws_cfg cvmx_sso_ws_cfg_t;

/**
 * cvmx_sso_ws_eco
 */
union cvmx_sso_ws_eco {
	uint64_t u64;
	struct cvmx_sso_ws_eco_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t eco_rw                       : 8;  /**< N/A */
#else
	uint64_t eco_rw                       : 8;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_sso_ws_eco_s              cn73xx;
	struct cvmx_sso_ws_eco_s              cnf75xx;
};
typedef union cvmx_sso_ws_eco cvmx_sso_ws_eco_t;

/**
 * cvmx_sso_ws_pc#
 *
 * CSR reserved addresses: (225): 0x3100..0x3800
 * CSR align addresses: ===========================================================================================================
 * SSO_WS_PCX = SSO Work Schedule Performance Counter
 *              (one per group)
 *
 * Counts the number of work schedules for each group.
 * Counter rolls over through zero when max value exceeded.
 */
union cvmx_sso_ws_pcx {
	uint64_t u64;
	struct cvmx_sso_ws_pcx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ws_pc                        : 64; /**< Work schedule performance counter for group X */
#else
	uint64_t ws_pc                        : 64;
#endif
	} s;
	struct cvmx_sso_ws_pcx_s              cn68xx;
	struct cvmx_sso_ws_pcx_s              cn68xxp1;
};
typedef union cvmx_sso_ws_pcx cvmx_sso_ws_pcx_t;

/**
 * cvmx_sso_xaq#_head_next
 *
 * These registers contain the pointer to the next buffer to become the head when the final cache
 * line in this buffer is read.
 */
union cvmx_sso_xaqx_head_next {
	uint64_t u64;
	struct cvmx_sso_xaqx_head_next_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_42_63               : 22;
	uint64_t ptr                          : 35; /**< Pointer, divided by 128 bytes. */
	uint64_t reserved_0_6                 : 7;
#else
	uint64_t reserved_0_6                 : 7;
	uint64_t ptr                          : 35;
	uint64_t reserved_42_63               : 22;
#endif
	} s;
	struct cvmx_sso_xaqx_head_next_s      cn73xx;
	struct cvmx_sso_xaqx_head_next_s      cn78xx;
	struct cvmx_sso_xaqx_head_next_s      cn78xxp1;
	struct cvmx_sso_xaqx_head_next_s      cnf75xx;
};
typedef union cvmx_sso_xaqx_head_next cvmx_sso_xaqx_head_next_t;

/**
 * cvmx_sso_xaq#_head_ptr
 *
 * These registers contain the pointer to the first entry of the external linked list(s) for a
 * particular group. Software must initialize the external linked list(s) by programming
 * SSO_XAQ()_HEAD_PTR, SSO_XAQ()_HEAD_NEXT, SSO_XAQ()_TAIL_PTR and
 * SSO_XAQ()_TAIL_NEXT to identical values.
 */
union cvmx_sso_xaqx_head_ptr {
	uint64_t u64;
	struct cvmx_sso_xaqx_head_ptr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_42_63               : 22;
	uint64_t ptr                          : 35; /**< Pointer, divided by 128 bytes. */
	uint64_t reserved_5_6                 : 2;
	uint64_t cl                           : 5;  /**< Cache line number in buffer. Cache line zero contains the next pointer. */
#else
	uint64_t cl                           : 5;
	uint64_t reserved_5_6                 : 2;
	uint64_t ptr                          : 35;
	uint64_t reserved_42_63               : 22;
#endif
	} s;
	struct cvmx_sso_xaqx_head_ptr_s       cn73xx;
	struct cvmx_sso_xaqx_head_ptr_s       cn78xx;
	struct cvmx_sso_xaqx_head_ptr_s       cn78xxp1;
	struct cvmx_sso_xaqx_head_ptr_s       cnf75xx;
};
typedef union cvmx_sso_xaqx_head_ptr cvmx_sso_xaqx_head_ptr_t;

/**
 * cvmx_sso_xaq#_tail_next
 *
 * These registers contain the pointer to the next buffer to become the tail when the final cache
 * line in this buffer is written.  Register fields are identical to those in
 * SSO_XAQ()_HEAD_NEXT above.
 */
union cvmx_sso_xaqx_tail_next {
	uint64_t u64;
	struct cvmx_sso_xaqx_tail_next_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_42_63               : 22;
	uint64_t ptr                          : 35; /**< Pointer, divided by 128 bytes. */
	uint64_t reserved_0_6                 : 7;
#else
	uint64_t reserved_0_6                 : 7;
	uint64_t ptr                          : 35;
	uint64_t reserved_42_63               : 22;
#endif
	} s;
	struct cvmx_sso_xaqx_tail_next_s      cn73xx;
	struct cvmx_sso_xaqx_tail_next_s      cn78xx;
	struct cvmx_sso_xaqx_tail_next_s      cn78xxp1;
	struct cvmx_sso_xaqx_tail_next_s      cnf75xx;
};
typedef union cvmx_sso_xaqx_tail_next cvmx_sso_xaqx_tail_next_t;

/**
 * cvmx_sso_xaq#_tail_ptr
 *
 * These registers contain the pointer to the last entry of the external linked list(s) for a
 * particular group.  Register fields are identical to those in SSO_XAQ()_HEAD_PTR above.
 * Software must initialize the external linked list(s) by programming
 * SSO_XAQ()_HEAD_PTR, SSO_XAQ()_HEAD_NEXT, SSO_XAQ()_TAIL_PTR and
 * SSO_XAQ()_TAIL_NEXT to identical values.
 */
union cvmx_sso_xaqx_tail_ptr {
	uint64_t u64;
	struct cvmx_sso_xaqx_tail_ptr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_42_63               : 22;
	uint64_t ptr                          : 35; /**< Pointer, divided by 128 bytes. */
	uint64_t reserved_5_6                 : 2;
	uint64_t cl                           : 5;  /**< Cache line number in buffer. Cache line zero contains the next pointer. */
#else
	uint64_t cl                           : 5;
	uint64_t reserved_5_6                 : 2;
	uint64_t ptr                          : 35;
	uint64_t reserved_42_63               : 22;
#endif
	} s;
	struct cvmx_sso_xaqx_tail_ptr_s       cn73xx;
	struct cvmx_sso_xaqx_tail_ptr_s       cn78xx;
	struct cvmx_sso_xaqx_tail_ptr_s       cn78xxp1;
	struct cvmx_sso_xaqx_tail_ptr_s       cnf75xx;
};
typedef union cvmx_sso_xaqx_tail_ptr cvmx_sso_xaqx_tail_ptr_t;

/**
 * cvmx_sso_xaq_aura
 */
union cvmx_sso_xaq_aura {
	uint64_t u64;
	struct cvmx_sso_xaq_aura_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t node                         : 2;  /**< Reserved. */
	uint64_t laura                        : 10; /**< FPA local-node aura to use for SSO XAQ allocations and frees. The FPA aura
                                                         selected by [LAURA] must correspond to a pool where the buffers (after any
                                                         FPA_POOL()_CFG[BUF_OFFSET]) are at least 4 KB. */
#else
	uint64_t laura                        : 10;
	uint64_t node                         : 2;
	uint64_t reserved_12_63               : 52;
#endif
	} s;
	struct cvmx_sso_xaq_aura_s            cn73xx;
	struct cvmx_sso_xaq_aura_s            cn78xx;
	struct cvmx_sso_xaq_aura_s            cn78xxp1;
	struct cvmx_sso_xaq_aura_s            cnf75xx;
};
typedef union cvmx_sso_xaq_aura cvmx_sso_xaq_aura_t;

/**
 * cvmx_sso_xaq_latency_pc
 */
union cvmx_sso_xaq_latency_pc {
	uint64_t u64;
	struct cvmx_sso_xaq_latency_pc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t count                        : 64; /**< Number of cycles waiting for XAQ read returns. This may be divided by SSO_XAQ_REQ_PC to
                                                         determine the average read latency. */
#else
	uint64_t count                        : 64;
#endif
	} s;
	struct cvmx_sso_xaq_latency_pc_s      cn73xx;
	struct cvmx_sso_xaq_latency_pc_s      cn78xx;
	struct cvmx_sso_xaq_latency_pc_s      cn78xxp1;
	struct cvmx_sso_xaq_latency_pc_s      cnf75xx;
};
typedef union cvmx_sso_xaq_latency_pc cvmx_sso_xaq_latency_pc_t;

/**
 * cvmx_sso_xaq_req_pc
 */
union cvmx_sso_xaq_req_pc {
	uint64_t u64;
	struct cvmx_sso_xaq_req_pc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t count                        : 64; /**< Number of XAQ read requests. */
#else
	uint64_t count                        : 64;
#endif
	} s;
	struct cvmx_sso_xaq_req_pc_s          cn73xx;
	struct cvmx_sso_xaq_req_pc_s          cn78xx;
	struct cvmx_sso_xaq_req_pc_s          cn78xxp1;
	struct cvmx_sso_xaq_req_pc_s          cnf75xx;
};
typedef union cvmx_sso_xaq_req_pc cvmx_sso_xaq_req_pc_t;

#endif
