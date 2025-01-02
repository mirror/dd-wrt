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
 * cvmx-uctlx-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon uctlx.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_UCTLX_DEFS_H__
#define __CVMX_UCTLX_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UCTLX_BIST_STATUS(unsigned long offset)
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			if ((offset == 0))
				return CVMX_ADD_IO_SEG(0x000118006F0000A0ull);
			break;
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				if ((offset == 0))
					return CVMX_ADD_IO_SEG(0x0001180068000008ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				if ((offset == 0))
					return CVMX_ADD_IO_SEG(0x0001180068000008ull);

			break;
	}
	cvmx_warn("CVMX_UCTLX_BIST_STATUS (offset = %lu) not supported on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118006F0000A0ull);
}
#else
static inline uint64_t CVMX_UCTLX_BIST_STATUS(unsigned long offset __attribute__ ((unused)))
{
	switch(cvmx_get_octeon_family()) {
		case OCTEON_CN66XX & OCTEON_FAMILY_MASK:
		case OCTEON_CNF71XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN61XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN63XX & OCTEON_FAMILY_MASK:
		case OCTEON_CN68XX & OCTEON_FAMILY_MASK:
			return CVMX_ADD_IO_SEG(0x000118006F0000A0ull);
		case OCTEON_CN78XX & OCTEON_FAMILY_MASK:
			if (OCTEON_IS_MODEL(OCTEON_CN78XX))
				return CVMX_ADD_IO_SEG(0x0001180068000008ull);
			if (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X))
				return CVMX_ADD_IO_SEG(0x0001180068000008ull);

	}
	return CVMX_ADD_IO_SEG(0x000118006F0000A0ull);
}
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UCTLX_CLK_RST_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UCTLX_CLK_RST_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118006F000000ull);
}
#else
#define CVMX_UCTLX_CLK_RST_CTL(offset) (CVMX_ADD_IO_SEG(0x000118006F000000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UCTLX_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UCTLX_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180068000000ull);
}
#else
#define CVMX_UCTLX_CTL(offset) (CVMX_ADD_IO_SEG(0x0001180068000000ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UCTLX_ECC(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UCTLX_ECC(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800680000F0ull);
}
#else
#define CVMX_UCTLX_ECC(offset) (CVMX_ADD_IO_SEG(0x00011800680000F0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UCTLX_EHCI_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UCTLX_EHCI_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118006F000080ull);
}
#else
#define CVMX_UCTLX_EHCI_CTL(offset) (CVMX_ADD_IO_SEG(0x000118006F000080ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UCTLX_EHCI_FLA(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UCTLX_EHCI_FLA(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118006F0000A8ull);
}
#else
#define CVMX_UCTLX_EHCI_FLA(offset) (CVMX_ADD_IO_SEG(0x000118006F0000A8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UCTLX_ERTO_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UCTLX_ERTO_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118006F000090ull);
}
#else
#define CVMX_UCTLX_ERTO_CTL(offset) (CVMX_ADD_IO_SEG(0x000118006F000090ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UCTLX_HOST_CFG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UCTLX_HOST_CFG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800680000E0ull);
}
#else
#define CVMX_UCTLX_HOST_CFG(offset) (CVMX_ADD_IO_SEG(0x00011800680000E0ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UCTLX_IF_ENA(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UCTLX_IF_ENA(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118006F000030ull);
}
#else
#define CVMX_UCTLX_IF_ENA(offset) (CVMX_ADD_IO_SEG(0x000118006F000030ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UCTLX_INTSTAT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UCTLX_INTSTAT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180068000030ull);
}
#else
#define CVMX_UCTLX_INTSTAT(offset) (CVMX_ADD_IO_SEG(0x0001180068000030ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UCTLX_INT_ENA(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UCTLX_INT_ENA(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118006F000028ull);
}
#else
#define CVMX_UCTLX_INT_ENA(offset) (CVMX_ADD_IO_SEG(0x000118006F000028ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UCTLX_INT_REG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UCTLX_INT_REG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118006F000020ull);
}
#else
#define CVMX_UCTLX_INT_REG(offset) (CVMX_ADD_IO_SEG(0x000118006F000020ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UCTLX_OHCI_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UCTLX_OHCI_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118006F000088ull);
}
#else
#define CVMX_UCTLX_OHCI_CTL(offset) (CVMX_ADD_IO_SEG(0x000118006F000088ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UCTLX_ORTO_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UCTLX_ORTO_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118006F000098ull);
}
#else
#define CVMX_UCTLX_ORTO_CTL(offset) (CVMX_ADD_IO_SEG(0x000118006F000098ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UCTLX_PORTX_CFG_HS(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset == 0)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_UCTLX_PORTX_CFG_HS(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180068000040ull);
}
#else
#define CVMX_UCTLX_PORTX_CFG_HS(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180068000040ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UCTLX_PORTX_CFG_SS(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset == 0)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_UCTLX_PORTX_CFG_SS(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180068000048ull);
}
#else
#define CVMX_UCTLX_PORTX_CFG_SS(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180068000048ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UCTLX_PORTX_CR_DBG_CFG(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset == 0)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_UCTLX_PORTX_CR_DBG_CFG(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180068000050ull);
}
#else
#define CVMX_UCTLX_PORTX_CR_DBG_CFG(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180068000050ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UCTLX_PORTX_CR_DBG_STATUS(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && (((offset == 0)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && (((offset == 0)) && ((block_id == 0))))))
		cvmx_warn("CVMX_UCTLX_PORTX_CR_DBG_STATUS(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x0001180068000058ull);
}
#else
#define CVMX_UCTLX_PORTX_CR_DBG_STATUS(offset, block_id) (CVMX_ADD_IO_SEG(0x0001180068000058ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UCTLX_PPAF_WM(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UCTLX_PPAF_WM(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118006F000038ull);
}
#else
#define CVMX_UCTLX_PPAF_WM(offset) (CVMX_ADD_IO_SEG(0x000118006F000038ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UCTLX_SHIM_CFG(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UCTLX_SHIM_CFG(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800680000E8ull);
}
#else
#define CVMX_UCTLX_SHIM_CFG(offset) (CVMX_ADD_IO_SEG(0x00011800680000E8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UCTLX_SPARE0(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UCTLX_SPARE0(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001180068000010ull);
}
#else
#define CVMX_UCTLX_SPARE0(offset) (CVMX_ADD_IO_SEG(0x0001180068000010ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UCTLX_SPARE1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset == 0)))))
		cvmx_warn("CVMX_UCTLX_SPARE1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x00011800680000F8ull);
}
#else
#define CVMX_UCTLX_SPARE1(offset) (CVMX_ADD_IO_SEG(0x00011800680000F8ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UCTLX_UPHY_CTL_STATUS(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && ((offset == 0)))))
		cvmx_warn("CVMX_UCTLX_UPHY_CTL_STATUS(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x000118006F000008ull);
}
#else
#define CVMX_UCTLX_UPHY_CTL_STATUS(offset) (CVMX_ADD_IO_SEG(0x000118006F000008ull))
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_UCTLX_UPHY_PORTX_CTL_STATUS(unsigned long offset, unsigned long block_id)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && (((offset <= 1)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && (((offset <= 1)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && (((offset <= 1)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && (((offset <= 1)) && ((block_id == 0)))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF71XX) && (((offset <= 1)) && ((block_id == 0))))))
		cvmx_warn("CVMX_UCTLX_UPHY_PORTX_CTL_STATUS(%lu,%lu) is invalid on this chip\n", offset, block_id);
	return CVMX_ADD_IO_SEG(0x000118006F000010ull) + (((offset) & 1) + ((block_id) & 0) * 0x0ull) * 8;
}
#else
#define CVMX_UCTLX_UPHY_PORTX_CTL_STATUS(offset, block_id) (CVMX_ADD_IO_SEG(0x000118006F000010ull) + (((offset) & 1) + ((block_id) & 0) * 0x0ull) * 8)
#endif

/**
 * cvmx_uctl#_bist_status
 *
 * This register indicates the results from the built-in self-test (BIST) runs of USBH memories.
 * A 0 indicates pass or never run, a 1 indicates fail. This register can be reset by IOI reset.
 */
union cvmx_uctlx_bist_status {
	uint64_t u64;
	struct cvmx_uctlx_bist_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_42_63               : 22;
	uint64_t uctl_xm_r_bist_ndone         : 1;  /**< BIST is not complete for the UCTL AxiMaster read-data FIFO. */
	uint64_t uctl_xm_w_bist_ndone         : 1;  /**< BIST is not complete for the UCTL AxiMaster write-data FIFO. */
	uint64_t reserved_35_39               : 5;
	uint64_t uahc_ram2_bist_ndone         : 1;  /**< BIST is not complete for the UAHC RxFIFO RAM (RAM2). */
	uint64_t uahc_ram1_bist_ndone         : 1;  /**< BIST is not complete for the UAHC TxFIFO RAM (RAM1). */
	uint64_t uahc_ram0_bist_ndone         : 1;  /**< BIST is not complete for the UAHC descriptor/register cache (RAM0). */
	uint64_t reserved_10_31               : 22;
	uint64_t uctl_xm_r_bist_status        : 1;  /**< BIST status of the UCTL AxiMaster read-data FIFO. */
	uint64_t uctl_xm_w_bist_status        : 1;  /**< BIST status of the UCTL AxiMaster write-data FIFO. */
	uint64_t reserved_6_7                 : 2;
	uint64_t data_bis                     : 1;  /**< UAHC EHCI Data Ram Bist Status */
	uint64_t desc_bis                     : 1;  /**< UAHC EHCI Descriptor Ram Bist Status */
	uint64_t erbm_bis                     : 1;  /**< UCTL EHCI Read Buffer Memory Bist Status */
	uint64_t reserved_0_2                 : 3;
#else
	uint64_t reserved_0_2                 : 3;
	uint64_t erbm_bis                     : 1;
	uint64_t desc_bis                     : 1;
	uint64_t data_bis                     : 1;
	uint64_t reserved_6_7                 : 2;
	uint64_t uctl_xm_w_bist_status        : 1;
	uint64_t uctl_xm_r_bist_status        : 1;
	uint64_t reserved_10_31               : 22;
	uint64_t uahc_ram0_bist_ndone         : 1;
	uint64_t uahc_ram1_bist_ndone         : 1;
	uint64_t uahc_ram2_bist_ndone         : 1;
	uint64_t reserved_35_39               : 5;
	uint64_t uctl_xm_w_bist_ndone         : 1;
	uint64_t uctl_xm_r_bist_ndone         : 1;
	uint64_t reserved_42_63               : 22;
#endif
	} s;
	struct cvmx_uctlx_bist_status_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t data_bis                     : 1;  /**< UAHC EHCI Data Ram Bist Status */
	uint64_t desc_bis                     : 1;  /**< UAHC EHCI Descriptor Ram Bist Status */
	uint64_t erbm_bis                     : 1;  /**< UCTL EHCI Read Buffer Memory Bist Status */
	uint64_t orbm_bis                     : 1;  /**< UCTL OHCI Read Buffer Memory Bist Status */
	uint64_t wrbm_bis                     : 1;  /**< UCTL Write Buffer Memory Bist Sta */
	uint64_t ppaf_bis                     : 1;  /**< PP Access FIFO Memory Bist Status */
#else
	uint64_t ppaf_bis                     : 1;
	uint64_t wrbm_bis                     : 1;
	uint64_t orbm_bis                     : 1;
	uint64_t erbm_bis                     : 1;
	uint64_t desc_bis                     : 1;
	uint64_t data_bis                     : 1;
	uint64_t reserved_6_63                : 58;
#endif
	} cn61xx;
	struct cvmx_uctlx_bist_status_cn61xx  cn63xx;
	struct cvmx_uctlx_bist_status_cn61xx  cn63xxp1;
	struct cvmx_uctlx_bist_status_cn61xx  cn66xx;
	struct cvmx_uctlx_bist_status_cn61xx  cn68xx;
	struct cvmx_uctlx_bist_status_cn61xx  cn68xxp1;
	struct cvmx_uctlx_bist_status_cn78xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_42_63               : 22;
	uint64_t uctl_xm_r_bist_ndone         : 1;  /**< BIST is not complete for the UCTL AxiMaster read-data FIFO. */
	uint64_t uctl_xm_w_bist_ndone         : 1;  /**< BIST is not complete for the UCTL AxiMaster write-data FIFO. */
	uint64_t reserved_35_39               : 5;
	uint64_t uahc_ram2_bist_ndone         : 1;  /**< BIST is not complete for the UAHC RxFIFO RAM (RAM2). */
	uint64_t uahc_ram1_bist_ndone         : 1;  /**< BIST is not complete for the UAHC TxFIFO RAM (RAM1). */
	uint64_t uahc_ram0_bist_ndone         : 1;  /**< BIST is not complete for the UAHC descriptor/register cache (RAM0). */
	uint64_t reserved_10_31               : 22;
	uint64_t uctl_xm_r_bist_status        : 1;  /**< BIST status of the UCTL AxiMaster read-data FIFO. */
	uint64_t uctl_xm_w_bist_status        : 1;  /**< BIST status of the UCTL AxiMaster write-data FIFO. */
	uint64_t reserved_3_7                 : 5;
	uint64_t uahc_ram2_bist_status        : 1;  /**< BIST status of the UAHC RxFIFO RAM (RAM2). */
	uint64_t uahc_ram1_bist_status        : 1;  /**< BIST status of the UAHC TxFIFO RAM (RAM1). */
	uint64_t uahc_ram0_bist_status        : 1;  /**< BIST status of the UAHC descriptor/register cache (RAM0). */
#else
	uint64_t uahc_ram0_bist_status        : 1;
	uint64_t uahc_ram1_bist_status        : 1;
	uint64_t uahc_ram2_bist_status        : 1;
	uint64_t reserved_3_7                 : 5;
	uint64_t uctl_xm_w_bist_status        : 1;
	uint64_t uctl_xm_r_bist_status        : 1;
	uint64_t reserved_10_31               : 22;
	uint64_t uahc_ram0_bist_ndone         : 1;
	uint64_t uahc_ram1_bist_ndone         : 1;
	uint64_t uahc_ram2_bist_ndone         : 1;
	uint64_t reserved_35_39               : 5;
	uint64_t uctl_xm_w_bist_ndone         : 1;
	uint64_t uctl_xm_r_bist_ndone         : 1;
	uint64_t reserved_42_63               : 22;
#endif
	} cn78xx;
	struct cvmx_uctlx_bist_status_cn78xx  cn78xxp1;
	struct cvmx_uctlx_bist_status_cn61xx  cnf71xx;
};
typedef union cvmx_uctlx_bist_status cvmx_uctlx_bist_status_t;

/**
 * cvmx_uctl#_clk_rst_ctl
 *
 * CLK_RST_CTL = Clock and Reset Control Reigster
 * This register controls the frequceny of hclk and resets for hclk and phy clocks. It also controls Simulation modes and Bists.
 */
union cvmx_uctlx_clk_rst_ctl {
	uint64_t u64;
	struct cvmx_uctlx_clk_rst_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_25_63               : 39;
	uint64_t clear_bist                   : 1;  /**< Clear BIST on the HCLK memories */
	uint64_t start_bist                   : 1;  /**< Starts BIST on the HCLK memories during 0-to-1
                                                         transition. */
	uint64_t ehci_sm                      : 1;  /**< Only set it during simulation time. When set to 1,
                                                         this bit sets the PHY in a non-driving mode so the
                                                         EHCI can detect device connection.
                                                         Note: it must not be set to 1, during normal
                                                         operation. */
	uint64_t ohci_clkcktrst               : 1;  /**< Clear clock reset. Active low.  OHCI initial reset
                                                         signal for the DPLL block. This is only needed by
                                                         simulation. The duration of the reset  in simulation
                                                         must be the same as HRST.
                                                         Note: it must be set to 1 during normal operation. */
	uint64_t ohci_sm                      : 1;  /**< OHCI Simulation Mode. It selects the counter value
                                                          for simulation or real time for 1 ms.
                                                         - 0: counter full 1ms; 1: simulation time. */
	uint64_t ohci_susp_lgcy               : 1;  /**< OHCI Clock Control Signal. Note: This bit must be
                                                         set to 0 if the OHCI 48/12Mhz clocks must be
                                                         suspended when the EHCI and OHCI controllers are
                                                         not active. */
	uint64_t app_start_clk                : 1;  /**< OHCI Clock Control Signal. When the OHCI clocks are
                                                         suspended, the system has to assert this signal to
                                                         start the clocks (12 and 48 Mhz). */
	uint64_t o_clkdiv_rst                 : 1;  /**< OHCI 12Mhz  clock divider reset. Active low. When
                                                         set to 0, divider is held in reset.
                                                         The reset to the divider is also asserted when core
                                                         reset is asserted. */
	uint64_t h_clkdiv_byp                 : 1;  /**< Used to enable the bypass input to the USB_CLK_DIV */
	uint64_t h_clkdiv_rst                 : 1;  /**< Host clock divider reset. Active low. When set to 0,
                                                         divider is held in reset. This must be set to 0
                                                         before change H_DIV0 and H_DIV1.
                                                         The reset to the divider is also asserted when core
                                                         reset is asserted. */
	uint64_t h_clkdiv_en                  : 1;  /**< Hclk enable. When set to 1, the hclk is gernerated. */
	uint64_t o_clkdiv_en                  : 1;  /**< OHCI 48Mhz/12MHz clock enable. When set to 1, the
                                                         clocks are gernerated. */
	uint64_t h_div                        : 4;  /**< The hclk frequency is sclk frequency divided by
                                                         H_DIV. The maximum frequency of hclk is 200Mhz.
                                                         The minimum frequency of hclk is no less than the
                                                         UTMI clock frequency which is 60Mhz. After writing a
                                                         value to this field, the software should read the
                                                         field for the value written. The [H_ENABLE] field of
                                                         this register should not be set until after this
                                                         field is set and  then read.
                                                         Only the following values are valid:
                                                            1, 2, 3, 4, 6, 8, 12.
                                                         All other values are reserved and will be coded as
                                                         following:
                                                            0        -> 1
                                                            5        -> 4
                                                            7        -> 6
                                                            9,10,11  -> 8
                                                            13,14,15 -> 12 */
	uint64_t p_refclk_sel                 : 2;  /**< PHY PLL Reference Clock Select.
                                                         - 00: uses 12Mhz crystal at USB_XO and USB_XI;
                                                         - 01: uses 12/24/48Mhz 2.5V clock source at USB_XO.
                                                             USB_XI should be tied to GND(Not Supported).
                                                         1x: Reserved. */
	uint64_t p_refclk_div                 : 2;  /**< PHY Reference Clock Frequency Select.
                                                           - 00: 12MHz,
                                                           - 01: 24Mhz (Not Supported),
                                                           - 10: 48Mhz (Not Supported),
                                                           - 11: Reserved.
                                                         Note: This value must be set during POR is active.
                                                         If a crystal is used as a reference clock,this field
                                                         must be set to 12 MHz. Values 01 and 10 are reserved
                                                         when a crystal is used. */
	uint64_t reserved_4_4                 : 1;
	uint64_t p_com_on                     : 1;  /**< PHY Common Block Power-Down Control.
                                                         - 1: The XO, Bias, and PLL blocks are powered down in
                                                             Suspend mode.
                                                         - 0: The XO, Bias, and PLL blocks remain powered in
                                                             suspend mode.
                                                          Note: This bit must be set to 0 during POR is active
                                                          in current design. */
	uint64_t p_por                        : 1;  /**< Power on reset for PHY. Resets all the PHY's
                                                         registers and state machines. */
	uint64_t p_prst                       : 1;  /**< PHY Clock Reset. The is the value for phy_rst_n,
                                                         utmi_rst_n[1] and utmi_rst_n[0]. It is synchronized
                                                         to each clock domain to generate the corresponding
                                                         reset signal. This should not be set to 1 until the
                                                         time it takes for six clock cycles (HCLK and
                                                         PHY CLK, which ever is slower) has passed. */
	uint64_t hrst                         : 1;  /**< Host Clock Reset. This is the value for hreset_n.
                                                         This should not be set to 1 until 12ms after PHY CLK
                                                         is stable. */
#else
	uint64_t hrst                         : 1;
	uint64_t p_prst                       : 1;
	uint64_t p_por                        : 1;
	uint64_t p_com_on                     : 1;
	uint64_t reserved_4_4                 : 1;
	uint64_t p_refclk_div                 : 2;
	uint64_t p_refclk_sel                 : 2;
	uint64_t h_div                        : 4;
	uint64_t o_clkdiv_en                  : 1;
	uint64_t h_clkdiv_en                  : 1;
	uint64_t h_clkdiv_rst                 : 1;
	uint64_t h_clkdiv_byp                 : 1;
	uint64_t o_clkdiv_rst                 : 1;
	uint64_t app_start_clk                : 1;
	uint64_t ohci_susp_lgcy               : 1;
	uint64_t ohci_sm                      : 1;
	uint64_t ohci_clkcktrst               : 1;
	uint64_t ehci_sm                      : 1;
	uint64_t start_bist                   : 1;
	uint64_t clear_bist                   : 1;
	uint64_t reserved_25_63               : 39;
#endif
	} s;
	struct cvmx_uctlx_clk_rst_ctl_s       cn61xx;
	struct cvmx_uctlx_clk_rst_ctl_s       cn63xx;
	struct cvmx_uctlx_clk_rst_ctl_s       cn63xxp1;
	struct cvmx_uctlx_clk_rst_ctl_s       cn66xx;
	struct cvmx_uctlx_clk_rst_ctl_s       cn68xx;
	struct cvmx_uctlx_clk_rst_ctl_s       cn68xxp1;
	struct cvmx_uctlx_clk_rst_ctl_s       cnf71xx;
};
typedef union cvmx_uctlx_clk_rst_ctl cvmx_uctlx_clk_rst_ctl_t;

/**
 * cvmx_uctl#_ctl
 *
 * This register controls clocks, resets, power, and BIST.
 *
 * This register can be reset by IOI reset.
 */
union cvmx_uctlx_ctl {
	uint64_t u64;
	struct cvmx_uctlx_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t clear_bist                   : 1;  /**< BIST fast-clear mode select. A BIST run with this bit set clears all entries in USBH RAMs
                                                         to 0x0.
                                                         There are two major modes of BIST: full and clear. Full BIST is run by the BIST state
                                                         machine when CLEAR_BIST is deasserted during BIST. Clear BIST is run if CLEAR_BIST is
                                                         asserted during BIST.
                                                         To avoid race conditions, software must first perform a CSR write operation that puts the
                                                         CLEAR_BIST setting into the correct state and then perform another CSR write operation to
                                                         set the BIST trigger (keeping the CLEAR_BIST state constant).
                                                         CLEAR BIST completion is indicated by UCTL()_BIST_STATUS. A BIST clear operation
                                                         takes almost 2,000 controller-clock cycles for the largest RAM. */
	uint64_t start_bist                   : 1;  /**< Rising edge starts BIST on the memories in USBH.
                                                         To run BIST, the controller clock must be both configured and enabled, and should be
                                                         configured to the maximum available frequency given the available coprocessor clock and
                                                         dividers.
                                                         Also, the UCTL, UAHC, and UPHY should be held in software- initiated reset (using
                                                         UPHY_RST, UAHC_RST, UCTL_RST) until BIST is complete.
                                                         BIST defect status can be checked after FULL BIST completion, both of which are indicated
                                                         in UCTL()_BIST_STATUS. The full BIST run takes almost 80,000 controller-clock cycles for
                                                         the largest RAM. */
	uint64_t ref_clk_sel                  : 2;  /**< Reference clock select. Choose reference-clock source for the SuperSpeed and high-speed
                                                         PLL blocks.
                                                         0x0 = Reference clock source for both PLLs come from the USB pads.
                                                         0x1 = Reserved.
                                                         0x2 = Reserved.
                                                         0x3 = Reserved.
                                                         This value can be changed only during UPHY_RST.
                                                         If REF_CLK_SEL = 0x0, then the reference clock input cannot be spread-spectrum. */
	uint64_t ssc_en                       : 1;  /**< Spread-spectrum clock enable. Enables spread-spectrum clock production in the SuperSpeed
                                                         function. If the input reference clock for the SuperSpeed PLL is already spread-spectrum,
                                                         then do not enable this feature. The clocks sourced to the SuperSpeed function must have
                                                         spread-spectrum to be compliant with the USB specification.
                                                         The high-speed PLL cannot support a spread-spectrum input, so REF_CLK_SEL = 0x0 must
                                                         enable this feature.
                                                         This value may only be changed during [UPHY_RST]. */
	uint64_t ssc_range                    : 3;  /**< Spread-spectrum clock range. Selects the range of spread-spectrum modulation when SSC_EN
                                                         is asserted and the PHY is spreading the SuperSpeed transmit clocks.
                                                         Applies a fixed offset to the phase accumulator.
                                                         0x0 = -4980 ppm downspread of clock.
                                                         0x1 = -4492 ppm.
                                                         0x2 = -4003 ppm.
                                                         0x3-0x7 = reserved.
                                                         All of these settings are within the USB 3.0 specification. The amount of EMI emission
                                                         reduction might decrease as the [SSC_RANGE] increases; therefore, the [SSC_RANGE] settings
                                                         can
                                                         be registered to enable the amount of spreading to be adjusted on a per-application basis.
                                                         This value can be changed only during UPHY_RST. */
	uint64_t ssc_ref_clk_sel              : 9;  /**< Enables non-standard oscillator frequencies to generate targeted MPLL output rates. Input
                                                         corresponds to the frequency-synthesis coefficient.
                                                         [55:53]: modulus - 1,
                                                         [52:47]: 2's complement push amount
                                                         Must leave at reset value of 0x0.
                                                         This value may only be changed during [UPHY_RST]. */
	uint64_t mpll_multiplier              : 7;  /**< Multiplies the reference clock to a frequency suitable for intended operating speed. Must
                                                         leave at reset value of 0x0. This value may only be changed during UPHY_RST.
                                                         This value is superseded by the REF_CLK_FSEL<5:3> selection. */
	uint64_t ref_ssp_en                   : 1;  /**< Enables reference clock to the prescaler for SuperSpeed function. This should always be
                                                         enabled since this output clock is used to drive the UAHC suspend-mode clock during
                                                         low-power states.
                                                         This value can be changed only during UPHY_RST or during low-power states.
                                                         The reference clock must be running and stable before UPHY_RST is deasserted and before
                                                         [REF_SSP_EN] is asserted. */
	uint64_t ref_clk_div2                 : 1;  /**< Divides the reference clock by 2 before feeding it into the REF_CLK_FSEL divider. Must
                                                         leave at reset value of 0x0.
                                                         This value can be changed only during UPHY_RST. */
	uint64_t ref_clk_fsel                 : 6;  /**< Selects the reference clock frequency for the SuperSpeed and high-speed PLL blocks. The
                                                         legal values are as follows:
                                                         0x27 = External reference clock 100 MHz.
                                                         All other values are reserved.
                                                         This value may only be changed during [UPHY_RST]. */
	uint64_t reserved_31_31               : 1;
	uint64_t h_clk_en                     : 1;  /**< Controller-clock enable. When set to 1, the controller clock is generated. This also
                                                         enables access to UCTL registers 0x30-0xF8. */
	uint64_t h_clk_byp_sel                : 1;  /**< Select the bypass input to the controller-clock divider.
                                                         0 = Use the divided coprocessor clock from the H_CLKDIV divider.
                                                         1 = Use the bypass clock from the GPIO pins.
                                                         This signal is just a multiplexer-select signal; it does not enable the controller clock.
                                                         You must still set H_CLK_EN separately. H_CLK_BYP_SEL select should not be changed
                                                         unless H_CLK_EN is disabled.
                                                         The bypass clock can be selected and running even if the controller-clock dividers are not
                                                         running. */
	uint64_t h_clkdiv_rst                 : 1;  /**< Controller clock divider reset. Divided clocks are not generated while the divider is
                                                         being reset.
                                                         This also resets the suspend-clock divider. */
	uint64_t reserved_27_27               : 1;
	uint64_t h_clkdiv_sel                 : 3;  /**< Controller clock-frequency-divider select. The controller-clock frequency is the
                                                         coprocessor-clock frequency divided by [H_CLKDIV_SEL] and must be at or below 300 MHz.
                                                         The divider values are the following:
                                                         0x0 = divide by 1.
                                                         0x1 = divide by 2.
                                                         0x2 = divide by 4.
                                                         0x3 = divide by 6.
                                                         0x4 = divide by 8.
                                                         0x5 = divide by 16.
                                                         0x6 = divide by 24.
                                                         0x7 = divide by 32.
                                                         For USB3:
                                                         * The controller-clock frequency must be at or above 125 MHz for any USB3 operation.
                                                         * The controller-clock frequency must be at or above
                                                         150 MHz for full-rate USB3 operation.
                                                         For USB2:
                                                         * The controller-clock frequency must be at or above 62.5 MHz for any USB2 operation.
                                                         * The controller-clock frequency must be at or above
                                                         90 MHz for full-rate USB2 operation.
                                                         This field can be changed only when H_CLKDIV_RST = 1. */
	uint64_t reserved_22_23               : 2;
	uint64_t usb3_port_perm_attach        : 1;  /**< Indicates this port is permanently attached. This is a strap signal; it should be modified
                                                         only when [UPHY_RST] is asserted. */
	uint64_t usb2_port_perm_attach        : 1;  /**< Indicates this port is permanently attached. This is a strap signal; it should be modified
                                                         only when [UPHY_RST] is asserted. */
	uint64_t reserved_19_19               : 1;
	uint64_t usb3_port_disable            : 1;  /**< Disables the USB3 (SuperSpeed) portion of this PHY. When set to 1, this signal stops
                                                         reporting connect/disconnect events on the port and keeps the port in disabled state. This
                                                         could be used for security reasons where hardware can disable a port regardless of whether
                                                         xHCI driver enables a port or not.
                                                         UAHC()_HCSPARAMS1[MAXPORTS] is not affected by this signal.
                                                         This is a strap signal; it should be modified only when [UPHY_RST] is asserted. */
	uint64_t reserved_17_17               : 1;
	uint64_t usb2_port_disable            : 1;  /**< Disables USB2 (high-speed/full-speed/low-speed) portion of this PHY. When set to 1, this
                                                         signal stops reporting connect/disconnect events on the port and keeps the port in
                                                         disabled state. This could be used for security reasons where hardware can disable a port
                                                         regardless of whether xHCI driver enables a port or not.
                                                         UAHC()_HCSPARAMS1[MAXPORTS] is not affected by this signal.
                                                         This is a strap signal; it should only be modified when [UPHY_RST] is asserted.
                                                         If Port0 is required to be disabled, ensure that the utmi_clk[0] is running at the normal
                                                         speed. Also, all the enabled USB2.0 ports should have the same clock frequency as Port0. */
	uint64_t reserved_15_15               : 1;
	uint64_t ss_power_en                  : 1;  /**< PHY SuperSpeed block power enable.
                                                         This is a strap signal; it should only be modified when [UPHY_RST] is asserted. */
	uint64_t reserved_13_13               : 1;
	uint64_t hs_power_en                  : 1;  /**< PHY high-speed block power enable.
                                                         This is a strap signal; it should only be modified when [UPHY_RST] is asserted. */
	uint64_t reserved_5_11                : 7;
	uint64_t csclk_en                     : 1;  /**< Turns on the USB UCTL interface clock (coprocessor clock). This enables access to UAHC
                                                         registers via the IOI, as well as UCTL registers starting from 0x30 via the RSL bus. */
	uint64_t reserved_3_3                 : 1;
	uint64_t uphy_rst                     : 1;  /**< PHY reset; resets UPHY; active-high. */
	uint64_t uahc_rst                     : 1;  /**< Software reset; resets UAHC; active-high. */
	uint64_t uctl_rst                     : 1;  /**< Software reset; resets UCTL; active-high.
                                                         Resets UAHC DMA and register shims. Resets UCTL RSL registers 0x30-0xF8.
                                                         Does not reset UCTL RSL registers 0x0-0x28.
                                                         UCTL RSL registers starting from 0x30 can be accessed only after the controller clock is
                                                         active and [UCTL_RST] is deasserted. */
#else
	uint64_t uctl_rst                     : 1;
	uint64_t uahc_rst                     : 1;
	uint64_t uphy_rst                     : 1;
	uint64_t reserved_3_3                 : 1;
	uint64_t csclk_en                     : 1;
	uint64_t reserved_5_11                : 7;
	uint64_t hs_power_en                  : 1;
	uint64_t reserved_13_13               : 1;
	uint64_t ss_power_en                  : 1;
	uint64_t reserved_15_15               : 1;
	uint64_t usb2_port_disable            : 1;
	uint64_t reserved_17_17               : 1;
	uint64_t usb3_port_disable            : 1;
	uint64_t reserved_19_19               : 1;
	uint64_t usb2_port_perm_attach        : 1;
	uint64_t usb3_port_perm_attach        : 1;
	uint64_t reserved_22_23               : 2;
	uint64_t h_clkdiv_sel                 : 3;
	uint64_t reserved_27_27               : 1;
	uint64_t h_clkdiv_rst                 : 1;
	uint64_t h_clk_byp_sel                : 1;
	uint64_t h_clk_en                     : 1;
	uint64_t reserved_31_31               : 1;
	uint64_t ref_clk_fsel                 : 6;
	uint64_t ref_clk_div2                 : 1;
	uint64_t ref_ssp_en                   : 1;
	uint64_t mpll_multiplier              : 7;
	uint64_t ssc_ref_clk_sel              : 9;
	uint64_t ssc_range                    : 3;
	uint64_t ssc_en                       : 1;
	uint64_t ref_clk_sel                  : 2;
	uint64_t start_bist                   : 1;
	uint64_t clear_bist                   : 1;
#endif
	} s;
	struct cvmx_uctlx_ctl_s               cn78xx;
	struct cvmx_uctlx_ctl_s               cn78xxp1;
};
typedef union cvmx_uctlx_ctl cvmx_uctlx_ctl_t;

/**
 * cvmx_uctl#_ecc
 *
 * This register can be used to disable ECC correction, insert ECC errors, and debug ECC
 * failures.
 * * The ECC_ERR* fields are captured when there are no outstanding ECC errors indicated in
 * INTSTAT and a new ECC error arrives. Prioritization for multiple events occurring on the same
 * cycle is indicated by the ECC_ERR_SOURCE enumeration: highest encoded value has highest
 * priority.
 * * The *ECC_*_DIS fields disable ECC correction; SBE and DBE errors are still reported. If
 * *ECC_*_DIS = 0x1, then no data-correction occurs.
 * * The *ECC_FLIP_SYND fields flip the syndrome<1:0> bits to generate single-bit/double-bit
 * error for testing.
 *
 * 0x0 = Normal operation.
 * 0x1 = SBE on bit[0].
 * 0x2 = SBE on bit[1].
 * 0x3 = DBE on bit[1:0].
 *
 * This register is accessible only when UCTL()_CTL[H_CLK_EN] = 1.
 *
 * This register can be reset by IOI reset or with UCTL()_CTL[UCTL_RST].
 */
union cvmx_uctlx_ecc {
	uint64_t u64;
	struct cvmx_uctlx_ecc_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t ecc_err_source               : 4;  /**< Source of ECC error, see UCTL_ECC_ERR_SOURCE_E. */
	uint64_t ecc_err_syndrome             : 8;  /**< Syndrome bits of the ECC error. */
	uint64_t ecc_err_address              : 16; /**< RAM address of the ECC error. */
	uint64_t reserved_21_31               : 11;
	uint64_t uctl_xm_r_ecc_flip_synd      : 2;  /**< Insert ECC error for testing purposes. */
	uint64_t uctl_xm_r_ecc_cor_dis        : 1;  /**< Enables ECC correction on UCTL AxiMaster read-data FIFO. */
	uint64_t uctl_xm_w_ecc_flip_synd      : 2;  /**< Insert ECC error for testing purposes. */
	uint64_t uctl_xm_w_ecc_cor_dis        : 1;  /**< Enables ECC correction on UCTL AxiMaster write-data FIFO. */
	uint64_t reserved_9_14                : 6;
	uint64_t uahc_ram2_ecc_flip_synd      : 2;  /**< Insert ECC error for testing purposes. */
	uint64_t uahc_ram2_ecc_cor_dis        : 1;  /**< Enables ECC correction on UAHC RxFIFO RAMs (RAM2). */
	uint64_t uahc_ram1_ecc_flip_synd      : 2;  /**< Insert ECC error for testing purposes. */
	uint64_t uahc_ram1_ecc_cor_dis        : 1;  /**< Enables ECC correction on UAHC TxFIFO RAMs (RAM1). */
	uint64_t uahc_ram0_ecc_flip_synd      : 2;  /**< Insert ECC error for testing purposes. */
	uint64_t uahc_ram0_ecc_cor_dis        : 1;  /**< Enables ECC correction on UAHC Desc/Reg cache (RAM0). */
#else
	uint64_t uahc_ram0_ecc_cor_dis        : 1;
	uint64_t uahc_ram0_ecc_flip_synd      : 2;
	uint64_t uahc_ram1_ecc_cor_dis        : 1;
	uint64_t uahc_ram1_ecc_flip_synd      : 2;
	uint64_t uahc_ram2_ecc_cor_dis        : 1;
	uint64_t uahc_ram2_ecc_flip_synd      : 2;
	uint64_t reserved_9_14                : 6;
	uint64_t uctl_xm_w_ecc_cor_dis        : 1;
	uint64_t uctl_xm_w_ecc_flip_synd      : 2;
	uint64_t uctl_xm_r_ecc_cor_dis        : 1;
	uint64_t uctl_xm_r_ecc_flip_synd      : 2;
	uint64_t reserved_21_31               : 11;
	uint64_t ecc_err_address              : 16;
	uint64_t ecc_err_syndrome             : 8;
	uint64_t ecc_err_source               : 4;
	uint64_t reserved_60_63               : 4;
#endif
	} s;
	struct cvmx_uctlx_ecc_s               cn78xx;
	struct cvmx_uctlx_ecc_s               cn78xxp1;
};
typedef union cvmx_uctlx_ecc cvmx_uctlx_ecc_t;

/**
 * cvmx_uctl#_ehci_ctl
 *
 * UCTL_EHCI_CTL = UCTL EHCI Control Register
 * This register controls the general behavior of UCTL EHCI datapath.
 */
union cvmx_uctlx_ehci_ctl {
	uint64_t u64;
	struct cvmx_uctlx_ehci_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t desc_rbm                     : 1;  /**< Descriptor Read Burst Mode on AHB bus
                                                         - 1: A read burst can be interruprted after 16 AHB
                                                             clock cycle
                                                         - 0: A read burst will not be interrupted until it
                                                             finishes or no more data available */
	uint64_t reg_nb                       : 1;  /**< 1: EHCI register access will not be blocked by EHCI
                                                          buffer/descriptor access on AHB
                                                         - 0: Buffer/descriptor and register access will be
                                                             mutually exclusive */
	uint64_t l2c_dc                       : 1;  /**< When set to 1, set the commit bit in the descriptor
                                                         store commands to L2C. */
	uint64_t l2c_bc                       : 1;  /**< When set to 1, set the commit bit in the buffer
                                                         store commands to L2C. */
	uint64_t l2c_0pag                     : 1;  /**< When set to 1, sets the zero-page bit in store
                                                         command to  L2C. */
	uint64_t l2c_stt                      : 1;  /**< When set to 1, use STT when store to L2C. */
	uint64_t l2c_buff_emod                : 2;  /**< Endian format for buffer from/to the L2C.
                                                         IN:       A-B-C-D-E-F-G-H
                                                         OUT0:  A-B-C-D-E-F-G-H
                                                         OUT1:  H-G-F-E-D-C-B-A
                                                         OUT2:  D-C-B-A-H-G-F-E
                                                         OUT3:  E-F-G-H-A-B-C-D */
	uint64_t l2c_desc_emod                : 2;  /**< Endian format for descriptor from/to the L2C.
                                                         IN:        A-B-C-D-E-F-G-H
                                                         OUT0:  A-B-C-D-E-F-G-H
                                                         OUT1:  H-G-F-E-D-C-B-A
                                                         OUT2:  D-C-B-A-H-G-F-E
                                                         OUT3:  E-F-G-H-A-B-C-D */
	uint64_t inv_reg_a2                   : 1;  /**< UAHC register address  bit<2> invert. When set to 1,
                                                         for a 32-bit NCB I/O register access, the address
                                                         offset will be flipped between 0x4 and 0x0. */
	uint64_t ehci_64b_addr_en             : 1;  /**< EHCI AHB Master 64-bit Addressing Enable.
                                                         - 1: enable ehci 64-bit addressing mode;
                                                         - 0: disable ehci 64-bit addressing mode.
                                                          When ehci 64-bit addressing mode is disabled,
                                                          UCTL_EHCI_CTL[L2C_ADDR_MSB] is used as the address
                                                          bit[39:32]. */
	uint64_t l2c_addr_msb                 : 8;  /**< This is the bit [39:32] of an address sent to L2C
                                                         for ehci whenUCTL_EHCI_CFG[EHCI_64B_ADDR_EN=0]). */
#else
	uint64_t l2c_addr_msb                 : 8;
	uint64_t ehci_64b_addr_en             : 1;
	uint64_t inv_reg_a2                   : 1;
	uint64_t l2c_desc_emod                : 2;
	uint64_t l2c_buff_emod                : 2;
	uint64_t l2c_stt                      : 1;
	uint64_t l2c_0pag                     : 1;
	uint64_t l2c_bc                       : 1;
	uint64_t l2c_dc                       : 1;
	uint64_t reg_nb                       : 1;
	uint64_t desc_rbm                     : 1;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_uctlx_ehci_ctl_s          cn61xx;
	struct cvmx_uctlx_ehci_ctl_s          cn63xx;
	struct cvmx_uctlx_ehci_ctl_s          cn63xxp1;
	struct cvmx_uctlx_ehci_ctl_s          cn66xx;
	struct cvmx_uctlx_ehci_ctl_s          cn68xx;
	struct cvmx_uctlx_ehci_ctl_s          cn68xxp1;
	struct cvmx_uctlx_ehci_ctl_s          cnf71xx;
};
typedef union cvmx_uctlx_ehci_ctl cvmx_uctlx_ehci_ctl_t;

/**
 * cvmx_uctl#_ehci_fla
 *
 * UCTL_EHCI_FLA = UCTL EHCI Frame Length Adjument Register
 * This register configures the EHCI Frame Length Adjustment.
 */
union cvmx_uctlx_ehci_fla {
	uint64_t u64;
	struct cvmx_uctlx_ehci_fla_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t fla                          : 6;  /**< EHCI Frame Length Adjustment. This feature
                                                         adjusts any offset from the clock source that drives
                                                         the uSOF counter.  The uSOF cycle time (number of
                                                         uSOF counter clock periods to generate a uSOF
                                                         microframe length) is equal to 59,488 plus this value.
                                                         The default value is 32(0x20), which gives an SOF cycle
                                                         time of 60,000 (each microframe has 60,000 bit times).
                                                         -------------------------------------------------
                                                          Frame Length (decimal)      FLA Value
                                                         -------------------------------------------------
                                                            59488                      0x00
                                                            59504                      0x01
                                                            59520                      0x02
                                                            ... ...
                                                            59984                      0x1F
                                                            60000                      0x20
                                                            60016                      0x21
                                                            ... ...
                                                            60496                      0x3F
                                                         --------------------------------------------------
                                                         Note: keep this value to 0x20 (decimal 32) for no
                                                         offset. */
#else
	uint64_t fla                          : 6;
	uint64_t reserved_6_63                : 58;
#endif
	} s;
	struct cvmx_uctlx_ehci_fla_s          cn61xx;
	struct cvmx_uctlx_ehci_fla_s          cn63xx;
	struct cvmx_uctlx_ehci_fla_s          cn63xxp1;
	struct cvmx_uctlx_ehci_fla_s          cn66xx;
	struct cvmx_uctlx_ehci_fla_s          cn68xx;
	struct cvmx_uctlx_ehci_fla_s          cn68xxp1;
	struct cvmx_uctlx_ehci_fla_s          cnf71xx;
};
typedef union cvmx_uctlx_ehci_fla cvmx_uctlx_ehci_fla_t;

/**
 * cvmx_uctl#_erto_ctl
 *
 * UCTL_ERTO_CTL = UCTL EHCI Readbuffer TimeOut Control Register
 * This register controls timeout for EHCI Readbuffer.
 */
union cvmx_uctlx_erto_ctl {
	uint64_t u64;
	struct cvmx_uctlx_erto_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t to_val                       : 27; /**< Read buffer timeout value
                                                         (value 0 means timeout disabled) */
	uint64_t reserved_0_4                 : 5;
#else
	uint64_t reserved_0_4                 : 5;
	uint64_t to_val                       : 27;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_uctlx_erto_ctl_s          cn61xx;
	struct cvmx_uctlx_erto_ctl_s          cn63xx;
	struct cvmx_uctlx_erto_ctl_s          cn63xxp1;
	struct cvmx_uctlx_erto_ctl_s          cn66xx;
	struct cvmx_uctlx_erto_ctl_s          cn68xx;
	struct cvmx_uctlx_erto_ctl_s          cn68xxp1;
	struct cvmx_uctlx_erto_ctl_s          cnf71xx;
};
typedef union cvmx_uctlx_erto_ctl cvmx_uctlx_erto_ctl_t;

/**
 * cvmx_uctl#_host_cfg
 *
 * This register allows configuration of various host controller (UAHC) features. Most of these
 * are strap signals and should be modified only while the controller is not running.
 *
 * This register is accessible only when UCTL()_CTL[H_CLK_EN] = 1.
 *
 * This register can be reset by IOI reset or with UCTL()_CTL[UCTL_RST].
 */
union cvmx_uctlx_host_cfg {
	uint64_t u64;
	struct cvmx_uctlx_host_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t host_current_belt            : 12; /**< This signal indicates the minimum value of all received BELT values and the BELT that is
                                                         set by the Set LTV command. */
	uint64_t reserved_38_47               : 10;
	uint64_t fla                          : 6;  /**< High-speed jitter adjustment. Indicates the correction required to accommodate mac3 clock
                                                         and utmi clock jitter to measure 125us duration. With FLA tied to 0x0, the high-speed
                                                         125us micro-frame is counted for 123933ns. The value needs to be programmed in terms of
                                                         high-speed bit times in a 30 MHz cycle. Default value that needs to be driven is 0x20
                                                         (assuming 30 MHz perfect clock).
                                                         FLA connects to the FLADJ register defined in the xHCI spec in the PCI configuration
                                                         space. Each count is equal to 16 high-speed bit times. By default when this register is
                                                         set to 0x20, it gives 125us interval. Now, based on the clock accuracy, you can decrement
                                                         the count or increment the count to get the 125 us uSOF window.
                                                         This is a strap signal; it should only be modified when UAHC is in reset (soft-reset
                                                         okay). */
	uint64_t reserved_29_31               : 3;
	uint64_t bme                          : 1;  /**< Bus-master enable. This signal is used to disable the bus-mastering capability of the
                                                         host. Disabling this capability stalls DMA accesses. */
	uint64_t oci_en                       : 1;  /**< Overcurrent-indication enable. When enabled, OCI input to UAHC is taken from the GPIO
                                                         signals and sense-converted based on OCI_ACTIVE_HIGH_EN. The MIO GPIO multiplexer must be
                                                         programmed accordingly.
                                                         When disabled, OCI input to UAHC is forced to the correct inactive state based on
                                                         OCI_ACTIVE_HIGH_EN.
                                                         This is a strap signal; it should only be modified when UAHC is in reset (soft-reset
                                                         okay). */
	uint64_t oci_active_high_en           : 1;  /**< Overcurrent sense selection. The off-chip sense (high/low) is converted to match the
                                                         controller's active-high sense.
                                                         0 = Overcurrent indication from off-chip source is active-low.
                                                         1 = Overcurrent indication from off-chip source is active-high.
                                                         This is a strap signal; it should only be modified when UAHC is in reset (soft-reset
                                                         okay). */
	uint64_t ppc_en                       : 1;  /**< Port-power-control enable.
                                                         0 = UAHC()_HCCPARAMS[PPC] report port-power-control feature is unavailable.
                                                         1 = UAHC()_HCCPARAMS[PPC] reports port-power-control feature is available. PPC output
                                                         from UAHC is taken to the GPIO signals and sense-converted based on PPC_ACTIVE_HIGH_EN.
                                                         The MIO GPIO multiplexer must be programmed accordingly.
                                                         This is a strap signal; it should only be modified when either the UCTL_CTL[UAHC] or
                                                         UAHC_GCTL[CoreSoftReset] is asserted. */
	uint64_t ppc_active_high_en           : 1;  /**< Port power control sense selection. The active-high port-power-control output to off-chip
                                                         source is converted to match the off-chip sense.
                                                         0 = Port-power control to off-chip source is active-low.
                                                         1 = Port-power control to off-chip source is active-high.
                                                         This is a strap signal; it should only be modified when either the UCTL_CTL[UAHC] or
                                                         UAHC_GCTL[CoreSoftReset] is asserted. */
	uint64_t reserved_0_23                : 24;
#else
	uint64_t reserved_0_23                : 24;
	uint64_t ppc_active_high_en           : 1;
	uint64_t ppc_en                       : 1;
	uint64_t oci_active_high_en           : 1;
	uint64_t oci_en                       : 1;
	uint64_t bme                          : 1;
	uint64_t reserved_29_31               : 3;
	uint64_t fla                          : 6;
	uint64_t reserved_38_47               : 10;
	uint64_t host_current_belt            : 12;
	uint64_t reserved_60_63               : 4;
#endif
	} s;
	struct cvmx_uctlx_host_cfg_s          cn78xx;
	struct cvmx_uctlx_host_cfg_s          cn78xxp1;
};
typedef union cvmx_uctlx_host_cfg cvmx_uctlx_host_cfg_t;

/**
 * cvmx_uctl#_if_ena
 *
 * UCTL_IF_ENA = UCTL Interface Enable Register
 *
 * Register to enable the uctl interface clock.
 */
union cvmx_uctlx_if_ena {
	uint64_t u64;
	struct cvmx_uctlx_if_ena_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_1_63                : 63;
	uint64_t en                           : 1;  /**< Turns on the USB UCTL interface clock */
#else
	uint64_t en                           : 1;
	uint64_t reserved_1_63                : 63;
#endif
	} s;
	struct cvmx_uctlx_if_ena_s            cn61xx;
	struct cvmx_uctlx_if_ena_s            cn63xx;
	struct cvmx_uctlx_if_ena_s            cn63xxp1;
	struct cvmx_uctlx_if_ena_s            cn66xx;
	struct cvmx_uctlx_if_ena_s            cn68xx;
	struct cvmx_uctlx_if_ena_s            cn68xxp1;
	struct cvmx_uctlx_if_ena_s            cnf71xx;
};
typedef union cvmx_uctlx_if_ena cvmx_uctlx_if_ena_t;

/**
 * cvmx_uctl#_int_ena
 *
 * UCTL_INT_ENA = UCTL Interrupt Enable Register
 *
 * Register to enable individual interrupt source in corresponding to UCTL_INT_REG
 */
union cvmx_uctlx_int_ena {
	uint64_t u64;
	struct cvmx_uctlx_int_ena_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t ec_ovf_e                     : 1;  /**< Ehci Commit OVerFlow Error */
	uint64_t oc_ovf_e                     : 1;  /**< Ohci Commit OVerFlow Error */
	uint64_t wb_pop_e                     : 1;  /**< Write Buffer FIFO Poped When Empty */
	uint64_t wb_psh_f                     : 1;  /**< Write Buffer FIFO Pushed When Full */
	uint64_t cf_psh_f                     : 1;  /**< Command FIFO Pushed When Full */
	uint64_t or_psh_f                     : 1;  /**< OHCI Read Buffer FIFO Pushed When Full */
	uint64_t er_psh_f                     : 1;  /**< EHCI Read Buffer FIFO Pushed When Full */
	uint64_t pp_psh_f                     : 1;  /**< PP Access FIFO  Pushed When Full */
#else
	uint64_t pp_psh_f                     : 1;
	uint64_t er_psh_f                     : 1;
	uint64_t or_psh_f                     : 1;
	uint64_t cf_psh_f                     : 1;
	uint64_t wb_psh_f                     : 1;
	uint64_t wb_pop_e                     : 1;
	uint64_t oc_ovf_e                     : 1;
	uint64_t ec_ovf_e                     : 1;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_uctlx_int_ena_s           cn61xx;
	struct cvmx_uctlx_int_ena_s           cn63xx;
	struct cvmx_uctlx_int_ena_s           cn63xxp1;
	struct cvmx_uctlx_int_ena_s           cn66xx;
	struct cvmx_uctlx_int_ena_s           cn68xx;
	struct cvmx_uctlx_int_ena_s           cn68xxp1;
	struct cvmx_uctlx_int_ena_s           cnf71xx;
};
typedef union cvmx_uctlx_int_ena cvmx_uctlx_int_ena_t;

/**
 * cvmx_uctl#_int_reg
 *
 * UCTL_INT_REG = UCTL Interrupt Register
 *
 * Summary of different bits of RSL interrupt status.
 */
union cvmx_uctlx_int_reg {
	uint64_t u64;
	struct cvmx_uctlx_int_reg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t ec_ovf_e                     : 1;  /**< Ehci Commit OVerFlow Error
                                                         When the error happenes, the whole NCB system needs
                                                         to be reset. */
	uint64_t oc_ovf_e                     : 1;  /**< Ohci Commit OVerFlow Error
                                                         When the error happenes, the whole NCB system needs
                                                         to be reset. */
	uint64_t wb_pop_e                     : 1;  /**< Write Buffer FIFO Poped When Empty */
	uint64_t wb_psh_f                     : 1;  /**< Write Buffer FIFO Pushed When Full */
	uint64_t cf_psh_f                     : 1;  /**< Command FIFO Pushed When Full */
	uint64_t or_psh_f                     : 1;  /**< OHCI Read Buffer FIFO Pushed When Full */
	uint64_t er_psh_f                     : 1;  /**< EHCI Read Buffer FIFO Pushed When Full */
	uint64_t pp_psh_f                     : 1;  /**< PP Access FIFO  Pushed When Full */
#else
	uint64_t pp_psh_f                     : 1;
	uint64_t er_psh_f                     : 1;
	uint64_t or_psh_f                     : 1;
	uint64_t cf_psh_f                     : 1;
	uint64_t wb_psh_f                     : 1;
	uint64_t wb_pop_e                     : 1;
	uint64_t oc_ovf_e                     : 1;
	uint64_t ec_ovf_e                     : 1;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_uctlx_int_reg_s           cn61xx;
	struct cvmx_uctlx_int_reg_s           cn63xx;
	struct cvmx_uctlx_int_reg_s           cn63xxp1;
	struct cvmx_uctlx_int_reg_s           cn66xx;
	struct cvmx_uctlx_int_reg_s           cn68xx;
	struct cvmx_uctlx_int_reg_s           cn68xxp1;
	struct cvmx_uctlx_int_reg_s           cnf71xx;
};
typedef union cvmx_uctlx_int_reg cvmx_uctlx_int_reg_t;

/**
 * cvmx_uctl#_intstat
 *
 * This register provides a summary of different bits of RSL interrupts. DBEs are detected and
 * SBE are corrected. For debugging output for ECC DBEs/SBEs, see UCTL()_ECC. This register can
 * be reset by IOI reset.
 */
union cvmx_uctlx_intstat {
	uint64_t u64;
	struct cvmx_uctlx_intstat_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_30_63               : 34;
	uint64_t xm_r_dbe                     : 1;  /**< Detected double-bit error on the UCTL AxiMaster read-data FIFO. */
	uint64_t xm_r_sbe                     : 1;  /**< Detected single-bit error on the UCTL AxiMaster read-data FIFO. */
	uint64_t xm_w_dbe                     : 1;  /**< Detected double-bit error on the UCTL AxiMaster write-data FIFO. */
	uint64_t xm_w_sbe                     : 1;  /**< Detected single-bit error on the UCTL AxiMaster write-data FIFO. */
	uint64_t reserved_22_25               : 4;
	uint64_t ram2_dbe                     : 1;  /**< Detected double-bit error on the UAHC RxFIFO RAMs (RAM2). */
	uint64_t ram2_sbe                     : 1;  /**< Detected single-bit error on the UAHC RxFIFO RAMs (RAM2). */
	uint64_t ram1_dbe                     : 1;  /**< Detected double-bit error on the UAHC TxFIFO RAMs (RAM1). */
	uint64_t ram1_sbe                     : 1;  /**< Detected single-bit error on the UAHC TxFIFO RAMs (RAM1). */
	uint64_t ram0_dbe                     : 1;  /**< Detected double-bit error on the UAHC Desc/Reg Cache (RAM0). */
	uint64_t ram0_sbe                     : 1;  /**< Detected single-bit error on the UAHC Desc/Reg Cache (RAM0). */
	uint64_t reserved_3_15                : 13;
	uint64_t xm_bad_dma                   : 1;  /**< Detected bad DMA access from UAHC to IOI. Error information is logged in
                                                         UCTL()_SHIM_CFG[XM_BAD_DMA_*]. Received a DMA request from UAHC that violates the
                                                         assumptions made by the AXI-to-IOI shim. Such scenarios include: illegal length/size
                                                         combinations and address out-of-bounds.
                                                         For more information on exact failures, see the description in
                                                         UCTL()_SHIM_CFG[XM_BAD_DMA_TYPE]. The hardware does not translate the request correctly
                                                         and results may violate IOI protocols. */
	uint64_t xs_ncb_oob                   : 1;  /**< Detected out-of-bound register access to UAHC over IOI. The UAHC defines 1MB of register
                                                         space, starting at offset 0x0. Any accesses outside of this register space cause this bit
                                                         to be set to 1. Error information is logged in UCTL()_SHIM_CFG[XS_NCB_OOB_*]. */
	uint64_t reserved_0_0                 : 1;
#else
	uint64_t reserved_0_0                 : 1;
	uint64_t xs_ncb_oob                   : 1;
	uint64_t xm_bad_dma                   : 1;
	uint64_t reserved_3_15                : 13;
	uint64_t ram0_sbe                     : 1;
	uint64_t ram0_dbe                     : 1;
	uint64_t ram1_sbe                     : 1;
	uint64_t ram1_dbe                     : 1;
	uint64_t ram2_sbe                     : 1;
	uint64_t ram2_dbe                     : 1;
	uint64_t reserved_22_25               : 4;
	uint64_t xm_w_sbe                     : 1;
	uint64_t xm_w_dbe                     : 1;
	uint64_t xm_r_sbe                     : 1;
	uint64_t xm_r_dbe                     : 1;
	uint64_t reserved_30_63               : 34;
#endif
	} s;
	struct cvmx_uctlx_intstat_s           cn78xx;
	struct cvmx_uctlx_intstat_s           cn78xxp1;
};
typedef union cvmx_uctlx_intstat cvmx_uctlx_intstat_t;

/**
 * cvmx_uctl#_ohci_ctl
 *
 * RSL registers starting from 0x10 can be accessed only after hclk is active and hreset is deasserted.
 *
 * UCTL_OHCI_CTL = UCTL OHCI Control Register
 * This register controls the general behavior of UCTL OHCI datapath.
 */
union cvmx_uctlx_ohci_ctl {
	uint64_t u64;
	struct cvmx_uctlx_ohci_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_19_63               : 45;
	uint64_t reg_nb                       : 1;  /**< 1: OHCI register access will not be blocked by EHCI
                                                          buffer/descriptor access on AHB
                                                         - 0: Buffer/descriptor and register access will be
                                                             mutually exclusive */
	uint64_t l2c_dc                       : 1;  /**< When set to 1, set the commit bit in the descriptor
                                                         store commands to L2C. */
	uint64_t l2c_bc                       : 1;  /**< When set to 1, set the commit bit in the buffer
                                                         store commands to L2C. */
	uint64_t l2c_0pag                     : 1;  /**< When set to 1, sets the zero-page bit in store
                                                         command to  L2C. */
	uint64_t l2c_stt                      : 1;  /**< When set to 1, use STT when store to L2C. */
	uint64_t l2c_buff_emod                : 2;  /**< Endian format for buffer from/to the L2C.
                                                         IN:       A-B-C-D-E-F-G-H
                                                         OUT0:  A-B-C-D-E-F-G-H
                                                         OUT1:  H-G-F-E-D-C-B-A
                                                         OUT2:  D-C-B-A-H-G-F-E
                                                         OUT3:  E-F-G-H-A-B-C-D */
	uint64_t l2c_desc_emod                : 2;  /**< Endian format for descriptor from/to the L2C.
                                                         IN:        A-B-C-D-E-F-G-H
                                                         OUT0:  A-B-C-D-E-F-G-H
                                                         OUT1:  H-G-F-E-D-C-B-A
                                                         OUT2:  D-C-B-A-H-G-F-E
                                                         OUT3:  E-F-G-H-A-B-C-D */
	uint64_t inv_reg_a2                   : 1;  /**< UAHC register address  bit<2> invert. When set to 1,
                                                         for a 32-bit NCB I/O register access, the address
                                                         offset will be flipped between 0x4 and 0x0. */
	uint64_t reserved_8_8                 : 1;
	uint64_t l2c_addr_msb                 : 8;  /**< This is the bit [39:32] of an address sent to L2C
                                                         for ohci. */
#else
	uint64_t l2c_addr_msb                 : 8;
	uint64_t reserved_8_8                 : 1;
	uint64_t inv_reg_a2                   : 1;
	uint64_t l2c_desc_emod                : 2;
	uint64_t l2c_buff_emod                : 2;
	uint64_t l2c_stt                      : 1;
	uint64_t l2c_0pag                     : 1;
	uint64_t l2c_bc                       : 1;
	uint64_t l2c_dc                       : 1;
	uint64_t reg_nb                       : 1;
	uint64_t reserved_19_63               : 45;
#endif
	} s;
	struct cvmx_uctlx_ohci_ctl_s          cn61xx;
	struct cvmx_uctlx_ohci_ctl_s          cn63xx;
	struct cvmx_uctlx_ohci_ctl_s          cn63xxp1;
	struct cvmx_uctlx_ohci_ctl_s          cn66xx;
	struct cvmx_uctlx_ohci_ctl_s          cn68xx;
	struct cvmx_uctlx_ohci_ctl_s          cn68xxp1;
	struct cvmx_uctlx_ohci_ctl_s          cnf71xx;
};
typedef union cvmx_uctlx_ohci_ctl cvmx_uctlx_ohci_ctl_t;

/**
 * cvmx_uctl#_orto_ctl
 *
 * UCTL_ORTO_CTL = UCTL OHCI Readbuffer TimeOut Control Register
 * This register controls timeout for OHCI Readbuffer.
 */
union cvmx_uctlx_orto_ctl {
	uint64_t u64;
	struct cvmx_uctlx_orto_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_32_63               : 32;
	uint64_t to_val                       : 24; /**< Read buffer timeout value
                                                         (value 0 means timeout disabled) */
	uint64_t reserved_0_7                 : 8;
#else
	uint64_t reserved_0_7                 : 8;
	uint64_t to_val                       : 24;
	uint64_t reserved_32_63               : 32;
#endif
	} s;
	struct cvmx_uctlx_orto_ctl_s          cn61xx;
	struct cvmx_uctlx_orto_ctl_s          cn63xx;
	struct cvmx_uctlx_orto_ctl_s          cn63xxp1;
	struct cvmx_uctlx_orto_ctl_s          cn66xx;
	struct cvmx_uctlx_orto_ctl_s          cn68xx;
	struct cvmx_uctlx_orto_ctl_s          cn68xxp1;
	struct cvmx_uctlx_orto_ctl_s          cnf71xx;
};
typedef union cvmx_uctlx_orto_ctl cvmx_uctlx_orto_ctl_t;

/**
 * cvmx_uctl#_port#_cfg_hs
 *
 * This register controls configuration and test controls for the high-speed port 0 PHY.
 *
 * This register is accessible only when UCTL()_CTL[H_CLK_EN] = 1.
 *
 * This register can be reset by IOI reset or UCTL()_CTL[UCTL_RST].
 */
union cvmx_uctlx_portx_cfg_hs {
	uint64_t u64;
	struct cvmx_uctlx_portx_cfg_hs_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_58_63               : 6;
	uint64_t comp_dis_tune                : 3;  /**< Disconnect threshold voltage. Adjusts the voltage level for the threshold used to detect a
                                                         disconnect event at the host.
                                                         A positive binary bit setting change results in a +1.5% incremental change in the
                                                         threshold voltage level, while a negative binary bit setting change results in a -1.5%
                                                         incremental change in the threshold voltage level. */
	uint64_t sq_rx_tune                   : 3;  /**< Squelch threshold adjustment. Adjusts the voltage level for the threshold used to detect
                                                         valid high-speed data.
                                                         A positive binary bit setting change results in a -5% incremental change in threshold
                                                         voltage level, while a negative binary bit setting change results in a +5% incremental
                                                         change in threshold voltage level. */
	uint64_t tx_fsls_tune                 : 4;  /**< Low-speed/full-speed source impedance adjustment. Adjusts the low- and full-speed single-
                                                         ended source impedance while driving high. This parameter control is encoded in
                                                         thermometer code.
                                                         A positive thermometer code change results in a -2.5% incremental change in source
                                                         impedance. A negative thermometer code change results in +2.5% incremental change in
                                                         source impedance. Any non-thermometer code setting (that is, 0x9) is not supported and
                                                         reserved. */
	uint64_t reserved_46_47               : 2;
	uint64_t tx_hs_xv_tune                : 2;  /**< Transmitter high-speed crossover adjustment. This bus adjusts the voltage at which the DP0
                                                         and DM0 signals cross while transmitting in high-speed mode.
                                                         0x0 = reserved.
                                                         0x1 = -15 mV.
                                                         0x2 = +15 mV.
                                                         0x3 = default setting. */
	uint64_t tx_preemp_amp_tune           : 2;  /**< High-speed transmitter preemphasis current control. Controls the amount of current
                                                         sourced to DP0 and DM0 after a J-to-K or K-to-J transition. The high-speed transmitter
                                                         preemphasis current is defined in terms of unit amounts. One unit amount is approximately
                                                         600 A and is defined as 1* preemphasis current.
                                                         0x0 = High-speed TX preemphasis is disabled.
                                                         0x1 = High-speed TX preemphasis circuit sources 1* preemphasis current.
                                                         0x2 = High-speed TX preemphasis circuit sources 2* preemphasis current.
                                                         0x3 = High-speed TX preemphasis circuit sources 3* preemphasis current.
                                                         If these signals are not used, set them to 0x0. */
	uint64_t reserved_41_41               : 1;
	uint64_t tx_preemp_pulse_tune         : 1;  /**< High-speed transmitter preemphasis duration control. Controls the duration for which the
                                                         high-speed preemphasis current is sourced onto DP0 or DM0. The high-speed transmitter
                                                         preemphasis duration is defined in terms of unit amounts. One unit of preemphasis duration
                                                         is approximately 580 ps and is defined as 1* preemphasis duration. This signal is valid
                                                         only if either TX_PREEMP_AMP_TUNE0[1] or TX_PREEMP_AMP_TUNE0[0] is set to 1.
                                                         0 = 2*, long preemphasis current duration (design default).
                                                         1 = 1*, short preemphasis current duration.
                                                         If this signal is not used, set it to 0. */
	uint64_t tx_res_tune                  : 2;  /**< USB source-impedance adjustment. Some applications require additional devices to be added
                                                         on the USB, such as a series switch, which can add significant series resistance. This bus
                                                         adjusts the driver source impedance to compensate for added series resistance on the USB.
                                                         0x0 = source impedance is decreased by approximately 1.5 ohm.
                                                         0x1 = design default.
                                                         0x2 = source impedance is decreased by approximately 2 ohm.
                                                         0x3 = source impedance is decreased by approximately 4 ohm.
                                                         Any setting other than the default can result in source-impedance variation across
                                                         process, voltage, and temperature conditions that does not meet USB 2.0 specification
                                                         limits. If this bus is not used, leave it at the default setting. */
	uint64_t tx_rise_tune                 : 2;  /**< High-speed transmitter rise-/fall-time adjustment. Adjusts the rise/fall times of the
                                                         high-speed waveform. A positive binary bit setting change results in a -4% incremental
                                                         change in the high-speed rise/fall time. A negative binary bit setting change results in a
                                                         +4% incremental change in the high-speed rise/fall time. */
	uint64_t tx_vref_tune                 : 4;  /**< High-speed DC voltage-level adjustment. Adjusts the high-speed DC level voltage.
                                                         A positive binary-bit-setting change results in a +1.25% incremental change in high-speed
                                                         DC voltage level, while a negative binary-bit-setting change results in a -1.25%
                                                         incremental change in high-speed DC voltage level.
                                                         The default bit setting is intended to create a HighSpeed transmit
                                                         DC level of approximately 400mV. */
	uint64_t reserved_4_31                : 28;
	uint64_t vatest_enable                : 2;  /**< Analog test-pin select. Enables analog test voltages to be placed on the ID0 pin.
                                                         0x0 = Test functionality disabled.
                                                         0x1 = Test functionality enabled.
                                                         0x2, 0x3 = Reserved, invalid settings.
                                                         See also the PHY databook for details on how to select which analog test voltage. */
	uint64_t loopback_enable              : 1;  /**< Places the high-speed PHY in loopback mode, which concurrently enables high-speed receive
                                                         and transmit logic. */
	uint64_t atereset                     : 1;  /**< Per-PHY ATE reset. When the USB core is powered up (not in suspend mode), an automatic
                                                         tester can use this to disable PHYCLOCK and FREECLK, then re-enable them with an aligned
                                                         phase.
                                                         0 = PHYCLOCK and FREECLK are available within a specific period after ATERESET is
                                                         deasserted.
                                                         1 = PHYCLOCK and FREECLK outputs are disabled. */
#else
	uint64_t atereset                     : 1;
	uint64_t loopback_enable              : 1;
	uint64_t vatest_enable                : 2;
	uint64_t reserved_4_31                : 28;
	uint64_t tx_vref_tune                 : 4;
	uint64_t tx_rise_tune                 : 2;
	uint64_t tx_res_tune                  : 2;
	uint64_t tx_preemp_pulse_tune         : 1;
	uint64_t reserved_41_41               : 1;
	uint64_t tx_preemp_amp_tune           : 2;
	uint64_t tx_hs_xv_tune                : 2;
	uint64_t reserved_46_47               : 2;
	uint64_t tx_fsls_tune                 : 4;
	uint64_t sq_rx_tune                   : 3;
	uint64_t comp_dis_tune                : 3;
	uint64_t reserved_58_63               : 6;
#endif
	} s;
	struct cvmx_uctlx_portx_cfg_hs_s      cn78xx;
	struct cvmx_uctlx_portx_cfg_hs_s      cn78xxp1;
};
typedef union cvmx_uctlx_portx_cfg_hs cvmx_uctlx_portx_cfg_hs_t;

/**
 * cvmx_uctl#_port#_cfg_ss
 *
 * This register controls configuration and test controls for the SS port 0 PHY.
 *
 * This register is accessible only when UCTL()_CTL[H_CLK_EN] = 1.
 *
 * This register can be reset by IOI reset or with UCTL()_CTL[UCTL_RST].
 */
union cvmx_uctlx_portx_cfg_ss {
	uint64_t u64;
	struct cvmx_uctlx_portx_cfg_ss_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t tx_vboost_lvl                : 3;  /**< TX voltage-boost level. Sets the boosted transmit launch amplitude (mVppd). The default
                                                         bit setting is intended to set the launch amplitude to approximately 1,008 mVppd. A
                                                         single, positive binary bit setting change results in a +156 mVppd change in the TX launch
                                                         amplitude.
                                                         A single, negative binary bit setting change results in a -156 mVppd change in the TX
                                                         launch amplitude. All settings more than one binary bit change should not be used.
                                                         0x3 = 0.844 V launch amplitude.
                                                         0x4 = 1.008 V launch amplitude.
                                                         0x5 = 1.156 V launch amplitude.
                                                         All others values are invalid. */
	uint64_t los_bias                     : 3;  /**< Loss-of-signal detector threshold-level control. A positive, binary bit setting change
                                                         results in a +15 mVp incremental change in the LOS threshold.
                                                         A negative binary bit setting change results in a -15 mVp incremental change in the LOS
                                                         threshold. The 0x0 setting is reserved and must not be used. The default 0x5 setting
                                                         corresponds to approximately 105 mVp.
                                                         0x0 = invalid.
                                                         0x1 = 45 mV.
                                                         0x2 = 60 mV.
                                                         0x3 = 75 mV.
                                                         0x4 = 90 mV.
                                                         0x5 = 105 mV (default).
                                                         0x6 = 120 mV.
                                                         0x7 = 135 mV. */
	uint64_t lane0_ext_pclk_req           : 1;  /**< When asserted, this signal enables the pipe0_pclk output regardless of power state (along
                                                         with the associated increase in power consumption). You can use this input to enable
                                                         pipe0_pclk in the P3 state without going through a complete boot sequence. */
	uint64_t lane0_tx2rx_loopbk           : 1;  /**< When asserted, data from TX predriver is looped back to RX slicers. LOS is bypassed and
                                                         based on the tx0_en input so that rx0_los = !tx_data_en. */
	uint64_t reserved_42_55               : 14;
	uint64_t pcs_rx_los_mask_val          : 10; /**< Configurable loss-of-signal mask width. Sets the number of reference clock cycles to mask
                                                         the incoming LFPS in U3 and U2 states. Masks the incoming LFPS for the number of reference
                                                         clock cycles equal to the value of pcs_rx_los_mask_val<9:0>. This control filters out
                                                         short, non-compliant LFPS glitches sent by a noncompliant host.
                                                         For normal operation, set to a targeted mask interval of 10us (value = 10us / Tref_clk).
                                                         If the UCTL()_CTL[REF_CLK_DIV2] is used, then
                                                         (value = 10us / (2 * Tref_clk)). These equations are based on the SuperSpeed reference
                                                         clock frequency. The value of PCS_RX_LOS_MASK_VAL should be as follows:
                                                         <pre>
                                                              Frequency   DIV2  LOS_MASK
                                                              ---------    ---  --------
                                                              200   MHz      1     0x3E8
                                                              125   MHz      0     0x4E2
                                                              104   MHz      0     0x410
                                                              100   MHz      0     0x3E8
                                                               96   MHz      0     0x3C0
                                                               76.8 MHz      1     0x180
                                                               52   MHz      0     0x208
                                                               50   MHz      0     0x1F4
                                                               48   MHz      0     0x1E0
                                                               40   MHz      1     0x0C8
                                                               38.4 MHz      0     0x180
                                                               26   MHz      0     0x104
                                                               25   MHz      0     0x0FA
                                                               24   MHz      0     0x0F0
                                                               20   MHz      0     0x0C8
                                                               19.2 MHz      0     0x0C0
                                                         </pre>
                                                         Setting this bus to 0x0 disables masking. The value should be defined when the PHY is in
                                                         reset. Changing this value during operation might disrupt normal operation of the link. */
	uint64_t pcs_tx_deemph_3p5db          : 6;  /**< Fine-tune transmitter driver deemphasis when set to 3.5db.
                                                         This static value sets the TX driver deemphasis value when
                                                         UAHC()_GUSB3PIPECTL()[TXDEEMPHASIS] is set to
                                                         0x1 (according to the PIPE3 specification). The values for transmit deemphasis are derived
                                                         from the following equation:
                                                         _ TX de-emphasis (db) = 20 * log_base_10((128 - 2 * pcs_tx_deemph)/128)
                                                         In general, the parameter controls are static signals to be set prior to taking the PHY
                                                         out of reset. However, you can dynamically change these values on-the-fly for test
                                                         purposes. In this case, changes to the transmitter to reflect the current value occur only
                                                         after the UAHC()_GUSB3PIPECTL()[TXDEEMPHASIS] changes. */
	uint64_t pcs_tx_deemph_6db            : 6;  /**< Fine-tune transmitter driver deemphasis when set to 6 db.
                                                         This static value sets the TX driver deemphasis value when
                                                         UAHC()_GUSB3PIPECTL()[TXDEEMPHASIS] is set to
                                                         0x2 (according to the PIPE3 specification). This bus is provided for completeness and as a
                                                         second potential launch amplitude. The values for transmit deemphasis are derived from the
                                                         following equation:
                                                         _ TX deemphasis (db) = 20 * log_base_10((128 - 2 * pcs_tx_deemph)/128)
                                                         In general, the parameter controls are static signals to be set prior to taking the PHY
                                                         out of reset. However, you can dynamically change these values on-the-fly for test
                                                         purposes. In this case, changes to the transmitter to reflect the current value occur only
                                                         after the UAHC()_GUSB3PIPECTL()[TXDEEMPHASIS] changes. */
	uint64_t pcs_tx_swing_full            : 7;  /**< Launch amplitude of the transmitter. Sets the launch amplitude of the transmitter. The
                                                         values for transmit amplitude are derived from the following equation:
                                                         TX amplitude (V) = vptx * ((pcs_tx_swing_full + 1)/128)
                                                         In general, the parameter controls are static signals to be set prior to taking the PHY
                                                         out of reset. However, you can dynamically change these values on-the-fly for test
                                                         purposes. In this case, changes to the transmitter to reflect the current value occur only
                                                         after the UAHC()_GUSB3PIPECTL()[TXDEEMPHASIS] changes. */
	uint64_t lane0_tx_term_offset         : 5;  /**< Transmitter termination offset. Reserved, set to 0x0. */
	uint64_t reserved_6_7                 : 2;
	uint64_t res_tune_ack                 : 1;  /**< Resistor tune acknowledge. While asserted, indicates a resistor tune is in progress. */
	uint64_t res_tune_req                 : 1;  /**< Resistor tune request. The rising edge triggers a resistor tune request (if one is not
                                                         already in progress). When asserted, [RES_TUNE_ACK] is asserted high until calibration of
                                                         the termination impedance is complete.
                                                         Tuning disrupts the normal flow of data; therefore, assert [RES_TUNE_REQ] only when the
                                                         PHY
                                                         is inactive. The PHY automatically performs a tune when coming out of PRST. */
	uint64_t reserved_0_3                 : 4;
#else
	uint64_t reserved_0_3                 : 4;
	uint64_t res_tune_req                 : 1;
	uint64_t res_tune_ack                 : 1;
	uint64_t reserved_6_7                 : 2;
	uint64_t lane0_tx_term_offset         : 5;
	uint64_t pcs_tx_swing_full            : 7;
	uint64_t pcs_tx_deemph_6db            : 6;
	uint64_t pcs_tx_deemph_3p5db          : 6;
	uint64_t pcs_rx_los_mask_val          : 10;
	uint64_t reserved_42_55               : 14;
	uint64_t lane0_tx2rx_loopbk           : 1;
	uint64_t lane0_ext_pclk_req           : 1;
	uint64_t los_bias                     : 3;
	uint64_t tx_vboost_lvl                : 3;
#endif
	} s;
	struct cvmx_uctlx_portx_cfg_ss_s      cn78xx;
	struct cvmx_uctlx_portx_cfg_ss_s      cn78xxp1;
};
typedef union cvmx_uctlx_portx_cfg_ss cvmx_uctlx_portx_cfg_ss_t;

/**
 * cvmx_uctl#_port#_cr_dbg_cfg
 *
 * This register allows indirect access to the configuration and test controls for the port 0
 * PHY.
 *
 * This register is accessible only when UCTL()_CTL[H_CLK_EN] = 1.
 *
 * This register can be reset by IOI reset or with UCTL()_CTL[UCTL_RST].
 */
union cvmx_uctlx_portx_cr_dbg_cfg {
	uint64_t u64;
	struct cvmx_uctlx_portx_cr_dbg_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t data_in                      : 16; /**< Address or data to be written to the CR interface. */
	uint64_t reserved_4_31                : 28;
	uint64_t cap_addr                     : 1;  /**< Rising edge triggers the [DATA_IN] field to be captured as the address. */
	uint64_t cap_data                     : 1;  /**< Rising edge triggers the [DATA_IN] field to be captured as the write data. */
	uint64_t read                         : 1;  /**< Rising edge triggers a register read operation of the captured address. */
	uint64_t write                        : 1;  /**< Rising edge triggers a register write operation of the captured address with the captured data. */
#else
	uint64_t write                        : 1;
	uint64_t read                         : 1;
	uint64_t cap_data                     : 1;
	uint64_t cap_addr                     : 1;
	uint64_t reserved_4_31                : 28;
	uint64_t data_in                      : 16;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_uctlx_portx_cr_dbg_cfg_s  cn78xx;
	struct cvmx_uctlx_portx_cr_dbg_cfg_s  cn78xxp1;
};
typedef union cvmx_uctlx_portx_cr_dbg_cfg cvmx_uctlx_portx_cr_dbg_cfg_t;

/**
 * cvmx_uctl#_port#_cr_dbg_status
 *
 * This register allows indirect access to the configuration and test controls for the port 0
 * PHY.
 *
 * This register is accessible only when UCTL()_CTL[H_CLK_EN] = 1.
 *
 * This register can be reset by IOI reset or with UCTL()_CTL[UCTL_RST].
 */
union cvmx_uctlx_portx_cr_dbg_status {
	uint64_t u64;
	struct cvmx_uctlx_portx_cr_dbg_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_48_63               : 16;
	uint64_t data_out                     : 16; /**< Last data read from the CR interface. */
	uint64_t reserved_1_31                : 31;
	uint64_t ack                          : 1;  /**< Acknowledge that the CAP_ADDR, CAP_DATA, READ, WRITE commands have completed. */
#else
	uint64_t ack                          : 1;
	uint64_t reserved_1_31                : 31;
	uint64_t data_out                     : 16;
	uint64_t reserved_48_63               : 16;
#endif
	} s;
	struct cvmx_uctlx_portx_cr_dbg_status_s cn78xx;
	struct cvmx_uctlx_portx_cr_dbg_status_s cn78xxp1;
};
typedef union cvmx_uctlx_portx_cr_dbg_status cvmx_uctlx_portx_cr_dbg_status_t;

/**
 * cvmx_uctl#_ppaf_wm
 *
 * UCTL_PPAF_WM = UCTL PP Access FIFO WaterMark Register
 *
 * Register to set PP access FIFO full watermark.
 */
union cvmx_uctlx_ppaf_wm {
	uint64_t u64;
	struct cvmx_uctlx_ppaf_wm_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_5_63                : 59;
	uint64_t wm                           : 5;  /**< Number of entries when PP Access FIFO will assert
                                                         full (back pressure) */
#else
	uint64_t wm                           : 5;
	uint64_t reserved_5_63                : 59;
#endif
	} s;
	struct cvmx_uctlx_ppaf_wm_s           cn61xx;
	struct cvmx_uctlx_ppaf_wm_s           cn63xx;
	struct cvmx_uctlx_ppaf_wm_s           cn63xxp1;
	struct cvmx_uctlx_ppaf_wm_s           cn66xx;
	struct cvmx_uctlx_ppaf_wm_s           cnf71xx;
};
typedef union cvmx_uctlx_ppaf_wm cvmx_uctlx_ppaf_wm_t;

/**
 * cvmx_uctl#_shim_cfg
 *
 * This register allows configuration of various shim (UCTL) features. The fields XS_NCB_OOB_*
 * are captured when there are no outstanding OOB errors indicated in INTSTAT and a new OOB error
 * arrives. The fields XS_BAD_DMA_* are captured when there are no outstanding DMA errors
 * indicated in INTSTAT and a new DMA error arrives.
 *
 * This register is accessible only when UCTL()_CTL[H_CLK_EN] = 1.
 *
 * This register can be reset by IOI reset or with UCTL()_CTL[UCTL_RST].
 */
union cvmx_uctlx_shim_cfg {
	uint64_t u64;
	struct cvmx_uctlx_shim_cfg_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t xs_ncb_oob_wrn               : 1;  /**< Read/write error log for out-of-bound UAHC register access.
                                                         0 = read, 1 = write. */
	uint64_t reserved_60_62               : 3;
	uint64_t xs_ncb_oob_osrc              : 12; /**< SRCID error log for out-of-bound UAHC register access. The IOI outbound SRCID for the OOB
                                                         error.
                                                         <59:58> = chipID.
                                                         <57> = Request source: 0 = core, 1 = IOI-device.
                                                         <56:51> = Core/IOI-device number. Note that for IOI devices, <56> is always 0.
                                                         <50:48> = SubID. */
	uint64_t xm_bad_dma_wrn               : 1;  /**< Read/write error log for bad DMA access from UAHC.
                                                         0 = read error log, 1 = write error log. */
	uint64_t reserved_44_46               : 3;
	uint64_t xm_bad_dma_type              : 4;  /**< ErrType error log for bad DMA access from UAHC. Encodes the type of error encountered
                                                         (error largest encoded value has priority). See UCTL_XM_BAD_DMA_TYPE_E. */
	uint64_t reserved_14_39               : 26;
	uint64_t dma_read_cmd                 : 2;  /**< Selects the IOI read command used by DMA accesses. See UCTL_DMA_READ_CMD_E. */
	uint64_t reserved_11_11               : 1;
	uint64_t dma_write_cmd                : 1;  /**< Selects the NCB write command used by DMA accesses. See UCTL_DMA_WRITE_CMD_E. */
	uint64_t dma_endian_mode              : 2;  /**< Selects the endian format for DMA accesses to the L2C. See UCTL_ENDIAN_MODE_E. */
	uint64_t reserved_2_7                 : 6;
	uint64_t csr_endian_mode              : 2;  /**< Selects the endian format for IOI CSR accesses to the UAHC. Note that when UAHC CSRs are
                                                         accessed via RSL, they are returned as big-endian. See UCTL_ENDIAN_MODE_E. */
#else
	uint64_t csr_endian_mode              : 2;
	uint64_t reserved_2_7                 : 6;
	uint64_t dma_endian_mode              : 2;
	uint64_t dma_write_cmd                : 1;
	uint64_t reserved_11_11               : 1;
	uint64_t dma_read_cmd                 : 2;
	uint64_t reserved_14_39               : 26;
	uint64_t xm_bad_dma_type              : 4;
	uint64_t reserved_44_46               : 3;
	uint64_t xm_bad_dma_wrn               : 1;
	uint64_t xs_ncb_oob_osrc              : 12;
	uint64_t reserved_60_62               : 3;
	uint64_t xs_ncb_oob_wrn               : 1;
#endif
	} s;
	struct cvmx_uctlx_shim_cfg_s          cn78xx;
	struct cvmx_uctlx_shim_cfg_s          cn78xxp1;
};
typedef union cvmx_uctlx_shim_cfg cvmx_uctlx_shim_cfg_t;

/**
 * cvmx_uctl#_spare0
 *
 * This register is a spare register. This register can be reset by IOI reset.
 *
 */
union cvmx_uctlx_spare0 {
	uint64_t u64;
	struct cvmx_uctlx_spare0_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_uctlx_spare0_s            cn78xx;
	struct cvmx_uctlx_spare0_s            cn78xxp1;
};
typedef union cvmx_uctlx_spare0 cvmx_uctlx_spare0_t;

/**
 * cvmx_uctl#_spare1
 *
 * This register is accessible only when UCTL()_CTL[H_CLK_EN] = 1.
 *
 * This register can be reset by IOI reset or with UCTL()_CTL[UCTL_RST].
 */
union cvmx_uctlx_spare1 {
	uint64_t u64;
	struct cvmx_uctlx_spare1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_uctlx_spare1_s            cn78xx;
	struct cvmx_uctlx_spare1_s            cn78xxp1;
};
typedef union cvmx_uctlx_spare1 cvmx_uctlx_spare1_t;

/**
 * cvmx_uctl#_uphy_ctl_status
 *
 * UPHY_CTL_STATUS = USB PHY Control and Status Reigster
 * This register controls the USB PHY test and Bist.
 */
union cvmx_uctlx_uphy_ctl_status {
	uint64_t u64;
	struct cvmx_uctlx_uphy_ctl_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_10_63               : 54;
	uint64_t bist_done                    : 1;  /**< PHY BIST DONE.  Asserted at the end of the PHY BIST
                                                         sequence. */
	uint64_t bist_err                     : 1;  /**< PHY BIST Error.  Valid when BIST_ENB is high.
                                                         Indicates an internal error was detected during the
                                                         BIST sequence. */
	uint64_t hsbist                       : 1;  /**< High-Speed BIST Enable */
	uint64_t fsbist                       : 1;  /**< Full-Speed BIST Enable */
	uint64_t lsbist                       : 1;  /**< Low-Speed BIST Enable */
	uint64_t siddq                        : 1;  /**< Drives the PHY SIDDQ input. Normally should be set
                                                         to zero. Customers not using USB PHY interface
                                                         should do the following:
                                                           Provide 3.3V to USB_VDD33 Tie USB_REXT to 3.3V
                                                           supply and Set SIDDQ to 1. */
	uint64_t vtest_en                     : 1;  /**< Analog Test Pin Enable.
                                                         1 = The PHY's ANALOG_TEST pin is enabled for the
                                                             input and output of applicable analog test
                                                             signals.
                                                         0 = The ANALOG_TEST pin is disabled. */
	uint64_t uphy_bist                    : 1;  /**< When set to 1,  it makes sure that during PHY BIST,
                                                         utmi_txvld == 0. */
	uint64_t bist_en                      : 1;  /**< PHY BIST ENABLE */
	uint64_t ate_reset                    : 1;  /**< Reset Input from ATE. This is a test signal. When
                                                         the USB core is powered up (not in suspend mode), an
                                                         automatic tester can use this to disable PHYCLOCK
                                                         and FREECLK, then re-enable them with an aligned
                                                         phase.
                                                         - 1:  PHYCLOCKs and FREECLK outputs are disable.
                                                         - 0: PHYCLOCKs and FREECLK are available within a
                                                             specific period after ATERESET is de-asserted. */
#else
	uint64_t ate_reset                    : 1;
	uint64_t bist_en                      : 1;
	uint64_t uphy_bist                    : 1;
	uint64_t vtest_en                     : 1;
	uint64_t siddq                        : 1;
	uint64_t lsbist                       : 1;
	uint64_t fsbist                       : 1;
	uint64_t hsbist                       : 1;
	uint64_t bist_err                     : 1;
	uint64_t bist_done                    : 1;
	uint64_t reserved_10_63               : 54;
#endif
	} s;
	struct cvmx_uctlx_uphy_ctl_status_s   cn61xx;
	struct cvmx_uctlx_uphy_ctl_status_s   cn63xx;
	struct cvmx_uctlx_uphy_ctl_status_s   cn63xxp1;
	struct cvmx_uctlx_uphy_ctl_status_s   cn66xx;
	struct cvmx_uctlx_uphy_ctl_status_s   cn68xx;
	struct cvmx_uctlx_uphy_ctl_status_s   cn68xxp1;
	struct cvmx_uctlx_uphy_ctl_status_s   cnf71xx;
};
typedef union cvmx_uctlx_uphy_ctl_status cvmx_uctlx_uphy_ctl_status_t;

/**
 * cvmx_uctl#_uphy_port#_ctl_status
 *
 * UPHY_PORTX_CTL_STATUS = USB PHY Port X Control and Status Reigsters
 * This register controls the each port of the USB PHY.
 */
union cvmx_uctlx_uphy_portx_ctl_status {
	uint64_t u64;
	struct cvmx_uctlx_uphy_portx_ctl_status_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_43_63               : 21;
	uint64_t tdata_out                    : 4;  /**< PHY test data out. Presents either interlly
                                                         generated signals or test register contenets, based
                                                         upon the value of TDATA_SEL */
	uint64_t txbiststuffenh               : 1;  /**< High-Byte Transmit Bit-Stuffing Enable. It must be
                                                         set to 1'b1 in normal operation. */
	uint64_t txbiststuffen                : 1;  /**< Low-Byte Transmit Bit-Stuffing Enable. It must be
                                                         set to 1'b1 in normal operation. */
	uint64_t dmpulldown                   : 1;  /**< D- Pull-Down Resistor Enable. It must be set to 1'b1
                                                         in normal operation. */
	uint64_t dppulldown                   : 1;  /**< D+ Pull-Down Resistor Enable. It must be set to 1'b1
                                                         in normal operation. */
	uint64_t vbusvldext                   : 1;  /**< In host mode, this input is not used and can be tied
                                                         to 1'b0. */
	uint64_t portreset                    : 1;  /**< Per-port reset */
	uint64_t txhsvxtune                   : 2;  /**< Transmitter High-Speed Crossover Adjustment */
	uint64_t txvreftune                   : 4;  /**< HS DC Voltage Level Adjustment
                                                         When the recommended 37.4 Ohm resistor is present
                                                         on USB_REXT, the recommended TXVREFTUNE value is 15 */
	uint64_t txrisetune                   : 1;  /**< HS Transmitter Rise/Fall Time Adjustment
                                                         When the recommended 37.4 Ohm resistor is present
                                                         on USB_REXT, the recommended TXRISETUNE value is 1 */
	uint64_t txpreemphasistune            : 1;  /**< HS transmitter pre-emphasis enable.
                                                         When the recommended 37.4 Ohm resistor is present
                                                         on USB_REXT, the recommended TXPREEMPHASISTUNE
                                                         value is 1 */
	uint64_t txfslstune                   : 4;  /**< FS/LS Source Impedance Adjustment */
	uint64_t sqrxtune                     : 3;  /**< Squelch Threshold Adjustment */
	uint64_t compdistune                  : 3;  /**< Disconnect Threshold Adjustment */
	uint64_t loop_en                      : 1;  /**< Port Loop back Test Enable
                                                         - 1: During data transmission, the receive logic is
                                                             enabled
                                                         - 0: During data transmission, the receive logic is
                                                             disabled */
	uint64_t tclk                         : 1;  /**< PHY port test clock, used to load TDATA_IN to the
                                                         UPHY. */
	uint64_t tdata_sel                    : 1;  /**< Test Data out select
                                                         - 1: Mode-defined test register contents are output
                                                         - 0: internally generated signals are output */
	uint64_t taddr_in                     : 4;  /**< Mode address for test interface. Specifies the
                                                         register address for writing to or reading from the
                                                         PHY test interface register. */
	uint64_t tdata_in                     : 8;  /**< Internal testing Register input data and select.
                                                         This is a test bus. Data presents on [3:0] and the
                                                         corresponding select (enable) presents on bits[7:4]. */
#else
	uint64_t tdata_in                     : 8;
	uint64_t taddr_in                     : 4;
	uint64_t tdata_sel                    : 1;
	uint64_t tclk                         : 1;
	uint64_t loop_en                      : 1;
	uint64_t compdistune                  : 3;
	uint64_t sqrxtune                     : 3;
	uint64_t txfslstune                   : 4;
	uint64_t txpreemphasistune            : 1;
	uint64_t txrisetune                   : 1;
	uint64_t txvreftune                   : 4;
	uint64_t txhsvxtune                   : 2;
	uint64_t portreset                    : 1;
	uint64_t vbusvldext                   : 1;
	uint64_t dppulldown                   : 1;
	uint64_t dmpulldown                   : 1;
	uint64_t txbiststuffen                : 1;
	uint64_t txbiststuffenh               : 1;
	uint64_t tdata_out                    : 4;
	uint64_t reserved_43_63               : 21;
#endif
	} s;
	struct cvmx_uctlx_uphy_portx_ctl_status_s cn61xx;
	struct cvmx_uctlx_uphy_portx_ctl_status_s cn63xx;
	struct cvmx_uctlx_uphy_portx_ctl_status_s cn63xxp1;
	struct cvmx_uctlx_uphy_portx_ctl_status_s cn66xx;
	struct cvmx_uctlx_uphy_portx_ctl_status_s cn68xx;
	struct cvmx_uctlx_uphy_portx_ctl_status_s cn68xxp1;
	struct cvmx_uctlx_uphy_portx_ctl_status_s cnf71xx;
};
typedef union cvmx_uctlx_uphy_portx_ctl_status cvmx_uctlx_uphy_portx_ctl_status_t;

#endif
