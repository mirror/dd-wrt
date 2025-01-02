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
 * cvmx-ocx-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon ocx.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_OCX_DEFS_H__
#define __CVMX_OCX_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_OCX_COM_BIST_STATUS CVMX_OCX_COM_BIST_STATUS_FUNC()
static inline uint64_t CVMX_OCX_COM_BIST_STATUS_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_OCX_COM_BIST_STATUS not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800110000F0ull);
}
#else
#define CVMX_OCX_COM_BIST_STATUS (CVMX_ADD_IO_SEG(0x00011800110000F0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_OCX_COM_DUAL_SORT CVMX_OCX_COM_DUAL_SORT_FUNC()
static inline uint64_t CVMX_OCX_COM_DUAL_SORT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_OCX_COM_DUAL_SORT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180011000008ull);
}
#else
#define CVMX_OCX_COM_DUAL_SORT (CVMX_ADD_IO_SEG(0x0001180011000008ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_OCX_COM_INT CVMX_OCX_COM_INT_FUNC()
static inline uint64_t CVMX_OCX_COM_INT_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_OCX_COM_INT not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180011000100ull);
}
#else
#define CVMX_OCX_COM_INT (CVMX_ADD_IO_SEG(0x0001180011000100ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_COM_LINKX_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 2)))))
		cvmx_warn("CVMX_OCX_COM_LINKX_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011000020ull) + ((offset) & 3) * 8;
}
#else
#define CVMX_OCX_COM_LINKX_CTL(offset) (CVMX_ADD_IO_SEG(0x0001180011000020ull) + ((offset) & 3) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_COM_LINKX_INT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 2)))))
		cvmx_warn("CVMX_OCX_COM_LINKX_INT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011000120ull) + ((offset) & 3) * 8;
}
#else
#define CVMX_OCX_COM_LINKX_INT(offset) (CVMX_ADD_IO_SEG(0x0001180011000120ull) + ((offset) & 3) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_OCX_COM_LINK_TIMER CVMX_OCX_COM_LINK_TIMER_FUNC()
static inline uint64_t CVMX_OCX_COM_LINK_TIMER_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_OCX_COM_LINK_TIMER not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180011000010ull);
}
#else
#define CVMX_OCX_COM_LINK_TIMER (CVMX_ADD_IO_SEG(0x0001180011000010ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_OCX_COM_NODE CVMX_OCX_COM_NODE_FUNC()
static inline uint64_t CVMX_OCX_COM_NODE_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_OCX_COM_NODE not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180011000000ull);
}
#else
#define CVMX_OCX_COM_NODE (CVMX_ADD_IO_SEG(0x0001180011000000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_DLLX_STATUS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1)))))
		cvmx_warn("CVMX_OCX_DLLX_STATUS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011000080ull) + ((offset) & 1) * 8;
}
#else
#define CVMX_OCX_DLLX_STATUS(offset) (CVMX_ADD_IO_SEG(0x0001180011000080ull) + ((offset) & 1) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_FRCX_STAT0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 5))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 5)))))
		cvmx_warn("CVMX_OCX_FRCX_STAT0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118001100FA00ull) + ((offset) & 7) * 8;
}
#else
#define CVMX_OCX_FRCX_STAT0(offset) (CVMX_ADD_IO_SEG(0x000118001100FA00ull) + ((offset) & 7) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_FRCX_STAT1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 5))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 5)))))
		cvmx_warn("CVMX_OCX_FRCX_STAT1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118001100FA80ull) + ((offset) & 7) * 8;
}
#else
#define CVMX_OCX_FRCX_STAT1(offset) (CVMX_ADD_IO_SEG(0x000118001100FA80ull) + ((offset) & 7) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_FRCX_STAT2(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 5))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 5)))))
		cvmx_warn("CVMX_OCX_FRCX_STAT2(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118001100FB00ull) + ((offset) & 7) * 8;
}
#else
#define CVMX_OCX_FRCX_STAT2(offset) (CVMX_ADD_IO_SEG(0x000118001100FB00ull) + ((offset) & 7) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_FRCX_STAT3(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 5))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 5)))))
		cvmx_warn("CVMX_OCX_FRCX_STAT3(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118001100FB80ull) + ((offset) & 7) * 8;
}
#else
#define CVMX_OCX_FRCX_STAT3(offset) (CVMX_ADD_IO_SEG(0x000118001100FB80ull) + ((offset) & 7) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_LNEX_BAD_CNT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 23))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 23)))))
		cvmx_warn("CVMX_OCX_LNEX_BAD_CNT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011008028ull) + ((offset) & 31) * 256;
}
#else
#define CVMX_OCX_LNEX_BAD_CNT(offset) (CVMX_ADD_IO_SEG(0x0001180011008028ull) + ((offset) & 31) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_LNEX_CFG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 23))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 23)))))
		cvmx_warn("CVMX_OCX_LNEX_CFG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011008000ull) + ((offset) & 31) * 256;
}
#else
#define CVMX_OCX_LNEX_CFG(offset) (CVMX_ADD_IO_SEG(0x0001180011008000ull) + ((offset) & 31) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_LNEX_INT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 23))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 23)))))
		cvmx_warn("CVMX_OCX_LNEX_INT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011008018ull) + ((offset) & 31) * 256;
}
#else
#define CVMX_OCX_LNEX_INT(offset) (CVMX_ADD_IO_SEG(0x0001180011008018ull) + ((offset) & 31) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_LNEX_INT_EN(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 23))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 23)))))
		cvmx_warn("CVMX_OCX_LNEX_INT_EN(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011008020ull) + ((offset) & 31) * 256;
}
#else
#define CVMX_OCX_LNEX_INT_EN(offset) (CVMX_ADD_IO_SEG(0x0001180011008020ull) + ((offset) & 31) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_LNEX_STAT00(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 23))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 23)))))
		cvmx_warn("CVMX_OCX_LNEX_STAT00(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011008040ull) + ((offset) & 31) * 256;
}
#else
#define CVMX_OCX_LNEX_STAT00(offset) (CVMX_ADD_IO_SEG(0x0001180011008040ull) + ((offset) & 31) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_LNEX_STAT01(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 23))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 23)))))
		cvmx_warn("CVMX_OCX_LNEX_STAT01(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011008048ull) + ((offset) & 31) * 256;
}
#else
#define CVMX_OCX_LNEX_STAT01(offset) (CVMX_ADD_IO_SEG(0x0001180011008048ull) + ((offset) & 31) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_LNEX_STAT02(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 23))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 23)))))
		cvmx_warn("CVMX_OCX_LNEX_STAT02(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011008050ull) + ((offset) & 31) * 256;
}
#else
#define CVMX_OCX_LNEX_STAT02(offset) (CVMX_ADD_IO_SEG(0x0001180011008050ull) + ((offset) & 31) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_LNEX_STAT03(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 23))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 23)))))
		cvmx_warn("CVMX_OCX_LNEX_STAT03(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011008058ull) + ((offset) & 31) * 256;
}
#else
#define CVMX_OCX_LNEX_STAT03(offset) (CVMX_ADD_IO_SEG(0x0001180011008058ull) + ((offset) & 31) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_LNEX_STAT04(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 23))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 23)))))
		cvmx_warn("CVMX_OCX_LNEX_STAT04(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011008060ull) + ((offset) & 31) * 256;
}
#else
#define CVMX_OCX_LNEX_STAT04(offset) (CVMX_ADD_IO_SEG(0x0001180011008060ull) + ((offset) & 31) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_LNEX_STAT05(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 23))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 23)))))
		cvmx_warn("CVMX_OCX_LNEX_STAT05(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011008068ull) + ((offset) & 31) * 256;
}
#else
#define CVMX_OCX_LNEX_STAT05(offset) (CVMX_ADD_IO_SEG(0x0001180011008068ull) + ((offset) & 31) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_LNEX_STAT06(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 23))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 23)))))
		cvmx_warn("CVMX_OCX_LNEX_STAT06(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011008070ull) + ((offset) & 31) * 256;
}
#else
#define CVMX_OCX_LNEX_STAT06(offset) (CVMX_ADD_IO_SEG(0x0001180011008070ull) + ((offset) & 31) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_LNEX_STAT07(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 23))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 23)))))
		cvmx_warn("CVMX_OCX_LNEX_STAT07(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011008078ull) + ((offset) & 31) * 256;
}
#else
#define CVMX_OCX_LNEX_STAT07(offset) (CVMX_ADD_IO_SEG(0x0001180011008078ull) + ((offset) & 31) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_LNEX_STAT08(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 23))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 23)))))
		cvmx_warn("CVMX_OCX_LNEX_STAT08(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011008080ull) + ((offset) & 31) * 256;
}
#else
#define CVMX_OCX_LNEX_STAT08(offset) (CVMX_ADD_IO_SEG(0x0001180011008080ull) + ((offset) & 31) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_LNEX_STAT09(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 23))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 23)))))
		cvmx_warn("CVMX_OCX_LNEX_STAT09(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011008088ull) + ((offset) & 31) * 256;
}
#else
#define CVMX_OCX_LNEX_STAT09(offset) (CVMX_ADD_IO_SEG(0x0001180011008088ull) + ((offset) & 31) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_LNEX_STAT10(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 23))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 23)))))
		cvmx_warn("CVMX_OCX_LNEX_STAT10(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011008090ull) + ((offset) & 31) * 256;
}
#else
#define CVMX_OCX_LNEX_STAT10(offset) (CVMX_ADD_IO_SEG(0x0001180011008090ull) + ((offset) & 31) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_LNEX_STAT11(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 23))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 23)))))
		cvmx_warn("CVMX_OCX_LNEX_STAT11(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011008098ull) + ((offset) & 31) * 256;
}
#else
#define CVMX_OCX_LNEX_STAT11(offset) (CVMX_ADD_IO_SEG(0x0001180011008098ull) + ((offset) & 31) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_LNEX_STAT12(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 23))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 23)))))
		cvmx_warn("CVMX_OCX_LNEX_STAT12(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800110080A0ull) + ((offset) & 31) * 256;
}
#else
#define CVMX_OCX_LNEX_STAT12(offset) (CVMX_ADD_IO_SEG(0x00011800110080A0ull) + ((offset) & 31) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_LNEX_STAT13(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 23))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 23)))))
		cvmx_warn("CVMX_OCX_LNEX_STAT13(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800110080A8ull) + ((offset) & 31) * 256;
}
#else
#define CVMX_OCX_LNEX_STAT13(offset) (CVMX_ADD_IO_SEG(0x00011800110080A8ull) + ((offset) & 31) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_LNEX_STAT14(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 23))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 23)))))
		cvmx_warn("CVMX_OCX_LNEX_STAT14(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800110080B0ull) + ((offset) & 31) * 256;
}
#else
#define CVMX_OCX_LNEX_STAT14(offset) (CVMX_ADD_IO_SEG(0x00011800110080B0ull) + ((offset) & 31) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_LNEX_STATUS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 23))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 23)))))
		cvmx_warn("CVMX_OCX_LNEX_STATUS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011008008ull) + ((offset) & 31) * 256;
}
#else
#define CVMX_OCX_LNEX_STATUS(offset) (CVMX_ADD_IO_SEG(0x0001180011008008ull) + ((offset) & 31) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_LNEX_STS_MSG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 23))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 23)))))
		cvmx_warn("CVMX_OCX_LNEX_STS_MSG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011008010ull) + ((offset) & 31) * 256;
}
#else
#define CVMX_OCX_LNEX_STS_MSG(offset) (CVMX_ADD_IO_SEG(0x0001180011008010ull) + ((offset) & 31) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_LNEX_TRN_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 23))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 23)))))
		cvmx_warn("CVMX_OCX_LNEX_TRN_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800110080D0ull) + ((offset) & 31) * 256;
}
#else
#define CVMX_OCX_LNEX_TRN_CTL(offset) (CVMX_ADD_IO_SEG(0x00011800110080D0ull) + ((offset) & 31) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_LNEX_TRN_LD(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 23))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 23)))))
		cvmx_warn("CVMX_OCX_LNEX_TRN_LD(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800110080C0ull) + ((offset) & 31) * 256;
}
#else
#define CVMX_OCX_LNEX_TRN_LD(offset) (CVMX_ADD_IO_SEG(0x00011800110080C0ull) + ((offset) & 31) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_LNEX_TRN_LP(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 23))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 23)))))
		cvmx_warn("CVMX_OCX_LNEX_TRN_LP(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800110080C8ull) + ((offset) & 31) * 256;
}
#else
#define CVMX_OCX_LNEX_TRN_LP(offset) (CVMX_ADD_IO_SEG(0x00011800110080C8ull) + ((offset) & 31) * 256)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_OCX_LNE_DBG CVMX_OCX_LNE_DBG_FUNC()
static inline uint64_t CVMX_OCX_LNE_DBG_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_OCX_LNE_DBG not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x000118001100FF00ull);
}
#else
#define CVMX_OCX_LNE_DBG (CVMX_ADD_IO_SEG(0x000118001100FF00ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_LNKX_CFG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 2)))))
		cvmx_warn("CVMX_OCX_LNKX_CFG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118001100F900ull) + ((offset) & 3) * 8;
}
#else
#define CVMX_OCX_LNKX_CFG(offset) (CVMX_ADD_IO_SEG(0x000118001100F900ull) + ((offset) & 3) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_OCX_PP_CMD CVMX_OCX_PP_CMD_FUNC()
static inline uint64_t CVMX_OCX_PP_CMD_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_OCX_PP_CMD not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800110000C8ull);
}
#else
#define CVMX_OCX_PP_CMD (CVMX_ADD_IO_SEG(0x00011800110000C8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_OCX_PP_RD_DATA CVMX_OCX_PP_RD_DATA_FUNC()
static inline uint64_t CVMX_OCX_PP_RD_DATA_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_OCX_PP_RD_DATA not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800110000D0ull);
}
#else
#define CVMX_OCX_PP_RD_DATA (CVMX_ADD_IO_SEG(0x00011800110000D0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_OCX_PP_WR_DATA CVMX_OCX_PP_WR_DATA_FUNC()
static inline uint64_t CVMX_OCX_PP_WR_DATA_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_OCX_PP_WR_DATA not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x00011800110000C0ull);
}
#else
#define CVMX_OCX_PP_WR_DATA (CVMX_ADD_IO_SEG(0x00011800110000C0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_QLMX_CFG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 5))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 5)))))
		cvmx_warn("CVMX_OCX_QLMX_CFG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118001100F800ull) + ((offset) & 7) * 8;
}
#else
#define CVMX_OCX_QLMX_CFG(offset) (CVMX_ADD_IO_SEG(0x000118001100F800ull) + ((offset) & 7) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_RLKX_ALIGN(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 2)))))
		cvmx_warn("CVMX_OCX_RLKX_ALIGN(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011018060ull) + ((offset) & 3) * 8192;
}
#else
#define CVMX_OCX_RLKX_ALIGN(offset) (CVMX_ADD_IO_SEG(0x0001180011018060ull) + ((offset) & 3) * 8192)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_RLKX_BLK_ERR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 2)))))
		cvmx_warn("CVMX_OCX_RLKX_BLK_ERR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011018050ull) + ((offset) & 3) * 8192;
}
#else
#define CVMX_OCX_RLKX_BLK_ERR(offset) (CVMX_ADD_IO_SEG(0x0001180011018050ull) + ((offset) & 3) * 8192)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_RLKX_ECC_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 2)))))
		cvmx_warn("CVMX_OCX_RLKX_ECC_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011018018ull) + ((offset) & 3) * 8192;
}
#else
#define CVMX_OCX_RLKX_ECC_CTL(offset) (CVMX_ADD_IO_SEG(0x0001180011018018ull) + ((offset) & 3) * 8192)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_RLKX_ENABLES(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 2)))))
		cvmx_warn("CVMX_OCX_RLKX_ENABLES(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011018000ull) + ((offset) & 3) * 8192;
}
#else
#define CVMX_OCX_RLKX_ENABLES(offset) (CVMX_ADD_IO_SEG(0x0001180011018000ull) + ((offset) & 3) * 8192)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_RLKX_FIFOX_CNT(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 13)) && ((block_id <= 2)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 13)) && ((block_id <= 2))))))
		cvmx_warn("CVMX_OCX_RLKX_FIFOX_CNT(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180011018100ull) + (((offset) & 15) + ((block_id) & 3) * 0x400ull) * 8;
}
#else
#define CVMX_OCX_RLKX_FIFOX_CNT(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180011018100ull) + (((offset) & 15) + ((block_id) & 3) * 0x400ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_RLKX_LNK_DATA(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 2)))))
		cvmx_warn("CVMX_OCX_RLKX_LNK_DATA(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011018028ull) + ((offset) & 3) * 8192;
}
#else
#define CVMX_OCX_RLKX_LNK_DATA(offset) (CVMX_ADD_IO_SEG(0x0001180011018028ull) + ((offset) & 3) * 8192)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_RLKX_MCD_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 2)))))
		cvmx_warn("CVMX_OCX_RLKX_MCD_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011018020ull) + ((offset) & 3) * 8192;
}
#else
#define CVMX_OCX_RLKX_MCD_CTL(offset) (CVMX_ADD_IO_SEG(0x0001180011018020ull) + ((offset) & 3) * 8192)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_OCX_STRAP CVMX_OCX_STRAP_FUNC()
static inline uint64_t CVMX_OCX_STRAP_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX)))
		cvmx_warn("CVMX_OCX_STRAP not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x000118001100FF08ull);
}
#else
#define CVMX_OCX_STRAP (CVMX_ADD_IO_SEG(0x000118001100FF08ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_TLKX_BIST_STATUS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 2)))))
		cvmx_warn("CVMX_OCX_TLKX_BIST_STATUS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011010008ull) + ((offset) & 3) * 8192;
}
#else
#define CVMX_OCX_TLKX_BIST_STATUS(offset) (CVMX_ADD_IO_SEG(0x0001180011010008ull) + ((offset) & 3) * 8192)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_TLKX_BYP_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 2)))))
		cvmx_warn("CVMX_OCX_TLKX_BYP_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011010030ull) + ((offset) & 3) * 8192;
}
#else
#define CVMX_OCX_TLKX_BYP_CTL(offset) (CVMX_ADD_IO_SEG(0x0001180011010030ull) + ((offset) & 3) * 8192)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_TLKX_ECC_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 2)))))
		cvmx_warn("CVMX_OCX_TLKX_ECC_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011010018ull) + ((offset) & 3) * 8192;
}
#else
#define CVMX_OCX_TLKX_ECC_CTL(offset) (CVMX_ADD_IO_SEG(0x0001180011010018ull) + ((offset) & 3) * 8192)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_TLKX_FIFOX_CNT(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 13)) && ((block_id <= 2)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 13)) && ((block_id <= 2))))))
		cvmx_warn("CVMX_OCX_TLKX_FIFOX_CNT(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180011010100ull) + (((offset) & 15) + ((block_id) & 3) * 0x400ull) * 8;
}
#else
#define CVMX_OCX_TLKX_FIFOX_CNT(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180011010100ull) + (((offset) & 15) + ((block_id) & 3) * 0x400ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_TLKX_LNK_DATA(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 2)))))
		cvmx_warn("CVMX_OCX_TLKX_LNK_DATA(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011010028ull) + ((offset) & 3) * 8192;
}
#else
#define CVMX_OCX_TLKX_LNK_DATA(offset) (CVMX_ADD_IO_SEG(0x0001180011010028ull) + ((offset) & 3) * 8192)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_TLKX_LNK_VCX_CNT(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 13)) && ((block_id <= 2)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 13)) && ((block_id <= 2))))))
		cvmx_warn("CVMX_OCX_TLKX_LNK_VCX_CNT(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180011010200ull) + (((offset) & 15) + ((block_id) & 3) * 0x400ull) * 8;
}
#else
#define CVMX_OCX_TLKX_LNK_VCX_CNT(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180011010200ull) + (((offset) & 15) + ((block_id) & 3) * 0x400ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_TLKX_MCD_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 2)))))
		cvmx_warn("CVMX_OCX_TLKX_MCD_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011010020ull) + ((offset) & 3) * 8192;
}
#else
#define CVMX_OCX_TLKX_MCD_CTL(offset) (CVMX_ADD_IO_SEG(0x0001180011010020ull) + ((offset) & 3) * 8192)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_TLKX_RTN_VCX_CNT(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 13)) && ((block_id <= 2)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 13)) && ((block_id <= 2))))))
		cvmx_warn("CVMX_OCX_TLKX_RTN_VCX_CNT(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180011010300ull) + (((offset) & 15) + ((block_id) & 3) * 0x400ull) * 8;
}
#else
#define CVMX_OCX_TLKX_RTN_VCX_CNT(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180011010300ull) + (((offset) & 15) + ((block_id) & 3) * 0x400ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_TLKX_STATUS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 2)))))
		cvmx_warn("CVMX_OCX_TLKX_STATUS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011010000ull) + ((offset) & 3) * 8192;
}
#else
#define CVMX_OCX_TLKX_STATUS(offset) (CVMX_ADD_IO_SEG(0x0001180011010000ull) + ((offset) & 3) * 8192)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_TLKX_STAT_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 2)))))
		cvmx_warn("CVMX_OCX_TLKX_STAT_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011010040ull) + ((offset) & 3) * 8192;
}
#else
#define CVMX_OCX_TLKX_STAT_CTL(offset) (CVMX_ADD_IO_SEG(0x0001180011010040ull) + ((offset) & 3) * 8192)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_TLKX_STAT_DATA_CNT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 2)))))
		cvmx_warn("CVMX_OCX_TLKX_STAT_DATA_CNT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011010408ull) + ((offset) & 3) * 8192;
}
#else
#define CVMX_OCX_TLKX_STAT_DATA_CNT(offset) (CVMX_ADD_IO_SEG(0x0001180011010408ull) + ((offset) & 3) * 8192)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_TLKX_STAT_ERR_CNT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 2)))))
		cvmx_warn("CVMX_OCX_TLKX_STAT_ERR_CNT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011010420ull) + ((offset) & 3) * 8192;
}
#else
#define CVMX_OCX_TLKX_STAT_ERR_CNT(offset) (CVMX_ADD_IO_SEG(0x0001180011010420ull) + ((offset) & 3) * 8192)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_TLKX_STAT_IDLE_CNT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 2)))))
		cvmx_warn("CVMX_OCX_TLKX_STAT_IDLE_CNT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011010400ull) + ((offset) & 3) * 8192;
}
#else
#define CVMX_OCX_TLKX_STAT_IDLE_CNT(offset) (CVMX_ADD_IO_SEG(0x0001180011010400ull) + ((offset) & 3) * 8192)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_TLKX_STAT_MATCHX(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 2)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 3)) && ((block_id <= 2))))))
		cvmx_warn("CVMX_OCX_TLKX_STAT_MATCHX(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180011010080ull) + (((offset) & 3) + ((block_id) & 3) * 0x400ull) * 8;
}
#else
#define CVMX_OCX_TLKX_STAT_MATCHX(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180011010080ull) + (((offset) & 3) + ((block_id) & 3) * 0x400ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_TLKX_STAT_MATX_CNT(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 3)) && ((block_id <= 2)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 3)) && ((block_id <= 2))))))
		cvmx_warn("CVMX_OCX_TLKX_STAT_MATX_CNT(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180011010440ull) + (((offset) & 3) + ((block_id) & 3) * 0x400ull) * 8;
}
#else
#define CVMX_OCX_TLKX_STAT_MATX_CNT(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180011010440ull) + (((offset) & 3) + ((block_id) & 3) * 0x400ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_TLKX_STAT_RETRY_CNT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 2)))))
		cvmx_warn("CVMX_OCX_TLKX_STAT_RETRY_CNT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011010418ull) + ((offset) & 3) * 8192;
}
#else
#define CVMX_OCX_TLKX_STAT_RETRY_CNT(offset) (CVMX_ADD_IO_SEG(0x0001180011010418ull) + ((offset) & 3) * 8192)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_TLKX_STAT_SYNC_CNT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 2))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 2)))))
		cvmx_warn("CVMX_OCX_TLKX_STAT_SYNC_CNT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180011010410ull) + ((offset) & 3) * 8192;
}
#else
#define CVMX_OCX_TLKX_STAT_SYNC_CNT(offset) (CVMX_ADD_IO_SEG(0x0001180011010410ull) + ((offset) & 3) * 8192)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_TLKX_STAT_VCX_CMD(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 5)) && ((block_id <= 2)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 5)) && ((block_id <= 2))))))
		cvmx_warn("CVMX_OCX_TLKX_STAT_VCX_CMD(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180011010480ull) + (((offset) & 7) + ((block_id) & 3) * 0x400ull) * 8;
}
#else
#define CVMX_OCX_TLKX_STAT_VCX_CMD(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180011010480ull) + (((offset) & 7) + ((block_id) & 3) * 0x400ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_TLKX_STAT_VCX_CON(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 13)) && ((block_id <= 2)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 13)) && ((block_id <= 2))))))
		cvmx_warn("CVMX_OCX_TLKX_STAT_VCX_CON(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180011010580ull) + (((offset) & 15) + ((block_id) & 3) * 0x400ull) * 8;
}
#else
#define CVMX_OCX_TLKX_STAT_VCX_CON(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180011010580ull) + (((offset) & 15) + ((block_id) & 3) * 0x400ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_OCX_TLKX_STAT_VCX_PKT(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset <= 13)) && ((block_id <= 2)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset <= 13)) && ((block_id <= 2))))))
		cvmx_warn("CVMX_OCX_TLKX_STAT_VCX_PKT(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180011010500ull) + (((offset) & 15) + ((block_id) & 3) * 0x400ull) * 8;
}
#else
#define CVMX_OCX_TLKX_STAT_VCX_PKT(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180011010500ull) + (((offset) & 15) + ((block_id) & 3) * 0x400ull) * 8)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_OCX_WIN_CMD CVMX_OCX_WIN_CMD_FUNC()
static inline uint64_t CVMX_OCX_WIN_CMD_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_OCX_WIN_CMD not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180011000048ull);
}
#else
#define CVMX_OCX_WIN_CMD (CVMX_ADD_IO_SEG(0x0001180011000048ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_OCX_WIN_RD_DATA CVMX_OCX_WIN_RD_DATA_FUNC()
static inline uint64_t CVMX_OCX_WIN_RD_DATA_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_OCX_WIN_RD_DATA not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180011000050ull);
}
#else
#define CVMX_OCX_WIN_RD_DATA (CVMX_ADD_IO_SEG(0x0001180011000050ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_OCX_WIN_TIMER CVMX_OCX_WIN_TIMER_FUNC()
static inline uint64_t CVMX_OCX_WIN_TIMER_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_OCX_WIN_TIMER not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180011000058ull);
}
#else
#define CVMX_OCX_WIN_TIMER (CVMX_ADD_IO_SEG(0x0001180011000058ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
#define CVMX_OCX_WIN_WR_DATA CVMX_OCX_WIN_WR_DATA_FUNC()
static inline uint64_t CVMX_OCX_WIN_WR_DATA_FUNC(void)
{
	if (!(OCTEON_IS_MODEL(OCTEON_CN78XX) || OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)))
		cvmx_warn("CVMX_OCX_WIN_WR_DATA not supported on this chip\n");
	return CVMX_ADD_IO_SEG(0x0001180011000040ull);
}
#else
#define CVMX_OCX_WIN_WR_DATA (CVMX_ADD_IO_SEG(0x0001180011000040ull))
#endif

/**
 * cvmx_ocx_com_bist_status
 *
 * Contains status from last memory BIST for all RX FIFO memories. BIST status for TX FIFO
 * memories and REPLAY Memories are organized by link and are located in
 * OCX_TLK(0..2)_BIST_STATUS.
 */
union cvmx_ocx_com_bist_status {
	uint64_t u64;
	struct cvmx_ocx_com_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t status                       : 36; /**< BIST status. */
#else
	uint64_t status                       : 36;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_ocx_com_bist_status_s     cn78xx;
	struct cvmx_ocx_com_bist_status_s     cn78xxp1;
};
typedef union cvmx_ocx_com_bist_status cvmx_ocx_com_bist_status_t;

/**
 * cvmx_ocx_com_dual_sort
 */
union cvmx_ocx_com_dual_sort {
	uint64_t u64;
	struct cvmx_ocx_com_dual_sort_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t sort                         : 2;  /**< Sorting procedure for multiple links to same node:
                                                         0x0 = All to lowest link number.
                                                         0x1 = Split by top/bottom L2C buses. (top to lowest link number).
                                                         0x2 = IOC 1st, IOR 2nd, Mem VCs to either based on most room in TX FIFOs.
                                                         0x3 = Illegal. */
#else
	uint64_t sort                         : 2;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_ocx_com_dual_sort_s       cn78xx;
	struct cvmx_ocx_com_dual_sort_s       cn78xxp1;
};
typedef union cvmx_ocx_com_dual_sort cvmx_ocx_com_dual_sort_t;

/**
 * cvmx_ocx_com_int
 */
union cvmx_ocx_com_int {
	uint64_t u64;
	struct cvmx_ocx_com_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_55_63               : 9;
	uint64_t io_badid                     : 1;  /**< I/O request or response cannot be sent because a link was not found with a packet node ID
                                                         matching the OCX_COM_LINK(0..2)_CTL[ID] with OCX_COM_LINK(0..2)_CTL[VALID] bit set.
                                                         Transaction has been dropped.  Should not occur during normal operation. This may indicate
                                                         a software/configuration failure and may be considered fatal. */
	uint64_t mem_badid                    : 1;  /**< Memory request or response cannot be sent because a link was not found with a packet node
                                                         ID matching the OCX_COM_LINK(0..2)_CTL[ID] with OCX_COM_LINK(0..2)_CTL[VALID] bit set.
                                                         Transaction has been dropped.  Should not occur during normal operation. This may indicate
                                                         a software/configuration failure and may be considered fatal. */
	uint64_t copr_badid                   : 1;  /**< Scheduler add work or buffer pool return cannot be sent because a link was not found with
                                                         a node ID matching the OCX_COM_LINK(0..2)_CTL[ID] with OCX_COM_LINK(0..2)_CTL[VALID] bit
                                                         set.
                                                         Transaction has been dropped.  Should not occur during normal operation. This may indicate
                                                         a software/configuration failure and may be considered fatal. */
	uint64_t win_req_badid                : 1;  /**< Window request specified in SLI_WIN_RD_ADDR, SLI_WIN_WR_ADDR, OCX_WIN_CMD or OCX_PP_CMD
                                                         cannot be sent because a link was not found with a request node ID matching the
                                                         OCX_COM_LINK(0..2)_CTL[ID] with OCX_COM_LINK(0..2)_CTL[VALID] bit set.  Transaction has
                                                         been
                                                         dropped.  Should not occur during normal operation. This may indicate a
                                                         software/configuration failure and may be considered fatal. */
	uint64_t win_req_tout                 : 1;  /**< Window or core request was dropped because it could not be send during the period
                                                         specified by OCX_WIN_TIMER.  Should not occur during normal operation. This may indicate a
                                                         software/configuration failure and may be considered fatal. */
	uint64_t win_req_xmit                 : 1;  /**< Window request specified in SLI_WIN_RD_ADDR, SLI_WIN_WR_ADDR, OCX_WIN_CMD or OCX_PP_CMD
                                                         has been scheduled for transmission. If the command was not expecting a response, then a
                                                         new command may be issued. */
	uint64_t win_rsp                      : 1;  /**< A response to a previous window request or core request has been received. A new command
                                                         may be issued. */
	uint64_t reserved_24_47               : 24;
	uint64_t rx_lane                      : 24; /**< SerDes RX lane interrupt. See OCX_LNE(0..23)_INT for more information. */
#else
	uint64_t rx_lane                      : 24;
	uint64_t reserved_24_47               : 24;
	uint64_t win_rsp                      : 1;
	uint64_t win_req_xmit                 : 1;
	uint64_t win_req_tout                 : 1;
	uint64_t win_req_badid                : 1;
	uint64_t copr_badid                   : 1;
	uint64_t mem_badid                    : 1;
	uint64_t io_badid                     : 1;
	uint64_t reserved_55_63               : 9;
#endif
	} s;
	struct cvmx_ocx_com_int_s             cn78xx;
	struct cvmx_ocx_com_int_s             cn78xxp1;
};
typedef union cvmx_ocx_com_int cvmx_ocx_com_int_t;

/**
 * cvmx_ocx_com_link#_ctl
 *
 * This register controls link operations.  In addition, the combination of some of
 * these conditions are used to generate the link_down status used by the L2C_OCI_CTL[SHTOEN] and
 * as a reset condition controlled by RST_OCX[RST_LINK].  This link_down status is true when one
 * of the following occurs:
 *
 * * Link is not initialized (see [UP]).
 * * Retry Counter expired (see OCX_COM_LINK_TIMER and OCX_COM_LINK(0..2)_INT[STOP].
 * * Receive REINIT request from Link Partner (see [REINIT]).
 * * Detected Uncorrectable ECC error while reading the Transmit FIFOs (see
 * OCX_COM_LINK(0..2)_INT[TXFIFO_DBE]).
 * * Detected Uncorrectable ECC error while reading the Replay Buffer (see
 * OCX_COM_LINK(0..2)_INT[REPLAY_DBE]).
 */
union cvmx_ocx_com_linkx_ctl {
	uint64_t u64;
	struct cvmx_ocx_com_linkx_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t cclk_dis                     : 1;  /**< Reserved. */
	uint64_t loopback                     : 1;  /**< Reserved. */
	uint64_t reinit                       : 1;  /**< Reinitialize link. Setting this bit forces link back into init state and sets the DROP
                                                         bit.
                                                         Setting the bit also causes the link to transmit a REINIT request to the link partner.
                                                         This bit must be cleared for link to operate normally. */
	uint64_t gate                         : 1;  /**< Reserved. */
	uint64_t auto_clr                     : 1;  /**< When set, automatically clears the local DROP bit if link partner forces
                                                         a reinitialization.  Typically disabled once software is running.
                                                         If clear, software must manage clearing [DROP] after it has verified
                                                         that any pending transactions have timed out. */
	uint64_t drop                         : 1;  /**< Drop all requests on given link. Typically set by hardware when link has failed or been
                                                         reinitialized. Cleared by software once pending link traffic is removed. (See
                                                         OCX_TLK(0..2)_FIFO(0..13)_CNT.) */
	uint64_t up                           : 1;  /**< Link is operating normally and exchanging control information. */
	uint64_t valid                        : 1;  /**< Link has valid lanes and is exchanging information. This bit will never be set if
                                                         OCX_LNK(0..2)_CFG[QLM_SELECT] is zero. */
	uint64_t id                           : 2;  /**< This ID is used to sort traffic by link. If more than one link has the same value, the
                                                         OCX_COM_DUAL_SORT[SORT] field and traffic VC are used to choose a link. This field is only
                                                         reset during a cold reset to an arbitrary value to avoid conflicts with the
                                                         OCX_COM_NODE[ID] field and should be configured by software before memory traffic is
                                                         generated. */
#else
	uint64_t id                           : 2;
	uint64_t valid                        : 1;
	uint64_t up                           : 1;
	uint64_t drop                         : 1;
	uint64_t auto_clr                     : 1;
	uint64_t gate                         : 1;
	uint64_t reinit                       : 1;
	uint64_t loopback                     : 1;
	uint64_t cclk_dis                     : 1;
	uint64_t reserved_10_63               : 54;
#endif
	} s;
	struct cvmx_ocx_com_linkx_ctl_s       cn78xx;
	struct cvmx_ocx_com_linkx_ctl_s       cn78xxp1;
};
typedef union cvmx_ocx_com_linkx_ctl cvmx_ocx_com_linkx_ctl_t;

/**
 * cvmx_ocx_com_link#_int
 */
union cvmx_ocx_com_linkx_int {
	uint64_t u64;
	struct cvmx_ocx_com_linkx_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_14_63               : 50;
	uint64_t bad_word                     : 1;  /**< Illegal word decoded on at least one lane of link. These receive errors may occur during
                                                         normal operation, and may likely occur during link bringup. Hardware normally will
                                                         automatically correct the error. Software may choose to count the number of these errors. */
	uint64_t align_fail                   : 1;  /**< Link lanes failed to align. These receive errors may occur during normal operation, and
                                                         may likely occur during link bringup. Hardware normally will automatically correct the
                                                         error. Software may choose to count the number of these errors. */
	uint64_t align_done                   : 1;  /**< Link lane alignment is complete. These may occur during normal operation, and will occur
                                                         during link bringup. Software should disable reception of these interrupts during normal
                                                         operation. */
	uint64_t up                           : 1;  /**< Link initialization is complete and is ready to pass traffic. Note this state occurs some
                                                         time after the link starts exchanging information as indicated in
                                                         OCX_COM_LINK(0..2)_CTL[UP]. These should not occur during normal operation. */
	uint64_t stop                         : 1;  /**< Link has stopped operating. Link retry count has reached threshold specified in
                                                         OCX_COM_LINK_TIMER; outgoing traffic has been dropped and an initialization request has
                                                         been reissued. These should not occur during normal operation. This may be considered
                                                         fatal. */
	uint64_t blk_err                      : 1;  /**< Link block error count has reached threshold specified in OCX_RLK(0..2)_BLK_ERR[LIMIT].
                                                         These receive errors may occur during normal operation. Hardware normally will
                                                         automatically correct the error. Software may choose to count the number of these errors. */
	uint64_t reinit                       : 1;  /**< Link has received a initialization request from link partner after link has been
                                                         established. These should not occur during normal operation */
	uint64_t lnk_data                     : 1;  /**< Set by hardware when a link data block is received in OCX_RLK(0..2)_LNK_DATA. It
                                                         software's responsibility to clear the bit after reading the data. */
	uint64_t rxfifo_dbe                   : 1;  /**< Double-bit error detected in FIFO RAMs. This error may be considered fatal. */
	uint64_t rxfifo_sbe                   : 1;  /**< Single-bit error detected/corrected in FIFO RAMs. Hardware automatically corrected the
                                                         error. Software may choose to count the number of these single-bit errors. */
	uint64_t txfifo_dbe                   : 1;  /**< Double-bit error detected in TX FIFO RAMs. This error may be considered fatal. */
	uint64_t txfifo_sbe                   : 1;  /**< Single-bit error detected/corrected in TX FIFO RAMs. Hardware automatically corrected the
                                                         error. Software may choose to count the number of these single-bit errors. */
	uint64_t replay_dbe                   : 1;  /**< Double-bit error detected in REPLAY BUFFER RAMs. This error may be considered fatal. */
	uint64_t replay_sbe                   : 1;  /**< Single-bit error detected/corrected in REPLAY BUFFER RAMs. Hardware automatically
                                                         corrected the error. Software may choose to count the number of these single-bit errors. */
#else
	uint64_t replay_sbe                   : 1;
	uint64_t replay_dbe                   : 1;
	uint64_t txfifo_sbe                   : 1;
	uint64_t txfifo_dbe                   : 1;
	uint64_t rxfifo_sbe                   : 1;
	uint64_t rxfifo_dbe                   : 1;
	uint64_t lnk_data                     : 1;
	uint64_t reinit                       : 1;
	uint64_t blk_err                      : 1;
	uint64_t stop                         : 1;
	uint64_t up                           : 1;
	uint64_t align_done                   : 1;
	uint64_t align_fail                   : 1;
	uint64_t bad_word                     : 1;
	uint64_t reserved_14_63               : 50;
#endif
	} s;
	struct cvmx_ocx_com_linkx_int_s       cn78xx;
	struct cvmx_ocx_com_linkx_int_s       cn78xxp1;
};
typedef union cvmx_ocx_com_linkx_int cvmx_ocx_com_linkx_int_t;

/**
 * cvmx_ocx_com_link_timer
 */
union cvmx_ocx_com_link_timer {
	uint64_t u64;
	struct cvmx_ocx_com_link_timer_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_24_63               : 40;
	uint64_t tout                         : 24; /**< Number of unacknowledged retry requests issued before link stops operation and
                                                         OCX_COM_LINK(0..2)_INT[STOP] is asserted. */
#else
	uint64_t tout                         : 24;
	uint64_t reserved_24_63               : 40;
#endif
	} s;
	struct cvmx_ocx_com_link_timer_s      cn78xx;
	struct cvmx_ocx_com_link_timer_s      cn78xxp1;
};
typedef union cvmx_ocx_com_link_timer cvmx_ocx_com_link_timer_t;

/**
 * cvmx_ocx_com_node
 */
union cvmx_ocx_com_node {
	uint64_t u64;
	struct cvmx_ocx_com_node_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t fixed_pin                    : 1;  /**< The current value of the OCI_FIXED_NODE pin. */
	uint64_t fixed                        : 1;  /**< ID Valid associated with the chip. This register is used by the link initialization
                                                         software to help assign IDs and is transmitted over CCPI. The FIXED field set during a
                                                         cold
                                                         reset to the value of the OCI_FIXED_NODE pin. The value is also readable in the
                                                         OCX_LNE(0..23)_STS_MSG[TX_META_DAT<2>] for each lane. The FIXED field of the link partner
                                                         can be examined by locally reading the OCX_LNE(0..23)_STS_MSG[RX_META_DAT<2>] on each
                                                         valid lane or remotely reading the OCX_COM_NODE[FIXED] on the link partner. */
	uint64_t id                           : 2;  /**< Node ID associated with the chip. This register is used by the rest of the chip to
                                                         determine what traffic is transmitted over CCPI. The value should not match the
                                                         OCX_COM_LINK(0..2)_CTL[ID] of any active link. The ID field is set during a cold reset to
                                                         the value of the OCI_NODE_ID pins. The value is also readable in the
                                                         OCX_LNE(0..23)_STS_MSG[TX_META_DAT<1:0>] for each lane. The ID field of the link partner
                                                         can be examined by locally reading the OCX_LNE(0..23)_STS_MSG[RX_META_DAT<1:0>] on each
                                                         valid lane or remotely reading the OCX_COM_NODE[ID] on the link partner. */
#else
	uint64_t id                           : 2;
	uint64_t fixed                        : 1;
	uint64_t fixed_pin                    : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_ocx_com_node_s            cn78xx;
	struct cvmx_ocx_com_node_s            cn78xxp1;
};
typedef union cvmx_ocx_com_node cvmx_ocx_com_node_t;

/**
 * cvmx_ocx_dll#_status
 *
 * Contains diagnostic information on the internal core clock DLLs. Index 0 is the northeast DLL,
 * 1 the southeast DLL.
 */
union cvmx_ocx_dllx_status {
	uint64_t u64;
	struct cvmx_ocx_dllx_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t max_dll_setting              : 12; /**< Max reported DLL setting. */
	uint64_t min_dll_setting              : 12; /**< Min reported DLL setting. */
	uint64_t pd_pos_rclk_refclk           : 1;  /**< Phase detector output. */
	uint64_t pdl_rclk_refclk              : 1;  /**< Phase detector output. */
	uint64_t pdr_rclk_refclk              : 1;  /**< Phase detector output. */
	uint64_t reserved_32_32               : 1;
	uint64_t dly_elem_enable              : 16; /**< Delay element enable. */
	uint64_t dll_setting                  : 12; /**< DLL setting. */
	uint64_t reserved_1_3                 : 3;
	uint64_t dll_lock                     : 1;  /**< DLL lock: 1 = locked, 0 = unlocked. */
#else
	uint64_t dll_lock                     : 1;
	uint64_t reserved_1_3                 : 3;
	uint64_t dll_setting                  : 12;
	uint64_t dly_elem_enable              : 16;
	uint64_t reserved_32_32               : 1;
	uint64_t pdr_rclk_refclk              : 1;
	uint64_t pdl_rclk_refclk              : 1;
	uint64_t pd_pos_rclk_refclk           : 1;
	uint64_t min_dll_setting              : 12;
	uint64_t max_dll_setting              : 12;
	uint64_t reserved_60_63               : 4;
#endif
	} s;
	struct cvmx_ocx_dllx_status_s         cn78xx;
	struct cvmx_ocx_dllx_status_cn78xxp1 {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} cn78xxp1;
};
typedef union cvmx_ocx_dllx_status cvmx_ocx_dllx_status_t;

/**
 * cvmx_ocx_frc#_stat0
 */
union cvmx_ocx_frcx_stat0 {
	uint64_t u64;
	struct cvmx_ocx_frcx_stat0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_21_63               : 43;
	uint64_t align_cnt                    : 21; /**< Indicates the number of alignment sequences received (i.e. those that do not violate the
                                                         current alignment). */
#else
	uint64_t align_cnt                    : 21;
	uint64_t reserved_21_63               : 43;
#endif
	} s;
	struct cvmx_ocx_frcx_stat0_s          cn78xx;
	struct cvmx_ocx_frcx_stat0_s          cn78xxp1;
};
typedef union cvmx_ocx_frcx_stat0 cvmx_ocx_frcx_stat0_t;

/**
 * cvmx_ocx_frc#_stat1
 */
union cvmx_ocx_frcx_stat1 {
	uint64_t u64;
	struct cvmx_ocx_frcx_stat1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_21_63               : 43;
	uint64_t align_err_cnt                : 21; /**< Indicates the number of alignment sequences received in error (i.e. those that violate the
                                                         current alignment). */
#else
	uint64_t align_err_cnt                : 21;
	uint64_t reserved_21_63               : 43;
#endif
	} s;
	struct cvmx_ocx_frcx_stat1_s          cn78xx;
	struct cvmx_ocx_frcx_stat1_s          cn78xxp1;
};
typedef union cvmx_ocx_frcx_stat1 cvmx_ocx_frcx_stat1_t;

/**
 * cvmx_ocx_frc#_stat2
 */
union cvmx_ocx_frcx_stat2 {
	uint64_t u64;
	struct cvmx_ocx_frcx_stat2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_21_63               : 43;
	uint64_t align_done                   : 21; /**< Indicates the number of attempts at alignment that succeeded. */
#else
	uint64_t align_done                   : 21;
	uint64_t reserved_21_63               : 43;
#endif
	} s;
	struct cvmx_ocx_frcx_stat2_s          cn78xx;
	struct cvmx_ocx_frcx_stat2_s          cn78xxp1;
};
typedef union cvmx_ocx_frcx_stat2 cvmx_ocx_frcx_stat2_t;

/**
 * cvmx_ocx_frc#_stat3
 */
union cvmx_ocx_frcx_stat3 {
	uint64_t u64;
	struct cvmx_ocx_frcx_stat3_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_21_63               : 43;
	uint64_t align_fail                   : 21; /**< Indicates the number of attempts at alignment that failed. */
#else
	uint64_t align_fail                   : 21;
	uint64_t reserved_21_63               : 43;
#endif
	} s;
	struct cvmx_ocx_frcx_stat3_s          cn78xx;
	struct cvmx_ocx_frcx_stat3_s          cn78xxp1;
};
typedef union cvmx_ocx_frcx_stat3 cvmx_ocx_frcx_stat3_t;

/**
 * cvmx_ocx_lne#_bad_cnt
 */
union cvmx_ocx_lnex_bad_cnt {
	uint64_t u64;
	struct cvmx_ocx_lnex_bad_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t tx_bad_crc32                 : 1;  /**< Send 1 diagnostic word with bad CRC32 to the selected lane.
                                                         Injects just once. */
	uint64_t tx_bad_6467_cnt              : 5;  /**< Send N bad 64B/67B code words on selected lane. */
	uint64_t tx_bad_sync_cnt              : 3;  /**< Send N bad sync words on selected lane. */
	uint64_t tx_bad_scram_cnt             : 3;  /**< Send N bad scram state on selected lane. */
#else
	uint64_t tx_bad_scram_cnt             : 3;
	uint64_t tx_bad_sync_cnt              : 3;
	uint64_t tx_bad_6467_cnt              : 5;
	uint64_t tx_bad_crc32                 : 1;
	uint64_t reserved_12_63               : 52;
#endif
	} s;
	struct cvmx_ocx_lnex_bad_cnt_s        cn78xx;
	struct cvmx_ocx_lnex_bad_cnt_s        cn78xxp1;
};
typedef union cvmx_ocx_lnex_bad_cnt cvmx_ocx_lnex_bad_cnt_t;

/**
 * cvmx_ocx_lne#_cfg
 */
union cvmx_ocx_lnex_cfg {
	uint64_t u64;
	struct cvmx_ocx_lnex_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_9_63                : 55;
	uint64_t rx_bdry_lock_dis             : 1;  /**< Disable word boundary lock. While disabled, received data is tossed. Once enabled,
                                                         received data is searched for legal 2-bit patterns. */
	uint64_t reserved_3_7                 : 5;
	uint64_t rx_stat_wrap_dis             : 1;  /**< Upon overflow, a statistics counter should saturate instead of wrapping. */
	uint64_t rx_stat_rdclr                : 1;  /**< CSR read to OCX_LNEx_STAT* clears the selected counter after returning its current value. */
	uint64_t rx_stat_ena                  : 1;  /**< Enable RX lane statistics counters. */
#else
	uint64_t rx_stat_ena                  : 1;
	uint64_t rx_stat_rdclr                : 1;
	uint64_t rx_stat_wrap_dis             : 1;
	uint64_t reserved_3_7                 : 5;
	uint64_t rx_bdry_lock_dis             : 1;
	uint64_t reserved_9_63                : 55;
#endif
	} s;
	struct cvmx_ocx_lnex_cfg_s            cn78xx;
	struct cvmx_ocx_lnex_cfg_s            cn78xxp1;
};
typedef union cvmx_ocx_lnex_cfg cvmx_ocx_lnex_cfg_t;

/**
 * cvmx_ocx_lne#_int
 */
union cvmx_ocx_lnex_int {
	uint64_t u64;
	struct cvmx_ocx_lnex_int_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t disp_err                     : 1;  /**< RX disparity error encountered. These receive errors may occur during normal operation,
                                                         and may likely occur during link bringup. Hardware normally will automatically correct the
                                                         error. Software may choose to count the number of these errors. */
	uint64_t bad_64b67b                   : 1;  /**< Bad 64B/67B codeword encountered. Once the bad word reaches the link, as denoted by
                                                         OCX_COM_LINK(0..2)_INT[BAD_WORD], a retry handshake is initiated. These receive errors may
                                                         occur during normal operation, and may likely occur during link bringup. Hardware normally
                                                         will automatically correct the error. Software may choose to count the number of these
                                                         errors. */
	uint64_t stat_cnt_ovfl                : 1;  /**< RX lane statistic counter overflow. */
	uint64_t stat_msg                     : 1;  /**< Status bits for the link or a lane transitioned from a 1 (healthy) to a 0 (problem). These
                                                         should not occur during normal operation. This may be considered fatal, depending on the
                                                         software protocol. */
	uint64_t dskew_fifo_ovfl              : 1;  /**< RX deskew FIFO overflow occurred. These receive errors may occur during normal operation,
                                                         and may likely occur during link bringup. Hardware normally will automatically correct the
                                                         error. Software may choose to count the number of these errors. */
	uint64_t scrm_sync_loss               : 1;  /**< 4 consecutive bad sync words or 3 consecutive scramble state mismatches. These receive
                                                         errors should not occur during normal operation, but may likely occur during link bringup.
                                                         Hardware normally will automatically correct the error. Software may choose to count the
                                                         number of these errors. */
	uint64_t ukwn_cntl_word               : 1;  /**< Unknown framing control word. Block type does not match any of (SYNC, SCRAM, SKIP, DIAG).
                                                         These receive errors may occur during normal operation. Hardware normally will
                                                         automatically correct the error. Software may choose to count the number of these errors. */
	uint64_t crc32_err                    : 1;  /**< Diagnostic CRC32 errors. These receive errors may occur during normal operation, typically
                                                         in the presence of other errors, and may likely occur during link bringup. Hardware
                                                         normally will automatically correct the error. Software may choose to count the number of
                                                         these errors. */
	uint64_t bdry_sync_loss               : 1;  /**< RX logic lost word boundary sync after 16 tries. Hardware automatically attempts to regain
                                                         word boundary sync. These receive errors should not occur during normal operation, but may
                                                         likely occur during link bringup. Hardware normally will automatically correct the error.
                                                         Software may choose to count the number of these errors. */
	uint64_t serdes_lock_loss             : 1;  /**< RX SerDes loses lock. These receive errors should not occur during normal operation. This
                                                         may be considered fatal. */
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
	struct cvmx_ocx_lnex_int_s            cn78xx;
	struct cvmx_ocx_lnex_int_s            cn78xxp1;
};
typedef union cvmx_ocx_lnex_int cvmx_ocx_lnex_int_t;

/**
 * cvmx_ocx_lne#_int_en
 */
union cvmx_ocx_lnex_int_en {
	uint64_t u64;
	struct cvmx_ocx_lnex_int_en_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t disp_err                     : 1;  /**< Reserved. */
	uint64_t bad_64b67b                   : 1;  /**< Enable bit for bad 64B/67B codeword encountered. */
	uint64_t stat_cnt_ovfl                : 1;  /**< Enable bit for RX lane statistic counter overflow. */
	uint64_t stat_msg                     : 1;  /**< Enable bit for status bits for the link or a lane transitioned from a 1 (healthy) to a 0 (problem). */
	uint64_t dskew_fifo_ovfl              : 1;  /**< Enable bit for RX deskew FIFO overflow occurred. */
	uint64_t scrm_sync_loss               : 1;  /**< Enable bit for 4 consecutive bad sync words or 3 consecutive scramble state mismatches. */
	uint64_t ukwn_cntl_word               : 1;  /**< Enable bit for unknown framing control word. Block type does not match any of (SYNC,
                                                         SCRAM, SKIP, DIAG). */
	uint64_t crc32_err                    : 1;  /**< Enable bit for diagnostic CRC32 errors. */
	uint64_t bdry_sync_loss               : 1;  /**< Enable bit for RX logic lost word boundary sync after 16 tries. */
	uint64_t serdes_lock_loss             : 1;  /**< Enable bit for RX SerDes loses lock. */
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
	struct cvmx_ocx_lnex_int_en_s         cn78xx;
	struct cvmx_ocx_lnex_int_en_s         cn78xxp1;
};
typedef union cvmx_ocx_lnex_int_en cvmx_ocx_lnex_int_en_t;

/**
 * cvmx_ocx_lne#_stat00
 */
union cvmx_ocx_lnex_stat00 {
	uint64_t u64;
	struct cvmx_ocx_lnex_stat00_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t ser_lock_loss_cnt            : 18; /**< Number of times the lane lost clock-data-recovery. Saturates. Interrupt on saturation if
                                                         OCX_LNE(0..23)_INT_EN[STAT_CNT_OVFL] is set. */
#else
	uint64_t ser_lock_loss_cnt            : 18;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_ocx_lnex_stat00_s         cn78xx;
	struct cvmx_ocx_lnex_stat00_s         cn78xxp1;
};
typedef union cvmx_ocx_lnex_stat00 cvmx_ocx_lnex_stat00_t;

/**
 * cvmx_ocx_lne#_stat01
 */
union cvmx_ocx_lnex_stat01 {
	uint64_t u64;
	struct cvmx_ocx_lnex_stat01_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t bdry_sync_loss_cnt           : 18; /**< Number of times a lane lost word boundary synchronization. Saturates. Interrupt on
                                                         saturation if OCX_LNE(0..23)_INT_EN[STAT_CNT_OVFL] is set. */
#else
	uint64_t bdry_sync_loss_cnt           : 18;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_ocx_lnex_stat01_s         cn78xx;
	struct cvmx_ocx_lnex_stat01_s         cn78xxp1;
};
typedef union cvmx_ocx_lnex_stat01 cvmx_ocx_lnex_stat01_t;

/**
 * cvmx_ocx_lne#_stat02
 */
union cvmx_ocx_lnex_stat02 {
	uint64_t u64;
	struct cvmx_ocx_lnex_stat02_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t syncw_bad_cnt                : 18; /**< Number of bad synchronization words. Saturates. Interrupt on saturation if
                                                         OCX_LNE(0..23)_INT_EN[STAT_CNT_OVFL] is set. */
#else
	uint64_t syncw_bad_cnt                : 18;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_ocx_lnex_stat02_s         cn78xx;
	struct cvmx_ocx_lnex_stat02_s         cn78xxp1;
};
typedef union cvmx_ocx_lnex_stat02 cvmx_ocx_lnex_stat02_t;

/**
 * cvmx_ocx_lne#_stat03
 */
union cvmx_ocx_lnex_stat03 {
	uint64_t u64;
	struct cvmx_ocx_lnex_stat03_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t syncw_good_cnt               : 18; /**< Number of good synchronization words. Saturates. Interrupt on saturation if
                                                         OCX_LNE(0..23)_INT_EN[STAT_CNT_OVFL] is set. */
#else
	uint64_t syncw_good_cnt               : 18;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_ocx_lnex_stat03_s         cn78xx;
	struct cvmx_ocx_lnex_stat03_s         cn78xxp1;
};
typedef union cvmx_ocx_lnex_stat03 cvmx_ocx_lnex_stat03_t;

/**
 * cvmx_ocx_lne#_stat04
 */
union cvmx_ocx_lnex_stat04 {
	uint64_t u64;
	struct cvmx_ocx_lnex_stat04_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t bad_64b67b_cnt               : 18; /**< Number of bad 64B/67B words, meaning bit 65 or 64 has been corrupted. Saturates. Interrupt
                                                         on saturation if OCX_LNE(0..23)_INT_EN[STAT_CNT_OVFL] is set. */
#else
	uint64_t bad_64b67b_cnt               : 18;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_ocx_lnex_stat04_s         cn78xx;
	struct cvmx_ocx_lnex_stat04_s         cn78xxp1;
};
typedef union cvmx_ocx_lnex_stat04 cvmx_ocx_lnex_stat04_t;

/**
 * cvmx_ocx_lne#_stat05
 */
union cvmx_ocx_lnex_stat05 {
	uint64_t u64;
	struct cvmx_ocx_lnex_stat05_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_27_63               : 37;
	uint64_t data_word_cnt                : 27; /**< Number of data words received. Saturates. Interrupt on saturation if
                                                         OCX_LNE(0..23)_INT_EN[STAT_CNT_OVFL] is set. */
#else
	uint64_t data_word_cnt                : 27;
	uint64_t reserved_27_63               : 37;
#endif
	} s;
	struct cvmx_ocx_lnex_stat05_s         cn78xx;
	struct cvmx_ocx_lnex_stat05_s         cn78xxp1;
};
typedef union cvmx_ocx_lnex_stat05 cvmx_ocx_lnex_stat05_t;

/**
 * cvmx_ocx_lne#_stat06
 */
union cvmx_ocx_lnex_stat06 {
	uint64_t u64;
	struct cvmx_ocx_lnex_stat06_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_27_63               : 37;
	uint64_t cntl_word_cnt                : 27; /**< Number of control words received. Saturates. Interrupt on saturation if
                                                         OCX_LNE(0..23)_INT_EN[STAT_CNT_OVFL] is set. */
#else
	uint64_t cntl_word_cnt                : 27;
	uint64_t reserved_27_63               : 37;
#endif
	} s;
	struct cvmx_ocx_lnex_stat06_s         cn78xx;
	struct cvmx_ocx_lnex_stat06_s         cn78xxp1;
};
typedef union cvmx_ocx_lnex_stat06 cvmx_ocx_lnex_stat06_t;

/**
 * cvmx_ocx_lne#_stat07
 */
union cvmx_ocx_lnex_stat07 {
	uint64_t u64;
	struct cvmx_ocx_lnex_stat07_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t unkwn_word_cnt               : 18; /**< Number of unknown control words. Saturates. Interrupt on saturation if
                                                         OCX_LNE(0..23)_INT_EN[STAT_CNT_OVFL] is set. */
#else
	uint64_t unkwn_word_cnt               : 18;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_ocx_lnex_stat07_s         cn78xx;
	struct cvmx_ocx_lnex_stat07_s         cn78xxp1;
};
typedef union cvmx_ocx_lnex_stat07 cvmx_ocx_lnex_stat07_t;

/**
 * cvmx_ocx_lne#_stat08
 */
union cvmx_ocx_lnex_stat08 {
	uint64_t u64;
	struct cvmx_ocx_lnex_stat08_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t scrm_sync_loss_cnt           : 18; /**< Number of times scrambler synchronization was lost (due to either 4 consecutive bad sync
                                                         words or 3 consecutive scrambler state mismatches). Saturates. Interrupt on saturation if
                                                         OCX_LNE(0..23)_INT_EN[STAT_CNT_OVFL] is set. */
#else
	uint64_t scrm_sync_loss_cnt           : 18;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_ocx_lnex_stat08_s         cn78xx;
	struct cvmx_ocx_lnex_stat08_s         cn78xxp1;
};
typedef union cvmx_ocx_lnex_stat08 cvmx_ocx_lnex_stat08_t;

/**
 * cvmx_ocx_lne#_stat09
 */
union cvmx_ocx_lnex_stat09 {
	uint64_t u64;
	struct cvmx_ocx_lnex_stat09_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t scrm_match_cnt               : 18; /**< Number of scrambler state matches received. Saturates. Interrupt on saturation if
                                                         OCX_LNE(0..23)_INT_EN[STAT_CNT_OVFL] is set. */
#else
	uint64_t scrm_match_cnt               : 18;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_ocx_lnex_stat09_s         cn78xx;
	struct cvmx_ocx_lnex_stat09_s         cn78xxp1;
};
typedef union cvmx_ocx_lnex_stat09 cvmx_ocx_lnex_stat09_t;

/**
 * cvmx_ocx_lne#_stat10
 */
union cvmx_ocx_lnex_stat10 {
	uint64_t u64;
	struct cvmx_ocx_lnex_stat10_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_18_63               : 46;
	uint64_t skipw_good_cnt               : 18; /**< Number of good skip words. Saturates. Interrupt on saturation if
                                                         OCX_LNE(0..23)_INT_EN[STAT_CNT_OVFL] is set. */
#else
	uint64_t skipw_good_cnt               : 18;
	uint64_t reserved_18_63               : 46;
#endif
	} s;
	struct cvmx_ocx_lnex_stat10_s         cn78xx;
	struct cvmx_ocx_lnex_stat10_s         cn78xxp1;
};
typedef union cvmx_ocx_lnex_stat10 cvmx_ocx_lnex_stat10_t;

/**
 * cvmx_ocx_lne#_stat11
 */
union cvmx_ocx_lnex_stat11 {
	uint64_t u64;
	struct cvmx_ocx_lnex_stat11_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_27_63               : 37;
	uint64_t crc32_err_cnt                : 27; /**< Number of errors in the lane CRC. Saturates. Interrupt on saturation if
                                                         OCX_LNE(0..23)_INT_EN[STAT_CNT_OVFL] is set. */
#else
	uint64_t crc32_err_cnt                : 27;
	uint64_t reserved_27_63               : 37;
#endif
	} s;
	struct cvmx_ocx_lnex_stat11_s         cn78xx;
	struct cvmx_ocx_lnex_stat11_s         cn78xxp1;
};
typedef union cvmx_ocx_lnex_stat11 cvmx_ocx_lnex_stat11_t;

/**
 * cvmx_ocx_lne#_stat12
 */
union cvmx_ocx_lnex_stat12 {
	uint64_t u64;
	struct cvmx_ocx_lnex_stat12_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_27_63               : 37;
	uint64_t crc32_match_cnt              : 27; /**< Number of CRC32 matches received. Saturates. Interrupt on saturation if
                                                         OCX_LNE(0..23)_INT_EN[STAT_CNT_OVFL] is set. */
#else
	uint64_t crc32_match_cnt              : 27;
	uint64_t reserved_27_63               : 37;
#endif
	} s;
	struct cvmx_ocx_lnex_stat12_s         cn78xx;
	struct cvmx_ocx_lnex_stat12_s         cn78xxp1;
};
typedef union cvmx_ocx_lnex_stat12 cvmx_ocx_lnex_stat12_t;

/**
 * cvmx_ocx_lne#_stat13
 */
union cvmx_ocx_lnex_stat13 {
	uint64_t u64;
	struct cvmx_ocx_lnex_stat13_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t trn_bad_cnt                  : 16; /**< Number of training frames received with an invalid control channel. Saturates. Interrupt
                                                         on saturation if OCX_LNE(0..23)_INT_EN[STAT_CNT_OVFL] is set. */
#else
	uint64_t trn_bad_cnt                  : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_ocx_lnex_stat13_s         cn78xx;
	struct cvmx_ocx_lnex_stat13_s         cn78xxp1;
};
typedef union cvmx_ocx_lnex_stat13 cvmx_ocx_lnex_stat13_t;

/**
 * cvmx_ocx_lne#_stat14
 */
union cvmx_ocx_lnex_stat14 {
	uint64_t u64;
	struct cvmx_ocx_lnex_stat14_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t trn_prbs_bad_cnt             : 16; /**< Number of training frames received with a bad PRBS pattern. Saturates. Interrupt on
                                                         saturation if OCX_LNE(0..23)_INT_EN[STAT_CNT_OVFL] is set. */
#else
	uint64_t trn_prbs_bad_cnt             : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_ocx_lnex_stat14_s         cn78xx;
	struct cvmx_ocx_lnex_stat14_s         cn78xxp1;
};
typedef union cvmx_ocx_lnex_stat14 cvmx_ocx_lnex_stat14_t;

/**
 * cvmx_ocx_lne#_status
 */
union cvmx_ocx_lnex_status {
	uint64_t u64;
	struct cvmx_ocx_lnex_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t rx_trn_val                   : 1;  /**< The control channel of a link training was received without any errors. */
	uint64_t rx_scrm_sync                 : 1;  /**< RX scrambler synchronization status. One when synchronization achieved. */
	uint64_t rx_bdry_sync                 : 1;  /**< RX word boundary sync status. One when synchronization achieved. */
#else
	uint64_t rx_bdry_sync                 : 1;
	uint64_t rx_scrm_sync                 : 1;
	uint64_t rx_trn_val                   : 1;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_ocx_lnex_status_s         cn78xx;
	struct cvmx_ocx_lnex_status_s         cn78xxp1;
};
typedef union cvmx_ocx_lnex_status cvmx_ocx_lnex_status_t;

/**
 * cvmx_ocx_lne#_sts_msg
 */
union cvmx_ocx_lnex_sts_msg {
	uint64_t u64;
	struct cvmx_ocx_lnex_sts_msg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rx_meta_val                  : 1;  /**< Meta-data received in the diagnostic word (per-lane) is valid. */
	uint64_t reserved_37_62               : 26;
	uint64_t rx_meta_dat                  : 3;  /**< Meta-data received in the diagnostic word (per-lane). */
	uint64_t rx_lne_stat                  : 1;  /**< Lane status received in the diagnostic word (per-lane). One when healthy (according to the
                                                         Interlaken spec). */
	uint64_t rx_lnk_stat                  : 1;  /**< Link status received in the diagnostic word (per-lane). One when healthy (according to the
                                                         Interlaken spec). */
	uint64_t reserved_5_31                : 27;
	uint64_t tx_meta_dat                  : 3;  /**< Meta-data transmitted in the diagnostic word (per-lane). */
	uint64_t tx_lne_stat                  : 1;  /**< Lane status transmitted in the diagnostic word (per-lane). One means healthy (according to
                                                         the Interlaken spec). */
	uint64_t tx_lnk_stat                  : 1;  /**< Link status transmitted in the diagnostic word (per-lane). One means healthy (according to
                                                         the Interlaken spec). */
#else
	uint64_t tx_lnk_stat                  : 1;
	uint64_t tx_lne_stat                  : 1;
	uint64_t tx_meta_dat                  : 3;
	uint64_t reserved_5_31                : 27;
	uint64_t rx_lnk_stat                  : 1;
	uint64_t rx_lne_stat                  : 1;
	uint64_t rx_meta_dat                  : 3;
	uint64_t reserved_37_62               : 26;
	uint64_t rx_meta_val                  : 1;
#endif
	} s;
	struct cvmx_ocx_lnex_sts_msg_s        cn78xx;
	struct cvmx_ocx_lnex_sts_msg_s        cn78xxp1;
};
typedef union cvmx_ocx_lnex_sts_msg cvmx_ocx_lnex_sts_msg_t;

/**
 * cvmx_ocx_lne#_trn_ctl
 */
union cvmx_ocx_lnex_trn_ctl {
	uint64_t u64;
	struct cvmx_ocx_lnex_trn_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t lock                         : 1;  /**< Training frame boundary locked. */
	uint64_t done                         : 1;  /**< Training done. For diagnostic use only may be written to 1 to force training done. */
	uint64_t ena                          : 1;  /**< OCX_LNEX_TRN_CTL[TRN_ENA]=1 indicates that the lane is currently training.  It is a status
                                                         bit used for debug.  It will read as zero when training has completed or when the QLM
                                                         isn't ready for training. */
	uint64_t eie_detect                   : 1;  /**< Electrical idle exit (EIE) detected. */
#else
	uint64_t eie_detect                   : 1;
	uint64_t ena                          : 1;
	uint64_t done                         : 1;
	uint64_t lock                         : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} s;
	struct cvmx_ocx_lnex_trn_ctl_s        cn78xx;
	struct cvmx_ocx_lnex_trn_ctl_s        cn78xxp1;
};
typedef union cvmx_ocx_lnex_trn_ctl cvmx_ocx_lnex_trn_ctl_t;

/**
 * cvmx_ocx_lne#_trn_ld
 */
union cvmx_ocx_lnex_trn_ld {
	uint64_t u64;
	struct cvmx_ocx_lnex_trn_ld_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t lp_manual                    : 1;  /**< Allow software to manually manipulate local device CU/SR by ignoring hardware update. */
	uint64_t reserved_49_62               : 14;
	uint64_t ld_cu_val                    : 1;  /**< Local device coefficient update field valid. */
	uint64_t ld_cu_dat                    : 16; /**< Local device coefficient update field data.
                                                         The format of this field is BGX_SPU_BR_TRAIN_CUP_S. */
	uint64_t reserved_17_31               : 15;
	uint64_t ld_sr_val                    : 1;  /**< Local device status report field valid. */
	uint64_t ld_sr_dat                    : 16; /**< Local device status report field data.
                                                         The format of this field is BGX_SPU_BR_TRAIN_REP_S. */
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
	struct cvmx_ocx_lnex_trn_ld_s         cn78xx;
	struct cvmx_ocx_lnex_trn_ld_s         cn78xxp1;
};
typedef union cvmx_ocx_lnex_trn_ld cvmx_ocx_lnex_trn_ld_t;

/**
 * cvmx_ocx_lne#_trn_lp
 */
union cvmx_ocx_lnex_trn_lp {
	uint64_t u64;
	struct cvmx_ocx_lnex_trn_lp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_49_63               : 15;
	uint64_t lp_cu_val                    : 1;  /**< Link partner coefficient update field valid. */
	uint64_t lp_cu_dat                    : 16; /**< Link partner coefficient update field data.
                                                         The format of this field is BGX_SPU_BR_TRAIN_CUP_S. */
	uint64_t reserved_17_31               : 15;
	uint64_t lp_sr_val                    : 1;  /**< Link partner status report field valid. */
	uint64_t lp_sr_dat                    : 16; /**< Link partner status report field data.
                                                         The format of this field is BGX_SPU_BR_TRAIN_REP_S. */
#else
	uint64_t lp_sr_dat                    : 16;
	uint64_t lp_sr_val                    : 1;
	uint64_t reserved_17_31               : 15;
	uint64_t lp_cu_dat                    : 16;
	uint64_t lp_cu_val                    : 1;
	uint64_t reserved_49_63               : 15;
#endif
	} s;
	struct cvmx_ocx_lnex_trn_lp_s         cn78xx;
	struct cvmx_ocx_lnex_trn_lp_s         cn78xxp1;
};
typedef union cvmx_ocx_lnex_trn_lp cvmx_ocx_lnex_trn_lp_t;

/**
 * cvmx_ocx_lne_dbg
 */
union cvmx_ocx_lne_dbg {
	uint64_t u64;
	struct cvmx_ocx_lne_dbg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t timeout                      : 24; /**< Number of core clocks (RCLKs) used by the bad lane timer. If this timer expires before all
                                                         enabled lanes can be made ready, then any lane which is not ready is disabled via
                                                         OCX_QLM(0..5)_CFG[SER_LANE_BAD]. For diagnostic use only. */
	uint64_t reserved_38_39               : 2;
	uint64_t frc_stats_ena                : 1;  /**< Enable FRC statistic counters. */
	uint64_t rx_dis_psh_skip              : 1;  /**< When [RX_DIS_PSH_SKIP]=0, skip words are destriped. When [RX_DIS_PSH_SKIP]=1, skip words
                                                         are
                                                         discarded in the lane logic. If the lane is in internal loopback mode, [RX_DIS_PSH_SKIP]
                                                         is
                                                         ignored and skip words are always discarded in the lane logic. */
	uint64_t rx_mfrm_len                  : 2;  /**< The quantity of data received on each lane including one sync word, scrambler state,
                                                         diagnostic word, zero or more skip words, and the data payload.
                                                         0x0 = 2048 words.
                                                         0x1 = 1024 words.
                                                         0x2 = 512 words.
                                                         0x3 = 128 words. */
	uint64_t rx_dis_ukwn                  : 1;  /**< Disable normal response to unknown words. They are still logged but do not cause an error
                                                         to all open channels. */
	uint64_t rx_dis_scram                 : 1;  /**< Disable lane scrambler. */
	uint64_t reserved_5_31                : 27;
	uint64_t tx_lane_rev                  : 1;  /**< TX lane reversal. When enabled, lane destriping is performed from the most significant
                                                         lane enabled to least significant lane enabled QLM_SELECT must be zero before changing
                                                         [LANE_REV]. */
	uint64_t tx_mfrm_len                  : 2;  /**< The quantity of data sent on each lane including one sync word, scrambler state,
                                                         diagnostic word, zero or more skip words, and the data payload.
                                                         0x0 = 2048 words.
                                                         0x1 = 1024 words.
                                                         0x2 = 512 words.
                                                         0x3 = 128 words. */
	uint64_t tx_dis_dispr                 : 1;  /**< Disparity disable. */
	uint64_t tx_dis_scram                 : 1;  /**< Scrambler disable. */
#else
	uint64_t tx_dis_scram                 : 1;
	uint64_t tx_dis_dispr                 : 1;
	uint64_t tx_mfrm_len                  : 2;
	uint64_t tx_lane_rev                  : 1;
	uint64_t reserved_5_31                : 27;
	uint64_t rx_dis_scram                 : 1;
	uint64_t rx_dis_ukwn                  : 1;
	uint64_t rx_mfrm_len                  : 2;
	uint64_t rx_dis_psh_skip              : 1;
	uint64_t frc_stats_ena                : 1;
	uint64_t reserved_38_39               : 2;
	uint64_t timeout                      : 24;
#endif
	} s;
	struct cvmx_ocx_lne_dbg_s             cn78xx;
	struct cvmx_ocx_lne_dbg_s             cn78xxp1;
};
typedef union cvmx_ocx_lne_dbg cvmx_ocx_lne_dbg_t;

/**
 * cvmx_ocx_lnk#_cfg
 */
union cvmx_ocx_lnkx_cfg {
	uint64_t u64;
	struct cvmx_ocx_lnkx_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_54_63               : 10;
	uint64_t qlm_manual                   : 6;  /**< QLM manual mask, where each bit corresponds to a QLM. A link automatically selects a QLM
                                                         unless either QLM_MANUAL[QLM] is set or a QLM is not eligible for the link.
                                                         _ QLM_MANUAL<0> = LNE(0..3) = QLM0.
                                                         _ QLM_MANUAL<1> = LNE(7..4) = QLM1.
                                                         _ QLM_MANUAL<2> = LNE(11..8) = QLM2.
                                                         _ QLM_MANUAL<3> = LNE(15..12) = QLM3.
                                                         _ QLM_MANUAL<4> = LNE(19..16) = QLM4.
                                                         _ QLM_MANUAL<5> = LNE(23..23) = QLM5.
                                                         _ LINK 0 may not select QLM4, QLM5.
                                                         _ LINK 1 may not select QLM0, QLM1, QLM4, QLM5.
                                                         _ LINK 2 may not select QLM0, QLM1.
                                                         During a cold reset, this field is initialized to 0x3F when OCI_SPD<3:0> == 0xF.
                                                         During a cold reset, this field is initialized to 0x0 when OCI_SPD<3:0> != 0xF.
                                                         This field is not modified by hardware at any other time.
                                                         This field is not affected by soft or warm reset. */
	uint64_t reserved_38_47               : 10;
	uint64_t qlm_select                   : 6;  /**< QLM select mask, where each bit corresponds to a QLM. A link will transmit/receive data
                                                         using only the selected QLMs. A link is enabled if any QLM is selected. The same QLM
                                                         should not be selected for multiple links.
                                                         [LANE_REV] has no effect on this mapping.
                                                         _ QLM_SELECT<0> = LNE(0..3) = QLM0.
                                                         _ QLM_SELECT<1> = LNE(7..4) = QLM1.
                                                         _ QLM_SELECT<2> = LNE(11..8) = QLM2.
                                                         _ QLM_SELECT<3> = LNE(15..12) = QLM3.
                                                         _ QLM_SELECT<4> = LNE(19..16) = QLM4.
                                                         _ QLM_SELECT<5> = LNE(23..23) = QLM5.
                                                         _ LINK 0 may not select QLM4, QLM5.
                                                         _ LINK 1 may not select QLM0, QLM1, QLM4, QLM5.
                                                         _ LINK 2 may not select QLM0, QLM1.
                                                         _ LINK 2 may not select QLM2 or QLM3 when LINK1 selects any QLM.
                                                         _ LINK 0 may not select QLM2 or QLM3 when LINK1 selects any QLM.
                                                         _ LINK 0 automatically selects QLM0 when [QLM_MANUAL]<0>=0.
                                                         _ LINK 0 automatically selects QLM1 when [QLM_MANUAL]<1>=0.
                                                         _ LINK 0 automatically selects QLM2 when [QLM_MANUAL]<2>=0 and OCX_QLM2_CFG.SER_LOCAL=0.
                                                         _ LINK 1 automatically selects QLM2 when [QLM_MANUAL]<2>=0 and OCX_QLM2_CFG.SER_LOCAL=1.
                                                         _ LINK 1 automatically selects QLM3 when [QLM_MANUAL]<3>=0 and OCX_QLM3_CFG.SER_LOCAL=1.
                                                         _ LINK 2 automatically selects QLM3 when [QLM_MANUAL]<3>=0 and OCX_QLM3_CFG.SER_LOCAL=0.
                                                         _ LINK 3 automatically selects QLM4 when [QLM_MANUAL]<4>=0.
                                                         _ LINK 3 automatically selects QLM5 when [QLM_MANUAL]<5>=0.
                                                         A link with QLM_SELECT = 000000 is invalid and will never exchange traffic with the
                                                         link partner. */
	uint64_t reserved_29_31               : 3;
	uint64_t data_rate                    : 13; /**< Reserved. */
	uint64_t low_delay                    : 6;  /**< Reserved. */
	uint64_t lane_align_dis               : 1;  /**< Disable the RX lane alignment. */
	uint64_t lane_rev                     : 1;  /**< RX lane reversal.   When enabled, lane destriping is performed from the most significant
                                                         lane enabled to least significant lane enabled [QLM_SELECT] must be zero before changing
                                                         [LANE_REV]. */
	uint64_t lane_rev_auto                : 1;  /**< Reserved. */
	uint64_t reserved_0_6                 : 7;
#else
	uint64_t reserved_0_6                 : 7;
	uint64_t lane_rev_auto                : 1;
	uint64_t lane_rev                     : 1;
	uint64_t lane_align_dis               : 1;
	uint64_t low_delay                    : 6;
	uint64_t data_rate                    : 13;
	uint64_t reserved_29_31               : 3;
	uint64_t qlm_select                   : 6;
	uint64_t reserved_38_47               : 10;
	uint64_t qlm_manual                   : 6;
	uint64_t reserved_54_63               : 10;
#endif
	} s;
	struct cvmx_ocx_lnkx_cfg_s            cn78xx;
	struct cvmx_ocx_lnkx_cfg_s            cn78xxp1;
};
typedef union cvmx_ocx_lnkx_cfg cvmx_ocx_lnkx_cfg_t;

/**
 * cvmx_ocx_pp_cmd
 *
 * Contains the address, read size and write mask to used for the core operation. Write data
 * should be written first and placed in the OCX_PP_WR_DATA register. Writing this register
 * starts the operation. A second write to this register while an operation is in progress will
 * stall. Data is placed in the OCX_PP_RD_DATA register.
 * This register has the same bit fields as OCX_WIN_CMD.
 */
union cvmx_ocx_pp_cmd {
	uint64_t u64;
	struct cvmx_ocx_pp_cmd_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t wr_mask                      : 8;  /**< Mask for the data to be written. When a bit is 1, the corresponding byte will be written.
                                                         The values of this field must be contiguous and for 1, 2, 4, or 8 byte operations and
                                                         aligned to operation size. A value of 0 will produce unpredictable results. Field is
                                                         ignored during a read (LD_OP=1). */
	uint64_t reserved_51_55               : 5;
	uint64_t ld_cmd                       : 2;  /**< The load command sent with the read:
                                                         0x0 = Load 1-bytes.
                                                         0x1 = Load 2-bytes.
                                                         0x2 = Load 4-bytes.
                                                         0x3 = Load 8-bytes. */
	uint64_t ld_op                        : 1;  /**< Operation Type 0=Store 1=Load operation. */
	uint64_t addr                         : 48; /**< The address used in both the load and store operations:
                                                         <47:40> = NCB_ID.
                                                         <39:38> = 0, Not used.
                                                         <37:36> = CCPI_ID.
                                                         <35:0> = Address.
                                                         When <47:43> == SLI & <42:40> == 0 bits <39:0> are:
                                                         <39:38> = 0, Not used.
                                                         <37:36> = CCPI_ID.
                                                         <35:32> = 0, Not used.
                                                         <31:24> = RSL_ID.
                                                         <23:0> = RSL register offset.
                                                         <2:0> are ignored in a store operation. */
#else
	uint64_t addr                         : 48;
	uint64_t ld_op                        : 1;
	uint64_t ld_cmd                       : 2;
	uint64_t reserved_51_55               : 5;
	uint64_t wr_mask                      : 8;
#endif
	} s;
	struct cvmx_ocx_pp_cmd_s              cn78xx;
	struct cvmx_ocx_pp_cmd_s              cn78xxp1;
};
typedef union cvmx_ocx_pp_cmd cvmx_ocx_pp_cmd_t;

/**
 * cvmx_ocx_pp_rd_data
 *
 * This register is the read response data associated with core command. Reads all-ones until
 * response is received.
 * This register has the same bit fields as OCX_WIN_RD_DATA.
 */
union cvmx_ocx_pp_rd_data {
	uint64_t u64;
	struct cvmx_ocx_pp_rd_data_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Read response data. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_ocx_pp_rd_data_s          cn78xx;
	struct cvmx_ocx_pp_rd_data_s          cn78xxp1;
};
typedef union cvmx_ocx_pp_rd_data cvmx_ocx_pp_rd_data_t;

/**
 * cvmx_ocx_pp_wr_data
 *
 * Contains the data to write to the address located in OCX_PP_CMD. Writing this register will
 * cause a write operation to take place.
 * This register has the same bit fields as OCX_WIN_WR_DATA.
 */
union cvmx_ocx_pp_wr_data {
	uint64_t u64;
	struct cvmx_ocx_pp_wr_data_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t wr_data                      : 64; /**< The data to be written. */
#else
	uint64_t wr_data                      : 64;
#endif
	} s;
	struct cvmx_ocx_pp_wr_data_s          cn78xx;
	struct cvmx_ocx_pp_wr_data_s          cn78xxp1;
};
typedef union cvmx_ocx_pp_wr_data cvmx_ocx_pp_wr_data_t;

/**
 * cvmx_ocx_qlm#_cfg
 */
union cvmx_ocx_qlmx_cfg {
	uint64_t u64;
	struct cvmx_ocx_qlmx_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t ser_low                      : 4;  /**< Reserved. */
	uint64_t reserved_42_59               : 18;
	uint64_t ser_limit                    : 10; /**< Reduce latency by limiting the amount of data in flight for each SerDes. For diagnostic
                                                         use only.  Removed in pass 2. */
	uint64_t crd_dis                      : 1;  /**< For diagnostic use only. */
	uint64_t reserved_27_30               : 4;
	uint64_t trn_rxeq_only                : 1;  /**< Reserved. */
	uint64_t timer_dis                    : 1;  /**< Disable bad lane timer. A timer counts core clocks (RCLKs) when any enabled lane is not
                                                         ready, i.e. not in the scrambler sync state. If this timer expires before all enabled
                                                         lanes can be made ready, then any lane which is not ready is disabled via
                                                         OCX_QLM(0..5)_CFG[SER_LANE_BAD]. This field is not affected by soft or warm reset. */
	uint64_t trn_ena                      : 1;  /**< Link training enable. Link training is performed during auto link bring up. Initialized to
                                                         1 during cold reset when OCI_SPD<3:0> pins indicate speed >= 5 GBAUD. Otherwise,
                                                         initialized to 0 during a cold reset. This field is not affected by soft or warm reset. */
	uint64_t ser_lane_ready               : 4;  /**< SerDes lanes that are ready for bundling into the link. */
	uint64_t ser_lane_bad                 : 4;  /**< SerDes lanes excluded from use. */
	uint64_t reserved_7_15                : 9;
	uint64_t ser_lane_rev                 : 1;  /**< SerDes lane reversal has been detected. */
	uint64_t ser_rxpol_auto               : 1;  /**< SerDes lane receive polarity auto detection mode. */
	uint64_t ser_rxpol                    : 1;  /**< SerDes lane receive polarity:
                                                         0 = RX without inversion.
                                                         1 = RX with inversion. */
	uint64_t ser_txpol                    : 1;  /**< SerDes lane transmit polarity:
                                                         0 = TX without inversion.
                                                         1 = TX with inversion. */
	uint64_t reserved_1_2                 : 2;
	uint64_t ser_local                    : 1;  /**< _ Auto initialization may set OCX_LNK0_CFG[QLM_SELECT<2>] = 1 only if
                                                         OCX_QLM2_CFG[SER_LOCAL] = 0.
                                                         _ Auto initialization may set OCX_LNK1_CFG[QLM_SELECT<2>] = 1 only if
                                                         OCX_QLM2_CFG[SER_LOCAL] = 1.
                                                         _ Auto initialization may set OCX_LNK1_CFG[QLM_SELECT<3>] = 1 only if
                                                         OCX_QLM3_CFG[SER_LOCAL] = 1.
                                                         _ Auto initialization may set OCX_LNK2_CFG[QLM_SELECT<3>] = 1 only if
                                                         OCX_QLM3_CFG[SER_LOCAL] = 0.
                                                         _ QLM0/1 can only participate in LNK0; therefore
                                                         OCX_QLM0/1_CFG[SER_LOCAL] has no effect.
                                                         _ QLM4/5 can only participate in LNK2; therefore
                                                         OCX_QLM4/5_CFG[SER_LOCAL] has no effect.
                                                         During a cold reset, initialized as follows:
                                                         _ OCX_QLM2_CFG.SER_LOCAL = pi_oci2_link1.
                                                         _ OCX_QLM3_CFG.SER_LOCAL = pi_oci3_link1.
                                                         The combo of pi_oci2_link1=1 and pi_oci3_link1=0 is illegal.
                                                         The combo of OCX_QLM2_CFG.SER_LOCAL=1 and OCX_QLM3_CFG.SER_LOCAL=0 is illegal. */
#else
	uint64_t ser_local                    : 1;
	uint64_t reserved_1_2                 : 2;
	uint64_t ser_txpol                    : 1;
	uint64_t ser_rxpol                    : 1;
	uint64_t ser_rxpol_auto               : 1;
	uint64_t ser_lane_rev                 : 1;
	uint64_t reserved_7_15                : 9;
	uint64_t ser_lane_bad                 : 4;
	uint64_t ser_lane_ready               : 4;
	uint64_t trn_ena                      : 1;
	uint64_t timer_dis                    : 1;
	uint64_t trn_rxeq_only                : 1;
	uint64_t reserved_27_30               : 4;
	uint64_t crd_dis                      : 1;
	uint64_t ser_limit                    : 10;
	uint64_t reserved_42_59               : 18;
	uint64_t ser_low                      : 4;
#endif
	} s;
	struct cvmx_ocx_qlmx_cfg_s            cn78xx;
	struct cvmx_ocx_qlmx_cfg_s            cn78xxp1;
};
typedef union cvmx_ocx_qlmx_cfg cvmx_ocx_qlmx_cfg_t;

/**
 * cvmx_ocx_rlk#_align
 */
union cvmx_ocx_rlkx_align {
	uint64_t u64;
	struct cvmx_ocx_rlkx_align_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t bad_cnt                      : 32; /**< Number of alignment sequences received in error (i.e. those that violate the current
                                                         alignment). Count saturates at max value. */
	uint64_t good_cnt                     : 32; /**< Number of alignment sequences received (i.e. those that do not violate the current
                                                         alignment). Count saturates at max value. */
#else
	uint64_t good_cnt                     : 32;
	uint64_t bad_cnt                      : 32;
#endif
	} s;
	struct cvmx_ocx_rlkx_align_s          cn78xx;
	struct cvmx_ocx_rlkx_align_s          cn78xxp1;
};
typedef union cvmx_ocx_rlkx_align cvmx_ocx_rlkx_align_t;

/**
 * cvmx_ocx_rlk#_blk_err
 */
union cvmx_ocx_rlkx_blk_err {
	uint64_t u64;
	struct cvmx_ocx_rlkx_blk_err_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t limit                        : 16; /**< Number of blocks received with errors before the OCX_COM_LINK(0..2)_INT[BLK_ERR] interrupt
                                                         is generated. */
	uint64_t count                        : 16; /**< Number of blocks received with one or more errors detected. Multiple errors may be
                                                         detected as the link starts up. */
#else
	uint64_t count                        : 16;
	uint64_t limit                        : 16;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_ocx_rlkx_blk_err_s        cn78xx;
	struct cvmx_ocx_rlkx_blk_err_s        cn78xxp1;
};
typedef union cvmx_ocx_rlkx_blk_err cvmx_ocx_rlkx_blk_err_t;

/**
 * cvmx_ocx_rlk#_ecc_ctl
 */
union cvmx_ocx_rlkx_ecc_ctl {
	uint64_t u64;
	struct cvmx_ocx_rlkx_ecc_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_36_63               : 28;
	uint64_t fifo1_flip                   : 2;  /**< Test pattern to cause ECC errors in top RX FIFO syndromes. */
	uint64_t fifo0_flip                   : 2;  /**< Test pattern to cause ECC errors in bottom RX FIFO syndromes. */
	uint64_t reserved_2_31                : 30;
	uint64_t fifo1_cdis                   : 1;  /**< ECC correction disable for top RX FIFO RAM. */
	uint64_t fifo0_cdis                   : 1;  /**< ECC correction disable for bottom RX FIFO RAM. */
#else
	uint64_t fifo0_cdis                   : 1;
	uint64_t fifo1_cdis                   : 1;
	uint64_t reserved_2_31                : 30;
	uint64_t fifo0_flip                   : 2;
	uint64_t fifo1_flip                   : 2;
	uint64_t reserved_36_63               : 28;
#endif
	} s;
	struct cvmx_ocx_rlkx_ecc_ctl_s        cn78xx;
	struct cvmx_ocx_rlkx_ecc_ctl_s        cn78xxp1;
};
typedef union cvmx_ocx_rlkx_ecc_ctl cvmx_ocx_rlkx_ecc_ctl_t;

/**
 * cvmx_ocx_rlk#_enables
 */
union cvmx_ocx_rlkx_enables {
	uint64_t u64;
	struct cvmx_ocx_rlkx_enables_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t mcd                          : 1;  /**< Master enable for all inbound MCD bits. This bit must be enabled by software. once any
                                                         Authentik validation has occurred and before any MCD traffic is generated. MCD traffic is
                                                         typically controlled by the OCX_TLK(0..2)_MCD_CTL register. */
	uint64_t m_req                        : 1;  /**< Master enable for all inbound memory requests. This bit is typically set at reset but is
                                                         cleared when operating in Authentik mode and must be enabled by software. */
	uint64_t io_req                       : 1;  /**< Master enable for all inbound I/O requests. This bit is typically set at reset but is
                                                         cleared when operating in Authentik mode and must be enabled by software. */
	uint64_t fwd                          : 1;  /**< Master enable for all inbound forward commands. This bit is typically set at reset but is
                                                         cleared when operating in Authentik mode and must be enabled by software. */
	uint64_t co_proc                      : 1;  /**< Master enable for all inbound coprocessor commands. This bit is typically set at reset but
                                                         is cleared when operating in Authentik mode and must be enabled by software. */
#else
	uint64_t co_proc                      : 1;
	uint64_t fwd                          : 1;
	uint64_t io_req                       : 1;
	uint64_t m_req                        : 1;
	uint64_t mcd                          : 1;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_ocx_rlkx_enables_s        cn78xx;
	struct cvmx_ocx_rlkx_enables_s        cn78xxp1;
};
typedef union cvmx_ocx_rlkx_enables cvmx_ocx_rlkx_enables_t;

/**
 * cvmx_ocx_rlk#_fifo#_cnt
 */
union cvmx_ocx_rlkx_fifox_cnt {
	uint64_t u64;
	struct cvmx_ocx_rlkx_fifox_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t count                        : 16; /**< RX FIFO count of 64-bit words to send to core. VC13 traffic is used immediately so the
                                                         FIFO count is always 0. (see OCX_RLK(0..2)_LNK_DATA). */
#else
	uint64_t count                        : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_ocx_rlkx_fifox_cnt_s      cn78xx;
	struct cvmx_ocx_rlkx_fifox_cnt_s      cn78xxp1;
};
typedef union cvmx_ocx_rlkx_fifox_cnt cvmx_ocx_rlkx_fifox_cnt_t;

/**
 * cvmx_ocx_rlk#_lnk_data
 */
union cvmx_ocx_rlkx_lnk_data {
	uint64_t u64;
	struct cvmx_ocx_rlkx_lnk_data_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t rcvd                         : 1;  /**< Reads state of OCX_COM_LINK(0..2)_INT[LNK_DATA]; set by hardware when a link data block is
                                                         received. */
	uint64_t reserved_60_62               : 3;
	uint64_t data                         : 60; /**< Contents of this register are received from the OCX_TLK(0..2)_LNK_DATA register on the
                                                         link partner. Each time a new value is received the RX_LDAT interrupt is generated. */
#else
	uint64_t data                         : 60;
	uint64_t reserved_60_62               : 3;
	uint64_t rcvd                         : 1;
#endif
	} s;
	struct cvmx_ocx_rlkx_lnk_data_s       cn78xx;
	struct cvmx_ocx_rlkx_lnk_data_s       cn78xxp1;
};
typedef union cvmx_ocx_rlkx_lnk_data cvmx_ocx_rlkx_lnk_data_t;

/**
 * cvmx_ocx_rlk#_mcd_ctl
 *
 * This debug register captures which new MCD bits have been received from the link partner. The
 * MCD bits are received when the both the OCX_RLK(0..2)_ENABLES[MCD] bit is set and the MCD was
 * not previously transmitted.
 */
union cvmx_ocx_rlkx_mcd_ctl {
	uint64_t u64;
	struct cvmx_ocx_rlkx_mcd_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t clr                          : 3;  /**< Inbound MCD value being driven by link(0..2). Set by hardware receiving an MCD packet and
                                                         cleared by this register. */
#else
	uint64_t clr                          : 3;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_ocx_rlkx_mcd_ctl_s        cn78xx;
	struct cvmx_ocx_rlkx_mcd_ctl_s        cn78xxp1;
};
typedef union cvmx_ocx_rlkx_mcd_ctl cvmx_ocx_rlkx_mcd_ctl_t;

/**
 * cvmx_ocx_strap
 *
 * This register provide read-only access to OCI straps.
 *
 */
union cvmx_ocx_strap {
	uint64_t u64;
	struct cvmx_ocx_strap_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_26_63               : 38;
	uint64_t oci3_lnk1                    : 1;  /**< OCI3_LNK1 strap. */
	uint64_t oci2_lnk1                    : 1;  /**< OCI2_LNK1 strap. */
	uint64_t reserved_17_23               : 7;
	uint64_t oci_fixed_node               : 1;  /**< OCI_FIXED_NODE strap. */
	uint64_t reserved_10_15               : 6;
	uint64_t oci_node_id                  : 2;  /**< OCI_NODE_ID<1:0> straps. */
	uint64_t reserved_4_7                 : 4;
	uint64_t oci_spd                      : 4;  /**< OCI_SPD<3:0> straps. */
#else
	uint64_t oci_spd                      : 4;
	uint64_t reserved_4_7                 : 4;
	uint64_t oci_node_id                  : 2;
	uint64_t reserved_10_15               : 6;
	uint64_t oci_fixed_node               : 1;
	uint64_t reserved_17_23               : 7;
	uint64_t oci2_lnk1                    : 1;
	uint64_t oci3_lnk1                    : 1;
	uint64_t reserved_26_63               : 38;
#endif
	} s;
	struct cvmx_ocx_strap_s               cn78xx;
};
typedef union cvmx_ocx_strap cvmx_ocx_strap_t;

/**
 * cvmx_ocx_tlk#_bist_status
 *
 * Contains status from last memory BIST for all TX FIFO memories and REPLAY memories in this
 * link. RX FIFO status can be found in OCX_COM_BIST_STATUS.
 */
union cvmx_ocx_tlkx_bist_status {
	uint64_t u64;
	struct cvmx_ocx_tlkx_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_15_63               : 49;
	uint64_t status                       : 15; /**< <14:13> = REPLAY Memories BIST Status <1:0>.
                                                         <12:0> = TX_FIFO<12:0> by link VC number. */
#else
	uint64_t status                       : 15;
	uint64_t reserved_15_63               : 49;
#endif
	} s;
	struct cvmx_ocx_tlkx_bist_status_s    cn78xx;
	struct cvmx_ocx_tlkx_bist_status_s    cn78xxp1;
};
typedef union cvmx_ocx_tlkx_bist_status cvmx_ocx_tlkx_bist_status_t;

/**
 * cvmx_ocx_tlk#_byp_ctl
 *
 * This register is for diagnostic use.
 *
 */
union cvmx_ocx_tlkx_byp_ctl {
	uint64_t u64;
	struct cvmx_ocx_tlkx_byp_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t vc_dis                       : 11; /**< VC bypass disable.  When set, the corresponding VC is restricted from using
                                                         the low latency TX FIFO bypass logic.  This logic is typically disabled for
                                                         VC0 only.  For diagnostic use only. */
	uint64_t reserved_0_0                 : 1;
#else
	uint64_t reserved_0_0                 : 1;
	uint64_t vc_dis                       : 11;
	uint64_t reserved_12_63               : 52;
#endif
	} s;
	struct cvmx_ocx_tlkx_byp_ctl_s        cn78xx;
};
typedef union cvmx_ocx_tlkx_byp_ctl cvmx_ocx_tlkx_byp_ctl_t;

/**
 * cvmx_ocx_tlk#_ecc_ctl
 */
union cvmx_ocx_tlkx_ecc_ctl {
	uint64_t u64;
	struct cvmx_ocx_tlkx_ecc_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_38_63               : 26;
	uint64_t rply1_flip                   : 2;  /**< Test pattern to cause ECC errors in RPLY1 RAM. */
	uint64_t rply0_flip                   : 2;  /**< Test pattern to cause ECC errors in RPLY0 RAM. */
	uint64_t fifo_flip                    : 2;  /**< Test pattern to cause ECC errors in TX FIFO RAM. */
	uint64_t reserved_3_31                : 29;
	uint64_t rply1_cdis                   : 1;  /**< ECC correction disable for replay top memories. */
	uint64_t rply0_cdis                   : 1;  /**< ECC correction disable for replay bottom memories. */
	uint64_t fifo_cdis                    : 1;  /**< ECC correction disable for TX FIFO memories. */
#else
	uint64_t fifo_cdis                    : 1;
	uint64_t rply0_cdis                   : 1;
	uint64_t rply1_cdis                   : 1;
	uint64_t reserved_3_31                : 29;
	uint64_t fifo_flip                    : 2;
	uint64_t rply0_flip                   : 2;
	uint64_t rply1_flip                   : 2;
	uint64_t reserved_38_63               : 26;
#endif
	} s;
	struct cvmx_ocx_tlkx_ecc_ctl_s        cn78xx;
	struct cvmx_ocx_tlkx_ecc_ctl_s        cn78xxp1;
};
typedef union cvmx_ocx_tlkx_ecc_ctl cvmx_ocx_tlkx_ecc_ctl_t;

/**
 * cvmx_ocx_tlk#_fifo#_cnt
 */
union cvmx_ocx_tlkx_fifox_cnt {
	uint64_t u64;
	struct cvmx_ocx_tlkx_fifox_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t count                        : 16; /**< TX FIFO count of bus cycles to send. */
#else
	uint64_t count                        : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_ocx_tlkx_fifox_cnt_s      cn78xx;
	struct cvmx_ocx_tlkx_fifox_cnt_s      cn78xxp1;
};
typedef union cvmx_ocx_tlkx_fifox_cnt cvmx_ocx_tlkx_fifox_cnt_t;

/**
 * cvmx_ocx_tlk#_lnk_data
 */
union cvmx_ocx_tlkx_lnk_data {
	uint64_t u64;
	struct cvmx_ocx_tlkx_lnk_data_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t data                         : 60; /**< Writes to this register transfer the contents to the OCX_RLK(0..2)_LNK_DATA register on
                                                         the receiving link. */
#else
	uint64_t data                         : 60;
	uint64_t reserved_60_63               : 4;
#endif
	} s;
	struct cvmx_ocx_tlkx_lnk_data_s       cn78xx;
	struct cvmx_ocx_tlkx_lnk_data_s       cn78xxp1;
};
typedef union cvmx_ocx_tlkx_lnk_data cvmx_ocx_tlkx_lnk_data_t;

/**
 * cvmx_ocx_tlk#_lnk_vc#_cnt
 */
union cvmx_ocx_tlkx_lnk_vcx_cnt {
	uint64_t u64;
	struct cvmx_ocx_tlkx_lnk_vcx_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t count                        : 16; /**< Link VC credits available for use. VC13 always reads 1 since credits are not required. */
#else
	uint64_t count                        : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_ocx_tlkx_lnk_vcx_cnt_s    cn78xx;
	struct cvmx_ocx_tlkx_lnk_vcx_cnt_s    cn78xxp1;
};
typedef union cvmx_ocx_tlkx_lnk_vcx_cnt cvmx_ocx_tlkx_lnk_vcx_cnt_t;

/**
 * cvmx_ocx_tlk#_mcd_ctl
 *
 * This register controls which MCD bits are transported via the link. For proper operation
 * only one link must be enabled in both directions between each pair of link partners.
 */
union cvmx_ocx_tlkx_mcd_ctl {
	uint64_t u64;
	struct cvmx_ocx_tlkx_mcd_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_3_63                : 61;
	uint64_t tx_enb                       : 3;  /**< Transmission enables for MCD bits <2:0>. */
#else
	uint64_t tx_enb                       : 3;
	uint64_t reserved_3_63                : 61;
#endif
	} s;
	struct cvmx_ocx_tlkx_mcd_ctl_s        cn78xx;
	struct cvmx_ocx_tlkx_mcd_ctl_s        cn78xxp1;
};
typedef union cvmx_ocx_tlkx_mcd_ctl cvmx_ocx_tlkx_mcd_ctl_t;

/**
 * cvmx_ocx_tlk#_rtn_vc#_cnt
 */
union cvmx_ocx_tlkx_rtn_vcx_cnt {
	uint64_t u64;
	struct cvmx_ocx_tlkx_rtn_vcx_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t count                        : 16; /**< Link VC credits to return. VC13 always reads 0 since credits are never returned. */
#else
	uint64_t count                        : 16;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_ocx_tlkx_rtn_vcx_cnt_s    cn78xx;
	struct cvmx_ocx_tlkx_rtn_vcx_cnt_s    cn78xxp1;
};
typedef union cvmx_ocx_tlkx_rtn_vcx_cnt cvmx_ocx_tlkx_rtn_vcx_cnt_t;

/**
 * cvmx_ocx_tlk#_stat_ctl
 */
union cvmx_ocx_tlkx_stat_ctl {
	uint64_t u64;
	struct cvmx_ocx_tlkx_stat_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_2_63                : 62;
	uint64_t clear                        : 1;  /**< Setting this bit clears all OCX_TLK(a)_STAT_*CNT, OCX_TLK(a)_STAT_*CMD,
                                                         OCX_TLK(a)_STAT_*PKT and OCX_TLK(0..2)_STAT_*CON registers. */
	uint64_t enable                       : 1;  /**< This bit controls the capture of statistics to the OCX_TLK(a)_STAT_*CNT,
                                                         OCX_TLK(a)_STAT_*CMD, OCX_TLK(a)_STAT_*PKT and OCX_TLK(a)_STAT_*CON registers. When set,
                                                         traffic will increment the corresponding registers. When cleared, traffic will be ignored. */
#else
	uint64_t enable                       : 1;
	uint64_t clear                        : 1;
	uint64_t reserved_2_63                : 62;
#endif
	} s;
	struct cvmx_ocx_tlkx_stat_ctl_s       cn78xx;
	struct cvmx_ocx_tlkx_stat_ctl_s       cn78xxp1;
};
typedef union cvmx_ocx_tlkx_stat_ctl cvmx_ocx_tlkx_stat_ctl_t;

/**
 * cvmx_ocx_tlk#_stat_data_cnt
 */
union cvmx_ocx_tlkx_stat_data_cnt {
	uint64_t u64;
	struct cvmx_ocx_tlkx_stat_data_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t count                        : 64; /**< Number of data blocks transferred over the CCPI Link while OCX_TLK(0..2)_STAT_CTL[ENABLE]
                                                         has been set. */
#else
	uint64_t count                        : 64;
#endif
	} s;
	struct cvmx_ocx_tlkx_stat_data_cnt_s  cn78xx;
	struct cvmx_ocx_tlkx_stat_data_cnt_s  cn78xxp1;
};
typedef union cvmx_ocx_tlkx_stat_data_cnt cvmx_ocx_tlkx_stat_data_cnt_t;

/**
 * cvmx_ocx_tlk#_stat_err_cnt
 */
union cvmx_ocx_tlkx_stat_err_cnt {
	uint64_t u64;
	struct cvmx_ocx_tlkx_stat_err_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t count                        : 64; /**< Number of blocks received with an error over the CCPI link while
                                                         OCX_TLK(0..2)_STAT_CTL[ENABLE] has been set. */
#else
	uint64_t count                        : 64;
#endif
	} s;
	struct cvmx_ocx_tlkx_stat_err_cnt_s   cn78xx;
	struct cvmx_ocx_tlkx_stat_err_cnt_s   cn78xxp1;
};
typedef union cvmx_ocx_tlkx_stat_err_cnt cvmx_ocx_tlkx_stat_err_cnt_t;

/**
 * cvmx_ocx_tlk#_stat_idle_cnt
 */
union cvmx_ocx_tlkx_stat_idle_cnt {
	uint64_t u64;
	struct cvmx_ocx_tlkx_stat_idle_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t count                        : 64; /**< Number of idle blocks transferred over the CCPI link while OCX_TLK(0..2)_STAT_CTL[ENABLE]
                                                         has been set. */
#else
	uint64_t count                        : 64;
#endif
	} s;
	struct cvmx_ocx_tlkx_stat_idle_cnt_s  cn78xx;
	struct cvmx_ocx_tlkx_stat_idle_cnt_s  cn78xxp1;
};
typedef union cvmx_ocx_tlkx_stat_idle_cnt cvmx_ocx_tlkx_stat_idle_cnt_t;

/**
 * cvmx_ocx_tlk#_stat_mat#_cnt
 */
union cvmx_ocx_tlkx_stat_matx_cnt {
	uint64_t u64;
	struct cvmx_ocx_tlkx_stat_matx_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t count                        : 64; /**< Number of packets that have matched OCX_TLK(a)_STAT_MATCH0 and have been transferred over
                                                         the CCPI link while OCX_TLK(0..2)_STAT_CTL[ENABLE] has been set. */
#else
	uint64_t count                        : 64;
#endif
	} s;
	struct cvmx_ocx_tlkx_stat_matx_cnt_s  cn78xx;
	struct cvmx_ocx_tlkx_stat_matx_cnt_s  cn78xxp1;
};
typedef union cvmx_ocx_tlkx_stat_matx_cnt cvmx_ocx_tlkx_stat_matx_cnt_t;

/**
 * cvmx_ocx_tlk#_stat_match#
 */
union cvmx_ocx_tlkx_stat_matchx {
	uint64_t u64;
	struct cvmx_ocx_tlkx_stat_matchx_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_25_63               : 39;
	uint64_t mask                         : 9;  /**< Setting these bits mask (really matches) the corresponding bit comparison for each packet. */
	uint64_t reserved_9_15                : 7;
	uint64_t cmd                          : 5;  /**< These bits are compared against the command for each packet sent over the link. If both
                                                         the unmasked VC and CMD bits match then OCX_TLK()_STAT_MAT()_CNT is incremented. */
	uint64_t vc                           : 4;  /**< These bits are compared against the link VC number for each packet sent over the link.
                                                         If both the unmasked VC and CMD bits match, then OCX_TLK()_STAT_MAT()_CNT is
                                                         incremented.  Only memory and I/O traffic are monitored.  Matches are limited to
                                                         VC0 through VC11. */
#else
	uint64_t vc                           : 4;
	uint64_t cmd                          : 5;
	uint64_t reserved_9_15                : 7;
	uint64_t mask                         : 9;
	uint64_t reserved_25_63               : 39;
#endif
	} s;
	struct cvmx_ocx_tlkx_stat_matchx_s    cn78xx;
	struct cvmx_ocx_tlkx_stat_matchx_s    cn78xxp1;
};
typedef union cvmx_ocx_tlkx_stat_matchx cvmx_ocx_tlkx_stat_matchx_t;

/**
 * cvmx_ocx_tlk#_stat_retry_cnt
 */
union cvmx_ocx_tlkx_stat_retry_cnt {
	uint64_t u64;
	struct cvmx_ocx_tlkx_stat_retry_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t count                        : 64; /**< Number of data blocks repeated over the CCPI link while OCX_TLK(0..2)_STAT_CTL[ENABLE] has
                                                         been set. */
#else
	uint64_t count                        : 64;
#endif
	} s;
	struct cvmx_ocx_tlkx_stat_retry_cnt_s cn78xx;
	struct cvmx_ocx_tlkx_stat_retry_cnt_s cn78xxp1;
};
typedef union cvmx_ocx_tlkx_stat_retry_cnt cvmx_ocx_tlkx_stat_retry_cnt_t;

/**
 * cvmx_ocx_tlk#_stat_sync_cnt
 */
union cvmx_ocx_tlkx_stat_sync_cnt {
	uint64_t u64;
	struct cvmx_ocx_tlkx_stat_sync_cnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t count                        : 64; /**< Number of sync (control) blocks transferred over the CCPI link while
                                                         OCX_TLK(0..2)_STAT_CTL[ENABLE] has been set. */
#else
	uint64_t count                        : 64;
#endif
	} s;
	struct cvmx_ocx_tlkx_stat_sync_cnt_s  cn78xx;
	struct cvmx_ocx_tlkx_stat_sync_cnt_s  cn78xxp1;
};
typedef union cvmx_ocx_tlkx_stat_sync_cnt cvmx_ocx_tlkx_stat_sync_cnt_t;

/**
 * cvmx_ocx_tlk#_stat_vc#_cmd
 */
union cvmx_ocx_tlkx_stat_vcx_cmd {
	uint64_t u64;
	struct cvmx_ocx_tlkx_stat_vcx_cmd_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t count                        : 64; /**< Number of commands on this VC that have been transfered over the CCPI link while
                                                         OCX_TLK(0..2)_STAT_CTL[ENABLE] has been set. For VCs 6 through 13 the number of commands
                                                         is equal to the number of packets. */
#else
	uint64_t count                        : 64;
#endif
	} s;
	struct cvmx_ocx_tlkx_stat_vcx_cmd_s   cn78xx;
	struct cvmx_ocx_tlkx_stat_vcx_cmd_s   cn78xxp1;
};
typedef union cvmx_ocx_tlkx_stat_vcx_cmd cvmx_ocx_tlkx_stat_vcx_cmd_t;

/**
 * cvmx_ocx_tlk#_stat_vc#_con
 */
union cvmx_ocx_tlkx_stat_vcx_con {
	uint64_t u64;
	struct cvmx_ocx_tlkx_stat_vcx_con_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t count                        : 64; /**< Number of conflicts on this VC while OCX_TLK(0..2)_STAT_CTL[ENABLE] has been set. A
                                                         conflict is indicated when a VC has one or more packets to send and no link credits are
                                                         available. VC13 does not require credits so no conflicts are ever indicated (i.e. reads
                                                         0). */
#else
	uint64_t count                        : 64;
#endif
	} s;
	struct cvmx_ocx_tlkx_stat_vcx_con_s   cn78xx;
	struct cvmx_ocx_tlkx_stat_vcx_con_s   cn78xxp1;
};
typedef union cvmx_ocx_tlkx_stat_vcx_con cvmx_ocx_tlkx_stat_vcx_con_t;

/**
 * cvmx_ocx_tlk#_stat_vc#_pkt
 */
union cvmx_ocx_tlkx_stat_vcx_pkt {
	uint64_t u64;
	struct cvmx_ocx_tlkx_stat_vcx_pkt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t count                        : 64; /**< Number of packets on this VC that have been transferred over the CCPI link while
                                                         OCX_TLK(0..2)_STAT_CTL[ENABLE] has been set. */
#else
	uint64_t count                        : 64;
#endif
	} s;
	struct cvmx_ocx_tlkx_stat_vcx_pkt_s   cn78xx;
	struct cvmx_ocx_tlkx_stat_vcx_pkt_s   cn78xxp1;
};
typedef union cvmx_ocx_tlkx_stat_vcx_pkt cvmx_ocx_tlkx_stat_vcx_pkt_t;

/**
 * cvmx_ocx_tlk#_status
 */
union cvmx_ocx_tlkx_status {
	uint64_t u64;
	struct cvmx_ocx_tlkx_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_56_63               : 8;
	uint64_t rply_fptr                    : 8;  /**< Replay buffer last free pointer. */
	uint64_t tx_seq                       : 8;  /**< Last block transmitted. */
	uint64_t rx_seq                       : 8;  /**< Last block received. */
	uint64_t reserved_23_31               : 9;
	uint64_t ackcnt                       : 7;  /**< Number of ACKs waiting to be transmitted. */
	uint64_t reserved_9_15                : 7;
	uint64_t drop                         : 1;  /**< Link is dropping all requests. */
	uint64_t sm                           : 6;  /**< Block state machine:
                                                         Bit<2>: Req / Ack (Init or retry only).
                                                         Bit<3>: Init.
                                                         Bit<4>: Run.
                                                         Bit<5>: Retry.
                                                         Bit<6>: Replay.
                                                         Bit<7>: Replay Pending. */
	uint64_t cnt                          : 2;  /**< Block subcount. Should always increment 0,1,2,3,0.. except during TX PHY stall. */
#else
	uint64_t cnt                          : 2;
	uint64_t sm                           : 6;
	uint64_t drop                         : 1;
	uint64_t reserved_9_15                : 7;
	uint64_t ackcnt                       : 7;
	uint64_t reserved_23_31               : 9;
	uint64_t rx_seq                       : 8;
	uint64_t tx_seq                       : 8;
	uint64_t rply_fptr                    : 8;
	uint64_t reserved_56_63               : 8;
#endif
	} s;
	struct cvmx_ocx_tlkx_status_s         cn78xx;
	struct cvmx_ocx_tlkx_status_s         cn78xxp1;
};
typedef union cvmx_ocx_tlkx_status cvmx_ocx_tlkx_status_t;

/**
 * cvmx_ocx_win_cmd
 *
 * For diagnostic use only. This register is typically written by hardware after accesses to the
 * SLI_WIN_* registers. Contains the address, read size and write mask to used for the window
 * operation. Write data should be written first and placed in the OCX_WIN_WR_DATA register.
 * Writing this register starts the operation. A second write to this register while an operation
 * is in progress will stall.
 */
union cvmx_ocx_win_cmd {
	uint64_t u64;
	struct cvmx_ocx_win_cmd_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t wr_mask                      : 8;  /**< Mask for the data to be written. When a bit is 1, the corresponding byte will be written.
                                                         The values of this field must be contiguous and for 1, 2, 4, or 8 byte operations and
                                                         aligned to operation size. A value of 0 will produce unpredictable results. Field is
                                                         ignored during a read (LD_OP=1). */
	uint64_t reserved_51_55               : 5;
	uint64_t ld_cmd                       : 2;  /**< The load command sent with the read:
                                                         0x0 = Load 1-bytes.
                                                         0x1 = Load 2-bytes.
                                                         0x2 = Load 4-bytes.
                                                         0x3 = Load 8-bytes. */
	uint64_t ld_op                        : 1;  /**< Operation Type 0=Store 1=Load operation. */
	uint64_t addr                         : 48; /**< The address used in both the load and store operations:
                                                         <47:40> = NCB_ID.
                                                         <39:38> = 0, Not used.
                                                         <37:36> = CCPI_ID.
                                                         <35:0> = Address.
                                                         When <47:43> == SLI & <42:40> == 0 bits <39:0> are:
                                                         <39:38> = 0, Not used.
                                                         <37:36> = CCPI_ID.
                                                         <35:32> = 0, Not used.
                                                         <31:24> = RSL_ID.
                                                         <23:0> = RSL register offset.
                                                         <2:0> are ignored in a store operation. */
#else
	uint64_t addr                         : 48;
	uint64_t ld_op                        : 1;
	uint64_t ld_cmd                       : 2;
	uint64_t reserved_51_55               : 5;
	uint64_t wr_mask                      : 8;
#endif
	} s;
	struct cvmx_ocx_win_cmd_s             cn78xx;
	struct cvmx_ocx_win_cmd_s             cn78xxp1;
};
typedef union cvmx_ocx_win_cmd cvmx_ocx_win_cmd_t;

/**
 * cvmx_ocx_win_rd_data
 *
 * For diagnostic use only. This register is the read response data associated with window
 * command. Reads all-ones until response is received.
 */
union cvmx_ocx_win_rd_data {
	uint64_t u64;
	struct cvmx_ocx_win_rd_data_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t data                         : 64; /**< Read response data. */
#else
	uint64_t data                         : 64;
#endif
	} s;
	struct cvmx_ocx_win_rd_data_s         cn78xx;
	struct cvmx_ocx_win_rd_data_s         cn78xxp1;
};
typedef union cvmx_ocx_win_rd_data cvmx_ocx_win_rd_data_t;

/**
 * cvmx_ocx_win_timer
 *
 * Number of core clocks before untransmitted WIN request is dropped and interrupt is issued.
 *
 */
union cvmx_ocx_win_timer {
	uint64_t u64;
	struct cvmx_ocx_win_timer_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_16_63               : 48;
	uint64_t tout                         : 14; /**< Number of core clocks times four. */
	uint64_t tout1                        : 2;  /**< Reserved as all-ones. */
#else
	uint64_t tout1                        : 2;
	uint64_t tout                         : 14;
	uint64_t reserved_16_63               : 48;
#endif
	} s;
	struct cvmx_ocx_win_timer_s           cn78xx;
	struct cvmx_ocx_win_timer_s           cn78xxp1;
};
typedef union cvmx_ocx_win_timer cvmx_ocx_win_timer_t;

/**
 * cvmx_ocx_win_wr_data
 *
 * For diagnostic use only. This register is typically written by hardware after accesses to the
 * SLI_WIN_WR_DATA register. Contains the data to write to the address located in the OCX_WIN_CMD
 * Register.
 */
union cvmx_ocx_win_wr_data {
	uint64_t u64;
	struct cvmx_ocx_win_wr_data_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t wr_data                      : 64; /**< The data to be written. */
#else
	uint64_t wr_data                      : 64;
#endif
	} s;
	struct cvmx_ocx_win_wr_data_s         cn78xx;
	struct cvmx_ocx_win_wr_data_s         cn78xxp1;
};
typedef union cvmx_ocx_win_wr_data cvmx_ocx_win_wr_data_t;

#endif
