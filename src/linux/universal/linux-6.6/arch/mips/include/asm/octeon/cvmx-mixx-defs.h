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
 * cvmx-mixx-defs.h
 *
 * Configuration and status register (CSR) type definitions for
 * Octeon mixx.
 *
 * This file is auto generated. Do not edit.
 *
 * <hr>$Revision$<hr>
 *
 */
#ifndef __CVMX_MIXX_DEFS_H__
#define __CVMX_MIXX_DEFS_H__

#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_MIXX_BIST(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_MIXX_BIST(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000100078ull) + ((offset) & 1) * 2048;
}
#else
#define CVMX_MIXX_BIST(offset) (CVMX_ADD_IO_SEG(0x0001070000100078ull) + ((offset) & 1) * 2048)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_MIXX_CTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_MIXX_CTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000100020ull) + ((offset) & 1) * 2048;
}
#else
#define CVMX_MIXX_CTL(offset) (CVMX_ADD_IO_SEG(0x0001070000100020ull) + ((offset) & 1) * 2048)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_MIXX_INTENA(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0)))))
		cvmx_warn("CVMX_MIXX_INTENA(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000100050ull) + ((offset) & 1) * 2048;
}
#else
#define CVMX_MIXX_INTENA(offset) (CVMX_ADD_IO_SEG(0x0001070000100050ull) + ((offset) & 1) * 2048)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_MIXX_IRCNT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_MIXX_IRCNT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000100030ull) + ((offset) & 1) * 2048;
}
#else
#define CVMX_MIXX_IRCNT(offset) (CVMX_ADD_IO_SEG(0x0001070000100030ull) + ((offset) & 1) * 2048)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_MIXX_IRHWM(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_MIXX_IRHWM(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000100028ull) + ((offset) & 1) * 2048;
}
#else
#define CVMX_MIXX_IRHWM(offset) (CVMX_ADD_IO_SEG(0x0001070000100028ull) + ((offset) & 1) * 2048)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_MIXX_IRING1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_MIXX_IRING1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000100010ull) + ((offset) & 1) * 2048;
}
#else
#define CVMX_MIXX_IRING1(offset) (CVMX_ADD_IO_SEG(0x0001070000100010ull) + ((offset) & 1) * 2048)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_MIXX_IRING2(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_MIXX_IRING2(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000100018ull) + ((offset) & 1) * 2048;
}
#else
#define CVMX_MIXX_IRING2(offset) (CVMX_ADD_IO_SEG(0x0001070000100018ull) + ((offset) & 1) * 2048)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_MIXX_ISR(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_MIXX_ISR(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000100048ull) + ((offset) & 1) * 2048;
}
#else
#define CVMX_MIXX_ISR(offset) (CVMX_ADD_IO_SEG(0x0001070000100048ull) + ((offset) & 1) * 2048)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_MIXX_ISR_W1S(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_MIXX_ISR_W1S(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000100050ull) + ((offset) & 1) * 2048;
}
#else
#define CVMX_MIXX_ISR_W1S(offset) (CVMX_ADD_IO_SEG(0x0001070000100050ull) + ((offset) & 1) * 2048)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_MIXX_ORCNT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_MIXX_ORCNT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000100040ull) + ((offset) & 1) * 2048;
}
#else
#define CVMX_MIXX_ORCNT(offset) (CVMX_ADD_IO_SEG(0x0001070000100040ull) + ((offset) & 1) * 2048)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_MIXX_ORHWM(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_MIXX_ORHWM(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000100038ull) + ((offset) & 1) * 2048;
}
#else
#define CVMX_MIXX_ORHWM(offset) (CVMX_ADD_IO_SEG(0x0001070000100038ull) + ((offset) & 1) * 2048)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_MIXX_ORING1(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_MIXX_ORING1(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000100000ull) + ((offset) & 1) * 2048;
}
#else
#define CVMX_MIXX_ORING1(offset) (CVMX_ADD_IO_SEG(0x0001070000100000ull) + ((offset) & 1) * 2048)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_MIXX_ORING2(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_MIXX_ORING2(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000100008ull) + ((offset) & 1) * 2048;
}
#else
#define CVMX_MIXX_ORING2(offset) (CVMX_ADD_IO_SEG(0x0001070000100008ull) + ((offset) & 1) * 2048)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_MIXX_REMCNT(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN52XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN56XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_MIXX_REMCNT(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000100058ull) + ((offset) & 1) * 2048;
}
#else
#define CVMX_MIXX_REMCNT(offset) (CVMX_ADD_IO_SEG(0x0001070000100058ull) + ((offset) & 1) * 2048)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_MIXX_TSCTL(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_MIXX_TSCTL(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000100068ull) + ((offset) & 1) * 2048;
}
#else
#define CVMX_MIXX_TSCTL(offset) (CVMX_ADD_IO_SEG(0x0001070000100068ull) + ((offset) & 1) * 2048)
#endif
#if CVMX_ENABLE_CSR_ADDRESS_CHECKING
static inline uint64_t CVMX_MIXX_TSTAMP(unsigned long offset)
{
	if (!(
	      (OCTEON_IS_MODEL(OCTEON_CN61XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN63XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN66XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN68XX) && ((offset == 0))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN73XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X) && ((offset <= 1))) ||
	      (OCTEON_IS_MODEL(OCTEON_CNF75XX) && ((offset <= 1)))))
		cvmx_warn("CVMX_MIXX_TSTAMP(%lu) is invalid on this chip\n", offset);
	return CVMX_ADD_IO_SEG(0x0001070000100060ull) + ((offset) & 1) * 2048;
}
#else
#define CVMX_MIXX_TSTAMP(offset) (CVMX_ADD_IO_SEG(0x0001070000100060ull) + ((offset) & 1) * 2048)
#endif

/**
 * cvmx_mix#_bist
 *
 * This register contains the BIST status for the MIX memories: 0 = pass or never run, 1 = fail.
 *
 */
union cvmx_mixx_bist {
	uint64_t u64;
	struct cvmx_mixx_bist_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_6_63                : 58;
	uint64_t opfdat                       : 1;  /**< BIST results for AGO OPF buffer RAM. */
	uint64_t mrgdat                       : 1;  /**< BIST results for AGI MRG buffer RAM. */
	uint64_t mrqdat                       : 1;  /**< BIST results for NBR CSR RDREQ RAM. */
	uint64_t ipfdat                       : 1;  /**< BIST results for MIX inbound packet RAM. */
	uint64_t irfdat                       : 1;  /**< BIST results for MIX I-Ring entry RAM. */
	uint64_t orfdat                       : 1;  /**< BIST results for MIX O-Ring entry RAM. */
#else
	uint64_t orfdat                       : 1;
	uint64_t irfdat                       : 1;
	uint64_t ipfdat                       : 1;
	uint64_t mrqdat                       : 1;
	uint64_t mrgdat                       : 1;
	uint64_t opfdat                       : 1;
	uint64_t reserved_6_63                : 58;
#endif
	} s;
	struct cvmx_mixx_bist_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_4_63                : 60;
	uint64_t mrqdat                       : 1;  /**< Bist Results for NBR CSR RdReq RAM
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t ipfdat                       : 1;  /**< Bist Results for MIX Inbound Packet RAM
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t irfdat                       : 1;  /**< Bist Results for MIX I-Ring Entry RAM
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
	uint64_t orfdat                       : 1;  /**< Bist Results for MIX O-Ring Entry RAM
                                                         - 0: GOOD (or bist in progress/never run)
                                                         - 1: BAD */
#else
	uint64_t orfdat                       : 1;
	uint64_t irfdat                       : 1;
	uint64_t ipfdat                       : 1;
	uint64_t mrqdat                       : 1;
	uint64_t reserved_4_63                : 60;
#endif
	} cn52xx;
	struct cvmx_mixx_bist_cn52xx          cn52xxp1;
	struct cvmx_mixx_bist_cn52xx          cn56xx;
	struct cvmx_mixx_bist_cn52xx          cn56xxp1;
	struct cvmx_mixx_bist_s               cn61xx;
	struct cvmx_mixx_bist_s               cn63xx;
	struct cvmx_mixx_bist_s               cn63xxp1;
	struct cvmx_mixx_bist_s               cn66xx;
	struct cvmx_mixx_bist_s               cn68xx;
	struct cvmx_mixx_bist_s               cn68xxp1;
	struct cvmx_mixx_bist_s               cn73xx;
	struct cvmx_mixx_bist_s               cn78xx;
	struct cvmx_mixx_bist_s               cn78xxp1;
	struct cvmx_mixx_bist_s               cnf75xx;
};
typedef union cvmx_mixx_bist cvmx_mixx_bist_t;

/**
 * cvmx_mix#_ctl
 *
 * MIX_CTL = MIX Control Register
 *
 * Description:
 *  NOTE: To write to the MIX_CTL register, a device would issue an IOBST directed at the MIO.
 *        To read the MIX_CTL register, a device would issue an IOBLD64 directed at the MIO.
 */
union cvmx_mixx_ctl {
	uint64_t u64;
	struct cvmx_mixx_ctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_12_63               : 52;
	uint64_t ts_thresh                    : 4;  /**< TimeStamp interrupt threshold. When the number of pending Timestamp interrupts
                                                         (MIX(0..1)_TSCTL[TSCNT]) is greater than
                                                         MIX(0..1)_CTL[TS_THRESH], then a programmable TimeStamp interrupt is issued (see
                                                         MIX(0..1)_INTR[TS]).
                                                         For CNXXXX, since the implementation only supports four outstanding timestamp interrupts,
                                                         this field should only be programmed from [0..3]. */
	uint64_t crc_strip                    : 1;  /**< Hardware CRC strip enable. When enabled, the last 4 bytes (CRC) of the ingress packet are
                                                         not included in cumulative packet byte length. In other words, the cumulative LEN field
                                                         for all I-Ring buffer entries associated with a given ingress packet will be 4 bytes less
                                                         (so that the final 4B hardware CRC packet data is not processed by software). */
	uint64_t busy                         : 1;  /**< MIX busy status bit. MIX asserts busy status any time there are:
                                                         * L2/DRAM read operations in-flight.
                                                         * L2/DRAM write operations in-flight.
                                                         After EN = 0, the MIX eventually completes any 'in-flight' transactions, at which point
                                                         the BUSY deasserts. */
	uint64_t en                           : 1;  /**< MIX enable bit. When [EN] = 0, MIX no longer arbitrates for any new L2/DRAM read/write
                                                         requests on the IOI. MIX completes any requests that are currently pending for the IOI. */
	uint64_t reset                        : 1;  /**< MIX soft reset. When software writes a 1 to this field, the MIX logic executes a soft
                                                         reset.
                                                         During a soft reset, CSR accesses are not affected. However, the values of the fields are
                                                         affected by soft reset (except MIX(0..1)_CTL[RESET] itself).
                                                         After power-on, the MIX-BGX are held in reset until [RESET] is written to 0. Software must
                                                         also perform a MIX(0..1)_CTL CSR read after this write to ensure the soft reset
                                                         deassertion has had sufficient time to propagate to all MIO-MIX internal logic before
                                                         any subsequent MIX CSR accesses are issued.
                                                         The intended 'soft reset' sequence is:
                                                         * Write [EN] = 0 (to prevent any NEW transactions from being started).
                                                         * Wait for [BUSY] = 0 (to indicate that all in-flight transactions have completed).
                                                         * Write [RESET] = 1, followed by a MIX(0..1)_CTL register read and wait for the result.
                                                         * Re-initialize the MIX just as would be done for a hard reset.
                                                         Once the MIX has been soft-reset, please refer to MIX Bring-up Sequence, MIX Bring-up
                                                         Sequence to complete the MIX re-initialization sequence. */
	uint64_t lendian                      : 1;  /**< Packet little-endian mode enable.
                                                         When the mode is set, MIX byte-swaps packet data load/store operations at the MIX/IOB
                                                         boundary.
                                                         0 = Big-endian mode.
                                                         1 = Little-endian mode. */
	uint64_t nbtarb                       : 1;  /**< MIX CB-request arbitration mode. When cleared to 0, the arbiter is fixed priority with the
                                                         following priority scheme:
                                                         * I-Ring packet write request. (Highest Priority.)
                                                         * O-Ring packet read request.
                                                         * I-Ring entry write request.
                                                         * I-Ring entry read request.
                                                         * O-Ring entry read request.
                                                         When set to 1, the arbiter is round robin. */
	uint64_t mrq_hwm                      : 2;  /**< MIX CB-request FIFO programmable high watermark.
                                                         The MRQ contains 16 CB-requests which are CSR read/write requests. If the MRQ backs up
                                                         with HWM entries, then new CB-requests are stalled.
                                                         0x0 = HWM is 11.
                                                         0x1 = HWM is 10.
                                                         0x2 = HWM is 9.
                                                         0x3 = HWM is 8. */
#else
	uint64_t mrq_hwm                      : 2;
	uint64_t nbtarb                       : 1;
	uint64_t lendian                      : 1;
	uint64_t reset                        : 1;
	uint64_t en                           : 1;
	uint64_t busy                         : 1;
	uint64_t crc_strip                    : 1;
	uint64_t ts_thresh                    : 4;
	uint64_t reserved_12_63               : 52;
#endif
	} s;
	struct cvmx_mixx_ctl_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t crc_strip                    : 1;  /**< HW CRC Strip Enable
                                                         When enabled, the last 4 bytes(CRC) of the ingress packet
                                                         are not included in cumulative packet byte length.
                                                         In other words, the cumulative LEN field for all
                                                         I-Ring Buffer Entries associated with a given ingress
                                                         packet will be 4 bytes less (so that the final 4B HW CRC
                                                         packet data is not processed by software). */
	uint64_t busy                         : 1;  /**< MIX Busy Status bit
                                                         MIX will assert busy status any time there are:
                                                           1) L2/DRAM reads in-flight (NCB-arb to read
                                                              response)
                                                           2) L2/DRAM writes in-flight (NCB-arb to write
                                                              data is sent.
                                                           3) L2/DRAM write commits in-flight (NCB-arb to write
                                                              commit response).
                                                         NOTE: After MIX_CTL[EN]=0, the MIX will eventually
                                                         complete any "inflight" transactions, at which point the
                                                         BUSY will de-assert. */
	uint64_t en                           : 1;  /**< MIX Enable bit
                                                         When EN=0, MIX will no longer arbitrate for
                                                         any new L2/DRAM read/write requests on the NCB Bus.
                                                         MIX will complete any requests that are currently
                                                         pended for the NCB Bus. */
	uint64_t reset                        : 1;  /**< MIX Soft Reset
                                                          When SW writes a '1' to MIX_CTL[RESET], the
                                                          MII-MIX/AGL logic will execute a soft reset.
                                                          NOTE: During a soft reset, CSR accesses are not effected.
                                                          However, the values of the CSR fields will be effected by
                                                          soft reset (except MIX_CTL[RESET] itself).
                                                          NOTE: After power-on, the MII-AGL/MIX are held in reset
                                                          until the MIX_CTL[RESET] is written to zero. SW MUST also
                                                          perform a MIX_CTL CSR read after this write to ensure the
                                                          soft reset de-assertion has had sufficient time to propagate
                                                          to all MIO-MIX internal logic before any subsequent MIX CSR
                                                          accesses are issued.
                                                          The intended "soft reset" sequence is: (please also
                                                          refer to HRM Section 12.6.2 on MIX/AGL Block Reset).
                                                             1) Write MIX_CTL[EN]=0
                                                                [To prevent any NEW transactions from being started]
                                                             2) Wait for MIX_CTL[BUSY]=0
                                                                [To indicate that all inflight transactions have
                                                                 completed]
                                                             3) Write MIX_CTL[RESET]=1, followed by a MIX_CTL CSR read
                                                                and wait for the result.
                                                             4) Re-Initialize the MIX/AGL just as would be done
                                                                for a hard reset.
                                                         NOTE: Once the MII has been soft-reset, please refer to HRM Section
                                                         12.6.1 MIX/AGL BringUp Sequence to complete the MIX/AGL
                                                         re-initialization sequence. */
	uint64_t lendian                      : 1;  /**< Packet Little Endian Mode
                                                         (0: Big Endian Mode/1: Little Endian Mode)
                                                         When the mode is set, MIX will byte-swap packet data
                                                         loads/stores at the MIX/NCB boundary. */
	uint64_t nbtarb                       : 1;  /**< MIX CB-Request Arbitration Mode.
                                                         When set to zero, the arbiter is fixed priority with
                                                         the following priority scheme:
                                                             Highest Priority: I-Ring Packet Write Request
                                                                               O-Ring Packet Read Request
                                                                               I-Ring Entry Write Request
                                                                               I-Ring Entry Read Request
                                                                               O-Ring Entry Read Request
                                                         When set to one, the arbiter is round robin. */
	uint64_t mrq_hwm                      : 2;  /**< MIX CB-Request FIFO Programmable High Water Mark.
                                                         The MRQ contains 16 CB-Requests which are CSR Rd/Wr
                                                         Requests. If the MRQ backs up with "HWM" entries,
                                                         then new CB-Requests are 'stalled'.
                                                            [0]: HWM = 11
                                                            [1]: HWM = 10
                                                            [2]: HWM = 9
                                                            [3]: HWM = 8
                                                         NOTE: This must only be written at power-on/boot time. */
#else
	uint64_t mrq_hwm                      : 2;
	uint64_t nbtarb                       : 1;
	uint64_t lendian                      : 1;
	uint64_t reset                        : 1;
	uint64_t en                           : 1;
	uint64_t busy                         : 1;
	uint64_t crc_strip                    : 1;
	uint64_t reserved_8_63                : 56;
#endif
	} cn52xx;
	struct cvmx_mixx_ctl_cn52xx           cn52xxp1;
	struct cvmx_mixx_ctl_cn52xx           cn56xx;
	struct cvmx_mixx_ctl_cn52xx           cn56xxp1;
	struct cvmx_mixx_ctl_s                cn61xx;
	struct cvmx_mixx_ctl_s                cn63xx;
	struct cvmx_mixx_ctl_s                cn63xxp1;
	struct cvmx_mixx_ctl_s                cn66xx;
	struct cvmx_mixx_ctl_s                cn68xx;
	struct cvmx_mixx_ctl_s                cn68xxp1;
	struct cvmx_mixx_ctl_s                cn73xx;
	struct cvmx_mixx_ctl_s                cn78xx;
	struct cvmx_mixx_ctl_s                cn78xxp1;
	struct cvmx_mixx_ctl_s                cnf75xx;
};
typedef union cvmx_mixx_ctl cvmx_mixx_ctl_t;

/**
 * cvmx_mix#_intena
 *
 * MIX_INTENA = MIX Local Interrupt Enable Mask Register
 *
 * Description:
 *  NOTE: To write to the MIX_INTENA register, a device would issue an IOBST directed at the MIO.
 *        To read the MIX_INTENA register, a device would issue an IOBLD64 directed at the MIO.
 */
union cvmx_mixx_intena {
	uint64_t u64;
	struct cvmx_mixx_intena_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t tsena                        : 1;  /**< TimeStamp Interrupt Enable
                                                         If both the global interrupt mask bits (CIU2_EN_xx_yy_PKT[MII])
                                                         and this local interrupt mask bit is set, than an
                                                         interrupt is reported for an Outbound Ring with Timestamp
                                                         event (see: MIX_ISR[TS]). */
	uint64_t orunena                      : 1;  /**< ORCNT UnderFlow Detected Enable
                                                         If both the global interrupt mask bits (CIU2_EN_xx_yy_PKT[MII])
                                                         and this local interrupt mask bit is set, than an
                                                         interrupt is reported for an ORCNT underflow condition
                                                         MIX_ISR[ORUN]. */
	uint64_t irunena                      : 1;  /**< IRCNT UnderFlow Interrupt Enable
                                                         If both the global interrupt mask bits (CIU2_EN_xx_yy_PKT[MII])
                                                         and this local interrupt mask bit is set, than an
                                                         interrupt is reported for an IRCNT underflow condition
                                                         MIX_ISR[IRUN]. */
	uint64_t data_drpena                  : 1;  /**< Data was dropped due to RX FIFO full Interrupt
                                                         enable. If both the global interrupt mask bits
                                                         (CIU2_EN_xx_yy_PKT[MII]) and the local interrupt mask
                                                         bit(DATA_DRPENA) is set, than an interrupt is
                                                         reported for this event. */
	uint64_t ithena                       : 1;  /**< Inbound Ring Threshold Exceeded Interrupt Enable
                                                         If both the global interrupt mask bits (CIU2_EN_xx_yy_PKT[MII])
                                                         and this local interrupt mask bit is set, than an
                                                         interrupt is reported for an Inbound Ring Threshold
                                                         Exceeded event(IRTHRESH). */
	uint64_t othena                       : 1;  /**< Outbound Ring Threshold Exceeded Interrupt Enable
                                                         If both the global interrupt mask bits (CIU2_EN_xx_yy_PKT[MII])
                                                         and this local interrupt mask bit is set, than an
                                                         interrupt is reported for an Outbound Ring Threshold
                                                         Exceeded event(ORTHRESH). */
	uint64_t ivfena                       : 1;  /**< Inbound DoorBell(IDBELL) Overflow Detected
                                                         If both the global interrupt mask bits (CIU2_EN_xx_yy_PKT[MII])
                                                         and this local interrupt mask bit is set, than an
                                                         interrupt is reported for an Inbound Doorbell Overflow
                                                         event(IDBOVF). */
	uint64_t ovfena                       : 1;  /**< Outbound DoorBell(ODBELL) Overflow Interrupt Enable
                                                         If both the global interrupt mask bits (CIU2_EN_xx_yy_PKT[MII])
                                                         and this local interrupt mask bit is set, than an
                                                         interrupt is reported for an Outbound Doorbell Overflow
                                                         event(ODBOVF). */
#else
	uint64_t ovfena                       : 1;
	uint64_t ivfena                       : 1;
	uint64_t othena                       : 1;
	uint64_t ithena                       : 1;
	uint64_t data_drpena                  : 1;
	uint64_t irunena                      : 1;
	uint64_t orunena                      : 1;
	uint64_t tsena                        : 1;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_mixx_intena_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t orunena                      : 1;  /**< ORCNT UnderFlow Detected
                                                         If both the global interrupt mask bits (CIU_INTx_EN*[MII])
                                                         and this local interrupt mask bit is set, than an
                                                         interrupt is reported for an ORCNT underflow condition
                                                         MIX_ISR[ORUN]. */
	uint64_t irunena                      : 1;  /**< IRCNT UnderFlow Interrupt Enable
                                                         If both the global interrupt mask bits (CIU_INTx_EN*[MII])
                                                         and this local interrupt mask bit is set, than an
                                                         interrupt is reported for an IRCNT underflow condition
                                                         MIX_ISR[IRUN]. */
	uint64_t data_drpena                  : 1;  /**< Data was dropped due to RX FIFO full Interrupt
                                                         enable. If both the global interrupt mask bits
                                                         (CIU_INTx_EN*[MII]) and the local interrupt mask
                                                         bit(DATA_DRPENA) is set, than an interrupt is
                                                         reported for this event. */
	uint64_t ithena                       : 1;  /**< Inbound Ring Threshold Exceeded Interrupt Enable
                                                         If both the global interrupt mask bits (CIU_INTx_EN*[MII])
                                                         and this local interrupt mask bit is set, than an
                                                         interrupt is reported for an Inbound Ring Threshold
                                                         Exceeded event(IRTHRESH). */
	uint64_t othena                       : 1;  /**< Outbound Ring Threshold Exceeded Interrupt Enable
                                                         If both the global interrupt mask bits (CIU_INTx_EN*[MII])
                                                         and this local interrupt mask bit is set, than an
                                                         interrupt is reported for an Outbound Ring Threshold
                                                         Exceeded event(ORTHRESH). */
	uint64_t ivfena                       : 1;  /**< Inbound DoorBell(IDBELL) Overflow Detected
                                                         If both the global interrupt mask bits (CIU_INTx_EN*[MII])
                                                         and this local interrupt mask bit is set, than an
                                                         interrupt is reported for an Inbound Doorbell Overflow
                                                         event(IDBOVF). */
	uint64_t ovfena                       : 1;  /**< Outbound DoorBell(ODBELL) Overflow Interrupt Enable
                                                         If both the global interrupt mask bits (CIU_INTx_EN*[MII])
                                                         and this local interrupt mask bit is set, than an
                                                         interrupt is reported for an Outbound Doorbell Overflow
                                                         event(ODBOVF). */
#else
	uint64_t ovfena                       : 1;
	uint64_t ivfena                       : 1;
	uint64_t othena                       : 1;
	uint64_t ithena                       : 1;
	uint64_t data_drpena                  : 1;
	uint64_t irunena                      : 1;
	uint64_t orunena                      : 1;
	uint64_t reserved_7_63                : 57;
#endif
	} cn52xx;
	struct cvmx_mixx_intena_cn52xx        cn52xxp1;
	struct cvmx_mixx_intena_cn52xx        cn56xx;
	struct cvmx_mixx_intena_cn52xx        cn56xxp1;
	struct cvmx_mixx_intena_s             cn61xx;
	struct cvmx_mixx_intena_s             cn63xx;
	struct cvmx_mixx_intena_s             cn63xxp1;
	struct cvmx_mixx_intena_s             cn66xx;
	struct cvmx_mixx_intena_s             cn68xx;
	struct cvmx_mixx_intena_s             cn68xxp1;
};
typedef union cvmx_mixx_intena cvmx_mixx_intena_t;

/**
 * cvmx_mix#_ircnt
 *
 * MIX_IRCNT = MIX I-Ring Pending Packet Counter
 *
 * Description:
 *  NOTE: To write to the MIX_IRCNT register, a device would issue an IOBST directed at the MIO.
 *        To read the MIX_IRCNT register, a device would issue an IOBLD64 directed at the MIO.
 */
union cvmx_mixx_ircnt {
	uint64_t u64;
	struct cvmx_mixx_ircnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t ircnt                        : 20; /**< Pending number of I-Ring packets.
                                                         Whenever hardware writes a completion code of DONE, TRUNC, CRCERR or ERR, it increments
                                                         the IRCNT (to indicate to software the number of pending Input packets in system memory).
                                                         The hardware guarantees that the completion code write is always visible in system memory
                                                         before it increments the IRCNT.
                                                         Reading IRCNT returns the current inbound packet count.
                                                         Writing IRCNT decrements the count by the value written.
                                                         This register is used to generate interrupts to alert software of pending inbound MIX
                                                         packets in system memory.
                                                         In the case of inbound packets that span multiple I-Ring entries, software must keep track
                                                         of the number of I-Ring Entries associated with a given inbound packet to reclaim the
                                                         proper number of I-Ring Entries for re-use. */
#else
	uint64_t ircnt                        : 20;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_mixx_ircnt_s              cn52xx;
	struct cvmx_mixx_ircnt_s              cn52xxp1;
	struct cvmx_mixx_ircnt_s              cn56xx;
	struct cvmx_mixx_ircnt_s              cn56xxp1;
	struct cvmx_mixx_ircnt_s              cn61xx;
	struct cvmx_mixx_ircnt_s              cn63xx;
	struct cvmx_mixx_ircnt_s              cn63xxp1;
	struct cvmx_mixx_ircnt_s              cn66xx;
	struct cvmx_mixx_ircnt_s              cn68xx;
	struct cvmx_mixx_ircnt_s              cn68xxp1;
	struct cvmx_mixx_ircnt_s              cn73xx;
	struct cvmx_mixx_ircnt_s              cn78xx;
	struct cvmx_mixx_ircnt_s              cn78xxp1;
	struct cvmx_mixx_ircnt_s              cnf75xx;
};
typedef union cvmx_mixx_ircnt cvmx_mixx_ircnt_t;

/**
 * cvmx_mix#_irhwm
 *
 * MIX_IRHWM = MIX I-Ring High-Water Mark Threshold Register
 *
 * Description:
 *  NOTE: To write to the MIX_IHWM register, a device would issue an IOBST directed at the MIO.
 *        To read the MIX_IHWM register, a device would issue an IOBLD64 directed at the MIO.
 */
union cvmx_mixx_irhwm {
	uint64_t u64;
	struct cvmx_mixx_irhwm_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_40_63               : 24;
	uint64_t ibplwm                       : 20; /**< I-Ring backpressure low-watermark threshold.
                                                         When the number of available I-Ring entries (IDBELL) is less than IBPLWM, the BGX-MAC does
                                                         the following:
                                                         * in full-duplex mode: send periodic PAUSE packets.
                                                         * in half-duplex mode: force collisions.
                                                         This programmable mechanism is provided as a means to backpressure input traffic early
                                                         enough so that packets are not dropped by CNXXXX. */
	uint64_t irhwm                        : 20; /**< I-Ring entry high-watermark threshold. Used to determine when the number of inbound
                                                         packets in system memory (MIX(0..1)_IRCNT[IRCNT]) exceeds this IRHWM threshold. */
#else
	uint64_t irhwm                        : 20;
	uint64_t ibplwm                       : 20;
	uint64_t reserved_40_63               : 24;
#endif
	} s;
	struct cvmx_mixx_irhwm_s              cn52xx;
	struct cvmx_mixx_irhwm_s              cn52xxp1;
	struct cvmx_mixx_irhwm_s              cn56xx;
	struct cvmx_mixx_irhwm_s              cn56xxp1;
	struct cvmx_mixx_irhwm_s              cn61xx;
	struct cvmx_mixx_irhwm_s              cn63xx;
	struct cvmx_mixx_irhwm_s              cn63xxp1;
	struct cvmx_mixx_irhwm_s              cn66xx;
	struct cvmx_mixx_irhwm_s              cn68xx;
	struct cvmx_mixx_irhwm_s              cn68xxp1;
	struct cvmx_mixx_irhwm_s              cn73xx;
	struct cvmx_mixx_irhwm_s              cn78xx;
	struct cvmx_mixx_irhwm_s              cn78xxp1;
	struct cvmx_mixx_irhwm_s              cnf75xx;
};
typedef union cvmx_mixx_irhwm cvmx_mixx_irhwm_t;

/**
 * cvmx_mix#_iring1
 *
 * MIX_IRING1 = MIX Inbound Ring Register \#1
 *
 * Description:
 *  NOTE: To write to the MIX_IRING1 register, a device would issue an IOBST directed at the MIO.
 *        To read the MIX_IRING1 register, a device would issue an IOBLD64 directed at the MIO.
 */
union cvmx_mixx_iring1 {
	uint64_t u64;
	struct cvmx_mixx_iring1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_mixx_iring1_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t isize                        : 20; /**< Represents the Inbound Ring Buffer's Size(in 8B
                                                         words). The ring can be as large as 1M entries.
                                                         NOTE: This CSR MUST BE setup written by SW poweron
                                                         (when IDBELL/IRCNT=0). */
	uint64_t reserved_36_39               : 4;
	uint64_t ibase                        : 33; /**< Represents the 8B-aligned base address of the first
                                                         Inbound Ring entry in system memory.
                                                         NOTE: SW MUST ONLY write to this register during
                                                         power-on/boot code. */
	uint64_t reserved_0_2                 : 3;
#else
	uint64_t reserved_0_2                 : 3;
	uint64_t ibase                        : 33;
	uint64_t reserved_36_39               : 4;
	uint64_t isize                        : 20;
	uint64_t reserved_60_63               : 4;
#endif
	} cn52xx;
	struct cvmx_mixx_iring1_cn52xx        cn52xxp1;
	struct cvmx_mixx_iring1_cn52xx        cn56xx;
	struct cvmx_mixx_iring1_cn52xx        cn56xxp1;
	struct cvmx_mixx_iring1_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t isize                        : 20; /**< Represents the Inbound Ring Buffer's Size(in 8B
                                                         words). The ring can be as large as 1M entries.
                                                         NOTE: This CSR MUST BE setup written by SW poweron
                                                         (when IDBELL/IRCNT=0). */
	uint64_t ibase                        : 37; /**< Represents the 8B-aligned base address of the first
                                                         Inbound Ring entry in system memory.
                                                         NOTE: SW MUST ONLY write to this register during
                                                         power-on/boot code. */
	uint64_t reserved_0_2                 : 3;
#else
	uint64_t reserved_0_2                 : 3;
	uint64_t ibase                        : 37;
	uint64_t isize                        : 20;
	uint64_t reserved_60_63               : 4;
#endif
	} cn61xx;
	struct cvmx_mixx_iring1_cn61xx        cn63xx;
	struct cvmx_mixx_iring1_cn61xx        cn63xxp1;
	struct cvmx_mixx_iring1_cn61xx        cn66xx;
	struct cvmx_mixx_iring1_cn61xx        cn68xx;
	struct cvmx_mixx_iring1_cn61xx        cn68xxp1;
	struct cvmx_mixx_iring1_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t isize                        : 20; /**< Represents the inbound ring (I-Ring) buffer's size (in 8-byte words). The ring can be as
                                                         large as 1MB entries.
                                                         This CSR must be setup written by software poweron (when IDBELL/IRCNT=0). */
	uint64_t reserved_42_43               : 2;
	uint64_t ibase                        : 39; /**< Represents the 8-byte aligned base address of the first inbound ring
                                                         (I-Ring) entry in system memory.
                                                         Software must only write to this register during power-on/boot code. */
	uint64_t reserved_0_2                 : 3;
#else
	uint64_t reserved_0_2                 : 3;
	uint64_t ibase                        : 39;
	uint64_t reserved_42_43               : 2;
	uint64_t isize                        : 20;
#endif
	} cn73xx;
	struct cvmx_mixx_iring1_cn73xx        cn78xx;
	struct cvmx_mixx_iring1_cn73xx        cn78xxp1;
	struct cvmx_mixx_iring1_cn73xx        cnf75xx;
};
typedef union cvmx_mixx_iring1 cvmx_mixx_iring1_t;

/**
 * cvmx_mix#_iring2
 *
 * MIX_IRING2 = MIX Inbound Ring Register \#2
 *
 * Description:
 *  NOTE: To write to the MIX_IRING2 register, a device would issue an IOBST directed at the MIO.
 *        To read the MIX_IRING2 register, a device would issue an IOBLD64 directed at the MIO.
 */
union cvmx_mixx_iring2 {
	uint64_t u64;
	struct cvmx_mixx_iring2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_52_63               : 12;
	uint64_t itlptr                       : 20; /**< The inbound ring (I-Ring) tail pointer selects the I-Ring entry that the hardware will
                                                         process next. After the hardware completes receiving an inbound packet, it increments the
                                                         I-Ring tail pointer.
                                                         The I-Ring tail pointer hardware increment is always modulo MIX(0..1)_IRING2[ISIZE].
                                                         This field is read-only to software. */
	uint64_t reserved_20_31               : 12;
	uint64_t idbell                       : 20; /**< Represents the cumulative total of pending inbound ring (I-Ring) buffer entries. Each
                                                         I-Ring buffer entry contains an L2/DRAM byte pointer and a byte Length.
                                                         After software inserts a new entry into the I-Ring buffer, it 'rings the doorbell for the
                                                         inbound ring.' When the MIX hardware receives the doorbell ring, it advances the doorbell
                                                         count for the I-Ring.
                                                         Software must never cause the doorbell count for the I-Ring to exceed the size of
                                                         MIX(0..1)_IRING1[ISIZE]. A read of the CSR indicates the current doorbell count. */
#else
	uint64_t idbell                       : 20;
	uint64_t reserved_20_31               : 12;
	uint64_t itlptr                       : 20;
	uint64_t reserved_52_63               : 12;
#endif
	} s;
	struct cvmx_mixx_iring2_s             cn52xx;
	struct cvmx_mixx_iring2_s             cn52xxp1;
	struct cvmx_mixx_iring2_s             cn56xx;
	struct cvmx_mixx_iring2_s             cn56xxp1;
	struct cvmx_mixx_iring2_s             cn61xx;
	struct cvmx_mixx_iring2_s             cn63xx;
	struct cvmx_mixx_iring2_s             cn63xxp1;
	struct cvmx_mixx_iring2_s             cn66xx;
	struct cvmx_mixx_iring2_s             cn68xx;
	struct cvmx_mixx_iring2_s             cn68xxp1;
	struct cvmx_mixx_iring2_s             cn73xx;
	struct cvmx_mixx_iring2_s             cn78xx;
	struct cvmx_mixx_iring2_s             cn78xxp1;
	struct cvmx_mixx_iring2_s             cnf75xx;
};
typedef union cvmx_mixx_iring2 cvmx_mixx_iring2_t;

/**
 * cvmx_mix#_isr
 *
 * This register provides a summary of the MIX interrupt bits.
 *
 */
union cvmx_mixx_isr {
	uint64_t u64;
	struct cvmx_mixx_isr_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t ts                           : 1;  /**< Timestamp interrupt. Throws MIX_INTSN_E:MIX(0..1)_INT_TS. This bit is set and the
                                                         interrupt generated when the number of pending timestamp interrupts
                                                         (MIX(0..1)_TSCTL[TSCNT]) is greater than the timestamp interrupt threshold
                                                         (MIX(0..1)_CTL[TS_THRESH]) value. */
	uint64_t orun                         : 1;  /**< O-ring packet count underflow detected. Throws MIX_INTSN_E::MIX(0..1)_INT_ORUN. If
                                                         software writes a larger value than what is currently in the MIX(0..1)_ORCNT[ORCNT], then
                                                         hardware reports the underflow condition.
                                                         The MIX(0..1)_ORCNT[IOCNT] will clamp to zero.
                                                         If an ORUN underflow condition is detected, the integrity of the MIX hardware state has
                                                         been compromised. To recover, software must issue a software reset sequence. (See
                                                         MIX(0..1)_CTL[RESET.] */
	uint64_t irun                         : 1;  /**< I-ring packet count underflow detected. Throws MIX_INTSN_E::MIX(0..1)_INT_IRUN. If
                                                         software writes a larger value than what is currently in the MIX(0..1)_IRCNT[IRCNT], then
                                                         hardware reports the underflow condition.
                                                         The MIX(0..1)_IRCNT[IRCNT] will clamp to zero.
                                                         If an IRUN underflow condition is detected, the integrity of the MIX hardware state has
                                                         been compromised. To recover, software must issue a software reset sequence. (See
                                                         MIX(0..1)_CTL[RESET]). */
	uint64_t data_drp                     : 1;  /**< Data was dropped due to RX FIFO full. Throws MIX_INTSN_E::MIX(0..1)_INT_DATA_DRP. If this
                                                         event does occur, DATA_DRP is set and the interrupt is generated. */
	uint64_t irthresh                     : 1;  /**< Inbound ring packet threshold exceeded. Throws MIX_INTSN_E::MIX(0..1)_INT_IRTHRESH. When
                                                         the pending number of inbound packets in system memory (IRCNT) has exceeded a programmable
                                                         threshold (IRHWM), this bit is set and the interrupt is generated. To service this
                                                         interrupt, the IRCNT must first be lowered below the IRHWM before the W1C to this field. */
	uint64_t orthresh                     : 1;  /**< Outbound ring packet threshold exceeded. Throws MIX_INTSN_E::MIX(0..1)_INT_ORTHRESH. When
                                                         the pending number of outbound packets in system memory (ORCNT) has exceeded a
                                                         programmable threshold (ORHWM), this bit is set and the interrupt is generated. To service
                                                         this interrupt, the ORCNT must first be lowered below the ORHWM before the W1C to this
                                                         field. */
	uint64_t idblovf                      : 1;  /**< Inbound doorbell (IDBELL) overflow detected. Throws MIX_INTSN_E::MIX(0..1)_INT_IDBLOVF.
                                                         If software attempts to write to the MIX(0..1)_IRING2[IDBELL] with a value greater than
                                                         the remaining number of I-Ring buffer entries (MIX(0..1)_REMCNT[IREMCNT]), then the
                                                         following occurs:
                                                         * The MIX(0..1)_IRING2[IDBELL] write is IGNORED.
                                                         * [IDBLOVF] is set and the interrupt is generated.
                                                         Software should keep track of the number of I-Ring entries in use (i.e. the cumulative
                                                         number
                                                         of IDBELL write operations), and ensure that future IDBELL write operations don't exceed
                                                         the size of the I-Ring Buffer (MIX(0..1)_IRING2[ISIZE]). Software must reclaim I-Ring
                                                         entries by keeping track of the number of I-Ring entries, and writing to the
                                                         MIX(0..1)_IRCNT[IRCNT].
                                                         The MIX(0..1)_IRCNT[IRCNT] register represents the total number of packets (not I-Ring
                                                         entries) and software must further keep track of the number of I-Ring entries associated
                                                         with each packet as they are processed.
                                                         If an [IDBLOVF] occurs, it is an indication that software has overwritten the
                                                         I-Ring buffer, and the only recourse for recovery is a hardware reset. */
	uint64_t odblovf                      : 1;  /**< Outbound doorbell (ODBELL) overflow detected. Throws MIX_INTSN_E::MIX(0..1)_INT_ODBLOVF.
                                                         If software attempts to write to MIX(0..1)_ORING2[ODBELL] with a value greater than the
                                                         remaining number of O-Ring buffer entries (MIX(0..1)_REMCNT[OREMCNT]), then the following
                                                         occurs:
                                                         * The MIX(0..1)_IRING2[ODBELL] write operation is IGNORED.
                                                         * [ODBLOVF] is set and the interrupt is generated.
                                                         Software should keep track of the number of I-Ring entries in use (i.e. the cumulative
                                                         number of ODBELL write operations), and ensure that future ODBELL write operations don't
                                                         exceed the size of the O-Ring buffer (MIX(0..1)_ORING2[OSIZE]). Software must reclaim
                                                         O-Ring entries by writing to MIX(0..1)_ORCNT[ORCNT].
                                                         If an [ODBLOVF] occurs, it is an indication that software has overwritten the
                                                         O-Ring buffer, and the only recourse for recovery is a hardware reset. */
#else
	uint64_t odblovf                      : 1;
	uint64_t idblovf                      : 1;
	uint64_t orthresh                     : 1;
	uint64_t irthresh                     : 1;
	uint64_t data_drp                     : 1;
	uint64_t irun                         : 1;
	uint64_t orun                         : 1;
	uint64_t ts                           : 1;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_mixx_isr_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_7_63                : 57;
	uint64_t orun                         : 1;  /**< ORCNT UnderFlow Detected
                                                         If SW writes a larger value than what is currently
                                                         in the MIX_ORCNT[ORCNT], then HW will report the
                                                         underflow condition.
                                                         NOTE: The MIX_ORCNT[IOCNT] will clamp to to zero.
                                                         NOTE: If an ORUN underflow condition is detected,
                                                         the integrity of the MIX/AGL HW state has
                                                         been compromised. To recover, SW must issue a
                                                         software reset sequence (see: MIX_CTL[RESET] */
	uint64_t irun                         : 1;  /**< IRCNT UnderFlow Detected
                                                         If SW writes a larger value than what is currently
                                                         in the MIX_IRCNT[IRCNT], then HW will report the
                                                         underflow condition.
                                                         NOTE: The MIX_IRCNT[IRCNT] will clamp to to zero.
                                                         NOTE: If an IRUN underflow condition is detected,
                                                         the integrity of the MIX/AGL HW state has
                                                         been compromised. To recover, SW must issue a
                                                         software reset sequence (see: MIX_CTL[RESET] */
	uint64_t data_drp                     : 1;  /**< Data was dropped due to RX FIFO full
                                                         If this does occur, the DATA_DRP is set and the
                                                         CIU_INTx_SUM0,4[MII] bits are set.
                                                         If both the global interrupt mask bits (CIU_INTx_EN*[MII])
                                                         and the local interrupt mask bit(DATA_DRPENA) is set, than an
                                                         interrupt is reported for this event. */
	uint64_t irthresh                     : 1;  /**< Inbound Ring Packet Threshold Exceeded
                                                         When the pending \#inbound packets in system
                                                         memory(IRCNT) has exceeded a programmable threshold
                                                         (IRHWM), then this bit is set. If this does occur,
                                                         the IRTHRESH is set and the CIU_INTx_SUM0,4[MII] bits
                                                         are set if ((MIX_ISR & MIX_INTENA) != 0)).
                                                         If both the global interrupt mask bits (CIU_INTx_EN*[MII])
                                                         and the local interrupt mask bit(ITHENA) is set, than an
                                                         interrupt is reported for this event. */
	uint64_t orthresh                     : 1;  /**< Outbound Ring Packet Threshold Exceeded
                                                         When the pending \#outbound packets in system
                                                         memory(ORCNT) has exceeded a programmable threshold
                                                         (ORHWM), then this bit is set. If this does occur,
                                                         the ORTHRESH is set and the CIU_INTx_SUM0,4[MII] bits
                                                         are set if ((MIX_ISR & MIX_INTENA) != 0)).
                                                         If both the global interrupt mask bits (CIU_INTx_EN*[MII])
                                                         and the local interrupt mask bit(OTHENA) is set, than an
                                                         interrupt is reported for this event. */
	uint64_t idblovf                      : 1;  /**< Inbound DoorBell(IDBELL) Overflow Detected
                                                         If SW attempts to write to the MIX_IRING2[IDBELL]
                                                         with a value greater than the remaining \#of
                                                         I-Ring Buffer Entries (MIX_REMCNT[IREMCNT]), then
                                                         the following occurs:
                                                         1) The  MIX_IRING2[IDBELL] write is IGNORED
                                                         2) The ODBLOVF is set and the CIU_INTx_SUM0,4[MII]
                                                            bits are set if ((MIX_ISR & MIX_INTENA) != 0)).
                                                         If both the global interrupt mask bits (CIU_INTx_EN*[MII])
                                                         and the local interrupt mask bit(IVFENA) is set, than an
                                                         interrupt is reported for this event.
                                                         SW should keep track of the \#I-Ring Entries in use
                                                         (ie: cumulative \# of IDBELL writes),  and ensure that
                                                         future IDBELL writes don't exceed the size of the
                                                         I-Ring Buffer (MIX_IRING2[ISIZE]).
                                                         SW must reclaim I-Ring Entries by keeping track of the
                                                         \#IRing-Entries, and writing to the MIX_IRCNT[IRCNT].
                                                         NOTE: The MIX_IRCNT[IRCNT] register represents the
                                                         total \#packets(not IRing Entries) and SW must further
                                                         keep track of the \# of I-Ring Entries associated with
                                                         each packet as they are processed.
                                                         NOTE: There is no recovery from an IDBLOVF Interrupt.
                                                         If it occurs, it's an indication that SW has
                                                         overwritten the I-Ring buffer, and the only recourse
                                                         is a HW reset. */
	uint64_t odblovf                      : 1;  /**< Outbound DoorBell(ODBELL) Overflow Detected
                                                         If SW attempts to write to the MIX_ORING2[ODBELL]
                                                         with a value greater than the remaining \#of
                                                         O-Ring Buffer Entries (MIX_REMCNT[OREMCNT]), then
                                                         the following occurs:
                                                         1) The  MIX_ORING2[ODBELL] write is IGNORED
                                                         2) The ODBLOVF is set and the CIU_INTx_SUM0,4[MII]
                                                            bits are set if ((MIX_ISR & MIX_INTENA) != 0)).
                                                         If both the global interrupt mask bits (CIU_INTx_EN*[MII])
                                                         and the local interrupt mask bit(OVFENA) is set, than an
                                                         interrupt is reported for this event.
                                                         SW should keep track of the \#I-Ring Entries in use
                                                         (ie: cumulative \# of ODBELL writes),  and ensure that
                                                         future ODBELL writes don't exceed the size of the
                                                         O-Ring Buffer (MIX_ORING2[OSIZE]).
                                                         SW must reclaim O-Ring Entries by writing to the
                                                         MIX_ORCNT[ORCNT]. .
                                                         NOTE: There is no recovery from an ODBLOVF Interrupt.
                                                         If it occurs, it's an indication that SW has
                                                         overwritten the O-Ring buffer, and the only recourse
                                                         is a HW reset. */
#else
	uint64_t odblovf                      : 1;
	uint64_t idblovf                      : 1;
	uint64_t orthresh                     : 1;
	uint64_t irthresh                     : 1;
	uint64_t data_drp                     : 1;
	uint64_t irun                         : 1;
	uint64_t orun                         : 1;
	uint64_t reserved_7_63                : 57;
#endif
	} cn52xx;
	struct cvmx_mixx_isr_cn52xx           cn52xxp1;
	struct cvmx_mixx_isr_cn52xx           cn56xx;
	struct cvmx_mixx_isr_cn52xx           cn56xxp1;
	struct cvmx_mixx_isr_s                cn61xx;
	struct cvmx_mixx_isr_s                cn63xx;
	struct cvmx_mixx_isr_s                cn63xxp1;
	struct cvmx_mixx_isr_s                cn66xx;
	struct cvmx_mixx_isr_s                cn68xx;
	struct cvmx_mixx_isr_s                cn68xxp1;
	struct cvmx_mixx_isr_s                cn73xx;
	struct cvmx_mixx_isr_s                cn78xx;
	struct cvmx_mixx_isr_s                cn78xxp1;
	struct cvmx_mixx_isr_s                cnf75xx;
};
typedef union cvmx_mixx_isr cvmx_mixx_isr_t;

/**
 * cvmx_mix#_isr_w1s
 */
union cvmx_mixx_isr_w1s {
	uint64_t u64;
	struct cvmx_mixx_isr_w1s_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_8_63                : 56;
	uint64_t ts                           : 1;  /**< Timestamp interrupt. Throws MIX_INTSN_E:MIX(0..1)_INT_TS. This bit is set and the
                                                         interrupt generated when the number of pending timestamp interrupts
                                                         (MIX(0..1)_TSCTL[TSCNT]) is greater than the timestamp interrupt threshold
                                                         (MIX(0..1)_CTL[TS_THRESH]) value. */
	uint64_t orun                         : 1;  /**< O-ring packet count underflow detected. Throws MIX_INTSN_E::MIX(0..1)_INT_ORUN. If
                                                         software writes a larger value than what is currently in the MIX(0..1)_ORCNT[ORCNT], then
                                                         hardware reports the underflow condition.
                                                         The MIX(0..1)_ORCNT[IOCNT] will clamp to zero.
                                                         If an ORUN underflow condition is detected, the integrity of the MIX hardware state has
                                                         been compromised. To recover, software must issue a software reset sequence. (See
                                                         MIX(0..1)_CTL[RESET.] */
	uint64_t irun                         : 1;  /**< I-ring packet count underflow detected. Throws MIX_INTSN_E::MIX(0..1)_INT_IRUN. If
                                                         software writes a larger value than what is currently in the MIX(0..1)_IRCNT[IRCNT], then
                                                         hardware reports the underflow condition.
                                                         The MIX(0..1)_IRCNT[IRCNT] will clamp to zero.
                                                         If an IRUN underflow condition is detected, the integrity of the MIX hardware state has
                                                         been compromised. To recover, software must issue a software reset sequence. (See
                                                         MIX(0..1)_CTL[RESET]). */
	uint64_t data_drp                     : 1;  /**< Data was dropped due to RX FIFO full. Throws MIX_INTSN_E::MIX(0..1)_INT_DATA_DRP. If this
                                                         event does occur, DATA_DRP is set and the interrupt is generated. */
	uint64_t irthresh                     : 1;  /**< Inbound ring packet threshold exceeded. Throws MIX_INTSN_E::MIX(0..1)_INT_IRTHRESH. When
                                                         the pending number of inbound packets in system memory (IRCNT) has exceeded a programmable
                                                         threshold (IRHWM), this bit is set and the interrupt is generated. To service this
                                                         interrupt, the IRCNT must first be lowered below the IRHWM before the W1C to this field. */
	uint64_t orthresh                     : 1;  /**< Outbound ring packet threshold exceeded. Throws MIX_INTSN_E::MIX(0..1)_INT_ORTHRESH. When
                                                         the pending number of outbound packets in system memory (ORCNT) has exceeded a
                                                         programmable threshold (ORHWM), this bit is set and the interrupt is generated. To service
                                                         this interrupt, the ORCNT must first be lowered below the ORHWM before the W1C to this
                                                         field. */
	uint64_t idblovf                      : 1;  /**< Inbound doorbell (IDBELL) overflow detected. Throws MIX_INTSN_E::MIX(0..1)_INT_IDBLOVF.
                                                         If software attempts to write to the MIX(0..1)_IRING2[IDBELL] with a value greater than
                                                         the remaining number of I-Ring buffer entries (MIX(0..1)_REMCNT[IREMCNT]), then the
                                                         following occurs:
                                                         * The MIX(0..1)_IRING2[IDBELL] write is IGNORED.
                                                         * [IDBLOVF] is set and the interrupt is generated.
                                                         Software should keep track of the number of I-Ring entries in use (i.e. the cumulative
                                                         number
                                                         of IDBELL write operations), and ensure that future IDBELL write operations don't exceed
                                                         the size of the I-Ring Buffer (MIX(0..1)_IRING2[ISIZE]). Software must reclaim I-Ring
                                                         entries by keeping track of the number of I-Ring entries, and writing to the
                                                         MIX(0..1)_IRCNT[IRCNT].
                                                         The MIX(0..1)_IRCNT[IRCNT] register represents the total number of packets (not I-Ring
                                                         entries) and software must further keep track of the number of I-Ring entries associated
                                                         with each packet as they are processed.
                                                         If an [IDBLOVF] occurs, it is an indication that software has overwritten the
                                                         I-Ring buffer, and the only recourse for recovery is a hardware reset. */
	uint64_t odblovf                      : 1;  /**< Outbound doorbell (ODBELL) overflow detected. Throws MIX_INTSN_E::MIX(0..1)_INT_ODBLOVF.
                                                         If software attempts to write to MIX(0..1)_ORING2[ODBELL] with a value greater than the
                                                         remaining number of O-Ring buffer entries (MIX(0..1)_REMCNT[OREMCNT]), then the following
                                                         occurs:
                                                         * The MIX(0..1)_IRING2[ODBELL] write operation is IGNORED.
                                                         * [ODBLOVF] is set and the interrupt is generated.
                                                         Software should keep track of the number of I-Ring entries in use (i.e. the cumulative
                                                         number of ODBELL write operations), and ensure that future ODBELL write operations don't
                                                         exceed the size of the O-Ring buffer (MIX(0..1)_ORING2[OSIZE]). Software must reclaim
                                                         O-Ring entries by writing to MIX(0..1)_ORCNT[ORCNT].
                                                         If an [ODBLOVF] occurs, it is an indication that software has overwritten the
                                                         O-Ring buffer, and the only recourse for recovery is a hardware reset. */
#else
	uint64_t odblovf                      : 1;
	uint64_t idblovf                      : 1;
	uint64_t orthresh                     : 1;
	uint64_t irthresh                     : 1;
	uint64_t data_drp                     : 1;
	uint64_t irun                         : 1;
	uint64_t orun                         : 1;
	uint64_t ts                           : 1;
	uint64_t reserved_8_63                : 56;
#endif
	} s;
	struct cvmx_mixx_isr_w1s_s            cn73xx;
	struct cvmx_mixx_isr_w1s_s            cn78xx;
	struct cvmx_mixx_isr_w1s_s            cnf75xx;
};
typedef union cvmx_mixx_isr_w1s cvmx_mixx_isr_w1s_t;

/**
 * cvmx_mix#_orcnt
 *
 * MIX_ORCNT = MIX O-Ring Packets Sent Counter
 *
 * Description:
 *  NOTE: To write to the MIX_ORCNT register, a device would issue an IOBST directed at the MIO.
 *        To read the MIX_ORCNT register, a device would issue an IOBLD64 directed at the MIO.
 */
union cvmx_mixx_orcnt {
	uint64_t u64;
	struct cvmx_mixx_orcnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t orcnt                        : 20; /**< Pending number of O-Ring packets.
                                                         Whenever hardware removes a packet from the O-Ring, it increments the ORCNT (to indicate
                                                         to software the number of Output packets in system memory that can be reclaimed).
                                                         Reading ORCNT returns the current count.
                                                         Writing ORCNT decrements the count by the value written.
                                                         This register is used to generate interrupts to alert software of pending outbound MIX
                                                         packets that have been removed from system memory. (See MIX(0..1)_ISR[ORTHRESH]
                                                         description for more details.)
                                                         For outbound packets, the number of O-Ring packets is equal to the number of O-Ring
                                                         Entries. */
#else
	uint64_t orcnt                        : 20;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_mixx_orcnt_s              cn52xx;
	struct cvmx_mixx_orcnt_s              cn52xxp1;
	struct cvmx_mixx_orcnt_s              cn56xx;
	struct cvmx_mixx_orcnt_s              cn56xxp1;
	struct cvmx_mixx_orcnt_s              cn61xx;
	struct cvmx_mixx_orcnt_s              cn63xx;
	struct cvmx_mixx_orcnt_s              cn63xxp1;
	struct cvmx_mixx_orcnt_s              cn66xx;
	struct cvmx_mixx_orcnt_s              cn68xx;
	struct cvmx_mixx_orcnt_s              cn68xxp1;
	struct cvmx_mixx_orcnt_s              cn73xx;
	struct cvmx_mixx_orcnt_s              cn78xx;
	struct cvmx_mixx_orcnt_s              cn78xxp1;
	struct cvmx_mixx_orcnt_s              cnf75xx;
};
typedef union cvmx_mixx_orcnt cvmx_mixx_orcnt_t;

/**
 * cvmx_mix#_orhwm
 *
 * MIX_ORHWM = MIX O-Ring High-Water Mark Threshold Register
 *
 * Description:
 *  NOTE: To write to the MIX_ORHWM register, a device would issue an IOBST directed at the MIO.
 *        To read the MIX_ORHWM register, a device would issue an IOBLD64 directed at the MIO.
 */
union cvmx_mixx_orhwm {
	uint64_t u64;
	struct cvmx_mixx_orhwm_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_20_63               : 44;
	uint64_t orhwm                        : 20; /**< O-Ring entry high-watermark threshold. Used to determine when the number of outbound
                                                         packets in system memory that can be reclaimed (MIX(0..1)_ORCNT[ORCNT]) exceeds this ORHWM
                                                         threshold. */
#else
	uint64_t orhwm                        : 20;
	uint64_t reserved_20_63               : 44;
#endif
	} s;
	struct cvmx_mixx_orhwm_s              cn52xx;
	struct cvmx_mixx_orhwm_s              cn52xxp1;
	struct cvmx_mixx_orhwm_s              cn56xx;
	struct cvmx_mixx_orhwm_s              cn56xxp1;
	struct cvmx_mixx_orhwm_s              cn61xx;
	struct cvmx_mixx_orhwm_s              cn63xx;
	struct cvmx_mixx_orhwm_s              cn63xxp1;
	struct cvmx_mixx_orhwm_s              cn66xx;
	struct cvmx_mixx_orhwm_s              cn68xx;
	struct cvmx_mixx_orhwm_s              cn68xxp1;
	struct cvmx_mixx_orhwm_s              cn73xx;
	struct cvmx_mixx_orhwm_s              cn78xx;
	struct cvmx_mixx_orhwm_s              cn78xxp1;
	struct cvmx_mixx_orhwm_s              cnf75xx;
};
typedef union cvmx_mixx_orhwm cvmx_mixx_orhwm_t;

/**
 * cvmx_mix#_oring1
 *
 * MIX_ORING1 = MIX Outbound Ring Register \#1
 *
 * Description:
 *  NOTE: To write to the MIX_ORING1 register, a device would issue an IOBST directed at the MIO.
 *        To read the MIX_ORING1 register, a device would issue an IOBLD64 directed at the MIO.
 */
union cvmx_mixx_oring1 {
	uint64_t u64;
	struct cvmx_mixx_oring1_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_0_63                : 64;
#else
	uint64_t reserved_0_63                : 64;
#endif
	} s;
	struct cvmx_mixx_oring1_cn52xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t osize                        : 20; /**< Represents the Outbound Ring Buffer's Size(in 8B
                                                         words). The ring can be as large as 1M entries.
                                                         NOTE: This CSR MUST BE setup written by SW poweron
                                                         (when ODBELL/ORCNT=0). */
	uint64_t reserved_36_39               : 4;
	uint64_t obase                        : 33; /**< Represents the 8B-aligned base address of the first
                                                         Outbound Ring(O-Ring) Entry in system memory.
                                                         NOTE: SW MUST ONLY write to this register during
                                                         power-on/boot code. */
	uint64_t reserved_0_2                 : 3;
#else
	uint64_t reserved_0_2                 : 3;
	uint64_t obase                        : 33;
	uint64_t reserved_36_39               : 4;
	uint64_t osize                        : 20;
	uint64_t reserved_60_63               : 4;
#endif
	} cn52xx;
	struct cvmx_mixx_oring1_cn52xx        cn52xxp1;
	struct cvmx_mixx_oring1_cn52xx        cn56xx;
	struct cvmx_mixx_oring1_cn52xx        cn56xxp1;
	struct cvmx_mixx_oring1_cn61xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_60_63               : 4;
	uint64_t osize                        : 20; /**< Represents the Outbound Ring Buffer's Size(in 8B
                                                         words). The ring can be as large as 1M entries.
                                                         NOTE: This CSR MUST BE setup written by SW poweron
                                                         (when ODBELL/ORCNT=0). */
	uint64_t obase                        : 37; /**< Represents the 8B-aligned base address of the first
                                                         Outbound Ring(O-Ring) Entry in system memory.
                                                         NOTE: SW MUST ONLY write to this register during
                                                         power-on/boot code. */
	uint64_t reserved_0_2                 : 3;
#else
	uint64_t reserved_0_2                 : 3;
	uint64_t obase                        : 37;
	uint64_t osize                        : 20;
	uint64_t reserved_60_63               : 4;
#endif
	} cn61xx;
	struct cvmx_mixx_oring1_cn61xx        cn63xx;
	struct cvmx_mixx_oring1_cn61xx        cn63xxp1;
	struct cvmx_mixx_oring1_cn61xx        cn66xx;
	struct cvmx_mixx_oring1_cn61xx        cn68xx;
	struct cvmx_mixx_oring1_cn61xx        cn68xxp1;
	struct cvmx_mixx_oring1_cn73xx {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t osize                        : 20; /**< Represents the outbound ring (O-Ring) buffer's size (in 8-byte words). The ring can be as
                                                         large as 1MB entries.
                                                         This CSR must be setup written by software poweron (when ODBELL/ORCNT=0). */
	uint64_t reserved_42_43               : 2;
	uint64_t obase                        : 39; /**< Represents the 8-byte aligned base address of the first outbound ring (O-Ring) entry in
                                                         system memory.
                                                         Software must only write to this register during power-on/boot code. */
	uint64_t reserved_0_2                 : 3;
#else
	uint64_t reserved_0_2                 : 3;
	uint64_t obase                        : 39;
	uint64_t reserved_42_43               : 2;
	uint64_t osize                        : 20;
#endif
	} cn73xx;
	struct cvmx_mixx_oring1_cn73xx        cn78xx;
	struct cvmx_mixx_oring1_cn73xx        cn78xxp1;
	struct cvmx_mixx_oring1_cn73xx        cnf75xx;
};
typedef union cvmx_mixx_oring1 cvmx_mixx_oring1_t;

/**
 * cvmx_mix#_oring2
 *
 * MIX_ORING2 = MIX Outbound Ring Register \#2
 *
 * Description:
 *  NOTE: To write to the MIX_ORING2 register, a device would issue an IOBST directed at the MIO.
 *        To read the MIX_ORING2 register, a device would issue an IOBLD64 directed at the MIO.
 */
union cvmx_mixx_oring2 {
	uint64_t u64;
	struct cvmx_mixx_oring2_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_52_63               : 12;
	uint64_t otlptr                       : 20; /**< The outbound ring tail pointer selects the O-Ring entry that the hardware will process
                                                         next. After the hardware completes sending an outbound packet, it increments the O-Ring
                                                         tail pointer.
                                                         The O-Ring tail pointer hardware increment is always modulo MIX(0..1)_ORING2[OSIZE].
                                                         This field is read-only to software. */
	uint64_t reserved_20_31               : 12;
	uint64_t odbell                       : 20; /**< Represents the cumulative total of pending outbound ring (O-Ring) buffer entries. Each
                                                         O-Ring buffer entry contains an L2/DRAM byte pointer and a byte length.
                                                         After software inserts new entries into the O-Ring buffer, it 'rings the doorbell with the
                                                         count of the newly inserted entries.' When the MIX hardware receives the doorbell ring, it
                                                         increments the current doorbell count by the CSR write value.
                                                         Software must never cause the doorbell count for the O-Ring to exceed the size of
                                                         MIX(0..1)_ORING1[OSIZE]. A read of the CSR indicates the current doorbell count. */
#else
	uint64_t odbell                       : 20;
	uint64_t reserved_20_31               : 12;
	uint64_t otlptr                       : 20;
	uint64_t reserved_52_63               : 12;
#endif
	} s;
	struct cvmx_mixx_oring2_s             cn52xx;
	struct cvmx_mixx_oring2_s             cn52xxp1;
	struct cvmx_mixx_oring2_s             cn56xx;
	struct cvmx_mixx_oring2_s             cn56xxp1;
	struct cvmx_mixx_oring2_s             cn61xx;
	struct cvmx_mixx_oring2_s             cn63xx;
	struct cvmx_mixx_oring2_s             cn63xxp1;
	struct cvmx_mixx_oring2_s             cn66xx;
	struct cvmx_mixx_oring2_s             cn68xx;
	struct cvmx_mixx_oring2_s             cn68xxp1;
	struct cvmx_mixx_oring2_s             cn73xx;
	struct cvmx_mixx_oring2_s             cn78xx;
	struct cvmx_mixx_oring2_s             cn78xxp1;
	struct cvmx_mixx_oring2_s             cnf75xx;
};
typedef union cvmx_mixx_oring2 cvmx_mixx_oring2_t;

/**
 * cvmx_mix#_remcnt
 *
 * This register contains the MIX ring buffer remainder counts (useful for hardware debug only).
 *
 */
union cvmx_mixx_remcnt {
	uint64_t u64;
	struct cvmx_mixx_remcnt_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_52_63               : 12;
	uint64_t iremcnt                      : 20; /**< Remaining I-Ring buffer count. Reflects the number of unused/remaining I-Ring entries that
                                                         hardware currently detects in the I-Ring buffer. Hardware uses this value to detect I-Ring
                                                         doorbell overflows. (See MIX(0..1)_ISR[IDBLOVF].)
                                                         When software writes the MIX(0..1)_IRING1[ISIZE], [IREMCNT] is loaded with the
                                                         MIX(0..1)_IRING2[ISIZE] value. (Note: ISIZE should only be written at power-on, when it is
                                                         known that there are no I-Ring entries currently in use by hardware.) When software writes
                                                         to the IDBELL register, the [IREMCNT] is decremented by the CSR write value. When hardware
                                                         issues an I-Ring write request (onto the IOI), REMCNT is incremented by 1. */
	uint64_t reserved_20_31               : 12;
	uint64_t oremcnt                      : 20; /**< Remaining O-Ring buffer count. Reflects the number of unused/remaining O-Ring entries that
                                                         hardware currently detects in the O-Ring buffer. Hardware uses this value to detect O-Ring
                                                         doorbell overflows. (See MIX(0..1)_ISR[ODBLOVF].)
                                                         When software writes the MIX(0..1)_ORING1[OSIZE], [OREMCNT] is loaded with the
                                                         MIX(0..1)_ORING2[OSIZE] value. (Note: [OSIZE] should only be written at power-on, when it
                                                         is known that no O-Ring entries are currently in use by hardware.) When software writes to
                                                         the ODBELL register, OREMCNT is decremented by the CSR write value. When software writes
                                                         to [OREMCNT], it is decremented by the CSR write value. */
#else
	uint64_t oremcnt                      : 20;
	uint64_t reserved_20_31               : 12;
	uint64_t iremcnt                      : 20;
	uint64_t reserved_52_63               : 12;
#endif
	} s;
	struct cvmx_mixx_remcnt_s             cn52xx;
	struct cvmx_mixx_remcnt_s             cn52xxp1;
	struct cvmx_mixx_remcnt_s             cn56xx;
	struct cvmx_mixx_remcnt_s             cn56xxp1;
	struct cvmx_mixx_remcnt_s             cn61xx;
	struct cvmx_mixx_remcnt_s             cn63xx;
	struct cvmx_mixx_remcnt_s             cn63xxp1;
	struct cvmx_mixx_remcnt_s             cn66xx;
	struct cvmx_mixx_remcnt_s             cn68xx;
	struct cvmx_mixx_remcnt_s             cn68xxp1;
	struct cvmx_mixx_remcnt_s             cn73xx;
	struct cvmx_mixx_remcnt_s             cn78xx;
	struct cvmx_mixx_remcnt_s             cn78xxp1;
	struct cvmx_mixx_remcnt_s             cnf75xx;
};
typedef union cvmx_mixx_remcnt cvmx_mixx_remcnt_t;

/**
 * cvmx_mix#_tsctl
 *
 * This register contains the control fields for MIX timestamps. Software can read this register
 * to determine the number pending timestamp interrupts ([TSCNT]), the number outstanding
 * timestamp requests in flight ([TSTOT]), and the number of available timestamp entries (TSAVL)
 * in the timestamp FIFO.
 *
 * Writing to this register advances the MIX(0..1)_TSTAMP FIFO head pointer by 1 and decrements
 * the [TSCNT, TSTOT] pending counts by 1. For example, if software reads [TSCNT] = 2 (two
 * pending timestamp interrupts), it would immediately issue this sequence:
 *
 * 1. a MIX(0..1)_TSTAMP[TSTAMP] read operation followed by MIX(0..1)_TSCTL write operation (i.e.
 * it
 * gets the timestamp value, pops the timestamp FIFO, and decrements pending counts by 1).
 *
 * 2. a MIX(0..1)_TSTAMP[TSTAMP] read operation followed by MIX(0..1)_TSCTL write operation.
 *
 * Note for software: A MIX(0..1)_TSCTL write operation is ignored when
 * MIX(0..1)_TSCTL[TSCNT] = 0 (i.e., TimeStamp FIFO empty).
 */
union cvmx_mixx_tsctl {
	uint64_t u64;
	struct cvmx_mixx_tsctl_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t reserved_21_63               : 43;
	uint64_t tsavl                        : 5;  /**< Number of MIX timestamp entries available for use. [TSAVL] MAX = 4
                                                         (implementation depth of timestamp FIFO).
                                                         [TSAVL] = (IMPLEMENTATION_DEPTH = 4(MAX) - TSCNT). */
	uint64_t reserved_13_15               : 3;
	uint64_t tstot                        : 5;  /**< Number of pending MIX timestamp requests in flight. [TSTOT] must never exceed MAX = 4
                                                         (implementation depth of timestamp FIFO). */
	uint64_t reserved_5_7                 : 3;
	uint64_t tscnt                        : 5;  /**< Number of pending MIX timestamp interrupts. [TSCNT] must never exceed MAX=4
                                                         (implementation depth of timestamp FIFO). */
#else
	uint64_t tscnt                        : 5;
	uint64_t reserved_5_7                 : 3;
	uint64_t tstot                        : 5;
	uint64_t reserved_13_15               : 3;
	uint64_t tsavl                        : 5;
	uint64_t reserved_21_63               : 43;
#endif
	} s;
	struct cvmx_mixx_tsctl_s              cn61xx;
	struct cvmx_mixx_tsctl_s              cn63xx;
	struct cvmx_mixx_tsctl_s              cn63xxp1;
	struct cvmx_mixx_tsctl_s              cn66xx;
	struct cvmx_mixx_tsctl_s              cn68xx;
	struct cvmx_mixx_tsctl_s              cn68xxp1;
	struct cvmx_mixx_tsctl_s              cn73xx;
	struct cvmx_mixx_tsctl_s              cn78xx;
	struct cvmx_mixx_tsctl_s              cn78xxp1;
	struct cvmx_mixx_tsctl_s              cnf75xx;
};
typedef union cvmx_mixx_tsctl cvmx_mixx_tsctl_t;

/**
 * cvmx_mix#_tstamp
 *
 * This register contains the MIX timestamp value.
 *
 */
union cvmx_mixx_tstamp {
	uint64_t u64;
	struct cvmx_mixx_tstamp_s {
#ifdef __BIG_ENDIAN_BITFIELD
	uint64_t tstamp                       : 64; /**< MIX timestamp value. When software sets up an O-Ring entry with
                                                         [49] = 1 (TSTAMP), the packet is tagged with a special 'SOP with TSTAMP' flag as it is
                                                         sent to the BGX. Later the BGX sends sample strobe(s) to capture a global 64-bit timestamp
                                                         value, followed by a 'commit' strobe which writes the last sampled value into the outbound
                                                         timestamp FIFO (max depth = 4) and increments
                                                         MIX(0..1)_TSCTL[TSCNT] to indicate the total number of pending timestamp interrupts.
                                                         If the number of pending timestamp interrupts (MIX(0..1)_TSCTL[TSCNT]) is greater than the
                                                         MIX(0..1)_CTL[TS_THRESH] value, then a programmable interrupt is also triggered (see
                                                         MIX(0..1)_ISR[TS].
                                                         Software then reads MIX(0..1)_TSTAMP[TSTAMP], and must then write MIX(0..1)_TSCTL, which
                                                         will decrement MIX(0..1)_TSCTL[TSCNT] to indicate that a single timestamp interrupt has
                                                         been serviced.
                                                         The MIO-MIX hardware tracks up to MAX = 4 outstanding timestamped outbound packets at a
                                                         time. All subsequent O-RING entries with SOP-TSTAMP will be stalled until software can
                                                         service the 4 outstanding interrupts. Software can read
                                                         MIX(0..1)_TSCTL to determine the number of pending timestamp interrupts (TSCNT), plus the
                                                         number of outstanding timestamp requests in flight (TSTOT), as well as the number of
                                                         available timestamp entries (TSAVL).
                                                         A MIX_TSTAMP read when
                                                         MIX(0..1)_TSCTL[TSCNT] = 0 will result in a return value of all zeroes. Software should
                                                         only read this register when
                                                         MIX(0..1)_ISR[TS] = 1 (or when MIX(0..1)_TSCTL[TSCNT] != 0) to retrieve the timestamp
                                                         value recorded by hardware. If software reads the [TSTAMP] when hardware has not
                                                         recorded a valid timestamp, then an all zeroes value is returned. */
#else
	uint64_t tstamp                       : 64;
#endif
	} s;
	struct cvmx_mixx_tstamp_s             cn61xx;
	struct cvmx_mixx_tstamp_s             cn63xx;
	struct cvmx_mixx_tstamp_s             cn63xxp1;
	struct cvmx_mixx_tstamp_s             cn66xx;
	struct cvmx_mixx_tstamp_s             cn68xx;
	struct cvmx_mixx_tstamp_s             cn68xxp1;
	struct cvmx_mixx_tstamp_s             cn73xx;
	struct cvmx_mixx_tstamp_s             cn78xx;
	struct cvmx_mixx_tstamp_s             cn78xxp1;
	struct cvmx_mixx_tstamp_s             cnf75xx;
};
typedef union cvmx_mixx_tstamp cvmx_mixx_tstamp_t;

#endif
